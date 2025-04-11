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
 * Description: BSD Sockets Interface (tfSetIfMtu)
 *
 * Filename: trsetmtu.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trsetmtu.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2012/11/27 03:59:52JST $
 * $Author: pcarney $
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
 * Set the MTU for an interface 
 */
int tfSetIfMtu (ttUserInterface interfaceId, int ifMtu)
{
    int     errorCode;
    tt16Bit af;

    if (ifMtu >= TM_IP_MIN_MTU)
    {
        errorCode = tfValidInterface((ttDeviceEntryPtr)interfaceId);
        if (errorCode == TM_ENOERROR)
        {
            af = TM_16BIT_ZERO;
            tm_call_lock_wait(
                        &(((ttDeviceEntryPtr)interfaceId)->devLockEntry));
#ifdef TM_USE_IPV4
            ((ttDeviceEntryPtr)interfaceId)->devMtu=(tt16Bit)ifMtu;
            af = (tt16Bit)AF_INET;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
            if (ifMtu >= TM_6_PMTU_MINIMUM)
            {
                ((ttDeviceEntryPtr)interfaceId)->dev6Mtu=(tt16Bit)ifMtu;
                af = (tt16Bit)(af | AF_INET6);
            }
#endif /* TM_USE_IPV6 */
            tm_call_unlock(
                        &(((ttDeviceEntryPtr)interfaceId)->devLockEntry));
#ifdef TM_USE_IPV6
            if (af != TM_16BIT_ZERO)
#endif /* TM_USE_IPV6 */
            {
                tfRtUpdateDevMtu((ttDeviceEntryPtr)interfaceId,
                                 (tt16Bit)ifMtu, af);
            }
        }
    }
    else
    {
        errorCode = TM_EINVAL;
    }
    return(errorCode);
}
