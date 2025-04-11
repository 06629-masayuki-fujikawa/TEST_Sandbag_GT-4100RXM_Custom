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
 * Description: Global Definitions for trinit.c.
 * Filename: trglobal.h
 * Author: Odile & Paul
 * Date Created:
 * $Source: include/trglobal.h $
 *
 * Modification History
 * $Revision: 6.0.2.19 $
 * $Date: 2012/02/16 15:22:32JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TRGLOBAL_H_

#define _TRGLOBAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* 
 * All global variables are either in ttGlobal (true global), or in 
 * ttContext (per context variables). If user does not use multiple context
 * context variables are true global variables, but are still kept in separate
 * ttContext structure.
 */
typedef struct tsGlobal
{
#ifdef TM_USE_SHEAP
/* Simple heap variables */
    ttRawMemoryPtr                      tvSheapFreeListPtr;
    ttRawMemoryPtr                      tvSheapFragmentListPtr;
    tt32Bit                             tvSheapUsedLongs;
    tt32Bit                             tvSheapPageSize;
#endif /* TM_USE_SHEAP */

#ifdef TM_LOCK_NEEDED
#ifdef TM_USE_SHEAP
/* Simple heap lock */
#define TM_GLBL_SHEAP_LOCK_IDX      0
#define tvSheapLock                 tvLocks[TM_GLBL_SHEAP_LOCK_IDX]
/* Simple heap pre-allocated counting semaphore for lock/unlock */
#define tvSheapCountSem             tvCountSems[TM_GLBL_SHEAP_LOCK_IDX]
#else /* !TM_USE_SHEAP */
#define TM_GLBL_SHEAP_LOCK_IDX     -1 
#endif /* !TM_USE_SHEAP */

#ifdef TM_USER_PACKET_DATA_ALLOC
#define TM_GLBL_PKTDATA_LOCK_IDX (TM_GLBL_SHEAP_LOCK_IDX + 1)
/* User packet data alloc lock */
#define tvPacketDataAllocLock       tvLocks[TM_GLBL_PKTDATA_LOCK_IDX]
/* User packet pre-allocated data alloc counting semaphore for lock/unlock */
#define tvPacketDataAllocCountSem   tvCountSems[TM_GLBL_PKTDATA_LOCK_IDX]
#else /* !TM_USER_PACKET_DATA_ALLOC */
#define TM_GLBL_PKTDATA_LOCK_IDX TM_GLBL_SHEAP_LOCK_IDX
#endif /* !TM_USER_PACKET_DATA_ALLOC */

#if defined(TM_USE_SHEAP) || defined(TM_USER_PACKET_DATA_ALLOC)
#define TM_GLBL_MAX_LOCKS   (TM_GLBL_PKTDATA_LOCK_IDX + 1)
    ttLockEntry                         tvLocks[TM_GLBL_MAX_LOCKS];
    ttCountSem                          tvCountSems[TM_GLBL_MAX_LOCKS];
#endif /* TM_USE_SHEAP || TM_USER_PACKET_DATA_ALLOC */
#endif /* TM_LOCK_NEEDED */

#ifdef TM_USE_RAM_FS
/* RAM file system: RAM drive pointer */
    ttVoidPtr                           tvRamfsPtr;
    ttLockEntry                         tvRamfsLockEntry;
#endif /* TM_USE_RAM_FS */
#ifdef TM_MULTIPLE_CONTEXT
    ttList                              tvContextHead; /* head of context lists */
/* Last context ID */
    ttUser32Bit                         tvLastContextId;
/*
 * The current network ID used for multiple context interface driver
 */
    ttUser32Bit                         tvCurrNetworkId;
#endif /* TM_MULTIPLE_CONTEXT */
#if (defined TM_USE_DOS_FS) || (defined TM_USE_EMU_UNIX_FS)
/* Dos Root */
    char                       TM_FAR * tvRootDirPtr;
#endif /* TM_USE_DOS_FS || TM_USE_EMU_UNIX_FS */

#ifdef TM_LOCK_NEEDED

#ifdef TM_USE_SHEAP
/* Simple heap pre-allocated counting semaphore for lock/unlock */
#define tvSheapCountSemUsed             tvCountSemFlags[TM_GLBL_SHEAP_LOCK_IDX]
#endif /* TM_USE_SHEAP */

#ifdef TM_USER_PACKET_DATA_ALLOC
#define tvPacketDataAllocCountSemUsed   tvCountSemFlags[TM_GLBL_PKTDATA_LOCK_IDX]
#endif /* TM_USER_PACKET_DATA_ALLOC */

#if defined(TM_USE_SHEAP) || defined(TM_USER_PACKET_DATA_ALLOC)
    tt8Bit                              tvCountSemFlags[TM_GLBL_MAX_LOCKS]; 
#endif /* TM_USE_SHEAP || TM_USER_PACKET_DATA_ALLOC */

#endif /* TM_LOCK_NEEDED */

#ifdef TM_USE_TWOFISH
/* 
 * Indicates whether global TWO FISH table has been initialized
 * (Table initialized once at run time.)
 */
    tt8Bit                              tvTwoFishInit; 
#endif /* TM_USE_TWOFISH */
/* to make sure ttGlobal is not empty */
    tt8Bit                              tvDummy;
} ttGlobal;

typedef ttGlobal TM_FAR * ttGlobalPtr;

/*
 * Per context variables.
 */

