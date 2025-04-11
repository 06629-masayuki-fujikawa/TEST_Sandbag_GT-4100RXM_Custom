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
 * Description: Macro definitions for IPv6
 *
 * Filename: t6macro.h
 * Author: Ed, Jason, & Odile
 * Date Created: 3/26/02
 * $Source: include/t6macro.h $
 *
 * Modification History
 * $Revision: 6.0.2.17 $
 * $Date: 2012/03/08 04:01:57JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _T6MACRO_H_
#define _T6MACRO_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TM_USE_IPV6

#ifdef TM_6_USE_MLDV2
#ifndef TM_6_USE_RFC3590
#define TM_6_USE_RFC3590
#endif /* TM_6_USE_RFC3590 */
#endif /* TM_6_USE_MLDV2 */

#ifdef TM_6_USE_MIP_MN
#ifndef TM_6_USE_MIP
#define TM_6_USE_MIP
#endif /* TM_6_USE_MIP */
#ifdef TM_SINGLE_INTERFACE
/* TM_SINGLE_INTERFACE will not work for Mobile IPv6 mobile node because
   the mobile node has a virtual home interface with a configured link-local
   scope address - this will cause Router Discovery to fail on the mobile
   interface if we allow it to stay enabled, so we undefine it here. */
#undef TM_SINGLE_INTERFACE
#endif /* TM_SINGLE_INTERFACE */
#endif /* TM_6_USE_MIP_MN */

#ifndef TM_6_USE_MIP_MN
/* undefine Mobile IPv6 mobile node feature macros, if mobile node isn't used*/
#ifdef TM_6_MN_DISABLE_HOME_DETECT
#undef TM_6_MN_DISABLE_HOME_DETECT
#endif /* TM_6_MN_DISABLE_HOME_DETECT */

#ifdef TM_6_USE_MIP_RA_RTR_ADDR
#undef TM_6_USE_MIP_RA_RTR_ADDR
#endif /* TM_6_USE_MIP_RA_RTR_ADDR */
#endif /* ! TM_6_USE_MIP_MN */

#if ( !defined(TM_6_USE_MIP_MN) && !defined(TM_6_USE_MIP_CN) )
#ifdef TM_6_USE_MIP_RO
/* undefine Mobile IPv6 feature macros, if we aren't using Mobile IPv6 */
#undef TM_6_USE_MIP_RO
#endif /* TM_6_USE_MIP_RO */
#endif /* Mobile IPv6 not enabled */

#ifdef TM_6_USE_MIP_CN
#ifndef TM_6_USE_MIP
#define TM_6_USE_MIP
#endif /* TM_6_USE_MIP */
#define TM_6_USE_ROUTING_HEADER

#ifndef TM_6_USE_MIP_RO
/* MIPv6 correspondent node always uses route optimization */
#define TM_6_USE_MIP_RO
#endif /* ! TM_6_USE_MIP_RO */
#endif /* TM_6_USE_MIP_CN */

/* To avoid having IPv6 auto-configured/discovered routing entries assigned
   a multi-home index that conflicts with IPv4 usage, we use a different
   range in the routing tree for IPv6 multi-home index when the stack is run
   as dual IP layer. This works because IPv6 never uses this multi-home index
   to select the source address of packets to send.
   NOTE: manually configured IPv6 addresses have multi-home indexes in the
   range of 0..(TM_MAX_IPS_PER_IF - 1), and auto-configured IPv6 addresses
   have multi-home indexes in the range
   TM_MAX_IPS_PER_IF..((2 * TM_MAX_IPS_PER_IF) - 1) */
#define TM_6_RT_MHOME_INDEX_OFFSET 32000
#if (TM_MAX_IPS_PER_IF > TM_6_RT_MHOME_INDEX_OFFSET)
#error "TM_MAX_IPS_PER_IF cannot be greater than 32000!"
#endif /* TM_MAX_IPS_PER_IF check */

#define TM_6_RT_START_PREFIX_MHOME_INDEX        64000
#define TM_6_RT_RESERVED_MHOME_INDEX            65535U

#ifdef TM_6_USE_NUD
/* IPv6 NUD */
#ifndef TM_ARP_UPDATE_ON_RECV 
#define TM_ARP_UPDATE_ON_RECV
#endif /* ! TM_ARP_UPDATE_ON_RECV */
#endif /* TM_6_USE_NUD */

/* IPV6 address length */
#define TM_6_IP_ADDRESS_LENGTH     16
#define TM_6PAK_IP_ADDRESS_LENGTH  tm_packed_byte_count(TM_6_IP_ADDRESS_LENGTH)

/* Host prefix length == Number of bits in IP address */
#define TM_6_IP_HOST_PREFIX_LENGTH      128

/* IPv4-compatible IPv6 prefix length */
#define TM_6_IP_V4COMPAT_PREFIX_LENGTH  96

/* Minimum value for IPv6 path MTU. ([RFC2460].R5:10) */
#define TM_6_PMTU_MINIMUM               (tt16Bit) 1280
/* IPv6 Fragment Extension Header length */
#define TM_6_IP_FRAG_EXT_HDR_LEN        8
#define TM_6PAK_IP_FRAG_EXT_HDR_LEN \
 tm_packed_byte_count(TM_6_IP_FRAG_EXT_HDR_LEN)
/* "More fragment" flag for Fragment header */
#define TM_6_EXTH_MORE_FRAGS_FLAG       0x0001

#ifndef TM_6_DISABLE_PMTU_DISC
/*
 * User can remove all IPv6 Path MTU discovery code by defining
 * TM_6_DISABLE_PMTU_DISC in trsystem.h.
 */
#define TM_6_PMTU_DISC
#endif /* TM_6_DISABLE_PMTU_DISC */
    
#ifndef TM_6_DEF_TUNNEL_MTU
/* Default IPv6-over-IPv4 tunnel MTU. */
#ifdef TM_6_PMTU_DISC
#define TM_6_DEF_TUNNEL_MTU 65535
#else /* !TM_6_PMTU_DISC */
#define TM_6_DEF_TUNNEL_MTU 1280
#endif /* TM_6_PMTU_DISC */
#endif /* TM_DEF_TUNNEL_MTU */

/*
 *   Decimal   Hexadecimal     Description
 *   -------   -----------     -----------
 *    127         7F           Reserved
 *    126         7E           Mobile IPv6 Home-Agents anycast [4]
 *    0-125       00-7D        Reserved
 */
#define TM_6_ANYCAST_ID_HOME_AGENT 0x7E

#ifdef TM_IN6ADDR_NO_NESTED_BRACES
/* IPV6 HOST MASK */
#define TM_IN6ADDR_HOST_MASK                                              \
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,                 \
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }
#else /* !TM_IN6ADDR_NO_NESTED_BRACES */
#define TM_IN6ADDR_HOST_MASK                                              \
        {{{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,               \
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }}}
#endif /* TM_IN6ADDR_NO_NESTED_BRACES */


#ifdef TM_IN6ADDR_NO_NESTED_BRACES
/* IPv4-mapped IPv6 default gateway address */
#define TM_IN6ADDR_IPV4_ANY_INIT                                          \
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                 \
            0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00 }
#else /* !TM_IN6ADDR_NO_NESTED_BRACES */
#define TM_IN6ADDR_IPV4_ANY_INIT                                          \
        {{{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,               \
            0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00 }}}
#endif /* TM_IN6ADDR_NO_NESTED_BRACES */

#ifndef TM_6_ROUTER_LIST_MAX_ENTRIES
/* The maximum number of entries that can be stored in the IPv6 Default Router
   List (must be at least 2).
   ([RFC2461].R6.3.4:30, [RFC2461].R6.3.4:40) */
#define TM_6_ROUTER_LIST_MAX_ENTRIES    3
#endif /* ! TM_6_ROUTER_LIST_MAX_ENTRIES */
#define TM_6_USE_TEMP_GWY
/* Default router lists */
#define TM_6_DEF_ROUTER_LIST  0
#ifdef TM_6_USE_TEMP_GWY
/* Temporary default router list (added statically by the user as temporary) */
#define TM_6_DEF_TEMP_ROUTER_LIST (TM_6_DEF_ROUTER_LIST + 1)
#else /* !TM_6_USE_TEMP_GWY */
#define TM_6_DEF_TEMP_ROUTER_LIST (TM_6_DEF_ROUTER_LIST)
#endif /* !TM_6_USE_TEMP_GWY */
/* Number of active router lists (non deleted/non expired) */
#define TM_6_ACTIVE_DEF_ROUTER_LISTS (TM_6_DEF_TEMP_ROUTER_LIST + 1)
/* List of expired routers to be deleted and freed */
#define TM_6_DEF_EXP_ROUTER_LIST  (TM_6_DEF_TEMP_ROUTER_LIST + 1)
/* Number of default router lists */
#define TM_6_NUM_DEF_ROUTER_LISTS (TM_6_DEF_EXP_ROUTER_LIST + 1) 

#ifndef TM_6_MAX_AUTOCONF_IPS_PER_IF    
/* Maximum number of IPv6 address that can be stateless auto-configured on
   an interface.
   ([RFC2462].R5.2:60, [RFC2373].R2.1:10, [RFC2462].R5.5.3:90) */
#ifdef TM_6_USE_PREFIX_DISCOVERY
#define TM_6_MAX_AUTOCONF_IPS_PER_IF    4
#else /* ! TM_6_USE_PREFIX_DISCOVERY */
#define TM_6_MAX_AUTOCONF_IPS_PER_IF    1
#endif /* ! TM_6_USE_PREFIX_DISCOVERY */
#endif /* ! TM_6_MAX_AUTOCONF_IPS_PER_IF */

#ifndef TM_6_RDNSS_MAX_ENTRY
#define TM_6_RDNSS_MAX_ENTRY            3
#endif /* TM_6_RDNSS_MAX_ENTRY */

#ifndef TM_6_DNSSL_MAX_ENTRY
#define TM_6_DNSSL_MAX_ENTRY            3
#endif /* TM_6_DNSSL_MAX_ENTRY */

#ifndef TM_6_DNSSL_MAX_DOMAIN_LEN
#define TM_6_DNSSL_MAX_DOMAIN_LEN       255
#endif /* TM_6_DNSSL_MAX_DOMAIN_LEN */

#ifdef TM_6_USE_DHCP
#define TM_6_DHCP_DUID_TYPE_NONE        0   /* Indicating DUID not set yet  */
#define TM_6_DHCP_DUID_TYPE_EN          2   /* Enterprise number/ID based   */
#define TM_6_DHCP_DUID_TYPE_LL          3   /* Link-layer address           */

#ifndef TM_6_MAX_DHCP_IPS_PER_IA
#define TM_6_MAX_DHCP_IPS_PER_IA        4   /* Max addressse on a IA        */
#endif /* TM_6_MAX_DHCP_IPS_PER_IA */

#ifndef TM_6_MAX_DHCP_SERVER_PER_IF
#define TM_6_MAX_DHCP_SERVER_PER_IF     4   /* Max servers saved for a IF   */
#endif /* TM_6_MAX_DHCP_SERVER_PER_IF */

#ifndef TM_6_DHCP_MAX_DNS_SERVERS
#define TM_6_DHCP_MAX_DNS_SERVERS       2   /* Max DNS servers in btEntry   */
#endif /* TM_6_DHCP_MAX_DNS_SERVERS */

#ifndef TM_6_DHCP_MAX_DOMAIN_LEN
#define TM_6_DHCP_MAX_DOMAIN_LEN        128
#endif /* TM_6_DHCP_MAX_DOMAIN_LEN */

#ifdef TM_6_DHCP_USE_FQDN
/* Default value for the FQDN flags field */
#define TM_6_DHCP_FQDN_FLAGS_DEFAULT 0x01
#endif /* TM_6_DHCP_USE_FQDN */

#else
#define TM_6_MAX_DHCP_IPS_PER_IA        0
#define TM_6_MAX_DHCP_SERVER_PER_IF     0
#endif /* TM_6_USE_DHCP */

/* Flags for storing router advertisement flags that pertain to DHCPv6 */
#define TM_6_DHCP_FLAG_SET_M            ((tt8Bit)0x01)
#define TM_6_DHCP_FLAG_SET_O            ((tt8Bit)0x02)

#ifdef TM_6_USE_MIP_HA
/*
 * Proxy addresses configure only if we are a HA, we receive on these address
 * but never use these addresses as source addresses
 * these are actually the addresses of the mobile nodes that we are tunneling 
 * packets to and defending the addresses for  
 */
#ifndef TM_6_USE_MIP
#define TM_6_USE_MIP
#endif /* TM_6_USE_MIP */
#define TM_6_MAX_MIP_PROXY_IPS_PER_IF   4
#else /*  TM_6_USE_MIP_HA */
#define TM_6_MAX_MIP_PROXY_IPS_PER_IF   0
#endif /*  TM_6_USE_MIP_HA */

/* Max mhome address of locally configured addresses */
#define TM_6_MAX_LOCAL_MHOME         \
     (TM_MAX_IPS_PER_IF             \
    + TM_6_MAX_AUTOCONF_IPS_PER_IF  \
    + TM_6_MAX_DHCP_IPS_PER_IA)

/* Max mhome address of locally configured addresses as well as proxy addr */
#define TM_6_MAX_MHOME (TM_6_MAX_LOCAL_MHOME + TM_6_MAX_MIP_PROXY_IPS_PER_IF)

/* Hop-by-Hop Options and Destination Options */
#define TM_6_HDR_OPT_PAD1               (tt8Bit)0
#define TM_6_HDR_OPT_PADN               (tt8Bit)1
#define TM_6_HDR_OPT_ROUTER_ALERT       (tt8Bit)5
#define TM_6_ROUTER_ALERT_VALUE_MLD     (tt16Bit)0
#define TM_6_ROUTER_ALERT_MIN_LEN       8

#define TM_6PAK_ROUTER_ALERT_MIN_LEN \
  tm_packed_byte_count(TM_6_ROUTER_ALERT_MIN_LEN)
/* IPv6 Destination Option type for Home Address option. */
#define TM_6_HDR_OPT_HOME_ADDR          (tt8Bit) 201

/* Mask used to determine the action that must be taken when processing a
   received extension header option that we don't recognize. Refer to
   tt6ExtHdrOption.eho6OptionType. */
#define TM_6_OPTION_ACTION_MASK         (tt8Bit) 0xC0
/* Extension header option action indicates skip over the option and continue
   processing the header. */
#define TM_6_OPTION_ACTION_SKIP         (tt8Bit) 0x00
/* Extension header option action indicates discard the packet. */
#define TM_6_OPTION_ACTION_DISCARD      (tt8Bit) 0x40
/* Extension header option action indicates discard the packet and send an ICMP
   Parameter Problem message (unrecognized Option Type) to the source. */
#define TM_6_OPTION_ACTION_FORCEICMP    (tt8Bit) 0x80
/* Extension header option action indicates discard the packet and send an ICMP
   Parameter Problem message (unrecognized Option Type) to the source, but only
   if the destination was not multicast. */
#define TM_6_OPTION_ACTION_ICMP         (tt8Bit) 0xC0


/* ICMPv6 stuff */
/* type: 1 byte, code: 1 byte, checksum: 2 byte */
#define TM_6_ICMP_HEADER_LEN 4

/* The minimum number of bytes in a received ICMPv6 message. */
#define TM_6_MIN_ICMP_MSG_LEN           8

#define TM_6PAK_MIN_ICMP_MSG_LEN \
  tm_packed_byte_count(TM_6_MIN_ICMP_MSG_LEN)

/* ICMPv6 Destination Unreachable error message type */
#define TM_6_ICMP_TYPE_DEST_UNREACH     (tt8Bit) 1

/* Destination Unreachable: no route to destination */
#define TM_6_ICMP_CODE_NO_ROUTE         (tt8Bit) 0

/* Destination Unreachable: administratively prohibited (firewall filter) */
#define TM_6_ICMP_CODE_ADMIN            (tt8Bit) 1

/* Destination Unreachable: beyond scope of source address */
#define TM_6_ICMP_CODE_SCOPE            (tt8Bit) 2

/* Destination Unreachable: address unreachable */
#define TM_6_ICMP_CODE_ADDRESS          (tt8Bit) 3

/* Destination Unreachable: port unreachable */
#define TM_6_ICMP_CODE_NO_PORT          (tt8Bit) 4

/* Destination Unreachable: Source address failed ingress/egress policy */
/* (this code is an informative subset of code 1) */
#define TM_6_ICMP_CODE_ADMIN_SOURCE     (tt8Bit) 5

/* Destination Unreachable: Reject route to destination */
/* (this code is an informative subset of code 1) */
#define TM_6_ICMP_CODE_ADMIN_DEST       (tt8Bit) 6

/* ICMPv6 Packet Too Big error message type */
#define TM_6_ICMP_TYPE_PKT_TOO_BIG      (tt8Bit) 2

