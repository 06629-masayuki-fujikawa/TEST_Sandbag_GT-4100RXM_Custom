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
 * Description:  IGMP stub functions
 * Filename:     trigmp.c
 * Author:       Odile
 * Date Created: 08/26/99
 * $Source: source/stubs/ipv4/trigmp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:46JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h> 
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

#ifdef TM_IGMP

int tfEtherMcast( ttVoidPtr        interfaceHandle,
                  tt4IpAddress     groupAddress,
                  int              command )
{
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(groupAddress);
    TM_UNREF_IN_ARG(command);

    return TM_ENOERROR;
}

void tfIgmpDevInit (ttDeviceEntryPtr devPtr, tt16Bit mhomeIndex)
{
    TM_UNREF_IN_ARG(devPtr);
    TM_UNREF_IN_ARG(mhomeIndex);
}

void tfIgmpInit(void)
{
}

int tfIgmpSetSockOpt ( ttSocketEntryPtr   socketEntryPtr,
                       int                optionName,
                       ttConstCharPtr     optionValuePtr,
                       int                optionLength )
{
    TM_UNREF_IN_ARG(socketEntryPtr);
    TM_UNREF_IN_ARG(optionName);
    TM_UNREF_IN_ARG(optionValuePtr);
    TM_UNREF_IN_ARG(optionLength);
    return TM_EPROTONOSUPPORT;
}

int tfIgmpGetSockOpt ( ttSocketEntryPtr   socketEntryPtr,
                       int                optionName,
                       ttConstCharPtr     optionValuePtr,
                       ttIntPtr           optionLengthPtr )
{
    TM_UNREF_IN_ARG(socketEntryPtr);
    TM_UNREF_IN_ARG(optionName);
    TM_UNREF_IN_ARG(optionValuePtr);
    TM_UNREF_IN_ARG(optionLengthPtr);
    return TM_EPROTONOSUPPORT;
}

int tfIgmpMember(  ttDeviceEntryPtr   devPtr
                 , tt4IpAddress        multiCastGroup
#ifndef TM_SINGLE_INTERFACE_HOME
                 , tt16BitPtr         mhomeIndexPtr
#endif /* !TM_SINGLE_INTERFACE_HOME */
                 )
{
    TM_UNREF_IN_ARG(devPtr);
    TM_UNREF_IN_ARG(multiCastGroup);
#ifndef TM_SINGLE_INTERFACE_HOME
    TM_UNREF_IN_ARG(mhomeIndexPtr);
#endif /* TM_SINGLE_INTERFACE_HOME */
    return TM_ENOENT;
}

/* MCAST group in socket cache and source address not filtered? */
tt8Bit tfIgmpSocketCacheMatch(
    ttSocketEntryPtr socketEntryPtr,
    ttDeviceEntryPtr devEntryPtr,
    tt4IpAddress     groupAddress
#ifdef TM_USE_IGMPV3
   ,tt4IpAddress     srcAddress
#endif /* TM_USE_IGMPV3 */
    )
{
    TM_UNREF_IN_ARG(socketEntryPtr);
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(groupAddress);
#ifndef TM_USE_IGMPV3
    TM_UNREF_IN_ARG(srcAddress);
#endif /* TM_USE_IGMPV3 */
    return TM_8BIT_NO;
}

void tfIgmpSocketCachePurge(ttSocketEntryPtr socketEntryPtr)
{
    TM_UNREF_IN_ARG(socketEntryPtr);
}

void tfIgmpIncomingPacket(ttPacketPtr packetPtr)
{
    TM_UNREF_IN_ARG(packetPtr);
}

#else /* !TM_IGMP */
int tlIgmpStub = 0;
#endif /* TM_IGMP */
