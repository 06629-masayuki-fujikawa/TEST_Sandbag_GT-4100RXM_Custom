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
 * Description: IP functions
 * Filename: trip.c
 * Author: Odile
 * Date Created: 12/04/97
 * $Source: source/trip.c $
 *
 * Modification History
 * $Revision: 6.0.2.38 $
 * $Date: 2013/02/02 14:29:53JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_IPV4

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/*
 * local macros
 */

/* Define unreferenced macro for PC-LINT compile error suppression */

#ifdef TM_LINT
LINT_UNREF_MACRO(tm_iface_net_match)
#endif /* TM_LINT */


#define TM_IP_OPT_VAL           0   /* offset to option name */
#define TM_IP_OPT_LEN           1   /* offset to option length */
#define TM_IP_OPT_OFFSET        2   /* offset to option offset */
/* Minimum value for option offset-1 */
#define TM_IP_OPT_MINOFF        (tt8Bit)3
 /* last option */
#define TM_IP_OPT_EOL           (tt8Bit)0
#define TM_IP_OPT_NOP           (tt8Bit)1   /* no (empty) option */
#define TM_IP_OPT_RR            (tt8Bit)7   /* Record route */
#define TM_IP_OPT_LSR           (tt8Bit)3   /* Loose source route */
#define TM_IP_OPT_SSR           (tt8Bit)9   /* Strict source route */

#define TM_OPT_RR_FLAG          0x1
#define TM_OPT_SSR_FLAG         0x2
#define TM_OPT_LSR_FLAG         0x4

#define TM_ONE_GIGABYTE         ((tt32Bit)1073741824)

#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#define tm_get_ip_opt(optPtr, offset) \
    (*optPtr >> (24 - (TM_IP_OPT_VAL << 3))) & 0xff
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
#define tm_get_ip_opt(optPtr, offset) \
    (*(optPtr + (offset>>1)) >> (8 - ((offset % 2) << 3))) & 0xff
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
#define tm_get_ip_opt(optPtr, offset) \
    optPtr[offset]
#endif /* !TM_DSP */

/* Backward compatibility */
#ifndef tm_ip_is_class_broadcast
#define tm_ip_is_class_broadcast(ipAddr) \
  (    !tm_ip_is_class_d_e(ipAddr) \
    && ((tm_ip_class_net_mask(ipAddr) | (ipAddr)) == TM_IP_LIMITED_BROADCAST) )
#endif /* tm_ip_is_class_broadcast */


#ifdef TM_SINGLE_INTERFACE_HOME
/* ipAddr is directed broadcast for device */
#define tm_ip_d_broad_match(devPtr, ipAddr)                         \
        (    tm_4_ll_is_broadcast(devPtr)                           \
          && tm_ip_match(ipAddr, devPtr->devDBroadIpAddr) )
#define tm_ip_class_broad_match(devPtr, ipAddr)                     \
        (    tm_4_ll_is_broadcast(devPtr)                           \
          && (tm_ip_is_class_broadcast(ipAddr))                     \
          && (tm_ip_net_match(ipAddr,                               \
                              tm_ip_dev_addr(devPtr, 0),            \
                              tm_ip_class_net_mask(ipAddr))))
#define tm_iface_any_conf(ifaceMhomePtr) \
        tfIfaceConfig(TM_IP_BROADCAST_FLAG)
#else /* !TM_SINGLE_INTERFACE_HOME */
/* Get device/multi-home index of address match. */
#define tm_iface_addr_match(ipAddr, ifaceMhomePtr) \
        tfIfaceMatch(ipAddr, tfMhomeAddrMatch, ifaceMhomePtr)
/* Get device/multi-home index of network match. */
#define tm_iface_net_match(ipAddr, ifaceMhomePtr) \
        tfIfaceMatch(ipAddr, tfMhomeNetMatch, ifaceMhomePtr)
/* Get device/multi-home index of directed broadcast match. */
#define tm_iface_dbroad_match(ipAddr, ifaceMhomePtr) \
        tfIfaceMatch(ipAddr, tfMhomeDBroadMatch, ifaceMhomePtr)
#define tm_iface_class_broad_match(ipAddr, ifaceMhomePtr) \
        tfIfaceMatch(ipAddr, tfMhomeClassBroadMatch, ifaceMhomePtr)
#define tm_iface_non_zero_addr(ifaceMhomePtr) \
        tfIfaceMatch(TM_IP_ZERO, tfMhomeNoMatchAddr, ifaceMhomePtr)
#define tm_iface_any_conf(ifaceMhomePtr) \
        tfIfaceMatch(TM_IP_ZERO, tfMhomeAnyConf, ifaceMhomePtr)

#endif /* TM_SINGLE_INTERFACE_HOME */

/*
 * Local functions
 */
#ifndef TM_SINGLE_INTERFACE_HOME
static void tfIpForwardPacket(ttPacketPtr     fwdPacketPtr,
                              tt8Bit          optFlag,
                              tt4IpAddressPtr sRouteIpAddrPtr,
                              tt4IpAddressPtr rRouteIpAddrPtr);

/* Get multi-home index of zero network match for a given interface */
static int tfMhomeZeroNet(ttDeviceEntryPtr   devPtr,
                          tt4IpAddress       ipAddress,
                          tt16BitPtr         ifaceMhomePtr);

/* Get multi-home index of any non zero address for a given interface */
TM_NEARCALL static int tfMhomeNoMatchAddr (ttDeviceEntryPtr   devPtr,
                        tt4IpAddress        ipAddr,
                        tt16BitPtr         ifaceMhomePtr);

static int tfMhomeClassBroadMatch(ttDeviceEntryPtr   devPtr,
                                  tt4IpAddress       ipAddress,
                                  tt16BitPtr         ifaceMhomePtr);

#ifdef TM_USE_STRONG_ESL
static ttDeviceEntryPtr tfIpSocBindDevPtr(ttSocketEntryPtr  socketEntryPtr);
static int tfIpAnyConfDevPtr(ttDeviceEntryPtr  devPtr,
                             tt16BitPtr        mhomeIndexPtr);
#endif /* TM_USE_STRONG_ESL */
static ttDeviceEntryPtr tfIfaceAnyConf(tt16BitPtr mhomeIndexPtr);
#endif /* !TM_SINGLE_INTERFACE_HOME */

/*
 * tfIpSendPacket() Function Description
 * This function is called by the ULP (ICMP, IGMP, TCP, or UDP), or by the
 * forward function.
 * 1) If TM_IP_FORWARD_FLAG is not set (called from ULP):
 *     1) if outgoing device is not known and destination is not limited
 *        broadcast try and get that information from a one entry router
 *        cache that keep the last sent packet route and if not there, or
 *        that route is no longer valid, from the routing table based
 *        on our destination IP address. If successful, we get a route
 *        pointer otherwise we free the ttPacket and return an error. If
 *        source IP address is zero, use the outgoing device/multihome index
 *        source IP address (from the route). We also store the outgoing
 *        device pointer in pktDeviceEntryPtr.
 *     3) If outgoing device is not known, and destination is limited
 *        broadcast, broadcast on first configured interface. If source
 *        Ip address in IP Header is zero, copy source address from
 *        first configured interface. We also store the outgoing device
 *        pointer in pktDeviceEntryPtr.
 *     4) If destination address is multicast, set the TM_IP_MULTICAST_FLAG
 *        otherwise if destination address is directed broadcast, set
 *        the TM_IP_BROADCAST_FLAG.
 *     5) We attempt to build the IP header. Note that the source IP address,
 *        destination IP address, Don't fragment bit, tos and
 *        ttl have already been set by the ULP.
 *  2) If device is LAN and TM_IP_MULTICAST_FLAG is set then map
 *     to Ethernet multicast address  if device is capable otherwise to
 *     Ethernet broadcast. If device is LAN and destination IP address
 *     is broadcast, then map to Ethernet broadcast address. If device is
 *     LAN and routing entry has valid ethernet address get that ethernet
 *     address. In all these cases, we will set
 *     TM_ETHERNET_ADDRESS_FLAG and copy the ethernet address in
 *     the ethernet address destination area of the ethernet header. Also
 *     if device is LAN, we store the TM_ETHER_IP_TYPE in the ethernet header.
 *  3) We then check for fragmentation. We check the route/outgoing
 *     interface MTU. If packet is small enough, we send the packet to
 *     the device calling tfDeviceSend(packetPtr) if ethernet address is
 *     already stored, otherwise we call tfArpResolve() and return.
 *  4) Otherwise if packet is too big and we do not fragment
 *     is set or tvIpFragment is not set, we return an error (so that
 *     tfIpForwardPacket can send an ICMP error message back (for PATH MTU
 *     discovery.)). Otherwise we call tfIpFragmentPacket(packetPtr) to
 *     fragment the packet
 *
 *     Note: when tfDeviceSend(packetPtr) is called,
 *     packetPtr->pktChainDataLength contains the total length of the IP
 *     packet,
 *     packetPtr->pktDeviceEntryPtr contains the pointer to the outgoing
 *     device,
 *     The ethernet address and type are already stored in the ethernet
 *     header.
 * End of tfIpSendPacket description
 * tfIpSendPacket Parameters Description:
 * locEntryPtr            pointer to socket entry lock, or IP forward lock
 * varsPtr                Maintained variables so the IPsec task can call back
 *                        into this function and jump to immediately after the
 *                        point where the packet was queued to the IPsec task
 * packetPtr
 * packetPtr->pktChainDataLength contains the size of the IP datagram,
 * packetPtr->pktLinkDataPtr points to the IP datagram,
 * packetPtr->pktLinkDataLength includes the IP header size.
 * packetPtr->pktIpHdrLen is the IP header length.
 * packetPtr->pktFlags contains the IP flags.
 *
 * The IP header already contains source IP address, Destination IP address,
 * do/donot fragment bit, ULP protocol, ttl, and tos from the ULP.
 * More about source IP address set by the ULP:
 * If TCP is the TL and a SYN has been received, TCP would use the TCP IP
 * address the remote peer knows about as a source address. Also for
 * multi-homing a UDP application should be able to specify the source IP
 * address, a response should have the same source IP address as the
 * destination of the request. For ICMP (multihoming) also a response should
 * have the same source IP address as the destination of the request. For
 * TCP (multihoming) when the TCP sends its first SYN, it must either use a
 * source IP address elected by the user, otherwise select its source IP
 * address from the IP layer and use it in every other segment.
 *
 * Returns
 * Value   Meaning
 * Different error values
 *
 * Note: When changing this function, if you introduce a new variable that is
 *       set prior to the call to tfIpsecTaskEnqueue() and read after it, or
 *       you change an existing variable so that it is now used in this
 *       manner, you must add the variable to the appropriate union element
 *       in ttIpsecTaskLocalVars
 */
#ifdef TM_LINT
LINT_UNACCESS_SYM_BEGIN(ipTotalLength)
LINT_UNACCESS_SYM_BEGIN(ipId)
#endif /* TM_LINT */
#ifdef TM_USE_IPSEC
int tfIpSendPacket(
    ttPacketPtr               packetPtr,
    ttLockEntryPtr            lockEntryPtr,
    ttPktLenPtr               ipsecMtuAdjustPtr
#ifdef TM_USE_IPSEC_TASK
    , ttIpsecTaskListEntryPtr varsPtr
#endif /* TM_USE_IPSEC_TASK */
    )
#else /* ! TM_USE_IPSEC */
int tfIpSendPacket (ttPacketPtr packetPtr, ttLockEntryPtr lockEntryPtr)
#endif /* ! TM_USE_IPSEC */
{
#ifdef TM_USE_IPSEC_TASK
    ttIpsecTaskListEntryPtr localVarsPtr;
#endif /* TM_USE_IPSEC_TASK */
    ttIpHeaderPtr           iphPtr;
    ttRteEntryPtr           rtePtr;
    ttRteEntryPtr           rteClonedPtr;
    ttDeviceEntryPtr        devPtr;
    ttSharedDataPtr         pktShrDataPtr;
    tt8BitPtr               ethDestAddrPtr;
    ttLinkLayerEntryPtr     linkLayerProtocolPtr;
#ifdef TM_LOCK_NEEDED
    ttLockEntryPtr          socketLockEntryPtr;
#endif /* TM_LOCK_NEEDED */
#ifdef TM_DEV_SEND_OFFLOAD
    ttDevSendOffloadPtr     pktSendOffloadPtr;
#endif /* TM_DEV_SEND_OFFLOAD */
#ifdef TM_USE_IPSEC
    ttSadbRecordPtr         sadbPtr;
#endif /* TM_USE_IPSEC */
#ifdef TM_USE_TCP
#ifndef TM_USE_PREDICTABLE_IPID
    ttSocketEntryPtr        socketPtr;
#endif /* !TM_USE_PREDICTABLE_IPID */
#endif /* TM_USE_TCP */
#ifdef TM_BYPASS_ETHER_LL
    ttEtherHeaderPtr        etherPtr;
#endif /* TM_BYPASS_ETHER_LL */
#ifdef TM_4_USE_SCOPE_ID
    tt6IpAddress            ipv4MappedSrcAddr;
    tt6IpAddress            ipv4MappedDestAddr;
#endif /* TM_4_USE_SCOPE_ID */
#ifdef TM_USE_IPSEC
    ttPktLen                savedChainPktLen;
#endif /* TM_USE_IPSEC */
    ttPktLen                devMtu;
    ttPktLen                availHdrLen;
    int                     errorCode;
    int                     is_lan;
    int                     etherBroadMcast;
#ifdef TM_USE_IPSEC
    int                     ipsecStatus;
#endif /*TM_USE_IPSEC*/
    int                     naterr;
#ifdef TM_BYPASS_ETHER_LL
    int                     padding;
#endif /* TM_BYPASS_ETHER_LL */
    tt16Bit                 pktFlags;
    tt16Bit                 ipId;
    tt16Bit                 ipTotalLength;
#ifndef TM_USE_PREDICTABLE_IPID
    tt16Bit                 ipIdInc;
#endif /* TM_USE_PREDICTABLE_IPID */
    tt8Bit                  is_gateway;
    tt8Bit                  sendArpRequest;
#ifdef TM_DEV_SEND_OFFLOAD
    tt8Bit                  segOffload;
#endif /* TM_DEV_SEND_OFFLOAD */

#ifdef TM_DEMO_TIME_LIMIT
    if (tm_context(tvDemoTimer.tmrUserParm1.gen32bitParm >= TM_DEMO_LIMIT_SEC))
    {
        tfKernelError("tfIpSendPacket", "demo time limit has expired.");
        tm_thread_stop;
    }
#endif /* TM_DEMO_TIME_LIMIT */

#ifdef TM_USE_IPSEC_TASK
/*
 * If varsPtr isn't null, this function is being called in the context of the
 * IPsec task. We must extract the saved local variables and jump to after
 * the point where the IPsec task was invoked
 */
    if (varsPtr)
    {
#ifdef TM_LOCK_NEEDED
        socketLockEntryPtr = (ttLockEntryPtr)0;
#endif /* TM_LOCK_NEEDED */
        savedChainPktLen   = varsPtr->itlVars.locTx.tx4ChainLen;
        errorCode          = varsPtr->itlErrorCode;
#ifdef TM_DEV_SEND_OFFLOAD
        segOffload         = varsPtr->itlVars.locTx.tx4SegOffload;
#endif /* TM_DEV_SEND_OFFLOAD */
/* Jump to the code immeidately after where this packet was enqueued */
        goto tfIpSendPacketAfterIpsec;
    }
#endif /* TM_USE_IPSEC_TASK */

    errorCode = TM_ENOERROR; /* assume success */
    iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
    pktShrDataPtr = packetPtr->pktSharedDataPtr;
    rtePtr = pktShrDataPtr->dataRtePtr;
    rteClonedPtr = pktShrDataPtr->dataRteClonedPtr;
    pktFlags = packetPtr->pktFlags;
    devPtr = packetPtr->pktDeviceEntryPtr;
    packetPtr->pktNetworkLayer = TM_NETWORK_IPV4_LAYER;
#ifdef TM_DEV_SEND_OFFLOAD
    segOffload = TM_8BIT_NO;
#endif /* TM_DEV_SEND_OFFLOAD */
#ifdef TM_USE_IPSEC
    savedChainPktLen = packetPtr->pktChainDataLength;
#endif /* TM_USE_IPSEC */

#ifdef TM_LOCK_NEEDED
/* assume Ip forward/IP send lock */
    socketLockEntryPtr = (ttLockEntryPtr)0;
    if (lockEntryPtr != (ttLockEntryPtr)0) /* socket or IP cache */
    {
        if (     (lockEntryPtr != &tm_context(tvRtIpForwCacheLock))
              && (lockEntryPtr != &tm_context(tvRtIpSendCacheLock)))
        {
/* Socket lock */
            socketLockEntryPtr = lockEntryPtr;
        }
    }
#endif /* TM_LOCK_NEEDED */
    if (tm_16bit_bits_not_set(pktFlags, TM_IP_FORWARD_FLAG))
    {
/* TM_IP_FORWARD_FLAG is not set (called from ULP) */
        if ( tm_16bit_bits_not_set( pktFlags, TM_IP_DEST_ROUTE_SET) )
/* Non socket IP send, and tfIpDestToPacket() not bypassed */
        {
#ifdef TM_ERROR_CHECKING
            if (lockEntryPtr != (ttLockEntryPtr)0)
            {
/* Should never happen */
                tfKernelError("tfIpSendPacket",
                              "socket send with no destination route set");
            }
#else /* TM_ERROR_CHECKING */
            TM_UNREF_IN_ARG(lockEntryPtr); /* To avoid compiler warning */
#endif /* TM_ERROR_CHECKING */
#ifdef TM_LOCK_NEEDED
            tm_lock_wait(&tm_context(tvRtIpSendCacheLock));
            lockEntryPtr = &tm_context(tvRtIpSendCacheLock);
#endif /* TM_LOCK_NEEDED */
#ifdef TM_4_USE_SCOPE_ID
/* if these addresses are link-local scope, we don't know here what interface
   to scope them to */
            tm_6_addr_to_ipv4_mapped(
                iphPtr->iphSrcAddr, &ipv4MappedSrcAddr);
            tm_6_addr_to_ipv4_mapped(
                iphPtr->iphDestAddr, &ipv4MappedDestAddr);
#endif /* TM_4_USE_SCOPE_ID */
#ifdef TM_USE_STRONG_ESL
            tm_context(tvRtIpSendCache).rtcDevPtr = devPtr;
#endif /* TM_USE_STRONG_ESL */
            errorCode = tfIpDestToPacket(
                packetPtr,
#ifdef TM_4_USE_SCOPE_ID
                &ipv4MappedSrcAddr,
                &ipv4MappedDestAddr,
#else /* ! TM_4_USE_SCOPE_ID */
                iphPtr->iphSrcAddr,
                iphPtr->iphDestAddr,
#endif /* ! TM_4_USE_SCOPE_ID */
                TM_IP_DEF_TOS,
                &tm_context(tvRtIpSendCache),
                iphPtr);
#ifdef TM_USE_STRONG_ESL
/* Reset */
            tm_context(tvRtIpSendCache).rtcDevPtr = (ttDeviceEntryPtr)0;
#endif /* TM_USE_STRONG_ESL */
            if (errorCode != TM_ENOERROR)
            {
                goto badIpSend;
            }
/* Values updated by tfIpDestToPacket */
            rtePtr = pktShrDataPtr->dataRtePtr;
            rteClonedPtr = pktShrDataPtr->dataRteClonedPtr;
            pktFlags = packetPtr->pktFlags;
            devPtr = packetPtr->pktDeviceEntryPtr;
        }
#ifdef TM_ERROR_CHECKING
        if ( tm_16bit_bits_not_set(pktFlags, TM_OUTPUT_DEVICE_FLAG) )
        {
/* This could not happen. */
            tfKernelError("tfIpSendPacket",
                          "No output device initialized");
            tm_thread_stop;
        }
#endif /* TM_ERROR_CHECKING */

#ifdef TM_SNMP_MIB
/*
 * Number of IP datagrams which local IP user-protocols (including ICMP)
 * supplied to IP in requests for transmission (does not include forwarded
 * packets).
 */
        tm_context(tvIpData).ipOutRequests++;
#ifdef TM_USE_NG_MIB2
        tm_64Bit_incr(devPtr->dev4Ipv4Mib.ipIfStatsHCOutRequests);
        tm_64Bit_incr(tm_context(tvDevIpv4Data).ipIfStatsHCOutRequests);
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */

/*
 * We attempt to build the IP header. Note that the source IP address,
 * destination IP address, Don't fragment bit, IP length, tos and
 * ttl have already been set by the ULP.
 */
#ifdef TM_USE_IPSEC
/* for IPsec, if we already finish IPsec processing, we don't need
 * to increase ipId. The old ipId is included in AH calculation.
 */
        if (     (!(tm_context(tvIpsecPtr)))
             ||  (tm_8bit_bits_not_set((packetPtr->pktIpsecLevel),
                                       TM_IPSEC_LVFLAG_FINISHED)))
        {
#endif /* TM_USE_IPSEC */

#ifdef TM_USE_PREDICTABLE_IPID
            tm_kernel_set_critical;
            ipId = ++tm_context(tvIpId);
            tm_kernel_release_critical;
#else /* !TM_USE_PREDICTABLE_IPID */
/* 
 * Using a global, predictable IP ID is a security flaw unless we are
 * inside a valid TCP connection.
 */
#ifdef TM_USE_TCP
            socketPtr = packetPtr->pktSharedDataPtr->dataSocketEntryPtr;
            if (    (socketPtr != (ttSocketEntryPtr)0)
                 && (    (socketPtr->socProtocolNumber == IP_PROTOTCP)
                      && (((ttTcpVectPtr)(ttVoidPtr)socketPtr)->
                                   tcpsState > TM_TCPS_LISTEN) )
               )
            {
                tm_kernel_set_critical;
                ipId = ++tm_context(tvTcpIpId);
                tm_kernel_release_critical;
            }
            else
#endif /* TM_USE_TCP */
            {
/* If there's no socket, increase the IP ID by from 1 to 1024 */
                tfGetRandomBytes((tt8BitPtr)&ipIdInc, sizeof(ipIdInc));
                tm_kernel_set_critical;
                tm_context(tvIpId) = (tt16Bit)
                                (  tm_context(tvIpId)
                                 + (tt16Bit)((tt16Bit)(ipIdInc & 0x03FF) + 1));
                ipId = tm_context(tvIpId);
                tm_kernel_release_critical;
            }
#endif /* TM_USE_PREDICTABLE_IPID */
            tm_htons(ipId, iphPtr->iphId);

#ifdef TM_USE_IPSEC
        }
#endif /* TM_USE_IPSEC */

        iphPtr->iphVersLen = tm_ip_set_vers_Len(packetPtr->pktIpHdrLen);
        iphPtr->iphFlagsFragOff &= TM_IP_DONT_FRAG;

/* for ipTotalLength, if need fragment later, it will change. However, IPsec
 * need it to be here, no matter fragment later or no fragment.
 */
#ifdef TM_USE_TCP
#ifdef TM_DEV_SEND_OFFLOAD
        pktSendOffloadPtr = (ttDevSendOffloadPtr)packetPtr->pktDevOffloadPtr;
        if (    (pktSendOffloadPtr != (ttDevSendOffloadPtr)0)
             && (tm_tcp_send_seg_offload(pktSendOffloadPtr->devoFlags) ) )
        {
            segOffload = TM_8BIT_YES;
/* Increase tvIpId or tvTcpIpId by the number of segments sent - 1 */
            ipId = (tt16Bit)
                       (   pktSendOffloadPtr->devoPayload
                         / (ttUser32Bit)pktSendOffloadPtr->devoFramePayload);

            if (    (ttUser32Bit)(pktSendOffloadPtr->devoFramePayload * ipId)
                 == pktSendOffloadPtr->devoPayload )
            {
                ipId--;
            }
            tm_kernel_set_critical;
#ifdef TM_USE_PREDICTABLE_IPID
            tm_context(tvIpId) = (tt16Bit)(tm_context(tvIpId) + ipId);
#else /* !TM_USE_PREDICTABLE_IPID */
            tm_context(tvTcpIpId) = (tt16Bit)(tm_context(tvTcpIpId) + ipId);
#endif /* !TM_USE_PREDICTABLE_IPID */
            tm_kernel_release_critical;
/* Set the IP header total length to zero */
            iphPtr->iphTotalLength = TM_16BIT_ZERO;
        }
        else
#endif /* TM_DEV_SEND_OFFLOAD */
#endif /* TM_USE_TCP */
        {
            ipTotalLength = (tt16Bit)packetPtr->pktChainDataLength;
            tm_htons(ipTotalLength, iphPtr->iphTotalLength);
        }

/* Device MTU */
        devMtu = (ttPktLen)devPtr->devMtu;

#ifndef TM_USE_IPSEC
/* for IPsec, we always calculate checksum, so that
 * we have good packet header when IPsec applied .
 * (this packet could be tunneled )
 */
        if (    (packetPtr->pktChainDataLength <= devMtu)
#ifdef TM_DEV_SEND_OFFLOAD
             || (segOffload != TM_8BIT_ZERO)
#endif /* TM_DEV_SEND_OFFLOAD */
           )
#endif /* TM_USE_IPSEC */
        {
/* No fragmentation */
/*  Copy ttPktLen pktChainDataLength to 16-bit */
            iphPtr->iphChecksum = (tt16Bit)0;
#ifdef TM_DEV_SEND_OFFLOAD
            if (! (    (pktSendOffloadPtr != (ttDevSendOffloadPtr)0)
                    && (tm_16bit_one_bit_set(pktSendOffloadPtr->devoFlags,
                                             TM_DEVO_IP_CHKSUM_OFFLOAD)) ) )
#endif /* TM_DEV_SEND_OFFLOAD */
            {
#ifndef TM_TM_INFINEON_C166
                if (iphPtr->iphVersLen == TM_IP_NOOPT_VERSLEN)
/* Fast checksum for IP header with no options */
                {
                    iphPtr->iphChecksum =
                        tfIpHdr5Checksum((tt16BitPtr)(ttVoidPtr)iphPtr);
                }
                else
#endif /*TM_TM_INFINEON_C166*/
                {
                    iphPtr->iphChecksum = tfPacketChecksum(
                            packetPtr,
                            (ttPktLen)tm_byte_count(packetPtr->pktIpHdrLen),
                            TM_16BIT_NULL_PTR,
                            TM_16BIT_ZERO );
                }
            }
        }

    }
    else /* forward case */
    {
/* device MTU */
        devMtu = (ttPktLen)devPtr->devMtu;
    }
#ifdef TM_USE_IPSEC
    if (    !(tm_context(tvIpsecPtr))
         || tm_8bit_one_bit_set(packetPtr->pktIpsecLevel,
                                TM_IPSEC_LVFLAG_FINISHED)
/* if TM_BUF_HDR_RECYCLED bit is set, we know we have bypassed IPsec
 * last time, otherwise, this bit should not be set.
 */
         || tm_16bit_one_bit_set(pktShrDataPtr->dataFlags,
                                 TM_BUF_HDR_RECYCLED) )
    {
        goto outIpsecPolicy;
    }
/* in case this packet gets queued.*/
#ifdef TM_LOCK_NEEDED
    packetPtr->pktSendLockEntryPtr = lockEntryPtr;
#endif /* TM_LOCK_NEEDED*/

    if(pktShrDataPtr->dataSocketEntryPtr != (ttSocketEntryPtr)0)
    {
        if(tm_8bit_one_bit_set(
            pktShrDataPtr->dataSocketEntryPtr->socFlags2,
            TM_SOCF2_IPSEC_BYPASS))
        {
            goto outIpsecPolicy;
        }
        else if(tm_8bit_one_bit_set(
            pktShrDataPtr->dataSocketEntryPtr->socFlags2,
            TM_SOCF2_IPSEC_DISCARD))
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfIpSendPacket", "Policy requires discarding");
#endif /* TM_ERROR_CHECKING */
            errorCode = TM_EPOLICYDISCARD;
            goto badIpSend;
        }
    }

    if(packetPtr->pktSadbRecordPtr != (ttSadbRecordPtr)0)
    {
/* we already have the SA queued in packet, this packet
 * come from post-IKE Phase 2 processing.
 * we apply this SA directly and don't add HEAD |TRAILER any more
 */
        goto applyIpsecPolicy;
    }
