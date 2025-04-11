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
 * Description: 802.3 Ethernet Link Layer
 *
 * Filename: tr8023.c
 * Author: Paul
 * Date Created: 11/12/04
 * $Source: source/tr8023.c $
 *
 * Modification History
 * $Revision: 6.0.2.4 $
 * $Date: 2010/05/29 08:24:54JST $
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

#ifdef TM_USE_ETHER_8023_LL

int tf8023Send(ttPacketPtr packetPtr);
int tf8023Recv(void TM_FAR *interfaceId, void TM_FAR *bufferHandle);
int tf8023Ioctl(int operation);
int tf8023Error(int type);

typedef struct tsE8023Header
{
#ifdef TM_DSP
/*
 * Size of this struct is 13 without the dummy value. Adding it makes it 14
 * (TM_ETHER_HEADER_SIZE)
 */
    unsigned int dummy;
#endif /* TM_DSP */
    ttEnetAddress ethDstAddr;
    ttEnetAddress ethSrcAddr;
    tt16Bit       ethLength;
    tt8Bit        ethSsap;
    tt8Bit        ethDsap;
    tt8Bit        ethControl;
    tt8Bit        ethOui[3];
    tt16Bit       ethType;
} ttE8023Header;

typedef ttE8023Header TM_FAR * ttE8023HeaderPtr;

#define TM_E8023_LLC_SIZE    8
#define TM_E8023_HEADER_SIZE (TM_ETHER_HEADER_SIZE + TM_E8023_LLC_SIZE)

/* 
 * Used to initialize the link layer in the link layer list
 */
ttUserLinkLayer tfUseE8023(int llMode)
{
    ttLinkLayerEntryPtr linkLayerEntryPtr;

/* Lock the Link Layer list */
    tm_call_lock_wait(&tm_context(tvLinkLayerListLock));
    linkLayerEntryPtr = tm_context(tvLinkLayerListPtr);
    if (llMode > TM_E8023_MODE_ONLY)
    {
        linkLayerEntryPtr = (ttLinkLayerEntryPtr)0;
        goto useEthernetUnlock; /* Bad Parameter */
    }
/* Search for Ethernet Protocol in Link Layer List */
/* Check for duplicate */
    while (linkLayerEntryPtr != TM_LINK_NULL_PTR)
    {
        if (tm_strcmp( (char TM_FAR *)(linkLayerEntryPtr->lnkNameArray),
                       "ETH8023") == TM_STRCMP_EQUAL)
        {
            goto useEthernetUnlock; /* entry already in */
        }
        linkLayerEntryPtr = linkLayerEntryPtr->lnkNextPtr;
    }
/* if none exists then malloc an entry */
    linkLayerEntryPtr = (ttLinkLayerEntryPtr)tm_kernel_malloc(
                                          (unsigned)sizeof(ttLinkLayerEntry));
    if (linkLayerEntryPtr==TM_LINK_NULL_PTR)
    {
/* if the malloc fails then return a null pointer */
        goto useEthernetUnlock;
    }

/*
 * The default MTU for an Ethernet interface should continue to be 1500 for
 * both IPv4 and IPv6.  [RFC2464].R2:10
 */ 
    linkLayerEntryPtr->lnkMtu = 1492;
    
    linkLayerEntryPtr->lnkOpenFuncPtr = tfLinkOpen;
    linkLayerEntryPtr->lnkCloseFuncPtr = tfEtherCommonClose;
    linkLayerEntryPtr->lnkSendFuncPtr = tf8023Send;
    linkLayerEntryPtr->lnkRecvFuncPtr = tf8023Recv;
    linkLayerEntryPtr->lnkIoctlFuncPtr = tf8023Ioctl;
    linkLayerEntryPtr->lnkErrorFuncPtr = tf8023Error;
    linkLayerEntryPtr->lnkMcastFuncPtr = tm_ether_mcast_func;
    linkLayerEntryPtr->lnkNotifyFuncPtr = TM_LL_NOTIFY_FUNC_NULL_PTR;
    linkLayerEntryPtr->lnkPostFuncPtr = tfDeviceSendOnePacket;
    linkLayerEntryPtr->lnkMode = (tt8Bit)llMode;
    linkLayerEntryPtr->lnkHeaderLength = TM_E8023_HEADER_SIZE;
    linkLayerEntryPtr->lnkLinkLayerProtocol = TM_LINK_LAYER_E8023;
    linkLayerEntryPtr->lnkPhysAddrLen = TM_ETHER_ADDRESS_LENGTH;
    tm_ether_copy( tvEtherBroadcast.enuAddress,
                   linkLayerEntryPtr->lnkPhysBroadAddr );
    tm_strcpy((char TM_FAR *)(linkLayerEntryPtr->lnkNameArray),"ETH8023");

#ifdef TM_USE_IPV4
/* ARP for IPv4... */    
    linkLayerEntryPtr->lnkResolveFuncPtr = tfArpResolve;
    linkLayerEntryPtr->lnkResolveCleanFuncPtr = tfArpResolveClean;
    linkLayerEntryPtr->lnkSendRequestFuncPtr =
                                    (ttLnkSendRequestFuncPtr)tfArpSendRequest;
#endif /* TM_USE_IPV4 */
    
#ifdef TM_USE_IPV6
/* Neighbor Discovery for IPv6... */    
    linkLayerEntryPtr->lnk6ResolveFuncPtr = tf6NdResolveAddress;
    linkLayerEntryPtr->lnk6ResolveCleanFuncPtr = tf6NdResolveClean;
    linkLayerEntryPtr->lnk6ResIncomingFuncPtr = tf6NdAdvertIncoming;
    linkLayerEntryPtr->lnk6McastFuncPtr = tf6EtherMcast;
#endif /* TM_USE_IPV6 */
    
/* Stuff it into the table */
    linkLayerEntryPtr->lnkNextPtr = tm_context(tvLinkLayerListPtr);
    tm_context(tvLinkLayerListPtr) = linkLayerEntryPtr;
useEthernetUnlock:
    tm_call_unlock(&tm_context(tvLinkLayerListLock));
    return((ttUserLinkLayer)linkLayerEntryPtr);    
/* return the link layer pointer */
}

