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
 * Description: internal API functions for mobile IPv6 MN (stub file)
 *
 * Filename: t6mipmn.c
 * Author: Ed Remmell 
 * Date Created: 11/15/2002
 * $Source: source/stubs/mip6mn/t6mipmn.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:18JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_6_USE_MIP_MN

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/* macro definitions */

void tf6MnInit( void )
{
    return;
}

void tf6MnStartBeaconTimer(tt32Bit advertIntervalMsec)
{
    TM_UNREF_IN_ARG(advertIntervalMsec);
}

void tf6MnStartL3MoveTimer(
    ttVoidPtr l3MoveTimerPtr,
    ttGenericUnion userParm1,
    ttGenericUnion userParm2 )
{

/* supress compiler warnings */
    TM_UNREF_IN_ARG(l3MoveTimerPtr);
    TM_UNREF_IN_ARG(userParm1);
    TM_UNREF_IN_ARG(userParm2);
}

void tf6MnResetInterface(ttDeviceEntryPtr devEntryPtr)
{
    TM_UNREF_IN_ARG(devEntryPtr);
}

int tf6MnMoveFromHome(void)
{
    return TM_6_ENOTIMPLEMENTED;
}

#ifndef TM_6_MN_DISABLE_HOME_DETECT
int tf6MnMoveToHome(void)
{
    return TM_6_ENOTIMPLEMENTED;
}
#endif /* ! TM_6_MN_DISABLE_HOME_DETECT */

int tf6MnStateMachine(
    tt6MnHaBulEntryPtr  haBulPtr,
    ttPacketPtr         packetPtr,
    tt6MnsEvent         event )
{
    TM_UNREF_IN_ARG(haBulPtr);
    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(event);
    
    return 0;
}

#ifdef TM_6_USE_MIP_RO
int tf6MnRrStateMachine(
    tt6MnRrBulEntryPtr rrBulPtr,
    ttPacketPtr packetPtr,
    tt6MnsEvent event )
{
    TM_UNREF_IN_ARG(rrBulPtr);
    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(event);

    return 0;
}

tt6MnRrBulEntryPtr tf6MnFindRrBulEntry(
    tt16Bit                     mHomeIndex,
    tt6ConstIpAddressPtr        cnIpAddrPtr )
{
    TM_UNREF_IN_ARG(mHomeIndex);
    TM_UNREF_IN_ARG(cnIpAddrPtr);
    
    return (tt6MnRrBulEntryPtr) 0;
}

tt6MnRrBulEntryPtr tf6MnAddRrBulEntry(
    tt16Bit                     mHomeIndex,
    tt6ConstIpAddressPtr        cnIpAddrPtr )
{
    TM_UNREF_IN_ARG(mHomeIndex);
    TM_UNREF_IN_ARG(cnIpAddrPtr);

    return (tt6MnRrBulEntryPtr) 0;
}
#endif /* TM_6_USE_MIP_RO */

#ifdef TM_6_USE_PREFIX_DISCOVERY
void tf6MobilePrefixAdvIncomingPacket(ttPacketPtr packetPtr)
{
    TM_UNREF_IN_ARG(packetPtr);
}
#endif /* TM_6_USE_PREFIX_DISCOVERY */

void tf6MnBuildHaoDestExtHdr(
    ttAncHdrPtr         ancHdrPtr,
    tt6IpAddressPtr     coAddrPtr)
{
    TM_UNREF_IN_ARG(ancHdrPtr);
    TM_UNREF_IN_ARG(coAddrPtr);
}

void tf6MnHaBulFailureTimer(
    ttVoidPtr failureTimerPtr,
    ttGenericUnion userParm1,
    ttGenericUnion userParm2 )
{
    TM_UNREF_IN_ARG(failureTimerPtr);
    TM_UNREF_IN_ARG(userParm1);
    TM_UNREF_IN_ARG(userParm2);
}

#else /* ! TM_6_USE_MIP_MN */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV6 is not defined */
int tl6MipMnDummy = 0;

#endif /* ! TM_6_USE_MIP_MN */
