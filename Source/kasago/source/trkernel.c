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
 * Description: Kernel Hooks
 *              NOTE: This File is intended to be modified by the user
 *                    In order to add a new operating system
 * Filename: trkernel.c
 * Author: Paul
 * Date Created: 11/10/97
 * $Source: source/trkernel.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2010/04/02 16:39:42JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * This file is an example stub file. Please modify this file for the target
 * platform or use an example provided in the 'source/kernel' directory
 */
#define TM_COLOR_PRINT_MAX_LINE 256


#include <stdio.h>
#include <stdlib.h>
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>

// MH810100(S) S.Fujii 2020/07/17 warning‘Î‰ž
extern	void	_di( void );
extern	void	_ei( void );
// MH810100(E) S.Fujii 2020/07/17 warning‘Î‰ž

/* 
 * Dynamically allocate the Treck simple heap. Only called when
 * TM_DYNAMIC_CREATE_SHEAP has been #define'd.
 * User can change the size of the created heap by updating *sheapSizePtr
 * prior to calling malloc.
 * The start of the simple heap (i.e. the pointer returned) being
 * aligned on a 32-bit boundary.
 * Return:
 * Pointer to beginning of dynamically created heap.
 */
#ifdef TM_DYNAMIC_CREATE_SHEAP
ttUser32Bit TM_FAR * tfKernelSheapCreate (ttUser32Bit TM_FAR * sheapSizePtr)
{
/* *sheapSizePtr can be changed by the user */

/*
 * NOTE 'C' library malloc() is not re-entrant. If malloc() were to be used
 * for TM_TRECK_PREEMPTIVE_KERNEL, it would have to be protected (with a
 * mutex for example).
 */
    return (ttUser32Bit TM_FAR *)(tfKernelMalloc((*sheapSizePtr)));
}

#ifdef TM_USE_STOP_TRECK
void tfKernelSheapFree (ttUser32Bit TM_FAR * sheapPtr)
{
/*
 * NOTE 'C' library free() is not re-entrant. If free() were
 * to be used for TM_TRECK_PREEMPTIVE_KERNEL, it would have to be protected
 * (with a mutex for example).
 */
    tfKernelFree(sheapPtr);
    return;
}
#endif /* TM_USE_STOP_TRECK */
#endif /* TM_DYNAMIC_CREATE_SHEAP */

/*
 * Allocate a memory block of size from the kernel
 */
void TM_FAR * tfKernelMalloc(unsigned size)
{
/*
 * NOTE 'C' library malloc() is not re-entrant. If malloc() were to be used
 * for TM_TRECK_PREEMPTIVE_KERNEL, it would have to be protected (with a
 * mutex for example).
 */
/*    memoryPtr = (void TM_FAR *)(malloc((unsigned)size)); */
    TM_UNREF_IN_ARG(size);
    return (void TM_FAR *)0;
}

/*
 * Free a memory block back to the kernel
 */
void tfKernelFree(void TM_FAR *memoryBlockPtr)
{
/*
 * NOTE 'C' library free() is not re-entrant. If free() were to be used
 * for TM_TRECK_PREEMPTIVE_KERNEL, it would have to be protected (with a
 * mutex for example).
 */
/*   free(memoryBlockPtr); */
    TM_UNREF_IN_ARG(memoryBlockPtr);
    return;
}

/*
 * Stop all other calls into Treck by 
 * a kernel context switch
 */
void tfKernelSetCritical(void)
{
    /*
    Example:
    kernelDisableInterrupt();
    */
// MH810100(S)
	_di();
// MH810100(E)
    return;
}

/*
 * Resume normal operation (from tm_kernel_set_critical)
 */
void tfKernelReleaseCritical(void)
{
    /*
    Example:
    kernelEnableInterrupt();
    */
// MH810100(S)
	_ei();
// MH810100(E)
    return;
}

/*
 * Counting Semaphore Operations
 *
 * The examples assume the following:
 * A semaphore is of type int
 * Success from a Kernel call is 0
 * Failure from a Kernel Call is -1
 *
 */

