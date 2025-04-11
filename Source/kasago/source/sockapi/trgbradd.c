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
 * Description: BSD Sockets Interface (tfGetBroadcastAddress)
 *
 * Filename: trgbradd.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trgbradd.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:12JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_IPV4

#include <trtype.h>
#include <trproto.h>
#ifdef TM_LOCK_NEEDED
#include <trglobal.h>
#endif /* TM_LOCK_NEEDED */

/* Get the Broadcast address of an interface */
int tfGetBroadcastAddress(ttUserInterface         interfaceId,
                          ttUserIpAddress TM_FAR  *ifBroadcastAddressPtr,
                          unsigned char           mHomeIndex)
{
    ttDeviceEntryPtr    devPtr;
    tt4IpAddress        netMask;
    tt4IpAddress        ipAddress;
    int                 errorCode;

    devPtr = (ttDeviceEntryPtr)interfaceId;
/* Check ifBroadCastAddressPtr parameter */
    if (ifBroadcastAddressPtr != (ttUserIpAddress TM_FAR *)0)
    {
/* Check interface ID parameter */
        errorCode = tfValidConfigInterface(devPtr, (tt16Bit) mHomeIndex);
        if (errorCode == TM_ENOERROR)
        {
            tm_call_lock_wait(&(devPtr->devLockEntry));
            if (tm_4_ll_is_lan(devPtr))
            {
                tm_ip_copy( tm_ip_dev_dbroad_addr(devPtr, mHomeIndex),
                            *ifBroadcastAddressPtr );
            }
            else
            {
                tm_ip_copy(tm_ip_dev_mask(devPtr, mHomeIndex), netMask);
                tm_ip_copy(tm_ip_dev_addr(devPtr, mHomeIndex), ipAddress);
                tm_ip_copy(tm_ip_bld_d_broadcast(((tt4IpAddress)ipAddress),
                                                 ((tt4IpAddress)netMask)),
                            *ifBroadcastAddressPtr );
            }
            tm_call_unlock(&(devPtr->devLockEntry));
        }
    }
    else
    {
        errorCode = TM_EINVAL;
    }
    return (errorCode);
}

#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4GbraddDummy = 0;
#endif /* ! TM_USE_IPV4 */
