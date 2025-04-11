/*
 * Copyright Notice:
 * Copyright Treck Incorporated  1997 - 2016
 * Copyright Zuken Elmic Japan   1997 - 2016
 * No portions or the complete contents of this file may be copied or
 * distributed in any form (including but not limited to printed or electronic
 * forms) without the expressed written consent of Treck Incorporated OR
 * Zuken Elmic.  Copyright laws and International Treaties protect the
 * contents of this file.  Unauthorized use is prohibited.
 * All rights reserved.
 *
 * Description: Critical Section Locking
 * Filename: trlock.c
 * Author: Paul
 * Date Created: 11/10/97
 * $Source: source/trlock.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2010/09/30 02:57:34JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h> 
#include <trmacro.h>


#if ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED)) )
/*
 * Include
 */
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/* Get a newly allocated semaphore structure, or a recycled semaphore */
static ttCountSemPtr tfCountSemGet(int listIndex);

#else  /* ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED))  */

/* Suppress PC-LINT compile error for unused header files */
#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */


#endif  /* ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED))  */

/*
 * tfLock
 * Gains a lock on a resource
 * Each resource must have an unique ttLockEntry
 */

#ifdef TM_LOCK_NEEDED
int tfLock( ttLockEntryPtr entryToLockPtr,
            tt8Bit         flag)
{
/* New semaphore from Kernel to add to queue */
    ttCountSemPtr      lockSemaphorePtr;
/* Return Code */
    int                status;
    int                kernelErrorCode;
#if defined(TM_USE_SHEAP) || defined(TM_USER_PACKET_DATA_ALLOC)
    int                index;
#endif /* defined(TM_USE_SHEAP) || defined(TM_USER_PACKET_DATA_ALLOC) */
/* Local to avoid multiple dereference */
    tt16Bit            lockCount;

/*
 * Loop if we have to create a semaphore to avoid system call in critical
 * section
 */
    do
    {
        status = TM_LOC_OKAY;
/* set the critical section */
        tm_kernel_set_critical;

/* increment and check the resource lock count */
        lockCount = ++(entryToLockPtr->locCount);
/* If the entry's lock count is 1 then */
        if (lockCount == 1)
        {
/* release the critical section */
            tm_kernel_release_critical;
        }
        else
        {
#ifdef TM_ERROR_CHECKING
/* Should always be NonZero */
            if (lockCount != 0)
            {
#endif /* TM_ERROR_CHECKING */
/* if the entry's lock count is greater than one and we cannot wait */
                if (flag == TM_LOC_NOWAIT)
                {
/* Set the Lock Count back to where we found it */
                    --(entryToLockPtr->locCount);

/* release the critical section*/
                    tm_kernel_release_critical;

/* return would block */
                    status = TM_LOC_WOULDBLOCK;
                }
/* if the entry's lock count is greater than one */
                else
                {
#ifdef TM_AUTO_TEST
                    tfKernelError("tfLock", "Double locking");
#endif /* TM_AUTO_TEST */
                    if (!(entryToLockPtr->locCountSemValid))
                    {
/* 
 * Obtain a semaphore from the unused counting semaphore list
 */
#if defined(TM_USE_SHEAP) || defined(TM_USER_PACKET_DATA_ALLOC)
                        index = -1;
#ifdef TM_USE_SHEAP
                        if  (entryToLockPtr == &tm_global(tvSheapLock)) 
                        {
                            index = TM_GLBL_SHEAP_LOCK_IDX;
                        }
#endif /* TM_USE_SHEAP */
#ifdef TM_USER_PACKET_DATA_ALLOC
                        if (entryToLockPtr == &tm_global(tvPacketDataAllocLock))
                        {
                            index = TM_GLBL_PKTDATA_LOCK_IDX;
                        }
#endif /* TM_USER_PACKET_DATA_ALLOC */
                        if (index != -1)
                        {
                            if (tm_global(tvCountSemFlags[index] != 1))
                            {
                                tfKernelError("tfLock",
                                    "No pre-allocated global counting semaphore");
                            }
                            entryToLockPtr->locCountSemValid =
                                                 tm_global(tvCountSemFlags[index]);
                            if (entryToLockPtr->locCountSemValid)
                            {
                                entryToLockPtr->locCountSemPtr =
                                         &(tm_global(tvCountSems[index]));
                            }
                        }
                        else
#endif /* defined(TM_USE_SHEAP) || defined(TM_USER_PACKET_DATA_ALLOC) */
                        {
/* if there is a recycled semaphore available, save it in this lock entry */
                            entryToLockPtr->locCountSemPtr =
                                         tfCountSemGet(TM_COUSEM_RECYC_INDEX);
                            if (entryToLockPtr->locCountSemPtr
                                                         != (ttCountSemPtr)0)
                            {
                                entryToLockPtr->locCountSemValid = 1;
                            }
                        }
#ifdef TM_FASTER_UNLOCK
/* increment the ownwer count to account for the semaphore */
                        if (entryToLockPtr->locCountSemValid)
                        {
                            ++(entryToLockPtr->locCount);
                        }
#endif /* TM_FASTER_UNLOCK */
                    }
                    if (entryToLockPtr->locCountSemValid)
                    {
/* If we have created a semaphore */
/* release the critical section*/
                        tm_kernel_release_critical;
/* pend on the semaphore */
                        kernelErrorCode =
                            tfKernelPendCountSem((ttUserGenericUnionPtr)
                              &(entryToLockPtr->locCountSemPtr->couCountSem));
                        if (kernelErrorCode == TM_KERN_ERROR)
                        {
#ifdef TM_ERROR_CHECKING
                            tfKernelError("tfLock",
                                    "tfKernelPendCountSem On Old Sem Failed");
#endif /* TM_ERROR_CHECKING */
/* Severe Error */
                            tm_thread_stop;
                        }

                    }
/* If we have not created a semaphore */
                    else
                    {
/* Decrement the resource lock count */
                        --(entryToLockPtr->locCount);

/* release the critical section*/
                        tm_kernel_release_critical;

/* Go to operating system and get a counting semaphore */
                        lockSemaphorePtr = tfCountSemAlloc(TM_8BIT_YES);
                        if (lockSemaphorePtr == TM_LOC_NULL_PTR)
                        {
#ifdef TM_ERROR_CHECKING
                            tfKernelError("tfLock",
                                          "Allocating Semaphore Failed");
#endif /* TM_ERROR_CHECKING */
/* Severe Error */
                            tm_thread_stop;
                        }    
                        kernelErrorCode = 
                            tfKernelCreateCountSem((ttUserGenericUnionPtr)&
                                             (lockSemaphorePtr->couCountSem));
                        if (kernelErrorCode == TM_KERN_ERROR)
                        {
#ifdef TM_ERROR_CHECKING
                            tfKernelError("tfLock",
                                         "tfKernelCreateCountSem Failed");
#endif /* TM_ERROR_CHECKING */
/* Severe Error */
                            tm_thread_stop;
                        }
/* Save the counting semaphore in a recycled list */

                        tm_kernel_set_critical;
                        tfRecycleSemaphore(lockSemaphorePtr);
                        tm_kernel_release_critical;
                        status = TM_LOC_INCOMPLETE;
                    }
                }

#ifdef TM_ERROR_CHECKING
            }
/* Should Never Happen */
            else
            {
/* release the critical section */
                tm_kernel_release_critical;
                tfKernelError("tfLock","Lock Count Overflow");
/* Severe Error */
                tm_thread_stop;
            }
#endif /* TM_ERROR_CHECKING */
        }
    }
/* End of Loop */
    while(status == TM_LOC_INCOMPLETE);

/* Go ahead and return the status */
    return status;
}