/*
 * Create a counting semaphore
 */
int tfKernelCreateCountSem(ttUserGenericUnionPtr countingSemaphore)
{
    /*
    Example:
    int kernelSemaphore;
    int errorCode;

    tm_get_current_context(contextHandle);
    kernelSemaphore=kernelCreateSemaphore();
    tm_set_current_context(contextHandle);
    if(kernelSemaphore == -1)
    {
        errorCode = TM_KERN_ERROR;
    }
    else
    {
        countingSemaphore->genIntParm=kernelSemaphore;
        errorCode = TM_KERN_OKAY;
    }
    return errorCode;
    */
    TM_UNREF_IN_ARG(countingSemaphore);
    return TM_KERN_OKAY;
}

/*
 * Wait on a counting semaphore
 */
int tfKernelPendCountSem(ttUserGenericUnionPtr countingSemaphore)
{
    /*
    Example:
    int     errorCode;
    tm_get_current_context(contextHandle);
    if (kernelPend(countingSemaphore->genIntParm)==0)
    {
        errorCode = TM_KERN_OKAY;
    }
    else
    {
        errorCode = TM_KERN_ERROR;
    }
    tm_set_current_context(contextHandle);
    return errorCode;
    */
    TM_UNREF_IN_ARG(countingSemaphore);
    return TM_KERN_OKAY;
}

#ifdef TM_USE_PEND_TIMEOUT
/*
 * Wait on a counting semaphore up to a timeout value (in ticks)
 */
int tfKernelPendCountSemTimeout( ttUserGenericUnionPtr countingSemaphore, 
                                 ttUser32Bit tickTimeout )
{
    /*
    Example:
    int     errorCode;
    tm_get_current_context(contextHandle);
    if (kernelPendTimeout(countingSemaphore->genIntParm, tickTimeout)==0)
    {
        errorCode = TM_KERN_OKAY;
    }
    else
    {
        errorCode = TM_ETIMEDOUT;
    }
    tm_set_current_context(contextHandle);
    return errorCode;
    */
    TM_UNREF_IN_ARG(countingSemaphore);
    TM_UNREF_IN_ARG(tickTimeout);
    return TM_KERN_OKAY;
}
#endif /* TM_USE_PEND_TIMEOUT */

/*
 * Resume waiting tasks on a counting semaphore
 */
int tfKernelPostCountSem(ttUserGenericUnionPtr countingSemaphore)
{
    /*
    Example:
    int errorCode;
    tm_get_current_context(contextHandle);
    if (kernelPost(countingSemaphore->genIntParm)==0)
    {
        errorCode = TM_KERN_OKAY;
    }
    else
    {
        errorCode = TM_KERN_ERROR;
    }
    tm_set_current_context(contextHandle);
    return errorCode;
    */
    TM_UNREF_IN_ARG(countingSemaphore);
    return TM_KERN_OKAY;
}

/*
 * Delete a counting semaphore
 */
int tfKernelDeleteCountSem(ttUserGenericUnionPtr countingSemaphore)
{
    /*
    Example:
    tm_get_current_context(contextHandle);
    if (kernelDeleteSemaphore(countingSemaphore->genIntParm)==0)
    {
        errorCode = TM_KERN_OKAY;
    }
    else
    {
        errorCode = TM_KERN_ERROR;
    }
    tm_set_current_context(contextHandle);
    return errorCode;
    */
    TM_UNREF_IN_ARG(countingSemaphore);
    return TM_KERN_OKAY;
}

void  tfKernelError(const char TM_FAR *functionName,
                    const char TM_FAR *errorMessage)
{
    TM_UNREF_IN_ARG(functionName);
    TM_UNREF_IN_ARG(errorMessage);
    while(1);
}

void  tfKernelWarning(const char TM_FAR *functionName,
                      const char TM_FAR *errorMessage)
{
    TM_UNREF_IN_ARG(functionName);
    TM_UNREF_IN_ARG(errorMessage);
    return;
}

void  tfKernelInitialize(void)
{
    return;
}

void  tfKernelUnInitialize(void)
{
    return;
}

