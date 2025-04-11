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
 * Description:     tfRawSocket() API + tf4RawIncomingPacket()
 * Filename:        trrawsoc.c
 * Author:          Odile
 * Date Created:    07/26/00
 * $Source: source/sockapi/trrawsoc.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:43:59JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h> 
#include <trmacro.h>

#if (defined(TM_USE_RAW_SOCKET) && defined(TM_USE_IPV4))

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/*
 * tfRawSocket function Description:
 * Create a raw socket. A raw socket enables the user to send or
 * receive data just above the IP layer.
 * Given a transport layer protocol and IP address, returns a raw socket
 * bound to TM_RAW_IP_PORT, and ipAddress. Note that ipAddress can be
 * a zero IP address, if the user does not want to bind to a specific
 * IP address.
 * The user can use this API, only if TM_USE_RAW_SOCKET has
 * been defined in trsystem.h.
 * Example of tfRawSocket() usage is shown in the loop back test
 * module txscatlp.c.
 * Parameters
 * ipAddress        User IP address
 * protocol         protocol for example:
 *                   . IPPROTO_IGMP
 *                   . IPPROTO_OSPF
 *                   . IPPROTO_ICMP
 * Return value
 * >= 0             success
 * TM_SOCKET_ERROR  failure.
 *
 * In case of failure, error code can be obtained by calling
 * tfGetSocketError(TM_SOCKET_ERROR):
 *      TM_EINVAL       Invalid protocol for raw socket send/receive.
 *                      The following protocols are disallowed:
 *                          IPPROTO_UDP
 *                          IPPROTO_TCP
 *      TM_EINVAL       There is no interface configured with the specified
 *                      IP address (only if ipAddress parameter is not zero.)
 *      TM_EADDRINUSE   A raw socket has already been opened for this protocol
 *      TM_EBADF        Invalid socket descriptor
 */
int tfRawSocket(ttUserIpAddress ipAddress, int protocol)
{
    int                 socketDescriptor;
    int                 errorCode;
    struct sockaddr_in  addr;
    ttSockAddrPtrUnion  tempSockAddr;
#if (defined(TM_USE_REUSEADDR_LIST) && defined(TM_USE_IPV6))
    int                 reuseAddr;
#endif /* TM_USE_REUSEADDR_LIST && dual stack */

    tempSockAddr.sockInPtr = &addr;
/* Check parameter */
/*
 * Set return value to TM_SOCKET_ERROR, and error code
 * to TM_EINVAL, if protocol is UDP, or TCP. Otherwise allow the raw socket
 * open.
 */
    if ( (protocol == IPPROTO_TCP) || (protocol == IPPROTO_UDP) )
    {
        socketDescriptor = TM_SOCKET_ERROR;
        errorCode = TM_EINVAL;
    }
    else
    {
/* Create a raw socket of type given by the user */
        socketDescriptor = socket(PF_INET, SOCK_RAW, protocol);
        errorCode = TM_ENOERROR;
        if (socketDescriptor != TM_SOCKET_ERROR)
        {
#if (defined(TM_USE_REUSEADDR_LIST) && defined(TM_USE_IPV6))

/* just in case the user wants to use both IPv4 and IPv6 raw sockets with the
   same protocol, enable the socket reuse list for this raw socket */
            reuseAddr = 1;
            errorCode = setsockopt(
                socketDescriptor, 
                SOL_SOCKET, 
                SO_REUSEADDR, 
                (char *) &reuseAddr, 
                sizeof(reuseAddr));
#endif /* TM_USE_REUSEADDR_LIST && dual stack */

/*
 * Bind to raw socket port.
 */
            addr.sin_len = sizeof(struct sockaddr_in);
            addr.sin_family = PF_INET;
            addr.sin_port = TM_RAW_IP_PORT;
            tm_ip_copy(ipAddress, addr.sin_addr.s_addr);
            errorCode = bind( socketDescriptor,
                              tempSockAddr.sockPtr,
                              sizeof(struct sockaddr_in));
            if (errorCode == TM_SOCKET_ERROR)
            {
/* Retrieve the socket error for failed bind call */
                errorCode = tfGetSocketError(socketDescriptor);
                socketDescriptor = TM_SOCKET_ERROR;
                (void)tfClose(socketDescriptor);
            }
        }
    }
    if (errorCode != TM_ENOERROR)
    {
        tfSocketErrorRecord(TM_SOCKET_ERROR, errorCode);
    }
    return socketDescriptor;
}
    
