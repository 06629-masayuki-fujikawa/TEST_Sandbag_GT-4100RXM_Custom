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
 * Description: BSD Sockets Interface (tfAddDefaultGateway)
 *
 * Filename: traddgw.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/traddgw.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:42:40JST $
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
#include <trglobal.h>

/*
 * tfAddDefaultGateway function description:
 * IPV4 only function
 * Called from the socket interface to add a default gateway 
 * If interface is a Pt2Pt (SLIP or PPP) interface, ignore the gateway
 * IP address given by the user, and use the Pt2Pt peer IP address instead.
 * Parameters:
 * InterfaceHandle:  pointer to the device entry
 * GatewayIpAddress: IP address of the default gateway
 * Return values:
 * See tfRtAddEntry()
 */
int tfAddDefaultGateway (ttUserInterface interfaceHandle,
                         ttUserIpAddress gatewayIpAddress
#ifdef TM_USE_STRONG_ESL
                       , int             hops
#endif /* TM_USE_STRONG_ESL */
                        )
{
    ttDeviceEntryPtr    devEntryPtr;
    int                 errorCode;
#if ((!defined(TM_USE_STRONG_ESL)) && (!defined(TM_USE_IPV6)))
    int                 hops;
#endif /* !TM_USE_STRONG_ESL && !TM_USE_IPV6 */
#ifndef TM_SINGLE_INTERFACE_HOME
    tt16Bit             mhIndex;
#endif /* !TM_SINGLE_INTERFACE_HOME */
    
    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
#if ((!defined(TM_USE_STRONG_ESL)) && (!defined(TM_USE_IPV6)))
    hops = 1;
#endif /* !TM_USE_STRONG_ESL && !TM_USE_IPV6 */
/* Check that the interface is valid and that is is configured */
#ifdef TM_SINGLE_INTERFACE_HOME
    errorCode = tfValidConfigInterface( devEntryPtr,
                                        TM_8BIT_ZERO );
#else /* !TM_SINGLE_INTERFACE_HOME */
/* Make sure the interface is valid */
    errorCode = tfValidInterface(devEntryPtr);
    if (errorCode == TM_ENOERROR)
    {
/*
 * NOTE TM_DEV_IP_CONFIG is INPUT parameter for tfMhomeAnyConf(). mhIndex
 * will be set by tfMhomeAnyConf if successful.
 */
        mhIndex = TM_DEV_IP_CONFIG;
/* Make sure that it is configured on a mhome */
        errorCode = tfMhomeAnyConf(devEntryPtr, 0, &mhIndex);
    }
#endif /* !TM_SINGLE_INTERFACE_HOME */
    if (errorCode == TM_ENOERROR)
    {
        if (devEntryPtr == tm_context(tvLoopbackDevPtr))
        {
/* No gateway can't be added to the loop back interface */
            errorCode = TM_EINVAL;
        }
        else
        {
            if ( tm_ll_is_pt2pt(devEntryPtr) )
            {
/* PPP always use the remote side's IP address as the default gateway */
                tm_call_lock_wait( &(devEntryPtr->devLockEntry) );
                tm_ip_copy( tfDeviceDestIpAddress(devEntryPtr),
                            gatewayIpAddress);
                tm_call_unlock( &(devEntryPtr->devLockEntry) );
            }
            if (tm_ip_zero(gatewayIpAddress))
            {
                errorCode = TM_EINVAL;
            }
            else
            {
/*
 * All validation passed, add the default gateway into the routing table.
 */
#ifdef TM_USE_IPV6
/* 
* In dual IP layer mode, we must always have an IPv4 default gateway entry
* in the routing tree, even if it isn't configured. Replace the existing
* default gateway, if it is not configured.
*/
                errorCode = tf4RtAddDefGw(devEntryPtr, gatewayIpAddress
#ifdef TM_USE_STRONG_ESL
                                        , hops
#endif /* TM_USE_STRONG_ESL */
                                         );
#else /* ! dual IP layer */
                errorCode = tf4RtAddRoute( devEntryPtr, TM_IP_ZERO, TM_IP_ZERO,
                                            gatewayIpAddress, TM_16BIT_ZERO,
/* 
 * Note that the add route function will set the mhome index from
 * the matching local route. So the passed value is irrelevant.
 */
                                            TM_16BIT_ZERO, hops,
                                            TM_RTE_INF,
                                            TM_RTE_STATIC | TM_RTE_INDIRECT,
                                            TM_8BIT_YES );
#endif /* ! dual IP layer */
            }
        }
    }
    return errorCode;
}
#else /* !TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

int addgwDummy = 0;

#endif /* !TM_USE_IPV4 */
