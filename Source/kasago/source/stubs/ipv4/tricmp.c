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
 * Description: stubs for ICMP functions
 * Filename: tricmp.c
 * Author: Odile
 * Date Created: 12/29/97
 * $Source: source/stubs/ipv4/tricmp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:45JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_IPV4

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

void tfIcmpIncomingPacket (ttPacketPtr packetPtr)
{
    TM_UNREF_IN_ARG(packetPtr);

    return;
}

int tfIcmpCreatePacket (ttDeviceEntryPtr   devPtr,
                        tt16Bit            mhomeIndex,
                        tt16Bit            flags,
                        tt4IpAddress       ipSrcAddr,
                        tt4IpAddress       ipDestAddr,
                        tt8Bit             icmpType,
                        tt8Bit             icmpCode,
                        tt4IpAddress       ipAddr)
{
    TM_UNREF_IN_ARG(devPtr);
    TM_UNREF_IN_ARG(mhomeIndex);
    TM_UNREF_IN_ARG(flags);
    TM_UNREF_IN_ARG(ipSrcAddr);
    TM_UNREF_IN_ARG(ipDestAddr);
    TM_UNREF_IN_ARG(icmpType);
    TM_UNREF_IN_ARG(icmpCode);
    TM_UNREF_IN_ARG(ipAddr);

    return TM_EPROTONOSUPPORT;
}

int tfRawSendPacket ( ttSocketEntryPtr socketPtr,
                      ttPacketPtr      packetPtr )
{
    TM_UNREF_IN_ARG(socketPtr);
    TM_UNREF_IN_ARG(packetPtr);

    return TM_EPROTONOSUPPORT;
}

void tfIcmpErrPacket(ttPacketPtr      packetPtr,
                     tt8Bit           icmpType,
                     tt8Bit           icmpCode,
                     tt32Bit          icmpParam,
                     ttDeviceEntryPtr outDevPtr,
                     tt4IpAddress     ipAddr)
{
    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(icmpType);
    TM_UNREF_IN_ARG(icmpCode);
    TM_UNREF_IN_ARG(icmpParam);
    TM_UNREF_IN_ARG(outDevPtr);
    TM_UNREF_IN_ARG(ipAddr);

    return;
}

#else /* ! TM_USE_IPV4 */

LINT_UNUSED_HEADER

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4IcmpDummy = 0;
#endif /* ! TM_USE_IPV4 */
