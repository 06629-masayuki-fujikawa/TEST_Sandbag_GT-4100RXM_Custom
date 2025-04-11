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
 * Description: BSD Sockets Interface (tfGetPppPeerIpAddress)
 *
 * Filename: trgpppip.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trgpppip.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:22JST $
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
 * Get the IP address of a point to point peer (PPP, or SLIP)
 */
int tfGetPppPeerIpAddress (ttUserInterface interfaceHandle,
                           ttUserIpAddress TM_FAR *ifIpAddressPtr)
{
    ttDeviceEntryPtr devEntryPtr;
    int              errorCode;
    
    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
    errorCode = tfValidConfigInterface( devEntryPtr, TM_16BIT_ZERO );
    if (errorCode == TM_ENOERROR)
    {
        errorCode = TM_EINVAL; /* assume failure */
        if (ifIpAddressPtr != (ttUserIpAddress TM_FAR *)0)
        {
            if (tm_ll_is_pt2pt(devEntryPtr))
            {
                tm_call_lock_wait( &(devEntryPtr->devLockEntry) );
                tm_ip_copy( tfDeviceDestIpAddress(devEntryPtr),
                            *ifIpAddressPtr );
                tm_call_unlock( &(devEntryPtr->devLockEntry) );
                errorCode = TM_ENOERROR; /* success */
            }
        }
    }
    return errorCode;
}
#endif /* TM_USE_IPV4 */

    
/****************************************************************************
 * FUNCTION: tfNgGetPt2PtPeerIpAddress
 *
 * PURPOSE: Next Generation API replaces tfGetPppPeerIpAddress
 *
 * PARAMETERS:
 *   interfaceHandle
 *       Handle to the interface whose peer IP address is to be retrieved
 *   ifIpAddrPtr
 *       IP Address struct to save the retrieved Peer IP address, 
 *       ifIpAddrPtr->ss_family should be properly set.
 * RETURNS:
 *   TM_ENOERROR
 *      success    
 *   TM_EINVAL
 *      the interface handle is invalid, or the device is not point to point
 *   TM_ENETDOWN
 *      the point to point connection is not configured. 
 *
 * NOTES:
 *
 ****************************************************************************/
int tfNgGetPt2PtPeerIpAddress(
    ttUserInterface interfaceHandle,
    struct sockaddr_storage TM_FAR * ifIpAddrPtr )
{
#ifdef TM_USE_IPV6
    ttDeviceEntryPtr devEntryPtr;
#endif /* TM_USE_IPV6 */
    int              errorCode;
    tt8Bit           family;

/* parameter validation */
    if (ifIpAddrPtr == TM_SOCKADDR_STORAGE_NULL_PTR)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        family = ifIpAddrPtr->ss_family;
        tm_memset(ifIpAddrPtr, 0, sizeof(struct sockaddr_storage));
        ifIpAddrPtr->ss_family = family;
        ifIpAddrPtr->ss_len    = sizeof(struct sockaddr_storage);

        switch (family)
        {
#ifdef TM_USE_IPV4
        case AF_INET:
            errorCode = tfGetPppPeerIpAddress(
                                interfaceHandle,
                                &(ifIpAddrPtr->addr.ipv4.sin_addr.s_addr));
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
                    tm_call_lock_wait( &(devEntryPtr->devLockEntry) );
/* store the sin6_addr */
                    tm_6_ip_copy_structs(
                            devEntryPtr->dev6Pt2PtPeerIpAddr,
                            ifIpAddrPtr->addr.ipv6.sin6_addr);

#ifdef TM_4_USE_SCOPE_ID
                    if (IN6_IS_ADDR_V4MAPPED(
                            &ifIpAddrPtr->addr.ipv6.sin6_addr))
                    {
/* store the sin6_scope_id */
                        tm_4_get_embedded_scope_id(
                            &ifIpAddrPtr->addr.ipv6.sin6_addr, 
                            ifIpAddrPtr->addr.ipv6.sin6_scope_id);
/* unscope the address */
                        tm_4_dev_unscope_addr(
                            &ifIpAddrPtr->addr.ipv6.sin6_addr);
                    }
                    else
#endif /* TM_4_USE_SCOPE_ID */
                    {
/* store the sin6_scope_id */
                        tm_6_get_embedded_scope_id(
                            &ifIpAddrPtr->addr.ipv6.sin6_addr, 
                            ifIpAddrPtr->addr.ipv6.sin6_scope_id);
/* unscope the address */
                        tm_6_dev_unscope_addr(
                            &ifIpAddrPtr->addr.ipv6.sin6_addr);
                    }
                    
                    tm_call_unlock( &(devEntryPtr->devLockEntry) );
                    errorCode = TM_ENOERROR; /* success */
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
