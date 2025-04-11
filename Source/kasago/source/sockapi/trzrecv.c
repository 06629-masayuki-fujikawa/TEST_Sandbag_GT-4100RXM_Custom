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
 * Description: BSD Sockets Interface (tfZeroCopyRecv)
 *
 * Filename: trzrecv.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trzrecv.c $
 *
 * Modification History
 * $Revision: 6.0.2.4 $
 * $Date: 2012/07/17 05:34:50JST $
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
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
#include <trssl.h>
#endif /* (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER)) */

/*
 * Take some data from the receive queue and give it to the user 
 * Without a copy
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
int tfZeroCopyRecv(int                   socketDescriptor, 
                   ttUserMessage TM_FAR *bufferHandlePtr, 
                   char TM_FAR * TM_FAR *dataPtrPtr,
                   int                   maxDataLength,
                   int                   flags)
{
    ttSocketEntryPtr socketEntryPtr;
#ifdef TM_USE_TCP
    ttTcpVectPtr     tcpVectPtr;
#endif /* TM_USE_TCP */
    ttPacketPtr      packetPtr;
    ttPacketPtr      retPacketPtr;
    ttPktLen         copyCount;
    ttPktLen         datagramLength;
    int              errorCode;
#ifdef TM_USE_TCP
    tt8Bit           protocolNumber;
#endif /* TM_USE_TCP */
#ifdef TM_DSP
    ttPktLen         byteCount;
#endif /* TM_DSP */
#ifdef TM_DSP
    ttPktLen         bcopyCount;
#endif /* TM_DSP */
#ifdef TM_USE_BSD_DOMAIN
    int              af;
#endif /* TM_USE_BSD_DOMAIN */
#if (defined(TM_DSP) && defined(TM_USE_TCP))
    unsigned int     offset;
#endif /* TM_DSP && TM_USE_TCP */

    
    errorCode = TM_ENOERROR;
    copyCount = (ttPktLen)0;
    retPacketPtr = TM_PACKET_NULL_PTR;
#ifdef TM_USE_TCP
    protocolNumber = (tt8Bit)0;
#endif /* TM_USE_TCP */
/* PARAMETER CHECK */
/* Map from socket descriptor to locked socket entry pointer */
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckValidLock(  socketDescriptor
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
        goto zeroCopyRecvFinish;
    }
#ifdef TM_USE_TCP
    tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr);
#endif /* TM_USE_TCP */
    if (    (bufferHandlePtr == (ttUserMessage TM_FAR *)0)
         || (dataPtrPtr == (char TM_FAR * TM_FAR *)0)
/* we allow bufferlength between 0 and 65534 on 16-bit architecture */
         || (maxDataLength == -1)
         || (maxDataLength == 0)
         || ((flags & ~TM_ZEROCOPYRECV_FLAGS) != 0) )
    {
        errorCode = TM_EINVAL;
        goto zeroCopyRecvFinish;
    }
#ifdef TM_USE_TCP
    protocolNumber = socketEntryPtr->socProtocolNumber;
#endif /* TM_USE_TCP */
/* Check to see if the socket is connected */
    if (    (!(socketEntryPtr->socFlags & TM_SOCF_CONNECTED))
#ifdef TM_USE_TCP
         && (protocolNumber != (tt8Bit)IP_PROTOTCP) 
#endif /* TM_USE_TCP */
       )
    {
        errorCode = TM_ENOTCONN;
        goto zeroCopyRecvFinish;
    }
/* If user did not set any wait/dontwait flag */
    if ((flags & TM_SOCF_MSG_WAIT_FLAGS) ==  0)
    {
/* use the socket default */
        flags |= (socketEntryPtr->socFlags & TM_SOCF_NONBLOCKING);
    }
#if (defined(TM_USE_SSL_CLIENT) || (defined(TM_USE_SSL_SERVER)))
#ifdef TM_USE_TCP
    if (protocolNumber == (tt8Bit)IP_PROTOTCP)
    {
        if (    (tm_16bit_all_bits_set(tcpVectPtr->tcpsSslFlags,
                                        TM_SSLF_RECV_ENABLED 
                                      | TM_SSLF_MORE_RECORDS) )
             || (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                        TM_SSLF_HANDSHK_PROCESS) )
           )
        {
/*
 * We have at least one full SSL record ready for decryption, or we need to
 * process a handshake message.
 */
            (void)tfSslIncomingRecord(tcpVectPtr->tcpsSslConnStatePtr,
                                      TM_SSL_RECV_API);
        }
    }
#endif /* TM_USE_TCP */
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */
    packetPtr = socketEntryPtr->socReceiveQueueNextPtr;
/* Check for Data */
#ifdef TM_PEND_POST_NEEDED
    while (packetPtr == TM_PACKET_NULL_PTR)
