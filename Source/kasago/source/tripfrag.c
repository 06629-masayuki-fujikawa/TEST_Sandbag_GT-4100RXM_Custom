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
 * Description: IP fragmentation function
 * Filename: tripfrag.c
 * Author: Odile
 * Date Created: 12/05/97
 * $Source: source/tripfrag.c $
 *
 * Modification History
 * $Revision: 6.0.2.4 $
 * $Date: 2013/01/18 04:32:34JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>  /* For TM_IP_FRAGMENT, TM_IP_REASSEMBLY */
#include <trmacro.h> /* For LINT_UNUSED_HEADER */

/*
 * TM_IP_FRAGMENT defined in trsystem.h if fragmentation is needed.
 * TM_IP_REASSEMBLY defined in trsystem.h if IP reassembly is needed.
 */

#ifdef TM_USE_IPV4

#if defined(TM_IP_FRAGMENT) || defined(TM_IP_REASSEMBLY)

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/*
 * Local macros
 */

#ifdef TM_IP_FRAGMENT

/* round to a multiple of 8 bytes */
#define TM_ROUND8_SIZE                7
#define TM_ROUND8_MASK_SIZE          ~((unsigned)TM_ROUND8_SIZE)

/* IP more fragment flag in host byte order. */
#define TM_IP_HOST_MORE_FRAG          0x2000

#endif /* TM_IP_FRAGMENT */

#ifdef TM_IP_REASSEMBLY

#define TM_IPFH_NULL_PTR   (ttIpFHEntryPtr)0

/*
 * Local functions
 */
/* Aging timer */
static void tfIpFragTimer (ttVoidPtr      timerBlockPtr,
                           ttGenericUnion userParm1,
                           ttGenericUnion userParm2);

/* Remove a fragment head from the fragment head list */
static void tfIpFragHeadRemove(ttIpFHEntryPtr ipFHptr,
                               ttIpFHEntryPtr ipFHPrevPtr);

static void tfIpFragRejectPkt(ttIpFHEntryPtr ipFHPtr,
                              ttIpFHEntryPtr ipFHPrevPtr);

/*
 * Function Description
 * 0) First, we lock the fragment head list tvIpFHList.
 * 1) Then, if tvIpFHList (list of fragment head structures) is empty,
 *    we create a cyclic timer with a timeout of 1 second. The call back
 *    function is tfIpFragTimer(). If we fail to allocate a timer, we
 *    unlock the tvIpFHList,  we free the packet and return.
 * 2) If it is not empty we try and find a matching fragment head (we match
 *    on IP Id, IP source address, destination address, and protocol field)
 * 3) If it is empty or we did not find a matching fragment head, we
 *    allocate a new fragment head (structure ttIpFHEntry), and
 *    initialize the ipfTtl timeout field with 60 seconds (tvFragTtl),
 *    initialize its ipfSrcAddress, ipfDstAddress, ipfId, and
 *    ipfUlp, with the IP header values, and insert it in the
 *    tvIpFHList. If we cannot allocate a fragment head, we free the
 *    packet, free the timer (if tvIpFHList is empty), unlock tvIpFHList
 *    and return a null pointer.
 * 5) Then we subtract from the pktChainDataLength and pktLinkDataLength the
 *    IP header length and add the IP header length to the pktLinkDataPtr
 *    since fragmentation offset computation is done without any header.
 *    Then we copy the offset (after having converted it in bytes) in the
 *    dataFragBBOffset field for insertion comparisons, and the fragment
 *    flag (more or not) in dataFragMore (to check for last fragment). We
 *    also compute the end offset of our fragment and store it in
 *    dataFragEBOffset (for overlap comparisons).
 * 6) Then we find the right spot in the fragment queue using the
 *    dataFragBBOffset, and check for overlap of previous fragment
 *    and next fragment using dataFragEBOffset, trimming if necessary.
 * 7) Insert our ttPacket in the fragment queue using the
 *    pktChainNextPtr.
 * 8) If our Ip Datagram is complete, we update the first ttPacket's fields,
 *    and point to the first IP header.
 *    Then we reassemble it with ttpackets linking the end of each fragment
 *    to the beginning of the next fragment with the pktLinkNextPtr,
 *    and zeroing the pktChainNextPtr of all the fragments.
 *    Then we call tfIpFragHeadRemove() (to remove the fragment head from
 *    the fragment head list). We remove the timer if the fragment list is
 *    empty. We unlock the fragment head list, and return the first
 *    ttpacket.
 * 9) If our IP datagram is not complete, we unlock the fragment head list,
 *    and we return a null pointer.
 * Parameter       Description
 * packetPtr       A pointer to a packet chain (ttPacketPtr) (with
 *                 packetPtr->pktLinkDataPtr pointing to the IP header
 *                 in ttPacket )
 * Returns
 * Value   Meaning
 * TM_PACKET_NULL_PTR   The packet has been queued awaiting other
 *                      fragments or an error occurred.
 * packetPtr            A pointer to a new packet chain (ttPacketPtr)
 *                      (with packetPtr->pktLinkDataPtr pointing to the IP
 *                      header in ttPacket, and pktChainDataLength updated)
 */
