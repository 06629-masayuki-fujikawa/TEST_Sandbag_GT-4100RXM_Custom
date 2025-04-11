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
 * Description: UDP functions
 * Filename: trudp.c
 * Author: Odile
 * Date Created: 01/07/98
 * $Source: source/trudp.c $
 *
 * Modification History
 * $Revision: 6.0.2.7 $
 * $Date: 2010/09/17 02:03:13JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


#include <trsocket.h> 
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/*
 * Local macros
 */
#ifdef TM_DEV_SEND_OFFLOAD
#define  tm_udp_send_xsum_offload(offloadFlags)                        \
    tm_16bit_one_bit_set( offloadFlags,                                \
                          TM_DEVO_UDP_CHKSUM_OFFLOAD )
#endif /* TM_DEV_SEND_OFFLOAD */

/*
 * Local types
 */

/*
 * tfUdpSendPacket Function Description
 * This function is called by the socket layer when the user issues any
 * send function (such as send(), sendto(), etc.). We expect that there is
 * enough space for us to add the UDP header before the user data. We fill
 * in the UDP header and the IP header source and destination addresses.
 * For UDP checksum computation we build the pseudo header fields. We fill
 * in the other IP header fields that need to be set from the socket entry
 * structure.
 * 1. Fill in UDP header: Make the packet point to the UDP header
 *    and copy socPeerLayer4Port, and socOurLayer4Port in the UDP
 *    header. We copy the UDP datagram length (pktChainDataLength)
 *    in network byte order, and zero the checksum field.
 * 2. Fill in some of the IP header fields: save a pointer to the
 *    IP header and call tfIpDestToPacket(). Fill in the
 *    IP header fragment field and ULP protocol field. Save the socket
 *    entry TTL and unlock the socket entry.
 * 3. Checksum computation: If UDP checksum is enabled (tvUdpChecksumOn
 *    is 1), we make the packet point to the pseudo header. Note that
 *    since we do not support IP header options, the pseudo header
 *    (with some fields re-arranged, see trtype.h for details) overlays
 *    the bottom of the IP header. We initialize the UDP length in the
 *    pseudo header (overlays IP header checksum) with the value stored in
 *    the UDP header, zero the zero field in the pseudo header
 *    (overlays the IP header ttl). The other pseudo-header fields overlay
 *    the IP header fields initialized in tfIpDestToPacket().
 *    We compute the UDP checksum.
 *    If the result is 0, we need to change it to 0xFFFF (since a UDP
 *    checksum of 0 would indicate that no checksum has been computed,
 *    and would make the other side not verify the checksum).
 * 4. We fill in the saved IP header TTL field from the socket entry,
 *    the IP header ULP protocol, and the flagsFragOff field.
 * 5. Fill in pktIpHderLen, Point to the IP header and call the IP layer: 
 *    tfIpSendPacket(packetPtr, &socketPtr->socLockEntry)
 *
 * Parameters
 * Parameter  Description
 * socketPtr  Socket entry pointer of the sending socket. We expect
 *            socPeerIpAddress and socPeerLayer4Port to contain the
 *            destination IP address and destination UDP port in
 *            network byte order. socOurLayer4Port should contain the
 *            source UDP port in network byte order. For multihoming, the
 *            user could also have set the socOurIfIpAddress. socIpTtl
 *            and socTos should contain the IP TTL , and TOS respectively
 *            (either set by the user or default values when socket opens.)
 * packetPtr  pointer to message to be sent. We expect pktChainDataLength to
 *            contain the total length of the user data, pktLinkDataPtr
 *            to point to the data to be sent, pktLinkDataLength to contain
 *            the size of the first shared user data area. There should be
 *            enough space ahead of pktLinkDataPtr to add UDP/IP/LL headers.
 *
 * Returns
 * Value   Meaning
 * 0       no error
 * TM_EHOSTUNREACH could not send out
 */
int tfUdpSendPacket (ttSocketEntryPtr socketPtr,
                     ttPacketPtr      packetPtr)
{
    ttIpAddressPtr      ipSrcAddrPtr;
#ifdef TM_USE_IPV4
    ttIpHeaderPtr       iphPtr;
    ttPseudoHeaderPtr   pshPtr;
    tt4IpAddress        ip4SrcAddr;
#endif /* TM_USE_IPV4 */
    ttUdpHeaderPtr      udphPtr;
    int                 errorCode;
#ifdef TM_USE_IPV4
    ttPktLen            ipv4PakHdrLength;
    ttPktLen            ipv4HdrLength;
#endif /* TM_USE_IPV4 */
    tt16Bit             udpLength;
#ifdef TM_USE_IPV6
    tt6IpHeaderPtr      iph6Ptr;
    tt6PseudoHeaderPtr  psh6Ptr;
    int                 hopLimit;
    tt8Bit              saved6IpHops;
#endif /* TM_USE_IPV6 */
#ifdef TM_DEV_SEND_OFFLOAD
    ttDevSendOffloadPtr pktSendOffloadPtr;
#ifdef TM_USE_IPV6
    int                 pmtu;
#endif /* TM_USE_IPV6 */
#endif /* TM_DEV_SEND_OFFLOAD */

/* point to UDP header */
    tm_pkt_llp_hdr(packetPtr, TM_PAK_UDP_HDR_LEN);
    udphPtr = (ttUdpHeaderPtr)packetPtr->pktLinkDataPtr;
/* copy the UDP datagram length (pktChainDataLength) in network byte order */
    udpLength = (tt16Bit)packetPtr->pktChainDataLength;
    tm_htons(udpLength, udphPtr->udpLength);
/* and zero the checksum field. */
    udphPtr->udpChecksum = TM_16BIT_ZERO;
/* User call: socketPtr != TM_SOCKET_ENTRY_NULL_PTR. */
/* copy socPeerLayer4Port, and socOurLayer4Port in the UDP header */
    if (    (packetPtr->pktSharedDataPtr->dataLocalPortNumber != TM_16BIT_ZERO)
         && (    ((unsigned short) (socketPtr->socOurLayer4Port) == TM_WILD_PORT)
#ifdef TM_USE_IKE
#ifdef TM_USE_NATT
              || (packetPtr->pktSharedDataPtr->dataLocalPortNumber
                                         == tm_const_htons(TM_IKE_NAT_PORT))
#endif /* TM_USE_NATT */
#endif /* TM_USE_IKE */
            )
       )
    {
/* assume the user has specified a specific source port# to use by calling
   tfSendToFrom */
        udphPtr->udpSrcPort = packetPtr->pktSharedDataPtr->dataLocalPortNumber;
    }
    else
    {
        udphPtr->udpSrcPort = socketPtr->socOurLayer4Port;
    }
    udphPtr->udpDstPort = socketPtr->socPeerLayer4Port;
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
    
/*
 * IPv4 tfIpDestToPacket()
 * Fill in IP header source and destination addresses and tos
 * We need to wait after the checksum to fill in the ttl (since the IP header
 * TTL field is zeroed for UDP checksum computation).
 * IPv6 tf6IpDestToPacket()
 * Fill in IP header source and destination addresses and discovered/default
 * Hops Limit. We will fill-in the other fields after checksum computation.
 */
#ifdef TM_USE_IPV4
    ipv4PakHdrLength = TM_4PAK_IP_MIN_HDR_LEN;
/* Byte count of IP header length */
    ipv4HdrLength = tm_byte_count(ipv4PakHdrLength);
#ifdef TM_USE_IPV6
    if (    (socketPtr->socProtocolFamily == PF_INET)
         || (tm_6_addr_is_ipv4_mapped(&socketPtr->socPeerIpAddress)) )
#endif /* dual */
    {
/* IPv4 protocol family, or IPV4 mapped IPV6 address, use IPv4 header */
        iphPtr = (ttIpHeaderPtr)(packetPtr->pktLinkDataPtr
                                 - TM_4PAK_IP_MIN_HDR_LEN);
#ifdef TM_USE_IPV6
        iph6Ptr = (tt6IpHeaderPtr)0;
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
        tm_ip_copy(*ipSrcAddrPtr, ip4SrcAddr);
#endif /* TM_USE_IPV6 */
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
            iphPtr );
    }
