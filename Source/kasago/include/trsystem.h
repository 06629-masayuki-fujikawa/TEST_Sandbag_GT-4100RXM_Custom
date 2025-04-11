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
 * Description:     System Description Macro's
 * This file is intended for the user to modify for their hardware/os platform
 * Filename:        trsystem.h
 * Author:          Odile & Paul
 * Date Created:    9/22/97
 * $Source: include/trsystem.h $
 *
 * Modification History
 * $Revision: 6.0.2.37 $
 * $Date: 2015/06/19 13:01:44JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TRSYSTEM_H_

#define _TRSYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

/* comment out the following line if you do not want to use IPv4 protocols */
#define TM_USE_IPV4

/* uncomment the following line if you want to use IPv6 protocols */
/* #define TM_USE_IPV6 */

/*
 * Uncomment the following line if you want to be able to call tfStopTreck()
 * (resp. tfStopTreckMultipleContext() that will stop the treck stack for 
 * the given context (resp. all contexts), and free all Treck stack allocated
 * memory for the given context (resp. all contexts). 
 * Warning: All tasks making calls to the Treck stack, should be stopped
 * prior to calling tfStopTreck() or tfStopTreckMultipleContext()
 */
/* #define TM_USE_STOP_TRECK */

/*
 * Uncomment the following line if you want to use a backward compatible
 * version of tf6DhcpSetOption
 */
/* #define TM_6_DHCP_OLD_SET_OPTION */

/*
 * Uncomment TM_DISABLE_SOCKET_HASH_LIST, if you have limited data space,
 * or do not plan on having a lot of sockets. In that case the Treck
 * stack will not use hash tables during a socket table lookup.
 */
#define TM_DISABLE_SOCKET_HASH_LIST

/*
 * Uncomment TM_USE_SOCKET_RB_TREE, if you have limited data space, and
 * therefore disabled the socket hash tables, or reduced their sizes
 * dramatically, and still want a reasonable socket look up table time.
 * This will enable the socket Red Black tree code.
 */
#define TM_USE_SOCKET_RB_TREE

/*
 * Uncomment TM_6_USE_OBSOLETE_TF6GETROUTERID if you want to use the old
 * (obsoleted) version of tf6GetRouterId that returns an aggregate.
 * This function has been replaced by tf6NgGetRouterId() in sockapi6/t6grtrid.c
 */
/* #define TM_6_USE_OBSOLETE_TF6GETROUTERID */

/* comment out the following line if you do not want to use DHCP */
#define TM_USE_DHCP

/* comment out the following line if you do not want to use PPP protocols */
#define TM_USE_PPP

/* uncomment the following line if you want to use the new 802.3 Ethernet
   link-layer, which supports Ethernet II (DIX) in addition to IEEE 802.3 */
/* #define TM_USE_ETHER_8023_LL */

/*
 * Uncomment the following line if you want to use the PPPoE client link layer
 */
/* #define TM_USE_PPPOE_CLIENT */

/*
 * Uncomment the following line if you want to use the PPPoE server link layer
 */
/* #define TM_USE_PPPOE_SERVER */

/*
 * Uncomment the following line, if you want to use the netstat tool for
 * for outputting the routing entries, ARP entries and socket entries
 */
/* #define TM_USE_NETSTAT */

/*
 * Uncomment the following line to enable support for splitting out IPv4
 * and IPv6 traffic in the send path to separate virtual channels to
 * simulate multiple hosts as multiple IP aliases. Note that this is
 * incompatible with the default mode of IPv6 operation, therefore IPv6
 * prefix discovery must be disabled.
 */
/* #define TM_USE_VCHAN */

/*
 * Uncomment the following line to speed up lookup in the recieve-path of IP
 * aliases configured on the interface. This is intended for situations where
 * the user has configured many (i.e. >50) IP aliases on a single interface. 
 */
/* #define TM_OPTIMIZE_MANY_MHOMES */

/*
 * Uncomment the following line for SNMPv1 only.
 * (SNMP option only.)
 */
/* #define TM_SNMP_VERSION 1 */

/*
 * Uncomment the following line for SNMPv1 and SNMPv2c (bilingual).
 * (SNMP option only.)
 */
/* #define TM_SNMP_VERSION 2 */

/*
 * Uncomment the following line for SNMPv1, SNMPv2c and SNMPv3 (trilingual).
 * (SNMP option only.)
 */
/* #define TM_SNMP_VERSION 3 */

/*
 * Uncomment the TM_SNMP_CACHE macro if you want the stack to keep a cache of
 * the UDP sockets, TCP vectors, ARP table, and Routing table for SNMP.
 * (SNMP option only.)
 */
/* #define TM_SNMP_CACHE */

/*
 * Comment out TM_USE_RFC1213_MIB2 if you do not want the deprecated RFC1213
 * MIB2 objects that are designed only for IPv4 to be included in the SNMP
 * agent.
 * This option requires to enable TM_USE_IPV4 and TM_USE_SNMP_CACHE to be fully
 * functional). If you are running the stack in dual mode (IPV4 and IPV6), V6
 * objects will not be displayed in those table.
 * (SNMP option only.)
 */
#define TM_USE_RFC1213_MIB2

/*
 * Comment out TM_USE_NG_MIB2 to disable the Next Generation MIB2, which
 * consists of tables that support both IPv4 and IPv6 entities.
 * This option requires to enable TM_SNMP_CACHE to be fully functional.
 * (SNMP option only.)
 */
#define TM_USE_NG_MIB2

/*
 * Uncomment the following line for 8-bit OIDs.
 * (SNMP option only.)
 */
/* #define TM_SNMP_8BIT_SUBIDS */

/*
 * Uncomment the following line 16-bit OIDs.
 * (SNMP option only.)
 */ 
/* #define TM_SNMP_16BIT_SUBIDS */

/*
 * TM_IP_FRAGMENT macro. Enable support for IPv4 fragmentation.
 * Comment out to save code space when IPv4 fragmentation is not needed.
 */
#define TM_IP_FRAGMENT

/*
 * TM_IP_REASSEMBLY macro. Enable support for IPv4 reassembly.
 * Comment out to save code space when IPv4 reassembly is not needed.
 */
#define TM_IP_REASSEMBLY

/* 
 * This macro is commented out by default.
 * If you define TM_IP_FRAGMENT, and your device driver supports scattered 
 * send, then define TM_IP_FRAGMENT_NO_COPY to avoid an extra internal data
 * copy when an IP datagram is fragmented.
 */
/* #define TM_IP_FRAGMENT_NO_COPY */

/*
 * Uncomment the TM_DISABLE_TELNETD macro, if you want to remove the TELNETD code.
 */
#define TM_DISABLE_TELNETD

/*
 * Uncomment the TM_DISABLE_FTP macro, if you want to remove the FTP
 */
/* #define TM_DISABLE_FTP */

/*
 * Uncomment the TM_DISABLE_FTPD macro, if you want to remove the FTPD code.
 */
/* #define TM_DISABLE_FTPD */

/*
 * Uncomment the TM_DISABLE_TFTP macro, if you want to remove the TFTP
 */
#define TM_DISABLE_TFTP

/*
 * Uncomment the TM_DISABLE_TFTPD macro, if you want to remove the TFTPD code.
 */
#define TM_DISABLE_TFTPD

/*
 * Uncomment the TM_DISABLE_TCP macro, if you want to remove the TCP code.
 */
/* #define TM_DISABLE_TCP */

/*
 * Uncomment the TM_DISABLE_TCP_128BIT_RANDOM_ISS macro, if you want to
 * prevent the TCP code from using the MD5 hash algorithm to generate the TCP
 * initial sequence number based on an 128 bit random number.
 * WARNING: If you define that macro, then the code will use a 32-bit random
 * number instead and the stack will be more susceptible to security attacks
 * described in US-CERT VU#498440.
 */
#define TM_DISABLE_TCP_128BIT_RANDOM_ISS

/*
 * Uncomment the TM_DISABLE_TCP_SACK macro, if you want to remove the TCP
 * Selective Acknowlegment Options (RFC2018) code.
 */
/* #define TM_DISABLE_TCP_SACK */

/*
 * Uncomment the TM_DISABLE_TCP_FACK macro if you want to remove the TCP
 * Forward Acknowledgement algorithm code.
 * Disabling TCP FACK prohibits TCP SACK from retransmitting more than
 * one SACK'ed segment. This option is recommended for lossy links and
 * should only be disabled if there is no packet loss on the link.
 * TCP FACK requires TCP SACK, therefore TM_DISABLE_TCP_SACK implies
 * TM_DISABLE_TCP_FACK.
 */
/* #define TM_DISABLE_TCP_FACK */
    
/*
 * Uncomment the TM_DISABLE_TCP_RFC2414 macro, if you want to disable
 * the TCP Initial Send Window Increase as described in RFC 2414.
 */
/* #define TM_DISABLE_TCP_RFC2414 */

/*
 * Uncomment the TM_DISABLE_TCP_RFC2581 macro, if you want to remove the TCP
 * congestion control update to RFC2001 as described in RFC2581.
 */
/* #define TM_DISABLE_TCP_RFC2581 */

/*
 * Uncomment the TM_DISABLE_TCP_RFC3042 macro, if you want to remove the
 * Limited Transmit Algorithm that enhances TCP's Loss Recovery as described 
 * in RFC3042.
 */
/* #define TM_DISABLE_TCP_RFC3042 */

/*
 * Comment out the TM_DISABLE_TCP_ACK_PUSH macro, if you want the
 * stack to send an immediate TCP ACK, when receiving a TCP segment with
 * the PUSH bit set.
 * (Important: this macro should be commented out when the peer runs
 *  windows 2000.)
 */
#define TM_DISABLE_TCP_ACK_PUSH

/*
 * Uncomment the TM_DISABLE_PMTU_DISC macro, if you want to disable automatic
 * Path MTU discovery on indirect routes.
 */
/* #define TM_DISABLE_PMTU_DISC */

/*
 * Uncomment the TM_DISABLE_UDP_CACHE to remove the one socket cache UDP
 * lookup. This could be done to either save code space, or in case the
 * compiler has trouble parsing the one single UDP cache lookup code as
 * that code is somewhat complex. Removing the one socket UDP cache lookup
 * does not change the UDP functionality.
 */
#define TM_DISABLE_UDP_CACHE

/*
 * Uncomment the TM_ARP_UPDATE_ON_RECV, if you want to avoid sending an ARP
 * request every 10 minutes on an active ARP entry, i.e. an ARP entry for
 * a destination IP address that the stack is actively sending to.
 * Note that uncommenting out this macro will affect speed and size, since
 * every incoming packet will be checked for an ARP cache match on its
 * destination IP address.
 */
/* #define TM_ARP_UPDATE_ON_RECV */

/* 
 * Uncomment the TM_NO_COPY_OVER_USER_DATA, if you want to prevent the stack
 * from copying the TCP header in acked data area of a user owned buffer.
 */
/* #define TM_NO_COPY_OVER_USER_DATA */

/*
 * Currently the stack will start sending with a full size TCP Initial Window
 * on direct routes
 * Uncomment the TM_USE_TCP_SMALL_IW_DIRECT, if you want the stack to start
 * sending using a reduced TCP Initial Window as it does on indirect routes.
 */
/* #define TM_USE_TCP_SMALL_IW_DIRECT */

/*
 * Currently the stack will discard any SYN segment that have the FIN, or PSH,
 * or URG bit set, and have the ACK bit not set, if there is no data.
 * Uncomment the TM_USE_TCP_CERT_VU464113 macro, if you want the stack
 * to discard these segments even when there is data. 
 * Note that defining this macro will not allow T/TCP to work.
 */
/* #define TM_USE_TCP_CERT_VU464113 */

/*
 * Uncomment the TM_USE_TCP_PACKET macro, if you want to modify the
 * TCP behaviour and force TCP to send data on user send packet boundaries.
 * Warning: uncommenting this macro, will make TCP less efficient, and
 * will also disable Path MTU discovery, and TCP Selective Acknowledgements.
 */
/* #define TM_USE_TCP_PACKET */

/*
 * Uncomment the TM_USE_REUSEADDR_LIST macro, if you want to use the
 * SO_REUSEADDR socket level option with setsockopt(). This enables use of
 * the socket reuse address list, i.e. enable binding on the same port using
 * different local IP addresses.
 */
#define TM_USE_REUSEADDR_LIST

/*
 * Uncomment the TM_USE_REUSEPORT macro, if you want to use the
 * SO_REUSEPORT socket level option with setsockopt(). This enables use of
 * the socket reuse address list, and enables binding several sockets on the
 * same port and same local IP address.
 */
/* #define TM_USE_REUSEPORT */

/*
 * Uncomment the TM_USE_AUTO_IP macro, if you want to use Auto configuration,
 * or want to add collision detection.
 */
/* #define TM_USE_AUTO_IP */

/*
 * Uncomment TM_USE_DHCP_FQDN to enable the DHCP FQDN option
 */
/* #define TM_USE_DHCP_FQDN */

/*
 * If you do not need to select from a collection of DHCP offers, you
 * can uncomment the TM_DISABLE_DHCP_COLLECT macro to remove the code
 * that provides support for this feature.
 */
/* #define TM_DISABLE_DHCP_COLLECT */

/*
 * Uncomment the TM_USE_RAW_SOCKET macro, if you want to use raw sockets.
 * (tfRawSocket() need to be called, instead of socket())
 */
/* #define TM_USE_RAW_SOCKET */

/*
 * Uncomment the TM_RAW_SOCKET_INPUT_ICMP_REQUESTS, if you want ICMP echo
 * requests, and ICMP address mask requests to be delivered to a raw ICMP
 * socket.
 */
/* #define TM_RAW_SOCKET_INPUT_ICMP_REQUESTS */

/*
 * Uncomment TM_PING_REPLY_BROADCAST if you want ICMP replies to be sent
 * when the requested destination is a broadcast or a multicast address.
 */
/* #define TM_PING_REPLY_BROADCAST */

/*
 * Uncomment TM_PING_REPLY_MULTICAST if you want ICMP replies to be sent
 * when the requested destination is a multicast address.
 * (not needed if TM_PING_REPLY_BROADCAST is enabled)
 */
/* #define TM_PING_REPLY_MULTICAST */

/*
 * Uncomment the TM_IGMP macro, if you have purchased the IGMP protocol
 */
/* #define TM_IGMP */

/*
 * Uncomment the TM_USE_IGMPV3 macro, if you have purchased the IGMP protocol
 * and wish to use IGMPv3.
 */
/* #define TM_USE_IGMPV3 */

/*
 * Uncomment TM_DNS_USE_SRV if you want DNS SRV record support.
 * This applies only if you have purchased the DNS Resolver product.
 */
/* #define TM_DNS_USE_SRV */

/*
 * Uncomment TM_DNS_MAX_SOCKETS_ALLOWED if you want to specify the number
 * of simultaneous DNS requests allowed. Default: 1.
 * This applies only if you have purchased the DNS Resolver product.
 */
/* #define TM_DNS_MAX_SOCKETS_ALLOWED 1 */

/*
 * Uncomment TM_DNS_TRANS_ID_BACKLOG_SIZE if you want to specify the size of the
 * backlog of random Transaction IDs. All newly generated Transaction IDs are
 * required to be unique to this list and to all Transaction IDs currently in
 * the DNS cache. The default value is 8. For security purposes this value
 * should remain as small as is reasonably possible.
 * This applies only if you have purchased the DNS Resolver product.
 */
/* #define TM_DNS_TRANS_ID_BACKLOG_SIZE 8 */

/* 
 * Uncomment TM_USE_SNTP if you want to use to synchronize computer clocks 
 * in the Internet.
 */
/* #define TM_USE_SNTP */
/* 
 * Uncomment TM_USE_ERROR_CHECKING if you want to perform internal checking
 * for SNTP message.
 */
/* #define TM_SNTP_ERROR_CHECKING */
/* 
 * Uncomment TM_USE_SNTP_MULTICAST if you want to use multicast mode for SNTP.
 */