/* Before we add any extension headers, we save the ulp and the original
 * source and destination address, as well as port number, into the selector
 */
    if(tm_8bit_bits_not_set(packetPtr->pktIpsecLevel,
                            TM_IPSEC_LVFLAG_LEVEL1))
    {
/* pktIpsecLevel indicates how many IPsec head layers has been applied to this
 * packet, zero means it just comes from ulp
 */
        if(packetPtr->pktSelectorPtr)
        {
            tm_free_raw_buffer(packetPtr->pktSelectorPtr);
        }
        packetPtr->pktSelectorPtr = (ttVoidPtr)
            tm_get_raw_buffer(sizeof(ttIpsecSelector));
        if(packetPtr->pktSelectorPtr == (ttVoidPtr)0)
        {
            errorCode = TM_ENOBUFS;
            goto badIpSend;
        }
        tm_bzero(packetPtr->pktSelectorPtr, sizeof(ttIpsecSelector));
/* Does not need IPSEC lock */
        tfIpsecGetSelector(packetPtr, iphPtr->iphUlp,
            0,
            (ttIpsecSelectorPtr)packetPtr->pktSelectorPtr,
            TM_IPSEC_OUTBOUND);
    }
    if (
/* For nested IKE consideration, even if it is an IKE packet, we should not
 * automatically bypass it. We will bypass it in policy query function and
 * when this packet satisfies tfIkeBypassNestedProtection call.
 * Does not need IPSEC lock.
 */
#ifdef TM_USE_IKE
         (tm_context(tvIkePtr)) &&
#endif /* TM_USE_IKE */
         (iphPtr->iphUlp == TM_IP_UDP)
       )
    {
        (void)tfIsIkePacket(packetPtr, (tt8BitPtr)(ttVoidPtr)iphPtr,
                            packetPtr->pktIpHdrLen);
    }
    ipsecStatus = tm_context(tvIpsecFunc).ipsecPolicyQueryFuncPtr(
                                        packetPtr, &sadbPtr, TM_IPSEC_OUTBOUND);
    if (    (ipsecStatus != TM_IPSEC_POLICY_IPSEC)
         && (sadbPtr != (ttSadbRecordPtr)0) )
    {
/* Return sadb ownership */
        tfSadbRecordUnget(TM_UL(0), sadbPtr);
    }
    switch(ipsecStatus)
    {
    case TM_IPSEC_POLICY_NOPOLICY:
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfIpSendPacket", "IPSEC no policy found");
#endif /* TM_ERROR_CHECKING */
        errorCode = TM_EPOLICYNOTFOUND;
        goto badIpSend;
    case TM_IPSEC_POLICY_NOSA:
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfIpSendPacket", "IPSEC no SA found");
#endif /* TM_ERROR_CHECKING */
        errorCode = TM_ESANOTFOUND; /* should queue the packet in IKE case*/
/* we just free the packet in this time
 * already trigger IKE inside IPSEC processing, already queued,
 * don't do anything here, even donothing after
 */
        goto badIpSend;
    case TM_IPSEC_POLICY_BYPASS:
        goto outIpsecPolicy;
/* Not reached */
    case TM_IPSEC_POLICY_DISCARD:
/* either IPsec Policy requires discarding, or error happens in IPsec/IKE
 * processing, then we need to discard this packet
 */
        errorCode = TM_EPOLICYDISCARD;
        goto badIpSend;
    case TM_IPSEC_POLICY_IPSEC:
/* need ipsec processing*/
        break;
    case TM_IPSEC_PACKET_QUEUED:
#ifdef TM_LOCK_NEEDED
/* IKE has unlocked the cache */
        lockEntryPtr = (ttLockEntryPtr)0;
#endif /* TM_LOCK_NEEDED */
        errorCode = TM_IPSEC_PACKET_QUEUED;
        goto outIpSend;
    default:
        errorCode = TM_EPOLICYDISCARD;
        goto badIpSend;
    }
/* apply the policy to this packet. Transfer sadbPtr ownership here. */
    packetPtr->pktSadbRecordPtr = (ttVoidPtr)sadbPtr;
applyIpsecPolicy:

#ifdef TM_USE_IPSEC_TASK
/*
 * Save the important local variables and queue the packet to the IPsec task.
 * Only variables which were set before this point in the code and will be
 * used after this point must be saved
 */
    localVarsPtr = (ttIpsecTaskListEntryPtr)
              tm_get_raw_buffer(sizeof(ttIpsecTaskListEntry));
    if (localVarsPtr == (ttIpsecTaskListEntryPtr)0)
    {
        errorCode = TM_ENOBUFS;
        goto badIpSend;
    }
    localVarsPtr->itlPacketPtr                  = packetPtr;
    localVarsPtr->itlVars.locTx.tx4MtuAdjustPtr = ipsecMtuAdjustPtr;
    localVarsPtr->itlVars.locTx.tx4ChainLen     = savedChainPktLen;
#ifdef TM_DEV_SEND_OFFLOAD
    localVarsPtr->itlVars.locTx.tx4SegOffload   = segOffload;
#endif /* TM_DEV_SEND_OFFLOAD */
    localVarsPtr->itlVars.locTx.tx4FuncPtr =
        tm_context(tvIpsecFunc).ipsecApplyOutgoingPolicyFuncPtr;
    localVarsPtr->itlQueuedFrom                 = TM_IPSEC_TASK_SRC_IP_TX;
/* Cache route entry (in locTx vars structure) BEFORE unlocking */
    tfIpsecTaskCache(packetPtr, &(localVarsPtr->itlVars.locTx.tx4RtCache));
#ifdef TM_LOCK_NEEDED
/* Unlock before we pass this packet off to the queue */
    if (lockEntryPtr)
    {
        tm_unlock(lockEntryPtr);
        lockEntryPtr = (ttLockEntryPtr)0;
    }
#endif /* TM_LOCK_NEEDED */
    errorCode = tfIpsecTaskEnqueue(localVarsPtr);
#else /* TM_USE_IPSEC_TASK */
    errorCode =  tm_context(tvIpsecFunc).
                      ipsecApplyOutgoingPolicyFuncPtr(&packetPtr);
#endif /* TM_USE_IPSEC_TASK */

    if (errorCode != TM_ENOERROR)
    {
        if(errorCode == TM_IPSEC_PACKET_QUEUED)
        {
#if ((!defined(TM_USE_IPSEC_TASK)) && defined(TM_LOCK_NEEDED))
/* IKE has unlocked the cache */
            lockEntryPtr = (ttLockEntryPtr)0;
#endif /* ((!defined(TM_USE_IPSEC_TASK)) && defined(TM_LOCK_NEEDED)) */
            goto outIpSend;
        }
        else
        {
            goto badIpSend;
        }
    }

#ifdef TM_USE_IPSEC_TASK
tfIpSendPacketAfterIpsec:
#endif /* TM_USE_IPSEC_TASK */

/* after apply IPsec policy, the following fields may change */
#ifdef TM_USE_IPV6
    if(packetPtr->pktNetworkLayer == TM_NETWORK_IPV6_LAYER)
/* we built an IPv6 tunnel header according to the IPsec policy
 */
    {
#ifdef TM_LOCK_NEEDED
        if (lockEntryPtr != (ttLockEntryPtr)0)
        {
            tm_unlock(lockEntryPtr);
            lockEntryPtr = (ttLockEntryPtr)0;
        }
/* Will be unlocked by tf6IpSendPacket() */
        tm_lock_wait(&tm_context(tv6RtIpSendCacheLock));
#endif /* TM_LOCK_NEEDED*/
        packetPtr->pktRouteCachePtr = &tm_context(tv6RtIpSendCache);
        errorCode = tf6IpSendPacket(packetPtr,
                                    &tm_context(tv6RtIpSendCacheLock)
#ifdef TM_USE_IPSEC_TASK
                                    , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
                                    );

        goto outIpSend;
    }
#endif /* TM_USE_IPV6 */

/*  After apply IPsec, (such as adding IP tunnel header)
 *  the following value may have changed
 */
    iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
    pktShrDataPtr = packetPtr->pktSharedDataPtr;
    rtePtr = pktShrDataPtr->dataRtePtr;
    rteClonedPtr = pktShrDataPtr->dataRteClonedPtr;
    pktFlags = packetPtr->pktFlags;
    devPtr = packetPtr->pktDeviceEntryPtr;
    devMtu = (ttPktLen)devPtr->devMtu;
outIpsecPolicy:
#endif /*TM_USE_IPSEC*/
#ifdef TM_FORWARDING_STATS
    if (tm_16bit_one_bit_set(pktFlags, TM_IP_FORWARD_FLAG))
    {
        tm_call_lock_wait(&(devPtr->devLockEntry));
        devPtr->devForwTxPkts++;
        devPtr->devForwTxBytes.bnBytes += packetPtr->pktChainDataLength;
        if (devPtr->devForwTxBytes.bnBytes >= TM_ONE_GIGABYTE)
        {
            devPtr->devForwTxBytes.bnGbytes++;
            devPtr->devForwTxBytes.bnBytes -= TM_ONE_GIGABYTE;
        }
        tm_call_unlock(&(devPtr->devLockEntry));
    }
#endif /* TM_FORWARDING_STATS */

#ifdef TM_USE_FILTERING
    if (    (tm_context(tvUserFilterCallback) != (ttUserFilterCallback)0)
         && (devPtr->devFlag2 & TM_DEVF2_FILTERING))
    {
/* if this is an ICMP error that the filter is sending, don't filter again */
        if (tm_16bit_one_bit_set(packetPtr->pktUserFlags,
                                 TM_PKTF_USER_NO_FILTER))
        {
            packetPtr->pktUserFlags &= ~TM_PKTF_USER_NO_FILTER;
        }
        else
        {
#ifdef TM_USE_FILTERING_CONTIGUOUS_DATA
            if (tm_pkt_hdr_is_not_cont(packetPtr,
                                       packetPtr->pktChainDataLength))
            {
#ifdef TM_LOCK_NEEDED
                if (socketLockEntryPtr != (ttLockEntryPtr)0)
                {
                    packetPtr->pktUserFlags |= TM_PKTF_USER_SOCKET_LOCKED;
                }
#endif /* TM_LOCK_NEEDED */
                packetPtr = tfContiguousSharedBuffer(packetPtr,
                                    packetPtr->pktChainDataLength);
                if (packetPtr == (ttPacketPtr)0)
                {
                    goto badIpSend;
                }
#ifdef TM_LOCK_NEEDED
                if (socketLockEntryPtr != (ttLockEntryPtr)0)
                {
                    packetPtr->pktUserFlags &= ~TM_PKTF_USER_SOCKET_LOCKED;
                }
#endif /* TM_LOCK_NEEDED */
/* In case we changed packetPtr */
                iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
                pktShrDataPtr = packetPtr->pktSharedDataPtr;
            }
#endif /* TM_USE_FILTERING_CONTIGUOUS_DATA */
            errorCode = (*(tm_context(tvUserFilterCallback)))(
                                (ttUserInterface)devPtr,
                                (ttVoidPtr)iphPtr,
                                (ttVoidPtr)(packetPtr->pktLinkDataPtr +
                                        tm_ip_hdr_len(iphPtr->iphVersLen)),
                                (int)packetPtr->pktChainDataLength,
                                TM_FILTER_OUTGOING);

            if (errorCode == TM_EOPNOTSUPP)
            {
                goto badIpSend;
            }
        }
    }
#endif /* TM_USE_FILTERING */

    if (tm_nat_public(devPtr))
    {
/*
 * Do not translate, if we will fail to fragment, so that we can send the
 * ICMP error back on the incoming interface.
 */
        if (    (packetPtr->pktChainDataLength <= devMtu)
#ifdef TM_DEV_SEND_OFFLOAD
             || (segOffload != TM_8BIT_ZERO)
#endif /* TM_DEV_SEND_OFFLOAD */
#ifdef TM_IP_FRAGMENT
             || (    (tm_context(tvIpFragment) != 0)
                 && (!((iphPtr->iphFlagsFragOff & TM_IP_DONT_FRAG))) )
#endif /* TM_IP_FRAGMENT */
           )
        {
            naterr = (*devPtr->devNatFuncPtr)(&packetPtr, TM_NAT_SEND);
            if (naterr != TM_ENOERROR)
            {
                goto badIpSend;
            }
/* In case we changed packetPtr */
            iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
            pktShrDataPtr = packetPtr->pktSharedDataPtr;
        }
    }
#ifdef TM_SNMP_MIB
/* Reset flags that could have been set if packet is forwarded or echoed */
    tm_16bit_clr_bit( pktFlags, (  TM_LL_BROADCAST_FLAG
                                 | TM_LL_MULTICAST_FLAG
                                 | TM_LL_UNICAST_FLAG ) );
#endif /* TM_SNMP_MIB */
    sendArpRequest = TM_8BIT_ZERO;
    is_lan = tm_4_ll_is_lan(devPtr);
    availHdrLen = (ttPktLen)
                            (   packetPtr->pktLinkDataPtr
                              - packetPtr->pktSharedDataPtr->dataBufFirstPtr);
    if (availHdrLen < (ttPktLen)(devPtr->devLinkHeaderLength))
    {
/*
 * Not enough room ahead for the link layer header
 * (forwarded or reply packet)
 */
        errorCode = tfPktHeadTailAdd(&packetPtr,
                                     (ttPktLen)packetPtr->pktIpHdrLen,
                                     (ttPktLen)0,
                                     (ttPktLen)0);
        if (errorCode != TM_ENOERROR)
        {
            goto badIpSend; /* free the packet */
        }
/* We changed packetPtr */
        iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
        pktShrDataPtr = packetPtr->pktSharedDataPtr;
    }
/*
 * Indicate ethernet type
 */
/*    packetPtr->pktNetworkLayer = TM_NETWORK_IPV4_LAYER;*/
    if (is_lan)
    {
#ifdef TM_BYPASS_ETHER_LL
/* Min size met? */
        if (packetPtr->pktChainDataLength < TM_ETHER_IP_MIN_PACKET)
        {
/*
 * We need to pad with zeroes
 */
            padding =
                    TM_ETHER_IP_MIN_PACKET - (int)packetPtr->pktChainDataLength;
            if (    (packetPtr->pktChainDataLength
                                                == packetPtr->pktLinkDataLength)
                 && ( (   packetPtr->pktSharedDataPtr->dataBufLastPtr
                        - packetPtr->pktLinkDataPtr) >= TM_ETHER_IP_MIN_PACKET )
               )
/*
 * Contiguous buffer (i.e. non TCP packet, or TCP packet with data copied at
 * the end of the header), and allocated packet has room to add the padding.
 */
            {
                tm_bzero(   packetPtr->pktLinkDataPtr
                          + packetPtr->pktLinkDataLength,
                          padding);
                packetPtr->pktLinkDataLength = TM_ETHER_IP_MIN_PACKET;
                packetPtr->pktChainDataLength = TM_ETHER_IP_MIN_PACKET;
            }
            else
            {
/*
 * Scattered buffer, or allocated packet too short for padding:
 *  add an extra link for the padding initialized with zeroes.
 * Note that if the device cannot handle the extra link,
 * tfDeviceSend() will copy to a new buffer. No need to worry about copying
 * here.
 */
                errorCode = tfPacketTailAdd(packetPtr, padding);
                if (errorCode != TM_ENOERROR)
                {
                    goto badIpSend; /* free the packet */
                }
            }
        }
#endif /* TM_BYPASS_ETHER_LL */
        if ( tm_16bit_bits_not_set( pktShrDataPtr->dataFlags,
                                    TM_BUF_HDR_RECYCLED) )
        {
#ifdef TM_BYPASS_ETHER_LL
/* Build ethernet header */
/*
 * Cannot yet make the packet point to ethernet header (in case of
 * fragmentation)
 */
            etherPtr = (ttEtherHeaderPtr)
                            ((tt8BitPtr)iphPtr - TM_ETHER_HEADER_SIZE);
/*
 * Store source Ethernet Address
 */
            tm_ether_copy( packetPtr->pktDeviceEntryPtr->devPhysAddrArr,
                           etherPtr->ethSrcAddr );
/*
 * We also store the TM_ETHER_IP_TYPE in the ethernet header.
 *
 */
            etherPtr->ethType = TM_ETHER_IP_TYPE;
            ethDestAddrPtr = (tt8BitPtr)(&(etherPtr->ethDstAddr[0]));
#else /* TM_BYPASS_ETHER_LL */
            ethDestAddrPtr = (tt8BitPtr)&pktShrDataPtr->dataEthernetAddress[0];
#endif /* TM_BYPASS_ETHER_LL */

/* Check for most common case first */
            if ( tm_16bit_one_bit_set(pktFlags, TM_IP_RTE_FLAG) )
            {
                if ( tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_INDIRECT) )
                {
/* point to ARP entry */
                    rtePtr = rteClonedPtr;
                    pktShrDataPtr->dataRtePtr = rtePtr;
                    is_gateway = TM_8BIT_YES;
                }
                else
                {
                    is_gateway = TM_8BIT_ZERO;
                }
                if ( tm_16bit_all_bits_set( rtePtr->rteFlags,
                                            TM_RTE_LINK_LAYER|TM_RTE_CLONED) )
/* resolved ARP, or Directed broadcast, or multicast entry */
                {
                    if (rtePtr->rteRefresh == TM_8BIT_YES)
                    {
/* Set while protected with cache lock */
                        rtePtr->rteRefresh = TM_8BIT_ZERO;
                        sendArpRequest = TM_8BIT_YES;
                        if (is_gateway != TM_8BIT_ZERO)
                        {
/* copy intermediate gateway IP address for ARP request. */
#ifdef TM_USE_IPV6
                            tm_6_ip_copy_structs(
                                rtePtr->rteLeafNode.rtnLSKey,
                                pktShrDataPtr->dataSpecDestIpAddress);
#else /* ! TM_USE_IPV6 */
                            tm_ip_copy( rtePtr->rteLeafNode.rtnLSKey,
                                        pktShrDataPtr->dataSpecDestIpAddress);
#endif /* ! TM_USE_IPV6 */
                        }
                        else
                        {
/* copy destination address for ARP request */
#ifdef TM_USE_IPV6
                            tm_6_addr_to_ipv4_mapped(
                                (tt4IpAddress)(iphPtr->iphDestAddr),
                                &(pktShrDataPtr->dataSpecDestIpAddress));
#else /* ! TM_USE_IPV6 */
                            tm_ip_copy(
                                iphPtr->iphDestAddr,
                                pktShrDataPtr->dataSpecDestIpAddress);
#endif /* ! TM_USE_IPV6 */
                        }
                    }
                    tm_ether_copy(rtePtr->rteHSEnetAdd,
                                  ethDestAddrPtr);
                    pktFlags |= TM_ETHERNET_ADDRESS_FLAG;
#ifdef TM_SNMP_MIB
                    if ( tm_16bit_bits_not_set( rtePtr->rteFlags,
                                                TM_RTE_ARP) )
/* Directed Broadcast or multicast entry */
                    {
                        if ( tm_16bit_one_bit_set( rtePtr->rteFlags,
                                                   TM_RTE_MCAST ) )
                        {
/* Multicast entry */
                            pktFlags |= TM_LL_MULTICAST_FLAG;
                        }
                        else
                        {
                            if ( tm_16bit_one_bit_set( rtePtr->rteFlags,
                                                       TM_RTE_BCAST ) )
                            {
/* Directed Broadcast entry */
                                pktFlags |= TM_LL_BROADCAST_FLAG;
                            }
                        }
                    }
#endif /* TM_SNMP_MIB */
                }
                else
                {
#ifdef TM_ERROR_CHECKING
                    if ( tm_16bit_bits_not_set( rtePtr->rteFlags,
                                                TM_RTE_CLONED ) )
                    {
                        goto noArpEntry;
                    }
#endif /* TM_ERROR_CHECKING */
                    tm_kernel_set_critical;
/* Let ARP timer know that this entry is being resolved */
                    rtePtr->rteTtl = TM_RTE_INF;
/*
 * Increase ARP entry owner count so that entry does not disappear,
 * while we are waiting for ARP resolution. We need to do it here
 * before we unlock our cache.
 */
                    rtePtr->rteOwnerCount++;
                    tm_kernel_release_critical;
                    if (is_gateway != TM_8BIT_ZERO)
                    {
/* copy intermediate gateway IP address for ARP resolve. */
#ifdef TM_USE_IPV6
                        tm_6_ip_copy_structs(
                            rtePtr->rteLeafNode.rtnLSKey,
                            pktShrDataPtr->dataSpecDestIpAddress);
#else /* ! TM_USE_IPV6 */
                        tm_ip_copy( rtePtr->rteLeafNode.rtnLSKey,
                                    pktShrDataPtr->dataSpecDestIpAddress);
#endif /* ! TM_USE_IPV6 */
                    }
                    else
                    {
/* copy destination address for ARP resolve */
#ifdef TM_USE_IPV6
                        tm_6_addr_to_ipv4_mapped(
                            (tt4IpAddress)(iphPtr->iphDestAddr),
                            &(pktShrDataPtr->dataSpecDestIpAddress));
#else /* ! TM_USE_IPV6 */
                        tm_ip_copy(
                            iphPtr->iphDestAddr,
                            pktShrDataPtr->dataSpecDestIpAddress);
#endif /* ! TM_USE_IPV6 */
                    }
                }
            }
            else
            {
/*
 * This would happen if destination is limited broadcast, or we
 * did not get a routing pointer because we knew that our destination
 * was multicast or directed broadcast, or we already have our Ethernet
 * address in packetPtr->pktEthernetAddress
 */
                if ( tm_16bit_bits_not_set( pktFlags,
                                            TM_ETHERNET_ADDRESS_FLAG) )
                {
#ifdef TM_USE_LBCAST_CONFIG
/* if limited broadcasts have been disabled and this is one of them, drop it */
                    if (    (tm_context(tvIpLbcastEnable) == TM_8BIT_ZERO)
                         && (tm_ip_match(iphPtr->iphDestAddr,
                                         TM_IP_LIMITED_BROADCAST)))
                    {
                        errorCode = TM_EHOSTUNREACH;
                        goto badIpSend;
                    }
#endif /* TM_USE_LBCAST_CONFIG */
/*
 * Map multicast or broadcast destination IP address to Ethernet address
 */

                    etherBroadMcast = tfIpBroadMcastToLan(
                                                 iphPtr->iphDestAddr,
                                                 packetPtr->pktDeviceEntryPtr,
                                                 packetPtr->pktMhomeIndex,
                                                 ethDestAddrPtr );
                    if (etherBroadMcast != TM_ETHER_UCAST)
                    {
#ifdef TM_SNMP_MIB
                        if (etherBroadMcast == TM_ETHER_BCAST)
                        {
                            pktFlags = (tt16Bit)(
                                        pktFlags
                                      | (tt16Bit)(   TM_ETHERNET_ADDRESS_FLAG
                                                   | TM_LL_BROADCAST_FLAG) );
                        }
                        else
                        {
                            pktFlags = (tt16Bit)(
                                        pktFlags
                                      | (tt16Bit)(   TM_ETHERNET_ADDRESS_FLAG
                                                   | TM_LL_MULTICAST_FLAG) );
                        }
#else /* !TM_SNMP_MIB */
                        pktFlags |= TM_ETHERNET_ADDRESS_FLAG;
#endif /* TM_SNMP_MIB */
                    }
#ifdef TM_ERROR_CHECKING
                    else
                    {
/* No ARP cache. This should not happen */
noArpEntry:             tfKernelError("tfIpSendPacket", "no ARP cache entry");
                        errorCode = TM_EHOSTUNREACH;
                        goto badIpSend;
                    }
#endif /* TM_ERROR_CHECKING */
                }
#ifdef TM_BYPASS_ETHER_LL
                else
                {
                    tm_ether_copy(pktShrDataPtr->dataEthernetAddress,
                                  ethDestAddrPtr);
                }
#endif /* TM_BYPASS_ETHER_LL */
            }
        }
        else
        {
            pktFlags |= TM_ETHERNET_ADDRESS_FLAG;

            if (    (rtePtr != (ttRteEntryPtr)0)
                 && (rtePtr->rteRefresh == TM_8BIT_YES) )
            {
/* Set while protected with cache lock */
                rtePtr->rteRefresh = TM_8BIT_ZERO;
                sendArpRequest = TM_8BIT_YES;
                if (tm_16bit_one_bit_set( packetPtr->pktFlags2,
                                          TM_PF2_INDIRECT) )
                {
/* copy intermediate gateway IP address for ARP request. */
#ifdef TM_USE_IPV6
                    tm_6_ip_copy_structs(
                        rtePtr->rteLeafNode.rtnLSKey,
                        pktShrDataPtr->dataSpecDestIpAddress);
#else /* ! TM_USE_IPV6 */
                    tm_ip_copy(
                        rtePtr->rteLeafNode.rtnLSKey,
                        pktShrDataPtr->dataSpecDestIpAddress);
#endif /* ! TM_USE_IPV6 */
                }
                else
                {
/* copy destination address for ARP request */
#ifdef TM_USE_IPV6
                    tm_6_addr_to_ipv4_mapped(
                        (tt4IpAddress)(iphPtr->iphDestAddr),
                        &(pktShrDataPtr->dataSpecDestIpAddress));
#else /* ! TM_USE_IPV6 */
                    tm_ip_copy(
                        iphPtr->iphDestAddr,
                        pktShrDataPtr->dataSpecDestIpAddress);
#endif /* ! TM_USE_IPV6 */
                }
            }
        }
    }
