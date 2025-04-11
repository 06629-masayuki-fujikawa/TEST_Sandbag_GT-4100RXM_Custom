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
 * Description: SNMP agent cache functions (for UDP socket table, 
 *              connected TCP vectors table, routing table, Arp table.)
 * Filename: trsnmpca.c
 * Author: Odile
 * Date Created: 01/18/02
 * $Source: source/sockapi/trsnmpca.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:44:28JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


#include <trsocket.h>
#include <trmacro.h>
#ifdef TM_SNMP_CACHE
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/* Local macros */



/* local types */
typedef ttNodePtr (TM_CODE_FAR * ttSnmpSearchFuncPtr)(
    ttSnmpCacheEntryPtr snmpCacheEntryPtr,
    ttListPtr           listPtr);

/* Local functions */

static void tfSnmpdCacheDeleteEntry (ttSnmpCacheEntryPtr snmpCacheEntryPtr);

/* Insert an entry in its corresponding cache */
static void tfSnmpdCacheInsertEntry(ttSnmpCacheEntryPtr snmpCacheEntryPtr);

#ifdef TM_USE_IPV4
/* 
 * Copy ARP entry destination address + copy device index to
 * passed ARP search structure
 */
static void tf4SnmpArpLexGraphOrder(ttRteEntryPtr       rtePtr,
                                    tt4SnmpArpSearchPtr snmp4ArpSearchPtr);
/* tfListWalk CB function ARP entry comparison */
static int tf4SnmpArpCompareCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam);

/* Search through the IPv4 ARP table to find point of insertion */
static ttNodePtr tf4SnmpArpSearch(ttSnmpCacheEntryPtr snmpCacheEntryPtr,
                                  ttListPtr           listPtr);

/* 
 * Copy Route entry dest, bit offset, hops and device index
 * to the passed Route search structure
 */
static void tf4SnmpRouteLexGraphOrder(ttRteEntryPtr         rtePtr,
                                      tt4SnmpRouteSearchPtr snmp4RouteSearchPtr);

/* tfListWalk CB function Route entry comparison */
static int tf4SnmpRouteCompareCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam);

/* Search through the IPv4 Route table to find point of insertion */
static ttNodePtr tf4SnmpRouteSearch(ttSnmpCacheEntryPtr snmpCacheEntryPtr,
                                    ttListPtr           listPtr);

/* Copy 2-tuple IPv4 address/port , and devIndex */
static void tf4SnmpUdpLexGraphOrder(ttTupleDevPtr       tupleDevPtr,
                                    tt4SnmpUdpTuplePtr  snmp4UdpTuplePtr);

/* tfListWalk CB function UDP entry comparison */
static int tf4SnmpUdpTupleCompareCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam);

/* Search through the IPv4 UDP table to find point of insertion */
static ttNodePtr tf4SnmpUdpSearch(ttSnmpCacheEntryPtr snmpCacheEntryPtr,
                                 ttListPtr           listPtr);

/* Copy 4-tuple IPv4 address/port, and devIndex */
static void tf4SnmpTcpLexGraphOrder(ttTupleDevPtr      tupleDevPtr,
                                    tt4SnmpTcpTuplePtr snmp4TcpTuplePtr);

/* tfListWalk CB function TCP entry comparison */
static int tf4SnmpTcpTupleCompareCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam);

/* Search through the IPv4 TCP table to find point of insertion */
static ttNodePtr tf4SnmpTcpSearch(ttSnmpCacheEntryPtr snmpCacheEntryPtr,
                                 ttListPtr           listPtr);
#endif /* TM_USE_IPV4 */


#ifdef TM_USE_IPV6
/* Copy IPv6 address, and reset scope id  */
static void tfSnmpAddrLexGraphOrder(ttIpAddressPtr netAddrPtr,
                                    ttIpAddressPtr hostAddrPtr);

/* 
 * Copy ARP entry destination address + copy device index to
 * passed ARP search structure
 */
static void tfSnmpArpLexGraphOrder(ttRteEntryPtr       rtePtr,
                                   ttSnmpArpSearchPtr  snmpArpSearchPtr);

/* tfListWalk CB function ARP entry comparison */
static int tfSnmpArpCompareCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam);

/* Search through the IPv ARP table to find point of insertion */
static ttNodePtr tfSnmpArpSearch(ttSnmpCacheEntryPtr snmpCacheEntryPtr,
                                  ttListPtr           listPtr);

/* 
 * Copy Route entry dest, bit offset, hops and device index
 * to the passed Route search structure
 */
static void tfSnmpRouteLexGraphOrder(ttRteEntryPtr          rtePtr,
                                     ttSnmpRouteSearchPtr  snmpRouteSearchPtr);
/* tfListWalk CB function Route entry comparison */
static int tfSnmpRouteCompareCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam);

/* Search through the IPv Route table to find point of insertion */
static ttNodePtr tfSnmpRouteSearch(ttSnmpCacheEntryPtr snmpCacheEntryPtr,
                                  ttListPtr           listPtr);

/* Copy 2-tuple IPv6 address/port + devIndex */
static void tfSnmpUdpLexGraphOrder(ttTupleDevPtr      tupleDevPtr,
                                   ttSnmpUdpTuplePtr  snmpUdpTuplePtr);

/* tfListWalk CB function UDP entry comparison */
static int tfSnmpUdpTupleCompareCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam);

/* Search through the IPv6 UDP table to find point of insertion */
static ttNodePtr tfSnmpUdpSearch(ttSnmpCacheEntryPtr snmpCacheEntryPtr,
                                 ttListPtr           listPtr);

/* Copy 4-tuple IPv6 address/port + devIndex */
static void tfSnmpTcpLexGraphOrder(ttTupleDevPtr      tupleDevPtr,
                                   ttSnmpTcpTuplePtr  snmpTcpTuplePtr);

/* tfListWalk CB function TCP entry comparison */
static int tfSnmpTcpTupleCompareCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam);

/* Search through the IPv6 TCP table to find point of insertion */
static ttNodePtr tfSnmpTcpSearch(ttSnmpCacheEntryPtr snmpCacheEntryPtr,
                                 ttListPtr           listPtr);
#endif /* TM_USE_IPV6 */

/* 
 * Copy Prefix entry device index, address family, prefix, prefix length
 * to passed Prefix search structure
 */
static void tfSnmpPrefixLexGraphOrder(ttSnmpPrefixCachePtr   prefixCachePtr,
                                      ttSnmpPrefixSearchPtr  snmpPrefixSearchPtr);

/* tfListWalk CB function Prefix entry comparison */
static int tfSnmpPrefixCompareCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam);

static int tfSnmpPrefixCompDelCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam);

/* Search through the IPv4,6 Prefix table to find point of insertion */
static ttNodePtr tfSnmpPrefixSearch(ttSnmpCacheEntryPtr snmpCacheEntryPtr,
                                    ttListPtr           listPtr);

/*
 * tfSnmpdCacheDeleteRoute function description:
 * . Common call for ARP cache entry, and IP routing entry deletion
 *   from the SNMP cache.
 * . Called with the Patricia Tree Lock on.
 * . Called from tfRtRemoveEntry() when routing/Arp cache entry
 *   is to be removed from the Patricia tree. 
 * . That routing entry may or may not be in the cache, as
 *   it could also be a broadcast, multicast entry that does
 *   not belong in the SNMP routing/Arp entries cache, but
 *   tfRtRemoveEntry() checks that the entry is in the cache
 *   prior to calling this routine.
 *   Parameters  Meaning
 *   rtePtr      Pointer to routing entry to be removed from the cache.
 *   No Return
 */
