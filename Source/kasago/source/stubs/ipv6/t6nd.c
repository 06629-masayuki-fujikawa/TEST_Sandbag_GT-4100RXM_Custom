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
 * Description: stubs for Common Neighbor Discovery routines
 *
 * Filename: t6nd.c
 * Author: Jason Schmidlapp
 * Date Created:
 * $Source: source/stubs/ipv6/t6nd.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:06JST $
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

int tf6NdSendNeighSolicit(ttDeviceEntryPtr      devEntryPtr,
                          tt6ConstIpAddressPtr  targIpAddrPtr,
                          tt6ConstIpAddressPtr  srcAddrPtr,
                          tt6ConstIpAddressPtr  destAddrPtr)
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(targIpAddrPtr);
    TM_UNREF_IN_ARG(srcAddrPtr);
    TM_UNREF_IN_ARG(destAddrPtr);

    return TM_EPROTONOSUPPORT;
}

int tf6NdSendNeighAdvert(ttPacketPtr           packetPtr,
                         ttDeviceEntryPtr      devEntryPtr,
                         tt16Bit               mHomeIndex,
                         tt6ConstIpAddressPtr  destAddrPtr,
                         tt8Bit                solicitFlag)
{
    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(mHomeIndex);
    TM_UNREF_IN_ARG(destAddrPtr);
    TM_UNREF_IN_ARG(solicitFlag);

    return TM_EPROTONOSUPPORT;
}

void tf6NdIncomingPacket(ttPacketPtr  packetPtr,
                         tt8Bit       msgType,
                         tt8Bit       msgCode)
{
    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(msgType);
    TM_UNREF_IN_ARG(msgCode);

    return;
}


#else /* ! TM_USE_IPV6 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV6 is not defined */
int tv6NdDummy = 0;
#endif /* ! TM_USE_IPV6 */

/***************** End Of File *****************/
