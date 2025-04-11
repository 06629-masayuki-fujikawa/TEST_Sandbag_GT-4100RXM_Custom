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
 * Description: BSD Sockets Interface (tfGetIfPhysAddr)
 *
 * Filename: trgifadd.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trgifadd.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:19JST $
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
#include <trproto.h>
#ifdef TM_LOCK_NEEDED
#include <trglobal.h>
#endif /* TM_LOCK_NEEDED */

/* 
 * Get the Physical Address for an Interface 
 */
int tfGetIfPhysAddr (ttUserInterface interfaceId,
                     char TM_FAR *physAddrPtr,
                     int physAddrLength)
{
    tt8Bit addressLength;
    int    errorCode;

    if ( physAddrPtr != (char TM_FAR *)0 )
    {
        errorCode = tfValidInterface((ttDeviceEntryPtr)interfaceId);
/* Check parameter */
        if (errorCode == TM_ENOERROR)
        {
            tm_call_lock_wait(
                           &(((ttDeviceEntryPtr)interfaceId)->devLockEntry));
            addressLength =
                          ((ttDeviceEntryPtr)interfaceId)->devPhysAddrLength;
            if (addressLength == 0)
            {
                errorCode=TM_ENXIO;
            }
            else
            {
                if (physAddrLength < (int)addressLength)
                {
                    errorCode=TM_EINVAL;
                }
                else
                {
                    tm_bcopy(((ttDeviceEntryPtr)interfaceId)->devPhysAddrArr,
                            physAddrPtr,addressLength);
                }
            }
            tm_call_unlock(&(((ttDeviceEntryPtr)interfaceId)->devLockEntry));
        }
    }
    else
    {
        errorCode=TM_EINVAL;
    }
    return(errorCode);
}

