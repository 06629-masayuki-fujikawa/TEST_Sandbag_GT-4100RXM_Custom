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
 * Description: tfFinishOpenInterface(). Finish an Interface configuration
 *              when user knows its IP address/IP netmask.
 *
 * Filename: trfopnif.c
 * Author: Odile
 * Date Created: 02/10/00
 * $Source: source/sockapi/trfopnif.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:08JST $
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

/*
 * tfFinishConfigInterface function description.
 * Finish opening an Interface that the user had started to open with
 * the TM_DEV_IP_USER_BOOT flag. That flag caused the Treck stack not
 * to store the IP address/netmask in the routing table, leaving
 * the interface in a half configured state. In this function,
 * we will attempt to insert the ipaddress/netmask into the routing table.
 *
 * Parameters
 * Parameter            Description
 * interfaceHandle      Interface handle as returned by tfAddInterface
 * ipAddress            IP address to configure the interface at multihome
 *                      index 0 with
 * ipNetmask            IP netmask of the IP address to configure.
 * mhomeIndex           Multi home index
 *
 * Returns
 * Value                Meaning
 * TM_ENOBUFS           Not enough memory to complete the operation
 * TM_EINVAL            Bad parameter
 * TM_EADDRNOTAVAIL     Addempt to configure the device with a broadcast
 *                      address
 * TM_EALREADY          IP address/netmask already in the routing table
 * TM_EPERM             User did not call tfOpenInterface() or
 *                      tfConfigInterface() with TM_DEV_IP_USER_BOOT.
 */
int tfFinishConfigInterface ( ttUserInterface interfaceHandle,
                              ttUserIpAddress ipAddress,
                              ttUserIpAddress ipNetMask,
                              int             mhomeIndex )
{
    ttDeviceEntryPtr devEntryPtr;
    int              errorCode;

    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;

    errorCode = tfValidInterface( devEntryPtr );
    if ( ( errorCode == TM_ENOERROR )
         && (mhomeIndex < TM_MAX_IPS_PER_IF) )
    {
        errorCode = TM_EINVAL;
        tm_call_lock_wait(&(devEntryPtr->devLockEntry));
        if ( tm_8bit_one_bit_set(tm_ip_dev_conf_flag(devEntryPtr, mhomeIndex),
                                 TM_DEV_IP_USER_BOOT) )
        {
            if (tm_8bit_bits_not_set(
                                tm_ip_dev_conf_flag(devEntryPtr, mhomeIndex),
                                (TM_DEV_IP_CONFIG) ) )
            {
                tm_ip_copy( ipAddress,
                            tm_ip_dev_addr(devEntryPtr, mhomeIndex) );
                tm_ip_copy( ipNetMask,
                            tm_ip_dev_mask(devEntryPtr, mhomeIndex) );
                tm_4_ip_dev_prefixLen(devEntryPtr, mhomeIndex) = 
                        (tt8Bit) tf4NetmaskToPrefixLen(ipNetMask);
                errorCode = tfDeviceStart( (ttDeviceEntryPtr) interfaceHandle, 
                                           (tt16Bit)mhomeIndex,
                                           PF_INET);
            }
            else
            {
                errorCode = TM_EALREADY;
            }
        }
        else
        {
            errorCode = TM_EPERM;
        }
        tm_call_unlock(&(devEntryPtr->devLockEntry));
    }
    return errorCode;
}

int tfFinishOpenInterface ( ttUserInterface interfaceHandle,
                            ttUserIpAddress ipAddress,
                            ttUserIpAddress ipNetMask)
{
/* Finish config Interface at multi-home index 0 */
    return tfFinishConfigInterface(interfaceHandle, ipAddress, ipNetMask, 0);
}

#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4FopnifDummy = 0;
#endif /* ! TM_USE_IPV4 */
