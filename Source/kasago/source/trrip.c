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
 * Description: RIP functions
 * Filename: trrip.c
 * Author: Odile
 * Date Created: 01/13/98
 * $Source: source/trrip.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:48:22JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_IPV4

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/* Define PC-LINT compile options for unrefereced macros */
#ifdef TM_LINT
LINT_UNREF_MACRO(TM_RIP_MAX_DATA_LEN)
LINT_UNREF_MACRO(TM_RIP_AUTH)
LINT_UNREF_MACRO(TM_RIP_QUERY)
#endif /* TM_LINT */

/*
 * Local macros
 */
#define TM_RIP_REQUEST      1 /* RIP request command */
#define TM_RIP_RESPONSE     2 /* RIP response command */

#define TM_RIP_HEADER_LEN   4
#define TM_RIP_ENTRY_LEN    20
/* Minimum RIP data length of a RIP packet: header + one entry */
#define TM_RIP_MIN_DATA_LEN (TM_RIP_HEADER_LEN + TM_RIP_ENTRY_LEN)
#define TM_RIP_MAX_DATA_LEN 512  /* Maximum RIP data length of a RIP packet */

#define TM_RIP_VERS_1       1 /* RIP version 1 */
#define TM_RIP_VERS_2       2 /* RIP version 2 */

#define TM_RIP_AUTH         0xFFFFU
#define TM_RIP_MAX_ENTRIES  25

#define TM_RIP_QUERY        8 /* Treck extension. Query for router info */

#define TM_RIP_PF_INET      tm_const_htons(PF_INET)

#define TM_RIP_N_INF_METRIC tm_const_htonl(TM_UL(16))

#define TM_RIP_UDP_PORT     tm_const_htons(520)

/*
 * Local types
 */
/* Network headers */
typedef struct tsRipHeader {
    tt8Bit      ripCommand; /* 1 request, 2 response */
    tt8Bit      ripVersion; /* 1 version 1, 2 version 2 */
    tt16Bit     ripUnused; /* zero for RIP 1 */
} ttRipHeader;

typedef struct tsRipAddr {
    tt16Bit     ripaAFIdent;
    tt16Bit     ripaRouteTag; /* zero for RIP 1 */
    tt4IpAddress ripaIpAddr;
    tt4IpAddress ripaNetMask; /* zero for RIP 1 */
    tt4IpAddress ripaNextHop; /* zero for RIP 1 */
    tt32Bit     ripaMetric;
} ttRipAddr;

#ifdef TM_LINT
LINT_UNREF_MEMBER_BEGIN
#endif /* TM_LINT */
typedef struct tsRipAuth { 
    tt16Bit     ripaAuth;  /* overlays ripaAFIdent,  == TM_RIP_AUTH */
    tt16Bit     ripaAuthType;  /* Authentication type,  == ntohs(2) */
    tt8Bit      ripaPassword[16];  /* authentication password */
} ttRipAuth;
#ifdef TM_LINT
LINT_UNREF_MEMBER_END
#endif /* TM_LINT */

/* END of network headers */

typedef ttRipHeader TM_FAR * ttRipHeaderPtr;
typedef ttRipAddr   TM_FAR * ttRipAddrPtr;

/*
 * Local functions
 */
static void tfRipRecvCB(int socketDescriptor,
                        int socketCBFlags);

/*
 * tfUseRip() Function Description
 * Called by the user, when he wants to turn on RIP.
 * If a UDP RIP socket has not been opened yet, we open it, bind it to port
 * 520, set the socket ttl to 0, and register a receive call back function
 * tfRipRecvCB() which will be called every time a RIP packet is received.
 * No parameter.
 * Return value is error (for either socket(), bind(), setsockopt(), or
 * register receive call back), or 0 if successfull.
 */

