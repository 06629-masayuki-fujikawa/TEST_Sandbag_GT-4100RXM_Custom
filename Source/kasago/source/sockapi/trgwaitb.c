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
 * Description: BSD Sockets Interface (tfGetWaitingBytes)
 *
 * Filename: trgwaitb.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trgwaitb.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:27JST $
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
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
#include <trssl.h>
#endif /* (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER)) */


/* 
 * Get the number of bytes waiting to be read on a socket.
 */ 
int tfGetWaitingBytes(int socketDescriptor)
{
    int              retCode;
    int              errorCode;
#ifdef TM_USE_BSD_DOMAIN
    int              af;
#endif /* TM_USE_BSD_DOMAIN */
    ttSocketEntryPtr socketEntryPtr;
#if (defined(TM_USE_SSL_CLIENT) || (defined(TM_USE_SSL_SERVER)))
    ttTcpVectPtr     tcpVectPtr;
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */
    
/* PARAMETER CHECK */
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckValidLock(socketDescriptor
#ifdef TM_USE_BSD_DOMAIN
                                            , &af
#endif /* TM_USE_BSD_DOMAIN */
                                           );
    if (socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR)
    {
#if (defined(TM_USE_SSL_CLIENT) || (defined(TM_USE_SSL_SERVER)))
        if (socketEntryPtr->socProtocolNumber == (tt8Bit)IP_PROTOTCP)
        {
            tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr);
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
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */
        retCode = (int)(socketEntryPtr->socRecvQueueBytes);
        errorCode = TM_ENOERROR;
    }
    else
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
        retCode = TM_SOCKET_ERROR;
    }
    return tfSocketReturn(socketEntryPtr, socketDescriptor,
                          errorCode, retCode);
}
