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
 * Description: BSD Sockets Interface (tfGetNetMask)
 *
 * Filename: trgetmsk.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trgetmsk.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:17JST $
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

/****************************************************************************
* FUNCTION: tfNgGetPrefixLen
*
* PURPOSE: This function is a replacement for tfGetNetMask to support IPv6.
*
* PARAMETERS:
*   interfaceHandle:
*       Interface handle passed by the user
*   prefixLenPtr:
*       Pointer to the prefix length, an output parameter.
*   addrFamily:
*       The address family (i.e. AF_INET for IPv4, AF_INET6 for IPv6) of the
*       address to get the prefix length for.
*   mHomeIndex:
*       multi-home index of IP address on the interface
*
* RETURNS:
*   TM_EINVAL:
*       Invalid value specified for prefixLenPtr
*   TM_EAFNOSUPPORT:
*       addrFamily was set to an invalid value for address family
*
* NOTES:
*
****************************************************************************/
int tfNgGetPrefixLen(
    ttUserInterface interfaceHandle,
    ttUser8Bit TM_FAR * prefixLenPtr,
    int addrFamily,
    unsigned int mHomeIndex )
{
    ttDeviceEntryPtr    devEntryPtr;
    int                 errorCode;

    errorCode = TM_ENOERROR; /* assume success */

    if (prefixLenPtr == (ttUser8Bit TM_FAR *) 0)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
        switch (addrFamily)
        {
#ifdef TM_USE_IPV4
        case AF_INET:
/* Check interfaceHandle parameter */
            errorCode = tfValidConfigInterface(
                devEntryPtr, (tt16Bit) mHomeIndex);
            if (errorCode == TM_ENOERROR)
            {
                *prefixLenPtr = tm_4_ip_dev_prefixLen(devEntryPtr,mHomeIndex);
            }
            break;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
        case AF_INET6:
/* Check interfaceHandle parameter */
            errorCode = tf6ValidConfigInterface(
                devEntryPtr, (tt16Bit) mHomeIndex);
            if (errorCode == TM_ENOERROR)
            {
                *prefixLenPtr = devEntryPtr->dev6PrefixLenArray[mHomeIndex];
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
/* Get the Net Mask of an interface */
int tfGetNetMask(
    ttUserInterface         interfaceId,
    ttUserIpAddress TM_FAR  *netMaskPtr,
    unsigned char           mHomeIndex)
{
    ttDeviceEntryPtr    devEntryPtr;
    int                 errorCode;
    
    devEntryPtr = (ttDeviceEntryPtr)interfaceId;
/* Check netMaskPtr parameter */
    if (netMaskPtr != (ttUserIpAddress TM_FAR *)0)
    {
/* Check interfaceId, and mHomeIndex parameter */
        errorCode = tfValidConfigInterface(devEntryPtr, (tt16Bit) mHomeIndex);
        if (errorCode == TM_ENOERROR)
        {
            tm_call_lock_wait(&(devEntryPtr->devLockEntry));
            tm_ip_copy( tm_ip_dev_mask(devEntryPtr,mHomeIndex),
                        *netMaskPtr );
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

