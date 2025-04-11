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
 * Description: IPv6 function prototypes
 *
 * Filename: t6proto.h
 * Author: Ed Remmell
 * Date Created: 3/27/02
 * $Source: include/t6proto.h $
 *
 * Modification History
 * $Revision: 6.0.2.8 $
 * $Date: 2011/12/13 22:06:14GMT $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _T6PROTO_H_
#define _T6PROTO_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TM_USE_IPV6


/* Remove any IPv6 auto-configured/discovered entries from the routing tree.
   This includes auto-configured prefixes, auto-discovered default routers,
   etc. */
TM_PROTO_EXTERN void tf6RtDeviceClose(
    ttDeviceEntryPtr devEntryPtr);

#ifdef TM_6_USE_NUD
/*
 * timer that monitors the ARP entries in the reachable state, 
 * and transit the entries with reachable timer lapsed to STATE state
 */
TM_NEARCALL TM_PROTO_EXTERN void tf6NudReachableTimeOut(
    ttVoidPtr           timerBlockPtr,
    ttGenericUnion      userParm1,
    ttGenericUnion      userParm2);
/*
 *  Timer for the NUD delay and PROBE state
 */
TM_PROTO_EXTERN void tf6NudNsTimeOut(
    ttVoidPtr       timerBlockPtr,
    ttGenericUnion  userParm1,
    ttGenericUnion  userParm2);

/* 
 * Update ARP entries as required ny NUD upon receiving of unsolicited 
 * neighbor discovery message 
 */
TM_PROTO_EXTERN void tf6NudUnsolicited(
    tt8Bit              msgType,
    ttIpAddressPtr      ipAddrPtr,
    tt8BitPtr           physAddrPtr,
    tt8Bit              physAddrLength,
    ttDeviceEntryPtr    devEntryPtr,
    tt8Bit              targetIsDest);

#ifdef TM_USE_STOP_TRECK
TM_PROTO_EXTERN void tf6NudDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_STOP_TRECK */

#endif /* TM_6_USE_NUD */

/* update an ARP entry with the tree lock */
TM_PROTO_EXTERN int tf6RtArpUpdateByRte(
    ttRteEntryPtr   rtePtr,
    tt8BitPtr       physAddrPtr,
    tt8Bit          physAddrLen,
    tt8Bit          nudState,
    tt8Bit          isRouter,
    tt16Bit         updateFlags );

/* Select an appropriate source address to be used for communication. */
TM_PROTO_EXTERN int tf6SelectSourceAddress(
    ttDeviceEntryPtr        devEntryPtr,
    tt6IpAddressPtr         srcIpAddrPtr,
    ttDeviceEntryPtrPtr     srcDevEntryPtr,
    tt16BitPtr              srcMHomeIndexPtr,
    tt6ConstIpAddressPtr    destIpAddrPtr,
    tt8BitPtr               destIpAddrTypePtr,
    tt8Bit                  flags);

#ifdef TM_6_USE_RFC3484
/* Perfom add, modify and delete operations on policy table entries */
TM_PROTO_EXTERN int    tf6PolicyTable (
        ttUserInterface interfaceHandle,
        tt8Bit          operationCode,
        struct in6_addr prefix,
        int             prefixLen,
        tt16Bit         precedence,
        tt16Bit         label);

/* Display all policy table entries or by matching precendence or label */
TM_PROTO_EXTERN int    tf6PolicyTableDisplay (
        ttUserInterface          interfaceHandle,
        ttPolicyTableEntryPtrPtr policyTablePtrPtr,
        tt8Bit                   displayAllFlag,
        tt16Bit                  precedence,
        tt16Bit                  label);

/* To indicate the address preference for mobile and privacy IPv6 address */
TM_PROTO_EXTERN int tf6SelectAddressPreference (
        ttUserInterface interfaceHandle,
        tt8Bit          flag,
        tt8Bit          optionValue);

/* To free the policy table */
TM_PROTO_EXTERN int    tf6PolicyTableFree (
        ttPolicyTableEntryPtrPtr policyTablePtrPtr);

/* Initialize the policy table on the interface */
TM_PROTO_EXTERN int    tfPolicyTableInit (
        ttDeviceEntryPtr devPtr);

/* Select an appropriate destination address to be used for communication */
TM_PROTO_EXTERN int    tf6SelectDestinationAddress (
        ttDeviceEntryPtr                  devEntryPtr,
        struct addrinfo TM_FAR * TM_FAR * destIpAddrPtrPtr);
#endif /* TM_6_USE_RFC3484 */

/* Attempt to find a routing entry (either a local network entry, a static
   route or a default gateway) and then cache the result. */
TM_PROTO_EXTERN int tf6IpCacheRte(
    ttRtCacheEntryPtr rtcPtr, tt6ConstIpAddressPtr ipDestAddrPtr);

/*
 * Finds the outgoing device based on the given destination address and fills
 * in the traffic class, flow label, source address and destination address
 * fields in the IPv6 header.
 */
TM_PROTO_EXTERN int tf6IpDestToPacket(
    ttPacketPtr packetPtr, tt6ConstIpAddressPtr srcIpAddrPtr,
    tt6ConstIpAddressPtr destIpAddrPtr, ttRtCacheEntryPtr rtcPtr,
    tt6IpHeaderPtr iphPtr);

/*
 * add or delete a group address to a list of device enabled multicast
 * addresses, specific to Ethernet link-layer
 */
TM_NEARCALL TM_PROTO_EXTERN int tf6EtherMcast(
    ttVoidPtr interfaceHandle, tt6ConstIpAddressPtr groupAddrPtr, int command);

/* IPv6 address configuration/unconfiguration functions that skip multi-home
   index validation, so that they can be called by DHCPv6 */
TM_PROTO_EXTERN int tf6ConfigInterface(
    ttUserInterface            interfaceHandle,
    ttConstSockAddrStoragePtr  ipAddrPtr,
    int                        prefixLen, 
    int                        flags, 
    int                        ipv6Flags, 
    int                        buffersPerFrameCount,
    tt6DevAddrNotifyFuncPtr    dev6AddrNotifyFuncPtr,
    unsigned int               mHomeIndex);

TM_PROTO_EXTERN int tf6UnConfigInterface(
    ttUserInterface     interfaceHandle,
    unsigned int        mHomeIndex);

/*
 * Check whether a user device handle and multi home index are valid,
 * and that the device is configured for that multi home index
 */
TM_PROTO_EXTERN int tf6ValidConfigInterface (
    ttDeviceEntryPtr checkDevEntryPtr,
    tt16Bit          mHomeIndex );

/* validate that we can configure a specified IPv6 address on an interface */
TM_PROTO_EXTERN int tf6ValidConfigAddr(
    tt6ConstIpAddressPtr ipAddrPtr, int prefixLen);

/* tf6PrefixLenToNetmask calculates the IPv6 netmask from a specified prefix
   length. Refer to the macro tm_6_netmask. */
TM_PROTO_EXTERN void tf6PrefixLenToNetmask(
    int prefixLen, tt6IpAddressPtr netmaskPtr);

/* tf6DelayInitSolTimer implements a one-shot timer to delay sending the
   initial Router/Neighbor Solicitation message as specified by requirements
   [RFC2461].R6.3.7:50 and [RFC2462].R5.4.2:30. */
