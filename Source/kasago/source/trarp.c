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
 * Description: ARP Send and Incoming Routines
 *
 * Filename: trarp.c
 * Author: Paul
 * Date Created: 11/10/97
 * $Source: source/trarp.c $
 *
 * Modification History
 * $Revision: 6.0.2.6 $
 * $Date: 2012/03/31 06:05:01JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_IPV4

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/*
 * Local macros
 */

#ifdef TM_DSP
/* Copy from Ethernet address array to ARP header send Ethernet address */
#define tm_ether_to_arp_send(enetAddr, arpHeaderPtr)                         \
{                                                                            \
    arpHeaderPtr->arpSendPhyAddress0 = enetAddr[0];                          \
    arpHeaderPtr->arpSendPhyAddress1 = enetAddr[1];                          \
    arpHeaderPtr->arpSendPhyAddress2 = enetAddr[2];                          \
    arpHeaderPtr->arpSendPhyAddress3 = enetAddr[3];                          \
    arpHeaderPtr->arpSendPhyAddress4 = enetAddr[4];                          \
    arpHeaderPtr->arpSendPhyAddress5 = enetAddr[5];                          \
}
/* Copy from Ethernet address array to ARP header Target Ethernet address */
#define tm_ether_to_arp_target(enetAddr, arpHeaderPtr)                       \
{                                                                            \
    arpHeaderPtr->arpTargetPhyAddress0 = enetAddr[0];                        \
    arpHeaderPtr->arpTargetPhyAddress1 = enetAddr[1];                        \
    arpHeaderPtr->arpTargetPhyAddress2 = enetAddr[2];                        \
    arpHeaderPtr->arpTargetPhyAddress3 = enetAddr[3];                        \
    arpHeaderPtr->arpTargetPhyAddress4 = enetAddr[4];                        \
    arpHeaderPtr->arpTargetPhyAddress5 = enetAddr[5];                        \
}
/* Copy from ARP header Target to Ethernet address array */
#define tm_arp_target_to_ether(arpHeaderPtr, enetAddr)                       \
{                                                                            \
    enetAddr[0] = arpHeaderPtr->arpTargetPhyAddress0;                        \
    enetAddr[1] = arpHeaderPtr->arpTargetPhyAddress1;                        \
    enetAddr[2] = arpHeaderPtr->arpTargetPhyAddress2;                        \
    enetAddr[3] = arpHeaderPtr->arpTargetPhyAddress3;                        \
    enetAddr[4] = arpHeaderPtr->arpTargetPhyAddress4;                        \
    enetAddr[5] = arpHeaderPtr->arpTargetPhyAddress5;                        \
}
/* Copy from ARP header send Ethernet address to Ethernet address array */
#define tm_arp_send_to_ether(arpHeaderPtr, enetAddr)                         \
{                                                                            \
    enetAddr[0] = arpHeaderPtr->arpSendPhyAddress0;                          \
    enetAddr[1] = arpHeaderPtr->arpSendPhyAddress1;                          \
    enetAddr[2] = arpHeaderPtr->arpSendPhyAddress2;                          \
    enetAddr[3] = arpHeaderPtr->arpSendPhyAddress3;                          \
    enetAddr[4] = arpHeaderPtr->arpSendPhyAddress4;                          \
    enetAddr[5] = arpHeaderPtr->arpSendPhyAddress5;                          \
}
/* Copy from send Ethernet address to target Ethernet address in ARP header */
#define tm_ether_arp_send_to_target(arpHdrFromPtr, arpHdrToPtr)              \
{                                                                            \
    arpHdrToPtr->arpTargetPhyAddress0 = arpHdrFromPtr->arpSendPhyAddress0;   \
    arpHdrToPtr->arpTargetPhyAddress1 = arpHdrFromPtr->arpSendPhyAddress1;   \
    arpHdrToPtr->arpTargetPhyAddress2 = arpHdrFromPtr->arpSendPhyAddress2;   \
    arpHdrToPtr->arpTargetPhyAddress3 = arpHdrFromPtr->arpSendPhyAddress3;   \
    arpHdrToPtr->arpTargetPhyAddress4 = arpHdrFromPtr->arpSendPhyAddress4;   \
    arpHdrToPtr->arpTargetPhyAddress5 = arpHdrFromPtr->arpSendPhyAddress5;   \
}
/* Copy from IP address local variable to ARP header send Net field */
#define tm_ipaddr_to_arp_send(ipAddr, arpHeaderPtr)                          \
{                                                                            \
    arpHeaderPtr->arpSendNetAddress0 = ( (ipAddr) >> 24 ) & 0xff;            \
    arpHeaderPtr->arpSendNetAddress1 = ( (ipAddr) >> 16 ) & 0xff;            \
    arpHeaderPtr->arpSendNetAddress2 = ( (ipAddr) >> 8 )  & 0xff;            \
    arpHeaderPtr->arpSendNetAddress3 = ( ipAddr )         & 0xff;            \
}
/* Copy from IP address local variable to ARP header target Net field */
#define tm_ipaddr_to_arp_target(ipAddr, arpHeaderPtr)                        \
{                                                                            \
    arpHeaderPtr->arpTargetNetAddress0 = ( (ipAddr) >> 24 ) & 0xff;          \
    arpHeaderPtr->arpTargetNetAddress1 = ( (ipAddr) >> 16 ) & 0xff;          \
    arpHeaderPtr->arpTargetNetAddress2 = ( (ipAddr) >> 8 )  & 0xff;          \
    arpHeaderPtr->arpTargetNetAddress3 = ( ipAddr )         & 0xff;          \
}
/* Copy from ARP header send Net field to IP address local variable */
#define tm_arp_send_to_ipaddr(arpHeaderPtr, ipAddr)                          \
{                                                                            \
    ipAddr = TM_UL(0);                                                       \
    ipAddr |= ((tt32Bit) arpHeaderPtr->arpSendNetAddress0 & 0xff) << 24;     \
    ipAddr |= ((tt32Bit) arpHeaderPtr->arpSendNetAddress1 & 0xff) << 16;     \
    ipAddr |= ((tt32Bit) arpHeaderPtr->arpSendNetAddress2 & 0xff) << 8;      \
    ipAddr |= (tt32Bit) arpHeaderPtr->arpSendNetAddress3 & 0xff;             \
}
/* Copy from ARP header Target Net field to IP address local variable */
#define tm_arp_target_to_ipaddr(arpHeaderPtr, ipAddr)                        \
{                                                                            \
    ipAddr = TM_UL(0);                                                       \
    ipAddr |= ((tt32Bit) arpHeaderPtr->arpTargetNetAddress0 & 0xff) << 24;   \
    ipAddr |= ((tt32Bit) arpHeaderPtr->arpTargetNetAddress1 & 0xff) << 16;   \
    ipAddr |= ((tt32Bit) arpHeaderPtr->arpTargetNetAddress2 & 0xff) << 8;    \
    ipAddr |= (tt32Bit) arpHeaderPtr->arpTargetNetAddress3 & 0xff;           \
}
/* Copy from Arp header Send IP address to ARP header Target IP address */
#define tm_ip_arp_send_to_target(arpHdrFromPtr, arpHdrToPtr)                               \
{                                                                            \
    arpHdrToPtr->arpTargetNetAddress0 = arpHdrFromPtr->arpSendNetAddress0;   \
    arpHdrToPtr->arpTargetNetAddress1 = arpHdrFromPtr->arpSendNetAddress1;   \
    arpHdrToPtr->arpTargetNetAddress2 = arpHdrFromPtr->arpSendNetAddress2;   \
    arpHdrToPtr->arpTargetNetAddress3 = arpHdrFromPtr->arpSendNetAddress3;   \
}
#else /* !TM_DSP */
/* Copy IP addresses when they are short (but not long) aligned */
#define tm_ipaddr_copy(from, to)                                             \
{                                                                            \
    (to)[0] = (from)[0];                                                     \
    (to)[1] = (from)[1];                                                     \
    (to)[2] = (from)[2];                                                     \
    (to)[3] = (from)[3];                                                     \
}
/* Copy from Ethernet address array to ARP header send Ethernet address */
#define tm_ether_to_arp_send(enetAddr, arpHeaderPtr)                         \
        tm_ether_copy(enetAddr, arpHeaderPtr->arpSendPhyAddress)
