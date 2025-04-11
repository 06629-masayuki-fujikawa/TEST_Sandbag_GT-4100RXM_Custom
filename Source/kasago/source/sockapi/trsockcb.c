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
 * Description: BSD Sockets Interface (tfRegisterSocketCB)
 *
 * Filename: trsockcb.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trsockcb.c $
 *
 * Modification History
 * $Revision: 6.0.2.4 $
 * $Date: 2015/07/27 11:06:30JST $
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
#ifdef TM_USE_SSL
#include <trssl.h>
#endif /* TM_USE_SSL */

/* 
 * Used to register a function to call upon completion of any socket
 * event as set in flags.
 */
int tfRegisterSocketCB(int                      socketDescriptor,
                       ttUserSocketCBFuncPtr    socketCBFuncPtr,
                       int                      flags)
{
    int              errorCode;
    ttSocketEntryPtr socketEntryPtr;
#ifdef TM_USE_BSD_DOMAIN
    int              af;
#endif /* TM_USE_BSD_DOMAIN */
    
/* PARAMETER CHECK */
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckValidLock(  socketDescriptor
#ifdef TM_USE_BSD_DOMAIN
                                            , &af
#endif /* TM_USE_BSD_DOMAIN */
                                           );
    if (socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR)
    {
        if (    (socketCBFuncPtr == (ttUserSocketCBFuncPtr)0)
             && (flags != 0) )
        {
            errorCode = TM_EINVAL;
        }
        else
        {
            errorCode = TM_ENOERROR;
            socketEntryPtr->socUserCBFuncPtr=
                                       (ttUserSocketCBFuncPtr)socketCBFuncPtr;
            socketEntryPtr->socCBFlags = (tt16Bit)flags;
            tm_16bit_clr_bit( socketEntryPtr->socFlags,
                              TM_SOCF_CB_PARAM_ON );
            if (socketCBFuncPtr != (ttUserSocketCBFuncPtr)0)
            {
                tfSockCBNotify(socketEntryPtr);
            }
        }
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
    }
    return tfSocketReturn(socketEntryPtr, socketDescriptor,
                          errorCode, TM_ENOERROR);
}

void tfSockCBNotify (ttSocketEntryPtr socketEntryPtr)
{
    int selectFlags;
    int socketCBFlags;

    selectFlags = 0;
    socketCBFlags = 0;
    if (socketEntryPtr->socRecvQueueBytes != 0)
    {
        selectFlags = TM_SELECT_READ;
        socketCBFlags = TM_CB_RECV;
    }
    if ( tm_16bit_one_bit_set(socketEntryPtr->socFlags, TM_SOCF_REOF) )
/* received EOF mark */
    {
        selectFlags = TM_SELECT_READ;
        socketCBFlags |= TM_CB_REMOTE_CLOSE;
    }
#ifdef TM_USE_SSL
    if (socketEntryPtr->socProtocolNumber == (tt8Bit)IP_PROTOTCP)
    {
        if (tm_16bit_one_bit_set(
                    ((ttTcpVectPtr)(ttVoidPtr)socketEntryPtr)->tcpsSslFlags,
                    TM_SSLF_HANDSHK_PROCESS))
        {
            selectFlags |= TM_SELECT_EXCEPTION;
            socketCBFlags |= TM_CB_SSL_HANDSHK_PROCESS;
        }
        if (tm_16bit_one_bit_set(
                    ((ttTcpVectPtr)(ttVoidPtr)socketEntryPtr)->tcpsSslFlags,
                    TM_SSLF_ESTABLISHED))
        {
            selectFlags |= TM_SELECT_WRITE;
            socketCBFlags |= TM_CB_SSL_ESTABLISHED;
        }
        if (tm_16bit_one_bit_set(
                    ((ttTcpVectPtr)(ttVoidPtr)socketEntryPtr)->tcpsSslFlags,
                    TM_SSLF_HANDSHK_FAILURE))
        {
            selectFlags |= (TM_SELECT_READ | TM_SELECT_WRITE);
            socketCBFlags |= TM_CB_SSL_HANDSHK_FAILURE;
        }
    }
#endif /* TM_USE_SSL */
    if (selectFlags != 0)
    {
        tfSocketNotify( socketEntryPtr,
                        selectFlags,
                        socketCBFlags,
                        0 );
    }
}