TM_NEARCALL TM_PROTO_EXTERN void tf6DelayInitSolTimer(
    ttVoidPtr      delayTimerPtr,
    ttGenericUnion devEntryPtr,
    ttGenericUnion mHomeIndex );

/* tf6DelayInitSolTimerRemove removes the one-shot timer to delay sending the
   initial Router/Neighbor Solicitation message as specified by requirements
   [RFC2461].R6.3.7:50 and [RFC2462].R5.4.2:30. */
TM_PROTO_EXTERN void tf6DelayInitSolTimerRemove(
    ttDeviceEntryPtr devEntryPtr,
    tt16Bit          mHomeIndex);

/* tf6FinishOpenInterface completes the process of configuring an IPv6
   address on the interface. */
TM_PROTO_EXTERN int tf6FinishOpenInterface(
    ttDeviceEntryPtr devEntryPtr, unsigned int mHomeIndex );

/* tf6SetInterfaceId sets the IPv6 interface ID to the specified 64-bit
   interface ID. ([RFC2464].R4:10, [RFC2464].R4:20) */
TM_PROTO_EXTERN int tf6SetInterfaceId(
    ttDeviceEntryPtr devEntryPtr, tt8BitPtr interfaceIdPtr );

/* Set a random interface ID per [RFC3041] (called with the device lock on). */
TM_PROTO_EXTERN int tf6SetRandomLockedInterfaceId(ttDeviceEntryPtr devPtr);

/* tf6IntEui48SetInterfaceId sets the interface ID of an IPv6-capable interface,
   based on the specified IEEE 48-bit MAC address.
   ([RFC2464].R4:10, [RFC2464].R4:20) */
TM_PROTO_EXTERN int tf6IntEui48SetInterfaceId(
    ttDeviceEntryPtr devEntryPtr, ttConst8BitPtr macAddrPtr );

/* Given an IPv6 address and a device pointer, find the matching IPv6 address
   on that device. */
TM_PROTO_EXTERN int tf6MhomeAddrMatch(
    ttDeviceEntryPtr devEntryPtr,
    tt6ConstIpAddressPtr ipAddrPtr,
    tt16BitPtr multiHomeIndexPtr );

/* Given an IPv6 address and a device pointer, find the prefix-matching 
   IPv6 address on that device. */
TM_PROTO_EXTERN int tf6MhomeNetMatch (
    ttDeviceEntryPtr     devPtr,
    tt6ConstIpAddressPtr ipAddressPtr,
    tt16BitPtr           ifaceMhomePtr);


/* ICMPv6 internal APIs */
/* Process a received ICMPv6 packet.
   ([RFC2463].R2.4:10, [RFC2463].R2.4:20, [RFC2463].R2.4:40, [RFC2463].R3.1:70,
   [RFC2463].R3.2:20, [RFC2463].R3.3:20, [RFC2463].R3.4:20, [RFC2463].R2.3:10,
   [RFC2460].R8.1:50, [RFC2461].R6.1.2:10, [RFC2461].R7.1.1:10,
   [RFC2461].R7.1.2:10, [RFC2461].R8.1:10, [RFC2463].R4.1:20,
   [RFC2463].R4.2:10, [RFC2463].R4.2:50) */
TM_PROTO_EXTERN void tf6IcmpIncomingPacket( ttPacketPtr packetPtr );

/* This function is called when we need to send an ICMPv6 error message in
   response to an incoming IP datagram.
   ([RFC2463].R2.4:30, [RFC2463].R2.4:50, [RFC2463].R2.2:10, [RFC2463].R2.2:20,
   [RFC2463].R2.2:30, ([RFC2463].R2.4:100) */
TM_PROTO_EXTERN void tf6IcmpErrPacket(
    ttPacketPtr origPacketPtr,
    tt8Bit      errType,
    tt8Bit      errCode,
    tt32Bit     errParam );

/* This function sends an ICMPv6 Echo Reply packet in response to an invoking
   Echo Request packet. ([RFC2463].R4.2:30, [RFC2463].R4.2:40,
   [RFC2463].R2.2:10, [RFC2463].R2.2:20) */
TM_PROTO_EXTERN void tf6IcmpEchoReplyPacket(
    ttPacketPtr echoRequestPacketPtr );

/* To Allocate and send a ICMPv6 packet, and then free it */
int tf6IcmpSend(
    tt6ConstIpAddressPtr    srcAddrPtr,
    tt6ConstIpAddressPtr    dstAddrPtr,
    ttDeviceEntryPtr        devPtr,
    tt8Bit                  type,
    tt8Bit                  code,
    tt8BitPtr               dataPtr,
    int                     dataLen,
    tt16Bit                 pktFlags);


/* IPv6 Multicast Listener Discovery internal APIs */
/* Internal API called by setsockopt to set a Multicast Listener Discovery
   socket-level option. */
TM_PROTO_EXTERN int tf6MldSetSockOpt(
    ttSocketEntryPtr    socketEntryPtr,
    int                 optionName,
    ttConstCharPtr      optionValuePtr,
    int                 optionLen );

#ifdef TM_6_USE_MLD
/* Start or restart the delay timer for a specified MLD entry, specifying a
   random delay between 0 and maxRespDelayMsec.
   ([RFC2710].R4:110, [RFC2710].R4:111, [RFC2710].R4:112) */
TM_PROTO_EXTERN void tf6MldStartDelayTimer(
    ttDeviceEntryPtr devEntryPtr,
    tt6MldEntryPtr mldEntryPtr,
    tt16Bit maxRespDelayMsec
#ifdef TM_6_USE_MLDV2
    ,
    tt8Bit    queryType
#endif /* TM_6_USE_MLDV2 */
    );

#ifdef TM_6_USE_MLDV2
TM_PROTO_EXTERN int tf6IPv6MultiCastListen(
    int socketDescriptor,
    ttUserInterface interfaceHandle, 
    tt6IpAddressPtr mcastGroupAddrPtr,
    tt8Bit filterMode,
    tt6SourceAddrEntryPtr sourceAddrEntryPtr );

/* Get index of source in source list */
TM_PROTO_EXTERN int tfMldv2CacheGetSource (
    tt6SourceAddrEntryPtr   srcAddrEntryPtr,
    tt6IpAddressPtr         srcIpAddrPtr);
#endif /* TM_6_USE_MLDV2 */

/* Process a received Multicast Listener Discovery message.
   ([RFC2710].R2:10, [RFC2710].R3.7:10, [RFC2710].R4:100, [RFC2710].R4:120,
   [RFC2710].R4:140, [RFC2710].R9:10) */
TM_PROTO_EXTERN void tf6MldIncomingPacket(
    ttPacketPtr packetPtr, tt8Bit msgType );

/* Internal API called by getsockopt to get a Multicast Listener Discovery
   socket-level option. */
TM_PROTO_EXTERN int tf6MldGetSockOpt(
    ttSocketEntryPtr    socketEntryPtr,
    int                 optionName,
    ttConstCharPtr      optionValuePtr,
    ttIntPtr            optionLenPtr );