typedef struct tsContext
{
#ifdef TM_MULTIPLE_CONTEXT
    ttNode                              tvContextNode; /* links in context list */
/* This context ID */        
    ttUser32Bit                         tvContextId;
#endif /* TM_MULTIPLE_CONTEXT */
#ifdef TM_ERROR_CHECKING
/* Temporary buffer used by tm_assert() to build the error message */
    char                                tvAssertTempBuf[256];
#endif  /* TM_ERROR_CHECKING */
#if ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED)) )
    ttCountSemPtr                       tvCountSemListPtrArr[2];
#define TM_COUSEM_ALLOC_INDEX  0
#define TM_COUSEM_RECYC_INDEX  1
/* Counting semaphores allocated list */
#define tvCountSemAllocListPtr          tvCountSemListPtrArr[TM_COUSEM_ALLOC_INDEX]
/* Counting semaphores recycle list */
#define tvCountSemRecycListPtr          tvCountSemListPtrArr[TM_COUSEM_RECYC_INDEX]
/* Keep track of counting semaphores currently in use */
    int                                 tvCountSemUsedCount; 
#endif  /* ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED))  */

#ifdef TM_PEND_POST_NEEDED
/* Select pend entry recycle list */
    ttList                              tvSelPendEntryHead;
#ifdef TM_LOCK_NEEDED
/* Lock for select pend entry recycle list */
    ttLockEntry                         tvSelPendEntryLock;
#endif /* TM_LOCK_NEEDED */
#endif /* TM_PEND_POST_NEEDED */
/*
 * Only used for dynamic memory management.
 */
#ifdef TM_USE_DYNAMIC_MEMORY
/*
 * Raw Memory Queues
 */
    ttRawMemoryPtr                      tvBufQListPtr[TM_BUF_Q_NUM];
/* 
 * Recycled buffer lists.
 */
    ttRcylPtr                           tvRcyListPtr[TM_RECY_NUM];
#ifdef TM_LOCK_NEEDED
/*
 * Dynamic Memory Queue Lock
 */
    ttLockEntry                         tvDynMemLockEntry;
#endif /* TM_LOCK_NEEDED */
/* Amount of memory in use by the stack (not recycled, not freed) */
    tt32Bit                             tvMemRecycledBytes;
#endif /* TM_USE_DYNAMIC_MEMORY */



#ifdef TM_USE_IPV4
#ifdef TM_USE_FILTERING
    ttUserFilterCallback                tvUserFilterCallback;
#endif /* TM_USE_FILTERING */
#ifdef TM_USE_IGMP_FILTERING
    ttUserIgmpFilterCallback            tvUserIgmpFilterCallback;
#endif /* TM_USE_IGMP_FILTERING */
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_6_USE_FILTERING
    tt6UserFilterCallback               tv6UserFilterCallback;
#endif /* TM_6_USE_FILTERING */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_TIMER_CB
    ttUserTimerCBFuncPtr                tvTimerCBFuncPtr;
    ttUserGenericUnion                  tvTimerCBParam;
#endif /* TM_USE_TIMER_CB */
#ifdef TM_USE_UPNP_DEVICE
/* lock entry used to serialize access to UPnP, SSDP and GENA */
    ttLockEntry                         tvUpnpLock;
/* run-time configurable information associated with a UPnP device instance */
    ttUpnpDeviceTable TM_FAR *          tvUpnpDeviceTable;
#endif /* TM_USE_UPNP_DEVICE */

#if (defined(TM_USE_UPNP_DEVICE) || defined(TM_USE_SOAP))
/* pointer to UPnP state vector, dynamically allocated by tfUpnpUserStart */
    ttUpnpVectPtr                       tvUpnpVectPtr;
/* run-time configurable information associated with a UPnP DCP */
    ttUpnpDcpTable TM_FAR *             tvUpnpDcpTable;
#endif /* TM_USE_UPNP_DEVICE || TM_USE_SOAP */
#ifdef TM_USE_INDRT
/* GHS INDRT debugger: pointer to INDRT specific structure. Accessed in ISR */
    ttVoidPtr                           tvIndrtPtr;
#endif /* TM_USE_INDRT */
/* Low Water Mark for notifying device send completes. Accessed in ISR. */
    tt32Bit                             tvNotifySendCompLowWater;
#ifdef TM_USE_SOAP
    tt32Bit                             tvSoapOptions;
#endif /* TM_USE_SOAP */
#ifdef TM_6_USE_MIP_MN
/* Mobile IPv6 mobile node state vector */
    tt6MnVect                           tv6MnVect;
/* DHAAD state machine data */
    tt6DhaadInfo                        tv6DhaadInfo;
#endif /* TM_6_USE_MIP_MN */

#ifdef TM_6_USE_MIP_CN
/* 
 * A single timer that scans through the binding cache when adding, deleting,
 * or updating any binding cache entry, perform lifetime expiration process 
 * for expiring/expired entries, and reschedule the timer near the expiration
 * time of the next entry. "near expiration" means the lifetime left is 
 * smaller than TM_6_MIP_NEAR_EXP_TIME msecs.
 */
    ttTimerPtr    tv6MipBindingTimerPtr;

/* The nonce and Kcn referesh timer, files every 30 seconds */
    ttTimerPtr    tv6MipNonceTimerPtr;

/* lock for the correspondent node binding cache */
    ttLockEntry   tv6BindingLock;

/* the active mip6 cn node keys */
    tt32Bit       tv6MipKcn[TM_6_MIP_MAX_NONCE_COUNT]
                           [tm_align(TM_6_MIP_KCN_LEN) / sizeof(tt32Bit)];

/* the active nonces */
    tt32Bit       tv6MipNonce[TM_6_MIP_MAX_NONCE_COUNT]
                             [tm_align(TM_6_MIP_NONCE_LEN) / sizeof(tt32Bit)];

/* the correspondent node's biding cache */
    ttList        tv6MipBindingList;

#endif /* TM_6_USE_MIP_CN */

#ifdef TM_USE_PKI
    ttVoidPtr           tvPkiPtr;
#ifdef TM_LOCK_NEEDED
    ttLockEntry         tvPkiLockEntry;
#endif /* TM_LOCK_NEEDED */
#endif /* TM_USE_PKI */
/* 
 * all crypto engine, software engine or hifn7951 
 * hardware crypto engine, are linked here.
 */
   ttVoidPtr            tvCryptoLibPtr;
#ifdef TM_LOCK_NEEDED
   ttLockEntry          tvCryptoLockEntry;
#endif /* TM_LOCK_NEEDED */

   ttVoidPtr            tvCompLibPtr;
#ifdef TM_LOCK_NEEDED
   ttLockEntry          tvCompLockEntry;
#endif /* TM_LOCK_NEEDED */

#ifdef TM_USE_IPSEC
/* IPsec global variable, of type ttIpsecEntryPtr */
    ttVoidPtr           tvIpsecPtr;
/* IPsec global structure, of type ttIpsecFuncEntry */
    ttIpsecFuncEntry    tvIpsecFunc;
#ifdef TM_LOCK_NEEDED
    ttLockEntry         tvIpsecLockEntry;
#endif /* TM_LOCK_NEEDED */
#ifdef TM_USE_IPSEC_TASK
    ttList              tvIpsecTaskList;
#ifdef TM_PEND_POST_NEEDED
    ttPendEntry         tvIpsecTaskPendSem;
#endif  /* TM_PEND_POST_NEEDED */
#ifdef TM_LOCK_NEEDED
    ttLockEntry         tvIpsecTaskLockEntry;
#endif /* TM_LOCK_NEEDED */
    int                 tvIpsecTaskQSize;
    tt8Bit              tvIpsecTaskRunning;
    tt8Bit              tvIpsecTaskExecuting;
#endif /* TM_USE_IPSEC_TASK */
#ifdef TM_USE_IPSEC_HANDOFF_TASK
    ttList              tvIpsecHandoffTaskList;
#ifdef TM_PEND_POST_NEEDED
    ttPendEntry         tvIpsecHandoffTaskPendSem;
#endif  /* TM_PEND_POST_NEEDED */
#ifdef TM_LOCK_NEEDED
    ttLockEntry         tvIpsecHandoffTaskLockEntry;
#endif /* TM_LOCK_NEEDED */
    int                 tvIpsecHandoffTaskQSize;
    tt8Bit              tvIpsecHandoffTaskRunning;
    tt8Bit              tvIpsecHandoffTaskExecuting;
#endif /* TM_USE_IPSEC_HANDOFF_TASK */
#endif /* TM_USE_IPSEC */

#ifdef TM_USE_ASYNC_CRYPTO
    ttEventEntry        tvAsyncCryptoEvent;
    ttLockEntry         tvAsyncCryptoLockEntry;
#endif /* TM_USE_ASYNC_CRYPTO */

#ifdef TM_USE_IP_REASSEMBLY_TASK
    ttList              tvIpReassemblyTaskList;
#ifdef TM_PEND_POST_NEEDED
    ttPendEntry         tvIpReassemblyTaskPendSem;
#endif  /* TM_PEND_POST_NEEDED */
#ifdef TM_LOCK_NEEDED
    ttLockEntry         tvIpReassemblyTaskLockEntry;
#endif /* TM_LOCK_NEEDED */
    int                 tvIpReassemblyTaskQSize;
    tt8Bit              tvIpReassemblyTaskRunning;
#endif /* TM_USE_IP_REASSEMBLY_TASK */

    ttLockEntry         tvPriLock;
    ttVoidPtr           tvPriListPtr;

#ifdef TM_USE_IKE
/* IKE global variable, of type ttIkeGlobalPtr */
    ttVoidPtr           tvIkePtr;
#ifdef TM_LOCK_NEEDED
    ttLockEntry         tvIkeLockEntry;
#endif /* TM_LOCK_NEEDED */
#ifdef TM_USE_IKE_TASK
#if ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED)) )
    ttLockEntry         tvIkeExecuteLockEntry;
    ttLockEntry         tvIkeExecuteLockEnd;
#endif  /* ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED))  */
#ifdef TM_PEND_POST_NEEDED
    ttPendEntry         tvIkePendSem;
#endif  /* TM_PEND_POST_NEEDED */
/*
 * Controls freeing the IKE global structure with the EXEC lock on.
 * Could be moved to the ikeGlobal structure if we use a single
 * IKE lock
 */
    tt8Bit              tvIkeShutdown;
#endif /* TM_USE_IKE_TASK */
#endif /* TM_USE_IKE */

#if (defined(TM_USE_SSL_SERVER) || defined(TM_USE_SSL_CLIENT))
    ttVoidPtr           tvSslTlsPtr;
#ifdef TM_LOCK_NEEDED
    ttLockEntry         tvSslTlsLockEntry;
#endif /* TM_LOCK_NEEDED */
#endif /* TM_USE_SSL_SERVER || TM_USE_SSL_CLIENT */

/* when multiple PPP authentication method is used, such as CHAP, 
 * PAP and MSCHAP, we need assign priority value to each method, 
 * so that the authenticator can control the proposal order according
 * to the priority.
 * If MS-CHAP is not used, we always prefer CHAP to PAP
 */
    tt32Bit             tvPppAuthPriority;

#ifdef TM_SNMP_MIB
    ttVoidPtr           tvSnmpdPtr;
#endif /* TM_SNMP_MIB */

#ifdef TM_SNMP_CACHE
    ttSnmpdCacheHead    tvSnmpdCaches[TM_SNMPD_CACHES];
#endif /* TM_SNMP_CACHE */

#ifdef TM_USE_IPV6
    ttDeviceEntryPtr    tv6McastDevPtr;

#ifdef TM_6_USE_PREFIX_DISCOVERY
/* Stateless Address Auto-Configuration prefix aging timer */
    ttTimerPtr          tv6RtPrefixAgingTimerPtr;
#endif /* TM_6_USE_PREFIX_DISCOVERY */

#ifdef TM_6_USE_NUD
/* Neighbor Unreachability Detection reachable timer */
    ttTimerPtr          tv6ReachableTimerPtr;
#endif /* TM_6_USE_NUD */

#ifdef TM_6_IP_REASSEMBLY
/* Timer used for IPv6 reassembly timeouts. */
    ttTimerPtr          tv6IpFHTimerPtr;

/* Queue of fragment heads */
    tt6IpFHEntryPtr     tv6IpFHPtr;
#endif /* TM_6_IP_REASSEMBLY */

/* IPv6-over-IPv4 virtual link layer handle. */
    ttLinkLayerEntryPtr tv6Ipv4TunnelLinkLayerHandle;

/* IPv6-over-IPv6 virtual link layer handle. */    
    ttLinkLayerEntryPtr tv6Ipv6TunnelLinkLayerHandle; 

/* Default configured tunnel device entry pointer. */
    ttDeviceEntryPtr    tv6Ipv4DefGwTunnelPtr;

/* IPv6-over-IPv4 automatic tunnel device entry pointer. */
    ttDeviceEntryPtr    tv6Ipv4AutoTunnelPtr;

/* Handles ICMPv6 errors in an IPv6 tunnel */
    tt6TunnelIcmpErrorFuncPtr tv6Ipv6IcmpTunnelFuncPtr;

/* Handles ICMPv4 errors in an IPv6 tunnel */
    tt6TunnelIcmpErrorFuncPtr tv6Ipv4IcmpTunnelFuncPtr;
    
#ifdef TM_6_IP_REASSEMBLY
#ifdef TM_LOCK_NEEDED
/* Lock for the fragment head list. */
    ttLockEntry         tv6IpFHLock;
#endif /* TM_LOCK_NEEDED */
#endif /* TM_6_IP_REASSEMBLY */

/* IPv6 cached routing entry. */
    ttRtCacheEntry      tv6RtIpForwCache;

/* IPv6 cached routing entry lock - needed even if TM_LOCK_NEEDED is not
   defined. */
    ttLockEntry         tv6RtIpForwCacheLock;

/* IPv6 send routing cache entry. */
    ttRtCacheEntry      tv6RtIpSendCache;

/* IPv6 send routing cache lock entry.
   Needed also when TM_LOCK_NEEDED not defined */
    ttLockEntry         tv6RtIpSendCacheLock;

#ifdef TM_6_USE_RAW_SOCKET
/* IPv6 raw socket cache lock */
    ttLockEntry         tv6RawCacheLock;

/* Cached IPv6 raw socket. */
    ttSocketEntryPtr    tv6RawSocketPtr;
#endif /* TM_6_USE_RAW_SOCKET */
#ifdef TM_6_IP_FRAGMENT
/* 
 * Used to populate Identification field in IPv6 Fragment extension header
 * ([RFC2460].R4.5:30)
 */
    tt32Bit             tv6IpFragId;
#endif /* TM_6_IP_FRAGMENT */
#ifdef TM_6_IP_REASSEMBLY
/* Fragmentation resolution (default, 60 seconds). */
    tt32Bit             tv6FragResolution;

/* Maximum size in bytes of an IPv6 datagram waiting to be reassembled */
    tt32Bit             tv6FragEntryMaxSize;
#endif /* TM_6_IP_REASSEMBLY */
    
/* Maximum number of times to send unicast Neighbor Solicitation 
   when trying to resolve an address. */
    tt32Bit             tv6NdUnicastRequestTries;

/* Maximum number of times to send multicast Neighbor Solicitation 
   when trying to resolve an address. */
    tt32Bit             tv6NdMcastRequestTries;
#ifdef TM_6_USE_ARP_QUIET_TIME
/* IPv6 ARP quiet time */
    tt32Bit             tv6NdQuietTime;
#endif /* TM_6_USE_ARP_QUIET_TIME */

/* Limit the rate at which ICMPv6 error messages are sent.
   ([RFC2463].R2.4:100 updated by [RFC4443].R2.4(f)) */
    tt32Bit             tv6IcmpTxErrLastTickCnt;
    tt32Bit             tv6IcmpTxErrTokenCnt;

/* Refer to TM_6_OPTION_ICMP_TX_ERR_LIMIT. ([RFC2463].R2.4:110 updated by
 * [RFC4443].R2.4(f)) */
    tt32Bit             tv6IcmpTxErrLimitMsec;

/* Refer to TM_6_OPTION_ICMP_TX_ERR_BURST. ([RFC4443].R2.4(f)) */
    tt32Bit             tv6IcmpTxErrLimitBurst;

#ifdef TM_6_USE_MIP_RO
/* Refer to TM_6_OPTION_MAX_BINDING_LIFETIME */
    tt32Bit             tv6CnMaxBindingLifeMsec;
#endif /* TM_6_USE_MIP_RO */

#ifdef TM_6_USE_MIP_MN
/* Refer to TM_6_OPTION_MN_REG_LT_BIAS. ([MIPV6_18++].R11.7.1:10) */
    tt32Bit             tv6MnRegLtBiasMsec;
/* Refer to TM_6_OPTION_MN_DEREG_TIMEOUT */
    tt32Bit             tv6MnDeregTimeoutMsec;
/* Refer to TM_6_OPTION_MN_1ST_REG_INIT_TIMEOUT */
    tt32Bit             tv6InitBaTimeoutFirstRegMsec;
#endif /* TM_6_USE_MIP_MN */

#ifdef TM_6_PMTU_DISC
/* The amount of time (in seconds) to wait before increasing an IPv6
   Path MTU estimate. */
    tt32Bit             tv6RtPathMtuTimeout;
#endif /* TM_6_PMTU_DISC */
#endif /* TM_USE_IPV6 */

#ifdef TM_USE_LOGGING_LEVELS
    ttLogLevelStoragePtr       tvLogLevelStoragePtr;
#endif /* TM_USE_LOGGING_LEVELS */
#if (defined(TM_DEBUG_LOGGING) || defined(TM_TRACE_LOGGING) || \
     defined(TM_ERROR_LOGGING) || defined(TM_MEMORY_LOGGING) || \
     defined(TM_USE_LOGGING_LEVELS))