ttPacketPtr tfIpReassemblePacket(ttPacketPtr packetPtr)
{
    ttIpFHEntryPtr  ipFHPtr;
    ttIpFHEntryPtr  ipFHPrevPtr;
    ttIpFHEntryPtr  ipFHPtr2;
    ttIpFHEntryPtr  ipFHPrevPtr2;
    ttIpFHEntryPtr  ipFHOldestTooBig;
    ttIpFHEntryPtr  ipFHBeforeOldestTooBig;
    ttIpHeaderPtr   iphPtr;        /* pointer to IP header */
    ttPacketPtr     nextPacketPtr;
    ttPacketPtr     prevPacketPtr;
    ttPacketPtr     dumpPacketPtr;
    ttSharedDataPtr pktShrDataPtr;
    tt16Bit         fragOff;
    ttPktLen        bytes;
    ttPktLen        ebytes;
    ttPktLen        overlap;
    tt8Bit          ipHdrLength;
    ttGenericUnion  timerParm1;

    fragOff = 0;
    iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
    ipFHPrevPtr = TM_IPFH_NULL_PTR;
/* LOCK list of fragment heads */
    tm_lock_wait(&tm_context(tvIpFHLock));
    ipFHPtr = tm_context(tvIpFHPtr);
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
    packetPtr->pktDeviceEntryPtr->dev4Ipv4Mib.ipIfStatsReasmReqds++;
    tm_context(tvDevIpv4Data).ipIfStatsReasmReqds++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
/* If it is null (no fragment head), create a timer */
    if (ipFHPtr == TM_IPFH_NULL_PTR)
    {
        timerParm1.genIntParm = 0; /* init to avoid compiler warning */
        tm_context(tvIpFHTimerPtr) = tfTimerAdd(tfIpFragTimer,
                                                timerParm1, /* unused */
                                                timerParm1, /* unused */
                                                tm_context(tvFragResolution),
                                                TM_TIM_AUTO);
        if (tm_context(tvIpFHTimerPtr) == TM_TMR_NULL_PTR)
        {
/* free the packet, unlock fragment head list and return if we fail to
 * allocate a timer
 */
            goto badReass;
        }
    }
    else
    {
/* Non empty list of fragment heads; try and find a matching head */
/*  while fragment head list is not empty we try and find a matching fragment
 *  head (we match on IP Id, IP source address, destination address, and
 *  protocol field)
 */
        while (ipFHPtr != TM_IPFH_NULL_PTR)
        {
            if ( (ipFHPtr->ipfId == iphPtr->iphId) &&
                 (ipFHPtr->ipfUlp == iphPtr->iphUlp) &&
                 tm_ip_match(ipFHPtr->ipfDestAddr, iphPtr->iphDestAddr) &&
                 tm_ip_match(ipFHPtr->ipfSrcAddr, iphPtr->iphSrcAddr) )
            {
/* Match found. Try and insert the packet in that fragment head list entry */
                goto processPacket;
            }
/* keep track of previous in case packet is reassembled */
            ipFHPrevPtr = ipFHPtr;
            ipFHPtr = ipFHPtr->ipfHeadNextPtr;
        }
    }
/*
 * Empty fragment head list, or we did not find a match: create a new
 * fragment head
 */
/* 
 * BUG ID 207:
 * Check that we did not reach our limit of number of IP datagrams waiting
 * to be reassembled.
 */
    if (tm_context(tvFragQSize) < tm_context(tvFragMaxQSize))
    {
        ipFHPtr = (ttIpFHEntryPtr)tm_get_raw_buffer(sizeof(ttIpFHEntry));
        if (ipFHPtr != (ttIpFHEntryPtr)0)
        {
/* 
 * zero allocated entry because code below assumes ipfPacketNextPtr, 
 * and ipfSize are initialized to 0 
 */
            tm_bzero(ipFHPtr, sizeof(ttIpFHEntry));
        }
    }
    if (ipFHPtr == TM_IPFH_NULL_PTR)
    {
/* Remove Reassembly timer, if fragment list is empty */
        if (tm_context(tvIpFHPtr) == TM_IPFH_NULL_PTR)
        {
            tm_timer_remove(tm_context(tvIpFHTimerPtr));
        }
badReass:
#ifdef TM_SNMP_MIB
/*
 * Number of failures detected by the IP re-assembly algorithm (for
 * whatever reason: timed out, errors, etc).
 */
        tm_context(tvIpData).ipReasmFails++;
#ifdef TM_USE_NG_MIB2
        packetPtr->pktDeviceEntryPtr->dev4Ipv4Mib.ipIfStatsReasmFails++;
        tm_context(tvDevIpv4Data).ipIfStatsReasmFails++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
/* UNLOCK tvIpFHPtr (lists of fragment heads) */
        tm_unlock(&tm_context(tvIpFHLock));
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
        return TM_PACKET_NULL_PTR;
    }
/*
 * Insert new fragment head in fragment head list, and initialize it.
 */
    ipFHPrevPtr = TM_IPFH_NULL_PTR;
    ipFHPtr->ipfHeadNextPtr = tm_context(tvIpFHPtr);
    tm_context(tvIpFHPtr) = ipFHPtr;
    (tm_context(tvFragQSize))++;
    tm_ip_copy(iphPtr->iphSrcAddr, ipFHPtr->ipfSrcAddr);
    tm_ip_copy(iphPtr->iphDestAddr, ipFHPtr->ipfDestAddr);
    ipFHPtr->ipfId = iphPtr->iphId;
    ipFHPtr->ipfTtl = tm_context(tvFragTtl);
    ipFHPtr->ipfUlp = iphPtr->iphUlp;
processPacket:
/* IP header length */
    ipHdrLength = packetPtr->pktIpHdrLen;
/* 
 * BUG ID 207:
 * Check that we did not reach our maximum size limit per IP datagram.
 */
    ipFHPtr->ipfSize += (   packetPtr->pktChainDataLength 
                          - tm_byte_count(ipHdrLength) );
    if (ipFHPtr->ipfSize > tm_context(tvFragEntryMaxSize))
    {
        tfFreeChainPacket(ipFHPtr->ipfPacketNextPtr, TM_SOCKET_UNLOCKED);
        ipFHPtr->ipfPacketNextPtr = (ttPacketPtr)0;
/* There is a possible DOS against our reassembly code. Given the following:
 * 1) Because we run on embedded systems with limited memory, we must limit
 *    the maximum size of incoming packets that requre reassembly
 * 2) Also because we run on embedded systems with limited memory, we must
 *    limit the number of fragmented packets we're holding on to for
 *    reassembly at any given time
 * 3) We must keep track of packets that fit in to category 1 because there
 *    may be more fragments coming that we need to free up
 * Because of these three facts, we must keep track of two sets of
 * fragmented packet - packets that we're actively trying to reassemble, and
 * packets that are too large, but we still have to track to free up any more
 * fragments that come in (Note: We don't actually hold on to the too-big
 * packets - just enough info to match up any new fragments that come in).
 * When we detect that a packet it too large to reassemble, we change its
 * status to "too big" and keep it on the list. We will allow a maximum number
 * of active reassembly packets, and a maximum number of "too big" reassembly
 * packets to be in the list at the same time.
 */
        if (ipFHPtr->ipfTooBig != TM_8BIT_YES)
        {
            ipFHPtr->ipfTooBig = TM_8BIT_YES;
/* Decrement the number of packets undergoing reassembly */
            (tm_context(tvFragQSize))--;
            if (tm_context(tvFragFailedQSize) >= tm_context(tvFragFailedMaxQSize))
            {
                ipFHPtr2 = tm_context(tvIpFHPtr);
                ipFHPrevPtr2 = TM_IPFH_NULL_PTR;
                ipFHOldestTooBig = TM_IPFH_NULL_PTR;
                ipFHBeforeOldestTooBig = TM_IPFH_NULL_PTR;
                while (ipFHPtr2 != TM_IPFH_NULL_PTR)
                {
                    if (ipFHPtr2->ipfTooBig == TM_8BIT_YES)
                    {
                        ipFHOldestTooBig = ipFHPtr2;
                        ipFHBeforeOldestTooBig = ipFHPrevPtr2;
                    }
                    ipFHPrevPtr2 = ipFHPtr2;
                    ipFHPtr2 = ipFHPtr2->ipfHeadNextPtr;
                }
/* Free the oldest "too big" entry */
                tfIpFragRejectPkt(ipFHOldestTooBig, ipFHBeforeOldestTooBig);
            }
/* Increment the number of "too big" entries */
            tm_context(tvFragFailedQSize)++;
        }
        goto badReass;
    }
/*
 * Point to data. Fragment offsets are relative to data portion of original
 * IP datagram
 */
    tm_pkt_ulp_hdr(packetPtr, ipHdrLength);
/* Convert 8-byte Offset in original datagram to byte offset */
    tm_ntohs(iphPtr->iphFlagsFragOff, fragOff);
    bytes = (ttPktLen)(((ttPktLen)(fragOff&TM_IP_HFRAG_OFFSET)) << 3);
/* End byte offset of our fragment in original datagram */
    ebytes = bytes + packetPtr->pktChainDataLength;
    pktShrDataPtr = packetPtr->pktSharedDataPtr;
/* Keep copy of byte offset for later insertion comparisons */
    pktShrDataPtr->dataFragBBOffset = bytes;
/* Keep copy of End byte offset of the fragment for overlap comparisons */
    pktShrDataPtr->dataFragEBOffset = ebytes;
/* Keep track of last fragment */
    if ( tm_16bit_one_bit_set(iphPtr->iphFlagsFragOff, TM_IP_MORE_FRAG) )
    {
        pktShrDataPtr->dataFragMore = TM_8BIT_YES;
    }
/*
 * Loop through all the fragments already queued, to find the
 * right spot to insert this fragment (between prevPacketPtr
 * and nextPacketPtr)
 */
    nextPacketPtr = ipFHPtr->ipfPacketNextPtr;
    prevPacketPtr = TM_PACKET_NULL_PTR;
    while (nextPacketPtr != TM_PACKET_NULL_PTR)
    {
        if (nextPacketPtr->pktFragBBOffset > bytes)
        {
            break;
        }
        prevPacketPtr = nextPacketPtr;
        nextPacketPtr = nextPacketPtr->pktChainNextPtr;
    }
/* Check for overlap of data between previous fragment and our fragment */
    if (prevPacketPtr != TM_PACKET_NULL_PTR)
    {
        if (prevPacketPtr->pktFragEBOffset > bytes)
        {
            overlap = (ttPktLen)(prevPacketPtr->pktFragEBOffset - bytes);
            if (overlap >= packetPtr->pktChainDataLength)
/* All of our fragment data are already included in previous fragment */
            {
                goto badReass;
            }
/* Head trim our fragment by the amount of overlapping data */
#if defined(TM_ERROR_CHECKING) || defined(TM_USE_DRV_SCAT_RECV) || defined(TM_USE_IPSEC)
            if (packetPtr->pktChainDataLength == packetPtr->pktLinkDataLength)
#endif /* TM_ERROR_CHECKING || TM_USE_DRV_SCAT_RECV || TM_USE_IPSEC */
            {
/* Non scattered data case */
                tm_pkt_ulp_hdr(packetPtr, tm_packed_byte_count(overlap));
            }
#if defined(TM_ERROR_CHECKING) || defined(TM_USE_DRV_SCAT_RECV) || defined(TM_USE_IPSEC)
            else
            {
/* Scattered data case */
                tfPacketHeadTrim(packetPtr, overlap);
            }
#endif /* TM_ERROR_CHECKING || TM_USE_DRV_SCAT_RECV || TM_USE_IPSEC */
            packetPtr->pktFragBBOffset += overlap;
        }
    }
/* Check for overlap of data between our fragment and next fragment(s) */
    while ( (nextPacketPtr != TM_PACKET_NULL_PTR) &&
            (ebytes > nextPacketPtr->pktFragBBOffset) )
    {
        overlap = (ttPktLen)(ebytes - nextPacketPtr->pktFragBBOffset);
        if (overlap < nextPacketPtr->pktChainDataLength)
        {
/* Head trim next fragment by the amount of overlapping data */
#if defined(TM_ERROR_CHECKING) || defined(TM_USE_DRV_SCAT_RECV) || defined(TM_USE_IPSEC)
            if (nextPacketPtr->pktChainDataLength
                            == nextPacketPtr->pktLinkDataLength)
#endif /* TM_ERROR_CHECKING || TM_USE_DRV_SCAT_RECV || TM_USE_IPSEC */
            {
/* Non scattered data */
                tm_pkt_ulp_hdr(nextPacketPtr, tm_packed_byte_count(overlap));
            }
#if defined(TM_ERROR_CHECKING) || defined(TM_USE_DRV_SCAT_RECV) || defined(TM_USE_IPSEC)
            else
            {
/* Scattered data */
#if !defined(TM_USE_DRV_SCAT_RECV) && !defined(TM_USE_IPSEC)
                tfKernelError("tfIpReassemblePacket",
                              "Incoming scattered data");
#endif /* !TM_USE_DRV_SCAT_RECV  && !TM_USE_IPSEC */
                tfPacketHeadTrim(nextPacketPtr, overlap);
            }
#endif /* TM_ERROR_CHECKING || TM_USE_DRV_SCAT_RECV || TM_USE_IPSEC */
            nextPacketPtr->pktFragBBOffset += overlap;
            break;
        }
/* overlap covers the next packet: remove it from the queue */
        dumpPacketPtr = nextPacketPtr;
        if (prevPacketPtr != TM_PACKET_NULL_PTR)
        {
            prevPacketPtr->pktChainNextPtr =
                                    nextPacketPtr->pktChainNextPtr;
        }
        else
        {
            ipFHPtr->ipfPacketNextPtr = nextPacketPtr->pktChainNextPtr;
        }
/* Point to next fragment */
        nextPacketPtr = nextPacketPtr->pktChainNextPtr;
/* Free the removed packet */
        tfFreePacket(dumpPacketPtr, TM_SOCKET_UNLOCKED);
    }
/* We have found the right spot. Insert the packet */
    packetPtr->pktChainNextPtr = nextPacketPtr;
    if (prevPacketPtr == TM_PACKET_NULL_PTR)
    {
/* at head of queue */
        ipFHPtr->ipfPacketNextPtr = packetPtr;
    }
    else
    {
        prevPacketPtr->pktChainNextPtr = packetPtr;
    }
/* Check if packet is reassembled */
    bytes = TM_32BIT_ZERO;
    nextPacketPtr = ipFHPtr->ipfPacketNextPtr;
    prevPacketPtr = TM_PACKET_NULL_PTR;
#ifdef TM_ERROR_CHECKING
    if (nextPacketPtr == TM_PACKET_NULL_PTR)
    {
/* UNLOCK Fragment list/recycle list */
        tm_unlock(&tm_context(tvIpFHLock));
        tfKernelError("tfIpReassemblePacket",
                      "Empty list after packet insertion");
        tm_thread_stop;
    }
#endif /* TM_ERROR_CHECKING */
/* Check for no hole in the chain of packets */
    do
    {
        if (nextPacketPtr->pktFragBBOffset != bytes)
        {
            packetPtr = TM_PACKET_NULL_PTR;
            goto outReass;
        }
        bytes += nextPacketPtr->pktChainDataLength;
        prevPacketPtr = nextPacketPtr;
        nextPacketPtr = nextPacketPtr->pktChainNextPtr;
    }
    while (nextPacketPtr != TM_PACKET_NULL_PTR);
/* Check that the last packet is the last fragment */
    if (prevPacketPtr->pktFragMore != TM_8BIT_ZERO)
    {
        packetPtr = TM_PACKET_NULL_PTR;
        goto outReass;
    }
/* Packet fully reassembled, Update first link packet. Point to IP Header */
    packetPtr = ipFHPtr->ipfPacketNextPtr;
    ipHdrLength = packetPtr->pktIpHdrLen;
    packetPtr->pktChainDataLength = bytes + tm_byte_count(ipHdrLength);
    packetPtr->pktLinkDataLength += tm_byte_count(ipHdrLength);
    packetPtr->pktLinkDataPtr -= ipHdrLength;
/*
 * We do not update IP length and checksum in IP header 
 * since we do not need that information in the ULP
 */
/* link all the links together */
    nextPacketPtr = packetPtr;
    while (nextPacketPtr->pktChainNextPtr != TM_PACKET_NULL_PTR)
    {
        prevPacketPtr = nextPacketPtr;
/* first link of next fragment */
        nextPacketPtr = nextPacketPtr->pktChainNextPtr;
#if defined(TM_ERROR_CHECKING) || defined(TM_USE_DRV_SCAT_RECV) || defined(TM_USE_IPSEC)
/*
 * If scattered data first link had no data in (completely head trimmed).
 * (Only possible if we support scattered data in. If we don't support
 * incoming scattered data, it is an error.)
 */
        while (    (nextPacketPtr != TM_PACKET_NULL_PTR)
                && (nextPacketPtr->pktLinkDataLength == 0) )
        {
#if !defined(TM_USE_DRV_SCAT_RECV) && !defined(TM_USE_IPSEC)
#ifdef TM_LOG_IPV4_NOTICE
            tfEnhancedLogWrite(TM_LOG_MODULE_IPV4,
                               TM_LOG_LEVEL_NOTICE,
                               "tfIpReassemblePacket: "
                                    "Incoming scattered data (empty link)");
#endif /* TM_LOG_IPV4_NOTICE */
#endif /* !TM_USE_DRV_SCAT_RECV  && !TM_USE_IPSEC */
/* In case next fragment links had been headtrimmed */
            dumpPacketPtr = nextPacketPtr;
            nextPacketPtr = (ttPacketPtr)nextPacketPtr->pktLinkNextPtr;
            if (nextPacketPtr != TM_PACKET_NULL_PTR)
            {
                nextPacketPtr->pktChainNextPtr =
                                              dumpPacketPtr->pktChainNextPtr;
#if defined(TM_USE_DRV_SCAT_RECV) || defined(TM_USE_IPSEC)
/* Initialize pktLinkExtraCount on the new first link */
                nextPacketPtr->pktLinkExtraCount = 
                                          dumpPacketPtr->pktLinkExtraCount - 1;
                tm_assert(tfIpReassemblePacket, 
                          nextPacketPtr->pktLinkExtraCount >= 0);
#endif /* TM_USE_DRV_SCAT_RECV || TM_USE_IPSEC */
            }
            else
            {
/* Empty fragment: should not happen */
#ifdef TM_ERROR_CHECKING
#ifdef TM_LOG_IPV4_NOTICE
                tfEnhancedLogWrite(TM_LOG_MODULE_IPV4,
                                   TM_LOG_LEVEL_NOTICE,
                                   "tfIpReassemblePacket: Empty fragment");
#endif /* TM_LOG_IPV4_NOTICE */
#endif /* TM_ERROR_CHECKING */
                nextPacketPtr = dumpPacketPtr->pktChainNextPtr;
            }
            tfFreeSharedBuffer(dumpPacketPtr, TM_SOCKET_UNLOCKED);
        }
        if (nextPacketPtr == TM_PACKET_NULL_PTR)
/* Possible only if we found an empty fragment */
        {
            break;
        }
#endif /* TM_ERROR_CHECKING || TM_USE_DRV_SCAT_RECV || TM_USE_IPSEC */
/* Zero the chain link */
        prevPacketPtr->pktChainNextPtr = TM_PACKET_NULL_PTR;
#if defined(TM_ERROR_CHECKING) || defined(TM_USE_DRV_SCAT_RECV) || defined(TM_USE_IPSEC)
        while (prevPacketPtr->pktLinkNextPtr != (ttVoidPtr)0)
        {
#if !defined(TM_USE_DRV_SCAT_RECV) && !defined(TM_USE_IPSEC)
#ifdef TM_LOG_IPV4_NOTICE
/* Scattered data. Last link of previous fragment */
            tfEnhancedLogWrite(TM_LOG_MODULE_IPV4,
                               TM_LOG_LEVEL_NOTICE,
                               "tfIpReassemblePacket: "
                                    "Incoming scattered data");
#endif /* TM_LOG_IPV4_NOTICE */
#endif /* !TM_USE_DRV_SCAT_RECV  && !TM_USE_IPSEC */
            prevPacketPtr = (ttPacketPtr)prevPacketPtr->pktLinkNextPtr;
        }
#endif /* TM_ERROR_CHECKING || TM_USE_DRV_SCAT_RECV || TM_USE_IPSEC */
/* Glue last link of previous fragment with first link of next fragment */
        prevPacketPtr->pktLinkNextPtr = (ttVoidPtr)nextPacketPtr;
/*
 * Count number of extra links. Needed in case the packet is being
 * sent back out by upper layer (i.e ICMP echo reply) through the device
 * without being fragmented.
 */
#if defined(TM_USE_DRV_SCAT_RECV) || defined(TM_USE_IPSEC)
        packetPtr->pktLinkExtraCount += nextPacketPtr->pktLinkExtraCount + 1;
#else /* !TM_USE_DRV_SCAT_RECV && !TM_USE_IPSEC */
        packetPtr->pktLinkExtraCount++;
#endif /* !TM_USE_DRV_SCAT_RECV && !TM_USE_IPSEC */
    }
/* Dequeue fragment head */
    tfIpFragHeadRemove(ipFHPtr, ipFHPrevPtr);
#ifdef TM_DEV_RECV_OFFLOAD
    if (packetPtr->pktDevOffloadPtr != (ttVoidPtr)0)
    {
/* Cannot use offloaded recv checksum if we have reassembled the packet */
        packetPtr->pktDevOffloadPtr = (ttVoidPtr)0;
    }
#endif /* TM_DEV_RECV_OFFLOAD */

/* 
 * if reassembling success, clear the first datagram's More flag bit
 * and the fragment offset. IPsec will test if these fields are zero.
 */
     tm_16bit_clr_bit(
            ((ttIpHeaderPtr)(packetPtr->pktLinkDataPtr))->iphFlagsFragOff,
            TM_IP_MORE_FRAG_OFFSET);

outReass:
/* UNLOCK Fragment list lock */
    tm_unlock(&tm_context(tvIpFHLock));

    return packetPtr;
}