/* Copy from Ethernet address array to ARP header Target Ethernet address */
#define tm_ether_to_arp_target(enetAddr, arpHeaderPtr)                       \
        tm_ether_copy(enetAddr, arpHeaderPtr->arpTargetPhyAddress)
/* Copy from ARP header Target to Ethernet address array */
#define tm_arp_target_to_ether(arpHeaderPtr, enetAddr)                       \
        tm_ether_copy(arpHeaderPtr->arpTargetPhyAddress, enetAddr)
/* Copy from ARP header send Ethernet address to Ethernet address array */
#define tm_arp_send_to_ether(arpHeaderPtr, enetAddr)                         \
        tm_ether_copy(arpHeaderPtr->arpSendPhyAddress, enetAddr)
/* Copy from send Ethernet address to target Ethernet address in ARP header */
#define tm_ether_arp_send_to_target(arpHdrFromPtr, arpHdrToPtr)              \
        tm_ether_copy( arpHdrFromPtr->arpSendPhyAddress,                     \
                       arpHdrToPtr->arpTargetPhyAddress)
/* Copy from IP address local variable to ARP header send Net field */
#define tm_ipaddr_to_arp_send(ipAddr, arpHeaderPtr)                          \
        tm_ipaddr_copy((tt8BitPtr)(&ipAddr), arpHeaderPtr->arpSendNetAddress)
/* Copy from IP address local variable to ARP header target Net field */
#define tm_ipaddr_to_arp_target(ipAddr, arpHeaderPtr)                        \
        tm_ipaddr_copy((tt8BitPtr)(&ipAddr),                                 \
                       arpHeaderPtr->arpTargetNetAddress)
/* Copy from ARP header send Net field to IP address local variable */
#define tm_arp_send_to_ipaddr(arpHeaderPtr, ipAddr)                          \
        tm_ipaddr_copy(arpHeaderPtr->arpSendNetAddress,                      \
                       (tt8BitPtr)(&ipAddr))
/* Copy from ARP header Target Net field to IP address local variable */
#define tm_arp_target_to_ipaddr(arpHeaderPtr, ipAddr)                        \
        tm_ipaddr_copy(arpHeaderPtr->arpTargetNetAddress,                    \
                       (tt8BitPtr)(&ipAddr))
/* Copy from Arp header Send IP address to ARP header Target IP address */
#define tm_ip_arp_send_to_target(arpHdrFromPtr, arpHdrToPtr)                 \
        tm_ipaddr_copy(arpHdrFromPtr->arpSendNetAddress,                     \
                       arpHdrToPtr->arpTargetNetAddress)
#endif /* !TM_DSP */

/*
 * Local functions
 */

static int tfArpSendReply(ttDeviceEntryPtr devPtr, ttPacketPtr packetPtr);
static void tfArpTimeOut (
    ttVoidPtr        timerBlockPtr,
    ttGenericUnion   userParm1,
    ttGenericUnion   userParm2);
static void tfArpResEntryRemove(
    ttDeviceEntryPtr devPtr,
    ttResEntryPtr    resEntryPtr,
    ttPacketPtr      packetPtr);
static ttResEntryPtr tfArpResolveSearch(
    ttDeviceEntryPtr devPtr,
    tt4IpAddress     ipAddress);
static void tfArpCleanUpResEntry(
    ttVoidPtr        timerBlockPtr,
    tt8Bit           flags);

/*
 * Resolve an IP address to an ethernet address
 */
