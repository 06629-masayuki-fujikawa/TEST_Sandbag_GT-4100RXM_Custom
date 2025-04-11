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
 * Description: stubs for SLIP Link Layer
 *
 * Filename: trslip.c
 * Author: Jason
 * Date Created: 7/1/98
 * $Source: source/stubs/ipv4/trslip.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:55JST $
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

ttUserLinkLayer tfUseSlip (void)
{
    return (ttUserLinkLayer) 0;
}


int tfSlipVectInit (ttDeviceEntryPtr devEntryPtr)
{
    TM_UNREF_IN_ARG(devEntryPtr);

    return TM_EPROTONOSUPPORT;
}

#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4SlipDummy = 0;
#endif /* ! TM_USE_IPV4 */
