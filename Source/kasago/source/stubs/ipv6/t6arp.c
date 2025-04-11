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
 * Description:
 * stubs for IPv6 address resolution
 *
 * Filename: t6arp.c
 * Author: Jason Schmidlapp
 * Date Created: 4/20/2002
 * $Source: source/stubs/ipv6/t6arp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:57JST $
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

ttResEntryPtr tf6NdResolveSearch(ttDeviceEntryPtr devEntryPtr,
                                 tt6IpAddressPtr  ipAddressPtr)
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(ipAddressPtr);

    return (ttResEntryPtr) 0;
}

void tf6NdResEntryRemove(ttDeviceEntryPtr devEntryPtr,
                         ttResEntryPtr    resEntryPtr,
                         ttPacketPtr      packetPtr)
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(resEntryPtr);
    TM_UNREF_IN_ARG(packetPtr);

    return; 
}

void tf6NdAdvertIncoming(ttPacketPtr packetPtr)
{
    TM_UNREF_IN_ARG(packetPtr);

    return;
}

int tf6NdResolveAddress(ttPacketPtr packetPtr)
{
    TM_UNREF_IN_ARG(packetPtr);

    return TM_EPROTONOSUPPORT;
}

void tf6NdResEntryRemoveAll(ttDeviceEntryPtr devEntryPtr)
{
    TM_UNREF_IN_ARG(devEntryPtr);
    return;
}

#else /* ! TM_USE_IPV6 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV6 is not defined */
int tv6ArpDummy = 0;
#endif /* ! TM_USE_IPV6 */


/***************** End Of File *****************/
