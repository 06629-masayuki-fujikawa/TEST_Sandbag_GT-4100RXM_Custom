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
 * Description: stubs for Stateless address auto-configuration for IPv6
 *
 * Filename: t6auto.c
 * Author: Ed Remmell
 * Date Created: 5/20/2002
 * $Source: source/stubs/ipv6/t6auto.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:58JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_IPV6

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

int tf6ConfigInterfaceId(
    ttDeviceEntryPtr devEntryPtr )
{
    TM_UNREF_IN_ARG(devEntryPtr);

    return TM_EPROTONOSUPPORT;
} 

int tf6UnConfigInterfaceId(
    ttDeviceEntryPtr devEntryPtr )
{
    TM_UNREF_IN_ARG(devEntryPtr);

    return TM_EPROTONOSUPPORT;
}

int tf6DadConfigAddr(
    ttDeviceEntryPtr devEntryPtr,
    tt16Bit multiHomeIndex,
    tt6IpAddressPtr ipAddrPtr,
    tt8Bit prefixLen )
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(multiHomeIndex);
    TM_UNREF_IN_ARG(ipAddrPtr);
    TM_UNREF_IN_ARG(prefixLen);

    return TM_EPROTONOSUPPORT;
}

#ifdef TM_6_USE_PREFIX_DISCOVERY
int tf6DadAutoConfigPrefix(
    ttDeviceEntryPtr devEntryPtr,
    tt6IpAddressPtr prefixPtr )
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(prefixPtr);

    return TM_EPROTONOSUPPORT;
}
#endif /* TM_6_USE_PREFIX_DISCOVERY */

void tf6DadConfigComplete(
    ttDeviceEntryPtr devEntryPtr,
    tt16Bit multiHomeIndex )
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(multiHomeIndex);

    return;
}

#ifdef TM_6_USE_DAD
tt6DadEntryPtr tf6DadFindEntry(
    ttDeviceEntryPtr devEntryPtr,
    tt6ConstIpAddressPtr targetAddrPtr )
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(targetAddrPtr);

    return (tt6DadEntryPtr) 0;
}

void tf6DadFailed(
    ttDeviceEntryPtr devEntryPtr,
    tt16Bit multiHomeIndex,
    tt6DadEntryPtr dadEntryPtr )
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(multiHomeIndex);
    TM_UNREF_IN_ARG(dadEntryPtr);

    return;
}

void tf6DadDupDetected(
    ttDeviceEntryPtr devEntryPtr,
    tt16Bit multiHomeIndex )
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(multiHomeIndex);

    return;
} 
#endif /* TM_6_USE_DAD */

#ifdef TM_6_USE_PREFIX_DISCOVERY
void tf6RtInvalidatePrefixTimer(
    ttVoidPtr invalidateTimerPtr,
    ttGenericUnion userParm1,
    ttGenericUnion userParm2 )
{
    TM_UNREF_IN_ARG(invalidateTimerPtr);
    TM_UNREF_IN_ARG(userParm1);
    TM_UNREF_IN_ARG(userParm2);

    return;
}

void tf6RtDeprecatePrefixTimer(
    ttVoidPtr deprecateTimerPtr,
    ttGenericUnion userParm1,
    ttGenericUnion userParm2 )
{
    TM_UNREF_IN_ARG(deprecateTimerPtr);
    TM_UNREF_IN_ARG(userParm1);
    TM_UNREF_IN_ARG(userParm2);

    return;
}

void tf6RtUnDeprecatePrefixTimer(
    ttVoidPtr unDeprecateTimerPtr,
    ttGenericUnion userParm1,
    ttGenericUnion userParm2 )
{
    TM_UNREF_IN_ARG(unDeprecateTimerPtr);
    TM_UNREF_IN_ARG(userParm1);
    TM_UNREF_IN_ARG(userParm2);

    return;
}
#endif /* TM_6_USE_PREFIX_DISCOVERY */

#else /* ! TM_USE_IPV6 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV6 is not defined */
int tv6AutoDummy = 0;
#endif /* ! TM_USE_IPV6 */

/***************** End Of File *****************/
