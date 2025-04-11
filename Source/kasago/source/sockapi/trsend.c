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
 * Description: BSD Sockets Interface (send)
 *
 * Filename: trsend.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trsend.c $
 *
 * Modification History
 * $Revision: 6.0.2.10 $
 * $Date: 2012/09/03 12:41:43JST $
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
 * Send some data on the network
 * For datagram protocol make sure there is enough room
 * for the whole datagram, for stream protocol send a piece
 */
int send (
    int socketDescriptor, 
    const char TM_FAR *bufferPtr, 
    int bufferLength,
    int flags)
{
    ttSocketEntryPtr socketEntryPtr;
#ifdef TM_USE_TCP
    ttTcpVectPtr     tcpVectPtr;
#endif /* TM_USE_TCP */
    ttPacketPtr      newPacketPtr;
    tt32Bit          maxQueueSize;
    tt32Bit          totalInFlightBytes;
#ifdef TM_USE_TCP
    tt32Bit          amountToCopy;
    ttPktLen         appendBytes;
#endif /* TM_USE_TCP */
    ttPktLen         allocLength;
    ttPktLen         copyBytes;
    int              errorCode;
    int              hdrSize;
#ifdef TM_USE_TCP
    tt8Bit           protocol;
#endif /* TM_USE_TCP */
    int              retCode;
#ifdef TM_USE_BSD_DOMAIN
    int              af;
#endif /* TM_USE_BSD_DOMAIN */
#ifdef TM_USE_TCP
    tt8Bit           socketProtocol;
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
    tt8Bit           sslEnabled;
#endif /* (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER)) */
#endif /* TM_USE_TCP */

    errorCode = TM_ENOERROR;
    copyBytes = (ttPktLen)0;
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
        tm_assert(send, socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR);
        retCode = tfBsdSend(socketEntryPtr, bufferPtr, bufferLength, flags);
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
        if ( socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR )
        {
            errorCode = TM_EBADF;
            goto sendExit;
        }
#ifdef TM_USE_TCP
        tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr);
#endif /* TM_USE_TCP */
        if (    (bufferPtr == (const char TM_FAR *)0)
/* we allow bufferlength between 0 and 65534 on 16-bit architecture */
             || (bufferLength == -1)
             || (bufferLength == 0)
             || ((flags & ~TM_SEND_FLAGS) != 0) )
        {
            errorCode = TM_EINVAL;
            goto sendExit;
        }
        
        errorCode = socketEntryPtr->socSendError;
        if (errorCode != TM_ENOERROR)
        {
            socketEntryPtr->socSendError = 0;
            goto sendExit;
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
        protocol = socketEntryPtr->socProtocolNumber;
        socketProtocol = protocol;
        if (protocol == (tt8Bit)IP_PROTOTCP)
        {
            hdrSize = 0;
            if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_PACKET))
            {
                if (bufferLength > (int)(tcpVectPtr->tcpsEffSndMss))
                {
/* bug ID 105: if TM_TCP_PACKET is set, check if the specified buffer size is
   larger than the effective MSS, in which case we return the error
   TM_EMSGSIZE. */
                    errorCode = TM_EMSGSIZE;
                    goto sendFinish;
                }
/* So that we have room to copy the header */
                hdrSize = TM_MAX_SEND_TCP_HEADERS_SIZE;
/* So that we send like UDP, preserving datagram boundaries */
                protocol = IP_PROTOTPACKET; 
            }

            if (tm_16bit_one_bit_set( socketEntryPtr->socFlags,
                                      TM_SOCF_NO_MORE_SEND_DATA))
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
                goto sendFinish;
            }
            else
            {
/* ANVL 5.24 + 5.25: Queue data in SYN-SENT, and SYN-RCVD states */
                if (tcpVectPtr->tcpsState < TM_TCPS_SYN_SENT)
                {
/* If no connection establishment has started */
                    errorCode = TM_ENOTCONN;
                    goto sendFinish;
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
                goto sendFinish;
            }
            hdrSize = TM_MAX_SEND_HEADERS_SIZE;
        }
        totalInFlightBytes =   socketEntryPtr->socSendQueueBytes 
                             + (tt32Bit)bufferLength;
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
#ifdef TM_USE_TCP
        sslEnabled = TM_8BIT_ZERO;
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
                sslEnabled = TM_8BIT_YES;
                totalInFlightBytes =   totalInFlightBytes 
                                     + tcpVectPtr->tcpsSslSendQBytes;
            }
        }
