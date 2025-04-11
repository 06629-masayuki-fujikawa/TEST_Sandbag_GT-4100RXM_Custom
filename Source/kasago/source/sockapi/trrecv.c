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
 * Description: BSD Sockets Interface (recv)
 *
 * Filename: trrecv.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trrecv.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:04JST $
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
#if defined(TM_ERROR_CHECKING) && defined(TM_USE_BSD_DOMAIN)
#include <trglobal.h>
#endif /* defined(TM_ERROR_CHECKING) && defined(TM_USE_BSD_DOMAIN) */
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
#include <trssl.h>
#endif /* (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER)) */

/*
 * Take some data from the receive queue and give it to the user 
 */
int recv(int socketDescriptor, char TM_FAR *bufferPtr, int bufferLength,
         int flags)
{
    ttSocketEntryPtr socketEntryPtr;
#ifdef TM_USE_TCP
    ttTcpVectPtr     tcpVectPtr;
    ttPacketPtr      nxtPacketPtr;
#endif /* TM_USE_TCP */
    ttPacketPtr      packetPtr;
    ttPacketPtr      recvdPacketPtr;
    ttPktLen         copyCount;
    ttPktLen         recvdPacketLength;
    ttPktLen         linkCopyCount;
#ifdef TM_USE_TCP
    ttPktLen         nowaitCopyCount;
    ttPktLen         bufferLeftSpace;
#endif /* TM_USE_TCP */
    int              errorCode;
    int              retCode;
#ifdef TM_USE_TCP
    tt8Bit           protocolNumber;
#ifdef TM_DSP
    int              bufferByteOffset;
    unsigned int     dataOffset;
#endif /* TM_DSP */
#endif /* TM_USE_TCP */
#ifdef TM_USE_BSD_DOMAIN
    int             af;
#endif /* TM_USE_BSD_DOMAIN */

#ifdef TM_DSP
/* keep track of the byte offset into the next word of the user's buffer */
    bufferByteOffset = 0;
#endif /* TM_DSP */
/* PARAMETER CHECK */
/* Map from socket descriptor to locked socket entry pointer */
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_ANY;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckValidLock(  socketDescriptor
#ifdef TM_USE_BSD_DOMAIN
                                            , &af
#endif /* TM_USE_BSD_DOMAIN */
                                           );
#ifdef TM_USE_BSD_DOMAIN
    if (af == TM_BSD_FAMILY_NONINET) 
    {
        tm_assert(recv, socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR);
        retCode = tfBsdRecv( (ttVoidPtr)socketEntryPtr, bufferPtr,
                             bufferLength, flags );
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
        errorCode = TM_ENOERROR;
        recvdPacketLength = (ttPktLen)0;
        if ( socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR )
        {
            errorCode = TM_EBADF;
            goto recvFinish;
        }
#ifdef TM_USE_TCP
        tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr);
#endif /* TM_USE_TCP */
        if (    (bufferPtr == (char TM_FAR *)0)
/* we allow bufferlength between 0 and 65534 on 16-bit architecture */
             || (bufferLength == -1)
             || (bufferLength == 0)
             || ((flags & ~TM_RECV_FLAGS) != 0) )
        {
            errorCode = TM_EINVAL;
            goto recvFinish;
        }

#ifdef TM_USE_TCP
        protocolNumber = socketEntryPtr->socProtocolNumber;
#endif /* TM_USE_TCP */
/* Check to see if the socket is connected */
        if (   (!(socketEntryPtr->socFlags & TM_SOCF_CONNECTED))
#ifdef TM_USE_TCP
            && (protocolNumber != (tt8Bit)IP_PROTOTCP)
#endif /* TM_USE_TCP */
           )
        {
/* For a non TCP socket, can only use recv on connected sockets. */
            errorCode = TM_ENOTCONN;
            goto recvFinish;
        }
/* If user did not set any wait/dontwait flag */
        if ((flags & TM_SOCF_MSG_WAIT_FLAGS) ==  0)
        {
/* use the socket default */
            flags |= (socketEntryPtr->socFlags & TM_SOCF_NONBLOCKING);
        }
#ifdef TM_USE_TCP
        if (protocolNumber == (tt8Bit)IP_PROTOTCP)
        {
#if (defined(TM_USE_SSL_CLIENT) || (defined(TM_USE_SSL_SERVER)))
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
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */
            if ((flags & MSG_OOB) != 0)
            {
                errorCode = tfTcpRecvOobData(tcpVectPtr, bufferPtr, flags);
                if (errorCode == TM_ENOERROR)
                {
                    recvdPacketLength = (ttPktLen)1; /* one byte */
                }
                goto recvFinish;
            }
        }
        bufferLeftSpace = (ttPktLen)bufferLength;
#ifdef TM_PEND_POST_NEEDED
        do
#endif /* TM_PEND_POST_NEEDED */
#endif /* TM_USE_TCP */
        {
            packetPtr = socketEntryPtr->socReceiveQueueNextPtr;
/* Check for Data */
#ifdef TM_PEND_POST_NEEDED
            while (packetPtr == TM_PACKET_NULL_PTR)
#else /* !TM_PEND_POST_NEEDED */
            if (packetPtr == TM_PACKET_NULL_PTR)
#endif /* !TM_PEND_POST_NEEDED */
/* 
 *  No data. 
 */
            {
                errorCode = socketEntryPtr->socRecvError;
                if (errorCode != TM_ENOERROR)
                {
#ifdef TM_USE_TCP
                    if (recvdPacketLength != (ttPktLen)0)
                    {
/* Return partial data */
                        errorCode = TM_ENOERROR;
                    }
                    else
#endif /* TM_USE_TCP */
                    {
/* No data, and error on socket. Return read error on socket */
                        socketEntryPtr->socRecvError = 0;
                    }
                    goto recvError;
                }
#ifdef TM_USE_TCP
                if (protocolNumber == (tt8Bit)IP_PROTOTCP)
                {
/*
 * Check whether there is any reason to wait for more data. OK to make
 * a call to TCP state machine (instead of in-lining check on TCP state)
 * since there is no data in the receive queue anyway.
 */
                    errorCode = tfTcpRecv(tcpVectPtr);
                    if (errorCode != TM_ENOERROR)
                    {
                        if (recvdPacketLength != (ttPktLen)0)
                        {
/* Return partial data */
                            errorCode = TM_ENOERROR;
                        }
                        goto recvError;
                    }
                }
#endif /* TM_USE_TCP */
#ifdef TM_PEND_POST_NEEDED
                if (!(flags & TM_SOCF_NONBLOCKING))
                {
                    (void)tm_pend(&socketEntryPtr->socRecvPendEntry,
                                  &socketEntryPtr->socLockEntry);
#if (defined(TM_USE_SSL_CLIENT) || (defined(TM_USE_SSL_SERVER)))
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
                            (void)tfSslIncomingRecord
                                        (tcpVectPtr->tcpsSslConnStatePtr,
                                        TM_SSL_RECV_API);
                        }
                    }
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */
                    packetPtr = socketEntryPtr->socReceiveQueueNextPtr;
                }
                else
#endif /* TM_PEND_POST_NEEDED */
                {
#ifdef TM_USE_TCP
                    if (recvdPacketLength == (ttPktLen)0)
#endif /* TM_USE_TCP */
                    {
                        errorCode = TM_EWOULDBLOCK;
                    }
                    goto recvFinish;
                }
            }
