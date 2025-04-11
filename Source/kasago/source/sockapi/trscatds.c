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
 * Description:     Device Driver Single Scattered Send function APIs
 * Filename:        trscatds.c
 * Author:          Odile
 * Date Created:    06/10/2002
 * $Source: source/sockapi/trscatds.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:12JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h> 
#include <trmacro.h>

#ifdef TM_USE_DRV_ONE_SCAT_SEND
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/*
 * tfUseInterfaceOneScatSend Function Description
 * Specify a driver driver scattered send function. This enables the stack
 * to call the device driver with a single send function per packet
 * (scattered or not).
 * User need to call tfUseInterfaceOneScatSend between the 
 * tfAddInterface, and tfOpenInterface/tfConfigInterface calls.
 * 1. Check that the interface is a valid added interface
 * 2. Check that the transmit queue is not enabled. Not supported with 
 *    this call.
 * 3. Check that the intrface is not point to point. Not supported with
 *    this call.
 * 3. Add the single scattered send function on the interface.
 * Parameters
 * interfaceHandle
 * devScatRecvFuncPtr
 * Returns
 * TM_ENOERROR  Success
 * TM_EINVAL    error in Parameter.
 */
int tfUseInterfaceOneScatSend (
                ttUserInterface         interfaceHandle,
                ttDevOneScatSendFuncPtr drvOneScatSendFuncPtr )
{
    ttDeviceEntryPtr devEntryPtr;
    int              errorCode;

    errorCode = TM_ENOERROR;
    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
/* Check parameter */
    errorCode = tfValidInterface(devEntryPtr);
    if ( errorCode == TM_ENOERROR )
    {
        if (drvOneScatSendFuncPtr == (ttDevOneScatSendFuncPtr)0)
        {
            errorCode = TM_EINVAL;
        }
/* 
 * Cannot set after device has been opened. Cannot set after setting
 * transmit queue.
 * Cannot set or on point to point interface.
 */
        if (    tm_16bit_one_bit_set(devEntryPtr->devFlag,
                                     TM_DEV_OPENED)
             || (tm_ll_is_pt2pt(devEntryPtr)) )
        {
            errorCode = TM_EPERM;
        }
        if (devEntryPtr->devXmitEntryPtr != (ttDevXmitEntryPtr)0)
        {
            tfKernelWarning("tfUseInterfaceOneScatSend",
               "must be called before tfUseInterfaceXmitQueue\n");
            errorCode = TM_EPERM;
        }
#ifdef TM_USE_DCACHE_MANAGEMENT
        if (devEntryPtr->devCacheFlushFuncPtr != (ttDevCacheMgmtFuncPtr)0)
        {
            tfKernelWarning("tfUseInterfaceOneScatSend",
               "must be called before tfUseInterfaceCacheFlushFunc\n");
            errorCode = TM_EPERM;
        }
#endif /* TM_USE_DCACHE_MANAGEMENT */
        if (errorCode == TM_ENOERROR)
        {
            devEntryPtr->devOneScatSendFuncPtr = drvOneScatSendFuncPtr;
        }
    }
    return (errorCode);
}


#else /* !TM_USE_DRV_ONE_SCAT_SEND */


#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_DRV_ONE_SCAT_SEND is not defined */
int tvScatDsDummy = 0;

#endif /* TM_USE_DRV_ONE_SCAT_SEND */
