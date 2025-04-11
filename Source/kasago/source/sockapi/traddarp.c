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
 * Description: BSD Sockets Interface (tfAddArpEntry)
 *
 * Filename: traddarp.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/traddarp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:42:39JST $
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
/* Add an ARP Entry to ARP table */
int tfAddArpEntry (
#ifdef TM_USE_STRONG_ESL
                   ttUserInterface      interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                   ttUserIpAddress      arpIpAddress,
                   const char TM_FAR *  physAddrPtr,
                   int                  physAddrLength)
{
    int           errorCode;

    if (    (tm_ip_zero(arpIpAddress))
            || (physAddrPtr == (const char TM_FAR *)0)
#if (TM_MAX_PHYS_ADDR <= 255)
            || (physAddrLength > TM_MAX_PHYS_ADDR)
#else /* !(TM_MAX_PHYS_ADDR <= 255) */
            || (physAddrLength > 255)
#endif /* !(TM_MAX_PHYS_ADDR <= 255) */
#ifdef TM_USE_STRONG_ESL
         || ( tfValidInterface(interfaceHandle) != TM_ENOERROR )
#endif /* TM_USE_STRONG_ESL */
       )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        errorCode = tf4RtArpAddEntry(
#ifdef TM_USE_STRONG_ESL
                                    (ttDeviceEntryPtr)interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                                    (tt4IpAddress)arpIpAddress,
                                    (tt8BitPtr)physAddrPtr,
                                    TM_RTE_INF,
                                    (tt8Bit) physAddrLength);
    }
    return errorCode;
}
#endif /* TM_USE_IPV4 */

/***************************************************************************
 * FUNCTION: tfNgAddArpEntry
 *
 * PURPOSE: Next Generation API replaces tfAddArpEntry.
 *
 * PARAMETERS:
 *   arpIpAddrPtr
 *       pointer to the IP addr information, 
 *   physAddrPtr
 *       pointer to a char array that contains the physical address
 *   physAddrLen
 *       length of the physical address
 * RETURNS:
 *   TM_ENOERROR
 *      sucess
 *   TM_EINVAL
 *       arpIpAddrPtr or physAddrPtr  is null pointer
 *       or physAddrLen is too short
 *
 ***************************************************************************/
int tfNgAddArpEntry(
#ifdef TM_USE_STRONG_ESL
    ttUserInterface                         interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
    const struct sockaddr_storage TM_FAR *  arpIpAddrPtr,
    const ttUser8Bit TM_FAR *               physAddrPtr, 
    int                                     physAddrLen )
{
    int          errorCode;
#ifdef TM_USE_IPV6
    tt6IpAddress    scopedIpAddr;
#endif /* TM_USE_IPV6 */

    if (    (physAddrPtr == (const ttUser8Bit TM_FAR *)0 )
         || (physAddrLen <= 0) 
         || (arpIpAddrPtr == (ttConstSockAddrStoragePtr)0)
#ifdef TM_USE_STRONG_ESL
         || ( tfValidInterface(interfaceHandle) != TM_ENOERROR )
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
            errorCode = tf4RtArpAddEntry(
#ifdef TM_USE_STRONG_ESL
                        (ttDeviceEntryPtr)interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                        (tt4IpAddress)arpIpAddrPtr->addr.ipv4.sin_addr.s_addr,
                        (tt8BitPtr)physAddrPtr,
                        TM_RTE_INF,
                        (tt8Bit) physAddrLen);
            break;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
        case AF_INET6:
            tm_6_ip_copy_structs(
                            arpIpAddrPtr->addr.ipv6.sin6_addr, 
                            scopedIpAddr);
#ifdef TM_4_USE_SCOPE_ID
            if (IN6_IS_ADDR_V4MAPPED(&scopedIpAddr))
            {
                tm_4_addr_add_scope(
                    &scopedIpAddr,
                    arpIpAddrPtr->addr.ipv6.sin6_scope_id);
            }
            else
#endif /* TM_4_USE_SCOPE_ID */
            {
                tm_6_addr_add_scope(
                    &scopedIpAddr,
                    arpIpAddrPtr->addr.ipv6.sin6_scope_id);
            }
            errorCode = tf6RtArpAddEntry(
#ifdef TM_USE_STRONG_ESL
                            (ttDeviceEntryPtr)interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                            &scopedIpAddr,
                            (tt8BitPtr)physAddrPtr,
                            TM_RTE_INF,
                            (tt8Bit) physAddrLen);
            break;
#endif /* TM_USE_IPV6 */
        
        default:
            errorCode = TM_EAFNOSUPPORT;
            break;
        }
    }        

    return errorCode;
}
