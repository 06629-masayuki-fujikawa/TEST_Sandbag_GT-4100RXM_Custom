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
 * Description: tfSetMcastInterface && tfAddMcastRoute API
 * Filename: trsetmci.c
 * Author: Odile
 * Date Created: 08/25/99
 * $Source: source/sockapi/trsetmci.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:20JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h> 
#include <trmacro.h>

#ifdef TM_USE_IPV4

#include <trtype.h>
#include <trproto.h>

/*
 * tfSetMcastInterface Function description:
 * User can specify a default interface to be used to send multicast
 * destination IP packets. This default interface will be used to send
 * outgoing multicast packets, when the user application does not specify an
 * interface (via the IPO_MULTICAST_IF setsockopt()) on the socket.
 *
 * . Call tfAddMcastRoute with IP address 224.0.0.0, and class A
 *   netmask 255.0.0.0, interfaceHandle, and mhomeIndex.
 *
 * Parameters       meaning
 * interfaceHandle  Pointer to interface to be used as default output
 *                  interface for outgoing multicast packets.
 *
 * Return value     meaning
 * TM_ENOERROR      success
 * TM_EINVAL        Invalid interface handle
 * TM_ENETDOWN      Interface not configured yet
 * TM_EADDRNOTAVAIL Interface not configured with the multicast enabled flag
 * TM_EALREADY      Default multicast interface already set
 */
int tfSetMcastInterface (ttUserInterface interfaceId, tt8Bit mhomeIndex)
{
    return tfAddMcastRoute (interfaceId, TM_IP_MCAST_UNSPEC_HOST,
                            TM_IP_FOUR_HIGH_BITS, mhomeIndex);
}

/*
 * tfAddMcastRoute Function description:
 * User can specify an interface to be used to send multicast
 * IP packets for the specified multicast destination address, destination
 * netmask.
 * This interface will be used to send matching outgoing multicast packets,
 * when the user application does not specify an
 * interface (via the IPO_MULTICAST_IF setsockopt()) on the socket.
 *
 *   1. Check that ttUserHandle is a valid configured interface
 *   2. Check that the destination address is multicast, and that it is
 *      multicast enabled.
 *   3. If netmask is a host mask (i.e. host route):
 *      . Map from mcast IP address to mcast ethernet address
 *      . Call tfRtLockAdd to insert a static host route with resolved
 *        Ethernet address, storing the mapped ethernet address in the route.
 *   4. Otherwise (if netmask is not a host mask), call tfRtLockAdd with
 *      TM_RTE_CLONE flag, mcastAddress, and netmask, to insert a static
 *      route.
 *
 * Parameters       meaning
 * interfaceHandle  Pointer to interface to be used as default output
 *                  interface for outgoing multicast packets.
 *
 * Return value     meaning
 * TM_ENOERROR      success
 * TM_EINVAL        Invalid interface handle
 * TM_ENETDOWN      Interface not configured yet
 * TM_EADDRNOTAVAIL Interface not configured with the multicast enabled flag
 *                  or IP address is not multicast.
 * TM_EALREADY      multicast route already set
 */
int tfAddMcastRoute (ttUserInterface interfaceId,
                     ttUserIpAddress mcastAddress,
                     ttUserIpAddress netmask,
                     tt8Bit          mhomeIndex)
{
    ttDeviceEntryPtr    devPtr;
    tt4IpAddress        gatewayIpAddr;
    ttEnetAddressUnion  enetAddr;
    int                 errorCode;
    tt16Bit             flags;

    devPtr = (ttDeviceEntryPtr)interfaceId;

    errorCode = tfValidInterface(devPtr);
    if (errorCode == TM_ENOERROR)
    {
        if ( tm_ip_is_multicast(mcastAddress) && (tm_dev_mcast_enb(devPtr)) )
        {
            if (tm_ip_match(netmask, TM_IP_HOST_MASK))
            {
                (void)tfIpBroadMcastToLan(
                                   mcastAddress,
                                   devPtr,
                                   (tt16Bit) mhomeIndex,
                                   (tt8BitPtr)&(enetAddr.enuAddress));
/*
 * Multicast host entry.
 */
                flags = (tt16Bit)(   TM_RTE_CLONED
                                   | TM_RTE_STATIC
                                   | TM_RTE_LINK_LAYER
                                   | TM_RTE_MCAST );

                errorCode = tf4RtAddHost(
                    devPtr,
                    mcastAddress,
                    (tt8BitPtr)&(enetAddr.enuAddress),
                    TM_ETHER_ADDRESS_LENGTH,
                    (tt16Bit) mhomeIndex,
                    TM_RTE_INF,
                    flags );
            }
            else
            {
                flags = TM_RTE_CLONE | TM_RTE_STATIC;
                tm_ip_copy( tm_ip_and(netmask, mcastAddress),
                            gatewayIpAddr);
                errorCode = tf4RtAddRoute(
                    devPtr,
                    mcastAddress,
                    netmask,
                    gatewayIpAddr,
                    TM_16BIT_ZERO,
                    (tt16Bit) mhomeIndex,
                    0,
                    TM_RTE_INF,
                    flags,
                    TM_8BIT_YES );
            }
        }
        else
        {
            errorCode = TM_EADDRNOTAVAIL;
        }
    }
    return errorCode;
}

#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4SetmciDummy = 0;
#endif /* ! TM_USE_IPV4 */
