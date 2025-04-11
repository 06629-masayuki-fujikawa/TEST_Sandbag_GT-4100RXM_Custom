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
 * Description: BSD Sockets Interface (tfDisablePathMtuDisc)
 *
 * Filename: trpmtu.c
 * Author: Odile
 * Date Created: 11/12/99
 * $Source: source/sockapi/trpmtu.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:54JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


/*
 * Include
 */
#include <trsocket.h> /* for TM_PMTU_DISC conditions */
#include <trmacro.h> /* for TM_PMTU_DISC & LINT_UNUSED_HEADER */
#include <trtype.h>
#include <trproto.h>

#if (defined(TM_PMTU_DISC) && defined(TM_USE_IPV4))


/*
 * Called from the socket interface to add a host static routing entry with
 * a set Path MTU, and a disable Path MTU discovery flag, which will
 * disable path MTU discovery for packets sent to that destination.
 * If the user does not specify the Path MTU, then the default
 * outgoing interface MTU will be used.
 * Parameters:
 * destIpAddress: host destination IP address
 * pathMtu:       If 0, use interface MTU. Otherwise use this MTU.
 * Return values:
 * TM_EINVAL       If destIpAddress is zero
 * TM_EHOSTUNREACH If there is no route to the destination IP addres
 * TM_EPERM        Route is direct, no need to enable Path MTU discovery.
 * TM_ENOBUFS      Not enough memory to allocate new routing entry
 */
int tfDisablePathMtuDisc ( 
#ifdef TM_USE_STRONG_ESL
                           ttUserInterface     interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                           ttUserIpAddress     destIpAddress,
                           unsigned short      pathMtu )
{
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr    devEntryPtr;
#endif /* TM_USE_STRONG_ESL */ 
    int                 errorCode;

#ifdef TM_USE_STRONG_ESL
    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;     
    
    if ( tfValidInterface(devEntryPtr) != TM_ENOERROR) 
    {
        errorCode = TM_EINVAL;
    }
#endif /* TM_USE_STRONG_ESL */ 
    {
        if ( tm_ip_not_zero(destIpAddress) )
        {
            errorCode = tfRtNextHopMtu(
#ifdef TM_USE_STRONG_ESL
                                  devEntryPtr,
#endif /* TM_USE_STRONG_ESL */
                                  destIpAddress, pathMtu, (ttIpHeaderPtr)0);
        }
        else
        {
            errorCode = TM_EINVAL;
        }
    }
    return errorCode;
}

#endif /* TM_PMTU_DISC && TM_USE_IPV4 */

#if (defined(TM_PMTU_DISC) || defined(TM_6_PMTU_DISC))
/*
 * tfNgDisablePathMtuDisc Function Description
 * Disables Path MTU discovery and sets the path MTU for the IPv4 or IPv6
 * destination address to the specified value. ([RFC1981].R5.6:10)
 *
 * Parameters
 * Parameter      Description
 * destIpAddrPtr  Pointer to the destination address to set the path MTU value
 *                for.
 * pathMtu        New path MTU value for the specified destination.
 *
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Path MTU successfully updated.
 * TM_ENOPROTOOPT   Path MTU discovery not enabled for the specified address
 *                  type.
 * TM_EINVAL        Destination address pointer is null.
 * TM_EINVAL        Destination address is zero.
 * TM_EAFNOSUPPORT  Destination address is not either IPv6 or IPv4.
 *
 * 1. Verify that destination pointer is not null.  If null, return TM_EINVAL.
 * 2. For an IPv4 address:
 *     2.1. If PMTU discovery is not enabled for IPv4, return TM_ENOPROTOOPT.
 *     2.2. Verify that address is non-zero.
 *     2.3. Update IPv4 path MTU value and disable future PMTU discovery for
 *          this destination.
 * 3. For an IPv6 address:
 *     3.1. If PMTU discovery is not enabled for IPv6, return TM_EPROTONOOPT.
 *     3.2. Update IPv6 path MTU value and disable future PMTU discovery for
 *          this destination.
 * 4. If address is neither IPv6 nor IPv4, return TM_EAFNOSUPPORT.
 */ 