int tfArpResolve (ttPacketPtr packetPtr)
{

    ttResEntryPtr    resEntryPtr;
    ttDeviceEntryPtr devPtr;
    ttSharedDataPtr  pktShrDataPtr;
    ttPacketPtr      oldPacketPtr;
    ttPacketPtr      tempPacketPtr;
    ttSocketEntryPtr socketEntryPtr;
#ifndef TM_SINGLE_INTERFACE_HOME
    int              isMatch;
#endif /* TM_SINGLE_INTERFACE_HOME */
    int              errorCode;
    tt8Bit           arpRequestSendCalled;
    tt8Bit           needFreePacket;
    tt16Bit          mhomeIndex;
    ttGenericUnion   userParm;

    errorCode = TM_ENOERROR;
/* Assume we are not calling the ARP request send function */
    arpRequestSendCalled = TM_8BIT_ZERO;
/* Assume we do need to free a packet */
    needFreePacket = TM_8BIT_YES;
    devPtr = packetPtr->pktDeviceEntryPtr;
    pktShrDataPtr = packetPtr->pktSharedDataPtr;
/*
 * Lock the device. It will be unlocked by tfArpSendRequest, or at
 * the bottom of this routine, if we do not call the send ARP request
 * function
 */
    tm_call_lock_wait(&(devPtr->devLockEntry));
    if (pktShrDataPtr->dataRtePtr->rteFlags & TM_RTE_REJECT)
    {
/* Arp entry is invalid (we timed out on previous ARP requests) */
        errorCode = TM_EHOSTDOWN;
        goto arpResolveFinish;
    }
/* Look for a matching destination IP address in the arp resolve list */
    resEntryPtr = tfArpResolveSearch(
        devPtr,
#ifdef TM_USE_IPV6
        pktShrDataPtr->dataSpecDestIpAddress.s6LAddr[3]
#else /* ! TM_USE_IPV6 */
        pktShrDataPtr->dataSpecDestIpAddress
#endif /* ! TM_USE_IPV6 */
        );
    if (resEntryPtr != TM_RES_ENTRY_NULL_PTR)
    {
/*
 * Already waiting for a resolution on this device:
 * 1. If new packet is a socket packet:
 *    If the socket is not a TCP socket, try and queue the packet at the
 *    end of the current queue because the queue will be limited by the size of
 *    the socket queue. (We do not queue TCP socket packets as they will be
 *    re-transmitted.)
 * 2. else (new packet is not a socket packet)
 *  2.1 If there is a socket packet in the queue, do not queue the new one, and dump it.
 *  2.2 else dump the packets in the queue, and insert the new one. (old behavior)
 */
/* Packet at head of the queue */
        oldPacketPtr = resEntryPtr->resPacketPtr;

/* socket entry pointer might not be stored in the first packet in the
 * chain */
        socketEntryPtr = pktShrDataPtr->dataSocketEntryPtr;
        tempPacketPtr = packetPtr;
        while ((socketEntryPtr == (ttSocketEntryPtr)0) &&
               (tempPacketPtr->pktChainNextPtr != (ttPacketPtr)0))
        {
            tempPacketPtr = tempPacketPtr->pktChainNextPtr;
            socketEntryPtr =
                tempPacketPtr->pktSharedDataPtr->dataSocketEntryPtr;
        }

        if (socketEntryPtr != (ttSocketEntryPtr)0)
/* new packet sent through a socket */
        {
            if (((int)socketEntryPtr->socProtocolNumber) != IPPROTO_TCP)
            {
                while (oldPacketPtr->pktChainNextPtr != TM_PACKET_NULL_PTR)
                {
                    oldPacketPtr = oldPacketPtr->pktChainNextPtr;
                }
/* Queue packet at the end */
                oldPacketPtr->pktChainNextPtr = packetPtr;
/*
 * Do not free the packet, but we want to unget the ARP entry, because its
 * owner count has already been increased with the first packet in the queue.
 */
                needFreePacket = TM_8BIT_ZERO;
            }
        }
        else
/* new packet is not a socket packet */
        {
            while (oldPacketPtr != TM_PACKET_NULL_PTR)
            {
/* socket entry pointer might not be stored in the first packet in the
 * chain */
                socketEntryPtr =
                        oldPacketPtr->pktSharedDataPtr->dataSocketEntryPtr;
                tempPacketPtr = oldPacketPtr;
                while ((socketEntryPtr == (ttSocketEntryPtr)0) &&
                       (tempPacketPtr->pktLinkNextPtr != (ttPacketPtr)0))
                {
                    tempPacketPtr = tempPacketPtr->pktLinkNextPtr;
                    socketEntryPtr =
                        tempPacketPtr->pktSharedDataPtr->dataSocketEntryPtr;
                }

                if (socketEntryPtr != (ttSocketEntryPtr)0)
                {
/* At least one packet in the queue is a socket packet */
                    break;
                }
                oldPacketPtr = oldPacketPtr->pktChainNextPtr;
            }
            if (oldPacketPtr == (ttPacketPtr)0)
/* Replace with the new one */
            {
                oldPacketPtr = resEntryPtr->resPacketPtr;
/* replace with the new one */
                resEntryPtr->resPacketPtr = packetPtr;
/* Dump all queued packets (at the end of the function) */
                packetPtr = oldPacketPtr;
                pktShrDataPtr = packetPtr->pktSharedDataPtr;
            }
/* else one packet in the queue is a socket packet, so dump this new packet.  */
        }
    }
    else
    {
/* No matching entry so add this one */
        resEntryPtr = (ttResEntryPtr)tm_get_raw_buffer(
                                                (unsigned)sizeof(ttResEntry));
/*
 * Note: no need to bzero allocated entry, because all fields are initialized
 * below
 */
        if (resEntryPtr == TM_RES_ENTRY_NULL_PTR)
        {
/*
 * Not enough memroy to resolve the ARP entry. Make sure the ARP entry
 * will be removed by the aging ARP timer.
 */
            pktShrDataPtr->dataRtePtr->rteTtl = TM_UL(0);
            errorCode = TM_ENOBUFS;
            goto arpResolveFinish;
        }
        tm_bzero(resEntryPtr, sizeof(ttResEntry));
        resEntryPtr->resPacketPtr = packetPtr;
        resEntryPtr->resNextPtr = devPtr->devResEntryPtr;
        if (devPtr->devResEntryPtr != TM_RES_ENTRY_NULL_PTR)
        {
            devPtr->devResEntryPtr->resPrevPtr = resEntryPtr;
        }
        devPtr->devResEntryPtr = resEntryPtr;
        resEntryPtr->resRequestTries = tm_context(tvArpRequestTries);
        resEntryPtr->resDeviceEntryPtr = devPtr;
        userParm.genVoidParmPtr = (ttVoidPtr) resEntryPtr;
/* one for tfArpResEntryRemove + one for tfArpCleanUpResEntry() */
        resEntryPtr->resOwnerCount = 2;
        resEntryPtr->resTimerPtr = tfTimerAddExt(
                                            &resEntryPtr->resTimeOutTimer,
                                            tfArpTimeOut,
                                            tfArpCleanUpResEntry,
                                            userParm,
                                            userParm, /* unused */
                                            tm_context(tvArpRetryTime),
                                            TM_TIM_AUTO );

/*
 * Get the correct sender address to use in the ARP request by finding a
 * multihome index on this device with an address that is on the same subnet
 * as the target address.  This may be the same as pktMhomeIndex, but is not
 * guaranteed.  If no address is found or only using a single
 * interface/multihome, default to the value specified in pktMhomeIndex.
 */
#ifndef TM_SINGLE_INTERFACE_HOME
        isMatch = tfMhomeNetMatch(
                            devPtr,
#ifdef TM_USE_IPV6
                  pktShrDataPtr->dataSpecDestIpAddress.s6LAddr[3],
#else /* ! TM_USE_IPV6 */
                  pktShrDataPtr->dataSpecDestIpAddress,
#endif /* ! TM_USE_IPV6 */
                  &mhomeIndex);
        if (isMatch != TM_ENOERROR)
#endif /* TM_SINGLE_INTERFACE_HOME */
        {
            mhomeIndex = packetPtr->pktMhomeIndex;
        }

/* Device unlocked in tfArpSendRequest */
        errorCode = tfArpSendRequest(
            (ttVoidPtr)devPtr,
            mhomeIndex,
#ifdef TM_USE_IPV6
            pktShrDataPtr->dataSpecDestIpAddress.s6LAddr[3],
#else /* ! TM_USE_IPV6 */
            pktShrDataPtr->dataSpecDestIpAddress,
#endif /* ! TM_USE_IPV6 */
            pktShrDataPtr->dataRtePtr
            );
/*
 * Do not unget the ARP entry, and do not free the packet. Also device is unlocked
 * in tfArpSendPacket
 */
        arpRequestSendCalled = TM_8BIT_YES;
/*
 * Not necessary, because if arpRequestSendCalled is set, then check below is
 * not reached.
 */
        needFreePacket = TM_8BIT_ZERO;
    }
arpResolveFinish:
    if (arpRequestSendCalled == TM_8BIT_ZERO)
    {
/* Did not call tfArpSendRequest */
/* Unlock the device */
        tm_call_unlock(&devPtr->devLockEntry);
/* Release ownership of ARP entry */
        tfRtUnGet(pktShrDataPtr->dataRtePtr);
        if (needFreePacket != TM_8BIT_ZERO)
        {
/* Free original IP packet */
            tfFreeChainPacket(packetPtr, TM_SOCKET_UNLOCKED);
        }
    }
    return errorCode;
}

