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
 * Description: BSD Sockets Interface (getsockname)
 *
 * Filename: trgsockn.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trgsockn.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:25JST $
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
 * Get our local IP and Port if assigned
 */
int getsockname(int socketDescriptor, struct sockaddr TM_FAR *myAddressPtr, 
                int TM_FAR *addressLengthPtr)
{
    int                        errorCode;
    int                        addressLength;
    int                        retCode;
#ifdef TM_USE_BSD_DOMAIN
    int                        af;
#endif /* TM_USE_BSD_DOMAIN */
    ttSocketEntryPtr           socketEntryPtr;

/* PARAMETER CHECK */
    if (addressLengthPtr != (int TM_FAR *)0)
    {
        addressLength = *addressLengthPtr;
    }
    else
    {
        addressLength = 0;
    }
    
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_ANY;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckAddrLenLock(  socketDescriptor,
                                                myAddressPtr,
                                                addressLength,
                                                &errorCode
#ifdef TM_USE_BSD_DOMAIN
                                              , &af
#endif /* TM_USE_BSD_DOMAIN */
                                             );
/* errorCode set by tfSocketCheckAndLock, if socketEntryPtr is null */
#ifdef TM_USE_BSD_DOMAIN
    if (af == TM_BSD_FAMILY_NONINET) 
    {
        tm_assert(getsockname, socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR);
        retCode = tfBsdGetSockName( (ttVoidPtr)socketEntryPtr, myAddressPtr,
                                    addressLengthPtr );
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
        if (socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR)
        {
            if (myAddressPtr == TM_SOCKADDR_NULL_PTR)
            {
                errorCode = TM_EINVAL;
            }
            else
            {
                errorCode = TM_ENOERROR;
            }
        }
        retCode = tfSocketReturnAddr( socketEntryPtr, socketDescriptor,
                                      myAddressPtr, 
                                      errorCode, TM_ENOERROR, 
                                      TM_ADDR_LOCAL_FLAG );
    }
    return retCode;
}


