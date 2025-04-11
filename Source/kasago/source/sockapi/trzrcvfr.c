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
 * Description: BSD Sockets Interface (tfZeroCopyRecvFrom)
 *
 * Filename: trzrcvfr.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trzrcvfr.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2012/07/17 05:34:05JST $
 * $Author: lfox $
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
/*
 * Question  What exactly is zero copy?  How is zero copy different from the 
 *  "regular" way? Does the use of zero copy functions affect how my device 
 *  driver is written? Is the zero copy facility proprietary to the Elmic stack?
 * 
 * Answer  Zero copy means that your application uses one of our buffers, so 
 *  you avoid the Elmic stack doing an extra copy from your application buffer 
 *  to our buffer in the transmit-path, or vice versa in the receive-path. In 
 *  the receive-path, nothing needs to change in your device driver. Basically, 
 *  in the receive-path you still pass a received packet to the stack in your 
 *  driverReceive function the same way as you currently do, only once the stack 
 *  queues the data on a socket to be received, calling tfZeroCopyRecv/
 *  tfZeroCopyRecvFrom on that socket avoids the extra copy from the socket 
 *  receive queue buffer to your application buffer, since instead we return 
 *  to you a pointer to our buffer (bufferHandlePtr) and a pointer to the packet 
 *  data in that buffer (dataPtrPtr) as well as the length of the received data. 
 *  Next, you need to give that buffer back to the Treck stack by calling 
 *  tfFreeZeroCopyBuffer. In the transmit-path, you need to get one of our 
 *  buffers to put your application data into before you call tfZeroCopySend/
 *  tfZeroCopySendTo, and you do that by calling tfGetZeroCopyBuffer. Once you 
 *  have made the call to tfZeroCopySend/tfZeroCopySendTo, that zero copy buffer 
 *  is no longer yours, since you have given it back to the Elmic stack. 
 * 
 *  No changes are required to your device driver to use zero copy functions. 
 * 
 * The zero copy functions are Elmic proprietary extensions to the BSD sockets 
 * API. 
 */
int tfZeroCopyRecvFrom(int                     socketDescriptor, 
                       ttUserMessage   TM_FAR *bufferHandlePtr, 
                       char   TM_FAR * TM_FAR *dataPtrPtr,
                       int                     maxDataLength,
                       int                     flags, 
                       struct sockaddr TM_FAR *fromAddressPtr,
                       int             TM_FAR *fromAddressLengthPtr)
{
    ttSocketEntryPtr    socketEntryPtr;
    ttPacketPtr         packetPtr;
    ttPacketPtr         retPacketPtr;
    ttPktLen            copyCount;
    ttPktLen            datagramLength;
    int                 errorCode;
    ttSockAddrPtrUnion  tempSockAddr;
    int                 addressLength;
#ifdef TM_DSP
    ttPktLen            byteCount;
#endif /* TM_DSP */
#ifdef TM_DSP
    ttCharPtr           outDataPtr;
    ttPktLen            bcopyCount;
#endif /* TM_DSP */
#ifdef TM_USE_BSD_DOMAIN
    int                 af;
#endif /* TM_USE_BSD_DOMAIN */

    tempSockAddr.sockPtr = fromAddressPtr;
    errorCode = TM_ENOERROR;
    copyCount = (ttPktLen)0;
    retPacketPtr = TM_PACKET_NULL_PTR;
/* PARAMETER CHECK */

    if (fromAddressLengthPtr != (int TM_FAR *)0)
    {
        addressLength = *fromAddressLengthPtr;
    }
    else
    {
        addressLength = 0;
    }

/*
 * Map from socket descriptor to a locked socket entry pointer and verify
 * that the address length is at least what we expect for this address type.
 */    
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
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
        goto zeroCopyRecvFromFinish;
    }
    if (    (bufferHandlePtr == (ttUserMessage TM_FAR *)0)
         || (dataPtrPtr == (char TM_FAR * TM_FAR *)0)
/* we allow bufferlength between 0 and 65534 on 16-bit architecture */
         || (maxDataLength == -1)
         || (maxDataLength == 0)
         || ((flags & ~TM_ZEROCOPYRECVFROM_FLAGS) != 0) )
    {
        errorCode = TM_EINVAL;
        goto zeroCopyRecvFromFinish;
    }
    