/*
 * tfUnLock
 * Release a lock previously obtained
 */

void tfUnLock( ttLockEntryPtr entryToUnLockPtr)
{
/* Local to allow us to exit critical section */
    ttCountSemPtr      lockSemaphorePtr;
    int                kernelErrorCode;
#ifdef TM_FASTER_UNLOCK
    tt16Bit            lockCount;
#endif /* TM_FASTER_UNLOCK */
/* Local to avoid multiple dereference */
    tt8Bit             lockSemValid;

/* Set the critical section and decrement the lock count for the entry */
    tm_kernel_set_critical;

/* save the valid flag to avoid multiple dereference */
    lockSemValid = entryToUnLockPtr->locCountSemValid;

#ifndef TM_FASTER_UNLOCK
/* If nobody else is waiting on this lock */
    if ( --(entryToUnLockPtr->locCount) == 0)
#else /* TM_FASTER_UNLOCK */
/* decrement the lock count */
    lockCount = --(entryToUnLockPtr->locCount);
/* If nobody else is waiting on this lock */
    if (   (lockCount == 0)
        || ((lockCount == 1) && (lockSemValid == 1)))
#endif /* TM_FASTER_UNLOCK */
    {
/* If there is a semaphore to be freed */
        if (lockSemValid != TM_8BIT_ZERO)
        {
/* 
 * Free the semaphore back to the semaphore queue 
 * (except for the global lock) 
 */
#if (defined(TM_USE_SHEAP) || defined(TM_USER_PACKET_DATA_ALLOC))
            if (
#ifdef TM_USE_SHEAP
                   (entryToUnLockPtr != &tm_global(tvSheapLock))
#endif /* TM_USE_SHEAP */
#if (defined(TM_USE_SHEAP) && defined(TM_USER_PACKET_DATA_ALLOC))
                &&
#endif /* TM_USE_SHEAP && TM_USER_PACKET_DATA_ALLOC */
#ifdef TM_USER_PACKET_DATA_ALLOC
                   (entryToUnLockPtr != &tm_global(tvPacketDataAllocLock))
#endif /* TM_USER_PACKET_DATA_ALLOC */
                )
#endif /* TM_USE_SHEAP || TM_USER_PACKET_DATA_ALLOC */
            {
                tfRecycleSemaphore(entryToUnLockPtr->locCountSemPtr);
            }
/* 
 * Clear the lock entry of the semaphore since we put it on the free list
 * or since in the case of the global lock, we do not have any task waiting on
 * the lock.
 */
            entryToUnLockPtr->locCountSemValid = 0;
#ifdef TM_FASTER_UNLOCK
/* decrement the locCount to account for the semaphore */
            --(entryToUnLockPtr->locCount);
#endif /* TM_FASTER_UNLOCK */
        }
/* Ok to exit critical section here */
        tm_kernel_release_critical;
/* Return Sucess */
    }
    else
    {
/* Other people are waiting */
        lockSemaphorePtr = entryToUnLockPtr->locCountSemPtr;

/* Since we saved the semaphore we can release the critical section */
        tm_kernel_release_critical;

#ifdef TM_ERROR_CHECKING
/* We should have a valid semaphore saved away here */
        if (lockSemValid != 1)
        {
/* 
 * Other people were waiting for this semaphore, but there is no
 * Semaphore.  Something is corrupt and should never happen
 */
            tfKernelError("tfUnLock","Lock is Corrupt");
            tm_thread_stop;
        }
        else
#endif /* TM_ERROR_CHECKING  */
        {

/* If other people are waiting then we should signal them */
            kernelErrorCode = tfKernelPostCountSem((ttUserGenericUnionPtr)
                                            &(lockSemaphorePtr->couCountSem));

            if (kernelErrorCode == TM_KERN_ERROR)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelError("tfUnLock","tfKernelPostCountSem");
#endif /* TM_ERROR_CHECKING */
                tm_thread_stop;
            }
        }
    }
    return;
}