/* ICMPv6 Time Exceeded error message type */
#define TM_6_ICMP_TYPE_TIME_EXCEEDED    (tt8Bit) 3

/* Time Exceeded: hop limit exceeded in transit */
#define TM_6_ICMP_CODE_TRANSIT          (tt8Bit) 0

/* Time Exceeded: fragment reassembly time exceeded */
#define TM_6_ICMP_CODE_REASSEMBLY       (tt8Bit) 1

/* ICMPv6 Parameter Problem error message type */
#define TM_6_ICMP_TYPE_PARM_PROBLEM     (tt8Bit) 4

/* Parameter Problem: erroneous header field */
#define TM_6_ICMP_CODE_HEADER           (tt8Bit) 0

/* Parameter Problem: unrecognized next header */
#define TM_6_ICMP_CODE_NEXT_HEADER      (tt8Bit) 1

/* Parameter Problem: unrecognized option */
#define TM_6_ICMP_CODE_OPTION           (tt8Bit) 2

/* ICMPv6 Echo Request message type */
#define TM_6_ICMP_TYPE_ECHO_REQUEST     (tt8Bit) 128

/* ICMPv6 Echo Reply message type */
#define TM_6_ICMP_TYPE_ECHO_REPLY       (tt8Bit) 129

/* The minimum number of bytes in a received MLD message. */
#define TM_6_MIN_MLD_MSG_LEN            24

/* ICMPv6 MLD Query message type */
#define TM_6_MLD_TYPE_QUERY             (tt8Bit) 130

/* ICMPv6 MLD Report message type */
#define TM_6_MLD_TYPE_REPORT            (tt8Bit) 131

/* ICMPv6 MLD Done message type */
#define TM_6_MLD_TYPE_DONE              (tt8Bit) 132

/* The minimum number of bytes in a received MLDv2 message*/
#define TM_6_MIN_MLDV2_MSG_LEN          28

/* Query type is MLDv1. */
#define TM_6_MLDV1_QUERY                (tt8Bit) 0

#ifdef TM_6_USE_MLDV2
/* MLDV2 Report message type */
#define TM_6_MLDV2_TYPE_REPORT          (tt8Bit) 143

/* The maximum number of bytes in a received MLDv2 message*/
#define TM_6_MAX_MLDV2_MSG_LEN          1452

/* Maximum number of Source Addresses */
#define TM_6_MAX_SOURCE_ADDR            (tt8Bit) 89

/* Unsolicited Report Interval in seconds */
#define   TM_6_MLDV2_UNSOL_INTVL        1

/* Default value of Robustness Variable */
#define TM_6_MLD_ROBUST_VAR             2

/* Default value of Query Interval in seconds */
#define TM_6_MLD_QUERY_INTR             125

/* Default value of Query Response Interval in seconds */
#define TM_6_MLD_QUERY_RESP_INTR        10000

/* MLDV2 Report message type */
#define TM_6_MODE_IS_INCLUDE                   (tt8Bit) 1
#define TM_6_MODE_IS_EXCLUDE                   (tt8Bit) 2
#define TM_6_CHANGE_TO_INCLUDE_MODE            (tt8Bit) 3
#define TM_6_CHANGE_TO_EXCLUDE_MODE            (tt8Bit) 4
#define TM_6_ALLOW_NEW_SOURCES                 (tt8Bit) 5
#define TM_6_BLOCK_OLD_SOURCES                 (tt8Bit) 6

/* MLDV2 Query types */
#define TM_6_GENERAL_QUERY                     (tt8Bit) 1
#define TM_6_MC_ADDR_SPECIFIC_QUERY            (tt8Bit) 2
#define TM_6_MC_ADDR_AND_SRC_SPECIFIC_QUERY    (tt8Bit) 3
#define TM_6_STATE_CHANGE_QUERY                (tt8Bit) 4
#endif /* TM_6_USE_MLDV2 */

/* Minimum size of a router solicit. */
#define TM_6_MIN_ROUT_SOLICIT_LEN       8

#define TM_6PAK_MIN_ROUT_SOLICIT_LEN \
  tm_packed_byte_count(TM_6_MIN_ROUT_SOLICIT_LEN)

/* ICMPv6 Router Solicitation message type */
#define TM_6_ND_TYPE_ROUT_SOLICIT       (tt8Bit) 133

/* Minimum size of a router advertisement. */
#define TM_6_MIN_ROUT_ADVERT_LEN        16

#define TM_6PAK_MIN_ROUT_ADVERT_LEN \
  tm_packed_byte_count(TM_6_MIN_ROUT_ADVERT_LEN)

/* ICMPv6 Router Advertisement message type */
#define TM_6_ND_TYPE_ROUT_ADVERT        (tt8Bit) 134

/* Mask used to test if the Router Advertisement Managed address configuration
   flag is set. */
#define TM_6_ND_RA_MANAGED_FLAG         (tt8Bit) 0x80

/* Mask used to test if the Router Advertisement Other stateful configuration
   flag is set. */
#define TM_6_ND_RA_OTHER_FLAG           (tt8Bit) 0x40

/* Minimum size of a neighbor solicitation or advertisement */
#define TM_6_ND_MIN_NEIGH_LEN           24

#define TM_6PAK_ND_MIN_NEIGH_LEN \
  tm_packed_byte_count(TM_6_ND_MIN_NEIGH_LEN)

/* Length of Neighbor Solicitation/Advertisement with Source Link-Layer Address
 * option/Target Link-Layer Address option */
#define TM_6_ND_NEIGH_LEN               (TM_MAX_PHYS_ADDR + 26)

/* ICMPv6 Neighbor Solicitation message type */
#define TM_6_ND_TYPE_NEIGH_SOLICIT      (tt8Bit) 135

/* ICMPv6 Neighbor Advertisement message type */
#define TM_6_ND_TYPE_NEIGH_ADVERT       (tt8Bit) 136

/* Mask used to test if the Neighbor Advertisement Router flag is set. */
#define TM_6_ND_NA_ROUTER_FLAG          (tt8Bit) 0x80

/* Mask used to test if the Neighbor Advertisement Solicited flag is set. */
#define TM_6_ND_NA_SOLICITED_FLAG       (tt8Bit) 0x40

/* Mask used to test if the Neighbor Advertisement Override flag is set. */
#define TM_6_ND_NA_OVERRIDE_FLAG        (tt8Bit) 0x20

/* Minimum size of a redirect message */
#define TM_6_MIN_REDIRECT_LEN           40

#define TM_6PAK_MIN_REDIRECT_LEN \
  tm_packed_byte_count(TM_6_MIN_REDIRECT_LEN)

/* ICMPv6 Redirect message type */
#define TM_6_ND_TYPE_REDIRECT           (tt8Bit) 137

/* ICMPv6 ND Source Link Layer Address option type */
#define TM_6_ND_TYPE_SOURCE_ADDR        (tt8Bit) 1

/* ICMPv6 ND Target Link Layer Address option type */
#define TM_6_ND_TYPE_TARGET_ADDR        (tt8Bit) 2

/* ICMPv6 ND Prefix Information option type */
#define TM_6_ND_TYPE_PREFIX_INFO        (tt8Bit) 3

/* Mask used to test if the ND Prefix Information on-link flag is set. */
#define TM_6_ND_PI_ONLINK_FLAG          (tt8Bit) 0x80

/* Mask used to test if the ND Prefix Information autonomous address
   configuration flag is set. */
#define TM_6_ND_PI_AUTO_FLAG            (tt8Bit) 0x40

/* Mask used to test if the ND Prefix Information Router Address flag is set */
#define TM_6_ND_PI_RTR_ADDR_FLAG        (tt8Bit) 0x20

/* ICMPv6 ND Redirected Header option type */
#define TM_6_ND_TYPE_REDIRECT_HDR       (tt8Bit) 4

/* ICMPv6 ND MTU option type */
#define TM_6_ND_TYPE_MTU                (tt8Bit) 5

/* ICMPv6 ND Router Advertisement Interval option type */
#define TM_6_ND_TYPE_RA_INTERVAL        (tt8Bit) 7

/* ICMPv6 ND Recursive DNS Server option type */
#define TM_6_ND_TYPE_RDNSS              (tt8Bit) 25

/* ICMPv6 ND DNS Search List option type */
#define TM_6_ND_TYPE_DNSSL              (tt8Bit) 31

/* Tell tf6IpIncomingPacket what the minimum number of bytes is that we expect
   to be kept contiguous starting with the base ICMPv6 header. */
#define TM_6_MAX_ICMP_HDR_LEN     (TM_6_MIN_ICMP_MSG_LEN + TM_6_IP_MIN_HDR_LEN)

/**
 ** IPv6 socket flags (soc6Flags)
 **/
/* Reflects inverse of socket option IPV6_MULTICAST_LOOP [RFC3493].
   "If a multicast datagram is sent to a group to which the sending
    host itself belongs (on the outgoing interface), a copy of the
    datagram is looped back by the IP layer for local delivery ..." */
#define TM_6_SOCF_NO_MCAST_LOOP         (tt16Bit)0x0001
/* Reflects socket option IPV6_V6ONLY [RFC3493].
   "This socket option restricts AF_INET6 sockets to IPv6 communications
    only." */
#define TM_6_SOCF_V6ONLY                (tt16Bit)0x0002


/* IPv6 routing entry flags */
/* Routing entry flag (i.e. ttRteEntry.rte6Flags) that indicates that the route
   is an IPv6-over-IPv4 configured tunnel.
   ([RFC2893].R4:10, [RFC2893].R4.1:10) */
#define TM_6_RTE_CONF_TUNL_FLAG         (tt16Bit) 0x0001
/* Routing entry flag (i.e. ttRteEntry.rte6Flags) that indicates that the route
   is an IPv6-over-IPv4 automatic tunnel.
   ([RFC2893].R5:10, [RFC2893].R5.3:10) */
#define TM_6_RTE_AUTO_TUNL_FLAG         (tt16Bit) 0x0002
/* Routing entry flag (i.e. ttRteEntry.rte6Flags) that indicates that the route
   is a generic IPv6 tunnel.
   ([RFC2473].R3.4:10) */
#define TM_6_RTE_IPV6_TUNL_FLAG         (tt16Bit) 0x0004
/* Routing entry flag (i.e. ttRteEntry.rte6Flags) indicating whether the
   neighbor is a router or a host. This corresponds to the Neighbor Cache
   IsRouter flag.
   ([RFC2461].R7.2.3:50, [RFC2461].R6.3.4:300, [RFC2461].R6.3.4:310) */
#define TM_6_RTE_IS_ROUTER_FLAG         (tt16Bit) 0x0008


/* IPv6 address cache flag (ttDeviceEntry.cachedAddrFlag) indicateing that a
   link-local IPv6 address has been configured on the interface (or has been
   set on the interface, and is pending the results of Duplicate Address
   Detection), therefore the interface is IPv6 multicast-capable. This flag is
   cleared to disable the IPv6 part of the interface by calling
   tf6DisableInterface.
   ([RFC2373].R2.1:10) */
#define TM_6_DEV_LINK_LOCAL_FLAG        (tt16Bit) 0x0001
#ifndef TM_6_USE_RFC3879 
/* site local address to be depricate */
/* IPv6 address cache flag (ttDeviceEntry.cachedAddrFlag) indicateing that a
   site-local IPv6 address has been configured on the interface. */
#define TM_6_DEV_SITE_LOCAL_FLAG        (tt16Bit) 0x0002
#endif /* TM_6_USE_RFC3879 */ 
/* IPv6 address cache flag (ttDeviceEntry.cachedAddrFlag) indicateing that a
   global scope IPv6-native address has been configured on the interface. */
#define TM_6_DEV_IPV6_NATIVE_FLAG       (tt16Bit) 0x0004
/* IPv6 address cache flag (ttDeviceEntry.cachedAddrFlag) indicateing that a
   global scope IPv4-compatible IPv6 address has been configured on the
   interface. */
#define TM_6_DEV_IPV4_COMPAT_FLAG       (tt16Bit) 0x0008
/* IPv6 address cache flag (ttDeviceEntry.cachedAddrFlag) indicateing that a
   global scope 6-to-4 IPv6 address has been configured on the interface. */
#define TM_6_DEV_6TO4_FLAG              (tt16Bit) 0x0010
/* IPv6 address cache flag (ttDeviceEntry.cachedAddrFlag) indicateing that a
   global scope private IPv6-native address has been configured on the
   interface. */
#define TM_6_DEV_LOCAL_FLAG             (tt16Bit) 0x0020

/*
 * IPv6 device flags
 * WARNING!!! flags >= 0x100 of dev6Flags are used by the public API
 * NOTE that 0x10 (TM_DEV_IP_TEMPORARY) is only used to let the stack know
 * that the address is temporary. It is not set on dev6Flags.
 */

/* IPv6 device flag (i.e. ttDeviceEntry.dev6Flags) that indicates that an
   interface ID has been assigned to the interface. The interface ID is used
   for stateless address auto-configuration. */
#define TM_6_DEV_INTERFACE_ID_FLAG      (tt16Bit) 0x0001

/* IPv6 device flag (i.e. ttDeviceEntry.dev6Flags) that indicates that a
   link-layer address has been configured on the interface, therefore the
   interface supports IPv6 address resolution.
   ([RFC2461].R7.2.2:30, [RFC2461].R7.2.2:40, [RFC2461].R7.2.4:30,
   [RFC2461].R7.2.6:30, [RFC2461].R6.3.7:40) */
#define TM_6_DEV_LL_ADDRESS_FLAG        (tt16Bit) 0x0002

/* IPv6 link-layer flag (i.e. ttDeviceEntry.dev6Flags) that indicates that IPv6
   connectivity is being attempted on the (PPP) interface. */
#define TM_6_LL_CONNECTING              (tt16Bit) 0x0004

/* IPv6 link-layer flag (i.e. ttDeviceEntry.dev6Flags) that indicates that IPv6
   connectivity has been established on the (PPP) interface. */
#define TM_6_LL_CONNECTED               (tt16Bit) 0x0008

/* IPv6 device flag (i.e. ttDeviceEntryPtr.dev6Flags) that indicates if the
   initial delay after opening an interface for the first time
   (tf6DelayInitSolTimer) has completed. */
#define TM_6_DEV_INIT_DELAY_COMPLETE    (tt16Bit) 0x0010

/* Closing, pending on sending the DHCPv6 release message and 
 * waiting for the reply or timeout 
 */
#define TM_6_DEV_CLOSING                (tt16Bit) 0x0020

#define TM_6_DEV_INIT_DELAY_STARTED     (tt16Bit) 0x0080

#ifdef TM_DSP
#define tm_6_ether_mcast(eth)           \
    (    ( (eth)[0] == 0x33 )           \
      && ( (eth)[1] == 0x33 ) )
#else /* !TM_DSP */
#define tm_6_ether_mcast(eth)           \
    (((tt16BitPtr)(eth))[0] == (tt16Bit)0x3333)
#endif /* !TM_DSP */

/* WARNING!!! flags >= 0x100 of dev6Flags are used by the public API */

#define tm_6_dev_forward(devPtr)        tm_16bit_one_bit_set( \
                                            devPtr->dev6Flags, \
                                            TM_6_DEV_IP_FORW_ENB)
#define tm_6_dev_mcast_forward(devPtr)  tm_16bit_all_bits_set( \
                                            devPtr->dev6Flags, \
                                            TM_6_DEV_IP_FORW_MCAST_ENB \
                                            | TM_6_DEV_IP_FORW_ENB)

/* device virtual channel */
#define tm_6_ip_dev_vchan(devPtr, mhomeIndex)  \
        (devPtr)->dev6VChanArray[mhomeIndex]

/* Lan link layer that supports address resolution and Neighbor Cache */
#define tm_6_ll_is_lan(devPtr) tm_4_ll_is_lan(devPtr)

/* IPv6 address flags (i.e. ttDeviceEntry.dev6IpAddrFlagsArray) */
/* IPv6 address flag that indicates that the IPv6 address is configured on
   the interface at the specified multi-home index. */
#define TM_6_DEV_IP_CONFIG              (tt8Bit)0x01
/* IPv6 address flag that indicates that configuration of the IPv6 address
   is being attempted on the interface at the specified multi-home index,
   i.e. Duplicate Address Detection is being performed. */
#define TM_6_DEV_IP_CONF_STARTED        (tt8Bit)0x02
/* IPv6 address flag that indicates that the IPv6 address at the specified
   multi-home index is deprecated. ([RFC2462].R5.5.4:30) */
#define TM_6_DEV_IP_DEPRECATED_FLAG     (tt8Bit)0x04
/* IPv6 address flag that indicates that the specified home address has an
   active mobility binding in the home agent. Used by Mobile IPv6. */