#ifdef TM_USE_IPV6
    else
#endif /* dual layer */
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    {
/* IPv6 protocol family */
/* Point to IP header */
        iph6Ptr = (tt6IpHeaderPtr) 
                        (packetPtr->pktLinkDataPtr - TM_6PAK_IP_MIN_HDR_LEN);
        errorCode = tf6IpDestToPacket(
                                    packetPtr,
                                    ipSrcAddrPtr,
                                    &socketPtr->socPeerIpAddress,
                                    &socketPtr->socRteCacheStruct,
                                    iph6Ptr);
/* Dual IP layer. Initialize the iphPtr to zero. */
#ifdef TM_USE_IPV4
        iphPtr = (ttIpHeaderPtr)0;
#endif /* dual */
    }
#endif /* TM_USE_IPV6 */
    if (errorCode == TM_ENOERROR)
    {
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
        if (iphPtr != (ttIpHeaderPtr)0)
#endif /* dual */
        {
            iphPtr->iphUlp = TM_IP_UDP;
            iphPtr->iphFlagsFragOff = TM_16BIT_ZERO;
            if (tm_context(tvUdpChecksumOn))
            {
/*
 * Pseudo header pointer:
 * Note that since we do not support IP header options, the pseudo-header
 * overlays the IP header.  
 */
                pshPtr = (ttPseudoHeaderPtr)( ((tt8BitPtr)iphPtr) + 
                        (unsigned)(TM_4PAK_IP_MIN_HDR_LEN
                                   - TM_4PAK_PSEUDO_HDR_LEN) );
/* Initialize pseudo header length. Overlays IP header checksum */
                pshPtr->pshLayer4Len = udphPtr->udpLength; /* network order */
/* Initialize pseudo header zero field. Overlays TTL */
                pshPtr->pshZero = TM_8BIT_ZERO;
#ifdef TM_DEV_SEND_OFFLOAD
                if (   tm_udp_send_xsum_offload(
                            packetPtr->pktDeviceEntryPtr->devOffloadFlags)
                     && ( (ipv4HdrLength + udpLength) <=
                            packetPtr->pktDeviceEntryPtr->devMtu ) )
/* Only offload the UDP checksum if we do not fragment */
                {
                    pktSendOffloadPtr = (ttDevSendOffloadPtr)
                            &(packetPtr->pktSharedDataPtr->dataDevSendOffload);
                    pktSendOffloadPtr->devoFlags = (tt16Bit)
                             (    packetPtr->pktDeviceEntryPtr->devOffloadFlags
                               & (   TM_DEVO_UDP_CHKSUM_OFFLOAD
                                   | TM_DEVO_IP_CHKSUM_OFFLOAD ) );
                    udphPtr->udpChecksum = tfPacketChecksum(
                        TM_PACKET_NULL_PTR,
                        (ttPktLen)0,
                        (tt16BitPtr)(ttVoidPtr)pshPtr,
                        (tt16Bit)tm_byte_count(TM_4PAK_PSEUDO_HDR_LEN));
                    pktSendOffloadPtr->devoTucse = 0;
                    pktSendOffloadPtr->devoTucss = (tt16Bit)
                         (   packetPtr->pktDeviceEntryPtr->devLinkHeaderLength
                           + ipv4HdrLength );
                    pktSendOffloadPtr->devoTucso = (tt16Bit)
                                           (   pktSendOffloadPtr->devoTucss
                                             + TM_UDP_BYTE_XSUM_OFFSET );
                    if (   pktSendOffloadPtr->devoFlags 
                         & TM_DEVO_IP_CHKSUM_OFFLOAD )
                    {
/* IP header checksum */
/* offset to first byte to be included in Xsum */
                       pktSendOffloadPtr->devoIpcss = 
                           packetPtr->pktDeviceEntryPtr->devLinkHeaderLength;
                       pktSendOffloadPtr->devoIpcso = (tt16Bit)
                                         (  pktSendOffloadPtr->devoIpcss
                                          + TM_IP_BYTE_XSUM_OFFSET );
/* Offset to last byte to be in included in Xsum */
                       pktSendOffloadPtr->devoIpcse = (tt16Bit)  
                                              (   pktSendOffloadPtr->devoIpcss
                                                + (ipv4HdrLength - 1) );
                    }
                    packetPtr->pktDevOffloadPtr = (ttVoidPtr)pktSendOffloadPtr;
                }
                else
#endif /* TM_DEV_SEND_OFFLOAD */
                {
/* Compute the checksum */
                    udphPtr->udpChecksum = tfPacketChecksum(
                        packetPtr,
                        (ttPktLen)udpLength,
                        (tt16BitPtr)(ttVoidPtr)pshPtr,
                        (tt16Bit)tm_byte_count(TM_4PAK_PSEUDO_HDR_LEN));
/*
 * If the result is 0, we need to change it to 0xFFFF (since a UDP
 * checksum of 0 would indicate that no checksum has been computed,
 * and would make the peer not verify the checksum).
 */
                    if (udphPtr->udpChecksum == TM_16BIT_ZERO)
                    {
                        udphPtr->udpChecksum = 0xFFFFU;
                    }
                }
            }
/* Initialize packet IP header length */
            packetPtr->pktIpHdrLen = (tt8Bit)ipv4PakHdrLength;
/* Point to IP header */
            packetPtr->pktLinkDataPtr = (tt8BitPtr)iphPtr;
/* LinkDataLength including the IP header */
            packetPtr->pktLinkDataLength += ipv4HdrLength;
            packetPtr->pktChainDataLength += ipv4HdrLength;
/* Initialize IP TTL from value saved earlier */
            if ( tm_16bit_one_bit_set( packetPtr->pktFlags, 
                                       TM_IP_MULTICAST_FLAG ) )
            {
                iphPtr->iphTtl = socketPtr->socIpMcastTtl;
            }
            else
            {
                iphPtr->iphTtl = socketPtr->socIpTtl;
            }
        }
#ifdef TM_USE_IPV6
        else
#endif /* dual layer */
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
        {
/*
 * Pseudo header pointer:
 * Note that since we do not support IP header options, the pseudo-header
 * overlays the IP header.  
 */
            psh6Ptr = (tt6PseudoHeaderPtr)((tt8BitPtr)iph6Ptr);
/* Save IP Hops limit as set by tf6IpDestToPacket */
            saved6IpHops = iph6Ptr->iph6Hops;
/* Initialize pseudo header length. Overlays flow label */
            psh6Ptr->psh616BitLayer4Len = udphPtr->udpLength; /* network order */
            psh6Ptr->psh616Bit0Layer4Len = TM_16BIT_ZERO; /* network order */
/* Initialize pseudo header zero field. Overlays Payload, hops */
            tm_6_zero_arr(psh6Ptr->psh6ZeroArr);
/* ULP field */
            psh6Ptr->psh6Protocol = IPPROTO_UDP; 
#ifdef TM_DEV_SEND_OFFLOAD
            if (tm_udp_send_xsum_offload(
                            packetPtr->pktDeviceEntryPtr->devOffloadFlags))
            {
/* Only offload the UDP checksum if we do not fragment */
/* Need to protect the IPv6 & route MTU, since they could be changed dynamically
   due to a router advertisement. */
                tm_kernel_set_critical;
                pmtu = packetPtr->pktDeviceEntryPtr->dev6Mtu;
                tm_kernel_release_critical;
#ifdef TM_6_PMTU_DISC
                if (packetPtr->pktFlags & TM_IP_RTE_FLAG)
                {
                    tm_kernel_set_critical;                    
                    pmtu = packetPtr->pktRtePtr->rteMtu;
                    tm_kernel_release_critical;                    
                }
#endif /* TM_6_PMTU_DISC */
                if ( ((TM_6_IP_MIN_HDR_LEN + udpLength) > pmtu) )
                {
/* Fragmentation would occur. Do not offload. */
                    pmtu = 0;
                }
            }
            else
            {
                pmtu = 0;
            }
            if (pmtu != 0)
            {
                pktSendOffloadPtr = 
                            &(packetPtr->pktSharedDataPtr->dataDevSendOffload);
                pktSendOffloadPtr->devoFlags = (tt16Bit)
                                               TM_DEVO_UDP_CHKSUM_OFFLOAD;
                udphPtr->udpChecksum = tfPacketChecksum(
                                TM_PACKET_NULL_PTR,
                                (ttPktLen)0,
                                (tt16BitPtr)(ttVoidPtr)psh6Ptr,
                                (tt16Bit)TM_6_IP_MIN_HDR_LEN);
                pktSendOffloadPtr->devoTucse = 0;
                pktSendOffloadPtr->devoTucss = (tt16Bit)
                         (   packetPtr->pktDeviceEntryPtr->devLinkHeaderLength
                           + TM_6_IP_MIN_HDR_LEN );
                pktSendOffloadPtr->devoTucso = (tt16Bit)
                                           (   pktSendOffloadPtr->devoTucss
                                             + TM_UDP_BYTE_XSUM_OFFSET );
                packetPtr->pktDevOffloadPtr = (ttVoidPtr)pktSendOffloadPtr; 
            }
            else
#endif /* TM_DEV_SEND_OFFLOAD */
            {
/* Compute the checksum */
                udphPtr->udpChecksum = tfPacketChecksum(
                                packetPtr,
                                (ttPktLen)udpLength,
                                (tt16BitPtr)(ttVoidPtr)psh6Ptr,
                                (tt16Bit)TM_6_IP_MIN_HDR_LEN);
/*
 * If the result is 0, we need to change it to 0xFFFF (since a UDP
 * checksum of 0 would indicate that no checksum has been computed,
 * and would make the peer not verify the checksum).
 */
                if (udphPtr->udpChecksum == TM_16BIT_ZERO)
                {
                    udphPtr->udpChecksum = 0xFFFFU;
                }
            }
/* Initialize IP hops. Try user set value first */
            if ( tm_16bit_one_bit_set(
                     packetPtr->pktFlags, TM_IP_MULTICAST_FLAG) )
            {
/* Socket-level IPV6_MULTICAST_HOPS.([IPV6REQ].R2.18:10) */
                hopLimit = socketPtr->soc6McastHopLimit;
            }
            else
            {
/* Socket-level IPV6_UNICAST_HOPS.([IPV6REQ].R2.18:10) */
               hopLimit = socketPtr->soc6HopLimit; 
            }
            if (hopLimit != -1)
            {
/* -1 is a special value that indicates that soc6{Mcast}HopLimit hasn't been
   initialized. */
/* User did set hops limit. Use the user limit, instead of the discovered one */
                saved6IpHops = (tt8Bit)hopLimit;
            }
            iph6Ptr->iph6Hops = saved6IpHops;
            iph6Ptr->iph6Nxt = TM_IP_UDP;
/* Initialize IP header traffic class && flow label */
            iph6Ptr->iph6Flow = socketPtr->soc6FlowInfo;
        }
#endif /* TM_USE_IPV6 */
#ifdef TM_SNMP_MIB
/* Total Number of UDP datagrams sent from this entity. */
#ifdef TM_USE_NG_MIB2
        tm_64Bit_incr(tm_context(tvUdpData).udpHCOutDatagrams);
#else /* TM_USE_NG_MIB2 */
        tm_context(tvUdpData).udpOutDatagrams++;
#endif /* !TM_USE_NG_MIB2 */
/* Number of UDP datagrams sent from this UDP
 * endpoint (socket)
 */
        tm_64Bit_incr(socketPtr->socHCOutDatagrams);
        tm_64Bit_augm(socketPtr->socHCOutOctets, udpLength);

#endif /* TM_SNMP_MIB */
/* Send the packet */
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
        if (iphPtr != (ttIpHeaderPtr)0)
#endif /* dual */
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
#ifdef TM_USE_IPV6
        else
#endif /* dual */
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
        {
            errorCode = tf6IpSendPacket(packetPtr,
                                        &socketPtr->socLockEntry
                                        
#ifdef TM_USE_IPSEC_TASK
                                        , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
                                        );
        }
#endif /* TM_USE_IPV6 */
    }
    else
    {
/* free the packet if we could not send it */
        tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
    }
    return errorCode;
}


