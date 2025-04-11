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
 * Description: stubs for source or destination address selection for IPv6
 *
 * Filename: t6defsel.c
 * Author: 
 * Date Created: 6/07/2007
 * $Source: source/stubs/ipv6/t6defsel.c $
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
#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_IPV6

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>
#include <t6proto.h>

#ifdef TM_6_USE_RFC3484
int tf6SelectSourceAddress(
    ttDeviceEntryPtr        devEntryPtr,
    tt6IpAddressPtr         srcIpAddrPtr,
    ttDeviceEntryPtrPtr     srcDevPtrPtr,
    tt16BitPtr              srcMHomeIndexPtr,
    tt6ConstIpAddressPtr    destIpAddrPtr,
    tt8BitPtr               destIpAddrTypePtr,
    tt8Bit                  flags)
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(srcIpAddrPtr);
    TM_UNREF_IN_ARG(srcDevPtrPtr);
    TM_UNREF_IN_ARG(srcMHomeIndexPtr);
    TM_UNREF_IN_ARG(destIpAddrPtr);
    TM_UNREF_IN_ARG(destIpAddrTypePtr);
    TM_UNREF_IN_ARG(flags);

    return TM_EPROTONOSUPPORT;
}

int tf6PolicyTable (
    ttUserInterface interfaceHandle,
    tt8Bit          operationCode,
    struct in6_addr prefix,
    int             prefixLen,
    tt16Bit         precedence,
    tt16Bit         label)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(operationCode);
    TM_UNREF_IN_ARG(prefix);
    TM_UNREF_IN_ARG(prefixLen);
    TM_UNREF_IN_ARG(precedence);
    TM_UNREF_IN_ARG(label);

    return TM_EPROTONOSUPPORT;
}

int tf6PolicyTableDisplay (
    ttUserInterface             interfaceHandle,
    ttPolicyTableEntryPtrPtr    policyTablePtrPtr,
    tt8Bit                      displayAllFlag,
    tt16Bit                     precedence,
    tt16Bit                     label)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(policyTablePtrPtr);
    TM_UNREF_IN_ARG(displayAllFlag);
    TM_UNREF_IN_ARG(precedence);
    TM_UNREF_IN_ARG(label);

    return TM_EPROTONOSUPPORT;
}

int tf6SelectAddressPreference(
    ttUserInterface  interfaceHandle,
    tt8Bit           flag,
    tt8Bit           optionValue)
{
    TM_UNREF_IN_ARG(interfaceHandle);
    TM_UNREF_IN_ARG(flag);
    TM_UNREF_IN_ARG(optionValue);

    return TM_EPROTONOSUPPORT;
}

int tf6PolicyTableFree (ttPolicyTableEntryPtrPtr policyTablePtrPtr)
{
    TM_UNREF_IN_ARG(policyTablePtrPtr);

    return TM_EPROTONOSUPPORT;
}

int tfPolicyTableInit (ttDeviceEntryPtr devPtr)
{
    TM_UNREF_IN_ARG(devPtr);

    return TM_EPROTONOSUPPORT;
}

int tf6SelectDestinationAddress (
    ttDeviceEntryPtr                  devEntryPtr,
    struct addrinfo TM_FAR * TM_FAR * destIpAddrPtrPtr)
{
    TM_UNREF_IN_ARG(devEntryPtr);
    TM_UNREF_IN_ARG(destIpAddrPtrPtr);

    return TM_EPROTONOSUPPORT;
}
#endif /* TM_6_USE_RFC3484 */

#else /* ! TM_USE_IPV6 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV6 is not defined */
int tv6DefselDummy = 0;
#endif /* ! TM_USE_IPV6 */

/***************** End Of File *****************/
