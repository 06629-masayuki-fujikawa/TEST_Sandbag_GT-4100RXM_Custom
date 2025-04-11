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
 * Description: BSD Sockets Interface (tfDelArpByIpAddr, tfDelArpByPhysAddr)
 *
 * Filename: trdelarp.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trdelarp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:42:54JST $
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
/* Delete an ARP Entry from ARP table using IP address to locate entry*/
int tfDelArpEntryByIpAddr (
#ifdef TM_USE_STRONG_ESL
                            ttUserInterface interfaceHandle,
#endif /* TM_USE_STRONG_ESL */

                            ttUserIpAddress arpIpAddress)
{
    ttArpMapping arpMapping;
    int          errorCode;

    if (    (tm_ip_zero(arpIpAddress))
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
        arpMapping.arpmCommand = TM_4_ARP_DEL_BY_IP;
#ifdef TM_USE_STRONG_ESL
        arpMapping.arpmDevPtr = (ttDeviceEntryPtr)interfaceHandle;
#endif /* TM_USE_STRONG_ESL */
        errorCode = tfRtArpCommand(&arpMapping);
    }
    return errorCode;
}
#endif /* TM_USE_IPV4 */

/****************************************************************************
 * FUNCTION: tfNgDelArpEntryByIpAddr
 *
 * PURPOSE: Next Generation API replaces tfDelArpEntryByIpAddr
 *
 * PARAMETERS:
 *   arpIpAddrPtr
 *       ip address information pointer
 *
 * RETURNS:
 *   TM_ENOERROR
 *       success    
 *   TM_EINVAL
 *       arpIpAddrPtr is null pointer
 *   TM_ENOENT
 *       no ARP entry for the IP address found
 *
 ****************************************************************************/
int tfNgDelArpEntryByIpAddr(
#ifdef TM_USE_STRONG_ESL
    ttUserInterface                        interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
    const struct sockaddr_storage TM_FAR * arpIpAddrPtr )
{
#ifdef TM_USE_IPV6
    ttArpMapping arpMapping;
#endif /* TM_USE_IPV6 */
    int          errorCode;

    if (arpIpAddrPtr == (ttConstSockAddrStoragePtr)0)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        switch (arpIpAddrPtr->ss_family)
        {
#ifdef TM_USE_IPV4
        case AF_INET:
            errorCode = tfDelArpEntryByIpAddr(
#ifdef TM_USE_STRONG_ESL
                                interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                                arpIpAddrPtr->addr.ipv4.sin_addr.s_addr);
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
                arpMapping.arpmCommand = TM_6_ARP_DEL_BY_IP;
                tm_6_ip_copy_structs(arpIpAddrPtr->addr.ipv6.sin6_addr,
                                     arpMapping.arpm6IpAddress);
#ifdef TM_4_USE_SCOPE_ID
                if (IN6_IS_ADDR_V4MAPPED(&arpMapping.arpm6IpAddress))
                {
                    tm_4_addr_add_scope(&arpMapping.arpm6IpAddress,
                                        arpIpAddrPtr->addr.ipv6.sin6_scope_id);
                }
                else
#endif /* TM_4_USE_SCOPE_ID */
                {
                    tm_6_addr_add_scope(&arpMapping.arpm6IpAddress,
                                        arpIpAddrPtr->addr.ipv6.sin6_scope_id);
                }
#ifdef TM_USE_STRONG_ESL
                arpMapping.arpmDevPtr = (ttDeviceEntryPtr)interfaceHandle;
#endif /* TM_USE_STRONG_ESL */
                errorCode = tfRtArpCommand(&arpMapping);
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
/* Delete an ARP Entry from ARP table using physical address to locate entry*/
int tfDelArpEntryByPhysAddr (
#ifdef TM_USE_STRONG_ESL
                             ttUserInterface     interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                             const char TM_FAR * physAddrPtr, int physAddrLen)
{
    ttArpMapping arpMapping;
    int          errorCode;

    if (    (physAddrPtr == (const char TM_FAR *)0)
         || (physAddrLen > TM_MAX_PHYS_ADDR)
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
        arpMapping.arpmCommand = TM_4_ARP_DEL_BY_PHYS;
#ifdef TM_USE_STRONG_ESL
        arpMapping.arpmDevPtr = (ttDeviceEntryPtr)interfaceHandle;
#endif /* TM_USE_STRONG_ESL */
        errorCode = tfRtArpCommand(&arpMapping);
    }
    return errorCode;
}
#endif /* TM_USE_IPV4 */

/****************************************************************************
 * FUNCTION: tfNgDelArpEntryByPhysAddr
 *
 * PURPOSE: Next Generation API replaces tfDelArpEntryByPhysAddr
 *
 *   addrFamily
 *       can be either AF_INET or AF_INET6
 *   physAddrPtr
 *       pointer to a char array that contains the physical address
 *   physAddrLen
 *       length of the physical address
 * RETURNS:
 *   TM_ENOERROR
 *      sucess
 *   TM_EAFNOSUPPORT
 *      address family not supported 
 *   TM_EINVAL
 *       physAddrPtr is null pointer or physAddrLen is too short
 *   TM_ENOENT 
 *      no such Arp entry
 *
 ****************************************************************************/
int tfNgDelArpEntryByPhysAddr (
#ifdef TM_USE_STRONG_ESL
    ttUserInterface             interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
    int                         addrFamily, 
    const ttUser8Bit TM_FAR *   physAddrPtr,
    int                         physAddrLen )
{
#ifdef TM_USE_IPV6
    ttArpMapping arpMapping;
#endif /* TM_USE_IPV6 */
    int          errorCode;

    if (    (physAddrPtr == (const ttUser8Bit TM_FAR *)0 )
         || (physAddrLen <= 0) )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        switch (addrFamily)
        {
#ifdef TM_USE_IPV4
        case AF_INET:
            errorCode = tfDelArpEntryByPhysAddr(
#ifdef TM_USE_STRONG_ESL
                                interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                                (const char TM_FAR *)physAddrPtr,
                                physAddrLen);
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
                arpMapping.arpmCommand = TM_6_ARP_DEL_BY_PHYS;
                tm_bcopy(physAddrPtr,
                         arpMapping.arpmEnetAddress,
                         physAddrLen );
                arpMapping.arpmPhysAddrLen = (tt8Bit)physAddrLen;
#ifdef TM_USE_STRONG_ESL
                arpMapping.arpmDevPtr = (ttDeviceEntryPtr)interfaceHandle;
#endif /* TM_USE_STRONG_ESL */
                errorCode = tfRtArpCommand(&arpMapping);
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

#ifdef TM_USE_ARP_FLUSH
/* Flush the ARP table for the given address family/families */
int tfArpFlush (int af)
{
    int errorCode;

/* We only support AF_INET, and AF_INET6 */
    af = af & (AF_INET | AF_INET6);
#ifndef TM_USE_IPV6
    af &= ~AF_INET6;
#endif /* TM_USE_IPV6 */
#ifndef TM_USE_IPV4
    af &= ~AF_INET;
#endif /* TM_USE_IPV4 */
    if ((af & (AF_INET|AF_INET6)) != 0)
    {
        errorCode = TM_ENOERROR;
        tfRtArpFlush(af
#ifdef TM_LOCK_NEEDED
                   , TM_LOC_WAIT
#endif /* TM_LOCK_NEEDED */
                    );
    }
    else
    {
        errorCode = TM_EINVAL;
    }
    return errorCode;
}
#endif /* TM_USE_ARP_FLUSH */

