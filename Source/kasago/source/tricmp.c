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
 * Description: ICMP functions
 * Filename: tricmp.c
 * Author: Odile
 * Date Created: 12/29/97
 * $Source: source/tricmp.c $
 *
 * Modification History
 * $Revision: 6.0.2.19 $
 * $Date: 2013/02/08 12:23:34JST $
 * $Author: lfox $
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
 * Local macros
 */
/* ICMP address mask header length (12) */
#define TM_ICMP_ADDRM_LEN           12

#define TM_ICMP_RDISC_ADV_MIN_LEN   16
/* Icmp router discovery default lifetime in seconds(1800) */
#define TM_ICMP_RDISC_DEF_LIFE      tm_const_htons(1800)
/* ICMP router discovery default preference level (0) */
#define TM_ICMP_RDISC_DEF_PREF      TM_UL(0) /* network byte order */

#define TM_ICMP_MAX_CREATE_HEADER   TM_ICMP_RDISC_ADV_MIN_LEN

#define TM_4PAK_TL_HDR_LEN          tm_packed_byte_count(8)

#define TM_4PAK_ICMP_MIN_ERR_HDRS \
    ( TM_4PAK_ICMP_MIN_LEN + TM_4PAK_IP_MIN_HDR_LEN + TM_4PAK_TL_HDR_LEN )

#define tm_icmp_min_err_header(ipHdrLen) \
    (tm_byte_count(TM_4PAK_ICMP_MIN_LEN + ipHdrLen + TM_4PAK_TL_HDR_LEN))


/*
 * Local functions
 */
static tt8Bit tfIcmpReplyPacket (ttPacketPtr packetPtr, tt8Bit updateChecksum);

/*
 * tfIcmpIncomingPacket() Function Description:
 * This function is called (from the IP layer) when an incoming ICMP
 * datagram is coming from the network.
 * 1. First, we check the validity of the ICMP header, that it is at least
 *    8bytes long and then make the packet point to the ICMP header (after
 *    having saved the pointer to the IP header), and compute the ICMP
 *    checksum, free the packet and return if either of these 2 tests fail.
 * 2. If the ICMP type is echo request,  we set the icmp type to
 *    TM_ICMP_TYPE_ECHO_RPL and, if we don't allow fragmentation, and packet
 *    is bigger than MTU, we truncate the packet (RFC1122), then call
 *    tfIcmpReplyPacket(packetPtr, updateChecksum), allowing a checksum
 *    update if we did not truncate.
 *    If linklayer is broadast or pktFlags is broadcast, tfIcmpReplyPacket
 *    will not be called (to prevent broadcast storm) unless
 *    TM_PING_REPLY_BROADCAST is defined in trsystem.h.
 *    If linklayer is multicast or pktFlags is multicast, tfIcmpReplyPacket
 *    will not be called unless TM_PING_REPLY_MULTICAST or
 *    TM_PING_REPLY_BROADCAST are defined in trsystem.h.
 * 3. If the ICMP type is information request or reply, timestamp request
 *    or reply or any other ICMP type we do not handle, we ignore and
 *    discard the packet and return.
 * 4. If the ICMP type is information request or reply, timestamp request
 *    or reply, we ignore and discard the packet and return.
 * 5. If the ICMP type is address mask request, if the ICMP length is at
 *    less than TM_ICMP_ADDRM_LEN), or if tfIcmpAddrMaskAgent is not set,
 *    we just discard the packet and return. Otherwise, we find the address
 *    mask. We get the address mask corresponding to pktDevicePtr and
 *    pktMhomeIndex, set the type to TM_ICMP_TYPE_MASK_RPL, set the
 *    ICMP header address mask field with the address mask and then call
 *    tfIcmpReplyPacket(packetPtr, 0);
 * 6. If the ICMP type is router discovery solicitation. If tvIpForward is
 *    0 or pktDevicePtr does not allow forwarding we just discard the packet
 *    and return, otherwise if pktSrcIpAddress is 0, make it a limited
 *    broadcast, and then call
 *    tfIcmpCreatePacket(pktDeviceEntryPtr, pktMhomeIndex,
 *    TM_OUTPUT_DEVICE_FLAG, pktspecDestIpAddress,
 *    pktsrcIpAddress, TM_ICMP_TYPE_RDISC_ADV, 0, specDestIpAddress);
 *    Note that we have to use tfIcmpCreatePacket() and not
 *    tfIcmpReplyPacket() since the advertisement packet is bigger than
 *    the solicitation packet (RFC1256).
 * 7. If the ICMP type is redirect and tvIpForward is 0, and tfRipOn is 0,
 *    let the router know about it by calling tf4RtRedirect(DestIpAddress
 *    (from bad IP packet header), newRouter (from redirect gateway field),
 *    srcIpAddress (from IP header)). If tvIpForward is non zero, or
 *    tfRipOn, we ignore redirects.
 * 8. If the ICMP type is any other error message (Destination unreachable,
 *    time exceeded, Parameter problem, source quench), we map the ICMP
 *    error to a socket error. We try and find the
 *    socket associated with the ULP destination protocol, ULP destination
 *    and source port, IP source and destination address of the bad packet,
 *    calling tf4SocketLookup(protocol, srcIpAddress, srcPort,
 *    DestIpaddress, destPort, flag).
 *    If it is a TCP socket and we got a source quench, we trigger a slow
 *    start else if it is a TCP socket and the ICMP type is destination
 *    unreachable and code 2, 3, or 4, we should abort the TCP connection,
 *    passing the error code to the abort function, else we call a function
 *    that queues the error with the socket descriptor if there is no error
 *    callback function, otherwise we call the socket error call back
 *    function passing the socket index, and error code.
 * 9. If the ICMP type is echo reply we let the router code know about
 *    the echo reply. We also get the corresponding ICMP socket
 *    calling tf4SocketLookup(protocol, DestIpAddress, destPort, 0, 0
 *    TM_SOC_TREE_CL_FLAG),
 *    and call tfSocketIncomingPacket() which should:
 *    call the application callback function if there is any,
 *    otherwise queue the data to the socket.
 * 10.If the ICMP type is an address mask reply and if tvIpForward is 0,
 *    we check the validity of the mask (by checking that it is not all 1's
 *    , and if it is not 0, then that the first 8 bits are on), and store
 *    it on the interface. (A router (tvIpForward != 0 should ignore address
 *    mask replies.)
 * 11.If the ICMP type is a router discovery advertisement and if
 *    tvIpForward is 0, we add the gateway as a default static route for the
 *    interface the packet came in. (A router (tvIpForward != 0) should
 *    ignore router discovery advertisements.) We also ignore router
 *    discovery advertisement if we have RIP?.
 *
 * Parameter   Description
 * packetPtr   pointer to ttPacket
 *             packetPtr->pktLinkDataPtr points to the IP header,
 *             packetPtr->pktFlags gives the link layer and IP layer broadcast
 *              multicast status.
 *             packetPtr->pktTotalLength is the length of the IP packet.
 *             packetPtr->pktDeviceEntryPtr points to the device the packet
 *              came in.
 *             packetPtr->pktSpecDestIpAddr contains the specific IP
 *               destination address,
 *             packetPtr->pktDestIpAddr contains the IP destination address
 *               from the IP header.
 *             packetPtr->pktSrcIpAddr contains the source address of the
 *               UDP datagram.
 *             packetPtr->pktFlags indicates whether the destination
 *               IP address is broadcast, unicast or multicast
 *             packetPtr->pktIpHdrLen IP header length in bytes
 *              (so that we know whether there are options)
 *
 * Returns
 * Value   Meaning
 * None
 *
 */
void tfIcmpIncomingPacket (ttPacketPtr packetPtr)
{
    ttIcmpHeaderPtr      icmphPtr;
    ttDeviceEntryPtr     devPtr;
    ttTlHeaderPtr        tlPtr;
    ttSocketEntryPtr     socketPtr;
#ifdef TM_USE_TCP
    ttTcpVectPtr         tcpVectPtr;
#endif /* TM_USE_TCP */
    ttSharedDataPtr      pktShrDataPtr;
#ifdef TM_USE_RAW_SOCKET
    ttPacketPtr          rawPacketPtr;
#ifdef TM_RAW_SOCKET_INPUT_ICMP_REQUESTS
    ttPacketPtr          saveRawPacketPtr;
#endif /* TM_RAW_SOCKET_INPUT_ICMP_REQUESTS */
#endif /* TM_USE_RAW_SOCKET */
#ifdef TM_DEV_RECV_OFFLOAD
    ttDevRecvOffloadPtr  pktDevRecvOffloadPtr;
#endif /* TM_DEV_RECV_OFFLOAD */
    ttTupleDev           tupleDev;
    tt4IpAddress         gatewayIpAddr;
    tt4IpAddress         netMask;
    ttPktLen             icmpLength;
    tt16Bit              lifeTime;
    tt16Bit              ipHdrLen;
    tt16Bit              flag;
    tt16Bit              checksum;
#ifndef TM_SINGLE_INTERFACE_HOME
    tt16Bit              mhomeIndex;
#endif /* !TM_SINGLE_INTERFACE_HOME */
    int                  errorCode;
    int                  minIcmpByteLength;
    int                  cbFlags;
    int                  prefixLen;
#if (defined(TM_PMTU_DISC) && defined(TM_USE_IPSEC) \
                           && defined(TM_USE_STRONG_ESL))
    int                  physPort;
#endif /* TM_PMTU_DISC && TM_USE_IPSEC && TM_USE_STRONG_ESL */
    tt8Bit               ulp;
    tt8Bit               icmpCode;
#define lNextHopMtu      lifeTime /* reuse */
    tt8Bit               updateChecksum;
    tt8Bit               needFree;
    tt8Bit               pingRequestMessage;
#ifdef TM_MOBILE_IP4
    ttMipIncomingFuncPtr mipIcmpFuncPtr;
#endif /* TM_MOBILE_IP4 */
#ifdef TM_PMTU_DISC
#ifdef TM_USE_IPSEC
    ttSaIdentity         said;
    tt32BitPtr           temp32Ptr;
    ttSadbRecordPtr      saPtr;
#endif /* TM_USE_IPSEC */
#endif /* TM_PMTU_DISC */
#ifdef TM_USE_IPV6
    tt6TunnelIcmpErrorFuncPtr tunnelFuncPtr;
#endif /* TM_USE_IPV6 */

#ifdef TM_SNMP_MIB
/* Number of ICMP messages which the entity received (includes errors) */
    tm_context(tvIcmpData).icmpInMsgs++;
#endif /* TM_SNMP_MIB */
/*
 * Check the validity of the ICMP header, that it is at least
 * 8bytes long
 */
    needFree = TM_8BIT_YES;
    ipHdrLen = packetPtr->pktIpHdrLen;
    devPtr = packetPtr->pktDeviceEntryPtr;
    errorCode = TM_ENOERROR;
    minIcmpByteLength = tm_icmp_min_err_header(ipHdrLen);
#ifdef TM_USE_DRV_SCAT_RECV
/*
 * If ICMP header is not contiguous, make it contiguous. Assumes biggest
 * possible ICMP header
 */
    if (tm_pkt_hdr_is_not_cont(packetPtr, minIcmpByteLength))
    {
        packetPtr = tfContiguousSharedBuffer(packetPtr,
                                             minIcmpByteLength);
        if (packetPtr == (ttPacketPtr)0)
        {
            goto icmpIncomingExit;
        }
    }
#endif /* TM_USE_DRV_SCAT_RECV */
    pktShrDataPtr = packetPtr->pktSharedDataPtr;
    socketPtr = (ttSocketEntryPtr)0;
#ifdef TM_USE_RAW_SOCKET
    rawPacketPtr = tfDuplicatePacket(packetPtr);