#define TM_6_DEV_MIP_REGISTERED_FLAG    (tt8Bit)0x08
/* Configured IP address is temporary */
#define TM_6_DEV_IPADDR_TEMPORARY       (tt8Bit)0x10
 

/* IPv6 Ping API stuff */
/* Default size of ping data. */
#define TM_6_PING_DEFAULT_DATALENGTH    (64 - TM_6_MIN_ICMP_MSG_LEN)
/* Maximum ICMPv6 data length */
#define TM_6_ICMP_MAX_DATALENGTH        (65535 - TM_6_IP_MIN_HDR_LEN)
/* Maximum PING data length (when over IPv6) */
#define TM_6_PING_MAX_DATALENGTH \
    (TM_6_ICMP_MAX_DATALENGTH - TM_6_MIN_ICMP_MSG_LEN)


/* IPv6 Multicast Listener Discovery stuff */
/* The maximum number of milliseconds to delay before retransmitting the
   initial MLD Report message.
   ([RFC2710].R4:160) */
#define TM_6_MLD_MAX_RETX_DELAY_MSEC    (tt16Bit) 3000


#ifndef TM_6_PREFIX_TIMER_RES_SEC
/* IPv6 Stateless Address Auto-configuration stuff */
/* Specifies the resolution (in seconds) of the address invalidation timer
   (i.e. tf6RtPrefixAgingTimer). */
/* to pass IPv6 Ready logo tests, we specify 1 second here */
#define TM_6_PREFIX_TIMER_RES_SEC      TM_UL(1)
#endif /* ! TM_6_PREFIX_TIMER_RES_SEC */


/* IPv6 Neighbor Discovery stuff (includes Address Resolution and
   Neighbor Unreachability Detection) */
/* MAX_MULTICAST_SOLICIT specified by RFC-2461. */
#define TM_6_ND_MAX_MCAST_SOLICIT       3
/* MAX_NEIGHBOR_ADVERTISEMENT specified by RFC-2461. */
#define TM_6_ND_MAX_NEIGH_ADVERT        3
#ifdef TM_6_USE_ARP_QUIET_TIME
/* Quiet time. Units are milliseconds. */
#define TM_6_ND_QUIET_TIME              TM_UL(10)
#endif /* TM_6_USE_ARP_QUIET_TIME */
/* REACHABLE_TIME specified by RFC-2461. Units are milliseconds. */
#define TM_6_ND_DEF_REACHABLE_TIME      30000
/* RETRANS_TIMER specified by RFC-2461. Units are milliseconds. */
#define TM_6_ND_DEF_RETRANS_TIMER       1000
/* DELAY_FIRST_PROBE_TIME specified by RFC-2461. Units are milliseconds. */
#define TM_6_ND_DELAY_1ST_PROBE_TIME    5000
/* The maximum amount of time between when the reachable time (used for
   neighbor unreachability detection) is randomized when receiving identical
   base reachable time values from a router. Units are milliseconds. */
#define TM_6_ND_RANDOMIZE_DELAY         TM_UL(7200000)
/* Minimum prefix lifetime to accept from an unauthenticated router advert,
   in seconds (2 hours) */
#define TM_6_MIN_UNAUTH_PREF_LIFETIME   7200

/* longest possible reachable time, in milliseconds, 1 hour */
#define TM_6_NUD_LONGEST_REACHABLE_TIME TM_UL(3600000)
    
/* IPv6 Neighbor Unreachability Detection states */
/* special value for NUD state indicates that this is an IPv4 ARP cache entry
   for an ARP entry that is in the routing tree */
#define TM_6_NUD_IPV4_STATE             0
/* the entry is just cloned - this includes INCOMPLETE state */
#define TM_6_NUD_CLONED_STATE           1
/* Confirmed neighbor reachability. */
#define TM_6_NUD_REACHABLE_STATE        2
/* Unconfirmed neighbor reachability. */
#define TM_6_NUD_STALE_STATE            4
/* Waiting for neighbor reachability confirmation before entering
   TM_6_NUD_PROBE_STATE. */
#define TM_6_NUD_DELAY_STATE            8
/* Actively probing. The neighbor is no longer known to be reachable, and
   Neighbor Solicitation probes are being sent to verify neighbor
   reachability. */
#define TM_6_NUD_PROBE_STATE            16
/* Invalidated Neighbor Cache entry. */
#define TM_6_NUD_INVALID_STATE          32

/* values for updateFlags parameter of tf6RtArpUpdateByRte */
#define TM_6_ARP_UPDATE_PHYS_ADDR       (tt16Bit) 0x0001
#define TM_6_ARP_UPDATE_NUD_STATE       (tt16Bit) 0x0002
#define TM_6_ARP_UPDATE_IS_ROUTER       (tt16Bit) 0x0004
#define TM_6_ARP_UPDATE_CACHE_LOCKED    (tt16Bit) 0x1000

/* Command values for tf6RtKeyCommand */
#define TM_6_RT_KEY_GET_LIFETIMES       0
#define TM_6_RT_KEY_SET_LIFETIMES       1
#define TM_6_RT_KEY_EXISTS              2
#define TM_6_RT_KEY_SET_ENDPOINT        3
#define TM_6_RT_KEY_EXISTS_ON_DEV       4

/*
 * The default time (in seconds) to wait before increasing the IPv6 Path MTU
 * estimate.  Should never be less than 5 minutes.  ([RFC1981].R4:30)
 */
#define TM_6_RT_PMTU_TIMEOUT 600

/* Default values for IPv6 options */
#ifndef TM_6_DAD_DEF_XMITS      
/* Default value of interface option TM_6_DEV_OPTIONS_DAD_XMITS.
   ([RFC2462].R5.1:50) */
#define TM_6_DAD_DEF_XMITS              1
#endif /* ! TM_6_DAD_DEF_XMITS */
#ifndef TM_6_SOL_DEF_TIMEOUT
/* RFC-2461 RTR_SOLICITATION_INTERVAL:
   Default value (in milliseconds) of interface option
   TM_6_DEV_OPTIONS_SOL_TIMEOUT. */
#define TM_6_SOL_DEF_TIMEOUT            4000
#endif /* ! TM_6_SOL_DEF_TIMEOUT */
#ifndef TM_6_SOL_DEF_INIT_DELAY 
/* RFC-2461 MAX_RTR_SOLICITATION_DELAY:
   Default value (in milliseconds) of interface option
   TM_6_DEV_OPTIONS_SOL_INIT_DELAY. */
#define TM_6_SOL_DEF_INIT_DELAY         1000
#endif /* ! TM_6_SOL_DEF_INIT_DELAY */
#ifndef TM_6_SOL_DEF_RETRY      
/* RFC-2461 MAX_RTR_SOLICITATIONS:
   Default value of interface option TM_6_DEV_OPTIONS_SOL_RETRY.
   ([RFC2461].R6.3.7:70) */
#define TM_6_SOL_DEF_RETRY              2
#endif /* ! TM_6_SOL_DEF_RETRY */
#ifndef TM_6_IP_DEF_FORWARD             
/* Default value of Treck option TM_6_OPTION_IP_FORWARDING (IPv6 forwarding
   disabled). */
#define TM_6_IP_DEF_FORWARD             0
#endif /* ! TM_6_IP_DEF_FORWARD */
#ifndef TM_6_IP_DEF_FRAGMENT            
/* Default value of Treck option TM_6_OPTION_IP_FRAGMENT (IPv6 fragmentation
   enabled). ([RFC2460].R5:70) */
#define TM_6_IP_DEF_FRAGMENT            1
#endif /* ! TM_6_IP_DEF_FRAGMENT */
#ifndef TM_6_IP_DEF_HOPS                
/* The initial default value of the socket-level option IPV6_UNICAST_HOPS,
   which is used to set the hop limit for outgoing unicast packets. Note that
   this can be overridden by parameter discovery. ([IPV6REQ].R2.4:10) */
#define TM_6_IP_DEF_HOPS                64
#endif /* ! TM_6_IP_DEF_HOPS */
#ifndef TM_6_IP_DEF_MULTICAST_HOPS      
/* The default value of the socket-level option IPV6_MULTICAST_HOPS, which is
   used to set the hop limit for outgoing multicast packets.
   ([IPV6REQ].R2.18:10) */
#define TM_6_IP_DEF_MULTICAST_HOPS      1
#endif /* ! TM_6_IP_DEF_MULTICAST_HOPS */
#ifndef TM_6_IP_DEF_FRAG_TTL            
/* Default value (in seconds) of the Treck option TM_6_OPTION_IP_FRAG_TTL
   (IPv6 fragment reassembly time). */
#define TM_6_IP_DEF_FRAG_TTL            60
#endif /* ! TM_6_IP_DEF_FRAG_TTL */
#ifndef TM_6_IP_DEF_FRAG_RES
/* Default IPv6 fragment reassembly timer resolution. */
#define TM_6_IP_DEF_FRAG_RES            TM_UL(1000)
#endif /* TM_6_IP_DEF_FRAG_RES */

#ifndef TM_6_IP_FRAG_Q_SIZE
/* Default number of IP datagrams waiting for reassembly */
#define TM_6_IP_FRAG_Q_SIZE               (tt8Bit)5
#endif /* TM_6_IP_FRAG_Q_SIZE */
#ifndef TM_6_IP_FRAG_FAILED_Q_SIZE
/* Default number of IP datagrams too big to be reassembled that we continue
 * tracking, in case more fragments arrive */
#define TM_6_IP_FRAG_FAILED_Q_SIZE        (tt8Bit)5
#endif /* TM_6_IP_FRAG_FAILED_Q_SIZE */
#ifndef TM_6_IP_FRAG_ENTRY_MAX_SIZE
/* Default maximum IP datagram data size (without IP header) in the IP 
   reassembly queue. */
#define TM_6_IP_FRAG_ENTRY_MAX_SIZE       TM_UL(8200)
#endif /* TM_6_IP_FRAG_ENTRY_MAX_SIZE */
#ifndef TM_6_ICMP_DEF_TX_ERR_LIMIT_MSEC 
/* Default value (in milliseconds) of Treck option
   TM_6_OPTION_ICMP_TX_ERR_LIMIT (rate limit sending ICMPv6 error messages).
   ([RFC2463].R2.4:110 updated by [RFC4443].R2.4(f)) */
#define TM_6_ICMP_DEF_TX_ERR_LIMIT_MSEC 500
#else
#if (TM_6_ICMP_DEF_TX_ERR_LIMIT_MSEC == 0)
#error "TM_6_ICMP_DEF_TX_ERR_LIMIT_MSEC must not be zero!"
#endif /* TM_6_ICMP_DEF_TX_ERR_LIMIT_MSEC == 0 */
#endif /* ! TM_6_ICMP_DEF_TX_ERR_LIMIT_MSEC */
#ifndef TM_6_ICMP_DEF_TX_ERR_LIMIT_BURST
/* Default value (number of packets) of Treck option
   TM_6_OPTION_ICMP_TX_ERR_BURST (burst limit sending ICMPv6 error messages).
   ( RFC4443 section 2.4(f) ) */
#define TM_6_ICMP_DEF_TX_ERR_LIMIT_BURST 10
#else
#if (TM_6_ICMP_DEF_TX_ERR_LIMIT_BURST == 0)
#error "TM_6_ICMP_DEF_TX_ERR_LIMIT_BURST must not be zero!"
#endif /* TM_6_ICMP_DEF_TX_ERR_LIMIT_BURST == 0 */
#endif /* ! TM_6_ICMP_DEF_TX_ERR_LIMIT_BURST */
#ifndef TM_6_ND_MAX_UNICAST_SOLICIT     
/* RFC-2461 MAX_UNICAST_SOLICIT:
   Default value of Treck option TM_6_OPTION_ND_MAX_RETRY */
#define TM_6_ND_MAX_UNICAST_SOLICIT     3
#endif /* ! TM_6_ND_MAX_UNICAST_SOLICIT */
#ifndef TM_6_IP_DEF_DEPRECATE_ADDR  
/* Default value of Treck option TM_6_OPTION_IP_DEPRECATE_ADDRESS (do not
   prevent any new communication from using a deprecated IPv6 address).
   ([RFC2462].R5.5.4:70) */
#define TM_6_IP_DEF_DEPRECATE_ADDR      0
#endif /* ! TM_6_IP_DEF_DEPRECATE_ADDR */
#ifndef TM_6_MIP_DEF_RO_ENABLE
/* Default value of Treck option TM_6_OPTION_MIP_RO_ENABLE (enable Mobile
   IPv6 route optimization). ([MIPV6_18++].R14.4.5:20) */
#define TM_6_MIP_DEF_RO_ENABLE          1
#endif /* ! TM_6_MIP_DEF_RO_ENABLE */
#ifndef TM_6_MN_DEF_EAGER_CELL_SWITCH
/* Default value of Treck option TM_6_OPTION_MN_EAGER_CELL_SWITCH (disable
   mobile node eager cell switching). */
#define TM_6_MN_DEF_EAGER_CELL_SWITCH   0
#endif /* ! TM_6_MN_DEF_EAGER_CELL_SWITCH */
#ifndef TM_6_CN_DEF_MAX_BINDING_LIFE
/* Default value (in seconds) of Treck option TM_6_OPTION_MAX_BINDING_LIFETIME
   (maximum lifetime of Mobile IPv6 Correspondent Node binding cache entry).
   ([MIPV6_18++].R5.2.6:50) */
#define TM_6_CN_DEF_MAX_BINDING_LIFE    420
#endif /* ! TM_6_CN_DEF_MAX_BINDING_LIFE */
#ifndef TM_6_MN_DEF_REG_LT_BIAS
/* Default value (in seconds) of Treck option TM_6_OPTION_MN_REG_LT_BIAS
   (binding registration lifetime downward bias for mobile node
   re-registration). */
#define TM_6_MN_DEF_REG_LT_BIAS         300
#endif /* ! TM_6_MN_DEF_REG_LT_BIAS */
#ifndef TM_6_MN_DEF_DEREG_TIMEOUT 
/* Default value (in seconds) of Treck option TM_6_OPTION_MN_DEREG_TIMEOUT
   (timeout period for mobile node auto-deregistration). */
#define TM_6_MN_DEF_DEREG_TIMEOUT       10
#endif /* ! TM_6_MN_DEF_DEREG_TIMEOUT */
#ifndef TM_6_RR_DEF_MAX_BCE_ENTRIES
/* Default value of Treck option TM_6_OPTION_RR_MAX_BCE_ENTRIES (maximum
   number of correspondent node binding cache entries). */
#define TM_6_RR_DEF_MAX_BCE_ENTRIES     8
#endif /* ! TM_6_RR_DEF_MAX_BCE_ENTRIES */
#ifndef TM_6_RR_DEF_MAX_BUL_ENTRIES
/* Default value of Treck option TM_6_OPTION_RR_MAX_BUL_ENTRIES (maximum
   number of mobile node route optimization binding update list entries). */
#define TM_6_RR_DEF_MAX_BUL_ENTRIES     16
#endif /* ! TM_6_RR_DEF_MAX_BUL_ENTRIES */
#ifndef TM_6_MN_DEF_BEACON_THRESHOLD
/* Default value of Treck option TM_6_OPTION_MN_BEACON_THRESHOLD (maximum
   number of consecutive Router Advertisement "beacons" from its current
   default router that the mobile node can tolerate dropping). */
#define TM_6_MN_DEF_BEACON_THRESHOLD    4
#endif /* ! TM_6_MN_DEF_BEACON_THRESHOLD */
#ifndef TM_6_MN_DEF_1ST_REG_INIT_TIMEOUT
/* Default value (in milliseconds) of Treck option
   TM_6_OPTION_MN_1ST_REG_INIT_TIMEOUT (InitialBindackTimeoutFirstReg Mobile
   IPv6 protocol configuration variable. ([ISSUE181].R13:50) */
#define TM_6_MN_DEF_1ST_REG_INIT_TIMEOUT 1500
#endif /* ! TM_6_MN_DEF_1ST_REG_INIT_TIMEOUT */

#if (defined(TM_6_USE_MIP_MN) && !defined(TM_6_DISABLE_TAHI_MN_TEST))
/* a number of TAHI MIPv6 MN (Mobile Node) tests require the TM_6_TAHI_MN_TEST
   macro be #define'd to pass */
#define TM_6_TAHI_MN_TEST
#endif /* ! TM_6_DISABLE_TAHI_MN_TEST */
    
/*
 * Ethernet type for IPv6.
 */
#ifdef TM_LITTLE_ENDIAN
#define TM_6_ETHER_IP_TYPE 0xDD86
#else /* TM_BIG_ENDIAN */
#define TM_6_ETHER_IP_TYPE 0x86DD
#endif /* TM_LITTLE_ENDIAN */
    
