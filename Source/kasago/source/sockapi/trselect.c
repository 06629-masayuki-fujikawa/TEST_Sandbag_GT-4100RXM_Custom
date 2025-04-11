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
 * Description: BSD Sockets Interface (select)
 *
 * Filename: trselect.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trselect.c $
 *
 * Modification History
 * $Revision: 6.0.2.4 $
 * $Date: 2015/06/08 15:36:14JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
#include <trssl.h>
#endif /* (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER)) */

/*
 * Local types
 */
#ifdef TM_PEND_POST_NEEDED
/* Element in queue of free select pend entries */
typedef struct tsSelPendEntry
{
    ttNode                            selNode;
    ttPendEntry                       selPendEntry;
} ttSelQPendEntry;


/* Lock for the queue of free select pend entries */
typedef ttSelQPendEntry TM_FAR * ttSelQPendEntryPtr;
#endif /* TM_PEND_POST_NEEDED */

/*
 * Local functions
 */
#ifdef TM_PEND_POST_NEEDED
#ifndef TM_USE_PEND_TIMEOUT
void tfSelectTimeOut (ttVoidPtr      timerBlockPtr,
                      ttGenericUnion userParm1,
                      ttGenericUnion userParm2);
#endif /* !TM_USE_PEND_TIMEOUT */
#endif /* TM_PEND_POST_NEEDED */
static int tfSelectCheckList(unsigned int             numberOfSockets,
                             ttPendEntryPtr           pendEntryPtr,
                             fd_set TM_FAR * TM_FAR * outFdSetPtrPtr,
                             fd_set TM_FAR *          readSocketPtr,
                             fd_set TM_FAR *          writeSocketPtr,
                             fd_set TM_FAR *          exceptionSocketPtr);

static int tfSelectCheckEvent(unsigned int   socketDescriptor,
                              ttPendEntryPtr pendEntryPtr,
                              int            checkMask);

#ifdef TM_PEND_POST_NEEDED
static ttSelQPendEntryPtr tfSelectGetPendEntry(void);
static void               tfSelectReturnPendEntry(ttSelQPendEntryPtr);
#endif /* TM_PEND_POST_NEEDED */


#ifdef TM_PEND_POST_NEEDED
#ifndef TM_USE_PEND_TIMEOUT
void tfSelectTimeOut (ttVoidPtr      timerBlockPtr,
                      ttGenericUnion userParm1,
                      ttGenericUnion userParm2)
{
/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);
    
    tm_post((ttPendEntryPtr)userParm1.genVoidParmPtr,
            TM_LOCK_ENTRY_NULL_PTR,
            TM_ETIMEDOUT);
}
#endif /* !TM_USE_PEND_TIMEOUT */
#endif /* TM_PEND_POST_NEEDED */

/*
 * wait for an event on a socket or a group of sockets
 * select is limited on how deep it goes so we use the
 * stack for items to avoid mallocs
 */
int select(int                     numberOfSockets, 
           fd_set TM_FAR *         readSocketPtr,
           fd_set TM_FAR *         writeSocketPtr,
           fd_set TM_FAR *         exceptionSocketPtr,
           struct timeval TM_FAR * timeOutPtr)
{
    fd_set TM_FAR *      outFdSetPtrPtr[3];
    ttCharPtr            outAllocPtr;
    ttCharPtr            outListPtr;
    ttPendEntryPtr       pendEntryPtr;
    tt32Bit              fdSetSize;
    tt32Bit              allocSize;
    int                  numberHits;
    int                  errorCode;
    int                  numberOfFdSets;
#ifdef TM_PEND_POST_NEEDED
    ttSelQPendEntryPtr   selQPendEntryPtr; 
#ifdef TM_USE_PEND_TIMEOUT
    int                  retCode;
    tt32Bit              timeOutVal;
#else /* !TM_USE_PEND_TIMEOUT */
    ttTimerPtr           timerEntryPtr;
    ttGenericUnion       timerParm1;
#endif /* !TM_USE_PEND_TIMEOUT */
    tt32Bit              timeOut;
    tt32Bit              startTime;
    tt32Bit              elapsedTime;
    tt8Bit               hasPended; /* instead of decreasing pendPostCount */
    tt8Bit               allocatePendEntry;
    tt8Bit               timedOut;
#endif /* TM_PEND_POST_NEEDED */
    
