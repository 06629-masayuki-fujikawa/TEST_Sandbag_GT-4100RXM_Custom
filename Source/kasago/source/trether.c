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
 * Description: Ethernet Link Layer
 *
 * Filename: trether.c
 * Author: Paul
 * Date Created: 11/10/97
 * $Source: source/trether.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:47:22JST $
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


int tfEtherSend(ttPacketPtr packetPtr);
int tfEtherIoctl(int operation);
int tfEtherError(int type);


/* 
 * Used to initialize the link layer in the link layer list
 */
 
ttUserLinkLayer tfUseEthernet(void)
{
    ttLinkLayerEntryPtr linkLayerEntryPtr;
    int errorCode;
/* Search for Ethernet Protocol in Link Layer List */

/* Lock the Link Layer list */
    tm_call_lock_wait(&tm_context(tvLinkLayerListLock));
    linkLayerEntryPtr = tm_context(tvLinkLayerListPtr);
/* Check for duplicate */
    while (linkLayerEntryPtr != TM_LINK_NULL_PTR)
    {
        errorCode = tm_strcmp( (ttCharPtr)(linkLayerEntryPtr->lnkNameArray),
                               "ETHDIX");
        if( errorCode == TM_STRCMP_EQUAL )
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
    linkLayerEntryPtr->lnkMtu = 1500;
    
    linkLayerEntryPtr->lnkOpenFuncPtr = tfLinkOpen;
    linkLayerEntryPtr->lnkCloseFuncPtr = tfEtherCommonClose;
    linkLayerEntryPtr->lnkSendFuncPtr = tfEtherSend;
    linkLayerEntryPtr->lnkRecvFuncPtr = tfEtherRecv;
    linkLayerEntryPtr->lnkIoctlFuncPtr = tfEtherIoctl;
    linkLayerEntryPtr->lnkErrorFuncPtr = tfEtherError;
    linkLayerEntryPtr->lnkMcastFuncPtr = tm_ether_mcast_func;
    linkLayerEntryPtr->lnkNotifyFuncPtr = TM_LL_NOTIFY_FUNC_NULL_PTR;
    linkLayerEntryPtr->lnkPostFuncPtr = tfDeviceSendOnePacket;
    linkLayerEntryPtr->lnkHeaderLength = TM_ETHER_HEADER_SIZE;
    linkLayerEntryPtr->lnkLinkLayerProtocol = TM_LINK_LAYER_ETHERNET;
    linkLayerEntryPtr->lnkPhysAddrLen = TM_ETHER_ADDRESS_LENGTH;
    tm_ether_copy( tvEtherBroadcast.enuAddress,
                   linkLayerEntryPtr->lnkPhysBroadAddr );
    tm_strcpy((ttCharPtr)(linkLayerEntryPtr->lnkNameArray),"ETHDIX");

#ifdef TM_USE_IPV4
/* ARP for IPv4... */    
    linkLayerEntryPtr->lnkResolveFuncPtr = tfArpResolve;
    linkLayerEntryPtr->lnkSendRequestFuncPtr =
        (ttLnkSendRequestFuncPtr) tfArpSendRequest;
    linkLayerEntryPtr->lnkResolveCleanFuncPtr = tfArpResolveClean;
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
int tfEtherSend(ttPacketPtr packetPtr)
{
    ttEtherHeaderPtr etherHeaderPtr;
    int              padding;
    int              retCode;
    
    
    retCode = TM_LL_OKAY;
    
/* Put the ethernet header in front of the packet */
    packetPtr->pktLinkDataLength += TM_ETHER_HEADER_SIZE;
    packetPtr->pktChainDataLength += TM_ETHER_HEADER_SIZE;
#ifdef TM_DSP
    packetPtr->pktLinkDataPtr -= sizeof(ttEtherHeader);
#else /* !TM_DSP */
    packetPtr->pktLinkDataPtr -= TM_ETHER_HEADER_SIZE;
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
    if (!(packetPtr->pktSharedDataPtr->dataFlags & TM_BUF_HDR_RECYCLED))
    {
        etherHeaderPtr = (ttEtherHeaderPtr)(packetPtr->pktLinkDataPtr);
        
/* Copy the device address into the packet */
        tm_ether_copy( (packetPtr->pktDeviceEntryPtr->devPhysAddrArr),
                       (etherHeaderPtr->ethSrcAddr));
/* Copy the destination address into the packet */
        tm_ether_copy( (packetPtr->pktEthernetAddress),
                       (etherHeaderPtr->ethDstAddr));
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
    return (retCode);
}

/* 
 * Process an incoming Ethernet
 * We convert from the exposed USER API functions to the
 * Internal form since this is called from the device code
 */
int tfEtherRecv(void TM_FAR *interfaceId, void TM_FAR *bufferHandle)
{
    ttEtherHeaderPtr        etherHeaderPtr;
    ttPacketPtr             packetPtr;
    ttDeviceEntryPtr        devEntryPtr;
    tt8BitPtr               pktEtherAddressPtr;
    tt8BitPtr               etherHeaderAddressPtr;
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
#ifdef TM_USE_NG_MIB2
                tm_64Bit_incr(tm_context(tvDevIpv4Data).ipIfStatsHCInMcastPkts);
                tm_64Bit_augm(tm_context(tvDevIpv4Data).ipIfStatsHCInMcastOctets,
                    packetPtr->pktUserStruct.pktuLinkDataLength);
#endif /* TM_USE_NG_MIB2 */
                tm_64Bit_incr(devEntryPtr->dev4Ipv4Mib.ipIfStatsHCInMcastPkts);
                tm_64Bit_augm(devEntryPtr->dev4Ipv4Mib.ipIfStatsHCInMcastOctets,
                    packetPtr->pktUserStruct.pktuLinkDataLength);
            }
            else
#endif /* TM_USE_IPV4 */
            {
#ifdef TM_USE_IPV6
                if (etherHeaderPtr->ethType == TM_6_ETHER_IP_TYPE)
                {
#ifdef TM_USE_NG_MIB2
                    tm_64Bit_incr(devEntryPtr->dev6Ipv6Mib.ipIfStatsHCInMcastPkts);
                    tm_64Bit_incr(tm_context(tvDevIpv6Data).ipIfStatsHCInMcastPkts);
                    tm_64Bit_augm(devEntryPtr->dev6Ipv6Mib.ipIfStatsHCInMcastOctets,
                        packetPtr->pktUserStruct.pktuLinkDataLength);
                    tm_64Bit_augm(tm_context(tvDevIpv6Data).ipIfStatsHCInMcastOctets,
                        packetPtr->pktUserStruct.pktuLinkDataLength);
#endif /* TM_USE_NG_MIB2 */
                }
#endif /* TM_USE_IPV6 */
            }
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
    switch (etherHeaderPtr->ethType)
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
                                , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
                                );
            break;
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_PPPOE
/* Ethernet type during Discovery phase */
        case TM_PPPOE_DISC_ETH_TYPE:
/* Ethernet type during PPP session phase */
        case TM_PPPOE_PPP_ETH_TYPE:
            tfPppoeIncomingPacket(packetPtr, etherHeaderPtr->ethType);
            break;
#endif /* TM_USE_PPPOE */
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

int tfEtherIoctl(int operation)
{
    TM_UNREF_IN_ARG(operation);
    return TM_LL_OKAY;
}

int tfEtherError(int type)
{
    TM_UNREF_IN_ARG(type);
    return TM_LL_OKAY;
}