/* Now that we have our ARP entry pointer , it is OK to unlock the cache */
#ifdef TM_LOCK_NEEDED
    if (lockEntryPtr != (ttLockEntryPtr)0)
    {
        tm_unlock(lockEntryPtr);
/* unlocked */
        lockEntryPtr = (ttLockEntryPtr)0;
    }
#endif /* TM_LOCK_NEEDED */
    packetPtr->pktFlags = pktFlags;
    if (sendArpRequest == TM_8BIT_YES)
    {
        linkLayerProtocolPtr = devPtr->devLinkLayerProtocolPtr;
        if (    (linkLayerProtocolPtr != TM_LINK_NULL_PTR)
             && (linkLayerProtocolPtr->lnkSendRequestFuncPtr !=
                                                (ttLnkSendRequestFuncPtr)0) )
        {
            tm_call_lock_wait(&(devPtr->devLockEntry));
/* ARP code will unlock device entry, when done */
            (void)(*(devPtr->devLinkLayerProtocolPtr->
                             lnkSendRequestFuncPtr))(
                                  (ttVoidPtr)devPtr,
                                  packetPtr->pktMhomeIndex,
#ifdef TM_USE_IPV6
                                  pktShrDataPtr->
                                  dataSpecDestIpAddress.s6LAddr[3],
#else /* ! TM_USE_IPV6 */
                                  pktShrDataPtr->dataSpecDestIpAddress,
#endif /* ! TM_USE_IPV6 */
                                  (ttVoidPtr) pktShrDataPtr->dataRtePtr
                                 );
        }
    }
/* Check for fragmentation */
    if (    (packetPtr->pktChainDataLength <= devMtu)
#ifdef TM_DEV_SEND_OFFLOAD
         || (segOffload != TM_8BIT_ZERO)
#endif /* TM_DEV_SEND_OFFLOAD */
       )
    {
        if (is_lan)
        {
#ifdef TM_BYPASS_ETHER_LL
/* point to Ethernet header (tm_pkt_llp_hdr) */
            packetPtr->pktChainDataLength += TM_ETHER_HEADER_SIZE;
            packetPtr->pktLinkDataLength += TM_ETHER_HEADER_SIZE;
            packetPtr->pktLinkDataPtr -= TM_ETHER_HEADER_SIZE;
#endif /* TM_BYPASS_ETHER_LL */
            if (tm_16bit_bits_not_set(pktFlags, TM_ETHERNET_ADDRESS_FLAG))
            {
/* This packet cannot be recycled */
                tm_16bit_clr_bit( pktShrDataPtr->dataFlags,
                                  TM_BUF_TCP_HDR_BLOCK );
#ifdef TM_BYPASS_ETHER_LL
/* ARP code will unget ARP entry, when done with the ARP entry*/
                errorCode = tfArpResolve(packetPtr);
#else /* !TM_BYPASS_ETHER_LL */
                linkLayerProtocolPtr = devPtr->devLinkLayerProtocolPtr;
                if (    (linkLayerProtocolPtr != TM_LINK_NULL_PTR)
                     && (linkLayerProtocolPtr->lnkResolveFuncPtr !=
                                                (ttLnkResolveFuncPtr)0) )
                {
/* ARP code will unget ARP entry, when done with the ARP entry */
                    errorCode = (*(linkLayerProtocolPtr->lnkResolveFuncPtr))
                                                (packetPtr);
                }
                else
                {
/*
 * Release ownership of ARP entry, since link resolve function could not
 * be called.
 */
                    tfRtUnGet(rtePtr);
                    errorCode = TM_ENODEV;
                    goto badIpSend; /* free the packet */
                }
#endif /* TM_BYPASS_ETHER_LL */
                goto outIpSend;
            }
        }
        errorCode = tfDeviceSend(packetPtr);
        goto outIpSend;
    }
    else
    {
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
        devPtr->dev4Ipv4Mib.ipIfStatsOutFragReqds++;
        tm_context(tvDevIpv4Data).ipIfStatsOutFragReqds++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */

#ifdef TM_IP_FRAGMENT
        if (    (tm_context(tvIpFragment) == 0)
             || ((iphPtr->iphFlagsFragOff & TM_IP_DONT_FRAG)) )
        {
#ifdef TM_SNMP_MIB
/*
 * Number of IP datagrams that have been discarded because they needed to
 * be fragmented at this entity but could not be.
 */
            tm_context(tvIpData).ipFragFails++;
#ifdef TM_USE_NG_MIB2
            devPtr->dev4Ipv4Mib.ipIfStatsOutFragFails++;
            tm_context(tvDevIpv4Data).ipIfStatsOutFragFails++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
            if (    is_lan
                 && tm_16bit_bits_not_set(pktFlags,
                 TM_ETHERNET_ADDRESS_FLAG))
            {
                tfRtUnGet(rtePtr);
            }
            errorCode = TM_EMSGSIZE;
        }
        else
        {
/* This packet cannot be recycled */
            tm_16bit_clr_bit( pktShrDataPtr->dataFlags,
                              TM_BUF_TCP_HDR_BLOCK );

            errorCode = tfIpFragmentPacket(packetPtr, devMtu);
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
            if (errorCode == TM_ENOERROR)
            {
                devPtr->dev4Ipv4Mib.ipIfStatsOutFragOKs++;
                tm_context(tvDevIpv4Data).ipIfStatsOutFragOKs++;
            }
            else
            {
                devPtr->dev4Ipv4Mib.ipIfStatsOutFragFails++;
                tm_context(tvDevIpv4Data).ipIfStatsOutFragFails++;
            }
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
            goto outIpSend;
        }
#else /* !TM_IP_FRAGMENT */
        if (    is_lan
             && tm_16bit_bits_not_set(pktFlags, TM_ETHERNET_ADDRESS_FLAG))
        {
            tfRtUnGet(rtePtr);
        }
#ifdef TM_SNMP_MIB
/*
 * Number of IP datagrams that have been discarded because they needed to
 * be fragmented at this entity but could not be.
 */
        tm_context(tvIpData).ipFragFails++;
#ifdef TM_USE_NG_MIB2
        devPtr->dev4Ipv4Mib.ipIfStatsOutFragFails++;
        tm_context(tvDevIpv4Data).ipIfStatsOutFragFails++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
        errorCode = TM_EMSGSIZE;
#endif /* TM_IP_FRAGMENT */

#ifdef TM_USE_IPSEC
/* keep track of the extra bytes that IPsec added to this packet, so that
   when tfIpForwardPacket sends the ICMP Destination Unreachable "Fragmentation
   Needed" error message it can adjust the MTU stored in the message */
        if (ipsecMtuAdjustPtr != (ttPktLenPtr)0)
        {
            *ipsecMtuAdjustPtr =
                (ttPktLen) (packetPtr->pktChainDataLength - savedChainPktLen);
        }
#endif /* TM_USE_IPSEC */
    }
badIpSend:
#ifdef TM_LOCK_NEEDED
/*
 * If something bad happened, and the lock is still on,
 * Unlock.
 */
    if (lockEntryPtr != (ttLockEntryPtr)0)
    {
        tm_call_unlock(lockEntryPtr);
        lockEntryPtr = (ttLockEntryPtr)0;
    }
#endif /* TM_LOCK_NEEDED */

/* This packet cannot be recycled */
    tm_16bit_clr_bit( pktShrDataPtr->dataFlags,
                      TM_BUF_TCP_HDR_BLOCK );
    tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
outIpSend:
#ifdef TM_USE_IPSEC_TASK
/* Free the buffer containing the saved local variables */
    if (varsPtr)
    {
        tfIpsecTaskFreeVars(varsPtr);
    }
#ifdef TM_LOCK_NEEDED
    else
#endif /* TM_LOCK_NEEDED */
#endif /* TM_USE_IPSEC_TASK */
#ifdef TM_LOCK_NEEDED
    {
/*
 * if lock has been unlocked, and lock was a socket lock, and we weren't
 * called from the context of the IPsec task:
 */
        if (    (lockEntryPtr == (ttLockEntryPtr)0)
             && (socketLockEntryPtr != (ttLockEntryPtr)0)
           )
        {
/*
 * Since socket has been unlocked, re-lock it since the send thread needs
 * ownership.
 */
            tm_lock_wait(socketLockEntryPtr);
        }
    }
#endif /* TM_LOCK_NEEDED */
    return errorCode;
}
#ifdef TM_LINT
LINT_UNACCESS_SYM_END(ipId)
LINT_UNACCESS_SYM_END(ipTotalLength)
#endif /* TM_LINT */
/*
 * tfIpIncomingPacket() Function Description
 * 1) IP header validity check
 *   1) packetPtr->pktChainDataLength should be bigger than mininum size IP
 *      header (20).
 *   2) IP version (in the packet IP header) should be 4.
 *   3) IP header length field (in the packet IP header)  should be at
 *      least 5 words long (i.e. 20).
 *   4) The IP total length field (in the packet IP header) should be bigger
 *      than the IP header length (4*IP header length field).
 *   5) The IP total length field (in the packet IP header) should be less
 *      or equal to the amount of data in the packet.
 *   6) Check IP checksum.
 *  If any one of these tests fail we will silently discard the IP packet and
 *  log the error and return.
 * 2) (optional for speed): check for invalid source address.
 *  We reject any Packet whose source address is limited broadcast, loopback
 *  on a non loop back interface, multicast, or (? directed broadcast
 *  address) on the interface it came in.
 * 3) Check for packet truncation or padding: If the IP total
 *  length retrieved in 5 above is bigger than packetPtr->pktChainDataLength
 *  we will discard the IP packet and return. If it is smaller then we
 *  tail trim the packet.
 * 4) Process IP source route, record route options (for
 *  router configurations only).
 *  Updating the route value in the option will have to wait until we find
 *  the outgoing interface in tfIpForwardPacket(): Check for expired/non
 *  expired routing option. For non-expired routing option, we save a
 *  pointer to the ipAddress of the next router (for update in
 *  tfIpForwardPacket()). Also check for errors like duplicate Source Route
 *  options in which case an ICMP parameter problem will be sent
 *  and we will return.
 * 5) Check most common cases first. Check if the Packet is for
 *  this host and cannot be forwarded by checking the following on the IP
 *  destination address.
 *   1) The destination address matches one of the host's IP addresses on
 *      any valid interface and if there is a routing option, it is expired.
 *      This is our Specific Destination Ip Address. ipFlags is
 *      TM_IP_UNICAST_FLAG.
 *   2) It is a limited broadcast address, or (getting a BOOTP/DHCP reply
 *      (might not be limited broadcast, but we just want the reply even
 *       if we are not configured yet, or if it is for another client, that
 *       we proxy arp for)).
 *      Our Specific Destination Ip address is any valid IP
 *      address on the interface the data came in. We use the source IP
 *      address to find the best match first. If it is 0, we use the
 *      tvIpRouterId. ipFlags is TM_IP_BROADCAST_FLAG.
 *   3) It is a multicast address and the host is a member of this
 *      group on this interface or packet is for IGMP.
 *      (Note: Reject packet, if source address matches one of the
 *      interfaces Ip address. (Avoid getting back our own multicast).)
 *      Our Specific Destination Ip address (for non IGMP packets) is ip
 *      address of interface packet came in . ipFlags is
 *      TM_IP_MULTICAST_FLAG. If address is non local multicast and if
 *      tvIpMcastForward is set and forwarding of multicast is enabled on
 *      interface, set the flag to TM_IP_MCAST_FORWARD_FLAG.
 *      (Note that in the first implementation, tvIpMcastForward is always 0,
 *       but the hook is there for when we do support multicast routing.)
 * 6) If all of the tests performed in 5 above failed check if the
 *  IP destination address is a directed broadcast on any interface and
 *  get the matched interface/multihome index pair back (first hybrid
 *  local/forward case). If  we got a matched interface back  (Specific
 *  Destination address is the IP address of the interface/multihome index
 *  ), and the interface does not match the interface the packet came in
 *  or if we get the same interface the packetPtr->pktFlags flag is
 *  TM_LL_UNICAST_FLAG (network multihoming case), and if tvIpForward is set
 *  and forwarding of directed broadcast enabled on both interfaces,
 *  data is for the host and needs to be forwarded too, otherwise it is only
 *  for the host.
 * 7) Else (not a directed broadcast, not a muticast), (forward
 *  only case), if tvIPForward is set and IP forwarding is enabled on the
 *  interface and neither TM_LL_BROADCAST_FLAG nor TM_LL_MULTICAST_FLAG is
 *  set in packetPtr->pktFlags, and (IP address is not obsolete Berkeley
 *  limited or ? directed broadcast and IP address is not class E) then set
 *  IpFlags to TM_IP_UNICAST_FLAG|TM_IP_FORWARD_FLAG, otherwise free the
 *  packet, and return.
 * 8) If TM_IP_FORWARD_FLAG is set, packet is to be forwarded.
 *  Check that the source IP address is valid for forwarding
 *  (cannot forward packets whose source IP address is 0, or
 *  network part 0 for any network prefix on the incoming device).
 *  If source address is not valid, do not forward, otherwise call
 *  tfIpForwardPacket(), after having duplicated the packet if
 *  TM_IP_LOCAL_FLAG is also set.
 * 9) If TM_IP_LOCAL_FLAG is set process the packet locally.
 *  (We get here for all cases where we need to process the
 *   packet locally (3 cases of 5 + check in 6.) )
 *   Re-assemble the packet if necessary. If packet was
 *   fragmented then call tfIpReassemblePacket(ttPacket * packetPtr). If
 *   tfIpReassemblePacket() returns null then we return otherwise we get a
 *   pointer to the first ttPacket in the re-assembled chain.
 * 10) Pass the packet up to the ULP calling either UDP, TCP or
 *   ICMP with pktLinkDataPtr pointing to the ULP header. For example
 *   for UDP it would be tfUdpIncomingPacket(packetPtr),
 *   packetPtr->pktChainDataLength is the length of the UDP packet,
 *   packetPtr->pktSpecDestIPAddress contains the specific IP Destination
 *   Address, packetPtr->pktSrcIpAddress contains the source IP address, and
 *   pktFlags indicates whether the destination IP address is Broadcast,
 *   Unicast or multicast, pktLinkDataPtr points to the UDP header, and
 *   pktRxIphPtr points to the IP header.
 *
 * Parameter is a pointer to a packet (ttPacket) with
 * packetPtr->pktDeviceEntryPtr pointing to the device the packet came in
 * packetPtr->pktChainDataLength containing the total length of the packet
 * and packetPtr->pktLinkDataLength containing the length of the first
 * scattered data area.
 * packetPtr->pktFlags should be either TM_LL_UNICAST_FLAG,
 * TM_LL_BROADCAST_FLAG, TM_LL_MULTICAST_FLAG, or TM_LL_UNSPECIFIED_FLAG
 * packetPtr->pktEthernetAddress should contain the ethernet address for
 * Ethernet packets.
 * varsPtr      Maintained variables so the IPsec task can call back into this
 *              function and jump to immediately after the point where the
 *              packet was queued to the IPsec task.
 *
 * Note: When changing this function, if you introduce a new variable that is
 *       set prior to the call to tfIpsecTaskEnqueue() and read after it, or
 *       you change an existing variable so that it is now used in this
 *       manner, you must add the variable to the appropriate union element
 *       in ttIpsecTaskLocalVars
 */
