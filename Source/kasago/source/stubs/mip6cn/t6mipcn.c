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
 * Description: functions for mobile IPv6 CN (stub file)
 *
 * Filename: t6mipcn.c
 * Author: Bryan Wang
 * Date Created: 11/15/2002
 * $Source: source/stubs/mip6cn/t6mipcn.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:16JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */
#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_6_USE_MIP_CN

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>


tt6BindingEntryPtr tf6MipGetBinding(tt6ConstIpAddressPtr homeAddrPtr)
{
    TM_UNREF_IN_ARG(homeAddrPtr);
    return TM_6_BINDING_NULL_PTR;
}

int tf6HaoOptsExtHdrIncomingPacket(
    ttPacketPtr     packetPtr,
    tt6IpAddressPtr homeAddrPtr)
{
    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(homeAddrPtr);
    
    return TM_ENOERROR;
}

void tf6MipNonceTimeOut(
    const ttVoidPtr timerBlockPtr,
    ttGenericUnion  userParm1,
    ttGenericUnion  userParm2)
{
    TM_UNREF_IN_ARG(userParm1);
    TM_UNREF_IN_ARG(userParm2);
}

void tf6MipRegenNonceKcn(int index)
{
    TM_UNREF_IN_ARG(index);
}

void tf6MipCnIcmpIncomingPacket(ttPacketPtr packetPtr)
{
    TM_UNREF_IN_ARG(packetPtr);
}

int tf6BuildRoutingHeader (
    tt8BitPtr               bufferPtr,
    tt8Bit                  type,
    tt6ConstIpAddressPtr    ipAddrPtr,
    tt8Bit                  numOfAddr)
{
    TM_UNREF_IN_ARG(bufferPtr);
    TM_UNREF_IN_ARG(type);
    TM_UNREF_IN_ARG(ipAddrPtr);
    TM_UNREF_IN_ARG(numOfAddr);

    return TM_EPROTONOSUPPORT;
}

#else /* TM_6_USE_MIP_CN */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV6 is not defined */
int tl6MipCnDummy = 0;

#endif /* TM_6_USE_MIP_CN */
