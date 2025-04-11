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
 * Description: Treck init function
 * Filename: trinit.c
 * Author: Odile
 * Date Created: 11/17/97
 * $Source: source/trinit.c $
 *
 * Modification History
 * $Revision: 6.0.2.18 $
 * $Date: 2012/02/16 15:26:37JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>

/* To force declaration of global variables */
#define TM_GLOBAL

#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/* Lookup table used to lookup a 32-bit mask given a prefix length. */
const tt4IpAddress  TM_CONST_QLF tvRt32ContiguousBitsPtr[] = {
/*  0  */       TM_UL(0)
/*  1  */     , tm_const_htonl(TM_UL(0x80000000)) 
/*  2  */     , tm_const_htonl(TM_UL(0xC0000000))
/*  3  */     , tm_const_htonl(TM_UL(0xE0000000))
/*  4  */     , tm_const_htonl(TM_UL(0xF0000000))
/*  5  */     , tm_const_htonl(TM_UL(0xF8000000))
/*  6  */     , tm_const_htonl(TM_UL(0xFC000000))
/*  7  */     , tm_const_htonl(TM_UL(0xFE000000)) 
/*  8  */     , tm_const_htonl(TM_UL(0xFF000000))
/*  9  */     , tm_const_htonl(TM_UL(0xFF800000)) 
/* 10  */     , tm_const_htonl(TM_UL(0xFFC00000))
/* 11  */     , tm_const_htonl(TM_UL(0xFFE00000))
/* 12  */     , tm_const_htonl(TM_UL(0xFFF00000))
/* 13  */     , tm_const_htonl(TM_UL(0xFFF80000))
/* 14  */     , tm_const_htonl(TM_UL(0xFFFC0000))
/* 15  */     , tm_const_htonl(TM_UL(0xFFFE0000))
/* 16  */     , tm_const_htonl(TM_UL(0xFFFF0000))
/* 17  */     , tm_const_htonl(TM_UL(0xFFFF8000)) 
/* 18  */     , tm_const_htonl(TM_UL(0xFFFFC000))
/* 19  */     , tm_const_htonl(TM_UL(0xFFFFE000))
/* 20  */     , tm_const_htonl(TM_UL(0xFFFFF000))
/* 21  */     , tm_const_htonl(TM_UL(0xFFFFF800))
/* 22  */     , tm_const_htonl(TM_UL(0xFFFFFC00))
/* 23  */     , tm_const_htonl(TM_UL(0xFFFFFE00))
/* 24  */     , tm_const_htonl(TM_UL(0xFFFFFF00))
/* 25  */     , tm_const_htonl(TM_UL(0xFFFFFF80))
/* 26  */     , tm_const_htonl(TM_UL(0xFFFFFFC0))
/* 27  */     , tm_const_htonl(TM_UL(0xFFFFFFE0))
/* 28  */     , tm_const_htonl(TM_UL(0xFFFFFFF0))
/* 29  */     , tm_const_htonl(TM_UL(0xFFFFFFF8))
/* 30  */     , tm_const_htonl(TM_UL(0xFFFFFFFC))
/* 31  */     , tm_const_htonl(TM_UL(0xFFFFFFFE))
              , tm_const_htonl(TM_UL(0xFFFFFFFF))
};

#ifdef TM_USE_STOP_TRECK
static void tfLinkLayerListFree(
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* TM_MULTIPLE_CONTEXT */
                                );
#endif /* TM_USE_STOP_TRECK */

#ifdef TM_USE_STOP_TRECK
#ifdef TM_MULTIPLE_CONTEXT
static int tfStopTreckCB (ttNodePtr nodePtr, ttGenericUnion genParm);
#endif /* TM_MULTIPLE_CONTEXT */
#endif /* TM_USE_STOP_TRECK */

#ifndef TM_MULTIPLE_CONTEXT
/*
 * Called by the user prior to calling
 * tfStartTreck() or tfInitTreckOptions(). This routine will reset
 * the global variable tvGlobalsSet, which will ensure that the stack
 * initializes all global variables and simple heap as well, when
 * tfStartTreck() or (tfInitTreckOptions() instead) is called.
 * If TM_MULTIPLE_CONTEXT is defined, then the user should call
 * tfInitTreckMultipleContext() instead.
 */
void tfInitTreck (void)
{
    tvGlobalsSet = 0;
    return;
}
#endif /* !TM_MULTIPLE_CONTEXT */

/* First initialization routine. Initialize the stack globally */
void tfInitTreckMultipleContext (void)
{
    tvGlobalsSet = TM_GLOBAL_MAGIC_NUMBER;
/*
 * Timer initialization
 */
/* Timer tick length in milliseconds. */
    tvTimerTickLength = TM_TICK_LENGTH;

/*
 * Pend/Post enabled global variable
 */
    tvPendPostEnabled = TM_DEF_PEND_POST_STATUS;

#ifdef TM_LOCK_NEEDED
/*
 * Lock enabled global variable
 */
    tvLockEnabled = TM_DEF_LOCK_STATUS;
#endif /* TM_LOCK_NEEDED */
/*
 * Simple heap variables
 */
/* 
 * Initialize globals structure before creating the simple heap 
 * Note that if we wanted to allocated the ttGlobal structure out of the heap
 * we would need to move the simple heap variables out of the ttGlobal 
 * structure.
 */
    tm_bzero(&tvGlobals, sizeof(tvGlobals));
/*
 * Create the Treck simple heap. Only happens if TM_USE_SHEAP is defined.
 */
#ifdef TM_USE_SHEAP
    tm_global(tvSheapPageSize) = TM_SHEAP_SIZE/TM_SHEAP_NUM_PAGE;
    tm_sheap_create();
#endif /* TM_USE_SHEAP */
/* Initial File system drive */
    tvFSInitDrive = 0;
/* Initial File system working directory has not been set */
    tvFSInitWorkingDirSet = TM_8BIT_ZERO;

/*
 * Timer global variables.
 */
/* tvTime not initialized on purpose (to save value across reboots) */

/* tvIpId not initialized on purpose (to save value across reboots) */

/* Initialize device driver ISR parameters table */
    tm_bzero(&tvDeviceIsrPara[0], sizeof(tvDeviceIsrPara));
/*
 * Initialize the Kernel Interface (after heap creation, and before counting
 * semaphore creation)
 */
    tfKernelInitialize();
#ifdef TM_LOCK_NEEDED
#ifdef TM_USE_SHEAP
/* Go to operating system and get a counting semaphore */
    (void)tfLockCountSemGlobalCreate(TM_GLBL_SHEAP_LOCK_IDX);
#endif /* TM_USE_SHEAP */
#ifdef TM_USER_PACKET_DATA_ALLOC
/* Go to operating system and get a counting semaphore */
    (void)tfLockCountSemGlobalCreate(TM_GLBL_PKTDATA_LOCK_IDX);
#endif /* TM_USER_PACKET_DATA_ALLOC */
#endif /* TM_LOCK_NEEDED */

#ifdef TM_MULTIPLE_CONTEXT
/*
 * No current context yet. User will allocate. 
 */
    tvCurrentContext = (ttContextPtr)0;
    tfListInit(&(tm_global(tvContextHead)));
#else /* !TM_MULTIPLE_CONTEXT */
/* single context is not initialized yet */
    tm_context(tvContextGlobalsSet) = TM_UL(0);
#endif /* !TM_MULTIPLE_CONTEXT */
/* MBUF initilization for BSD multi domain support */
#ifdef TM_USE_BSD_DOMAIN
    tfBsdMbufInit();
#endif /* TM_USE_BSD_DOMAIN */

#ifdef TM_USE_DOS_FS
    tfInitDosFs();
#endif /* TM_USE_DOS_FS */
#ifdef TM_USE_EMU_UNIX_FS
    tfInitEmuUnixFs();
#endif /* TM_USE_EMU_UNIX_FS */
}


/*
 * First initialization routine per context. 
 * Initialize context global variables settable by the user.
 */