void tfIpIncomingPacket( ttPacketPtr               packetPtr
#ifdef TM_USE_IPSEC_TASK
                         , ttIpsecTaskListEntryPtr varsPtr
#endif /* TM_USE_IPSEC_TASK */
)
{
#ifdef TM_IGMP
    int                     errCode;
#endif /* TM_IGMP */
#ifdef TM_USE_IPSEC_TASK
    ttIpsecTaskListEntryPtr localVarsPtr;
#endif /* TM_USE_IPSEC_TASK */
    ttDeviceEntryPtr        devPtr;        /* incoming device pointer */
    ttIpHeaderPtr           iphPtr;         /* pointer to IP header */
/* Pointer to packet shared data structure */
    ttSharedDataPtr         pktShrDataPtr;
#ifdef TM_DEV_RECV_OFFLOAD
    ttDevRecvOffloadPtr     pktDevRecvOffloadPtr;
#endif /* TM_DEV_RECV_OFFLOAD */
#ifndef TM_SINGLE_INTERFACE_HOME
    ttDeviceEntryPtr        tempDevPtr;
    ttDeviceEntryPtr        destDevPtr;    /* outgoing device pointer */
    tt4IpAddressPtr         sRouteIpAddrPtr;/* ptr to route option offset */
    tt4IpAddressPtr         rRouteIpAddrPtr;/* ptr to route option offset */
    ttPacketPtr             fwdPacketPtr;   /* forward packet pointer */
    tt8BitPtr               optPtr;         /* Pointer to options */
#endif /* !TM_SINGLE_INTERFACE_HOME */
    tt4IpAddress            ipSrcAddr;      /* IP packet source address */
/* specific destination ip address */
    tt4IpAddress            ipSpecDestAddr;
#ifdef TM_USE_DRV_SCAT_RECV
    ttPktLen                contLength;
#endif /* TM_USE_DRV_SCAT_RECV */
    tt16Bit                 ipTotalLength;  /* IP datagram length */
    tt16Bit                 ipHdrLength;    /* IP header length */
#define lOptSR   checksum  /* reuse */
#define lIpFlags checksum /* reuse tt16Bit */
    tt16Bit                 checksum;       /* IP header checksum */
    tt8Bit                  savChar;        /* IP version */
    tt8Bit                  ulp;
    tt8Bit                  needFreePacket;
#if (defined(TM_USE_IPSEC) || defined(TM_USE_FILTERING))
    int                     errorCode;
#endif /* TM_USE_IPSEC || TM_USE_FILTERING */
#ifdef TM_USE_IPSEC
#ifndef TM_USE_IPSEC_TASK
    tt8Bit                  hdrLength;
    tt8Bit                  nextHeader;
#endif /* !TM_USE_IPSEC_TASK */
    tt8Bit                  needIncomingPolicyCheck;
#endif /*TM_USE_IPSEC*/
#ifndef TM_SINGLE_INTERFACE_HOME
    tt8Bit                  optDataLen;     /* length of all options */
    tt8Bit                  optVal;         /* Option value */
    tt8Bit                  optLen;         /* Option length */
/* Option offset for route options */
    tt8Bit                  optOffset;
    auto tt16Bit            mhomeIndex;
    tt8Bit                  optFlag;
#endif /* !TM_SINGLE_INTERFACE_HOME */
#if ((!defined(TM_SINGLE_INTERFACE_HOME)) || defined(TM_USE_FILTERING))
    tt8Bit                  icmpType;
    tt8Bit                  icmpCode;
    tt8Bit                  icmpParam;
#endif /* !TM_SINGLE_INTERFACE_HOME || TM_USE_FILTERING */
    int                     naterr;

#ifdef TM_DEMO_TIME_LIMIT
    if (tm_context(tvDemoTimer).tmrUserParm1.gen32bitParm >= TM_DEMO_LIMIT_SEC)
    {
        tfKernelError("tfIpIncomingPacket", "demo time limit has expired.");
        tm_thread_stop;
    }
#endif /* TM_DEMO_TIME_LIMIT */

#ifdef TM_USE_IPSEC_TASK
/*
 * If varsPtr isn't null, this function is being called in the context of the
 * IPsec task. We must extract the saved local variables and jump to after
 * the point where the IPsec task was invoked
 */
    if (varsPtr)
    {
        checksum       = varsPtr->itlVars.locRx.rx4Checksum;
        needFreePacket = varsPtr->itlVars.locRx.rx4FreePacket;

        errorCode      = varsPtr->itlErrorCode;

/* Free the buffer containing the saved local variables */
        tm_free_raw_buffer(varsPtr);
/* Jump to the code immeidately after where this packet was enqueued */
        goto tfIpRecvPacketAfterIpsec;
    }

#endif /* TM_USE_IPSEC_TASK */


#ifdef TM_DEV_RECV_OFFLOAD
/* avoid compiler warning */
    pktDevRecvOffloadPtr = (ttDevRecvOffloadPtr)0;
#endif /* TM_DEV_RECV_OFFLOAD */

#ifndef TM_SINGLE_INTERFACE_HOME
/* assume no unexpired routing option */
    optFlag = TM_8BIT_ZERO;
    sRouteIpAddrPtr = (tt4IpAddressPtr)0;/* to avoid compiler warning */
    rRouteIpAddrPtr = (tt4IpAddressPtr)0;/* to avoid compiler warning */
    optLen = TM_8BIT_ZERO;/* to avoid compiler warning */
#endif /* !TM_SINGLE_INTERFACE_HOME */
#ifdef TM_SNMP_MIB
/* All received IP packets */
    tm_context(tvIpData).ipInReceives++;
#endif /* TM_SNMP_MIB */
    needFreePacket  = TM_8BIT_YES; /* Assume we need to free the packet */
    devPtr = packetPtr->pktDeviceEntryPtr;
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
     tm_64Bit_incr(devPtr->dev4Ipv4Mib.ipIfStatsHCInReceives);
     tm_64Bit_incr(tm_context(tvDevIpv4Data).ipIfStatsHCInReceives);
     tm_64Bit_augm(tm_context(tvDevIpv4Data).ipIfStatsHCInOctets,
                  (tt32Bit)packetPtr->pktUserStruct.pktuLinkDataLength);
#endif /* TM_USE_NG_MIB2 */
     tm_64Bit_augm(devPtr->dev4Ipv4Mib.ipIfStatsHCInOctets,
                  (tt32Bit)packetPtr->pktUserStruct.pktuLinkDataLength);
#endif /* TM_SNMP_MIB */
#ifdef TM_USE_DRV_SCAT_RECV
/* If IP header is not contiguous, make it contiguous */
    contLength =  TM_4_IP_MIN_HDR_LEN + TM_4PAK_ICMP_MIN_LEN;
#if defined(TM_USE_FILTERING) && defined(TM_USE_FILTERING_CONTIGUOUS_DATA)
    if (    (tm_context(tvUserFilterCallback) !=
                                (ttUserFilterCallback)0)
        && (devPtr->devFlag2 & TM_DEVF2_FILTERING))
/* If packet is not contiguous, make it contiguous */
    {
        contLength = packetPtr->pktChainDataLength;
    }
#endif /* TM_USE_FILTERING && TM_USE_FILTERING_CONTIGUOUS_DATA */
    if (tm_pkt_hdr_is_not_cont(packetPtr, contLength))
    {
        packetPtr = tfContiguousSharedBuffer(packetPtr, contLength);
        if (packetPtr == (ttPacketPtr)0)
        {
            goto ipIncomingExit;
        }
    }
#endif /* TM_USE_DRV_SCAT_RECV */
    iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
    ipHdrLength = tm_ip_hdr_len(iphPtr->iphVersLen);
#ifdef TM_USE_DRV_SCAT_RECV
/*
 * If IP header has options, and IP header is not contiguous,
 * make it contiguous
 */
    if (    (ipHdrLength != TM_4_IP_MIN_HDR_LEN)
         && tm_pkt_hdr_is_not_cont(packetPtr,
                                   ipHdrLength + TM_4PAK_ICMP_MIN_LEN) )
    {
        packetPtr = tfContiguousSharedBuffer(packetPtr,
                                         ipHdrLength + TM_4PAK_ICMP_MIN_LEN);
        if (packetPtr == (ttPacketPtr)0)
        {
            goto ipIncomingExit;
        }
        iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
    }
#endif /* TM_USE_DRV_SCAT_RECV */
    packetPtr->pktNetworkLayer = TM_NETWORK_IPV4_LAYER;
    pktShrDataPtr = packetPtr->pktSharedDataPtr;
    tm_ntohs(iphPtr->iphTotalLength, ipTotalLength);
/* Initialize packet IP header length */
    packetPtr->pktIpHdrLen = (tt8Bit)tm_packed_byte_count(ipHdrLength);
    savChar = tm_ip_version(iphPtr->iphVersLen);
    ulp = iphPtr->iphUlp;

/*
 * IP header validity check
 *   1) packetPtr->pktChainDataLength should be bigger than mininum size IP
 *      header (20).
 *   2) IP version (in the packet IP header) should be 4.
 *   3) IP header length field (in the packet IP header)  should be at
 *      least 5 words long (i.e. 20).
 *   4) The IP total length field (in the packet IP header) should be bigger
 *      than the IP header length (4*IP header length field).
 *   5) The IP total length field (in the packet IP header) should be less
 *      or equal to the amount of data in the packet.
 *   6) Check IP checksum.
 */
#ifdef TM_LINT
LINT_UNINIT_SYM_BEGIN(ipTotalLength)
#endif /* TM_LINT */

    if (    (packetPtr->pktChainDataLength
             > tm_byte_count(TM_4PAK_IP_MIN_HDR_LEN))
         && (savChar == TM_IP_4)
         && (tm_packed_byte_count(ipHdrLength) >= TM_4PAK_IP_MIN_HDR_LEN)
         && (ipTotalLength > ipHdrLength)
         && ((ttPktLen)ipTotalLength <= packetPtr->pktChainDataLength) )
#ifdef TM_LINT
LINT_UNINIT_SYM_END(ipTotalLength)
#endif /* TM_LINT */

    {
#ifdef TM_DEV_RECV_OFFLOAD
        pktDevRecvOffloadPtr = (ttDevRecvOffloadPtr)packetPtr->pktDevOffloadPtr;
        if (    (pktDevRecvOffloadPtr != (ttDevRecvOffloadPtr)0)
             && (   pktDevRecvOffloadPtr->devoRFlags
                   & TM_DEVOR_IP_RECV_CHKSUM_VALIDATED ) )
        {
/* No need to validate the checksum */
            checksum = TM_16BIT_ZERO;
        }
        else
#endif /* TM_DEV_RECV_OFFLOAD */
        {
            if (tm_packed_byte_count(ipHdrLength) == TM_4PAK_IP_MIN_HDR_LEN)
            {

/* fast checksum */
                checksum = tfIpHdr5Checksum((tt16BitPtr)(ttVoidPtr)iphPtr);
            }
            else
            {
                checksum = tfPacketChecksum(
                    packetPtr,
                    (ttPktLen)ipHdrLength,
                    TM_16BIT_NULL_PTR,
                    TM_16BIT_ZERO);
            }
        }
#ifdef TM_DEV_RECV_OFFLOAD
        if (    (checksum == TM_16BIT_ZERO)
             && (pktDevRecvOffloadPtr != (ttDevRecvOffloadPtr)0)
             && (   pktDevRecvOffloadPtr->devoRFlags
                  & TM_DEVOR_PACKET_RECV_CHKSUM_COMPUTED ) )
        {
/* Checksum was computed, peel off */
            tfDevoRecvPeelChecksum(packetPtr, (tt16BitPtr)iphPtr,
                                   ipHdrLength);
        }
#endif /* TM_DEV_RECV_OFFLOAD */
    }
    else
    {
/* invalid checksum, because IP header is invalid */
        checksum = (tt16Bit)1;
    }
    if (checksum == (tt16Bit)0)
    {

        tm_ip_copy(iphPtr->iphSrcAddr, ipSrcAddr);
#ifdef TM_IP_SRC_ERROR_CHECKING
/*
 *  (optional for speed): check for invalid source address.
 *  We reject any Packet whose source address is limited broadcast, loopback
 *  on a non loop back interface, multicast, experimental or class broadcast,
 *  or directed broadcast address on the interface it came in. Note that the
 *  check on class broadcast will also eliminate class D, and E, and the
 *  limited broadcast address.
 */
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
        if (   (    (tm_ip_class_net_mask(ipSrcAddr) | (ipSrcAddr))
                 != TM_IP_LIMITED_BROADCAST)
            && (! (   (tm_ip_is_loop_back(ipSrcAddr))
                   && (devPtr != tm_context(tvLoopbackDevPtr))) )
#ifdef TM_SINGLE_INTERFACE_HOME
            && (!(tm_ip_d_broad_match(devPtr, ipSrcAddr)))
#else /* !TM_SINGLE_INTERFACE_HOME */
            && (!(tfMhomeDBroadMatch(devPtr, ipSrcAddr, &mhomeIndex) ==
                                                             TM_ENOERROR))
#endif /* TM_SINGLE_INTERFACE_HOME */
           )
#endif /* TM_IP_SRC_ERROR_CHECKING */
/* PRQA L:L1 */
        {
/*
 *  Check for packet truncation or padding: If the IP total length retrieved
 *  in 5 above is bigger than packetPtr->pktChainDataLength we will
 *  discard the IP packet and return. If it is smaller then update
 *  pktChainDataLength and pktLinkDataLength.
 */
            if ( (ttPktLen)ipTotalLength <= packetPtr->pktChainDataLength )
            {
                if ( (ttPktLen)ipTotalLength !=
                                     packetPtr->pktChainDataLength )
                {
#ifdef TM_DEV_RECV_OFFLOAD
                    if (    (pktDevRecvOffloadPtr != (ttDevRecvOffloadPtr)0)
                         && (   pktDevRecvOffloadPtr->devoRFlags
                              & TM_DEVOR_PACKET_RECV_CHKSUM_COMPUTED ) )
                    {
                        tfDevoRecvTrimChecksum( packetPtr,
                                                ipHdrLength,
                                                ipTotalLength );
                    }
#endif /* TM_DEV_RECV_OFFLOAD */
/*
 * Tail trim.
 */
#if defined(TM_ERROR_CHECKING) || defined(TM_USE_DRV_SCAT_RECV)
                    if (packetPtr->pktChainDataLength !=
                                               packetPtr->pktLinkDataLength)
/* Scattered data, subroutine call */
                    {
#ifndef TM_USE_DRV_SCAT_RECV
                        tfKernelError("tfIpIncomingPacket",
                                      "Incoming scattered data");
#endif /* !TM_USE_DRV_SCAT_RECV */
                        tfPacketTailTrim(packetPtr, (ttPktLen)ipTotalLength,
                                         TM_SOCKET_UNLOCKED);
                    }
                    else
#endif /* defined(TM_ERROR_CHECKING) || defined(TM_USE_DRV_SCAT_RECV) */
                    {
/* no scattered data */
                        packetPtr->pktChainDataLength =
                                                     (ttPktLen)ipTotalLength;
                        packetPtr->pktLinkDataLength =
                                                     (ttPktLen)ipTotalLength;
                    }
                }
                if (tm_nat_public(devPtr))
                {
                    naterr =
                        (*devPtr->devNatFuncPtr)(&packetPtr, TM_NAT_INCOMING);
                    if (naterr != TM_ENOERROR)
                    {
                        goto ipIncomingExit; /* needFreePacket is set */
                    }
/* In case we changed packetPtr */
                    iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
                    pktShrDataPtr = packetPtr->pktSharedDataPtr;
                }
#ifdef TM_USE_FILTERING
                if (    (tm_context(tvUserFilterCallback) !=
                                        (ttUserFilterCallback)0)
                    && (devPtr->devFlag2 & TM_DEVF2_FILTERING))
                {
                    errorCode = (*(tm_context(tvUserFilterCallback)))(
                            (ttUserInterface)devPtr,
                            (ttVoidPtr)iphPtr,
                            (ttVoidPtr)(packetPtr->pktLinkDataPtr +
                                    tm_ip_hdr_len(iphPtr->iphVersLen)),
                            (int)packetPtr->pktChainDataLength,
                            TM_FILTER_INCOMING);
                    if (errorCode != TM_ENOERROR)
                    {
                        if (errorCode == TM_EOPNOTSUPP)
                        {
                            goto ipIncomingExit;
                        }
                        if (errorCode == TM_ENETUNREACH)
                        {
                            icmpCode = (tt8Bit)TM_ICMP_CODE_NET;
                        }
                        else
                        {
                            icmpCode = (tt8Bit)TM_ICMP_CODE_HOST;
                        }
                        icmpType = TM_ICMP_TYPE_UNREACH;
                        icmpParam = TM_8BIT_ZERO;
/* mark this packet so it does not get refiltered */
                        packetPtr->pktUserFlags |= TM_PKTF_USER_NO_FILTER;
                        tfIcmpErrPacket(packetPtr,
                                        icmpType,
                                        (tt8Bit)icmpCode,
                                        (tt32Bit)icmpParam,
                                        TM_DEV_NULL_PTR,
                                        TM_IP_ZERO);
                        needFreePacket = TM_8BIT_ZERO;
                        goto ipIncomingExit;
                    }
                }
#endif /* TM_USE_FILTERING */
#ifdef TM_USE_IPSEC
/* Does not need IPSEC lock */
                if (
                     tm_context(tvIpsecPtr) &&
/* For nested IKE consideration, even if it is an IKE packet, we should not
 * automatically bypass it. We will bypass it in policy query function and
 * when this packet satisfies tfIkeBypassNestedProtection call.
 * Does not need IPSEC lock.
 */
#ifdef TM_USE_IKE
                     (tm_context(tvIkePtr)) &&
#endif /* TM_USE_IKE */
                     (ulp == TM_IP_UDP)
                   )
                {
                    packetPtr = tfIsIkePacket(packetPtr,
                                              (tt8BitPtr)(ttVoidPtr)iphPtr,
                                              ipHdrLength);
#ifdef TM_USE_NATT
/* See if we have a UDP/ESP packet */
                    if (tm_16bit_one_bit_set(packetPtr->pktFlags2,
                                             TM_PF2_ESP_UDP))
                    {
                        ulp = TM_IP_ESP; /* really UDP/ESP */
                    }
#endif /* TM_USE_NATT */
                }
#endif /* TM_USE_IPSEC*/
/* Destination Ip Address */
                tm_ip_copy(iphPtr->iphDestAddr, ipSpecDestAddr);
/*
 *  Process IP options. We only support trace route and record
 *  route options when packet needs to be forwarded (non expired routing
 *  option). We need to check for routing options here because if there
 *  is a non expired routing option, a packet whose destination address is
 *  ours will have to be forwarded to its next hop as found in the routing
 *  option. We only need to do so if we are a router
 *  (tvIpForward on), since we would not get a non expired routing option
 *  otherwise. If there is a non expired routing option, we will update
 *  ipSpecDestAddr to the next intermediate destination (next hop).
 */
#ifndef TM_SINGLE_INTERFACE_HOME
                if (tm_packed_byte_count(ipHdrLength) > TM_4PAK_IP_MIN_HDR_LEN)
/* options are present */
                {
/*
 * Check for non expired routing option if we are a router
 */
                    if (tm_context(tvIpForward))
                    {
/* check if we need to forward routing options */
/* point to options */
                        optPtr = (tt8BitPtr)(
                            (ttCharPtr)iphPtr + TM_4PAK_IP_MIN_HDR_LEN );
                        icmpType = TM_ICMP_TYPE_PARM_PROB;
                        icmpCode = TM_ICMP_CODE_PARM;
                        icmpParam = TM_8BIT_ZERO;
                        lOptSR = 0;
                        for (optDataLen = (tt8Bit)
                                       (ipHdrLength -
                                        tm_byte_count(TM_4PAK_IP_MIN_HDR_LEN));
/* length of unprocessed options */
                             optDataLen != (tt8Bit)0;
/* Point to the next option */
                             optPtr += optLen )
                        {
/* Get the next option */
                            optPtr = tfIpGetNextOpt(optPtr, &optDataLen,
                                                    &optVal, &optLen,
                                                    &icmpParam,
                                                    (tt8Bit)ipHdrLength);
                            if (icmpParam != TM_8BIT_ZERO)
                            {
/* error in option length field */
                                 break;
                            }
                            if (   (optVal == TM_IP_OPT_RR)
                                || (optVal == TM_IP_OPT_SSR)
                                || (optVal == TM_IP_OPT_LSR) )
                            {
/* pointer to next hop */
                                optOffset = (tt8Bit)
                                        (tm_get_ip_opt(optPtr,
                                                       TM_IP_OPT_OFFSET) - 1);
                                if (optOffset < TM_IP_OPT_MINOFF)
                                {
/* Invalid pointer to next hop. Parameter error pointer: */
                                    icmpParam = (tt8Bit)( ipHdrLength
                                                         + TM_IP_OPT_OFFSET
                                                         - optDataLen);
                                    break;
                                }
/* Source route option (loose or strict) */
                                if (   (optVal == TM_IP_OPT_SSR)
                                    || (optVal == TM_IP_OPT_LSR) )
                                {
                                    if (lOptSR)
                                    {
/* Duplicate route option. Parameter error pointer: */
                                        icmpParam = (tt8Bit)(  ipHdrLength
                                                              - optDataLen);
                                        break;
                                    }
                                    lOptSR = 1;
/* destination not on this host */
                                    pktShrDataPtr->dataDestDevPtr =
                                        tm_iface_addr_match(ipSpecDestAddr,
                                                   &packetPtr->pktMhomeIndex);
                                    if (pktShrDataPtr->dataDestDevPtr ==
                                                              TM_DEV_NULL_PTR)
                                    {
                                        if (optVal == TM_IP_OPT_SSR)
                                        {
/* Strict source route does not allow that */
                                            icmpType = TM_ICMP_TYPE_UNREACH;
                                            icmpCode = TM_ICMP_CODE_SRC;
                                            break;
                                        }
                                        else
                                        {
/* Loose source route, just need to forward */
                                            continue;
                                        }
                                    }
                                }
                                if (optOffset <= optLen -
                                        tm_byte_count(
                                            (tt8Bit) sizeof(tt4IpAddress)))

/* Non expired option (room to record outgoing interface IP address) */
                                {
/* Save pointer to Ip address to record to update later in tfIpForwardPacket*/
                                    if (optVal == TM_IP_OPT_RR)
                                    {
                                        rRouteIpAddrPtr =
                                               (tt4IpAddressPtr)(optPtr+
                                               tm_packed_byte_count(
                                                   optOffset));
                                            optFlag |= TM_OPT_RR_FLAG;
                                    }
                                    else
                                    {
                                        sRouteIpAddrPtr =
                                               (tt4IpAddressPtr)(optPtr+
                                               tm_packed_byte_count(
                                                   optOffset));
                                        if (optVal == TM_IP_OPT_SSR)
                                        {
                                            optFlag |= TM_OPT_SSR_FLAG;
                                        }
                                        else
                                        {
                                            optFlag |= TM_OPT_LSR_FLAG;
                                        }
                                    }
/* Update offset in routing option */
#ifdef TM_32BIT_DSP
                                    *optPtr |= ((optOffset + 1 +
                                                     tm_byte_count(
                                                     sizeof(tt4IpAddress))) <<
                                                     8) & 0x0000ff00;
#else /* !TM_32BIT_DSP */

                                    optPtr[TM_IP_OPT_OFFSET] = (tt8Bit)
                                                       (optOffset + 1 +
                                                        sizeof(tt4IpAddress));
#endif /* TM_32BIT_DSP */
/* Copy intermediate destination IP address (from routing option) */
                                    if ( tm_8bit_one_bit_set(optFlag,
                                        TM_OPT_SSR_FLAG|TM_OPT_LSR_FLAG) )
                                    {
                                        tm_ip_ptr_u_copy(sRouteIpAddrPtr,
                                                         ipSpecDestAddr);
                                    }
                                }
                            }
                        }
                        if (   (icmpCode != TM_ICMP_CODE_PARM)
                            || (icmpParam != 0))
                        {
#ifdef TM_SNMP_MIB
/* Problem in IP header (including options) */
                            tm_context(tvIpData).ipInHdrErrors++;
#ifdef TM_USE_NG_MIB2
                            devPtr->dev4Ipv4Mib.ipIfStatsInHdrErrors++;
                            tm_context(tvDevIpv4Data).ipIfStatsInHdrErrors++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
                            tfIcmpErrPacket(packetPtr,
                                            icmpType,
                                            (tt8Bit)icmpCode,
                                            (tt32Bit)icmpParam,
                                            TM_DEV_NULL_PTR,
                                            TM_IP_ZERO);
                            needFreePacket = TM_8BIT_ZERO;
                            goto ipIncomingExit; /* return */
                        }
                    }
                }
#endif /* !TM_SINGLE_INTERFACE_HOME */
#ifdef TM_DROP_RX_IP_SRC_RTE
                if (    (tm_packed_byte_count(ipHdrLength)
                            > TM_4PAK_IP_MIN_HDR_LEN)
                     && !(tm_context(tvIpForward)) )
/* options are present, and IP forwarding is disabled */
                {
/* when we are not forwarding packets, for security reasons discard any
   packets we receive which have a source routing IP option */
/* point to options */
                    optPtr = (tt8BitPtr)(
                        (ttCharPtr)iphPtr + TM_4PAK_IP_MIN_HDR_LEN );
                    icmpParam = TM_8BIT_ZERO;
                    for (optDataLen = (tt8Bit)
                                       (ipHdrLength -
                                        tm_byte_count(TM_4PAK_IP_MIN_HDR_LEN));
/* length of unprocessed options */
                         optDataLen != (tt8Bit)0;
/* Point to the next option */
                         optPtr += optLen )
                    {
/* Get the next option */
                        optPtr = tfIpGetNextOpt(optPtr, &optDataLen,
                                                &optVal, &optLen,
                                                &icmpParam,
                                                (tt8Bit)ipHdrLength);
                        if (icmpParam != TM_8BIT_ZERO)
                        {
/* error in option length field */
                            break;
                        }
                        if (   (optVal == TM_IP_OPT_SSR)
                            || (optVal == TM_IP_OPT_LSR))
                        {
/* for security reasons, discard packets that have source routing IP option */
                            goto ipIncomingExit; /* needFreePacket is set*/
                        }
                    }
                }
#endif /* TM_DROP_RX_IP_SRC_RTE */

                lIpFlags = packetPtr->pktFlags;
/*
 * Save a copy of the IP destination IP address for user peek
 * (tfRecvFromTo()).
 */
                tm_ip_copy(ipSpecDestAddr, packetPtr->pktDestIpAddress);
/*  Check most common cases first. Check if the Packet is for
 *  this host and cannot be forwarded by checking the following on the IP
 *  destination address.
 *   1) The destination address matches one of the host's IP addresses on
 *      any valid interface and if there is a routing option, it is expired.
 *      This is our Specific Destination Ip Address. lIpFlags is
 *      TM_IP_UNICAST_FLAG.
 */
#ifdef TM_SINGLE_INTERFACE_HOME
                if (    tm_8bit_one_bit_set( tm_ip_dev_conf_flag(devPtr, 0),
                                             TM_DEV_IP_CONFIG)
                     && tm_ip_match( ipSpecDestAddr,
                                     tm_ip_dev_addr(devPtr, 0) ) )
#else /* ! TM_SINGLE_INTERFACE_HOME */
/* destDevPtr != 0 if destination IP address is for this host */
#ifdef TM_4_USE_SCOPE_ID
                if (IN4_IS_ADDR_LINKLOCAL(ipSpecDestAddr))
                {
                    if (tfMhomeAddrMatch(
                            devPtr,
                            ipSpecDestAddr,
                            &packetPtr->pktMhomeIndex) == TM_ENOERROR )
                    {
                        pktShrDataPtr->dataDestDevPtr = devPtr;
                    }
                }
                else
#endif /* TM_4_USE_SCOPE_ID */
                {
                    pktShrDataPtr->dataDestDevPtr = tm_iface_addr_match(
                        ipSpecDestAddr,
                        &packetPtr->pktMhomeIndex);
                }

                if ( pktShrDataPtr->dataDestDevPtr != TM_DEV_NULL_PTR )
#endif /* ! TM_SINGLE_INTERFACE_HOME */
                {
                    lIpFlags |= TM_IP_UNICAST_FLAG|TM_IP_LOCAL_FLAG;
                    goto processLocal; /* goto to avoid check on lIpFlags */
                }
/*
 * 2) We are not checking for IP address match on the incoming device,
 *    or
 *    It is a limited broadcast address, or (getting a BOOTP/DHCP reply
 *      (might not be limited broadcast, but we just want the reply even
 *       if we are not configured yet, or if it is for another client, that
 *       we proxy arp for)).
 *      Our Specific Destination Ip address is any valid IP address on the
 *      interface the data came in. We use the source IP address to find
 *      the best match. lIpFlags is TM_IP_BROADCAST_FLAG.
 */
                else
                {
/*
 * Device is trusting the link, and expects every packet to be for this host.
 */
                    if (tm_dev_no_check(devPtr))
                    {
                        pktShrDataPtr->dataDestDevPtr = devPtr;
                        lIpFlags |= TM_IP_UNICAST_FLAG|TM_IP_LOCAL_FLAG;
                        goto processLocal;
                    }
#ifdef TM_USE_LBCAST_CONFIG
/* if limited broadcasting is disabled, drop all incoming limited broadcasts */
                    if (    (tm_context(tvIpLbcastEnable) == TM_8BIT_ZERO)
                         && (tm_ip_match(ipSpecDestAddr,
                                         TM_IP_LIMITED_BROADCAST)) )
                    {
                        needFreePacket = TM_8BIT_YES;
                        goto ipIncomingExit;
                    }
#endif /* TM_USE_LBCAST_CONFIG */
                    if (   tm_ip_match( ipSpecDestAddr,
                                        TM_IP_LIMITED_BROADCAST )
                        || (
#ifdef TM_USE_STRONG_ESL
                                (    devPtr->devBootSocketDescriptor
                                  != TM_SOCKET_ERROR)
#else /* TM_USE_STRONG_ESL */
                                (    tm_context(tvBootSocketDescriptor)
                                  != TM_SOCKET_ERROR)
#endif /* TM_USE_STRONG_ESL */
                             && ( ulp == TM_IP_UDP )
                             && ( TM_BOOTPC_PORT == ( (ttTlHeaderPtr)
                                   (   packetPtr->pktLinkDataPtr
                                     + tm_packed_byte_count(ipHdrLength) )
                                     )->tlDport
                                )
                             && (    tm_16bit_one_bit_set(   devPtr->devFlag,
                                                             TM_DEV_IP_BOOTP
                                                           | TM_DEV_IP_DHCP )
                                )
                           )
                       )
                    {
                        if (tm_ip_match(ipSpecDestAddr, TM_IP_LIMITED_BROADCAST))
                        {
                            lIpFlags |= TM_IP_BROADCAST_FLAG | TM_IP_LOCAL_FLAG;
                        }
                        else
                        {
                            lIpFlags |= TM_IP_LOCAL_FLAG;
                        }
#ifdef TM_SINGLE_INTERFACE_HOME
                        tm_ip_copy( tm_ip_dev_addr(devPtr, 0),
                                    ipSpecDestAddr );
#else /* !TM_SINGLE_INTERFACE_HOME */
                        pktShrDataPtr->dataDestDevPtr = devPtr;
/*
 * Only look for configured address for the specific destination address:
 * NOTE TM_DEV_IP_CONFIG is INPUT parameter to tfMhomeAnyConf(). mhomeIndex
 * will be set by tfMhomeAnyConf or tfMhomeNetMatch if successful.
 */
                        mhomeIndex = (tt16Bit)TM_DEV_IP_CONFIG;
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
                        if (   ( tfMhomeNetMatch(devPtr,
                                                 ipSrcAddr,
                                                 &mhomeIndex)
                                                             == TM_ENOERROR)
/* If no network match on source address, use first configured address */
                            || (tfMhomeAnyConf(devPtr, ipSrcAddr,
                                               &mhomeIndex)
                                                           == TM_ENOERROR) )
/* PRQA L:L1 */
                        {
                            packetPtr->pktMhomeIndex = mhomeIndex;
                            tm_ip_copy(tm_ip_dev_addr(devPtr, mhomeIndex),
                                       ipSpecDestAddr);
                        }
#endif /* TM_SINGLE_INTERFACE_HOME */
                        goto processLocal; /*goto to avoid check on lIpFlags */
                    }
/*
 *   3) It is a multicast address and the host is a member of this
 *      group on this interface or packet is for IGMP.
 *      (Note: Reject packet, if source address matches one of the
 *      interfaces Ip address. (We do not want to receive our own multicast).)
 *      Our Specific Destination Ip address (for non IGMP packets) is
 *      address of interface packet came in. lIpFlags is TM_IP_MULTICAST_FLAG.
 *      If address is non local multicast and if
 *      tvIpMcastForward is set and multicast address is non local
 *      add TM_IP_MCAST_FORWARD_FLAG.
 *      (Note that in the first implementation, tvIpMcastForward is always 0,
 *       but the hook is there for when we do support multicast routing.)
 */
                    else
                    {
                        if (tm_ip_is_multicast(ipSpecDestAddr))
                        {
#ifdef TM_SINGLE_INTERFACE_HOME
/* Reject multicast that we sent, unless TM_IP_ALLOW_MCAST_LOOPBACK is
 * defined. In that case, allow everything except IGMP */
                            if ( tm_ip_match( ipSrcAddr,
                                              tm_ip_dev_addr(devPtr, 0) )
#ifdef TM_IP_ALLOW_MCAST_LOOPBACK
                                    && (ulp == TM_IP_IGMP)
#endif /* TM_IP_ALLOW_MCAST_LOOPBACK */
                               )
                            {
                                goto ipIncomingExit; /* free the packet */
                            }
#ifdef TM_IGMP
                            errCode = tfIgmpMember(devPtr, ipSpecDestAddr
#ifdef TM_USE_IGMPV3
                                                  , ipSrcAddr
#endif /* TM_USE_IGMPV3 */
                                                  );
                            if ( errCode == TM_ENOERROR)
                            {
                                lIpFlags |= TM_IP_MULTICAST_FLAG |
                                            TM_IP_LOCAL_FLAG;
                                tm_ip_copy( tm_ip_dev_addr(devPtr, 0),
                                            ipSpecDestAddr );
                            }
/* don't need to be a member to receive IGMP multicasts */
                            else
                            {
                                if (ulp == TM_IP_IGMP)
                                {
                                    lIpFlags |= TM_IP_MULTICAST_FLAG |
                                                TM_IP_LOCAL_FLAG;
                                }
                            }
#endif /* TM_IGMP */
#else /* !TM_SINGLE_INTERFACE_HOME */
                            pktShrDataPtr->dataDestDevPtr = devPtr;
/* Reject multicast that we sent. We do not allow loop back. */
                            tempDevPtr = tm_iface_addr_match(ipSrcAddr, &mhomeIndex);
                            if ( (tempDevPtr != TM_DEV_NULL_PTR)
#ifdef TM_IP_ALLOW_MCAST_LOOPBACK
                                    && (ulp == TM_IP_IGMP)
#endif /* TM_IP_ALLOW_MCAST_LOOPBACK */
                               )
                            {
                                goto ipIncomingExit; /* free the packet */
                            }
#ifdef TM_IGMP
                            errCode = tfIgmpMember(devPtr, ipSpecDestAddr
#ifdef TM_USE_IGMPV3
                                           , ipSrcAddr
#endif /* TM_USE_IGMPV3 */
                                           , &mhomeIndex);
                            if ( errCode == TM_ENOERROR)
                            {
                                lIpFlags |= TM_IP_MULTICAST_FLAG |
                                            TM_IP_LOCAL_FLAG;

                                if ( ulp != TM_IP_IGMP )
/*
 * For non IGMP ULP (Upper Layer Protocols), compute the specific destination
 * IP address (i.e address of interface packet came in).
 */
                                {
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
                                    if (    !tm_ip_is_local_mcast_address(
                                                        iphPtr->iphDestAddr)
/*
 * If multicast is local, try and get the multihome index by checking
 * network match on source IP address.
 */
                                         || (    tfMhomeNetMatch(
                                                   devPtr,
                                                   ipSrcAddr,
                                                   &packetPtr->pktMhomeIndex )
                                              != TM_ENOERROR ) )
/* PRQA L:L1 */
                                    {
/*
 * Multicast address is non local, or we did not find a network match
 * on source IP address, use mhome index returned by tfIgmpMember
 */
                                        packetPtr->pktMhomeIndex = mhomeIndex;
                                    }
                                    tm_ip_copy( tm_ip_dev_addr(
                                                   devPtr,
                                                   packetPtr->pktMhomeIndex ),
                                                ipSpecDestAddr );
                                }
                            }
/*
 * For IGMP we need to keep the original multicast destination address.
 * don't need to be a member to receive IGMP multicasts
 */
                            else
                            {
                                if (ulp == TM_IP_IGMP)
                                {
                                    lIpFlags |= TM_IP_MULTICAST_FLAG |
                                                TM_IP_LOCAL_FLAG;
                                }
                            }
#endif /* TM_IGMP */
#ifdef TM_IP_MCAST_FORWARD
                            if (    tm_context(tvIpMcastForward)
                                 && tm_4_dev_mcast_forward(devPtr)
                                 && !tm_ip_is_local_mcast_address(iphPtr->
                                                                iphDestAddr) )
                            {
                                lIpFlags |= TM_IP_MCAST_FORWARD_FLAG;
                            }
#endif /* TM_IP_MCAST_FORWARD */
#endif /* TM_SINGLE_INTERFACE_HOME */
                        }
                        else
                        {
/*
 *  6) If all of the tests performed in 5 above failed check if the
 *  IP destination address is a directed broadcast on any interface and
 *  get the matched interface/multihome index pair back (first hybrid
 *  local/forward case). If  we got a matched interface back (Specific
 *  Destination address is the IP address of the interface/multihome index),
 *  packet is for this host. In addition, if interface does not match the
 *  interface the packet came in or, if we get the same interface the
 *  packetPtr->pktFlags flag is TM_LL_UNICAST_FLAG, set the forward flag
 *  if forwarding is enabled.
 */
#ifdef TM_SINGLE_INTERFACE_HOME
                            if (   tm_ip_d_broad_match(devPtr, ipSpecDestAddr)
                                || tm_ip_class_broad_match(devPtr,
                                                           ipSpecDestAddr)
                               )
                            {
                                tm_ip_copy( tm_ip_dev_addr(devPtr, 0),
                                            ipSpecDestAddr );
                                lIpFlags |= TM_IP_BROADCAST_FLAG |
                                            TM_IP_LOCAL_FLAG;
                            }
#else /* !TM_SINGLE_INTERFACE_HOME */
                            destDevPtr = tm_iface_dbroad_match(ipSpecDestAddr,
                                                   &packetPtr->pktMhomeIndex);
                            if (destDevPtr == TM_DEV_NULL_PTR)
                            {
                                if (tm_ip_is_class_broadcast(ipSpecDestAddr))
                                {
                                    destDevPtr = tm_iface_class_broad_match(
                                                   ipSpecDestAddr,
                                                   &packetPtr->pktMhomeIndex);
                                }
                            }
/* destDevPtr != 0 if destination IP address is for this host */
                            if (destDevPtr != TM_DEV_NULL_PTR)
                            {
                                pktShrDataPtr->dataDestDevPtr = destDevPtr;
                                tm_ip_copy(tm_ip_dev_addr(
                                                  destDevPtr,
                                                  packetPtr->pktMhomeIndex),
                                           ipSpecDestAddr);
                                if (    tm_context(tvIpForward)
                                     && tm_context(tvIpDbcastForward)
                                     && tm_dev_dbroad_forward(devPtr)
                                     && tm_dev_dbroad_forward(destDevPtr)
                                     && (    (destDevPtr != devPtr)
                                          || tm_16bit_one_bit_set(lIpFlags,
                                                       TM_LL_UNICAST_FLAG) ) )
                                {
                                    lIpFlags |= TM_IP_BROADCAST_FLAG |
                                                TM_IP_LOCAL_FLAG     |
                                                TM_IP_FORWARD_FLAG;
                                }
                                else
                                {
                                    lIpFlags |= TM_IP_LOCAL_FLAG |
                                                TM_IP_BROADCAST_FLAG;
                                }
                            }
#endif /* !TM_SINGLE_INTERFACE_HOME */
                            else
                            {
/* 7) Else (not a directed broadcast, not a muticast), (forward
 *  only case), if tvIPForward is set and IP forwarding is enabled on the
 *  interface and neither TM_LL_BROADCAST_FLAG nor TM_LL_MULTICAST_FLAG is
 *  set in packetPtr->pktFlags, and IP address is not obsolete Berkeley
 *  limited and IP address is not class E and IP address not loopback then
 *  set lIpFlags to TM_IP_UNICAST_FLAG|TM_IP_FORWARD_FLAG, otherwise free the
 *  packet, and return.
 */
#ifndef TM_SINGLE_INTERFACE_HOME
                                if (    tm_context(tvIpForward)
                                     && tm_4_dev_forward(devPtr)
                                     && tm_16bit_bits_not_set( lIpFlags,
                                                       TM_LL_BROADCAST_FLAG
                                                     | TM_LL_MULTICAST_FLAG )
                                     && tm_ip_not_zero(ipSpecDestAddr)
                                     && !tm_ip_is_class_e(ipSpecDestAddr)
                                     && !tm_ip_is_loop_back(ipSpecDestAddr) )
                                {
                                    lIpFlags |= TM_IP_FORWARD_FLAG|
                                                TM_IP_UNICAST_FLAG;
                                }
                                else
#endif /* !TM_SINGLE_INTERFACE_HOME */
                                {
#ifndef TM_SINGLE_INTERFACE_HOME
/* Find a mhome in the process of configuring with TM_DEV_IP_USER_BOOT
 * NOTE TM_DEV_IP_CONF_STARTED|TM_DEV_IP_USER_BOOT is INPUT parameter
 * to tfMhomeAnyConf().
 * mhomeIndex will be set by tfMhomeAnyConf if successful.
 */
                                    mhomeIndex = (tt16Bit)
                                                 (  TM_DEV_IP_CONF_STARTED
                                                  | TM_DEV_IP_USER_BOOT);
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
                                    if (    ((int)devPtr->devMhomeUsedEntries
                                                                   == 0)
                                         && (tfMhomeAnyConf(devPtr, ipSrcAddr,
                                                       &mhomeIndex)
                                                           == TM_ENOERROR) )
/* PRQA L:L1 */
#else /* TM_SINGLE_INTERFACE_HOME */
                                    if ( tm_8bit_all_bits_set(
                                                tm_ip_dev_conf_flag(devPtr, 0),
                                                (  TM_DEV_IP_CONF_STARTED
                                                 | TM_DEV_IP_USER_BOOT) ) )
#endif /* TM_SINGLE_INTERFACE_HOME */
                                    {
                                        pktShrDataPtr->dataDestDevPtr = devPtr;
                                        lIpFlags |= (  TM_IP_UNICAST_FLAG
                                                     | TM_IP_LOCAL_FLAG );
                                        goto processLocal;
                                    }
                                    else
                                    {
#ifdef TM_SNMP_MIB
/*
 * Problem with IP Header Destination IP address, or entity is not
 * an IP router and destination is not a local address
 */
                                        tm_context(tvIpData).ipInAddrErrors++;
#ifdef TM_USE_NG_MIB2
                                        devPtr->dev4Ipv4Mib.ipIfStatsInAddrErrors++;
                                        tm_context(tvDevIpv4Data).ipIfStatsInAddrErrors++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
/* cannot forward, free the packet */
                                        goto ipIncomingExit;
                                    }
                                }
                            }
                        }
                    }
                }
/*
 * 8) If TM_IP_FORWARD_FLAG is set, forward the packet
 *   (tfIpForwardPacket ()), after having duplicated the packet if
 *   TM_IP_LOCAL_FLAG is also set.
 */
/* Update pktFlags with local flags variable value */
                packetPtr->pktFlags = lIpFlags;
#ifndef TM_SINGLE_INTERFACE_HOME
                fwdPacketPtr = packetPtr;
#ifdef TM_IP_MCAST_FORWARD
                if (tm_16bit_one_bit_set(lIpFlags,
                                         TM_IP_FORWARD_FLAG |
                                         TM_IP_MCAST_FORWARD_FLAG))
#else /* !TM_IP_MCAST_FORWARD */
                if (tm_16bit_one_bit_set(lIpFlags, TM_IP_FORWARD_FLAG))
#endif /* TM_IP_MCAST_FORWARD */
                {
/*
 * Packet is to be forwarded. Check that the source IP address is valid for
 * forwarding (cannot forward packets whose source IP address is 0, or
 * network part 0 for any network prefix on the incoming device).
 * If source address is not valid, do not forward.
 */
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
                    if (   tm_ip_zero(ipSrcAddr)
                        || (tfMhomeZeroNet(devPtr, ipSrcAddr, &mhomeIndex)
                                                          == TM_ENOERROR) )
/* PRQA L:L1 */
                    {
#ifdef TM_SNMP_MIB
                        if ( tm_16bit_bits_not_set( lIpFlags,
                                                    TM_IP_LOCAL_FLAG) )
                        {
/* Problem in IP header */
                            tm_context(tvIpData).ipInHdrErrors++;
#ifdef TM_USE_NG_MIB2
                            devPtr->dev4Ipv4Mib.ipIfStatsInHdrErrors++;
                            tm_context(tvDevIpv4Data).ipIfStatsInHdrErrors++;
#endif /* TM_USE_NG_MIB2 */
                        }
#endif /* TM_SNMP_MIB */
/* So that we know that the IP packet is not being forwarded */
                        tm_16bit_clr_bit( lIpFlags,
                                          (  TM_IP_FORWARD_FLAG
                                           | TM_IP_MCAST_FORWARD_FLAG) );
/* cannot forward */
                        goto checkLocal;
                    }
                    if (tm_16bit_one_bit_set(lIpFlags, TM_IP_LOCAL_FLAG))
                    {
/*
 * REVNOTE: If packet is UDP, we probably should copy the packet instead
 * of duplicating it, since local UDP will alter the IP header for Checksum
 * computation.
 */
                        fwdPacketPtr = tfDuplicatePacket(packetPtr);
                        if (fwdPacketPtr == TM_PACKET_NULL_PTR)
                        {
/* So that we know that the IP packet is not being forwarded */
                            tm_16bit_clr_bit( lIpFlags,
                                              (  TM_IP_FORWARD_FLAG
                                               | TM_IP_MCAST_FORWARD_FLAG) );
/* cannot forward, check if it is a packet for this host */
                            goto processLocal;
                        }
                    }
#ifdef TM_SNMP_MIB
/* Number of forward attempts */
                    tm_context(tvIpData).ipForwDatagrams++;
#ifdef TM_USE_NG_MIB2
                    tm_64Bit_incr(devPtr->dev4Ipv4Mib.ipIfStatsHCInForwDatagrams);
                    tm_64Bit_incr(tm_context(tvDevIpv4Data).ipIfStatsHCInForwDatagrams);
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
#ifdef TM_FORWARDING_STATS
                    tm_call_lock_wait(&(devPtr->devLockEntry));
                    devPtr->devForwRxPkts++;
                    devPtr->devForwRxBytes.bnBytes +=
                            fwdPacketPtr->pktChainDataLength;
                    if (devPtr->devForwRxBytes.bnBytes >= TM_ONE_GIGABYTE)
                    {
                        devPtr->devForwRxBytes.bnGbytes++;
                        devPtr->devForwRxBytes.bnBytes -= TM_ONE_GIGABYTE;
                    }
                    tm_call_unlock(&(devPtr->devLockEntry));
#endif /* TM_FORWARDING_STATS */
#ifdef TM_IP_MCAST_FORWARD
                    if (tm_16bit_one_bit_set(lIpFlags, TM_IP_FORWARD_FLAG))
                    {
                        tfIpForwardPacket(fwdPacketPtr, optFlag,
                                          sRouteIpAddrPtr, rRouteIpAddrPtr);
                    }
                    else
                    {
                        tfIpMcastForwardPacket(fwdPacketPtr, optFlag,
                                            sRouteIpAddrPtr, rRouteIpAddrPtr);
                    }
#else /* !TM_IP_MCAST_FORWARD */
                    tfIpForwardPacket(fwdPacketPtr, optFlag, sRouteIpAddrPtr,
                                      rRouteIpAddrPtr);
#endif /* TM_IP_MCAST_FORWARD */
                }
checkLocal:
#endif /* !TM_SINGLE_INTERFACE_HOME */
/* 9) If TM_IP_LOCAL_FLAG is set process the packet locally.
 *  (We get here for all cases where we need to process the
 *   packet locally (3 cases of 5 + check in 6.) )
 *   Re-assemble the packet if necessary. If packet was
 *   fragmented then call tfIpReassemblePacket(ttPacket * packetPtr). If
 *   tfIpReassemblePacket() returns null then we return otherwise we get a
 *   pointer to the first ttPacket in the re-assembled chain.
 *   pass the packet up to the ULP calling either UDP, TCP or
 *   ICMP with pktLinkDataPtr pointing to the ULP header. For example
 *   for UDP it would be tfUdpIncomingPacket(packetPtr),
 *   packetPtr->pktChainDataLength is the length of the UDP packet,
 *   packetPtr->pktSpecDestIPAddress contains the specific IP Destination
 *   Address, packetPtr->pktSrcIpAddress contains the source IP address, and
 *   pktFlags indicates whether the destination IP address is Broadcast,
 *   Unicast or multicast, pktLinkDataPtr points to the UDP header, and
 *   pktRxIphPtr points to the IP header.
 */
                if ( tm_16bit_one_bit_set(lIpFlags, TM_IP_LOCAL_FLAG) )
                {
processLocal:
/* Update pktFlags with local variable IpFlags value */
                    packetPtr->pktFlags = lIpFlags;
/* IP Fragmentation flags */
                    if ( tm_16bit_one_bit_set(iphPtr->iphFlagsFragOff,
                                         TM_IP_MORE_FRAG|TM_IP_FRAG_OFFSET) )
                    {
#ifdef TM_SNMP_MIB
/* The number of IP fragments received which needed to be reassembled */
                        tm_context(tvIpData).ipReasmReqds++;
#endif /* TM_SNMP_MIB */
#ifdef TM_IP_REASSEMBLY
                        packetPtr = tfIpReassemblePacket(packetPtr);
                        if (packetPtr == TM_PACKET_NULL_PTR)
                        {
/* packet queued in reassembly queue. Just return. Do not free. */
                            needFreePacket = TM_8BIT_ZERO;
                            goto ipIncomingExit;
                        }
#ifdef TM_USE_IPSEC
/* after reassembling, we need to tell again if this is an IKE packet
 * or not
 */
                        if (tm_context(tvIpsecPtr))
                        {

                            iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
                            ipHdrLength = tm_ip_hdr_len(iphPtr->iphVersLen);
/* Does not need IPSEC lock */
                            if (
/* For nested IKE consideration, even if it is an IKE packet, we should not
 * automatically bypass it. We will bypass it in policy query function and
 * when this packet satisfies tfIkeBypassNestedProtection call.
 * Does not need IPSEC lock.
 */
#ifdef TM_USE_IKE
                                 (tm_context(tvIkePtr)) &&
#endif /* TM_USE_IKE */
                                 (iphPtr->iphUlp == TM_IP_UDP)
                               )
                            {
                                packetPtr = tfIsIkePacket(packetPtr,
                                              (tt8BitPtr)(ttVoidPtr)iphPtr,
                                              ipHdrLength);
                            }
                        }
#endif /* TM_USE_IPSEC */

/* Reassembled packet. Update shared data pointer */
                        pktShrDataPtr = packetPtr->pktSharedDataPtr;
#ifdef TM_SNMP_MIB
/* Number of IP datagrams successfully re-assembled. */
                        tm_context(tvIpData).ipReasmOKs++;
#ifdef TM_USE_NG_MIB2
                        devPtr->dev4Ipv4Mib.ipIfStatsReasmOKs++;
                        tm_context(tvDevIpv4Data).ipIfStatsReasmOKs++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */

#else /* !TM_IP_REASSEMBLY */

/* IP fragmenation and reassembly not turned on. Discard the fragment */
#ifdef TM_SNMP_MIB
/*
 * Number of failures detected by the IP re-assembly algorithm (for
 * whatever reason: timed out, errors, etc).
 */
                        tm_context(tvIpData).ipReasmFails++;
#ifdef TM_USE_NG_MIB2
            devPtr->dev4Ipv4Mib.ipIfStatsReasmFails++;
            tm_context(tvDevIpv4Data).ipIfStatsReasmFails++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
                        goto ipIncomingExit; /* needFreePacket is set */

#endif /* TM_IP_REASSEMBLY */
                    }
/* Did not initialize before, since those 2 fields are altered by
 * tfIpReassemblePacket()
 */
#ifdef TM_USE_IPV6
                    tm_6_addr_to_ipv4_mapped(
                        (tt4IpAddress)(ipSpecDestAddr),
                        &(pktShrDataPtr->dataSpecDestIpAddress));
#else /* ! TM_USE_IPV6 */
                    tm_ip_copy(
                        ipSpecDestAddr,
                        pktShrDataPtr->dataSpecDestIpAddress);
#endif /* ! TM_USE_IPV6 */
                    tm_ip_copy(ipSrcAddr,
                               pktShrDataPtr->dataSrcIpAddress);

#ifdef TM_USE_IPSEC
                    if (tm_context(tvIpsecPtr))
                    {
                        needIncomingPolicyCheck = TM_8BIT_YES;
                        switch(ulp)
                        {
                            case TM_IP_TCP:
                            case TM_IP_UDP:
/*
 * When using IKE policies, we do not need to check IKE packets
 * against IPsec selectors before the global IKE bypass comes into
 * play.  In this case, we can bypass immediately because this will
 * pass the packet to IKE which will in turn do an IKE policy check.
 *
 */
                                if (
                                (tm_8bit_one_bit_set(packetPtr->pktIpsecLevel,
                                    TM_IPSEC_LVFLAG_IKEPACKET))
                                    &&
                                (tm_context(tvIpsecFunc).ipsecIkeBypass)
                                   )
                                {
                                    needIncomingPolicyCheck = TM_8BIT_NO;
                                }
#ifdef TM_IGMP
                            case TM_IP_IGMP:
#endif /* TM_IGMP */
                                break;
                            case TM_IP_ICMP:
/*
 * For ICMP, we don't do incoming policy check if
 * TM_IPSEC_INCOMING_ICMP_BYPASS is defined
 */
                                if (
                                tm_context(tvIpsecFunc).ipsecIcmpInBypass)
                                {
                                    needIncomingPolicyCheck = TM_8BIT_NO;
                                }
                                break;
                            case TM_IP_AH:
                            case TM_IP_ESP:
                                needIncomingPolicyCheck = TM_8BIT_NO;
                                break;
                            case TM_IP_IP_ENCAP:
                            case TM_IP_MIN_ENCAP:
#ifdef TM_USE_IPV6
                            case TM_IP_IPV6:
#endif /* TM_USE_IPV6 */
/* IP-in-IP encapsulation or Minimal IP encapsulation. */
                                if (tm_context(tvIpDecapFuncPtr) !=
                                                     (ttIpHandlerFuncPtr) 0)
                                {
                                    needIncomingPolicyCheck = TM_8BIT_NO;
                                    break;
                                }
/* else fallthrough to default if there is no decap function  */
                            default:
#ifndef TM_USE_RAW_SOCKET
                                needIncomingPolicyCheck = TM_8BIT_NO;
#endif /* !TM_USE_RAW_SOCKET */
                                break;
                        }
                        if (needIncomingPolicyCheck)
                        {
/* already passed the IPsec processing, need to verify IPsec policy */
                            errorCode = (&tm_context(tvIpsecFunc))
                                ->ipsecIncomingPolicyCheckFuncPtr( packetPtr,
                                                                   ulp );
                            if( errorCode != TM_ENOERROR )
                            {
#ifdef TM_ERROR_CHECKING
                                tfKernelWarning("tfIpIncomingPolicyCheck",
                                     "Incoming IPSEC policy check failed");
#endif /* TM_ERROR_CHECKING */
                                goto ipIncomingExit; /* needFreePacket is set */
                            }
                        }
                    }
#endif /*TM_USE_IPSEC*/
                    switch(ulp)
                    {
#ifdef TM_USE_TCP
                    case TM_IP_TCP:
                        tfTcpIncomingPacket(packetPtr);
                        break;
#endif /* TM_USE_TCP */
                    case TM_IP_ICMP:
                        tfIcmpIncomingPacket(packetPtr);
                        break;
#ifdef TM_IGMP
                    case TM_IP_IGMP:
                        tfIgmpIncomingPacket(packetPtr);
                        break;
#endif /* TM_IGMP */
                    case TM_IP_UDP:
                        tfUdpIncomingPacket(packetPtr);
                        break;
#ifdef TM_USE_IPSEC
                    case TM_IP_AH:
                    case TM_IP_ESP:
                        if (tm_context(tvIpsecPtr) != (ttVoidPtr)0)
                        {


#ifdef TM_USE_IPSEC_TASK
/*
 * Save the important local variables and queue the packet to the IPsec task.
 * Only variables which were set before this point in the code and will be
 * used after this point must be saved
 */
                            localVarsPtr = (ttIpsecTaskListEntryPtr)
                                    tm_get_raw_buffer(
                                    sizeof(ttIpsecTaskListEntry));

                            if (localVarsPtr == (ttIpsecTaskListEntryPtr)0)
                            {
                                needFreePacket = TM_8BIT_YES;
                                errorCode = TM_ENOBUFS;
                                goto ipIncomingExit;
                            }
                            localVarsPtr->itlPacketPtr = packetPtr;
                            localVarsPtr->itlVars.locRx.rx4Checksum =
                                checksum;
                            localVarsPtr->itlVars.locRx.rx4FreePacket =
                                needFreePacket;
                            localVarsPtr->itlVars.locRx.rx4Ulp = ulp;
                            localVarsPtr->itlVars.locRx.rx4FuncPtr =
                                tm_context(tvIpsecFunc).ipsecIncomingFuncPtr;
                            localVarsPtr->itlQueuedFrom =
                                TM_IPSEC_TASK_SRC_IP_RX;

                            errorCode = tfIpsecTaskEnqueue(localVarsPtr);
#else /* TM_USE_IPSEC_TASK */
                            errorCode =
                                tm_context(tvIpsecFunc).ipsecIncomingFuncPtr
                                     (packetPtr,
                                      ulp,
                                      &hdrLength,
                                      &nextHeader);
#endif /* TM_USE_IPSEC_TASK */



                            if (errorCode != TM_ENOERROR)
                            {
/* already freed if an error occurred */
                                needFreePacket = TM_8BIT_ZERO;
                                goto ipIncomingExit;
                            }
                            break;
                        }
/* else fallthru to default if tfUseIpsec() has not been called */
#endif /* TM_USE_IPSEC */
                    default:
/* IP-in-IP encapsulation and Minimal IP encapsulation. */
                        if (   (   (ulp == TM_IP_IP_ENCAP)
                                || (ulp == TM_IP_MIN_ENCAP)
#ifdef TM_USE_IPV6
                                || (ulp == TM_IP_IPV6)
#endif /* TM_USE_IPV6 */
                               )
                            && (tm_context(tvIpDecapFuncPtr) !=
                                          (ttIpHandlerFuncPtr) 0
                               )
                           )
                        {
                            (*(tm_context(tvIpDecapFuncPtr)))(packetPtr, ulp);
                        }
                        else
                        {
#ifdef TM_USE_RAW_SOCKET
                            tf4RawIncomingPacket(packetPtr);
#else /* !TM_USER_RAW_SOCKET */
/* packet has not been sent up */
                            tm_16bit_clr_bit( lIpFlags, TM_IP_LOCAL_FLAG );
#endif /* TM_USE_RAW_SOCKET */
                        }
                    }

#ifdef TM_USE_IPSEC_TASK
tfIpRecvPacketAfterIpsec:
#endif /* TM_USE_IPSEC_TASK */

                    if (tm_16bit_bits_not_set(lIpFlags, TM_IP_LOCAL_FLAG))
                    {
#ifdef TM_SNMP_MIB
/* Input datagrams discarded because of an unknown or unsupported protocol */
                        tm_context(tvIpData).ipInUnknownProtos++;
#endif /* TM_SNMP_MIB */
                        tfIcmpErrPacket(packetPtr,
                                        TM_ICMP_TYPE_UNREACH,
                                        TM_ICMP_CODE_PROTO,
                                        TM_32BIT_ZERO, /* unused */
                                        TM_DEV_NULL_PTR,
                                        TM_IP_ZERO);
                        needFreePacket = TM_8BIT_ZERO;
                    }
#ifdef TM_SNMP_MIB
                    else
                    {
/* Input datagrams successfully delivered to upper layer protocols */
                        tm_context(tvIpData).ipInDelivers++;
#ifdef TM_USE_NG_MIB2
                        tm_64Bit_incr(devPtr->dev4Ipv4Mib.ipIfStatsHCInDelivers);
                        tm_64Bit_incr(tm_context(tvDevIpv4Data).ipIfStatsHCInDelivers);
#endif /* TM_USE_NG_MIB2 */
                    }
#endif /* TM_SNMP_MIB */
                }
/* packet has been forwarded or sent up */
#ifdef TM_SINGLE_INTERFACE_HOME
                if (tm_16bit_one_bit_set(lIpFlags, TM_IP_LOCAL_FLAG))
#else /* !TM_SINGLE_INTERFACE_HOME */
                if (tm_16bit_one_bit_set(lIpFlags,
                                           TM_IP_FORWARD_FLAG
                                         | TM_IP_MCAST_FORWARD_FLAG
                                         | TM_IP_LOCAL_FLAG))
#endif /* TM_SINGLE_INTERFACE_HOME */
                {
/* Do not free the packet */
                    needFreePacket = TM_8BIT_ZERO;
                }
/* otherwise free the packet */
            }
#if ( (defined(TM_SNMP_MIB) || defined(TM_ERROR_CHECKING)) )
            else
            {
#ifdef TM_SNMP_MIB
/* Problem in IP header */
                tm_context(tvIpData).ipInHdrErrors++;
#ifdef TM_USE_NG_MIB2
                devPtr->dev4Ipv4Mib.ipIfStatsInHdrErrors++;
                tm_context(tvDevIpv4Data).ipIfStatsInHdrErrors++;
                devPtr->dev4Ipv4Mib.ipIfStatsInTruncatedPkts++;
                tm_context(tvDevIpv4Data).ipIfStatsInTruncatedPkts++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfIpIncomingPacket", "Truncated packet");
#endif /* TM_ERROR_CHECKING */
            }
#endif  /* ( (defined(TM_SNMP_MIB) || defined(TM_ERROR_CHECKING))  */
        }
#if ( (defined(TM_SNMP_MIB) || defined(TM_ERROR_CHECKING)) )
#ifdef TM_IP_SRC_ERROR_CHECKING
        else
        {
#ifdef TM_SNMP_MIB
/* Problem in IP header */
            tm_context(tvIpData).ipInHdrErrors++;
#ifdef TM_USE_NG_MIB2
            devPtr->dev4Ipv4Mib.ipIfStatsInHdrErrors++;
            tm_context(tvDevIpv4Data).ipIfStatsInHdrErrors++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfIpIncomingPacket", "Invalid source address");
#endif /* TM_ERROR_CHECKING */
        }
#endif /* TM_IP_SRC_ERROR_CHECKING */
#endif  /* ( (defined(TM_SNMP_MIB) || defined(TM_ERROR_CHECKING))  */
    }
#if ( (defined(TM_SNMP_MIB) || defined(TM_ERROR_CHECKING)) )
    else
    {
#ifdef TM_SNMP_MIB
/* Problem in IP header */
        tm_context(tvIpData).ipInHdrErrors++;
#ifdef TM_USE_NG_MIB2
        devPtr->dev4Ipv4Mib.ipIfStatsInHdrErrors++;
        tm_context(tvDevIpv4Data).ipIfStatsInHdrErrors++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfIpIncomingPacket", "Bad IP header");
#endif /* TM_ERROR_CHECKING */
    }
#endif  /* ( (defined(TM_SNMP_MIB) || defined(TM_ERROR_CHECKING))  */
ipIncomingExit:
    if (needFreePacket != TM_8BIT_ZERO)
    {
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    }
    return;
#undef lOptSR
#undef lIpFlags
}

