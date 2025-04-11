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
 * Description: BSD Sockets Interface (tfRegisterSocketCBParam)
 *
 * Filename: trsockcp.c
 * Author: Odile
 * Date Created: 11/30/98
 * $Source: source/sockapi/trsockcp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:32JST $
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
 * Used to register a function to call upon completion of any socket
 * event as set in flags. socketCBParamFuncPtr is a user defined pointer,
 * and is passed as an extra parameter by the call back function.
 */
int tfRegisterSocketCBParam(int                         socketDescriptor,
                            ttUserSocketCBParamFuncPtr  socketCBParamFuncPtr,
                            void TM_FAR                 *socketUserPtr,
                            int                         flags)
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
        if (    (socketCBParamFuncPtr == (ttUserSocketCBParamFuncPtr)0)
             && (flags != 0) )
        {
            errorCode = TM_EINVAL;
        }
        else
        {
            errorCode = TM_ENOERROR;
            socketEntryPtr->socUserCBParamFuncPtr =
                             (ttUserSocketCBParamFuncPtr)socketCBParamFuncPtr;
            socketEntryPtr->socApiEntryPtr = socketUserPtr;
            socketEntryPtr->socCBFlags = (tt16Bit)flags;
            socketEntryPtr->socFlags |= TM_SOCF_CB_PARAM_ON;
            if (socketCBParamFuncPtr != (ttUserSocketCBParamFuncPtr)0)
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
