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
 * Description: BSD Sockets Interface (tfGetIpAddress)
 *
 * Filename: trgetip.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trgetip.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:16JST $
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

/* local function prototypes */
#ifdef TM_USE_IPV4
static int tf4GetIpAddress(
    ttUserInterface        interfaceId,
    ttUserIpAddress TM_FAR *ifIpAddressPtr,
    tt16Bit                mHomeIndex);
#endif /* TM_USE_IPV4 */

/****************************************************************************
* FUNCTION: tfNgGetIpAddress
*
* PURPOSE: This function is a replacement for tfGetIpAddress to support IPv6.
*
* PARAMETERS:
*   interfaceHandle:
*       Interface handle passed by the user
*   ifIpAddrPtr:
*       Pointer to the IP address, an output parameter.
*   addrFamily:
*       The address family (i.e. AF_INET for IPv4, AF_INET6 for IPv6) of the
*       address to get the prefix length for.
*   mHomeIndex:
*       multi-home index of IP address on the interface
*
* RETURNS:
*   TM_EINVAL:
*       Invalid value specified for ifIpAddrPtr
*   TM_EAFNOSUPPORT:
*       addrFamily was set to an invalid value for address family
*
* NOTES:
*
****************************************************************************/
int tfNgGetIpAddress(
    ttUserInterface interfaceHandle,
    struct sockaddr_storage TM_FAR * ifIpAddrPtr,
    int addrFamily, unsigned int mHomeIndex )
{
#ifdef TM_USE_IPV6
    ttDeviceEntryPtr    devEntryPtr;
#endif /* TM_USE_IPV6 */
    int                 errorCode;

    errorCode = TM_ENOERROR; /* assume success */

    if (ifIpAddrPtr == TM_SOCKADDR_STORAGE_NULL_PTR)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        tm_bzero(ifIpAddrPtr, sizeof(struct sockaddr_storage));
        ifIpAddrPtr->ss_family = (tt8Bit) addrFamily;
        switch (addrFamily)
        {
#ifdef TM_USE_IPV4
        case AF_INET:
            ifIpAddrPtr->addr.ipv4.sin_len = sizeof(struct sockaddr_in);
            errorCode = tf4GetIpAddress(
                interfaceHandle,
                &(ifIpAddrPtr->addr.ipv4.sin_addr.s_addr),
                (tt16Bit) mHomeIndex);
            break;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
        case AF_INET6:
/* Check interfaceHandle parameter */
            devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
            errorCode = tf6ValidConfigInterface(
                devEntryPtr, (tt16Bit) mHomeIndex);
            if (errorCode == TM_ENOERROR)
            {
                tm_call_lock_wait(&(devEntryPtr->devLockEntry));
/* copy the IPv6 address to the user's buffer */
                tf6SetSockAddrFromIpAddr(
                    &(devEntryPtr->dev6IpAddrArray[mHomeIndex]),
                    devEntryPtr,
                    (struct sockaddr_in6 TM_FAR *) ifIpAddrPtr);
                tm_call_unlock(&(devEntryPtr->devLockEntry));
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

#ifdef TM_USE_IPV4
/* Get the IP address of an interface */
int tfGetIpAddress(ttUserInterface         interfaceId,
                   ttUserIpAddress TM_FAR  *ifIpAddressPtr,
                   unsigned char           mHomeIndex)
{
    return tf4GetIpAddress(
        interfaceId, ifIpAddressPtr, (tt16Bit) mHomeIndex);
}

/* Get the IP address of an interface, supports more than 256 IP aliases
   configured on an interface. */
static int tf4GetIpAddress(
    ttUserInterface         interfaceId,
    ttUserIpAddress TM_FAR  *ifIpAddressPtr,
    tt16Bit                 mHomeIndex)
{
    ttDeviceEntryPtr    devEntryPtr;
    int                 errorCode;

    devEntryPtr = (ttDeviceEntryPtr)interfaceId;
/* Check ifIpAddressPtr parameter */
    if (ifIpAddressPtr != (ttUserIpAddress TM_FAR *)0)
    {
/* Check interfaceId, and mHomeIndex parameter */
        errorCode = tfValidConfigInterface(devEntryPtr, mHomeIndex);
        if (errorCode == TM_ENOERROR)
        {
            tm_call_lock_wait(&(devEntryPtr->devLockEntry));
            tm_ip_copy( tm_ip_dev_addr(devEntryPtr,mHomeIndex),
                        *ifIpAddressPtr );
            tm_call_unlock(&(devEntryPtr->devLockEntry));
        }
    }
    else
    {
        errorCode = TM_EINVAL;
    }
    return (errorCode);
}
#endif /* TM_USE_IPV4 */

