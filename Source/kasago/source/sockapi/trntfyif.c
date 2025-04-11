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
 * Description: tfNotifyInterfaceTask
 *
 * Filename: trntfyif.c
 * Author: Odile
 * Date Created: 02/25/00
 * $Source: source/sockapi/trntfyif.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:51JST $
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
 * tfNotifyInterfaceTask() function description
 * Function Called by a driver routine (but not the ISR routine) to notify
 * the stack of incoming packets, when bug in the chip causes it to fail
 * to interrupt for a packet. The NE2000 (DP8390 chip) driver uses this
 * function for incoming packets.
 * VERY IMPORTANT: SHOULD NOT BE CALLLED FROM AN ISR. From an ISR, the
 *                 notification function is tfNotifyInterfaceIsr()
 * Notify the user with the number of packets received,
 * and the number of packets that have been transmitted by the chip, if
 * the chip failed to interrupt for these events.
 * . If number of packets received is non zero:
 *   Will cause:
 *    tfCheckReceiveInterface to return 0 (polling check method),
 *   or
 *    tfWaitReceiveInterface (task pending check method) to return.
 * . If number of packets transmitted by the chip is non zero:
 *    Will cause:
 *     tfCheckSentInterface to return 0 (polling check method),
 *    or
 *     tfWaitSentInterface (task pending check method) to return.
 *    But only if number of bytes that have been transmitted reaches a
 *    configurable threshold tvNotifySendCompLowWater.
 * Parmaeters:
 * interfaceHandle           Interface handle as returned by tfAddInterface()
 * numberRecvPackets         Count of Number of packets received in the ISR.
 *                           A zero value indicates that the ISR did not
 *                           get any recv packet notification
 * numberSendCompletePackets Total number of send complete packets. A zero
 *                           value indicates that the ISR did not get any
 *                           send complete notification.
 * totalNumberBytesSent      Total number of bytes sent in the send complete
 *                           packets.
 * Return
 * No return
 */
void tfNotifyInterfaceTask ( ttUserInterface interfaceHandle,
                             int             numberRecvPackets,
                             int             numberSendCompletePackets,
                             ttUser32Bit     numberBytesSent,
                             ttUser32Bit     flag )
{
    ttDeviceEntryPtr devPtr;
#ifdef TM_MULTIPLE_CONTEXT
    ttUserContext    currentContext;
#endif /* TM_MULTIPLE_CONTEXT */
    ttEventEntryPtr  recvEventEntryPtr;
    ttEventEntryPtr  sentEventEntryPtr;
    tt32Bit          devBytesNotified;
    tt32Bit          devPacketsNotified;
    tt16Bit          savRecvEventCount;
    tt16Bit          savSentEventCount;
    tt8Bit           savRecvPending;
    tt8Bit           savSentPending;

    TM_UNREF_IN_ARG(flag); /* Unused parameter */
    devPtr = (ttDeviceEntryPtr)interfaceHandle;
#ifdef TM_MULTIPLE_CONTEXT
    currentContext = tfGetCurrentContext();
    tfSetCurrentContext(devPtr->devContextPtr);
#endif /* TM_MULTIPLE_CONTEXT */
    recvEventEntryPtr = &(devPtr->devRecvEvent);
    sentEventEntryPtr = &(devPtr->devSentEvent);
    savRecvEventCount = TM_16BIT_ZERO;
    savSentEventCount = TM_16BIT_ZERO;
    savRecvPending = TM_8BIT_NO;
    savSentPending = TM_8BIT_NO;
    if (numberRecvPackets != 0)
    {
        tm_kernel_set_critical;
/*
 * Save counts of recv events. This tells us whether the recv task
 * has backed up work.
 */
        savRecvEventCount = recvEventEntryPtr->eventCount;
/* Save recv task pending status */
        savRecvPending = recvEventEntryPtr->eventPending;
        recvEventEntryPtr->eventCount = (tt16Bit)(
                                             recvEventEntryPtr->eventCount
                                           + (tt16Bit)numberRecvPackets );
        tm_kernel_release_critical;
    }
    devPacketsNotified = 0;
    if (numberSendCompletePackets != 0)
    {
        tm_kernel_set_critical;
/*
 * Save count of sent events. This tells us wheter the send complete
 * task has backed up work.
 */
        savSentEventCount = sentEventEntryPtr->eventCount;
/* Save send complete task pending status */
        savSentPending = sentEventEntryPtr->eventPending;
        devBytesNotified = devPtr->devBytesNotified + numberBytesSent;
        devPacketsNotified =   devPtr->devPacketsNotified
                             + (tt32Bit)numberSendCompletePackets;
        if (devBytesNotified > tm_context(tvNotifySendCompLowWater))
        {
            sentEventEntryPtr->eventCount = (tt16Bit)(
                                             sentEventEntryPtr->eventCount
                                           + (tt16Bit)devPacketsNotified );
            devPtr->devBytesNotified = 0;
            devPtr->devPacketsNotified = 0;
        }
        else
        {
            devPtr->devPacketsNotified = devPacketsNotified;
            devPtr->devBytesNotified = devBytesNotified;
/* Postpone notification until we reach the threshold */
            devPacketsNotified = 0;
        }
        tm_kernel_release_critical;
    }
/*
 * Try and post only if we are notified of a received packet, and the recv
 * task is pending / about to pend.
 */
    if (    (numberRecvPackets != 0)
         && (savRecvPending != TM_8BIT_NO)
         && (savRecvEventCount == (tt16Bit)0) )
    {
/*
 * Only really posting if TM_TASK_RECV is defined, and either
 * TM_TRECK_PREEMPTIVE_KERNEL, or TM_TRECK_NONPREMPTIVE_KERNEL is defined
 */
        tm_kernel_post_recv_event(&(recvEventEntryPtr->eventUnion));
    }
/*
 * Try and post only if we are notified of a sent packet (and we reached
 * the threshold), and the send complete task is pending / about to pend.
 */
    if (    (devPacketsNotified != 0)
         && (savSentPending != TM_8BIT_NO)
         && (savSentEventCount == (tt16Bit)0) )
    {
/*
 * Only really posting if TM_TASK_SEND is defined, and either
 * TM_TRECK_PREEMPTIVE_KERNEL, or TM_TRECK_NONPREMPTIVE_KERNEL is defined
 */
        tm_kernel_post_sent_event(&(sentEventEntryPtr->eventUnion));
    }
#ifdef TM_MULTIPLE_CONTEXT
    tfSetCurrentContext(currentContext);
#endif /* TM_MULTIPLE_CONTEXT */
}