/*
 * tfUdpIncomingPacket Function Description
 *  The IP layer calls this function when there is an incoming UDP datagram.
 *  RFC768 and RFC1122 makes no mention of double checking UDP length
 *  so we will let the checksum take care of it. We will however check
 *  it inside an ifdef TM_ERROR_CHECKING.
 * 1. Save pointer to IP header and UDP headers. Compute UDP length
 *    from packet IP length.
 * 2. Inside #ifdef TM_ERROR_CHECKING, check whether the computed UDP
 *    length from the IP packet matches the UDP length from the UDP header.
 *    If smaller, free the packet and return. If bigger, tail trim the packet.
 * 3. If checksum field is non zero, check the UDP checksum:
 *    Point to Pseudo header. Save the TTL field and IP checksum fields
 *    since they will be modified during the UDP checksum computation.
 *    If there are IP options, save the content of the other fields
 *    that will be modified in the pseudoheader, and copy the IP fields
 *    that we need for the checksum. We then zero the pseudo header
 *    zero field and store the UDP length in the pseudo-header length field.
 *    Check the UDP checksum on pseudo-header + UDP datagram.
 * 4. Check first on local UDP cache for a match
 *    on destination port. If no match call tfSocketLookup(TM_IP_UDP,
 *    destIpAddres, destPort, 0, 0, TM_16BIT_ZERO).
 *    If no socket entry is found send an ICMP unreachable for port error
 *    message back to the sender, after making the packet point back to the
 *    IP header if we had computed the checksum, and after having restored
 *    the IP header fields modified during the UDP checksum computation
 *    and return.
 * 5. If socket is connected and packet source address/port does not match
 *    socket peer address/port, free the packet.
 * 6. Store peer's data port in shared data, make packet point to user data,
 *    and call  tfSocketIncomingPacket(socketEntryPtr, packetPtr)
 *
 * Parameters
 * Parameter    Description
 * packetPtr    Pointer to ttPacket containing the UDP incoming data.
 *              packetPtr->pktFlags gives the link layer and IP layer
 *               broadcast multicast status.
 *              packetPtr->pktLinkDataPtr points to the IP header for IPv4,
 *              packetPtr->pktLinkDataPtr points to the UDP header for IPv6,
 *              packetPtr->pktDeviceEntryPtr points to the device the packet
 *               came in.
 *              packetPtr->pktChainDataLength is the length of the IP packet.
 *              packetPtr->pktSpecDestIpAddr contains the specific IP
 *               destination address,
 *              packetPtr->pktDestIpAddr contains the IP destination address
 *               from the IP header.
 *              packetPtr->pktSrcIpAddr contains the source address of the
 *               UDP datagram.
 *              packetPtr->pktFlags indicates whether the destination
 *               IP address is broadcast, unicast or multicast
 *              IPv4 only: packetPtr->pktIpHdrLen IP header length in bytes
 *              (so that we know whether there are options)
 *              IPv6 only: packetPtr->pkt6RxIphPtr points to the IPv6 header
                Dual mode: packetPtr->pktNetworkLayer  is set
                 to TM_NETWORK_IPV4_LAYER if the packet is an IPv4 packet,
                 or to TM_NETWORK_IPV6_LAYER if the packet is an IPv6 packet.
 */