/*Check to see if it is a TCP socket */
    if (socketEntryPtr->socProtocolNumber == IP_PROTOTCP)
    {
        errorCode = TM_EPROTOTYPE;
        goto zeroCopyRecvFromFinish;
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
            goto zeroCopyRecvFromFinish;
        }
        if (flags & TM_SOCF_NONBLOCKING)
        {
            errorCode = TM_EWOULDBLOCK;
            goto zeroCopyRecvFromFinish;
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
                goto zeroCopyRecvFromFinish;
            }
#else /* TM_PEND_POST_NEEDED */
            socketEntryPtr->socRecvError = 0;
            goto zeroCopyRecvFromFinish;
#endif /* TM_PEND_POST_NEEDED */
        }
    }
    datagramLength = packetPtr->pktChainDataLength;
    if (datagramLength > (ttPktLen)maxDataLength)
    {
        errorCode = TM_EMSGSIZE;
        if ((flags & MSG_PEEK) == 0)
/* User not peeking, trim excess data */
        {
            tfPacketTailTrim(packetPtr, (ttPktLen)maxDataLength,
                             TM_SOCKET_LOCKED);
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
                             tempSockAddr.sockIn6Ptr->sin6_addr.s6LAddr,1)
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
            (tempSockAddr.sockInPtr)->sin_len = 
                sizeof(struct sockaddr_in);
            (tempSockAddr.sockInPtr)->sin_family=AF_INET;
            (tempSockAddr.sockInPtr)->sin_port=
                (unsigned short)(packetPtr->pktSharedDataPtr->
                                 dataPeerPortNumber);
            tm_ip_copy(packetPtr->pktSrcIpAddress,
                       (tempSockAddr.sockInPtr)->sin_addr.s_addr);
#endif /* TM_USE_IPV4 */
        }
    }    
    copyCount = packetPtr->pktChainDataLength;
    if (copyCount > (ttPktLen)maxDataLength)
/* If user is peeking, we have not trimmed the excess data */
    {
        copyCount = (ttPktLen)maxDataLength;
    }
/* Check for IP Fragmented data */
    if (    (    packetPtr->pktLinkDataLength 
              != copyCount )
         && (    (flags & MSG_SCATTERED) == 0 )
#ifdef TM_DSP
/* Make sure we do not copy twice for DSP */
         && ( !(socketEntryPtr->socOptions & SO_UNPACKEDDATA) )
#endif /* TM_DSP */
       )
    {
/*
 * Because the incoming data is fragmented and we need to deliver the whole
 * datagram back to the user (to preserve datagram boundaries), because
 * the user does not want scattered data, and we are not going to copy the
 * data anyways at the end of this routine for DSP, we are forced to copy to 
 * make the data contiguous. 
 */
        retPacketPtr = tfGetSharedBuffer(TM_MAX_SEND_HEADERS_SIZE,
                                         copyCount,
                                         TM_16BIT_ZERO);
        if (retPacketPtr == TM_PACKET_NULL_PTR)
        {
            errorCode = TM_ENOBUFS;
            goto zeroCopyRecvFromFinish;
        }
        else
        {
            if ((flags & MSG_PEEK) == 0)
/* If the user is not peeking */
            {
/* Remove the packet from the receive queue, user owns the packet */
                socketEntryPtr->socReceiveQueueNextPtr=
                                               packetPtr->pktChainNextPtr;
/* Update receive queue size */
                socketEntryPtr->socRecvQueueBytes -= (tt32Bit)datagramLength;
                socketEntryPtr->socRecvQueueDgrams--;
/* User owns the packet and is allowed to free it */
                retPacketPtr->pktUserFlags = TM_PKTF_USER_OWNS;
                tfCopyPacket(packetPtr, retPacketPtr);
/* Since we copied the whole data area, and we are not peeking, free it */
                tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
            }
            else
            {
                tfCopyPacket(packetPtr, retPacketPtr);
            }
        }
    }
    else
    {
/*
 * If the packet is NOT IP fragmented, or the user wants scattered data,
 * or we know we are going to copy the packet anyways at the end of 
 * this routine to unpack DSP data, then we just hand the scattered packet 
 */
        retPacketPtr = packetPtr;
        if ((flags & MSG_PEEK) == 0)
/* If the user is not peeking */
        {
/* Remove the packet from the receive queue, user owns the packet */
            socketEntryPtr->socReceiveQueueNextPtr =
                                               packetPtr->pktChainNextPtr;
/* Update receive queue size */
            socketEntryPtr->socRecvQueueBytes -=
                                             (tt32Bit)datagramLength;
            socketEntryPtr->socRecvQueueDgrams--;
/* User owns the packet and is allowed to free it */
            retPacketPtr->pktUserFlags = TM_PKTF_USER_OWNS;
        }
    }