#ifdef TM_USE_STOP_TRECK
#if defined(TM_USE_SHEAP) || defined(TM_USER_PACKET_DATA_ALLOC)
void tfLockCountSemGlobalDelete (int index)
{
    if (tm_global(tvCountSemFlags[index]) != 0)
    { 
        tm_global(tvCountSemFlags[index])  = 0;
        tm_global(tvLocks[index]).locCountSemValid = 0;
        (void)tfKernelDeleteCountSem((ttUserGenericUnionPtr)
                    &(tm_global(tvCountSems[index]).couCountSem));
    }
    return;
}
#endif /* defined(TM_USE_SHEAP) || defined(TM_USER_PACKET_DATA_ALLOC) */
#endif /* TM_USE_STOP_TRECK */

#if defined(TM_USE_SHEAP) || defined(TM_USER_PACKET_DATA_ALLOC)
int tfLockCountSemGlobalCreate (int index)
{
    int kernelErrorCode;

    kernelErrorCode = TM_ENOERROR;
    if (tm_global(tvCountSemFlags[index])  == 0) 
    {
        kernelErrorCode = tfKernelCreateCountSem((ttUserGenericUnionPtr)
                    &(tm_global(tvCountSems[index]).couCountSem));
        if (kernelErrorCode == TM_KERN_ERROR)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelError("tfLockCountSemGlobalCreate", "Kernel create count semaphore FAILED.\n");
#endif /* TM_ERROR_CHECKING */
            tm_thread_stop;
        }
        else
        {
            tm_global(tvCountSemFlags[index])  = 1; 
        }
    }
    return kernelErrorCode;
}
#endif /* defined(TM_USE_SHEAP) || defined(TM_USER_PACKET_DATA_ALLOC) */

