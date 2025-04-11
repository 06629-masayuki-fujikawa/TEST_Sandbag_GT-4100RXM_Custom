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
 * Description: PPP Link Layer stub functions
 *
 * Filename:     stubs/trppp.c
 * Author:       Jason
 * Date Created: 7/13/98
 * $Source: source/stubs/ppp/trppp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:25JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

ttUserLinkLayer tfUseAsyncPpp(ttUserLnkNotifyFuncPtr linkNotifyFuncPtr)
{
    TM_UNREF_IN_ARG(linkNotifyFuncPtr);
    return ((ttUserLinkLayer)0);
}

ttUserLinkLayer tfUseAsyncServerPpp(ttUserLnkNotifyFuncPtr linkNotifyFuncPtr)
{
    TM_UNREF_IN_ARG(linkNotifyFuncPtr);
    return ((ttUserLinkLayer)0);
}

int tfPppSetOption(ttUserInterface    interfaceId, 
                   int                protocolLevel,
                   int                remoteLocalFlag, 
                   int                optionName,
                   const char TM_FAR *optionValuePtr,
                   int                optionLength)
{
    TM_UNREF_IN_ARG(interfaceId);
    TM_UNREF_IN_ARG(protocolLevel);
    TM_UNREF_IN_ARG(remoteLocalFlag);
    TM_UNREF_IN_ARG(optionName);
    TM_UNREF_IN_ARG(optionValuePtr);
    TM_UNREF_IN_ARG(optionLength);
    return TM_EPROTONOSUPPORT;
}                                  


int tfPapRegisterAuthenticate(ttUserInterface interfaceId,
                              ttPapAuthenticateFunctPtr functPtr)

{
    TM_UNREF_IN_ARG(interfaceId);
    TM_UNREF_IN_ARG(functPtr);
    return TM_EPROTONOSUPPORT;
}

int tfChapRegisterAuthenticate(ttUserInterface interfaceId,
                               ttChapAuthenticateFunctPtr functPtr)
{
    TM_UNREF_IN_ARG(interfaceId);
    TM_UNREF_IN_ARG(functPtr);
    return TM_EPROTONOSUPPORT;
}