/*
 * Function Description
 * tfIpFragRejectPkt is called whenever we need to reject a packet we're
 * trying to reassemble (either because it's too large, or it timed out.
 * Parameters
 * Parameter       Description
 * ipFHPtr         Entry in reassembly queue to reject
 * ipFHPrevPtr     Entry in reassembly queue immediately before rejected entry
 * Returns
 * Value   Meaning
 * No return value
*/
static void tfIpFragRejectPkt(ttIpFHEntryPtr ipFHPtr,
                              ttIpFHEntryPtr ipFHPrevPtr)
{
    ttPacketPtr    packetPtr;
    ttPacketPtr    icmpPacketPtr;
/*
 * BUG ID 43: Make sure IP fragment packet points to the IP header, before
 * we send an ICMP error message for time exceeded in reassembly
 */
    ttPktLen       ipHdrLen;

    packetPtr = ipFHPtr->ipfPacketNextPtr;
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
#ifdef TM_SNMP_MIB
/*
 * Number of failures detected by the IP re-assembly algorithm (for
 * whatever reason: timed out, errors, etc).
 */
        tm_context(tvIpData).ipReasmFails++;
#ifdef TM_USE_NG_MIB2
        packetPtr->pktDeviceEntryPtr->dev4Ipv4Mib.ipIfStatsReasmFails++;
        tm_context(tvDevIpv4Data).ipIfStatsReasmFails++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
        if (packetPtr->pktFragBBOffset == 0)
        {
/* If first fragment in, send an ICMP error message */
            icmpPacketPtr = packetPtr;
/* Point to next fragment, (tfIcmpErrPacket will free the first fragment) */
            packetPtr = packetPtr->pktChainNextPtr;
/*
 * BUG ID 43: Make sure IP fragment packet points to the IP header, before
 * we send an ICMP error message for time exceeded in reassembly
 */
            ipHdrLen = (ttPktLen)icmpPacketPtr->pktIpHdrLen;
            tm_pkt_llp_hdr(icmpPacketPtr, ipHdrLen);
/* End of BUG ID 43 */
#if defined(TM_USE_VIPER) || defined(TM_USE_VIPER_TEST)
#ifdef TM_USE_IP_REASSEMBLY_TASK
            if (icmpPacketPtr->pktIrlEntryPtr != (ttVoidPtr)0)
/* Exception packet */
            {
                tfIkeExceptIcmpErr(icmpPacketPtr, TM_32BIT_ZERO,
                                   TM_IP_ZERO,
                                   TM_ICMP_TYPE_TIME_XCEED,
                                   TM_ICMP_CODE_XCEED_REASS);
            }
            else
#endif /* TM_USE_IP_REASSEMBLY_TASK */
#endif /* (TM_USE_VIPER || TM_USE_VIPER_TEST) */
            {
                tfIcmpErrPacket(icmpPacketPtr,
                    TM_ICMP_TYPE_TIME_XCEED,
                    TM_ICMP_CODE_XCEED_REASS,
                    TM_32BIT_ZERO,
                    TM_DEV_NULL_PTR,
                    TM_IP_ZERO);
            }
        }
/* Go down the chain and free each linked entry */
        tfFreeChainPacket(packetPtr, TM_SOCKET_UNLOCKED);
    }
