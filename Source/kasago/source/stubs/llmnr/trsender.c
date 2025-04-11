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
 * Description: LLMNR Protocol, sender stub functions 
 *
 * Filename: trsender.c
 * Author:
 * Date Created: 2007/03/23
 * $Source: source/stubs/llmnr/trsender.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:58JST $
 * $Author: 
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/* 
 * Include 
 */

#include <trllmnr.h>

#ifdef TM_USE_LLMNR_SENDER

int tfLlmnrSenderInit(ttDeviceEntryPtr devPtr, int llmnrFlag)
{
    TM_UNREF_IN_ARG(devPtr);
    TM_UNREF_IN_ARG(llmnrFlag);

    return TM_EPROTONOSUPPORT;
}

int  tfLlmnrSenderUnInit(ttDeviceEntryPtr   devPtr)
{
    TM_UNREF_IN_ARG(devPtr);

    return TM_EPROTONOSUPPORT;
}

int tfLlmnrSetSenderOption(int optionType, int optionValue)
{
    TM_UNREF_IN_ARG(optionType);
    TM_UNREF_IN_ARG(optionValue);

    return TM_EPROTONOSUPPORT;
}

int tfLlmnrGetAddrInfo( ttUserInterface                   interfaceHandle,
                        const char *                      nodeName,
                        short                             nodeNameLength,
                        const struct addrinfo TM_FAR   *  hintsPtr,
                        struct addrinfo TM_FAR * TM_FAR * resPtrPtr)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(nodeName);
    TM_UNREF_IN_ARG(nodeNameLength);
    TM_UNREF_IN_ARG(hintsPtr);
    TM_UNREF_IN_ARG(resPtrPtr);

    return TM_EPROTONOSUPPORT;
}

int tfLlmnrGetNameInfo (ttUserInterface                 interfaceHandle,
                        const struct sockaddr TM_FAR *  addressPtr,
                        short                           addressLength,
                        char TM_FAR *                   hostname,
                        short                           hostnameLength,
                        short                           flags )
{
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(addressPtr);
    TM_UNREF_IN_ARG(addressLength);
    TM_UNREF_IN_ARG(hostname);
    TM_UNREF_IN_ARG(hostnameLength);
    TM_UNREF_IN_ARG(flags);

    return TM_EPROTONOSUPPORT;
}

void tfLlmnrFreeAddrInfo (struct addrinfo * addrInfoPtr)
{
    TM_UNREF_IN_ARG(addrInfoPtr);
}

#else /* !TM_USE_LLMNR_SENDER */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_LLMNR_SENDER is not defined */
int tvSenderDummy = 0;

#endif /* TM_USE_LLMNR_SENDER */
