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
 * Description: BSD Sockets Interface (sendto)
 *
 * Filename: trsendto.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trsendto.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:19JST $
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
int sendto (int                           socketDescriptor,
            const char            TM_FAR *bufferPtr,
            int                           bufferLength,
            int                           flags,
            const struct sockaddr TM_FAR *toAddressPtr,
            int                           toAddressLength)
{
    ttSocketEntryPtr    socketEntryPtr;
    int                 errorCode;
    ttPacketPtr         newPacketPtr;
    int                 retCode;
#ifdef TM_USE_BSD_DOMAIN
    int                 af;
#endif /* TM_USE_BSD_DOMAIN */

    errorCode = TM_ENOERROR;
    
/* PARAMETER CHECK */
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_ANY;
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
#ifdef TM_USE_BSD_DOMAIN
    if (af == TM_BSD_FAMILY_NONINET)
    {
        tm_assert(sendto, socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR);
        retCode = tfBsdSendTo( socketEntryPtr, bufferPtr, bufferLength, flags,
                               toAddressPtr, toAddressLength );
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
        if ( socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR )
        {
            goto sendtoFinish;
        }
        if (    (bufferPtr == (const char TM_FAR *)0)
/* we allow bufferlength between 0 and 65534 on 16-bit architecture */
             || (bufferLength == -1) 
             || (bufferLength == 0)
             || ((flags & ~TM_SENDTO_FLAGS) != 0)
             || (toAddressPtr == (const struct sockaddr TM_FAR *)0))
        {
            errorCode = TM_EINVAL;
            goto sendtoFinish;
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
                } while ((socketEntryPtr->socSendQueueBytes
                          + (tt32Bit)bufferLength
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
/* Copy the data */
#ifdef TM_DSP
        if (socketEntryPtr->socOptions & SO_UNPACKEDDATA)
        {
            tm_bcopy_to_packed((ttConstVoidPtr)bufferPtr,
                               (ttVoidPtr)(newPacketPtr->pktLinkDataPtr),
                               bufferLength); 
        }
        else
#endif /* TM_DSP */
        {

            tm_bcopy((ttConstVoidPtr)bufferPtr,
                     (ttVoidPtr)(newPacketPtr->pktLinkDataPtr),
                     (unsigned)tm_packed_byte_count(bufferLength));
        }

        errorCode = tfSocketSend(socketEntryPtr,newPacketPtr,
                                 (unsigned)bufferLength,flags);
sendtoFinish:
/*
 * tfSocketReturn will return TM_SOCKET_ERROR if errorCode is set, otherwise
 * it will return bufferLength.
 */
        retCode = tfSocketReturn(socketEntryPtr, socketDescriptor,
                                 errorCode, bufferLength);
    }
    return retCode;
}
