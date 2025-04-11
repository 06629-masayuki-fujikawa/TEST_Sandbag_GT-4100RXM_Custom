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
 * Description: IP router functions 
 * Filename: trroute.c
 * Author: Odile
 * Date Created: 11/10/97
 * $Source: source/trroute.c $
 *
 * Modification History
 * $Revision: 6.0.2.43 $
 * $Date: 2015/02/18 14:31:04JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h> 
#ifdef TM_RT_OUTPUT
#include <stdio.h>
#endif /* TM_RT_OUTPUT */
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>


/*
 * Local Macros constant definitions
 */

/* Supress unreferenced macro error for PC-LINT */
#ifndef TM_DSP
#ifdef TM_LINT
LINT_UNREF_MACRO(TM_RT_BITS_PER_BYTE)
#endif /* TM_LINT */
#endif /* TM_DSP */
#ifndef TM_RT_OUTPUT
#ifdef TM_LINT
LINT_UNREF_MACRO(tm_rt_rte_to_leaf)
#endif /* TM_LINT */
#endif /* !TM_RT_OUTPUT */

/*
 * TRY_BIT_OFFSET should only be set if
 * There are a lot of entries in the routing table
 */
/*#define TM_TRY_BIT_OFFSET */

/* Null node */
#define TM_RADIX_NULL_PTR (ttRtRadixNodePtr)0

#define TM_RT_BITS_PER_BYTE       8

/*
 * Given a leaf, corresponding routing entry
 */
#define tm_rt_leaf_to_rte(leafPtr) ((ttRteEntryPtr)((ttVoidPtr)leafPtr))

/* Patricia Tree flags */
/* Node (not leaf) in Patricia tree */
#define TM_RTN_NODE         (tt8Bit)0x01
/* Root leaf/node in Patricia tree */
#define TM_RTN_ROOT         (tt8Bit)0x02
/* Node or leaf is in the tree */
#define TM_RTN_IN           (tt8Bit)0x04
 /* Mask field is not all ones */
#define TM_RTN_NETWORK      (tt8Bit)0x08
/* Leaf is part of a network list */
#define TM_RTN_LIST_NETWORK (tt8Bit)0x10

/* rtnNSBitMask macros */
#ifdef TM_LITTLE_ENDIAN
#define TM_RTN_HIGH_BIT      0x80
#define TM_RTN_LOW_BIT       0x01
#else /* !TM_LITTLE_ENDIAN */
#define TM_RTN_HIGH_BIT      (tt4IpAddress)TM_UL(0x80000000)
#define TM_RTN_LOW_BIT       (tt4IpAddress)TM_UL(0x00000001)
#endif /* TM_LITTLE_ENDIAN */

/*
 * Path MTU plateau values as defined in RFC 1191
 */
/* TM_PMTU_MAX defined in trmacro.h */
/*#define TM_PMTU_MAX              (tt16Bit)65535 */
#ifdef TM_SA_PMTU
#define TM_PMTU_SECOND_PLATEAU   ((tt16Bit)32000 & (tt16Bit)(~0x7))
#define TM_PMTU_16MB_TKN_RING    ((tt16Bit)17914 & (tt16Bit)(~0x7))
#define TM_PMTU_802_4            ((tt16Bit)8166 & (tt16Bit)(~0x7))
#define TM_PMTU_FDDI             ((tt16Bit)4352 & (tt16Bit)(~0x7))
#define TM_PMTU_802_5            ((tt16Bit)2002 & (tt16Bit)(~0x7))
#define TM_PMTU_802_3            ((tt16Bit)1492 & (tt16Bit)(~0x7))
#define TM_PMTU_SLIP             ((tt16Bit)1006 & (tt16Bit)(~0x7))
#define TM_PMTU_ARCNET           ((tt16Bit)508 & (tt16Bit)(~0x7))
#define TM_PMTU_LOW_DELAY        ((tt16Bit)296 & (tt16Bit)(~0x7))
#define TM_PMTU_NULL             ((tt16Bit)0 & (tt16Bit)(~0x7))
#else /* !TM_SA_PMTU */
#define TM_PMTU_SECOND_PLATEAU   (tt16Bit)32000
#define TM_PMTU_16MB_TKN_RING    (tt16Bit)17914
#define TM_PMTU_802_4            (tt16Bit)8166
#define TM_PMTU_FDDI             (tt16Bit)4352
#define TM_PMTU_802_5            (tt16Bit)2002
#define TM_PMTU_802_3            (tt16Bit)1492
#define TM_PMTU_SLIP             (tt16Bit)1006
#define TM_PMTU_ARCNET           (tt16Bit)508
#define TM_PMTU_LOW_DELAY        (tt16Bit)296
#define TM_PMTU_NULL             (tt16Bit)0
#endif /* TM_SA_PMTU */

/* Minimum Path MTU that we will accept */
#ifndef TM_PMTU_MINIMUM
#define TM_PMTU_MINIMUM          TM_PMTU_LOW_DELAY
#endif /* TM_PMTU_MINIMUM */

/* Try next lower Path MTU estimate */
#define TM_PMTU_LOWER            (tt8Bit)0
/* Try next higher Path MTU estimate */
#define TM_PMTU_HIGHER           (tt8Bit)1

/*
 * Number of entries in the RFC 1192 MTU plateau table.
 * From TM_PMTU_MAX to TM_PMTU_LOW_DELAY, not including TM_PMTU_NUL
 */
#define TM_PMTU_ENTRIES          10

/* type definitions */
typedef struct ts6RtDevMtuUpdate
{
    ttDeviceEntryPtr dmuDevEntryPtr;
    tt16Bit          dmuNewMtu;
    tt16Bit          dmuAf;
} tt6RtDevMtuUpdate;

typedef tt6RtDevMtuUpdate TM_FAR * tt6RtDevMtuUpdatePtr;

#ifdef TM_USE_STRONG_ESL
#ifdef TM_USE_IPV6
typedef struct ts6RtDevGw
{
    ttDeviceEntryPtr dgwDevPtr;
    tt6IpAddressPtr  dgwIp6AddrPtr;
} tt6RtDevGw;
typedef tt6RtDevGw TM_FAR * tt6RtDevGwPtr;
#endif /* TM_USE_IPV6 */
#endif /* TM_USE_STRONG_ESL */

#ifdef TM_USE_IPV6
typedef int (TM_CODE_FAR * tt6RtDefRouterWalkPtr)(
                               ttNodePtr      nodePtr,
                               ttGenericUnion genParam);
#endif /* TM_USE_IPV6 */

/*
 * static functions declarations
 */

#ifdef TM_USE_IPV6
#ifdef TM_6_USE_PREFIX_DISCOVERY
static int tfRtPrefixAgingCB(
    ttRteEntryPtr   rtePtr,
    ttVoidPtr       voidPtr);

static ttRtRadixNodePtr tf6RtGetNonOnLink(tt6IpAddressPtr prefixAddrPtr,
                                          tt8Bit          prefixLen);
#endif /* TM_6_USE_PREFIX_DISCOVERY */
#endif /* TM_USE_IPV6 */

/*
 * Reset all routing caches pointing to either an indirect route (if
 * flag is TM_RTE_INDIRECT), or an lan ARP/Mcast/Bcast mapping
 * entries (if flag is TM_RTE_CLONED).
 */
#ifdef TM_LOCK_NEEDED
static void tfRtResetCaches(tt16Bit           flag, 
                            int               addressFamilyFlag,
                            tt8Bit            lockFlag);
#define tm_rt_reset_caches(rteFlag, afFlag, lockFlag) \
    tfRtResetCaches(rteFlag, afFlag, lockFlag) 
#else /* !TM_LOCK_NEEDED */
static void tfRtResetCaches(tt16Bit rteFlag, int addressFamilyFlag);
#define tm_rt_reset_caches(rteFlag, afFlag, lockFlag) \
    tfRtResetCaches(rteFlag, afFlag)
#endif /* TM_LOCK_NEEDED */

#ifdef TM_USE_ARP_FLUSH
#ifndef TM_SNMP_CACHE
static int tfRtRemArpEntriesCB(ttRteEntryPtr rtePtr,  ttVoidPtr parmPtr);
#else /* TM_SNMP_CACHE */
#ifdef TM_USE_IPV4
static int tf4RtArpNodeDeleteCB(ttNodePtr nodePtr, ttGenericUnion  genParam);
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
static int tf6RtArpNodeDeleteCB(ttNodePtr nodePtr, ttGenericUnion  genParam);
#endif /* TM_USE_IPV6 */
#endif /* TM_SNMP_CACHE */
#endif /* TM_USE_ARP_FLUSH */

#ifdef TM_USE_IPV4
/*
 * Delete an IPv4 ARP entry pointed to by rtePtr.
 */
static void tf4RtArpDelByRte(ttRteEntryPtr rtePtr);
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
/*
 * Delete an IPv6 ARP entry pointed to by rtePtr.
 */
static void tf6RtArpDelByRte(ttRteEntryPtr rtePtr);
#endif /* TM_USE_IPV6 */

#if (defined(TM_USE_IPV4) || defined(TM_6_PMTU_DISC))
/*
 * ARP entry timer
 */
TM_NEARCALL static void tfRtArpTimer (ttVoidPtr      timerBlockPtr,
                          ttGenericUnion userParm1,
                          ttGenericUnion userParm2);
#endif /* TM_USE_IPV4 || TM_6_PMTU_DISC */

/*
 * RIP and indirect routes timer
 */
TM_NEARCALL static void tfRtTimer (ttVoidPtr      timerBlockPtr,
                       ttGenericUnion userParm1,
                       ttGenericUnion userParm2);

/*
 * Routing Entry processing function
 * Remove any obsolete indirect entry (pointing to a non up local entry)
 * Also remove any multicast entry which have same device/multihome index
 * as voidPtr (local routing entry pointer)
 */
TM_NEARCALL static int tfRtRemObsLocalCB(ttRteEntryPtr rtePtr, ttVoidPtr voidPtr);

/* 
 * Remove a host entry, and remove all gateways (except default routers)
 * pointing to it
 */
static void tfRtRemoveHostEntry(ttRteEntryPtr rtePtr,
                                unsigned int  ownerCount,
                                tt16Bit       af);
/*
 * Remove any gateway pointing to the passed cloned entry (voidPtr).
 */
TM_NEARCALL static int tfRtRemoveGwToClonedCB (ttRteEntryPtr rtePtr, ttVoidPtr voidPtr);

#ifdef TM_USE_IPV6
static int tf6RtRemAutoEntriesCB(ttRteEntryPtr rtePtr, ttVoidPtr voidPtr);
static void tf6RtDefRouterDelAllByDevPtr(ttDeviceEntryPtr devPtr);
static void tf6RtDefRouterInvalid(
#ifdef TM_USE_STRONG_ESL
                      ttDeviceEntryPtr      devPtr,
#endif /* TM_USE_STRONG_ESL */
                      ttRteEntryPtr         rtePtr);
/*
 *  This is a tree walk call back function to remove a route entry if it 
 *  has a certain gateway IP address as the route.
 */
static int tfRtRemoveGwEntryCB (ttRteEntryPtr   rtePtr, 
                                ttVoidPtr       ipAddrPtr);
/* tfListWalk CB function to return the active default IPv6 gateway.  */
static int tf6RtDefRouterGetCB(ttNodePtr nodePtr, ttGenericUnion genParam);
/* tfListWalk CB function to return the first default IPv6 gateway.  */
static int tf6RtDefRouterGetAnyCB (ttNodePtr nodePtr, ttGenericUnion genParam);
/* 
 * tfListWalk CB function to return a default IPv6 gateway that
 * matches the IPv6 address.
 */
static int tf6RtDefRouterGetByIpCB(ttNodePtr nodePtr, ttGenericUnion genParam);
/* tfListWalk CB function to age default IPv6 gateways.  */
TM_NEARCALL static int tf6RtDefRouterTimerExpireCB (ttNodePtr nodePtr,
                                                    ttGenericUnion genParam);
/* tfListWalk CB function to delete expired default gateways */
TM_NEARCALL static int tf6RtDefRouterDelExpiredCB (ttNodePtr nodePtr,
                                                   ttGenericUnion genParam);
/*
 * tfListWalk() CB function to delete router entry from the list if we have a
 * match on the device
 */
TM_NEARCALL static int tf6RtDefRouterExpireMatchDevCB (ttNodePtr      nodePtr,
                                           ttGenericUnion genParam);
/* Search through all router lists */
static tt6RtDefRouterEntryPtr tf6RtDefRouterWalk(
#ifdef TM_USE_STRONG_ESL
                      ttDeviceEntryPtr      devPtr,
#endif /* TM_USE_STRONG_ESL */
                      tt6RtDefRouterWalkPtr walkFuncPtr,
                      ttGenericUnion        genParam);

/* Same as tf6RtDefRouterSubWalk but start at listIndex */
static ttNodePtr tf6RtDefRouterSubWalk(
                      tt6RtDefRouterHeadPtr     def6HeadPtr,
                      tt6RtDefRouterWalkPtr     walkFuncPtr,
                      ttGenericUnion            genParam,
                      int                       listIndex);
/* 
 * Called by tf6RtAddDefRouter/tf6RtDefRouterDelByDrt to insert an IPv6
 * default routing entry in the tree.
 */
static int tf6RtDefRouterAddByDrt(tt6RtDefRouterEntryPtr def6RtrPtr);

/* 
 * Called by tf6RtAddDefRouter/Aging timer when a routing entry TTL delete a 
 * routing entry whose time to live has expired. 
 * Also called by tf6RtDelDefRouter()
 */
static int tf6RtDefRouterDelByDrt(
#ifdef TM_USE_STRONG_ESL
                                  ttDeviceEntryPtr       devPtr,
#endif /* TM_USE_STRONG_ESL */
                                  tt6RtDefRouterEntryPtr def6RtrPtr);

/*
 * IPv6 Default routers list Timer
 */
static void tf6RtDefRouterTimer ( ttVoidPtr      timerBlockPtr,
                                  ttGenericUnion userParm1,
                                  ttGenericUnion userParm2 );

/* Add a new default router in the routing tree */
static tt6RtDefRouterEntryPtr  tf6RtDefRouterAddNewInTree(
#ifdef TM_USE_STRONG_ESL
                                  ttDeviceEntryPtr    devPtr
#else /* !TM_USE_STRONG_ESL */
                                  void
#endif /* !TM_USE_STRONG_ESL */
                                    );

/* tfListWalk CB function to add default router if it is reachable */
TM_NEARCALL static int tf6RtDefRouterAddReachableCB(ttNodePtr nodePtr,
                                        ttGenericUnion genParam);

/* tfListWalk CB function to add default router */
TM_NEARCALL static int tf6RtDefRouterAddAnyCB (ttNodePtr nodePtr, ttGenericUnion genParam);

/* Add a default router that failed to add previously */
static void tf6RtDefRouterTag(
#ifdef TM_USE_STRONG_ESL
                              ttDeviceEntryPtr devPtr
#else /* TM_USE_STRONG_ESL */
                              void
#endif /* TM_USE_STRONG_ESL */
                              );

/*
 * tfListWalk CB function to insert a default IPv6 gateway, that failed
 * to insert before.
 */
static int tf6RtTagDefRouterCB(ttNodePtr nodePtr, ttGenericUnion genParam);
#endif /* TM_USE_IPV6 */

static int tfRtUpdateDevMtuCB(ttRteEntryPtr rtePtr, ttVoidPtr newMtuPtr);

/*
 * Routing Entry processing function
 * Age any RIP/REDIRECT entry and remove it when expired
 */
static int tfRtAgeEntryCB(ttRteEntryPtr rtePtr, ttVoidPtr voidPtr);

#if (defined(TM_USE_IPV4) || defined(TM_6_PMTU_DISC))
/*
 * Routing Entry processing function
 * Age any ARP entry and remove it when expired
 */
static int tfRtArpAgeEntryCB(ttRteEntryPtr rtePtr, ttVoidPtr voidPtr);
#endif /* TM_USE_IPV4 || TM_6_PMTU_DISC */

/*
 * returns with no error if the routing entry is an ARP entry corresponding
 * to address (as specifiend in ARP mapping command) contained in
 * arpMapping structure.
 */
static int tfRtArpMappingCB(ttRteEntryPtr rtePtr, ttVoidPtr arpMappingPtr);

/*
 * If rtePtr is an ARP entry not being resolved and not being referenced
 * by another pointer, check whether its TTL is less than the current
 * least used ARP entry if any (stored in leastUsedRtePtrPtr). It if is
 * less, or there was none stored in leastUsedRtePtrPtr, store it in
 * leastUsedRtePtrPtr.
 */
TM_NEARCALL static int tfRtArpGetLeastUsedCB( ttRteEntryPtr rtePtr,
                                  ttVoidPtr leastUsedRtePtrPtr );

/* 
 * Add an ARP entry. Common code between IPv4 and IPv6.
 * IPv4: called from tf4RtArpAddEntry which is itself called when a user
 * adds an ARP entry, or in ARP incoming processing.
 * IPv6: Called from tf6RtArpAddEntry which is itself called when a user
 * adds an ARP entry.
 */
static int tfRtArpAddEntry ( 
    ttRtCacheEntryPtr  rtcCachePtr,
    tt8BitPtr          physAddrPtr,
    tt32Bit            ttl,
    tt8Bit             physAddrLen 
                           );
/*Delete a route. Called by tf6RtDelRoute() or tf4RtDelRoute() */
static int tfRtDelRoute(ttRteEntryPtr rtePtr, tt16Bit rteCreateFlag);

#ifdef TM_RT_OUTPUT
/*
 * Routing Entry processing function
 * Print a routing entry
 */
static int tfRtPrintEntryCB(ttRteEntryPtr rtePtr, ttVoidPtr voidPtr);

/* Print a leaf node */
static void tfRtPrintLeaf(ttCharPtr msg, ttRtRadixNodePtr leafPtr);

#ifdef TM_RT_INT_OUTPUT
/* Print an internal node */
static void tfRtPrintNode(ttCharPtr msg, ttRtRadixNodePtr nodePtr);
#endif /* TM_RT_INT_OUTPUT */

#endif /* TM_RT_OUTPUT */

/*
 * Find an entry in the routing table for the destination IP address
 * This function is called by tfRtFindNClone() after exclusive access to
 * the routing table has been obtained.
 */
static ttRteEntryPtr tfRtFind(
#ifdef TM_USE_STRONG_ESL
   ttDeviceEntryPtr     devPtr,
#endif /* TM_USE_STRONG_ESL */
#ifdef TM_USE_IPV6
    ttIpAddressPtr      dest6IpAddrPtr
#else /* ! TM_USE_IPV6 */
    tt4IpAddress        destIpAddr
#endif /* ! TM_USE_IPV6 */
    );

/* Get router entry for the Destination IP address
 * If REJECT flag is set, return null pointer.
 * If we got a LAN network entry (TM_RTN_CLONE set), clone a ARP router entry.
 */
static ttRteEntryPtr tfRtFindNClone(
#ifdef TM_USE_STRONG_ESL
   ttDeviceEntryPtr     devPtr,
#endif /* TM_USE_STRONG_ESL */
#ifdef TM_USE_IPV6
    ttIpAddressPtr      dest6IpAddrPtr
#else /* ! TM_USE_IPV6 */
    tt4IpAddress        destIpAddr
#endif /* ! TM_USE_IPV6 */
    );
#ifdef TM_USE_STOP_TRECK
static int tfRtRemEntriesCB(ttRteEntryPtr rtePtr,  ttVoidPtr parmPtr);
#endif /* TM_USE_STOP_TRECK */

/*
 * tfRtFindIpAddrIpNet() is called by tf4RtDelRoute(), tf6RtDelRoute(),
 * tf4RtGetDefault(), tf4RtModifyDefGatewayMhome(), tfRtArpCommand(),
 * tfRtRip(), and by tfRtAddEntry() to
 * try and find the unique route entry for a given destination IP address
 * destination network mask.
 * Function is called with routing table locked. So we don't need to
 * lock the table.
 */
ttRteEntryPtr tfRtFindIpAddrIpNet( 
#ifdef TM_USE_STRONG_ESL
   ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
#ifdef TM_USE_IPV6
    ttIpAddressPtr ip6AddrPtr,
    int            prefixLength
#else /* ! TM_USE_IPV6 */
    tt4IpAddress   ipAddress,
    tt4IpAddress   netMask
#endif /* ! TM_USE_IPV6 */
    );

/*
 * tfRtFindSockAddr() is called by tfNgRtDestExists(), tfRtInterface().
 * Try and find a route entry for a given sock addr destination IP address
 * destination network mask/prefix.
 * Function is called with routing table locked. So we don't need to
 * lock the table.
 */
static ttRteEntryPtr tfRtFindSockAddr(
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr                        devPtr,
#endif /* TM_USE_STRONG_ESL */
    const struct sockaddr_storage TM_FAR *  sockAddrPtr,
#ifdef TM_USE_IPV6
    int                                     prefixLen 
#else /* !TM_USE_IPV6 */
    tt4IpAddress                            netMask
#endif /* !TM_USE_IPV6 */
    );

/*
 * tfRtRtRemoveEntry is called by tf4RtDelRoute(), and tf6RtDelRoute().
 * Try and remove the routing table entry pointed to by rtEntryPtr from
 * the routing table.
 */
static void tfRtRemoveEntry(ttRteEntryPtr rtePtr);

/*
 *  Called by tfRtAddRoute() to insert a new entry in the routing table
 *  Also called by tfRtGet()/tfRtClone() when we need to clone an
 *  entry for ARP. Also called by tfRtInit(), tf4RtAddDefGw(), 
 *  tf4RtInsertEmptyDefaultGateway() for IPV4 default gateway.
 */
static int  tfRtAddEntry(
    ttRtRadixNodePtr        nodePtr, /* Node to start the search at */
    ttRteEntryPtr           rtePtr, /* Newly allocated routing entry */
    ttDeviceEntryPtr        devPtr, /* device for the route */
/* destination network address */
#ifdef TM_USE_IPV6
    int                     prefixLength,
#else /* ! TM_USE_IPV6 */
    tt4IpAddress            ipAddr,
    tt4IpAddress            netMask,
#endif /* ! TM_USE_IPV6 */
/* multihome index for local routes */
    tt16Bit                 mhomeIndex,
    int                     hops, /* metrics for non local routes */
    tt32Bit                 ttl, /* Time to live for this entry */
/* flag to indicate creator local/static/redirect/RIP */
    tt16Bit                 flags );

/*
 * Clone a routing entry. Return 0 on success.
 */
static ttRteEntryPtr tfRtClone( 
    ttRteEntryPtr           rtePtrPtr,
#ifdef TM_USE_IPV6
    ttIpAddressPtr          dest6IpAddrPtr
#else /* ! TM_USE_IPV6 */
    tt4IpAddress            destIpAddr
#endif /* ! TM_USE_IPV6 */
    );

/*
 * Cache routing entry (and corresponding ARP entry if routing entry
 * is indirect).
 */
static void tfRtCacheGet (ttRtCacheEntryPtr rtcPtr, ttRteEntryPtr rtePtr);

/*
 * copy oldNode to newNode, reset old node's in flag
 */
static void tfRtCopyNode( ttRtRadixNodePtr oldNodePtr,
                          ttRtRadixNodePtr newNodePtr );

static void tfRtInsertNet( ttRtRadixNodePtr     nodePtr,
                           ttRtRadixNodePtr     leafPtr,
                           int                  maskLen );

/* 
 * Called by the tf{4,6}RtAddxxx() functions. 
 * Allocate a routing entry.
 * Lock the tree if needed, Initializing the owner count to 1.
 * Map IPV4 address to IPv4-mapped IPv6 address if needed
 * Copy IP address in ttRteEntry structure.
 */
#ifdef TM_USE_IPV6
static ttRteEntryPtr tf6RtAllocNLockNCopy(tt8Bit               needLock, 
                                          tt4IpAddress         ip4Address,
                                          tt6ConstIpAddressPtr ip6AddrPtr);
#else /* TM_USE_IPV6 */
static ttRteEntryPtr tfRtAllocNLockNCopy(tt8Bit          needLock);
#endif /* TM_USE_IPV6 */

/*
 * Called by the tf{4,6}RtAddxxx() functions. 
 * Insert entry in SNMP routing cache, if insertion to Patricia tree succeeded.
 * UNLOCK the tree to release access to the routing table, if needed.
 * Free routing entry, if insertion failed. 
 * Ignore error for local route if errorCode is TM_EALREADY, since local
 * routes is for the same interface.
 */
static int tfRtCacheUnlockNClean( ttRteEntryPtr rtePtr,
                                  tt8Bit needUnLock, int errorCode,
                                  tt16Bit flags );

/* 
 * Common function to remove entries to be removed after a tree walk.
 */
static void tfRtTreeRem(void);

/* 
 * Common function to unlock the tree.
 */
static void tfRtTreeUnlock(void);

/* 
 * Common function to unlock the tree, and reset
 * the caches, when entries have been added/deleted 
 */
static void tfRtTreeUnlockNResetCaches(void);


#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
/*
 * Called by IPV4 functions in dual mode, to map the IPv4 address, and
 * netmask to IPv6, and call tfRtFindIpAddrIpNet().
 */
static ttRteEntryPtr tf4RtFindIpAddrIpNet(
#ifdef TM_USE_STRONG_ESL
                                           ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                                           tt4IpAddress     destIpAddress,
                                           tt4IpAddress     destNetmask
                                         );
/*
 * Called by IPV4 functions in dual mode, to map the IPv4 address, and
 * netmask to IPv6, and call tfRtFind().
 */
static ttRteEntryPtr tf4RtFind(
#ifdef TM_USE_STRONG_ESL
                               ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                               tt4IpAddress     destIpAddress
                              );

/* Insert an empty IPv4 default gateway */
static int tf4RtInsertEmptyDefaultGateway(
#ifdef TM_USE_STRONG_ESL
                                    ttDeviceEntryPtr devPtr
#else /* TM_USE_STRONG_ESL */
                                    void
#endif /* TM_USE_STRONG_ESL */
                                   );
#endif /* TM_USE_IPV4 */

#endif /* TM_USE_IPV6 */


#ifdef TM_PMTU_DISC
static tt16Bit tfRtPickNextPlateauMtu(tt16Bit currentMtu, tt8Bit flag);
#endif /* TM_PMTU_DISC */

#if (defined(TM_PMTU_DISC) || defined(TM_6_PMTU_DISC))
static int tfRtAddPathMtuEntry(
#ifdef TM_USE_STRONG_ESL
                               ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                                ttRteEntryPtr   rtePtr,
#ifdef TM_USE_IPV6
                                ttIpAddressPtr  dest6IpAddrPtr,
#else /* !TM_USE_IPV6 */
                                ttIpAddress     destIpAddress,
#endif /* TM_USE_IPV6 */
                                tt32Bit         timeout,
                                tt16Bit         flags,
                                tt16Bit         nextMtu );
#endif /* TM_PMTU_DISC || TM_6_PMTU_DISC */


#ifdef TM_PMTU_DISC
/*
 * RFC1191 MTU plateau table
 */
static const tt16Bit  TM_CONST_QLF tlMtuPlateauValues[TM_PMTU_ENTRIES + 1] =
{
    TM_PMTU_MAX,         TM_PMTU_SECOND_PLATEAU, TM_PMTU_16MB_TKN_RING,
    TM_PMTU_802_4,       TM_PMTU_FDDI,           TM_PMTU_802_5,
    TM_PMTU_802_3,       TM_PMTU_SLIP,           TM_PMTU_ARCNET,
    TM_PMTU_LOW_DELAY,   TM_PMTU_NULL
};
#endif /* TM_PMTU_DISC */

/*
 * NOTE about locking multiple resources and deadlocks: Care must be taken
 * when locking more than one resource, that if a task obtains the lock to at
 * least 2 different resources, it always happens in the same order to avoid
 * deadlock. For example, if TASK1 locks res1, TASK2 locks res2, TASK1
 * tries and locks res2 and Pend, TASK2 tries and locks res1 and Pend, we get
 * into a deadlock. Both TASK1 and TASK2 will pend forever waiting for the
 * other task to release the resource it locked.
 * This is easily avoided by not locking one of the 2 resources while
 * the other one is locked, or if we must obtain the lock to 2 or more
 * resources that the order in which it happens is always the same. In our
 * case we never lock the tree after having obtained the lock to the recycle
 * queue. Note that we do sometimes lock the recycle queue after we obtained
 * the lock for the tree, but this will not result in a deadlock since we
 * never obtain the lock to the tree after having obtained the lock to the
 * recycle queue.
 */


/*
 * Local macros function definitions
 */
#ifndef TM_LITTLE_ENDIAN
 /* bitmask of bit to be tested */
#define tm_rtn_bit_mask(bitnumber)        (TM_RTN_HIGH_BIT >> bitnumber)
#endif /* TM_LITTLE_ENDIAN */

#ifdef TM_USE_IPV6
#define tm_rtn_bit_set(nodePtr, keyPtr)                                     \
     (tm_ip_and(nodePtr->rtnNSBitMask,                                      \
                keyPtr->s6LAddr[nodePtr->rtnNS6LIndex]) != TM_IP_ZERO)
#else /* !TM_USE_IPV6 */
#define tm_rtn_bit_set(nodePtr, key)    \
                 (tm_ip_and(nodePtr->rtnNSBitMask, key) != TM_IP_ZERO)
#endif /* !TM_USE_IPV6 */

/*
 * Left child of an internal node
 */
#define tm_rt_left_child(nodePtr)  nodePtr->rtnNSLeftPtr

/*
 * Right child of an internal node
 */
#define tm_rt_right_child(nodePtr) nodePtr->rtnNSRightPtr

/*
 * Parent of a leaf/node
 */
#define tm_rt_parent(nodePtr)      nodePtr->rtnParentPtr

/*
 * Previous leaf in a chain of duplicated leaves
 */
#define tm_rt_prev_dup(leafPtr)    tm_rt_parent(leafPtr)

/*
 * Next leaf in a chain of duplicated leaves
 */
#define tm_rt_next_dup(leafPtr)    leafPtr->rtnLSDupKeyPtr

/*
 * Previous entry in a chain of network entries (for network checks)
 */
#define tm_rt_prev_net(leafPtr)   leafPtr->rtnLSPrevPtr

/*
 * Next entry in a chain of network entries (for network checks)
 */
#define tm_rt_next_net(nodePtr)   nodePtr->rtnNextPtr

/*
 * Number of matching bits so far down in the tree
 */
#define tm_rt_matching_bits(nodePtr) nodePtr->rtnBitOffset

/*
 * Mask length of a leaf entry
 */
#define tm_rt_mask_len(leafPtr) tm_rt_matching_bits(leafPtr)

/*
 * Given a routing entry, corresponding leaf node
 */
#define tm_rt_rte_to_leaf(rtePtr) (&(rtePtr->rteLeafNode))

/*
 * Given a routing entry, corresponding internal node
 */
#define tm_rt_rte_to_int_node(rtePtr) (&(rtePtr->rteInternalNode))

/*
 * Given a leaf, internal node from the corresponding routing entry
 */
#define tm_rt_leaf_to_int_node(leafPtr) \
    tm_rt_rte_to_int_node(tm_rt_leaf_to_rte(leafPtr))


/*
 * For a given key, find out the child (left or right) of a node (nodePtr)
 *
 * nodePtr, key value args 
 */
#define tm_rt_get_child(nodePtr, key) \
    ((tm_rtn_bit_set(nodePtr, key)) ? \
            tm_rt_right_child(nodePtr) : tm_rt_left_child(nodePtr))

/*
 * given a child and a parent, find the other child of the parent
 *
 * parentPtr value arg, childPtr value arg
 */
#define tm_rt_get_other_child(parentPtr, childPtr) \
    ( (tm_rt_left_child(parentPtr) == childPtr) ? \
        tm_rt_right_child(parentPtr) : \
        tm_rt_left_child(parentPtr) )


/*
 * Given a key, search down the tree starting at nodePtr until we reach a
 * leaf.
 *
 * nodePtr var arg, key value arg
 */
#define tm_rt_tree_search(nodePtr, key) \
{ \
    while (tm_16bit_one_bit_set(nodePtr->rtnFlags, TM_RTN_NODE)) \
    { \
        nodePtr = tm_rt_get_child(nodePtr, key); \
    } \
}

/*
 * fetch the left most leaf of the tree
 */
#define tm_rt_tree_leftmost_leaf(nodePtr) \
{ \
    while (tm_16bit_one_bit_set(nodePtr->rtnFlags, TM_RTN_NODE)) \
    { \
        nodePtr = tm_rt_left_child(nodePtr); \
    } \
}

#ifdef TM_ERROR_CHECKING
/*
 * Check whether we reached a non valid leaf (null)
 *
 * nodePtr value arg.
 */
#define tm_rt_check_null_leaf(nodePtr) \
{ \
    if (nodePtr == TM_RADIX_NULL_PTR) \
    { \
        tfKernelError("tm_rt_tree_search", \
                      "Null leaf in the tree"); \
        tfRtTreeUnlock(); \
        tm_thread_stop; \
    } \
}
#endif /* TM_ERROR_CHECKING */


/*
 * Given a child's key, set the left or right child of its parent (parentPtr)
 * to its child node (childPtr).
 *
 * parentPtr, childPtr, key value args.
 */
#define tm_rt_set_child_key(parentPtr, childPtr, key) \
{ \
    if (tm_rtn_bit_set(parentPtr, key))  \
    { \
        tm_rt_right_child(parentPtr) = childPtr; \
    } \
    else \
    { \
        tm_rt_left_child(parentPtr) = childPtr; \
    } \
}

/*
 * Given a child's key, set the left and right child of its parent (parentPtr)
 * to its children leaf and node (leafPtr, nodePtr). We know leafPtr is
 * a leaf, but nodePtr could be either a leaf or a node.
 *
 * parentPtr, leafPtr, leafKey, nodePtr value args
 */
#define tm_rt_set_children_key(parentPtr, leafPtr, leafKey, nodePtr) \
{ \
    if (tm_rtn_bit_set(parentPtr, leafKey))  \
    { \
        tm_rt_right_child(parentPtr) = leafPtr; \
        tm_rt_left_child(parentPtr) = nodePtr; \
    } \
    else \
    { \
        tm_rt_left_child(parentPtr) = leafPtr; \
        tm_rt_right_child(parentPtr) = nodePtr; \
    } \
}

#define tm_rt_set_parent(parentPtr, childPtr) \
{ \
    tm_rt_parent(childPtr) = parentPtr; \
}

/*
 * Given a child's key, set the left or right child of its parent (parentPtr)
 * to its child node (childPtr). Set the child node (childPtr)'s parent to
 * parentPtr (childPtr's current grand-parent).
 *
 * parentPtr, childPtr, key value args.
 */
#define tm_rt_set_child_parent_key(parentPtr, childPtr, key) \
{ \
    tm_rt_set_child_key(parentPtr, childPtr, key); \
    tm_rt_set_parent(parentPtr, childPtr); \
}

/*
 * Given a child's key, set the left and right child of its parent (parentPtr)
 * to its children leaf and node (leafPtr, nodePtr). We know leafPtr is
 * a leaf, but nodePtr could be either a leaf or a node. Set both children's
 * parent to the parent (parentPtr).
 *
 * parentPtr, leafPtr, leafKey, nodePtr value args
 */
#define tm_rt_set_children_parent(parentPtr, leafPtr, leafKey, nodePtr) \
{ \
    tm_rt_set_children_key(parentPtr, leafPtr, leafKey, nodePtr); \
    tm_rt_set_parent(parentPtr, leafPtr); \
    tm_rt_set_parent(parentPtr, nodePtr); \
}


/*
 * Given a parent, and one of its children, set the left or right child
 * to its new child node (childPtr).
 *
 * parentPtr, childPtr, key value args.
 */
#define tm_rt_set_new_child(parentPtr, oldChildPtr, newChildPtr) \
{ \
    if (tm_rt_left_child(parentPtr) == oldChildPtr) \
    { \
        tm_rt_left_child(parentPtr) = newChildPtr; \
    } \
    else \
    { \
        tm_rt_right_child(parentPtr) = newChildPtr; \
    } \
}

/*
 * Find insertion point in the tree. We walk up the tree from a reached
 * leaf until we find the parent and (left or right) child for the
 * node that we want to insert (the one that has bitOffset). Our new
 * parent should be the first node that we find on our way up that has a
 * smaller bit offset than the entry that we want to insert.
 * This piece of code is also used to find the node in the tree where
 * we want to insert the non host leaf (for network matches). The match
 * is the child of the node that has a rtnBitOffset (matching bits) < mask
 * length.
 * Note that this piece of code is called with a non zero bitOffset. So
 * a match is guaranteed since the root has a rtnBitoffset of zero.
 *
 * childPtr var arg, parentPtr non initialized var arg, bitOffset value arg.
 */
#define tm_rt_tree_find_insert(childPtr, parentPtr, bitOffset) \
{ \
    parentPtr = tm_rt_parent(childPtr); \
    while (tm_rt_matching_bits(parentPtr) >= bitOffset) \
    { \
        childPtr = parentPtr; \
        parentPtr = tm_rt_parent(childPtr); \
    } \
}

#ifdef TM_ERROR_CHECKING
#define tm_rt_check_dup_nodes(childPtr, bitOffset) \
{ \
    if (   tm_16bit_one_bit_set(childPtr->rtnFlags, TM_RTN_NODE) \
        && (tm_rt_matching_bits(childPtr) == bitOffset) ) \
    { \
        tfKernelError("tfRtAddEntry", " inserting a duplicate node"); \
        tfRtTreeUnlock(); \
        tm_thread_stop; \
    } \
}
#endif /* TM_ERROR_CHECKING */

/*
 * Given a mask length find the node in which to insert the network leaf.
 *
 * nodePtr var arg, tempPtr temp var, bitOffset value arg.
 */
#define tm_rt_tree_find_net_list(nodePtr, tempPtr, bitOffset) \
{ \
    if (bitOffset != TM_16BIT_ZERO) \
    { \
        tm_rt_tree_find_insert(nodePtr, tempPtr, bitOffset); \
    } \
    else \
    { \
        nodePtr = tm_context(tvRtTreeHeadPtr); \
    } \
}

/*
 * Check whether a leaf has the same mask as netMask whose length is maskBits.
 *
 * leafPtr, maskBits, value args.
 */
#define tm_rt_same_mask(leafPtr, maskBits) \
    (tm_rt_mask_len(leafPtr) == maskBits)

/*
 * Remove a leaf from a doubly linked list of network entries. It has
 * already been verified that leafPtr is part of a list of network entries.
 *
 * leafPtr value arg
 */
#define tm_rt_remove_net(leafPtr) \
{ \
    tm_rt_next_net(tm_rt_prev_net(leafPtr)) = tm_rt_next_net(leafPtr); \
    if (tm_rt_next_net(leafPtr) != TM_RADIX_NULL_PTR) \
    { \
        tm_rt_prev_net(tm_rt_next_net(leafPtr)) = tm_rt_prev_net(leafPtr); \
    } \
}

/*
 * Remove a leaf from a doubly linked list of duplicates.
 *
 * leafPtr value arg, prevDupPtr value arg, nxtDupPtr value arg
 */
#define tm_rt_remove_dup_member(leafPtr, prevDupPtr, nxtDupPtr) \
{ \
    tm_rt_next_dup(prevDupPtr) = tm_rt_next_dup(leafPtr); \
    if (nxtDupPtr != TM_RADIX_NULL_PTR) \
    { \
        tm_rt_prev_dup(nxtDupPtr) = prevDupPtr; \
    } \
}


/*
 * Global Functions
 */

/* Called by the Treck initialization routine
 * 1. We initialize the lock entry for the routing table (Patricia Tree)
 * 1. We initialize the pointer to the head of the Patricia tree:
 *    tvRtTreeHeadPtr points to the  address of tvRtRadixTreeHead.rhHeadNode.
 * 3. We then initialize an empty Patricia tree. It is a structure containing
 *    three nodes. The middle node is the head of the tree, and the left
 *    node is the leftmost leaf of the tree (all bits in the key set to
 *    zero), and the right node is the rightmost leaf of the tree (all bits
 *    in the key set to 1). We zero this Patricia tree head structure
 *    tvRtRadixTreeHead, and then we make the  parent pointer of the head
 *    node of the tree point to itself. We make the left leaf parent pointer,
 *    and the right leaf parent pointer point to the head node.
 *    The flags of the left and right leaves indicate that the nodes are
 *    part of the tree, they are at the root structure, and they are
 *    leaves. The flags of the head node indicate that it is root,
 *    part of the tree, and an internal node. The left pointer
 *    of the head node point to the left leaf, and the right pointer of
 *    the head node point to the right leaf. The bit mask of the head
 *    node (used to make the left/right decision) is set to 0x80000000UL for
 *    big endian and 0x00000080UL for little endian CPUS,
 *    which means that we start testing the first bit
 *    (in network byte order). The right leaf key is 0xFFFFFFFFUL. All
 *    other fields are zeroes.
 * 4. We then initialize the router entries recycle queue and its lock
 */

void tfRtInit ( void  )
{
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
#ifndef TM_USE_STRONG_ESL
    int                 errorCode;
#endif /* TM_USE_STRONG_ESL */
#endif /* dual IP layer */
    int                 i;
#endif /* TM_USE_IPV6 */

/* Initialize global pointer to head of Patricia tree */
    tm_context(tvRtTreeHeadPtr) = &tm_context(tvRtRadixTreeHead).rthHeadNode;
/* Initialize Patricia tree head structure, which contains the head node,
 * the leftmost and rightmost leaf nodes.
 */
/* parent of head is head itself */
    tm_context(tvRtTreeHeadPtr)->rtnParentPtr = tm_context(tvRtTreeHeadPtr);
/* parent of leftmost leaf is head of tree */
    tm_context(tvRtRadixTreeHead).rthLeftEndNode.rtnParentPtr =
                                                tm_context(tvRtTreeHeadPtr);
/* parent of righmost leaf is head of tree */
    tm_context(tvRtRadixTreeHead).rthRightEndNode.rtnParentPtr =
                                                tm_context(tvRtTreeHeadPtr);
/* leaf in the tree, in root structure */
    tm_context(tvRtRadixTreeHead).rthLeftEndNode.rtnFlags =
                                                TM_RTN_IN|TM_RTN_ROOT;
/* leaf in the tree, in root structure */
    tm_context(tvRtRadixTreeHead).rthRightEndNode.rtnFlags =
                                                TM_RTN_IN|TM_RTN_ROOT;
/* node in the tree, in root structure */
    tm_context(tvRtTreeHeadPtr)->rtnFlags = TM_RTN_IN|TM_RTN_ROOT|TM_RTN_NODE;
/* left child of root node */
    tm_rt_left_child(tm_context(tvRtTreeHeadPtr)) =
                               &tm_context(tvRtRadixTreeHead).rthLeftEndNode;
/* right child of root node */
    tm_rt_right_child(tm_context(tvRtTreeHeadPtr)) =
                               &tm_context(tvRtRadixTreeHead).rthRightEndNode;
/* bit mask for left/right decision in root node */
    tm_ip_copy(TM_IP_ONE_HIGH_BIT, tm_context(tvRtTreeHeadPtr)->rtnNSBitMask);
/* rtnBitOffset in root node initialized to zero */
/* IPV6 rtn6LongIndex, and rtn6LongBitOffset in root node initialized to zero */
/* Right most leaf has invalid destination IP address (or key)*/
#ifdef TM_USE_IPV6
    for (i = 0; i < 3; i++)
    {
        tm_ip_copy( 
           TM_IP_ALL_ONES,
           tm_context(tvRtRadixTreeHead).rthRightEndNode.rtnLSKey.s6LAddr[i]);
    }
#ifndef TM_USE_STRONG_ESL
    for (i = 0; i < TM_6_NUM_DEF_ROUTER_LISTS; i++)
    {
        tfListInit(&(tm_context(tvRtRadixTreeHead).rth6DefaultRouterHead
                                                  .drh6RouterList[i]));
    }
#endif /* !TM_USE_STRONG_ESL */
    tm_context(tvRtRadixTreeHead).rthRightEndNode.rtnPrefixLength = 
                                                TM_6_IP_HOST_PREFIX_LENGTH;
    tm_context(tvRtRadixTreeHead).rthRightEndNode.rtnLS6MaskPrefix = 32;
    tm_context(tvRtRadixTreeHead).rthRightEndNode.rtnLS6MaskIndex = 3;
/* 
 * To support longest prefix match on a routing table entry having duplicate
 *  keys of 0, the left end of the tree must have a netmask of all one's. 
 */
    tm_context(tvRtRadixTreeHead).rthLeftEndNode.rtnPrefixLength = 
                                                TM_6_IP_HOST_PREFIX_LENGTH;
    tm_context(tvRtRadixTreeHead).rthLeftEndNode.rtnLS6MaskPrefix = 32;
    tm_context(tvRtRadixTreeHead).rthLeftEndNode.rtnLS6MaskIndex = 3;
#else /* !TM_USE_IPV6 */
/* IPv4 mode only */
    tm_ip_copy( TM_IP_ALL_ONES,
                tm_context(tvRtRadixTreeHead).rthRightEndNode.rtnLSKey );
    tm_ip_copy( TM_IP_ALL_ONES,
                tm_context(tvRtRadixTreeHead).rthRightEndNode.rtnLSMask);
/* 
 * To support longest prefix match on a routing table entry having duplicate
 *  keys of 0, the left end of the tree must have a netmask of all one's. 
 */
    tm_ip_copy( TM_IP_ALL_ONES,
                tm_context(tvRtRadixTreeHead).rthLeftEndNode.rtnLSMask);
#endif /* !TM_USE_IPV6 */

#ifdef TM_USE_IPV6
#if (defined(TM_6_USE_PREFIX_DISCOVERY) || defined(TM_6_TAHI_MN_TEST))
    tm_context(tv6RtPrefixMhomeIndex) = TM_6_RT_START_PREFIX_MHOME_INDEX;
#endif /* TM_6_USE_PREFIX_DISCOVERY or TM_6_TAHI_MN_TEST */
#endif /* TM_USE_IPV6 */

#ifdef TM_USE_IPDUAL
#ifndef TM_USE_STRONG_ESL
/* in dual IP layer mode, we must always have an IPv4 default gateway entry
   in the routing tree, even if it isn't configured. */
    errorCode = tf4RtInitInsertEmptyDefaultGateway();

    if (errorCode != TM_ENOERROR)
    {
/* Should never happen */
        tfKernelError("tfRtInit", "Could not add an IPV4 default gateway\n");
    }
#endif /* TM_USE_STRONG_ESL */
#endif /* TM_USE_IPDUAL */
}


/*
 * set physical address mapping to a routing table ARP Entry
 * Called by the ARP interface, or user interface to set a physical address
 * mapping
 * Parameters: rtePtr pointer to ARP entry in routing table
 *             physAddrPtr pointer to link layer address
 */
int tfRtArpAddByRte ( ttRteEntryPtr     rtePtr,
                      tt8BitPtr         physAddrPtr,
                      tt32Bit           ttl,
                      tt8Bit            physAddrLen )
{
    int errorCode;
    
#ifdef TM_ERROR_CHECKING
    if (   (physAddrPtr == TM_8BIT_NULL_PTR)
        || (rtePtr == TM_RTE_NULL_PTR))
    {
        tfKernelError("tfRtArpAddByRte", "bad parameters");
        tm_thread_stop;
    }
#endif

/* Check that the entry is an ARP entry, and that it is up */
    if (    tm_16bit_all_bits_set( rtePtr->rteFlags,
                                   TM_RTE_ARP | TM_RTE_CLONED )
         && tm_16bit_all_bits_set( rtePtr->rteFlags2,
                                   TM_RTE2_HOST | TM_RTE2_UP ) )
    {
#ifdef TM_USE_ETHER_8023_LL
        rtePtr->rte8023Flags |= (TM_RTE8023_ETH_RESPONSE|TM_RTE8023_ETH_FIRST);
        if (physAddrLen & TM_RT_FRAME_8023)
        {
            physAddrLen &= ~TM_RT_FRAME_8023;
            rtePtr->rte8023Flags |= TM_RTE8023_ETH_8023;
        }
        else
        {
            tm_8bit_clr_bit( rtePtr->rte8023Flags, TM_RTE8023_ETH_8023 );
        }
#endif /* TM_USE_ETHER_8023_LL */
 
        if (rtePtr->rteFlags & TM_RTE_LINK_LAYER)
        {
/* reset previous mapping */
            tm_16bit_clr_bit( rtePtr->rteFlags, TM_RTE_LINK_LAYER );
        }
        tm_bcopy(physAddrPtr, rtePtr->rteHSPhysAddr, physAddrLen);
        rtePtr->rteFlags |= TM_RTE_LINK_LAYER;
        rtePtr->rteRefresh = TM_8BIT_ZERO;
        rtePtr->rteTtl = ttl;

#ifdef TM_USE_IPV6
        tm_kernel_set_critical;
        rtePtr->rteHSLastReachedTickCount = tvTime;
        tm_kernel_release_critical;

/* When the stack is running in dual IP layer mode, the IPv4 part of the
   stack must set ttRteHostGwayUnion.HSPhysAddrLen to 6 in any ARP Cache
   entries that it uses. */
        rtePtr->rteHSPhysAddrLen = physAddrLen;
#else /* ! TM_USE_IPV6 */
        TM_UNREF_IN_ARG(physAddrLen); /* avoid compiler warning */
#endif /* ! TM_USE_IPV6 */
        errorCode = TM_ENOERROR;
    }
    else
    {
        errorCode = TM_EINVAL;
    }
    return errorCode;
}



/*
 * Called by the socket API interface.
 * Walk the tree, to find an ARP entry corresponding to the valid
 * field in arpMapping structure, and execute the command in arpmCommand
 * field of ttArpMapping structure as follows:
 *
 *   TM_ARP_GET_BY_PHYS:
 *      Get an ARP Entry mapping in routing table using Physical address to
 *      locate.
 *   TM_ARP_GET_BY_IP:
 *      Get an ARP Entry mapping in routing table using IP address to locate 
 *   TM_ARP_DEL_BY_IP:
 *      Erase an ARP Entry mapping in routing table using IP address to locate
 *   TM_ARP_DEL_BY_PHYS:
 *      Erase an ARP Entry mapping in routing table using Physical address to
 *      locate.
 *   TM_ARP_SET_TTL
 *      Change ttl of ARP entry.
 *   TM_ARP_SET_PHYS
 *      Change physical address of ARP entry.
 * Parameters:
 *   arpMappingPtr
 *      Pointer to structure containing ARP table command to execute, 
 *      and its parameters.
 * Return values: 
 *    TM_ENOERROR
 *      success
 *    TM_ENOENT 
 *      no such Arp entry
 *    TM_EINVAL 
 *      unrecognized ARP command, or
 *      provided physical address buffer is too short
 */
int tfRtArpCommand (ttArpMappingPtr arpMappingPtr)
{
    ttRteEntryPtr rtePtr;
    tt8Bit          length;
    tt8Bit          match;
    int           errorCode;
    tt16Bit       af;
    tt8Bit        arpCommand;

    arpCommand = arpMappingPtr->arpmCommand;
    errorCode = TM_ENOERROR;
#ifndef TM_USE_IPV6
    if (arpCommand >  TM_ARP_LAST_COMMAND)
    {
        errorCode = TM_EINVAL;
    }
#endif /* !TM_USE_IPV6 */
#ifndef TM_USE_IPV4
    if (tm_8bit_bits_not_set(arpCommand, TM_6_ARP_CMD_FLAG))
    {
        errorCode = TM_EINVAL;
    }
#endif /* !TM_USE_IPV4 */
#ifndef TM_USE_IPDUAL
    if (errorCode == TM_ENOERROR)
#endif /* !TM_USE_IPDUAL */
    {
/* LOCK the tree to obtain exclusive access to the routing table */
        tm_call_lock_wait(&tm_context(tvRtTreeLock));
        if (tm_8bit_one_bit_set( arpCommand,
                             (TM_ARP_GET_BY_PHYS | TM_ARP_DEL_BY_PHYS)))
        {
/* No key. No choice but to walk the tree */
            errorCode = tfRtTreeWalk( tfRtArpMappingCB,
                                      (ttVoidPtr)arpMappingPtr,
                                      TM_8BIT_ZERO );
        }
        else
        {
/* Locate ARP entry in the tree, using the key, and host mask */
            errorCode = TM_ENOENT;
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
            if ( tm_8bit_one_bit_set(arpCommand, TM_6_ARP_CMD_FLAG) )
            {
#endif /* TM_USE_IPV4 */
                rtePtr = tfRtFindIpAddrIpNet( 
#ifdef TM_USE_STRONG_ESL
                                              arpMappingPtr->arpmDevPtr,
#endif /* TM_USE_STRONG_ESL */
                                              &(arpMappingPtr->arpm6IpAddress),
                                              TM_6_IP_HOST_PREFIX_LENGTH);
#ifdef TM_USE_IPV4
            }
            else
            {
                rtePtr = tf4RtFindIpAddrIpNet(
#ifdef TM_USE_STRONG_ESL
                                               arpMappingPtr->arpmDevPtr,
#endif /* TM_USE_STRONG_ESL */
                                               arpMappingPtr->arpm4IpAddress,
                                               TM_IP_HOST_MASK);
            }
#endif /* TM_USE_IPV4 */
#else /* !TM_USE_IPV6 */
            rtePtr = tfRtFindIpAddrIpNet( 
#ifdef TM_USE_STRONG_ESL
                                          arpMappingPtr->arpmDevPtr,
#endif /* TM_USE_STRONG_ESL */
                                          arpMappingPtr->arpm4IpAddress,
                                          TM_IP_HOST_MASK);
#endif /* !TM_USE_IPV6 */
            if (    (rtePtr != (ttRteEntryPtr)0)
                 && tm_16bit_one_bit_set( rtePtr->rteFlags, TM_RTE_ARP )
                 && tm_16bit_all_bits_set( rtePtr->rteFlags2,
                                           TM_RTE2_HOST | TM_RTE2_UP ) )
            {
                if (tm_8bit_one_bit_set(arpCommand, TM_ARP_DEL_BY_IP))
/* Delete even non resolved ARP entry */
                {
                    errorCode = TM_ENOERROR;
                    af = (tt16Bit)AF_INET;
#ifdef TM_USE_IPV6
                    if (tm_8bit_one_bit_set(arpCommand, TM_6_ARP_CMD_FLAG))
                    {
                        af = (tt16Bit)AF_INET6;
                    }
#endif /* TM_USE_IPV6 */
                    tfRtRemoveHostEntry(rtePtr, 1, af);
                }
                else if (tm_8bit_one_bit_set(arpCommand, TM_ARP_CHK_BY_IP))
/* indicate whether the entry exists or not */
                {
                    errorCode = TM_ENOERROR;
                }
                else
/*
 * All other commands valid on resolved ARP entries only, i.e. if
 * TM_RTE_LINK_LAYER set
 */
                {
                    if ( tm_16bit_one_bit_set(rtePtr->rteFlags, 
                                              TM_RTE_LINK_LAYER ) )
                    {
                        length = rtePtr->rteHSPhysAddrLen;
/*
 * Since code is common between IPv6, and IPv4, reset the IPV6 flag,
 * to keep the code smaller.
 */
                        arpCommand = (tt8Bit)(arpCommand & (TM_ARP_CMD_SET));
                        switch(arpCommand)
                        {
                        case TM_ARP_GET_BY_IP:
                            if (arpMappingPtr->arpmPhysAddrLen < length)
                            {
                                errorCode = TM_EINVAL; /* not enough space */
                                arpMappingPtr->arpmPhysAddrLen 
                                                        = (tt8Bit)length;
                            }
                            else
                            {
                                errorCode = TM_ENOERROR; /* ARP entry found */
                                tm_bcopy( rtePtr->rteHSPhysAddr,
                                          arpMappingPtr->arpmPhysAddr,
                                          length);
                                arpMappingPtr->arpmPhysAddrLen 
                                                        = (tt8Bit)length;
#ifdef TM_SNMP_MIB
/* ecr: get the devIndex and TTL for the SNMP Agent */
                                arpMappingPtr->arpmDevIndex =
                                        (int)rtePtr->rteOrigDevPtr->devIndex;
#endif /* TM_SNMP_MIB */
                                arpMappingPtr->arpmTtl = rtePtr->rteTtl;
                            }
                            break;
                        case TM_ARP_SET_TTL:
                            errorCode = TM_ENOERROR; /* ARP entry found */
                            tm_phys_addr_match(arpMappingPtr->arpmPhysAddr,
                                               rtePtr->rteHSPhysAddr,
                                               length,
                                               match);
                            if (match == length)
                            {
                                errorCode = TM_ENOERROR; /* ARP entry found */
                                rtePtr->rteTtl = arpMappingPtr->arpmTtl;
                                if (rtePtr->rteTtl == TM_RTE_INF)
                                {
/* static ARP cache entry (one created by the user) has an infinite lifetime */
                                    rtePtr->rteFlags |= TM_RTE_STATIC;
                                }
                                else
                                {
/* dynamic ARP cache entry (one resolved by ARP) has a finite lifetime */
                                    tm_16bit_clr_bit( rtePtr->rteFlags,
                                                      TM_RTE_STATIC );
#ifdef TM_USE_IPV6
                                    tm_kernel_set_critical;
                                    rtePtr->rteHSLastReachedTickCount = tvTime;
                                    tm_kernel_release_critical; 
#endif /* TM_USE_IPV6 */
                                }
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
                                tm_kernel_set_critical;
                                rtePtr->rteLastUpdated = tm_snmp_time_ticks(tvTime);
                                tm_kernel_release_critical;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
                            }
                            break;
                        case TM_ARP_SET_PHYS:
                            errorCode = TM_ENOERROR; /* ARP entry found */
                            tm_bcopy( arpMappingPtr->arpmPhysAddr,
                                      rtePtr->rteHSPhysAddr,
                                      arpMappingPtr->arpmPhysAddrLen);
                            rtePtr->rteHSPhysAddrLen =
                                              arpMappingPtr->arpmPhysAddrLen;
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
                            tm_kernel_set_critical;
                            rtePtr->rteLastUpdated = tm_snmp_time_ticks(tvTime);
                            tm_kernel_release_critical;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
                            break;
                        default:
/* someone passed us an unrecognized command code, return an error */
                            errorCode = TM_EINVAL;
                            break;
                        }
                    }
                }
            }
        }
/* UNLOCK to release access to the routing table */
        tfRtTreeUnlockNResetCaches();
    }
    return errorCode;
}

/*
 * Tree walk process routine. Called for each entry in the
 * routing tree, to execute the user command passed to tfRtArpCommand()
 * (TM_ARP_GET_BY_PHYS, or TM_ARP_DEL_BY_PHYS).
 * Returns with no error if the routing entry is an ARP entry corresponding
 * to the ttArpMapping structure entry.
 */
static int tfRtArpMappingCB (ttRteEntryPtr rtePtr, ttVoidPtr voidPtr)
{
    ttArpMappingPtr arpMappingPtr;
    int length;
    int match;
#ifdef TM_USE_IPV6
    int isIpv4Mapped;
    int isIpv4Command;
#endif /* TM_USE_IPV6 */
    int errorCode;

    arpMappingPtr = (ttArpMappingPtr)voidPtr;
    errorCode = TM_ENOENT;
/* Check that the routing entry is an ARP entry, and that it is up */
    if (    tm_16bit_all_bits_set( rtePtr->rteFlags,
                                   TM_RTE_LINK_LAYER | TM_RTE_ARP )
         && tm_16bit_all_bits_set( rtePtr->rteFlags2,
                                   TM_RTE2_HOST | TM_RTE2_UP ) 
#ifdef TM_USE_STRONG_ESL
         && (rtePtr->rteOrigDevPtr == arpMappingPtr->arpmDevPtr) 
#endif /* TM_USE_STRONG_ESL */
       )
    {
        length = arpMappingPtr->arpmPhysAddrLen;
        if (length == TM_ETHER_ADDRESS_LENGTH)
        {
            match = tm_ether_match( arpMappingPtr->arpmEnetAddress,
                                    rtePtr->rteHSEnetAdd );
        }
        else
        {
            tm_phys_addr_match(arpMappingPtr->arpmPhysAddr,
                               rtePtr->rteHSPhysAddr, length, match);
            if (match != length)
            {
                match = 0;
            }
        }
        if (match)
        {
#ifdef TM_USE_IPV6
            isIpv4Mapped = tm_6_addr_is_ipv4_mapped(&(rtePtr->rteDest));
            isIpv4Command = tm_8bit_bits_not_set(arpMappingPtr->arpmCommand,
                                                 TM_6_ARP_CMD_FLAG);
            if (   (isIpv4Mapped && isIpv4Command)
                || (!isIpv4Mapped && !isIpv4Command) )
#endif /* TM_USE_IPV6 */
            {
                switch(arpMappingPtr->arpmCommand)
                {
#ifdef TM_USE_IPV4
                    case TM_4_ARP_GET_BY_PHYS:
#ifdef TM_USE_IPV6
                        tm_6_ip_mapped_copy_6to4(
                                              &(rtePtr->rteDest),
                                              arpMappingPtr->arpm4IpAddress);
#else /* !TM_USE_IPV6 */
                        tm_ip_copy(rtePtr->rteDest,
                                   arpMappingPtr->arpm4IpAddress);
#endif /* !TM_USE_IPV6 */
#ifdef TM_SNMP_MIB
                        arpMappingPtr->arpmDevIndex =
                                        (int)rtePtr->rteOrigDevPtr->devIndex;
#endif /* TM_SNMP_MIB */
                        break;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
                    case TM_6_ARP_GET_BY_PHYS:
                        tm_6_ip_copy_structs(rtePtr->rteDest,
                                             arpMappingPtr->arpm6IpAddress);
#ifdef TM_SNMP_MIB
                        arpMappingPtr->arpmDevIndex =
                                        (int)rtePtr->rteOrigDevPtr->devIndex;
#endif /* TM_SNMP_MIB */
                        break;
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
                    case TM_4_ARP_DEL_BY_PHYS:
                        tfRtRemoveHostEntry(rtePtr, 1, (tt16Bit)AF_INET);
                        break;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
                    case TM_6_ARP_DEL_BY_PHYS:
                        tfRtRemoveHostEntry(rtePtr, 1, (tt16Bit)AF_INET6);
                        break;
#endif /* TM_USE_IPV6 */
                    default:
                        break;
                }
                errorCode = TM_ENOERROR; /* done */
            }
        }
    }
    return errorCode; /* ask for next entry, if errorCode is TM_ENOENT */
}



/*
 * Given a destination IP address in the cache, obtain a pointer
 * to the corresponding routing table and store it in the cache.
 * 1. Obtain exclusive access to the routing table.
 * 2. Get a routing entry corresponding to the given destination address
 * 3. If a match has been found we store it in the second field of the cache 
 * entry, increase the ownership count of the routing entry. error is 0.
 * 3. If a match has not been found, we store a null pointer in the second 
 * field. error is TM_EHOSTUNREACH
 * 4. We release exclusive access to the routing table.
 * 5. We return the error.
 * Parameter   Description
 * ttRtCachePtr    Pointer to a cache entry where the destination IP address
 *                 is stored. On successful return, the route entry pointer 
 *                 inside the cache entry is initialized
 * 
 * Returns
 * 0               no error
 * TM_EHOSTUNREACH no match
 * TM_EINVAL       parameter error
 * TM_EWOULDBLOCK  routing table was locked and the user did not want to wait
 */
int tfRtGet (ttRtCacheEntryPtr rtcCachePtr)
{
    ttRteEntryPtr               rtePtr;
#ifdef TM_USE_IPV6
#if defined(TM_6_USE_MIP_MN) || defined(TM_6_ENABLE_ONLINK_ASSUMPTION)
    ttDeviceEntryPtr            devPtr;
#endif /* TM_6_USE_MIP_MN || TM_6_ENABLE_ONLINK_ASSUMPTION */
    tt6IpAddressPtr             destIpAddrPtr;
#ifdef TM_6_ENABLE_ONLINK_ASSUMPTION
    tt6RtDefRouterHeadPtr       def6HeadPtr;
#endif /* TM_6_ENABLE_ONLINK_ASSUMPTION */
#endif /* TM_USE_IPV6 */
#ifdef TM_6_USE_MIP_CN
    tt6IpAddress                coAddr;
    tt6BindingEntryPtr          bindingPtr;
#endif /* TM_6_USE_MIP_CN */
#ifdef TM_6_USE_MIP_MN
    struct in6_addr             defRtrAddr;
#endif /* TM_6_USE_MIP_MN */
#ifdef TM_6_ENABLE_ONLINK_ASSUMPTION
    tt32Bit                     scopeId;
#endif /* TM_6_ENABLE_ONLINK_ASSUMPTION */
    int                         errorCode;

#ifdef TM_ERROR_CHECKING
#ifdef TM_USE_NETSTAT
#ifdef TM_DEBUG_LOGGING
    tfxLogRteTable("tfRtGet");
    tfxLogArpTable("tfRtGet");
#ifdef TM_USE_IPV6    
    tm_debug_log4("tfRtGet: %lx:%lx:%lx:%lx",
                  ntohl(rtcCachePtr->rtcDestIpAddr.s6LAddr[0]),
                  ntohl(rtcCachePtr->rtcDestIpAddr.s6LAddr[1]),
                  ntohl(rtcCachePtr->rtcDestIpAddr.s6LAddr[2]),
                  ntohl(rtcCachePtr->rtcDestIpAddr.s6LAddr[3]));
#else /* ! TM_USE_IPV6 */    
    tm_debug_log1("tfRtGet: %lx",
                  ntohl(rtcCachePtr->rtcDestIpAddr));
#endif /* ! TM_USE_IPV6 */    
#endif /* TM_DEBUG_LOGGING */
#endif /*TM_USE_NETSTAT*/

    if (rtcCachePtr == TM_RTC_NULL_PTR)
    {
        errorCode = TM_EINVAL;
        goto exitRtGet;
    }
#endif /* TM_ERROR_CHECKING */

#ifdef TM_USE_IPV6
    destIpAddrPtr = &(rtcCachePtr->rtcDestIpAddr);
#endif /* TM_USE_IPV6 */

#ifdef TM_6_USE_MIP_CN
    if (rtcCachePtr->rtcRtoFlag != 0xFF)
/* RTO not turned off */
    {
/* lock the binding cache, tm_context(tv6BindingLock)*/
        tm_lock_wait(&tm_context(tv6BindingLock));
/* lookup the binding entry using the destination address of the packet (home
   address of the binding) */
        rtcCachePtr->rtc6CnBindingPtr =
            (ttVoidPtr) tf6MipGetBinding(destIpAddrPtr);
        bindingPtr = (tt6BindingEntryPtr) rtcCachePtr->rtc6CnBindingPtr;

/* If we find a non-home reg binding, we replace the dest add with CoA */
        if (   (bindingPtr != TM_6_BINDING_NULL_PTR)
#ifdef TM_6_USE_MIP_HA
            && (bindingPtr ->bind6HomeReg == TM_8BIT_NO) 
#endif /* TM_6_USE_MIP_HA */
            )
        {
/* copy the CoA to a local variable, since after the unlock, the binding could
   be removed at any time */
            tm_6_ip_copy_structs(bindingPtr->bind6CoAddr, coAddr);

/* use the CoA as the destination address */
            destIpAddrPtr = &coAddr;
        }
/* Unlock the binding cache */
        tm_unlock(&tm_context(tv6BindingLock));
    }
#endif /* TM_6_USE_MIP_CN */

/* LOCK the tree to obtain exclusive access to the routing table */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));
/* Find an entry in the routing table for the destination IP address */
/* If entry is a LAN network destination, clone for specific host */
#ifdef TM_USE_IPV6
    rtePtr = tfRtFindNClone( 
#ifdef TM_USE_STRONG_ESL
                              rtcCachePtr->rtcDevPtr,
#endif /* TM_USE_STRONG_ESL */
                              destIpAddrPtr);
#else /* !TM_USE_IPV6 */
    rtePtr = tfRtFindNClone(
#ifdef TM_USE_STRONG_ESL
                             rtcCachePtr->rtcDevPtr,
#endif /* TM_USE_STRONG_ESL */
                             rtcCachePtr->rtcDestIpAddr);
#endif /* !TM_USE_IPV6 */
    if (rtePtr != TM_RTE_NULL_PTR)
    {
/*
 * If one found, store in cache and increase owner count, also store
 * corresponding ARP entry if route is indirect
 */
        tfRtCacheGet(rtcCachePtr, rtePtr);
        errorCode = TM_ENOERROR;
    }
    else
    {
        errorCode = TM_EHOSTUNREACH;
    }

#ifdef TM_6_USE_MIP_MN
/* if destIpAddrPtr points to the home agent address and MIPv6 is active,
   then change it to use the current default router instead (i.e. lookup
   using IPv6 unspecified address). This is done because when the virtual
   home interface is open, it is quite possible that the prefix of the home
   agent address is created as a local route associated with that interface,
   and we would never be able to send to the home agent if we used that local
   route. */
    if ((errorCode == TM_ENOERROR)
        && (IN6_ARE_ADDR_EQUAL(
                destIpAddrPtr,
                &(tm_context(tv6MnVect).mns6HomeAgentAddr)))
        && (tm_context(tv6MnVect).mns6VirtHomeIfacePtr
            == rtcCachePtr->rtcRtePtr->rteOrigDevPtr))
    {
/* attempt to send through the current default router */
        tm_kernel_set_critical;
        tm_6_ip_copy_structs(
            tm_context(tv6MnVect).mns6LinkLocalDefRtrAddr, defRtrAddr);
        devPtr = tm_context(tv6MnVect).mns6MobileIfacePtr;
        tm_kernel_release_critical;

/* assume failure */
        errorCode = TM_EHOSTUNREACH;
/* the router address must be link-local scope, otherwise it is bogus */
        if (IN6_IS_ADDR_LINKLOCAL(&defRtrAddr))
        {
            tfRtCacheUnGet(rtcCachePtr);

/* act as if we got a Redirect to the router */
            tf6RtAddRoute(devPtr,
                          destIpAddrPtr,
                          TM_6_IP_HOST_PREFIX_LENGTH,
                          &defRtrAddr, 
                          TM_16BIT_ZERO,
                          TM_MAX_IPS_PER_IF, /* bogus */
                          1, /* hops */
                          TM_RTE_INF,
                          TM_RTE_REDIRECT|TM_RTE_INDIRECT,
/* Routing tree already locked */
                          TM_8BIT_ZERO);
/* Do not need to flush caches */
            tm_context(tvRtRadixTreeHead).rthResetCachesFlags = TM_16BIT_ZERO;
            tm_context(tvRtRadixTreeHead).rthResetCachesAf = TM_16BIT_ZERO;
            rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                                devPtr,
#endif /* TM_USE_STRONG_ESL */
                                destIpAddrPtr);
            if (rtePtr != TM_RTE_NULL_PTR)
            {
/*
 * If one found, store in cache and increase owner count, also store
 * corresponding ARP entry if route is indirect
 */
                tfRtCacheGet(rtcCachePtr, rtePtr);
                errorCode = TM_ENOERROR;
            }
        }
    }
#endif /* TM_6_USE_MIP_MN */

/* UNLOCK the tree to release access to the routing table */
/* No need to reset caches */
    tfRtTreeUnlock(); 

#ifdef TM_6_ENABLE_ONLINK_ASSUMPTION
    if ((errorCode != TM_ENOERROR) && !IN6_IS_ADDR_MULTICAST(destIpAddrPtr)
        && !tm_6_ip_zero(destIpAddrPtr)
#ifdef TM_USE_IPV4
        && !(IN6_IS_ADDR_V4MAPPED(destIpAddrPtr))
#endif /* TM_USE_IPV4 */
        )
    {
/* No route was found. Per [RFC2461].R5.2:30, if there is no default router,
   we should now assume the destination is on-link, and send the packet out
   any interface that has IPv6 enabled. This entails creating a special
   unresolved ARP cache entry when we don't have a clonable local route to
   clone it from. */
#ifdef TM_USE_STRONG_ESL
        def6HeadPtr = (tt6RtDefRouterHeadPtr)0;
        if (rtcCachePtr->rtcDevPtr != (ttDeviceEntryPtr)0)
        {
            def6HeadPtr =
                    &(rtcCachePtr->rtcDevPtr->dev6DefaultRouterHead);
        }
        else
        {
/* Find an IPv6 enabled device with a default router */
            devPtr = tf6RtGetNextDefaultGatewayList(0);
            if (devPtr != (ttDeviceEntryPtr)0)
            {
                def6HeadPtr = &(devPtr->dev6DefaultRouterHead);
            }
        }
#else /* !TM_USE_STRONG_ESL */
        def6HeadPtr =
                    &(tm_context(tvRtRadixTreeHead).rth6DefaultRouterHead);
#endif /* TM_USE_STRONG_ESL */
        if (
#ifdef TM_USE_STRONG_ESL
             (def6HeadPtr == (tt6RtDefRouterHeadPtr)0) ||
#endif /* TM_USE_STRONG_ESL */
             (def6HeadPtr->drh6RouterCount == 0)
           )
        {
#ifdef TM_USE_STRONG_ESL
            if (rtcCachePtr->rtcDevPtr != (ttDeviceEntryPtr)0)
            {
                devPtr = rtcCachePtr->rtcDevPtr;
                if (    !(tm_6_link_local_cached(devPtr))
                     || (    !(tm_6_ll_is_lan(devPtr))
                          && !(tm_ll_uses_ppp(devPtr))) )
                {
                    errorCode = TM_EHOSTUNREACH;
                    goto exitRtGet;
                }
            }
            else
#endif /* TM_USE_STRONG_ESL */
            {
                tm_6_get_embedded_scope_id(destIpAddrPtr, scopeId);
/* search the list of IPv6-enabled devices until we either find one
   that supports link-layer addresses or which is a PPP interface */
                tm_lock_wait(&tm_context(tvDeviceListLock));
                devPtr = tm_context(tvDeviceList);
                while (devPtr != TM_DEV_NULL_PTR)
                {
                    if (    (tm_6_link_local_cached(devPtr))
                        && (tm_6_ll_is_lan(devPtr) || tm_ll_uses_ppp(devPtr))
                        && (    !IN6_IS_ADDR_LINKLOCAL(destIpAddrPtr)
                             || (scopeId == (tt32Bit)devPtr->devIndex) )
                       )
                    {
                        break;
                    }
                    devPtr = devPtr->devNextDeviceEntry;
                }
                tm_unlock(&tm_context(tvDeviceListLock));
            }
            if ((devPtr == TM_DEV_NULL_PTR) || tm_dev_is_loopback(devPtr))
            {
/*  make sure that it isn't the loopback interface. If it is, we cannot
    continue. */
                errorCode = TM_EHOSTUNREACH;
                goto exitRtGet;
            }
/* LOCK the tree to obtain exclusive access to the routing table */
            tm_call_lock_wait(&tm_context(tvRtTreeLock));
            if (tm_ll_uses_ppp(devPtr))
            {
/* use the peer PPP address as the destination address */
                destIpAddrPtr = &(devPtr->dev6Pt2PtPeerIpAddr);
            }
            else
            {
/* if the interface supports link-layer addresses, then create a host
   entry for the destination in the INCOMPLETE state (i.e. without the
   TM_RTE_LINK_LAYER bit set) and return this as the routing entry. */
                tf6RtAddHost(
                    devPtr,
                    destIpAddrPtr,
                    TM_8BIT_NULL_PTR, /* physical address ptr */
                    (tt8Bit) 0,       /* physical address len */
                    TM_6_RT_RESERVED_MHOME_INDEX,
                    TM_RTE_INF,
                    TM_RTE_ARP | TM_RTE_CLONED,
                    TM_8BIT_NO);     /* tree already locked */
            }

/* Find an entry in the routing table for the destination IP address */
/* If entry is a LAN network destination, clone for specific host */
            rtePtr = tfRtFindNClone(
#ifdef TM_USE_STRONG_ESL
                                     devPtr,
#endif /* TM_USE_STRONG_ESL */
                                     destIpAddrPtr);
            if (rtePtr != TM_RTE_NULL_PTR)
            {
/*
 * If one found, store in cache and increase owner count, also store
 * corresponding ARP entry if route is indirect
 */
                tfRtCacheGet(rtcCachePtr, rtePtr);
                errorCode = TM_ENOERROR;
            }
/* UNLOCK the tree to release access to the routing table */
/* No need to reset caches */
            tfRtTreeUnlock(); 
        }
    }
#endif /* TM_6_ENABLE_ONLINK_ASSUMPTION */

#if (defined(TM_ERROR_CHECKING) || defined(TM_6_ENABLE_ONLINK_ASSUMPTION))
exitRtGet:
#endif /* TM_ERROR_CHECKING or TM_6_ENABLE_ONLINK_ASSUMPTION */

    return errorCode;
}

/*
 * Cache routing entry (and corresponding ARP entry if routing entry
 * is indirect).
 * 1. Store passed routing entry in the cache and increase its owner count.
 * 2. If routing entry is indirect, store corresponding ARP cache entry in
 *    the cache and increase its owner count.
 *
 * Parameter   Description
 * rtcPtr    Pointer to a cache entry where we store the route entry pointer
 *           and possibly corresponding ARP entry if route entry is indirect.
 * rtePtr    Pointer to routing entry to be cached.
 *
 * Returns
 * none.
 */
static void tfRtCacheGet (ttRtCacheEntryPtr rtcPtr, ttRteEntryPtr rtePtr)
{
    tm_incr_owner_count(rtePtr->rteOwnerCount);
    rtcPtr->rtcRtePtr = rtePtr;
    if (    (tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_INDIRECT))
         && (rtePtr->rteClonedPtr != TM_RTE_NULL_PTR) )
    {
/*
 * If route is indirect, point to ARP entry if there is one, and increase its
 * owner count.
 */
        rtcPtr->rtcRteClonedPtr = rtePtr->rteClonedPtr;
        tm_incr_owner_count(rtePtr->rteClonedPtr->rteOwnerCount);
    }
    return;
}


/*
 * tfRtUnGet Function Description:
 * Release ownership to a routing entry. If ownership is zero, recycle the
 * entry.
 * 1. We decrease the ownership count of the routing entry.
 * 2. If the ownership count is zero
 *    a. If entry is a gateway entry and points to a local/Arp entry,
 *       decrease ownership of the entry the gateway points to.
 *    b. recycle the routing entry.
 * Note about owner count and UP/IN flag: We have to use ownership count
 * to keep track of number of people pointing to a routing entry. We
 * have to use the UP/IN flag to figure out whether a route is still valid
 * (in the tree). We can only recycle the entry when owner count reaches 0.
 *
 * Parameter    Description
 * ttRtEntryPtr Pointer to a routing entry
 *
 * Return values none:
 */

void tfRtUnGet (ttRteEntryPtr rtePtr)
{
    unsigned int ownerCount;


#ifdef TM_ERROR_CHECKING
#ifdef TM_USE_NETSTAT
#ifdef TM_DEBUG_LOGGING
    tfxLogRteTable("tfRtUnGet");
    tfxLogArpTable("tfRtUnGet");
    tfxLogRteEntry("tfRtUnGet", rtePtr);
#endif /* TM_DEBUG_LOGGING */
#endif /*TM_USE_NETSTAT*/
/* Ownership count on a routing entry should be != 0 before an unget */
    if (rtePtr->rteOwnerCount == 0)
    {
        tfKernelError("tfRtUnGet", "ownership count of routing entry < 0\n");
    }
#endif /* TM_ERROR_CHECKING */
    tm_decr_owner_count(ownerCount, rtePtr->rteOwnerCount);
/*
 * if owner count is 0, entry no longer in the tree, and nobody is
 * pointing to it
 */
    if (ownerCount == 0)
    {
#ifdef TM_ERROR_CHECKING
/* The leaf node/internal node better not be root, or in the tree */
        if (    tm_16bit_one_bit_set(rtePtr->rteFlags2, TM_RTE2_UP)
             || tm_16bit_one_bit_set( (   rtePtr->rteLeafNode.rtnFlags
                                        | rtePtr->rteInternalNode.rtnFlags),
                                      ( TM_RTN_IN | TM_RTN_ROOT ) ) )
        {
            rtePtr->rteOwnerCount = 1; /* Do not recycle the entry */
            tfKernelError("tfRtUnGet",
        "Attempting to free a routing entry still in the tree, or a root!\n");
        }
        else
#endif /* TM_ERROR_CHECKING */
        {
            if ( tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_INDIRECT) )
/* If gateway entry */
            {
                if (rtePtr->rteClonedPtr != TM_RTE_NULL_PTR)
                {
/* If gateway entry points to cloned route, dereference cloned entry. */
                    tfRtUnGet(rtePtr->rteClonedPtr);
                    rtePtr->rteClonedPtr = TM_RTE_NULL_PTR;
                }
            }
/* Put back the entry in the recycle queue */
            tm_recycle_free(rtePtr, TM_RECY_RTE);
#ifdef TM_ERROR_CHECKING
/* Keep track of out of tree non recycled entries */
            tm_context(tvRteLimboCount)--;
#endif /* TM_ERROR_CHECKING */
        }
    }
}



/*
 * Static functions
 */

#if (defined(TM_USE_IPV4) || defined(TM_6_PMTU_DISC))
/*
 * Route Arp timer. Fired every tvArpResolution (60 seconds) to check on a
 * route Arp entry. Need to lock the tree to prevent insertion of a
 * new ARP entry, while we are walking down the list.
 * If timer expires, reset the mapping
 */
static void tfRtArpTimer (ttVoidPtr      timerBlockPtr,
                          ttGenericUnion userParm1,
                          ttGenericUnion userParm2)
{
/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);
    
    (void)tfRtTreeWalk(tfRtArpAgeEntryCB,
                       userParm1.genVoidParmPtr,
                       TM_8BIT_YES);
    return;
}
#endif /* TM_USE_IPV4 || TM_6_PMTU_DISC */

/*
 * Walk down the tree and age all RIP and Indirect routes.
 *
 * We call the walk down tree routing, passing the function
 * tfRtAgeEntryCB() as a parameter, which will be called for each
 * leaf, with the corresponding routing entry as a parameter.
 */
static void tfRtTimer (ttVoidPtr      timerBlockPtr,
                       ttGenericUnion userParm1,
                       ttGenericUnion userParm2)
{

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);
    
    (void)tfRtTreeWalk(tfRtAgeEntryCB, userParm1.genVoidParmPtr, TM_8BIT_YES);
    return;
}

/*
 * tfRtTreeWalk function description:
 * Walk down the tree, and for each reached leaf call the routing entry
 * processing function. We stop when we have walked down the whole tree,
 * or when the processing function returns with no error.
 *
 * Lock the Tree if not already locked.
 * 0) Indicate that we are walking the tree by increasing the tree walk count
 * 1) We try and find the leftmost leaf first
 * 2) Then in a loop, we get the next leaf from the tree (see
 *    tm_rt_get_next_leaf() for details, and process the
 *    current leaf and its duplicates while the next leaf is not the
 *    rightmost root leaf.
 * 3) decrease the tree walk count indicating that we finished one tree walk.
 *    If the walk count is 0, i.e we are on the outer part of nested tree walks,
 *    then really removed routing entries queued to be removed.
 * Unlock the tree if locked here (cleaning up caches if needed)
 *
 * Algorithm to process the current leaf and its duplicates:
 * In a loop, we get the next duplicate, and call the processing function
 * passing the routing entry corresponding to the current leaf,
 * while there is a next duplicate.
 *
 *
 * In both loops, care is taken to compute the next leaf, before processing
 * the current leaf since current leaf might be removed from the tree.
 *
 */
int tfRtTreeWalk ( ttIntRtFuncPtr processRtePtr,
                   ttVoidPtr      voidPtr,
                   tt8Bit         needLock )
{
    ttRtRadixNodePtr    nodePtr;
    ttRtRadixNodePtr    nextPtr;
    ttRtRadixNodePtr    nextDupPtr;
    int                 errorCode;

#ifdef TM_LOCK_NEEDED
    if (needLock != TM_8BIT_ZERO)
    {
/* LOCK the tree to obtain exclusive access to the routing table */
        tm_call_lock_wait(&tm_context(tvRtTreeLock));
    }
#else /* !TM_LOCK_NEEDED */
    TM_UNREF_IN_ARG(needLock);
#endif /* !TM_LOCK_NEEDED */
    tm_context(tvRtRadixTreeHead).rthTreeWalkCount++;
/* start at root of the tree */
    nodePtr = tm_context(tvRtTreeHeadPtr);
/* Go all the way to the left. Starting leaf to check */
    tm_rt_tree_leftmost_leaf(nodePtr);
#ifdef TM_ERROR_CHECKING
    tm_rt_check_null_leaf(nodePtr); /* check that the leaf is non null */
#endif /* TM_ERROR_CHECKING */
    nextPtr = nodePtr; /* starting point to search next leaf */
    errorCode = TM_ENOENT;
    do
    {
/*
 * Get next leaf to check:
 * While we are a right child and not a root, we go up the tree.
 * Now we are either a left child or root; we take the leftmost leaf of the
 * parent's right child.
 * (note that the root's parent is the parent itself).
*/
        while (    (tm_rt_right_child(tm_rt_parent(nextPtr)) == nextPtr)
                && tm_16bit_bits_not_set(nextPtr->rtnFlags, TM_RTN_ROOT) )
        {
            nextPtr = tm_rt_parent(nextPtr);
        }
        nextPtr = tm_rt_right_child(tm_rt_parent(nextPtr));
        tm_rt_tree_leftmost_leaf(nextPtr);
/* End of next leaf retrieval */
#ifdef TM_ERROR_CHECKING
        tm_rt_check_null_leaf(nextPtr); /* check that the leaf is non null */
#endif /* TM_ERROR_CHECKING */
/* process the current leaf nodePtr and its duplicates */
        nextDupPtr = nodePtr; /* starting point of duplicate search */
        do
        {
/* compute the next duplicate since currrent leaf could be removed */
            nextDupPtr = tm_rt_next_dup(nextDupPtr);
/* process current leaf/duplicate nodePtr (root nodes are not leaves) */
            if ( tm_16bit_bits_not_set(nodePtr->rtnFlags, TM_RTN_ROOT) )
            {
#ifdef TM_ERROR_CHECKING
                if ( (tm_rt_leaf_to_rte(nodePtr))->rteOwnerCount == 0 )
                {
                    tfKernelError("tfRtTreeWalk", "Corrupted Patricia tree");
                }
#endif /* TM_ERROR_CHECKING */
                errorCode = (*processRtePtr)( tm_rt_leaf_to_rte(nodePtr),
                                              voidPtr );
                if (errorCode == TM_ENOERROR)
                {
                    goto treeWalkExit;
                }
            }
/* process next duplicate */
            nodePtr = nextDupPtr;
        }
        while (nodePtr != TM_RADIX_NULL_PTR);
/* end of processing the current leaf and its duplicates */
/* process the next leaf. Save pointer to reached leaf */
        nodePtr = nextPtr;
/* We are done when we have reached the rightmost root leaf */
    }
    while ( tm_16bit_bits_not_set(nodePtr->rtnFlags, TM_RTN_ROOT) );
treeWalkExit:
    tm_context(tvRtRadixTreeHead).rthTreeWalkCount--;
    if (tm_context(tvRtRadixTreeHead).rthTreeWalkCount == TM_16BIT_ZERO)
/*
 * No longer in a tree walk. Safe to remove the routing entries queued to
 * be removed while we were walking the tree(s).
 */
    {
        tfRtTreeRem();
    }
#ifdef TM_LOCK_NEEDED
    if (needLock != TM_8BIT_ZERO)
    {
/* UNLOCK the tree to release access to the routing table */
        tfRtTreeUnlockNResetCaches();
    }
#endif /* TM_LOCK_NEEDED */
    return errorCode;
}


/*
 * Algorithm to age the passed routing entry:
 * It routing entry is a REDIRECT or RIP routing entry, or SNMP routing
 * entry, or router discovery, path MTU discovery entry we
 * decrease the routing entry TTL by 1, and if it reaches zero for a
 * non path MTU discovery entry, we remove the entry. If the entry is a
 * Path MTU discovery entry (TM_RTE_PMTU_DISC set, and TM_RTE_NO_PMTU_DISC
 * not set, and it reaches zero, try and pick the next plateau MTU value.
 * If there is a next plateau MTU, and it is not bigger than the device
 * MTU, then add a new host entry with a 2 minutes timeout, otherwise
 * remove the entry.
 */
static int tfRtAgeEntryCB (ttRteEntryPtr rtePtr, ttVoidPtr voidPtr)
{
#ifdef TM_PMTU_DISC
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
   ttIpAddress dest6IpAddr;
#endif /* TM_USE_IPV6 */
   tt16Bit     nextMtu;
   tt8Bit      needRemoveEntry;
#endif /* TM_USE_IPV4 */
#endif /* TM_PMTU_DISC */

/* Unused Parameter */
    voidPtr=voidPtr;
    if ( tm_16bit_one_bit_set( rtePtr->rteFlags, TM_RTE_DYNAMIC) )
/* RIP, Router Discovery, Path MTU discovery, or REDIRECT gateway entry */
    {
        if ( rtePtr->rteTtl != TM_RTE_INF )
        {
/* decrease time to live */
            if ( rtePtr->rteTtl > tm_context(tvRtTimerResolution) )
            {
                rtePtr->rteTtl -= tm_context(tvRtTimerResolution);
            }
            else
            {
/*
 * If TTL is zero, remove entry.
 *
 * For Path MTU discovery entries:
 * IPv6 PMTU entries should be allowed to timeout and be removed as usual.
 * This will cause anything using the previous PMTU to reset to the link MTU,
 * and perform PMTU discovery again if necessary. ([RFC1981].R4:20,
 * [RFC1981].R5.3:20)
 * IPv4 PMTU entries will perform the algorithm below, which raises the PMTU
 * to the next plateau, etc
 */
#if (defined(TM_USE_IPV4) && defined(TM_PMTU_DISC))
                needRemoveEntry = TM_8BIT_YES;                
                if (    tm_16bit_one_bit_set(rtePtr->rteFlags,
                                             TM_RTE_PMTU_DISC)
                     && tm_16bit_bits_not_set(rtePtr->rteFlags,
                                             TM_RTE_NO_PMTU_DISC)
#ifdef TM_USE_IPV6
                     && (IN6_IS_ADDR_V4MAPPED(&rtePtr->rteDest))
#endif /* TM_USE_IPV6 */
                    )
                {
/*
 * If entry is path MTU discovery, and Path MTU discovery is allowed.
 */
/* OK to use rteMtu unprotected here, because IPv4 entries do not have
   their MTU updated dynamically (as IPv6 ones do). */
                    nextMtu = tfRtPickNextPlateauMtu( rtePtr->rteMtu,
                                                      TM_PMTU_HIGHER );
                    if (    (nextMtu != 0)
/* PATH MTU Maximum not reached */
                         && (nextMtu <= rtePtr->rteDevPtr->devMtu) )
/* and Next MTU does not exceed our interface MTU */
                    {
#ifdef TM_USE_IPV6
                        tm_6_ip_copy_structs(rtePtr->rteDest, dest6IpAddr);
#endif /* TM_USE_IPV6 */
/* Enter new entry with bigger MTU */
                        (void)tfRtAddPathMtuEntry(
#ifdef TM_USE_STRONG_ESL
                                            rtePtr->rteOrigDevPtr,
#endif /* TM_USE_STRONG_ESL */
                                            rtePtr,
#ifdef TM_USE_IPV6
                                            &(dest6IpAddr),
#else /* !TM_USE_IPV6 */
                                            rtePtr->rteDest,
#endif /* !TM_USE_IPV6 */
/*
 * Our Path MTU discovery timed out. Attempt a larger MTU with a 2 minutes
 * timeout
 */
                                            tm_context(tvRtLargerMtuTimeout),
                                              TM_RTE_INDIRECT
                                            | TM_RTE_PMTU_DISC,
                                            nextMtu);
/* Entry has been replaced, do not remove it */
                        needRemoveEntry = TM_8BIT_ZERO;
                    }
                }
                if (needRemoveEntry != TM_8BIT_ZERO)
#endif /* TM_PMTU_DISC && TM_USE_IPV4 */
                {
/* if zero remove entry */
                    tfRtRemoveEntry(rtePtr);
                }
            }
        }
    }
    return TM_ENOENT; /* ask for next entry */
}

#ifdef TM_USE_IPV4
/*
 * Delete an IPv4 ARP entry pointed to by rtePtr.
 */
static void tf4RtArpDelByRte (ttRteEntryPtr rtePtr)
{
/*
 * If a cache entry or a gateway is pointing to the entry, and if the
 * entry is an ARP entry, invalidate the mapping.
 * Also set the ttl to 0, so that the Arp timer, at the next tick,
 * will check again on the entry ownership to try and remove the entry.
 */
    if (rtePtr->rteOwnerCount > 1)
    {
        if ( tm_16bit_all_bits_set( rtePtr->rteFlags,
                                    TM_RTE_LINK_LAYER | TM_RTE_ARP ) )
        {
            tm_16bit_clr_bit( rtePtr->rteFlags, TM_RTE_LINK_LAYER );
        }
        rtePtr->rteTtl = TM_UL(0);
    }
/* otherwise remove it */
    else
    {
        tfRtRemoveEntry(rtePtr);
    }
    return;
}
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
/*
 * Delete an IPv6 ARP entry pointed to by rtePtr. Do not remove gateways
 * pointing to it.
 */
static void tf6RtArpDelByRte (ttRteEntryPtr rtePtr)
{
#ifdef TM_6_USE_NUD
    ttTimerPtr          tempTimerPtr;
#endif /* TM_6_USE_NUD */

#ifdef TM_6_USE_NUD
    if (rtePtr->rte6HSNudTimerPtr != (ttTimerPtr)0)
    {
        tempTimerPtr = rtePtr->rte6HSNudTimerPtr;
        if (tempTimerPtr != TM_TMR_NULL_PTR)
        { 
            tm_timer_remove(tempTimerPtr) ;
            rtePtr->rte6HSNudTimerPtr = TM_TMR_NULL_PTR;
/* undo the owner count increase when this timer was first started */
            tfRtUnGet(rtePtr);
        }
    }
#endif /* TM_6_USE_NUD */
    if (rtePtr->rteOwnerCount <= 1)
    {
/* Nobody points to this entry via the rteClonedPtr */
        if (tm_16bit_one_bit_set(rtePtr->rteFlags2, TM_RTE2_UP))
        {
            tfRtRemoveEntry(rtePtr);
        }
    }
    else
    {
/* Invalid the mapping forcing a new ARP resolution */
        tm_16bit_clr_bit(rtePtr->rteFlags, TM_RTE_LINK_LAYER);
        rtePtr->rte6HSNudState = TM_6_NUD_CLONED_STATE;
    }
}
#endif /* TM_USE_IPV6 */

#if (defined(TM_USE_IPV4) || defined(TM_6_PMTU_DISC))
/*
 * Function called from tfRtTreeWalk.
 * If the passed routing entry pointer parameter is an ARP entry, or
 * cloned MULTICAST mapping entry, age it.
 * If timeout has expired remove the entry from the tree.
 */
static int tfRtArpAgeEntryCB (ttRteEntryPtr rtePtr, ttVoidPtr voidPtr)
{

#ifdef TM_6_PMTU_DISC
    tt32Bit elapsedMsec;
#endif /* TM_6_PMTU_DISC */
    
    TM_UNREF_IN_ARG(voidPtr); /* avoid compiler warning */

/* If valid host cloned entry, ( ARP/multicast/broadcast mapping entry): */
    if (    tm_16bit_one_bit_set( rtePtr->rteFlags, TM_RTE_CLONED )
         && tm_16bit_all_bits_set( rtePtr->rteFlags2,
                                   TM_RTE2_HOST | TM_RTE2_UP )
         && (rtePtr->rteTtl != TM_RTE_INF) )
    {
#ifdef TM_6_PMTU_DISC
        if (rtePtr->rte6HSNudState != TM_6_NUD_IPV4_STATE)
        {
/* For IPv6 entries, check if the timeout for increasing the PMTU has expired,
   based on the timestamp in the routing entry.  If so, reset the PMTU to
   the link MTU. ([RFC1981].R4:20, [RFC1981].R5.3:20) */
            tm_kernel_set_critical;
            elapsedMsec = (tt32Bit) (tvTime - rtePtr->rte6HSLastPmtuTickCount);
            tm_kernel_release_critical;
            if (elapsedMsec > tm_context(tv6RtPathMtuTimeout)*TM_UL(1000))
            {
/* Need to protect the IPv6 MTU, since it could be changed dynamically
   due to a router advertisement. */
                tm_kernel_set_critical;
                rtePtr->rteMtu = rtePtr->rteDevPtr->dev6Mtu;
                rtePtr->rte6HSLastPmtuTickCount = tvTime;
                tm_kernel_release_critical;
            }
        }
        else
#endif /* TM_6_PMTU_DISC */
        {
#ifdef TM_USE_IPV4
            if ( rtePtr->rteTtl > tm_context(tvArpResolution) )
/* Decrease time to live by ARP resolution (60000 milliseconds) */
            {
                rtePtr->rteTtl -= tm_context(tvArpResolution);
                if (tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_ARP))
                {
                    if (rtePtr->rteTtl <= tm_context(tvArpResolution))
/* If ARP entry time to live was <= 2 * tvArpResolution */
                    {
/* Indicate that ARP entry should be refreshed */
                        rtePtr->rteRefresh = TM_8BIT_YES;
                    }
                }
            }
            else
            {
                tf4RtArpDelByRte(rtePtr);
            }
#endif /* TM_USE_IPV4 */
        }
    }
    return TM_ENOENT;
}
#endif /* TM_USE_IPV4 || TM_6_PMTU_DISC */

/*
 * If rtePtr is an ARP entry not being resolved and not being referenced
 * by another pointer, or a multicast mapping entry, check whether its TTL
 * is less than the current used ARP entry if any (stored in
 * leastUsedRtePtrPtr). It if is less, or there was none stored in
 * leastUsedRtePtrPtr, store it in leastUsedRtePtrPtr.
 */
static int tfRtArpGetLeastUsedCB ( ttRteEntryPtr rtePtr,
                                   ttVoidPtr leastUsedRtePtrPtr )
{
    ttRteEntryPtr leastUsedRtePtr;
#ifdef TM_USE_IPV6
    tt32Bit       leastUsedElapsedMsec;
    tt32Bit       elapsedMsec;
#endif /* TM_USE_IPV6 */

    leastUsedRtePtr = *((ttRteEntryPtr TM_FAR *)leastUsedRtePtrPtr);
    if (   tm_16bit_all_bits_set( rtePtr->rteFlags2,
                                  TM_RTE2_HOST | TM_RTE2_UP )
/* If ARP/MCAST/BCAST mapping entry */
        && tm_16bit_one_bit_set( rtePtr->rteFlags, TM_RTE_CLONED )
/* and not being static */
        && tm_16bit_bits_not_set(rtePtr->rteFlags, TM_RTE_STATIC)
/* and nobody points to it (i.e. no cache, no ARP resolution) */
        && ( rtePtr->rteOwnerCount == 1 ) )
    {
        if ( leastUsedRtePtr == TM_RTE_NULL_PTR )
        {
            *((ttRteEntryPtr TM_FAR *)leastUsedRtePtrPtr) = rtePtr;
        }
        else
        {
#ifdef TM_USE_IPV6
            tm_kernel_set_critical;
            elapsedMsec = (tt32Bit) tvTime
                - (tt32Bit) rtePtr->rteHSLastReachedTickCount;
            leastUsedElapsedMsec = (tt32Bit) tvTime
                - (tt32Bit) leastUsedRtePtr->rteHSLastReachedTickCount;
            tm_kernel_release_critical;      

            if ( elapsedMsec > leastUsedElapsedMsec)
            {
                *((ttRteEntryPtr TM_FAR *)leastUsedRtePtrPtr) = rtePtr;
            }
#else /* ! TM_USE_IPV6 */
            if (rtePtr->rteTtl < leastUsedRtePtr->rteTtl)
            {
                *((ttRteEntryPtr TM_FAR *)leastUsedRtePtrPtr) = rtePtr;
            }
#endif /* TM_USE_IPV6 */
        }
    }
    return TM_ENOENT;
}

/*
 * Remove any entry using device/multihome index of routing entry voidPtr
 */
static int tfRtRemObsLocalCB (ttRteEntryPtr rtePtr, ttVoidPtr voidPtr)
{
    ttRteEntryPtr    localRtePtr;
#ifdef TM_USE_IPV6
    int              match;
    int              i;
#endif /* TM_USE_IPV6 */

    localRtePtr = (ttRteEntryPtr)voidPtr;
    if (rtePtr != localRtePtr )
    {
        if (    (rtePtr->rteOrigDevPtr == localRtePtr->rteOrigDevPtr)
             && (rtePtr->rteMhomeIndex == localRtePtr->rteMhomeIndex)
             && (tm_16bit_bits_not_set(rtePtr->rteFlags, TM_RTE_LOCAL))
          )
        {
#ifdef TM_USE_IPDUAL
/* Insert Ipv4 dummy default gateway after the current one is removed */
            if (tm_16bit_one_bit_set(rtePtr->rteFlags2, TM_RTE2_IP4_DEF_GW))
            {
                rtePtr->rteFlags2 = (tt16Bit)
                                (rtePtr->rteFlags2 | TM_RTE2_IP4_INS_DEF_GW);
            }
#endif /* TM_USE_IPDUAL */
            tfRtRemoveEntry(rtePtr);
        }
#ifdef TM_USE_IPV6
        else
        {
/*
 * Check for a match corresponding to this call: (void) tfNgAddStaticRoute(
 *                   (ttUserInterface)tm_context(tvLoopbackDevPtr),
 *                   &destAddr,
 *                   TM_6_IP_HOST_PREFIX_LENGTH,
 *                   &gwAddr,
 *                   1 );
 */
             if (    (rtePtr->rteDevPtr == tm_context(tvLoopbackDevPtr))
                  && (rtePtr->rteLeafNode.rtnPrefixLength ==
                                                 TM_6_IP_HOST_PREFIX_LENGTH))
            {
                tm_6_ip_net_match_leaf_prefix(match,
                                              &(localRtePtr->rteLeafNode.rtnLSKey),
                                              &(rtePtr->rteLeafNode.rtnLSKey),
                                              localRtePtr->rteLeafNode.rtnLS6MaskIndex,
                                              localRtePtr->rteLeafNode.rtnLS6MaskPrefix,
                                              i);
                if (match)
                {
                    tfRtRemoveEntry(rtePtr);
                }
            }
        }
#endif /* TM_USE_IPV6 */
    }
/* ask for the next entry. We need to check the whole tree */
    return TM_ENOENT;
}

/*
 * Remove any gateway pointing to the passed cloned entry (voidPtr), but
 * not a default router.
 */
static int tfRtRemoveGwToClonedCB (ttRteEntryPtr rtePtr, ttVoidPtr voidPtr)
{
    ttRteEntryPtr clonedRtePtr;

    clonedRtePtr = (ttRteEntryPtr)voidPtr;
    if (       ( rtePtr != clonedRtePtr )
            && ( rtePtr->rteClonedPtr == clonedRtePtr) )
    {
        if ( tm_16bit_all_bits_set( rtePtr->rteFlags, 
                                    TM_RTE_CLONE | TM_RTE_LOCAL) )
/* 
 * If this is a local entry, then the cloned entry is loop back for 
 * the interface, just reset the back pointer to it.
 */
        {
            rtePtr->rteClonedPtr = TM_RTE_NULL_PTR;
        }
        else
        {
            if (      tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_STATIC)
#ifdef TM_USE_IPV6
                 || (    tm_16bit_one_bit_set(rtePtr->rteFlags,
                                              TM_RTE_INDIRECT)
                      && tm_6_ip_match((  tt6IpAddressPtr)&in6addr_any
                                        , &(rtePtr->rteDest))
                    )
#endif /* TM_USE_IPV6 */
               )
            {
/*
 * Mark the clone entry that the static route/default router points to as
 * invalid
 */
                clonedRtePtr->rteFlags2 |= TM_RTE2_INVALID;
            }
            else
/*
 * Remove the entry only if it is not a static route and not a default
 * router
 */
            {
                tfRtRemoveEntry(rtePtr);
            }
        }
    }
/* Ask for the next entry. We need to check the whole tree */
    return TM_ENOENT;
}

/*
 * Find an entry in the routing table for the destination IP address
 * This function is called after exclusive access to the routing
 * table has been obtained.
 * Based on the passed destination IP address passed, we scan the routing
 * table to find the best match.
 * 1) We first go down the tree, and when we reach a leaf check  for a
 * host/network match.
 * 2) If this fails we check for a network match on the duplicate keys of
 * the leaf
 * 3) If this fails too, we backtrack up the tree until we find a network
 * match.
 * 4) We return a pointer to a tree leaf on success, null pointer
 * otherwise. Note that since a tree leaf is the first structure of a
 * routing entry, the tree leaf pointer is also a routing entry pointer.
 * Parameter:
 *   Destination Ip address for which we want to obtain a routing table
 *   entry pointer.
 * Return values:
 *   Routing entry pointer on success
 *   Null routing entry pointer otherwise.
 */
static ttRteEntryPtr tfRtFind (
#ifdef TM_USE_STRONG_ESL
   ttDeviceEntryPtr     devPtr,
#endif /* TM_USE_STRONG_ESL */
#ifdef TM_USE_IPV6
    ttIpAddressPtr      dest6IpAddrPtr
#else /* ! TM_USE_IPV6 */
    tt4IpAddress        destIpAddr
#endif /* ! TM_USE_IPV6 */
    )
{
    ttRtRadixNodePtr      nodePtr;
    ttRtRadixNodePtr      leafPtr;
    ttRteEntryPtr         foundRtePtr;
#ifdef TM_USE_STRONG_ESL
    ttRteEntryPtr         tempRtePtr;
#endif /* TM_USE_STRONG_ESL */
#ifdef TM_USE_IPV6
    int                   match;
    int                   i;
#endif /* TM_USE_IPV6 */
#ifdef TM_TRY_BIT_OFFSET

#ifdef TM_USE_IPV6
    int                   longIndex;
#endif /* TM_USE_IPV6 */
#ifndef TM_LITTLE_ENDIAN
    tt4IpAddress          bittest;
#endif /* TM_LITTLE_ENDIAN */
    tt16Bit               bits;

#endif /* TM_TRY_BIT_OFFSET */
 
    foundRtePtr = (ttRteEntryPtr)0; /* assume not found */
/* Start at head of tree */
    nodePtr = tm_context(tvRtTreeHeadPtr);
/* Go down the tree until we reach a leaf. nodePtr changed to a leaf */
#ifdef TM_USE_IPV6
    tm_rt_tree_search(nodePtr, dest6IpAddrPtr);
#else  /* !TM_USE_IPV6 */
    tm_rt_tree_search(nodePtr, destIpAddr);
#endif /* !TM_USE_IPV6 */
#ifdef TM_ERROR_CHECKING
    tm_rt_check_null_leaf(nodePtr); /* check that the leaf is non null */
#endif /* TM_ERROR_CHECKING */
/*
 * We have now reached a leaf. Check for host or network match. Note that
 * we check for network match on the first leaf of the chain of duplicate
 * keys, but this would yield the host match as well since the most
 * specific route (host) would be first and would match that way (mask
 * all one's).
 */
#ifdef TM_USE_IPV6
    tm_6_ip_net_match_leaf_prefix(match, &(nodePtr->rtnLSKey), dest6IpAddrPtr,
                                  nodePtr->rtnLS6MaskIndex, 
                                  nodePtr->rtnLS6MaskPrefix,
                                  i);
    if (match)
#else  /* !TM_USE_IPV6 */
    if (tm_ip_net_match(nodePtr->rtnLSKey, destIpAddr, nodePtr->rtnLSMask))
#endif /* !TM_USE_IPV6 */
    {
        if (tm_16bit_one_bit_set(nodePtr->rtnFlags, TM_RTN_ROOT))
        {
/*
 * Were we asked for the default entry? If so, make sure we point to it.
 * (If there is no default entry, next duplicate is NULL, and we will
 * return a null pointer (no match).) Note that this check will also
 * return a null pointer, in case the user had requested a route to
 * limited broadcast address.
 */
             nodePtr = tm_rt_next_dup(nodePtr);
#ifdef TM_USE_STRONG_ESL
/*
 * This is an end point of the tree (either left or right).
 */
            if (devPtr != (ttDeviceEntryPtr)0)
/* 
 * If we need to match on the device too (non null device):
 * We need to go through all the leaves right here, and not below as
 * backtracking done below will lead us nowhere.
 */
            {
                while (nodePtr != (ttRtRadixNodePtr)0)
                {
                    foundRtePtr = tm_rt_leaf_to_rte(nodePtr);
                    if (foundRtePtr->rteOrigDevPtr == devPtr)
                    {
                        goto rtFindExit; /* exact match */
                    }
                    nodePtr = tm_rt_next_dup(nodePtr);
                }
            }
#endif /* TM_USE_STRONG_ESL */
        }
        foundRtePtr = tm_rt_leaf_to_rte(nodePtr);
#ifdef TM_USE_STRONG_ESL
/*
 * If we have found an exact match, need to make sure that we match
 * on the device
 */
        if (nodePtr != (ttRtRadixNodePtr)0)
        {
            if (    (devPtr == (ttDeviceEntryPtr)0)
                 || (foundRtePtr->rteOrigDevPtr == devPtr) )
            {
                goto rtFindExit; /* exact match */
            }
            else
            {
/* We will need to go through the leaves below */
                foundRtePtr = (ttRteEntryPtr)0; /* reset */
            }
        }
        else
        {
            goto rtFindExit; /* null default gateway or brodcast addr match */
        }
#else /* TM_USE_STRONG_ESL */
        goto rtFindExit; /* exact match */
#endif /* TM_USE_STRONG_ESL */
    }
/*
 * No host match. Let us check for other network matches. First check through
 * all duplicate keys of the leaf we are at (network routes).
 * STRONG_ESL: also check for host match for the device in duplicate keys.
 */
#ifdef TM_TRY_BIT_OFFSET
/* a bit more work here.
 *
 * bittest temp var, bits var arg. Other parameters unchanged. Result
 * stored in bits.
 */
#ifdef TM_USE_IPV6
    for (longIndex = 0; longIndex <= 3, longIndex++)
    {
        if (tm_ip_not_match(nodePtr->rtnLSKey.s6LAddr[longIndex], 
                            dest6IpAddrPtr->s6LAddr[longIndex]) )
        {
            break;
        }
    }
    tm_assert(tfRtFind, (longIndex <= 3));
#ifdef TM_LITTLE_ENDIAN
    bits = tfRtBitOffset(nodePtr->rtnLSKey.s6LAddr[longIndex],
                         dest6IpAddrPtr->s6LAddr[longIndex], &bittest);
#else /* !TM_LITTLE_ENDIAN */
/* 
 * rtnLSkey.s6Laddr[i] value arg, dest6IpAddrPtr->s6LAddr[i] value arg,
 * bittest temp var, bits var arg
 */
   tm_ip_bit_offset(nodePtr->rtnLSKey.s6LAddr[longIndex], 
                    dest6IpAddrPtr->s6LAddr[longIndex], bittest, bits);
#endif /* TM_LITTLE_ENDIAN */
#else /* !TM_USE_IPV6 */
#ifdef TM_LITTLE_ENDIAN
    tm_ip_bit_offset(nodePtr->rtnLSkey, destIpAddr, bits);
#else /* TM_BIG_ENDIAN */
    tm_ip_bit_offset(nodePtr->rtnLSkey, destIpAddr, bittest, bits);
#endif /* TM_BIG_ENDIAN */
#endif /* !TM_USE_IPV6 */
#endif /* TM_TRY_BIT_OFFSET */
/* save nodePtr, use leafPtr to go down the list of duplicates */
    leafPtr = tm_rt_next_dup(nodePtr);
    while (leafPtr != TM_RADIX_NULL_PTR)
    {
#ifdef TM_USE_STRONG_ESL
        tempRtePtr = tm_rt_leaf_to_rte(leafPtr);
        if (    (devPtr == (ttDeviceEntryPtr)0)
             || (tempRtePtr->rteOrigDevPtr == devPtr) )
#endif /* TM_USE_STRONG_ESL */
        {
/* Network portion of addresses match ? */
#ifdef TM_TRY_BIT_OFFSET
/* less work here */
/* matching bits cover netmask */
            if (bits >= tm_rt_mask_len(leafPtr))
#else /* TM_TRY_BIT_OFFSET */
#ifdef TM_USE_IPV6
            tm_6_ip_net_match_leaf_prefix(match, &(leafPtr->rtnLSKey),
                                          dest6IpAddrPtr,
                                          leafPtr->rtnLS6MaskIndex,
                                          leafPtr->rtnLS6MaskPrefix,
                                          i);
            if (match)
#else /* !TM_USE_IPV6 */
            if ( tm_ip_net_match( leafPtr->rtnLSKey, destIpAddr,
                                  leafPtr->rtnLSMask ) )
#endif /* !TM_USE_IPV6 */
#endif /* TM_TRY_BIT_OFFSET */
            {
                foundRtePtr = tm_rt_leaf_to_rte(leafPtr);
                goto rtFindExit;
            }
        }
        leafPtr = tm_rt_next_dup(leafPtr); /* get duplicate key leaf */
    }
/*
 * No match yet. Backup up the tree for network matches until we
 * find a network match or we reach the root
 */
    do
    {
/* Back up the tree. nodePtr moves up the tree */
       nodePtr = tm_rt_parent(nodePtr);
/* Check list of non host route entries for the subtree */
/* leafPtr is used to go down the linked list of network leaves */
       leafPtr = tm_rt_next_net(nodePtr);
       while (leafPtr != TM_RADIX_NULL_PTR)
       {
#ifdef TM_USE_STRONG_ESL
            tempRtePtr = tm_rt_leaf_to_rte(leafPtr);
            if (    (devPtr == (ttDeviceEntryPtr)0)
                 || (tempRtePtr->rteOrigDevPtr == devPtr) )
#endif /* TM_USE_STRONG_ESL */
            {
/* Check for a network match */
#ifdef TM_TRY_BIT_OFFSET
/* matching bits cover netmask (less work here) */
                if (bits >= tm_rt_mask_len(leafPtr))
#else /* TM_TRY_BIT_OFFSET */
#ifdef TM_USE_IPV6
                tm_6_ip_net_match_leaf_prefix(match, &(leafPtr->rtnLSKey),
                                          dest6IpAddrPtr,
                                          leafPtr->rtnLS6MaskIndex,
                                          leafPtr->rtnLS6MaskPrefix,
                                          i);
                if (match)
#else /* !TM_USE_IPV6 */
                if ( tm_ip_net_match( destIpAddr, leafPtr->rtnLSKey,
                                      leafPtr->rtnLSMask ) )
#endif /* !TM_USE_IPV6 */
#endif /* TM_TRY_BIT_OFFSET */
                {
                    foundRtePtr = tm_rt_leaf_to_rte(leafPtr);
                    goto rtFindExit;
                }
            }
/* next non host leaf to check for this subtree */
            leafPtr = tm_rt_next_net(leafPtr);
        }
    }
    while (nodePtr != tm_context(tvRtTreeHeadPtr));
rtFindExit:
    return foundRtePtr;
}

/*
 * Called by tf4RtAddRoute(), tf6RtAddRoute(), tfRtGet()/tfRtClone(),
 * tfRtInit(), tf4RtReplaceRoute to insert a routing entry into the
 * Patricia tree.
 * 1) Initialize the routing entry with passed parameters
 * 2) If there is already an entry for ipAddr/Netmask, if local remove it
 *    otherwise return error.
 * 3) Add the entry to the tree
 * 4) If indirect entry, add ARP router entry for the gateway and point to it.
 *
 * Parameters
 * rtePtr: new routing entry to be initialized and inserted.
 * devPtr: device for the route 
 * ipAddr: Destination Ip address 
 * netMask: destination network address 
 * mhomeIndex: multihome index for local routes
 * gateway: Gateway IP address for non local routes, interface IP address
 *          for local routes.
 * hops: number of hops to destination (0 for local route)
 * ttl: Time to live for the route entry
 * flags: flag to indicate creator local/static/redirect/RIP, host route etc.
 *
 * Return value:
 * TM_ENOERROR          success
 * TM_EALREADY          route already in the tree
 * TM_EHOSTUNREACH      destination unreachable
 */

static int tfRtAddEntry (
    ttRtRadixNodePtr        nodePtr, /* Node to start the search at */
    ttRteEntryPtr           rtePtr, /* new routing entry */
    ttDeviceEntryPtr        devPtr, /* device for the route */
#ifdef TM_USE_IPV6
    int                     prefixLength,
#else /* ! TM_USE_IPV6 */
    tt4IpAddress            ipAddress,
    tt4IpAddress            netMask,
#endif /* ! TM_USE_IPV6 */
/* multihome index for local routes */
    tt16Bit                 mhomeIndex,
    int                     hops, /* metrics for non local routes */
    tt32Bit                 ttl,
/* flag to indicate creator local/static/redirect/RIP */
    tt16Bit                 flags )
{
        ttRtRadixNodePtr    leafPtr; /* leaf node to be inserted */
        ttRtRadixNodePtr    intNodePtr; /* internal node to be inserted */
        ttRtRadixNodePtr    parentPtr; /* temporary */
        ttRtRadixNodePtr    nextPtr; /* temporary */
        ttRtRadixNodePtr    nextTempPtr; /* temporary */
        ttRteEntryPtr       oRtePtr; /* temporary */
        ttGenericUnion      timerParm1;
#ifdef TM_USE_IPV6
        ttIpAddressPtr      ip6AddrPtr;
#endif /* TM_USE_IPV6 */
auto    tt4IpAddress        bittest; /* temporary */
#ifdef TM_USE_IPV6
        int                 longIndex;
#endif /* TM_USE_IPV6 */
auto    int                 errorCode;
#ifdef TM_USE_IPV6
        tt16Bit             savedMhomeIndex;
#endif /* TM_USE_IPV6 */
        tt16Bit             af;
        tt16Bit             keyBits; /* bitoffset for internal node */
        tt16Bit             maskBits; /* bitoffset for leaf */
        tt16Bit             maskLength; /* temporary */
#if (!defined(TM_LITTLE_ENDIAN)) || defined(TM_USE_IPV6)
        tt16Bit             longBitOffset; /* bitoffset for internal node */
#endif /* (!defined(TM_LITTLE_ENDIAN)) || defined(TM_USE_IPV6) */
        tt8Bit              removeEntry;
        tt8Bit              arpClone; /* adding an ARP clone with dup key? */
#ifdef TM_USE_IPV6
        tt8Bit              cacheTemp;
        tt8Bit              newTemp;
#endif /* TM_USE_IPV6 */

#ifdef TM_ERROR_CHECKING
#ifdef TM_USE_NETSTAT
#ifdef TM_DEBUG_LOGGING
    tfxLogRteTable("tfRtAddEntry");
    tfxLogArpTable("tfRtAddEntry");
#endif /* TM_DEBUG_LOGGING */
#endif /*TM_USE_NETSTAT*/
#endif /* TM_ERROR_CHECKING */

    errorCode = TM_ENOERROR;
    arpClone = TM_8BIT_NO;
    tm_ip_copy(TM_IP_ZERO, bittest);
/* store the device pointer in the routing entry early, just in case we fail
   to add the routing entry, so that tfRtCacheUnlockNClean can reference it */
    rtePtr->rteDevPtr = devPtr;
    rtePtr->rteOrigDevPtr = devPtr;

/*
 * Compute the key IP address. We need to mask the given IP address
 * with the network mask for local addition (tfConfigInterface), and also
 * for static route, in case the user did not do it properly.
 */
#ifdef TM_USE_IPV6
    ip6AddrPtr = &(rtePtr->rteDest);
    savedMhomeIndex = mhomeIndex;
#ifdef TM_USE_IPV4
    if (!IN6_IS_ADDR_V4MAPPED(ip6AddrPtr))
    {
/* special hack for dual IP layer mode:
   offset the IPv6 multi-home index in local route entries so that it doesn't
   conflict with IPv4 route entries. */
        if (mhomeIndex < TM_6_RT_MHOME_INDEX_OFFSET)
        {
            mhomeIndex += TM_6_RT_MHOME_INDEX_OFFSET;
        }
    }
#endif /* TM_USE_IPV4 */
/* 
 * Move applying the net prefix after the "special hack", otherwise the
 * "special hack" will hack IPv4 addresses if the prefix is smaller than 96!!
 */
    if (prefixLength < TM_6_IP_HOST_PREFIX_LENGTH)
    {
        tm_6_ip_net_prefix(ip6AddrPtr, prefixLength, longIndex);
    }
/*
 * Check whether this network IP address/network mask is already in the
 * routing table
 */
    oRtePtr = tfRtFindIpAddrIpNet(
#ifdef TM_USE_STRONG_ESL
                                  devPtr,
#endif /* TM_USE_STRONG_ESL */
                                  ip6AddrPtr, prefixLength);
#else /* ! TM_USE_IPV6 */
    tm_ip_copy(tm_ip_net(ipAddress, netMask), ipAddress);
    oRtePtr = tfRtFindIpAddrIpNet(
#ifdef TM_USE_STRONG_ESL
                                   devPtr,
#endif /* TM_USE_STRONG_ESL */
                                   ipAddress, netMask);
#endif /* ! TM_USE_IPV6 */
    if (oRtePtr != TM_RTE_NULL_PTR)
    {
/* Remove any existing routing entry for our local route */
        if (tm_16bit_one_bit_set(flags, TM_RTE_LOCAL))
        {
            removeEntry = TM_8BIT_NO;
            if (    (oRtePtr->rteDevPtr != devPtr)
                 || tm_16bit_bits_not_set(oRtePtr->rteFlags, TM_RTE_LOCAL) )
/* non identical devices, or non local route */
            {
                removeEntry = TM_8BIT_YES;
            }
#ifdef TM_USE_IPV6
            else
/* Two identical local prefixes on the same device */
            {
                if (savedMhomeIndex != mhomeIndex)
/* IPv6 addresses */
                {
                    if (    (oRtePtr->rteOrigMhomeIndex >=
                                            TM_6_RT_START_PREFIX_MHOME_INDEX)
                         && (savedMhomeIndex < TM_6_MAX_MHOME)
                         && (savedMhomeIndex >= TM_MAX_IPS_PER_IF) )
                    {
                        removeEntry = TM_8BIT_YES;
                    }
                    else
                    {
                        if (oRtePtr->rteOrigMhomeIndex < TM_6_MAX_MHOME)
                        {
                            cacheTemp = (tt8Bit)
                    (devPtr->dev6IpAddrFlagsArray[oRtePtr->rteOrigMhomeIndex]
                                                & TM_6_DEV_IPADDR_TEMPORARY);
                        }
                        else
                        {
                            cacheTemp = 0;
                        }
                        if (savedMhomeIndex < TM_6_MAX_MHOME)
                        {
                            newTemp = (tt8Bit)
                            (devPtr->dev6IpAddrFlagsArray[savedMhomeIndex]
                                                & TM_6_DEV_IPADDR_TEMPORARY);
                        }
                        else
                        {
                            newTemp = 0;
                        }
                        if (cacheTemp && !newTemp)
                        {
                            removeEntry = TM_8BIT_YES;
                        }
                    }
                    if (removeEntry)
                    {
                        if (oRtePtr->rteTtl != TM_RTE_INF)
                        {
/* Update the ttl */
                            ttl = oRtePtr->rteTtl;
                        }
                    }
                }
            }
#endif /* TM_USE_IPV6 */
            if (removeEntry)
            {
                tfRtRemoveEntry(oRtePtr);
            }
            else
            {
/*
 * If the new local route entry is on the same interface as the old local
 * route entry, and the old local route entry is not temporary, or both
 * local routes are temporary, then don't remove the old one
 */
                errorCode = TM_EALREADY;
                goto rtAddEntryExit;
            }
        }
        else
        {
/* If creator is same, just update the ttl */
            if ((tt16Bit)(oRtePtr->rteFlags & TM_RTE_CREATE_MASK) == flags)
            {
                oRtePtr->rteTtl = ttl;
            }
            if (    tm_16bit_all_bits_set(oRtePtr->rteFlags,
                                         TM_RTE_INDIRECT | TM_RTE_CLONE)
                 && tm_16bit_all_bits_set(flags,
                                          TM_RTE_CLONED | TM_RTE_ARP))
            {
/* Adding an ARP clone with same key and netmask as static route */
                arpClone = TM_8BIT_YES;
            }
            else
            {
                if (    tm_16bit_all_bits_set(flags,
                                              TM_RTE_INDIRECT | TM_RTE_STATIC)
                     && tm_16bit_all_bits_set(oRtePtr->rteFlags,
                                              TM_RTE_CLONED | TM_RTE_ARP))
                {
                    af = AF_INET;
#ifdef TM_USE_IPV6
                    if (oRtePtr->rte6HSNudState != TM_6_NUD_IPV4_STATE)
                    {
                        af = AF_INET6;
                    }
#endif /* TM_USE_IPV6 */
                    tfRtRemoveHostEntry(oRtePtr, 1, af);
                }
                else
                {
/* Do not allow the user to add a static route without removing it first */
                    errorCode = TM_EALREADY;
                    goto rtAddEntryExit;
                }
            }
/* Allow cloning an ARP entry at the same IP address */
        }
    }
    if (    tm_16bit_one_bit_set( flags, TM_RTE_DYNAMIC)
/* RIP, Router Discovery, or REDIRECT gateway entry */
         && (tm_context(tvRtEntriesCount) >= tm_context(tvRtMaxEntries)) )
    {
#ifdef TM_SNMP_MIB
/*
 * Number of routing entries which were chosen to be
 * discarded even though they are valid.
 */
#ifdef TM_USE_IPV4
        tm_context(tvIpData).ipRoutingDiscards++;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_NG_MIB2
        tm_context(tvIpForwardData).inetCidrRouteDiscards++;
#endif /* TM_USE_NG_MIB2 */

#endif /* TM_SNMP_MIB */
        errorCode = TM_ENOBUFS;
        goto rtAddEntryExit;
    }
/* Initialize the routing entry with passed parameters */
    rtePtr->rteMhomeIndex = mhomeIndex;
#ifdef TM_USE_IPV6
/* Use mhome index without IPv6 offset */
    rtePtr->rteOrigMhomeIndex = savedMhomeIndex;
#else /* !TM_USE_IPV6 */
    rtePtr->rteOrigMhomeIndex = mhomeIndex;
    tm_ip_copy(ipAddress, rtePtr->rteDest);
#endif /* !TM_USE_IPV6 */
/*
 * No need to add to device owner count, since device will delete
 * routing entry first.
 */
    rtePtr->rteHops = (tt32Bit)hops;
    rtePtr->rteTtl = ttl;
/* for the time being, we only use clones with local routes */
    if (tm_16bit_one_bit_set(flags, TM_RTE_CLONE))
    {
#ifdef TM_USE_IPV6
        rtePtr->rteClonePrefixLength = TM_6_IP_HOST_PREFIX_LENGTH;
#else /* !TM_USE_IPV6 */
        rtePtr->rteCloneMask = TM_IP_HOST_MASK;
#endif /* !TM_USE_IPV6 */
    }
/* Pointers to leaf node and internal node of routing entry */
    leafPtr = &rtePtr->rteLeafNode;
    intNodePtr = &rtePtr->rteInternalNode;
#ifdef TM_USE_IPV6
    tm_6_ip_copy_dststruct(ip6AddrPtr, leafPtr->rtnLSKey);
    maskBits = (tt16Bit)prefixLength;
    tm_rt_mask_len(leafPtr) = maskBits; /* length of netmask */
    tm_6_ip_prefix_to_long_index(prefixLength, longIndex);
    leafPtr->rtnLS6MaskIndex = (tt16Bit)longIndex;
    leafPtr->rtnLS6MaskPrefix = (tt16Bit)
                    tm_6_ip_prefix_to_4_prefix(prefixLength, longIndex); 
#else /* !TM_USE_IPV6 */
    tm_ip_copy(ipAddress, leafPtr->rtnLSKey);
    tm_ip_copy(netMask, leafPtr->rtnLSMask);
/* Length of netmask: Number of leading contiguous 1's.  */
    maskBits = (tt16Bit)tf4NetmaskToPrefixLen(netMask);
    if (tm_ip_not_match(tvRt32ContiguousBitsPtr[maskBits], netMask))
    {
        errorCode = TM_EINVAL;
        goto rtAddEntryExit;
    }
/* Leaf node initialization. We store the key and network mask */
    tm_rt_mask_len(leafPtr) = maskBits; /* length of netmask */
#endif /* !TM_USE_IPV6 */
/* End of routing entry Initialization */
/* Add a brand new entry */
    {
/* start the search at nodePtr. Key is ipAddress. Network mask is netMask */
/* search down the tree until we find a leaf */
#ifdef TM_USE_IPV6
        tm_rt_tree_search(nodePtr, ip6AddrPtr);
#else /* TM_USE_IPV6 */
        tm_rt_tree_search(nodePtr, ipAddress);
#endif /* !TM_USE_IPV6 */
#ifdef TM_ERROR_CHECKING
        tm_rt_check_null_leaf(nodePtr);
#endif /* TM_ERROR_CHECKING */
/* check whether we have duplicate key */
#ifdef TM_USE_IPV6
        if (tm_6_ip_match(&(nodePtr->rtnLSKey), ip6AddrPtr))
#else /* !TM_USE_IPV6 */
        if (tm_ip_match(nodePtr->rtnLSKey, ipAddress))
#endif /* !TM_USE_IPV6 */
        {
#ifdef TM_USE_STRONG_ESL
            oRtePtr = tm_rt_leaf_to_rte(nodePtr);
#endif /* TM_USE_STRONG_ESL */
/*
 * Duplicate key. insert LeafPtr in list of duplicate keys, sorted
 * by decreasing length of masks. If entry is already in, return error.
 * Strong End System Model adition:
 * For same mask, if device index is different, insert by increasing number
 * of hops or increasing device index if number of hops same.
 * Insert ARP clone entry BEFORE routing entry at same IP address, so that it
 * can be found first by tfRtGet() since rteClonedPtr points to the local
 * route not the ARP entry.
 */
            if (    (tm_rt_mask_len(nodePtr) > maskBits)
                 || (tm_16bit_one_bit_set(nodePtr->rtnFlags, TM_RTN_ROOT))
#ifdef TM_USE_STRONG_ESL
/* check for different device index, if mask len is same */
                 || (    (tm_rt_same_mask(nodePtr, maskBits))
                      && (devPtr->devIndex != oRtePtr->rteOrigDevPtr->devIndex)
                      && (    ((tt32Bit)hops > oRtePtr->rteHops)
                           || (    ((tt32Bit)hops == oRtePtr->rteHops)
                                && (devPtr->devIndex
                                        > oRtePtr->rteOrigDevPtr->devIndex) )
                         )
                    )
#endif /* TM_USE_STRONG_ESL */
               )
            {
/*
 * Entry has a shorter mask or is default
 * Strong End System Model adition:
 * or Entry has same mask, and has bigger number of hops, or bigger
 * device index.
 * insert inside the list of duplicates after the first one.
 */
/*
 * keep nodePtr as pointer to head of duplicate list. Use parentPtr, and
 * nextPtr to move pointers around.
 */
                parentPtr = nodePtr;
                nextPtr = tm_rt_next_dup(parentPtr);
                while (nextPtr != TM_RADIX_NULL_PTR)
                {
/* Insert leaf with the longest masks first */
                    if ( tm_rt_mask_len(nextPtr) < maskBits )
                    {
/* insert leafPtr before nextPtr  (next previous is leafPtr) */
                        tm_rt_prev_dup(nextPtr) = leafPtr;
                        break;
                    }
                    if ( tm_rt_same_mask(nextPtr, maskBits) )
                    {
#ifdef TM_USE_STRONG_ESL
                        oRtePtr = tm_rt_leaf_to_rte(nextPtr);
/* Check for different device index, if mask len is same */
                        if (devPtr->devIndex !=
                                        oRtePtr->rteOrigDevPtr->devIndex)
                        {
                            if (   ((tt32Bit)hops < oRtePtr->rteHops)
                                || (    ((tt32Bit)hops == oRtePtr->rteHops)
                                     && (devPtr->devIndex
                                          < oRtePtr->rteOrigDevPtr->devIndex) )
                               )
                            {
/* insert leafPtr before nextPtr (next previous is leafPtr) */
                                tm_rt_prev_dup(nextPtr) = leafPtr;
                                break;
                           }
                        }
                        else
/* 
 * If routing entry already in (duplicate key with same mask), and device
 * index is the same return error, unless we have an ARP entry at the
 * same IP address as the route.
 */
#endif /* TM_USE_STRONG_ESL */
                        {
                            if (arpClone)
                            {
                                tm_rt_prev_dup(nextPtr) = leafPtr;
                                break;
                            }
                            else
                            {
/* 
 * If routing entry already in (duplicate key with same mask), return error.
 */
                                errorCode = TM_EALREADY;
                                goto rtAddEntryExit;
                            }
                        }
                    }
/* Since this is not a circular list we need to keep track of the parent */
                    parentPtr = nextPtr;
                    nextPtr = tm_rt_next_dup(parentPtr);
                }
/*
 * insert leafPtr between parentPtr and nextPtr (next three macros)
 */
                tm_rt_next_dup(leafPtr) = nextPtr; /* leaf next is nextPtr */
/* leaf previous is parentPtr */
                tm_rt_prev_dup(leafPtr) = parentPtr;
/* parent next is leafPtr */
                tm_rt_next_dup(parentPtr) = leafPtr;
/* nodePtr point to head of list of duplicates */
            }
            else
            { 
/*
 * Leaf has a longer mask than or same mask as the current head and is not
 * root
 */
                if (tm_rt_same_mask(nodePtr, maskBits))
/* If same mask, and not an ARP clone and same index, fail */
                {
                    if (    (!arpClone)
#ifdef TM_USE_STRONG_ESL
                         && (devPtr->devIndex 
                                 == oRtePtr->rteOrigDevPtr->devIndex)
#endif /* TM_USE_STRONG_ESL */
                       )
                    {
                        errorCode = TM_EALREADY;
                        goto rtAddEntryExit;
                    }
                }
/*
 * insert leafPtr at head of list of duplicates, before the current head
 * (nodePtr)
 */
                parentPtr = tm_rt_parent(nodePtr);
#ifdef TM_USE_IPV6
                tm_rt_set_child_parent_key(parentPtr, leafPtr, ip6AddrPtr);
#else /* !TM_USE_IPV6 */
                tm_rt_set_child_parent_key(parentPtr, leafPtr, ipAddress);
#endif /* !TM_USE_IPV6 */
                tm_rt_prev_dup(nodePtr) = leafPtr;
                tm_rt_next_dup(leafPtr) = nodePtr;
/* nodePtr points to current head's internal node */
                nodePtr = tm_rt_leaf_to_int_node(nodePtr);
#ifdef TM_ERROR_CHECKING
/* Make sure there is a grandparent */
                if ( tm_16bit_one_bit_set(parentPtr->rtnFlags, TM_RTN_ROOT) )
                {
                    tfKernelError("tfRtAddEntry",
                                  "Duplicate head and no grandfather");
                    tfRtTreeUnlock();
                    tm_thread_stop;
                }
/* Verify that current head internal node is in the tree */
                if ( tm_16bit_bits_not_set( nodePtr->rtnFlags,
                                            TM_RTN_IN) )
                {
                    tfKernelError("tfRtAddEntry",
                                  "Duplicate head internal node not in tree");
                    tfRtTreeUnlock();
                    tm_thread_stop;
                }
#endif /* TM_ERROR_CHECKING */
/*
 * tfRtCopyNode will copy internal node of current head to internal node
 * of new head, and will reset the TM_RTN_IN flag in internal node of
 * current head. (That way only the head of a list of duplicates will
 * have its internal node in the tree. This is necessary so that we
 * are able to swap node, if we need to, when we remove the duplicate head
 * routing entry from the tree (see tfRtRemoveEntry() for algorithm).)
 */
                tfRtCopyNode(nodePtr, intNodePtr);
                nodePtr = leafPtr; /* point to head of list of duplicates */
            }
        }
        else
        {
/*
 * Not a duplicate key.
 * Find the number of leading key bits that match
 */
#ifdef TM_USE_IPV6
            for (longIndex = 0; longIndex <= 3; longIndex++)
            {
                if (tm_ip_not_match(nodePtr->rtnLSKey.s6LAddr[longIndex], 
                                    ip6AddrPtr->s6LAddr[longIndex]) )
                {
                    break;
                }
            }
            tm_assert(tfRtAddEntry, (longIndex <= 3));
#ifdef TM_LITTLE_ENDIAN
            keyBits = tfRtBitOffset(nodePtr->rtnLSKey.s6LAddr[longIndex], 
                                    ip6AddrPtr->s6LAddr[longIndex], &bittest);
#else /* !TM_LITTLE_ENDIAN */
/* 
 * rtnLSkey.s6Laddr[i] value arg, ipAddPtr->s6LAddr[i] value arg, 
 * bittest temp var, keyBits var arg 
 */
           tm_ip_bit_offset(nodePtr->rtnLSKey.s6LAddr[longIndex], 
                            ip6AddrPtr->s6LAddr[longIndex], bittest, keyBits);
#endif /* TM_LITTLE_ENDIAN */
           longBitOffset = keyBits; /* Bit offset in one 32-bit word */
/* Bit offset over the entire IP address: */
           keyBits = (tt16Bit)(keyBits + (tt16Bit)(longIndex << 5));
           intNodePtr->rtnNS6LIndex = (tt16Bit)longIndex;
           intNodePtr->rtnNS6LBitOffset = longBitOffset;
#else /* !TM_USE_IPV6 */
#ifdef TM_LITTLE_ENDIAN
            keyBits = tfRtBitOffset(nodePtr->rtnLSKey, ipAddress, &bittest);
#else /* !TM_LITTLE_ENDIAN */
/* rtnLSkey value arg, ipAddress value arg, bittest temp var, keyBits var arg */
            tm_ip_bit_offset(nodePtr->rtnLSKey, ipAddress, bittest, keyBits);
            longBitOffset = keyBits; /* Bit offset in one 32-bit word */
#endif /* TM_LITTLE_ENDIAN */
#endif /* !TM_USE_IPV6 */
#ifdef TM_ERROR_CHECKING
            if (    (keyBits > 
                        tm_bytes_to_bits(tm_byte_count(sizeof(ttIpAddress))))
                 || (keyBits == 0))
            {
                tfKernelError("tfRtAddEntry", "Corrupted Patricia Tree");
                tfRtTreeUnlock();
                tm_thread_stop;
            }
#endif /* TM_ERROR_CHECKING */
/*
 * IntNodePtr internal node to insert. Set its bitOffset (used for addition
 * decisions) and BitMask (used for left/right decision when we search the
 * tree down).
 */
            /* Offset to first bit that differs */
            tm_rt_matching_bits(intNodePtr) = keyBits;
            /* bitmask of bit to be tested */
#ifdef TM_LITTLE_ENDIAN
            tm_ip_copy(bittest, intNodePtr->rtnNSBitMask);
#else /* !TM_LITTLE_ENDIAN */
            tm_ip_copy(tm_rtn_bit_mask(longBitOffset), 
                       intNodePtr->rtnNSBitMask);
#endif /* TM_LITTLE_ENDIAN */
/*
 * Find insertion point in the tree for intNodePtr. We know that one
 * of its children will be leafPtr. We need to find intNodePtr's parent
 * and its other child. We walk up the tree from the reached leaf (nodePtr),
 * using nodePtr and parentPtr to climb up the tree  until we find a parent/
 * child pair that will be intNodePtr's new parent (parentPtr)
 * and intNodePtr's new other child (nodePtr)). Our new parent should be the
 * first node that we find on  our way up that has a smaller bit offset than
 * the entry that we want to insert. The new child is the current child of
 * the new parent in the branch we climbed up.
 * nodePtr var arg, parentPtr non initialized var arg, keyBits value arg
 */
            tm_rt_tree_find_insert(nodePtr, parentPtr, keyBits);
/* Check whether we are not inserting a duplicate node */
#ifdef TM_ERROR_CHECKING
            tm_rt_check_dup_nodes(nodePtr, keyBits);
#endif /* TM_ERROR_CHECKING */

/*
 * Insertion point: parentPtr is parent, nodePtr will be one of the children
 * of IntNodePtr. Make intNodePtr the child of this parent 
 */
#ifdef TM_USE_IPV6
            tm_rt_set_child_parent_key(parentPtr, intNodePtr, ip6AddrPtr);
/* set children of intNodePtr (new entry leaf leafPtr, and nodePtr) */
            tm_rt_set_children_parent(intNodePtr, leafPtr, ip6AddrPtr,
                                      nodePtr);
#else /* !TM_USE_IPV6 */
            tm_rt_set_child_parent_key(parentPtr, intNodePtr, ipAddress);
/* set children of intNodePtr (new entry leaf leafPtr, and nodePtr) */
            tm_rt_set_children_parent(intNodePtr, leafPtr, ipAddress,
                                      nodePtr);
#endif /* !TM_USE_IPV6 */
            intNodePtr->rtnFlags = TM_RTN_IN|TM_RTN_NODE;
            if ( tm_16bit_one_bit_set(nodePtr->rtnFlags, TM_RTN_NODE) )
            {
/*
 * If intNodePtr's other child (nodePtr) is a node,
 * move network entries from nodePtr network list whose mask lengths are <=
 * intNodePtr->rtnBitOffset (keyBits) to intNodePtr network list.
 * nodePtr walks the list of network entries of nodePtr.
 */
                nodePtr = tm_rt_next_net(nodePtr);
/*
 * Insert mask in parent node. parentPtr walks the list of network entries
 * for intNodePtr
 */
                parentPtr = intNodePtr;
                while (nodePtr != TM_RADIX_NULL_PTR)
                {
                    nextTempPtr = tm_rt_next_net(nodePtr);
                    maskLength = tm_rt_mask_len(nodePtr);
                    if ( maskLength <= keyBits )
                    {
/* remove network entry from current node. nodePtr value arg */
                        tm_rt_remove_net(nodePtr);
                        tfRtInsertNet(parentPtr, nodePtr, (int) maskLength);
/*
 * nodePtr has been inserted in intNodePtr list, start next insertion at
 * current nodePtr.
 */
                        parentPtr = nodePtr;
                    }
                    nodePtr = nextTempPtr;
                }
            }
            else
            {
                do
                {
                    if (   tm_16bit_one_bit_set(nodePtr->rtnFlags,
                                                TM_RTN_NETWORK)
                        && tm_16bit_bits_not_set( nodePtr->rtnFlags,
                                                  TM_RTN_LIST_NETWORK) )
                    {
/*
 * If intNodePtr's other child (nodePtr) is a leaf routing network entry
 * and if it is not already inserted in a tree node's network list,
 * insert it in intNodePtr network list, if its mask length is smaller
 * or equal to the bit offset for intNodePtr.
 */
                        maskLength = tm_rt_mask_len(nodePtr); /* mask length */
                        if (maskLength <= keyBits)
                        {
                            parentPtr = intNodePtr;
                            tfRtInsertNet(parentPtr, nodePtr, (int) maskLength);
                            nodePtr->rtnFlags |= TM_RTN_LIST_NETWORK;
                        }
                    }
                    nodePtr = tm_rt_next_dup(nodePtr);
                } while (nodePtr != (ttRtRadixNodePtr)0);
            }
            nodePtr = leafPtr; /* starting point of searching up the tree */
        }
/*
 * Node(s) inserted in tree. Insert leaf network entry in network list.
 * Set leaf and internal node flags
 */
        if ( maskBits != tm_bytes_to_bits(tm_byte_count(sizeof(ttIpAddress))) )
/* non host route (leaf network entry) */
        {
/*
 * If leaf network entry insert leaf node in appropriate network list.
 * Go up the tree (from the new leaf/head of duplicates) to add new node
 * in network list of the last node from leaf (first node from top)
 * that has its rtnBitOffset >= mask length.
 *
 * nodePtr points to leaf/head of duplicates. parentPtr is don't care
 * nodePtr var arg, parentPtr temp var, maskBits value arg.
 */
            tm_rt_tree_find_net_list(nodePtr, parentPtr, maskBits);
            if (tm_16bit_one_bit_set(nodePtr->rtnFlags, TM_RTN_NODE))
            {
/*
 * nodePtr points to non leaf node where we want to insert network entry.
 * Insert leafPtr into network list of nodePtr. parentPtr is don't
 * care.
 * nodePtr value arg/temp var, parentPtr temp var, leafPtr, maskBits value
 * args
 */
                tfRtInsertNet(nodePtr, leafPtr, (int) maskBits);
                leafPtr->rtnFlags |= TM_RTN_LIST_NETWORK;
            }
/* Set route entry and leaf flags */
            rtePtr->rteFlags2 = TM_RTE2_UP;
            leafPtr->rtnFlags |= (TM_RTN_IN | TM_RTN_NETWORK);
        }
        else
        {
/* host route */
/* Set route entry and leaf flags */
            rtePtr->rteFlags2 = (tt16Bit)(TM_RTE2_UP | TM_RTE2_HOST);
            leafPtr->rtnFlags = TM_RTN_IN;
        }
        rtePtr->rteFlags = flags;
    }
/* If ARP/MCAST/BCAST mapping entry */
    if ( tm_16bit_one_bit_set(flags, TM_RTE_CLONED) )
    {
/* initialize the NS transmission timer to null */
#ifdef TM_6_USE_NUD
        rtePtr->rte6HSNudTimerPtr = TM_TMR_NULL_PTR;
#endif /* TM_6_USE_NUD */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
        if (IN6_IS_ADDR_V4MAPPED(&(rtePtr->rteDest)))
        {
            rtePtr->rte6HSNudState = TM_6_NUD_IPV4_STATE;
        }
        else
#endif /* dual IP layer */
        {
            rtePtr->rte6HSNudState = TM_6_NUD_CLONED_STATE;
        }
#endif /* TM_USE_IPV6 */
        
/*
 * Add a timer for cloned entries - this timer should only be started if the
 * timer hasn't already been started and if this is the first IPv4 ARP entry
 * or if this is the first IPv6 ARP entry and Path MTU discovery is
 * enabled.
 */
#if (defined(TM_USE_IPV4) || defined(TM_6_PMTU_DISC))
        if (tm_context(tvRtArpTimerPtr) == TM_TMR_NULL_PTR)
        {
            timerParm1.genVoidParmPtr = (ttVoidPtr)0;
            tm_context(tvRtArpTimerPtr) = tfTimerAdd(
                                                  tfRtArpTimer,
                                                  timerParm1, /* unused */
                                                  timerParm1, /* unused */
                                                  tm_context(tvArpResolution),
                                                  TM_TIM_AUTO );
/* Do not check on errors from timer, since timer stops thread on failure */
        }
#endif /* TM_USE_IPV4 || TM_6_PMTU_DISC */

/* Counter for ALL ARP entries (both IPv4 and IPv6) */
        tm_context(tvRtArpEntriesCount)++;

/* Counter for just IPv4 ARP entries (only in dual mode). */
#ifdef TM_USE_IPV6
        if (rtePtr->rte6HSNudState == TM_6_NUD_IPV4_STATE)
#endif /* TM_USE_IPV6 */
        {
#ifdef TM_USE_IPV4
            tm_context(tv4RtArpEntriesCount)++;
#endif /* TM_USE_IPV4 */
        }
        
    }
/* End of adding entry in tree */
    if (tm_16bit_one_bit_set(flags, TM_RTE_INDIRECT))
    {
#ifdef TM_USE_IPDUAL
/* Find gateway on the real device, not the virtual device */
        if (devPtr->devTunnelEntryPtDevPtr == TM_DEV_NULL_PTR)
#endif /* TM_USE_IPDUAL */
        {
            if (
#ifdef TM_USE_IPV6
/* Either LAN or Not a zero address */
               (    (tm_6_ll_is_lan(devPtr))
                 || (! (   IN6_IS_ADDR_UNSPECIFIED(
                             &(rtePtr->rteGSGateway))
#ifdef TM_USE_IPV4
                         || tm_6_addr_ipv4_mapped_zero(&(rtePtr->rteGSGateway)) 
#endif /* TM_USE_IPV4 */
                       )
                    )
               )
#else /* !TM_USE_IPV6 */
/* Either LAN or Not a zero address */
               (    tm_4_ll_is_lan(devPtr)
                 || !tm_ip_zero(rtePtr->rteGSGateway))
#endif /* TM_USE_IPV6 */
               )
            {
/*
 * Get router entry for the gateway. If we got a network routing entry
 * and network is LAN, the following code will clone to the ARP router
 * entry
 */
                oRtePtr = tfRtFindNClone(
#ifdef TM_USE_STRONG_ESL
                                           devPtr,
#endif /* TM_USE_STRONG_ESL */
#ifdef TM_USE_IPV6
                                           &(rtePtr->rteGSGateway)
#else /* ! TM_USE_IPV6 */
                                           rtePtr->rteGSGateway
#endif /* ! TM_USE_IPV6 */
                                        );
                if (    (oRtePtr != TM_RTE_NULL_PTR)
                     && tm_16bit_bits_not_set( oRtePtr->rteFlags,
                                               TM_RTE_INDIRECT) )
                {
/* If one found in the tree, increase the ownership count */
                    tm_incr_owner_count(oRtePtr->rteOwnerCount);
                    if (    tm_16bit_one_bit_set(oRtePtr->rteFlags, TM_RTE_ARP)
/* ARP entry */
                         && tm_16bit_bits_not_set(oRtePtr->rteFlags, TM_RTE_STATIC)
/* ARP entry not entered by the user */
                         && (oRtePtr->rteTtl == TM_RTE_INF) )
/* Newly created ARP entry without a mapping */
                    {
/*
 * A newly created unmapped ARP entry has its rteTtl initialized to
 * TM_RTE_INF. Make sure that the ARP entry can be removed if the route is
 * removed without ever having its ARP entry mapped.
 */
                        oRtePtr->rteTtl = tm_context(tvRtTimeout);
#ifdef TM_USE_IPV6
                        tm_kernel_set_critical;
                        rtePtr->rteHSLastReachedTickCount = tvTime;
                        tm_kernel_release_critical; 
#endif /* TM_USE_IPV6 */
                    }
/* Point to it */
                    rtePtr->rteClonedPtr = oRtePtr;
#ifndef TM_USE_STRONG_ESL
                    if (oRtePtr->rteOrigDevPtr != devPtr)
                    {
/* Copy device. For strong ESL, devPtr already match. */
                        rtePtr->rteDevPtr = oRtePtr->rteOrigDevPtr;
                        rtePtr->rteOrigDevPtr = oRtePtr->rteOrigDevPtr;
                    }
#endif /* TM_USE_STRONG_ESL */
/* Copy mhome */
                    rtePtr->rteMhomeIndex = oRtePtr->rteMhomeIndex;
                    rtePtr->rteOrigMhomeIndex = oRtePtr->rteOrigMhomeIndex;
/* 
 * Copy the MTU. Need to protect the route MTU, since it could be changed
 * dynamically due to an IPv6 router advertisement.
 */
                    tm_kernel_set_critical;
                    rtePtr->rteMtu = oRtePtr->rteMtu;
                    tm_kernel_release_critical;
                    if ( tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_DYNAMIC) )
/* RIP, Router Discovery, or REDIRECT gateway entry */
                    {
                        if (tm_context(tvRtTimerPtr) == TM_TMR_NULL_PTR)
                        {
                            timerParm1.genVoidParmPtr = (ttVoidPtr)0;
                            tm_context(tvRtTimerPtr) =
                                      tfTimerAdd( tfRtTimer,
                                                  timerParm1, /* unused */
                                                  timerParm1, /* unused */
                                                  tm_context(tvRtTimerResolution),
                                                  TM_TIM_AUTO);
                        }
/*
 * Keep track of RIP, redirect entries, Path MTu and router discovery
 * entries
 */
                        tm_context(tvRtEntriesCount)++;
                    }
                    else
                    {
/* 
 * If the gateway is to one of our interface IP address, then
 * make the gateway entry clonable, so that ARP entry will be
 * created on the fly for any IP address that does not get found in
 * the tree. Note that in that case the gateway rteClonedPtr points to the
 * local route not the ARP entry, but that is OK because the clone
 * logic will create the ARP entry when tfRtGet() is called, and later
 * on tfRtGet() will return the ARP entry directly.
 */
                        if (tm_4_ll_is_lan(oRtePtr->rteOrigDevPtr))
                        {
#ifdef TM_USE_IPV6
                            ip6AddrPtr = &(tm_6_ip_dev_addr(
                                                     oRtePtr->rteOrigDevPtr,
                                                     oRtePtr->rteOrigMhomeIndex));
#endif /* TM_USE_IPV6 */
                            if (
#ifdef TM_USE_IPV4
                                (
#ifdef TM_USE_IPV6
                                     tm_6_addr_is_ipv4_mapped(
                                                      &(rtePtr->rteGSGateway)) &&
#endif /* TM_USE_IPV6 */
                                     tm_ip_match(
                                        tm_4_ip_addr(rtePtr->rteGSGateway),
                                        tm_ip_dev_addr(oRtePtr->rteOrigDevPtr, 
                                                       oRtePtr->rteOrigMhomeIndex))
                                )
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPDUAL
                                ||
#endif /* TM_USE_IPDUAL */
#ifdef TM_USE_IPV6
                                (    tm_6_ip_local_match(
                                        &rtePtr->rteGSGateway, ip6AddrPtr) &&
                                     (    IN6_IS_ADDR_LINKLOCAL(
                                                            &rtePtr->rteGSGateway)
                                      ||  tm_6_ip_scope_match(
                                            &rtePtr->rteGSGateway, ip6AddrPtr)
                                     )
                                )
#endif /* TM_USE_IPV6 */
                               )
                            {
/* Make the entry clonable */
                                rtePtr->rteFlags |= TM_RTE_CLONE;
#ifdef TM_USE_IPV6
                                rtePtr->rteClonePrefixLength =
                                                         TM_6_IP_HOST_PREFIX_LENGTH;
#else /* !TM_USE_IPV6 */
                                tm_ip_copy(TM_IP_ALL_ONES, rtePtr->rteCloneMask);
#endif /* !TM_USE_IPV6 */
                            }
                        }
                    }
                }
                else
                {
/* tfRtRemoveEntry will decrease the owner count */
                    rtePtr->rteOwnerCount++;
                    tfRtRemoveEntry(rtePtr);
                    errorCode = TM_EHOSTUNREACH;
                    goto rtAddEntryExit;
                }
            }
        }
    }
/* Reset all routing entry caches pointing to indirect/arp/local routes */
    tm_context(tvRtRadixTreeHead).rthResetCachesFlags |= (  TM_RTE_INDIRECT
                                                          | TM_RTE_CLONED
                                                          | TM_RTE_LOCAL );
    tm_context(tvRtRadixTreeHead).rthResetCachesAf |= (AF_INET | AF_INET6);
#ifdef TM_ERROR_CHECKING
#ifdef TM_USE_NETSTAT
#ifdef TM_DEBUG_LOGGING
/* Route entry inserted in the tree */
    tfxLogRteEntry("tfRtAddEntry", rtePtr);
#endif /* TM_DEBUG_LOGGING */
#endif /*TM_USE_NETSTAT*/
#endif /* TM_ERROR_CHECKING */
rtAddEntryExit:
    return errorCode;
}

/*
 * Given a destination address, return output device from route.
 */
ttUserInterface tfRtInterface(
                struct sockaddr_storage TM_FAR * sockAddrPtr
                )
{
    ttUserInterface  interfaceHandle;
    ttRteEntryPtr    rtePtr; 

    interfaceHandle = (ttUserInterface)0;

    tm_call_lock_wait(&tm_context(tvRtTreeLock));
    rtePtr = tfRtFindSockAddr(
#ifdef TM_USE_STRONG_ESL
                                (ttDeviceEntryPtr)0,
#endif /* TM_USE_STRONG_ESL */
                                sockAddrPtr,
#ifdef TM_USE_IPV6
                                TM_6_IP_HOST_PREFIX_LENGTH 
#else /* !TM_USE_IPV6 */
                                TM_IP_ALL_ONES
#endif /* !TM_USE_IPV6 */
                                  );
    if (rtePtr != (ttRteEntryPtr)0)
    {
        interfaceHandle = (ttUserInterface)rtePtr->rteDevPtr;
    }
    tfRtTreeUnlock(); 
    return interfaceHandle;
}

/*
 * tfRtFindSockAddr() is called by tfNgRtDestExists(), tfRtInterface().
 * Try and find a route entry for a given sock addr destination IP address
 * destination network mask/prefix length.
 * Function is called with routing table locked. So we don't need to
 * lock the table.
 */
static ttRteEntryPtr tfRtFindSockAddr(
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr                        devPtr,
#endif /* TM_USE_STRONG_ESL */
    const struct sockaddr_storage TM_FAR *  sockAddrPtr,
#ifdef TM_USE_IPV6
    int                                     prefixLen 
#else /* !TM_USE_IPV6 */
    tt4IpAddress                            netMask
#endif /* !TM_USE_IPV6 */
    )
{
    ttRteEntryPtr    rtePtr; 
#ifdef TM_USE_IPV6
    ttIpAddressPtr   ip6AddrPtr;
    ttIpAddress      temp6IpAddr;
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
    tt4IpAddress     ipAddress;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    int              maskIndex;
#endif /* TM_USE_IPV6 */

#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
    if (sockAddrPtr->addr.ipSockAddr.sa_family == AF_INET6)
    {
#endif /* TM_USE_IPV4 */
        ip6AddrPtr  = (ttIpAddressPtr)&(sockAddrPtr->addr.ipv6.sin6_addr);

/* Retrieve the IPv4 address from a v4 mapped IPv6 address - used below to
 * verify the address being bound to is configured on this interface.
 */ 
#ifdef TM_USE_IPV4
        if ( !(IN6_IS_ADDR_V4MAPPED(ip6AddrPtr)))
#endif /* TM_USE_IPV4 */
        {
/* Embed scope ID (if necessary). Need to copy to a temporary variable since we
 * can't modify the user's parameter.
 */ 
            tm_6_ip_copy_dststruct(ip6AddrPtr, temp6IpAddr);
            ip6AddrPtr = &temp6IpAddr;
            tm_6_addr_add_scope(
                ip6AddrPtr, sockAddrPtr->addr.ipv6.sin6_scope_id);
        }
#ifdef TM_USE_IPV4
    }
    else
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
    {
        tm_ip_copy( (tt4IpAddress)(sockAddrPtr->addr.ipv4.sin_addr.s_addr),
                    ipAddress);
#ifdef TM_USE_IPV6
        ip6AddrPtr = &temp6IpAddr;
        tm_6_addr_to_ipv4_mapped(ipAddress, ip6AddrPtr);
#endif /* TM_USE_IPV6 */
    }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    if (prefixLen < TM_6_IP_HOST_PREFIX_LENGTH)
    {
        if (ip6AddrPtr != &temp6IpAddr)
        {
            tm_6_ip_copy_dststruct(ip6AddrPtr, temp6IpAddr);
            ip6AddrPtr = &temp6IpAddr;
        }
        tm_6_ip_net_prefix(ip6AddrPtr, prefixLen, maskIndex);
    }
    rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                       devPtr,
#endif /* TM_USE_STRONG_ESL */
                       ip6AddrPtr);

    if (rtePtr != TM_RTE_NULL_PTR)
    {
        if (tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_REJECT))
        {
            rtePtr = TM_RTE_NULL_PTR;
        }
    }
#else /*  !TM_USE_IPV6*/
    tm_ip_copy(tm_ip_net(ipAddress, netMask), ipAddress);
    rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                       devPtr,
#endif /* TM_USE_STRONG_ESL */
                       ipAddress);

#endif /* !TM_USE_IPV6 */
    return rtePtr;
}

#if (defined(TM_SNMP_MIB) && defined(TM_USE_IPV4))
/****************************************************************************
* FUNCTION: tfSnmpRtFindIpRouteDest
*
* PURPOSE: Called by the SNMP Agent to get a snapshot of a routing entry
*
* PARAMETERS:
*   ipRouteEntryPtr:
*       Points to a buffer allocated by the SNMP Agent which, on return,
*       has been set to a copy of the routing entry that was found.
*   ipRouteDest:
*       The key of the routing entry in the routing tree that we want to
*       find. This already has the mask applied. This is set to the value of
*       the ipRouteDest field of the ipRouteEntry by the SNMP Agent.
*
* RETURNS:
*   TM_ENOERROR:
*       Successfully found routing entry.
*   TM_ENOENT:
*       Could not find routing entry.
*
* NOTES:
*
****************************************************************************/
int tfSnmpRtFindIpRouteDest(
#ifdef TM_USE_STRONG_ESL
                            ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                            ttRteEntryPtr    ipRouteEntryPtr,
                            tt4IpAddress     ipRouteDest)
{
    ttRteEntryPtr rtePtr;
    int           retVal;

/* initialize default values */
    retVal = TM_ENOENT; /* assume row not found */

/* lock the routing tree for tfRtFind */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));

/* attempt to find the routing entry */
#ifdef TM_USE_IPV6
    rtePtr = tf4RtFind(
#ifdef TM_USE_STRONG_ESL
                       devPtr,
#endif /* TM_USE_STRONG_ESL */
                       ipRouteDest);
#else  /* !TM_USE_IPV6 */
    rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                       devPtr,
#endif /* TM_USE_STRONG_ESL */
                       ipRouteDest);
#endif /* !TM_USE_IPV6 */
    if (rtePtr != TM_RTE_NULL_PTR)
    {
/* we found the routing entry having the specified key */
        retVal = TM_ENOERROR;

        if (ipRouteEntryPtr != TM_RTE_NULL_PTR)
        {
/* grab a snapshot of the routing entry for the SNMP Agent */
            tm_bcopy(rtePtr, ipRouteEntryPtr, sizeof(ttRteEntry));
        }
    }

/* unlock the routing tree */
    tfRtTreeUnlock();

    return retVal;
}
#endif /* (defined(TM_SNMP_MIB) && defined(TM_USE_IPV4)) */



#if (defined(TM_SNMP_MIB) && defined(TM_USE_NG_MIB2))
/****************************************************************************
* FUNCTION: tfSnmpNgRtFindIpRouteDest
*
* PURPOSE: Called by the SNMP Agent to get a snapshot of a routing entry
*
* PARAMETERS:
*   ipRouteEntryPtr:
*       Points to a buffer allocated by the SNMP Agent which, on return,
*       has been set to a copy of the routing entry that was found.
*   ipRouteDest:
*       The key of the routing entry in the routing tree that we want to
*       find. This already has the mask applied. This is set to the value of
*       the ipRouteDest field of the ipRouteEntry by the SNMP Agent.
*
* RETURNS:
*   TM_ENOERROR:
*       Successfully found routing entry.
*   TM_ENOENT:
*       Could not find routing entry.
*
* NOTES:
*
****************************************************************************/
int tfSnmpNgRtFindIpRouteDest(
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
    ttRteEntryPtr    ipRouteEntryPtr,
    ttIpAddressPtr   ipRouteDest)
{
    ttRteEntryPtr rtePtr;
    int           retVal;

/* initialize default values */
    retVal = TM_ENOENT; /* assume row not found */

/* validate input parameters */
    tm_assert(tfSnmpNgRtFindIpRouteDest,
              (ipRouteEntryPtr != TM_RTE_NULL_PTR));

/* lock the routing tree for tfRtFind */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));

/* attempt to find the routing entry */
#ifdef TM_USE_IPV6
    rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                      devPtr,
#endif /* TM_USE_STRONG_ESL */
                      ipRouteDest);
#else /* TM_USE_IPV6 */
    rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                      devPtr,
#endif /* TM_USE_STRONG_ESL */
                     *ipRouteDest);
#endif /* !TM_USE_IPV6 */

    if (rtePtr != TM_RTE_NULL_PTR)
    {
/* we found the routing entry having the specified key */
        retVal = TM_ENOERROR;
/* grab a snapshot of the routing entry for the SNMP Agent */
        tm_bcopy(rtePtr, ipRouteEntryPtr, sizeof(ttRteEntry));
    }

/* unlock the routing tree */
    tfRtTreeUnlock();

    return retVal;
}
#endif /* (defined(TM_SNMP_MIB) && defined(TM_USE_IPV4)) */




/*
 * tfRtFindIpAddrIpNet() is called by tf4RtDelRoute(), tf6RtDelRoute(),
 * tfRtArpCommand(), and by tfRtAddEntry() to
 * try and find the unique route entry for a given destination IP address
 * destination network mask.
 * Function is called with routing table locked. So we don't need to
 * lock the table.
 * IP address and network mask already anded before this call, so we
 * do not have to do it here.
 * Parameters:
 * ipAddress: destination Ip address
 * netMask: destination network mask
 * Return value:
 * Pointer to a routing entry if match is found
 * Null pointer otherwise
 */
ttRteEntryPtr tfRtFindIpAddrIpNet (
#ifdef TM_USE_STRONG_ESL
   ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
#ifdef TM_USE_IPV6
    ttIpAddressPtr  ip6AddrPtr,
    int             prefixLength
#else /* ! TM_USE_IPV6 */
    tt4IpAddress    ipAddress,
    tt4IpAddress    netMask 
#endif /* ! TM_USE_IPV6 */
    )
{
    ttRtRadixNodePtr    nodePtr;
    ttRteEntryPtr       rtePtr;
#ifdef TM_USE_STRONG_ESL
    ttRteEntryPtr       tempRtePtr;
#endif /* TM_USE_STRONG_ESL */
#ifdef TM_USE_IPV6
    tt6IpAddress        temp6IpAddress;
    int                 maskIndex;
#endif /* TM_USE_IPV6 */

#ifdef TM_USE_IPV6
    maskIndex = 0;
#endif /* TM_USE_IPV6 */

    rtePtr = TM_RTE_NULL_PTR;
/* Start at head of tree */
    nodePtr = tm_context(tvRtTreeHeadPtr);
/* Go down the tree until we reach a leaf */
#ifdef TM_USE_IPV6
    if (prefixLength < TM_6_IP_HOST_PREFIX_LENGTH)
    {
        tm_6_ip_copy_dststruct(ip6AddrPtr, temp6IpAddress);
        ip6AddrPtr = &temp6IpAddress;
        tm_6_ip_net_prefix(ip6AddrPtr, prefixLength, maskIndex);
    }
    tm_rt_tree_search(nodePtr, ip6AddrPtr);
#else /* !TM_USE_IPV6 */
    tm_ip_copy(tm_ip_net(ipAddress, netMask), ipAddress);
    tm_rt_tree_search(nodePtr, ipAddress);
#endif /* !TM_USE_IPV6 */
#ifdef TM_ERROR_CHECKING
    tm_rt_check_null_leaf(nodePtr);
#endif /* TM_ERROR_CHECKING */
/* We have now reached a leaf. Do keys match exactly? */
#ifdef TM_USE_IPV6
    if ( tm_6_ip_match(&(nodePtr->rtnLSKey), ip6AddrPtr) )
#else /* !TM_USE_IPV6 */
    if ( tm_ip_match(nodePtr->rtnLSKey, ipAddress) )
#endif /* !TM_USE_IPV6 */
    {
/* Keys match exactly */
        if ( tm_16bit_one_bit_set(nodePtr->rtnFlags, TM_RTN_ROOT) )
/*
 * Check for default entry.
 * Were we asked for the default entry? If so, make sure we point to it.
 * (If there is no default entry, next duplicate is NULL, and we will
 * return a null pointer (no match).) Note that this check will also
 * return a null pointer, in case the user had requested a route to
 * limited broadcast address.
 */
        {
            nodePtr = tm_rt_next_dup(nodePtr);
#ifdef TM_USE_STRONG_ESL
/*
 * This is an end point of the tree (either left or right). We need to go
 * through all the leaves to find a match on the device if it is non null.
 */
            if (devPtr != (ttDeviceEntryPtr)0)
            {
                while (nodePtr != (ttRtRadixNodePtr)0)
                {
                    tempRtePtr = tm_rt_leaf_to_rte(nodePtr);
                    if (tempRtePtr->rteOrigDevPtr == devPtr)
                    {
                        break; /* exact match */
                    }
                    nodePtr = tm_rt_next_dup(nodePtr);
                 }
            }
#endif /* TM_USE_STRONG_ESL */
        }
/*
 * Check the list of duplicate (same key) for an exact network mask match.
 */
        while (   (nodePtr != TM_RADIX_NULL_PTR )
#ifdef TM_USE_IPV6
               && ((int)nodePtr->rtnBitOffset != prefixLength)
#else /* !TM_USE_IPV6 */
               && (tm_ip_not_match(nodePtr->rtnLSMask, netMask))
#endif /* !TM_USE_IPV6 */
              )
        {
            nodePtr = tm_rt_next_dup(nodePtr);
        }
#ifdef TM_USE_STRONG_ESL
        if ( (devPtr != (ttDeviceEntryPtr)0) && (nodePtr != TM_RADIX_NULL_PTR) )
        {
            do
            {
                tempRtePtr = tm_rt_leaf_to_rte(nodePtr);
                if (devPtr == tempRtePtr->rteOrigDevPtr)
                {
                    break;
                }
                nodePtr = tm_rt_next_dup(nodePtr);
            }
            while (   (nodePtr != TM_RADIX_NULL_PTR)
#ifdef TM_USE_IPV6
                   && ((int)nodePtr->rtnBitOffset == prefixLength)
#else /* !TM_USE_IPV6 */
                   && (tm_ip_match(nodePtr->rtnLSMask, netMask))
#endif /* !TM_USE_IPV6 */
              );
        }
#endif /* TM_USE_STRONG_ESL */
        rtePtr = tm_rt_leaf_to_rte(nodePtr);
    }
    return rtePtr;
}


/*
 * tfRtRemoveEntry function description:
 * This function is called with the routing table locked.
 * Try and remove the router table entry pointed to by rtEntryPtr, from
 * the routing tree.
 * 0) If function called in the context of a routing tree walk, postpone
 *    removal when we exit from the tree walk, by queueing the entry
 *    in the routing tree remove list, if not already queued.
 *    If function is not called in the context of the routing tree walk, make
 *    sure to dequeue the entry if it had been queued for later removal in
 *    a previous tree walk.
 * 1) Derefence leafPtr, and remove the leaf from the tree
 *   Note that since we are removing a routing entry, and a routing entry
 *   contains a leaf node and an internal node, we have to make sure that
 *   its internal node is not in the tree. If the routing entry internal node
 *   is in the tree, we need to copy its contents in a free internal node.
 *     a. If Leaf is part of a network list, remove from the list
 *     b. If duplicate member (parent is a leaf), remove from duplicate
 *        chain. (We know that routing entry internal node is free (see
 *        tfRtAddEntry.)
 *     c. else If duplicate head (next duplicate non nil), make the
 *        next duplicate in the chain of duplicates, the head of the chain,
 *        by making the current duplicate head's parent the parent of the
 *        next duplicate. If routing entry internal node is in the tree,
 *        swap it with the next duplicate internal node which we know is
 *        not in the tree (see tfRtAddEntry()).
 *    d. else (not a duplicate), remove the node (parent of the leaf) from
 *       the tree, and make the grandparent of the leaf (parent of the node)
 *       the parent of the node's other child. If the node has a network list
 *       attached to it, move it to the node's other child if it is a node.
 *       If the other child is not a node, just remove all the leaves from
 *       the node's network list. If routing entry internal node is in the
 *       tree, swap it with node (parent of the leaf) which is free since
 *       we were about to remove it.
 * 2) If entry is cloneable, remove loop back routing entry.
 * 3) Deal with local route removal:
 *       If we are removing a local entry, Remove any entry sharing same
 *       device/multihome index as the local entry we are removing,
 *       (i.e ARP/MCAST/BCAST mapping entry, or gateway
 *        entry pointing to the removed (non up) local entry or gateway
 *        entry pointing to an ARP entry for the local entry to be removed)
 * 4) Free the entry:
 *    a. We reset the TM_RTE2_UP and TM_RTN_IN flags.
 *    b. We call tfRtUnGet() which will decrease the owner count and
 *       free the entry if the owner count is 0.
 * Parameters:
 * rtePtr: pointer to routing entry we want to remove
 *
 */
static void tfRtRemoveEntry ( ttRteEntryPtr rtePtr )
{
    ttRtRadixNodePtr    leafPtr; /* leaf and temporary */
    ttRtRadixNodePtr    nodePtr; /* internal node to be removed */
    ttRtRadixNodePtr    otherNodePtr; /* temporary */
    ttRtRadixNodePtr    nextPtr; /* temporary */
    ttRtRadixNodePtr    nextTempPtr; /* temporary */
    ttRteEntryPtr       curRtePtr;
    ttRteEntryPtrPtr    prevNextRtePtrPtr;
#ifdef TM_USE_IPV6
    ttIpAddressPtr      keyPtr;
#ifdef TM_6_USE_NUD
    unsigned int        ownerCount;
#endif /* TM_6_USE_NUD */
#else /* ! TM_USE_IPV6 */
    ttIpAddress         key;
#endif /* ! TM_USE_IPV6 */
    tt16Bit             maskLength;
    tt16Bit             leafFlags;

#ifdef TM_ERROR_CHECKING
#ifdef TM_USE_NETSTAT
#ifdef TM_DEBUG_LOGGING
    tfxLogRteTable("tfRtRemoveEntry");
    tfxLogArpTable("tfRtRemoveEntry");
    tfxLogRteEntry("tfRtRemoveEntry", rtePtr);
#endif /* TM_DEBUG_LOGGING */
#endif /*TM_USE_NETSTAT*/
    if (rtePtr == TM_RTE_NULL_PTR)
    {
        tfKernelError("tfRtRemoveEntry", "null pointer removal");
                            /* Severe Error */
        tfRtTreeUnlock();
        tm_thread_stop;
    }
#endif /* TM_ERROR_CHECKING */
    if (tm_context(tvRtRadixTreeHead).rthTreeWalkCount != TM_16BIT_ZERO)
    {
/*
 * In a tree walk. Postpone removing the routing entry until we return from
 * Tree walk routine
 */
        if (tm_16bit_bits_not_set(rtePtr->rteFlags2, TM_RTE2_IN_REM_LIST))
        {
/* Insert in remove list if not already in */
            rtePtr->rteNextRemPtr = 
                        tm_context(tvRtRadixTreeHead).rthRteRemListPtr;
            tm_context(tvRtRadixTreeHead).rthRteRemListPtr = rtePtr;
            rtePtr->rteFlags2 |= TM_RTE2_IN_REM_LIST;
            if ( tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_INDIRECT) )
            {
                if (rtePtr->rteClonedPtr != TM_RTE_NULL_PTR)
                {
/* If gateway entry points to cloned route, dereference cloned entry. */
                    tfRtUnGet(rtePtr->rteClonedPtr);
                    rtePtr->rteClonedPtr = TM_RTE_NULL_PTR;
                }
            }
        }
    }
    else
    {
        if (tm_16bit_one_bit_set(rtePtr->rteFlags2, TM_RTE2_IN_REM_LIST))
        {
/* Remove from remove list if already in */
            curRtePtr = tm_context(tvRtRadixTreeHead).rthRteRemListPtr;
            prevNextRtePtrPtr =
                             &(tm_context(tvRtRadixTreeHead).rthRteRemListPtr);
            while ((curRtePtr != (ttRteEntryPtr)0) && (curRtePtr != rtePtr))
            {
                prevNextRtePtrPtr = &(curRtePtr->rteNextRemPtr);
                curRtePtr  = curRtePtr->rteNextRemPtr;
            }
            if (curRtePtr == rtePtr)
/* Should always be true */
            {
                *prevNextRtePtrPtr = rtePtr->rteNextRemPtr;
                tm_16bit_clr_bit(rtePtr->rteFlags2, TM_RTE2_IN_REM_LIST);
                rtePtr->rteNextRemPtr = (ttRteEntryPtr)0;
            }
        }
        otherNodePtr = (ttRtRadixNodePtr)0; /* to avoid compiler warning */
/*
 * Remove ARP timer if removing the last alive ARP/MCAST/BCAST
 * mapping entry.
 */
        if (    tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_CLONED )
             && tm_16bit_bits_not_set(rtePtr->rteFlags2,
                                      TM_RTE2_LOCAL_DUPLICATE) )
        {

/* Counter for ALL ARP entries (both IPv4 and IPv6) */        
            tm_context(tvRtArpEntriesCount)--;

#ifdef TM_USE_IPV6
            if (rtePtr->rte6HSNudState == TM_6_NUD_IPV4_STATE)
#endif /* TM_USE_IPV6 */
/* Counter for just IPv4 ARP entries. */
            {
#ifdef TM_USE_IPV4
                tm_context(tv4RtArpEntriesCount)--;
#endif /* TM_USE_IPV4 */
            }
#ifdef TM_USE_IPV6
#ifdef TM_6_USE_NUD
            else
            {
                if (rtePtr->rte6HSNudTimerPtr != TM_TMR_NULL_PTR)
                {
                    tm_timer_remove(rtePtr->rte6HSNudTimerPtr);
                    rtePtr->rte6HSNudTimerPtr = TM_TMR_NULL_PTR;
/* undo the owner count increase when this timer was first started */
                    tm_decr_owner_count(ownerCount, rtePtr->rteOwnerCount);
                }
            }
#endif /* TM_6_USE_NUD */
#endif /* TM_USE_IPV6 */

#if (defined(TM_USE_IPV4) || defined(TM_6_PMTU_DISC))
#ifdef TM_6_PMTU_DISC
            if (tm_context(tvRtArpEntriesCount) == 0)
#else /* !TM_6_PMTU_DISC */
            if (tm_context(tv4RtArpEntriesCount) == 0)
#endif /* TM_6_PMTU_DISC */
            {
                if (tm_context(tvRtArpTimerPtr) != TM_TMR_NULL_PTR)
                {
                    tm_timer_remove(tm_context(tvRtArpTimerPtr));
                    tm_context(tvRtArpTimerPtr) = TM_TMR_NULL_PTR;
                }
            }
#endif /* TM_USE_IPV4 || TM_6_PMTU_DISC */

        }
/*
 * If we are removing the last Router Discovery, REDIRECT or RIP entry,
 * remove the dynamic routing entry aging timer.
 */
        if ( tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_DYNAMIC) )
        {
/* decrease the timer count */
            tm_context(tvRtEntriesCount)--;
            if (tm_context(tvRtEntriesCount) == 0)
            {
                if (tm_context(tvRtTimerPtr) != TM_TMR_NULL_PTR)
                {
/* This could be called from the timer thread. */
                    tm_timer_remove(tm_context(tvRtTimerPtr));
                    tm_context(tvRtTimerPtr) = TM_TMR_NULL_PTR;
                }
            }
        }
/* End of timer removals */
/* point to leaf node */
        leafPtr = &rtePtr->rteLeafNode;
        leafFlags = leafPtr->rtnFlags;
#ifdef TM_USE_IPV6
        keyPtr = &(leafPtr->rtnLSKey);
#else /* !TM_USE_IPV6 */
        key = leafPtr->rtnLSKey;
#endif /* !TM_USE_IPV6 */
#ifdef TM_ERROR_CHECKING
/*
 * Make sure routing entry is not corrupted. leaf node better not be a
 * root or an internal node. Also check that the entry has
 * not been removed before.
 */
        if (   tm_16bit_one_bit_set(leafFlags, TM_RTN_ROOT|TM_RTN_NODE)
            || tm_16bit_bits_not_set(leafFlags, TM_RTN_IN) )
        {
            tfKernelError("tfRtRemoveEntry",
                          "Routing entry root or corrupted or removed twice");
                          /* Severe Error */
            tfRtTreeUnlock();
            tm_thread_stop;
        }
#endif /* TM_ERROR_CHECKING */
/* Remove entry from the tree */
        if ( tm_16bit_one_bit_set(leafFlags, TM_RTN_LIST_NETWORK) )
        {
/* If Leaf is a network route, remove from its network list */
#ifdef TM_ERROR_CHECKING
            if (tm_rt_prev_net(leafPtr) == TM_RADIX_NULL_PTR)
            {
                tfKernelError("tfRtRemoveEntry",
                              "Network routing entry not on a network list");
                              /* Severe Error */
                tfRtTreeUnlock();
                tm_thread_stop;
            }
#endif /* TM_ERROR_CHECKING */
/* remove from the list. leafPtr value arg */
            tm_rt_remove_net(leafPtr);
            tm_8bit_clr_bit( leafPtr->rtnFlags, TM_RTN_LIST_NETWORK );
        }
        nodePtr = tm_rt_parent(leafPtr);
        nextPtr = tm_rt_next_dup(leafPtr);
/*
 * check whether parent is a leaf.
 */
        if ( tm_16bit_bits_not_set(nodePtr->rtnFlags, TM_RTN_NODE) )
        {
/* Duplicate member (parent is a leaf), remove from duplicate chain */
#ifdef TM_ERROR_CHECKING
#ifdef TM_USE_IPV6
            if (!(tm_6_ip_match(&(nodePtr->rtnLSKey), keyPtr)))
#else /* !TM_USE_IPV6 */
            if (tm_ip_not_match(nodePtr->rtnLSKey, key))
#endif /* !TM_USE_IPV6 */
            {
                tfKernelError("tfRtRemoveEntry",
                              "duplicate chain with non duplicate keys!");
                                /* Severe Error */
                tfRtTreeUnlock();
                tm_thread_stop;
            }
/* The duplicate member internal node should not be in the tree */
            if ( tm_16bit_one_bit_set( rtePtr->rteInternalNode.rtnFlags,
                                       TM_RTN_IN) )
            {
                tfKernelError("tfRtRemoveEntry",
                              "duplicate member with internal node in tree!");
                            /* Severe Error */
                tfRtTreeUnlock();
                tm_thread_stop;
            }
#endif /* TM_ERROR_CHECKING */
            tm_rt_remove_dup_member(leafPtr, nodePtr, nextPtr); /* value args */
        }
        else
        {
            if (nextPtr != TM_RADIX_NULL_PTR)
            {
/* duplicate head (next duplicate non nil, and parent is node) */
#ifdef TM_ERROR_CHECKING
                if (   tm_16bit_one_bit_set(nextPtr->rtnFlags, TM_RTN_NODE)
#ifdef TM_USE_IPV6
                    || (!(tm_6_ip_match(&(nextPtr->rtnLSKey), keyPtr)))
#else /* !TM_USE_IPV6 */
                    || tm_ip_not_match(nextPtr->rtnLSKey, key)
#endif /* !TM_USE_IPV6 */
                    )
                {
                    tfKernelError("tfRtRemoveEntry",
                                  "duplicate chain with non duplicate!");
                                /* Severe Error */
                    tfRtTreeUnlock();
                    tm_thread_stop;
                }
#endif /* TM_ERROR_CHECKING */
/*
 * make the next duplicate in the chain of duplicates, the head of the chain.
 * otherNodePtr is parent, nextPtr (next duplicate) is child.
 */
                otherNodePtr = nodePtr;
#ifdef TM_USE_IPV6
                tm_rt_set_child_parent_key(otherNodePtr, nextPtr, keyPtr);
#else /* !TM_USE_IPV6 */
                tm_rt_set_child_parent_key(otherNodePtr, nextPtr, key);
#endif /* !TM_USE_IPV6 */
/*
 * Get internal node of nextPtr. This is the free internal node we can use to
 * copy internal node of the routing entry that we are removing. See below.
 */
                nodePtr = tm_rt_leaf_to_int_node(nextPtr);
#ifdef TM_ERROR_CHECKING
/* The next duplicate internal node should not be in the tree */
                if (tm_16bit_one_bit_set(nodePtr->rtnFlags, TM_RTN_IN))
                {
                    tfKernelError("tfRtRemoveEntry",
                                  "next duplicate internal node is not free!");
                                /* Severe Error */
                    tfRtTreeUnlock();
                    tm_thread_stop;
                }
#endif /* TM_ERROR_CHECKING */
            }
            else
/*
 * Not a duplicate: remove nodePtr (parent of the leaf) from the tree, and
 *   make the grandparent the parent of nodePtr's other child.
 *   If nodePtr has a network list attached to it, move it to
 *   nodePtr 's other child if it is a node. If the other child
 *   is not a node, just remove all the leaves from nodePtr's network list.
 *   Since we are removing the routing entry, and the routing entry contains
 *   a leaf node and an internal node, we have to make sure that its internal
 *   node is not in the tree. If routing entry internal node is in the tree,
 *   copy its contents in nodePtr and keep nodePtr around.
 */
            {
                nextPtr = tm_rt_parent(nodePtr); /* grandparent */
                otherNodePtr = tm_rt_get_other_child(nodePtr, leafPtr);
/*
 * make grandparent (nextPtr) the parent of the parent (nodePtr)'s
 * other child (otherNodePtr).
 */
#ifdef TM_USE_IPV6
                tm_rt_set_child_parent_key(nextPtr, otherNodePtr, keyPtr);
#else /* !TM_USE_IPV6 */
                tm_rt_set_child_parent_key(nextPtr, otherNodePtr, key);
#endif /* !TM_USE_IPV6 */
                if ( tm_16bit_one_bit_set(otherNodePtr->rtnFlags, TM_RTN_NODE) )
                {
                    nextPtr = tm_rt_next_net(nodePtr);
                    while (nextPtr != TM_RADIX_NULL_PTR)
                    {
                        maskLength = tm_rt_mask_len(nextPtr);
                        nextTempPtr = tm_rt_next_net(nextPtr);
/* remove network entry from nodePtr' network list */
                        tm_rt_remove_net(nextPtr); /* nextPtr value arg */
                        tfRtInsertNet(otherNodePtr, nextPtr, (int) maskLength);
/*
 * nextPtr has been inserted in other child's list, start next insertion at
 * current node (nextPtr).
 */
                        otherNodePtr = nextPtr;
                        nextPtr = nextTempPtr;
                    }
                }
                else
/*
 * Other child is a leaf. Just reset network list flag from all the leaves
 * from nodePtr's network list.
 */
                {
                    nextPtr = tm_rt_next_net(nodePtr);
                    tm_rt_next_net(nodePtr) = TM_RADIX_NULL_PTR;
                    while (nextPtr != TM_RADIX_NULL_PTR)
                    {
                        tm_8bit_clr_bit(nextPtr->rtnFlags, TM_RTN_LIST_NETWORK);
                        nextPtr = tm_rt_next_net(nextPtr);
/* remove mask from node */
                    }
                }
            }

/* nodePtr no longer in the tree */
            tm_8bit_clr_bit( nodePtr->rtnFlags, TM_RTN_IN );
            otherNodePtr = &rtePtr->rteInternalNode; /* internal node */
            if (tm_16bit_one_bit_set(otherNodePtr->rtnFlags, TM_RTN_IN))
            {
/*
 * internal node (otherNodePtr) is in the tree. We need to save
 * its contents in free nodePtr. That way otherNodePtr can be freed
 * along with the leafPtr. (They are both members of the same
 * routing entry.) tfRtCopyNode will reset the TM_RTN_IN flag in
 * otherNodePtr. After the copy, nodePtr is in the tree.
 */
                tfRtCopyNode(otherNodePtr, nodePtr);
            }
        }
/* End of removal from the tree */
/* If entry is a local entry */
        if ( tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_LOCAL) )
        {
/*
 * remove any entry sharing same device/multihome index as the local
 * entry we are removing (i.e ARP/MCAST/BCAST mapping entry,
 * or gateway entry pointing to the removed (non up) local entry or
 * associated ARP entry). Note that we could not find a matching
 * IPv6 default router because the default routes uses the reserved
 * mhome index not used to configure local routes.
 */
            (void)tfRtTreeWalk(tfRtRemObsLocalCB, rtePtr, TM_8BIT_ZERO);
        }
#ifdef TM_SNMP_CACHE
        if (rtePtr->rteSnmpCacheEntry.sceNode.nodeNextPtr != (ttVoidPtr)0)
        {
            tfSnmpdCacheDeleteRoute(rtePtr);
        }
#endif /* TM_SNMP_CACHE */
/* Free the entry */
    tm_8bit_clr_bit( rtePtr->rteLeafNode.rtnFlags, TM_RTN_IN );
    tm_16bit_clr_bit( rtePtr->rteFlags2, TM_RTE2_UP );
/* Remove default router entry if it points to this entry */
#ifdef TM_USE_IPV6
        tf6RtDelDefRouter(
#ifdef TM_USE_STRONG_ESL
                          rtePtr->rteOrigDevPtr,
#endif /* TM_USE_STRONG_ESL */
                          &(rtePtr->rteDest), TM_8BIT_ZERO);
#endif /* TM_USE_IPV6 */

#ifdef TM_ERROR_CHECKING
/* Keep track of out of tree non recycled entries */
#ifdef TM_USE_IPDUAL
        if (    (tm_16bit_bits_not_set(rtePtr->rteFlags2, TM_RTE2_IP4_DEF_GW))
             || (tm_16bit_bits_not_set(rtePtr->rteFlags, TM_RTE_REJECT)) )
/* Not the reject default IPv4 gateway */
#endif /* TM_USE_IPDUAL */
        {
            tm_context(tvRteLimboCount)++;
        }
#endif /* TM_ERROR_CHECKING */
#ifdef TM_USE_IPDUAL
        if (tm_16bit_all_bits_set(rtePtr->rteFlags2,
                                  TM_RTE2_IP4_DEF_GW | TM_RTE2_IP4_INS_DEF_GW))
/*
 * We are removing the default IPv4 gateway and we want it replaced because
 * in dual mode we always need to have an IPv4 default gateway. So if we remove
 * the Ipv4 default gateway, insert a dummy one.
 */
        {
            rtePtr->rteFlags2 = (tt16Bit)
                                (rtePtr->rteFlags2 & ~TM_RTE2_IP4_INS_DEF_GW);
            (void)tf4RtInsertEmptyDefaultGateway(
#ifdef TM_USE_STRONG_ESL
                                                  rtePtr->rteOrigDevPtr
#endif /* TM_USE_STRONG_ESL */
                                                  );
        }
#endif /* TM_USE_IPDUAL */
        tfRtUnGet(rtePtr);
/* End of freeing the entry */
    }
}


/*
 * Clone a routing entry for ARP entry, our own IP address, directed
 * broadcast or multicast entry. Called with routing tree lock set. Called
 * by tfRtGet() via (tfRtFindNClone) (itself called from the layers 3-4
 * (including ARP), or from user add ARP entry) or tfRtAddEntry() (for an
 * indirect route for the gateway ARP router entry).
 * 0) Check for permanent cloned entries (our own IP address,
 *    multicast/broadcast)
 * 1) We allocate a new routing entry
 * 2) We use the routing fields from the routing entry we want to clone
 *    (for the mask field we use the clone mask field)
 * 3) Insert the cloned routing entry in the tree.
 * 4) return cloned routing entry.
 *
 * rtePtr       pointer to local lan link layer network routing entry,
 * destIpAddr   destination IP address
 */

static ttRteEntryPtr tfRtClone (
    ttRteEntryPtr rtePtr, 
#ifdef TM_USE_IPV6
    ttIpAddressPtr dest6IpAddrPtr
#else /* ! TM_USE_IPV6 */
    tt4IpAddress   destIpAddr
#endif /* ! TM_USE_IPV6 */
    )
{
    ttRteEntryPtr       cloneRtePtr;
    ttDeviceEntryPtr    devPtr;
    tt32Bit             ttlTimeout;
    int                 linkBroadMcast;
    int                 errorCode;
#ifdef TM_SNMP_CACHE
    int                 index;
#endif /* TM_SNMP_CACHE */
    tt16Bit             flags;
/* link layer address */
    tt8Bit              linkPhysAddr[TM_MAX_PHYS_ADDR];
    tt8BitPtr           destLinkPhysAddrPtr;
    int                 isLan;
    tt16Bit             mhomeIndex;
    int                 failed;

/*
 * Default timeout for an ARP entry is TM_RTE_INF before a link
 * layer mapping is found.
 * ARP timeout initialized when the ARP code/User store a mapping.
 * If this ARP entry is created because a route is being inserted, then
 * ARP timeout will be overwritten with tvRtTimeout in tfRtAddEntry(),
 * to make sure that the ARP cache entry will be eventually removed,
 * if we fail to get an ARP reply from the gateway.
 */
    cloneRtePtr = TM_RTE_NULL_PTR;
    linkBroadMcast = TM_ETHER_UCAST;
    ttlTimeout = tm_context(tvArpTimeout);
    devPtr = rtePtr->rteOrigDevPtr;
    mhomeIndex = rtePtr->rteOrigMhomeIndex;
    isLan = tm_4_ll_is_lan(devPtr);
/*
 * First check for permanent cloned routing entries (our own IP address,
 * or multicast/broadcast).
 */
#ifdef TM_USE_IPV6
    if (tm_6_ip_match(dest6IpAddrPtr, &(rtePtr->rteHSDevIpAdd)))
#else /* !TM_USE_IPV6 */
    if (tm_ip_match(destIpAddr, rtePtr->rteHSDevIpAdd))
#endif /* !TM_USE_IPV6 */
/* If destination is to our own interface, interface is loopback */
    {
      /* Mapped entry */
#if ( (defined(TM_SINGLE_INTERFACE_HOME) || defined(TM_LOOP_TO_DRIVER)) )

/* Make the entry non permanent, since it points to the local device */
        flags = TM_RTE_CLONED | TM_RTE_LINK_LAYER;
        linkBroadMcast = -1; /* So that the physical address is copied below */

#else /* !defined(TM_SINGLE_INTERFACE_HOME) && !defined(TM_LOOP_TO_DRIVER) */
        if (tm_context(tvLoopbackDevPtr) == TM_DEV_NULL_PTR)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfRtClone",
                            "No loop back driver configured");
#endif /* TM_ERROR_CHECKING */
            goto rtCloneExit;
        }
        flags = TM_RTE_LINK_LAYER; /* permanent entry */
        ttlTimeout = TM_RTE_INF; /* permanent entry */

#endif /* !defined(TM_SINGLE_INTERFACE_HOME) && !defined(TM_LOOP_TO_DRIVER) */
    }
    else
    {
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
        if (!IN6_IS_ADDR_V4MAPPED(dest6IpAddrPtr))
        {
#endif /* dual IP layer */
            if (IN6_IS_ADDR_MULTICAST(dest6IpAddrPtr))
            {
                (void)tf6IpMcastToLan(dest6IpAddrPtr, devPtr,
                                      linkPhysAddr);
/*
 * Multicast entry.
 */
                flags = (tt16Bit)(   TM_RTE_CLONED
                                     | TM_RTE_LINK_LAYER
                                     | TM_RTE_MCAST );
            }
            else
            {
                flags = TM_RTE_CLONED | TM_RTE_ARP;
            }
#ifdef TM_USE_IPV4
        }
        else
        {
#endif /* dual IP layer */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
            linkBroadMcast = tfIpBroadMcastToLan(
                dest6IpAddrPtr->s6LAddr[3],
                devPtr,
                mhomeIndex,
                linkPhysAddr);
#else /* !TM_USE_IPV6 */
            linkBroadMcast = tfIpBroadMcastToLan(
                destIpAddr,
                devPtr,
                mhomeIndex,
                linkPhysAddr);
#endif /* !TM_USE_IPV6 */
            if ( linkBroadMcast == TM_ETHER_UCAST )
            {
/* Arp entry for LAN interface */
                flags = TM_RTE_CLONED | TM_RTE_ARP;
                if (!isLan)
                {
/* Do not add/Get an ARP entry on non LAN interfaces */
                    cloneRtePtr = rtePtr;
                }
            }
            else
/*
 * If we mapped to Broadcast or multicast link address
 */
            {
                if (linkBroadMcast == TM_ETHER_MULTI)
                {
/*
 * Multicast entry.
 */
                    flags = (tt16Bit)(   TM_RTE_CLONED
                                         | TM_RTE_LINK_LAYER
                                         | TM_RTE_MCAST );
                }
                else
                {
/*
 * Broadcast entry.
 */
                    flags = (tt16Bit)(   TM_RTE_CLONED
                                         | TM_RTE_LINK_LAYER
                                         | TM_RTE_BCAST );
                }
            }
#ifdef TM_USE_IPV6
        }
#endif /* dual IP layer */
#endif /* TM_USE_IPV4 */
    }
    if (cloneRtePtr == TM_RTE_NULL_PTR)
    {
/* If not permanent entry, check on ARP cache limit */
        if (    (flags != TM_RTE_LINK_LAYER)
             && (    tm_context(tvRtArpEntriesCount) 
                  >= tm_context(tvArpMaxEntries) ) )
        {
            failed = 0;
            while (failed < 2)
            {
/* We reached the limit. Try and find the least used entry. */
                (void)tfRtTreeWalk( tfRtArpGetLeastUsedCB, &cloneRtePtr,
                                    TM_8BIT_ZERO );
                if (cloneRtePtr != TM_RTE_NULL_PTR)
                {
/* Remove the least used ARP entry */
                    tfRtRemoveEntry(cloneRtePtr);
                    break;
                }
                else
                {
                    failed++;
                    if (failed == 1)
                    {
/* Try and free as many ARP/MCAST/BCAST mapping entries as we can */
                        tm_rt_reset_caches( TM_RTE_CLONED, 
                                            (AF_INET | AF_INET6) , 
                                            TM_LOC_NOWAIT );
                    }
                    else
                    {
#ifdef TM_ERROR_CHECKING
                        tfKernelWarning("tfRtClone", "ARP cache full");
#endif /* TM_ERROR_CHECKING */
                        goto rtCloneExit;
                    }
                }
            }
        }
/* Add it to the Patricia tree */
/* Allocate a new routing entry */
        cloneRtePtr = (ttRteEntryPtr)tm_recycle_alloc(
                                                (unsigned)sizeof(ttRteEntry),
                                                 TM_RECY_RTE );
        if (cloneRtePtr != TM_RTE_NULL_PTR)
        { 
/* Brand new entry. Initialize owner count */
            cloneRtePtr->rteOwnerCount = 1; /* no need to be critical here */
#ifdef TM_SNMP_CACHE
/* Initialize SNMPD/NSTAT cache index */
            cloneRtePtr->rteSnmpCacheEntry.sceTableIndex = TM_SNMPC_NO_INDEX;
#endif /* TM_SNMP_CACHE */
#ifdef TM_USE_IPV6
            tm_6_ip_copy_structs( rtePtr->rteHSDevIpAdd,
                                  cloneRtePtr->rteHSDevIpAdd);
            tm_6_ip_copy_dststruct(dest6IpAddrPtr, cloneRtePtr->rteDest);
#else /* !TM_USE_IPV6 */
            tm_ip_copy(rtePtr->rteHSDevIpAdd, cloneRtePtr->rteHSDevIpAdd);
#endif /* !TM_USE_IPV6 */
            cloneRtePtr->rteHSPhysAddrLen = rtePtr->rteHSPhysAddrLen;
/* Need to protect the route MTU, since it could be changed dynamically
   due to an IPv6 router advertisement. */
            tm_kernel_set_critical;
            cloneRtePtr->rteMtu = rtePtr->rteMtu;
            tm_kernel_release_critical;
            
/* NOTE: we do not set the rteTtl for an IPv6 entry to TM_RTE_INF here,
   instead we wait until it transitions out of the CLONED/INCOMPLETE state */
            errorCode = tfRtAddEntry( tm_context(tvRtTreeHeadPtr),
                                      cloneRtePtr,
                                      devPtr,
#ifdef TM_USE_IPV6
                                      rtePtr->rteClonePrefixLength,
#else /* !TM_USE_IPV6 */
                                      destIpAddr,
                                      rtePtr->rteCloneMask,
#endif /* !TM_USE_IPV6 */
                                      mhomeIndex,
                                      (int)rtePtr->rteHops,
                                      ttlTimeout,
                                      flags );
            if (errorCode == TM_ENOERROR)
            {
                if (linkBroadMcast != TM_ETHER_UCAST)
                {
/* Broadcast/multicast/driver link address, store the mapping */
                    if (isLan)
/* Note rteHSPhysAddrLen is zero on non LAN interfaces. */
                    {
                        destLinkPhysAddrPtr = &(cloneRtePtr->rteHSPhysAddr[0]);
                        if (linkBroadMcast == -1)
                        {
/* Sending to our own IP address. Store link layer address. */
                            tm_bcopy(devPtr->devPhysAddrArr,
                                     destLinkPhysAddrPtr,
                                     rtePtr->rteHSPhysAddrLen);
                        }
                        else
                        {
/* LAN broadcast/multicast mapping as stored by tfIpBroadMcastToLan() */
#ifdef TM_LINT
LINT_UNINIT_AUTO_BEGIN(linkPhysAddr)
#endif /* TM_LINT */
                            tm_bcopy(linkPhysAddr, destLinkPhysAddrPtr,
                                     rtePtr->rteHSPhysAddrLen);
#ifdef TM_LINT
LINT_UNINIT_AUTO_END(linkPhysAddr)
#endif /* TM_LINT */
                        }
                    }
                }
#ifdef TM_SNMP_CACHE
                else
                {
                    if (flags == (TM_RTE_CLONED | TM_RTE_ARP))
                    {
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
                        tm_kernel_set_critical;
                        cloneRtePtr->rteLastUpdated = tm_snmp_time_ticks(tvTime);
                        tm_kernel_release_critical;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
                        index = TM_SNMPC_ARP_INDEX;
                    }
                    else
                    {
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
                        tm_kernel_set_critical;
                        cloneRtePtr->rteLastUpdated = (tt32Bit)tvTime;
                        tm_kernel_release_critical;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
                        index = TM_SNMPC_ROUTE_INDEX;
                    }
                    tfSnmpdCacheInsertRoute(cloneRtePtr, index);
                }
#endif /* TM_SNMP_CACHE */
#if !defined(TM_SINGLE_INTERFACE_HOME) && !defined(TM_LOOP_TO_DRIVER)
                if (flags == TM_RTE_LINK_LAYER) /* permanent loop back entry */
                {
                    rtePtr->rteClonedPtr = cloneRtePtr;
                    cloneRtePtr->rteDevPtr = tm_context(tvLoopbackDevPtr);
                   
                }
#endif /* !defined(TM_SINGLE_INTERFACE_HOME) && !defined(TM_LOOP_TO_DRIVER) */
            }
            else
            {
/* routing entry could not be inserted. Recycle it */
#ifdef TM_ERROR_CHECKING
/* Keep track of out of tree non recycled entries */
                tfKernelWarning(
                    "tfRtClone", 
                    "Could not insert ARP entry in routing tree");
                tm_context(tvRteLimboCount)++;
#endif /* TM_ERROR_CHECKING */
                tfRtUnGet(cloneRtePtr);
                cloneRtePtr = TM_RTE_NULL_PTR;
            }
        }
#ifdef TM_ERROR_CHECKING
        else
        {
            tfKernelError("tfRtClone", "No buffer to allocate clone entry");
        }
#endif /* TM_ERROR_CHECKING */
    }
rtCloneExit:
/* return cloned entry */
    return cloneRtePtr;
}

/*
 * Get router entry for the Destination IP address 
 * If REJECT flag is set, return null pointer.
 * If we got a LAN network entry (TM_RTE_CLONE set), clone a ARP router entry.
 *
 * destIpAddr value arg
 */
static ttRteEntryPtr tfRtFindNClone (
#ifdef TM_USE_STRONG_ESL
   ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
#ifdef TM_USE_IPV6
    ttIpAddressPtr dest6IpAddrPtr
#else /* ! TM_USE_IPV6 */
    tt4IpAddress   destIpAddr
#endif /* ! TM_USE_IPV6 */
    )
{
    ttRteEntryPtr rtePtr;

#ifdef TM_USE_IPV6
    rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                      devPtr,
#endif /* TM_USE_STRONG_ESL */
                      dest6IpAddrPtr);
#else /* ! TM_USE_IPV6 */
    rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                      devPtr,
#endif /* TM_USE_STRONG_ESL */
                      destIpAddr);
#endif /* ! TM_USE_IPV6 */
    if (rtePtr != TM_RTE_NULL_PTR) 
    {
        if ( tm_16bit_bits_not_set(rtePtr->rteFlags, TM_RTE_REJECT) )
        {
            if ( tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_CLONE) )
/* If entry is for a LAN network destination, clone for specific host */
            {
#ifdef TM_USE_IPV6
                rtePtr = tfRtClone(rtePtr, dest6IpAddrPtr);
#else /* ! TM_USE_IPV6 */
                rtePtr = tfRtClone(rtePtr, destIpAddr);
#endif /* ! TM_USE_IPV6 */
            }
        }
        else
        {
            rtePtr = TM_RTE_NULL_PTR;
        }
    }
    return rtePtr;
}

/*
 * tfRtResetCaches() Function Description
 * Called by the router code with TM_RTE_INDIRECT, or TM_RTE_CLONED
 * when all the socket and IP send and forward routing cache
 * entries need to be reset, because a new route has been added
 * (TM_RTE_INDIRECT flag), or because there is no more room in the ARP
 * cache (TM_RTE_CLONED).
 * Also called by the user ARP cache flush routine.
 *
 * Parameter       Description
 * rteFlag         TM_RTE_INDIRECT, or TM_RTE_ARP
 *
 * No Return
 */
static void tfRtResetCaches (  tt16Bit           rteFlag, 
                               int               addressFamilyFlag
#ifdef TM_LOCK_NEEDED
                             , tt8Bit            lockFlag
#endif /* !TM_LOCK_NEEDED */
                            )
{
    tfIpResetCaches( rteFlag, addressFamilyFlag
#ifdef TM_LOCK_NEEDED
                   , lockFlag
#endif /* !TM_LOCK_NEEDED */
                    );
    tfSocketResetCaches( rteFlag, addressFamilyFlag
#ifdef TM_LOCK_NEEDED
                       , lockFlag
#endif /* !TM_LOCK_NEEDED */
                    );
}

/*
 * tfRtCacheUnGet() Function Description
 * Called to release routing entry acquired in the cache,
 * and also arp entry (if any) concurrently acquired in the cache.
 *
 * Parameters
 * Parameter       Description
 * rtcPtr         Pointer to the cache entry
 *
 * No Return
 */

void tfRtCacheUnGet (ttRtCacheEntryPtr rtcPtr)
{
    tfRtCacheReset(rtcPtr, (tt16Bit)0xFFFF);
}

/*
 * tfRtCacheReset() Function Description
 * Called by the socket interface when the user closes the socket
 * to release the socket routing cache entry with rteFlag == 0xFFFF, or
 * by tfRtResetCaches() to reset an individual cache
 * pointing to an ARP/MCAST/BCAST entry (rteFlag == TM_RTE_CLONED),
 * or pointing to an indirect route (rteFlag == TM_RTE_INDIRECT).
 * We release the routing cache entry, if it corresponds to the type
 * specified in the rteFlag parameter. In that case, we also release the ARP
 * cache entry if it had been concurrently acquired (if routing entry was
 * indirect.)
 *
 * Parameters
 * Parameter       Description
 * rtcPtr         Pointer to the cache entry
 * rteFlag        either 0xFFFF (any type), or TM_RTE_CLONED
 *                or TM_RTE_INDIRECT (indirect route).
 *
 * No Return
 */

void tfRtCacheReset (ttRtCacheEntryPtr rtcPtr, tt16Bit rteFlag)
{
    ttRteEntryPtr rtePtr;

    if (rtcPtr->rtcRtePtr != TM_RTE_NULL_PTR)
    {
        rtePtr = rtcPtr->rtcRtePtr;
        if (tm_16bit_one_bit_set(rtePtr->rteFlags, rteFlag))
        {
            tfRtUnGet(rtcPtr->rtcRtePtr);
            rtcPtr->rtcRtePtr = TM_RTE_NULL_PTR;
            if (rtcPtr->rtcRteClonedPtr != TM_RTE_NULL_PTR)
/* Corresponding ARP entry */
            {
                tfRtUnGet(rtcPtr->rtcRteClonedPtr);
                rtcPtr->rtcRteClonedPtr = TM_RTE_NULL_PTR;
            }
#ifdef TM_USE_IPV6
            rtcPtr->rtcDestIpAddrType = TM_8BIT_ZERO;
#endif /* TM_USE_IPV6 */
        }
    }
}


/*
 * copy oldNode to newNode, reset old node's in flag
 *
 * oldNodePtr value arg, newNodePtr value arg, tempPtr temp var
 */
static void tfRtCopyNode ( ttRtRadixNodePtr oldNodePtr,
                           ttRtRadixNodePtr newNodePtr )
{
    ttRtRadixNodePtr tempPtr;

/* copy old node to new node */
    tm_bcopy(oldNodePtr, newNodePtr, sizeof(ttRtRadixNode));

/*
 * If there is a network list attached to old node, make it point to
 * new node
 */
    tempPtr = tm_rt_next_net(oldNodePtr);
    if (tempPtr != TM_RADIX_NULL_PTR)
    {
        tm_rt_prev_net(tempPtr) = newNodePtr;
    }
/* Make the old node's children point to the new node */
    tm_rt_parent(tm_rt_left_child(oldNodePtr)) = newNodePtr;
    tm_rt_parent(tm_rt_right_child(oldNodePtr)) = newNodePtr;
    tempPtr = tm_rt_parent(oldNodePtr); /* parent */
/* Make the old node's parent  point to new node */
    tm_rt_set_new_child(tempPtr, oldNodePtr, newNodePtr);
    tm_8bit_clr_bit( oldNodePtr->rtnFlags, TM_RTN_IN );
}

/*
 * Insert network entry (leafPtr) in sorted list of network entries whose
 * head is nodePtr. On return nodePtr will be leafPtr previous, and nextPtr
 * will be leafPtr next.
 *
 * nodePtr value arg/temp var, nextPtr temp var, leafPtr, maskLen value args
 */
static void tfRtInsertNet ( ttRtRadixNodePtr nodePtr,
                            ttRtRadixNodePtr leafPtr,
                            int              maskLen )
{
    ttRtRadixNodePtr nextPtr;
#ifdef TM_USE_STRONG_ESL
    ttRteEntryPtr    rtePtr;
    ttDeviceEntryPtr devPtr;
    tt32Bit          hops;
#endif /* TM_USE_STRONG_ESL */

#ifdef TM_USE_STRONG_ESL
    rtePtr = tm_rt_leaf_to_rte(leafPtr);
    hops = rtePtr->rteHops;
    devPtr = rtePtr->rteOrigDevPtr;
#endif /* TM_USE_STRONG_ESL */
    nextPtr = tm_rt_next_net(nodePtr);
    while (nextPtr != TM_RADIX_NULL_PTR)
    {
/* Insert leaf with with the longest masks first */
        if (maskLen > (int)(tm_rt_mask_len(nextPtr)))
        {
            tm_rt_prev_net(nextPtr) = leafPtr;
            break;
        }
#ifdef TM_USE_STRONG_ESL
        else
        {
            if (maskLen == (int)(tm_rt_mask_len(nextPtr))) 
/*
 * For same mask sort in order of increasing hops.
 * For same mask, and same hops, sort in order of increasing devIndex
 */
            {
                 rtePtr = tm_rt_leaf_to_rte(nextPtr);
                 if (    (hops < rtePtr->rteHops)
                      || (    (hops == rtePtr->rteHops)
                           && (devPtr->devIndex
                                   < rtePtr->rteOrigDevPtr->devIndex ) ) )
                 {
                    tm_rt_prev_net(nextPtr) = leafPtr;
                    break;
                 }
            }
        }
#endif /* TM_USE_STRONG_ESL */
/* need to move nodePtr around since list is not cicular */
        nodePtr = nextPtr;
        nextPtr = tm_rt_next_net(nextPtr);
    }
    tm_rt_next_net(leafPtr) = nextPtr;
    tm_rt_prev_net(leafPtr) = nodePtr;
    tm_rt_next_net(nodePtr) = leafPtr;
}

#ifdef TM_LITTLE_ENDIAN

tt16Bit tfRtBitOffset( tt4IpAddress    key,
                       tt4IpAddress    ipAddr,
                       tt4IpAddressPtr bitMaskPtr )
{ 
auto    tt4IpAddress    bittest;
        tt8BitPtr       ucp;
#ifndef TM_DSP
        tt8BitPtr       ucp2;
#endif /* ! TM_DSP */
        tt8Bit          uc;
        tt16Bit         bits;
        tt16Bit         bytes;

    bittest = tm_ip_xor(key, ipAddr); /* keep only the bits that differ */
    ucp = (tt8BitPtr)&bittest;
    bytes = (tt16Bit)0;
#ifdef TM_DSP
    while ((((*ucp >> (bytes << 3)) & 0xFF) == TM_8BIT_ZERO) && 
            (bytes < (sizeof(tt4IpAddress) << 2)))
    { 
        bytes++;
    } 
    if (bytes != (sizeof(tt4IpAddress) << 2))
    {
        bits = TM_RT_BITS_PER_BYTE; /* assume all bits match */
        uc = (*ucp >> (bytes << 3)) & 0xFF;
/* decrease by the number of non matching bits in the failing byte */
        while (uc != TM_8BIT_ZERO)
        {
            bits--;
            uc >>= 1;
        }
        if (bitMaskPtr != (tt4IpAddressPtr)0)
        {
            *bitMaskPtr = (0xffffffff & (((TM_RTN_HIGH_BIT >> bits) & 0xFF) << 
                                         (bytes << 3)));
        }
    }
    else
    {
        bits = (tt16Bit)0;
        if (bitMaskPtr != (tt4IpAddressPtr)0)
        {
            *bitMaskPtr = 0;
        }
    }
#else /* !TM_DSP */
    while ((*ucp == TM_8BIT_ZERO) && (bytes < sizeof(tt4IpAddress)))
    { 
        ucp++;
        bytes++;
    } 
    if (bytes != sizeof(tt4IpAddress))
    {
        bits = TM_RT_BITS_PER_BYTE; /* assume all bits match */
        uc = *ucp;
/* decrease by the number of non matching bits in the failing byte */
        while (uc != TM_8BIT_ZERO)
        {
            bits--;
            uc >>= 1;
        }
    }
    else
    {
        bits = (tt16Bit)0;
    }
    if (bitMaskPtr != (tt4IpAddressPtr)0)
    {
        tm_ip_copy(TM_IP_ZERO, *bitMaskPtr);
        if (bytes != sizeof(tt4IpAddress))
        {
            ucp2 = (tt8BitPtr)bitMaskPtr + bytes;
            *ucp2 = (tt8Bit)( (TM_RTN_HIGH_BIT >> bits) & 0xFF );
        }
    }
#endif /* ! TM_DSP */
    bits = (tt16Bit)(bits + tm_bytes_to_bits(bytes));
    return bits;
}

#endif /* TM_LITTLE_ENDIAN */

#ifdef TM_PMTU_DISC
/*
 * tfRtPickNextPlateauMtu function description
 * Find next Path MTU to try.
 * Called with flag parameter set to TM_PMTU_LOWER, if we just have received
 * an ICMP error message with a message size error because we had set the
 * DF bit in the original message, and the IP datagram was too big, and the
 * router sending the ICMP message did not set the next hop MTU field.
 * We need to pick as the next PMTU estimate the greatest plateau value
 * that is less than the current MTU.
 * Called with the flag parameter set to TM_PMTU_HIGHER, if we have
 * successfully sent IP datagrams without occuring fragmentation for
 * the timeout period, and we want to try the next higher Path MTU
 * estimate at the end of the timeout period.
 * Parameters
 * Parameter       Description
 * currentMtu      Current IP Path MTU estimate used
 * flag            either TM_PMTU_LOWER or TM_PMTU_HIGHER
 * Return value
 * Next IP MTU to try
 */
static tt16Bit
tfRtPickNextPlateauMtu(tt16Bit currentMtu, tt8Bit flag)
{
    int     plateauIndex;

    plateauIndex = 0;
/*
 * Find the index to the greatest plateau value that is less or equal
 * than the current MTU.
 */
    while (plateauIndex < TM_PMTU_ENTRIES)
    {
        if (currentMtu >= tlMtuPlateauValues[plateauIndex])
        {
            break;
        }
        plateauIndex++;
    }
/* Pick next plateau MTU to try (lower or higher) */
    if (flag == TM_PMTU_LOWER)
/*
 * Pick as the next PMTU estimate the greatest plateau value that is less
 * than the current MTU.
 */
    {
/*
 * If in the while loop above, we did not find a lower MTU plateau value
 * (plateauIndex == TM_PMTU_ENTRIES), do nothing. We will just return
 * the null entry at the end of the table.
 */
        if (plateauIndex < TM_PMTU_ENTRIES)
/*
 * If in the while loop above, we found the entry that is less or equal than
 * the current MTU.
 */
        {
            if (currentMtu == tlMtuPlateauValues[plateauIndex])
/*
 * If the entry is the same as the current MTU, pick the
 * next value down by increasing  the index. No need to check
 * for table overflow since plateauIndex is less than TM_PMTU_ENTRIES
 * and the table goes from 0 to TM_PMTU_ENTRIES.
 */
            {
                plateauIndex++;
            }
/* 
 * If it is less, do nothing, we will just return that lesser value at the
 * current index.
 */
        }
    }
    else
/* Pick next higher MTU */
    {
        if (plateauIndex != 0)
        {
            plateauIndex--;
        }
        else
        {
/*
 * If we are currently at the biggest possible MTU. There is no new MTU
 * to try. Just return 0, so that we know we do not need try a new value.
 * (Table index goes from 0, to TM_PMTU_ENTRIES, and value at index
 *  TM_PMTU_ENTRIES is 0.)
 */
            plateauIndex = TM_PMTU_ENTRIES;
        }
    }
    return tlMtuPlateauValues[plateauIndex];
}
#endif /* TM_PMTU_DISC */

#if (defined(TM_PMTU_DISC) || defined(TM_6_PMTU_DISC))
/*
 * tfRtAddPathMtuEntry function description
 * Add a new host path MTU routing entry.
 * . If routing entry is host routing entry, delete it.
 * . Add new host routing with timeout value, and flags parameters
 * . Get new routing entry, and check that it is a host entry
 * . If nextMtu is non zero, set the routing entry MTU with it.
 * Parameters
 * Parameter    Description
 * rtePtr       Pointer to current routing entry to destination IP address.
 * timeout      Timeout value of new routing entry to insert.
 * flags        Flags value of new routing entry to insert.
 * nextMtu   New Path MTU to use
 *
 * Return values 
 * TM_ENOERROR      New host path MTU routing entry was added
 * TM_EHOSTUNREACH  Destination IP address unreachable
 * TM_ENOBUFS       Not enough memory to allocate new routing entry
 * TM_ENOENT        Could not insert host entry
 */
static int tfRtAddPathMtuEntry( 
#ifdef TM_USE_STRONG_ESL
                                ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                                ttRteEntryPtr    rtePtr,
#ifdef TM_USE_IPV6
                                ttIpAddressPtr   dest6IpAddrPtr,
#else /* !TM_USE_IPV6 */
                                ttIpAddress      destIpAddress,
#endif /* TM_USE_IPV6 */
                                tt32Bit          timeout,
                                tt16Bit          flags,
                                tt16Bit          nextMtu )
{
    ttDeviceEntryPtr    rteDevPtr;
#ifdef TM_USE_IPV6
    ttIpAddress         gway6Addr;
#else /* !TM_USE_IPV6 */
    ttIpAddress         gwayAddress;
#endif /* TM_USE_IPV6 */
    int                 errorCode;
    tt16Bit             rteMhomeIndex;

    rteDevPtr = rtePtr->rteDevPtr;
    rteMhomeIndex = rtePtr->rteMhomeIndex;
#ifdef TM_USE_IPV6
    tm_6_ip_copy_structs(rtePtr->rteGSGateway, gway6Addr);
#else /* !TM_USE_IPV6 */
    tm_ip_copy(rtePtr->rteGSGateway, gwayAddress);
#endif /* TM_USE_IPV6 */
    if ( tm_16bit_all_bits_set( rtePtr->rteFlags2,
                                TM_RTE2_HOST | TM_RTE2_UP) )
/*
 * If the current entry is a host entry, remove it.
 */
    {
        tfRtRemoveEntry(rtePtr);
    }
/* Entry added with device MTU */
#ifdef TM_USE_IPV6
    errorCode = tf6RtAddRoute( rteDevPtr,
                               dest6IpAddrPtr,
                               TM_6_IP_HOST_PREFIX_LENGTH,
                               &gway6Addr,
                               TM_16BIT_ZERO, /* tag */
                               rteMhomeIndex, 1,
                               timeout,
                               flags,
/* no need to lock */
                               TM_8BIT_ZERO);
#else /* !TM_USE_IPV6 */
    errorCode = tf4RtAddRoute( rteDevPtr,
                               destIpAddress,
                               TM_IP_HOST_MASK,
                               gwayAddress,
                               TM_16BIT_ZERO, /* tag */
                               rteMhomeIndex, 1,
                               timeout,
                               flags,
/* no need to lock */
                               TM_8BIT_ZERO);
#endif /* TM_USE_IPV6 */
    if (errorCode == TM_ENOERROR)
    {
#ifdef TM_USE_IPV6
        rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                          devPtr,
#endif /* TM_USE_STRONG_ESL */
                          dest6IpAddrPtr);
#else /* !TM_USE_IPV6 */
        rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                          devPtr,
#endif /* TM_USE_STRONG_ESL */
                          destIpAddress);
#endif /* !TM_USE_IPV6 */
        errorCode = TM_EPERM;
        if (rtePtr != TM_RTE_NULL_PTR)
        {
            if ( tm_16bit_all_bits_set( rtePtr->rteFlags2,
                                        TM_RTE2_HOST | TM_RTE2_UP ) )
            {
                if (nextMtu != 0)
/*
 * If User did not set the MTU, use default MTU (device MTU), otherwise
 * set the MTU to the parameter value.
 */
                {
                    rtePtr->rteMtu = nextMtu;
                }
                errorCode = TM_ENOERROR;
            }
        }
    }
    return errorCode;
}
#endif /* TM_PMTU_DISC || TM_6_PMTU_DISC */


/*
 * tfIpResetCaches function description:
 * Called by the router code with TM_RTE_INDIRECT, or TM_RTE_CLONED rteFlag, 
 * when the IP send and IP forward routing cache entries need to be reset, 
 * because a new route has been added (TM_RTE_INDIRECT), or because there is 
 * no more room in the ARP cache (TM_RTE_CLONED).
 * lockFlag can have two possible values TM_LOC_WAIT or TM_LOC_NOWAIT.
 * TM_LOC_WAIT
 *   If lockFlag is TM_LOCK_WAIT, then this function is called with the 
 *   routing tree unlocked, and not in the send/forward path. It is safe
 *   to lock the IP routing caches.
 * TM_LOC_NOWAIT
 *   . If lockFlag is TM_LOC_NOWAIT, then this function is called with the
 *     routing tree locked. If the routing tree is locked it is unsafe to 
 *     lock an IP forward or IP route cache. (example: send task locking
 *     an IP routing cache, and locking the tree; app task adding a route
 *     entry which calls this function with the tree locked)
 *   . If lockFlag is TM_LOC_WAIT, then this function could be called
 *     in the send/forward path with an IP routing cache already locked,
 *     resulting in a deadlock if we attempt to lock the same lock again.
 *   . To prevent deadlocks, if lockFlag is set to TM_LOC_NOWAIT,
 *     we will not attempt to reset any IP routing cache that is already 
 *     locked. 
 */
void tfIpResetCaches (  tt16Bit           rteFlag
                      , int               addressFamilyFlag
#ifdef TM_LOCK_NEEDED
                      , tt8Bit            lockFlag
#endif /* TM_LOCK_NEEDED */
                      )
{
    ttRtCacheEntryPtr rtcPtr;
    tt8Bit            cacheEntries;
#ifdef TM_LOCK_NEEDED
    ttLockEntryPtr    lockEntryPtr;
#endif /* TM_LOCK_NEEDED */

#ifdef TM_USE_IPV4
    if (addressFamilyFlag & AF_INET)
    {
/* First reset IPv4 send cache */
#ifdef TM_LOCK_NEEDED
        lockEntryPtr = &tm_context(tvRtIpSendCacheLock);
#endif /* TM_LOCK_NEEDED */
        rtcPtr = &tm_context(tvRtIpSendCache);
        cacheEntries = TM_8BIT_ZERO;
        do
        {
            if (rtcPtr->rtcRtePtr != TM_RTE_NULL_PTR)
            {
#ifdef TM_LOCK_NEEDED
                if (   (tvLockEnabled == TM_8BIT_ZERO)
                    || (tfLock(lockEntryPtr, lockFlag) == TM_LOC_OKAY) )
#endif /* TM_LOCK_NEEDED */
                {
                    tfRtCacheReset(rtcPtr, rteFlag);
                    tm_call_unlock(lockEntryPtr);
                }
            }
/* Second reset IP forward cache */
            rtcPtr = &tm_context(tvRtIpForwCache);
#ifdef TM_LOCK_NEEDED
            lockEntryPtr = &tm_context(tvRtIpForwCacheLock);
#endif /* TM_LOCK_NEEDED */
            cacheEntries++;
        } while (cacheEntries == (tt8Bit)1);
    }
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
    if (addressFamilyFlag & AF_INET6)
    {
/* First reset IPv6 send cache */
#ifdef TM_LOCK_NEEDED
        lockEntryPtr = &tm_context(tv6RtIpSendCacheLock);
#endif /* TM_LOCK_NEEDED */
        rtcPtr = &tm_context(tv6RtIpSendCache);
        cacheEntries = TM_8BIT_ZERO;
        do
        {
            if (rtcPtr->rtcRtePtr != TM_RTE_NULL_PTR)
            {
#ifdef TM_LOCK_NEEDED
                if (   (tvLockEnabled == TM_8BIT_ZERO)
                    || (tfLock(lockEntryPtr, lockFlag) == TM_LOC_OKAY) )
#endif /* TM_LOCK_NEEDED */
                {
                    tfRtCacheReset(rtcPtr, rteFlag);
                    tm_call_unlock(lockEntryPtr);
                }
            }
/* Second reset IP forward cache */
            rtcPtr = &tm_context(tv6RtIpForwCache);
#ifdef TM_LOCK_NEEDED
            lockEntryPtr = &tm_context(tv6RtIpForwCacheLock);
#endif /* TM_LOCK_NEEDED */
            cacheEntries++;
        } while (cacheEntries == (tt8Bit)1);
    }
#endif /* TM_USE_IPV6 */

    return;
}

/* 
 * Called by the tf{4,6}RtAddxxx() functions. 
 * Lock the tree if needed an allocate a routing entry, Initializing the
 * owner count to 1. 
 */
#ifdef TM_USE_IPV6
static ttRteEntryPtr tf6RtAllocNLockNCopy(tt8Bit               needLock, 
                                          tt4IpAddress         ip4Address,
                                          tt6ConstIpAddressPtr ip6AddrPtr)
#else /* TM_USE_IPV6 */
static ttRteEntryPtr tfRtAllocNLockNCopy(tt8Bit          needLock)
#endif /* TM_USE_IPV6 */
{
    ttRteEntryPtr       rtePtr;

#ifdef TM_ERROR_CHECKING
/* Check that routing tree has been initialized, and is not corrupted */
    if (tm_context(tvRtTreeHeadPtr) == TM_RADIX_NULL_PTR)
    {
        tfKernelError("tfRtAllocNLockNCopy",
                      "Routing table corrupted or not initialized");
        tm_thread_stop;
    }
#endif /* TM_ERROR_CHECKING */
/* Allocate a new routing entry */
    rtePtr = (ttRteEntryPtr)tm_recycle_alloc( (unsigned)sizeof(ttRteEntry),
                                              TM_RECY_RTE );

    if (rtePtr != TM_RTE_NULL_PTR)
    {
/* Brand new entry. Initialize owner count */
        rtePtr->rteOwnerCount = 1; /* no need to be critical here */
#ifdef TM_SNMP_CACHE
/* Initialize SNMPD/NSTAT cache index */
        rtePtr->rteSnmpCacheEntry.sceTableIndex = TM_SNMPC_NO_INDEX;
#endif /* TM_SNMP_CACHE */
/* Copy destination IP address */
#ifdef TM_USE_IPV6
        if (ip6AddrPtr == (tt6ConstIpAddressPtr)0)
        {
            tm_6_addr_to_ipv4_mapped(ip4Address, &(rtePtr->rteDest));
        }
        else
        {
            tm_6_ip_copy_dststruct(ip6AddrPtr, rtePtr->rteDest);
        }
#endif /* TM_USE_IPV6 */
/* LOCK the tree to obtain exclusive access to the routing table */
        if (needLock)
        {
            tm_call_lock_wait(&tm_context(tvRtTreeLock));
        }
    }
    return rtePtr;
}

/*
 * Called by the tf{4,6}RtAddxxx() functions. 
 * Insert entry in SNMP routing cache, if insertion to Patricia tree succeeded.
 * UNLOCK the tree to release access to the routing table, if needed.
 * Free routing entry, if insertion failed. 
 * Ignore error for local route if errorCode is TM_EALREADY, since local
 * routes is for the same interface.
 */
static int tfRtCacheUnlockNClean ( ttRteEntryPtr rtePtr,
                                   tt8Bit needUnLock, int errorCode,
                                   tt16Bit flags )
{
    tt8BitPtr        destLinkPhysAddrPtr;
#ifndef TM_USE_IPV6
    ttIpAddress      ipAddress;
#endif /* TM_USE_IPV6 */
    int              retCode;
    
    retCode = errorCode;
#ifdef TM_SNMP_CACHE
    if (errorCode == TM_ENOERROR)
    {
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
        tm_kernel_set_critical;
        rtePtr->rteLastUpdated = (tt32Bit)tvTime;
        tm_kernel_release_critical;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
        tfSnmpdCacheInsertRoute(rtePtr, TM_SNMPC_ROUTE_INDEX);
    }
#endif /* TM_SNMP_CACHE */

/*
 * special case for local route entry, to handle multiple addresses configured
 * on the same interface with the same prefix (subnet):  The original
 * local route entry has already been added, but we need to add a loopback host
 * route entry for this new address, since the normal clone logic (tfRtClone)
 * cannot handle this case.  This entry will be removed when the address is
 * unconfigured.
 */
    if (    tm_16bit_one_bit_set(flags, TM_RTE_LOCAL)
         && (errorCode == TM_EALREADY) )
    {
#ifdef TM_USE_IPV6
        tm_6_ip_copy_structs(rtePtr->rteHSDevIpAdd,
                             rtePtr->rteDest);
#else /* !TM_USE_IPV6 */
        tm_ip_copy(rtePtr->rteHSDevIpAdd, ipAddress);
#endif /* TM_USE_IPV6 */
        retCode = TM_ENOERROR;
        errorCode = tfRtAddEntry( tm_context(tvRtTreeHeadPtr),
                                  rtePtr,
                                  rtePtr->rteOrigDevPtr,
#ifdef TM_USE_IPV6
                                  TM_6_IP_HOST_PREFIX_LENGTH,
#else /* !TM_USE_IPV6 */
                                  ipAddress,
                                  TM_IP_HOST_MASK,
#endif /* TM_USE_IPV6 */
                                  rtePtr->rteOrigMhomeIndex,
                                  (int) rtePtr->rteHops,
                                  TM_RTE_INF,
                                  TM_RTE_STATIC | TM_RTE_LINK_LAYER);
        if (    (errorCode == TM_ENOERROR)
             && tm_16bit_one_bit_set(flags, TM_RTE_CLONE) )
        {
/* 
 * So that we do get a NO ARP entry kernel error in tfIpSendPacket(). Also
 * done after tfRtAddEntry() so that we do not add an ARP timer.
 */
            rtePtr->rteFlags = (tt16Bit)(rtePtr->rteFlags | TM_RTE_CLONED);
            rtePtr->rteFlags2 = (tt16Bit)
                            (rtePtr->rteFlags2 | TM_RTE2_LOCAL_DUPLICATE);
            destLinkPhysAddrPtr = &(rtePtr->rteHSPhysAddr[0]);
/* Sending to our own IP address. Store link layer address. */
            tm_bcopy(rtePtr->rteOrigDevPtr->devPhysAddrArr,
                     destLinkPhysAddrPtr,
                     rtePtr->rteHSPhysAddrLen);
        }
#if ( !(defined(TM_SINGLE_INTERFACE_HOME)) && !(defined(TM_LOOP_TO_DRIVER)) )
        rtePtr->rteDevPtr = tm_context(tvLoopbackDevPtr);
#endif /* !defined(TM_SINGLE_INTERFACE_HOME) && !defined(TM_LOOP_TO_DRIVER) */
#ifdef TM_SNMP_CACHE
        if (errorCode == TM_ENOERROR)
        {
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
            tm_kernel_set_critical;
            rtePtr->rteLastUpdated = (tt32Bit)tvTime;
            tm_kernel_release_critical;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
            tfSnmpdCacheInsertRoute(rtePtr, TM_SNMPC_ROUTE_INDEX);
        }
#endif /* TM_SNMP_CACHE */
    }
    
    if (needUnLock)
    {
        tfRtTreeUnlockNResetCaches();
    }
    if (errorCode != TM_ENOERROR)
    {
/* routing entry could not be inserted. Recycle it */
#ifdef TM_ERROR_CHECKING
/* Keep track of out of tree non recycled entries */
        tm_context(tvRteLimboCount)++;
#endif /* TM_ERROR_CHECKING */
        tfRtUnGet(rtePtr);
    }
    return retCode;
}

/* 
 * Add an ARP entry. Common code between IPv4 and IPv6.
 * IPv4: called from tf4RtArpAddEntry which is itself called when a user
 * adds an ARP entry, or in ARP incoming processing.
 * IPv6: Called from tf6RtArpAddEntry which is itself called when a user
 * adds an ARP entry.
 */
static int tfRtArpAddEntry ( 
    ttRtCacheEntryPtr  rtcCachePtr,
    tt8BitPtr          physAddrPtr,
    tt32Bit            ttl,
    tt8Bit             physAddrLen 
                )
{
    ttRteEntryPtr   rtePtr;
    int             errorCode;

    errorCode = tfRtGet(rtcCachePtr);
    if (errorCode == TM_ENOERROR)
    {
        rtePtr = rtcCachePtr->rtcRtePtr;
        if (tm_16bit_bits_not_set(rtePtr->rteFlags, TM_RTE_STATIC))
        {
            errorCode = tfRtArpAddByRte(rtePtr, physAddrPtr, ttl, physAddrLen);
            if (errorCode == TM_ENOERROR)
            {
                if (ttl == TM_RTE_INF)
/* Permanent ARP entry */
                {
                    rtePtr->rteFlags |= TM_RTE_STATIC;
                }
            }
        }
/*
 * Release ownership, since we are not keeping the cache around
 */
        tfRtCacheUnGet(rtcCachePtr);
    }
    return errorCode;
}

/*
 * Delete a route. Called by tf6RtDelRoute() or tf4RtDelRoute()
 */
static int tfRtDelRoute(ttRteEntryPtr rtePtr, tt16Bit rteCreateFlag)
{
    ttRteEntryPtr       prevRtePtr;
    int                 errorCode;

    errorCode = TM_EPERM;
    prevRtePtr = TM_RTE_NULL_PTR;
    do
    {
        if (    tm_16bit_bits_not_set( rtePtr->rteFlags, TM_RTE_ARP )
             && tm_16bit_one_bit_set(rtePtr->rteFlags, rteCreateFlag) )
/*
 * If entry is not ARP entry, and deletor is identical to creator,
 * delete the entry.
 */
        {
            errorCode = TM_ENOERROR;
            if (prevRtePtr != TM_RTE_NULL_PTR)
            {
                if (tm_16bit_one_bit_set(prevRtePtr->rteFlags, TM_RTE_ARP))
/* Previous entry at same key is ARP clone, remove it */
                {
                    tfRtRemoveEntry(prevRtePtr);
                }
            }
/* Delete entry */
            tfRtRemoveEntry(rtePtr);
            break;
        }
        else
        {
            prevRtePtr = rtePtr;
            rtePtr = tm_rt_leaf_to_rte(
                                tm_rt_next_dup(tm_rt_rte_to_leaf(prevRtePtr)));
        }
    } while (rtePtr != (ttRteEntryPtr)0);
    return errorCode;
}

/* 
 * IPv6 functions
 */
#ifdef TM_USE_IPV6
/* IPV6 call only */
/* Called by tfNgConfigInterface to add a local route */
int tf6RtAddLocal( ttDeviceEntryPtr     devPtr,
/* local destination network for LAN, Remote IP address for SLIP/PPP */
                                  ttIpAddressPtr        dest6IpAddrPtr,
/* Network mask of local network (All one's for SLIP/PPP) */
                                  int                   myNetPrefixLength,
/* IP address of the interface */
                                  ttIpAddressPtr        my6IpAddrPtr,
/* Multihome index of the interface */
                                  tt16Bit               myMhomeIndex )
{
    int                 is_lan;
    int                 errorCode;
    tt16Bit             flags;

    is_lan = tm_6_ll_is_lan(devPtr);
#ifdef TM_ERROR_CHECKING
/* Loop back should be on loop back interface */
    if (    (    (tm_6_addr_ipv4_mapped_loopback(my6IpAddrPtr))
              || (IN6_IS_ADDR_LOOPBACK(my6IpAddrPtr)) )
         && ( devPtr != tm_context(tvLoopbackDevPtr) ) )
    {
        errorCode = TM_EINVAL;
    }
    else
#endif /* TM_ERROR_CHECKING */
    {
/* Allow cloning for local LAN link layer address. */
        if ( is_lan && (myNetPrefixLength < TM_6_IP_HOST_PREFIX_LENGTH) ) 
        {
            flags = TM_RTE_CLONE | TM_RTE_LOCAL;
        }
        else
        {
            flags = TM_RTE_LOCAL;
        }
        errorCode = tf6RtAddRoute( devPtr,
                                   dest6IpAddrPtr,
                                   myNetPrefixLength,
                                   my6IpAddrPtr,
                                   TM_16BIT_ZERO,
                                   myMhomeIndex, 0, TM_RTE_INF,
                                   flags, TM_8BIT_YES );
        if (errorCode == TM_ENOERROR)
        {
            tf6RtDefRouterTag(
#ifdef TM_USE_STRONG_ESL
                              devPtr
#endif /* TM_USE_STRONG_ESL */
                             );
        }
    }
    return errorCode;
}

/*
 * IPV6 call only.
 * Called by tfNgDelDefaultGateway, or by tfNgDelStaticRoute, or by
 * tfNgUncongigInterface to delete a route
 */
int tf6RtDelRoute(
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr    devPtr,
#endif /* TM_USE_STRONG_ESL */
    ttIpAddressPtr      dest6IpAddrPtr,
    int                 prefixLength,
    tt16Bit             rteCreateFlag )
{
    ttRteEntryPtr rtePtr;
    int           errorCode;

    errorCode = TM_ENOERROR;
#ifdef TM_ERROR_CHECKING
    if (tm_context(tvRtTreeHeadPtr) == TM_RADIX_NULL_PTR)
    {
        tfKernelError("tf6RtDelRoute",
                      "Routing table corrupted or not initialized");
/* Severe Error */
        tm_thread_stop;
    }
#endif /* TM_ERROR_CHECKING */
/* LOCK the tree to obtain exclusive access to the routing table */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));
/*
 * Check whether this IP address/network mask is in the routing table.
 */
    rtePtr = tfRtFindIpAddrIpNet(
#ifdef TM_USE_STRONG_ESL
                                 devPtr,
#endif /* TM_USE_STRONG_ESL */
                                 dest6IpAddrPtr, prefixLength);
    if (rtePtr != TM_RTE_NULL_PTR)
/* If entry already exists in the routing table */
    {
        errorCode = tfRtDelRoute(rtePtr, rteCreateFlag);
    }
    else
    {
        errorCode = TM_ENOENT;
    }
/* UNLOCK the tree to release access to the routing table */
    tfRtTreeUnlockNResetCaches();
    return errorCode;
}

/*
 * tf6RtAddRoute function description.
 * IPV6 call only 
 * Add a non local route to the routing tree.
 * Called by tfNgAddStaticRoute(), and other to lock the routing
 * table, allocate a new router entry and insert it in the routing table
 * 1) Call tfRtAllocNLockNCopy to allocate a new routing entry, lock
 *    the Patricia tree, and copy the IP address in the routing entry
 *    destination field.
 * 2) Copy gateway address in routing entry
 * 3) call tfRtAddEntry() to insert the entry in the Patricia tree.
 * 4) Call tfRtCacheUnlockNClean to cache the entry in the SNMP routing
 *    cache, unlock the tree, and unget the routing entry if we failed
 *    to insert in the tree.
 * Parameters
 * devPtr: device for the route 
 * ipAddr: Destination Ip address 
 * netMask: destination network address 
 * mHomeIndex: multihome index for local routes
 * gateway: Gateway IP address for non local routes, interface IP address
 *          for local routes.
 * hops: number of hops to destination (0 for local route)
 * ttl: Time to live for the route entry
 * flags: flag to indicate creator local/static/redirect/RIP, host route etc.
 *
 * Return value:
 * O on success
 * TM_ENOBUFS, when routing entry could not be allocated
 */
int tf6RtAddRoute(
/* route device */
                                 ttDeviceEntryPtr       devPtr,
/* Destination Ip address */
                                 ttIpAddressPtr         ip6AddrPtr,
/* destination network address */
                                 int                    prefixLength,
/* Gateway for non local routes, interface IP address for local routes. */
                                 ttIpAddressPtr         gway6AddrPtr,
/* Gateway tag (for RIP) */
                                 tt16Bit                gwayTag,
/* multihome index */
                                 tt16Bit                mHomeIndex,
/* metrics for non local routes */
                                 int                    hops,
                                 tt32Bit                ttl,
/* flag to indicate owner local/static/redirect/RIP */
                                 tt16Bit                flags,
                                 tt8Bit                 needLock )
{
    ttRteEntryPtr       rtePtr;
    int                 errorCode;

/* Allocate routing entry, and initialize destination IP address */
    rtePtr = tf6RtAllocNLockNCopy(needLock, TM_IP_ZERO, ip6AddrPtr);

    if (rtePtr != TM_RTE_NULL_PTR)
    {
        rtePtr->rteHSPhysAddrLen = devPtr->devPhysAddrLength;
        if (tm_16bit_one_bit_set(flags, TM_RTE_LOCAL))
        {
            tm_6_ip_copy_dststruct(gway6AddrPtr, rtePtr->rteHSDevIpAdd);
#ifdef TM_6_USE_PREFIX_DISCOVERY
            rtePtr->rte6HSPreferredLifetime = TM_RTE_INF;
#endif /* TM_6_USE_PREFIX_DISCOVERY */
        }
        else
        {
            tm_6_ip_copy_dststruct(gway6AddrPtr, rtePtr->rteGSGateway);
            rtePtr->rteGSTag = gwayTag;
        }
/* Need to protect the IPv6 MTU, since it could be changed dynamically
   due to a router advertisement. */
        tm_kernel_set_critical;
        rtePtr->rteMtu = devPtr->dev6Mtu;
        tm_kernel_release_critical;
        
        errorCode = tfRtAddEntry(
                            tm_context(tvRtTreeHeadPtr), rtePtr, devPtr,
                            prefixLength,
                            mHomeIndex,
                            hops, ttl, flags );
/*
 * Unlock the tree, cache entry if successful or free routing entry otherwise.
 */
        errorCode = tfRtCacheUnlockNClean(rtePtr, needLock, errorCode, flags);
    }
    else
    {
        errorCode =  TM_ENOBUFS;
    }
    return errorCode;
}

/*
 * tf6RtAddHost function description:
 * Called by tfNgAddHostRoute()
 * to add a host entry to the routing table (Patricia tree) to set
 * a particular destination interface for a given multicast address. If the
 * physical address pointer is not NULL, the link layer address will also
 * be copied into the new entry.  Note that it is still the caller's
 * responsibility to set the TM_RTE_LINK_LAYER flag.
 * 1) Call tf6RtAllocNLockNCopy to allocate a new routing entry, lock
 *    the Patricia tree, and copy the IP address in the routing entry
 *    destination field.
 * 2) Copy physical address in routing entry
 * 3) call tfRtAddEntry() to insert the entry in the Patricia tree.
 * 4) Call tfRtCacheUnlockNClean to cache the entry in the SNMP routing
 *    cache, unlock the tree, and unget the routing entry if we failed
 *    to insert in the tree.
 * Parameters
 * devPtr: device for the route 
 * ipAddr: Destination Ip address 
 * physaddrPtr: pointer to physical address (possibly NULL)
 * physAddrLength: length of the physical address
 * mHomeIndex: multihome index 
 * ttl: Time to live for the route entry
 * flags: flag to indicate creator local/static/redirect/RIP, host route etc.
 * needLock: Boolean specifying whether to lock the routing tree
 * 
 * Return value:
 * O on success
 * TM_ENOBUFS, when routing entry could not be allocated
 */
int tf6RtAddHost(
/* route device */
                  ttDeviceEntryPtr       devPtr,
/* Destination Ip address */
                  tt6ConstIpAddressPtr   ip6AddrPtr,
/* physical address */
                  tt8BitPtr              physAddrPtr,
/* physical address len */
                  tt8Bit                 physAddrLen,
/* multihome index */
                  tt16Bit                mhomeIndex,
                  tt32Bit                ttl,
/* flag to indicate cloned/static/link-layer/mcast */
                  tt16Bit                flags,
                  tt8Bit                 needLock )
{
    ttRteEntryPtr       rtePtr;
    int                 errorCode;

/* Allocate routing entry, and initialize destination IP address */
    rtePtr = tf6RtAllocNLockNCopy(needLock, TM_IP_ZERO, ip6AddrPtr);
    if (rtePtr != TM_RTE_NULL_PTR)
    {
        if (physAddrPtr != TM_8BIT_NULL_PTR)
        {
            tm_bcopy(physAddrPtr, rtePtr->rteHSPhysAddr, (unsigned)physAddrLen);
            rtePtr->rteHSPhysAddrLen = physAddrLen;
        }
/* Need to protect the IPv6 MTU, since it could be changed dynamically
   due to a router advertisement. */
        tm_kernel_set_critical;
        rtePtr->rteMtu = devPtr->dev6Mtu;
        tm_kernel_release_critical;
        
        errorCode = tfRtAddEntry(
                            tm_context(tvRtTreeHeadPtr), rtePtr, devPtr,
                            TM_6_IP_HOST_PREFIX_LENGTH,
                            mhomeIndex,
                            0, ttl, flags );
/*
 * Unlock the tree, cache entry if successful or free routing entry otherwise.
 */
        errorCode = 
                tfRtCacheUnlockNClean(rtePtr, needLock, errorCode, flags);
    }
    else
    {
        errorCode =  TM_ENOBUFS;
    }
    return errorCode;
}

/*
 * tf6RtAddDefRouter Function Description:
 * Add an IPv6 default router to the list of router entries and possibly
 * the routing tree, or update ttl if already present. It updated ttl
 * is zero, remove entry from the tree.
 * 0. Lock the tree
 * 1. Look up the list of already inserted default router entries to
 *    find a match.
 * 2. If a match is found, update the ttl, and if the ttl is zero
 *    delete the entry from the list and the tree if entry is in the tree.
 * 3. If a match is not found, and we have not exceeded the maximum number
 *    of IPv6 default router entries, and ttl is not zero:
 *  3.a Allocate a new default IPv6 router entry.
 *  3.b Initialize new default IPv6 router entry, with passed parameters.
 *  3.c insert at the end of the list.
 *  3.d If there is no default IPv6 router entry in the tree, insert
 *      the new entry in the tree, i.e. tf6RtDefRouterAddByDrt().
 * 4. Unlock the tree
 *
 * Parameters
 * ip6AddrPtr       Pointer to IPv6 address.
 * devPtr           Pointer to interface associated with the route.
 * ttl              Router lifetime in seconds.
 * flags            TM_DRT6_TEMPORARY
 *
 * Returns
 * TM_ENOERROR      success
 * TM_EHOSTUNREACH  destination unreachable
 * TM_ENOENT        ttl is zero, but there is no default router entry to delete
 * TM_ENOBUFS       Not enough memory to allocate a new default gateway entry
 *                  or new router entry, or list of default routers is full.
 */
int tf6RtAddDefRouter ( tt6IpAddressPtr     ip6AddrPtr,
                        ttDeviceEntryPtr    devPtr,
                        tt32Bit             ttl,
                        tt16Bit             flags )
{
/* Pointer to IPV6 default router structure */
    tt6RtDefRouterHeadPtr  def6HeadPtr;
/* New entry */
    tt6RtDefRouterEntryPtr def6RtrPtr;
    tt6RtDefRouterEntryPtr def6InTreeRtrPtr;
/* Pointer to entry that is in the tree */
    ttRteEntryPtr          def6RtrRtePtr;
    ttListPtr              def6HeadListPtr;
    ttGenericUnion         genParam;
#ifdef TM_USE_STRONG_ESL
    tt6RtDevGw             rt6DevGwStruct;
#endif /* TM_USE_STRONG_ESL */
    ttVoidPtr              rt6VoidPtr;
    int                    errorCode;
    int                    listIndex;
    tt8Bit                 found;

    errorCode = TM_ENOERROR;
    found = TM_8BIT_ZERO;

    if (ttl != TM_RTE_INF)
    {
        ttl = ttl * TM_UL(1000); /* convert to milliseconds */
    }
/* Assume no defaul gateway */
    def6InTreeRtrPtr = (tt6RtDefRouterEntryPtr)0;
/* 0. Lock the tree */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));
#ifdef TM_USE_STRONG_ESL
    def6HeadPtr = &(devPtr->dev6DefaultRouterHead);
#else /* !TM_USE_STRONG_ESL */
    def6HeadPtr = &(tm_context(tvRtRadixTreeHead).rth6DefaultRouterHead);
#endif /* !TM_USE_STRONG_ESL */
/* 1. Search the lists of default gateways to try and find a match */
    genParam.genVoidParmPtr = (ttVoidPtr)ip6AddrPtr;
    def6RtrPtr = tf6RtDefRouterWalk(
#ifdef TM_USE_STRONG_ESL
                                      devPtr,
#endif /* TM_USE_STRONG_ESL */
                                      tf6RtDefRouterGetByIpCB,
                                      genParam);
    if (def6RtrPtr != (tt6RtDefRouterEntryPtr)0)
    {
        def6RtrPtr->drt6Ttl = ttl;
        if (ttl == TM_UL(0))
        {
            (void)tf6RtDefRouterDelByDrt(
#ifdef TM_USE_STRONG_ESL
                                         devPtr,
#endif /* TM_USE_STRONG_ESL */
                                         def6RtrPtr);
        }
/* Found the default router we are trying to add/expire */
        found = TM_8BIT_YES;
    }
    else
    {
/* Find if there exists a default router */
        genParam.gen16BitParm = TM_16BIT_ZERO;
        def6InTreeRtrPtr = tf6RtDefRouterWalk(
#ifdef TM_USE_STRONG_ESL
                                      devPtr,
#endif /* TM_USE_STRONG_ESL */
                                      tf6RtDefRouterGetCB,
                                      genParam);
    }
    if (found == TM_8BIT_ZERO)
    {
        if (ttl != TM_UL(0))
        {
#ifdef TM_6_USE_TEMP_GWY
            if (tm_16bit_one_bit_set(flags, TM_DRT6_TEMPORARY))
            {
                listIndex = TM_6_DEF_TEMP_ROUTER_LIST;
            }
            else
#endif /* TM_6_USE_TEMP_GWY */
            {
                listIndex = TM_6_DEF_ROUTER_LIST; 
            }
            def6HeadListPtr = &def6HeadPtr->drh6RouterList[listIndex]; 
            if (def6HeadListPtr->listCount < TM_6_ROUTER_LIST_MAX_ENTRIES)
            {
/*
 * 3. If no match is found, and ttl is not zero, and there is room:
 */
/* 3.a ALlocate new entry */
                def6RtrPtr = (tt6RtDefRouterEntryPtr)
                             tm_get_raw_buffer(sizeof(tt6RtDefRouterEntry));
                if (def6RtrPtr != (tt6RtDefRouterEntryPtr)0)
                {
/* 3.b Initialize new entry */
                    tm_6_ip_copy_dststruct(ip6AddrPtr, def6RtrPtr->drt6IpAddr);
                    def6RtrPtr->drt6Ttl = ttl;
                    def6RtrPtr->drt6DevPtr = devPtr;
                    def6RtrPtr->drt6ListIndex = (tt8Bit)listIndex;
                    def6RtrPtr->drt6RtePtr = (ttRteEntryPtr)0;
/* Default */
                    def6RtrPtr->drt6Flags = flags;
#ifdef TM_USE_IPV4
                    if ( IN6_IS_ADDR_V4MAPPED(ip6AddrPtr) )
                    {
                        def6RtrPtr->drt6Flags |= TM_DRT6_V4_TUNNEL;
                    }
#endif /* TM_USE_IPV4 */
                    
/* 3.c Add at the end of the list */
                    tfListAddToTail(def6HeadListPtr, &def6RtrPtr->drt6Node);
                    if (    (ttl != TM_RTE_INF)
                         && (def6HeadPtr->drh6RouterTimerPtr == (ttTimerPtr)0) )
                    {
#ifdef TM_USE_STRONG_ESL
                        genParam.genVoidParmPtr = (ttVoidPtr)devPtr;
#endif /* TM_USE_STRONG_ESL */
                        
                        def6HeadPtr->drh6RouterTimerPtr = 
                                  tfTimerAdd( tf6RtDefRouterTimer,
                                              genParam,
                                              genParam, /* unused */
                                              tm_context(tvRtTimerResolution),
                                              TM_TIM_AUTO);
                    }
                    def6HeadPtr->drh6RouterCount++;

/*
 * Before attempting to insert a new default router in the tree, check if the
 * current default router is an IPv4 tunnel, or a temporary gateway that we
 * want to replace with a non temporary one. If so, remove it from the tree.
 * This ensures that a native IPv6 router always overrides an IPv4 default
 * tunnel, and that a non temporary IPv6 router always overrides a temporary
 * one.
 */ 
                    if (def6InTreeRtrPtr != (tt6RtDefRouterEntryPtr)0)
                    {
/* Current default router */
                        def6RtrRtePtr = def6InTreeRtrPtr->drt6RtePtr; /* non null */
                        if ( 
#ifdef TM_USE_IPV4
/* IPv4 tunnel */
                                (def6InTreeRtrPtr->drt6Flags & TM_DRT6_V4_TUNNEL)
                            ||
#endif /* TM_USE_IPV4 */
/*
 * Temporary gateway that we want to replace with a non temporary one, that is
 * not an IPv4 tunnel
 */
                                (     (   def6InTreeRtrPtr->drt6Flags
                                        & TM_DRT6_TEMPORARY)
                                  &&  (tm_16bit_bits_not_set(
                                         def6RtrPtr->drt6Flags,
                                        (TM_DRT6_TEMPORARY | TM_DRT6_V4_TUNNEL)))
                                )
                           )
                        {
                            if ( tm_16bit_one_bit_set( def6RtrRtePtr->rteFlags2, 
                                                       TM_RTE2_UP ) )
                            {
/* Remove from the tree */
                                tfRtRemoveEntry(def6RtrRtePtr);
                            }
                            tfRtUnGet(def6RtrRtePtr);
/* current default router has been removed */
                            def6RtrRtePtr = (ttRteEntryPtr)0;
/* No longer pointing to current default router as it has just been removed*/
                            def6InTreeRtrPtr->drt6RtePtr = TM_RTE_NULL_PTR;
                        }
                    }
                    else
                    {
/* No current default router */
                        def6RtrRtePtr = (ttRteEntryPtr)0;
                    }
/* 3.d Insert in tree, if there is no other def router in the tree */
                    if (def6RtrRtePtr == (ttRteEntryPtr)0)
                    {
                        errorCode = tf6RtDefRouterAddByDrt(def6RtrPtr);
                    }
                }
                else
                {
                    errorCode = TM_ENOBUFS;
                }
            }
            else
            {
                errorCode = TM_ENOBUFS;
            }
        }
        else
        {
            errorCode = TM_ENOENT;
        }
    }
    if ((ttl == 0) && (found == TM_8BIT_ZERO))
    {
#ifdef TM_USE_STRONG_ESL
        rt6DevGwStruct.dgwDevPtr = devPtr;
        rt6DevGwStruct.dgwIp6AddrPtr = ip6AddrPtr;
        rt6VoidPtr = &rt6DevGwStruct;
#else /* !TM_USE_STRONG_ESL */
        rt6VoidPtr = (ttVoidPtr)ip6AddrPtr;
#endif /* !TM_USE_STRONG_ESL */
        (void)tfRtTreeWalk( tfRtRemoveGwEntryCB, 
                            rt6VoidPtr, 
                            TM_8BIT_ZERO );
    }
/* 4. UNLOCK the tree to release access to the routing table */
    tfRtTreeUnlockNResetCaches();
    return errorCode;
}

/* 
 * tf6RtDelDefRouter function description
 * Remove the gateway with the given IPv6 address from the list of default
 * routers. 
 * 1. Lock the tree.
 * 2. Search the list of default routers for a match on the IP address.
 * 3. If a match is found remove it from the list, and possibly the tree
 *    by calling tf6RtDefRouterDelByDrt().
 * 4. Unlock the tree.
 * Parameters:
 * ip6AddrPtr   Pointer to IPV6 address
 * Returns
 * TM_ENOENT    Entry not in list of routing entries
 */
int tf6RtDelDefRouter (
#ifdef TM_USE_STRONG_ESL
                       ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                       tt6IpAddressPtr  ip6AddrPtr,
                       tt8Bit           needLock)
{
/* Entry in the list */
    tt6RtDefRouterEntryPtr def6RtrPtr;
    ttGenericUnion         genParam;
    int                    errorCode;

    errorCode = TM_ENOENT;
/* Lock the tree */
    if (needLock != TM_8BIT_ZERO)
    {
        tm_call_lock_wait(&tm_context(tvRtTreeLock));
    }
/* 1. Search the lists of default gateways to try and find a match */
    genParam.genVoidParmPtr = (ttVoidPtr)ip6AddrPtr;
    def6RtrPtr = tf6RtDefRouterWalk(
#ifdef TM_USE_STRONG_ESL
                                    devPtr,
#endif /* TM_USE_STRONG_ESL */
                                    tf6RtDefRouterGetByIpCB,
                                    genParam);
    if (def6RtrPtr != (tt6RtDefRouterEntryPtr)0)
    {
/* 
 * If a match is found: 
 * . remove default router from the list and from 
 *   the tree, if entry is in the tree. 
 * . Exit.
 */
        errorCode = tf6RtDefRouterDelByDrt(
#ifdef TM_USE_STRONG_ESL
                                            devPtr,
#endif /* TM_USE_STRONG_ESL */
                                            def6RtrPtr);
    }
/* UnLock the tree */
    if (needLock != TM_8BIT_ZERO)
    {
        tfRtTreeUnlockNResetCaches();
    }
    return errorCode;
}

/* 
 * tf6RtGetDefRouter function description
 * Searches the list of default routers for an entry with the specified flags.
 * Returns the IPv6 address of the first entry found that is active.  Called
 * with the routing tree unlocked.
 * 
 * 1. Lock the routing tree.
 * tm_call_lock_wait(&tm_context(tvRtTreeLock));
 * 2. If no entry is found, return TM_ENOENT.
 * 3. Scan through the (possibly empty) list of routers for an entry with the
 *    specified flags set.  If one is found, copy the address into ip6AddrPtr
 *    and return TM_ENOERROR.
 * 4. Unlock the routing tree.
 *    tfRtTreeUnlock(); 
 * 
 * Parameters:
 * ip6AddrPtr   When TM_ENOERROR is returned, this is filled in with the IPv6
 *              address of the first router in the list with the specified
 *              flags set.
 * flags        Flags to search for.
 * 
 * Returns
 * TM_ENOERROR  Default router found. address of router copied into *ip6AddrPtr.
 * TM_ENOENT    No default router with the specified flags was found.
 * 
 */
int tf6RtGetDefRouter(
#ifdef TM_USE_STRONG_ESL
                      ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                      tt6IpAddressPtr ip6AddrPtr,
                      tt16Bit         flags)
{
    tt6RtDefRouterEntryPtr  def6RtrPtr;    
    ttGenericUnion          genParam;
    int                     errorCode;
    
/* 1. Lock the routing tree. */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));
    genParam.gen16BitParm = flags;
    def6RtrPtr = tf6RtDefRouterWalk(
#ifdef TM_USE_STRONG_ESL
                                    devPtr,
#endif /* TM_USE_STRONG_ESL */
                                    tf6RtDefRouterGetCB,
                                    genParam);

    if (def6RtrPtr != (tt6RtDefRouterEntryPtr)0)
    {
        errorCode = TM_ENOERROR;
        tm_6_ip_copy( &(def6RtrPtr->drt6IpAddr),  ip6AddrPtr);
    }
    else
    {
/* 2. If no entry is found, return TM_ENOENT. */
        errorCode = TM_ENOENT;
    }
/* 4. Unlock the routing tree. */
    tfRtTreeUnlock();
    return errorCode;
}

/* 
 * tf6RtGetDefRouterDrt function description
 * Searches the list of default routers for an entry matching the IPv6 gateway
 * if it is not the IPv6 any address, otherwise for an entry matching the
 * specified flags,
 * Returns the first default gateway matching the criteria.  Called
 * with the routing tree locked.
 * 1. Set the parameter and call back function according to the criteria.
 * 3. Scan through the (possibly empty) list of routers for a matching
 *    gateway
 * 4. Return pointer to first default router matching the criteria.
 * 
 * Parameters:
 * ip6AddrPtr   Pointer to IPv6 address of the gateway that we are searching. 
 *              If IPv6 address is 0, we can match any gateway.
 * flags        Flags to search for if IPv6 address is any.
 * 
 * Returns
 * Pointer to first matching default gateway.
 * 
 */
tt6RtDefRouterEntryPtr tf6RtGetDefRouterDrt(
#ifdef TM_USE_STRONG_ESL
                      ttDeviceEntryPtr      devPtr,
#endif /* TM_USE_STRONG_ESL */
                      tt6ConstIpAddressPtr  ip6AddrPtr,
                      tt16Bit               flags)
{
    tt6RtDefRouterEntryPtr  def6RtrPtr;    
    tt6RtDefRouterWalkPtr   walkFuncPtr;
    ttGenericUnion          genParam;
    
    if (tm_6_ip_match((tt6IpAddressPtr)&in6addr_any,
                      ip6AddrPtr))
    {
        walkFuncPtr = tf6RtDefRouterGetAnyCB;
        genParam.gen16BitParm = flags;
    }
    else
    {
        walkFuncPtr = tf6RtDefRouterGetByIpCB;
        genParam.genVoidParmPtr = (ttVoidPtr)ip6AddrPtr;
    }
/* 1. Search the lists of default gateways to try and find a match */
    def6RtrPtr = tf6RtDefRouterWalk(
#ifdef TM_USE_STRONG_ESL
                                    devPtr,
#endif /* TM_USE_STRONG_ESL */
                                    walkFuncPtr,
                                    genParam);
    return def6RtrPtr;
}

tt6RtDefRouterEntryPtr tf6RtGetNextDefRouterDrt(
#ifdef TM_USE_STRONG_ESL
                      ttDeviceEntryPtr       devPtr,
#endif /* TM_USE_STRONG_ESL */
                      tt6RtDefRouterEntryPtr def6RtrPtr)
{
/* Head of circular list */
    tt6RtDefRouterHeadPtr   def6HeadPtr;
    ttListPtr               def6HeadListPtr;
    ttNodePtr               nodePtr;
    ttGenericUnion          genParam;
    int                     listIndex;

#ifdef TM_USE_STRONG_ESL
    def6HeadPtr = &(devPtr->dev6DefaultRouterHead);
#else /* !TM_USE_STRONG_ESL */
    def6HeadPtr = &(tm_context(tvRtRadixTreeHead).rth6DefaultRouterHead);
#endif /* !TM_USE_STRONG_ESL */
    nodePtr = (ttNodePtr)0;
    if (def6HeadPtr->drh6RouterCount != TM_16BIT_ZERO)
    {
        listIndex = def6RtrPtr->drt6ListIndex;
        def6HeadListPtr = &def6HeadPtr->drh6RouterList[listIndex]; 
        if (def6RtrPtr->drt6Node.nodeNextPtr !=
                                 &(def6HeadListPtr->listHeadNode))
        {
            nodePtr = def6RtrPtr->drt6Node.nodeNextPtr;
        }
        if (nodePtr == (ttNodePtr)0)
        {
            listIndex++;
            if (listIndex < TM_6_ACTIVE_DEF_ROUTER_LISTS)
            {
                genParam.gen16BitParm = TM_16BIT_ZERO;
                nodePtr = tf6RtDefRouterSubWalk(
                                def6HeadPtr,
                                tf6RtDefRouterGetAnyCB,
                                genParam,
                                listIndex);
            }
        }
    }
    return (tt6RtDefRouterEntryPtr)nodePtr;
}

static ttNodePtr tf6RtDefRouterSubWalk (
                      tt6RtDefRouterHeadPtr     def6HeadPtr,
                      tt6RtDefRouterWalkPtr     walkFuncPtr,
                      ttGenericUnion            genParam,
                      int                       listIndex )
{
    ttListPtr               def6HeadListPtr;
    ttNodePtr               nodePtr;

    nodePtr = (ttNodePtr)0;
    for (;
         listIndex < TM_6_ACTIVE_DEF_ROUTER_LISTS;
         listIndex++)
    {
        def6HeadListPtr = &def6HeadPtr->drh6RouterList[listIndex]; 
        if (def6HeadListPtr->listCount != 0)
        {
             nodePtr = tfListWalk(def6HeadListPtr, walkFuncPtr,
                                  genParam);
             if (nodePtr != (ttNodePtr)0)
             {
                break;
             }
        }
    }
    return nodePtr;
}

static tt6RtDefRouterEntryPtr tf6RtDefRouterWalk (
#ifdef TM_USE_STRONG_ESL
                      ttDeviceEntryPtr      devPtr,
#endif /* TM_USE_STRONG_ESL */
                      tt6RtDefRouterWalkPtr walkFuncPtr,
                      ttGenericUnion        genParam
                )
{
/* Head of circular list */
    tt6RtDefRouterHeadPtr   def6HeadPtr;
    tt6RtDefRouterEntryPtr  def6RtrPtr;
/*  
 * 3. Scan through the (possibly empty) list of routers for an entry with the
 *    specified flags set.  If one is found, copy the address into ip6AddrPtr
 *    and return TM_ENOERROR.
 */
#ifdef TM_USE_STRONG_ESL
    def6HeadPtr = &(devPtr->dev6DefaultRouterHead);
#else /* !TM_USE_STRONG_ESL */
    def6HeadPtr = &(tm_context(tvRtRadixTreeHead).rth6DefaultRouterHead);
#endif /* !TM_USE_STRONG_ESL */
    def6RtrPtr = (tt6RtDefRouterEntryPtr)0;
    if (def6HeadPtr->drh6RouterCount != TM_16BIT_ZERO)
    {
        def6RtrPtr = (tt6RtDefRouterEntryPtr)
                            tf6RtDefRouterSubWalk(def6HeadPtr,
                                                  walkFuncPtr,
                                                  genParam,
                                                  0);
    }
    return def6RtrPtr;
}

/*
 * tfListWalk CB function to return any default IPv6 gateway that
 * matches the flags.
 */
static int tf6RtDefRouterGetAnyCB (ttNodePtr nodePtr, ttGenericUnion genParam)
{
    tt6RtDefRouterEntryPtr  def6RtrPtr;    
    int                     retCode;
    tt16Bit                 flags;

    flags = genParam.gen16BitParm;
    retCode = (int)TM_8BIT_NO;
    def6RtrPtr = (tt6RtDefRouterEntryPtr)(ttVoidPtr)nodePtr;
    if ( tm_16bit_all_bits_set(def6RtrPtr->drt6Flags, flags) )
    {
       retCode = (int)TM_8BIT_YES;
    }
    return retCode;
}

/*
 * tfListWalk CB function to return the active default IPv6 gateway.
 * Get a gateway that is active (i.e. in the routing tree) and that
 * matches the flags.
 */
static int tf6RtDefRouterGetCB (ttNodePtr nodePtr, ttGenericUnion genParam)
{
    tt6RtDefRouterEntryPtr  def6RtrPtr;    
    int                     retCode;
    tt16Bit                 flags;

    flags = genParam.gen16BitParm;
    retCode = (int)TM_8BIT_NO;
    def6RtrPtr = (tt6RtDefRouterEntryPtr)(ttVoidPtr)nodePtr;
    if ( tm_16bit_all_bits_set(def6RtrPtr->drt6Flags, flags) )
    {
        if (def6RtrPtr->drt6RtePtr != (ttRteEntryPtr)0)
/*
 * only return a gateway that is active (i.e. in the routing tree)
 */
        {
           retCode = (int)TM_8BIT_YES;
        }
    }
    return retCode;
}

/*
 * tfListWalk CB function to return a default IPv6 gateway that
 * matches the IPv6 address.
 */
static int tf6RtDefRouterGetByIpCB(ttNodePtr nodePtr, ttGenericUnion genParam)
{
    tt6RtDefRouterEntryPtr  def6RtrPtr;    
    tt6IpAddressPtr         ip6AddrPtr;
    int                     retCode;

    ip6AddrPtr = genParam.genVoidParmPtr;
    retCode = (int)TM_8BIT_NO;
    def6RtrPtr = (tt6RtDefRouterEntryPtr)(ttVoidPtr)nodePtr;
    if (tm_6_ip_match(ip6AddrPtr, &(def6RtrPtr->drt6IpAddr)))
/*
 * return gateway that matches the IPv6 address.
 */
    {
       retCode = (int)TM_8BIT_YES;
    }
    return retCode;
}

/*
 * tf6RtDefRouterAddByDrt function description
 * Called with the routing tree locked. Add an IPv6 default router
 * knowing the pointer to an entry in the circular list of default
 * IPv6 router entries to the routing Patricia tree.
 * 1. Allocate a new routing entry copying the destination IP address
 * 2. Copy gateway address
 * 3. Insert in the tree.
 * 4. Cache entry if successful or free routing entry otherwise 
 * 5. Point to new default routing entry in the tree.
 *
 * Parameters
 * def6RtrPtr   Pointer to a default router entry in the circular list of 
 *              def IPv6 routers.
 * Returns
 */

static int tf6RtDefRouterAddByDrt(tt6RtDefRouterEntryPtr def6RtrPtr)
{
    ttRteEntryPtr         rtePtr;
    int                   errorCode;
    tt16Bit               flags;

/* 1. Allocate routing entry, and initialize destination IP address */
    rtePtr = tf6RtAllocNLockNCopy( TM_8BIT_ZERO, 
                                   TM_IP_ZERO, 
                                   (tt6IpAddressPtr)&in6addr_any );
    if (rtePtr != (ttRteEntryPtr)0)
    {
/* 2. Copy gateway address */
        tm_6_ip_copy_dststruct( &(def6RtrPtr->drt6IpAddr),
                                rtePtr->rteGSGateway );
/* Need to protect the IPv6 MTU, since it could be changed dynamically
   due to a router advertisement. */
        tm_kernel_set_critical;
        rtePtr->rteMtu = def6RtrPtr->drt6DevPtr->dev6Mtu;
        tm_kernel_release_critical;
/* 3. Insert in tree */
        if (def6RtrPtr->drt6Ttl == TM_RTE_INF)
        {
            flags = TM_RTE_STATIC | TM_RTE_INDIRECT;
        }
        else
        {
            flags = TM_RTE_INDIRECT;
        }
        errorCode = tfRtAddEntry(
                      tm_context(tvRtTreeHeadPtr), rtePtr, 
                      def6RtrPtr->drt6DevPtr,
                      0,
                      TM_6_RT_RESERVED_MHOME_INDEX, 
                      1, 
                      def6RtrPtr->drt6Ttl,
                      flags);
        
/*
 * When adding an IPv4 tunnel gateway to the routing table, copy the tunnel
 * endpoint into the routing entry.
 */
#ifdef TM_USE_IPV4
        if (def6RtrPtr->drt6Flags & TM_DRT6_V4_TUNNEL)
        {
            tm_6_ip_copy(&def6RtrPtr->drt6IpAddr, &rtePtr->rteHSTunnelExitAddr);
        }
#endif /* TM_USE_IPV4 */
        
/* 4. Cache entry if successful or free routing entry otherwise  */
        (void)tfRtCacheUnlockNClean( rtePtr, 
                                     TM_8BIT_ZERO, /* no unlock */
                                     errorCode, 
                                     TM_RTE_STATIC | TM_RTE_INDIRECT );
        if (errorCode == TM_ENOERROR)
        {
/* 5. Point to new default routing entry in the tree */ 
            tm_incr_owner_count(rtePtr->rteOwnerCount);
            def6RtrPtr->drt6RtePtr = rtePtr;
            tm_16bit_clr_bit( def6RtrPtr->drt6Flags, TM_DRT6_INSERT_FAILED );
        }
        else
        {
/* 
 * We failed adding the default router entry to the tree, mark this entry
 * so that it can be added later when a local route is added. 
 */

            def6RtrPtr->drt6Flags |= TM_DRT6_INSERT_FAILED;
        }
    }
    else
    {
        errorCode = TM_ENOBUFS;
    }
    return errorCode;
}

/* Add default router if it is reachable */
static int tf6RtDefRouterAddReachableCB (ttNodePtr nodePtr, ttGenericUnion genParam)
{
    tt6RtDefRouterEntryPtr  def6RtrPtr;    
    tt6RtDefRouterHeadPtr   def6HeadPtr;
    ttRteEntryPtr           linkRtePtr;
    int                     errorCode;
    int                     retCode;

#ifndef TM_USE_STRONG_ESL
    TM_UNREF_IN_ARG(genParam);
#endif /* !TM_USE_STRONG_ESL */
    retCode = (int)TM_8BIT_NO;
    def6RtrPtr = (tt6RtDefRouterEntryPtr)(ttVoidPtr)nodePtr;
#ifdef TM_USE_STRONG_ESL
    def6HeadPtr = &(((ttDeviceEntryPtr)(genParam.genVoidParmPtr))
                                                ->dev6DefaultRouterHead);
#else /* !TM_USE_STRONG_ESL */
    def6HeadPtr = &(tm_context(tvRtRadixTreeHead).rth6DefaultRouterHead);
#endif /* !TM_USE_STRONG_ESL */
#ifdef TM_USE_IPV4
    if (  (tm_16bit_bits_not_set(def6RtrPtr->drt6Flags,
                                 TM_DRT6_V4_TUNNEL))
/* Only pick a default gateway tunnel if it is the only gateway */
       || (def6HeadPtr->drh6RouterCount == 1) )
#endif /* TM_USE_IPV4 */
    {
/*
 * we check if the gateway is in a reachable state.
 */
        linkRtePtr = tfRtFindIpAddrIpNet( 
#ifdef TM_USE_STRONG_ESL
                                          def6RtrPtr->drt6DevPtr,
#endif /* TM_USE_STRONG_ESL */
                                          &(def6RtrPtr->drt6IpAddr),
                                          TM_6_IP_HOST_PREFIX_LENGTH);
        if (    (linkRtePtr != (ttRteEntryPtr)0)
             && (    linkRtePtr->rte6HSNudState
                  == TM_6_NUD_REACHABLE_STATE )
           )
        {
            errorCode = tf6RtDefRouterAddByDrt(def6RtrPtr);
            if (errorCode == TM_ENOERROR)
            {
                retCode = (int)TM_8BIT_YES;
            }
        }
    }
    return retCode;
}

/* Add default router */
static int tf6RtDefRouterAddAnyCB (ttNodePtr nodePtr, ttGenericUnion genParam)
{
    tt6RtDefRouterEntryPtr  def6RtrPtr;    
    tt6RtDefRouterHeadPtr   def6HeadPtr;
    ttRteEntryPtr           linkRtePtr;
    int                     retCode;
    int                     errorCode;

#ifndef TM_USE_STRONG_ESL
     TM_UNREF_IN_ARG(genParam);
#endif /* !TM_USE_STRONG_ESL */
    retCode = (int)TM_8BIT_NO; /* keep walking */
    def6RtrPtr = (tt6RtDefRouterEntryPtr)(ttVoidPtr)nodePtr;
#ifdef TM_USE_STRONG_ESL
    def6HeadPtr = &(((ttDeviceEntryPtr)(genParam.genVoidParmPtr))
                                                ->dev6DefaultRouterHead);
#else /* !TM_USE_STRONG_ESL */
    def6HeadPtr = &(tm_context(tvRtRadixTreeHead).rth6DefaultRouterHead);
#endif /* !TM_USE_STRONG_ESL */
#ifdef TM_USE_IPV4
    if (  (tm_16bit_bits_not_set(def6RtrPtr->drt6Flags,
                                 TM_DRT6_V4_TUNNEL))
/* Only pick a default gateway tunnel if it is the only gateway */
       || (def6HeadPtr->drh6RouterCount == 1) )
#endif /* TM_USE_IPV4 */
    {
/* Find an entry in the routing table for the destination IP address
 * If entry is a LAN network destination, clone for specific host
 * If prefix has not been configured, add the cloned entry manually.
 */
        linkRtePtr = tfRtFindNClone(
#ifdef TM_USE_STRONG_ESL
                                    def6RtrPtr->drt6DevPtr,
#endif /* TM_USE_STRONG_ESL */
                                    &(def6RtrPtr->drt6IpAddr));
        if (linkRtePtr != (ttRteEntryPtr)0)
        {
/* We were able to clone */
            retCode = (int)TM_8BIT_YES;
        }
        else if (    (tm_6_ll_is_lan(def6RtrPtr->drt6DevPtr))
                  && (tm_16bit_one_bit_set(def6RtrPtr->drt6Flags,
                                           TM_DRT6_TEMPORARY)) )
        {
/* 
 * Not able to clone. Gateway prefix is not configured yet.
 * if the interface supports cloning of link-layer addresses, and the entry
 * was added by the user as temporary, then create a
 * cloned host entry for the destination in the INCOMPLETE state (i.e. without
 * the TM_RTE_LINK_LAYER bit set) so that we are able to add the default
 * gateway to the routing table.
 */
            errorCode = tf6RtAddHost(
                                    def6RtrPtr->drt6DevPtr,
                                    &(def6RtrPtr->drt6IpAddr),
                                    TM_8BIT_NULL_PTR,/* physical address ptr */
                                    (tt8Bit) 0,      /* physical address len */
                                    TM_6_RT_RESERVED_MHOME_INDEX,
                                    TM_RTE_INF,
                                    TM_RTE_ARP | TM_RTE_CLONED,
                                    TM_8BIT_NO);     /* tree already locked */
            if (errorCode == TM_ENOERROR)
            {
                retCode = (int)TM_8BIT_YES;
            }
        }
    }
    if (retCode)
    {
        errorCode = tf6RtDefRouterAddByDrt(def6RtrPtr);
        if (errorCode != TM_ENOERROR)
        {
            retCode = TM_8BIT_NO;
        }
    }
    return retCode;
}

/*
 * tf6RtDefRouterDelByDrt function description
 * Called with the routing tree locked. Remove an IPv6 default router
 * knowing the pointer to an entry in the circular list of default
 * IPv6 router entries.
 * 1.a Remove entry from the IPv6 default routers circular list, 
 * 2. If default router points to a routing entry:
 *  2.a If entry is in the tree, remove it.
 *  2.b call tfRtUnGet to decrease owner count and possibly free the routing 
 *      entry.
 *  2.c Pick another IPv6 default router entry from the list, and insert
 *      in tree. When choosing an entry from the list, try and pick an
 *      entry that has a valid link layer mapping.
 * 3. Walk the routing tree to remove all indirect routes pointing to
 *    this default router.
 * 4. Free circular list entry.
 * Parameters
 * def6RtrPtr   Pointer to a default router entry in the circular list of 
 *              def IPv6 routers.
 * Returns
 */
static int tf6RtDefRouterDelByDrt(
#ifdef TM_USE_STRONG_ESL
                                  ttDeviceEntryPtr       devPtr,
#endif /* TM_USE_STRONG_ESL */
                                  tt6RtDefRouterEntryPtr def6RtrPtr)
{
/* Pointer to IPV6 default router structure */
    tt6RtDefRouterHeadPtr  def6HeadPtr;
/* Head of circular list */
    ttListPtr               def6HeadListPtr;
/* Pointer to entry that is in the tree */
    ttRteEntryPtr           def6RtrRtePtr;
#ifdef TM_USE_STRONG_ESL
    tt6RtDevGw              rt6DevGwStruct;
#endif /* TM_USE_STRONG_ESL */
    ttVoidPtr               rt6VoidPtr;
#ifdef TM_USE_IPV4
    ttDeviceEntryPtr        tunlDevPtr;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV4
    tt16Bit                 mhomeIndex;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_STRONG_ESL
    def6HeadPtr = &(devPtr->dev6DefaultRouterHead);
#else /* !TM_USE_STRONG_ESL */
    def6HeadPtr = &(tm_context(tvRtRadixTreeHead).rth6DefaultRouterHead);
#endif /* !TM_USE_STRONG_ESL */
    if (def6HeadPtr->drh6RouterCount != TM_16BIT_ZERO)
    {
        def6HeadListPtr = &def6HeadPtr->drh6RouterList[
                                            def6RtrPtr->drt6ListIndex];
        def6RtrRtePtr = def6RtrPtr->drt6RtePtr;
        def6HeadPtr->drh6RouterCount--;
        tfListRemove(def6HeadListPtr, &def6RtrPtr->drt6Node);
/* 2. If default router points to a routing entry:*/
        if (def6RtrRtePtr != (ttRteEntryPtr)0)
        {
/* 2.a Remove entry from the tree, if it had not been removed yet. */
            if ( tm_16bit_one_bit_set( def6RtrRtePtr->rteFlags2, TM_RTE2_UP ) )
            {
                tfRtRemoveEntry(def6RtrRtePtr);
            }

#ifdef TM_6_USE_MIP_MN
            if ((tm_context(tv6MnVect).mns6Flags & TM_6_MNS_MOBILE_NODE_ACTIVE)
/* avoid recursive L3 move notification */
                && !(tm_context(tv6MnVect).mns6Flags & TM_6_MNS_L3_MOVE_PENDING))
            {
/* we need to notify the user of this and initiate L3 move processing.
   ([MIPV6_20].R8.5:140, [MIPV6_18++].R11.5.1:40) */

/* initiate L3 move processing in the context of the timer task */
                tm_context(tvRtRadixTreeHead).rthL3MoveDetected = TM_8BIT_YES;
            }
#endif /* TM_6_USE_MIP_MN */
#ifdef TM_USE_IPV4
            if ( tm_16bit_one_bit_set(def6RtrPtr->drt6Flags,
                                      TM_DRT6_V4_TUNNEL) )
            {
                tunlDevPtr = tm_context(tv6Ipv4DefGwTunnelPtr);
/* Clean up the tunnel source addresses */
                for (mhomeIndex = 0;
                     mhomeIndex < tunlDevPtr->dev6MhomeUsedEntries;
                     mhomeIndex++)
                {
                    if ((tunlDevPtr->dev6IpAddrFlagsArray[mhomeIndex]
                                                    & TM_6_DEV_IP_CONFIG))
                    {
                        tm_8bit_clr_bit(
                            tunlDevPtr->dev6IpAddrFlagsArray[mhomeIndex],
                            TM_6_DEV_IP_CONFIG);
                    }
                }
            }
#endif /* TM_USE_IPV4 */
/* 2.b Decrease owner count */
            tfRtUnGet(def6RtrRtePtr);
/* 2.c Pick next default router from the list */
            if (def6HeadPtr->drh6RouterCount != TM_16BIT_ZERO)
/* If lists are non empty */
            {
                (void)tf6RtDefRouterAddNewInTree(
#ifdef TM_USE_STRONG_ESL
                                                 devPtr
#endif /* TM_USE_STRONG_ESL */
                                                 );
            }
        }
    }
/*
 * 3. Remove any redirect entry pointing to the gateway being removed 
 */
#ifdef TM_USE_STRONG_ESL
    rt6DevGwStruct.dgwDevPtr = def6RtrPtr->drt6DevPtr;
    rt6DevGwStruct.dgwIp6AddrPtr = &(def6RtrPtr->drt6IpAddr);
    rt6VoidPtr = &rt6DevGwStruct;
#else /* !TM_USE_STRONG_ESL */
    rt6VoidPtr = (ttVoidPtr)&(def6RtrPtr->drt6IpAddr); 
#endif /* !TM_USE_STRONG_ESL */
    (void)tfRtTreeWalk( tfRtRemoveGwEntryCB, 
                        rt6VoidPtr, 
                        TM_8BIT_ZERO );
/* 4. Free entry */
    tm_free_raw_buffer((ttRawBufferPtr)(ttVoidPtr)def6RtrPtr);
    return TM_ENOERROR;
}

static tt6RtDefRouterEntryPtr  tf6RtDefRouterAddNewInTree(
#ifdef TM_USE_STRONG_ESL
                                  ttDeviceEntryPtr    devPtr
#else /* !TM_USE_STRONG_ESL */
                                  void
#endif /* !TM_USE_STRONG_ESL */
                                    )
{
    tt6RtDefRouterEntryPtr  def6RtrPtr;
/* Head of circular list */
    ttGenericUnion          genParam;

#ifdef TM_USE_STRONG_ESL
    genParam.genVoidParmPtr = (ttVoidPtr)devPtr;
#else /* !TM_USE_STRONG_ESL */
    genParam.genVoidParmPtr = (ttVoidPtr)0;
#endif /* !TM_USE_STRONG_ESL */
    def6RtrPtr = tf6RtDefRouterWalk(
#ifdef TM_USE_STRONG_ESL
                                    devPtr,
#endif /* TM_USE_STRONG_ESL */
                                    tf6RtDefRouterAddReachableCB,
                                    genParam);
    if (def6RtrPtr == (tt6RtDefRouterEntryPtr)0)
    {
        def6RtrPtr = tf6RtDefRouterWalk(
#ifdef TM_USE_STRONG_ESL
                                    devPtr,
#endif /* TM_USE_STRONG_ESL */
                                    tf6RtDefRouterAddAnyCB,
                                    genParam);
    }
    return def6RtrPtr;
}

/* tfListWalk CB function to age default IPv6 gateways.  */
static int tf6RtDefRouterTimerExpireCB (ttNodePtr nodePtr, ttGenericUnion genParam)
{
    tt6RtDefRouterHeadPtr   def6HeadPtr;
    tt6RtDefRouterEntryPtr  def6RtrPtr;    

#ifndef TM_USE_STRONG_ESL
     TM_UNREF_IN_ARG(genParam);
#endif /* !TM_USE_STRONG_ESL */
    def6RtrPtr = (tt6RtDefRouterEntryPtr)(ttVoidPtr)nodePtr;
    if (def6RtrPtr->drt6Ttl != TM_RTE_INF)
    {
/* 
 * . update the ttl. 
 * . If ttl is zero, move entry into the expired list. 
 */
        if ( def6RtrPtr->drt6Ttl > tm_context(tvRtTimerResolution) )
        {
            def6RtrPtr->drt6Ttl -= tm_context(tvRtTimerResolution);
        }
        else
        {
            def6RtrPtr->drt6Ttl = TM_UL(0);
#ifdef TM_USE_STRONG_ESL
            def6HeadPtr = &(((ttDeviceEntryPtr)(genParam.genVoidParmPtr))
                                                ->dev6DefaultRouterHead);
#else /* !TM_USE_STRONG_ESL */
            def6HeadPtr = &(tm_context(tvRtRadixTreeHead).rth6DefaultRouterHead);
#endif /* !TM_USE_STRONG_ESL */
/* Remove from the current list */
            tfListRemove(&def6HeadPtr->drh6RouterList[
                                        def6RtrPtr->drt6ListIndex], 
                         &def6RtrPtr->drt6Node);
/* Add to the expired list */
            def6RtrPtr->drt6ListIndex = TM_6_DEF_EXP_ROUTER_LIST;
            tfListAddToTail(&def6HeadPtr->drh6RouterList[
                                            TM_6_DEF_EXP_ROUTER_LIST],
                            &def6RtrPtr->drt6Node);
        }
    }
    return (int)TM_8BIT_NO; /* keep walking */
}

/*
 * tfListWalk CB function to delete expired default routers
 */
static int tf6RtDefRouterDelExpiredCB (ttNodePtr      nodePtr,
                                       ttGenericUnion genParam)
{
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr devPtr;
#endif /* TM_USE_STRONG_ESL */

#ifdef TM_USE_STRONG_ESL
    devPtr = (ttDeviceEntryPtr)genParam.genVoidParmPtr;
#else /* !TM_USE_STRONG_ESL */
    TM_UNREF_IN_ARG(genParam);
#endif /* !TM_USE_STRONG_ESL */
    (void)tf6RtDefRouterDelByDrt(
#ifdef TM_USE_STRONG_ESL
                                  ((ttDeviceEntryPtr)genParam.genVoidParmPtr),
#endif /* TM_USE_STRONG_ESL */
                                  (tt6RtDefRouterEntryPtr)(ttVoidPtr)nodePtr);
    return (int)TM_8BIT_NO; /* keep walking */
}

/*
 * tf6RtDefRouterTimer function description
 * Age all the default routers in the IPv6 default router list.
 * 1. Lock the routing tree
 * 2. Dereference the head of the IPv6 default router list
 * 3. For each entry in the IPv6 default router list.
 *  3.a If the ttl is bigger than the timer resolution, decrease the ttl
 *  3.b Otherwise delete the default router, calling tf6RtDefRouterDelByDrt
 * 4. Unlock the routing tree
 */
static void tf6RtDefRouterTimer ( ttVoidPtr      timerBlockPtr,
                                  ttGenericUnion userParm1,
                                  ttGenericUnion userParm2 )
{
/* Default routers structure */
    tt6RtDefRouterHeadPtr  def6HeadPtr;
    ttListPtr              def6HeadListPtr;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);
#ifndef TM_USE_STRONG_ESL
    TM_UNREF_IN_ARG(userParm1);
#endif /* !TM_USE_STRONG_ESL */

    tm_call_lock_wait(&tm_context(tvRtTreeLock));
#ifdef TM_USE_STRONG_ESL
    def6HeadPtr = &(((ttDeviceEntryPtr)(userParm1.genVoidParmPtr))
                                                ->dev6DefaultRouterHead);
#else /* !TM_USE_STRONG_ESL */
    def6HeadPtr = &(tm_context(tvRtRadixTreeHead).rth6DefaultRouterHead);
#endif /* !TM_USE_STRONG_ESL */
    if (def6HeadPtr->drh6RouterCount != TM_16BIT_ZERO)
    {
/*
 * Only search the non temporary list. Temporary gateways never expire (set to
 * infinite timeout).
 */
        def6HeadListPtr = &def6HeadPtr->drh6RouterList[TM_6_DEF_ROUTER_LIST];
        if (def6HeadListPtr->listCount != 0)
        {
/*
 * NOTE: We cannot delete the current default gateway and add a new one while
 * we walk the default gateway lists. So we just move the expired default
 * gateways to an expired list. We will remove them from the expired list,
 * and add a new default router if need be after we are done walking the
 * current default gateway lists.
 */
/* 1. Search the lists of default gateways that can expire. Move them to the
 * expired list */
            (void)tfListWalk(def6HeadListPtr, tf6RtDefRouterTimerExpireCB,
                             userParm1);
/*
 * 2. Walk the list of expired routers. Delete them, if one of them is the default
 * router select a new one.
 */
            def6HeadListPtr = &def6HeadPtr->drh6RouterList[
                                                    TM_6_DEF_EXP_ROUTER_LIST];
            if (def6HeadListPtr->listCount != 0)
            {
                (void)tfListWalk(def6HeadListPtr, tf6RtDefRouterDelExpiredCB,
                                 userParm1);
            }
        }
    }
    if (def6HeadPtr->drh6RouterCount == TM_16BIT_ZERO)
    {
        tm_timer_remove(def6HeadPtr->drh6RouterTimerPtr);
        def6HeadPtr->drh6RouterTimerPtr = (ttTimerPtr)0;
    }
    tfRtTreeUnlockNResetCaches();
}

/*
 * tf6RtDefRouterTag Function Description:
 * Called when a local route is added. 
 * If we had failed earlier to insert a default router entry in the tree try 
 * and do so now.
 * 0. Lock the tree
 * 1. Loop through the default routers.
 *    . If there is already a default router inserted in the tree, break.
 *    . If the TM_DRT6_INSERT_FAILED bit is set, try an insert in the tree.
 *      Break is we are successful.
 * 2. Unlock the tree
 *
 * Parameters
 * devPtr           Pointer to interface associated with the route.
 *
 * Returns
 * None
 */
static void tf6RtDefRouterTag(
#ifdef TM_USE_STRONG_ESL
                              ttDeviceEntryPtr devPtr
#else /* TM_USE_STRONG_ESL */
                              void
#endif /* TM_USE_STRONG_ESL */
                             )
{
    ttGenericUnion         genParam;

/* 0. Lock the tree */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));
    genParam.gen16BitParm = 0xFFFF;
    (void)tf6RtDefRouterWalk(
#ifdef TM_USE_STRONG_ESL
                              devPtr,
#endif /* TM_USE_STRONG_ESL */
                              tf6RtTagDefRouterCB,
                              genParam);
/* 2. UNLOCK the tree to release access to the routing table */
    tfRtTreeUnlockNResetCaches();
    return;
}

/*
 * tfListWalk CB function to insert a default IPv6 gateway, that failed
 * to insert before.
 */
static int tf6RtTagDefRouterCB(ttNodePtr nodePtr, ttGenericUnion genParam)
{
    tt6RtDefRouterEntryPtr  def6RtrPtr;    
    int                     errorCode;
    int                     retCode;

    TM_UNREF_IN_ARG(genParam);
    retCode = (int)TM_8BIT_NO;
    def6RtrPtr = (tt6RtDefRouterEntryPtr)(ttVoidPtr)nodePtr;
    if (def6RtrPtr->drt6RtePtr != (ttRteEntryPtr)0)
    {
/* There exists a default entry in the tree. Stop the search. */
        retCode = (int)TM_8BIT_YES;
    }
    else
    {
        if ( tm_16bit_one_bit_set( def6RtrPtr->drt6Flags, 
                                   TM_DRT6_INSERT_FAILED) )
        {
/* If we failed earlier to insert, do so now */
            errorCode = tf6RtDefRouterAddByDrt(def6RtrPtr);
            if (errorCode == TM_ENOERROR)
            {
/* We now have a default entry in the tree. Stop the search. */
                retCode = TM_8BIT_YES;
            }
        }
    }
    return retCode;
}


/*
 * tfListWalk() CB function to delete router entry from the list if we have a
 * match on the device
 */
static int tf6RtDefRouterExpireMatchDevCB (ttNodePtr      nodePtr,
                                           ttGenericUnion genParam)
{
    tt6RtDefRouterEntryPtr  def6RtrPtr;    
    ttDeviceEntryPtr        devPtr;

    def6RtrPtr = (tt6RtDefRouterEntryPtr)(ttVoidPtr)nodePtr;
    devPtr = (ttDeviceEntryPtr)genParam.genVoidParmPtr;
    if (    (def6RtrPtr->drt6DevPtr  == devPtr)
#ifdef TM_6_USE_MIP_MN
         || (devPtr == (ttDeviceEntryPtr)0)
#endif /* TM_6_USE_MIP_MN */
       )
    {
/* Delete router entry from the list if we have a match on the device */
        (void)tf6RtDefRouterDelByDrt(
#ifdef TM_USE_STRONG_ESL
                                     devPtr,
#endif /* TM_USE_STRONG_ESL */
                                     def6RtrPtr);
    }
    return (int)TM_8BIT_NO;
}

/*
 * Called when closing a device. Delete all default router entries
 * on that device.
 */
static void tf6RtDefRouterDelAllByDevPtr(ttDeviceEntryPtr devPtr)
{
/* Pointer to IPV6 default router structure */
    tt6RtDefRouterHeadPtr   def6HeadPtr;
    ttListPtr               def6HeadListPtr;
    ttGenericUnion          genParam;

    tm_assert_is_locked(
        tf6RtDefRouterDelAllByDevPtr, &tm_context(tvRtTreeLock));

#ifdef TM_USE_STRONG_ESL
    def6HeadPtr = &devPtr->dev6DefaultRouterHead;
#else /* !TM_USE_STRONG_ESL */
    def6HeadPtr = &(tm_context(tvRtRadixTreeHead).rth6DefaultRouterHead);
#endif /* !TM_USE_STRONG_ESL */
/* 
 * 1. Search the list of default gateways
 */
    if (def6HeadPtr->drh6RouterCount != TM_16BIT_ZERO)
/* Non empty list */
    {
        genParam.genVoidParmPtr = (ttVoidPtr)devPtr;
/*
 * Walk the default router active lists, and put the matching routers to be
 * deleted in the expired list.
 */
        (void)tf6RtDefRouterWalk(
#ifdef TM_USE_STRONG_ESL
                                  devPtr,
#endif /* TM_USE_STRONG_ESL */
                                  tf6RtDefRouterExpireMatchDevCB,
                                  genParam);
        def6HeadListPtr = &def6HeadPtr->drh6RouterList[
                                                    TM_6_DEF_EXP_ROUTER_LIST];
        if (def6HeadListPtr->listCount != 0)
        {
/* Delete all routers in the expired list */
            (void)tfListWalk(def6HeadListPtr, tf6RtDefRouterDelExpiredCB,
                             genParam);
        }
    }
    return;
}

static void tf6RtDefRouterInvalid (
#ifdef TM_USE_STRONG_ESL
                      ttDeviceEntryPtr      devPtr,
#endif /* TM_USE_STRONG_ESL */
                      ttRteEntryPtr         rtePtr)
{
    tt6RtDefRouterHeadPtr  def6HeadPtr;
    tt6RtDefRouterEntryPtr def6RtrPtr;
    ttListPtr              def6HeadListPtr;
    ttNodePtr              nodePtr;
    ttRteEntryPtr          def6RtePtr;
    ttRteEntryPtr          clonedRtePtr;
    ttGenericUnion         genParam;

/* Find if there exists a default router */
    genParam.gen16BitParm = TM_16BIT_ZERO;
    def6RtrPtr = tf6RtDefRouterWalk(
#ifdef TM_USE_STRONG_ESL
                                      devPtr,
#endif /* TM_USE_STRONG_ESL */
                                      tf6RtDefRouterGetCB,
                                      genParam);
    if (def6RtrPtr != (tt6RtDefRouterEntryPtr)0)
    {
        def6RtePtr = def6RtrPtr->drt6RtePtr;
        clonedRtePtr = def6RtePtr->rteClonedPtr;
        if (clonedRtePtr == rtePtr)
/* Default router pointing to the invalid ARP entry */
        {
#ifdef TM_USE_STRONG_ESL
            def6HeadPtr = &(devPtr->dev6DefaultRouterHead);
#else /* !TM_USE_STRONG_ESL */
            def6HeadPtr = &(tm_context(tvRtRadixTreeHead).rth6DefaultRouterHead);
#endif /* !TM_USE_STRONG_ESL */
            def6HeadListPtr = &def6HeadPtr->drh6RouterList[
                                            def6RtrPtr->drt6ListIndex];
            nodePtr = &def6RtrPtr->drt6Node;
            if (nodePtr->nodeNextPtr != &(def6HeadListPtr->listHeadNode))
/* If not currently at the end of the list */
            {
/* Remove from current list and add to the end of the list */
                tfListRemove(def6HeadListPtr, &def6RtrPtr->drt6Node);
                tfListAddToTail(def6HeadListPtr, &def6RtrPtr->drt6Node);
            }
/*
 * disassociate from ARP entry before removing default gateway from the tree
 * to prevent recursion.
 */
            def6RtePtr->rteClonedPtr = (ttRteEntryPtr)0;
            if (tm_16bit_one_bit_set(def6RtePtr->rteFlags2, TM_RTE2_UP))
            {
/* Remove default gateway from the tree */
                tfRtRemoveEntry(def6RtePtr);
            }
/* Remove default gateway association with routing entry in the tree */
            tfRtUnGet(def6RtePtr);
            def6RtrPtr->drt6RtePtr = (ttRteEntryPtr)0;
            if (clonedRtePtr != (ttRteEntryPtr)0)
/* Associated ARP entry */
            {
/*
 * No longer pointed to by the default gateway routing entry. (We reset
 * rteClonedPtr above.
 */
                tfRtUnGet(clonedRtePtr);
                tf6RtArpDelByRte(clonedRtePtr);
            }
/* Pick the next default gateway */
            (void)tf6RtDefRouterAddNewInTree(
#ifdef TM_USE_STRONG_ESL
                                             devPtr
#endif /* TM_USE_STRONG_ESL */
                                             );
        }
    }
}

/*
 * tf6RtRedirect
 * Function Description
 * Called by ICMPv6 when a Redirect message is received.  Adds the appropriate
 * route, either a route to the gateway for the destination, or a host entry
 * if the destination and gateway addresses match.  Also updates the link
 * layer address and reachability state if a Target Link-Layer option was
 * included.
 *
 * 1. Lock the routing tree.
 * 2. Find the routing entry associated with the destination address
 *    specificied in the Redirect message.
 * 3. If no route was found, exit routine.
 * 4. Verify that the source address of the Redirect is the same as the
 *    current first-hop router for the specified ICMP Destination
 *    Address. ([RFC2461].R8.1:10)
 * 5. Delete the current entry so we can add a new one, but only if it is a
 *    host route.
 * 6. If the target address is the same as the destination address, this
 *    indicates that we should treat the target as being on link.  In this
 *    case, we need to add a host entry.  If the destination and target
 *    addresses are different, this is considered a router, so set the
 *    IsRouter flag. ([RFC2461].R8.3:70, [RFC2461].R8.3:80, [RFC2461].R8.3:90,
 *    [RFC2461].R8.3:10, [RFC2461].R8.3:20)
 * 7. Find the new entry so we can update the IPv6 route flags and possibly
 *    the link layer address.
 * 8. If the redirect message contains a Target Link-Layer Address option and
 *    this routing entry is cloned, update the link layer address (note: for
 *    an interface that would receive this option( i.e. a LAN), we should
 *    always find a cloned entry; we double check anyways).
 * 8.1. If a link layer address has already been set for this routing entry:
 *     8.1.1. If the Target Link-Layer Address is different from the cached
 *            value (rteHSPhysAddr), update the routing entry with the new
 *            link layer address from the redirect message and set reachability
 *            state to STALE. ([RFC2461].R8.3:50)
 *     8.1.2. If the Target Link-Layer Address is identical to the cache value,
 *            so do nothing. ([RFC2461].R8.3:60)
 * 8.2. Since no link layer address has been set (no 'Neighbor Cache' entry
 *      exists), go ahead and add it by copying the link layer address in the
 *      option to the routing entry (rteHSPhysAddr), setting the
 *      TM_RTE_LINK_LAYER flag and setting the reachability state to STALE.
 *      ([RFC2461].R8.3:40, [RFC2461].R8.3:30)
 * 9. Unlock the routing tree.
 *     
 * Parameters
 * Parameter     Description
 * devPtr   Device for this router (device the redirect was received on).
 * mhomeIndex    Multihome index the redirect was destined for.
 * destIpAddrPtr Original IP address that caused the redirect.
 * gwIpAddrPtr   New gateway for the destination address.
 * srcIpAddrPtr  Source address of the redirect message.
 * physAddrPtr   Pointer to the link layer address if a Target Link-Layer
 *               Address option was included with the redirect.  Otherwise,
 *               should be set to null.
 * physAddrLen   Length of the link layer address, if one is set.
 *
 * Returns
 * None
 */ 
int tf6RtRedirect(ttDeviceEntryPtr devPtr,
                  tt16Bit          mhomeIndex,
                  tt6IpAddressPtr  destIpAddrPtr,
                  tt6IpAddressPtr  gwIpAddrPtr,
                  tt6IpAddressPtr  srcIpAddrPtr,
                  tt8BitPtr        physAddrPtr,
                  tt8Bit           physAddrLen)
{
    tt16Bit       newRteFlags;
#ifndef TM_6_USE_NUD
    tt16Bit       newRte6Flags;
    tt8Bit        updatePhysAddr;
    tt8Bit        physAddrMatch;
#endif /* !TM_6_USE_NUD */
    ttRteEntryPtr rtePtr;
    int           errorCode;

#ifdef TM_ERROR_CHECKING
    tm_debug_log8("tf6RtRedirect: %lx:%lx:%lx:%lx => %lx:%lx:%lx:%lx",
                  ntohl(destIpAddrPtr->s6LAddr[0]),
                  ntohl(destIpAddrPtr->s6LAddr[1]),
                  ntohl(destIpAddrPtr->s6LAddr[2]),
                  ntohl(destIpAddrPtr->s6LAddr[3]),
                  ntohl(gwIpAddrPtr->s6LAddr[0]),
                  ntohl(gwIpAddrPtr->s6LAddr[1]),
                  ntohl(gwIpAddrPtr->s6LAddr[2]),
                  ntohl(gwIpAddrPtr->s6LAddr[3]));
#ifdef TM_USE_NETSTAT
#ifdef TM_DEBUG_LOGGING
    tfxLogRteTable("tf6RtRedirect");
#endif /* TM_DEBUG_LOGGING */
#endif /*TM_USE_NETSTAT*/
#endif /* TM_ERROR_CHECKING */

    errorCode = TM_ENOERROR;

/* 1. Lock the routing tree. */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));

/*    
 * 2. Find the routing entry associated with the destination address
 *    specificied in the Redirect message.
 */
    rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                      devPtr,
#endif /* TM_USE_STRONG_ESL */
                      destIpAddrPtr);
    
/* 3. If no route was found, exit routine. */
    if (rtePtr != TM_RTE_NULL_PTR)
    {
/*    
 * 4. Verify that the source address of the Redirect is the same as the
 *    current first-hop router for the specified ICMP Destination
 *    Address. ([RFC2461].R8.1:10)
 */
        if ( tm_6_ip_match(&rtePtr->rteGSGateway, srcIpAddrPtr))
        {
/*    
 * 5. Delete the current entry so we can add a new one, but only if it is a
 *    host route.
 */
/*
 * Reset all the caches that points to either a cloned entry or indirect 
 * entry 
 */
            tm_context(tvRtRadixTreeHead).rthResetCachesFlags 
                                            |= TM_RTE_INDIRECT | TM_RTE_CLONED;
            tm_context(tvRtRadixTreeHead).rthResetCachesAf 
                                            |= (AF_INET6);
            if ( tm_16bit_all_bits_set( rtePtr->rteFlags2,
                                        TM_RTE2_HOST | TM_RTE2_UP ) )
            {
                tfRtRemoveHostEntry(rtePtr, 1, (tt16Bit)AF_INET6);
            }

/*
 * 6. If the target address is the same as the destination address, this
 *    indicates that we should treat the target as being on link.  In this
 *    case, we need to add a host entry.  If the destination and target
 *    addresses are different, this is considered a router, so set the
 *    IsRouter flag. ([RFC2461].R8.3:70, [RFC2461].R8.3:80, [RFC2461].R8.3:90,
 *    [RFC2461].R8.3:10, [RFC2461].R8.3:20)
 */
            if ( tm_6_ip_match(destIpAddrPtr, gwIpAddrPtr) )
            {
                newRteFlags = TM_RTE_ARP | TM_RTE_CLONED;
                if (physAddrPtr != TM_8BIT_NULL_PTR)
                {
                    newRteFlags |= TM_RTE_LINK_LAYER;
                }

#ifndef TM_6_USE_NUD
                newRte6Flags = 0;
/* Physical address updated by tf6RtAddHost, no need to do it below. */
                updatePhysAddr = TM_8BIT_ZERO;
#endif /* ! TM_6_USE_NUD */

                tf6RtAddHost(devPtr,
                             destIpAddrPtr,
                             physAddrPtr,
                             physAddrLen,
                             mhomeIndex,
                             TM_RTE_INF,
                             newRteFlags,
/* Routing tree already locked */                             
                             TM_8BIT_ZERO);
            }
            else
            {
#ifndef TM_6_USE_NUD
                newRte6Flags = TM_6_RTE_IS_ROUTER_FLAG;
/* Attempt to update link layer address below */
                updatePhysAddr = TM_8BIT_YES;
#endif /* ! TM_6_USE_NUD */

                tf6RtAddRoute(devPtr,
                              destIpAddrPtr,
                              TM_6_IP_HOST_PREFIX_LENGTH,
                              gwIpAddrPtr,
                              TM_16BIT_ZERO,
                              mhomeIndex,
                              1, /* hops */
                              TM_RTE_INF,
                              TM_RTE_REDIRECT|TM_RTE_INDIRECT,
/* Routing tree already locked */
                              TM_8BIT_ZERO);
            }

#ifndef TM_6_USE_NUD
/*    
 * 7. Find the new entry so we can update the IPv6 route flags and possibly
 *    the link layer address.
 */
            rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                              devPtr,
#endif /* TM_USE_STRONG_ESL */
                              destIpAddrPtr);
            rtePtr->rte6Flags = newRte6Flags;

/*    
 * 8. If the redirect message contains a Target Link-Layer Address option and
 *    this routing entry is cloned, update the link layer address (note: for
 *    an interface that would receive this option (i.e. a LAN), we should
 *    always find a cloned entry, but we double check anyways).
 */
            if (    (updatePhysAddr == TM_8BIT_YES)
                 && (physAddrPtr != TM_8BIT_NULL_PTR)
                 && (rtePtr->rteFlags & TM_RTE_ARP))
            {

/*        
 * 8.1. If a link layer address has already been set for this routing entry
 */
                if (rtePtr->rteFlags & TM_RTE_LINK_LAYER)
                {
/*        
 *     8.1.1. If the Target Link-Layer Address is different from the cached
 *            value (rteHSPhysAddr), update the routing entry with the new
 *            link layer address from the redirect message and set reachability
 *            state to STALE (done below via tfRtArpAddByRte).
 *            ([RFC2461].R8.3:50)
 *     8.1.2. If the Target Link-Layer Address is identical to the cache value,
 *            do nothing. ([RFC2461].R8.3:60)
 */
                    tm_6_phys_addr_match(physAddrPtr,
                                         rtePtr->rteHSPhysAddr,
                                         physAddrLen,
                                         physAddrMatch);

/* The new link layer address matches the old one, so no need to update. */
                    if (physAddrMatch == TM_8BIT_YES)
                    {
                        updatePhysAddr = TM_8BIT_ZERO;
                    }
                    
                }
/*        
 * 8.2. If no link layer address has been set (no 'Neighbor Cache' entry
 *      exists), go ahead and add it by copying the link layer address in the
 *      option to the routing entry (rteHSPhysAddr), setting the
 *      TM_RTE_LINK_LAYER flag and setting the reachability state to STALE.
 *      ([RFC2461].R8.3:40, [RFC2461].R8.3:30)
 */
                if (updatePhysAddr == TM_8BIT_YES)
                {
                    tfRtArpAddByRte(
                        rtePtr,
                        physAddrPtr,
                        TM_RTE_INF, /* IPv6 uses rteHSLastReachedTickCount */
                        physAddrLen);
                    rtePtr->rte6HSNudState = TM_6_NUD_STALE_STATE;
                }
            }
#endif /* ! TM_6_USE_NUD */

        } /* Source address matches next hop address */
        else
        {
            errorCode = TM_EINVAL;
        }
    } /* Routing entry found */
    else
    {
        errorCode = TM_ENOENT;
    }

/* 9. Unlock the routing tree. */
    tfRtTreeUnlockNResetCaches();
    return errorCode;
}

/*
 * tf6RtKeyCommand Function Description:
 * Accesses the specified prefix routing entry.  Used to retrieve & update
 * lifetimes.  The follow commands are supported:
 *
 *   TM_6_RT_KEY_GET_LIFETIMES:
 *     Returns the current valid and preferred lifetimes.
 *     
 *   TM_6_RT_KEY_SET_LIFETIMES:
 *     Updates the valid and preferred lifetimes for this prefix.
 *
 *   TM_6_RT_KEY_EXISTS:
 *     Returns TM_ENOERROR if the specified entry exists.
 *   
 * Parameters
 * Parameter        Description
 * keyCmdPtr        Pointer to structure to routing key command to execute.
 * Returns
 * 0                Command executed successfully.
 * TM_ENOENT        Routing entry for this prefix was not found.
 * TM_EINVAL        Unrecognized command in rkc6Command.
 */
int tf6RtKeyCommand(tt6RtKeyCommandPtr keyCmdPtr,
                    tt8Bit             needLock)
{
    ttRteEntryPtr    rtePtr;
#ifdef TM_6_USE_PREFIX_DISCOVERY
    ttRtRadixNodePtr nodePtr;
#endif /* TM_6_USE_PREFIX_DISCOVERY */
    int              errorCode;
#ifdef TM_6_USE_PREFIX_DISCOVERY
    tt16Bit          flags;
#endif /* TM_6_USE_PREFIX_DISCOVERY */
    tt8Bit           command;
    
    tm_assert(tf6RtKeyCommand, keyCmdPtr != (tt6RtKeyCommandPtr)0);
    tm_assert(tf6RtKeyCommand, keyCmdPtr->rkc6AddrPtr != (tt6IpAddressPtr)0);

    errorCode = TM_ENOERROR;
    
/* Need exclusive access to the routing tree */
    if (needLock == TM_8BIT_YES)
    {
        tm_call_lock_wait(&tm_context(tvRtTreeLock));
    }

/* Try to find an exact match on the IP address + prefix length */
    rtePtr = tfRtFindIpAddrIpNet(
#ifdef TM_USE_STRONG_ESL
                                 keyCmdPtr->rkc6DevPtr,
#endif /* TM_USE_STRONG_ESL */
                                 keyCmdPtr->rkc6AddrPtr,
                                 keyCmdPtr->rkc6PrefixLength
                                 );

    command = keyCmdPtr->rkc6Command;
#ifdef TM_6_USE_PREFIX_DISCOVERY
    flags = keyCmdPtr->rkc6Flags;
    keyCmdPtr->rkc6Flags = TM_16BIT_ZERO; /* assume on-link */
#endif /* TM_6_USE_PREFIX_DISCOVERY */
    if (command != TM_6_RT_KEY_SET_ENDPOINT)
/* SET_ENDPOINT only on an on-link route, which may or may not be local */
    {
        if (    (rtePtr != TM_RTE_NULL_PTR)
             && tm_16bit_bits_not_set(rtePtr->rteFlags, TM_RTE_LOCAL)
           )
        {
/* Only lookup local routes when commands is not SET_ENDPOINT */
            rtePtr = TM_RTE_NULL_PTR;
        }

#ifdef TM_6_USE_PREFIX_DISCOVERY
        if (    tm_16bit_one_bit_set(flags, TM_6_RKC_NON_ONLINK)
             && (rtePtr == (ttRteEntryPtr)0) )
        {
/* search non on link list, if we failed to find an on-link local route */
            nodePtr = tf6RtGetNonOnLink(keyCmdPtr->rkc6AddrPtr,
                                        keyCmdPtr->rkc6PrefixLength);
            if (nodePtr != (ttRtRadixNodePtr)0)
            {
/* If an entry was found (nodePtr != NULL), map to routing entry. */
                rtePtr = tm_rt_leaf_to_rte(nodePtr);
/* Routing entry is non on-link */
                keyCmdPtr->rkc6Flags = TM_6_RKC_NON_ONLINK;
            }
        }
#endif /* TM_6_USE_PREFIX_DISCOVERY */
    }
    
    if (rtePtr != TM_RTE_NULL_PTR)
    {

        switch (command)
        {
        case TM_6_RT_KEY_GET_LIFETIMES:
            keyCmdPtr->rkc6ValidLifetime = rtePtr->rteTtl;
#ifdef TM_6_USE_PREFIX_DISCOVERY
            keyCmdPtr->rkc6PrefLifetime  = rtePtr->rte6HSPreferredLifetime;
#endif /* TM_6_USE_PREFIX_DISCOVERY */
            break;
                
        case TM_6_RT_KEY_SET_LIFETIMES:
            rtePtr->rteTtl = keyCmdPtr->rkc6ValidLifetime;
#ifdef TM_6_USE_PREFIX_DISCOVERY
            rtePtr->rte6HSPreferredLifetime = keyCmdPtr->rkc6PrefLifetime;
#endif /* TM_6_USE_PREFIX_DISCOVERY */
            break;
                
        case TM_6_RT_KEY_EXISTS:
/* Just do nothing, TM_ENOERROR will be returned since the key was found. */
            break;

        case TM_6_RT_KEY_EXISTS_ON_DEV:
/* check if we find the specified local route associated with the specified
   outgoing interface */
            if (keyCmdPtr->rkc6DevPtr != rtePtr->rteDevPtr)
            {
/* no, it is associated with a different interface */
                errorCode = TM_ENOENT;
            }
            break;

                
        case TM_6_RT_KEY_SET_ENDPOINT:
            tm_assert(tf6RtKeyCommand, keyCmdPtr->rkc6EndpointAddrPtr != 0);
            tm_6_ip_copy( keyCmdPtr->rkc6EndpointAddrPtr, 
                          &rtePtr->rteHSTunnelExitAddr );
            break;   
                
        default:
            errorCode = TM_EINVAL;
        }
    }
    else
    {
        errorCode = TM_ENOENT;
    }

/* Unlock the tree to release exclusive access */
    if (needLock == TM_8BIT_YES)
    {
        tfRtTreeUnlock();
    }
    
    return errorCode;
}

int tf6RtArpAddEntry ( 
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr  devPtr,
#endif /* TM_USE_STRONG_ESL */
    ttIpAddressPtr    ipAddressPtr,
    tt8BitPtr         physAddrPtr,
    tt32Bit           ttl,
    tt8Bit            physAddrLen 
    )
{
auto    ttRtCacheEntry  rtcCache;

    tm_bzero(&rtcCache, sizeof(ttRtCacheEntry));
#ifdef TM_USE_STRONG_ESL
    rtcCache.rtcDevPtr = devPtr;
#endif /* TM_USE_STRONG_ESL */
    tm_6_ip_copy_dststruct(ipAddressPtr, rtcCache.rtcDestIpAddr);
    return tfRtArpAddEntry(&rtcCache, physAddrPtr, ttl, physAddrLen); 
}

/*
 * tfNgRtDestExists Function Description:
 * Called by the user to find out if there exists a route to a given
 * destination (given by destination sock addr/prefix length).
 * Parameters
 * Parameter        Description
 * ipAddress        Destination IP address
 * netMask          Destination netmask
 * Returns
 * 0                Route to destination IP address found
 * TM_EHOSTUNREACH  no Route to the destination IP address.
 */
int tfNgRtDestExists(
#ifdef TM_USE_STRONG_ESL
    ttUserInterface                         interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
    const struct sockaddr_storage TM_FAR *  sockAddrPtr,
    int                                     prefixLen )
{
    ttRteEntryPtr    rtePtr; 
    int              errorCode;

#ifdef TM_USE_STRONG_ESL
    if (    (interfaceHandle != (ttUserInterface)0)
         && (tfValidInterface(interfaceHandle) != TM_ENOERROR))
    {
        errorCode = TM_EINVAL;
    }
    else
#endif /* TM_USE_STRONG_ESL */
    {
        errorCode = TM_EHOSTUNREACH;
        tm_call_lock_wait(&tm_context(tvRtTreeLock));
        rtePtr = tfRtFindSockAddr(
#ifdef TM_USE_STRONG_ESL
                                  (ttDeviceEntryPtr)interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                                  sockAddrPtr,
                                  prefixLen );
        if (rtePtr != (ttRteEntryPtr)0)
        {
            errorCode = TM_ENOERROR;
        }
        tfRtTreeUnlock(); 
    }
    return errorCode;
}


#ifdef TM_6_USE_PREFIX_DISCOVERY
/*
 * tf6RtUpdateNonOnLink
 * Function Description
 * Updates the lifetimes of a non-on-link prefix, adding an entry if necessary.
 * Called with the device lock on.
 *
 * 1. Lock the routing tree.
 * 2. Search through the non-on-link entry for an entry matching the specified
 *    prefix.
 * 3. If an entry was found (nodePtr != NULL), update it's lifetimes.
 * 4. If no entry was found (nodePtr == NULL), need to create a new entry:
 * 5. Allocate a new routing entry, copy the destination address to the new
 *    entry and add this entry to the head of the list.  Routing tree already
 *    locked.
 *     5.1. If we fail to allocate a routing entry, return TM_ENOBUFS;
 *     5.2. Set the entry's prefix length and lifetimes.
 *     5.3. Add this entry to the head of the list of 'non-on-link' prefixes.
 * 6. Unlock the routing tree.
 * 
 * Parameters
 * Parameter          Description
 * prefixAddrPtr      Pointer to the prefix address.
 * prefixLen          Prefix length.
 * validLifetime      Valid lifetime of the prefix.
 * preferredLifetime  Preferred lifetime of the prefix.
 *
 * Returns
 * TM_ENOERROR  Prefix lifetimes updated successfully.
 * TM_ENOBUFS   Couldn't allocate new routing entry.
 */
int tf6RtUpdateNonOnLink(ttDeviceEntryPtr   devPtr,
                         tt6IpAddressPtr    prefixAddrPtr,
                         tt8Bit             prefixLen,
                         tt32Bit            validLifetime,
                         tt32Bit            preferredLifetime,
                         tt8Bit             ipsecAuthFlag)
{
    ttRtRadixNodePtr nodePtr;
    ttRtRadixNodePtr nextNodePtr;    
    ttRteEntryPtr    rtePtr;
    int              errorCode;
    ttGenericUnion   userParm1, userParm2;
    tt8Bit           needUnDeprecate;

#ifndef TM_LOCK_NEEDED
    TM_UNREF_IN_ARG(devPtr);
#endif /* TM_LOCK_NEEDED */
    errorCode = TM_ENOERROR;
    needUnDeprecate = TM_8BIT_NO;
/* Moved initialization of userParm1 and userParm2 for compiler warnings */
    userParm1.gen32bitParm = prefixAddrPtr->s6LAddr[0];
    userParm2.gen32bitParm = prefixAddrPtr->s6LAddr[1];
    
/* 1. Lock the routing tree. */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));    
    
/*    
 * 2. Search through the non-on-link entry for an entry matching the specified
 *    prefix.
 */
    nodePtr = tf6RtGetNonOnLink(prefixAddrPtr, prefixLen);

/* 3. If an entry was found (nodePtr != NULL), update it's lifetimes. */
    if (nodePtr != (ttRtRadixNodePtr)0)
    {
        rtePtr = tm_rt_leaf_to_rte(nodePtr);

        if (ipsecAuthFlag == TM_8BIT_YES)
        {
            rtePtr->rteTtl  = validLifetime;
        }
        else
        {
/*    
 * If this advertisement was not authenticated,  the new lifetime must be
 * greater than the previous lifetime or greater than 2 hours.  If not, the
 * valid lifetime should be updated to 2 hours. [RFC2462].R5.5.3:100
 */
            if (    (validLifetime > TM_6_MIN_UNAUTH_PREF_LIFETIME)
                    || (validLifetime > rtePtr->rteTtl) )
            {
                rtePtr->rteTtl = validLifetime;
            }
            else
            {
                rtePtr->rteTtl = TM_6_MIN_UNAUTH_PREF_LIFETIME;
            }
        }

        if ((rtePtr->rte6HSPreferredLifetime == 0) && (preferredLifetime != 0))
        {
/*    
 * If the routing entry was in a deprecated state, call tf6UnDeprecatePrefix
 * if a non-zero preferred lifetime has been assigned.
 */
            needUnDeprecate = TM_8BIT_YES;
        }

        rtePtr->rte6HSPreferredLifetime = preferredLifetime;
    }
    else
    {
        
/*
 * 4. If no entry was found (nodePtr == NULL), need to create a new entry:
 * 5. Allocate a new routing entry, copy the destination address to the new
 *    entry and add this entry to the head of the list.  Routing tree already
 *    locked.
 */
        rtePtr = tf6RtAllocNLockNCopy(
            TM_8BIT_ZERO, TM_IP_ZERO, prefixAddrPtr);
         
        if (rtePtr != TM_RTE_NULL_PTR)
        {
/* 5.2. Set the entry's prefix length and lifetimes. */
            nodePtr = tm_rt_rte_to_leaf(rtePtr);
            tm_6_ip_copy(prefixAddrPtr, &nodePtr->rtnLSKey);
/* the prefix aging timer uses rteHSDevIpAdd */
            tm_6_ip_copy(prefixAddrPtr, &(rtePtr->rteHSDevIpAdd));
            nodePtr->rtnPrefixLength = prefixLen;
            rtePtr->rteTtl  = validLifetime;
            rtePtr->rte6HSPreferredLifetime = preferredLifetime;
/* the prefix aging timer only processes local route entries */
            rtePtr->rteFlags = TM_RTE_LOCAL;
              
/* 5.3. Add this entry to the head of the list of 'non-on-link' prefixes. */
            nextNodePtr =
                tm_context(tvRtRadixTreeHead).rth6NonOnLinkPrefixList;
            tm_context(tvRtRadixTreeHead).rth6NonOnLinkPrefixList = nodePtr;
            nodePtr->rtnNextPtr = nextNodePtr;
        }
        else
        {
/* 5.1. If we fail to allocate a routing entry, return TM_ENOBUFS; */
            errorCode = TM_ENOBUFS;
        }
    }
    
/* 6. Unlock the routing tree. */
    tfRtTreeUnlock();
    
    if (needUnDeprecate != TM_8BIT_NO)
    {
        tm_call_unlock(&(devPtr->devLockEntry));
        tf6PrefixTimerCommon(userParm1, userParm2, tf6UnDeprecatePrefix);
        tm_call_lock_wait(&(devPtr->devLockEntry));
    }
    return errorCode;
}

/*
 * tf6RtCheckNonOnLink
 * Function Description
 * Check if the specified prefix exists in the 'non-on-link' prefix list.
 *
 * 1. Lock the routing tree.
 * 2. Search through the non-on-link entry for an entry matching the specified
 *    prefix.
 * 3. Unlock the routing tree.
 * 
 * Parameters
 * Parameter      Description
 * prefixAddrPtr  Pointer to the prefix address.
 * prefixLen      Prefix length.
 *
 * Returns
 * TM_ENOERROR  Prefix entry exists in list.
 * TM_ENOENT    Prefix entry does not exist in the 'non-on-link' list.
 */
int tf6RtCheckNonOnLink(tt6IpAddressPtr prefixAddrPtr,
                        tt8Bit          prefixLen)
{
    int              errorCode;
    ttRtRadixNodePtr nodePtr;

/* 1. Lock the routing tree. */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));
    
/*    
 * 2. Search through the non-on-link entry for an entry matching the specified
 *    prefix.
 */
    nodePtr = tf6RtGetNonOnLink(prefixAddrPtr, prefixLen);
    if ( nodePtr != (ttRtRadixNodePtr)0)
    {
        errorCode = TM_ENOERROR;
    }
    else
    {
        errorCode = TM_ENOENT;
    }
    
/* 3. Unlock the routing tree. */
    tfRtTreeUnlock();
    
    return errorCode;
}

/*
 * tf6RtGetNonOnLink
 * Function Description
 * get a pointer to an entry in the 'non-on-link' prefix list.
 * Called with the routing tree locked.
 *
 * Search through the non-on-link entry for an entry matching the specified
 * prefix.
 * 
 * Parameters
 * Parameter      Description
 * prefixAddrPtr  Pointer to the prefix address.
 * prefixLen      Prefix length.
 *
 * Returns
 * NULL
 * non NULL       Pointer to entry
 */
static ttRtRadixNodePtr tf6RtGetNonOnLink(tt6IpAddressPtr prefixAddrPtr,
                                          tt8Bit          prefixLen)
{
    ttRtRadixNodePtr nodePtr;

/*    
 * 2. Search through the non-on-link entry for an entry matching the specified
 *    prefix.
 */
    nodePtr = tm_context(tvRtRadixTreeHead).rth6NonOnLinkPrefixList;
    while ( nodePtr != (ttRtRadixNodePtr)0)
    {
        if (    tm_6_ip_match(prefixAddrPtr, &nodePtr->rtnLSKey)
             && (nodePtr->rtnPrefixLength == prefixLen))
        {
            break;
        }
        nodePtr = nodePtr->rtnNextPtr;
    }
    
    return nodePtr;
}

/*
 * tf6RtDelNonOnLink
 * Function Description
 * Removes an entry from the 'non-on-link' prefix list.
 *
 * 1. Lock the routing tree.
 * 2. Search through the non-on-link entry for an entry matching the specified
 *    prefix.
 * 3. If an entry was found, remove it from the list and free the entry back
 *    to the recycle list.
 * 4. Unlock the routing tree.
 * 
 * Parameters
 * Parameter      Description
 * prefixAddrPtr  Pointer to the prefix address.
 * prefixLen      Prefix length.
 *
 * Returns
 * TM_ENOERROR  Prefix entry removed successfully.
 * TM_ENOENT    Prefix entry does not exist in the 'non-on-link' list.
 */
int tf6RtDelNonOnLink(tt6IpAddressPtr prefixAddrPtr,
                      tt8Bit          prefixLen)
{
    int              errorCode;
    ttRtRadixNodePtr nodePtr;
    ttRtRadixNodePtr prevNodePtr;
    
/* 1. Lock the routing tree. */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));
    
/*    
 * 2. Search through the non-on-link entry for an entry matching the specified
 *    prefix.
 */
    nodePtr = tm_context(tvRtRadixTreeHead).rth6NonOnLinkPrefixList;
    prevNodePtr = (ttRtRadixNodePtr)0;
    errorCode = TM_ENOENT;
    while ( nodePtr != (ttRtRadixNodePtr)0)
    {
        if (    tm_6_ip_match(prefixAddrPtr, &nodePtr->rtnLSKey)
             && (nodePtr->rtnPrefixLength == prefixLen))
        {
            errorCode = TM_ENOERROR;
            break;
        }
        prevNodePtr = nodePtr;
        nodePtr = nodePtr->rtnNextPtr;
    }

/*        
 * 3. If an entry was found, remove it from the list and free the entry back
 *    to the recycle list.
 */
    if (nodePtr != (ttRtRadixNodePtr)0)
    {
        if (prevNodePtr == (ttRtRadixNodePtr)0)
        {
/* First entry in the list */            
            tm_context(tvRtRadixTreeHead).rth6NonOnLinkPrefixList =
                nodePtr->rtnNextPtr;
        }
        else
        {
            prevNodePtr->rtnNextPtr = nodePtr->rtnNextPtr;
        }
        tm_recycle_free(nodePtr, TM_RECY_RTE);
    }
        
/* 4. Unlock the routing tree. */
    tfRtTreeUnlock();
    
    return errorCode;
}

/****************************************************************************
* FUNCTION: tf6RtPrefixAgingTimer
*
* PURPOSE: 
*   This function locks the routing tree so that it can walk and age all of
*   the IPv6 local route entries in the routing tree for which the netmask is
*   not all 1's (i.e. prefix length < 128), and also ages entries in the
*   non-on-link prefix list (i.e. ttRtRadixHeadEntry.rth6NonOnLinkPrefixList),
*   and as a consequence of local route aging, transitions the state of IPv6
*   address prefixes from preferred to deprecated (when
*   ttRteEntry.rte6HSPreferredLifetime becomes 0) to invalid (when
*   ttRteEntry.rteTtl becomes 0). The IPv6 addresses that have been manually
*   and auto-configured using the affected address prefixes go through the same
*   state transitions as the associated prefixes. This function creates a new
*   one-shot timer to perform each state transitions. This timer is set to
*   expire immediately, and when it expires, it calls either
*   tf6RtDeprecatePrefixTimer or tf6RtInvalidatePrefixTimer to do all of the
*   work of deprecating or invalidating the address prefix and its associated
*   addresses, including deleting the local route entry from the routing table,
*   if the prefix has been invalidated.
*
* PARAMETERS:
*   prefixAgingTimerPtr:
*       Pointer to the address invalidation timer that just expired.
*   userParm1:
*       Not used.
*   userParm2:
*       Not used.
*
* RETURNS:
*   Nothing
*
* NOTES:
*
****************************************************************************/
void tf6RtPrefixAgingTimer(
    ttVoidPtr prefixAgingTimerPtr,
    ttGenericUnion timerParm1,
    ttGenericUnion timerParm2 )
{
    ttRtRadixNodePtr nodePtr;
    ttRtRadixNodePtr prevNodePtr;
    ttRteEntryPtr    rtePtr;
    ttRtRadixNodePtr nextNodePtr;

    tm_trace_func_entry(tf6RtPrefixAgingTimer);

/* supress compiler warnings */
    TM_UNREF_IN_ARG(prefixAgingTimerPtr);
    TM_UNREF_IN_ARG(timerParm1);
    TM_UNREF_IN_ARG(timerParm2);

/* lock the routing tree. */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));

    (void)tfRtTreeWalk(tfRtPrefixAgingCB, TM_6_IPADDR_NULL_PTR, TM_8BIT_NO);

/* process the non-on-link prefixes */
    nodePtr = tm_context(tvRtRadixTreeHead).rth6NonOnLinkPrefixList;
    prevNodePtr = (ttRtRadixNodePtr)0;
    while ( nodePtr != (ttRtRadixNodePtr)0)
    {
        rtePtr = tm_rt_leaf_to_rte(nodePtr);
/* cannot access next after freeing entry */
        nextNodePtr = nodePtr->rtnNextPtr; 
        (void) tfRtPrefixAgingCB(rtePtr, (ttVoidPtr) 0);

/* check to see if this non-on-link prefix has been invalidated, in which case
   we remove it fromm the non-on-link prefix list */
        if (rtePtr->rteTtl == TM_UL(0))
        {
            if (prevNodePtr == (ttRtRadixNodePtr)0)
            {
/* First entry in the list */            
                tm_context(tvRtRadixTreeHead).rth6NonOnLinkPrefixList =
                    nodePtr->rtnNextPtr;
            }
            else
            {
                prevNodePtr->rtnNextPtr = nodePtr->rtnNextPtr;
            }
            tm_recycle_free(nodePtr, TM_RECY_RTE);
        }
        else
        {
/* Move prevNodePtr only if entry has NOT been removed */
            prevNodePtr = nodePtr;
        }
        nodePtr = nextNodePtr;
    }

/* unlock the routing tree. */
    tfRtTreeUnlockNResetCaches();
    
    tm_trace_func_exit(tf6RtPrefixAgingTimer);
    return;
}

static int tfRtPrefixAgingCB(
    ttRteEntryPtr   rtePtr,
    ttVoidPtr       voidPtr)
{
    tt6IpAddressPtr ipAddrPtr;
    ttGenericUnion  timerParm1, timerParm2;

/* supress compiler warning */
    TM_UNREF_IN_ARG(voidPtr);

    if ( tm_16bit_bits_not_set(rtePtr->rteFlags, TM_RTE_LOCAL))
    {
/* we only care about local route entries here */
        goto rtPrefixAgingCBFinish;
    }

    ipAddrPtr = &(rtePtr->rteHSDevIpAdd);
    if (IN6_IS_ADDR_V4MAPPED(ipAddrPtr))
    {
/* we only care about IPv6 local route entries here */
        goto rtPrefixAgingCBFinish;
    }

/* setup to call the prefix timer functions */
    timerParm1.gen32bitParm = ipAddrPtr->s6LAddr[0];
    timerParm2.gen32bitParm = ipAddrPtr->s6LAddr[1];

    if (rtePtr->rteTtl != TM_RTE_INF)
    {
        if (rtePtr->rteTtl > (tt32Bit)tm_context(tv6PrefixTimerResSec))
        {
            rtePtr->rteTtl -= (tt32Bit)tm_context(tv6PrefixTimerResSec);
        }
        else
        {
/* invalidating the prefix should cause it to be removed from the routing tree
   or from the non-on-link prefix list if it is non-on-link */
            rtePtr->rteTtl = TM_UL(0);

/* invalidate the IPv6 address prefix */
            (void) tfTimerAdd(
                tf6RtInvalidatePrefixTimer,
                timerParm1,
                timerParm2,
                0, /* expire immediately */
                0);

            goto rtPrefixAgingCBFinish;
        }
    }

    if (rtePtr->rte6HSPreferredLifetime != TM_RTE_INF)
    {
        if (rtePtr->rte6HSPreferredLifetime >
                (tt32Bit)tm_context(tv6PrefixTimerResSec))
        {
            rtePtr->rte6HSPreferredLifetime -=
                (tt32Bit)tm_context(tv6PrefixTimerResSec);
        }
        else
        {
            if (rtePtr->rte6HSPreferredLifetime != TM_UL(0))
            {
/* we should only deprecate the prefix once */
                rtePtr->rte6HSPreferredLifetime = TM_UL(0);

/* deprecate the IPv6 address prefix */
                (void) tfTimerAdd(
                    tf6RtDeprecatePrefixTimer,
                    timerParm1,
                    timerParm2,
                    0, /* expire immediately */
                    0);
            }
        }
    }

rtPrefixAgingCBFinish:
/*  Return TM_ENOENT to ask for the next entry. */
    return TM_ENOENT;
}

/* start the IPv6 Prefix Discovery prefix aging timer, if it hasn't been
   started yet. */
void tf6RtStartPrefixAgingTimer()
{
    ttGenericUnion      timerParm1;

    tm_call_lock_wait(&tm_context(tvRtTreeLock));

    if (tm_context(tv6RtPrefixAgingTimerPtr) == TM_TMR_NULL_PTR)
    {
/* For compiler warning */
        timerParm1.genVoidParmPtr = (ttVoidPtr)0;

/* start the IPv6 prefix aging timer */
        tm_context(tv6RtPrefixAgingTimerPtr) = tfTimerAdd(
            tf6RtPrefixAgingTimer,
            timerParm1, /* unused */
            timerParm1, /* unused */
            tm_context(tv6PrefixTimerResSec) * 1000,
            TM_TIM_AUTO );
    }

    tfRtTreeUnlock();
}
#endif /* TM_6_USE_PREFIX_DISCOVERY */

#ifdef TM_6_PMTU_DISC
/*
 * tf6RtNextHopMtu Function Description
 * Called by either ICMPv6 in response to a "Packet Too Big" message or called
 * by the user to disable PMTU for this destination.
 *
 * Parameters
 * Parameter    Description
 * ip6AddrPtr   Destination address to set the new PMTU value for.
 * nextHopMtu   The new PMTU for the specified destination.
 * isStaticFlag Set to true if the user is adding a PMTU entry.
 *
 * Returns
 * Value        Meaning
 * TM_ENOERROR  PMTU value updated/added successfully.
 *
 * 1. Lock the routing tree.
 * 2. Get routing entry for destination address.
 * 3. If no routing entry is found, return TM_EHOSTUNREACH.
 * 4. Check for the validity of the next hop MTU:
 *     4.1. If the next hop MTU is less than the IPv6 minimum, set the PMTU
 *          to the minimum. ([RFC1981].R4:40)
 *     4.2. Only allow the PMTU to be increased by the user; otherwise return
 *          TM_EPERM. ([RFC1981].R4:60)
 * 5. If an indirect (off-link) routing entry is found:
 *     5.1. If we're being called from the user, make this entry static and
 *          disable PMTU discovery for this destination.
 *     5.2. Call tfRtAddPathMtu entry to add or update the PMTU entry for
 *          this destination. ([RFC1981].R4:30)
 * 6. If a ARP (on-link) entry is found:
 *     6.1. If the current PMTU does not match the new MTU, set the PMTU "last
 *          updated" timestamp on the routing entry.
 *     6.2. There is no need to add any additional entries here, we can simply
 *          update the PMTU for this destination directly in the ARP entry.
 *          ([RFC1981].R4:30)  Do not set the TM_RTE_PMTU_DISC flag!
 *          rtePtr->rteTtl = nextHopMtu;
 * 7. If a routing entry was found, but was not an indirect entry or ARP
 *    entry, return TM_EPERM.
 */    
int tf6RtNextHopMtu(
#ifdef TM_USE_STRONG_ESL
                    ttDeviceEntryPtr     devPtr,
#endif /* TM_USE_STRONG_ESL */
                    tt6ConstIpAddressPtr ip6ConstAddrPtr,
                    tt16Bit              nextHopMtu,
                    tt8Bit               isStaticFlag)
{
    tt6IpAddress    ip6Addr;
    tt6IpAddressPtr ip6AddrPtr;
    ttRteEntryPtr   rtePtr;
    tt32Bit         timeout;
    tt16Bit         flags;
    tt16Bit         routeMtu;
    int             errorCode;

    tm_6_ip_copy(ip6ConstAddrPtr, &ip6Addr);
    ip6AddrPtr = &ip6Addr;
    
/* 1. Lock the routing tree. */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));
    
/* 2. Get routing entry for destination address. */
    rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                      devPtr,
#endif /* TM_USE_STRONG_ESL */
                      ip6AddrPtr);
    
/* 3. If no routing entry is found, return TM_EHOSTUNREACH. */
    if (rtePtr != TM_RTE_NULL_PTR)
    {
    
/* 4. Check for the validity of the next hop MTU:
 *     4.1. If the next hop MTU is less than the IPv6 minimum, set the PMTU
 *          to the minimum. ([RFC1981].R4:40)
 */
        if (nextHopMtu < TM_6_PMTU_MINIMUM)
        {
            if (isStaticFlag == TM_8BIT_ZERO)
            {
/* special IPv6 Path MTU discovery requirement to support IPv6-to-IPv4
   protocol translator that doesn't keep PMTU state: */
/* section 4 RFC-1981: A node may receive a Packet Too Big message reporting
   a next-hop MTU that is less than the IPv6 minimum link MTU. In that
   case, the node is not required to reduce the size of subsequent packets
   sent on the path to less than the IPv6 minimun link MTU, but rather must
   include a Fragment header in those packets. */
                nextHopMtu = TM_6_PMTU_MINIMUM - TM_6_IP_FRAG_EXT_HDR_LEN;
            }
            else
            {
                nextHopMtu = TM_6_PMTU_MINIMUM;
            }
        }

/* Need to protect the IPv6 MTU, since it could be changed dynamically
   due to a router advertisement. */
        tm_kernel_set_critical;
        routeMtu = rtePtr->rteMtu;
        tm_kernel_release_critical;
        
/*
 *     4.2. Only allow the user to increase the PMTU; otherwise return
 *          TM_EPERM. ([RFC1981].R4:60)
 */
        if ( (nextHopMtu > routeMtu) && (isStaticFlag == TM_8BIT_ZERO))
        {
            errorCode = TM_EPERM;
        }
        else
        {

/* 5. If an indirect (off-link) routing entry is found: 
 *     5.1. If we're being called from the user, make this entry static and
 *          disable PMTU discovery for this destination.
 */
            if (tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_INDIRECT))
            {
                if (isStaticFlag == TM_8BIT_YES)
                {
                    timeout = TM_RTE_INF;
                    flags =
                        TM_RTE_INDIRECT |
                        TM_RTE_STATIC |
                        TM_RTE_NO_PMTU_DISC;
                }
                else
                {
                    timeout = tm_context(tv6RtPathMtuTimeout) * TM_UL(1000);
                    flags =
                        TM_RTE_INDIRECT |
                        TM_RTE_PMTU_DISC;
                }

/*
 *     5.2. Call tfRtAddPathMtu entry to add or update the PMTU entry for
 *          this destination. ([RFC1981].R4:30)
 */
                errorCode = tfRtAddPathMtuEntry(
#ifdef TM_USE_STRONG_ESL
                                                devPtr,
#endif /* TM_USE_STRONG_ESL */
                                                rtePtr,
                                                ip6AddrPtr,
                                                timeout,
                                                flags,
                                                nextHopMtu);

                tm_context(tvRtRadixTreeHead).rthResetCachesFlags 
                                            |= TM_RTE_INDIRECT;
                tm_context(tvRtRadixTreeHead).rthResetCachesAf 
                                            |= (AF_INET6);
            }
            else if (tm_16bit_one_bit_set(rtePtr->rteFlags,TM_RTE_CLONED))
            {
/* 6. If a cloned (ARP or tunnel) entry is found: */

/*           
 *     6.1. If the current PMTU does not match the new MTU, set the PMTU "last
 *          updated" timestamp on the routing entry.
 */
                if (routeMtu != nextHopMtu)
                {
                    tm_kernel_set_critical;
                    rtePtr->rte6HSLastPmtuTickCount = tvTime;
                    tm_kernel_release_critical;
                }

/*           
 *     6.2. There is no need to add any additional entries here, we can simply
 *          update the PMTU for this destination directly in the cloned entry.
 *          ([RFC1981].R4:30)  Do not set the TM_RTE_PMTU_DISC flag!
 */          
                rtePtr->rteMtu = nextHopMtu;

                tm_context(tvRtRadixTreeHead).rthResetCachesFlags 
                                            |= TM_RTE_CLONED;
                tm_context(tvRtRadixTreeHead).rthResetCachesAf 
                                            |= (AF_INET6);
                errorCode = TM_ENOERROR;
            }
            else
            {
/*           
 * 7. If a routing entry was found, but was not an indirect entry or cloned
 *    entry, return TM_EPERM.
 */
                errorCode = TM_EPERM;
            }
        }
    }
    else
    {
        errorCode = TM_EHOSTUNREACH;
    }

    tfRtTreeUnlockNResetCaches();
    
    return errorCode;
}

#endif /* TM_6_PMTU_DISC */


#endif /* TM_USE_IPV6 */

/* 
 * IPv4 only functions
 */
#ifdef TM_USE_IPV4

/*
 * tfRtDestExists Function Description:
 * Called by the user to find out if there exists a route to a given
 * destination (given by destination IP address/destination netmask).
 * Parameters
 * Parameter        Description
 * ipAddress        Destination IP address
 * netMask          Destination netmask
 * Returns
 * 0                Route to destination IP address found
 * TM_EHOSTUNREACH  no Route to the destination IP address.
 */
int tfRtDestExists(
#ifdef TM_USE_STRONG_ESL
    ttUserInterface interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
    ttUserIpAddress ipAddress,
    ttUserIpAddress netMask )
{
    ttRteEntryPtr   rtePtr;
    int             errorCode;

#ifdef TM_USE_STRONG_ESL
    if (    (interfaceHandle != (ttUserInterface)0)
         && (tfValidInterface(interfaceHandle) != TM_ENOERROR))
    {
        errorCode = TM_EINVAL;
    }
    else
#endif /* TM_USE_STRONG_ESL */
    {
        errorCode = TM_EHOSTUNREACH;
/* Destination network */
        tm_ip_copy(tm_ip_net(ipAddress, netMask), ipAddress);
/* LOCK the tree to obtain exclusive access to the routing table */
        tm_call_lock_wait(&tm_context(tvRtTreeLock));
#ifdef TM_USE_IPV6
        rtePtr = tf4RtFind(
#ifdef TM_USE_STRONG_ESL
                           interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                           ipAddress);
        if (rtePtr != TM_RTE_NULL_PTR)
        {
            if (tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_REJECT))
            {
                rtePtr = TM_RTE_NULL_PTR;
            }
        }
#else /* ! TM_USE_IPV6 */
        rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                          interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                          ipAddress);
#endif /* ! TM_USE_IPV6 */
        if (rtePtr != TM_RTE_NULL_PTR)
        {
            errorCode = TM_ENOERROR;
        }
/* UNLOCK the tree to release access to the routing table */
        tfRtTreeUnlock();
    }
    return errorCode;
}

/*
 * Function called from the socket interface to delete a static route from the
 * routing table and by tfRemoveInterface() to delete a local route from the
 * routing table.
 * Parameters:
 * DestIpAddress: Destination IP address (key) of the route
 * myNetMask:     IP network mask of the route
 * rteCreateFlag: Deletor ID, one of TM_RTE_STATIC, TM_RTE_LOCAL
 * Return values:
 *   0 on success
 *   TM_ENOENT if routing enty cannot be found
 *
 */
int tf4RtDelRoute ( 
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
    tt4IpAddress     destIpAddress,
    tt4IpAddress     destNetMask,
    tt16Bit          rteCreateFlag )
{
    ttRteEntryPtr      rtePtr;
    int                errorCode;

    errorCode = TM_ENOERROR;
#ifdef TM_ERROR_CHECKING
    if (tm_context(tvRtTreeHeadPtr) == TM_RADIX_NULL_PTR)
    {
        tfKernelError("tf4RtDelRoute",
                      "Routing table corrupted or not initialized");
/* Severe Error */
        tm_thread_stop;
    }
#endif /* TM_ERROR_CHECKING */
/* LOCK the tree to obtain exclusive access to the routing table */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));
/*
 * Check whether this IP address/network mask is in the routing table.
 */
#ifdef TM_USE_IPV6
    rtePtr = tf4RtFindIpAddrIpNet(
#ifdef TM_USE_STRONG_ESL
                                  devPtr,
#endif /* TM_USE_STRONG_ESL */
                                  destIpAddress, destNetMask);
#else /* ! TM_USE_IPV6 */
    rtePtr = tfRtFindIpAddrIpNet(
#ifdef TM_USE_STRONG_ESL
                                 devPtr,
#endif /* TM_USE_STRONG_ESL */
                                 destIpAddress, destNetMask);
#endif /* ! TM_USE_IPV6 */
    if (rtePtr != TM_RTE_NULL_PTR)
/* If entry already exists in the routing table */
    {
#ifndef TM_USE_IPV6
/* 
 * Note that if TM_USE_IPV6 is defined then IPV4 tfDelDefaultGateway calls
 * tf4RtDelDefGw, and does not call tf4RtDelRoute 
 */
        if (    tm_ip_zero(destNetMask) /* called by tfDelDefaultGateway */
/* user specified gateway (non zero) */
             && tm_ip_not_zero(destIpAddress)
/* user specified gateway, different from current route gateway */
             && tm_ip_not_match(destIpAddress, rtePtr->rteGSGateway) )
        {
            errorCode = TM_ENOENT;
        }
        else
#endif /* !TM_USE_IPV6 */
        {
            errorCode = tfRtDelRoute(rtePtr, rteCreateFlag);
        }
    }
    else
    {
        errorCode = TM_ENOENT;
    }
/* UNLOCK the tree to release access to the routing table */
    tfRtTreeUnlockNResetCaches();
    return errorCode;
}

/*
 * Function called to add a local route in the routing table.
 * Parameters:
 * devPtr: pointer to device entry for the local route
 * Destination IP address: IP address of the network or of the remote
 * side for Point to point network.
 * myNetMask: IP network mask to be used on this device
 * myIpAddress: IP address to be used on this device
 * myMhomeIndex: Multihome Index on this device for IP address
 * Return values:
 * 0 on success
 * TM_EINVAL for parameter problems.
 */
int tf4RtAddLocal ( ttDeviceEntryPtr devPtr,
                    tt4IpAddress     destIpAddress,
                    tt4IpAddress     myNetMask,
                    tt4IpAddress     myIpAddress,
                    tt16Bit          myMhomeIndex )
{
    int                 errorCode;
    tt16Bit             flags;

    if (    tm_ip_is_loop_back(myIpAddress)
         && (devPtr != tm_context(tvLoopbackDevPtr)) )
    {
        errorCode = TM_EADDRNOTAVAIL;
    }
    else
    {
/* Allow cloning for local LAN link layer address. */
        flags = TM_RTE_LOCAL; /* default case */
        if (tm_4_ll_is_broadcast(devPtr))
        {
          if (tm_ip_match(myNetMask, TM_IP_HOST_MASK))
          {
                if (tm_4_ll_is_lan(devPtr))
                {
                    flags = TM_RTE_LOCAL | TM_RTE_CLONED | TM_RTE_LINK_LAYER;
                }
                /* else default TM_RTE_LOCAL */
          }
          else
          {
                flags = TM_RTE_CLONE | TM_RTE_LOCAL;
          }
        } /* else default TM_RTE_LOCAL */
        errorCode = tf4RtAddRoute( devPtr,
                                   destIpAddress,
                                   myNetMask,
                                   myIpAddress,
                                   TM_16BIT_ZERO,
                                   myMhomeIndex, 0, TM_RTE_INF,
                                   flags, TM_8BIT_YES );
    }
    return errorCode;
}

/*
 * Called by tfAddStaticRoute(), tf4RtAddLocal, tf4RtRedirect, tfRtRip,
 * BOOTP/DHCP, MOBILIP, ICMP, SNMP to add a new routing entry
 * to the routing table (Patricia tree).
 * 1) Call tfRtAllocNLockNCopy to allocate a new routing entry, lock
 *    the Patricia tree, and for IPV6 only copy the IPv4-mapped IPV6 
 *    address in the routing entry destination field.
 * 2) Copy gateway address in routing entry
 * 3) call tfRtAddEntry() to insert the entry in the Patricia tree.
 * 4) Call tfRtCacheUnlockNClean to cache the entry in the SNMP routing
 *    cache, unlock the tree, and unget the routing entry if we failed
 *    to insert in the tree.
 * Parameters
 * devPtr: device for the route 
 * ipAddr: Destination Ip address 
 * netMask: destination network address 
 * mHomeIndex: multihome index for local routes
 * gateway: Gateway IP address for non local routes, interface IP address
 *          for local routes.
 * tag: Used by tfRtRip.
 * hops: number of hops to destination (0 for local route)
 * ttl: Time to live for the route entry
 * flags: flag to indicate creator local/static/redirect/RIP, host route etc.
 *
 * Return value:
 * O on success
 * TM_ENOBUFS, when routing entry could not be allocated
 */
int tf4RtAddRoute (
    ttDeviceEntryPtr        devPtr, /* device for the route */
/* Destination Ip address, key */
    tt4IpAddress            ipAddress,
/* destination network address */
    tt4IpAddress            netMask,
/* Gateway for non local routes, interface IP address for local routes, etc. */
    tt4IpAddress            gateway,
    tt16Bit                 gwayTag,
/* multihome index for local routes */
    tt16Bit                 mhomeIndex,
    int                     hops, /* metrics for non local routes */
    tt32Bit                 ttl,
/* flag to indicate creator local/static/redirect/RIP */
    tt16Bit                 flags,
    tt8Bit                  needLock )
{
    ttRteEntryPtr       rtePtr;
#ifdef TM_USE_IPV6
    int                 prefixLength;
#endif /* TM_USE_IPV6 */
    int                 errorCode;

#ifdef TM_USE_IPV6
/* Allocate routing entry, and initialize destination IP address */
    rtePtr = tf6RtAllocNLockNCopy(needLock, ipAddress, (tt6IpAddressPtr)0);
#ifdef TM_4_USE_SCOPE_ID
    tm_4_dev_scope_addr(&(rtePtr->rteDest), devPtr);
#endif /* TM_4_USE_SCOPE_ID */
#else /* !TM_USE_IPV6 */
    rtePtr = tfRtAllocNLockNCopy(needLock);
#endif /* !TM_USE_IPV6 */
    if (rtePtr != TM_RTE_NULL_PTR)
    {
        rtePtr->rteHSPhysAddrLen = devPtr->devPhysAddrLength;
#ifdef TM_USE_IPV6
/* Prefix length of the IPV4 address */
        prefixLength = tf4NetmaskToPrefixLen(netMask);
/* Prefix length of the IPv4-mapped IPv6 address */
        prefixLength = tm_6_prefixlen_to_ipv4_mapped(prefixLength);
        if (tm_16bit_one_bit_set(flags, TM_RTE_LOCAL))
        {
            if (tm_16bit_all_bits_set(flags, TM_RTE_CLONED | TM_RTE_LINK_LAYER))
            {
/* Sending to our own IP address. Store link layer address. */
                tm_bcopy(devPtr->devPhysAddrArr,
                         &(rtePtr->rteHSPhysAddr[0]),
                         devPtr->devPhysAddrLength);
            }
            else
            {
                tm_6_addr_to_ipv4_mapped(gateway, &rtePtr->rteHSDevIpAdd);
#ifdef TM_4_USE_SCOPE_ID
                tm_4_dev_scope_addr(&rtePtr->rteHSDevIpAdd, devPtr);
#endif /* TM_4_USE_SCOPE_ID */
            }
#ifdef TM_6_USE_PREFIX_DISCOVERY
            rtePtr->rte6HSPreferredLifetime = TM_RTE_INF;
#endif /* TM_6_USE_PREFIX_DISCOVERY */
        }
        else
        {
            tm_6_addr_to_ipv4_mapped(gateway, &rtePtr->rteGSGateway);
#ifdef TM_4_USE_SCOPE_ID
            tm_4_dev_scope_addr(&rtePtr->rteGSGateway, devPtr);
#endif /* TM_4_USE_SCOPE_ID */
        }
#else /* !TM_USE_IPV6 */
        if (tm_16bit_one_bit_set(flags, TM_RTE_LOCAL))
        {
            if (tm_16bit_all_bits_set(flags, TM_RTE_CLONED | TM_RTE_LINK_LAYER))
            {
/* Sending to our own IP address. Store link layer address. */
                tm_bcopy(devPtr->devPhysAddrArr,
                         &(rtePtr->rteHSPhysAddr[0]),
                         devPtr->devPhysAddrLength);
            }
            else
            {
                tm_ip_copy(gateway, rtePtr->rteHSDevIpAdd);
            }
        }
        else
        {
            tm_ip_copy(gateway, rtePtr->rteGSGateway);
        }
#endif /* !TM_USE_IPV6 */
        rtePtr->rteGSTag = gwayTag;
        rtePtr->rteMtu = devPtr->devMtu;
        errorCode = tfRtAddEntry(
                            tm_context(tvRtTreeHeadPtr), rtePtr, devPtr,
#ifdef TM_USE_IPV6
                            prefixLength,
#else /* !TM_USE_IPV6 */
                            ipAddress, netMask,
#endif /* !TM_USE_IPV6 */
                            mhomeIndex,
                            hops, ttl, flags );
/*
 * Unlock the tree, cache entry if successful or free routing entry otherwise.
 * Ignore error for local route if errorCode is TM_EALREADY.
 */
        errorCode = tfRtCacheUnlockNClean(rtePtr, needLock, errorCode, flags);
    }
    else
    {
        errorCode =  TM_ENOBUFS;
    }
    return errorCode;
}

/*
 * IPV4 functions only
 */
/*
 * tfRtArpAddEntry function description:
 * Add/Set an ARP Entry to routing table for a given destination IP address.
 * Called by the ARP interface, or User interface to set a physical
 * address mapping
 * Parameters: ipAddress LAN Ip address to map
 *             physAddrPtr pointer to link layer address
 * return values:
 * 0                success
 * TM_EINVAL        bad parameter
 * TM_EHOSTUNREACH  No network for the ip address parameter
 */
int tf4RtArpAddEntry ( 
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr  devPtr,
#endif /* TM_USE_STRONG_ESL */
    tt4IpAddress      ipAddress,
    tt8BitPtr         physAddrPtr,
    tt32Bit           ttl,
    tt8Bit            physAddrLen 
    )
{
auto    ttRtCacheEntry  rtcCache;

    tm_bzero(&rtcCache, sizeof(ttRtCacheEntry));
#ifdef TM_USE_STRONG_ESL
    rtcCache.rtcDevPtr = devPtr;
#endif /* TM_USE_STRONG_ESL */
#ifdef TM_USE_IPV6
    tm_6_addr_to_ipv4_mapped(ipAddress, &(rtcCache.rtcDestIpAddr));
#else /* !TM_USE_IPV6 */
    tm_ip_copy(ipAddress, rtcCache.rtcDestIpAddr);
#endif /* !TM_USE_IPV6 */
    return tfRtArpAddEntry(&rtcCache, physAddrPtr, ttl, physAddrLen); 
}

/*
 * Function called from the socket interface to get the IPv4 default gateway
 * from the routing table
 * Parameters:
 * gatewayIpAddrPtr: pointer to gateway IP address to store result into.
 * Return values:
 *   0 on success
 *   TM_ENOENT if routing enty cannot be found
 *
 */
int tf4RtGetDefault (
#ifdef TM_USE_STRONG_ESL
                     ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                     tt4IpAddressPtr  gatewayIpAddrPtr,
                     ttIntPtr         hopsPtr
                    )
{
    ttRteEntryPtr      rtePtr;
    int                errorCode;

    errorCode = TM_ENOENT;
/* LOCK the tree to obtain exclusive access to the routing table */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));
/*
 * Check whether this IP address/network mask is in the routing table.
 */
#ifdef TM_USE_IPV6
/* The logic to get the IPv4 default gateway needs to change in dual IP
   layer mode: since there always is an entry for the IPv4 default gateway
   (i.e. ttRtRadixHeadEntry.rth4DefaultGatewayPtr, or 
   devPtr->dev4DefaultGatewayPtr), tfRtGetDefault must check to
   see if the flag TM_RTE_REJECT is set, in which case it has not been
   configured and tfRtGetDefault returns TM_ENOENT. */
#ifdef TM_USE_STRONG_ESL
    rtePtr = devPtr->dev4DefaultGatewayPtr;
#else /* !TM_USE_STRONG_ESL */
    rtePtr = tm_context(tvRtRadixTreeHead).rth4DefaultGatewayPtr;
#endif /* !TM_USE_STRONG_ESL */
    if ( tm_16bit_one_bit_set( rtePtr->rteFlags, TM_RTE_REJECT) )
    {
        rtePtr = TM_RTE_NULL_PTR;
    }
    else
    {
/* Copy an IPv4-mapped IPV6 address into an IPV4 address */
        tm_6_ip_mapped_copy_6to4(&(rtePtr->rteGSGateway), *gatewayIpAddrPtr);
        if (hopsPtr != (ttIntPtr)0)
        {
            *hopsPtr = (int)rtePtr->rteHops;
        }
        errorCode = TM_ENOERROR;
    }
#else /* ! TM_USE_IPV6 */
    rtePtr = tfRtFindIpAddrIpNet(
#ifdef TM_USE_STRONG_ESL
                                  devPtr,
#endif /* TM_USE_STRONG_ESL */
                                  TM_IP_ZERO, TM_IP_ZERO);
    if (rtePtr != TM_RTE_NULL_PTR)
/* If entry already exists in the routing table */
    {
        tm_ip_copy(rtePtr->rteGSGateway, *gatewayIpAddrPtr);
        if (hopsPtr != (ttIntPtr)0)
        {
            *hopsPtr = (int)rtePtr->rteHops;
        }
        errorCode = TM_ENOERROR;
    }
#endif /* ! TM_USE_IPV6 */
/* UNLOCK the tree to release access to the routing table */
    tfRtTreeUnlock();
    return errorCode;
}

/*
 * tfRtEchoReply Function Description:
 * Called by ICMP when an echo reply has been received 
 * If ipAddress is in routing table and entry is routing discovery,
 * RIP or Redirect, increase time to live
 */
void tfRtEchoReply (
#ifdef TM_USE_STRONG_ESL
                    ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                    tt4IpAddress     ipAddress)
{
   ttRteEntryPtr   rtePtr;

/* LOCK the tree to obtain exclusive access to the routing table */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));
#ifdef TM_USE_IPV6
    rtePtr = tf4RtFind(
#ifdef TM_USE_STRONG_ESL
                       devPtr,
#endif /* TM_USE_STRONG_ESL */
                       ipAddress);
#else  /* !TM_USE_IPV6 */
    rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                      devPtr,
#endif /* TM_USE_STRONG_ESL */
                      ipAddress);
#endif /* !TM_USE_IPV6 */
    if (rtePtr != TM_RTE_NULL_PTR)
    {
        if ( tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_ROUTE_DYNAMIC) )
/* RIP, Router Discovery, or REDIRECT gateway entry */
        {
            rtePtr->rteTtl = tm_context(tvRtTimeout); /* 3 minutes */
        }
    }
/* UNLOCK the tree to release access to the routing table */
    tfRtTreeUnlock();
}

#ifdef TM_PMTU_DISC
/*
 * tfRtNextHopMtu Function Description.
 * 1. Called by ICMP when a UNREACH for fragment arrives (iphPtr non null)
 * 2. Or called by the user tfDisablePathMtuDisc() when the user
 *    wants to disable PATH MTU discovery on a host route (iphPtr null).
 * Algorithm.
 * Lock the tree
 * Find route entry in routing table.
 * If routing entry is found:
 * . If function called by the user, set timeout value to infinity,
 *   and set new routing entry flags to static, and to disable
 *   path MTU.
 * . If function is called from ICMP, set timeout to default path MTU
 *   discovery timeout (10 minutes), and set new routing entry flags to
 *   dynamic path MTU discovery. If routing entry is a host entry,
 *   return an error if path MTU is not allowed (by setting errorCode).
 *   ErrorCode is checked in all next statements. Next check the next Hop MTU
 *   found in the ICMP header. If none was set (router
 *   which does not implement RFC1191), then find the next plateau MTU as
 *   described in RFC 1191. If we were already at the lowest possible MTU,
 *   set the flags to disable path MTU. If the next hop MTU is bigger than
 *   the previously cached MTU, do not allow a change in the MTU.
 *   If we are not going to change the MTU, and we were not changing the
 *   routing entry flags either, then set the errorCode to disallow
 *   the routing entry change/addition.
 * . If errorCode not set, call tfRtAddPathMtuEntry() to delete the current
 *   routing entry if it is a host entry, and to add new host routing
 *   with timeout value, flags, and MTU as found earlier.
 * . UnLock the tree
 * . Return the errorCode.
 * Parameters
 * Parameter    Description
 * ipAddress    destination IP address of the path MTU discovery
 * nextHopMtu   New Path MTU to try
 * iphPtr       Pointer to IP header when called from ICMP fragment
 *              error.
 *
 * Return values 
 * TM_ENOERROR      New host path MTU routing entry was added
 * TM_EHOSTUNREACH  Destination IP address unreachable
 * TM_ENOBUFS       Not enough memory to allocate new routing entry
 * TM_ENOENT        Could not insert host entry
 * TM_EPERM         Path MTU discovery not allowed.
 */
int tfRtNextHopMtu (
#ifdef TM_USE_STRONG_ESL
                     ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                     tt4IpAddress   ipAddress,
                     tt16Bit        nextHopMtu,
                     ttIpHeaderPtr  iphPtr )
{
    ttRteEntryPtr    rtePtr;
#ifdef TM_USE_IPV6
    ttIpAddress      ip6Addr;
#endif /* ! TM_USE_IPV6 */
    tt32Bit          timeout;
    int              errorCode;
    tt16Bit          oldIpHdrLength;
    tt16Bit          flags;
    tt16Bit          oldMtu;
    tt16Bit          rteMtu;

    errorCode = TM_ENOERROR;
#ifdef TM_USE_IPV6
/* Convert IPv4 address into an IPv4-mapped IPv6 address */
    tm_6_addr_to_ipv4_mapped(ipAddress, &ip6Addr);
#endif /* ! TM_USE_IPV6 */
/* LOCK the tree to obtain exclusive access to the routing table */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));
#ifdef TM_USE_IPV6
    rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                      devPtr,
#endif /* TM_USE_STRONG_ESL */
                      &ip6Addr);
#else /* ! TM_USE_IPV6 */
    rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                      devPtr,
#endif /* TM_USE_STRONG_ESL */
                      ipAddress);
#endif /* ! TM_USE_IPV6 */
    if (rtePtr != TM_RTE_NULL_PTR)
    {
        if ( tm_16bit_one_bit_set( rtePtr->rteFlags, TM_RTE_INDIRECT) )
/* Indirect routing entry found */
        {
            rteMtu = rtePtr->rteMtu;
            if (iphPtr == (ttIpHeaderPtr)0)
            {
/*
 * Called from the user, Make the host entry static. Disable Path MTU
 * discovery
 */
                timeout = TM_RTE_INF;
                flags =   TM_RTE_INDIRECT | TM_RTE_STATIC
                        | TM_RTE_NO_PMTU_DISC;
/* Note: MTU will Default to device MTU if not set by the user */
            }
            else
            {
/*
 * Called from ICMP. Make the host entry a dynamic path MTU discovery entry.
 * Do not disable Path MTU discovery
 */
                flags =  TM_RTE_INDIRECT | TM_RTE_PMTU_DISC;
/* 10 minutes */
                timeout = tm_context(tvRtDecreasedMtuTimeout);
                if (    tm_16bit_all_bits_set( rtePtr->rteFlags2,
                                               TM_RTE2_HOST | TM_RTE2_UP) )
/*
 * If there was a host entry to the remote, we cannot change the routing
 * entry if path MTU discovery was not allowed.
 */
                {
                    if ( tm_16bit_one_bit_set( rtePtr->rteFlags,
                                               TM_RTE_NO_PMTU_DISC) )
                    {
/* Disallow modification of the routing table */
                        errorCode = TM_EPERM;
                    }
                }
                if (errorCode == TM_ENOERROR)
/* Change allowed so far */
                {
                    if (nextHopMtu == 0)
/*
 * Router not implementing RFC 1191 (not setting the next hop MTU field).
 * We need to estimate the next lower Path MTU to try.
 */
                    {
/*
 * From RFC 1191:
 * The ICMP Destination Unreachable message, contains the IP header
 * of the original datagram, which contains the Total length of the
 * datagram that was too big to be forwarded without fragmentation.
 * Since this Total Length may be less than the current
 * PMTU estimate, but is nonetheless larger than the actual PMTU, it may
 * be a good input to the method for choosing the next PMTU estimate.
 */
                        tm_ntohs(iphPtr->iphTotalLength, oldMtu);
#ifdef TM_LINT
LINT_UNINIT_SYM_BEGIN(oldMtu)
#endif /* TM_LINT */
                        if (oldMtu > rteMtu)
#ifdef TM_LINT
LINT_UNINIT_SYM_END(oldMtu)
#endif /* TM_LINT */
                        {
/*
 * RFC 1191:
 * Note: routers based on implementations derived from 4.2BSD
 * Unix send an incorrect value for the Total Length of the
 * original IP datagram.  The value sent by these routers is the
 * sum of the original Total Length and the original Header
 * Length (expressed in octets).  Since it is impossible for the
 * host receiving such a Datagram Too Big message to know if it
 * sent by one of these routers, the host must be conservative
 * and assume that it is.  If the Total Length field returned is
 * not less than the current PMTU estimate, it must be reduced by
 * 4 times the value of the returned Header Length field.
 */
                            oldIpHdrLength =
                                            tm_ip_hdr_len(iphPtr->iphVersLen);
                            oldIpHdrLength = (tt16Bit)(4 * oldIpHdrLength);
                            if (oldMtu > oldIpHdrLength)
                            {
                                oldMtu = (tt16Bit)(oldMtu - oldIpHdrLength);
                            }
                            else
                            {
/*
 * Information from the ICMP error message cannot be trusted. Use the
 * cache value.
 */
                                oldMtu = rteMtu;
                            }
                        }
                        nextHopMtu = tfRtPickNextPlateauMtu( oldMtu,
                                                             TM_PMTU_LOWER );
                    }
                    if (nextHopMtu < TM_PMTU_MINIMUM)
/* Our minimum PMTU is set at 296 bytes */
                    {
                        nextHopMtu = TM_PMTU_MINIMUM;
/*
 * Do not try to increase the Path MTU at the route timeout.
 * Packets are fragmented even with our smallest possible MTU.
 * When the route timeout, we will switch to the non host route, and
 * retry the path MTU discovery starting at the device IP MTU.
 */
                        flags =   TM_RTE_INDIRECT | TM_RTE_PMTU_DISC
                                | TM_RTE_NO_PMTU_DISC;
                    }
                    if (nextHopMtu >= rteMtu)
/*
 * RFC 1191
 * Do not increase MTU beyond our previously cached entry,
 * as a result of a don't fragment error
 */
                    {
/* No MTU change */
                        nextHopMtu = rteMtu;
                        if ( flags == (TM_RTE_INDIRECT | TM_RTE_PMTU_DISC) )
/*
 * If we are not changing the flags either, then there is no need to
 * add/replace the routing entry.
 */
                        {
                            errorCode = TM_EPERM;
                        }
                    }
                }
            }
            if (errorCode == TM_ENOERROR)
/*
 * No error if command was from the user, or there was no host entry with
 * PATH MTU disabled.
 */
            {
                errorCode = tfRtAddPathMtuEntry(
#ifdef TM_USE_STRONG_ESL
                                                devPtr,
#endif /* TM_USE_STRONG_ESL */
                                                rtePtr,
#ifdef TM_USE_IPV6
                                                &ip6Addr,
#else /* !TM_USE_IPV6 */
                                                ipAddress,
#endif /* TM_USE_IPV6 */
                                                timeout,
                                                flags, nextHopMtu);
            }
        }
        else
        {
            errorCode = TM_EPERM;
        }
    }
    else
    {
/* No routing entry found for destination IP address */
        errorCode = TM_EHOSTUNREACH;
    }
/* UNLOCK the tree to release access to the routing table */
    tfRtTreeUnlockNResetCaches();
    return errorCode;
}
#endif /* TM_PMTU_DISC */

/*
 * tf4RtRedirect() Function description
 * Called by ICMP when a redirect has been received 
 * 1. Check that 'gateway' is on the same connected (sub-net) through
 *    which the redirect arrived (netMatch on 'devPtr'/'mhomeIndex'),
 *    and that it is not our own IP address.
 *    (otherwise we silently ignore the redirect) (RFC 1122)
 * 2. Lock the tree, and get current routing entry for 'destIpAddr'.
 * 3. Check that 'srcIpAddr' is valid source for the Redirect (it should be
 *    the current first-hop gateway for 'destIpAddr', information that we
 *    get from the current routing entry)) ( otherwise we silently ignore
 *    the redirect) (RFC 1122)
 * 4. Delete current routing entry only if it is a host route (only
 *    delete host route, i.e DO NOT DELETE NETWORK ROUTE!)
 *    (RFC 1122).
 * 5. Add new entry with new 'gateway' using the host netmask
 *    (all ones). (Also set parameter so that tf4RtAddRoute does not 
 *    lock/unlock the tree.)
 * 6. Unlock the tree.
 *
 * Parameters:
 * devPtr      Interface through which the redirect came
 * mhomeIndex  Interface multi home index through which the redirect came
 * destIpAddr  Original destination IP address that caused the redirect
 * gateway     New gateway to use for the redirect
 * srcIpAddr   source of the redirect.
 *
 * No return value
 */
void tf4RtRedirect ( ttDeviceEntryPtr devPtr,
                     tt16Bit          mhomeIndex,
                     tt4IpAddress     destIpAddress,
                     tt4IpAddress     gateway,
                     tt4IpAddress     srcIpAddr )
{
    tt4IpAddress        intfIpAddr;
    tt4IpAddress        intfNetMask;
    ttRteEntryPtr       rtePtr;
#ifndef TM_SINGLE_INTERFACE_HOME
    ttDeviceEntryPtr    anyDevPtr;
    tt16Bit             anyMhomeIndex;
#endif /* TM_SINGLE_INTERFACE_HOME */
/*
 * 1. Check that 'gateway' is on the same connected (sub-net) through
 *    which the redirect arrived (netMatch on 'devPtr'/'mhomeIndex'),
 *    and that it is not our own IP address.
 *    (otherwise we silently ignore the redirect) (RFC 1122)
 */
    tm_ip_copy(tm_ip_dev_addr(devPtr, mhomeIndex), intfIpAddr);
    tm_ip_copy(tm_ip_dev_mask(devPtr, mhomeIndex), intfNetMask);
    if (    tm_ip_net_match(gateway, intfIpAddr, intfNetMask)
         && !tm_ip_match(gateway, intfIpAddr) )
    {
/* 2. LOCK the tree, and get current routing entry for 'destIpAddress'. */
        tm_call_lock_wait(&tm_context(tvRtTreeLock));
#ifdef TM_USE_IPV6
        rtePtr = tf4RtFind(
#ifdef TM_USE_STRONG_ESL
                          devPtr,
#endif /* TM_USE_STRONG_ESL */
                          destIpAddress);
#else /* !TM_USE_IPV6 */
        rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                        devPtr,
#endif /* TM_USE_STRONG_ESL */
                        destIpAddress);
#endif /* !TM_USE_IPV6 */
        if (rtePtr != TM_RTE_NULL_PTR)
        {
/*
 * 3. Check that 'srcIpAddr' is valid source for the Redirect (it should be
 *    the current first-hop gateway for 'destIpAddress', information that we
 *    get from the current routing entry)) ( otherwise we silently ignore
 *    the redirect) (RFC 1122)
 */
            if (    tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_INDIRECT)
#ifdef TM_USE_IPV6
                 && tm_6_addr_is_ipv4_mapped(&(rtePtr->rteGSGateway))
                 && tm_ip_match( tm_6_ip_mapped_6to4(&(rtePtr->rteGSGateway)),
                                 srcIpAddr )
#else /* !TM_USE_IPV6 */
                 && tm_ip_match(rtePtr->rteGSGateway, srcIpAddr)
#endif /* !TM_USE_IPV6 */
#ifndef TM_SINGLE_INTERFACE_HOME
                 && (rtePtr->rteMhomeIndex == mhomeIndex)
#endif /* TM_SINGLE_INTERFACE_HOME */
                 && (rtePtr->rteDevPtr == devPtr) )
            {
#ifndef TM_SINGLE_INTERFACE_HOME
/* Check that gateway is not one of our own IP address on another interface */
#ifdef TM_4_USE_SCOPE_ID
                if (IN4_IS_ADDR_LINKLOCAL(gateway))
                {
                    anyDevPtr = (ttDeviceEntryPtr)0;
                }
                else
#endif /* TM_4_USE_SCOPE_ID */
                {
                    anyDevPtr = tfIfaceMatch( gateway,
                                              tfMhomeAddrMatch,
                                              &anyMhomeIndex );
                }
                if ( anyDevPtr == (ttDeviceEntryPtr)0 )
#endif /* TM_SINGLE_INTERFACE_HOME */
                {
                    if ( tm_16bit_all_bits_set( rtePtr->rteFlags2,
                                                TM_RTE2_HOST | TM_RTE2_UP ) )
/*
 * 4. Delete current routing entry only if it is a host route (only
 *    delete host route, i.e DO NOT DELETE NETWORK ROUTE!)
 *    (RFC 1122). Note that we delete, and do not modify the existing routing
 *    entry, since individual routing entries are never locked.
 */
                    {
                        tfRtRemoveEntry(rtePtr);
                    }
/*
 * 5. Add new entry with new 'gateway' using the host netmask
 *    (all ones). Set last paramter to zero, so that tf4RtAddRoute() 
 *    does not lock/unlock the tree.
 */
                    (void)tf4RtAddRoute(devPtr, destIpAddress,
                                        TM_IP_HOST_MASK, gateway,
                                        TM_16BIT_ZERO, mhomeIndex, 1,
                                        tm_context(tvRtTimeout),
                                        TM_RTE_INDIRECT|TM_RTE_REDIRECT,
/* no need to lock */
                                        TM_8BIT_ZERO);
                }
            } /* source IP address is valid for redirect */
        } /* Destination in routing table */
/* UNLOCK the tree to release access to the routing table */
        tfRtTreeUnlockNResetCaches();
    } /* netMatch of gateway */
}

/*
 * tfRtRip description
 * Called by tfRipRecvCB when an incoming RIP response arrives.
 * 1. LOCK the tree to obtain exclusive access to the routing table
 * 2. Check wheter we have already a route for this destination/mask
 * 3. Do not overwrite a route created by local user.
 * 4. If the existing route has same gateway, update the TTL, metrics,
 *    and if metrics is Infinity, remove entry from the routing tree.
 * 5. If the existing route does not have the same gateway, replace the
 *    route only if metrics is lower. If metrics is the same, replace only
 *    if timeout of the current route is at least halfway to the expiration
 *    point.
 * To remove, add, or replace an entry, the local variables needRemoveEntry,
 * and needAddEntry are used; replace is achieved by setting both
 * needRemoveEntry, and needAddEntry.
 * If needRemoveEntry is set, the current routing entry is removed.
 * If needAddEntry is set, the new route is added. 
 *
 * Parameters      description
 * devPtr:         device for the route
 * mHomeIndex:     multihome index for the route
 * destIpAddr:     destination Ip address
 * destNetMask:    destination network address
 * nextHop:        gateway IP address for the route
 * metrics:        number of hops to destination
 * routeTag:       Rip route tag
 *
 * Return value:
 * no return
 */
void tfRtRip ( ttDeviceEntryPtr devPtr, tt16Bit mhomeIndex,
               tt4IpAddress destIpAddress, tt4IpAddress destNetMask,
               tt4IpAddress nextHop, int metrics, tt16Bit routeTag )
{
    ttRteEntryPtr       rtePtr;
#ifdef TM_USE_IPV6
    ttIpAddress         next6Hop;
#endif /* TM_USE_IPV6 */
    tt16Bit             creator;
    tt8Bit              needAddEntry;
    tt8Bit              needRemoveEntry;

/* LOCK the tree to obtain exclusive access to the routing table */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));
/* Do we have an entry for this destination/mask */
#ifdef TM_USE_IPV6
    rtePtr = tf4RtFindIpAddrIpNet(
#ifdef TM_USE_STRONG_ESL
                      devPtr,
#endif /* TM_USE_STRONG_ESL */
                      destIpAddress, destNetMask);
#else /* ! TM_USE_IPV6 */
    rtePtr = tfRtFindIpAddrIpNet(
#ifdef TM_USE_STRONG_ESL
                       devPtr,
#endif /* TM_USE_STRONG_ESL */
                       destIpAddress, destNetMask);
#endif /* ! TM_USE_IPV6 */
    needAddEntry = TM_8BIT_ZERO;
    needRemoveEntry = TM_8BIT_ZERO;
    if (rtePtr != TM_RTE_NULL_PTR)
/* If route already exists */
    {
/* Get route creator */
        creator = (tt16Bit)(rtePtr->rteFlags & TM_RTE_CREATE_MASK);
        if (creator >= TM_RTE_RIP)
/* If route creator is not local or static */
        {
/*
 * RFC 1058:
 * If there is an existing route, first compare gateways.
 */
#ifdef TM_USE_IPV6
            tm_6_addr_to_ipv4_mapped(nextHop, &(next6Hop));
            if ( tm_6_ip_match(&(next6Hop), &(rtePtr->rteGSGateway)) )
#else /* !TM_USE_IPV6 */
            if ( tm_ip_match(nextHop, rtePtr->rteGSGateway) )
#endif /* !TM_USE_IPV6 */
            {
/*
 * RFC 1058:
 * If this datagram is from the same gateway as the existing route,
 * reinitialize the timeout.
 */
                rtePtr->rteTtl = tm_context(tvRtTimeout);
/*
 * RFC 1058:
 * same gateway and different metrics: update timeout. update metrics
 */
                rtePtr->rteHops = (tt32Bit)metrics;
/*
 * RFC 1058:
 * If the new metric is 16 (infinity), this starts the process for
 * deleting the route.  The route is no longer used for routing packets.
 * REVNOTE: we just remove the routing entry, we will not start a deletion
 * process since we are just a listener.
 */
                if ( metrics == TM_RIP_INF_METRIC )
                {
                    needRemoveEntry = TM_8BIT_YES;
                }
            }
            else
            {
/*
 * RFC1058:
 * Route is from a different gateway. Only replace the route if metrics
 * is lower
 */
                if ( (tt32Bit)metrics < rtePtr->rteHops )
                {
                    needRemoveEntry = TM_8BIT_YES;
                    needAddEntry = TM_8BIT_YES;
                }
/* RFC 1058:
 * If the new metric is the same as the old one, it is simplest to do
 * nothing further (beyond reinitializing the timeout, as specified
 * above).  However, the 4BSD routed uses an additional heuristic here.
 * Normally, it is senseless to change to a route with the same metric
 * as the existing route but a different gateway.  If the existing route
 * is showing signs of timing out, though, it may be better to switch to
 * an equally-good alternative route immediately, rather than waiting
 * for the timeout to happen.  (See section 3.3 for a discussion of
 * timeouts.)  Therefore, if the new metric is the same as the old one,
 * routed looks at the timeout for the existing route.  If it is at
 * least halfway to the expiration point, routed switches to the new
 * route.  That is, the gateway is changed to the source of the current
 * message.  This heuristic is optional.
 */
                else
                {
                    if ( (tt32Bit)metrics == rtePtr->rteHops )
/* If same metrics */
                    {
                        if ( rtePtr->rteTtl < (tm_context(tvRtTimeout)/2) )
/* If the timeout is at least halfway to the expiration point */
                        {
                            needRemoveEntry = TM_8BIT_YES;
                            needAddEntry = TM_8BIT_YES;
                        }
                    }
                }
            }
        } /* if (creator >= TM_RTE_RIP) */
    }
    else
    {
        if ( metrics != TM_RIP_INF_METRIC )
        {
            needAddEntry = TM_8BIT_YES;
        }
    }
    if ( needRemoveEntry != TM_8BIT_ZERO )
    {
        tfRtRemoveEntry(rtePtr);
    }
    if ( needAddEntry != TM_8BIT_ZERO )
    {
        (void)tf4RtAddRoute(devPtr, destIpAddress, destNetMask,
                            nextHop, routeTag, mhomeIndex, metrics,
                            tm_context(tvRtTimeout),
                            TM_RTE_INDIRECT|TM_RTE_RIP, TM_8BIT_ZERO);
    }
    tfRtTreeUnlockNResetCaches();
}

/*
 * tf4RtModifyDefGatewayMhome function description
 * Update the multihome index for the default gateway routing entry.
 *
 * Parameters
 * Parameter    Description
 * mhomeIndex   new multihome index for the default gateway
 *
 * Return values 
 * TM_ENOERROR  Multihome index successfully updated
 * TM_ENOENT    Could not modify multihome index (entry not found)
 */
int tf4RtModifyDefGatewayMhome(
#ifdef TM_USE_STRONG_ESL
                                ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                                tt16Bit mhomeIndex )
{
    ttRteEntryPtr      rtePtr;
    int                errorCode;

    errorCode = TM_ENOENT;

/* 
 * Lock the routing table since tfRtFindIpAddrIpNet requires the tree to be
 * locked, and so we can modify the default gateway entry.
 */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));

/* Find routing table entry for the default gateway */
#ifdef TM_USE_IPV6
/* The logic to get the IPv4 default gateway needs to change in dual IP
   layer mode: since there always is an entry for the IPv4 default gateway
   (i.e. ttRtRadixHeadEntry.rth4DefaultGatewayPtr, or 
   devPtr->dev4DefaultGatewayPtr), tfRtGetDefault must check to
   see if the flag TM_RTE_REJECT is set, in which case it has not been
   configured and tfRtGetDefault returns TM_ENOENT. */
#ifdef TM_USE_STRONG_ESL
    rtePtr = devPtr->dev4DefaultGatewayPtr;
#else /* !TM_USE_STRONG_ESL */
    rtePtr = tm_context(tvRtRadixTreeHead).rth4DefaultGatewayPtr;
#endif /* !TM_USE_STRONG_ESL */
    if ( tm_16bit_one_bit_set( rtePtr->rteFlags, TM_RTE_REJECT) )
    {
        rtePtr = TM_RTE_NULL_PTR;
    }
#else /* ! TM_USE_IPV6 */
    rtePtr = tfRtFindIpAddrIpNet(
#ifdef TM_USE_STRONG_ESL
                                 devPtr,
#endif /* TM_USE_STRONG_ESL */
                                 TM_IP_ZERO, TM_IP_ZERO);
#endif /* ! TM_USE_IPV6 */

/* If default GW exists in the routing table, update it's multihome index */
    if (rtePtr != TM_RTE_NULL_PTR)
    {
        rtePtr->rteMhomeIndex = mhomeIndex;
        rtePtr->rteOrigMhomeIndex = mhomeIndex;
        errorCode = TM_ENOERROR;
    }
    tfRtTreeUnlock();
    return errorCode;
}

/*
 * tfRtAddHost function description:
 * Called by tfAddHostRoute()
 * to add a host entry to the routing table (Patricia tree) to set
 * a particular destination interface for a given multicast address.
 * 1) Call tfRtAllocNLockNCopy to allocate a new routing entry, lock
 *    the Patricia tree, and for IPV6 only copy the IPv4-mapped IPV6 
 *    address in the routing entry destination field.
 * 2) Copy physical address in routing entry
 * 3) call tfRtAddEntry() to insert the entry in the Patricia tree.
 * 4) Call tfRtCacheUnlockNClean to cache the entry in the SNMP routing
 *    cache, unlock the tree, and unget the routing entry if we failed
 *    to insert in the tree.
 * Parameters
 * devPtr: device for the route 
 * ipAddr: Destination Ip address 
 * physaddrPtr: pointer to physical address
 * physAddrLength: length of the physical address
 * mHomeIndex: multihome index 
 * ttl: Time to live for the route entry
 * flags: flag to indicate creator local/static/redirect/RIP, host route etc.
 *
 * Return value:
 * O on success
 * TM_ENOBUFS, when routing entry could not be allocated
 */
int tf4RtAddHost(
/* route device */
                  ttDeviceEntryPtr       devPtr,
/* Destination Ip address */
                  tt4IpAddress           ipAddress,
/* physical address */
                  tt8BitPtr              physAddrPtr,
/* physical address len */
                  tt8Bit                 physAddrLen,
/* multihome index */
                  tt16Bit                mhomeIndex,
                  tt32Bit                ttl,
/* flag to indicate cloned/static/link-layer/mcast */
                  tt16Bit                flags )
{
    ttRteEntryPtr       rtePtr;
    int                 errorCode;

#ifdef TM_USE_IPV6
/* Allocate routing entry, and initialize destination IP address */
    rtePtr = tf6RtAllocNLockNCopy(TM_8BIT_YES, ipAddress, (tt6IpAddressPtr)0);
#else /* !TM_USE_IPV6 */
    rtePtr = tfRtAllocNLockNCopy(TM_8BIT_YES);
#endif /* !TM_USE_IPV6 */
    if (rtePtr != TM_RTE_NULL_PTR)
    {
        tm_bcopy(physAddrPtr, rtePtr->rteHSPhysAddr ,(unsigned)physAddrLen);
        rtePtr->rteHSPhysAddrLen = physAddrLen;
        rtePtr->rteMtu = devPtr->devMtu;
        errorCode = tfRtAddEntry(
                            tm_context(tvRtTreeHeadPtr), rtePtr, devPtr,
#ifdef TM_USE_IPV6
                            TM_6_IP_HOST_PREFIX_LENGTH,
#else /* !TM_USE_IPV6 */
                            ipAddress, TM_IP_HOST_MASK,
#endif /* !TM_USE_IPV6 */
                            mhomeIndex,
                            0, ttl, flags );
/*
 * Unlock the tree, cache entry if successful or free routing entry otherwise.
 */
        errorCode = 
                tfRtCacheUnlockNClean(rtePtr, TM_8BIT_YES, errorCode, flags);
    }
    else
    {
        errorCode =  TM_ENOBUFS;
    }
    return errorCode;
}


#ifdef TM_USE_IPV6
/* 
 * IPV4 functions used only in dual mode.
 * Function used in dual mode, by IPV4 APIs.
 * Lock the tree.
 * Remove existing route from the tree (rtePtr), and re-insert at location
 * indicated by parameters.
 * Unlock the tree.
 */
int tf4RtAddDefGw (ttDeviceEntryPtr devPtr,
                   tt4IpAddress     gwayAddr
#ifdef TM_USE_STRONG_ESL
                 , int              hops
#endif /* TM_USE_STRONG_ESL */
                  )
{
    ttRteEntryPtr       rtePtr; /* routing entry */
    int                 errorCode;
#ifndef TM_USE_STRONG_ESL
    int                 hops;
#endif /* !TM_USE_STRONG_ESL */

    tm_call_lock_wait(&tm_context(tvRtTreeLock));
#ifdef TM_USE_STRONG_ESL
    if (devPtr != (ttDeviceEntryPtr)0)
    {
        rtePtr = devPtr->dev4DefaultGatewayPtr;
    }
    else
    {
        rtePtr = (ttRteEntryPtr)0;
    }
#else /* !TM_USE_STRONG_ESL */
    hops = 1;
    rtePtr = tm_context(tvRtRadixTreeHead).rth4DefaultGatewayPtr;
#endif /* !TM_USE_STRONG_ESL */
    if (rtePtr != (ttRteEntryPtr)0)
    {
        if ( tm_16bit_bits_not_set(rtePtr->rteFlags, TM_RTE_INDIRECT) )
        {
            tfRtRemoveEntry(rtePtr);
/*
 * Passing a null IPv6 address pointer will automatically make the rteDest
 * field IPv4 mapped upon return from this routine.
 */
            rtePtr = tf6RtAllocNLockNCopy( TM_8BIT_ZERO, TM_IP_ZERO, 
                                           (tt6IpAddressPtr)0 );
            if (rtePtr != (ttRteEntryPtr)0)
            {
                tm_6_addr_to_ipv4_mapped(gwayAddr, &(rtePtr->rteGSGateway));
/* Default IPV4 gateway */
                if (devPtr != TM_DEV_NULL_PTR)
                {
/* Adding a default gateway */
                    rtePtr->rteMtu = devPtr->devMtu;
#ifdef TM_4_USE_SCOPE_ID
                    tm_4_dev_scope_addr(&rtePtr->rteGSGateway, devPtr);
#endif /* TM_4_USE_SCOPE_ID */
                }
                errorCode = tfRtAddEntry( tm_context(tvRtTreeHeadPtr),
                                          rtePtr, devPtr, 
                                          TM_6_DEF_4GWAY_PREFIX_LENGTH,
                                          TM_16BIT_ZERO,
                                          hops,
                                          TM_RTE_INF, 
                                          TM_RTE_STATIC | TM_RTE_INDIRECT);
                if (errorCode == TM_ENOERROR)
                {
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
                tm_kernel_set_critical;
                rtePtr->rteLastUpdated = (tt32Bit)tvTime;
                tm_kernel_release_critical;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
#ifdef TM_SNMP_CACHE
                    tfSnmpdCacheInsertRoute(rtePtr, TM_SNMPC_ROUTE_INDEX);
#endif /* TM_SNMP_CACHE */
#ifdef TM_USE_STRONG_ESL
                    devPtr->dev4DefaultGatewayPtr = rtePtr;
#else /* !TM_USE_STRONG_ESL */
                    tm_context(tvRtRadixTreeHead).rth4DefaultGatewayPtr = rtePtr;
#endif /* !TM_USE_STRONG_ESL */
                    rtePtr->rteFlags2 = (tt16Bit)
                                         (rtePtr->rteFlags2 | TM_RTE2_IP4_DEF_GW);
                }
                else
                {
                    tfRtUnGet(rtePtr);
                }
            }
            else
            {
                errorCode = TM_ENOBUFS;
            }
            if (errorCode != TM_ENOERROR)
            {
/* Insert empty default gateway in the tree */
               (void)tf4RtInsertEmptyDefaultGateway(
#ifdef TM_USE_STRONG_ESL
                                                 devPtr
#endif /* !TM_USE_STRONG_ESL */
                                                    );
            }
        }
        else
        {
            errorCode = TM_EALREADY;
        }
    }
    else
    {
        errorCode = TM_EINVAL;
    }
    tfRtTreeUnlockNResetCaches();
    return errorCode;
}

int tf4RtDelDefGw (
#ifdef TM_USE_STRONG_ESL
                   ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                   tt4IpAddress     gwayAddr)
{
    int                 errorCode;
    ttRteEntryPtr       rtePtr; /* routing entry */

    tm_call_lock_wait(&tm_context(tvRtTreeLock));
#ifdef TM_USE_STRONG_ESL
    rtePtr = devPtr->dev4DefaultGatewayPtr;
#else /* !TM_USE_STRONG_ESL */
    rtePtr = tm_context(tvRtRadixTreeHead).rth4DefaultGatewayPtr;
#endif /* !TM_USE_STRONG_ESL */
    tm_assert(tf4RtDelDefGw, rtePtr != (ttRteEntryPtr)0);
    if (     (tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_INDIRECT))
/* user specified gateway, same current route gateway */
          && (    tm_ip_match(gwayAddr, tm_4_ip_addr(rtePtr->rteGSGateway) )
               || tm_ip_zero(gwayAddr) ) )
    {
#ifdef TM_USE_IPDUAL
/* Insert Ipv4 dummy default gateway after the current one is removed */
        rtePtr->rteFlags2 = (tt16Bit)
                                (rtePtr->rteFlags2 | TM_RTE2_IP4_INS_DEF_GW);
#endif /* TM_USE_IPDUAL */
        tfRtRemoveEntry(rtePtr);
        errorCode = TM_ENOERROR;
    }
    else
    {
        errorCode = TM_ENOENT;
    }
    tfRtTreeUnlockNResetCaches();
    return errorCode;
}

/*
 * Called by IPV4 functions in dual mode, to map the IPv4 address, and
 * netmask to IPv6, and call tfRtFindIpAddrIpNet().
 */
static ttRteEntryPtr tf4RtFindIpAddrIpNet (
#ifdef TM_USE_STRONG_ESL
                                           ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                                           tt4IpAddress destIpAddress,
                                           tt4IpAddress destNetMask
                                         )
{
    tt6IpAddress  dest6IpAddr;
    ttRteEntryPtr rtePtr;
    int           prefixLength;

    tm_6_addr_to_ipv4_mapped(destIpAddress, &dest6IpAddr);
/* Prefix length of the IPV4 address */
    prefixLength = tf4NetmaskToPrefixLen(destNetMask);
/* Prefix length of the IPv4-mapped IPv6 address */
    prefixLength = tm_6_prefixlen_to_ipv4_mapped(prefixLength);
    rtePtr = tfRtFindIpAddrIpNet(
#ifdef TM_USE_STRONG_ESL
                                 devPtr,
#endif /* TM_USE_STRONG_ESL */
                                 &dest6IpAddr, prefixLength
                                );
    return rtePtr;
}

/*
 * Called by IPV4 functions in dual mode, to map the IPv4 address, and
 * netmask to IPv6, and call tfRtFindIpAddrIpNet().
 */
static ttRteEntryPtr tf4RtFind(
#ifdef TM_USE_STRONG_ESL
                               ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                               tt4IpAddress destIpAddress
                              )
{
    tt6IpAddress  dest6IpAddr;
    ttRteEntryPtr rtePtr;

    tm_6_addr_to_ipv4_mapped(destIpAddress, &dest6IpAddr);
    rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
                      devPtr,
#endif /* TM_USE_STRONG_ESL */
                      &dest6IpAddr);
    return rtePtr;
}

/* 
 * In dual IP layer mode, we must always have an IPv4 default gateway entry
 * in the routing tree, even if it isn't configured.
 * First time we insert the empty default gateway. Initialize it, and insert
 * it.
 */
int tf4RtInitInsertEmptyDefaultGateway(
#ifdef TM_USE_STRONG_ESL
                                    ttDeviceEntryPtr   devPtr
#else /* !TM_USE_STRONG_ESL */
                                    void
#endif /* !TM_USE_STRONG_ESL */
                                      )
{
    ttRteEntryPtr       rtePtr;
    int                 errorCode;

#ifdef TM_USE_STRONG_ESL
    rtePtr = devPtr->dev4RejectDefaultGatewayPtr;
#else /* !TM_USE_STRONG_ESL */
    rtePtr = &(tm_context(tvRtRadixTreeHead).rth4RejectDefaultGateway);
#endif /* !TM_USE_STRONG_ESL */
/* For belonging to the tvRtRadixTreeHead structure */
    rtePtr->rteOwnerCount = 1; 
/* destination */
    tm_6_ip_copy_structs(tvIn6AddrIpv4Any, rtePtr->rteDest);
    errorCode = tf4RtInsertEmptyDefaultGateway(
#ifdef TM_USE_STRONG_ESL
                                               devPtr
#endif /* TM_USE_STRONG_ESL */
                                               );
    return errorCode;
}

static int tf4RtInsertEmptyDefaultGateway(
#ifdef TM_USE_STRONG_ESL
                                    ttDeviceEntryPtr devPtr
#endif /* TM_USE_STRONG_ESL */
                                  )
{
   ttRteEntryPtr rtePtr;
   int           errorCode;

/* in dual IP layer mode, we must always have an IPv4 default gateway entry
   in the routing tree, even if it isn't configured. */
#ifdef TM_USE_STRONG_ESL
    rtePtr = devPtr->dev4RejectDefaultGatewayPtr;
#else /* !TM_USE_STRONG_ESL */
    rtePtr = &(tm_context(tvRtRadixTreeHead).rth4RejectDefaultGateway);
#endif /* !TM_USE_STRONG_ESL */
    tm_incr_owner_count(rtePtr->rteOwnerCount);
/* No gateway yet, i.e zero */
    errorCode = tfRtAddEntry(
        tm_context(tvRtTreeHeadPtr),
        rtePtr,
#ifdef TM_USE_STRONG_ESL
        devPtr,
#else /* !TM_USE_STRONG_ESL */
        (ttDeviceEntryPtr)0, /* no device yet */
#endif /* !TM_USE_STRONG_ESL */ 
/* 
 * DUAL mode:
 * We are adding an entry at key value 0, for the IPV4 default gateway.
 * TM_6_DEF_4GWAY_PREFIX_LENGTH is defined as
 * 80, and since the first 80 bits of an IPV4-mapped IPV6 address
 * are zeroes, this will guarantee that an IPV4 address will match on
 * the zero key with that prefix.
 * In dual mode, we use an 80 prefix length for the default IPV4 default 
 * gateway, to make sure that we get a match on the key value zero, 
 * with a prefix length that is longer than an IPV6 prefix length. 
 * In this way, if we do not find another match for an IPV4 
 * destination address, we will find the IPV4 default gateway first.
 * when trying to find a match on the zero key if we have to go
 * through the list of duplicates for the zero key value. (Recall that
 * the leaves in the list of duplicates are inserted with longest prefix
 * length first.)
 * Also an 80 prefix length will guarantee that there will be no match on 
 * an IPV6 address destination, since an IPV6 address is guaranteed to have 
 * some bits set in the first 80 bits.
 * Also add the entry with a high metrics so that it won't be found ahead
 * of a real default gateway on another interface when TM_USE_STRONG_ESL is
 * defined.
 */
        TM_6_DEF_4GWAY_PREFIX_LENGTH, 
        TM_16BIT_ZERO,
        16, TM_RTE_INF, 
/* 
 * TM_RTE_REJECT because initially, we don't have any IPv4 default 
 * gateway configured. 
 * Do not set TM_RTE_INDIRECT yet because we do not have a corresponding 
 * local route, and would not be able to create the corresponding cloned
 * ARP entry.
 */
        TM_RTE_STATIC | TM_RTE_REJECT );
#ifdef TM_USE_STRONG_ESL
    devPtr->dev4DefaultGatewayPtr = rtePtr;
#else /* !TM_USE_STRONG_ESL */
    tm_context(tvRtRadixTreeHead).rth4DefaultGatewayPtr = rtePtr;
#endif /* !TM_USE_STRONG_ESL */
    rtePtr->rteFlags2 = (tt16Bit)(rtePtr->rteFlags2 | TM_RTE2_IP4_DEF_GW);
    return errorCode;
}
#endif /* dual IP layer */

#endif /* TM_USE_IPV4 */

#ifdef TM_RT_OUTPUT

void tfRtTreePrint (void)
{
    (void)tfRtTreeWalk(tfRtPrintEntryCB, TM_VOID_NULL_PTR, TM_8BIT_YES);
}


static int tfRtPrintEntryCB (ttRteEntryPtr rtePtr, ttVoidPtr voidPtr)
{
    ttRtRadixNodePtr    leafPtr;
#ifdef TM_RT_INT_OUTPUT
    ttRtRadixNodePtr    parentPtr;
    ttRtRadixNodePtr    intNodePtr;
    ttRtRadixNodePtr    otherPtr;
    int                 validIntNode;
#endif /* TM_RT_INT_OUTPUT */

    TM_UNREF_IN_ARG(voidPtr); /* compiler */
    leafPtr = tm_rt_rte_to_leaf(rtePtr);
    printf(
        "RTE FLAGS 0x%x 0x%x gway 0x%lx TTL 0x%lx Owner %d clonedPtr 0x%x\n",
        (unsigned)rtePtr->rteFlags,
        (unsigned)rtePtr->rteFlags2,
        ntohl(rtePtr->rteGSGateway),
        rtePtr->rteTtl,
        (int)rtePtr->rteOwnerCount,
        rtePtr->rteClonedPtr);
    tfRtPrintLeaf("LEAF", leafPtr);
#ifdef TM_RT_INT_OUTPUT
    parentPtr = tm_rt_parent(leafPtr);
    if (tm_16bit_one_bit_set(parentPtr->rtnFlags, TM_RTN_NODE))
    {
        tfRtPrintNode("PARENT NODE", parentPtr);
        otherPtr = tm_rt_left_child(parentPtr);
        if (otherPtr != leafPtr)
        {
            if (tm_16bit_one_bit_set(otherPtr->rtnFlags, TM_RTN_NODE))
            {
                tfRtPrintNode("PARENT LEFT CHILD NODE", otherPtr);
            }
            else
            {
                tfRtPrintLeaf("PARENT LEFT CHILD LEAF", otherPtr);
            }
            printf("LEAF IS RIGHT CHILD OF PARENT\n");
        }
        else
        {
            printf("LEAF IS LEFT CHILD OF PARENT\n");
            otherPtr = tm_rt_right_child(parentPtr);
            if (tm_16bit_one_bit_set(otherPtr->rtnFlags, TM_RTN_NODE))
            {
                tfRtPrintNode("PARENT RIGHT CHILD NODE", otherPtr);
            }
            else
            {
                tfRtPrintLeaf("PARENT RIGHT CHILD LEAF", otherPtr);
            }
        }
    }
    else
    {
        tfRtPrintLeaf("LEAF DUP PREV", parentPtr);
        otherPtr = tm_rt_next_dup(leafPtr);
        if (otherPtr != TM_RADIX_NULL_PTR)
        {
            tfRtPrintLeaf("LEAF DUP NEXT", otherPtr);
        }
    }
    intNodePtr = tm_rt_rte_to_int_node(rtePtr);
    validIntNode = tm_16bit_one_bit_set(intNodePtr->rtnFlags, TM_RTN_IN);
    if (validIntNode)
    {
        tfRtPrintNode("INTNODE", intNodePtr);
        otherPtr = tm_rt_parent(intNodePtr);
        tfRtPrintNode("INTNODE PARENT", otherPtr);
        otherPtr = tm_rt_left_child(intNodePtr);
        if (tm_16bit_one_bit_set(otherPtr->rtnFlags, TM_RTN_NODE))
        {
            tfRtPrintNode("INTNODE LEFT CHILD NODE", otherPtr);
        }
        else
        {
            tfRtPrintLeaf("INTNODE LEFT CHILD LEAF", otherPtr);
        }
        otherPtr = tm_rt_right_child(intNodePtr);
        if (tm_16bit_one_bit_set(otherPtr->rtnFlags, TM_RTN_NODE))
        {
            tfRtPrintNode("INTNODE RIGHT CHILD NODE", otherPtr);
        }
        else
        {
            tfRtPrintLeaf("INTNODE RIGHT CHILD LEAF", otherPtr);
        }
        otherPtr = tm_rt_next_net(intNodePtr);
        while (otherPtr != TM_RADIX_NULL_PTR)
        {
            tfRtPrintLeaf("INTNODE NETWORK LIST LEAF", otherPtr);
            otherPtr = tm_rt_next_net(otherPtr);
        }
    }
#endif /* TM_RT_INT_OUTPUT */
    printf("**************************************************\n");
    return TM_ENOENT; /* ask for another entry */
}

static void tfRtPrintLeaf (ttCharPtr msg, ttRtRadixNodePtr leafPtr)
{
    printf( "%s: 0x%x", msg, leafPtr);
    printf( " flags 0x%x destIPAddr 0x%lx destMask 0x%lx\n",
             (unsigned)leafPtr->rtnFlags, ntohl(leafPtr->rtnLSKey),
             ntohl(leafPtr->rtnLSMask) );
}

#ifdef TM_RT_INT_OUTPUT
static void tfRtPrintNode (ttCharPtr msg, ttRtRadixNodePtr nodePtr)
{
    printf( "%s: 0x%x", msg, nodePtr);
    printf( " flags 0x%x bitOffset %d bitMask 0x%lx\n",
            (unsigned)nodePtr->rtnFlags,
            (int)tm_rt_matching_bits(nodePtr),
            ntohl(nodePtr->rtnNSBitMask) );
}
#endif /* TM_RT_INT_OUTPUT */

#endif /* TM_RT_OUTPUT */

/* Neighbor Unreachability Detection stuff */
/****************************************************************************
* FUNCTION: tfNudSetReachable
*
* PURPOSE: 
*   Calling this function indicates that a connection is making forward
*   progress.
*   ([RFC2461].R7.3.1:10, [RFC2461].R7.3.1:20, [RFC2461].R7.3.1:30,
*   [RFC2461].R7.3.1:40, [RFC2461].R7.3.3:20, [RFC2461].R7.3.3:30)
*
* PARAMETERS:
*   rteEntryPtr
*       Pointer to the routing entry.
*
* RETURNS:
*   Nothing
*
* NOTES:
*   This function needs to be moved to trroute.c
*
****************************************************************************/
void tfNudSetReachable(
    ttRteEntryPtr rteEntryPtr )
{
#ifdef TM_USE_IPV6
    ttGenericUnion  dummyUserParm;
    int             errorCode;
#endif /* TM_USE_IPV6 */

#ifdef TM_USE_IPV6
/* avoid compiler warning */
    dummyUserParm.genIntParm = 0;
#endif /* TM_USE_IPV6 */

/* If the routing entry is not in the INCOMPLETE state (this is always the
   case, we don't store INCOMPLETE entries in the routing tree),
   [RFC2461].R7.3.3:20, then: */

/* Store the current time as the most recent time that this neighbor was
   confirmed reachable:
   ([RFC2461].R7.3.3:30) */
    /* done in tf6RtArpUpdateByRte */
#ifdef TM_USE_IPV6
    if (rteEntryPtr->rte6HSNudState != TM_6_NUD_IPV4_STATE)
    {
/* If it is not an IPv4 entry, then mark it as REACHABLE to indicate that
   forward-progress was detected: */

/* Stop the DELAY or PROBE timer if there is any, and set the state to
   REACHABLE */
        errorCode = tf6RtArpUpdateByRte(
            rteEntryPtr,
            TM_8BIT_NULL_PTR,
            TM_8BIT_ZERO,
            TM_6_NUD_REACHABLE_STATE,
            TM_8BIT_ZERO,
            TM_6_ARP_UPDATE_NUD_STATE);

#ifdef TM_6_USE_NUD
/* We saw one failure scenario where the route entry was marked down causing
   tf6RtArpUpdateByRte to fail. Refer to bug ID 740. */
        if (errorCode == TM_ENOERROR)
        {
/* Reschedule the REACHABLE timer */
            tf6NudReachableTimeOut(
                TM_TMR_NULL_PTR, dummyUserParm, dummyUserParm);
        }
#endif /* TM_6_USE_NUD */
    }
#ifdef TM_USE_IPV4
    else
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
    {
        tm_kernel_set_critical;
        rteEntryPtr->rteTtl = tm_context(tvArpTimeout);
#ifdef TM_USE_IPV6
        rteEntryPtr->rteHSLastReachedTickCount = tvTime;
#endif /* TM_USE_IPV6 */
        tm_kernel_release_critical;      
    }
#endif /* TM_USE_IPV4 */

    return;
}

#ifdef TM_USE_IPV6
/*
 * tf6RtArpUpdateByRte
 * Function Description
 *   This function locks the router tree and update the entry members with 
 *   the values provided as parameters to this function.
 * 
 *Internals
 * 1. LOCK the tree to obtain exclusive access to the routing table
 * 2. Verify that the rtePtr is valid and pointing to a ARP entry 
 * 3. update physical address if needs to
 *    3.1. verify input physical address pointer is valid & address length >0
 *    3.2. Copy the physical address with critical section protection 
 * 4.   update the NUD state if needs to
 *    4.1. change state
 *    4.2. update reachable timestamp if new state is REACHABLE
 * 6. update the isRouter flag if needs to 
 * 7. update the timer pointer if needs to
 *    if there is a state change and the new state is 
 *    REACHABLE or STALE, stop the timer, and update the
 *    timerPtr to null.
 * 8. UNLOCK the tree to release access to the routing table
 *
 * Parameters
 *   Parameter    Description
 *   rtePtr       The ARP entry to be updated
 *   physAddrPtr  New Physical address to be stored into the route entry
 *   physAddrLen  New physical address length
 *   nudState     New NUD state
 *   isRouter     0: reset isRouter flag,
 *                non-0 : set isRouter flag
 *   updateFlags  Flag specifies which emmbers are to be updated
 * Returns:
 *   TM_EINVAL:     Invalid input parameter(s)
 *   TM_ENOERROR:   Successful
 */
int tf6RtArpUpdateByRte(
    ttRteEntryPtr   rtePtr,
    tt8BitPtr       physAddrPtr,
    tt8Bit          physAddrLen,
    tt8Bit          nudState,
    tt8Bit          isRouter,
    tt16Bit         updateFlags )
{
#ifdef TM_USE_STRONG_ESL
    tt6RtDevGw          rt6DevGwStruct;
#endif /* TM_USE_STRONG_ESL */
    ttVoidPtr           rt6VoidPtr;
#ifdef TM_6_USE_NUD
    ttTimerPtr          tempTimerPtr;
    ttGenericUnion      userParm;
#endif /* TM_6_USE_NUD */
    int                 errorCode;
    tt8Bit              removeDefaultRouterFlag;
    
    errorCode = TM_ENOERROR;
    removeDefaultRouterFlag = TM_8BIT_ZERO;
/* 1. LOCK the tree to obtain exclusive access to the routing table */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));
/* 1. Verify that the rtePtr is valid and pointing to a ARP entry */
    if (   (rtePtr == TM_RTE_NULL_PTR) 
         || !tm_16bit_all_bits_set( rtePtr->rteFlags,
                                    TM_RTE_ARP | TM_RTE_CLONED )
         || !tm_16bit_all_bits_set( rtePtr->rteFlags2,
                                   TM_RTE2_HOST | TM_RTE2_UP ) )
    {
        errorCode = TM_EINVAL;
        goto arpUpdateFinish;
    }
#ifdef TM_USE_ETHER_8023_LL
    if (physAddrLen != 0)
    {
        rtePtr->rte8023Flags |= (TM_RTE8023_ETH_RESPONSE|TM_RTE8023_ETH_FIRST);
        if (physAddrLen & TM_RT_FRAME_8023)
        {
            physAddrLen &= ~TM_RT_FRAME_8023;
            rtePtr->rte8023Flags |= TM_RTE8023_ETH_8023;
        }
        else
        {
            rtePtr->rte8023Flags &= ~TM_RTE8023_ETH_8023;
        }
    }
#endif /* TM_USE_ETHER_8023_LL */
/* 3. update physical address if needs to */
    if (updateFlags & TM_6_ARP_UPDATE_PHYS_ADDR)
    {
/*    3.1. verify input physAddrPtr is valid & address length >0 */
        if (   physAddrPtr == TM_8BIT_NULL_PTR
            || physAddrLen == 0)
        {
            errorCode = TM_EINVAL;
            goto arpUpdateFinish;
        }
/*    3.2. Copy the physical address with critical section protection */
        tm_kernel_set_critical;
        tm_bcopy(physAddrPtr, rtePtr->rteHSPhysAddr, physAddrLen);
        rtePtr->rteHSPhysAddrLen = physAddrLen;
        rtePtr->rteFlags |= TM_RTE_LINK_LAYER;
        tm_kernel_release_critical;
    }

/* 4. update the NUD state if needs to */
    if (updateFlags & TM_6_ARP_UPDATE_NUD_STATE)
    {
/*    4.1. change state */
        rtePtr->rte6HSNudState = nudState;
        if (nudState == TM_6_NUD_INVALID_STATE)
        {
/* remove the ARP entry from the tree */
            tfRtRemoveHostEntry(rtePtr, 1, (tt16Bit)AF_INET6);
            goto arpUpdateFinish;
        }
        else
        {
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
            tm_kernel_set_critical;
            rtePtr->rteLastUpdated = tm_snmp_time_ticks(tvTime);
            tm_kernel_release_critical;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */

/*    4.2. update reachable timestamp if new state is REACHABLE */
            if (nudState == TM_6_NUD_REACHABLE_STATE)
            {
                tm_kernel_set_critical;
                rtePtr->rteHSLastReachedTickCount = tvTime;
                tm_kernel_release_critical;
            }
        }
    }
 
/* 6. update the isRouter flag if needs to */
    if (updateFlags & TM_6_ARP_UPDATE_IS_ROUTER)
    {
        if(isRouter != TM_8BIT_ZERO)
        {
            rtePtr->rte6Flags |= TM_6_RTE_IS_ROUTER_FLAG;  
        }
        else
        {
/* 6.1. If the IsRouter flag changes from TRUE to FALSE, remove the entry
   from the default router list. [RFC2461].R7.3.3:140 */
            if (rtePtr->rte6Flags & TM_6_RTE_IS_ROUTER_FLAG)
            {
                removeDefaultRouterFlag = TM_8BIT_YES;
            }

            tm_16bit_clr_bit( rtePtr->rte6Flags, TM_6_RTE_IS_ROUTER_FLAG );
        }
    }

#ifdef TM_6_USE_NUD
/* 7. update the timer pointer if needs to */
    if (updateFlags & TM_6_ARP_UPDATE_NUD_STATE)
    {
        switch(nudState)
        {
        case TM_6_NUD_REACHABLE_STATE:
        case TM_6_NUD_STALE_STATE:
/*
 *    7.1. if there is a state change and the new state is 
 *         REACHABLE or STALE, stop the timer
 */
            tempTimerPtr = rtePtr->rte6HSNudTimerPtr;
            if (tempTimerPtr != TM_TMR_NULL_PTR)
            { 
                tm_timer_remove(tempTimerPtr) ;
                rtePtr->rte6HSNudTimerPtr = TM_TMR_NULL_PTR;

/* undo the owner count increase when this timer was first started */
                tfRtUnGet(rtePtr);
            }
            break;

        case TM_6_NUD_DELAY_STATE:
            if (rtePtr->rte6HSNudTimerPtr == TM_TMR_NULL_PTR)
            {
/* 
 * OL: If we give an rtePtr to a timer call back function, we really need
 * to increase the owner count.
 */
                tm_kernel_set_critical;
                rtePtr->rteOwnerCount++;
                tm_kernel_release_critical;
                userParm.genVoidParmPtr = rtePtr;
                rtePtr->rte6HSNudTimerPtr =
                    tfTimerAdd( tf6NudNsTimeOut, 
                                userParm, 
                                userParm, 
                                TM_6_ND_DELAY_1ST_PROBE_TIME,
                                0);
            }
            break;

        default:
            break;
        }
    }
#endif /* TM_6_USE_NUD */

    if (removeDefaultRouterFlag == TM_8BIT_YES)
    {
/*
 * If the IsRouter flag changes from TRUE to FALSE, remove the entry
 * from the default router list. [RFC2461].R7.3.3:140 
 * That will also remove the routing entries that uses the neighbor as 
 * the gateway if it is in the list.
 */
        errorCode = tf6RtDelDefRouter(
#ifdef TM_USE_STRONG_ESL
                                      rtePtr->rteOrigDevPtr,
#endif /* TM_USE_STRONG_ESL */
                                      &(rtePtr->rteDest), TM_8BIT_ZERO);
        if (errorCode != TM_ENOERROR)
/* 
 * If rteDest was not in the default router list then we need to walk the tree
 * to remove all indirect entries pointing to this gateway.
 */
        {
#ifdef TM_USE_STRONG_ESL
            rt6DevGwStruct.dgwDevPtr = rtePtr->rteOrigDevPtr;
            rt6DevGwStruct.dgwIp6AddrPtr = &(rtePtr->rteDest);
            rt6VoidPtr = &rt6DevGwStruct;
#else /* !TM_USE_STRONG_ESL */
            rt6VoidPtr = (ttVoidPtr)&(rtePtr->rteDest);
#endif /* !TM_USE_STRONG_ESL */
            errorCode = tfRtTreeWalk(tfRtRemoveGwEntryCB, 
                                     rt6VoidPtr, 
                                     TM_8BIT_ZERO);
        }
    }

arpUpdateFinish:
/* 8. UNLOCK the tree to release access to the routing table */
    if (tm_16bit_one_bit_set(updateFlags, TM_6_ARP_UPDATE_CACHE_LOCKED))
    {
/* Unlock the Routing tree without resetting the caches */
        tfRtTreeUnlock();
    }
    else
    {
/* Unlock the Routing tree, and reset caches */
        tfRtTreeUnlockNResetCaches();
    }
    return errorCode;
}
#endif /* TM_USE_IPV6 */


#ifdef TM_USE_IPV6
/****************************************************************************
 * FUNCTION: tfRtRemoveGwEntryCB
 *
 * PURPOSE: 
 *  This is a tree walk call back function to remove a route entry if it 
 *  has a certain gateway IP address as the route. This is needed when NUD 
 *  detected that the gateway is no longer reachable. NOTE: we only want 
 *  to remove dynamically created gateway entries (i.e. host gateway entry
 *  dynamically created by receipt and processing of an ICMPv6 Redirect 
 *  message), and not entries that were manually added by the user 
 *  (i.e. static routes).
 *
 * INTERNALS:
 *  1. If this is not a gateway entry 
 *     (i.e. flag bit TM_RTE_INDIRECT is not set), 
 *     then return TM_ENOENT to ask for one more.
 *  2. If this gateway entry is a static route 
 *     (i.e. flag bit TM_RTE_STATIC is set), 
 *     we don't remove it, so return TM_ENOENT to ask for one more.
 *  3. Compare the gateway IP address of the route entry with the IP address
 *     pointed by the ipAddrPtr, use tm_6_ip_match when TM_USE_IPV6 
 *     is define, else use tm_ip_match.
 *  4. If matches, remove the route entry by calling tfRtRemoveEntry().
 *  5. Return TM_ENOENT to ask for one more.
 * PARAMETERS:
 *   rtePtr     
 *      The route entry to be checked for removing if it's gateway IP 
 *      address matches the IP address pointed by the second parameter
 *   ipAddrPtr  
 *      Points to the IP address of the gateway to be removed. 
 *
 * RETURNS:
 *   TM_ENOENT
 *
 * NOTES:
 *
 ****************************************************************************/
static int tfRtRemoveGwEntryCB(
    ttRteEntryPtr   rtePtr,
    ttVoidPtr       voidPtr)
{
    tt6IpAddressPtr     ip6AddrPtr;
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr    devPtr;
#endif /* TM_USE_STRONG_ESL */
/*    
 *  1. If this is not a gateway entry 
 *     (i.e. flag bit TM_RTE_INDIRECT is not set), 
 *     then return TM_ENOENT to ask for one more.
 *  2. If this gateway entry is a static route 
 *     (i.e. flag bit TM_RTE_STATIC is set), 
 *     we don't remove it, so return TM_ENOENT to ask for one more.
 *  3. Compare the gateway IP address of the route entry with the IP address
 *     pointed by the ipAddrPtr, use tm_6_ip_match when TM_USE_IPV6 
 *     is define, else use tm_ip_match.
 */
#ifdef TM_USE_STRONG_ESL
    devPtr = ((tt6RtDevGwPtr)voidPtr)->dgwDevPtr;
    ip6AddrPtr = ((tt6RtDevGwPtr)voidPtr)->dgwIp6AddrPtr;
#else /* !TM_USE_STRONG_ESL */
    ip6AddrPtr = (ttIpAddressPtr)voidPtr;
#endif /* !TM_USE_STRONG_ESL */
    if(    tm_16bit_one_bit_set (rtePtr->rteFlags, TM_RTE_INDIRECT)
        && tm_16bit_bits_not_set(rtePtr->rteFlags, TM_RTE_STATIC)
#ifdef TM_USE_STRONG_ESL
        && (devPtr == rtePtr->rteOrigDevPtr)
#endif /* TM_USE_STRONG_ESL */
        && tm_6_ip_match(ip6AddrPtr, &(rtePtr->rteNextHop))
      )
    {
/*
 *  4. If matches, remove the route entry by calling tfRtRemoveEntry().
 *     Note that we can get away with not checking the entry is a
 *     default router, because the caller of the tfRtTreeWalk with
 *     this function, removes the matching (if any) default router anyways.
 */
        tfRtRemoveEntry(rtePtr);
    }

/*  5. Return TM_ENOENT to ask for then next entry. */
    return TM_ENOENT;
}
#endif /* TM_USE_IPV6 */

/*
 * tfRtRemoveHostEntry function description
 * Function called with the tree locked.
 * Remove an IPv4/v6 host entry from the Patricia tree
 * 1. Check the owner count
 * 1.1 If the owner count is bigger than the ownerCount we expect 
 * 1.1.1 Try and reset all caches pointing to this entry
 * 1.1.2 If owner count is still bigger than the ownerCount we expect
 *       try and remove all gateways pointing to this entry.
 * 1.2 Remove entry from the tree.
 *
 * Parameters
 * rtePtr     Pointer to host routing entry
 * af         AF_INET (IPv4), or AF_INET6 (IPv6)
 * ownerCount Expected owner count of the entry.
 */
static void tfRtRemoveHostEntry (ttRteEntryPtr rtePtr,
                                unsigned int  ownerCount,
                                tt16Bit       af)
{
    if (rtePtr->rteOwnerCount > ownerCount)
/* 1.1 If the owner count is bigger than the ownerCount we expect  */
    {
/* 1.1.1 Try and reset all caches pointing to this entry */
        tm_rt_reset_caches(TM_RTE_CLONED, af, TM_LOC_NOWAIT);
        if (rtePtr->rteOwnerCount > ownerCount)
/* 
 * 1.1.2 If owner count is still bigger than the ownerCount check for
 *       gateways/local routes pointing to the entry.
 */
        {
            tfRtTreeWalk(tfRtRemoveGwToClonedCB, rtePtr, TM_8BIT_ZERO);
        }
    }
    if (tm_16bit_bits_not_set(rtePtr->rteFlags2, TM_RTE2_INVALID))
    {
/* 
 * 1.2 Remove entry from the tree. Note that it is safe to remove this
 * entry from the tree, even if it is being resolved, because we have
 * removed all entries pointing to this entry via the rteClonedPtr.
 */
/* make sure we are still in the tree before we try to remove from the tree */
        if ( tm_16bit_all_bits_set( rtePtr->rteFlags2,
                                    TM_RTE2_HOST | TM_RTE2_UP ) )
        {
            tfRtRemoveEntry(rtePtr);
        }
    }
    else
    {
/* 
 * 1.2 There is a static route/default router pointing to this ARP entry. We cannot
 * remove it, and we will change the ARP mapping to stale so that it is usable.
 */
       tm_16bit_clr_bit(rtePtr->rteFlags2, TM_RTE2_INVALID);
#ifdef TM_USE_IPV6
       if (af == AF_INET6)
       {
            if (rtePtr->rte6HSNudState  == TM_6_NUD_INVALID_STATE)
            {
/*
 * Move corresponding default router to end of the list. Switch default router
 * if it points to this unreachable ARP entry.
 * Note: the following call may free the ARP entry and the default router
 * entry.
 */
                tf6RtDefRouterInvalid(
#ifdef TM_USE_STRONG_ESL
                              rtePtr->rteOrigDevPtr,
#endif /* TM_USE_STRONG_ESL */
                              rtePtr);
            }
            else if (tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_LINK_LAYER))
/* Make the ARP entry usable */
            {
                rtePtr->rte6HSNudState = TM_6_NUD_STALE_STATE;
            }
        }
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPDUAL
        else
#endif /* TM_USE_IPDUAL */
#ifdef TM_USE_IPV4
        {
            tf4RtArpDelByRte(rtePtr);
        }
#endif /* TM_USE_IPV4 */
    }
}

#ifdef TM_USE_IPV6
/*
 * tf6RtUpdateDevMtu function description
 * Update IPv6 routing entries for a given device with a new MTU received in a
 * router advertisement.  Called with the tree unlocked.
 * 
 * Parameters
 * devPtr     Pointer to the device that the RA was received on.
 * newMtu     New IPv6 MTU received in the RA.
 */
void tf6RtUpdateDevMtu(ttDeviceEntryPtr devPtr, tt16Bit newMtu)
{
    tfRtUpdateDevMtu(devPtr, newMtu, AF_INET6);
}
#endif /* TM_USE_IPV6 */

/*
 * tfRtUpdateDevMtu function description
 * Update routing entries for a given device with a new MTU received in a
 * router advertisement or set by the user.  Called with the tree unlocked.
 * 
 * Parameters
 * devPtr     Pointer to the device that the RA was received on.
 * newMtu     New IPv6 MTU received in the RA.
 * af         address family flag (AF_INET, AF_INET6)
 */
void tfRtUpdateDevMtu(ttDeviceEntryPtr devPtr, tt16Bit newMtu, tt16Bit af)
{
    tt6RtDevMtuUpdate  mtuUpdate; 

    mtuUpdate.dmuDevEntryPtr = devPtr;
    mtuUpdate.dmuNewMtu = newMtu;
    mtuUpdate.dmuAf = af;
    
/* Lock the tree before walking through entries. */
    tfRtTreeWalk( tfRtUpdateDevMtuCB,
                  (ttVoidPtr) &mtuUpdate,
                  TM_8BIT_YES );

/* Reset all routing caches, since the MTU may have changed. */ 
    tm_rt_reset_caches(TM_RTE_CLONED | TM_RTE_INDIRECT, af, TM_LOC_WAIT);
}

/*
 * tfRtUpdateDevMtuCB function description
 * Routing tree walk callback function to update routing entries with
 * new MTU received from router advertisement, or set by the user.
 *
 * 1. Update matching IPv6 entries if AF_INET6 is set, IPv4 entries if AF_INET
 *    is set.
 * 2. If IPv6/IPv4 Path MTU discovery is enabled:
 *     2.1. If the user has explicitly set an MTU for this route, do not
 *          modify it.
 *     2.2. If this entry has had the MTU set be Path MTU discovery, only
 *          update the MTU if the new MTU is less than the current Path MTU.
 *          The current Path MTU may be smaller than the link MTU, but
 *          should never be larger.
 * 3. If this is not a Path MTU entry (or IPv6 Path MTU discovery and
 *    IPv4 Path MTU discovery are disabled), update the MTU.
 * 4. Upate the MTU
 * 
 * Parameters
 * rtePtr     Pointer to current routing entry.
 * newMtuPtr  Pointer to new IPv6 MTU received in the RA.
 */

static int tfRtUpdateDevMtuCB(ttRteEntryPtr rtePtr, ttVoidPtr newMtuPtr)
{
    tt6RtDevMtuUpdatePtr  mtuUpdatePtr;
#ifdef TM_USE_IPDUAL
    int                   rteAf;
    int                   matchRteAf;
#endif /* TM_USE_IPDUAL */

    mtuUpdatePtr = (tt6RtDevMtuUpdatePtr) newMtuPtr;
/* 1. Only update matching entries. */
    if (rtePtr->rteDevPtr == mtuUpdatePtr->dmuDevEntryPtr)
    {
#ifdef TM_USE_IPDUAL
        if (mtuUpdatePtr->dmuAf != (tt16Bit)(AF_INET | AF_INET6))
        {
            rteAf = AF_INET6;
            if (    tm_6_addr_is_ipv4_mapped(&(rtePtr->rteDest))
/* 
 * Special check for default IPv4 gateway, because default IPv4 gateway's key
 * does not appear V4 mapped because it is masked with an 80 bits prefix
 * length.
 */
#ifdef TM_USE_STRONG_ESL
                 || (rtePtr == rtePtr->rteOrigDevPtr->dev4DefaultGatewayPtr)
#else /* !TM_USE_STRONG_ESL */
                 || (rtePtr == tm_context(tvRtRadixTreeHead).rth4DefaultGatewayPtr)
#endif /* TM_USE_STRONG_ESL */
               )
             {
                rteAf = AF_INET;
             }
            matchRteAf = rteAf & (int)(mtuUpdatePtr->dmuAf);
        }
        else
        {
            matchRteAf = TM_8BIT_YES;
        }
        if (matchRteAf)
#endif /* TM_USE_IPDUAL */
        {
#if ((defined(TM_USE_IPV4) && defined(TM_PMTU_DISC)) || (defined(TM_6_PMTU_DISC)))
/* 2. If Path MTU discovery is enabled: */
/*
 *     2.1. If the user has explicitly set an MTU for this route, do not
 *          modify it.
 *     2.2. If this entry has had the MTU set be Path MTU discovery, only
 *          update the MTU if the new MTU is less than the current Path MTU.
 *          The current Path MTU may be smaller than the link MTU, but
 *          should never be larger.
 */
            if (    tm_16bit_bits_not_set(rtePtr->rteFlags,
                                          TM_RTE_NO_PMTU_DISC)
                 && (    tm_16bit_bits_not_set (rtePtr->rteFlags,
                                                TM_RTE_PMTU_DISC)
                      || (mtuUpdatePtr->dmuNewMtu < rtePtr->rteMtu)))
#endif /* (TM_USE_IPV4 && TM_PMTU_DISC) || TM_6_PMTU_DISC */
            {
/*            
 *   Just update the MTU directly in the existing routing entry. 
 *   We will invalidate all routing caches upon return.
 */            
                tm_kernel_set_critical;
                rtePtr->rteMtu = mtuUpdatePtr->dmuNewMtu;
                tm_kernel_release_critical;
            }
        }
    }
/*  Return TM_ENOENT to ask for the next entry. */
    return TM_ENOENT;
}

#ifdef TM_USE_IPV6
/****************************************************************************
* FUNCTION: tf6RtDeviceClose
*
* PURPOSE: Remove any IPv6 auto-configured/discovered entries from the
*   routing tree. This includes auto-configured prefixes, auto-discovered
*   default routers, etc. Also removes IPv6 static routes associated with
*   the specified device.
*
* PARAMETERS:
*   devPtr:
*       pointer to the device that we want to remove routing entries for.
*
* RETURNS:
*   Nothing
*
* NOTES:
*
****************************************************************************/
void tf6RtDeviceClose(ttDeviceEntryPtr devPtr)
{
    int errorCode;

/* delete all auto-configured/discovered IPv6 routing entries and IPv6
   static routes associated with the device */
/* LOCK the tree to obtain exclusive access to the routing table */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));
    tf6RtDefRouterDelAllByDevPtr(devPtr);
    do
    {
        errorCode = tfRtTreeWalk(tf6RtRemAutoEntriesCB, devPtr, TM_8BIT_NO);
/* 
 * If tf6RtRemAutoEntriesCB remove a local route, we stop the tree walk because
 * removing a local route generates another tree walk, and it is not
 * save to continue the tree walk then. So we have to start a new tree
 * walk from scratch.
 */
    }
    while (errorCode == TM_ENOERROR);
/* Release the tree LOCK */
    tfRtTreeUnlockNResetCaches();

    return;
}
#endif /* TM_USE_IPV6 */

#ifdef TM_USE_IPV6
#ifdef TM_6_USE_MIP_MN
/* delete all IPv6 default routers, called by Mobile IPv6 mobile node */
void tf6RtDelAllDefRouter(
#ifdef TM_USE_STRONG_ESL
                          ttDeviceEntryPtr devPtr
#endif /* TM_USE_STRONG_ESL */
                          )
{

    tm_trace_func_entry(tf6RtDelAllDefRouter);

/* LOCK the tree to obtain exclusive access to the routing table */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));

/* delete all IPv6 default routers (Null devPtr is wild card) */
    tf6RtDefRouterDelAllByDevPtr(
#ifdef TM_USE_STRONG_ESL
                                  devPtr
#else /* !TM_USE_STRONG_ESL */
                                  (ttDeviceEntryPtr)0
#endif /* !TM_USE_STRONG_ESL */
                                );

/* Release the tree LOCK */
    tfRtTreeUnlockNResetCaches();
    
    tm_trace_func_exit(tf6RtDelAllDefRouter);
}
#endif /* TM_6_USE_MIP_MN */
#endif /* TM_USE_IPV6 */

#ifdef TM_USE_IPV6
/*
 * Remove any IPv6 routing entries that were dynamically discovered. Mainly,
 * this consists of local route entries discovered using prefix discovery and
 * host entries cloned off of those local route entries. However, it may also
 * consist of orphan host entries dynamically created in the absence of a
 * router so that the stack can send packets on an interface even when there
 * is no matching routing entry (refer to requirement [RFC2461].R5.2:30).
 */
static int tf6RtRemAutoEntriesCB(ttRteEntryPtr rtePtr, ttVoidPtr voidPtr)
{
    ttDeviceEntryPtr devPtr;
    int              isLocalRoute;
    int              errorCode;

    devPtr = ((ttDeviceEntryPtr)voidPtr);
    errorCode =  TM_ENOENT;
    if (( rtePtr->rteDevPtr == devPtr)
        && ( rtePtr->rteMhomeIndex >= TM_6_RT_START_PREFIX_MHOME_INDEX ))
    {
        tm_assert( tf6RtRemAutoEntriesCB, 
                   (    (rtePtr->rteLeafNode.rtnPrefixLength != 0) 
                     || (!tm_6_ip_zero(&(rtePtr->rteDest))) ) )
        isLocalRoute = (rtePtr->rteFlags & TM_RTE_LOCAL);
        tfRtRemoveEntry(rtePtr);
        if (isLocalRoute)
        {
/* Stop the tree walk if we have just removed a local route */
            errorCode = TM_ENOERROR;
        }
    }
/* ask for the next entry. We need to check the whole tree */
    return errorCode;
}
#endif /* TM_USE_IPV6 */


#ifdef TM_USE_IPV6
#ifdef TM_6_USE_NUD
#ifdef TM_6_USE_MIP_MN
/* initiate NUD probing of current default router */
void tf6NudProbeDefRouter(void)
{
    ttArpMapping        arpMapping;
    ttRtCacheEntry      rtcCache;
    ttRteEntryPtr       rtePtr;
    ttDeviceEntryPtr    devPtr;
    tt6IpAddress        destIpAddr;
    tt6IpAddress        targIpAddr;
    ttGenericUnion      userParm;
    int                 errorCode;

    tm_trace_func_entry(tf6NudProbeDefRouter);

    rtePtr = TM_RTE_NULL_PTR;
    devPtr = TM_DEV_NULL_PTR;

/* 1. Locate the current default router  */
    if (tm_context(tv6MnVect).mns6Flags & TM_6_MNS_MOBILE_NODE_ACTIVE)
    {
/* since we dont' want one to be cloned if it doesn't exist, we call
 * tfArpCommand to check it first
 */
         arpMapping.arpmCommand = TM_6_ARP_CHK_BY_IP;
         tm_kernel_set_critical;
         tm_6_ip_copy_structs(tm_context(tv6MnVect).mns6LinkLocalDefRtrAddr,
                              arpMapping.arpm6IpAddress);
#ifdef TM_USE_STRONG_ESL
         devPtr = tm_context(tv6MnVect).mns6MobileIfacePtr;
#endif /* TM_USE_STRONG_ESL */
         tm_kernel_release_critical;
#ifdef TM_USE_STRONG_ESL
         arpMapping.arpmDevPtr = devPtr;
#endif /* TM_USE_STRONG_ESL */

         errorCode = tfRtArpCommand(&arpMapping);

         if (errorCode == TM_ENOERROR)
         {
            tm_bzero(&rtcCache, sizeof(ttRtCacheEntry));
#ifdef TM_USE_STRONG_ESL
            rtcCache.rtcDevPtr = devPtr;
#endif /* TM_USE_STRONG_ESL */
/* if it exist, get it with owner count */
             tm_6_ip_copy_structs(arpMapping.arpm6IpAddress,
                                  rtcCache.rtcDestIpAddr);

/* increase the owner count so we can safely access this routing entry */
             errorCode = tfRtGet(&rtcCache);

             if (errorCode == TM_ENOERROR)
             {
                 rtePtr = rtcCache.rtcRtePtr;
                 tm_assert(tf6NudProbeDefRouter, (rtePtr != TM_RTE_NULL_PTR));
             }
         }
    }

/* 2. If get the entry, start probing it if not already in the PROBE state */
    if (rtePtr != TM_RTE_NULL_PTR)
    {
/* NOTE: the Neighbor Cache entry cannot be in the INCOMPLETE state, because
   we used TM_6_ARP_CHK_BY_IP to get it */
        if (rtePtr->rte6HSNudState != TM_6_NUD_PROBE_STATE)
        {
/* LOCK the tree to avoid a race condition occuring with the NUD timer
   handler */
            tm_call_lock_wait(&tm_context(tvRtTreeLock));

/* In DELAY state, there is 5 second timer */
            if (rtePtr->rte6HSNudTimerPtr != TM_TMR_NULL_PTR)
            {
                tm_timer_remove(rtePtr->rte6HSNudTimerPtr);
                rtePtr->rte6HSNudTimerPtr = TM_TMR_NULL_PTR;

/* undo the owner count increase when this timer was first started */
                tfRtUnGet(rtePtr);
            }

            if (tm_16bit_one_bit_set(rtePtr->rteFlags2, TM_RTE2_UP))
/* tf6RtRemoveHostEntry has not been called */
            {
/*  We could not be possibly be called with an indirect routing entry */
                tm_assert(
                    tf6NudNSTimeOut, 
                    tm_16bit_bits_not_set(rtePtr->rteFlags, TM_RTE_INDIRECT));

/* transition the Neighbor Cache entry to the PROBE state, send probe, and
   start probe retransmission timer */
                rtePtr->rte6HSNudState  = TM_6_NUD_PROBE_STATE;
/* reset the transmission count */
                rtePtr->rte6HSNsTries   = tm_context(tv6NdUnicastRequestTries);

                if(rtePtr->rte6HSNsTries == TM_32BIT_ZERO)
                {
/*
 *  3.  If the retry counter is zero we've run out of retransmissions:
 *            [RFC2461].R7.3.3:90
 */
                    rtePtr->rte6HSNudState  = TM_6_NUD_INVALID_STATE;

/*
 *      3.2.  remove the entry. If a default router points to this
 *            entry, it will automatically be removed.
 */ 
                    tfRtRemoveHostEntry(rtePtr, 2, (tt16Bit)AF_INET6);
                }
                else
                {
/*   
 *  4. else the retry counter is not zero, send Neighbor Solicitation
 *            for the target address. [RFC2461].R7.3.3:70
 */
/*
 *      4.1   Send the unicast neighbor solicitation
 *            target address is the IP address  (unscoped)
 *            that the ARP entry is resolving 
 *            source IPv6 address to send the solicitation from
 *            destination address is same as the target address. (scoped)
*/
                    tm_6_ip_copy_structs(rtePtr->rteDest, destIpAddr);

/* the target address has to be unscoped */
                    tm_6_ip_copy_structs(rtePtr->rteDest, targIpAddr);
                    tm_6_dev_unscope_addr(&targIpAddr);

                    devPtr  = rtePtr->rteDevPtr;

/*      4.2.  decrement the retransmission counter */
                    rtePtr->rte6HSNsTries --;

                    tm_assert(tf6NudProbeDefRouter,
                              (rtePtr->rte6HSNudTimerPtr == TM_TMR_NULL_PTR));
/*
 *      4.3.  if we just come to the PROBE state from the DELAY state, 
 *            need to start the retransmit timer            
 */
/* 
 * OL: If we give an rtePtr to a timer call back function, we really need
 * to increase the owner count.
 */
                    tm_kernel_set_critical;
                    rtePtr->rteOwnerCount++;
                    tm_kernel_release_critical;
                    userParm.genVoidParmPtr = rtePtr;
                    rtePtr->rte6HSNudTimerPtr = tfTimerAdd(
                        tf6NudNsTimeOut,
                        userParm,
                        userParm,
                        devPtr->dev6NeighSolicitReTxPeriod,
                        TM_TIM_AUTO);
                }
            }

/*    unlock the tree */
            tfRtTreeUnlockNResetCaches();
        }

/* send the NS message here, if we need to , this must be done without the
   tree lock, since tf6NdSendNeighSolicit would try to lock the tree as well 
*/
        if (devPtr != TM_DEV_NULL_PTR)
        {
            tf6NdSendNeighSolicit(
                devPtr,
                &targIpAddr,
                &devPtr->dev6IpAddrArray[devPtr->dev6LinkLocalMhomeIndex],
                &destIpAddr);
        }

/* we are done accessing this routing entry, so decrement the owner count */
        tfRtCacheUnGet(&rtcCache);
    }

    tm_trace_func_exit(tf6NudProbeDefRouter);
    return;
}
#endif /* TM_6_USE_MIP_MN */
#endif /* TM_6_USE_NUD */
#endif /* TM_USE_IPV6 */

#ifdef TM_USE_IPV6
#ifdef TM_6_USE_NUD
/****************************************************************************
 * FUNCTION: tf6NudNsTimeOut
 *
 * PURPOSE: 
 *   Called by auto-timer when an ARP entry is due in either the DELAY or PROBE
 *   state for transmission/retransmission of unicast NS.  Transmit the
 *   solicitation or remove the entry if we've run out of retransmissions.
 *   [RFC2461].R7.3.3:50, [RFC2461].R7.3.3:60, [RFC2461].R7.3.3:70,
 *   [RFC2461].R7.3.3:80, [RFC2461].R7.3.3:90
 * INTERNALS:

 *  0. lock the route tree 
 *  1. if entry is in DELAY state, [RFC2461].R7.3.3:60 
 *      1.1  remove the one-shot DELAY timer, 
 *      1.2.  change state to PROBE 
 *      1.3.  reset the transmission count 
 *  2.  assert state is PROBE, 
 *  3.  If the retry counter is zero we've run out of retransmissions:
 *            [RFC2461].R7.3.3:90
 *      3.1.  remove the retransmission timer: 
 *      3.2.  remove the entry 
 *      3.3.  remove all non-static indirect routes whose gateway matches this 
 *            neighbor's IP address. 
 *  4. else the retry counter is not zero, send Neighbor Solicitation
 *            for the target address. [RFC2461].R7.3.3:70
 *      4.1   Send the unicast neighbor solicitation
 *            target address is the IP address 
 *            that the ARP entry is resolving 
 *            source IPv6 address to send the solicitation from
 *            destination address is same as the target address.
 *      4.2.  decrement the retransmission counter 
 *      4.3.  if we just come to the PROBE state from the DELAY state, 
 *            need to start the retransmit timer            
 * 5. unlock the tree 
 *
 * PARAMETERS:
 *   voidPtr 
 *      pointer to the timer that just expired.
 *   userParm1
 *      the ip address of the nieghbor whoes entry is time out
 *
 * RETURNS:
 *   Nothing
 *
 * NOTES:
 *
 ****************************************************************************/
void tf6NudNsTimeOut(
    ttVoidPtr       timerBlockPtr,
    ttGenericUnion  userParm1,
    ttGenericUnion  userParm2)
{
    ttRteEntryPtr               rtePtr;    
    ttDeviceEntryPtr            devPtr;
    ttTimerPtr                  nsTimerPtr;
    tt6IpAddress                destIpAddr;
    tt6IpAddress                targIpAddr;

/* parameter processing */
    nsTimerPtr = (ttTimerPtr)timerBlockPtr;
    rtePtr = (ttRteEntryPtr)userParm1.genVoidParmPtr;
    tm_assert(tf6NudNsTimeOut, rtePtr != TM_RTE_NULL_PTR)
/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    devPtr    = TM_DEV_NULL_PTR;

/*  0. lock the route tree, */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));
/* verify that we haven't been removed in another task */
    if (tm_timer_not_reinit(nsTimerPtr))
    {
        if (tm_16bit_one_bit_set(rtePtr->rteFlags2, TM_RTE2_UP))
/* tfRtRemoveHostEntry has not been called */
        {

/*  We could not be possibly be called with an indirect routing entry */
            tm_assert(
                tf6NudNSTimeOut, 
                tm_16bit_bits_not_set(rtePtr->rteFlags, TM_RTE_INDIRECT));
/*  1. if entry is in DELAY state, [RFC2461].R7.3.3:60 */
            if (rtePtr->rte6HSNudState == TM_6_NUD_DELAY_STATE)
            {
/*      1.1  remove the one-shot DELAY timer, */
                tm_timer_remove(nsTimerPtr);
                rtePtr->rte6HSNudTimerPtr = TM_TMR_NULL_PTR;
/*      1.2.  change state to PROBE */
                rtePtr->rte6HSNudState  = TM_6_NUD_PROBE_STATE;
/*      1.3.  reset the transmission count */
                rtePtr->rte6HSNsTries   = tm_context(tv6NdUnicastRequestTries);
            }

/*  2.  assert state is PROBE, */
            tm_assert(tf6NudNsTimeOut, 
                      rtePtr->rte6HSNudState == TM_6_NUD_PROBE_STATE);

            if(rtePtr->rte6HSNsTries == TM_32BIT_ZERO)
            {
/*
 *  3.  If the retry counter is zero we've run out of retransmissions:
 *            [RFC2461].R7.3.3:90
 */
/*      3.1.  remove the retransmission timer: */
                tm_timer_remove(nsTimerPtr);
                rtePtr->rte6HSNudTimerPtr = TM_TMR_NULL_PTR;

/*
 *      3.2.  remove the entry. If a default router points to this
 *            entry, it will automatically be removed.
 */ 
                rtePtr->rte6HSNudState  = TM_6_NUD_INVALID_STATE;
                tfRtRemoveHostEntry(rtePtr, 2, (tt16Bit)AF_INET6);
/* To undo the owner count increase when this timer was first started */
                tfRtUnGet(rtePtr);
            }
            else
            {
/*   
 *  4. else the retry counter is not zero, send Neighbor Solicitation
 *            for the target address. [RFC2461].R7.3.3:70
 */
/*
 *      4.1   Send the unicast neighbor solicitation
 *            target address is the IP address  (unscoped)
 *            that the ARP entry is resolving 
 *            source IPv6 address to send the solicitation from
 *            destination address is same as the target address. (scoped)
*/
                tm_6_ip_copy_structs(rtePtr->rteDest, destIpAddr);

/* the target address has to be unscoped */
                tm_6_ip_copy_structs(rtePtr->rteDest, targIpAddr);
                tm_6_dev_unscope_addr(&targIpAddr);

                devPtr  = rtePtr->rteDevPtr;

/*      4.2.  decrement the retransmission counter */
                rtePtr->rte6HSNsTries --;

                if (rtePtr->rte6HSNudTimerPtr == TM_TMR_NULL_PTR)
                {
/*
 *      4.3.  if we just come to the PROBE state from the DELAY state, 
 *            need to start the retransmit timer            
 */
                    nsTimerPtr = tfTimerAdd(
                        tf6NudNsTimeOut,
                        userParm1,
                        userParm2,
                        devPtr->dev6NeighSolicitReTxPeriod,
                        TM_TIM_AUTO);
                    rtePtr->rte6HSNudTimerPtr = nsTimerPtr;
                }
            }
        }
        else
        {
/* Entry has been removed from the tree. Make sure the timer is removed */
            tm_timer_remove(nsTimerPtr);
            rtePtr->rte6HSNudTimerPtr = TM_TMR_NULL_PTR;
/* To undo the owner count increase when this timer was first started */
            tfRtUnGet(rtePtr);
        }
    }
/*  unlock the tree */
    tfRtTreeUnlockNResetCaches();

/* send the NS message here, if we need to , this must be done without the
   tree lock, since tf6NdSendNeighSolicit would try to lock the tree as well 
*/
    if (devPtr != TM_DEV_NULL_PTR)
    {
        tf6NdSendNeighSolicit(
                devPtr,
                &targIpAddr,
                &devPtr->dev6IpAddrArray[devPtr->dev6LinkLocalMhomeIndex],
                &destIpAddr);
    }

    return;
}
#endif /* TM_6_USE_NUD */
#endif /* TM_USE_IPV6 */

/*
 * Common function to unlock the tree, when entries have been added/deleted.
 * Unlock the tree, and call tfRtResetCaches() if needed with the tree
 * unlocked.
 */
static void tfRtTreeUnlockNResetCaches (void)
{
    int               resetCachesAf;
    tt16Bit           resetCachesFlags;
#ifdef TM_6_USE_MIP_MN
    ttGenericUnion    dummyTmrParm;
    tt8Bit            rtL3MoveDetected;
#endif /* TM_6_USE_MIP_MN */

    resetCachesFlags = tm_context(tvRtRadixTreeHead).rthResetCachesFlags;
/* Initialize resetCachesAf for compiler warning */
    resetCachesAf = (int)tm_context(tvRtRadixTreeHead).rthResetCachesAf;
    if (resetCachesFlags != TM_16BIT_ZERO)
    {
        tm_context(tvRtRadixTreeHead).rthResetCachesFlags = TM_16BIT_ZERO;
        tm_context(tvRtRadixTreeHead).rthResetCachesAf = TM_16BIT_ZERO;
    }
#ifdef TM_6_USE_MIP_MN
    rtL3MoveDetected = tm_context(tvRtRadixTreeHead).rthL3MoveDetected;
    tm_context(tvRtRadixTreeHead).rthL3MoveDetected = TM_8BIT_NO;
#endif /* TM_6_USE_MIP_MN */
    tfRtTreeUnlock();
/* Call tfRtResetCaches() with tree lock unlocked */
    if (resetCachesFlags != TM_16BIT_ZERO)
    {
        tm_rt_reset_caches(resetCachesFlags, resetCachesAf, TM_LOC_WAIT);
    }
#ifdef TM_6_USE_MIP_MN
    if (rtL3MoveDetected != TM_8BIT_NO)
    {
        dummyTmrParm.gen32bitParm = 0;
        tf6MnStartL3MoveTimer(TM_TMR_NULL_PTR, dummyTmrParm, dummyTmrParm);
    }
#endif /* TM_6_USE_MIP_MN */
}

/*
 * Unlock the routing tree.
 */
static void tfRtTreeUnlock (void)
{
    tm_unlock(&tm_context(tvRtTreeLock));
}

/*
 * Remove routing entries queued to the tree remove list (while we were in a
 * tree walk) from the routing tree.
 */
static void tfRtTreeRem (void)
{
    ttRteEntryPtr rtePtr;

    rtePtr = tm_context(tvRtRadixTreeHead).rthRteRemListPtr;
    while (rtePtr !=(ttRteEntryPtr)0)
    {
/* Save Next on the list */
/* Dequeue top of the list */
        tm_context(tvRtRadixTreeHead).rthRteRemListPtr = rtePtr->rteNextRemPtr;
        tm_16bit_clr_bit(rtePtr->rteFlags2, TM_RTE2_IN_REM_LIST);
        rtePtr->rteNextRemPtr = (ttRteEntryPtr)0;
/* Process removal of current routing entry */
        tfRtRemoveEntry(rtePtr);
/*
 * Next. We have to refresh from the top of the list, in case of recursive
 * tfRtTreeWalk()
 */
        rtePtr = tm_context(tvRtRadixTreeHead).rthRteRemListPtr;
    }
}

#ifdef TM_USE_STRONG_ESL
#ifdef TM_USE_IPV6
ttDeviceEntryPtr tf6RtGetNextDefaultGatewayList(unsigned int index)
{
    tt6RtDefRouterHeadPtr def6HeadPtr;
    ttDeviceEntryPtr      devPtr;

    def6HeadPtr = (tt6RtDefRouterHeadPtr)0;
    tm_lock_wait(&tm_context(tvDeviceListLock));
    devPtr = tm_context(tvDeviceList);
    while (devPtr != TM_DEV_NULL_PTR)
    {
        if (   !(tm_dev_is_loopback(devPtr))
             && (devPtr->devIndex > index)
           )
        {
            def6HeadPtr = &(devPtr->dev6DefaultRouterHead);
            if (def6HeadPtr->drh6RouterCount != 0)
            {
                break;
            }
        }
        devPtr = devPtr->devNextDeviceEntry;
    }
    tm_unlock(&tm_context(tvDeviceListLock));
    return devPtr;
}
#endif /* TM_USE_IPV6 */
#endif /* TM_USE_STRONG_ESL */

#ifdef TM_USE_ARP_FLUSH
#ifdef TM_SNMP_CACHE
#ifdef TM_USE_IPV4
static int tf4RtArpNodeDeleteCB (
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam)
{
    TM_UNREF_IN_ARG(genParam);
    tf4RtArpDelByRte((ttRteEntryPtr)
        (((ttSnmpCacheEntryPtr)(ttVoidPtr)nodePtr)->sceTableEntryPtr));
    return 0;
}
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
static int tf6RtArpNodeDeleteCB (
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam)
{
    TM_UNREF_IN_ARG(genParam);
    tf6RtArpDelByRte((ttRteEntryPtr)
        (((ttSnmpCacheEntryPtr)(ttVoidPtr)nodePtr)->sceTableEntryPtr));
    return 0;
}
#endif /* TM_USE_IPV6 */
#endif /* TM_SNMP_CACHE */
/*
 * Flush the ARP table entries corresponding to the passed address family(ies)
 */
void tfRtArpFlush ( int af
#ifdef TM_LOCK_NEEDED
                  , tt8Bit lockFlag
#endif /* TM_LOCK_NEEDED */
                  )
{
#ifdef TM_SNMP_CACHE
    ttSnmpdCacheHeadPtr snmpdCacheHeadPtr;
    ttListCBFuncPtr     listCBFuncPtr;
    ttGenericUnion      genParam;
    int                 cacheIndex;
#endif /* TM_SNMP_CACHE */

/* Reset all caches pointing to ARP entries */
    tm_rt_reset_caches(TM_RTE_CLONED, af, lockFlag);
/* lock the tree */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));
#ifdef TM_LOCK_NEEDED
/* Reset all caches pointing to ARP entries after having acquired the lock */
    tm_rt_reset_caches(TM_RTE_CLONED, af, TM_LOC_NOWAIT);
#endif /* TM_LOCK_NEEDED */
#ifndef TM_SNMP_CACHE
/* walk the whole tree */
    (void)tfRtTreeWalk(tfRtRemArpEntriesCB, &af, TM_8BIT_NO);
#else /* TM_SNMP_CACHE */
/* Walk the ARP list */
/*
 * Prevent removal on entries while we walk the list, otherwise we would have
 * a deadlock when the tfRtRemoveEntry() tries and locks the SNMP cache that
 * is already locked here
 */
    tm_context(tvRtRadixTreeHead).rthTreeWalkCount++;
#ifdef TM_USE_IPDUAL
    while (af != 0)
#endif /* TM_USE_IPDUAL */
    {
/* Assume IPv4 ARP cache */
        cacheIndex = TM_SNMPC_ARP_INDEX;
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPDUAL
        if (af & AF_INET)
#endif /* TM_USE_IPDUAL */
        {
            af &= ~AF_INET;
        }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPDUAL
        else
#endif /* TM_USE_IPDUAL */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPDUAL
        if (af & AF_INET6)
#endif /* TM_USE_IPDUAL */
        {
            af &= ~AF_INET6;
            cacheIndex = TM_SNMPC6_ARP_INDEX;
        }
#endif /* TM_USE_IPV6 */
/* start from the first route entry */
        snmpdCacheHeadPtr = &(tm_context(tvSnmpdCaches)[cacheIndex]);
        if (snmpdCacheHeadPtr->snmpcList.listCount != 0)
        {
            listCBFuncPtr = (ttListCBFuncPtr)0; 
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPDUAL
            if (cacheIndex == TM_SNMPC_ARP_INDEX)
#endif /* TM_USE_IPDUAL */
            {
                listCBFuncPtr = tf4RtArpNodeDeleteCB;
            }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPDUAL
            else
#endif /* TM_USE_IPDUAL */
#ifdef TM_USE_IPV6
            {
                listCBFuncPtr = tf6RtArpNodeDeleteCB;
            }
#endif /* TM_USE_IPV6 */
            if (listCBFuncPtr != (ttListCBFuncPtr)0)
            {
                genParam.genIntParm = 0;
/* List walk protected by the routing tree lock */
                tfListWalk(&(snmpdCacheHeadPtr->snmpcList),
                           listCBFuncPtr, genParam);
            }
        }
    }
/* Done with the walk */
    tm_context(tvRtRadixTreeHead).rthTreeWalkCount--;
#endif /* TM_SNMP_CACHE */
/* delete entries removed during the tree or ARP cache walk */
    tfRtTreeRem();
/* Unlock the tree */
    tfRtTreeUnlock();
    return;
}

#ifndef TM_SNMP_CACHE
/*
 * Tree walk call back function.
 */
static int tfRtRemArpEntriesCB (ttRteEntryPtr rtePtr,  ttVoidPtr parmPtr)
{
    int af;

    if (    tm_16bit_all_bits_set( rtePtr->rteFlags,
                                   (TM_RTE_CLONED | TM_RTE_ARP) )
/* ARP */
         && tm_16bit_all_bits_set( rtePtr->rteFlags2,
                                   TM_RTE2_HOST | TM_RTE2_UP )
       )
/* host entry in the tree */
    {
/* ARP entry */
        af = *((ttIntPtr)parmPtr);
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPDUAL
        if (rtePtr->rte6HSNudState == TM_6_NUD_IPV4_STATE)
#endif /* TM_USE_IPDUAL */
        {
/* IPv4 ARP entry */
            if (af & AF_INET)
            {
                tf4RtArpDelByRte(rtePtr);
            }
        }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPDUAL
        else
#endif /* TM_USE_IPDUAL */
#ifdef TM_USE_IPV6
        {
/* IPv6 ARP entry */
            if (af & AF_INET6)
            {
                tf6RtArpDelByRte(rtePtr);
            }
        }
#endif /* TM_USE_IPV6 */
    }
    return TM_ENOENT;
}
#endif /* !TM_SNMP_CACHE */
#endif /* TM_USE_ARP_FLUSH */

#ifdef TM_USE_STOP_TRECK
struct tsRtFlushEntry
{
    int     rtfAf;
    tt16Bit rtfFlags;
};

typedef struct tsRtFlushEntry TM_FAR * ttRtFlushEntryPtr;


/* Free all router/arp entries + route tables */
TM_PROTO_EXTERN void tfRtDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                )
{
#ifdef TM_USE_IPV6
#ifdef TM_6_USE_PREFIX_DISCOVERY
    if (tm_context_var(tv6RtPrefixAgingTimerPtr) != TM_TMR_NULL_PTR)
    {
        tm_timer_remove(tm_context_var(tv6RtPrefixAgingTimerPtr));
        tm_context_var(tv6RtPrefixAgingTimerPtr) = TM_TMR_NULL_PTR;
    }
#endif /* TM_6_USE_PREFIX_DISCOVERY */
#endif /* TM_USE_IPV6 */
#if (defined(TM_USE_IPV4) || defined(TM_6_PMTU_DISC))
    if (tm_context_var(tvRtArpTimerPtr) != TM_TMR_NULL_PTR)
    {
        tm_timer_remove(tm_context_var(tvRtArpTimerPtr));
        tm_context_var(tvRtArpTimerPtr) = TM_TMR_NULL_PTR;
    }
#endif /* TM_USE_IPV4 || TM_6_PMTU_DISC */
    if (tm_context_var(tvRtTimerPtr) != TM_TMR_NULL_PTR)
    {
        tm_timer_remove(tm_context_var(tvRtTimerPtr));
        tm_context_var(tvRtTimerPtr) = TM_TMR_NULL_PTR;
    }
    tfRtFlush(
                AF_INET | AF_INET6, 0xFFFF
#ifdef TM_LOCK_NEEDED
              , TM_8BIT_NO
#endif /* TM_LOCK_NEEDED */
             );
}
        
/*
 * Flush the route/ARP table entries corresponding to the passed address
 * family(ies)
 */
void tfRtFlush (
                 int           af,
                 tt16Bit       flags
#ifdef TM_LOCK_NEEDED
               , tt8Bit       lockFlag
#endif /* TM_LOCK_NEEDED */
                  )
{
    struct tsRtFlushEntry   rtFlushEntry;

/* Reset all caches pointing to entries as designated by the flags parameter */
    tm_rt_reset_caches((tt16Bit)flags, af, lockFlag);
/* lock the tree */
    tm_call_lock_wait(&tm_context(tvRtTreeLock));
#ifdef TM_LOCK_NEEDED
/* Reset all caches again after having acquired the lock */
    tm_rt_reset_caches((tt16Bit)flags, af, TM_LOC_NOWAIT);
#endif /* TM_LOCK_NEEDED */
/* walk the whole tree */
    rtFlushEntry.rtfAf = af;
    rtFlushEntry.rtfFlags = (tt16Bit)flags;
    (void)tfRtTreeWalk(tfRtRemEntriesCB, &rtFlushEntry, TM_8BIT_NO);
/* delete entries removed during the tree walk */
    tfRtTreeRem();
/* Unlock the tree */
    tfRtTreeUnlock();
    return;
}

/*
 * Tree walk call back function.
 */
static int tfRtRemEntriesCB (ttRteEntryPtr rtePtr,  ttVoidPtr parmPtr)
{
    tt16Bit flags;

    flags = ((ttRtFlushEntryPtr)parmPtr)->rtfFlags;
    if (    tm_16bit_one_bit_set(rtePtr->rteFlags, flags)
         && tm_16bit_one_bit_set(rtePtr->rteFlags2, TM_RTE2_UP) )
    {
        tfRtRemoveEntry(rtePtr);
    }
    return TM_ENOENT;
}
#endif /* TM_USE_STOP_TRECK */

#if defined(TM_USE_IKEV2) && defined(TM_IKEV2_REMOTE_SERVER) 
int tfRtGetGwFromDest(
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr        devPtr,
#endif /* TM_USE_STRONG_ESL */
    const ttIpAddressPtr    destIpAddrPtr,
    const ttIpAddressPtr    netMaskPtr,
    ttIpAddressPtr          gatewayIpAddrPtr)
{
   ttIpAddress     netAddr;
   ttRteEntryPtr   rtePtr;
   int             errorCode;

   errorCode = TM_EHOSTUNREACH;
/* Destination network */
#ifdef TM_USE_IPV6
   tm_6_ip_net(&netAddr, destIpAddrPtr, netMaskPtr);
#else  /* TM_USE_IPV6 */
   tm_ip_copy(tm_ip_net(*destIpAddrPtr, *netMaskPtr), netAddr);
#endif /* TM_USE_IPV6 */

/* LOCK the tree to obtain exclusive access to the routing table */
   tm_call_lock_wait(&tm_context(tvRtTreeLock));
   rtePtr = tfRtFind(
#ifdef TM_USE_STRONG_ESL
       devPtr,
#endif /* TM_USE_STRONG_ESL */
#ifdef TM_USE_IPV6
       &netAddr
#else  /* TM_USE_IPV6 */
       netAddr
#endif /* TM_USE_IPV6 */
       );
   if (rtePtr != TM_RTE_NULL_PTR)
   {
       if (tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_INDIRECT))
       {
           tm_ip_copy(rtePtr->rteGSGateway, *gatewayIpAddrPtr);
       }
       else
       {
           tm_ip_copy(*destIpAddrPtr, *gatewayIpAddrPtr);
       }
       errorCode = TM_ENOERROR;
   }
/* UNLOCK the tree to release access to the routing table */
   tfRtTreeUnlock();
   return errorCode;
}
#endif /* defined(TM_USE_IKEV2) && defined(TM_IKEV2_REMOTE_SERVER) */