#else /* !TM_PEND_POST_NEEDED */
    if (packetPtr == TM_PACKET_NULL_PTR)
#endif /* !TM_PEND_POST_NEEDED */
    {
        errorCode = socketEntryPtr->socRecvError;
        if (errorCode != TM_ENOERROR)
        {
            socketEntryPtr->socRecvError = 0;
            goto zeroCopyRecvError;
        }
#ifdef TM_USE_TCP
        if (protocolNumber == IP_PROTOTCP)
        {
/*
 * Check whether there is any reason to wait for more data. OK to make
 * a call to TCP state machine (instead of in-lining check on TCP state)
 * since there is no data in the receive queue anyway.
 */
            errorCode = tfTcpRecv(tcpVectPtr);
            if (errorCode != TM_ENOERROR)
            {
                goto zeroCopyRecvError;
            }
        }
#endif /* TM_USE_TCP */
#ifdef TM_PEND_POST_NEEDED
        if (!(flags & TM_SOCF_NONBLOCKING))
        {
/* blocking mode */
            errorCode = tm_pend(&socketEntryPtr->socRecvPendEntry,
                                &socketEntryPtr->socLockEntry);
#if (defined(TM_USE_SSL_CLIENT) || (defined(TM_USE_SSL_SERVER))) 
#ifdef TM_USE_TCP
            if (protocolNumber == (tt8Bit)IP_PROTOTCP)
            {
                if (    (tm_16bit_all_bits_set(tcpVectPtr->tcpsSslFlags,
                                                TM_SSLF_RECV_ENABLED 
                                              | TM_SSLF_MORE_RECORDS) )
                     || (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                                TM_SSLF_HANDSHK_PROCESS) )
                   )
                {
/*
 * We have at least one full SSL record ready for decryption, or we need to
 * process a handshake message.
 */
                    (void)tfSslIncomingRecord(tcpVectPtr->tcpsSslConnStatePtr,
                                              TM_SSL_RECV_API);
                }
            }
#endif /* TM_USE_TCP */
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */
            packetPtr = socketEntryPtr->socReceiveQueueNextPtr;
        }
        else
#endif /* TM_PEND_POST_NEEDED */
        {
/* non blocking mode */
            errorCode = TM_EWOULDBLOCK;
            goto zeroCopyRecvFinish;
        }
    }
#ifdef TM_USE_TCP
/* TCP socket */
    if (protocolNumber == IP_PROTOTCP)
    {
        if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_PACKET)
            && (packetPtr->pktLinkDataLength > (ttPktLen)maxDataLength))
        {
/* bug ID 105: if TM_TCP_PACKET is set, check if the specified buffer size is
   smaller than the received TCP segment, in which case we return the error
   TM_EMSGSIZE. */
            errorCode = TM_EMSGSIZE;
            goto zeroCopyRecvFinish;
        }

        if (    (socketEntryPtr->socOobMark != 0)
             && ((flags & MSG_PEEK) == 0)
             && ((tt32Bit)maxDataLength > socketEntryPtr->socOobMark) )
        {
/* stop reading at out of band mark, in case user cares about finding it */
            maxDataLength = (int)(socketEntryPtr->socOobMark);
        }
/* duplicate or hand over the data */
        if (packetPtr->pktLinkDataLength > (ttPktLen)maxDataLength)
        {
/* If the user is not peeking */
            if ((flags & MSG_PEEK) == 0)
            {
/* 
 * There is more data in this packet than we have room for
 * So just duplicate packet  and don't free up update the packet pointer
 */
                retPacketPtr = tfDuplicateSharedBuffer(packetPtr);
                if (retPacketPtr == TM_PACKET_NULL_PTR)
                {
                    errorCode = TM_ENOBUFS;
                    goto zeroCopyRecvFinish;
                }
                else
                {
/*
 * Update receive queue internal pointer to get passed the user received
 * data
 */
#ifdef TM_DSP
                    offset = (unsigned int)(  maxDataLength
                                            + packetPtr->pktLinkDataByteOffset);
                    packetPtr->pktLinkDataPtr += tm_packed_byte_len(offset);
/* keep track of the 8-bit byte index (into the word) of the new start of
   data in this buffer */
                    packetPtr->pktLinkDataByteOffset =
                                        (offset) % TM_DSP_BYTES_PER_WORD;
#else /* !TM_DSP */
                    packetPtr->pktLinkDataPtr += (ttPktLen)maxDataLength;
#endif /* !TM_DSP */
                    packetPtr->pktLinkDataLength -= (ttPktLen)maxDataLength;
                    copyCount = (ttPktLen)maxDataLength;
                }
            }
/* If the user is peeking. */
            else
            {
/* 
 * There is more data in this packet than the user wants. Let him have
 * a peek at it, up to his maximum data length.
 */
                retPacketPtr = packetPtr;
                copyCount = (ttPktLen)maxDataLength;
            }
        }