void tfContextGlobalVarsInit (void)
{
#ifndef TM_MULTIPLE_CONTEXT
    tm_bzero(&tvCurrentContextStruct, sizeof(ttContext));
#endif /* TM_MULTIPLE_CONTEXT */
/* Low Water Mark for notifying device send completes. Accessed in ISR. */
    tm_context(tvNotifySendCompLowWater) = TM_NOTIFY_SEND_LOW_WATER;
/* 
 * Maximum number of timers we execute in a single call to tfTimerExecute,
 * a value of 0 means there is no limit. tvTimerMaxExecute initialized
 * to 0 by default.
 */
/* Number of times to try an ARP request */
    tm_context(tvArpRequestTries) = TM_ARP_REQUEST_TRIES;
    tm_context(tvArpRetryTime) = TM_ARP_RETRY_TIME;
    tm_context(tvArpQuietTime) = TM_ARP_QUIET_TIME;
#if (defined(TM_USE_IPV4) && defined(TM_USE_AUTO_IP))
    tm_ip_copy(TM_IP_LOCAL_FIRST, tm_context(tvAutoIpAddress));
#endif /* TM_USE_IPV4 && TM_USE_AUTO_IP */

/* Timer context global variables */
#ifdef TM_USE_TIMER_CACHE
#ifdef TM_USE_TIMER_WHEEL
/* Maximum timer interval on the timer wheel */
    tm_context(tvTimerMaxWheelIntv) = TM_TIM_MAX_WHEEL_INTV;
#endif /* TM_USE_TIMER_WHEEL */
#endif /* TM_USE_TIMER_CACHE */
    
/*
 * Socket context global variables.
 */
/* Maximum number of sockets.*/
    tm_context(tvMaxNumberSockets) = TM_SOC_INDEX_MAX;
#ifdef TM_USE_SOCKET_HASH_LIST
#ifdef TM_USE_TCP
/* TCP socket table hash list size */
    tm_context(tvSocketTableListHashSize[TM_SOTB_TCP_CON]) =
                                             TM_SOTB_TCP_CON_HASH_SIZE;
#endif /* TM_USE_TCP */
/* Non TCP sockets table hash list size */
    tm_context(tvSocketTableListHashSize[TM_SOTB_NON_CON]) =
                                             TM_SOTB_NON_CON_HASH_SIZE;
#endif /* TM_USE_SOCKET_HASH_LIST */
#ifdef TM_USE_TCP
    tm_context(tvTcpTmWtTableHashSize) = TM_SOTB_TCP_CON_HASH_SIZE;
/*
 * TCP socket table:
 * Threshold of number of sockets in the socket lookup table before we switch
 * from a random local port selection to a sequential local port selection
 */
    tm_context(tvSocketThresholdSequentialPort[TM_SOTB_TCP_CON]) =
                                             TM_SOTB_TCP_CON_PORT_THRS;
#endif /* TM_USE_TCP */
/* 
 * non TCP sockets table:
 * Threshold of number of sockets in the socket lookup table before we switch
 * from a random local port selection to a sequential local port selection
 */
    tm_context(tvSocketThresholdSequentialPort[TM_SOTB_NON_CON]) =
                                             TM_SOTB_NON_CON_PORT_THRS;
/* Number of open sockets, zero by default */
#ifdef TM_USE_TCP
/* Maximum number of allocated TCP sockets, not including in TIME_WAIT.*/
    tm_context(tvMaxTcpVectAllocCount) = TM_TCPVECT_MAX_COUNT;
/* Number of allocated TCP sockets, zero by default */
/* Maximum number of allocated TCP Time Wait Vectors */
    tm_context(tvMaxTcpTmWtAllocCount) = TM_TCPTMWT_MAX_COUNT;
/* Number of allocated TCP Time Wait Vectors, zero by default */
#endif /* TM_USE_TCP */
/* Default receive queue size in bytes */
    tm_context(tvDefRecvQueueBytes) = TM_SOC_RECV_Q_BYTES;
/* Default send queue size in bytes */
    tm_context(tvDefSendQueueBytes) = TM_SOC_SEND_Q_BYTES;
/* Default receive queue size in datagrams (non-TCP) */
    tm_context(tvDefRecvQueueDgrams) = TM_SOC_RECV_Q_DGRAMS;
/* Default send queue size in datagrams (non-TCP) */
    tm_context(tvDefSendQueueDgrams) = TM_SOC_SEND_Q_DGRAMS;

/* Default socket options */
    tm_context(tvSocDefOptions) = TM_SOC_DEF_OPTIONS;
/*
 * Threshold in bytes below which we try and append to previous send buffer
 * for TCP
 */
    tm_context(tvDefSendAppendThreshold) = TM_SOC_SEND_APPEND_THRESHOLD;
/*
 * Fraction use of the buffer below which we try and copy to a smaller
 * recv buffer for UDP
 */
     tm_context(tvDefRecvAppendFraction) = TM_SOC_RECV_APPEND_FRACTION;
/*
 * Fraction use of the buffer below which we try and copy to a smaller
 * recv buffer for UDP
 */
    tm_context(tvDefRecvCopyFraction) = TM_SOC_RECV_COPY_FRACTION;
/*
 * Default out of band data queue size (beyond the default send queue size in
 * bytes).
 */
    tm_context(tvDefSendOobQueueBytes) = TM_SOC_SEND_OOB_Q_BYTES;
/* Default Linger time when closing a TCP socket */
    tm_context(tvDefLingerTime) = TM_SOC_LINGER_TIME;
/*
 * Arp global variables.
 */
/* Maximum number of ARP entries */
    tm_context(tvArpMaxEntries) = TM_ARP_MAX_ENTRIES;
/* Default ARP TIMEOUT for an ARP entry 600 seconds (10 minutes) */
    tm_context(tvArpTimeout) = TM_ARP_TIMEOUT; /* in milliseconds */
/* ARP aging timer resolution 60 seconds (1 minute) */
    tm_context(tvArpResolution) = TM_ARP_RESOLUTION; /* in milliseconds */

#ifdef TM_USE_IPV4
/* ARP smart cache option that allows us to store all ARP */
    tm_context(tvArpSmart) = TM_ARP_DEF_SMART; /* Default on */
#endif /* TM_USE_IPV4 */

/*
 * Router global variables.
 */
/* default router id 0.0.0.0 */
    tm_ip_copy(TM_IP_ROUTER_ID, tm_context(tvRouterId));

#ifdef TM_USE_IPV4
/* 0, or 1. Default is RIP disabled (0)*/
    tm_context(tvRipOn) = TM_RIP_DEF;
/* default is sending RIP 2 broadcasts */
    tm_context(tvRipSendMode) = TM_RIP_DEF_SEND_MODE;
/* default is receiving RIP 2 packets only */
    tm_context(tvRipRecvMode) = TM_RIP_DEF_RECV_MODE;
#endif /* TM_USE_IPV4 */

/* Maximum number of dynamic routing entries */
    tm_context(tvRtMaxEntries) = TM_RT_MAX_ENTRIES;
/*
 * Aging route timer resolution (to age RIP, REDIRECT, ROUTER ADV entries.. )
 * Default value is 30 seconds.
 */
/* in milliseconds */
    tm_context(tvRtTimerResolution) = TM_RT_TIMER_RESOLUTION;
/*
 * Route entry timeout value for RIP, REDIRECT.
 * Default value is 180 seconds (3 minutes).
 */
    tm_context(tvRtTimeout) = TM_RT_TIMEOUT; /* in milliseconds */

#ifdef TM_PMTU_DISC
/*
 * Path MTU host routing entry timeout value when the path mtu estimate
 * is decreased.
 */
/* In milliseconds */
    tm_context(tvRtDecreasedMtuTimeout) = TM_RT_DECREASED_MTU_TIMEOUT;
/*
 * Path MTU host routing entry timeout value when attempting a
 * larger MTU.
 */
/* in milliseconds */
    tm_context(tvRtLargerMtuTimeout) = TM_RT_LARGER_MTU_TIMEOUT;
#endif /* TM_PMTU_DISC */

#ifdef TM_USE_IPV4
/*
 * User DHCP global variable. Can only be changed prior to calling
 * tfStartTreck().
 */
    tm_context(tvMaxUserDhcpEntries) = TM_MAX_USER_DHCP_ENTRIES;

/*
 * IP global variables.
 */
/* Default is no forwarding of IP packets */
    tm_context(tvIpForward) = TM_IP_DEF_FORWARD;
/*
 * Default is forwarding of directed broadcast of IP packets
 * (if tvIpForward is set)
 */
    tm_context(tvIpDbcastForward) =  TM_IP_DEF_DBCAST_FORWARD;
#ifdef TM_IP_MCAST_FORWARD
/* forward multicast IP packets (always 0): no forwarding of multicast */
    tm_context(tvIpMcastForward) = TM_IP_DEF_MCAST_FORWARD;
#endif /* TM_IP_MCAST_FORWARD */
#ifdef TM_USE_LBCAST_CONFIG
    tm_context(tvIpLbcastEnable) = TM_IP_DEF_LBCAST_ENABLE;
#endif /* TM_USE_LBCAST_CONFIG */
/* Default IP time to live (64) */
    tm_context(tvIpDefTtl) = TM_IP_DEF_TTL;
/* Default IP TOS */
    tm_context(tvIpDefTos) = TM_IP_DEF_TOS;
/* No Ip forward call back function. Zero by default */
#ifdef TM_IP_FRAGMENT
/*
 * IP Fragmentation
 */
/* Default is fragmentation allowed */
    tm_context(tvIpFragment) = TM_IP_DEF_FRAGMENT;
#endif /* TM_IP_FRAGMENT */
#ifdef TM_IP_REASSEMBLY
/*
 * IP Reassembly
 */
/* Default reassembly timer resolution is 1 second */
    tm_context(tvFragResolution) = TM_IP_DEF_FRAG_RES; /* in milliseconds */
/* Maximum size of an IP datagram waiting to be reassembled */
    tm_context(tvFragEntryMaxSize) = TM_IP_FRAG_ENTRY_MAX_SIZE;
/* Maximum number of IP datagrams waiting to be reassembled */
    tm_context(tvFragMaxQSize) = TM_IP_FRAG_Q_SIZE;
/* Maximum number of IP datagrams that were too big to be reassembled to
 * track, in case more fragments come in */
    tm_context(tvFragFailedMaxQSize) = TM_IP_FRAG_FAILED_Q_SIZE;
#endif /* TM_IP_REASSEMBLY */
#if defined(TM_IP_REASSEMBLY) || defined(TM_SNMP_VERSION)
/* Default reassembly timeout value in seconds. Default is 64.  */
    tm_context(tvFragTtl) = TM_IP_DEF_FRAG_TTL;
#endif /* TM_IP_REASSEMBLY || TM_SNMP_VERSION */ 

/*
 * ICMP global variables
 */
/* Default is no ICMP address mask agent */
    tm_context(tvIcmpAddrMaskAgent) = TM_ICMP_DEF_ADDRMAGENT;

#ifdef TM_USE_ECHO_CONFIG
/* Default is to allow ICMP echo responses */
    tm_context(tvIcmpEchoEnable) = TM_ICMP_DEF_ECHO_ENABLE;
#endif /* TM_USE_ECHO_CONFIG */

/*
 * ICMP Initialization. tvIcmpSeqno, 0 by default. Should be random number.
 */

/*
 * SLIP variables
 */
    tm_context(tvSlipMtu) = TM_SLIP_MTU;
#endif /* TM_USE_IPV4 */

/*
 * UDP global variables
 */
/* UDP checksum turned on by default */
    tm_context(tvUdpChecksumOn) = TM_UDP_DEF_CHECKSUM;
/* UDP Incoming checksum check turned on by default */
    tm_context(tvUdpIncomingChecksumOn) = TM_UDP_DEF_INCOMING_CHECKSUM;

#ifdef TM_USE_TCP
/*
 * TCP global variables
 */
/* TCP Incoming checksum check turned on by default */
    tm_context(tvTcpIncomingChecksumOn) = TM_TCP_DEF_INCOMING_CHECKSUM;
/* Default MSS to use when peer does not send an MSS */
    tm_context(tvTcpDefMss) = TM_TCP_DEF_MSS;
/* User can change tvTcpDefOptions for all connections or via a set socket
 * option for a given connection. Default is:
 * window scale option on, selective ACK option on, Time stamp option on,
 * plus congestion avoidance slow start on, RFC1122 URG pointer mode.
 */
    tm_context(tvTcpDefOptions) = TM_TCP_DEF_OPTIONS;
/*
 * Note: user can set tvTcpDelayAckTime for all connections here,
 * or use socket option to set it for individual connections.
 * Default is 200 ms. Setting it to 0 disables delay ACK, and causes an ACK
 * to be sent for every received packet.
 */
    tm_context(tvTcpDelayAckTime) = TM_TCP_DELAY_ACK_TIME; /* default 200 ms */
/* Tcp connection time, default 75 seconds */
    tm_context(tvTcpConnTime) = TM_TCP_CONN_TIME;
/* Default Tcp retransmission time (default milliseconds) */
    tm_context(tvTcpDefRtoTime) = TM_TCP_DEF_RTO_TIME;
/* min Tcp retransmission time (in milliseconds) */
    tm_context(tvTcpMinRtoTime) = TM_TCP_MIN_RTO_TIME;
/* max Tcp retransmission time (in milliseconds) */
    tm_context(tvTcpMaxRtoTime) = TM_TCP_MAX_RTO_TIME;
/* Max retransmission count (default 12) */
    tm_context(tvTcpMaxReXmitCnt) = TM_TCP_MAX_REXMIT_CNT;
/* min Tcp zero window probe time (in milliseconds) */
    tm_context(tvTcpMinProbeWndTime) = TM_TCP_MIN_PROBEWINDOW_TIME;
/* max Tcp zero window probe time (in milliseconds) */
    tm_context(tvTcpMaxProbeWndTime) = TM_TCP_MAX_PROBEWINDOW_TIME;
/* Amount of time to stay in FIN WAIT 2 when socket has been closed
 * (10 minutes) in seconds.
 */
    tm_context(tvTcpFinWt2Time) = TM_TCP_FINWAIT2_TIME;
/*
 * 2*Maximum segment life time (2*30 seconds) (TIME WAIT time) in
 * milliseconds
 */
    tm_context(tvTcp2MSLTime) = TM_TCP_2MSL_TIME;
/* Keep alive timer (2 hours) in seconds */
    tm_context(tvTcpKeepAliveTime) = TM_TCP_KEEPALIVE_TIME;
/* Keep alive Probe Interval/Idle update time (75s) in seconds. */
    tm_context(tvTcpIdleIntvTime) = TM_TCP_IDLE_INTV;
/* Keep alive Max Probe count (8) */
    tm_context(tvTcpKeepAliveProbeCnt) = TM_TCP_KEEPALIVE_PROBECNT;
#endif /* TM_USE_TCP */

    tm_context(tvContextGlobalsSet) = TM_GLOBAL_MAGIC_NUMBER;

/* Initialize default trailer size (usually zero) */
    tm_context(tvPakPktSendTrailerSize) =
        (ttPktLen) tm_packed_byte_count(TM_DEF_SEND_TRAILER_SIZE);

#ifdef TM_USE_IPV6
/* IPv6 global variables */
#ifdef TM_6_USE_PREFIX_DISCOVERY
/* prefixing aging timer resolution */
    tm_context(tv6PrefixTimerResSec) = (tt16Bit) TM_6_PREFIX_TIMER_RES_SEC;
#endif /* TM_6_USE_PREFIX_DISCOVERY */
/* ([RFC2463].R2.4:110 updated by [RFC4443].R2.4(f)) */
    tm_context(tv6IcmpTxErrLimitMsec) = TM_6_ICMP_DEF_TX_ERR_LIMIT_MSEC;
    tm_context(tv6IcmpTxErrLimitBurst) = TM_6_ICMP_DEF_TX_ERR_LIMIT_BURST;
    tm_context(tv6IpForward) = TM_6_IP_DEF_FORWARD;
#ifdef TM_6_IP_FRAGMENT
/* ([RFC2460].R5:70) */
    tm_context(tv6IpFragment) = TM_6_IP_DEF_FRAGMENT;
#endif /* TM_6_IP_FRAGMENT */
/* ([RFC2462].R5.5.4:70) */
    tm_context(tv6IpDeprecateAddr) = TM_6_IP_DEF_DEPRECATE_ADDR;
    tm_context(tv6NdUnicastRequestTries) = TM_6_ND_MAX_UNICAST_SOLICIT;
    tm_context(tv6NdMcastRequestTries) = TM_6_ND_MAX_MCAST_SOLICIT;
#ifdef TM_6_USE_ARP_QUIET_TIME
    tm_context(tv6NdQuietTime) = TM_6_ND_QUIET_TIME;
#endif /* TM_6_USE_ARP_QUIET_TIME */

#ifdef TM_6_PMTU_DISC
    tm_context(tv6RtPathMtuTimeout) = TM_6_RT_PMTU_TIMEOUT;
#endif /* TM_6_PMTU_DISC */
    
#ifdef TM_6_IP_REASSEMBLY
/*
 * IPv6 Fragment Reassembly
 */
/* default reassembly timer resolution is 1 second */
    tm_context(tv6FragResolution) = TM_6_IP_DEF_FRAG_RES; /* in milliseconds */
/* Maximum size of an IP datagram waiting to be reassembled */
    tm_context(tv6FragEntryMaxSize) = TM_6_IP_FRAG_ENTRY_MAX_SIZE;
/* Maximum number of IP datagrams that were too big to be reassembled to
 * track, in case more fragments come in */
    tm_context(tv6FragFailedMaxQSize) = TM_6_IP_FRAG_FAILED_Q_SIZE;
/* Maximum number of IP datagrams waiting to be reassembled */
    tm_context(tv6FragMaxQSize) = TM_6_IP_FRAG_Q_SIZE;
#endif /* TM_6_IP_REASSEMBLY */
#if defined(TM_6_IP_REASSEMBLY) || defined(TM_SNMP_VERSION)
/* Default reassembly timeout value in seconds. Default is 64. */
    tm_context(tv6FragTtl) = TM_6_IP_DEF_FRAG_TTL;
#endif /* TM_6_IP_REASSEMBLY || TM_SNMP_VERSION */ 
#ifdef  TM_6_USE_MIP_RO
    tm_context(tv6CnMaxBindingLifeMsec)
        = TM_6_CN_DEF_MAX_BINDING_LIFE * 1000;
/* ([MIPV6_18++].R14.4.5:20) */
    tm_context(tv6MipRoEnable) = (tt8Bit) TM_6_MIP_DEF_RO_ENABLE;
#endif /* TM_6_USE_MIP_RO */

#ifdef TM_6_USE_MIP_MN
    tm_context(tv6MnRegLtBiasMsec) = TM_6_MN_DEF_REG_LT_BIAS * 1000;
    tm_context(tv6MnDeregTimeoutMsec) = TM_6_MN_DEF_DEREG_TIMEOUT * 1000;
    tm_context(tv6MnMaxBulEntries) = TM_6_RR_DEF_MAX_BUL_ENTRIES;
    tm_context(tv6MnBeaconThresh) = TM_6_MN_DEF_BEACON_THRESHOLD;
    tm_context(tv6InitBaTimeoutFirstRegMsec) =
        TM_6_MN_DEF_1ST_REG_INIT_TIMEOUT;
    tm_context(tv6MnEagerCellSwitch) = (tt8Bit) TM_6_MN_DEF_EAGER_CELL_SWITCH;
#endif /* TM_6_USE_MIP_MN */

#ifdef TM_6_USE_MIP_CN
    tm_context(tv6CnMaxBceEntries) = TM_6_RR_DEF_MAX_BCE_ENTRIES;
#endif /* TM_6_USE_MIP_CN */

#ifdef TM_6_USE_MIP_HA
    tm_context(tv6MipIsHa)  = TM_6_MIP_DEF_IS_HA;
#endif /* TM_6_USE_MIP_HA */
#endif /* TM_USE_IPV6 */
/* BSD multi domain, domain link list header pointer. zero by default. */

#ifdef TM_USE_HTTPD_SSI
/*  The maximum SSI tag length in the SSI enabled pages. */
    tm_context(tvHttpdSsiMaxTagLen) = TM_HTTPD_SSI_MAX_TAG_LEN;
#endif /* TM_USE_HTTPD_SSI */

#ifdef TM_USE_HTTPD_CGI
/* minium number of CGI variables per allocation. */
    tm_context(tvHttpdCgiMinArgCnt) = TM_HTTPD_CGI_MIN_ARG;
/*  maximum number of CGI variables supported */
    tm_context(tvHttpdCgiMaxArgCnt) = TM_HTTPD_CGI_MAX_ARG;
#endif /* TM_USE_HTTPD_CGI */
#ifdef TM_USE_HTTPD
/* 
 * maximum buffer size to hold CGI variable names and values, or contiguous
 * recvd body buffer.
 */
    tm_context(tvHttpdBodyMaxRecvBufSize) = (tt16Bit)
                                            TM_HTTPD_BODY_MAX_RECV_BUF;
#endif /* TM_USE_HTTPD */

#ifdef TM_USE_HTTPC
    tm_context(tvHttpcMaxLineLength) = (tt16Bit)TM_HTTPC_MAX_LINE_LENGTH;
#endif /* TM_USE_HTTPC */

#ifdef TM_USE_UPNP_DEVICE
/* The default SSDP advertisement duration */
    tm_context(tvSsdpDefaultCacheCtl) = TM_SSDP_DEF_CACHE_CTL;
#endif /* TM_USE_UPNP_DEVICE */
}