    numberHits = 0; /* for compiler warning */
    fdSetSize = (tt32Bit)0; /* for compiler warning */
    errorCode = TM_ENOERROR;
    pendEntryPtr = TM_PEND_ENTRY_NULL_PTR;
    outAllocPtr = (ttCharPtr)0;

#ifdef TM_PEND_POST_NEEDED
    timedOut = TM_8BIT_NO;
    timeOut = TM_UL(0); /* for compiler warning */
    allocatePendEntry = TM_8BIT_NO;
    tm_kernel_set_critical;
    startTime = tvTime;
    tm_kernel_release_critical;
    if (tvPendPostEnabled != TM_8BIT_ZERO)
    {
        allocatePendEntry = TM_8BIT_YES;
        if (timeOutPtr != (struct timeval TM_FAR *)0)
        {
            timeOut = ((tt32Bit)(timeOutPtr->tv_usec)/TM_L(1000))+
                      ((tt32Bit)(timeOutPtr->tv_sec)*TM_MSECS_PER_SECOND);
            if (timeOut == TM_UL(0))
            {
/*
 * We do not need to allocate a pend entry, if the user wants to return
 * right away.
 */
                allocatePendEntry = TM_8BIT_ZERO;
            }
        }
    }
    else
#endif /* TM_PEND_POST_NEEDED */
    {
        TM_UNREF_IN_ARG(timeOutPtr);
    }

/*
 * Validate numberOfSockets variable
 */
    if ((tt32Bit)numberOfSockets > tm_context(tvMaxNumberSockets))
    {
/* 
 * Limit the number of sockets to the maximum allowed.  Do not return an error
 * to allow for backwards compatibility with applicaitons that may pass in 
 * values such as FD_SETSIZE - see Bug ID 1789 for more info
 */        
        numberOfSockets = (int)tm_context(tvMaxNumberSockets);
    }
    if (numberOfSockets > FD_SETSIZE)
    {
/*
 * Do not allow socket descriptor bigger than FD_SETSIZE -1.
 */
        numberOfSockets = FD_SETSIZE;
    }
/* Do not allocate/copy more than what the user set */
    numberOfFdSets = 0;
    if (exceptionSocketPtr)
    {
        numberOfFdSets++;
    }
    if (writeSocketPtr)
    {
        numberOfFdSets++;
    }
    if (readSocketPtr)
    {
        numberOfFdSets++;
    }
    if (numberOfFdSets != 0)
    {
        if (numberOfSockets == FD_SETSIZE)
        {
            fdSetSize = sizeof(fd_set);
        }
        else
        {
            fdSetSize = (tt32Bit)numberOfSockets;
            fdSetSize = sizeof(fd_mask)* (howmany(fdSetSize, NFDBITS));
        }
        allocSize = fdSetSize * (tt32Bit)numberOfFdSets;
        outAllocPtr = (ttCharPtr)(ttVoidPtr)tm_get_raw_buffer(allocSize);
        if (outAllocPtr == (ttCharPtr)0)
        {
            errorCode = TM_ENOBUFS;
        }
        else
        {
            tm_bzero(outAllocPtr, allocSize);
/* Note bzero of outFdSetPtrPtr[] not necessary */
            outListPtr = outAllocPtr;
            if (readSocketPtr)
            {
                outFdSetPtrPtr[0] = (fd_set TM_FAR *)(ttVoidPtr)outListPtr;
                outListPtr = outListPtr + fdSetSize;
            }
            if (writeSocketPtr)
            {
                outFdSetPtrPtr[1] = (fd_set TM_FAR *)(ttVoidPtr)outListPtr;
                outListPtr = outListPtr + fdSetSize;
            }
            if (exceptionSocketPtr)
            {
                outFdSetPtrPtr[2] = (fd_set TM_FAR *)(ttVoidPtr)outListPtr;
            }
        }
    }
#ifdef TM_PEND_POST_NEEDED
    if (errorCode == TM_ENOERROR)
    {
        do
        {
            selQPendEntryPtr = (ttSelQPendEntryPtr)0;
            hasPended = TM_8BIT_ZERO;
            if (tvPendPostEnabled != TM_8BIT_ZERO)
            {
                if (allocatePendEntry != TM_8BIT_ZERO)
                {
/* Get a pend entry from a free list (allocate it if list is empty) */
                    selQPendEntryPtr = tfSelectGetPendEntry();
                    if (selQPendEntryPtr == (ttSelQPendEntryPtr)0)
                    {
/* return TM_ENOBUFS error, if non blocking select return no hit */
                        errorCode = TM_ENOBUFS;
                    }
                    else
                    {
                        pendEntryPtr = &(selQPendEntryPtr->selPendEntry);
                        tm_bzero(pendEntryPtr, sizeof(ttPendEntry));
/* Create a Pend semaphore */
                        (void)tfCreatePendEntry(pendEntryPtr);
                    }
                }
            }
#endif /* TM_PEND_POST_NEEDED */
            if (numberOfFdSets != 0)
            {
                numberHits = tfSelectCheckList( (unsigned int)numberOfSockets, 
                                                pendEntryPtr,
                                                outFdSetPtrPtr,
                                                readSocketPtr, writeSocketPtr,
                                                exceptionSocketPtr);
                if (numberHits == TM_SOCKET_ERROR)
                {
/* Remember we failed, but execute common pendEntry clean up code. */
#ifdef TM_PEND_POST_NEEDED
                    if (errorCode == TM_ENOERROR)
#endif /* TM_PEND_POST_NEEDED */
/*
 * Only set the error, if not already set (if we failed to allocate the
 * pend entry).
 */
                    {
                        errorCode = TM_EBADF;
                    }
                }
            }
#ifdef TM_PEND_POST_NEEDED
            if (tvPendPostEnabled != TM_8BIT_ZERO)
            {
                if (pendEntryPtr != TM_PEND_ENTRY_NULL_PTR)
                {
                    timedOut = TM_8BIT_NO;
                    if (    (numberHits == 0)
                         && (errorCode == TM_ENOERROR)
                       )
                    {
/* Try and pend */
                        hasPended = TM_8BIT_YES;
                        if (timeOutPtr != (struct timeval TM_FAR *)0)
                        {
/* Select has a non zero timeout (since pendEntryPtr is non null) */
#ifdef TM_USE_PEND_TIMEOUT
                            timeOutVal = (tt32Bit)(timeOut/tvTimerTickLength);
                            if (timeOutVal == TM_UL(0))
                            {
                                timeOutVal = TM_UL(1);
                            }
                            retCode =
                                tfKernelPendCountSemTimeout(
                                 (ttUserGenericUnionPtr)
                                 &(pendEntryPtr->pendSemaphorePtr->couCountSem),
                                 timeOutVal);
                            if (retCode != TM_KERN_OKAY)
                            {
/* We timed out */
                                timedOut = TM_8BIT_YES;
                                hasPended = TM_8BIT_ZERO;
                            }
#else /* !TM_USE_PEND_TIMEOUT */
                            timerParm1.genVoidParmPtr = (ttVoidPtr)pendEntryPtr;
                            timerEntryPtr = tfTimerAdd(tfSelectTimeOut,
                                                       timerParm1,
                                                       timerParm1, /* unused */
                                                       timeOut,
                                                       TM_TIM_AUTO);
                            (void)tfKernelPendCountSem(
                               (ttUserGenericUnionPtr)
                               &(pendEntryPtr->pendSemaphorePtr->couCountSem));
                            if (pendEntryPtr->pendStatus == TM_ETIMEDOUT)
                            {
                                timedOut = TM_8BIT_YES;
                            }
/* Remove our timer */
                            tfTimerLockedRemove(timerEntryPtr);
#endif /* !TM_USE_PEND_TIMEOUT */
                            if (timedOut == TM_8BIT_NO)
/* Compute the remaining timeout */
                            {
                                tm_kernel_set_critical;
                                elapsedTime = (tt32Bit)(tvTime - startTime);
                                tm_kernel_release_critical;
                                if (timeOut > elapsedTime) 
                                {
                                    timeOut = (tt32Bit)
                                                (timeOut - elapsedTime);
                                }
                                else
                                {
                                    timedOut = TM_8BIT_YES;
                                }
                            }
                        }
                        else
                        {
/* Block forever */
                            (void)tfKernelPendCountSem(
                                 (ttUserGenericUnionPtr)&(
                                 pendEntryPtr->pendSemaphorePtr->couCountSem));
                        }
                    }
                    if (numberOfFdSets != 0)
/*
 * This code is to allow us to clean up a semaphore that was posted
 * to but never pended on, if we free it back to the system
 * without cleaning it up, the counting semaphore could aleady
 * have a post to it when we recycle it. Also recycle the counting
 * semaphore.
 */
                    {
/* Clean up our pend entries */
                        numberHits = tfSelectCheckList(
                                            (unsigned int)numberOfSockets,
                                            TM_PEND_ENTRY_NULL_PTR,
                                            outFdSetPtrPtr,
                                            readSocketPtr, writeSocketPtr,
                                            exceptionSocketPtr);
                    }
                    tm_kernel_set_critical;
/* Check to see if we posted and have never actually pended */
                    if (    (pendEntryPtr->pendPostCount > 0)
                         && (hasPended == TM_8BIT_ZERO) )
                    {
                        tm_kernel_release_critical;
/* 
 * If so, then go ahead and pend since it will return right away
 * and let the system reset the counting semaphore value.
 */
                         (void)tfKernelPendCountSem(
                               (ttUserGenericUnionPtr)&(
                               pendEntryPtr->pendSemaphorePtr->couCountSem));
                        tm_kernel_set_critical;
                    }
/* Recycle the counting semaphore */
                    tfRecycleSemaphore(pendEntryPtr->pendSemaphorePtr);
                    tm_kernel_release_critical;
/*
 * Return the pendEntry to the free list
 */
                    tfSelectReturnPendEntry(selQPendEntryPtr);
                    pendEntryPtr = (ttPendEntryPtr)0;
                }
                else
                {
                    if (errorCode == TM_ENOBUFS)
/* No pend entry because we did not have enough memory */
                    {
                        if (numberHits > 0)
/* We got some hits without pending */
                        {
                            errorCode = TM_ENOERROR;
                        }
                    }
                }
            }
        }
        while (
                       (numberHits == 0)
                    && (allocatePendEntry != TM_8BIT_ZERO)
                    && (errorCode == TM_ENOERROR)
                    && (timedOut == TM_8BIT_NO)
                    && (numberOfFdSets != 0)
                  ) ;
    }
#endif /* TM_PEND_POST_NEEDED */
    if (errorCode != TM_ENOERROR)
    {
        numberHits = TM_SOCKET_ERROR; /* ignore result of clean-up call */
/* NOTE: RECORD THE ERROR for socket index TM_SOCKET_ERROR */
        tfSocketErrorRecord(TM_SOCKET_ERROR, errorCode);
    }
    else
    {
        if (numberOfFdSets != 0)
        {
            if (readSocketPtr != (fd_set TM_FAR *)0)
            {
                tm_bcopy(outFdSetPtrPtr[0], readSocketPtr, fdSetSize);
            }
            if (writeSocketPtr != (fd_set TM_FAR *)0)
            {
                tm_bcopy(outFdSetPtrPtr[1], writeSocketPtr, fdSetSize);
            }
            if (exceptionSocketPtr != (fd_set TM_FAR *)0)
            {
                tm_bcopy(outFdSetPtrPtr[2], exceptionSocketPtr, fdSetSize);
            }
        }
    }
    if (outAllocPtr != (ttCharPtr)0)
    {
        tm_free_raw_buffer(outAllocPtr);
    }
    return (numberHits);
}

