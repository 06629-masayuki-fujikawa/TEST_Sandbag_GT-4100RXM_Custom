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
 * Description:  rename BSD datatypes and functions, so that Treck stack 
 *               can be compiled as an application on BSD compiant systems.
 *
 * Filename: trbsd.h
 * Author: Jin Shang
 * Date Created: 8/15/2002
 * $Source: include/trbsd.h $
 *
 * Modification History
 * $Revision: 6.0.2.4 $
 * $Date: 2010/11/25 06:17:04JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TRBSD_H_
#define _TRBSD_H_

#undef AF_INET
#undef PF_INET
#undef AF_INET6
#undef PF_INET6

#undef IPPORT_RESERVED
#undef IPPROTO_IP 
#undef IPPROTO_ICMP
#undef IPPROTO_IGMP
#undef IPPROTO_TCP
#undef IPPROTO_UDP
#undef IPPROTO_SCTP
#undef IPPROTO_HOPOPTS
#undef IPPROTO_IPV6
#undef IPPROTO_ROUTING
#undef IPPROTO_FRAGMENT
#undef IPPROTO_ESP
#undef IPPROTO_AH
#undef IPPROTO_ICMPV6
#undef IPPROTO_NONE
#undef IPPROTO_DSTOPTS

#ifdef IPPROTO_IPV4
#undef IPPROTO_IPV4
#endif /* IPPROTO_IPV4 */

#undef SOCK_STREAM
#undef SOCK_DGRAM
#undef SOCK_RAW 
#undef SOCK_RDM
#undef SOCK_SEQPACKET 

#undef SO_DEBUG 
#undef SO_ACCEPTCONN
#undef SO_REUSEADDR
#undef SO_KEEPALIVE 
#undef SO_DONTROUTE
#undef SO_BROADCAST
#undef SO_LINGER 
#undef SO_OOBINLINE
#undef SO_TIMESTAMP 
#undef SO_SNDBUF 
#undef SO_RCVBUF
#undef SO_SNDLOWAT
#undef SO_RCVLOWAT 
#undef SO_SNDTIMEO
#undef SO_RCVTIMEO
#undef SO_ERROR 
#undef SO_TYPE 
#undef SO_BINDTODEVICE

#undef FIONBIO 
#undef FIONREAD
#undef SIOCATMARK
#undef IP_TTL 
#undef IP_TOS 
#undef IP_MULTICAST_TTL 
#undef IP_MULTICAST_IF 
#undef IP_HDRINCL 
#undef IP_ADD_MEMBERSHIP
#undef IP_DROP_MEMBERSHIP

#ifdef TM_USE_IGMPV3
#undef IP_BLOCK_SOURCE
#undef IP_UNBLOCK_SOURCE
#undef IP_ADD_SOURCE_MEMBERSHIP
#undef IP_DROP_SOURCE_MEMBERSHIP
#undef IP_MSFILTER_SIZE
#endif /* TM_USE_IGMPV3 */

#undef MSG_OOB 
#undef MSG_PEEK
#undef MSG_DONTROUTE
#undef MSG_EOR  
#undef MSG_TRUNC 
#undef MSG_CTRUNC
#undef MSG_WAITALL
#undef MSG_DONTWAIT

#undef SOL_SOCKET 
#undef NFDBITS
#undef FD_SET
#undef fd_set
#undef FD_CLR
#undef FD_ISSET
#undef FD_SETSIZE
#undef FD_ZERO
#undef IF_NAMESIZE 
#undef INADDR_ANY
#undef INADDR_LOOPBACK

/* for TM_USE_IPv6 */
#undef INGADDR_ANY_INIT
#undef INGADDR_LOOPBACK_INIT
#undef IN6_ARE_ADDR_EQUAL
#undef IN6_IS_ADDR_UNSPECIFIED
#undef IN6_IS_ADDR_LOOPBACK
#undef IN6_IS_ADDR_LINKLOCAL
#undef IN6_IS_ADDR_SITELOCAL
#undef IN6_IS_ADDR_V4MAPPED
#undef IN6_IS_ADDR_V4COMPAT
#undef IN6_IS_ADDR_MC_SITELOCAL
#undef IN6_IS_ADDR_MC_ORGLOCAL
#undef IN6_IS_ADDR_MC_GLOBAL
#undef IPV6_JOIN_GROUP
#undef IPV6_LEAVE_GROUP
#undef IPV6_MULTICAST_LOOP
#undef IPV6_V6ONLY
#undef IPV6_PKTINFO
#undef IPV6_HOPLIMIT
#undef IPV6_NEXTHOP
#undef IPV6_DSTOPTS
#undef IPV6_RTHDR
#undef s6_addr
#undef s_addr
#undef INGADDR_ANY_INIT
#undef INGADDR_LOOPBACK_INIT
#undef IN6_IS_ADDR_MULTICAST
#undef IN6_IS_ADDR_MC_MODELOCAL
#undef IN6_IS_ADDR_MC_LINKLOCAL
#undef IPV6_MULTICAST_IF
#undef IPV6_MULTICAST_HOPS
#undef IPV6_HOPOPTS
#undef IN6ADDR_ANY_INIT
#undef IN6ADDR_LOOPBACK_INIT
#undef IN6_IS_ADDR_MC_NODELOCAL
#undef IPV6_UNICAST_HOPS