void tfSnmpdCacheDeleteRoute (ttRteEntryPtr rtePtr)
{
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
    ttSnmpCacheEntryPtr snmpCacheEntryPtr;
    int                 cacheIndex;

    snmpCacheEntryPtr = &(rtePtr->rteSnmpCacheEntry);
    cacheIndex = snmpCacheEntryPtr->sceTableIndex;
/* Only keep indirect routes, and local routes */
    if (((cacheIndex == TM_SNMPC_ROUTE_INDEX)
#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
        || (cacheIndex == TM_SNMPC6_ROUTE_INDEX)
#endif /* dual */
        )
        && (tm_16bit_one_bit_set(rtePtr->rteFlags, 
                                 TM_RTE_INDIRECT | TM_RTE_LOCAL))
        && (tm_16bit_bits_not_set(rtePtr->rteFlags, TM_RTE_REJECT)))
    {
        tm_context(tvIpForwardData).inetCidrRouteNumber--;
    }
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
    tfSnmpdCacheDeleteEntry(&(rtePtr->rteSnmpCacheEntry));
    return;
}

/*
 * tfSnmpdCacheInsertRoute function description:
 * . Called with the Patricia Tree Lock on.
 * . Called from tfRtClone() when Arp cache entry
 *   has been added to the Patricia tree.  
 * . Or Called from tfRtLockAdd() when routing entry
 *   has been added to the Patricia tree. 
 * . If entry is an arp entry, use the SNMP Arp table cache
 *   and insert the ARP cache entry by devIndex/Ip address.
 * . If entry is a routing entry, use the SNMP routing table cache
 *   and insert the routing entry by Ip address.
 * . Determine which cache the entry is to be inserted to
 * . Use common routine tfSnmpCacheInsertEntry()
 *   Parameters  Meaning
 *   rtePtr      Pointer to ARP entry, or routing entry to be added to
 *               the cache.
 *   cacheIndex  indicates if rtePtr is an ARP entry (TM_SNMPC_ARP_INDEX),
 *               or a routing entry (TM_SNMPC_ROUTE_INDEX).
 *   No Return
 */
void tfSnmpdCacheInsertRoute (ttRteEntryPtr rtePtr , int cacheIndex)
{
    tt8Bit              entryType;


    if (cacheIndex == TM_SNMPC_ARP_INDEX)
    {
        entryType = TM_SNMPC_ARP_TYPE;
    }
    else
    {
        entryType = TM_SNMPC_ROUTE_TYPE;
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
/* Only keep indirect routes, and local routes */
        if ((tm_16bit_one_bit_set(rtePtr->rteFlags, 
                                  TM_RTE_INDIRECT | TM_RTE_LOCAL))
            && tm_16bit_bits_not_set(rtePtr->rteFlags, TM_RTE_REJECT))
        {
            tm_context(tvIpForwardData).inetCidrRouteNumber++;
        }
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
    }
/*
 * In dual mode, convert to appropriate IPv4, or IPv6 cache.
 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
/* In dual mode, check which cache the entry should be inserted in */
    if (!(tm_6_addr_is_ipv4_mapped(&(rtePtr->rteDest))))
    {
/* Not IPv4 key/nexhop */
        if ( (    (    rtePtr->rteLeafNode.rtnPrefixLength 
                    != TM_6_DEF_4GWAY_PREFIX_LENGTH ) )
               || ( tm_6_ip_not_zero(&(rtePtr->rteDest)) ) )
/* Not IPv4 default gateway */
        {
/* IPv6 index in dual mode */
            cacheIndex = cacheIndex + TM_SNMPC6_BASE_INDEX;
        }
    }
#else /* dual */
/* In IPv6 mode only, there should not be any mapped IPv4 address */
    tm_assert( tfSnmpdCacheInsertRoute,
               !(tm_6_addr_is_ipv4_mapped(&(rtePtr->rteDest))) );
#endif /* IPv6 only */
#endif /* TM_USE_IPV6 */

/* Store cache index for deletion */
    rtePtr->rteSnmpCacheEntry.sceTableIndex = (tt8Bit)cacheIndex;
    rtePtr->rteSnmpCacheEntry.sceTableEntryPtr = (ttVoidPtr)rtePtr;
    rtePtr->rteSnmpCacheEntry.sceParmPtr = (ttVoidPtr)rtePtr;
    rtePtr->rteSnmpCacheEntry.sceTableEntryType = entryType;
    tfSnmpdCacheInsertEntry(&(rtePtr->rteSnmpCacheEntry));
}

/*
 * tfSnmpdCacheDeleteSocket function description:
 * . Common call for UDP socket entry, and TCP socket deletion
 *   from the SNMP cache.
 * . Called with the Socket Tree Lock on from tfSocketTreeDelete(), when
 *   socket entry is to be removed from the tree.
 *   Parameters      Meaning
 *   socketPtr       Pointer to a socket entry to be deleted from the cache.
 *   No Return
 */
void tfSnmpdCacheDeleteSocket (ttSocketEntryPtr socketPtr)
{
    tfSnmpdCacheDeleteEntry(&(socketPtr->socSnmpCacheEntry));
    return;
}

/*
 * tfSnmpdCacheDeleteTmWtVect function description:
 * . Called with the Table Time Wait Table Lock on from tfTcpTmWtVectClose(),
 *   when a TCP time wait vector entry is to be removed from the Time Wait
 *   vector hash table.
 *   Parameters      Meaning
 *   tcpTmWtVectPtr  Pointer to a Time Wait TCP vector entry to be deleted
 *                   from the cache.
 *   No Return
 */
void tfSnmpdCacheDeleteTmWtVect (ttTcpTmWtVectPtr tcpTmWtVectPtr)
{
    tfSnmpdCacheDeleteEntry(&(tcpTmWtVectPtr->twsSnmpCacheEntry));
    return;
}

/*
 * tfSnmpdCacheInsertSocket function description:
 * . Called with the Socket Tree Lock on.
 * . Called from tfSocketTreeInsert() when a socket entry has been  
 *   added to the Socket Tree.  
 * . If entry is a UDP socket, use the SNMP UDP socket table cache
 *   and insert the UDP socket by local IP address/local port.
 * . If entry is a TCP socket, use the SNMP TCP vector table cache 
 *   and insert the socket entry by local IP address/local port/
 *   remote IP address/remote port.
 * . Insertion is in a null terminated doubly linked list pointed to by
 *   the SNMP cache head.
 * . Update pointers to first, and last element of the list.
 * . Update the SNMP cache serial number.
 *   Parameters      Meaning
 *   socketPtr       Pointer to a socket entry to be added to the cache.
 *   No Return
 */