/*
 * Second initialization routine, called from tfStartTreck(). 
 * Initializes non user settable context
 * variables and call every local initialization routine. 
 * User can change user settable global/context variables before 
 * tfContextInit() is called, i.e. before tfStartTreck() is called.
 */
void tfContextInit (void)
{
#ifdef TM_DEMO_TIME_LIMIT
    ttGenericUnion  timerParm1;
#endif /* TM_DEMO_TIME_LIMIT */
#ifdef TM_USE_TIMER_CACHE
    tt32Bit         timePeriod;
    unsigned int    numberSlots;
    unsigned int    allocSize;
#endif /* TM_USE_TIMER_CACHE */
    int             i;

#ifdef TM_ERROR_CHECKING
#ifdef TM_USE_ANSI_LINE_FILE
#if defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86)
/*
 * Memory leak detection initialization 
 */
    tfMemoryLeakInit();
#endif /* defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86) */
#endif /*  TM_USE_ANSI_LINE_FILE */
#endif /* TM_ERROR_CHECKING */
/*
 * Lock initialization (in line)
 */
#if ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED)) )
/* 
 * Allocate list of pre-allocated counting semaphores, after kernel has been
 * initialized. 
 */
    (void)tfCountSemAlloc(TM_8BIT_NO);
#endif  /* ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED))  */

#ifdef TM_PEND_POST_NEEDED
/*
 * Select pend entry free list initialization, after heap creation
 */
    tfListInit(&tm_context(tvSelPendEntryHead));
#endif /* TM_PEND_POST_NEEDED */

#if (defined(TM_DEBUG_LOGGING) || defined(TM_TRACE_LOGGING) || \
     defined(TM_ERROR_LOGGING) || defined(TM_MEMORY_LOGGING) || \
     defined(TM_USE_LOGGING_LEVELS))