#endif /* TM_USE_RAW_SOCKET */

    if ( (packetPtr->pktChainDataLength) >=
         ( ((ttPktLen) tm_byte_count(ipHdrLen+TM_4PAK_ICMP_MIN_LEN)) ) )
    {
/* Point to ICMP header */
        tm_pkt_ulp_hdr(packetPtr, ipHdrLen);
/* Icmp datagram size */
        icmpLength = packetPtr->pktChainDataLength;
/* Check that the ICMP checksum is valid. */
#ifdef TM_DEV_RECV_OFFLOAD
        pktDevRecvOffloadPtr = (ttDevRecvOffloadPtr)packetPtr->pktDevOffloadPtr;
        if (    (pktDevRecvOffloadPtr != (ttDevRecvOffloadPtr)0)
             && (   pktDevRecvOffloadPtr->devoRFlags
                   & (TM_DEVOR_PACKET_RECV_CHKSUM_COMPUTED ) ) )
        {
/*
 * Use hardware computed checksum on ICMP header + data.
 */
            checksum = pktDevRecvOffloadPtr->devoRecvXsum;
        }
        else
#endif /* TM_DEV_RECV_OFFLOAD */
        {
            checksum = tfPacketChecksum(
                      packetPtr, icmpLength, TM_16BIT_NULL_PTR, TM_16BIT_ZERO);
        }
        if (checksum == 0)
        {
            icmphPtr = (ttIcmpHeaderPtr)packetPtr->pktLinkDataPtr;
            icmpCode = icmphPtr->icmCode;
            switch (icmphPtr->icmType)
            {
/*
 * 2. If the ICMP type is echo request, we set the icmp type to
 *    TM_ICMP_TYPE_ECHO_RPL and, if we don't allow fragmentation, and packet
 *    is bigger than MTU, we truncate the packet (RFC1122), then call
 *    tfIcmpReplyPacket(packetPtr, updateChecksum), allowing a checksum
 *    update if we did not truncate.
 *    If linklayer is broadast or pktFlags is broadcast, tfIcmpReplyPacket
 *    will not be called (to prevent broadcast storm) unless
 *    TM_PING_REPLY_BROADCAST is defined in trsystem.h.
 *    If linklayer is multicast or pktFlags is multicast, tfIcmpReplyPacket
 *    will not be called unless TM_PING_REPLY_MULTICAST or
 *    TM_PING_REPLY_BROADCAST are defined in trsystem.h.
 */
                case TM_ICMP_TYPE_ECHO_REQ:
#ifdef TM_SNMP_MIB
/* Number of ICMP Echo (request) messages received. */
                    tm_context(tvIcmpData).icmpInEchos++;
#endif /* TM_SNMP_MIB */

#ifdef TM_USE_ECHO_CONFIG
/* first check to see if ICMP echo responses have been disabled via options */
                    if (tm_context(tvIcmpEchoEnable) == (TM_8BIT_ZERO))
                    {
#ifdef TM_USE_RAW_SOCKET
#ifndef TM_RAW_SOCKET_INPUT_ICMP_REQUESTS
                        if (rawPacketPtr != TM_PACKET_NULL_PTR)
                        {
                            tfFreePacket(rawPacketPtr, TM_SOCKET_UNLOCKED);
                            rawPacketPtr = TM_PACKET_NULL_PTR;
                        }
#endif /* !TM_RAW_SOCKET_INPUT_ICMP_REQUESTS */
#endif /* TM_USE_RAW_SOCKET */

                        needFree = TM_8BIT_YES;
                        break;
                    }
#endif /* TM_USE_ECHO_CONFIG */

#ifdef TM_USE_RAW_SOCKET
#ifdef TM_RAW_SOCKET_INPUT_ICMP_REQUESTS

/* we need to copy the packet if we are using raw sockets, so that the
   application can receive the original ICMP Echo-Request */
                    if (rawPacketPtr != TM_PACKET_NULL_PTR)
                    {
                        saveRawPacketPtr = rawPacketPtr;
                        rawPacketPtr = tfGetSharedBuffer(
                            (TM_4_IP_MIN_HDR_LEN + TM_LL_MAX_HDR_LEN),
                            saveRawPacketPtr->pktChainDataLength,
                            TM_16BIT_ZERO);
                        if (rawPacketPtr != TM_PACKET_NULL_PTR)
                        {
                            tfCopyPacket(saveRawPacketPtr, rawPacketPtr);
                        }

/* It is possible that we couldn't allocate another packet to copy and
   preserve the Echo-Request, in which case we won't deliver the
   Echo-Request to the raw socket - better that than to deliver the
   Echo-Reply which overwrote it. */
                        tfFreePacket(saveRawPacketPtr, TM_SOCKET_UNLOCKED);
                    }
#else /* ! TM_RAW_SOCKET_INPUT_ICMP_REQUESTS */
                    if (rawPacketPtr != TM_PACKET_NULL_PTR)
                    {
/* avoid passing this packet to the raw socket, since it will be modified
   to be a reply packet */
                        tfFreePacket(rawPacketPtr, TM_SOCKET_UNLOCKED);
                        rawPacketPtr = TM_PACKET_NULL_PTR;
                    }
#endif /* ! TM_RAW_SOCKET_INPUT_ICMP_REQUESTS */
#endif /* TM_USE_RAW_SOCKET */

/* If TM_PING_REPLY_BROADCAST is defined, reply to all packets.
 * If TM_PING_REPLY_BROADCAST is not defined and TM_PING_REPLY_MULTICAST,
 * drop broadcast packets but reply to multicast packets.
 * If TM_PING_REPLY_BROADCAST and TM_PING_REPLY_MULTICAST are not defined,
 * drop both multicast and broadcast packets.
 */
#ifndef TM_PING_REPLY_BROADCAST

#ifdef TM_PING_REPLY_MULTICAST
#define TM_ICMP_FORBIDEN_RECV_PKT_FLAGS TM_IP_BROADCAST_FLAG|\
                                        TM_LL_BROADCAST_FLAG
#else /* !TM_PING_REPLY_MULTICAST */
#define TM_ICMP_FORBIDEN_RECV_PKT_FLAGS  TM_IP_BROADCAST_FLAG|\
                                         TM_LL_BROADCAST_FLAG|\
                                         TM_IP_MULTICAST_FLAG|\
                                         TM_LL_MULTICAST_FLAG
#endif /* !TM_PING_REPLY_MULTICAST */

                    if ( tm_16bit_bits_not_set(packetPtr->pktFlags,
                                  TM_ICMP_FORBIDEN_RECV_PKT_FLAGS) )
#endif /* !TM_PING_REPLY_BROADCAST */
                    {
/* Make sure that the incoming (and now outgoing) device has been configured */
#ifdef TM_SINGLE_INTERFACE_HOME
                        if ( tm_8bit_one_bit_set(
                                        tm_ip_dev_conf_flag(devPtr, 0),
                                        TM_DEV_IP_CONFIG ) )
#else /* !TM_SINGLE_INTERFACE_HOME */
                        if (devPtr != tm_context(tvLoopbackDevPtr))
                        {
                            mhomeIndex = (tt16Bit)(TM_DEV_IP_CONFIG);
                            errorCode = tfMhomeAnyConf(devPtr,
                                                       TM_IP_ZERO,
                                                       &mhomeIndex);
                        }
                        if (errorCode == TM_ENOERROR)
#endif /* !TM_SINGLE_INTERFACE_HOME */
                        {
/* Assume we can update the checksum (without computing it from scratch) */
                            updateChecksum = TM_8BIT_YES;
                            icmphPtr->icmType = TM_ICMP_TYPE_ECHO_RPL;
#ifdef TM_IP_FRAGMENT
                            if (tm_context(tvIpFragment) == 0)
#endif /* TM_IP_FRAGMENT */
/* If we do not want to send fragments */
                            {
                                if ((icmpLength + tm_byte_count(ipHdrLen)) >
                                                       (ttPktLen)devPtr->devMtu)
/* and if we received a fragmented packet */
                                {
                                    tfPacketTailTrim(packetPtr,
                                                    (ttPktLen)(
                                                        (ttPktLen)devPtr->devMtu
                                                    - tm_byte_count(
                                                           (ttPktLen)ipHdrLen)),
                                                    TM_SOCKET_UNLOCKED);
/* Packet truncation. We have to compute the checksum from scatch */
                                    updateChecksum = TM_8BIT_ZERO;
                                }
                            }
                            if (    (updateChecksum != TM_8BIT_ZERO)
                                && (icmphPtr->icmSUIdent == TM_16BIT_ZERO)
                                && (icmphPtr->icmSUSeqno == TM_16BIT_ZERO)
                            )
                            {
                                updateChecksum = TM_8BIT_ZERO;
                            }
                            needFree = tfIcmpReplyPacket(packetPtr,
                                                         updateChecksum);
                        }
                    }
                    break;
/*
 * 3. If the ICMP type is timestamp request or reply, information request
 *    or reply or any other ICMP type we do not handle, we ignore and
 *    discard the packet and return.
 */
                case TM_ICMP_TYPE_TIME_REQ:
#ifdef TM_SNMP_MIB
/* Number of ICMP Timestamp (request) messages received. */
                    tm_context(tvIcmpData).icmpInTimestamps++;
                    break;
#endif /* TM_SNMP_MIB */
                case TM_ICMP_TYPE_TIME_RPL:
#ifdef TM_SNMP_MIB
/* Number of ICMP Timestamp Reply messages received. */
                    tm_context(tvIcmpData).icmpInTimestampReps++;
                    break;
#endif /* TM_SNMP_MIB */
                case TM_ICMP_TYPE_INFO_REQ:
                case TM_ICMP_TYPE_INFO_RPL:
                default:
                    break;
                case TM_ICMP_TYPE_MASK_REQ:
#ifdef TM_SNMP_MIB
/* Number of ICMP Address Mask Request messages received. */
                    tm_context(tvIcmpData).icmpInAddrMasks++;
#endif /* TM_SNMP_MIB */

#ifdef TM_USE_RAW_SOCKET
#ifdef TM_RAW_SOCKET_INPUT_ICMP_REQUESTS
/* we need to copy the packet if we are using raw sockets, so that the
   application can receive the original ICMP Mask-Request */
                    if (rawPacketPtr != TM_PACKET_NULL_PTR)
                    {
                        saveRawPacketPtr = rawPacketPtr;
                        rawPacketPtr = tfGetSharedBuffer(
                            (TM_4_IP_MIN_HDR_LEN + TM_LL_MAX_HDR_LEN),
                            saveRawPacketPtr->pktChainDataLength,
                            TM_16BIT_ZERO);
                        if (rawPacketPtr != TM_PACKET_NULL_PTR)
                        {
                            tfCopyPacket(saveRawPacketPtr, rawPacketPtr);
                        }

/* It is possible that we couldn't allocate another packet to copy and
   preserve the Mask-Request, in which case we won't deliver the
   Mask-Request to the raw socket - better that than to deliver the
   Mask-Reply which overwrote it. */
                        tfFreePacket(saveRawPacketPtr, TM_SOCKET_UNLOCKED);
                    }
#else /* ! TM_RAW_SOCKET_INPUT_ICMP_REQUESTS */
                    if (rawPacketPtr != TM_PACKET_NULL_PTR)
                    {
/* avoid passing this packet to the raw socket, since it will be modified
   to be a reply packet */
                        tfFreePacket(rawPacketPtr, TM_SOCKET_UNLOCKED);
                        rawPacketPtr = TM_PACKET_NULL_PTR;
                    }
#endif /* ! TM_RAW_SOCKET_INPUT_ICMP_REQUESTS */
#endif /* TM_USE_RAW_SOCKET */

/*
 * 4. If the ICMP type is address mask request, if the ICMP length is at
 *    less than TM_ICMP_ADDRM_LEN), or if tfIcmpAddrMaskAgent is not set,
 *    we just discard the packet and return. Otherwise, we find the address
 *    mask. We get the address mask corresponding to pktDevicePtr and
 *    pktMhomeIndex, set the type to TM_ICMP_TYPE_MASK_RPL, set the
 *    ICMP header address mask field with the address mask and then call
 *    tfIcmpReplyPacket(packetPtr);
 */
                    if (    (tm_context(tvIcmpAddrMaskAgent) != TM_8BIT_ZERO)
                         && (icmpLength >= (ttPktLen)TM_ICMP_ADDRM_LEN) )
                    {
                        icmphPtr->icmType = TM_ICMP_TYPE_MASK_RPL;
#ifdef TM_SINGLE_INTERFACE_HOME
                        tm_ip_copy( tm_ip_dev_mask(devPtr, 0),
                                    icmphPtr->icmDUAddrMask );
#else /* !TM_SINGLE_INTERFACE_HOME */
                        if (devPtr != pktShrDataPtr->dataDestDevPtr)
                        {
#ifdef TM_ERROR_CHECKING
                            tfKernelWarning("tfIpIncomingPacket",
                "Address mask request destination not on incoming interface");
#endif /* TM_ERROR_CHECKING */
                            goto freeIncoming;
                        }
                        tm_ip_copy( tm_ip_dev_mask(devPtr,
                                                   packetPtr->pktMhomeIndex),
                                    icmphPtr->icmDUAddrMask);
#endif /* TM_SINGLE_INTERFACE_HOME */
                        needFree = tfIcmpReplyPacket(packetPtr, TM_8BIT_ZERO);
                    }
                    break;
                case TM_ICMP_TYPE_RDISC_SOL:
#ifdef TM_SNMP_MIB
                    tm_context(tvIcmpData).icmpInRdiscSol++;
#endif /* TM_SNMP_MIB */
/*
 * If the ICMP type is router discovery solicitation. If tvIpForward is
 * 0 or pktDevicePtr does not allow forwarding we just discard the packet
 * and return, otherwise if ipSrcAddress is 0, make it a limited
 * broadcast, and then call
 * tfIcmpCreatePacket(pktDeviceEntryPtr, mhomeIndex, TM_OUTPUT_DEVICE_FLAG,
 *                    specDestIpAddr, srcIpAddress, TM_ICMP_TYPE_RDISC_ADV,
 *                    0, specDestIpAddress); Note that
 * we have to use tfIcmpCreatePacket() and not tfIcmpReplyPacket() since the
 * advertisement packet is bigger than the solicitation packet (RFC1256).
 */
#ifndef TM_SINGLE_INTERFACE_HOME
                    if (   (tm_context(tvIpForward) != 0)
                        && tm_4_dev_forward(devPtr)
                        && tm_ip_not_zero(
                            tm_4_ip_addr(pktShrDataPtr->
                                                      dataSpecDestIpAddress))
                        )
                    {
                        if (tm_ip_zero(pktShrDataPtr->dataSrcIpAddress))
                        {
                            tm_ip_copy(TM_IP_LIMITED_BROADCAST,
                                       pktShrDataPtr->dataSrcIpAddress);
                        }
                        if (devPtr == pktShrDataPtr->dataDestDevPtr)
                        {
                            (void)tfIcmpCreatePacket(
                                devPtr,
                                packetPtr->pktMhomeIndex,
                                TM_OUTPUT_DEVICE_FLAG,
                                tm_4_ip_addr(pktShrDataPtr->
                                             dataSpecDestIpAddress),
                                pktShrDataPtr->dataSrcIpAddress,
                                TM_ICMP_TYPE_RDISC_ADV,
                                0,
                                tm_4_ip_addr(pktShrDataPtr->
                                                       dataSpecDestIpAddress)
                                );
                        }
#ifdef TM_ERROR_CHECKING
                        else
                        {
                            tfKernelWarning("tfIpIncomingPacket",
                     "Router Disc Sol destination not on incoming interface");
                         }
#endif /* TM_ERROR_CHECKING */
                    }
#endif /* TM_SINGLE_INTERFACE_HOME */
                    break;
                case TM_ICMP_TYPE_REDIRECT:
#ifdef TM_SNMP_MIB
/* Number of ICMP Redirect messages received. */
                    tm_context(tvIcmpData).icmpInRedirects++;
#endif /* TM_SNMP_MIB */
/*
 * If the ICMP type is redirect and tvIpForward is 0, and tfRipOn is 0,
 * let the router know about it by calling tf4RtRedirect(pktDeviceEntryPtr,
 * pktMhomeIndex, destIpAddress (from bad IP packet header), newRouter
 * (from redirect gateway field), srcIpAddress (from IP header)). If
 * tvIpForward is non zero, or tfRipOn is non zero, we ignore redirects.
 */
                    if (    (tm_context(tvIpForward) == 0)
                         && (tm_context(tvRipOn) == 0)
                         && (icmpLength >= TM_4PAK_ICMP_MIN_ERR_HDRS) )
                    {
                        tf4RtRedirect(devPtr,
                                      packetPtr->pktMhomeIndex,
                                      icmphPtr->icmDUIpHeader.iphDestAddr,
                                      icmphPtr->icmSURedGwy,
                                      pktShrDataPtr->dataSrcIpAddress);
                    }
                    break;
/*  8. If the ICMP type is any other error message (Destination unreachable,
 *    time exceeded, Parameter problem, source quench), we map the ICMP
 *    error to a socket error. We try and find the
 *    socket associated with the ULP destination protocol, ULP destination
 *    and source port, IP source and destination address of the bad packet,
 *    calling tf4SocketLookup(protocol, srcIpAddress, srcPort,
 *    DestIpaddress, destPort, flag).
 *    If it is a TCP socket and we got a source quench, we trigger a slow
 *    start else if it is a TCP socket and the ICMP type is destination
 *    unreachable and code 2, 3, or 4, we should abort the TCP connection,
 *    passing the error code to the abort function, else we call a function
 *    that queues the error with the socket descriptor if there is no error
 *    callback function, otherwise we call the socket error call back
 *    function passing the socket index, and error code.
 */
                case TM_ICMP_TYPE_UNREACH:
#ifdef TM_SNMP_MIB
/* Number of ICMP Destination Unreachable messages received. */
                    tm_context(tvIcmpData).icmpInDestUnreachs++;
#endif /* TM_SNMP_MIB */
                    if (icmpCode == TM_ICMP_CODE_FRAG)
                    {
#ifdef TM_PMTU_DISC
                        tm_ntohs(icmphPtr->icmSUNxtHopMtu, lNextHopMtu);

#ifdef TM_USE_IPSEC
                        if  (   (tm_context(tvIpsecPtr))
                             && (   (icmphPtr->icmDUIpHeader.iphUlp ==
                                                                TM_IP_AH)
                                 || (icmphPtr->icmDUIpHeader.iphUlp ==
                                                                TM_IP_ESP) ) )
                        {
                            said.siDstSockaddr.ss_family = PF_INET;
                            said.siDstSockaddr.addr.ipv4.sin_addr.s_addr =
                                icmphPtr->icmDUIpHeader.iphDestAddr;
                            said.siProto = icmphPtr->icmDUIpHeader.iphUlp;
                            temp32Ptr = (tt32Bit*)( (tt8Bit*)
                                &icmphPtr->icmDUIpHeader
                                + tm_ip_hdr_len(icmphPtr->
                                icmDUIpHeader.iphVersLen) );
                            said.siSpi = htonl(* temp32Ptr);
#ifdef TM_USE_STRONG_ESL
                            physPort = tm_dev_get_phys_port(devPtr);
                            said.siPortMask = 1 << physPort;
#endif /* TM_USE_STRONG_ESL */

                            /* get the SA using SPI */
                            errorCode = tfSadbRecordGet(TM_IPSEC_OUTBOUND,
                                                        &said,
                                                        &saPtr);
                            if(errorCode != TM_ENOERROR)
                            {
                                tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
                                return;
                            }

                            LINT_UNINIT_SYM_BEGIN(lifeTime)
                                errorCode = tfRtNextHopMtu(
#ifdef TM_USE_STRONG_ESL
                                devPtr,
#endif /* TM_USE_STRONG_ESL */
                                saPtr->sadbDstIpAddress.
                                            addr.ipv4.sin_addr.s_addr,
                                lNextHopMtu,
                                &icmphPtr->icmDUIpHeader);
                            LINT_UNINIT_SYM_END(lifeTime)
                        }
                        else
                        {
                            LINT_UNINIT_SYM_BEGIN(lifeTime)
                                errorCode = tfRtNextHopMtu(
#ifdef TM_USE_STRONG_ESL
                                devPtr,
#endif /* TM_USE_STRONG_ESL */
                                icmphPtr->icmDUIpHeader.iphDestAddr,
                                lNextHopMtu,
                                &icmphPtr->icmDUIpHeader);
                            LINT_UNINIT_SYM_END(lifeTime)
                        }

#else /* TM_USE_IPSEC */

/*
 * Store lNextHopMtu in host routing entry corresponding to original dest addr
 */
                        LINT_UNINIT_SYM_BEGIN(lifeTime)
                            errorCode = tfRtNextHopMtu(
#ifdef TM_USE_STRONG_ESL
                            devPtr,
#endif /* TM_USE_STRONG_ESL */
                            icmphPtr->icmDUIpHeader.iphDestAddr,
                            lNextHopMtu,
                            &icmphPtr->icmDUIpHeader);
                        LINT_UNINIT_SYM_END(lifeTime)
#endif /* TM_USE_IPSEC */
                            if (errorCode == TM_ENOERROR)
                            {
/* Changed path MTU routing entry. Notify upper layers about it. */
                                errorCode = TM_EMSGSIZE;
                            }
                            else
                            {
/*
 * No routing entry change, probably because this ICMP error message
 * notifies us of a path MTU change that has already taken place.
 * RFC 1191 states that in that case the TCP layer should ignore the
 * error, and not retransmit.
 */
                                errorCode = TM_ENOERROR;
                            }
#else /* !TM_PMTU_DISC */
                            errorCode = TM_EMSGSIZE;
#endif /* TM_PMTU_DISC */
                    }
                    else
                    {
                        if (icmpCode <= TM_ICMP_CODE_HOST_TOS)
                        {
                            switch (icmpCode)
                            {
                            case TM_ICMP_CODE_PROTO:
                            case TM_ICMP_CODE_PORT:
/* Rejection by remote host */
                                errorCode = TM_ECONNREFUSED;
                                break;
                            default:
/* Cannot reach remote host */
                                errorCode = TM_EHOSTUNREACH;
                            }
                        }
                        else
                        {
                            errorCode = TM_EXT_EHOSTUNREACH;
                        }
                    }
                    if (errorCode != TM_ENOERROR)
                    {
                        goto reportErr;
                    }
                    break;
                case TM_ICMP_TYPE_TIME_XCEED:
#ifdef TM_SNMP_MIB
/* Number of ICMP Time Exceeded messages received. */
                    tm_context(tvIcmpData).icmpInTimeExcds++;
#endif /* TM_SNMP_MIB */
                    if (icmpCode == TM_ICMP_CODE_XCEED_TRANS)
                    {
                        errorCode = TM_EXT_TTLTRANSIT;
                    }
                    else
                    {
                        errorCode = TM_EXT_TTLREASS;
                    }
                    goto reportErr;
                case TM_ICMP_TYPE_PARM_PROB:
#ifdef TM_SNMP_MIB
/* Number of ICMP Parameter Problem messages received. */
                    tm_context(tvIcmpData).icmpInParmProbs++;
#endif /* TM_SNMP_MIB */
                    errorCode = TM_ENOPROTOOPT;
                    goto reportErr;
                case TM_ICMP_TYPE_SRC_QNCH:
#ifdef TM_SNMP_MIB
/* Number of ICMP Source Quench messages received. */
                    tm_context(tvIcmpData).icmpInSrcQuenchs++;
#endif /* TM_SNMP_MIB */
                    errorCode = TM_EXT_SRCQNCH;
reportErr:          if (icmpLength >= (ttPktLen) minIcmpByteLength)
                    {
                        ulp = icmphPtr->icmDUIpHeader.iphUlp;
                        tlPtr = (ttTlHeaderPtr)(
                                     (tt8BitPtr)(&(icmphPtr->icmDUIpHeader)) +
                                     ipHdrLen);
                        pingRequestMessage = TM_8BIT_ZERO;
                        flag = TM_16BIT_ZERO;
#ifdef TM_USE_IPV6
                        tm_6_addr_to_ipv4_mapped(
                                   icmphPtr->icmDUIpHeader.iphDestAddr,
                                   &(tupleDev.tudRemoteIpAddress));
                        tm_6_addr_to_ipv4_mapped(
                                   icmphPtr->icmDUIpHeader.iphSrcAddr,
                                   &(tupleDev.tudLocalIpAddress));
#ifdef TM_4_USE_SCOPE_ID
                        tm_4_dev_scope_addr(&(tupleDev.tudRemoteIpAddress),
                                            packetPtr->pktDeviceEntryPtr);
                        tm_4_dev_scope_addr(&(tupleDev.tudLocalIpAddress),
                                            packetPtr->pktDestDevPtr);
#endif /* TM_4_USE_SCOPE_ID */
#else /* !TM_USE_IPV6 */
                        tm_ip_copy(icmphPtr->icmDUIpHeader.iphDestAddr,
                                   tupleDev.tudRemoteIpAddress);
                        tm_ip_copy(icmphPtr->icmDUIpHeader.iphSrcAddr,
                                   tupleDev.tudLocalIpAddress);
#endif /* !TM_USE_IPV6 */
                        if ((ulp == TM_IP_TCP) || (ulp == TM_IP_UDP))
                        {
                            tupleDev.tudLocalIpPort = tlPtr->tlSport;
                            tupleDev.tudRemoteIpPort = tlPtr->tlDport;
                            if (ulp == TM_IP_TCP)
                            {
                                flag = (tt16Bit)(   TM_SOC_TREE_CO_FLAG
                                                  | TM_SOC_TREE_EXACT_MATCH );
                            }
                        }
                        else
                        {
/* ICMP, IGMP */
/* IGMP, or ICMP messages other than ICMP echo requests. */
                            tupleDev.tudLocalIpPort = TM_RAW_IP_PORT;
                            tupleDev.tudRemoteIpPort = TM_PORT_ZERO;
                            if (ulp == TM_IP_ICMP)
                            {
                                if  (    tlPtr->tlIcmType
                                      == TM_ICMP_TYPE_ECHO_REQ )
/* ICMP PING echo request */
                                {
/* Ping socket port is ICMP header request/reply identifier */
                                    tupleDev.tudLocalIpPort
                                                    = tlPtr->tlIcmIdent;
                                    pingRequestMessage = TM_8BIT_YES;
                                }
                            }
#ifdef TM_USE_IPV6
                            else
                            {
/*
 * When receiving an ICMPv4 error message for a tunneled IPv6 packet (i.e. the
 * ULP for the original packet is IPv6), call tf6Ipv4TunnelIcmpIncomingError for
 * processing.  Packet will be freed by tf6Ipv4TunnelIcmpIncomingError.
 */
                                if (ulp == TM_IP_IPV6)
                                {
                                    tunnelFuncPtr =
                                        tm_context(tv6Ipv4IcmpTunnelFuncPtr);
                                    if (tunnelFuncPtr !=
                                            (tt6TunnelIcmpErrorFuncPtr) 0)
                                    {
                                        (*tunnelFuncPtr)(packetPtr,
                                                         (tt8BitPtr) tlPtr);
                                        goto icmpIncomingExit;
                                    }
                                }

                            }
#endif /* TM_USE_IPV6 */
                        }
#if (defined(TM_USE_STRONG_ESL) || defined(TM_IGMP) || defined(TM_6_USE_MLD))
                        tupleDev.tudDevPtr = devPtr;
#endif /* TM_USE_STRONG_ESL || TM_IGMP || TM_6_USE_MLD */
                        tupleDev.tudProtocolNumber = ulp;
                        tupleDev.tudAddressFamily = AF_INET;
                        socketPtr = tfSocketLookup(
                            &tupleDev,
                            flag);

                        if (socketPtr != TM_SOCKET_ENTRY_NULL_PTR)
                        {
/* Socket Entry locked by tf4SocketLookup */
/*
 * Copy info from ICMP error message in socket entry, so that the user can
 * retrieve that information if needed (trace route application for example)
 */
/*
 * Indicate that entry has been written by ICMP, and contains an error
 * message that the user has not read yet.
 */
                            socketPtr->socIcmpInfo.icmCtrlW = 1;
/* ICMP type of ICMP error message */
                            socketPtr->socIcmpInfo.icmTypeB =
                                                            icmphPtr->icmType;
/* ICMP code of ICMP error message */
                            socketPtr->socIcmpInfo.icmCodeB =
                                                            icmphPtr->icmCode;
/* Source IP address of the ICMP error message in network byte order */
#ifdef TM_USE_IPV6
                            tm_6_addr_to_ipv4_mapped(
                                (tt4IpAddress) (packetPtr->pktSrcIpAddress),
                                &(socketPtr->socIcmpInfo.icmSrcIPAddr));
#else /* ! TM_USE_IPV6 */
                            tm_ip_copy( packetPtr->pktSrcIpAddress,
                                        socketPtr->socIcmpInfo.icmSrcIPAddr );
#endif /* ! TM_USE_IPV6 */

                            if (errorCode == TM_EXT_TTLTRANSIT)
                            {
/* BUG ID 40: Check for wraparound condition */
                                if (socketPtr->socIpTtl != TM_IP_MAX_TTL)
/* End of BUG ID 40: Check for wraparound condition */
                                {
/* increase the TTL */
                                    socketPtr->socIpTtl++;
                                }
                            }
#ifdef TM_USE_TCP
                            if (socketPtr->socProtocolNumber == IP_PROTOTCP)
                            {
                                tcpVectPtr = (ttTcpVectPtr)
                                                    ((ttVoidPtr)socketPtr);
                                if (tcpVectPtr->tcpsState > TM_TCPS_LISTEN)
/*
 * Report TCP errors, only if socket is not in the closed state and not
 * a listening socket.
 * TCP will delay reporting the error to the user until the connection
 * times out. If error is source quench, TCP will attempt slow start.
 */
                                {
                                    tfTcpIcmpErrInput ( tcpVectPtr,
                                                        tlPtr,
                                                        errorCode,
                                                        icmpCode   );
                                }
                            }
/*
 * Report all errors to user, including errors > TM_EXT_ERROR_BASE
 */
                            else  /* non TCP sockets */
#endif /* TM_USE_TCP */
                            {
                                if (    ( socketPtr->socProtocolNumber !=
                                          IP_PROTOUDP )
                                     || ( tm_16bit_one_bit_set(
                                                       socketPtr->socFlags,
                                                       TM_SOCF_CONNECTED ) ) )
                                 {
                                    if (pingRequestMessage)
                                    {
                                         (void)tfSocketIncomingPacket(
                                             socketPtr, packetPtr );
                                         needFree = TM_8BIT_ZERO;
                                         cbFlags =
                                            (TM_CB_SOCKET_ERROR | TM_CB_RECV);
                                    }
                                    else
                                    {
/* For UDP sockets only notify if socket is connected */
                                        cbFlags = TM_CB_SOCKET_ERROR;
                                    }
                                    tfSocketNotify(
                                               socketPtr,
                                               TM_SELECT_READ|TM_SELECT_WRITE,
                                               cbFlags,
                                               errorCode);
                                 }
                            }
                        }
                    }
                    break;
                case TM_ICMP_TYPE_ECHO_RPL:
#ifdef TM_SNMP_MIB
/* Number of ICMP Echo Reply messages received. */
                    tm_context(tvIcmpData).icmpInEchoReps++;
#endif /* TM_SNMP_MIB */
/*
 * 9. If the ICMP type is echo reply we let the router code know about
 *    the echo reply if request was from router code, otherwise we get
 *    the corresponding ICMP socket calling tf4SocketLookup(TM_IP_ICMP,
 *    DestIpAddress, destPortNumber, 0, 0, TM_SOC_TREE_CL_FLAG), and call
 *    tfSocketIncomingPacket() which should:
 *    call the application callback function if there is any,
 *    otherwise queue the data to the socket.
 */
                    tm_bzero(&tupleDev, sizeof(tupleDev));
/* ICMP Identification number is local port */
                    tupleDev.tudLocalIpPort = (ttIpPort)icmphPtr->icmSUIdent;
/* Local ip address (from IP header destination address field) */
#ifdef TM_USE_IPV6
                    tm_6_addr_to_ipv4_mapped(
                            (tt4IpAddress)pktShrDataPtr->dataDestIpAddress,
                            &(tupleDev.tudLocalIpAddress));
#else /* !TM_USE_IPV6 */
                    tm_ip_copy(pktShrDataPtr->dataDestIpAddress,
                               tupleDev.tudLocalIpAddress);
#endif /* !TM_USE_IPV6 */
                    if (tupleDev.tudLocalIpPort == TM_ICMP_KERNEL_IDENT)
                    {
                        tfRtEchoReply(
#ifdef TM_USE_STRONG_ESL
                                      devPtr,
#endif /* TM_USE_STRONG_ESL */
                                      pktShrDataPtr->dataSrcIpAddress);
                    }
                    else
                    {
#if (defined(TM_USE_STRONG_ESL) || defined(TM_IGMP) || defined(TM_6_USE_MLD))
                        tupleDev.tudDevPtr = devPtr;
#endif /* TM_USE_STRONG_ESL || TM_IGMP || TM_6_USE_MLD */
                        tupleDev.tudProtocolNumber = TM_IP_ICMP;
                        tupleDev.tudAddressFamily = AF_INET;
                        socketPtr = tfSocketLookup(
                            &tupleDev,
                            TM_16BIT_ZERO );
                        if (socketPtr != TM_SOCKET_ENTRY_NULL_PTR)
                        {
                            errorCode = tfSocketIncomingPacket(socketPtr,
                                                               packetPtr);
                            if (errorCode == TM_ENOERROR)
                            {
                                tfSocketNotify(socketPtr, TM_SELECT_READ,
                                               TM_CB_RECV, 0);
                            }
                            needFree = TM_8BIT_ZERO;
                        }
                    }
                    break;
                case TM_ICMP_TYPE_MASK_RPL:
#ifdef TM_SNMP_MIB
/* Number of ICMP Address Mask Reply messages received. */
                 tm_context(tvIcmpData).icmpInAddrMaskReps++;
#endif /* TM_SNMP_MIB */
/*
 * 10. If the ICMP type is an address mask reply and if tvIpForward is 0,
 *    we check the validity of the mask (by checking that it is not all 1's
 *    , and that it is contiguous), and store
 *    it on the interface. (A router (tvIpForward != 0 should ignore address
 *    mask replies.)
 */
                    if (icmpLength >= (ttPktLen)TM_ICMP_ADDRM_LEN)
                    {
                        tm_ip_copy(icmphPtr->icmDUAddrMask, netMask);
                        prefixLen = tf4NetmaskToPrefixLen(netMask);
                        if (   (tm_context(tvIpForward) == 0)
/* Valid netmask is not all 1's, and is contiguous.  */
                            && (tm_ip_not_match(netMask, TM_IP_ALL_ONES))
                            && (tm_ip_match(tvRt32ContiguousBitsPtr[prefixLen],
                                            netMask))
                            )
                        {
#ifdef TM_SINGLE_INTERFACE_HOME
                            tm_ip_copy( netMask,
                                        tm_ip_dev_mask(devPtr, 0) );
                            tm_4_ip_dev_prefixLen(devPtr, 0) =
                                (tt8Bit)prefixLen;
#else /* ! TM_SINGLE_INTERFACE_HOME */
                            if (devPtr == pktShrDataPtr->dataDestDevPtr)
                            {
                                tm_ip_copy( netMask,
                                            tm_ip_dev_mask(devPtr,
                                                  packetPtr->pktMhomeIndex));
                                tm_4_ip_dev_prefixLen(
                                    devPtr, packetPtr->pktMhomeIndex) =
                                    (tt8Bit) tf4NetmaskToPrefixLen(netMask);
                            }
#ifdef TM_ERROR_CHECKING
                            else
                            {
                                tfKernelWarning("tfIpIncomingPacket",
                 "Address Mask Reply destination not on incoming interface");
                            }
#endif /* TM_ERROR_CHECKING */
#endif /* ! TM_SINGLE_INTERFACE_HOME */
                        }
                    }
                    break;
                case TM_ICMP_TYPE_RDISC_ADV:
#ifdef TM_SNMP_MIB
                    tm_context(tvIcmpData).icmpInRdiscAdv++;
#endif /* TM_SNMP_MIB */

                    if (devPtr->devFlag2 & TM_DEVF2_DISABLE_ICMP_RTR_ADVERT)
                    {
                        break;
                    }
/* 10.If the ICMP type is a router discovery advertisement and if
 *    tvIpForward is 0, we add the gateway as a default static route for the
 *    interface the packet came in. (A router (tvIpForward != 0) should
 *    ignore router discovery advertisements.) We also ignore router
 *    discovery advertisement if we have RIP?.
 */

/*
 * If Mobile IP is enabled on the incoming interface, pass this packet to the
 * Mobile IP module.
 */
#ifdef TM_MOBILE_IP4
                    if (devPtr->devMobileIp4VectPtr != (ttMobileVectPtr) 0)
                    {
                        mipIcmpFuncPtr =
                            devPtr->devMobileIp4VectPtr->mipIncomingFuncPtr;
                        if (mipIcmpFuncPtr != (ttMipIncomingFuncPtr) 0)
                        {

                            (*mipIcmpFuncPtr)(packetPtr);
                        }
                    }
#endif /* TM_MOBILE_IP4 */

                    if (    (tm_context(tvIpForward) == 0)
                         && (tm_context(tvRipOn) == 0) )
                    {
/* Router code will add default gateway only if there is no default */
/* We only support one default gateway. So only get the first gateway from
 * the ICMP router advertisement
 */
                        if (icmpLength >= TM_ICMP_RDISC_ADV_MIN_LEN)
                        {
                            tm_ntohs(icmphPtr->icmSURdiscLife, lifeTime);
                            tm_ip_copy(icmphPtr->icmDURdiscGwy, gatewayIpAddr);
                            (void)tf4RtAddRoute(
                                devPtr, /* device */
/* Default destination Ip address */
                                TM_IP_ZERO,
/* Default destination network address */
                                TM_IP_ZERO,
/* gateway */
                                gatewayIpAddr,
                                TM_16BIT_ZERO,
                                TM_16BIT_ZERO, /* 0 */
                                1, /* metrics */
/* time to live for the route */
                                ((tt32Bit)lifeTime)*1000,
/* Entry created by router discovery */
                                TM_RTE_RDISC|TM_RTE_INDIRECT,
/* need to lock the tree */
                                TM_8BIT_YES);
                        }
                    }
                    break;
                case TM_ICMP_TYPE_MS_SYNC_REQ:
/* ICMP MS Sync request packet */
                    (void)tfIcmpCreatePacket(
                        devPtr,
                        packetPtr->pktMhomeIndex,
                        TM_OUTPUT_DEVICE_FLAG,
                        tm_4_ip_addr(pktShrDataPtr->dataSpecDestIpAddress),
                        pktShrDataPtr->dataSrcIpAddress,
                        TM_ICMP_TYPE_MS_SYNC_RPL,
                        0,
                        tm_4_ip_addr(pktShrDataPtr->dataSpecDestIpAddress)
                        );
                    break;
            }
        }
#ifdef TM_SNMP_MIB
        else
        {
/*
 * Number of ICMP messages which the entity received but determined as
 * having ICMP-specific errors (bad ICMP checksums, bad length, etc.).
 */
            tm_context(tvIcmpData).icmpInErrors++;
        }
#endif /* TM_SNMP_MIB */
    }
