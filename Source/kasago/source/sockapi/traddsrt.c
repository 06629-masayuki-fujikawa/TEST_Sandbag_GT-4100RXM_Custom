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
 * Description: BSD Sockets Interface (tfAddStaticRoute)
 *
 * Filename: traddsrt.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/traddsrt.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:42:42JST $
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
#ifdef TM_SINGLE_INTERFACE_HOME
/* Route IP address does not match my zero host IP address */
#define tm_rt_network_not_match(devPtr, ipAddr, ifaceMhomePtr) \
        (    ((~devPtr->devNetMask) & ipAddr ) \
          || tm_ip_match(devPtr->devNetMask, TM_IP_HOST_MASK) )
/* Route IP address does not match my dbroad cast IP address */
#define tm_rt_dbroad_not_match(devPtr, ipAddr, ifaceMhomePtr) \
        (    !tm_4_ll_is_broadcast(devPtr) \
          || tm_ip_not_match(ipAddr, devPtr->devDBroadIpAddr))

#else /* !TM_SINGLE_INTERFACE_HOME */
/* Route IP address does not match mhome zero host IP address */
#define tm_rt_network_not_match(devPtr, ipAddr, ifaceMhomePtr) \
        ( (tfMhomeNetMatch(devPtr, ipAddr, ifaceMhomePtr) != TM_ENOERROR) \
        ||((~(devPtr->devNetMaskArray[*ifaceMhomePtr]))&ipAddr) \
        || tm_ip_match(devPtr->devNetMaskArray[*ifaceMhomePtr],  \
                       TM_IP_HOST_MASK) )
/* Route IP address does not match mhome dbroad cast IP address */
#define tm_rt_dbroad_not_match(devPtr, ipAddr, ifaceMhomePtr) \
        (    !tm_4_ll_is_broadcast(devPtr) \
          || (tfMhomeDBroadMatch(devPtr, ipAddr, ifaceMhomePtr) \
                                 != TM_ENOERROR) )

#endif /* TM_SINGLE_INTERFACE_HOME */
/*
 * Called from the socket interface to add a static route.
 * Parameters:
 * InterfaceId: pointer to the device entry
 * destIpAddress: destination IP address
 * destNetMask: destination Network mask
 * gateway: IP address of the gateway for this route
 * Hops: Number of gateway hops to destination
 * Return values:
 * TM_EINVAL if any of the first 4 parameters is zero
 * for additional error values see tfRtAddEntry()
 */
int tfAddStaticRoute (ttUserInterface interfaceId,
                      ttUserIpAddress destIpAddress,
                      ttUserIpAddress destNetMask,
                      ttUserIpAddress gateway,
                      int             hops)
{
    ttDeviceEntryPtr    devPtr;
    int                 errorCode;
    tt16Bit             flags;
#ifndef TM_SINGLE_INTERFACE_HOME
    tt16Bit             mHomeIndex;
#endif /* !TM_SINGLE_INTERFACE_HOME */

#ifndef TM_SINGLE_INTERFACE_HOME
    mHomeIndex = 0;
#endif /* !TM_SINGLE_INTERFACE_HOME */
    devPtr = (ttDeviceEntryPtr)interfaceId;
    errorCode = tfValidConfigInterface( devPtr, TM_16BIT_ZERO );
    if (errorCode == TM_ENOERROR)
    {
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
        if (  (tm_ip_not_zero(destIpAddress))
           && (tm_ip_not_zero(destNetMask))
           && (tm_ip_not_zero(tm_ip_net( (tt4IpAddress)destIpAddress,
                                         (tt4IpAddress)destNetMask )))
           && (   (tm_ip_not_zero(gateway))
               || (!(tm_4_ll_is_lan(devPtr))) )
           && (  tm_ll_is_pt2pt(devPtr)
               || 
/*
 * BUG ID 201: Broadcast and zero host address should not be used as
 * static route.
 */
                 (  (  (tm_ip_match(TM_IP_LOOP_BACK_ADDR, gateway))
                    || (tm_rt_network_not_match(devPtr,gateway,&mHomeIndex)) )
                 && ( tm_rt_dbroad_not_match(devPtr, gateway, &mHomeIndex)) )
              )
           && (hops > 0) )
/* PRQA L:L1 */
        {
            if (tm_ip_is_loop_back(destIpAddress))
            {
                flags = TM_RTE_REJECT|TM_RTE_INDIRECT|TM_RTE_STATIC;
            }
            else
            {
                flags = TM_RTE_INDIRECT|TM_RTE_STATIC;
            }
            errorCode = tf4RtAddRoute((ttDeviceEntryPtr)interfaceId, 
                                      destIpAddress, destNetMask,
                                      gateway, TM_16BIT_ZERO,
                                      TM_16BIT_ZERO, hops , TM_RTE_INF,
                                      flags, TM_8BIT_YES);
        }
        else
        {
            errorCode=TM_EINVAL;
        }
    }
    return errorCode;
}
#endif /* TM_USE_IPV4 */