/*
 * Create an event structure for pend/post from an ISR, and
 * also for XMIT task pend/post.
 */
void tfKernelCreateEvent(ttUserGenericUnionPtr eventPtr)
{
    TM_UNREF_IN_ARG(eventPtr);
    return;
}

/*
 * Task wait on an Event
 */
void tfKernelPendEvent(ttUserGenericUnionPtr eventPtr)
{
    TM_UNREF_IN_ARG(eventPtr);
    return;
}

#ifdef TM_USE_PEND_TIMEOUT
/*
 * Task wait on an Event up to a timeout value (in ticks).
 */
int tfKernelPendEventTimeout( ttUserGenericUnionPtr eventPtr, 
                               ttUser32Bit tickTimeout )
{
    /*
    Example:
    int     errorCode;
    tm_get_current_context(contextHandle);
    if (kernelEventPendTimeout(eventPtr->genIntParm, timeout)==0)
    {
        errorCode = TM_KERN_OKAY;
    }
    else
    {
        errorCode = TM_KERN_ERROR;
    }
    tm_set_current_context(contextHandle);
    return errorCode;
    */
    TM_UNREF_IN_ARG(eventPtr);
    TM_UNREF_IN_ARG(tickTimeout);
    return TM_KERN_OKAY;
}
#endif /* TM_USE_PEND_TIMEOUT */

/*
 * Resume task waiting on this Event. Called from a task.
 */
void tfKernelTaskPostEvent(ttUserGenericUnionPtr eventPtr)
{
    TM_UNREF_IN_ARG(eventPtr);
    return;
}

/*
 * Resume task waiting on this event. Called from an ISR.
 */
void tfKernelIsrPostEvent(ttUserGenericUnionPtr eventPtr)
{
    TM_UNREF_IN_ARG(eventPtr);
    return;
}

/*
 * Yield the CPU. Used by tfInterfaceSpinLock().
 */
void tfKernelTaskYield(void)
{
    return;
}

int tfKernelPrint(int msgType, const char TM_FAR * format, ...)
{
    int                         nResult;
    char                        buffer[TM_COLOR_PRINT_MAX_LINE];
    va_list                     args;
    
    va_start(args, format);
    nResult = tfVSPrintF(buffer, format, args);
    va_end(args);

    TM_UNREF_IN_ARG(msgType);
    TM_UNREF_IN_ARG(format);
    nResult = 0;

    printf("%s", buffer);
    return nResult;
}

/* 
 * Treck Stack calls this fucntion to ask for the system time. It is 
 * used in the appliction protocols such as the web server and SMTP client.
 * User could get the time from the system clock, from network, 
 * or from application user interface.
 * 
 * Parameters:
 *  secondsPtr: 
 *      Pointer to store number of seconds elapsed since midnight (00:00:00), 
 *      January 1, 1970, Coordinated Universal Time (UTC). 
 *      UTC is the international time standard. It is the current term for
 *      what was commonly referred to as Greenwich Meridian Time (GMT). 
 *      This matches the return value of the ANSI C library 
 *      function time_t time(time_t *)
 *  daysPtr:
 *      The maximum number of seconds a 32 bit unsigned integer can hold is
 *      2**32 = 4G. this is approximately 126 year. 
 *      In case this value ever wrap arounds, we need another space to hold the 
 *      a bigger units of time. This is the purpose of daysPtr. 
 *  NOTE: The total laps of time is the days stored in daysPtr plus the seconds
 *        stored in the secondsPtr  
 * Returns:
 *  TM_KERN_OKAY:   Sucessful
 *  TM_KERN_ERROR:  Failed to obtain the system time.
 *  
 */

/*
 * The user can replace Treck's internal random number generator by defining
 * TM_USE_KERNEL_RNG in trsystem.h and replacing the contents of this function
 * to return a random 32-bit value.
 */ 
#ifdef TM_USE_KERNEL_RNG
ttUser32Bit tfKernelGetRandom (void)
{
    return TM_KERN_OKAY;
}
#endif /* TM_USE_KERNEL_RNG */
