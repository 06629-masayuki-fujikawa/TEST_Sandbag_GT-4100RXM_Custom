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
 * Description:
 * Implements Link Quality Monitoring for PPP. Refer to RFC-1989.
 *
 * Filename: trlqm.c
 * Author: Ed Remmell
 * Date Created:
 * $Source: source/stubs/ppp/trlqm.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:46:23JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h> /* For TM_PPP_LQM */
#include <trmacro.h> /* For LINT_UNUSED_HEADER */

#if ( defined(TM_USE_PPP) && defined(TM_PPP_LQM) )

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/****************************************************************************
* FUNCTION: tfUsePppLqm
*
* RETURNS:
    TM_EPROTONOSUPPORT;
*
* NOTES:
*
****************************************************************************/
int tfUsePppLqm(
    ttUserInterface interfaceHandle,
    ttUser32Bit     lqrReTxPeriodMsec)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(lqrReTxPeriodMsec);
    return TM_EPROTONOSUPPORT;
}

/****************************************************************************
* FUNCTION: tfFreePppLqm
*
*
* RETURNS:
    TM_EPROTONOSUPPORT;
*
* NOTES:
*
****************************************************************************/
int     tfFreePppLqm(
    ttUserInterface     interfaceHandle)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    return TM_EPROTONOSUPPORT;
}


/****************************************************************************
* FUNCTION: tfLqmRegisterMonitor
*
* RETURNS:
*   TM_EPROTONOSUPPORT;
*
* NOTES:
*
****************************************************************************/
int tfLqmRegisterMonitor(
    ttUserInterface         interfaceHandle,
    ttLqmMonitorFuncPtr     monitorFuncPtr,
    ttUser16Bit             hysteresisMaxFailures,
    ttUser16Bit             hysteresisSamples)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(monitorFuncPtr);
    TM_UNREF_IN_ARG(hysteresisMaxFailures);
    TM_UNREF_IN_ARG(hysteresisSamples);
    return TM_EPROTONOSUPPORT;
}


/****************************************************************************
* FUNCTION: tfLqmSendLinkQualityReport
*
*
* RETURNS:
*   TM_EPROTONOSUPPORT;
*
*
****************************************************************************/
int tfLqmSendLinkQualityReport(
    ttUserInterface interfaceHandle)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    return TM_EPROTONOSUPPORT;
}

/****************************************************************************
* FUNCTION: tfPppSendEchoRequest
*
*
* RETURNS:
*   TM_EPROTONOSUPPORT;
*
****************************************************************************/
int tfPppSendEchoRequest(
    ttUserInterface         interfaceHandle,
    ttUser8Bit              echoRequestId,
    const char TM_FAR *     dataPtr,
    int                     dataLen,
    ttEchoReplyFuncPtr      echoReplyFuncPtr)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(echoRequestId);
    TM_UNREF_IN_ARG(dataPtr);
    TM_UNREF_IN_ARG(dataLen);
    TM_UNREF_IN_ARG(echoReplyFuncPtr);
    return TM_EPROTONOSUPPORT;
}

/****************************************************************************
* FUNCTION: tfLqmSetLqrTimerPeriod
*
*
* RETURNS:
*   TM_EPROTONOSUPPORT;
*
****************************************************************************/
int tfLqmSetLqrTimerPeriod(
    ttUserInterface interfaceHandle,
    ttUser32Bit     lqrTimerPeriodMsec)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(lqrTimerPeriodMsec);
    return TM_EPROTONOSUPPORT;
}

/****************************************************************************
* FUNCTION: tfLqmGetLocalLqrTimerPeriod
*
* RETURNS:
*   TM_EPROTONOSUPPORT;
*
****************************************************************************/
ttUser32Bit tfLqmGetLocalLqrTimerPeriod(
    ttUserInterface interfaceHandle)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    return TM_EPROTONOSUPPORT;
}

/****************************************************************************
* FUNCTION: tfLqmGetPeerLqrTimerPeriod
*
* RETURNS:
*   TM_EPROTONOSUPPORT;
*
****************************************************************************/
ttUser32Bit tfLqmGetPeerLqrTimerPeriod(
    ttUserInterface interfaceHandle)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    return TM_EPROTONOSUPPORT;
}

/****************************************************************************
* FUNCTION: tfLqmIncomingPacket
*
*
* RETURNS:
*   TM_EPROTONOSUPPORT;
*
****************************************************************************/
int tfLqmIncomingPacket(
    ttPppVectPtr    pppVectPtr,
    ttPacketPtr     packetPtr)
{
    TM_UNREF_IN_ARG(pppVectPtr);
    TM_UNREF_IN_ARG(packetPtr);
    return TM_EPROTONOSUPPORT;
}
/****************************************************************************
* FUNCTION: tfLqmIsEnabled
*
* RETURNS:
*   0:
*       LQM is disabled on the link.
*
****************************************************************************/
int tfLqmIsEnabled(
    ttPppVectPtr    pppVectPtr)
{
    TM_UNREF_IN_ARG(pppVectPtr);
    return 0;
}

/****************************************************************************
* FUNCTION: tfLqmDisable
*
* RETURNS:
*   Nothing
*
*
****************************************************************************/
void tfLqmDisable(
    ttPppVectPtr    pppVectPtr)
{
    TM_UNREF_IN_ARG(pppVectPtr);
    return;
}

/****************************************************************************
* FUNCTION: tfLqmEnable
*
*
* RETURNS:
*   Nothing
*
****************************************************************************/
void tfLqmEnable(
    ttPppVectPtr        pppVectPtr,
    tt32Bit             negotiatedLqrTimerHsec,
    tt32Bit             peerLqrTimerHsec)
{
    TM_UNREF_IN_ARG(pppVectPtr);
    TM_UNREF_IN_ARG(negotiatedLqrTimerHsec);
    TM_UNREF_IN_ARG(peerLqrTimerHsec);
    return;
}
#else /* !TM_PPP_LQM && TM_USE_PPP*/

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */


/* To allow link for builds when TM_PPP_LQM is not defined */
int tvLqmDummy = 0;
#endif /* TM_PPP_LQM */
