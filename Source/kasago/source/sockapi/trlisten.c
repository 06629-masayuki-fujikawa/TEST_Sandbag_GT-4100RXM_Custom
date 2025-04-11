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
 * Description: BSD Sockets Interface (listen)
 *
 * Filename: trlisten.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trlisten.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:43JST $
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
#if (defined(TM_USE_BSD_DOMAIN) || defined(TM_USE_SSL_SERVER)) \
    && defined(TM_ERROR_CHECKING)
#include <trglobal.h>
#endif /* (TM_USE_BSD_DOMAIN || TM_USE_SSL_SERVER) && (TM_ERROR_CHECKING) */

/*
 * Setup the maximum amount of connections that we will allow
 */
int listen(int socketDescriptor, int backLog)
{
    ttSocketEntryPtr socketEntryPtr;
    int              errorCode;
    int              retCode;
#ifdef TM_USE_BSD_DOMAIN
    int              af;
#endif /* TM_USE_BSD_DOMAIN */
    
/* PARAMETER CHECK */
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_ANY;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckValidLock(socketDescriptor
#ifdef TM_USE_BSD_DOMAIN
                                            , &af
#endif /* TM_USE_BSD_DOMAIN */
                                           );
#ifdef TM_USE_BSD_DOMAIN
    if (af == TM_BSD_FAMILY_NONINET) 
    {
        tm_assert(listen, socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR);
        retCode = tfBsdListen((ttVoidPtr)socketEntryPtr, backLog);
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
        if (socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR)
        {
            errorCode = TM_EBADF;
            goto listenFinish;
        }
        if (backLog < 0)
        {
            errorCode = TM_EINVAL;
            goto listenFinish;
        }
#ifdef TM_USE_TCP
/* Make sure we are setting up listen on a TCP socket only */
        if (socketEntryPtr->socProtocolNumber == (tt8Bit)IP_PROTOTCP)
        {
/* Set the backlog of maximum connections waiting for accept */
            socketEntryPtr->socFlags |= TM_SOCF_LISTENING;
#ifdef TM_USE_SSL_SERVER
/* we allocate SSL state when you set this socket to be ssl_enabled,
 * for a listening socket, we don't use SSL state, free it.
 */
            if(((ttTcpVectPtr)socketEntryPtr)->tcpsSslConnStatePtr)
            {
                tm_free_raw_buffer(((ttTcpVectPtr)socketEntryPtr)->
                             tcpsSslConnStatePtr);
                ((ttTcpVectPtr)socketEntryPtr)->
                             tcpsSslConnStatePtr = (ttVoidPtr)0;
            }
#endif /* TM_USE_SSL_SERVER */
            errorCode = tfTcpOpen( (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr),
                                   TM_TCPE_PASSIVE_OPEN );
            if (errorCode == TM_ENOERROR)
            {
/* Allow matches in tree */
                if (tm_16bit_bits_not_set(socketEntryPtr->socFlags,
                                          TM_SOCF_IN_TREE))
                {
/* Pick random port in connection less tree */
                    retCode = (int)tfRandPortInsert(
                           socketEntryPtr,
                           &(socketEntryPtr->socTuple),
                           (tt16Bit)(  TM_SOC_TREE_CL_FLAG
                                     | (   socketEntryPtr->socOptions
                                         & (SO_REUSEADDR | SO_REUSEPORT) ) ) );
                    if (retCode == 0)
/* No port */
                    {
                        errorCode = TM_EADDRINUSE;
                    }
                }
                else
                {
                    tm_16bit_clr_bit( socketEntryPtr->socCoFlags,
                                  TM_SOC_TREE_NO_MATCH_FLAG );
                }
            }
            if (errorCode == TM_ENOERROR)
            {
                socketEntryPtr->socMaxBackLog = (tt16Bit)backLog;
            }
            else
            {
                tm_16bit_clr_bit( socketEntryPtr->socFlags,
                                  TM_SOCF_LISTENING );
            }
        }
        else
#endif /* TM_USE_TCP */
        {
            errorCode = TM_EOPNOTSUPP;
        }
listenFinish:
        retCode = tfSocketReturn(socketEntryPtr, socketDescriptor,
                                 errorCode, TM_ENOERROR);
    }
    return retCode;
}