#ifdef TM_SNMP_MIB
    else
    {
/*
 * Number of ICMP messages which the entity received but determined as
 * having ICMP-specific errors (bad ICMP checksums, bad length, etc.).
 */
        tm_context(tvIcmpData).icmpInErrors++;
    }
#endif /* TM_SNMP_MIB */
    if (socketPtr != TM_SOCKET_ENTRY_NULL_PTR)
    {
/* Incoming thread no longer has ownership */
        tm_socket_checkin_unlock(socketPtr);
    }
#ifndef TM_SINGLE_INTERFACE_HOME
freeIncoming:
#endif /* TM_SINGLE_INTERFACE_HOME */
#ifdef TM_USE_RAW_SOCKET
    if (rawPacketPtr != TM_PACKET_NULL_PTR)
    {
        tf4RawIncomingPacket(rawPacketPtr);
    }
#endif /* TM_USE_RAW_SOCKET */
    if (needFree != TM_8BIT_ZERO)
    {
/* socket was not locked, or has just been unlocked */
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    }
#if (defined(TM_USE_DRV_SCAT_RECV) || defined(TM_USE_IPV6))
icmpIncomingExit:
#endif /* TM_USE_DRV_SCAT_RECV || TM_USE_IPV6 */
    return;
#undef lNextHopMtu
}