#undef AF_UNSPEC
#undef PF_UNSPEC
#undef EAI_AGAIN
#undef EAI_BADFLAGS
#undef EAI_FAIL
#undef EAI_FAMILY
#undef EAI_MEMORY
#undef EAI_NONAME
#undef EAI_SERVICE
#undef EAI_SOCKTYPE
#undef EAI_SYSTEM
#undef EAI_OVERFLOW
#undef AI_PASSIVE
#undef AI_CANONNAME
#undef AI_NUMERICHOST
#undef AI_NUMERICSERV
#undef AI_V4MAPPED
#undef AI_ALL
#undef AI_ADDRCONFIG
#undef NI_NOFQDN
#undef NI_NUMERICHOST
#undef NI_NAMEREQD
#undef NI_NUMERICSERV
#undef NI_DGRAM

#undef CMSG_ALIGN
#undef CMSG_FIRSTHDR
#undef CMSG_NXTHDR
#undef CMSG_DATA
#undef CMSG_SPACE
#undef CMSG_LEN


#define in6_addr                ttIn6Addr
#define ipv6_mreq               ttIpv6Mreq
#define sockaddr_in6            ttSockAddrIn6
#define in6addr_any             ttIn6AddrAny
#define in6addr_loopback        ttIn6AddrLoopBack

#define cmsghdr                 ttCmsgHdr
#define socklen_t               ttSocklenT

#define u_char                  ttUCharN
#define u_short                 ttUShortN
#define u_int                   ttUIntN
#define u_long                  ttULongN 
#define caddr_t                 ttCaddrT

#define timeval                 ttTimeval
#define sockaddr                ttSockaddr
#define in_addr                 ttInAddr
#define ip_mreq                 ttIpMreq
#ifdef TM_USE_IGMPV3
#define ip_mreq_source          ttIpMreqSource
#define ip_msfilter             ttIpMsFilter
#endif /* TM_USE_IGMPV3 */
#define sockaddr_in             ttSockaddrIn
#define sockaddr_storage        ttSockaddrStorage
#define iovec                   ttIovec
#define linger                  ttLinger
#define msghdr                  ttMsghdr

#define addrinfo                ttAddrInfo
#define getaddrinfo             tfGetAddrInfo
#define gai_strerror            tfGaiStrError
#define getnameinfo             tfGetNameInfo
#define freeaddrinfo            tfFreeAddrInfo
#define fd_set                  ttFdSet
#define fd_mask                 ttFdMask

#ifdef TM_USE_IFNAMEINDEX
#define if_nameindex            ttIfNameIndex
#define if_freenameindex        ttIfFreeNameIndex
#endif /* TM_USE_IFNAMEINDEX */

#define inet_addr               tfInetAddr
#define inet_aton               tfInetAton
#define inet_ntoa               tfInetNtoa
#define inet_ntop               tfInetNtop
#define inet_pton               tfInetPton

#define socket                  tfSocket
#define bind                    tfBind
#define send                    tfSend
#define listen                  tfListen
#define connect                 tfConnect
#define accept                  tfAccept
#define recv                    tfRecv
#define sendto                  tfSendto
#define recvfrom                tfRecvfrom
#define readv                   tfReadv
#define writev                  tfWritev
#define getipv4sourcefilter     tfGetipv4sourcefilter
#define setipv4sourcefilter     tfSetipv4sourcefilter
#define setsockopt              tfSetsockopt
#define getsockopt              tfGetsockopt
#define getpeername             tfGetpeername
#define getsockname             tfGetsockname
#define select                  tfSelect
#define shutdown                tfShutDown

#endif /* #define _TRBSD_H_ */