/* support for debug, error and trace logging to circular buffer */
    ttLogCtrlBlkPtr     tvLogCtrlBlkPtr;
#endif

/* lookup table used to optimize storage of device pointer so that it only
   requires a 16-bit SNMP device index (1-based). */
    ttDeviceEntryPtr    tvDevIndexToDevPtr[TM_MAX_NUM_IFS + 1];

#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
/* sort IP addresses to improve performance of IP address lookup in the
   receive-path when many IP aliases are configured on a single interface.
   The SNMP Agent can use this to speed up non-exact match processing
   in tfIpAddrTableEntryGet. */

#ifdef TM_USE_IPV4
    ttSortedIpAddrCache tv4SortedIpAddrCache;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
    ttSortedIpAddrCache tv6SortedIpAddrCache;
#endif /* TM_USE_IPV6 */
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */

/* 
 * Globals Initialized Flag
 */
    tt32Bit               tvContextGlobalsSet;
/*
 * Timer variables.
 */
/* Control block for managing timer queues */
    ttTimerCtrl           tvTimerCtrl;
#ifdef TM_USE_TIMER_WHEEL
/* Maximum timer interval on the timer wheel */
    tt32Bit               tvTimerMaxWheelIntv;
#endif /* TM_USE_TIMER_WHEEL */
#ifdef TM_DEMO_TIME_LIMIT
    ttTimer               tvDemoTimer;
#endif /* TM_DEMO_TIME_LIMIT */

/*
 * ARP Resolve Queue
 */
    ttPacketPtr           tvArpResolveQueuePtr;
#ifdef TM_LOCK_NEEDED
    ttLockEntry           tvArpResolveQLock;
    ttLockEntry           tvArpResolveLockEntry;
#endif /* TM_LOCK_NEEDED */

/*
 * ARP resolution
 */

    tt32Bit               tvArpRequestTries;
    tt32Bit               tvArpRetryTime;
    tt32Bit               tvArpQuietTime;


/* 
 * ARP cache
 */

/* ARP entry timeout in milliseconds (600,000) */
    tt32Bit               tvArpTimeout;
/* ARP entries timer resolution in milliseconds (60,000) */
    tt32Bit               tvArpResolution;
/* Maximum number of ARP cache entries */
    int                   tvArpMaxEntries;
/* No socket errorCode */
    int                   tvErrorCode;
/*
 * Proxy ARP table
 */
/* Pointer to first element of proxy Arp table */
    ttProxyEntryPtr       tvProxyHeadPtr;
/* Proxy Lock */
    ttLockEntry           tvProxyLockEntry;


#ifdef TM_USE_BSD_DOMAIN
/*
 * Socket Array of Pointers to the Socket Entries
 * Allocated out of heap from size passed into initialize */
/* array of structures containing address family, and socket pointer */
    ttAFSocketPtr         tvSocketArray; 
/* domain link list header pointer */
    ttVoidPtr             tvBsdDomainHdrPtr;
/* domain lock entry */
    ttLockEntry           tvBsdDomainLockEntry;
#else /* !TM_USE_BSD_DOMAIN */
/* array of socket pointers */
    ttSocketEntryPtrPtr   tvSocketArray;
#endif /* !TM_USE_BSD_DOMAIN */
    tt16BitPtr            tvSocketErrorArray;
/* Maximum number of sockets */
    tt32Bit               tvMaxNumberSockets;
/* Number of open sockets */
    tt32Bit               tvNumberOpenSockets;
#ifdef TM_USE_TCP
/* Maximum number of allocated TCP sockets */
    tt32Bit               tvMaxTcpVectAllocCount;
/* Number of allocated TCP sockets */
    tt32Bit               tvTcpVectAllocCount;
/* Maximum number of allocated Time Wait TCP Vectors */
    tt32Bit               tvMaxTcpTmWtAllocCount;
/* Number of Time Wait TCP vectors in hash table lists */
    tt32Bit               tvTcpTmWtTableMembers;
#endif /* TM_USE_TCP */
/* Last slot used in the socket array */
    tt32Bit               tvSocketSearchIndex;
#ifdef TM_LOCK_NEEDED
/* Socket array lock */
    ttLockEntry           tvSocketArrayLock;
#endif /* TM_LOCK_NEEDED */
/*
 * Lookup socket tables (red-black tree roots, or lists)
 * [TM_SOTB_TCP_CON] TCP sockets
 * [TM_SOTB_NON_CON] other sockets (UDP, raw, etc..)
 */
#ifdef TM_USE_TCP
#define TM_SOTB_TCP_CON         0
#define TM_SOTB_NON_CON         1
#define TM_SOTB_NUM             2 /* number of tables */
#else /* !TM_USE_TCP */
#define TM_SOTB_NON_CON         0
#define TM_SOTB_NUM             1 /* number of tables */
#endif /* !TM_USE_TCP */
/* 
 * Port conflict resolution:
 * Index in tvSocketPortSearchMembers, and tvSocketPortSearchLastPort arrays:
 * [TM_SOTB_TCP_CON] TCP connected sockets
 * [TM_SOTB_NON_CON] other sockets (UDP, raw, etc..)
 * [TM_SOTB_NUM]     TCP non-connected sockets
 */
/* number of port conflict resolution slots */
#ifdef TM_USE_TCP
/* 2 slots for TCP, 1 for others */
#define TM_SOTB_NUM_SEARCH_PORT TM_SOTB_NUM  + 1
#else /* !TM_USE_TCP */
/* 1 slot for others */
#define TM_SOTB_NUM_SEARCH_PORT TM_SOTB_NUM 
#endif /* !TM_USE_TCP */

#ifdef TM_USE_SOCKET_RB_TREE
#ifdef TM_USE_SOCKET_HASH_LIST
/* Two socket tree root hash lists */
    ttSocketEntryPtrPtr   tvSocketTableRootPtrPtr[TM_SOTB_NUM];
#else /* !TM_USE_SOCKET_HASH_LIST */
/* Two socket tree roots */
    ttSocketEntryPtr      tvSocketTableRootPtr[TM_SOTB_NUM];
#endif /* !TM_USE_SOCKET_HASH_LIST */
#else /* !TM_USE_SOCKET_RB_TREE */
#ifdef TM_USE_SOCKET_HASH_LIST
/* Two socket table hash lists */
    ttListPtr             tvSocketTableListPtr[TM_SOTB_NUM];
/* Number of non empty buckets for each hash list */
    unsigned int          tvSocketTableListHashBucketFilled[TM_SOTB_NUM];
#else /* !TM_USE_SOCKET_HASH_LIST */
/* Two socket tables */
    ttList                tvSocketTableList[TM_SOTB_NUM];
#endif /* TM_USE_SOCKET_HASH_LIST */
#endif /* !TM_USE_SOCKET_RB_TREE */
/*
 * Number of [TCP connected sockets, non TCP sockets, non connected TCP sockets]
 * in the 2 socket tables (not counting reuseaddress list members)
 * Value used for port clonflict determination.
 */
    tt32Bit               tvSocketPortSearchMembers[TM_SOTB_NUM_SEARCH_PORT];
#ifdef TM_USE_SOCKET_HASH_LIST
/* Size of each hash list */
    unsigned int          tvSocketTableListHashSize[TM_SOTB_NUM];