/* support for debug, error and trace logging to circular buffer */
    tm_context(tvLogCtrlBlkPtr) = 
        tfLogAllocBuf(TM_DEF_MAX_LOG_MSG_LEN, TM_DEF_NUM_LOG_MSGS);
#endif
#ifdef TM_USE_LOGGING_LEVELS
    tfInitEnhancedLogging();
#endif /* TM_USE_LOGGING_LEVELS */
#ifdef TM_SNMP_CACHE
    for (i = 0; i < TM_SNMPD_CACHES ; i++)
    {
        tfListInit(&(tm_context(tvSnmpdCaches)[i].snmpcList));
    }
#endif /* TM_SNMP_CACHE */
/*
 * Timer Initialization.
 * Variables Initialized to zero by default.
 * Initialize timer wheel, and circular doubly linked lists in the wheel.
 * Initialize circular doubly linked lists outside of the timer wheel.
 */
#ifdef TM_USE_TIMER_CACHE
#ifdef TM_USE_TIMER_WHEEL
/*
 * number of wheel slots; Round up tickLength + add extra slot for current
 * time
 */
    timePeriod = (tt32Bit)
            (   (tt32Bit)(   tm_context(tvTimerMaxWheelIntv)
                           + (2 * tvTimerTickLength) - 1 )
              / tvTimerTickLength );
    numberSlots = (unsigned int)timePeriod;
    tm_context(tvTimerCtrl).timerWheel.twlNumberSlots = (tt16Bit)numberSlots;
