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
 * Description: stubs for IPv6 tunneling
 *
 * Filename: t6iptunl.c
 * Author: Jason Schmidlapp
 * Date Created:
 * $Source: source/stubs/ipv6/t6iptunl.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:04JST $
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

int tfConfigTunnel(ttDeviceEntryPtr                 tunnelDevPtr,
                   ttDeviceEntryPtr                 entryPtDevPtr,
                   tt16Bit                          entryPtMhomeIndex,
             const struct sockaddr_storage TM_FAR * destAddrPtr,
                   int                              prefixLen,
             const struct sockaddr_storage TM_FAR * endpointAddrPtr,
             const struct sockaddr_storage TM_FAR * srcAddrPtr,
                   tt16Bit                          flags)
{
    TM_UNREF_IN_ARG(tunnelDevPtr);
    TM_UNREF_IN_ARG(entryPtDevPtr);
    TM_UNREF_IN_ARG(entryPtMhomeIndex);
    TM_UNREF_IN_ARG(destAddrPtr);
    TM_UNREF_IN_ARG(prefixLen);
    TM_UNREF_IN_ARG(endpointAddrPtr);
    TM_UNREF_IN_ARG(srcAddrPtr);
    TM_UNREF_IN_ARG(flags);

    return TM_EPROTONOSUPPORT;
}

int tfUnConfigTunnel(ttDeviceEntryPtr tunnelDevPtr, tt16Bit flags)
{
    TM_UNREF_IN_ARG(tunnelDevPtr);
    TM_UNREF_IN_ARG(flags);

    return TM_EPROTONOSUPPORT;
}

int tfTunnelSendStub(ttPacketPtr packetPtr)
{
    packetPtr=packetPtr;
    return TM_LL_OKAY;
}
#else /* ! TM_USE_IPV6 */

LINT_UNUSED_HEADER

/* To allow link for builds when TM_USE_IPV6 is not defined */
int tv6IpTunnelDummy = 0;
#endif /* ! TM_USE_IPV6 */

/***************** End Of File *****************/