#endif /* TM_USE_TCP */
#endif /* (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER)) */
        if (   (totalInFlightBytes <= maxQueueSize)
            && (
#ifdef TM_USE_TCP
                    (socketProtocol == (tt8Bit)IP_PROTOTCP) ||
#endif /* TM_USE_TCP */
                   (socketEntryPtr->socSendQueueDgrams
                    < socketEntryPtr->socMaxSendQueueDgrams)))
        {
/* Allocate the packet/data */
            allocLength = (ttPktLen)bufferLength;
#ifdef TM_USE_TCP
            if (protocol == (tt8Bit)IP_PROTOTCP)
            {
                if (
#if (defined(TM_USE_SSL_CLIENT) || (defined(TM_USE_SSL_SERVER)))
                        ( sslEnabled == TM_8BIT_ZERO) &&
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */
                        ( socketEntryPtr->socSendQueueBytes != 0 )
                     && (    (unsigned)bufferLength
                          <= (unsigned)socketEntryPtr->socSendAppendThreshold )
                     && ( (flags & MSG_OOB) == 0 ) 
                   )
                {
                    copyBytes = tfSendAppend( socketEntryPtr,
                                              bufferPtr,
                                              (ttPktLen)bufferLength,
#ifdef TM_USE_QOS
#ifdef TM_USE_IPV4
                                              (int)socketEntryPtr->socIpTos,
#else /* !TM_USE_IPV4 */
                                              0,
#endif /* !TM_USE_IPV4 */
#endif /* TM_USE_QOS */
                                              0 );
                    if (copyBytes != (ttPktLen)0)
                    {
                        goto sendFinish; /* All bytes appended. We are done */
                    }
/* No byte appended. We will allocate new buffer for bufferLength */
                }
                if (    (unsigned)bufferLength 
                     <= (unsigned)socketEntryPtr->socSendAppendThreshold)
                {
/* Make sure that we will have room for data not yet sent */
                    allocLength = (ttPktLen)socketEntryPtr->socSendAppendThreshold;
                }
            }
#endif /* TM_USE_TCP */
            newPacketPtr = tfGetSharedBuffer(
                                         hdrSize, allocLength, TM_16BIT_ZERO);
            if (newPacketPtr == TM_PACKET_NULL_PTR)
            {
/*
 * Only return an error if copyBytes is 0 to let the user know some
 * data has been copied
 */
                errorCode = TM_ENOBUFS;
            }
            else
            {
/* Copy the data */
                newPacketPtr->pktLinkDataLength = (ttPktLen)bufferLength;
                newPacketPtr->pktChainDataLength = (ttPktLen)bufferLength;

#ifdef TM_DSP
                if (socketEntryPtr->socOptions & SO_UNPACKEDDATA)
                {
                    tm_bcopy_to_packed((ttConstVoidPtr) bufferPtr,
                                   (ttVoidPtr)(newPacketPtr->pktLinkDataPtr),
                                   (unsigned)bufferLength);
                }
                else
#endif /* TM_DSP */
                {
                    tm_bcopy((ttConstVoidPtr)bufferPtr,
                             (ttVoidPtr)(newPacketPtr->pktLinkDataPtr),
                             (unsigned)tm_packed_byte_count(bufferLength));
                }

/* Send the data */
                errorCode = tfSocketSend(socketEntryPtr,
                                         newPacketPtr,(unsigned)bufferLength,
                                         flags);
                if (errorCode == TM_ENOERROR)
                {
                    copyBytes += (ttPktLen)bufferLength;
                }
                goto sendFinish;
            }
        }
        else
        {   
            if (flags & TM_SOCF_NONBLOCKING)
            {
#ifdef TM_USE_TCP
                if (protocol == IP_PROTOTCP)
                {
/*
 * For TCP only: Handle situation, not enough room in the send queue 
 * for the message in NONBLOCKING MODE
 */
/* Calculate the Space left in the queue */
                    totalInFlightBytes = socketEntryPtr->socSendQueueBytes;
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
                    if (sslEnabled)
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
/* Dont queue up anything if less than 1/4 of the send queue is available */
                    if (amountToCopy < socketEntryPtr->socLowSendQueueBytes)
                    {
/* "Silly Queueing Avoidance" of TCP packets */
                        errorCode = TM_EWOULDBLOCK;
                        goto sendFinish;
                    }
                    if (
#if (defined(TM_USE_SSL_CLIENT) || (defined(TM_USE_SSL_SERVER)))
                            ( sslEnabled == TM_8BIT_ZERO) &&
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */
                            ( socketEntryPtr->socSendQueueBytes != 0 )
                         && (    (ttPktLen)amountToCopy
                              <= (ttPktLen)
                                       socketEntryPtr->socSendAppendThreshold)
                         && ( (flags & MSG_OOB) == 0 ) 
                       )
                    {
                        copyBytes = tfSendAppend( socketEntryPtr,
                                                  bufferPtr,
                                                  (ttPktLen)amountToCopy,
#ifdef TM_USE_QOS
#ifdef TM_USE_IPV4
                                                 (int)socketEntryPtr->socIpTos,
#else /* !TM_USE_IPV4 */
                                                  0,
#endif /* !TM_USE_IPV4 */
#endif /* TM_USE_QOS */
                                                  0);
                        if (copyBytes != (ttPktLen)0)
                        {
/* All bytes appended. We are done */
                            goto sendFinish; 
                        }
/* No byte appended. We will allocate new buffer */
                    }
                    if (    (unsigned)amountToCopy 
                         <= (unsigned)socketEntryPtr->socSendAppendThreshold)
                    {
/* Make sure that we will have room for data not yet sent */
                        allocLength = (ttPktLen)socketEntryPtr->socSendAppendThreshold;
                    }
                    else
                    {
                        allocLength = (ttPktLen)amountToCopy;
                    }
/* Allocate the packet/data */
                    newPacketPtr = tfGetSharedBuffer(
                                            hdrSize, allocLength, TM_16BIT_ZERO);
                    if (newPacketPtr == TM_PACKET_NULL_PTR)
                    {
/*
 * Only return an error if copyBytes is 0 to let the user know some
 * data has been copied
 */
                        errorCode = TM_ENOBUFS;
                        goto sendFinish;
                    }
                    newPacketPtr->pktLinkDataLength = (ttPktLen)amountToCopy;
                    newPacketPtr->pktChainDataLength = (ttPktLen)amountToCopy;
/* Copy the portion of the data that fits in the send queue */
#ifdef TM_DSP
                    if (socketEntryPtr->socOptions & SO_UNPACKEDDATA)
                    {
                        tm_bcopy_to_packed(
                                    (ttConstVoidPtr) bufferPtr,
                                    (ttVoidPtr)(newPacketPtr->pktLinkDataPtr),
                                    (unsigned)amountToCopy);
                    }
                    else
#endif /* TM_DSP */
                    {
                        tm_bcopy((ttConstVoidPtr)bufferPtr,
                                 (ttVoidPtr)(newPacketPtr->pktLinkDataPtr),
                                 (unsigned) tm_packed_byte_count(amountToCopy));
                    }
/* Send a portion of the data */
                    errorCode = tfSocketSend(socketEntryPtr,newPacketPtr,
                                             (ttPktLen)amountToCopy, flags);
                    copyBytes = (ttPktLen)amountToCopy;
                }
                else
#endif /* TM_USE_TCP */
                {
/* non-TCP */
                    if ((tt32Bit)bufferLength >
                                         socketEntryPtr->socMaxSendQueueBytes)
                    {
                        errorCode = TM_EMSGSIZE;
                        goto sendFinish;
                    }
                    else
                    {
                        errorCode = TM_EWOULDBLOCK;
                        goto sendFinish;
                    }
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
                        totalInFlightBytes = 
                                           socketEntryPtr->socSendQueueBytes;
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
                        if (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                                 TM_SSLF_SEND_ENABLED))
                        {
                            totalInFlightBytes = totalInFlightBytes
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
#ifdef TM_DSP
                        if ( (amountToCopy + (tt32Bit)copyBytes) <
                                                     (tt32Bit)bufferLength )
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
                        while (   (amountToCopy < socketEntryPtr->
                                                        socLowSendQueueBytes)
                               && ((amountToCopy + (tt32Bit)copyBytes) <
                                                        (tt32Bit)bufferLength) )

                        {
/*
 * Delay until we have at least 1/4 of send queue free or we have room
 * for all of our remaining data
 */
                            errorCode = tm_pend(
                                        &socketEntryPtr->socSendPendEntry,
                                        &socketEntryPtr->socLockEntry);
#ifdef TM_PEND_POST_NEEDED
                            if (errorCode != TM_ENOERROR)
                            {
/* TCP hard error occured. Let the user know about it */
                                socketEntryPtr->socSendError = 0;
                                goto sendFinish;
                            }
/* recompute amount of data left in the send queue after the pend */
                            totalInFlightBytes = 
                                           socketEntryPtr->socSendQueueBytes;
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
                                totalInFlightBytes =
                                               totalInFlightBytes
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
#ifdef TM_DSP
                            if ( (amountToCopy + (tt32Bit)copyBytes) <
                                                     (tt32Bit)bufferLength )
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
                            socketEntryPtr->socSendError = 0;
                            goto sendFinish;
#endif /* TM_PEND_POST_NEEDED */
                        }
/* Do not copy more than bufferlength */
                        if (   (amountToCopy + (tt32Bit)copyBytes)
                             > (tt32Bit)bufferLength )
                        {
/* send the rest of the user's data */
                            amountToCopy =   (tt32Bit)bufferLength
                                           - (tt32Bit)copyBytes;
                        }
/*
 * If amount to copy is less than the append threshold and there is a
 * packet in the send queue, and we are not sending out of band data
 */
                        if (
#if (defined(TM_USE_SSL_CLIENT) || (defined(TM_USE_SSL_SERVER)))
                                (tm_16bit_bits_not_set(tcpVectPtr->tcpsSslFlags,
                                                    TM_SSLF_SEND_ENABLED)) &&
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */
                                ( socketEntryPtr->socSendQueueBytes != 0 )
                             && ((ttPktLen)amountToCopy <= (ttPktLen)
                                        socketEntryPtr->socSendAppendThreshold)
                             && ( (flags & MSG_OOB) == 0 ) )
                        {
                            appendBytes = tfSendAppend( socketEntryPtr,
                                                        bufferPtr,
                                                        (ttPktLen)amountToCopy,
#ifdef TM_USE_QOS
#ifdef TM_USE_IPV4
                                                  (int)socketEntryPtr->socIpTos,
#else /* !TM_USE_IPV4 */
                                                        0,
#endif /* !TM_USE_IPV4 */
#endif /* TM_USE_QOS */
                                                        0 );
                            if (appendBytes != (ttPktLen)0)
                            {
/* Appended the whole buffer */
                                copyBytes += appendBytes;
#ifdef TM_DSP
                                if (   socketEntryPtr->socOptions 
                                     & SO_UNPACKEDDATA )
                                {
                                    bufferPtr += (unsigned int)appendBytes;
                                }
                                else
#endif /* TM_DSP */
                                {
                                    bufferPtr += (unsigned int)
                                           tm_packed_byte_count(appendBytes);
                                }
                                continue;
                            }
/* Could not append the user data, allocate a new buffer below */
                        }
                        if (    (unsigned)amountToCopy 
                             <= (unsigned)socketEntryPtr->socSendAppendThreshold)
                        {
/* Make sure that we will have room for data not yet sent */
                            allocLength = (ttPktLen)socketEntryPtr->socSendAppendThreshold;
                        }
                        else
                        {
                            allocLength = (ttPktLen)amountToCopy;
                        }
/* Allocate the packet/data */
                        newPacketPtr = tfGetSharedBuffer(
                            hdrSize, (ttPktLen)allocLength, TM_16BIT_ZERO);
                        if (newPacketPtr == TM_PACKET_NULL_PTR)
                        {
/*
 * Only return an error if copyBytes is 0 to let the user know some
 * data has been copied
 */
                            if (copyBytes == (ttPktLen)0)
                            {
                                errorCode = TM_ENOBUFS;
                            }
                            goto sendFinish;
                        }
                        newPacketPtr->pktLinkDataLength = (ttPktLen)amountToCopy;
                        newPacketPtr->pktChainDataLength = (ttPktLen)amountToCopy;
/* Copy the data */
#ifdef TM_DSP
                        if (socketEntryPtr->socOptions & SO_UNPACKEDDATA)
                        {
                            tm_bcopy_to_packed(
                                    (ttConstVoidPtr) bufferPtr,
                                    (ttVoidPtr)(newPacketPtr->pktLinkDataPtr),
                                    (unsigned)amountToCopy);
                            bufferPtr += (ttPktLen)amountToCopy;
                        }
                        else
#endif /* TM_DSP */
                        {
                            tm_bcopy((ttConstVoidPtr)bufferPtr,
                                (ttVoidPtr)(newPacketPtr->pktLinkDataPtr),
                                (unsigned)tm_packed_byte_count(amountToCopy));
                            bufferPtr += 
                                 (ttPktLen)tm_packed_byte_count(amountToCopy);
                        }
/* send up to the max send queue size */
                        errorCode = tfSocketSend(socketEntryPtr,
                                                 newPacketPtr,
                                                 (ttPktLen)amountToCopy,
                                                 flags);
                        if (errorCode != TM_ENOERROR)
                        {
                            goto sendFinish;
                        }
                        copyBytes += (ttPktLen)amountToCopy;
                    } while (    (copyBytes < (ttPktLen)bufferLength)
                              && (tm_16bit_bits_not_set(
                                            socketEntryPtr->socFlags,
                                            TM_SOCF_NO_MORE_SEND_DATA)) );
                }
                else
#endif /* TM_USE_TCP */
                {
/* non-TCP */
                    if ((tt32Bit)bufferLength >
                                         socketEntryPtr->socMaxSendQueueBytes)
                    {
                        errorCode = TM_EMSGSIZE;
                        goto sendFinish;
                    }
/* loop until we have enough room for the entire datagram */
#ifdef TM_PEND_POST_NEEDED
                    do
                    {
                        errorCode = tm_pend(&socketEntryPtr->socSendPendEntry,
                                            &socketEntryPtr->socLockEntry);
                        if (errorCode != TM_ENOERROR)
                        {
                            socketEntryPtr->socSendError = 0;
                            goto sendFinish;
                        }
                    } while ((socketEntryPtr->socSendQueueBytes
                              + (tt32Bit)bufferLength
                              > socketEntryPtr->socMaxSendQueueBytes)
                             || (socketEntryPtr->socSendQueueDgrams
                                 >= socketEntryPtr->socMaxSendQueueDgrams));
                    newPacketPtr = tfGetSharedBuffer(
                        hdrSize, (ttPktLen) bufferLength, TM_16BIT_ZERO);
                    if (newPacketPtr == TM_PACKET_NULL_PTR)
                    {
                        errorCode = TM_ENOBUFS;
                        goto sendFinish;
                    }
/* Copy the data */
#ifdef TM_DSP
                    if (socketEntryPtr->socOptions & SO_UNPACKEDDATA)
                    {
                        tm_bcopy_to_packed(
                                    (ttConstVoidPtr) bufferPtr,
                                    (ttVoidPtr)(newPacketPtr->pktLinkDataPtr),
                                    (unsigned)bufferLength);
                    }
                    else
#endif /* TM_DSP */
                    {
                        tm_bcopy((ttConstVoidPtr)bufferPtr,
                                 (ttVoidPtr)(newPacketPtr->pktLinkDataPtr),
                                 (unsigned)tm_packed_byte_count(bufferLength));
                    }

/* Send all of the data */
                    errorCode = tfSocketSend(socketEntryPtr,newPacketPtr,
                                             (ttPktLen)bufferLength,flags);
                    copyBytes = (ttPktLen)bufferLength;
#else /* TM_PEND_POST_NEEDED */
                    socketEntryPtr->socSendError = 0;
                    goto sendFinish;
#endif /* TM_PEND_POST_NEEDED */
                }
            }
        }

