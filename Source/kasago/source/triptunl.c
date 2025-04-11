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
 * Description: IPv4 tunneling (incoming only)
 *
 * Filename: triptunl.c
 * Author: Jason
 * Date Created: 7/8/01
 * $Source: source/triptunl.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:47:50JST $
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

static void tfIpDecapIncomingPacket(ttPacketPtr packetPtr, tt8Bit ulp);

/* 
 * tfUseIpTunneling Function description
 * Enables the reception of encapsulated IP packets.
 *
 * Sets the global IP decapsulation function pointer to tfDecapIncomingPacket.
 */
void tfUseIpTunneling( void )
{
    tm_context(tvIpDecapFuncPtr) = tfIpDecapIncomingPacket;
}
 

/*
 * tfDecapIncomingPacket Function description
 * Decapsulates an incoming IP-in-IP tunneled packet. 
 * 
 * 1. Increment the tunnel count for this packet.  If this count exceeds the 
 *    maximum allowed, discard this packet.  This is designed to protect against 
 *    excessive recursion.
 * 2. For IP within IP encapsulated packets...
 *     2.1. Set the packet data pointer to pointer at the inner IP header and 
 *          adjust the packet length accordingly.
 *     2.2. Pass this inner packet back to the IP layer for processing.
 * 3. For minimally encapsulated packets...
 *     3.1. Save fields from the forwarding header (original source and 
 *          destination address, checksum and protocol).
 *     3.2. Build new IP header over forwarding header and outer IP header.
 *     3.3. Adjust packet data pointers and lengths and pass decapsulated packet 
 *          back to IP layer for processing.
 * 
 * Parameter    Description
 * packetPtr    Pointer to incoming packet structure
 * ulp          Protocol number of outer IP header
 * 
 */
static void tfIpDecapIncomingPacket(ttPacketPtr packetPtr, tt8Bit ulp)
{
    ttIpHeaderPtr iphPtr;
    int           errorCode;
    tt16Bit       ipHdrLen;

    errorCode = TM_ENOERROR;

#ifdef TM_USE_IPSEC
/* Clear the SA selector so we can process the tunneled packet correctly */
    if (tm_context(tvIpsecPtr) != (ttVoidPtr)0)
    {
        tfIpsecSalistSelectorClear(packetPtr);
    }
#endif /* TM_USE_IPSEC */

/*
 * 1. Increment the tunnel count for this packet.  If this count exceeds the 
 *    maximum allowed, discard this packet.  This is designed to protect against 
 *    excessive recursion.
 */
    packetPtr->pktTunnelCount++;
    if (packetPtr->pktTunnelCount > TM_IP_MAX_TUNNEL_COUNT)
    {
/* drop this packet... */
        errorCode = TM_EPERM;
    }
/* 2. For IP within IP encapsulated packets... */
    if (ulp == TM_IP_IP_ENCAP)
    {
        iphPtr = (ttIpHeaderPtr) packetPtr->pktLinkDataPtr;

/*
 *     2.1. Set the packet data pointer to pointer at the inner IP header and 
 *          adjust the packet length accordingly.
 */
        ipHdrLen = tm_ip_hdr_len(iphPtr->iphVersLen);
        tm_pkt_ulp_hdr(packetPtr, ipHdrLen);

/*     2.2. Pass this inner packet back to the IP layer for processing. */
        tfIpIncomingPacket(packetPtr
#ifdef TM_USE_IPSEC_TASK
                           ,(ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
                           );
    }


/* 3. For minimally encapsulated packets... */
    else
    {
#ifdef TM_USE_IPV6
        if (ulp == TM_IP_IPV6)
        {
            iphPtr = (ttIpHeaderPtr) packetPtr->pktLinkDataPtr;
            ipHdrLen = tm_ip_hdr_len(iphPtr->iphVersLen);
            tm_pkt_ulp_hdr(packetPtr, ipHdrLen);
            tf6IpIncomingPacket(packetPtr
#ifdef TM_USE_IPSEC_TASK
                                , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
                                );
        }
        else
#endif /* TM_USE_IPV6 */
        {
/* drop this packet... */
            errorCode = TM_EINVAL;
        }
    }
    if (errorCode != TM_ENOERROR)
    {
/* drop this packet... */
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    }
}

#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */


/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4IpTunlDummy = 0;
#endif /* ! TM_USE_IPV4 */

