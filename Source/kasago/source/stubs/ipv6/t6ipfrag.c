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
 * Description: stubs for IPv6 fragmentation
 *
 * Filename: t6ipfrag.c
 * Author: Jason Schmidlapp
 * Date Created: 7/12/2002
 * $Source: source/stubs/ipv6/t6ipfrag.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:03JST $
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

#ifdef TM_6_IP_FRAGMENT

ttPacketPtr tf6IpReassemblePacket(ttPacketPtr packetPtr, tt8BitPtr ipHdrPtr )
{
    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(ipHdrPtr);

    return (ttPacketPtr) 0;
}

int tf6IpFragmentPacket(
    ttPacketPtr origPacketPtr,
    ttPktLen    unFragLen,
    ttPktLen    devMtu,
    tt8Bit      nextHdr
)
{
    TM_UNREF_IN_ARG(origPacketPtr);
    TM_UNREF_IN_ARG(unFragLen);
    TM_UNREF_IN_ARG(devMtu);
    TM_UNREF_IN_ARG(nextHdr);

    return TM_EPROTONOSUPPORT;
}
#endif /* TM_6_IP_FRAGMENT */

#else /* ! TM_USE_IPV6 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV6 is not defined */
int tv6IpFragDummy = 0;
#endif /* ! TM_USE_IPV6 */

/***************** End Of File *****************/

