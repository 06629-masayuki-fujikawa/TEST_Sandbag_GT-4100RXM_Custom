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
 * Description: Interface Transmit Task Functions
 *
 * Filename: trxmttsk.c
 * Author: Odile
 * Date Created: 01/31/00
 * $Source: source/sockapi/trxmttsk.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:56JST $
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
#if (defined(TM_LOCK_NEEDED)) || (defined(TM_SNMP_MIB) && defined(TM_USE_IPV4))
#include <trglobal.h>
#endif /* TM_LOCK_NEEDED || (TM_SNMP_MIB && TM_USE_IPV4) */

/*
 * Transmit a packet from the Interface. Called from the transmit task
 * by the user.
 * 1. Lock the device driver lock
 * 2. If device is opened
 *  2.1 If there is a packet to transmit in the send queue, point
 *      packet to transmit to next packet in the send queue, and send the
 *      current packet, using the post link send function. Error code is
 *      as returned by the post link send function.
 *  2.2 If there is no packet to transmit, error is TM_ENOENT.
 * 3. If device is not opened error is TM_ENXIO
 * 3. Unlock the device driver lock
 * 4. Return error code to the user.
 * Parameters
 * interfaceHandle  Interface handle as returned by tfAddInterface()
 * Return
 * value       meaning
 * TM_ENOERROR A packet was successfully sent to the driver. 
 * TM_ENOENT   No packet waiting to be sent
 * TM_ENXIO    Device was not opened/configured.
 * TM_EIO      At least the last buffer in the packet was not sent by the
 *             device driver. It was then removed from the send queue,
 *             and freed.
 */
int tfXmitInterface(ttUserInterface interfaceHandle)
{
    ttPacketPtr      packetPtr;
    ttDeviceEntryPtr devEntryPtr;
    int              retCode;
    tt8Bit           needFreePacket;

    retCode = TM_ENXIO; /* If device is not opened */
    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
    needFreePacket = TM_8BIT_ZERO;
    packetPtr = TM_PACKET_NULL_PTR;
    tm_call_lock_wait(&(devEntryPtr->devDriverLockEntry));
    if ( tm_16bit_one_bit_set( devEntryPtr->devFlag, TM_DEV_OPENED) )
/* Device opened */
    {
        packetPtr = devEntryPtr->devXmitTaskPacketPtr;
        if (packetPtr != TM_PACKET_NULL_PTR)
/* There is a packet to transmit in the interface send queue */
        {
/* Update next packet to transmit in the send queue */
            devEntryPtr->devXmitTaskPacketPtr = packetPtr->pktChainNextPtr;
            devEntryPtr->devSendQueueCurPtr = packetPtr;
#ifdef TM_USE_DRV_ONE_SCAT_SEND
            if (devEntryPtr->devOneScatSendFuncPtr 
                            != (ttDevOneScatSendFuncPtr)0)
            {
/* 
 * Bypass tfDeviceSendOnePacket, and call the device driver send function
 * only once with the scattered send buffer.
 */
                retCode = (*(devEntryPtr->devOneScatSendFuncPtr))(
                                    (ttUserInterface)devEntryPtr, 
                                    (ttUserPacketPtr)packetPtr );
            }
            else
#endif /* TM_USE_DRV_ONE_SCAT_SEND */
            {
/*
 * Call the post link layer routine, i.e either:
 * tfPppStuffPacket() for a PPP link layer, or
 * tfSlipStuffPacket() for a SLIP link layer, or
 * tfDeviceSendOnePacket() for any other link layer.
 * For each packet link for Ethernet, or for each send buffer for PPP, or
 * SLIP, then the post link layer routines call the device
 * driver send function directly, and the post link layer function will
 * return the device driver error, if any, only if returned when sending
 * the last packet buffer.
 */
                retCode = (*(devEntryPtr->devLinkLayerProtocolPtr->
                                                  lnkPostFuncPtr))(packetPtr);
            }
            if (retCode != TM_ENOERROR)
            {
/*
 * If device driver send failed on the last buffer (TM_USER_BUFFER_LAST),
 * then tfDeviceSendFailed() will remove the packet from the send queue,
 * and set needFreePacket.
 * In all cases, tfDeviceSendFailed() will upate the SNMP MIB variables
 * errors.
 */
                retCode = tfDeviceSendFailed( devEntryPtr, packetPtr, retCode,
                                              &needFreePacket );
            }
        }
        else
        {
/* No packet waiting to be transmitted */
            retCode = TM_ENOENT;
        }
    }
/* Unlock device driver. Check if we need to call tfSendCompleteInterface() */
    tfDeviceUnlockSendComplete(devEntryPtr);

#ifdef TM_USE_PPP
/*
 * If PPP wanted to close the device but was forced to wait for the transmit
 * task, close it now
 */
#ifdef TM_LCP_XMIT_TASK_TERM_ACK_FIX
    if (packetPtr->pktFlags2 & TM_PF2_PPP_CLOSE_DEVICE)
    {
        tfPppUserNotify(devEntryPtr,TM_LL_CLOSE_COMPLETE);
    }
#endif /* TM_LCP_XMIT_TASK_TERM_ACK_FIX */
#endif /* TM_USE_PPP */

/*
 * Free packet if tfDeviceSendFailed() was called and the packet was
 * the last packet
 */
    if (needFreePacket != TM_8BIT_ZERO)
    {
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    }
/* Return error code */
    return retCode;
}

/*
 * Check to see if data is waiting to be transmitted
 * When tfCheckXmitInterface returns 0, the user can then call
 * tfXmitInterface() so that the stack can transmit the next packet
 * to transmit in the send queue.
 * Parameters
 * interfaceHandle  Interface handle as returned by tfAddInterface()
 * Return
 * value          meaning
 * TM_ENOERROR    if there is data to be transmitted,
 * TM_EWOULDBLOCK if there is no data to be transmitted
 */
int tfCheckXmitInterface(ttUserInterface interfaceHandle)
{
    return tfCheckEventInterface(
                        &(((ttDeviceEntryPtr)interfaceHandle)->devXmitEvent));
}

#ifdef TM_XMIT_PEND_POST_NEEDED
/*
 * Wait for data to be transmitted, if there is none, then wait.
 * When tfWaitXmitInterface returns, the user can then call
 * tfXmitInterface() so that the stack can transmit the next packet to
 * transmit in the send queue.
 * VERY IMPORTANT:
 * Can only be used when there is a separate transmit task, and TM_TASK_XMIT
 * is defined in trsystem.h.
 */
int tfWaitXmitInterface ( ttUserInterface interfaceHandle )
{
    return tfWaitEventInterface(
                        &(((ttDeviceEntryPtr)interfaceHandle)->devXmitEvent));
}
#endif /* TM_XMIT_PEND_POST_NEEDED */
