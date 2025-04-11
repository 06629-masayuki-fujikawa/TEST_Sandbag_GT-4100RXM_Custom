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
 * Description: BSD Sockets Interface (tfSendToFrom)
 *
 * Filename: trsendfr.c
 * Author: Odile
 * Date Created: 12/14/00
 * $Source: source/sockapi/trsendfr.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:18GMT+09:00 $
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
 * tfSendToFrom function description
 * Same as sendto, but takes 1 extra parameter to let the user specify
 * the source address.
 * Parameters
 * Parameter        Description
 * socketDescriptor The socket descriptor to use to send data
 * bufferPtr        The buffer to send
 * bufferLength     The length of the buffer to send
 * flags            MSG_DONTWAIT: Don't wait for room in the socket send queue
 *                  0: wait for room in the socket send queue
 * toAddressPtr     The address to send the data to
 * addressLength    The length of the area pointed to by toAddressPtr, or
 *                  fromAddressPtr
 * fromAddressPtr   The address to send the data from
 * Returns
 * Value            Meaning
 * >=0              Number of bytes actually sent on the socket
 * -1               An error occured, error can be retrieved with
 *                  tfGetSocketError()
 * tfGetSocketError will return:
 * TM_EBADF         The socket descriptor is invalid
 * TM_ENOBUFS       There was insufficient user memory available to complete
 *                  the operation
 * TM_EMSGSIZE      The message was too long
 * TM_EPROTOTYPE    TCP protocol requires usage of send, not sendto
 * TM_EWOULDBLOCK   The socket is marked as non-blocking and
 *                  the send operation would block
 */
int tfSendToFrom ( int                           socketDescriptor,
                   const char            TM_FAR *bufferPtr,
                   int                           bufferLength,
                   int                           flags,
                   const struct sockaddr TM_FAR *toAddressPtr,
                   int                           addressLength,
                   const struct sockaddr TM_FAR *fromAddressPtr )
{
    ttSocketEntryPtr           socketEntryPtr;
    int                        errorCode;
    ttPacketPtr                newPacketPtr;
    ttSockAddrPtrUnion         fromSockAddr;
#ifdef TM_USE_BSD_DOMAIN
    int                        af;
#endif /* TM_USE_BSD_DOMAIN */
    
    errorCode = TM_ENOERROR;
    fromSockAddr.sockPtr = (struct sockaddr TM_FAR *)fromAddressPtr; 
    
/* PARAMETER CHECK */
/*
 * Map from socket descriptor to a locked socket entry pointer and verify
 * that the address length is at least what we expect for this address type.
 */ 
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckAddrLock(  socketDescriptor,
                                             toAddressPtr,
                                             addressLength,
                                             &errorCode
#ifdef TM_USE_BSD_DOMAIN
                                           , &af
#endif /* TM_USE_BSD_DOMAIN */
                                          );

    if ( socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR )
    {
/* errorCode set by tfSocketCheckAddrLock */
        goto sendtoFinish;
    }
    if ((bufferPtr == (const char TM_FAR *)0)
/* we allow bufferlength between 0 and 65534 on 16-bit architecture */
        || (bufferLength == -1) 
        || (bufferLength == 0)
        || ((flags & ~TM_SENDTO_FLAGS) != 0)
        || (toAddressPtr == (const struct sockaddr TM_FAR *)0))
    {
        errorCode = TM_EINVAL;
        goto sendtoFinish;
    }

    if (fromAddressPtr != (const struct sockaddr TM_FAR *)0)
    {
/* Verify that the address type matches the protocol type of this socket. */
        if (fromSockAddr.sockPtr->sa_family
            != socketEntryPtr->socProtocolFamily)
        {
            errorCode = TM_EAFNOSUPPORT;
            goto sendtoFinish;
        }
    
/* verify that the address length is at least what we expect */
#ifdef TM_USE_IPV4
        if ((fromSockAddr.sockPtr->sa_family == AF_INET) &&
            (addressLength < (int)sizeof(struct sockaddr_in)))
        {
            errorCode = TM_EINVAL;
            goto sendtoFinish;
        }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
        if (fromSockAddr.sockPtr->sa_family == AF_INET6)
        {
            if (addressLength < (int)sizeof(struct sockaddr_in6))
            {
                errorCode = TM_EINVAL;
                goto sendtoFinish;
            }
#ifndef TM_USE_IPV4
            else
            {
/* IPv4-mapped IPv6 addresses are not supported in IPv6-only mode */
                if (IN6_IS_ADDR_V4MAPPED(
                        &(fromSockAddr.sockIn6Ptr->sin6_addr)))
                {
/* IPv4 protocol is not available in IPv6-only mode */
                    errorCode = TM_ENOPROTOOPT;
                    goto sendtoFinish;
                }                
            }
#endif /* !TM_USE_IPV4 */
        }
#endif /* TM_USE_IPV6 */
    }
    
    errorCode = socketEntryPtr->socSendError;
    if (errorCode != TM_ENOERROR)
    {
        socketEntryPtr->socSendError = 0;
        goto sendtoFinish;
    }

/* Check to see if it is a TCP socket */
    if (socketEntryPtr->socProtocolNumber == (tt8Bit)IP_PROTOTCP)
    {
        errorCode = TM_EPROTOTYPE;
        goto sendtoFinish;
    }
/* Check to see if it is a connected socket */
    if (socketEntryPtr->socFlags & TM_SOCF_CONNECTED)
    {
        errorCode = TM_EISCONN;
        goto sendtoFinish;
    }
    if ((tt32Bit)bufferLength > socketEntryPtr->socMaxSendQueueBytes)
    {
        errorCode = TM_EMSGSIZE;
        goto sendtoFinish;
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
            errorCode = TM_EWOULDBLOCK;
            goto sendtoFinish;
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
                    socketEntryPtr->socSendError = 0;
                    goto sendtoFinish;
                }
            } while ((socketEntryPtr->socSendQueueBytes + (tt32Bit)bufferLength
                      > socketEntryPtr->socMaxSendQueueBytes)
                     || (socketEntryPtr->socSendQueueDgrams
                         >= socketEntryPtr->socMaxSendQueueDgrams));