int tfUseRip (void)
{
     char                ttl;
     int                 errorCode;
auto struct  sockaddr_in addr;
     ttSockAddrPtrUnion  tempSockAddr;

    if (tm_context(tvRipSocketDescriptor) == TM_SOCKET_ERROR)
    {
        tm_context(tvRipSocketDescriptor) =
                                    socket(PF_INET, SOCK_DGRAM, IP_PROTOUDP);
        if (tm_context(tvRipSocketDescriptor) != TM_SOCKET_ERROR)
        {
            addr.sin_family = PF_INET;
            addr.sin_port = TM_RIP_UDP_PORT; /* network byte order */
            tm_ip_copy(TM_IP_ZERO, addr.sin_addr.s_addr);
            tempSockAddr.sockInPtr = &addr;
            errorCode = bind( tm_context(tvRipSocketDescriptor),
                              tempSockAddr.sockPtr,
                              sizeof(struct sockaddr_in) );
            if (errorCode != TM_SOCKET_ERROR)
            {
                ttl = TM_IP_NO_FORW_TTL; /* Multicast time to live */
                errorCode = setsockopt( tm_context(tvRipSocketDescriptor),
                                        IP_PROTOIP, IPO_TOS,
                                        (const char TM_FAR *)&ttl,
                                        sizeof(unsigned char) );
                if (errorCode != TM_SOCKET_ERROR)
                {
                    errorCode = tfRegisterSocketCB(
                                            tm_context(tvRipSocketDescriptor),
                                            tfRipRecvCB,
                                            TM_CB_RECV );

                    if (errorCode != TM_SOCKET_ERROR)
                    {
                        tm_context(tvRipOn) = TM_8BIT_YES;
                    }
                    else
                    {
#ifdef TM_ERROR_CHECKING
                        tfKernelError("tfUseRip",
                                "Failed to register recv call back function");
#endif /* TM_ERROR_CHECKING */
                        goto ripSockClose;
                    }
                }
                else
                {
#ifdef TM_ERROR_CHECKING
                    tfKernelError("tfUseRip",
                              "Failed to set socket IP TTL");
#endif /* TM_ERROR_CHECKING */
                    goto ripSockClose;
                }
            }
            else
            {
#ifdef TM_ERROR_CHECKING
                tfKernelError("tfUseRip", "Failed to bind UDP RIP socket");
#endif /* TM_ERROR_CHECKING */
ripSockClose:
                (void)tfClose(tm_context(tvRipSocketDescriptor));
                tm_context(tvRipSocketDescriptor) = TM_SOCKET_ERROR;
ripRetErr:
                errorCode = tfGetSocketError(
                                          tm_context(tvRipSocketDescriptor) );
            }
        }
        else
        {
#ifdef TM_ERROR_CHECKING
            tfKernelError("tfUseRip", "Failed to open UDP RIP socket");
#endif /* TM_ERROR_CHECKING */
            errorCode = TM_SOCKET_ERROR;
            goto ripRetErr;
        }
    }
    else
    {
        errorCode = TM_EALREADY;
    }
    return errorCode;
}

/*
 * tfRipRecvCB() Function Description
 * 1) Process the RIP packet only if user has turned on RIP and RIP receive
 *    and silently ignore the packet otherwise.
 * 2) Check that data length covers at least the RIP header length 
 *    and silently ignore the packet otherwise.
 * 3) Check that our receive mode is compatible with the RIP version
 *    retrieved from the RIP header, and silently ignore the packet
 *    otherwise.
 * 4) If retrieved version is RIP Version 1, check "must be zero"
 *    unused header field, and silently ignore the packet if non zero.
 * 5) Process response:
 *    . Check that RIP source port is tm_const_htons(520), that the source
 *      of the RIP packet is not ours, and source is on our network, and
 *      silently ignore RIP packet from invalid source 
 *    . In a loop process each RIP entry:
 *        .. We skip RIP version 2 authorization field.
 *        .. Only process AF_INET entries.
 *        .. Make sure that metric field is valid. (Should be less or
 *           equal to 16, and for RIP Version 1: enforce "must be zero"
 *           header fields.
 *        .. Validate IP address field (destination IP address):
 *           Ip Address cannot be class D or E, loop back, or net 0
 *           except for a zero address (for default entry)
 *        .. Check nextHop (gateway) (retrieved from the RIP header).
 *           If nextHop is zero (RIP version 1 and some cases of RIP
 *           version 2) use peerIpAddress as nextHop. If nextHop is non
 *           zero (some cases of RIP version 2 only), validate next hop
 *           the same way we validated the peer address. If it matches
 *           our incoming interface or is not on our incoming network
 *           use peerIpAddress as nextHop instead.
 *        .. Compute destination netMask: If the subnet mask is not
 *           set in the RIP header (RIP version 1 always, RIP version 2
 *           sometimes) and the destination IpAddr is not a default entry,
 *           we need to compute according to the RFC 1058 rules:
 *           "routes to a subnet must not be sent outside the network of
 *           which the subnet is a part", so if we have a network match on
 *           our incoming interface, we can use that subnet, otherwise if
 *           we do not have a network match on our incoming interface with
 *           ipAddr, we will use the class mask of ipAddr and "as a subnet
 *           number would be useless without the subnet mask", ipAddr is
 *           "assumed to represent a host if" it "has non-zero host part",
 *           so if ipAddr has any host bit set, the netmask is a host mask
 *           (all ones).
 *        .. Update metric, following RFC1058 rules. For RIP, every
 *           interface has same metric of one. Increase metric by one,
 *           if metric has not reached infinity yet (16).
 *        .. Update the routing table with the retrieved information by
 *           calling tfRtRip(devPtr, mhomeIndex, ipAddr, netMask, nextHop,
                             metric, ripAddrPtr->ripaRouteTag);
 * 6) Ignore Request: we are silent, and do not process RIP requests.
 *    (Note that RFC1058, specifies that even silent RIP hosts should
 *     respond to RIP request when the source port is not 520, but we don't
 *     for this release. We will in a next release.)
 * 7) Free the incoming packet.
 *
 * Parameters:
 * socketDescriptor socketDescritor of the RIP socket opened in tfUseRip()
 * bufferLen        length of the RIP header and data.
 * packetPtr        Pointer to incoming ttPacket. To be freed by us.
 *                  We also retrieve pktSpecDestIpAddr, pktDeviceEntryPtr,
 *                  pktMhomeIndex
 * bufferPtr        pointer to the data in packet
 * ourIpAddr        Ip address targeted by the sender
 * peerIpAddr       IP address of the source of the message
 * peerIpPort       IP port of the source of the message
 *
 */