/* TCP socket */
#ifdef TM_USE_TCP
            if (protocolNumber == (tt8Bit)IP_PROTOTCP)
            {
                if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_PACKET)
                    && (packetPtr->pktLinkDataLength > bufferLeftSpace))
                {
/* bug ID 105: if TM_TCP_PACKET is set, check if the specified buffer size is
 * smaller than the received TCP segment, in which case we return the error
 * TM_EMSGSIZE. 
 */
                    errorCode = TM_EMSGSIZE;
                    goto recvFinish;
                }

                if (    (socketEntryPtr->socOobMark != 0)
                     && ((tt32Bit)bufferLeftSpace > socketEntryPtr->socOobMark) )
                {
/* stop reading at out of band mark, in case user cares about finding it */
                    bufferLeftSpace = (ttPktLen)(socketEntryPtr->socOobMark);
                }
                nowaitCopyCount = (ttPktLen)0;
                while (    (bufferLeftSpace != (ttPktLen)0)
                        && (packetPtr != TM_PACKET_NULL_PTR) )
                {
/* Copy the data */
                
                    if (packetPtr->pktLinkDataLength > bufferLeftSpace)
                    {
/* 
 * There is more data in this packet than we have room for
 * So just copy and don't free up.
 */
#ifdef TM_DSP
/* Check for socket option */
                        if (socketEntryPtr->socOptions & SO_UNPACKEDDATA)
                        {
                            tm_bcopy_to_unpacked(
                                  (ttConstVoidPtr)(packetPtr->pktLinkDataPtr),
                                  (int)(packetPtr->pktLinkDataByteOffset),
                                  (ttVoidPtr)bufferPtr,
                                  (unsigned)bufferLeftSpace);
                        }
                        else
                        {
                            tfMemCopyOffset((int *)packetPtr->pktLinkDataPtr,
                                            packetPtr->pktLinkDataByteOffset,
/* destination word pointer for copy */
                                            (int *)bufferPtr,
/* destination 8-bit byte offset for copy */
                                            bufferByteOffset,
                                            bufferLeftSpace );
                        }
#else /* !TM_DSP */
                        tm_bcopy((ttConstVoidPtr)(packetPtr->pktLinkDataPtr),
                                 (ttVoidPtr)bufferPtr,
                                 (unsigned)bufferLeftSpace);
#endif /* !TM_DSP */

/* If the user is not peeking */
                        if ((flags & MSG_PEEK) == 0)
                        {
/* Update packet data pointer and length */
#ifdef TM_DSP
/* Total byte offset */
                            dataOffset = (unsigned int)
                                         (  bufferLeftSpace
                                          + packetPtr->pktLinkDataByteOffset);
                            packetPtr->pktLinkDataPtr += 
                                               tm_packed_byte_len(dataOffset);
/* keep track of the 8-bit byte index (into the word) of the new start of
   data in this buffer */
                            packetPtr->pktLinkDataByteOffset =
                                    dataOffset % TM_DSP_BYTES_PER_WORD;
#else /* !TM_DSP */
                            packetPtr->pktLinkDataPtr += bufferLeftSpace;
#endif /* !TM_DSP */
                            packetPtr->pktLinkDataLength -= bufferLeftSpace;
                        }
                        nowaitCopyCount += bufferLeftSpace;
                        bufferLeftSpace = (ttPktLen)0;
                    }
                    else
                    {
/* Copy this data area to the user buffer */
                        nowaitCopyCount += packetPtr->pktLinkDataLength;
                        bufferLeftSpace = (ttPktLen)(  
                                               bufferLeftSpace 
                                             - packetPtr->pktLinkDataLength);
#ifdef TM_DSP
                        if (socketEntryPtr->socOptions & SO_UNPACKEDDATA)
                        {
                            tm_bcopy_to_unpacked(
                                  (ttConstVoidPtr)(packetPtr->pktLinkDataPtr),
                                  (int)(packetPtr->pktLinkDataByteOffset),
                                  (ttVoidPtr)bufferPtr,
                                  (unsigned)(packetPtr->pktLinkDataLength));
/* Update the buffer Pointer */
                            bufferPtr += packetPtr->pktLinkDataLength;
                        }
                        else
                        {
                            tfMemCopyOffset((int *)packetPtr->pktLinkDataPtr,
                                            packetPtr->pktLinkDataByteOffset,
/* destination word pointer for copy */
                                            (int *)bufferPtr,
/* destination 8-bit byte offset for copy */
                                            bufferByteOffset,
                                            packetPtr->pktLinkDataLength);
/* Total byte offset */
                           bufferByteOffset =  packetPtr->pktLinkDataLength
                                             + bufferByteOffset;
/* Update the buffer Pointer */
                            bufferPtr += tm_packed_byte_len(bufferByteOffset);
/* keep track of the 8-bit byte index (into the word) of the new start of
   data in this buffer */
                            bufferByteOffset =
                                     bufferByteOffset % TM_DSP_BYTES_PER_WORD;
                        }
#else /* !TM_DSP */
                        {
                            tm_bcopy(
                                  (ttConstVoidPtr)(packetPtr->pktLinkDataPtr),
                                  (ttVoidPtr)bufferPtr,
                                  (unsigned)(packetPtr->pktLinkDataLength));
/* Update the buffer Pointer */
                            bufferPtr += packetPtr->pktLinkDataLength;
                        }
#endif /* !TM_DSP */

                        nxtPacketPtr = (ttPacketPtr)packetPtr->pktLinkNextPtr;

/* Since we copied the whole data area, free it if the user is not peeking */
                        if ((flags & MSG_PEEK) == 0)
                        {
                            tfFreeSharedBuffer(packetPtr, TM_SOCKET_LOCKED);
                        }
/* 
 * For TCP, since there is no boundary, data is queued using
 * pktLinkNextPtr.
 */
                        packetPtr = nxtPacketPtr;
                    }
                }