void tfSnmpdCacheInsertSocket (ttSocketEntryPtr socketPtr)
{
    int                 cacheIndex;
    tt8Bit              entryType;

    entryType = (tt8Bit)TM_SNMPC_UDP_TYPE; 
/* Find which cache this entry belongs to */
    if (socketPtr->socProtocolNumber == IP_PROTOUDP)
    {
        cacheIndex = TM_SNMPC_UDP_INDEX;
/*
 * In dual mode, convert to appropriate IPv4, or IPv6 cache.
 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
/* In dual mode, check which cache the entry should be inserted in */
        if (    (socketPtr->socProtocolFamily == PF_INET6)
             && (!(tm_6_addr_is_ipv4_mapped(
                                     &(socketPtr->socOurIfIpAddress)))) )
        {
/* IPv6 index in dual mode */
            cacheIndex = cacheIndex + TM_SNMPC6_BASE_INDEX;
        }
#else /* IPv6 only */
/* In IPv6 mode only, there should not be any mapped IPv4 address */
        tm_assert( tfSnmpdCacheInsertSocket,
                   !(tm_6_addr_is_ipv4_mapped(
                                 &(socketPtr->socOurIfIpAddress))) );
#endif /* IPv6 only */
#endif /* TM_USE_IPV6 */
    }
    else
    {
        if (socketPtr->socProtocolNumber == IP_PROTOTCP)
        {
/* Connected */
            entryType = (tt8Bit)TM_SNMPC_TCP_TYPE; 
            cacheIndex = TM_SNMPC_TCP_INDEX;
/*
 * In dual mode, convert to appropriate IPv4, or IPv6 cache.
 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
/* In dual mode, check which cache the entry should be inserted in */
            if (socketPtr->socProtocolFamily == PF_INET6)
            {
                 if (    (    (socketPtr->socCoFlags 
                                               == TM_SOC_TREE_CL_FLAG)
                           && (!(tm_6_addr_is_ipv4_mapped(
                                &(socketPtr->socOurIfIpAddress)))) )
                      || (    (entryType == TM_SNMPC_TCP_TYPE) 
                           && (!(tm_6_addr_is_ipv4_mapped(
                                &(socketPtr->socPeerIpAddress)))) ) )
                {
/* IPv6 index in dual mode */
                    cacheIndex = cacheIndex + TM_SNMPC6_BASE_INDEX;
                }
            }
#else /* IPv6 only */
/* In IPv6 mode only, there should not be any mapped IPv4 address */
            tm_assert( tfSnmpdCacheInsertSocket,
                       (   !(tm_6_addr_is_ipv4_mapped(
                                 &(socketPtr->socOurIfIpAddress)))
                        && !(tm_6_addr_is_ipv4_mapped(
                                 &(socketPtr->socPeerIpAddress))) ) );
#endif /* IPv6 only */
#endif /* TM_USE_IPV6 */
        }
        else
        {
            cacheIndex = -1; /* Non TCP, non UDP socket */
        }
    }
    if (cacheIndex != -1)
    {
        socketPtr->socSnmpCacheEntry.sceTableIndex = (tt8Bit)cacheIndex;
        socketPtr->socSnmpCacheEntry.sceParmPtr = (ttVoidPtr)
                                                      &(socketPtr->socTupleDev);
        socketPtr->socSnmpCacheEntry.sceTableEntryPtr = (ttVoidPtr)socketPtr;
        socketPtr->socSnmpCacheEntry.sceTableEntryType = entryType;
        tfSnmpdCacheInsertEntry(&(socketPtr->socSnmpCacheEntry));
    }
}

/* Insert a TCP time wait vector in the TCP cache */
void tfSnmpdCacheInsertTmWtVect (ttTcpTmWtVectPtr tcpTmWtVectPtr)
{
    int cacheIndex;

    cacheIndex = TM_SNMPC_TCP_INDEX;
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
/* Dual mode */
    if (tcpTmWtVectPtr->twsNetworkLayer != TM_NETWORK_IPV4_LAYER) 
    {
/* IPv6 in dual mode */
        cacheIndex = TM_SNMPC_TCP_INDEX + TM_SNMPC6_BASE_INDEX;
    }
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */
    tcpTmWtVectPtr->twsSnmpCacheEntry.sceTableIndex = (tt8Bit)cacheIndex;
    tcpTmWtVectPtr->twsSnmpCacheEntry.sceTableEntryPtr = (ttVoidPtr)
                                                               tcpTmWtVectPtr;
    tcpTmWtVectPtr->twsSnmpCacheEntry.sceParmPtr = (ttVoidPtr)
                                            &(tcpTmWtVectPtr->twsTupleDev);
    tcpTmWtVectPtr->twsSnmpCacheEntry.sceTableEntryType =
                                            (tt8Bit)TM_SNMPC_TCP_TIME_WAIT_TYPE;
    tfSnmpdCacheInsertEntry(&(tcpTmWtVectPtr->twsSnmpCacheEntry));
}

/* 
 * tfSnmpdCacheDeleteEntry function description:
 * . Common call for socket, route, ARP, deletion.
 * . Map TCP socket to SNMP TCP vector cache, and UDP socket to SNMP UDP 
 *   socket cache. 
 * . Delete entry from list starting at SNMP cache head.
 *   head. 
 * . Update pointers to first, and last element of the list.
 * . Update the corresponding table SNMP cache serial number.
 */
static void tfSnmpdCacheDeleteEntry (ttSnmpCacheEntryPtr snmpCacheEntryPtr)
{
    ttSnmpdCacheHeadPtr snmpdCacheHeadPtr;
    int                 cacheIndex;

/* Find which cache this entry belongs to */
    cacheIndex = snmpCacheEntryPtr->sceTableIndex;
    tm_assert(tfSnmpdCacheDeleteEntry, cacheIndex != TM_SNMPC_NO_INDEX);
    snmpdCacheHeadPtr = &(tm_context(tvSnmpdCaches)[cacheIndex]);
#ifdef TM_LOCK_NEEDED
/* Lock the cache (to lock out SNMP cache lookup) */
    tm_call_lock_wait(&(snmpdCacheHeadPtr->snmpcLockEntry));
#endif /* TM_LOCK_NEEDED */
    if (snmpdCacheHeadPtr->snmpcNextNodePtr == &(snmpCacheEntryPtr->sceNode))
    {
/*
 * Make Insert next to scan point to next, if next to scan node is being
 * deleted
 */
        snmpdCacheHeadPtr->snmpcNextNodePtr =
                                     snmpCacheEntryPtr->sceNode.nodeNextPtr;
    }
    tfListRemove(&(snmpdCacheHeadPtr->snmpcList),
                 &(snmpCacheEntryPtr->sceNode));
    snmpCacheEntryPtr->sceTableIndex = TM_SNMPC_NO_INDEX;
#ifdef TM_SNMP_VERSION
    if (snmpdCacheHeadPtr->snmpcLastReadNodePtr ==
                                             &(snmpCacheEntryPtr->sceNode))
    {
/* Make GETNEXT point to next, if last read node is being deleted */
        snmpdCacheHeadPtr->snmpcLastReadNodePtr =
                                     snmpCacheEntryPtr->sceNode.nodeNextPtr; 
    }
#endif /* TM_SNMP_VERSION */
#ifdef TM_LOCK_NEEDED
/* Unlock the cache (to allow SNMP lookup) */
    tm_call_unlock(&(snmpdCacheHeadPtr->snmpcLockEntry));
#endif /* TM_LOCK_NEEDED */
}