/*
 * tfIpGetNextOpt Function Description
 * Get the next IP option, updating the length of all options that have not
 * been processed yet, updating the option value, and either updating the option
 * length if there is no error, or updating the ICMP error parameter if we find
 * an error in the option length field.
 * Parameters
 * Parameter     Description
 * optPtr        Pointer to start of the option search
 * optDatalenPtr Pointer to length of options left to be processed. Updated
 *               upon return.
 * optValPtr     Pointer to the found option value.
 * optLenPtr     Pointer to the found option length.
 * icmpParaPtr   Pointer to ICMP parameter error set when we detect an error.
 * ipHdrLen      Length of the IP header including the option (byte count).
 *               Used to compute the ICMP parameter in case of an error.
 * RETURN
 * Value         Meaning
 * optPtr        Pointer to the option that we found.
 */
tt8BitPtr tfIpGetNextOpt (tt8BitPtr optPtr,
                          tt8BitPtr optDataLenPtr,
                          tt8BitPtr optValPtr,
                          tt8BitPtr optLenPtr,
                          tt8BitPtr icmpParamPtr,
                          tt8Bit    ipHdrLen)
{
    tt8Bit optDataLen;
    tt8Bit optVal;
    tt8Bit optLen;

    for (optDataLen = *optDataLenPtr;
         optDataLen != (tt8Bit)0;
         optPtr += optLen )
    {
        optVal = tm_get_ip_opt(optPtr, TM_IP_OPT_VAL);

/* Only keep the option number. Mask off class and copy flag. */
        optVal = (tt8Bit)(optVal & 0x1F);
        *optValPtr = optVal;
        if (optVal == TM_IP_OPT_EOL)
        {
            *optLenPtr = optDataLen;
            optDataLen = 0;
            break;
        }
        if (optVal == TM_IP_OPT_NOP)
        {
            optLen = 1;
            optDataLen--;
        }
        else
        {
/* Length of the option */
            optLen = tm_get_ip_opt(optPtr, TM_IP_OPT_LEN);
            *optLenPtr = optLen;
            if (    (optLen <= (tt8Bit)TM_IP_OPT_LEN)
                 || (optLen > optDataLen) )
            {
/* Invalid option length */
/* parameter error pointer: IP header + offset to this option length */
                *icmpParamPtr = (tt8Bit)( ipHdrLen
                                         + TM_IP_OPT_OFFSET
                                         - optDataLen);
                break;
            }
            optDataLen = (tt8Bit)(optDataLen - optLen);
            break;
        }
    }
    *optDataLenPtr = optDataLen;
    return optPtr;
}

