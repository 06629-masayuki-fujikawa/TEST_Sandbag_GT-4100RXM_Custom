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
 * Description: LLMNR Protocol, common stub functions for sender/responder
 *
 * Filename: trllmnr.c
 * Author: 
 * Date Created: 2007/03/23
 * $Source: source/stubs/llmnr/trllmnr.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:58JST $
 * $Author: 
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/* 
 * Header files
 */
#include <trllmnr.h>

int tfLlmnrInit(
        ttUserInterface                 interfaceHandle,
        int                             llmnrFlag, 
        int                             llmnrOperationalMode,
        const char TM_FAR *             responderName,
        int                             resLength,
        ttRespNameEventNotifyFuncPtr    respNameEventNotifyFuncPtr)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(llmnrFlag);
    TM_UNREF_IN_ARG(llmnrOperationalMode);
    TM_UNREF_IN_ARG(responderName);
    TM_UNREF_IN_ARG(resLength);
    TM_UNREF_IN_ARG(respNameEventNotifyFuncPtr);

    return TM_ENOERROR;
}

int tfLlmnrUnInit(ttUserInterface interfaceHandle)
{
    TM_UNREF_IN_ARG(interfaceHandle);

    return TM_ENOERROR;
}