/* Highest depth of entries in a bucket for each hash list */
    unsigned int          tvSocketTableListHighDepth[TM_SOTB_NUM];
#endif /* TM_USE_SOCKET_HASH_LIST */
/*
 * Number of times through the loop to find a random port. Two for the TCP
 * socket lookup table (TCP connected, TCP non connected), one for the non
 * TCP lookup table.
 */
    unsigned int          tvSocketPortSearchHighLoopCount[TM_SOTB_NUM_SEARCH_PORT];
/*
 * Last random port used, 2 for the TCP socket lookup table (one for connected
 * TCP, and one for non connected TCP), and one for the non TCP one
 */
    tt16Bit               tvSocketPortSearchLastPort[TM_SOTB_NUM_SEARCH_PORT];
/*
 * Threshold of number of sockets in a socket lookup table before we switch
 * from a random local port selection to a sequential local port selection
 */
    unsigned int          tvSocketThresholdSequentialPort[TM_SOTB_NUM];
#ifdef TM_LOCK_NEEDED
/* One lock for all lookup socket tables */
    ttLockEntry           tvSocketRootLock;
/* To lock the number of open sockets */
    ttLockEntry           tvSocketOpenLock;
#endif /* TM_LOCK_NEEDED */
#ifdef TM_USE_TCP
/* Time wait TCP vector hash table */
    ttListPtr             tvTcpTmWtTableListPtr;
#ifdef TM_LOCK_NEEDED
/* Time wait TCP vector hash table lock */
    ttLockEntry           tvTcpTmWtTableLock;
#endif /* TM_LOCK_NEEDED */
/* Time wait TCP vector hash table size */
    unsigned int          tvTcpTmWtTableHashSize;
/* Time Wait TCP vector hash table non empty buckets */
    unsigned int          tvTcpTmWtTableHashBucketFilled;
/* Highest depth of entries in a bucket for Time Wait hash table */
    unsigned int          tvTcpTmWtTableHighDepth;
#endif /* TM_USE_TCP */
/* 
 * Default Socket Queue Sizes
 */
    tt32Bit               tvDefRecvQueueBytes; /* receive queue */
    tt32Bit               tvDefSendQueueBytes; /* send queue */
    tt16Bit               tvDefRecvQueueDgrams; /* receive queue - datagrams */
    tt16Bit               tvDefSendQueueDgrams; /* send queue - datagrams */
/* send OOB data queue */
    tt32Bit               tvDefSendOobQueueBytes;

/* 
 * IP global variables
 */
 
#ifdef TM_USE_IPV4
#ifdef TM_IP_REASSEMBLY
/* Ip collection (reassembly) */
/* default fragmentation timer resolution is 1 second */
    tt32Bit               tvFragResolution; /* in milliseconds */
/* Maximum size in bytes of an IP datagram waiting to be reassembled */
    tt32Bit               tvFragEntryMaxSize;
#endif /* TM_IP_REASSEMBLY */
#endif /* TM_USE_IPV4 */

    tt4IpAddress          tvRouterId; /* Router Id */

#ifdef TM_USE_IPV4
    ttUserIpForwCBFuncPtr tvIpForwCBFuncPtr;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    tt6UserIpForwCBFuncPtr tv6IpForwCBFuncPtr;
#endif /* TM_USE_IPV6 */

#ifdef TM_USE_IPV4
/* Single routing cache entry for forwarding IP packets */
    ttRtCacheEntry        tvRtIpForwCache;
/*
 * single routing cache entry for sending IP packets (used by non socket
 * IP Send's)
 */
    ttRtCacheEntry        tvRtIpSendCache;
#ifdef TM_LOCK_NEEDED
/* Lock entry to access last sent packet routing cache */
    ttLockEntry           tvRtIpSendCacheLock;
/* Lock entry to access last forwarded packet routing cache */
    ttLockEntry           tvRtIpForwCacheLock;
#endif /* TM_LOCK_NEEDED */

#ifdef TM_IP_REASSEMBLY
/*
 * IP Reassembly global variables.
 */
/* queue of fragment heads */
    ttIpFHEntryPtr        tvIpFHPtr;
#ifdef TM_LOCK_NEEDED
/*
 * lock for the fragment heads list, and recycle queue of fragment heads
 * Use only one lock for the 2 lists, since the recycle list is always
 * accessed when the fragment head list is accessed.
 */
    ttLockEntry           tvIpFHLock;
#endif /* TM_LOCK_NEEDED */
/* Aging timer for fragments */
    ttTimerPtr            tvIpFHTimerPtr;
#endif /* TM_IP_REASSEMBLY */
 
/* 
 * Pointer to IP decapsulation routine.  Initialized to null by 
 * tfGlobalVarsInit and set by tfUseIpTunneling
 */
    ttIpHandlerFuncPtr    tvIpDecapFuncPtr;

/*
 * BOOTP/DHCP global variables
 */
/* BOOTP/DHCP client socket lock */
#ifdef TM_LOCK_NEEDED
    ttLockEntry           tvBootSocketLockEntry;
#endif /* TM_LOCK_NEEDED */
#ifndef TM_USE_STRONG_ESL
/* BOOTP/DHCP client socket */
    int                   tvBootSocketDescriptor;
/* BOOTP/DHCP client socket open count */
    int                   tvBootSocketOpenCount;
/* BOOTP/DHCP relay agent socket */
    int                   tvBootRelayAgentDescriptor;
#endif /* !TM_USE_STRONG_ESL */
/* BOOTP/DHCP Relay variables */
    ttDeviceEntryPtr      tvBootServDevEntryPtr;
    tt4IpAddress          tvBootServIpAddress;

/*
 * Maximum user DHCP entries per device (on top of the TM_MAX_IPS_PER_IF
 * automatic configuration entries.)
 * (Default 0)
 */
    int                   tvMaxUserDhcpEntries;
#endif /* TM_USE_IPV4 */

/*
 * Router global variables
 */
#ifdef TM_USE_IPV4
/* RIP socket */
    int                   tvRipSocketDescriptor;
#endif /* TM_USE_IPV4 */
/*
 * Maximum number of routing entries, not including ARP cache entries
 */
    int                   tvRtMaxEntries;
#ifdef TM_ERROR_CHECKING
/* Number of routing entries out of the tree, but not in the recycle queue */
    int                    tvRteLimboCount;
#endif /* TM_ERROR_CHECKING */
/* Number of dynamic routing entries */
    int                    tvRtEntriesCount;

/* Number of ARP entries and cloned multicast entries */
    int                    tvRtArpEntriesCount;

#ifdef TM_USE_IPV4
/* Number of IPv4 ARP entries and cloned multicast entries. */
    int                    tv4RtArpEntriesCount;
#endif /* TM_USE_IPV4 */
    
/* Aging Router timer resolution */
    tt32Bit               tvRtTimerResolution;
/* Default RIP/Redirect entry timeout value */
    tt32Bit               tvRtTimeout;

#ifdef TM_PMTU_DISC
/*
 * Default Path MTU host routing entry timeout when the Path MTU estimate
 * is decreased in milliseconds. (10 minutes).
 */
    tt32Bit               tvRtDecreasedMtuTimeout;
/*
 * Default Path MTU host routing entry timeout when a larger path MTU estimate
 * is attempted in milliseconds. (2 minutes).
 */
    tt32Bit               tvRtLargerMtuTimeout;
#endif /* TM_PMTU_DISC */

/*
 * Empty Routing Table:
 * Structure containing three radix root nodes in the Patricia tree.
 * The middle one is the head of the tree. The left one is the root
 * leaf with all bits 0, and the right one is the root leaf with all
 * bits 1 (invalid route entry). (The default gateway in the Patricia
 * tree will be stored as a duplicate of the root left leaf.)
 */
    ttRtRadixHeadEntry     tvRtRadixTreeHead;
/* Point to the top radix node (middle one in tvRtRadixTreeHead) */
    ttRtRadixNodePtr       tvRtTreeHeadPtr;

#ifdef TM_LOCK_NEEDED
/* To lock the tree for read or write access */
    ttLockEntry            tvRtTreeLock;
#endif /* TM_LOCK_NEEDED */

/* Aging route timer */
    ttTimerPtr             tvRtTimerPtr;
    ttTimerPtr             tvRtArpTimerPtr;
/*
 * IGMP global variables
 */
#if (defined(TM_IGMP) && defined(TM_USE_IPV4))
/* Default IGMP V1 Router present timeout in milliseconds (400 seconds) */
    int                   tvIgmpRouterV1Timeout;
/* Default IGMP Unsolicited Report interval (10 secons) */
    int                   tvIgmpUnsolReportIntv;
#ifdef TM_USE_IGMPV3
   tt16Bit                tvIgmpSoMaxSrcFilter;
   tt16Bit                tvIgmpIpMaxSrcFilter;
#endif /* TM_USE_IGMPV3 */
#endif /* TM_IGMP and TM_USE_IPV4 */

#if (defined(TM_IGMP) || defined(TM_USE_IPV6))
/*
 * Number of Ethernet multicast entries that can be allocated per allocation.
 */
    int                   tvEnetMcastBlockEntries;
/* Maximum number of successive tvEnetMcastBlockEntries allocations */
    int                   tvEnetMcastMaxAllocations;
#endif /* TM_IGMP || TM_USE_IPV6 */
    
#ifdef TM_USE_IPV4
/*
 * Arp collision detection global variables.
 */
    ttArpChkIn            tvArpChkInCBFunc;
#ifdef TM_LOCK_NEEDED
    ttLockEntry           tvArpChkListLock;
#endif /* TM_LOCK_NEEDED */
    ttArpChkEntryPtr      tvArpChkList;
    int                   tvArpChkActiveEntries;
#endif /* TM_USE_IPV4 */
#if (defined(TM_USE_AUTO_IP) && defined(TM_USE_IPV4))
/* Auto IP global variable */
    tt4IpAddress          tvAutoIpAddress; /* Current AUTO IP address */
#endif /* (defined(TM_USE_AUTO_IP) && defined(TM_USE_IPV4)) */
/*
 * Raw socket global variables
 */
    ttSocketEntryPtr      tvRawSocketPtr;
    ttLockEntry           tvRawCacheLock;