/* Allocate one list per slot on the timer wheel */
    allocSize = sizeof(ttList) * numberSlots;
    tm_context(tvTimerCtrl).timerWheel.twlListPtr =
                                        tm_get_raw_buffer(allocSize);
    if (tm_context(tvTimerCtrl).timerWheel.twlListPtr == (ttListPtr)0)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelError("tfContextInit","FATAL: Unable to malloc timer wheel");
#endif /* TM_ERROR_CHECKING */
        tm_thread_stop;
    }
    tm_bzero(tm_context(tvTimerCtrl).timerWheel.twlListPtr, allocSize);
/* Initialize timer wheel lists */
    for (i = 0; i < (int)numberSlots; i++)
    {
        tfListInit(&(tm_context(tvTimerCtrl).timerWheel.twlListPtr[i]));
    }
#endif /* TM_USE_TIMER_WHEEL */
#endif /* TM_USE_TIMER_CACHE */
    for (i = 0; i < TM_TIMER_LISTS; i++)
    {
        tfListInit(&(tm_context(tvTimerCtrl).timerList[i].tmlList));
#ifdef TM_USE_TIMER_CACHE
        switch (i)
        {
#ifdef TM_USE_TIMER_WHEEL
            case 1:
                tm_context(tvTimerCtrl).timerList[i].tmlMinInterval =
                                            tm_context(tvTimerMaxWheelIntv);
                break;
#endif /* TM_USE_TIMER_WHEEL */
#ifdef TM_USE_TIMER_MULT_ACTIVE_LIST 
            case 2:
                tm_context(tvTimerCtrl).timerList[i].tmlMinInterval =
                                        2 * tm_context(tvTimerMaxWheelIntv);
                break;
            case 3:
                tm_context(tvTimerCtrl).timerList[i].tmlMinInterval =
                                        8 * tm_context(tvTimerMaxWheelIntv);
                break;
            case 4:
                tm_context(tvTimerCtrl).timerList[i].tmlMinInterval =
                                        64 * tm_context(tvTimerMaxWheelIntv);
                break;
#endif /* TM_USE_TIMER_MULT_ACTIVE_LIST  */
            default:
                break;
        }
#endif /* TM_USE_TIMER_CACHE */
    }
/*
 * Socket initialization
 */
    tfRandSeed(0xa5f0, 0xf0a5);
/*
 * Initialize the socket array, and trees/tables.
 */
    tfSocketInit(tm_context(tvMaxNumberSockets));
/*
 * Link Layer Initialization. Initialized to zero by default.
 * The link layer is done here because the User installs the link layers
 */
/*
 * Router initialization 
 */
    tfRtInit(); /* Before device initialization */
/* 
 * Proxy ARP initialization. Initialized to zero by default.
 */
/*
 * Collision Detection initialization. Initialized to zero by default.
 */
/*
 * Device initialization (after tfRtInit()).Initialized to zero by default.
 */
/*
 * IP initialization:
 * IP forward single routing cache, non socket IP send single routing cache,
 * IP send single routing cache lock. Initialized to zero by default.
 */
/*
 * IP reassembly initialization. Initialized to zero by default.
 */

/*
 * IGMP initialization
 */
#if (defined(TM_IGMP) && defined(TM_USE_IPV4))
    tfIgmpInit();
#endif /* defined(TM_IGMP) && defined(TM_USE_IPV4) */

/*
 * Raw socket initialization. Initialized to zero by default.
 */

/*
 * UDP initialization. Initialized to zero by default.
 */

#ifdef TM_USE_IPV4
/*
 * RIP initialization
 */
    tm_context(tvRipSocketDescriptor) = TM_SOCKET_ERROR;
#ifndef TM_USE_STRONG_ESL
/*
 * BOOTP/DHCP initialization
 */
    tm_context(tvBootSocketDescriptor) = TM_SOCKET_ERROR;
    tm_context(tvBootRelayAgentDescriptor) = TM_SOCKET_ERROR;
#endif /* TM_USE_STRONG_ESL */
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV4
#ifndef TM_USE_PREDICTABLE_IPID
    tfGetRandomBytes((tt8BitPtr)&tm_context(tvIpId),
                     sizeof(tm_context(tvIpId)));
#endif /* !TM_USE_PREDICTABLE_IPID */
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_TCP
/*
 * TCP initialization
 */
    tfTcpInit();
#endif /* TM_USE_TCP */

#ifdef TM_USE_FTPD
/*
 * FTPD initialization
 */
/* No ftp server listening socket yet */
    tm_context(tvFtpdListenDesc) = TM_SOCKET_ERROR;
/* Initialize Run Q, and List of FTPD open connections so they be both empty */
    for (i = 0; i < 2; i++)
    {
        tm_context(tvFtpdConQHead).servHeadNextQEntryPtr(i) =
                                                &(tm_context(tvFtpdConQHead));
        tm_context(tvFtpdConQHead).servHeadPrevQEntryPtr(i) =
                                                &(tm_context(tvFtpdConQHead));
    }
/* End of FTPD initialization */
#endif /* TM_USE_FTPD */

#ifdef TM_USE_TELNETD
/*
 * TELNETD initialization
 */
/* No Telnet server listening socket yet */
    tm_context(tvTeldListenDesc) = TM_SOCKET_ERROR;
/* 
 * Initialize Run Q, and List of TELNETD open connections so they be both 
 * empty 
 */
    for (i = 0; i < 2; i++)
    {
        tm_context(tvTeldConQHead).servHeadNextQEntryPtr(i) =
                                                &tm_context(tvTeldConQHead);
        tm_context(tvTeldConQHead).servHeadPrevQEntryPtr(i) =
                                                &tm_context(tvTeldConQHead);
    }
/* End of TELNETD initialization */
#endif /* TM_USE_TELNETD */

/* HTTPD initialization */
#ifdef TM_USE_HTTPD
/* This is the max value of of numberServers allowed to in tfHttpdUserInit */
    tm_context(tvHttpdUpperMaxNumberServers) = TM_HTTPD_DEF_MAX_SERVERS;
#endif /* TM_USE_HTTPD */

#ifdef TM_SNMP_MIB
#ifdef TM_USE_TCP
/*
 * Initialization of SNMP Agent Mib variables
 */
/* Van Jacobson's Algorithm */
    tm_context(tvTcpData).tcpRtoAlgorithm = (ttS32Bit)4;
    tm_context(tvTcpData).tcpRtoMin = (ttS32Bit)tm_context(tvTcpMinRtoTime);
    tm_context(tvTcpData).tcpRtoMax = (ttS32Bit)tm_context(tvTcpMaxRtoTime);
    if (tm_context(tvMaxTcpVectAllocCount) < tm_context(tvMaxNumberSockets))
    {
        tm_context(tvTcpData).tcpMaxConn =
            (ttS32Bit)tm_context(tvMaxTcpVectAllocCount);
    }
    else
    {
        tm_context(tvTcpData).tcpMaxConn =
            (ttS32Bit)tm_context(tvMaxNumberSockets);
    }
