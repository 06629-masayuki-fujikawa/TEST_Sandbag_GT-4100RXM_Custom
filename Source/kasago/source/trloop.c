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
 * Description: Loopback Link Layer & Driver
 *
 * Filename: trloop.c
 * Author: Paul
 * Date Created: 11/10/97
 * $Source: source/trloop.c $
 *
 * Modification History
 * $Revision: 6.0.2.4 $
 * $Date: 2010/08/24 08:15:30JST $
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

#define TM_NULL_LINK_LAYER_MTU 1500

/*
 * Define the TM_DISABLE_LOOP_INLINE_SEND_RECV macro to test loop back 
 * driver with a separate recv task. In that case the loop back 
 * received data will no longer be processed in the send path, but will be 
 * processed when tfRecvInterface/tfRecvScatInterface is called. In the
 * examples directory, the txscatlp.c, and txscatdr.c modules contain sample
 * code that uses this feature.
 */
/* #define TM_DISABLE_LOOP_INLINE_SEND_RECV */

static ttUserLinkLayer tfInitNullLinkLayer(void);
    
static int tfLoopbackDrvSend( ttUserInterface  interfaceHandle,
                              char     TM_FAR  *dataPtr,
                              int              dataLength,
                             int              flag );

static int tfLoopbackDrvRecv( ttUserInterface    interfaceHandle,
                              ttCharPtr TM_FAR  *dataPtrPtr,
                              ttIntPtr           dataSizePtr,
                              ttUserBufferPtr    packetPtrPtr
#ifdef TM_DEV_RECV_OFFLOAD
                     , ttDevRecvOffloadPtr recvOffloadPtr
#endif /* TM_DEV_RECV_OFFLOAD */
                     );



