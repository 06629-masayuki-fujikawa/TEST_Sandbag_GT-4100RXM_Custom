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
 * Description: BSD Sockets Interface (readv)
 *
 * Filename: trreadv.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trreadv.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:02JST $
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
 * Perform a scatter read from a socket
 */
int readv (int socketDescriptor, struct iovec TM_FAR * iovecPtr, int iocount)
{

    ttSocketEntryPtr socketEntryPtr;
    ttUserMessage    buf;
    char TM_FAR *    dataPtr;
    int              i;
    int              bufferLength;
    int              totalLength;
    int              recvLength;
    int              ioOffset;
    int              errorCode;
    int              flags;
    int              sockError;
#ifdef TM_USE_BSD_DOMAIN
    int              af;
#endif /* TM_USE_BSD_DOMAIN */
    

#ifdef TM_USE_BSD_DOMAIN
/*
 * Map from socket descriptor to locked socket entry pointer, but only
 * for non inet sockets.
 */
    af = TM_BSD_FAMILY_NONINET;
    socketEntryPtr = tfSocketCheckValidLock(  socketDescriptor
                                            , &af
                                           );
    if (af == TM_BSD_FAMILY_NONINET) 
    {
        tm_assert(readv, socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR);
        totalLength = tfBsdReadv( (ttVoidPtr)socketEntryPtr, iovecPtr, iocount);
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
/* PARAMETER CHECK */
        totalLength = TM_SOCKET_ERROR;
        errorCode = TM_EINVAL; /* assume bad parameters */
        if (    (iovecPtr != (struct iovec TM_FAR *)0)
             && (iocount > 0) )
        {
            bufferLength = 0;
/* Compute the total buffer length */
            for (i = 0; i < iocount; i++)
            {
/* Each individual io entries should have a length bigger than 0 */
                if (iovecPtr[i].iov_len <= 0)
                {
                    bufferLength = 0;
                    break;
                }
                bufferLength += iovecPtr[i].iov_len;
            }

            if (bufferLength > 0)
            {
                errorCode = TM_ENOERROR; /* parameters are ok */

                totalLength = 0;
                ioOffset = 0;
                i = 0;
                flags = 0;
                while (bufferLength > 0)
                {

/* 
 * The first time tfZeroCopyRecv is called it should block until some data is
 * available in the TCP receive queue.  However, tfZeroCopyRecv doesn't
 * necessarily return all of the data available, but simply the next buffer
 * in the queue.  To ensure that all of the available data is copied into the
 * user's buffers we continue to call tfZeroCopyRecv.  Since we're just 
 * interested in the data currently in the queue and not on waiting for any
 * future data, every call after the first is done in non-blocking mode.
 */
                    recvLength = tfZeroCopyRecv(socketDescriptor,
                                                &buf, 
                                                &dataPtr, 
                                                bufferLength, 
                                                flags);
                    flags = MSG_DONTWAIT;

                    if (recvLength > 0)
                    {
                        totalLength += recvLength;
                        bufferLength -= recvLength;
                        while (recvLength > iovecPtr[i].iov_len)
                        {
                            tm_bcopy( dataPtr,
                                      iovecPtr[i].iov_base + ioOffset,
                                      iovecPtr[i].iov_len);
                            dataPtr += iovecPtr[i].iov_len;
                            recvLength -= iovecPtr[i++].iov_len;
                            ioOffset = 0;
                        }
                        if (recvLength > 0)
                        {
                            tm_bcopy( dataPtr, iovecPtr[i].iov_base,
                                      recvLength );
                            dataPtr += recvLength;
                            ioOffset = recvLength;
                        }
                        (void)tfFreeZeroCopyBuffer(buf);
                    }
                    else /* error or EOF */
                    {

                        if (totalLength == 0)
                        {
                            totalLength = recvLength;
                        }
                        else
                        {

#ifdef TM_USE_BSD_DOMAIN
                            af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
                            socketEntryPtr = 
                                       tfSocketCheckValidLock( socketDescriptor
#ifdef TM_USE_BSD_DOMAIN
                                                              , &af
#endif /* TM_USE_BSD_DOMAIN */
                                                             );

                            if ( socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR )
                            {
                                if (recvLength == TM_SOCKET_ERROR)
                                {
/* 
 * Socket error: If data has already been received into the user's buffers
 * and a socket error is encountered, first return the data to the user.
 * When the user calls readv/recv again the error will be returned at that
 * point.
 */
                                    sockError = 
                                            tfGetSocketError(socketDescriptor);

                                    if (sockError != TM_EWOULDBLOCK)
                                    {
                                        socketEntryPtr->socRecvError =
                                                           (tt16Bit)sockError;
                                    }
                                }
                                else
                                {
/*
 * EOF:  If data has already been copied into the user's buffers, there is no
 * method for returning an EOF code to them along with all of the data.
 * Mark the socket so that the next time the user reads from this socket an
 * EOF will be returned.
 */
                                    socketEntryPtr->socFlags |= TM_SOCF_REOF;                    
                                }
                                (void)tfSocketReturn(socketEntryPtr,
                                                     socketDescriptor, 0, 0);
                            }
/*
 *                          else
 *                          {
 *
 * The socket being used has been closed since calling tfZeroCopyRecv.  Return
 * the data already copied to the user's buffers.  The error will be returned
 * the next time readv is called with this socket.
 * 
 *                          }
 */
                        }

                        break;

                    }
                }
            }
        }

        if (errorCode != TM_ENOERROR) /* bad parameter */
        {
/* Record the socket error for socketDescriptor */
            tfSocketErrorRecord(socketDescriptor, errorCode);
        }
    }
    return (totalLength);
}