/*
 * tfIpForwardPacket() Function Description
 * 1) If routing option, get the destination Ip address (destIpAddress)
 *  from the routing option, otherwise the destination Ip address
 *  (destIpAddress) is the destination Ip address from the Ip header.
 * 2) We first lookup a forward routing cache entry (that contains an IP
 *  destination address and a routing table pointer). If the cache is
 *  empty, or the routing pointer is no longer valid (route not up) or if
 *  the destination address (destIpAddress) does not match the one in the
 *  cache entry (in the later 2 cases we release ownership of the routing
 *  entry), we try and fill in the routing cache by looking up the IP
 *  destination address (destIpAddress) in the routing table. If we fail to
 *  find a match, we send an ICMP destination unreachable for host message
 *  back to the source. (Note that it makes sense to cache the last
 *  forward routing entry since it is very likely that successive
 *  packets will be for the same destination.)
 * 3) We then process the routing IP options if any: we send an ICMP bad
 *  source error if we have strict source route and the next gateway is not
 *  directly accessible; we update the source route/record route fields in
 *  the IP header.
 * 4) Chek that forwarding is enabled on outbound interface and
 *  that we are not forwarding on the same interface the device came from,
 *  unless the outbound device is LAN (to allow forwarding on multiple
 *  subnets LAN networks (multihoming)).
 * 5) Check that IP TTL is bigger than its minimum value, in wich case
 *  we decrement the TTL, otherwise we send an ICMP time expired
 *  message back to the sender.
 * 6) If we updated any options, compute the IP checksum
 *  otherwise update the IP checksum (because of the TTL change).
 * 7) Duplicate the fist ttpacket/ttSharedData (for ICMP error
 *  messages)
 * 8) We call tfIpSendPacket, passing the packetPtr and pointer to the
 *  forward cache. pktFlags contains the IP flags
 *  as initialized in ipIncomingPacket, pktRtePtr contains the pointer
 *  to the route, pktIpHdrLen contains the IP header length.
 * 9) We check for errors and send ICMP error message back to
 *  the source if there was an error, otherwise we just free the duplicated
 *  ttPacket.
 * Parameter       Description
 * packetPtr         A pointer to a packet (ttPacket*) (with
 *                   packetPtr->pktLinkDataPtr pointing to the IP header in
 *                   ttSharedData
 *                   packetPtr->pktChainDataLength containing the IP packet
 *                   length and packetPtr->pktDeviceEntryPtr pointing to
 *                   the incoming device entry)
 * optFlag           Indicates whether there are route/record route options
 * sRouteIpAddrPtr   Pointer to the Ip address of the gateway we need
 *                   to send the source route packet to if there is route
 *                   option
 * rRouteIpAddrPtr   Pointer to the entry in the option where we need to
 *                   store the Ip address of the exiting interface
 *
 * Returns
 * Value   Meaning
 * NONE
 */
#ifndef TM_SINGLE_INTERFACE_HOME
static void tfIpForwardPacket( ttPacketPtr      packetPtr,
                               tt8Bit           optFlag,
                               tt4IpAddressPtr  sRouteIpAddrPtr,
                               tt4IpAddressPtr  rRouteIpAddrPtr )
{
    tt4IpAddress            ipAddr;
    ttIpHeaderPtr           iphPtr;
    ttRteEntryPtr           rtePtr;
    ttDeviceEntryPtr        outDevPtr;
    ttPacketPtr             icmpPacketPtr;
    ttUserIpForwCBFuncPtr   ipForwCBFuncPtr;
#ifdef TM_USE_IPSEC
    ttPktLen                ipsecMtuAdjust;
    int                     errorCode;
#endif /* TM_USE_IPSEC */
    unsigned                icmpLength;
    int                     status;
    tt16Bit                 mhomeIndex;
    tt8Bit                  icmpType;
    tt8Bit                  icmpCode;
    tt8Bit                  needUnlock;
/* 8 Bytes of saved data for ICMP error packet */
    tt8Bit                  savedIcmpError[TM_4PAK_ICMP_MIN_LEN];
/* saved IP header for ICMP error packet */
    tt8Bit                  savedIpError[TM_4PAK_IP_MIN_HDR_LEN];
    tt8Bit                  needRestoreForIcmp;

    needRestoreForIcmp = 0;

/* Point to IP header */
    iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
    outDevPtr = (ttDeviceEntryPtr)0;
    needUnlock = TM_8BIT_YES;
    icmpType = TM_ENOERROR; /* to avoid compiler warning */
    icmpCode = TM_ENOERROR; /* to avoid compiler warning */
    icmpLength = TM_4PAK_ICMP_MIN_LEN; /* to avoid compiler warning */
    status = TM_ENOERROR;
#ifdef TM_USE_IPSEC
    ipsecMtuAdjust = (ttPktLen) 0;
#endif /* TM_USE_IPSEC */

#ifdef TM_USE_IPSEC
    if (tm_context(tvIpsecPtr))
    {
/* clear all previous IPsec level setting */
        if (tm_context(tvIpsecFunc).ipsecIkeBypass == 0)
        {
            packetPtr->pktIpsecLevel = 0;
        }
/* all forwarded packet will be policy checked, including IKE packet */
        errorCode = tm_context(tvIpsecFunc).ipsecIncomingPolicyCheckFuncPtr(
                                                            packetPtr,
                                                            iphPtr->iphUlp );
        if( errorCode != TM_ENOERROR )
        {
            tm_debug_log2(
                "tfIpForwardPacket: failed incoming policy check "
                "%lx:%lx:%lx:%lx -> %lx:%lx:%lx:%lx",
                ntohl(iphPtr->iphSrcAddr),
                ntohl(iphPtr->iphDestAddr));

            tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
            needUnlock = TM_8BIT_ZERO;
            goto forwExit;
        }
    }
#endif /* TM_USE_IPSEC*/
/*
 * If routing option, get the destination Ip address (ipAddr)
 * from the routing option, otherwise the destination Ip address
 * (destIpAddress) is the destination Ip address from the Ip header.
 */
    if ( tm_8bit_one_bit_set(optFlag, TM_OPT_SSR_FLAG|TM_OPT_LSR_FLAG) )
    {
        tm_ip_ptr_u_copy(sRouteIpAddrPtr, ipAddr);
    }
    else
    {
        tm_ip_copy(iphPtr->iphDestAddr, ipAddr);
    }
/*
 * Lookup the forward routing cache entry (that contains an IP destination
 * and a routing table pointer). Check if the the cache is empty, or the
 * routing pointer is no longer valid, or if the destination address
 * (ipAddr) does not match the one in the cache entry.
 */
    tm_lock_wait(&tm_context(tvRtIpForwCacheLock));
    rtePtr = tm_context(tvRtIpForwCache).rtcRtePtr;
    if (    (rtePtr == TM_RTE_NULL_PTR)
         || tm_16bit_bits_not_set(rtePtr->rteFlags2, TM_RTE2_UP)
         || (
#ifdef TM_USE_IPV6
             !tm_ip_match(
                 tm_context(tvRtIpForwCache).rtcDestIpAddr.s6LAddr[3], ipAddr)
#else /* ! TM_USE_IPV6 */
             !tm_ip_match(tm_context(tvRtIpForwCache).rtcDestIpAddr, ipAddr)
#endif /* ! TM_USE_IPV6 */
             ) )
    {
/* Get and cache the routing entry corresponding to ipAddr in rtcPtr */
#ifdef TM_USE_STRONG_ESL
        tm_context(tvRtIpForwCache).rtcDevPtr = (ttDeviceEntryPtr)0;
#endif /* TM_USE_STRONG_ESL */
        status = tfIpCacheRte(&tm_context(tvRtIpForwCache), ipAddr);
        if (status == TM_ENOERROR)
        {
/* Valid routing entry */
            rtePtr = tm_context(tvRtIpForwCache).rtcRtePtr;
        }
/* if IPSEC is used, no matter we find the route or not, we are going to
 * call tfIpSendPacket. Because IPsec policy may require tunnel header.
 * We should use the tunnel terminal to look for route entry anyway
 */
        else
#ifdef TM_USE_IPSEC
        if (!(tm_context(tvIpsecPtr)))
#endif /* TM_USE_IPSEC */
        {

/*
 * If we fail to find a match (status != TM_ENOEROR), and the user
 * did not register a user IP forward call back function, we send
 * an ICMP destination unreachable for host message back to the source.
 * If the user registered an IP forward call back function, we call
 * the user IP forward call back function to let the user
 * know that an attempt to reach a given destination address has failed.
 * If the user wants to fix the problem (by starting an on-demand
 * configuration on another interface), then he returns TM_ENOERROR, and
 * we silently drop the packet.
 * If the user wants us to proceed and let the source of the packet know
 * that the packet could not be forwarded, he returns an error, and
 * we send an ICMP destination unreachable for host message back to
 * the source.
 */
            if (tm_context(tvIpForwCBFuncPtr) != (ttUserIpForwCBFuncPtr)0)
            {
                ipForwCBFuncPtr = tm_context(tvIpForwCBFuncPtr);
/* Unlock the route IP forward cache before calling the user */
                tm_call_unlock(&tm_context(tvRtIpForwCacheLock));
                needUnlock = TM_8BIT_ZERO;
                status = (*ipForwCBFuncPtr)(iphPtr->iphSrcAddr, ipAddr);
                if (status == TM_ENOERROR)
                {
/* Drop the packet. No ICMP error message */
                    goto forwNoRoute;
                }
            }
            icmpType = TM_ICMP_TYPE_UNREACH;
            icmpCode = TM_ICMP_CODE_HOST;
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
            packetPtr->pktDeviceEntryPtr->dev4Ipv4Mib.ipIfStatsInNoRoutes++;
            tm_context(tvDevIpv4Data).ipIfStatsInNoRoutes++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
            goto forwExit; /* ICMP error and return */
        }
#ifdef TM_USE_IPSEC
/* we enter here because we didn't find a proper route entry for our forwarding
 * packet. Since there may be an IPsec tunnel required, we postphone the route
 * entry locating until we fill the tunnel header.
 */
        else
        {
            tm_16bit_clr_bit( packetPtr->pktFlags,
                              (  TM_IP_RTE_FLAG
                               | TM_OUTPUT_DEVICE_FLAG
                               | TM_IP_DEST_ROUTE_SET) );
/* if error happens when sending this packet, we don't have to do anything
 */
            icmpPacketPtr = (ttPacketPtr)0;

            tm_debug_log2(
                "tfIpForwardPacket: IPSEC_SEND_NO_ROUTE "
                "%lx:%lx:%lx:%lx -> %lx:%lx:%lx:%lx",
                ntohl(iphPtr->iphSrcAddr),
                ntohl(iphPtr->iphDestAddr));

            goto IPSEC_SEND_NO_ROUTE;
        }
#endif /* TM_USE_IPSEC */
    }

    status = TM_ICMP_ERROR;
    if (optFlag != TM_8BIT_ZERO)
    {
/* Process the routing IP options */
        if (    tm_8bit_one_bit_set(optFlag, TM_OPT_SSR_FLAG)
             && tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_INDIRECT) )
        {
/*
 * We send an ICMP bad source error if we have strict source route and the
 * next gateway is not directly accessible.
 */
            icmpType = TM_ICMP_TYPE_UNREACH;
            icmpType = TM_ICMP_CODE_SRC;
            goto forwExit; /* ICMP error and return */
        }
/* We update the record route/source route fields. */
        tm_ip_copy(tm_ip_rte_dev_addr(rtePtr), ipAddr);
        if ( tm_8bit_one_bit_set(optFlag, TM_OPT_SSR_FLAG|TM_OPT_LSR_FLAG) )
        {
            tm_ip_u_ptr_copy(ipAddr, sRouteIpAddrPtr);
        }
        if ( tm_8bit_one_bit_set(optFlag, TM_OPT_RR_FLAG) )
        {
            tm_ip_u_ptr_copy(ipAddr, rRouteIpAddrPtr);
        }
    }
/*
 * We then check that forwarding is permitted on the outgoing
 * interface.
 */
    outDevPtr = rtePtr->rteDevPtr;
    mhomeIndex = rtePtr->rteMhomeIndex;
#ifdef TM_ERROR_CHECKING
    if (outDevPtr == TM_DEV_NULL_PTR)
    {
        tfKernelError("tfIpForwardPacket",
                      "Null device pointer in routing entry");
        tm_thread_stop;
    }
#endif /* TM_ERROR_CHECKING */
/*
 * If forwarding is enabled on outbound interface and
 * if we are not forwarding on the same interface the device came from,
 * unless the outbound device is LAN (to allow forwarding on multiple subnets
 * LAN networks (multihoming))
 * or the application has explicitly allowed reflected forwarding.
 */
    if (    tm_4_dev_forward(outDevPtr)
         && (    (outDevPtr != packetPtr->pktDeviceEntryPtr)
              || tm_4_ll_is_lan(outDevPtr)
              || tm_dev_forward_reflect(outDevPtr) )
#ifdef TM_USE_AUTO_IP
         && (   !tm_ip_is_local(iphPtr->iphSrcAddr)
              || tm_nat_public(outDevPtr)
            )
         && (    !tm_ip_is_local(ipAddr)
              || tm_nat_public(packetPtr->pktDeviceEntryPtr)
            )
#endif /* TM_USE_AUTO_IP */
       )
    {
/* Check that TTL > its minimum value */
        if (iphPtr->iphTtl > TM_IP_MIN_FORW_TTL)
        {
            iphPtr->iphTtl--; /* decrement the TTL */
/*
 * Checksum: If we updated any options, compute the IP checksum
 *  otherwise update the IP checksum (because of the TTL change).
 */
            if (optFlag)
            {
/* Zero before we recompute our checksum */
                iphPtr->iphChecksum = (tt16Bit)0;
                iphPtr->iphChecksum = tfPacketChecksum(
                    packetPtr,
                    (ttPktLen) (tm_byte_count(packetPtr->pktIpHdrLen)),
                    TM_16BIT_NULL_PTR,
                    TM_16BIT_ZERO);

            }
            else
            {
                iphPtr->iphChecksum = tfIpUpdateChecksum(iphPtr->iphChecksum,
                                              (tt32Bit)tm_const_htons(0x100));
            }
/*
 *  Duplicate the fist ttpacket/ttSharedData (for ICMP error
 *  messages)
 */
            icmpPacketPtr = tfDuplicateSharedBuffer(packetPtr);
/* Save data for ICMP error message */
            icmpLength = packetPtr->pktLinkDataLength - packetPtr->pktIpHdrLen;
            if (icmpLength > TM_4PAK_ICMP_MIN_LEN)
            {
                icmpLength = (unsigned)TM_4PAK_ICMP_MIN_LEN;
            }
            tm_bcopy(packetPtr->pktLinkDataPtr + packetPtr->pktIpHdrLen,
                     &savedIcmpError[0], icmpLength);
/* Save IP header for ICMP error message */
            tm_bcopy(packetPtr->pktLinkDataPtr,
                     &savedIpError[0],
                     TM_4PAK_IP_MIN_HDR_LEN);
            needRestoreForIcmp = 1;
/*
 * REVNOTE:for next release: check if we need to send a redirect here
 * (Note that RFC1122 warns against sending redirect with multi-homing
 */
/*
 * We call tfIpSendPacket(packetPtr, &tvRtIpForwCacheLock). pktFlags
 * contains the IP flags as initialized in ipIncomingPacket, pktRtePtr
 * contains the pointer to the route, pktIpHdrLen contains the IP header
 * length.
 */
            packetPtr->pktDeviceEntryPtr = outDevPtr;
            packetPtr->pktMhomeIndex = mhomeIndex;
            packetPtr->pktSharedDataPtr->dataRtePtr = rtePtr;
            packetPtr->pktSharedDataPtr->dataRteClonedPtr =
                                 tm_context(tvRtIpForwCache).rtcRteClonedPtr;
            packetPtr->pktFlags |= TM_IP_RTE_FLAG|TM_OUTPUT_DEVICE_FLAG;
#ifdef TM_USE_IPSEC
IPSEC_SEND_NO_ROUTE:
/* store the corresponding cache information, in case this packet is
 * queued by IKE
 */
            packetPtr->pktRouteCachePtr = &tm_context(tvRtIpForwCache);
#endif /* TM_USE_IPSEC */

            status = tfIpSendPacket(
                packetPtr,
#ifdef TM_LOCK_NEEDED
                &tm_context(tvRtIpForwCacheLock)
#else /* !TM_LOCK_NEEDED */
                (ttLockEntryPtr)0
#endif /* !TM_LOCK_NEEDED */
#ifdef TM_USE_IPSEC
                , &ipsecMtuAdjust
#ifdef TM_USE_IPSEC_TASK
                , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
#endif /* TM_USE_IPSEC */
                );
/* Cache unlocked in tfIpSendPacket */
            needUnlock = TM_8BIT_ZERO;
            if (icmpPacketPtr == TM_PACKET_NULL_PTR)
            {
                status = TM_ENOERROR;
                goto forwExit; /* just return */
            }

#ifdef TM_DEBUG_LOGGING
            if (status != 0)
            {
                tm_debug_log1(
                    "tfIpForwardPacket: tfIpSendPacket failed, ret=%d",
                    status);
            }
#endif /* TM_DEBUG_LOGGING */

/*
 * We check for errors and send ICMP error message back to
 * the source if there was an error, otherwise we just free the duplicated
 * ttPacket.
 */
            packetPtr = icmpPacketPtr;
            switch (status)
            {
                case 0:
#ifdef TM_USE_IPSEC
                case TM_IPSEC_PACKET_QUEUED:
#endif /* TM_USE_IPSEC */
                    status = TM_ENOERROR;
                    tfFreeSharedBuffer(packetPtr, TM_SOCKET_UNLOCKED);
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
/* count the number of packets we successfully forwarded */
                    tm_64Bit_incr(outDevPtr->dev4Ipv4Mib.ipIfStatsHCOutForwDatagrams);
                    tm_64Bit_incr(tm_context(tvDevIpv4Data).ipIfStatsHCOutForwDatagrams);
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
                    break;
                case TM_EMSGSIZE:
                    icmpType = TM_ICMP_TYPE_UNREACH;
                    icmpCode = TM_ICMP_CODE_FRAG;
                    break;
                case TM_ENOBUFS:
                case TM_ENOMEM:
                    icmpType = TM_ICMP_TYPE_SRC_QNCH;
                    icmpCode = 0;
                    break;
                default:
                    icmpType = TM_ICMP_TYPE_UNREACH;
                    icmpCode = TM_ICMP_CODE_HOST;
                    break;
            }
        }
        else
        {
/* If TTL > 1 we send an ICMP time expired message back to the sender. */
            icmpType = TM_ICMP_TYPE_TIME_XCEED;
            icmpCode = TM_ICMP_CODE_XCEED_TRANS;
        }
    }
    else
    {
/*
 * Route not allowed on outgoing interface, or packet is being forwarded
 * back on the same interface for a point to point connection. Do not
 * send an ICMP error message, and free the packet.
 */
forwNoRoute:
#ifdef TM_SNMP_MIB
        tm_context(tvIpData).ipOutNoRoutes++;
#endif /* TM_SNMP_MIB */
        tm_debug_log2(
            "tfIpForwardPacket: forwNoRoute "
            "%lx:%lx:%lx:%lx -> %lx:%lx:%lx:%lx",
            ntohl(iphPtr->iphSrcAddr),
            ntohl(iphPtr->iphDestAddr));
        status = TM_ENOERROR;
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    }
forwExit:
    if (needUnlock != TM_8BIT_ZERO)
    {
        tm_unlock(&tm_context(tvRtIpForwCacheLock));
    }
    if (status != TM_ENOERROR)
    {
        if (needRestoreForIcmp)
        {
/* Restore data for ICMP error message */
            tm_bcopy( &savedIcmpError[0],
                      packetPtr->pktLinkDataPtr + packetPtr->pktIpHdrLen,
                      icmpLength);
/* Restore IP header for ICMP error message */
            tm_bcopy( &savedIpError[0],
                      packetPtr->pktLinkDataPtr,
                      TM_4PAK_IP_MIN_HDR_LEN);
        }
        tfIcmpErrPacket(packetPtr,
                        icmpType,
                        icmpCode,
#ifdef TM_USE_IPSEC
                        (tt32Bit) ipsecMtuAdjust,
#else /* ! TM_USE_IPSEC */
                        TM_32BIT_ZERO, /* unused */
#endif /* ! TM_USE_IPSEC */
                        outDevPtr,
/*
 * REVNOTE: when we do send ICMP redirect, make this a variable
 * initialized to TM_IP_ZERO, that can be set for ICMP redirect
 */
                        TM_IP_ZERO);
    }
    return;
}
#endif /* TM_SINGLE_INTERFACE_HOME */