/* mcast group in socket cache? */
TM_PROTO_EXTERN tt8Bit tf6MldSocketCacheMatch(
    ttSocketEntryPtr socketEntryPtr,
    ttDeviceEntryPtr devEntryPtr,
    tt6IpAddressPtr  mcastGroupAddrPtr
#ifdef TM_6_USE_MLDV2
    ,
    ttIntPtr         indexPtr
#endif /*TM_6_USE_MLDV2*/
    );

/* Become a member of the specified multicast group, thereby indicating the
   desire to receive packets sent to that multicast group. */
TM_PROTO_EXTERN int tf6MldJoinGroup(
    tt8Bit              socketCall,
    ttDeviceEntryPtr    devEntryPtr,
    tt6IpAddressPtr     mcastGroupAddrPtr
#ifdef TM_6_USE_MLDV2
    ,
    tt8Bit                  filterMode,
    tt6SourceAddrEntryPtr   sourceAddrEntryPtr,
    tt8Bit                  socMatch
#endif /*TM_6_USE_MLDV2*/
    );

/* Remove membership in the specified multicast group, thereby indicating the
   desire to stop receiving packets sent to that multicast group. */
TM_PROTO_EXTERN int tf6MldLeaveGroup(
    tt8Bit              socketCall,
    ttDeviceEntryPtr    devEntryPtr, 
    tt6IpAddressPtr     mcastGroupAddrPtr
#ifdef TM_6_USE_MLDV2
    ,
    tt8Bit                  filterMode,
    tt6SourceAddrEntryPtr   sourceAddrEntry
#endif /* TM_6_USE_MLDV2 */
    );

/* Leave all multicast groups joined on this socket. Free up cache area. */
TM_PROTO_EXTERN void tf6MldSocketCachePurge(
    ttSocketEntryPtr socketEntryPtr
#ifdef TM_6_USE_MLDV2
    ,
    tt8Bit                  filterMode,
    tt6SourceAddrEntryPtr   sourceAddrEntry
#endif /* TM_6_USE_MLDV2 */
    );

#ifdef TM_6_USE_RFC3590
/* Send new MLD Report messages for all multicast addresses to join on the
   interface after a valid link-local address is available. */
TM_PROTO_EXTERN void tf6InitMldSend(
    ttDeviceEntryPtr    devEntryPtr);
#endif /* TM_6_USE_RFC3590 */
#endif /* TM_6_USE_MLD */
/*
 * Add an entry on the specificied interface for the specified multicast group.
 */
tt6MldEntryPtr tf6MldAddGroup(ttDeviceEntryPtr devEntryPtr,
                              tt6IpAddressPtr  mcastGroupAddrPtr
#ifdef TM_6_USE_MLDV2
                              ,
                              tt8Bit                    filterMode,
                              tt6SourceAddrEntryPtr     sourceAddrEntry
#endif /* TM_6_USE_MLDV2 */
                              );
/*
 * Delete multicast entry, notifiying the device driver
 * the desire to stop receiving packets sent to that multicast group.
 */
int tf6MldDeleteGroup(ttDeviceEntryPtr    devEntryPtr,
                      tt6IpAddressPtr   mcastGroupAddrPtr);

/* This function is called by tf6IpIncomingPacket to determine if a specified
   multicast group address has been joined on a specified interface. */
/* NOTE: tf6MldIsMember is intentionally left enabled even when TM_6_USE_MLD
   is not #define'd. This is mainly done so that it would be possible to
   provide some level of support for receiving multicast packets, even when
   MLD is not desired. */
TM_PROTO_EXTERN int tf6MldIsMember(
    ttDeviceEntryPtr    devEntryPtr,
    tt6IpAddressPtr     mcastGroupAddrPtr,
    tt16BitPtr          mhomeIndexPtr );


/* IPv6 Stateless Address Auto-Configuration internal APIs */
/* This function initiates stateless address auto-configuration, by attempting
   to auto-configure a link-local IPv6 address on the specified interface. */
TM_PROTO_EXTERN int tf6ConfigInterfaceId( ttDeviceEntryPtr devEntryPtr );

/* This function unconfigures the interface ID from the specified interface,
   which includes unconfiguring all IPv6 addresses previously configured on the
   interface using stateless address auto-configuration (i.e. using the
   interface ID). */
TM_PROTO_EXTERN int tf6UnConfigInterfaceId( ttDeviceEntryPtr devEntryPtr );

/* This function initiates manual configuration of the specified IPv6 address
   on the specified interface at the specified multi-home index. */
TM_PROTO_EXTERN int tf6DadConfigAddr(
    ttDeviceEntryPtr    devEntryPtr,
    tt16Bit             multiHomeIndex,
    tt6IpAddressPtr     ipAddrPtr,
    tt8Bit              prefixLen );

#ifdef TM_6_USE_PREFIX_DISCOVERY
/* This function initiates stateless address auto-configuration of the
   specified IPv6 address prefix on the specified interface. */
TM_PROTO_EXTERN int tf6DadAutoConfigPrefix(
    ttDeviceEntryPtr    devEntryPtr, 
    tt6IpAddressPtr     prefixPtr );
#endif /* TM_6_USE_PREFIX_DISCOVERY */

/* This function is called when Duplicate Address Detection has successfully
   completed for our tentative address, and after tfDeviceStart has completed
   the configuration of the address on the interface. */
TM_PROTO_EXTERN void tf6DadConfigComplete(
    ttDeviceEntryPtr    devEntryPtr, 
    tt16Bit             multiHomeIndex );

TM_PROTO_EXTERN int tf6DadFinishConfigAddr(
    ttDeviceEntryPtr devEntryPtr,
    tt16Bit          multiHomeIndex);

#ifdef TM_6_USE_DAD
/* This function is called by tf6NdIncomingPacket when a Neighbor Solicitation
   or Neighbor Advertisement message is received to lookup the DAD entry (if
   there is one) associated with the target address in the message. */
TM_PROTO_EXTERN tt6DadEntryPtr tf6DadFindEntry(
    ttDeviceEntryPtr        devEntryPtr, 
    tt6ConstIpAddressPtr    targetAddrPtr );

/* This function is called to unlink the specified DAD entry from the
   linked-list of DAD entries chained off the interface, stop the associated
   DAD timer if there is any, and recycle the DAD entry. */
TM_PROTO_EXTERN void tf6DadRemoveEntry(
    ttDeviceEntryPtr devEntryPtr, tt6DadEntryPtr dadEntryPtr );

/* This function is called when Duplicate Address Detection has failed for our
   tentative address. */
TM_PROTO_EXTERN void tf6DadFailed(
    ttDeviceEntryPtr    devEntryPtr,
    tt16Bit             multiHomeIndex,
    tt6DadEntryPtr      dadEntryPtr);

/* This function is called when we detect that a different node is using an
   IPv6 address that we have already configured on the interface.
   ([RFC2462].R5.4.4:10) */
TM_PROTO_EXTERN void tf6DadDupDetected(
    ttDeviceEntryPtr    devEntryPtr, 
    tt16Bit             multiHomeIndex );
#endif /* TM_6_USE_DAD */