#else /* TM_PEND_POST_NEEDED */
            socketEntryPtr->socSendError = 0;
            goto sendtoFinish;
#endif /* TM_PEND_POST_NEEDED */
        }            
    }        
    errorCode = tfSocketCopyDest(socketEntryPtr, toAddressPtr
#ifdef TM_4_USE_SCOPE_ID
                                , (ttPacketPtr)0
#endif /* TM_4_USE_SCOPE_ID */
                                );
    if (errorCode != TM_ENOERROR)
    {
        goto sendtoFinish;
    }
    newPacketPtr = tfGetSharedBuffer(
        TM_MAX_SEND_HEADERS_SIZE, (ttPktLen) bufferLength, TM_16BIT_ZERO);
    if (newPacketPtr == TM_PACKET_NULL_PTR)
    {
        errorCode = TM_ENOBUFS;
        goto sendtoFinish;
    }
    if (fromAddressPtr != (const struct sockaddr TM_FAR *)0)
    {
/* Indicate that source address should be retrived from dataFromIpAddress */
        newPacketPtr->pktUserFlags |= TM_PKTF_USER_SEND_FROM;
#ifdef TM_USE_IPV6
        if (fromSockAddr.sockPtr->sa_family == AF_INET6)
        {
            tm_6_ip_copy_structs(
                fromSockAddr.sockIn6Ptr->sin6_addr,
                newPacketPtr->pktSharedDataPtr->dataFromIpAddress);
/* Embed scope ID (if necessary).  */ 
            tm_6_addr_add_scope(
                &newPacketPtr->pktSharedDataPtr->dataFromIpAddress,
                fromSockAddr.sockIn6Ptr->sin6_scope_id);
        }
        else
        {
            tm_6_addr_to_ipv4_mapped(
                (tt4IpAddress)(fromSockAddr.sockInPtr->sin_addr.s_addr),
                &(newPacketPtr->pktSharedDataPtr->dataFromIpAddress));
        }
#else /* ! TM_USE_IPV6 */
        tm_ip_copy((tt4IpAddress)(fromSockAddr.sockInPtr->sin_addr.s_addr),
                    newPacketPtr->pktSharedDataPtr->dataFromIpAddress);
#endif /* ! TM_USE_IPV6 */
        newPacketPtr->pktSharedDataPtr->dataLocalPortNumber
            = (ttIpPort)(fromSockAddr.sockInPtr->sin_port);
    }
/* Copy the data */
    tm_bcopy((ttConstVoidPtr)bufferPtr,
             (ttVoidPtr)(newPacketPtr->pktLinkDataPtr),
             (unsigned)tm_packed_byte_count(bufferLength));
    errorCode = tfSocketSend(socketEntryPtr,newPacketPtr,
                             (unsigned)bufferLength,flags);
sendtoFinish:
/*
 * tfSocketReturn will return TM_SOCKET_ERROR if errorCode is set, otherwise
 * it will return bufferLength.
 */
    return tfSocketReturn(socketEntryPtr, socketDescriptor,
                          errorCode, bufferLength);
} 