/*
 * tf4RawIncomingPacket function description:
 * Called from tfIpIncomingPacket() when an IP datagram comes from the
 * network, and when the IP header ULP protocol is neither
 * TCP, nor UDP.
 * When tf4RawIncomingPacket is called the packetPtr link data pointer
 * points to the IP header of the incoming IP datagram.
 * . Save the ulp (obtained from iphPtr->iphUlp).
 * . Check a one socket cache to retrieve the socket entry pointer for
 *   the incoming data:
 * . Lock the cache.
 * . If cache is empty, or cache contains a raw socket for a different
 *   protocol:
 *     .. call tf4SocketLookup()
 *     .. Cache socketPtr in local cache if a raw socket for the ULP
 *        protocol is found.
 * . Unlock the cache
 * . if there is a socket, queue to the socket by calling
 *   tfSocketIncomingPacket.
 * . If tfSocketIncomingPacket did not fail, notify the socket of
 *   the received data:
 * . If packet did not get queued, then free the packet
 * Parameters
 * Parameter    Description
 * packetPtr    Pointer to ttPacket containing the raw ULP incoming data.
 *              packetPtr->pktLinkDataPtr points to the IP header,
 *              packetPtr->pktTotalLength is the length of the IP packet.
 *              packetPtr->pktSpecDestIpAddr contains the specific IP
 *              destination address, and packetPtr->pktSrcIpAddr
 *              contains the source address of the IP datagram.
 *              packetPtr->pktFlags indicates whether the destination
 *              IP address is broadcast, unicast or multicast
 *              packetPtr->pktIpHdrLen IP header length in bytes
 *              (so that we know whether there are options)
 */