int tfUseLoopBack (void)
{
    int                      errorCode;
    ttUserInterface          interfaceHandle;
#ifdef TM_USE_IPV6
    ttDeviceEntryPtr         devEntryPtr;
#endif /* TM_USE_IPV6 */
   
    errorCode = TM_ENOERROR;
    tm_context(tvNullLinkLayerHandle) = (ttLinkLayerEntryPtr)
                                                 tfInitNullLinkLayer();
/* Add the Interface */
    interfaceHandle = tfAddInterface("LOOPBACK",
                                     tm_context(tvNullLinkLayerHandle),
                                     (ttDevOpenCloseFuncPtr)0,
                                     (ttDevOpenCloseFuncPtr)0,
                                     tfLoopbackDrvSend,
/*
 * Dummy tfLoopbackDrvRecv(). Not called since tfRecvInterface() is
 * not called for a loop back device.
 */
                                     tfLoopbackDrvRecv,
                                     (ttDevFreeRecvFuncPtr)0,
                                     (ttDevIoctlFuncPtr)0,
                                     (ttDevGetPhysAddrFuncPtr)0,
                                     &errorCode);
    if (errorCode == TM_ENOERROR)
    {
        tm_context(tvLoopbackDevPtr) = (ttDeviceEntryPtr)interfaceHandle;

#ifdef TM_USE_IPV4
/* Config the IPv4 Interface */
        errorCode = tf4ConfigInterface(interfaceHandle,
                                      tm_const_htonl(TM_UL(0x7F000001)),
                                      TM_UL(0xFFFFFFFF),
                                      TM_DEV_SCATTER_SEND_ENB,
                                      1501,
                                      TM_16BIT_ZERO);
       if (errorCode == TM_ENOERROR)
       {
/*
 * Add a reject route to network 127.0.0.0 to reject all outgoing packets to
 * network 127.0.0.0 (except to 127.0.0.1 which is a host route configured
 * above).
 */
            errorCode = tfAddStaticRoute(
                (ttUserInterface)tm_context(tvLoopbackDevPtr),
                (tt4IpAddress)tm_const_htonl(TM_UL(0x7F000000)),
                (tt4IpAddress)tm_const_htonl(TM_UL(0xFF000000)),
                (tt4IpAddress)tm_const_htonl(TM_UL(0x7F000001)),
                1 );

#ifdef TM_ERROR_CHECKING 
            if (errorCode != TM_ENOERROR)
            {
                tfKernelError(
                    "tfInitLoopBack",
                    "Could not add IPv4 router reject Loop back entry");
            }
#endif /* TM_ERROR_CHECKING */
        }
#ifdef TM_ERROR_CHECKING 
        else
        {
            tfKernelError("tfInitLoopBack",
                          "Could not configure IPv4 Loop back interface");
        }
#endif /* TM_ERROR_CHECKING */
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
/* Config the IPv6 Interface */
        devEntryPtr = (ttDeviceEntryPtr) interfaceHandle;
        tm_call_lock_wait(&(devEntryPtr->devLockEntry));

/*
 * This is a loopback interface, and so doesn't really have an interface
 * identifier, but we need to set this flag because we expect other
 * link layer types to have an interface ID set before calling tfLinkOpen
 * and tfDeviceStart.
 */ 
        devEntryPtr->dev6Flags |= TM_6_DEV_INTERFACE_ID_FLAG;
        
        tm_6_ip_copy_structs(in6addr_loopback,
                             devEntryPtr->dev6Pt2PtPeerIpAddr);

        tm_6_ip_copy_structs(in6addr_loopback,
                             devEntryPtr->dev6IpAddrArray[0]);
        devEntryPtr->dev6PrefixLenArray[0] = TM_6_IP_HOST_PREFIX_LENGTH;

/*
 * Open the device and link layer for IPv6.
 * We don't want to call tfNgConfigInterface for the IPv6 loopback address,
 * because it does too much - address autoconfiguration, DAD, etc.  Just mark
 * the loopback device as opened.
 */
        devEntryPtr->devFlag |= TM_DEV_SCATTER_SEND_ENB | TM_DEV_OPENED;
        devEntryPtr->devScatterExtraCount = 1500;
        errorCode = (*(devEntryPtr->devLinkLayerProtocolPtr->lnkOpenFuncPtr))
            (((void TM_FAR *) interfaceHandle), (tt16Bit) 0, PF_INET6);

        if (errorCode == TM_ENOERROR)
        {
            errorCode = tfDeviceStart(devEntryPtr, 0, PF_INET6);
        }
        
        tm_call_unlock(&(devEntryPtr->devLockEntry));
        
#ifdef TM_ERROR_CHECKING 
        if (errorCode != TM_ENOERROR)
        {
            tfKernelError("tfInitLoopBack",
                          "Could not configure IPv6 Loop back interface");
        }
#endif /* TM_ERROR_CHECKING */
       
#endif /* TM_USE_IPV6 */
    }
#ifdef TM_ERROR_CHECKING 
    else
    {
        tfKernelError("tfInitLoopBack", "Could not add Loop back interface");
    }
#endif /* TM_ERROR_CHECKING */
    return errorCode;
}

ttUserLinkLayer tfUseNullLinkLayer ( void )
{
#if (defined(TM_SINGLE_INTERFACE_HOME) || defined(TM_SINGLE_INTERFACE))
    if (tm_context(tvNullLinkLayerHandle) == TM_LINK_NULL_PTR)
    {
        tm_context(tvNullLinkLayerHandle) = (ttLinkLayerEntryPtr)
                                                    tfInitNullLinkLayer();
    }
#endif /* TM_SINGLE_INTERFACE_HOME or TM_SINGLE_INTERFACE */

    return (ttUserLinkLayer)tm_context(tvNullLinkLayerHandle);
}

/* 
 * Used to initialize the link layer in the link layer list
 */