/* Called by tfNgConfigInterface to add a local route */
TM_PROTO_EXTERN int tf6RtAddLocal( 
    ttDeviceEntryPtr    devEntryPtr,
/* local destination network for Ethernet, Remote IP address for SLIP/PPP */
    ttIpAddressPtr      destinationPtr,
/* Network mask of local network (All one's for SLIP/PPP) */
    int                 myNetPrefixLength,
/* IP address of the interface */
    ttIpAddressPtr      myIpAddrPtr,
/* Multihome index of the interface */
    tt16Bit             myMultiHomeIndex );

/*
 * Called by tfNgDelDefaultGateway, or by tfNgDelStaticRoute, or by
 * tfNgUncongigInterface to delete a route
 */
TM_PROTO_EXTERN int tf6RtDelRoute(
#ifdef TM_USE_STRONG_ESL
   ttDeviceEntryPtr     devEntryPtr,
#endif /* TM_USE_STRONG_ESL */
    ttIpAddressPtr      destIpAddrPtr,
    int                 destMaskPrefixLength,
    tt16Bit             rteCreateFlag );

/*
 * Add a non local route to the routing tree.
 * Called by tfNgAddStaticRoute(), and other to lock the routing
 * table, allocate a new router entry and insert it in the routing table
 */
TM_PROTO_EXTERN int tf6RtAddRoute(
/* route device */
    ttDeviceEntryPtr    devEntryPtr,
/* Destination Ip address */
    ttIpAddressPtr      IpAddrPtr,
/* destination network address */
    int                 netMaskPrefixLength,
/* Gateway for non local routes, interface IP address for local routes. */
    ttIpAddressPtr      gwayAddrPtr,
/* Gateway Tag (for RIP) */
    tt16Bit             gwayTag,
/* multihome index */
    tt16Bit             multiHomeIndex,
/* metrics for non local routes */
    int                 hops,
    tt32Bit             ttl,
/* flag to indicate owner local/static/redirect/RIP */
    tt16Bit             flags,
    tt8Bit              needLock );

TM_PROTO_EXTERN int tf6RtAddHost(
 
    ttDeviceEntryPtr       devEntryPtr,     /* route device */
    tt6ConstIpAddressPtr   ip6AddrPtr,      /* Destination Ip address */
    tt8BitPtr              physAddrPtr,     /* physical address */
    tt8Bit                 physAddrLen,     /* physical address len */
    tt16Bit                multiHomeIndex,  /* multihome index */
    tt32Bit                ttl,             /* metrics for non local routes */
    tt16Bit                flags,           /*cloned/static/link-layer/mcast */
    tt8Bit                 needLock );

/* Called by ICMPv6 when a redirect has been received */    
TM_PROTO_EXTERN int tf6RtRedirect(
    ttDeviceEntryPtr    devEntryPtr,
    tt16Bit             mhomeIndex,
    tt6IpAddressPtr     destIpAddrPtr,
    tt6IpAddressPtr     gwIpAddrPtr,
    tt6IpAddressPtr     srcIpAddrPtr,
    tt8BitPtr           physAddrPtr,
    tt8Bit              physAddrLen);      

#ifdef TM_6_USE_PREFIX_DISCOVERY
/* Removes an entry from the 'non-on-link' prefix list. */
TM_PROTO_EXTERN int tf6RtDelNonOnLink(tt6IpAddressPtr prefixAddrPtr,
                                      tt8Bit          prefixLen);

/* Check if the specified prefix exists in the 'non-on-link' prefix list. */
TM_PROTO_EXTERN int tf6RtCheckNonOnLink(tt6IpAddressPtr     prefixAddrPtr,
                                        tt8Bit              prefixLen);

/* Updates the lifetimes of a non-on-link prefix, adding entry if necessary. */
TM_PROTO_EXTERN int tf6RtUpdateNonOnLink(ttDeviceEntryPtr   devPtr,
                                         tt6IpAddressPtr    prefixAddrPtr,
                                         tt8Bit             prefixLen,
                                         tt32Bit            validLifetime,
                                         tt32Bit            preferredLifetime,
                                         tt8Bit             ipsecAuthFlag);
    
/* This function locks the routing tree so that it can walk and age IPv6
   address prefixes from preferred to deprecated to invalid. The IPv6
   addresses that have been manually and auto-configured using the affected
   address prefixes go through the same state transitions as the associated
   prefixes. */
TM_NEARCALL TM_PROTO_EXTERN void tf6RtPrefixAgingTimer(
    ttVoidPtr prefixAgingTimerPtr,
    ttGenericUnion userParm1,
    ttGenericUnion userParm2 );

TM_PROTO_EXTERN void tf6RtStartPrefixAgingTimer(void);

/* This function is the address invalidation timer specified by requirement
   [RFC2461].R6.3.4:130. */
TM_NEARCALL TM_PROTO_EXTERN void tf6RtInvalidatePrefixTimer(
    ttVoidPtr invalidateTimerPtr,
    ttGenericUnion userParm1,
    ttGenericUnion userParm2 );

/* This function deprecates an address prefix, which causes all of the
   configured addresses having that prefix to also become deprecated. */
TM_NEARCALL TM_PROTO_EXTERN void tf6RtDeprecatePrefixTimer(
    ttVoidPtr deprecateTimerPtr,
    ttGenericUnion userParm1,
    ttGenericUnion userParm2 );

/* common code for all prefix timer handlers */
TM_PROTO_EXTERN void tf6PrefixTimerCommon(
    ttGenericUnion userParm1,
    ttGenericUnion userParm2,
    tt6PrefixFuncPtr prefixFuncPtr);
/* Undeprecate the specified IPv6 address prefix on the specified interface */
TM_NEARCALL TM_PROTO_EXTERN void tf6UnDeprecatePrefix(
    ttDeviceEntryPtr devEntryPtr, tt6IpAddressPtr prefixPtr );
#endif /* TM_6_USE_PREFIX_DISCOVERY */
    
/* This function updates or retrieves prefix lifetimes, and determines if
   certain prefixes exist. */
TM_PROTO_EXTERN int tf6RtKeyCommand(tt6RtKeyCommandPtr keyCmdPtr,
                                    tt8Bit             needLock);

/* Called by either ICMPv6 in response to a "Packet Too Big" message or called
   by the user to disable PMTU for this destination. */
TM_PROTO_EXTERN int tf6RtNextHopMtu(
#ifdef TM_USE_STRONG_ESL
                                    ttDeviceEntryPtr     devPtr,
#endif /* TM_USE_STRONG_ESL */
                                    tt6ConstIpAddressPtr ip6AddrPtr,
                                    tt16Bit              nextHopMtu,
                                    tt8Bit               isStaticFlag);
#ifdef TM_USE_STRONG_ESL
ttDeviceEntryPtr tf6RtGetNextDefaultGatewayList(unsigned int index);
#endif /* TM_USE_STRONG_ESL */
    
/* IPv6 Address Resolution internal APIs */
TM_PROTO_EXTERN void tf6NdFinishAddrRes(
    ttDeviceEntryPtr devEntryPtr, 
    ttIpAddressPtr   ipAddrPtr,
    tt8BitPtr        llAddrPtr,
    tt8Bit           llAddrLen);