/* #define TM_USE_SNTP_MULTICAST */

/* 
 * PPP options 
 */
/*
 * Uncomment the TM_PPP_LQM macro, if you want PPP Link Quality Monitoring
 */
/* #define TM_PPP_LQM */

/*
 * Uncomment the TM_USE_IPHC to enable IP header compression (RFC-2507) with
 * PPP.
 */
/* #define TM_USE_IPHC */

/*
 * Uncomment the TM_USE_EAP to enable Extensible Authentication Protocol with
 * PPP.
 */
/* #define TM_USE_EAP */

/* define the following macro to use MSCHAP */
/*#define TM_USE_PPP_MSCHAP*/

/*
 * NAT Options
 */

/* Enable NAT progress display with TM_ERROR_LOGGING, TM_DEBUG_LOGGING */
/* and/or TM_TRACE_LOGGING.  (TM_NAT_TRACE used to do this.)           */

/* To enable tfNatDump(), which can display a list of NAT triggers,    */
/* define TM_DEBUG_LOGGING in this file (TM_NAT_DUMP used to do this)  */

/* Discard packets that NAT does not transform */
#define TM_NAT_STRICT

/*
 * Uncomment the TM_USE_UPNP_DEVICE macro to enable UPnP network device
 * support.
 * NOTE: if you enable TM_USE_UPNP_DEVICE, you must also enable TM_USE_HTTPD
 * and TM_USE_HTTPD_SSI, since Treck UPnP uses Treck web server SSI.
 */
/* #define TM_USE_UPNP_DEVICE */

/*
 * Uncomment the TM_USE_SOAP macro to enable SOAP.
 * NOTE: if you enable TM_USE_SOAP, you must also enable TM_USE_HTTPD, since
 * Treck SOAP uses Treck web server.
 * NOTE: TM_USE_SOAP is automatically enabled for you when you enable
 * TM_USE_UPNP_DEVICE.
 */
/* #define TM_USE_SOAP */

/*
 * Uncomment the TM_USE_UNICODE macro to enable Treck UPnP support for
 * Unicode. By default, Treck UPnP assumes that all service state
 * variables of UPnP type "string" or "char" are ASCII, not Unicode.
 */
/* #define TM_USE_UNICODE */ 

/*
 * Uncomment the TM_USE_FLOATING_POINT macro to enable Treck UPnP support for
 * the UPnP floating point types "r4", "r8", "fixed.14.4" and "float".
 */
/* #define TM_USE_FLOATING_POINT */

/*
 * Uncomment the TM_USE_HTTPD to enable HTTPD (web server)
 */
/* #define TM_USE_HTTPD */

/*
 * Uncomment the TM_USE_HTTPD_CGI to enable CGI supprt of HTTPD (web server)
 */
/* #define TM_USE_HTTPD_CGI */
    
/*
 * Uncomment the TM_USE_POP3 to enable POP3
 */
/* #define TM_USE_POP3 */

/*
 * Uncomment the TM_USE_SMTP to enable SMTP
 */
/* #define TM_USE_SMTP */

/*
 * Uncomment the TM_MOBILE_IP4 macro, if you have purchased the Mobile IPv4
 * option.
 */
/* #define TM_MOBILE_IP4 */

/*
 * Uncomment the TM_MIP_USE_DHCP macro if you have purchased the Mobile IPv4
 * option and the DHCP option.
 * Note that this option will enable TM_USE_DHCP automatically
 */
/* #define TM_MIP_USE_DHCP */

/*
 * IPsec Option
 * Uncomment the TM_USE_IPSEC macro, if you have purchased the IPSEC option.
 */
/*#define TM_USE_IPSEC */


/*
 * IPsec SA Timer Option
 * Uncomment the TM_DISABLE_IPSEC_SA_TIMERS macro, if you want to disable the
 * lifetime timers.  If this is done, there should be some external timer that
 * manages the callback to invoke the soft and hard lifetime logic.
 */
/*#define TM_DISABLE_IPSEC_SA_TIMERS */

/*
 * IKEv1 Option
 * Uncomment the TM_USE_IKEV1 macro, if you are going to use IKEv1 as KEY 
 * management rather than manual keying
 */
/*#define TM_USE_IKEV1 */

/*
 * IKEv2 Option
 * Uncomment the TM_USE_IKEV2 macro, if you are going to use IKEv2 as KEY 
 * management rather than manual keying
 */
/*#define TM_USE_IKEV2 */

/*
 * Uncomment this to set the minimum percentage of the IKE SA hard 
 * byte lifetime allowed before the soft byte lifetime expires to 20%.
 * If not set, this defaults to 20%.
 */
/* #define TM_ISAKMPSA_VLIFETIME_SOFT_MARGIN_PCT 20 */

/*
 * Uncomment this to set the minimum percentage of the IPsec SA hard 
 * byte lifetime allowed before the soft byte lifetime expires to 20%.
 * If not set, this defaults to 20%.
 */
/* #define TM_IPSECSA_VLIFETIME_SOFT_MARGIN_PCT 20 */


/*
 * Uncomment the TM_USE_IKE_TASK macro, if you want to
 * run IKE as its own task while processing received packets from the network.
 */
/* #define TM_USE_IKE_TASK */

/*
 * Uncomment the TM_USE_NATT macro, if you want to enable the
 * Nat traversal code in IKE.
 */
/* #define TM_USE_NATT */

/*
 * Comment the TM_IKE_CHECK_HOST_ENTRY_ID macro, if you want to enable the
 * checking peer entry ID when IKE policy is for the host.
 */
#define TM_IKE_CHECK_HOST_ENTRY_ID

#ifdef TM_USE_IKEV2
/*
 * Uncomment the TM_IKEV2_USE_TRANSPORT_MODE macro, if you want to eanble the
 * Transport mode in IKEv2.
 */
/* #define TM_IKEV2_USE_TRANSPORT_MODE */

/*
 * Uncomment the TM_USE_IKEV2_EAP_AUTH macro, if you want to eanble the
 * EAP Authentication in IKEv2.
 */
/* #define TM_USE_IKEV2_EAP_AUTH */

#ifdef TM_USE_IKEV2_EAP_AUTH
/*
 * Uncomment the TM_USE_IKEV2_EAP_IDENTITY macro, if you want to eanble the
 * Authentication method EAP IDENTITY in IKEv2 eap authentication.
 */
/* #define TM_USE_IKEV2_EAP_IDENTITY */
/*
 * Uncomment the TM_USE_IKEV2_EAP_MD5 macro, if you want to eanble the
 * Authentication method EAP MD5 in IKEv2 eap authentication.
 */
#define TM_USE_IKEV2_EAP_MD5
/*
 * Uncomment the TM_USE_IKEV2_EAP_MSCHAPV2 macro, if you want to eanble the
 * Authentication method EAP MSCHAPV2 in IKEv2 eap authentication.
 */
#define TM_USE_IKEV2_EAP_MSCHAPV2
#endif /* TM_USE_IKEV2_EAP_AUTH */

/*
 * Uncomment the TM_IKEV2_USE_AH macro, if you want to eanble the
 * Authentication Header in IKEv2.
 */
/* #define TM_IKEV2_USE_AH */

/*
 * Uncomment the TM_IKEV2_USE_COOKIE macro, if you want to eanble the
 * COOKIE in IKEv2.
 */
/* #define TM_IKEV2_USE_COOKIE */

/*
 * Uncomment the TM_IKEV2_ENABLE_IKE_SA_REKEY macro, if you want to eanble the
 * IKE SA rekey in IKEv2. 
 */
/* #define TM_IKEV2_ENABLE_IKE_SA_REKEY */

/*
 * Uncomment the TM_IKEV2_REMOTE_CLIENT macro, if you want to eanble the
 * IRAC (IPsec Remote Access Client) in IKEv2. 
 */
/* #define TM_IKEV2_REMOTE_CLIENT */

/*
 * Uncomment the TM_IKEV2_REMOTE_SERVER macro, if you want to eanble the
 * IRAS (IPsec Remote Access Server) Lite in IKEv2. 
 */
/* #define TM_IKEV2_REMOTE_SERVER */

/*
 * Comment the TM_IKEV2_USE_MICROSOFT_EXTENSION macro, if you want to connect to
 * Windows in IKEv2. 
 */
#define TM_IKEV2_USE_MICROSOFT_EXTENSION

#endif /* TM_USE_IKEV2 */

/*
 * Enhanced Logging
 * Uncomment the TM_USE_LOGGING_LEVEL macro, if you want to use Enhanced
 * Logging.  Note that each module and level will also need to be defined
 * (i.e. TM_LOG_ENB_ALL_LEVELS and TM_LOG_ENB_ALL_MODULES or specific
 * levels/modules as required).  For more information, see trmacro.h or
 * consult the appropriate user documentation.
 */
/*#define TM_USE_LOGGING_LEVELS */

/*
 * SSL Client Option
 * Uncomment the TM_USE_SSL_CLIENT macro, if you are going to use SSL client
 * functionality
 */
//#define TM_USE_SSL_CLIENT

/*
 * SSL Server Option
 * Uncomment the TM_USE_SSL_SERVER macro, if you are going to use SSL server
 * functionality
 */
//#define TM_USE_SSL_SERVER

/*
 * SMTP client over SSL option
 * Uncomment the TM_USE_SMTP_SSL macro, if you are going to use SMTP client
 * over SSL functionality
 */
/* #define TM_USE_SMTP_SSL */

/*
 * FTP client over SSL option
 * Uncomment the TM_USE_FTP_SSL macro, if you are going to use FTP client
 * over SSL functionality
 */
/*#define TM_USE_FTP_SSL */
#if ( ( defined (TM_USE_FTPD) ) && ( defined (TM_USE_SSL_SERVER) ) )
/*
 * FTP server over SSL option
 * Uncomment the TM_USE_FTPD_SSL macro, if you are going to use FTP server
 * over SSL functionality
 */
/*#define TM_USE_FTPD_SSL */
#endif /* (defined (TM_USE_FTPD) && (defined (TM_USE_SSL_SERVER) ) */
/*
 * SSL Debuging option
 * Uncomment the TM_SSL_DEBUG macro, if you are going to debug SSL 
 * encrypt/decrypt, MAC, and padding
 */
/*#define TM_SSL_DEBUG */

/*
 * Allow the crypto engine to block while processing data
 */
/* #define TM_USE_ASYNC_CRYPTO */

/*
 * Uncomment the TM_DEV_SEND_OFFLOAD macro, if you want to use the
 * device driver TCP segmentation/checksum offload capabilities.
 */
/* #define TM_DEV_SEND_OFFLOAD */

/*
 * Uncomment the TM_DEV_RECV_OFFLOAD macro, if you want to use the
 * device driver recv packet checksum offload capabilities.
 */
/* #define TM_DEV_RECV_OFFLOAD */

/* 
 * Uncomment the TM_USE_DRV_ONE_SCAT_SEND macro, if you want to use
 * a single call to the device driver, passing the packet handle, even
 * when sending a frame with scattered buffers. Note that to enable 
 * this feature, this macro needs to be added, and tfUseInterfaceOneScatSend
 * needs to be called on the interface that supports it.
 */
#define TM_USE_DRV_ONE_SCAT_SEND

/* 
 * Uncomment the TM_USE_DRV_SCAT_RECV macro, if you want to allow
 * the device driver recv function to pass back a frame to the stack
 * in scattered buffers ("Gather Read"). Note that to enable this feature, 
 * this macro needs to be added, and  tfUseInterfaceScatRecv needs to be 
 * called on the interface that supports it.
 */
/* #define TM_USE_DRV_SCAT_RECV */

/*
 * Uncommment the TM_USE_USER_PARAM macro if you want Treck to supply and
 * manage a user defined data item that is unique for each TCP connection.
 * This data can be stored and retrieved using the TM_TCP_USER_PARAM option
 * when calling setsockopt() and getsockopt(), respectively.
 * When TM_USE_DRV_SCAT_RECV and TM_USE_DRV_ONE_SCAT_SEND are also defined,
 * this data item can be accessed by your device driver, as follows:
 *  - In the ttDevOneScatSendFuncPtr function you set by calling
 *    tfUseInterfaceOneScatSend(), read the pktuUserParam member of the
 *    ttUserPacketPtr parameter. Treck will copy the user data from the
 *    TCP socket to the ttUserPacket structure before calling your
 *    scattered send function.
 *  - In the ttDevScatRecvFuncPtr function you set by calling
 *    tfUseInterfaceScatRecv(), write the druUserParam member in the first
 *    ttDruBlock you return via the ttDruBlockPtrPtr parameter. Treck will
 *    copy this data from the ttDruBlock to the new TCP socket that is
 *    created for each incoming connection request received by a listening
 *    socket.
 * For an active connection, your application should set the user data
 * prior to calling connect().
 * For a passive connection, your application can retrieve the data from
 * the new socket returned by accept().
 */
/* #define TM_USE_USER_PARAM */

/*
 * Uncommment TM_NEED_ETHER_32BIT_ALIGNMENT and uncomment TM_USE_DRV_SCAT_RECV,
 * without using tfUseInterfaceScatRecv(),
 * if you want tfRecvInterface to make the TCP/IP header aligned on a
 * 4-byte boundary, if it is not aligned coming from a single buffer in
 * an Ethernet device driver.
 */
/* #define TM_NEED_ETHER_32BIT_ALIGNMENT */

/*
 * If your processor has a data cache and your network hardware supports DMA,
 * you should define this macro to be the cache line size (in bytes) of your
 * data cache. This ensures that all packet buffers will be allocated to start
 * and end at cache line boundaries, making cache flushing and invalidation in
 * the driver much easier to handle.
 * This option will cause extra RAM utilization in your system. Up to
 * ((2 * TM_PACKET_BUF_CACHE_ALIGN) - 2) extra bytes will be allocated for
 * each packet buffer to fulfill the alignment requirements.
 */
/* #define TM_PACKET_BUF_CACHE_ALIGN 64 */

/*
 * This macro causes the stack to manage data cache coherency internally. If
 * this is defined, the API's tfUseInterfaceCacheInvalFunc,
 * tfUseInterfaceCacheFlushFunc, and tfGetCacheSafeDrvBuf become available. The
 * driver buffer pool (tfPoolXXX) is also modified to use this functionality.
 * The stack will call the registered callback functions to flush or invalidate
 * data packets as needed. tfGetCacheSafeDrvBuf can be called to pre-allocate
 * receive buffers in the driver, if necessary.
 * TM_PACKET_BUF_CACHE_ALIGN must be correctly defined for this to work.
 * Note: This will not work on byte-stuffing interfaces (PPP, SLIP, etc.)
 */
/* #define TM_USE_DCACHE_MANAGEMENT */

/*
 * Uncomment the TM_LOOP_TO_DRIVER, if you want to loop back all the way
 * to the driver when sending to an Ethernet interface IP address.
 */
#define TM_LOOP_TO_DRIVER

/*
 * Undefine/delete the TM_INDRV_INLINE_SEND_RECV macro to test intra 
 * machine driver with a separate recv task. In that case the intra machine 
 * received data will no longer be processed in the send path, but will be 
 * processed when tfRecvInterface/tfRecvScatInterface is called. In the
 * examples directory, the txscatlp.c, and txscatdr.c modules contain sample
 * code that uses this feature.
 */
#define TM_INDRV_INLINE_SEND_RECV

/*
 * Uncomment the TM_USE_INDRT, if you want to use the GHS INDRT debugging
 * tools . You also need to call tfUseIndrt() on the Ethernet interface
 * you want the INDRT debug messages to be sent and received on.
 */
/* #define TM_USE_INDRT */

/*
 * Uncomment the TM_MULTIPLE_CONTEXT macro, if you want to use multiple
 * instances of the Treck stack.
 */
/* #define TM_MULTIPLE_CONTEXT */

/*
 * Uncomment the TM_USE_STRONG_ESL macro, if you want to bind a socket
 * to an interface/device, which will restrict traffic to and from
 * that socket to traffic on the bound device.
 */
