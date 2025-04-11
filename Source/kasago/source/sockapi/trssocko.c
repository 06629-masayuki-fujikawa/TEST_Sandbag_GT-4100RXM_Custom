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
 * Description: BSD Sockets Interface (setsockopt)
 *
 * Filename: trssocko.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trssocko.c $
 *
 * Modification History
 * $Revision: 6.0.2.4 $
 * $Date: 2011/04/22 05:50:16JST $
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
#if defined(TM_ERROR_CHECKING) && defined(TM_USE_BSD_DOMAIN)
#include <trglobal.h>
#endif /* defined(TM_ERROR_CHECKING) && defined(TM_USE_BSD_DOMAIN) */

#ifdef TM_USE_STRONG_ESL
#include <trglobal.h>
#endif /* TM_USE_STRONG_ESL */

#ifdef TM_USE_STRONG_ESL
static int tfCheckAddressConfig (ttIpAddressPtr ipAddrPtr,
                                 ttDeviceEntryPtr devPtr);
#endif /* TM_USE_STRONG_ESL */

/*
 * Set options for a socket or protocol on a socket
 */
int setsockopt(int                socketDescriptor, 
               int                protocolLevel, 
               int                optionName,
               const char TM_FAR *optionValuePtr,
               int                optionLength)
{

    ttSocketEntryPtr socketEntryPtr;
    ttTcpVectPtr     tcpVectPtr;
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr devPtr;
#endif /* TM_USE_STRONG_ESL */
#ifdef TM_USE_IPV4
    ttSockTuple      sockTuple;
#endif /* TM_USE_IV4 */
    tt32Bit          rcvWndSize;
    tt32Bit          maxRcvWindow32;
    int              errorCode;
    int              retCode;
#ifdef TM_USE_IPV6
    int              setHopLimit;
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_BSD_DOMAIN
    int              af;
#endif /* TM_USE_BSD_DOMAIN */
#ifdef TM_USE_IPV4
    tt16Bit          treeFlag;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV4
#ifdef TM_USE_STRONG_ESL
#ifndef TM_SINGLE_INTERFACE_HOME
    tt16Bit          anyMhomeIndex;
#endif /* !TM_SINGLE_INTERFACE_HOME */
#endif /* TM_USE_STRONG_ESL */
#endif /* TM_USE_IPV4 */

/* PARAMETER CHECK */
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_ANY;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckValidLock(  socketDescriptor
#ifdef TM_USE_BSD_DOMAIN
                                            , &af
#endif /* TM_USE_BSD_DOMAIN */
                                           );
#ifdef TM_USE_BSD_DOMAIN
    if (af == TM_BSD_FAMILY_NONINET)
    {
        tm_assert(setsockopt, socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR);
        retCode = tfBsdSetSockOpt( socketEntryPtr, protocolLevel, optionName,
                                   optionValuePtr, optionLength);
    }
    else
#endif /* TM_USE_BSD_DOMAIN */
    {
        if (socketEntryPtr == TM_SOCKET_ENTRY_NULL_PTR)
        {
            errorCode = TM_EBADF;
            goto setsockoptFinish;
        }
        tcpVectPtr = (ttTcpVectPtr)((ttVoidPtr)socketEntryPtr);
        if (   (optionName < 0)
            || (optionValuePtr == (const char TM_FAR *)0)
            || (optionLength <= 0) )
        {
            errorCode = TM_EINVAL;
            goto setsockoptFinish;
        }
    
        errorCode = TM_ENOERROR;
    
        if (protocolLevel == SOL_SOCKET)
        {
    
/* Check the socket option for correctness and set it if it is correct */
            switch (optionName)
            {
/*
 * The following are Boolean Options so we just turn them
 * on and off
 */
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
    
                    if (optionLength != (int)sizeof(int))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        if (*((int TM_FAR *)(optionValuePtr)) == 0)
                        {
/* Turn it Off */
                            socketEntryPtr->socOptions &= ~((tt16Bit)optionName);
                        }
                        else
                        {
/* Turn it On */
                            socketEntryPtr->socOptions |= ((tt16Bit)optionName);
                        }
                    }   
                    break;
    
/* linger on close if data present */
                case SO_LINGER:         
                    if (optionLength < (int)sizeof(struct linger))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        if (((struct linger TM_FAR *)(optionValuePtr))
                                                            ->l_onoff == 1)
                        {
                            socketEntryPtr->socLingerTime = (tt16Bit)
                               (((struct linger TM_FAR *)(optionValuePtr))->
                                                                    l_linger);
                            socketEntryPtr->socOptions |= SO_LINGER;
                        }
                        else
                        {
                            socketEntryPtr->socLingerTime = 0;
                            tm_16bit_clr_bit( socketEntryPtr->socOptions,
                                              SO_LINGER );
                        }
                    }
                    break;
    
/* send buffer size */
                case SO_SNDBUF:         
                    if (optionLength < (int)sizeof(tt32Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        socketEntryPtr->socMaxSendQueueBytes = 
                            *((tt32BitPtr)(optionValuePtr));
/* Automatically adjust the low water mark */
                        socketEntryPtr->socLowSendQueueBytes =
                            tm_send_low_water(*((tt32BitPtr)(optionValuePtr)));
                    }
                    break;
    
/* receive buffer size */
                case SO_RCVBUF:         
                    if (optionLength < (int)sizeof(tt32Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        rcvWndSize = *((tt32BitPtr)(optionValuePtr));
                        if (    (socketEntryPtr->socProtocolNumber ==
                                                          (tt8Bit)IP_PROTOTCP)
                             && (tcpVectPtr->tcpsState > TM_TCPS_LISTEN) )
                        {
/*
 * With TCP, after connection has started, assume we are not allowing the
 * change of the socket recv buffer. 
 * 1. We cannot allow shrinking of the receive window after the connection
 *    establishment has started. We will reset that error down below so
 *    that it is not returned to the user, so that we can silently ignore
 *    shrinking of the receive window (socketpair()).
 * 2. If the change results in a bigger recv window we will allow the
 *    change based on the TCP window scale. 
 */
                            errorCode = TM_EPERM;
                            if (rcvWndSize
                                    > socketEntryPtr->socMaxRecvQueueBytes)
/* recv window not being shrunk */
                            {
/* Allow change */
                                errorCode = TM_ENOERROR;
/*
 * Restrict the value to whatever TCP window scale is being used, but note
 * that TCP always makes that check before sending a recv window
 * adverstisement.
 */
                                if (tcpVectPtr->tcpsRcvWndScale
                                                         != TM_8BIT_ZERO)
                                {
                                    maxRcvWindow32 = TM_TCP_MAX_SCALED_WND
                                                << tcpVectPtr->tcpsRcvWndScale;
                                }
                                else
                                {
                                    maxRcvWindow32 = TM_TCP_MAX_SCALED_WND;
                                }
                                if (rcvWndSize > maxRcvWindow32)
/* 
 * Silently adjust to maximum allowed based on negotiated window scale, but
 * note that TCP will always make that check
 */
                                {
                                    rcvWndSize = maxRcvWindow32;
                                }
                            }
                        }
                        if (errorCode == TM_ENOERROR)
                        {
                            socketEntryPtr->socMaxRecvQueueBytes = rcvWndSize;
/* Automatically adjust the low water mark */
                            socketEntryPtr->socLowRecvQueueBytes =
                                                tm_recv_low_water(rcvWndSize);
                        }
                        if (errorCode == TM_EPERM)
                        {
/* Silently ignore shrinking of the receive window (socketpair()) */
                            errorCode = TM_ENOERROR;
                        }
                    }
                    break;
/* send buffer size in datagrams (non-TCP) */
                case TM_SO_SND_DGRAMS:
                    if ((optionLength < (int)sizeof(tt32Bit))
                        || (socketEntryPtr->socProtocolNumber ==
                            (tt8Bit)IP_PROTOTCP))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
/* we store this option internally as 16-bit */
                        socketEntryPtr->socMaxSendQueueDgrams =
                            (tt16Bit) *((tt32BitPtr)(optionValuePtr));
                    }
                    break;
    
/* receive buffer size in datagrams (non-TCP) */
                case TM_SO_RCV_DGRAMS:
                    if ((optionLength < (int)sizeof(tt32Bit))
                        || (socketEntryPtr->socProtocolNumber ==
                            (tt8Bit)IP_PROTOTCP))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
/* we store this option internally as 16-bit */
                        socketEntryPtr->socMaxRecvQueueDgrams =
                            (tt16Bit) *((tt32BitPtr)(optionValuePtr));
                    }
                    break;
    
/* send low-water mark */
                case SO_SNDLOWAT:       
                    if (optionLength < (int)sizeof(tt32Bit))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        socketEntryPtr->socLowSendQueueBytes = 
                            *((tt32BitPtr)(optionValuePtr));
                        
                    }
                    break;
    
/* receive low-water mark */
                case SO_RCVLOWAT:       
#ifdef TM_ERROR_CHECKING
                    if (    (optionLength < (int)sizeof(tt32Bit))
/* Do not allow low water mark bigger than maximum receive queue size */
                         || (*((tt32BitPtr)(optionValuePtr)) >
                                          socketEntryPtr->socMaxRecvQueueBytes) )
#else /* TM_ERROR_CHECKING */
                    if ( (optionLength < (int)sizeof(tt32Bit)) )
#endif /* TM_ERROR_CHECKING */
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        socketEntryPtr->socLowRecvQueueBytes = 
                            *((tt32BitPtr)(optionValuePtr));
                        
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

                case TM_SO_SNDAPPEND:
                    if (optionLength != (int)sizeof(unsigned int))
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        socketEntryPtr->socSendAppendThreshold =
                            (tt16Bit)
                            (*((unsigned int TM_FAR *)(optionValuePtr)));
                    }   
                    break;
                case TM_SO_RCVCOPY:
                    if (    (optionLength != (int)sizeof(unsigned int))
                         || (*((unsigned int TM_FAR*)optionValuePtr) == 0) )
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        socketEntryPtr->socRecvCopyFraction =
                            (tt16Bit)(*((unsigned int TM_FAR *)optionValuePtr));
                    }   
                    break;