void tfUdpIncomingPacket(ttPacketPtr packetPtr)
{
#ifdef TM_USE_IPV6
    tt32Bit             saved6PshLayer4Len;
#ifdef TM_DSP
#ifdef TM_16BIT_DSP
    unsigned int        saved6PshZero[2];
#else /* !TM_16BIT_DSP */
    unsigned int        saved6PshZero[1];
#endif /* !TM_16BIT_DSP */
#else /* !TM_DSP */
    tt8Bit              saved6PshZero[3];
#endif /* !TM_DSP */
    tt6IpHeaderPtr      iph6Ptr;
    tt6PseudoHeaderPtr  psh6Ptr;
    tt6IpAddressPtr     ipv6LocalAddrPtr;
    tt6IpAddressPtr     ipv6PeerAddrPtr;
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
    ttIpHeaderPtr       iphPtr;
    ttPseudoHeaderPtr   pshPtr;
    tt16Bit             savedPshLayer4Len;
    tt8Bit              savedPshZero;
#endif /* TM_USE_IPV4 */
    ttUdpHeaderPtr      udphPtr;
    ttSocketEntryPtr    socketPtr;
#ifndef TM_DISABLE_UDP_CACHE
    ttSocketEntryPtr    purgeCacheSocketPtr;
#endif /* !TM_DISABLE_UDP_CACHE */
#ifdef TM_DEV_RECV_OFFLOAD
    ttDevRecvOffloadPtr pktDevRecvOffloadPtr;
#endif /* TM_DEV_RECV_OFFLOAD */
#ifdef TM_USE_REUSEADDR_LIST
    ttSocketEntryPtr    nextSocketPtr;
    ttPacketPtr         newPacketPtr;
#endif /* TM_USE_REUSEADDR_LIST */
#if (defined(TM_USE_NATT) && defined(TM_USE_IPSEC))
     ttSadbIntRecordPtr       sadbIntPtr;
     ttPseudoHeader           prvPsh;
#endif /* TM_USE_NATT && TM_USE_IPSEC */
    ttTupleDev          tupleDev;
    int                 errorCode;
#if (defined(TM_USE_DRV_SCAT_RECV) && defined(TM_USE_IPV4))
    int                 contByteLength; 
#endif /* TM_USE_DRV_SCAT_RECV and TM_USE_IPV4 */
    tt16Bit             udpLength;
    tt16Bit             udpLengthFromIp;
    tt16Bit             ipHdrLen;
    tt16Bit             checksum;
    tt16Bit             flag;
#ifdef TM_USE_REUSEADDR_LIST
    tt8Bit              firstMatch;
#endif /* TM_USE_REUSEADDR_LIST */

#ifndef TM_DISABLE_UDP_CACHE
    purgeCacheSocketPtr = (ttSocketEntryPtr)0;
#endif /* !TM_DISABLE_UDP_CACHE */
#ifdef TM_USE_IPV4
    pshPtr = (ttPseudoHeaderPtr)0;
/* avoid compiler warnings */
    savedPshZero = TM_8BIT_ZERO;
    savedPshLayer4Len = TM_16BIT_ZERO;
#if (defined(TM_USE_NATT) && defined(TM_USE_IPSEC))
    sadbIntPtr = (ttSadbIntRecordPtr)0;
    tm_bzero(&prvPsh,sizeof(ttPseudoHeader));
#endif /* TM_USE_NATT && TM_USE_IPSEC */
#endif /* TM_USE_IPV4 */
    socketPtr = (ttSocketEntryPtr)0; /* to avoid compiler warning */
    tm_assert(tfUdpIncomingPacket, 
              (packetPtr->pktNetworkLayer == TM_NETWORK_IPV6_LAYER)
              || (packetPtr->pktNetworkLayer == TM_NETWORK_IPV4_LAYER));
    if (tm_16bit_one_bit_set(packetPtr->pktFlags, TM_IP_MULTICAST_FLAG))
    {
        flag = TM_SOC_TREE_MCAST_MATCH;
    }
    else if (tm_16bit_one_bit_set(packetPtr->pktFlags, TM_IP_BROADCAST_FLAG))
    {
        flag = TM_SOC_TREE_BCAST_MATCH;
    }
    else
    {
        flag = TM_16BIT_ZERO;
    }
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
    if (packetPtr->pktNetworkLayer == TM_NETWORK_IPV4_LAYER)
#endif /* dual */
    {
        ipHdrLen = packetPtr->pktIpHdrLen;
#ifdef TM_USE_DRV_SCAT_RECV
        contByteLength = tm_byte_count(ipHdrLen) + TM_MAX_UDP_HDR_LEN ;
/* If IP header + TCP header is not contiguous, make it contiguous */
        if (tm_pkt_hdr_is_not_cont(packetPtr, contByteLength))
        {
            packetPtr = tfContiguousSharedBuffer( packetPtr, 
                                                  contByteLength );
            if (packetPtr == (ttPacketPtr)0)
            {
                goto udpIncomingExit;
            }
        }
#endif /* TM_USE_DRV_SCAT_RECV */
/* Save pointer to IP header */
        iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
/* Point to UDP header */
        tm_pkt_ulp_hdr(packetPtr, ipHdrLen);
#ifdef TM_USE_IPV6
        iph6Ptr = (tt6IpHeaderPtr)0;
        psh6Ptr = (tt6PseudoHeaderPtr)0;
#endif /* dual */
    }
#ifdef TM_USE_IPV6
    else
#endif /* dual layer */
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    {
/* For IPv6, packet already points to the UDP header */
#ifdef TM_USE_DRV_SCAT_RECV
        if (tm_pkt_hdr_is_not_cont(packetPtr,  TM_MAX_UDP_HDR_LEN))
        {
            packetPtr = tfContiguousSharedBuffer( packetPtr, 
                                                  TM_MAX_UDP_HDR_LEN );
            if (packetPtr == (ttPacketPtr)0)
            {
                goto udpIncomingExit;
            }
        }
#endif /* TM_USE_DRV_SCAT_RECV */
        iph6Ptr = packetPtr->pkt6RxIphPtr;
        psh6Ptr = packetPtr->pkt6RxPseudoHdrPtr;
/* Dual IP layer. Initialize the iphPtr to zero. */
#ifdef TM_USE_IPV4
        iphPtr = (ttIpHeaderPtr)0;
        ipHdrLen = (tt16Bit)0; /* not needed yet */
#endif /* dual */
    }
#endif /* TM_USE_IPV6 */
/* UDP length from packet/IP information */
    udpLengthFromIp = (tt16Bit)packetPtr->pktChainDataLength;
    udphPtr = (ttUdpHeaderPtr)(packetPtr->pktLinkDataPtr);
/* Check that data length covers at least the size of the UDP header */
/* UDP length from UDP header */
    tm_ntohs(udphPtr->udpLength, udpLength);
#ifdef TM_LINT
LINT_UNINIT_SYM_END(udpLength)
#endif /* TM_LINT */
/* Check UDP length from UDP header */
    if (    (udpLength < TM_MAX_UDP_HDR_LEN)
         || (udpLengthFromIp < udpLength) )
/* 
 * If UDP length from UDP header is not big enough to cover the UDP header, 
 * or if it is bigger than size given by IP, drop the packet.
 */
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfUdpIncoming", "Bad UDP header length");
#endif /* TM_ERROR_CHECKING */
        goto outUdpIncoming; /* return */
    }