/* tm_6_netmask
   Description: IPv6 macro used to calculate the IPv6 netmask from a
        specified prefix length.
   Parameters:
        (IN) int prefixLen:
            the prefix length in bits
        (OUT) tt6IpAddressPtr netmaskPtr:
            pointer to the IPv6 netmask to set
*/
#define tm_6_netmask(prefixLen, netMaskPtr) \
    tf6PrefixLenToNetmask((prefixLen), (netMaskPtr))

/* tm_6_ip_match
   Description: compare 2 IPv6 addresses to see if they are the same
   Parameters:
        (IN) tt6IpAddressPtr ipAddr1Ptr:
            pointer to the 1st IPv6 address to compare
        (IN) tt6IpAddressPtr ipAddr2Ptr:
            pointer to the 2nd IPv6 address to compare
   Notes:
        IPv6 addresses will typically be on the same subnet but have different
        interface IDs, so we optimize for that case by checking for a match on
        the network part of the address last.
*/
#define tm_6_ip_match(ipAddr1Ptr, ipAddr2Ptr) \
    (((ipAddr1Ptr)->s6LAddr[3] == (ipAddr2Ptr)->s6LAddr[3]) \
     && ((ipAddr1Ptr)->s6LAddr[2] == (ipAddr2Ptr)->s6LAddr[2]) \
     && ((ipAddr1Ptr)->s6LAddr[1] == (ipAddr2Ptr)->s6LAddr[1]) \
     && ((ipAddr1Ptr)->s6LAddr[0] == (ipAddr2Ptr)->s6LAddr[0]))

/* tm_6_ip_local_match
   Description: compare 2 Link Local IPv6 addresses to see if they are the same
   without comparing the scope id.
   Parameters:
        (IN) tt6IpAddressPtr ipAddr1Ptr:
            pointer to the 1st IPv6 address to compare
        (IN) tt6IpAddressPtr ipAddr2Ptr:
            pointer to the 2nd IPv6 address to compare
   Notes:
        IPv6 addresses will typically be on the same subnet but have different
        interface IDs, so we optimize for that case by checking for a match on
        the network part of the address last.
*/
#define tm_6_ip_local_match(ipAddr1Ptr, ipAddr2Ptr) \
    (((ipAddr1Ptr)->s6LAddr[3] == (ipAddr2Ptr)->s6LAddr[3]) \
     && ((ipAddr1Ptr)->s6LAddr[2] == (ipAddr2Ptr)->s6LAddr[2]) \
     && ((ipAddr1Ptr)->s6LAddr[0] == (ipAddr2Ptr)->s6LAddr[0]))

/* tm_6_ip_local_match
   Description: compare the scope ID of 2 Link Local IPv6 addresses to see if
   they are the same
   Parameters:
        (IN) tt6IpAddressPtr ipAddr1Ptr:
            pointer to the 1st IPv6 address to compare
        (IN) tt6IpAddressPtr ipAddr2Ptr:
            pointer to the 2nd IPv6 address to compare
*/
#define tm_6_ip_scope_match(ipAddr1Ptr, ipAddr2Ptr) \
     ((ipAddr1Ptr)->s6LAddr[1] == (ipAddr2Ptr)->s6LAddr[1])

/* tm_6_ip_not_match
   Description: compare 2 IPv6 addresses to see if they are different
   Parameters:
        (IN) tt6IpAddressPtr ipAddr1Ptr:
            pointer to the 1st IPv6 address to compare
        (IN) tt6IpAddressPtr ipAddr2Ptr:
            pointer to the 2nd IPv6 address to compare
   Notes:
        IPv6 addresses will typically be on the same subnet but have different
        interface IDs, so we optimize for that case by checking for a
        mismatch on the network part of the address last.
*/
#define tm_6_ip_not_match(ipAddr1Ptr, ipAddr2Ptr) \
    (((ipAddr1Ptr)->s6LAddr[3] != (ipAddr2Ptr)->s6LAddr[3]) \
     || ((ipAddr1Ptr)->s6LAddr[2] != (ipAddr2Ptr)->s6LAddr[2]) \
     || ((ipAddr1Ptr)->s6LAddr[1] != (ipAddr2Ptr)->s6LAddr[1]) \
     || ((ipAddr1Ptr)->s6LAddr[0] != (ipAddr2Ptr)->s6LAddr[0]))

/* tm_6_ip_zero
   Description: test IPv6 address to see if it is the unspecified address
   Parameters:
        (IN) tt6IpAddressPtr ipAddrPtr:
            pointer to the IPv6 address to test
*/
#define tm_6_ip_zero(ipAddrPtr) \
    (((ipAddrPtr)->s6LAddr[0] == TM_UL(0)) \
     && ((ipAddrPtr)->s6LAddr[1] == TM_UL(0)) \
     && ((ipAddrPtr)->s6LAddr[2] == TM_UL(0)) \
     && ((ipAddrPtr)->s6LAddr[3] == TM_UL(0)))

/* tm_6_ip_not_zero
   Description: test IPv6 address to see if it is not the unspecified address
   Parameters:
        (IN) tt6IpAddressPtr ipAddrPtr:
            pointer to the IPv6 address to test
*/
#define tm_6_ip_not_zero(ipAddrPtr)                                         \
    (((ipAddrPtr)->s6LAddr[0] != TM_UL(0))                                  \
     || ((ipAddrPtr)->s6LAddr[1] != TM_UL(0))                               \
     || ((ipAddrPtr)->s6LAddr[2] != TM_UL(0))                               \
     || ((ipAddrPtr)->s6LAddr[3] != TM_UL(0)))

/* tm_6_ip_net_match
   Description: compare 2 IPv6 addresses to see if they are on the same subnet
   Parameters:
        (IN) tt6IpAddressPtr ipAddr1Ptr:
            pointer to the 1st IPv6 address to compare
        (IN) tt6IpAddressPtr ipAddr2Ptr:
            pointer to the 2nd IPv6 address to compare
        (IN) tt6IpAddressPtr netmaskPtr:
            pointer to the IPv6 netmask
   Notes:
        IPv6 address prefixes are normally 64-bits in length, so we optimize
        for that case.
*/
#define tm_6_ip_net_match(ipAddr1Ptr, ipAddr2Ptr, netmaskPtr)               \
    (tm_ip_zero(tm_ip_net(tm_ip_xor(                                        \
            (ipAddr1Ptr)->s6LAddr[0],                                       \
            (ipAddr2Ptr)->s6LAddr[0]),                                      \
        (netmaskPtr)->s6LAddr[0]))                                          \
     && tm_ip_zero(tm_ip_net(tm_ip_xor(                                     \
            (ipAddr1Ptr)->s6LAddr[1],                                       \
            (ipAddr2Ptr)->s6LAddr[1]),                                      \
        (netmaskPtr)->s6LAddr[1]))                                          \
     && (((netmaskPtr)->s6LAddr[2] == TM_UL(0))                             \
         || tm_ip_zero(tm_ip_net(                                           \
                tm_ip_xor((ipAddr1Ptr)->s6LAddr[2],                         \
                    (ipAddr2Ptr)->s6LAddr[2]),                              \
                (netmaskPtr)->s6LAddr[2])))                                 \
     && (((netmaskPtr)->s6LAddr[3] == TM_UL(0))                             \
         || tm_ip_zero(tm_ip_net(                                           \
                tm_ip_xor((ipAddr1Ptr)->s6LAddr[3],                         \
                    (ipAddr2Ptr)->s6LAddr[3]),                              \
                (netmaskPtr)->s6LAddr[3]))))

/* 
 * tm_6_ip_net
 *  Description: Extract network portion of an address.
 *  Parameters:
 *       (OUT) tt6IpAddressPtr   resultIpAddrPtr: 
 *                         Pointer to IPv6 address to store result into
 *       (IN) tt6IpAddressPtr ipAddrPtr: pointer to IPv6 address
 *       (IN) tt6IpAddressPtr netMaskPtr: pointer to IPv6 netmask
 */
#define tm_6_ip_net(resultIpAddrPtr, ipAddrPtr, netMaskPtr)                 \
{                                                                           \
    (resultIpAddrPtr)->s6LAddr[0] =                                       \
            tm_ip_net((ipAddrPtr)->s6LAddr[0], (netMaskPtr)->s6LAddr[0]);   \
    (resultIpAddrPtr)->s6LAddr[1] =                                       \
            tm_ip_net((ipAddrPtr)->s6LAddr[1], (netMaskPtr)->s6LAddr[1]);   \
    (resultIpAddrPtr)->s6LAddr[2] =                                       \
            tm_ip_net((ipAddrPtr)->s6LAddr[2], (netMaskPtr)->s6LAddr[2]);   \
    (resultIpAddrPtr)->s6LAddr[3] =                                       \
            tm_ip_net((ipAddrPtr)->s6LAddr[3], (netMaskPtr)->s6LAddr[3]);   \
}

/*
 * tm_6_ip_prefix_to_long_index(prefixLength, longIndex)
 * Description: Given a prefix length, find out the index in the 4 long
 * array where the end of the netmask lands. 
 *       (IN)  int   prefixLength 
 *       (OUT) int   longIndex. 
 */
#define tm_6_ip_prefix_to_long_index(prefixLength, longIndex)               \
{                                                                           \
    longIndex = 0;                                                          \
    if ((prefixLength) != 0)                                                \
    {                                                                       \
        longIndex = ((prefixLength) - 1) >> 5;                              \
    }                                                                       \
}

/*
 * tm_6_ip_prefix_to_4_prefix(prefixLength, longIndex, longPrefix)
 * Description: Given an IPV6 prefix length, and a long index 
 * (i.e index in array of 4 longs where the prefix length ends), 
 * find the sub-prefix length in the 32-bit long word.
 * For example a prefix lengh == 60, with long index 1, will
 * yield, 28. 
 * Note that this macro should only be called with a longIndex
 * such that the prefix length ends in the 32-bit word referenced by
 * that long index. Given a valid prefix length, the 
 * tm_6_ip_prefix_to_long_index macro yields a long index that meets
 * this criteria.
 * Parameters
 *       (IN)  int   prefixLength 
 *       (IN)  int   longIndex 
 *  Returns
 *       int   longPrefix. 
 */
#define tm_6_ip_prefix_to_4_prefix(prefixLength, longIndex)               \
         ((prefixLength) - ((longIndex) << 5))

/*
 * tm_6_ip_prefix_to_4_mask(prefixLength, longIndex)
 * Description: Given an IPV6 prefix length, and a long index 
 * (i.e index in the array of 4 longs, where the prefix length ends), 
 * find the 32-bit mask.
 * For example a prefix lengh == 60, with long index 1, will
 * yield, 0xFFFFFF00. 
 * Note that this macro should only be called with a longIndex
 * such that the prefix length ends in the 32-bit word referenced by
 * that long index. Given a valid prefix length, the 
 * tm_6_ip_prefix_to_long_index macro yields a long index that meets
 * this criteria.
 * Parameters
 *       (IN)  int   prefixLength 
 *       (IN)  int   longIndex 
 *  Returns
 *       int   longMask. 
 */
#define tm_6_ip_prefix_to_4_mask(prefixLength, longIndex)                   \
  tvRt32ContiguousBitsPtr[tm_6_ip_prefix_to_4_prefix(prefixLength, longIndex)]


/* 
 * tm_6_ip_net_prefix
 * Description: Extract network portion of an address.
 *  Parameters:
 *       (IN/OUT) tt6IpAddress   ipAddrPtr: 
 *                      OUT   Pointer to IPv6 address to store result into
 *                      IN    Pointer to IPv6 address
 *       (IN)     int prefixLength: IP address prefix length.
 *       (TEMP)   int maskIndex
 */
#define tm_6_ip_net_prefix(ipAddrPtr, prefixLength, maskIndex)              \
{                                                                           \
    tm_6_ip_prefix_to_long_index(prefixLength, maskIndex)                   \
    ipAddrPtr->s6LAddr[maskIndex] = tm_ip_net(                              \
                 (ipAddrPtr)->s6LAddr[maskIndex],                           \
                 tm_6_ip_prefix_to_4_mask(prefixLength, maskIndex));        \
    while (maskIndex < 3)                                                   \
    {                                                                       \
        maskIndex++;                                                        \
        (ipAddrPtr)->s6LAddr[maskIndex] = TM_IP_ZERO;                       \
    }                                                                       \
}

/* 
 * tm_6_ip_net_match_prefix
 *  Description: compare 2 IPv6 addresses to see if they are on the same subnet
 *  using the prefix length
 *  Parameters:
 *       (OUT) int            match
 *            result 
 *       (IN) tt6IpAddressPtr ipAddr1Ptr:
 *           pointer to the 1st IPv6 address to compare
 *       (IN) tt6IpAddressPtr ipAddr2Ptr:
 *           pointer to the 2nd IPv6 address to compare
 *        (IN) int prefix1Length:
 *           prefixLength 
 *       (TEMP) maskIndex
 *       (TEMP) i
 *  Notes:
 *       IPv6 address prefixes are normally 64-bits in length, so we optimize
 *       for that case.
 */
#define tm_6_ip_net_match_prefix(match, ipAddr1Ptr, ipAddr2Ptr,             \
                                 prefixLength, maskIndex, i)                \
{                                                                           \
    tm_6_ip_prefix_to_long_index(prefixLength, maskIndex)                   \
    match = 1;                                                              \
    for (i= 0; i < maskIndex; i++)                                          \
    {                                                                       \
        if ( tm_ip_not_zero(tm_ip_xor((ipAddr1Ptr)->s6LAddr[i],             \
                                    (ipAddr2Ptr)->s6LAddr[i])) )            \
        {                                                                   \
            match = 0;                                                      \
            break;                                                          \
        }                                                                   \
    }                                                                       \
    if (match == 1)                                                         \
    {                                                                       \
        if (tm_ip_not_zero(tm_ip_net(                                       \
                              tm_ip_xor((ipAddr1Ptr)->s6LAddr[maskIndex],   \
                                        (ipAddr2Ptr)->s6LAddr[maskIndex]),  \
                       tm_6_ip_prefix_to_4_mask(prefixLength, maskIndex)))) \
        {                                                                   \
            match = 0;                                                      \
        }                                                                   \
    }                                                                       \
}

/*
 * tm_6_ip_net_match_leaf
 * Description: Find if there is a network match on the leaf
 * Parameters:
 *      (OUT) match 
 *          Store the result of the match (1 == success, 0 == failure)
 *      (IN) leafAddrPtr 
 *          Pointer to leaf node  Key IP address
 *      (IN)  ipAddrPtr
 *          Pointer to the IP address to match with
 *      (INT) leaf32Mask
 *            32-bit netmask to match at lMaskIndex
 *      (IN)  lMaskIndex
 *            Index in array of long work where the leaf32Mask is relevant
 *      (TEMP)i
 */
#define tm_6_ip_net_match_leaf(match, leafAddrPtr, ipAddrPtr, leaf32Mask,   \
                               lMaskIndex, i)                               \
{                                                                           \
    match = 1;                                                              \
    for (i= 0; (tt16Bit)i < lMaskIndex; i++)                                \
    {                                                                       \
        if ( tm_ip_not_zero(tm_ip_xor((leafAddrPtr)->s6LAddr[i],            \
                                      (ipAddrPtr)->s6LAddr[i])) )           \
        {                                                                   \
            match = 0;                                                      \
            break;                                                          \
        }                                                                   \
    }                                                                       \
    if (match == 1)                                                         \
    {                                                                       \
        if (tm_ip_not_zero(tm_ip_net(                                       \
                             tm_ip_xor((leafAddrPtr)->s6LAddr[lMaskIndex],  \
                                       (ipAddrPtr)->s6LAddr[lMaskIndex]),   \
                             leaf32Mask )))                                 \
        {                                                                   \
            match = 0;                                                      \
        }                                                                   \
    }                                                                       \
}

/*
 * tm_6_ip_net_match_leaf_prefix
 * Description: Find if there is a network match on the leaf
 * Parameters:
 *      (OUT) match 
 *          Store the result of the match (1 == success, 0 == failure)
 *      (IN) leafAddrPtr 
 *          Pointer to leaf node  Key IP address
 *      (IN)  ipAddrPtr
 *          Pointer to the IP address to match with
 *      (IN)  lMaskIndex
 *            Index in array of long work where the prefix ends.
 *      (IN)  lMaskPrefix
 *            subLength of prefix in the long word where the prefix ends. 
 *      (TEMP)i
 */