#ifdef TM_USE_STRONG_ESL
                case SO_BINDTODEVICE:
                    {
/* First field in struct ifreq pointed to by optionValuePtr */
                        if (*optionValuePtr == '\0')
                        {
                            socketEntryPtr->socFlags2 &=
                                         ~TM_SOCF2_BINDTODEVICE;
                            socketEntryPtr->socRteCacheStruct.rtcDevPtr =
                                                           (ttDeviceEntryPtr)0;
#ifdef TM_SNMP_CACHE
                            socketEntryPtr->socTupleDev.tudDevPtr =
                                                           (ttDeviceEntryPtr)0;
#endif /* TM_SNMP_CACHE */
                        }
                        else
                        {
/* Convert from name to device pointer */
                            devPtr = (ttDeviceEntryPtr)
                                      tfInterfaceNameToDevice(optionValuePtr);
                            if (devPtr != (ttDeviceEntryPtr)0)
                            {
                                if (    tm_16bit_one_bit_set(
                                                socketEntryPtr->socFlags,
                                                TM_SOCF_BIND)
                                     || tm_8bit_one_bit_set(
                                                socketEntryPtr->socFlags2,
                                                TM_SOCF2_IP_SRCADDR) )
                                {
                                    errorCode = tfCheckAddressConfig(
                                            &socketEntryPtr->socOurIfIpAddress,
                                            devPtr);
                                }
                                if (errorCode == TM_ENOERROR)
                                {
                                    socketEntryPtr->socFlags2 |=
                                             TM_SOCF2_BINDTODEVICE;
                                    socketEntryPtr->socRteCacheStruct.
                                                           rtcDevPtr = devPtr;
#ifdef TM_SNMP_CACHE
                                    socketEntryPtr->socTupleDev.tudDevPtr =
                                                                       devPtr;
#endif /* TM_SNMP_CACHE */
                                }
                            }
                            else
                            {
                                errorCode = TM_ENOENT;
                            }
                        }
                    }
                    break;