#ifndef TM_DISABLE_UDP_CACHE
/*
 * UDP global variables (UDP cache)
 */
    ttSocketEntryPtr      tvUdpSocketPtr;
    ttLockEntry           tvUdpCacheLock;
#endif /* !TM_DISABLE_UDP_CACHE */

#ifdef TM_USE_TCP
/*
 * TCP global context variables
 */
/* 
 * tvTcpIss
 * Initial sequence number (incremented by 1 every 4us) and used by
 * the TCP code to initialize the initial sequence number (corresponding
 * to the SYN) of a new connection.
 */
    tt32Bit               tvTcpIss;
/* 
 * Tcp time stamp to keep track of time changes for update of tvTcpIss in
 * tfTcpGetIss(). 
 */
    tt32Bit               tvTcpTimerTime;
#ifdef TM_USE_TCP_128BIT_RANDOM_ISS
/* TCP secret random number for outgoing connections */
    tt32Bit               tvTcpSecret[4];
#ifndef TM_OPTIMIZE_SIZE
/* TCP secret random number MD5 context */
    ttMd5Ctx              tvTcpSecretMdContext;
#endif /* !TM_OPTIMIZE_SIZE */
#else /* !TM_USE_TCP_128BIT_RANDOM_ISS */
    tt32Bit               tvTcpSecretIss;
#endif /* !TM_USE_TCP_128BIT_RANDOM_ISS */
/* TCP delay ACK timeout in milli-seconds. Disabled is set to zero. */
    tt32Bit               tvTcpDelayAckTime;
/* Tcp connection timeout in milli-seconds */
    tt32Bit               tvTcpConnTime;
/* Default Tcp retransmission time in milli-seconds */
    tt32Bit               tvTcpDefRtoTime;
/* min Tcp retransmission time in milli-seconds */
    tt32Bit               tvTcpMinRtoTime;
/* max Tcp retransmission time in milli-seconds */
    tt32Bit               tvTcpMaxRtoTime;
/* min Tcp zero window probe time in milli-seconds */
    tt32Bit               tvTcpMinProbeWndTime;
/* max Tcp zero window probe time in milli-seconds */
    tt32Bit               tvTcpMaxProbeWndTime;
/* Max retransmission count */
    int                   tvTcpMaxReXmitCnt;
/*
 * Amount of time to stay in FIN WAIT 2 when socket has been closed
 * (Default 10 minutes)
 */
    int                   tvTcpFinWt2Time; /* in seconds */
/* 2 * Maximum segment life time (2 * 30 seconds) (TIME WAIT time) */
    int                   tvTcp2MSLTime; /* in seconds */
/* Keep alive time (2 hours) in seconds */
    int                   tvTcpKeepAliveTime; /* in seconds */
/* Keep alive Probe Interval/Idle update time (75s) in seconds */
    int                   tvTcpIdleIntvTime; /* in seconds */
/* Keep alive Max Probe count (8) */
    int                   tvTcpKeepAliveProbeCnt;

/* One cache entry for TCP incoming packets */
    ttTcpVectPtr          tvTcpVectPtr;
#ifdef TM_LOCK_NEEDED
/* Lock for the one cache entry for TCP incoming packets */
    ttLockEntry           tvTcpCacheLock;
#endif /* TM_LOCK_NEEDED */
#endif /* TM_USE_TCP */
/*
 * Device Entries
 */
    ttDeviceEntryPtr      tvDeviceList;       /* The Active Device List */
    ttDeviceEntryPtr      tvDeviceToFreeList; /* The To Free Device List */
#ifdef TM_LOCK_NEEDED
/* Device List/Device to Free list Lock */
    ttLockEntry           tvDeviceListLock;
#endif /* TM_LOCK_NEEDED */
    ttDeviceEntryPtr      tvLoopbackDevPtr; /* loop back device */

    ttLinkLayerEntryPtr   tvLinkLayerListPtr;
#ifdef TM_LOCK_NEEDED
    ttLockEntry           tvLinkLayerListLock;
#endif /* TM_LOCK_NEEDED */
    ttLinkLayerEntryPtr   tvNullLinkLayerHandle; /* Null link layer */

/* Size of trailer to add to tail of outgoing packets.  Initialized to zero */
    ttPktLen              tvPakPktSendTrailerSize;

#ifdef TM_USE_TFTP
/*
 * TFTP client global variable. 
 */
    ttVoidPtr             tvTftpClientPtr;
#endif /* TM_USE_TFTP */
#ifdef TM_USE_TFTPD
/*
 * TFTP server global variable. 
 */
    ttVoidPtr             tvTftpSrvPtr;
#endif /* TM_USE_TFTPD */

#ifdef TM_USE_FTPD
/*
 * FTP server global variables. Initialized in tfContextInit().
 */

/* Ftp connections Idle timeout */
    tt32Bit               tvFtpdIdleTimeout;
/*
 * FTP server queue of connection entries.
 */
/* Head of the FTP server connection queue */
    ttServerQueueHead     tvFtpdConQHead;
/*
 * Lock for ftpd execution (protects #PendingConnections, RUN Q, connection
 * list, FTPD executing state (tvFtpdExecuting, tlFtpdShutdown,
 * tvFtpdListenDesc), and CB flags in each entry in RUN Q).
 */
#if ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED)) )
    ttLockEntry           tvFtpdExecuteLockEntry;
#endif  /* ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED))  */
/* Pend counting semaphore for the FTPD */
    ttPendEntry           tvFtpdPendSem;

/* FTPD listening descriptor */
    int                   tvFtpdListenDesc;

/* Indicates whether the FTPD is executing */
    int                   tvFtpdExecuting;
/*
 * FTPD variables initialized when user starts the FTPD (tfFtpdUserStart()).
 */
/* User file system flags */
    int                   tvFtpdFsFlags;
/* Ascii like binary for FTP file transfer */
    int                   tvFtpdFsTypeFlag;
/* Maximum number of open connections */
    int                   tvFtpdMaxConnections;
/* Keep track of number of pending connections */
    int                   tvFtpdPendingConnections;
/* FTPD blocking or non blocking */
    int                   tvFtpdBlockingState;
/* Indicates whether the FTPD is shutdown */
    int                   tvFtpdShutdown;
#ifdef TM_USE_FTPD_SSL
/* Whether or not SSL is required/allowed on the control channel */
    int                   tvFtpdSslCtrlPermissions;
/* Whether or not SSL is required/allowed on the data channel */
    int                   tvFtpdSslDataPermissions;
/* The FTP server's SSL session ID */
    int                   tvFtpdSslSessionId;
#endif /* TM_USE_FTPD_SSL */
#endif /* TM_USE_FTPD */

/* maximum number of timers we execute in a single call to tfTimerExecute,
   a value of 0 means there is no limit */
    int                   tvTimerMaxExecute;

#ifdef TM_USE_UPNP_DEVICE
/* 
 * Default SSDP advertisement duration, in seconds, used in CACHE-CONTROL
 * header. Default is 1800 seconds.
 */
    int                   tvSsdpDefaultCacheCtl;
#endif /* TM_USE_UPNP_DEVICE */
    
#ifdef TM_USE_HTTPD
/* Pointer to HTTPd server handles */
    ttVoidPtr             tvHttpdHandlesPtr;
#ifdef TM_USE_HTTPD_SSI
#ifdef TM_HTTPD_SSI_DEBUG
    FILE TM_FAR         * tvHttpdLogFilePtr;
#endif /* TM_HTTPD_SSI_DEBUG */
#endif /* TM_USE_HTTPD_SSI */
    tt16Bit               tvHttpdUpperMaxNumberServers;
    tt16Bit               tvHttpdMaxNumberServers;
    tt16Bit               tvHttpdNumberServers;
/*
 * Maximum buffer size to hold CGI variable names and values, or contiguous
 * received body buffer
 */
    tt16Bit               tvHttpdBodyMaxRecvBufSize;
#ifdef TM_USE_HTTPD_SSI
/*  The maximum SSI tag length in the SSI enabled pages. */
    tt16Bit               tvHttpdSsiMaxTagLen;
#endif /* TM_USE_HTTPD_SSI */

#ifdef TM_USE_HTTPD_CGI
/* minium number of CGI variables per allocation. */
    tt16Bit               tvHttpdCgiMinArgCnt;
/* maximum number of CGI variables. */
    tt16Bit               tvHttpdCgiMaxArgCnt;
#endif /* TM_USE_HTTPD_CGI */
#endif /* TM_USE_HTTPD */

#ifdef TM_USE_HTTPC
/* maximum line length */
    tt16Bit               tvHttpcMaxLineLength;
#endif /* TM_USE_HTTPC */

#if (defined(TM_SNMP_MIB) || defined(TM_PPP_LQM))
/*
 * SNMP AGENT MIB variable shared with PPP LQM.
 */
#ifdef TM_USE_NG_MIB2
#ifdef TM_USE_IPV4
    ttIfStatMib           tvDevIpv4Data;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    ttIfStatMib           tvDevIpv6Data;
#endif /* TM_USE_IPV6 */
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB || TM_PPP_LQM */
#ifdef TM_SNMP_MIB
/*
 * SNMP AGENT MIB variables
 */
/* Added to remove compilation error in Ipv6 only mode */
    ttIpMib               tvIpData;
#ifdef TM_USE_IPV4
    ttIcmpMib             tvIcmpData;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    tt6Icmpv6Mib          tvIcmpv6Data;
#endif /* TM_USE_IPV6 */
    ttUdpMib              tvUdpData;
#ifdef TM_USE_TCP
    ttTcpMib              tvTcpData;
#endif /* TM_USE_TCP */
#ifdef TM_USE_NG_MIB2
    ttIpForwardMib tvIpForwardData;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */


#ifdef TM_LOCK_NEEDED
/*
 * tfVSScanF() lock
 */
/* Proxy Arp lock */
    ttLockEntry           tvScanfLockEntry;
#endif /* TM_LOCK_NEEDED */

#ifdef TM_USE_TELNETD
/* Head of the TELNET server connection queue */
    ttServerQueueHead     tvTeldConQHead;
#if ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED)) )
    ttLockEntry           tvTeldExecuteLockEntry;
