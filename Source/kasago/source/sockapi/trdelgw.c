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
 * Description: BSD Sockets Interface (tfDelDefaultGateway)
 *
 * Filename: trdelgw.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trdelgw.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:42:55JST $
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
#ifdef TM_USE_IPV6
#include <trglobal.h>
#endif /* dual IP layer */

/*
 * IPV4 only function.
 * tfDelDefaultGateway function description:
 * Called from the socket interface to delete the default gateway
 * Parameters:
 * gatewayIpAddress: default gateway Ip address
 * Return values:
 * See tfDelRoute()
 *
 */
int tfDelDefaultGateway (
#ifdef TM_USE_STRONG_ESL
                         ttUserInterface  interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                         ttUserIpAddress  gatewayIpAddress)
{
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr    devEntryPtr;
#endif /* TM_USE_STRONG_ESL */ 
    int                 errorCode;

#ifdef TM_USE_STRONG_ESL
    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;     
    
    if ( tfValidInterface(devEntryPtr) != TM_ENOERROR) 
    {
        errorCode = TM_EINVAL;
    }
    else
#endif /* TM_USE_STRONG_ESL */ 
    {
        if (tm_ip_not_zero((tt4IpAddress)gatewayIpAddress))
        {
#ifdef TM_USE_IPV6
/* in dual IP layer mode, we must always have an IPv4 default gateway entry
   in the routing tree, even if it isn't configured. */
            errorCode = tf4RtDelDefGw(
#ifdef TM_USE_STRONG_ESL
                                       devEntryPtr,
#endif /* TM_USE_STRONG_ESL */
                                       gatewayIpAddress);
#else /* ! dual IP layer */
            errorCode = tf4RtDelRoute(
#ifdef TM_USE_STRONG_ESL
                                       devEntryPtr,
#endif /* TM_USE_STRONG_ESL */
                                       (tt4IpAddress)gatewayIpAddress,
                                       TM_IP_ZERO,
                                       TM_RTE_STATIC);
#endif /* ! dual IP layer */
        }
        else
        {
            errorCode = TM_EINVAL;
        }
    }
    return (errorCode);
}

#else /* !TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

int delgwDummy = 0;

#endif /* !TM_USE_IPV4 */
