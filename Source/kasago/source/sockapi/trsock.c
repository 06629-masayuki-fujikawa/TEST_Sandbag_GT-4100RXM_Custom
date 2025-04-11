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
 * Description: BSD Sockets Interface (socket)
 *
 * Filename: trsock.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trsock.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:29JST $
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

/*
 * Allocate a new Socket Index
 * It is not bound to a port number yet
 */
int socket (int family, int type, int protocol)
{
    int              errorCode;
    int              retCode;
    int              index;
    ttSocketEntryPtr socketEntryPtr;
    
    errorCode=TM_ENOERROR;
    index = TM_SOCKET_ERROR;
    socketEntryPtr = TM_SOCKET_ENTRY_NULL_PTR;
/* Check the parameters */

    if (
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
/* dual IP layer */
        (family != PF_INET) && (family != PF_INET6)
#else /* ! TM_USE_IPV4 */
/* IPv6-only */
        (family != PF_INET6)
#endif /* ! TM_USE_IPV4 */
#else /* ! TM_USE_IPV6 */
/* IPv4-only */
        (family != PF_INET)
#endif /* ! TM_USE_IPV6 */
        )
    {
/* per POSIX return EAFNOSUPPORT if we don't support the address family. */
        errorCode = TM_EAFNOSUPPORT;
        goto socketFinish;
    }

/* We only support Internet Protocols */
    if ( 
           ((type != SOCK_RAW) && (!(tm_treck_sock_protocols(protocol))))
        ||
#ifdef TM_USE_IPV6      
#ifdef TM_USE_IPV4
/* dual */
           ((protocol == IP_PROTOICMP) && (family != PF_INET))
        || ((protocol == IPPROTO_ICMPV6) && (family != PF_INET6))
        || 
#else /* !TM_USE_IPV4 */
/* IPv6-only */
           (protocol == IP_PROTOIGMP)
        || (protocol == IP_PROTOICMP)
        || 
#endif /* ! TM_USE_IPV4 */
#else /* ! TM_USE_IPV6 */
/* IPv4-only */
           (protocol == IPPROTO_ICMPV6)        
        ||
#endif /* ! TM_USE_IPV6 */
/* Support only for raw, datagram and stream sockets */
           (    (type != SOCK_STREAM)
             && (type != SOCK_DGRAM)
             && (type != SOCK_RAW) )
/* If it is TCP is must be STREAM */
        || ( (protocol == IP_PROTOTCP) && (type != SOCK_STREAM) )
/* If it is UDP then it must be DATAGRAM */
        || ( (protocol == IP_PROTOUDP) && (type != SOCK_DGRAM) )
/* If it is IGMP is must be raw. */
#ifdef TM_USE_IPV4
#ifdef TM_IGMP
        || ( (protocol == IP_PROTOIGMP) && (type != SOCK_RAW) )
#endif /* TM_IGMP */
/* If it is ICMP is must be raw */
        || ( (protocol == IP_PROTOICMP) && (type != SOCK_RAW) )
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
        || ( (protocol == IPPROTO_ICMPV6) && (type != SOCK_RAW) )
#endif /* TM_USE_IPV6 */
        )
    {
        errorCode = TM_EPROTONOSUPPORT;
        goto socketFinish;
    }

/* If the caller did not specify a protocol to use */
    if (protocol == TM_SOC_PROTO_DEFAULT)
    {
/* For stream it is TCP */
        if (type == SOCK_STREAM)
        {
            protocol = IP_PROTOTCP;
        }
        else
        {
/* For datagram it is UDP */
            if (type == SOCK_DGRAM)
            {
                protocol = IP_PROTOUDP;
            }
            else
            {
/* For RAW it is ICMP */
                if (type == SOCK_RAW)
                {
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
/* dual IP layer */
                    if (family == PF_INET)
                    {
                        protocol = IP_PROTOICMP;
                    }
                    else
                    {
                        protocol = IPPROTO_ICMPV6;
                    }
#else /* ! TM_USE_IPV4 */
/* IPv6-only */
                    protocol = IPPROTO_ICMPV6;
#endif /* ! TM_USE_IPV4 */
#else /* ! TM_USE_IPV6 */
/* IPv4-only */
                    protocol = IP_PROTOICMP;
#endif /* ! TM_USE_IPV6 */
                }
                else
                {
                    errorCode = TM_EPROTONOSUPPORT;
                    goto socketFinish;
                }
            }
        }
    }

/* Unless raw socket support is enabled, the only protocol that we support with
   IPv6 raw sockets is IPPROTO_ICMPV6 */
#if (defined(TM_USE_IPV6) && !defined(TM_6_USE_RAW_SOCKET))
    if ((family == PF_INET6) && (type == SOCK_RAW)
        && (protocol != IPPROTO_ICMPV6))
    {
        errorCode = TM_EPROTONOSUPPORT;
        goto socketFinish;
    }
#endif /* TM_USE_IPV6 && !TM_6_USE_RAW_SOCKET */

/* lets allocate a socket entry, and initialize protocol number 
 * owner count initialized to 1 in tfSocketAllocate() because we own
 * the socket
 */
    socketEntryPtr = tfSocketAllocate((tt8Bit)protocol, &errorCode);
 
    if (socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR)
    {
/*
 * The allocate failed either because there is no more buffers, or
 * we are exceeding the maximum number of sockets.
 */
        goto socketFinish;
    }

/* initialize the address family on the socket */
    socketEntryPtr->socProtocolFamily = (tt8Bit) family;
    if (protocol == IP_PROTOTCP)
    {
/* socket opened in connection less state. Do not allow a lookup match yet */
        socketEntryPtr->socCoFlags = (tt16Bit)
                                    (   TM_SOC_TREE_CL_FLAG
                                      | TM_SOC_TREE_NO_MATCH_FLAG);
    }
    index = tfSocketArrayInsert(socketEntryPtr);
    if ((unsigned int)index == TM_SOC_NO_INDEX)
    {
        errorCode = TM_EMFILE;
    }
    else
    {
/*
 * If an IPv4 socket is opened while IPv6 is enabled, its default address
 * should be initialized to a IPv4-mapped zero address.
 */ 
#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
        if (family == PF_INET)
        {
            tm_6_addr_to_ipv4_mapped( TM_IP_ZERO,
                                      &socketEntryPtr->socOurIfIpAddress );
        }
#endif /* TM_USE_IPV6 && TM_USE_IPV4 */
    }
socketFinish:
#ifdef TM_USE_BSD_DOMAIN
    if (errorCode == TM_EAFNOSUPPORT)
    {
        retCode = tfBsdSocket(family, type, protocol);
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
/* If there is an error, record the socket error for TM_SOCKET_ERROR */
        retCode = tfSocketReturn( socketEntryPtr, TM_SOCKET_ERROR, errorCode, 
                                  index );
    }
    return retCode;
}
