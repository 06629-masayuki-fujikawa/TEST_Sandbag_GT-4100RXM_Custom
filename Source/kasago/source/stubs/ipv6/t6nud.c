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
 * Description: stubs for IPv6 Neighbor Unreachability Detection
 *
 * Filename: t6nud.c
 * Author: Bryan Wang
 * Date Created:
 * $Source: source/stubs/ipv6/t6nud.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:07JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_6_USE_NUD

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

void tf6NudUnsolicited(
        tt8Bit              msgType,
        ttIpAddressPtr      ipAddrPtr,
        tt8BitPtr           physAddrPtr,
        tt8Bit              physAddrLength,
        ttDeviceEntryPtr    devEntryPtr,
        tt8Bit              targetIsDest)
{
    TM_UNREF_IN_ARG(msgType);
    TM_UNREF_IN_ARG(ipAddrPtr);
    TM_UNREF_IN_ARG(physAddrPtr);
    TM_UNREF_IN_ARG(physAddrLength);
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(targetIsDest);

    return;
}

void tf6NudReachableTimeOut(
    ttVoidPtr           timerBlockPtr,
    ttGenericUnion      userParm1,
    ttGenericUnion      userParm2)
{
    TM_UNREF_IN_ARG(timerBlockPtr);
    TM_UNREF_IN_ARG(userParm1);
    TM_UNREF_IN_ARG(userParm2);

    return;
}

#else /* ! TM_6_USE_NUD */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV6 is not defined */
int tv6NudDummy = 0;
#endif /* ! TM_6_USE_NUD */

/***************** End Of File *****************/