/* 
 * If UDP length from UDP header is smaller than size given by IP, reduce 
 * the length of packet accordingly 
 */
    if (udpLengthFromIp > udpLength)
/* UDP header length and IP header length do not agree */
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfUdpIncoming", "Trimmed incoming UDP packet");
#endif /* TM_ERROR_CHECKING */
/* If bigger, tail trim the packet. */
        tfPacketTailTrim(packetPtr,
                         (ttPktLen)(udpLength),
                         TM_SOCKET_UNLOCKED);
    }
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
    if (iphPtr != (ttIpHeaderPtr)0)
#endif /* dual */
    {
#ifdef TM_USE_IPV6
        tm_6_zero_arr(saved6PshZero);
        saved6PshLayer4Len = TM_32BIT_ZERO;
#endif /* TM_USE_IPV6 */
/* If udp header checksum field is non zero, need to compute checksum */
        if (    (udphPtr->udpChecksum != TM_16BIT_ZERO)
             && (tm_context(tvUdpIncomingChecksumOn)))
        {
#ifdef TM_DEV_RECV_OFFLOAD
            pktDevRecvOffloadPtr = (ttDevRecvOffloadPtr)
                                            packetPtr->pktDevOffloadPtr;
            if (    (pktDevRecvOffloadPtr != (ttDevRecvOffloadPtr)0)
                 && (   pktDevRecvOffloadPtr->devoRFlags 
                       & (   TM_DEVOR_TCP_RECV_CHKSUM_VALIDATED ) ) )
            {
/* No need to validate the checksum */
                checksum = TM_16BIT_ZERO;
            }
            else
#endif /* TM_DEV_RECV_OFFLOAD */
            {
/* Point to the Pseudo header for packet checksum */
                pshPtr = (ttPseudoHeaderPtr)( ((tt8BitPtr)iphPtr) + 
                      (unsigned)(TM_4PAK_IP_MIN_HDR_LEN
                                 - TM_4PAK_PSEUDO_HDR_LEN) );
#if (defined(TM_USE_NATT) && defined(TM_USE_IPSEC))
/* Because a NAT change is done, dst of TCP pseudo header is a
 * local IP address. It is changed to a global IP address to use therefore.
 */
                sadbIntPtr = (ttSadbIntRecordPtr)packetPtr->pktSadbRecordPtr;
/* SA doesn't exist in the case of BYPASS and so on.Therefore, it is copied
 * only in the (except for 0) case that SA exists.But, it is copied on prvPsh
 * because received IP header is rewritten.
 */
                if (sadbIntPtr != (ttSadbIntRecordPtr)0)
                {
                    tm_bcopy(pshPtr, &prvPsh, sizeof(ttPseudoHeader));
                    tm_ip_copy(sadbIntPtr->sadbiNatOaDstAddr,
                               pshPtr->pshDstIpAddr);
                    tm_ip_copy(sadbIntPtr->sadbiNatOaSrcAddr,
                               pshPtr->pshSrcIpAddr);
                }
#endif /* TM_USE_NATT && TM_USE_IPSEC */

/*
 * Save ipTtl, and ipChecksum fields modified during UDP checksum
 * computation.
 */
                savedPshZero = pshPtr->pshZero;
                savedPshLayer4Len = pshPtr->pshLayer4Len;
                pshPtr->pshLayer4Len = udphPtr->udpLength; /* network order */
                pshPtr->pshZero = TM_8BIT_ZERO;
#ifdef TM_DEV_RECV_OFFLOAD
                if (    (pktDevRecvOffloadPtr != (ttDevRecvOffloadPtr)0)
                     && (   pktDevRecvOffloadPtr->devoRFlags 
                          & TM_DEVOR_PACKET_RECV_CHKSUM_COMPUTED ) )
                {
/* 
 * Checksum only the pseudo header and add to hardware computed checksum on
 * UDP header + data.
 */
                    checksum = tfDevoRecvPacketChecksum( 
                        pktDevRecvOffloadPtr,
                        (tt16BitPtr)(ttVoidPtr)pshPtr,
                        tm_byte_count(TM_4PAK_PSEUDO_HDR_LEN) );
                }
                else
#endif /* TM_DEV_RECV_OFFLOAD */
                {
                    checksum = tfPacketChecksum(
                        packetPtr,
                        (ttPktLen)udpLength,
                        (tt16BitPtr)(ttVoidPtr)pshPtr,
                        tm_byte_count(TM_4PAK_PSEUDO_HDR_LEN));
                }
                if (checksum != TM_16BIT_ZERO)
                {
#ifdef TM_ERROR_CHECKING
                    tfKernelWarning("tfUdpIncoming",
                                    "Checksum on incoming UDP packet failed");
#endif /* TM_ERROR_CHECKING */
                    goto outUdpIncoming; /* return */
                }
#if (defined(TM_USE_NATT) && defined(TM_USE_IPSEC))
                else
                {
                    if (sadbIntPtr != (ttSadbIntRecordPtr)0)
                    {
/* NAT-OAi and NAT-OAr are copied and because Checksum was calculated,
 * it returns it.
 */
                        tm_ip_copy(prvPsh.pshDstIpAddr,
                                   pshPtr->pshDstIpAddr);
                        tm_ip_copy(prvPsh.pshSrcIpAddr,
                                   pshPtr->pshSrcIpAddr);
            }
        }
#endif /* TM_USE_NATT && TM_USE_IPSEC */
            }
        }
#if (defined(TM_USE_NATT) && defined(TM_USE_IPSEC))
        packetPtr->pktSadbRecordPtr = (ttSadbRecordPtr)0;
#endif /* TM_USE_NATT && TM_USE_IPSEC */
    }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
    else
