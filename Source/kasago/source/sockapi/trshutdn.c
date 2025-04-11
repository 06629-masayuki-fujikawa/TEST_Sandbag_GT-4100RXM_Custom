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
 * Description: BSD Sockets Interface (shutdown)
 *
 * Filename: trshutdn.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trshutdn.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:24JST $
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

int shutdown(int socketDescriptor, int howToShutdown)
{

    ttSocketEntryPtr socketEntryPtr;
    int              errorCode;
    int              retCode;
#ifdef TM_USE_TCP
    tt16Bit          flags;
    tt8Bit           protocol;
    tt8Bit           selectFlag;
#endif /* TM_USE_TCP */
#ifdef TM_USE_BSD_DOMAIN
    int              af;
#endif /* TM_USE_BSD_DOMAIN */

    errorCode = TM_ENOERROR;

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
        tm_assert(shutdown, socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR);
        retCode = tfBsdShutDown( socketEntryPtr, howToShutdown);
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
/* PARAMETER CHECK */
        if (socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR)
        {
            if ((howToShutdown < 0) || (howToShutdown > 2))
            {
                errorCode = TM_EINVAL;
                goto shutdownExit;
            }
/* socketEntryPtr non nil, socket is not closed yet */
#ifdef TM_USE_TCP
            protocol = socketEntryPtr->socProtocolNumber;
            if (protocol == IP_PROTOTCP)
            {
/* If socket is not closing or already closed */
                if (    (tm_16bit_bits_not_set(socketEntryPtr->socFlags,
                                               TM_SOCF_CLOSING))
                     && (tm_16bit_one_bit_set(socketEntryPtr->socFlags,
                                              TM_SOCF_OPEN)) )
                {
                    if (howToShutdown == 0) /* read-only */
                    {
                         flags = TM_SOCF_NO_MORE_RECV_DATA;
                         selectFlag = TM_SELECT_READ;
                    }
                    else
                    {
                        if (howToShutdown == 1) /* write only */
                        {
                            flags = TM_SOCF_NO_MORE_SEND_DATA;
                            selectFlag = TM_SELECT_WRITE;
                        }
                        else /* read-write */
                        {
                            flags = TM_SOCF_NO_MORE_RECV_DATA|
                                    TM_SOCF_NO_MORE_SEND_DATA;
                            selectFlag = TM_SELECT_READ|TM_SELECT_WRITE;
                        }
                    }
                    socketEntryPtr->socFlags |= flags;
/* Let any pending recv and/or send about the shutdown */
                    tfSocketNotify(socketEntryPtr,
                                   (int)selectFlag,
                                   0,
                                   TM_ESHUTDOWN);
                    if (flags & TM_SOCF_NO_MORE_RECV_DATA)
                    {
/* 
 * If there is any unread data, flush receive and holding queues, and 
 * abort the connection.
 */
                        tfTcpShutRead((ttTcpVectPtr)socketEntryPtr);
                    }
                    if (flags & TM_SOCF_NO_MORE_SEND_DATA)
                    {
/*
 * Let TCP deal with sending a FIN if needed.
 */
                        errorCode = 
                                tfTcpShutWrite((ttTcpVectPtr)socketEntryPtr);
                    }
                }
                else
                {
                    errorCode = TM_ESHUTDOWN;
                }
            }
            else
#endif /* TM_USE_TCP */
            {
                errorCode = TM_EOPNOTSUPP;
            }
        }
        else
        {
            errorCode = TM_EBADF;
        }
shutdownExit:
        retCode =  tfSocketReturn(socketEntryPtr, socketDescriptor,
                                  errorCode, TM_ENOERROR);
    }
    return retCode;
}

