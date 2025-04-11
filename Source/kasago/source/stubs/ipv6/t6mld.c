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
 * Description: stubs for Multicast Listener Discovery for IPv6
 *
 * Filename: t6mld.c
 * Author: Ed Remmell
 * Date Created: 5/20/2002
 * $Source: source/stubs/ipv6/t6mld.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:05JST $
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

#ifdef TM_6_USE_MLD
void tf6MldIncomingPacket(
    ttPacketPtr packetPtr, tt8Bit msgType )
{
    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(msgType);

    return;
}

int tf6MldSetSockOpt(
    ttSocketEntryPtr socketEntryPtr,
    int optionName,
    ttConstCharPtr optionValuePtr,
    int optionLen )
{
    TM_UNREF_IN_ARG(socketEntryPtr);
    TM_UNREF_IN_ARG(optionName);
    TM_UNREF_IN_ARG(optionValuePtr);
    TM_UNREF_IN_ARG(optionLen);

    return TM_EPROTONOSUPPORT;
}

int tf6MldGetSockOpt(
    ttSocketEntryPtr socketEntryPtr,
    int optionName,
    ttConstCharPtr optionValuePtr,
    ttIntPtr optionLenPtr )
{
    TM_UNREF_IN_ARG(socketEntryPtr);
    TM_UNREF_IN_ARG(optionName);
    TM_UNREF_IN_ARG(optionValuePtr);
    TM_UNREF_IN_ARG(optionLenPtr);

    return TM_EPROTONOSUPPORT;
}

int tf6MldJoinGroup(
    tt8Bit           socketCall,
    ttDeviceEntryPtr devEntryPtr,
    tt6IpAddressPtr  mcastGroupAddrPtr )
{
    TM_UNREF_IN_ARG(socketCall);
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(mcastGroupAddrPtr);

    return TM_EPROTONOSUPPORT;
}

int tf6MldLeaveGroup(
    tt8Bit              socketCall,
    ttDeviceEntryPtr    devEntryPtr,
    tt6IpAddressPtr     mcastGroupAddrPtr )
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(mcastGroupAddrPtr);

    return TM_EPROTONOSUPPORT;
}

void tf6MldSocketCachePurge(ttSocketEntryPtr socketEntryPtr)
{
    TM_UNREF_IN_ARG(socketEntryPtr);
    return;
}

tt8Bit tf6MldSocketCacheMatch(
    ttSocketEntryPtr socketEntryPtr,
    ttDeviceEntryPtr devEntryPtr,
    tt6IpAddressPtr  mcastGroupAddrPtr )
{
    TM_UNREF_IN_ARG(socketEntryPtr);
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(mcastGroupAddrPtr);
    return;
}

#endif /* TM_6_USE_MLD */

int tf6MldIsMember(
    ttDeviceEntryPtr devEntryPtr,
    tt6IpAddressPtr mcastGroupAddrPtr,
    tt16BitPtr mhomeIndexPtr )
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(mcastGroupAddrPtr);
    TM_UNREF_IN_ARG(mhomeIndexPtr);

    return TM_EPROTONOSUPPORT;
}

#else /* ! TM_USE_IPV6 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV6 is not defined */
int tv6MldDummy = 0;
#endif /* ! TM_USE_IPV6 */

/***************** End Of File *****************/