sendFinish:
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
/*
 * If errorCode not set, will return copyBytes, otherwise it will set
 * the errorCode in the user error array, and return TM_SOCKET_ERROR instead
 */
sendExit:
        retCode = tfSocketReturn(socketEntryPtr, socketDescriptor,
                                 errorCode, (int)copyBytes);
    }
    return retCode;
}

#ifdef TM_USE_TCP
/*
 * Try and append the user buffer to the end of the last packet
 * in the TCP send queue. Copy all or nothing, i.e., if there is not
 * enough room for all the user data, do not append anything.
 */
#ifdef TM_LINT
LINT_NULL_PTR_BEGIN(prevPacketPtr)
#endif /* TM_LINT */
ttPktLen tfSendAppend ( 
    ttSocketEntryPtr    socketEntryPtr,
    const char TM_FAR * bufferPtr,
    ttPktLen            bufferLength,
#ifdef TM_USE_QOS
    int                 tos,
#endif /* TM_USE_QOS */
    int                 flags )
{
    ttPacketPtr     prevPacketPtr;
    ttPktLen        leftOverBytes;
    ttSharedDataPtr pktShrDataPtr;
    ttTcpVectPtr    tcpVectPtr;
#ifdef TM_DSP
    ttCharPtr       destPtr;
    unsigned int    destOffset;
#endif /* TM_DSP */

#if (!defined(TM_USE_SSL_CLIENT) && !defined(TM_USE_SSL_SERVER))
    TM_UNREF_IN_ARG(flags); /* compiler warning */
#endif /* !(defined(TM_USE_SSL_CLIENT) && !defined(TM_USE_SSL_SERVER)) */
    tcpVectPtr = (ttTcpVectPtr)(ttVoidPtr)socketEntryPtr;
#if(defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
    if(flags & TM_SSLF_SEND_ENABLED)
    {
        prevPacketPtr = ((ttSslConnectStatePtr)(ttVoidPtr)(tcpVectPtr
                        ->tcpsSslConnStatePtr))->scsWritePktTailPtr;
    }
    else
#endif /* (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER)) */
    {
        prevPacketPtr = socketEntryPtr->socSendQueueLastPtr;
    }

#ifdef TM_ERROR_CHECKING
    if (prevPacketPtr == TM_PACKET_NULL_PTR)
    {
        tfKernelError("tfSendAppend", "Send queue corrupted");
    }
#endif /* TM_ERROR_CHECKING */
    pktShrDataPtr = prevPacketPtr->pktSharedDataPtr;
#ifdef TM_USE_QOS
    if (pktShrDataPtr->dataIpTos != (tt8Bit)tos)
/* Do not append data to buffer with different type of service */
    {
        bufferLength = 0;
    }
    else
#endif /* TM_USE_QOS */
    {
    leftOverBytes = (ttPktLen)
        ((   (ttPktLen)tm_byte_count(   pktShrDataPtr->dataBufLastPtr 
                                      - prevPacketPtr->pktLinkDataPtr )
             - prevPacketPtr->pktLinkDataLength )
#ifdef TM_DSP
         - prevPacketPtr->pktLinkDataByteOffset
#endif /* TM_DSP */
            );
    if (bufferLength > leftOverBytes)
    {
        bufferLength = 0;
    }
    if (bufferLength != 0)
    {
#ifdef TM_DSP
/* Total offset */
        destOffset =   prevPacketPtr->pktLinkDataLength
                     + prevPacketPtr->pktLinkDataByteOffset;
/* Point to beginning of word at offset */
        destPtr =   (ttCharPtr) (prevPacketPtr->pktLinkDataPtr 
                                 + tm_packed_byte_len(destOffset));
/* Offset within the word pointed to by destPtr */
        destOffset = destOffset % TM_DSP_BYTES_PER_WORD;
        if (socketEntryPtr->socOptions & SO_UNPACKEDDATA)
        {
/* 
 * DSP: copies unpacked data from the user into packed data, appending it to
 * the end of the last packet in the send queue
 */
            tm_bcopy_to_packed_offset((ttConstVoidPtr)bufferPtr,
                                      destPtr,
                                      (unsigned)bufferLength,
                                      destOffset);
        }
        else if (destOffset != (unsigned int)0)
        {
/*
 * DSP: copies packed data from the user, appending it to the end of the last
 * packet in the send queue.  Can't use bcopy since the last buffer isn't
 * aligned (not a multiple of the word size).
 */
            tfMemCopyOffset((int *)bufferPtr,
                            0,
                            (int *)destPtr,
                            destOffset,
                            bufferLength);
        }
        else
        {
/* destPtr is aligned on a word boundary */
            tm_bcopy(bufferPtr,
                     destPtr,
                     (unsigned)tm_packed_byte_count(bufferLength));
        }
#else /* !TM_DSP */
        tm_bcopy(bufferPtr,
                 prevPacketPtr->pktLinkDataPtr 
                  + prevPacketPtr->pktLinkDataLength,
                 (unsigned)bufferLength);
#endif /* !TM_DSP */
        prevPacketPtr->pktLinkDataLength += bufferLength;
#if(defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
        if(flags & TM_SSLF_SEND_ENABLED)
        {
            tcpVectPtr->tcpsSslSendQBytes = (tt32Bit)
                                           (    tcpVectPtr->tcpsSslSendQBytes
                                             +  (tt32Bit)bufferLength );
        }
        else
#endif /*(defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))*/
        {
            socketEntryPtr->socSendQueueBytes += (tt32Bit)bufferLength;
            if (    (tcpVectPtr->tcpsState == TM_TCPS_ESTABLISHED)
                 || (tcpVectPtr->tcpsState == TM_TCPS_CLOSE_WAIT) )
            {
                (void)tfTcpSendPacket(tcpVectPtr);
            }
        }
    }
    }
    return bufferLength; /* amount appended */
}
#ifdef TM_LINT
LINT_NULL_PTR_END(prevPacketPtr)
#endif /* TM_LINT */
#endif /* TM_USE_TCP */