#define tm_6_ip_net_match_leaf_prefix(match, leafAddrPtr, ipAddrPtr,        \
                                      lMaskIndex, lMaskPrefix, i)           \
{                                                                           \
    match = 1;                                                              \
    for (i= 0; (tt16Bit)i < lMaskIndex; i++)                                \
    {                                                                       \
        if ( tm_ip_not_zero(tm_ip_xor((leafAddrPtr)->s6LAddr[i],            \
                                      (ipAddrPtr)->s6LAddr[i])) )           \
        {                                                                   \
            match = 0;                                                      \
            break;                                                          \
        }                                                                   \
    }                                                                       \
    if (match == 1)                                                         \
    {                                                                       \
        if (tm_ip_not_zero(tm_ip_net(                                       \
                             tm_ip_xor((leafAddrPtr)->s6LAddr[lMaskIndex],  \
                                       (ipAddrPtr)->s6LAddr[lMaskIndex]),   \
                             tvRt32ContiguousBitsPtr[lMaskPrefix] )))       \
        {                                                                   \
            match = 0;                                                      \
        }                                                                   \
    }                                                                       \
}

/* returns TRUE if ipAddr1Ptr address is greater than ipAddr2Ptr address */
#define tm_6_ip_greater_than(ipAddr1Ptr, ipAddr2Ptr) \
    (((ipAddr1Ptr)->s6LAddr[0] > (ipAddr2Ptr)->s6LAddr[0]) \
     || (((ipAddr1Ptr)->s6LAddr[0] == (ipAddr2Ptr)->s6LAddr[0]) \
         && ((ipAddr1Ptr)->s6LAddr[1] > (ipAddr2Ptr)->s6LAddr[1])) \
     || (((ipAddr1Ptr)->s6LAddr[0] == (ipAddr2Ptr)->s6LAddr[0]) \
         && ((ipAddr1Ptr)->s6LAddr[1] == (ipAddr2Ptr)->s6LAddr[1]) \
         && ((ipAddr1Ptr)->s6LAddr[2] > (ipAddr2Ptr)->s6LAddr[2])) \
     || (((ipAddr1Ptr)->s6LAddr[0] == (ipAddr2Ptr)->s6LAddr[0]) \
         && ((ipAddr1Ptr)->s6LAddr[1] == (ipAddr2Ptr)->s6LAddr[1]) \
         && ((ipAddr1Ptr)->s6LAddr[2] == (ipAddr2Ptr)->s6LAddr[2]) \
         && ((ipAddr1Ptr)->s6LAddr[3] > (ipAddr2Ptr)->s6LAddr[3])))

/* returns TRUE if ipAddr1Ptr address is less than ipAddr2Ptr address */
#define tm_6_ip_less_than(ipAddr1Ptr, ipAddr2Ptr) \
    (((ipAddr1Ptr)->s6LAddr[0] < (ipAddr2Ptr)->s6LAddr[0]) \
     || (((ipAddr1Ptr)->s6LAddr[0] == (ipAddr2Ptr)->s6LAddr[0]) \
         && ((ipAddr1Ptr)->s6LAddr[1] < (ipAddr2Ptr)->s6LAddr[1])) \
     || (((ipAddr1Ptr)->s6LAddr[0] == (ipAddr2Ptr)->s6LAddr[0]) \
         && ((ipAddr1Ptr)->s6LAddr[1] == (ipAddr2Ptr)->s6LAddr[1]) \
         && ((ipAddr1Ptr)->s6LAddr[2] < (ipAddr2Ptr)->s6LAddr[2])) \
     || (((ipAddr1Ptr)->s6LAddr[0] == (ipAddr2Ptr)->s6LAddr[0]) \
         && ((ipAddr1Ptr)->s6LAddr[1] == (ipAddr2Ptr)->s6LAddr[1]) \
         && ((ipAddr1Ptr)->s6LAddr[2] == (ipAddr2Ptr)->s6LAddr[2]) \
         && ((ipAddr1Ptr)->s6LAddr[3] < (ipAddr2Ptr)->s6LAddr[3])))

/* tm_6_ip_copy
   Description: copy one IPv6 address to another
   Parameters:
        (IN) tt6IpAddressPtr srcIpAddrPtr:
            pointer to the IPv6 address to copy from
        (OUT) tt6IpAddressPtr destIpAddrPtr:
            pointer to the IPv6 address to copy to
*/
#define tm_6_ip_copy(srcIpAddrPtr, destIpAddrPtr) \
{ \
    (destIpAddrPtr)->s6LAddr[0] = (srcIpAddrPtr)->s6LAddr[0]; \
    (destIpAddrPtr)->s6LAddr[1] = (srcIpAddrPtr)->s6LAddr[1]; \
    (destIpAddrPtr)->s6LAddr[2] = (srcIpAddrPtr)->s6LAddr[2]; \
    (destIpAddrPtr)->s6LAddr[3] = (srcIpAddrPtr)->s6LAddr[3]; \
}

/* 
 * tm_6_ip_copy_dststruct
 *  Description: copy one IPv6 address to another
 *  Parameters:
 *       (IN) tt6IpAddressPtr srcIpAddrPtr:
 *           pointer to the IPv6 address to copy from
 *       (OUT) tt6IpAddress destIpAddr:
 *           IPv6 address structure to copy to
 */
#define tm_6_ip_copy_dststruct(srcIpAddrPtr, destIpAddr) \
{ \
    (destIpAddr).s6LAddr[0] = (srcIpAddrPtr)->s6LAddr[0]; \
    (destIpAddr).s6LAddr[1] = (srcIpAddrPtr)->s6LAddr[1]; \
    (destIpAddr).s6LAddr[2] = (srcIpAddrPtr)->s6LAddr[2]; \
    (destIpAddr).s6LAddr[3] = (srcIpAddrPtr)->s6LAddr[3]; \
}

/* 
 * tm_6_ip_copy_srcstruct
 *  Description: copy one IPv6 address to another
 *  Parameters:
 *       (IN) tt6IpAddress srcIpAddr:
 *           IPv6 address to copy from
 *       (OUT) tt6IpAddressPtr destIpAddrPtr:
 *           pointer to the IPv6 address to copy to
 */
#define tm_6_ip_copy_srcstruct(srcIpAddr, destIpAddrPtr) \
{ \
    (destIpAddrPtr)->s6LAddr[0] = (srcIpAddr).s6LAddr[0]; \
    (destIpAddrPtr)->s6LAddr[1] = (srcIpAddr).s6LAddr[1]; \
    (destIpAddrPtr)->s6LAddr[2] = (srcIpAddr).s6LAddr[2]; \
    (destIpAddrPtr)->s6LAddr[3] = (srcIpAddr).s6LAddr[3]; \
}

/* 
 * tm_6_ip_copy_srcstruct
 *  Description: copy one IPv6 address to another
 *  Parameters:
 *       (IN) tt6IpAddress srcIpAddr:
 *           IPv6 address to copy from
 *       (OUT) tt6IpAddress destIpAddr:
 *           IPv6 address to copy to
 */
#define tm_6_ip_copy_structs(srcIpAddr, destIpAddr) \
{ \
    (destIpAddr).s6LAddr[0] = (srcIpAddr).s6LAddr[0]; \
    (destIpAddr).s6LAddr[1] = (srcIpAddr).s6LAddr[1]; \
    (destIpAddr).s6LAddr[2] = (srcIpAddr).s6LAddr[2]; \
    (destIpAddr).s6LAddr[3] = (srcIpAddr).s6LAddr[3]; \
}

/* tm_6_ip_hdr_in6_addr
   Description: Casts an IPv6 address in a network header (tt6IpAddrArr)
        to an in6_addr structure.
   Parameters:
        (IN) tt6IpAddrArr ipHdrAddr:
            IP address to cast.
*/
#define tm_6_ip_hdr_in6_addr(ipHdrAddr) \
    (*(tt6IpAddressPtr)&(ipHdrAddr))

/* swap the two ipv6 addresses, both referred as pointers */
#define tm_6_ip_swap(ipAddrAPtr, ipAddrBPtr)                        \
    tm_ip_swap((ipAddrAPtr)->s6LAddr[0], (ipAddrBPtr)->s6LAddr[0])  \
    tm_ip_swap((ipAddrAPtr)->s6LAddr[1], (ipAddrBPtr)->s6LAddr[1])  \
    tm_ip_swap((ipAddrAPtr)->s6LAddr[2], (ipAddrBPtr)->s6LAddr[2])  \
    tm_ip_swap((ipAddrAPtr)->s6LAddr[3], (ipAddrBPtr)->s6LAddr[3])

/* tm_6_is_addr_sol_node
   Description: test IPv6 address to see if it is a solicited-node
        multicast address.
   Parameters:
        (IN) tt6IpAddressPtr ipAddrPtr:
            pointer to the IPv6 address to test
*/
#ifdef TM_LITTLE_ENDIAN
#define tm_6_is_addr_sol_node(ipAddrPtr) \
    (((ipAddrPtr)->s6LAddr[0] == TM_UL(0x000002FF)) \
     && ((ipAddrPtr)->s6LAddr[1] == TM_UL(0)) \
     && ((ipAddrPtr)->s6LAddr[2] == TM_UL(0x01000000)) \
     && (((ipAddrPtr)->s6LAddr[3] & TM_UL(0x000000FF)) \
         == TM_UL(0x000000FF)))
#else /* TM_BIG_ENDIAN */
#define tm_6_is_addr_sol_node(ipAddrPtr) \
    (((ipAddrPtr)->s6LAddr[0] == TM_UL(0xFF020000)) \
     && ((ipAddrPtr)->s6LAddr[1] == TM_UL(0)) \
     && ((ipAddrPtr)->s6LAddr[2] == TM_UL(0x00000001)) \
     && (((ipAddrPtr)->s6LAddr[3] & TM_UL(0xFF000000)) \
         == TM_UL(0xFF000000)))
#endif /* TM_BIG_ENDIAN */

/* tm_6_is_addr_same_sol_node
   Description: test two unicast IPv6 address to see if they map to
        the same solicited node multicast address.
   Parameters:
        (IN) tt6IpAddressPtr ipAddrPtr1:
            pointer to the first IPv6 address to test
        (IN) tt6IpAddressPtr ipAddrPtr2:
            pointer to the second IPv6 address to test
*/
#ifdef TM_LITTLE_ENDIAN
#define tm_6_is_addr_same_sol_node(ipAddrPtr1, ipAddrPtr2) \
((((ipAddrPtr1)->s6LAddr[3] ^ (ipAddrPtr2)->s6LAddr[3]) \
        & TM_UL(0xFFFFFF00)) == TM_UL(0))
#else /* TM_BIG_ENDIAN */
#define tm_6_is_addr_same_sol_node(ipAddrPtr1, ipAddrPtr2) \
((((ipAddrPtr1)->s6LAddr[3] ^ (ipAddrPtr2)->s6LAddr[3]) \
        & TM_UL(0x00FFFFFF)) == TM_UL(0))
#endif /* TM_BIG_ENDIAN */

/* tm_6_is_addr_all_nodes
   Description: test IPv6 address to see if it is the link-local scope
        all nodes multicast address.
   Parameters:
        (IN) tt6IpAddressPtr ipAddrPtr:
            pointer to the IPv6 address to test
*/
#ifdef TM_LITTLE_ENDIAN
#define tm_6_is_addr_all_nodes(ipAddrPtr) \
    (((ipAddrPtr)->s6LAddr[0] == TM_UL(0x000002FF)) \
     && ((ipAddrPtr)->s6LAddr[1] == TM_UL(0)) \
     && ((ipAddrPtr)->s6LAddr[2] == TM_UL(0)) \
     && (((ipAddrPtr)->s6LAddr[3] == TM_UL(0x01000000))))
#define tm_6_is_addr_all_routers(ipAddrPtr) \
    (((ipAddrPtr)->s6LAddr[0] == TM_UL(0x000002FF)) \
     && ((ipAddrPtr)->s6LAddr[1] == TM_UL(0)) \
     && ((ipAddrPtr)->s6LAddr[2] == TM_UL(0)) \
     && (((ipAddrPtr)->s6LAddr[3] == TM_UL(0x02000000))))
#define tm_6_is_addr_all_nodes_routers(ipAddrPtr) \
    (((ipAddrPtr)->s6LAddr[0] == TM_UL(0x000002FF)) \
     && ((ipAddrPtr)->s6LAddr[1] == TM_UL(0)) \
     && ((ipAddrPtr)->s6LAddr[2] == TM_UL(0)) \
     && (    ((ipAddrPtr)->s6LAddr[3] == TM_UL(0x01000000)) \
          || ((ipAddrPtr)->s6LAddr[3] == TM_UL(0x02000000)) ) )
#else /* TM_BIG_ENDIAN */
#define tm_6_is_addr_all_nodes(ipAddrPtr) \
    (((ipAddrPtr)->s6LAddr[0] == TM_UL(0xFF020000)) \
     && ((ipAddrPtr)->s6LAddr[1] == TM_UL(0)) \
     && ((ipAddrPtr)->s6LAddr[2] == TM_UL(0)) \
     && (((ipAddrPtr)->s6LAddr[3] == TM_UL(0x00000001))))
#define tm_6_is_addr_all_routers(ipAddrPtr) \
    (((ipAddrPtr)->s6LAddr[0] == TM_UL(0xFF020000)) \
     && ((ipAddrPtr)->s6LAddr[1] == TM_UL(0)) \
     && ((ipAddrPtr)->s6LAddr[2] == TM_UL(0)) \
     && (((ipAddrPtr)->s6LAddr[3] == TM_UL(0x00000002))))
#define tm_6_is_addr_all_nodes_routers(ipAddrPtr) \
    (((ipAddrPtr)->s6LAddr[0] == TM_UL(0xFF020000)) \
     && ((ipAddrPtr)->s6LAddr[1] == TM_UL(0)) \
     && ((ipAddrPtr)->s6LAddr[2] == TM_UL(0)) \
     && (    ((ipAddrPtr)->s6LAddr[3] == TM_UL(0x00000001)) \
          || ((ipAddrPtr)->s6LAddr[3] == TM_UL(0x00000002)) ) )
#endif /* TM_BIG_ENDIAN */

/* tm_6_is_addr_local_mcast
   Description: test IPv6 address to see if it is the link-local scope
        multicast address.
   Parameters:
        (IN) tt6IpAddressPtr ipAddrPtr:
            pointer to the IPv6 address to test
*/
#ifdef TM_LITTLE_ENDIAN
#define tm_6_is_addr_local_mcast(ipAddrPtr) \
    ((ipAddrPtr)->s6SAddr[0] == TM_UL(0x02FF))
#else /* TM_BIG_ENDIAN */
#define tm_6_is_addr_local_mcast(ipAddrPtr) \
    ((ipAddrPtr)->s6SAddr[0] == TM_UL(0xFF02))
#endif /* TM_BIG_ENDIAN */

/* tm_6_is_addr_ipv6_native
   Description: test IPv6 address to see if it is a global scope
        IPv6-native address.
   Parameters:
        (IN) tt6IpAddressPtr ipAddrPtr:
            pointer to the IPv6 address to test
*/
#ifdef TM_LITTLE_ENDIAN
#define tm_6_is_addr_ipv6_native(ipAddrPtr) \
    (((ipAddrPtr)->s6LAddr[0] & TM_UL(0x000000E0)) == TM_UL(0x00000020))
#else /* TM_BIG_ENDIAN */
#define tm_6_is_addr_ipv6_native(ipAddrPtr) \
    (((ipAddrPtr)->s6LAddr[0] & TM_UL(0xE0000000)) == TM_UL(0x20000000))
#endif /* TM_BIG_ENDIAN */

/* tm_6_is_addr_6to4
   Description: test IPv6 address to see if it is a global scope
        6to4 IPv6 address.
   Parameters:
        (IN) tt6IpAddressPtr ipAddrPtr:
            pointer to the IPv6 address to test
*/
#ifdef TM_LITTLE_ENDIAN
#define tm_6_is_addr_6to4(ipAddrPtr) \
    (((ipAddrPtr)->s6LAddr[0] & TM_UL(0x0000FFFF)) == TM_UL(0x00000220))
#else /* TM_BIG_ENDIAN */
#define tm_6_is_addr_6to4(ipAddrPtr) \
    (((ipAddrPtr)->s6LAddr[0] & TM_UL(0xFFFF0000)) == TM_UL(0x20020000))
#endif /* TM_BIG_ENDIAN */

/* tm_6_ip_flow_info
   Description: Extract the 28-bit flow information, which consists of the
        Traffic Class and the Flow Label.
   Parameters:
        (IN) tt32Bit verFlowInfo:
            the first 32-bits of the IPv6 packet header in network byte order
*/
#ifdef TM_LITTLE_ENDIAN
#define tm_6_ip_flow_info(verFlowInfo) \
    (tt32Bit)(ntohl(verFlowInfo) & TM_UL( 0x0FFFFFFF))
#else /* TM_BIG_ENDIAN */
#define tm_6_ip_flow_info(verFlowInfo) \
    (tt32Bit)((verFlowInfo) & TM_UL( 0x0FFFFFFF))
#endif /* TM_BIG_ENDIAN */

