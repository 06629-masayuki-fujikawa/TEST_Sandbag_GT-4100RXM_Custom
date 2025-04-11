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
 * Description: Treck MULTI Context Functions to Store/Retrieve
 *              Device Specific Pointer
 * Filename: trcontxt.c
 * Author: Odile
 * Date Created: 9/22/2000
 * $Source: source/sockapi/trcontxt.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:42:53JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>

int tfDeviceStorePointer( ttUserInterface interfaceHandle,
                          ttVoidPtr       deviceDriverPtr)
{
    ttDeviceEntryPtr devEntryPtr;
    int              errorCode;

    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
    errorCode = tfValidInterface(devEntryPtr);
    if (errorCode == TM_ENOERROR )
    {
        if (devEntryPtr->devDriverPtr == (ttVoidPtr)0)
        {
            devEntryPtr->devDriverPtr = deviceDriverPtr;
        }
        else
        {
            errorCode = TM_EALREADY;
        }
    }
    return errorCode;
}


ttVoidPtr tfDeviceClearPointer( ttUserInterface interfaceHandle )
{
    ttDeviceEntryPtr devEntryPtr;
    ttVoidPtr        deviceDriverPtr;
    int              errorCode;

    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
    errorCode = tfValidInterface(devEntryPtr);
    deviceDriverPtr = (ttVoidPtr)0;
    if (errorCode == TM_ENOERROR )
    {
        if (devEntryPtr->devDriverPtr != (ttVoidPtr)0)
        {
            deviceDriverPtr = devEntryPtr->devDriverPtr;
            devEntryPtr->devDriverPtr = (ttVoidPtr)0;
        }
    }
    return deviceDriverPtr;
}

ttVoidPtr tfDeviceGetPointer(ttUserInterface interfaceHandle)
{
    return ((ttDeviceEntryPtr)interfaceHandle)->devDriverPtr;
}

