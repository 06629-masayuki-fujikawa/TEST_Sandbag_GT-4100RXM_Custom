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
 * Description: Extensible Authentication Protocol implementation Sub File
 *
 * Filename: treap.c
 * Author: Jason Schmidlapp
 * Date Created: 3/31/03
 * $Source: source/stubs/eap/treap.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:33JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_EAP

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/*
 * tfEapMd5RegisterAuthenticate Function Description
 *
 * Returns
 * TM_EPROTONOSUPPORT
 */ 
int tfEapMd5RegisterAuthenticate(ttUserInterface     interfaceHandle,
                                 ttEapMd5AuthFuncPtr authFuncPtr)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(authFuncPtr);
    return TM_EPROTONOSUPPORT;
}

/*
 * tfEapSetOption Function Description
 *
 * Returns
 * TM_EPROTONOSUPPORT;
 *
 */ 
int tfEapSetOption(ttUserInterface      interfaceHandle,
                   int                  optionName,
                   const char TM_FAR *  optionValuePtr,
                   int                  optionLength)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(optionName);
    TM_UNREF_IN_ARG(optionValuePtr);
    TM_UNREF_IN_ARG(optionLength);
    return TM_EPROTONOSUPPORT;
}

/*
 * tfEapClose Function Description
 *
 * Returns
 * TM_EPROTONOSUPPORT;
 */ 
int tfEapClose(ttVoidPtr eapVectParmPtr)
{
    TM_UNREF_IN_ARG(eapVectParmPtr);
    return TM_EPROTONOSUPPORT;
}

/*
 * tfEapIncomingPacket Function Description
 *
 * Returns
 * none
 */
void tfEapIncomingPacket(ttVoidPtr    eapVectParmPtr,
                         ttPacketPtr  packetPtr)
{
    TM_UNREF_IN_ARG(eapVectParmPtr);
    TM_UNREF_IN_ARG(packetPtr);
    return;
}

/*
 * tfEapInit Function Description
 *
 * Returns
 * TM_EPROTONOSUPPORT
 */

int tfEapInit(ttVoidPtrPtr         eapVectParmPtrPtr,
              ttDeviceEntryPtr     devEntryPtr,
              ttAuthNotifyFuncPtr  notifyFuncPtr)
{
    TM_UNREF_IN_ARG(eapVectParmPtrPtr);
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(notifyFuncPtr);
    return TM_EPROTONOSUPPORT;
}
/*
 * tfEapOpen Function Description
 *
 * Returns
 * TM_EPROTONOSUPPORT
 */
int tfEapOpen(ttVoidPtr    eapVectParmPtr,
              tt8Bit       authPeerFlag)
{
    TM_UNREF_IN_ARG(eapVectParmPtr);
    TM_UNREF_IN_ARG(authPeerFlag);
    return TM_EPROTONOSUPPORT;
}

#else /* !TM_USE_EAP */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */


/* To allow link for builds when TM_USE_EAP is not defined */
int tvEapDummy = 0;
#endif /* TM_USE_EAP */

/***************** End Of File *****************/