/* #define TM_USE_STRONG_ESL */

/*
 * Uncomment the TM_USE_STRICT_STRONG_ESL macro, if you want the stack
 * to automatically bind a TCP accepted socket (whose listening socket
 * was not bound to a device) to the incoming device.
 */
/* #define TM_USE_STRICT_STRONG_ESL */

/*
 * By default the prototypes in the header files are not declared extern.
 * If your linker dictates that they should be declared extern, then
 * uncomment the following macro.
 */

/* #define TM_PROTO_EXTERN extern */

/*
 * Uncomment the TM_DISABLE_ANSI_LINE_FILE macro if your compiler does not
 * support the ANSI __LINE__ and __FILE__ macros.
 * The tm_assert() macro will use the ANSI __LINE__ and __FILE__ macros (if
 * available) to print the source line number and source file name identifying
 * where in the code an assertion failure occurred for debugging purposes.
 */
/* #define TM_DISABLE_ANSI_LINE_FILE */

/* Uncomment the TM_PC_LINT macro, if you are using PC-LINT */
/*#define TM_PC_LINT */


/*
 * Thread stop is used to stop a thread which has had an unrecoverable error
 * by default defined to be a forever loop
 *
 * Unrecoverable errors dictate that the system MUST be reset!
 * You can define this to the reset for your board
 */

/* #define tm_thread_stop rtosReset() */

/*
 * Define the TM_PING_MILLISECONDS_INTERVAL macro, if you want the
 * pingInterval parameter passed to the obsolete tfPingOpenStart API
 * to be in milliseconds rather than in seconds.
 */
/* #define TM_PING_MILLISECONDS_INTERVAL */


/*
 * Performance Macros
 */

/*
 * If we are using ethernet, we can use it inline to increase speed.
 */
#define TM_BYPASS_ETHER_LL

/*
 * Optimize for speed
 */
#define TM_OPTIMIZE_SPEED

/*
 * Optimize for size
 */
/* #define TM_OPTIMIZE_SIZE */

/*
 * If you have only a single interface, and you only configure a single IP
 * address on that interface (i.e. multi-homing is not required), then
 * uncomment the TM_SINGLE_INTERFACE_HOME macro to reduce code size.
 *
 * Warning: Defining this will prevent addition of the loop back device.
 * 1. Packets sent to the single interface IP address will be sent all the
 *    way to the driver, instead of being loop back by the stack.
 * 2. Packets cannot be sent to the IP loop back address, 127.0.0.1.
 */
/* #define TM_SINGLE_INTERFACE_HOME */

/*
 * If you have only a single interface, and you are using IPv6, then
 * uncomment the TM_SINGLE_INTERFACE macro to reduce code size. Note that
 * all IPv6 interfaces are multi-homed, so you cannot use the
 * TM_SINGLE_INTERFACE_HOME macro with IPv6.
 */
/* #define TM_SINGLE_INTERFACE */

/*
 * If you want to use AutoIP on multiple interfaces with Treck dual stack
 * IPv4+IPv6, then uncomment the TM_4_USE_SCOPE_ID macro. When
 * TM_4_USE_SCOPE_ID is enabled, any non-global scope IPv4 address matching
 * 10.0.0.0/8, 172.16.0.0/12, 192.168.0.0/16 or 169.254.0.0/16 (refer to the
 * macro IN4_IS_ADDR_LINKLOCAL in trmacro.h) must be stored in IPv4-mapped
 * IPv6 address format using "struct sockaddr_in6", and you must call the API
 * tf6SockaddrSetScopeId to set the scope ID on this "struct sockaddr_in6"
 * before passing it to any Treck API.
 */
/* #define TM_4_USE_SCOPE_ID */
    
/*
 * User defined checksum routines.
 * One of either TM_USER_OCS or TM_CUSTOM_OCS may be defined.
 * 1. If neither is defined, the stack will use a standard C implementation of
 *    the checksum or include a CPU-specific implementation if the user has
 *    defined an additional CPU specific macro such as TM_BORLAND_ASM_OCS,
 *    TM_GNUXSCALE_ASM_OCS, etc.
 * 2. TM_USER_OCS will cause the stack to include the file ocksum/trcsuser.c,
 *    which must be created by the user and must contain the function
 *    tfPacketChecksum().
 * 3. TM_CUSTOM_OCS will cause the stack to not compile in any checksum
 *    routine, meaning the user must link in a module containing the function
 *    tfPacketChecksum().
 *
 *    See the file trpkt.c for the standard implementation of this function.
 */
/* #define TM_USER_OCS */
/* #define TM_CUSTOM_OCS */

/* 
 * User defined random number generator
 * TM_USE_KERNEL_RNG will cause Treck's internal random number generator
 * to be replaced by the user-defined tfKernelGetRandom function.
 */
/* #define TM_USE_KERNEL_RNG */

/* If you have an different keyword for interrupt, then #define TM_INTERRUPT
   here to be that keyword
*/
/* #define TM_INTERRUPT interrupt */

/* !! NOTE about TM_GLOBAL_QLF and TM_GLOBAL_NEAR: *
 * 
 * Only one of them can be redefined to have a value, otherwise compiler
 * errors will occur
 */

/* Default qualifier for global variables. If you would like to put all global
 * variables into one memory type (e.g. far, huge, near), then #define
 * TM_GLOBAL_QLF here to be that qualifier and leave TM_GLOBAL_NEAR undefined
 */
/* #define TM_GLOBAL_QLF */

/* Default qualifier for frequently used global variables.
 * If you would like to put the frequently used global variables into 
 * one memory type (e.g. near) and the rest of them into a different
 * memory type (e.g. far, huge), then #define the memory type for frequently
 * used global variables here as TM_GLOBAL_NEAR and leave TM_GLOBAL_QLF
 * undefined
 */
/* #define TM_GLOBAL_NEAR */


/* Default qulifier for constants. If you would like to put all constants
 * into one memory type (e.g. far, huge, near), the #define TM_CONST_QLF
 * here to be that qualifier.
 */
/* #define TM_CONST_QLF */

/*
 * Uncomment the TM_USE_SHEAP macro if your RTOS does not provide heap
 * management routines, or if you do not want the Treck stack to allocate its
 * blocks of memory from the RTOS heap. When TM_USE_SHEAP is defined, the stack
 * allocates its blocks of memory from the Treck simple heap static array.
 */
#define TM_USE_SHEAP

/*
 * Simple heap size (used by simple heap allocation routines in trsheap.c)
 * This is used by tfSheapMalloc() and tfSheapFree() provided in
 * trsheap.c, and called by tfKernelMalloc() and tfKernelFree() when
 * the RTOS does not provide heap support, and you do not want to use
 * C library malloc and free.
 * This is not documented because it is an OS supplement.
 * On a DSP platform, the heap size should be specified in words; on all
 * other targets the heap size should be specified in bytes.
 */
#define TM_SHEAP_SIZE TM_UL(1)*TM_UL(192)*TM_UL(1024)

/*
 * Minimum block size in bytes when using the simple heap, either when
 * allocating a new block, or when refragmenting a freed block. 
 * If not defined here, default value is 8 if TM_ERROR_CHECKING is
 * defined, 4 otherwise.
 */
/* #define TM_SHEAP_MIN_BLOCK_SIZE 16 */

/*
 * Uncomment the TM_SHEAP_MARK_MEMORY macro, so that simple heap
 * initial memory is filled with 0xFE, so that allocated simple heap memory
 * is filled with 0xA0, and that, freed simple heap memory is filled with 0xFA.
 * This helps check whether memory is accessed after having been freed.
 */
/* #define TM_SHEAP_MARK_MEMORY */

/*
 * Uncomment the TM_DYNAMIC_CREATE_SHEAP macro if you want the Treck simple
 * heap to be dynamically allocated by calling tfKernelSheapCreate rather
 * than implemented as a static array. It is usually necessary to define
 * TM_DYNAMIC_CREATE_SHEAP, if TM_SHEAP_NUM_PAGE is bigger than 1 (see below).
 */
/* #define TM_DYNAMIC_CREATE_SHEAP */

/*
 * Number of memory pages. Instead of a unique array, the simple heap can be
 * divided into several ones. Uncomment TM_SHEAP_NUM_PAGE and indicate the
 * number of pages if you are using the simple heap with a system that has
 * paged memory and that one page is too small to hold the whole simple heap
 * memory (static or preallocated by tfKernelSheapCreate).
 * In this case the simple heap will be divided in several non-contiguous
 * pages. If you do define TM_SHEAP_NUM_PAGE to a number bigger than 1,
 * then you will likely need to define TM_DYNAMIC_CREATE_SHEAP also.
 * The default for TM_SHEAP_NUM_PAGE is 1. Each page of memory must
 * have the same size. The page size is TM_SHEAP_SIZE/TM_SHEAP_NUM_PAGE.
 * If the page size is lower or equal to 4Kb (TM_BUF_Q6_SIZE), you cannot
 * use dynamic memory and must define TM_DISABLE_DYNAMIC_MEMORY.
 */
/* #define TM_SHEAP_NUM_PAGE 8 */

/*
 * Uncomment TM_DISABLE_DYNAMIC_MEMORY if you need to save ROM and RAM space.
 * Note that this will disable the Treck internal recycled buffer lists, which
 * is part of the "Treck Learn" feature. The system will require less
 * memory, but will be slower, because the system will not be able to take
 * advantage of the "Treck Learn" feature.
 */
/* #define TM_DISABLE_DYNAMIC_MEMORY */

/*
 * Uncomment TM_USE_FILTERING to allow IPv4 datagrams to be delivered
 * to a user-specified callback function on a per-interface basis.  These
 * callbacks can be enabled/disabled for each device however this macro must
 * be enabled before any filter callback processing logic is processed.
 */
/* #define TM_USE_FILTERING */

/*
 * Uncomment TM_USE_FILTERING_CONTIGUOUS_DATA, if you already have uncommented
 * TM_USE_FILTERING, and if you wish to peek at the data in the user-specified
 * call back routine. In that case the data will be contiguous with the IPv4
 * header, and ulp header.
 */
/* #define TM_USE_FILTERING_CONTIGUOUS_DATA */

/*
 * Uncomment TM_6_USE_FILTERING to allow IPv6 datagrams to be delivered
 * to a user-specified callback function on a per-interface basis.  These
 * callbacks can be enabled/disabled for each device however this macro must
 * be enabled before any filter callback processing logic is processed.
 * When TM_6_USE_FILTERING is defined only the IPv6 header followed by
 * two bytes is guaranteed to be contiguous.
 * Please see also TM_6_USE_FILTERING_CONT_DATA below
 */
/* #define TM_6_USE_FILTERING */

/*
 * Uncomment TM_6_USE_FILTERING_CONT_DATA, if you already have uncommented
 * TM_6_USE_FILTERING, and if you wish to peek at the ULP, and/or data in the
 * user-specified call back routine. In that case the ULP and data will be
 * contiguous with the IPv6 header.
 */
/* #define TM_6_USE_FILTERING_CONT_DATA */

/*
 * Uncomment TM_USE_LBCAST_CONFIG to provide dynamic configuration of IPv4
 * Limited Broadcasting through the use of the TM_OPTION_IP_LBCAST_ENABLE
 * option to tfSetTreckOptions.
 */
/* #define TM_USE_LBCAST_CONFIG */

/*
 * Uncomment TM_USE_ECHO_CONFIG to provide dynamic configuration of IPv4
 * ICMP Echo Reples through the use of the TM_OPTION_ICMP_ECHO_ENABLE
 * option to tfSetTreckOptions.
 */
/* #define TM_USE_ECHO_CONFIG */

/*
 * Uncomment TM_USE_TCP_REXMIT_CONTROL to provide dynamic control of the
 * TCP Retransmission timer (pause, resume, reset).
 */
/* #define TM_USE_TCP_REXMIT_CONTROL */

/* Uncomment TM_USE_TCP_REXMIT_TEST to provide a Treck Demo toggle button
 * that provides for controlling whether TCP packets are dropped or processed
 */
/* #define TM_USE_TCP_REXMIT_TEST */

/* Uncomment TM_USE_IFNAMEINDEX to include support for RFC2553 functions
 * if_nameindex() and if_freenameindex().
 */
/* #define TM_USE_IFNAMEINDEX */

/* Enable only one of the following to choose the file system */
/* #define TM_USE_DOS_FS */
#define TM_USE_RAM_FS

/* #define TM_USE_ROM_FS */

/* preconfigured Win32 projects */
/* #define TM_TRECK_DEMO */
/* #define TM_AUTO_TEST */

/*
 * RTOS/Kernel Macros
 */

/* #define TM_KERNEL_WIN32_X86 */
/* #define TM_KERNEL_VISUAL_X86 */
/* #define TM_KERNEL_LINUX_APP */
/* #define TM_KERNEL_ELX_86 */
/* #define TM_KERNEL_UCOS_CPU32 */
/* #define TM_KERNEL_UCOS_X86 */
/* #define TM_KERNEL_UCOS_PPC */
/* #define TM_KERNEL_UCOS_MIPS */
/* #define TM_KERNEL_AMX_CPU32 */
/* #define TM_KERNEL_AMX_X86 */
/* #define TM_KERNEL_DOS_X86 */
/* #define TM_KERNEL_THREADX_ARM7 */
/* #define TM_KERNEL_THREADX_CPU32 */
/* #define TM_KERNEL_NONE_EZ80 */
/* #define TM_KERNEL_RZK_EZ80 */
/* #define TM_KERNEL_UCOS_XSCALE */
/* #define TM_KERNEL_NONE_H8S */
/* #define TM_KERNEL_UCOS_SH2 */
/* #define TM_KERNEL_CMX_C16X */
/* #define TM_KERNEL_REALOS_FR */
/* #define TM_KERNEL_TI_DSP_BIOS */
/* #define TM_KERNEL_QUADROS */
/* #define TM_KERNEL_NONE_S56XX */
/* #define TM_KERNEL_NONE_XILINX_MICROBLAZE */
/* #define TM_KERNEL_NONE_XILINX_POWERPC */
/* #define TM_KERNEL_NONE_RCM3200 */

/*
 * Do we have seperate tasks for processing receive or send complete
 */

/*
 * Received packets are processed in a seperate Receive task
 * This all depends on how we setup our device driver and if we
 * are using mode 2 or mode 4 (see below)
 */
/* #define TM_TASK_RECV */

/*
 * By default, packets are sent in the context of the application
 * sending the data, or of the timer task for retransmission of data,
 * or of the receive task for TCP acks, or PING echo replies for
 * example. If you wish to use a separate transmit task to
 * send the data to the driver, then define TM_TASK_XMIT.
 */
/* #define TM_TASK_XMIT */

/*
 * When a send complete occurs, we process it in a seperate task
 * This all depends on how we setup our device driver and if we
 * are using mode 2 or mode 4 (see below)
 */
/* #define TM_TASK_SEND */

/*
 * Device driver macros
 */

/* #define TM_QUICC */


/*
 * Compiler macros to take advantage on inline assembly (when needed)
 */

/* #define TM_COMPILER_SDS */     /* The SDS Crosscode Compiler for 68K */
/* #define TM_COMPILER_DDI_PPC */ /* The Diab Data Compiler for the PPC */
/* #define TM_COMPILER_GHS_PPC */ /* The Green Hills Compiler for the PPC */
/* #define TM_COMPILER_MRI_68K */ /* The MRI Compiler for 68K           */
/* #define TM_COMPILER_GHS_ARM */ /* The Green Hills Compiler for the ARM */
/* #define TM_COMPILER_GHS_68K */ /* The Green Hills Compiler for the 68K */
/* #define TM_COMPILER_GNU_PPC */ /* GNU GCC Compiler for the PPC */
/* #define TM_COMPILER_CW_5282 */ /* Metrowerks CodeWarrior for Motorola 5282*/

/* 
 * Define this macro, if compiler cannot handle nested braces
 * in the constant initialization of in6_addr structure 
 * (trsocket.h/trglobal.h/trinit.c)
 */
