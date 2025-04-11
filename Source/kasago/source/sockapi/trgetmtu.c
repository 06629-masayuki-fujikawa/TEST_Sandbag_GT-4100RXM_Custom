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
 * Description: BSD Sockets Interface (tfGetIfMtu)
 *
 * Filename: trgetmtu.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trgetmtu.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:18JST $
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
#ifdef TM_LOCK_NEEDED
#include <trglobal.h>
#endif /* TM_LOCK_NEEDED */

/* 
 * Get the MTU for an interface 
 */
int tfGetIfMtu (ttUserInterface interfaceId, int TM_FAR *ifMtuPtr)
{
    int errorCode;

    if (ifMtuPtr != (int TM_FAR *)0) 
    {
        errorCode = tfValidInterface((ttDeviceEntryPtr)interfaceId);
        if (errorCode == TM_ENOERROR)
        {
            tm_call_lock_wait(
                          &(((ttDeviceEntryPtr)interfaceId)->devLockEntry));
#ifdef TM_USE_IPV4
            *ifMtuPtr=(int)(((ttDeviceEntryPtr)interfaceId)->devMtu);
#else /* !TM_USE_IPV4 */
            *ifMtuPtr=(int)(((ttDeviceEntryPtr)interfaceId)->dev6Mtu);
#endif /* !TM_USE_IPV4 */
            tm_call_unlock(&(((ttDeviceEntryPtr)interfaceId)->devLockEntry));
        }
    }
    else
    {
        errorCode=TM_EINVAL;
    }
    return(errorCode);
}