#endif  /* ( (defined(TM_LOCK_NEEDED) || defined(TM_PEND_POST_NEEDED))  */
    int                   tvTeldExecuting;
    int                   tvTeldListenDesc;
/*
 * Variables initialized when user starts the TELNETD
 */
/* Telnet options on */
    int                   tvTeldOptionsAllowed;
/* Maximum number of open connections */
    int                   tvTeldMaxConnections;
/* Keep track of number of pending connections */
    int                   tvTeldPendingConnections;
/* TELNETD blocking or non blocking */
    int                   tvTeldBlockingState;
/* Indicates whether the TELNETD is shutdown */
    int                   tvTeldShutdown;
/* Telnet connections Idle timeout */
    tt32Bit               tvTeldIdleTimeout;
#ifdef TM_PEND_POST_NEEDED
/* Pend counting semaphore for the TELNETD */
    ttPendEntry           tvTeldPendSem;
#endif /* TM_PEND_POST_NEEDED */
#endif /* TM_USE_TELNETD */

/*
 * DNS resolver variables
 */
/* List of DNS servers in order of priority (index 0 is primary, etc) */
    ttDnsServerEntry      tvDnsServerList[TM_DNS_MAX_SERVERS];

/* Points to the start of the DNS cache */
    ttDnsCacheEntryPtr    tvDnsCachePtr;

#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr      tvDnsDevPtr;
#endif /* TM_USE_STRONG_ESL */

    ttLockEntry           tvDnsLockEntry;

/* Host table support */
    ttList                      tvDnsHostTable;
    ttDnsHostTableSearchFuncPtr tvDnsHostTableSearchFuncPtr;
#ifdef TM_LOCK_NEEDED
    ttLockEntry                 tvDnsHostTableLockEntry;
#endif /* TM_LOCK_NEEDED */

/* TTL to use for DNS cache entries */
    tt32Bit               tvDnsCacheTtl;
/*
 * The number of DNS sockets allowed to be allocated.
 * This dictates the number of simultaneous requests that are allowed.
 */
    tt32Bit               tvDnsMaxNumberSockets;
/* The number of DNS sockets that have been allocated */
    tt32Bit               tvDnsNumSocksAllocated;
/* The array index of the current Transaction ID */
    tt32Bit               tvDnsCurrentTransId;
/* Number of entries current in the DNS cache */
    int                   tvDnsCacheEntries;
/* indicates blocking mode of resolver: TM_BLOCKING_ON or TM_BLOCKING_OFF */
    int                   tvDnsBlockingMode;
/* Maximum number of entries in the DNS cache.  Settable by user. */
    int                   tvDnsCacheSize;
/* Maximum number of query retransmissions to attempt */
    int                   tvDnsRetries;
/* Time (in seconds) to wait for a response to come back from the DNS server */
    int                   tvDnsTimeout;

/*
 * LLMNR sender/responder variables
 */
#if (defined(TM_USE_LLMNR_SENDER) || defined(TM_USE_LLMNR_RESPONDER))
#ifdef TM_USE_IPV4
/* LLMNR Multicast IPv4 address storage structure */
    struct sockaddr_storage llmnrMultiCastV4;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
/* LLMNR Multicast IPv6 address storage structure */
    struct sockaddr_storage llmnrMultiCastV6;
#endif /* TM_USE_IPV6 */
#endif /* (defined(TM_USE_LLMNR_SENDER) || defined(TM_USE_LLMNR_RESPONDER)) */

#ifdef TM_USE_LLMNR_SENDER
/* LLMNR sender lock variable. To modify sender attributes */
    ttLockEntry             tvLlmnrSenderLockEntry;
/*
 * LLMNR Sender global void pointer, to contain the global information
 * of the Sender.
 */
    ttVoidPtr               tvLlmnrSenderOptPtr;
#endif /* TM_USE_LLMNR_SENDER */

#ifdef TM_USE_LLMNR_RESPONDER
/* Responder socket for all responder communications */
    int                     tvLlmnrResponderSocket;
/* LLMNR responder lock variable. To modify responder attributes */
    ttLockEntry             tvLlmnrResponderLockEntry;
/*
 * LLMNR Responder global void pointer, to contain the global information
 * of the Responder.
 */
    ttVoidPtr               tvLlmnrResponderOptPtr;
#endif /* TM_USE_LLMNR_RESPONDER */

#ifdef TM_USE_IPV6
#ifdef TM_6_USE_MIP_MN
/* Refer to TM_6_OPTION_RR_MAX_BUL_ENTRIES */
    tt16Bit               tv6MnMaxBulEntries;
/* Refer to TM_6_OPTION_MN_BEACON_THRESHOLD */
    tt16Bit               tv6MnBeaconThresh;
#endif /* TM_6_USE_MIP_MN */

#ifdef TM_6_USE_MIP_CN
/* Refer to TM_6_OPTION_RR_MAX_BCE_ENTRIES */
    tt16Bit               tv6CnMaxBceEntries;
/* current nonce index */
    tt16Bit               tv6MipCurrNonceIndex;
#endif /* TM_6_USE_MIP_CN */
#endif /* TM_USE_IPV6 */

/*
 * Variables used for random seed generator.
 */
    tt16Bit               tvVector1;
    tt16Bit               tvVector2;
/*
 * Threshold in bytes below which we try and append to previous send buffer
 * for TCP
 */
    tt16Bit               tvDefSendAppendThreshold;
/*
 * Fraction use of the buffer below which we try and append to previous
 * recv buffer for TCP
 */
    tt16Bit               tvDefRecvAppendFraction;
/*
 * Fraction use of the buffer below which we try and copy to a smaller
 * recv buffer for UDP
 */
    tt16Bit               tvDefRecvCopyFraction;

/* Default Linger time when closing a TCP socket */
    tt16Bit               tvDefLingerTime;

/* Default TCP options: window scale, selective ACK, time stamp options, plus
 * default congestion avoidance
 */
    tt16Bit               tvTcpDefOptions;

/*
 * TCP global variables
 */
    tt16Bit               tvTcpDefMss; /* Default TCP MSS */

#ifdef TM_USE_IPV4
/* 
 * ICMP global variables
 */
    tt16Bit               tvIcmpSeqno; /* for request/reply */
/*
 * SLIP global variables
 */
    tt16Bit               tvSlipMtu;
/*
 * BOOTP/DHCP variables.
 */
    tt16Bit               tvBootServMhomeIndex;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_DHCP_FQDN
/*
 * FQDN structure
 */
    ttFqdn                              tvFqdnStruct;
#ifdef TM_LOCK_NEEDED
/* Lock for select pend entry recycle list */
    ttLockEntry                         tvFqdnStructLock;
#endif /* TM_LOCK_NEEDED */

#endif /* TM_USE_DHCP_FQDN */

/*
 * DNS resolver variables
 */
/* Last request ID to be sent. */
    tt16Bit               tvDnsTransId[TM_DNS_TRANS_ID_BACKLOG_SIZE];

/* Default socket options */
    tt16Bit               tvSocDefOptions;

#ifdef TM_USE_IPV6
#if (defined(TM_6_USE_PREFIX_DISCOVERY) || defined(TM_6_TAHI_MN_TEST))
    tt16Bit               tv6RtPrefixMhomeIndex;
/* IPv6 prefix aging timer resolution */
    tt16Bit               tv6PrefixTimerResSec;
#endif /* TM_6_USE_PREFIX_DISCOVERY or TM_6_TAHI_MN_TEST */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_TCP
/* Indicates whether tvTcpSecret has been initialized */
    tt8Bit                tvTcpSecretInit;
#endif /* TM_USE_TCP */
#ifdef TM_USE_IPV4
#ifdef TM_IP_FRAGMENT
/* Fragment bigger than MTU packets */
    tt8Bit                tvIpFragment;  /* default 1, allowed */
#endif /* TM_IP_FRAGMENT */
#ifdef TM_IP_REASSEMBLY
/* Ip collection (reassembly) */
/* Maximum allowed number of IP datagrams waiting to be reassembled. */
    tt8Bit                tvFragMaxQSize;
/* Actual number of IP datagrams waiting to be reassembled */
    tt8Bit                tvFragQSize;
/* Number of IP datagrams that were too big to reassemble, but we're still
 * tracking in case more fragments come in */
    tt8Bit                tvFragFailedQSize;
/* Maximum number of "too big" datagrams to track */
    tt8Bit                tvFragFailedMaxQSize;
#endif /* TM_IP_REASSEMBLY */
#if defined(TM_IP_REASSEMBLY) || defined(TM_SNMP_VERSION)
/* IP Reassembly timeout in seconds. Default is 60 */
    tt8Bit                tvFragTtl;
#endif /* TM_IP_REASSEMBLY || TM_SNMP_VERSION */ 
#endif /* TM_USE_IPV4 */

/* Flag for whether DNS is initialized or not */
    tt8Bit                tvDnsInitialized;

#ifdef TM_USE_IPV4
/*
 * ARP cache
 */
/* Whether to store gratuitious ARP cache mappings */
    tt8Bit                tvArpSmart;
/* 
 * IP global variables
 */
/* Host(do not forward) or Router (forward) */
    tt8Bit                tvIpForward; /* 0 means host, 1 router */
/* Forward directed broadcast IP packets (default is 1) */
    tt8Bit                tvIpDbcastForward;
#ifdef TM_IP_MCAST_FORWARD
/* 1 means multicast router */
    tt8Bit                tvIpMcastForward;
#endif /* TM_IP_MCAST_FORWARD */
#ifdef TM_USE_LBCAST_CONFIG
    tt8Bit                tvIpLbcastEnable; /* 1 (def) enabled, 0 disabled */
#endif /* TM_USE_LBCAST_CONFIG */
/* default IP time to live */
    tt8Bit                tvIpDefTtl;
/* Default IP TOS */
    tt8Bit                tvIpDefTos;

/*
 * RIP variables
 */
/* Rip enabled/disabled */
    tt8Bit                tvRipOn;
/*
 * tvRipSendMode has 4 possible values that determine the RIP packets being
 * sent: TM_RIP_NONE (no RIP packets are being sent),
 * TM_RIP_1 (only RIP 1 packets are being sent), TM_RIP_1_COMPAT
 * (default, sending only RIP 2 packets, broadcasting them ),
 * TM_RIP_2 (sending only RIP 2 packets, multicasting them)
 */
    tt8Bit                tvRipSendMode;