/* #define TM_IN6ADDR_NO_NESTED_BRACES */
/* 
 * Define this macro, if compiler cannot handle nested braces
 * in the constant initialization of tlFtpdCmdArray array (trftpd.c)
 */
/* #define TM_NO_NESTED_BRACES */

/*--------------------------------------------------------------------------*/

/*
 * The following macros are primitives that can be turned on/off to suite
 * your design (use these if you do NOT use the RTOS macros)
 */

/*
 * Processor Macros
 */
/* #define TM_EZ80 */
/* #define TM_INTEL_X86 */
/* #define TM_INTEL_XSCALE */
/* #define TM_MOTOROLA_CPU32 */
/* #define TM_MOTOROLA_68K */
/* #define TM_MOTOROLA_COLDFIRE */
/* #define TM_MOTOROLA_PPC */
/* #define TM_TMS320_C3 */
/* #define TM_TMS320_C3_BIG_ENDIAN */ /* For TI C3 like DSP with byte order reversed */
/* #define TM_TMS320_C5 */
/* #define TM_TMS320_C6 */

/*
 * Processor int size
 */

/*
 * Uncomment to avoid compiler warnings, if your processor integer size is
 * 16-bit
 */
/* #define TM_INT_LE16 */


/*
 * Word Order
 */

/* #define TM_LITTLE_ENDIAN */

/* To perform internal checking for the Treck Stack */
#define TM_ERROR_CHECKING

/*
 * Number of milliseconds per tick
 */
/* #define TM_TICK_LENGTH  10 */

/*
 * Uncomment TM_DISABLE_TIMER_CACHE if you have limited data space,
 * and do not plan on having a lot of sockets, routes, IPSEC SAs, and
 * therefore won't have a lot of timers.
 * In that case the timer code will just use one active timer queue, and won't
 * use a timer wheel for timers set to expire within TM_TIM_MAX_WHEEL_INTV ms.
 * NOTE: The timer execute thread will use more CPU time if this macro is
 * defined.
 * NOTE: If TM_OPTIMIZE_SIZE is defined, the timer cache is already disabled.
 */
#define TM_DISABLE_TIMER_CACHE

/*
 * If you have not uncommented TM_DISABLE_TIMER_CACHE, i.e. are using the
 * timer wheel, you may uncomment TM_USE_TIMER_MULT_ACTIVE_LIST, if you will
 * need a lot of timers, but do not have enough memory to extend the timer
 * wheel. In that case you can keep the timer wheel relatively small, and
 * will have four active lists (instead of one).
 */
/* #define TM_USE_TIMER_MULT_ACTIVE_LIST */

/*
 * Power save macro:
 * Uncomment TM_USE_TIMER_CB if you want to avoid calling
 * tfTimerUpdate/tfTimerExecute periodically, and would rather
 * have the stack call a user defined CB function (registered when the
 * user calls tfRegisterTimerCB()) that lets the user know when to call
 * tfTimerExecute() next.
 * In that case, the user is responsible for updating tvTime, and
 * tvTimeRollOver prior to calling tfTimerExecute().
 * NOTE: cannot uncomment TM_DISABLE_TIMER_CACHE for this to work.
 * NOTE: For power save mode, it is best to also define
 *       TM_USE_TIMER_MULT_ACTIVE_LIST, if the timer wheel is relatively
 *       small
 */
/* #define TM_USE_TIMER_CB */


/*
 * Power save macro:
 * Uncomment TM_USE_TIMER_INTERVAL_QUERY if the user wants to query
 * the stack (tfTimerIntervalQuery()) for the interval before the next call
 * to tfTimerExecute() before going into power save mode.
 * The user is responsible for updating tvTime, and tvTimeRollOver prior
 * to calling tfTimerExecute(), and tfTimerIntervalQuery().
 */
/* #define TM_USE_TIMER_INTERVAL_QUERY */


/*
 * Uncomment TM_USE_TCP_INCOMING_CB if you want to enable code that
 * can provide socket specific notification of valid incoming TCP
 * packets.
 * When enabled, the user can call tfRegisterSocketPacketCB() to
 * install a function on a particular TCP streaming socket that will
 * be called once for every valid incoming packet that is destined
 * for that socket.
 */
/* #define TM_USE_TCP_INCOMING_CB */

/*
 * Uncomment TM_USE_SOCKET_IMPORT if you want to enable code that
 * can create a new connected socket from information provided
 * by the user.
 */
/* #define TM_USE_SOCKET_IMPORT */


/*
 * Mode 1:
 * Treck TCP/IP is used without an RTOS or Kernel
 */
#define TM_TRECK_NO_KERNEL

/*
 * Mode 2:
 * Treck TC/IP is used as a shared library on a non-preemptive RTOS
 */
/* #define TM_TRECK_NONPREEMPTIVE_KERNEL */

/*
 * Mode 3:
 * Treck TCP/IP is used as an independent task on an RTOS
 * (preemptive or nonpreemtive)
 */
/* #define TM_TRECK_TASK */

/*
 * Mode 4:
 * Treck TCP/IP is used as a shared library on a premptive kernel
 */
/* #define TM_TRECK_PREEMPTIVE_KERNEL */


#define TM_BSD_CLASH
#define TM_USE_CLIB

#ifdef  TM_TRECK_DEMO
/*
 * Predefined Kernel Macros
 */
#define TM_USE_LOGGING_LEVELS
#define TM_LOG_ENB_ALL_LEVELS
#define TM_LOG_ENB_ALL_MODULES
#define TM_DEBUG_LOGGING

#define TM_USE_TCP_REXMIT_CONTROL
#define TM_USE_TCP_REXMIT_TEST
#ifdef TM_USE_ECHO_CONFIG
#define TM_ICMP_DEF_ECHO_ENABLE 1
#endif /* TM_USE_ECHO_CONFIG */
#ifdef TM_USE_LBCAST_CONFIG
#define TM_IP_DEF_LBCAST_ENABLE 0
#endif /* TM_USE_LBCAST_CONFIG */

#define TM_USE_MIME
#define TM_PROBE_INTERVAL   100
#define TM_MAX_PROBE        2
#ifndef TM_ERROR_CHECKING
#define TM_ERROR_CHECKING
#endif /* TM_ERROR_CHECKING */
#define TM_TRACE_LOGGING
#define TM_ERROR_LOGGING
#define TM_USE_TIMER_CB
#define TM_USE_REUSEADDR_LIST
#define TM_USE_REUSEPORT
#define TM_LOOP_TO_DRIVER
#define TM_KERNEL_WIN32_X86
#define TM_USE_NETSTAT
#define TM_USE_IPV6
#define TM_6_USE_MIP_MN
#define TM_6_USE_MIP_CN
#define TM_USE_IPV4
#define TM_USE_AUTO_IP
#ifndef TM_USE_RAM_FS /* not enough room in RAM FS for pcap file */
#define TM_USE_SNIFF
#endif /* TM_USE_RAM_FS */
#define TM_APP_CREATE_TIMER_THREAD
#define TM_USE_TEST_POINTS
#define TM_USE_SMTP
#define TM_USE_HTTPD
#define TM_USE_HTTPC
#define TM_USE_HTTPD_CGI
#define TM_USE_HTTPD_SSI
#define TM_USE_FTPD_SSL
#define TM_USE_POP3
#define TM_USE_IPSEC
#define TM_USE_PRIORITIES
#define TM_USE_IKEV1
#define TM_USE_IKEV2
#define TM_USE_PKI
#ifdef _DEBUG
#define TM_IKE_PHASE1_NULL_ENCRYPT
#define TM_IKE_PHASE1_REKEYING_TEST
#define TM_IKE_DEBUG
#else /* _DEBUG */
#ifdef TM_ERROR_CHECKING
/* undef TM_ERROR_CHECKING so that we don't generate memoryleak.dat */
#undef TM_ERROR_CHECKING
#endif /* TM_ERROR_CHECKING*/
#endif /* _DEBUG */
#define TM_SNMP_VERSION     2
#define TM_SNMP_CACHE
#define TM_IGMP
#define TM_USE_IGMPV3
#define TM_USE_UPNP_DEVICE
#define TM_6_USE_DHCP
#define TM_6_DHCP_USE_FQDN
#define TM_6_DHCP_USE_AUTH
/* SHA-384 and SHA-512 require TM_AALG_VAR_BLOCK_SIZE. */
#define TM_AALG_VAR_BLOCK_SIZE
#define TM_USE_SHA384
#define TM_USE_SHA512

/* #define TM_TAHI_TEST */
/* #define TM_TAHI_TEST_DNS */
/* #define TM_TAHI_TEST_ISAKMP */
/* #define TM_TAHI_TEST_DHCPV6 */
/* #define TM_TAHI_TEST_DNS */
/* #define TM_TAHI_TEST_ISAKMP */
/* #define TM_USE_IGMPV3 */

#ifdef TM_TAHI_TEST
#ifdef TM_TAHI_TEST_DNS
#define TM_TAHI_TEST_DNS_V4
#define TM_DNS_USE_SRV
#ifdef TM_TAHI_TEST_DNS_V4
#define TM_TAHI_TEST_INTF1_DNS     "192.168.1.20"
#define TM_TAHI_TEST_INTF2_DNS     "192.168.1.30"
#else /* TM_TAHI_TEST_DNS_V4 */
#undef TM_TAHI_TEST_INTF1_IP6_ADDR
#define TM_TAHI_TEST_INTF1_IP6_ADDR "3ffe:501:ffff:100::10"
#define TM_TAHI_TEST_INTF2_IP6_ADDR "3ffe:501:ffff:100::11"
#define TM_TAHI_TEST_INTF1_DNS      "3ffe:501:ffff:101::20"
#define TM_TAHI_TEST_INTF2_DNS      "3ffe:501:ffff:101::30"
#endif /* TM_TAHI_TEST_DNS_V4 */
#endif /* TM_TAHI_TEST_DNS */

#if (defined(TM_TAHI_TEST_DNS) || defined(TM_USE_IGMPV3))
#define TM_TAHI_TEST_INTF1_IP_ADDR "192.168.0.10"
#define TM_TAHI_TEST_INTF1_GW      "192.168.0.1"
#define TM_TAHI_TEST_INTF1_NETMASK "255.255.255.0"
#endif /* TM_TAHI_TEST_DNS || TM_USE_IGMPV3 */

#if (defined(TM_TAHI_TEST_DNS) || defined(TM_TAHI_TEST_DHCPV6))
#define TM_DNS_MAX_SOCKETS_ALLOWED 2
#endif /* TM_TAHI_TEST_DNS || TM_TAHI_TEST_DHCPV6 */

#define TM_TAHI_TEST_INTF_NUM     2
#define TM_TAHI_TEST_INTF2_IP_ADDR "30.0.0.201"
#define TM_TAHI_TEST_INTF2_GW      "30.0.0.201"
#define TM_TAHI_TEST_INTF2_NETMASK "255.255.255.0"
#define TM_TAHI_TEST_TN_IP_ADDR    "30.0.0.104"

#ifndef TM_TAHI_TEST_INTF2_DNS
#define TM_TAHI_TEST_INTF2_DNS     "0.0.0.0"
#endif /* TM_TAHI_TEST_INTF2_DNS */

#undef TM_USE_DHCP
#undef TM_6_USE_MIP_MN
#undef TM_6_USE_MIP_CN
#undef TM_SNMP_VERSION
#undef TM_SNMP_CACHE
#undef TM_USE_SNIFF
#undef TM_DEBUG_LOGGING
#undef TM_TRACE_LOGGING
#undef TM_ERROR_LOGGING
#undef TM_USE_NETSTAT
#undef TM_USE_TCP_REXMIT_CONTROL
#undef TM_USE_TCP_REXMIT_TEST

#ifdef TM_USE_IGMPV3
#define TM_IP_DEF_MULTICAST_TTL    1
#endif /* TM_USE_IGMPV3 */

#ifdef TM_TAHI_TEST_DHCPV6
#define TM_6_USE_DHCP
#define TM_6_DHCP_USE_AUTH
#undef TM_TAHI_TEST_INTF1_IP6_ADDR
#undef TM_TAHI_TEST_INTF2_IP6_ADDR
#else /* TM_TAHI_TEST_DHCPV6 */
#undef TM_6_USE_DHCP
#undef TM_6_DHCP_USE_FQDN
#undef TM_6_DHCP_USE_AUTH
#endif /* TM_TAHI_TEST_DHCPV6 */

#ifdef TM_TAHI_TEST_ISAKMP
#undef TM_6_USE_NUD
#undef TM_TAHI_TEST_INTF1_IP6_ADDR
#undef TM_TAHI_TEST_INTF2_IP6_ADDR
#define TM_USE_SEC_STAT
#define TM_PUBKEY_USE_RSA
/* #define TM_PUBKEY_USE_DSA */
#else /* TM_TAHI_TEST_ISAKMP */
#undef TM_USE_IKEV1
#undef TM_USE_IKEV2
#undef TM_USE_IPSEC
#undef TM_IKE_DEBUG
#undef TM_IPSEC_DEBUG
#undef TM_USE_PKI
#undef TM_USE_FTPD_SSL
#endif /* TM_TAHI_TEST_ISAKMP */

#endif /* TM_TAHI_TEST */

#endif /* TM_TRECK_DEMO */


#ifdef  TM_AUTO_TEST
//#define TM_USE_RAM_FS
//#define TM_USE_ROM_FS
//#define TM_DISABLE_TIMER_CACHE
//#define TM_USE_TIMER_CB

//#define TM_USE_SMTP_SSL
#define TM_USE_STOP_TRECK
#define TM_USE_IFNAMEINDEX
#define TM_USE_FILTERING
#define TM_USE_FILTERING_CONTIGUOUS_DATA
#define TM_6_USE_FILTERING
#define TM_6_USE_FILTERING_CONT_DATA
#define TM_6_USE_IP_FORWARD
#define TM_USE_TIMER_INTERVAL_QUERY
#define TM_6_USE_DHCP
#define TM_6_DHCP_USE_AUTH
#define TM_6_DHCP_USE_FQDN
#define TM_6_DEBUG_DHCP
#define TM_USE_MIME
#define TM_USE_SMTP
#define TM_USE_POP3
#define TM_USE_EAP
#define TM_DNS_USE_SRV
#define TM_USE_LOGGING_LEVELS
#define TM_LOG_ENB_ALL_LEVELS
#define TM_LOG_ENB_ALL_MODULES
#define TM_TRACE_LOGGING
#define TM_DEBUG_LOGGING
#define TM_ERROR_LOGGING
#define TM_USE_UPNP_DEVICE
#define TM_IGMP
#define TM_USE_IGMPV3
#define TM_USE_ETHER_8023_LL
#define TM_LOCK_NEEDED
#define TM_DEV_SEND_OFFLOAD
#define TM_DEV_RECV_OFFLOAD
#define TM_SNMP_CACHE
#define TM_MULTIPLE_CONTEXT
#define TM_KERNEL_VISUAL_X86
#define TM_USE_REUSEADDR_LIST
#define TM_USE_REUSEPORT
#define TM_USE_NETSTAT
#ifndef TM_USE_RAM_FS /* not enough room in RAM FS for pcap file */
#define TM_USE_SNIFF
#endif /* TM_USE_RAM_FS */
#define TM_USE_TEST_POINTS

/*
 * If TM_USE_CRYPTO_TEST is defined, tests will be run on the following
 * crypto algorithms, if defined:
 *
 * SHA-256 Hash and HMAC tests      TM_USE_SHA256
 * SHA-384 Hash and HMAC tests      TM_USE_SHA384
 * SHA-512 Hash and HMAC tests      TM_USE_SHA512
 * AES-XCBC-MAC Hash test           (TM_USE_AESXCBC && TM_USE_AES)
 * AES-CBC Encryption test          TM_USE_AES
 * AES-CTR Encryption test          TM_USE_AES
 */