static ttUserLinkLayer tfInitNullLinkLayer ( void )
{
    ttLinkLayerEntryPtr linkLayerEntryPtr;
    int isEqual;
/* Search for NULL in Link Layer List */
/* Lock the Link Layer list */
    tm_call_lock_wait(&tm_context(tvLinkLayerListLock));
    linkLayerEntryPtr = tm_context(tvLinkLayerListPtr);
/* Check for duplicate */
    while (linkLayerEntryPtr != TM_LINK_NULL_PTR)
    {
        isEqual = tm_strcmp( (ttCharPtr)(linkLayerEntryPtr->lnkNameArray),
                             "NULL");
        if( isEqual == TM_STRCMP_EQUAL )
        {
            goto initNullLinkUnlock; /* entry already in */
        }
        linkLayerEntryPtr = linkLayerEntryPtr->lnkNextPtr;
    }
/* if none exists then malloc an entry */
    linkLayerEntryPtr = (ttLinkLayerEntryPtr)
                         tm_kernel_malloc((unsigned)sizeof(ttLinkLayerEntry));
    if (linkLayerEntryPtr == TM_LINK_NULL_PTR)
    {
/* if the malloc fails then return a null pointer */
        goto initNullLinkUnlock;
    }
    tm_bzero(linkLayerEntryPtr, sizeof(ttLinkLayerEntry));
/*
 * NOTE: Need to make a global for the MTU
 */
    linkLayerEntryPtr->lnkMtu = TM_NULL_LINK_LAYER_MTU;
    linkLayerEntryPtr->lnkOpenFuncPtr = tfLinkOpen;
    linkLayerEntryPtr->lnkCloseFuncPtr = tfNullLinkClose;
    linkLayerEntryPtr->lnkSendFuncPtr = tfNullLinkSend;
    linkLayerEntryPtr->lnkRecvFuncPtr = tfNullLinkRecv;
    linkLayerEntryPtr->lnkIoctlFuncPtr = tfNullLinkIoctl;
    linkLayerEntryPtr->lnkErrorFuncPtr = tfNullLinkError;
    linkLayerEntryPtr->lnkPostFuncPtr = tfDeviceSendOnePacket;
    linkLayerEntryPtr->lnkLinkLayerProtocol = TM_LINK_LAYER_NULL;
    tm_strcpy((ttCharPtr)(linkLayerEntryPtr->lnkNameArray), "NULL");
/* Stuff it into the table */
    linkLayerEntryPtr->lnkNextPtr = tm_context(tvLinkLayerListPtr);
    tm_context(tvLinkLayerListPtr) = linkLayerEntryPtr;

initNullLinkUnlock:
    tm_call_unlock(&tm_context(tvLinkLayerListLock));
    return((ttUserLinkLayer)linkLayerEntryPtr);    
/* return the link layer pointer */
}
    
/*
 * Close an Loopback Link Layer
 * Does nothing
 */
int tfNullLinkClose ( void TM_FAR *interfaceId )
{

#ifdef TM_USE_IPV6
    tm_16bit_clr_bit( ((ttDeviceEntryPtr)interfaceId)->dev6Flags,
                      (TM_6_LL_CONNECTING | TM_6_LL_CONNECTED) );

#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
    tm_kernel_set_critical;
    tm_context(tvIpData).ipv6InterfaceTableLastChange = 
        tm_snmp_time_ticks(tvTime);
    tm_kernel_release_critical;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
#else /* !TM_USE_IPV6 */
/* Loopback does not need a close for IPv4 */
    TM_UNREF_IN_ARG(interfaceId);
#endif /* TM_USE_IPV6 */

    return TM_LL_OKAY;
}

/*
 * Send an IP Packet
 */
int tfNullLinkSend ( ttPacketPtr packetPtr )
{
    int retCode;
    
    
    TM_UNREF_IN_ARG(packetPtr);
    retCode = TM_LL_OKAY;
    
    return (retCode);
}

/* 
 * Process an incoming packet
 * We convert from the exposed USER API functions to the
 * Internal form since this is called from the device code
 */
