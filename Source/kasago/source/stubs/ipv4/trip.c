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
 * Description: stubs for IP functions
 * Filename: trip.c
 * Author: Odile
 * Date Created: 12/04/97
 * $Source: source/stubs/ipv4/trip.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:47JST $
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


int tfIpSendPacket(  ttPacketPtr packetPtr
                   , ttLockEntryPtr lockEntryPtr
#ifdef TM_USE_IPSEC    
                   , ttPktLenPtr    ipsecMtuAdjustPtr
#endif /* TM_USE_IPSEC */
                    )
{
    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(lockEntryPtr);
#ifdef TM_USE_IPSEC    
    TM_UNREF_IN_ARG(ipsecMtuAdjustPtr);
#endif /* TM_USE_IPSEC */

    return TM_EPROTONOSUPPORT;
}

void tfIpIncomingPacket( ttPacketPtr packetPtr
#ifdef TM_USE_IPSEC_TASK
                        ,(ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
                        )
{
    TM_UNREF_IN_ARG(packetPtr);

    return;
}

tt16Bit tfIpHdr5Checksum(tt16BitPtr iphPtr)
{
    TM_UNREF_IN_ARG(iphPtr);

    return 0;
}

int tfIpCacheRte(
    ttRtCacheEntryPtr rtcPtr,
    tt4IpAddress      ipDestAddr
    )
{
    TM_UNREF_IN_ARG(rtcPtr);
    TM_UNREF_IN_ARG(ipDestAddr);

    return TM_EPROTONOSUPPORT;
}

int tfIpBroadMcastToLan (tt4IpAddress ipAddr, ttDeviceEntryPtr devPtr,
                         tt16Bit mhomeIndex, tt8BitPtr etherPtr)
{
    TM_UNREF_IN_ARG(ipAddr);
    TM_UNREF_IN_ARG(devPtr);
    TM_UNREF_IN_ARG(mhomeIndex);
    TM_UNREF_IN_ARG(etherPtr);

    return TM_EPROTONOSUPPORT;
}

tt4IpAddress tfGetRouterId (void)
{
    return 0;
}

#ifdef TM_SINGLE_INTERFACE_HOME
ttDeviceEntryPtr tfIfaceConfig(tt16Bit flag)
{
    TM_UNREF_IN_ARG(flag);

    return (ttDeviceEntryPtr) 0;
}

#else /* !TM_SINGLE_INTERAFACE_HOME */

int tfMhomeAddrMatch (ttDeviceEntryPtr   devPtr,
                     tt4IpAddress        ipAddress,
                     tt16BitPtr          ifaceMhomePtr)
{
    TM_UNREF_IN_ARG(devPtr);
    TM_UNREF_IN_ARG(ipAddress);
    TM_UNREF_IN_ARG(ifaceMhomePtr);

    return TM_EPROTONOSUPPORT;
}

int tfMhomeAnyConf (ttDeviceEntryPtr   devPtr,
                    tt4IpAddress       ipAddr,
                    tt16BitPtr         ifaceMhomePtr)
{
    TM_UNREF_IN_ARG(devPtr);
    TM_UNREF_IN_ARG(ipAddr);
    TM_UNREF_IN_ARG(ifaceMhomePtr);

    return TM_EPROTONOSUPPORT;
}

int tfMhomeNetMatch (ttDeviceEntryPtr   devPtr,
                     tt4IpAddress       ipAddress,
                     tt16BitPtr         ifaceMhomePtr)
{
    TM_UNREF_IN_ARG(devPtr);
    TM_UNREF_IN_ARG(ipAddress);
    TM_UNREF_IN_ARG(ifaceMhomePtr);

    return TM_EPROTONOSUPPORT;
}

int tfMhomeDBroadMatch (ttDeviceEntryPtr   devPtr,
                        tt4IpAddress       ipAddress,
                        tt16BitPtr         ifaceMhomePtr)
{
    TM_UNREF_IN_ARG(devPtr);
    TM_UNREF_IN_ARG(ipAddress);
    TM_UNREF_IN_ARG(ifaceMhomePtr);

    return TM_EPROTONOSUPPORT;
}

ttDeviceEntryPtr tfIfaceMatch (tt4IpAddress    ipAddress,
                               ttMhomeFunctPtr mhomeMatchFuncPtr,
                               tt16BitPtr      ifaceMhomePtr)
{
    TM_UNREF_IN_ARG(ipAddress);
    TM_UNREF_IN_ARG(mhomeMatchFuncPtr);
    TM_UNREF_IN_ARG(ifaceMhomePtr);

    return (ttDeviceEntryPtr) 0;
}
#endif /* TM_SINGLE_INTERFACE_HOME */

int tfIpDestToPacket (ttPacketPtr       packetPtr,
                      tt4IpAddress      srcIpAddress,
                      tt4IpAddress      peerIpAddress,
                      tt8Bit            ipTos,
                      ttRtCacheEntryPtr rtcPtr,
                      ttIpHeaderPtr     iphPtr)
{
    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(srcIpAddress);
    TM_UNREF_IN_ARG(peerIpAddress);
    TM_UNREF_IN_ARG(ipTos);
    TM_UNREF_IN_ARG(rtcPtr);
    TM_UNREF_IN_ARG(iphPtr);

    return TM_EPROTONOSUPPORT;
}

#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4IpDummy = 0;
#endif /* ! TM_USE_IPV4 */