#ifdef TM_ERROR_CHECKING

/* Array of maximum locks to check */
static struct tsLockCheck    tvLockCheckArray[TM_MAX_LOCK_CHECK];

/* Parallel Check of numberLocs locks */
static void tfLockCheckVarCount (int numberLocks);

/* Timeout to unlock a lock */
static void tfLockCheckTimeout(ttVoidPtr      timerBlockPtr,
                               ttGenericUnion userParm1,
                               ttGenericUnion userParm2);

/*
 * Function Description:
 * Check that the locking mechanism is operating correctly
 * Calls tfLockCheckVarCount() to check if lock works with different
 * number of locks
 */
void tfLockCheck (void)
{
    int i;

    tm_bzero((ttVoidPtr)&tvLockCheckArray[0], sizeof(tvLockCheckArray));
    i = 0;
/* Sequential check */
    while (i < TM_MAX_LOCK_CHECK)
    {
        i++;
        tfLockCheckVarCount(1);
    }
/* Parallel check of TM_MAX_LOCK_CHECK */
    tfLockCheckVarCount(TM_MAX_LOCK_CHECK);
}


/*
 * Timeout function to unlock a locked entry that the check function
 * should be waiting on.
 */
static void tfLockCheckTimeout(ttVoidPtr      timerBlockPtr,
                               ttGenericUnion userParm1,
                               ttGenericUnion userParm2)
{
    ttLockCheckPtr lockCheckPtr;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);
    
    lockCheckPtr = (ttLockCheckPtr)userParm1.genVoidParmPtr;
    lockCheckPtr->lckFlag = 0;
    tm_call_unlock(&lockCheckPtr->lckLockEntry);
}

/*
 * Parallel check of numberLocks locks
 */
static void tfLockCheckVarCount (int numberLocks)
{
    int i;
    ttGenericUnion timerParm1;

    i = 0;
/* Lock all the entries up to numberLocks */
    while (i < numberLocks)
    {
/* Lock the entry */
        tm_call_lock_wait(&tvLockCheckArray[i].lckLockEntry);
        tvLockCheckArray[i].lckFlag = 1;
/* Call the timer to unlock */
        timerParm1.genVoidParmPtr = (ttVoidPtr) (&tvLockCheckArray[i]);
        (void)tfTimerAdd( tfLockCheckTimeout,
                          timerParm1,
                          timerParm1, /* unused */
                          (tt32Bit)( TM_LOCK_CHECK_DELAY + (i * 100) ),
                          0 );
        i++;
    }
    i = 0;
/* Wait on each entry up to numberLocks to wait for the timer to fire */
    while (i < numberLocks)
    {
        tm_call_lock_wait(&tvLockCheckArray[i].lckLockEntry);
/* Timer should have unlocked, otherwise we have a bug */
#ifdef TM_FASTER_UNLOCK
        if (    (tvLockCheckArray[i].lckFlag != 0)
             || (     (    (tvLockCheckArray[i].lckLockEntry.locCount != 2)
                        || (tvLockCheckArray[i].lckLockEntry.locCountSemValid
                                                                         == 0)
                      )
                   && (    (tvLockCheckArray[i].lckLockEntry.locCount != 1)
                        || (tvLockCheckArray[i].lckLockEntry.locCountSemValid
                                                                         != 0)
                      )
                )
           )
#else /* !TM_FASTER_UNLOCK */
        if (    (tvLockCheckArray[i].lckFlag != 0)
             || (tvLockCheckArray[i].lckLockEntry.locCount != 1) )
#endif /* !TM_FASTER_UNLOCK */
        {
/*
 * Timer has not been fired, so entry has not been unlocked. We should not
 * be here.
 */
            if (tvLockEnabled == 0)
            {
                tfKernelWarning( "tfLockCheck",
                                 "Treck Locking is not enabled.\n" );
            }
            else
            {
                tfKernelError("tfLockCheck", "Treck Locking FAILED.\n");
            }
        }
        else
        {
            tfKernelWarning("tfLockCheck", "Treck Locking is WORKING.\n");
        }
        tm_call_unlock(&tvLockCheckArray[i].lckLockEntry);
        i++;
    }
    return;
}

