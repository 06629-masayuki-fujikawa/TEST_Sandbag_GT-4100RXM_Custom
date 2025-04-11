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
 * Description: LLMNR Protocol, responder stub functions 
 *
 * Filename: trrespon.c
 * Author: 
 * Date Created: 2007/04/19
 * $Source: source/stubs/llmnr/trrespon.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:58JST $
 * $Author: 
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/* 
 * Include Header files
 */
    
#include <trllmnr.h>

#ifdef TM_USE_LLMNR_RESPONDER

int tfLlmnrConfigResponderName(ttUserInterface  interfaceHandle, 
                               char *           responderName, 
                               int              length)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(responderName);
    TM_UNREF_IN_ARG(length);

    return TM_EPROTONOSUPPORT;
}

int tfLlmnrResponderInit(ttDeviceEntryPtr               devPtr,
                         int                            llmnrFlag,
                         ttUserConstCharPtr             responderName,
                         int                            resLength,
                         ttRespNameEventNotifyFuncPtr   
                                              respNameEventNotifyFuncPtr)
{
    TM_UNREF_IN_ARG(devPtr);
    TM_UNREF_IN_ARG(llmnrFlag);
    TM_UNREF_IN_ARG(responderName);
    TM_UNREF_IN_ARG(resLength);
    TM_UNREF_IN_ARG(respNameEventNotifyFuncPtr);

    return TM_EPROTONOSUPPORT;
}

int  tfLlmnrResponderUnInit(ttDeviceEntryPtr   devPtr)
{
    TM_UNREF_IN_ARG(devPtr);

    return TM_EPROTONOSUPPORT;
}

#else /* !TM_USE_LLMNR_RESPONDER */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_LLMNR_RESPONDER is not defined */
int tvResponderDummy = 0;

#endif /* TM_USE_LLMNR_RESPONDER */