/* If the user is not peeking */
                if ((flags & MSG_PEEK) == 0)
                {
/* Update receive queue */
                    socketEntryPtr->socReceiveQueueNextPtr = packetPtr;
/*
 * Let TCP know that window is larger by nowaitCopyCount. 
 * Update receive queue size. Update out of band mark. Try and send
 * a window update if permitted by TCP algorithms.
 */
                    tfTcpRecvCmplt(tcpVectPtr, nowaitCopyCount, 0);
                }
                recvdPacketLength += nowaitCopyCount;
            }
            else
#endif /* TM_USE_TCP */
            {
/* Datagram protocols */
/* Most common case. User has enough room in its buffer. */
                copyCount = packetPtr->pktChainDataLength;
                recvdPacketPtr = packetPtr;
                recvdPacketLength = copyCount;
/* If the user is not peeking */
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
                              (unsigned)linkCopyCount);
/* Update the buffer Pointer */
                        bufferPtr += linkCopyCount;
                    }
                    else
                    {
                        tfMemCopyOffset((int *)packetPtr->pktLinkDataPtr,
                                        packetPtr->pktLinkDataByteOffset,
/* destination word pointer for copy */
                                        (int *)bufferPtr,
/* destination 8-bit byte offset for copy */
                                        bufferByteOffset,
                                        linkCopyCount);

/* Total byte offset */
                       bufferByteOffset = linkCopyCount + bufferByteOffset;
/* Update the buffer Pointer */
                        bufferPtr += tm_packed_byte_len(bufferByteOffset);
/* keep track of the 8-bit byte index (into the word) of the new start of
   data in this buffer */
                        bufferByteOffset =
                                   (bufferByteOffset) % TM_DSP_BYTES_PER_WORD;
                    }