/*
 * tfIcmpReplyPacket Function Description
 * 1. Compute the ICMP checksum
 * 2. If pktSrcIpAddress is 0 , make it a limited broadcast
 * 3. We point to IP header, swap source and destination addresses in
 *    packetPtr, initialize iphTtl with TM_IP_MAX_TTL, iphTos
 *    with TM_IP_DEF_TOS, add to pktFlags:
 *    TM_IP_DEST_ROUTE_SET|TM_OUTPUT_DEVICE| TM_ETHERNET_ADDRESS_FLAG for LAN,
 *    TM_IP_DEST_ROUTE_SET|TM_OUTPUT_DEVICE otherwise, and then call
 *    tfIpSendPacket(packetPtr, (ttLockEntryPtr)0);
 * Parameters
 * Parameter       Description
 * packetPtr       pointer to the incoming ttPacket. The ttPacket structure
 *                 contains the source IP address and specific Destination
 *                 IP address of the original packet
 * ipHeaderLen     IP header length
 *
 * Returns
 * Value   Meaning
 * None
 */
static tt8Bit tfIcmpReplyPacket (ttPacketPtr packetPtr, tt8Bit updateChecksum)
{
    ttIcmpHeaderPtr icmphPtr;
    ttIpHeaderPtr   iphPtr;
    ttSharedDataPtr pktShrDataPtr;
#ifdef TM_LOCK_NEEDED
    ttLockEntryPtr  lockEntryPtr;
#endif /* TM_LOCK_NEEDED */
    int             errorCode;
    tt16Bit         ipHdrLen;
#ifndef TM_LOOP_TO_DRIVER
#ifndef TM_SINGLE_INTERFACE_HOME
    tt16Bit         tempMhomeIndex;
#endif /* !TM_SINGLE_INTERFACE_HOME */
#endif /* !TM_LOOP_TO_DRIVER */
    tt8Bit          needFree;
#ifdef TM_4_USE_SCOPE_ID
    tt6IpAddress        ipv4MappedDestAddr;
    ttDeviceEntryPtr    inDevPtr;
#endif /* TM_4_USE_SCOPE_ID */
#ifndef TM_LOOP_TO_DRIVER
#ifndef TM_SINGLE_INTERFACE_HOME
    ttDeviceEntryPtr    tmpDevPtr;
#endif /* !TM_SINGLE_INTERFACE_HOME */
#endif /* !TM_LOOP_TO_DRIVER */

#ifdef TM_4_USE_SCOPE_ID
    inDevPtr = packetPtr->pktDeviceEntryPtr;
#endif /* TM_4_USE_SCOPE_ID */
    needFree = TM_8BIT_NO;
#ifdef TM_SNMP_MIB
/* Number of ICMP messages which this entity attempted to send. */
    tm_context(tvIcmpData).icmpOutMsgs++;
/* Number of ICMP Echo Reply messages sent. */
    tm_context(tvIcmpData).icmpOutEchoReps++;
#endif /* TM_SNMP_MIB */
#ifdef TM_DEV_RECV_OFFLOAD
/* If recv checksum was offloaded, reset offload pointer */
    packetPtr->pktDevOffloadPtr = (ttVoidPtr)0;
#endif /* TM_DEV_RECV_OFFLOAD */
    icmphPtr = (ttIcmpHeaderPtr)packetPtr->pktLinkDataPtr;
/* Compute or update the ICMP checksum */
    if (updateChecksum != TM_8BIT_ZERO)
    {
/* Update the checksum for echo reply when there is no packet truncation */
        icmphPtr->icmChecksum = tfIpUpdateChecksum(
                                            icmphPtr->icmChecksum,
                                            (tt32Bit)tm_const_htons(0x800));
    }
    else
/* Compute the checksum for all other cases */
    {
        icmphPtr->icmChecksum = 0;
        icmphPtr->icmChecksum = tfPacketChecksum(
            packetPtr,
            packetPtr->pktChainDataLength,
            TM_16BIT_NULL_PTR,
            TM_16BIT_ZERO);
    }
/*  If pktSrcIpAddress is 0 , make it a limited broadcast */
    pktShrDataPtr = packetPtr->pktSharedDataPtr;
    if (tm_ip_match(pktShrDataPtr->dataSrcIpAddress, TM_IP_ZERO))
    {
        tm_ip_copy(TM_IP_LIMITED_BROADCAST, pktShrDataPtr->dataSrcIpAddress);
    }
/* save ipHdrLen to local variable (used 3 times in tm_pkt_llp_hdr) */
    ipHdrLen = packetPtr->pktIpHdrLen;
/* Point to new IP header */
    tm_pkt_llp_hdr(packetPtr, ipHdrLen);
    iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
#ifndef TM_LOOP_TO_DRIVER
#ifndef TM_SINGLE_INTERFACE_HOME
    tmpDevPtr = tfIfaceMatch(pktShrDataPtr->dataSrcIpAddress, tfMhomeAddrMatch,
                             &tempMhomeIndex);
#endif /* !TM_SINGLE_INTERFACE_HOME */
#endif /* !TM_LOOP_TO_DRIVER */
    if (    (tm_context(tvIcmpRouteReply) == TM_8BIT_NO) /* default */
#ifndef TM_LOOP_TO_DRIVER
#ifndef TM_SINGLE_INTERFACE_HOME
         && (tmpDevPtr == TM_DEV_NULL_PTR)
#endif /* !TM_SINGLE_INTERFACE_HOME */
#endif /* !TM_LOOP_TO_DRIVER */
       )
    {
        errorCode = TM_ENOERROR;
#ifdef TM_LOCK_NEEDED
        lockEntryPtr = (ttLockEntryPtr)0;
#endif /* TM_LOCK_NEEDED */
/*Swap source and destination addresses (using specific destination address)*/
        tm_ip_copy(pktShrDataPtr->dataSrcIpAddress, iphPtr->iphDestAddr);
        tm_ip_copy(tm_4_ip_addr(pktShrDataPtr->dataSpecDestIpAddress),
                   iphPtr->iphSrcAddr );
/* Initialize iphTos with default */
        iphPtr->iphTos = tm_context(tvIpDefTos);
/*
 * Add to pktFlags:   TM_IP_DEST_ROUTE_SET|TM_OUTPUT_DEVICE_FLAG|
 *                    TM_ETHERNET_ADDRESS_FLAG for LAN,
 *                    TM_IP_DEST_ROUTE_SET|TM_OUTPUT_DEVICE_FLAG otherwise.
 *
 */
        if (tm_4_ll_is_lan(packetPtr->pktDeviceEntryPtr))
        {
            packetPtr->pktFlags = TM_IP_DEST_ROUTE_SET |
                                   TM_OUTPUT_DEVICE_FLAG |
                                   TM_ETHERNET_ADDRESS_FLAG;
        }
        else
        {
            packetPtr->pktFlags = TM_IP_DEST_ROUTE_SET | TM_OUTPUT_DEVICE_FLAG;
        }
    }
    else
    {
#ifdef TM_LOCK_NEEDED
/* Lock IP Send cache */
        lockEntryPtr = &tm_context(tvRtIpSendCacheLock);
        tm_call_lock_wait(lockEntryPtr);
#endif /* TM_LOCK_NEEDED */
/* Exchange source and destination Ip addresses. Find return route. */
#ifdef TM_4_USE_SCOPE_ID
        tm_6_addr_to_ipv4_mapped(
            pktShrDataPtr->dataSrcIpAddress, &ipv4MappedDestAddr);
        tm_4_dev_scope_addr(&ipv4MappedDestAddr, inDevPtr);
#endif /* TM_4_USE_SCOPE_ID */
#ifdef TM_USE_STRONG_ESL
        tm_context(tvRtIpSendCache).rtcDevPtr = packetPtr->pktDeviceEntryPtr;
#endif /* TM_USE_STRONG_ESL */
        errorCode = tfIpDestToPacket( packetPtr,
#ifdef TM_4_USE_SCOPE_ID
/* source */               &pktShrDataPtr->dataSpecDestIpAddress,
/* destination */          &ipv4MappedDestAddr,
#else /* ! TM_4_USE_SCOPE_ID */
/* source */               tm_4_ip_addr(pktShrDataPtr->dataSpecDestIpAddress),
/* destination */          pktShrDataPtr->dataSrcIpAddress,
#endif /* ! TM_4_USE_SCOPE_ID */
/* initialize iphTos */    tm_context(tvIpDefTos),
                           &tm_context(tvRtIpSendCache),
                           iphPtr);
        if (errorCode != TM_ENOERROR)
        {
/* Unlock the IP send cache */
            tm_call_unlock(&tm_context(tvRtIpSendCacheLock));
/* Free the packet */
            needFree = TM_8BIT_YES;
        }
    }
    if (errorCode == TM_ENOERROR)
    {
/*
 * initialize iphTtl with TM_IP_MAX_TTL
 * (iphUlp same as request)
 */
        iphPtr->iphTtl = TM_IP_MAX_TTL;
        iphPtr->iphFlagsFragOff = TM_16BIT_ZERO;
/* send the packet */
        (void)tfIpSendPacket(
            packetPtr,
#ifdef TM_LOCK_NEEDED
            lockEntryPtr
#else /* !TM_LOCK_NEEDED */
            (ttLockEntryPtr)0
#endif /* TM_LOCK_NEEDED */
#ifdef TM_USE_IPSEC
            , (ttPktLenPtr)0
#ifdef TM_USE_IPSEC_TASK
            , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
#endif /* TM_USE_IPSEC */
            );
    }
    return needFree;
}