/*
 * Create the header for an packet destined for ethernet
 */
#define TM_ETHER_FRAME_UNK  0
#define TM_ETHER_FRAME_DIX  1
#define TM_ETHER_FRAME_8023 2

int tf8023Send(ttPacketPtr packetPtr)
{
    ttEtherHeaderPtr etherHeaderPtr;
    ttE8023HeaderPtr e8023HeaderPtr;
    ttRteEntryPtr    rtePtr;
    int              padding;
    int              retCode;
    tt16Bit          etherHeaderLength;
    ttPktLen         framePayloadLen;
    tt8Bit           rte8023Flags;
    tt8Bit           etherFrameType;
    tt8Bit           llMode;
    
    
    retCode = TM_LL_OKAY;
    llMode=packetPtr->pktDeviceEntryPtr->devLinkLayerProtocolPtr->lnkMode;
    rtePtr=packetPtr->pktSharedDataPtr->dataRtePtr;
/* Check to see if a routing entry is available for this frame */
    etherFrameType = TM_ETHER_FRAME_8023;
    if (llMode != TM_E8023_MODE_ONLY)
    {
        if (rtePtr == (ttRteEntryPtr)0)
        {
/*
 * No routing entry so it must be a broadcast or gratuitous ARP, and cannot be
 * TCP
 */
            if (llMode == TM_E8023_MODE_SECONDARY)
            {
                etherFrameType=TM_ETHER_FRAME_DIX;
            }
        }
        else
        {
            rte8023Flags = rtePtr->rte8023Flags;
            if (rte8023Flags & TM_RTE8023_ETH_RESPONSE)
            {
                if (!(rte8023Flags & TM_RTE8023_ETH_8023))
                {
                    etherFrameType=TM_ETHER_FRAME_DIX;
                }
            }
            else
            {
                if (rte8023Flags & TM_RTE8023_ETH_FIRST)
                {
                    if (rte8023Flags & TM_RTE8023_ETH_8023)
                    {
                        rtePtr->rte8023Flags &= ~TM_RTE8023_ETH_8023;
                        etherFrameType=TM_ETHER_FRAME_DIX;
                    }
                    else
                    {
                        rtePtr->rte8023Flags |= TM_RTE8023_ETH_8023;
                    }
                }
                else
                {
                    switch (llMode)
                    {
/*
 * This case will not be reached because of the above if statement
 *                       case TM_E8023_MODE_ONLY:
 *                       
 *                           rtePtr->rte8023Flags |= TM_RTE8023_ETH_RESPONSE;
 *                           rtePtr->rte8023Flags |= TM_RTE8023_ETH_8023;
 */
                        case TM_E8023_MODE_PRIORITY:
                            rtePtr->rte8023Flags |= TM_RTE8023_ETH_8023;
                            break;
                        case TM_E8023_MODE_SECONDARY:
                            rtePtr->rte8023Flags &= ~TM_RTE8023_ETH_8023;
                            etherFrameType=TM_ETHER_FRAME_DIX;
                            break;
                        default: /* default case unreached */
                            break;
                    }
                    rtePtr->rte8023Flags |= TM_RTE8023_ETH_FIRST;
                }
            }
        }
    }

    framePayloadLen = packetPtr->pktChainDataLength;
    if (etherFrameType == TM_ETHER_FRAME_8023)
    {
        etherHeaderLength = TM_E8023_HEADER_SIZE;
/* for an 802.3 frame, the payload includes the 8-byte LLC header following the
   802.3 Ethernet header */
        framePayloadLen += TM_E8023_LLC_SIZE;
    }
    else
    {
        etherHeaderLength = TM_ETHER_HEADER_SIZE;
    }
/* Put the ethernet header in front of the packet */
    packetPtr->pktLinkDataLength += etherHeaderLength;
    packetPtr->pktChainDataLength = framePayloadLen + TM_ETHER_HEADER_SIZE;
#ifdef TM_DSP
    if (etherFrameType == TM_ETHER_FRAME_8023)
    {
        packetPtr->pktLinkDataPtr -= sizeof(ttE8023Header);
    }
    else
    {
        packetPtr->pktLinkDataPtr -= sizeof(ttEtherHeader);
    }
#else /* !TM_DSP */
    packetPtr->pktLinkDataPtr -= etherHeaderLength;
#endif /* TM_DSP */

/* Min size met? */
    if (packetPtr->pktChainDataLength < TM_ETHER_MIN_PACKET)
    {
/* 
 * We need to pad with zeroes
 */
        padding =  TM_ETHER_MIN_PACKET - (int)packetPtr->pktChainDataLength;
        if (    (packetPtr->pktChainDataLength == packetPtr->pktLinkDataLength)
             && ( (  packetPtr->pktSharedDataPtr->dataBufLastPtr
                       - packetPtr->pktLinkDataPtr) >= TM_ETHER_MIN_PACKET )
           )
/* 
 * Contiguous buffer (i.e. non TCP packet, or TCP packet with data copied at
 * the end of the header), and allocated packet has room to add the padding.
 */
        {
            tm_bzero(   packetPtr->pktLinkDataPtr
                      + packetPtr->pktLinkDataLength,
                      padding);
            packetPtr->pktLinkDataLength = TM_ETHER_MIN_PACKET;
            packetPtr->pktChainDataLength = TM_ETHER_MIN_PACKET;
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
            retCode = tfPacketTailAdd(packetPtr, padding);
        }
    }

    e8023HeaderPtr = (ttE8023HeaderPtr) (packetPtr->pktLinkDataPtr);
    if (    (llMode != TM_E8023_MODE_ONLY) /* could toggle between 2 modes */
         || !(packetPtr->pktSharedDataPtr->dataFlags & TM_BUF_HDR_RECYCLED))
    {
/* this packet header is not recycled, or is not 802.3 only */
/* Point the Ethernet II Header to be the same start as the 802.3 Header */
        etherHeaderPtr = (ttEtherHeaderPtr) e8023HeaderPtr;
        
/* Copy the device address into the packet */
        tm_ether_copy( (packetPtr->pktDeviceEntryPtr->devPhysAddrArr),
                       (etherHeaderPtr->ethSrcAddr));
/* Copy the destination address into the packet */
        tm_ether_copy( (packetPtr->pktEthernetAddress),
                       (etherHeaderPtr->ethDstAddr));
        if (etherFrameType == TM_ETHER_FRAME_8023)
        {
/* the length that we store in the 802.3 Ethernet header is the size of
   the LLC packet following it, which includes the LLC header and IP packet */
            e8023HeaderPtr->ethLength = htons((tt16Bit) framePayloadLen);
            e8023HeaderPtr->ethSsap = (tt8Bit)0xAA;
            e8023HeaderPtr->ethDsap = (tt8Bit)0xAA;
            e8023HeaderPtr->ethControl = (tt8Bit)0x03;
            e8023HeaderPtr->ethOui[0] = (tt8Bit)0x00;
            e8023HeaderPtr->ethOui[1] = (tt8Bit)0x00;
            e8023HeaderPtr->ethOui[2] = (tt8Bit)0x00;

/* Set the packet type */
#ifdef TM_USE_IPV4
            if (packetPtr->pktNetworkLayer == TM_NETWORK_IPV4_LAYER)
            {
                e8023HeaderPtr->ethType = TM_ETHER_IP_TYPE;
            }
            else
#endif /* TM_USE_IPV4 */
            {
#ifdef TM_USE_IPV6
                if (packetPtr->pktNetworkLayer == TM_NETWORK_IPV6_LAYER)
                {
                    e8023HeaderPtr->ethType = TM_6_ETHER_IP_TYPE;
                }
                else
#endif /* TM_USE_IPV6 */
                {    
                    if (packetPtr->pktNetworkLayer == TM_NETWORK_ARP_LAYER)
                    {
                        e8023HeaderPtr->ethType = TM_ETHER_ARP_TYPE;
                    }
                    else
                    {
/* Should never happen */
                        retCode = TM_EPROTONOSUPPORT;
                    }
                }
            }
        }
        else
        {
/* Set the packet type */
#ifdef TM_USE_IPV4
            if (packetPtr->pktNetworkLayer == TM_NETWORK_IPV4_LAYER)
            {
                etherHeaderPtr->ethType = TM_ETHER_IP_TYPE;
            }
            else
#endif /* TM_USE_IPV4 */
            {
#ifdef TM_USE_IPV6
                if (packetPtr->pktNetworkLayer == TM_NETWORK_IPV6_LAYER)
                {
                    etherHeaderPtr->ethType = TM_6_ETHER_IP_TYPE;
                }
                else
#endif /* TM_USE_IPV6 */
                {    
                    if (packetPtr->pktNetworkLayer == TM_NETWORK_ARP_LAYER)
                    {
                        etherHeaderPtr->ethType = TM_ETHER_ARP_TYPE;
                    }
                    else
                    {
/* Should never happen */
                        retCode = TM_EPROTONOSUPPORT;
                    }
                }
            }
        }
    }
    else if (etherFrameType == TM_ETHER_FRAME_8023)
    {
/* this packet header is recycled: update the payload length in the 802.3
   Ethernet frame header */
        e8023HeaderPtr->ethLength = htons((tt16Bit) framePayloadLen);
    }

    return (retCode);
}

