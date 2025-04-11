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
 * Description: BSD Sockets Interface (tfGetDefaultGateway)
 *
 * Filename: trgetgw.c
 * Author: Odile
 * Date Created: 03/22/99
 * $Source: source/sockapi/trgetgw.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:15JST $
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

/*
 * Called from the socket interface to get the default gateway IP address
 * in network byte order.
 * Parameters:
 * gatewayIpAddressPtr: pointer to gateway IP address to store result into.
 * Return values:
 * TM_ENOERROR  success
 * TM_EINVAL    bad parameter
 * TM_ENOENT    no default gateway
 *
 */
int tfGetDefaultGateway (
#ifdef TM_USE_STRONG_ESL
                         ttUserInterface          interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                         ttUserIpAddress TM_FAR * gatewayIpAddressPtr)
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
        if ( gatewayIpAddressPtr != (ttUserIpAddress TM_FAR *)0 )
        {
            errorCode = tf4RtGetDefault(
#ifdef TM_USE_STRONG_ESL
                                         devEntryPtr,
#endif /* TM_USE_STRONG_ESL */
                                         (tt4IpAddressPtr)gatewayIpAddressPtr,
                                         (ttIntPtr)0);
        }
        else
        {
            errorCode = TM_EINVAL;
        }
    }
    return (errorCode);
}

#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4GetgwDummy = 0;
#endif /* ! TM_USE_IPV4 */