/* tm_6_ip_traffic_class
   Description: Extract the 8-bit Traffic Class, the format of which is
        specified by [RFC2474].
   Parameters:
        (IN) tt32Bit verFlowInfo:
            the first 32-bits of the IPv6 packet header in network byte order
*/
#ifdef TM_LITTLE_ENDIAN
#define tm_6_ip_traffic_class(verFlowInfo) \
    (tt32Bit)((ntohs(*((tt16BitPtr) &(verFlowInfo))) & 0x0FF0U) >> 4)
#else /* TM_BIG_ENDIAN */
#define tm_6_ip_traffic_class(verFlowInfo) \
    (tt32Bit)((*((tt16BitPtr) &(verFlowInfo)) & 0x0FF0U) >> 4)
#endif /* TM_BIG_ENDIAN */

/* tm_6_ip_flow_label
   Description: Extract the 20-bit Flow Label.
   Parameters:
        (IN) tt32Bit verFlowInfo:
            the first 32-bits of the IPv6 packet header in network byte order
*/
#ifdef TM_LITTLE_ENDIAN
#define tm_6_ip_flow_label(verFlowInfo) \
    (tt32Bit)(ntohl(verFlowInfo) & TM_UL( 0x000FFFFF))
#else /* TM_BIG_ENDIAN */
#define tm_6_ip_flow_label(verFlowInfo) \
    (tt32Bit)((verFlowInfo) & TM_UL( 0x000FFFFF))
#endif /* TM_BIG_ENDIAN */

/* tm_6_ip_version
   Description: IPv6 analogue of existing macro tm_ip_version,
        should return 6
   Parameters:
        (IN) tt8Bit verClass:
            The first byte of the IPv6 packet header.
*/
#define tm_6_ip_version(verClass) (tt8Bit)(((tt8Bit) (verClass)) >> 4)

/* tm_6_exth_frag_offset
   Description: extract the fragment offset from
        tt6FragmentExtHdr.fra6OffsetFlags.
   Parameters:
        (IN) tt16Bit offsetFlag:
            refer to tt6FragmentExtHdr.fra6OffsetFlags.
*/
#ifdef TM_LITTLE_ENDIAN
#define tm_6_exth_frag_offset(offsetFlag) \
    ((((tt16Bit) ntohs(offsetFlag)) & 0xFFF8U) >> 3)
#else /* TM_BIG_ENDIAN */
#define tm_6_exth_frag_offset(offsetFlag) \
    ((((tt16Bit) (offsetFlag)) & 0xFFF8U) >> 3)
#endif /* TM_BIG_ENDIAN */

/* tm_6_ip_zero_arr
   Description: Zero IPv6 pseudo header zero array
   Parameters:
        (IN) h6ZeroArr
            start of IPv6 Zero array.
*/
#ifdef TM_DSP
#ifdef TM_16BIT_DSP
#define tm_6_zero_arr(h6ZeroArr)    \
        h6ZeroArr[0] = 0;           \
        h6ZeroArr[1] = 0;
#else /* !TM_16BIT_DSP */
#define tm_6_zero_arr(h6ZeroArr)    \
        h6ZeroArr[0] = 0;
#endif /* !TM_16BIT_DSP */
#else /* !TM_DSP */
#define tm_6_zero_arr(h6ZeroArr)    \
        h6ZeroArr[0] = 0;           \
        h6ZeroArr[1] = 0;           \
        h6ZeroArr[2] = 0;
#endif /* !TM_DSP */

/* tm_6_copy_zero_arr
   Description: Copy IPv6 pseudo header zero array
   Parameters:
        (IN) fromh6ZeroArr
            start of IPv6 Zero array to copy from.
             toh6ZeroArr
            start of IPv6 Zero array to copy to.
*/
#ifdef TM_DSP
#ifdef TM_16BIT_DSP
#define tm_6_copy_zero_arr(fromh6ZeroArr, toh6ZeroArr)  \
        toh6ZeroArr[0] = fromh6ZeroArr[0];  \
        toh6ZeroArr[1] = fromh6ZeroArr[1];
#else /* !TM_16BIT_DSP */
#define tm_6_copy_zero_arr(fromh6ZeroArr, toh6ZeroArr)  \
        toh6ZeroArr[0] = fromh6ZeroArr[0];
#endif /* !TM_16BIT_DSP */
#else /* !TM_DSP */
#define tm_6_copy_zero_arr(fromh6ZeroArr, toh6ZeroArr)  \
        toh6ZeroArr[0] = fromh6ZeroArr[0];              \
        toh6ZeroArr[1] = fromh6ZeroArr[1];              \
        toh6ZeroArr[2] = fromh6ZeroArr[2];
#endif /* !TM_DSP */

/* tm_6_addr_to_sol_node
   Description: convert an IPv6 address into a solicited-node multicast
        address.
   Parameters:
        (IN) tt6IpAddressPtr inIpAddrPtr:
            pointer to the IPv6 address to convert from
        (OUT) tt6IpAddressPtr solNodeIpAddrPtr:
            pointer to the IPv6 solicited-node multicast address to convert to
*/
#ifdef TM_LITTLE_ENDIAN
#define tm_6_addr_to_sol_node(inIpAddrPtr, solNodeIpAddrPtr) \
{ \
    (solNodeIpAddrPtr)->s6LAddr[0] = TM_UL(0x000002FF); \
    (solNodeIpAddrPtr)->s6LAddr[1] = TM_UL(0); \
    (solNodeIpAddrPtr)->s6LAddr[2] = TM_UL(0x01000000); \
    (solNodeIpAddrPtr)->s6LAddr[3] = \
        (inIpAddrPtr)->s6LAddr[3] | TM_UL(0x000000FF); \
}
#else /* TM_BIG_ENDIAN */
#define tm_6_addr_to_sol_node(inIpAddrPtr, solNodeIpAddrPtr) \
{ \
    (solNodeIpAddrPtr)->s6LAddr[0] = TM_UL(0xFF020000); \
    (solNodeIpAddrPtr)->s6LAddr[1] = TM_UL(0); \
    (solNodeIpAddrPtr)->s6LAddr[2] = TM_UL(0x00000001); \
    (solNodeIpAddrPtr)->s6LAddr[3] = \
        (inIpAddrPtr)->s6LAddr[3] | TM_UL(0xFF000000); \
}
#endif /* TM_BIG_ENDIAN */

#ifdef TM_LITTLE_ENDIAN
#define TM_IPV4_3_MAP   TM_UL(0xFFFF0000)
#else /* TM_BIG_ENDIAN */
#define TM_IPV4_3_MAP   TM_UL(0x0000FFFF)
#endif /* TM_BIG_ENDIAN */

/*
 * tm_6_addr_to_ipv4_mapped
 * Description: convert an IPv4 address into an IPv4-mapped IPv6 address
 * Parameters:
 *      (IN) tt32Bit inIpv4Addr:
 *          pointer to the IPv4 address to convert from
 *      (OUT) tt6IpAddressPtr outIpv6AddrPtr:
 *          pointer to the IPv4-mapped IPv6 address to convert to
 */
#define tm_6_addr_to_ipv4_mapped(inIpv4Addr, outIpv6AddrPtr)    \
{                                                               \
    (outIpv6AddrPtr)->s6LAddr[0] = TM_UL(0);                    \
    (outIpv6AddrPtr)->s6LAddr[1] = TM_UL(0);                    \
    (outIpv6AddrPtr)->s6LAddr[2] = TM_IPV4_3_MAP;               \
    (outIpv6AddrPtr)->s6LAddr[3] = (inIpv4Addr);                \
}

#define tm_6_addr_add_ipv4_map_field(outIpv6AddrPtr)    \
    (outIpv6AddrPtr)->s6LAddr[2] = TM_IPV4_3_MAP;

/* 
 * tm_6_addr_ipv4_mapped_zero
 *  Description: Check that an IPv4-mapped IPv6 address is a zero IPV4 address
 *  Parameters:
 *       (IN) tt32Bit inIpv6AddrPtr:
 *           pointer to the IPV4-mapped IPV6 address.
 */
#define tm_6_addr_ipv4_mapped_zero(inIpv6AddrPtr)               \
    (    ( (inIpv6AddrPtr)->s6LAddr[0] == TM_UL(0) )            \
      && ( (inIpv6AddrPtr)->s6LAddr[1] == TM_UL(0) )            \
      && ( (inIpv6AddrPtr)->s6LAddr[2] == TM_IPV4_3_MAP )       \
      && ( (inIpv6AddrPtr)->s6LAddr[3] == TM_UL(0) ) )          \

/* 
 * tm_6_addr_is_ipv4_mapped
 *  Description: Check that an IPV6 address is an IPv4-mapped IPv6
 *               address.
 *  Parameters:
 *       (IN) tt32Bit inIpv6AddrPtr:
 *           pointer to the IPV6 address.
 */
#ifdef TM_4_USE_SCOPE_ID
#define tm_6_addr_is_ipv4_mapped(inIpv6AddrPtr)                 \
    (    ( (inIpv6AddrPtr)->s6LAddr[0] == TM_UL(0) )            \
      && ( (inIpv6AddrPtr)->s6LAddr[1] == TM_UL(0) )            \
      && ( (inIpv6AddrPtr)->s6SAddr[5] == (tt16Bit) 0xFFFF ) )
#else /* ! TM_4_USE_SCOPE_ID */
#define tm_6_addr_is_ipv4_mapped(inIpv6AddrPtr)                 \
    (    ( (inIpv6AddrPtr)->s6LAddr[0] == TM_UL(0) )            \
      && ( (inIpv6AddrPtr)->s6LAddr[1] == TM_UL(0) )            \
      && ( (inIpv6AddrPtr)->s6LAddr[2] == TM_IPV4_3_MAP ) )
#endif /* ! TM_4_USE_SCOPE_ID */

/* 
 * tm_6_addr_ipv4_mapped_loopback
 *  Description: Check that an IPv4-mapped IPv6 address is a loopback 
 *  IPV4 address.
 *  Parameters:
 *       (IN) tt32Bit inIpv6AddrPtr:
 *           pointer to the IPV4-mapped IPV6 address.
 */
#define tm_6_addr_ipv4_mapped_loopback(inIpv6AddrPtr)           \
    (    ( (inIpv6AddrPtr)->s6LAddr[0] == TM_UL(0) )            \
      && ( (inIpv6AddrPtr)->s6LAddr[1] == TM_UL(0) )            \
      && ( (inIpv6AddrPtr)->s6LAddr[2] == TM_IPV4_3_MAP )       \
      && ( tm_ip_is_loop_back(inIpv6AddrPtr->s6LAddr[3]) ) )    \

/* 
 * tm_6_netmask_to_ipv4_mapped
 *  Description: convert an IPv4 netmask into an IPv4-mapped IPv6 address 
 *  netmask
 *  Parameters:
 *       (IN) tt32Bit inIpv4Addr:
 *           pointer to the IPv4 netmask to convert from
 *       (OUT) tt6IpAddressPtr outIpv6AddrPtr:
 *           pointer to the IPv4-mapped IPv6 address netmask to convert to
 */
#define tm_6_netmask_to_ipv4_mapped(inIpv4Addr, outIpv6AddrPtr) \
{                                                               \
    (outIpv6AddrPtr)->s6LAddr[0] = TM_IP_ALL_ONES;              \
    (outIpv6AddrPtr)->s6LAddr[1] = TM_IP_ALL_ONES;              \
    (outIpv6AddrPtr)->s6LAddr[2] = TM_IP_ALL_ONES;              \
    (outIpv6AddrPtr)->s6LAddr[3] = (inIpv4Addr);                \
}

/*
 * tm_6_addr_to_ipv4_compat
 * Description: convert an IPv4 address into an IPv4-compatible IPv6 address
 * Parameters:
 *      (IN) tt32Bit inIpv4Addr:
 *          pointer to the IPv4 address to convert from
 *      (OUT) tt6IpAddressPtr outIpv6AddrPtr:
 *          pointer to the IPv4-compatible IPv6 address to convert to
 */
#define tm_6_addr_to_ipv4_compat(inIpv4Addr, outIpv6AddrPtr)    \
{                                                               \
    (outIpv6AddrPtr)->s6LAddr[0] = TM_UL(0);                    \
    (outIpv6AddrPtr)->s6LAddr[1] = TM_UL(0);                    \
    (outIpv6AddrPtr)->s6LAddr[2] = TM_UL(0);                    \
    (outIpv6AddrPtr)->s6LAddr[3] = (inIpv4Addr);                \
}

/* 
 * tm_6_addr_ipv4_compat_zero
 *  Description: Check that an IPv4-compatible IPv6 address is a zero IPV4
 *               address
 *  Parameters:
 *       (IN) tt32Bit inIpv6AddrPtr:
 *           pointer to the IPV4-mapped IPV6 address.
 */
#define tm_6_addr_ipv4_compat_zero(inIpv6AddrPtr)               \
    (    ( (inIpv6AddrPtr)->s6LAddr[0] == TM_UL(0) )            \
      && ( (inIpv6AddrPtr)->s6LAddr[1] == TM_UL(0) )            \
      && ( (inIpv6AddrPtr)->s6LAddr[2] == TM_UL(0) )            \
      && ( (inIpv6AddrPtr)->s6LAddr[3] == TM_UL(0) ) )          \

/* 
 * tm_6_addr_is_ipv4_compat
 *  Description: Check that an IPV6 address is an IPv4-compatible IPv6
 *               address.
 *  Parameters:
 *       (IN) tt32Bit inIpv6AddrPtr:
 *           pointer to the IPV6 address.
 */
#define tm_6_addr_is_ipv4_compat(inIpv6AddrPtr)                 \
    (    ( (inIpv6AddrPtr)->s6LAddr[0] == TM_UL(0) )            \
      && ( (inIpv6AddrPtr)->s6LAddr[1] == TM_UL(0) )            \
      && ( (inIpv6AddrPtr)->s6LAddr[2] == TM_UL(0) ) )

/* 
 * tm_6_addr_ipv4_compat_loopback
 *  Description: Check that an IPv4-compatible IPv6 address is a loopback 
 *  IPV4 address.
 *  Parameters:
 *       (IN) tt32Bit inIpv6AddrPtr:
 *           pointer to the IPV4-mapped IPV6 address.
 */
#define tm_6_addr_ipv4_compat_loopback(inIpv6AddrPtr)           \
    (    ( (inIpv6AddrPtr)->s6LAddr[0] == TM_UL(0) )            \
      && ( (inIpv6AddrPtr)->s6LAddr[1] == TM_UL(0) )            \
      && ( (inIpv6AddrPtr)->s6LAddr[2] == TM_UL(0) )            \
      && ( tm_ip_is_loop_back(inIpv6AddrPtr->s6LAddr[3]) ) )    \

/* 
 * tm_6_netmask_to_ipv4_compat
 *  Description: convert an IPv4 netmask into an IPv4-compatible IPv6 address 
 *  netmask
 *  Parameters:
 *       (IN) tt32Bit inIpv4Addr:
 *           pointer to the IPv4 netmask to convert from
 *       (OUT) tt6IpAddressPtr outIpv6AddrPtr:
 *           pointer to the IPv4-compatible IPv6 address netmask to convert to
 */
#define tm_6_netmask_to_ipv4_compat(inIpv4Addr, outIpv6AddrPtr) \
{                                                               \
    (outIpv6AddrPtr)->s6LAddr[0] = TM_IP_ALL_ONES;              \
    (outIpv6AddrPtr)->s6LAddr[1] = TM_IP_ALL_ONES;              \
    (outIpv6AddrPtr)->s6LAddr[2] = TM_IP_ALL_ONES;              \
    (outIpv6AddrPtr)->s6LAddr[3] = (inIpv4Addr);                \
}

/*
 * Prefix length for the IPv4 default gateway in dual mode.
 */
#define TM_6_DEF_4GWAY_PREFIX_LENGTH 80

/* 
 * Increase in prefix length when converting from an IPV4 address, to an 
 * IPv4-mapped IPV6 address
 */
#define TM_6_IPV6_PREFIXLEN_INCREASE     96

/* 
 * tm_6_prefixlen_to_ipv4_mapped
 *  Description: convert an IPv4 address prefix length into an IPv4-mapped 
 *  IPv6 address prefix length.
 *  Parameters:
 *       (IN) prefixLength:
 *          integer containing the IPV4 address prefix length 
 */
#define tm_6_prefixlen_to_ipv4_mapped(prefixLength)                         \
    ((prefixLength) + TM_6_IPV6_PREFIXLEN_INCREASE)

/*
 * Copy an IPv4-mapped IPV6 address into an IPV4 address
 */
#define tm_6_ip_mapped_copy_6to4(ipv6AddrPtr, ipAddress)                     \
    ipAddress = (ipv6AddrPtr)->s6LAddr[3]

/*
 * Get the IPv4 IP address from an IPv4-mapped IPV6 address
 */