#define TM_USE_CRYPTO_TEST
/*
 * Include TM_USE_FAILED_SHA_TESTS to run tests that need extra bits,
 * which are currently not supported
 */
/* #define TM_USE_FAILED_SHA_TESTS */

#define TM_USE_IPSEC
#define TM_USE_SEC_STAT
#define TM_USE_IKEV1
#define TM_USE_IKEV2
#define TM_USE_IKE_DPD
#define TM_USE_NATT
#define TM_USE_PKI
#define TM_USE_TEST_IKE_PKI
#define TM_IKE_PHASE1_REKEYING_TEST
#define TM_USE_IPV6
#define TM_6_USE_MIP_HA
#define TM_6_USE_MIP_MN
#define TM_6_USE_MIP_CN
#define TM_6_USE_RAW_SOCKET
#define TM_USE_PPP_MSCHAP
#define TM_USE_EAP
#define TM_USE_IPHC
#define TM_USE_RAW_SOCKET
#define TM_USE_HTTPD
#define TM_USE_HTTPD_CGI
#define TM_USE_HTTPD_SSI
#define TM_USE_SSL_SERVER
#define TM_USE_SSL_CLIENT
#define TM_USE_FTP_SSL
#define TM_USE_FTPD_SSL
#define TM_SOC_RECV_Q_DGRAMS  30
#define TM_USE_PPPOE_CLIENT
#define TM_USE_PPPOE_SERVER
#ifdef TM_SINGLE_INTERFACE_HOME
#undef TM_USE_IPV6
#endif /* TM_SINGLE_INTERFACE_HOME */
#endif /* TM_AUTO_TEST */

#ifdef  TM_KERNEL_REALOS_FR
/*
 * REALOS/FR
 * FUJITSU iTRON(ver3.0) for the MBxxxxx
 * It runs on a Fujitsu MBxxxxx platform
 */
#define TM_BIG_ENDIAN                      /* Use CPU Endian */
#define TM_TRECK_PREEMPTIVE_KERNEL         /* REAL/OS is Pre-Emptive Kernel */
#define TM_TICK_LENGTH  100                /* number of milliseconds per tick */
/*#define TM_BYPASS_ETHER_LL*/             /* Use Ethernet Module */
#define TM_TASK_RECV                       /* Use Recieve Task */
#define TM_USE_SHEAP                       /* Use Simple Heap Memory */
#ifdef  TM_USE_SHEAP
/* Simple Heap Memory Size is 256Kbyte */
#define TM_SHEAP_SIZE TM_UL(256)*TM_UL(1024)
#endif /* TM_USE_SHEAP */
#endif /* TM_KERNEL_REALOS_FR */

#ifdef TM_64BIT_LINUX_GCC
#define TM_USER_32BIT_TYPE
#define TM_USER_PTR_CAST_TO_INT
typedef unsigned int ttUser32Bit;
typedef signed int   ttUserS32Bit;
typedef uintptr_t ttUserPtrCastToInt;
#endif /* TM_64BIT_LINUX_GCC */

#ifdef TM_64BIT_WINDOWS
#define TM_USER_32BIT_TYPE
#define TM_USER_PTR_CAST_TO_INT
typedef unsigned int ttUser32Bit;
typedef signed int   ttUserS32Bit;
typedef __int64      ttUserPtrCastToInt;
#endif /* TM_64BIT_WINDOWS */

#ifdef TM_KERNEL_ELX_86
/*
 * ELX-86
 * This Kernel runs Treck TCP/IP as it's own task
 * It runs on a Intel x86 platform in Real mode
 * It also uses far memory(heap)
 */
#define TM_TRECK_TASK
#define TM_TICK_LENGTH  100 /* number of milliseconds per tick */
#define TM_INTEL_X86
#define TM_FAR far
/*
 * Define the ELX movsb for high speed copy
 */
void movsb(void TM_FAR *dest, const void TM_FAR *src, unsigned int len);
/* Use movsb for our tm_bcopy */
#define tm_bcopy(src, dest, count)  movsb( (void TM_FAR *)(dest),      \
                                           (const void TM_FAR *)(src), \
                                           (unsigned int)(count))

#endif /* TM_KERNEL_ELX_86 */

#ifdef TM_KERNEL_UCOS_XSCALE_RTASK
/*
 * uC/OS for the XSCALE Core of INTEL processors
 * uC/OS is preemptive and we use Treck TCP/IP as a shared library
 */
#define TM_KERNEL_UCOS_XSCALE
#define TM_LITTLE_ENDIAN
#define TM_USE_DRV_ONE_SCAT_SEND
#define TM_DEV_SEND_OFFLOAD
#define TM_DEV_RECV_OFFLOAD

#define TM_TICK_LENGTH  5 /* number of milliseconds per tick */
#define TM_OPTIMIZE_SPEED
/*
 * TM_ETHER_HW_ALIGN specifies what type of boundary (4 byte, 16 byte, etc)
 * the ethernet buffer is aligned on.  This is useful for devices that require
 * a DMA'ed recieve buffer to be aligned on a certain boundary.
 */
#define TM_ETHER_HW_ALIGN TM_UL(16) /* The XSCALE requires 16-byte alignment */

#define TM_BYPASS_ETHER_LL
/*
 * large heap size because of the large numbers of driver receive and
 * transmit descriptors.
 */
#define TM_SHEAP_SIZE TM_UL(512)*TM_UL(1024)
#define TM_USE_SHEAP

#endif /* TM_KERNEL_UCOS_XSCALE_RTASK */

#ifdef TM_KERNEL_UCOS_XSCALE
#define TM_TRECK_PREEMPTIVE_KERNEL
#define TM_TASK_RECV
#endif /*TM_KERNEL_UCOS_XSCALE */

#ifdef TM_KERNEL_UCOS_PPC_RTASK
/*
 * uC/OS for the PPC Core of the Motorola processors
 * PPC includes the MPC860
 * uC/OS is preemptive and we use Treck TCP/IP as a shared library
 */
#define TM_KERNEL_UCOS_PPC
#define TM_TASK_RECV
#endif

#ifdef TM_KERNEL_UCOS_PPC
#define TM_TRECK_PREEMPTIVE_KERNEL
#define TM_TICK_LENGTH  10 /* number of milliseconds per tick */
#define TM_MOTOROLA_PPC

/*
 * TM_ETHER_HW_ALIGN specifies what type of boundary (4 byte, 16 byte, etc)
 * the ethernet buffer is aligned on.  This is useful for devices that require
 * a DMA'ed recieve buffer to be aligned on a certain boundary.
 */
#define TM_ETHER_HW_ALIGN TM_UL(16) /* The MPC860 requires 16-byte alignment */

#define TM_BYPASS_ETHER_LL
/*
 * large heap size because of the large numbers of driver receive and
 * transmit descriptors.
 */
#define TM_SHEAP_SIZE TM_UL(256)*TM_UL(1024)
#define TM_USE_SHEAP
#endif /*TM_KERNEL_UCOS_PPC */

#ifdef TM_KERNEL_NP_UCOS_PPC_RTASK
/*
 * uC/OS for the PPC Core of the Motorola processors
 * PPC includes the MPC860
 * uC/OS is NON-Preemptive and we use Treck TCP/IP as a shared library
 */
#define TM_KERNEL_NP_UCOS_PPC
#define TM_TASK_RECV
#endif

#ifdef TM_KERNEL_NP_UCOS_PPC
#define TM_TRECK_NONPREEMPTIVE_KERNEL
#define TM_TICK_LENGTH  10 /* number of milliseconds per tick */
#define TM_MOTOROLA_PPC

/*
 * TM_ETHER_HW_ALIGN specifies what type of boundary (4 byte, 16 byte, etc)
 * the ethernet buffer is aligned on.  This is useful for devices that require
 * a DMA'ed recieve buffer to be aligned on a certain boundary.
 */
#define TM_ETHER_HW_ALIGN TM_UL(16) /* The MPC860 requires 16-byte alignment */
#define TM_BYPASS_ETHER_LL
/*
 * large heap size because of the large numbers of driver receive and
 * transmit descriptors.
 */
#define TM_SHEAP_SIZE TM_UL(256)*TM_UL(1024)
#define TM_USE_SHEAP
#endif /*TM_KERNEL_NP_UCOS_PPC */

#ifdef TM_KERNEL_NONE_H8S
/*
 * H8S, no kernel
 * CPU: H8S/2674R
 */
#define TM_HITACHI_H8S
#define TM_FAR
#define TM_TICK_LENGTH  10 /* number of milliseconds per tick */
#define TM_BYPASS_ETHER_LL
#define TM_USE_DRV_ONE_SCAT_SEND
#define TM_LOOP_TO_DRIVER
#define TM_USE_SHEAP
#define TM_SHEAP_SIZE TM_UL(128)*TM_UL(1024)
#define TM_TRECK_NO_KERNEL
#define TM_ETHER_POLLING_MODE
#define TM_INTERRUPT
#define TM_IGMP
#endif /*TM_KERNEL_NONE_H8S */

#ifdef TM_KERNEL_UCOS_RTASK
/*
 * uC/OS for the CPU32 Core of the Motorola 68K processors
 * CPU32 includes 68332 and 68360
 * uC/OS is preemptive and we use Treck TCP/IP as a shared library
 */
#define TM_KERNEL_UCOS_CPU32
#define TM_TASK_RECV
#endif

#ifdef TM_KERNEL_UCOS_CPU32
#define TM_TRECK_PREEMPTIVE_KERNEL
#define TM_TICK_LENGTH  10 /* number of milliseconds per tick */
#define TM_MOTOROLA_CPU32
#ifdef TM_QUICC
#ifndef TM_SOC_SEND_Q_BYTES
/* Default send queue size in bytes */
#define TM_SOC_SEND_Q_BYTES 16384
#endif /* TM_SOC_SEND_Q_BYTES */

#define TM_BYPASS_ETHER_LL
#endif /* TM_QUICC */

/*
 * large heap size because of the large numbers of driver receive and
 * transmit descriptors.
 */
#define TM_SHEAP_SIZE TM_UL(256)*TM_UL(1024)
#define TM_USE_SHEAP
#endif /*TM_KERNEL_UCOS_CPU32 */

#ifdef TM_KERNEL_NP_UCOS_RTASK
/*
 * Non Preemptive uC/OS for the CPU32 Core of the
 * Motorola 68K processors
 * CPU32 includes 68332 and 68360
 * uC/OS is non-preemptive and we use Treck TCP/IP as a shared library
 */
#define TM_KERNEL_NP_UCOS_CPU32
#define TM_TASK_RECV
#endif

#ifdef TM_KERNEL_NP_UCOS_CPU32
#define TM_TRECK_NONPREEMPTIVE_KERNEL
#define TM_TICK_LENGTH  10 /* number of milliseconds per tick */
#define TM_MOTOROLA_CPU32
#ifdef TM_QUICC
#ifndef TM_SOC_SEND_Q_BYTES
/* Default send queue size in bytes */
#define TM_SOC_SEND_Q_BYTES 16384
#endif /* TM_SOC_SEND_Q_BYTES */

#define TM_BYPASS_ETHER_LL
#endif /* TM_QUICC */
/*
 * large heap size because of the large numbers of driver receive and
 * transmit descriptors.
 */
#define TM_SHEAP_SIZE TM_UL(256)*TM_UL(1024)
#define TM_USE_SHEAP
#endif /*TM_KERNEL_NP_UCOS_CPU32 */

#ifdef TM_KERNEL_AMX_CPU32
#define TM_TASK_RECV
#define TM_TRECK_PREEMPTIVE_KERNEL
#define TM_TICK_LENGTH  10 /* number of milliseconds per tick */
#define TM_MOTOROLA_CPU32
#ifdef TM_QUICC            /* defined in sds360cc.bat */
#ifndef TM_SOC_SEND_Q_BYTES
/* Default send queue size in bytes */
#define TM_SOC_SEND_Q_BYTES 16384
#endif /* TM_SOC_SEND_Q_BYTES */

#define TM_BYPASS_ETHER_LL
#endif /* TM_QUICC */

/*
 * large heap size because of the large numbers of driver receive and
 * transmit descriptors.
 */
#define TM_SHEAP_SIZE TM_UL(256)*TM_UL(1024)
/* Use AMX memory pool code. Do not use our SHEAP (Simple HEAP) */
#endif /*TM_KERNEL_AMX_CPU32 */


#ifdef TM_KERNEL_AMX_X86
#define TM_TASK_RECV
#define TM_TRECK_PREEMPTIVE_KERNEL
#define TM_TICK_LENGTH  55 /* number of milliseconds per tick */
#define TM_INTEL_X86
#endif /*TM_KERNEL_AMX_X86 */


#ifdef TM_KERNEL_DOS_X86
#define TM_TICK_LENGTH 55
#define TM_TRECK_NO_KERNEL
#define TM_INTEL_X86
/* #define TM_USE_SHEAP */
/* #define TM_SHEAP_SIZE TM_UL(54)*TM_UL(1024) */
/* #define TM_BYPASS_ETHER_LL */
#define TM_FAR far
#endif /* TM_KERNEL_DOS_X86 */


#ifdef TM_KERNEL_UCOS_X86
/*
 * uC/OS for the Intel Family
 * uC/OS is preemptive and we use Treck TCP/IP as a shared library
 */
#define TM_TICK_LENGTH  55 /* number of milliseconds per tick */
#define TM_TRECK_PREEMPTIVE_KERNEL
#define TM_TASK_RECV
#define TM_INTEL_X86
#define TM_OPTIMIZE_SIZE
#define TM_BYPASS_ETHER_LL
#define TM_SHEAP_SIZE TM_UL(29)*TM_UL(1024)
#define TM_USE_SHEAP
#define TM_FAR far
#endif /*TM_KERNEL_UCOS_X86 */

#ifdef TM_KERNEL_WIN32_X86
/*
 * Windows 32 for the x86 family, 
 * Treck runs as multi-thread windows application.
 */
/* We now use the EMU_UNIX_FS instead */
#define TM_USE_EMU_UNIX_FS
#define TM_RT_TIMER_RESOLUTION TM_UL(1000) /* interop test: 1 sec resolution */
#define TM_TICK_LENGTH  10 /* number of milliseconds per tick */
#define TM_TRECK_PREEMPTIVE_KERNEL
#define TM_TASK_RECV
#define TM_LITTLE_ENDIAN
#define TM_X86_ASM_SWAP
#define TM_OPTIMIZE_SPEED
#define TM_BYPASS_ETHER_LL
#define TM_SHEAP_SIZE   TM_UL(512)*TM_UL(1024)
#define TM_USE_SHEAP
/* To check for memory corruption */
#define TM_SHEAP_MARK_MEMORY
/* To check for end of allocated memory overwrite */
#define TM_DISABLE_DYNAMIC_MEMORY
#define TM_BSD_CLASH
#define TM_FAR
#define TM_USE_CLIB
#undef TM_INDRV_INLINE_SEND_RECV 
#endif /* TM_KERNEL_WIN32_X86 */

 
#ifdef TM_KERNEL_VISUAL_X86
/*
 * Loop back testing using Microsoft Visual C++
 */
#define TM_USE_EMU_UNIX_FS
#define TM_FAR
#define TM_TICK_LENGTH  10 /* number of milliseconds per tick */
#define TM_TRECK_TASK /* Turbo Treck run in a single task */
#define TM_OPTIMIZE_SPEED
#define TM_LITTLE_ENDIAN
#define TM_X86_ASM_SWAP
#define TM_BYPASS_ETHER_LL
#define TM_SHEAP_SIZE   TM_UL(512)*TM_UL(1024)
#define TM_USE_SHEAP
/* To check for memory corruption */
#define TM_SHEAP_MARK_MEMORY
/* To check for end of allocated memory overwrite */
#define TM_DISABLE_DYNAMIC_MEMORY
#define TM_BSD_CLASH
#define TM_USE_CLIB
#define TM_USE_DRV_ONE_SCAT_SEND /* Allow more efficient driver send */
/* Allow incoming scattered device driver recv data */
#define TM_USE_DRV_SCAT_RECV 
/* 
 * More efficient IP fragmentation when device driver supports scattered
 * send
 */