/* Remove from fragment list, and put into recycle list */
    tfIpFragHeadRemove(ipFHPtr, ipFHPrevPtr);

    return;
}
 

/*
 * Function Description
 * tfIpFragTimer scans the tvIpFHList to check for expired fragments.
 * It will first lock (with a cannot wait flag) the tvIpFHList to do
 * the scan. If it obtains the lock, for each tfIpFragEntry, it will
 * decrease the ipfTtl field and if it reaches 0, send
 * an ICMP time expired message for the initial fragment, free all the
 * fragments in the list, and remove and free the tfIpFHEntry. If the
 * fragment head list is empty, it will also free the timer. It will then
 * unlock the tvIpFHList.
 * Parameters
 * Parameter       Description
 * argPtr           Unused
 * Returns
 * Value   Meaning
 * No return value
*/
static void tfIpFragTimer (ttVoidPtr      timerBlockPtr,
                           ttGenericUnion userParm1,
                           ttGenericUnion userParm2)
{
    ttIpFHEntryPtr ipFHPtr;
    ttIpFHEntryPtr ipFHNextPtr;
    ttIpFHEntryPtr ipFHPrevPtr;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm1);
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);
    
/* LOCK tvIpFHList (list of fragment heads) */
    tm_if_call_lock_nowait(&tm_context(tvIpFHLock))
    {
        ipFHPtr = tm_context(tvIpFHPtr);
        if (ipFHPtr != TM_IPFH_NULL_PTR)
        {
            ipFHPrevPtr = TM_IPFH_NULL_PTR;
/* Walk the list of fragment heads */
            while (ipFHPtr != TM_IPFH_NULL_PTR)
            {
                if (ipFHPtr->ipfTtl == 1)
/* If timer expired on fragment */
                {
/* Save next pointer, since it will be changed by tfIpFragRejectPkt */
                    ipFHNextPtr = ipFHPtr->ipfHeadNextPtr;
                    tfIpFragRejectPkt(ipFHPtr, ipFHPrevPtr);
/* Process next entry */
                    ipFHPtr = ipFHNextPtr;
                }
                else
                {
/* just decrease TTL */
                    ipFHPtr->ipfTtl--;
                    ipFHPrevPtr = ipFHPtr;
/* Process next entry */
                    ipFHPtr = ipFHPtr->ipfHeadNextPtr;
                }
            }
        }
        tm_call_unlock(&tm_context(tvIpFHLock));
    }
}

/*
 * Remove a fragment head from the fragment head list and
 * insert into the recycle list
 */
static void tfIpFragHeadRemove(ttIpFHEntryPtr ipFHPtr,
                               ttIpFHEntryPtr ipFHPrevPtr)
{
    if (ipFHPtr->ipfTooBig == TM_8BIT_YES)
    {
        (tm_context(tvFragFailedQSize))--;
    }
    else
    {
        (tm_context(tvFragQSize))--;
    }
    if (ipFHPrevPtr != TM_IPFH_NULL_PTR)
    {
       ipFHPrevPtr->ipfHeadNextPtr = ipFHPtr->ipfHeadNextPtr;
    }
    else
    {
       tm_context(tvIpFHPtr) = ipFHPtr->ipfHeadNextPtr;
    }
/* Put into recycle list */
    tm_free_raw_buffer(ipFHPtr);
/* Remove timer if fragment list is empty */
    if (tm_context(tvIpFHPtr) == TM_IPFH_NULL_PTR)
    {
        tm_timer_remove(tm_context(tvIpFHTimerPtr));
    }
}

#ifdef TM_USE_STOP_TRECK
void tfIpFragDeInit(
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                )
{
    while (tm_context_var(tvIpFHPtr) != TM_IPFH_NULL_PTR)
    {
        tfIpFragHeadRemove(tm_context_var(tvIpFHPtr),
                           TM_IPFH_NULL_PTR);
    }
}
#endif /* TM_USE_STOP_TRECK */
#endif /* TM_IP_REASSEMBLY */

#ifdef TM_IP_FRAGMENT
/*
 * tfIpFragmentPacket function description
 *  1. Save original IP datagram pointers, original packet pointers, and
 *     original pktFlags in local variables, since these will be used
 *     in a loop.
 *     Compute totalDataLength without Ip header, and maximum fragment size
 *     with and without IP header allowed in each fragment.
 *     Also save original fragment offset, and original more fragment
 *     flag (if original IP datagram is being forwarded and was itself a
 *     fragment, these could be non zero).
 *  2. In a loop until we reach the end of the original packet,
 *     if last fragment shorter than the other fragments, re-compute
 *     the length of the fragment (with and without IP header),
 *     for each fragment we compute the length of the current fragment,
 *     we allocate a ttPacket/ttSharedData, copy the original pktFlags,
 *     pktDeviceEntryPtr and dataRtePtr, and dataSpecDestIpAddress (the
 *     later for ARP resolve). If we bypass the ethernet link layer and the
 *     device is a LAN device, we copy the original link layer header. If
 *     we do not bypass the ethernet link layer and the device is a LAN
 *     device, and the ethernet destination address is known, we copy it in
 *     the dataEthernetAddress field. We copy the IP header including
 *     options, and then Update the IP length, the fragment field and the
 *     checksum, in the fragment packet. If the fragment is the last
 *     packet to send, and the original IP datagram was a UDP or ICMP
 *     buffer (TM_BUF_USER_SEND, set that last fragment dataFlags
 *     with TM_BUF_USER_SEND, copy dataSocketEntryPtr, and
 *     dataUserDataLength, and reset those in the original packet shared
 *     data. We do this for flow control, so that the UDP, or ICMP send
 *     queue only gets updated when the last fragment is freed by the
 *     driver, and not when we free the original packet later in this
 *     routine. We then copy the data from the original packet into the
 *     fragment packet going though the original packet links if necessary.
 *     We update all data pointers as we copy. We then send the fragment
 *     calling the link layer send routine or link layer resolve function.
 *     If we call the link layer resolve function, we check whether the
 *     ethernet address has been resolved for the next fragment.
 *  3. We free the original packet.
 *
 * Parameter       Description
 *
 * packetPtr       A pointer to a packet chain (ttPacketPtr) (with
 *                 packetPtr->pktLinkDataPtr pointing to the IP header
 *                 in ttPacket )
 * devMtu          IP MTU for each fragment
 *
 * Returns
 * Value           Meaning
 * TM_ENOERROR     packet was fragmented successfully
 * TM_ENOBUFS      not enough memory to fragment
 * other           As returned by device send /ethernet send/ arp resolve.
 */