#define tm_6_ip_mapped_6to4(ipv6AddrPtr)                                    \
    (ipv6AddrPtr)->s6LAddr[3]

/* device Ip address */
#define tm_6_ip_dev_addr(devPtr, mhomeIndex) \
        (devPtr)->dev6IpAddrArray[mhomeIndex]

/* Copies a link layer address with an arbitrary length */
#define tm_6_phys_addr_copy( srcAddrPtr, destAddrPtr, physAddrLen, i)   \
{                                                                       \
    for (i=0;i<(physAddrLen);i++)                                       \
    {                                                                   \
        ((tt8BitPtr)(destAddrPtr))[i] =                                 \
            ((tt8BitPtr)(srcAddrPtr))[i];                               \
    }                                                                   \
}

/* 
 *
 * tm_6_phys_addr_match
 *  Description: Compares two link layer address; result stored in 'isMatch'
 *
 *  Parameters:
 *       (IN) tt8BitPtr addr1Ptr:
 *              Pointer to first physical address to check
 *       (IN) tt8BitPtr addr2Ptr:
 *              Pointer to second physical address to check
 *       (IN) int       addrLen
                Length of physical addresses
 *       (OUT) int      isMatch
 *          non-zero if the physical addresses match, else set to 0
 */
#define tm_6_phys_addr_match(addr1Ptr, addr2Ptr, addrLen, isMatch)      \
{                                                                       \
    tm_phys_addr_match(addr1Ptr, addr2Ptr, addrLen, isMatch);           \
    if (isMatch != addrLen)                                             \
    {                                                                   \
        isMatch = 0;                                                    \
    }                                                                   \
}

/* tm_6_embed_scope_id
   Description: embed scope ID in local scope address, which is the
        format used internally.
   Parameters:
        (IN) tt6IpAddressPtr ipAddrPtr:
            pointer to the local scope IPv6 address to embed the scope ID in
        (IN) tt32Bit scopeId:
            the scope ID to embed
*/
#ifdef TM_SINGLE_INTERFACE
#define tm_6_embed_scope_id(ipAddrPtr, scopeId)
#else /* ! TM_SINGLE_INTERFACE */
#define tm_6_embed_scope_id(ipAddrPtr, scopeId)                             \
{                                                                           \
    (ipAddrPtr)->s6LAddr[1] = (tt32Bit)scopeId;                             \
}
#endif /* ! TM_SINGLE_INTERFACE */

/* tm_6_get_embedded_scope_id
   Description: get embedded scope ID from local scope address
   Parameters:
        (IN) tt6IpAddressPtr ipAddrPtr:
            pointer to the local scope IPv6 address that has the embedded
            scope ID.
        (OUT) tt32Bit scopeId:
            Set to 0 if the address that ipAddrPtr points to is not
            local-scope, else set to the scope ID embedded in the address.
*/
#ifdef TM_SINGLE_INTERFACE
#define tm_6_get_embedded_scope_id(ipAddrPtr, scopeId)          \
{                                                               \
    scopeId = (tt32Bit) 0;                                      \
}
#else /* ! TM_SINGLE_INTERFACE */
#define tm_6_get_embedded_scope_id(ipAddrPtr, scopeId)          \
{                                                               \
    scopeId = (tt32Bit) 0;                                      \
    if (IN6_IS_ADDR_LINKLOCAL(ipAddrPtr))                       \
    {                                                           \
        scopeId = (ipAddrPtr)->s6LAddr[1];                      \
    }                                                           \
}
#endif /* ! TM_SINGLE_INTERFACE */

/*
 * tm_6_addr_is_local
 *  Description:  Check whether a specified IP address is link local, or
 *  site local.
 *  Parameters:
 *       (IN) tt6IpAddressPtr ipAddrPtr:
 *           pointer to the IPv6 address
 */
#ifdef TM_DSP
#define tm_6_addr_is_local(ipAddrPtr)                                         \
  (    ((((ipAddrPtr)->ip6Addr.ip6U32[0] >> 24) & 0xff) == (ttUser8Bit) 0xfe) \
    && (((((ipAddrPtr)->ip6Addr.ip6U32[0] >> 16) & 0xff) & (ttUser8Bit) 0x80)  \
       != (ttUser8Bit) 0) )
#else /* !TM_DSP */
#define tm_6_addr_is_local(ipAddrPtr)                                       \
  (    ((ipAddrPtr)->s6_addr[0] == (ttUser8Bit) 0xfe)                       \
    && (((ipAddrPtr)->s6_addr[1] & (ttUser8Bit) 0x80) != (ttUser8Bit) 0) )
#endif /* TM_DSP */

/* tm_6_dev_scope_addr
   Description: Scope an IPv6 address to a specified interface.
   Parameters:
        (IN) tt6IpAddressPtr ipAddrPtr:
            pointer to the IPv6 address
        (IN) ttDeviceEntryPtr devPtr:
            pointer to the interface to scope the IPv6 address to
*/
#ifdef TM_SINGLE_INTERFACE
#define tm_6_dev_scope_addr(ipAddrPtr, devPtr)
#else /* ! TM_SINGLE_INTERFACE */
#define tm_6_dev_scope_addr(ipAddrPtr, devPtr)                              \
  if (IN6_IS_ADDR_LINKLOCAL(ipAddrPtr))                                     \
  {                                                                         \
    tm_6_embed_scope_id(ipAddrPtr, devPtr->devIndex);                       \
  }
#endif /* ! TM_SINGLE_INTERFACE */

/* 
 * tm_6_addr_add_scope
 *  Description: Add the scope to an IPv6 address if it is a link local or
 *  site local address.
 *  Parameters:
 *       (IN) tt6IpAddressPtr ipAddrPtr:
 *           pointer to the IPv6 address
 *       (IN) scopeId:
 *           User passed scope Id of the IPv6 address
 */
#ifdef TM_SINGLE_INTERFACE
#define tm_6_addr_add_scope(ipAddrPtr, scopeId)
#else /* ! TM_SINGLE_INTERFACE */
#define tm_6_addr_add_scope(ipAddrPtr, scopeId)                             \
  if (IN6_IS_ADDR_LINKLOCAL(ipAddrPtr))                                     \
  {                                                                         \
    tm_6_embed_scope_id(ipAddrPtr, scopeId);                                \
  }
#endif /* ! TM_SINGLE_INTERFACE */

/* 
 * tm_6_clear_scope_id
 *  Description: Remove the dev Id/Site Id from a link local, or site local
 *       IPv6 address associated with a given interface, i.e. undo what the
 *       tm_6_embed_scope_id macro did.
 *  Parameters:
 *       (IN) tt6IpAddressPtr ipAddrPtr:
 *           pointer to the IPv6 address
 */
#ifdef TM_SINGLE_INTERFACE
#define tm_6_clear_scope_id(ipAddrPtr)
#else /* ! TM_SINGLE_INTERFACE */
#define tm_6_clear_scope_id(ipAddrPtr)                                      \
{                                                                           \
    (ipAddrPtr)->s6LAddr[1] = TM_UL(0);                                     \
}
#endif /* ! TM_SINGLE_INTERFACE */

/* 
 * tm_6_host_clear_scope_id
 *  Description: 
 *       Remove the dev Id/Site Id from a link local, or site local
 *       IPv6 address associated with a given interface, i.e. undo what the
 *       tm_6_embed_scope_id macro did, but with the IPv6 address in
 *       host byte order.
 *       This macro has the same purpose as the tm_6_clear_scope_id macro,
 *       but the IPv6 address is in host byte order here, whereas it is in
 *       network byte order in the tm_6_clear_scope_id macro.
 *  Parameters:
 *       (IN) tt6IpAddressPtr ipAddrPtr:
 *           pointer to the IPv6 address in host byte order.
 */
#ifdef TM_SINGLE_INTERFACE
#define tm_6_host_clear_scope_id(ipAddrPtr)
#else /* ! TM_SINGLE_INTERFACE */
#define tm_6_host_clear_scope_id(ipAddrPtr)                                 \
{                                                                           \
    (ipAddrPtr)->s6LAddr[1] = TM_UL(0);                                     \
}
#endif /* ! TM_SINGLE_INTERFACE */

/* 
 * tm_6_dev_unscope_addr
 *  Description: Remove the dev Id/Site Id from a link local, or site local
 *       IPv6 address associated with a given interface, i.e. undo what the
 *       tm_6_dev_scope_addr macro did.
 *  Parameters:
 *       (IN) tt6IpAddressPtr ipAddrPtr:
 *           pointer to the IPv6 address
 */
#ifdef TM_SINGLE_INTERFACE
#define tm_6_dev_unscope_addr(ipAddrPtr)
#else /* ! TM_SINGLE_INTERFACE */
#define tm_6_dev_unscope_addr(ipAddrPtr)                                    \
  if (IN6_IS_ADDR_LINKLOCAL(ipAddrPtr))                                     \
  {                                                                         \
    tm_6_clear_scope_id(ipAddrPtr);                                         \
  }
#endif /* ! TM_SINGLE_INTERFACE */

/* tm_6_link_local_cached
   Description: Check if an interface has an IPv6 link local address cached.
   Parameters:
        (IN) ttDeviceEntryPtr devPtr:
            pointer to the interface to check
*/
#define tm_6_link_local_cached(devPtr)                                      \
    (devPtr->dev6AddrCacheFlags & TM_6_DEV_LINK_LOCAL_FLAG)

/* tm_6_dev_is_enabled
   Description: Check if an interface has IPv6 enabled.
   Parameters:
        (IN) ttDeviceEntryPtr devPtr:
            pointer to the interface to check
*/
#define tm_6_dev_is_enabled(devPtr)                                         \
    (   (tm_6_link_local_cached(devPtr))                                    \
     && (devPtr->dev6IpAddrFlagsArray[                                      \
             devPtr->dev6LinkLocalMhomeIndex] & TM_6_DEV_IP_CONFIG) )

#ifdef TM_6_USE_MLD
/* Null pointers */
#define TM_6_SOC_MLD_INFO_NULL_PTR    (tt6SockMldInfoPtr)0
#ifdef  TM_6_USE_MLDV2
#define TM_6_MLD_MCADDR_REC_NULL_PTR  (tt6MldMcAddrRecPtr)0
#define TM_6_SRC_ADDR_ENTRY_NULL_PTR  (tt6SourceAddrEntryPtr)0
#define TM_6_LIST_OF_SRCADDR_NULL_PTR (tt6ListOfSourceAddrPtr)0
#endif  /* TM_6_USE_MLDV2 */
#endif /* TM_6_USE_MLD */
#define TM_6_IPADDR_NULL_PTR          (tt6IpAddressPtr)0
#define TM_6_LL_MCAST_FUNC_NULL_PTR   (tt6LnkMcastFuncPtr)0
#define TM_6_LL_RES_INC_FUNC_NULL_PTR (tt6LnkResIncomingFuncPtr)0
#define TM_6_MLD_ENTRY_NULL_PTR       (tt6MldEntryPtr)0
#define TM_6_DAD_ENTRY_NULL_PTR       (tt6DadEntryPtr)0
#define TM_6_MN_HA_BUL_NULL_PTR       (tt6MnHaBulEntryPtr)0
#define TM_6_MN_RR_BUL_NULL_PTR       (tt6MnRrBulEntryPtr)0
#ifdef TM_6_USE_RFC3484
#define TM_POL_TAB_ENTRY_NULL_PTR     (ttPolicyTableEntryPtr)0
#endif /* TM_6_USE_RFC3484 */

/* Typecodes for tf6SelectSourceAddress, identify the type of the destination
   address. */
#define TM_6_ADDR_TYPE_UNDEFINED        (tt8Bit)0
#define TM_6_ADDR_TYPE_LINK_LOCAL       (tt8Bit)1
#define TM_6_ADDR_TYPE_SITE_LOCAL       (tt8Bit)2
#define TM_6_ADDR_TYPE_IPV6_NATIVE      (tt8Bit)3
#define TM_6_ADDR_TYPE_6TO4             (tt8Bit)4
#define TM_6_ADDR_TYPE_IPV4_COMPAT      (tt8Bit)5
#define TM_6_ADDR_TYPE_LOCAL            (tt8Bit)6

/* Flags used by tf6SelectSourceAddress */
#define TM_6_EMBED_SCOPE_ID_FLAG        (tt8Bit)1

#ifdef TM_6_USE_RFC3484
/* Device Entry Pointer - dev6SelectAddressFlag values */
/* This macro is used to prefer home address over care-of address */
#define TM_PREF_HOME_ADDR               (tt8Bit) 0x00
/* This macro is used to prefer care-of address over home address */
#define TM_PREF_CARE_OF_ADDR            (tt8Bit) 0x01
/* This macro is used to prefer public address over temporary address */
#define TM_PREF_PUBLIC_ADDR             (tt8Bit) 0x00
/* This macro is used to prefer temporary address over public address */
#define TM_PREF_TEMP_ADDR               (tt8Bit) 0x02

/* The following macros are used, when calling tf6PolicyTableDisplay() API 
   as a value to the displayAllFlag parameter */
/* Flag used as argument to display all entries in policy table */
#define TM_POLTAB_DISPLAY_ALL_FLAG_SET      (tt8Bit) 0x00
/* Flag used to display matched precedence entries of policy table */
#define TM_POLTAB_DISPLAY_PRECEDENCE_FLAG   (tt8Bit) 0x01
/* Flag used to display matched label entries of policy table */
#define TM_POLTAB_DISPLAY_LABEL_FLAG        (tt8Bit) 0x02

/* This macro is used to specify the operationCode argument when calling
   the user API tf6PolicyTable() */
/* Flag to add an entry to the policy table */
#define TM_POLTAB_ADD                   (tt8Bit) 0x01
/* Flag to modify an entry in the policy table */
#define TM_POLTAB_MODIFY                (tt8Bit) 0x02
/* Flag to delete an entry from the policy table */
#define TM_POLTAB_DELETE                (tt8Bit) 0x03

/* This macro is used, when calling tf6SelectAddressPreference() API 
   as a value to the flag parameter (second argument) */
/* Flag indicating that the optionValue is related to mobile IPv6 address */
#define TM_SELECT_MOBILE_ADDRESS_FLAG   (tt8Bit) 0x01
/* Flag indicating that the optionValue is related to privacy IPv6 address */
#define TM_SELECT_PRIVACY_ADDRESS_FLAG  (tt8Bit) 0x02

/* tm_com_prefix_len
   Description: Find the common prefix length between two IPv6 addresses.
   Parameters:
        (IN) tt6IpAddressPtr ipAddrPtr1:
            pointer to the IPv6 address 1 to check
        (IN) tt6IpAddressPtr ipAddrPtr2:
            pointer to the IPv6 address 2 to check
        (OUT) int matchLen:
            length if the matching bits between the two IPv6 addresses
        (IN) int count:
            counter used to count the index
        (IN) tt8Bit mask:
            mask used to match bitwise from the most significant bit
*/
#define tm_com_prefix_len(ipAddrPtr1, ipAddrPtr2, matchLen,             \
                     count, mask)                                       \
{                                                                       \
    matchLen = 0;                                                       \
    for (count = 0; count < 8; count++)                                 \
    {                                                                   \
        mask = 0x80;                                                    \
        while (mask != 0x00)                                            \
        {                                                               \
            if (!((ipAddrPtr1->s6_addr[count] & mask) ^                 \
                 (ipAddrPtr2->s6_addr[count] & mask)) )                 \
            {                                                           \
                matchLen++;                                             \
                mask >>= 1;                                             \
            }                                                           \
            else                                                        \
            {                                                           \
                count = 8;                                              \
                break;                                                  \
            }                                                           \
        }                                                               \
    }                                                                   \
}
#endif /* TM_6_USE_RFC3484 */

#if (defined(TM_6_USE_MIP_MN) || defined(TM_6_USE_MIP_CN))
/* factor used to convert the lifetime field in the BU message into
   milliseconds */
#define TM_6_MIP_BCE_LIFETIME_UNIT_MSEC         4000

#ifndef TM_6_ND_OPT_RA_PERIOD 
/* IPv6 Neighbor Discovery option type Advertisement Interval, pending
   formal assignment by IANA. */
#define TM_6_ND_OPT_RA_PERIOD           (tt8Bit) 7
#endif /* ! TM_6_ND_OPT_RA_PERIOD */

#ifndef TM_6_ND_OPT_HA_INFO
/* IPv6 Neighbor Discovery option type Home Agent Information, pending
   formal assignment by IANA. */
#define TM_6_ND_OPT_HA_INFO             (tt8Bit) 8
#endif /* ! TM_6_ND_OPT_HA_INFO */

#ifndef TM_6_ICMP_TYPE_HAAD_REQUEST 
/* ICMPv6 Home Agent Address Discovery Request message type, pending
   formal assignment by IANA. */
