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
 * Description: BSD Sockets Interface (tfDelStaticRoute)
 *
 * Filename: trdelsrt.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trdelsrt.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:42:56JST $
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
#include <trglobal.h>

#ifdef TM_USE_IPV4
/*
 * Delete a static route from the routing table
 */
int tfDelStaticRoute (
#ifdef TM_USE_STRONG_ESL
                      ttUserInterface  interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                      ttUserIpAddress  destIpAddress,
                      ttUserIpAddress  destNetMask)
{
    int                 errorCode;
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr    devEntryPtr;
#endif /* TM_USE_STRONG_ESL */ 

#ifdef TM_USE_STRONG_ESL
    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;     
    if ( tfValidInterface(devEntryPtr) != TM_ENOERROR) 
    {
        errorCode = TM_EINVAL;
    }
    else
#endif /* TM_USE_STRONG_ESL */ 
    {
        if (    (tm_ip_not_zero((tt4IpAddress)destIpAddress))
             && (tm_ip_not_zero((tt4IpAddress)destNetMask))
             && (tm_ip_not_zero(tm_ip_net( (tt4IpAddress)destIpAddress,
                                           (tt4IpAddress)destNetMask ))) )
        {
            errorCode = tf4RtDelRoute(
#ifdef TM_USE_STRONG_ESL
                                      devEntryPtr,
#endif /* TM_USE_STRONG_ESL */
                                      (tt4IpAddress)destIpAddress,
                                      (tt4IpAddress)destNetMask,
                                      TM_RTE_STATIC);
        }
        else
        {
            errorCode = TM_EINVAL;
        }
    }
    return (errorCode);
} 
#endif /* TM_USE_IPV4 */

/****************************************************************************
* FUNCTION: tfNgDelStaticRoute
*
* PURPOSE: Next Generation API replaces tfDelStaticRoute
*
* PARAMETERS:
*   None
*
* RETURNS:
*   Nothing
*
* NOTES:
*
****************************************************************************/
int tfNgDelStaticRoute(
#ifdef TM_USE_STRONG_ESL
    ttUserInterface                         interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
    const struct sockaddr_storage TM_FAR *  destIpAddrPtr,
    int                                     prefixLen )
{
#ifdef TM_USE_STRONG_ESL
   ttDeviceEntryPtr devEntryPtr;
#endif /* TM_USE_STRONG_ESL */
#ifdef TM_USE_IPV6
    tt6IpAddressPtr ipv6DestAddressPtr;
    tt6IpAddress    ipv6DestAddress;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV4
    tt4IpAddress    ipv4NetMask;    
#endif /* TM_USE_IPV4 */
    int             errorCode;
    
/* parameter validation */
#ifdef TM_USE_STRONG_ESL
    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;     
    
    if ( tfValidInterface(devEntryPtr) != TM_ENOERROR) 
    {
        errorCode = TM_EINVAL;
    }
    else
#endif /* TM_USE_STRONG_ESL */ 
    {
        if (destIpAddrPtr == (ttConstSockAddrStoragePtr)0)
        {
            errorCode = TM_EINVAL;
        }
        else
        {
            switch (destIpAddrPtr->ss_family)
            {
#ifdef TM_USE_IPV4
            case AF_INET:
                tm_4_netmask(prefixLen, ipv4NetMask);
                errorCode = 
                    tfDelStaticRoute(
#ifdef TM_USE_STRONG_ESL
                                     devEntryPtr,
#endif /* TM_USE_STRONG_ESL */
                                     destIpAddrPtr->addr.ipv4.sin_addr.s_addr, 
                                     ipv4NetMask);
                break;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
            case AF_INET6:
                ipv6DestAddressPtr = 
                        (tt6IpAddressPtr)&(destIpAddrPtr->addr.ipv6.sin6_addr);
                if (    (tm_6_ip_not_zero(ipv6DestAddressPtr))
                     && (prefixLen > 0)
                     && (tm_6_ip_not_zero(ipv6DestAddressPtr)))
                {
                    tm_6_ip_copy_dststruct(ipv6DestAddressPtr,
                                           ipv6DestAddress);
                    ipv6DestAddressPtr = &ipv6DestAddress;
#ifdef TM_4_USE_SCOPE_ID
                    if (tm_6_addr_is_ipv4_mapped(ipv6DestAddressPtr))
                    {
                        tm_4_addr_add_scope(
                            ipv6DestAddressPtr,
                            destIpAddrPtr->addr.ipv6.sin6_scope_id);
                    }
                    else
#endif /* TM_4_USE_SCOPE_ID */
                    {
                        tm_6_addr_add_scope(
                            ipv6DestAddressPtr,
                            destIpAddrPtr->addr.ipv6.sin6_scope_id);
                    }

                    errorCode = tf6RtDelRoute(
#ifdef TM_USE_STRONG_ESL
                                              devEntryPtr,
#endif /* TM_USE_STRONG_ESL */
                                              ipv6DestAddressPtr,
                                              prefixLen,
                                              TM_RTE_STATIC);
                }
                else
                {
                    errorCode = TM_EINVAL;
                }
                break;
#endif /* TM_USE_IPV6 */
            default:
                errorCode = TM_EAFNOSUPPORT;
                break;
            }
        }
    }
    return errorCode;
}