/* Insert an entry in its corresponding cache */
static void tfSnmpdCacheInsertEntry(ttSnmpCacheEntryPtr snmpCacheEntryPtr)
{
    ttSnmpdCacheHeadPtr snmpdCacheHeadPtr;
    ttSnmpSearchFuncPtr snmpSearchFuncPtr;
    ttListPtr           listPtr;
    ttNodePtr           insertNodePtr;
    ttNodePtr           nodePtr;
    int                 cacheIndex;

    insertNodePtr = &(snmpCacheEntryPtr->sceNode);
#ifdef TM_ERROR_CHECKING
/* Check if already inserted in the cache */
    if (insertNodePtr->nodeNextPtr != (ttNodePtr)0)
    {
        tfKernelError("tfSnmpdCacheInsertEntry", "Entry already in cache!\n");
    }
    else
#endif /* TM_ERROR_CHECKING */
    {
        cacheIndex = snmpCacheEntryPtr->sceTableIndex;
        snmpdCacheHeadPtr = &(tm_context(tvSnmpdCaches)[cacheIndex]);
        snmpSearchFuncPtr = (ttSnmpSearchFuncPtr)0; /* compiler warning */
#ifdef TM_LOCK_NEEDED
/* Lock the cache (to lock out SNMP cache lookup) */
        tm_call_lock_wait(&(snmpdCacheHeadPtr->snmpcLockEntry));
#endif /* TM_LOCK_NEEDED */
        listPtr = &(snmpdCacheHeadPtr->snmpcList);
/* Save search criteria in local variables for loop efficiency */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
        if (cacheIndex >= TM_SNMPC6_BASE_INDEX)
#endif /* TM_USE_IPV4 */
        {
            switch (snmpCacheEntryPtr->sceTableEntryType)
            {
                case TM_SNMPC_ARP_TYPE:
                    snmpSearchFuncPtr = tfSnmpArpSearch;
                    break;
                case TM_SNMPC_ROUTE_TYPE:
                    snmpSearchFuncPtr = tfSnmpRouteSearch;
                    break;
                case TM_SNMPC_UDP_TYPE:
                    snmpSearchFuncPtr = tfSnmpUdpSearch;
                    break;
                case TM_SNMPC_TCP_TYPE:
/* Fall through */
                case TM_SNMPC_TCP_TIME_WAIT_TYPE: 
                    snmpSearchFuncPtr = tfSnmpTcpSearch;
                    break;
                case TM_SNMPC_PREFIX_TYPE:
                    snmpSearchFuncPtr = tfSnmpPrefixSearch;
                    break;
                default:
                    snmpSearchFuncPtr = (ttSnmpSearchFuncPtr)0;
                    break;
            }
        }
#ifdef TM_USE_IPV4
        else
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */
        {
#ifdef TM_USE_IPV4
            switch (snmpCacheEntryPtr->sceTableEntryType)
            {
                case TM_SNMPC_ARP_TYPE:
                    snmpSearchFuncPtr = tf4SnmpArpSearch;
                    break;
                case TM_SNMPC_ROUTE_TYPE:
                    snmpSearchFuncPtr = tf4SnmpRouteSearch;
                    break;
                case TM_SNMPC_UDP_TYPE:
                    snmpSearchFuncPtr = tf4SnmpUdpSearch;
                    break;
                case TM_SNMPC_TCP_TYPE:
/* Fall through */
                case TM_SNMPC_TCP_TIME_WAIT_TYPE: 
                    snmpSearchFuncPtr = tf4SnmpTcpSearch;
                    break;
                case TM_SNMPC_PREFIX_TYPE:
                    snmpSearchFuncPtr = tfSnmpPrefixSearch;
                    break;
                default:
                    snmpSearchFuncPtr = (ttSnmpSearchFuncPtr)0;
                    break;
            }
#endif /* TM_USE_IPV4 */
        }
        if (snmpSearchFuncPtr != (ttSnmpSearchFuncPtr)0)
        {
            nodePtr = (*snmpSearchFuncPtr)(snmpCacheEntryPtr, listPtr);
            if (nodePtr == (ttNodePtr)0)
/* Insert at end of the list */
            {
                tfListAddToTail(listPtr, insertNodePtr);
            }
            else
            {
/* Insert before nodePtr */
                insertNodePtr->nodeNextPtr = nodePtr;
                insertNodePtr->nodePrevPtr = nodePtr->nodePrevPtr;
                nodePtr->nodePrevPtr->nodeNextPtr = insertNodePtr;
                nodePtr->nodePrevPtr = insertNodePtr;
                listPtr->listCount++;
            }
        }
#ifdef TM_LOCK_NEEDED
/* Unlock the cache (to allow SNMP lookup) */
        tm_call_unlock(&(snmpdCacheHeadPtr->snmpcLockEntry));
#endif /* TM_LOCK_NEEDED */
    }
}


#ifdef TM_USE_IPV4
/* 
 * Copy ARP entry destination address and device index in network
 * byte order to passed SNMP ARP search structure
 */
static void tf4SnmpArpLexGraphOrder(ttRteEntryPtr       rtePtr,
                                    tt4SnmpArpSearchPtr snmp4ArpSearchPtr)
{
    tt32Bit devIndex;

    devIndex = rtePtr->rteDevIndex;
    tm_htonl(devIndex, snmp4ArpSearchPtr->snmpa4DevIndex);
    tm_ip_copy(tm_4_ip_addr(rtePtr->rteDest), snmp4ArpSearchPtr->snmpa4Dest);
    return;
}
/*
 * tfListWalk CB function
 * Parameter is pointer to an ARP search structre
 * Copy ARP entry dest and device index to a local ARP
 * search structure
 * compare the 2 structures.
 */
static int tf4SnmpArpCompareCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam)
{
    tt4SnmpArpSearch    snmp4ArpSearch;
    int                 retCode;

    tf4SnmpArpLexGraphOrder((ttRteEntryPtr)
        (((ttSnmpCacheEntryPtr)(ttVoidPtr)nodePtr)->sceTableEntryPtr),
        &snmp4ArpSearch);
    retCode = TM_8BIT_NO;
    if (tm_memcmp(&snmp4ArpSearch,
                        genParam.genVoidParmPtr,
                        sizeof(tt4SnmpArpSearch)) > 0)
    {
        retCode = TM_8BIT_YES;
    }
    return retCode;
}

/* Search through the IPv4 ARP table to find point of insertion */
static ttNodePtr tf4SnmpArpSearch(ttSnmpCacheEntryPtr snmpCacheEntryPtr,
                                  ttListPtr           listPtr)
{
    ttNodePtr           nodePtr;
    tt4SnmpArpSearch    snmp4ArpSearch;
    ttGenericUnion      genParam;

    tf4SnmpArpLexGraphOrder((ttRteEntryPtr)(snmpCacheEntryPtr->sceTableEntryPtr),
                       &snmp4ArpSearch);
    genParam.genVoidParmPtr = &snmp4ArpSearch;
    nodePtr = tfListWalk(listPtr,
                         tf4SnmpArpCompareCB, genParam);
    return nodePtr;
}

/* 
 * Copy Route entry dest, bit offset, hops and device index
 * to the passed Route search structure
 */
static void tf4SnmpRouteLexGraphOrder(ttRteEntryPtr          rtePtr,
                                      tt4SnmpRouteSearchPtr  snmp4RouteSearchPtr)
{
#ifdef TM_USE_STRONG_ESL
    tt32Bit temp32Bit;
#endif /* TM_USE_STRONG_ESL */

    tm_ip_copy(tm_4_ip_addr(rtePtr->rteDest), snmp4RouteSearchPtr->snmpr4Dest);
    snmp4RouteSearchPtr->snmpr4BitOffset = rtePtr->rteLeafNode.rtnBitOffset; 
#ifdef TM_USE_STRONG_ESL
    temp32Bit = rtePtr->rteHops;
    tm_htonl(temp32Bit, snmp4RouteSearchPtr->snmpr4Hops);
    temp32Bit = rtePtr->rteDevIndex;
    tm_htonl(temp32Bit, snmp4RouteSearchPtr->snmpr4DevIndex);
#endif /* TM_USE_STRONG_ESL */
    return;
}
/*
 * tfListWalk CB function
 * Parameter is pointer to a Route search structre
 * Copy Route entry dest, bit offset, hops and device index to a local Route
 * search structure
 * compare the 2 structures.
 */