#else /* !TM_DSP */
                    {
                        tm_bcopy((ttConstVoidPtr)(packetPtr->pktLinkDataPtr),
                                 (ttVoidPtr)bufferPtr,
                                 (unsigned)linkCopyCount);
/* Update the buffer Pointer */
                        bufferPtr += linkCopyCount;
                    }
#endif /* !TM_DSP */
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
                    socketEntryPtr->socRecvQueueBytes -= 
                                                (tt32Bit)recvdPacketLength;
                    socketEntryPtr->socRecvQueueDgrams--;
/* Free copied packet */
                    tfFreePacket(recvdPacketPtr, TM_SOCKET_LOCKED);
                }
            }
        }
#ifdef TM_USE_TCP
#ifdef TM_PEND_POST_NEEDED
        while (    
               (flags & MSG_WAITALL) 
            && (protocolNumber == (tt8Bit)IP_PROTOTCP)
            && (bufferLeftSpace != (ttPktLen)0)
            && (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags, TM_TCPF_PACKET)));

#endif /* TM_PEND_POST_NEEDED */
#endif /* TM_USE_TCP */
recvError:
        if (errorCode == TM_ESHUTDOWN)
        {
/* If the connection is shut down, return no error */
            if ( tm_16bit_one_bit_set(socketEntryPtr->socFlags, TM_SOCF_REOF) )
            {
                errorCode = TM_ENOERROR;
            }
        }
recvFinish:
/*
 * If errorCode not set, will return recvdPacketLength, otherwise it will set
 * the errorCode in the user error array, and return TM_SOCKET_ERROR instead
 */
        retCode = tfSocketReturn( socketEntryPtr, socketDescriptor,
                                  errorCode, (int)recvdPacketLength );
    }
    return retCode;
}