/*
 * tfIcmpCreatePacket() Function Description
 * This function allows any part of the stack to send an ICMP message.
 * Typically this will  be used by the router code. At this time we only
 * support ICMP address mask request/reply and Router discovery
 * advertisement and solicitations, and ICMP echo request (for gateway
 * pings):
 * 1. We first create a new ttPacket/ttShared data with IP and LL header
 *    sizes, and TM_ICMP_MAX_CREATE_HEADER data size (Right now ICMP Router
 *    discovery advertisement type will yield the biggest ICMP header).
 * 2. Second word is zero for router discovery solicitation, for router
 *    discovery advertisement it is number of addresses (1), lifetime(1800
 *    or 30mn), and address entry size (2). For all other ICMP packets
 *    it is Sequence number / Unique Identification number.
 *    In addition for address mask reply and router discovery
 *    advertisement we fill in the icmAddressMask field with the passed
 *    ipAddress. For router discovery advertisement we also fill in the
 *    icmRouterPrefLevel with the default TM_ICMP_RDISC_DEF_PREF.
 *    We initialize icmpLength with 16 for router discovery advertisement,
 *    12 for address mask reply and request, and with 8 for every other case.
 *    We update the packet lengths with the computed icmpLength, and
 *    we compute the ICMP checksum.
 * 3. We point to the IP header, initialize iphTtl with TM_IP_MAX_TTL
 *    for echo requests, 1 otherwise, iphUlp with TM_IP_ICMP, pktIpHdrLen
 *    with TM_4PAK_IP_MIN_HDR_LEN, and we call
 *    tfIpSendPacket(packetPtr, (ttLockEntryPtr)0);
 *
 * Parameters
 * Parameter       Description
 * devPtr          Pointer to device through which new ICMP packet is to be
 *                 sent
 * mhomeIndex      Multihome Index of outgoing interface
 * srcAddress      Source IP address
 * destAddress     Destination IP address
 * icmpType        ICMP type field
 * icmpCode        ICMP code field
 * ipAddr          Router IP address (for router discovery advertisement)  or
 *                 address mask for address mask reply
 *
 * Returns
 * Value   Meaning
 * None
 */
