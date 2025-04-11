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
 * Description:  btdhcp stub function
 * Filename:     stubs/trbtdhcp.c
 * Author:       Jason
 * Date Created: 7/13/98
 * $Source: source/stubs/dhcp/trbtdhcp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:29JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>


int tfBtInit( ttDeviceEntryPtr        devEntryPtr,
              ttUserDhcpNotifyFuncPtr dhcpNotifyFuncPtr,
              int                     index,
              tt8Bit                  type,
              tt8Bit                  bootIndex )
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(dhcpNotifyFuncPtr);
    TM_UNREF_IN_ARG(index);
    TM_UNREF_IN_ARG(type);
    TM_UNREF_IN_ARG(bootIndex);
    return TM_EPROTONOSUPPORT;
}

ttBtEntryPtr tfBtMap( ttDeviceEntryPtr devEntryPtr,
                      int              index,
                      tt8Bit           type,
                      tt8Bit           bootIndex )
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(index);
    TM_UNREF_IN_ARG(type);
    TM_UNREF_IN_ARG(bootIndex);
    return (ttBtEntryPtr)0;
}

ttBtEntryPtr tfBtAlloc ( ttDeviceEntryPtr        devEntryPtr,
                         int                     index,
                         tt8Bit                  type,
                         tt8Bit                  bootIndex )
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(index);
    TM_UNREF_IN_ARG(type);
    TM_UNREF_IN_ARG(bootIndex);
    return (ttBtEntryPtr)0;
}

int tfBtSetOption( ttUserInterface interfaceHandle,
                   int             index,
                   ttUser8Bit      bootOption,
                   ttUserCharPtr   dataPtr,
                   ttUser8Bit      dataLen,
                   ttUser8Bit      indexType,
                   ttUser8Bit      bootIndex )
{
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(index);
    TM_UNREF_IN_ARG(bootOption);
    TM_UNREF_IN_ARG(dataPtr);
    TM_UNREF_IN_ARG(dataLen);
    TM_UNREF_IN_ARG(indexType);
    TM_UNREF_IN_ARG(bootIndex);
    return TM_EPROTONOSUPPORT;
}

int tfBtGetOption( ttUserInterface  interfaceHandle,
                   int              index,
                   ttUser8Bit       bootOption,
                   ttUserCharPtr    dataPtr,
                   ttUser8BitPtr    dataLenPtr,
                   ttUser8Bit       indexType,
                   ttUser8Bit       bootIndex )
{
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(index);
    TM_UNREF_IN_ARG(bootOption);
    TM_UNREF_IN_ARG(dataPtr);
    TM_UNREF_IN_ARG(dataLenPtr);
    TM_UNREF_IN_ARG(indexType);
    TM_UNREF_IN_ARG(bootIndex);
    return TM_EPROTONOSUPPORT;
}
