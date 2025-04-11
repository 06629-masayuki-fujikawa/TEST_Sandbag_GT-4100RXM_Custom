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
 * Description: BSD Sockets Interface (tfGetArpEntryByIpAddr,
 *                                     tfGetArpEntryByPhysAddr)
 *
 * Filename: trgetarp.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trgetarp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:14JST $
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
/* Get an ARP Entry from ARP table using physical address to locate entry*/
int tfGetArpEntryByPhysAddr ( 
#ifdef TM_USE_STRONG_ESL
    ttUserInterface             interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
    const char      TM_FAR *    physAddrPtr,
    int                         physAddrLen,
    ttUserIpAddress TM_FAR *    arpIpAddressPtr)
{
    ttArpMapping arpMapping;
    int          errorCode;

    if (    (physAddrPtr == (const char TM_FAR *)0)
         || (physAddrLen > TM_MAX_PHYS_ADDR)
         || (arpIpAddressPtr == (ttUserIpAddress TM_FAR *)0)
#ifdef TM_USE_STRONG_ESL
         ||  (tfValidInterface(interfaceHandle) != TM_ENOERROR)
#endif /* TM_USE_STRONG_ESL */
       )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        tm_bcopy( physAddrPtr, arpMapping.arpmEnetAddress, physAddrLen);
        arpMapping.arpmPhysAddrLen = (tt8Bit) physAddrLen;
        arpMapping.arpmCommand = TM_4_ARP_GET_BY_PHYS;
#ifdef TM_USE_STRONG_ESL
        arpMapping.arpmDevPtr = (ttDeviceEntryPtr)interfaceHandle;
#endif /* TM_USE_STRONG_ESL */
        errorCode = tfRtArpCommand(&arpMapping);
        if (errorCode == TM_ENOERROR)
        {
            tm_ip_copy(arpMapping.arpmIpAddress, *arpIpAddressPtr);
        }
    }
    return errorCode;
}
#endif /* TM_USE_IPV4 */

/****************************************************************************
* FUNCTION: tfNgGetArpEntryByPhysAddr
*
* PURPOSE: Next Generation API replaces tfGetArpEntryByPhysAddr
*
* PARAMETERS:
*   addrFamily
*       can be either AF_INET or AF_INET6
*   physAddrPtr
*       pointer to a char array that contains the physical address
*   physAddrLen
*       length of the physical address
*   arpIpAddrPtr
*       Output parameter, pointer to the IP addr information, 
*       of type struct sockaddr_storage
* RETURNS:
*   TM_ENOERROR
*      sucess
*   TM_EAFNOSUPPORT
*      address family not supported 
*   TM_EINVAL
*       physAddrPtr or arpIpAddrPtr  is null pointer
*       or physAddrLen is too short
*   TM_ENOENT 
*      no such Arp entry
* NOTES:
*
****************************************************************************/
int tfNgGetArpEntryByPhysAddr (
#ifdef TM_USE_STRONG_ESL
    ttUserInterface                     interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
    int                                 addrFamily, 
    const ttUser8Bit TM_FAR *           physAddrPtr,
    int                                 physAddrLen, 
    struct sockaddr_storage TM_FAR *    arpIpAddrPtr )
{
#ifdef TM_USE_IPV6
    ttArpMapping arpMapping;
#endif /* TM_USE_IPV6 */
    int          errorCode;

    if (    (physAddrPtr == (const ttUser8Bit TM_FAR *)0 )
         || (physAddrLen <= 0) 
         || (arpIpAddrPtr == TM_SOCKADDR_STORAGE_NULL_PTR) )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        tm_bzero((ttVoidPtr) arpIpAddrPtr, sizeof(struct sockaddr_storage));
        arpIpAddrPtr->ss_family = (tt8Bit)addrFamily;
        arpIpAddrPtr->ss_len    = sizeof(struct sockaddr_storage);

