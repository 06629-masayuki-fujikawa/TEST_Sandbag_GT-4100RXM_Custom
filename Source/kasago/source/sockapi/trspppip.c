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
 * Description: BSD Sockets Interface (tfSetPppPeerIpAddress)
 *
 * Filename: trspppip.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trspppip.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:35JST $
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
#ifdef TM_LOCK_NEEDED
#include <trglobal.h>
#endif /* TM_LOCK_NEEDED */

#ifdef TM_USE_IPV4
/* 
 * Set the IP address of a Pt2Pt peer (PPP, or SLIP)
 */
int tfSetPppPeerIpAddress ( ttUserInterface interfaceHandle,
                            ttUserIpAddress ifIpAddress )
{
    ttDeviceEntryPtr devEntryPtr;
    int              errorCode;
    
    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
    errorCode = tfValidInterface(devEntryPtr);
    if (errorCode == TM_ENOERROR)
    {
        errorCode = TM_EINVAL; /* assume failure */
        if (tm_ll_is_pt2pt(devEntryPtr))
        {
            tm_call_lock_wait(&(devEntryPtr->devLockEntry));
            if ( !(   tm_ip_dev_conf_flag(devEntryPtr, 0)
                    & TM_DEV_IP_CONFIG ) )
/* If interface is not configured yet */
            {
/*
 * If IP address is non zero, and tfConfigInterface() call had started
 * (non zero devFlag), do not allow IP address to be the same as the
 * configuration IP address. Otherwise:
 */
                if (    tm_ip_zero(ifIpAddress)
                     || (devEntryPtr->devFlag == 0)
                     || tm_ip_not_match( (tt4IpAddress)ifIpAddress,
                                         tm_ip_dev_addr(devEntryPtr, 0) ) )
                {
                    tm_ip_copy( ((tt4IpAddress)ifIpAddress),
                                devEntryPtr->devPt2PtPeerIpAddr );
                    errorCode = TM_ENOERROR;
                }
            }
            else
            {
/* Cannot set Peer PPP ip address after connection */
                errorCode = TM_EISCONN;
            }
            tm_call_unlock(&(devEntryPtr->devLockEntry));
        }
    }
    return (errorCode);
}
#endif /* TM_USE_IPV4 */

/****************************************************************************
 * FUNCTION: tfNgSetPt2PtPeerIpAddress
 *
 * PURPOSE: Next Generation API replaces tfSetPppPeerIpAddress
 *
 * PARAMETERS:
 *   interfaceHandle
 *       Handle to the interface whose peer IP address to be updated
 *   ifIpAddrPtr
 *       IP Address to be updated to the peer 
 * RETURNS:
 *   TM_ENOERROR
 *      success    
 *   TM_EINVAL
 *      the interface handle is invalid, or the device is not point to point
 *   TM_EISCONN
 *      the point to point connection is already established
 *
 * NOTES:
 *
 ****************************************************************************/
int tfNgSetPt2PtPeerIpAddress(
    ttUserInterface interfaceHandle,
    const struct sockaddr_storage TM_FAR * ifIpAddrPtr )
{
#ifdef TM_USE_IPV6
    ttDeviceEntryPtr devEntryPtr;
#endif /* TM_USE_IPV6 */
    int                 errorCode;

/* parameter validation */
    if (ifIpAddrPtr == (ttConstSockAddrStoragePtr)0)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        switch (ifIpAddrPtr->ss_family)
        {
#ifdef TM_USE_IPV4
        case AF_INET:
            errorCode = tfSetPppPeerIpAddress(
                                interfaceHandle,
                                ifIpAddrPtr->addr.ipv4.sin_addr.s_addr);
            break;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
        case AF_INET6:
            devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
            errorCode = tfValidInterface(devEntryPtr);
            if (errorCode == TM_ENOERROR)
            {
                errorCode = TM_EINVAL; /* assume failure */
                if (tm_ll_is_pt2pt(devEntryPtr))
                {
                    tm_call_lock_wait(&(devEntryPtr->devLockEntry));
                    if (!tm_6_dev_is_enabled(devEntryPtr))
                    {
/* If interface is not configured yet */
                        tm_6_ip_copy_structs(
                            ifIpAddrPtr->addr.ipv6.sin6_addr,
                            devEntryPtr->dev6Pt2PtPeerIpAddr );
/* the peer PPP IPv6 address is stored on the device with the embedded
   scope ID */
                        tm_6_dev_scope_addr(
                            &(devEntryPtr->dev6Pt2PtPeerIpAddr), devEntryPtr);
                        errorCode = TM_ENOERROR;
                    }
                    else
                    {
/* Cannot set Peer PPP ip address after interface is configured */
                        errorCode = TM_EISCONN;
                    }
                    tm_call_unlock(&(devEntryPtr->devLockEntry));
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
