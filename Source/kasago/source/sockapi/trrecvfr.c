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
 * Description: BSD Sockets Interface (recvfrom)
 *
 * Filename: trrecvfr.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trrecvfr.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:05JST $
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
 * Take some data from the receive queue and give it to the user 
 */
int recvfrom(int                     socketDescriptor, 
             char            TM_FAR *bufferPtr, 
             int                     bufferLength, 
             int                     flags,
             struct sockaddr TM_FAR *fromAddressPtr,
             int             TM_FAR *fromAddressLengthPtr)
{
    ttSocketEntryPtr   socketEntryPtr;
    ttPacketPtr        packetPtr;
    ttPktLen           copyCount;
    ttPktLen           recvdPacketLength;
    ttPktLen           linkCopyCount;
    int                errorCode;
    ttPacketPtr        recvdPacketPtr;
    ttSockAddrPtrUnion tempSockAddr;
    int                addressLength;
    int                retCode;
#ifdef TM_USE_BSD_DOMAIN
    int                af;
#endif /* TM_USE_BSD_DOMAIN */

    tempSockAddr.sockPtr = fromAddressPtr; 

    errorCode = TM_ENOERROR;
    recvdPacketLength = (ttPktLen)0;
/* PARAMETER CHECK */
    if (fromAddressLengthPtr != (int TM_FAR *)0)
    {
        addressLength = *fromAddressLengthPtr;
    }
    else
    {
        addressLength = 0;
    }
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_ANY;
#endif /* TM_USE_BSD_DOMAIN */
/*
 * Map from socket descriptor to a locked socket entry pointer and verify
 * that the address length is at least what we expect for this address type.
 */    
    socketEntryPtr = tfSocketCheckAddrLenLock(  socketDescriptor,
                                                fromAddressPtr,
                                                addressLength,
                                                &errorCode
#ifdef TM_USE_BSD_DOMAIN
                                              , &af
#endif /* TM_USE_BSD_DOMAIN */
                                              );
#ifdef TM_USE_BSD_DOMAIN
    if (af == TM_BSD_FAMILY_NONINET)
    {
        tm_assert(recvfrom, socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR);
        retCode = tfBsdRecvFrom( socketEntryPtr, bufferPtr, bufferLength, 
                                 flags, fromAddressPtr, fromAddressLengthPtr );
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
        if ( socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR )
        {
/* errorCode set by tfSocketCheckAndLock */
            goto recvfromFinish;
        }
        if (    (bufferPtr == (char TM_FAR *)0)
/* we allow bufferlength between 0 and 65534 on 16-bit architecture */
             || (bufferLength == -1)
             || (bufferLength == 0)
             || ((flags & ~TM_RECVFROM_FLAGS) != 0) )
        {
            errorCode = TM_EINVAL;
            goto recvfromFinish;
        }
        
/*Check to see if it is a TCP socket */
        if (socketEntryPtr->socProtocolNumber == IP_PROTOTCP)
        {
            errorCode = TM_EPROTOTYPE;
            goto recvfromFinish;
        }
/* If user did not set any wait/dontwait flag */
        if ((flags & TM_SOCF_MSG_WAIT_FLAGS) ==  0)
        {
/* use the socket default */
            flags |= (socketEntryPtr->socFlags & TM_SOCF_NONBLOCKING);
        }
        packetPtr = socketEntryPtr->socReceiveQueueNextPtr;
/* Check for Data */
        if (packetPtr == TM_PACKET_NULL_PTR)
        {
            errorCode = socketEntryPtr->socRecvError;
            if (errorCode != TM_ENOERROR)
            {
                socketEntryPtr->socRecvError = 0;
                goto recvfromFinish;
            }
            if (flags & TM_SOCF_NONBLOCKING)
            {
                errorCode = TM_EWOULDBLOCK;
                goto recvfromFinish;
            }
            else
            {
#ifdef TM_PEND_POST_NEEDED
                errorCode = tm_pend(&socketEntryPtr->socRecvPendEntry,
                                    &socketEntryPtr->socLockEntry);
                if (errorCode != TM_ENOERROR)
                {
                    socketEntryPtr->socRecvError = 0;
                }
                packetPtr = socketEntryPtr->socReceiveQueueNextPtr;
/* Check for Data */
                if (    (errorCode != TM_ENOERROR) 
                     || (packetPtr==TM_PACKET_NULL_PTR) )
                {
                    if (errorCode == TM_ENOERROR)
                    {
                        errorCode = TM_EWOULDBLOCK;
                    }
                    goto recvfromFinish;
                }
#else /* TM_PEND_POST_NEEDED */
                socketEntryPtr->socRecvError = 0;
                goto recvfromFinish;
#endif /* TM_PEND_POST_NEEDED */
            }
        }
        if (fromAddressPtr != TM_SOCKADDR_NULL_PTR)
        {
/* Save the from address */
#ifdef TM_USE_IPV6
            if (socketEntryPtr->socProtocolFamily == AF_INET6)
            {
                *fromAddressLengthPtr = sizeof(struct sockaddr_in6);
                tempSockAddr.sockIn6Ptr->sin6_len =
                    sizeof(struct sockaddr_in6);
                tempSockAddr.sockIn6Ptr->sin6_family = AF_INET6;
                tempSockAddr.sockIn6Ptr->sin6_port = (unsigned short)
                    (packetPtr->pktSharedDataPtr->dataPeerPortNumber);
                tempSockAddr.sockIn6Ptr->sin6_scope_id = (u_long) 0;
    
/*
 * When running in dual stack mode, it's possible to receive IPv4 packets
 * on a PF_INET6 socket, assuming that there's no PF_INET socket bound to
 * the same port.  So if this packet is an IPv4 packet, convert the source
 * address into a IPv4-mapped IPv6 address for the sockaddr_in6 struct.
 */ 
#ifdef TM_USE_IPV4
                if (packetPtr->pktNetworkLayer == TM_NETWORK_IPV4_LAYER)
                {
                    tm_6_addr_to_ipv4_mapped(
                        packetPtr->pktSrcIpAddress,
                        &tempSockAddr.sockIn6Ptr->sin6_addr );

#ifdef TM_4_USE_SCOPE_ID
                    if (IN4_IS_ADDR_LINKLOCAL(packetPtr->pktSrcIpAddress))
                    {
/* link-local scope */
                        tempSockAddr.sockIn6Ptr->sin6_scope_id = (ttUser32Bit) 
                            packetPtr->pktDeviceEntryPtr->devIndex;
                    }
#endif /* TM_4_USE_SCOPE_ID */
                }
                else
#endif /* TM_USE_IPV4 */
                {
                    tm_6_ip_copy_structs(
                        tm_6_ip_hdr_in6_addr(
                            packetPtr->pkt6RxIphPtr->iph6SrcAddr),
                        tempSockAddr.sockIn6Ptr->sin6_addr );
    
                    if (tm_6_addr_is_local(
                            &(tempSockAddr.sockIn6Ptr->sin6_addr)))
                    {
                        tm_6_dev_unscope_addr(
                            &tempSockAddr.sockIn6Ptr->sin6_addr);
                        
/* set the scope ID from the interface */
#ifdef TM_DSP
                        if ((tm_dsp_get_byte_larr(
                                 tempSockAddr.sockIn6Ptr->sin6_addr.s6LAddr, 1)
                             & (ttUser8Bit) 0xc0) == (ttUser8Bit) 0x80)
#else /* !TM_DSP */
                        if ((tempSockAddr.sockIn6Ptr->sin6_addr.s6_addr[1]
                             & (ttUser8Bit) 0xc0) == (ttUser8Bit) 0x80)
#endif /* TM_DSP */
                        {
/* link-local scope */
                            tempSockAddr.sockIn6Ptr->sin6_scope_id
                                = (ttUser32Bit) 
                                packetPtr->pktDeviceEntryPtr->devIndex;
                        }
#ifndef TM_6_USE_RFC3879
/* to depricate sitelocal address, do not set scope ID */
                        else
                        {
/* site-local scope */
                            tempSockAddr.sockIn6Ptr->sin6_scope_id
                                = (ttUser32Bit) 
                                packetPtr->pktDeviceEntryPtr->dev6SiteId;
                        }
#endif /* TM_6_USE_RFC3879 */
                    }
                }
            }
            else
#endif /* TM_USE_IPV6 */
            {
#ifdef TM_USE_IPV4
                *fromAddressLengthPtr = sizeof(struct sockaddr_in);
                tempSockAddr.sockInPtr->sin_len = sizeof(struct sockaddr_in);
                tempSockAddr.sockInPtr->sin_family = AF_INET;
                tempSockAddr.sockInPtr->sin_port = (unsigned short)
                    (packetPtr->pktSharedDataPtr->dataPeerPortNumber);
                tm_ip_copy(packetPtr->pktSrcIpAddress,
                           tempSockAddr.sockInPtr->sin_addr.s_addr);
#endif /* TM_USE_IPV4 */
            }
            
        }    
/* Most common case. User has enough room in its buffer. */
        copyCount = packetPtr->pktChainDataLength;
        recvdPacketPtr = packetPtr;
        recvdPacketLength = copyCount;
        if (copyCount > (ttPktLen)bufferLength)
        {
/* Return truncated packet */
            errorCode = TM_EMSGSIZE;
/* Copy only up to user buffer length */
            copyCount = (ttPktLen)bufferLength;
        }
        do
        {
             if (copyCount < packetPtr->pktLinkDataLength)
             {
/* Copy only up to user buffer length */
                linkCopyCount = copyCount;
             }
             else
             {
                linkCopyCount = packetPtr->pktLinkDataLength;
             }
/* Copy this data area to the user buffer */
#ifdef TM_DSP
            if (socketEntryPtr->socOptions & SO_UNPACKEDDATA)
            {
                tm_bcopy_to_unpacked(
                                   (ttConstVoidPtr)(packetPtr->pktLinkDataPtr),
                                   (int)(packetPtr->pktLinkDataByteOffset),
                                   (ttVoidPtr)bufferPtr,
                                   linkCopyCount);
                bufferPtr += linkCopyCount;
            }
            else
#endif /* TM_DSP */
            {
                tm_bcopy((ttConstVoidPtr)(packetPtr->pktLinkDataPtr),
                         (ttVoidPtr)bufferPtr,
                         (unsigned)tm_packed_byte_count(
                                                linkCopyCount));
                bufferPtr += tm_packed_byte_count(linkCopyCount);
            }
/* 
 * For Datagram protocols, Keep the datagram boundary 
 */
            copyCount -= linkCopyCount;
            packetPtr = (ttPacketPtr)packetPtr->pktLinkNextPtr;
        } while (    (packetPtr != TM_PACKET_NULL_PTR) 
                  && (copyCount != (ttPktLen)0) );
/* 
 * Since we copied the data area, it if the user is not peeking, unqueue
 * and free the packet
 */
        if ((flags & MSG_PEEK) == 0)
        {
/* Update receive queue */
            socketEntryPtr->socReceiveQueueNextPtr =
                                           recvdPacketPtr->pktChainNextPtr;
/* Update receive queue size */
            socketEntryPtr->socRecvQueueBytes -= (tt32Bit)recvdPacketLength;
            socketEntryPtr->socRecvQueueDgrams--;
/* Free copied packet */
            tfFreePacket(recvdPacketPtr, TM_SOCKET_LOCKED);
        }

recvfromFinish:
/*
 * If errorCode not set, will return recvdPacketLength, otherwise it will set
 * the errorCode in the user error array, and return TM_SOCKET_ERROR instead
 */
        retCode = tfSocketReturn(socketEntryPtr, socketDescriptor,
                                 errorCode, (int)recvdPacketLength);
    }
    return retCode;
}
