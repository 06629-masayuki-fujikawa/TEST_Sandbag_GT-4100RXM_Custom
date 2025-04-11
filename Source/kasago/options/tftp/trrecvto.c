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
 * Description: BSD Sockets Interface (tfRecvFromTo)
 *
 * Filename: trrecvto.c
 * Author: Odile
 * Date Created: 12/14/00
 * $Source: source/sockapi/trrecvto.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:07GMT+09:00 $
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
 * tfRecvFromTo function description
 * Take some data from the receive queue and give it to the user.
 * Same as recvfrom, but takes 1 extra parameter to let the user get
 * the destination address of the packet.
 * Parameters
 * Parameter        Description
 * socketDescriptor the socket descriptor to use to recv data
 * bufferPtr        the buffer to store the received data
 * bufferLength     the length of the buffer to store the received data
 * flags            MSG_DONTWAIT: Don't wait for data
 *                  0: wait for data to come in
 * fromAddressPtr   where to store the address the data came from
 * addressLengthPtr length of the area pointed to by fromAddressPtr, or
 *                  toAddressPtr
 * toAddressPtr     where to store the address the data was sent to
 * Returns
 * Value            Meaning
 * >=0              Number of bytes actually sent on the socket
 * -1               an error occured, error can be retrieved with
 *                  tfGetSocketError()
 * tfGetSocketError will return:
 * TM_EBADF         The socket descriptor is invalid
 * TM_ENOBUFS       There was insufficient user memory available to complete
 *                  the operation
 * TM_EMSGSIZE      The message was too long
 * TM_EPROTOTYPE    TCP protocol requires usage of recv, not recvfrom
 * TM_EWOULDBLOCK   The socket is marked as non-blocking and
 *                  the recv operation would block
 */
int tfRecvFromTo( int                     socketDescriptor,
                  char            TM_FAR *bufferPtr,
                  int                     bufferLength,
                  int                     flags,
                  struct sockaddr TM_FAR *fromAddressPtr,
                  int             TM_FAR *addressLengthPtr,
                  struct sockaddr TM_FAR *toAddressPtr )
{
    ttSocketEntryPtr   socketEntryPtr;
    ttPacketPtr        packetPtr;
    ttPacketPtr        recvdPacketPtr;
    ttPktLen           copyCount;
    ttPktLen           linkCopyCount;
    ttPktLen           recvdPacketLength;
    int                errorCode;
    ttSockAddrPtrUnion fromTempAddr;
    ttSockAddrPtrUnion toTempAddr;
    int                addressLength;
#ifdef TM_USE_BSD_DOMAIN
    int                af;
#endif /* TM_USE_BSD_DOMAIN */

