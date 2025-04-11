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
 * Description: BSD Sockets Interface (connect)
 *
 * Filename: trconn.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trconn.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:42:52JST $
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

/*
 * Connect for TCP and set up a machine to send to and receive from on UDP
 */
int connect(int                           socketDescriptor, 
            const struct sockaddr TM_FAR *addressPtr,
            int                           addressLength)
{
    ttSocketEntryPtr           socketPtr;
#ifdef TM_USE_TCP
    ttTcpVectPtr               tcpVectPtr;
#endif /* TM_USE_TCP */
    ttSockAddrPtrUnion         internetAddressPtrU;
#ifdef TM_USE_IPV4
    ttPacketPtr                packetPtr;
    ttIpHeader                 iphStruct; /* 20 bytes */
#endif /* TM_USE_IPV4 */
    ttSockTuple                sockTuple;
    tt16Bit                    treeFlag;
    int                        retCode;
    int                        errorCode;
#if defined(TM_USE_IPV4) && defined(TM_USE_IPV6)
    int                        addrFamily;
#endif /* defined(TM_USE_IPV4) && defined(TM_USE_IPV6) */
#ifdef TM_USE_BSD_DOMAIN
    int                        af;
#endif /* TM_USE_BSD_DOMAIN */
#ifdef TM_USE_IPV6
    ttRtCacheEntryPtr          rtcPtr;
    ttDeviceEntryPtr           devPtr;
    tt8Bit                     destIpAddrType;
    tt8Bit                     cacheAddrTypeFlag;    
#endif /* TM_USE_IPV6 */
    tt8Bit                     reinsertInTree;

    errorCode = TM_ENOERROR;
    reinsertInTree = TM_8BIT_NO;

/* PARAMETER CHECK */

/*
 * Map from socket descriptor to a locked socket entry pointer and verify
 * that the address length is at least what we expect for this address type.
 */    
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_ANY;
#endif /* TM_USE_BSD_DOMAIN */
    socketPtr = tfSocketCheckAddrLock(socketDescriptor,
                                      addressPtr,
                                      addressLength,
                                      &errorCode
#ifdef TM_USE_BSD_DOMAIN
                                    , &af
#endif /* TM_USE_BSD_DOMAIN */
                                     );
#ifdef TM_USE_BSD_DOMAIN
    if (af == TM_BSD_FAMILY_NONINET) 
    {
        tm_assert(connect, socketPtr != TM_SOCKET_ENTRY_NULL_PTR);
        retCode = tfBsdConnect( (ttVoidPtr)socketPtr, addressPtr,
                                addressLength );
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
        if (socketPtr == TM_SOCKET_ENTRY_NULL_PTR)
        {
/* errorCode set by tfSocketCheckAndLock */
            goto connectFinish;
        }
        if ( addressPtr == (const struct sockaddr TM_FAR *)0 )
        {
            errorCode = TM_EINVAL;
            goto connectFinish;
        }
    
/* Check the address to make sure it is valid */
        internetAddressPtrU.sockPtr = (struct sockaddr TM_FAR *)addressPtr; 
    
#ifdef TM_USE_TCP
        if ( socketPtr->socProtocolNumber == IP_PROTOTCP )
        {
/* TCP protocols */
            tcpVectPtr = (ttTcpVectPtr)((ttSocketEntryPtr)socketPtr);
/* Point to the user structure */
            tcpVectPtr->tcpsSockAddrUPtr = &internetAddressPtrU;
            errorCode = tfTcpOpen( tcpVectPtr,
                                   TM_TCPE_ACTIVE_OPEN );
/*
 * Make sure we reset the connect parameter pointer, in case tfTcpStateMachine
 * does not call tfTcpSendSyn().
 */
            tcpVectPtr->tcpsSockAddrUPtr = (ttSockAddrPtrUnionPtr)0;
        }
        else
#endif /* TM_USE_TCP */
        {
/* Non TCP protocols */
            tm_bcopy(&(socketPtr->socTuple), &sockTuple,
                     sizeof(sockTuple));
            treeFlag = (tt16Bit)(   socketPtr->socOptions
                                  & (SO_REUSEADDR | SO_REUSEPORT) );
            if (tm_16bit_bits_not_set(socketPtr->socFlags, 
                                      TM_SOCF_IN_TREE))
            {
/* Pick random port */
                treeFlag = (tt16Bit)(   treeFlag
                                      | TM_SOC_TREE_RANDOM_PORT);
            }
/*
 * Load up the socket entry unconditionally for non TCP protocols 
 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
            if (socketPtr->socProtocolFamily == AF_INET6)
            {
#endif /* TM_USE_IPV4 */
                tm_6_ip_copy_structs(
                    internetAddressPtrU.sockIn6Ptr->sin6_addr,
                    sockTuple.sotRemoteIpAddress);
#ifdef TM_USE_IPV4
                if (tm_6_addr_is_ipv4_mapped(&(sockTuple.sotRemoteIpAddress)))
                {
#if defined(TM_USE_IPV4) && defined(TM_USE_IPV6)
                    addrFamily = AF_INET;

#ifdef TM_4_USE_SCOPE_ID
                    tm_4_addr_add_scope(
                        &(sockTuple.sotRemoteIpAddress),
                        internetAddressPtrU.sockIn6Ptr->sin6_scope_id);
#endif /* TM_4_USE_SCOPE_ID */
#endif /* defined(TM_USE_IPV4) && defined(TM_USE_IPV6) */
                }
                else
#endif /* TM_USE_IPV4 */
                {
                    tm_6_addr_add_scope(
                        &(sockTuple.sotRemoteIpAddress),
                        internetAddressPtrU.sockIn6Ptr->sin6_scope_id);
#if defined(TM_USE_IPV4) && defined(TM_USE_IPV6)
                    addrFamily = AF_INET6;
#endif /* defined(TM_USE_IPV4) && defined(TM_USE_IPV6) */
                }
#ifdef TM_USE_IPV4
            }
            else
            {
/* AF_INET */            
                tm_6_addr_to_ipv4_mapped((tt4IpAddress)
                        (internetAddressPtrU.sockInPtr->sin_addr.s_addr),
                        &(sockTuple.sotRemoteIpAddress));
#if defined(TM_USE_IPV4) && defined(TM_USE_IPV6)
                addrFamily = AF_INET;
#endif /* defined(TM_USE_IPV4) && defined(TM_USE_IPV6) */
            }
#endif /* TM_USE_IPV4 */
#else /* ! TM_USE_IPV6 */
            tm_ip_copy(
                (tt4IpAddress)(internetAddressPtrU.sockInPtr->sin_addr.s_addr),
                sockTuple.sotRemoteIpAddress);
#if defined(TM_USE_IPV4) && defined(TM_USE_IPV6)
            addrFamily = AF_INET;
#endif /* defined(TM_USE_IPV4) && defined(TM_USE_IPV6) */
#endif /* ! TM_USE_IPV6 */
            sockTuple.sotRemoteIpPort =
                           (ttIpPort)(internetAddressPtrU.sockInPtr->sin_port);
/* Try and set the local IP address */
            if (
#ifdef TM_USE_IPV6
                IN6_IS_ADDR_UNSPECIFIED(&socketPtr->socOurIfIpAddress) 
#ifdef TM_USE_IPV4
/* dual IP layer */
                || tm_6_addr_ipv4_mapped_zero(
                                           &socketPtr->socOurIfIpAddress)
#endif /* TM_USE_IPV4 */
#else /* ! TM_USE_IPV6 */
/* IPv4 only */
                tm_ip_zero(socketPtr->socOurIfIpAddress)
#endif /* ! TM_USE_IPV6 */
                )
            {
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
                if (addrFamily == AF_INET6)
                {
#endif /* TM_USE_IPV4 */
                    tm_6_ip_copy_structs(
                        sockTuple.sotRemoteIpAddress,
                        socketPtr->socRteCacheStruct.rtcDestIpAddr); 

                    rtcPtr = &(socketPtr->socRteCacheStruct);
                    errorCode = tfRtGet(rtcPtr);

                    if (errorCode != TM_ENOERROR)
                    {
/* No route found for destination address. If this is a multicast address,
 * check for a default multicast device for this socket.
 */
                        if (IN6_IS_ADDR_MULTICAST(
                                &(sockTuple.sotRemoteIpAddress)))
                        {
                            devPtr = socketPtr->soc6McastDevPtr;
                            if (devPtr == TM_DEV_NULL_PTR)
                            {
/*
 * No route to this multicast address could be found, and the user did not
 * set the IPV6_MULTICAST_IF option and did not bind to a local IP address.
 * We have no way to determine a source address; quit and return an error.
 */ 
                                goto connectFinish;
                            }
                            else
                            {
/* Use the default multicast interface for this socket.  Since there's no
 * routing entry, don't try to cache the address type.
 */ 
                                destIpAddrType = TM_6_ADDR_TYPE_UNDEFINED;
                                cacheAddrTypeFlag = TM_8BIT_ZERO;
                            }
                        }
                        else
                        {
/* Couldn't find a route to the specified unicast address. */
                            goto connectFinish;
                        }
                    }
                    else
                    {
/* Route found to the destination; use the cached address type */
                        destIpAddrType = rtcPtr->rtcDestIpAddrType;
                        devPtr = rtcPtr->rtcRtePtr->rteDevPtr;
                        cacheAddrTypeFlag = TM_8BIT_YES;
                    }

/* Pick an appropriate source address for our destination, embed the scope ID */
                    errorCode =
                        tf6SelectSourceAddress(devPtr,
                              &(sockTuple.sotLocalIpAddress),
                              (ttDeviceEntryPtrPtr)0,
                              TM_16BIT_NULL_PTR,
                              &(sockTuple.sotRemoteIpAddress),
                              &destIpAddrType,
                              TM_6_EMBED_SCOPE_ID_FLAG);
                                                       
                    if (errorCode == TM_ENOERROR)
                    {
/* If we got one from tfRtGet, store the address type of the destination in
 * the routing cache entry.
 */ 
                        if (cacheAddrTypeFlag == TM_8BIT_YES)
                        {
                            rtcPtr->rtcDestIpAddrType = destIpAddrType;
                        }
/* Save source IP address for getsockname() benefit */                        
                        reinsertInTree = TM_8BIT_YES;
                    }
#ifdef TM_USE_IPV4
                }
                else
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
                {
/* Allocate an empty ttPacket/ttSharedData */
                    packetPtr = tfGetSharedBuffer(0,
                                                  0,
                                                  TM_16BIT_ZERO);
                    if (packetPtr != TM_PACKET_NULL_PTR)
                    {

                        errorCode = tfIpDestToPacket(
                            packetPtr,
#ifdef TM_4_USE_SCOPE_ID
                            &in6addr_any,
                            &(sockTuple.sotRemoteIpAddress),
#else /* ! TM_4_USE_SCOPE_ID */
                            TM_IP_ZERO,
                            tm_4_ip_addr(sockTuple.sotRemoteIpAddress),
#endif /* ! TM_4_USE_SCOPE_ID */
                            1,
                            &socketPtr->socRteCacheStruct,
                            &iphStruct );
                        if (errorCode == TM_ENOERROR)
                        {
/* Save source IP address for getsockname() benefit */
#ifdef TM_USE_IPV6
                            tm_6_addr_to_ipv4_mapped(
                                iphStruct.iphSrcAddr,
                                &(sockTuple.sotLocalIpAddress));
#else /* !TM_USE_IPV6 */
                            tm_ip_copy(
                                   iphStruct.iphSrcAddr,
                                   tm_4_ip_addr(sockTuple.sotLocalIpAddress));
#endif /* !TM_USE_IPV6 */
#ifdef TM_4_USE_SCOPE_ID
                            if (tm_16bit_one_bit_set(
                                    packetPtr->pktFlags,
                                    TM_OUTPUT_DEVICE_FLAG))
                            {
/* we know the output device, so use it to embed the scope ID if the IPv4
   address is link-local scope */
                                tm_4_addr_add_scope(
                                    &(sockTuple.sotLocalIpAddress),
                                    packetPtr->pktDeviceEntryPtr->devIndex);
                            }
#endif /* !TM_4_USE_SCOPE_ID */
                            reinsertInTree = TM_8BIT_YES;
                        }

                        tfFreeSharedBuffer(packetPtr, TM_SOCKET_LOCKED);
                    }
                    else
                    {
                        errorCode = TM_ENOBUFS;
                    }
                }
#endif /* TM_USE_IPV4 */              
            }
            else
            {
                if (tm_16bit_bits_not_set(socketPtr->socFlags,
                                          TM_SOCF_IN_TREE))
                {
                    reinsertInTree = TM_8BIT_YES;
                }
            }
        
            if (errorCode == TM_ENOERROR)
            {
                if (reinsertInTree)
                {
                    retCode = tfSocketTreeReInsert(
                                        socketPtr,
                                        &sockTuple,
                                        treeFlag);

                    if (retCode != TM_SOC_RB_OKAY)
                    {
                        errorCode = TM_EADDRINUSE;
                    }
                }
                else
                {
                    tm_bcopy(&sockTuple,
                             &(socketPtr->socTuple),
                             sizeof(sockTuple));
                }
                if (errorCode == TM_ENOERROR)
                {
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
                    if (addrFamily == AF_INET6)
#endif /* TM_USE_IPV4 */
                    {
/* Save the flow label and traffic class values from the user */
                        socketPtr->soc6FlowInfo =
                                internetAddressPtrU.sockIn6Ptr->sin6_flowinfo;
                    }
#endif /* TM_USE_IPV6 */
                    socketPtr->socFlags |= TM_SOCF_CONNECTED;
                }
            }
        }
connectFinish:
        retCode = tfSocketReturn(socketPtr, socketDescriptor,
                                 errorCode, TM_ENOERROR);
    }
    return retCode;
}