        switch (addrFamily)
        {
#ifdef TM_USE_IPV4
        case AF_INET:
            errorCode = tfGetArpEntryByPhysAddr(
#ifdef TM_USE_STRONG_ESL
                                interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                                (const char TM_FAR *)physAddrPtr,
                                physAddrLen,
                                &arpIpAddrPtr->addr.ipv4.sin_addr.s_addr);
            break;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
        case AF_INET6:
#ifdef TM_USE_STRONG_ESL
            if (tfValidInterface(interfaceHandle) != TM_ENOERROR)
            {
                errorCode = TM_EINVAL;
            }
            else
#endif /* TM_USE_STRONG_ESL */
            {
                arpMapping.arpmCommand = TM_6_ARP_GET_BY_PHYS;
                tm_bcopy(physAddrPtr,
                         arpMapping.arpmEnetAddress,
                         physAddrLen ); 
                arpMapping.arpmPhysAddrLen = (tt8Bit)physAddrLen;
#ifdef TM_USE_STRONG_ESL
                arpMapping.arpmDevPtr = (ttDeviceEntryPtr)interfaceHandle;
#endif /* TM_USE_STRONG_ESL */
                errorCode = tfRtArpCommand(&arpMapping);
                if (errorCode == TM_ENOERROR)
                {
/* store the sin6_addr */
                    tm_6_ip_copy_structs(arpMapping.arpm6IpAddress,
                                         arpIpAddrPtr->addr.ipv6.sin6_addr);
#ifdef TM_4_USE_SCOPE_ID
                    if (IN6_IS_ADDR_V4MAPPED(&arpIpAddrPtr->addr.ipv6.sin6_addr))
                    {
/* store the sin6_scope_id */
                        tm_4_get_embedded_scope_id(
                            &arpIpAddrPtr->addr.ipv6.sin6_addr, 
                            arpIpAddrPtr->addr.ipv6.sin6_scope_id);
/* unscope the address */
                        tm_4_dev_unscope_addr(
                            &arpIpAddrPtr->addr.ipv6.sin6_addr);
                    }
                    else
#endif /* TM_4_USE_SCOPE_ID */
                    {
/* store the sin6_scope_id */
                        tm_6_get_embedded_scope_id(
                            &arpIpAddrPtr->addr.ipv6.sin6_addr, 
                            arpIpAddrPtr->addr.ipv6.sin6_scope_id);
/* unscope the address */
                        tm_6_dev_unscope_addr(
                            &arpIpAddrPtr->addr.ipv6.sin6_addr);
                    }
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

#ifdef TM_USE_IPV4
/* Get an ARP Entry from ARP table using IP Address to locate entry*/
int tfGetArpEntryByIpAddr(
#ifdef TM_USE_STRONG_ESL
    ttUserInterface     interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
    ttUserIpAddress     arpIpAddress,
    char  TM_FAR *      physAddrPtr,
    int                 physAddrBufLen)
{
    ttArpMapping arpMapping;
    int          errorCode;

    if (    (tm_ip_zero(arpIpAddress))
         || (physAddrPtr == (char TM_FAR *)0)
         || (physAddrBufLen != TM_MAX_PHYS_ADDR)
#ifdef TM_USE_STRONG_ESL
         || (tfValidInterface(interfaceHandle) != TM_ENOERROR)
#endif /* TM_USE_STRONG_ESL */
       )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        tm_ip_copy(arpIpAddress, arpMapping.arpmIpAddress);
        arpMapping.arpmCommand = TM_4_ARP_GET_BY_IP;
        arpMapping.arpmPhysAddrLen = (tt8Bit) physAddrBufLen;
#ifdef TM_USE_STRONG_ESL
        arpMapping.arpmDevPtr = (ttDeviceEntryPtr)interfaceHandle;
#endif /* TM_USE_STRONG_ESL */
        errorCode = tfRtArpCommand(&arpMapping);
        if (errorCode == TM_ENOERROR)
        {
            tm_bcopy( arpMapping.arpmEnetAddress,
                      physAddrPtr,
                      TM_MAX_PHYS_ADDR );
        }
    }
    return errorCode;
}
#endif /* TM_USE_IPV4 */

/****************************************************************************
* FUNCTION: tfNgGetArpEntryByIpAddr
*
* PURPOSE: Next Generation API replaces tfGetArpEntryByIpAddr
*
* PARAMETERS:
*   arpIpAddrPtr
*       ip address information pointer
*   physAddrPtr
*       physical address buffer pointer
*   physAddrBufLen
*       The length in bytes of the buffer pointed to by physAddrPtr.
*   physAddrLenPtr
*       Output parameter, set to the length of the physical address
*       returned in the buffer pointed to by physAddrPtr.
*
* RETURNS:
*   TM_ENOERROR
*       success    
*   TM_EINVAL
*       arpIpAddrPtr, physAddrPtr or physAddrLenPtr is null pointer
*       or physAddrBufLen is too short
*   TM_ENOENT
*
* NOTES:
*
****************************************************************************/
int tfNgGetArpEntryByIpAddr (
#ifdef TM_USE_STRONG_ESL
    ttUserInterface     interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
    const struct sockaddr_storage TM_FAR * arpIpAddrPtr,
    ttUser8Bit TM_FAR * physAddrPtr, 
    int                 physAddrBufLen,
    int TM_FAR *        physAddrLenPtr )
{

    ttArpMapping arpMapping;
    int          errorCode;

    errorCode = TM_ENOERROR;

/* parameter validation */
    if (  (arpIpAddrPtr == (ttConstSockAddrStoragePtr)0)
        ||(physAddrPtr == TM_8BIT_NULL_PTR)
        ||(physAddrLenPtr == (int TM_FAR *)0)
        ||(physAddrBufLen <= 0)
#ifdef TM_USE_STRONG_ESL
        || (tfValidInterface(interfaceHandle) != TM_ENOERROR)
#endif /* TM_USE_STRONG_ESL */
       )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        switch (arpIpAddrPtr->ss_family)
        {
#ifdef TM_USE_IPV4
        case AF_INET:
            arpMapping.arpmCommand = TM_4_ARP_GET_BY_IP;
            tm_ip_copy(arpIpAddrPtr->addr.ipv4.sin_addr.s_addr, 
                       arpMapping.arpm4IpAddress);
            break;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
        case AF_INET6:
            arpMapping.arpmCommand = TM_6_ARP_GET_BY_IP;
            tm_6_ip_copy_structs(arpIpAddrPtr->addr.ipv6.sin6_addr, 
                                 arpMapping.arpm6IpAddress);
#ifdef TM_4_USE_SCOPE_ID
            if (IN6_IS_ADDR_V4MAPPED(
                    &arpMapping.arpm6IpAddress))
            {
                tm_4_addr_add_scope(
                    &arpMapping.arpm6IpAddress,
                    arpIpAddrPtr->addr.ipv6.sin6_scope_id);
            }
            else
#endif /* TM_4_USE_SCOPE_ID */
            {
                tm_6_addr_add_scope(
                    &arpMapping.arpm6IpAddress,
                    arpIpAddrPtr->addr.ipv6.sin6_scope_id);
            }
            break;
#endif /* TM_USE_IPV6 */
        
        default:
            errorCode = TM_EAFNOSUPPORT;
            break;
        }
        arpMapping.arpmPhysAddrLen = (tt8Bit) physAddrBufLen;
    }

    if (errorCode == TM_ENOERROR)
    {
#ifdef TM_USE_STRONG_ESL
        arpMapping.arpmDevPtr = (ttDeviceEntryPtr)interfaceHandle;
#endif /* TM_USE_STRONG_ESL */
        errorCode = tfRtArpCommand(&arpMapping);
        *physAddrLenPtr = arpMapping.arpmPhysAddrLen;
        if (errorCode == TM_ENOERROR)
        {
            tm_bcopy( arpMapping.arpmEnetAddress,
                      physAddrPtr,
                      *physAddrLenPtr);
        }
    }

    return errorCode;
}