static int tfSelectCheckList(unsigned int             numberOfSockets,
                             ttPendEntryPtr           pendEntryPtr,
                             fd_set TM_FAR * TM_FAR * outFdSetPtrPtr,
                             fd_set TM_FAR *          readSocketPtr,
                             fd_set TM_FAR *          writeSocketPtr,
                             fd_set TM_FAR *          exceptionSocketPtr)
{
    unsigned int     selectIndex;
    unsigned int     bitIndex;
    unsigned int     fdsBitsIndex;
    fd_mask          bitSet;
    fd_mask          readMask;
    fd_mask          writeMask;
    fd_mask          exceptionMask;
    unsigned int     socketDescriptor;
    int              numberHits;
    int              checkMask;
    int              retMask;
    unsigned int     upperBitIndex;
    
    numberHits = 0;
/* LOCK the array to get the socket entries */
    tm_lock_wait(&tm_context(tvSocketArrayLock));
    for ( selectIndex = 0;
          selectIndex < numberOfSockets;
          selectIndex += (unsigned int)NFDBITS )
    {
        bitSet = (fd_mask)1;
        fdsBitsIndex = selectIndex/(unsigned int)NFDBITS;
/* Upper bit index in a given set */
        upperBitIndex = numberOfSockets - selectIndex;
/* 
 * Take the minimum of NFDBITS (biggest index number in a given set) and 
 * upperBitIndex. No need to check further if we have reached the 
 * index in a given set, corresponding to the number of sockets. 
 */
        if (upperBitIndex > (unsigned int)NFDBITS)

        {
            upperBitIndex = (unsigned int)NFDBITS;
        }
/* Move as much code as possible out of the loop */
        if (readSocketPtr != (fd_set TM_FAR *)0)
        {
            readMask = readSocketPtr->fds_bits[fdsBitsIndex];
        }
        else
        {
            readMask = (fd_mask)0;
        }
        if (writeSocketPtr != (fd_set TM_FAR *)0)
        {
            writeMask = writeSocketPtr->fds_bits[fdsBitsIndex];
        }
        else
        {
            writeMask = (fd_mask)0;
        }
        if (exceptionSocketPtr != (fd_set TM_FAR *)0)
        {
            exceptionMask = exceptionSocketPtr->fds_bits[fdsBitsIndex];
        }
        else
        {
            exceptionMask = (fd_mask)0;
        }
        if (    (readMask != (fd_mask)0) 
             || (writeMask != (fd_mask)0) 
             || (exceptionMask != (fd_mask)0) )
/* At least one non zero mask */
        {
            for (bitIndex = 0; (bitIndex < upperBitIndex); bitIndex++)
            {
                checkMask = 0;
                if (bitSet & readMask)
                {
                    checkMask |= TM_SELECT_READ;
                }
                if (bitSet & writeMask)
                {
                    checkMask |= TM_SELECT_WRITE;
                }
                if (bitSet & exceptionMask)
                {
                    checkMask |= TM_SELECT_EXCEPTION;
                }
                if (checkMask != 0)
                {
                    socketDescriptor= selectIndex + bitIndex;
                    retMask = tfSelectCheckEvent( socketDescriptor,
                                                  pendEntryPtr,
                                                  checkMask);
                    if (retMask != 0)
                    {
/*
 * Make sure that retMask is no more than checkMask.
 * Note: should not be necessary to do as tfSelectCheckEvent() should make sure
 * of that.
 */
                        retMask = (checkMask & retMask);
                        if (retMask & TM_SELECT_READ)
/* Note check on outFdSetPtrPtr[0] not necessary */
                        {
                            outFdSetPtrPtr[0]->fds_bits[fdsBitsIndex]
                                                                     |= bitSet;
                            numberHits++;
                        }
                        if (retMask & TM_SELECT_WRITE)
/* Note check on outFdSetPtrPtr[1] not necessary */
                        {
                            outFdSetPtrPtr[1]->fds_bits[fdsBitsIndex]
                                                                     |= bitSet;
                            numberHits++;
                        }
                        if (retMask & TM_SELECT_EXCEPTION)
/* Note check on outFdSetPtrPtr[2] not necessary */
                        {
                            outFdSetPtrPtr[2]->fds_bits[fdsBitsIndex]
                                                                     |= bitSet;
                            numberHits++;
                        }
                    }
                }    
                bitSet = (fd_mask)((ttUser32Bit)bitSet << 1);
            }
        }
    }
/* UNLOCK the array */
    tm_unlock(&tm_context(tvSocketArrayLock));
    return (numberHits);
}