/* 
 * If user has more than enough room for our packet:
 */
        else
        {
            retPacketPtr = packetPtr;
/* If the user is not peeking */
            if ((flags & MSG_PEEK) == 0)
            {
/* 
 * Remove the packet from the receive queue, user owns the packet.
 * For TCP, since there is no boundary, data is queued using
 * pktLinkNextPtr.
 */
                socketEntryPtr->socReceiveQueueNextPtr = (ttPacketPtr)
                                                   packetPtr->pktLinkNextPtr;
/* Need to set pktLinkNextPtr to null for tfFreeZeroCopyBuffer() */
                packetPtr->pktLinkNextPtr = (ttVoidPtr)0;
            }
            copyCount = retPacketPtr->pktLinkDataLength;
        }
/* If the user is not peeking */
        if ((flags & MSG_PEEK) == 0)
        {
/* User owns the packet and is allowed to free it */
            retPacketPtr->pktUserFlags = TM_PKTF_USER_OWNS;
/* zero-copy send expects pktLinkExtraCount to be 0 */
            retPacketPtr->pktLinkExtraCount = 0;
/*
 * Let TCP know that window is larger by copyCount. Update receive queue size.
 * Update out of band mark
 */
            if (socketEntryPtr->socReceiveQueueNextPtr == (ttPacketPtr)0)
            {
/* Try and send a window update if permitted by TCP algorithm. */
                flags = 0;
            }
            else
            {
#define TM_TCP_NO_WINDOW_UPDATE 1
/* Do not send any window update to avoid sending too many. */
                flags = TM_TCP_NO_WINDOW_UPDATE;
            }
            tfTcpRecvCmplt(tcpVectPtr, copyCount, flags);
        }
    }
    else
#endif /* TM_USE_TCP */
    {
/* non-TCP */        
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
                goto zeroCopyRecvFinish;
            }
            else
            {
/* If the user is not peeking */
                if ((flags & MSG_PEEK) == 0)
                {
/* Remove the fragmented piece from the receive queue */
                    socketEntryPtr->socReceiveQueueNextPtr=
                                               packetPtr->pktChainNextPtr;
/* Update receive queue size */
                    socketEntryPtr->socRecvQueueBytes -=
                                                     (tt32Bit)datagramLength;
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
/* Remove the packet from the receive queue */
                socketEntryPtr->socReceiveQueueNextPtr=
                                               packetPtr->pktChainNextPtr;
/* Update the receive queue size */
                socketEntryPtr->socRecvQueueBytes -= (tt32Bit)datagramLength;
                socketEntryPtr->socRecvQueueDgrams--;
/* User owns the packet and is allowed to free it */
                retPacketPtr->pktUserFlags = TM_PKTF_USER_OWNS;
            }
        }
    }
/*
 * Reset packet flags before returning the buffer, in case the user
 * reuses the buffer in a zero copy send.
 */
    tm_assert(tfZeroCopyRecv, retPacketPtr != TM_PACKET_NULL_PTR);
    retPacketPtr->pktFlags = TM_16BIT_ZERO;

zeroCopyRecvError:
    if (errorCode == TM_ESHUTDOWN)
    {
/* Return 0, if we have reached end of file */
        if ( tm_16bit_one_bit_set(socketEntryPtr->socFlags, TM_SOCF_REOF) )
        {
            errorCode = TM_SOCF_REOF;
        }
    }

zeroCopyRecvFinish:
    if (    (errorCode != TM_ENOERROR) /* Error or EOF */
         && (    (errorCode != TM_EMSGSIZE)
#ifdef TM_USE_TCP
              || (protocolNumber == (tt8Bit)IP_PROTOTCP)
#endif /* TM_USE_TCP */
            )
       )
    {
        if (errorCode == TM_SOCF_REOF)
        {
/* Do not set error, so zero copyCount will be returned by tfSocketReturn() */
            errorCode = TM_ENOERROR;
        }
        if ( bufferHandlePtr != (ttUserMessage TM_FAR *)0)
        {
            *bufferHandlePtr = TM_PACKET_NULL_PTR;
        }
    }
    else /* no error, or UDP EMSGSIZE */
    {
#ifdef TM_DSP
        if (socketEntryPtr->socOptions & SO_UNPACKEDDATA)
        {
            *bufferHandlePtr = tfGetZeroCopyBuffer( tm_byte_count(copyCount), 
                                                    dataPtrPtr);

            if (*bufferHandlePtr == (ttUserMessage) 0)
            {
                errorCode = TM_ENOBUFS;                
            }
            else
            {
                packetPtr = retPacketPtr;
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
                        (int) (packetPtr->pktLinkDataByteOffset),
                        *dataPtrPtr,
                        bcopyCount );
                    packetPtr = (ttPacketPtr)packetPtr->pktLinkNextPtr;
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