/*
 * Reset packet flags before returning the buffer, in case the user
 * reuses the buffer in a zero copy send.
 */
    tm_assert(tfZeroCopyRecvFrom, retPacketPtr != TM_PACKET_NULL_PTR);
    retPacketPtr->pktFlags = TM_16BIT_ZERO;

zeroCopyRecvFromFinish:
    if ( (errorCode == TM_ENOERROR) || (errorCode == TM_EMSGSIZE) )
/* no error, or truncated */
    {

#ifdef TM_DSP
        if (socketEntryPtr->socOptions & SO_UNPACKEDDATA)
        {
            *bufferHandlePtr = tfGetZeroCopyBuffer( copyCount, 
                                                    dataPtrPtr);

            if (*bufferHandlePtr == (ttUserMessage) 0)
            {
                errorCode = TM_ENOBUFS;                
            }
            else
            {
                packetPtr = retPacketPtr;
                outDataPtr = *dataPtrPtr;
                byteCount = copyCount;
                do
                {
                    if (byteCount >= packetPtr->pktLinkDataLength)
                    {
                        byteCount -= packetPtr->pktLinkDataLength;
                        bcopyCount = packetPtr->pktLinkDataLength;
                    }
                    else
                    {
                        bcopyCount = byteCount;
                        byteCount = 0;
                    }
                    tm_bcopy_to_unpacked(
                                      packetPtr->pktLinkDataPtr,
                                      (int)(packetPtr->pktLinkDataByteOffset),
                                      outDataPtr,
                                      bcopyCount );
                    packetPtr = (ttPacketPtr)packetPtr->pktLinkNextPtr;
/* Update unpacked pointer in output buffer */
                    outDataPtr += bcopyCount;
                }
                while (    (packetPtr != (ttPacketPtr)0) 
                        && (byteCount != (ttPktLen)0)
                    );
            }
            if ((flags & MSG_PEEK) == 0)
            {
                tfFreePacket(retPacketPtr, TM_SOCKET_LOCKED);
            }
        }
        else
#endif /* TM_DSP */
        {
#ifdef TM_LINT
LINT_NULL_PTR_BEGIN(bufferHandlePtr)
LINT_NULL_PTR_BEGIN(retPacketPtr)
LINT_NULL_PTR_BEGIN(dataPtrPtr)
#endif /* TM_LINT */
/* Return buffer handle to the user */
        *bufferHandlePtr = (ttUserMessage)retPacketPtr;
/* Return data pointer to the user */
        *dataPtrPtr = (char TM_FAR *)retPacketPtr->pktLinkDataPtr;
#ifdef TM_LINT
LINT_NULL_PTR_END(bufferHandlePtr)
LINT_NULL_PTR_END(retPacketPtr)
LINT_NULL_PTR_END(dataPtrPtr)
#endif /* TM_LINT */
        }
    }
/*
 * Return read bytes to the user:
 * If errorCode not set, will return copyCount, otherwise it will set
 * the errorCode in the user error array, and return TM_SOCKET_ERROR instead
 */
    return tfSocketReturn(socketEntryPtr, socketDescriptor,
                          errorCode, (int)copyCount);
}
