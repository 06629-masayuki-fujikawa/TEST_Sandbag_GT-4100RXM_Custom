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
 * Description: Check that an IP address is not in use by another host
 *              on a given network.
 *
 * Filename: trautoip.c
 * Author: Odile
 * Date Created: 02/27/01
 * $Source: source/stubs/autoip/trautoip.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:27JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h> /* For TM_USE_AUTO_IP */
#include <trmacro.h> /* For LINT_UNUSED_HEADER */

#ifdef TM_USE_AUTO_IP

#include <trtype.h>

/*
 * tfAutoIpPickIpAddress
 * Return value
 * zero                failure
 */
ttUserIpAddress tfAutoIPPickIpAddress (void)
{
    return TM_IP_ZERO;
}

/*
 * tfUserStartArpSend
 * Returns
 * Value            Meaning
 * TM_ENOENT        The user is not checking the IP address for collision
 *                  detection.
 */

int tfUserStartArpSend ( ttUserInterface interfaceHandle,
                         ttUserIpAddress ipAddress,
                         int             numberArpProbes,
                         ttUser32Bit     arpProbeInterval,
                         ttUser32Bit     timeout )
{
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(ipAddress);
    TM_UNREF_IN_ARG(numberArpProbes);
    TM_UNREF_IN_ARG(arpProbeInterval);
    TM_UNREF_IN_ARG(timeout);
    return TM_ENOENT;
}

#else /* !TM_USE_AUTO_IP */
#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */
int tvAutoIpsDummy = 0;
#endif /* TM_USE_AUTO_IP */