#endif /* TM_ERROR_CHECKING */

#else /* TM_LOCK_NEEDED */
int tvLockDummy = 0;
#endif /* TM_LOCK_NEEDED */

#ifdef TM_PEND_POST_NEEDED
/* 
 * Wait on an event up to a specified timeout value (in ticks).
 * The lock entry is passed in so that we can create and save the pend entry
 * before we unlock and call the Kernel Semaphore Pend
 * Upon return we lock it again
 * NOTE: Only works on LOCKS with TM_LOC_WAIT flag set
 * Return error status
 */
int tfPend( ttPendEntryPtr pendEntryPtr,
            ttLockEntryPtr lockEntryPtr
#ifdef TM_USE_PEND_TIMEOUT
          , tt32Bit tickTimeout 
#endif /* TM_USE_PEND_TIMEOUT */
            )
{
    ttCountSemPtr      pendSemaphorePtr;   
    int                errorCode;
    int                kernelErrorCode;

    if (tvPendPostEnabled != TM_8BIT_ZERO)
    {
        pendSemaphorePtr = tfCreatePendEntry(pendEntryPtr);
        tm_call_unlock(lockEntryPtr);
/* pend on the semaphore */
#ifdef TM_USE_PEND_TIMEOUT
        if (tickTimeout == TM_UL(0))
#endif /* TM_USE_PEND_TIMEOUT */
        {
            kernelErrorCode = tfKernelPendCountSem((ttUserGenericUnionPtr)
                                            &(pendSemaphorePtr->couCountSem));
            if (kernelErrorCode == TM_KERN_ERROR)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelError("tfPend",
                              "tfKernelPendCountSem On New Sem Failed");
#endif /* TM_ERROR_CHECKING */
/* Severe Error */
                tm_thread_stop;
            }
        }
#ifdef TM_USE_PEND_TIMEOUT
        else
        {
            kernelErrorCode = tfKernelPendCountSemTimeout(
                                              (ttUserGenericUnionPtr)
                                             &(pendSemaphorePtr->couCountSem),
                                             tickTimeout);
        }
#endif /* TM_USE_PEND_TIMEOUT */
        tm_kernel_set_critical;
#ifdef TM_USE_PEND_TIMEOUT
        if (kernelErrorCode != TM_KERN_OKAY)
        {
            errorCode = TM_ETIMEDOUT;
            pendEntryPtr->pendCount--;
            pendEntryPtr->pendStatus = TM_ETIMEDOUT;
        }
#endif /* TM_USE_PEND_TIMEOUT */
        if (    ( pendEntryPtr->pendCount == 0 )
             && ( pendEntryPtr->pendSemaphorePtr != TM_LOC_NULL_PTR ) )
        {
            
            pendEntryPtr->pendSemaphorePtr = TM_LOC_NULL_PTR;
/* Free the semaphore back to the semaphore queue */
            tfRecycleSemaphore(pendSemaphorePtr);
        }
        errorCode = (int)pendEntryPtr->pendStatus;
        tm_kernel_release_critical;
        tm_call_lock_wait(lockEntryPtr);
#ifndef TM_LOCK_NEEDED
/* To get rid of unused parameter warning for no lock systems */
        TM_UNREF_IN_ARG(lockEntryPtr);
#endif /* TM_LOCK_NEEDED */
    }
    else
    {
        errorCode = TM_EWOULDBLOCK;
    }
    return errorCode;
}


/*
 * Post an Event to some task that is waiting
 */
