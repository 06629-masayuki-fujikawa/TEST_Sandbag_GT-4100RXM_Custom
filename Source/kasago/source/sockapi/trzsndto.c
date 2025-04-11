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
 * Description: BSD Sockets Interface (tfZeroCopySendTo)
 *
 * Filename: trzsndto.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trzsndto.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:02JST $
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
 * Send some data on the network
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
int tfZeroCopySendTo (int                           socketDescriptor,
                      ttUserMessage                 bufferHandle,
                      int                           bufferLength,
                      int                           flags,
                      const struct sockaddr TM_FAR *toAddressPtr,
                      int                           toAddressLength)
{
    ttSocketEntryPtr    socketEntryPtr;
    ttPacketPtr         packetPtr;
    tt8Bit              needFreePacket;
    int                 retCode;
    int                 errorCode;
    ttPacketPtr         newPacketPtr;
    ttPktLen            availHdrLen;
#ifdef TM_USE_BSD_DOMAIN
    int                 af;
#endif /* TM_USE_BSD_DOMAIN */
    
    errorCode = TM_ENOERROR;
    needFreePacket = TM_8BIT_YES;
/* PARAMETER CHECK */
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
/*
 * Map from socket descriptor to a locked socket entry pointer and verify
 * that the address length is at least what we expect for this address type.
 */ 
    socketEntryPtr = tfSocketCheckAddrLock(  socketDescriptor,
                                             toAddressPtr,
                                             toAddressLength,
                                             &errorCode
#ifdef TM_USE_BSD_DOMAIN
                                           , &af
#endif /* TM_USE_BSD_DOMAIN */
                                           );
    if ( socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR )
    {
/* errorCode set by tfSocketCheckAndLock */
        goto zeroCopySendToFinish;
    }
    packetPtr = (ttPacketPtr)bufferHandle;
    if (    (packetPtr == TM_PACKET_NULL_PTR)
         || ( !(   packetPtr->pktUserFlags
                 & TM_PKTF_USER_OWNS ) ) )
    {
        errorCode = TM_EFAULT;
/* Not a zero copy buffer handle */
        needFreePacket = TM_8BIT_ZERO;
        goto zeroCopySendToFinish;
    }

    if ((bufferLength == -1)
        || (bufferLength == 0)
        || ((flags & MSG_OOB))
        || ((flags & ~TM_ZEROCOPYSENDTO_FLAGS) != 0)
        || (toAddressPtr == (const struct sockaddr TM_FAR *)0))
    {
        errorCode = TM_EINVAL;
        goto zeroCopySendToFinish;
    }
#ifdef TM_DSP
    if (socketEntryPtr->socOptions & SO_UNPACKEDDATA)
    {            
        if (packetPtr->pktLinkExtraCount != 0)
/* Cannot send scattered data if SO_UNPACKEDDATA is set */
        {
            errorCode = TM_EPERM;
            goto zeroCopySendToFinish;
        }
    }
#endif /* TM_DSP */
/* Check to see if it is a TCP socket */
    if (socketEntryPtr->socProtocolNumber == IP_PROTOTCP)
    {
        errorCode = TM_EPROTOTYPE;
        goto zeroCopySendToFinish;
    }
/* Check to see if it is a connected socket */
    if (socketEntryPtr->socFlags & TM_SOCF_CONNECTED)
    {
        errorCode = TM_EISCONN;
        goto zeroCopySendToFinish;
    }
    if ((tt32Bit)bufferLength > socketEntryPtr->socMaxSendQueueBytes)
    {
        errorCode = TM_EMSGSIZE;
        goto zeroCopySendToFinish;
    }
/* If user did not set any wait/dontwait flag */
    if ((flags & TM_SOCF_MSG_WAIT_FLAGS) ==  0)
    {
/* use the socket default */
        flags |= (socketEntryPtr->socFlags & TM_SOCF_NONBLOCKING);
    }
    if ((socketEntryPtr->socSendQueueBytes + (tt32Bit)bufferLength >
         socketEntryPtr->socMaxSendQueueBytes)
        || (socketEntryPtr->socSendQueueDgrams
            >= socketEntryPtr->socMaxSendQueueDgrams))
    {
        if (flags & TM_SOCF_NONBLOCKING)
        {
/* User stil owns the buffer */
            needFreePacket = TM_8BIT_ZERO;
            errorCode = TM_EWOULDBLOCK;
            goto zeroCopySendToFinish;
        }
        else
        {
/* loop until we have enough room for the entire datagram */
#ifdef TM_PEND_POST_NEEDED
            do
            {
                errorCode = tm_pend(&socketEntryPtr->socSendPendEntry,
                                    &socketEntryPtr->socLockEntry);
                if (errorCode != TM_ENOERROR)
                {
                    goto zeroCopySendToFinish;
                }
            } while ((socketEntryPtr->socSendQueueBytes
                      + (tt32Bit)bufferLength
                      > socketEntryPtr->socMaxSendQueueBytes)
                     || (socketEntryPtr->socSendQueueDgrams
                         >= socketEntryPtr->socMaxSendQueueDgrams));
#else /* TM_PEND_POST_NEEDED */
            goto zeroCopySendToFinish;
#endif /* TM_PEND_POST_NEEDED */
        }            
    }
    errorCode = tfSocketCopyDest(socketEntryPtr, toAddressPtr
#ifdef TM_4_USE_SCOPE_ID
                               , packetPtr
#endif /* TM_4_USE_SCOPE_ID */
                                );
    if (errorCode != TM_ENOERROR)
    {
        goto zeroCopySendToFinish;
    }
    if ((flags & MSG_SCATTERED) == 0)
    {
/* Load the sizes of the packet */
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
 * Note that in tfZeroCopyUserBufferSendTo() we set the MSG_SCATTERED flag even
 * though the message is not scattered so that this code prepends room for
 * the headers.
 */
            availHdrLen = (ttPktLen) (packetPtr->pktLinkDataPtr
                - packetPtr->pktSharedDataPtr->dataBufFirstPtr);
            if (availHdrLen < TM_MAX_SEND_HEADERS_SIZE)
            {
                newPacketPtr = tfGetSharedBuffer(
                    TM_MAX_SEND_HEADERS_SIZE, (ttPktLen) 0, TM_16BIT_ZERO);
                if (newPacketPtr != (ttPacketPtr) 0)
                {
                    newPacketPtr->pktChainDataLength =
                        packetPtr->pktChainDataLength;
                    newPacketPtr->pktLinkNextPtr = ((ttVoidPtr)bufferHandle);
                    newPacketPtr->pktLinkExtraCount =
                                            packetPtr->pktLinkExtraCount + 1;
                    packetPtr = newPacketPtr;
                }
                else
                {
                    errorCode = TM_ENOBUFS;
                    goto zeroCopySendToFinish;
                }
            }
        }
    }