/* Send a Neighbor Discovery solicitation (address resolution request). */
TM_PROTO_EXTERN int tf6NdSendNeighSolicit(
    ttDeviceEntryPtr     devEntryPtr,
    tt6ConstIpAddressPtr targIpAddrPtr,
    tt6ConstIpAddressPtr srcAddrPtr,
    tt6ConstIpAddressPtr destAddrPtr );

/* Send Neighbor Advertisement for the specified address. */
TM_PROTO_EXTERN int tf6NdSendNeighAdvert(
    ttPacketPtr          packetPtr,
    ttDeviceEntryPtr     devEntryPtr,
    tt16Bit              mHomeIndex,
    tt6ConstIpAddressPtr destAddrPtr,
    tt8Bit               solicitFlag );

/* Process incoming IPv6 Neighbor Discovery advertisement. */
TM_NEARCALL TM_PROTO_EXTERN void tf6NdAdvertIncoming( ttPacketPtr packetPtr );


/* Determine the link-layer address of a neighbor given only its IP address. */
TM_NEARCALL TM_PROTO_EXTERN int tf6NdResolveAddress( ttPacketPtr packetPtr );

/*
 * Search through the list of unresolved addresses associated with this
 * interface.
 */
TM_PROTO_EXTERN ttResEntryPtr tf6NdResolveSearch(
    ttDeviceEntryPtr devEntryPtr,
    tt6IpAddressPtr ipAddressPtr);

/*
 * Remove a resolve entry from the list of unresolved addresses associated
 * with this interface.
 */
TM_PROTO_EXTERN void tf6NdResEntryRemove(
    ttDeviceEntryPtr devEntryPtr,
    ttResEntryPtr    resEntryPtr,
    ttPacketPtr      packetPtr);

/*
 * Remove all matching resolve entries from the list of unresolved addresses 
 * associated with this interface.
 */
TM_NEARCALL TM_PROTO_EXTERN void tf6NdResolveClean(ttUserInterface interfaceHandle,
                                       ttIpAddressPtr  ipAddrPtr,
                                       int             prefixLength);

/* IPv6 Parameter Discovery internal APIs */
/* Process incoming router advertisement packets for the various parameters,
   including retrans time, base reachable time, hop limit, MTU and prefix
   information. */
TM_PROTO_EXTERN void tf6ParamDiscIncomingPacket( ttPacketPtr packetPtr );


/* IPv6 Neighbor and Router Discovery internal APIs */
/* Send an ICMPv6 router solicitation. */
TM_PROTO_EXTERN int tf6SendRtrSol( ttDeviceEntryPtr devPtr );

/* Start soliciting the local router(s) for an advertisement.  This routine is
   called when the interface is opened, reopened or has been disabled or
   detached from the link for some time. [RFC2461].R6.3.7:10 */
TM_PROTO_EXTERN void tf6InitRtrSol( ttDeviceEntryPtr devPtr );

/* Remove the router solicitation timer */
TM_PROTO_EXTERN void tf6RemoveRtrSol(ttDeviceEntryPtr devEntryPtr);

/* Process a received Neighbor Discovery message.
   ([RFC2461].R6.2.6:10, [RFC2461].R6.1.2:10, [RFC2461].R7.1.1:10,
   [RFC2461].R7.1.2:10, [RFC2461].R8.1:10, [RFC2464].R2:20, [RFC2464].R2:30) */
TM_PROTO_EXTERN void tf6NdIncomingPacket(
    ttPacketPtr packetPtr, tt8Bit msgType, tt8Bit msgCode );

/* Add the specified gateway to the list of default routers. */
TM_PROTO_EXTERN int tf6RtAddDefRouter(
    tt6IpAddressPtr     ipAddrPtr,
    ttDeviceEntryPtr    devEntryPtr,
    tt32Bit             ttl,
    tt16Bit             flags);

/* Remove the gateway with the given IPv6 address from the list of default
   routers. */
TM_PROTO_EXTERN int tf6RtDelDefRouter(
#ifdef TM_USE_STRONG_ESL
                                      ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                                      tt6IpAddressPtr  ipAddrPtr,
                                      tt8Bit           needLock );

/*
 * Searches the list of default routers for an entry with the specified flags.
 * copies the IPv6 address of the first entry found.
 */
TM_PROTO_EXTERN int tf6RtGetDefRouter(
#ifdef TM_USE_STRONG_ESL
                                      ttDeviceEntryPtr devPtr,
#endif /* TM_USE_STRONG_ESL */
                                      tt6IpAddressPtr ip6AddrPtr,
                                      tt16Bit         flags);
    
/*
 * Used by SNMP
 * Searches the list of default routers for an entry that matches the IPv6 address if
 * it is not any, or the specified flags otherwise.
 * Returns the pointer to the first matching router found.
 */
TM_PROTO_EXTERN tt6RtDefRouterEntryPtr tf6RtGetDefRouterDrt(
#ifdef TM_USE_STRONG_ESL
                                      ttDeviceEntryPtr      devPtr,
#endif /* TM_USE_STRONG_ESL */
                                      tt6ConstIpAddressPtr  ip6AddrPtr,
                                      tt16Bit               flags);
/*
 * Used by SNMP
 * Walk through the default router lists.
 * Return the next default router starting at the passed default router.
 */
TM_PROTO_EXTERN tt6RtDefRouterEntryPtr tf6RtGetNextDefRouterDrt(
#ifdef TM_USE_STRONG_ESL
                                      ttDeviceEntryPtr       devPtr,
#endif /* TM_USE_STRONG_ESL */
                                      tt6RtDefRouterEntryPtr def6RtrPtr);

/*
 * Update IPv6 routing entries for a given device with a new MTU received in a
 * router advertisement.  Called with the tree unlocked.
 */   
TM_PROTO_EXTERN void tf6RtUpdateDevMtu(ttDeviceEntryPtr devEntryPtr,
                                       tt16Bit          newMtu);
    
/* Host-specific fragmentation of a packet, called when sending a packet that
   is larger than the link MTU of the outgoing interface. */
TM_PROTO_EXTERN int tf6IpFragmentPacket(
    ttPacketPtr origPacketPtr,
    ttPktLen    unFragLen,
    ttPktLen    devMtu,
    tt8Bit      nextHdr );

/* This function is invoked in the receive-path to process a received Fragment
   extension header, and to reassemble a fragmented IPv6 packet.
   ([RFC2460].R4:100, [RFC2460].R4.5:20, [RFC2460].R5:80, [RFC2460].R5:90,
   [RFC2460].R4.5:80, [RFC2460].R4.5:90, [RFC2460].R4.5:100,
   [RFC2460].R4.5:110, [RFC2460].R4.5:120, [RFC2460].R4.5:130,
   [RFC2460].R4.5:140, [RFC2460].R4.5:150, [RFC2460].R4.5:160) */
TM_PROTO_EXTERN ttPacketPtr tf6IpReassemblePacket(
    ttPacketPtr packetPtr, tt8BitPtr ipHdrPtr );

#ifdef TM_USE_STOP_TRECK
TM_PROTO_EXTERN void tf6IpFragDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_STOP_TRECK */