/*
 * tvRipRecvMode has 4 possible values that determine the RIP packets being
 * received: TM_RIP_NONE (no RIP packets are being received),
 * TM_RIP_1 (only RIP 1 packets are being received),
 * TM_RIP_2 (only RIP 2 packets are being received),
 * TM_RIP_1|TM_RIP_2 (both RIP 1 and Rip 2 packets are being received)
 */
    tt8Bit                tvRipRecvMode;
/* 
 * ICMP global variables
 */
    tt8Bit                tvIcmpAddrMaskAgent;
    tt8Bit                tvIcmpRouteReply;
#ifdef TM_USE_ECHO_CONFIG
    tt8Bit                tvIcmpEchoEnable;
#endif /* TM_USE_ECHO_CONFIG */

#ifndef TM_SINGLE_INTERFACE_HOME
    tt8Bit                tvIcmpErrDstAsSrc;
#endif /* TM_SINGLE_INTERFACE_HOME */
#endif /* TM_USE_IPV4 */
/*
 * UDP global variable
 */
/* Enable/disable Udp cheksum. Default is 1 (enabled). */
    tt8Bit                tvUdpChecksumOn;
/* Enable/disable Incoming check on Udp cheksum. Default is 1 (enabled). */
    tt8Bit                tvUdpIncomingChecksumOn;
/* Enable/disable Incoming check on TCP cheksum. Default is 1 (enabled). */
    tt8Bit                tvTcpIncomingChecksumOn;

/*
 * Device entries
 */
/*
 * Number of added interfaces. Adding interfaces increments this value.
 * Initialized to zero. Does not include Loop back interface.
 */
    tt16Bit               tvAddedInterfaces;
#ifdef TM_USE_IPV6
#ifdef TM_6_USE_MIP_CN
/* current nonce count */
    tt8Bit                tv6MipCurrNonceCount;
#endif /* TM_6_USE_MIP_CN */

#ifdef TM_6_IP_REASSEMBLY
/* Maximum allowed number of IP datagrams waiting to be reassembled. */
    tt8Bit                tv6FragMaxQSize;
    
/* Actual number of IP datagrams waiting to be reassembled */
    tt8Bit                tv6FragQSize;

/* Maximum number of "too big" datagrams to track */
    tt8Bit                tv6FragFailedMaxQSize;

/* Number of IP datagrams that were too big to reassemble, but we're still
 * tracking in case more fragments come in */
    tt8Bit                tv6FragFailedQSize;
#endif /* TM_6_IP_REASSEMBLY */

#if defined(TM_6_IP_REASSEMBLY) || defined(TM_SNMP_VERSION)
/* IP Reassembly timeout in seconds. Default is 60 */
    tt8Bit                tv6FragTtl;
#endif /* TM_6_IP_REASSEMBLY || TM_SNMP_VERSION */ 
    
/* Enables/disables IPv6 forwarding at the IPv6 level. Refer to 
   TM_6_OPTION_IP_FORWARDING. */
    tt8Bit                tv6IpForward;
/* Refer to TM_6_OPTION_IP_DEPRECATE_ADDR. ([RFC2462].R5.5.4:70) */
    tt8Bit                tv6IpDeprecateAddr;

#ifdef TM_6_IP_FRAGMENT
/* Refer to TM_6_OPTION_IP_FRAGMENT. ([RFC2460].R5:70) */
    tt8Bit                tv6IpFragment;
#endif /* TM_6_IP_FRAGMENT */

#ifdef TM_6_USE_MIP_MN
/* Refer to TM_6_OPTION_MN_EAGER_CELL_SWITCH. */
    tt8Bit                tv6MnEagerCellSwitch;
#endif /* TM_6_USE_MIP_MN */

#ifdef TM_6_USE_MIP_RO
/* Refer to TM_6_OPTION_MIP_RO_ENABLE. ([MIPV6_18++].R14.4.5:10) */
    tt8Bit                tv6MipRoEnable;
#endif /* TM_6_USE_MIP_RO */

#ifdef TM_6_USE_MIP_HA
/* flag to indicate whether this node acts a mipv6 home agent */
    tt8Bit                tv6MipIsHa;
#endif /* TM_6_USE_MIP_HA */
#endif /* TM_USE_IPV6 */

#ifdef TM_USE_UPNP_DEVICE
/* owner count used to protect access to UPnP state vector */
    tt8Bit                tvUpnpVectOwnerCount;
#endif /* TM_USE_UPNP_DEVICE */

/* Context Globals */
#ifdef TM_6_USE_DHCP
    tt6DhcpDuidPtr  tv6DhcpDuidPtr;  /* Pointer to the DHCPv6 client DUID*/
#ifdef TM_6_DHCP_OLD_SET_OPTION
/* Transmission parameter table */
    ttIntPtr        tv6DhcpTransParamPtr;
#endif /* TM_6_DHCP_OLD_SET_OPTION */
    tt8Bit          tv6DhcpDuidLen;  /* Length of the  DHCPv6 client DUID*/
    int             tv6DhcpSocket;   /* DHCPv6 UDP socket    */
/* Number of entities sharing the socket */
    int             tv6DhcpSocketOpenCount;
#ifdef TM_LOCK_NEEDED
    ttLockEntry     tv6DhcpGlobalLock; /* Global DHCPv6 lock */
#endif  /* TM_LOCK_NEEDED */
#endif /* TM_6_USE_DHCP */
#ifdef TM_USE_TCP_REXMIT_TEST
    int             tvFtpSockDesc;
    ttUser8Bit      tvTcpDropPackets;
    ttUser8Bit      tvTcpDataOnly;
    ttUser8Bit      tvSocketDisableRecv;
#endif /* TM_USE_TCP_REXMIT_TEST */
#if (defined(TM_USE_UPNP_DEVICE) || defined(TM_USE_SOAP))
/* UPNP OS version string */
    ttCharPtr       tvUpnpOsVersion;
#endif /* TM_USE_UPNP_DEVICE || TM_USE_SOAP */
#ifdef TM_ERROR_CHECKING
    tt32Bit          tvGetRawCount;
    tt32Bit          tvFreeRawCount;
#ifdef TM_USE_ANSI_LINE_FILE
#if (defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86) \
     || defined(TM_MEMORY_LOGGING))
#if (defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86))
    FILE *           tvMemoryLeakDetector;
#endif /* defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86) */
#ifdef TM_LOCK_NEEDED
    ttLockEntry      tvMemoryLeakLock;
#endif /* TM_LOCK_NEEDED */
#endif /* Win32 or TM_MEMORY_LOGGING */
#endif /*  TM_USE_ANSI_LINE_FILE */
#endif /* TM_ERROR_CHECKING */
#ifdef TM_USE_FTPD_SSL
#ifdef TM_AUTO_TEST
    ttUserFtpHandle  tvClientFtpHandle;
#endif /* TM_AUTO_TEST */
#endif /* TM_USE_FTPD_SSL */
#ifdef TM_USE_IPV4
/*
 * IP header Id incremented every time an IP datagram is sent.
 */
    tt16Bit          tvIpId;
#ifndef TM_USE_PREDICTABLE_IPID
#ifdef TM_USE_TCP
/*
 * IP header Id incremented every time a TCP segment is sent.
 */
    tt16Bit          tvTcpIpId;
#endif /* TM_USE_TCP */
#endif /* !TM_USE_PREDICTABLE_IPID */
#endif /* TM_USE_IPV4 */
} ttContext;

typedef ttContext TM_FAR * ttContextPtr;

/*
 * Global variables for all contexts.
 */
/* Current context */
#ifdef TM_MULTIPLE_CONTEXT
TM_EXTERN ttContextPtr          TM_GLOBAL_QLF tvCurrentContext;
#else /* !TM_MULTIPLE_CONTEXT */
TM_EXTERN ttContext             TM_GLOBAL_QLF tvCurrentContextStruct;
#endif /* !TM_MULTIPLE_CONTEXT */

#ifdef TM_USE_FTPD_SSL
#ifdef TM_AUTO_TEST
TM_EXTERN ttUserContext         TM_GLOBAL_QLF tvContextHandleServer;
TM_EXTERN ttUserContext         TM_GLOBAL_QLF tvContextHandleClient;
#endif /* TM_AUTO_TEST */
#endif /* TM_USE_FTPD_SSL */
/*
 * Structure containing global variables, other than per context, and other
 * than OS interface variables
 */
TM_EXTERN ttGlobal              TM_GLOBAL_QLF tvGlobals;

/*
 * (non context) Global variables set or not. Cannot be in ttGlobal, 
 * because it is checked prior to tvGlobals initialization.
 */
TM_EXTERN tt32Bit               TM_GLOBAL_QLF tvGlobalsSet;

/*
 * Timer global variables
 */
#ifndef tvTime
/* 
 * tvTime not declared in ttGlobal structure to save value across reboots 
 * and because can be changed from Timer ISR.
 * Note: User could define tvTime/tvTimeRollOver in their trsystem.h file
 * so that they can keep track of the time themselves, and avoid having
 * to call tfTimerUpdate() periodically.
 */
TM_EXTERN tt32Bit               TM_GLOBAL_QLF tvTime;
#endif /* tvTime */
#ifndef tvTimeRollOver
TM_EXTERN tt32Bit               TM_GLOBAL_QLF tvTimeRollOver;
#endif /* tvTimeRollOver */
/* tvTimerTickLength used during timer ISR */
TM_EXTERN unsigned              TM_GLOBAL_QLF tvTimerTickLength;

/*
 * OS interface global variables.
 */
#ifdef TM_USE_KERNEL_CRITICAL_STATUS
/* Not in ttGlobal because just used to store/restore critical status */
TM_EXTERN ttUserKernelCriticalStatus tvKernelCriticalStatus;
#endif /* TM_USE_KERNEL_CRITICAL_STATUS */

/*
 * Pend/Post global variable. Not in ttGlobal, because can be initialized
 * in tfKernelInitialize().
 */
TM_EXTERN tt8Bit                TM_GLOBAL_QLF tvPendPostEnabled;