LINT_UNACCESS_SYM_BEGIN(icmpSeqno)
int tfIcmpCreatePacket (ttDeviceEntryPtr   devPtr,
                        tt16Bit            mhomeIndex,
                        tt16Bit            flags,
                        tt4IpAddress       ipSrcAddr,
                        tt4IpAddress       ipDestAddr,
                        tt8Bit             icmpType,
                        tt8Bit             icmpCode,
                        tt4IpAddress       ipAddr)
{
    ttPacketPtr        packetPtr;
    ttIcmpHeaderPtr    icmphPtr;
    ttIpHeaderPtr      iphPtr;
    tt16BitPtr         data16Ptr;
    tt16Bit            icmpLength;
    tt16Bit            icmpSeqno;
    int                errorCode;

#ifdef TM_SNMP_MIB
/* Number of ICMP messages which this entity attempted to send. */
    tm_context(tvIcmpData).icmpOutMsgs++;
#endif /* TM_SNMP_MIB */
/*
 *  We first create a new ttPacket/ttShared data with IP and LL header
 *  sizes, and TM_ICMP_MAX_CREATE_HEADER data size (Right now ICMP Router
 *  discovery advertisement type will yield the biggest ICMP header).
 */
    packetPtr = tfGetSharedBuffer(
                            (int)(TM_4_IP_MIN_HDR_LEN + TM_LL_MAX_HDR_LEN),
                            (ttPktLen)TM_ICMP_MAX_CREATE_HEADER,
                            TM_16BIT_ZERO);
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
/*
 * pktDevicePtr, pktMhomeIndex, pktFlags, pktSrcIpAddress,
 * pktSpecDestIpAddress, pktEthernetAddress initialized with passed
 * parameters.
 */
        packetPtr->pktDeviceEntryPtr = devPtr;
        packetPtr->pktMhomeIndex = mhomeIndex;
        packetPtr->pktFlags = flags;
/* Point to ICMP header */
        icmphPtr = (ttIcmpHeaderPtr)packetPtr->pktLinkDataPtr;
/* We then fill in the ICMP header with the passed type and code parameters.*/
        icmphPtr->icmType = icmpType;
        icmphPtr->icmCode = icmpCode;
        icmphPtr->icmChecksum = 0;
/*
 * Second word is zero for router discovery solicitation, for router
 * discovery advertisement it is number of addresses (1), lifetime(1800
 * or 30mn), and address entry size (2). For all other ICMP packets
 * it is Sequence number / Unique Identification number.
 * In addition for address mask reply and router discovery
 * advertisement we fill in the icmAddressMask field with the passed
 * ipAddress. For router discovery advertisement we also fill in the
 * icmRouterPrefLevel with the default TM_ICMP_RDISC_DEF_PREF.
 * We initialize icmpLength with 16 for router discovery advertisement,
 * 12 for address mask reply and request, and with 8 for every other case.
 * We update the packet lengths with the computed icmpLength, and
 * we compute the ICMP checksum.
 */
#ifdef TM_CASE
        switch (icmpType)
        {
            case TM_ICMP_TYPE_RDISC_SOL:
#ifdef TM_SNMP_MIB
                tm_context(tvIcmpData).icmpOutRdiscSol++;
#endif /* TM_SNMP_MIB */
                icmphPtr->icmSUUnused = TM_UL(0);

                icmpLength = tm_byte_count(TM_4PAK_ICMP_MIN_LEN);
                break;
            case TM_ICMP_TYPE_RDISC_ADV:
#ifdef TM_SNMP_MIB
                tm_context(tvIcmpData).icmpOutRdiscAdv++;
#endif /* TM_SNMP_MIB */
                icmphPtr->icmSURdiscNumAddrs = 1;
                icmphPtr->icmSURdiscAddrSiz = 2;
                icmphPtr->icmSURdiscLife = TM_ICMP_RDISC_DEF_LIFE;
                icmphPtr->icmDURdiscPref = TM_ICMP_RDISC_DEF_PREF;
                icmpLength = TM_ICMP_RDISC_ADV_MIN_LEN;
/* Gateway for Router discovery */
                tm_ip_copy(ipAddr, icmphPtr->icmDURdiscGwy);
                break;
            case TM_ICMP_TYPE_MASK_RPL:
#ifdef TM_SNMP_MIB
/* Number of ICMP Address Mask Reply messages sent. */
                tm_context(tvIcmpData).icmpOutAddrMaskReps++;
#endif /* TM_SNMP_MIB */
                icmpLength = TM_ICMP_ADDRM_LEN;
/* Address mask for MASK_RPL */
                tm_ip_copy(ipAddr, icmphPtr->icmDUAddrMask);
identSeq:
                tm_kernel_set_critical;
                icmpSeqno = ++tm_context(tvIcmpSeqno);
                tm_kernel_release_critical;
                icmphPtr->icmSUIdent = TM_ICMP_KERNEL_IDENT;
                tm_htons(icmpSeqno,icmphPtr->icmSUSeqno);
                break;
            case TM_ICMP_TYPE_MASK_REQ:
#ifdef TM_SNMP_MIB
/* Number of ICMP Address Mask Request messages sent. */
                tm_context(tvIcmpData).icmpOutAddrMasks++;
#endif /* TM_SNMP_MIB */
                icmpLength = TM_ICMP_ADDRM_LEN;
                goto identSeq;
            case TM_ICMP_TYPE_MS_SYNC_RPL:
                icmpLength = (tt16Bit)(TM_4PAK_ICMP_MIN_LEN + TM_MS_SYNC_SIZE);
                data16Ptr = (tt16BitPtr)&(icmphPtr->icmDUData[0]);
                *data16Ptr++ = (tt16Bit)(~TM_MS_SYNC_1);
                *data16Ptr++ = (tt16Bit)(~TM_MS_SYNC_2);
                *data16Ptr   = (tt16Bit)(~TM_MS_SYNC_3);
                goto identSeq;
/* echo request */
            default:
#ifdef TM_SNMP_MIB
/* Number of ICMP Echo (request) messages sent. */
                tm_context(tvIcmpData).icmpOutEchos++;
#endif /* TM_SNMP_MIB */
                icmpLength = TM_4PAK_ICMP_MIN_LEN;
                goto identSeq;
        }
#else /* !TM_CASE (use if/else, smaller code size) */
        if ( icmpType == TM_ICMP_TYPE_RDISC_SOL )
        {
            icmphPtr->icmSUUnused = TM_UL(0);
            icmpLength = TM_4PAK_ICMP_MIN_LEN;
        }
        else
        {
            if (icmpType == TM_ICMP_TYPE_RDISC_ADV)
            {
                icmphPtr->icmSURdiscNumAddrs = 1;
                icmphPtr->icmSURdiscAddrSiz = 2;
                icmphPtr->icmSURdiscLife = TM_ICMP_RDISC_DEF_LIFE;
                icmphPtr->icmDURdiscPref = TM_ICMP_RDISC_DEF_PREF;
                icmpLength = TM_ICMP_RDISC_ADV_MIN_LEN;
/* Gateway for Router discovery */
                tm_ip_copy(ipAddr, icmphPtr->icmDURdiscGwy);
            }
            else
            {
                if (icmpType == TM_ICMP_TYPE_MASK_RPL )
                {
#ifdef TM_SNMP_MIB
/* Number of ICMP Address Mask Reply messages sent. */
                    tm_context(tvIcmpData).icmpOutAddrMaskReps++;
#endif /* TM_SNMP_MIB */
                    icmpLength = TM_ICMP_ADDRM_LEN;
/* Address mask for MASK_RPL */
                    tm_ip_copy(ipAddr, icmphPtr->icmDUAddrMask);
identSeq:
                    tm_kernel_set_critical;
                    icmpSeqno = ++tm_context(tvIcmpSeqno);
                    tm_kernel_release_critical;
                    icmphPtr->icmSUIdent = TM_ICMP_KERNEL_IDENT;
                    tm_htons(icmpSeqno,icmphPtr->icmSUSeqno);
                }
                else
                {
                    if (icmpType == TM_ICMP_TYPE_MASK_REQ)
                    {
#ifdef TM_SNMP_MIB
/* Number of ICMP Address Mask Request messages sent. */
                        tm_context(tvIcmpData).icmpOutAddrMasks++;
#endif /* TM_SNMP_MIB */
                        icmpLength = TM_ICMP_ADDRM_LEN;
                        goto identSeq;
                    }
                    else
                    {
                        if (icmpType == TM_ICMP_TYPE_MS_SYNC_RPL)
                        {
                            icmpLength = (tt16Bit)
                                (tm_byte_count(TM_4PAK_ICMP_MIN_LEN) +
                                 TM_MS_SYNC_SIZE);
                            data16Ptr = (tt16BitPtr)&(icmphPtr->icmDUData[0]);
/* PRQA: QAC Message 277: The variable is used as a bit pattern */
/* PRQA S 277 L1 */
                            *data16Ptr++ = (tt16Bit)(~TM_MS_SYNC_1);
                            *data16Ptr++ = (tt16Bit)(~TM_MS_SYNC_2);
                            *data16Ptr   = (tt16Bit)(~TM_MS_SYNC_3);
/* PRQA L:L1 */
                            goto identSeq;
                        }
                        else
                        {
/* echo request */
#ifdef TM_SNMP_MIB
/* Number of ICMP Echo (request) messages sent. */
                            tm_context(tvIcmpData).icmpOutEchos++;
#endif /* TM_SNMP_MIB */
                            icmpLength = tm_byte_count(TM_4PAK_ICMP_MIN_LEN);
                            goto identSeq;
                        }
                    }
                }
            }
        }
#endif /* TM_CASE */
/* Update lengths with computed length */
        packetPtr->pktChainDataLength = (ttPktLen)icmpLength;
        packetPtr->pktLinkDataLength = (ttPktLen)icmpLength;
/* Checksum */
        icmphPtr->icmChecksum = 0;
        icmphPtr->icmChecksum = tfPacketChecksum(
            packetPtr,
            (ttPktLen)icmpLength,
            TM_16BIT_NULL_PTR,
            TM_16BIT_ZERO);

/* Add ip header to icmp length */
        icmpLength += tm_byte_count(TM_4PAK_IP_MIN_HDR_LEN);
        packetPtr->pktChainDataLength = (ttPktLen)icmpLength;
        packetPtr->pktLinkDataLength = (ttPktLen)icmpLength;
/* point to IP header */
        iphPtr = (ttIpHeaderPtr)(packetPtr->pktLinkDataPtr -
                                 TM_4PAK_IP_MIN_HDR_LEN);
        packetPtr->pktLinkDataPtr = (tt8BitPtr)iphPtr;
        tm_ip_copy(ipSrcAddr, iphPtr->iphSrcAddr);
        tm_ip_copy(ipDestAddr, iphPtr->iphDestAddr);
/*
 *  We initialize iphTtl with TM_IP_MAX_TTL for echo requests, 1 otherwise.
 *  We initialize iphUlp with TM_IP_ICMP, and we call
 *  tfIpSendPacket(packetPtr, (ttLockEntryPtr)0);
 */
        if (icmpType == TM_ICMP_TYPE_ECHO_REQ)
        {
            iphPtr->iphTtl = TM_IP_MAX_TTL;
        }
        else
        {
            iphPtr->iphTtl = 1;
        }
        iphPtr->iphUlp = TM_IP_ICMP;
        iphPtr->iphFlagsFragOff = TM_16BIT_ZERO;
        packetPtr->pktIpHdrLen = TM_4PAK_IP_MIN_HDR_LEN;
        errorCode = tfIpSendPacket(
            packetPtr,
            (ttLockEntryPtr)0
#ifdef TM_USE_IPSEC
            , (ttPktLenPtr)0
#ifdef TM_USE_IPSEC_TASK
            , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
#endif /* TM_USE_IPSEC */
            );
    }
    else
    {
        errorCode = TM_ENOBUFS;
#ifdef TM_SNMP_MIB
/*
 * Number of ICMP messages which this entity did not send due to problems
 * discovered within ICMP such as a lack of buffers.
 */
        tm_context(tvIcmpData).icmpOutErrors++;
#endif /* TM_SNMP_MIB */
    }
    return errorCode;
}
LINT_UNACCESS_SYM_END(icmpSeqno)

/*
 * tfRawSendPacket Function Description
 * This function is called from the socket interface, when the user has
 * opened a raw socket and sends some data. For example an application
 * could send an ICMP echo request.
 * I. If IP hdr is included
 *   1. Set the TM_IP_FORWARD_FLAG bit in the ttPacket flags field to prevent
 *      the IP send packet function from altering the IP header.
 *   2. Find the route
 * II. Else If iphdr is not included:
 *   1. We decrease the pktLinkDataPtr by the IP header length, and increase
 *      pktLinkDataLength and pktChainDataLength by that same value
 *   2. We fill in some IP header fields (ttl/tos from socket entry)
 *      ULP protocol, Source IP address and destination IP address).
 * III.
 *   We then call tfIpSendPacket(packetPtr, (ttLockEntryPtr)0);
 *
 * Parameters
 * Parameter       Description
 * socketPtr       Pointer to the socket entry
 * packetPtr       pointer to ttPacket where pktLinkDataPtr points to the
 *                 IP header, pktChainDataLength gives the size of the IP
 *                 datagram,  pktLinkDataLength includes also the IP header.
 *                 pktIpHdrLen is initialized if this call is not made
 *                 through a socket.
 *
 * Returns
 * Value   Meaning
 */