int tfNgDisablePathMtuDisc(
#ifdef TM_USE_STRONG_ESL
                           ttUserInterface                  interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                           struct sockaddr_storage TM_FAR * destIpAddrPtr,
                           int                              pathMtu)
{
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr    devEntryPtr;
#endif /* TM_USE_STRONG_ESL */ 
    int                 errorCode;

    
#ifdef TM_USE_STRONG_ESL
    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;     
    
    if ( tfValidInterface(devEntryPtr) != TM_ENOERROR) 
    {
        errorCode = TM_EINVAL;
    }
    else
#endif /* TM_USE_STRONG_ESL */ 
    {
/*
 * 1. Verify that destination pointer is not null.  If null, return
 *    TM_EINVAL.
 */
        if (destIpAddrPtr != (struct sockaddr_storage TM_FAR *)0)
        {
            errorCode = TM_ENOERROR;
/* 2. For an IPv4 address: */
#ifdef TM_USE_IPV4
            if (destIpAddrPtr->ss_family == AF_INET)
            {
                
/*     2.1. If PMTU discovery is not enabled for IPv4, return TM_ENOPROTOOPT. */
#ifndef TM_PMTU_DISC
                errorCode = TM_ENOPROTOOPT;
#else /* !TM_PMTU_DISC */
                
/*     2.2. Verify that address is non-zero. */
                if (tm_ip_zero(destIpAddrPtr->addr.ipv4.sin_addr.s_addr))
                {
                    errorCode = TM_EINVAL;
                }
                else
                {
/*
 *     2.3. Update IPv4 path MTU value and disable future PMTU discovery for
 *          this destination.
 */
                    errorCode = tfRtNextHopMtu(
#ifdef TM_USE_STRONG_ESL
                        devEntryPtr,
#endif /* TM_USE_STRONG_ESL */
                        destIpAddrPtr->addr.ipv4.sin_addr.s_addr,
                        (tt16Bit) pathMtu,
                        (ttIpHeaderPtr) 0 );
                }
#endif /* TM_PMTU_DISC */
            }
            else
#endif /* TM_USE_IPV4 */
            {
                
/* 3. For an IPv6 address: */
#ifdef TM_USE_IPV6
                if (destIpAddrPtr->ss_family == AF_INET6)
                {
/*     3.1. If PMTU discovery is not enabled for IPv6, return TM_EPROTONOOPT. */
#ifndef TM_6_PMTU_DISC
                    errorCode = TM_ENOPROTOOPT;
#else /* !TM_6_PMTU_DISC */
/*
 *     3.2. Update IPv6 path MTU value and disable future PMTU discovery for
 *          this destination.
 */
                    errorCode = tf6RtNextHopMtu(
#ifdef TM_USE_STRONG_ESL
                        devEntryPtr,
#endif /* TM_USE_STRONG_ESL */
                        &destIpAddrPtr->addr.ipv6.sin6_addr,
                        (tt16Bit) pathMtu,
                        TM_8BIT_YES); /* is static == true */
#endif /* TM_6_PMTU_DISC */
                }
                else
#endif /* TM_USE_IPV6 */
                {
/* 4. If address is neither IPv6 nor IPv4, return TM_EAFNOSUPPORT. */
                    errorCode = TM_EAFNOSUPPORT;
                }
            }
        }
        else
        {
            errorCode = TM_EINVAL;
        }
    }
    return errorCode;
}
#endif /* TM_PMTU_DISC || TM_6_PMTU_DISC */

#ifdef TM_USE_IPV6
/*
 * tf6GetPathMtu Function Description
 * Retrieves the current Path MTU for the specified destination address.  This
 * path MTU does not take into account overhead added by IPSec, but does
 * include protocol tunneling overhead. [IPV6REQ].R2.12:10
 *
 * Parameters
 * Parameter     Description
 * destIpAddrPtr IPv6 address of the destination to get the Path MTU for.
 * pathMtuPtr    Set to the current IPv6 Path MTU upon return, if no error
 *               occurred.
 *
 * Returns
 * Value        Meaning
 * TM_ENOERROR  IPv6 Path MTU successfully returned.
 * TM_EINVAL    Destination address is not an IPv6 address.
 * TM_EINVAL    Destination address pointer is null.
 * TM_EINVAL    Pointer to Path MTU result is null.
 * TM_ENOENT    No route to the destination IPv6 address was found.
 *
 * 1. Init the routing cache entry pointer to null.  If this is non-null at
 *    the end of the routine, we need to release ownership of the entry.
 *    Otherwise, no entry was found.
 * 2. Verify that the address passed in by the user is not a null pointer and
 *    is an IPv6 address.  If not, return TM_EINVAL.
 * 3. Verify that the Path MTU result pointer passed by the user is not null.
 *    If not, return TM_EINVAL.
 * 4. Get routing entry for the destination IP address.  If this fails, return
 *    TM_ENOENT.
 * 5. Save MTU from routing entry.
 * 6. If a routing entry was found, release ownership of it now.
 */ 