/*
 * Search for a matching destination IP address that is waiting for
 * resolution on this device (DEVICE MUST BE LOCKED BEFORE CALLED)
 */
static ttResEntryPtr tfArpResolveSearch (
    ttDeviceEntryPtr devPtr,
    tt4IpAddress ipAddress)
{
    ttResEntryPtr resEntryPtr;

    resEntryPtr = devPtr->devResEntryPtr;
    while (resEntryPtr != TM_RES_ENTRY_NULL_PTR)
    {
        if (
#ifdef TM_USE_IPV6
            tm_ip_match(
                ipAddress,
                resEntryPtr->resPacketPtr->pktSharedDataPtr->
                dataSpecDestIpAddress.s6LAddr[3] )
#else /* ! TM_USE_IPV6 */
            tm_ip_match(
                ipAddress,
                resEntryPtr->resPacketPtr->pktSharedDataPtr->
                dataSpecDestIpAddress )
#endif /* ! TM_USE_IPV6 */
            )
        {
            break;
        }
        resEntryPtr = resEntryPtr->resNextPtr;
    }
    return (resEntryPtr);
}

/*
 * An ARP request has timed out
 */
static void tfArpTimeOut (
    ttVoidPtr      timerBlockPtr,
    ttGenericUnion userParm1,
    ttGenericUnion userParm2)
{
    ttDeviceEntryPtr devPtr;
    ttPacketPtr      packetPtr;
    ttSharedDataPtr  pktShrDataPtr;
    ttResEntryPtr    resEntryPtr;
#ifndef TM_SINGLE_INTERFACE_HOME
    int              isMatch;
#endif /* TM_SINGLE_INTERFACE_HOME */
    tt16Bit          mhomeIndex;
#ifdef TM_LOCK_NEEDED
    tt8Bit           needUnlock;
#endif /* TM_LOCK_NEEDED */

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);

    resEntryPtr = (ttResEntryPtr) userParm1.genVoidParmPtr;

    devPtr = resEntryPtr->resDeviceEntryPtr;
#ifdef TM_LOCK_NEEDED
/* Lock the device */
    tm_call_lock_wait(&(devPtr->devLockEntry));
    needUnlock = TM_8BIT_YES;
#endif /* TM_LOCK_NEEDED */
    if (tm_timer_not_reinit((ttTimerPtr)timerBlockPtr))
    {
/* Save the packetPtr */
        packetPtr = resEntryPtr->resPacketPtr;
        pktShrDataPtr = packetPtr->pktSharedDataPtr;
        if (resEntryPtr->resRequestTries == 0)
        {
/*
 * Resume this route (the ARP quiet time expired)
 */
/* Make sure the ARP entry will be removed by the aging ARP timer */
            pktShrDataPtr->dataRtePtr->rteTtl = TM_UL(0);
/* Delete the timer */
            tm_timer_remove((ttTimerPtr)timerBlockPtr);
            resEntryPtr->resTimerPtr = TM_TMR_NULL_PTR;
/* Now remove it from the list (device UNLOCKED in tfArpResEntryRemove)*/
            tfArpResEntryRemove(devPtr,
                                resEntryPtr,
                                packetPtr);
            tfFreeChainPacket(packetPtr, TM_SOCKET_UNLOCKED);
#ifdef TM_LOCK_NEEDED
            needUnlock = TM_8BIT_NO;
#endif /* TM_LOCK_NEEDED */
        }
        else
        {
/* Timeout timer */
            resEntryPtr->resRequestTries--;
            if( resEntryPtr->resRequestTries == 0 )
            {
/* We really timed out so mark the route entry as down */
                pktShrDataPtr->dataRtePtr->rteFlags |= TM_RTE_REJECT;
/* Quiet time timer */
                tm_timer_new_time((ttTimerPtr)timerBlockPtr,
                                  tm_context(tvArpQuietTime));
            }
            else
            {

/*
 * Get the correct sender address to use in the ARP request by finding a
 * multihome index on this device with an address that is on the same subnet
 * as the target address.  This may be the same as pktMhomeIndex, but is not
 * guaranteed.  If no address is found or only using a single
 * interface/multihome, default to the value specified in pktMhomeIndex.
 */
#ifndef TM_SINGLE_INTERFACE_HOME
            isMatch = tfMhomeNetMatch(
                          devPtr,
#ifdef TM_USE_IPV6
                          pktShrDataPtr->dataSpecDestIpAddress.s6LAddr[3],
#else /* ! TM_USE_IPV6 */
                          pktShrDataPtr->dataSpecDestIpAddress,
#endif /* ! TM_USE_IPV6 */
                          &mhomeIndex);
                if (isMatch != TM_ENOERROR)
#endif /* TM_SINGLE_INTERFACE_HOME */
                {
                    mhomeIndex = packetPtr->pktMhomeIndex;
                }

/* Device unlocked in tfArpSendRequest */
                (void)tfArpSendRequest(
                    (ttVoidPtr)devPtr,
                    mhomeIndex,
#ifdef TM_USE_IPV6
                    pktShrDataPtr->dataSpecDestIpAddress.s6LAddr[3],
#else /* ! TM_USE_IPV6 */
                    pktShrDataPtr->dataSpecDestIpAddress,
#endif /* ! TM_USE_IPV6 */
                    pktShrDataPtr->dataRtePtr
                    );
#ifdef TM_LOCK_NEEDED
                needUnlock = TM_8BIT_NO;
#endif /* TM_LOCK_NEEDED */
            }
        }
    }
#ifdef TM_LOCK_NEEDED
    if (needUnlock == TM_8BIT_YES)
    {
        tm_call_unlock(&(devPtr->devLockEntry));
    }
#endif /* TM_LOCK_NEEDED */
}

/*
 * Timer clean up function. called by the timer when timer is freed in the
 * Timer execute loop.
 */
static void tfArpCleanUpResEntry (ttVoidPtr timerBlockPtr, tt8Bit flags)
{
    ttResEntryPtr    resEntryPtr;
    ttDeviceEntryPtr devPtr;

    resEntryPtr = (ttResEntryPtr)
            (tm_tmr_arg1((ttTimerPtr)timerBlockPtr).genVoidParmPtr);
    devPtr = resEntryPtr->resDeviceEntryPtr;
    if (!(flags & TM_TIMER_CLEANUP_LOCKED))
    {
/* Lock the device */
    if (devPtr != (ttDeviceEntryPtr)0)
    {
        tm_call_lock_wait(devPtr->devLockEntry);
    }
    }
    resEntryPtr->resOwnerCount--;
    if (resEntryPtr->resOwnerCount == 0)
    {
        tm_free_raw_buffer(resEntryPtr);
    }
    if (!(flags & TM_TIMER_CLEANUP_LOCKED))
    {
/* Unlock the device */
        if (devPtr != (ttDeviceEntryPtr)0)
        {
            tm_call_unlock(devPtr->devLockEntry);
        }
    }
}

/*
 * Free all matching packets waiting for ARP resolution on the device.
 * Called with device lock on. Return with device lock on.
 */