#endif /* TM_USE_TCP */
#endif /* TM_SNMP_MIB */

/* Indicate that the user hasn't called tfDnsInit() yet. */
    tm_context(tvDnsInitialized) = TM_8BIT_NO;

/*
 * LLMNR
 * Initializations of LLMNR paramters
 */
#ifdef TM_USE_LLMNR_SENDER
/* Initialize sender options pointer with null */
    tm_context(tvLlmnrSenderOptPtr) = (ttVoidPtr)0;
#endif /* TM_USE_LLMNR_SENDER */
#ifdef TM_USE_LLMNR_RESPONDER
/* Initialize Responder socket with TM_SOCKET_ERROR */
    tm_context(tvLlmnrResponderSocket) = TM_SOCKET_ERROR;
/* Initialize Responder options pointer with null */
    tm_context(tvLlmnrResponderOptPtr) = (ttVoidPtr)0;
#endif /* TM_USE_LLMNR_RESPONDER */

#ifdef TM_USE_IPV4
/* Initialize IP tunnel handling function (initially disabled) */
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPSEC
/* IPsec global variable initialize */
#ifdef TM_USE_IPSEC_TASK
    tfListInit(&tm_context(tvIpsecTaskList));
#endif /* TM_USE_IPSEC_TASK */
#ifdef TM_USE_IPSEC_HANDOFF_TASK
    tfListInit(&tm_context(tvIpsecHandoffTaskList));
#endif /* TM_USE_IPSEC_HANDOFF_TASK */
#endif /* TM_USE_IPSEC */

#ifdef TM_USE_ASYNC_CRYPTO
    tfKernelCreateEvent(&tm_context(tvAsyncCryptoEvent).eventUnion);
#endif /* TM_USE_ASYNC_CRYPTO */

#ifdef TM_USE_IP_REASSEMBLY_TASK
    tfListInit(&tm_context(tvIpReassemblyTaskList));
#endif /* TM_USE_IP_REASSEMBLY_TASK */


#ifdef TM_SNMP_MIB
/* SNMP Agent global variable initialize. zero by default. */
#endif /* TM_SNMP_MIB */
#ifdef TM_SNMP_CACHE
/* 
 * Initialize SNMP agent caches for ARP table, routing table, 
 * TCP vector table, and UDP socket table. Zero by default.
 */
#endif /* TM_SNMP_CACHE */
/*
 * Loopback initialization. Has to be after SNMP agent caches initialization,
 * since tfUseLoopBack() inserts entries in the routing table.
 */
#if (!defined(TM_SINGLE_INTERFACE_HOME) && !defined(TM_SINGLE_INTERFACE))
    (void)tfUseLoopBack();
#endif /* !TM_SINGLE_INTERFACE_HOME and !TM_SINGLE_INTERFACE */

#ifdef TM_USE_IPV6
/* IPv6 global variables */
    tm_kernel_set_critical;
    tm_context(tv6IcmpTxErrLastTickCnt) = tvTime;
    tm_kernel_release_critical;
    tm_context(tv6IcmpTxErrTokenCnt) = tm_context(tv6IcmpTxErrLimitBurst);
#endif /* TM_USE_IPV6 */

#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
/* When operating in IPv6/Ipv4 dual-stack mode, enable automatic tunnels:
   ([RFC2893].R5:10) */
    tm_context(tv6Ipv4AutoTunnelPtr) = tf6InitAutoTunnels();
    
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */

#if (defined(TM_USE_IPV6) || defined(TM_IGMP))
/* tvEnetMcastMaxEntries initialized to TM_BUF_Q1_SIZE/8 */
    tm_context(tvEnetMcastBlockEntries)   = TM_DEF_ENET_MCAST_ENTRIES;
    tm_context(tvEnetMcastMaxAllocations) = TM_DEF_ENET_MCAST_ALLOCATIONS;
#endif /* TM_USE_IPV6 || TM_IGMP */

#ifdef TM_USE_IPV6

#ifdef TM_6_USE_MIP_CN
/* Mobile IPv6 correspndent node initialization */
/* initialize the binding cache */
    tfListInit(&tm_context(tv6MipBindingList));

/* create the first set of nonce and Kcn */
    tf6MipRegenNonceKcn(0);
#endif /* TM_6_USE_MIP_CN */

#ifdef TM_6_USE_MIP_MN
#ifdef TM_USE_IKE
    tf6MnInit();
#endif /* TM_USE_IKE */
#endif /* TM_6_USE_MIP_MN */

#endif /* TM_USE_IPV6 */

#ifdef TM_USE_UPNP_DEVICE
/* do one-time UPnP initialization that must be done before the user calls
   tfUpnpUserStart */
    tfUpnpInit();
#endif /* TM_USE_UPNP_DEVICE */
/* DHCPv6 Initialization */
#ifdef TM_USE_IPV6
#ifdef TM_6_USE_DHCP
/* DHCPv6 socket not opened */
    tm_context(tv6DhcpSocket) = TM_SOCKET_ERROR;
#endif /* TM_6_USE_DHCP */
#endif /* TM_USE_IPV6 */
#ifdef TM_DEMO_TIME_LIMIT
    timerParm1.gen32bitParm = (tt32Bit)0;
    (void)tfTimerAddExt(&tm_context(tvDemoTimer),
                        tfTimerDemoCallBack,
                        (ttTmCUFuncPtr)0,
                        timerParm1,
                        timerParm1, /* unused */
                        TM_DEMO_LIMIT_SEC * TM_UL(1000), /* every hour */
                        TM_TIM_AUTO );
#endif /* TM_DEMO_TIME_LIMIT */
}

/*
 * Setup Treck Protocol Stack
 */
int tfStartTreck(void)
{
    int             errorCode;
    tt32Bit         networkLong;
    
    errorCode = TM_ENOERROR;
#ifndef TM_MULTIPLE_CONTEXT
    if (tvGlobalsSet != TM_GLOBAL_MAGIC_NUMBER)
    {
/*
 * If user configured multiple contexts, this should be called
 * directly by the user once, before the user creates a context,
 * and calls tfStartTreck() for a context.
 * In non multiple context we call the initialization routine
 * once (either here or in tfInitSetOptions()).
 */
        tfInitTreckMultipleContext();
    }
/* In single context, only one call to tfStartTreck() */
    tvGlobalsSet = TM_UL(0);
#else /* TM_MULTIPLE_CONTEXT */
    if (    (tvGlobalsSet != TM_GLOBAL_MAGIC_NUMBER)
         || (tvCurrentContext == (ttContextPtr)0) )
    {
/* User forgot to globally initialize, or create and set the current context */
        errorCode = TM_EFAULT;
    }
    else
#endif /* TM_MULTIPLE_CONTEXT */
    {
        if (tm_context(tvContextGlobalsSet) != TM_GLOBAL_MAGIC_NUMBER)
        {
            tfContextGlobalVarsInit();
        }
        tm_context(tvContextGlobalsSet) = TM_UL(0);
/* Check for correct byte order */
        networkLong = TM_NET_ENDIAN_LONG;
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
        if (    ((*((tt8BitPtr)&networkLong) >> 24) & 0xff)
             == TM_NET_ENDIAN_FIRST )
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
        if (    ((*((tt8BitPtr)&networkLong) >> 8) & 0xff)
             == TM_NET_ENDIAN_FIRST )
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
        if ( *((tt8BitPtr)&networkLong) == TM_NET_ENDIAN_FIRST )
#endif /* !TM_DSP */
        {
            if (tvTimerTickLength != 0)
            {
/* tvTimerTickLength is non zero, OK to initialize the Treck stack */
                tfContextInit();
            }
            else
            {
/* Zero tick length not permitted */
                errorCode = TM_EPERM;
#ifdef TM_ERROR_CHECKING
                tfKernelWarning( "tfStartTreck",
"Please call tfInitTreckOption with a non zero TM_OPTION_TICK_LENGTH value");
#endif /* TM_ERROR_CHECKING */
            }
        }
        else
        {
            errorCode = TM_EFAULT;
#ifdef TM_ERROR_CHECKING
#ifdef TM_LITTLE_ENDIAN
            tfKernelError( "tfStartTreck",
"Please re-compile the code with TM_LITTLE_ENDIAN removed from trsystem.h");
#else /* !TM_LITTLE_ENDIAN */
            tfKernelError( "tfStartTreck",
"Please re-compile the code with TM_LITTLE_ENDIAN defined in trsystem.h");
#endif /* !TM_LITTLE_ENDIAN */
#endif /* TM_ERROR_CHECKING */
        }
    }
#ifdef TM_LOG_ENB_MODULE_GEN
/* Output the Treck version number to the log  */
    tfEnhancedLogWrite(
            TM_LOG_MODULE_GEN,
            TM_LOG_LEVEL_NONE,
            "Started %s ",
            TM_PROJECT_REVISION);
#endif /* TM_LOG_ENB_MODULE_GEN */
    return (errorCode);
}

