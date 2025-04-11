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
 * $Source: source/stubs/igmp/trigmp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:37JST $
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
                 , tt4IpAddress       multicastGroup
#ifndef TM_SINGLE_INTERFACE_HOME
                 , tt16BitPtr         mhomeIndexPtr
#endif /* !TM_SINGLE_INTERFACE_HOME */
                 )
{
#ifndef TM_SINGLE_INTERFACE_HOME
    tt16Bit      mhomeIndex;
#endif /* !TM_SINGLE_INTERFACE_HOME */
    int          errorCode;

    TM_UNREF_IN_ARG(devPtr);
    errorCode = TM_ENOENT;
    if (tm_ip_is_local_mcast_address(multicastGroup))
    {
/* 
 * Find a configured slot or a slot in the process of being configured by the 
 * user
 */
#ifdef TM_SINGLE_INTERFACE_HOME
       if (    ( tm_8bit_one_bit_set(tm_ip_dev_conf_flag(devPtr, 0),
                                   TM_DEV_IP_CONFIG) )
            || ( tm_8bit_all_bits_set(tm_ip_dev_conf_flag(devPtr, 0),
                            (TM_DEV_IP_CONF_STARTED | TM_DEV_IP_USER_BOOT)) ) )
        {
            errorCode = TM_ENOERROR;
        }
#else /* !TM_SINGLE_INTERFACE_HOME */
        if ((int)devPtr->devMhomeUsedEntries != 0)
        {
/*
 * If a slot is cofigured, use first configured:
 * NOTE TM_DEV_IP_CONFIG is INPUT parameter to tm_iface_any_conf().
 * mhomeIndex will be set if devPtr is non null.
 */
            mhomeIndex = (tt16Bit)TM_DEV_IP_CONFIG;
        }
        else
        {
/* else use a booting interface if any.
 * NOTE TM_DEV_IP_CONF_STARTED|TM_DEV_IP_USER_BOOT is INPUT parameter to
 * tm_iface_any_conf().
 * mhomeIndex will be set if devPtr is non null.
 */
            mhomeIndex = (tt16Bit)(   TM_DEV_IP_CONF_STARTED
                                    | TM_DEV_IP_USER_BOOT );
        }
        if (tfMhomeAnyConf(devPtr, TM_IP_ZERO, &mhomeIndex)
                                            == TM_ENOERROR)
        {
            *mhomeIndexPtr = mhomeIndex;
            errorCode = TM_ENOERROR;
        }
#endif /* !TM_SINGLE_INTERFACE_HOME */
    }
    return errorCode;
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
#ifdef TM_USE_IGMPV3
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
    tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
}

void tfIgmpDevClose(ttDeviceEntryPtr devEntryPtr)
{
    TM_UNREF_IN_ARG(devEntryPtr);
}

#else /* !TM_IGMP */
int tlIgmpStub = 0;
#endif /* TM_IGMP */