void tfArpResolveClean (ttUserInterface   interfaceHandle,
                        tt4IpAddress      intfIpAddr,
                        tt4IpAddress      intfNetMask)
{
    ttResEntryPtr    resEntryPtr;
    ttPacketPtr      packetPtr;
    ttRteEntryPtr    rtePtr;
    ttDeviceEntryPtr devPtr;
    ttResEntryPtr    nextResEntryPtr;

    devPtr = (ttDeviceEntryPtr)interfaceHandle;
    resEntryPtr = devPtr->devResEntryPtr;
    while (resEntryPtr != (ttResEntryPtr)0)
    {
        nextResEntryPtr = resEntryPtr->resNextPtr;
        {
/* Save the packetPtr */
            packetPtr = resEntryPtr->resPacketPtr;
            rtePtr = packetPtr->pktSharedDataPtr->dataRtePtr;
#ifdef TM_USE_IPV6
            if (    (tm_6_addr_is_ipv4_mapped(&rtePtr->rteDest))
                 && (tm_ip_net_match(
                        tm_4_ip_addr(rtePtr->rteDest),
                        intfIpAddr,
                        intfNetMask)) )
#else /* !TM_USE_IPV6 */
            if (tm_ip_net_match(rtePtr->rteDest, intfIpAddr, intfNetMask))
#endif /* !TM_USE_IPV6 */
            {
/* Make sure the ARP entry will be removed by the aging ARP timer */
                rtePtr->rteTtl = TM_UL(0);
                tfArpResEntryRemove(devPtr, resEntryPtr, packetPtr);
                tfFreeChainPacket(packetPtr, TM_SOCKET_UNLOCKED);
/* Device unlocked in tfArpResEntryRemove() */
                tm_call_lock_wait(&(devPtr->devLockEntry));
/* Because device was unlocked, start from the beginnning of the quueue */
                nextResEntryPtr = devPtr->devResEntryPtr;
            }
        }
        resEntryPtr = nextResEntryPtr;
    }
}

/*
 * Remove a resolve entry from the list
 */
static void tfArpResEntryRemove (
    ttDeviceEntryPtr devPtr,
    ttResEntryPtr    resEntryPtr,
    ttPacketPtr      packetPtr)
{
    ttTimerPtr  timerPtr;
/*
 * Since this entry has resolved or the quiet time has expired,
 * (un)reject the route entry
 */
    tm_16bit_clr_bit( packetPtr->pktSharedDataPtr->dataRtePtr->rteFlags,
                      TM_RTE_REJECT );
/* Are we the first entry in the list */
    if (resEntryPtr->resPrevPtr == TM_RES_ENTRY_NULL_PTR)
    {
/* If so then remove us and make our next ptr the root (Could be null) */
        devPtr->devResEntryPtr = resEntryPtr->resNextPtr;
/* We are the first, so check to see if there is another entry */
        if (resEntryPtr->resNextPtr != TM_RES_ENTRY_NULL_PTR)
        {
/* There is so null out it's previous ptr */
            resEntryPtr->resNextPtr->resPrevPtr = TM_RES_ENTRY_NULL_PTR;
        }
    }
    else
    {
/* Make our prev point to our next */
        resEntryPtr->resPrevPtr->resNextPtr = resEntryPtr->resNextPtr;
/* Check to see if we have a next */
        if (resEntryPtr->resNextPtr != TM_RES_ENTRY_NULL_PTR)
        {
/* we do so make him point to our previous */
            resEntryPtr->resNextPtr->resPrevPtr = resEntryPtr->resPrevPtr;
        }
    }
    timerPtr = resEntryPtr->resTimerPtr;
/* Stop and remove the timer if it exists */
    if (timerPtr != (ttTimerPtr)0)
    {
/* Don't allow the tfArpTimeOut/tfArpResume timer to do anything */
        tm_timer_remove(timerPtr);
        resEntryPtr->resTimerPtr = (ttTimerPtr)0;
    }
    resEntryPtr->resOwnerCount--; /* protected with the device lock */
    if (resEntryPtr->resOwnerCount == 0)
/* No timer */
    {
/* Free the resolution entry */
        tm_free_raw_buffer(resEntryPtr);
    }
    tm_call_unlock(&(devPtr->devLockEntry));
/* release ownership of ARP entry */
    tfRtUnGet(packetPtr->pktSharedDataPtr->dataRtePtr);
}

/*
 * Generate and Send an ARP Request
 */
int tfArpSendRequest ( ttVoidPtr     interfaceHandle,
                       tt16Bit       mhomeIndex,
                       tt4IpAddress  ipAddress,
                       ttRteEntryPtr rteEntryPtr)
{
    ttArpHeaderPtr          arpHeaderPtr;
    ttPacketPtr             packetPtr;
    ttSharedDataPtr         pktShrDataPtr;
    ttLinkLayerEntryPtr     linkLayerPtr;
    ttDeviceEntryPtr        devPtr;
    int                     retCode;

#ifdef TM_SINGLE_INTERFACE_HOME
    TM_UNREF_IN_ARG(mhomeIndex); /* unused parameter */
#endif /* TM_SINGLE_INTERFACE_HOME */
    devPtr = (ttDeviceEntryPtr)interfaceHandle;

#ifdef TM_MOBILE_IP4
/*
 * While a mobile IPv4 node is on a foreign network it may not send broadcast
 * ARP requests.
 */
    if ( tm_mobile_ip4_enb(devPtr) )
    {
/*
 * If the node is bound or in the process of binding to a FA,
 * unlock the device and return, since we can't send ARP requests
 * on a foreign network.
 */
        if ( (devPtr->devMobileIp4VectPtr->mipState > 2) &&
             !(devPtr->devMobileIp4VectPtr->mipStatusFlags &
                                                      TM_MIP_ST_COLOCATE_ADDR))
        {
            retCode = TM_ENOERROR;
            tm_call_unlock(&(devPtr->devLockEntry));
            goto arpRequestEnd;
        }
    }
#endif /* TM_MOBILE_IP4 */


/* Get a shared buffer of minimum Ethernet length */
    packetPtr = tfGetSharedBuffer( TM_LL_MAX_HDR_LEN,
                                   (ttPktLen)TM_ETHER_IP_MIN_PACKET,
                                   TM_16BIT_ZERO);
    if (packetPtr == TM_PACKET_NULL_PTR)
    {
        tm_call_unlock(&(devPtr->devLockEntry));
        retCode = TM_ENOBUFS;
        goto arpRequestEnd;
    }
    tm_bzero(packetPtr->pktLinkDataPtr,
             tm_packed_byte_count(TM_ETHER_IP_MIN_PACKET));
    pktShrDataPtr = packetPtr->pktSharedDataPtr;
/* Save off the routing entry for the route we are trying to resolve */
    packetPtr->pktSharedDataPtr->dataRtePtr=rteEntryPtr;
    arpHeaderPtr = (ttArpHeaderPtr)(packetPtr->pktLinkDataPtr);
/* Fill in the fields */
    arpHeaderPtr->arpHardware = TM_ARP_HARDWARE_ETHERNET;
    arpHeaderPtr->arpProtocol = TM_ETHER_IP_TYPE;
    arpHeaderPtr->arpHardwareLength = TM_ETHER_ADDRESS_LENGTH;
    arpHeaderPtr->arpProtocolLength = TM_IP_ADDRESS_LENGTH;
    arpHeaderPtr->arpOperation = TM_ARP_REQUEST;

/* The target MAC address in the ARP request is already all 0's */

    tm_ether_to_arp_send(devPtr->devPhysAddrArr, arpHeaderPtr);

/* Our destination is a broadcast for requests */
    linkLayerPtr = devPtr->devLinkLayerProtocolPtr;
    tm_ether_copy((linkLayerPtr->lnkPhysBroadAddr),
                   (pktShrDataPtr->dataEthernetAddress));

    if (mhomeIndex != (tt16Bit) TM_MAX_IPS_PER_IF)
/* Traditional ARP request */
    {
/* We copy this to prevent misaligned long word accesses */
        tm_ipaddr_to_arp_send( tm_ip_dev_addr(devPtr, mhomeIndex),
                               arpHeaderPtr );
    }
/* else  ARP Probe, the sender net address is zero */

/* This is already long word aligned, so use 32bit copy */
    tm_ipaddr_to_arp_target(ipAddress, arpHeaderPtr);

    packetPtr->pktDeviceEntryPtr = devPtr;
/* TM_LL_BROADCAST_FLAG for SNMP MIB counter */
    packetPtr->pktFlags |= TM_LL_BROADCAST_FLAG;
    packetPtr->pktNetworkLayer = TM_NETWORK_ARP_LAYER;
    tm_call_unlock(&(devPtr->devLockEntry));
#ifdef TM_BYPASS_ETHER_LL
    retCode = (*(linkLayerPtr->lnkSendFuncPtr))(packetPtr);
    if (retCode != TM_ENOERROR)
    {
        goto arpRequestEnd;
    }
#endif /* TM_BYPASS_ETHER_LL */
#ifndef TM_SINGLE_INTERFACE_HOME
    packetPtr->pktMhomeIndex = mhomeIndex;
#endif /* ! TM_SINGLE_INTERFACE_HOME */
    retCode = tfDeviceSend(packetPtr);
arpRequestEnd:
    return(retCode);
}

