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
 * Description: functions for mobile IPv6 HA, this implementation of the HA is 
 *              for testing of our MN and CN only, not an complete
 *              implementation. (stub file)
 *
 * Filename: t6mipha.c
 * Author: Bryan Wang
 * Date Created: 11/15/2002
 * $Source: source/stubs/mip6ha/t6mipha.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:17JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_6_USE_MIP_HA

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/* Functions specific to home agent comes here */

int tf6MipEnableHa(ttUser8Bit isHa)
{
    TM_UNREF_IN_ARG(isHa);
    return TM_6_ENOTIMPLEMENTED;
}

void tf6MnDhaadRequestIncomingPacket(ttPacketPtr packetPtr)
{
    TM_UNREF_IN_ARG(packetPtr);
}

void tf6MobilePrefixSolIncomingPacket(ttPacketPtr packetPtr)
{
    TM_UNREF_IN_ARG(packetPtr);
}

#else /* TM_6_USE_MIP_HA */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */
int tl6MipHaDummy = 0;


#endif /* TM_6_USE_MIP_HA */