static int tfSelectCheckEvent(unsigned int   socketDescriptor,
                              ttPendEntryPtr pendEntryPtr,
                              int            checkMask)
{
    ttSocketEntryPtr socketEntryPtr;
#ifdef TM_USE_TCP
    ttTcpVectPtr     tcpVectPtr;
    ttTcpVectPtr     conReqTcpVectPtr;
#endif /* TM_USE_TCP */
    tt32Bit          availableSpace;
    tt32Bit          totalInFlightBytes;
    int              retCode;
    int              recvErrorCode;
#ifdef TM_USE_TCP
    tt8Bit           protocol;
#endif /* TM_USE_TCP */

#ifdef TM_USE_BSD_DOMAIN
    socketEntryPtr = (ttSocketEntryPtr)
                    tm_context(tvSocketArray)[socketDescriptor].afsSocketPtr;
#else /* !TM_USE_BSD_DOMAIN */
    socketEntryPtr = tm_context(tvSocketArray)[socketDescriptor];
#endif /* !TM_USE_BSD_DOMAIN */

/* Check the socket descriptor to see if it is valid */
    if (socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR)
    {
/* BUG ID 139 */
/* Register an invalid/closed socket descriptor as a READ, WRITE hit */
/* Make sure that the user selected on those events */
        retCode = (TM_SELECT_READ | TM_SELECT_WRITE) & checkMask;
        goto selectCheckEventFinish;
    }

#ifdef TM_USE_BSD_DOMAIN
    if (    tm_context(tvSocketArray)[socketDescriptor].afsAF 
         == TM_BSD_FAMILY_NONINET)
    {
        retCode = tfBsdSelectCheckEvent( (ttVoidPtr)socketEntryPtr, 
                                         pendEntryPtr, checkMask);
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
/* LOCK the socket entry */
        tm_lock_wait(&(socketEntryPtr->socLockEntry));
        retCode = 0;
#ifdef TM_USE_TCP
        protocol = socketEntryPtr->socProtocolNumber;
#endif /* TM_USE_TCP */
        if (checkMask & TM_SELECT_READ)
        {
#if (defined(TM_USE_SSL_CLIENT) || (defined(TM_USE_SSL_SERVER)))
#ifdef TM_USE_TCP
            if (protocol == IP_PROTOTCP)
            {
                tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr);
                if (    (tm_16bit_all_bits_set(tcpVectPtr->tcpsSslFlags,
                                                TM_SSLF_RECV_ENABLED 
                                              | TM_SSLF_MORE_RECORDS) )
                     || (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                                TM_SSLF_HANDSHK_PROCESS) )
                   )
                {
/*
 * We have at least one full SSL record ready for decryption, or we need to
 * process a handshake message.
 */
                    (void)tfSslIncomingRecord(tcpVectPtr->tcpsSslConnStatePtr,
                                              TM_SSL_RECV_API);
                }
            }
#endif /* TM_USE_TCP */
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */
            if (socketEntryPtr->socRecvQueueBytes != 0)
            {
/* we have data */
                retCode |= TM_SELECT_READ;
            }
            else
            {
                recvErrorCode = (int)socketEntryPtr->socRecvError; 
#ifdef TM_USE_TCP
                if (protocol == IP_PROTOTCP)
/* TCP protocol */
                {
                    tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr);
/* Check for an accept event */
                    if ( tm_16bit_all_bits_set(
                           socketEntryPtr->socFlags,
                           TM_SOCF_LISTENING|TM_SOCF_IN_TREE|TM_SOCF_OPEN ) )
/* Socket is in the tree and is a listening socket */
                    {
                        conReqTcpVectPtr =
                                tcpVectPtr->tcpsAcceptQueue.tcpConReqNextPtr;
                        if (conReqTcpVectPtr != tcpVectPtr)
                        {
/* At leat one socket not yet accepted */
                            retCode |= TM_SELECT_READ;
/* tell the user about it */
                        }
                    }
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
                    else
                    {
                        if (   (tm_16bit_all_bits_set(
                                      tcpVectPtr->tcpsSslFlags,
                                        TM_SSLF_SESSION_SET
                                      | TM_SSLF_RECV_CLOSE_NOTIFY))
                           )
                        {
/* CLOSE NOTIFICATION */
                            retCode |= TM_SELECT_READ;
/* Notify only once */
                            tcpVectPtr->tcpsSslFlags &=
                                                  ~TM_SSLF_RECV_CLOSE_NOTIFY;
/* tell the user about it */
                        }
                    }
#endif /* (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER)) */
/* Check for TCP receive error */
                    if (    ((retCode & TM_SELECT_READ) == 0)
                         && (recvErrorCode == TM_ENOERROR) )
                    {
                        recvErrorCode = tfTcpRecv(tcpVectPtr);
                        if (recvErrorCode == TM_ENOTCONN)
/*
 * If the socket is not connected, it is not ready to read, so should not get
 * a read error event
 */
                        {
                            recvErrorCode = TM_ENOERROR;
                        }
                    }
                }
#endif /* TM_USE_TCP */
                if (recvErrorCode != TM_ENOERROR)
                {
/* recv error registers as read bit for select */
                    retCode |= TM_SELECT_READ;
                }
            }
        }
        if (checkMask & TM_SELECT_WRITE)
        {
            totalInFlightBytes = socketEntryPtr->socSendQueueBytes;
#ifdef TM_USE_TCP
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
            if (protocol == (tt8Bit)IP_PROTOTCP)
            {
                tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr);
                if (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                        TM_SSLF_SEND_ENABLED))
                {
                    totalInFlightBytes =   totalInFlightBytes 
                                         + tcpVectPtr->tcpsSslSendQBytes;
                }
            }