#ifdef TM_MULTIPLE_CONTEXT
ttUserContext tfCreateTreckContext (void)
{
    ttContextPtr contextPtr;

    if (tvGlobalsSet != TM_GLOBAL_MAGIC_NUMBER)
    {
        contextPtr = (ttContextPtr)0;
    }
    else
    {
        contextPtr = tm_kernel_malloc(sizeof(ttContext));
        if (contextPtr != (ttContextPtr)0)
        {
/* Not initialized yet */
            tm_bzero(contextPtr, sizeof(ttContext));
/* Append new context to global list */
            tfListAddToTail(&(tm_global(tvContextHead)),
                            &(contextPtr->tvContextNode));
/* Context identification (used when creating debug files) */
            contextPtr->tvContextId = tm_global(tvLastContextId)++;
        }
    }
    return (ttUserContext)contextPtr;
}

void tfSetCurrentContext (ttUserContext contextHandle)
{
    tvCurrentContext = (ttContextPtr)contextHandle;
}

ttUserContext tfGetCurrentContext (void)
{
    return (ttUserContext)tvCurrentContext;
}

#endif /* TM_MULTIPLE_CONTEXT */


#ifdef TM_USE_STOP_TRECK
int tfStopTreck (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext   contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                )
{
#ifdef TM_MULTIPLE_CONTEXT
    ttUserContext   savContextHandle;
#endif /* TM_MULTIPLE_CONTEXT */
    int             errorCode;

#ifdef TM_MULTIPLE_CONTEXT
    if (!(tfListMember(&(tm_global(tvContextHead)), (ttNodePtr)contextHandle)))
/* Verify that this context has been created, and not removed */
    {
/* Already removed */
        errorCode = TM_EPERM;
    }
    else
#endif /* TM_MULTIPLE_CONTEXT */
    {
#ifdef TM_MULTIPLE_CONTEXT
        savContextHandle = tfGetCurrentContext();
        tfSetCurrentContext(contextHandle);
#endif /* TM_MULTIPLE_CONTEXT */
#ifdef TM_USE_UPNP_DEVICE
        tfUpnpDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                    contextHandle
#endif /* TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_UPNP_DEVICE */
#ifdef TM_USE_TFTP
        tfTftpDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                    contextHandle
#endif /* TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_TFTP */
#ifdef TM_USE_TFTPD
        tfTftpdDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                    contextHandle
#endif /* TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_TFTPD */
#ifdef TM_USE_FTPD
        tfFtpdDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                    contextHandle
#endif /* TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_FTPD */
#ifdef TM_USE_TELNETD
        tfTeldDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                    contextHandle
#endif /* TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_TELNETD */
#ifdef TM_USE_HTTPD
        tfHttpdDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                        contextHandle
#endif /* TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_HTTPD */
        tfDnsDeInit(
#ifdef TM_MULTIPLE_CONTEXT
                        contextHandle
#endif /* TM_MULTIPLE_CONTEXT */
                );
/* Delete and free all counting semaphores. Verify that none is in use */
#if ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED)) )
        errorCode = tfCountSemDeInit(
#ifdef TM_MULTIPLE_CONTEXT
                                      contextHandle
#endif /* TM_MULTIPLE_CONTEXT */
                                     );
        if (errorCode == TM_ENOERROR)
#else  /* (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED))  */
        errorCode = TM_ENOERROR;
#endif  /* (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED))  */
        {
/* Close all devices */
            tfDeviceListClose(TM_DEVF_UNINITIALIZING);
#ifdef TM_6_USE_MIP_MN
            tf6MnDeInit(
#ifdef TM_MULTIPLE_CONTEXT
                         contextHandle
#endif /* TM_MULTIPLE_CONTEXT */
                          );
#endif /* TM_6_USE_MIP_MN */
#ifdef TM_USE_IPSEC
            tfIpsecUninitialize();
#endif /* TM_USE_IPSEC */
#ifdef TM_MULTIPLE_CONTEXT
/* Remove context from global list */
            tfListRemove(&(tm_global(tvContextHead)), (ttNodePtr)contextHandle);
#endif /* TM_MULTIPLE_CONTEXT */
#ifdef TM_PEND_POST_NEEDED
/* Free all select pre-allocated pend entries */
            tfSelectDeInit(
#ifdef TM_MULTIPLE_CONTEXT
                         contextHandle
#endif /* TM_MULTIPLE_CONTEXT */
                          );
#endif /* TM_PEND_POST_NEEDED */
#ifdef TM_6_USE_NUD
            tf6NudDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                         contextHandle
#endif /* TM_MULTIPLE_CONTEXT */
                          );
#endif /*  TM_6_USE_NUD */
/* Free all socket entries and tables */
            tfSocketDeInit(
#ifdef TM_MULTIPLE_CONTEXT
                         contextHandle
#endif /* TM_MULTIPLE_CONTEXT */
                          );
/* Free all router/arp entries + route entries */
            tfRtDeInit(
#ifdef TM_MULTIPLE_CONTEXT
                         contextHandle
#endif /* TM_MULTIPLE_CONTEXT */
                            );
#ifdef TM_USE_IPV4
#ifdef TM_IP_REASSEMBLY
            tfIpFragDeInit(
#ifdef TM_MULTIPLE_CONTEXT
                         contextHandle
#endif /* TM_MULTIPLE_CONTEXT */
                            );
#endif /* TM_IP_REASSEMBLY */
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_6_IP_REASSEMBLY
            tf6IpFragDeInit(
#ifdef TM_MULTIPLE_CONTEXT
                         contextHandle
#endif /* TM_MULTIPLE_CONTEXT */
                            );
#endif /* TM_6_IP_REASSEMBLY */
#endif /* TM_USE_IPV6 */
            tfTimerDeInit(
#ifdef TM_MULTIPLE_CONTEXT
                         contextHandle
#endif /* TM_MULTIPLE_CONTEXT */
                            );
#ifdef TM_USE_TIMER_WHEEL
            if (tm_context_var(tvTimerCtrl).timerWheel.twlListPtr !=
                                                            (ttListPtr)0)
            {
                tm_free_raw_buffer(
                            tm_context_var(tvTimerCtrl).timerWheel.twlListPtr);
                tm_context_var(tvTimerCtrl).timerWheel.twlListPtr =
                                                                  (ttListPtr)0;
            }
#endif /* TM_USE_TIMER_WHEEL */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
/* When operating in IPv6/Ipv4 dual-stack mode, free automatic tunnels: */
            if (tm_context(tv6Ipv4AutoTunnelPtr) != TM_DEV_NULL_PTR)
            {
                tm_kernel_free(tm_context(tv6Ipv4AutoTunnelPtr));
                tm_context(tv6Ipv4AutoTunnelPtr) = TM_DEV_NULL_PTR;
            }
            if (tm_context(tv6Ipv4DefGwTunnelPtr) != TM_DEV_NULL_PTR)
            {
                tm_kernel_free(tm_context(tv6Ipv4DefGwTunnelPtr));
                tm_context(tv6Ipv4DefGwTunnelPtr) = TM_DEV_NULL_PTR;
            }
#endif /* TM_USE_IPV4 */
#ifdef TM_6_USE_DHCP
#ifdef TM_6_DHCP_OLD_SET_OPTION
            tm_call_lock_wait(&tm_context(tv6DhcpGlobalLock));
            if (tm_context(tv6DhcpTransParamPtr) != (ttIntPtr)0)
            {
                tm_free_raw_buffer(tm_context(tv6DhcpTransParamPtr));
                tm_context(tv6DhcpTransParamPtr) = (ttIntPtr)0;
            }
            tm_call_unlock(&tm_context(tv6DhcpGlobalLock));
#endif /* TM_6_DHCP_OLD_SET_OPTION */
#endif /* TM_6_USE_DHCP */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV6
            tm_context(tv6McastDevPtr) = (ttDeviceEntryPtr)0;
#endif /* TM_USE_IPV6 */
            tfDeviceListFree(
#ifdef TM_MULTIPLE_CONTEXT
                             contextHandle
#endif /* TM_MULTIPLE_CONTEXT */
                            );
#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
            tm_context(tv6Ipv4TunnelLinkLayerHandle) = TM_LINK_NULL_PTR;
#endif /* (TM_USE_IPV6 && TM_USE_IPV4) */
#ifdef TM_USE_IPV6
            tm_context(tv6Ipv6TunnelLinkLayerHandle) =  TM_LINK_NULL_PTR;
#endif /* TM_USE_IPV6 */
            tfLinkLayerListFree(
#ifdef TM_MULTIPLE_CONTEXT
                             contextHandle
#endif /* TM_MULTIPLE_CONTEXT */
                                );
#if (defined(TM_DEBUG_LOGGING) || defined(TM_TRACE_LOGGING) || \
     defined(TM_ERROR_LOGGING) || defined(TM_MEMORY_LOGGING) || \
     defined(TM_USE_LOGGING_LEVELS))
/* support for debug, error and trace logging to circular buffer */
            if (tm_context_var(tvLogCtrlBlkPtr) != (ttLogCtrlBlkPtr)0)
            {
                tfLogFreeBuf(tm_context_var(tvLogCtrlBlkPtr)); 
                tm_context_var(tvLogCtrlBlkPtr) = (ttLogCtrlBlkPtr)0;
            }
#endif
            tfFreeDynamicMemory();
#ifdef TM_USE_LOGGING_LEVELS
            if (tm_context_var(tvLogLevelStoragePtr) != (ttLogLevelStoragePtr)0)
            {
                tm_kernel_free(tm_context_var(tvLogLevelStoragePtr));
                tm_context_var(tvLogLevelStoragePtr) = (ttLogLevelStoragePtr)0;
            }
#endif /* TM_USE_LOGGING_LEVELS */
#ifdef TM_ERROR_CHECKING
#ifdef TM_USE_ANSI_LINE_FILE
#if defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86)
            if (tm_context(tvMemoryLeakDetector) != (ttVoidPtr)0)
            {
                fclose(tm_context(tvMemoryLeakDetector));
                tm_context(tvMemoryLeakDetector) = (ttVoidPtr)0;
            }
#endif /* defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86) */
#endif /*  TM_USE_ANSI_LINE_FILE */
#endif /* TM_ERROR_CHECKING */
#ifdef TM_MULTIPLE_CONTEXT
            tm_kernel_free(contextHandle);
#endif /* TM_MULTIPLE_CONTEXT */
#ifdef TM_USE_SHEAP
#ifdef TM_MULTIPLE_CONTEXT
            if (tm_global(tvContextHead).listCount == 0)
#endif /* TM_MULTIPLE_CONTEXT */
/* no more context */
            {
#if defined(TM_USE_DOS_FS) || defined(TM_USE_EMU_UNIX_FS)
                if (tm_global(tvRootDirPtr) != (ttCharPtr)0)
                {
                    tm_kernel_free(tm_global(tvRootDirPtr));
                    tm_global(tvRootDirPtr) = (ttCharPtr)0;
                }
#endif /* TM_USE_DOS_FS || TM_USE_EMU_UNIX_FS */
#ifdef TM_USE_RAM_FS
                if (tm_global(tvRamfsPtr) != TM_VOID_NULL_PTR)
                {
                    tm_kernel_free(tm_global(tvRamfsPtr));
                    tm_global(tvRamfsPtr) = TM_VOID_NULL_PTR;
                }
#endif /* TM_USE_RAM_FS */
#ifdef TM_LOCK_NEEDED
#ifdef TM_USER_PACKET_DATA_ALLOC
/* Delete user packet data counting semaphore (before tfKernelUnInitialize() */
                tfLockCountSemGlobalDelete(TM_GLBL_PKTDATA_LOCK_IDX);
#endif /* TM_USER_PACKET_DATA_ALLOC */
#ifdef TM_USE_SHEAP
/* Delete simple heap counting semaphore (before tfKernelUnInitialize() */
                tfLockCountSemGlobalDelete(TM_GLBL_SHEAP_LOCK_IDX);
#endif /* TM_USE_SHEAP */
#endif /* TM_LOCK_NEEDED */
                tfKernelUnInitialize(); /* Before tfSheapDeInit() */
                tfSheapDeInit();
                tvGlobalsSet = TM_UL(0);
            }
#endif /* TM_USE_SHEAP */
        }