void tfPost( ttPendEntryPtr pendEntryPtr, ttLockEntryPtr lockEntryPtr,
             int errorCode)
{
/* Local to allow us to exit critical section */
    ttCountSemPtr      pendSemaphorePtr;   
    int                kernelErrorCode;

/* Set the critical section and decrement the lock count for the entry */
    tm_kernel_set_critical;

    pendSemaphorePtr = pendEntryPtr->pendSemaphorePtr;
/* Make sure that we have a pending semaphore */
    if (pendSemaphorePtr != TM_LOC_NULL_PTR)
    {
        pendEntryPtr->pendStatus = (tt16Bit)errorCode;
        while (pendEntryPtr->pendCount > 0)
        {
/* Count how many times we called the OS to POST */
            pendEntryPtr->pendPostCount++;
/* Other people are waiting */
            pendEntryPtr->pendCount--;

/* Since we saved the semaphore we can release the critical section */
            tm_kernel_release_critical;

#ifdef TM_LOCK_NEEDED
            if (lockEntryPtr != TM_LOCK_ENTRY_NULL_PTR)
            {
                tm_unlock(lockEntryPtr);
            }
#else
/* To get rid of unused parameter warning for no lock systems */
            TM_UNREF_IN_ARG(lockEntryPtr);
#endif /* TM_LOCK_NEEDED */

/* We unlock to allow other tasks to run after the post */

/* If other people are waiting then we should signal them */
            kernelErrorCode = tfKernelPostCountSem((ttUserGenericUnionPtr)
                                          &(pendSemaphorePtr->couCountSem));

             if (kernelErrorCode == TM_KERN_ERROR)
             {
#ifdef TM_ERROR_CHECKING
                 tfKernelError("tfUnLock", "tfKernelPostCountSem");
#endif /* TM_ERROR_CHECKING */
                 tm_thread_stop;
             }


#ifdef TM_LOCK_NEEDED
            if (lockEntryPtr != TM_LOCK_ENTRY_NULL_PTR)
            {
                tm_lock_wait(lockEntryPtr);
            }
#endif /* TM_LOCK_NEEDED */
            tm_kernel_set_critical;
            if (pendEntryPtr->pendPostCount == 0)
            {
                break;
            }
        }
    }
    tm_kernel_release_critical;
}

/*
 * Create a pend entry (but don't pend on it yet)
 * used by select(), and tfPend. Always called in a critical section.
 */
ttCountSemPtr tfCreatePendEntry (ttPendEntryPtr pendEntryPtr)
{
/* Local to allow us to exit critical section */
    ttCountSemPtr      pendSemaphorePtr;   
    int                kernelErrorCode;

/* set critical section */
    tm_kernel_set_critical;
    if (pendEntryPtr->pendSemaphorePtr == TM_LOC_NULL_PTR)
    {
/* obtain a semaphore from the unused counting semaphore list */
        pendSemaphorePtr = tfCountSemGet(TM_COUSEM_RECYC_INDEX);
        if (pendSemaphorePtr == (ttCountSemPtr)0)
        {
/* release the critical section*/
            tm_kernel_release_critical;
/* Go to operating system and get a counting semaphore */
            pendSemaphorePtr = tfCountSemAlloc(TM_8BIT_YES);
            if (pendSemaphorePtr == TM_LOC_NULL_PTR)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelError("tfPend","tfCountSemAlloc Failed");
#endif /* TM_ERROR_CHECKING */
/* Severe Error */
                tm_thread_stop;
            }
            kernelErrorCode = tfKernelCreateCountSem((ttUserGenericUnionPtr)
                                           &(pendSemaphorePtr->couCountSem));
            if (kernelErrorCode == TM_KERN_ERROR)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelError("tfPend","tfKernelCreateCountSem Failed");
#endif /* TM_ERROR_CHECKING */
/* Severe Error */
                tm_thread_stop;
            }
/* set the critical section */
            tm_kernel_set_critical;
        }
/* initialize pendEntry with new semaphore */
        pendEntryPtr->pendSemaphorePtr = pendSemaphorePtr;
        pendEntryPtr->pendPostCount = 0;
        pendEntryPtr->pendCount = 1;
        pendEntryPtr->pendStatus = (tt16Bit)0;
    }
    else
    {
        pendSemaphorePtr = pendEntryPtr->pendSemaphorePtr;
        pendEntryPtr->pendCount++;
    }
/* release the critical section*/
    tm_kernel_release_critical;
    return pendSemaphorePtr;
}
#else /* TM_PEND_POST_NEEDED */
int tvPendPostDummy = 0;
#endif /* TM_PEND_POST_NEEDED */

#if ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED)) )
/*
 * Insert counting semaphore in list of free counting semaphores.
 * Always called in critical section.
 */