int tfNullLinkRecv ( void TM_FAR *interfaceId, void TM_FAR *bufferHandle )
{
    ttPacketPtr         packetPtr;
    tt8Bit              ipVersion;
    int                 retCode;
    ttIpHeaderPtr       iphPtr;         /* pointer to IP header */

    retCode = TM_ENOERROR;
    packetPtr = (ttPacketPtr)bufferHandle;
#ifndef TM_LOOP_TO_DRIVER
    if (packetPtr->pktOrigDevPtr != (ttDeviceEntryPtr)0)
    {
        packetPtr->pktDeviceEntryPtr = packetPtr->pktOrigDevPtr;
    }
    else
#endif /* !TM_LOOP_TO_DRIVER */
    {
        packetPtr->pktDeviceEntryPtr = (ttDeviceEntryPtr)interfaceId;
    }
#if (defined(TM_SNMP_MIB) || defined(TM_PPP_LQM))
/* link layer recv routine */
    ((ttDeviceEntryPtr)interfaceId)->devInUcastPkts++;
#endif /* TM_SNMP_MIB or TM_PPP_LQM */

/* we use ttIpHeader to get the IP header version, this code works for DSPs */
    iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
    ipVersion = tm_ip_version(iphPtr->iphVersLen);

    switch (ipVersion)
    {
#ifdef TM_USE_IPV4
        case TM_IP_4:
            tfIpIncomingPacket(packetPtr
#ifdef TM_USE_IPSEC_TASK
                               , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
                               );
            break;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
        case TM_IP_6:
            tf6IpIncomingPacket(packetPtr
#ifdef TM_USE_IPSEC_TASK
                                , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
                                );
            break;
#endif /* TM_USE_IPV6 */

        default:
/* Packet Not For Turbo TRECK */
#ifdef TM_SNMP_MIB
#ifdef TM_USE_IPV4
            ((ttDeviceEntryPtr)interfaceId)->
                        dev4Ipv4Mib.ipIfStatsInUnknownProtos++;
#ifdef TM_USE_NG_MIB2
            tm_context(tvDevIpv4Data).ipIfStatsInUnknownProtos++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_USE_IPV4 */
#endif /* TM_SNMP_MIB */
            tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
            retCode = TM_EPROTONOSUPPORT;
            break;
    }
    return retCode;
}

int tfNullLinkIoctl ( int operation )
{
    TM_UNREF_IN_ARG(operation);
    return TM_LL_OKAY;
}

int tfNullLinkError ( int type )
{
    TM_UNREF_IN_ARG(type);
    return TM_LL_OKAY;
}