/* Process a received IPv6 packet. ([RFC2460].R4:10, [RFC2460].R4:40,
   [RFC2460].R4.1:30, [RFC2460].R4.7:10, [RFC2460].R4:20, [RFC2460].R4:30,
   [RFC2460].R4:50, [RFC2460].R4:60, [RFC2463].R3.1:20, [RFC2893].R5.5:30,
   [RFC2373].R2.8:10, [RFC2462].R5.5.4:40, [RFC2462].R5.5.4:100,
   [RFC2373].R2.5.4:30) */
TM_PROTO_EXTERN void tf6IpIncomingPacket(ttPacketPtr               packetPtr
#ifdef TM_USE_IPSEC_TASK
                                         , ttIpsecTaskListEntryPtr varsPtr
#endif /* TM_USE_IPSEC_TASK */
                                         );

/* Maps an IPv6 multicast address into the corresponding hardware address. */
TM_PROTO_EXTERN int tf6IpMcastToLan(tt6ConstIpAddressPtr    ipAddrPtr,
                                    ttDeviceEntryPtr        devEntryPtr,
                                    tt8BitPtr               linkAddrPtr);
    
/* Given an IP address, matching function find if any device has a matching
 * multihome entry
 */ 
TM_PROTO_EXTERN ttDeviceEntryPtr tf6IfaceMatch (
    tt6ConstIpAddressPtr ipAddressPtr, tt6MhomeFunctPtr mhomeMatchFuncPtr,
    tt16BitPtr ifaceMhomePtr);
    
/*
 * Send an IPv6 packet.  Called from an ULP or from tf6IpForwardPacket.
 * Assumes that pktLinkDataPtr points at start of ULP header if called from
 * ULP or at the start of the IPv6 header if being forwarded.
 * ([RFC2460].R4.5:10, [RFC2893].R5.3:15, [RFC2893].R5.3:20, [RFC2460].R5:70,
 * [RFC2461].R5.2:10, [RFC2461].R5.2:20, R2.4:10, [RFC2460].R7:20, R2.4:20,
 * [RFC2460].R6:10, [RFC2460].RA:20, [RFC2460].RA:30, [RFC2460].RA:40)
 */
TM_PROTO_EXTERN int tf6IpSendPacket(ttPacketPtr               packetPtr,
                                    ttLockEntryPtr            lockEntryPtr
#ifdef TM_USE_IPSEC_TASK
                                    , ttIpsecTaskListEntryPtr varsPtr
#endif /* TM_USE_IPSEC_TASK */
                                    );       
    
/*
 * User call to send data on a raw socket. If raw socket is of type ICMPv6
 * for example, ICMPv6 header filled by the user.
 * Destination IP address stored in ttPacket. (If source known, stored in
 * ttPacket)
 */
TM_PROTO_EXTERN int tf6RawSendPacket(ttSocketEntryPtr      sockPtr,
                                     ttPacketPtr           packetPtr );

/*
 * This routine is called when an address on this interface changes state.
 * This includes being added or removed from the interface, as well as
 * changing from a preferred to a deprecated address, and vice versa.
 * tf6UpdateCacheAddres will determine if the current cached address for this
 * address type (site-local, 6to4, etc) should be replaced.  The device should
 * be locked when calling and will remain lock when returned.
 */ 
TM_PROTO_EXTERN void tf6UpdateCacheAddress(
    ttDeviceEntryPtr devEntryPtr, tt16Bit mHomeIndex); 

/* Initialize a sockaddr_in6 from an IPv6 address */
TM_PROTO_EXTERN void tf6SetSockAddrFromIpAddr(
    tt6IpAddressPtr ipAddrPtr,
    ttDeviceEntryPtr devEntryPtr,
    struct sockaddr_in6 TM_FAR * sockAddrPtr);
    
#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
/* APIs for sorting IP addresses to improve performance of IP address lookup
   in the receive-path when many IP aliases are configured on a single
   interface. */

TM_PROTO_EXTERN void tf6InsertSortedIpAddr(
    ttDeviceEntryPtr insertDevEntryPtr, tt16Bit insertMHomeIndex,
    ttSortedIpAddrCachePtr ipAddrCachePtr);
TM_PROTO_EXTERN ttIpAddrEntryPtr tf6LookupSortedIpAddr(
    tt6ConstIpAddressPtr ipAddrPtr, int devIndex,
    ttSortedIpAddrCachePtr ipAddrCachePtr);
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */

/* Initializes IPv6-over-IPv4 automatic tunnels. */
TM_PROTO_EXTERN ttDeviceEntryPtr tf6InitAutoTunnels(void);

/*
 * Creates an IPv6 over IPv6 tunnel virtual device.  Creates the virtual
 * tunnel device and sets the appropriate link layer function pointers, as
 * well as setting the global function pointer to handle ICMPv6 messages from
 * within the tunnel.
 */
TM_PROTO_EXTERN ttDeviceEntryPtr tf6AddIpv6Tunnel(int TM_FAR * errorPtr);

/* Configures a tunnel on the specified virtual tunnel interface. */
TM_PROTO_EXTERN int tfConfigTunnel(
          ttDeviceEntryPtr                 tunnelDevPtr,
          ttDeviceEntryPtr                 entryPtDevPtr,
          tt16Bit                          entryPtMhomeIndex,
    const struct sockaddr_storage TM_FAR * destAddrPtr,
          int                              prefixLen,
    const struct sockaddr_storage TM_FAR * endpointAddrPtr,
    const struct sockaddr_storage TM_FAR * srcAddrPtr,
          tt16Bit                          flags);
/*
 * Unconfigures the specified tunnel virtual device.  Currently, we assume
 * that this is only unconfiguring an IPv6-over-IPv6 or an IPv6-over-IPv4
 * tunnel.
 */
TM_PROTO_EXTERN int tfUnConfigTunnel(ttDeviceEntryPtr tunnelDevPtr,
                                     tt16Bit          flags);


/* Creates an IPv6 over IPv4 tunnel virtual device. */
TM_PROTO_EXTERN ttDeviceEntryPtr tf6AddIpv4Tunnel(int TM_FAR * errorPtr);
    
TM_NEARCALL TM_PROTO_EXTERN int tfTunnelSendStub(ttPacketPtr packetPtr);

/* common function used by generic IPv6 tunnel and Mobile IPv6 mobile node 
   MN-to-HA tunnel link-layer send functions */
TM_PROTO_EXTERN int tf6Ipv6TunnelSendToFrom(
    ttPacketPtr                 packetPtr,
    tt6ConstIpAddressPtr        entryIpAddrPtr,
    tt6ConstIpAddressPtr        exitIpAddrPtr );

/* Processes an incoming raw packet.  Called from tf6IpIncomingPacket. */
TM_PROTO_EXTERN  int tf6RawIncomingPacket(ttPacketPtr packetPtr, tt8Bit ulp);

#ifdef TM_6_USE_MIP_RO
/* Uncache all routes (CN, and/or MN with RO) */
TM_PROTO_EXTERN void tf6MipRrResetRouteCaches(
                                          tt6ConstIpAddressPtr destIpAddrPtr,
                                          tt6ConstIpAddressPtr srcIpAddrPtr );
#endif /* TM_6_USE_MIP_RO */

#ifdef TM_6_USE_MIP_MN
#ifdef TM_6_USE_MIP_RO
/* attempt to start RO with a peer on receiving a ULP packet from the peer */
TM_PROTO_EXTERN void tf6MnStartRoIncomingPacket(ttPacketPtr packetPtr);
#endif /* TM_6_USE_MIP_RO */

