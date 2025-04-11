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
 * Description: BSD Sockets Interface (getsockopt)
 *
 * Filename: trgsocko.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trgsocko.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2011/04/22 05:50:15JST $
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
#if defined(TM_USE_BSD_DOMAIN) && defined(TM_ERROR_CHECKING)
#include <trglobal.h>
#endif /* defined(TM_USE_BSD_DOMAIN) && defined(TM_ERROR_CHECKING) */

/*
 * Get options for a socket or protocol on a socket
 */
int getsockopt(int                socketDescriptor, 
               int                protocolLevel, 
               int                optionName,
               char TM_FAR       *optionValuePtr,
               int TM_FAR        *optionLengthPtr)
{

    ttSocketEntryPtr socketEntryPtr;
    int              errorCode;
    int              retCode;
#ifdef TM_USE_BSD_DOMAIN
    int              af;
#endif /* TM_USE_BSD_DOMAIN */

/* PARAMETER CHECK */
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_ANY;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckValidLock(socketDescriptor
#ifdef TM_USE_BSD_DOMAIN
                                            , &af
#endif /* TM_USE_BSD_DOMAIN */
                                           );
#ifdef TM_USE_BSD_DOMAIN
    if (af == TM_BSD_FAMILY_NONINET) 
    {
        tm_assert(getsockopt, socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR);
        retCode = tfBsdGetSockOpt((ttVoidPtr)socketEntryPtr, protocolLevel, 
                                  optionName, optionValuePtr, optionLengthPtr);
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
        if (socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR)
        {
            errorCode = TM_EBADF;
            goto getsockoptFinish;
        }
    
        if (    (optionName < 0)
             || (optionValuePtr == (char TM_FAR *)0)
             || (optionLengthPtr == (int TM_FAR *) 0) )
        {
            errorCode = TM_EINVAL;
            goto getsockoptFinish;
        }
    
        errorCode = TM_ENOERROR;
    
        if (protocolLevel == SOL_SOCKET)
        {
    
/* Check the socket option for correctness and get it if it is correct */
            switch (optionName)
            {
/* socket has had listen() */
                case SO_ACCEPTCONN:     
/* No Break Because all boolean options are processed the same */
    
/* allow local address reuse */
                case SO_REUSEADDR:      
/* No Break Because all boolean options are processed the same */
    
/* keep connections alive */
                case SO_KEEPALIVE:      
/* No Break Because all boolean options are processed the same */
    
/* leave received OOB data in line */
                case SO_OOBINLINE:      
/* No Break Because all boolean options are processed the same */
    
#ifdef TM_USE_REUSEPORT
/* allow local address & port reuse */
                case SO_REUSEPORT:      
#endif /* TM_USE_REUSEPORT */
/* No Break Because all boolean options are processed the same */

/* Allow send to broadcast address */
                case SO_BROADCAST:
/* No Break Because all boolean options are processed the same */
    
/* DSP only: user data is unpacked */
                case SO_UNPACKEDDATA:
/* No Break Because all boolean options are processed the same */
    
                    if (*optionLengthPtr < (int)sizeof(int))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        *optionLengthPtr = sizeof(int);
                        if (socketEntryPtr->socOptions & optionName)
                        {
                            *((int TM_FAR *)optionValuePtr) = 1;
                        }
                        else
                        {
                            *((int TM_FAR *)optionValuePtr) = 0;
                        }
                    }
                    break;
                case SO_LINGER:         
/* linger on close if data present */
                    if (*optionLengthPtr < (int)sizeof(struct linger))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        *optionLengthPtr = sizeof(struct linger);
                        if ((socketEntryPtr->socOptions) & SO_LINGER)
                        {
                            ((struct linger TM_FAR *)(optionValuePtr))->l_onoff
                                                                        = 1;
                            ((struct linger TM_FAR *)(optionValuePtr))->l_linger
                                               = socketEntryPtr->socLingerTime;
                        }
                        else
                        {
                            ((struct linger TM_FAR *)(optionValuePtr))->l_onoff
                                                                        = 0;
                        }
                    }
                    break;
/* send buffer size */
                case SO_SNDBUF:         
                    if (*optionLengthPtr < (int)sizeof(ttUser32Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        *optionLengthPtr = sizeof(ttUser32Bit);
                        *((ttUser32Bit TM_FAR *)(optionValuePtr)) =
                                socketEntryPtr->socMaxSendQueueBytes;
                    }
                    break;
/* receive buffer size */
                case SO_RCVBUF:         
                    if (*optionLengthPtr < (int)sizeof(ttUser32Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        *optionLengthPtr = sizeof(ttUser32Bit);
                        *((ttUser32Bit TM_FAR *)(optionValuePtr)) =
                                socketEntryPtr->socMaxRecvQueueBytes;
                    }
                    break;
/* send buffer size in datagrams (non-TCP) */
                case TM_SO_SND_DGRAMS:
                    if (*optionLengthPtr < (int)sizeof(ttUser32Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        *optionLengthPtr = sizeof(ttUser32Bit);
/* we store this option internally as 16-bit */
                        *((ttUser32Bit TM_FAR *)(optionValuePtr)) =
                            (tt32Bit) socketEntryPtr->socMaxSendQueueDgrams;
                    }
                    break;
/* receive buffer size in datagrams (non-TCP) */
                case TM_SO_RCV_DGRAMS:
                    if (*optionLengthPtr < (int)sizeof(ttUser32Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        *optionLengthPtr = sizeof(ttUser32Bit);
/* we store this option internally as 16-bit */
                        *((ttUser32Bit TM_FAR *)(optionValuePtr)) =
                            (tt32Bit) socketEntryPtr->socMaxRecvQueueDgrams;
                    }
                    break;
/* send low-water mark */
                case SO_SNDLOWAT:       
                    if (*optionLengthPtr < (int)sizeof(ttUser32Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        *optionLengthPtr = (int)sizeof(ttUser32Bit);
                        *((ttUser32Bit TM_FAR *)(optionValuePtr)) =
                                socketEntryPtr->socLowSendQueueBytes;
                    }
                    break;
/* receive low-water mark */
                case SO_RCVLOWAT:       
                    if (*optionLengthPtr < (int)sizeof(ttUser32Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        *optionLengthPtr = (int)sizeof(ttUser32Bit);
                        *((ttUser32Bit TM_FAR *)(optionValuePtr)) =
                                socketEntryPtr->socLowRecvQueueBytes;
                    }
                    break;
/* send timeout not supported */
                case SO_SNDTIMEO:       
                    errorCode = TM_EOPNOTSUPP;
                    break;
/* receive timeout not supported */
                case SO_RCVTIMEO:       
                    errorCode = TM_EOPNOTSUPP;
                    break;
/* don't route option not supported */
                case SO_DONTROUTE:
                    errorCode = TM_EOPNOTSUPP;
                    break;
/* Retrieve the kernel socket error, and reset it */
                case SO_ERROR:
                    if (*optionLengthPtr < (int)sizeof(int))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        if (socketEntryPtr->socRecvError != 0)
                        {
                            retCode = socketEntryPtr->socRecvError;
                            socketEntryPtr->socRecvError = 0;
                        }
                        else
                        {
                            retCode = socketEntryPtr->socSendError;
                        }
                        socketEntryPtr->socSendError = 0;
                        *optionLengthPtr = sizeof(int);
                        *((int TM_FAR *)(optionValuePtr)) = retCode;
                    }
                    break;
/* send append threshold */
                case TM_SO_SNDAPPEND:
                    if (*optionLengthPtr < (int)sizeof(unsigned int))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        *optionLengthPtr = (int)sizeof(unsigned int);
                        *((unsigned int TM_FAR *)(optionValuePtr)) =
                                       socketEntryPtr->socSendAppendThreshold;
                    }
                    break;
/* receive copy fraction */
                case TM_SO_RCVCOPY:
                    if (*optionLengthPtr < (int)sizeof(unsigned int))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        *optionLengthPtr = sizeof(unsigned int);
                        *((unsigned int TM_FAR *)(optionValuePtr)) =
                                         socketEntryPtr->socRecvCopyFraction;
                    }
                    break;
#ifdef TM_USE_STRONG_ESL
                case SO_BINDTODEVICE:
                    if (*optionLengthPtr == 0)
                    {
                        errorCode = TM_EINVAL;
                    }
                    else if (!(  socketEntryPtr->socFlags2
                               & TM_SOCF2_BINDTODEVICE))
                    {
/* BINDTODEVICE not set on this socket */
                        *optionLengthPtr = 0;
                        *optionValuePtr = (char)0;
                    }
                    else
                    {
/* Convert from device to interface name */
                        errorCode =
                            tfInterfaceName(
                                socketEntryPtr->socRteCacheStruct.rtcDevPtr,
                                optionValuePtr,
                                *optionLengthPtr);
                        if (errorCode == TM_ENOERROR)
                        {
                            optionValuePtr[(*optionLengthPtr)-1] = (char)0;
                            *optionLengthPtr = tm_strlen(optionValuePtr);
                        }
                    }
                    break;
#endif /* TM_USE_STRONG_ESL */
                default:
                    errorCode = TM_ENOPROTOOPT;
                    break;
            }  
        }
        else
        {
            switch (protocolLevel)
            {
/* Switch on the protocol number 6 for TCP and 17 for UDP */
                case IP_PROTOTCP:
#ifdef TM_USE_TCP
                    if (    socketEntryPtr->socProtocolNumber 
                         == (tt8Bit)IP_PROTOTCP )
                    {
                        errorCode = tfTcpGetOption(
                                      (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr),
                                      optionName, optionValuePtr, 
                                      optionLengthPtr);
                    }
                    else
#endif /* TM_USE_TCP */
                    {
                        errorCode = TM_EINVAL;
                    }
                    break;
/* Since IPv4 does not have any protocol number we have a special macro */
#ifdef TM_USE_IPV4
                case IP_PROTOIP:
/* Check the socket option for correctness and get it if it is correct */
                    switch (optionName)
                    {
                        case IPO_TOS:         
                            if (*optionLengthPtr < (int)sizeof(unsigned char))
                            {
                                errorCode = TM_EINVAL;
                            }
                            else
                            {
                                *optionLengthPtr = sizeof(unsigned char);
                                *((unsigned char TM_FAR *)(optionValuePtr)) =
                                        socketEntryPtr->socIpTos;
                            }
                            break;
                        case IPO_RCV_TOS:         
                            if (*optionLengthPtr < (int)sizeof(unsigned char))
                            {
                                errorCode = TM_EINVAL;
                            }
                            else
                            {
                                *optionLengthPtr = sizeof(unsigned char);
                                *((unsigned char TM_FAR *)(optionValuePtr)) =
                                        socketEntryPtr->socIpConTos;
                            }
                            break;
                        case IPO_TTL:         
                            if (*optionLengthPtr < (int)sizeof(unsigned char))
                            {
                                errorCode = TM_EINVAL;
                            }
                            else
                            {
                                *optionLengthPtr = sizeof(unsigned char);
                                *((unsigned char TM_FAR *)(optionValuePtr)) =
                                        socketEntryPtr->socIpTtl;
                            }
                            break;
                        case IPO_SRCADDR:
                            if (*optionLengthPtr < (int)sizeof(tt4IpAddress))
                            {
                                errorCode = TM_EINVAL;
                            }
                            else
                            {
                                *optionLengthPtr = sizeof(tt4IpAddress);
                                tm_ip_copy(
                                    tm_4_ip_addr(
                                        socketEntryPtr->socOurIfIpAddress),
                                    *((tt4IpAddressPtr)(optionValuePtr)));
                            }
                            break;
#ifdef TM_IGMP
                        case IPO_MULTICAST_TTL:
                        case IPO_MULTICAST_IF:
                            errorCode = tfIgmpGetSockOpt(socketEntryPtr,
                                                         optionName,
                                                         optionValuePtr,
                                                         optionLengthPtr);
                            break;
#ifdef TM_USE_IGMPV3
                        case IPO_BLOCK_SOURCE: /* exclude mode */
                        case IPO_UNBLOCK_SOURCE: /* exclude mode */
                        case IPO_ADD_SOURCE_MEMBERSHIP: /* include mode */
                        case IPO_DROP_SOURCE_MEMBERSHIP: /* include mode */
                        case IPO_ADD_MEMBERSHIP:
                        case IPO_DROP_MEMBERSHIP:
                            errorCode = TM_EOPNOTSUPP;
                            break;
#endif /* TM_USE_IGMPV3 */
#endif /* TM_IGMP */
                       case IPO_HDRINCL:
    
                            if (*optionLengthPtr < (int)sizeof(int))
                            {
                                errorCode = TM_EINVAL;
                            }
                            else
                            {
                                *optionLengthPtr = sizeof(int);
                                if (socketEntryPtr->socOptions & SO_IPHDRINCL)
                                {
                                    *((int TM_FAR *)optionValuePtr) = 1;
                                }
                                else
                                {
                                    *((int TM_FAR *)optionValuePtr) = 0;
                                }
                            }
                            break;
                        default:
                            errorCode = TM_ENOPROTOOPT;
                            break;
                    }
                    break;
#endif /* TM_USE_IPV4 */
    
#ifdef TM_USE_IPV6
                case IPPROTO_IPV6:
                    switch (optionName)
                    {
#ifdef TM_6_USE_MIP_MN
                    case TM_6_IPO_MN_USE_HA_TUNNEL:
                        if (*optionLengthPtr < (int)sizeof(int))
                        {
                            errorCode = TM_EINVAL;
                        }
                        else
                        {
                            *optionLengthPtr = sizeof(int);
                            if (socketEntryPtr->socOptions & optionName)
                            {
                                *((int TM_FAR *)optionValuePtr) = 1;
                            }
                            else
                            {
                                *((int TM_FAR *)optionValuePtr) = 0;
                            }
                        }
                        break;
#endif /* TM_6_USE_MIP_MN */

                    case TM_6_IPV6_FLOWINFO:
                        if (*optionLengthPtr < (int)sizeof(ttUser32Bit))
                        {
                            errorCode = TM_EINVAL;
                        }
                        else
                        {
                            *optionLengthPtr = sizeof(ttUser32Bit);
                            *((ttUser32Bit TM_FAR *)(optionValuePtr)) =
                                    socketEntryPtr->soc6FlowInfo;
                        }
                        break;

                        case IPV6_UNICAST_HOPS:
                            if (*optionLengthPtr < (int)sizeof(int))
                            {
                                errorCode = TM_EINVAL;
                            }
                            else
                            {
                                *optionLengthPtr = sizeof(int);
                                *((int TM_FAR *)(optionValuePtr)) =
                                    socketEntryPtr->soc6HopLimit;
                            }
                            break;
                            
                        case IPV6_MULTICAST_HOPS:
                            if (*optionLengthPtr < (int)sizeof(int))
                            {
                                errorCode = TM_EINVAL;
                            }
                            else
                            {
                                *optionLengthPtr = sizeof(int);
                                *((ttIntPtr) optionValuePtr) =
                                        socketEntryPtr->soc6McastHopLimit;
                            }
                            break;

#ifdef TM_6_USE_MLD
                        case IPV6_MULTICAST_IF:
                            errorCode = tf6MldGetSockOpt(socketEntryPtr,
                                                         optionName,
                                                         optionValuePtr,
                                                         optionLengthPtr);
                            break;
#endif /* TM_6_USE_MLD */

#ifdef TM_6_USE_RAW_SOCKET
                        case IPV6_CHECKSUM:
                            if (*optionLengthPtr < (int)sizeof(int))
                            {
                                errorCode = TM_EINVAL;
                            }
                            else
                            {
                                *optionLengthPtr = sizeof(int);
                                *((int TM_FAR *)(optionValuePtr)) =
                                    socketEntryPtr->soc6RawChecksumOffset;
                            }
                            break;

                        case TM_6_IPO_HDRINCL:
                            if (*optionLengthPtr < (int)sizeof(int))
                            {
                                errorCode = TM_EINVAL;
                            }
                            else
                            {
                                *optionLengthPtr = sizeof(int);
                                if (socketEntryPtr->socOptions & SO_IPHDRINCL)
                                {
                                    *((int TM_FAR *)optionValuePtr) = 1;
                                }
                                else
                                {
                                    *((int TM_FAR *)optionValuePtr) = 0;
                                }
                            }
                            break;
                            
#endif /* TM_6_USE_RAW_SOCKET */

#ifdef TM_6_USE_MULTICAST_LOOP
/* Loopback outgoing IPv6 multicast packets for local sockets that have
 * joined the destination group. */
                        case IPV6_MULTICAST_LOOP:
                            if (*optionLengthPtr < (int)sizeof(int))
                            {
                                errorCode = TM_EINVAL;
                            }
                            else
                            {
                                *optionLengthPtr = sizeof(int);
                                if (socketEntryPtr->soc6Flags
                                                & TM_6_SOCF_NO_MCAST_LOOP)
                                {
                                    *((int TM_FAR *)optionValuePtr) = 0;
                                }
                                else
                                {
                                    *((int TM_FAR *)optionValuePtr) = 1;
                                }
                            }
                            break;
#endif /* TM_6_USE_MULTICAST_LOOP */

/* Restrict the socket to IPv6 communications only */
                        case IPV6_V6ONLY:
                            if (*optionLengthPtr < (int)sizeof(int))
                            {
                                errorCode = TM_EINVAL;
                            }
                            else
                            {
                                *optionLengthPtr = sizeof(int);
                                if (socketEntryPtr->soc6Flags
                                                & TM_6_SOCF_V6ONLY)
                                {
                                    *((int TM_FAR *)optionValuePtr) = 1;
                                }
                                else
                                {
                                    *((int TM_FAR *)optionValuePtr) = 0;
                                }
                            }
                            break;

                        default:
                            errorCode = TM_ENOPROTOOPT;
                            break;
                    }
                    break;
#endif /* TM_USE_IPV6 */
                    
                default:
                    if (protocolLevel < 0)
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        errorCode = TM_ENOPROTOOPT;
                    }
                    break;
            }
        }
getsockoptFinish:
        retCode = tfSocketReturn(socketEntryPtr, socketDescriptor,
                                 errorCode, TM_ENOERROR);
    }
    return retCode;
}

