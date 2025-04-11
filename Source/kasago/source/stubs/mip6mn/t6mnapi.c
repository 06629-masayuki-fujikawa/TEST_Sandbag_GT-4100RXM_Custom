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
 * Description: Public APIs for Mobile IPv6 Mobile Node (stub file)
 *
 * Filename: t6mnapi.c
 * Author: Ed Remmell
 * Date Created:
 * $Source: source/stubs/mip6mn/t6mnapi.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:19JST $
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

tt6MnStatus tf6MnGetStatus( void )
{
    return TM_6_MN_HOME_NETWORK;
}

int tf6MnMoveNotify(
    ttUserInterface     mobileIfaceHandle,
    tt6MnMoveType       moveType,
    int                 flags )
{
    TM_UNREF_IN_ARG(mobileIfaceHandle);
    TM_UNREF_IN_ARG(moveType);
    TM_UNREF_IN_ARG(flags);

    return TM_6_ENOTIMPLEMENTED;
}

int tf6MnRegisterBinding(
    const struct sockaddr_storage TM_FAR *      homeAddrPtr,
    const struct sockaddr_storage TM_FAR *      careOfAddrPtr,
    int                                         maxHaBindingLifetimeSec,
    tt6MnBindingNotifyFuncPtr                   mnBindingNotifyFuncPtr,
    int                                         timeoutSec,
    int                                         flags )
{
    TM_UNREF_IN_ARG(homeAddrPtr);
    TM_UNREF_IN_ARG(careOfAddrPtr);
    TM_UNREF_IN_ARG(maxHaBindingLifetimeSec);
    TM_UNREF_IN_ARG(mnBindingNotifyFuncPtr);
    TM_UNREF_IN_ARG(timeoutSec);
    TM_UNREF_IN_ARG(flags);

    return TM_6_ENOTIMPLEMENTED;
}

ttUserInterface tf6MnStartMobileIp(
    ttUserInterface                             mobileIfaceHandle,
    const ttUser8BitPtr                         virtHomeEui64IdPtr,
    const struct sockaddr_storage TM_FAR *      homeAgentAddrPtr,
    const struct sockaddr_storage TM_FAR *      homePrefixPtr,
    int                                         homePrefixLen,
    tt6DevAddrNotifyFuncPtr                     virtHomeAddrNotifyPtr,
    tt6MnNotifyFuncPtr                          mnNotifyFuncPtr,
    int                                         flags,
    int TM_FAR *                                errorCodePtr )
{
    TM_UNREF_IN_ARG(mobileIfaceHandle);
    TM_UNREF_IN_ARG(homeAgentAddrPtr);
    TM_UNREF_IN_ARG(homePrefixPtr);
    TM_UNREF_IN_ARG(homePrefixLen);
    TM_UNREF_IN_ARG(virtHomeAddrNotifyPtr);
    TM_UNREF_IN_ARG(mnNotifyFuncPtr);
    TM_UNREF_IN_ARG(flags);
    TM_UNREF_IN_ARG(errorCodePtr);

    return (ttUserInterface) 0;
}

int tf6MnStopMobileIp( int flags )
{
    TM_UNREF_IN_ARG(flags);

    return TM_6_ENOTIMPLEMENTED;
}

int tf6MnGetHomeAgentAddress(
    const struct sockaddr_storage * homePrefixPtr,
    int                     homePrefixLen,
    struct sockaddr_storage *       homeAgentAddrPtr,
    int                     addrBufferLen,
    int                     flags)  /* TM_BLOCKING_ON */
{
    TM_UNREF_IN_ARG(homePrefixPtr);
    TM_UNREF_IN_ARG(homePrefixLen);
    TM_UNREF_IN_ARG(homeAgentAddrPtr);
    TM_UNREF_IN_ARG(addrBufferLen);
    TM_UNREF_IN_ARG(flags);

    return 0;
}

void tf6MnDhaadReplyIncomingPacket(ttPacketPtr packetPtr)
{
    TM_UNREF_IN_ARG(packetPtr);
}

#else /* TM_6_USE_MIP_MN */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_6_USE_MIP_MN is not defined */
int tl6MnApiDummy = 0;

#endif /* TM_6_USE_MIP_MN */
