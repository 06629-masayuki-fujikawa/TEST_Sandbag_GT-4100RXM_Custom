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
 * Description: Obsolete (deprecated) routines
 *
 * Filename: trdevobs.c
 * Author: Odile
 * Date Created: 02/02/00
 * $Source: source/sockapi/trdevobs.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:42:57JST $
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
#include <trglobal.h>

/*
 * tfNotifySentInterfaceIsr() function description.
 * This function is deprecated. Users should use tfNotifyInterfaceIsr()
 * instead.
 * Called by driver ISR routine.
 * Notify the user that there have been packets transmitted.
 * Will cause:
 *  tfCheckSentInterface to return 0 (polling check method),
 * or
 *  tfWaitSentInterface (task pending check method) to return.
 * But only if number of bytes that have been transmitted reaches a
 * configurable threshold tvNotifySendCompLowWater.
 */
void tfNotifySentInterfaceIsr(ttUserInterface interfaceHandle, int bytesSent)
{
    ttEventEntryPtr  eventEntryPtr;
    ttDeviceEntryPtr devPtr;
#ifdef TM_MULTIPLE_CONTEXT
    ttUserContext    currentContext;
#endif /* TM_MULTIPLE_CONTEXT */
    tt32Bit          devBytesNotified;
    tt32Bit          devPacketsNotified;
    tt16Bit          savSentEventCount;

    devPtr = (ttDeviceEntryPtr)interfaceHandle;
#ifdef TM_MULTIPLE_CONTEXT
    currentContext = tfGetCurrentContext();
    tfSetCurrentContext(devPtr->devContextPtr);
#endif /* TM_MULTIPLE_CONTEXT */
    devBytesNotified = devPtr->devBytesNotified + (tt32Bit)bytesSent;
    devPacketsNotified = devPtr->devPacketsNotified + 1;
    if (devBytesNotified > tm_context(tvNotifySendCompLowWater))
/* If we reached the threshold */
    {
        eventEntryPtr = &(devPtr->devSentEvent);
/* Save sent event count before update */
        savSentEventCount = eventEntryPtr->eventCount;
        eventEntryPtr->eventCount = (tt16Bit)(   eventEntryPtr->eventCount
                                              + (tt16Bit)devPacketsNotified );
        devPtr->devBytesNotified = 0;
        devPtr->devPacketsNotified = 0;
/* Only post if send complete task could potentially be pending */
        if (    (savSentEventCount == (tt16Bit)0)
             && (eventEntryPtr->eventPending != TM_8BIT_ZERO) )
        {
/*
 * Only really posting if TM_TASK_SEND is defined, and either
 * TM_TRECK_PREEMPTIVE_KERNEL, or TM_TRECK_NONPREMPTIVE_KERNEL is defined
 */
            tm_kernel_isr_post_sent_event(&(eventEntryPtr->eventUnion));
        }    
    }
    else
    {
        devPtr->devPacketsNotified = devPacketsNotified;
        devPtr->devBytesNotified = devBytesNotified;
    }    
#ifdef TM_MULTIPLE_CONTEXT
    tfSetCurrentContext(currentContext);
#endif /* TM_MULTIPLE_CONTEXT */
}

/*
 * tfNotifyReceiveInterfaceIsr() function description.
 * This function is deprecated. Users should use tfNotifyInterfaceIsr()
 * instead.
 * Called by driver ISR routine.
 * Notify the user that there have been packets received.
 * Will cause:
 *  tfCheckReceiveInterface to return 0 (polling check method),
 * or
 *  tfWaitReceiveInterface (task pending check method) to return.
 */
void tfNotifyReceiveInterfaceIsr(ttUserInterface interfaceHandle)
{
    ttEventEntryPtr eventEntryPtr;
    tt16Bit         savRecvEventCount;

    eventEntryPtr = &(((ttDeviceEntryPtr)interfaceHandle)->devRecvEvent);
/* Save recv event count before update */
    savRecvEventCount = eventEntryPtr->eventCount;
    eventEntryPtr->eventCount++;
/* Only post if recv task could be pending */
    if (    (savRecvEventCount == (tt16Bit)0)
         && (eventEntryPtr->eventPending != TM_8BIT_ZERO) )
    {
/*
 * Only really posting if TM_TASK_RECV is defined, and either
 * TM_TRECK_PREEMPTIVE_KERNEL, or TM_TRECK_NONPREMPTIVE_KERNEL is defined
 */
        tm_kernel_isr_post_recv_event(&(eventEntryPtr->eventUnion));
    }
}