/*
 * Send an ARP REPLY Packet, reuse the ARP REQUEST
 */
static int tfArpSendReply (ttDeviceEntryPtr devPtr, ttPacketPtr packetPtr)
{
    ttArpHeaderPtr      arpHeaderPtr;
    ttArpHeaderPtr      arpHdrFromPtr;
    ttPacketPtr         fromPacketPtr;
    ttPktLen            availHdrLen;
    int                 retCode;
    tt4IpAddressUnion   tempIpAddress;

    availHdrLen = (ttPktLen)
                            (   packetPtr->pktLinkDataPtr
                              - packetPtr->pktSharedDataPtr->dataBufFirstPtr);
    arpHdrFromPtr = (ttArpHeaderPtr)(packetPtr->pktLinkDataPtr);
    fromPacketPtr = packetPtr;
    if (availHdrLen < (ttPktLen)(devPtr->devLinkHeaderLength))
    {
/*
 * Not enough room ahead for the link layer header
 * Get a shared buffer of minimum Ethernet length
 */
        packetPtr = tfGetSharedBuffer(TM_LL_MAX_HDR_LEN,
                                      (ttPktLen)TM_ETHER_IP_MIN_PACKET,
                                      TM_16BIT_ZERO);
        if (packetPtr == TM_PACKET_NULL_PTR)
        {
            retCode = TM_ENOBUFS;
            tfFreePacket(fromPacketPtr, TM_SOCKET_UNLOCKED);
            goto arpReplyEnd;
        }
        tm_bzero(packetPtr->pktLinkDataPtr,
                 tm_packed_byte_count(TM_ETHER_IP_MIN_PACKET));
        packetPtr->pktDeviceEntryPtr = devPtr;
        packetPtr->pktNetworkLayer = TM_NETWORK_ARP_LAYER;
        packetPtr->pktMhomeIndex = fromPacketPtr->pktMhomeIndex;
        arpHeaderPtr = (ttArpHeaderPtr)(packetPtr->pktLinkDataPtr);
/* Fill in the fields that do not get changed below */
        arpHeaderPtr->arpHardware = TM_ARP_HARDWARE_ETHERNET;
        arpHeaderPtr->arpProtocol = TM_ETHER_IP_TYPE;
        arpHeaderPtr->arpHardwareLength = TM_ETHER_ADDRESS_LENGTH;
        arpHeaderPtr->arpProtocolLength = TM_IP_ADDRESS_LENGTH;
    }
    else
    {
#ifdef TM_USE_DRV_SCAT_RECV
/* Remove excess bytes from the end of the original ARP request */
        if (packetPtr->pktLinkExtraCount != 0)
        {
            tfFreePacket(packetPtr->pktLinkNextPtr, TM_SOCKET_UNLOCKED);
            packetPtr->pktLinkNextPtr = (ttPacketPtr)0;
            packetPtr->pktLinkExtraCount = 0;
        }
#endif /* TM_USE_DRV_SCAT_RECV */
#ifdef TM_DEV_RECV_OFFLOAD
/* If recv checksum was offloaded, reset offload pointer */
        packetPtr->pktDevOffloadPtr = (ttVoidPtr)0;
#endif /* TM_DEV_RECV_OFFLOAD */
        arpHeaderPtr = (ttArpHeaderPtr)(packetPtr->pktLinkDataPtr);
    }
    arpHeaderPtr->arpOperation = TM_ARP_REPLY;
/* Store correct packet length */
    packetPtr->pktLinkDataLength = (ttPktLen)TM_ARP_HEADER_SIZE;
    packetPtr->pktChainDataLength = (ttPktLen)TM_ARP_HEADER_SIZE;
/*
 * Make receiver the sender and plug in our physical address as sender:
 */
/* 1. Copy arpSendPhyAddress to arpTargetPhyAddress in ARP header */
    tm_ether_arp_send_to_target(arpHdrFromPtr, arpHeaderPtr);
/* 2. Now plug in our physical address in arpSendPhyAddress */
    tm_ether_to_arp_send(devPtr->devPhysAddrArr, arpHeaderPtr);
/* 3. Swap target and send IP addresses in ARP header */
/* 3.1 Copy from ARP header Target IP address to local IP address */
    tm_arp_target_to_ipaddr(arpHdrFromPtr, tempIpAddress.addrLong);
/* 3.2 Copy from Arp header Send IP address to ARP header Target IP address */
    tm_ip_arp_send_to_target(arpHdrFromPtr, arpHeaderPtr);
/* 3.3 Copy from local Ip address to ARP header send IP address */
    tm_ipaddr_to_arp_send(tempIpAddress.addrLong, arpHeaderPtr);
#ifdef TM_USE_AUTO_IP
/*
 * After succesfully configuring an AUTO IP address, all subsequent
 * ARP replies should be broadcast.
 */
    if (tm_ip_is_local(tempIpAddress.addrLong))
    {
#ifdef TM_SNMP_MIB
        packetPtr->pktFlags |= TM_LL_BROADCAST_FLAG;
#endif /* TM_SNMP_MIB */
/* Send an ARP reply as a broadcast if source address is IPV4 local */
        tm_ether_copy( devPtr->devLinkLayerProtocolPtr->lnkPhysBroadAddr,
                       packetPtr->pktEthernetAddress );
    }
    else
#endif /* TM_USE_AUTO_IP */
    {
        tm_arp_target_to_ether(arpHeaderPtr, packetPtr->pktEthernetAddress);
    }
/* End of swapping ARP information between request and reply */
    if (fromPacketPtr != packetPtr)
/* We sent the ARP reply in a new packet.
 * Need to free the old one after having copied the ARP request information.
 */
    {
        tfFreePacket(fromPacketPtr, TM_SOCKET_UNLOCKED);
    }
#ifdef TM_BYPASS_ETHER_LL
    retCode = (*(devPtr->devLinkLayerProtocolPtr->lnkSendFuncPtr))(
                                                                   packetPtr);
    if (retCode != TM_ENOERROR)
    {
/* We failed to add the link layer */
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
        goto arpReplyEnd;
    }
#endif /* TM_BYPASS_ETHER_LL */
#ifdef TM_SNMP_MIB
    tm_16bit_clr_bit( packetPtr->pktFlags,
                      (TM_LL_BROADCAST_FLAG | TM_LL_MULTICAST_FLAG));
#endif /* TM_SNMP_MIB */

    retCode = tfDeviceSend(packetPtr);
arpReplyEnd:
    return(retCode);
}

