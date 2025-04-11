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
 * Description: stubs for IPv6 functions
 *
 * Filename: t6ip.c
 * Author: Ed Remmell
 * Date Created: 3/27/02
 * $Source: source/stubs/ipv6/t6ip.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:02JST $
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

void tf6PrefixLenToNetmask(
    int prefixLen, tt6IpAddressPtr netmaskPtr)
{
    TM_UNREF_IN_ARG(prefixLen);
    TM_UNREF_IN_ARG(netmaskPtr);

    return;
}

void tf6IpIncomingPacket(ttPacketPtr packetPtr
#ifdef TM_USE_IPSEC_TASK
                        ,(ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
                        )
{
    TM_UNREF_IN_ARG(packetPtr);

    return;
}

int tf6IpMcastToLan(tt6ConstIpAddressPtr ipAddrPtr,
                    ttDeviceEntryPtr     devEntryPtr,
                    tt8BitPtr            linkAddrPtr)
{
    TM_UNREF_IN_ARG(ipAddrPtr);
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(linkAddrPtr);

    return TM_EPROTONOSUPPORT;
}

int tf6MhomeAddrMatch (ttDeviceEntryPtr devPtr,
                       tt6ConstIpAddressPtr ipAddressPtr,
                       tt16BitPtr ifaceMhomePtr)
{
    TM_UNREF_IN_ARG(devPtr);
    TM_UNREF_IN_ARG(ipAddressPtr);
    TM_UNREF_IN_ARG(ifaceMhomePtr);

    return TM_EPROTONOSUPPORT;
}

ttDeviceEntryPtr tf6IfaceMatch (tt6ConstIpAddressPtr ipAddressPtr,
                                tt6MhomeFunctPtr mhomeMatchFuncPtr,
                                tt16BitPtr       ifaceMhomePtr)
{
    TM_UNREF_IN_ARG(ipAddressPtr);
    TM_UNREF_IN_ARG(mhomeMatchFuncPtr);
    TM_UNREF_IN_ARG(ifaceMhomePtr);

    return (ttDeviceEntryPtr) 0;
}

int tf6IpDestToPacket(
    ttPacketPtr packetPtr, tt6ConstIpAddressPtr srcIpAddrPtr,
    tt6ConstIpAddressPtr destIpAddrPtr, ttRtCacheEntryPtr rtcPtr,
    tt6IpHeaderPtr iph6Ptr)
{
    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(srcIpAddrPtr);
    TM_UNREF_IN_ARG(destIpAddrPtr);
    TM_UNREF_IN_ARG(rtcPtr);
    TM_UNREF_IN_ARG(iph6Ptr);

    return TM_EPROTONOSUPPORT;
}

int tf6IpSendPacket(
    ttPacketPtr packetPtr,
    ttLockEntryPtr lockEntryPtr)
{
    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(lockEntryPtr);

    return TM_EPROTONOSUPPORT;
}

int tf6IpCacheRte(
    ttRtCacheEntryPtr rtcPtr, tt6ConstIpAddressPtr ipDestAddrPtr)
{
    TM_UNREF_IN_ARG(rtcPtr);
    TM_UNREF_IN_ARG(ipDestAddrPtr);

    return TM_EPROTONOSUPPORT;
}

int tf6SelectSourceAddress(
    ttDeviceEntryPtr devEntryPtr,
    tt6IpAddressPtr srcIpAddrPtr,
    tt16BitPtr srcMHomeIndexPtr,
    tt6ConstIpAddressPtr destIpAddrPtr,
    tt8BitPtr destIpAddrTypePtr,
    tt8Bit flags)
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(srcIpAddrPtr);
    TM_UNREF_IN_ARG(srcMHomeIndexPtr);
    TM_UNREF_IN_ARG(destIpAddrPtr);
    TM_UNREF_IN_ARG(destIpAddrTypePtr);
    TM_UNREF_IN_ARG(flags);

    return TM_EPROTONOSUPPORT;
}

void tf6UpdateCacheAddress(
    ttDeviceEntryPtr devEntryPtr,
    tt16Bit mhomeIndex )
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(mhomeIndex);

    return;
}

#else /* ! TM_USE_IPV6 */

LINT_UNUSED_HEADER

/* To allow link for builds when TM_USE_IPV6 is not defined */
int tv6IpDummy = 0;
#endif /* ! TM_USE_IPV6 */

/***************** End Of File *****************/