#ifdef TM_MULTIPLE_CONTEXT
        if (contextHandle == savContextHandle)
        {
/* Context has been freed */
            savContextHandle = (ttUserContext)0;
        }
        tfSetCurrentContext(savContextHandle);
#endif /* TM_MULTIPLE_CONTEXT */
    }
    return errorCode;
}

static void tfLinkLayerListFree (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                )
{
    ttLinkLayerEntryPtr linkLayerEntryPtr;

/* Lock the Link Layer list */
    tm_call_lock_wait(&tm_context_var(tvLinkLayerListLock));
    tm_context(tvNullLinkLayerHandle) = TM_LINK_NULL_PTR;
/* First entry */
    linkLayerEntryPtr = tm_context_var(tvLinkLayerListPtr);
    while (linkLayerEntryPtr != TM_LINK_NULL_PTR)
    {
/* remove current entry from the list */
        tm_context_var(tvLinkLayerListPtr) = linkLayerEntryPtr->lnkNextPtr;
/* Free current entry */
        tm_kernel_free(linkLayerEntryPtr);
/* Get next entry */
        linkLayerEntryPtr = tm_context_var(tvLinkLayerListPtr);
    }
    tm_call_unlock(&tm_context_var(tvLinkLayerListLock));
    return;    
}

#ifdef TM_MULTIPLE_CONTEXT
int tfStopTreckMultipleContext (void)
{
    ttGenericUnion      genParm;
    int                 errorCode;

    genParm.genIntParm = 0;
    tfListWalk(&(tm_global(tvContextHead)), tfStopTreckCB, genParm);
    if (tm_global(tvContextHead).listCount != 0)
/* We failed to stop one or more contexts */
    {
        errorCode = TM_EPERM;
    }
    else
    {
        errorCode = TM_ENOERROR;
    }
    return errorCode;
}

static int tfStopTreckCB (ttNodePtr nodePtr, ttGenericUnion genParm)
{
    TM_UNREF_IN_ARG(genParm);

    (void)tfStopTreck((ttUserContext)nodePtr);
    return TM_8BIT_NO; /* Keep walking */
}
#endif /* TM_MULTIPLE_CONTEXT */

#endif /* TM_USE_STOP_TRECK */