/* Define compile options. */
#ifdef TM_LINT
LINT_UNACCESS_SYM_BEGIN(fragFlagsFragOff)
#endif /* TM_LINT */
int tfIpFragmentPacket (ttPacketPtr origPacketPtr, ttPktLen devMtu)
{
    ttPacketPtr         fragPacketPtr;
    ttPacketPtr         origLinkPacketPtr;
    ttPacketPtr         prevFragPacketPtr;
    ttPacketPtr         firstFragPacketPtr;
    ttIpHeaderPtr       fragIpHdrPtr;
    ttIpHeaderPtr       origIpHdrPtr;
    ttSharedDataPtr     fragSharedDataPtr;
    ttSharedDataPtr     origSharedDataPtr;
    ttRteEntryPtr       origRtePtr;
    tt8BitPtr           etherSrcOrigPtr;
    ttDeviceEntryPtr    origDeviceEntryPtr;
#ifdef TM_BYPASS_ETHER_LL
    tt8BitPtr           fragLinkLayerHdrPtr;
    tt8BitPtr           origLinkLayerHdrPtr;
#else /* TM_BYPASS_ETHER_LL */
    tt8BitPtr           etherSrcFragPtr;
    ttLinkLayerEntryPtr linkLayerProtocolPtr;
#endif /* TM_BYPASS_ETHER_LL */
#ifdef TM_IP_FRAGMENT_NO_COPY
    ttPacketPtr         fragHeadPacketPtr;
#else /* !TM_IP_FRAGMENT_NO_COPY */
    tt8BitPtr           fragDataPtr;
#endif /* !TM_IP_FRAGMENT_NO_COPY */
    tt8BitPtr           origLinkDataPtr;
    ttPktLen            ipHdrLen;
    ttPktLen            totalDataLength;
    ttPktLen            origLinkDataLength;
    ttPktLen            fragMaxSize;
    ttPktLen            fragOffset; /* in bytes */
    ttPktLen            fragDataLength; /* length of current fragment */
/* amount of data to copy in one fragment */
    ttPktLen            dataLength;
/* amount of data to copy in one link of a fragment */
    ttPktLen            linkLength;
    int                 errorCode;
    int                 isLan;
/* length of current fragment including IP header */
    tt16Bit             fragIpDataLength;
/* IP header flags frag offset field */
    tt16Bit             fragFlagsFragOff;
    tt16Bit             origPktFlags;
    tt16Bit             origFragOffset;
    tt16Bit             origFragFlags;
#ifdef TM_BYPASS_ETHER_LL
    tt8Bit              llHdrLength;
#endif /* TM_BYPASS_ETHER_LL */
    tt8Bit              resolveFunctionCalled;
    tt8Bit              needArpResolution;

    errorCode = TM_ENOERROR;
    resolveFunctionCalled = TM_8BIT_ZERO;
    prevFragPacketPtr = TM_PACKET_NULL_PTR;
    firstFragPacketPtr = TM_PACKET_NULL_PTR;
/*
 * Save copies to all original pointers since these will be used in a loop
 */
/* Original device entry Pointer */
    origDeviceEntryPtr = origPacketPtr->pktDeviceEntryPtr;
/* Orignal shared data pointer */
    origSharedDataPtr = origPacketPtr->pktSharedDataPtr;
/* Original Routing entry pointer */
    origRtePtr = origSharedDataPtr->dataRtePtr;
/* Original IP header pointer */
    origIpHdrPtr = (ttIpHeaderPtr)origPacketPtr->pktLinkDataPtr;
/* Is device LAN ? */
    isLan = tm_4_ll_is_lan(origDeviceEntryPtr);
/* 
 * Original ethernet source address pointer, llHdrLength, 
 * origLinkLayerHdrPtr initializations should be inside of if (isLan).
 * Removed if (isLan) to avoid a compiler warning.
 */
    {
#ifdef TM_BYPASS_ETHER_LL
/* Link layer header length */
        llHdrLength = origDeviceEntryPtr->devLinkHeaderLength;
/* Original link layer header */
        origLinkLayerHdrPtr = origPacketPtr->pktLinkDataPtr - llHdrLength;
/* Original ethernet source address pointer */
        etherSrcOrigPtr = (tt8BitPtr)((ttEtherHeaderPtr)origLinkLayerHdrPtr)->ethSrcAddr;
#else /* !TM_BYPASS_ETHER_LL */
/* Original ethernet source address pointer */
        etherSrcOrigPtr = (tt8BitPtr)&origSharedDataPtr->dataEthernetAddress[0];
#endif /* TM_BYPASS_ETHER_LL */
    }
/* End of initialization that should be inside if (isLan) */
/* IP header length */
    ipHdrLen = (ttPktLen)origPacketPtr->pktIpHdrLen;
/* Total data length (without the IP header) */
    totalDataLength =   origPacketPtr->pktChainDataLength
                      - tm_byte_count(ipHdrLen);
/* Packet flags in original packet */
    origPktFlags = origPacketPtr->pktFlags;
    if (    (!isLan)
         || (origPktFlags & TM_ETHERNET_ADDRESS_FLAG) )
    {
        needArpResolution = TM_8BIT_NO;
    }
    else
    {
        needArpResolution = TM_8BIT_YES;
    }
/*
 * If packet is being forwarded, and the IP packet is itself a fragment
 * into an IP packet, we have to add its offset to the offset we
 * are going to compute for each fragment. We save this into flagsFragOff,
 * so that we do not have to recompute it in the loop. We have to put it in
 * host byte order to be able to add. Also save the original IP more
 * fragment flag for the same reason.
 */
    tm_ntohs(origIpHdrPtr->iphFlagsFragOff, origFragFlags);
#ifdef TM_LINT
LINT_UNINIT_SYM_BEGIN(origFragFlags)
#endif /* TM_LINT */
    origFragOffset = (tt16Bit)(origFragFlags & ~TM_IP_HOST_MORE_FRAG);
#ifdef TM_LINT
LINT_UNINIT_SYM_END(origFragFlags)
#endif /* TM_LINT */
    origFragFlags = (tt16Bit)(origFragFlags & TM_IP_HOST_MORE_FRAG);

/*
 * Compute the maximum fragment data size (without IP header).
 * Fragment size without IP header should end on an 8 byte (or 64-bit)
 * boundary. (RFC 791).
 */
    fragMaxSize = (ttPktLen) (   (devMtu - tm_byte_count(ipHdrLen))
                               & TM_ROUND8_MASK_SIZE);

/* origLinkPacketPtr used to go though the links of original packet */
    origLinkPacketPtr = origPacketPtr;
/* origLinkDataPtr used to go through data of original packet */
    origLinkDataPtr = ((tt8BitPtr)origIpHdrPtr) + (int)ipHdrLen;
/*
 * origLinkDataLength, datalength of original link (updated as we go
 * through the links (if any) of the original packet)
 */
    origLinkDataLength =   origLinkPacketPtr->pktLinkDataLength
                         - tm_byte_count(ipHdrLen);

/*
 * Fragment data offset in bytes of the fragment we are sending into
 * original IP datagram
 */
    fragOffset = (ttPktLen)0;
/* Data length (without IP header) of a fragment. */
    fragDataLength = fragMaxSize;
/* Total IP length of the fragment (including IP header) */
    fragIpDataLength = (tt16Bit)(tm_byte_count(ipHdrLen) + fragMaxSize);

#ifdef TM_IP_FRAGMENT_NO_COPY
/* Not necessary. Just for compiler warning */
    fragSharedDataPtr = (ttSharedDataPtr)0;
/*
 * Loop through the original packet until we have sent all the fragments
 */
    do
    {
#ifdef TM_ERROR_CHECKING
        if (origLinkPacketPtr == (ttPacketPtr)0)
        {
            tfKernelError(
                "tfIpFragmentPacket",
                "pktChainDataLength doesn't match sum of " \
                "pktLinkDataLength values");
        }
#endif /* TM_ERROR_CHECKING */

/* compute data length (without IP header) of this fragment */
        if (totalDataLength < fragMaxSize)
        {
/* Last fragment shorter than other fragments */
            fragDataLength  = totalDataLength;
/* Total IP length of the shorter next fragment (including IP header) */
            fragIpDataLength = (tt16Bit)(   tm_byte_count(ipHdrLen)
                                          + fragDataLength );
        }
/*
 * Update total data length. When it reaches zero, we are sending our
 * last fragment.
 */
        totalDataLength -= fragDataLength;
/* 
 * Duplicate the shared buffer. pktLinkDataLength, and pktChainDatalength 
 * updated below if necessary.
 */
        fragPacketPtr = tfDuplicateSharedBuffer(origLinkPacketPtr);
        if (fragPacketPtr == (ttPacketPtr)0)
        {
            errorCode = TM_ENOBUFS;
            break;
        }
        if (fragOffset == (ttPktLen)0)
/* If we are sending the first packet, point to the original IP header */
        {
            fragHeadPacketPtr = fragPacketPtr;
            fragHeadPacketPtr->pktLinkExtraCount = 0;
            fragIpHdrPtr = (ttIpHeaderPtr)fragHeadPacketPtr->pktLinkDataPtr;
            if (origLinkPacketPtr->pktLinkDataLength
                > (ttPktLen)fragIpDataLength)
            {
/* If fragment is not scattered. */
                fragHeadPacketPtr->pktLinkDataLength = fragIpDataLength;
            }
        }
        else
/* 
 * else we are not sending the first packet, we need to make sure that
 * the duplcated ttPacket points to the data that has not been sent yet, and
 * we need to allocate a link for the IP header
 */ 
        {
/*
 * Make sure that the duplicate shared buffer points to the data that has not
 * been sent yet.
 */
            fragPacketPtr->pktLinkDataPtr = origLinkDataPtr;
            fragPacketPtr->pktLinkDataLength = origLinkDataLength;
/* Allocate a link for the IP header */
            fragHeadPacketPtr = tfGetSharedBuffer( TM_LL_MAX_HDR_LEN, 
                                                   tm_byte_count(ipHdrLen), 
                                                   TM_16BIT_ZERO );
            if (fragHeadPacketPtr == TM_PACKET_NULL_PTR)
            {
                tfFreePacket(fragPacketPtr, TM_SOCKET_UNLOCKED);
                errorCode = TM_ENOBUFS;
                break;
            }
            fragIpHdrPtr = (ttIpHeaderPtr)fragHeadPacketPtr->pktLinkDataPtr;
            fragHeadPacketPtr->pktLinkNextPtr = (ttVoidPtr)fragPacketPtr;
            fragHeadPacketPtr->pktLinkExtraCount = 1;
            fragSharedDataPtr = fragHeadPacketPtr->pktSharedDataPtr;
/*
 * Copy relevant fields from original packet: pktFlags, pktDeviceEntryPtr,
 * dataRtePtr, dataSpecDestIpAddress (for ARP).
 */
            fragHeadPacketPtr->pktFlags = origPktFlags;
            fragHeadPacketPtr->pktDeviceEntryPtr = origDeviceEntryPtr;
            fragSharedDataPtr->dataRtePtr = origRtePtr;
#ifdef TM_USE_VCHAN
            fragHeadPacketPtr->pktMhomeIndex = origPacketPtr->pktMhomeIndex;
#endif /* TM_USE_VCHAN */

/*
 * Only needed for LAN device, when TM_ETHERNET_ADDRESS_FLAG is not set
 * (i.e we need to go through ARP).
 */
            tm_ip_copy( origSharedDataPtr->dataSpecDestIpAddress,
                        fragSharedDataPtr->dataSpecDestIpAddress );
/*
 * Copy the IP header including all options if this is the first fragment.
 * For additional fragment, we need to copy only the options that have
 * the copy bit set. We just go ahead and copy all the options in
 * successive fragments as well to reduce code size, since we know that
 * the options that we support need copying anyways.
 */
#ifdef TM_BYPASS_ETHER_LL
            if (isLan)
            {
                fragLinkLayerHdrPtr =    ((tt8BitPtr)fragIpHdrPtr)
                                       - (int)llHdrLength;
/*
 * If we bypass the Ethernet Link layer, we also need to copy the
 * ethernet header. Copy both link layer header and IP header with options.
 */
                tm_bcopy( origLinkLayerHdrPtr,
                          fragLinkLayerHdrPtr,
                          llHdrLength + ipHdrLen );
            }
            else
            {
                tm_bcopy( origIpHdrPtr,
                          fragIpHdrPtr,
                          ipHdrLen );
            }
#else /* ! TM_BYPASS_ETHER_LL */
/*
 * If we do not bypass the Ethernet link layer, we do not have a link
 * layer built yet. So we only need to copy the IP header with options.
 */
            tm_bcopy( origIpHdrPtr,
                      fragIpHdrPtr,
                      ipHdrLen);
/*
 * If we had already found our ethernet address, copy it from the
 * original shared data structure.
 */
            if (isLan)
            {
                etherSrcFragPtr = &fragSharedDataPtr->dataEthernetAddress[0];
                if (origPktFlags & TM_ETHERNET_ADDRESS_FLAG)
                {
                    tm_ether_copy(etherSrcOrigPtr, etherSrcFragPtr);
                }
            }
#endif /* TM_BYPASS_ETHER_LL */
        }
/* Update length of the fragment packet */
        tm_htons(fragIpDataLength, fragIpHdrPtr->iphTotalLength);
        fragHeadPacketPtr->pktChainDataLength = fragIpDataLength;
/* Update flagsFragOff of this fragment packet */
        linkLength = fragOffset >> 3; /* keep it as ttPktLen while shifting*/
        fragFlagsFragOff =  (tt16Bit) ( ((tt16Bit)linkLength + origFragOffset)
                                       | origFragFlags);
        if (totalDataLength == (ttPktLen)0)
        {
/*
 * Last fragment. Copy dataSocketEntryPtr, dataFlags, and dataUserDataLength
 * for flow control with driver, and reset them in original packet.
 * This way the send queue will be updated only when we free the last
 * fragment packet, not when we free the original packet at the end of this
 * routine.
 */
            if (origSharedDataPtr->dataFlags & TM_BUF_USER_SEND)
            {
                fragSharedDataPtr->dataFlags |= TM_BUF_USER_SEND;
                fragSharedDataPtr->dataSocketEntryPtr =
                                        origSharedDataPtr->dataSocketEntryPtr;
                fragSharedDataPtr->dataUserDataLength =
                                        origSharedDataPtr->dataUserDataLength;
                origSharedDataPtr->dataFlags &= ~TM_BUF_USER_SEND;
                origSharedDataPtr->dataSocketEntryPtr = (ttSocketEntryPtr)0;
            }
        }
        else
        {
/* more fragment to send */
            fragFlagsFragOff |= TM_IP_HOST_MORE_FRAG;
        }
/* flagsFragOff of this fragment packet in network byte order */
        tm_htons(fragFlagsFragOff, fragIpHdrPtr->iphFlagsFragOff);

/*
 * Compute IP header checksum
 */
        fragIpHdrPtr->iphChecksum = 0;
        if (ipHdrLen == TM_4PAK_IP_MIN_HDR_LEN)
        {
            fragIpHdrPtr->iphChecksum = tfIpHdr5Checksum(
                (tt16BitPtr)((ttVoidPtr)fragIpHdrPtr));
        }
        else
        {
            fragIpHdrPtr->iphChecksum = tfPacketChecksum( fragHeadPacketPtr,
                                                     tm_byte_count(ipHdrLen),
                                                     TM_16BIT_NULL_PTR,
                                                     TM_16BIT_ZERO );
        }
/* All done with the headers */
/* Now point to duplicated data */
        dataLength = fragDataLength;
        while (dataLength != (ttPktLen)0)
        {
            if (origLinkDataLength <= dataLength)
            {
                dataLength -= origLinkDataLength;
                origLinkDataLength = 0;
/* origLinkDataLength == 0 */
/* We are done with the current link in the original packet */
/* Point to the next link in the original packet */
                origLinkPacketPtr = 
                        (ttPacketPtr)origLinkPacketPtr->pktLinkNextPtr;
                if (origLinkPacketPtr != TM_PACKET_NULL_PTR)
                {
/* Update fields for the next tfDuplicateSharedBuffer */
                    origLinkDataLength = origLinkPacketPtr->pktLinkDataLength;
                    origLinkDataPtr = origLinkPacketPtr->pktLinkDataPtr;
                    if (dataLength != (ttPktLen)0)
                    {
                        fragPacketPtr->pktLinkNextPtr = (ttVoidPtr)
                                tfDuplicateSharedBuffer(origLinkPacketPtr);
                        fragPacketPtr = 
                                (ttPacketPtr)fragPacketPtr->pktLinkNextPtr;
                        if (fragPacketPtr == (ttPacketPtr)0)
                        {
                            tfFreePacket(fragHeadPacketPtr, TM_SOCKET_UNLOCKED);
                            errorCode = TM_ENOBUFS;
                            goto ipFragExit;
                        }
                        fragHeadPacketPtr->pktLinkExtraCount++;
                    }
                }
#ifdef TM_ERROR_CHECKING
                else
                {
                    if ( dataLength != (ttPktLen)0 )
                    {
                        tfKernelError("tfIpFragmentPacket",
                                  "Trying to fragment a corrupted packet");
                    }
                }
#endif /* TM_ERROR_CHECKING */
            }
            else
            {
                if (fragPacketPtr != fragHeadPacketPtr)
                {
/* 
 * Last link of the current fragment. Update the link data length since it
 * is shorter.
 */
                    fragPacketPtr->pktLinkDataLength = (ttPktLen)dataLength;
                    fragPacketPtr->pktFlags2 |= TM_PF2_TRAILING_DATA;
                }
                origLinkDataLength -= dataLength;
/* OriglinkDataLength != 0 */
/* Update fields for the next tfDuplicateSharedBuffer */
                origLinkDataPtr += (int)tm_packed_byte_count(dataLength);
                dataLength = 0;
            }
        }
/* Send the packet */        
#ifdef TM_BYPASS_ETHER_LL
        if (isLan)
        {
/* Check for mininum size ethernet packet */
            if ( fragHeadPacketPtr->pktChainDataLength 
                                               < TM_ETHER_IP_MIN_PACKET )
            {
                errorCode = tfPacketTailAdd( fragHeadPacketPtr,
                                  (int)TM_ETHER_IP_MIN_PACKET
                                - (int)fragHeadPacketPtr->pktChainDataLength );

                if (errorCode != TM_ENOERROR)
                {
                    tfFreePacket(fragHeadPacketPtr, TM_SOCKET_UNLOCKED);
                    errorCode = TM_ENOBUFS;
                    break;
                }
            }
/* Add link layer size */
            fragHeadPacketPtr->pktChainDataLength += (ttPktLen)llHdrLength;
            fragHeadPacketPtr->pktLinkDataLength += (ttPktLen)llHdrLength;
            fragHeadPacketPtr->pktLinkDataPtr -= (ttPktLen)llHdrLength;
        }
#endif /* TM_BYPASS_ETHER_LL */
        if (needArpResolution == TM_8BIT_ZERO)
        {
            errorCode = tfDeviceSend(fragHeadPacketPtr);
/* 
 * BUG ID 285: Stop processing the packet and return the error if we fail
 * to send a fragment.
 */
            if (errorCode != TM_ENOERROR)
            {
                break; /* BUG ID 285 */
            }
        }
        else
        {
/*
 * We're a LAN and our ethernet address is NOT known, so call ARP.
 */
/* Chain in all the fragments for ARP resolution */
            if (prevFragPacketPtr == TM_PACKET_NULL_PTR)
/* First fragment */
            {
                firstFragPacketPtr = fragHeadPacketPtr;
            }
            else
            {
                prevFragPacketPtr->pktChainNextPtr = fragHeadPacketPtr;
            }
            prevFragPacketPtr = fragHeadPacketPtr;
            if (totalDataLength == (ttPktLen)0)
/* Last fragment. Queue the chain of fragments for resolution. */
            {
#ifdef TM_BYPASS_ETHER_LL
/*
 * Ownership count already increased for the packet ARP resolve 
 * in tfIpSendPacket().
 */
                resolveFunctionCalled = TM_8BIT_YES;
                errorCode = tfArpResolve(firstFragPacketPtr);
#else /* !TM_BYPASS_ETHER_LL */
                linkLayerProtocolPtr =
                                 origDeviceEntryPtr->devLinkLayerProtocolPtr;
                if (    (linkLayerProtocolPtr != TM_LINK_NULL_PTR)
                     && (linkLayerProtocolPtr->lnkResolveFuncPtr !=
                                            (ttLnkResolveFuncPtr)0) )
                {
/*
 * Ownership count already increased for the packet ARP resolve
 * in tfIpSendPacket().
 */
                    resolveFunctionCalled = TM_8BIT_YES;
                    errorCode = (*(linkLayerProtocolPtr->lnkResolveFuncPtr))
                                                         (firstFragPacketPtr);
                }
                else
                {
                    errorCode = TM_ENODEV;
                }
#endif /* TM_BYPASS_ETHER_LL */
            }
            if (errorCode != TM_ENOERROR)
            {
                break; /* free original packet, and return */
            }
/* Check whether address has been resolved before sending the next fragment */
            if (tm_16bit_all_bits_set( origRtePtr->rteFlags,
                                       TM_RTE_LINK_LAYER|TM_RTE_CLONED) )
            {
                tm_ether_copy(origRtePtr->rteHSEnetAdd, etherSrcOrigPtr);
                origPktFlags |= TM_ETHERNET_ADDRESS_FLAG;
            }
        }
#ifdef TM_SNMP_MIB
/*
 * Number of IP datagram fragments that have been generated as a result of
 * fragmentation at this entity.
 */
        tm_context(tvIpData).ipFragCreates++;
#ifdef TM_USE_NG_MIB2
        devPtr->dev4Ipv4Mib.ipIfStatsOutFragCreates++;
        tm_context(tvDevIpv4Data).ipIfStatsOutFragCreates++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
/* Update fragment offset for next fragment packet */
        fragOffset += fragDataLength;
    }
    while ( totalDataLength != (ttPktLen)0);

        

#else /* !TM_IP_FRAGMENT_NO_COPY */
/*
 * Loop through the original packet until we have sent all the fragments
 */
    do
    {
#ifdef TM_ERROR_CHECKING
        if (origLinkPacketPtr == (ttPacketPtr)0)
        {
            tfKernelError(
                "tfIpFragmentPacket",
                "pktChainDataLength doesn't match sum of " \
                "pktLinkDataLength values");
        }
#endif /* TM_ERROR_CHECKING */

/* compute data length (without IP header) of this fragment */
        if (totalDataLength < fragMaxSize)
        {
/* Last fragment shorter than other fragments */
            fragDataLength  = totalDataLength;
/* Total IP length of the shorter last fragment (including IP header) */
            fragIpDataLength = (tt16Bit)(   tm_byte_count(ipHdrLen)
                                          + fragDataLength );
        }
/*
 * Update total data length. When it reaches zero, we are sending our
 * last fragment.
 */
        totalDataLength -= fragDataLength;
/*
 * Allocate a new ttPacket buffer for fragment. We use TM_LL_MAX_HDR_LEN to
 * make sure that IP header is 32-bit aligned.
 * Allocate a buffer so that we can fit link layer , IP header and
 * fragDataLength
 */
        fragPacketPtr = tfGetSharedBuffer( TM_LL_MAX_HDR_LEN,
                                           (ttPktLen)fragIpDataLength,
                                           0);
        if (fragPacketPtr == TM_PACKET_NULL_PTR)
        {
            errorCode = TM_ENOBUFS;
            goto ipFragExit;
        }
/*
 * Initialize: IP header pointer, and shared data pointer from fragment 
 * packet.
 */
        fragIpHdrPtr = (ttIpHeaderPtr)fragPacketPtr->pktLinkDataPtr;
        fragSharedDataPtr = fragPacketPtr->pktSharedDataPtr;
/*
 * Copy relevant fields from original packet: pktFlags, pktDeviceEntryPtr,
 * dataRtePtr, dataSpecDestIpAddress (for ARP).
 */
        fragPacketPtr->pktFlags = origPktFlags;
        fragPacketPtr->pktDeviceEntryPtr = origDeviceEntryPtr;
        fragSharedDataPtr->dataRtePtr = origRtePtr;
/*
 * Only needed for LAN device, when TM_ETHERNET_ADDRESS_FLAG is not set
 * (i.e we need to go through ARP).
 */
#ifdef TM_USE_IPV6
        tm_6_ip_copy_structs(origSharedDataPtr->dataSpecDestIpAddress,
                             fragSharedDataPtr->dataSpecDestIpAddress);
#else /* ! TM_USE_IPV6 */
        tm_ip_copy(origSharedDataPtr->dataSpecDestIpAddress,
                   fragSharedDataPtr->dataSpecDestIpAddress);
#endif /* ! TM_USE_IPV6 */
/*
 * Copy the IP header including all options if this is the first fragment.
 * For additional fragment, we need to copy only the options that have
 * the copy bit set. We just go ahead and copy all the options in
 * successive fragments as well to reduce code size, since we know that
 * the options that we support need copying anyways.
 */
#ifdef TM_BYPASS_ETHER_LL
        if (isLan)
        {
/*
 * If we bypass the Ethernet Link layer, we also need to copy the
 * ethernet header. Copy both link layer header and IP header with options.
 */
/* Initialize link layer header pointer from fragment packet */
            fragLinkLayerHdrPtr =    ((tt8BitPtr)fragIpHdrPtr)
                                   - (int)llHdrLength;
            tm_bcopy( origLinkLayerHdrPtr,
                      fragLinkLayerHdrPtr,
                      llHdrLength + ipHdrLen );
        }
        else
        {
            tm_bcopy( origIpHdrPtr,
                      fragIpHdrPtr,
                      ipHdrLen );
        }
#else /* ! TM_BYPASS_ETHER_LL */
/*
 * If we do not bypass the Ethernet link layer, we do not have a link
 * layer built yet. So we only need to copy the IP header with options.
 */
        tm_bcopy( origIpHdrPtr,
                  fragIpHdrPtr,
                  ipHdrLen);
/*
 * If we had already found our ethernet address, copy it from the
 * original shared data structure.
 */
        if (isLan)
        {
            etherSrcFragPtr = (tt8BitPtr)&fragSharedDataPtr->dataEthernetAddress[0];
            if (origPktFlags & TM_ETHERNET_ADDRESS_FLAG)
            {
                tm_ether_copy(etherSrcOrigPtr, etherSrcFragPtr);
            }
        }
#endif /* TM_BYPASS_ETHER_LL */
/* Update total length of the fragment packet */
        tm_htons(fragIpDataLength, fragIpHdrPtr->iphTotalLength);
/* Update flagsFragOff of this fragment packet */
        linkLength = fragOffset >> 3; /* keep it as ttPktLen while shifting*/
        fragFlagsFragOff =  (tt16Bit) ( ((tt16Bit)linkLength + origFragOffset)
                                       | origFragFlags);
        if (totalDataLength == (ttPktLen)0)
        {
/*
 * Last fragment. Copy dataSocketEntryPtr, dataFlags, and dataUserDataLength
 * for flow control with driver, and reset them in original packet.
 * This way the send queue will be updated only when we free the last
 * fragment packet, not when we free the original packet at the end of this
 * routine.
 */
            if (origSharedDataPtr->dataFlags & TM_BUF_USER_SEND)
            {
                fragSharedDataPtr->dataFlags |= TM_BUF_USER_SEND;
                fragSharedDataPtr->dataSocketEntryPtr =
                                        origSharedDataPtr->dataSocketEntryPtr;
                fragSharedDataPtr->dataUserDataLength =
                                        origSharedDataPtr->dataUserDataLength;
                tm_16bit_clr_bit( origSharedDataPtr->dataFlags,
                                  TM_BUF_USER_SEND );
                origSharedDataPtr->dataSocketEntryPtr = (ttSocketEntryPtr)0;
            }
        }
        else
        {
/* more fragment to send */
            fragFlagsFragOff |= TM_IP_HOST_MORE_FRAG;
        }
/* flagsFragOff of this fragment packet in network byte order */
        tm_htons(fragFlagsFragOff, fragIpHdrPtr->iphFlagsFragOff);

/*
 * Compute IP header checksum
 */
        fragIpHdrPtr->iphChecksum = 0;
        if (ipHdrLen == TM_4PAK_IP_MIN_HDR_LEN)
        {
            fragIpHdrPtr->iphChecksum = tfIpHdr5Checksum(
                (tt16BitPtr)((ttVoidPtr)fragIpHdrPtr));
        }
        else
        {
            fragIpHdrPtr->iphChecksum = tfPacketChecksum(
                                                    fragPacketPtr,
                                                    tm_byte_count(ipHdrLen),
                                                    TM_16BIT_NULL_PTR,
                                                    TM_16BIT_ZERO);
        }
/* All done with the headers */
/* Now copy the data */
        fragDataPtr = ((tt8BitPtr)fragIpHdrPtr) + (int)ipHdrLen;
        dataLength = fragDataLength;
        while (dataLength != (ttPktLen)0)
        {
            if (origLinkDataLength < dataLength)
            {
                linkLength = origLinkDataLength;
            }
            else
            {
                linkLength = dataLength;
            }
            tm_bcopy( origLinkDataPtr,
                      fragDataPtr,
                      tm_packed_byte_count(linkLength) );
            fragDataPtr += tm_packed_byte_count(linkLength);
            dataLength -= linkLength;
            origLinkDataLength -= linkLength;
            if (origLinkDataLength == (ttPktLen)0)
            {
/* We are done with the current link in the original packet */
                if (origLinkPacketPtr != origPacketPtr)
/*
 * If not first link, free. (We cannot free the first link, since we need to
 * keep the IP header information around.)
 */
                {
/*
 * Remove current link from original chain, so that it does not get freed
 * twice (here and out of the loop below).
 */
                    origPacketPtr->pktLinkNextPtr =
                                          origLinkPacketPtr->pktLinkNextPtr;
/* Free current link */
                    tfFreeSharedBuffer(origLinkPacketPtr, TM_SOCKET_UNLOCKED);
                }
/* Point to the next link in the original packet */
                origLinkPacketPtr = (ttPacketPtr)origPacketPtr->pktLinkNextPtr;
                if (origLinkPacketPtr != TM_PACKET_NULL_PTR)
                {
                    origLinkDataLength = origLinkPacketPtr->pktLinkDataLength;
                    origLinkDataPtr = origLinkPacketPtr->pktLinkDataPtr;
                }
#ifdef TM_ERROR_CHECKING
                else
                {
                    if ( dataLength != (ttPktLen)0 )
                    {
                        tfKernelError("tfIpFragmentPacket",
                                  "Trying to fragment a corrupted packet");
                    }
                }
#endif /* TM_ERROR_CHECKING */
            }
            else
            {
                origLinkDataPtr += (int)tm_packed_byte_count(linkLength);
            }
        }
/* Send the packet */        
#ifdef TM_BYPASS_ETHER_LL
        if (isLan)
        {
/* Check for mininum size ethernet packet */
            if ( fragPacketPtr->pktChainDataLength < TM_ETHER_IP_MIN_PACKET )
            {
                fragPacketPtr->pktChainDataLength = TM_ETHER_IP_MIN_PACKET;
            }
/* Add link layer size */
            fragPacketPtr->pktChainDataLength += (ttPktLen)llHdrLength;
            fragPacketPtr->pktLinkDataLength =
                                           fragPacketPtr->pktChainDataLength;
            fragPacketPtr->pktLinkDataPtr -= (ttPktLen)llHdrLength;
        }
#endif /* TM_BYPASS_ETHER_LL */
        if (needArpResolution == TM_8BIT_ZERO)
        {
            errorCode = tfDeviceSend(fragPacketPtr);
/* 
 * BUG ID 285: Stop processing the packet and return the error if we fail
 * to send a fragment.
 */
            if (errorCode != TM_ENOERROR)
            {
                break; /* BUG ID 285 */
            }
        }
        else
        {
/*
 * We're a LAN and our ethernet address is NOT known, so call ARP.
 */
/* Chain in all the fragments for ARP resolution */
            if (prevFragPacketPtr == TM_PACKET_NULL_PTR)
/* First fragment */
            {
                firstFragPacketPtr = fragPacketPtr;
            }
            else
            {
                prevFragPacketPtr->pktChainNextPtr = fragPacketPtr;
            }
            prevFragPacketPtr = fragPacketPtr;
            if (totalDataLength == 0)
            {
#ifdef TM_BYPASS_ETHER_LL
/*
 * Ownership count already increased for packet ARP resolve
 * in tfIpSendPacket().
 */
                resolveFunctionCalled = TM_8BIT_YES;
                errorCode = tfArpResolve(firstFragPacketPtr);
#else /* !TM_BYPASS_ETHER_LL */
                linkLayerProtocolPtr =
                                 origDeviceEntryPtr->devLinkLayerProtocolPtr;
                if (    (linkLayerProtocolPtr != TM_LINK_NULL_PTR)
                    && (linkLayerProtocolPtr->lnkResolveFuncPtr !=
                                            (ttLnkResolveFuncPtr)0) )
                {
/*
 * Ownership count already increased for packet ARP resolve
 * in tfIpSendPacket().
 */
                    resolveFunctionCalled = TM_8BIT_YES;
                    errorCode = (*(linkLayerProtocolPtr->lnkResolveFuncPtr))
                                                         (firstFragPacketPtr);
                }
                else
                {
                    errorCode = TM_ENODEV;
                }
#endif /* TM_BYPASS_ETHER_LL */
            }
            if (errorCode != TM_ENOERROR)
            {
                break; /* free original packet, and return */
            }
/* Check whether address has been resolved before sending the next fragment */
            if (tm_16bit_all_bits_set( origRtePtr->rteFlags,
                                       TM_RTE_LINK_LAYER|TM_RTE_CLONED) )
            {
                tm_ether_copy(origRtePtr->rteHSEnetAdd, etherSrcOrigPtr);
                origPktFlags |= TM_ETHERNET_ADDRESS_FLAG;
            }
        }
#ifdef TM_SNMP_MIB
/*
 * Number of IP datagram fragments that have been generated as a result of
 * fragmentation at this entity.
 */
        tm_context(tvIpData).ipFragCreates++;
#ifdef TM_USE_NG_MIB2
        origDeviceEntryPtr->dev4Ipv4Mib.ipIfStatsOutFragCreates++;
        tm_context(tvDevIpv4Data).ipIfStatsOutFragCreates++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
/* Update fragment offset for next fragment packet */
        fragOffset += fragDataLength;
    }
    while ( totalDataLength != (ttPktLen)0);
#endif /* !TM_IP_FRAGMENT_NO_COPY */
ipFragExit:
    if (errorCode != TM_ENOERROR)
    {
        if (    (needArpResolution != TM_8BIT_NO)
             && (resolveFunctionCalled == TM_8BIT_ZERO) )
        {
/*
 * Release ownership of ARP entry for first fragment (since it was increased
 * in tfIpSendPacket()), since link resolve function could not be called.
 */
            tfRtUnGet(origRtePtr);
/* Free all fragments queued so far for ARP resolution */
            tfFreeChainPacket(firstFragPacketPtr, TM_SOCKET_UNLOCKED);
        }
    }
/* Free original packet */
    tfFreePacket(origPacketPtr, TM_SOCKET_UNLOCKED);
#ifdef TM_SNMP_MIB
/*
 * Number of IP datagrams that have been successfully fragmented at this
 * entity.
 */
    if (errorCode == TM_ENOERROR)
    {
        tm_context(tvIpData).ipFragOKs++;
    }
#endif /* TM_SNMP_MIB */
    return errorCode;
}
#ifdef TM_LINT
LINT_UNACCESS_SYM_END(fragFlagsFragOff)
LINT_UNINIT_AUTO_END(etherSrcOrigPtr)
#endif /* TM_LINT */
#endif /* TM_IP_FRAGMENT */

#else /* !TM_IP_FRAGMENT && ! TM_IP_REASSEMBLY */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */


/* 
 * To allow link for builds when TM_IP_FRAGMENT and TM_IP_REASSEMBLY are 
 * not defined 
 */
int tv4IpfragDummy = 0;

#endif  /* !TM_IP_FRAGMENT && ! TM_IP_REASSEMBLY  */

#else /* !TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */


/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4IpfragDummy = 0;

#endif /* !TM_USE_IPV4 */