#define TM_IP_FRAGMENT_NO_COPY 
#endif /* TM_KERNEL_VISUAL_X86 */

/* Linux Raw_socket Driver */
/* #define TM_KERNEL_LINUX_APP */

#ifdef TM_KERNEL_LINUX_APP
/* For swap macro definitions, Linux socket calls */
#define TM_KERNEL_LINUX
#define TM_INTEL_GNU_ASM_SWAP
#define TM_FAR
#define TM_TICK_LENGTH        10 /* number of milliseconds per tick */
#define TM_TRECK_PREEMPTIVE_KERNEL
#define TM_OPTIMIZE_SPEED
#define TM_TASK_RECV
#define TM_LITTLE_ENDIAN
#define TM_BYPASS_ETHER_LL
#define TM_SHEAP_SIZE   TM_UL(256)*TM_UL(1024)
#define TM_USE_SHEAP
#define TM_DYNAMIC_CREATE_SHEAP
#define TM_BSD_CLASH
/* following define is for SNMPD Trace*/
#define TM_DEBUG_LOGGING
#define TM_TRACE_LOGGING
#define TM_ERROR_LOGGING
#define TM_DEF_MAX_LOG_MSG_LEN 4096
#define TM_DEF_NUM_LOG_MSGS    10
#endif /* TM_KERNEL_LINUX_APP */

#ifdef TM_KERNEL_NP_NUCLEUSRTX_C3X
/*
 * Nucleus RTX for the TI TMS320C3x Family
 * Running Nucleus RTX in non-preemptive mode
 */
#define TM_TRECK_NONPREEMPTIVE_KERNEL
#define TM_TICK_LENGTH  10
#define TM_TMS320_C3
#define TM_BYPASS_ETHER_LL
#endif /* TM_KERNEL_NUCLEUSRTX_C3X */

#ifdef TM_KERNEL_NONE_ARM7
/*
 * ARM7 processor, no RTOS
 */
#define TM_ARM7
#define TM_TICK_LENGTH 10
#define TM_USE_SHEAP
#define TM_SHEAP_SIZE TM_UL(128)*TM_UL(1024)
#define TM_TRECK_NO_KERNEL
#define TM_BYPASS_ETHER_LL
#endif /* TM_KERNEL_NONE_ARM7 */

#ifdef TM_KERNEL_THREADX_ARM7
/*
 * ThreadX for the ARM7
 */
#define TM_ARM7
#define TM_TICK_LENGTH 10
#define TM_TRECK_PREEMPTIVE_KERNEL
#define TM_TASK_RECV
#define TM_BYPASS_ETHER_LL
#endif /* TM_KERNEL_THREADX_ARM7 */

#ifdef TM_KERNEL_THREADX_CPU32
/*
 * ThreadX for the Motorola CPU32 core (68332 & 68360)
 */
#define TM_MOTOROLA_CPU32
#define TM_TICK_LENGTH 10
#define TM_TRECK_PREEMPTIVE_KERNEL
#define TM_TASK_RECV
#define TM_BYPASS_ETHER_LL
#endif /* TM_KERNEL_THREADX_CPU32 */

#ifdef TM_KERNEL_TI_DSP_BIOS
/*
 * TI DSP BIOS 
 */
#define TM_TICK_LENGTH 10
#define TM_TRECK_PREEMPTIVE_KERNEL
#define TM_TASK_RECV
#define TM_TASK_XMIT
#define TM_USE_CLIB
#define TM_OPTIMIZE_SIZE
#define TM_OPTIMIZE_SPEED
#define TM_BYPASS_ETHER_LL
#define TM_IP_FRAGMENT_NO_COPY
/* Default recv queue size in bytes */
#define TM_SOC_RECV_Q_BYTES  16384
/* Default send queue size in bytes */
#define TM_SOC_SEND_Q_BYTES  16384
#endif /* TM_KERNEL_TI_DSP_BIOS */

#ifdef TM_KERNEL_NONE_EZ80
/*
 * EZ80 evaluation board, no kernel
 */
#define TM_EZ80
#define TM_USE_SHEAP
#define TM_SHEAP_SIZE TM_UL(128)*TM_UL(1024)
#define TM_TRECK_NO_KERNEL
#define TM_BYPASS_ETHER_LL
#define TM_Z80L92_ASM_OCS
#endif /* TM_KERNEL_NONE_EZ80 */

#ifdef TM_KERNEL_RZK_EZ80
/*
 * EZ80 evaluation board, RZK(RTOS)
 */
#define TM_EZ80
#define TM_USE_SHEAP
#define TM_SHEAP_SIZE TM_UL(128)*TM_UL(1024)
#define TM_TRECK_PREEMPTIVE_KERNEL
#define TM_TASK_RECV
#define TM_Z80L92_ASM_OCS
#define TM_USE_KERNEL_CRITICAL_STATUS

#ifndef TM_USER_KERNEL_CRITICAL_STATUS_TYPE
#define TM_USER_KERNEL_CRITICAL_STATUS_TYPE
typedef unsigned int ttUserKernelCriticalStatus;
#endif /* TM_USER_KERNEL_CRITICAL_STATUS_TYPE */

extern unsigned int RZKDisableInterrupts(void);
extern void RZKEnableInterrupts(unsigned int interruptsMask);

#define tm_kernel_set_critical \
            tvKernelCriticalStatus = RZKDisableInterrupts()
#define tm_kernel_release_critical \
            RZKEnableInterrupts(tvKernelCriticalStatus)

#endif /*TM_KERNEL_NONE_EZ80 */


#ifdef TM_KERNEL_UCOS_MIPS
/*
 * MIPS4kc, Malta evaluation board, ucos kernel
 * gnu tools: sde-mips
 */
#define TM_KERNEL_MIPS4k
#define TM_TRECK_PREEMPTIVE_KERNEL
#define TM_TASK_RECV
#define TM_TICK_LENGTH        10 
#define TM_GNUMIPS4K_ASM_CRITICAL
/* define ENDIAN in makefile for little and big */
#define TM_BYPASS_ETHER_LL
#define TM_SHEAP_SIZE   TM_UL(256)*TM_UL(1024)
#define TM_USE_SHEAP
#define printf tfMipsPrintf
#endif /* TM_KERNEL_UCOS_MIPS */


#ifdef TM_KERNEL_UCOS_SH2
/*
 * HITACHI SH2 Processor - uCOS-II ( big endian )
 *  SH7616 (SH2-DSP) evaluation board.
 */
#define TM_HITACHI_SH2
#define TM_TASK_RECV
#define TM_TASK_SEND
#define TM_TASK_XMIT
#define TM_TICK_LENGTH  10   /* number of milliseconds per tick */
#define TM_TRECK_PREEMPTIVE_KERNEL
#define TM_OPTIMIZE_SPEED
#define TM_BYPASS_ETHER_LL
#define TM_USE_SHEAP
#define TM_SHEAP_SIZE ( 128UL * 1024UL )
#define TM_USE_DRV_ONE_SCAT_SEND
#endif /* TM_KERNEL_UCOS_SH2 */

#ifdef TM_KERNEL_CMX_C16X
/*
 * Infineon  C166 Processor - CMX RTOS
 * Vorne 87AL board. 
 */
#define TM_INFINEON_C166
#define TM_TICK_LENGTH 10
#define TM_TRECK_PREEMPTIVE_KERNEL
#define TM_BYPASS_ETHER_LL
#define TM_DISABLE_TCP_SACK
#define TM_USE_DRV_ONE_SCAT_SEND
#define TM_OPTIMIZE_SPEED

#define TM_TASK_RECV
/* #define TM_TASK_XMIT */

/* Dymanically allocate Sheap as pages of 16kbytes */
#define TM_USE_SHEAP
#define TM_DYNAMIC_CREATE_SHEAP
#define TM_SHEAP_SIZE       (TM_UL(128)*TM_UL(1024))
#define TM_SHEAP_PAGE_SIZE  (TM_UL(64)*TM_UL(1024))
#define TM_SHEAP_NUM_PAGE   (TM_SHEAP_SIZE/TM_SHEAP_PAGE_SIZE)

#define TM_FAR huge
#define TM_GLOBAL_QLF huge
#define TM_CONST_QLF huge
#define TM_INTERRUPT

#ifdef TM_ERROR_CHECKING
#undef TM_ERROR_CHECKING
#endif /* TM_ERROR_CHECKING */

#define TM_SINGLE_INTERFACE_HOME

/*inlcude the KEIL compiler library that supports huge argement type */
#include "trkeil.h"
#include <string.h>
#include <ctype.h>

#endif /* TM_KERNEL_CMX_C16X */

#ifdef TM_KERNEL_NONE_RCM3200
/*
 * Rabbit Core Module 3200, no kernel
 */
#define TM_RCM3200

#define TM_LITTLE_ENDIAN

#define TM_USE_DHCP
#define TM_DNS_USE_SRV
#define TM_IGMP
#define TM_USE_IPV6
/* #define TM_USE_IPSEC */
/* #define TM_USE_IKEV1 */

/* Prevent definition of TM_USE_SW_CRYPTOENGINE later */
#define TM_X_USE_SW_CRYPTOENGINE

/* Prevent link error in function tfMd5Transform() */
#define TM_DISABLE_TCP_128BIT_RANDOM_ISS

#define TM_TICK_LENGTH 10
#define TM_TRECK_TASK
#define TM_BYPASS_ETHER_LL
#define TM_DISABLE_TCP_SACK
#define TM_OPTIMIZE_SIZE

#define TM_USE_SHEAP
#define TM_DYNAMIC_CREATE_SHEAP
#define TM_SHEAP_SIZE       (TM_UL(30)*TM_UL(1024))
#define TM_SHEAP_PAGE_SIZE  (TM_UL(30)*TM_UL(1024))

#define TM_FAR
#define TM_CODE_FAR
#define TM_GLOBAL_QLF
#define TM_CONST_QLF
#define TM_INTERRUPT
#define TM_NEARCALL _nearcall

/* The following definitions are for debugging only */
/* #define TM_LOOP_TO_DRIVER */
/* #undef TM_INDRV_INLINE_SEND_RECV */

#ifdef TM_USE_PPP
#undef TM_USE_PPP
#endif /* TM_USE_PPP */

#ifdef TM_ERROR_CHECKING
#undef TM_ERROR_CHECKING
#endif /* TM_ERROR_CHECKING */

#include <string.h>
#include <ctype.h>

#endif /* TM_KERNEL_NONE_RCM3200 */

#ifdef TM_KERNEL_NONE_S56XX
/*
 * Stretch S56XX with no OS
 */
/* Assembly critical sections */
extern unsigned rval;
#ifndef XTSTR
#define _XTSTR(x)   # x
#define XTSTR(x)    _XTSTR(x)
#endif
#define tm_kernel_set_critical      rval = \
    ({ unsigned __tmp; \
       __asm__ __volatile__(    "rsil   %0, " XTSTR(15) "\n" \
                        : "=a" (__tmp) : : "memory" ); \
            __tmp;})
#define tm_kernel_release_critical \
    do{ unsigned __tmp = (rval); \
            __asm__ __volatile__(   "wsr    %0, " XTSTR(PS) " ; rsync\n" \
                        : : "a" (__tmp) : "memory" ); \
        }while(0)
/* Custom checksum in source\kernel\none\s56xx\checksum.c */
#define TM_CUSTOM_OCS
#define TM_USE_CLIB
#define TM_BSD_CLASH
#define TM_USE_SHEAP
#define TM_SHEAP_SIZE (TM_UL(255)*TM_UL(1024))
#define TM_DYNAMIC_CREATE_SHEAP
#define TM_SHEAP_NUM_PAGE 1
#define TM_LITTLE_ENDIAN
#define TM_TICK_LENGTH  10
#define TM_TRECK_NO_KERNEL
#endif /* TM_KERNEL_NONE_S56XX */


#ifdef TM_KERNEL_NONE_XILINX_MICROBLAZE
#define TM_TRECK_NO_KERNEL  
#define TM_BSD_CLASH
#define TM_BIG_ENDIAN      
#define TM_TICK_LENGTH  10 
#define TM_USE_SHEAP 
#define TM_SHEAP_SIZE TM_UL(4)*TM_UL(1024)*TM_UL(1024)  
#define TM_OPTIMIZE_SPEED 
#define TM_BYPASS_ETHER_LL 
#define TM_USE_DRV_ONE_SCAT_SEND
#define TM_NEED_ETHER_32BIT_ALIGNMENT
#define TM_USE_DRV_SCAT_RECV
#define TM_ETHER_HW_TX_ALIGN 4
#define TM_DEV_SEND_OFFLOAD 
/* These are for debugging */
/* #define TM_ERROR_CHECKING */
/* #define TM_DISABLE_DYNAMIC_MEMORY */
/* #define TM_SHEAP_MARK_MEMORY */
#endif /* TM_KERNEL_NONE_XILINX_MICROBLAZE */

#ifdef TM_KERNEL_NONE_XILINX_POWERPC
#define TM_TRECK_NO_KERNEL
#define TM_BSD_CLASH
#define TM_BIG_ENDIAN      
#define TM_TICK_LENGTH  10
#define TM_USE_SHEAP
#define TM_SHEAP_SIZE TM_UL(4)*TM_UL(1024)*TM_UL(1024) 
#define TM_OPTIMIZE_SPEED 
#define TM_BYPASS_ETHER_LL 
#define TM_DEV_SEND_OFFLOAD 
#define TM_USE_DRV_ONE_SCAT_SEND
/* This is for debugging */
/* #define TM_ERROR_CHECKING */
#endif /* TM_KERNEL_NONE_XILINX_POWERPC */


#ifdef TM_INTEL_X86
/*
 * Intel x86
 */
#define TM_LITTLE_ENDIAN
#ifdef __BORLANDC__
/* In line assembly critical section */
#define TM_X86_ASM_CRITICAL
/* In line network byte ordering */
#define TM_X86_ASM_SWAP
/* Use assembly checksum routine */
#define TM_BORLAND_ASM_OCS
#endif /* __BORLANDC__ */
#ifdef _MSC_VER
/* In line assembly critical section */
#define TM_X86_ASM_CRITICAL
/* In line network byte ordering */
#define TM_X86_ASM_SWAP
#endif /* _MSC_VER */
#endif /* TM_INTEL_X86 */

#ifdef TM_INTEL_XSCALE
/*
 * Intel XScale (80200)
 */
#define TM_LITTLE_ENDIAN
/* Use assembly checksum routine */
#define TM_GNUXSCALE_ASM_OCS
#endif /* TM_INTEL_XSCALE */

#ifdef TM_MOTOROLA_PPC
/*
 * Motorola Power PC Core
 */
#undef TM_LITTLE_ENDIAN
#ifdef TM_COMPILER_DDI_PPC
/* In line assembly critical section */
#define TM_DDIPPC_ASM_CRITICAL
#define TM_DDIPPC_ASM_OCS
#endif /* TM_COMPILER_DDI_PPC */

#ifdef TM_COMPILER_GHS_PPC
#define TM_GHSPPC_ASM_CRITICAL
#define TM_GHSPPC_ASM_OCS
#endif /* TM_COMPILER_GHS_PPC */

#ifdef TM_COMPILER_GNU_PPC
#define TM_GNUPPC_ASM_CRITICAL
#define TM_GNUPPC_ASM_OCS
#endif /* TM_COMPILER_GNU_PPC */
#endif /* TM_MOTOROLA_PPC */

#ifdef TM_MOTOROLA_CPU32
/*
 * Motorola CPU32 Core (68332 & 68360)
 */
#undef TM_LITTLE_ENDIAN
#ifdef TM_COMPILER_SDS
/* In line assembly critical section */
#define TM_SDS68K_ASM_CRITICAL
#define TM_SDS68K_ASM_OCS
#endif /* TM_COMPILER_SDS */