int tfLoopbackDrvSend ( ttUserInterface  interfaceHandle,
                        char     TM_FAR  *dataPtr,
                        int              dataLength,
                        int              flag )
{
    ttDeviceEntryPtr devEntryPtr;
    ttPacketPtr      packetPtr;
    int              errorCode;
#ifdef TM_DSP
    unsigned int     destOffset;
#endif /* TM_DSP */

    errorCode = TM_ENOERROR;
    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;

/* 
 * Check if there is an existing packet that we are writing into.
 * In scattered send case, we concatenate onto the end of an existing packet
 * buffer when we do the copy. 
 */
#ifdef TM_DISABLE_LOOP_INLINE_SEND_RECV
/* Multiple packet queue. Check packet at the end of the queue. */
    packetPtr = devEntryPtr->devRcvLastQueuePtr;
#else /* !TM_DISABLE_LOOP_INLINE_SEND_RECV */
/* Single packet queue */
    packetPtr = devEntryPtr->devReceiveQueuePtr;
#endif /* !TM_DISABLE_LOOP_INLINE_SEND_RECV */
/* copy into a single buffer */
    if (    (packetPtr == TM_PACKET_NULL_PTR)
#ifdef TM_DISABLE_LOOP_INLINE_SEND_RECV
         || tm_16bit_bits_not_set( packetPtr->pktUserFlags, 
                                   TM_PKTF_USER_DEV_WRITE )
#endif /* TM_DISABLE_LOOP_INLINE_SEND_RECV */
       )
    {
/* 
 * Either there is no packet currently in the queue, or packet at the
 * end of the queue is ready to be received. 
 */
        packetPtr = tfGetSharedBuffer( 0,
                                       (ttPktLen)TM_NULL_LINK_LAYER_MTU,
                                       TM_16BIT_ZERO );
        if (packetPtr == TM_PACKET_NULL_PTR)
        {
#ifdef TM_SNMP_MIB
#ifdef TM_USE_IPV4
            devEntryPtr->dev4Ipv4Mib.ipIfStatsOutDiscards++;
#ifdef TM_USE_NG_MIB2
            tm_context(tvDevIpv4Data).ipIfStatsOutDiscards++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_USE_IPV4 */
#endif /* TM_SNMP_MIB */
            errorCode = TM_ENOBUFS;
            goto loopbackSendFinish;
        }
/* Queue packet */
#ifdef TM_DISABLE_LOOP_INLINE_SEND_RECV
/* Packet is being written into */
        packetPtr->pktUserFlags |= TM_PKTF_USER_DEV_WRITE;
        packetPtr->pktRecvNextPtr = (ttPacketPtr)0;
        if (devEntryPtr->devReceiveQueuePtr == TM_PACKET_NULL_PTR)
        {
/* Empty queue. */
            devEntryPtr->devReceiveQueuePtr = packetPtr;
            devEntryPtr->devRcvLastQueuePtr = packetPtr;
        }
        else
        {
/* Non Empty queue. Queue at the end of the queue */
            devEntryPtr->devRcvLastQueuePtr->pktRecvNextPtr = packetPtr;
            devEntryPtr->devRcvLastQueuePtr = packetPtr;
        }
#else /* !TM_DISABLE_LOOP_INLINE_SEND_RECV */
/* Single packet queue */
        devEntryPtr->devReceiveQueuePtr = packetPtr;
#endif /* !TM_DISABLE_LOOP_INLINE_SEND_RECV */
        devEntryPtr->devReceiveDataPtr = packetPtr->pktLinkDataPtr;
        devEntryPtr->devReceiveDataLength = 0;
    }
#ifdef TM_DSP
    destOffset = devEntryPtr->devReceiveDataLength % TM_DSP_BYTES_PER_WORD;
    if (destOffset != 0)
    {
        tfMemCopyOffset( (int *) dataPtr, 0,
                         (int *) devEntryPtr->devReceiveDataPtr,
                         destOffset,
                         dataLength );
/* update pointer to next word to copy to */
        devEntryPtr->devReceiveDataPtr +=
            tm_packed_byte_len(dataLength + destOffset);
    }
    else
#endif /* TM_DSP */
    {
        tm_bcopy(dataPtr, 
                 devEntryPtr->devReceiveDataPtr, 
                 (unsigned)tm_packed_byte_count(dataLength));
/* update pointer to next word to copy to */
        devEntryPtr->devReceiveDataPtr += tm_packed_byte_len(dataLength);
    }
    devEntryPtr->devReceiveDataLength =
        (tt16Bit) (devEntryPtr->devReceiveDataLength + (tt16Bit)dataLength);
    if (flag != TM_USER_BUFFER_MORE)
    {
#ifndef TM_LOOP_TO_DRIVER
       packetPtr->pktOrigDevPtr = devEntryPtr->devOrigDevPtr;
#endif /* !TM_LOOP_TO_DRIVER */
        tfSendCompleteInterface( interfaceHandle,
                                 TM_DEV_SEND_COMPLETE_DRIVER );
/* The final segment so send it up */
/*
 * We can inline the tfRecvInterface() function keeping only what we need
 * (if TM_DISABLE_LOOP_INLINE_SEND_RECV is not defined), otherwise the user 
 * needs to call tfRecvInterface().
 */
#ifdef TM_DISABLE_LOOP_INLINE_SEND_RECV
        packetPtr->pktLinkDataLength = devEntryPtr->devReceiveDataLength;
        packetPtr->pktUserFlags = TM_PKTF_USER_DEV_OWNS;
/* Notify the Treck stack that we've received a packet */
        tfNotifyInterfaceTask(interfaceHandle, 1, 0, 0UL, 0UL);
#else /* !TM_DISABLE_LOOP_INLINE_SEND_RECV */
        dataLength = (int)devEntryPtr->devReceiveDataLength;
        packetPtr->pktLinkDataLength = (ttPktLen)dataLength;
        packetPtr->pktChainDataLength = (ttPktLen)dataLength;
        devEntryPtr->devReceiveQueuePtr = TM_PACKET_NULL_PTR;
/* Unlock the device send/receive */
        tm_unlock(&(devEntryPtr->devDriverLockEntry));
        errorCode = (*(devEntryPtr->devLinkLayerProtocolPtr->lnkRecvFuncPtr))(
                                               (void TM_FAR *)interfaceHandle,
                                               (void TM_FAR *)packetPtr);
        tm_lock_wait(&(devEntryPtr->devDriverLockEntry));
/* End of in line tfRecvInterface */
#endif /* !TM_DISABLE_LOOP_INLINE_SEND_RECV */
    }
loopbackSendFinish:
    return errorCode;    
}

