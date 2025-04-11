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
 * Description: BSD Sockets Interface (tfGetSendCompltBytes)
 *
 * Filename: trgcompb.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trgcompb.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:13JST $
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

/* 
 * Get the number of bytes sent and acked on a socket.
 */ 
int tfGetSendCompltBytes(int socketDescriptor)
{
    ttSocketEntryPtr socketEntryPtr;
    int              retCode;
    int              errorCode;
#ifdef TM_USE_BSD_DOMAIN
    int              af;
#endif /* TM_USE_BSD_DOMAIN */

    retCode = TM_SOCKET_ERROR;
    errorCode = TM_EOPNOTSUPP;

/* Parameter check */
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
        if (socketEntryPtr->socProtocolNumber == (tt8Bit)IP_PROTOTCP)
        {
            retCode = (int)(((ttTcpVectPtr)socketEntryPtr)->
                                                      tcpsSendCompltBytes);
            ((ttTcpVectPtr)socketEntryPtr)->tcpsSendCompltBytes = 0;
            errorCode = TM_ENOERROR;
        }
    }
    else
    {
#ifdef TM_USE_BSD_DOMAIN
        if (af != TM_BSD_FAMILY_NONINET)
#endif /* TM_USE_BSD_DOMAIN */
        {
            errorCode = TM_EBADF;
        }
    }
    return tfSocketReturn( socketEntryPtr, socketDescriptor,
                           errorCode, retCode );
}
