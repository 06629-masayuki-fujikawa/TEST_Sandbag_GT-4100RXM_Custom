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
 * Description: BSD Sockets Interface (tfZeroCopySend)
 *
 * Filename: trzsend.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trzsend.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2010/06/10 23:44:22JST $
 * $Author: pcarney $
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
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
#include <trssl.h>
#endif /* (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER)) */

/* 
 * Send some data on the network (with Zero Copy)
 * For datagram protocol make sure there is enough room
 * for the whole datagram, for stream protocol send a piece
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
int tfZeroCopySend (int             socketDescriptor,
                    ttUserMessage   bufferHandle,
                    int             bufferLength,
                    int             flags)
{
    ttSocketEntryPtr socketEntryPtr;
#ifdef TM_USE_TCP
    ttTcpVectPtr     tcpVectPtr;
#endif /* TM_USE_TCP */
    ttPacketPtr      packetPtr;
    ttPacketPtr      newPacketPtr;
    tt32Bit          maxQueueSize;
#ifdef TM_USE_TCP
    tt32Bit          amountToCopy;
    tt32Bit          bytesLeftToSend;
#endif /* TM_USE_TCP */
    tt32Bit          totalInFlightBytes;
    ttPktLen         copyBytes;
#ifdef TM_USE_TCP
    ttPktLen         totalBytes;
#endif /* TM_USE_TCP */
    int              errorCode;
    int              retCode;
    int              linkExtraCount;
#ifdef TM_USE_TCP
    tt8Bit           protocol;
    tt8Bit           socketProtocol;
#endif /* TM_USE_TCP */
    tt8Bit           needFreePacket;
    tt8Bit           msgFitsInSendQ;
#if (defined(TM_DSP) && defined(TM_USE_TCP))
    ttPacketPtr      copyPacketPtr;
#endif /* TM_DSP && TM_USE_TCP */
    ttPktLen         availHdrLen;
#ifdef TM_USE_BSD_DOMAIN
    int              af;
#endif /* TM_USE_BSD_DOMAIN */
    
    errorCode = TM_ENOERROR;
#ifdef TM_USE_TCP
    socketProtocol = TM_8BIT_ZERO;
#endif /* TM_USE_TCP */
    needFreePacket = TM_8BIT_YES;
    msgFitsInSendQ = TM_8BIT_NO; 
    copyBytes = (ttPktLen)0;
    packetPtr = (ttPacketPtr)bufferHandle;
/* PARAMETER CHECK */
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
/* Map from socket descriptor to locked socket entry pointer */
    socketEntryPtr = tfSocketCheckValidLock(socketDescriptor
#ifdef TM_USE_BSD_DOMAIN
                                            , &af
#endif /* TM_USE_BSD_DOMAIN */
                                           );
    if ( socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR )
    {
#ifdef TM_USE_BSD_DOMAIN
        if (af == TM_BSD_FAMILY_NONINET)
        {
            errorCode = TM_EOPNOTSUPP;
        }
        else
#endif /* TM_USE_BSD_DOMAIN */
        {
            errorCode = TM_EBADF;
        }
        goto zeroCopySendExit;
    }
#ifdef TM_USE_TCP
    tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr);
#endif /* TM_USE_TCP */
    if (    (packetPtr == (ttPacketPtr)0)
         || ( !(   packetPtr->pktUserFlags
                 & TM_PKTF_USER_OWNS ) ) )
    {
        errorCode = TM_EFAULT;
/* Not a zero copy buffer handle */
        needFreePacket = TM_8BIT_ZERO;
        goto zeroCopySendExit;
    }
    linkExtraCount = packetPtr->pktLinkExtraCount;
#ifdef TM_USE_TCP
    socketProtocol = socketEntryPtr->socProtocolNumber;
    protocol = socketProtocol;
#endif /* TM_USE_TCP */
    if (    (bufferLength == -1)
/* we allow bufferlength between 0 and 65534 on 16-bit architecture */
         || (bufferLength == 0)
         || ((flags & ~TM_ZEROCOPYSEND_FLAGS) != 0)
#ifdef TM_USE_TCP
         || ( (linkExtraCount != 0) && (socketProtocol == IPPROTO_TCP) )
#endif /* TM_USE_TCP */
       )
    {
        errorCode = TM_EINVAL;
        goto zeroCopySendExit;
    }