    errorCode = TM_ENOERROR;
    recvdPacketLength = (ttPktLen)0;
    fromTempAddr.sockPtr = fromAddressPtr; 
    toTempAddr.sockPtr = toAddressPtr; 
/* PARAMETER CHECK */
    if (addressLengthPtr != (int TM_FAR *)0)
    {
        addressLength = *addressLengthPtr;
    }
    else
    {
        addressLength = 0;
    }
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_INET;
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
    if ( socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR )
    {
/* errorCode set by tfSocketCheckAndLock */
        goto recvToFinish;
    }
    if (    (bufferPtr == (char TM_FAR *)0)
/* we allow bufferlength between 0 and 65534 on 16-bit architecture */
         || (bufferLength == -1)
         || (bufferLength == 0)
         || ((flags & ~TM_RECVFROM_FLAGS) != 0)
/* If user requested the to address */
         || (   (toAddressPtr != (struct sockaddr TM_FAR *) 0)
/*  and no room for the address */
#ifdef TM_USE_IPV4
             && (    (socketEntryPtr->socProtocolFamily == AF_INET)
                  && (addressLength < (int)sizeof(struct sockaddr_in)) )
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
             && (    (socketEntryPtr->socProtocolFamily == AF_INET6)
                  && (addressLength < (int)sizeof(struct sockaddr_in6)) )
#endif /* TM_USE_IPV6 */
            )
       )
    {
        errorCode = TM_EINVAL;
        goto recvToFinish;
    }
    
/*Check to see if it is a TCP socket */
    if (socketEntryPtr->socProtocolNumber == IP_PROTOTCP)
    {
        errorCode = TM_EPROTOTYPE;
        goto recvToFinish;
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
            goto recvToFinish;
        }
        if (flags & TM_SOCF_NONBLOCKING)
        {
            errorCode = TM_EWOULDBLOCK;
            goto recvToFinish;
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
            if ((errorCode != TM_ENOERROR) || (packetPtr==TM_PACKET_NULL_PTR))

            {
                if (errorCode == TM_ENOERROR)
                {
                    errorCode = TM_EWOULDBLOCK;
                }
                goto recvToFinish;
            }
#else /* TM_PEND_POST_NEEDED */
            socketEntryPtr->socRecvError = 0;
            goto recvToFinish;
#endif /* TM_PEND_POST_NEEDED */
        }
    }
    if (fromAddressPtr != TM_SOCKADDR_NULL_PTR)
    {
/* Save the from address */
#ifdef TM_USE_IPV6
        if (socketEntryPtr->socProtocolFamily == AF_INET6)
        {
            *addressLengthPtr = sizeof(struct sockaddr_in6);
            fromTempAddr.sockIn6Ptr->sin6_len =
                sizeof(struct sockaddr_in6);
            fromTempAddr.sockIn6Ptr->sin6_family = AF_INET6;
            fromTempAddr.sockIn6Ptr->sin6_port = (unsigned short)
                (packetPtr->pktSharedDataPtr->dataPeerPortNumber);
            fromTempAddr.sockIn6Ptr->sin6_scope_id = (u_long) 0;

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
                    &fromTempAddr.sockIn6Ptr->sin6_addr );

#ifdef TM_4_USE_SCOPE_ID
                if (IN4_IS_ADDR_LINKLOCAL(packetPtr->pktSrcIpAddress))
                {
/* link-local scope */
                    fromTempAddr.sockIn6Ptr->sin6_scope_id = (ttUser32Bit) 
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
                    fromTempAddr.sockIn6Ptr->sin6_addr );

                if (tm_6_addr_is_local(
                        &(fromTempAddr.sockIn6Ptr->sin6_addr)))
                {
                    tm_6_dev_unscope_addr(
                        &fromTempAddr.sockIn6Ptr->sin6_addr);
                    
/* set the scope ID from the interface */
                    if ((fromTempAddr.sockIn6Ptr->sin6_addr.s6_addr[1]
                         & (ttUser8Bit) 0xc0) == (ttUser8Bit) 0x80)
                    {
/* link-local scope */
                        fromTempAddr.sockIn6Ptr->sin6_scope_id
                            = (ttUser32Bit) 
                            packetPtr->pktDeviceEntryPtr->devIndex;
                    }
#ifndef TM_6_USE_RFC3879
/* site local address to be depricated, don't set scope id for site local 
   address */
                    else
                    {
/* site-local scope */
                        fromTempAddr.sockIn6Ptr->sin6_scope_id
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
            *addressLengthPtr = sizeof(struct sockaddr_in);
            fromTempAddr.sockInPtr->sin_len = sizeof(struct sockaddr_in);
            fromTempAddr.sockInPtr->sin_family=AF_INET;
            fromTempAddr.sockInPtr->sin_port=(unsigned short)
                (packetPtr->pktSharedDataPtr->
                 dataPeerPortNumber);
            tm_ip_copy(packetPtr->pktSrcIpAddress,
                       fromTempAddr.sockInPtr->sin_addr.s_addr);
#endif /* TM_USE_IPV4 */
        }
    }    
    if (toAddressPtr != TM_SOCKADDR_NULL_PTR)
    {
/* Save the to address */
#ifdef TM_USE_IPV6
        if (socketEntryPtr->socProtocolFamily == AF_INET6)
        {
            *addressLengthPtr = sizeof(struct sockaddr_in6);
            toTempAddr.sockIn6Ptr->sin6_len =
                sizeof(struct sockaddr_in6);
            toTempAddr.sockIn6Ptr->sin6_family = AF_INET6;
            toTempAddr.sockIn6Ptr->sin6_port = (unsigned short)
                (packetPtr->pktSharedDataPtr->dataLocalPortNumber);
            tm_assert(tfRecvFromTo,
                      (((unsigned short) (socketEntryPtr->socOurLayer4Port)
                        == TM_WILD_PORT)
                       || ((unsigned short) (socketEntryPtr->socOurLayer4Port)
                           == toTempAddr.sockIn6Ptr->sin6_port)));
            toTempAddr.sockIn6Ptr->sin6_scope_id = (u_long) 0;

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
                    packetPtr->pktDestIpAddress,
                    &toTempAddr.sockIn6Ptr->sin6_addr );

#ifdef TM_4_USE_SCOPE_ID
                if (IN4_IS_ADDR_LINKLOCAL(packetPtr->pktSrcIpAddress))
                {
/* link-local scope */
                    toTempAddr.sockIn6Ptr->sin6_scope_id = (ttUser32Bit) 
                        packetPtr->pktDeviceEntryPtr->devIndex;
                }
#endif /* TM_4_USE_SCOPE_ID */
            }
            else
#endif /* TM_USE_IPV4 */
            {
                tm_6_ip_copy_structs(
                    tm_6_ip_hdr_in6_addr(
                        packetPtr->pkt6RxIphPtr->iph6DestAddr),
                    toTempAddr.sockIn6Ptr->sin6_addr );

                if (tm_6_addr_is_local(
                        &(toTempAddr.sockIn6Ptr->sin6_addr)))
                {
                    tm_6_dev_unscope_addr(
                        &toTempAddr.sockIn6Ptr->sin6_addr);
                    
/* set the scope ID from the interface */
                    if ((toTempAddr.sockIn6Ptr->sin6_addr.s6_addr[1]
                         & (ttUser8Bit) 0xc0) == (ttUser8Bit) 0x80)
                    {
/* link-local scope */
                        toTempAddr.sockIn6Ptr->sin6_scope_id
                            = (ttUser32Bit) 
                            packetPtr->pktDeviceEntryPtr->devIndex;
                    }
#ifndef TM_6_USE_RFC3879 
/* site local address to be depricated */
                    else
                    {
/* site-local scope */
                        toTempAddr.sockIn6Ptr->sin6_scope_id
                            = (ttUser32Bit) 
                            packetPtr->pktDeviceEntryPtr->dev6SiteId;
                    }
#endif /* TM_6_USE_RFC3879 */

                    tm_6_dev_unscope_addr(&toTempAddr.sockIn6Ptr->sin6_addr);
                }
            }
        }
        else