static int tf4SnmpRouteCompareCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam)
{
    tt4SnmpRouteSearch  snmp4RouteSearch;
    int                 retCode;

    tf4SnmpRouteLexGraphOrder((ttRteEntryPtr)
        (((ttSnmpCacheEntryPtr)(ttVoidPtr)nodePtr)->sceTableEntryPtr),
        &snmp4RouteSearch);
    retCode = TM_8BIT_NO;
    if (tm_memcmp(&snmp4RouteSearch,
                        genParam.genVoidParmPtr,
                        sizeof(tt4SnmpRouteSearch)) > 0)
    {
        retCode = TM_8BIT_YES;
    }
    return retCode;
}

/* Search through the IPv4 Route table to find point of insertion */
static ttNodePtr tf4SnmpRouteSearch(ttSnmpCacheEntryPtr snmpCacheEntryPtr,
                                  ttListPtr           listPtr)
{
    ttNodePtr           nodePtr;
    tt4SnmpRouteSearch  snmp4RouteSearch;
    ttGenericUnion      genParam;

    tf4SnmpRouteLexGraphOrder((ttRteEntryPtr)(snmpCacheEntryPtr->sceTableEntryPtr),
                         &snmp4RouteSearch);
    genParam.genVoidParmPtr = &snmp4RouteSearch;
    nodePtr = tfListWalk(listPtr,
                         tf4SnmpRouteCompareCB, genParam);
    return nodePtr;
}

/* Copy 2-tuple IPv4 address/port, and devIndex */
static void tf4SnmpUdpLexGraphOrder(ttTupleDevPtr       tupleDevPtr,
                                    tt4SnmpUdpTuplePtr  snmp4UdpTuplePtr)
{
    tm_bzero(snmp4UdpTuplePtr, sizeof(tt4SnmpUdpTuple));
    snmp4UdpTuplePtr->snmpuAddrFamily = tupleDevPtr->tudAddressFamily;
/* Copy local IP address and IP port in network byte order (lexicographical order)*/
    tm_ip_copy(tm_4_ip_addr(tupleDevPtr->tudTuple.sotLocalIpAddress),
               snmp4UdpTuplePtr->snmpu4LocalAddress); 
    snmp4UdpTuplePtr->snmpu4LocalPort = tupleDevPtr->tudTuple.sotLocalIpPort;
#ifdef TM_USE_STRONG_ESL
    if (tupleDevPtr->tudDevPtr != (ttDeviceEntryPtr)0)
    {
         snmp4UdpTuplePtr->snmpu4DevIndex = tupleDevPtr->tudDevPtr->devIndex;
    }
    else
    {
         snmp4UdpTuplePtr->snmpu4DevIndex = 0;
    }
#endif /* TM_USE_STRONG_ESL */
    return;
}

/*
 * tfListWalk CB function
 * Parameter is pointer to a 2-tuple IPv4 address, port
 * Copy node 4-tuple into a 2-tuple IPv4 address, port
 * Compare copied node parameter with passed parameter
 */
static int tf4SnmpUdpTupleCompareCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam)
{
    tt4SnmpUdpTuple     snmp4UdpTuple;
    int                 retCode;

    tf4SnmpUdpLexGraphOrder(
        (ttTupleDevPtr)(((ttSnmpCacheEntryPtr)(ttVoidPtr)nodePtr)->sceParmPtr),
        &snmp4UdpTuple);
    retCode = TM_8BIT_NO;
    if (tm_memcmp(&snmp4UdpTuple,
                        genParam.genVoidParmPtr,
                        sizeof(tt4SnmpUdpTuple)) > 0)
    {
        retCode = TM_8BIT_YES;
    }
    return retCode;
}

/* Search through the IPv4 UDP table to find point of insertion */
static ttNodePtr tf4SnmpUdpSearch(ttSnmpCacheEntryPtr snmpCacheEntryPtr,
                                 ttListPtr           listPtr)
{
    ttNodePtr           nodePtr;
    tt4SnmpUdpTuple     snmp4UdpTuple;
    ttGenericUnion      genParam;

    tf4SnmpUdpLexGraphOrder(snmpCacheEntryPtr->sceParmPtr, &snmp4UdpTuple);
    genParam.genVoidParmPtr = &snmp4UdpTuple;
    nodePtr = tfListWalk(listPtr,
                         tf4SnmpUdpTupleCompareCB, genParam);
    return nodePtr;
}

/* Copy 4-tuple IPv4 address/port, and devIndex */
static void tf4SnmpTcpLexGraphOrder(ttTupleDevPtr      tupleDevPtr,
                                    tt4SnmpTcpTuplePtr snmp4TcpTuplePtr)
{
    tf4SnmpUdpLexGraphOrder(tupleDevPtr,
                      (tt4SnmpUdpTuplePtr)(ttVoidPtr)snmp4TcpTuplePtr);
    tm_ip_copy(tm_4_ip_addr(tupleDevPtr->tudTuple.sotRemoteIpAddress),
               snmp4TcpTuplePtr->snmpt4RemoteAddress);
    snmp4TcpTuplePtr->snmpt4RemotePort = tupleDevPtr->tudTuple.sotRemoteIpPort;
#ifdef TM_USE_STRONG_ESL
    if (tupleDevPtr->tudDevPtr != (ttDeviceEntryPtr)0)
    {
         snmp4TcpTuplePtr->snmpt4DevIndex = tupleDevPtr->tudDevPtr->devIndex;
    }
    else
    {
         snmp4TcpTuplePtr->snmpt4DevIndex = 0;
    }
#endif /* TM_USE_STRONG_ESL */
    return;
}

/*
 * tfListWalk CB function
 * Parameter is pointer to a 4-tuple IPv4 address, port
 * Copy node 4-tuple into a 4-tuple IPv4 address, port
 * Compare copied node parameter with passed parameter
 */
static int tf4SnmpTcpTupleCompareCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam)
{
    tt4SnmpTcpTuple     snmp4TcpTuple;
    int                 retCode;

    tf4SnmpTcpLexGraphOrder(
        (ttTupleDevPtr)(((ttSnmpCacheEntryPtr)(ttVoidPtr)nodePtr)->sceParmPtr),
        &snmp4TcpTuple);
    retCode = TM_8BIT_NO;
    if (tm_memcmp(&snmp4TcpTuple,
                        genParam.genVoidParmPtr,
                        sizeof(tt4SnmpTcpTuple)) > 0)
    {
        retCode = TM_8BIT_YES;
    }
    return retCode;
}

/* Search through the IPv4 TCP table to find point of insertion */
static ttNodePtr tf4SnmpTcpSearch(ttSnmpCacheEntryPtr snmpCacheEntryPtr,
                                 ttListPtr           listPtr)
{
    ttNodePtr           nodePtr;
    tt4SnmpTcpTuple     snmp4TcpTuple;
    ttGenericUnion      genParam;

    tf4SnmpTcpLexGraphOrder(snmpCacheEntryPtr->sceParmPtr, &snmp4TcpTuple);
    genParam.genVoidParmPtr = &snmp4TcpTuple;
    nodePtr = tfListWalk(listPtr,
                         tf4SnmpTcpTupleCompareCB, genParam);
    return nodePtr;
}
#endif /* TM_USE_IPV4 */


