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
 * Description: tfSetReachable API for NUD
 *
 * Filename: tfsetrch.c
 * Author: Ed Remmell
 * Date Created:
 * $Source: source/sockapi/trsetrch.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2014/12/05 14:19:23JST $
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

/* constant declarations and enumerations */

/* type definitions */

/* local function prototypes */

/* local variable definitions */

/* static const tables */

/* macro definitions */

/****************************************************************************
 * FUNCTION: tfSetReachable
 *
 * PURPOSE: 
 *   This function is to notify NUD of reachability confirmations from ULPs.
 *   This is a public API wrapping the internal API tfNudSetReachable, 
 *   implements [RFC2461].R7.3.1:10, [RFC2461].R7.3.1:30, 
 *              [RFC2461].R7.3.1:40)
 * INTERNALS
 *  1.  Validate input pointers, return TM_EINVAL if addrPtr is NULL
 *  2.  If the IP address is multicast, then we lie and tell the caller 
 *      the new reachable time is the default value (i.e. 
 *      TM_6_ND_DEF_REACHABLE_TIME for V6 and tvArpTimeout for v4), 
 *      and return TM_ENOERROR.
 *  3.  if the address pointed to by addrPtr is an IPV6 address, Embed 
 *      the scope ID  into ipAddr 
 *      3.1.  create a copy of the IPV6 address, so we can embed the scope ID
 *      3.2  scope the address local if local
 *          3.2.1.  if interface is invalid (call tf6ValidConfigInterface), 
 *                  return  TM_EINVAL
 *          3.2.2.  scope the address,
 *  4.  check whether an matching ARP entry exists
 *  5.  obtains the route entry by calling tfRtGet(),
 *      5.1.  If it is an indirect routing entry (i.e. a gateway), then get 
 *            the cloned routing entry for the gateway:
 *      5.2.  if the found ARP entry is not resolved, return TM_ENOENT:
 *      5.3.  Set reachable 
 *          5.3.1.  set the state of the matching entry to be REACHABLE
 *                  by calling  tfNudSetReachable() with the route obtained
 *                  (NOTE: this might be a cloned routing entry for a 
 *                  gateway, see 4.1 above).
 *          5.3.2.  if reachableMsecPtr is not NULL, store the reachable
 *                  time, (NOTE: the unit of this value is in milliseconds).
 *  6.  call tfRtCacheUnGet to decrease the owner count of the route entry.
 *
 * PARAMETERS:
 *   addrPtr
 *      IP address of a node that the application has received reachability
 *      confirmation in the form of "forward progress", it could be a 
 *      neighbor, or a remote node, connected through router(s). 
 *      If a neighbor, the  reachability for the neighbor will be set to 
 *      REACHABLE, else, the reachability for the first router in the path
 *      to the node is set to REACHABLE.
 *     ([RFC2461].R7.3.1:30, [RFC2461].R7.3.1:40)
 *   interface
 *      Interface at which the node is connected, needed only when addrPtr
 *      is a local scope address, for global address, this parameter is
 *      ignored
 *   reachableMsecPtr
 *      Carry back the new reachable time of the matching ARP entry, 
 *      in milliseconds, if NULL, no value will be carried back.
 *
 * RETURNS:
 *   TM_ENOERROR
 *      if successful
 *   TM_EINVAL
 *      addrPtr was NULL, or addrPtr points to a local scope IPV6 address
 *      but interface was invalid.
 *   TM_ENOENT
 *      No ARP entry exist, send some packets first to the destination
 *      IP address to cause the ARP entry to be created.
 *
 * NOTES:
 *
 ****************************************************************************/
