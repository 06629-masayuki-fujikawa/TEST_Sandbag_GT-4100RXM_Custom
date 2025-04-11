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
 * Description:     DNS Resolver Stub Functions
 * Filename:        stubs/trresolv.c
 * Author:          Jason
 * Date Created:    8/28/01
 * $Source: source/stubs/dnsresol/trresolv.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:45:32JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


#include <trsocket.h> 
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>


int tfDnsInit(int blockingMode)
{
    TM_UNREF_IN_ARG(blockingMode);
    return TM_EPROTONOSUPPORT;
}

#ifdef TM_USE_STOP_TRECK
void tfDnsDeInit(
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                )
{
#ifdef TM_MULTIPLE_CONTEXT
    TM_UNREF_IN_ARG(contextHandle);
#endif /* TM_MULTIPLE_CONTEXT */
    return;
}
#endif /* TM_USE_STOP_TRECK */

int tfDnsSetOption(int optionType, int optionValue)
{
    TM_UNREF_IN_ARG(optionType);
    TM_UNREF_IN_ARG(optionValue);
    return TM_EPROTONOSUPPORT;
}

int tfDnsSetServer(ttUserIpAddress serverIpAddr, int serverNumber)
{
    TM_UNREF_IN_ARG(serverIpAddr);
    TM_UNREF_IN_ARG(serverNumber);
    return TM_EPROTONOSUPPORT;
}

int tfDnsGetHostByName( const char TM_FAR * hostnameStr, 
                        ttUserIpAddressPtr  ipAddressPtr )
{
    TM_UNREF_IN_ARG(hostnameStr);
    TM_UNREF_IN_ARG(ipAddressPtr);
    return TM_EPROTONOSUPPORT;
}

int tfDnsGetHostByAddr( ttUserIpAddress ipAddr,
                        char TM_FAR *   hostnameStr,
                        int             hostnameStrLength ) 
{
    TM_UNREF_IN_ARG(ipAddr);
    TM_UNREF_IN_ARG(hostnameStr);
    TM_UNREF_IN_ARG(hostnameStrLength);
    return TM_EPROTONOSUPPORT;
}

int tfDnsGetNextMailHost( const char TM_FAR *     hostnameStr,
                          ttUserIpAddress         lastIpAddress,
                          unsigned short          lastPreference,
                          ttUserIpAddressPtr      ipAddressPtr,
                          unsigned short TM_FAR * mxPrefPtr )
{
    TM_UNREF_IN_ARG(hostnameStr);
    TM_UNREF_IN_ARG(lastIpAddress);
    TM_UNREF_IN_ARG(lastPreference);
    TM_UNREF_IN_ARG(ipAddressPtr);
    TM_UNREF_IN_ARG(mxPrefPtr);
    return TM_EPROTONOSUPPORT;
}                     

int tfDnsGetMailHost( const char TM_FAR *      hostnameStr,
                      ttUserIpAddressPtr       ipAddressPtr,
                      unsigned short TM_FAR *  mxPrefPtr )
{
    TM_UNREF_IN_ARG(hostnameStr);
    TM_UNREF_IN_ARG(ipAddressPtr);
    TM_UNREF_IN_ARG(mxPrefPtr);
    return TM_EPROTONOSUPPORT;                                                              
}                     

int tfNgDnsGetMailHost(const char TM_FAR *               hostnameStr,
                       const struct addrinfo TM_FAR *    hintsPtr,
                       struct addrinfo TM_FAR * TM_FAR * resPtrPtr)
{
    TM_UNREF_IN_ARG(hostnameStr);
    TM_UNREF_IN_ARG(hintsPtr);
    TM_UNREF_IN_ARG(resPtrPtr);
    return TM_EPROTONOSUPPORT;
}

int tfNgDnsGetNextMailHost(const char TM_FAR *               hostnameStr,
                           struct addrinfo TM_FAR *          lastIpAddrPtr,
                           unsigned short                    lastPref,
                           const struct addrinfo TM_FAR *    hintsPtr,
                           struct addrinfo TM_FAR * TM_FAR * resPtrPtr)
{
    TM_UNREF_IN_ARG(hostnameStr);
    TM_UNREF_IN_ARG(lastIpAddrPtr);
    TM_UNREF_IN_ARG(lastPref);
    TM_UNREF_IN_ARG(hintsPtr);
    TM_UNREF_IN_ARG(resPtrPtr);
    return TM_EPROTONOSUPPORT;
}

int tfNgDnsSetServer(struct sockaddr_storage TM_FAR * serverAddrPtr,
                     int                              serverNumber)
{
    TM_UNREF_IN_ARG(serverAddrPtr);
    TM_UNREF_IN_ARG(serverNumber);
    return TM_EPROTONOSUPPORT;
}

int tfDnsGenerateQuery( ttConstCharPtr        hostnameStr,
                        tt16Bit               queryType,
                        int                   addrFamily,
                        ttDnsCacheEntryPtrPtr entryPtrPtr )
{
    TM_UNREF_IN_ARG(hostnameStr);
    TM_UNREF_IN_ARG(queryType);
    TM_UNREF_IN_ARG(addrFamily);
    TM_UNREF_IN_ARG(entryPtrPtr);
    return TM_EPROTONOSUPPORT;
}

void freeaddrinfo(struct addrinfo TM_FAR * addrInfoPtr)
{
    TM_UNREF_IN_ARG(addrInfoPtr);
    return;
}

int getaddrinfo(const char TM_FAR *               nodeName,
                const char TM_FAR *               serviceName, 
                const struct addrinfo TM_FAR *    hintsPtr,
                struct addrinfo TM_FAR * TM_FAR * resPtrPtr)
{
    TM_UNREF_IN_ARG(nodeName);
    TM_UNREF_IN_ARG(serviceName);
    TM_UNREF_IN_ARG(hintsPtr);
    TM_UNREF_IN_ARG(resPtrPtr);
    return TM_EPROTONOSUPPORT;
}

int getnameinfo( const struct sockaddr TM_FAR * addressPtr,
                 int           addressLength,
                 char TM_FAR * hostname,
                 int           hostnameLength,
                 char TM_FAR * serviceName,
                 int           serviceNameLength,
                 int           flags )
{
    TM_UNREF_IN_ARG(addressPtr);
    TM_UNREF_IN_ARG(addressLength);
    TM_UNREF_IN_ARG(hostname);
    TM_UNREF_IN_ARG(hostnameLength);
    TM_UNREF_IN_ARG(serviceName);
    TM_UNREF_IN_ARG(serviceNameLength);
    TM_UNREF_IN_ARG(flags);
    return TM_EPROTONOSUPPORT;
}

void tfDnsCacheInvalidate(void)
{
    return;
}
