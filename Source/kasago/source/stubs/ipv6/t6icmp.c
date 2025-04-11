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
 * Description: stubs for ICMPv6
 *
 * Filename: t6icmp.c
 * Author: Ed Remmell
 * Date Created: 5/20/2002
 * $Source: source/stubs/ipv6/t6icmp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:01JST $
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

void tf6IcmpIncomingPacket(
    ttPacketPtr packetPtr )
{
    TM_UNREF_IN_ARG(packetPtr);

    return;
}

void tf6IcmpErrPacket(
    ttPacketPtr origPacketPtr,
    tt8Bit errType,
    tt8Bit errCode,
    tt32Bit errParam )
{
    TM_UNREF_IN_ARG(origPacketPtr);
    TM_UNREF_IN_ARG(errType);
    TM_UNREF_IN_ARG(errCode);
    TM_UNREF_IN_ARG(errParam);

    return;
}

void tf6IcmpEchoReplyPacket(
    ttPacketPtr echoPacketPtr )
{
    TM_UNREF_IN_ARG(echoPacketPtr);

    return;
}

int tf6RawSendPacket(
    ttSocketEntryPtr socketPtr, ttPacketPtr packetPtr )
{
    TM_UNREF_IN_ARG(socketPtr);
    TM_UNREF_IN_ARG(packetPtr);

    return TM_EPROTONOSUPPORT;
}

#else /* ! TM_USE_IPV6 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV6 is not defined */
int tv6IcmpDummy = 0;
#endif /* ! TM_USE_IPV6 */

/***************** End Of File *****************/