#ifdef TM_USE_IPV6
/* Copy IPv6 address, and reset scope id  */
static void tfSnmpAddrLexGraphOrder(ttIpAddressPtr netAddrPtr,
                                    ttIpAddressPtr snmpAddrPtr)
{
    tm_6_ip_copy(netAddrPtr, snmpAddrPtr);
#ifndef TM_SINGLE_INTERFACE
    if (IN6_IS_ADDR_LINKLOCAL(netAddrPtr))
    {
/* Unscope the link-local in the copy */
        tm_6_clear_scope_id(snmpAddrPtr);
    }
#endif /* TM_SINGLE_INTERFACE */
}

/* 
 * Copy ARP entry destination address + copy device index to
 * passed ARP search structure
 */
static void tfSnmpArpLexGraphOrder(ttRteEntryPtr       rtePtr,
                                   ttSnmpArpSearchPtr  snmpArpSearchPtr)
{
    tt32Bit devIndex;

    tfSnmpAddrLexGraphOrder(&(rtePtr->rteDest),
                       &(snmpArpSearchPtr->snmpaDest));
    devIndex = rtePtr->rteDevIndex;
    tm_htonl(devIndex, snmpArpSearchPtr->snmpaDevIndex);
    return;
}
/*
 * tfListWalk CB function
 * Parameter is pointer to an ARP search structre
 * Copy ARP entry dest and device index to a local ARP
 * search structure
 * compare the 2 structures.
 */
static int tfSnmpArpCompareCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam)
{
    ttSnmpArpSearch    snmpArpSearch;
    int                 retCode;

    tfSnmpArpLexGraphOrder((ttRteEntryPtr)
        (((ttSnmpCacheEntryPtr)(ttVoidPtr)nodePtr)->sceTableEntryPtr),
        &snmpArpSearch);
    retCode = TM_8BIT_NO;
    if (tm_memcmp(&snmpArpSearch,
                        genParam.genVoidParmPtr,
                        sizeof(ttSnmpArpSearch)) > 0)
    {
        retCode = TM_8BIT_YES;
    }
    return retCode;
}

/* Search through the IPv ARP table to find point of insertion */
static ttNodePtr tfSnmpArpSearch(ttSnmpCacheEntryPtr snmpCacheEntryPtr,
                                  ttListPtr           listPtr)
{
    ttNodePtr           nodePtr;
    ttSnmpArpSearch    snmpArpSearch;
    ttGenericUnion      genParam;

    tfSnmpArpLexGraphOrder((ttRteEntryPtr)(snmpCacheEntryPtr->sceTableEntryPtr),
                        &snmpArpSearch);
    genParam.genVoidParmPtr = &snmpArpSearch;
    nodePtr = tfListWalk(listPtr,
                         tfSnmpArpCompareCB, genParam);
    return nodePtr;
}

/* 
 * Copy Route entry dest, bit offset, hops and device index
 * to the passed Route search structure
 */
static void tfSnmpRouteLexGraphOrder(ttRteEntryPtr          rtePtr,
                                     ttSnmpRouteSearchPtr  snmpRouteSearchPtr)
{
#ifdef TM_USE_STRONG_ESL
    tt32Bit temp32Bit;
#endif /* TM_USE_STRONG_ESL */

    tfSnmpAddrLexGraphOrder(&(rtePtr->rteDest),
                       &(snmpRouteSearchPtr->snmprDest));
    snmpRouteSearchPtr->snmprBitOffset = rtePtr->rteLeafNode.rtnBitOffset; 
#ifdef TM_USE_STRONG_ESL
    temp32Bit = rtePtr->rteHops;
    tm_htonl(temp32Bit, snmpRouteSearchPtr->snmprHops);
    temp32Bit = rtePtr->rteDevIndex;
    tm_htonl(temp32Bit, snmpRouteSearchPtr->snmprDevIndex);
#endif /* TM_USE_STRONG_ESL */
    return;
}
/*
 * tfListWalk CB function
 * Parameter is pointer to a Route search structre
 * Copy Route entry dest, bit offset, hops and device index to a local Route
 * search structure
 * compare the 2 structures.
 */
static int tfSnmpRouteCompareCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam)
{
    ttSnmpRouteSearch  snmpRouteSearch;
    int                 retCode;

    tfSnmpRouteLexGraphOrder((ttRteEntryPtr)
        (((ttSnmpCacheEntryPtr)(ttVoidPtr)nodePtr)->sceTableEntryPtr),
        &snmpRouteSearch);
    retCode = TM_8BIT_NO;
    if (tm_memcmp(&snmpRouteSearch,
                        genParam.genVoidParmPtr,
                        sizeof(ttSnmpRouteSearch)) > 0)
    {
        retCode = TM_8BIT_YES;
    }
    return retCode;
}

/* Search through the IPv Route table to find point of insertion */
static ttNodePtr tfSnmpRouteSearch(ttSnmpCacheEntryPtr snmpCacheEntryPtr,
                                  ttListPtr           listPtr)
{
    ttNodePtr           nodePtr;
    ttSnmpRouteSearch    snmpRouteSearch;
    ttGenericUnion      genParam;

    tfSnmpRouteLexGraphOrder((ttRteEntryPtr)(snmpCacheEntryPtr->sceTableEntryPtr),
                        &snmpRouteSearch);
    genParam.genVoidParmPtr = &snmpRouteSearch;
    nodePtr = tfListWalk(listPtr,
                         tfSnmpRouteCompareCB, genParam);
    return nodePtr;
}

/* Copy 2-tuple IPv6 address/port + devIndex */
static void tfSnmpUdpLexGraphOrder(ttTupleDevPtr      tupleDevPtr,
                                   ttSnmpUdpTuplePtr  snmpUdpTuplePtr)
{
    tfSnmpAddrLexGraphOrder(&(tupleDevPtr->tudTuple.sotLocalIpAddress),
                       &(snmpUdpTuplePtr->snmpuLocalAddress));
    snmpUdpTuplePtr->snmpuLocalPort = tupleDevPtr->tudTuple.sotLocalIpPort;
#ifdef TM_USE_STRONG_ESL
    if (tupleDevPtr->tudDevPtr != (ttDeviceEntryPtr)0)
    {
         snmpUdpTuplePtr->snmpuDevIndex = tupleDevPtr->tudDevPtr->devIndex;
    }
    else
    {
         snmpUdpTuplePtr->snmpuDevIndex = 0;
    }
#endif /* TM_USE_STRONG_ESL */
    return;
}

/*
 * tfListWalk CB function
 * Parameter is pointer to a 2-tuple IPv6 address, port 
 * Copy node 4-tuple into a 2-tuple IPv6 address, port
 * Compare copied node parameter with passed parameter
 */
static int tfSnmpUdpTupleCompareCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam)
{
    ttSnmpUdpTuple      snmpUdpTuple;
    int                 retCode;

    tfSnmpUdpLexGraphOrder(
        (ttTupleDevPtr)(((ttSnmpCacheEntryPtr)(ttVoidPtr)nodePtr)->sceParmPtr),
        &snmpUdpTuple);
    retCode = TM_8BIT_NO;
    if (tm_memcmp(&snmpUdpTuple,
                        genParam.genVoidParmPtr,
                        sizeof(ttSnmpUdpTuple)) > 0)
    {
        retCode = TM_8BIT_YES;
    }
    return retCode;
}

