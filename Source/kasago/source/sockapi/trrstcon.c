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
 * Description: BSD Sockets Interface (tfResetConnection)
 *
 * Filename: trrstcon.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trrstcon.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:08JST $
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

#ifdef TM_USE_BSD_DOMAIN
int tfAbort(int socketDescriptor)
{
    return tfResetConnection(socketDescriptor);
}
#endif /* TM_USE_BSD_DOMAIN */

/* To Abort the TCP Connection */
int tfResetConnection(int socketDescriptor)
{
    ttSocketEntryPtr socketEntryPtr;
    int              errorCode;
    int              retCode;
#ifdef TM_USE_BSD_DOMAIN
    int                af;
#endif /* TM_USE_BSD_DOMAIN */

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
        tm_assert(tfAbort, socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR);
        retCode = tfBsdAbort(socketEntryPtr);
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
        if (socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR)
        {
#ifdef TM_USE_TCP
            if (socketEntryPtr->socProtocolNumber == (tt8Bit)IP_PROTOTCP)
            {
                errorCode = TM_ENOERROR;
                (void)tfTcpAbort((ttTcpVectPtr)socketEntryPtr, TM_ECONNABORTED);
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
        retCode = tfSocketReturn(socketEntryPtr, socketDescriptor,
                                 errorCode, TM_ENOERROR);
    }
    return retCode;
}