/*
 * Process an incoming ARP request
 */
int tfArpIncomingPacket (ttPacketPtr packetPtr)
{
    ttArpHeaderPtr     arpHeaderPtr;
    ttDeviceEntryPtr   devPtr;
    ttResEntryPtr      resEntryPtr;
    ttProxyEntryPtr    proxyEntryPtr;
    tt8BitPtr          etherPtr;
    ttPacketPtr        resolvePacketPtr;
    ttPacketPtr        nxtResPacketPtr;
    tt4IpAddress       senderNetAddress;
    tt4IpAddress       targetNetAddress;
    ttArpMapping       arpMapping;
    int                retCode;
    int                errorCode;
    tt16Bit            arpOperation;
    ttEnetAddressUnion enetAddress;
    tt32Bit            ttl;
    tt8Bit             sendArpReply;
    tt8Bit             addArpEntry;
    tt8Bit             flag8023;
#ifndef TM_SINGLE_INTERFACE_HOME
    tt16Bit            anyMhomeIndex;
#endif /* TM_SINGLE_INTERFACE_HOME */
#ifdef TM_USE_IPV6
    int                match;
#endif /* TM_USE_IPV6 */

    senderNetAddress = TM_IP_ZERO;
    retCode = TM_ENOERROR;
#ifdef TM_USE_DRV_SCAT_RECV
/* If ARP header is not contiguous, make it contiguous */
    if (tm_pkt_hdr_is_not_cont(packetPtr, TM_ARP_HEADER_SIZE))
    {
        packetPtr = tfContiguousSharedBuffer(packetPtr, TM_ARP_HEADER_SIZE);
        if (packetPtr == (ttPacketPtr)0)
        {
            goto arpIncomingExit;
        }
    }
#endif /* TM_USE_DRV_SCAT_RECV */
#ifdef TM_USE_ETHER_8023_LL
/* Set the Ether Type in the routing entry */
    if (packetPtr->pktSharedDataPtr->dataFlags & TM_BUF_LL_E8023)
    {
        flag8023=TM_RT_FRAME_8023;
    }
    else
    {
        flag8023=0;
    }
#else /* TM_USE_ETHER_8023_LL */
    flag8023=0;
#endif /* TM_USE_ETHER_8023_LL */

    packetPtr->pktNetworkLayer = TM_NETWORK_ARP_LAYER;
    arpHeaderPtr = (ttArpHeaderPtr)(packetPtr->pktLinkDataPtr);
    devPtr = packetPtr->pktDeviceEntryPtr;
    sendArpReply = TM_8BIT_ZERO;
    arpOperation = arpHeaderPtr->arpOperation;
/*
 * BUG ID 206: We should check the hardware type when receiving ARP packet.
 * If the hardware type is not correct, drope the ARP packet.
 * Currently, only ethernet(0x0001) type is supported.
 */
    if (   (   (arpHeaderPtr->arpHardware == TM_ARP_HARDWARE_ETHERNET)
            || (arpHeaderPtr->arpHardware == TM_ARP_HARDWARE_802) )
        && (arpHeaderPtr->arpHardwareLength == TM_ETHER_ADDRESS_LENGTH)
        && (   (arpOperation == TM_ARP_REQUEST)
            || (arpOperation == TM_ARP_REPLY) )  )
    {
/*
 * Save ARP header send net address field, arpSendNetAddress
 * to local variable, before calling tfArpSendReply(), as arpSendNetAddress
 * will be swapped with arpTargetNetAddress in that routine.
 */
        tm_arp_send_to_ipaddr(arpHeaderPtr, senderNetAddress);
/*
 * Save arpSendPhyAddress before we could call tfArpSendReply, as it will be
 * changed to our own Ethernet address in tfArpSendReply.
 */
        tm_arp_send_to_ether(arpHeaderPtr, enetAddress.enuAddress);

        if (tm_context(tvArpChkActiveEntries) != 0)
/* Collision Detection is active */
        {
/* The net address is aligned on a long word boundary */
            tm_arp_target_to_ipaddr(arpHeaderPtr, targetNetAddress);
            (*tm_context(tvArpChkInCBFunc))(
#ifdef TM_USE_STRONG_ESL
                                             devPtr,
#endif /* TM_USE_STRONG_ESL */
                                             arpOperation,
                                             senderNetAddress,
                                             targetNetAddress,
                                             enetAddress.enuAddress );
        }

        if (arpHeaderPtr->arpOperation == TM_ARP_REQUEST)
        {
/* The net address is aligned on a long word boundary */
            tm_arp_target_to_ipaddr(arpHeaderPtr, targetNetAddress);

/* Compare the incoming address to our device IP addresses */
#ifdef TM_SINGLE_INTERFACE_HOME
            if ( tm_ip_match( targetNetAddress,
                              tm_ip_dev_addr(devPtr,0)) )
            {
/* They match so send the reply */
                sendArpReply = TM_8BIT_YES;
            }
#else /* TM_SINGLE_INTERFACE_HOME */
            errorCode = tfMhomeAddrMatch( devPtr,
                                          targetNetAddress,
                                          &anyMhomeIndex );
            if (errorCode == TM_ENOERROR)
            {
/* One match so send the reply */
                sendArpReply = TM_8BIT_YES;
            }
            packetPtr->pktMhomeIndex = anyMhomeIndex;

#endif /* TM_SINGLE_INTERFACE_HOME */
            if (    ( sendArpReply == TM_8BIT_ZERO )
                 && ( tm_context(tvProxyHeadPtr) != (ttProxyEntryPtr)0 ) )
            {
/* If match on devices failed try the proxy arp table */
                tm_call_lock_wait(&tm_context(tvProxyLockEntry));
                proxyEntryPtr = tfArpFindProxyEntry(targetNetAddress);
                if (proxyEntryPtr != (ttProxyEntryPtr)0)
                {
                    sendArpReply = TM_8BIT_YES;
                }
                tm_call_unlock(&tm_context(tvProxyLockEntry));
            }
            if (sendArpReply != TM_8BIT_ZERO)
            {
/* They match so send the reply */
                (void)tfArpSendReply(devPtr, packetPtr);
            }
        }
/*
 * Check to see if we are waiting for an arp reply (for both cases of
 * receiving an ARP request, or an ARP reply).
 */
        tm_call_lock_wait(&(devPtr->devLockEntry));
        resEntryPtr = tfArpResolveSearch(devPtr, senderNetAddress);
        if (resEntryPtr != TM_RES_ENTRY_NULL_PTR)
        {
/* Save off the packet ptr from the resolve queue */
            resolvePacketPtr = resEntryPtr->resPacketPtr;
            (void)tfRtArpAddByRte(
                resolvePacketPtr->pktRtePtr,
                enetAddress.enuAddress,
                tm_context(tvArpTimeout),
                (tt8Bit)(TM_ETHER_ADDRESS_LENGTH | flag8023));
/* Now remove it from the list (device UNLOCKED in tfArpResEntryRemove)*/
            tfArpResEntryRemove(devPtr,
                                resEntryPtr,
                                resolvePacketPtr);
/* Send out waiting packets */
             do
             {
                nxtResPacketPtr = resolvePacketPtr->pktChainNextPtr;
#ifdef TM_BYPASS_ETHER_LL
                etherPtr = (tt8BitPtr)
                       &((ttEtherHeaderPtr)resolvePacketPtr->pktLinkDataPtr)
                                                              ->ethDstAddr[0];
#else /* TM_BYPASS_ETHER_LL */
                etherPtr = (tt8BitPtr)&resolvePacketPtr->pktEthernetAddress[0];
#endif
/* Copy ethernet address from enetAddress to area pointed to by etherPtr */
                tm_ether_copy(enetAddress.enuAddress, etherPtr);

                (void)tfDeviceSend(resolvePacketPtr);
                resolvePacketPtr = nxtResPacketPtr;
             } while (resolvePacketPtr != TM_PACKET_NULL_PTR);
        }
        else
        {
            tm_call_unlock(&(devPtr->devLockEntry));
/*
 * We were not waiting for a reply. Assume that we need to update or add
 * an ARP entry. (We will do so unconditionally if the ARP smart cache
 * option is turned on, or (if ARP smart cache option is not turned on,
 * to save memory) only if it were an ARP request sent to us, or an ARP reply).
 */
            addArpEntry = TM_8BIT_YES;
#ifdef TM_4_USE_SCOPE_ID
            tm_6_addr_to_ipv4_mapped(
                senderNetAddress, &(arpMapping.arpm6IpAddress));
            tm_4_dev_scope_addr(&arpMapping.arpm6IpAddress, devPtr);
#endif /* TM_4_USE_SCOPE_ID */
            ttl = tm_context(tvArpTimeout);
/*
 * If the smart ARP option is not turned on, and if it is not an ARP request
 * sent to us, and it is not an ARP reply, then we will update only
 * an existing ARP entry.
 */
            if (    (tm_context(tvArpSmart) == TM_8BIT_ZERO)
                 && (sendArpReply == TM_8BIT_ZERO)
                 && (arpOperation != TM_ARP_REPLY) )
            {
                addArpEntry = TM_8BIT_ZERO;
/* Check if there exists an ARP entry */
#ifdef TM_4_USE_SCOPE_ID
                arpMapping.arpmCommand = TM_6_ARP_GET_BY_IP;
#else /* ! TM_4_USE_SCOPE_ID */
                arpMapping.arpmCommand = TM_4_ARP_GET_BY_IP;
                tm_ip_copy(senderNetAddress, arpMapping.arpmIpAddress);
#endif /* ! TM_4_USE_SCOPE_ID */
                arpMapping.arpmPhysAddrLen = (tt8Bit)TM_ETHER_ADDRESS_LENGTH;
#ifdef TM_USE_STRONG_ESL
                arpMapping.arpmDevPtr = devPtr;
#endif /* TM_USE_STRONG_ESL */
                errorCode = tfRtArpCommand(&arpMapping);
                if (errorCode == TM_ENOERROR)
                {
#ifdef TM_USE_IPV6
                    tm_phys_addr_match(
                        enetAddress.enuAddress,
                        arpMapping.arpmPhysAddr,
                        arpMapping.arpmPhysAddrLen,
                        match);
#endif /* TM_USE_IPV6 */

                    if (
/*
 * Check that the source Ethernet address of the incoming packet does not
 * match that of the ARP cache entry.
 */
#ifdef TM_USE_IPV6
                        (match != arpMapping.arpmPhysAddrLen)
#else /* ! TM_USE_IPV6 */
                        (!(tm_ether_match(enetAddress.enuAddress,
                                          arpMapping.arpmEnetAddress)))
#endif /* ! TM_USE_IPV6 */
                        )
                    {
/* Remote changed its Mac address. Update ARP entry. Use exising TTL */
                        ttl = arpMapping.arpmTtl;
                        addArpEntry = TM_8BIT_YES;
                    }
                }
            }
            if (addArpEntry)
            {
#ifdef TM_4_USE_SCOPE_ID
                (void) tf6RtArpAddEntry(
#ifdef TM_USE_STRONG_ESL
                    devPtr,
#endif /* TM_USE_STRONG_ESL */
                    &(arpMapping.arpm6IpAddress),
                    enetAddress.enuAddress,
                    ttl,
                    (tt8Bit)(TM_ETHER_ADDRESS_LENGTH | flag8023) );
#else /* ! TM_4_USE_SCOPE_ID */
                (void)tf4RtArpAddEntry(
#ifdef TM_USE_STRONG_ESL
                    devPtr,
#endif /* TM_USE_STRONG_ESL */
                    senderNetAddress,
                    enetAddress.enuAddress,
                    ttl,
                    (tt8Bit)(TM_ETHER_ADDRESS_LENGTH | flag8023) );
#endif /* ! TM_4_USE_SCOPE_ID */
            }
        }
    }
    else
    {
        retCode = TM_EPROTONOSUPPORT;
    }
/* Check to see if we have used this packet to generate a reply */
    if (sendArpReply == TM_8BIT_ZERO)
    {
/* We didn't so free it */
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    }
#ifdef TM_USE_DRV_SCAT_RECV
arpIncomingExit:
#endif /* TM_USE_DRV_SCAT_RECV */
    return(retCode);
}

ttProxyEntryPtr tfArpFindProxyEntry (tt4IpAddress ipAddress)
{
    ttProxyEntryPtr proxyEntryPtr;

    proxyEntryPtr = tm_context(tvProxyHeadPtr);
    while (proxyEntryPtr != (ttProxyEntryPtr)0)
    {
        if (tm_ip_match(ipAddress, proxyEntryPtr->proxIpAddress))
        {
            break;
        }
        proxyEntryPtr = proxyEntryPtr->proxNextPtr;
    }
    return proxyEntryPtr;
}

#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4ArpDummy = 0;
#endif /* ! TM_USE_IPV4 */