#endif /* dual layer */
    {
/* Save fields modified during UDP checksum computation. */
        tm_6_copy_zero_arr(psh6Ptr->psh6ZeroArr, saved6PshZero);
        saved6PshLayer4Len = psh6Ptr->psh6Layer4Len;
        if (tm_context(tvUdpIncomingChecksumOn))
        {
#ifdef TM_DEV_RECV_OFFLOAD
            pktDevRecvOffloadPtr = 
                            (ttDevRecvOffloadPtr)packetPtr->pktDevOffloadPtr;
            if (    (pktDevRecvOffloadPtr != (ttDevRecvOffloadPtr)0)
                 && (   pktDevRecvOffloadPtr->devoRFlags 
                       & (   TM_DEVOR_TCP_RECV_CHKSUM_VALIDATED ) ) )
            {
/* No need to validate the checksum */
                checksum = TM_16BIT_ZERO;
            }
            else
#endif /* TM_DEV_RECV_OFFLOAD */
            {
/* 
 * Build the UDP pseudo-header from the received IPv6 packet header 
 */
/* Store the length of the UDP message in the pseudo header. */
/* network order */
                psh6Ptr->psh616BitLayer4Len = udphPtr->udpLength; 
/* network order */
                psh6Ptr->psh616Bit0Layer4Len = TM_16BIT_ZERO; 
                tm_6_zero_arr(psh6Ptr->psh6ZeroArr);
/* Restore because deleted by psh6Zero assignment */
                psh6Ptr->psh6Protocol  = IPPROTO_UDP; 
#ifdef TM_DEV_RECV_OFFLOAD
                if (    (pktDevRecvOffloadPtr != (ttDevRecvOffloadPtr)0)
                     && (   pktDevRecvOffloadPtr->devoRFlags 
                           & (TM_DEVOR_PACKET_RECV_CHKSUM_COMPUTED ) ) )
                {
/* 
 * Checksum only the pseudo header and add to hardware computed checksum on
 * UDP header + data.
 */
                    checksum = tfDevoRecvPacketChecksum( 
                                         pktDevRecvOffloadPtr,
                                         (tt16BitPtr)(ttVoidPtr)psh6Ptr,
                                         (tt16Bit)TM_6_IP_MIN_HDR_LEN );
                }
                else
#endif /* TM_DEV_RECV_OFFLOAD */
                {
/* Compute the UDP checksum on the UDP message and pseudo header */
                    checksum = tfPacketChecksum( packetPtr,
                                                 packetPtr->pktChainDataLength,
                                                 (tt16BitPtr)psh6Ptr,
                                                 (tt16Bit)TM_6_IP_MIN_HDR_LEN);
                }
                if (checksum != TM_16BIT_ZERO)
                {
#ifdef TM_ERROR_CHECKING
                    tfKernelWarning("tfUdpIncoming",
                                    "Checksum on incoming UDP packet failed");
#endif /* TM_ERROR_CHECKING */
                    goto outUdpIncoming; /* return */
                }
            }
        }
    }
#endif /* TM_USE_IPV6 */
/* Target incoming socket */
/* network order, local port */
    tupleDev.tudLocalIpPort = udphPtr->udpDstPort;
#ifdef TM_USE_IKE
#ifdef TM_USE_NATT
    if (tupleDev.tudLocalIpPort == tm_const_htons(TM_IKE_NAT_PORT))
    {
/* Queue to IKE socket */
        tupleDev.tudLocalIpPort = tm_const_htons(TM_IKE_PORT);
    }
#endif /* TM_USE_NATT */
#endif /* TM_USE_IKE */
/* network order, remote port */
    tupleDev.tudRemoteIpPort = udphPtr->udpSrcPort;
/* Prepare Source/Destination IP address for socket lookup */
#ifdef TM_USE_IPV6 
/* Add scope to real IPV6 address */
#ifdef TM_USE_IPV4
    if (iphPtr != (ttIpHeaderPtr)0)
/* Dual mode map, to IPV6 address */
    {
        tupleDev.tudAddressFamily = AF_INET;
        ipv6LocalAddrPtr = &tupleDev.tudLocalIpAddress;
        tm_6_addr_to_ipv4_mapped(iphPtr->iphDestAddr,
                                 ipv6LocalAddrPtr);
#ifdef TM_4_USE_SCOPE_ID
        tm_4_dev_scope_addr(ipv6LocalAddrPtr, packetPtr->pktDestDevPtr);
#endif /* TM_4_USE_SCOPE_ID */
/* Prepare IP header source IP address for connected socket check*/
        ipv6PeerAddrPtr = &tupleDev.tudRemoteIpAddress;
        tm_6_addr_to_ipv4_mapped(iphPtr->iphSrcAddr,
                                 ipv6PeerAddrPtr);
#ifdef TM_4_USE_SCOPE_ID
        tm_4_dev_scope_addr(ipv6PeerAddrPtr, packetPtr->pktDeviceEntryPtr);
#endif /* TM_4_USE_SCOPE_ID */
    }
    else
#endif /* dual */
    {
        tupleDev.tudAddressFamily = AF_INET6;
        ipv6LocalAddrPtr = &tm_6_ip_hdr_in6_addr(iph6Ptr->iph6DestAddr);
/* Add scope to real IPV6 address */
        tm_6_dev_scope_addr(ipv6LocalAddrPtr, packetPtr->pktDestDevPtr);
        tm_6_ip_copy(ipv6LocalAddrPtr, &(tupleDev.tudLocalIpAddress));
/* Prepare IP header source IP address for connected socket check*/
        ipv6PeerAddrPtr = &tm_6_ip_hdr_in6_addr(iph6Ptr->iph6SrcAddr);
/* Add scope to real IPV6 address */
        tm_6_dev_scope_addr(ipv6PeerAddrPtr, packetPtr->pktDeviceEntryPtr);
        tm_6_ip_copy(ipv6PeerAddrPtr, &(tupleDev.tudRemoteIpAddress));
    }
#else /* !TM_USE_IPV6 */
    tupleDev.tudAddressFamily = AF_INET;
/* remote address */
    tm_ip_copy(iphPtr->iphSrcAddr, tupleDev.tudRemoteIpAddress);
/* local address */
    tm_ip_copy(iphPtr->iphDestAddr, tupleDev.tudLocalIpAddress);