int tf6GetPathMtu(
#ifdef TM_USE_STRONG_ESL
                  ttUserInterface                  interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                  struct sockaddr_storage TM_FAR * destIpAddrPtr,
                  int TM_FAR *                     pathMtuPtr)
{

#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr    devEntryPtr;
#endif /* TM_USE_STRONG_ESL */ 
    ttLinkLayerEntryPtr linkLayerEntryPtr;
    ttRtCacheEntry      rtCache;
    ttRteEntryPtr       rtePtr;
    int                 errorCode;
    int                 pathMtu;

#ifdef TM_USE_STRONG_ESL
    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;     
    
    if ( tfValidInterface(devEntryPtr) != TM_ENOERROR) 
    {
        errorCode = TM_EINVAL;
    }
    else
#endif /* TM_USE_STRONG_ESL */ 
    {
/*    
 * 2. Verify that the address passed in by the user is not a null pointer and
 *    is an IPv6 address.  If not, return TM_EINVAL.
 * 3. Verify that the Path MTU result pointer passed by the user is not null.
 *    If not, return TM_EINVAL.
 */
        if (    (destIpAddrPtr == (struct sockaddr_storage TM_FAR *) 0)
             || (pathMtuPtr == (int TM_FAR *) 0) )
        {
            errorCode = TM_EINVAL;
        }
        else
        {
/*
 * 1. Init the routing cache entry pointer to null.  If this is non-null at
 *    the end of the routine, we need to release ownership of the entry.
 *    Otherwise, no entry was found.
 */
            tm_bzero(&rtCache, sizeof(ttRtCacheEntry));
/*
 * 4. Get routing entry for the destination IP address.  If this fails, return
 *    TM_ENOENT.
 */    
            tm_6_ip_copy(&destIpAddrPtr->addr.ipv6.sin6_addr,
                         &rtCache.rtcDestIpAddr);
#ifdef TM_USE_STRONG_ESL
            rtCache.rtcDevPtr = devEntryPtr;
#endif /* TM_USE_STRONG_ESL */
            errorCode = tfRtGet(&rtCache);

            if (errorCode == TM_ENOERROR)
            {
                rtePtr = rtCache.rtcRtePtr;
                
/* 5. Save MTU from routing entry (or, if disabled, just use the minimum). */
#ifdef TM_6_PMTU_DISC
                pathMtu = rtePtr->rteMtu;
#else /* !TM_6_PMTU_DISC */
                pathMtu = TM_6_PMTU_MINIMUM;
#endif /* TM_6_PMTU_DISC */

/* Adjust for IPv6 header. */
                pathMtu -= TM_6_IP_MIN_HDR_LEN;

/* Adjust for tunnel headers, if any. */
                linkLayerEntryPtr = rtePtr->rteDevPtr->devLinkLayerProtocolPtr;
                if (linkLayerEntryPtr->lnkLinkLayerProtocol ==
                                                       TM_LINK_LAYER_V6_TUNNEL)
                {
                    pathMtu -= linkLayerEntryPtr->lnkHeaderLength;
                }

                *pathMtuPtr = pathMtu;
                
            }
  
/* 6. If a routing entry was found, release ownership of it now. */
            if (rtCache.rtcRtePtr != TM_RTE_NULL_PTR)
            {
                tfRtCacheUnGet(&rtCache);
            }
        }
    }
    
    return errorCode;
}
#endif /* TM_USE_IPV6 */
    