/*
 * Loopback Driver Receive function.
 * Never called if TM_DISABLE_LOOP_INLINE_SEND_RECV is not defined.
 * Otherwise called from the loop back driver send routine.
 */
int tfLoopbackDrvRecv ( ttUserInterface     interfaceHandle,
                        ttCharPtr TM_FAR  * dataPtrPtr,
                        ttIntPtr            dataSizePtr,
                        ttUserBufferPtr     packetPtrPtr
#ifdef TM_DEV_RECV_OFFLOAD
                      , ttDevRecvOffloadPtr recvOffloadPtr
#endif /* TM_DEV_RECV_OFFLOAD */
                      )
{
#ifdef TM_DISABLE_LOOP_INLINE_SEND_RECV
    ttDeviceEntryPtr devEntryPtr;
    ttPacketPtr      packetPtr;
    int              errorCode;

#ifdef TM_DEV_RECV_OFFLOAD
    TM_UNREF_IN_ARG(recvOffloadPtr); /* compiler warning for unused parameter */
#endif /* TM_DEV_RECV_OFFLOAD */
    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
/* Check at head of the queue */
    packetPtr = devEntryPtr->devReceiveQueuePtr;
    if (    (packetPtr != TM_PACKET_NULL_PTR) 
         && tm_16bit_bits_not_set(packetPtr->pktFlags,
                                  TM_PKTF_USER_DEV_WRITE) )
/* Packet in the queue, and ready to be received */
    {
/* unlink this packet from the head of the queue of received packets */
        devEntryPtr->devReceiveQueuePtr = 
                devEntryPtr->devReceiveQueuePtr->pktRecvNextPtr;
        if (devEntryPtr->devReceiveQueuePtr == (ttPacketPtr)0)
        {
/* Empty queue */
            devEntryPtr->devRcvLastQueuePtr = (ttPacketPtr)0;
        }
        packetPtr->pktRecvNextPtr = (ttPacketPtr)0;
        *dataSizePtr = (int)packetPtr->pktLinkDataLength;
        *dataPtrPtr = (ttCharPtr)packetPtr->pktLinkDataPtr;
        errorCode = TM_ENOERROR;
    }
    else
    {
        packetPtr = (ttPacketPtr)0;
        errorCode = TM_ENOBUFS;
    }
    *packetPtrPtr = packetPtr;
    return errorCode;
#else /* !TM_DISABLE_LOOP_INLINE_SEND_RECV */
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(dataPtrPtr);
    TM_UNREF_IN_ARG(dataSizePtr);
    TM_UNREF_IN_ARG(packetPtrPtr);
#ifdef TM_DEV_RECV_OFFLOAD
    TM_UNREF_IN_ARG(recvOffloadPtr); /* compiler warning for unused parameter */
#endif /* TM_DEV_RECV_OFFLOAD */
    return TM_ENOERROR;
#endif /* !TM_DISABLE_LOOP_INLINE_SEND_RECV */
}