#ifdef TM_LOCK_NEEDED
/*
 * lock global variable.
 */
TM_EXTERN tt8Bit                TM_GLOBAL_QLF tvLockEnabled;
#endif /* TM_LOCK_NEEDED */


/* Default file system working direectory has been set */
TM_EXTERN tt8Bit                TM_GLOBAL_QLF tvFSInitWorkingDirSet;
/* Initial File system drive */
TM_EXTERN int                   TM_GLOBAL_QLF tvFSInitDrive;


/*
 * Device driver ISR parameters global variable. Not in ttGlobal because
 * accessed from device driver ISR.
 */
TM_EXTERN ttDeviceIsrPara       TM_GLOBAL_QLF tvDeviceIsrPara[
                                                    TM_MAX_NUMBER_DEVICES];

/*
 * Win32 Thread Local Storage index, needed for each thread to store
 * and retriece it's context pointer. Not in ttGlobal, because win32 variable.
 */
#if ( defined(TM_KERNEL_WIN32_X86) && defined(TM_MULTIPLE_CONTEXT) )
TM_EXTERN tt32Bit               TM_GLOBAL_QLF tvTlsIndex;
#endif /* defined(TM_KERNEL_WIN32_X86) && defined(TM_MULTIPLE_CONTEXT) */

/*
 * Global constants
 */

#ifndef TM_GLOBAL
/*
 * Lookup table used to convert prefix length to netmask. Not declared
 * in ttGlobal, because pointer to constant table.
 * Initialized in trinit.c which defines TM_GLOBAL
 */
TM_EXTERN const tt4IpAddress  TM_CONST_QLF tvRt32ContiguousBitsPtr[]; 
#endif /* TM_GLOBAL */


#if (defined(TM_USE_HTTPD) || defined(TM_USE_HTTPC) || defined(TM_USE_HTTPU))
/* 
 * HTTP pointers to shared constant tables, initialized in trhttp.c.
 * Not declared in ttGlobal, because pointers to constant tables
 */
#ifndef TM_HTTP_GLOBAL
/*
 * Declared as extern (and not TM_EXTERN) because initialized in trhttp.c, and
 * not in trinit.c
 */
extern ttConstCharPtr const TM_CONST_QLF tvHttpVersionStr[];
extern ttConstCharPtr const TM_CONST_QLF tvHttpMethodStr[];
extern ttConstCharPtr const TM_CONST_QLF tvHttpHeaderNames[];
#endif /* !TM_HTTP_GLOBAL */
#endif /* !(TM_USE_HTTPD) || TM_USE_HTTPC || TM_USE_HTTPU) */
#if defined(TM_USE_PPP) || defined(TM_USE_PPPOE)
/*
 * PPP constant global variable. Kludge to return a non null pointer to the
 * user.
 * Will always have zero in it. Not declared in ttGlobal, because it is 
 * a constant. Pointer to this constant char is returned by the user
 * MSCHAP function to specify a null password.
 */
#ifdef TM_GLOBAL
const char TM_GLOBAL_QLF tvChapEmptyChar = '\0';
#endif /* TM_GLOBAL */
#endif /* TM_USE_PPP || TM_USE_PPPOE */

/*
 * Constant Ethernet broadcast address. Not declared in ttGlobal, because
 * constant.
 */
#ifdef TM_GLOBAL
/*
 * Ethernet global constant variables when TM_GLOBAL is defined.
 */
#ifdef TM_DSP
/* Constant initialization */
const ttEnetAddressUnion  TM_CONST_QLF tvEtherBroadcast =
    {{ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
#else /* !TM_DSP */
/* Constant initialization */
#ifdef TM_NO_NESTED_BRACES
const ttEnetAddressUnion  TM_CONST_QLF tvEtherBroadcast =
    {0xFFFF, 0xFFFF, 0xFFFF};
#else /* !TM_NO_NESTED_BRACES */
const ttEnetAddressUnion  TM_CONST_QLF tvEtherBroadcast =
    {{ 0xFFFF, 0xFFFF, 0xFFFF}};
#endif /* !TM_NO_NESTED_BRACES */
#endif /* !TM_DSP */
#else /* !TM_GLOBAL */
TM_EXTERN const ttEnetAddressUnion  TM_CONST_QLF tvEtherBroadcast;
#endif /* !TM_GLOBAL */

#ifdef TM_USE_IPV6

#ifdef TM_GLOBAL
/* 
 * These variables are not part of ttGlobal because they are just constant
 * initialized at compile time.
 */
/* Initialized only when included by trinit.c which defines TM_GLOBAL */
TM_EXTERN const struct in6_addr TM_CONST_QLF in6addr_any = 
        IN6ADDR_ANY_INIT;
TM_EXTERN const struct in6_addr TM_CONST_QLF in6addr_loopback = 
        IN6ADDR_LOOPBACK_INIT;
TM_EXTERN const struct in6_addr TM_CONST_QLF in6addr_nodelocal_allnodes =
        IN6ADDR_NODELOCAL_ALLNODES_INIT;
TM_EXTERN const struct in6_addr TM_CONST_QLF in6addr_linklocal_allnodes =
        IN6ADDR_LINKLOCAL_ALLNODES_INIT;
TM_EXTERN const struct in6_addr TM_CONST_QLF in6addr_linklocal_allrouters =
        IN6ADDR_LINKLOCAL_ALLROUTERS_INIT;
#ifdef TM_6_USE_MLDV2
TM_EXTERN const struct in6_addr TM_CONST_QLF in6addr_mldv2_allrouters =
        IN6ADDR_MLDV2_ALLROUTERS_INIT;
#endif /* TM_6_USE_MLDV2 */
TM_EXTERN const struct in6_addr TM_CONST_QLF tvIn6AddrHostMask = 
        TM_IN6ADDR_HOST_MASK;
TM_EXTERN const struct in6_addr TM_CONST_QLF tvIn6AddrIpv4Any = 
    TM_IN6ADDR_IPV4_ANY_INIT;
#else /* !TM_GLOBAL */
/* Declared here because, not declared in trsocket.h */
TM_EXTERN const struct in6_addr TM_CONST_QLF tvIn6AddrHostMask;
TM_EXTERN const struct in6_addr TM_CONST_QLF tvIn6AddrIpv4Any;
#endif /* !TM_GLOBAL */
#endif /* TM_USE_IPV6 */

#ifdef TM_USE_UPNP_DEVICE
#ifndef TM_GLOBAL
/* SOAP error codes and matching error description text */
TM_EXTERN const ttSoapErrorObject tvSoapErrorTable[];
/* const tables generated by upnpcgen */
/* tvUpnpDcpConstTable is the table of UPnP DCPs (device description XML
   documents) generated by upnpcgen in trcgdcp.c */
TM_EXTERN const ttUpnpDcpConstTable tvUpnpDcpConstTable[];
/* tvUpnpDeviceConstTable is the table of UPnP device instances generated
   by upnpcgen in trcgdev.c */
TM_EXTERN const ttUpnpDeviceConstTable tvUpnpDeviceConstTable[];
#endif /* ! TM_GLOBAL */
#endif /* TM_USE_UPNP_DEVICE */

#if (defined(TM_USE_UPNP_DEVICE) || defined(TM_USE_SOAP))
#ifndef TM_GLOBAL
/* tvUpnpActionConstTable is the table of UPnP service SOAP action instances
   generated by upnpcgen in trcgact.c */
TM_EXTERN const ttUpnpActionConstTable tvUpnpActionConstTable[];
/* tvUpnpInArgConstTable is the table of UPnP SOAP action input argument
   instances generated by upnpcgen in trcgiarg.c */
TM_EXTERN const ttUpnpInArgConstTable tvUpnpInArgConstTable[];
/* tvUpnpOutArgConstTable is the table of UPnP SOAP action output argument
   instances generated by upnpcgen in trcgoarg.c */
TM_EXTERN const ttUpnpOutArgConstTable tvUpnpOutArgConstTable[];
/* tvUpnpServiceConstTable is the table of UPnP service instances generated
   by upnpcgen in trcgserv.c */
TM_EXTERN const ttUpnpServiceConstTable tvUpnpServiceConstTable[];
/* tvUpnpStateVarConstTable is the table of UPnP service evented state variable
   instances generated by upnpcgen in trcgsvar.c */
TM_EXTERN const ttUpnpStateVarConstTable tvUpnpStateVarConstTable[];
#endif /* !TM_GLOBAL */
#endif /* TM_USE_UPNP_DEVICE || TM_USE_SOAP */

#ifndef TM_IKE_GLOBAL
/*
 * Declared as extern (and not TM_EXTERN) because initialized in trike.c, and
 * not in trinit.c
 */
#ifdef TM_USE_IKE_DPD
/*
 * DPD Vendor ID Hash. 
 * Not declared in ttGlobal, because pointer to constant table.
 */
extern const tt8Bit TM_GLOBAL_QLF tvDpdVendorIdPtr[];
#endif /* TM_USE_IKE_DPD */

#ifdef TM_USE_NATT
/*
 * NAT-Traversal Vendor ID Hash. 
 * Not declared in ttGlobal, because pointer to constant table.
 */
/* RFC3946 */
extern const tt8Bit TM_GLOBAL_QLF tvNattVendorIdPtr[];

/* draft02 */
extern const tt8Bit TM_GLOBAL_QLF tvNattDraft02VendorIdPtr[];
#endif /* TM_USE_NATT */
/*
 * Treck IKE Vendor ID Hash. 
 * Not declared in ttGlobal, because pointer to constant table.
 */
extern const tt8Bit TM_GLOBAL_QLF tvTreckVendorIdPtr[];
#endif /* !TM_IKE_GLOBAL */

#ifndef TM_IPSEC_GLOBAL
#ifdef TM_USE_IPSEC_64BIT_KB_LIFETIMES
/* IPsec 64-bit KByte lifetime max value */
extern const ttUser64Bit TM_GLOBAL_QLF tvIpsecMaxLifetime;
#endif /* TM_USE_IPSEC_64BIT_KB_LIFETIMES */
#endif /* !TM_IPSEC_GLOBAL */


#ifdef __cplusplus
}
#endif

#endif /* _TRGLOBAL_H_ */
