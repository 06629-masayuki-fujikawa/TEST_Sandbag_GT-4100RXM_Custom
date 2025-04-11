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
 * Description:  DHCP stub function
 * Filename:     stub/trdhcp.c
 * Author:       Jason
 * Date Created: 7/13/98
 * $Source: source/stubs/dhcp/trdhcp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:30JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>


int tfUseDhcp(ttUserInterface          interfaceHandle,
              ttDevNotifyFuncPtr       intfNotifyFuncPtr)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(intfNotifyFuncPtr);
    return TM_EPROTONOSUPPORT;
}

int tfDhcpStart ( ttDeviceEntryPtr deviceEntryPtr, int index, tt8Bit type )
{
    TM_UNREF_IN_ARG(deviceEntryPtr);
    TM_UNREF_IN_ARG(index);
    TM_UNREF_IN_ARG(type);
    return TM_EPROTONOSUPPORT;
}

int tfDhcpStop ( ttDeviceEntryPtr deviceEntryPtr, int index, tt8Bit type )
{
    TM_UNREF_IN_ARG(deviceEntryPtr);
    TM_UNREF_IN_ARG(index);
    TM_UNREF_IN_ARG(type);
    return TM_EPROTONOSUPPORT;
}

void tfDhcpRecvCB ( ttPacketPtr  packetPtr,
                           ttBtEntryPtr btEntryPtr )
{
    TM_UNREF_IN_ARG(btEntryPtr);
    tfFreePacket(packetPtr,TM_SOCKET_UNLOCKED);
    return;
}
