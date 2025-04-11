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
 * Description: stubs for IPv6 over IPv4 tunneling
 *
 * Filename: t6v4tunl.c
 * Author: Jason Schmidlapp
 * Date Created: 9/16/2002
 * $Source: source/stubs/ipv4/t6v4tunl.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:39JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>

#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

int tf6Ipv4TunnelSend(ttPacketPtr packetPtr)
{
    TM_UNREF_IN_ARG(packetPtr);

    return TM_EPROTONOSUPPORT;
}

void tf4TunnelIcmpIncomingError (ttPacketPtr packetPtr, tt8BitPtr origIpHdrPtr)
{
    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(origIpHdrPtr);

    return;
}

ttDeviceEntryPtr tf6AddIpv4Tunnel(int TM_FAR * errorPtr)
{
    TM_UNREF_IN_ARG(errorPtr);

    return (ttDeviceEntryPtr) 0;
}

ttDeviceEntryPtr tf6InitAutoTunnels(void)
{
    return (ttDeviceEntryPtr) 0;
}
#else /* ! (TM_USE_IPV6 && TM_USE_IPV4) */

LINT_UNUSED_HEADER

/* To allow link for builds when TM_USE_IPV6 is not defined */
int tv6Ipv4TunnelDummy = 0;
#endif /* ! (TM_USE_IPV6 && TM_USE_IPV4) */

/***************** End Of File *****************/