int tfRawSendPacket ( ttSocketEntryPtr socketPtr,
                      ttPacketPtr      packetPtr )
{
    ttIpAddressPtr      ipSrcAddrPtr;
    ttIpHeaderPtr       iphPtr;
    tt4IpAddress        ip4SrcAddr;
    int                 errorCode;
    tt16Bit             ipHdrLength;
    tt8Bit              protocol;
#ifdef TM_4_USE_SCOPE_ID
    tt6IpAddress        ipv4MappedSrcAddr;
    tt6IpAddress        ipv4MappedDestAddr;
#endif /* TM_4_USE_SCOPE_ID */
    tt16Bit             ipId;

/*
 * user call. socketPtr always non null.
 */
    protocol = socketPtr->socProtocolNumber;
#ifdef TM_SNMP_MIB
    if (protocol == (tt8Bit)IPPROTO_ICMP)
    {
/* Number of ICMP messages which this entity attempted to send. */
        tm_context(tvIcmpData).icmpOutMsgs++;
    }
#endif /* TM_SNMP_MIB */
    if ( tm_16bit_one_bit_set(socketPtr->socOptions, SO_IPHDRINCL) )
/* User has included the IP header */
    {
/*
 * Set the TM_IP_FORWARD_FLAG bit in the ttPacket flags field to prevent
 * the IP send packet function from altering the IP header.
 */
        packetPtr->pktFlags |= TM_IP_FORWARD_FLAG;
        iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
        ipHdrLength = tm_ip_hdr_len(iphPtr->iphVersLen);
/* Check IP header length */
        if (    (ipHdrLength > TM_4_IP_MAX_HDR_LEN)
             || (ipHdrLength < TM_4_IP_MIN_HDR_LEN) )
        {
/*
 * If IP header is bigger, or smaller than allowed by RFC's, free the
 * user buffers, and return TM_EFAULT
 */
            errorCode = TM_EFAULT;
        }
        else
        {
            packetPtr->pktIpHdrLen = (tt8Bit)tm_packed_byte_count(ipHdrLength);
#ifdef TM_4_USE_SCOPE_ID
/* if these addresses are link-local scope, we don't know here what interface
   to scope them to */
            tm_6_addr_to_ipv4_mapped(
                iphPtr->iphSrcAddr, &ipv4MappedSrcAddr);
            tm_6_addr_to_ipv4_mapped(
                iphPtr->iphDestAddr, &ipv4MappedDestAddr);
#endif /* TM_4_USE_SCOPE_ID */
            errorCode = tfIpDestToPacket(
                packetPtr,
#ifdef TM_4_USE_SCOPE_ID
                &ipv4MappedSrcAddr,
                &ipv4MappedDestAddr,
#else /* ! TM_4_USE_SCOPE_ID */
                iphPtr->iphSrcAddr,
                iphPtr->iphDestAddr,
#endif /* ! TM_4_USE_SCOPE_ID */
                iphPtr->iphTos,
                &socketPtr->socRteCacheStruct,
                iphPtr );
#ifdef TM_SNMP_MIB
/*
 * Number of IP datagrams which local IP user-protocols (including ICMP)
 * supplied to IP in requests for transmission (does not include forwarded
 * packets). Since we set the TM_IP_FORWARD_FLAG, we need to increase
 * this MIB variable here.
 */
            if (errorCode == TM_ENOERROR)
            {
                tm_context(tvIpData).ipOutRequests++;
#ifdef TM_USE_NG_MIB2
                 tm_64Bit_incr(packetPtr->pktDeviceEntryPtr->
                                dev4Ipv4Mib.ipIfStatsHCOutRequests);
                 tm_64Bit_incr(tm_context(tvDevIpv4Data).ipIfStatsHCOutRequests);
#endif /* TM_USE_NG_MIB2 */
            }
#endif /* TM_SNMP_MIB */

            if (iphPtr->iphId == TM_16BIT_ZERO)
            {
/* if the IP header identification field is 0, then set it */
                tm_kernel_set_critical;
                ipId = ++tm_context(tvIpId);
                tm_kernel_release_critical;
                tm_htons(ipId, iphPtr->iphId);
            }

/* compute the IP packet header checksum */
            iphPtr->iphChecksum = (tt16Bit)0;
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
                            TM_16BIT_ZERO);
            }
        }
    }
    else
    {
/* Do not allow IP header options from user. */
        packetPtr->pktIpHdrLen = TM_4PAK_IP_MIN_HDR_LEN;
        tm_pkt_llp_hdr(packetPtr, TM_4PAK_IP_MIN_HDR_LEN);
        iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
        iphPtr->iphUlp = protocol;
        iphPtr->iphFlagsFragOff = TM_16BIT_ZERO; /* need to check flags */

        iphPtr->iphTtl = socketPtr->socIpTtl;
/*
 * Fill in source, destination addresses and TOS in IP header,
 */
        if ( tm_16bit_one_bit_set(packetPtr->pktUserFlags,
                                  TM_PKTF_USER_SEND_FROM) )
        {
/* User selected source address for that call */
            ipSrcAddrPtr = &(packetPtr->pktSharedDataPtr->dataFromIpAddress);
        }
        else
        {
/* Use socket source address for IP header source */
            ipSrcAddrPtr = &(socketPtr->socOurIfIpAddress);
        }
#ifdef TM_USE_IPV6
        if (tm_6_addr_is_ipv4_mapped(ipSrcAddrPtr))
        {
/* IPv4 mapped IPv6 address */
            tm_ip_copy(tm_4_ip_addr(*ipSrcAddrPtr), ip4SrcAddr);
        }
        else
        {
/* IPv6 native address. Pick a configured IPv4 address as a source address */
            tm_ip_copy(TM_IP_ZERO, ip4SrcAddr);
        }
#else /* !TM_USE_IPV6 */
        tm_ip_copy(tm_4_ip_addr(*ipSrcAddrPtr), ip4SrcAddr);
#endif /* dual */
        errorCode = tfIpDestToPacket(
            packetPtr,
#ifdef TM_4_USE_SCOPE_ID
            ipSrcAddrPtr,
            &socketPtr->socPeerIpAddress,
#else /* ! TM_4_USE_SCOPE_ID */
            ip4SrcAddr,
            tm_4_ip_addr(socketPtr->socPeerIpAddress),
#endif /* ! TM_4_USE_SCOPE_ID */
#ifdef TM_USE_QOS
            packetPtr->pktIpTos,
#else /* !TM_USE_QOS */
            socketPtr->socIpTos,
#endif /* !TM_USE_QOS */
            &socketPtr->socRteCacheStruct,
            iphPtr);
    }
    if (errorCode == TM_ENOERROR)
    {
        errorCode = tfIpSendPacket(
            packetPtr,
            &socketPtr->socLockEntry
#ifdef TM_USE_IPSEC
            , (ttPktLenPtr)0
#ifdef TM_USE_IPSEC_TASK
            , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
#endif /* TM_USE_IPSEC */
            );
    }
    else
    {
/* Free the packet if we could not send it */
        tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
    }
    return errorCode;
}

/*
 * tfIcmpErrPacket() Function Description:
 * This function is called when we need to send an error message in
 * response to an incoming IP datagram.
 * 1. To prevent broadcast storms, the ICMP code will check the following
 *    (from RFC 1122) before sending an ICMP error message. It will discard
 *    the packet and return if one of the following statements about the
 *    passed ttPacket is true:
 *    1) an ICMP error message, or
 *    2) a datagram destined to an IP broadcast or IP multicast address, or
 *    3) a datagram sent as a link-layer broadcast, or
 *    4) a non-initial fragment, or
 *    5) a datagram whose source address does not define a single host
 *       -- e.g., a zero address,
 *       or a broadcast address (already checked in tfIpIncomingPacket()),
 *       or a multicast address (already checked in tfIpIncomingPacket()),
 *       or a Class E address (already checked in tfIpIncomingPacket()),
 *       or a loopback address.
 * 2. To build the ICMP error message, we allocate a new ttPacket
 *    with Header size IP (20)+ LL (16) and  data size TM_ICMP_MAX_ERROR_MSG.
 * 3. We then fill in the first 2 words of the ICMP header. First word:
 *    icmCode and icmType from the parameters. Second word: if type is
 *    ICMP redirect, we fill in the icmRouterIpAddress. For all other error
 *    messages, we zero the second word (icmUnused32), and in addition, if
 *    type is parameter problem, we fill in the icmParamProbOffset, else if
 *    type and code indicates a path MTU error message, we fill in the
 *    icmPathMtuStruct.
 * 4. We then copy the mininum of IP header + 8 bytes of data or actual
 *    IP length of the offending IP datagram in the icmDataUnion.icmIpHeader
 *    area.  (We get the IP length of the offending datagram from the
 *    old ttPacket pktLinkDataLength).
 *    We update pktLinkDataLength and pktChainDataLength of the new ttPacket
 *    with the length of the new ICMP packet. We then compute the ICMP
 *    checksum of the new ICMP packet.
 * 5. We initialize pktIpHdrLen with minimum IP header, copy
 *    pktDeviceEntryPtr from the old packet.
 *    We point to IP header, use source IP address of old IP header as
 *    our destination address, and use incoming device IP address as our
 *    source address, initialize iphTtl with TM_IP_MAX_TTL,
 *    iphTos with TM_IP_DEF_TOS, iphUlp with TM_IP_ICMP. We initialize
 *    pktEthernetAddress (if LAN) from the  old packet, initialize pktFlags
 *    with TM_OUTPUT_DEVICE or TM_OUTPUT_DEVICE| TM_ETHERNET_ADDRESS_FLAG (if
 *    LAN), add to pktFlags TM_IP_DEST_ROUTE_SET to bypass IP route
 *    cache, and call tfIpSendPacket(newPacketPtr, (ttLockEntryPtr)0) and
 *    free the old packet.
 *
 * Parameters
 * Parameter       Description
 * packetPtr       pointer to ttPacket where pktLinkDataPtr points to the
 *                 IP header, pktChainDataLength gives the size of the IP
 *                 datagram, pktFlags gives the link layer broadcast status,
 *                 pktLinkDataLength includes also the IP header,
 *                 pktDevicePtr points to the device the packet came in.
 * icmpType        ICMP type field
 * icmpCode        ICMP code field
 * icmpParam       ICMP parameter problem offset (to be stored
 *                         in second word)
 * outDevPtr       Outgoing device pointer. Allows to retrieve the
 *                 MTU for path discovery (to be stored in second word)
 * ipAddress       for ICMP redirect, IP address of the correct router (to be
 *                 stored in second word)
 *
 * Returns
 * Value   Meaning
 * None
 *
*/

