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
 * Description: Sockets Interface (tfSocketArrayWalk)
 *
 * Filename: trsocwlk.c
 * Author: Odile
 * Date Created: 08/06/99
 * $Source: source/sockapi/trsocwlk.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:33JST $
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
#ifdef TM_LOCK_NEEDED
#include <trproto.h>
#endif /* TM_LOCK_NEEDED */
#include <trglobal.h>

/*
 * tfSocketArrayWalk() function description:
 * Walk the list of sockets, and call back the user supplied call back
 * function passing the socket descriptor, and user supplied argument,
 * for each open socket until we reach the end of the list, or the
 * user returns a non zero value, whichever comes first.
 * Parameters:
 * callBackFuncPtr  User call back function
 * argPtr           User pointer argument
 * Return value:
 * TM_ENOERROR      No error
 * TM_EINVAL        callBackFuncPtr is NULL
 * non zero value   error as returned by the call back function.
 */
int tfSocketArrayWalk (ttWalkCBFuncPtr callBackFuncPtr, ttVoidPtr argPtr)
{
    ttSocketEntryPtr socketEntryPtr;
    int              errorCode;
    int              socketDesc;

    if (callBackFuncPtr == (ttWalkCBFuncPtr)0)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        errorCode = TM_ENOERROR;
/* LOCK the socket array */
        tm_call_lock_wait(&tm_context(tvSocketArrayLock));
/* LOOP through the list of sockets */
        for ( socketDesc = 0;
              (tt32Bit)socketDesc < tm_context(tvMaxNumberSockets);
              socketDesc++ )
        {
/* Socket Entry pointer */
#ifdef TM_USE_BSD_DOMAIN
            socketEntryPtr = (ttSocketEntryPtr) 
                            tm_context(tvSocketArray)[socketDesc].afsSocketPtr;
#else /* !TM_USE_BSD_DOMAIN */
            socketEntryPtr = tm_context(tvSocketArray)[socketDesc];
#endif /* !TM_USE_BSD_DOMAIN */
            if (socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR)
/* Opened socket */
            {
/* UNLOCK the socket array before calling the user call back function */
                tm_call_unlock(&tm_context(tvSocketArrayLock));
/* User call back function */
                errorCode = (*callBackFuncPtr)(socketDesc, argPtr);
                if (errorCode != TM_ENOERROR)
                {
/* The call back function returned an error (i.e TM_ENOENT) to make us stop */
                    break;
                }
/*
 * The call back function returned with no error, RELOCK the socket array
 * after having called the user call back function to get the next entry
 */
                tm_call_lock_wait(&tm_context(tvSocketArrayLock));
            }
        }
        if (errorCode == TM_ENOERROR)
/* No error case: socket array still locked, UNLOCK the socket array */
        {
            tm_call_unlock(&tm_context(tvSocketArrayLock));
        }
    }
    return errorCode;
}
