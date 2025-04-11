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
 * Description: common functions for mobile IPv6, share by MN, CN and the 
 *              HA for testing (stub file)
 *
 * Filename: t6mip.c
 * Author: Bryan Wang
 * Date Created: 11/15/2002
 * $Source: source/stubs/mip6/t6mip.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:15JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


#include <trsocket.h>
#include <trmacro.h>

#if (defined(TM_6_USE_MIP_MN) || defined(TM_6_USE_MIP_CN))

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>


/* constant declarations and enumerations */

/* type definitions */

/* local function prototypes */

/* local variable definitions */

/* macro definitions */

void tf6MobilityHdrIncomingPacket(ttPacketPtr packetPtr)
{
    tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    return;
}

#else /* ! Mobile IPv6 enabled */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */
int tl6MipDummy = 0;

#endif /* ! Mobile IPv6 enabled */