#ifdef TM_DSP
    if (socketEntryPtr->socOptions & SO_UNPACKEDDATA)
    {            
        if (linkExtraCount != 0)
/* Cannot send scattered data if SO_UNPACKEDDATA is set */
        {
            errorCode = TM_EPERM;
            goto zeroCopySendFinish;
        }
    }
#endif /* TM_DSP */
    errorCode = socketEntryPtr->socSendError;
    if (errorCode != TM_ENOERROR)
    {
        socketEntryPtr->socSendError = 0;
        goto zeroCopySendExit;
    }
/* If user did not set any wait/dontwait flag */
    if ((flags & TM_SOCF_MSG_WAIT_FLAGS) ==  0)
    {
/* use the socket default */
        flags |= (socketEntryPtr->socFlags & TM_SOCF_NONBLOCKING);
    }
    maxQueueSize = socketEntryPtr->socMaxSendQueueBytes;
/*
 * Disallow access for a not connected UDP or ICMP socket
 * or for a TCP socket when the user has already issued a write shutdown,
 * or for a TCP socket that has a vector attached to it with state
 * bigger than listen.
 * (i.e allow send with a non blocking connect on a TCP socket, before
 * connect finishes).
 */
#ifdef TM_USE_TCP
    if (protocol == (tt8Bit)IP_PROTOTCP)
    {
        if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_PACKET))
        {
            if (bufferLength > (int)(tcpVectPtr->tcpsEffSndMss))
            {
/* bug ID 105: if TM_TCP_PACKET is set, check if the specified buffer size is
   larger than the effective MSS, in which case we return the error
   TM_EMSGSIZE. */
                errorCode = TM_EMSGSIZE;
                goto zeroCopySendFinish;
            }
/* So that the packet boundaries are respected */
            protocol = (tt8Bit)IP_PROTOTPACKET;
        }

        if ( tm_16bit_one_bit_set( socketEntryPtr->socFlags,
                                   TM_SOCF_NO_MORE_SEND_DATA ) )
        {
/*
 * User has issued a write shutdown, or a network reset occured.
 */
            errorCode = socketEntryPtr->socSendError;
            if (errorCode == TM_ENOERROR)
            {
/* errorCode should be already be set. Just in case. */
                errorCode = TM_ESHUTDOWN;
            }
            goto zeroCopySendFinish;
        }
        else
        {
/* ANVL 5.24 + 5.25: Queue data in SYN-SENT, and SYN-RCVD states */
            if (tcpVectPtr->tcpsState < TM_TCPS_SYN_SENT)
            {
/* If no connection establishment has started */
                errorCode = TM_ENOTCONN;
                goto zeroCopySendFinish;
            }
        }
    }
    else
#endif /* TM_USE_TCP */
    {
/* Non TCP sockets */
        if (!(socketEntryPtr->socFlags & TM_SOCF_CONNECTED))
        {
            errorCode = TM_ENOTCONN;
            goto zeroCopySendFinish;
        }
    }
    totalInFlightBytes =   socketEntryPtr->socSendQueueBytes 
                         + (tt32Bit)bufferLength;
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
#ifdef TM_USE_TCP
    if (socketProtocol == (tt8Bit)IP_PROTOTCP)
    {
        if ( tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                  TM_SSLF_HANDSHK_PROCESS) )
        {
/*
 * We need to process an SSL handshake message.
 */
            (void)tfSslIncomingRecord(tcpVectPtr->tcpsSslConnStatePtr,
                                      TM_SSL_OTHER_API);
        }
        if (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                 TM_SSLF_SEND_ENABLED))
        {
            totalInFlightBytes =   totalInFlightBytes 
                                 + tcpVectPtr->tcpsSslSendQBytes;
        }
    }
#endif /* TM_USE_TCP */
#endif /* (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER)) */

#ifdef TM_USE_TCP
    if (socketProtocol == (tt8Bit)IP_PROTOTCP)
    {
        if (   (    (totalInFlightBytes <= maxQueueSize) )
               || (    (flags & MSG_OOB)
                       && (protocol == (tt8Bit)IP_PROTOTCP)
                       && (totalInFlightBytes <=
                           maxQueueSize + TM_SOC_SEND_OOB_Q_BYTES) ) )
        {
            msgFitsInSendQ = TM_8BIT_YES; 
        }
    }
    else
