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
 * Description: BSD Sockets Interface (bind)
 *
 * Filename: trbind.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trbind.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2010/09/22 04:21:37JST $
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

#define TM_CHECK_CONFIG_IP_ADDRESS 0

/*
 * Bind the socket to a port number (without it we are using the random
 * port number), and to an IP address.
 */
int bind(int                            socketDescriptor, 
         const struct sockaddr TM_FAR * localAddressPtr, 
         int                            localAddressLength)
{
/* Common routine with tfBindNoCheck() */
    return tfBindFlag( socketDescriptor,
                       localAddressPtr,
                       localAddressLength,
                       TM_CHECK_CONFIG_IP_ADDRESS );
}

/* Common routine called by both bind(), and tfBindNoCheck() */
int tfBindFlag(int                            socketDescriptor,
               const struct sockaddr TM_FAR * localAddressPtr,
               int                            localAddressLength,
               int                            flag )
{
    ttSocketEntryPtr    socketEntryPtr;
    ttDeviceEntryPtr    anyDevPtr;
    ttSockTuple         sockTuple;
    ttSockAddrPtrUnion  localAddressInPtr;
    int                 errorCode;
    int                 retCode;
#ifdef TM_USE_BSD_DOMAIN
    int                 af;
#endif /* TM_USE_BSD_DOMAIN */
    ttIpPort            portNumber;
    tt16Bit             treeFlag;
    tt8Bit              addrFamily;
    tt8Bit              noAddrMatch;
#ifdef TM_USE_IPV6
    tt6IpAddressPtr     ipAddrPtr;
#endif /* TM_USE_IPV6 */
#ifndef TM_SINGLE_INTERFACE_HOME
auto    tt16Bit         anyMhomeIndex;
#endif /* TM_SINGLE_INTERFACE_HOME */
#ifdef TM_4_USE_SCOPE_ID
    tt16Bit             srcDevIndex;
#endif /* TM_4_USE_SCOPE_ID */
    
    errorCode = TM_ENOERROR;
#ifdef TM_4_USE_SCOPE_ID
    srcDevIndex = TM_16BIT_ZERO;
#endif /* TM_4_USE_SCOPE_ID */

/* to suppress compiler warnings only, doesn't need to be initialized indeed */
#ifdef TM_USE_IPV6
    ipAddrPtr = TM_6_IPADDR_NULL_PTR;
#endif /* TM_USE_IPV6 */
    localAddressInPtr.sockPtr = (struct sockaddr TM_FAR *)localAddressPtr;
    
/* PARAMETER CHECK */

/*
 * Map from socket descriptor to a locked socket entry pointer and verify
 * that the address length is at least what we expect for this address type.
 */    
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_ANY;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckAddrLock( socketDescriptor,
                                            localAddressPtr,
                                            localAddressLength,
                                            &errorCode
#ifdef TM_USE_BSD_DOMAIN
                                          , &af
#endif /* TM_USE_BSD_DOMAIN */
                                            );
#ifdef TM_USE_BSD_DOMAIN
    if (af == TM_BSD_FAMILY_NONINET) 
    {
        tm_assert(bind, socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR);
        retCode = tfBsdBind( (ttVoidPtr)socketEntryPtr, localAddressPtr, 
                             localAddressLength );
    }
    else
#endif /* !TM_USE_BSD_DOMAIN */
    {
        if (socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR)
        {
/* errorCode set by tfSocketCheckAndLock */
            goto bindFinish;
        }
        if ( localAddressPtr == (const struct sockaddr TM_FAR *)0 )
        {
            errorCode = TM_EINVAL;
            goto bindFinish;
        }

        addrFamily = localAddressPtr->sa_family;
        tm_bcopy(&(socketEntryPtr->socTuple), &sockTuple, sizeof(sockTuple));
    
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
        if (addrFamily == AF_INET6)
        {
#endif /* TM_USE_IPV4 */
            portNumber = (ttIpPort)(localAddressInPtr.sockIn6Ptr->sin6_port);
            ipAddrPtr  = &(localAddressInPtr.sockIn6Ptr->sin6_addr);
            tm_6_ip_copy_dststruct(ipAddrPtr, sockTuple.sotLocalIpAddress);
            ipAddrPtr = &(sockTuple.sotLocalIpAddress);

/* Retrieve the IPv4 address from a v4 mapped IPv6 address - used below to
 * verify the address being bound to is configured on this interface.
 */ 
#ifdef TM_USE_IPV4
            if ( IN6_IS_ADDR_V4MAPPED(ipAddrPtr))
            {
                addrFamily = AF_INET; /* changed below for link-local scope */

#ifdef TM_4_USE_SCOPE_ID
                if (IN4_IS_ADDR_LINKLOCAL(
                                tm_4_ip_addr(sockTuple.sotLocalIpAddress)))
                {
                    srcDevIndex =
                        (tt16Bit) localAddressInPtr.sockIn6Ptr->sin6_scope_id;

                    if (srcDevIndex
                        > ((int)tm_context(tvAddedInterfaces) & 0xFFFF))
                    {
/* scope ID is bogus */                        
                        errorCode = TM_EINVAL;
                        goto bindFinish;
                    }
                }
#endif /* TM_4_USE_SCOPE_ID */
            }
        }
        else
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
        {
            portNumber = (ttIpPort)(localAddressInPtr.sockInPtr->sin_port);
            tm_ip_copy(
                (tt4IpAddress)(localAddressInPtr.sockInPtr->sin_addr.s_addr),
                tm_4_ip_addr(sockTuple.sotLocalIpAddress));
        }
#endif /* TM_USE_IPV4 */

        if (flag == TM_CHECK_CONFIG_IP_ADDRESS)
        {
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
            if (addrFamily == AF_INET6)
            {
#endif /* TM_USE_IPV4 */
                if (    !(tm_6_ip_zero(ipAddrPtr))
                     && !(IN6_IS_ADDR_MULTICAST(ipAddrPtr)) )
                {
/* Check that IP address is a valid interface address */
#ifdef TM_USE_STRONG_ESL
                    if (tm_8bit_all_bits_set(socketEntryPtr->socFlags2,
                                             TM_SOCF2_BINDTODEVICE))
                    {
                        anyDevPtr = socketEntryPtr->socRteCacheStruct.rtcDevPtr;
                        if (tf6MhomeAddrMatch(
                                anyDevPtr,
                                ipAddrPtr,
                                &anyMhomeIndex) != TM_ENOERROR )
                        {
/* address is not configured on bound interface */
                            anyDevPtr = (ttDeviceEntryPtr)0;
                        }
                    }
                    else
#endif /* TM_USE_STRONG_ESL */
                    {
                        anyDevPtr = tf6IfaceMatch(ipAddrPtr,
                                                  tf6MhomeAddrMatch,
                                                  &anyMhomeIndex);
                    }
                    if ( anyDevPtr == (ttDeviceEntryPtr)0 )
                    {
                        errorCode = TM_EINVAL;
                        goto bindFinish;
                    }
                }
#ifdef TM_USE_IPV4
            }
            else
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
            {
                if (    tm_ip_not_zero(
                                  tm_4_ip_addr(sockTuple.sotLocalIpAddress))
                     && !tm_ip_is_multicast(
                                  tm_4_ip_addr(sockTuple.sotLocalIpAddress)) )
                {
/* Check that IP address is a valid interface address */
#ifdef TM_SINGLE_INTERFACE_HOME
                    anyDevPtr = tfIfaceConfig(0);
                    if ( anyDevPtr != (ttDeviceEntryPtr)0 )
                    {
                        if ( !tm_ip_match(
                                    tm_4_ip_addr(sockTuple.sotLocalIpAddress),
                                    tm_ip_dev_addr(anyDevPtr, 0)) )
                        {
                            anyDevPtr = (ttDeviceEntryPtr)0;
                        }
                    }
#else /* TM_SINGLE_INTERFACE_HOME */
#ifdef TM_USE_STRONG_ESL
                    if (tm_8bit_all_bits_set(socketEntryPtr->socFlags2,
                                             TM_SOCF2_BINDTODEVICE))
                    {
/* Bound to device */
                        anyDevPtr = socketEntryPtr->socRteCacheStruct.rtcDevPtr;
                        if (tfMhomeAddrMatch(
                                anyDevPtr,
                                tm_4_ip_addr(sockTuple.sotLocalIpAddress),
                                &anyMhomeIndex) != TM_ENOERROR )
                        {
/* could not find the specified IPv4 address on the interface
   specified by SO_BINDTODEVICE */
                            errorCode = TM_EINVAL;
                            goto bindFinish;
                        }
                    }
                    else
                    {
                        anyDevPtr = (ttDeviceEntryPtr)0;
                    }
                    if (anyDevPtr == (ttDeviceEntryPtr)0)
/* Not bound to device */
#endif /* TM_USE_STRONG_ESL */
                    {
#ifdef TM_4_USE_SCOPE_ID
                        if (srcDevIndex != TM_16BIT_ZERO)
                        {
                            anyDevPtr = 
                                tm_context(tvDevIndexToDevPtr)[srcDevIndex];
                            if (tfMhomeAddrMatch(
                                    anyDevPtr,
                                    tm_4_ip_addr(sockTuple.sotLocalIpAddress),
                                    &anyMhomeIndex) != TM_ENOERROR )
                            {
/* could not find the specified link-local scope IPv4 address on the interface
   specified by sin6_scope_id */
                                anyDevPtr = (ttDeviceEntryPtr)0;
                            }
                        }
                        else
#endif /* TM_4_USE_SCOPE_ID */
                        {
                            anyDevPtr = tfIfaceMatch(
                                    tm_4_ip_addr(sockTuple.sotLocalIpAddress),
                                    tfMhomeAddrMatch,
                                    &anyMhomeIndex);
                        }
                    }
#endif /* !TM_SINGLE_INTERFACE_HOME */
                    if ( anyDevPtr == (ttDeviceEntryPtr)0 )
                    {
                        errorCode = TM_EINVAL;
                        goto bindFinish;
                    }
                }
            }
#endif /* TM_USE_IPV4 */
        }

#ifdef TM_4_USE_SCOPE_ID
        if (srcDevIndex != TM_16BIT_ZERO)
        {
/* Treat the link-local scope IPv4 address as an IPv4-mapped IPv6 address for
   storing on the socket and reinsertion into the socket tree. We must
   embedded the scope ID first, however. */
            addrFamily = AF_INET6;
            tm_4_addr_add_scope(ipAddrPtr, srcDevIndex);
        }
        else
#endif /* TM_4_USE_SCOPE_ID */
#ifdef TM_USE_IPV6
        if (addrFamily == AF_INET6)
        {
/* Embed scope ID (if necessary) before inserting into the tree.  Need to
 * copy to a temporary variable since we can't modify the user's parameter.
 */ 
            tm_6_addr_add_scope(
                ipAddrPtr, localAddressInPtr.sockIn6Ptr->sin6_scope_id);
        }
#endif /* TM_USE_IPV6 */

        if ( tm_16bit_one_bit_set(socketEntryPtr->socFlags, TM_SOCF_BIND) )
        {
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
            if (addrFamily == AF_INET6)
            {
#endif /* TM_USE_IPV4 */
                noAddrMatch = (tt8Bit)
                    tm_6_ip_not_match( ipAddrPtr,
                                       &(socketEntryPtr->socOurIfIpAddress) );
#ifdef TM_USE_IPV4
            }
            else
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
            {
                noAddrMatch = (tt8Bit)tm_ip_not_match(
                              tm_4_ip_addr(sockTuple.sotLocalIpAddress),
                              tm_4_ip_addr(socketEntryPtr->socOurIfIpAddress));
            }
#endif /* TM_USE_IPV4 */
        
            if (    (!(socketEntryPtr->socOptions & SO_REUSEADDR))
                 || (socketEntryPtr->socOurLayer4Port != portNumber)
                 || noAddrMatch )
            {
                errorCode = TM_EINVAL;
            }
            goto bindFinish;
        }
/* If no address was specified, use the default interface address */    
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
        if ( addrFamily == AF_INET6 )
        {
#endif /* TM_USE_IPV4 */
            if (tm_6_ip_zero(ipAddrPtr))
            {
/* socOurIfIpAddress will already have embedded scope ID */
                tm_6_ip_copy_structs(socketEntryPtr->socOurIfIpAddress,
                                     sockTuple.sotLocalIpAddress);
            }
#ifdef TM_USE_IPV4
        }
        else
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
        {
            if (    (addrFamily == AF_INET)
                 && (tm_ip_zero(tm_4_ip_addr(sockTuple.sotLocalIpAddress))) )
            {
                tm_ip_copy(tm_4_ip_addr(socketEntryPtr->socOurIfIpAddress),
                           tm_4_ip_addr(sockTuple.sotLocalIpAddress));
            }
        }
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
        if ( addrFamily == AF_INET6 )
        {
#endif /* TM_USE_IPV4 */
/* Save the flow label and traffic class values from the user */
            socketEntryPtr->soc6FlowInfo =
                    localAddressInPtr.sockIn6Ptr->sin6_flowinfo;
#ifdef TM_USE_IPV4
        }
        else
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
        {
#ifdef TM_USE_IPV6
            tm_6_addr_to_ipv4_mapped(
                                tm_4_ip_addr(sockTuple.sotLocalIpAddress),
                                &(sockTuple.sotLocalIpAddress));
#endif /* TM_USE_IPV6 */
        }
#endif /* TM_USE_IPV4 */
        if (portNumber != (ttIpPort)0)
        {
            sockTuple.sotLocalIpPort = portNumber;
        }
        if (    (portNumber != 0)
             || (socketEntryPtr->socProtocolNumber != IPPROTO_TCP)
             || (tm_16bit_one_bit_set(socketEntryPtr->socFlags, 
                                      TM_SOCF_IN_TREE))
           )
        {
            treeFlag = (tt16Bit)(   socketEntryPtr->socCoFlags
                                  | (   socketEntryPtr->socOptions
                                      & (SO_REUSEADDR | SO_REUSEPORT) ) );
            if (    (portNumber == 0)
                 && (tm_16bit_bits_not_set(socketEntryPtr->socFlags,
                                           TM_SOCF_IN_TREE)))
            {
                treeFlag = (tt16Bit)(treeFlag | TM_SOC_TREE_RANDOM_PORT);
            }
            retCode = tfSocketTreeReInsert(socketEntryPtr,
                                           &sockTuple,
                                           treeFlag);
            
            if ( retCode != TM_SOC_RB_OKAY )
            {
                errorCode = TM_EADDRINUSE;
                goto bindFinish;
            }
        }
        else
/*
 * Do not insert in the tree, if
 * portNumber is 0, and TCP socket is not in the tree yet.
 */
        {
            tm_bcopy(&sockTuple, &(socketEntryPtr->socTuple),
                     sizeof(sockTuple));
        }

        socketEntryPtr->socFlags |= TM_SOCF_BIND;
        if (portNumber != 0)
        {
            tm_8bit_clr_bit( socketEntryPtr->socFlags2, 
                              TM_SOCF2_BIND_PORT_ZERO); 
        }
        else
        {
            socketEntryPtr->socFlags2 |= TM_SOCF2_BIND_PORT_ZERO; 
        }
    
bindFinish:
        retCode = tfSocketReturn(socketEntryPtr, socketDescriptor, errorCode,
                                 TM_ENOERROR);
    }
    return retCode;
}