#endif /* (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER)) */
#endif /* TM_USE_TCP */
            if (totalInFlightBytes >= socketEntryPtr->socMaxSendQueueBytes)
            {
                availableSpace = (tt32Bit)0;
            }
            else
            {
                availableSpace =   socketEntryPtr->socMaxSendQueueBytes
                                 - totalInFlightBytes;
            }
            if (availableSpace >= socketEntryPtr->socLowSendQueueBytes)
            {
#ifdef TM_USE_TCP
                if (protocol == IP_PROTOTCP)
                {
/*
 * Allow send only after socket is connected. If user has issued a
 * shutdown let the user know about it too.
 */
                    if ( tm_16bit_one_bit_set(socketEntryPtr->socFlags,
                                TM_SOCF_CONNECTED|TM_SOCF_NO_MORE_SEND_DATA) )
                    {                
                        retCode |= TM_SELECT_WRITE;
                    }
                }
                else
#endif /* TM_USE_TCP */
                {     
/* Connectionless and we have space */
                    retCode |= TM_SELECT_WRITE;
                }
            }
            if (socketEntryPtr->socSendError != 0)
            {
/* send error registers as write bit for select */
                retCode |= TM_SELECT_WRITE;
            }
        }
#ifdef TM_USE_TCP
        if (checkMask & TM_SELECT_EXCEPTION)
        {
            if (protocol == IP_PROTOTCP)
/* TCP protocol */
            {
                tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr);
                if (    (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags2,
                                               TM_TCPF2_OOBDATA_OUT))
                     && (    (socketEntryPtr->socOobMark != TM_UL(0))
                          || (tm_16bit_one_bit_set(socketEntryPtr->socFlags,
                                                   TM_SOCF_RCVATMARK)) )
                   )
                {
/* If urgent data not already copied, and in urgent mode, or at Out of band mark */
                    retCode |= TM_SELECT_EXCEPTION;
                }