#endif /* TM_USE_TCP */
    {
/* non-TCP */
        if ((totalInFlightBytes <= maxQueueSize)
            && (socketEntryPtr->socSendQueueDgrams
                < socketEntryPtr->socMaxSendQueueDgrams))
        {
            msgFitsInSendQ = TM_8BIT_YES; 
        }
    }

    if (msgFitsInSendQ == TM_8BIT_YES)
    {
/* Load the sizes of the packet */
        if (    ((flags & MSG_SCATTERED) == 0) 
#ifdef TM_USE_TCP
             || (socketProtocol == (tt8Bit)IP_PROTOTCP )
#endif /* TM_USE_TCP */
           )
        {
            packetPtr->pktChainDataLength = (ttPktLen)bufferLength;
            packetPtr->pktLinkDataLength = (ttPktLen)bufferLength;
        }
        else
        {
#ifdef TM_DSP
            if (!(socketEntryPtr->socOptions & SO_UNPACKEDDATA))
#endif /* TM_DSP */
            {
/* 
 * Scattered non TCP data from the user.  Verify that there is enough space 
 * for the send path to all necessary headers (UDP, IP, link layer, etc).  
 * This area must be contiguous. If not enough space, allocate a new  buffer.
 * Note that in tfZeroCopyUserBufferSend() we set the MSG_SCATTERED flag even
 * though the message is not scattered so that this code prepends room fo
 * the headers.
 */
                availHdrLen = (ttPktLen)
                            (   packetPtr->pktLinkDataPtr
                              - packetPtr->pktSharedDataPtr->dataBufFirstPtr);
                if (tm_byte_count(availHdrLen) < TM_MAX_SEND_HEADERS_SIZE)
                {
                    newPacketPtr = tfGetSharedBuffer(
                        TM_MAX_SEND_HEADERS_SIZE, (ttPktLen) 0, TM_16BIT_ZERO);
                    if (newPacketPtr != (ttPacketPtr) 0)
                    {
                        newPacketPtr->pktChainDataLength = 
                                                        (ttPktLen)bufferLength;
                        newPacketPtr->pktLinkNextPtr = (ttVoidPtr)packetPtr;
                        newPacketPtr->pktLinkExtraCount = linkExtraCount + 1;
                        packetPtr = newPacketPtr;
                    }
                    else
                    {
                        errorCode = TM_ENOBUFS;
                        goto zeroCopySendFinish;
                    }
                }
            }
        }
#ifdef TM_DSP
        if (socketEntryPtr->socOptions & SO_UNPACKEDDATA)
        {            
            newPacketPtr = tfGetSharedBuffer(
                TM_MAX_SEND_HEADERS_SIZE, (ttPktLen) bufferLength, TM_16BIT_ZERO);
            if (newPacketPtr != TM_PACKET_NULL_PTR)
            {
                tm_bcopy_to_packed(packetPtr->pktLinkDataPtr,
                                   newPacketPtr->pktLinkDataPtr,
                                   bufferLength);
                tfFreeZeroCopyBuffer((ttUserMessage)packetPtr);
                packetPtr = newPacketPtr;
            }
            else
            {
                errorCode = TM_ENOBUFS;
                goto zeroCopySendFinish;
            }
        }
#endif /* TM_DSP */
#ifdef TM_USE_TCP
        if (socketProtocol != (tt8Bit)IP_PROTOTCP)
#endif /* TM_USE_TCP */
        {
            packetPtr->pktSharedDataPtr->dataUserDataLength = 
                                                       (ttPktLen)bufferLength;
        }
#ifdef TM_DEV_RECV_OFFLOAD
/* If recv checksum was offloaded, reset offload pointer */
        packetPtr->pktDevOffloadPtr = (ttVoidPtr)0;
#endif /* TM_DEV_RECV_OFFLOAD */
/* Send the data. tfSocketSend thread owns the buffer (original or new) */
        needFreePacket = TM_8BIT_ZERO; /* tfSocketSend owns the buffer */
        errorCode = tfSocketSend(socketEntryPtr,
                                 packetPtr,
                                 (ttPktLen)bufferLength,
                                 flags);
        if (errorCode == TM_ENOERROR)
        {
            copyBytes = (ttPktLen)bufferLength;
        }
        goto zeroCopySendFinish;
    }
    else
    {   
        if (flags & TM_SOCF_NONBLOCKING)
        {
            if ((tt32Bit)bufferLength > socketEntryPtr->socMaxSendQueueBytes)
            {
                errorCode = TM_EMSGSIZE;
                goto zeroCopySendFinish;
            }
            else
            {
/* User still owns the buffer */
                needFreePacket = TM_8BIT_ZERO;
                errorCode = TM_EWOULDBLOCK;
                goto zeroCopySendFinish;
            }
        }
        else
        {
#ifdef TM_USE_TCP
            if (protocol == IP_PROTOTCP)
            {
/* 
 * For TCP we can send just a part of the message if we don't have room
 * for the whole thing
 */
                do
                {
/* Calculate the Space left in the queue */
                    totalInFlightBytes = socketEntryPtr->socSendQueueBytes;
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
                    if (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                             TM_SSLF_SEND_ENABLED))
                    {
                        totalInFlightBytes =   totalInFlightBytes
                                             + tcpVectPtr->tcpsSslSendQBytes;
                    }
#endif /* (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER)) */
                    if (totalInFlightBytes < maxQueueSize)
                    {
                        amountToCopy =   maxQueueSize
                                       - totalInFlightBytes;
                    }
                    else
                    {
                        amountToCopy = (tt32Bit)0;
                    }
/*
 * Do not copy more than bufferlength. The first time through this is
 * not necessary, but this could become necessary if we send more than the
 * window size, and we loop more than once, and we do not pend in a
 * subsequent loop pass, after the peer acknowleges some data.
 * (OL/11-19-99 BUGFIX 444)
 */
                    bytesLeftToSend =   (tt32Bit)bufferLength
                                      - (tt32Bit)copyBytes;
                    if (amountToCopy > bytesLeftToSend)
                    {
                        amountToCopy = bytesLeftToSend;
                    }
#ifdef TM_DSP
                    else if (amountToCopy < bytesLeftToSend)
                    {
/* 
 * If we aren't ready to send the rest of the user's data, then make sure
 * that the number of bytes we do send is word-aligned so that a subsequent
 * send here from the user's buffer starts on a word-aligned boundary (we don't
 * keep track of the byte offset for the user's data here).
 * Word-align the number of bytes to send/copy:
 */
                        amountToCopy &= ~TM_DSP_ROUND_PTR;
                    }
#endif /* TM_DSP */
/* "Silly Queueing Avoidance" of TCP packets */
                    while (    (   amountToCopy
                                 < socketEntryPtr->socLowSendQueueBytes )
                            && (   amountToCopy < bytesLeftToSend ) )

                    {
/*
 * Delay until we have at least 1/4 of send queue free or we have room
 * for all of our remaining data
 */
#ifdef TM_PEND_POST_NEEDED
                        errorCode = tm_pend(
                                    &socketEntryPtr->socSendPendEntry,
                                    &socketEntryPtr->socLockEntry);
                        if (errorCode != TM_ENOERROR)
                        {
/* TCP hard error occured. Let the user know about it */
                            socketEntryPtr->socSendError = 0;
                            goto zeroCopySendFinish;
                        }
/* recompute amount of data left in the send queue after the pend */
                        totalInFlightBytes = socketEntryPtr->socSendQueueBytes;
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
                        if ( tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                                  TM_SSLF_HANDSHK_PROCESS) )
                        {
/*
 * We need to process an SSL handshake message.
 */
                            (void)tfSslIncomingRecord
                                        (tcpVectPtr->tcpsSslConnStatePtr,
                                        TM_SSL_OTHER_API);
                        }
                        if (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                                 TM_SSLF_SEND_ENABLED))
                        {
                            totalInFlightBytes =   totalInFlightBytes
                                                 + tcpVectPtr->tcpsSslSendQBytes;
                        }
#endif /* (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER)) */
                        if (totalInFlightBytes < maxQueueSize)
                        {
                            amountToCopy =   maxQueueSize
                                           - totalInFlightBytes;
                        }
                        else
                        {
                            amountToCopy = (tt32Bit)0;
                        }
/* Do not copy more than bufferlength */
                        bytesLeftToSend = 
                                (tt32Bit)bufferLength - (tt32Bit)copyBytes;
                        if (amountToCopy > bytesLeftToSend)
                        {
                            amountToCopy = bytesLeftToSend;
                        }
#ifdef TM_DSP
                        else if (amountToCopy < bytesLeftToSend)
                        {
/* 
 * If we aren't ready to send the rest of the user's data, then make sure
 * that the number of bytes we do send is word-aligned so that a subsequent
 * send here from the user's buffer starts on a word-aligned boundary (we don't
 * keep track of the byte offset for the user's data here).
 * Word-align the number of bytes to send/copy:
 */
                            amountToCopy &= ~TM_DSP_ROUND_PTR;
                        }
#endif /* TM_DSP */
#else /* TM_PEND_POST_NEEDED */
                        socketEntryPtr->socRecvError = 0;
                        goto zeroCopySendFinish;
#endif /* TM_PEND_POST_NEEDED */
                    }
                    totalBytes = copyBytes + (ttPktLen)amountToCopy;
                    if (totalBytes < (ttPktLen)bufferLength)
                    {
/*
 * Duplicate the packet, since we have not reached the end of the
 * buffer yet.
 */
                        newPacketPtr = tfDuplicateSharedBuffer(packetPtr);
                        if (newPacketPtr == TM_PACKET_NULL_PTR)
                        {
                            errorCode = TM_ENOBUFS;
                            goto zeroCopySendFinish;
                        }
                    }
                    else
                    {
                        newPacketPtr = packetPtr;
/* We did not duplicate the packet. tfSocketSend thread owns the buffer */
                        needFreePacket = TM_8BIT_ZERO;
                    }
/* Load the sizes of the packet */
                    newPacketPtr->pktChainDataLength = (ttPktLen)amountToCopy;
                    newPacketPtr->pktLinkDataLength = (ttPktLen)amountToCopy;
#ifdef TM_DSP
                    if (socketEntryPtr->socOptions & SO_UNPACKEDDATA)
                    {
                        copyPacketPtr = 
                            tfGetSharedBuffer(
                                TM_MAX_SEND_HEADERS_SIZE,
                                (ttPktLen) amountToCopy,
                                TM_16BIT_ZERO);

                        if (copyPacketPtr != TM_PACKET_NULL_PTR)
                        {
                            tm_bcopy_to_packed(
                                       newPacketPtr->pktLinkDataPtr,
                                       copyPacketPtr->pktLinkDataPtr,
                                       amountToCopy);
/* Free either duplicated buffer or original buffer */
                            tfFreeSharedBuffer(newPacketPtr, TM_SOCKET_LOCKED);
                            newPacketPtr = copyPacketPtr;
                        }
                        else
                        {
/* 
 * Free either duplicated buffer or original buffer. (If freeing
 * duplicated buffer, then the original buffer will be freed up at the
 * end of this routine (because needFreePacket != 0))
 */
                            tfFreeSharedBuffer(newPacketPtr, TM_SOCKET_LOCKED);
                            errorCode = TM_ENOBUFS;
                            goto zeroCopySendFinish;
                        }
                    }
#endif /* TM_DSP */
/*
 * Send the data. tfSocketSend() thread owns either the original
 * buffer, or the duplicated buffer.
 */
                    errorCode = tfSocketSend(socketEntryPtr,
                                             newPacketPtr,
                                             (ttPktLen)amountToCopy,
                                             flags);
                    if (errorCode != TM_ENOERROR)
                    {
                        goto zeroCopySendFinish;
                    }
                    if (needFreePacket != TM_8BIT_ZERO)
/*
 * If not sending last packet (i.e sending a duplicate buffer), update
 * data pointer in original buffer.
 */
                    {
                        packetPtr->pktLinkDataPtr +=
                                tm_packed_byte_len((unsigned)amountToCopy);
                    }
                    copyBytes = totalBytes;
                } while (    (copyBytes < (ttPktLen)bufferLength)
                          && (tm_16bit_bits_not_set(
                                        socketEntryPtr->socFlags,
                                        TM_SOCF_NO_MORE_SEND_DATA)) );
            }
            else