/* start/restart beacon timer used to monitor Router Advertisements sent by
   current default router, period is multiple of Advertisement Interval
   option in RA */
TM_PROTO_EXTERN void tf6MnStartBeaconTimer(tt32Bit advertIntervalMsec);

/* stop beacon timer, called when we switch away from a default router */
TM_PROTO_EXTERN void tf6MnStopBeaconTimer(void);

/* execute a move away from home */
TM_PROTO_EXTERN int tf6MnMoveFromHome(void);
#ifndef TM_6_MN_DISABLE_HOME_DETECT
/* execute a move to home */
TM_PROTO_EXTERN int tf6MnMoveToHome(void);
#endif /* ! TM_6_MN_DISABLE_HOME_DETECT */

/* build Home Address destination option as ancillary data */
TM_PROTO_EXTERN void tf6MnBuildHaoDestExtHdr(
    ttAncHdrPtr         ancHdrPtr,
    tt6IpAddressPtr     coAddrPtr);

/* Process a received Binding Acknowledgement message */
TM_PROTO_EXTERN void tf6MipBaIncomingPacket(ttPacketPtr packetPtr);

/* Process a received Binding Error message */
TM_PROTO_EXTERN void tf6MipBeIncomingPacket(ttPacketPtr packetPtr);

/* Process a received Binding Refresh Request message */
TM_PROTO_EXTERN void tf6MipBrrIncomingPacket(ttPacketPtr packetPtr);

/* Process a received Home Test message */
TM_PROTO_EXTERN void tf6MipHotIncomingPacket(ttPacketPtr packetPtr);

/* Process a received Care-of Test message */
TM_PROTO_EXTERN void tf6MipCotIncomingPacket(ttPacketPtr packetPtr);

/* One-shot timer, initiates L3 move processing. */
TM_PROTO_EXTERN void tf6MnStartL3MoveTimer(
    ttVoidPtr l3MoveTimerPtr,
    ttGenericUnion userParm1,
    ttGenericUnion userParm2 );

/* mobile node state machine for home registration BUL entry */
TM_PROTO_EXTERN int tf6MnStateMachine(
    tt6MnHaBulEntryPtr bulHaEntryPtr,
    ttPacketPtr packetPtr,
    tt6MnsEvent event );

#ifdef TM_6_USE_MIP_RO
/* mobile node state machine for route optimization BUL entry */
TM_PROTO_EXTERN int tf6MnRrStateMachine(
    tt6MnRrBulEntryPtr bulRrEntryPtr,
    ttPacketPtr packetPtr,
    tt6MnsEvent event );

/* find the specified route optimization BUL entry, if it exists */
TM_PROTO_EXTERN tt6MnRrBulEntryPtr tf6MnFindRrBulEntry(
    tt16Bit                     homeAddrMHomeIndex,
    tt6ConstIpAddressPtr        cnIpAddrPtr );

/* find an existing or add a new route optimization BUL entry */
TM_PROTO_EXTERN tt6MnRrBulEntryPtr tf6MnAddRrBulEntry(
    tt16Bit                     mHomeIndex,
    tt6ConstIpAddressPtr        cnIpAddrPtr );
#endif /* TM_6_USE_MIP_RO */

/* failure timer for home registration BUL entry */
TM_PROTO_EXTERN void tf6MnHaBulFailureTimer(
    ttVoidPtr failureTimerPtr,
    ttGenericUnion userParm1,
    ttGenericUnion userParm2 );

#ifdef TM_6_USE_NUD
/* initiate NUD probing of current default router */
TM_PROTO_EXTERN void tf6NudProbeDefRouter(void);
#endif /* TM_6_USE_NUD */

/* remove all default routers from the default router list */
TM_PROTO_EXTERN void tf6RtDelAllDefRouter(
#ifdef TM_USE_STRONG_ESL
                                          ttDeviceEntryPtr devPtr
#endif /* TM_USE_STRONG_ESL */
                                         );

/* remove all dynamically created entries associated with the specified
   interface from the routing tree */
TM_PROTO_EXTERN void tf6MnResetInterface(ttDeviceEntryPtr devEntryPtr);

/* initiate TCP fast retransmits on the specified interface */
TM_PROTO_EXTERN void tfTcpBackOnLink(ttDeviceEntryPtr devEntryPtr);

#ifdef TM_USE_IKE
/* initialize the mobile node */
TM_PROTO_EXTERN void tf6MnInit( void );
#endif /* TM_USE_IKE */

/* Processing the DHAAD REPLY incoming message */
TM_PROTO_EXTERN void tf6MnDhaadReplyIncomingPacket(ttPacketPtr packetPtr);

/* Process the mobile prefix advertisement message */
TM_PROTO_EXTERN void tf6MobilePrefixAdvIncomingPacket(ttPacketPtr packetPtr);

#ifdef TM_USE_STOP_TRECK
TM_PROTO_EXTERN void tf6MnDeInit (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                );
#endif /* TM_USE_STOP_TRECK */

#endif /* TM_6_USE_MIP_MN */

#endif /* TM_USE_IPV6 */

#if (defined(TM_6_USE_MIP_MN) || defined(TM_6_USE_MIP_CN))
/*
 *  called by tf6IpIncomingPacket () when proto==IPPROTO_MH (TBD)
 *  call individual handler to handle difference MH types
 */
TM_PROTO_EXTERN void tf6MobilityHdrIncomingPacket(ttPacketPtr packetPtr);

/*
 *  called by tf6MobilityHdrIncomingPacket when the module handles this 
 *  message type is not enabled by macro
 */
TM_PROTO_EXTERN void tf6MipNullIncomingPacket(ttPacketPtr packetPtr);

/* prepare and send a MH message */
TM_PROTO_EXTERN int tf6MipSendMh(
    tt6MipHeaderPtr         mhPtr,
    ttListPtr               optListPtr,
    tt6ConstIpAddressPtr    srcAddrPtr,
    tt6ConstIpAddressPtr    destAddrPtr,
    ttConst8BitPtr          hoKeygenPtr,
    ttConst8BitPtr          coKeygenPtr,
    tt6ConstIpAddressPtr    coAddrPtr,
    tt6ConstIpAddressPtr    finalDestAddrPtr,
    ttAncHdrPtr             ancHdrPtr,
    tt16Bit                 pktFlags);

/*
 *  compute the authenticator, can be used by MN and CN to generate and 
 *  re-generate the authenticator for BU and BA message.
 */
TM_PROTO_EXTERN void tf6MipComputeAuth (
    ttConst8BitPtr          hoKeygenPtr,
    ttConst8BitPtr          coKeygenPtr,
    tt6ConstIpAddressPtr    coIpAddrPtr,
    tt6ConstIpAddressPtr    cnIpAddrPtr,
    ttConst8BitPtr          mhPtr,
    int                     mhLen,
    tt8BitPtr               authPtr);

/* This function gets the total length of the options*/
TM_PROTO_EXTERN int tf6GetOptionListLength(ttListPtr optListPtr);

/* Get the options out of the list of option and copy them into a buffer 
 * if padLen is not zero, append a PAD to the end of the buffer
 * caller make sure that bufferPtr is big enough
 */