/*
 * 20 bytes IP header length (most common case)
 */
tt16Bit tfIpHdr5Checksum(tt16BitPtr iphPtr)
{
    tt32Bit sum;

/*  Unroll 10 words  */
#ifdef TM_32BIT_DSP
    sum  = (iphPtr[0] >> 16) & 0xffff; sum += iphPtr[0] & 0xffff;
    sum += (iphPtr[1] >> 16) & 0xffff; sum += iphPtr[1] & 0xffff;
    sum += (iphPtr[2] >> 16) & 0xffff; sum += iphPtr[2] & 0xffff;
    sum += (iphPtr[3] >> 16) & 0xffff; sum += iphPtr[3] & 0xffff;
    sum += (iphPtr[4] >> 16) & 0xffff; sum += iphPtr[4] & 0xffff;
#else /* !TM_32BIT_DSP */
/* Should work OK for 16-bit DSP. */
    sum = iphPtr[0];
    sum += iphPtr[1];
    sum += iphPtr[2];
    sum += iphPtr[3];
    sum += iphPtr[4];
    sum += iphPtr[5];
    sum += iphPtr[6];
    sum += iphPtr[7];
    sum += iphPtr[8];
    sum += iphPtr[9];
#endif /* TM_32BIT_DSP */

/*  Fold 32-bit sum to 16 bits, adding posssible carries from fold */
    tm_add_carries_sum(sum);
#ifdef TM_32BIT_DSP
    return (tt16Bit)(~sum) & 0xffff;
#else /* !TM_32BIT_DSP */
    return (tt16Bit)~sum;
#endif /* TM_32BIT_DSP */
}

/*
 * Cache the routing entry corresponding to ipDestAddr in rtcPtr, after
 * having first emptied it, by releasing ownership to any route entry the
 * cache was pointing to.
 */
int tfIpCacheRte(
    ttRtCacheEntryPtr rtcPtr,
    tt4IpAddress      ipDestAddr
    )
{
    int             status;

    tfRtCacheUnGet(rtcPtr);
/*
 * Empty cache (rtcRtePtr is null): we try and fill in the routing cache
 * by looking up the IP destination address (destIpAddress) in the routing
 * table.
 */
#ifdef TM_USE_IPV6
    tm_6_addr_to_ipv4_mapped( (tt4IpAddress)(ipDestAddr),
                              &(rtcPtr->rtcDestIpAddr) );
#else /* ! TM_USE_IPV6 */
    tm_ip_copy(ipDestAddr, rtcPtr->rtcDestIpAddr);
#endif /* ! TM_USE_IPV6 */
    status = tfRtGet(rtcPtr);
    return status;
}


/*
 * If IP address is broadcast or multicast, map IP broadcast or IP multicast
 * address to Ethernet broadcast or multicast address.
 * Return value     meaning
 * TM_ETHER_UCAST   Unicast address
 * TM_ETHER_BCAST   Broadcast addrress
 * TM_ETHER_MULTI   Multicast address
 */
int tfIpBroadMcastToLan (tt4IpAddress ipAddr, ttDeviceEntryPtr devPtr,
                         tt16Bit mhomeIndex, tt8BitPtr etherAddrPtr)
{
#ifndef TM_DSP
    tt8BitPtr       tempIpPtr;
#endif /* TM_DSP */
    int             retCode;

#ifdef TM_SINGLE_INTERFACE_HOME
    TM_UNREF_IN_ARG(mhomeIndex);
#endif /* TM_SINGLE_INTERFACE_HOME */
    retCode = TM_ETHER_UCAST;
/* When destination is directed broadcast, initialize broadcast enet address */
    if (    tm_ip_match(ipAddr, TM_IP_LIMITED_BROADCAST)
         || tm_ip_match(ipAddr, tm_ip_dev_dbroad_addr(devPtr, mhomeIndex) )
         || (tm_ip_is_class_broadcast(ipAddr))
       )
    {
sendBroad:
        retCode = TM_ETHER_BCAST;
        tm_ether_copy(tvEtherBroadcast.enuAddress, etherAddrPtr);
    }
    else
    {
        if (tm_ip_is_multicast(ipAddr))
        {
            if (tm_dev_mcast_enb(devPtr))
            {
#ifdef TM_DSP

                etherAddrPtr[0] = 0x01;
                etherAddrPtr[1] = 0x00;
                etherAddrPtr[2] = 0x5e;
                etherAddrPtr[3] =
                    (tt8Bit)((ipAddr >> 16)&((tt8Bit)0x7F));
                etherAddrPtr[4] =
                    (tt8Bit)((ipAddr >>  8)&((tt8Bit)0xFF));
                etherAddrPtr[5] =
                    (tt8Bit)((ipAddr >>  0)&((tt8Bit)0xFF));
                retCode = TM_ETHER_MULTI;

#else /* !TM_DSP */

                tempIpPtr = (tt8BitPtr)&ipAddr;
                etherAddrPtr[0] = 0x01;
                etherAddrPtr[1] = 0x00;
                etherAddrPtr[2] = 0x5e;
                etherAddrPtr[3] = (tt8Bit)((tempIpPtr[1])&((tt8Bit)0x7F));
                etherAddrPtr[4] = tempIpPtr[2];
                etherAddrPtr[5] = tempIpPtr[3];
                retCode = TM_ETHER_MULTI;

#endif /* TM_DSP */
            }
            else
            {
                goto sendBroad;
            }
        }
    }
    return retCode;
}

#ifdef TM_SINGLE_INTERFACE_HOME

ttDeviceEntryPtr tfIfaceConfig (tt16Bit flag)
{
    ttDeviceEntryPtr devPtr;

    tm_lock_wait(&tm_context(tvDeviceListLock));
    devPtr = tm_context(tvDeviceList);
    if (devPtr != TM_DEV_NULL_PTR)
    {
        if ( tm_8bit_bits_not_set( tm_ip_dev_conf_flag(devPtr, 0),
                                   TM_DEV_IP_CONFIG ) )
        {
/* Device not configured yet */
            if (    (flag != TM_IP_BROADCAST_FLAG)
                 || (tm_8bit_bits_not_set( tm_ip_dev_conf_flag(devPtr, 0),
                                           TM_DEV_IP_CONF_STARTED)) )
            {
/* Do not allow output, if not (broadcast and booting) */
                devPtr = TM_DEV_NULL_PTR;
            }
        }
    }
    tm_unlock(&tm_context(tvDeviceListLock));
    return devPtr;
}

tt4IpAddress tfGetRouterId (void)
{
    ttDeviceEntryPtr    devPtr;
    tt4IpAddress         routerId;

    devPtr = tfIfaceConfig(0);
    if (devPtr != TM_DEV_NULL_PTR)
    {
        tm_ip_copy(tm_ip_dev_addr(devPtr, 0), routerId);
    }
    else
    {
        tm_ip_copy(tm_context(tvRouterId), routerId);
    }
    return routerId;
}

#else /* !TM_SINGLE_INTERFACE_HOME */

/*
 * Function Description
 * Given a source IP address and a device pointer, find in that device entry
 * the ip address match.
 * Parameter       Description
 * devPtr          Pointer to the Device entry
 * ipAddress       IP address for match
 * ifaceMhomePtr   where to store the multihome index of the match
 * Returns
 * Value   Meaning
 * 0       success
 * non zero        error value
 */
int tfMhomeAddrMatch (ttDeviceEntryPtr   devPtr,
                     tt4IpAddress        ipAddress,
                     tt16BitPtr          ifaceMhomePtr)
{
    int     errorCode;
#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
    ttIpAddrEntryPtr ipAddrEntryPtr;
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */
    int     maxMhomeEntries;
    int     i;

    errorCode = TM_ENOENT;
#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
    if (tm_context(tv4SortedIpAddrCache).sipcIpAddrArray
        != TM_IP_ADDR_ENTRY_NULL_PTR)
    {
        ipAddrEntryPtr = tf4LookupSortedIpAddr(
            ipAddress, devPtr->devIndex, &(tm_context(tv4SortedIpAddrCache)),
            TM_8BIT_ZERO);
        if (ipAddrEntryPtr != TM_IP_ADDR_ENTRY_NULL_PTR)
        {
            *ifaceMhomePtr = (tt16Bit) ipAddrEntryPtr->mHomeIndex;
            errorCode = TM_ENOERROR;
        }
    }
    else
    {
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */
    maxMhomeEntries = (int) devPtr->devMhomeUsedEntries;
    for ( i = 0; i < maxMhomeEntries; i++ )
    {
        if (    tm_8bit_one_bit_set( tm_ip_dev_conf_flag(devPtr, i),
                                     TM_DEV_IP_CONFIG)
             && tm_ip_match(ipAddress, tm_ip_dev_addr(devPtr, i)) )
        {
            *ifaceMhomePtr = (tt16Bit)i;
            errorCode = TM_ENOERROR;
            break; /* out of the loop */
        }
    }
#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
    }
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */
    return errorCode;
}

/*
 * Function Description
 * Given a device pointer, find a valid IP address that does not match ipAddr
 * Parameter       Description
 * devPtr          Pointer to the Device entry
 * ifaceMhomePtr   where to store the multihome index of the match
 * Returns
 * Value   Meaning
 * 0       success
 * non zero        error value
 */
static int tfMhomeNoMatchAddr(ttDeviceEntryPtr   devPtr,
                              tt4IpAddress       ipAddr,
                              tt16BitPtr         ifaceMhomePtr)
{
    int     errorCode;
    int     maxMhomeEntries;
    int     i;

    errorCode = TM_ENOENT;
    maxMhomeEntries = (int) devPtr->devMhomeUsedEntries;
    for ( i = 0; i < maxMhomeEntries; i++ )
    {
        if (    tm_8bit_one_bit_set( tm_ip_dev_conf_flag(devPtr, i),
                                     TM_DEV_IP_CONFIG)
             && !tm_ip_match(tm_ip_dev_addr(devPtr, i), ipAddr) )
        {
            *ifaceMhomePtr = (tt16Bit)i;
            errorCode = TM_ENOERROR;
            break; /* out of the loop */
        }
    }
    return errorCode;
}

/*
 * Function Description
 * Given a device pointer, find a configured multi home
 * Parameter      Description
 * devPtr         Pointer to the Device entry
 * ifaceMhomePtr  INPUT: bit mask that indicates the criteria for the match
 *                      (configured, or in the process of being configured)
 *                OUTPUT: where to store the multihome index of the match
 * Returns
 * Value   Meaning
 * 0       success
 * non zero        error value
 */
int tfMhomeAnyConf (ttDeviceEntryPtr   devPtr,
                    tt4IpAddress       ipAddr,
                    tt16BitPtr         ifaceMhomePtr)
{
    int     errorCode;
    int     maxMhomeEntries;
    int     i;
    tt8Bit  testBits;

    TM_UNREF_IN_ARG(ipAddr); /* Unused parameter (so the compiler will not complain) */
    errorCode = TM_ENOENT;
    if (devPtr == tm_context(tvLoopbackDevPtr))
    {
/* Do not want to return loop back interface */
        goto mhomeAnyConfExit; /* return */
    }
/*
 * If   testBits == TM_DEV_IP_CONFIG, look for a configured IP address
 * else (testBits == TM_DEV_IP_CONF_STARTED), look for a mhome in the
 *       process of being configured.
 */
    testBits = (tt8Bit)(*ifaceMhomePtr);
    if (testBits == TM_DEV_IP_CONFIG)
    {
/* Use maximum configured multi home */
        maxMhomeEntries = (int)devPtr->devMhomeUsedEntries;
    }
    else
    {
/* If no mhome is configured yet, use compile time maximum */
        maxMhomeEntries = TM_MAX_IPS_PER_IF;
    }
    for ( i = 0; i < maxMhomeEntries; i++ )
    {
        if (tm_8bit_all_bits_set( tm_ip_dev_conf_flag(devPtr, i),
                                  testBits))
        {
/* Configured/booting address*/
            *ifaceMhomePtr = (tt16Bit)i;
            errorCode = TM_ENOERROR;
            goto mhomeAnyConfExit; /* return */
        }
    }
mhomeAnyConfExit:
    return errorCode;
}

/*
 * Function Description
 * Given an IP address and a device pointer, find in that device entry
 * the network match.
 * Parameter       Description
 * devPtr          Pointer to the Device entry
 * ipAddress       IP address for network match
 * ifaceMhomePtr   where to store the multihome index of the match
 * Returns
 * Value   Meaning
 * 0       success
 * non zero        error value
 */
int tfMhomeNetMatch (ttDeviceEntryPtr   devPtr,
                     tt4IpAddress       ipAddress,
                     tt16BitPtr         ifaceMhomePtr)
{
    int     errorCode;
    int     isPt2Pt;
    int     maxMhomeEntries;
    int     i;

    errorCode = TM_ENOENT;
    maxMhomeEntries = (int) devPtr->devMhomeUsedEntries;
    isPt2Pt = tm_ll_is_pt2pt(devPtr);
    for ( i = 0; i < maxMhomeEntries; i++ )
    {
        if ( tm_8bit_one_bit_set( tm_ip_dev_conf_flag(devPtr, i),
                                  TM_DEV_IP_CONFIG) )
        {
            if (   (   !isPt2Pt
                    && tm_ip_not_zero(tm_ip_dev_mask(devPtr, i))
                    && tm_ip_net_match(ipAddress,
                                       tm_ip_dev_addr(devPtr, i),
                                       tm_ip_dev_mask(devPtr, i)) )
                || (   isPt2Pt
                    && tm_ip_match(ipAddress,
                                    devPtr->devPt2PtPeerIpAddr) ) )
            {
                    *ifaceMhomePtr = (tt16Bit)i;
                    errorCode = TM_ENOERROR;
                    break; /* out of the loop */
            }
        }
    }
    return errorCode;
}

/*
 * Function Description
 * Given an IP address and a device pointer, find if network part
 * of ipAddress is zero.
 * Parameter       Description
 * devPtr          Pointer to the Device entry
 * ipAddress       IP address for network match
 * ifaceMhomePtr   where to store the multihome index of the match
 * Returns
 * Value   Meaning
 * 0       success
 * non zero        error value
 */
static int tfMhomeZeroNet(ttDeviceEntryPtr   devPtr,
                          tt4IpAddress       ipAddress,
                          tt16BitPtr         ifaceMhomePtr)
{
    int     errorCode;
    int     maxMhomeEntries;
    int     i;

    errorCode = TM_ENOENT;
    maxMhomeEntries = (int) devPtr->devMhomeUsedEntries;
    for ( i = 0; i < maxMhomeEntries; i++ )
    {
        if (    tm_8bit_one_bit_set( tm_ip_dev_conf_flag(devPtr, i),
                                     TM_DEV_IP_CONFIG)
             && tm_ip_zero( tm_ip_net( ipAddress,
                                       tm_ip_dev_mask(devPtr, i) ) ) )
        {
            *ifaceMhomePtr = (tt16Bit)i;
            errorCode = TM_ENOERROR;
            break; /* out of the loop */
        }
    }
    return errorCode;
}

/*
 * Function Description
 * Given a device, an IP address and a device pointer, find in that device
 * entry a directed broadcast match.
 * Parameter       Description
 * devPtr          Pointer to the Device entry
 * ipAddress       IP address for match
 * ifaceMhomePtr   where to store the multihome index for the match
 * Returns
 * Value   Meaning
 * 0       success
 * non zero        error value
 */
int tfMhomeDBroadMatch (ttDeviceEntryPtr   devPtr,
                        tt4IpAddress       ipAddress,
                        tt16BitPtr         ifaceMhomePtr)
{
    int     errorCode;
#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
    ttIpAddrEntryPtr ipAddrEntryPtr;
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */
    int     maxMhomeEntries;
    int     i;

    errorCode = TM_ENOENT;
    if (tm_4_ll_is_broadcast(devPtr))
    {
#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
        if (devPtr->devSortedDBroadIpAddrCache.sipcIpAddrArray
            != TM_IP_ADDR_ENTRY_NULL_PTR)
        {
            ipAddrEntryPtr = tf4LookupSortedIpAddr(
                ipAddress, devPtr->devIndex,
                &(devPtr->devSortedDBroadIpAddrCache),
                TM_8BIT_YES);
            if (ipAddrEntryPtr != TM_IP_ADDR_ENTRY_NULL_PTR)
            {
                *ifaceMhomePtr = (tt16Bit) ipAddrEntryPtr->mHomeIndex;
                errorCode = TM_ENOERROR;
            }
        }
        else
        {
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */
        maxMhomeEntries = (int) devPtr->devMhomeUsedEntries;
        for ( i = 0 ; i < maxMhomeEntries; i++ )
        {
            if (    tm_8bit_one_bit_set( tm_ip_dev_conf_flag(devPtr, i),
                                         TM_DEV_IP_CONFIG )
                    && tm_ip_match( ipAddress,
                                    tm_ip_dev_dbroad_addr(devPtr, i) ) )
            {
                *ifaceMhomePtr = (tt16Bit)i;
                errorCode = TM_ENOERROR;
                break; /* out of the loop */
            }
        }
#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
        }
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */
    }
    return errorCode;
}

/*
 * Function Description
 * Given a device, an IP address and a device pointer, find in that device
 * entry a class broadcast match.
 * Parameter       Description
 * devPtr          Pointer to the Device entry
 * ipAddress       IP address for match
 * ifaceMhomePtr   where to store the multihome index for the match
 * Returns
 * Value   Meaning
 * 0       success
 * non zero        error value
 */
static int tfMhomeClassBroadMatch (ttDeviceEntryPtr   devPtr,
                                   tt4IpAddress       ipAddress,
                                   tt16BitPtr         ifaceMhomePtr)
{
    int          maxMhomeEntries;
    int          errorCode;
    int          i;

    errorCode = TM_ENOENT;
    if (tm_ip_is_class_broadcast(ipAddress))
    {
        if (tm_4_ll_is_broadcast(devPtr))
        {
            maxMhomeEntries = (int) devPtr->devMhomeUsedEntries;
            for ( i = 0 ; i < maxMhomeEntries; i++ )
            {
                if (    tm_8bit_one_bit_set( tm_ip_dev_conf_flag(devPtr, i),
                                             TM_DEV_IP_CONFIG )
                     && (tm_ip_net_match(ipAddress,
                                         tm_ip_dev_addr(devPtr, i),
                                         tm_ip_class_net_mask(ipAddress))))
                {
                    *ifaceMhomePtr = (tt16Bit)i;
                    errorCode = TM_ENOERROR;
                    break; /* out of the loop */
                }
            }
        }
    }
    return errorCode;
}

/*
 * Function Description
 * Given an IP address, matching function find if any device has a matching
 * multihome entry
 * Parameter       Description
 * ipAddress       IP address for function match
 * ifaceMhomePtr   Where to store the matching multi-home index
 * Returns
 * Value                Meaning
 * device pointer       success
 * Null pointer         no match
 */
ttDeviceEntryPtr tfIfaceMatch (tt4IpAddress    ipAddress,
                               ttMhomeFunctPtr mhomeMatchFuncPtr,
                               tt16BitPtr      ifaceMhomePtr)
{
    ttDeviceEntryPtr    devPtr;
    int                 errorCode;
#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
    ttIpAddrEntryPtr ipAddrEntryPtr;
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */

    tm_lock_wait(&tm_context(tvDeviceListLock));
#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
    if ((mhomeMatchFuncPtr == tfMhomeAddrMatch)
        && (tm_context(tv4SortedIpAddrCache).sipcIpAddrArray
            != TM_IP_ADDR_ENTRY_NULL_PTR))
    {
/* special case: we are being asked to do a global search across all interfaces
   for a specific local IP address */
        ipAddrEntryPtr = tf4LookupSortedIpAddr(
            ipAddress, 0, &(tm_context(tv4SortedIpAddrCache)),
            TM_8BIT_ZERO);
        if (ipAddrEntryPtr != TM_IP_ADDR_ENTRY_NULL_PTR)
        {
            devPtr = tm_context(tvDevIndexToDevPtr)[ipAddrEntryPtr->devIndex];
            *ifaceMhomePtr = (tt16Bit) ipAddrEntryPtr->mHomeIndex;
        }
        else
        {
/* IP address was not found. */
            devPtr = TM_DEV_NULL_PTR;
        }
    }
    else
    {
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */
    devPtr = tm_context(tvDeviceList);
    while (devPtr != TM_DEV_NULL_PTR)
    {
        errorCode = (*mhomeMatchFuncPtr)(devPtr, ipAddress, ifaceMhomePtr);
        if( errorCode == TM_ENOERROR )
        {
            break;
        }
        devPtr = devPtr->devNextDeviceEntry;
    }
#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
    }
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */
    tm_unlock(&tm_context(tvDeviceListLock));
    return devPtr;
}


tt4IpAddress tfGetRouterId (void)
{
        ttDeviceEntryPtr    devPtr;
auto    tt16Bit             mHomeIndex;
        tt4IpAddress         routerId;

    devPtr = tm_iface_non_zero_addr(&mHomeIndex);
    if (devPtr != TM_DEV_NULL_PTR)
    {
        tm_ip_copy(tm_ip_dev_addr(devPtr, mHomeIndex), routerId);
    }
    else
    {
        tm_ip_copy(tm_context(tvRouterId), routerId);
    }
    return routerId;
}
#endif /* TM_SINGLE_INTERFACE_HOME */

/*
 * tfIpDestToPacket Function Description
 * Given a destination IP address, find the outgoing interface/route and
 * fill in the IP header Source and Destination IP addresses, and
 * ttl and tos.
 * 1. We verify that destination IP address is non zero.
 * 2. If destination address is multicast, get the interface corresponding
 *    to the source address if non zero, otherwise get the route
 *    from the socket routing cache/routing table and interface from the
 *    route.
 * 3. For limited broadcast, get the first configured interface.
 * 3. If not multicast or limited broadcast, get the route from the socket
 *    routing cache/routing table, and interface from the route.
 * 5. Use the source address parameter if non null, otherwise use the
 *    IP address from the interface and store in IP header.
 * 6. Initialize IP header TOS with passed parameter,
 *    packet interface/multihome index from values found above,
 *    IP header destination address from passed parameter.
 * Return 0 on success, error value on failure
 * Called by tfIcmpSendPacket(), tfUdpSendPacket(), tfTcpConnect(),
 *           with socket route cache.
 * Called by tfIpSendPacket() with IP route cache.
 * (Note that tfTcpConnect() will reject a broadcast/multicast
 *  address. Done in tfTcpConnect() to not impact UDP/ICMP send)
 */