#endif /* TM_USE_TCP */
            {
/* non-TCP */
                if (   (tt32Bit)bufferLength
                     > socketEntryPtr->socMaxSendQueueBytes )
                {
                    errorCode = TM_EMSGSIZE;
                    goto zeroCopySendFinish;
                }
#ifdef TM_PEND_POST_NEEDED
/* loop until we have enough room for the entire datagram */
                do
                {
                    errorCode = tm_pend(&socketEntryPtr->socSendPendEntry,
                                        &socketEntryPtr->socLockEntry);
                    if (errorCode != TM_ENOERROR)
                    {
                        socketEntryPtr->socSendError = 0;
                        goto zeroCopySendFinish;
                    }
                } while (((socketEntryPtr->socSendQueueBytes
                           + (tt32Bit)bufferLength)
                          > socketEntryPtr->socMaxSendQueueBytes)
                         || (socketEntryPtr->socSendQueueDgrams
                             >= socketEntryPtr->socMaxSendQueueDgrams));
/* Load the sizes of the packet */
                if (    ((flags & MSG_SCATTERED) == 0) 
#ifdef TM_USE_TCP
                     || (socketProtocol == (tt8Bit)IP_PROTOTCP ) /* needed */
#endif /* TM_USE_TCP */
                   )
                {
                    packetPtr->pktChainDataLength = (ttPktLen)bufferLength;
                    packetPtr->pktLinkDataLength = (ttPktLen)bufferLength;
                }
                else
                {
/* 
 * Scattered non TCP data from the user.  Verify that there is enough space 
 * for the send path to all necessary headers (UDP, IP, link layer, etc).  
 * This area must be contiguous. If not enough space, allocate a new  buffer.
 * Note that in tfZeroCopyUserBufferSend() we set the MSG_SCATTERED flag even
 * though the message is not scattered so that this code prepends room fo
 * the headers.
 */
#ifdef TM_DSP
                    if (!(socketEntryPtr->socOptions & SO_UNPACKEDDATA))
#endif /* TM_DSP */
                    {
                        availHdrLen = (ttPktLen)
                            (   packetPtr->pktLinkDataPtr 
                              - packetPtr->pktSharedDataPtr->dataBufFirstPtr);
                        if (tm_byte_count(availHdrLen)
                                                < TM_MAX_SEND_HEADERS_SIZE)
                        {
                            newPacketPtr = tfGetSharedBuffer(
                                TM_MAX_SEND_HEADERS_SIZE,
                                (ttPktLen) 0,
                                TM_16BIT_ZERO);
                            if (newPacketPtr != (ttPacketPtr) 0)
                            {
                                newPacketPtr->pktChainDataLength = 
                                                      (ttPktLen)bufferLength;
                                newPacketPtr->pktLinkNextPtr =
                                                     ((ttVoidPtr)bufferHandle);
                                newPacketPtr->pktLinkExtraCount =
                                                            linkExtraCount + 1;
                                packetPtr = newPacketPtr;
                            }
                            else
                            {
                                errorCode = TM_ENOBUFS;
                                goto zeroCopySendFinish;
                            }
                        }
                    }
                }
#ifdef TM_DSP 
                if (socketEntryPtr->socOptions & SO_UNPACKEDDATA)
                {
/* 
 * tfSocketSend owns the new buffer .
 */
                    newPacketPtr = tfGetSharedBuffer(
                        TM_MAX_SEND_HEADERS_SIZE,
                        (ttPktLen) bufferLength,
                        TM_16BIT_ZERO);
                    if (newPacketPtr != TM_PACKET_NULL_PTR)
                    {
                        tm_bcopy_to_packed( packetPtr->pktLinkDataPtr,
                                            newPacketPtr->pktLinkDataPtr,
                                            bufferLength);
                        tfFreeZeroCopyBuffer((ttUserMessage)packetPtr);
                        packetPtr = newPacketPtr;
                    }
                    else
                    {
                        errorCode = TM_ENOBUFS;
                        goto zeroCopySendExit;
                    }
                }
#endif /* TM_DSP */
#ifdef TM_USE_TCP
                if (socketProtocol != (tt8Bit)IP_PROTOTCP)
#endif /* TM_USE_TCP */
                {
                    packetPtr->pktSharedDataPtr->dataUserDataLength = 
                                                     (ttPktLen)bufferLength;
                }
/* Send the data. tfSocketSend thread owns the buffer (original or new) */
                needFreePacket = TM_8BIT_ZERO;
                errorCode = tfSocketSend(socketEntryPtr,
                                         packetPtr,
                                         (ttPktLen)bufferLength,
                                         flags);
                copyBytes = (ttPktLen)bufferLength;
#else /* TM_PEND_POST_NEEDED */
                socketEntryPtr->socRecvError = 0;
                goto zeroCopySendFinish;
#endif /* TM_PEND_POST_NEEDED */
            }
        }
    }

zeroCopySendFinish:
#ifdef TM_USE_TCP
    if (    (flags & MSG_EOF)
         && (errorCode == TM_ENOERROR)
         && (socketProtocol == IP_PROTOTCP)
         && ((int)copyBytes == bufferLength) )
    {
/* TCP close */
        errorCode = tfTcpClose(tcpVectPtr);
    }
#endif /* TM_USE_TCP */
zeroCopySendExit:
/*
 * Return sent bytes to the user:
 * If errorCode not set, will return copyBytes, otherwise it will set
 * the errorCode in the user error array, and return TM_SOCKET_ERROR instead
 */
    retCode = tfSocketReturn(socketEntryPtr, socketDescriptor,
                             errorCode, (int)copyBytes);
    if (needFreePacket != TM_8BIT_ZERO)
    {
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    }
    return retCode;
}