TM_PROTO_EXTERN void tf6OptionListToBuffer(
    const ttListPtr     optListPtr,
    tt8BitPtr           bufferPtr,
    tt16Bit             padLen);

/* prepare and send a BE message */
TM_PROTO_EXTERN int tf6MipSendBe (
    tt8Bit                  satusCode,
    tt6ConstIpAddressPtr    srcAddrPtr,
    tt6ConstIpAddressPtr    destAddrPtr,
    tt6ConstIpAddressPtr    homeAddrPtr);
#endif /* TM_6_USE_MIP_MN || TM_6_USE_MIP_CN */

#ifdef TM_6_USE_MIP_CN
/* reset CN route caches for a specified MN home address */
TM_PROTO_EXTERN void tf6CnResetRouteCaches(
    tt6ConstIpAddressPtr    destIpAddrPtr );
/*
 *  Add a new binding entry into the binding cache, or update 
 *  an existing one. If adding a new binding entry, if the 
 *  binding cache is already full, try to remove a least used 
 *  entry to insert the new binding entry. Add/update the host 
 *  entry into the routing tree, key is the MN's home address.
*/
TM_PROTO_EXTERN int tf6MipSetBinding(
    tt6ConstIpAddressPtr    homeAddrPtr,
    tt6ConstIpAddressPtr    coAddrPtr,
    tt32Bit                 lifetime,
#ifdef TM_6_USE_MIP_HA
    tt8Bit                  homeReg,
    tt16Bit                 proxyMHome,
#endif /* TM_6_USE_MIP_HA */
    tt16Bit                 seq);

/*
 *  Delete an existing binding entry and remove the correspondent
 *  host entry from the routing tree.
 */
TM_PROTO_EXTERN int tf6MipDelBinding(tt6ConstIpAddressPtr homeAddrPtr);

/*
 *  Get the binding entry that matches the given home address. The 
 *  binding cache must be locked when this function is called. MUST 
 *  be called with tvBindingLock locked.
 */
TM_PROTO_EXTERN tt6BindingEntryPtr tf6MipGetBinding(
    tt6ConstIpAddressPtr homeAddrPtr);

/*
 *  scans through the binding cache when adding, deleting, or updating any
 *  binding cache entry, perform lifetime expiration process for 
 *  expiring/expired entries, and reschedule the timer near the expiration 
 *  time of the next entry to expire.
 */
TM_PROTO_EXTERN void tf6MipBindingTimeOut(
    ttVoidPtr       timerBlockPtr,
    ttGenericUnion  userParm1,
    ttGenericUnion  userParm2);

/*
 *  called by tf6OptsExtHdrIncomingPacket () when 
 *  proto==IPPROTO_DSTOPTS &&
 *  optionType == TM_6_HDR_OPT_HOME_ADDR 
 *  check the validity of the option, if valid, replace the source address in 
 *  the IP header with the home address, if not, drop the packet.
 */
TM_PROTO_EXTERN int tf6HaoOptsExtHdrIncomingPacket(
    ttPacketPtr     packetPtr,
    tt6IpAddressPtr homeAddrPtr);

/*
 *  called by tf6MobilityHdrIncomingPacket () when the received MH type is 
 *  TM_6_MH_TYPE_HOTI.
 */
TM_PROTO_EXTERN void tf6MipHotiIncomingPacket(ttPacketPtr packetPtr);

/*
 *  called by tf6MobilityHdrIncomingPacket () when the received MH type is 
 *  TM_6_MH_TYPE_COTI.
 */
TM_PROTO_EXTERN void tf6MipCotiIncomingPacket(ttPacketPtr packetPtr);

/*
 *  called by tf6MobilityHdrIncomingPacket () when the received MH type is 
 *  TM_6_MH_TYPE_BU.
 */
TM_PROTO_EXTERN void tf6MipBuIncomingPacket(ttPacketPtr packetPtr);

/*
 * CN processing of an ICMP error message, called by tf6IcmpIncoming when 
 * the message type is ICMP Destination Unreachable message. 
 * 
 * if the dest address of the original packet is an mobile node (CN has 
 * a binding entry, Calls tf6MipGetBinding to check whether a binding exists)
 * If we receive TM_6_MIP_CN_MAX_DEST_UNREACH_ICMP ICMP error messages 
 * associated w/ the same MN within TM_6_MIP_CN_ICMP_MIONTOR_PERIOD after 
 * we've send a message via its CoA, then remove the binding entry
 */
TM_PROTO_EXTERN void tf6MipCnIcmpIncomingPacket(ttPacketPtr packetPtr);

TM_PROTO_EXTERN int tf6BuildRoutingHeader (
    tt8BitPtr               bufferPtr,
    tt8Bit                  type,
    tt6ConstIpAddressPtr    ipAddrPtr,
    tt8Bit                  numOfAddr);

/* Discard the oldest valid pair of nonce and Kcn, regenerate a new pair */
TM_PROTO_EXTERN void tf6MipNonceTimeOut(
    ttVoidPtr       timerBlockPtr,
    ttGenericUnion  userParm1,
    ttGenericUnion  userParm2);

/* generate the nonce and Kcn at the given index,
 * index is a number between [0, TM_6_MIP_MAX_NONCE - 1]
 */
TM_PROTO_EXTERN void tf6MipRegenNonceKcn(int index);

#endif /* TM_6_USE_MIP_CN */

/* Form the Anycast address as RFC2526 */
TM_PROTO_EXTERN void tf6FormAnycastAddr(
    tt6ConstIpAddressPtr    prefixPtr,
    int                     prefixLen,
    tt8Bit                  anycastId,
    tt6IpAddressPtr         anyAddrPtr);

#ifdef TM_6_USE_MIP_HA

/* Process the DHAAD REQ msg, Prepare and send a DHAAD reply message */
TM_PROTO_EXTERN void tf6MnDhaadRequestIncomingPacket(ttPacketPtr packetPtr);

/* Process the mobile prefix solicit message */
TM_PROTO_EXTERN void tf6MobilePrefixSolIncomingPacket(ttPacketPtr packetPtr);

#endif /* TM_6_USE_MIP_HA */

/* Prefix option processing, for both stationary and mobile node. */
int tf6PrefixDiscIncomingPacket(
    tt6NdPrefixInfoOptionPtr    prefOptPtr,
    ttDeviceEntryPtr            devEntryPtr,
    tt8Bit                      ipsecAuthFlag);

int tf6RdnssIncoming(
    tt6NdRdnssOptionPtr rdnssOptPtr,
    ttDeviceEntryPtr devPtr );
int tf6DnsslIncoming(
    tt6NdDnsslOptionPtr dnsslOptPtr,
    ttDeviceEntryPtr devPtr );

#ifdef TM_6_USE_DHCP
/* Free DHCPv6 memory and any running timers */
void tf6DhcpFreeTimersAndMtbs(ttDeviceEntryPtr devPtr);

/* Free memory allocated for DHCPv6 and stored on the device entry */
void tf6DhcpFreeDevMembers(ttDeviceEntryPtr devPtr);
#endif /* TM_6_USE_DHCP */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _T6PROTO_H_ */