/* Search through the IPv6 UDP table to find point of insertion */
static ttNodePtr tfSnmpUdpSearch(ttSnmpCacheEntryPtr snmpCacheEntryPtr,
                                 ttListPtr           listPtr)
{
    ttNodePtr           nodePtr;
    ttSnmpUdpTuple      snmpUdpTuple;
    ttGenericUnion      genParam;

    tfSnmpUdpLexGraphOrder(snmpCacheEntryPtr->sceParmPtr, &snmpUdpTuple);
    genParam.genVoidParmPtr = &snmpUdpTuple;
    nodePtr = tfListWalk(listPtr,
                         tfSnmpUdpTupleCompareCB, genParam);
    return nodePtr;
}

/* Copy 4-tuple IPv6 address/port + devIndex */
static void tfSnmpTcpLexGraphOrder(ttTupleDevPtr      tupleDevPtr,
                                   ttSnmpTcpTuplePtr  snmpTcpTuplePtr)
{
    tfSnmpUdpLexGraphOrder(tupleDevPtr,
                      (ttSnmpUdpTuplePtr)(ttVoidPtr)snmpTcpTuplePtr);
    tfSnmpAddrLexGraphOrder(&(tupleDevPtr->tudTuple.sotRemoteIpAddress),
                       &(snmpTcpTuplePtr->snmptRemoteAddress));
    snmpTcpTuplePtr->snmptRemotePort = tupleDevPtr->tudTuple.sotRemoteIpPort;
#ifdef TM_USE_STRONG_ESL
    if (tupleDevPtr->tudDevPtr != (ttDeviceEntryPtr)0)
    {
         snmpTcpTuplePtr->snmptDevIndex = tupleDevPtr->tudDevPtr->devIndex;
    }
    else
    {
         snmpTcpTuplePtr->snmptDevIndex = 0;
    }
#endif /* TM_USE_STRONG_ESL */
    return;
}

/*
 * tfListWalk CB function
 * Parameter is pointer to a 4-tuple IPv6 address, port
 * Copy node 4-tuple into a 4-tuple IPv6 address, port
 * Compare converted node parameter with passed parameter
 */
static int tfSnmpTcpTupleCompareCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam)
{
    ttSnmpTcpTuple      snmpTcpTuple;
    int                 retCode;

    tfSnmpTcpLexGraphOrder(
        (ttTupleDevPtr)(((ttSnmpCacheEntryPtr)(ttVoidPtr)nodePtr)->sceParmPtr),
        &snmpTcpTuple);
    retCode = TM_8BIT_NO;
    if (tm_memcmp(&snmpTcpTuple,
                        genParam.genVoidParmPtr,
                        sizeof(ttSnmpTcpTuple)) > 0)
    {
        retCode = TM_8BIT_YES;
    }
    return retCode;
}

/* Search through the IPv6 TCP table to find point of insertion */
static ttNodePtr tfSnmpTcpSearch(ttSnmpCacheEntryPtr snmpCacheEntryPtr,
                                 ttListPtr           listPtr)
{
    ttNodePtr           nodePtr;
    ttSnmpTcpTuple      snmpTcpTuple;
    ttGenericUnion      genParam;

    tfSnmpTcpLexGraphOrder(snmpCacheEntryPtr->sceParmPtr, &snmpTcpTuple);
    genParam.genVoidParmPtr = &snmpTcpTuple;
    nodePtr = tfListWalk(listPtr,
                         tfSnmpTcpTupleCompareCB, genParam);
    return nodePtr;
}
#endif /* TM_USE_IPV6 */

/*
 * tfSnmpdCacheInsertPrefix function description:
 * cache prefixes of interfaces
 * 
 * Parameters
 * Parameters  Description
 * addrFamily  address family
 * devPtr      pointer to the device entry
 * mHomeIndex  multihome index
 *
 * Returns
 * Value            Description
 * TM_ENOERROR      no error
 * not TM_ENOERROR  error
 */
int tfSnmpdCacheInsertPrefix(tt8Bit            addrFamily,
                             ttDeviceEntryPtr  devPtr,
                             tt16Bit           mHomeIndex)
{
    ttSnmpPrefixCachePtr  newCachePtr;
    int                   errorCode;

    errorCode = TM_ENOERROR;
#ifdef TM_ERROR_CHECKING
    if (((addrFamily != AF_INET) && (addrFamily != AF_INET6))
        || (devPtr == (ttDeviceEntryPtr)0)
        || (mHomeIndex < 0))
    {
        errorCode = TM_EINVAL;
        goto tfSnmpdCacheInsertPrefixExit;
    }
#endif /* TM_ERROR_CHECKING */

    newCachePtr =
        (ttSnmpPrefixCachePtr)tm_malloc(sizeof(ttSnmpPrefixCache));
    if (newCachePtr == (ttSnmpPrefixCachePtr)0)
    {
        errorCode = TM_ENOBUFS;
        goto tfSnmpdCacheInsertPrefixExit;
    }
    tm_bzero(newCachePtr, sizeof(ttSnmpPrefixCache));
    newCachePtr->snmpAddrFamily = addrFamily;
    newCachePtr->snmpDevPtr = devPtr;
    newCachePtr->snmpMHomeIndex = mHomeIndex;

/* Store cache index for deletion */
    newCachePtr->snmpCacheEntry.sceTableIndex = TM_SNMPC_PREFIX_INDEX;
    newCachePtr->snmpCacheEntry.sceTableEntryPtr = (ttVoidPtr)newCachePtr;
    newCachePtr->snmpCacheEntry.sceParmPtr = (ttVoidPtr)newCachePtr;
    newCachePtr->snmpCacheEntry.sceTableEntryType = TM_SNMPC_PREFIX_TYPE;
    tfSnmpdCacheInsertEntry(&(newCachePtr->snmpCacheEntry));

tfSnmpdCacheInsertPrefixExit:
    return errorCode;
} /* int tfSnmpdCacheInsertPrefix() */

/* 
 * Copy Prefix entry device index in network byte order, address family,
 * prefix, prefix length to passed Prefix search structure
 */
static void tfSnmpPrefixLexGraphOrder(ttSnmpPrefixCachePtr   prefixCachePtr,
                                      ttSnmpPrefixSearchPtr  snmpPrefixSearchPtr)
{
    ttDeviceEntryPtr    devPtr;
    tt32Bit             devIndex;
#ifdef TM_USE_IPV4
    tt4IpAddress        intfIpAddr;
    tt4IpAddress        intfNetMask;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    ttIpAddress         prefix;
    int                 maskIndex;
#endif /* TM_USE_IPV6 */
    tt16Bit             mHomeIndex;

    tm_bzero(snmpPrefixSearchPtr, sizeof(ttSnmpPrefixSearch));
    devPtr = prefixCachePtr->snmpDevPtr;
    mHomeIndex = prefixCachePtr->snmpMHomeIndex;
    devIndex = devPtr->devIndex;
    tm_htonl(devIndex, snmpPrefixSearchPtr->snmppDevIndex);
    snmpPrefixSearchPtr->snmppAddrFamily = prefixCachePtr->snmpAddrFamily;
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
    if (prefixCachePtr->snmpAddrFamily == AF_INET)
#endif /* TM_USE_IPV6 */
    {
        tm_ip_copy(tm_ip_dev_addr(devPtr, mHomeIndex), intfIpAddr);
        tm_ip_copy(tm_ip_dev_mask(devPtr, mHomeIndex), intfNetMask);
        tm_ip_copy(tm_ip_net(intfIpAddr, intfNetMask),
                            snmpPrefixSearchPtr->snmpp4Prefix);
        snmpPrefixSearchPtr->snmpp4PrefixLen =
                            tm_4_ip_dev_prefixLen(devPtr, mHomeIndex);
    }
#ifdef TM_USE_IPV6
    else
#endif /* TM_USE_IPV6 */
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    {
        tm_6_ip_copy_structs(tm_6_ip_dev_addr(devPtr, mHomeIndex), prefix);
        tm_6_ip_net_prefix((&prefix),
                            devPtr->dev6PrefixLenArray[mHomeIndex],
                            maskIndex);
        tm_6_ip_copy_structs(prefix, snmpPrefixSearchPtr->snmppPrefix);
        snmpPrefixSearchPtr->snmppPrefixLen =
                            devPtr->dev6PrefixLenArray[mHomeIndex];
    }
#endif /* TM_USE_IPV6 */
    return;
}