#ifdef TM_COMPILER_MRI
#define TM_MRI_ASM_CRITICAL
#endif /* TM_COMPILER_MRI */

#endif /* TM_MOTOROLA_CPU32 */

#ifdef TM_COMPILER_CW_5282
/* Metrowerks CodeWarrior for Motorola 5282 (Coldfire) */
#define TM_CW5282_ASM_OCS
#endif /* TM_COMPILER_CW_5282 */

#ifdef TM_MOTOROLA_68K
/*
 * Motorola 68000 (EC) Core
 */
#undef TM_LITTLE_ENDIAN

#ifdef TM_COMPILER_SDS
#define TM_SDS68K_ASM_CRITICAL
#endif /* TM_COMPILER_SDS */

#ifdef TM_COMPILER_MRI
#define TM_MRI_ASM_CRITICAL
#endif /* TM_COMPILER_MRI */

#endif /* TM_MOTOROLA_68K */

#ifdef TM_MOTOROLA_COLDFIRE
/* Motorola Coldfire - MCF5282, MCF5485 */
#undef TM_LITTLE_ENDIAN
#define TM_ETHER_HW_ALIGN       16 /* ethernet receive buffer alignment */
#endif /* TM_MOTOROLA_COLDFIRE */
    
#ifdef TM_HITACHI_H8S
/*
 * Hitachi H8S
 */
#undef TM_LITTLE_ENDIAN
#endif /* TM_HITACHI_H8S */

#ifdef TM_HITACHI_SH2
/*
 * Hitachi SH2
 */
#define TM_BIG_ENDIAN
#endif /* TM_HITACHI_SH2 */


#ifdef TM_INFINEON_C166
/*
 * TM_INFINEON_C166
 */
#define TM_LITTLE_ENDIAN
#define TM_C166_OCS
#endif /* TM_INFINEON_C166 */


#ifdef TM_ARM7
/*
 * ARM7 Core
 */
#define TM_LITTLE_ENDIAN

#ifdef TM_COMPILER_GHS_ARM
#define TM_GHSARM7_ASM_CRITICAL
#define TM_GHSARM7_ASM_OCS
#endif /* TM_COMPILER_GHS_ASM */

#endif /* TM_ARM7 */

#ifdef TM_TMS320_C3_BIG_ENDIAN
/* TI DSP C3 like processor but with byte order reversed */
#define TM_TMS320_C3
#endif /* TM_TMS320_C3 */

#ifdef TM_TMS320_C3
/*
 * Texas Instruments TMS320C3x Core 
 *
 * The TMS320C3X processor accesses memory in 32 bit chunks.
 * But, the ethernet driver wants to set the data in the words
 * in a little endian way with the least significant data byte in
 * the most significant byte of the word, etc.  Therefore, we
 * define little endian for network to host conversions.
 */
#undef TM_LITTLE_ENDIAN

/*
 * By default, the sockets API expects data to be byte packed.  This behavior
 * can be changed by enabling the SO_UNPACKEDDATA option per socket.  Uncomment
 * the line below to enable this option for all sockets.
 */
/* #define TM_SOC_DEF_OPTIONS SO_UNPACKEDDATA */

#define TM_USER_32BIT_TYPE
typedef unsigned int ttUser32Bit;
typedef signed int   ttUserS32Bit;

#define TM_UL(x) x##U
#define TM_L(x) x
#endif /* TM_TMS320_C3 */


/*
 * Texas Instruments TMS320C5x Core
 */
#ifdef TM_TMS320_C5

#undef TM_LITTLE_ENDIAN
/*
 * By default, the sockets API expects data to be byte packed.  This behavior
 * can be changed by enabling the SO_UNPACKEDDATA option per socket.  Uncomment
 * the line below to enable this option for all sockets.
 */
/* #define TM_SOC_DEF_OPTIONS SO_UNPACKEDDATA */

#endif /* TM_TMS320_C5 */

#ifdef TM_TMS320_C6
/*
 * Texas Instruments TMS320C6000 core
 */
#define TM_USER_32BIT_TYPE
typedef unsigned int ttUser32Bit;
typedef signed int   ttUserS32Bit;

#define TM_UL(x) x##U
#define TM_L(x) x

#define TM_TI_C6000_ASM_OCS
#endif /* TM_TMS320_C6 */


#ifdef TM_EZ80
/*
 * Zilog eZ80 processor
 */
#define TM_FAR
#define TM_TICK_LENGTH  55 /* number of milliseconds per tick */
#define TM_LITTLE_ENDIAN
#endif /* TM_EZ80 */

#ifndef TM_TICK_LENGTH
#define TM_TICK_LENGTH 0
#endif /* TM_TICK_LENGTH */


#ifdef TM_USE_IPV6
/*
 * TM_6_IP_FRAGMENT macro. Enable support for IPv6 fragmentation.
 * Comment out to save code space when IPv6 fragmentation is 
 * not needed.
 */
#define TM_6_IP_FRAGMENT

/*
 * TM_6_IP_REASSEMBLY macro. Enable support for IPv6 reassembly.
 * Comment out to save code space when IPv6 reassembly is
 * not needed.
 */
#define TM_6_IP_REASSEMBLY

/*
 * TM_6_USE_RFC3484 macro is used to enable Default Address Selection
 * of IPv6 source and destination Addresses 
 */
/* #define TM_6_USE_RFC3484 */

/*
 * TM_6_USE_IP_FORWARD macro. Enable IPv6 forwarding code. Comment out to save
 * code space when IPv6 forwarding is not needed.
 */
/* #define TM_6_USE_IP_FORWARD */

/* Uncomment the TM_6_IP_SRC_ERROR_CHECKING macro if you want validation of
   the source address of received IPv6 packets. */
/* #define TM_6_IP_SRC_ERROR_CHECKING */

/*
 * Uncomment the TM_6_DISABLE_PMTU_DISC macro, if you want to disable automatic
 * IPv6 Path MTU discovery.
 */
/* #define TM_6_DISABLE_PMTU_DISC */
    

/* 
 * TM_6_USE_NUD macro. Enable IPv6 Neighbor Unreachability Detection code.
 * Comment out to save code space when IPv6 Neighbor Unreachability Detection
 * is not needed.
 */
#define TM_6_USE_NUD

/* 
 * TM_6_USE_MLD macro. Enable IPv6 Multicast Listener Discovery code. Comment
 * out to save code space when IPv6 Multicast Listener Discovery is not needed.
 */
#define TM_6_USE_MLD

/* 
 * TM_6_USE_MULTICAST_LOOP macro. Enable support for the IPV6_MULTICAST_LOOP
 * socket option described in RFC3493:
 *      "If a multicast datagram is sent to a group to which the sending
 *       host itself belongs (on the outgoing interface), a copy of the
 *       datagram is looped back by the IP layer for local delivery."
 * If TM_6_USE_MULTICAST_LOOP is undefined, outgoing IPv6 multicast packets
 * are never looped back unless the user's device driver explicitly does so.
 * If TM_6_USE_MULTICAST_LOOP is defined, loopback of outgoing IPv6 multicast
 * packets is controlled by the IPV6_MULTICAST_LOOP socket option, with
 * loopback being the default action (as stated in RFC3493).
 */
/* #define TM_6_USE_MULTICAST_LOOP */

/* 
 * TM_6_USE_RFC3590 macro.  Enable Source Address Selection for the Multicast 
 * Listener Discovery (MLD) Protocol as defined in RFC 3590.
 */
/* #define TM_6_USE_RFC3590 */

/* 
 * TM_6_USE_MLDV2 macro. Enable IPv6 Multicast Listener Discovery code. Comment
 * out to save code space when IPv6 Multicast Listener Discovery is not needed.
 */
/* #define TM_6_USE_MLDV2 */

/* 
 * TM_6_ENABLE_MLDV1 macro. Enable IPv6 Multicast Listener Discovery Version1
 * Interoperability code in Version2. Comment out to save code space when 
 * IPv6 Multicast Listener Discovery Version1 Interoperability is not needed.
 */
/* #define TM_6_ENABLE_MLDV1 */

/*
 * TM_6_USE_DAD macro. Enable IPv6 Duplicate Address Detection code. Comment
 * out to save code space when IPv6 Duplicate Address Detection is not needed.
 */
#define TM_6_USE_DAD

/* 
 * TM_6_USE_PREFIX_DISCOVERY macro. Enable IPv6 Prefix Discovery code, part
 * of stateless address auto-configuration. Comment out to save code space
 * when IPv6 Prefix Discovery is not needed.
 */
#define TM_6_USE_PREFIX_DISCOVERY

/*
 * TM_6_ENABLE_ONLINK_ASSUMPTION macro.
 * Neighbor Discovery for IPv6 [RFC4861] replaces [RFC2461] and removes the
 * following text from section 5.2 for reasons explained in [RFC4943]:
 *      "If the Default Router List is empty, the sender assumes that the
 *       destination is on-link."
 * Uncomment the following macro to undo this change and re-enable the
 * on-link assumption code.
 */
/* #define TM_6_ENABLE_ONLINK_ASSUMPTION */

/*
 * Uncomment the following line to enable strict RFC 4861 conformance.
 * The only effect this has is that it prevents Treck from sending
 * Router Solicitation messages when a prefix address expires.
 */
/* #define TM_USE_RFC4861_STRICT */

/* 
 * TM_6_USE_MIP_CN macro. Enable Mobile IPv6 correspondent node code.
 * Comment out to save code space when Mobile IPv6 correspondent node
 * functionality is not needed.
 */
/* #define TM_6_USE_MIP_CN */

/* 
 * TM_6_USE_MIP_MN macro. Enable Mobile IPv6 mobile node code. Comment out
 * to save code space when Mobile IPv6 mobile node functionality is not
 * needed.
 */
/* #define TM_6_USE_MIP_MN */

/*
 * TM_6_MIP_DRAFT24 macro. Use Mobility Header protocol ID and ICMPv6
 * message types per draft-ietf-mobileip-ipv6-24.txt, rather than IANA
 * assignments.
 */
/* #define TM_6_MIP_DRAFT24 */

/* 
 * TM_6_USE_MIP_HA macro. Enable Mobile IPv6 home agent code. Comment out
 * to save code space when Mobile IPv6 mobile node functionality is not
 * needed.
 * NOTE: As of IPv6 phase II release, this is only a HA lite that dones't 
 *       implement the whole functionality of a MIP6 home agent
 */
/* #define TM_6_USE_MIP_HA */

/* 
 * TM_6_USE_MIP_RO macro. Enable Mobile IPv6 route optimization. Comment
 * out to save code space when Mobile IPv6 route optimization is not needed.
 * ([MIPV6_18++].R14.4.5:10, [MIPV6_18++]R14.4.5:20)
 */
#define TM_6_USE_MIP_RO

/* 
 * TM_6_MN_DISABLE_HOME_DETECT macro. Disable Mobile IPv6 Mobile Node home
 * detection/return to home functionality. If you know that the mobile node
 * never returns home, then you can define this macro to save some code
 * space.       
 */
/* #define TM_6_MN_DISABLE_HOME_DETECT */

/*
 * TM_6_USE_MIP_RA_RTR_ADDR macro. When enabled, the mobile node uses the
 * Router Address option (when available) in received Router Advertisement
 * messages to determine if a Router Advertisement is from the current
 * default router.
 */
#define TM_6_USE_MIP_RA_RTR_ADDR

/*
 * TM_6_USE_RFC3879 macro. When enabled, Deprecates Site Local Addresses
 * with FEC0/10:: prefix
 */
#define TM_6_USE_RFC3879
#endif /* TM_USE_IPV6 */


/*
 * For phase 1 negotiation, Elmic IKE supports digital signature
 * authentication method. It includes RSA/DSA algorithm, PEM/DER 
 * certification, X509v3 and ASN1 format, multi CA and CRL. TM_USE_PKI
 * macro shoule be defined in order to use this part. Whenever you define
 * TM_USE_SSL_CLIENT or TM_USE_SSL_SERVER, this macro is automatically
 * defined
 */
/* #define TM_USE_PKI */


#ifdef TM_USE_IPSEC
/*
 * IPsec and IKE Options
 */

/* 
 * Define TM_IPSEC_DEBUG to debug encryption/authentication in IPsec
 */
/* #define TM_IPSEC_DEBUG */

/* 
 * Define TM_IKE_DEBUG to debug IKE
 */
/*#define TM_IKE_DEBUG*/
 
/* 
 * For IPsec sequence, define the TM_IPSEC_USE_ANTIREPLAY to check the 
 * sequence number. Even this macro is defined, in the following cases,
 * we don't do antireplay checking. 1) not authenticated ESP packet
 * 2)Elmic recommends not to define anti-replay in manual keying
 * (if we do anti-replay in manual keying, all cycled
 * sequence packets will be dropped, unless the user manually update
 * the SA before the cycle happens.)
 */
#define TM_IPSEC_USE_ANTIREPLAY

/*
 * If TM_IPSEC_TFC_PADDING is defined, we add TFC padding to ESP header.
 */
/* #define TM_IPSEC_TFC_PADDING */

/* 
 * If we don't want to do ICMP incoming policy check at all, define this
 */
/* #define TM_IPSEC_INCOMING_ICMP_BYPASS */

/* 
 * If TM_IPSEC_USE_64BIT_SEQ_NO is defined, we add Seq# (high-order bits).
 */
/* #define TM_IPSEC_USE_64BIT_SEQ_NO */

/*
 * If TM_USE_IPSEC_IPCOMP is defined, we add IPComp header.
 */
/*#define TM_USE_IPSEC_IPCOMP */

#ifdef TM_USE_IPSEC_IPCOMP
/* 
 * Define TM_IPCOMP_DEBUG to debug IPComp
 */
/* #define TM_IPCOMP_DEBUG */
#endif /* TM_USE_IPSEC_IPCOMP */
/* 
 * If a router is tunneling ICMP packet generated by other
 * routers, the source address of the inner header will not 
 * be the tunnel ends address, the source address check at the destination
 * will fail. Define TM_IPSEC_INCOMING_ICMP_NOSRCCHK to skip ICMP
 * source address checking, 

 * If TM_IPSEC_INCOMING_ICMP_BYPASS is defined, we don't do 
 * ICMP policy check at all 
 */

/*#define TM_IPSEC_INCOMING_ICMP_NOSRCCHK */

/* 
 * If you want to bypass IPsec protection to avoid the chicken-and-egg problem
 * for ICMPv6 ND and MLD message, please define this. If TM_USE_IKE is defined, 
 * you should define this macro, otherwise, you have to manually add the 
 * corresponding SA as the first thing.
 */
#define TM_6_IPSEC_ICMPV6_NDMLD_BYPASS 

/* 
 * The replay window size, default is 32 
 */
#define TM_IPSEC_DEFAULT_REPLAY_SIZE          32

/* 
 * IPsec SA Limit
 * The following defines the total number of IPsec SA pairs that can be 
 * present at one time.  When this limit is met, any new SA creation results
 * in the removal of the oldest SA.  The default is 100 pairs (100 incoming,
 * (100 outgoing).  To override this value, define a new value here.
 * To make the number of allowable SAs unlimited, set this to 0.
 * Note:  This is limited to a tt16Bit value.
 */
/*#define TM_IPSEC_SAD_MAX_SIZE 100 */

/* Define the following macro to bypass any nested IKE packet, don't do any 
 * protection for nested IKE packet. Otherwise, if the following macro is
 * not defined, nested IKE packets are subject to policy check.
 */
/* #define TM_IPSEC_BYPASS_NESTIKE_PACKET */

#if defined(TM_USE_IKEV1) || defined(TM_USE_IKEV2)


/* #define TM_IKE_ERROR_NOTIFY */

/* 
 * IKE SA Limit
 * The following defines the total number of IKE SAs that can be present
 * at one time.  When this limit is met, any new SA creation results in
 * the removal of the oldest SA.  The default is 100.  To override this
 * value, define a new value here.  To make the number of allowable SAs
 * unlimited, set this to 0.  Note:  This is limited to a tt16Bit value.
 */
