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
 * Description: IGMP functions
 * Filename: trigmp.c
 * Author: Odile
 * Date Created: 08/18/99
 * $Source: source/trigmp.c $
 *
 * Modification History
 * $Revision: 6.0.2.10 $
 * $Date: 2011/07/05 07:07:59JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>  /* For TM_IGMP */
#include <trmacro.h>   /* For LINT_UNUSED_HEADER */

#if defined(TM_IGMP) && defined(TM_USE_IPV4)

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/*
 * Introduction
 * This module contains the Treck implementation of the host extension
 * for IP multicasting, and IP hosts IGMP version 1, and version 2
 * protocol, as described in RFC 1112, and RFC 2236. The IP router
 * alert option, described in RFC 2113, is also used as required by
 * RFC 2236.
 * 
 * IP multicasting is the transmission of an IP datagram to a "host group",
 * a set of zero or more hosts identified by a single IP destination address.
 * A host group IP address is a multicast IP address, or class D IP address,
 * where the first 4 bits of the address are 1110. The range of host group
 * IP addresses (class D) are from 224.0.0.1 to 239.255.255.255.
 * Class D addresses starting with 224.0.0.x and 224.0.1.x have been reserved
 * for various permanent assignments (see RFC 1700). The range of addresses
 * between 224.0.0.1 and 224.0.0.255 are local multicast addresses (i.e. they
 * cannot be forwarded beyond the local subnet), and have been reserved
 * for the use of routing protocols and other low-level topology discovery
 * or maintenance protocols, such as gateway discovery and group membership
 * recording.
 * The membership of a host group is dynamic: hosts may join and leave groups
 * at any time.
 * 
 * IGMP (Internet Group Management Protocol) is the protocol used by IP hosts
 * to report their multicast group memberships to multicast routers. Local
 * host groups (224.0.0.0 through 224.0.0.255) memberships do not need to be
 * reported, since as indicated above they cannot be forwarded beyond the
 * local subnet. Without the IGMP protocol, a host can send multicast IP
 * datagrams, but cannot receive any.
 * 
 * Description
 * We will describe the design of the IP multicasting, and the host IP IGMP
 * protocol implementation in the Treck stack in detail in two sections:
 * 1. sending multicast packets
 * 2. IGMP protocol
 * 
 * SENDING MULTICAST PACKETS:
 * send multicast API:
 * Sending multicast packets does not require new API functions. The user
 * uses the send(), and sendto() functions on a UDP socket with a multicast
 * IP address destination.
 * 
 * IP outgoing interface for multicast packets:
 * The user can choose a per socket default interface to send multicast
 * packets by using a new setsockopt() option (IPO_MULTICAST_IF) at the
 * IPPROTO_IP level with the IP address of the outgoing interface as the
 * option value. If the user does not specify a destination interface that
 * way, then we will use a default outgoing interface if the user specified
 * a system wide one after having configured the interface with the
 * tfSetMulticastInterface() call. Otherwise the multicast sendto() will fail.
 * 
 * 
 * IP ttl for multicast packets:
 * By default the IP layer will send a multicast packet with a TTL value of 1,
 * unless the user changes the default Multicast IP ttl value for the
 * socket with a setsockopt(). A TTL value of 1, will not allow the
 * multicast IP datagram to be forwarded beyond the local subnet.
 * The option IPO_MULTICAST_TTL at the IPPROTO_IP level has been added
 * to setsockopt()/getsockopt.
 * 
 * Mapping multicast packets to layer 2 addresses:
 * No work is required here. The current tfIpBroadMcastToLan() function
 * already maps from multicast address to ethernet multicast if the
 * multicast mapping is enabled for the outgoing device. The flag enabling
 * the multicast mapping, TM_DEV_MCAST_ENB, is already documented in the
 * user's manual.
 * 
 * IGMP PROTOCOL:
 * 
 * 1. RECEIVING UDP MULTICAST PACKETS:
 * When an IP datagram whose destination IP address is a multicast address
 * arrives on an interface from the network, and that IP datagram ULP (upper
 * level protocol) is not IGMP, we need to check that the host is a member
 * of the multicast group on the interface it came in. The existing IP code
 * already performs those checks and calls tfIgmpMember() to execute the
 * check. But the actual function tfIgmpMember() need to be implemented:
 * given an interface handle, and a destination multicast IP address,
 * return TM_ENOERROR if the host is a member of the destination multicast
 * IP address on the interface it came in, return TM_ENOERROR otherwise.
 * A host is a member of a group address for a given interface, if it
 * joined that multicast group.
 * If TM_USE_IGMPV3 is defined source address is verified as well.
 * 
 * 2. RECEIVING IGMP multicast packets:
 * When IGMP packets come from the network, tfIgmpIncomingPacket() is called.
 * The packet is checked for minimum length requirements, checksum, and IGMP
 * type.
 * 
 * 3. JOINING A HOST GROUP
 * To join a host group, a user socket application will use setsockopt()
 * with the IPO_ADD_MEMBERSHIP option at the IPPROTO_IP level. The option
 * pointer points to a structure containing the IP host group address, and
 * the interface (defined by its IP address).
 * It will return TM_ADDNOTAVAIL if the IP host group address is not a class D
 * address, or there is no interface configured with the IP address stored
 * in the structure. It will then call tfIgmpJoinHostGroup().
 * It will return TM_EADDRINUSE, if there was a previous
 * successful IPO_ADD_MEMBERSHIP call for that host group on the same
 * interface. Otherwise, this is the first call to IPO_ADD_MEMBERSHIP for
 * the pair. The IP host group address will be added to the interface, then
 * tfEnetMcast() will be called (for a LAN interface) to add an entry to the
 * list of ethernet multicast addresses for the device, then
 * a driver specific ioctl call will be made to enable reception of that
 * multicast address.
 * This is achieved by using the driver ioctl command with the
 * TM_DEV_SET_MCAST_LIST flag, and passing a pointer
 * to a list of ethernet multicast addresses, and the number of those
 * addresses. We keep track of the list of ethernet multicast addresses, so
 * that the driver does not have to. Then an IGMP report is sent.
 * If TM_USE_IGMPV3 is defined, the user can use additional APIs andi
 * setsockopt.
 * 
 * 4. TRECK STACK INITIALIZATION OF THE IGMP PROTOCOL:
 * The IGMP protocol initialization routine, tfIgmpInit() is called from
 * tfInit().
 * tfIgmpDevInit() called from each configured interface.
 * When a new interface is configured, the 224.0.0.1 host group will be
 * joined for that interface. If RIP is enabled, then make also that
 * interface join the 224.0.0.2 host group.
 * 
 * Limitations
 * This implementation is for IP hosts only. It does not include multicast
 * routing.
 * No loop back of multicast packet is allowed.
 *
 */
/*
 * local macros
 */
/* IGMP V1 response time (10 seconds) */
#define TM_IGMP_V1_RESP_TIME        10
/* IGMP entry is in the non member state (igmpState) */
#define TM_IGMP_NON_MEMBER          0
/* IGMP entry corresponds to the local all hosts group (cannot send report) */
#define TM_IGMP_LOCAL               1
/* IGMP entry is in the process of sending a join (igmpState) */
#define TM_IGMP_JOIN_PENDING        2
/* IGMP entry is in the idle member state (igmpState) */
#define TM_IGMP_IDLE_MEMBER         3
/* IGMP entry is in the delaying member state (igmpState) */
#define TM_IGMP_DELAYING_MEMBER     4
#define TM_IGMP_QUERY_DELAYING_MEMBER     4
/* IGMP entry saw another host sending a report (igmpFlag) */
#define TM_IGMP_OTHER_GROUP_REPORT  0
/* IGMP entry was last group to report (igmpFlag) */
#define TM_IGMP_LAST_GROUP_REPORT   1
/* No IGMPv1/IGMPv2 router present on that interface (default) */
/* #define TM_IGMP_DEV_DEFAULT         0 */
/* IGMP v1 router present */
#define TM_IGMP_DEV_V1              (1 << 1)
/* IGMP minimum length, in bytes */
#define TM_IGMP_MIN_LEN             8
#define TM_IP_OVERHEAD                                                  \
    (TM_4_IP_MIN_HDR_LEN+TM_IP_ROUTER_ALERT_LEN)
#ifdef TM_USE_IGMPV3
#define TM_IP_DEF_IGMPV3_TOS        0xc0 /* Per RFC3376 */
/* IGMP v2 router present */
#define TM_IGMP_DEV_V2              (1 << 2) 
#define TM_IGMP_V3_MIN_LEN          12
#define TM_IGMP_V3_REP_LEN          8
#define TM_IGMP_V3_GRP_LEN          8
/* Header overhead for one group */
#define TM_IGMP_V3_REP_ONEGR_OVERHEAD                                   \
    (TM_IGMP_V3_REP_LEN + TM_IGMP_V3_GRP_LEN)
#define TM_IP_MCAST_V3_REPORTS_DEST tm_const_htonl(TM_UL(0xE0000016))
#define TM_ONE_ADDR_LEN             4
#define tm_source_len(numSrc)       (numSrc * TM_ONE_ADDR_LEN) 
#define tm_num_src(len, overHead) \
        (tt16Bit)((len - overHead) / TM_ONE_ADDR_LEN)
#endif /* TM_USE_IGMPV3 */
/* IGMP header types */
#define TM_IGMP_QUERY               0x11
#define TM_IGMP_V2_REPORT           0x16
#define TM_IGMP_LEAVE               0x17
#define TM_IGMP_V1_REPORT           0x12
#ifdef TM_USE_IGMPV3
#define TM_IGMP_V3_REPORT           0x22
#endif /* TM_USE_IGMPV3 */

#ifdef TM_USE_IGMPV3
/* Indicate an invalid mode, i.e. source list is invalid */
#define TM_IGMP_INVALID_MODE          0xFFFF

/*
 * listType values (listType is a parameter passed to the routines used to add
 * or drop sources.
 */
#define TM_IGMP_IGMP_LIST             0
#define TM_IGMP_SOCKET_LIST           1
#define TM_IGMP_NETWORK_LIST          2
/*
 * IGMP header group record type (igmpgRecordType)
 */
/* 
 * "Current-State Record" sent by a system in response to a Query: 
 * MODE_IS_INCLUDE - indicates that the interface has a filter mode of 
 *                   include for the specified group address.
 * MODE_IS_EXCLUDE - indicates that the interface has a filter mode of
 *                   exclude for the specified group address.
 */
#define TM_MODE_IS_INCLUDE          1
/*#define TM_MODE_IS_EXCLUDE          2 (defined in trmacro.h) */
/* 
 * "Filter-Mode-Change Record" sent by a system whenever an interface-level
 * IGMP entry filter mode changes (from INCLUDE to EXCLUDE, or vice versa)
 * as a result of a user API invocation.
 * CHANGE_TO_INCLUDE_MODE - indicates that the interface has changed to
 *                          INCLUDE filter mode for the specified multicast
 *                          address.
 * CHANGE_TO_EXCLUDE_MODE - indicates that the interface has changed to
 *                          EXCLUDE filter mode for the specified
 *                          multicast address.
 */
#define TM_CHANGE_TO_INCLUDE_MODE   3
#define TM_CHANGE_TO_EXCLUDE_MODE   4
/*
 * "Source-List-Change Record" sent by a system whenever an interface-level
 * IGMP entry source list changes and is NOT coincident with a change of
 * filter mode as a result of a user API invocation.
 * ALLOW_NEW_SOURCES - interface wants to hear from sources in the list for
 *                     the specified multicast address.
 * BLOCK_OLD_SOURCES - interface does not want to hear from sources in the list
 *                     for the specified multicast address.
 */
#define TM_ALLOW_NEW_SOURCES        5
#define TM_BLOCK_OLD_SOURCES        6
#endif /* TM_USE_IGMPV3 */

/* IP router alert option length (RFC 2113) */
#define TM_IP_ROUTER_ALERT_LEN      4
#define TM_PAK_IP_ROUTER_ALERT_LEN  tm_packed_byte_count(TM_IP_ROUTER_ALERT_LEN)

/* IP router alert option (RFC 2113) */
#define TM_IP_ROUTER_ALERT_WORD     tm_const_htonl(TM_UL(0x94040000))
#define TM_IP_OPT_RA                (tt8Bit)0x14 /* masked off with 0x1F */
#define TM_IP_OPT_RA_LEN            (tt8Bit)TM_IP_ROUTER_ALERT_LEN

#ifndef TM_USE_IGMPV3
#ifndef TM_IGMP_SOCKET_BLOCKING_FACTOR
/* blocking factor used to cache ttSockIgmpInfo on the socket */
#define TM_IGMP_SOCKET_BLOCKING_FACTOR 4
#endif /* TM_IGMP_SOCKET_BLOCKING_FACTOR */
#endif /* TM_USE_IGMPV3 */

#ifdef TM_USE_IGMPV3

#ifndef TM_IGMP_SOURCE_BLOCKING_FACTOR
/* blocking factor used to cache sources in a source list */
#define TM_IGMP_SOURCE_BLOCKING_FACTOR 4
#endif /* TM_IGMP_SOURCE_BLOCKING_FACTOR */

/*
 * User has issued an add source command
 * Has to be zero, because it is the default value used by
 * setipv4sourcefilter()
 */
#define TM_IGMP_CMND_ADD_SRC     0
/* User has issued a drop source command */
#define TM_IGMP_CMND_DROP_SRC    1

#endif /* TM_USE_IGMPV3 */

#ifdef TM_SINGLE_INTERFACE_HOME
/* ipAddr is directed broadcast for device */
#define tm_ip_d_broad_match(devPtr, ipAddr)                         \
        (    tm_4_ll_is_broadcast(devPtr)                           \
          && tm_ip_match(ipAddr, devPtr->devDBroadIpAddr) )
#endif /* TM_SINGLE_INTERFACE_HOME */

#ifdef TM_USE_IGMPV3
/* Macros to extract IGMPV3 query header fields */
/* Per RFC3376 lowest 4 bits */
#define tm_igmp_mant(x)     (tt8Bit)((x) & 0x0F)
/* Bits 5,6, and 7 */
#define tm_igmp_exp(x)      (tt8Bit)(((x) >> 4) & 0x07)
/* bit 4: unused on a host */
/*#define tm_igmp_sflag(x)    (((x) >> 3) & 0x01) */
/* Lowest 3 bits */
#define tm_igmp_qrv(x)      (tt8Bit)((x) & 0x07)
#endif /* TM_USE_IGMPV3 */

/*
 * Local types.
 */

/* User ip_mreq structure (defined in trsocket.h) pointer */
typedef struct ip_mreq        TM_FAR * ttIpMreqPtr;
/* User ip_mreq_source structure (defined in trsocket.h) pointer */
typedef struct ip_mreq_source TM_FAR * ttIpMreqSourcePtr;

#ifdef TM_USE_IGMPV3
/*
 * Structure used to convey parameters in call back routine to find a match in
 * the socket mcast cache.
 */
typedef struct tsIgmpMatch
{
    ttDeviceEntryPtr        iiDevicePtr;
    tt4IpAddress            iiMcastAddr;
} ttIgmpMatch;
typedef ttIgmpMatch TM_FAR * ttIgmpMatchPtr;
#endif /* TM_USE_IGMPV3 */

/* START of IGMP Network headers */

#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#ifndef TM_32BIT_DSP_BIG_ENDIAN
/*
 * Common IGMP header used for reception, and by common send routine
 * tfIgmpSendPacket()
 */
typedef struct tsIgmpHeader
{
/* Igmp Checksum */
    unsigned int                igmphChecksum    : 16;
/* Max response time in 10th of a second (0 for V1, which means 10) */
    unsigned int                igmphMaxRespTime : 8;
/*
 * IGMP type
 * (TM_IGMP_QUERY, TM_IGMP_V2_REPORT TM_IGMP_LEAVE, TM_IGMP_V1_REPORT )
 */
    unsigned int                igmphType        : 8;
/*
 * multicast group address (used for report, leave, or group specific query)
 */
    tt4IpAddress                igmphGroupAddress;
#ifdef TM_USE_IGMPV3
    unsigned int                igmphNumSrc      : 16;
/* QQIC field */
    unsigned int                igmphQqic        : 8;
/* QRV + S bit field */
    unsigned int                igmphQrv         : 8;
/* variable number of addresses */
    tt4IpAddress                igmphSrcAddress[1];
#endif /* TM_USE_IGMPV3 */
} ttIgmpHeader;
#ifdef TM_USE_IGMPV3
/* One group record used in IGMPv3 report header */
typedef struct tsIgmpGroupRecord
{
/* Number of sources (variable array below) */
    unsigned int                igmpgNumSrc      : 16;
/* Length of auxiliary data in 32-bit words. Must be zero in IGMPv3 */
    unsigned int                igmpgAuxDataLen  : 8;
/* Record type. INCLUDE, EXCLUDE, etc.. */
    unsigned int                igmpgRecordType  : 8;
/* Group address for this record */
    tt4IpAddress                igmpgGroupAddress;
/* variable number of addresses */
    tt4IpAddress                igmpgSrcAddress[1];
/*
 * Auxiliary data in multiple of 32-bit words follow the list of sources.
 * Must be null in IGMPv3
 */
} ttIgmpGroupRecord;

/* IGMPv3 report header */
typedef struct tsIgmpV3Report
{
/* Igmp Checksum */
    unsigned int                igmphChecksum    : 16;
/* Reserved field */
    unsigned int                igmph1Reserved   : 8;
/*
 * IGMP type (TM_IGMP_V3_REPORT (== 0x22))
 */
    unsigned int                igmphType        : 8;
/*
 * Number of groups (not set as variable array below because igmphGroupRecord
 * is of variable length),
 */
    unsigned int                igmphNumGroup    : 16;
/* Reserved field */
    unsigned int                igmph2Reserved   : 16;
/*
 * variable number of group records.
 * First group record. Not set as an array because igmphGroupRecord is of
 * variable length.
 */
    ttIgmpGroupRecord           igmphGroupRecord;
} ttIgmpV3Report;
#endif /* TM_USE_IGMPV3 */
#else /* TM_32BIT_DSP_BIG_ENDIAN */
/*
 * Common IGMP header used for reception, and by common send routine
 * tfIgmpSendPacket()
 */
typedef struct tsIgmpHeader
{
/*
 * IGMP type
 * (TM_IGMP_QUERY, TM_IGMP_V2_REPORT TM_IGMP_LEAVE, TM_IGMP_V1_REPORT )
 */
    unsigned int                igmphType        : 8;
/* Max response time in 10th of a second (0 for V1, which means 10) */
    unsigned int                igmphMaxRespTime : 8;
/* Igmp Checksum */
    unsigned int                igmphChecksum    : 16;
/*
 * multicast group address (used for report, leave, or group specific query)
 */
    tt4IpAddress                igmphGroupAddress;
#ifdef TM_USE_IGMPV3
/* QRV + S bit field */
    unsigned int                igmphQrv         : 8;
/* QQIC field */
    unsigned int                igmphQqic        : 8;
    unsigned int                igmphNumSrc      : 16;
/* variable number of addresses */
    tt4IpAddress                igmphSrcAddress[1];
#endif /* TM_USE_IGMPV3 */
} ttIgmpHeader;
#ifdef TM_USE_IGMPV3
/* One group record used in IGMPv3 report header */
typedef struct tsIgmpGroupRecord
{
/* Record type. INCLUDE, EXCLUDE, etc.. */
    unsigned int                igmpgRecordType  : 8;
/* Length of auxiliary data in 32-bit words. Must be zero in IGMPv3 */
    unsigned int                igmpgAuxDataLen  : 8;
/* Number of sources (variable array below) */
    unsigned int                igmpgNumSrc      : 16;
/* Group address for this record */
    tt4IpAddress                igmpgGroupAddress;
/* variable number of addresses */
    tt4IpAddress                igmpgSrcAddress[1];
/*
 * Auxiliary data in multiple of 32-bit words follow the list of sources.
 * Must be null in IGMPv3
 */
} ttIgmpGroupRecord;

/* IGMPv3 report header */
typedef struct tsIgmpV3Report
{
/*
 * IGMP type (TM_IGMP_V3_REPORT (== 0x22))
 */
    unsigned int                igmphType        : 8;
/* Reserved field */
    unsigned int                igmph1Reserved   : 8;
/* Igmp Checksum */
    unsigned int                igmphChecksum    : 16;
/* Reserved field */
    unsigned int                igmph2Reserved   : 16;
/*
 * Number of groups (not set as variable array below because igmphGroupRecord
 * is of variable length),
 */
    unsigned int                igmphNumGroup    : 16;
/*
 * First group record. Not set as an array because igmphGroupRecord is of
 * variable length.
 */
    ttIgmpGroupRecord           igmphGroupRecord;
} ttIgmpV3Report;
#endif /* TM_USE_IGMPV3 */
#endif /* TM_32BIT_DSP_BIG_ENDIAN */
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
/*
 * Common IGMP header used for reception, and by common send routine
 * tfIgmpSendPacket()
 */
typedef struct tsIgmpHeader
{
/*
 * IGMP type
 * (TM_IGMP_QUERY, TM_IGMP_V2_REPORT TM_IGMP_LEAVE, TM_IGMP_V1_REPORT )
 */
    unsigned int                igmphType        : 8;
/* Max response time in 10th of a second (0 for V1, which means 10) */
    unsigned int                igmphMaxRespTime : 8;
/* Igmp Checksum */
    tt16Bit                     igmphChecksum;
/*
 * multicast group address (used for report, leave, or group specific query)
 */
    tt4IpAddress                igmphGroupAddress;
#ifdef TM_USE_IGMPV3
/* QRV + S bit field */
    unsigned int                igmphQrv         : 8;
/* QQIC field */
    unsigned int                igmphQqic        : 8;
    unsigned int                igmphNumSrc      : 16;
/* variable number of addresses */
    tt4IpAddress                igmphSrcAddress[1];
#endif /* TM_USE_IGMPV3 */
} ttIgmpHeader;

#ifdef TM_USE_IGMPV3
/* One group record used in IGMPv3 report header */
typedef struct tsIgmpGroupRecord
{
/* Record type. INCLUDE, EXCLUDE, etc.. */
    unsigned int                igmpgRecordType  : 8;
/* Length of auxiliary data in 32-bit words. Must be zero in IGMPv3 */
    unsigned int                igmpgAuxDataLen  : 8;
/* Number of sources (variable array below) */
    unsigned int                igmpgNumSrc      : 16;
/* Group address for this record */
    tt4IpAddress                igmpgGroupAddress;
/* variable number of addresses */
    tt4IpAddress                igmpgSrcAddress[1];
/*
 * Auxiliary data in multiple of 32-bit words follow the list of sources.
 * Must be null in IGMPv3
 */
} ttIgmpGroupRecord;

/* IGMPv3 report header */
typedef struct tsIgmpV3Report
{
/*
 * IGMP type (TM_IGMP_V3_REPORT (== 0x22))
 */
    unsigned int                igmphType        : 8;
/* Reserved field */
    unsigned int                igmph1Reserved   : 8;
/* Igmp Checksum */
    unsigned int                igmphChecksum    : 16;
/* Reserved field */
    unsigned int                igmph2Reserved   : 16;
/*
 * Number of groups (not set as variable array below because igmphGroupRecord
 * is of variable length),
 */
    unsigned int                igmphNumGroup    : 16;
/*
 * First group record. Not set as an array because igmphGroupRecord is of
 * variable length.
 */
    ttIgmpGroupRecord           igmphGroupRecord;
} ttIgmpV3Report;
#endif /* TM_USE_IGMPV3 */
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
/*
 * Common IGMP header used for reception, and by common send routine
 * tfIgmpSendPacket()
 */
typedef struct tsIgmpHeader
{
/*
 * IGMP type
 * (TM_IGMP_QUERY, TM_IGMP_V2_REPORT TM_IGMP_LEAVE, TM_IGMP_V1_REPORT )
 */
    tt8Bit                      igmphType;
/* Max response time in 10th of a second (0 for V1, which means 10) */
    tt8Bit                      igmphMaxRespTime;
/* Igmp Checksum */
    tt16Bit                     igmphChecksum;
/*
 * multicast group address (used for v2/v1 report, leave, or
 * group specific query)
 */
    tt4IpAddress                igmphGroupAddress;
#ifdef TM_USE_IGMPV3
/* QRV + S bit field */
    tt8Bit                      igmphQrv;
/* QQIC field */
    tt8Bit                      igmphQqic;
/* Number of sources */
    tt16Bit                     igmphNumSrc;
/* variable number of addresses */
    tt4IpAddress                igmphSrcAddress[1];
#endif /* TM_USE_IGMPV3 */
} ttIgmpHeader;

#ifdef TM_USE_IGMPV3
/* One group record used in IGMPv3 report header */
typedef struct tsIgmpGroupRecord
{
/* Record type. INCLUDE, EXCLUDE, etc.. */
    tt8Bit                      igmpgRecordType;
/* Length of auxiliary data in 32-bit words. Must be zero in IGMPv3 */
    tt8Bit                      igmpgAuxDataLen;
/* Number of sources (variable array below) */
    tt16Bit                     igmpgNumSrc;
/* Group address for this record */
    tt4IpAddress                igmpgGroupAddress;
/* variable number of addresses */
    tt4IpAddress                igmpgSrcAddress[1];
/*
 * Auxiliary data in multiple of 32-bit words follow the list of sources.
 * Must be null in IGMPv3
 */
} ttIgmpGroupRecord;

/* IGMP v3 report (0x22) header */
typedef struct tsIgmpV3Report
{
/*
 * IGMP type (TM_IGMP_V3_REPORT (== 0x22))
 */
    tt8Bit                      igmphType;
/* Reserved field */
    tt8Bit                      igmph1Reserved;
/* Igmp Checksum */
    tt16Bit                     igmphChecksum;
/* Reserved field */
    tt16Bit                     igmph2Reserved;
/*
 * Number of groups (not set as variable array below because igmphGroupRecord
 * is of variable length),
 */
    tt16Bit                     igmphNumGroup;
/*
 * First group record. Not set as an array because igmphGroupRecord is of
 * variable length.
 */
    ttIgmpGroupRecord           igmphGroupRecord;
} ttIgmpV3Report;

#endif /* TM_USE_IGMPV3 */

#endif /* !TM_DSP */

#ifdef TM_USE_IGMPV3
typedef ttIgmpGroupRecord TM_FAR * ttIgmpGroupRecordPtr;
typedef ttIgmpV3Report TM_FAR * ttIgmpV3ReportPtr;
#endif /* TM_USE_IGMPV3 */

typedef ttIgmpHeader TM_FAR * ttIgmpHeaderPtr;

/* END of IGMP Network headers */

/*
 * IGMPV1 or IGMPV2 entry timer for a multicast host group that the user has
 * joined on an interface. Used to retransmit the membership report, and
 * also used to respond to IGMPv1 or IGMPv2 queries.
 */
#define TM_IGMP_TIMER          0
#ifdef TM_USE_IGMPV3
/*
 * IGMPv3 query timer (to delay sending a current state report in response to
 * a query)
 */
#define TM_IGMP_QUERY_TIMER    1
/* IGMP source timer (to retransmit an IGMPv3 state change report) */
#define TM_IGMP_SRC_TIMER      2
/* Maximum number of timers */
#define TM_IGMP_MAX_TIMERS     3
#else /* TM_USE_IGMPV3 */
/* Maximum number of timers */
#define TM_IGMP_MAX_TIMERS     1
#endif /* TM_USE_IGMPV3 */
/*
 * IGMP entry conveying multicast host group, and source lists
 */
typedef struct tsIgmpEntry
{
/* Next IGMP entry in the list for this device */
    struct tsIgmpEntry TM_FAR * igmpNextPtr;
/* Previous IGMP entry in the list for this device */
    struct tsIgmpEntry TM_FAR * igmpPrevPtr;
/* Device associated with this IGMP entry */
    ttDeviceEntryPtr            igmpDevPtr;
/* Multicast host group joined */
    tt4IpAddress                igmpHostGroup;
/* Timers
 * 0. (IGMPv1/IGMPv2 timer active in delaying member state)
 * 1. (for delayed response to IGMPV3 query)
 * 2. (for retransmission of IGMPV3 state change)
 */
    ttTimerPtr                  igmpTimerPtr[TM_IGMP_MAX_TIMERS];
#ifdef TM_USE_IGMPV3
/* 
 * 8 source lists:
 * [0] IGMP entry current list based on sockets exclude [1], and include [2]
 *     lists.
 * [1] sockets include source list (TM_MODE_IS_INCLUDE), index has to be 1
 * [2] sockets exclude source list (TM_MODE_IS_EXCLUDE), index has to be 2
 * [3] pending TO_IN(x) source list index has to be 3
 * [4] pending TO_EX(x) source list index has to be 4
 * [5] pending ALLOW(x) new sources source list index has to be 5
 * [6] pending BLOCK(x) old sources source list index has to be 6
 * [7] recorded list based on current list and source group specific queries
 * End of 8 source lists.
 */
#define TM_IGMP_CURRENT_LIST          0
/*#define TM_MODE_IS_INCLUDE          1 */
/*#define TM_MODE_IS_EXCLUDE          2 */
/*#define TM_CHANGE_TO_INCLUDE_MODE   3 */
/*#define TM_CHANGE_TO_EXCLUDE_MODE   4 */
/*#define TM_ALLOW_NEW_SOURCES        5 */
/*#define TM_BLOCK_OLD_SOURCES        6 */
/* has to be contiguous with TM_BLOCK_OLD_SOURCES */
#define TM_IGMP_RECORDED_LIST         7
/*
 * Maximum number of state change reports: TO_IN(), TO_EX(), ALLOW(), BLOCK(),
 * (index 3- index 6).
 */
#define TM_IGMP_MAX_STATE_CHANGE_REPORTS 4
#define TM_IGMP_MAX_FILTERMODE_REPORTS   2 /* TO_IN(), TO_EX() */
#define TM_IGMP_MAX_SRCLIST_REPORTS      2 /* ALLOW(), BLOCK() */
/*
 * Maximum number of pending responses: TO_IN(), TO_EX(), ALLOW(), BLOCK(),
 * Recorded list (index 3- index 7).
 */
#define TM_IGMP_MAX_PENDING_RESPONSES 5
/* 8 source lists */
#define TM_IGMP_MAX_SRC_LIST (TM_IGMP_RECORDED_LIST + 1)
    ttSourceList                igmpSrcList[TM_IGMP_MAX_SRC_LIST];
/*
 * Number of sockets/entities which have joined in exclude mode with no source
 * blocked
 */
    tt16Bit                     igmpCurNumEmptyExcl;
#endif /* TM_USE_IGMPV3 */
/* Mhome index corresponding to interface IP address configured by the user */
    tt16Bit                     igmpMhomeIndex;
/* How many sockets/entities have joined this group */
    tt16Bit                     igmpOwnerCount;
/* non member, local member, joining member, idle member, or delaying member */
    tt8Bit                      igmpState;
/* Flag to indicate that we were the last host to send a report */
    tt8Bit                      igmpFlag;
#ifdef TM_USE_IGMPV3
/* Number of times we transmit the IGMP state change report */
    tt8Bit                      igmpSrcRobustness;
#endif /* TM_USE_IGMPV3 */
/* Flag to indicate that the kernel joined this group */
    tt8Bit                      igmpKernelJoin;
} ttIgmpEntry;

typedef ttIgmpEntry TM_FAR * ttIgmpEntryPtr;


/*
 * Local IGMP variables
 */

/*
 * Local functions:
 */
/*
 * tfIgmpTimer is called by the timer execute thread to re-send a
 * V1/V2 report, in case the first one is lost, or or to send a delayed
 * V1/V2 response to a query.
 */
void tfIgmpTimer (ttVoidPtr      timerBlockPtr,
                  ttGenericUnion userParm1,
                  ttGenericUnion userParm2);

/*
 * tfIgmpVTimer. Timer added when any lower version (v1, or v2) query is
 * received on the interface to keep track of a lower version router.
 * When this timer expires, we are allowed to send higher version reports.
 */
TM_NEARCALL void tfIgmpVTimer (ttVoidPtr      timerBlockPtr,
                   ttGenericUnion userParm1,
                   ttGenericUnion userParm2);

/*
 * tfIgmpJoinHostGroup requests that this hosts become a member
 * of the host group identified by groupAddress on the given network
 * interface.
 */
static int tfIgmpJoinHostGroup(
    tt8Bit           socketCall,
    ttIpMreqSrcPtr   imrIntPtr);

/*
 * tfIgmpLeaveHostGroup requests that this hosts give up its
 * membership in the host group identified by groupAddress on the given
 * network interface.
 */
static int tfIgmpLeaveHostGroup(
    tt8Bit           socketCall,
    ttIpMreqSrcPtr   imrIntPtr
#ifdef TM_USE_IGMPV3
  , ttSourceListPtr oldSrcListPtr
#endif /* TM_USE_IGMPV3 */
    );

/*
 * tfIgmpFindEntry called to retrieve an IGMP entry on an interface for the
 * given mcast group.
 */
static ttIgmpEntryPtr tfIgmpFindEntry(tt4IpAddress     mcastGroup,
                                      ttDeviceEntryPtr devEntryPtr);

/*
 * Allocate a new IGMP entry, telling the device driver about the multicast
 * addition.
 */
static ttIgmpEntryPtr tfIgmpAddEntry (
    ttIpMreqSrcPtr  imrIntPtr,
    ttIntPtr         errorCodePtr);

/*
 * Decrease an IGMP entry owner count, removing it from the corresponding
 * device cache if the owner count reaches 0.
 */
static int tfIgmpEntryUnget(ttIgmpEntryPtr igmpEntryPtr);
/* Free an IGMP entry */
static void tfIgmpEntryFree(ttIgmpEntryPtr igmpEntryPtr);

/* Send a v1, or v2 report, or a v2 leave */
static int tfIgmpV1V2Send(ttIgmpEntryPtr igmpEntryPtr, int type);

/* common code for v1/v2/v3 to send a packet on the wire */
static int tfIgmpSendPacket(
    ttDeviceEntryPtr devEntryPtr,
    ttPacketPtr      packetPtr,
    int              type);

/* Add a version timer to keep track of routers' versions. */
static void tfIgmpAddVTimer(ttDeviceEntryPtr devEntryPtr,
#ifdef TM_USE_IGMPV3
                            tt32Bit          queryResponseInterval,
#endif /* TM_USE_IGMPV3 */
                            int              version);

/* Compute a random 32-bit value between 0, and maxTimeoutInMilliSecs */
static tt32Bit tfIgmpRandom(tt32Bit maxTimeoutInMilliSecs);

/*
 * Add or refresh a timer with a random timeout vaue between 0, and
 * maxResponseTime.
 */
static void tfIgmpAddRandomTimer(
    ttTimerPtrPtr  timerPtrPtr,
    ttTmCBFuncPtr  callBackFunctionPtr,
    ttGenericUnion timerParm1,
    tt32Bit        maxResponseTime);

/* Remove timer at given index */
static void tfIgmpTimerRemove(
    ttIgmpEntryPtr igmpEntryPtr,
    int timerIndex);

#ifdef TM_USE_IGMPV3
/* Add an IGMPv3 source timer for retransmission of state change reports */
static void tfIgmpSrcTimer(
    ttVoidPtr      timerBlockPtr,
    ttGenericUnion userParm1,
    ttGenericUnion userParm2);
/* Remove the IGMPv3 source timer */
static void tfIgmpSrcTimerRemove(
    ttIgmpEntryPtr igmpEntryPtr);
/*
 * Done with the source timer. Cleanup, and check if we need to schedule
 * ALLOW and/or BLOCK reports
 */
static void tfIgmpSrcTimerDoneOrNewReports (
    ttIgmpEntryPtr igmpEntryPtr,
    int index);
/* Remove the IGMPv3 general query timer */
static void tfIgmpDevQueryTimerRemove(
    ttDeviceEntryPtr devEntryPtr);
/*
 * IGMPv3 general query timer (to send a delayed response to an IGMPv3 general
 * query)
 */
static void tfIgmpGeneralQueryTimer(
    ttVoidPtr      timerBlockPtr,
    ttGenericUnion userParm1,
    ttGenericUnion userParm2);
/*
 * Check whether we have anything to send as a response to an IGMPv3 general
 * query.
 */
static tt8Bit tfIgmpSomethingToReport(ttDeviceEntryPtr devEntryPtr);
/*
 * IGMPv3 group query timer (to send a delayed response to an IGMPv3 group
 * query)
 */
static void tfIgmpGroupQueryTimer(
    ttVoidPtr      timerBlockPtr,
    ttGenericUnion userParm1,
    ttGenericUnion userParm2);
/*
 * Send the current state report using either the current source list, or
 * the queried recorded source list.
 */
static int tfIgmpSendCurrentStateReport(
    ttIgmpEntryPtr  igmpEntryPtr, 
    ttSourceListPtr srcListPtr,
    ttPacketPtrPtr  packetPtrPtr,
    tt8BitPtr       unlockedPtr);
#endif /* TM_USE_IGMPV3 */

/* 
 * tfIgmpEntryJoinHostGroup called when the user joins a new host group and
 * when we send V1, or V2 only reports.
 */
static int tfIgmpEntryJoinHostGroup(
    ttIgmpEntryPtr   igmpEntryPtr);

/* 
 * tfIgmpEntryLeaveHostGroup called when the user leaves a host group and
 * when we send V1, or V2 only reports.
 */
static int tfIgmpEntryLeaveHostGroup(
    ttIgmpEntryPtr   igmpEntryPtr);

/* Map membership setsockopt interface address to a device */
static int tfIgmpSetSockInterface(
    ttIpMreqSrcPtr   imrIntPtr);

/* Get socket mcast cache entry given a multicast group/device/socket. */
static ttSockIgmpInfoPtr
tfIgmpSocketCacheGetParams(
    ttSocketEntryPtr socketEntryPtr,
    ttDeviceEntryPtr devEntryPtr,
    tt4IpAddress     groupAddress);

#ifdef TM_USE_IGMPV3
/* IGMP setsockopt() SOURCE commands */
static int tfIgmpSocketSourceCommand(
    ttSockIgmpInfoPtr sockIgmpInfoPtr,
    ttIpMreqSrcPtr     imrIntPtr);
#ifdef TM_IP_SRC_ERROR_CHECKING
/* (optional): check for invalid source address. */
static int tfIgmpCheckValidSrcAddr(ttIpMreqSrcPtr imrIntPtr);
#endif /* TM_IP_SRC_ERROR_CHECKING */
#endif /* TM_USE_IGMPV3 */
/* Cache multicast group join on socket */
static int tfIgmpSocketCacheNewInsert(
    ttIpMreqSrcPtr imrIntPtr);

/* Get and remove multicast group join cached on socket */
static void tfIgmpSocketCacheGetRemove(ttIpMreqSrcPtr imrIntPtr);

/* Remove passed cache entry from socket mcast cache */
static void tfIgmpSocketCacheRemove(
    ttSockIgmpInfoPtr   sockIgmpInfoPtr,
    ttSocketEntryPtr    socketEntryPtr);

#ifdef TM_USE_IGMPV3

/*
 * List walk call back function per socket mcast cache entry when we want to
 * purge the socket mcast cache
 */
static int tfIgmpSocketCacheEntryPurge(ttNodePtr nodePtr,
                                       ttGenericUnion genParm1);
/* Free a socket multicast cache entry, and its associated source list.  */
static void tfIgmpSocketCacheFree(
    ttSockIgmpInfoPtr sockIgmpInfoPtr);
/*
 * List walk call back function when we want to find a match in the socket
 * mcast cache
 */
static int tfIgmpSocketCacheEntryMatch(ttNodePtr nodePtr,
                                       ttGenericUnion genParm1);

/* Is source allowed per passed source list */
static tt8Bit tfIgmpCacheSrcAllowed(ttSourceListPtr srcListPtr,
                                    tt4IpAddress    srcAddress);
/* Get index of source in source list */
static int tfIgmpCacheGetSource (
    ttSourceListPtr     srcListPtr,
    ttIntPtr            searchIndexPtr,
    tt4IpAddress        srcIpAddr);
/* uncache the source addresses from this mcast cache on the socket */
static int tfIgmpSocketCacheDropSource (
    ttSockIgmpInfoPtr       sockIgmpInfoPtr,
    ttIpMreqSrcPtr          imrIntPtr);
/* uncache these source addresses from this source list */
static int tfIgmpCacheDropSource(
    ttSourceListPtr         srcListPtr,
    struct in_addr TM_FAR * srcAddrPtr,
    int                     numSrc,
    tt8Bit                  listType,
    tt8Bit                  sorted);
/* Add source addresses to this source list */
static int tfIgmpCacheAddSource(
    ttSourceListPtr         srcListPtr,
    ttIntPtr                addIndexPtr,
    struct in_addr TM_FAR * srcAddrPtr,
    int                     numSrc,
    tt8Bit                  listType,
    tt8Bit                  sorted);
/* Add one source address in sorted source list */
static int tfIgmpCacheSortedInsert(
    ttSourceListPtr         srcListPtr,
    ttIntPtr                searchIndexPtr,
    struct in_addr          srcAddr,
    tt8Bit                  listType);

/*
 * Tell IGMP of source list changes, possibly sending an IGMPV3 state change
 * report if allowed and needed.
 */
static int tfIgmpEntryChangeSource(
    ttIgmpEntryPtr  igmpEntryPtr,
    ttIpMreqSrcPtr  imrIntPtr,
    ttSourceListPtr oldSrcListPtr,
    int             deltaEmptyExclJoin,
    tt8Bit          ignoreError);

/* Send an IGMPv3 state change report */
static void tfIgmpSendStateChangeReport (ttIgmpEntryPtr igmpEntryPtr);

/* 
 * Allocate space for sources and owner counts, and copy sources and owner
 * counts into newly allocated buffer.
 */
static int tfIgmpAllocateCopySources(
    ttSourceListPtr srcListPtr,
    int             allocCount,
    tt8Bit          realloc);
/* Duplicate one source list (using new allocation) */
static int tfIgmpCopySourceList(
    ttSourceListPtr oldSrcListPtr,
    ttSourceListPtr newSrcListPtr);
/* Free a source list */
static void tfIgmpFreeSourceList(ttSourceListPtr srcListPtr);
/*
 * Build an IGMP entry current src list based on sockets include/exclude
 * lists, and compare previous current src list, and new current list to build
 * the IGMPv3 reports.
 */
static int tfIgmpBuildCurSrcListNReports(ttIgmpEntryPtr igmpEntryPtr, tt8BitPtr changePtr);
/* Cancel IGMPv3 pending reports, and retransmission timers on the device */
static void tfIgmpCancelDevPendingReports(ttDeviceEntryPtr devEntryPtr);
/* Cancel IGMPv3 pending reports for one mulitcast group */
static void tfIgmpCancelPendingReports(
    ttIgmpEntryPtr  igmpEntryPtr,
    int             reportOffset,
    int             numReports);
/* Build a recorded list using queried sources and current list */
static void tfIgmpBuildRecordedList(
    ttIgmpEntryPtr          igmpEntryPtr,
    tt4IpAddressPtr         srcAddrPtr,
    tt16Bit                 numSrc);
/*  
 * Build a new source list, using all sources from the A source list, that are
 * not included in the B source list (A-B). 
 */
static int tfIgmpDiffSourceList(
    ttSourceListPtr  newSrcListPtr,
    ttSourceListPtr  aSrcListPtr,
    ttSourceListPtr  bSrcListPtr);
#endif /* TM_USE_IGMPV3 */

/*
 * tfEtherMcast adds (command == TM_LNK_ADD_MCAST) or deletes (command ==
 * TM_LNK_DEL_MCAST) a group address to a list of device enabled multicast
 * addresses. This function is called with the device locked. Called from IGMP
 * (IPv4).
 *
 * 1.   If groupAddr is zero, this indicates that this is general command
 *      (i.e. we want to delete or enable all addresses).
 * 2.   If it's not zero, call tfIpBroadMcastToLan to convert the IP
 *      address into an Ethernet multicast address.  TM_ETHER_MULTI will be
 *      returned if the address was converted successfully.
 * 3.   If no error occurred, call tfEtherSetMcastAddr to add/remove the given
 *      Ethernet address on the device.
 *      
 * Parameters
 * Parameter          Description
 * interfaceHandle    Pointer to interface to which we want to add the host
 *                    group address membership.
 * groupAddr          Multicast host group address
 * command            One of TM_LNK_ADD_MCAST, or TM_LNK_DEL_MCAST
 *
 * Returns
 * Value              Meaning
 * TM_ENOERROR        Success
 * TM_ENOBUFS         Not enough memory to execute the command
 * TM_EADDRNOTAVAIL   Multicast bit not enabled, or address is not multicast IP
 */   
int tfEtherMcast ( ttVoidPtr     interfaceHandle,
                   tt4IpAddress  groupAddress,
                   int           command )
{
    ttDeviceEntryPtr   devEntryPtr;
    int                multiCast;
    int                errorCode;
    ttEnetAddressUnion enetAddr;
    tt8Bit             specificCmd;

    devEntryPtr = (ttDeviceEntryPtr) interfaceHandle;

/*
 * 1.   If groupAddr is zero, this indicates that this is general command
 *      (i.e. we want to delete or enable all addresses).
 */     
    if ( tm_ip_zero(groupAddress) )
    {
        specificCmd = TM_8BIT_ZERO;
        multiCast   = TM_ETHER_MULTI;
    }
    else
    {
/*
 * 2.   If it's not zero, call tfIpBroadMcastToLan to convert the IP address
 *      into an Ethernet multicast address.  TM_ETHER_MULTI will be returned
 *      if the address was converted successfully.
 */     
        specificCmd = TM_8BIT_YES;
        multiCast = tfIpBroadMcastToLan( groupAddress, devEntryPtr,
                                         TM_16BIT_ZERO, enetAddr.enuAddress );
    }

/*
 * 3.   If no error occurred, call tfEtherSetMcastAddr to add/remove the given
 *      Ethernet address on the device.
 */     
    if (multiCast == TM_ETHER_MULTI)
    {
        errorCode = tfEtherSetMcastAddr(devEntryPtr,
                                        enetAddr.enuAddress,
                                        specificCmd,
                                        command);
    }
    else
    {
        errorCode = TM_EADDRNOTAVAIL;
    }

    
    return errorCode;
}

/****************************************************************************
 * FUNCTION: tfIgmpSocketCacheNewInsert
 *
 * PURPOSE: Cache multicast group join on socket. Keep track of the multicast
 *   groups we've joined on this socket, so that when we later close this
 *   socket, we can auto-leave these groups.
 * If TM_USE_IGMPV3 is defined we use a doubly link list to insert a new socket
 * mcast cache entry, using our tfListxx() APIs. We also add the source list to
 * the entry.
 * If TM_USE_IGMPV3 is not defined we use an array to store the socket mcast
 * cache entry.
 *
 * PARAMETERS:
 * imrIntPtr        Pointer to internal mcast request structure conveying
 *                  parameters set by the user in the user API.
 *
 * RETURNS:
 * TM_ENOERROR      Success
 * TM_ENOBUFS       Not enough memory to cache the new entry
 * TM_EADDRNOTAVAIL duplicate source addresses from the user
 *
 * NOTES:
 *
 ****************************************************************************/
static int tfIgmpSocketCacheNewInsert (ttIpMreqSrcPtr imrIntPtr)
{
    ttSocketEntryPtr    socketEntryPtr;
    ttSockIgmpInfoPtr   sockIgmpInfoPtr;
#ifndef TM_USE_IGMPV3
    ttSockIgmpInfoPtr   arrayToRealloc;
    int                 index;
    int                 oldInfoAllocCount;
#endif /* !TM_USE_IGMPV3 */
    int                 errorCode;

    socketEntryPtr = imrIntPtr->imrSocketPtr;
#ifdef TM_USE_IGMPV3
/* Allocate a socket mcast cache entry */
    sockIgmpInfoPtr = (ttSockIgmpInfoPtr)tm_get_raw_buffer(
                                            sizeof(ttSockIgmpInfo));
    if (sockIgmpInfoPtr != TM_SOC_IGMP_INFO_NULL_PTR)
    {
        errorCode = TM_ENOERROR;
        tm_bzero(sockIgmpInfoPtr, sizeof(ttSockIgmpInfo));
/* Copy fields derived from user parameters */
        sockIgmpInfoPtr->siiDevicePtr = imrIntPtr->imrDevPtr;
        tm_ip_copy(imrIntPtr->imrMcastAddr, sockIgmpInfoPtr->siiMcastAddr);
/* Add IGMPv3 source list to the entry */
        sockIgmpInfoPtr->siiSourceList.srcMode = (tt16Bit)imrIntPtr->imrFmode;
        if (imrIntPtr->imrSrcAddrPtr != (struct in_addr TM_FAR *)0)
        {
            errorCode = tfIgmpSocketCacheAddSource(sockIgmpInfoPtr,
                                                   imrIntPtr);
            if (errorCode != TM_ENOERROR)
            {
                tfIgmpSocketCacheFree(sockIgmpInfoPtr);
                sockIgmpInfoPtr = (ttSockIgmpInfoPtr)0;
            }
        }
/* Add socket cache mcast entry to the list on the socket */
        if (sockIgmpInfoPtr != (ttSockIgmpInfoPtr)0)
        {
            tfListAddToTail(&socketEntryPtr->socIgmpInfoList,
                            &sockIgmpInfoPtr->siiNode);
        }
    }
    else
    {
        errorCode = TM_ENOBUFS;
    }
#else /* !TM_USE_IGMPV3 */
    sockIgmpInfoPtr = (ttSockIgmpInfoPtr)0;
    errorCode = TM_ENOERROR;
    if (socketEntryPtr->socIgmpInfoArray == TM_SOC_IGMP_INFO_NULL_PTR)
    {
/* do the initial allocation, if we haven't cached anything yet */
        socketEntryPtr->socIgmpInfoArray =
            (ttSockIgmpInfoPtr) tm_get_raw_buffer(
                TM_IGMP_SOCKET_BLOCKING_FACTOR * sizeof(ttSockIgmpInfo));
            
        if (socketEntryPtr->socIgmpInfoArray == TM_SOC_IGMP_INFO_NULL_PTR)
        {
            errorCode = TM_ENOBUFS;
            goto exitIgmpSocketCacheNewInsert;
        }
        else
        {
            socketEntryPtr->socIgmpInfoAllocCount =
                TM_IGMP_SOCKET_BLOCKING_FACTOR;
        }
    }

/* prepare to cache this multicast group on this socket */
    index = socketEntryPtr->socIgmpInfoInUseCount;
    if (index >= (int)(socketEntryPtr->socIgmpInfoAllocCount))
    {
/* realloc and copy */
        arrayToRealloc = socketEntryPtr->socIgmpInfoArray;
        oldInfoAllocCount = socketEntryPtr->socIgmpInfoAllocCount;
        socketEntryPtr->socIgmpInfoArray =
            (ttSockIgmpInfoPtr) tm_get_raw_buffer((ttPktLen)
                (oldInfoAllocCount + TM_IGMP_SOCKET_BLOCKING_FACTOR)
                * (int)sizeof(ttSockIgmpInfo));
        if (socketEntryPtr->socIgmpInfoArray == TM_SOC_IGMP_INFO_NULL_PTR)
        {
/* failed to realloc */
            socketEntryPtr->socIgmpInfoArray = arrayToRealloc;
            goto exitIgmpSocketCacheNewInsert;
        }
        else
        {
/* copy to the reallocated buffer */
            tm_bcopy(
                arrayToRealloc, socketEntryPtr->socIgmpInfoArray,
                oldInfoAllocCount * (int)sizeof(ttSockIgmpInfo));
/* free the old buffer */
            tm_free_raw_buffer((ttRawBufferPtr) arrayToRealloc);
/* store the new number of entries reallocated */
            socketEntryPtr->socIgmpInfoAllocCount = (tt16Bit)
                (oldInfoAllocCount + TM_IGMP_SOCKET_BLOCKING_FACTOR);
        }
    }

/* cache this multicast group on this socket */
    socketEntryPtr->socIgmpInfoInUseCount++;

    socketEntryPtr->socIgmpInfoArray[index].siiDevicePtr = imrIntPtr->imrDevPtr;
    sockIgmpInfoPtr = &socketEntryPtr->socIgmpInfoArray[index];
    tm_ip_copy(imrIntPtr->imrMcastAddr, sockIgmpInfoPtr->siiMcastAddr);
exitIgmpSocketCacheNewInsert:
#endif /* !TM_USE_IGMPV3 */
    return errorCode;
}

#ifdef TM_USE_IGMPV3
/****************************************************************************
 * FUNCTION: tfIgmpSocketCacheFree
 *
 * PURPOSE:
 *   Free passed cache entry
 *   . free the socket source list, and socket mcast cache entry.
 *
 * PARAMETERS:
 *   sockIgmpInfoPtr:
 *       Pointer to the cache entry to remove from the socket mcast cache.
 *
 * RETURNS:
 *   NONE
 *
 ****************************************************************************/
static void tfIgmpSocketCacheFree (
    ttSockIgmpInfoPtr sockIgmpInfoPtr)
{
/* Free the socket source list on the socket multicast cache entry */
    tfIgmpFreeSourceList(&sockIgmpInfoPtr->siiSourceList);
/* Free the socket multicast cache entry */
    tm_free_raw_buffer(sockIgmpInfoPtr);
}
#endif /* TM_USE_IGMPV3 */

/****************************************************************************
 * FUNCTION: tfIgmpSocketCacheMatch
 *
 * PURPOSE: Check whether given multicast group is joined on socket, on that
 *          device. If TM_USE_IGMPV3 is defined, check also that the source
 *          address is allowed.
 *
 * PARAMETERS:
 *   socketEntryPtr:
 *       Pointer to the socket on which we want to check membership to the
 *       specified multicast group. 
 *   devEntryPtr:
 *       The interface on which we want to check the membership of the
 *       specified multicast group.
 *   groupAddress:
 *       The IPv4 multicast group address
 *   srcAddress:
 *       The IPv4 source address
 *
 * RETURNS:
 *   0:  The specified multicast group was NOT already previously joined on
 *       this socket, or the source address is not allowed.
 *   1:  The specified multicast group was already previously joined on this
 *       socket, and the source address is allowed.
 *
 * NOTES:
 *
 ****************************************************************************/
tt8Bit tfIgmpSocketCacheMatch(
    ttSocketEntryPtr socketEntryPtr,
    ttDeviceEntryPtr devEntryPtr,
    tt4IpAddress     groupAddress
#ifdef TM_USE_IGMPV3
   ,tt4IpAddress     srcAddress
#endif /* TM_USE_IGMPV3 */
    )
{
    ttSockIgmpInfoPtr sockIgmpInfoPtr;
    tt8Bit            isMember;

    sockIgmpInfoPtr = tfIgmpSocketCacheGetParams(socketEntryPtr,
                                                 devEntryPtr,
                                                 groupAddress);
    if (sockIgmpInfoPtr != (ttSockIgmpInfoPtr)0)
    {
/* match on both multicast group address and multicast device */
#ifdef TM_USE_IGMPV3
/* check if this source address is allowed according to the socket source list */
        isMember = tfIgmpCacheSrcAllowed(&sockIgmpInfoPtr->siiSourceList,
                                         srcAddress);
#else /* !TM_USE_IGMPV3*/
        isMember = TM_8BIT_YES;
#endif /* !TM_USE_IGMPV3*/
    }
    else
    {
        isMember = TM_8BIT_NO;
    }
    return isMember;
}

#ifdef TM_USE_IGMPV3
/****************************************************************************
 * FUNCTION: tfIgmpCacheSrcAllowed
 *
 * PURPOSE: Check whether given source address is allowed on a source list.
 * 1. Check whether the source is in the source list. If index is -1, source
 *    address is not in the source list.
 * 2. Source address is allowed:
 *      If mode is exclude, and source address is not in the source list,
 *   or if mode is include and source address is in the source list.
 *
 * PARAMETERS:
 *   srcListPtr:
 *       Pointer to a source list
 *   srcAddress:
 *       The IPv4 source address we want to check
 *
 * RETURNS:
 *   0:  The specified source address is not allowed according to the passed
 *       source list.
 *   1:  The specified source address is allowed according to the passed
 *       source list.
 *
 * NOTES:
 *
 ****************************************************************************/
static tt8Bit
tfIgmpCacheSrcAllowed (ttSourceListPtr srcListPtr, tt4IpAddress srcAddress)
{
    int    index;
    int    searchIndex;
    tt8Bit isMember;

/* Is source in the list ? */
    if (srcListPtr->srcCount == TM_16BIT_ZERO)
    {
/* Empty source list */
        index = -1;
    }
    else
    {
        searchIndex = 0;
        index = tfIgmpCacheGetSource(srcListPtr,
                                     &searchIndex,
                                     srcAddress);
    }
    if (   (    (index == -1)
             && (srcListPtr->srcMode == TM_MODE_IS_EXCLUDE) )
/* Not in the source list, and mode is exclude */
        || (    (index != -1)
             && (srcListPtr->srcMode == TM_MODE_IS_INCLUDE) )
/* or in the source list, and mode is include */
      )
/* Multcast incoming traffic allowed from this peer address */
    {
        isMember = TM_8BIT_YES;
    }
    else
    {
        isMember = TM_8BIT_NO;
    }
    return isMember;
}
#endif /* TM_USE_IGMPV3 */

/****************************************************************************
 * FUNCTION: tfIgmpSocketCacheGet
 *
 * PURPOSE: Get socket mcast cache entry corresponding to multicast group on
 *          socket given a user mcast request structure.
 * . Extract socketEntryPtr, devEntryPtr, and mcast address from the user mcast
 *   request structure, and call tfIgmpSocketCacheGetParams().
 *
 * PARAMETERS:
 *   imrIntPtr 
 *      Pointer to internal mcast request structure conveying parameters set
 *      by the user in the user API.
 * RETURNS:
 *   NULL
 *       No corresponding socket info found. Group address was not joined.
 *   sockIgmpInfoPtr
 *       The specified multicast group was already previously joined on this
 *       socket.
 *
 * NOTES:
 *
 ****************************************************************************/
ttSockIgmpInfoPtr
tfIgmpSocketCacheGet (ttIpMreqSrcPtr imrIntPtr)
{
    return tfIgmpSocketCacheGetParams(imrIntPtr->imrSocketPtr,
                                      imrIntPtr->imrDevPtr,
                                      imrIntPtr->imrMcastAddr);
}

/****************************************************************************
 * FUNCTION: tfIgmpSocketCacheGetParams
 *
 * PURPOSE: Get socket mcast cache entry given a multicast group/device/socket.
 *  . If TM_USE_IGMPV3 is defined, the socket mcast cache is a doubly linked
 *    list, and we use tfListWalk() to find the match, using a call back match
 *    function.
 *  . If TM_USE_IGMPV3 is not defined we look up the socket mcast cache array.
 *
 * PARAMETERS:
 *   socketEntryPtr:
 *       Pointer to the socket on which we want to check membership to the
 *       specified multicast group. 
 *   devEntryPtr:
 *       The interface on which we want to check the membership of the specified
 *       multicast group.
 *   groupAddress:
 *       The IPv4 multicast group address
 *
 * RETURNS:
 *   NULL
 *       No corresponding socket info found. Group address was not joined.
 *   sockIgmpInfoPtr
 *       The specified multicast group was already previously joined on this
 *       socket.
 *
 * NOTES:
 *
 ****************************************************************************/
static ttSockIgmpInfoPtr
tfIgmpSocketCacheGetParams (ttSocketEntryPtr socketEntryPtr,
                            ttDeviceEntryPtr devEntryPtr,
                            tt4IpAddress     groupAddress)
{
    ttSockIgmpInfoPtr sockIgmpInfoPtr;
#ifdef TM_USE_IGMPV3
    ttIgmpMatch       igmpMatch;
    ttGenericUnion    genParm1;
#else /* !TM_USE_IGMPV3 */
    int               index;
    int               maxIndex;
#endif /* !TM_USE_IGMPV3 */

    sockIgmpInfoPtr = (ttSockIgmpInfoPtr)0;
/* check if we've previously cached this multicast group on this socket */
#ifdef TM_USE_IGMPV3
/* Socket mcast cache is a doubly linked list */
    if (socketEntryPtr->socIgmpInfoList.listCount != 0)
    {
        igmpMatch.iiDevicePtr = devEntryPtr;
        tm_ip_copy(groupAddress, igmpMatch.iiMcastAddr);
        genParm1.genVoidParmPtr = (ttVoidPtr)&igmpMatch;
        sockIgmpInfoPtr = (ttSockIgmpInfoPtr)(ttVoidPtr)
                            tfListWalk(&socketEntryPtr->socIgmpInfoList,
                                       tfIgmpSocketCacheEntryMatch, genParm1);
    }
#else /* !TM_USE_IGMPV3 */
/* Socket mcast cache is an array */
    maxIndex = (int)(socketEntryPtr->socIgmpInfoInUseCount);
    for (index = 0; index < maxIndex; index++)
    {
        if (tm_ip_match(
                groupAddress,
                socketEntryPtr->socIgmpInfoArray[index].siiMcastAddr)
            && (socketEntryPtr->socIgmpInfoArray[index].siiDevicePtr
                == devEntryPtr))
        {
/* match on both multicast group address and multicast device */
            sockIgmpInfoPtr = &socketEntryPtr->socIgmpInfoArray[index];
            break;
        }
    }
#endif /* !TM_USE_IGMPV3 */
    return sockIgmpInfoPtr;
}

/*
 * tfIgmpJoinHostGroup Function Description
 * The tfIgmpJoinHostGroup function requests that this hosts become a member
 * of the host group identified by groupAddress on the given network
 * interface.
 * This function is called with the device locked.
 * 1. Call tfIgmpFindEntry() to find out if groupAddress is already in the list
 *    of IGMP entries on the interface.
 * 2. If it does not exist, call tfIgmpAddEntry() to allocate and insert a new
 *    IGMP entry.
 * 3. else increase owner count on the igmpEntry
 * 4. IGMPV3: determine which version (1, 2, or 3) of membership report we need
 *     to send.
 * 5. IGMPv3: Check if we have a transition to Exclusive Join with no source
 * 6. IGMPV3: call tfIgmpEntryChangeSource() to store the source list and
 *    transition to exclusive join with no source. In tfIgmpEntryChangeSource
 *    a state change report will be sent if needed (i.e. a change occured at
 *    the IGMP level), and allowed (i.e. we did not detect a lower version
 *    router). If a state change report is sent, we will also schedule its
 *    retransmission.
 * 7. IGMPV3: If tfIgmpEntryChangeSource() failed, unget the IGMP entry, and
 *    free the entry if it was dequeued (i.e owner count reached 0).
 * 8. If we are supposed to send v1, or v2 reports (i.e. a lower version
 *    router was detected), call tfIgmpEntryJoinHostGroup() to send the report,
 *    and schedule a timer to resend it one more time.
 * Parameters
 * Parameter        Description
 * socketCall       1: call made from socket APIs.
 *                  0: called by the kernel.
 * imrIntPtr        Pointer to internal mcast request structure conveying
 *                  parameters set by the user in the user API.
 * Returns
 * Value              Meaning
 * TM_ENOERROR      success
 * TM_ENOBUFS       Not enough memory to store the new entry, or send a
 *                  report.
 */
static int tfIgmpJoinHostGroup (
    tt8Bit           socketCall,
    ttIpMreqSrcPtr   imrIntPtr)
{
    ttIgmpEntryPtr      igmpEntryPtr;
    int                 errorCode;
#ifdef TM_USE_IGMPV3
    int                 dequeued;
    int                 deltaEmptyExclJoin;
    tt8Bit              v1orv2;
#endif /* TM_USE_IGMPV3 */

    TM_UNREF_IN_ARG(socketCall);
    errorCode = TM_ENOERROR;
    igmpEntryPtr = tfIgmpFindEntry(imrIntPtr->imrMcastAddr,
                                   imrIntPtr->imrDevPtr);
    if (igmpEntryPtr == (ttIgmpEntryPtr)0)
    {
        igmpEntryPtr = tfIgmpAddEntry(imrIntPtr, &errorCode);
    }
    else
    {
        if (    (socketCall != TM_8BIT_NO)
             || (igmpEntryPtr->igmpKernelJoin == TM_8BIT_ZERO) )
/*
 * Increment the owner count for joins, if called from socket, or first time
 * call from kernel.
 */
        {
            igmpEntryPtr->igmpOwnerCount++;
        }
    }
    if (igmpEntryPtr != (ttIgmpEntryPtr)0)
    {
        if (    (socketCall != TM_8BIT_NO)
              || (igmpEntryPtr->igmpKernelJoin == TM_8BIT_NO) 
            )
        {
#ifdef TM_USE_IGMPV3
            if (tm_8bit_one_bit_set(igmpEntryPtr->igmpDevPtr->devIgmpVFlag,
                                  (TM_IGMP_DEV_V1 | TM_IGMP_DEV_V2)))
            {
/* Lower version router was detected */
                v1orv2 = TM_8BIT_YES;
            }
            else
            {
                v1orv2 = TM_8BIT_NO;
            }
            if (    (imrIntPtr->imrFmode == TM_MODE_IS_EXCLUDE)
                 && (imrIntPtr->imrNumSrc == 0))
/* Transition to Exclusive Join with no source */
            {
                deltaEmptyExclJoin = 1;
            }
            else
            {
                deltaEmptyExclJoin = 0;
            }
/*
 * Use imrIntPtr to store source list, and mode. Send state change report
 * if needed and allowed.
 * Device potentially unlocked/relocked.
 */
            errorCode = tfIgmpEntryChangeSource(igmpEntryPtr, imrIntPtr,
                                                (ttSourceListPtr)0,
                                                deltaEmptyExclJoin,
                                                TM_8BIT_NO
                                                );
            if (errorCode != TM_ENOERROR)
            {
/* Undo if we fail */
                dequeued = tfIgmpEntryUnget(igmpEntryPtr);
                if (dequeued)
                {
                    tfIgmpEntryFree(igmpEntryPtr);
                }
            }
            else if (v1orv2 != TM_8BIT_NO)
#endif /* TM_USE_IGMPV3 */
            {
/* Send v1, or v2 report */
                (void)tfIgmpEntryJoinHostGroup(igmpEntryPtr);
            }
            if (    (socketCall == TM_8BIT_NO)
#ifdef TM_USE_IGMPV3
                 && (errorCode == TM_ENOERROR)
#endif /* TM_USE_IGMPV3 */
               )
            {
                igmpEntryPtr->igmpKernelJoin = TM_8BIT_YES;
            }
        }
    }
    return errorCode;
}

/*
 * tfIgmpAddEntry Function Description
 * Allocate a new IGMP entry, telling the device driver about the multicast
 * addition.
 * This function is called with the device locked.
 * 1. allocate a new ttIgmpEntry entry, zero it. Use tm_get_raw_buffer().
 * 2. Initialize IGMP entry fields from passed parameters, and owner count
 *    to 1.
 * 3. If the device is loop back or mcast address is loop back initialize
 *    IGMP state to TM_IGMP_LOCAL so that we do not send reports for that
 *    group, otherwise initialize the state to TM_IGMP_NON_MEMBER.
 * 4. If TM_USE_IGMPV3 is defined, initialize the IGMP entrys source lists.
 * 5. Let the device driver know that we now want to receive the group
 *    multicast, by calling linkLayer
 *    (*lnkMcastFuncPtr) to add the muticast group.
 * 6. If (*lnkMcastFuncPtr)() is successful, queue the new IGMP entry on the
 *    device.
 * Parameter        Description
 * imrIntPtr        Pointer to internal mcast request structure conveying
 *                  parameters set by the user in the user API.
 * errorCodePtr     pointer to error to set.
 *
 * Returns
 * Value              Meaning
 * igmpEntryPtr     pointer to added IGMP entry
 */
static ttIgmpEntryPtr tfIgmpAddEntry (
    ttIpMreqSrcPtr   imrIntPtr,
    ttIntPtr         errorCodePtr)
{
    ttDeviceEntryPtr    devEntryPtr;
    ttLinkLayerEntryPtr devLinkLayerProtocolPtr;
    ttIgmpEntryPtr      igmpEntryPtr;
    ttIgmpEntryPtr      firstIgmpEntryPtr;
#ifdef TM_USE_IGMPV3
    int                 i;
#endif /* TM_USE_IGMPV3 */
    int                 errorCode;

    igmpEntryPtr = (ttIgmpEntryPtr)tm_get_raw_buffer(sizeof(ttIgmpEntry));
    if (igmpEntryPtr == (ttIgmpEntryPtr)0)
    {
        errorCode = TM_ENOBUFS;
    }
    else
    {
/* Initialize */
        tm_bzero((ttVoidPtr)igmpEntryPtr, sizeof(ttIgmpEntry));
        devEntryPtr = imrIntPtr->imrDevPtr;
        igmpEntryPtr->igmpDevPtr = devEntryPtr;
        tm_ip_copy(imrIntPtr->imrMcastAddr, igmpEntryPtr->igmpHostGroup);
        igmpEntryPtr->igmpMhomeIndex = imrIntPtr->imrMhomeIndex;
        igmpEntryPtr->igmpOwnerCount = 1;
        if (    (tm_dev_is_loopback(igmpEntryPtr->igmpDevPtr))
             || (tm_ip_is_local_mcast_address(igmpEntryPtr->igmpHostGroup)))
        {
            igmpEntryPtr->igmpState = TM_IGMP_LOCAL; /* Do not send reports */
        }
        else
        {
            igmpEntryPtr->igmpState = TM_IGMP_NON_MEMBER; /* not a member yet */
        }
#ifdef TM_USE_IGMPV3
/*
 * All source lists are initialized as empty, i.e. include mode with no source list
 * for all lists except the TO_EX(x) and TO_IN(x) lists which are initialized
 * with the TM_IGMP_INVALID_MODE. See below.
 */
        for (i = 0; i < TM_IGMP_MAX_SRC_LIST; i++)
        {
            igmpEntryPtr->igmpSrcList[i].srcMode = TM_MODE_IS_INCLUDE;
        }
/*
 * Initialize TO_IN(x), TO_EX(x), with an invalid mode (empty), so that we do
 * not send a report (yet). That value will be changed when a change of state
 * dictates that we send a report.
 */
        for ( i = TM_CHANGE_TO_INCLUDE_MODE;
              i <= TM_CHANGE_TO_EXCLUDE_MODE;
              i++)
        {
            igmpEntryPtr->igmpSrcList[i].srcMode = TM_IGMP_INVALID_MODE;
        }
#endif /* TM_USE_IGMPV3 */
        errorCode = TM_ENOERROR;
        devLinkLayerProtocolPtr = devEntryPtr->devLinkLayerProtocolPtr;
/* Tell the device driver */
        if (devLinkLayerProtocolPtr->lnkMcastFuncPtr
                                                 != TM_LL_MCAST_FUNC_NULL_PTR)
        {
/* Buffer the mcast address at the device level, and tell the device driver */
            errorCode = (*(devLinkLayerProtocolPtr->lnkMcastFuncPtr))(
                                                  (ttVoidPtr)devEntryPtr,
                                                  imrIntPtr->imrMcastAddr,
                                                  TM_LNK_ADD_MCAST);
        }
        if (errorCode == TM_ENOERROR)
        {
/* Insert in device cache */
            firstIgmpEntryPtr = (ttIgmpEntryPtr)devEntryPtr->devIgmpPtr;
            igmpEntryPtr->igmpNextPtr = firstIgmpEntryPtr;
            if (firstIgmpEntryPtr != (ttIgmpEntryPtr)0)
            {
                firstIgmpEntryPtr->igmpPrevPtr = igmpEntryPtr;
            }
            devEntryPtr->devIgmpPtr = (ttVoidPtr)igmpEntryPtr;
/* End of insertion */
        }
        else
        {
/*
 * Undo intermediate buffering of the mcast address on the device, in case we
 * failed to add at the device driver level.
 */
            (void)(*(devLinkLayerProtocolPtr->lnkMcastFuncPtr))(
                                                  (ttVoidPtr)devEntryPtr,
                                                  imrIntPtr->imrMcastAddr,
                                                  TM_LNK_DEL_MCAST);
            tm_free_raw_buffer(igmpEntryPtr);
            igmpEntryPtr = (ttIgmpEntryPtr)0;
        }
    }
    *errorCodePtr = errorCode;
    return igmpEntryPtr;
}

/*
 * tfIgmpEntryJoinHostGroup Function Description.
 * Called when the user joins a new host group and when we send V1, or V2 only
 * reports.
 * 1. If state is in the non member state (not joined, not joining).
 *    1.1 Set state to JOIN_PENDING
 *    1.2 call tfIgmpV1V2Send(igmpEntryPtr, TM_IGMP_V2_REPORT)
 *    1.3 set state to TM_IGMP_DELAYING_MEMBER
 *    1.4 start a non auto timer with call back function tfIgmpTimer()
 *        to expire in a random value between 0, and
 *        tvIgmpUnsolReportIntv (10 seconds), so that we can re-send the
 *        report one more time.
 * Parameters
 * Parameter        Description
 * igmpEntryPtr     Pointer to igmp entry
 * Returns
 * Value              Meaning
 * TM_ENOERROR      no error
 */
static int tfIgmpEntryJoinHostGroup (
    ttIgmpEntryPtr   igmpEntryPtr)
{
    tt32Bit             timeout;
    ttGenericUnion      timerParm1;

/* Join only if we are not in the process of joining or have not joined yet */
    if (igmpEntryPtr->igmpState == TM_IGMP_NON_MEMBER)
    {
        igmpEntryPtr->igmpState = TM_IGMP_JOIN_PENDING;
        (void)tfIgmpV1V2Send(igmpEntryPtr, TM_IGMP_V2_REPORT);
        igmpEntryPtr->igmpState = TM_IGMP_DELAYING_MEMBER;
/*
 * Initialize the time out with a random value between 0, and
 * tvIgmpUnsolReportIntv time value (in milliseconds).
 */
        timerParm1.genVoidParmPtr = (ttVoidPtr)igmpEntryPtr;
        timeout = tfIgmpRandom( (tt32Bit) ( 
                            ((tt32Bit)(tm_context(tvIgmpUnsolReportIntv))) *
                                        TM_UL(1000) ) );
        igmpEntryPtr->igmpTimerPtr[TM_IGMP_TIMER] = 
                    tfTimerAdd( tfIgmpTimer,
                                timerParm1,
                                timerParm1, /* unused */
                                timeout,
                                TM_TIM_AUTO );
    }
    return TM_ENOERROR;
}

/****************************************************************************
 * FUNCTION: tfIgmpSocketCacheGetRemove
 *
 * PURPOSE: Remove multicast group join cached on socket. Keep track of the
 *   multicast groups we've joined on this socket, so that when we later close
 *   this socket, we can auto-leave these groups.
 *
 * PARAMETERS:
 * imrIntPtr        Pointer to internal mcast request structure conveying
 *                  parameters set by the user in the user API.
 * RETURNS:
 *   NONE
 *
 * NOTES:
 *
 ****************************************************************************/
static void tfIgmpSocketCacheGetRemove (ttIpMreqSrcPtr imrIntPtr)
{
    ttSockIgmpInfoPtr   sockIgmpInfoPtr;

/* Get cache entry */
    sockIgmpInfoPtr = tfIgmpSocketCacheGet(imrIntPtr);
    if (sockIgmpInfoPtr != (ttSockIgmpInfoPtr)0)
    {
/* Remove it */
        tfIgmpSocketCacheRemove(sockIgmpInfoPtr, imrIntPtr->imrSocketPtr);
    }
    return;
}

/****************************************************************************
 * FUNCTION: tfIgmpSocketCacheRemove
 *
 * PURPOSE:
 *   Remove passed cache entry from socket mcast cache.
 *   . If TM_USE_IGMPV3 is defined, use tfListRemove() to remove the entry
 *     from the doubly linked list, and free the socket source list, and
 *     socket mcast cache entry.
 *   . If TM_USE_IGMPV3 is not defined, remove the cache entry from the array.
 *
 * PARAMETERS:
 *   sockIgmpInfoPtr:
 *       Pointer to the cache entry to remove from the socket mcast cache.
 *   socketEntryPtr:
 *       Pointer to the socket on which we want to leave the specified
 *       multicast group.
 *
 * RETURNS:
 *   NONE
 *
 ****************************************************************************/
static void tfIgmpSocketCacheRemove(
    ttSockIgmpInfoPtr   sockIgmpInfoPtr,
    ttSocketEntryPtr    socketEntryPtr)
{
#ifndef TM_USE_IGMPV3
    ttSockIgmpInfoPtr   maxSockIgmpInfoPtr;
#endif /* !TM_USE_IGMPV3 */

#ifdef TM_USE_IGMPV3
/* Remove entry from the doubly linked list */
    tfListRemove(&socketEntryPtr->socIgmpInfoList,
                 &sockIgmpInfoPtr->siiNode);
/* Free the entry and its associated source list */
    tfIgmpSocketCacheFree(sockIgmpInfoPtr);
#else /* !TM_USE_IGMPV3 */
/*
 * Uncache this multicast group on the socket: remove the cache entry from the
 * array.
 */
    maxSockIgmpInfoPtr = &(socketEntryPtr->socIgmpInfoArray[
                            (int)(socketEntryPtr->socIgmpInfoInUseCount - 1)]);

    while (sockIgmpInfoPtr < maxSockIgmpInfoPtr)
    {
/* avoid overlapping bcopy by copying up one entry at a time */
        tm_bcopy(sockIgmpInfoPtr + 1,
                 sockIgmpInfoPtr,
                 sizeof(ttSockIgmpInfo));
        sockIgmpInfoPtr++;
    }
/* Number of mcast cache entries on the socket */
    socketEntryPtr->socIgmpInfoInUseCount--;
#endif /* !TM_USE_IGMPV3 */
    return;
}

#ifdef TM_USE_IGMPV3
/****************************************************************************
 * FUNCTION: tfIgmpFreeSourcList
 *           IGMPv3 only function.
 *
 * PURPOSE:
 *   Free a source list.
 *   1. If we are not using preallocated sources:
 *     1.1 If a buffer was allocated to store sources, and owner counts
 *       1.1.1 free it
 *       1.1.2 Reset source address pointer, and source owner count pointer.
 *       1.1.3 Reset source allocation count.
 *       1.1.4 Reset source list count.
 *   2. When using preallocated sources, reset the source list count.
 *
 *
 * PARAMETERS:
 *   srcListPtr
 *       Pointer to a source list.
 *
 * RETURNS:
 *   NONE
 *
 ****************************************************************************/
static void tfIgmpFreeSourceList(ttSourceListPtr srcListPtr)
{
#ifdef TM_IGMP_PREALLOCATED_SRC
    if (srcListPtr->srcAddrPtr != &srcListPtr->srcAddrArr[0])
#endif /* TM_IGMP_PREALLOCATED_SRC */
    {
        if (srcListPtr->srcAddrPtr != (struct in_addr TM_FAR *)0)
        {
            tm_free_raw_buffer((ttRawBufferPtr)(ttVoidPtr)
                               (srcListPtr->srcAddrPtr));
            srcListPtr->srcAddrPtr = (struct in_addr TM_FAR *)0;
            srcListPtr->srcOwnerCountPtr = (tt16BitPtr)0;
            srcListPtr->srcAllocCount = TM_16BIT_ZERO;
#ifndef TM_IGMP_PREALLOCATED_SRC
            srcListPtr->srcCount = TM_16BIT_ZERO;
#endif /* !TM_IGMP_PREALLOCATED_SRC */
        }
    }
#ifdef TM_IGMP_PREALLOCATED_SRC
    srcListPtr->srcCount = TM_16BIT_ZERO;
#endif /* TM_IGMP_PREALLOCATED_SRC */
    return;
}
#endif /* TM_USE_IGMPV3 */

/*
 * tfIgmpLeaveHostGroup Function Description
 * The tfIgmpLeaveHostGroup function requests that this hosts give up its
 * membership in the host group identified by groupAddress on the given
 * network interface. This function is called with the device locked.
 * 1. Call tfIgmpFindEntry() to locate the group address in the list of IGMP
 *    entries for that interface.
 * 2. If groupAddress is not in the list of IGMP entries on
 *    the interface exit with TM_EADDRNOTAVAIL error code.
 * 3. Unget the entry lowering the owner count, and dequeuing it from the
 *    cache if the owner count reaches 0.
 * 3. IGMPv3: Did we detect a lower version router on the network?
 * 4. IGMPv3: Determine if we have a transition from an exclusive join with no
 *            source. 
 * 5. IGMPV3: call tfIgmpEntryChangeSource() to store the source list change
 *    and transition from exclusive join with no source. In
 *    tfIgmpEntryChangeSource a state change report will be sent if allowed
 *    (i.e. we did not detect a lower version router). If a state change
 *    report is sent, we will also schedule its retransmission. If entry needs
 *    to be freed it will be only done after the retransmission has occured.
 * 6. If we need to send a v1/v2 report, or the mcast group is local, or
 *    an error occured (i.e. we did not queue to an IGMPv3 source timer).
 *    6.1 If it is not a socket call, or a socket call, and the IGMP entry
 *        owner count is 0.
 *        6.1.1 If we need to send a v1/v2 report,
 *              call tfIgmpEntryLeaveHostGroup
 *        6.1.2 If the IGMP entry was dequeued, free it.
 * Parameters
 * Parameter        Description
 * socketCall       1: call made from socket APIs.
 *                  0: called by the kernel.
 * imrIntPtr        Pointer to internal mcast request structure conveying
 *                  parameters set by the user in the user API.
 * Returns
 * Value              Meaning
 * TM_ENOERROR      success
 * TM_EADDRNOTAVAIL This host group was not joined on that interface
 */
static int tfIgmpLeaveHostGroup(
    tt8Bit          socketCall,
    ttIpMreqSrcPtr  imrIntPtr
#ifdef TM_USE_IGMPV3
  , ttSourceListPtr oldSrcListPtr
#endif /* TM_USE_IGMPV3 */
    )
{
    ttIgmpEntryPtr      igmpEntryPtr;
    int                 dequeued;
    int                 errorCode;
#ifdef TM_USE_IGMPV3
    int                 deltaEmptyExclJoin;
    tt8Bit              v1orv2;
#endif /* TM_USE_IGMPV3 */

    errorCode = TM_ENOERROR;
    igmpEntryPtr = tfIgmpFindEntry(imrIntPtr->imrMcastAddr,
                                   imrIntPtr->imrDevPtr);
    if (igmpEntryPtr == (ttIgmpEntryPtr)0)
    {
/* No match */
        errorCode = TM_EADDRNOTAVAIL;
    }
    else
    {
/* we found a match */
        if (    (socketCall != TM_8BIT_NO)
             || (igmpEntryPtr->igmpKernelJoin != TM_8BIT_NO) )
        {
            if (socketCall == TM_8BIT_NO)
            {
                igmpEntryPtr->igmpKernelJoin = TM_8BIT_NO;
            }
/* Unget, and if owner count reaches 0, remove the entry from the cache */
            dequeued = tfIgmpEntryUnget(igmpEntryPtr);
#ifdef TM_USE_IGMPV3
            if (tm_8bit_one_bit_set(igmpEntryPtr->igmpDevPtr->devIgmpVFlag,
                                    (TM_IGMP_DEV_V1 | TM_IGMP_DEV_V2)))
            {
/* We need to send v1, or v2 report, as a lower version router was detected. */
                v1orv2 = TM_8BIT_YES;
            }
            else
            {
                v1orv2 = TM_8BIT_NO;
            }
#ifdef TM_USE_STOP_TRECK
            if (tm_16bit_one_bit_set(igmpEntryPtr->igmpDevPtr->devFlag2,
                                     TM_DEVF2_UNINITIALIZING))
            {
                errorCode = TM_ESHUTDOWN;
            }
            else
#endif /* TM_USE_STOP_TRECK */
            {
                if (    (oldSrcListPtr != (ttSourceListPtr)0)
                     && (  (oldSrcListPtr->srcCount != TM_16BIT_ZERO)
                         || (oldSrcListPtr->srcMode != TM_MODE_IS_EXCLUDE))
                   )
                {
                    deltaEmptyExclJoin = 0;
                }
                else
                {
/*
 * Transition from empty excl join (no source list, or empty source list in
 * EXCLUDE mode)
 */
                    deltaEmptyExclJoin = -1;
                }
/* 
 * Store the source list change, and transition from empty eclusive join. Send
 * state change report if allowed (did not detect a lower version router).
 * Device potentially unlocked/relocked.
 */
                errorCode = tfIgmpEntryChangeSource(igmpEntryPtr, imrIntPtr,
                                                    oldSrcListPtr,
                                                    deltaEmptyExclJoin,
                                                    TM_8BIT_YES);
            }
            if (   (v1orv2 != TM_8BIT_NO)
                || (dequeued)
                || (igmpEntryPtr->igmpState == TM_IGMP_LOCAL)
                || (errorCode != TM_ENOERROR) )
/* We did not queue to an IGMPv3 source timer. We need to free here. */
#endif /* !TM_USE_IGMPV3 */
            {
                errorCode = TM_ENOERROR; /* Do not return error to the user */
/* V1 or V2 leave: */
                if (dequeued)
/*
 * Last leave.
 */
                {
#ifdef TM_USE_IGMPV3
                    if (v1orv2 != TM_8BIT_NO)
#endif /* TM_USE_IGMPV3 */
                    {
                        (void)tfIgmpEntryLeaveHostGroup(igmpEntryPtr);
                    }
                    if (dequeued)
                    {
/* Free the entry if it was dequeued earlier */
                        tfIgmpEntryFree(igmpEntryPtr);
                    }
                }
            }
        }
    }
    return errorCode;
}

/****************************************************************************
 * FUNCTION: tfIgmpEntryFree
 *
 * PURPOSE:
 * Free an IGMP entry
 *   1. IGMPv3: Free all source lists.
 *   2. IGMPv3: Free IGMPv3 source timer, and all IGMP timers.
 *   3. Free IGMP timer.
 *   4. Free the entry.
 *
 * PARAMETERS:
 *  igmpEntryPtr 
 *       Pointer to an IGMP entry.
 *
 * RETURNS:
 *   NONE
 *
 ****************************************************************************/
static void tfIgmpEntryFree(ttIgmpEntryPtr igmpEntryPtr)
{
    int                 timerIndex;
#ifdef TM_USE_IGMPV3
    int                 i;
#endif /* TM_USE_IGMPV3 */

    timerIndex = 0;
#ifdef TM_USE_IGMPV3
    for (i = 0; i < TM_IGMP_MAX_SRC_LIST; i++)
    {
        tfIgmpFreeSourceList(&igmpEntryPtr->igmpSrcList[i]);
    }
    tfIgmpSrcTimerRemove(igmpEntryPtr);
    for (timerIndex = 0; timerIndex < TM_IGMP_MAX_TIMERS; timerIndex++)
#endif /* TM_USE_IGMPV3 */
    {
        tfIgmpTimerRemove(igmpEntryPtr, timerIndex);
    }
    tm_free_raw_buffer((ttRawBufferPtr)igmpEntryPtr);
    return;
}

/*
 * tfIgmpEntryLeaveHostGroup Function Description
 * Called when the user leaves a host group and when we send V1, or V2 only
 * reports.
 * The tfIgmpEntryLeaveHostGroup function requests that this hosts give up its
 * membership in the host group identified by igmpEntryPtr->igmpHostGroup
 * on the given network interface. This function is called with the device
 * locked.
 * 1. Save the IGMP state before deleting the timer.
 * 2. Delete the IGMPv1/v2 timer (if any)
 * 3. If state is >  TM_IGMP_JOIN_PENDING (i.e. not local, not non member, and
 *    not joining.)
 * 3.1 Set state to TM_IGMP_NON_MEMBER
 * 3.2 if interface used IGMP v2 (TM_IGM_DEV_V1 not set on the devIgmpVFlag),
 *     and the last report flag in the igmp entry is set
 *     (TM_IGMP_LAST_GROUP_REPORT), generate an IGMP leave for that
 *     group address, by calling:
 *     tfIgmpV1V2Send(igmpEntryPtr, TM_IGMP_LEAVE)
 * Parameters
 * Parameter        Description
 * igmpEntryPtr     Pointer to an igmp entry
 * Returns
 * Value              Meaning
 * TM_ENOERROR      success
 */
static int tfIgmpEntryLeaveHostGroup (ttIgmpEntryPtr   igmpEntryPtr)
{
    int                 errorCode;
    tt8Bit              igmpState;

    errorCode = TM_ENOERROR;
    igmpState = igmpEntryPtr->igmpState;
    tfIgmpTimerRemove(igmpEntryPtr, TM_IGMP_TIMER);
    if (igmpState > TM_IGMP_JOIN_PENDING)
    {
        igmpEntryPtr->igmpState = TM_IGMP_NON_MEMBER;
        if (    (tm_8bit_bits_not_set(igmpEntryPtr->igmpDevPtr->devIgmpVFlag,
                                      TM_IGMP_DEV_V1))
             && (igmpEntryPtr->igmpFlag == TM_IGMP_LAST_GROUP_REPORT)
           )
        {
            (void)tfIgmpV1V2Send(igmpEntryPtr, TM_IGMP_LEAVE);
        }
    }
    return errorCode;
}

/*
 * tfIgmpEntryUnget Function Description
 * Unget an IGMP entry:
 * Decrease IGMP entry owner count, dequeuing it if it reaches 0.
 * 1. Decrease IGMP entry owner count.
 * 2. If owner count is 0, remove entry from the list of IGMP entries for
 *    that interface
 * 3. Call LinkLayer (*lnkMcastFuncPtr)( groupAddress, interfaceHandle,
 *                                       TM_LNK_DEL_MCAST)
 * Parameters
 * Parameter        Description
 * igmpEntryPtr     Pointer to an igmp entry
 * Returns
 * Value              Meaning
 * 1                Igmp entry was dequeued
 * 0                Igmp entry still in the cache.
 */
static int tfIgmpEntryUnget (ttIgmpEntryPtr   igmpEntryPtr)
{
    ttIgmpEntryPtr      nextIgmpPtr;
    ttLinkLayerEntryPtr devLinkLayerProtocolPtr;
    ttDeviceEntryPtr    devEntryPtr;
    int                 dequeued;

    devEntryPtr = igmpEntryPtr->igmpDevPtr;
    igmpEntryPtr->igmpOwnerCount--;
    if (igmpEntryPtr->igmpOwnerCount == 0)
    {
        nextIgmpPtr = igmpEntryPtr->igmpNextPtr;
        if (nextIgmpPtr != (ttIgmpEntryPtr)0)
        {
            nextIgmpPtr->igmpPrevPtr = igmpEntryPtr->igmpPrevPtr;
        }
        if (igmpEntryPtr->igmpPrevPtr != (ttIgmpEntryPtr)0)
        {
            igmpEntryPtr->igmpPrevPtr->igmpNextPtr = nextIgmpPtr;
        }
        else
        {
            devEntryPtr->devIgmpPtr = (ttVoidPtr)nextIgmpPtr;
        }
        dequeued = 1;
        devLinkLayerProtocolPtr = devEntryPtr->devLinkLayerProtocolPtr;
        if (devLinkLayerProtocolPtr->lnkMcastFuncPtr
                                         != TM_LL_MCAST_FUNC_NULL_PTR)
        {
            (void)(*(devLinkLayerProtocolPtr->lnkMcastFuncPtr))(
                                                  (ttVoidPtr)devEntryPtr,
                                                  igmpEntryPtr->igmpHostGroup,
                                                  TM_LNK_DEL_MCAST );
        }
    }
    else
    {
        dequeued = 0;
    }
    return dequeued;
}

/*
 * tfIgmpMember function description
 * Given a device (interface), a destination multicast IP address, and
 * a source address
 * returns TM_ENOERROR if the host is a member of the destination multicast
 * IP address on the interface it came in, and the source address is allowed;
 * returns TM_ENOENT otherwise.
 * 1. Lock the device
 * 2. call tfIgmpFindEntry(multicastGroup, devEntryPtr)
 *  2.1 if tfIgmpFindEntry() returns a non null entry pointer,
 *    2.1.1 IGMPv3: Chek that the source address is allowed.
 *    2.1.2 IGMPv3: If source is not allowed set errorCode to TM_ENOENT else:
 *    2.1.3 set errorCode to TM_ENOERROR.
 *    2.1.4 set *mhomeIndexPtr with the entry mhomeIndex.
 *  2.2. else set errorCode to TM_ENOENT, but if the device configuration has
 *      been started, but has not finished, we will let local multicast
 *      packets through by setting the errorCode to TM_ENOERROR in that case,
 *      as the user cannot add membership to a multicast address prior to
 *      configuration.
 * 3. Unlock the device
 * 4. return errorCode.
 *
 * Parameters
 * Parameter        Description
 * devEntryPtr      Pointer to interface
 * multiCastGroup   Ip address of multicast group
 * mhomeIndexPtr    Pointer to multi home index to be filled by tfIgmpMember.
 * 
 * Returns
 * Value            Meaning
 * TM_ENOERROR      success
 * TM_ENOENT        No match 
 *
 */
int tfIgmpMember(  ttDeviceEntryPtr   devEntryPtr
                 , tt4IpAddress       multiCastGroup
#ifdef TM_USE_IGMPV3
                 , tt4IpAddress       srcAddress
#endif /* TM_USE_IGMPV3 */
#ifndef TM_SINGLE_INTERFACE_HOME
                 , tt16BitPtr         mhomeIndexPtr
#endif /* TM_SINGLE_INTERFACE_HOME */
                 )
{
    ttIgmpEntryPtr  igmpEntryPtr;
    int             errorCode;
#ifndef TM_SINGLE_INTERFACE_HOME
    int             errCode;
    tt16Bit         mhomeIndex;
#endif /* TM_SINGLE_INTERFACE_HOME */
#ifdef TM_USE_IGMPV3
    tt8Bit          isMember;
#endif /* TM_USE_IGMPV3 */

    tm_lock_wait(&(devEntryPtr->devLockEntry));
    igmpEntryPtr = tfIgmpFindEntry(multiCastGroup, devEntryPtr);
    if (igmpEntryPtr != (ttIgmpEntryPtr)0)
/* Found an IGMP entry corresponding to multicast group and device */ 
    {
#ifdef TM_USE_IGMPV3
/* 
 * Check if this source address is allowed according to the IGMP current
 * source list.
 */
        isMember = tfIgmpCacheSrcAllowed(
                        &igmpEntryPtr->igmpSrcList[TM_IGMP_CURRENT_LIST],
                        srcAddress);
        if (!isMember)
        {
/* Source not allowed */
            errorCode = TM_ENOENT;
        }
        else
#endif /* TM_USE_IGMPV3 */
        {
/* allowed */
            errorCode = TM_ENOERROR;
        }
#ifndef TM_SINGLE_INTERFACE_HOME
/* Initialize corresponding multi home */
        *mhomeIndexPtr = igmpEntryPtr->igmpMhomeIndex;
#endif /* TM_SINGLE_INTERFACE_HOME */
    }
    else
    {
        errorCode = TM_ENOENT; /* assume failure */
        if (tm_ip_is_local_mcast_address(multiCastGroup))
/* local multicast group */
        {
/*
 * If the device configuration has been started, but has not finished, we
 * will let local multicast packets through, as the user cannot add membership
 * to a multicast address prior to configuration.
 */
#ifdef TM_SINGLE_INTERFACE_HOME
           if ( tm_8bit_all_bits_set( tm_ip_dev_conf_flag(devEntryPtr, 0),
                                      (   TM_DEV_IP_CONF_STARTED
                                        | TM_DEV_IP_USER_BOOT ) ) )
            {
                errorCode = TM_ENOERROR;
            }
#else /* !TM_SINGLE_INTERFACE_HOME */
            if ((int)devEntryPtr->devMhomeUsedEntries == 0)
            {
                mhomeIndex = (TM_DEV_IP_CONF_STARTED | TM_DEV_IP_USER_BOOT);
                errCode = tfMhomeAnyConf(devEntryPtr, TM_IP_ZERO, &mhomeIndex);
                if (errCode == TM_ENOERROR)
                {
                    *mhomeIndexPtr = mhomeIndex;
                    errorCode = TM_ENOERROR;
                }
            }
#endif /* !TM_SINGLE_INTERFACE_HOME */
        }
    }
/* Unlock the device */
    tm_unlock(&(devEntryPtr->devLockEntry));
    return errorCode;
}

/*
 * tfIgmpInit Function description
 * Called from tfInit() if TM_IGMP is defined.
 * . tvIgmpUnsolReportIntv initialized to TM_IGMP_UNSOL_REPORT_INTV
 * . tvIgmpRouterV1Timeout initialized to TM_IGMP_ROUTER_V1_TIMEOUT
 * . IGMPv3: tvIgmpSoMaxSrcFilter initialized to TM_SO_MAX_SOURCE_FILTER
 * . IGMPv3: tvIgmpIpMaxSrcFilter initialized to TM_IP_MAX_SOURCE_FILTER
 * Parameters       none
 *
 * Returns
 * none
 */
void tfIgmpInit(void)
{
    tm_context(tvIgmpUnsolReportIntv)     = TM_IGMP_UNSOL_REPORT_INTV;
    tm_context(tvIgmpRouterV1Timeout)     = TM_IGMP_ROUTER_V1_TIMEOUT;
#ifdef TM_USE_IGMPV3
    tm_context(tvIgmpSoMaxSrcFilter)      = (tt16Bit)TM_SO_MAX_SOURCE_FILTER;
    tm_context(tvIgmpIpMaxSrcFilter)      = (tt16Bit)TM_IP_MAX_SOURCE_FILTER;
#endif /* TM_USE_IGMPV3 */
    return;
}

/*
 * tfIgmpDevInit Function Description
 * Called for first each non loop back configured interface, when configuring
 * the first multi home.
 * Called with the device locked.
 * . Initialize default IGMP robustness variable for the device.
 * . Initialize default IGMP QQI for the device with the default
 *   query interval.
 * . Zero the internal mcast request structure
 * . Initialize imrMcastAddr to 224.0.0.1 (TM_IP_MCAST_ALL_HOSTS)
 * . Initialize imrDevPtr, and imrMhomeIndex with device, mhome being
 *   configured.
 * . Initialize imrOptionName with IPO_ADD_MEMBERSHIP
 * . IGMPv3: initialize imrFmode to EXCLUDE.
 * . Call tfIgmpJoinHostGroup()
 * . If RIP is enabled, initialize imrMcastAddr to TM_IP_MCAST_ALL_RTRS,
 *   and call (void)tfIgmpJoinHostGroup()
 * Parameters
 * Parameter   meaning
 * devEntryPtr Pointer to interface that is being configured
 * mhomeIndex  multihome index of the device slot being configured.
 * Returns
 * none
 */
void tfIgmpDevInit (ttDeviceEntryPtr devEntryPtr, tt16Bit mhomeIndex)
{
    ttIpMreqSrc             imrIntStruct;
    ttIpMreqSrcPtr          imrIntPtr;
    int                     errorCode;

    devEntryPtr->devLnkMcastAllAddresses = TM_8BIT_ZERO;
#ifdef TM_USE_IGMPV3
    if (devEntryPtr->devIgmpRobustness == TM_8BIT_ZERO)
/* Not changed yet */
    {
        devEntryPtr->devIgmpRobustness = TM_IGMP_DEF_ROBUSTNESS;
    }
    if (devEntryPtr->devIgmpQqi == 0)
/* Not changed yet */
    {
        devEntryPtr->devIgmpQqi = TM_IGMP_QUERY_INTERVAL;
    }
#endif /* TM_USE_IGMPV3 */
    imrIntPtr = &imrIntStruct;
    tm_bzero(imrIntPtr, sizeof(ttIpMreqSrc));
    imrIntPtr->imrDevPtr = devEntryPtr;
    tm_ip_copy(TM_IP_MCAST_ALL_HOSTS, imrIntPtr->imrMcastAddr);
    imrIntPtr->imrMhomeIndex = mhomeIndex;
    imrIntPtr->imrOptionName = IPO_ADD_MEMBERSHIP;
#ifdef TM_USE_IGMPV3
    imrIntPtr->imrFmode = TM_MODE_IS_EXCLUDE;
#endif /* TM_USE_IGMPV3 */
    errorCode = tfIgmpJoinHostGroup(TM_8BIT_NO, imrIntPtr);
    if (errorCode == TM_ENOERROR)
    {
        if (tm_context(tvRipOn))
        {
            tm_ip_copy(TM_IP_MCAST_ALL_RTRS, imrIntPtr->imrMcastAddr);
            (void)tfIgmpJoinHostGroup(TM_8BIT_NO, imrIntPtr);
        }
    }
    return;
}

/*
 * Function Description
 * Called when last multi home in unconfigured on a non loop back interface.
 * Called with the device locked.
 * . Zero the internal mcast request structure
 * . Initialize imrMcastAddr to 224.0.0.1 (TM_IP_MCAST_ALL_HOSTS)
 * . Initialize imrDevPtr with device, being unconfigured.
 * . Initialize imrOptionName with IPO_DROP_MEMBERSHIP
 * . IGMPv3: initialize imrFmode to INCLUDE.
 * . Call tfIgmpLeaveHostGroup()
 * . If RIP is enabled, initialize imrMcastAddr to TM_IP_MCAST_ALL_RTRS,
 *   and call (void)tfIgmpLeaveHostGroup()
 * Parameters
 * Parameter   meaning
 * devEntryPtr Pointer to interface that is being unconfigured
 * Return
 * none
 */
void tfIgmpDevClose (ttDeviceEntryPtr devEntryPtr)
{
    ttIpMreqSrc             imrIntStruct;
    ttIpMreqSrcPtr          imrIntPtr;

    imrIntPtr = &imrIntStruct;
    tm_bzero(imrIntPtr, sizeof(ttIpMreqSrc));
    imrIntPtr->imrDevPtr = devEntryPtr;
    tm_ip_copy(TM_IP_MCAST_ALL_HOSTS, imrIntPtr->imrMcastAddr);
    imrIntPtr->imrOptionName = IPO_DROP_MEMBERSHIP;
#ifdef TM_USE_IGMPV3
    imrIntPtr->imrFmode = TM_MODE_IS_INCLUDE;
#endif /* TM_USE_IGMPV3 */
    (void)tfIgmpLeaveHostGroup(
        TM_8BIT_NO,
        imrIntPtr
#ifdef TM_USE_IGMPV3
     , (ttSourceListPtr)0
#endif /* TM_USE_IGMPV3 */
        );
    if (tm_context(tvRipOn))
    {
        tm_ip_copy(TM_IP_MCAST_ALL_RTRS, imrIntPtr->imrMcastAddr);
        (void)tfIgmpLeaveHostGroup(
            TM_8BIT_NO,
            imrIntPtr
#ifdef TM_USE_IGMPV3
          , (ttSourceListPtr)0
#endif /* TM_USE_IGMPV3 */
            );
    }
    return;
}

/*
 * tfIgmpFindEntry Function Description
 * Called to retrieve an IGMP entry on an interface for the given mcast group.
 * Always called with the device locked.
 * . Search through the devIgmpPtr list for the devEntryPtr interface
 * . If found return pointer to the IGMP entry
 * . Otherwise return null.
 * Parameters
 * Parameter        Description
 * group Address     Multicast host group address
 * devEntryPtr       Pointer to interface
 *
 * Returns
 * Value              Meaning
 * (ttIgmpEntryPtr)0 No entry found
 * igmpEntryPtr      Pointer to entry
 */
static ttIgmpEntryPtr tfIgmpFindEntry( tt4IpAddress     mcastGroup,
                                       ttDeviceEntryPtr devEntryPtr )
{
    ttIgmpEntryPtr igmpEntryPtr;

    igmpEntryPtr = devEntryPtr->devIgmpPtr;
    while (igmpEntryPtr != (ttIgmpEntryPtr)0)
    {
        if ( tm_ip_match(igmpEntryPtr->igmpHostGroup, mcastGroup) )
        {
            break;
        }
        igmpEntryPtr = igmpEntryPtr->igmpNextPtr;
    }
    return igmpEntryPtr;
}

/*
 * tfIgmpV1V2Send Function description
 * Generate an IGMP packet for membership report, or leave message. Queries
 * are not supported since we are a host:
 * 1. allocate a ttPacket/ttSharedData with minimum IP header length +
 *    router alert option length + LL header size,
 *    and TM_IGMP_MIN_LEN data size. If failed, exit with TM_ENOBUFS error.
 * 2. If we fail the allocation return TM_ENOBUFS.
 * 3. If type is TM_IGMP_V2_REPORT
 *    . If devIgmpV1Flag has the TM_IGMP_DEV_V1 bit set, convert to
 *      TM_IGMP_V1_REPORT,
 *    . set last report flag in IGMP entry (TM_IGMP_LAST_GROUP_REPORT).
 * 4. IGMP header igmphGroupAddress <- groupAddress from IGMP entry.
 * 5. Call tfIgmpSendPacket()
 * 6. return errorCode returned by tfIgmpSendPacket().
 *
 * Parameters
 * Parameter        Description
 * igmpEntryPtr      Pointer to entry
 * 
 * Returns
 * Value              Meaning
 * TM_ENOBUF         Not enough memory to allocate a packet
 * TM_ENETDOWN       Network is down.
 * TM_ENOERROR       Success.
 */

static int tfIgmpV1V2Send(ttIgmpEntryPtr igmpEntryPtr, int type)
{
    ttIgmpHeaderPtr  igmpHdrPtr;
    ttPacketPtr      packetPtr;
    int              errorCode;

#ifdef TM_USE_STOP_TRECK
    if (tm_16bit_one_bit_set(igmpEntryPtr->igmpDevPtr->devFlag2,
                             TM_DEVF2_UNINITIALIZING))
    {
        errorCode = TM_ESHUTDOWN;
    }
    else
#endif /* TM_USE_STOP_TRECK */
    {
        packetPtr = 
            tfGetSharedBuffer(
                (int)
                (TM_LL_MAX_HDR_LEN+TM_IP_OVERHEAD),
                (ttPktLen)TM_IGMP_MIN_LEN,
                TM_16BIT_ZERO);
    
        if (packetPtr == (ttPacketPtr)0)
        {
            errorCode = TM_ENOBUFS;
        }
        else
        {
            packetPtr->pktMhomeIndex = igmpEntryPtr->igmpMhomeIndex;
            if (type == TM_IGMP_V2_REPORT)
            {
                if (tm_8bit_one_bit_set(igmpEntryPtr->igmpDevPtr->devIgmpVFlag,
                                        TM_IGMP_DEV_V1))
                {
                    type = TM_IGMP_V1_REPORT;
                }
                igmpEntryPtr->igmpFlag = TM_IGMP_LAST_GROUP_REPORT;
            }
            igmpHdrPtr = (ttIgmpHeaderPtr)packetPtr->pktLinkDataPtr;
/* Only in v1 or v2 report */
            tm_ip_copy(igmpEntryPtr->igmpHostGroup,
                       igmpHdrPtr->igmphGroupAddress );
            errorCode = tfIgmpSendPacket(igmpEntryPtr->igmpDevPtr,
                                         packetPtr,
                                         type);
        }
    }
    return errorCode;
}

/*
 * tfIgmpSendPacket Function description
 * Fill an IGMP packet for membership report, or leave message. Queries
 * are not supported since we are a host.
 * Function called with the device lock.
 * NOTE device unlocked and relocked when we call tfIpSendPacket().
 * Function called by tfIgmpV1V2Send, IGMPV3 tfIgmpSendStateChangeReport(),
 * and IGMPV3 tfIgmpSendCurrentStateReport();
 * 1: Fill the IGMP header fields: with type, max resp time set to 0
 *    igmphType <- type
 *    igmphMaxRespTime <- 0
 * 2. Zero checksum field, and compute IGMP checksum
 * 3. We initialize pktDeviceEntryPtr and pktMhomeIndex with values from
 *    the IGMP entry, OR the pktFlags with TM_OUTPUT_DEVICE_FLAG so that
 *    the IP layer knows the destination device and does not query the
 *    routing table.
 *    We initialize pktIpHdrLen with   TM_4PAK_IP_MIN_HDR_LEN
 *                                   + TM_IP_ROUTER_ALERT_LEN
 *    We point to IP header by subtracting the IP header length
 *    (minimum IP header length + router alert option length) from
 *    pktLinkDataPtr, and adding it to both pktLinkDataLength, and
 *    pktChainDataLength,
 *    initialize iphTtl with 1,
 *    iphUlp with TM_IP_IGMP, iphFlagsFragOff with 0,
 *    iphSrcAddr with the IGMP entry device/mhome address,
 *    iphDestAddr with 224.0.0.2 (TM_IP_MCAST_ALL_RTRS) for type
 *    TM_IGMP_LEAVE, 224.0.0.22 for IGMPv3 report, otherwise iphDestAddr is
 *    groupAddress from IGMP header.
 *    Unlock the device
 *    we call tfIpSendPacket(packetPtr, (ttLockEntryPtr)0);
 *    Relock the device
 * 4. Return errorCode returned by tfIpSendPacket
 *
 * Parameters
 * Parameter        Description
 * devEntryPtr      Pointer to device entry
 * 
 * Returns
 * Value              Meaning
 * TM_ENOBUF         Not enough memory to allocate a packet
 * TM_ENETDOWN       Network is down
 */
static int tfIgmpSendPacket(
        ttDeviceEntryPtr devEntryPtr,
        ttPacketPtr      packetPtr,
        int              type)
{
    ttIgmpHeaderPtr  igmpHdrPtr;
    ttIpHeaderPtr    iphPtr;
    tt32BitPtr       iphOptionPtr;
    int              errorCode;

/*
 * Fill the IGMP header fields: with type, 0 max resp time/reserved field, and
 * (only for v1 or v2 report) group address from IGMP entry
 */
    igmpHdrPtr = (ttIgmpHeaderPtr)packetPtr->pktLinkDataPtr;
    igmpHdrPtr->igmphType = (tt8Bit)type;
/* matches reserved field for IGMPv3 report */
    igmpHdrPtr->igmphMaxRespTime = 0;
/* Zero IGMP header checksum */
    igmpHdrPtr->igmphChecksum = (tt16Bit)0;
/* Compute checksum on IGMP header */
    igmpHdrPtr->igmphChecksum = tfPacketChecksum(
            packetPtr,
            packetPtr->pktChainDataLength,
            TM_16BIT_NULL_PTR,
            TM_16BIT_ZERO);
/*
 * Initialize pktDeviceEntryPtr with deviceEntryPtr.
 */
    packetPtr->pktDeviceEntryPtr = devEntryPtr;
/*
 * OR the pktFlags with TM_OUTPUT_DEVICE_FLAG so that the IP layer
 * knows the destination device is valid and does not query the routing table.
 */
    packetPtr->pktFlags |=   TM_OUTPUT_DEVICE_FLAG
                           | TM_IP_DEST_ROUTE_SET;
/* Initialize pktIpHdrLen with TM_4PAK_IP_MIN_HDR_LEN+TM_IP_ROUTER_ALERT_LEN */
    packetPtr->pktIpHdrLen =
            TM_4PAK_IP_MIN_HDR_LEN + TM_PAK_IP_ROUTER_ALERT_LEN;
/* Point to new IP header */
    tm_pkt_llp_hdr(packetPtr,
                       (TM_4PAK_IP_MIN_HDR_LEN + TM_PAK_IP_ROUTER_ALERT_LEN));
    iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
/* initialize iphTtl with 1 */
    iphPtr->iphTtl = 1;
/* initialize iphTos with TM_IP_DEF_TOS */
#ifdef TM_USE_IGMPV3
    if (type == TM_IGMP_V3_REPORT) 
    {
        iphPtr->iphTos = TM_IP_DEF_IGMPV3_TOS;
    }
    else
#endif /* TM_USE_IGMPV3 */
    {
        iphPtr->iphTos = TM_IP_DEF_TOS;
    }
/* Initialize iphUlp with TM_IP_IGMP */
    iphPtr->iphUlp = TM_IP_IGMP;
/* Initialize iphFlagsFragOff with 0 */
    iphPtr->iphFlagsFragOff = TM_16BIT_ZERO;
    iphOptionPtr = (tt32BitPtr)((ttCharPtr)iphPtr +TM_4PAK_IP_MIN_HDR_LEN);
    *iphOptionPtr = TM_IP_ROUTER_ALERT_WORD;
    if (type == TM_IGMP_LEAVE)
    {
/*
 * Initialize iphDestAddr with 224.0.0.2 (TM_IP_MCAST_ALL_RTRS) for type
 * TM_IGMP_LEAVE,
 */
        tm_ip_copy(TM_IP_MCAST_ALL_RTRS, iphPtr->iphDestAddr);
    }
    else
    {
#ifdef TM_USE_IGMPV3
        if (type == TM_IGMP_V3_REPORT) 
        {
            tm_ip_copy(TM_IP_MCAST_V3_REPORTS_DEST, iphPtr->iphDestAddr);
        }
        else
#endif /* TM_USE_IGMPV3 */
        {
/* otherwise iphDestAddr is groupAddress from IGMP entry. */
            tm_ip_copy(igmpHdrPtr->igmphGroupAddress, iphPtr->iphDestAddr);
        }
    }
/* Initialize iphSrcAddr with the IGMP entry device/mhome address */
    tm_ip_copy( tm_ip_dev_addr( packetPtr->pktDeviceEntryPtr,
                                packetPtr->pktMhomeIndex ),
                 iphPtr->iphSrcAddr );
/* UNLOCK the device before sending */
    tm_call_unlock(&(devEntryPtr->devLockEntry));
    errorCode = tfIpSendPacket(
            packetPtr,
            (ttLockEntryPtr)0
#ifdef TM_USE_IPSEC
            , (ttPktLenPtr)0
#ifdef TM_USE_IPSEC_TASK
            , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
#endif /* TM_USE_IPSEC */
        );
/* RELOCK the device after sending */
    tm_call_lock_wait(&(devEntryPtr->devLockEntry));
    return errorCode;
}

/*
 * tfIgmpTimer function description
 * Called by the timer execute thread to re-send a V1/V2 report, in case the
 * first one is lost, or or to send a delayed V1/V2 response to a query.
 * Timer Expired, send report, change state:
 *   . Extract igmpEntryPtr from timer parameter, and device from IGMP entry
 *   . Lock the device
 *   . If the timer has not been removed, or re-initialized:
 *     .. Remove the timer
 *     .. Reset igmp entry timer pointer.
 *     .. If IgmpEntry state is delaying member state:
 *       ... set state to TM_IGMP_IDLE_MEMBER
 *       ... call tfIgmpV1V2Send(igmpEntryPtr, TM_IGMP_V2_REPORT);
 *   . Unlock the device
 *   . Return.
 * Parameters
 * Parameter        Description
 * timerBlockPtr    Pointer to timer structure
 * userParm1        Timer parameter 1 (devEntryPtr)
 * userParm2        Timer parameter 2
 * Returns
 * None
 */
void tfIgmpTimer (ttVoidPtr      timerBlockPtr,
                  ttGenericUnion userParm1,
                  ttGenericUnion userParm2)
{
    ttTimerPtr       tmrPtr;
#ifdef TM_LOCK_NEEDED
    ttDeviceEntryPtr devEntryPtr;
#endif /* TM_LOCK_NEEDED */
    ttIgmpEntryPtr   igmpEntryPtr;

    igmpEntryPtr = (ttIgmpEntryPtr)(userParm1.genVoidParmPtr);
    
/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);

#ifdef TM_LOCK_NEEDED
    devEntryPtr = igmpEntryPtr->igmpDevPtr;
#endif /* TM_LOCK_NEEDED */
    tm_call_lock_wait(&(devEntryPtr->devLockEntry));
    tmrPtr = igmpEntryPtr->igmpTimerPtr[TM_IGMP_TIMER];
    if ( (tmrPtr == timerBlockPtr) && tm_timer_not_reinit(tmrPtr) )
    {
/* Delete the timer */
        tm_timer_remove(tmrPtr);
        igmpEntryPtr->igmpTimerPtr[TM_IGMP_TIMER] = (ttTimerPtr)0;
        if (igmpEntryPtr->igmpState == TM_IGMP_DELAYING_MEMBER)
        {
            igmpEntryPtr->igmpState = TM_IGMP_IDLE_MEMBER;
            (void)tfIgmpV1V2Send(igmpEntryPtr, TM_IGMP_V2_REPORT);
        }
    }
    tm_call_unlock(&(devEntryPtr->devLockEntry));
    return;
}

/*
 * tfIgmpVTimer Function description
 * Timer added when any lower version (v1, or v2) query is received on the
 * interface to keep track of a lower version router.
 * When this timer expires reset the corresponding version flag on
 * the devEntryPtr->devIgmpVFlag field, and reset the corresponding
 * timer: devIgmpVTmrPtr[version - 1], so that we can now send higher
 * version reports. 
 * . Extract the version from the timer parameter.
 * . If timer has not already been reset, or re-initialized:
 *  .. Lock the device
 *  .. reset devIgmpVTmrPtr[version - 1]
 *  .. Reset version flag in devIgmpVFlag so that we can now send higher
 *     version reports.
 * . Unlock the device
 * Return.
 * Parameters
 * Parameter        Description
 * timerBlockPtr    Pointer to timer structure
 * userParm1        Timer parameter 1 (version)
 * userParm2        Timer parameter 2
 * Returns
 * None
 */
void tfIgmpVTimer (ttVoidPtr      timerBlockPtr,
                   ttGenericUnion userParm1,
                   ttGenericUnion userParm2)
{
    ttTimerPtr       tmrPtr;
    ttDeviceEntryPtr devEntryPtr;
    int              version;
    tt8Bit           flag;

    devEntryPtr = (ttDeviceEntryPtr)userParm1.genVoidParmPtr;
    version = userParm2.genIntParm;
    tm_call_lock_wait(&(devEntryPtr->devLockEntry));
    tmrPtr = devEntryPtr->devIgmpVTmrPtr[version - 1];
    if ( (tmrPtr == timerBlockPtr) && tm_timer_not_reinit(tmrPtr) )
    {
/* Delete the timer */
        tm_timer_remove(tmrPtr);
        flag = (tt8Bit)(1 << version);
        devEntryPtr->devIgmpVTmrPtr[version - 1] = (ttTimerPtr)0;
        devEntryPtr->devIgmpVFlag &= ~(flag);
    }
    tm_call_unlock(&(devEntryPtr->devLockEntry));
    return;
}

/*
 * tfIgmpIncomingPacket Function Description
 * Called when an IGMP packet is received from the network.
 * 1. IGMP packets are not supposed to be exceed the device MTU 
 * 2. Point to IGMP header, update pktLinkDataLength, and pktChainDataLength 
 * 3. Check that the IGMP length is at least 8 bytes
 * 4. Save pointer to IGMP header
 * 5. Check the IGMP checksum.
 * 6. Make the packet contiguous for at least the first 8 bytes of the IGMP
 *    header
 * 7. Copy the group address from the header
 * 8. case switch statement on the IGMP header type
 *    8.1 IGMP query type. copy the destination IP address from the packet
 *        and the max response time from the IGMP header. Initialize version
 *        to an invalid value (default).
 *    8.1.1 Acceptable queries are either non general queries (non zero group
 *          address) or queries sent to 224.0.0.1
 *    8.1.2 Get the max response time from the header, and determine the
 *          version run by the querier by checking the length of the packet, 
 *          and the max response time. 
 *    8.1.2.1 If IGMP length is >= 12, version is 3. Compute max Response time
 *            according to RFC3376's formula.
 *    8.1.2.2 If IGMP length is 8, and max response time is 0, check that
 *            the group address is zero in the IP header, and set version to
 *            1 in that case. Use default value for maxResponseTime (converted
 *            in milliseconds).
 *    8.1.2.3 If IGMP length is 8, and max response time is not 0, version
 *            is v2.
 *    8.1.3 If version is 3 or 2 check that the router alert option is
 *          present (to prevent DOS attack per RFC3376.)
 *    8.1.4 Make sure that IGMPv3 query is contiguous.
 *    8.1.5 Check that version 2, or 3 queries have either the group address
 *          set to 0, or to a multicast address. Convert the max response time
 *          to milliseconds.
 *    8.1.6 Valid contiguous packet: common code for all versions of query:
 *         8.1.6.1 Lock the device.
 *         8.1.6.2 Extract V3 specific fields: qqi, and qrv1, numSrc, and src
 *                 address pointer
 *         8.1.6.3 If we detect a lower version query
 *           8.1.6.3.1 Cancel all pending v3 reports and source timers on the
 *                     interface.
 *           8.1.6.3.2 add or re-initialize (if already added) the device v1/v2
 *                     router detection timer. Set a version flag to prohibit
 *                     sending any higher version packets until that timer
 *                     expires.
 *         8.1.6.4 Process the v1, v2, v3 query
 *           8.1.6.4.1
 *              If query is v3 and we have not detected a lower version router
 *              process the query.
 *                  If query is general, schedule an IGMP device timer at a
 *                  random delay between 0, and maxResponseTime.
 *                  If query is group or group source specific, schedule an
 *                  IGMP timer at a random delay between 0, and
 *                  maxResponseTime. If the request is group specific, clear
 *                  pending recorded sources. If the request is group and 
 *                  source specific, add the sources to the recorded list.
 *           8.1.6.4.2 else v1/v2 query or (v3 query when we have detected a
 *              lower version router. 
 *              loop through all entries in the device list, and for each
 *              entry:
 *              If state is not TM_IGMP_NON_MEMBER and IGMP group address
 *              matches the IGMP entry group address (i.e. either it is 0, or
 *              the same):
 *              . Set the state to TM_IGMP_DELAYING_MEMBER
 *              . Initialize the response time with a random value between 0
 *                and max response time value.
 *              . If there is no timer, add a per IGMP entry timer,
 *                and set the state to TM_IGMP_DELAYING_MEMBER
 *              . Else ther is a timer, re-initialize
 *                the timer if max response time is < current timer.
 *         8.1.6.5 UnLock the device.
 *     8.2 TM_IGMP_V2_REPORT
 *         TM_IGMP_V1_REPORT
 *         Call tfIgmpFindEntry() with the IGMP header group address, to
 *         get the IGMP entry being reported. If entry is found, and
 *         IGMP entry state is TM_IGMP_DELAYING_MEMBER,
 *         delete the timer, reset timer field, clear the last
 *         entry to report flag in the IGMP entry, and set state to
 *         TM_IGMP_IDLE_MEMBER.
 *     8.3 TM_IGMP_V3_REPORT
 *          Ignore on a host (we are not a multicast router) per RFC3376.
 *     8.4 TM_IGMP_LEAVE (version 2 only)
 *         Ignore (we are not a multicast router).
 * 9. If the user uses raw sockets
 *      give the packet to the raw socket, after pointing to the IP header
 *    else
 *      free the packet.
 * 10. Return.
 * Parameters
 * Parameter    Description
 * packetPtr    Pointer to ttPacket containing the IGMP incoming data.
 *              packetPtr->pktLinkDataPtr points to the IP header,
 *              packetPtr->pktTotalLength is the length of the IP packet.
 *              packetPtr->pktSpecDestIpAddr contains the IP multicast
 *              destination address, and packetPtr->pktSrcIpAddr
 *              contains the source address of the IGMP datagram.
 *              packetPtr->pktFlags indicates that the destination
 *              IP address is multicast. packetPtr->pktIpHdrLen contains
 *              the IP header length in bytes
 *              (so that we know whether there are options)
 * Returns
 * NONE
 */
void tfIgmpIncomingPacket(ttPacketPtr packetPtr)
{
    ttIpHeaderPtr     iphPtr;
    ttIgmpHeaderPtr   igmpHdrPtr;
    ttIgmpEntryPtr    igmpEntryPtr;
    ttDeviceEntryPtr  devEntryPtr;
#ifdef TM_USE_IGMPV3
    ttSourceListPtr   srcListPtr;
    tt8BitPtr         optPtr;
    tt4IpAddressPtr   srcAddrPtr;
#endif /* TM_USE_IGMPV3 */
    ttGenericUnion    timerParm1;
    tt4IpAddress      groupIpAddress;
    tt32Bit           maxResponseTime;
    ttPktLen          igmpLength;
    int               version;
#ifdef TM_USE_IGMP_FILTERING
    int               errorCode;
#endif /* TM_USE_IGMP_FILTERING */
    tt16Bit           ipHdrLen;
    tt16Bit           pktChksumResult;
#ifdef TM_USE_IGMPV3
    tt16Bit           numSrc;
    tt8Bit            optDataLen;     /* length of all options */
    tt8Bit            optVal;         /* Option value */
    tt8Bit            optLen;         /* Option length */
    tt8Bit            icmpParam;      /* ICMP error */
    tt8Bit            igmpCode;
    tt8Bit            report;         /* something to report */
#endif /* TM_USE_IGMPV3 */

/* IP header */
    iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
/* IP header length */
    ipHdrLen = packetPtr->pktIpHdrLen;
/* Device packets came in from */
    devEntryPtr = packetPtr->pktDeviceEntryPtr;
/* 1. IGMP packets are not supposed to be exceed the device MTU */
    if (packetPtr->pktChainDataLength > devEntryPtr->devMtu)
    {
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
        goto igmpIncomingExit;
    }
/* 2. Point to IGMP header, update pktLinkDataLength, and pktChainDataLength */
    tm_pkt_ulp_hdr(packetPtr, ipHdrLen);
/* IGMP length from packet/IP information */
    igmpLength = packetPtr->pktChainDataLength;
/* 3. Check that the IGMP length is at least 8 bytes */
    if (igmpLength >= (ttPktLen)(TM_IGMP_MIN_LEN))
    {
/* 4. Save pointer to IGMP header */
        igmpHdrPtr = (ttIgmpHeaderPtr)(packetPtr->pktLinkDataPtr);
/* 5. Check that the IGMP checksum is valid. */
        pktChksumResult = tfPacketChecksum(packetPtr,
                                           igmpLength,
                                           TM_16BIT_NULL_PTR,
                                           TM_16BIT_ZERO);
        if (pktChksumResult == (tt16Bit)0)
        {
#if (defined(TM_USE_DRV_SCAT_RECV) || defined(TM_USE_IGMP_FILTERING))
/*
 * 6. Make the packet contiguous for at least the first 8 bytes of the IGMP
 *    header
 */
            if (tm_pkt_hdr_is_not_cont(packetPtr, TM_IGMP_MIN_LEN))
            {
                packetPtr = tfContiguousSharedBuffer(packetPtr,
                                                     TM_IGMP_MIN_LEN);
                if (packetPtr == (ttPacketPtr)0)
                {
                    goto igmpIncomingExit;
                }
/* Refresh our packet pointers (tfContiguousSharedBuffer may have allocated
 * a larger buffer) */
                igmpHdrPtr = (ttIgmpHeaderPtr)(packetPtr->pktLinkDataPtr);
                iphPtr = (ttIpHeaderPtr)((tt8BitPtr)igmpHdrPtr - ipHdrLen);
            }
#endif /* TM_USE_DRV_SCAT_RECV || TM_USE_IGMP_FILTERING */

#ifdef TM_USE_IGMP_FILTERING
            if (   (tm_context(tvUserIgmpFilterCallback) !=
                                        (ttUserIgmpFilterCallback)0)
                && (devEntryPtr->devFlag2 & TM_DEVF2_IGMP_FILTERING) )
            {
#ifdef TM_USE_IGMP_FILTERING_CONTIGUOUS_DATA
                if (tm_pkt_hdr_is_not_cont(packetPtr, igmpLength))
                {
                    packetPtr = tfContiguousSharedBuffer(packetPtr, igmpLength);
                    if (packetPtr == (ttPacketPtr)0)
                    {
                        goto igmpIncomingExit;
                    }
/* Refresh our packet pointers (tfContiguousSharedBuffer may have allocated
 * a larger buffer) */
                    igmpHdrPtr = (ttIgmpHeaderPtr)(packetPtr->pktLinkDataPtr);
                    iphPtr = (ttIpHeaderPtr)((tt8BitPtr)igmpHdrPtr - ipHdrLen);
                }
#endif /* TM_USE_IGMP_FILTERING_CONTIGUOUS_DATA */
                errorCode = (*(tm_context(tvUserIgmpFilterCallback)))(
                                (ttUserInterface)devEntryPtr,
                                (ttVoidPtr)iphPtr,
                                (ttVoidPtr)igmpHdrPtr,
                                (int)packetPtr->pktChainDataLength);
                if (errorCode != TM_ENOERROR)
                {
                    tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
                    goto igmpIncomingExit;
                }
            }
#endif /* TM_USE_FILTERING */

/* 7. Copy the group address from the header */
            tm_ip_copy(igmpHdrPtr->igmphGroupAddress, groupIpAddress);
            switch (igmpHdrPtr->igmphType)
/* 8. case switch on the IGMP type */
            {
            case TM_IGMP_QUERY:
/*
 * 8.1 IGMP query type. copy the max response time from the IGMP header.
 *     Assume version is invalid.
 */
                maxResponseTime = (tt32Bit)igmpHdrPtr->igmphMaxRespTime;
                version = 0; /* invalid version */
/*
 * 8.1.1
 * RFC3376:
 * "Hosts SHOULD ignore v1, v2 or v3 General Queries sent to a
 * multicast address other than 224.0.0.1, the all-systems address."
 * So acceptable queries are either non general queries (non zero group
 * address) or queries sent to 224.0.0.1
 *
 */
                if (    !(tm_ip_zero(groupIpAddress))
                     || (tm_ip_match(iphPtr->iphDestAddr,
                                     TM_IP_MCAST_ALL_HOSTS )))

                {
/*
 * 8.1.2 Get version and max response time
 * RFC3376: "
 *  "The IGMP version of a Membership Query message is determined as
 *   follows:
 *     IGMPv1 Query: length = 8 octets AND Max Resp Code field is zero
 *     IGMPv2 Query: length = 8 octets AND Max Resp Code field is
 *                   non-zero
 *     IGMPv3 Query: length >= 12 octets
 * Query messages that do not match any of the above conditions
 * (e.g., a Query of length 10 octets) MUST be silently ignored."
 */
#ifdef TM_USE_IGMPV3
                    igmpCode = igmpHdrPtr->igmphMaxRespTime;
                    if (igmpLength >= TM_IGMP_V3_MIN_LEN)
                    {
/* 8.1.2.1 v3. */
                        version = TM_IGMP_V3;
/* Computed as Per RFC3376 */
                        if (igmpCode >= (tt8Bit)128)
/* Max Resp Time = (mant | 0x10) << (exp + 3) */
                        {
/* maxResponseTime in 1/10th second */
                            maxResponseTime = (tt32Bit)
                                    (tm_igmp_mant(igmpCode) | 0x10);
                            maxResponseTime = maxResponseTime
                                     << (tm_igmp_exp(igmpCode) + 3);
                        }
                    }
                    else if (igmpLength == TM_IGMP_MIN_LEN)
#endif /* TM_USE_IGMPV3 */
                    {
                        if (maxResponseTime == TM_UL(0))
                        {
/*
 * Query from a version 1 multicast router:
 * check that the query is general: IGMP header group address
 * igmphGroupAddress is 0.
 * (destination being 224.0.0.1 is already checked as common code.)
 */
                            if (tm_ip_zero(groupIpAddress))
                            {
/* 8.1.2.2 V1 */
                                version = TM_IGMP_V1;
/*
 * Initialize v1 max response time to TM_IGMP_V1_RESP_TIME, and convert it
 * to milliseconds
 */
                                maxResponseTime =
                                    TM_IGMP_V1_RESP_TIME * TM_UL(1000);
                            }
                        }
                        else
                        {
/* 8.1.2.3 Query from a version 2 multicast router */
                            version = TM_IGMP_V2;
                        }
                    }
                }
#ifdef TM_USE_IGMPV3
/* 8.1.3 Check that version 3 queries carry the router alert option */
                if (    (version == TM_IGMP_V3)
                     || (    (version == TM_IGMP_V2)
                          && (tm_16bit_bits_not_set(
                                  devEntryPtr->devFlag2,
                                  TM_DEVF2_NO_IGMPV2_RA)) ) )
/*
 * Query from a version 3 multicast router
 * RFC3376:
 * "Hosts SHOULD ignore v2 or v3 Queries without the Router-Alert option."
 * Note that we do not check the router alert option on IGMPv2 queries
 * because the cisco router does not add that option!! We would end up
 * throwing away the IGMPv2 queries.
 */
                {
/* length of all options */
                    optDataLen = (tt8Bit)
                                       (ipHdrLen -
                                        tm_byte_count(TM_4PAK_IP_MIN_HDR_LEN));
/* point to options */
                    optPtr = (tt8BitPtr)(
                            (ttCharPtr)iphPtr + TM_4PAK_IP_MIN_HDR_LEN );
                    optVal = TM_8BIT_ZERO;
                    icmpParam = TM_8BIT_ZERO;
                    while (optDataLen != 0)
                    {
/* Get the next option */
                        optPtr = tfIpGetNextOpt(optPtr, &optDataLen,
                                                &optVal, &optLen,
                                                &icmpParam,
                                                (tt8Bit)ipHdrLen);
                        if (icmpParam != TM_8BIT_ZERO)
/* Error in option length field */
                        {
                            break;
                        }
                        if (optVal == TM_IP_OPT_RA)
                        {
                            if (optLen != TM_IP_OPT_RA_LEN)
                            {
/* Router alert option lenght is incorrect */
                                optVal = TM_8BIT_ZERO;
                            }
                            break;
                        }
/* Point to the next option */
                        optPtr += optLen;
                    }
                    if (optVal != TM_IP_OPT_RA)
                    {
                        version = 0; /* Invalid packet */
                    }
                }
#ifdef TM_USE_DRV_SCAT_RECV
                if (version == TM_IGMP_V3)
/* 8.1.4 Make sure that IGMPv3 query is contiguous.*/
                {
/* Make sure the whole packet is contiguous */
                    if (tm_pkt_hdr_is_not_cont(packetPtr,
                                               igmpLength))
                    {
/* Make sure the whole packet is contiguous */
                        packetPtr = tfContiguousSharedBuffer(packetPtr,
                                                             igmpLength);
                        if (packetPtr == (ttPacketPtr)0)
                        {
                            goto igmpIncomingExit;
                        }
/* Refresh our packet pointers (tfContiguousSharedBuffer may have allocated
 * a larger buffer) */
                        igmpHdrPtr = (ttIgmpHeaderPtr)
                                        (packetPtr->pktLinkDataPtr);
                        iphPtr = (ttIpHeaderPtr)
                                        ((tt8BitPtr)igmpHdrPtr - ipHdrLen);
                    }
                }
#endif /* TM_USE_DRV_SCAT_RECV */
#endif /* TM_USE_IGMPV3 */
                if (version > TM_IGMP_V1)
                {
/*
 * 8.1.5 Check that version 2, or 3 queries have either the group address
 * set to 0, or to a multicast address
 */
                    if (   (    tm_ip_zero(groupIpAddress)
                            || tm_ip_is_multicast(groupIpAddress) ) )
/* General query or group specific */
                    {
/* Change v2/v3 maxResponseTime from 1/10th seconds to milliseconds */
                        maxResponseTime *= TM_UL(100); /* In milliseconds */
                    }
                    else
/* Invalid query */
                    {
                        version = 0;
                    }
                }
                if (version != 0)
/* 8.1.6 Valid contiguous packet */
                {
/* 8.1.6.1 Lock the device */
                    tm_call_lock_wait(&(devEntryPtr->devLockEntry));
#ifdef TM_USE_IGMPV3
                    srcAddrPtr = (tt4IpAddressPtr)0;
                    numSrc = TM_16BIT_ZERO;
                    if (version == TM_IGMP_V3)
                    {
/* 8.1.6.2 Process V3 query fields
 * Extract QRV, and QQI values, numSrc, and src address pointer
 */
/* Robustness variable as given by the router */
                        igmpCode = igmpHdrPtr->igmphQrv;
                        igmpCode = tm_igmp_qrv(igmpCode);
                        if (    (igmpCode != TM_8BIT_ZERO)
                             && (igmpCode <= (tt8Bit)7))
/* If not zero, and less than 7, use that instead of our default */
                        {
                            devEntryPtr->devIgmpRobustness = igmpCode;
                        }
                        else
                        {
                            devEntryPtr->devIgmpRobustness =
                                                 TM_IGMP_DEF_ROBUSTNESS;
                        }
/* Querier Query Interval Code */
                        igmpCode = igmpHdrPtr->igmphQqic;
/* Computed as Per RFC3376 */
                        if (igmpCode >= (tt8Bit)128)
/* Qqi = (mant | 0x10) << (exp + 3) */
                        {
                            devEntryPtr->devIgmpQqi = (tt16Bit)
                                    (tm_igmp_mant(igmpCode) | 0x10);
                            devEntryPtr->devIgmpQqi = (tt16Bit)
                                    (   devEntryPtr->devIgmpQqi
                                     << (tm_igmp_exp(igmpCode) + 3));
                        }
                        else
                        {
                            devEntryPtr->devIgmpQqi = igmpCode;
                        }
                        if (devEntryPtr->devIgmpQqi == 0)
                        {
                            devEntryPtr->devIgmpQqi = TM_IGMP_QUERY_INTERVAL;
                        }
                        tm_ntohs(igmpHdrPtr->igmphNumSrc, numSrc);
                        if (numSrc != TM_16BIT_ZERO)
                        {
                            srcAddrPtr = &igmpHdrPtr->igmphSrcAddress[0];
                        }
                    }
#endif /* TM_USE_IGMPV3 */
                    if (version < TM_IGMP_MAX_VERSION)
/* 8.1.6.3 Detection of a lower verion timer */
                    {
#ifdef TM_USE_IGMPV3
                        if (devEntryPtr->devIgmpVTmrPtr[version - 1]
                                            == (ttTimerPtr)0)
                        {
/*
 * 8.1.6.3.1
 * RFC3376: cancel pending reports and retransmission timers when detecting a
 * different version router on the interface
 */
                            tfIgmpCancelDevPendingReports(devEntryPtr);
                        }
#endif /* TM_USE_IGMPV3 */
/*
 * 8.1.6.3.2
 * add or re-initialize (if already added) the device v1/v2
 * router detection timer
 */
                        tfIgmpAddVTimer(devEntryPtr,
#ifdef TM_USE_IGMPV3
                                        maxResponseTime,
#endif /* TM_USE_IGMPV3 */
                                        version);
                    }
/* 8.1.6.4 Process the v1/v2/v3 query */
#ifdef TM_USE_IGMPV3
                    if  (    (version == TM_IGMP_V3)
                          && (tm_8bit_bits_not_set(
                                           devEntryPtr->devIgmpVFlag,
                                           (TM_IGMP_DEV_V1 | TM_IGMP_DEV_V2)))
                        )
/* V3 query and we are allowed to process it */
                    {
                        if (tm_ip_zero(groupIpAddress))
/* General query */
                        {
                            if (numSrc == TM_16BIT_ZERO)
/*
 *  1. If there is a pending response to a previous General Query
 *     scheduled sooner than the selected delay, no additional response
 *     needs to be scheduled.
 *
 *  2. If the received Query is a General Query, the interface timer is
 *     used to schedule a response to the General Query after the
 *     selected delay.  Any previously pending response to a General
 *     Query is canceled.
 */
                            {
                                report = tfIgmpSomethingToReport(devEntryPtr);
                                if (report)
                                {
/* Something to send */
                                    timerParm1.genVoidParmPtr =
                                                     (ttVoidPtr)devEntryPtr;
                                    tfIgmpAddRandomTimer(
                                            &devEntryPtr->devIgmpQueryTmrPtr,
                                            tfIgmpGeneralQueryTimer,
                                            timerParm1,
                                            maxResponseTime);
                                }
                            }
                        }
                        else
                        {
/*
 *  3. If the received Query is a Group-Specific Query or a Group-and-
 *     Source-Specific Query and there is no pending response to a
 *     previous Query for this group, then the group timer is used to
 *     schedule a report.  If the received Query is a Group-and-Source-
 *     Specific Query, the list of queried sources is recorded to be used
 *     when generating a response.
 *
 *  4. If there already is a pending response to a previous Query
 *     scheduled for this group, and either the new Query is a Group-
 *     Specific Query or the recorded source-list associated with the
 *     group is empty, then the group source-list is cleared and a single
 *     response is scheduled using the group timer.  The new response is
 *     scheduled to be sent at the earliest of the remaining time for the
 *     pending report and the selected delay.
 *
 *  5. If the received Query is a Group-and-Source-Specific Query and
 *     there is a pending response for this group with a non-empty
 *     source-list, then the group source list is augmented to contain
 *     the list of sources in the new Query and a single response is
 *     scheduled using the group timer.  The new response is scheduled to
 *     be sent at the earliest of the remaining time for the pending
 *     report and the selected delay.
 */
                            igmpLength -= TM_IGMP_V3_MIN_LEN;
                            if ((ttPktLen)tm_source_len(numSrc) == igmpLength)
/*
 * Check that the IGMP query length is consistant with the number of sources
 * claimed
 */
                            {
                                igmpEntryPtr = tfIgmpFindEntry(groupIpAddress,
                                                               devEntryPtr);
                                if (    (igmpEntryPtr != (ttIgmpEntryPtr)0)
                                     && (igmpEntryPtr->igmpState !=
                                                             TM_IGMP_LOCAL))
                                {
                                    if (    (srcAddrPtr == (tt4IpAddressPtr)0)
/* New group specific query */
                                         ||
/*
 * or pending group specific query (recorded source-list is empty with a
 * pending timer)
 */
                                            (   (igmpEntryPtr->igmpTimerPtr[
                                                     TM_IGMP_QUERY_TIMER]
                                                           != TM_TMR_NULL_PTR)
                                              && (igmpEntryPtr->igmpSrcList[
                                                  TM_IGMP_RECORDED_LIST].
                                                  srcCount == TM_16BIT_ZERO)
                                            )
                                       )
                                    {
                                        srcListPtr =
                                            &igmpEntryPtr->igmpSrcList[
                                                        TM_IGMP_CURRENT_LIST];
/* Free previously recorded list */
                                        tfIgmpFreeSourceList(
                                                &igmpEntryPtr->igmpSrcList[
                                                    TM_IGMP_RECORDED_LIST]);
                                    }
                                    else
                                    {
/* Build the recorded list merging the queried sources with our current list */
                                        tfIgmpBuildRecordedList(igmpEntryPtr,
                                                                srcAddrPtr,
                                                                numSrc);
                                        srcListPtr =
                                            &igmpEntryPtr->igmpSrcList[
                                                        TM_IGMP_RECORDED_LIST];
                                    }
                                    if (   (srcListPtr->srcMode
                                                         == TM_MODE_IS_EXCLUDE)
                                         || (srcListPtr->srcCount
                                                             != TM_16BIT_ZERO)
                                       )
/* No error, and something to send */
                                    {
                                        timerParm1.genVoidParmPtr =
                                                     (ttVoidPtr)igmpEntryPtr;
                                        tfIgmpAddRandomTimer(
                                           &igmpEntryPtr->igmpTimerPtr[
                                                        TM_IGMP_QUERY_TIMER],
                                           tfIgmpGroupQueryTimer,
                                           timerParm1,
                                           maxResponseTime);
                                    }
                                }
                            }
                        }
                    }
                    else
#endif /* TM_USE_IGMPV3 */
                    {
/* V1 or V2 query (or V3 query when a lower version router is detected) */
                        igmpEntryPtr = devEntryPtr->devIgmpPtr;
                        while (igmpEntryPtr != (ttIgmpEntryPtr)0)
                        {
                            if (   (    tm_ip_zero(groupIpAddress)
                                     || tm_ip_match(
                                                 igmpEntryPtr->igmpHostGroup,
                                                 groupIpAddress) )
                                 && (    igmpEntryPtr->igmpState
                                      > TM_IGMP_JOIN_PENDING ) )
/*
 * Query is non specific, or found a specific query match, and state is
 * either delaying member or idle member
 */
                            {
                                igmpEntryPtr->igmpState =
                                                       TM_IGMP_DELAYING_MEMBER;
/*
 * If there is no v1/v2 delaying timer, add a per IGMP entry timer
 * Else, reset the timer if max response time is < current timer.
 */
                                timerParm1.genVoidParmPtr =
                                                       (ttVoidPtr)igmpEntryPtr;
                                tfIgmpAddRandomTimer(
                                    &igmpEntryPtr->igmpTimerPtr[TM_IGMP_TIMER],
                                    tfIgmpTimer,
                                    timerParm1,
                                    maxResponseTime);
                            }
/* Move to next group on this device */                        
                            igmpEntryPtr = igmpEntryPtr->igmpNextPtr;
                        }
                    }
/* 8.1.6.5 UnLock the device */
                    tm_call_unlock(&(devEntryPtr->devLockEntry));
                }
                break;
            case TM_IGMP_V2_REPORT:
            case TM_IGMP_V1_REPORT:
/*
 * 8.2 Call tfIgmpFindEntry() with the IGMP header group address, to
 * get the IGMP entry being reported.
 */
                tm_call_lock_wait(&(devEntryPtr->devLockEntry));
                igmpEntryPtr = tfIgmpFindEntry(groupIpAddress,
                                               devEntryPtr);
                if (igmpEntryPtr != (ttIgmpEntryPtr)0)
/* Entry found */
                {
                    tfIgmpTimerRemove(igmpEntryPtr, TM_IGMP_TIMER);
/* Other host last report */
                    igmpEntryPtr->igmpFlag = TM_IGMP_OTHER_GROUP_REPORT;
                }
                tm_call_unlock(&(devEntryPtr->devLockEntry));
                break;
#ifdef TM_USE_IGMPV3
            case TM_IGMP_V3_REPORT:
/* 8.3 Ignore on a host */
#endif /* TM_USE_IGMPV3 */
            case TM_IGMP_LEAVE:
/* 8.4 Ignore on a host */
            default:
                break;
            }
        }
    }
/* 9. Pass up the packet to a raw socket or free it */
#ifdef TM_USE_RAW_SOCKET
/* Point to IP header */
    tm_pkt_llp_hdr(packetPtr, ipHdrLen);
    tf4RawIncomingPacket(packetPtr);
#else /* !TM_USE_RAW_SOCKET */
    tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
#endif /* TM_USE_RAW_SOCKET */
igmpIncomingExit:
    return;
}

/*
 * tfIgmpAddRandomTimer
 * Function description
 * Add a timer computing a random timeout.
 * . Initialize timeout with a random number between 0, an maxResponseTime.
 * . If timer has not been added, add it so that it expires within that
 *   timeout value.
 * . otherwise, checks that it expires within maxResponseTime. If not update
 *   with the new random timeout.
 * . return
 * Parameters
 * Parameter        Description
 * timerPtrPtr          Pointer to Pointer to timer structure (updated)
 * callBackFunctionPtr  Timer call back function
 * timerParm1           Timer parameter 1
 * maxResponseTime      maximum timeout in milliseconds
 * Returns
 * None
 */
static void tfIgmpAddRandomTimer (
    ttTimerPtrPtr  timerPtrPtr,
    ttTmCBFuncPtr  callBackFunctionPtr,
    ttGenericUnion timerParm1,
    tt32Bit        maxResponseTime)
{
    tt32Bit           timeout;
    tt32Bit           timeRemaining;

    timeout = tfIgmpRandom(maxResponseTime);
    if (*timerPtrPtr == (ttTimerPtr)0 )
/* No timer was added. Add a new one */
    {
        *timerPtrPtr = tfTimerAdd( callBackFunctionPtr,
                                   timerParm1,
                                   timerParm1, /* unused */
                                   timeout,
                                   TM_TIM_AUTO);
    }
    else
    {
        timeRemaining = tfTimerMsecLeft(*timerPtrPtr);
        if (maxResponseTime < timeRemaining)
        {
/*
 * Refresh the timer timeout to the new smaller value if the new timer is set
 * to expire sooner
 */
            tm_timer_new_time(*timerPtrPtr,
                              timeout);
        }
    }
    return;
}

/*
 * tfIgmpAddVTimer Function description
 * Add a version timer to keep track of routers'versions.
 * Add or re-initialize (if already added) the device v1/v2
 * router detection timer. Set a version flag to prohibit
 * sending any higher version packets until that timer
 * expires.
 * . Compute the timeout:
 *   .. IGMPV3: Use RFC3376 formula.
 *   .. Non IGMPv3: Use tvIgmpRouterV1Timeout
 * . If a timer does not exist for this version, add it, otherwise refresh
 *   the timeout value.
 * . Set the version flag so that we do not send higher version reports.
 * Parameters 
 * Parameter                Description
 * devEntryPtr              Device on which to add the version timer
 * queryResponseInterval    IGMPv3: used to compute the timeout value
 * version                  router IGMP version detected
 * Return
 * none
 */
static void tfIgmpAddVTimer(ttDeviceEntryPtr devEntryPtr,
#ifdef TM_USE_IGMPV3
                            tt32Bit          queryResponseInterval,
#endif /* TM_USE_IGMPV3 */
                            int              version)
{
    ttGenericUnion    timerParm1;
    ttGenericUnion    timerParm2;
    tt32Bit           timeout;
    int               timerIndex;

#ifdef TM_USE_IGMPV3
/* Computed as Per RFC3376:
 * "This value MUST be ((the Robustness Variable) times (the Query
 *  Interval in the last Query received)) plus (one Query Response
 *  Interval)".
 */
    timeout = (tt32Bit)
              (  (tt32Bit)(  ((tt32Bit)devEntryPtr->devIgmpRobustness)
                           * ((tt32Bit)(((tt32Bit)(devEntryPtr->devIgmpQqi))
                                                   * TM_UL(1000))) )
               + (tt32Bit)(queryResponseInterval) );
#else /* !TM_USE_IGMPV3 */
    timeout = (tt32Bit)tm_context(tvIgmpRouterV1Timeout) * TM_UL(1000);
#endif /* !TM_USE_IGMPV3 */
    timerIndex = version - 1;
    if (devEntryPtr->devIgmpVTmrPtr[timerIndex] == (ttTimerPtr)0)
/* No previous version timer for this version. Add the timer */
    {
        timerParm1.genVoidParmPtr = (ttVoidPtr) devEntryPtr;
        timerParm2.genIntParm = version;
        devEntryPtr->devIgmpVTmrPtr[timerIndex] =
                                tfTimerAdd(tfIgmpVTimer,
                                           timerParm1,
                                           timerParm2,
                                           timeout,
                                           TM_TIM_AUTO);
    }
    else
    {
/* previous version timer for this version. Refresh the timer */
        tm_timer_new_time (devEntryPtr->devIgmpVTmrPtr[timerIndex],
                           timeout );
    }
/* Set flag to prohibit sending higher version reports */
    devEntryPtr->devIgmpVFlag |= (tt8Bit)(1 << version);
    return;
}

/*
 * tfIgmpSetSockOpt function description:
 * Function called from setsockopt() for an IGMP IP_PROTOIP level socket
 * option.
 * Common call for basic IGMP APIs.
 * 1. If protocol is TCP, return TM_EPROTOTYPE 
 * 2. Initialize fmode -1
 * 3. switch on optName:
 * 3.1 IPO_MULTICAST_TTL specify the IP ttl for outgoing multicast datagrams
 *     argument type is at least an unsigned char
 * 3.1,1. verify that option length is sizeof(unsigned char)
 * 3.1.2. copy argument to socIpMcastTtl.
 *
 * 3.2 IPO_MULTICAST_IF:
 * specify configured IP address of outgoing interface for multicast
 * UDP datagrams sent on this socket. A zero IP address parameter
 * indicates that the user want to reset the multicast socket outgoing
 * interface.
 * data type: struct in_addr
 * 3.2.1. Verify that option length is at least sizeof(struct in_addr)
 * 3.2.2. If address option is zero IP address, set socMcastDevPtr to null pointer.
 * 3.2.3. otherwise find matching interface:
 * 3.2.4. If valid interface, set in socMcastDevPtr, and socMcastMhomeIndex
 * 3.2.5. If invalid interface return TM_ADDRNOTAVAIL.
 *
 * 3.3 IPO_DROP_MEMBERSHIP/IPO_ADD_MEMBERSHIP:
 * 3.3.1. Verify that optionLength is sizeof(struct ip_mreq) otherwise exit
 *       with TM_EINVAL error code.
 * 3.3.2. Copy mcast address, and interface address from user structure.
 * 3.3.3. Set fmode to 0.
 *
 * 3.4 IGMPV3: IPO_BLOCK_SOURCE/IPO_UNBLOCK_SOURCE/IPO_ADD_SOURCE_MEMBERSHIP/
 *             IPO_DROP_SOURCE_MEMBERSHIP:
 * 3.4.1. Verify that optionLength is sizeof(struct ip_mreq_source) otherwise
 *        exit with TM_EINVAL error code.
 * 3.4.2. Copy mcast address, and interface address from user structure.
 * 3.4.3. Point to user source address in user structure
 * 3.4.3. Set numSrc to 1.
 * 3.4.4. Set fmode to 0.
 *
 * 4. If fmode it not -1 (common code for IPO_DROP_MEMBERSHIP/IPO_ADD_MEMBERSHIP
 *    + IGMPV3: IPO_BLOCK_SOURCE/IPO_UNBLOCK_SOURCE/IPO_ADD_SOURCE_MEMBERSHIP/
 *    IPO_DROP_SOURCE_MEMBERSHIP):
 *  4.1 If optionName is IPO_ADD_SOURCE_MEMBERSHIP/IPO_DROP_SOURCE_MEMBERSHIP/
 *      IPO_DROP_MEMBERSHIP set fmode to INCLUDE (IGMPV3 only.)
 *  4.2 else fmode is EXCLUDE (IGMPV3 only.)
 *  4.3 Call tfIgmpUserParameters to copy the user parameter to our own
 *      internal mcast request structure, verifiying the parameters, and
 *      mapping interface address to device entry pointer/mhome index.
 *  4.4 If we verified the parameters successfully
 *   4.4.1 Increase the number of user API pending calls.
 *   4.4.2 Call common tfIgmpSetSockMembership()
 *   4.4.3 Call common tfIgmpSocketJoinReturn() to decrease the number of
 *         user API pending calls, and clean up if the socket had been
 *         closed during the tfIgmpSetSockMembership() call.
 * 5. Return errorCode
 *
 * Parameters
 * Parameter            Description
 * socketEntryPtr       Pointer to socket entry
 * optionName           option name (IPO_ADD_MEMBERSHIP, etc..)
 * optionValuePtr       Pointer to user structure
 * optionLength         User structure length
 *
 * Returns
 * Return value     meaning
 * TM_ENOERROR      success
 * TM_EINVAL        Invalid option length
 * IPO_MULTICAST_IF
 * TM_EADDRNOTAVAIL Invalid interface IP address
 * IPO_ADD_MEMBERSHIP
 * Return value     meaning
 * TM_EADDRNOTAVAIL Invalid interface IP address
 * TM_EADDRINUSE    This host group has already been added to the interface.
 * TM_ENOBUFS       Not enough memory to store the new entry.
 * IPO_DROP_MEMBERSHIP
 * Return value     meaning
 * TM_EADDRNOTAVAIL Invalid interface IP address
 * TM_EADDRNOTAVAIL This host group had not been added to the interface
 * IPO_ADD_SOURCE_MEMBERSHIP:
 * Return value     meaning
 * TM_EADDRNOTAVAIL Invalid interface IP address
 * TM_EINVAL        Filter mode incommpatible with previous one
 * TM_EADDRNOTAVAIL Invalid source address
 * TM_EADDRNOTAVAIL Adding a source on a local mcast group
 * TM_ENOBUFS        Not enough memory to store the new entry.
 * IPO_DROP_SOURCE_MEMBERSHIP:
 * Return value     meaning
 * TM_EADDRNOTAVAIL Invalid interface IP address
 * TM_EINVAL        Filter mode incommpatible with previous one
 * TM_EADDRNOTAVAIL Invalid source address
 * TM_EADDRNOTAVAIL This host group had not been added to the interface
 * IPO_BLOCK_SOURCE:
 * IPO_UNBLOCK_SOURCE:
 * Return value     meaning
 * TM_EINVAL        Filter mode incommpatible with previous one
 * TM_EADDRNOTAVAIL Invalid interface IP address
 * TM_EADDRNOTAVAIL Invalid source address
 * TM_EADDRNOTAVAIL Adding a source on a local mcast group
 * TM_EADDRNOTAVAIL This host group had not been added to the interface
 */
int tfIgmpSetSockOpt ( ttSocketEntryPtr   socketEntryPtr,
                       int                optionName,
                       ttConstCharPtr     optionValuePtr,
                       int                optionLength )
{
    ttDeviceEntryPtr        devEntryPtr;
    ttIpMreqPtr             imrPtr;
    ttIpMreqSrcPtr          imrIntPtr;
#ifdef TM_USE_IGMPV3
    ttIpMreqSourcePtr       imrsPtr;
    struct in_addr TM_FAR * sourceAddrPtr;
#endif /* TM_USE_IGMPV3 */
    tt4IpAddress            ipAddress;
    tt4IpAddress            ifIpAddr;
    ttIpMreqSrc             imrIntStruct;
    int                     fmode;
#ifdef TM_USE_IGMPV3
    int                     numSrc;
#endif /* TM_USE_IGMPV3 */
    int                     errorCode;
    tt16Bit                 mhomeIndex;

    if (socketEntryPtr->socProtocolNumber == (tt8Bit)IP_PROTOTCP)
/* Need to check here for IPO_MULTICAST_TTL and IPO_MULTICAST_IF */
    {
        errorCode = TM_EPROTOTYPE;
    }
    else
    {
        fmode = -1;
        mhomeIndex = TM_16BIT_ZERO;
        errorCode = TM_ENOERROR;
        imrIntPtr = &imrIntStruct;
        tm_ip_copy(TM_IP_ZERO, ipAddress); /* compiler */
        tm_ip_copy(TM_IP_ZERO, ifIpAddr); /* compiler */
#ifdef TM_USE_IGMPV3
        numSrc = 0; /* compiler */
        sourceAddrPtr = (struct in_addr TM_FAR *)0; /* compiler */
#endif /* TM_USE_IGMPV3 */
        switch (optionName)
        {
            case IPO_MULTICAST_TTL:
/* IPO_MULTICAST_TTL  specify the IP ttl for outgoing multicast datagrams */
                if (optionLength < (int)sizeof(unsigned char))
                {
/* Verify that option length is sizeof(unsigned char) */
                    errorCode = TM_EINVAL;
                }
                else
                {
/* Copy argument to socIpMcastTtl. */
                    socketEntryPtr->socIpMcastTtl =
                                 *((unsigned char TM_FAR *)(optionValuePtr));
                }
                break;
            case IPO_MULTICAST_IF:
                if (optionLength < (int)sizeof(struct in_addr))
                {
/* Verify that option length is at least sizeof(struct in_addr) */
                    errorCode = TM_EINVAL;
                }
                else
                {
                    tm_ip_copy(
                        ((struct in_addr TM_FAR *)(optionValuePtr))->s_addr,
                        ipAddress );
                    if (tm_ip_zero(ipAddress))
                    {
/* If address option is zero IP address, set socMcastDevPtr to null pointer */
                        socketEntryPtr->socMcastDevPtr = (ttDeviceEntryPtr)0;
                    }
                    else
                    {
/* Otherwise find matching interface */
#ifdef TM_SINGLE_INTERFACE_HOME
                        devEntryPtr = tfIfaceConfig(0);
                        if ( devEntryPtr != (ttDeviceEntryPtr)0 )
                        {
                            if ( !tm_ip_match( ipAddress,
                                           tm_ip_dev_addr(devEntryPtr, 0)) )
                            {
                                devEntryPtr = (ttDeviceEntryPtr)0;
                            }
                        }
                        mhomeIndex = TM_16BIT_ZERO;
#else /* TM_SINGLE_INTERFACE_HOME */
                        devEntryPtr = tfIfaceMatch(ipAddress, tfMhomeAddrMatch,
                                                   &mhomeIndex);
#endif /* TM_SINGLE_INTERFACE_HOME */
                        if ( devEntryPtr == (ttDeviceEntryPtr)0 )
                        {
/* Invalid interface, return TM_EADDRNOTAVAIL */
                            errorCode = TM_EADDRNOTAVAIL;
                        }
                        else
                        {
/* If valid interface, set in socMcastDevPtr, and socMcastMhomeIndex */
                            socketEntryPtr->socMcastDevPtr = devEntryPtr;
                            socketEntryPtr->socMcastMhomeIndex = mhomeIndex;
                        }
                    }
                }
                break;
            case IPO_ADD_MEMBERSHIP:
            case IPO_DROP_MEMBERSHIP:
/* Check the option length */
                if (optionLength != (int)sizeof(struct ip_mreq))
                {
/* Verify that optionLength is sizeof(struct ip_mreq) */
                    errorCode = TM_EINVAL;
                }
                else
                {
/* 
 * Copy user parameters in a version independant fashion.
 * (Note that imr_sourceaddr is inserted before imr_interface in IGMPV3, so
 * we cannot use common code to extract imr_interface.)
 */
                    imrPtr = (ttIpMreqPtr)(ttVoidPtr)optionValuePtr;
                    tm_ip_copy(imrPtr->imr_multiaddr.s_addr, ipAddress);
                    tm_ip_copy(imrPtr->imr_interface.s_addr, ifIpAddr);
                    fmode = 0;
                }
                break;
#ifdef TM_USE_IGMPV3
            case IPO_BLOCK_SOURCE:
            case IPO_UNBLOCK_SOURCE:
            case IPO_ADD_SOURCE_MEMBERSHIP:
            case IPO_DROP_SOURCE_MEMBERSHIP:
                if (optionLength != (int)sizeof(struct ip_mreq_source))
                {
/* Verify that optionLength is sizeof(struct ip_mreq_source) */
                    errorCode = TM_EINVAL;
                }
                else
                {
                    imrsPtr = (ttIpMreqSourcePtr)(ttVoidPtr)optionValuePtr;
/* 
 * Copy user parameters in a version independant fashion.
 * (Note that imr_sourceaddr is inserted before imr_interface in IGMPV3, so
 * we cannot use common code to extract imr_interface.)
 */
                    tm_ip_copy(imrsPtr->imr_multiaddr.s_addr, ipAddress);
                    tm_ip_copy(imrsPtr->imr_interface.s_addr, ifIpAddr);
                    sourceAddrPtr = &imrsPtr->imr_sourceaddr;
                    numSrc = 1;
                    fmode = 0;
                }
                break;
#endif /* TM_USE_IGMPV3 */
            default:
                break;
        }
        if (fmode != -1)
/* 
 * Common code for membership setsockopt: 
 * IPO_ADD_MEMBERSHIP, IPO_DROP_MEMBERSHIP,
 * IPO_BLOCK_SOURCE, IPO_UNBLOCK_SOURCE,
 * IPO_ADD_SOURCE_MEMBERSHIP, IPO_DROP_SOURCE_MEMBERSHIP
 */
        {
#ifdef TM_USE_IGMPV3
            if (    (optionName == IPO_ADD_SOURCE_MEMBERSHIP)
                 || (optionName == IPO_DROP_SOURCE_MEMBERSHIP)
                 || (optionName == IPO_DROP_MEMBERSHIP))
            {
                fmode = MCAST_INCLUDE;
            }
            else
            {
                fmode = MCAST_EXCLUDE;
            }
#endif /* TM_USE_IGMPV3 */
/* Copy and verify user parameters */
            errorCode = tfIgmpUserParameters(
                                        imrIntPtr, socketEntryPtr, 
                                        ipAddress,
                                        ifIpAddr,
#ifdef TM_USE_IGMPV3
                                        sourceAddrPtr,
                                        numSrc,
                                        fmode,
#endif /* TM_USE_IGMPV3 */
                                        optionName);
            if (errorCode == TM_ENOERROR)
            {
/* 
 * Increase number of joins pending on this socket, to prevent a close from
 * accessing IGMP while the socket IGMP cache and stack IGMP cache are not
 * in SYNC.
 */
                socketEntryPtr->socIgmpJoinsPending++;
/* Socket potentially unlocked/relocked */
                errorCode = tfIgmpSetSockMembership(imrIntPtr);
/* 
 * Return from membership call. Socket cache and IGMP cache now in SYNCH.
 * Make sure to purge socket IGMP info cache if socket had been close.
 */
                tfIgmpSocketJoinReturn(socketEntryPtr);
            }
        }
    }
    return errorCode;
}

/*
 * tfIgmpUserParameters Function Description
 * IGMP user APIs common parameters' check and initialization 
 * Called from tfIgmpSetSockopt(), when setsockopt optionName is
 * IPO_DROP_MEMBERSHIP/IPO_ADD_MEMBERSHIP + IGMPV3: IPO_BLOCK_SOURCE/
 * IPO_UNBLOCK_SOURCE/IPO_ADD_SOURCE_MEMBERSHIP/IPO_DROP_SOURCE_MEMBERSHIP
 * Also called from tfComGetipv4sourcefilter() and tfComSetipv4sourcefilter()
 *
 * . If socket's protocol is TCP exit with TM_EPROTOTYPE error
 * . If socket is closing exit with TM_EBADF error
 * . If multiAddr (multicast address) is not a class D address,
 *   exit with TM_EADDRNOTAVAIL error code
 * . Copy parameters to internal mcast request structure.
 * . Translate MCAST_INCLUDE to TM_MODE_IS_INCLUDE, and MCAST_EXCLUDE to
 *   TM_MODE_IS_EXCLUDE.
 * . Call tfIgmpSetSockInterface(imrIntPtr); to map IPv4 address to
 *   devEntryPtr / multi home
 * . Call tfIgmpCheckValidSrcAddr() to check that the source addresses are
 *   valid.
 * . Return error to the user.
 * Parameters
 * Parameter        Description
 * imrIntPtr        Pointer to internal mcast request structure conveying
 *                  parameters set by the user in the user API.
 * socketEntryPtr   Pointer to socket entry
 * multiAddr        IPv4 multicast group address.
 * interfaceAddr    Interface IPv4 address
 * Returns
 * Value              Meaning
 * TM_ENOERROR      no error
 * TM_EADDRNOTAVAIL Host group address is not multicast
 *                  or user trying to add source(s) to a local mcast address
 *                  or could not map interface address to a device
 *                  or source address is invalid
 */
int tfIgmpUserParameters(ttIpMreqSrcPtr   imrIntPtr,
                         ttSocketEntryPtr socketEntryPtr, 
                         tt4IpAddress     multiAddr,
                         tt4IpAddress     interfaceAddr,
#ifdef TM_USE_IGMPV3
                  struct in_addr TM_FAR * srcAddrPtr,
                         int              numSrc,
                         int              fmode,
#endif /* TM_USE_IGMPV3 */
                         int              optionName)
{
    int errorCode;

    if (socketEntryPtr->socProtocolNumber == (tt8Bit)IP_PROTOTCP)
/* Socket not SOCK_DGRAM and not SOCK_RAW */
    {
        errorCode = TM_EPROTOTYPE;
    }
    else if (tm_16bit_one_bit_set(socketEntryPtr->socFlags,
                                  TM_SOCF_CLOSING))
/* Socket in the process of being closed. Socket mcast cache cannot be used. */
    {
        errorCode = TM_EBADF;
    }
    else
    {
/* Check that the address is multicast */
        if (!(tm_ip_is_class_d(multiAddr)))
        {
            errorCode = TM_EADDRNOTAVAIL;
        }
        else
        {
/* Initialize the structure */
            tm_bzero(imrIntPtr, sizeof(ttIpMreqSrc));
            imrIntPtr->imrSocketPtr = socketEntryPtr;
            tm_ip_copy(multiAddr, imrIntPtr->imrMcastAddr);
            tm_ip_copy(interfaceAddr, imrIntPtr->imrIfAddr);
            imrIntPtr->imrOptionName = optionName;
#ifdef TM_USE_IGMPV3
/*
 * To guard against a user changing the macros in trsocket.h, use protocol
 * macros that cannot be changed, as those macros are used to index an array.
 */
            if (fmode == MCAST_EXCLUDE)
            {
                fmode = TM_MODE_IS_EXCLUDE;
            }
            else
            {
                fmode = TM_MODE_IS_INCLUDE;
            }
            imrIntPtr->imrSrcAddrPtr = srcAddrPtr;
            imrIntPtr->imrNumSrc = numSrc;
            imrIntPtr->imrFmode = fmode;
            if ((numSrc != 0) && tm_ip_is_local_mcast_address(multiAddr))
/* Cannot use source address membership with a local multi group address */
            {
                errorCode = TM_EADDRNOTAVAIL;
            }
            else
            {
                errorCode = TM_ENOERROR;
            }
            if (errorCode == TM_ENOERROR)
#endif /* TM_USE_IGMPV3 */
            {
/*
 * Find the interface corresponding to interfaceAddr
 * Socket unlocked, and relocked
 */
                errorCode = tfIgmpSetSockInterface(imrIntPtr);
#ifdef TM_USE_IGMPV3
#ifdef TM_IP_SRC_ERROR_CHECKING
                if (    (errorCode == TM_ENOERROR)
                     && (imrIntPtr->imrSrcAddrPtr
                                     != (struct in_addr TM_FAR *)0)
                   )
                {
                    errorCode = tfIgmpCheckValidSrcAddr(imrIntPtr);
                }
#endif /* TM_IP_SRC_ERROR_CHECKING */
#endif /* TM_USE_IGMPV3 */
            }
        }
    }
    return errorCode;
}

/* 
 * tfIgmpSetSockMembership Function Description
 * . Common code for membership setsockopt after user parameters' check:
 *   IPO_ADD_MEMBERSHIP, IPO_DROP_MEMBERSHIP,
 *   IPO_BLOCK_SOURCE, IPO_UNBLOCK_SOURCE,
 *   IPO_ADD_SOURCE_MEMBERSHIP, IPO_DROP_SOURCE_MEMBERSHIP
 * . Also called from tfComSetipv4sourcefilter() with option name set to
 *   IPO_DROP_MEMBERSHIP when group membership is being dropped.
 *
 * 1. Get socket mcast cache entry by calling tfIgmpSocketCacheGet().
 * 2. Swich on optionName
 * 2.1 IPO_ADD_MEMBERSHIP
 *   2.1.1 If there is already a socket mcast cache entry, return TM_EADDRINUSE
 *   2.1.2 else call tfIgmpSocketCacheJoin()
 * 2.2 IPO_DROP_MEMBERSHIP
 *   2.1.1 If there is no socket mcast cache entry, return TM_EADDRNOTAAIL
 *   2.1.2 else
 *     2.1.2.1 move the current source list from the cache to a local list
 *     2.1.2.2 Remove socket mcast cache entry from the cache.
 *     2.1.2.3 Unlock the socket lock
 *     2.1.2.4 Lock the device lock
 *     2.1.2.5 Call tfIgmpLeaveHostGroup() passing user mcast request, and old
 *             source list that we are leaving.
 *     2.1.2.6 Free the local source list.
 *     2.1.2.7 Unlock the device lock
 *     2.1.2.8 Relock the socket
 * 2.3 IPO_BLOCK_SOURCE/IPO_ADD_SOURCE_MEMBERSHIP
 *     set srcCommand to TM_IGMP_CMND_ADD_SRC
 * 2.4 IPO_UNBLOCK_SOURCE/IPO_DROP_SOURCE_MEMBERSHIP
 *     set srcCommand to TM_IGMP_CMND_DROP_SRC
 * 3. If srcCommand is set above
 *  3.1 Store srcCommand in internal mcast request structure.
 *  3.2 Call tfIgmpSocketSourceCommand()
 * Parameters
 * Parameter        Description
 * imrIntPtr        Pointer to internal mcast request structure conveying
 *                  parameters set by the user in the user API.
 * Returns
 * Value              Meaning
 * TM_ENOERROR      success
 * IPO_ADD_MEMBERSHIP
 * Return value     meaning
 * TM_EADDRINUSE    This host group has already been added to the interface.
 * TM_ENOBUFS       Not enough memory to store the new entry.
 * IPO_DROP_MEMBERSHIP
 * Return value     meaning
 * TM_EADDRNOTAVAIL This host group had not been added to the interface
 * IPO_ADD_SOURCE_MEMBERSHIP:
 * Return value     meaning
 * TM_ENOBUFS       Not enough memory to store the new entry, or source list.
 * IPO_DROP_SOURCE_MEMBERSHIP:
 * Return value     meaning
 * TM_EADDRNOTAVAIL This host group had not been added to the interface
 * IPO_BLOCK_SOURCE:
 * IPO_UNBLOCK_SOURCE:
 * Return value     meaning
 * TM_EADDRNOTAVAIL This host group had not been added to the interface
 */
int tfIgmpSetSockMembership (ttIpMreqSrcPtr   imrIntPtr)
{
    ttSocketEntryPtr    socketEntryPtr;
    ttDeviceEntryPtr    devEntryPtr;
    ttSockIgmpInfoPtr   sockIgmpInfoPtr;
#ifdef TM_USE_IGMPV3
    ttSourceListPtr     oldSrcListPtr;
    ttSourceList        oldSourceList;
#endif /* TM_USE_IGMPV3 */
    int                 optionName;
#ifdef TM_USE_IGMPV3
    int                 srcCommand;
    int                 fmode;
#endif /* TM_USE_IGMPV3 */
    int                 errorCode;

#ifdef TM_USE_IGMPV3
    srcCommand = -1;
    fmode = -1;
#endif /* TM_USE_IGMPV3 */
    errorCode = TM_ENOERROR;
    socketEntryPtr = imrIntPtr->imrSocketPtr;
    devEntryPtr = imrIntPtr->imrDevPtr;
    optionName = imrIntPtr->imrOptionName;
/* Get the socket mcast cache entry if any */
    sockIgmpInfoPtr = tfIgmpSocketCacheGet(imrIntPtr);
    switch (optionName)
    {
        case IPO_ADD_MEMBERSHIP:
            if (sockIgmpInfoPtr != (ttSockIgmpInfoPtr)0)
            {
/* Already added */
                errorCode = TM_EADDRINUSE;
                break;
            }
            errorCode = tfIgmpSocketCacheJoin(imrIntPtr);
            break;
        case IPO_DROP_MEMBERSHIP:
            if (sockIgmpInfoPtr == (ttSockIgmpInfoPtr)0)
            {
/* Nothing to drop */
                errorCode = TM_EADDRNOTAVAIL;
                break;
            }
#ifdef TM_USE_IGMPV3
            oldSrcListPtr = &oldSourceList;
/* Move the source list to a local list */
            tfIgmpMoveSourceList(&sockIgmpInfoPtr->siiSourceList,
                                 oldSrcListPtr);
#endif /* TM_USE_IGMPV3 */
/* Remove socket mcast cache entry from the socket mcast cache */
            tfIgmpSocketCacheRemove(sockIgmpInfoPtr,
                                    socketEntryPtr);
/* Unlock socket entry because we are locking the device */
            tm_call_unlock(&(socketEntryPtr->socLockEntry));
/*
 * . Lock the interface
 * . call tfIgmpLeaveHostGroup();
 * . unlock the interface
 */
            tm_call_lock_wait(&(devEntryPtr->devLockEntry));
            (void)tfIgmpLeaveHostGroup(TM_8BIT_YES,
                                       imrIntPtr
#ifdef TM_USE_IGMPV3
                                      ,oldSrcListPtr
#endif /* TM_USE_IGMPV3 */
                                       );
#ifdef TM_USE_IGMPV3
/* Free the local source list */
            tfIgmpFreeSourceList(oldSrcListPtr);
#endif /* TM_USE_IGMPV3 */
/* Unlock device */
            tm_call_unlock(&(devEntryPtr->devLockEntry));
/* relock socket entry */
            tm_call_lock_wait(&(socketEntryPtr->socLockEntry));
            break;
#ifdef TM_USE_IGMPV3
        case IPO_BLOCK_SOURCE: /* exclude mode */
        case IPO_ADD_SOURCE_MEMBERSHIP: /* include mode */
            srcCommand = TM_IGMP_CMND_ADD_SRC;
            break;
        case IPO_UNBLOCK_SOURCE: /* exclude mode */
        case IPO_DROP_SOURCE_MEMBERSHIP: /* include mode */
            srcCommand = TM_IGMP_CMND_DROP_SRC;
            break;
#endif /* TM_USE_IGMPV3 */
        default:
            break;
    }
#ifdef TM_USE_IGMPV3
    if (srcCommand != -1)
/* 
 * Common code for 
 * IPO_BLOCK_SOURCE, IPO_UNBLOCK_SOURCE,
 * IPO_ADD_SOURCE_MEMBERSHIP, IPO_DROP_SOURCE_MEMBERSHIP
 */
    {
        imrIntPtr->imrSrcCommand = srcCommand;
/* Socket potentially unlocked/relocked */
        errorCode = tfIgmpSocketSourceCommand(sockIgmpInfoPtr,
                                              imrIntPtr);
    }
#endif /* TM_USE_IGMPV3 */
    return errorCode;
}

/* 
 * tfIgmpSocketCacheJoin Function Description
 * Join a mcast group possibly passing a source list.
 * Called when the user calls either:
 * . setsockopt with IPO_ADD_MEMBERSHIP, or
 * . setsockopt with IPO_ADD_SOURCE_MEMBERSHIP when group had not been joined
 *   on the socket yet, or
 * . tfComSetipv4sourcefilter() (common code for setipv4sourcefilter() or
 *   tfIoctl()) to join a group that had not been joined on the socket yet. 
 * 1. Try and allocate and insert a new socket mcast cache entry, storing the
 *    mcast group, associated device entry pointer, and user source list by
 *    calling tfIgmpSocketCacheNewInsert(). If we fail, return TM_ENOBUFS.
 * 2. Unlock the socket lock.
 * 3. Lock the device
 * 4. Call tfIgmpJoinHostGroup() specifying that the call is from a socket,
 *    and passing the user parameters, to join the group at the IGMP level.
 * 5. Unlock the device.
 * 6. Relock the socket
 * 7. If an error occured remove and free the socket mcast cache entry.
 * Parameters
 * Parameter        Description
 * imrIntPtr        Pointer to internal mcast request structure conveying
 *                  parameters set by the user in the user API.
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Success
 * TM_ENOBUFS       Not enough memory to allocate a socket mcast cache entry,
 *                  or source list.
 */
int tfIgmpSocketCacheJoin (ttIpMreqSrcPtr imrIntPtr)
{
    ttSocketEntryPtr    socketEntryPtr;
    ttDeviceEntryPtr    devEntryPtr;
    int                 errorCode;

    errorCode = tfIgmpSocketCacheNewInsert(imrIntPtr);
    if (errorCode == TM_ENOERROR)
    {
        socketEntryPtr = imrIntPtr->imrSocketPtr;
/* Unlock socket entry because we are locking the device */
        tm_call_unlock(&(socketEntryPtr->socLockEntry));
/*
 * . Lock the interface
 * . call tfIgmpJoinHostGroup();
 * . unlock the interface
 */
        devEntryPtr = imrIntPtr->imrDevPtr;
        tm_call_lock_wait(&(devEntryPtr->devLockEntry));
        errorCode = tfIgmpJoinHostGroup(TM_8BIT_YES,
                                        imrIntPtr);
/* Unlock device */
        tm_call_unlock(&(devEntryPtr->devLockEntry));
/* relock socket entry */
        tm_call_lock_wait(&(socketEntryPtr->socLockEntry));
        if (errorCode != TM_ENOERROR)
        {
/* clean up */
            (void)tfIgmpSocketCacheGetRemove(imrIntPtr);
        }
    }
    return errorCode;
}

/*
 * tfIgmpSocketJoinReturn Function Description
 * Called just before returning from tfIgmpSetSockOpt(),
 * or tfComSetipv4sourcefilter.
 * Return from join. Make sure to purge socket IGMP info cache if socket
 * had been closed while the user mcast API call had been processed, as
 * we do not purge the socket mcast cache when we close the socket if
 * there is a user mcast API call being processed.
 * 1. Decrease the number of pending socket mcast calls.
 * 2. If the socket had been closed during the socket mcast call, purge
 *    the socket mcache.
 * Parameters
 * Parameter        Meaning
 * socketEntryPtr   Pointer to socket entry.
 * Returns
 * NONE
 */
void tfIgmpSocketJoinReturn(ttSocketEntryPtr socketEntryPtr)
{
/* socket IGMP cache and stack IGMP cache now in SYNC */
    socketEntryPtr->socIgmpJoinsPending--;
    if (    ( tm_16bit_one_bit_set( socketEntryPtr->socFlags,
                                    TM_SOCF_CLOSING))
         || (tm_16bit_bits_not_set(socketEntryPtr->socFlags,
                                   TM_SOCF_OPEN)) 
       )
    {
/* If socket was closed while join was pending, make sure that we clean up */
        tfIgmpSocketCachePurge(socketEntryPtr);
    }
    return;
}

#ifdef TM_USE_IGMPV3
/*
 * tfIgmpSocketSourceCommand Function Description
 * Comon code for Basic APIs.
 * Called by tfIgmpSetSockMembership when the user calls setsockopt with
 * option name IPO_BLOCK_SOURCE, IPO_ADD_SOURCE_MEMBERSHIP, IGMP_CMND_ADD_SRC
 * IPO_UNBLOCK_SOURCE, or IPO_DROP_SOURCE_MEMBERSHIP
 * Add or drop the source at the socket mcast cache level, and add or drop the
 * at the IGMP level, possibly sending an IGMPv3 report.
 * 1. Get socket mcast cache entry. If we have a socket mcast cache entry:
 * 1.1. Check that the new filter mode is compatible with the previous one.
 * 1.2. If we are adding a source: 
 *      (option name == IPO_BLOCK_SOURCE or IPO_ADD_SOURCE_MEMBERSHIP =>
 *      srcCommand <- TM_IGMP_CMND_ADD_SRC)
 *  1.2.1 Check if we transition from Exclusive Join with no source
 *  1.2.2 Add the source to the socket mcast cache entry source list.
 *  1.2.3 If we succeed:
 *   1.2.3.1 Add the source at the IGMP level passing whether we transition
 *           from exclusive join with no source on the socket.
 *           (socket potentially unlocked/relocked)
 *   1.2.3.2 If we fail, reget the socket cache mcast entry, and drop the
 *           source.
 * 1.3. else we are dropping a source:
 *  1.3.1 If mode is INCLUDE and we are dropping the last source member,
 *        then drop membership: set option name to IPO_DROP_MEMBERSHIP, and
 *        call tfIgmpSetSockMembership(). (socket potentially unlocked/relocked)
 *  1.3.2 else we are just dropping a source.
 *   1.3.2.1 Drop the source from the socket mcast cache entry source list
 *           (won't fail.)
 *   1.3.2.2 If successful,
 *     1.3.2.2.1 Check if we transition to Exclusive Join with no source on the
 *               socket.
 *     1.3.2.2.2 Drop the source at the IGMP level passing whether we transition
 *               to exclusive join with no source on the socket (won't fail.)
 *               (socket potentially unlocked/relocked)
 * 2. else we do not have a socket mcast cache entry
 *   2.1 If new filter mode is EXCLUDE (IPO_BLOCK_SOURCE, IPO_UNBLOCK_SOURCE)
 *        return TM_EINVAL, as IPO_ADD_MEMBERSHIP should have been used
 *        instead.
 *   2.2 If command is a drop source command, return TM_EADDRNOTAVAIL as there
 *       is nothing to delete.
 *       (option name == IPO_UNBLOCK_SOURCE or IPO_DROP_SOURCE_MEMBERSHIP =>
 *          srcCommand <- TM_IGMP_CMND_DROP_SRC)
 *   2.3 Call tfIgmpSocketCacheJoin() (socket potentiall unlocked/relocked)
 * 3. Return
 *
 * Parameters
 * Parameter        Meaning
 * sockIgmpInfoPtr  Pointer to socket mcast cache entry.
 * imrIntPtr        Pointer to internal mcast request structure conveying
 *                  parameters set by the user in the user API.
 * Returns
 * Value            Meaning
 * TM_ENOERROR      success
 * TM_ENOBUFS       not enough memory to add the source
 * TM_EINVAL        new filter mode is incompatible with the previous one.
 * TM_EADDRNOTAVAIL Trying to drop a source that was not added
 */
static int tfIgmpSocketSourceCommand(ttSockIgmpInfoPtr  sockIgmpInfoPtr,
                                     ttIpMreqSrcPtr     imrIntPtr)
{
    ttSourceListPtr srcListPtr;
    int             errorCode;
    int             srcCommand;
    int             fmode;
    int             deltaEmptyExclJoin;

    srcCommand = imrIntPtr->imrSrcCommand;
    fmode = imrIntPtr->imrFmode;
    errorCode = TM_ENOERROR;
    if (sockIgmpInfoPtr != (ttSockIgmpInfoPtr)0)
    {
        if (sockIgmpInfoPtr->siiSourceList.srcMode != (tt16Bit)fmode)
/* Previous mode incompatible. Invalid */
        {
            errorCode = TM_EINVAL;
        }
        else
        {
            deltaEmptyExclJoin = 0;
            if (srcCommand == TM_IGMP_CMND_ADD_SRC)
/* Add */
            {
                if (    (sockIgmpInfoPtr->siiSourceList.srcMode
                                                         == TM_MODE_IS_EXCLUDE)
                     && (sockIgmpInfoPtr->siiSourceList.srcCount
                                                         == TM_16BIT_ZERO))
                {
/* Transition from Exclusive Join with no source */
                    deltaEmptyExclJoin = -1;
                }
/* tfIgmpSocketCacheAddSource will catch adding duplicates */
                errorCode = tfIgmpSocketCacheAddSource(sockIgmpInfoPtr,
                                                       imrIntPtr);
                if (errorCode == TM_ENOERROR)
                {
/*
 * Tell IGMP passing imrIntPtr. 
 * Socket unlocked/relocked
 */
                    errorCode = tfIgmpChangeSource(imrIntPtr,
                                                   (ttSourceListPtr)0,
                                                    deltaEmptyExclJoin);
                    if (errorCode != TM_ENOERROR)
                    {
/* 
 * If fails call
 * tfIgmpSocketCacheDropSource after regetting sockIgmpInfoPtr
 */
                        sockIgmpInfoPtr = tfIgmpSocketCacheGet(imrIntPtr);
                        if (sockIgmpInfoPtr != (ttSockIgmpInfoPtr)0)
                        {
                            (void)tfIgmpSocketCacheDropSource(
                                                            sockIgmpInfoPtr,
                                                            imrIntPtr);
                        }
                    }
                }
            }
            else
            {
                srcListPtr = &sockIgmpInfoPtr->siiSourceList;
                if (    (srcListPtr->srcMode == TM_MODE_IS_INCLUDE)
                     && (srcListPtr->srcCount == (tt16Bit)1)
                     && (tm_ip_match(srcListPtr->srcAddrPtr[0].s_addr,
                                     imrIntPtr->imrSrcAddrPtr[0].s_addr)))
/* 
 * If mode is INCLUDE and we are dropping the last source member,
 * then drop membership
*/
                {
                    imrIntPtr->imrOptionName = IPO_DROP_MEMBERSHIP;
                    imrIntPtr->imrNumSrc = 0;
/* Socket potentially unlocked/relocked */
                    errorCode = tfIgmpSetSockMembership(imrIntPtr);
                }
                else
                {
                    errorCode = tfIgmpSocketCacheDropSource(sockIgmpInfoPtr,
                                                            imrIntPtr);
                    if (errorCode == TM_ENOERROR)
                    {
                        if (    (sockIgmpInfoPtr->siiSourceList.srcMode
                                                     == TM_MODE_IS_EXCLUDE)
                             && (sockIgmpInfoPtr->siiSourceList.srcCount
                                                     == TM_16BIT_ZERO))
/* Transition to Exclusive Join with no source */
                        {
                            deltaEmptyExclJoin = 1;
                        }
/*
 * Tell IGMP passing imrIntPtr. 
 * Socket unlocked/relocked
 */
                        (void)tfIgmpChangeSource(imrIntPtr,
                                                 (ttSourceListPtr)0,
                                                 deltaEmptyExclJoin);
                    }
                }
            }
        }
    }
    else
    {
        if (fmode == TM_MODE_IS_EXCLUDE)
/* Trying to use an Any-Source option without doing IP_ADD_MEMBERSHIP */
        {
            errorCode = TM_EINVAL;
        }
        else if (srcCommand == TM_IGMP_CMND_DROP_SRC)
        {
/* IP_DROP_SOURCE_MEMBERSHIP and nothing to delete */
            errorCode = TM_EADDRNOTAVAIL;
        }
        else
        {
/* 
 * IPO_ADD_SOURCE_MEMBERSHIP
 * Socket potentially unlocked/relocked
 */
            errorCode = tfIgmpSocketCacheJoin(imrIntPtr);
        }
    }
    return errorCode;
}

#ifdef TM_IP_SRC_ERROR_CHECKING
/* 
 * tfIgmpCheckValidSrcAddr() Function Description
 * Called only if TM_IP_SRC_ERROR_CHECKING is defined.
 * (optional): check for invalid source address in the passed list
 * of source addresses.
 *  We reject any source address which is limited broadcast, loopback
 *  on a non loop back interface, multicast, experimental or class broadcast, 
 *  or directed broadcast address on the interface. Note that the check
 *  on class broadcast will also eliminate class D, and E, and the limited
 *  broadcast address.
 * Parameters
 * Parameter        Meaning
 * imrIntPtr        Pointer to internal mcast request structure conveying
 *                  parameters set by the user in the user API.
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Valid source address.
 * TM_EADDRNOTAVAIL Invalid source address.
 */
static int tfIgmpCheckValidSrcAddr (ttIpMreqSrcPtr imrIntPtr)
{
    ttDeviceEntryPtr devPtr;
    tt4IpAddress     ipSrcAddr;
    int              i;
    int              errorCode;

    errorCode = TM_ENOERROR;
    devPtr = imrIntPtr->imrDevPtr;
    for (i = 0; i < imrIntPtr->imrNumSrc; i++)
    {
        tm_ip_copy(imrIntPtr->imrSrcAddrPtr[i].s_addr, ipSrcAddr);
        if (   (    (tm_ip_class_net_mask(ipSrcAddr) | (ipSrcAddr))
                 == TM_IP_LIMITED_BROADCAST)
            || (    (tm_ip_is_loop_back(ipSrcAddr))
                 && (!(tm_dev_is_loopback(devPtr))) )
#ifdef TM_SINGLE_INTERFACE_HOME
            || (tm_ip_d_broad_match(devPtr, ipSrcAddr))
#else /* !TM_SINGLE_INTERFACE_HOME */
            || (tfMhomeDBroadMatch(devPtr, ipSrcAddr,
                                 &imrIntPtr->imrMhomeIndex) == TM_ENOERROR)
#endif /* TM_SINGLE_INTERFACE_HOME */
           )
        {
            errorCode = TM_EADDRNOTAVAIL;
            break;
        }
    }
    return errorCode;
}
#endif /* TM_IP_SRC_ERROR_CHECKING */

/* 
 * tfIgmpCacheGetSource() Function Description.
 * Get index of source address in source list
 * 1. Start the search at *searchIndexPtr
 * 2. For each index in the source list:
 *  2.1 If we find a match break, and update *searchIndexPtr for
 *      the next search.
 *  2.2 If source address is less than the current entry in the source list
 *      we won't find a match as the source list is sorted so
 *      break, and update *searchIndexPtr for the next search.
 * Parameters
 * Parameter          Description
 * srcListPtr         Pointer to source list to be searched.
 * searchIndexPtr     Pointer to index we want to start the search at.
 *                    Updated with found index.
 * srcIpAddr          source IP addres we are looking for
 * Returns
 * Value        Meaning
 * -1           srcIpAddr is not in the source list
 * >= 0         srcIpAddr is in the source list at that index.
 */
static int tfIgmpCacheGetSource (
    ttSourceListPtr     srcListPtr,
    ttIntPtr            searchIndexPtr,
    tt4IpAddress        srcIpAddr)
{
    int             index;
    int             maxIndex;
    int             found;

    maxIndex = (int)(srcListPtr->srcCount);
    found = 0;
    for (index = *searchIndexPtr; index < maxIndex; index++)
    {
        if (tm_ip_match(
                srcIpAddr,
                srcListPtr->srcAddrPtr[index].s_addr))
        {
/* match on source address */
            found = 1;
/* Start of next search */
            *searchIndexPtr = index;
            break;
        }
        if (srcIpAddr < srcListPtr->srcAddrPtr[index].s_addr)
/* Sorted list */
        {
/* Start of next search */
            *searchIndexPtr = index;
            break;
        }
    }
    if (!found)
    {
        index = -1;
    }
    return index;
}

/*
 * tfIgmpSocketCacheAddSource Function Description
 * Add user source addresses pointed to by imrSrcAddrPtr to this socket mcast
 * cache entry
 * 1. Start search at the begining of the source list.
 * 2. Call common function tfIgmpCacheAddSource() to add the user source
 *    addresses to the passed source list (socket mcast cache entry source
 *    list), specifying that the source list is a socket list, and that the
 *    user source addresses are not sorted.
 * Parameters
 * Parameter        Description
 * sockIgmpInfoPtr  Pointer to socket mcast cache entry
 * imrIntPtr        Pointer to internal mcast request structure conveying
 *                  parameters set by the user in the user API.
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Success
 * TM_ENOBUFS       Not enough memory to store the source addresses
 * TM_EADDRNOTAVAIL One of the source addresses already in the socket mcast
 *                  cache entry source list.
 */
int tfIgmpSocketCacheAddSource (
    ttSockIgmpInfoPtr   sockIgmpInfoPtr,
    ttIpMreqSrcPtr      imrIntPtr)
{
    int                 errorCode;
    int                 addIndex;

    addIndex = 0;
    errorCode = tfIgmpCacheAddSource(&sockIgmpInfoPtr->siiSourceList,
                                     &addIndex,
                                     imrIntPtr->imrSrcAddrPtr,
                                     imrIntPtr->imrNumSrc,
                                     TM_IGMP_SOCKET_LIST, /* Socket list */
                                     TM_8BIT_NO);         /* not sorted */
    return errorCode;
}

/* 
 * tfIgmpCacheAddSource Function Description
 * Add source addresses to this source list
 * Called when source addresses need to be added to a socket source list, or
 * to an IGMP source list, or to a network source list (from queries).
 * 1. Check that we have not reached the source list limit for socket source
 *    list, or IGMP source list. (Network source list limit (to guard against
 *    DoS attacks) is checked before this function is called.) For an IGMP
 *    source list take into account that the list could have duplicates.
 * 2. If number of sources we want to add exceeds the limit, return TM_ENOBUFS
 * 3. else
 *   3.1 If the number of sources we want to add exceeds the allocation,
 *       reallocate and copy already stored sources. Use a blocking factor so
 *       that we do not constantly have to re-allocate.
 *   3.2 For each source address that we want to add, call
 *       tfIgmpCacheSortedInsert() to insert one source address in the source
 *       list in sorted order. Pass the pointer to the start of the addition
 *       index search, which is being updated with the insertion index. If the
 *       source addresses that we are adding are not already sorted, reset
 *       the value pointed to by that index, as we have to search for the
 *       insertion from scratch for each address, i.e. from the beginning of
 *       the source list.
 * Parameters
 * Parameter        Description
 * srcListPtr       Pointer to source list to add to
 * addIndexPtr      Pointer to index to start searching from for the addition.
 *                  value pointed to is updated with start index for next
 *                  insertion if list is sorted, otherwise updated with 0.
 * srcAddrPtr       Pointer to sources to add
 * numSrc           Number of sources to ad
 * listType         source list type (either socket, IGMP, or network)
 * sorted           whether the sources to add are sorted.
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Success
 * TM_ENOBUFS       Not enough memory to store the source addresses
 * TM_EADDRNOTAVAIL Only if listType is socket: one of the source addresses
 *                  already in the socket mcast cache entry source list.
 */
int tfIgmpCacheAddSource (
    ttSourceListPtr         srcListPtr,
    ttIntPtr                addIndexPtr,
    struct in_addr TM_FAR * srcAddrPtr,
    int                     numSrc,
    tt8Bit                  listType,
    tt8Bit                  sorted)
{
    tt4IpAddressPtr         srcIpAddrPtr;
    int                     totalNumSrc;
    int                     allocCount;
    int                     maxNumSrc;
    int                     index;
    int                     matches;
    int                     searchIndex;
    int                     i;
    int                     errorCode;

    srcIpAddrPtr = (tt4IpAddressPtr)0;
    totalNumSrc = (int)(srcListPtr->srcCount) + numSrc;
    if (listType == TM_IGMP_SOCKET_LIST)
    {
/* Socket source list */
        maxNumSrc = tm_context(tvIgmpSoMaxSrcFilter);
    }
    else
/* IGMP/network source list */
    {
        maxNumSrc = tm_context(tvIgmpIpMaxSrcFilter);
/*
 * Subtract sources that are already in the list, since we could have
 * duplicates due to other socket joins.
 */
        if (srcListPtr->srcCount != TM_16BIT_ZERO)
        {
            matches = 0;
            searchIndex = 0;
            for (i = *addIndexPtr; i < numSrc; i++)
            {
                index = tfIgmpCacheGetSource(srcListPtr,
                                             &searchIndex,
                                             srcAddrPtr[i].s_addr);
        
                if (index != -1)
                {
                    matches ++;
                }
                if (!sorted)
                {
                    searchIndex = 0;
                }
            }
            totalNumSrc -= matches;
        }
    }
    if (    (totalNumSrc > maxNumSrc)
         && (listType != TM_IGMP_NETWORK_LIST) )
/* Limit for network recorded list is checked somewhere else. */
    {
        errorCode = TM_ENOBUFS;
    }
    else
    {
        errorCode = TM_ENOERROR;
#ifdef TM_IGMP_PREALLOCATED_SRC
        if (    (srcListPtr->srcAddrPtr == (struct in_addr TM_FAR *)0)
             && (totalNumSrc <= TM_IGMP_SRC_ARR_SIZE) )
        {
            srcListPtr->srcAddrPtr =
                                &srcListPtr->srcAddrArr[0];
            srcListPtr->srcOwnerCountPtr =
                                &srcListPtr->srcOwnerCountArr[0];
            srcListPtr->srcAllocCount = (tt16Bit)TM_IGMP_SRC_ARR_SIZE;
        }
        else
#endif /* TM_IGMP_PREALLOCATED_SRC */
        {
            if (totalNumSrc > (int)srcListPtr->srcAllocCount)
            {
/* Allocate using a blocking factor to avoid too many re-allocations */
                allocCount = ( ( (    totalNumSrc
                                   + (TM_IGMP_SOURCE_BLOCKING_FACTOR - 1))
                                / TM_IGMP_SOURCE_BLOCKING_FACTOR)
                              * TM_IGMP_SOURCE_BLOCKING_FACTOR); 
/* realloc and copy */
                errorCode = tfIgmpAllocateCopySources(srcListPtr, allocCount,
                                                      TM_8BIT_YES);
            }
        }
        if (errorCode == TM_ENOERROR)
        {
/* cache the source addresses on this source list */
            for (i = 0; i < numSrc; i++)
            {
                errorCode = tfIgmpCacheSortedInsert(srcListPtr,
                                                    addIndexPtr,
                                                    srcAddrPtr[i],
                                                    listType);
                if (!sorted)
                {
/*
 * Sources to be added are not sorted. Start search for insertion of next
 * source address at the beginning of the source list
 */
                    *addIndexPtr = 0;
                }
                if (errorCode != TM_ENOERROR)
                {
/* Break out of the loop if we fail */
                    break;
                }
            }
        }
    }
    return errorCode;
}

/* 
 * tfIgmpAllocateCopySources
 * Allocate space for sources and owner counts in a source list, and copy
 * existing sources and owner counts into newly allocated buffer.
 * Called when we either want to reallocate space for sources and owner counts
 * because the source list is too small, or when we just want to copy the
 * source list (for backup).
 * 1. Allocate space for sources and corresponding owner counts. The sources
 *    are at the beginning of the buffer, and owner counts follow.
 * 2. If we failed to allocate, return TM_ENOBUFS
 * 3. Point to new source owner counts area.
 * 4. If we had sources in the current source list
 *  4.1 copy old sources and owner counts to newly allocated buffer in their
 *      respective (separate) areas.
 *  4.2 If it is a reallocation (and not just a copy) free the old source list
 *      by calling tfIgmpFreeSourceList(), and re-initialize the current
 *      number of sources as it is wiped out when we call
 *      tfIgmpFreeSourceList()
 * 5. If it is a reallocation, and not a copy inialize the source list
 *    allocation count as it has changed.
 * 6. Make the source list source addresses' pointer and source owner counts'
 *    pointers point to their respective areas in the newly allocated buffer.
 * Parameters
 * Parameter        Description
 * srcListPtr       Pointer to source list where we want to reallocate the
 *                  sources and sources' owner counts. 
 * allocCount       Number of sources to allocate
 * realloc          reallocating (not just copying)
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Success
 * TM_ENOBUFS       Not enough memory to allocate the source addresses and
 *                  owner counts.
 */
static int tfIgmpAllocateCopySources (
    ttSourceListPtr srcListPtr,
    int             allocCount,
    tt8Bit          realloc)
{
    struct in_addr TM_FAR * newSrcIpAddrPtr;
    tt16BitPtr              newSrcOwnerCountPtr;
    int                     oldNumSrc;
    int                     errorCode;

    newSrcIpAddrPtr =
                (struct in_addr TM_FAR *)tm_get_raw_buffer(
                                        allocCount * (  sizeof(struct in_addr)
                                                      + sizeof(tt16Bit)));
    if (newSrcIpAddrPtr == (struct in_addr TM_FAR *)0)
    {
/* failed to allocate */
        errorCode = TM_ENOBUFS;
    }
    else
    {
        errorCode = TM_ENOERROR;
        oldNumSrc = (int)(srcListPtr->srcCount);
        newSrcOwnerCountPtr = (tt16BitPtr)(ttVoidPtr)
                                        (newSrcIpAddrPtr + allocCount);
        if (srcListPtr->srcAddrPtr != (struct in_addr TM_FAR *)0)
        {
/* copy to the reallocated buffer */
            tm_bcopy(srcListPtr->srcAddrPtr,
                     newSrcIpAddrPtr,
                     oldNumSrc * sizeof(struct in_addr));
            tm_bcopy(srcListPtr->srcOwnerCountPtr,
                     newSrcOwnerCountPtr,
                     oldNumSrc * sizeof(tt16Bit));
            if (realloc)
/* free the old buffer if it is a reallocation (and not just a copy) */
            {
                tfIgmpFreeSourceList(srcListPtr);
/* Re-initialize number of sources (reset by tfIgmpFreeSourceList) */
                srcListPtr->srcCount = (tt16Bit)oldNumSrc;
            }
        }
        if (realloc)
        {
/* Reallocation, and not a copy: store the new number of entries reallocated */
            srcListPtr->srcAllocCount = (tt16Bit)(allocCount);
        }
/* Point to newly allocated buffer */
        srcListPtr->srcAddrPtr = newSrcIpAddrPtr;
        srcListPtr->srcOwnerCountPtr = newSrcOwnerCountPtr;
    }
    return errorCode;
}

/* 
 * tfIgmpCacheSortedInsert Function Description
 * Add one source address in sorted source list
 * 1. For each address in the source list starting at *seachIndexPtr
 *   1.1 If the addresses match
 *     1.1.1 If the list is not a socket list increase the owner count.
 *     1.1.2 else the list is a socket list set errorCode to TM_EADDRNOTAVAIL.
 *     1.1.3 Set the *searchIndexPtr for start of next search.
 *     1.1.4 set i to -1 so that we exit without inserting.
 *     1.1.5 break.
 *   1.2 If we have reached the insertion point (new source address is less
 *       than the current source list address.
 *     1.2.1 set the *searchIndexPtr for start of next search.
 *     1.2.2 break.
 * 2. If we need to insert (i not set to -1), insert at index i
 *   2.1 Move existing sources and owner count at insertion point one slot
 *       higher.
 *   2.2 Copy new source with owner count 1 at insertion point index.
 *   2.3 Increase number of sources in the list.
 * 3. Return
 *  
 * Parameters
 * Parameter        Description
 * srcListPtr       Pointer to source list to add to
 * searchIndexPtr   Pointer to index to start searching from for the addition.
 *                  value pointed to is updated with start index search of
 *                  next insertion point. Useful when the sources to be
 *                  added are sorted.
 * srcAddr          source adress to add
 * listType         source list type (either socket, IGMP, or network)
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Success
 * TM_EADDRNOTAVAIL Only if listType is socket: source address already in the
 *                  socket mcast cache entry source list.
 */
static int tfIgmpCacheSortedInsert(
    ttSourceListPtr         srcListPtr,
    ttIntPtr                searchIndexPtr,
    struct in_addr          srcAddr,
    tt8Bit                  listType)
{
    int     i;
    int     j;
    int     errorCode;

    errorCode = TM_ENOERROR;
/* Find insertion point */
    for (i = *searchIndexPtr; i < srcListPtr->srcCount; i++)
    {
        if (srcAddr.s_addr == srcListPtr->srcAddrPtr[i].s_addr)
/* Found a match */
        {
            if (listType != TM_IGMP_SOCKET_LIST)
            {
/* If IGMP/network list, increase owner count */
                srcListPtr->srcOwnerCountPtr[i]++;
            }
            else
/*
 * It is an error if called from socket API, as the user cannot add a source
 * that has already been added
 */
            {
                errorCode = TM_EADDRNOTAVAIL;
            }
            *searchIndexPtr = i;
            i = -1; /* Do not insert */
            break;
        }
        if (srcAddr.s_addr < srcListPtr->srcAddrPtr[i].s_addr)
/* Insertion point */
        {
            *searchIndexPtr = i;
            break;
        }
    }
/* Insert if needed */
    if (i != -1)
/* Insert at index i */
    {
        for (j = srcListPtr->srcCount; j > i ; j--)
/* Move bigger sources up one slot */
        {
            srcListPtr->srcAddrPtr[j] = srcListPtr->srcAddrPtr[j-1];
            srcListPtr->srcOwnerCountPtr[j] =
                                     srcListPtr->srcOwnerCountPtr[j-1];
        }
/* Copy address at insertion point */
        srcListPtr->srcAddrPtr[i].s_addr = srcAddr.s_addr;
        srcListPtr->srcOwnerCountPtr[i] = 1;
        srcListPtr->srcCount++;
    }
    return errorCode;
}

/*
 * tfIgmpSocketCacheDropSource Function Description
 * Uncache these source addresses from this mcast cache on the socket
 * . Call common function tfIgmpCacheDropSource() to drop the user source
 *   addresses from the passed source list (socket mcast cache entry source
 *   list), specifying that the source list is a socket list, and that the
 *   user source addresses are not sorted.
 * Parameters
 * Parameter        Description
 * sockIgmpInfoPtr  Pointer to socket mcast cache entry
 * imrIntPtr        Pointer to internal mcast request structure conveying
 *                  parameters set by the user in the user API.
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Success
 * TM_EADDRNOTAVAIL source address was not in the socket mcast cache entry
 *                  source list.
 */
static int tfIgmpSocketCacheDropSource (
    ttSockIgmpInfoPtr       sockIgmpInfoPtr,
    ttIpMreqSrcPtr          imrIntPtr)
{
    int errorCode;

    errorCode = tfIgmpCacheDropSource(&sockIgmpInfoPtr->siiSourceList,
                                      imrIntPtr->imrSrcAddrPtr,
                                      imrIntPtr->imrNumSrc,
                                      TM_IGMP_SOCKET_LIST,
                                      TM_8BIT_NO);
    return errorCode;
}

/* 
 * tfIgmpCacheDropSource Function Description
 * uncache source addresses from this source list
 * Called when source addresses need to be dropped from a socket source list,
 * or from an IGMP source list, or from a network source list (from queries).
 * 1. For each source address that we want to drop,
 *   1.1 call tfIgmpCacheGetSource() to check whether the source to be dropped
 *       is in the source list, and to get its index.
 *   1.2 If the source to be dropped is not in the list set errorCode to
 *       TM_EADDRNOTAVAIL, but break out of the loop and return only if the
 *       source list is a socket source list.
 *   1.3 else source is in the source list, decrease the source list owner count
 *     1.3.1 If source list owner count is zero
 *       1.3.1.1 remove the entry, by moving higher index sources down by one
 *               slot.
 *       1.3.1.2 Decrease number of sources in the list.
 * 2. Return.
 * Parameters
 * Parameter        Description
 * srcListPtr       Pointer to source list to drop addresses from
 * srcAddrPtr       Pointer to sources to drop
 * numSrc           Number of sources to drop
 * listType         source list type (either socket, IGMP, or network)
 * sorted           whether the sources to drop are sorted.
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Success
 * TM_EADDRNOTAVAIL Only if listType is socket: One of the source address not
 *                  in the socket mcast cache entry source list.
 */
static int tfIgmpCacheDropSource(
    ttSourceListPtr         srcListPtr,
    struct in_addr TM_FAR * srcAddrPtr,
    int                     numSrc,
    tt8Bit                  listType,
    tt8Bit                  sorted)
{
    int               maxIndex;
    int               index;
    int               searchIndex;
    int               i;
    int               errorCode;

    errorCode = TM_ENOERROR;
    searchIndex = 0;
    for (i = 0; i < numSrc; i++)
    {
        index = tfIgmpCacheGetSource(srcListPtr,
                                     &searchIndex,
                                     srcAddrPtr[i].s_addr);
        if (!sorted)
        {
            searchIndex = 0;
        }
        if (index == -1)
/* Trying to delete an unexisting source address */
        {
            errorCode = TM_EADDRNOTAVAIL;
            if (listType == TM_IGMP_SOCKET_LIST)
            {
                break;
            }
        }
        else
        {
            maxIndex = ((int)(srcListPtr->srcCount)) - 1;
/* Decrease owner count */
            srcListPtr->srcOwnerCountPtr[index]--;
            if (srcListPtr->srcOwnerCountPtr[index] == 0)
            {
/* uncache this source address from this group cache on the socket */
                while (index < maxIndex )
                {
/* avoid overlapping bcopy by copying one entry at a time */
                    srcListPtr->srcAddrPtr[index].s_addr =
                                     srcListPtr->srcAddrPtr[index+1].s_addr;
                    srcListPtr->srcOwnerCountPtr[index] =
                                     srcListPtr->srcOwnerCountPtr[index+1];
                    index++;
                }
                srcListPtr->srcCount--;
            }
        }
    }
    return errorCode;
}
#endif /* TM_USE_IGMPV3 */

/*
 * tfIgmpSetSockInterface Function Description
 * Map membership setsockopt interface address or group address to a device
 * Find the interface:
 * 1. If imrIntPtr->imrIfAddr is zero, find the interface through the
 *    routing table using the group multicast address. Make sure the routing
 *    entry found is for a multicast group address (not the default gateway).
 *    Because we unlock the socket to query the routing
 *    table, check that the socket has not been closed after we relock
 *    the socket.
 * 2. If imrIntPtr->imrIfAddr is non-zero, then find the matching
 *    interface.
 *    devEntryPtr = tfIfaceMatch( imrIntPtr->imrIfAddr,
 *                               tfMhomeAddrMatch, &mhomeIndex );
 * 3. Copy found device entry pointer and multihome index in the structure
 *    pointed to by imrIntPtr.
 * 4. If we did not find an interface exit with TM_EEADDRNOTAVAIL errorCode.
 * Parameters
 * Parameter        Meaning
 * imrIntPtr        Pointer to internal mcast request structure conveying
 *                  parameters set by the user in the user API.
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Success
 * TM_EADDRNOTAVAIL No device found configured with the user passed interface
 *                  address or if the user passed interface address is zero,
 *                  no multicast routing entry found.  
 * TM_EBADF         socket closed while we had unlocked the socket.
 */
static int tfIgmpSetSockInterface (ttIpMreqSrcPtr   imrIntPtr)
{
    ttSocketEntryPtr    socketEntryPtr;
    ttDeviceEntryPtr    devEntryPtr;
    ttRtCacheEntry      rtCache;
    ttRtCacheEntryPtr   rtcPtr;
#ifdef TM_USE_IPV6
    tt6IpAddress        tempIp6Addr;
#endif /* TM_USE_IPV6 */
    int                 errorCode;
    tt16Bit             mhomeIndex;

    errorCode = TM_ENOERROR;
    if (tm_ip_zero(imrIntPtr->imrIfAddr))
    {
        socketEntryPtr = imrIntPtr->imrSocketPtr;
        if (    socketEntryPtr->socMcastDevPtr
            != (ttDeviceEntryPtr)0 )
        {
            devEntryPtr = socketEntryPtr->socMcastDevPtr;
            mhomeIndex = socketEntryPtr->socMcastMhomeIndex;
        }
        else
        {
/* Unlock the socket lock, because we are calling tfRtGet() */
            tm_call_unlock(&(socketEntryPtr->socLockEntry));
/*
* If imrIntPtr->imrIfAddr is zero, find the interface, corresponding
* to the multicast destination address through the routing table. This will
* only work if the user chose a default multicast interface with the
* tfSetMcastInterface() call.
*/
            rtcPtr = &rtCache;
            tm_bzero(rtcPtr, sizeof(ttRtCacheEntry));
#ifdef TM_USE_IPV6
            tm_6_addr_to_ipv4_mapped(
                            (tt4IpAddress)(imrIntPtr->imrMcastAddr),
                            &tempIp6Addr);
            tm_6_ip_copy_structs(
                            tempIp6Addr, rtcPtr->rtcDestIpAddr);
#else /* ! TM_USE_IPV6 */
            tm_ip_copy( imrIntPtr->imrMcastAddr,
                        rtcPtr->rtcDestIpAddr);
#endif /* ! TM_USE_IPV6 */
/* Default */
            devEntryPtr = (ttDeviceEntryPtr)0;
            mhomeIndex = TM_16BIT_ZERO;
            errorCode = tfRtGet(rtcPtr);
            if (errorCode == TM_ENOERROR)
            {
                if (tm_ip_is_class_d(tm_4_ip_addr(rtcPtr->rtcRtePtr->rteDest)))
                {
                    devEntryPtr = rtcPtr->rtcRtePtr->rteOrigDevPtr;
                    mhomeIndex = rtcPtr->rtcRtePtr->rteOrigMhomeIndex;
                }
                tfRtCacheUnGet(rtcPtr);
            }
            else
            {
                errorCode = TM_EADDRNOTAVAIL;
            }
/* Relock the socket, before we return */
            tm_call_lock_wait(&(socketEntryPtr->socLockEntry));
            if (    (tm_16bit_bits_not_set(socketEntryPtr->socFlags,
                                           TM_SOCF_OPEN))
                ||  (tm_16bit_one_bit_set(socketEntryPtr->socFlags,
                                           TM_SOCF_CLOSING)))
            {
/* 
 * (We need to re-check that the socket is opened because we unlocked it when
 * calling tfRtGet().)
 * Socket closed, or in the process of closing.
 */
                errorCode = TM_EBADF;
                devEntryPtr = (ttDeviceEntryPtr)0;
            }
        }
    }
    else
    {
/*
* If imrIntPtr->imrIfAddr is non-zero, then find the matching
* interface
*/
#ifdef TM_SINGLE_INTERFACE_HOME
        devEntryPtr = tfIfaceConfig(0);
        if ( devEntryPtr != (ttDeviceEntryPtr)0 )
        {
            if ( !tm_ip_match(imrIntPtr->imrIfAddr,
                              tm_ip_dev_addr(devEntryPtr, 0)) )
            {
                devEntryPtr = (ttDeviceEntryPtr)0;
            }
        }
        mhomeIndex = TM_16BIT_ZERO;
#else /* TM_SINGLE_INTERFACE_HOME */
        devEntryPtr = tfIfaceMatch(imrIntPtr->imrIfAddr,
                                   tfMhomeAddrMatch,
                                   &mhomeIndex);
#endif /* TM_SINGLE_INTERFACE_HOME */
    }
    imrIntPtr->imrDevPtr = devEntryPtr;
    imrIntPtr->imrMhomeIndex = mhomeIndex;
    if (devEntryPtr == (ttDeviceEntryPtr)0)
    {
        if (errorCode == TM_ENOERROR)
        {
            errorCode = TM_EADDRNOTAVAIL;
        }
    }
    return errorCode;
}

/*
 * tfIgmpGetSockOpt function description:
 * Function called from getsockopt() for IPO_MULTICAST_TTL,
 * or IPO_MULTICAST_IF options at the IP_PROTOIP level
 * switch on optName:
 *
 * IPO_MULTICAST_TTL  IP ttl for outgoing multicast datagrams
 * . verify that option length pointer points to a value that is at least
 *   sizeof(unsigned char) otherwise exit with TM_EINVAL error code.
 * . set value pointed to by option length pointer to sizeof(unsigned char)
 * . copy socIpMcastTtl to argument.
 *
 * IPO_MULTICAST_IF:
 * . verify that option length pointer points to a value that is at least
 *   sizeof(struct in_addr) otherwise exit with TM_EINVAL error code.
 * . set value pointed to by option length pointer to sizeof(struct in_addr)
 * . If the socket default outgoing multicast interface is not set, store
 *   0 IP address in the argument, otherwise store IP address of socket
 *   default outgoing interface for multicast UDP datagrams sent on this
 *   socket. 
 *
 * Parameters
 * Parameter        Description
 * socketEntryPtr   Pointer to socket entry
 * optionName       IPO_MULTICAST_TTL, or IPO_MULTICAST_IF
 * optionValuePtr   Pointer to option value to copy into 
 * optionLengthPtr  Pointer to option length, updated on return.
 *
 * Returns:
 * Return value     meaning
 * TM_ENOERROR      success
 * TM_EINVAL        Invalid option length
 */
int tfIgmpGetSockOpt ( ttSocketEntryPtr   socketEntryPtr,
                       int                optionName,
                       ttConstCharPtr     optionValuePtr,
                       ttIntPtr           optionLengthPtr )
{
    int                 errorCode;

    errorCode = TM_ENOERROR;
    switch (optionName)
    {
        case IPO_MULTICAST_TTL:
/* IPO_MULTICAST_TTL get the IP ttl for outgoing multicast datagrams */
            if (*optionLengthPtr < (int)sizeof(unsigned char))
            {
                errorCode = TM_EINVAL;
            }
            else
            {
                *optionLengthPtr = (int)sizeof(unsigned char);
                *((unsigned char TM_FAR *)(optionValuePtr)) =
                                socketEntryPtr->socIpMcastTtl;
            }
            break;
        case IPO_MULTICAST_IF:
/*
 * IPO_MULTICAST_IF get the IP address of default socket outgoing multicast
 * interface
 */
            if (*optionLengthPtr < (int)sizeof(struct in_addr))
            {
/* Verify that option length is at least sizeof(struct in_addr) */
                errorCode = TM_EINVAL;
            }
            else
            {
                *optionLengthPtr = (int)sizeof(struct in_addr);
                if (socketEntryPtr->socMcastDevPtr != (ttDeviceEntryPtr)0)
                {
/*
 * If socket multicast interface set, get IP address of interface/mhome
 * index.
 */
                        tm_ip_copy(
                         tm_ip_dev_addr(socketEntryPtr->socMcastDevPtr,
                                        socketEntryPtr->socMcastMhomeIndex),
                         ((struct in_addr TM_FAR *)(optionValuePtr))->s_addr);
                }
                else
                {
/* If socket multicast interface not set, return 0. */
                    tm_ip_copy( TM_IP_ZERO,
                                *((tt4IpAddressPtr)(optionValuePtr)) );
                }
            }
            break;
        default:
/* Not reached */
            errorCode = TM_ENOPROTOOPT;
            break;
    }
    return errorCode;
}

/*
 * tfIgmpRandom Function Description
 * Compute a random value in milliseconds between 0 and maxTimeoutInSeconds
 * . get a random number
 * . Get the remainder of the division between the random number and
 *   the maximum timeout value in milliseconds.
 * . return that value
 * Parameters:
 * Parameter            Description
 * maxTimeoutInSeconds  maximum timeout in seconds
 * Returns
 * Value                Meaning
 * timeout              random timeout in milliseconds.
 */
static tt32Bit tfIgmpRandom( tt32Bit maxTimeoutInMilliSecs )
{
    tt32Bit timeout;

    timeout = tfGetRandom();
    timeout = timeout % (tt32Bit)(maxTimeoutInMilliSecs);
    return timeout;
}

/*
 * tfIgmpTimerRemove Function Description
 * Remove an IGMP timer if any for a given IGMP entry, and timer index.
 * If a Timer is running on the IGMP entry at that index
 * . Remove it
 * . Reset the timer pointer
 * . change state to IDLE_MEMBER
 * Parameters
 * Parameter        Description
 * igmpEntryPtr     Pointer to IGMP entry
 * index            Timer Index
 * Returns
 * None
 */
static void tfIgmpTimerRemove (
    ttIgmpEntryPtr igmpEntryPtr,
    int timerIndex)
{
    if ( igmpEntryPtr->igmpTimerPtr[timerIndex] != (ttTimerPtr)0 )
/* Delaying member state */
    {
/* Delete the timer */
        tm_timer_remove(igmpEntryPtr->igmpTimerPtr[timerIndex]);
/* Reset timer field */
        igmpEntryPtr->igmpTimerPtr[timerIndex] = (ttTimerPtr)0;
/* Idle member */
        igmpEntryPtr->igmpState = TM_IGMP_IDLE_MEMBER;
    }
    return;
}

#ifdef TM_USE_IGMPV3
/*
 * tfIgmpGeneralQueryTimer function description
 * Called by the timer execute thread to send a delayed response to
 * a general query on the device.
 *   1. Get the device Entry Pointer from the timer parameter 1.
 *   2. Lock the device
 *   3. Check that the device query timer is still running. If it is:
 *     3.1. For each IGMP group entry in the device IGMP cache,
 *        3.1.1 Save the next IGMP entry.
 *        3.1.2 If the IGMP entry does not represent a local multicast address
 *          3.1.2.1 get the current source list
 *          3.1.2.2 If the current source list is non empty or the mode is
 *                  exclude
 *            3.1.2.2.1 Save the next IGMP entry multicast host group.
 *            3.1.2.2.2 call tfIgmpSendCurrentStateReport() to send the current
 *                      state report.
 *            3.1.2.2.3 break out of the loop if an error occured.
 *            3.1.2.2.4 Re-find the next IGMP entry using the saved multicast
 *                      host group if the device was unlocked relocked in
 *                      tfIgmpSendCurrentStateReport()
 *     3.2 Because we try and re-use the same packet for the current state
 *         report for different IGMP group entries, make sure we flush the last
 *         packet.
 *     3.3 Remove the general query timer.
 *   4. Unlock the device
 *   5. Return.
 * Parameters
 * Parameter        Description
 * timerBlockPtr    Pointer to timer structure
 * userParm1        Timer parameter 1 (devEntryPtr)
 * userParm2        Timer parameter 2
 * Returns
 * NONE
 */
static void tfIgmpGeneralQueryTimer (ttVoidPtr      timerBlockPtr,
                                     ttGenericUnion userParm1,
                                     ttGenericUnion userParm2)
{
    ttDeviceEntryPtr devEntryPtr;
    ttIgmpEntryPtr   igmpEntryPtr;
    ttIgmpEntryPtr   nextIgmpEntryPtr;
    ttSourceListPtr  srcListPtr;
    ttPacketPtr      packetPtr;
    ttTimerPtr       tmrPtr;
    tt4IpAddress     multicastGroup;
    int              errorCode;
    tt8Bit           unlocked;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    devEntryPtr = (ttDeviceEntryPtr)(userParm1.genVoidParmPtr);
    tm_call_lock_wait(&(devEntryPtr->devLockEntry));
    tmrPtr = devEntryPtr->devIgmpQueryTmrPtr;
    if ( (tmrPtr == timerBlockPtr) && tm_timer_not_reinit(tmrPtr) )
    {
        igmpEntryPtr = devEntryPtr->devIgmpPtr;
        packetPtr = (ttPacketPtr)0;
        tm_ip_copy(TM_IP_ZERO, multicastGroup); /* compiler warning */
/* For each entry in the device IGMP cache */
        while (igmpEntryPtr != (ttIgmpEntryPtr)0)
        {
/* Move to next group on this device */                        
            nextIgmpEntryPtr = igmpEntryPtr->igmpNextPtr;
            if (igmpEntryPtr->igmpState != TM_IGMP_LOCAL)
/* If IGMP group entry is not local */
            {
/* Current source list */
                srcListPtr = &igmpEntryPtr->igmpSrcList[TM_IGMP_CURRENT_LIST];
                if (    (srcListPtr->srcMode == TM_MODE_IS_EXCLUDE)
                     || (srcListPtr->srcCount != TM_16BIT_ZERO))
/* Something to report */
                {
/* Device potentially unlocked/relocked */
                    unlocked = TM_8BIT_NO;
                    if (nextIgmpEntryPtr != (ttIgmpEntryPtr)0)
                    {
                        tm_ip_copy(nextIgmpEntryPtr->igmpHostGroup,
                                   multicastGroup);
                    }
                    errorCode = tfIgmpSendCurrentStateReport(igmpEntryPtr,
                                                             srcListPtr,
                                                             &packetPtr,
                                                             &unlocked);
                    if (errorCode != TM_ENOERROR)
                    {
                        break;
                    }
                    else
                    {
                        if (    (unlocked == TM_8BIT_YES)
                             && (nextIgmpEntryPtr != (ttIgmpEntryPtr)0) )
/* If we have sent a report, the device was unlocked/relocked,
 * we need to re-find the next IGMP entry */
                        {
                            nextIgmpEntryPtr = tfIgmpFindEntry(multicastGroup,
                                                               devEntryPtr);
                        }
                    }
                }
            }
            igmpEntryPtr = nextIgmpEntryPtr;
        }
        if (packetPtr != (ttPacketPtr)0)
        {
/* Flush the (last non full) packet */
            tfIgmpSendPacket(devEntryPtr, packetPtr, TM_IGMP_V3_REPORT);
        }
/* Remove the general query timer on the device */
        tfIgmpDevQueryTimerRemove(devEntryPtr);
    }
    tm_call_unlock(&(devEntryPtr->devLockEntry));
    return;
}

/*
 * tfIgmpSomethingToReport Function Description.
 * Called before we schedule a general query timer. 
 * Check whether we have anything to send as a response to a query.
 * Loop through all the IGMP entries in the device IGMP cache, and
 * check whether the current source list is either non empty, or is
 * in exclude mode, in which case we have something to report.
 * Parameters
 * Parameter        Description
 * devEntryPtr      Pointer to device structure
 * Returns
 * Value    Meaning
 * 1        Something to report
 * 0        Nothing to report. No need to start the general query timer.
 */
static tt8Bit tfIgmpSomethingToReport (ttDeviceEntryPtr devEntryPtr)
{
    ttIgmpEntryPtr   igmpEntryPtr;
    ttSourceListPtr  srcListPtr;
    tt8Bit           report;

/* Avoid compiler warnings about unused parameters */
    igmpEntryPtr = devEntryPtr->devIgmpPtr;
    report = TM_8BIT_NO;
/* For each entry in the device IGMP cache */
    while (igmpEntryPtr != (ttIgmpEntryPtr)0)
    {
        if (igmpEntryPtr->igmpState != TM_IGMP_LOCAL)
/* If IGMP group entry is not local */
        {
/* Current source list */
            srcListPtr = &igmpEntryPtr->igmpSrcList[TM_IGMP_CURRENT_LIST];
            if (    (srcListPtr->srcMode == TM_MODE_IS_EXCLUDE)
                 || (srcListPtr->srcCount != TM_16BIT_ZERO))
/* Something to report */
            {
                report = TM_8BIT_YES;
                break;
            }
        }
/* Move to next group on this device */                        
        igmpEntryPtr = igmpEntryPtr->igmpNextPtr;
    }
    return report;
}

/*
 * tfIgmpGroupQueryTimer function description
 * Called by the timer execute thread to send a delayed response to
 * a group or source group query.
 *   1. Get the IGMP Entry Pointer from the timer parameter 1.
 *   2. Get the device entry pointer from the IGMP entry.
 *   3. Lock the device
 *   4. Check that the group query timer is still running. If it is:
 *     4.1 Remove the group query timer.
 *     4.2. If the IGMP entry is non local:
 *       4.2.1 Pick the recorded source list if it is non empty, otherwise
 *             pick the current source list. 
 *       4.2.2 If there is something to report, call
 *             tfIgmpSendCurrentStateReport() to send the report corresponding
 *             to the picked source list. (If the picked source list is the
 *             recorded one, then it will be freed in
 *             tfIgmpSendCurrentStateReport() before we send the packet and
 *             therefore before we unlock/relock the device in that function.)
 *   5. Unlock the device
 *   6. Return.
 * Parameters
 * Parameter        Description
 * timerBlockPtr    Pointer to timer structure
 * userParm1        Timer parameter 1 (igmpEntryPtr)
 * userParm2        Timer parameter 2
 * Returns
 * NONE
 */
static void tfIgmpGroupQueryTimer (ttVoidPtr      timerBlockPtr,
                                   ttGenericUnion userParm1,
                                   ttGenericUnion userParm2)
{
    ttDeviceEntryPtr devEntryPtr;
    ttIgmpEntryPtr   igmpEntryPtr;
    ttSourceListPtr  srcListPtr;
    ttTimerPtr       tmrPtr;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    igmpEntryPtr = (ttIgmpEntryPtr)(userParm1.genVoidParmPtr);
    devEntryPtr = igmpEntryPtr->igmpDevPtr;
    tm_call_lock_wait(&(devEntryPtr->devLockEntry));
    tmrPtr = igmpEntryPtr->igmpTimerPtr[TM_IGMP_QUERY_TIMER];
    if ( (tmrPtr == timerBlockPtr) && tm_timer_not_reinit(tmrPtr) )
    {
        tfIgmpTimerRemove(igmpEntryPtr, TM_IGMP_QUERY_TIMER);
        if (igmpEntryPtr->igmpState != TM_IGMP_LOCAL)
        {
            if (igmpEntryPtr->igmpSrcList[TM_IGMP_RECORDED_LIST].srcCount
                                                             == TM_16BIT_ZERO)
            {
                srcListPtr = &igmpEntryPtr->igmpSrcList[TM_IGMP_CURRENT_LIST];
            }
            else
            {
                srcListPtr = &igmpEntryPtr->igmpSrcList[TM_IGMP_RECORDED_LIST];
            }
            if (    (srcListPtr->srcMode == TM_MODE_IS_EXCLUDE)
                 || (srcListPtr->srcCount != TM_16BIT_ZERO))
            {
/* Something to report */
/* Device unlocked and relocked */
                (void)tfIgmpSendCurrentStateReport(
                                   igmpEntryPtr,
                                   srcListPtr,
                                   (ttPacketPtrPtr)0,
                                   (tt8BitPtr)0);
            }
        }
    }
    tm_call_unlock(&(devEntryPtr->devLockEntry));
    return;
}

/*
 * tfIgmpBuildRecordedList Function Description.
 * Called when a source group specific query arrives.
 * Build the recorded list merging the queried sources (pending response) with
 * our current list (sources in the interface state).
 * A is our current source list, B is the list of queried sources.
 * We build the current state record in the recorded list according
 * to the rules below where A*B means the intersection of set A, and set B,
 * and B-A means all sources in set B that are not in set A.
 * set of sources in the
 *     interface state   pending response record   Current-State Record
 *       INCLUDE (A)                B                   IS_IN (A*B)
 *       EXCLUDE (A)                B                   IS_IN (B-A)
 * To prevent DoS attacks, do not record more sources that could fit
 * in one packet.
 * 1. If IGMP entry is not local
 *   1.1 Compute the number of sources that could fit in the device MTU
 *   1.2 Point to the recorded source list
 *   1.3 recorded source list mode is INCLUDE: TM_MODE_IS_INCLUDE.
 *   1.4 Get the current source list filter mode.
 *   1.5 If current source list filter mode is INCLUDE, we can only build
 *       a recorded source list if both the queried source list and the
 *       current source list are non emptry (since we are taking the
 *       intersection of the two sets.)
 *   1.6 If there is something to report
 *    1.6.1 Check whether we have the special case B-0 (empty current list in
 *          Exclude mode. If we do set variable bMinusZero to 1.
 *    1.6.2. For each source address in the queried sources
 *      1.6.2.1 If we exceed the number of sources that can fit in the MTU,
 *              break.
 *      1.6.2.2 If bMinusZero is set the source address is to be reported.
 *      1.6.2.3 else
 *        1.6.2.3.1 Check if the source address is in the current source list
 *                  by getting the index to the source address in the list.
 *        1.6.2.3.2 If the current source list mode is EXCLUDE, the source
 *                  address is to be reported only if the source address is NOT
 *                  in the current source list (index == -1).
 *        1.6.2.3.3 If the current source list mode is INCLUDE, the source
 *                  address is to the reported nly if the source address IS
 *                  in the current source list.
 *      1.6.4 If the source address is to be reported, add it to the recorded
 *            source list.
 *      1.6.5 If we fail to add the source address, break out of the loop, but
 *            keep the built source list. (We will send what we have build so
 *            far.)
 * 2. Return.
 * Parameters
 * Parameter        Description
 * igmpEntryPtr     Pointer to IGMP entry.
 * srcAddrPtr       Pointer to queried sources
 * numSrc           Number of sources to query
 * Returns
 * Value            Meaning
 * None
 */
static void tfIgmpBuildRecordedList (
    ttIgmpEntryPtr          igmpEntryPtr,
    tt4IpAddressPtr         srcAddrPtr,
    tt16Bit                 numSrc)
{
    ttSourceListPtr srcListPtr;
    struct in_addr  srcAddress;
    int             i;
    int             index;
    int             searchIndex;
    int             addIndex;
    int             check;
    int             report;
    int             errorCode;
    tt16Bit         maxNumSrc;
    tt8Bit          bMinusZero;

    if (igmpEntryPtr->igmpState != TM_IGMP_LOCAL)
    {
/*
 * RFC3376 states:
 * "A DoS attack on a host could be staged through forged Group-and-
 *  Source-Specific Queries.  The attacker can find out about membership
 *  of a specific host with a general query.  After that it could send a
 *  large number of Group-and-Source-Specific queries, each with a large
 *  source list and the Maximum Response Time set to a large value.  The
 *  host will have to store and maintain the sources specified in all of
 *  those queries for as long as it takes to send the delayed response.
 *  This would consume both memory and CPU cycles in order to augment the
 *  recorded sources with the source lists included in the successive
 *  queries.
 *  To protect against such a DoS attack, a host stack implementation
 *  could restrict the number of Group-and-Source-Specific Queries per
 *  group membership within this interval, and/or record only a limited
 *  number of sources."
 *  We will restrict the maximum number of sources to the maximum number
 *  of sources that would fit in one packet.
 */
        maxNumSrc = tm_num_src(igmpEntryPtr->igmpDevPtr->devMtu
                                - TM_IP_OVERHEAD,
                               TM_IGMP_V3_REP_ONEGR_OVERHEAD);
/*
 * set of sources in the
 *     interface state   pending response record   Current-State Record
 *     ---------------   -----------------------   --------------------
 *
 *       INCLUDE (A)                B                   IS_IN (A*B)
 *
 *       EXCLUDE (A)                B                   IS_IN (B-A)
 *
 */
        srcListPtr = &igmpEntryPtr->igmpSrcList[TM_IGMP_RECORDED_LIST];
/* Recorded list is always in include mode */
        srcListPtr->srcMode = TM_MODE_IS_INCLUDE;
        check = igmpEntryPtr->igmpSrcList[TM_IGMP_CURRENT_LIST].srcMode;
        if (check == TM_MODE_IS_INCLUDE)
/*       INCLUDE (A)                B                   IS_IN (A*B) */
        {
            if (    (numSrc == TM_16BIT_ZERO)
                 || (igmpEntryPtr->igmpSrcList[TM_IGMP_CURRENT_LIST].
                                            srcCount == TM_16BIT_ZERO))
            {
/* Null intersection. Nothing to report */
                check = 0;
            }
        }
        if (check != 0)
/* Something to report */
        {
            if (    (check == TM_MODE_IS_EXCLUDE)
                 && (igmpEntryPtr->igmpSrcList[TM_IGMP_CURRENT_LIST].
                                                srcCount == 0))
            {
/* Special case B-0 */
                bMinusZero = TM_8BIT_YES;
            }
            else
            {
                bMinusZero = TM_8BIT_NO;
            }
            for (i = 0; i < numSrc; i++)
            {
/* DoS attack prevention. We limit the number of queried sources we record. */
                if (srcListPtr->srcCount >= maxNumSrc)
                {
                    break;
                }
                if (bMinusZero != TM_8BIT_ZERO)
/* Special case B-0 */
                {
/*       EXCLUDE (0)                B                   IS_IN (B-0) */
                    report = 1;
                }
                else
                {
                    searchIndex = 0; /* start of search */
                    index = tfIgmpCacheGetSource(
                             &igmpEntryPtr->igmpSrcList[TM_IGMP_CURRENT_LIST],
                             &searchIndex,
                             srcAddrPtr[i]);
                    if (check == TM_MODE_IS_EXCLUDE)
                    {
/*       EXCLUDE (A)                B                   IS_IN (B-A) */
                        report = (index == -1);
                    }
                    else
                    {
/*       INCLUDE (A)                B                   IS_IN (A*B) */
                        report = (index != -1);
                    }
                }
                if (report)
/* Source being queried, to be reported */
                {
                    addIndex = 0; /* start of search for addition */
                    tm_ip_copy(srcAddrPtr[i], srcAddress.s_addr);
                    errorCode = tfIgmpCacheAddSource(srcListPtr,
                                     &addIndex,
                                     &srcAddress,
                                     1,
                                     TM_IGMP_NETWORK_LIST, /* network list */
                                     TM_8BIT_NO);          /* not sorted */ 
                    if (errorCode != TM_ENOERROR)
/* Break out of the loop if we fail */
                    {
                        break;
                    }
                }
            }
        }
    }
    return;
}

/*
 * tfIgmpSendCurrentStateReport() Function Description
 * Allocate a packet (or used an already allocated one passed in
 * *packetPtrPtr) and build an IGMPv3 report based on the passed source list.
 * The passed source list is either the current source list, or the
 * recorded source list.
 * Use multiple packets if needed, as IGMPv3 reports cannot be fragmented.
 * If sources won't fit at the end of a previous packet, allocate a new packet.
 * In exclude mode, do not send more sources than can fit in one packet (per
 * RFC3376.)
 * Send all the packets that are full. The last non full packet is being
 * saved in *packetPtrPtr so that it can be re-used in the next call to this
 * routine.
 * NOTE that we postpone sending the packets until we have copied all the
 * sources from the source list, to prevent a re-entrancy problem that could
 * occur if we were to send a packet in the middle of reading the source list,
 * as the device is unlocked, and relocked when the packet is being sent.
 * Parameters
 * Parameter        Description
 * igmpEntryPtr     Pointer to IGMP entry.
 * srcAddrPtr       Pointer to source list to report
 * packetPtrPtr     Pointer to a packet pointer. Where to store pointer to a
 *                  packet we have not sent yet in this routine. Non null
 *                  only if called from the general query timer.
 * unlockedPtr      Where to store whether we have unlocked the device in 
 *                  this routine.
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Success
 * TM_ENOBUFS       Not enough memory to allocate a packet
 */
static int tfIgmpSendCurrentStateReport (
    ttIgmpEntryPtr  igmpEntryPtr, 
    ttSourceListPtr srcListPtr,
    ttPacketPtrPtr  packetPtrPtr,
    tt8BitPtr       unlockedPtr)
{
    ttDeviceEntryPtr        devEntryPtr;
    ttIgmpV3ReportPtr       igmpv3ReportPtr;
    ttIgmpGroupRecordPtr    igmpGrpRecPtr;
    ttPacketPtr             packetPtr;
    ttPacketPtr             newPacketPtr;
    ttPacketPtr             sendPacketPtr;
    struct in_addr TM_FAR * srcAddrPtr;
    int                     length;
    int                     maxLength;
    int                     errorCode;
    tt16Bit                 leftOverBytes;
    tt16Bit                 numSrc;
    tt16Bit                 maxNumSrc;
    tt16Bit                 totalNumSrc;
    tt16Bit                 numGroup;
    tt16Bit                 mode;

    errorCode = TM_ENOERROR;
#ifdef TM_USE_STOP_TRECK
    if (tm_16bit_one_bit_set(igmpEntryPtr->igmpDevPtr->devFlag2,
                             TM_DEVF2_UNINITIALIZING))
    {
        errorCode = TM_ESHUTDOWN;
    }
    else
#endif /* TM_USE_STOP_TRECK */
    {
        mode = srcListPtr->srcMode;
/* Number of sources to send */
        totalNumSrc = srcListPtr->srcCount;
        devEntryPtr = igmpEntryPtr->igmpDevPtr;
        maxLength = devEntryPtr->devMtu;
        if (maxLength < (TM_IP_OVERHEAD + TM_IGMP_V3_REP_ONEGR_OVERHEAD +4))
/*
 * The MTU should let us fill one IP header + V3 report header + one group
 * record + at least one address
 */
        {
            goto sendCurrentChangeReportExit;
        }
/* Number of sources that fit in one packet after the V3 report header, and
 * group record header
 */
        maxLength = maxLength - TM_IP_OVERHEAD;
        maxNumSrc = tm_num_src(maxLength, TM_IGMP_V3_REP_ONEGR_OVERHEAD);
        if (srcListPtr->srcMode == TM_MODE_IS_EXCLUDE)
/* In Exclude mode only send number of sources that fit in one packet */
        {
            if (totalNumSrc > maxNumSrc)
            {
                totalNumSrc = maxNumSrc;
            }
        }
        sendPacketPtr = (ttPacketPtr)0;
        if (packetPtrPtr != (ttPacketPtrPtr)0)
/* Called from general query timer */
        {
            sendPacketPtr = *packetPtrPtr;
            packetPtr = sendPacketPtr;
            newPacketPtr = packetPtr;
            *packetPtrPtr = (ttPacketPtr)0;
            if (newPacketPtr != (ttPacketPtr)0)
            {
                leftOverBytes =   ((tt16Bit)(tm_byte_count(
                              packetPtr->pktSharedDataPtr->dataBufLastPtr
                            - packetPtr->pktLinkDataPtr)));
                if (leftOverBytes <   tm_source_len(totalNumSrc)
                                    + TM_IGMP_V3_REP_ONEGR_OVERHEAD)
                {
/* If sources won't fit at the end of the previous packet, allocate a new one */
                    newPacketPtr = (ttPacketPtr)0;
                }
            }
        }
        else
        {
/*
 * Not called from a general query timer. No packetPtr passed to this
 * function
 */
            sendPacketPtr = (ttPacketPtr)0;
            packetPtr = (ttPacketPtr)0;
            newPacketPtr = (ttPacketPtr)0;
        }
/* Start of sources to send */
        srcAddrPtr = &srcListPtr->srcAddrPtr[0];
        do
        {
            numSrc = totalNumSrc;
            if (totalNumSrc > maxNumSrc)
            {
/* Number of sources that fit in one packet */
                numSrc = maxNumSrc;
            }
/* Number of sources left to send */
            totalNumSrc = (tt16Bit)(totalNumSrc - numSrc);
            if (newPacketPtr == (ttPacketPtr)0)
            {
                if (packetPtrPtr == (ttPacketPtrPtr)0)
                {
/* Only sending one list. Allocate space we need. */
                    length = TM_IGMP_V3_REP_ONEGR_OVERHEAD + tm_source_len(numSrc);
                }
                else
                {
/*
 * Called from general query timer:
 * Possibly sending multiple lists. Allocate up to the MTU.
 */
                    length = maxLength;
                }
/* Allocate the packet */
                newPacketPtr = 
                    tfGetSharedBuffer(
                    (int)
                    (TM_LL_MAX_HDR_LEN+TM_IP_OVERHEAD),
                    (ttPktLen)length,
                    TM_16BIT_ZERO);
                if (newPacketPtr == TM_PACKET_NULL_PTR)
                {
/* Failed to allocate */
                    errorCode = TM_ENOBUFS;
                    break;
                }
                newPacketPtr->pktMhomeIndex = igmpEntryPtr->igmpMhomeIndex;
                if (packetPtr == (ttPacketPtr)0)
                {
/* Head of the chain of packets to send */
                    sendPacketPtr = newPacketPtr;
                }
                else
/* Chain in the new packet at the end */
                {
                    packetPtr->pktChainNextPtr = newPacketPtr;
                }
/* Packet to build */
                packetPtr = newPacketPtr;
/* Pointer to where the IGMPv3 report starts */
                igmpv3ReportPtr = (ttIgmpV3ReportPtr)packetPtr->pktLinkDataPtr;
                igmpv3ReportPtr->igmph2Reserved = TM_16BIT_ZERO;
                igmpv3ReportPtr->igmphNumGroup = tm_const_htons(1);
/* First group record */
                igmpGrpRecPtr = &igmpv3ReportPtr->igmphGroupRecord;
                if (packetPtrPtr != (ttPacketPtrPtr)0)
/* Called from general query timer. Update the length as we add records */
                {
                    packetPtr->pktLinkDataLength =  TM_IGMP_V3_REP_ONEGR_OVERHEAD
                                                  + tm_source_len(numSrc);
                    packetPtr->pktChainDataLength = packetPtr->pktLinkDataLength;
                }
            }
            else
            {
/* 
 * Called from the general query timer 
 * Update the number of groups in the record, an adjust the length to take
 * into account the new record we are adding
 */
/* Pointer to where the IGMPv3 report starts */
                igmpv3ReportPtr = (ttIgmpV3ReportPtr)packetPtr->pktLinkDataPtr;
/* Pointer to where we want to add the next group record */
                igmpGrpRecPtr = (ttIgmpGroupRecordPtr)
                            (  (tt8BitPtr)(ttVoidPtr)igmpv3ReportPtr
                             + tm_packed_byte_count(packetPtr->pktLinkDataLength));
/* Number of group records we have added */
                tm_ntohs(igmpv3ReportPtr->igmphNumGroup, numGroup);
/* We are adding one more group record */
                numGroup++;
/* Update the IGMPv3 report header with the new number of group records */
                tm_htons(numGroup, igmpv3ReportPtr->igmphNumGroup);
/*
 * Called from general query timer. Update the length as we add records. Add 
 * the length of the group record we are adding
 */
                packetPtr->pktLinkDataLength +=   TM_IGMP_V3_GRP_LEN
                                                + tm_source_len(numSrc);
                packetPtr->pktChainDataLength = packetPtr->pktLinkDataLength;
/* 
 * General query timer:
 * End of updating the number of groups in the record, and adjust the length
 * to take into account the new record we are adding
 */
            }
/* Add the group record */
            igmpGrpRecPtr->igmpgRecordType = (tt8Bit)mode;
            igmpGrpRecPtr->igmpgAuxDataLen = TM_8BIT_ZERO;
            tm_htons(numSrc, igmpGrpRecPtr->igmpgNumSrc); 
            tm_ip_copy(igmpEntryPtr->igmpHostGroup,
                       igmpGrpRecPtr->igmpgGroupAddress);
            if (numSrc != TM_16BIT_ZERO)
            {
                tm_bcopy(srcAddrPtr,
                             igmpGrpRecPtr->igmpgSrcAddress,
                         tm_packed_byte_count(tm_source_len(numSrc)));
                srcAddrPtr += numSrc;
            }
/* End of adding the group record */
/*
 * Need to create a new packet, if there are more sources to send (i.e.
 * they did not fit in the current packet.)
 */
            newPacketPtr = (ttPacketPtr)0;
        } while (totalNumSrc != TM_16BIT_ZERO);
/*
 * If we send using the recorded source list. free it before we send the
 * packet(s) and therefore before we unlock/relock the device
 */
        if (srcListPtr == &igmpEntryPtr->igmpSrcList[TM_IGMP_RECORDED_LIST])
        {
            tfIgmpFreeSourceList(srcListPtr);
        }
/*
 * Loop through all created packets and send them, but do not send
 * the last one if we are called from the general query timer, and
 * the last one is not full.
 */
        while (sendPacketPtr != (ttPacketPtr)0)
        {
            newPacketPtr = sendPacketPtr->pktChainNextPtr;
            if (    (newPacketPtr == (ttPacketPtr)0)
                 && (packetPtrPtr != (ttPacketPtrPtr)0))
/* Last packet, and called from the general query timer */
            {
                leftOverBytes =   ((tt16Bit)(tm_byte_count(
                              packetPtr->pktSharedDataPtr->dataBufLastPtr
                            - packetPtr->pktLinkDataPtr)));
                if (leftOverBytes >= (TM_IGMP_V3_REP_ONEGR_OVERHEAD + 4))
/*
 * Called from general query timer, and room for more records. Postpone
 * sending.
 */
                {
                    *packetPtrPtr = sendPacketPtr;
                    break;
                }
            }
/* Unlink the packet */
            sendPacketPtr->pktChainNextPtr = (ttPacketPtr)0;
/*
 * Send the packet.
 * Device unlocked and relocked
 */
            tfIgmpSendPacket(devEntryPtr, sendPacketPtr,
                             TM_IGMP_V3_REPORT);
            if (unlockedPtr != (tt8BitPtr)0)
/* We sent a packet, and therefore unlocked/relocked the device */
            {
                *unlockedPtr = TM_8BIT_YES;
            }
            sendPacketPtr = newPacketPtr;
        }
    }
sendCurrentChangeReportExit:
/* return error code */
    return errorCode;
}

/*
 * tfIgmpSrcTimer function description
 * Called by the timer execute thread to re-send an IGMPv3 state change
 * report for robustness. (We send the IGMPv3 state change reports as
 * many times as the value in the robustness variable.)
 * If timer Expired, send report.
 * 1. Get igmp entry pointer from timer parameter
 * 2. Get the device from the igmp entry
 * 3. Lock the device
 * 4. If the timer has not been removed or re-intialized
 *   4.1 Get the IGMP entry owner count.
 *   4.2 If we detected a lower version router, stop this timer
 *   4.3 else send a state change report. If this timer needs to be removed
 *       it will be done in tfIgmpSendStateChangeReport() before the device
 *       is unlocked/relocked.
 *   4.4 If the owner count was zero before we sent the report, and we are
 *       done retransmitting, free the IGMP entry.
 * 5. Unlock the device
 * 6. Return.
 * Parameters
 * Parameter        Description
 * timerBlockPtr    Pointer to timer structure
 * userParm1        Timer parameter 1 (igmpEntryPtr)
 * userParm2        Timer parameter 2
 * Returns
 * NONE
 */
static void tfIgmpSrcTimer (ttVoidPtr      timerBlockPtr,
                            ttGenericUnion userParm1,
                            ttGenericUnion userParm2)
{
    ttTimerPtr       tmrPtr;
#ifdef TM_LOCK_NEEDED
    ttDeviceEntryPtr devEntryPtr;
#endif /* TM_LOCK_NEEDED */
    ttIgmpEntryPtr   igmpEntryPtr;
    tt16Bit          ownerCount;

/* Get Igmp entry from timer parameter */
    igmpEntryPtr = (ttIgmpEntryPtr)(userParm1.genVoidParmPtr);
/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);

#ifdef TM_LOCK_NEEDED
    devEntryPtr = igmpEntryPtr->igmpDevPtr;
#endif /* TM_LOCK_NEEDED */
    tm_call_lock_wait(&(devEntryPtr->devLockEntry));
    tmrPtr = igmpEntryPtr->igmpTimerPtr[TM_IGMP_SRC_TIMER];
    if ( (tmrPtr == timerBlockPtr) && tm_timer_not_reinit(tmrPtr) )
/* If timer has not been removed or re-initialized */
    {
/*
 * Check the owner count before we send the report, since we unlock/relock when
 * sending
 */
        ownerCount = igmpEntryPtr->igmpOwnerCount;
        if (tm_8bit_one_bit_set(igmpEntryPtr->igmpDevPtr->devIgmpVFlag,
                               (TM_IGMP_DEV_V1 | TM_IGMP_DEV_V2)))
/*
 * Double check that we did not detect a lower version router, since this IGMP
 * entry could have been dequeued prior to the detection, and as a result we
 * could not have removed this timer. Note that if a lower version router had
 * been detected after this IGMP entry had been dequeued, tfIgmpEntryFree()
 * will be called below, and all pending reports and timers will be cleaned up.
 */
        {
/* We detected a v1, or v2 router. Stop retransmission now. */
            tfIgmpSrcTimerRemove(igmpEntryPtr);
        }
        else
        {
/* 
 * Send the state change report. Device unlocked/relocked
 * Source timer removed in this routine if needed before the packets are
 * sent.
 */
            tfIgmpSendStateChangeReport(igmpEntryPtr);
        }
        if ((ownerCount == 0) && (igmpEntryPtr->igmpSrcRobustness == 0))
/* Done retransmitting, and owner count is zero */
        {
/*
 * If ownerCount was zero before we sent, that means that the IGMP entry was
 * dequeued, and therefore it is safe to free after we send, as nobody else
 * would have had access to it.
 */
            tfIgmpEntryFree(igmpEntryPtr);
        }
    }
    tm_call_unlock(&(devEntryPtr->devLockEntry));
    return;
}

/*
 * tfIgmpSrcTimerRemove Function Description
 * Remove the IGMPv3 state change report timer if any for a given IGMP entry.
 * If the IGMPv3 source timer is running on the IGMP entry
 * . Remove it
 * . Reset the robustness variable (number of retries) on the entry.
 * Parameters
 * Parameter        Description
 * igmpEntryPtr     Pointer to IGMP entry
 * Returns
 * NONE
 */
static void tfIgmpSrcTimerRemove (ttIgmpEntryPtr igmpEntryPtr)
{
    if ( igmpEntryPtr->igmpTimerPtr[TM_IGMP_SRC_TIMER] != (ttTimerPtr)0 )
/* Delaying member state */
    {
/* Delete the timer */
        tfIgmpTimerRemove(igmpEntryPtr, TM_IGMP_SRC_TIMER);
        igmpEntryPtr->igmpSrcRobustness = 0; /* no retries */
    }
    return;
}

/*
 * tfIgmpDevQueryTimerRemove Function Description
 * Remove the general IGMPv3 query timer if any for a given device entry.
 * If a general query IGMPv3 Timer is running on the device entry
 * . Remove it
 * . Reset the timer pointer
 * Parameters
 * Parameter        Description
 * devEntryPtr      Pointer to device entry
 * Returns
 * NONE
 */
static void tfIgmpDevQueryTimerRemove(ttDeviceEntryPtr devEntryPtr)
{
    if ( devEntryPtr->devIgmpQueryTmrPtr != (ttTimerPtr)0 )
/* Delaying member state */
    {
/* Delete the timer */
        tm_timer_remove(devEntryPtr->devIgmpQueryTmrPtr);
/* Reset timer field */
        devEntryPtr->devIgmpQueryTmrPtr = (ttTimerPtr)0;
    }
    return;
}

/*
 * tfIgmpSocketCacheEntryPurge Function Description
 * Called by tfIgmpSocketCachePurge()
 * List walk call back function called per socket mcast cache entry when we
 * want to purge the socket mcast cache.
 * Note that the socket mcast cache is detached from the socket while we
 * walk the list to ensure re-entrancy.
 * . Get internal mcast request structure from general parameter
 * . Copy info from socket mcast cache entry into that structure
 * . Lock the device
 * . Call tfIgmpEntryLeaveHostGroup
 * . relock the device
 * . Free the source list, and socket mcast cache entry
 * . Return 0, so that we keep walking the list.
 * Parameters
 * Parameter        Description
 * nodePtr          Pointer to node on the list which is a socket mcast cache
 *                  entry.
 * genParam1        parameter containing a pointer to the internal mcast
 *                  request structure.
 * Returns
 * Value            Meaning
 * 0                Keep walking the list
 */
static int tfIgmpSocketCacheEntryPurge (ttNodePtr      nodePtr,
                                        ttGenericUnion genParm1)
{
    ttSockIgmpInfoPtr           sockIgmpInfoPtr;
    ttDeviceEntryPtr            devEntryPtr;
    ttIpMreqSrcPtr              imrIntPtr;
    ttSourceListPtr             oldSrcListPtr;

    TM_UNREF_IN_ARG(genParm1);
/* Socket mcast cache entry */
    sockIgmpInfoPtr = (ttSockIgmpInfoPtr)(ttVoidPtr)nodePtr;
    devEntryPtr = sockIgmpInfoPtr->siiDevicePtr;
/* Source list on the socket mcast cache entry */
    oldSrcListPtr = &sockIgmpInfoPtr->siiSourceList;
/* internal mcast request structure */
    imrIntPtr = (ttIpMreqSrcPtr)genParm1.genVoidParmPtr;
/*
 * Initialize internal mcast request structure device entry pointer field, and
 * mcast address field from the socket mcast cache entry
 */
    imrIntPtr->imrDevPtr = devEntryPtr;
    tm_ip_copy(sockIgmpInfoPtr->siiMcastAddr, imrIntPtr->imrMcastAddr);
/* Unlock the device */
    tm_call_lock_wait(&(devEntryPtr->devLockEntry));
/* leave multicast group we are still joined to */
    (void)tfIgmpLeaveHostGroup(TM_8BIT_YES,
                               imrIntPtr
#ifdef TM_USE_IGMPV3
                              ,oldSrcListPtr
#endif /* TM_USE_IGMPV3 */
                               );
/* Relock the device */
    tm_call_unlock(&(devEntryPtr->devLockEntry));
/* Done with the node. Free the source list, and socket mcast cache entry. */
    tfIgmpFreeSourceList(oldSrcListPtr);
    tm_free_raw_buffer(nodePtr);
    return 0; /* keep walking */
}

/*
 * tfIgmpSocketCacheEntryMatch Function Description
 * Called by tfIgmpSocketCacheGetParams()
 * List walk call back function when we want to find a match in the socket
 * mcast cache
 * Compare the socket mcast cache entry multicast group address, and device
 * entry pointer with the ones stored in the structure pointed to by the
 * parameter. If they match return 1 (stop the walk), otherwise return 0
 * (keep walking).
 *
 * Parameters
 * Parameter        Description
 * nodePtr          Pointer to node on the list which is a socket mcast cache
 *                  entry.
 * genParam1        parameter containing a pointer to a structure containing
 *                  the multicast group address, and device entry pointer.
 * Returns
 * Value            Meaning
 * 1                We found a match. Stop walking the list.
 * 0                Keep walking the list
 */
static int tfIgmpSocketCacheEntryMatch (ttNodePtr      nodePtr,
                                        ttGenericUnion genParm1)
{
    ttSockIgmpInfoPtr           sockIgmpInfoPtr;
    ttIgmpMatchPtr              igmpMatchPtr;
    int                         retCode;

    sockIgmpInfoPtr = (ttSockIgmpInfoPtr)(ttVoidPtr)nodePtr;
    igmpMatchPtr = (ttIgmpMatchPtr)(genParm1.genVoidParmPtr);
    if (    (tm_ip_match(sockIgmpInfoPtr->siiMcastAddr,
                         igmpMatchPtr->iiMcastAddr))
         && (sockIgmpInfoPtr->siiDevicePtr == igmpMatchPtr->iiDevicePtr))
    {
        retCode = 1; /* match */
    }
    else
    {
        retCode = 0; /* keep walking */
    }
    return retCode;
}

/*
 * tfIgmpChangeSource Function Description
 * Called by tfIgmpSocketSourceCommand(), and tfComSetipv4sourcefilter()
 * Convey sources' change and/or filter mode's change to IGMP. IGMP will
 * send a state change report if the changes at the socket level, translate
 * into a state change at the IGMP level, and if we have not detected a lower
 * version router.
 * Socket unlocked/relocked
 * 1. Retrieve socket entry pointer from internal mcast request structure.
 * 2. Unlock the socket.
 * 3. Lock the device
 * 4. Find matching IGMP entry
 * 5. If found, call tfIgmpEntryChangeSource()
 * 6. Unlock the device
 * 7. Relock the socket.
 * 8. Return.
 * Parameters
 * Parameter            Description
 * imrIntPtr            Pointer to internal mcast request structure conveying
 *                      parameters set by the user in the user API.
 * oldSrcListPtr        Previous socket source list
 * deltaEmptyExclJoin   indicates if the socket transition to or from
 *                      exclude mode with no source.
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Success
 * TM_ENOBUFS       Not enough memory to store the source(s).
 */
int tfIgmpChangeSource(
    ttIpMreqSrcPtr  imrIntPtr,
    ttSourceListPtr oldSrcListPtr,
    int             deltaEmptyExclJoin)
{
    ttSocketEntryPtr socketEntryPtr;
    ttDeviceEntryPtr devEntryPtr;
    ttIgmpEntryPtr   igmpEntryPtr;
    int              errorCode;

    errorCode = TM_EADDRNOTAVAIL;
    socketEntryPtr = imrIntPtr->imrSocketPtr;
/* Unlock socket entry because we are locking the device */
    tm_call_unlock(&(socketEntryPtr->socLockEntry));
/* Lock the interface */
    devEntryPtr = imrIntPtr->imrDevPtr;
    tm_call_lock_wait(&(devEntryPtr->devLockEntry));
    igmpEntryPtr = tfIgmpFindEntry(imrIntPtr->imrMcastAddr,
                                   imrIntPtr->imrDevPtr);
    if (igmpEntryPtr != (ttIgmpEntryPtr)0)
    {
/* Device potentially unlocked/relocked. */
        errorCode = tfIgmpEntryChangeSource(igmpEntryPtr, imrIntPtr,
                                            oldSrcListPtr,
                                            deltaEmptyExclJoin,
                                            TM_8BIT_NO);
    }
/* Unlock device */
    tm_call_unlock(&(devEntryPtr->devLockEntry));
/* relock socket entry */
    tm_call_lock_wait(&(socketEntryPtr->socLockEntry));
    return errorCode;
}

/*
 * tfIgmpEntryChangeSource Function Description
 * Called by tfIgmpChangeSource(), and tfIgmpLeaveHostGroup(), and
 * tfIgmpJoinHostGroup().
 * Tell IGMP of source list changes from the socket, possibly sending an
 * IGMPV3 state change report if the changes at the socket level, translate
 * into a state change at the IGMP level, and if we have not detected a lower
 * version router.
 * Called with device locked.
 * Device potentially unlocked/relocked in this function.
 * 1. Save user filter mode (TM_MODE_IS_INCLUDE, or TM_MODE_IS_EXCLUDE).
 * 1. Add deltaEmptyExclJoin to igmpCurNumEmptyExcl
 * 2. Detect if we transition to and from 0 as we would need to send a report
 *    in that case.
 * 3. If the user is adding new/dropping old sources, backup the source list
 *    corresponding to the user filter mode saved in step 1.
 * 4. Add or drop the new sources according to the user command.
 * 5. If the user changed an old source list, we need to back up
 *    the corresponding IGMP source list (but only if we are not leaving the
 *    group as we need to leave the group even if there is an error. We just
 *    need to make sure that the IGMP source lists are in synch with the
 *    socket source lists, and not backing up the old IGMP source list
 *    will ensure that, as we could fail backing up the source list, but won't
 *    fail dropping sources from an existing list). Then drop the sources in
 *    the old source list from the corresponding IGMP source list.
 * 6. If we made any change, we need to build the current source list, and 
 *    state change report lists, and verify that the change results in
 *    our need to send a state change report.
 * 7. If an error occured, we need to move the backup lists (if any) to their
 *    original lists to undo any change, otherwise (no error), we need to free
 *    the backup lists.
 * 8. If any change occured with no error:
 *   8.1 If there is no IGMPv3 source timer for this entry
 *     8.1.1 add the IGMPv3 source timer that will take care of retransmitting
 *           the state change report.
 *     8.1.2 transmit the first state change report
 *   8.2 else there is an IGMPv3 source timer for this entry:
 *     8.2.1 refresh the timer only if the pending report is a source change
 *           report.
 * Note:
 * We backup the lists that we are about to change, so that we cannot fail
 * when we restore the lists if an error occured. This is to ensure that the
 * IGMP source lists are still in synch with the socket source lists in case
 * of an error.
 * Parameters
 * Parameter            Description
 * igmpentryPtr         Pointer to IGMP entry
 * imrIntPtr            Pointer to internal mcast request structure conveying
 *                      parameters set by the user in the user API.
 * oldSrcListPtr        Previous socket source list
 * deltaEmptyExclJoin   indicates if the socket transition to or from
 *                      exclude mode with no source.
 * ignoreError          Ignore error (set when leaving a group, as we have
 *                      to leave in that case, and should ignore the error
 *                      if we fail to allocate memory for a report.)
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Success
 * TM_ENOBUFS       Not enough memory to store the source(s).
 */
static int tfIgmpEntryChangeSource (
    ttIgmpEntryPtr  igmpEntryPtr,
    ttIpMreqSrcPtr  imrIntPtr,
    ttSourceListPtr oldSrcListPtr,
    int             deltaEmptyExclJoin,
    tt8Bit          ignoreError)
{
    ttSourceListPtr  igmpSrcListPtr;
    tt32Bit          timeout;
    ttGenericUnion   timerParm1;
    ttSourceList     backupSrcList[2];
    int              errorCode;
    int              i;
    int              mode;
    int              backupList[2];
    tt16Bit          savCurNumEmptyExcl;
    tt8Bit           change;

    errorCode = TM_ENOERROR;
    change = TM_8BIT_NO;
    mode = imrIntPtr->imrFmode;
    backupList[0] = 0; 
    backupList[1] = 0;
    savCurNumEmptyExcl = igmpEntryPtr->igmpCurNumEmptyExcl;
    if (deltaEmptyExclJoin != 0)
    {
        if (deltaEmptyExclJoin > 0)
        {
            igmpEntryPtr->igmpCurNumEmptyExcl++;
        }
        else
        {
            igmpEntryPtr->igmpCurNumEmptyExcl--;
        }
        if (    (savCurNumEmptyExcl == TM_16BIT_ZERO)
             || (igmpEntryPtr->igmpCurNumEmptyExcl == TM_16BIT_ZERO))
/* transition to or from 0 */
        {
            change = TM_8BIT_YES;
        }
    }
    if (imrIntPtr->imrNumSrc != 0)
    {
/*
 * Copy source list to backup, not freeing original source list. No need
 * to check for ignoreError, as we only ignore the error returned when
 * we leave the group, and therefore when imrNumSrc is zero.
 */
        errorCode = tfIgmpCopySourceList(&igmpEntryPtr->igmpSrcList[mode],
                                         &backupSrcList[0]);
        if (errorCode == TM_ENOERROR)
        {
            backupList[0] = mode;
        }
    }
    if (errorCode == TM_ENOERROR)
    {
        if (imrIntPtr->imrNumSrc != 0)
        {
            igmpSrcListPtr = &igmpEntryPtr->igmpSrcList[mode];
/* Add/dropping new sources */
            if (imrIntPtr->imrSrcCommand == TM_IGMP_CMND_ADD_SRC)
            {
/* Adding new sources */
                i = 0; /* start of search for addition */
                errorCode = tfIgmpCacheAddSource(
                                igmpSrcListPtr,
                                &i,
                                imrIntPtr->imrSrcAddrPtr,
                                imrIntPtr->imrNumSrc,
                                TM_IGMP_IGMP_LIST, /* IGMP list */
                                TM_8BIT_NO);       /* not sorted */
            }
            else
            {
/* Dropping new sources */
                (void)tfIgmpCacheDropSource(
                                igmpSrcListPtr,
                                imrIntPtr->imrSrcAddrPtr,
                                imrIntPtr->imrNumSrc,
                                TM_IGMP_IGMP_LIST, /* IGMP list */
                                TM_8BIT_NO);       /* not sorted */
            }
            change = TM_8BIT_YES;
        }
        if (errorCode == TM_ENOERROR)
        {
            if (oldSrcListPtr != (ttSourceListPtr)0)
            {
                if (oldSrcListPtr->srcCount != TM_16BIT_ZERO)
                {
/* we are dropping old sources */
                    mode = (int)oldSrcListPtr->srcMode;
                    igmpSrcListPtr =
                        &igmpEntryPtr->igmpSrcList[mode];
                    if (ignoreError == TM_8BIT_NO)
/*
 * Only backup if the caller is not going to ignore the error returned;
 * otherwise we need to not undo the changes in case of failure as the socket
 * source lists and kernel source lists have to be in synch, and the socket
 * source lists won't be undone when ignoreError is set.
 */
                    {
                        if (oldSrcListPtr->srcMode != backupList[0])
/* Source list not already backed up */
                        {
/* Copy source list to backup, not freeing original source list */
                            errorCode = tfIgmpCopySourceList(
                                             &igmpEntryPtr->igmpSrcList[mode],
                                             &backupSrcList[1]);
                            if (errorCode == TM_ENOERROR)
                            {
                                backupList[1] = mode;
                            }
                        }
                    }
                    if (errorCode == TM_ENOERROR)
                    {
/* Drop all addresses in the old ource list */
                        (void)tfIgmpCacheDropSource(
                            igmpSrcListPtr,
                            oldSrcListPtr->srcAddrPtr,
                            oldSrcListPtr->srcCount,
                            TM_IGMP_IGMP_LIST, /* IGMP list */
                            TM_8BIT_YES);      /* sorted */
                        change = TM_8BIT_YES;
                    }
                }
            }
            if (change != TM_8BIT_NO)
            {
/* 
 * Keep track of changes. Build new current source list. If the new source
 * list is different from the current one, then build the state change
 * report lists. If they are not different, reset change.
 */
                errorCode = tfIgmpBuildCurSrcListNReports(igmpEntryPtr,
                                                           &change);
            }
        }
/* Clean up: If an error occured undo changes, ortherwise free the backup */
        for (i = 0; i <= 1; i++)
        {
            mode = backupList[i];
            if (mode != 0)
            {
/*
 * If we failed, restore the source list we backed up, otherwise free the
 * backup list
 */
                tfIgmpRestoreSourceList(&backupSrcList[i],
                                        &igmpEntryPtr->igmpSrcList[mode],
                                        errorCode);
            }
        }
        if ((errorCode != TM_ENOERROR) && (ignoreError == TM_8BIT_NO))
        {
/* Only restore if the user is not going to ignore the returned error. */
            igmpEntryPtr->igmpCurNumEmptyExcl = savCurNumEmptyExcl;
        }
    }
    if (   (change != TM_8BIT_NO)
        && (errorCode == TM_ENOERROR))
/* We successfully built the state change report lists. Send them */
    {
/*
 * Initialize the time out with a random value between 0, and
 * tvIgmpUnsolReportIntv time value (in milliseconds).
 */
        timeout = tfIgmpRandom( (tt32Bit) ( 
                            ((tt32Bit)(tm_context(tvIgmpUnsolReportIntv))) *
                                        TM_UL(1000) ) );
        if (igmpEntryPtr->igmpTimerPtr[TM_IGMP_SRC_TIMER] == (ttTimerPtr)0)
        {
/* Refress the robustness variable, so that the new change is retransmitted. */
            igmpEntryPtr->igmpSrcRobustness =
                     igmpEntryPtr->igmpDevPtr->devIgmpRobustness;
            timerParm1.genVoidParmPtr = (ttVoidPtr)igmpEntryPtr;
            igmpEntryPtr->igmpTimerPtr[TM_IGMP_SRC_TIMER] = 
                    tfTimerAdd( tfIgmpSrcTimer,
                                timerParm1,
                                timerParm1, /* unused */
                                timeout,
                                TM_TIM_AUTO );
            if (igmpEntryPtr->igmpTimerPtr[TM_IGMP_SRC_TIMER]
                                                            == TM_TMR_NULL_PTR)
            {
/* Only send one report if we fail to allocate a timer */
                igmpEntryPtr->igmpSrcRobustness = 1;
                if (ignoreError)
                {
/*
 * If are leaving the group, and we fail to start the timer, set the errorCode
 * so that the entry can be freed
 */
                    errorCode = TM_ENOBUFS;
                }
            }
/* Build state change report packet(s), and send */
            tfIgmpSendStateChangeReport(igmpEntryPtr);
        }
        else
        {
/*
 * Refress the robustness variable, if the report currently pending is
 * a source change report
 */
            if (    (igmpEntryPtr->igmpSrcList[
                        TM_CHANGE_TO_INCLUDE_MODE].srcMode
                                     == TM_IGMP_INVALID_MODE)
                 && (igmpEntryPtr->igmpSrcList[
                     TM_CHANGE_TO_EXCLUDE_MODE].srcMode
                                     == TM_IGMP_INVALID_MODE) )
            {
                igmpEntryPtr->igmpSrcRobustness =
                     igmpEntryPtr->igmpDevPtr->devIgmpRobustness;
            }
        }
    }
    return errorCode;
}

/*
 * tfIgmpSendStateChangeReport Function Description
 * Called by tfIgmpEntryChangeSource(), and tfIgmpSrcTimer()
 * Allocate (a) packet(s) and build an IGMPv3 report based on the state change
 * report source lists.
 * Use multiple packets if needed, as IGMPv3 reports cannot be fragmented.
 * If sources won't fit at the end of a previous packet, allocate a new packet.
 * If the report is a change to exclude mode, do not send more sources than
 * can fit in one packet (per RFC3376.)
 * We are either sending a filter mode change report (either TO_EX() or TO_IN)
 * or we are sending source change report(s) (ALLOW() and/or BLOCK()).
 * If there is a filter mode change report, we need to send that one first.
 * the source change report(s) will be scheduled to be sent later.
 * If we have reached our number of retransmissions, check if we need to
 * remove the timer, or refresh the number of transmissions (if there are
 * pending source change reports) BEFORE sending the packets.
 * NOTE that we postpone sending the packets until we have copied all the
 * sources from the change report source lists, to prevent a re-entrancy
 * problem that could occur if we were to send a packet in the middle of
 * reading a state change report source list, as the device is unlocked, and
 * relocked when the packet is being sent.
 * Parameters
 * Parameter            Description
 * igmpentryPtr         Pointer to IGMP entry
 * Returns
 * Value                Meaning
 * TM_ENOERROR          Success
 * TM_ENOBUFS           Not enough memory to build the packet(s).
 */
static void tfIgmpSendStateChangeReport (ttIgmpEntryPtr igmpEntryPtr)
{
    ttDeviceEntryPtr        devEntryPtr;
    ttIgmpV3ReportPtr       igmpv3ReportPtr;
    ttIgmpGroupRecordPtr    igmpGrpRecPtr;
    ttPacketPtr             packetPtr;
    ttPacketPtr             newPacketPtr;
    ttPacketPtr             sendPacketPtr;
    struct in_addr TM_FAR * srcAddrPtr;
    int                     i;
    int                     index;
    int                     length;
    int                     maxLength;
    int                     extraLength;
    tt16Bit                 numSrc;
    tt16Bit                 maxNumSrc;
    tt16Bit                 totalNumSrc;
    tt16Bit                 extraNumSrc;
    tt16Bit                 extraMaxNumSrc;
    tt8Bit                  unlocked;

#ifdef TM_USE_STOP_TRECK
    if (tm_16bit_bits_not_set(igmpEntryPtr->igmpDevPtr->devFlag2,
                              TM_DEVF2_UNINITIALIZING))
#endif /* TM_USE_STOP_TRECK */
    {
        unlocked = TM_8BIT_NO;
/* Decrease the number of transmissions */
        --igmpEntryPtr->igmpSrcRobustness; 
        index = TM_BLOCK_OLD_SOURCES; /* compiler */
        for (i = 0; i < TM_IGMP_MAX_STATE_CHANGE_REPORTS; i++)
/*
 * Check if there is a non empty state change report to send. We check the
 * filter mode changes first.
 */
        {
            index = i + TM_CHANGE_TO_INCLUDE_MODE;
            if (igmpEntryPtr->igmpSrcList[index].srcMode != TM_IGMP_INVALID_MODE)
            {
                if (    (index <= TM_CHANGE_TO_EXCLUDE_MODE)
                     || (igmpEntryPtr->igmpSrcList[index].srcCount != 0))
                {
/* something to report */
                    break;
                }
            }
        }
        if (index <= TM_BLOCK_OLD_SOURCES)
/* something to report */
        {
/* Number of sources to send */
            totalNumSrc = igmpEntryPtr->igmpSrcList[index].srcCount;
            devEntryPtr = igmpEntryPtr->igmpDevPtr;
            maxLength = devEntryPtr->devMtu;
            if (maxLength < (TM_IP_OVERHEAD + TM_IGMP_V3_REP_ONEGR_OVERHEAD +4))
/*
 * The MTU should let us fill one IP header + V3 report header + one group
 * record + at least one address
 */
            {
                goto sendStateChangeReportExit;
            }
/*
 * Number of sources that fit in one message after the V3 report header, and
 * group record header
 */
            maxLength = maxLength - TM_IP_OVERHEAD;
            maxNumSrc = tm_num_src(maxLength, TM_IGMP_V3_REP_ONEGR_OVERHEAD);
            if (index == TM_CHANGE_TO_EXCLUDE_MODE)
/* In Exclude mode only send number of sources that fit in one message */
            {
                if (totalNumSrc > maxNumSrc)
                {
                    totalNumSrc = maxNumSrc;
                }
            }
            packetPtr = (ttPacketPtr)0;
            sendPacketPtr = (ttPacketPtr)0;
/* Start of sources to send */
            srcAddrPtr = &igmpEntryPtr->igmpSrcList[index].srcAddrPtr[0];
            extraNumSrc = TM_16BIT_ZERO;
            do
            {
                numSrc = totalNumSrc;
                if (totalNumSrc > maxNumSrc)
                {
/* Truncate to sources that will fit in one packet */
                    numSrc = maxNumSrc;
                }
/* Left over sources to send in the next iteration of the loop */
                totalNumSrc = (tt16Bit)(totalNumSrc - numSrc);
                length = TM_IGMP_V3_REP_ONEGR_OVERHEAD + tm_source_len(numSrc);
                if ((length < maxLength) && (index == TM_ALLOW_NEW_SOURCES))
/*
 * If we are sending an ALLOW(), and we have not fully allocated, check if we
 * need to allocate room for a BLOCK()
 */
                {
                    extraNumSrc = igmpEntryPtr->igmpSrcList[
                                        TM_BLOCK_OLD_SOURCES].srcCount;
                    if (extraNumSrc != TM_16BIT_ZERO)
/* sending a block */
                    {
                        extraLength = maxLength - length;
                        if ((extraLength) > (TM_IGMP_V3_GRP_LEN + 4))
/* room for more one group record and one source address */
                        {
                            extraMaxNumSrc = tm_num_src(extraLength,
                                                        TM_IGMP_V3_GRP_LEN);
                            if (extraNumSrc > extraMaxNumSrc)
                            {
                                extraNumSrc = extraMaxNumSrc;
                            }
                            length =   length + TM_IGMP_V3_GRP_LEN
                                     + tm_source_len(extraNumSrc);
                        }
                    }
                }
/* Allocate new packet */
                newPacketPtr = 
                    tfGetSharedBuffer(
                    (int)
                    (TM_LL_MAX_HDR_LEN+TM_IP_OVERHEAD),
                    (ttPktLen)length,
                    TM_16BIT_ZERO);
                if (newPacketPtr == TM_PACKET_NULL_PTR)
                {
/* Failed to allocate */
                    break;
                }
                newPacketPtr->pktMhomeIndex = igmpEntryPtr->igmpMhomeIndex;
                if (packetPtr == (ttPacketPtr)0)
                {
/* Head of the chain */
                    sendPacketPtr = newPacketPtr;
                }
                else
/*
 * Chain in the new packet, as we postpone sending the packets until we are
 * done copying all the sources.
 */
                {
                    packetPtr->pktChainNextPtr = newPacketPtr;
                }
                packetPtr = newPacketPtr;
/* Point to IGMPv3 report */
                igmpv3ReportPtr = (ttIgmpV3ReportPtr)packetPtr->pktLinkDataPtr;
                igmpv3ReportPtr->igmph2Reserved = TM_16BIT_ZERO;
/* One group */
                igmpv3ReportPtr->igmphNumGroup = tm_const_htons(1);
/* Point to first group record */
                igmpGrpRecPtr = &igmpv3ReportPtr->igmphGroupRecord;
                igmpGrpRecPtr->igmpgRecordType = (tt8Bit)index;
                igmpGrpRecPtr->igmpgAuxDataLen = TM_8BIT_ZERO;
/* Number of sources in the group record */
                tm_htons(numSrc, igmpGrpRecPtr->igmpgNumSrc); 
                tm_ip_copy(igmpEntryPtr->igmpHostGroup,
                           igmpGrpRecPtr->igmpgGroupAddress);
                if (numSrc != TM_16BIT_ZERO)
                {
/* Copy the sources */
                    tm_bcopy(srcAddrPtr,
                             igmpGrpRecPtr->igmpgSrcAddress,
                             tm_packed_byte_count(tm_source_len(numSrc)));
                    srcAddrPtr += numSrc;
                }
                if (   (totalNumSrc == TM_16BIT_ZERO)
                    && (index == TM_ALLOW_NEW_SOURCES))
/* Done with sending ALLOW(). Check if we need to send BLOCK(). */
                {
                    totalNumSrc = igmpEntryPtr->igmpSrcList[
                                  TM_BLOCK_OLD_SOURCES].srcCount;
                    if (totalNumSrc != TM_16BIT_ZERO)
                    {
                        index = TM_BLOCK_OLD_SOURCES;
                        srcAddrPtr = &igmpEntryPtr->igmpSrcList[index].
                                                                srcAddrPtr[0];
                        if (extraNumSrc != 0)
/* room at the end for one group record and at least one address */
                        {
/* Next group record */
                            igmpGrpRecPtr = (ttIgmpGroupRecordPtr)
                                  (&(igmpGrpRecPtr->igmpgSrcAddress[0]) + numSrc);

                            numSrc = extraNumSrc;
/* Update the number of groups in the IGMPv3 report header */
                            igmpv3ReportPtr->igmphNumGroup = tm_const_htons(2);
/* build the next group record */
                            igmpGrpRecPtr->igmpgRecordType = (tt8Bit)index;
                            igmpGrpRecPtr->igmpgAuxDataLen = TM_8BIT_ZERO;
                            tm_htons(numSrc, igmpGrpRecPtr->igmpgNumSrc); 
                            tm_ip_copy(igmpEntryPtr->igmpHostGroup,
                                       igmpGrpRecPtr->igmpgGroupAddress);
/* Copy the sources in the record */
                            tm_bcopy(srcAddrPtr,
                                     igmpGrpRecPtr->igmpgSrcAddress,
                                     tm_packed_byte_count(tm_source_len(numSrc)));
                            srcAddrPtr += numSrc;
/* Adjust number of sources that still need to be sent in the next iteration */
                            totalNumSrc = (tt16Bit)(totalNumSrc - numSrc);
                            numSrc = totalNumSrc;
                        }
                    }
                }
/* Loop until all sources have been copied in the packet(s) */
            } while (totalNumSrc != TM_16BIT_ZERO);
            if (sendPacketPtr != (ttPacketPtr)0)
            {
/* We will unlock the device when sending the packets below. Clean up now. */
                unlocked = TM_8BIT_YES;
                if (igmpEntryPtr->igmpSrcRobustness == 0)
                {
/*
 * Done with the source timer. Cleanup, and check if we need to schedule
 * ALLOW and/or BLOCK reports
 */
                    tfIgmpSrcTimerDoneOrNewReports(igmpEntryPtr, index);
                }
            }
            while (sendPacketPtr != (ttPacketPtr)0)
/* Send all the packets we have built */
            {
                newPacketPtr = sendPacketPtr->pktChainNextPtr;
                sendPacketPtr->pktChainNextPtr = (ttPacketPtr)0;
                tfIgmpSendPacket(igmpEntryPtr->igmpDevPtr, sendPacketPtr,
                                 TM_IGMP_V3_REPORT);
                sendPacketPtr = newPacketPtr;
            }
        }
sendStateChangeReportExit:
        if (     (unlocked == TM_8BIT_NO)
             && (igmpEntryPtr->igmpSrcRobustness == 0) )
        {
/*
 * Done with the source timer, and we did not send anything. Cleanup, and
 * check if we need to schedule ALLOW and/or BLOCK reports
 */
            tfIgmpSrcTimerDoneOrNewReports(igmpEntryPtr, index);
        }
    }
    return;
}

/*
 * tfIgmpSrcTimerDoneOrNewReports Function Description
 * Done with the source timer (igmpSrcRobustness is 0). Cleanup, and check if
 * we need to schedule ALLOW and/or BLOCK reports
 * 1. If we are done retransmitting
 *   1.1 If we just sent a filter mode change report
 *      1.1.1 Free the filter mode change source list
 *      1.1.2 If there is a non empty source change report source list,
 *             refresh the number of transmissions.
 *   1.2 else we just sent source change report(s), free the pending source
 *       change report lists.
 *   1.3 Remove the IGMPv3 source timer if we have not refreshed the number
 *       of transmissions.
 *
 * Parameters
 * Parameter            Description
 * igmpentryPtr         Pointer to IGMP entry
 * index                Index of change report source list that has just been
 *                      transmitted.
 * Returns
 * NONE
 */
static void tfIgmpSrcTimerDoneOrNewReports (
    ttIgmpEntryPtr igmpEntryPtr,
    int index)
{
    int i;

    if (igmpEntryPtr->igmpSrcRobustness == 0)
    {
        if (index <= TM_CHANGE_TO_EXCLUDE_MODE)
/* filter mode change */
        {
            tfIgmpCancelPendingReports(igmpEntryPtr, 0,
                                       TM_IGMP_MAX_FILTERMODE_REPORTS);
            for (i = 0; i < TM_IGMP_MAX_SRCLIST_REPORTS; i++)
            {
                index = i + TM_ALLOW_NEW_SOURCES;
                if (igmpEntryPtr->igmpSrcList[index].srcCount != 0)
                {
/* Schedule the source list reports for later */
                    igmpEntryPtr->igmpSrcRobustness =
                                 igmpEntryPtr->igmpDevPtr->devIgmpRobustness;
                    break;
                }
            }
        }
        else
/* Source list change */
        {
            tfIgmpCancelPendingReports(igmpEntryPtr,
                                       TM_IGMP_MAX_FILTERMODE_REPORTS, 
                                       TM_IGMP_MAX_SRCLIST_REPORTS);
        }
        if (igmpEntryPtr->igmpSrcRobustness == 0)
        {
            tfIgmpSrcTimerRemove(igmpEntryPtr);
        }
    }
}

/*
 * Restore previously backed up source list because an error occured during
 * processing (unable to allocate enough memory to add new sources to a list.)
 * Otherwise free the backed up source list.
 */
void tfIgmpRestoreSourceList (ttSourceListPtr backupSrcListPtr,
                              ttSourceListPtr srcListPtr,
                              int             errorCode)
{
    if (errorCode != TM_ENOERROR)
/* We failed. Restore the previous current source list */
    {
/* Free orignal source list */
        tfIgmpFreeSourceList(srcListPtr);
/* Move source list from backup, freeing backup source list */
        tfIgmpMoveSourceList(backupSrcListPtr,
                             srcListPtr);
    }
    else
    {
/* Otherwise just free the backup list */
        tfIgmpFreeSourceList(backupSrcListPtr);
    }
    return;
}

/*
 * tfIgmpBuildCurSrcListNReports Function Description
 * 1. Build an IGMP entry current source list based on sockets include/exclude
 *    lists.
 *   . If any socket is in exclude mode with no source,
 *     then we have to also be in exclude mode with no source.
 *   . else if exclude list non empty, we are also in exclude mode. Only keep
 *     the sources that are excluded by all entities which have joined, and
 *     that are not on the include list.
 *   . else we are in include mode, add all the souces from the include list.
 * 2. if multicast group is local, or we detected an IGMPv1 or IGMPv2 router
 *    or an error occured, reset the value pointed to by change pointer, as
 *    we won't bother building change report source lists. 
 * 3. else compare previous current source list, and new current source list
 *    to build the IGMPv3 state change report lists. If the previous current
 *    source list and new current source list are identical, do not
 *    bother to try and build the state change report lists, and reset the
 *    value pointed to by changePtr.
 * 4. If *changePtr is still non null, build the state change report source
 *    lists.
 * 4.1 Back up the current state change report source lists
 * 4.2 Compare the new current source list, with the previous current source
 *    list to build new state change report source lists according to RFC3376:
 *    Old State         New State         State-Change Record Sent
 *    ---------         ---------         ------------------------
 *
 *    INCLUDE (A)       INCLUDE (B)       ALLOW (B-A), BLOCK (A-B)
 *
 *    EXCLUDE (A)       EXCLUDE (B)       ALLOW (A-B), BLOCK (B-A)
 *
 *    INCLUDE (A)       EXCLUDE (B)       TO_EX (B)
 *
 *    EXCLUDE (A)       INCLUDE (B)       TO_IN (B)
 *  4.3 If the backed up change report source lists are being transmitted,
 *      Merge backed up change report source lists with the new change report
 *      source lists we just build using RFC3376 algorithm:
 *      . Don't bother merging with filter change reports as those need to be
 *        recomputed as we just did in 4.2.
 *      . If ALLOW source list in retransmission state not empty: Add the
 *        addresses in that source list to the new ALLOW report source list
 *        but ONLY IF if it is not in the new BLOCK report source list.
 *        (Avoid contradictory results.)
 *      . If BLOCK source list in retransmission state not empty: Add the
 *        addresses in that source list to the new BLOCK report source list
 *        but ONLY IF if it is not in the new ALLOW report source list.
 *        (Avoid contradictory results.)
 *  4.4 If we failed restore the change report source lists we backed up,
 *       otherwise free them.
 * 5. If we failed restore the current source list we backed up otherwise
 *    free it.
 * Parameters
 * Parameter            Description
 * igmpentryPtr         Pointer to IGMP entry
 * changePtr            Pointer to a value that we set to indicate whether
 *                      the new current source list is different from the
 *                      previous one.
 * Returns
 * Value                Meaning
 * TM_ENOERROR          Success
 * TM_ENOBUFS           Not enough memory to build the source lists.
 */
static int tfIgmpBuildCurSrcListNReports(
    ttIgmpEntryPtr igmpEntryPtr,
    tt8BitPtr      changePtr)
{
    ttSourceListPtr  curSrcListPtr;
    ttSourceListPtr  oldCurSrcListPtr;
    ttSourceListPtr  inclSrcListPtr;
    ttSourceListPtr  exclSrcListPtr;
    ttSourceListPtr  srcListPtr;
    ttSourceList     oldCurSrcList;
    ttSourceList     backupSrcList[TM_IGMP_MAX_STATE_CHANGE_REPORTS];
    int              maxIndex;
    int              index;
    int              backupList[TM_IGMP_MAX_STATE_CHANGE_REPORTS];
    int              addIndex;
    int              searchIndex;
    int              i;
    int              errorCode;

    errorCode = TM_ENOERROR;
    curSrcListPtr = &igmpEntryPtr->igmpSrcList[TM_IGMP_CURRENT_LIST];
    oldCurSrcListPtr = &oldCurSrcList;
/* backup the current source list, by moving it to its backup list */
    tfIgmpMoveSourceList(curSrcListPtr,
                         oldCurSrcListPtr);
/* Default mode: exclude mode with no source */
    curSrcListPtr->srcMode = TM_MODE_IS_EXCLUDE;
/* Build current source list */
    if (igmpEntryPtr->igmpCurNumEmptyExcl == TM_16BIT_ZERO)
/* No socket in exclude mode with no source */
    {
        inclSrcListPtr = &igmpEntryPtr->igmpSrcList[TM_MODE_IS_INCLUDE];
        exclSrcListPtr = &igmpEntryPtr->igmpSrcList[TM_MODE_IS_EXCLUDE];
        if (exclSrcListPtr->srcCount != TM_16BIT_ZERO)
        {
/*
 * Exclude list non empty. We are in exclude mode. Only keep the sources that
 * are excluded by all entities which have joined, and that are not on the
 * include list.
 */
            maxIndex = exclSrcListPtr->srcCount;
/* For each address in the exclude list */
            searchIndex = 0; /* Start of search (inclSrcListPtr) */
            addIndex = 0; /* Start of addition search (curSrcListPtr) */
            for (i = 0; i < maxIndex; i++)
            {
/*
 * Add it to the current source list, but only if the source is excluded by ALL the
 * sockets or entities who have joined,
 */
                if (    igmpEntryPtr->igmpOwnerCount
                     == exclSrcListPtr->srcOwnerCountPtr[i])
                {
/* and only if it is not in the include list? */
                    index = tfIgmpCacheGetSource(
                                         inclSrcListPtr,
                                         &searchIndex,
                                         exclSrcListPtr->srcAddrPtr[i].s_addr);
                    if (index == -1)
/* 
 * Source in the exclude list (joined by everybody), and not in the include
 * list
 */
                    {
                        errorCode = tfIgmpCacheAddSource(
                                             curSrcListPtr,
                                             &addIndex,
                                             &exclSrcListPtr->srcAddrPtr[i],
                                             1,
                                             TM_IGMP_IGMP_LIST, /* IGMP list */
                                             TM_8BIT_YES);      /* sorted */
                        if (errorCode != TM_ENOERROR)
/* Break out of the loop if we fail */
                        {
                            break;
                        }
                    }
                }
            }
        }
        else
/*
 * Exclude list is empty, and zero anysock, mode is include. Add all sources in
 * the include list to the report. Note that if include list is empty, we are
 * about to drop the group.
 */
        {
            curSrcListPtr->srcMode = TM_MODE_IS_INCLUDE;
            if (inclSrcListPtr->srcCount != TM_16BIT_ZERO)
            {
/* Include mode */
                addIndex = 0; /* Start of addition search (curSrcListPtr) */
                errorCode = tfIgmpCacheAddSource(
                                         curSrcListPtr,
                                         &addIndex,
                                         inclSrcListPtr->srcAddrPtr,
                                         inclSrcListPtr->srcCount,
                                         TM_IGMP_IGMP_LIST, /* IGMP list */
                                         TM_8BIT_YES);      /* sorted */
            }
        }
    }
/*
 * Build the reports, if we detected a change, and if multicast group is not
 * local, and if we did not detect an IGMPv1 or IGMPv2 router
 */
    if  (    (igmpEntryPtr->igmpState == TM_IGMP_LOCAL)
          || (tm_8bit_one_bit_set(igmpEntryPtr->igmpDevPtr->devIgmpVFlag,
                                  (TM_IGMP_DEV_V1 | TM_IGMP_DEV_V2)))
          || (errorCode != TM_ENOERROR) )
    {
/*
 * If multicast group is local, or we detected an IGMPv1 or IGMPv2 router, or
 * an error occured, reset the change flag, as we do not want to build new
 * reports.
 */
        *changePtr = TM_8BIT_NO;
    }
    else if (    (oldCurSrcListPtr->srcMode == curSrcListPtr->srcMode)
              && (oldCurSrcListPtr->srcCount == curSrcListPtr->srcCount))
/*
 * Otherwise check if we detected a change by comparing the current filter
 * source list to the previous one.
 */
    {
        for (i = 0; i < curSrcListPtr->srcCount; i++)
        {
            if (!tm_ip_match(oldCurSrcListPtr->srcAddrPtr[i].s_addr,
                             curSrcListPtr->srcAddrPtr[i].s_addr))
            {
                break;
            }
        }
        if (i == curSrcListPtr->srcCount)
        {
            *changePtr = TM_8BIT_NO;
        }
    }
    if (*changePtr != TM_8BIT_NO) 
    {
/*
 * Array used to store indices of backed up pending state reports.
 */
        tm_bzero(&backupList[0], sizeof(backupList));
/* 
 * Compare the back up list and new current list, and build the reports 
 * RFC3376:
 *    Old State         New State         State-Change Record Sent
 *    ---------         ---------         ------------------------
 *
 *    INCLUDE (A)       INCLUDE (B)       ALLOW (B-A), BLOCK (A-B)
 *
 *    EXCLUDE (A)       EXCLUDE (B)       ALLOW (A-B), BLOCK (B-A)
 *
 *    INCLUDE (A)       EXCLUDE (B)       TO_EX (B)
 *
 *    EXCLUDE (A)       INCLUDE (B)       TO_IN (B)
 */
        if (curSrcListPtr->srcMode !=  oldCurSrcListPtr->srcMode)
/* Build TO_IN(B) or TO_EX(B) */
        {
/* 
 * Backup pending state reports in case of failure. 
 * Since we are changing mode, we need to free all state change pending
 * reports, so move them while we do the backups.
 */
            for (i = 0; i < TM_IGMP_MAX_STATE_CHANGE_REPORTS; i++)
            {
                index = i + TM_CHANGE_TO_INCLUDE_MODE;
                tfIgmpMoveSourceList(&igmpEntryPtr->igmpSrcList[index],
                                     &backupSrcList[i]);
                if (index <= TM_CHANGE_TO_EXCLUDE_MODE)
                {
/* Make sure TO_IN() and TO_EX() are invalid */
                    igmpEntryPtr->igmpSrcList[index].srcMode =
                                                     TM_IGMP_INVALID_MODE;
                }
                backupList[i] = index;
            }
            if (curSrcListPtr->srcMode == TM_MODE_IS_INCLUDE)
            {
/* TO_IN(B) list */
                index = TM_CHANGE_TO_INCLUDE_MODE;
            }
            else
            {
/* TO_EX(B) list */
                index = TM_CHANGE_TO_EXCLUDE_MODE;
            }
            srcListPtr = &igmpEntryPtr->igmpSrcList[index];
            if (curSrcListPtr->srcCount != TM_16BIT_ZERO)
            {
                addIndex = 0; /* Start of addition search (srcListPtr) */
                errorCode = tfIgmpCacheAddSource(
                                        srcListPtr,
                                        &addIndex,
                                        curSrcListPtr->srcAddrPtr,
                                        curSrcListPtr->srcCount,
                                        TM_IGMP_IGMP_LIST, /* IGMP list */
                                        TM_8BIT_YES);      /* sorted */
            }
            if (errorCode == TM_ENOERROR)
/*
 * Success. Make sure to set the srcMode, so that it is no longer -1, and we
 * do send the report and make sure to cancel previous pending reports timer
 * so that we can start sending robustness packets.
 */
            {
/* Remove previous pending reports timer */
                tfIgmpSrcTimerRemove(igmpEntryPtr);
                srcListPtr->srcMode = (tt16Bit)index;
            }
        }
        else
/*
 * If mode is INCLUDE: Build ALLOW(B-A) and BLOCK(A-B)
 * or
 * If mode is EXCLUDE: Build ALLOW(A-B) and BLOCK(B-A)
 */
        {
/*
 * Move ALLOW() and BLOCK() reports to back up lists so that we can restore in
 * case of failure, and so that we can merge
 */
            for (i = TM_ALLOW_NEW_SOURCES - TM_CHANGE_TO_INCLUDE_MODE; 
                 i <= TM_BLOCK_OLD_SOURCES - TM_CHANGE_TO_INCLUDE_MODE; 
                 i++)
            {
                index = i + TM_CHANGE_TO_INCLUDE_MODE;
                tfIgmpMoveSourceList(&igmpEntryPtr->igmpSrcList[index],
                                     &backupSrcList[i]);
                backupList[i] = index;
            }
/* Build the difference ALLOW() and BLOCK() reports */
#define BminusAIndex index
#define AminusBIndex maxIndex 
            if (curSrcListPtr->srcMode == TM_MODE_IS_INCLUDE)
            {
/* ALLOW(B-A) */
                BminusAIndex = TM_ALLOW_NEW_SOURCES;
/* BLOCK(A-B) */
                AminusBIndex = TM_BLOCK_OLD_SOURCES;
            }
            else
            {
/* BLOCK(B-A) */
                BminusAIndex = TM_BLOCK_OLD_SOURCES;
/* ALLOW(A-B) */
                AminusBIndex = TM_ALLOW_NEW_SOURCES;
            }
/* Build B-A */
            errorCode = tfIgmpDiffSourceList(
                              &igmpEntryPtr->igmpSrcList[BminusAIndex],
                              curSrcListPtr,
                              oldCurSrcListPtr);
            if (errorCode == TM_ENOERROR)
            {
/* Build A-B */
                errorCode = tfIgmpDiffSourceList(
                             &igmpEntryPtr->igmpSrcList[AminusBIndex],
                             oldCurSrcListPtr,
                             curSrcListPtr);
            }
#undef BminusAIndex
#undef AminusBIndex
/* Merge with the pending in retransmission state reports. Make sure we do not
 * have contradictory results, i.e. we do not end up with the same address in
 * the ALLOW() and BLOCK() lists!!!
 */
            if (    (errorCode == TM_ENOERROR)
                 && (igmpEntryPtr->igmpSrcRobustness != (tt8Bit)0) )
/* In retransmission state */
            {
/* 
 * If allow list in retransmission state not empty: Add the addresses in that
 * list to the new allow report but ONLY IF if it is not in the new block list. 
 * (Avoid contradictory results.)
 * If block list in retransmission state not empty: Add the addresses in that
 * list to the new block report but ONLY IF if it is not in the new allow list.
 * (Avoid contradictory results.)
 */
/* If current index is allow(), subtraction is from new block() list */
                maxIndex = TM_BLOCK_OLD_SOURCES;
                for (i = TM_ALLOW_NEW_SOURCES - TM_CHANGE_TO_INCLUDE_MODE; 
                     i <= TM_BLOCK_OLD_SOURCES - TM_CHANGE_TO_INCLUDE_MODE; 
                     i++)
                {
                    if (backupSrcList[i].srcCount
                                         != TM_16BIT_ZERO)
                    {
                        index = i + TM_CHANGE_TO_INCLUDE_MODE;
                        errorCode = tfIgmpDiffSourceList(
                                        &igmpEntryPtr->igmpSrcList[index],
                                        &backupSrcList[i],
                                        &igmpEntryPtr->igmpSrcList[maxIndex]);
                        if (errorCode != TM_ENOERROR)
                        {
                            break;
                        }
                    }
/* 
 * Toggle subtraction list: 
 * If current index is block(), subtraction is from new allow() list
 */
                    maxIndex = TM_ALLOW_NEW_SOURCES;
                }
            }
        }
/* Restore or free TO_IN(), TO_EX(), ALLOW(), and BLOCK() backup lists */
        for (i = 0; i < TM_IGMP_MAX_STATE_CHANGE_REPORTS; i++)
        {
            index = backupList[i];
            if (index != 0)
            {
/*
 * If we failed, restore the source list we backed up, otherwise free the
 * backup
 */
                tfIgmpRestoreSourceList(&backupSrcList[i],
                                        &igmpEntryPtr->igmpSrcList[index],
                                        errorCode);
            }
        }
    }
/* Clean up current backup list: 
 * If we failed, restore the current IGMP source list we backed up, otherwise
 * free the backup
 */
    tfIgmpRestoreSourceList(oldCurSrcListPtr,
                            &igmpEntryPtr->igmpSrcList[TM_IGMP_CURRENT_LIST],
                            errorCode);
    return errorCode;
}

/*
 * tfIgmpCancelDevPendingReports Function Description
 * Called when we receive a v1 or v2 query.
 * For each IGMP entry in the device IGMP cache, cancel IGMPv3 state change
 * pending reports, and recorded list report; remove retransmission timers;
 * reset the number of transmissions.
 * Parameters
 * Parameter   meaning
 * devEntryPtr Pointer to interface that is being configured
 * Returns
 * none
 */
static void tfIgmpCancelDevPendingReports (ttDeviceEntryPtr devEntryPtr)
{
    ttIgmpEntryPtr  igmpEntryPtr;
    int             i;

    tfIgmpDevQueryTimerRemove(devEntryPtr);
    igmpEntryPtr = devEntryPtr->devIgmpPtr;
    while (igmpEntryPtr != (ttIgmpEntryPtr)0)
    {
/* Cancel all pending reports (i.e. 4 state changes lists + recorded list) */
        tfIgmpCancelPendingReports(igmpEntryPtr, 0,
                                   TM_IGMP_MAX_PENDING_RESPONSES);
/* Cancel retransmission timers */
        for (i = 0; i < TM_IGMP_MAX_TIMERS; i++)
        {
            tfIgmpTimerRemove(igmpEntryPtr, i);
        }
/* Reset the number of transmissions */
        igmpEntryPtr->igmpSrcRobustness = TM_16BIT_ZERO;
/* Move to next group on this device */                        
        igmpEntryPtr = igmpEntryPtr->igmpNextPtr;
    }
    return;
}

/*
 * tfIgmpCancelPendingReports Function Description
 * Cancel (numberReports - reportOffset) reports, i.e. free
 * from (reportOffset + TM_CHANGE_TO_INCLUDE_MODE) report
 * source list to (numReports + TM_CHANGE_TO_INCLUDE_MODE)
 * report source lsit.
 * 1. For each i in the range reportOffset, numReports - 1
 * 1.1. Convert index to a change report source list/recorded source list by
 *      adding TM_CHANGE_TO_INCLUDE_MODE
 * 1.2. Free the change report source list or recorded source list.
 * 1.3. If the source list is a filter mode change
 *      source list, make sure to mark it as invalid.
 *
 * Parameters
 * Parameter         Meaning
 * igmpEntryPtr      Pointer to IGMP entry
 * reportOffset      offset to first report to cancel
 * numberReports     number of reports 
 * Returns
 * NONE
 */
static void tfIgmpCancelPendingReports(
    ttIgmpEntryPtr  igmpEntryPtr,
    int             reportOffset,
    int             numReports)
{
    int            i;
    int            index;

    for (i = 0; i < numReports; i++)
/* For each i in the range */
    {
/* Convert to a change start report source list index */
        index = i + reportOffset + TM_CHANGE_TO_INCLUDE_MODE;
/* Free the change report source list */
        tfIgmpFreeSourceList(&igmpEntryPtr->igmpSrcList[index]);
        if (index <= TM_CHANGE_TO_EXCLUDE_MODE)
        {
/* Make sure TO_IN() and TO_EX() are invalid */
            igmpEntryPtr->igmpSrcList[index].srcMode =
                                                 TM_IGMP_INVALID_MODE;
        }
    }
}

/* 
 * tfIgmpDiffSourceList Function Description
 * Build a new source list, using all sources from the A source list, that are
 * not included in the B source list. (A-B)
 * For efficienty, consider the special case where B is empty (A-0). In that
 * case we can just add all sources from the A list into the new list.
 * Parameters
 * Parameter        Description
 * newSrcListPtr    Pointer to new source list to store the diff into
 * aSrcListPtr      Pointer to the A source list
 * bSrcListPtr      Pointer to the B source list
 * Returns
 * Value            Meaning
 * TM_ENOERROR      No error
 * TM_ENOBUFS       No memory to store new sources
 */
static int tfIgmpDiffSourceList(
    ttSourceListPtr  newSrcListPtr,
    ttSourceListPtr  aSrcListPtr,
    ttSourceListPtr  bSrcListPtr)
{
    int i;
    int index;
    int searchIndex;
    int addIndex;
    int maxIndex;
    int errorCode;

    errorCode = TM_ENOERROR;
    if (aSrcListPtr->srcCount != TM_16BIT_ZERO)
    {
/* aIdx list non empty. */
        addIndex = 0; /* start index of addition (newSrcListPtr) */
        if (bSrcListPtr->srcCount == TM_16BIT_ZERO)
/* bIdx list is empty (A-0). Just add */
        {
            errorCode = tfIgmpCacheAddSource(newSrcListPtr,
                                             &addIndex,
                                             &aSrcListPtr->srcAddrPtr[0],
                                             aSrcListPtr->srcCount,
                                             TM_IGMP_IGMP_LIST, /* IGMP list */
                                             TM_8BIT_YES);      /* sorted */
        }
        else
        {
/* bIdx list non empty. */
            maxIndex = (int)aSrcListPtr->srcCount;
            searchIndex = 0; /* start index of search (bSrcListPtr) */
/* For each address in the aIdx source list */
            for (i = 0; i < maxIndex; i++)
            {
/*
 * Add it to the newIdx source list, but only if it is not in the bIdx
 * source list
 */
                index = tfIgmpCacheGetSource(
                                           bSrcListPtr,
                                           &searchIndex,
                                           aSrcListPtr->srcAddrPtr[i].s_addr);
                if (index == -1)
/* Source in the aIdx list, and not in the bIdx list, add it to the new list */
                {
                    errorCode = tfIgmpCacheAddSource(newSrcListPtr,
                                             &addIndex,
                                             &aSrcListPtr->srcAddrPtr[i],
                                             1,
                                             TM_IGMP_IGMP_LIST, /* IGMP list */
                                             TM_8BIT_YES);      /* sorted */
                    if (errorCode != TM_ENOERROR)
/* Break out of the loop if we fail */
                    {
                        break;
                    }
                }
            }
        }
    }
    return errorCode;
}

/*
 * tfIgmpMoveSourceList Function Description
 * Copy and move one source list. If TM_IGMP_REALLOCATED_SRC is defined, make
 * sure to point to local arrays after the copy if needed. New source list
 * points to allocated memory, and old source list no longer point to it.
 * Parameters
 * Parameter        Description
 * oldSrcListPtr    Pointer to old source list
 * newSrcListPtr    Pointer to new source list to move old sources into
 * Returns
 * NONE
 */ 
void tfIgmpMoveSourceList (
    ttSourceListPtr oldSrcListPtr,
    ttSourceListPtr newSrcListPtr)
{
    tm_bcopy(oldSrcListPtr, newSrcListPtr, sizeof(ttSourceList));
    if (oldSrcListPtr->srcAddrPtr != (struct in_addr TM_FAR *)0)
    {
#ifdef TM_IGMP_PREALLOCATED_SRC
        if (oldSrcListPtr->srcAddrPtr == &oldSrcListPtr->srcAddrArr[0])
        {
            newSrcListPtr->srcAddrPtr = &newSrcListPtr->srcAddrArr[0];
            newSrcListPtr->srcOwnerCountPtr =
                                &newSrcListPtr->srcOwnerCountArr[0];
        }
        else
#endif /* TM_IGMP_PREALLOCATED_SRC */
        {
/*
 * Make sure that the old list does not point to same memory.
 */
            oldSrcListPtr->srcAddrPtr = (struct in_addr TM_FAR *)0;
            oldSrcListPtr->srcOwnerCountPtr = (tt16BitPtr)0;
            oldSrcListPtr->srcAllocCount = TM_16BIT_ZERO;
        }
        oldSrcListPtr->srcCount = TM_16BIT_ZERO;
    }
    tfIgmpFreeSourceList(oldSrcListPtr);
    return;
}

/*
 * tfIgmpCopySourceList Function Description
 * Duplicate one source list. If TM_IGMP_PREALLOCATED_SRC is defined make sure
 * to point to local arrays after the copy if sources were in local array,
 * otherwise make sure to allocate new buffer, and copy sources and owner
 * counts.
 * Parameters
 * Parameter        Description
 * oldSrcListPtr    Pointer to old source list
 * newSrcListPtr    Pointer to new source list to copy old sources into
 * Returns
 * Value            Meaning
 * TM_ENOERROR      No error
 * TM_ENOBUFS       No memory to allocate new sources
 */ 
static int tfIgmpCopySourceList (
    ttSourceListPtr oldSrcListPtr,
    ttSourceListPtr newSrcListPtr)
{
    int     errorCode;

    errorCode = TM_ENOERROR;
    tm_bcopy(oldSrcListPtr, newSrcListPtr, sizeof(ttSourceList));
#ifdef TM_IGMP_PREALLOCATED_SRC
    if (oldSrcListPtr->srcAddrPtr == &oldSrcListPtr->srcAddrArr[0])
    {
        newSrcListPtr->srcAddrPtr = &newSrcListPtr->srcAddrArr[0];
        newSrcListPtr->srcOwnerCountPtr =
                                &newSrcListPtr->srcOwnerCountArr[0];
    }
    else
#endif /* TM_IGMP_PREALLOCATED_SRC */
    {
        if (oldSrcListPtr->srcAddrPtr != (struct in_addr TM_FAR *)0)
        {
/* Source list was allocated in old source list */
            errorCode = tfIgmpAllocateCopySources(newSrcListPtr,
                                                  oldSrcListPtr->srcAllocCount,
                                                  TM_8BIT_NO);
        }
    }
    return errorCode;
}

#endif /* TM_USE_IGMPV3 */

/*
 * tfIgmpSocketCachePurge Function Description
 * Leave all multicast groups joined on this socket. Free up cache area.
 * Details:
 * If there are no pending IGMP user api calls:
 * IGMPV3:
 * . Detach the socket mcast from the socket for re-entrancy.
 * . Unlock socket
 * . Walk the detached socket mcast cache:
 *   For each multicast joined on this socket, leave the group on the
 *   corresponding device locking/unlocking the device, and free up socket
 *   mcast cache entry, and its source list.
 * . Re-lock the socket.
 * Not IGMPV3:
 * . Detach the socket mcast from the socket for re-entrancy.
 * . Unlock socket
 * . For each multicast joined on this socket, leave the group on the
 *   corresponding device with that device locked.
 *   Note that the device is locked/unlocked when we leave the group.
 * . Re-lock the socket.
 * . Free the socket mcast cache.
 * Parameters:
 * Parameter            Description
 * socketEntryPtr       Pointer to socket entry
 * Returns
 * NONE
 */
void tfIgmpSocketCachePurge (ttSocketEntryPtr socketEntryPtr)
{
#ifdef TM_USE_IGMPV3
    ttList                      sockIgmpInfoList;
    ttGenericUnion              genParm1;
#else /* !TM_USE_IGMPV3 */
    ttDeviceEntryPtr            devEntryPtr;
    ttSockIgmpInfoPtr           sockIgmpInfoPtr;
    int                         index;
#endif /* !TM_USE_IGMPV3 */
    ttIpMreqSrc                 imrIntStruct;
    ttIpMreqSrcPtr              imrIntPtr;
    int                         mcastCount;

    if (socketEntryPtr->socIgmpJoinsPending == 0)
    {
        imrIntPtr = &imrIntStruct;
        tm_bzero(imrIntPtr, sizeof(ttIpMreqSrc));
        imrIntPtr->imrOptionName = IPO_DROP_MEMBERSHIP;
#ifdef TM_USE_IGMPV3
        imrIntPtr->imrFmode = TM_MODE_IS_INCLUDE;
        mcastCount = socketEntryPtr->socIgmpInfoList.listCount;
        if (mcastCount != 0)
        {
/* Move list to temporary one */
            tfListReplace(&(socketEntryPtr->socIgmpInfoList.listHeadNode),
                          &(sockIgmpInfoList.listHeadNode));
            sockIgmpInfoList.listCount = mcastCount;
/* Empty list */
            tfListInit(&(socketEntryPtr->socIgmpInfoList));
/* Unlock the socket */
            tm_call_unlock(&socketEntryPtr->socLockEntry);
            genParm1.genVoidParmPtr = (ttVoidPtr)imrIntPtr;
/* Walk the list to purge it */
            (void)tfListWalk(&sockIgmpInfoList, tfIgmpSocketCacheEntryPurge,
                             genParm1);
/* Relock the socket */
            tm_call_lock_wait(&socketEntryPtr->socLockEntry);
        }
#else /* !TM_USE_IGMPV3 */
        sockIgmpInfoPtr = socketEntryPtr->socIgmpInfoArray;
        socketEntryPtr->socIgmpInfoArray = TM_SOC_IGMP_INFO_NULL_PTR;
        mcastCount = socketEntryPtr->socIgmpInfoInUseCount;
        socketEntryPtr->socIgmpInfoInUseCount = 0;
        if (sockIgmpInfoPtr != TM_SOC_IGMP_INFO_NULL_PTR)
        {
            if (mcastCount != 0)
            {
/* Unlock socket entry because we are locking the device */
                tm_call_unlock(&socketEntryPtr->socLockEntry);
/* leave the multicast groups that we joined before */
/* NOTE: the following code leaves multicast groups in reverse order to avoid
   the bcopy in tfIgmpSocketCacheRemove. This enables us to directly reference
   the cached information on the socket. However, this method will fail if
   tfIgmpLeaveHostGroup fails. */
                for (index = (mcastCount-1); index >= 0; index--)
                {
                    devEntryPtr = sockIgmpInfoPtr[index].siiDevicePtr;
                    imrIntPtr->imrDevPtr = devEntryPtr;
                    tm_ip_copy(sockIgmpInfoPtr[index].siiMcastAddr,
                               imrIntPtr->imrMcastAddr);
                    tm_call_lock_wait(&(devEntryPtr->devLockEntry));
/* leave each multicast group we are still joined to */
                    (void) tfIgmpLeaveHostGroup(
                                TM_8BIT_YES,
                                imrIntPtr);
                    tm_call_unlock(&(devEntryPtr->devLockEntry));
                }
/* Relock the socket, before we return */
                tm_call_lock_wait(&socketEntryPtr->socLockEntry);
            }
/* free any memory we allocated for caching multicast groups joined on
   the socket */
            tm_free_raw_buffer((ttRawBufferPtr)(ttVoidPtr)sockIgmpInfoPtr);
        }
#endif /* !TM_USE_IGMPV3 */
    }
    return;
}

#ifdef TM_USE_STOP_TRECK
void tfIgmpDeInit (ttDeviceEntryPtr devPtr)
{
    devPtr->devIgmpVFlag = TM_IGMP_DEV_V1;
#ifdef TM_USE_IGMPV3
    devPtr->devIgmpRobustness = 0;
    tfIgmpCancelDevPendingReports(devPtr);
#endif /* TM_USE_IGMPV3 */
}
#endif /* TM_USE_STOP_TRECK */

#else /* ! TM_IGMP || ! TM_USE_IPV4 */

LINT_UNUSED_HEADER
/* To allow link for builds when TM_IGMP is not defined */
int tv4IgmpDummy = 0;

#endif /* ! TM_IGMP || ! TM_USE_IPV4 */