/****************************************************************************
* FUNCTION: tfNgAddStaticRoute
*
* PURPOSE: Next Generation API replaces tfAddStaticRoute
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
int tfNgAddStaticRoute(
    ttUserInterface interfaceHandle,
    const struct sockaddr_storage TM_FAR * destIpAddrPtr,
    int prefixLen,
    const struct sockaddr_storage TM_FAR * gatewayPtr,
    int hops )
{
    int                 errorCode;
#ifdef TM_USE_IPV6
    tt6IpAddress        scopedDestIpAddr;
    tt6IpAddress        scopedGwyIpAddr;
    ttDeviceEntryPtr    devPtr;
    tt16Bit             flags;
    tt16Bit             mHomeIndex;
#endif /* TM_USE_IPV6 */

    errorCode   = TM_ENOERROR; /* assume success */

/* parameter validation */
    if ((destIpAddrPtr == (ttConstSockAddrStoragePtr)0)
        || (gatewayPtr == (ttConstSockAddrStoragePtr)0)
        || (destIpAddrPtr->ss_family != gatewayPtr->ss_family))
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        switch (destIpAddrPtr->ss_family)
        {
#ifdef TM_USE_IPV4
        case AF_INET:
            if (prefixLen > 32)
            {
                errorCode = TM_EINVAL;
            }
            else
            {
                errorCode = tfAddStaticRoute(
                    interfaceHandle,
                    destIpAddrPtr->addr.ipv4.sin_addr.s_addr,
                    tvRt32ContiguousBitsPtr[prefixLen],
                    gatewayPtr->addr.ipv4.sin_addr.s_addr,
                    hops);
            }
            break;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
        case AF_INET6:
            mHomeIndex = TM_6_RT_RESERVED_MHOME_INDEX;
            devPtr = (ttDeviceEntryPtr)interfaceHandle;
/* Check interfaceHandle parameter */
            errorCode = tfValidInterface(devPtr);
            if ((errorCode != TM_ENOERROR) ||
                (prefixLen < 3) || (prefixLen > 128))
            {
                errorCode = TM_EINVAL;
            }
            else
            {
                if (!(IN6_IS_ADDR_UNSPECIFIED(
                          &(destIpAddrPtr->addr.ipv6.sin6_addr)))
                    && (   (!(IN6_IS_ADDR_UNSPECIFIED(
                             &(gatewayPtr->addr.ipv6.sin6_addr))))
                        || (!(tm_6_ll_is_lan(devPtr))) )
                    && (hops > 0) )
                {
                    if (IN6_IS_ADDR_LOOPBACK(
                            &(destIpAddrPtr->addr.ipv6.sin6_addr)))
                    {
                        flags = TM_RTE_REJECT|TM_RTE_INDIRECT|TM_RTE_STATIC;
                    }
                    else
                    {
                        flags = TM_RTE_INDIRECT|TM_RTE_STATIC;
                    }
                    tm_6_ip_copy_structs(
                            destIpAddrPtr->addr.ipv6.sin6_addr,
                            scopedDestIpAddr);
                    tm_6_addr_add_scope(
                        &(scopedDestIpAddr),
                        destIpAddrPtr->addr.ipv6.sin6_scope_id);
                    tm_6_ip_copy_structs(
                            gatewayPtr->addr.ipv6.sin6_addr, 
                            scopedGwyIpAddr);
                    tm_6_addr_add_scope(
                        &(scopedGwyIpAddr),
                        gatewayPtr->addr.ipv6.sin6_scope_id);
                    errorCode = tf6RtAddRoute(
                        devPtr,
                        &(scopedDestIpAddr),
                        prefixLen,
                        &(scopedGwyIpAddr),
                        TM_16BIT_ZERO, /* tag */
                        mHomeIndex,
                        hops,
                        TM_RTE_INF,
                        flags,
                        TM_8BIT_YES);
                }
                else
                {
                    errorCode=TM_EINVAL;
                }
            }
            break;
#endif /* TM_USE_IPV6 */
        
        default:
            errorCode = TM_EAFNOSUPPORT;
            break;
        }
    }

    return errorCode;
}


