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
 * Description: BSD Sockets Interface (getpeername)
 *
 * Filename: trgpeer.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trgpeer.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:20JST $
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
#if defined(TM_USE_BSD_DOMAIN) && defined(TM_ERROR_CHECKING)
#include <trglobal.h>
#endif /* defined(TM_USE_BSD_DOMAIN) && defined(TM_ERROR_CHECKING) */

/*
 * Get the remote IP and Port if assigned
 */
int getpeername(int socketDescriptor, struct sockaddr TM_FAR *fromAddressPtr, 
                int TM_FAR *addressLengthPtr)
{
    int                        errorCode;
    int                        retCode;
    ttSocketEntryPtr           socketEntryPtr;
    int                        addressLength; 
#ifdef TM_USE_BSD_DOMAIN
    int                        af;
#endif /* TM_USE_BSD_DOMAIN */


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
/*
 * Map from socket descriptor to a locked socket entry pointer and verify
 * that the address length is at least what we expect for this address type.
 */    
    socketEntryPtr = tfSocketCheckAddrLenLock( socketDescriptor,
                                               fromAddressPtr,
                                               addressLength,
                                               &errorCode
#ifdef TM_USE_BSD_DOMAIN
                                             , &af
#endif /* TM_USE_BSD_DOMAIN */
                                              );
#ifdef TM_USE_BSD_DOMAIN
    if (af == TM_BSD_FAMILY_NONINET) 
    {
        tm_assert(getpeername, socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR);
        retCode = tfBsdGetPeerName( (ttVoidPtr)socketEntryPtr, fromAddressPtr, 
                                    addressLengthPtr );
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
        if (socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR)
        {
/* errorCode set by tfSocketCheckAndLock */
            goto getpeernameFinish;
        }
        if ( fromAddressPtr == TM_SOCKADDR_NULL_PTR )
        {
            errorCode = TM_EINVAL;
            goto getpeernameFinish;
        }
/* Check to see that is connected */
        if (!(socketEntryPtr->socFlags & TM_SOCF_CONNECTED))
        {
            errorCode = TM_ENOTCONN;
            goto getpeernameFinish;
        }
    
        errorCode = TM_ENOERROR;
    
getpeernameFinish:
        retCode = tfSocketReturnAddr(socketEntryPtr, socketDescriptor,
                                     fromAddressPtr,
                                     errorCode, TM_ENOERROR, TM_ADDR_PEER_FLAG);
    }
    return retCode;
}
