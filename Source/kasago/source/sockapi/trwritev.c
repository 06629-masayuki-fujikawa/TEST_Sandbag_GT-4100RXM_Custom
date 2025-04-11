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
 * Description: BSD Sockets Interface (writev)
 *
 * Filename: trwritev.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trwritev.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:55JST $
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

/*
 * Perform a scattered write to a socket
 */

int writev (int socketDescriptor, struct iovec TM_FAR * iovecPtr, int iocount)
{
    ttSocketEntryPtr socketEntryPtr;
    char     TM_FAR *dataPtr;
    ttUserMessage    buf;
    int              retCode;
    int              i;
    int              errorCode;
    tt32Bit          socSendQBytes;
    tt32Bit          bufferLength;
    tt32Bit          sendBytesLeft;
#ifdef TM_USE_BSD_DOMAIN
    int              af;
#endif /* TM_USE_BSD_DOMAIN */
    tt16Bit          nonBlocking;
    tt8Bit           socProtoNum;


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
        tm_assert(writev, socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR);
        retCode = tfBsdWritev( (ttVoidPtr)socketEntryPtr, iovecPtr, iocount);
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
        socProtoNum = socketEntryPtr->socProtocolNumber;
        socSendQBytes = socketEntryPtr->socSendQueueBytes;
        nonBlocking = (tt16Bit)(socketEntryPtr->socFlags & TM_SOCF_NONBLOCKING);
        (void)tfSocketReturn(socketEntryPtr, socketDescriptor,
                             TM_ENOERROR, TM_ENOERROR);

        retCode = TM_SOCKET_ERROR;
        errorCode = TM_EINVAL; /* assume bad parameters */
        if (    (iovecPtr != (struct iovec TM_FAR *)0)
             && (iocount > 0) && (iocount <= IOV_MAX) )
        {
            bufferLength = 0;
/* Compute the total buffer length */
            for (i = 0; i < iocount; i++)
            {
/* Each individual iovecPtr entries should have a length bigger than 0 */
                if (iovecPtr[i].iov_len <= 0)
                {
                    bufferLength = 0;
                    break;
                }
                bufferLength += (tt32Bit)(iovecPtr[i].iov_len);
            }
            sendBytesLeft = bufferLength;
            if (socProtoNum == IPPROTO_TCP)
            {
                if (nonBlocking != TM_16BIT_ZERO)
                {
                    sendBytesLeft = (tt32Bit)
                                    tfGetSendBytesLeft(socketDescriptor);
                    if (sendBytesLeft < bufferLength)
                    {
                        bufferLength = sendBytesLeft;
                    }
                }
            }
            else
            {
                if (bufferLength > socSendQBytes)
                {
                    bufferLength = 0;
                }
            }
            if ((bufferLength > 0) && (bufferLength <= SSIZE_MAX))
            {
/* Get a zero copy buffer and copy our data into it */
                buf = tfGetZeroCopyBuffer((int)bufferLength, &dataPtr);
                if (buf != (ttUserMessage)0)
                {
/* parameters are ok, and we got a zero copy buffer */
                    errorCode = TM_ENOERROR;
/* Copy the data into the buffer */
                    for (i = 0; i < iocount; i++)
                    {
                        if (sendBytesLeft < (tt32Bit)(iovecPtr[i].iov_len))
                        {
                            tm_bcopy( iovecPtr[i].iov_base, dataPtr,
                                      sendBytesLeft );
                            dataPtr += sendBytesLeft;
                            break;
                        }
                        else
                        {
                            tm_bcopy( iovecPtr[i].iov_base, dataPtr,
                                    iovecPtr[i].iov_len );
                            dataPtr += iovecPtr[i].iov_len;
                            sendBytesLeft -= (tt32Bit)(iovecPtr[i].iov_len);
                        }
                    }
                    retCode = tfZeroCopySend( socketDescriptor, buf,
                                              (int)bufferLength, 0);
                    if (retCode == TM_SOCKET_ERROR)
                    {
                        errorCode = tfGetSocketError(socketDescriptor);
                        if (errorCode == TM_EWOULDBLOCK)
                        {
/* We still own the buffer. Free it. */
                            (void)tfFreeZeroCopyBuffer(buf);
                        }
/* Error code already recorded for socketDescriptor */
                        errorCode = TM_ENOERROR;
                    }
                }
                else
                {
                    errorCode = TM_ENOBUFS;
                }
            }
        }
/* Bad parameter (TM_EINVAL), or no zero copy buffer (TM_ENOBUFS) */
        if (errorCode != TM_ENOERROR)
        {
/*
 * Record the socket error for socketDescriptor, so the user can retrieve it.
 */
            tfSocketErrorRecord(socketDescriptor, errorCode);
        }
    }
    return (retCode);
}


