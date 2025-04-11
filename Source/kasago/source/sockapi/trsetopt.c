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
 * Description: BSD Sockets Interface (tfSetTreckOptions)
 *
 * Filename: trsetopt.c
 * Author: Paul & Odile
 * Date Created: 11/10/97
 * $Source: source/sockapi/trsetopt.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:22JST $
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


/*
 * Set Treck options (only after a call to tfStartTreck)
 * Set a global variable with a new value.
 */

int tfSetTreckOptions(int optionName, ttUser32Bit optionValue)
{
    int         errorCode;
#ifdef TM_USE_HTTPD_SSI
    ttUser32Bit fileBlockSize;
#endif /* TM_USE_HTTPD_SSI */

    errorCode = TM_ENOERROR;
    switch (optionName)
    {
#ifdef TM_USE_IPV6
#ifdef TM_6_USE_PREFIX_DISCOVERY
        case TM_6_OPTION_PREFIX_TIMER_RES_SEC:
            tm_context(tv6PrefixTimerResSec) = (tt16Bit) optionValue;
            if (tm_context(tv6PrefixTimerResSec) == 0)
            {
/* smallest timer resolution we support */
                tm_context(tv6PrefixTimerResSec) = 1;
            }
            if (tm_context(tv6RtPrefixAgingTimerPtr) != TM_TMR_NULL_PTR)
            {
/* if the timer is already running, then restart it with the new timer period*/
                tm_timer_new_time(
                    tm_context(tv6RtPrefixAgingTimerPtr),
                    tm_context(tv6PrefixTimerResSec) * 1000);
            }
            break;
#endif /* TM_6_USE_PREFIX_DISCOVERY */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_TCP
        case TM_OPTION_TCP_SOCKETS_MAX:
            if (    (optionValue == 0)
                 || (optionValue >= (tt32Bit)TM_SOC_NO_INDEX)
                 || (optionValue > tm_context(tvMaxNumberSockets)) )
            {
                errorCode = TM_EINVAL;
            }
            else
            {
                tm_context(tvMaxTcpVectAllocCount) = optionValue;
            }
            break;
        case TM_OPTION_TCP_TMWT_MAX:
            tm_context(tvMaxTcpTmWtAllocCount) = optionValue;
            break;
        case TM_OPTION_SOTB_TCP_CON_PORT_THRS:
/*
 * TCP sockets:
 * Threshold of number of sockets in the socket lookup table before we switch
 * from a random local port selection to a sequential local port selection
 */
            tm_context(tvSocketThresholdSequentialPort[TM_SOTB_TCP_CON]) =
                                                     (unsigned int)optionValue;
            break;
#endif /* TM_USE_TCP */
        case TM_OPTION_SOTB_NON_CON_PORT_THRS: 
/*
 * Other sockets:
 * Threshold of number of sockets in the socket lookup table before we switch
 * from a random local port selection to a sequential local port selection
 */
            tm_context(tvSocketThresholdSequentialPort[TM_SOTB_NON_CON]) =
                                                     (unsigned int)optionValue;
            break;
        case TM_OPTION_RT_TIMER_RES_MSEC:
            if (optionValue < tvTimerTickLength)
            {
/* the timer resolution should not be less than the tick length */
                optionValue = tvTimerTickLength;
            }
            tm_context(tvRtTimerResolution) = optionValue;
            if (tm_context(tvRtTimerPtr) != TM_TMR_NULL_PTR)
            {
/* if the timer is already running, then restart it with the new timer period*/
                tm_timer_new_time(tm_context(tvRtTimerPtr), optionValue);
            }
            break;

        case TM_OPTION_TIMER_MAX_EXECUTE:
/* Maximum number of timers that can be executed in a single call to
   tfTimerExecute */
            if ( (int)optionValue >= 1 )
            {
                tm_context(tvTimerMaxExecute) = (int)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

#ifdef TM_USE_UPNP_DEVICE
        case TM_OPTION_SSDP_DEFAULT_CACHE_CTL:
/* 
 * Default SSDP advertisement duration, in seconds, used in CACHE-CONTROL
 * header. Default is 1800 seconds.
 */
            tm_context(tvSsdpDefaultCacheCtl) = (int)optionValue;
            break;
#endif /* TM_USE_UPNP_DEVICE */

#ifdef TM_USE_SOAP
        case TM_OPTION_SOAP_MPOST_ONLY:
            if(optionValue == 0)
            {
                tm_context(tvSoapOptions) &= ~TM_SOAPOPT_MPOSTONLY;
            }
            else
            {
                tm_context(tvSoapOptions) |= TM_SOAPOPT_MPOSTONLY;
            }
            break;
#endif /* TM_USE_SOAP */

#ifdef TM_USE_IPV4
        case TM_OPTION_ICMP_ROUTE_REPLY: 
            tm_context(tvIcmpRouteReply) = (tt8Bit)optionValue;
            break;

#ifdef TM_USE_ECHO_CONFIG
        case TM_OPTION_ICMP_ECHO_ENABLE:
            tm_context(tvIcmpEchoEnable) = (tt8Bit)optionValue;
#endif /* TM_USE_ECHO_CONFIG */

#ifndef TM_SINGLE_INTERFACE_HOME
        case TM_OPTION_ICMP_ERR_DST_AS_SRC:
            tm_context(tvIcmpErrDstAsSrc) = (tt8Bit)optionValue;
            break;
#endif /* TM_SINGLE_INTERFACE_HOME */
#endif /* TM_USE_IPV4 */

        case TM_OPTION_ARP_MAX_ENTRIES:
/* Maximum number of ARP entries */
            if ( (int)optionValue >= 1 )
            {
                tm_context(tvArpMaxEntries) = (int)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

        case TM_OPTION_ARP_MAX_RETRY:
            if ( (int)optionValue >= 1 )
            {
                tm_context(tvArpRequestTries) = optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

        case TM_OPTION_ARP_TIMEOUT_TIME:
            if ( optionValue >= TM_UL(1) )
            {
                tm_context(tvArpRetryTime) = optionValue*TM_UL(1000);
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

        case TM_OPTION_ARP_QUIET_TIME:
            if ( optionValue >= TM_UL(1) )
            {
                tm_context(tvArpQuietTime) = optionValue*TM_UL(1000);
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

        case TM_OPTION_ARP_TTL:
/* Arp cache entry time to live */
            if (optionValue != TM_RTE_INF)
            {
                tm_context(tvArpTimeout) = optionValue*TM_UL(1000);
            }
            else
            {
                tm_context(tvArpTimeout) = TM_RTE_INF;
            }
            break;

        case TM_OPTION_ROUTE_MAX_ENTRIES:
/* Maximum number of dynamic routing table entries */
            if ( (int)optionValue >= 1 )
            {
                tm_context(tvRtMaxEntries) = (int)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

        case TM_OPTION_ROUTER_ID:
/* Router ID */
            tm_ip_copy((tt4IpAddress)optionValue, tm_context(tvRouterId));
            break;

        case TM_OPTION_ROUTER_AGE_LIMIT:
/* Routing entry age limit */
            tm_context(tvRtTimeout) = optionValue*TM_UL(1000);
            break;

#ifdef TM_PMTU_DISC
        case TM_OPTION_DECREASED_PMTU_TTL:
/* Path MTU host entry age limit after the path MTU estimate is decreased */
            tm_context(tvRtDecreasedMtuTimeout) = optionValue*TM_UL(1000);
            break;

        case TM_OPTION_LARGER_PMTU_TTL:
/* Path MTU host entry age limit when a larger MTU is attempted */
            tm_context(tvRtLargerMtuTimeout) = optionValue*TM_UL(1000);
            break;
#endif /* TM_PMTU_DISC */

        case TM_OPTION_SEND_TRAILER_SIZE:
            if ( optionValue <= 0xFFFFUL)
            {
                tm_context(tvPakPktSendTrailerSize) =
                    (ttPktLen) tm_packed_byte_count(optionValue);
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;


#ifdef TM_USE_IPV4
        case TM_OPTION_RIP_ENABLE:
            if (    (tt8Bit)optionValue == TM_8BIT_YES
                 || (tt8Bit)optionValue == TM_8BIT_ZERO )
            {
                tm_context(tvRipOn) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

        case TM_OPTION_RIP_SEND_MODE:
            if (    ((tt8Bit)optionValue == TM_RIP_NONE)
                 || ((tt8Bit)optionValue == TM_RIP_1)
                 || ((tt8Bit)optionValue == TM_RIP_2)
                 || ((tt8Bit)optionValue == TM_RIP_2_BROADCAST) )
            {
                tm_context(tvRipSendMode) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

        case TM_OPTION_RIP_RECV_MODE:
            if (    ((tt8Bit)optionValue == TM_RIP_NONE)
                 || ((tt8Bit)optionValue == TM_RIP_1)
                 || ((tt8Bit)optionValue == TM_RIP_2)
                 || ((tt8Bit)optionValue == (TM_RIP_1|TM_RIP_2))
                 || ((tt8Bit)optionValue == TM_RIP_2_BROADCAST) )
            {
                tm_context(tvRipRecvMode) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

        case TM_OPTION_IP_FORWARDING:
            if (    (tt8Bit)optionValue == TM_8BIT_YES
                 || (tt8Bit)optionValue == TM_8BIT_ZERO )
            {
                tm_context(tvIpForward) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

        case TM_OPTION_IP_DBCAST_FORWARD:
            if (    (tt8Bit)optionValue == TM_8BIT_YES
                 || (tt8Bit)optionValue == TM_8BIT_ZERO )
            {
                tm_context(tvIpDbcastForward) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

#ifdef TM_USE_LBCAST_CONFIG
        case TM_OPTION_IP_LBCAST_ENABLE:
            if (    (tt8Bit)optionValue == TM_8BIT_YES
                 || (tt8Bit)optionValue == TM_8BIT_ZERO )
            {
                tm_context(tvIpLbcastEnable) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;
#endif /* TM_USE_LBCAST_CONFIG */

        case TM_OPTION_IP_TTL:
            if (optionValue <= TM_UL(0xFF))
            {
                tm_context(tvIpDefTtl) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

        case TM_OPTION_IP_TOS:
            if (optionValue <= TM_UL(0xFF))
            {
                tm_context(tvIpDefTos) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

#ifdef TM_IP_FRAGMENT
        case TM_OPTION_IP_FRAGMENT:
            if (    (tt8Bit)optionValue == TM_8BIT_YES
                 || (tt8Bit)optionValue == TM_8BIT_ZERO )
            {
                tm_context(tvIpFragment) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;
#endif /* TM_IP_FRAGMENT */

#ifdef TM_IP_REASSEMBLY
        case TM_OPTION_IP_FRAG_TTL:
            if ( (optionValue <= 0x7F) && (optionValue >= 1) )
            {
                tm_context(tvFragTtl) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

        case TM_OPTION_IP_FRAG_MAX_Q_SIZE:
/*
 * Option to change the maximum number of IP datagrams waiting to be
 * reassembled. If any fragment of a new IP datagram arrives when the
 * maximum number of IP datagrams waiting to be reassembled has been reached,
 * it is dropped. Default value is 5.
 */
            if (optionValue <= TM_UL(0x7F))
            {
                tm_context(tvFragMaxQSize) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

        case TM_OPTION_IP_FRAG_FAILED_MAX:
/*
 * Option to change the maximum number of IP datagrams that we will track that
 * were too big to be reassembled. If we receive a new "too large" datagram
 * when we've already reached the maximum, we will free the oldest.
 * Default value is 5.
 */
            if (optionValue <= TM_UL(0x7F))
            {
                tm_context(tvFragFailedMaxQSize) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

        case TM_OPTION_IP_FRAG_ENTRY_MAX_SIZE:
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
            if (tm_context(tvFragEntryMaxSize) != optionValue)
            {
                tm_kernel_set_critical;
                tm_context(tvIpData).ipv4InterfaceTableLastChange = 
                    tm_snmp_time_ticks(tvTime);
                tm_kernel_release_critical;
            }
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */

/*
 * Option to change the maximum size of an IP datagram waiting to be
 * reassembled. Beyond that size, we drop the entire IP datagram. Default value
 * is 8200 bytes.
 */
            tm_context(tvFragEntryMaxSize) = optionValue;
            break;
#endif /* TM_IP_REASSEMBLY */

        case TM_OPTION_ICMP_ADDR_MASK_AGENT:
            if (    (tt8Bit)optionValue == TM_8BIT_YES
                 || (tt8Bit)optionValue == TM_8BIT_ZERO )
            {
                tm_context(tvIcmpAddrMaskAgent) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

        case TM_OPTION_UDP_CHECKSUM:
            if (    (tt8Bit)optionValue == TM_8BIT_YES
                 || (tt8Bit)optionValue == TM_8BIT_ZERO )
            {
                tm_context(tvUdpChecksumOn) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

        case TM_OPTION_ARP_SMART:
            if (    (tt8Bit)optionValue == TM_8BIT_YES
                 || (tt8Bit)optionValue == TM_8BIT_ZERO )
            {
                tm_context(tvArpSmart) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

#endif /* TM_USE_IPV4 */

#if (defined(TM_IGMP) && defined(TM_USE_IPV4))
#ifdef TM_USE_IGMPV3
        case TM_OPTION_IGMP_SO_MAX_SRC_FILTER:
            tm_context(tvIgmpSoMaxSrcFilter) = (tt16Bit)optionValue;
            break;
        case TM_OPTION_IGMP_IP_MAX_SRC_FILTER:
            tm_context(tvIgmpIpMaxSrcFilter) = (tt16Bit)optionValue;
            break;
#endif /* TM_USE_IGMPV3 */
#endif /* TM_IGMP and TM_USE_IPV4 */
            
#ifdef TM_USE_IPV6
    case TM_6_OPTION_ICMP_TX_ERR_LIMIT:
        if (optionValue)
        {
            tm_context(tv6IcmpTxErrLimitMsec) = (tt32Bit)optionValue;
        }
        else
        {
            /* Reload the default setting */
            tm_context(tv6IcmpTxErrLimitMsec) = TM_6_ICMP_DEF_TX_ERR_LIMIT_MSEC;
        }
        break;

    case TM_6_OPTION_ICMP_TX_ERR_BURST:
        if (optionValue)
        {
            tm_context(tv6IcmpTxErrLimitBurst) = (tt32Bit)optionValue;
        }
        else
        {
            /* Reload the default setting */
            tm_context(tv6IcmpTxErrLimitBurst) = TM_6_ICMP_DEF_TX_ERR_LIMIT_BURST;
        }
        /* Refill the token bucket */
        tm_context(tv6IcmpTxErrTokenCnt) = tm_context(tv6IcmpTxErrLimitBurst);
        break;

    case TM_6_OPTION_ND_MAX_MCAST_RETRY:
        tm_context(tv6NdMcastRequestTries) = (tt32Bit) optionValue;
        break;

    case TM_6_OPTION_ND_MAX_UNICAST_RETRY:
        tm_context(tv6NdUnicastRequestTries) = (tt32Bit) optionValue;
        break;

    case TM_6_OPTION_IP_DEPRECATE_ADDR:
        if (    (tt8Bit)optionValue == TM_8BIT_YES
                || (tt8Bit)optionValue == TM_8BIT_ZERO )
        {
            tm_context(tv6IpDeprecateAddr) = (tt8Bit)optionValue;
        }
        else
        {
            errorCode = TM_EINVAL;
        }
        break;

#ifdef TM_6_USE_IP_FORWARD
    case TM_6_OPTION_IP_FORWARDING:
        if (    (tt8Bit)optionValue == TM_8BIT_YES
                || (tt8Bit)optionValue == TM_8BIT_ZERO )
        {
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
            if ((tm_context(tv6IpForward)^(tt8Bit)optionValue) == 1)
            {
                tm_kernel_set_critical;
                tm_context(tvIpData).ipv6InterfaceTableLastChange = 
                    tm_snmp_time_ticks(tvTime);
                tm_kernel_release_critical;
            }
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */      
            tm_context(tv6IpForward) = (tt8Bit)optionValue;
        }
        else
        {
            errorCode = TM_EINVAL;
        }
        break;
#endif /* TM_6_USE_IP_FORWARD */

#ifdef TM_6_IP_FRAGMENT
        case TM_6_OPTION_IP_FRAGMENT:
            if (    (tt8Bit)optionValue == TM_8BIT_YES
                 || (tt8Bit)optionValue == TM_8BIT_ZERO )
            {
                tm_context(tv6IpFragment) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;
#endif /* TM_6_IP_FRAGMENT */

#ifdef TM_6_IP_REASSEMBLY
        case TM_6_OPTION_IP_FRAG_TTL:
            if ( (optionValue <= 0x7F) && (optionValue >= 1) )
            {
                tm_context(tv6FragTtl) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

        case TM_6_OPTION_IP_FRAG_MAX_Q_SIZE:
/*
 * Option to change the maximum number of IPv6 datagrams waiting to be
 * reassembled. If any fragment of a new IPv6 datagram arrives when the
 * maximum number of IPv6 datagrams waiting to be reassembled has been reached,
 * it is dropped. Default value is 5.
 */
            if (optionValue <= 0x7F)
            {
                tm_context(tv6FragMaxQSize) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

        case TM_6_OPTION_IP_FRAG_FAILED_MAX:
/*
 * Option to change the maximum number of IP datagrams that we will track that
 * were too big to be reassembled. If we receive a new "too large" datagram
 * when we've already reached the maximum, we will free the oldest.
 * Default value is 5.
 */
            if (optionValue <= TM_UL(0x7F))
            {
                tm_context(tv6FragFailedMaxQSize) = (tt8Bit)optionValue;
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;

        case TM_6_OPTION_IP_FRAG_ENTRY_MAX_SIZE:
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
            if (tm_context(tv6FragEntryMaxSize) != optionValue)
            {
                tm_kernel_set_critical;
                tm_context(tvIpData).ipv6InterfaceTableLastChange = 
                    tm_snmp_time_ticks(tvTime);
                tm_kernel_release_critical;
            }
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
/*
 * Option to change the maximum size of an IPv6 datagram waiting to be
 * reassembled. Beyond that size, we drop the entire IPv6 datagram. Default
 * value is 8200 bytes.
 */
            tm_context(tv6FragEntryMaxSize) = optionValue;
            break;
#endif /* TM_6_IP_REASSEMBLY */

#ifdef TM_6_PMTU_DISC
/*
 * The time (in seconds) before an IPv6 Path MTU estimate is increased, in
 * order to discovery a larger Path MTU value.
 */
        case TM_6_OPTION_PATH_MTU_TTL:
            tm_context(tv6RtPathMtuTimeout) = optionValue;
            break;
#endif /* TM_6_PMTU_DISC */

#ifdef  TM_6_USE_MIP_RO
        case TM_6_OPTION_MIP_RO_ENABLE:
            tm_context(tv6MipRoEnable) = (tt8Bit) optionValue;
            break;
#endif /* TM_6_USE_MIP_RO */

#ifdef  TM_6_USE_MIP_RO
        case TM_6_OPTION_MAX_BINDING_LIFETIME:
/* we store it in the unit of milliseconds */
            tm_context(tv6CnMaxBindingLifeMsec) = optionValue * 1000;
            break;
#endif /* TM_6_USE_MIP_RO */

#ifdef TM_6_USE_MIP_MN
    case TM_6_OPTION_MN_REG_LT_BIAS:
        tm_context(tv6MnRegLtBiasMsec) = (tt32Bit) optionValue * 1000;
        break;

    case TM_6_OPTION_MN_DEREG_TIMEOUT:
        tm_context(tv6MnDeregTimeoutMsec) = (tt32Bit) optionValue * 1000;
        break;

    case TM_6_OPTION_RR_MAX_BUL_ENTRIES:
        tm_context(tv6MnMaxBulEntries) = (tt16Bit) optionValue;
        break;

    case TM_6_OPTION_MN_BEACON_THRESHOLD:
        tm_context(tv6MnBeaconThresh) = (tt16Bit) optionValue;
        break;

    case TM_6_OPTION_MN_1ST_REG_INIT_TIMEOUT:
        tm_context(tv6InitBaTimeoutFirstRegMsec) = (tt32Bit) optionValue;
        break;

    case TM_6_OPTION_MN_EAGER_CELL_SWITCH:
        tm_context(tv6MnEagerCellSwitch) = (tt8Bit) optionValue;
        break;
#endif /* TM_6_USE_MIP_MN */

#ifdef TM_6_USE_MIP_CN
    case TM_6_OPTION_RR_MAX_BCE_ENTRIES:
        tm_context(tv6CnMaxBceEntries) = (tt16Bit) optionValue;
        break;
#endif /* TM_6_USE_MIP_CN */

#endif /* TM_USE_IPV6 */

#ifdef TM_USE_HTTPD_SSI
/*  The maximum SSI tag length in the SSI enabled pages. */
    case TM_OPTION_SSI_MAX_TAG_LEN:
        fileBlockSize = tfFSGetFileBlockSize();
        if (fileBlockSize  == 0)
        {
            fileBlockSize  = TM_FS_FILE_BLOCK_SIZE;
        }
        if (optionValue >= fileBlockSize)
        {
/*  The maximum SSI tag length must be smaller than the file read size */
            errorCode = TM_EINVAL;
        }
        else
        {
/*  successfull */
            tm_context(tvHttpdSsiMaxTagLen) = (tt16Bit) optionValue;
        }
        break;
#endif /* TM_USE_HTTPD_SSI */

#ifdef TM_USE_HTTPD_CGI
    case TM_OPTION_CGI_MIN_ARG:
/* minium number of CGI variables per allocation. */
        tm_context(tvHttpdCgiMinArgCnt) = (tt16Bit)optionValue;
        break;

    case TM_OPTION_CGI_MAX_ARG:
/*  maximum number of CGI variables supported */
        tm_context(tvHttpdCgiMaxArgCnt) = (tt16Bit)optionValue;
        break;

#endif /* TM_USE_HTTPD_CGI */

#ifdef TM_USE_HTTPD
    case TM_OPTION_HTTPD_BODY_MAX_RCV_BUF:
/* maximum buffer size to hold CGI variable names and values, or contiguous recvd body buffer */
        tm_context(tvHttpdBodyMaxRecvBufSize) = (tt16Bit)optionValue;
        break;
#endif /* TM_USE_HTTPD */

#ifdef TM_USE_HTTPC
    case TM_OPTION_HTTPC_MAX_LINE_LENGTH:
/* maximum line length */
        if ((tt16Bit)optionValue == 0)
        {
            errorCode = TM_EINVAL;
        }
        else
        {
            tm_context(tvHttpcMaxLineLength) = (tt16Bit)optionValue;
        }
        break;
#endif /* TM_USE_HTTPC */

        default:
            errorCode = TM_EINVAL;
            break;

    }
    return(errorCode);
}