/* 
 * Process an incoming Ethernet
 * We convert from the exposed USER API functions to the
 * Internal form since this is called from the device code
 */
int tf8023Recv(void TM_FAR *interfaceId, void TM_FAR *bufferHandle)
{
    ttEtherHeaderPtr        etherHeaderPtr;
    ttE8023HeaderPtr        e8023HeaderPtr;
    ttPacketPtr             packetPtr;
    ttDeviceEntryPtr        devEntryPtr;
    tt8BitPtr               pktEtherAddressPtr;
    tt8BitPtr               etherHeaderAddressPtr;
    tt16Bit                 etherType;
    int                     flags;
    int                     retCode;
#ifdef TM_DEV_RECV_OFFLOAD
    ttDevRecvOffloadPtr     pktDevRecvOffloadPtr;
#endif /* TM_DEV_RECV_OFFLOAD */

    retCode = TM_ENOERROR;
    packetPtr = (ttPacketPtr)bufferHandle;
    devEntryPtr = (ttDeviceEntryPtr)interfaceId;
/* Store the device the packet came from */
    packetPtr->pktDeviceEntryPtr = devEntryPtr;
#ifdef TM_USE_DRV_SCAT_RECV
/* If Ethernet header is not contiguous, make it contiguous */
    if (tm_pkt_hdr_is_not_cont(packetPtr, TM_ETHER_HEADER_SIZE))
    {
        packetPtr = tfContiguousSharedBuffer(packetPtr, TM_ETHER_HEADER_SIZE);
        if (packetPtr == (ttPacketPtr)0)
        {
            retCode = TM_ENOBUFS;
            goto etherRecvExit;
        }
    }
#endif /* TM_USE_DRV_SCAT_RECV */

/* write the packet into the pcap file */
#ifdef TM_USE_SNIFF
#ifdef TM_KERNEL_WIN32_X86
    if(packetPtr != TM_PACKET_NULL_PTR)
    {
        tfPcapWritePacket(packetPtr);
    }
#endif /* TM_KERNEL_WIN32_X86 */
#endif /* TM_USE_SNIFF */


/* Check the type field */
    etherHeaderPtr = (ttEtherHeaderPtr)(packetPtr->pktLinkDataPtr);
/* Check to see if the type field is Ethernet II */
    if (ntohs(etherHeaderPtr->ethType) > 0x600)
    {
        e8023HeaderPtr = (ttE8023HeaderPtr)0;
        etherType=etherHeaderPtr->ethType;
        packetPtr->pktSharedDataPtr->dataFlags &= ~TM_BUF_LL_E8023;
#ifdef TM_DEV_RECV_OFFLOAD
        pktDevRecvOffloadPtr = (ttDevRecvOffloadPtr)packetPtr->pktDevOffloadPtr;
        if (    (pktDevRecvOffloadPtr != (ttDevRecvOffloadPtr)0)
             && (   pktDevRecvOffloadPtr->devoRFlags 
                  & (TM_DEVOR_PACKET_RECV_CHKSUM_COMPUTED ) ) )
        {
            tfDevoRecvPeelChecksum(packetPtr, (tt16BitPtr)etherHeaderPtr, 
                                   (tt16Bit)TM_ETHER_HEADER_SIZE);
        }
#endif /* TM_DEV_RECV_OFFLOAD */
#ifdef TM_DSP
        packetPtr->pktLinkDataPtr += sizeof(ttEtherHeader);
#else /* !TM_DSP */
        packetPtr->pktLinkDataPtr += TM_ETHER_HEADER_SIZE;
#endif /* TM_DSP */
        packetPtr->pktLinkDataLength -= TM_ETHER_HEADER_SIZE;
        packetPtr->pktChainDataLength -= TM_ETHER_HEADER_SIZE;
    }
/* Type field is really length so the frame is 802.3 */
    else
    {
        e8023HeaderPtr = (ttE8023HeaderPtr)(packetPtr->pktLinkDataPtr);
        packetPtr->pktSharedDataPtr->dataFlags |= TM_BUF_LL_E8023;
        etherType=e8023HeaderPtr->ethType;
#ifdef TM_DEV_RECV_OFFLOAD
        pktDevRecvOffloadPtr = (ttDevRecvOffloadPtr)packetPtr->pktDevOffloadPtr;
        if (    (pktDevRecvOffloadPtr != (ttDevRecvOffloadPtr)0)
             && (   pktDevRecvOffloadPtr->devoRFlags 
                  & (TM_DEVOR_PACKET_RECV_CHKSUM_COMPUTED ) ) )
        {
            tfDevoRecvPeelChecksum(packetPtr, (tt16BitPtr)etherHeaderPtr, 
                                   (tt16Bit)TM_E8023_HEADER_SIZE);
        }
#endif /* TM_DEV_RECV_OFFLOAD */
#ifdef TM_DSP
        packetPtr->pktLinkDataPtr += sizeof(ttE8023Header);
#else /* !TM_DSP */
        packetPtr->pktLinkDataPtr += TM_E8023_HEADER_SIZE;
#endif /* TM_DSP */
        packetPtr->pktLinkDataLength -= TM_E8023_HEADER_SIZE;
        packetPtr->pktChainDataLength -= TM_E8023_HEADER_SIZE;
    }

/* Copy the source ethernet address into the packet */
    pktEtherAddressPtr = (tt8BitPtr)packetPtr->pktEthernetAddress;
    etherHeaderAddressPtr = (tt8BitPtr)etherHeaderPtr->ethSrcAddr;
    tm_ether_copy(etherHeaderAddressPtr, pktEtherAddressPtr);
/*
 * Look at the ethernet destination address to check for destination
 * Ethernet broadcast/multicast.
 */
    etherHeaderAddressPtr = (tt8BitPtr)etherHeaderPtr->ethDstAddr;
    if (tm_ether_broadcast(etherHeaderAddressPtr))
    {
#if (defined(TM_SNMP_MIB) || defined(TM_PPP_LQM))
#ifdef TM_USE_IPV4
        if (etherHeaderPtr->ethType == TM_ETHER_IP_TYPE)
        {
            tm_64Bit_incr(devEntryPtr->dev4Ipv4Mib.ipIfStatsHCInBcastPkts);
#ifdef TM_USE_NG_MIB2
            tm_64Bit_incr(tm_context(tvDevIpv4Data).ipIfStatsHCInBcastPkts);
#endif /* TM_USE_NG_MIB2 */
        }
#endif /* TM_USE_IPV4 */
#endif /* TM_SNMP_MIB or TM_PPP_LQM */
        flags = TM_LL_BROADCAST_FLAG;

    }
    else
    {
        if (tm_ether_mcast(etherHeaderAddressPtr))
        {
#if (defined(TM_SNMP_MIB) || defined(TM_PPP_LQM))
#ifdef TM_USE_IPV4
            if (etherHeaderPtr->ethType == TM_ETHER_IP_TYPE)
            {
                tm_64Bit_incr(devEntryPtr->dev4Ipv4Mib.ipIfStatsHCInMcastPkts);
#ifdef TM_USE_NG_MIB2
                tm_64Bit_incr(tm_context(tvDevIpv4Data).ipIfStatsHCInMcastPkts);
#endif /* TM_USE_NG_MIB2 */
                tm_64Bit_augm(devEntryPtr->dev4Ipv4Mib.ipIfStatsHCInMcastOctets,
                 packetPtr->pktUserStruct.pktuLinkDataLength);
#ifdef TM_USE_NG_MIB2
                tm_64Bit_augm(tm_context(tvDevIpv4Data).ipIfStatsHCInMcastOctets,
                 packetPtr->pktUserStruct.pktuLinkDataLength);
#endif /* TM_USE_NG_MIB2 */
            }
#endif /* TM_USE_IPV4 */
#endif /* TM_SNMP_MIB or TM_PPP_LQM */
            flags = TM_LL_MULTICAST_FLAG;
        }
        else
        {
#if (defined(TM_SNMP_MIB) || defined(TM_PPP_LQM))
            devEntryPtr->devInUcastPkts++;
#endif /* TM_SNMP_MIB or TM_PPP_LQM */
            flags = TM_LL_UNICAST_FLAG;
        }
    }
    switch (etherType)
    {
#ifdef TM_USE_IPV4
        case TM_ETHER_ARP_TYPE:
            packetPtr->pktFlags = 0;
            retCode = tfArpIncomingPacket(packetPtr);
            break;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV4
        case TM_ETHER_IP_TYPE:
            packetPtr->pktFlags = (tt16Bit) flags;
            tfIpIncomingPacket(packetPtr
#ifdef TM_USE_IPSEC_TASK
                               ,(ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
                              );
            break;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
        case TM_6_ETHER_IP_TYPE:
            packetPtr->pktFlags = (tt16Bit) flags;
            tf6IpIncomingPacket(packetPtr
#ifdef TM_USE_IPSEC_TASK
                               ,(ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
                               );
            break;
#endif /* TM_USE_IPV6 */
            
        default:
/* Packet Not For TRECK */
#ifdef TM_SNMP_MIB
#ifdef TM_USE_IPV4
            devEntryPtr->dev4Ipv4Mib.ipIfStatsInUnknownProtos++;
#ifdef TM_USE_NG_MIB2
            tm_context(tvDevIpv4Data).ipIfStatsInUnknownProtos++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_USE_IPV4 */
#endif /* TM_SNMP_MIB */
            tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
            retCode = TM_EPROTONOSUPPORT;
            break;
    }
#ifdef TM_USE_DRV_SCAT_RECV
etherRecvExit:
#endif /* TM_USE_DRV_SCAT_RECV */

    return(retCode);
}

int tf8023Ioctl(int operation)
{
    TM_UNREF_IN_ARG(operation);
    return TM_LL_OKAY;
}

int tf8023Error(int type)
{
    TM_UNREF_IN_ARG(type);
    return TM_LL_OKAY;
}

#else /* !TM_USE_ETHER_8023_LL */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */


/* To allow link for builds when TM_USE_ETHER_8023_LL is not defined */
int tv8023Dummy = 0;

#endif /* !TM_USE_ETHER_8023_LL */