void tf4RawIncomingPacket(ttPacketPtr packetPtr)
{
    ttSocketEntryPtr  socketPtr;
    ttSocketEntryPtr  purgeCacheSocketPtr;
    ttIpHeaderPtr     iphPtr;
    ttTupleDev        tupleDev;
    int               errorCode;
    tt16Bit           flag;
    tt8Bit            ulp;
    tt8Bit            needFree;

    purgeCacheSocketPtr = (ttSocketEntryPtr)0;
/* assume failure to queue the packet to a raw socket */
    needFree = TM_8BIT_YES;
    socketPtr = (ttSocketEntryPtr)0; /* to avoid compiler warning */
    iphPtr = (ttIpHeaderPtr)(packetPtr->pktLinkDataPtr);
/* protocol */
    ulp = iphPtr->iphUlp;
/* Check on cache first */
/* LOCK the cache */
    tm_lock_wait(&tm_context(tvRawCacheLock));
    socketPtr = tm_context(tvRawSocketPtr);
    if (socketPtr != TM_SOCKET_ENTRY_NULL_PTR)
    {
/* LOCK the socket entry in the cache */
        tm_lock_wait(&socketPtr->socLockEntry);
/* Check that is is ours */
        if (socketPtr->socProtocolNumber == ulp)
        {
/* Ours (only one raw socket per ULP): incoming thread has ownership */
            tm_socket_checkout(socketPtr);
        }
        else
        {
/*
 * Keep cache locked.
 * Unlock tvRawSocketPtr.
 */
            tm_unlock(&socketPtr->socLockEntry);
            goto rawLookup;
        }
    }
    else
    {
/* If cache failed, lookup the socket tree */
rawLookup:
        if (tm_16bit_one_bit_set(packetPtr->pktFlags,
                                 TM_IP_MULTICAST_FLAG | TM_IP_BROADCAST_FLAG))
        {
            flag = TM_SOC_TREE_BCAST_MATCH;
        }
        else
        {
            flag = TM_16BIT_ZERO;
        }
        tm_bzero(&tupleDev, sizeof(tupleDev));
        tupleDev.tudLocalIpPort = TM_RAW_IP_PORT; /* Local Ip port */
#ifdef TM_USE_IPV6
        tm_6_addr_to_ipv4_mapped(iphPtr->iphDestAddr, /* Local IP address */
                                 &(tupleDev.tudLocalIpAddress));
#else /* !TM_USE_IPV6 */
        tm_ip_copy(iphPtr->iphDestAddr, /* Local IP address */
                   tupleDev.tudLocalIpAddress);
#endif /* TM_USE_IPV6 */
#if (defined(TM_USE_STRONG_ESL) || defined(TM_IGMP) || defined(TM_6_USE_MLD))
        tupleDev.tudDevPtr = packetPtr->pktDeviceEntryPtr;
#endif /* TM_USE_STRONG_ESL || TM_IGMP || TM_6_USE_MLD */
        tupleDev.tudProtocolNumber = ulp;
        tupleDev.tudAddressFamily = AF_INET;
        socketPtr = tfSocketLookup(&tupleDev, flag);
/* If there is no socket at destination port */
        if (socketPtr == TM_SOCKET_ENTRY_NULL_PTR)
        {
/* UNLOCK the cache */
            tm_call_unlock(&tm_context(tvRawCacheLock));
            goto outRawIncoming;
        }
        else
        {
            if (socketPtr != tm_context(tvRawSocketPtr))
/* socketPtr non null, update the cache if socketPtr not in the cache. */
            {
/*
 * If there is a cached socket that is different, remember to check it in 
 * later when we no longer have a lock on the current socket.
 */
                purgeCacheSocketPtr = tm_context(tvRawSocketPtr);
/* Initialize the cache */
                tm_context(tvRawSocketPtr) = socketPtr;
/* Checkout socket, since it is kept in our cache */
                tm_socket_checkout(socketPtr);
            }
        }
    }
/* UNLOCK the cache */
    tm_unlock(&tm_context(tvRawCacheLock));
#ifdef TM_LINT
LINT_BOOLEAN_BEGIN
#endif /* TM_LINT */
    if (socketPtr != TM_SOCKET_ENTRY_NULL_PTR)
#ifdef TM_LINT
LINT_BOOLEAN_END
#endif /* TM_LINT */
    {
        if (    (   (!(socketPtr->socFlags & TM_SOCF_CONNECTED))
/* If socket is connected only allow data from peer address */
                 || (tm_ip_match( packetPtr->pktSrcIpAddress,
                          tm_4_ip_addr(socketPtr->socPeerIpAddress) ) ) )
             && (   (!(socketPtr->socFlags & TM_SOCF_BIND))
/* If socket is bound only allow data to local IP address */
                 || (tm_ip_zero(tm_4_ip_addr(socketPtr->socOurIfIpAddress)))
                 || (tm_ip_match( iphPtr->iphDestAddr,
                          tm_4_ip_addr(socketPtr->socOurIfIpAddress) ) ) ) )
        {
            needFree = TM_8BIT_ZERO;
            errorCode = tfSocketIncomingPacket(socketPtr, packetPtr);
/*
 * Now that we have queued the data post to any pending thread
 * (recv or select) / CB user.
 */
            if (errorCode == TM_ENOERROR)
            {
                tfSocketNotify(socketPtr, TM_SELECT_READ, TM_CB_RECV, 0);
            }
        }
/* Incoming thread no longer has ownership of the socket. */
        tm_socket_checkin_unlock(socketPtr);
        if (purgeCacheSocketPtr != (ttSocketEntryPtr)0)
        {
/*
 * Previously cached socket was replaced, check it in.
 */
            tm_lock_wait(&(purgeCacheSocketPtr->socLockEntry));
            tm_socket_checkin_unlock(purgeCacheSocketPtr);
        }
    }
outRawIncoming:
    if (needFree != TM_8BIT_ZERO)
    {
/* socket was not locked, or has just been unlocked */
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    }
    return;
}

#else /* !TM_USE_RAW_SOCKET or !TM_USE_IPV4 */
/* Suppress PC-LINT compile error for unused header files */
#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_RAW_SOCKET is not defined */
int tvRawSocDummy = 0;
#endif /* !TM_USE_RAW_SOCKET or !TM_USE_IPV4 */