#endif /* !TM_USE_IPV6 */
#ifndef TM_DISABLE_UDP_CACHE
/* Check on cache first */
/* LOCK the cache */
    tm_lock_wait(&tm_context(tvUdpCacheLock));
    socketPtr = tm_context(tvUdpSocketPtr);
    if (socketPtr != TM_SOCKET_ENTRY_NULL_PTR)
    {
/* LOCK the socket entry in the cache */
        tm_lock_wait(&socketPtr->socLockEntry);
/* Check that is is ours */
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
        if (    (socketPtr->socOurLayer4Port == tupleDev.tudLocalIpPort)
#ifdef TM_USE_STRONG_ESL
             && (    tm_8bit_bits_not_set(socketPtr->socFlags2,
                                          TM_SOCF2_BINDTODEVICE)
                  || (socketPtr->socRteCacheStruct.rtcDevPtr ==
                                           packetPtr->pktDeviceEntryPtr)
                )
#endif /* TM_USE_STRONG_ESL */
#ifdef TM_USE_REUSEADDR_LIST
             && (    (tm_16bit_bits_not_set(flag, TM_SOC_TREE_M_B_CAST_MATCH))
/*
 * If the socket is in the list of reuse address sockets, then there may be
 * other sockets that match for multicast/broadcast in the reuse address list
 * before this one, so we will have to lookup the socket instead of using the
 * cached one.
 */
                  || (socketPtr->socInReuseAddrList == TM_8BIT_NO)
                )
#endif /* TM_USE_REUSEADDR_LIST */
/* Match on remote port/address if socket is connected. */
             &&
                (
                     (!(socketPtr->socFlags & TM_SOCF_CONNECTED))
                  || ( (udphPtr->udpSrcPort == socketPtr->socPeerLayer4Port) &&
#ifdef TM_USE_IPV6 
                       (tm_6_ip_match(ipv6PeerAddrPtr,
                                      &(socketPtr->socPeerIpAddress)))
#else /* !TM_USE_IPV6 */
                       (tm_ip_match(packetPtr->pktSrcIpAddress,
                                    socketPtr->socPeerIpAddress))
#endif /* !TM_USE_IPV6 */
                     )
                )
             &&
/*
 * Local address Match:
 * 1. exact match,
 * or
 * 2. local socket has a zero IP address, and no other
 * socket can be added at that port with a different local IP address,
 * or
 * 3. destination is multicast, and we are at the head of the list, and
 * socket address is IPv6 or both socket address and destination address
 * are IPv4.
 */
                (
                     (
/* 1. Exact match on local address */
#ifdef TM_USE_IPV6
                        (  tm_6_ip_match(&(socketPtr->socOurIfIpAddress),
                                         ipv6LocalAddrPtr) )
#else /* !TM_USE_IPV6 */
                        (  tm_ip_match(socketPtr->socOurIfIpAddress,
                                       iphPtr->iphDestAddr) )
#endif /* !TM_USE_IPV6 */
                     )
                  || (
/* 2. zero IP address */
#ifdef TM_USE_REUSEADDR_LIST
/*
 * If SO_REUSEADDR or SO_REUSEPORT is set then there may be other sockets that
 * match better in the list (before or after), so we will have to lookup the
 * socket instead of using the cached one.
 */
                         (tm_16bit_bits_not_set(
                                            socketPtr->socOptions,
                                            SO_REUSEADDR | SO_REUSEPORT)) &&
#endif /* TM_USE_REUSEADDR_LIST */
#ifdef TM_USE_IPV6
                         (    (tm_6_ip_zero(&(socketPtr->socOurIfIpAddress)))
                           || (    tm_6_addr_ipv4_mapped_zero(
                                            &(socketPtr->socOurIfIpAddress))
                                && IN6_IS_ADDR_V4MAPPED(ipv6LocalAddrPtr) ) )
#else /* !TM_USE_IPV6 */
                         (tm_ip_zero(socketPtr->socOurIfIpAddress))
#endif /* !TM_USE_IPV6 */
                     )
                  || (
/* 
 * 3. For multicast, or broadcast destination IP address:
 * use any socket (non connection oriented sockets only).
 */
                            (tm_16bit_one_bit_set(flag, TM_SOC_TREE_M_B_CAST_MATCH))
#ifdef TM_USE_IPDUAL
                         && (   (!IN6_IS_ADDR_V4MAPPED(
                                       &socketPtr->socOurIfIpAddress))
/* Local IPv6 address */
                              || (IN6_IS_ADDR_V4MAPPED(ipv6LocalAddrPtr))
/* Both IPv4 addresses. */
                             )
#endif /* TM_USE_IPDUAL */
                     )
                )
#if (defined(TM_IGMP) || defined(TM_6_USE_MLD))
/* Check IGMP socket cache */
            && (    (flag != TM_SOC_TREE_MCAST_MATCH)
/* Filter mcast traffic according to group membership/source filter */
                 || (tfSocketMcastMember(socketPtr
                                        ,packetPtr->pktDeviceEntryPtr
#ifdef TM_USE_IPV6
                                        ,ipv6LocalAddrPtr
#if (defined(TM_USE_IGMPV3) || defined(TM_6_USE_MLDV2))
                                        ,ipv6PeerAddrPtr
#endif /* TM_USE_IGMPV3 || TM_6_USE_MLDV2 */
#else /* !TM_USE_IPV6 */
                                        ,iphPtr->iphDestAddr
#ifdef TM_USE_IGMPV3
                                        ,iphPtr->iphSrcAddr
#endif /* TM_USE_IGMPV3 */
#endif /* !TM_USE_IPV6 */
#ifdef TM_LOCK_NEEDED
                                        ,TM_8BIT_YES
#endif /* TM_LOCK_NEEDED */
                                       )
                    ) 
               )
#endif /* TM_IGMP || TM_6_USE_MLD */
#ifdef TM_USE_IPDUAL
/* Prevent IPv4 sockets from receiving IPv6 packets; there may be an
 * IPv6-only socket available */
            &&  !(socketPtr->socProtocolFamily == AF_INET
                  && tupleDev.tudAddressFamily != AF_INET)
/* Prevent IPv6-only sockets from receiving IPv4 packets; there may be
 * an IPv4 socket available */
            &&  !(tm_16bit_one_bit_set(socketPtr->soc6Flags, TM_6_SOCF_V6ONLY)
                  && tupleDev.tudAddressFamily != AF_INET6)
#endif /* TM_USE_IPDUAL */
           )
/* PRQA L:L1 */
        {
/* Ours: incoming thread has ownership */
            tm_socket_checkout(socketPtr);
/* socketPtr non null, we will need to unlock the socket */
        }
        else
        {
/*
 * Unlock tvUdpSocketPtr.
 */
            tm_unlock(&socketPtr->socLockEntry);
            goto udpLookup;
        }
    }
    else
#endif /* !TM_DISABLE_UDP_CACHE */
    {
#ifndef TM_DISABLE_UDP_CACHE
/* If cache failed, lookup the socket tree */
udpLookup:
#endif /* !TM_DISABLE_UDP_CACHE */
#if (defined(TM_USE_STRONG_ESL) || defined(TM_IGMP) || defined(TM_6_USE_MLD))
        tupleDev.tudDevPtr = packetPtr->pktDeviceEntryPtr;
#endif /* TM_USE_STRONG_ESL || TM_IGMP || TM_6_USE_MLD */
        tupleDev.tudProtocolNumber = TM_IP_UDP;
        socketPtr = tfSocketLookup(&tupleDev, flag);
/* If there is no socket at destination port */
        if (socketPtr == TM_SOCKET_ENTRY_NULL_PTR)
        {
#ifndef TM_DISABLE_UDP_CACHE
/* UNLOCK the cache */
            tm_call_unlock(&tm_context(tvUdpCacheLock));
#endif /* !TM_DISABLE_UDP_CACHE */
#ifdef TM_SNMP_MIB
/*
 * Number of received UDP datagrams for which there was no application at
 * the destination port.
 */
            tm_context(tvUdpData).udpNoPorts++;
#endif /* TM_SNMP_MIB */
#ifdef TM_USE_IPV4
/* Send ICMP error message for port unreachable */
#ifdef TM_USE_IPV6
            if (iphPtr != (ttIpHeaderPtr)0)
#endif /* dual */
            {
                if (pshPtr != (ttPseudoHeaderPtr)0)
/*
 * Restore modified fields during UDP checksum, if we send an ICMP error
 * message
 */
                {
                    pshPtr->pshZero = savedPshZero;
                    pshPtr->pshLayer4Len = savedPshLayer4Len;
                }
/* Point back to IP header */
                packetPtr->pktLinkDataPtr = (tt8BitPtr)(ttVoidPtr)iphPtr;
/* IP header length */
                packetPtr->pktChainDataLength += tm_byte_count(ipHdrLen);
                packetPtr->pktLinkDataLength  += tm_byte_count(ipHdrLen); 
                tfIcmpErrPacket(packetPtr,
                                TM_ICMP_TYPE_UNREACH,
                                TM_ICMP_CODE_PORT,
                                TM_32BIT_ZERO,
                                TM_DEV_NULL_PTR,
                                TM_IP_ZERO);
            }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
            else
#endif /* dual layer */
/* Send ICMP error message for port unreachable */
            {
/*
 * Restore modified fields during UDP checksum, if we send an ICMP error
 * message
 */
                tm_6_copy_zero_arr(saved6PshZero, psh6Ptr->psh6ZeroArr);
                psh6Ptr->psh6Layer4Len = saved6PshLayer4Len;
/* IP header length */
                ipHdrLen = (tt16Bit)
                              (packetPtr->pktLinkDataPtr - (tt8BitPtr)iph6Ptr);
/* Point back to IP header */
                packetPtr->pktLinkDataPtr = (tt8BitPtr)(ttVoidPtr)iph6Ptr;
                packetPtr->pktChainDataLength += tm_byte_count(ipHdrLen);
                packetPtr->pktLinkDataLength  += tm_byte_count(ipHdrLen); 
/* Unscope the IP dest address */
#ifdef TM_SNMP_MIB
                tm_context(tvIcmpv6Data).icmpv6OutDestUnreachs++;
#endif /* TM_SNMP_MIB */
                tm_6_dev_unscope_addr(ipv6LocalAddrPtr);
                tf6IcmpErrPacket(packetPtr,
                                 TM_6_ICMP_TYPE_DEST_UNREACH,
                                 TM_6_ICMP_CODE_NO_PORT,
                                 TM_UL(0));
            }
#endif /* TM_USE_IPV6 */
            packetPtr = TM_PACKET_NULL_PTR;
            goto outUdpIncoming;
        }
#ifndef TM_DISABLE_UDP_CACHE
        else
        {
            if (socketPtr != tm_context(tvUdpSocketPtr))
/* socketPtr non null, update the cache if socketPtr not in the cache. */
            {
/*
 * If there is a cached socket that is different, remember to check it in 
 * later when we no longer have a lock on the current socket.
 */
                purgeCacheSocketPtr = tm_context(tvUdpSocketPtr);
/* Initialize the cache */
                tm_context(tvUdpSocketPtr) = socketPtr;
/* Checkout socket, since it is kept in our cache */
                tm_socket_checkout(socketPtr);
            }
        }
#endif /* !TM_DISABLE_UDP_CACHE */
    }