#ifdef TM_USE_SSL
                if ( tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                          TM_SSLF_HANDSHK_PROCESS) )
                {
                    retCode |= TM_SELECT_EXCEPTION;
                }
#endif /* TM_USE_SSL */
            }
        }
#endif /* TM_USE_TCP */
#ifdef TM_PEND_POST_NEEDED
        socketEntryPtr->socSelectPendEntryPtr=pendEntryPtr;
        if (pendEntryPtr == TM_PEND_ENTRY_NULL_PTR)
        {
            socketEntryPtr->socSelectFlags = 0;
        }
        else
        {
            socketEntryPtr->socSelectFlags = (tt16Bit)checkMask;
        }    
#else  /* !TM_PEND_POST_NEEDED */
        TM_UNREF_IN_ARG(pendEntryPtr);
#endif /* TM_PEND_POST_NEEDED */
        tm_unlock(&(socketEntryPtr->socLockEntry));
    }

selectCheckEventFinish:
    return(retCode);
}

#ifdef TM_PEND_POST_NEEDED
/* 
 * Return allocated memory for a select pend entry.
 * First look into list of free select pend entries. If list is empty,
 * Allocate a block of TM_SEL_PEND_BLOCK_COUNT entries and insert
 * extra pend entries in the list of free select pend entries.
 * It is done this way to prevent heap fragmentation, as memory allocated
 * for select pend entries are never returned to the heap. 
 */