int tfIpDestToPacket (ttPacketPtr       packetPtr,
#ifdef TM_4_USE_SCOPE_ID
                      tt6ConstIpAddressPtr srcV4MappedIpAddrPtr,
                      tt6ConstIpAddressPtr peerV4MappedIpAddrPtr,
#else /* ! TM_4_USE_SCOPE_ID */
                      tt4IpAddress      srcIpAddress,
                      tt4IpAddress      peerIpAddress,
#endif /* ! TM_4_USE_SCOPE_ID */
                      tt8Bit            ipTos,
                      ttRtCacheEntryPtr rtcPtr,
                      ttIpHeaderPtr     iphPtr)
{
    ttSocketEntryPtr  socketEntryPtr;
    ttRteEntryPtr     rtePtr;
    ttDeviceEntryPtr  devPtr;
    int               errorCode;
#if defined(TM_USE_STRONG_ESL) && !defined(TM_SINGLE_INTERFACE_HOME)
    int               retCode;
#endif /* TM_USE_STRONG_ESL && !TM_SINGLE_INTERFACE_HOME */
#ifdef TM_4_USE_SCOPE_ID
    tt4IpAddress      srcIpAddress;
    tt16Bit           srcDevIndex;
    tt4IpAddress      peerIpAddress;
    tt16Bit           peerDevIndex;
#endif /* TM_4_USE_SCOPE_ID */
    auto tt16Bit      mhomeIndex;

#ifdef TM_4_USE_SCOPE_ID
    tm_ip_copy(
        tm_6_ip_mapped_6to4(srcV4MappedIpAddrPtr), srcIpAddress);
    srcDevIndex = TM_16BIT_ZERO;
    if (IN4_IS_ADDR_LINKLOCAL((srcV4MappedIpAddrPtr)->s6LAddr[3]))
    {
        srcDevIndex = (tt16Bit) (srcV4MappedIpAddrPtr)->s6SAddr[4];
    }

    tm_ip_copy(
        tm_6_ip_mapped_6to4(peerV4MappedIpAddrPtr), peerIpAddress);
    peerDevIndex = TM_16BIT_ZERO;
    if (IN4_IS_ADDR_LINKLOCAL((peerV4MappedIpAddrPtr)->s6LAddr[3]))
    {
        peerDevIndex = (tt16Bit) (peerV4MappedIpAddrPtr)->s6SAddr[4];
    }
#endif /* TM_4_USE_SCOPE_ID */
    mhomeIndex = TM_16BIT_ZERO;
    errorCode = TM_ENOERROR;
    rtePtr = rtcPtr->rtcRtePtr;
    devPtr = packetPtr->pktDeviceEntryPtr;
    if (tm_ip_is_multicast(srcIpAddress))
    {
        tm_ip_copy(TM_IP_ZERO, srcIpAddress);
    }

#ifdef TM_USE_IPSEC
/* store the corresponding cache information, in case this packet is
 * queued by IKE
 */
    packetPtr->pktRouteCachePtr = rtcPtr;
#endif /* TM_USE_IPSEC */

#ifdef TM_ERROR_CHECKING
    if (   tm_16bit_one_bit_set(packetPtr->pktFlags, TM_OUTPUT_DEVICE_FLAG)
        && (devPtr == (ttDeviceEntryPtr)0) )
    {
        tfKernelError("tfIpDestToPacket", "corrupted output device flag");
    }
#endif /* TM_ERROR_CHECKING */

#ifdef TM_4_USE_SCOPE_ID
    if ((srcDevIndex != TM_16BIT_ZERO) && (peerDevIndex != TM_16BIT_ZERO)
        && (srcDevIndex != peerDevIndex))
    {
/* link-local scope source IP address isn't on same link as link-local
   scope destination IP address */
        errorCode = TM_EHOSTUNREACH;
        goto outIpDestToPacket;
    }
#endif /* TM_4_USE_SCOPE_ID */

    if ( tm_ip_is_multicast(peerIpAddress) )
    {
        packetPtr->pktFlags |= TM_IP_MULTICAST_FLAG;
    }

    if (   (rtePtr == TM_RTE_NULL_PTR)
        || tm_16bit_bits_not_set(rtePtr->rteFlags2, TM_RTE2_UP)
#ifdef TM_USE_STRONG_ESL
        || (    (rtcPtr->rtcDevPtr != (ttDeviceEntryPtr)0)
             && (rtePtr->rteOrigDevPtr != rtcPtr->rtcDevPtr) )
#endif /* TM_USE_STRONG_ESL */
        || (
#ifdef TM_USE_IPV6
            tm_ip_not_match(
                rtcPtr->rtcDestIpAddr.s6LAddr[3], peerIpAddress)
#else /* ! TM_USE_IPV6 */
            tm_ip_not_match(rtcPtr->rtcDestIpAddr, peerIpAddress)
#endif /* ! TM_USE_IPV6 */
            ) )
    {
        if (tm_ip_zero(peerIpAddress)) /* Do not allow a zero destination */
        {
            errorCode = TM_EDESTADDRREQ; /* Destination address required */
            goto outIpDestToPacket;
        }
        if ( tm_ip_is_multicast(peerIpAddress) )
        {
            if (tm_16bit_bits_not_set( packetPtr->pktFlags,
                                       TM_OUTPUT_DEVICE_FLAG) )
            {
                socketEntryPtr =
                            packetPtr->pktSharedDataPtr->dataSocketEntryPtr;
                if ( socketEntryPtr != (ttSocketEntryPtr)0 )
/* UDP/ICMP/IGMP only, but we only care about UDP multicasting anyways */
                {
                    devPtr = socketEntryPtr->socMcastDevPtr;
                    mhomeIndex = socketEntryPtr->socMcastMhomeIndex;
                }
                if (devPtr == (ttDeviceEntryPtr)0)
/* User did not use the IPO_MULTICAST_IF option */
                {
#ifdef TM_SINGLE_INTERFACE_HOME
/* return single interface, if configured */
                    devPtr = tfIfaceConfig(TM_IP_MULTICAST_FLAG);
                    if (devPtr == TM_DEV_NULL_PTR)
                    {
/* If no device configured, this will return TM_EHOSTUNREACH */
                        goto ipDestRoute;
                    }
                    mhomeIndex = TM_16BIT_ZERO;
#else /* !TM_SINGLE_INTERFACE_HOME */
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
#ifdef TM_USE_STRONG_ESL
                    devPtr = tfIpSocBindDevPtr(socketEntryPtr);
                    if (devPtr != TM_DEV_NULL_PTR)
/* socket bound to a device */
                    {
/* Make sure that there is an address configured on the device */
                        if (    (tm_ip_zero(srcIpAddress))
                             || (tfMhomeAddrMatch(devPtr,
                                             srcIpAddress,
                                             &mhomeIndex) != TM_ENOERROR) )
                        {
                            goto ipDestRoute;
                        }
                    }
                    else
/* User did not bind to a device */
#endif /* TM_USE_STRONG_ESL */
                    {
                        if (    (tm_ip_zero(srcIpAddress))
                             || (    (devPtr = tm_iface_addr_match(
                                                               srcIpAddress,
                                                               &mhomeIndex))
                                  == TM_DEV_NULL_PTR ) )
/* PRQA L:L1 */
/*
 * If source IP address is zero or there is no device corresponding to
 * source address. Find outgoing interface through routing table.
 */
                        {
                            goto ipDestRoute;
                        }
                    }
#endif /* TM_SINGLE_INTERFACE_HOME */
                    if (tm_dev_mcast_route(devPtr))
/*
 * Always use the routing table before sending, if user set
 * TM_DEV_IP_MCAST_ROUTE
 */
                    {
                        goto ipDestRoute;
                    }
                }
                packetPtr->pktFlags |= TM_OUTPUT_DEVICE_FLAG;
                packetPtr->pktDeviceEntryPtr = devPtr;
                packetPtr->pktMhomeIndex = mhomeIndex;
            }
        }
        else
        {
            if (   (tm_ip_match(peerIpAddress, TM_IP_LIMITED_BROADCAST))
                || (tm_ip_is_class_broadcast(peerIpAddress))
               )
            {
                if ( tm_16bit_bits_not_set( packetPtr->pktFlags,
                                            TM_OUTPUT_DEVICE_FLAG) )
/* REVNOTE: add check for OUTPUT_DEVICE_FLAG when we modify bind */
                {
#ifdef TM_SINGLE_INTERFACE_HOME
/* return single interface if it is configured, or booting */
                    devPtr = tfIfaceConfig(TM_IP_BROADCAST_FLAG);
                    mhomeIndex = TM_16BIT_ZERO;
                    if (!(tm_ip_match(peerIpAddress, TM_IP_LIMITED_BROADCAST)))
                    {
                        if (!tm_ip_class_broad_match(devPtr, peerIpAddress))
                        {
                            devPtr = (ttDeviceEntryPtr)0;
/*
 * Check for directed broadcast by cloning the directed broadcast in the
 * routing tree
 */
                            goto ipDestRoute;
                        }
                    }
#else /* TM_SINGLE_INTERFACE_HOME */
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
#ifdef TM_USE_STRONG_ESL
                    devPtr = tfIpSocBindDevPtr(
                            packetPtr->pktSharedDataPtr->dataSocketEntryPtr);
#endif /* TM_USE_STRONG_ESL */
                    if (!tm_ip_match(peerIpAddress, TM_IP_LIMITED_BROADCAST))
                    {
#ifdef TM_USE_STRONG_ESL
                        if (devPtr != TM_DEV_NULL_PTR)
/* User bound to a device */
                        {
                            retCode = tfMhomeClassBroadMatch(devPtr,
                                                             peerIpAddress,
                                                             &mhomeIndex);
                            if (retCode != TM_ENOERROR)
                            {
                                devPtr = (ttDeviceEntryPtr)0;
/*
 * Check for directed broadcast by cloning the directed broadcast in the
 * routing tree
 */
                                goto ipDestRoute;
                            }
                        }
                        else
/* User did not bind to a device */
#endif /* TM_USE_STRONG_ESL */
                        {
                            devPtr = tm_iface_class_broad_match(peerIpAddress,
                                                                &mhomeIndex);
                            if (devPtr == TM_DEV_NULL_PTR )
                            {
/*
 * Check for directed broadcast by cloning the directed broadcast in the
 * routing tree
 */
                                goto ipDestRoute;
                            }
                        }
                    }
                    else
                    {
#ifdef TM_USE_STRONG_ESL
                        if (devPtr != TM_DEV_NULL_PTR)
/* User bound to a device */
                        {
                            if (     tm_ip_zero(srcIpAddress)
                                 ||  (tfMhomeAddrMatch(devPtr,
                                                       srcIpAddress,
                                                       &mhomeIndex) !=
                                         TM_ENOERROR))
                            {
                                mhomeIndex = (tt16Bit)
                                            (TM_DEV_IP_CONFIG | TM_DEV_IP_CONF_STARTED);
                                retCode = tfIpAnyConfDevPtr(devPtr, &mhomeIndex);
                                if (retCode != TM_ENOERROR)
                                {
                                    devPtr = TM_DEV_NULL_PTR;
                                }
                            }
                        }
                        else
/* User did not bind to a device */
#endif /* TM_USE_STRONG_ESL */
                        {
                            if (   ( tm_ip_zero(srcIpAddress) )
/* Get device corresponding to source address */
                                || ( (devPtr = tm_iface_addr_match(srcIpAddress,
                                                                   &mhomeIndex))
                                     == TM_DEV_NULL_PTR ) )
/* PRQA L:L1 */
/*
 * If source IP address is zero or there is no device corresponding to
 * source address
 */
                            {
/*
 * Use first configured, or non PPP device in the process of being configured:
 * NOTE TM_DEV_IP_CONFIG, and then TM_DEV_IP_CONF_STARTED is INPUT parameter to
 * tm_iface_any_conf().
 * mhomeIndex will be set if devPtr is non null.
 */
                                mhomeIndex = (tt16Bit)
                                        (TM_DEV_IP_CONFIG | TM_DEV_IP_CONF_STARTED);
                                devPtr = tfIfaceAnyConf(&mhomeIndex);
                            }
                        }
                    }
#endif /* TM_SINGLE_INTERFACE_HOME */
                    if (devPtr == TM_DEV_NULL_PTR)
                    {
                        errorCode = TM_ENOENT;
                        goto outIpDestToPacket;
                    }
                    packetPtr->pktDeviceEntryPtr = devPtr;
                    packetPtr->pktMhomeIndex = mhomeIndex;
                    packetPtr->pktFlags |= TM_OUTPUT_DEVICE_FLAG |
                                           TM_IP_BROADCAST_FLAG;
                }
                else
                {
                    packetPtr->pktFlags |= TM_IP_BROADCAST_FLAG;
                }
                if (tm_ip_match(peerIpAddress, TM_IP_LIMITED_BROADCAST))
                {
                    if ( (devPtr != 0) && (tm_ll_is_pt2pt(devPtr)) )
                    {
/*
 * convert point to point limited broadcast address to other side's address
 */
                        tm_ip_copy( devPtr->devPt2PtPeerIpAddr,
                                    peerIpAddress);
/* If zero, convert to LIMITED BROADCAST */
                        if (tm_ip_zero(peerIpAddress))
                        {
                            tm_ip_copy(TM_IP_LIMITED_BROADCAST, peerIpAddress);
                        }
                    }
                }
            }
            else
            {
/*
 * Unspecified multicast source address, not limited broadcast case.
 * We can use the routing table. (For multicast, we will find the
 * system administrator choice of the outgoing interface).
 */
ipDestRoute:
                if (tm_16bit_bits_not_set( packetPtr->pktFlags,
                                           TM_ETHERNET_ADDRESS_FLAG) )
                {
#ifdef TM_4_USE_SCOPE_ID
                    errorCode = tf6IpCacheRte(rtcPtr, peerV4MappedIpAddrPtr);
#else /* ! TM_4_USE_SCOPE_ID */
                    errorCode = tfIpCacheRte(rtcPtr, peerIpAddress);
#endif /* ! TM_4_USE_SCOPE_ID */
                    if (errorCode != TM_ENOERROR)
                    {
/*
 * Not a valid entry. Check if we have a booting or configured interface.
 * If interface is LAN, use it only if destination address is multicast.
 */
#ifdef TM_SINGLE_INTERFACE_HOME
/* return single interface if it is configured, or booting */
                        devPtr = tm_iface_any_conf(&mhomeIndex);
                        mhomeIndex = TM_16BIT_ZERO;
#else /* TM_SINGLE_INTERFACE_HOME */
#ifdef TM_USE_STRONG_ESL
                        devPtr = tfIpSocBindDevPtr(
                                packetPtr->pktSharedDataPtr->dataSocketEntryPtr);
                        if (devPtr != TM_DEV_NULL_PTR)
                        {
                            mhomeIndex = (tt16Bit)
                                        (TM_DEV_IP_CONFIG | TM_DEV_IP_CONF_STARTED);
                            retCode = tfIpAnyConfDevPtr (devPtr, &mhomeIndex);
                            if (retCode != TM_ENOERROR)
                            {
                                devPtr = TM_DEV_NULL_PTR;
                            }
                        }
                        else
#endif /* TM_USE_STRONG_ESL */
                        {
/*
 * Use first configured, or non PPP device in the process of being configured:
 * NOTE TM_DEV_IP_CONFIG, and then TM_DEV_IP_CONF_STARTED is INPUT parameter to
 * tm_iface_any_conf().
 * mhomeIndex will be set if devPtr is non null.
 */
                            mhomeIndex = (tt16Bit)
                                        (TM_DEV_IP_CONFIG | TM_DEV_IP_CONF_STARTED);
                            devPtr = tfIfaceAnyConf(&mhomeIndex);
                        }
#endif /* TM_SINGLE_INTERFACE_HOME */
                        if (    (devPtr != TM_DEV_NULL_PTR)
                             && (    (!tm_4_ll_is_lan(devPtr))
                                  || tm_16bit_one_bit_set(
                                                       packetPtr->pktFlags,
                                                       TM_IP_MULTICAST_FLAG )
                                )
                           )
                        {
                            packetPtr->pktDeviceEntryPtr = devPtr;
                            packetPtr->pktMhomeIndex = mhomeIndex;
                            packetPtr->pktFlags |= TM_OUTPUT_DEVICE_FLAG;
                            errorCode = TM_ENOERROR;
                            goto ipDestToPacketFinish;
                        }
                        else
                        {
                            goto outIpDestToPacket;
                        }
                    }
/* Valid routing entry */
                    rtePtr = rtcPtr->rtcRtePtr;
                    goto ipSetFromRoute;
                }
            }
        }
    }
    else
    {
ipSetFromRoute:
/*
 * BUG ID 202: When sending out packets, we should use what user selected by
 * parameter mhomeIndex(in tfSendToInterface) as source IP address.
 */
        if(tm_16bit_bits_not_set(packetPtr->pktFlags2, TM_PF2_MHOMEINDEX_SET))
        {
/* Fill in packet structure from route */
            packetPtr->pktFlags |= TM_IP_RTE_FLAG | TM_OUTPUT_DEVICE_FLAG;
            packetPtr->pktDeviceEntryPtr = rtePtr->rteDevPtr;
#ifndef TM_LOOP_TO_DRIVER
            packetPtr->pktOrigDevPtr = rtePtr->rteOrigDevPtr;
#endif /* !TM_LOOP_TO_DRIVER */
#ifdef TM_SINGLE_INTERFACE_HOME
            packetPtr->pktMhomeIndex = rtePtr->rteMhomeIndex;
#else /* ! TM_SINGLE_INTERFACE_HOME */
            if ( (tm_ip_zero(srcIpAddress))
                 ||
#ifdef TM_USE_IPV6
                 tm_ip_match(
                     rtePtr->rteHSDevIpAdd.s6LAddr[3], srcIpAddress)
#else /* ! TM_USE_IPV6 */
                 tm_ip_match(rtePtr->rteHSDevIpAdd, srcIpAddress)
#endif /* ! TM_USE_IPV6 */
                )
            {
                packetPtr->pktMhomeIndex = rtePtr->rteMhomeIndex;
            }
            else
            {
                mhomeIndex = TM_16BIT_ZERO;
                (void)tm_iface_addr_match(srcIpAddress, &mhomeIndex);
                packetPtr->pktMhomeIndex = mhomeIndex;
            }
#endif /* ! TM_SINGLE_INTERFACE_HOME */
        }
        else
        {
            packetPtr->pktFlags |= TM_IP_RTE_FLAG;
        }
        packetPtr->pktSharedDataPtr->dataRtePtr = rtePtr;
        packetPtr->pktSharedDataPtr->dataRteClonedPtr = rtcPtr->rtcRteClonedPtr;
    }
ipDestToPacketFinish:
/* Fill in Source IP address in IP header */
    if (packetPtr->pktFlags2 & TM_PF2_ZERO_SRC_IP_ADDRESS)
    {
        tm_ip_copy(TM_IP_ZERO, iphPtr->iphSrcAddr);
    }
    else
    {
        if (tm_ip_zero(srcIpAddress))
        {
#ifndef TM_LOOP_TO_DRIVER
            if (packetPtr->pktOrigDevPtr != (ttDeviceEntryPtr)0)
            {
                tm_ip_copy( tm_ip_dev_addr( packetPtr->pktOrigDevPtr,
                                            packetPtr->pktMhomeIndex ),
                            iphPtr->iphSrcAddr );
            }
            else
#endif /* !TM_LOOP_TO_DRIVER */
            {
                tm_ip_copy( tm_ip_dev_addr( packetPtr->pktDeviceEntryPtr,
                                            packetPtr->pktMhomeIndex ),
                            iphPtr->iphSrcAddr );
            }
        }
        else
        {
            tm_ip_copy(srcIpAddress, iphPtr->iphSrcAddr);
        }
    }
/* Fill in tos in IP header */
    iphPtr->iphTos = ipTos;
    packetPtr->pktFlags |= TM_IP_DEST_ROUTE_SET;
/* Copy destination address from socket entry */
    tm_ip_copy(peerIpAddress, iphPtr->iphDestAddr);
outIpDestToPacket:
#ifdef TM_4_USE_SCOPE_ID
    devPtr = packetPtr->pktDeviceEntryPtr;
    if ((errorCode == TM_ENOERROR) && (devPtr != TM_DEV_NULL_PTR))
    {
        if (((srcDevIndex != TM_16BIT_ZERO)
             && (srcDevIndex != devPtr->devIndex))
            || ((peerDevIndex != TM_16BIT_ZERO)
                && (peerDevIndex != devPtr->devIndex)))
        {
/* link-local scope source IP address or link-local scope destination IP
   address isn't on same link as our output device/interface */
            errorCode = TM_EHOSTUNREACH;
        }
    }
#endif /* TM_4_USE_SCOPE_ID */
#ifdef TM_SNMP_MIB
    if (errorCode == TM_EHOSTUNREACH)
    {
        devPtr = packetPtr->pktDeviceEntryPtr;
        if (devPtr != TM_DEV_NULL_PTR)
        {
/*
 * Number of IP datagrams which local IP user-protocols (including ICMP)
 * supplied to IP in requests for transmission (does not include forwarded
 * packets).
 */
            tm_context(tvIpData).ipOutRequests++;
#ifdef TM_USE_NG_MIB2
            tm_64Bit_incr(devPtr->dev4Ipv4Mib.ipIfStatsHCOutRequests);
            tm_64Bit_incr(tm_context(tvDevIpv4Data).ipIfStatsHCOutRequests);
#endif /* TM_USE_NG_MIB2 */
        }
        tm_context(tvIpData).ipOutNoRoutes++;
#ifdef TM_USE_NG_MIB2
        tm_context(tvDevIpv4Data).ipIfStatsOutNoRoutes++;
#endif /* TM_USE_NG_MIB2 */
    }
#endif /* TM_SNMP_MIB */
    return errorCode;
}

#ifndef TM_SINGLE_INTERFACE_HOME
#ifdef TM_USE_STRONG_ESL
static ttDeviceEntryPtr tfIpSocBindDevPtr (ttSocketEntryPtr  socketEntryPtr)
{
    ttDeviceEntryPtr    devPtr;

    devPtr = TM_DEV_NULL_PTR;
    if (socketEntryPtr != (ttSocketEntryPtr)0)
    {
        devPtr = socketEntryPtr->socRteCacheStruct.rtcDevPtr;
    }
    return devPtr;
}

static int tfIpAnyConfDevPtr ( ttDeviceEntryPtr  devPtr,
                               tt16BitPtr        mhomeIndexPtr
                             )
{
    tt16Bit         mhomeIndex;
    int             errorCode;

    mhomeIndex = (tt16Bit)(*mhomeIndexPtr & TM_DEV_IP_CONFIG);
    errorCode = TM_ENXIO;
    if (mhomeIndex != 0)
    {
        errorCode = tfMhomeAnyConf(devPtr,
                                   TM_IP_ZERO,
                                   &mhomeIndex);
    }
    if (errorCode != TM_ENOERROR)
    {
        if (!(tm_ll_uses_ppp(devPtr)))
        {
/* mhomeIndex not modified by tfMhomeAnyConf when it returns an error */
            mhomeIndex = (tt16Bit)(mhomeIndex & TM_DEV_IP_CONF_STARTED);
            if (mhomeIndex != TM_16BIT_ZERO)
            {
                errorCode = tfMhomeAnyConf(devPtr, TM_IP_ZERO, &mhomeIndex);
            }
        }
    }
    if (errorCode == TM_ENOERROR)
    {
        *mhomeIndexPtr = mhomeIndex;
    }
    return errorCode;
}
#endif /* TM_USE_STRONG_ESL */

static ttDeviceEntryPtr tfIfaceAnyConf (tt16BitPtr mhomeIndexPtr)
{
    ttDeviceEntryPtr devPtr;
    tt16Bit          mhomeIndex;

/* NOTE TM_DEV_IP_CONFIG is INPUT parameter to tm_iface_any_conf().
 * mhomeIndex will be set if devPtr is non null.
 */
    mhomeIndex = (tt16Bit)(*mhomeIndexPtr & TM_DEV_IP_CONFIG);
    devPtr = TM_DEV_NULL_PTR;
    if (mhomeIndex != 0)
    {
        devPtr = tm_iface_any_conf(&mhomeIndex);
    }
    if (devPtr == TM_DEV_NULL_PTR)
    {
/*  or booting a non PPP/PPPoE interface.
 * NOTE TM_DEV_IP_CONF_STARTED is INPUT parameter to tm_iface_any_conf().
 * mhomeIndex will be set if devPtr is non null.
 */
        mhomeIndex = (tt16Bit)(*mhomeIndexPtr & TM_DEV_IP_CONF_STARTED);
        if (mhomeIndex != TM_16BIT_ZERO)
        {
            devPtr = tm_iface_any_conf(&mhomeIndex);
            if (devPtr != TM_DEV_NULL_PTR)
            {
                if (tm_ll_uses_ppp(devPtr))
                {
                    devPtr = TM_DEV_NULL_PTR;
                }
            }
        }
    }
    if (devPtr != TM_DEV_NULL_PTR)
    {
        *mhomeIndexPtr = mhomeIndex;
    }
    return devPtr;
}
#endif /* !TM_SINGLE_INTERFACE_HOME */
#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4IpDummy = 0;
#endif /* ! TM_USE_IPV4 */