void tfRecycleSemaphore (ttCountSemPtr semaphorePtr)
{
    semaphorePtr->couNext = tm_context(tvCountSemRecycListPtr);
    tm_context(tvCountSemRecycListPtr) = semaphorePtr;
    tm_context(tvCountSemUsedCount)--; /* Not in use */
    return;
}

/*
 * Get a newly allocated semaphore structure, or a recycled semaphore 
 * Remove counting semaphore from specified list
 * (list of allocated counting semaphores, or recycled ones.)
 * Always called in critical section.
 */
static ttCountSemPtr tfCountSemGet (int listIndex)
{
    ttCountSemPtr   countSemPtr;

    countSemPtr = tm_context(tvCountSemListPtrArr[listIndex]);
    if (countSemPtr != (ttCountSemPtr)0)
    {
/* Remove from the allocation list */
        tm_context(tvCountSemListPtrArr[listIndex]) = countSemPtr->couNext;
        tm_context(tvCountSemUsedCount)++; /* In use */
    }
    return countSemPtr;
}

/* 
 * Return allocated memory for a counting semaphore.
 * Allocate from a queue of preallocated counting semaphores. New
 * pre-allocated block is allocated in this routine if 
 * there is no more in the queue.
 * It is done this way to prevent heap fragmentation, as memory allocated
 * for counting semaphores are returned to the heap only when context is
 * being freed.
 * Also tfCountSemAlloc() will be called from the initialization routine, after 
 * heap has been initialized, to make sure that this memory is allocated at 
 * the beginning of the heap.
 */
ttCountSemPtr tfCountSemAlloc(tt8Bit getSemaphore)
{
    ttCountSemPtr   countSemPtr;
    int             i;

    tm_kernel_set_critical;
    if (tm_context(tvCountSemAllocListPtr) == (ttCountSemPtr)0)
    {
        for (i = 0; i < TM_COUNT_SEM_BLOCK_COUNT; i++)
        {
            tm_kernel_release_critical;
            countSemPtr = (ttCountSemPtr)(ttVoidPtr)tm_kernel_malloc(
                                      (unsigned)(sizeof(ttCountSem)) );
            tm_kernel_set_critical;
            if (countSemPtr != (ttCountSemPtr)0)
            {
                countSemPtr->couNext = tm_context(tvCountSemAllocListPtr);
                tm_context(tvCountSemAllocListPtr) = countSemPtr;
            }
            else
            {
                break;
            }
        }
    } 
    if (getSemaphore)
    {
        countSemPtr = tfCountSemGet(TM_COUSEM_ALLOC_INDEX);
    }
    else
    {
        countSemPtr = (ttCountSemPtr)0;
    }
    tm_kernel_release_critical;
    return countSemPtr;
}

#ifdef TM_USE_STOP_TRECK
/* Delete and Free list of counting semaphores for one context */
int tfCountSemDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                )
{
    ttCountSemPtr   countSemPtrArr[2];
    ttCountSemPtr   countSemPtr;
    int             errorCode;
    int             i;

    if (tm_context_var(tvCountSemUsedCount) != 0)
    {
/* There are counting semaphores still in use. Return an error to the user. */
        errorCode = TM_EPERM;
    }
    else
    {
        errorCode = TM_ENOERROR;
/* Move alloc, and recycle lists to local ones */
        tm_kernel_set_critical;
        for (i = 0; i < 2; i++)
        {
            countSemPtrArr[i] = tm_context_var(tvCountSemListPtrArr[i]);
            tm_context_var(tvCountSemListPtrArr[i]) = (ttCountSemPtr)0;
        }
        tm_kernel_release_critical;
/*
 * Delete all recycled counting semaphores, and free all recycled/allocated
 * ones
 */
        for (i = 0; i < 2; i++)
        {
            while (countSemPtrArr[i] != (ttCountSemPtr)0)
            {
                countSemPtr = countSemPtrArr[i];
                countSemPtrArr[i] = countSemPtr->couNext;
                if (i == TM_COUSEM_RECYC_INDEX)
                {
                    (void)tfKernelDeleteCountSem((ttUserGenericUnionPtr)
                                                 &(countSemPtr->couCountSem));
                }
                tm_kernel_free(countSemPtr);
            }
        }
    }
    return errorCode;
}
#endif /* TM_USE_STOP_TRECK */
#endif  /* ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED))  */