static ttSelQPendEntryPtr
tfSelectGetPendEntry(void)
{
    ttSelQPendEntryPtr  selQPendEntryPtr;
    int                 i;

    tm_lock_wait(&(tm_context(tvSelPendEntryLock)));
    if (tm_context(tvSelPendEntryHead).listCount == 0)
/* No free select pend entry. Allocate a block */
    {
        for (i = 0; i < TM_SEL_PEND_BLOCK_COUNT; i++)
        {
            selQPendEntryPtr = (ttSelQPendEntryPtr)(ttVoidPtr)
                        tm_get_raw_buffer((unsigned)(sizeof(ttSelQPendEntry)));
            if (selQPendEntryPtr != (ttSelQPendEntryPtr)0)
            {
/* Queue all entries */
                tfListAddToTail(&tm_context(tvSelPendEntryHead), 
                                &(selQPendEntryPtr->selNode));
            }
            else
            {
                break;
            }
        }
    }
    if (tm_context(tvSelPendEntryHead).listCount != 0)
    {
/* Remove a free select pend entry from the free list */
        selQPendEntryPtr = (ttSelQPendEntryPtr)(ttVoidPtr)
                        tm_context(tvSelPendEntryHead).listHeadNode.nodeNextPtr;
        tfListRemove(&tm_context(tvSelPendEntryHead),
                     tm_context(tvSelPendEntryHead).listHeadNode.nodeNextPtr);
    }
    else
    {
        selQPendEntryPtr = (ttSelQPendEntryPtr)0;
    }
    tm_unlock(&(tm_context(tvSelPendEntryLock)));
    return selQPendEntryPtr;
}

/*
 * Insert select pending entry at the end of the list of free select 
 * pend entries.
 */
static void
tfSelectReturnPendEntry(ttSelQPendEntryPtr selQPendEntryPtr)
{
    tm_lock_wait(&(tm_context(tvSelPendEntryLock)));
    tfListAddToTail(&tm_context(tvSelPendEntryHead),
                    &(selQPendEntryPtr->selNode));
    tm_unlock(&(tm_context(tvSelPendEntryLock)));
    return;
}

#ifdef TM_USE_STOP_TRECK
/*
 * Free list of pre-allocated select pend entries for one context.
 */
void tfSelectDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                )
{
    tfListFree(&(tm_context_var(tvSelPendEntryHead)));
}
#endif /* TM_USE_STOP_TRECK */

#endif /* TM_PEND_POST_NEEDED */