#ifdef TM_6_MIP_DRAFT24
#define TM_6_ICMP_TYPE_HAAD_REQUEST     (tt8Bit) 150
#else /* ! TM_6_MIP_DRAFT24 */
/* assigned by IANA */
#define TM_6_ICMP_TYPE_HAAD_REQUEST     (tt8Bit) 144
#endif /* ! TM_6_MIP_DRAFT24 */
#endif /* ! TM_6_ICMP_TYPE_HAAD_REQUEST */

#ifndef TM_6_ICMP_TYPE_HAAD_REPLY
/* ICMPv6 Home Agent Address Discovery Reply message type, pending
   formal assignment by IANA. */
#ifdef TM_6_MIP_DRAFT24
#define TM_6_ICMP_TYPE_HAAD_REPLY       (tt8Bit) 151
#else /* ! TM_6_MIP_DRAFT24 */
/* assigned by IANA */
#define TM_6_ICMP_TYPE_HAAD_REPLY       (tt8Bit) 145
#endif /* ! TM_6_MIP_DRAFT24 */
#endif /* ! TM_6_ICMP_TYPE_HAAD_REPLY */

#ifndef TM_6_ICMP_TYPE_PREFIX_SOLICIT 
/* ICMPv6 Mobile Prefix Solicitation message type, pending
   formal assignment by IANA. */
#ifdef TM_6_MIP_DRAFT24
#define TM_6_ICMP_TYPE_PREFIX_SOLICIT   (tt8Bit) 152
#else /* ! TM_6_MIP_DRAFT24 */
/* assigned by IANA */
#define TM_6_ICMP_TYPE_PREFIX_SOLICIT   (tt8Bit) 146
#endif /* ! TM_6_MIP_DRAFT24 */
#endif /* ! TM_6_ICMP_TYPE_PREFIX_SOLICIT */

#ifndef TM_6_ICMP_TYPE_PREFIX_ADVERT 
/* ICMPv6 Mobile Prefix Advertisement message type, pending
   formal assignment by IANA. */
#ifdef TM_6_MIP_DRAFT24
#define TM_6_ICMP_TYPE_PREFIX_ADVERT    (tt8Bit) 153
#else /* ! TM_6_MIP_DRAFT24 */
/* assigned by IANA */
#define TM_6_ICMP_TYPE_PREFIX_ADVERT    (tt8Bit) 147
#endif /* ! TM_6_MIP_DRAFT24 */
#endif /* ! TM_6_ICMP_TYPE_PREFIX_ADVERT */

#ifndef TM_6_MIP_DHAAD_RETRIES
/*
 * [MIPV6_21] DHAAD_RETRIES. The number of times that the MN may retransmit 
 * the Home Agent Address Discovery Request message. 
 */
#define TM_6_MIP_DHAAD_RETRIES          4
#endif /* ! TM_6_MIP_DHAAD_RETRIES */
#ifndef TM_6_MIP_INIT_BA_TIMEOUT_MSEC
/* [MIPV6_21] INITIAL_BINDACK_TIMEOUT. Units are milliseconds. */
#define TM_6_MIP_INIT_BA_TIMEOUT_MSEC           1000
#endif /* ! TM_6_MIP_INIT_BA_TIMEOUT_MSEC */
#ifndef TM_6_MIP_INIT_DHAAD_TIMEOUT_MSEC
/* [MIPV6_21] INITIAL_DHAAD_TIMEOUT. Units are milliseconds. */
#define TM_6_MIP_INIT_DHAAD_TIMEOUT_MSEC        3000
#endif /* ! TM_6_MIP_INIT_DHAAD_TIMEOUT_MSEC */
#ifndef TM_6_MIP_INIT_SOLICIT_TIMER_MSEC
/* [MIPV6_21] INITIAL_SOLICIT_TIMER. Units are milliseconds. */
#define TM_6_MIP_INIT_SOLICIT_TIMER_MSEC        3000
#endif /* ! TM_6_MIP_INIT_SOLICIT_TIMER_MSEC */
#ifndef TM_6_MIP_MAX_BA_TIMEOUT_MSEC
/* [MIPV6_21] MAX_BINDACK_TIMEOUT. Units are milliseconds. */
#define TM_6_MIP_MAX_BA_TIMEOUT_MSEC            32000
#endif /* ! TM_6_MIP_MAX_BA_TIMEOUT_MSEC */
#ifndef TM_6_MIP_MAX_NONCE_LIFE_MSEC
/* [MIPV6_21] MAX_NONCE_LIFETIME. Units are milliseconds. */
#define TM_6_MIP_MAX_NONCE_LIFE_MSEC            240000
#endif /* ! TM_6_MIP_MAX_NONCE_LIFE_MSEC */
#ifndef TM_6_MIP_MAX_TOKEN_LIFE_MSEC
/* [MIPV6_21] MAX_TOKEN_LIFETIME. Units are milliseconds. */
#define TM_6_MIP_MAX_TOKEN_LIFE_MSEC            210000
#endif /* ! TM_6_MIP_MAX_TOKEN_LIFE_MSEC */

#ifndef TM_6_MIP_MIN_TOKEN_LIFE_MSEC
/* MIPv6 MN uses TM_6_MIP_MIN_TOKEN_LIFE_MSEC to implement
   [ISSUE103].R5.2.7:60 and [MIPV6_18++].R11.6.1:20. The value of 60 seconds
   is chosen based on the recommendation in the MIPv6 specification that CNs
   generate a new nonce every 30 seconds, and the assumption that a CN
   will keep more than the last two nonces (the specification recommends that
   the CN keep the last 8 nonces). */
#define TM_6_MIP_MIN_TOKEN_LIFE_MSEC            60000
#endif /* ! TM_6_MIP_MIN_TOKEN_LIFE_MSEC */

#ifndef TM_6_MIP_MAX_UPDATE_RATE
/* [MIPV6_21] MAX_UPDATE_RATE. Unit are BUs per second. The maximum frequency
   for BUs that the MN sends to the HA to register new CoAs. */
#define TM_6_MIP_MAX_UPDATE_RATE                3
#endif /* ! TM_6_MIP_MAX_UPDATE_RATE */

/* mobility header types  */
#define TM_6_MH_TYPE_BRR    (tt8Bit) 0
#define TM_6_MH_TYPE_HOTI   (tt8Bit) 1
#define TM_6_MH_TYPE_COTI   (tt8Bit) 2
#define TM_6_MH_TYPE_HOT    (tt8Bit) 3
#define TM_6_MH_TYPE_COT    (tt8Bit) 4
#define TM_6_MH_TYPE_BU     (tt8Bit) 5
#define TM_6_MH_TYPE_BA     (tt8Bit) 6
#define TM_6_MH_TYPE_BE     (tt8Bit) 7

/* mobility option types */
#define TM_6_MO_TYPE_PAD1               (tt8Bit) 0     
#define TM_6_MO_TYPE_PADN               (tt8Bit) 1
#define TM_6_MO_TYPE_BIND_REF_ADVICE    (tt8Bit) 2 
#define TM_6_MO_TYPE_ALT_COA            (tt8Bit) 3
#define TM_6_MO_TYPE_NONCE_INDEX        (tt8Bit) 4
#define TM_6_MO_TYPE_BIND_AUTH_DATA     (tt8Bit) 5

/* Binding Acknowledgement and Binding Error status codes */
/* Unknown binding for Home Address destination option */
#define TM_6_MIP_BE_NO_BINDING          (tt8Bit) 1
/* Unrecognized MH Type value */
#define TM_6_MIP_BE_BAD_MH_TYPE         (tt8Bit) 2
/* Binding Update accepted */
#define TM_6_MIP_BA_ACCEPTED            (tt8Bit) 0
/* Binding Update accepted with reduced lifetime due to prefix deprecation */
#define TM_6_MIP_BA_DEPRECATED          (tt8Bit) 1
/* BU failed: reason unspecified */
#define TM_6_MIP_BA_UNSPEC_FAILED       (tt8Bit) 128
/* BU failed: administratively prohibited */
#define TM_6_MIP_BA_ADMIN_PROHIBIT      (tt8Bit) 129
/* BU failed: insufficient resources */
#define TM_6_MIP_BA_NO_RESOURCE         (tt8Bit) 130
/* BU failed: home registration not supported */
#define TM_6_MIP_BA_HOME_REG_NOSUPPORT  (tt8Bit) 131
/* BU failed: not home subnet */
#define TM_6_MIP_BA_NOT_HOME_SUBNET     (tt8Bit) 132
/* BU failed: not home agent for this mobile node */
#define TM_6_MIP_BA_NOT_HOME_AGENT      (tt8Bit) 133
/* BU failed: duplicate Address Detection failed */
#define TM_6_MIP_BA_DAD_FAILED          (tt8Bit) 134
/* BU failed: sequence number out of window */
#define TM_6_MIP_BA_BAD_SEQ             (tt8Bit) 135
/* BU failed: expired home nonce index */
#define TM_6_MIP_BA_BAD_HO_NONCE_INDEX  (tt8Bit) 136
/* BU failed: expired care-of nonce index */
#define TM_6_MIP_BA_BAD_CO_NONCE_INDEX  (tt8Bit) 137
/* BU failed: expired nonces */
#define TM_6_MIP_BA_BAD_NONCES          (tt8Bit) 138
/* BU failed: expired nonces */
#define TM_6_MIP_BA_REG_TYPE_CHANGE     (tt8Bit) 139
        
/* Binding Update flag values */
/* The Acknowledge (A) bit is set by the sending mobile node to request a
   Binding Acknowledgement (Section 6.1.8) be returned upon receipt of the
   Binding Update. */
#define TM_6_MIP_BU_FLAG_ACK    (tt8Bit) 0x80
/* The Home Registration (H) bit is set by the sending mobile node to request
   that the receiving node should act as this node's home agent.  The
   destination of the packet carrying this message MUST be that of a router
   sharing the same subnet prefix as the home address of the mobile node in
   the binding. */
#define TM_6_MIP_BU_FLAG_HOME   (tt8Bit) 0x40
/* The Link-Local Address Compatibility (L) bit is set when the home address
   reported by the mobile node has the same interfaceidentifier (IID) as the
   mobile node's link-local address. */
#define TM_6_MIP_BU_FLAG_LL     (tt8Bit) 0x20
/* The Key Management Mobility Capability (K) bit is cleared when the
   protocol used for establishing the IPsec security associations between the
   mobile node and the home agent does not survive movements. */
#define TM_6_MIP_BU_FLAG_KEYMGT (tt8Bit) 0x10

/* The Key Management Mobility +-+-+-+-+-+-+-+-+
 * Capability (K) bit for      |K|  Reserved   |
 * Binding Acknowledgement     +-+-+-+-+-+-+-+-+
 */
#define TM_6_MIP_BA_FLAG_KEYMGT (tt8Bit) 0x80

/* length of the autenticator, 12 byte, 96 bits */
#define TM_6_MIP_AUTH_LEN           12  
/* length of the autention option, 14 byte */
#define TM_6_MIP_AUTH_OPTION_LEN    (TM_6_MIP_AUTH_LEN + 2)
/* length of the Kbm = SHA1 (home keygen token | care-of keygen token) */
#define TM_6_MIP_KBM_LEN            20

#define TM_6_MIP_NONCE_INDEX_OPTION_LEN 4
#endif /* Mobile IPv6 enabled */

#ifdef TM_6_USE_MIP_RA_RTR_ADDR
#ifndef TM_6_MN_RA_RTR_ADDR_MAX_CNT
/* The number of global scope router addresses to keep track of for the
   purpose of detecting if a router advertisement we receive is from our
   current default router. */
#define TM_6_MN_RA_RTR_ADDR_MAX_CNT 5
#endif /* TM_6_MN_RA_RTR_ADDR_MAX_CNT */
#endif /* TM_6_USE_MIP_RA_RTR_ADDR */

#ifdef TM_6_USE_MIP_MN
#ifndef TM_6_MN_PREFIX_LT_BIAS_MSEC
/* The amount of time (in milliseconds) before a home prefix becomes invalid
   that will trigger us to start sending Mobile Prefix Solicitation messages.
   ([MIPV6_21].R11.4.2:10) */
#define TM_6_MN_PREFIX_LT_BIAS_MSEC 600000
#endif /* ! TM_6_MN_PREFIX_LT_BIAS_MSEC */

/* Mobile node state flags (i.e. tt6MnVect.mns6Flags) */
/* Mobile IPv6 mobile node state flag indicates that the Mobile IPv6 mobile
   node is active (i.e. tf6MnStartMobileIp has been called). */
#define TM_6_MNS_MOBILE_NODE_ACTIVE     (tt8Bit) 0x01
/* Mobile IPv6 mobile node state flag indicates that we sent a Mobile Prefix
   Solicitation and we are waiting for the solicited Mobile Prefix
   Advertisement. */
#define TM_6_MNS_SENT_PREFIX_SOLICIT    (tt8Bit) 0x02
/* Mobile IPv6 mobile node state flag indicates that we are in the middle of
   processing a L3 move notification, we are soliciting for a router and
   waiting to receive the solicited Router Advertisement message. */
#define TM_6_MNS_L3_MOVE_PENDING        (tt8Bit) 0x04
/* Mobile IPv6 mobile node state flag indicates that we have confirmed that
   the mobile interface is attached to our home network. */
#define TM_6_MNS_AT_HOME                (tt8Bit) 0x08
/* Mobile IPv6 mobile node state flag indicates that we have a previous
   primary CoA. */
#define TM_6_MNS_PREV_PRIMARY_COA       (tt8Bit) 0x10
/* Mobile IPv6 mobile node state flag indicates that we have successfully
   registered a mobility binding with the home agent for a primary CoA. */
#define TM_6_MNS_REG_PRIMARY_COA        (tt8Bit) 0x20
/* Mobile IPv6 mobile node state flag indicates that we have already
   modified IKE state and the existing IPsec SA's. Such modification
   must be made at each movement */
#define TM_6_MNS_IPSEC_BU               (tt8Bit) 0x40
/* Mobile IPv6 mobile node state flag indicates that the home agent has
   been confirmed to be reachable */
#define TM_6_MNS_HA_REACHABLE           (tt8Bit) 0x80

#endif /* TM_6_USE_MIP_MN */

#ifdef TM_6_USE_MIP_RO
#ifndef TM_6_MIP_NEAR_EXP_TIME_MSEC
/* in milliseconds, indicates the time before an entry is going to expire
   that we send BRR for it. */
#define TM_6_MIP_NEAR_EXP_TIME_MSEC             10000
#endif /* ! TM_6_MIP_NEAR_EXP_TIME_MSEC */
#endif /* TM_6_USE_MIP_RO */

#ifdef TM_6_USE_MIP_CN
/* in milliseconds, indicates the longest period of inactivity that we
   still consider an BE to be fresh. */
#define TM_6_MIP_BCE_IN_USE_TIME_MSEC           60000

#ifndef TM_6_MIP_CN_MAX_BINDINGS
/* max number of ntBinding entries we support */
#define TM_6_MIP_CN_MAX_BINDINGS    20
#endif /* ! TM_6_MIP_CN_MAX_BINDINGS */

#ifndef TM_6_MIP_CN_MAX_DEST_UNREACH_ICMP
/* 
 * we consider this ICMP percistent if it exceeds this number 
 * for the same original dest address, will result in the deletetion
 * of the binding entry for the MN
 */
#define TM_6_MIP_CN_MAX_DEST_UNREACH_ICMP   3
#endif /* TM_6_MIP_CN_MAX_DEST_UNREACH_ICMP */

#ifndef TM_6_MIP_CN_ICMP_MIONTOR_PERIOD
/* 
 * msecs, time we monitor the dest unreahable ICMP messages after we've
 * sent a packet to an MN via its care of address 
 */
#define TM_6_MIP_CN_ICMP_MIONTOR_PERIOD     5000
#endif /* TM_6_MIP_CN_ICMP_MIONTOR_PERIOD */

/* the period that an old nonce and Kcn pair is discarded and new pair
   is generated */
#define TM_NONCE_REGEN_MSEC         30000

/* The maximum number of nonces that we support.*/
#define TM_6_MIP_MAX_NONCE_COUNT    8

/* The maximum value of the nonce index */
#define TM_6_MIP_MAX_NONCE_INDEX    65536/* 16 bit */

/* The length of a nonce in bytes.*/
#define TM_6_MIP_NONCE_LEN          8

/* The length of Kcn in bytes*/
#define TM_6_MIP_KCN_LEN            20

#endif /* TM_6_USE_MIP_CN */

#ifdef TM_6_USE_MIP_HA
#define TM_6_MIP_DEF_IS_HA          TM_8BIT_NO
#endif /* TM_6_USE_MIP_HA */

#endif /* TM_USE_IPV6 */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _T6MACRO_H_ */