/*
 * tfListWalk CB function
 * Parameter is pointer to a Prefix search structre
 * Copy Prefix entry device index, address family, prefix and 
 * prefix length to a local Prefix search structure
 * compare the 2 structures.
 */
static int tfSnmpPrefixCompareCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam)
{
    ttSnmpPrefixSearch  snmpPrefixSearch;
    int                 retCode;

    tfSnmpPrefixLexGraphOrder((ttSnmpPrefixCachePtr)
        (((ttSnmpCacheEntryPtr)(ttVoidPtr)nodePtr)->sceTableEntryPtr),
        &snmpPrefixSearch);
    retCode = TM_8BIT_NO;
    if (tm_memcmp(&snmpPrefixSearch,
                        genParam.genVoidParmPtr,
                        sizeof(ttSnmpPrefixSearch)) > 0)
    {
        retCode = TM_8BIT_YES;
    }
    return retCode;
}

/* Search through the IPv4,6 Prefix table to find point of insertion */
static ttNodePtr tfSnmpPrefixSearch(ttSnmpCacheEntryPtr snmpCacheEntryPtr,
                                    ttListPtr           listPtr)
{
    ttNodePtr           nodePtr;
    ttSnmpPrefixSearch  snmpPrefixSearch;
    ttGenericUnion      genParam;

    tfSnmpPrefixLexGraphOrder((ttSnmpPrefixCachePtr)
        (snmpCacheEntryPtr->sceTableEntryPtr), &snmpPrefixSearch);
    genParam.genVoidParmPtr = &snmpPrefixSearch;
    nodePtr = tfListWalk(listPtr,
                         tfSnmpPrefixCompareCB, genParam);
    return nodePtr;
}

/*
 * tfSnmpdCacheDeletePrefix function description:
 * . Delete entry from null terminated doubly linked list pointed to by cache
 *   head.
 *
 * Parameters
 * Parameters  Description
 * addrFamily  address family
 * devPtr      pointer to the device entry
 * mHomeIndex  multihome index
 *
 * Returns
 * Value            Description
 * TM_ENOERROR      no error
 * not TM_ENOERROR  error
 */
int tfSnmpdCacheDeletePrefix(tt8Bit            addrFamily,
                             ttDeviceEntryPtr  devPtr,
                             tt16Bit           mHomeIndex)
{
    ttSnmpdCacheHeadPtr   cacheHeadPtr;
    ttSnmpPrefixCachePtr  cachedPtr;
    ttListPtr             listPtr;
    ttNodePtr             nodePtr;
    ttGenericUnion        genParam;
    ttSnmpPrefixDelSearch snmpPrefixDelSearch;
    int                   errorCode;

    errorCode = TM_ENOERROR;
#ifdef TM_ERROR_CHECKING
    if (((addrFamily != AF_INET) && (addrFamily != AF_INET6))
        || (devPtr == (ttDeviceEntryPtr)0)
        || (mHomeIndex < 0))
    {
        errorCode = TM_EINVAL;
        goto tfSnmpdCacheDeletePrefixExit;
    }
#endif /* TM_ERROR_CHECKING */

    tm_bzero(&snmpPrefixDelSearch, sizeof(ttSnmpPrefixDelSearch));
    snmpPrefixDelSearch.snmpdDevPtr = devPtr;
    snmpPrefixDelSearch.snmpdMHomeIndex = mHomeIndex;
    snmpPrefixDelSearch.snmpdAddrFamily = addrFamily;
    genParam.genVoidParmPtr = &snmpPrefixDelSearch;
    cacheHeadPtr = &(tm_context(tvSnmpdCaches)[TM_SNMPC_PREFIX_INDEX]);
#ifdef TM_LOCK_NEEDED
/* Lock the cache (to lock out SNMP cache lookup) */
    tm_call_lock_wait(&(cacheHeadPtr->snmpcLockEntry));
#endif /* TM_LOCK_NEEDED */
    listPtr = &(cacheHeadPtr->snmpcList);
    nodePtr = tfListWalk(listPtr,
                         tfSnmpPrefixCompDelCB, genParam);
#ifdef TM_LOCK_NEEDED
/* Unlock the cache (to allow SNMP lookup) */
    tm_call_unlock(&(cacheHeadPtr->snmpcLockEntry));
#endif /* TM_LOCK_NEEDED */
    if (nodePtr != (ttNodePtr)0)
    {
        cachedPtr = ((ttSnmpPrefixCachePtr)
            ((ttSnmpCacheEntryPtr)(ttVoidPtr)nodePtr)->sceTableEntryPtr);
        tfSnmpdCacheDeleteEntry(&(cachedPtr->snmpCacheEntry));
        tm_free(cachedPtr);
    }
#ifdef TM_ERROR_CHECKING
tfSnmpdCacheDeletePrefixExit:
#endif /* TM_ERROR_CHECKING */
    return errorCode;
} /* int tfSnmpdCacheDeletePrefix() */

/*
 * tfListWalk CB function
 * Parameter is pointer to a Prefix delete search structre
 * Prefix entry device pointer, address family and multihome
 * index to a local Prefix search structure
 * compare the 2 structures.
 */
static int tfSnmpPrefixCompDelCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam)
{
    ttSnmpPrefixDelSearch   snmpPrefixDelSearch;
    ttSnmpPrefixCachePtr    cachedPtr;
    int                     retCode;

    cachedPtr = ((ttSnmpPrefixCachePtr)
        ((ttSnmpCacheEntryPtr)(ttVoidPtr)nodePtr)->sceTableEntryPtr);
    tm_bzero(&snmpPrefixDelSearch, sizeof(ttSnmpPrefixDelSearch));
    snmpPrefixDelSearch.snmpdDevPtr = cachedPtr->snmpDevPtr;
    snmpPrefixDelSearch.snmpdMHomeIndex = cachedPtr->snmpMHomeIndex;
    snmpPrefixDelSearch.snmpdAddrFamily = cachedPtr->snmpAddrFamily;
    retCode = TM_8BIT_NO;
    if (tm_memcmp(&snmpPrefixDelSearch,
                        genParam.genVoidParmPtr,
                        sizeof(ttSnmpPrefixDelSearch)) == 0)
    {
        retCode = TM_8BIT_YES;
    }
    return retCode;
}

#else /* TM_SNMP_CACHE */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_SNMP_CACHE is not defined */
int tvSnmpdCacheDummy = 0;

#endif /* TM_SNMP_CACHE */