#endif /* TM_USE_STRONG_ESL */
/* No Matching Option so not supported */
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
                        errorCode = tfTcpSetOption( tcpVectPtr,
                                                    optionName, optionValuePtr,
                                                    optionLength);
                    }
                    else
#endif /* TM_USE_TCP */
                    {
                        errorCode = TM_EINVAL;
                    }
                    break;
/* Since IP does not have any protocol number we have a special macro */
#ifdef TM_USE_IPV4
                case IP_PROTOIP:
                    switch (optionName)
                    {
    
                        case IPO_TOS:
#ifdef TM_DSP
                            if (optionLength < (int)sizeof(unsigned char))
                            {
                                errorCode = TM_EINVAL;
                            }
                            else
#endif /* TM_DSP */
                            {
                                socketEntryPtr->socIpTos = 
                                  *((unsigned char TM_FAR *)(optionValuePtr));
                            }
                            break;
                        case IPO_TTL:
#ifdef TM_DSP
                            if (optionLength < (int)sizeof(unsigned char))
                            {
                                errorCode = TM_EINVAL;
                            }
                            else
#endif /* TM_DSP */
                            {
                                socketEntryPtr->socIpTtl = 
                                  *((unsigned char TM_FAR *)(optionValuePtr));
                            }
                            break;
    
                        case IPO_SRCADDR:
                            if (optionLength < (int)sizeof(tt4IpAddress))
                            {
                                errorCode = TM_EINVAL;
                            }
                            else
                            {
                                tm_bcopy(&(socketEntryPtr->socTuple),
                                         &sockTuple,
                                         sizeof(sockTuple));
/* This option is an IPv4 option only */
                                tm_ip_copy(
                                        *((tt4IpAddressPtr)(optionValuePtr)),
                                        tm_4_ip_addr(
                                                sockTuple.sotLocalIpAddress));
#ifdef TM_USE_STRONG_ESL
                                if (tm_8bit_one_bit_set(
                                                    socketEntryPtr->socFlags2,
                                                    TM_SOCF2_BINDTODEVICE))
                                {
                                    devPtr = socketEntryPtr->
                                                  socRteCacheStruct.rtcDevPtr;
#ifdef TM_SINGLE_INTERFACE_HOME
                                    if (  tm_8bit_bits_not_set(
                                            tm_ip_dev_conf_flag(devPtr, 0),
                                            TM_DEV_IP_CONFIG)
                                        && tm_ip_not_match(
                                              tm_4_ip_addr(
                                                 sockTuple.sotLocalIpAddress),
                                              tm_ip_dev_addr(devPtr, 0)) )
#else /* !TM_SINGLE_INTERFACE_HOME */
                                    if (tfMhomeAddrMatch(
                                             devPtr,
                                             tm_4_ip_addr(
                                                 sockTuple.sotLocalIpAddress),
                                             &anyMhomeIndex) != TM_ENOERROR )
#endif /* !TM_SINGLE_INTERFACE_HOME */
                                    {
/* could not find the specified IPv4 address on the bound interface */
                                        errorCode = TM_EINVAL;
                                    }
                                }
                                if (errorCode == TM_ENOERROR)
#endif /* TM_USE_STRONG_ESL */
                                {
                                    if (tm_16bit_one_bit_set(
                                               socketEntryPtr->socFlags,
                                               TM_SOCF_IN_TREE))
/* Socket already in the tree. Reinsert */
                                    {
                                        treeFlag = (tt16Bit) (
                                         socketEntryPtr->socCoFlags
                                       | (tt16Bit)(  socketEntryPtr->socOptions
                                                   & (  SO_REUSEADDR
                                                      | SO_REUSEPORT)) );
                                        retCode = tfSocketTreeReInsert(
                                                            socketEntryPtr,
                                                            &(sockTuple),
                                                            treeFlag );
                                        if (retCode != TM_SOC_RB_OKAY)
                                        {
                                            errorCode = TM_EADDRINUSE;
                                        }
                                    }
                                    else
/*
 * Do not insert in the tree, if socket is not in the tree yet.
 */
                                    {
                                       tm_bcopy(&sockTuple, &(socketEntryPtr->socTuple),
                                                sizeof(sockTuple));
                                    }
                                    if (errorCode == TM_ENOERROR)
                                    {
/*
 * BUG ID 203: if socket source IP address has been assigned by calling 
 * setsockopt with option IPO_SRCADDR, we should use the assigned IP
 * address.
 */
                                        socketEntryPtr->socFlags2 |= 
                                                        TM_SOCF2_IP_SRCADDR;
                                    }
                                }
                            }
                            break;
#ifdef TM_IGMP
                        case IPO_MULTICAST_TTL:
                        case IPO_MULTICAST_IF:
                        case IPO_ADD_MEMBERSHIP:
                        case IPO_DROP_MEMBERSHIP:
#ifdef TM_USE_IGMPV3
                        case IPO_BLOCK_SOURCE:
                        case IPO_UNBLOCK_SOURCE:
                        case IPO_ADD_SOURCE_MEMBERSHIP:
                        case IPO_DROP_SOURCE_MEMBERSHIP:
#endif /* TM_USE_IGMPV3 */
                            errorCode = tfIgmpSetSockOpt(socketEntryPtr,
                                                         optionName,
                                                         optionValuePtr,
                                                         optionLength);
                            break;
#endif /* TM_IGMP */
                        case IPO_HDRINCL:
                            if (optionLength != (int)sizeof(int))
                            {
                                errorCode = TM_EINVAL;
                            }
                            else
                            {
                                if (    (    socketEntryPtr->socProtocolNumber
                                          != IP_PROTOUDP)
                                     && (    socketEntryPtr->socProtocolNumber
                                          != IP_PROTOTCP) )
                                {
                                    if (*((int TM_FAR *)(optionValuePtr)) == 0)
                                    {
/* Turn it Off */
                                        tm_16bit_clr_bit( 
                                                socketEntryPtr->socOptions,
                                                SO_IPHDRINCL );
                                    }
                                    else
                                    {
/* Turn it On */
                                        socketEntryPtr->socOptions
                                                            |= SO_IPHDRINCL;
                                    }
                                }
                                else
                                {
                                    errorCode = TM_EPERM;
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
                    if (socketEntryPtr->socProtocolFamily == PF_INET6)
                    {
                        switch (optionName)
                        {
#ifdef TM_6_USE_MIP_MN
                        case TM_6_IPO_MN_USE_HA_TUNNEL:
                            if (optionLength != (int)sizeof(int))
                            {
                                errorCode = TM_EINVAL;
                            }
                            else
                            {
                                if (*((int TM_FAR *)(optionValuePtr)) == 0)
                                {
/* Turn it Off */
                                    tm_16bit_clr_bit( 
                                                socketEntryPtr->socOptions,
                                                (tt16Bit)optionName );
                                }
                                else
                                {
/* Turn it On */
                                    socketEntryPtr->socOptions
                                        |= ((tt16Bit)optionName);
                                }
                            }   
                            break;
#endif /* TM_6_USE_MIP_MN */

                        case TM_6_IPV6_FLOWINFO:
                            if (optionLength < (int)sizeof(tt32Bit))
                            {
                                errorCode = TM_EINVAL;
                            }
                            else
                            {
                                socketEntryPtr->soc6FlowInfo = 
                                  *((tt32BitPtr)(optionValuePtr));
                            }
                            break;

                            case IPV6_UNICAST_HOPS:
                                if (optionLength < (int)sizeof(int))
                                {
                                    errorCode = TM_EINVAL;
                                }
                                else
                                {
    
                                    setHopLimit = *(int TM_FAR *)optionValuePtr;
                                    if (setHopLimit < -1 || setHopLimit > 255 )
                                    {
/* error if < -1 or > 255 */
                                        errorCode = TM_EINVAL;
                                    }
                                    else
                                    {
                                        socketEntryPtr->soc6HopLimit = 
                                                setHopLimit;
                                    }
                                }
                                break;
                            case IPV6_MULTICAST_HOPS:
                                if (optionLength < (int)sizeof(int))
                                {
                                    errorCode = TM_EINVAL;
                                }
                               else
                                {
                                    socketEntryPtr->soc6McastHopLimit
                                        = *((ttIntPtr) optionValuePtr);
                                }
                                break;

                            case IPV6_MULTICAST_IF:
#ifdef TM_6_USE_MLD
                            case IPV6_JOIN_GROUP:
                            case IPV6_LEAVE_GROUP:
#endif /* TM_6_USE_MLD */
                                errorCode = tf6MldSetSockOpt(
                                    socketEntryPtr,
                                    optionName,
                                    optionValuePtr,
                                    optionLength);
                                break;

#ifdef TM_6_USE_RAW_SOCKET
                            case IPV6_CHECKSUM:
                                if (optionLength != (int)sizeof(int))
                                {
                                    errorCode = TM_EINVAL;
                                }
                                else
                                {
                                    socketEntryPtr->soc6RawChecksumOffset =
                                        *((int TM_FAR *)(optionValuePtr));
                                }
                                break;

                            case TM_6_IPO_HDRINCL:
                                if (optionLength != (int)sizeof(int))
                                {
                                    errorCode = TM_EINVAL;
                                }
                                else
                                {
                                    if (*((int TM_FAR *)(optionValuePtr)) == 0)
                                    {
/* Turn it Off */
                                        socketEntryPtr->socOptions &=
                                            ~SO_IPHDRINCL;
                                    }
                                    else
                                    {
/* Turn it On */
                                        socketEntryPtr->socOptions |=
                                            SO_IPHDRINCL;
                                    }
                                }   
                                break;
                                
#endif /* TM_6_USE_RAW_SOCKET */

#ifdef TM_6_USE_MULTICAST_LOOP
/* Loopback outgoing IPv6 multicast packets for local sockets that have
 * joined the destination group. */
                            case IPV6_MULTICAST_LOOP:
                                if (optionLength != (int)sizeof(int))
                                {
                                    errorCode = TM_EINVAL;
                                }
                                else
                                {
                                    if (*(int TM_FAR *)optionValuePtr)
                                    {
                                        socketEntryPtr->soc6Flags &= (tt16Bit)
                                                ~TM_6_SOCF_NO_MCAST_LOOP;
                                    }
                                    else
                                    {
                                        socketEntryPtr->soc6Flags |=
                                                TM_6_SOCF_NO_MCAST_LOOP;
                                    }
                                }
                                break;
#endif /* TM_6_USE_MULTICAST_LOOP */

/* Restrict the socket to IPv6 communications only */
                            case IPV6_V6ONLY:
                                if (optionLength != (int)sizeof(int))
                                {
                                    errorCode = TM_EINVAL;
                                }
                                else
                                {
                                    if (*(int TM_FAR *)optionValuePtr)
                                    {
                                        socketEntryPtr->soc6Flags |=
                                                TM_6_SOCF_V6ONLY;
                                    }
                                    else
                                    {
                                        tm_16bit_clr_bit( 
                                                    socketEntryPtr->soc6Flags,
                                                    TM_6_SOCF_V6ONLY);
                                    }
                                }
                                break;

                            default:
                                errorCode = TM_ENOPROTOOPT;
                        }
                    }
                    else
                    {
                        errorCode = TM_ENOPROTOOPT;
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
#ifdef TM_USE_TCP
        if (socketEntryPtr->socProtocolNumber == (tt8Bit)IP_PROTOTCP)
        {
            tfTcpPurgeTcpHeaders(tcpVectPtr);
        }
#endif /* TM_USE_TCP */
setsockoptFinish:
        retCode =  tfSocketReturn(socketEntryPtr, socketDescriptor,
                                  errorCode, TM_ENOERROR);
    }
    return retCode;
}

#ifdef TM_USE_STRONG_ESL
static int tfCheckAddressConfig (ttIpAddressPtr ipAddrPtr,
                                 ttDeviceEntryPtr devPtr)

{
#ifdef TM_USE_IPV4
    tt4IpAddress ipAddress;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    tt32Bit      scopeId;
#endif /* TM_USE_IPV6 */
    int          errorCode;
#ifndef TM_SINGLE_INTERFACE_HOME
    tt16Bit      anyMhomeIndex;
#endif /* !TM_SINGLE_INTERFACE_HOME */

    errorCode = TM_ENOERROR;
/* Check that the socket source address is configured on the device */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
    if (!(IN6_IS_ADDR_V4MAPPED(ipAddrPtr)))
#endif /* TM_USE_IPV4 */
    {
        if (!tm_6_ip_zero(ipAddrPtr) )
        {
            scopeId = (tt32Bit)0;
            if (IN6_IS_ADDR_LINKLOCAL(ipAddrPtr))
            {
/* Clear scope id */
                scopeId = (ipAddrPtr)->s6LAddr[1];
                tm_6_clear_scope_id(ipAddrPtr)
            }
/* Check that bound IP address is a valid address on the interface to be bound */
            if (tf6MhomeAddrMatch(
                    devPtr,
                    ipAddrPtr,
                    &anyMhomeIndex) != TM_ENOERROR )
            {
/* address is not configured on bound interface */
                errorCode = TM_EINVAL;
            }
            if (scopeId != (tt32Bit)0)
/* Restore scope */
            {
                tm_6_embed_scope_id(ipAddrPtr, scopeId)
            }
        }
    }
#ifdef TM_USE_IPV4
    else
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
    {
        ipAddress  = tm_4_ip_addr((*ipAddrPtr));
        if (tm_ip_not_zero(ipAddress) )
        {
#ifdef TM_SINGLE_INTERFACE_HOME
            if (  tm_8bit_bits_not_set( tm_ip_dev_conf_flag(devPtr, 0),
                                        TM_DEV_IP_CONFIG)
                && tm_ip_not_match(ipAddress, tm_ip_dev_addr(devPtr, 0)) )
#else /* !TM_SINGLE_INTERFACE_HOME */
            if (tfMhomeAddrMatch(
                    devPtr,
                    ipAddress,
                    &anyMhomeIndex) != TM_ENOERROR )
#endif /* !TM_SINGLE_INTERFACE_HOME */
            {
/* could not find the specified IPv4 address on the specified interface */
                errorCode = TM_EINVAL;
            }
        }
    }
#endif /* TM_USE_IPV4 */
    return errorCode;
}
#endif /* TM_USE_STRONG_ESL */