LINT_UNACCESS_SYM_BEGIN(nextHopMtu)
void tfIcmpErrPacket(ttPacketPtr      packetPtr,
                     tt8Bit           icmpType,
                     tt8Bit           icmpCode,
                     tt32Bit          icmpParam,
                     ttDeviceEntryPtr outDevPtr,
                     tt4IpAddress     ipAddr)
{
#ifndef TM_SINGLE_INTERFACE_HOME
    int              errCode;
#endif /* TM_SINGLE_INTERFACE_HOME */
    ttIpHeaderPtr    iphPtr; /* old, and new */
    ttIcmpHeaderPtr  newIcmphPtr;
    ttPacketPtr      newPacketPtr;
    ttSharedDataPtr  pktShrDataPtr;
    ttDeviceEntryPtr devPtr;
#ifndef TM_SINGLE_INTERFACE_HOME
    ttDeviceEntryPtr tmpDevPtr;
#endif /* TM_SINGLE_INTERFACE_HOME */
    tt4IpAddress     srcIpAddr;
#ifndef TM_SINGLE_INTERFACE_HOME
    tt4IpAddress     dstIpAddr;
#endif /* TM_SINGLE_INTERFACE_HOME */
#ifdef TM_USE_DRV_SCAT_RECV
    int              contByteLength;
#endif /* TM_USE_DRV_SCAT_RECV */
    unsigned         newIcmpLength;
    tt16Bit          ipHdrLen;
    tt16Bit          nextHopMtu;
#ifndef TM_SINGLE_INTERFACE_HOME
    tt16Bit          mhomeIndex;
    tt16Bit          junk;
#endif /* TM_SINGLE_INTERFACE_HOME */
    tt8Bit           errIcmpType;

/*
 * 1. To prevent broadcast storms, the ICMP code will check the following
 * (from RFC 1122) before sending an ICMP error message. It will discard
 * the packet and return if one of the following statements about the
 * passed ttPacket is true:
 *  1) an ICMP error message, or
 *  2) a datagram destined to an IP broadcast or IP multicast address, or
 *  3) a datagram sent as a link-layer broadcast, or
 *  4) a non-initial fragment, or
 *  5) a datagram whose source address does not define a single host
 *     -- e.g., a zero address,
 *     or a broadcast address (already checked in tfIpIncomingPacket()),
 *     or a multicast address (already checked in tfIpIncomingPacket()),
 *     or a Class E address (already checked in tfIpIncomingPacket())
 *     or a loopback address
 */
    ipHdrLen = packetPtr->pktIpHdrLen;
#ifdef TM_USE_DRV_SCAT_RECV
/*
 * If ICMP header is not contiguous, make it contiguous. Assumes biggest
 * possible ICMP header
 */
    contByteLength = tm_icmp_min_err_header(ipHdrLen);
    if (tm_pkt_hdr_is_not_cont(packetPtr, contByteLength))
    {
        packetPtr = tfContiguousSharedBuffer(packetPtr,
                                             contByteLength);
        if (packetPtr == (ttPacketPtr)0)
        {
            goto icmpErrOut;
        }
    }
#endif /* TM_USE_DRV_SCAT_RECV */
    pktShrDataPtr = packetPtr->pktSharedDataPtr;
    iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
    devPtr = packetPtr->pktDeviceEntryPtr;
/* Source IP address */
    tm_ip_copy(iphPtr->iphSrcAddr, srcIpAddr);
    if (iphPtr->iphUlp == TM_IP_ICMP)
    {
        errIcmpType =((ttIcmpHeaderPtr)((tt8BitPtr)iphPtr+ipHdrLen))->icmType;
    }
    else
    {
        errIcmpType = 0;
    }
    if (
/* tm_icmp_error_type(type) */
            (errIcmpType == TM_ICMP_TYPE_REDIRECT)
         || (errIcmpType == TM_ICMP_TYPE_UNREACH)
         || (errIcmpType == TM_ICMP_TYPE_SRC_QNCH)
         || (errIcmpType == TM_ICMP_TYPE_PARM_PROB)
         || (errIcmpType == TM_ICMP_TYPE_TIME_XCEED)
         || tm_16bit_one_bit_set(packetPtr->pktFlags, TM_IP_BROADCAST_FLAG |
                                                      TM_IP_MULTICAST_FLAG |
                                                      TM_LL_BROADCAST_FLAG)
         || (iphPtr->iphFlagsFragOff & TM_IP_FRAG_OFFSET)
         || tm_ip_zero(srcIpAddr)
#ifndef TM_IP_SRC_ERROR_CHECKING
/* This check also eliminates limited broadcast */
         || tm_ip_is_class_d_e(srcIpAddr)
#ifdef TM_SINGLE_INTERFACE_HOME
         || tm_ip_d_broad_match(devPtr, srcIpAddr)
#else /* !TM_SINGLE_INTERFACE_HOME */
         || (tfMhomeDBroadMatch(devPtr, srcIpAddr, &mhomeIndex) == 0)
#endif /* TM_SINGLE_INTERFACE_HOME */
#endif /* TM_IP_SRC_ERROR_CHECKING */
/* tfIpIncomingPacket() checks for loopback only on non loop back interface */
         || tm_ip_is_loop_back(srcIpAddr) )
    {
        goto icmpErrOut;
    }
#ifdef TM_SNMP_MIB
/* Number of ICMP messages which this entity attempted to send. */
    tm_context(tvIcmpData).icmpOutMsgs++;
#endif /* TM_SNMP_MIB */

/*
 * 2. To build the ICMP error message, we allocate a new ttPacket
 *    with Header size IP (20)+ LL (16) and  data size TM_ICMP_MAX_ERROR_MSG.
 */
    newPacketPtr = tfGetSharedBuffer((int)
                                 (TM_4_IP_MIN_HDR_LEN + TM_LL_MAX_HDR_LEN),
                                 (ttPktLen)TM_ICMP_MAX_ERROR_MSG, /* bytes */
                                 TM_16BIT_ZERO);
    if (newPacketPtr != TM_PACKET_NULL_PTR)
    {
/* point to ICMP header */
        newIcmphPtr = (ttIcmpHeaderPtr)newPacketPtr->pktLinkDataPtr;
/* 3. We then fill in the first 2 words of the ICMP header. First word:
 *    icmCode and icmType from the parameters. Second word: if type is
 *    ICMP redirect, we fill in the icmRouterIpAddress. For all other error
 *    messages, we zero the second word (icmUnused32), and in addition, if
 *    type is parameter problem, we fill in the icmParamProbOffset, else if
 *    type and code indicates a path MTU error message, we fill in the
 *    icmPathMtuStruct.
 */
        newIcmphPtr->icmType = icmpType;
        newIcmphPtr->icmCode = icmpCode;
        if (icmpType == TM_ICMP_TYPE_REDIRECT)
        {
#ifdef TM_SNMP_MIB
/* Number of Redirect messages sent. */
            tm_context(tvIcmpData).icmpOutRedirects++;
#endif /* TM_SNMP_MIB */
            tm_ip_copy(ipAddr, newIcmphPtr->icmSURedGwy);
        }
        else
        {
/* Covers last 3 bytes for parameter problem and first 2 bytes for path MTU */
            newIcmphPtr->icmSUUnused = TM_UL(0);
            if (icmpType == TM_ICMP_TYPE_PARM_PROB)
            {
#ifdef TM_SNMP_MIB
/* Number of ICMP Parameter Problem messages sent. */
                tm_context(tvIcmpData).icmpOutParmProbs++;
#endif /* TM_SNMP_MIB */
/* first (network order) byte of second word */
                 newIcmphPtr->icmSUParmProbl = (tt8Bit) icmpParam;
            }
            else
            {
                if ( icmpType == TM_ICMP_TYPE_UNREACH )
                {
#ifdef TM_SNMP_MIB
/* Number of ICMP Destination Unreachable messages sent. */
                    tm_context(tvIcmpData).icmpOutDestUnreachs++;
#endif /* TM_SNMP_MIB */
                    if (icmpCode == TM_ICMP_CODE_FRAG)
                    {
/* last (network order) 2 bytes of second word */
                        nextHopMtu = (tt16Bit)outDevPtr->devMtu;
#ifdef TM_USE_IPSEC
/* it is possible that when forwarding this packet, we tried to add
   headers/trailers to it (i.e. IPsec) that increased the size beyond the link
   MTU. If this is the case, then we compensate for these extra
   headers/trailers in the MTU value we send in this ICMP error message */
                        nextHopMtu = (tt16Bit)
                            (nextHopMtu - (tt16Bit) icmpParam);
#endif /* TM_USE_IPSEC */
                        tm_htons(nextHopMtu, newIcmphPtr->icmSUNxtHopMtu);
                    }
                }
#ifdef TM_SNMP_MIB
                else
                {
                    if (icmpType == TM_ICMP_TYPE_TIME_XCEED)
                    {
/* Number of ICMP Time Exceeded messages sent. */
                        tm_context(tvIcmpData).icmpOutTimeExcds++;
                    }
                    else
                    {
/* Number of ICMP Source Quench messages sent. */
                        if (icmpType ==TM_ICMP_TYPE_SRC_QNCH)
                        {
                            tm_context(tvIcmpData).icmpOutSrcQuenchs++;
                        }
                    }
                }
#endif /* TM_SNMP_MIB */
            }
        }
/* 4. We then copy the mininum of IP header + 8 bytes of data or actual
 *    IP length of the offending IP datagram in the icmDataUnion.icmIpHeader
 *    area.  (We get the IP length of the offending datagram from the
 *    old ttPacket pktLinkDataLength).
 *    We update pktLinkDataLength and pktChainDataLength of the new ttPacket
 *    with the length of the new ICMP packet. We then compute the ICMP
 *    checksum  on the ICMP packet.
 */
        if (packetPtr->pktLinkDataLength <
                (ttPktLen)(tm_byte_count(ipHdrLen) + 8))
        {

            newIcmpLength = (unsigned)packetPtr->pktLinkDataLength;
        }
        else
        {
            newIcmpLength = (unsigned)tm_byte_count(ipHdrLen) + 8;
        }
/* newIcmpLength contains the length of the copy of the offending datagram */
        tm_bcopy((ttConstVoidPtr)iphPtr,
                  (ttVoidPtr)&(newIcmphPtr->icmDUIpHeader),
                  tm_packed_byte_count(newIcmpLength));

/* Add first two words of ICMP header */
        newIcmpLength += tm_byte_count(TM_4PAK_ICMP_MIN_LEN);
        newPacketPtr->pktLinkDataLength = newIcmpLength;
        newPacketPtr->pktChainDataLength = newIcmpLength;
/* Compute ICMP checksum */
        newIcmphPtr->icmChecksum = 0U;
        newIcmphPtr->icmChecksum = tfPacketChecksum(
            newPacketPtr,
            (ttPktLen)newIcmpLength,
            TM_16BIT_NULL_PTR,
            TM_16BIT_ZERO);
/*
 * 5. We initialize pktIpHdrLen with minimum IP header, copy
 *    pktDeviceEntryPtr from the old packet.
 *    We point to IP header, use source IP address of old IP header as
 *    our destination address, and use incoming device IP address as our
 *    source address, initialize iphTtl with TM_IP_MAX_TTL,
 *    iphTos with TM_IP_DEF_TOS, iphUlp with TM_IP_ICMP. We initialize
 *    pktEthernetAddress (if LAN) from the old packet, initialize pktFlags
 *    with TM_OUTPUT_DEVICE or TM_OUTPUT_DEVICE| TM_ETHERNET_ADDRESS_FLAG (if
 *    LAN), add to pktFlags TM_IP_DEST_ROUTE_SET to bypass IP route
 *    cache, and call tfIpSendPacket(newPacketPtr, (ttLockEntryPtr)0)
 *    and free the old packet
 */
        newPacketPtr->pktIpHdrLen = TM_4PAK_IP_MIN_HDR_LEN;
        devPtr = packetPtr->pktDeviceEntryPtr;
        newPacketPtr->pktDeviceEntryPtr = devPtr;
#ifndef TM_SINGLE_INTERFACE_HOME
/* Save dest IP addr from original IP header */
        tm_ip_copy(iphPtr->iphDestAddr, dstIpAddr);
#endif /* TM_SINGLE_INTERFACE_HOME */
/* Point to new IP header */
        tm_pkt_llp_hdr(newPacketPtr, TM_4PAK_IP_MIN_HDR_LEN);
        iphPtr = (ttIpHeaderPtr)(newPacketPtr->pktLinkDataPtr);
/* Source Ip address of old IP header is our destination */
        tm_ip_copy(srcIpAddr, iphPtr->iphDestAddr);

#ifndef TM_SINGLE_INTERFACE_HOME
/* If the option is set, use the incoming packet's dest IP addr as our src */
        tmpDevPtr = tfIfaceMatch(dstIpAddr, tfMhomeAddrMatch, &junk);
        if (   (tm_context(tvIcmpErrDstAsSrc) == TM_8BIT_YES)
            && (tmpDevPtr != TM_DEV_NULL_PTR) )
        {
            tm_ip_copy(dstIpAddr, iphPtr->iphSrcAddr);
        }
        else
#endif /* TM_SINGLE_INTERFACE_HOME */
        {
/*
 * Compute our source address, and insert into IP header. Our source address
 * is the IP address of the incoming interface.
 */
#ifdef TM_SINGLE_INTERFACE_HOME
            tm_ip_copy(devPtr->devIpAddr, iphPtr->iphSrcAddr);
#else /* !TM_SINGLE_INTERFACE_HOME */
/*
 * NOTE TM_DEV_IP_CONFIG is INPUT parameter for tfMhomeAnyConf(). mhomeIndex
 * will be set by tfMhomeAnyConf or tfMhomeNetMatch if successful.
 */
            mhomeIndex = TM_DEV_IP_CONFIG;
            errCode = tfMhomeNetMatch( devPtr,srcIpAddr, &mhomeIndex);
            if ( errCode == TM_ENOERROR )
            {
                newPacketPtr->pktMhomeIndex = mhomeIndex;
            }
            else
            {
/*
 * If no network match on source address, use first configured address
 */
                errCode = tfMhomeAnyConf( devPtr, srcIpAddr, &mhomeIndex);
                if ( errCode == TM_ENOERROR )
                {
                    newPacketPtr->pktMhomeIndex = mhomeIndex;
                }
                else
                {
/* newPacketPtr->pktMhomeIndex is already 0 */
                    mhomeIndex = TM_16BIT_ZERO;
                }
            }
            tm_ip_copy(tm_ip_dev_addr(devPtr, mhomeIndex),
                       iphPtr->iphSrcAddr);
#endif /* TM_SINGLE_INTERFACE_HOME */
        }
/* TOS TTL */
        iphPtr->iphTos = TM_IP_DEF_TOS;
        iphPtr->iphTtl = TM_IP_MAX_TTL;
        iphPtr->iphFlagsFragOff = TM_16BIT_ZERO;
        iphPtr->iphUlp = TM_IP_ICMP;
        if (tm_4_ll_is_lan(packetPtr->pktDeviceEntryPtr))
        {
            newPacketPtr->pktFlags = TM_IP_DEST_ROUTE_SET |
                                      TM_OUTPUT_DEVICE_FLAG |
                                      TM_ETHERNET_ADDRESS_FLAG;
            tm_bcopy((ttConstVoidPtr)(pktShrDataPtr->dataEthernetAddress),
                      (ttVoidPtr)(newPacketPtr->pktEthernetAddress),
                      sizeof(ttEnetAddress));
        }
        else
        {
            newPacketPtr->pktFlags = TM_IP_DEST_ROUTE_SET |
                                      TM_OUTPUT_DEVICE_FLAG;
        }
        (void)tfIpSendPacket(
            newPacketPtr,
            (ttLockEntryPtr)0
#ifdef TM_USE_IPSEC
            , (ttPktLenPtr)0
#ifdef TM_USE_IPSEC_TASK
            , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
#endif /* TM_USE_IPSEC */
            );
    }
#ifdef TM_SNMP_MIB
    else
    {
/*
 * Number of ICMP messages which this entity did not send due to problems
 * discovered within ICMP such as a lack of buffers.
 */
        tm_context(tvIcmpData).icmpOutErrors++;
    }
#endif /* TM_SNMP_MIB */

icmpErrOut:
    tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    return;
}
LINT_UNACCESS_SYM_END(nextHopMtu)

#else /* ! TM_USE_IPV4 */

LINT_UNUSED_HEADER

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4IcmpDummy = 0;
#endif /* ! TM_USE_IPV4 */