/*#define TM_IKE_SAD_MAX_SIZE 100 */

/* 
 * IKE queued bytes limit.
 * Each policy content can only hold this maximum data waiting for IKE 
 * negotiation results. The default is 8000 bytes. To override this value
 * define a new value here. To prevent any data from being queued, set
 * this value to 0.
 */
/* #define TM_IKE_PACKET_MAX_QUEUE_BYTES  8000 */

/* Diffe-Hellman local secret size, NIST value = 256bits */
#define TM_IKE_DHSECRET_DEFAULT_SIZE          32

/* TM_IKE_PFS_KEY is set to require phase 2 PFS of key.
 * PFS of identity is set to require PFS of identity 
 */
#define TM_IKE_PFS_KEY_ENABLED

/* define AGGRESSIVE mode to use AGGRESSIVE mode rather than 
 * MAIN_MODE if we are the initiator for phase 1 negotiation 
 */
/*#define TM_IKE_PHASE1_AGGRESSIVE_ENABLED*/

/* for aggressive mode, we must know which Diffie-Hellman 
 * group we are going to use. (in order to send proper KE payload)
 */

#define TM_IKE_AGGRESSIVE_DHGROUP     TM_DHGROUP_2

/* INITIAL_CONTACT will be sent after we have encryption and
 * auth keys, and we only accept protected INITIAL_CONTACT message
 */
#define TM_IKE_INITIAL_CONTACT_ENABLED

/* For phase 1 negotiation, and use pre-sharedkey as the 
 * authentication method, Elmic IKE will try to find the 
 * pre-shared key for specified identification, if no pre-shared 
 * key found, and if TM_IKE_USE_DEFAULT_PRESHARED_KEY is defined, 
 * Elmic IKE will use the default preshared key to negotiate. 
 * If it is not defined and no preshared key is found, error will 
 * be returned.
 */

/* #define TM_IKE_USE_DEFAULT_PRESHARED_KEY */

#endif /* defined(TM_USE_IKEV1) || defined(TM_USE_IKEV2) */

#endif /* TM_USE_IPSEC */

#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
#ifndef TM_USE_PKI
#define TM_USE_PKI
#endif /* TM_USE_PKI */
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */

#ifdef TM_USE_PKI
/* Whether check that a certificate is alive. If enable, TM_PKI_TIME 
 * must be defined as current time in order to compare before/after
 * time, and the default value of TM_PKI_TIME is "030514000000Z" that 
 * means 2003, May.14 and Z means  Zulu, or Greenwich Mean Time.
 */
/*#define  TM_PKI_CERT_CHECK_ALIVE */

/* Whether verify a certificate. If enable, all Certificates
 * except root CA's will be verified by their and chain's CAs. 
 */
/*#define TM_PKI_CERT_NOT_VERIFY */

/* Define this macro if you wish to skip checking that the RootCA's
 * BasicConstraint extension is set to "critical"
 * THIS IS A SECURITY RISK!
 */
/*#define TM_PKI_ROOTCA_IGNORE_BCCRITICAL */

/* Define this macro if you wish to skip checking RootCA's
 * BasicConstraint extension field.
 * THIS IS A SECURITY RISK!
 */
#define TM_PKI_BCCRITICAL_CHECK_DISABLE
#endif /* TM_USE_PKI */

#if (defined(TM_USE_IPSEC) || defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))

/* if you want to use software implementation of any hash
 * algorithm (MD5 or SHA1 or RIPEMD), or cryptography algorithm (DES, 3DES
 * BLOWFISH, RC4, RC5, CAST128, TWOFISH, AES, DIFFIE-HELLMAN exchange,
 * RSA, DSA. you must define the following macro. If your hardware accelertor
 * supports all algorithms (including Diffie-Hellman key exchange),
 * you may want to comment out TM_USE_SW_CRYPTOENGINE to get
 * the minimal code size
 */
#ifndef TM_X_USE_SW_CRYPTOENGINE
#define TM_USE_SW_CRYPTOENGINE
#endif /* TM_X_USE_SW_CRYPTOENGINE */

/* Define the following macro to use HIFN 7951 hardware accelerator */
/*#define TM_USE_HF7951_CRYPTOENGINE */

/* Define the following macro to use MCF5235 hardware accelerator */
/*#define TM_USE_MCF5235_CRYPTOENGINE*/

/* SHA-384 and SHA-512 use a larger block size that other hash algorithms,
 * so TM_AALG_VAR_BLOCK_SIZE must be defined when using either hash algorithm.
 * When this macro is defined, ttAhAlgorithm has an extra field for algorithm
 * block size.
 */
/* #define TM_AALG_VAR_BLOCK_SIZE */

#ifdef TM_USE_SW_CRYPTOENGINE
/* Define the following software implemenation of cryptography algorithms.
 * If any of the following algorithms are supported by your hardware
 * accelerator, you may want to comment them out to reduce code size.
 */
/* DES and 3DES uses the same module */
#define TM_USE_MD2
#define TM_USE_MD5
#define TM_USE_SHA1
#define TM_USE_SHA256
/* * SHA-384 and SHA-512 require TM_AALG_VAR_BLOCK_SIZE. */
/* #define TM_AALG_VAR_BLOCK_SIZE */
/* #define TM_USE_SHA384 */
/* #define TM_USE_SHA512 */
#define TM_USE_SHAHMAC_96
#define TM_USE_AESXCBC
#define TM_USE_RIPEMD
#define TM_USE_DES 
#define TM_USE_3DES 
#define TM_USE_BLOWFISH
#define TM_USE_RC2
/*Must have patent license to use RC5 */
/*#define TM_USE_RC5*/
#define TM_USE_CAST128
#define TM_USE_AES
#define TM_USE_TWOFISH
#define TM_USE_ARCFOUR
#define TM_PUBKEY_USE_DIFFIEHELLMAN
/*
 * Define the following macro if you want to pad the Diffie-Hellman shared
 * secret with leading zeros when the length is < our prime vector.
 * This behavior is widely accepted.
 * This macro should be defined unless you are certain that the peer is not
 * padding. A mismatch of padding vs. non-padding between peers will cause
 * the peers to generate different shared secrets and the negotiation will fail.
 *
 * NOTE: This macro is required for IKE and IPsec to operate properly.
 */
#define TM_USE_DIFFIEHELLMAN_PADDING
#ifdef TM_USE_PKI
#define TM_PUBKEY_USE_RSA
#define TM_PUBKEY_USE_DSA
#endif /* TM_USE_PKI */
#endif /* TM_USE_SW_CRYPTOENGINE */

#ifdef TM_USE_MCF5235_CRYPTOENGINE
#ifndef TM_USE_MD5
#define TM_USE_MD5
#endif /* TM_USE_MD5 */
#ifndef TM_USE_SHA1
#define TM_USE_SHA1
#endif /* TM_USE_SHA1 */
#ifndef TM_USE_DES
#define TM_USE_DES 
#endif /* TM_USE_DES */
#ifndef TM_USE_3DES
#define TM_USE_3DES 
#endif /* TM_USE_3DES */
#ifndef TM_USE_AES
#define TM_USE_AES
#endif /* TM_USE_AES */
#endif /* TM_USE_MCF5235_CRYPTOENGINE */

#endif /* TM_USE_IPSEC || TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */

#ifdef TM_USE_IPSEC_IPCOMP

#define TM_USE_SW_COMPENGINE

#ifdef TM_USE_SW_COMPENGINE
/* DEFLATE module */
#define TM_USE_DEFLATE
#endif /* TM_USE_SW_COMPENGINE */
#endif /* TM_USE_IPSEC_IPCOMP */

/*
 * SSL TLS options
 */
/* Uncomment the following macro if user doesn't want SSL 3.0 */
//#define TM_USE_SSL_VERSION_30

/* Uncomment the following macro if user doesn't want SSL 3.1, i.e TLS 1.0 */
//#define TM_USE_SSL_VERSION_31

/* Uncomment the following macro if user doesn't want TLS 1.2 */
#define TM_USE_SSL_VERSION_33

/* uncomment the following macro if you don't want to accept SSL 2.0 hello */
#define TM_SSL_ACCEPT_20_CLIENTHELLO
/* If the following macro is defined, upon receiving close_notify, we will
 * immediately flush our send queue and send back close_notify to the peer,
 * and shutdown SSL send path. If this macro is not defined, it is up to the 
 * user to call tfSslConnectUserClose or tfClose to shutdown the SSL send 
 * path. Note that, if user doesn't call anything, the SSL send path is 
 * still ON until the socket is closed.
 */
#define TM_SSL_IMMEDIATE_CLOSE_NOTIFY

/* The minimum user data bytes to TRIGGER SSL send. Data less than this size 
 * will not trigger SSL send, instead, it will be queued.  Once SSL send
 * has been triggered up, all pending user data will be sent out and after
 * that SSL waits for the next trigger event to send again. The default value
 * is set to zero, ie, SSL wont queue any packet. If user chooses to set it
 * to something else, user should be aware that if application data is smaller
 * than this threshold, it won't be sent out unless user flush the channel
 * by calling tfSslUserSendFlush()
 */
#define TM_SSL_SEND_DATA_MIN_SIZE                0

/* The maximum value of user data in one single SSL record. By RFC, this value
 * is 16384. However, considering that peer may not have enough receive buffer
 * to queue, we use the following value (8000). So if user wants to send more 
 * than 8000 bytes of data, see 8100 bytes, we have to cut it into two SSL 
 * records, one contains 8000 user data bytes, another one contains 100 user 
 * data bytes (Note that even if 100 is less than TM_SSL_SEND_DATA_MIN_SIZE, 
 * we still send it out, because we used 8100 bytes to trigger up SSL send.)
 */
#define TM_SSL_SEND_DATA_MAX_SIZE                8000 

/* uncomment the following macro, if you don't want to use any ephemeral
 * Diffie-Hellman key exchange method. For example, if your SSL cipher suite
 * is something like SSL_RSA_XXXX or TLS_RSA_XXXX (The most popular case in 
 * real world), you don't need this macro.
 */
#define TM_SSL_USE_EPHEMERAL_DH

/* uncomment the following macro if you don't want do mutual authentication*/
#define TM_SSL_USE_MUTUAL_AUTH

/* If you don't define TM_EXPORT_RESTRICT, you may want to reject any 
 * exportable cipher suites by defining the following macro. Note that, 
 * by defining the following macro, you get higher security because you
 * reject any exportable cipher suite, the cost is that you may not be 
 * able to negotiation with clients which offer exportable cipher suites
 * only. If you do define TM_EXPORT_RESTRICT, this macro has no effect
 * to the behavior. Default not to be defined
 */
/*#define TM_SSL_REJECT_EXPORTABLE_CIPHER*/

/* uncomment the following macro only if 1) you want to negotiate EDH_DSS_XXX
 * cipher 2) negotiate with Netscape only 3) in SSL version 3.0 version only. 
 * If any of these three conditions is false, don't uncomment this macro.  
 * The description of how to encode DSS signature in the SSLv3 specification 
 * is insufficiently specific and Netscape interprets it differently from most
 * other vendors. Instead of DER encoding r and s, Netscape merely 
 * concatenates them into a single 40-byte field. Thus, although Netscape 
 * implements DSS for client authentication, it is not interoperable with 
 * other implementations. Despite widespread agreement on the "right thing" 
 * Netscape has refused to change their implementation, claiming that it 
 * complies with the SSL v3 specification and citing installed base. The TLS
 * specification clears up this issue; ALL TLS implementations must DER 
 * encode DSS signature 
 */
/* #define TM_SSL30_DSSSIGN_NETSCAPE_FORMAT */

/*The following Diffie-Hellman parameters take effect only if Ephemeral Diffie
 * hellman is enabled and is chosen as the key exchange method.
 */

/* if you don't have export restrict, we use OAKLEY group 1 prime
 * (768 bits)if Ephemeral Diffie-Hellman is going to be used. TM_
 * DHGROUP_2 is 1024 bits, and TM_DHGROUP_EXPORT is 512 bits. According
 * to draft draft-ietf-tls-56-bit-ciphersuites-01.txt, Changes in US export
 * regulations in 1999 permitted the export of software programs 
 * using 56-bit data encryption and 1024-bit key exchange
 */
#define TM_SSL_DHE_PRIME_GROUP             TM_DHGROUP_1

/* Diffie-Hellman generator is 2 */
#define TM_SSL_DHE_GENERATOR               TM_DH_GENERATOR_2

/* Diffie-Hellman private secret default size*/
#define TM_SSL_DHSECRET_DEFAULT_SIZE       32

/*
 * In Treck releases prior to 5.0.1.40, Treck did not strip the leading zeros
 * off of the padded Diffie-Hellman shared secret before performing the hash.
 * This caused hash mismatches between peers on rare occasions where the
 * shared secret length was != to the prime length. These hash mismatches were
 * recoverable but required a new SSL negotiation to take place.
 * This macro is here for interoperability with old versions.
 * This macro only has an effect when TM_USE_DIFFIEHELLMAN_PADDING is defined.
 */
/* TM_SSL_DH_HASH_WITH_PADDING */

/* Uncomment the following macro if the client allows no certificate 
 * operation even in client-authentication situation. If the macro is
 * defined, the SSL_CLIENT will ignore Cert_Request message if it
 * doesn't have any certificate, and continue normal SSL negotiation. 
 * To close the connection or not will be up to the server to decide. 
 * In any case the client will send an alert-warning message to server.
 */
/* #define TM_SSL_CLIENT_ALLOW_NO_CERT */

/* Uncomment the following macro If no suitable certificate is available,
 * the client send a certificate message containing no certificates. 
 */
#define TM_SSL_CLIENT_RESPONSE_NO_CERT

/*
 * Uncomment the following line, if you want to use C library routines,
 * instead of the equivalent library routines provided by the Turbo Treck 
 * stack.
 */
/* #define TM_USE_CLIB */

#ifdef TM_USE_CLIB
/* NOTE: you may need to modify the following, if you #define TM_USE_CLIB
 *
 * For isupper, tolower ... */
#include <ctype.h>
/* For memcpy, strchr ...   */
#include <string.h>
/* For printf, sprintf...   */
#include <stdio.h>
/* For strtoul... */
#include <stdlib.h>
#endif /* TM_USE_CLIB */

/* 
 * if TM_USE_SNIFF is defined, interface drivers will dump the packets to the
 * file system in libpcap format, to be opened by tcpdump, windump, ethereal, 
 * or other libpcap file readers.
 */
#ifndef TM_USE_RAM_FS /* not enough room in RAM FS for pcap file */
/* #define TM_USE_SNIFF */
#endif /* TM_USE_RAM_FS */

#ifdef TM_USE_UPNP_DEVICE
/* comment out TM_USE_GENA if your UPnP device doesn't have any evented
   service state variables, to save code space */
#define TM_USE_GENA    
#endif /* TM_USE_UPNP_DEVICE */

#ifdef TM_DISABLE_ALL_FILE_LOGGING
#define TM_DISABLE_ANSI_LINE_FILE /* Disable memoryleak.dat */
#undef TM_USE_SNIFF
#endif /* TM_DISABLE_ALL_FILE_LOGGING */

/*
 * If TM_USE_LLMNR_RESPONDER is defined ,LLMNR optional module with
 * responder only functionality is enabled. Uncomment out the below macro
 * to use LLMNR Responder functionality. If the below macro is used along
 * with TM_USE_LLMNR_SENDER macro, both sender and responder
 * functionalities will be enabled.
 */
/* #define TM_USE_LLMNR_RESPONDER */

/*
 * If TM_USE_LLMNR_SENDER is defined, LLMNR optoinal module with
 * sender only functionality  is enabled. Uncomment out the below macro
 * to use LLMNR sender functionality.  If the below macro is used along
 * with TM_USE_LLMNR_RESPONDER macro, both sender and responder
 * functionalities will be included.
 */
/* #define TM_USE_LLMNR_SENDER */

#ifdef __cplusplus
}
#endif

#endif /* _TRSYSTEM_H_ */