#endif /* TM_USE_IPV6 */
        {
#ifdef TM_USE_IPV4
            *addressLengthPtr = sizeof(struct sockaddr_in);
            toTempAddr.sockInPtr->sin_len = sizeof(struct sockaddr_in);
            toTempAddr.sockInPtr->sin_family=AF_INET;
/* Destination port */
            toTempAddr.sockInPtr->sin_port=(unsigned short)
                (packetPtr->pktSharedDataPtr->dataLocalPortNumber);
            tm_assert(tfRecvFromTo,
                      (((unsigned short) (socketEntryPtr->socOurLayer4Port)
                        == TM_WILD_PORT)
                       || ((unsigned short) (socketEntryPtr->socOurLayer4Port)
                           == toTempAddr.sockInPtr->sin_port)));
/* Destination IP address extracted from the IP header */
            tm_ip_copy(packetPtr->pktDestIpAddress,
                       toTempAddr.sockInPtr->sin_addr.s_addr);
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
            tm_bcopy_to_unpacked( (ttConstVoidPtr)(packetPtr->pktLinkDataPtr),
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
        packetPtr = (ttPacketPtr)packetPtr->pktLinkNextPtr;
        copyCount -= linkCopyCount;
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

recvToFinish:
/*
 * If errorCode not set, will return recvdPacketLength, otherwise it will set
 * the errorCode in the user error array, and return TM_SOCKET_ERROR instead
 */
    return tfSocketReturn(socketEntryPtr, socketDescriptor,
                          errorCode, (int)recvdPacketLength);
}
