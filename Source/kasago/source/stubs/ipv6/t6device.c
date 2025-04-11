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
 * Description: stubs for Generic Device Send and Incoming routines for IPv6
 *
 * Filename: t6device.c
 * Author: Ed Remmell
 * Date Created:
 * $Source: source/stubs/ipv6/t6device.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:59JST $
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

int tf6ValidConfigInterface(
    ttDeviceEntryPtr checkDevEntryPtr, tt16Bit mHomeIndex )
{
    TM_UNREF_IN_ARG(checkDevEntryPtr);
    TM_UNREF_IN_ARG(mHomeIndex);

    return TM_EPROTONOSUPPORT;
}

int tf6ValidConfigAddr(tt6ConstIpAddressPtr ipAddrPtr, int prefixLen)
{
    TM_UNREF_IN_ARG(ipAddrPtr);
    TM_UNREF_IN_ARG(prefixLen);

    return TM_EPROTONOSUPPORT;
}
        
int tf6FinishOpenInterface(
    ttDeviceEntryPtr devEntryPtr, unsigned int mHomeIndex)
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(mHomeIndex);

    return TM_EPROTONOSUPPORT;
}

void tf6DelayInitSolTimer(
    ttVoidPtr       delayTimerPtr,
    ttGenericUnion  devEntryPtr,
    ttGenericUnion  mHomeIndex)
{
    TM_UNREF_IN_ARG(delayTimerPtr);
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(mHomeIndex);

    return;
}
    
#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
void tf6InsertSortedIpAddr(
    ttDeviceEntryPtr insertDevEntryPtr, tt16Bit insertMHomeIndex,
    ttSortedIpAddrCachePtr ipAddrCachePtr)
{
    TM_UNREF_IN_ARG(insertDevEntryPtr);
    TM_UNREF_IN_ARG(insertMHomeIndex);
    TM_UNREF_IN_ARG(ipAddrCachePtr);

    return;
}

ttIpAddrEntryPtr tf6LookupSortedIpAddr(
    tt6ConstIpAddressPtr searchIpAddrPtr, int scopeDevIndex,
    ttSortedIpAddrCachePtr ipAddrCachePtr)
{
    TM_UNREF_IN_ARG(searchIpAddrPtr);
    TM_UNREF_IN_ARG(scopeDevIndex);
    TM_UNREF_IN_ARG(ipAddrCachePtr);

    return (ttIpAddrEntryPtr) 0;
}
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */

int tf6Eui48SetInterfaceId(
    ttDeviceEntryPtr devEntryPtr, tt8BitPtr macAddrPtr )
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(macAddrPtr);

    return TM_EPROTONOSUPPORT;
}

int tf6SetInterfaceId(
    ttDeviceEntryPtr devEntryPtr, tt8BitPtr interfaceIdPtr )
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(interfaceIdPtr);

    return TM_EPROTONOSUPPORT;
}

#else /* ! TM_USE_IPV6 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV6 is not defined */
int tv6DeviceDummy = 0;
#endif /* ! TM_USE_IPV6 */

/***************** End Of File *****************/