#ifndef TM_DISABLE_UDP_CACHE
/* UNLOCK the cache */
    tm_unlock(&tm_context(tvUdpCacheLock));
#endif /* !TM_DISABLE_UDP_CACHE */
#ifdef TM_USE_REUSEADDR_LIST
    firstMatch = TM_8BIT_YES;
    do
    {
        if (tm_16bit_one_bit_set(flag, TM_SOC_TREE_M_B_CAST_MATCH))
        {
/*
 * Only attempt to deliver to mutiple sockets if destination IP address is
 * multicast or broadcast
 */
            nextSocketPtr = socketPtr->socReuseAddrNextPtr;
        }
        else
        {
            nextSocketPtr = (ttSocketEntryPtr)0;
        }
#endif /* TM_USE_REUSEADDR_LIST */
#ifdef TM_USE_REUSEADDR_LIST
        if (firstMatch == TM_8BIT_YES)
#endif /* TM_USE_REUSEADDR_LIST */
        {
#ifdef TM_USE_REUSEADDR_LIST
            firstMatch = TM_8BIT_NO;
#endif /* TM_USE_REUSEADDR_LIST */
/* Point to User data */
            packetPtr->pktLinkDataLength -= TM_MAX_UDP_HDR_LEN;
            packetPtr->pktLinkDataPtr =   ((tt8BitPtr)(ttVoidPtr)udphPtr)
                                        + TM_PAK_UDP_HDR_LEN;
            packetPtr->pktChainDataLength =   udpLength
                                            - TM_MAX_UDP_HDR_LEN;
/* Store peer's port number */
            packetPtr->pktSharedDataPtr->dataPeerPortNumber =
                                            udphPtr->udpSrcPort;
/* Store local port number, to support bind to TM_WILD_PORT */
            packetPtr->pktSharedDataPtr->dataLocalPortNumber =
                                            udphPtr->udpDstPort;
#ifdef TM_SNMP_MIB
/* Total number of UDP datagrams delivered to UDP users. */

#ifdef TM_USE_NG_MIB2
            tm_64Bit_incr(tm_context(tvUdpData).udpHCInDatagrams);
#else /* TM_USE_NG_MIB2 */ 
            tm_context(tvUdpData).udpInDatagrams++;
#endif /* !TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
        }
#ifdef TM_SNMP_MIB
/* Number of UDP datagrams delivered for this UDP
 * endpoint (socket)
 */
        tm_64Bit_incr(socketPtr->socHCInDatagrams);
        tm_64Bit_augm(socketPtr->socHCInOctets, udpLength);
#endif /* TM_SNMP_MIB */
#ifdef TM_USE_REUSEADDR_LIST
        if (nextSocketPtr != (ttSocketEntryPtr)0)
        {
            newPacketPtr = tfDuplicatePacket(packetPtr);
            if (newPacketPtr == (ttPacketPtr)0)
            {
/* could not duplicate packet. */
                nextSocketPtr = (ttSocketEntryPtr)0;
            }
        }
        else
        {
            newPacketPtr = TM_PACKET_NULL_PTR;
        }
#endif /* TM_USE_REUSEADDR_LIST */
        errorCode = tfSocketIncomingPacket(socketPtr, packetPtr);
#ifdef TM_USE_REUSEADDR_LIST
        packetPtr = newPacketPtr;
#else /* !TM_USE_REUSEADDR_LIST */
        packetPtr = TM_PACKET_NULL_PTR;
#endif /* !TM_USE_REUSEADDR_LIST */
/*
 * Now that we have queued the data post to any pending thread
 * (recv or select) / CB user.
 */
        if (errorCode == TM_ENOERROR)
        {
            tfSocketNotify(socketPtr, TM_SELECT_READ, TM_CB_RECV, 0);
        }
#ifdef TM_USE_REUSEADDR_LIST
        if (nextSocketPtr != (ttSocketEntryPtr)0)
        {
/* Release ownership */
            tm_socket_checkin_unlock(socketPtr);
/* Get ownership of new one */
            socketPtr = tfSocketCheckOutReused(
#if (defined(TM_USE_STRONG_ESL) || defined(TM_IGMP) || defined(TM_6_USE_MLD))
                                               packetPtr->pktDeviceEntryPtr,
#endif /* TM_USE_STRONG_ESL || TM_IGMP || TM_6_USE_MLD */
                                               nextSocketPtr,
                                               &tupleDev.tudTuple,
                                               flag);
            if (socketPtr == (ttSocketEntryPtr)0)
            {
                break;
            }
        }
    }
    while (nextSocketPtr != (ttSocketEntryPtr)0);
#endif /* TM_USE_REUSEADDR_LIST */
/* Incoming thread no longer has ownership */
outUdpIncoming:
    if (socketPtr != TM_SOCKET_ENTRY_NULL_PTR)
    {
        tm_socket_checkin_unlock(socketPtr);
    }
#ifndef TM_DISABLE_UDP_CACHE
    if (purgeCacheSocketPtr != (ttSocketEntryPtr)0)
    {
/*
 * Previously cached socket was replaced, check it in.
 */
        tm_lock_wait(&(purgeCacheSocketPtr->socLockEntry));
        tm_socket_checkin_unlock(purgeCacheSocketPtr);
    }
#endif /* !TM_DISABLE_UDP_CACHE */
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
#ifdef TM_SNMP_MIB
/*
 * Number of received UDP datagrams that could not be delivered for
 * reasons other than the lack of an application at the destination port.
 */
        tm_context(tvUdpData).udpInErrors++;
#endif /* TM_SNMP_MIB */
/* socket was not locked, or has just been unlocked */
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    }
#ifdef TM_USE_DRV_SCAT_RECV
udpIncomingExit:
#endif /* TM_USE_DRV_SCAT_RECV */
    return;
}