#ifdef TM_DSP
    if (socketEntryPtr->socOptions & SO_UNPACKEDDATA)
    {
        newPacketPtr = tfGetSharedBuffer(
            TM_MAX_SEND_HEADERS_SIZE, (ttPktLen) bufferLength, TM_16BIT_ZERO);
        if (newPacketPtr != (ttPacketPtr) 0)
        {        

            tm_bcopy_to_packed(packetPtr->pktLinkDataPtr,
                               newPacketPtr->pktLinkDataPtr,
                               bufferLength);
            tfFreeZeroCopyBuffer(bufferHandle);
            packetPtr = newPacketPtr;
        }
        else
        {
            errorCode = TM_ENOBUFS;
            goto zeroCopySendToFinish;
        }
    }
#endif /* TM_DSP */
    packetPtr->pktSharedDataPtr->dataUserDataLength = (ttPktLen)bufferLength;
#ifdef TM_DEV_RECV_OFFLOAD
/* If recv checksum was offloaded, reset offload pointer */
    packetPtr->pktDevOffloadPtr = (ttVoidPtr)0;
#endif /* TM_DEV_RECV_OFFLOAD */
/* Send the data. tfSocketSend thread owns the buffer */
    needFreePacket = TM_8BIT_ZERO;
    errorCode = tfSocketSend(socketEntryPtr, packetPtr,
                             (ttPktLen)bufferLength, flags);

zeroCopySendToFinish:
/*
 * If errorCode not set, will return bufferLength, otherwise it will set
 * the errorCode in the user error array, and return TM_SOCKET_ERROR instead
 */
    retCode = tfSocketReturn(socketEntryPtr, socketDescriptor,
                             errorCode, bufferLength);
/* If buffer has not been freed up */
    if ( needFreePacket != TM_8BIT_ZERO)
    {
        (void)tfFreeZeroCopyBuffer(bufferHandle);
    }
    return retCode;
} 