static void TM_CODE_FAR tfRipRecvCB(int socketDescriptor,
                                    int socketCBFlags)
/* length of RIP packet */
{
     int                 bufferLen;
     ttPacketPtr         packetPtr;
/* point to RIP header */
     ttCharPtr           bufferPtr;
     tt4IpAddress        ourIpAddr;
     struct sockaddr_in  fromAddress;
     int                 fromAddressLength;
     tt4IpAddress        peerIpAddr;
     ttIpPort            peerIpPort;
#define lRipHdrPtr    ((ttRipHeaderPtr)bufferPtr)
     ttRipAddrPtr        ripAddrPtr;
     ttDeviceEntryPtr    devPtr;
     tt4IpAddress        ourNetMask;
     tt4IpAddress        ipAddr;
     tt4IpAddress        netMask;
     tt4IpAddress        nextHop;
     tt32Bit             metric;
     int                 intMetric;
     int                 i;
     tt8Bit              ripVersion;
#ifdef TM_ERROR_CHECKING
     tt8Bit              invalidIpAddress;
#endif /* TM_ERROR_CHECKING */
auto tt16Bit             mhomeIndex;
     ttSockAddrPtrUnion  tempSockAddr;


    TM_UNREF_IN_ARG(socketCBFlags);
    fromAddressLength = sizeof(struct sockaddr);
#define TM_MAX_RIP_LEN  4096
    tempSockAddr.sockInPtr = &fromAddress;
    bufferLen = tfZeroCopyRecvFrom(
            socketDescriptor,
            (ttUserMessage TM_FAR *)(ttVoidPtr)&packetPtr,
            (char TM_FAR * TM_FAR *)&bufferPtr,
            (int)TM_MAX_RIP_LEN,
            MSG_DONTWAIT,
            tempSockAddr.sockPtr,
            &fromAddressLength);
#ifdef TM_ERROR_CHECKING
    if (bufferLen == TM_SOCKET_ERROR)
    {
        tfKernelError("tfRipRecvCB", "No RIP packet");
        return;
    }
#endif /* TM_ERROR_CHECKING */
/* Process the packet only if user has turned on RIP and RIP receive */
    if (    (tm_context(tvRipOn) != TM_8BIT_ZERO)
         && (tm_context(tvRipRecvMode) != TM_RIP_NONE) )
    {
        ripVersion = lRipHdrPtr->ripVersion;
/* Check that data length covers at least the RIP header length */
        if (bufferLen >= TM_RIP_HEADER_LEN)
        {
/*
 * Check that our receive mode is compatible with the RIP version
 * retrieved from the RIP header
 */
            if (   (    (ripVersion  == TM_RIP_VERS_2)
                     && tm_8bit_bits_not_set( tm_context(tvRipRecvMode),
                                              TM_RIP_2 ) )
                || (    (ripVersion == TM_RIP_VERS_1)
                     && (   tm_8bit_bits_not_set( tm_context(tvRipRecvMode),
                                                  TM_RIP_1 )
/* RIP Version 1. Enforce "must be zero" unused header field. */
                         || (lRipHdrPtr->ripUnused != TM_16BIT_ZERO) ) ) )
            {
                goto outRipRecv;
            }
/* Process response */
            if (lRipHdrPtr->ripCommand == TM_RIP_RESPONSE)
            {
/*
 * Given the packet specific destination address, retrieve interface
 * and multi home index to get our network mask
 */
                peerIpPort = fromAddress.sin_port;
                tm_ip_copy(fromAddress.sin_addr.s_addr, peerIpAddr);
#ifdef TM_USE_IPV6
                tm_ip_copy(
                    packetPtr->pktSharedDataPtr->
                    dataSpecDestIpAddress.s6LAddr[3], ourIpAddr);
#else /* ! TM_USE_IPV6 */
                tm_ip_copy(
                    packetPtr->pktSharedDataPtr->dataSpecDestIpAddress,
                    ourIpAddr);
#endif /* ! TM_USE_IPV6 */
 
#ifdef TM_SINGLE_INTERFACE_HOME
                devPtr = packetPtr->pktDeviceEntryPtr;
                mhomeIndex = TM_16BIT_ZERO;
#else /* TM_SINGLE_INTERFACE_HOME */
                devPtr = tfIfaceMatch(ourIpAddr, tfMhomeAddrMatch,
                                      &mhomeIndex);
#endif /* TM_SINGLE_INTERFACE_HOME */
                if (devPtr == TM_DEV_NULL_PTR)
                {
                    goto outRipRecv;
                }
                tm_ip_copy(tm_ip_dev_mask(devPtr, mhomeIndex), ourNetMask);
/*
 * Check that RIP source port is tm_const_htons(520), that the source of the
 * RIP packet is not ours, and source is on our network
 */
                if (   (peerIpPort != TM_RIP_UDP_PORT)
                    || tm_ip_match(ourIpAddr, peerIpAddr)
                    || (!tm_ip_net_match(ourIpAddr, peerIpAddr, ourNetMask)) )
                {
/* Silently ignore RIP packet from invalid source */
                    goto outRipRecv;
                }
                bufferLen -= TM_RIP_HEADER_LEN;
                ripAddrPtr = (ttRipAddrPtr)(bufferPtr + TM_RIP_HEADER_LEN);
/* Process router entries in a loop */
                for (    i = 0
                       ; (bufferLen > 0) && (i < TM_RIP_MAX_ENTRIES)
                       ; i++, bufferLen -= TM_RIP_ENTRY_LEN, ripAddrPtr++)
                {
/*
 * if the first entry after the header is an authentication entry 
 * REVNOTE Process authentication entry in next release
 */
/*
                    if ( (ripVersion == TM_RIP_VERS_2)
                         && (i == 0)
                         && (ripAddrPtr->ripaAFIdent == TM_RIP_AUTH) )
                    {
                    }
                    else
*/
                    {
/* Only process PF_INET entries */
                    if (ripAddrPtr->ripaAFIdent == TM_RIP_PF_INET)
                    {
/* Retrieve metric */
                        tm_ntohl(ripAddrPtr->ripaMetric, metric);
                        intMetric = (int)metric;
/* Retrieve next Hop for RIP version 2 */
                        tm_ip_copy(ripAddrPtr->ripaNextHop, nextHop);
/* Retrieve net mask for RIP version 2 */
                        tm_ip_copy(ripAddrPtr->ripaNetMask, netMask);
                        if (   (intMetric <= TM_RIP_INF_METRIC)
/* Make sure that metric field is valid. Should be less or equal to 16 */
                             && (   (ripVersion != TM_RIP_VERS_1)
/* RIP Version 1: enforce "must be zero" header fields. */
                                 || (   ( ripAddrPtr->ripaRouteTag ==
                                          TM_16BIT_ZERO)
                                      && tm_ip_zero(netMask)
                                      && tm_ip_zero(nextHop)) ) )
                        {
/* Validate IP address field (destination IP address field)*/
                             tm_ip_copy(ripAddrPtr->ripaIpAddr, ipAddr);
#ifdef TM_ERROR_CHECKING
                             invalidIpAddress = TM_8BIT_YES;
#endif /* TM_ERROR_CHECKING */
/*
 * Ip Address cannot be class D or E, loop back, or net 0 except for
 * a zero address (for default entry)
 */
                            if (    (!tm_ip_is_class_d_e(ipAddr))
                                  && (!tm_ip_is_loop_back(ipAddr))
                                  && (   (!tm_ip_is_net_0(ipAddr))
                                      || tm_ip_zero(ipAddr) ) )

                            {
/*
 * Check nextHop (gateway).
 * If nextHop is zero (RIP version 1 and some cases of RIP version 2)
 * use peerIpAddress as nextHop. Otherwise validate next hop the same way
 * we validated the peer address. If it matches our incoming interface
 * or is not on our incoming network use peerIpAddr as nextHop
 * instead.
 */
                                if (      tm_ip_zero(nextHop)
                                       || tm_ip_match(ourIpAddr, nextHop)
                                       || (!tm_ip_net_match(ourIpAddr,
                                                            nextHop,
                                                            ourNetMask)) )
                                {
/* Use peerIpAddr for gateway */
                                    tm_ip_copy(peerIpAddr, nextHop);
                                }

/* Compute netMask */
/*If RIP VERSION 1, or subnet mask not set in RIP VERSION 2 */
                                if (    tm_ip_zero(netMask)
/* and If entry is non default gateway entry */
                                     && tm_ip_not_zero(ipAddr) )
                                {
/*
 * (From RFC1058)
 * "routes to a subnet must not be sent outside the network of which the
 * subnet is a part", so if we have a network match on our incoming interface,
 * we can use that subnet, otherwise if we do not have a network match on
 * our incoming interface with ipAddr, we will use the class mask of ipAddr
 * and "as a subnet number would be useless without the subnet mask",
 * ipAddr is "assumed to represent a host if" it "has non-zero host part".
 * So if ipAddr has any host bit set, the netmask is a host mask (all ones).
 */
/* If ipAddr is on our incoming device network, use that interface netMask */
                                    if (tm_ip_net_match( ipAddr,
                                                         ourIpAddr,
                                                         ourNetMask) )
                                    {
                                        tm_ip_copy(ourNetMask, netMask);
                                    }
                                    else
                                    {
/* Otherwise, try and use the class of ipAddr */
                                        tm_ip_copy(
                                            tm_ip_class_net_mask(ipAddr),
                                            netMask);
                                    }
/* if ipAddr has any host bit set, it is a host route */
                                    if (tm_ip_is_host(ipAddr, netMask))
                                    {
                                        tm_ip_copy(TM_IP_HOST_MASK, netMask);
                                    }
                                } /* If netMask is zero and ipAddr not zero*/
/*
 * Now that we have our netmask, check whether the IP address field
 * is broadcast. Ignore such entries.
 */
                                if ( !(tm_ip_is_broadcast(ipAddr, netMask)))
                                {
/*
 * Update metric. For RIP, every interface has same metric of one.
 * Increase metric by one, if metric has not reached infinity yet (16).
 */
                                    if (intMetric < TM_RIP_INF_METRIC)
                                    {
                                        intMetric++;
                                    }
/* Call router code with RIP entry */
                                    tfRtRip(
                                      devPtr, mhomeIndex, ipAddr,
                                      netMask, nextHop, intMetric,
                                      ripAddrPtr->ripaRouteTag );
#ifdef TM_ERROR_CHECKING
                                    invalidIpAddress = TM_8BIT_ZERO;
#endif /* TM_ERROR_CHECKING */
                                }
                            } /* if valid ip address */
#ifdef TM_ERROR_CHECKING
                            if ( invalidIpAddress != TM_8BIT_ZERO )
                            {
                                tfKernelWarning("tfRipRecvCB",
                                                "Invalid RIP address field");
                            } /* invalidIpAddress */
#endif /* TM_ERROR_CHECKING */
                        } /* if (valid metric, valid RIP 1 fields)*/
#ifdef TM_ERROR_CHECKING
                        else
                        {
                            tfKernelWarning("tfRipRecvCB",
                          "Invalid RIP metric, or non zero RIP1 zero fields");
                        }
#endif /* TM_ERROR_CHECKING */
                    } /* if AF_INET */
                    } /* else (! authentication field)*/
                } /* for */
            } /* TM_RIP_RESPONSE */
/* REVNOTE: Handle RIP request in a next release:
            else
            {
                if (lRipHdrPtr->ripCommand == TM_RIP_REQUEST)
                {
                }
            } 
*/
        } /* (bufferLen > TM_RIP_HEADER_LEN) */
    } /* (tvRipOn) */
outRipRecv:
/* Recv CB function called with socket unlocked */
    tfFreeSharedBuffer(packetPtr, TM_SOCKET_UNLOCKED);
#undef lRipHdrPtr
}

#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4RipDummy = 0;
#endif /* ! TM_USE_IPV4 */
