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
 * Description: tfInterfaceSpinLock API. Called from the driver send function.
 *
 * Filename: trxmttsk.c
 * Author: Odile
 * Date Created: 02/24/00
 * $Source: source/sockapi/trspnlck.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:34JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#ifdef TM_LOCK_NEEDED
#include <trproto.h>
#include <trglobal.h>
#endif /* TM_LOCK_NEEDED */

/*
 * Called from the device driver send routine to yield the CPU to
 * allow spin lock while waiting for room to transmit the data.
 * This function will yield the CPU, only if the user is using a transmit
 * task to interface between the Treck stack and the device driver,
 * i.e the user set that option with tfInterfaceSetOptions().
 * 1. Check that spin lock is allowed, i.e user uses an interface transmit
 *    task. If the user did not turn on that option, error code is set to
 *    TM_EPERM.
 * 2. If no error:
 *  2.1 Unlock the device driver lock
 *  2.2 Yield CPU
 *  2.3 Relock the device driver lock.
 * 3. Return error code to the user.
 * Parameters
 * interfaceHandle  Interface handle as returned by tfAddInterface()
 * Return
 * value          meaning
 * TM_ENOERROR    tfInterfaceSpinLock successfully called the OS yield.
 * TM_EPERM       Yield is disallowed because no transmit task is used.
 */
int tfInterfaceSpinLock (ttUserInterface interfaceHandle)
{
    ttDeviceEntryPtr devEntryPtr;
    int              retCode;

    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
    if (tm_16bit_one_bit_set(devEntryPtr->devFlag, TM_DEV_XMIT_TASK))
    {
        retCode = TM_ENOERROR;
        tm_call_unlock(&(devEntryPtr->devDriverLockEntry));
        tfKernelTaskYield();
        tm_call_lock_wait(&(devEntryPtr->devDriverLockEntry));
    }
    else
    {
        retCode = TM_EPERM;
    }
    return retCode;
}