int tfSetReachable(
    struct sockaddr_storage TM_FAR *    addrPtr,
    ttUserInterface                     interfaceHandle,
    ttUser32Bit TM_FAR *                reachableMsecPtr)
{

    int             errorCode;
    ttRteEntryPtr   rtePtr;
    ttRtCacheEntry  rtCache;
    ttArpMapping    arpMapping;
    
/* avoid compiler warning of unused formal parameter */
#ifndef TM_USE_IPV6
    TM_UNREF_IN_ARG(interfaceHandle);
#endif /* TM_USE_IPV6 */

    errorCode   = TM_ENOERROR;
    tm_bzero(&rtCache, sizeof(ttRtCacheEntry));
    
/*  1.  Validate input pointers, return TM_EINVAL if addrPtr is NULL*/
    if (    (addrPtr == (struct sockaddr_storage TM_FAR *)0 )
#ifdef TM_USE_STRONG_ESL
         || (tfValidInterface(interfaceHandle) != TM_ENOERROR)
#endif /* TM_USE_STRONG_ESL */
       )
    {
        errorCode = TM_EINVAL;
        goto setReachableFinish;
    }
/*
 *  2.  If the IP address is multicast, then we lie and tell the caller 
 *      the new reachable time is the default value (i.e. 
 *      TM_6_ND_DEF_REACHABLE_TIME for V6 and tvArpTimeout for v4), 
 *      and return TM_ENOERROR.
 */
#ifdef TM_USE_IPV6
/* PRQA: QAC Message 850: [U] Macro argument is empty. */
/* PRQA S 850 L1 */
    if (   (   (addrPtr->ss_family == AF_INET6)
            && (IN6_IS_ADDR_MULTICAST(&addrPtr->addr.ipv6.sin6_addr)))
#ifdef TM_USE_IPV4
        || (   (addrPtr->ss_family == AF_INET)
            && (tm_ip_is_local_mcast_address(addrPtr->addr.ipv4.sin_family)))
#endif /* dual */
        )
/* PRQA L:L1 */
    {
        if (reachableMsecPtr != (ttUser32Bit TM_FAR *)0)
        {
            *reachableMsecPtr = TM_6_ND_DEF_REACHABLE_TIME;
        }
        goto setReachableFinish;
    }
#else /* !TM_USE_IPV6 */
    if( tm_ip_is_local_mcast_address(addrPtr->addr.ipv4.sin_addr.s_addr) )
    {
        if (reachableMsecPtr != (ttUser32Bit TM_FAR *)0)
        {
            *reachableMsecPtr = tm_context(tvArpTimeout);
        }
        goto setReachableFinish;
    }
#endif /* !TM_USE_IPV6 */
/*
 *  3.  if the address pointed to by addrPtr is an IPV6 address, Embed 
 *      the scope ID into  rtCache.rtcDestIpAddr
 */
#ifdef TM_USE_IPV6
/*
 *      3.1.  create a copy of the IPV6 address, so we can embed the scope ID
 */
    tm_6_ip_copy_structs(addrPtr->addr.ipv6.sin6_addr, 
                         rtCache.rtcDestIpAddr);

    if (addrPtr->ss_family == AF_INET6)
    {
/*      3.2  scope the address local if link-local */
        if ( IN6_IS_ADDR_LINKLOCAL(&rtCache.rtcDestIpAddr) )
        {
/*
 *          3.2.1.  if interface is invalid (call tf6ValidConfigInterface), 
 *                   return  TM_EINVAL
 */
            errorCode = tf6ValidConfigInterface(
                                        (ttDeviceEntryPtr)interfaceHandle,
                                        TM_MAX_IPS_PER_IF );
            if( errorCode != TM_ENOERROR )
            {
                errorCode = TM_EINVAL;
                goto setReachableFinish;
            }
/*          3.2.2.  scope the address, */
            tm_6_dev_scope_addr(&rtCache.rtcDestIpAddr, 
                                ((ttDeviceEntryPtr)interfaceHandle));
        }
    }
#else /* TM_USE_IPV6 */
    tm_ip_copy(addrPtr->addr.ipv4.sin_addr.s_addr, 
               rtCache.rtcDestIpAddr);
#endif /* TM_USE_IPV6 */

/*  4.  check whether an matching ARP entry exists */
#ifdef TM_USE_IPV6
    arpMapping.arpmCommand = TM_6_ARP_CHK_BY_IP;
    tm_6_ip_copy_structs(rtCache.rtcDestIpAddr,
                         arpMapping.arpm6IpAddress);
#else /* !TM_USE_IPV6 */
    arpMapping.arpmCommand = TM_4_ARP_CHK_BY_IP;
    rtCache.rtcDestIpAddr = rtCache.rtcDestIpAddr;
#endif /* !TM_USE_IPV6 */
#ifdef TM_USE_STRONG_ESL
    arpMapping.arpmDevPtr = (ttDeviceEntryPtr)interfaceHandle;
#endif /* TM_USE_STRONG_ESL */
    errorCode = tfRtArpCommand(&arpMapping);
    if (errorCode != TM_ENOERROR)
    {
        errorCode = TM_ENOENT;
        goto setReachableFinish;
    }

/*  5.  obtains the route entry by calling tfRtGet(), */
#ifdef TM_USE_STRONG_ESL
    rtCache.rtcDevPtr = (ttDeviceEntryPtr)interfaceHandle;
#endif /* TM_USE_STRONG_ESL */
    errorCode = tfRtGet(&rtCache);
    rtePtr = rtCache.rtcRtePtr;
    if (  (errorCode != TM_ENOERROR)
        ||(rtePtr == TM_RTE_NULL_PTR) )
    {
        errorCode = TM_ENOENT;
        goto setReachableFinish;
    }
    else
    {
/*
 *      5.1.  If it is an indirect routing entry (i.e. a gateway), then get 
 *            the cloned routing entry for the gateway:
 */
        if ( rtePtr->rteFlags & TM_RTE_INDIRECT )
        {
            rtePtr = rtCache.rtcRteClonedPtr;
        }
/*      5.2.  if the found ARP entry is not resolved, return TM_ENOENT: */
        if (tm_16bit_bits_not_set(rtePtr->rteFlags, TM_RTE_LINK_LAYER))
        {
            errorCode = TM_ENOENT;
            goto setReachableFinish;
        }
/*      5.3.  Set reachable */
        else
        {
/*
 *          5.3.1.  set the state of the matching entry to be REACHABLE
 *                  by calling  tfNudSetReachable() with the route obtained
 *                  (NOTE: this might be a cloned routing entry for a 
 *                  gateway, see 4.1 above).
 */
            tfNudSetReachable(rtePtr);
/*
 *          5.3.2.  if reachableMsecPtr is not NULL, store the reachable
 *                  time, (NOTE: the unit of this value is in milliseconds).
 */
            if (reachableMsecPtr != (ttUser32Bit TM_FAR *)0 )
            {

#ifdef TM_USE_IPV6
                if (addrPtr->ss_family == AF_INET6)
                {
                    *reachableMsecPtr = rtePtr->rteDevPtr->dev6ReachableTime;
                }
#endif /* TM_USE_IPV6 */
                if (addrPtr->ss_family == AF_INET)
                {
                    *reachableMsecPtr = tm_context(tvArpTimeout);
                }
            }
        }
    }
setReachableFinish:
/*  6.  call tfRtCacheUnGet to decrease the owner count of the route entry. */
    if (rtCache.rtcRtePtr != TM_RTE_NULL_PTR)
    {
       tfRtCacheUnGet(&rtCache);
    }
    return errorCode;
}


/***************** End Of File *****************/
