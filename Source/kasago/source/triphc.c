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
 * Description: RFC-2507 IP Header Compression for point-to-point links
 *
 * Filename: triphc.c
 * Author: Jason
 * Date Created: 12/21/2000
 * $Source: source/triphc.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2010/09/17 01:51:41JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 *  IP Header Compression (IPHC) is used primarily over low to medium speed 
 *  point-to-point links.  It is desirable for a number of 
 *  reasons (from RFC-2507):
 * 
 *   .  Improve interactive response time
 *   .  Allow using small packets for bulk data with good line efficiency
 *   .  Allow using small packets for delay sensitive low data-rate traffic
 *   .  Decrease header overhead
 * 
 *   IPHC is especially desirable when using certain applications, most notably 
 *   streaming audio and video (e.g., VoIP), as this data generally consists of 
 *   small bundles of data that must be delivered in a timely fashion.
 * 
 *   The IPHC mechanism is based on the fact that for certain packet streams 
 *   (a TCP connection for example) the header fields change rarely if at all 
 *   (such as IP addresses and port numbers).  The fields that do change tend to 
 *   increase in a regular manner that can be represented by a delta value 
 *   between packets, such as TCP sequence numbers.  Therefore, in the general 
 *   case after the packet stream is established it is not necessary to 
 *   transmit the entire chain of headers, as the majority of the fields will 
 *   not have changed.  This method is similar to and based on Van Jacobson 
 *   TCP/IP header compression, as described in RFC-1144.
 */ 

#include <trsocket.h> 
#include <trmacro.h>

#if ( defined(TM_USE_PPP) && defined(TM_USE_IPHC) )

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/*
 * Local macros
 */

/* 
 * Default values for IP header compression parameters
 */
/* Default for the maximum time in seconds between sending UDP full headers. */
#define TM_IPHC_MAX_TIME_DEF         5  

/* 
 * Default for the largest number of compressed non-TCP headers that may be 
 * sent without sending a full header.
 */
#define TM_IPHC_MAX_PERIOD_DEF     256  

/* Default for the maximum number of compression contexts available. */
#define TM_IPHC_CONTEXT_SPACE_DEF   4

/* Default maximum size of header to compress. */
#define TM_IPHC_MAX_HEADER_DEF     168

/*
 * By default, the IPHC module should actively attempt to detect and recover 
 * from TCP errors.
 */
#define TM_IPHC_TCP_ERR_REC_DEF      1  

/* TCP compressed packet flags */
#define TM_IPHCF_R    0x80  /* R-field */
#define TM_IPHCF_O    0x40  /* TCP options */
#define TM_IPHCF_I    0x20  /* IP Identification */
#define TM_IPHCF_P    0x10  /* Push flag */
#define TM_IPHCF_S    0x08  /* Seq num */
#define TM_IPHCF_A    0x04  /* Ack num */
#define TM_IPHCF_W    0x02  /* Window size */
#define TM_IPHCF_U    0x01  /* Urgent pointer */
#define TM_IPHCF_SAWU 0x0f  /* Unidirectional data transfer */
#define TM_IPHCF_SWU  0x0b  /* Echoed interactive traffic */



#define TM_IPHC_VECT_NULL_PTR     (ttIphcVectPtr)0
#define TM_IPHC_VECT_NULL_PTRPTR  (ttIphcVectPtrPtr)0
#define TM_UDP_CTX_NULL_PTR       (ttIphcUdpContextPtr)0
#define TM_TCP_CTX_NULL_PTR       (ttIphcTcpContextPtr)0


/* Verifies that the packet type is valid */
#define tm_iphc_valid_pkt_type(pktType) \
    ( ((pktType) == TM_IPHC_UDP_COMPRESSED) || \
      ((pktType) == TM_IPHC_TCP_COMPRESSED) || \
      ((pktType) == TM_IPHC_FULL_HEADER) || \
      ((pktType) == TM_IPHC_CONTEXT_STATE))

/* 
 * Encode a TCP delta value into the compressed header, per RFC-1144.
 * Delta values less than 256 are encoded as a single byte.  Values larger
 * than this are encoded as 3 bytes, with a leading zero before the 16-bit
 * delta value 
 */
#define tm_iphc_encode_delta(delta, ptr) \
if (((delta) <= 0xff) && ((delta) > 0)) \
{ \
    *((ptr)++) = (tt8Bit) (delta & 0xff); \
} \
else \
{ \
    *((ptr)++) = 0; \
    *((ptr)++) = (tt8Bit) (((tt16Bit) (delta) & 0xff00) >> 8); \
    *((ptr)++) = (tt8Bit) ((delta) & 0x00ff); \
} 

/* Decode an unsigned TCP delta value from the compressed header */
#define tm_iphc_decode_delta(delta, ptr) \
if (*(ptr) != 0) \
{ \
    (delta) = (tt16Bit)( *((ptr)++)); \
} \
else \
{ \
    (ptr)++; \
    (delta) = (tt16Bit)((((*ptr)) << 8) | *((ptr+1))); \
    (ptr) += 2; \
}

/* 
 * Decode a signed TCP delta value from the compressed header 
 * (Assumes 'ptr' is 8-bit pointer)
 */
#define tm_iphc_decode_sdelta(delta, ptr) \
if (*(ptr) != 0) \
{ \
    (delta) = (ttS16Bit)(*((signed char TM_FAR *) ptr)); \
    (ptr)++; \
} \
else \
{ \
    (ptr)++; \
    (delta) = (ttS16Bit)(((*ptr) << 8) | *(ptr+1)); \
    (ptr) += 2; \
}

/* Apply a short delta value to a TCP/IP header */
#define tm_iphc_apply_sdelta(delta, value) \
{ \
    temp16 = (tt16Bit)((tt16Bit) delta + ntohs(value)); \
    tm_htons(temp16, value); \
}

/* Apply a long delta value to a TCP/IP header */
#define tm_iphc_apply_ldelta(delta, value) \
{ \
    temp32 = (tt32Bit) delta + ntohl(value); \
    tm_htonl(temp32, value); \
}

/* Get a long delta value from a TCP/IP header */
#define tm_iphc_ldelta(a, b) (ttS32Bit) (ntohl((a)) - ntohl((b)))

/* Get a short delta value from a TCP/IP header */ 
#define tm_iphc_sdelta(a, b) (ttS16Bit) (ntohs((a)) - ntohs((b)))

/* 
 * Local types
 */

#ifdef TM_IPHC_DEBUG
unsigned char outDataBuf[2048];
unsigned int outDataLen;
#endif /* TM_IPHC_DEBUG */

/*
 * Local functions
 */

/* Initializes the IP header compression module */
static int tfIphcInit(ttIphcVectPtrPtr iphcVectPtrPtr);

/* Closes the IP header compression module and frees all associated memory */
static int tfIphcClose(ttIphcVectPtr iphcVectPtr);

/* Finds the outgoing context for the given packet */
static int tfIphcFindContext(ttPacketPtr       packetPtr,
                             ttIphcVectPtr     iphcVectPtr,
                             ttVoidPtrPtr      ctxPtrPtr,
                             tt8BitPtr         ulpTypePtr,
                             tt8BitPtr         newCtxFlagPtr,
                             ttPktLen TM_FAR * totalHdrLenPtr);

/* Attempt to compress the given outgoing packet */
static int tfIphcCompress(ttPacketPtr  packetPtr,
                          tt32BitPtr    pktTypePtr,
                          ttIphcVectPtr iphcVectPtr);

/* Attempt to decompress the given incoming packet */
static int tfIphcDecompress(ttPacketPtr   packetPtr,
                            tt32BitPtr    pktTypePtr,
                            ttIphcVectPtr iphcVectPtr);

/* 
 * General routine for all IP header compression functions, called from the
 * link layer.
 */
static int tfIpHdrCompress(ttPacketPtr     packetPtr,
                    tt32BitPtr      pktTypePtr,
                    ttVoidPtrPtr    iphcOptPtrPtr,
                    tt32Bit         iphcOpcode);

/* Allocate a new series of outgoing contexts for the specified protocol */
static int tfIphcAllocCtx(ttIphcVectPtr          vectPtr,
                          tt8Bit                 proto,
                          tt16Bit                ctxSpace,
                          ttIphcUdpContextPtrPtr vectUdpCtxPtrPtr,
                          ttIphcTcpContextPtrPtr vectTcpCtxPtrPtr);
static void tfCopyPacketToBuffer(ttPacketPtr packetPtr,
                                 tt8BitPtr   bufPtr,
                                 ttPktLen    dataLength);


/*
 * tfUseIpHdrComp Function Description
 * This function enables and initializes IP header compression.  It should be 
 * called after the link layer is initialized but before the device is opened.
 * 
 *     Example:
 * 
 *      tfUseAsyncPpp(...);
 *      tfAddInterface(...);
 * 
 *      tfUseIpHdrComp(...);
 *      tfOpenInterface(...);
 *     
 * Internals
 * 1.  ERROR CHECKING: linkLayer handle is not NULL. 
 * 2.  Point the link layer's header compression function pointer at the IPHC 
 *     compression routine.
 * 
 * Parameters
 * Parameter   Description
 * linkLayer   Indicates the link layer to use IP header compression on.
 * 
 * Returns
 * Value       Meaning
 * TM_ENOERROR Successful
 * TM_EINVAL   Invalid parameter: linkLayer is NULL 
 */

int tfUseIpHdrComp(ttUserLinkLayer linkLayer)
{
    int errorCode;

/* 1.  ERROR CHECKING: linkLayer handle is not NULL.  */
    if (linkLayer == (ttUserLinkLayer) 0)
    {
        errorCode = TM_EINVAL;       
    }
    else
    {
/*
 * 2.  Point the link layer's header compression function pointer at the IPHC 
 *     compression routine.
 */
        errorCode = tfValidLinkLayer((ttLinkLayerEntryPtr) linkLayer);
        if (errorCode == TM_ENOERROR)
        {
            ((ttLinkLayerEntryPtr) linkLayer)->lnkHdrCompressFuncPtr =
                                                                tfIpHdrCompress;
        }
    }

    return errorCode;
}


/*
 * tfIpHdrCompress Function Description
 * Primary routine called by link layer to perform IP Header compression 
 * function.  The lnkHdrCompressFuncPtr in the ttLinkLayerEntry structure is 
 * set to this routine by tfUseIpHdrComp.  The link layer then calls that 
 * function pointer to access the IP Header compression module.  The iphcOpcode 
 * parameter specifies the operation to perform:
 * 
 * Opcode              Operation to perform
 * TM_IPHC_INIT        Initializes the IPHC module.  iphcOptPtrPtr should point 
 *                     to a pointer where the address of the option structure 
 *                     will be stored.  packetPtr and pktTypePtr should be 0.
 * TM_IPHC_CLOSE       Closes the IPHC module.  Frees option structure.  
 *                     packetPtr and pktTypePtr should be zero.
 * TM_IPHC_COMPRESS    Compresses an outgoing packet indicated by packetPtr.  
 *                     The packet data pointer should point to the start of the 
 *                     uncompressed IP header and the chain length should 
 *                     indicate the complete length of the IP datagram.  This 
 *                     routine will modify the packet pointer and length to 
 *                     correspond with the outgoing packet.  pktTypePtr will be 
 *                     set to the outgoing packettype. 
 * TM_IPHC_DECOMPRESS  Decompresses an incoming packet indicated by packetPtr.  
 *                     The packet data pointer should point to the start of the 
 *                     compressed IP header and the chain data length should 
 *                     indicate the length of the data plus the compressed 
 *                     headers.  There should be enough space preceding the data 
 *                     pointer to allow for the entire decompressed header.  
 *                     pktTypePtr should be set to the incoming packet type.
 * 
 * Internals
 * 1.  Call appropriate routine for specified opcode.
 * 
 * Parameters
 * Parameter      Description
 * packetPtr      Pointer to packet to be compressed/decompressed (set to 0 for 
 *                TM_IPHC_INIT & TM_IPHC_CLOSE).
 * pktTypePtr     Packet type of compressed/decompressed packet (set to 0 for 
 *                TM_IPHC_INIT & TM_IPHC_CLOSE).
 * iphcOptPtrPtr  A pointer to a pointer to the IP header compression option 
 *                structure.
 *     
 * Returns
 * Value       Meaning
 * TM_ENOERROR Successful
 */

static int tfIpHdrCompress(ttPacketPtr     packetPtr,
                           tt32BitPtr      pktTypePtr,
                           ttVoidPtrPtr    iphcOptPtrPtr,
                           tt32Bit         iphcOpcode)
{
    int errorCode;

/* 1.  Call appropriate routine for specified opcode. */
    switch (iphcOpcode)
    {
        case TM_IPHC_INIT:
            errorCode = tfIphcInit((ttIphcVectPtrPtr) iphcOptPtrPtr);
            break;

        case TM_IPHC_CLOSE:
            errorCode = tfIphcClose( *((ttIphcVectPtrPtr)iphcOptPtrPtr));
            break;

        case TM_IPHC_COMPRESS:
            errorCode = tfIphcCompress(packetPtr,
                                       pktTypePtr,
                                       *((ttIphcVectPtrPtr) iphcOptPtrPtr));
            break;

        case TM_IPHC_DECOMPRESS:
            errorCode = tfIphcDecompress(packetPtr,
                                         pktTypePtr,
                                         *((ttIphcVectPtrPtr) iphcOptPtrPtr));
            break;

        default:
            errorCode = TM_EINVAL;
    }

    return errorCode;
}

/*
 * tfIphcCompress Function Description
 * Compresses an outgoing TCP/IP or UDP/IP header.  The packet data pointer 
 * should point to the start of the uncompressed IP header and the packet 
 * chain data length should indicate the length of the IP datagram, including 
 * IP header.  When this routine exits these values will be adjusted to reflect 
 * the changes made to compress the headers.
 * 
 * 1. ERROR CHECKING: all parameters are not null.
 * 2. Determine the context identifier (CID) for this packet.  If no context
 *    space has been allocated yet for this protocol (UDP or TCP) this will
 *    allocate enough room for the maximum number of contexts, including IP
 *    header buffers.  If this is a new packet stream a new entry will be
 *    used, and if no entries are available the least recently used entry will
 *    be reused.
 * 3. Allocate a working buffer of size MAX_HEADER (this might be overkill,
 *    but we can always guarantee that the compressed header will never be
 *    larger than this).
 * 4. Scan through the outgoing header until we find a ULP header, compressing
 *    the headers into the working buffer (compressed packet header).  As we
 *    do this, we also determine if we need to send a full header because
 *    certain fields have changed.
 *     4.1. IPv4 header:
 *         4.1.1. If this is a fragment, do not compress this packet.
 *         4.1.2. If the TOS, Flags or TTL fields have changed, send a full
 *                header.
 *         4.1.3. If the IPv4 header includes options, add the entire IPv4
 *                header, including options, to the compressed packet header.
 *         4.1.4. Otherwise, unless the next header is TCP, add the IPv4
 *                Identification field to the compressed packet header.
 *         4.1.5. If this is the first IP header, save a pointer to the IPv4
 *                length field.
 *     4.2. IPv6 header:
 *         4.2.1. If the traffic class, flow label or hop limit have changed,
 *                send a full header.
 *         4.2.2. The IPv6 header has no RANDOM fields, so the entire header
 *                is compressed away.
 *         4.2.3. If this is the first IP header, save a pointer to the IPv6
 *                length field.
 *     4.3. IPv6 Hop-By-Hop & Destination Options header:
 *         4.3.1. Scan through list of options in the header.
 *         4.3.2. If the order or length of the options has changed at all,
 *                send a full header.
 *         4.3.3. Otherwise, all values are random and should be copied into
 *                the compressed packet header, with one exception: Jumbo
 *                Payload Lengths can be inferred, so this option value should
 *                be skipped.
 *     4.4. IPv6 Routing header:
 *         4.4.1. If any of the routing header has changed, send a full header.
 *         4.4.2. The IPv6 routing header has no RANDOM fields, so the entire
 *                header is compressed away.
 *     4.5. IPv6 Fragment header:
 *         4.5.1. If the next header field has changed, send a full header.
 *         4.5.2. Copy the M flag and the Res, Fragment Offset and
 *                Identification fields into the compressed packet header.
 *         4.5.3. We can't compress any headers after the fragment header,
 *                so stop parsing the chain of headers.
 *     4.6. IPSec AH:
 *         4.6.1. If the Length, Reserved or SPI fields have changed, send
 *                a full header.
 *         4.6.2. Copy the authentication data into the compressed packet.
 *     4.7. IPSec ESP header:
 *         4.7.1. If the SPI field has changed, send a full header.
 *         4.7.2. Since all data following the ESP is encrypted, we can't
 *                compress any more headers.
 *     4.8. The headers must remain in the same order, so if the next header
 *          field doesn't match, send a full header.  Otherwise, move to the
 *          next header.
 * 5. If outgoing packet is non-TCP...
 *       5.1. Determine if a full header should be sent (if we haven't already
 *            determined that a full header should be sent).  Send a full
 *            header if:
 *           5.1.1. If the maximum amount of time since the last full header
 *                  was sent has elapsed.
 *           5.1.2. If the number of compressed headers sent since the last full 
 *                  header is greater than the current period.  When a context 
 *                  change occurs, the current period is set to one, which
 *                  causes a full header to be sent after one compressed header
 *                  is sent.  Every time a full header is sent, the current
 *                  period is doubled, up to a configured maximum.
 *     5.2. If a full header should be sent:
 *         5.2.1. Save the current UDP header into our context structure.
 *         5.2.2. Increment the context generation.
 *         5.2.3. Place CID & generation in IP and UDP length fields.
 *         5.2.4. The full header packet is now ready to send.
 *     5.3. Compress a subheader following the UDP header, such as RTP, if 
 *          configured to do so.  Currently, this functionality is not supported 
 *          as there is not subheader compression module.
 *     5.4. If a compressed packet is to be sent, start forming it by placing
 *          the UDP checksum in the last 16-bits of the compressed header.  The 
 *          checksum is the only random field for UDP so the UDP header is now 
 *          compressed.
 *     5.5. Finally, add the CID and generation to the compressed header.  One 
 *          byte of the CID will be followed by one byte of the generation.  If 
 *          the CID requires 16-bits to represent the LSB will be placed after 
 *          the generation byte. The compressed packet is now ready to send.
 * 6. If outgoing packet is TCP...
 *     6.1. Determine if this header can be compressed.  Do not compress this 
 *          header if the SYN, FIN or RST bits are set in the TCP header.
 *          If there is no socket entry for this packet then this connection is
 *          in TIME_WAIT or is closed - don't bother compressing the outgoing
 *          packets.
 *     6.2. Compute the differences for the sequence number, acknowledgement 
 *          number, window size and urgent pointer.  These are needed to 
 *          determine if a full header should be sent.
 *     6.3. Determine if a full header should be sent.  Send a full header if:
 *         6.3.1. This packet is recognized to be a TCP retransmission.
 *         6.3.2. If the TCP URG flag is not set but the urgent pointer has 
 *                changed since the last sent packet.
 *         6.3.3. ACK field delta is greater than 65535 or less than zero.
 *         6.3.4. Sequence number delta is greater than 65535 or less than zero.
 *         6.3.5. Urgent pointer, sequence number and ack number have all
 *                changed.
 *         6.3.6. Nothing has changed between the new packet and the old
 *                packet.  This indicates that it is a duplicate ACK, window
 *                probe or retransmission.
 *         6.3.7. This context has been marked invalid by a CONTEXT_STATE packet 
 *                (the decompressor has requested a full header).
 *     6.4. If a full header does need to be sent:
 *         6.4.1. Place the CID in the IP length field.
 *         6.4.2. The full header packet is now ready to send.
 *     6.5. Start compressing the TCP header:
 *         6.5.1. Check if this header matches any of the special cases.  These 
 *                cases are for common types of traffic and in these special 
 *                cases we can not send certain data fields since they can be 
 *                reconstructed by the decompressor.
 *             6.5.1.1. Unidirectional data transfer:  If only the sequence
 *                      number has changed and if the change is the same as the
 *                      amount of data in the last packet.  Set change mask to
 *                      SAWU and don't send sequence number delta.
 *             6.5.1.2. Echoed interactive traffic:  If only the sequence
 *                      number and acknowledgement number have changed and this
 *                      change is the same as the amount of data in the last
 *                      packet set the change mask to SWU and don't send the
 *                      ACK or sequence number deltas.
 *         6.5.2. Update the TCP/IP headers and other context info in the
 *                context structure.
 *         6.5.3. Start forming the compressed TCP/IP header over the current
 *                one: (not all fields are needed): TCP options, IPv4 ID delta
 *                (assumed to be '1' if not included), sequence number delta,
 *                acknowledgement number delta, window size delta, urgent
 *                pointer value, TCP reserved field and IPv4 traffic class
 *                field.
 *         6.5.4. Copy the TCP checksum and change mask.  The change mask
 *                indicates which of the above values are included.  The
 *                compressed packet is now ready to send
 *     6.6. Save the total amount of TCP data in this packet.
 * 7. Free the working buffer.    
 * 
 * Returned packet types
 * Value                    Meaning
 * TM_IPHC_UDP_COMPRESSED   Compressed UDP header
 * TM_IPHC_TCP_COMPRESSED   Compressed TCP header
 * TM_IPHC_FULL_HEADER      Full TCP/UDP header
 * TM_IPHC_IP               No compression applied
 *  
 * Parameters
 * Parameter    Description
 * packetPtr    Pointer to the packet to be compressed.
 * pktTypePtr   Type of returned packet (see above).
 * iphcVectPtr  current IPHC state vector
 * 
 * Return value       Meaning
 * TM_ENOERROR        Packet compressed successfully.
 * TM_EINVAL          Invalid packet pointer.
 * TM_EINVAL          Invalid packet type pointer
 * TM_EINVAL          Invalid state vector pointer
 * TM_ENOBUFS         Not enough memory to allocate context space.
 * TM_EPROTONOSUPPORT Protocol type of packet is not IP/UDP or IP/TCP.
 */ 

static int tfIphcCompress(ttPacketPtr  packetPtr,
                          tt32BitPtr    pktTypePtr,
                          ttIphcVectPtr iphcVectPtr)
{
#ifdef TM_USE_IPV4
    ttIpHeaderPtr        ctxIpHdrPtr; 
    ttIpHeaderPtr        ipHdrPtr;
    ttS16Bit             idDelta;    
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    tt6IpHeaderPtr       ip6HdrPtr;
    tt6IpHeaderPtr       ctxIp6HdrPtr;
    tt6ExtHdrOptionPtr   hdrOptPtr;
    tt6ExtHdrOptionPtr   ctxHdrOptPtr;
    tt6FragmentExtHdrPtr fragHdrPtr;
    tt8BitPtr            endDataPtr;    
#endif /* TM_USE_IPV6 */
    ttIphcUdpContextPtr  udpCtxPtr;
    ttIphcTcpContextPtr  tcpCtxPtr;
    ttTcpHeaderPtr       tcphPtr;
    ttTcpHeaderPtr       ctxTcphPtr;
    ttAhHeaderPtr        ahHdrPtr;
    ttAhHeaderPtr        ctxAhHdrPtr;
    ttUdpHeaderPtr       udphPtr;
    ttVoidPtr            voidCtxPtr;
    ttPacketPtr          linkPktPtr;
    tt16BitPtr           ipHdrLengthPtr;
    tt8BitPtr            linkDataPtr;
    tt8BitPtr            compHdrBufPtr;
    tt8BitPtr            compDataPtr;
    tt8BitPtr            ctxHdrDataPtr;
    ttS32Bit             seqDelta;
    ttS32Bit             ackDelta;
    tt32Bit              fullHeaderTime;
    tt32Bit              currentTime;
    ttS16Bit             winDelta;
    ttS16Bit             urgDelta;
    tt16Bit              savedTcpChecksum;
    ttPktLen             linkDataLength;
    ttPktLen             availHdrLength;
    ttPktLen             totalHdrLength; 
    tt8Bit               flagBits;
    tt8Bit               rField;
    tt8Bit               oldRField;
    tt8Bit               urgFlag;
    tt8Bit               newContext;
    tt8Bit               newTcpOptions;
    tt8Bit               ulpProtoType;
    ttPktLen             hdrLength;
    tt8Bit               ulpFound;
    tt8Bit               nextHdrType;
    tt8Bit               sendFullHeader;
    tt8Bit               ctxNextHdrType;
    int                  errorCode;
    tt16Bit              temp16;

    seqDelta = 0;
    ackDelta = 0;
    winDelta = 0;
    urgDelta = 0;
    urgFlag = 0;
    rField = 0;
    oldRField = 0;
    savedTcpChecksum = 0;
    newTcpOptions = 0;
    ipHdrLengthPtr = (tt16BitPtr)0;
    compHdrBufPtr = TM_8BIT_NULL_PTR;

    tm_kernel_set_critical;
    currentTime = tvTime;
    tm_kernel_release_critical;
    
#ifdef TM_USE_IPV4
    idDelta = 0;    
    ipHdrPtr = (ttIpHeaderPtr)0;
    ctxIpHdrPtr = (ttIpHeaderPtr)0;
#endif /* TM_USE_IPV4 */

    sendFullHeader = TM_8BIT_ZERO;
    
/* 1. ERROR CHECKING: all parameters are not null. */
#ifdef TM_ERROR_CHECKING
    if ( (packetPtr   == TM_PACKET_NULL_PTR) ||
         (pktTypePtr  == TM_32BIT_NULL_PTR) ||
         (iphcVectPtr == TM_IPHC_VECT_NULL_PTR) )
    {
        errorCode = TM_EINVAL;
        goto iphcCompressExit;
    }
#endif /* TM_ERROR_CHECKING */

/*
 * Debugging code - copy the full (uncompressed) packet to a buffer.  This
 * buffer will be used when the packet is decompressed to see exactly where
 * decompression went wrong.
 * Normally disabled (TM_IPHC_DEBUG not defined).
 */ 
#ifdef TM_IPHC_DEBUG
    {
        ttPacketPtr pktPtr;
        int         index;

        pktPtr = packetPtr;
        index = 0;
        outDataLen = pktPtr->pktChainDataLength;
        while (pktPtr != (ttPacketPtr)0)
        {
            tm_bcopy(pktPtr->pktLinkDataPtr,
                     &outDataBuf[index],
                     pktPtr->pktLinkDataLength);
            index += pktPtr->pktLinkDataLength;
            pktPtr = pktPtr->pktLinkNextPtr;
        }
                
    }
#endif /* TM_IPHC_DEBUG */

    
/*
 * 2. Determine the context identifier (CID) for this packet.  If no context
 *    space has been allocated yet for this protocol (UDP or TCP) this will
 *    allocate enough room for the maximum number of contexts, including IP
 *    header buffers.  If this is a new packet stream a new entry will be
 *    used, and if no entries are available the least recently used entry will
 *    be reused.
 */
    if (*pktTypePtr == TM_IPHC_IP)
    {
        nextHdrType = IPPROTO_IPV4;
    }
    else
    {
        nextHdrType = IPPROTO_IPV6;
    }

    ulpProtoType = nextHdrType;
    errorCode = tfIphcFindContext(packetPtr,
                                  iphcVectPtr,
                                  (ttVoidPtrPtr) &voidCtxPtr,
                                  &ulpProtoType,
                                  &newContext,
                                  &totalHdrLength);
    if (errorCode != TM_ENOERROR)
    {
        goto iphcCompressExit;
    }

    if (ulpProtoType == IPPROTO_TCP)
    {
        totalHdrLength += TM_PAK_TCP_HDR_LEN;
    }
    else if (ulpProtoType == IPPROTO_UDP)
    {
        totalHdrLength += TM_PAK_UDP_HDR_LEN;
    }


/*
 * In the current implementation, all headers must be contiguous to be able
 * to compress a packet.  This is true in the common case.  There are some
 * cases (IPSec, tunneling) where this may not be true.  In this case when the
 * network headers are scattered, send a full header packet, rather than
 * trying to compress the headers.
 */
    if (packetPtr->pktLinkDataLength < totalHdrLength)
    {
        sendFullHeader = TM_8BIT_YES;
    }

/*
 * 3. Allocate a working buffer of size MAX_HEADER (this might be overkill,
 *    but we can always guarantee that the compressed header will never be
 *    larger than this).
 */    
    compHdrBufPtr = tm_get_raw_buffer(iphcVectPtr->iphcOpts.iphcoMaxHeader);
    if (compHdrBufPtr == TM_8BIT_NULL_PTR)
    {
        errorCode = TM_ENOBUFS;
        goto iphcCompressExit;
    }

    compDataPtr = compHdrBufPtr;

    if (ulpProtoType == IPPROTO_TCP)
    {
/* Move ahead in the outgoing header to the RANDOM fields (past the CID, flags
   and checksum fields. */
        compDataPtr += 4;
        ctxHdrDataPtr = ((ttIphcTcpContextPtr)voidCtxPtr)->tctxIpHeaderPtr;
    }
    else
    {
        udpCtxPtr = (ttIphcUdpContextPtr) voidCtxPtr;
/* Move ahead in the outgoing header to the RANDOM fields (past the CID and
   generation fields. */
        if (udpCtxPtr->uctxCid <= 0x00ff)
        {
            compDataPtr += 2;
        }
        else
        {
/* Enough space for a 16-bit CID. */
            compDataPtr += 3;
        }
        ctxHdrDataPtr = udpCtxPtr->uctxIpHeaderPtr;
    }
        
        
/*
 * 4. Scan through the outgoing header until we find a ULP header, compressing
 *    the headers into the working buffer (compressed packet header).  As we
 *    do this, we also determine if we need to send a full header because
 *    certain fields have changed. 
 */
    linkPktPtr     = packetPtr;
    linkDataPtr    = packetPtr->pktLinkDataPtr;
    linkDataLength = packetPtr->pktLinkDataLength;

    ulpFound = TM_8BIT_ZERO;
    ctxNextHdrType = 0;
    
    availHdrLength = iphcVectPtr->iphcOpts.iphcoMaxHeader;
    
    while (!ulpFound && !sendFullHeader)
    {
        switch (nextHdrType)
        {
#ifdef TM_USE_IPV4
/*     4.1. IPv4 header: */
            case IPPROTO_IPV4:
                ipHdrPtr       = (ttIpHeaderPtr) linkDataPtr;
                ctxIpHdrPtr    = (ttIpHeaderPtr) ctxHdrDataPtr;                
                nextHdrType    = ipHdrPtr->iphUlp;
                ctxNextHdrType = ctxIpHdrPtr->iphUlp;
                hdrLength      = TM_4_IP_MIN_HDR_LEN;

                if (hdrLength <= availHdrLength)
                {
/*         4.1.1. If this is a fragment, do not compress this packet. */
                    if (ipHdrPtr->iphFlagsFragOff & TM_IP_MORE_FRAG_OFFSET)
                    {
                        errorCode   = TM_ENOERROR;
                        *pktTypePtr = TM_IPHC_IP;
                        goto iphcCompressExit;
                    }

                    if (!newContext)
                    {
/*
 *         4.1.2. If the TOS, Flags or TTL fields have changed, send a full
 *                header.
 */
                        if (    (ipHdrPtr->iphTos != ctxIpHdrPtr->iphTos)
                             || (    ipHdrPtr->iphFlagsFragOff
                                  != ctxIpHdrPtr->iphFlagsFragOff)
                             || (ipHdrPtr->iphTtl != ctxIpHdrPtr->iphTtl) )
                        {
                            sendFullHeader = TM_8BIT_YES;
                        }
                    
/*
 *         4.1.3. If the IPv4 header includes options, add the entire IPv4
 *                header, including options, to the compressed packet header.
 */
                        if ( tm_ip_hdr_len(ipHdrPtr->iphVersLen) >
                             TM_4_IP_MIN_HDR_LEN )
                        {
                            tm_bcopy( ipHdrPtr, compDataPtr,
                                      tm_ip_hdr_len(ipHdrPtr->iphVersLen) );
                            compDataPtr += tm_ip_hdr_len(ipHdrPtr->iphVersLen);
                        }
                    
/*            
 *         4.1.4. Otherwise, unless the next header is TCP, add the IPv4
 *                Identification field to the compressed packet header.
 */
                        if (ulpProtoType != IPPROTO_TCP)
                        {
                            tm_bcopy( &ipHdrPtr->iphId,
                                      compDataPtr,
                                      sizeof(tt16Bit) );
                            compDataPtr += sizeof(tt16Bit);
                        }
                    }
                
/*
 *         4.1.5. If this is the first IP header, save a pointer to the IPv4
 *                length field.
 */
                    if (ipHdrLengthPtr == TM_16BIT_NULL_PTR)
                    {
                        ipHdrLengthPtr = &(ipHdrPtr->iphTotalLength);
                    }
                }
                break;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
/*     4.2. IPv6 header: */
            case IPPROTO_IPV6:
                ip6HdrPtr      = (tt6IpHeaderPtr) linkDataPtr;
                ctxIp6HdrPtr   = (tt6IpHeaderPtr) ctxHdrDataPtr;                
                nextHdrType    = ip6HdrPtr->iph6Nxt;
                ctxNextHdrType = ctxIp6HdrPtr->iph6Nxt;
                hdrLength      = TM_6_IP_MIN_HDR_LEN;

                if (hdrLength <= availHdrLength)
                {
/*
 *         4.2.1. If the traffic class, flow label or hop limit have changed,
 *                send a full header.
 */
                    if (    (ip6HdrPtr->iph6Flow != ctxIp6HdrPtr->iph6Flow)
                         || (ip6HdrPtr->iph6Hops != ctxIp6HdrPtr->iph6Hops) )
                    {
                        sendFullHeader = TM_8BIT_YES;
                    }
/*
 *         4.2.2. The IPv6 header has no RANDOM fields, so the entire header
 *                is compressed away.
 */

/*
 *         4.2.3. If this is the first IP header, save a pointer to the IPv6
 *                length field.
 */
                    if (ipHdrLengthPtr == TM_16BIT_NULL_PTR)
                    {
                        ipHdrLengthPtr = &(ip6HdrPtr->iph6Plen);
                    }
                }
                break;

/*     4.3. IPv6 Hop-By-Hop & Destination Options header: */
            case IPPROTO_HOPOPTS:
            case IPPROTO_DSTOPTS:
                
/*         4.3.1. Scan through list of options in the header. */
                hdrLength = (((*(linkDataPtr + 1)) + 1) << 3);

                if (hdrLength <= availHdrLength)
                {
                    nextHdrType = *linkDataPtr;
                    ctxNextHdrType = *ctxHdrDataPtr;
                    endDataPtr  = linkDataPtr + hdrLength;
                    linkDataPtr   += sizeof(tt16Bit);
                    ctxHdrDataPtr += sizeof(tt16Bit);
                    hdrLength -= sizeof(tt16Bit);
                    while (linkDataPtr < endDataPtr)
                    {
                        hdrOptPtr = (tt6ExtHdrOptionPtr) linkDataPtr;
                        ctxHdrOptPtr = (tt6ExtHdrOptionPtr) ctxHdrDataPtr;
/*
 *         4.3.2. If the order or length of the options has changed at all,
 *                send a full header.
 */
                        if (    (   hdrOptPtr->eho6OptionType
                                 != ctxHdrOptPtr->eho6OptionType)
                             || (   hdrOptPtr->eho6OptionLen
                                 != ctxHdrOptPtr->eho6OptionLen) )
                        {
                            sendFullHeader = TM_8BIT_YES;
                        }
                        else
                        {
/*
 *         4.3.3. Otherwise, all values are random and should be copied into
 *                the compressed packet header, with one exception: Jumbo
 *                Payload Lengths can be inferred, so this option value should
 *                be skipped.
 */
                            if (hdrOptPtr->eho6OptionType != 0xc2)
                            {
                                tm_bcopy(&hdrOptPtr->eho6OptionData[0],
                                         compDataPtr,
                                         hdrOptPtr->eho6OptionLen);
                                compDataPtr += hdrOptPtr->eho6OptionLen;
                            }
                        }

/* Move to next option. */
                        hdrOptPtr    += hdrOptPtr->eho6OptionLen + 2;
                        ctxHdrOptPtr += hdrOptPtr->eho6OptionLen + 2;
                    }
                }
                break;


/*     4.4. IPv6 Routing header: */
            case IPPROTO_ROUTING:
/*
 *         4.4.1. If any of the routing header has changed, send a full
 *                header.
 */
                hdrLength = (((*(linkDataPtr + 1)) + 1) << 3);

                if (hdrLength <= availHdrLength)
                {
                    nextHdrType = *linkDataPtr;
                    ctxNextHdrType = *ctxHdrDataPtr;
                    errorCode = tm_memcmp(linkDataPtr,
                                          ctxHdrDataPtr,
                                          hdrLength);
                    if (errorCode != 0)
                    {
                        sendFullHeader = TM_8BIT_YES;
                    }
/*
 *         4.4.2. The IPv6 routing header has no RANDOM fields, so the entire
 *                header is compressed away.
 */
                }
                break;

/*     4.5. IPv6 Fragment header: */
            case IPPROTO_FRAGMENT:
                hdrLength = TM_6_IP_FRAG_EXT_HDR_LEN;
/*         4.5.1. If the next header field has changed, send a full header. */
                if (TM_6_IP_FRAG_EXT_HDR_LEN <= availHdrLength)
                {
                    if (*linkDataPtr != *ctxHdrDataPtr)
                    {
                        sendFullHeader = TM_8BIT_ZERO;
                    }
                    else
                    {
/*
 *         4.5.2. Copy the M flag and the Res, Fragment Offset and
 *                Identification fields into the compressed packet header.
 */
                        fragHdrPtr = (tt6FragmentExtHdrPtr) linkDataPtr;
                        tm_bcopy(&fragHdrPtr->fra6OffsetFlags,
                                 compDataPtr,
                                 sizeof(fragHdrPtr->fra6OffsetFlags));
                        compDataPtr += sizeof(fragHdrPtr->fra6OffsetFlags);

                        tm_bcopy(&fragHdrPtr->fra6Ident,
                                 compDataPtr,
                                 sizeof(fragHdrPtr->fra6Ident));
                        compDataPtr += sizeof(fragHdrPtr->fra6Ident);
                    }
/*
 *         4.5.3. We can't compress any headers after the fragment header,
 *                so stop parsing the chain of headers.
 */                
                    ulpFound = TM_8BIT_YES;
                }
                break;
#endif /* TM_USE_IPV6 */
                
/*     4.6. IPSec AH: */
            case IPPROTO_AH:
                ahHdrPtr = (ttAhHeaderPtr) linkDataPtr;
                ctxAhHdrPtr = (ttAhHeaderPtr) ctxHdrDataPtr;
                hdrLength = ((((ahHdrPtr->ahPayloadLen + 1)) + 2) << 2);
                if (hdrLength <= availHdrLength)
                {
                    nextHdrType = ahHdrPtr->ahNextHeader;
                    ctxNextHdrType = ctxAhHdrPtr->ahNextHeader;
/*
 *         4.6.1. If the Length, Reserved or SPI fields have changed, send
 *                a full header.
 */
                    if (    (ahHdrPtr->ahPayloadLen != ctxAhHdrPtr->ahPayloadLen)
                         || (ahHdrPtr->ahReserve != ctxAhHdrPtr->ahReserve)
                         || (ahHdrPtr->ahSpi != ctxAhHdrPtr->ahSpi) )
                    {
                        sendFullHeader = TM_8BIT_YES;
                    }
                    else
                    {
/*         4.6.2. Copy the authentication data into the compressed packet. */
                        tm_bcopy(&ahHdrPtr->ahSeq,
                                 compDataPtr,
                                 hdrLength - 8);
                        compDataPtr += (hdrLength - 8);
                    }
                }
                break;

            case IPPROTO_TCP:
                tcphPtr = (ttTcpHeaderPtr) linkDataPtr;
                if ((ttPktLen)(tm_tcp_hdrLen(tcphPtr->tcpDataOffset))
                    > availHdrLength)
                {
/* Not enough space to compress TCP header - treat this as an unrecognized
   protocol. */
                    ulpProtoType = 0;
                }
                hdrLength = 0;
                ulpFound = TM_8BIT_YES;
                break;

            case IPPROTO_UDP:
                if (availHdrLength < TM_MAX_UDP_HDR_LEN)
                {
/* Not enough space to compress UDP header - treat this as an unrecognized
   protocol. */
                    ulpProtoType = 0;
                }
                hdrLength = 0;
                ulpFound = TM_8BIT_YES;
                break;
                
                
/* Unrecognized header type - we can assume that this is the ULP. */
            default:
                hdrLength = 0;
                ulpFound = TM_8BIT_YES;
                break;
                
        }
        
/*
 *     4.8. The headers must remain in the same order, so if the next header
 *          field doesn't match, send a full header.  Otherwise, move to the
 *          next header.
 */
/* Verify that there is enough space to compress this header.  Also, do not
   advance past the upper layer protocol header. */
        if ( (hdrLength <= availHdrLength) && (ulpFound == TM_8BIT_ZERO) )
        {
            if (nextHdrType != ctxNextHdrType)
            {
                sendFullHeader = TM_8BIT_YES;
            }

            availHdrLength -= hdrLength;
            ctxHdrDataPtr  += hdrLength;            
            if (hdrLength < linkDataLength)
            {
                linkDataLength -= hdrLength;
                linkDataPtr += hdrLength;
            }
            else
            {
/* The next header lies in the next scattered buffer, so move to the next
   packet. */
                if (linkPktPtr->pktLinkNextPtr == TM_PACKET_NULL_PTR)
                {
                    tfKernelError("tfIphcFindContext",
                                  "Bad header chain");
                }

                linkPktPtr = linkPktPtr->pktLinkNextPtr;
                linkDataPtr =
                    linkPktPtr->pktLinkDataPtr + (hdrLength - linkDataLength);
                linkDataLength =
                    linkPktPtr->pktLinkDataLength - (hdrLength - linkDataLength);
            }
            
        }
        else
        {
/* Maximum header size reached, so stop compressing. */
            ulpFound = TM_8BIT_YES;
        }
    }


/* 5. If outgoing packet is UDP... */
    if (ulpProtoType != IPPROTO_TCP)
    {
        udpCtxPtr = (ttIphcUdpContextPtr) voidCtxPtr;

        if (udpCtxPtr->uctxLastFullHdrTime <= currentTime)
        {
            fullHeaderTime = currentTime - udpCtxPtr->uctxLastFullHdrTime;
        }
        else
        {
            fullHeaderTime = TM_UL(0xffffffff) - 
                                 udpCtxPtr->uctxLastFullHdrTime + currentTime;
        }

/*
 *   5.1. Determine if a full header should be sent (if we haven't already
 *        determined that a full header should be sent).  Send a full header if:
 */
        if ( (sendFullHeader == TM_8BIT_YES) ||

/*
 *     5.1.1. If the maximum amount of time since the last full header was sent 
 *            has elapsed.
 */
             (fullHeaderTime > 
                     ((tt32Bit) (iphcVectPtr->iphcOpts.iphcoMaxTime * 
                     TM_UL(1000)))) ||           
/*
 *     5.1.2. If the number of compressed headers sent since the last full 
 *            header is greater than the current period.  When a context 
 *            change occurs, the current period is set to one, which causes a 
 *            full header to be sent after one compressed header is sent.  
 *            Every time a full header is sent, the current period is doubled, 
 *            up to a configured maximum.
 */
             (udpCtxPtr->uctxCompHdrCount > udpCtxPtr->uctxPeriod) )
        {
            if (newContext == TM_8BIT_YES)
            {
                udpCtxPtr->uctxPeriod = 1;
            }
            else
            {
                udpCtxPtr->uctxPeriod <<= 1;
                if (udpCtxPtr->uctxPeriod > 
                                           iphcVectPtr->iphcOpts.iphcoMaxPeriod)
                {
                    udpCtxPtr->uctxPeriod = 
                                           iphcVectPtr->iphcOpts.iphcoMaxPeriod;
                }
            }

            udpCtxPtr->uctxLastFullHdrTime = currentTime;

            sendFullHeader = TM_8BIT_YES;
        }
        else
        {
            sendFullHeader = TM_8BIT_ZERO;
        }

/* linkDataPtr points at the start of the ULP header after parsing through the
   headers above. */
        udphPtr = (ttUdpHeaderPtr) linkDataPtr;
        
/*   5.2. If a full header should be sent due to change in context: */
        if (sendFullHeader == TM_8BIT_YES)
        {
/*     5.2.1. Save the current UDP header and IP header(s) into our context
              structure. */
            tfCopyPacketToBuffer(
                packetPtr,
                udpCtxPtr->uctxIpHeaderPtr,
                iphcVectPtr->iphcOpts.iphcoMaxHeader - availHdrLength);

            if (ulpProtoType == IPPROTO_UDP)
            {
                tm_bcopy( (tt8BitPtr) udphPtr, 
                          &(udpCtxPtr->uctxUdpHeader), 
                          TM_PAK_UDP_HDR_LEN);
            }
            
            udpCtxPtr->uctxCompHdrCount = 0;

/*     5.2.2. Increment the context generation. */
            udpCtxPtr->uctxGeneration = \
                (tt8Bit)((udpCtxPtr->uctxGeneration + 1) % 0x3f);

/*     5.2.3. Place CID & generation in IP and UDP length fields. */
            if (udpCtxPtr->uctxCid <= 0x00ff)
            {
/* 8-bit CID */
                if (ipHdrLengthPtr != TM_16BIT_NULL_PTR)
                {
                    
                    temp16 = (tt16Bit) (udpCtxPtr->uctxCid
                                        | (udpCtxPtr->uctxGeneration << 8));
                    *ipHdrLengthPtr = htons(temp16);
                    if (ulpProtoType == IPPROTO_UDP)
                    {
                        udphPtr->udpLength     = 0;
                    }
                }
            }
            else
            {
/* 16-bit CID */
                if (ipHdrLengthPtr != TM_16BIT_NULL_PTR)
                {
                    temp16 = (tt16Bit)
                        (0x8000 | (udpCtxPtr->uctxGeneration << 8));
                    *ipHdrLengthPtr = htons(temp16);
                    if (ulpProtoType == IPPROTO_UDP)
                    {
                        udphPtr->udpLength = htons(udpCtxPtr->uctxCid);
                    }
                }
            }
/*     5.2.4. The full header packet is now ready to send. */
            *pktTypePtr = TM_IPHC_FULL_HEADER;

        }
        else
        {
            udpCtxPtr->uctxCompHdrCount++;

/*
 *   5.3. Compress a subheader following the UDP header, such as RTP, if 
 *        configured to do so.  Currently, this functionality is not supported 
 *        as there is not subheader compression module.
 */

/*
 *   5.4. If a compressed packet is to be sent, start forming it by placing the 
 *        UDP checksum in the last 16-bits of the compressed header.  The 
 *        checksum is the only random field for UDP so the UDP header is now 
 *        compressed.
 */
            if (ulpProtoType == IPPROTO_UDP)
            {
                *(compDataPtr++) =
                    (tt8Bit) ((udphPtr->udpChecksum & 0x00ff));
                *(compDataPtr++) =
                    (tt8Bit) ((udphPtr->udpChecksum & 0xff00) >> 8);
            }
            
/*
 *   5.5. Finally, add the CID and generation to the compressed header.  One 
 *        byte of the CID will be followed by one byte of the generation.  If 
 *        the CID requires 16-bits to represent the LSB will be placed after 
 *        the generation byte. The compressed packet is now ready to send.
 */        
            if (udpCtxPtr->uctxCid <= 0x00ff)
            {
/* 8-bit CID */
                *(compHdrBufPtr)   = (tt8Bit)(udpCtxPtr->uctxCid & 0x00ff);
                *(compHdrBufPtr+1) = udpCtxPtr->uctxGeneration;
            }
            else
            {
/* 16-bit CID */
                *(compHdrBufPtr) =
                    (tt8Bit)((udpCtxPtr->uctxCid & 0xff00) >> 8);
                *(compHdrBufPtr + 1) =
                    (tt8Bit)(0x80 | udpCtxPtr->uctxGeneration);
                *(compHdrBufPtr + 2) = (tt8Bit)(udpCtxPtr->uctxCid & 0x00ff);
            }

/* Copy the compressed header into the packet. */
            hdrLength = (ttPktLen) (compDataPtr - compHdrBufPtr);
            tm_bcopy(compHdrBufPtr,
                     packetPtr->pktLinkDataPtr + (totalHdrLength - hdrLength),
                     hdrLength);
            tm_pkt_ulp_hdr(packetPtr, totalHdrLength - hdrLength);
                
            *pktTypePtr = TM_IPHC_UDP_COMPRESSED;
        }
    }

/* 6. If outgoing packet is TCP... */
    else 
    {
        tcpCtxPtr  = (ttIphcTcpContextPtr) voidCtxPtr;
        ctxTcphPtr = &(tcpCtxPtr->tctxTcpHeader);
        
        tcphPtr = (ttTcpHeaderPtr) linkDataPtr;

        
/*
 *   6.1. Determine if this header can be compressed.  Do not compress this 
 *        header if the SYN, FIN or RST bits are set in the TCP header.
 *        If there is no socket entry for this packet then this connection is
 *        in  TIME_WAIT or is closed or not established - don't bother
 *        compressing the outgoing packets.
 */        
        if ( (tcphPtr->tcpControlBits & 
                                      (TM_TCP_SYN | TM_TCP_FIN | TM_TCP_RST)) ||
             (packetPtr->pktSharedDataPtr->dataSocketEntryPtr == 
                                                     TM_SOCKET_ENTRY_NULL_PTR) )
        {
            errorCode   = TM_ENOERROR;
/* Don't compress, so leave the packet type alone. */
            goto iphcCompressExit;
        }
        
        if (newContext == TM_8BIT_YES)
        {
            tcpCtxPtr->tctxSocketDesc = 
                      packetPtr->pktSharedDataPtr->dataSocketEntryPtr->socIndex;

            tcpCtxPtr->tctxValid          = TM_8BIT_YES;
            tcpCtxPtr->tctxLastPayloadLen = 0;
        }
         
/*
 *   6.2. Compute the differences for the sequence number, acknowledgement 
 *        number, window size and urgent pointer.  These are needed to 
 *        determine if a full header should be sent.
 */
        if (newContext == TM_8BIT_ZERO)
        {
            seqDelta = tm_iphc_ldelta(tcphPtr->tcpSeqNo, ctxTcphPtr->tcpSeqNo);
            ackDelta = tm_iphc_ldelta(tcphPtr->tcpAckNo, ctxTcphPtr->tcpAckNo);

            winDelta = tm_iphc_sdelta(tcphPtr->tcpWindowSize, 
                                      ctxTcphPtr->tcpWindowSize);
            urgDelta = tm_iphc_sdelta(tcphPtr->tcpUrgentPtr, 
                                      ctxTcphPtr->tcpUrgentPtr);

            urgFlag  = (tt8Bit)(tcphPtr->tcpControlBits & TM_TCP_URG);

#ifdef TM_USE_IPV4 
            if (*pktTypePtr == TM_IPHC_IP)
            {
                idDelta  = tm_iphc_sdelta(ipHdrPtr->iphId, ctxIpHdrPtr->iphId);
            }
#endif /* TM_USE_IPV4 */

/* 
 * The 'R-Field' consits of the 2 most significant bits of the IP TOS field
 * and all 6-bits of the TCP reserved field 
 */
            rField    = (tt8Bit) (((tcphPtr->tcpDataOffset & 0x0f) << 2) |
                                 ((tcphPtr->tcpControlBits & 0xc0) >> 6));
            oldRField = (tt8Bit) (((ctxTcphPtr->tcpDataOffset & 0xf) << 2) |
                                 ((tcphPtr->tcpControlBits & 0xc0) >> 6));

#ifdef TM_USE_IPV4 
            if (*pktTypePtr == TM_IPHC_IP)
            {
                rField    |= (tt8Bit)(ipHdrPtr->iphTos & 0xc0);
                oldRField |= (tt8Bit)(ctxIpHdrPtr->iphTos & 0xc0);
            }
#endif /* TM_USE_IPV4 */
            
            savedTcpChecksum = tcphPtr->tcpChecksum;

     
/* If there are new TCP options, we must send an uncompressed header */
            if ( tm_tcp_hdrLen(ctxTcphPtr->tcpDataOffset) != 
                                         tm_tcp_hdrLen(tcphPtr->tcpDataOffset) )
            {
                newTcpOptions = TM_8BIT_YES;
            }
            else
            {
                newTcpOptions = TM_8BIT_ZERO;
            }

        }
   
/* 
 * Restore bits 6,7 of the IP TOS field and all 6-bits of the TCP reserved 
 * field as these values MUST NOT update the context.
 */
        if (newContext == TM_8BIT_ZERO)
        {
#ifdef TM_USE_IPV4 
            if (*pktTypePtr == TM_IPHC_IP)
            {
                ctxIpHdrPtr->iphTos = \
                    (tt8Bit)((ctxIpHdrPtr->iphTos & 0x3f) | (oldRField & 0xc0));
            }
#endif /* TM_USE_IPV4 */

            ctxTcphPtr->tcpDataOffset =
                (tt8Bit)((ctxTcphPtr->tcpDataOffset & 0xf0) | 
                ((oldRField & 0x3c) >> 2));

            ctxTcphPtr->tcpControlBits =
                (tt8Bit)((ctxTcphPtr->tcpControlBits & 0x3f) | 
                ((oldRField & 0x03) << 6));
        }


/*     6.3. Determine if a full header should be sent.  Send a full header if: */

/*         6.3.1. This packet is recognized to be a TCP retransmission. */
        if ( (seqDelta < 0) ||

/*
 *         6.3.2. If the TCP URG flag is not set but the urgent pointer has 
 *                changed since the last sent packet.
 */
             ( (urgFlag == 0) && (urgDelta != 0)) ||

/*         6.3.3. ACK field delta is greater than 65535 or less than zero. */
             (ackDelta > TM_L(65535)) ||
             (ackDelta < 0) ||

/*         6.3.4. Sequence number delta is greater than 65535 or less than
                  zero. */
             (seqDelta > TM_L(65535)) ||
             (seqDelta < 0) ||

/*         6.3.5. Urgent pointer, sequence number and ack number have all
                  changed.*/
             ( (urgFlag != 0) && (seqDelta != 0) && (ackDelta != 0) ) ||

/*
 *         6.3.6. Nothing has changed between the new packet and the old
 *                packet.  This indicates that it is a duplicate ACK, window
 *                probe or retransmission.
 */
             ( (urgFlag == 0) && (seqDelta == 0) && 
               (ackDelta == 0) && (winDelta == 0) ) ||

/*
 *         6.3.7. This context has been marked invalid by a CONTEXT_STATE packet 
 *                (the decompressor has requested a full header).
 */
            (tcpCtxPtr->tctxValid == TM_8BIT_ZERO) ||

/*            The TCP data offset change which means there are new options */
            (newTcpOptions == TM_8BIT_YES) )
        {

/*     6.4. If a full header does need to be sent: */
            tfCopyPacketToBuffer(
                packetPtr,
                tcpCtxPtr->tctxIpHeaderPtr,
                iphcVectPtr->iphcOpts.iphcoMaxHeader - availHdrLength);

            tm_bcopy( (tt8BitPtr) tcphPtr, 
                      &(tcpCtxPtr->tctxTcpHeader), 
                      TM_PAK_TCP_HDR_LEN);

/*         6.4.1. Place the CID in the IP length field. */
            *ipHdrLengthPtr = htons(tcpCtxPtr->tctxCid);
            
/*         6.4.2. The full header packet is now ready to send. */
            *pktTypePtr = TM_IPHC_FULL_HEADER;

        }                       
        else
        {

            
/*     6.5. Start compressing the TCP header: */
            flagBits = 0;
            
/*
 *         6.5.1. Check if this header matches any of the special cases.  These 
 *                cases are for common types of traffic and in these special 
 *                cases we can not send certain data fields since they can be 
 *                reconstructed by the decompressor.
 *             6.5.1.1. Unidirectional data transfer:  If only the sequence
 *                      number has changed and if the change is the same as the
 *                      amount of data in the last packet.  Set change mask to
 *                      SAWU and don't send sequence number delta.
*/
            if ( (seqDelta == (ttS32Bit) tcpCtxPtr->tctxLastPayloadLen) &&
                 (ackDelta == 0) &&
                 (winDelta == 0) &&
                 (urgFlag  == 0) )
            {
                flagBits = TM_IPHCF_SAWU;
            }
/*
 *             6.5.1.2. Echoed interactive traffic:  If only the sequence
 *                      number and acknowledgement number have changed and this
 *                      change is the same as the amount of data in the last
 *                      packet set the change mask to SWU and don't send the
 *                      ACK or sequence number deltas.
 */                      
            if ( (seqDelta == (ttS32Bit) tcpCtxPtr->tctxLastPayloadLen) &&
                 (ackDelta == (ttS32Bit) tcpCtxPtr->tctxLastPayloadLen) &&
                 (winDelta == 0) &&
                 (urgFlag  == 0) )
            {
                flagBits = TM_IPHCF_SWU;
            }

/*
 *         6.5.2. Update the TCP/IP headers and other context info in the
 *                context structure.
 */
            tfCopyPacketToBuffer(
                packetPtr,
                tcpCtxPtr->tctxIpHeaderPtr,
                iphcVectPtr->iphcOpts.iphcoMaxHeader - availHdrLength);
            
            tm_bcopy( (tt8BitPtr) tcphPtr,
                      &(tcpCtxPtr->tctxTcpHeader),
                      TM_PAK_TCP_HDR_LEN );
            
/*
 *         6.5.3. Start forming the compressed TCP/IP header over the current
 *                one: (not all fields are needed): TCP options, IPv4 ID delta
 *                (assumed to be '1' if not included), sequence number delta,
 *                acknowledgement number delta, window size delta, urgent
 *                pointer value, TCP reserved field and IPv4 traffic class
 *                field.
 */
            
/* R-Octet: Bits 6,7 of the IP TOS field & TCP reserved field */
            if (rField != oldRField)
            {            
                flagBits    |= TM_IPHCF_R;
                *(compDataPtr++) = rField;
            }
            
/* 
 * TCP sequence & acknowledgement numbers, window size and urgent pointer.
 * Only process if not a special case (see above).
 */
            if ( !(flagBits & TM_IPHCF_SWU) && !(flagBits & TM_IPHCF_SAWU) )
            {
                if (urgFlag != 0)
                {
                    flagBits |= TM_IPHCF_U;
                    tm_iphc_encode_delta(tcphPtr->tcpUrgentPtr, compDataPtr);
                }

                if (winDelta != 0)
                {
                    flagBits |= TM_IPHCF_W;
                    tm_iphc_encode_delta(winDelta, compDataPtr);
                }

                if (ackDelta > 0)
                {
                    flagBits |= TM_IPHCF_A;
                    tm_iphc_encode_delta(ackDelta, compDataPtr);
                }
                
                if (seqDelta > 0)
                {
                    flagBits |= TM_IPHCF_S;
                    tm_iphc_encode_delta(seqDelta, compDataPtr);
                }
            }

/* IP identification */
#ifdef TM_USE_IPV4
            if ((idDelta != 1) && (*pktTypePtr == TM_IPHC_IP))
            {
                flagBits |= TM_IPHCF_I;
                tm_iphc_encode_delta(idDelta, compDataPtr);
            }
#endif /* TM_USE_IPV4 */
            
/* TCP options */
            if (tm_tcp_hdrLen(tcphPtr->tcpDataOffset) > TM_PAK_TCP_HDR_LEN)
            {
                flagBits |= TM_IPHCF_O;
            }    

/* 
 * If the PUSH bit was set in the outgoing TCP header, set it in the flags 
 * field as well.
 */
            if (tcphPtr->tcpControlBits & TM_TCP_PSH)
            {
                flagBits |= TM_IPHCF_P;                
            }

/*
 *         6.5.4. Copy the TCP checksum and change mask.  The change mask
 *                indicates which of the above values are included.  The
 *                compressed packet is now ready to send
 */
            
/* Context ID */
            *(compHdrBufPtr) = (tt8Bit)(tcpCtxPtr->tctxCid & 0x00ff);

/* Flags */
            *(compHdrBufPtr+1) = flagBits;
            
/* TCP Checksum */
            *(compHdrBufPtr+2) = (tt8Bit)((savedTcpChecksum & 0xff00) >> 8);
            *(compHdrBufPtr+3) = (tt8Bit)((savedTcpChecksum & 0x00ff));

/*     6.6. Save the total amount of TCP data in this packet. */
        tcpCtxPtr->tctxLastPayloadLen = packetPtr->pktChainDataLength
            - tcpCtxPtr->tctxIpHeaderLength
            - tm_tcp_hdrLen(tcphPtr->tcpDataOffset);
            
/* Copy compressed header into packet. */
            hdrLength = (ttPktLen) (compDataPtr - compHdrBufPtr);
            tm_bcopy(compHdrBufPtr,
                     packetPtr->pktLinkDataPtr + (totalHdrLength - hdrLength),
                     hdrLength);
            tm_pkt_ulp_hdr(packetPtr, totalHdrLength - hdrLength);
            
            *pktTypePtr = TM_IPHC_TCP_COMPRESSED;
        }

        
    }

iphcCompressExit:


/* 7. Free the working buffer. */
    if (compHdrBufPtr != TM_8BIT_NULL_PTR)
    {
        tm_free_raw_buffer(compHdrBufPtr);
    }
    
/*
 * If a compressed packet (including full header packets), disable TCP
 * header recycling because IP Header Compression modifies the header.
 */ 
    if ( (errorCode == TM_ENOERROR) && (*pktTypePtr != TM_IPHC_IP) )
    {
        packetPtr->pktSharedDataPtr->dataFlags &= ~TM_BUF_TCP_HDR_BLOCK;
    }
    
    return errorCode;
}


/* 
 * tfIphcDecompress Function Description
 * Decompress an incoming TCP/IP or UDP/IP header.
 *
 * 1. ERROR CHECKING: all parameters are not null and pktTypePtr is a valid 
 *                    packet type.
 * 2. If this is a compressed packet:
 *     2.1. Determine the context for this incoming packet.  The contexts for
 *          full headers will be determined later once the ULP is known after
 *          processing the chain of headers.
 *     2.2. If no context space has been allocated for this protocol, drop the
 *          packet (context space is allocated when receiving a full header,
 *          and a compressed header should never be sent before a full header).
 * 3. Process IP header chain.  This could include any number of IPv4 and/or
 *    IPv6 headers (possibly tunneled) including IPv6 extension headers.
 *     3.1. If this is a compressed header, start processing at the beginning
 *          of the saved IP header chain.  Otherwise, start at the beginning
 *          of the full header sent by the peer.
 *     3.2. Iterate through the included headers until we reach a ULP header
 *          or have processed an IPv6 Fragment header, or the end of the packet
 *          or saved header chain.
 *     3.3. IPv4 header:
 *         3.3.1. If this is the first header, save the Total Length field (for
 *                CID and/or generation).
 *         3.3.2. Set the IPv4 Total Length to the actual length of the packet,
 *                including this IPv4 header.
 *         3.3.3. If this is a compressed packet, and the IPv4 header does not
 *                include any options:
 *             3.3.3.1. Deal with the special case where the IPv4 header is
 *                      immediately followed by a TCP header.  In this case,
 *                      let the TCP decompression code deal with the IPv4 ID.
 *                      Otherwise, this is just another random field...
 *             3.3.3.2. Otherwise, get the Identification from the compressed
 *                      packet and set it in the IPv4 header.
 *             3.3.3.3. Compute the checksum over the IPv4 header.
 *         3.3.4. If this is a compressed packet, and the IPv4 header includes
 *                options (size > 20 bytes):
 *             3.3.4.1. The entire IPv4 header + options is included in the
 *                      compressed packet, so copy it into the context
 *                      structure.  Other headers following this one may still
 *                      be compressed, so continue to process header chain.
 *         3.3.5. Save the next header type and the current header length.
 *     3.4. IPv6 header:
 *         3.4.1. If this is the first header, save the Payload Length field
 *                (for CID and/or generation)
 *         3.4.2. Set the IPv6 Payload Length field to the actual length of
 *                the packet, excluding this IPv6 header.
 *         3.4.3. Nothing needs to be done for compressed IPv6 headers (all
 *                fields are NOCHANGE).
 *     3.5. IPv6 Hop-by-Hop, Destination options:
 *         3.5.1. Scan through individual options:
 *             3.5.1.1. Skip over Pad1, PadN options.
 *             3.5.1.2. All other option values are RANDOM, so read the value
 *                      from the compressed packet and insert into the option
 *                      header.
 *     3.6. IPv6 Routing header:
 *         3.6.1. Skip over Routing Header (all fields are NOCHANGE).
 *     3.7. IPv6 Fragment header:
 *         3.7.1. Read the fragment offset, identification, Res field and M
 *                flag from compressed packet and insert into the fragment
 *                header.
 *         3.7.2. Since headers after the Fragment header are not compressed,
 *                decompression stops once a Fragment header is encountered.
 *     3.8. IPSec Authentication header:
 *         3.8.1. Read authentication data from compressed packet, and insert
 *                into the authentication header.
 *     3.9. IPSec Encapsulating Security Payload Header:
 *         3.9.1. All data after the ESP is encrypted, and so is not compressed.
 * 4. If this was a full header:
 *     4.1. If no context space has been allocated yet for this protocol (UDP
 *          or TCP) allocate enough room for the maximum number of contexts,
 *          including buffers to hold maximum size IP header chains.
 *     4.2. Update the header length in the context structure.
 * 5. If incoming packet is UDP...
 *     5.1. If this is a full header, update the context with the new header 
 *          and the new generation number.
 *     5.2. Drop any compressed packets whose generation does not match the 
 *          generation in our context.  If the generations do not match it 
 *          means that we lost a full header and that our context is no 
 *          longer valid.
 *     5.3. If the upper layer protocol is UDP:     
 *         5.3.1. Save the UDP checksum from the compressed header in our
 *                context.
 *         5.3.2. Infer the UDP payload lengths from the link layer frame length.
 *         5.3.3. Decompress a subheader following the UDP header, such as
 *                RTP, if configured to do so.  Currently, this functionality
 *                is not supported as there is not subheader compression module.
 *     5.4. Copy the full decompressed header to the packet.
 * 6. If incoming packet is TCP...
 *     6.1. If this is a full header, update the context with the header.
 *
 *     6.2. If this is a compressed header, first check if this packet is for a 
 *          special case.  These cases are for common types of traffic and 
 *          in these special cases certain fields are omitted by the compressor 
 *          and must be reconstructed here.
 *         6.2.1. Unidirectional data transfer.  Change mask is set to SAWU.
 *                The sequence number has increased by the amount of data in
 *                the last packet.
 *         6.2.2. Echoed interactive traffic.  Change mask is set to SWU.  The 
 *                sequence number and acknowledgement number has increased by
 *                the amount of data in the last packet.
 *     6.3. If this is a compressed packet that is not a special case, start 
 *          updating the context by examining the compressed header.  Based on 
 *          the change mask, update the TCP options, IPv4 ID (assumed to be one 
 *          if not included), sequence  number, acknowledgement number, window 
 *          size, urgent pointer, TCP reserved & IP traffic class field and TCP 
 *          checksum.
 *     6.4. Copy the full decompressed header to the packet.           
 *     6.5. If TCP error recovery is enabled...
 *         6.5.1. Compute the TCP checksum on the decompressed packet.
 *         6.5.2. If the checksum fails, try to repair the TCP header.  This is 
 *                done by applying the delta values in the compressed header to 
 *                the current header again.  This will repair an error caused by 
 *                losing a previous compressed header that is identical to the 
 *                current one (very common in TCP bulk transfers).  The checksum 
 *                should be recomputed.
 *         6.5.3. If the checksum still fails, perform the above step again.
 *         6.5.4. If the checksum is still invalid, send a 'CONTEXT_STATE'
 *                packet back to the compressor which will request that a
 *                full header be sent for this context.
 * 7. If incoming packet is a TCP CONTEXT_STATE packet...
 *     7.1. If this is a TCP header request, search through the list of CID's 
 *           in the request and mark each context as invalid (i.e., the next 
 *           packet sent with this context must be sent with a full header).
 * 8. Return type of decompressed packet, if decompression was successful.
 * 
 * Parameters
 * Parameter    Description
 * packetPtr    Pointer to the packet to be uncompressed.  The packet pointer 
 *              should point to the start of the compressed header, and there 
 *              should be enough space before this point to place the entire 
 *              uncompressed header (either TM_IPHC_UDP_MAX_HDR or 
 *              TM_IPHC_TCP_MAX_HDR).
 * pktTypePtr   Type of the incoming packet, either TM_IPHC_UDP_COMPRESSED, 
 *              TM_IPHC_TCP_COMPRESSED or TM_IPHC_FULL_HEADER.
 * iphcVectPtr  Current IPHC state vector.
 * 
 * Return Value         Meaning
 * TM_ENOERROR          Successful
 * TM_EINVAL            Invalid packet pointer.
 * TM_EINVAL            Invalid packet type pointer.
 * TM_EINVAL            Invalid state vector pointer.
 * TM_EPROTONOSUPPORT   Packet type not supported (see above for list of 
 *                      supported types).
 * TM_ENOBUFS           Not enough memory to allocate context space.
 */ 
static int tfIphcDecompress(ttPacketPtr   packetPtr,
                            tt32BitPtr    pktTypePtr,
                            ttIphcVectPtr iphcVectPtr)
{
#ifdef TM_USE_IPV4
    ttIpHeaderPtr       ipHdrPtr;
    ttIpHeaderPtr       ctxIphPtr;
    ttIpHeaderPtr       origIpHdrPtr;
    ttPseudoHeaderPtr   pshPtr;
    tt8Bit              savedIpTtl;
    tt16Bit             savedIpChecksum;
    tt16Bit             idDelta;    
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    tt6FragmentExtHdrPtr fragHdrPtr;
    tt6PseudoHeaderPtr  psh6Ptr;    
    tt6IpHeaderPtr      ip6HdrPtr;
    tt8Bit              saved6IpHops;
    tt32Bit             saved6FlowInfo;
    tt8Bit              optType;
    ttPktLen            optLength;
    tt8BitPtr           optDataPtr;
    tt8BitPtr           endOptDataPtr;    
#endif /* TM_USE_IPV6 */
    ttAhHeaderPtr       ahHdrPtr;
    ttIphcUdpContextPtr udpCtxPtr;
    ttIphcTcpContextPtr tcpCtxPtr;
    ttTcpHeaderPtr      tcphPtr;
    ttTcpHeaderPtr      ctxTcphPtr;
    tt16BitPtr          ipHdrLengthPtr;
    tt16BitPtr          pseudoHdrPtr;
    tt8BitPtr           savLinkDataPtr; 
    tt8BitPtr           pktDataPtr;
    tt8BitPtr           endPktDataPtr;
    tt8BitPtr           compHdrDataPtr;
    tt32Bit             newPktType;
    ttPktLen            i; 
    ttPktLen            availHdrLength;
    ttS32Bit            seqDelta;
    ttS32Bit            ackDelta;
    ttS16Bit            winDelta;
    tt16Bit             pseudoHdrLen; 
    tt16Bit             temp16;
    tt32Bit             temp32;
    ttPktLen            pktLen;
    tt16Bit             contextId; 
    tt16Bit             delta; 
    tt16Bit             checksum;
    tt8Bit              generation; 
    tt8Bit              ulpProtoType;
    tt8Bit              flagBits;
    tt8Bit              ulpFound;
    tt8Bit              compressedPkt;
    tt8Bit              nextHdrType;
    ttPktLen            hdrLength;
    tt8Bit              savedIpHdrNxt;
    int                 errorCode;

    
    errorCode = TM_ENOERROR;

/* Avoid compiler warnings... */
    generation     = 0;
    udpCtxPtr      = TM_UDP_CTX_NULL_PTR;
    tcpCtxPtr      = TM_TCP_CTX_NULL_PTR;
    ipHdrLengthPtr = TM_16BIT_NULL_PTR;
    flagBits       = 0;
    endPktDataPtr  = TM_8BIT_NULL_PTR;
    savedIpHdrNxt  = 0;
    pseudoHdrPtr   = TM_16BIT_NULL_PTR;
    pseudoHdrLen   = 0;
#ifdef TM_USE_IPV6
    saved6FlowInfo = TM_UL(0);
    saved6IpHops   = 0;
    ip6HdrPtr      = (tt6IpHeaderPtr)0;
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
    savedIpChecksum = 0;
    savedIpTtl      = 0;
    ipHdrPtr        = (ttIpHeaderPtr)0;
    origIpHdrPtr    = (ttIpHeaderPtr)0;
#endif /* TM_USE_IPV4 */
    
/*
 * 1. ERROR CHECKING: all parameters are not null and pktTypePtr is a valid 
 *                    packet type.
 */
#ifdef TM_ERROR_CHECKING
    if ( (packetPtr   == TM_PACKET_NULL_PTR) ||
         (pktTypePtr  == TM_32BIT_NULL_PTR) ||
         (iphcVectPtr == TM_IPHC_VECT_NULL_PTR) ||
         (!tm_iphc_valid_pkt_type(*pktTypePtr)) )
    {
        errorCode = TM_EINVAL;
        goto iphcDecompressExit;
    }
#endif /* TM_ERROR_CHECKING */

    pktLen  = (tt16Bit) packetPtr->pktChainDataLength;

    pktDataPtr = TM_8BIT_NULL_PTR;
    compressedPkt = TM_8BIT_ZERO;

    newPktType = 0;
    
/* 2. If this is a compressed packet:
 *
 *     2.1. Determine the context for this incoming packet.  The contexts for
 *          full headers will be determined later once the ULP is known after
 *          processing the chain of headers.
 */
    compHdrDataPtr = packetPtr->pktLinkDataPtr;
    if (*pktTypePtr == TM_IPHC_UDP_COMPRESSED)
    {
        compressedPkt = TM_8BIT_YES;
        generation = (tt8Bit) (*(compHdrDataPtr+1) & 0x3f);

        if (*(compHdrDataPtr+1) & 0x80)
        {
/* 16-bit CID */
            contextId = (tt16Bit) ((*compHdrDataPtr << 8) | *(compHdrDataPtr+2));
            compHdrDataPtr += 3;
        }
        else
        {
/* 8-bit CID */
            contextId = *compHdrDataPtr;
            compHdrDataPtr += 2;
        }

/* If data is included in compressed header, move past it */
        if (*(packetPtr->pktLinkDataPtr+1) & 0x40)
        {          
            compHdrDataPtr++;
        }
        
/*
 *     2.2. If no context space has been allocated for this protocol, drop the
 *          packet (context space is allocated when receiving a full header,
 *          and a compressed header should never be sent before a full header).
 */
        if (   (iphcVectPtr->iphcUdpInCtxPtr != (ttIphcUdpContextPtr)0)
            && (contextId < iphcVectPtr->iphcOpts.iphcoInUdpSpace) )
        {
            udpCtxPtr = &(iphcVectPtr->iphcUdpInCtxPtr[contextId]);
            pktDataPtr = (tt8BitPtr) udpCtxPtr->uctxIpHeaderPtr;
            endPktDataPtr = pktDataPtr + udpCtxPtr->uctxIpHeaderLength;
        }
        else
        {
            errorCode = TM_EINVAL;
        }
        
    }
    else if (*pktTypePtr == TM_IPHC_TCP_COMPRESSED)
    {
        compressedPkt = TM_8BIT_YES;        
        contextId = *compHdrDataPtr++;
        flagBits = *compHdrDataPtr++;
        
        
/*
 *     2.2. If no context space has been allocated for this protocol, drop the
 *          packet (context space is allocated when receiving a full header,
 *          and a compressed header should never be sent before a full header).
 */
        if (   (iphcVectPtr->iphcTcpInCtxPtr != (ttIphcTcpContextPtr)0)
            && (contextId < iphcVectPtr->iphcOpts.iphcoInTcpSpace) )            
        {
            tcpCtxPtr  = &(iphcVectPtr->iphcTcpInCtxPtr[contextId]);
            pktDataPtr = (tt8BitPtr) tcpCtxPtr->tctxIpHeaderPtr;
            endPktDataPtr = pktDataPtr + tcpCtxPtr->tctxIpHeaderLength;
        }
        else
        {
            errorCode = TM_EINVAL;
        }
    }

    if (errorCode != TM_ENOERROR)
    {
/* An error occured, so drop the packet (packet should be freed by link layer
   receive routine. (e.g. tfPppAsyncRecv) */
        goto iphcDecompressExit;
    }

/* Context state updates are a special case, and don't have a normal chain of
   headers, so process these seperately. */
    if (*pktTypePtr == TM_IPHC_CONTEXT_STATE)
    {
        goto iphcDecompressCtxState;
    }
    
/*
 * 3. Process IP header chain.  This could include any number of IPv4 and/or
 *    IPv6 headers (possibly tunneled) including IPv6 extension headers.
 */

/*
 *     3.1. If this is a compressed header, start processing at the beginning
 *          of the saved IP header chain.  Otherwise, start at the beginning
 *          of the full header sent by the peer.
 */
    if (pktDataPtr == TM_8BIT_NULL_PTR)
    {
/* start at beginning of full header. */
        pktDataPtr = packetPtr->pktLinkDataPtr;
        endPktDataPtr = packetPtr->pktLinkDataPtr + packetPtr->pktLinkDataLength;
    }

/* Determine the type of the first header - must be either IPv4 or IPv6. */
#ifdef TM_USE_IPV4
    if ( ((*pktDataPtr & 0xF0) >> 4) == TM_IP_4 )
    {
        nextHdrType = IPPROTO_IPV4;
    }
    else
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    if ( ((*pktDataPtr & 0xF0) >> 4) == TM_IP_6 )
    {
        nextHdrType = IPPROTO_IPV6;
    }
    else
#endif /* TM_USE_IPV6 */
    {
        errorCode = TM_EPROTONOSUPPORT;
        goto iphcDecompressExit;
    }
    
/*
 *     3.2. Iterate through the included headers until we reach a ULP header
 *          or have processed an IPv6 Fragment header, or the end of the packet
 *          or saved header chain.
 */
    ulpFound       = TM_8BIT_ZERO;
/* Assume that the ULP type is 'non-TCP' (e.g. UDP) until we find a
   TCP header. */
    ulpProtoType   = 0;
    availHdrLength = iphcVectPtr->iphcOpts.iphcoMaxHeader;
    while ( (ulpFound == TM_8BIT_ZERO) && (pktDataPtr < endPktDataPtr) )
    {
        switch (nextHdrType)
        {

#ifdef TM_USE_IPV4
/*     3.3. IPv4 header: */
            case IPPROTO_IPV4:
                ipHdrPtr = (ttIpHeaderPtr) pktDataPtr;
                hdrLength = tm_ip_hdr_len(ipHdrPtr->iphVersLen);

                if (hdrLength <= availHdrLength)
                {
                    nextHdrType = ipHdrPtr->iphUlp;
/*
 *         3.3.1. If this is the first header, save the Total Length field (for
 *                CID and/or generation).
 */
                    if (newPktType == 0)
                    {
                        ipHdrLengthPtr = &ipHdrPtr->iphTotalLength;
                        newPktType = TM_IPHC_IP;
                        origIpHdrPtr = ipHdrPtr;
                
/*
 *         3.3.2. Set the IPv4 Total Length to the actual length of the packet,
 *                including this IPv4 header.
 *                (this is done later, after the packet is decompressed).
 */
                    }
                        
/*
 *         3.3.3. If this is a compressed packet, and the IPv4 header does not
 *                include any options:
 */
                    if (compressedPkt)
                    {
                        if (    tm_ip_hdr_len(ipHdrPtr->iphVersLen)
                             == TM_4_IP_MIN_HDR_LEN )
                        {
/*
 *             3.3.3.1. Deal with the special case where the IPv4 header is
 *                      immediately followed by a TCP header.  In this case,
 *                      let the TCP decompression code deal with the IPv4 ID.
 *                      Otherwise, this is just another random field...
 */
                            if ( !(ipHdrPtr->iphUlp == IPPROTO_TCP) )
                            {
                                tm_bcopy(compHdrDataPtr,
                                         &ipHdrPtr->iphId,
                                         sizeof(tt16Bit));
                                compHdrDataPtr += sizeof(tt16Bit);
                            }
                        }
                        else
                        {
/*
 *         3.3.4. If this is a compressed packet, and the IPv4 header includes
 *                options (size > 20 bytes):
 *             3.3.4.1. The entire IPv4 header + options is included in the
 *                      compressed packet, so copy it into the context
 *                      structure.  Other headers following this one may still
 *                      be compressed, so continue to process header chain.
 */
                            tm_bcopy(compHdrDataPtr,
                                     ipHdrPtr,
                                     tm_ip_hdr_len(ipHdrPtr->iphVersLen));
                            compHdrDataPtr +=
                                tm_ip_hdr_len(ipHdrPtr->iphVersLen);
                        }
                    
/*         3.3.5. Save the next header type and the current header length. */
                        nextHdrType = ipHdrPtr->iphUlp;
                    }
                }
                break;
#endif /* TM_USE_IPV4 */
                
#ifdef TM_USE_IPV6
/*     3.4. IPv6 header: */
            case IPPROTO_IPV6:
                ip6HdrPtr   = (tt6IpHeaderPtr) pktDataPtr;                
                hdrLength    = TM_6_IP_MIN_HDR_LEN;

                if (hdrLength <= availHdrLength)
                {
                    nextHdrType = ip6HdrPtr->iph6Nxt;                    
/*
 *         3.4.1. If this is the first header, save the Payload Length field
 *                (for CID and/or generation)
 */
                    if (newPktType == 0)
                    {
                        ipHdrLengthPtr = &ip6HdrPtr->iph6Plen;
                        newPktType = TM_IPHC_IPV6;

/*
 *         3.4.2. Set the IPv6 Payload Length field to the actual length of
 *                the packet, excluding this IPv6 header.
 *                (this is done later, after the packet is decompressed)
 */
                    }
/*
 *         3.4.3. Nothing needs to be done for compressed IPv6 headers (all
 *                fields are NOCHANGE).
 */
                }
                break;

/*     3.5. IPv6 Hop-by-Hop, Destination options: */
            case IPPROTO_DSTOPTS:
            case IPPROTO_HOPOPTS:
                hdrLength = ((( *(pktDataPtr + 1)) + 1) << 3);

                if (hdrLength <= availHdrLength)
                {
                    nextHdrType = *pktDataPtr;
                    if (compressedPkt)
                    {
                        optDataPtr = pktDataPtr + 2;
                        endOptDataPtr = optDataPtr + (hdrLength - 2);
/*         3.5.1. Scan through individual options: */
                        while (optDataPtr < endOptDataPtr)
                        {
                            optType   = *optDataPtr;
/*             3.5.1.1. Skip over Pad1, PadN options. */
                            if (optType == TM_6_HDR_OPT_PAD1)
                            {
                                optLength = 1;
                            }
                            else if(optType == TM_6_HDR_OPT_PADN)
                            {
                                optLength = ((int) *(optDataPtr + 1)) + 2;
                            }
                            else
                            {
/*
 *             3.5.1.2. All other option values are RANDOM, so read the value
 *                      from the compressed packet and insert into the option
 *                      header.
 */
                                optLength = ((( *(optDataPtr + 1)) + 1) << 3);
                                tm_bcopy(compHdrDataPtr,
                                         optDataPtr + 2,
                                         optLength);
                                compHdrDataPtr += optLength;
                            }
                            
                            optDataPtr += optLength;
                        }
                    }
                }
                break;

/*     3.6. IPv6 Routing header: */
            case IPPROTO_ROUTING:
/*         3.6.1. Skip over Routing Header (all fields are NOCHANGE). */
                hdrLength = (tt8Bit) ((*(pktDataPtr + 1) + 1) << 3);
                nextHdrType = *pktDataPtr;
                break;

/*     3.7. IPv6 Fragment header: */
            case IPPROTO_FRAGMENT:
                hdrLength = TM_6_IP_FRAG_EXT_HDR_LEN;

                if (hdrLength <= availHdrLength)
                {
                    nextHdrType = *pktDataPtr;
/*
 *         3.7.1. Read the fragment offset, identification, Res field and M
 *                flag from compressed packet and insert into the fragment
 *                header.
 */
                    if (compressedPkt)
                    {
                        fragHdrPtr = (tt6FragmentExtHdrPtr) pktDataPtr;
                        tm_bcopy(compHdrDataPtr,
                                 &(fragHdrPtr->fra6OffsetFlags),
                                 sizeof (fragHdrPtr->fra6OffsetFlags));
                        compHdrDataPtr += sizeof (fragHdrPtr->fra6OffsetFlags);

                        tm_bcopy(compHdrDataPtr,
                                 &(fragHdrPtr->fra6Ident),
                                 sizeof(fragHdrPtr->fra6Ident));
                        compHdrDataPtr += sizeof(fragHdrPtr->fra6Ident);
                    }
                
/*
 *         3.7.2. Since headers after the Fragment header are not compressed,
 *                decompression stops once a Fragment header is encountered.
 */
                    ulpFound = TM_8BIT_YES;
                }
                break;
#endif /* TM_USE_IPV6 */
                
/*     3.8. IPSec Authentication header: */
            case IPPROTO_AH:
                ahHdrPtr = (ttAhHeaderPtr) pktDataPtr;
                hdrLength = ((((ahHdrPtr->ahPayloadLen + 1)) + 2) << 2);

                if (hdrLength <= availHdrLength)
                {
                    nextHdrType = ahHdrPtr->ahNextHeader;
/*
 *         3.8.1. Read authentication data from compressed packet, and insert
 *                into the authentication header.
 */
                    if (compressedPkt)
                    {
                        tm_bcopy(compHdrDataPtr,
                                 &(ahHdrPtr->ahSeq),
                                 hdrLength - 8);
                        compHdrDataPtr += (hdrLength - 8);
                    }
                }
                break;

/*     3.9. IPSec Encapsulating Security Payload Header: */
            case IPPROTO_ESP:
                hdrLength = sizeof(tt32Bit);
/*
 *         3.9.1. All data after the ESP is encrypted, and so is not
 *                compressed.
 */
                ulpFound = TM_8BIT_YES;
                break;

            case IPPROTO_TCP:
                ulpProtoType = IPPROTO_TCP;
                ulpFound = TM_8BIT_YES;
                hdrLength = 0;
                break;

            case IPPROTO_UDP:
                ulpProtoType = IPPROTO_UDP;
                ulpFound = TM_8BIT_YES;
                hdrLength = 0;
                break;
                
/* Unrecognized header, assume that this is the ULP. */
            default:
                hdrLength = 0;
                ulpFound = TM_8BIT_YES;
                break;
        }

/* Before moving on to next header, verify that we're not going to exceed the
   maximum header length.  If we have, stop decompression. */
        if ( (hdrLength <= availHdrLength) && (ulpFound == TM_8BIT_ZERO) )
        {
            pktDataPtr     += hdrLength;
            availHdrLength -= hdrLength;
        }
        else
        {
            ulpFound =  TM_8BIT_YES;
        }
        
    }

    if (pktDataPtr == endPktDataPtr)
    {
        ulpProtoType = nextHdrType;
    }

/* 4. If this was a full header: */
    if (*pktTypePtr == TM_IPHC_FULL_HEADER)
    {
/*
 *     4.1. If no context space has been allocated yet for this protocol (UDP
 *          or TCP) allocate enough room for the maximum number of contexts,
 *          including buffers to hold maximum size IP header chains.
 */
        if (ulpProtoType == IPPROTO_TCP)
        {
            if (iphcVectPtr->iphcTcpInCtxPtr == TM_TCP_CTX_NULL_PTR)
            {
                errorCode = tfIphcAllocCtx(iphcVectPtr,
                                           ulpProtoType,
                                           iphcVectPtr->iphcOpts.iphcoInTcpSpace,
                                           &(iphcVectPtr->iphcUdpInCtxPtr),
                                           &(iphcVectPtr->iphcTcpInCtxPtr));
                
            }

            contextId = (tt16Bit)(ntohs(*ipHdrLengthPtr) & 0x00ff);

            tcpCtxPtr = &(iphcVectPtr->iphcTcpInCtxPtr[contextId]);
            hdrLength = (ttPktLen) (pktDataPtr - packetPtr->pktLinkDataPtr);
            tm_bcopy(packetPtr->pktLinkDataPtr,
                     tcpCtxPtr->tctxIpHeaderPtr,
                     hdrLength);
            tcpCtxPtr->tctxIpHeaderLength = hdrLength;
        }
        else
        {
            if (iphcVectPtr->iphcUdpInCtxPtr == TM_UDP_CTX_NULL_PTR)
            {
                errorCode = tfIphcAllocCtx(iphcVectPtr,
                                           ulpProtoType,
                                           iphcVectPtr->iphcOpts.iphcoInUdpSpace,
                                           &(iphcVectPtr->iphcUdpInCtxPtr),
                                           &(iphcVectPtr->iphcTcpInCtxPtr));
            }

            if (    (ntohs(*ipHdrLengthPtr) & 0x8000)
                 && (ulpProtoType == IPPROTO_UDP) )
            {
/* 16-bit CID - can only be used with UDP, where we know the location
   of the length field. */
                if (ulpProtoType == IPPROTO_UDP)
                {
                    contextId = ntohs(((ttUdpHeaderPtr)pktDataPtr)->udpLength);
                }
                else
                {
                    errorCode = TM_EPROTONOSUPPORT;
                    goto iphcDecompressExit;
                }
            }
            else
            {
/* 8-bit CID */
                contextId = (tt16Bit)(ntohs(*ipHdrLengthPtr) & 0x00ff);
            }

            udpCtxPtr = &(iphcVectPtr->iphcUdpInCtxPtr[contextId]);

/*     4.2. Update the header length in the context structure. */
            hdrLength = (ttPktLen) (pktDataPtr - packetPtr->pktLinkDataPtr);
            tm_bcopy(packetPtr->pktLinkDataPtr,
                     udpCtxPtr->uctxIpHeaderPtr,
                     hdrLength);
            udpCtxPtr->uctxIpHeaderLength = hdrLength;
        }

        
    }
                       

/* 5. If incoming packet is non-TCP... */
    if (ulpProtoType != IPPROTO_TCP)
    {
/*
 *   5.1. If this is a full header, update the context with the new header 
 *        and the new generation number.
 */
        if (*pktTypePtr == TM_IPHC_FULL_HEADER)
        {
            generation = (tt8Bit)((ntohs(*ipHdrLengthPtr) & 0x3f00) >> 8);
            
            udpCtxPtr->uctxGeneration = generation;

            if (ulpProtoType == IPPROTO_UDP)
            {
                temp16 = (tt16Bit) (pktLen - udpCtxPtr->uctxIpHeaderLength);
                ((ttUdpHeaderPtr)pktDataPtr)->udpLength = htons(temp16);

                tm_bcopy(pktDataPtr,
                         &(udpCtxPtr->uctxUdpHeader),
                         TM_PAK_UDP_HDR_LEN);
            }

#ifdef TM_USE_IPV4
            if (newPktType == TM_IPHC_IP)
            {
                *ipHdrLengthPtr = htons(pktLen);
                origIpHdrPtr->iphChecksum = 0;
                origIpHdrPtr->iphChecksum =
                    tfIpHdr5Checksum((tt16BitPtr)origIpHdrPtr);
            }
            else
#endif /* TM_USE_IPV4 */
            {
#ifdef TM_USE_IPV6 
                temp16 = (tt16Bit) (pktLen - TM_6_IP_MIN_HDR_LEN);
                *ipHdrLengthPtr = htons(temp16);
#endif /* TM_USE_IPV6 */
            }
            
        }
        else
        {

/*
 *   5.2. Drop any compressed packets whose generation does not match the 
 *        generation in our context.  If the generations do not match it 
 *        means that we lost a full header and that our context is no 
 *        longer valid.
 */        
            if (generation != udpCtxPtr->uctxGeneration)
            {
                *pktTypePtr = (tt32Bit) TM_IPHC_ERROR;
                errorCode = TM_EINVAL;
                goto iphcDecompressExit;                
            }

            pktLen = (ttPktLen)
                (pktLen - (compHdrDataPtr - packetPtr->pktLinkDataPtr)
                + udpCtxPtr->uctxIpHeaderLength);
            
/*   5.3. If the upper layer protocol is UDP: */
            if (ulpProtoType == IPPROTO_UDP)
            {
/*
 *       5.3.1. Save the UDP checksum from the compressed header in our
 *              context.
 */
                tm_bcopy(compHdrDataPtr,
                         &(udpCtxPtr->uctxUdpHeader.udpChecksum),
                         sizeof(tt16Bit));
                compHdrDataPtr += sizeof(tt16Bit);
                pktLen -= sizeof(tt16Bit);
                
/*
 *       5.3.2. Infer the UDP payload lengths from the link layer frame
 *              length.
 */
                temp16 = (tt16Bit)
                    (pktLen + TM_PAK_UDP_HDR_LEN
                     - udpCtxPtr->uctxIpHeaderLength);
                udpCtxPtr->uctxUdpHeader.udpLength = htons(temp16);
/*
 *       5.3.3. Decompress a subheader following the UDP header, such as RTP, if 
 *              configured to do so.  Currently, this functionality is not
 *              supported as there is not subheader compression module.
 */
                compHdrDataPtr -= TM_PAK_UDP_HDR_LEN;
                tm_bcopy(&(udpCtxPtr->uctxUdpHeader),
                         compHdrDataPtr,
                         TM_PAK_UDP_HDR_LEN);

                pktLen += TM_PAK_UDP_HDR_LEN;
            }

#ifdef TM_USE_IPV4
            if (newPktType == TM_IPHC_IP)
            {
                *ipHdrLengthPtr = htons(pktLen);
                origIpHdrPtr->iphChecksum = 0;
                origIpHdrPtr->iphChecksum =
                    tfIpHdr5Checksum((tt16BitPtr)origIpHdrPtr);
            }
            else
#endif /* TM_USE_IPV4 */
            {
#ifdef TM_USE_IPV6
                temp16 = (tt16Bit) (pktLen - TM_6_IP_MIN_HDR_LEN);
                *ipHdrLengthPtr = htons(temp16);
#endif /* TM_USE_IPV6 */
            }
            
/*   5.4. Copy the full decompressed header to the packet. */
            hdrLength = udpCtxPtr->uctxIpHeaderLength;
            compHdrDataPtr -= hdrLength;
            tm_bcopy(udpCtxPtr->uctxIpHeaderPtr,
                     compHdrDataPtr,
                     hdrLength);

            packetPtr->pktLinkDataPtr = compHdrDataPtr;
            packetPtr->pktLinkDataLength = pktLen;
            packetPtr->pktChainDataLength = pktLen;
        }
    }
/* 6. If incoming packet is TCP... */
    else /* Packet is definitely TCP */
    {
/*     6.1. If this is a full header, update the context with the header. */
        if (*pktTypePtr == TM_IPHC_FULL_HEADER)
        {
            tcphPtr   = (ttTcpHeaderPtr) pktDataPtr;
          
            contextId = (tt16Bit)(ntohs(*ipHdrLengthPtr) & 0x00ff);
            tcpCtxPtr = &(iphcVectPtr->iphcTcpInCtxPtr[contextId]);

/* TCP options are sent as data, and so aren't saved in the context. */
            tm_bcopy(pktDataPtr,
                     &(tcpCtxPtr->tctxTcpHeader),
                     TM_PAK_TCP_HDR_LEN);

#ifdef TM_USE_IPV4
            if (newPktType == TM_IPHC_IP)
            {
                *ipHdrLengthPtr = htons(pktLen);
                origIpHdrPtr->iphChecksum = 0;
                origIpHdrPtr->iphChecksum =
                    tfIpHdr5Checksum((tt16BitPtr)origIpHdrPtr);
            }
            else
#endif /* TM_USE_IPV4 */
            {
#ifdef TM_USE_IPV6
                temp16 = (tt16Bit) (pktLen - TM_6_IP_MIN_HDR_LEN);
                *ipHdrLengthPtr = htons(temp16);
#endif /* TM_USE_IPV6 */
            }
            
        }
        else
        {
            seqDelta = 0;
            ackDelta = 0;
            winDelta = 0;

            ctxTcphPtr = &(tcpCtxPtr->tctxTcpHeader);

#ifdef TM_USE_IPV4
            idDelta  = 0;            
            ctxIphPtr  = (ttIpHeaderPtr) tcpCtxPtr->tctxIpHeaderPtr;
#endif /* TM_USE_IPV4 */

            ctxTcphPtr->tcpChecksum =
                (tt16Bit)((*compHdrDataPtr << 8) | (*(compHdrDataPtr+1)));
            compHdrDataPtr += 2;

/*
 *     6.2. If this is a compressed header, first check if this packet is for a 
 *          special case.  These cases are for common types of traffic and 
 *          in these special cases certain fields are omitted by the compressor 
 *          and must be reconstructed here.
 */
            if ((flagBits & TM_IPHCF_SAWU) == TM_IPHCF_SAWU)
            {
/*
 *         6.2.1. Unidirectional data transfer.  Change mask is set to SAWU.
 *                The sequence number has increased by the amount of data in
 *                the last packet.
 */
                flagBits &= ~TM_IPHCF_SAWU;
                seqDelta  = (ttS32Bit)tcpCtxPtr->tctxLastPayloadLen;

                tm_iphc_apply_ldelta(seqDelta, ctxTcphPtr->tcpSeqNo);

            }


            if ((flagBits & TM_IPHCF_SAWU) == TM_IPHCF_SWU)
            {
/*            
 *         6.2.2. Echoed interactive traffic.  Change mask is set to SWU.  The 
 *                sequence number and acknowledgement number has increased by
 *                the amount of data in the last packet.
 */                
                flagBits &= ~TM_IPHCF_SWU;
                seqDelta  = (ttS32Bit)tcpCtxPtr->tctxLastPayloadLen;
                ackDelta  = (ttS32Bit)tcpCtxPtr->tctxLastPayloadLen;

                tm_iphc_apply_ldelta(seqDelta, ctxTcphPtr->tcpSeqNo);
                tm_iphc_apply_ldelta(ackDelta, ctxTcphPtr->tcpAckNo);
            }
/*
 *     6.3. If this is a compressed packet that is not a special case, start 
 *          updating the context by examining the compressed header.  Based on 
 *          the change mask, update the TCP options, IPv4 ID (assumed to be one 
 *          if not included), sequence  number, acknowledgement number, window 
 *          size, urgent pointer, TCP reserved & IP traffic class field and TCP 
 *          checksum.
 */
    
/* 
 * We have to clear out the control bits field since the previous packet
 * could have been a full header with the SYN/FIN/ACK bits set
 */
            ctxTcphPtr->tcpControlBits = 0;
    
/* Urgent pointer */
            if (flagBits & TM_IPHCF_U)
            {
                tm_iphc_decode_delta(delta, compHdrDataPtr);
                ctxTcphPtr->tcpUrgentPtr    = delta;
                ctxTcphPtr->tcpControlBits |= TM_TCP_URG;
            }

/* Window size delta */
            if (flagBits & TM_IPHCF_W)
            {
#ifdef TM_LINT
LINT_PTR_CAST_BEGIN
#endif /* TM_LINT */

                tm_iphc_decode_sdelta(winDelta, compHdrDataPtr);
#ifdef TM_LINT
LINT_PTR_CAST_END
#endif /* TM_LINT */

                tm_iphc_apply_sdelta(winDelta, ctxTcphPtr->tcpWindowSize);
            }



/* Acknowledgement number delta */
            if (flagBits & TM_IPHCF_A)
            {
                tm_iphc_decode_delta(ackDelta, compHdrDataPtr);
                tm_iphc_apply_ldelta(ackDelta, ctxTcphPtr->tcpAckNo);
            } 

/* Sequence number delta */
            if (flagBits & TM_IPHCF_S)
            {
                tm_iphc_decode_delta(seqDelta, compHdrDataPtr);
                tm_iphc_apply_ldelta(seqDelta, ctxTcphPtr->tcpSeqNo);
            }

/* IP ID (assumed to be 1 if not included in compressed header) */
#ifdef TM_USE_IPV4
            if (newPktType == TM_IPHC_IP)
            {
                if (flagBits & TM_IPHCF_I)
                {
                    tm_iphc_decode_delta(idDelta, compHdrDataPtr);
                }
                else
                {
                    idDelta = 1;
                }

                tm_iphc_apply_sdelta(idDelta, ctxIphPtr->iphId);
            }
#endif /* TM_USE_IPV4 */
            
            pktLen =
                pktLen - (ttPktLen) (compHdrDataPtr - packetPtr->pktLinkDataPtr)
                + tcpCtxPtr->tctxIpHeaderLength + TM_PAK_TCP_HDR_LEN;

#ifdef TM_USE_IPV4
            if (newPktType == TM_IPHC_IP)
            {
                *ipHdrLengthPtr = htons(pktLen);
                origIpHdrPtr->iphChecksum = 0;
                origIpHdrPtr->iphChecksum =
                    tfIpHdr5Checksum((tt16BitPtr)origIpHdrPtr);
            }
            else
#endif /* TM_USE_IPV4 */
            {
#ifdef TM_USE_IPV6
                temp16 = (tt16Bit) (pktLen - TM_6_IP_MIN_HDR_LEN);
                *ipHdrLengthPtr = htons(temp16);
#endif /* TM_USE_IPV6 */
            }
            
/*     6.4. Copy the full decompressed header to the packet.  */
            
/* 
 * There is actually no need to deal with TCP options at all: whether or not
 * there are TCP options, 'dataPtr' currently points at the end of the 
 * main TCP header.  If there are TCP options present, dataPtr will point
 * to the beginning of the options, otherwise it will point to the payload
 * (the same place in the packet, i.e. 40 bytes from the start of the IP 
 * header.  The TCP data offset will be correct since every time this value
 * changes an uncompressed header is sent.
 */
            compHdrDataPtr -= TM_PAK_TCP_HDR_LEN;
            tm_bcopy(ctxTcphPtr, compHdrDataPtr, TM_PAK_TCP_HDR_LEN);
            tcphPtr  = (ttTcpHeaderPtr) compHdrDataPtr;


            hdrLength = tcpCtxPtr->tctxIpHeaderLength;
            compHdrDataPtr -= hdrLength;
            tm_bcopy(tcpCtxPtr->tctxIpHeaderPtr,
                     compHdrDataPtr,
                     hdrLength);

            packetPtr->pktLinkDataPtr = compHdrDataPtr;
            packetPtr->pktLinkDataLength = pktLen;
            packetPtr->pktChainDataLength = pktLen;

        
            *pktTypePtr = newPktType;
            
/* Set the ACK and (optionally) the PUSH flags */
            tcphPtr->tcpControlBits |= TM_TCP_ACK;
            if (flagBits & TM_IPHCF_P)
            {
                tcphPtr->tcpControlBits |= TM_TCP_PSH;
            }

            *pktTypePtr = newPktType;

/*     6.5. If TCP error recovery is enabled... */            
            if ( iphcVectPtr->iphcOpts.iphcoTcpErrRec == TM_8BIT_YES )
            {
/*         6.5.1. Compute the TCP checksum on the decompressed packet. */

                savLinkDataPtr = packetPtr->pktLinkDataPtr;
                hdrLength = tcpCtxPtr->tctxIpHeaderLength;
                tm_pkt_ulp_hdr(packetPtr, hdrLength);

#ifdef TM_USE_IPV4
                if (newPktType == TM_IPHC_IP)
                {
                    ipHdrPtr = (ttIpHeaderPtr) savLinkDataPtr;
                    savedIpHdrNxt = ipHdrPtr->iphUlp;
                    savedIpTtl = ipHdrPtr->iphTtl;
                    savedIpChecksum = ipHdrPtr->iphChecksum;
                    pshPtr = (ttPseudoHeaderPtr) (savLinkDataPtr + 8);
                    pshPtr->pshLayer4Len =
                        htons(packetPtr->pktChainDataLength);
                    pshPtr->pshZero = TM_8BIT_ZERO;
                    pshPtr->pshProtocol = IPPROTO_TCP;
                    pseudoHdrPtr = (tt16BitPtr) pshPtr;
                    pseudoHdrLen = tm_byte_count(TM_4PAK_PSEUDO_HDR_LEN);
                }
                else
#endif /* TM_USE_IPV4 */
                {
#ifdef TM_USE_IPV6
                    ip6HdrPtr = (tt6IpHeaderPtr) savLinkDataPtr;
                    saved6IpHops   = ip6HdrPtr->iph6Hops;
                    savedIpHdrNxt  = ip6HdrPtr->iph6Nxt;
                    saved6FlowInfo = ip6HdrPtr->iph6Flow;
                    psh6Ptr = (tt6PseudoHeaderPtr) savLinkDataPtr;
                    psh6Ptr->psh616BitLayer4Len =
                        htons(packetPtr->pktChainDataLength);
                    psh6Ptr->psh616Bit0Layer4Len = TM_16BIT_ZERO; 
                    tm_6_zero_arr(psh6Ptr->psh6ZeroArr);
                    psh6Ptr->psh6Protocol  = IPPROTO_TCP; 
                    pseudoHdrPtr = (tt16BitPtr)psh6Ptr;
                    pseudoHdrLen = TM_6_IP_MIN_HDR_LEN;
#endif /* TM_USE_IPV6 */
                }

                checksum = tfPacketChecksum(packetPtr, 
                                            packetPtr->pktChainDataLength,
                                            pseudoHdrPtr,
                                            pseudoHdrLen);

/*
 *         6.5.2. If the checksum fails, try to repair the TCP header.  This is 
 *                done by applying the delta values in the compressed header to 
 *                the current header again.  This will repair an error caused by 
 *                losing a previous compressed header that is identical to the 
 *                current one (very common in TCP bulk transfers).  The checksum 
 *                should be recomputed.
 */
                if (checksum != 0)
                {
                    tm_iphc_apply_sdelta(winDelta, tcphPtr->tcpWindowSize);
                    tm_iphc_apply_ldelta(ackDelta, tcphPtr->tcpAckNo);
                    tm_iphc_apply_ldelta(seqDelta, tcphPtr->tcpSeqNo);

#ifdef TM_USE_IPV4 
                    if (newPktType == TM_IPHC_IP)
                    {
                        tm_iphc_apply_sdelta(idDelta,  ipHdrPtr->iphId);
                    }
#endif /* TM_USE_IPV4 */

/*         6.5.3. If the checksum still fails, perform the above step again. */
                    checksum = tfPacketChecksum(packetPtr, 
                                                packetPtr->pktChainDataLength,
                                                pseudoHdrPtr,
                                                pseudoHdrLen);
                    if (checksum != 0)
                    {
                        tm_iphc_apply_sdelta(winDelta, tcphPtr->tcpWindowSize);
                        tm_iphc_apply_ldelta(ackDelta, tcphPtr->tcpAckNo);
                        tm_iphc_apply_ldelta(seqDelta, tcphPtr->tcpSeqNo);

#ifdef TM_USE_IPV4
                        if (newPktType == TM_IPHC_IP)
                        {
                            tm_iphc_apply_sdelta(idDelta, ipHdrPtr->iphId);
                        }
#endif /* TM_USE_IPV4 */

/*
 *         6.5.4. If the checksum is still invalid, send a 'CONTEXT_STATE'
 *                packet back to the compressor which will request that a
 *                full header be sent for this context.
 */                
                        checksum = 
                            tfPacketChecksum(packetPtr, 
                                             packetPtr->pktChainDataLength,
                                             pseudoHdrPtr,
                                             pseudoHdrLen);
                        if (checksum != 0)
                        {
/* TBD: Send a CONTEXT_STATE message */
                            *pktTypePtr = (tt32Bit) TM_IPHC_ERROR;
                            errorCode = TM_EINVAL;
                            goto iphcDecompressExit;
                        }
                    }
                }

/* Update the context with the fixed TCP state. */
                tm_bcopy(packetPtr->pktLinkDataPtr,
                         ctxTcphPtr,
                         TM_PAK_TCP_HDR_LEN);

/* Restore fields overwritten by pseudoheader */
#ifdef TM_USE_IPV4
                if (newPktType == TM_IPHC_IP)
                {
                    ipHdrPtr->iphTtl = savedIpTtl;
                    ipHdrPtr->iphUlp = savedIpHdrNxt;
                    ipHdrPtr->iphChecksum = savedIpChecksum;
                }
                else
#endif /* TM_USE_IPV4 */
                {
#ifdef TM_USE_IPV6 
                    ip6HdrPtr->iph6Hops = saved6IpHops;
                    ip6HdrPtr->iph6Nxt  = savedIpHdrNxt;
                    ip6HdrPtr->iph6Flow = saved6FlowInfo;
                    ip6HdrPtr->iph6Plen = htons(packetPtr->pktChainDataLength);
#endif /* TM_USE_IPV6 */
                }

/* Move packet back to IP header */
                hdrLength = tcpCtxPtr->tctxIpHeaderLength;
                tm_pkt_llp_hdr(packetPtr, hdrLength);
            }
        }

/* 
 * Save the length of the payload for this packet, to be used for the
 * special TCP cases (only if decompression was successful).
 */
        if (errorCode == TM_ENOERROR)
        {
            hdrLength = tcpCtxPtr->tctxIpHeaderLength;
            tcpCtxPtr->tctxLastPayloadLen = packetPtr->pktChainDataLength
                - hdrLength - tm_tcp_hdrLen(tcphPtr->tcpDataOffset);
        }
    }
    

iphcDecompressCtxState:
/* 7. If incoming packet is a TCP CONTEXT_STATE packet... */    
    if (*pktTypePtr == TM_IPHC_CONTEXT_STATE)
    {
/*
 *     7.1. If this is a TCP header request, search through the list of CID's 
 *           in the request and mark each context as invalid (i.e., the next 
 *           packet sent with this context must be sent with a full header).
 */
        if (*pktDataPtr == 3)
        {

/* Indicates number of CID's in this header request */
            pktLen = *(pktDataPtr+1);

            for (i=0; i < pktLen; i++)
            {
                contextId = *(pktDataPtr+2+i);

                tcpCtxPtr = iphcVectPtr->iphcTcpOutCtxPtr;                

                while (tcpCtxPtr != 0)
                {

                    if (tcpCtxPtr->tctxCid == contextId)
                    {
                        tcpCtxPtr->tctxValid = TM_8BIT_ZERO;
                        break;
                    }

                    tcpCtxPtr = tcpCtxPtr->tctxNextPtr;
                }

            }

        }

        *pktTypePtr = (tt32Bit) TM_IPHC_ERROR;
        errorCode = TM_EINVAL;
        goto iphcDecompressExit;
    }
    
/* 8. Return type of decompressed packet, if decompression was successful. */    

    if (errorCode == TM_ENOERROR)
    {
        *pktTypePtr = newPktType;
    }
    
iphcDecompressExit:

/*
 * Debugging code - compare the buffer containing the full packet which was
 * saved in tfIphcCompress to the decompressed packet to see exactly where
 * decompression went wrong (if it did).
 * Normally disabled (TM_IPHC_DEBUG not defined).
 */ 
#ifdef TM_IPHC_DEBUG
    if (errorCode == TM_ENOERROR)
    {
        int       i;
        tt8BitPtr tempDataPtr;

        if (outDataLen != packetPtr->pktChainDataLength)
        {
            tfKernelError("tfiphcDecompress","Bad decompressed data length");
        }
        tempDataPtr = packetPtr->pktLinkDataPtr;
        for (i=0;i<(int)outDataLen;i++)
        {
            if (outDataBuf[i] != tempDataPtr[i])
            {
                tfKernelError("tfIphcDecompress","Bad decompressed data");
            }
        }
    }
#endif /* TM_IPHC_DEBUG */
    
    return errorCode;
}

                             
/*
 * tfIphcFindContext Function Description
 * Determines the compression context for specified outgoing packet.  
 *
 * 1. ERROR CHECKING: all parameters are not NULL.
 * 2. Scan through the outgoing headers to retrieve the information necessary
 *    to find a context (inner and outer source and destination addresses,
 *    upper layer protocol, inner and outer protocol families, and upper layer
 *    port numbers):
 *     2.1. IPv4 header:
 *         2.1.1. If this is the first header (the outer protocol family has
 *                not yet been set) save the IPv4 source and destination
 *                addresses as the outer addresses and set the outer protocol
 *                family to IPv4.
 *         2.1.2. If this is the second header (the outer protocol family has
 *                been set, but the inner family has not), save the IPv4 source
 *                and destination addresses as the inner addresses and set the
 *                inner protocol to IPv4.
 *     2.2. IPv6 header:
 *         2.2.1. If this is the first header (the outer protocol family has
 *                not yet been set) save the IPv6 source and destination
 *                addresses as the outer addresses and set the outer protocol
 *                family to IPv6.
 *         2.2.2. If this is the second header (the outer protocol family has
 *                been set, but the inner family has not), save the IPv6 source
 *                and destination addresses as the inner addresses and set the
 *                inner protocol to IPv6.
 *     2.3. IPv6 Hop-By-Hop, Destination options:
 *         2.3.1. Skip this header, as it is not used in determining context.
 *     2.4. IPv6 Routing:
 *         2.4.1. Skip this header, as it is not used in determining context.
 *                (The last address in a Type 0 routing header can be used as
 *                a defining field, but we don't implement this).
 *     2.5. IPv6 Fragment:
 *         2.5.1. For the purpose of determining context, we consider a
 *                Fragment header to be a ULP.  This guarantees that fragmented
 *                packets are never mixed with non-fragmented packets in the
 *                same context.
 *     2.6. IPSec AH:
 *         2.6.1. Skip this header, as it is not used in determining context.
 *     2.7. IPSec ESP:
 *         2.7.1. Since all data following the ESP header is encrypted, we
 *                consider a ESP to be a ULP.
 *     2.8. Unrecognized header: assume that this is the ULP header.
 *         2.8.1. If this is a UDP or TCP header, get the source and destination
 *                port numbers.
 *         2.8.2. Otherwise, set the source and destination ports to zero.
 * 3. If no context list for this ULP (either TCP or non-TCP (UDP)) allocate
 *    one now and use the first entry.  Set the "new context" flag to true.
 * 4. Otherwise, if the list already existed:
 *     4.1. Search through the context list for an entry which matches the
 *          information acquired from the packet above.  The fields should be
 *          checked in an order that will disqualify non-matching contexts as
 *          quickly as possible:
 *         4.1.1. Upper layer protocol.
 *         4.1.2. Outer protocol family (#if TM_USE_IPV4 && TM_USE_IPV6)
 *         4.1.3. Outer source and destination addresses.
 *         4.1.4. Inner protocol family (#if TM_USE_IPV4 && TM_USE_IPV6)
 *         4.1.5. Inner source and destination addresses, if this packet is
 *                tunneled (inner protocol family != 0)
 *         4.1.6. Upper layer source and destination ports.
 *     4.2. If no entry is found, choose the least recently used entry (the
 *          last in the list).  Set the "new context" flag to true.  If this
 *          is a non-UDP, non-TCP stream only pick an 8-bit CID, since we may
 *          not know where the second length field is.
 *     4.3. Otherwise, if an entry is found set the "new context" flag to false.
 *     4.4. Move this entry to the beginning of the list since it is now the
 *          most recently used.  Adjust the least recently used pointer.
 * 5. Return the ULP type.
 * 
 * Parameters
 * Parameter          Description
 * packetPtr          Pointer to packet structure associated with this 
 *                    outgoing data
 * iphcVectPtr        Current IPHC state vector.
 * ctxPtrPtr          Returns a pointer to the requested context.
 * ulpTypePtr         Returns the ULP type for this packet.  Initially should
 *                    be set to the first header type (IPv4 or IPv6)
 * newCtxFlagPtr      Specifies if the context being returned is "new" -
 *                    either a new entry or a reused old entry.
 * 
 * Return value       Meaning
 * TM_ENOERROR        List successfully searched - if context was found, 
 *                    *ctxPtrPtr will be set; if not found *ctxPtrPtr will be 
 *                    set to zero.
 * TM_EINVAL          Invalid packet pointer.
 * TM_EINVAL          Invalid data pointer.
 * TM_EINVAL          Invalid state vector pointer.
 * TM_EINVAL          Invalid context pointer pointer.
 * TM_EPROTONOSUPPORT Packet is not UDP or TCP
 */
static int tfIphcFindContext(ttPacketPtr       packetPtr,
                             ttIphcVectPtr     iphcVectPtr,
                             ttVoidPtrPtr      ctxPtrPtr,
                             tt8BitPtr         ulpTypePtr,
                             tt8BitPtr         newCtxFlagPtr,
                             ttPktLen TM_FAR * totalHdrLenPtr)    
{
#ifdef TM_USE_IPV4
    ttIpHeaderPtr       ipHdrPtr;    
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    tt6IpHeaderPtr      ip6HdrPtr;    
#endif /* TM_USE_IPV6 */
    int                 errorCode;
    ttIphcTcpContextPtr tcpCtxPtr;
    ttIphcUdpContextPtr udpCtxPtr;
    ttIphcTcpContextPtr lastTcpCtxPtr;
    ttIphcUdpContextPtr lastUdpCtxPtr;
    ttIphcCtxInfoPtr    ctxInfoPtr;
    ttPacketPtr         linkPktPtr;
    tt8BitPtr           linkDataPtr;
    ttPktLen            linkDataLength;
    ttPktLen            availHdrLength;
    ttPktLen            hdrLength;
    tt16Bit             srcPort;
    tt16Bit             destPort;
    tt8Bit              nextHdrType;
    tt8Bit              outerProtoFamily;
    tt8Bit              innerProtoFamily;
    tt8Bit              ulpFound;
    tt8Bit              ulpProtoType;
    tt8Bit              entryFound;
    ttIpAddress         innerDestAddr;
    ttIpAddress         innerSrcAddr;
    ttIpAddress         outerDestAddr;
    ttIpAddress         outerSrcAddr;
    
    errorCode = TM_ENOERROR;

/* 1. ERROR CHECKING: all parameters are not NULL. */
#ifdef TM_ERROR_CHECKING
    if ( (packetPtr   == TM_PACKET_NULL_PTR) ||
         (iphcVectPtr == TM_IPHC_VECT_NULL_PTR) ||
         (ctxPtrPtr   == TM_VOID_NULL_PTRPTR) )
    {
        errorCode = TM_EINVAL;
        goto iphcFindContextExit;
    }
#endif /* TM_ERROR_CHECKING */

    udpCtxPtr = TM_UDP_CTX_NULL_PTR;
    tcpCtxPtr = TM_TCP_CTX_NULL_PTR;
    destPort  = 0;
    srcPort   = 0;
    ulpProtoType = 0;

#ifdef TM_USE_IPV6
    tm_6_ip_copy_structs(in6addr_any, innerDestAddr);
    tm_6_ip_copy_structs(in6addr_any, innerSrcAddr);
    tm_6_ip_copy_structs(in6addr_any, outerDestAddr);
    tm_6_ip_copy_structs(in6addr_any, outerSrcAddr);    
#else /* !TM_USE_IPV6 */
    tm_ip_copy(TM_IP_ZERO, innerDestAddr);
    tm_ip_copy(TM_IP_ZERO, innerSrcAddr);
    tm_ip_copy(TM_IP_ZERO, outerDestAddr);
    tm_ip_copy(TM_IP_ZERO, outerSrcAddr);    
#endif /* TM_USE_IPV6 */
       
    
    
/*
 * 2. Scan through the outgoing headers to retrieve the information necessary
 *    to find a context (inner and outer source and destination addresses,
 *    upper layer protocol, inner and outer protocol families, and upper layer
 *    port numbers):
 */
    linkPktPtr     = packetPtr;
    linkDataPtr    = packetPtr->pktLinkDataPtr;
    linkDataLength = packetPtr->pktLinkDataLength;
    nextHdrType    = *ulpTypePtr;

    ulpFound = TM_8BIT_ZERO;
    availHdrLength = iphcVectPtr->iphcOpts.iphcoMaxHeader;

    outerProtoFamily = 0;
    innerProtoFamily = 0;
    
    while ( ulpFound == TM_8BIT_ZERO )
    {
        switch (nextHdrType)
        {

#ifdef TM_USE_IPV4
/*     2.1. IPv4 header: */
            case IPPROTO_IPV4:
                hdrLength = TM_4_IP_MIN_HDR_LEN;

                if (hdrLength <= availHdrLength)
                {
                    ipHdrPtr = (ttIpHeaderPtr) linkDataPtr;
                    nextHdrType = ipHdrPtr->iphUlp;
/*
 *         2.1.1. If this is the first header (the outer protocol family has
 *                not yet been set) save the IPv4 source and destination
 *                addresses as the outer addresses and set the outer protocol
 *                family to IPv4.
 */
                    if (outerProtoFamily == 0)
                    {
                        outerProtoFamily = IPPROTO_IPV4;
#ifdef TM_USE_IPV6
                        tm_6_addr_to_ipv4_mapped(ipHdrPtr->iphSrcAddr,
                                                 &outerSrcAddr);
                        tm_6_addr_to_ipv4_mapped(ipHdrPtr->iphDestAddr,
                                                 &outerDestAddr);
#else /* TM_USE_IPV6 */
                        tm_ip_copy(ipHdrPtr->iphSrcAddr, outerSrcAddr);
                        tm_ip_copy(ipHdrPtr->iphDestAddr, outerDestAddr);
#endif /* TM_USE_IPV6 */
                    }
                    else if (innerProtoFamily == 0)
                    {
/*
 *         2.1.2. If this is the second header (the outer protocol family has
 *                been set, but the inner family has not), save the IPv4 source
 *                and destination addresses as the inner addresses and set the
 *                inner protocol to IPv4.
 */
                        innerProtoFamily = IPPROTO_IPV4;
#ifdef TM_USE_IPV6
                        tm_6_addr_to_ipv4_mapped(ipHdrPtr->iphSrcAddr,
                                                 &innerSrcAddr);
                        tm_6_addr_to_ipv4_mapped(ipHdrPtr->iphDestAddr,
                                                 &innerDestAddr);
#else /* TM_USE_IPV6 */
                        tm_ip_copy(ipHdrPtr->iphSrcAddr, innerSrcAddr);
                        tm_ip_copy(ipHdrPtr->iphDestAddr, innerDestAddr);
#endif /* TM_USE_IPV6 */
                    }
                }
                break;
#endif /* TM_USE_IPV4 */
                
#ifdef TM_USE_IPV6
/*     2.2. IPv6 header: */
            case IPPROTO_IPV6:
                hdrLength = TM_6_IP_MIN_HDR_LEN;

                if (hdrLength <= availHdrLength)
                {
                    ip6HdrPtr = (tt6IpHeaderPtr) linkDataPtr;
                    nextHdrType = ip6HdrPtr->iph6Nxt;
/*
 *         2.2.1. If this is the first header (the outer protocol family has
 *                not yet been set) save the IPv6 source and destination
 *                addresses as the outer addresses and set the outer protocol
 *                family to IPv6.
 */
                    if (outerProtoFamily == 0)
                    {
                        outerProtoFamily = IPPROTO_IPV6;
                        tm_6_ip_copy(
                            &tm_6_ip_hdr_in6_addr(ip6HdrPtr->iph6SrcAddr),
                            &outerSrcAddr);
                        tm_6_ip_copy(
                            &tm_6_ip_hdr_in6_addr(ip6HdrPtr->iph6DestAddr),
                            &outerDestAddr);
                    }
                    else if (innerProtoFamily == 0)
                    {
/*
 *         2.2.2. If this is the second header (the outer protocol family has
 *                been set, but the inner family has not), save the IPv6 source
 *                and destination addresses as the inner addresses and set the
 *                inner protocol to IPv6.
 */
                        innerProtoFamily = IPPROTO_IPV6;
                        tm_6_ip_copy(
                            &tm_6_ip_hdr_in6_addr(ip6HdrPtr->iph6SrcAddr),
                            &innerSrcAddr);
                        tm_6_ip_copy(
                            &tm_6_ip_hdr_in6_addr(ip6HdrPtr->iph6DestAddr),
                            &innerDestAddr);
                    }
                }
                break;
                
/*     2.3. IPv6 Hop-By-Hop, Destination options: */
            case IPPROTO_HOPOPTS:
            case IPPROTO_DSTOPTS:
/*         2.3.1. Skip this header, as it is not used in determining context. */
                hdrLength = ((( *(linkDataPtr + 1)) + 1) << 3);
                nextHdrType = *linkDataPtr;
                break;
    
/*     2.4. IPv6 Routing: */
            case IPPROTO_ROUTING:
/*
 *         2.4.1. Skip this header, as it is not used in determining context.
 *                (The last address in a Type 0 routing header can be used as
 *                a defining field, but we don't implement this).
 */
                hdrLength = (tt8Bit) ((*(linkDataPtr + 1) + 1) << 3);
                nextHdrType = *linkDataPtr;
                break;
                
/*     2.5. IPv6 Fragment: */
            case IPPROTO_FRAGMENT:
/*
 *         2.5.1. For the purpose of determining context, we consider a
 *                Fragment header to be a ULP.  This guarantees that fragmented
 *                packets are never mixed with non-fragmented packets in the
 *                same context.
 */
                hdrLength = TM_6_IP_FRAG_EXT_HDR_LEN;
                ulpProtoType = IPPROTO_FRAGMENT;
                ulpFound = TM_8BIT_YES;
                break;
#endif /* TM_USE_IPV6 */
                
/*     2.6. IPSec AH: */
            case IPPROTO_AH:
/*         2.6.1. Skip this header, as it is not used in determining context. */
                nextHdrType = *linkDataPtr;
                hdrLength = (tt8Bit) ((((tt32Bit) *(linkDataPtr + 1)) + 2) << 2);
                break;
    
/*     2.7. IPSec ESP: */
            case IPPROTO_ESP:
/*    
 *         2.7.1. Since all data following the ESP header is encrypted, we
 *                consider a ESP to be a ULP.
 */
                hdrLength = sizeof(tt32Bit);
                ulpProtoType = IPPROTO_ESP;
                ulpFound = TM_8BIT_YES;
                break;
                
/*     2.8. Unrecognized header: assume that this is the ULP header. */
            default:
                ulpProtoType = nextHdrType;
                ulpFound = TM_8BIT_YES;
                hdrLength = 0;
/*
 *         2.8.1. If this is a UDP or TCP header, get the source and destination
 *                port numbers.
 */
                if (ulpProtoType == IPPROTO_TCP)
                {
                    srcPort = ((ttTcpHeaderPtr)linkDataPtr)->tcpSrcPort;
                    destPort = ((ttTcpHeaderPtr)linkDataPtr)->tcpDstPort;
                }
                else if (ulpProtoType == IPPROTO_UDP)
                {
                    srcPort = ((ttUdpHeaderPtr)linkDataPtr)->udpSrcPort;
                    destPort = ((ttUdpHeaderPtr)linkDataPtr)->udpDstPort;
                }
/*         2.8.2. Otherwise, keep the source and destination ports at zero. */
        }

/* Verify that there is enough space to compress this header.  Also, do not
   advance past the upper layer protocol header. */
        if ( (hdrLength <= availHdrLength) && (ulpFound == TM_8BIT_ZERO) )
        {
            availHdrLength -= hdrLength;
            if (hdrLength < linkDataLength)
            {
                linkDataLength -= hdrLength;
                linkDataPtr += hdrLength;
            }
            else
            {
/* The next header lies in the next scattered buffer, so move to the next
   packet. */
                if (linkPktPtr->pktLinkNextPtr == TM_PACKET_NULL_PTR)
                {
                    tfKernelError("tfIphcFindContext",
                                  "Bad header chain");
                }

                linkPktPtr = linkPktPtr->pktLinkNextPtr;
                linkDataPtr =
                    linkPktPtr->pktLinkDataPtr + (hdrLength - linkDataLength);
                linkDataLength =
                    linkPktPtr->pktLinkDataLength - (hdrLength - linkDataLength);
            }
        }
        else
        {
/* Maximum header size reached, so stop compressing. */
            ulpFound = TM_8BIT_YES;
        }
    }

    *totalHdrLenPtr = iphcVectPtr->iphcOpts.iphcoMaxHeader - availHdrLength;
    
/*
 * 3. If no context list for this ULP (either TCP or non-TCP (UDP)) allocated
 *    one now and use the first entry.  Set the "new context" flag to true.
 */
    if (ulpProtoType == IPPROTO_TCP)
    {
        if (iphcVectPtr->iphcTcpOutCtxPtr == (ttIphcTcpContextPtr)0)
        {
            errorCode = tfIphcAllocCtx(iphcVectPtr,
                                       ulpProtoType,
                                       iphcVectPtr->iphcOpts.iphcoOutTcpSpace,
                                       &(iphcVectPtr->iphcUdpOutCtxPtr),
                                       &(iphcVectPtr->iphcTcpOutCtxPtr));
        }
        tcpCtxPtr = iphcVectPtr->iphcTcpOutCtxPtr;
        ctxInfoPtr = &(tcpCtxPtr->tctxContextInfo);
    }
    else
    {
        if (iphcVectPtr->iphcUdpOutCtxPtr == (ttIphcUdpContextPtr)0)
        {
            errorCode = tfIphcAllocCtx(iphcVectPtr,
                                       ulpProtoType,
                                       iphcVectPtr->iphcOpts.iphcoOutUdpSpace,
                                       &(iphcVectPtr->iphcUdpOutCtxPtr),
                                       &(iphcVectPtr->iphcTcpOutCtxPtr));
        }
        udpCtxPtr = iphcVectPtr->iphcUdpOutCtxPtr;
        ctxInfoPtr = &(udpCtxPtr->uctxContextInfo);
    }

    if (errorCode != TM_ENOERROR)
    {
        goto iphcFindContextExit;
    }
    
    entryFound = TM_8BIT_ZERO;

    lastUdpCtxPtr = TM_UDP_CTX_NULL_PTR;
    lastTcpCtxPtr = TM_TCP_CTX_NULL_PTR;

/* 4. Otherwise, if the list already existed: */
    while (ctxInfoPtr != (ttIphcCtxInfoPtr)0)
    {
/*
 *     4.1. Search through the context list for an entry which matches the
 *          information acquired from the packet above.  The fields should be
 *          checked in an order that will disqualify non-matching contexts as
 *          quickly as possible:
 */
    
/*         4.1.1. Upper layer protocol. */
        if (    (ulpProtoType == ctxInfoPtr->ctxUpperLayerProto)

/*         4.1.2. Outer protocol family */
             && (outerProtoFamily == ctxInfoPtr->ctxOuterProtoFamily)

/*         4.1.3. Outer source and destination addresses. */
#ifdef TM_USE_IPV6
             && (tm_6_ip_match(&outerSrcAddr,
                               &ctxInfoPtr->ctxOuterSrcAddr))
             && (tm_6_ip_match(&outerDestAddr,
                               &ctxInfoPtr->ctxOuterDestAddr))
#else /* !TM_USE_IPV6 */
             && (tm_ip_match(outerSrcAddr,
                             ctxInfoPtr->ctxOuterSrcAddr))
             && (tm_ip_match(outerDestAddr,
                             ctxInfoPtr->ctxOuterDestAddr))
#endif /* TM_USE_IPV6 */

/*         4.1.4. Inner protocol family */
             && (innerProtoFamily == ctxInfoPtr->ctxInnerProtoFamily)

/*
 *         4.1.5. Inner source and destination addresses, if this packet is
 *                tunneled (inner protocol family != 0)
 */
             && (    (    (innerProtoFamily != 0)
#ifdef TM_USE_IPV6
                       && (tm_6_ip_match(&innerSrcAddr,
                                         &ctxInfoPtr->ctxInnerSrcAddr))
                       && (tm_6_ip_match(&outerDestAddr,
                                         &ctxInfoPtr->ctxOuterDestAddr))
#else /* !TM_USE_IPV6 */
                       && (tm_ip_match(outerSrcAddr,
                                       ctxInfoPtr->ctxOuterSrcAddr))
                       && (tm_ip_match(outerDestAddr,
                                       ctxInfoPtr->ctxOuterDestAddr)) 
#endif /* TM_USE_IPV6 */
                         )
                  || (innerProtoFamily == 0) )
    
/*         4.1.6. Upper layer source and destination ports. */
             && (srcPort == ctxInfoPtr->ctxSourcePort)
             && (destPort == ctxInfoPtr->ctxDestPort) )
        {
/* entry found. */
            entryFound = TM_8BIT_YES;
            break;
        }

/* Move to next entry in list. */
        ctxInfoPtr = (ttIphcCtxInfoPtr)0;
        if (ulpProtoType == IPPROTO_TCP)
        {
            if (tcpCtxPtr->tctxNextPtr != TM_TCP_CTX_NULL_PTR)
            {
                lastTcpCtxPtr = tcpCtxPtr;
                tcpCtxPtr = tcpCtxPtr->tctxNextPtr;
                ctxInfoPtr = &(tcpCtxPtr->tctxContextInfo);
            }
            else
            {
                break;
            }
        }
        else
        {
            if (udpCtxPtr->uctxNextPtr != TM_UDP_CTX_NULL_PTR)
            {
                lastUdpCtxPtr = udpCtxPtr;
                udpCtxPtr = udpCtxPtr->uctxNextPtr;
                ctxInfoPtr = &(udpCtxPtr->uctxContextInfo);
            }
            else
            {
                break;
            }
        }
    }
    
/*
 *     4.2. If no entry is found, choose the least recently used entry (the
 *          last in the list).  Set the "new context" flag to true.  If this
 *          is a non-UDP, non-TCP stream only pick an 8-bit CID, since we may
 *          not know where the second length field is.
 */
    if (entryFound == TM_8BIT_ZERO)
    {
/* Update context info. */
        if (ulpProtoType == IPPROTO_TCP)
        {
            ctxInfoPtr = &(tcpCtxPtr->tctxContextInfo);
        }
        else
        {
            ctxInfoPtr = &(udpCtxPtr->uctxContextInfo);
        }

        ctxInfoPtr->ctxSourcePort = srcPort;
        ctxInfoPtr->ctxDestPort   = destPort;
        ctxInfoPtr->ctxUpperLayerProto  = ulpProtoType;
        ctxInfoPtr->ctxOuterProtoFamily = outerProtoFamily;
        ctxInfoPtr->ctxInnerProtoFamily = innerProtoFamily;

#ifdef TM_USE_IPV6
        tm_6_ip_copy(&outerSrcAddr, &ctxInfoPtr->ctxOuterSrcAddr);
        tm_6_ip_copy(&outerDestAddr, &ctxInfoPtr->ctxOuterDestAddr);
/* tunneled packet. */
        if (innerProtoFamily != 0)
        {
            tm_6_ip_copy(&innerSrcAddr, &ctxInfoPtr->ctxInnerSrcAddr);
            tm_6_ip_copy(&innerDestAddr, &ctxInfoPtr->ctxInnerDestAddr);
        }
#else /* !TM_USE_IPV6 */
        tm_ip_copy(outerSrcAddr, ctxInfoPtr->ctxOuterSrcAddr);
        tm_ip_copy(outerDestAddr, ctxInfoPtr->ctxOuterDestAddr);
/* tunneled packet. */
        if (innerProtoFamily != 0)
        {
            tm_ip_copy(innerSrcAddr, ctxInfoPtr->ctxInnerSrcAddr);
            tm_ip_copy(innerDestAddr, ctxInfoPtr->ctxInnerDestAddr);
        }
#endif /* TM_USE_IPV6 */
        
        *newCtxFlagPtr = TM_8BIT_YES;
    }
    else
    {
/*
 *     4.3. Otherwise, if an entry is found set the "new context" flag to
 *          false.
 */
        *newCtxFlagPtr = TM_8BIT_ZERO;
    }
    
/*
 *     4.4. Move this entry to the beginning of the list since it is now the
 *          most recently used.  Adjust the least recently used pointer.
 */
    if (ulpProtoType == IPPROTO_TCP)
    {
        if (lastTcpCtxPtr != TM_TCP_CTX_NULL_PTR)
        {
/* Entry already at beginning of list, no need to do anything. */
            lastTcpCtxPtr->tctxNextPtr = tcpCtxPtr->tctxNextPtr;
            tcpCtxPtr->tctxNextPtr  = iphcVectPtr->iphcTcpOutCtxPtr;
            iphcVectPtr->iphcTcpOutCtxPtr = tcpCtxPtr;
        }
        *ctxPtrPtr = (ttVoidPtr)tcpCtxPtr;
    }
    else
    {
        if (lastUdpCtxPtr != TM_UDP_CTX_NULL_PTR) 
        {
/* Entry already at beginning of list, no need to do anything. */            
            lastUdpCtxPtr->uctxNextPtr = udpCtxPtr->uctxNextPtr;
            udpCtxPtr->uctxNextPtr     = iphcVectPtr->iphcUdpOutCtxPtr;
            iphcVectPtr->iphcUdpOutCtxPtr = udpCtxPtr;
        }
        *ctxPtrPtr = (ttVoidPtr)udpCtxPtr;
    }

    
/* 5. Return the ULP type and total header length. */
    *ulpTypePtr = ulpProtoType;

iphcFindContextExit:
    
    return errorCode;
}



/*
 * tfIphcInit Function Description
 * Initializes the IP Header Compression modules.  Allocates a new IPHC state 
 * vector and returns the address in iphcVectPtrPtr.
 *
 * 1.  ERROR CHECKING: All parameters are not null.
 * 2.  Allocate a new ttIphcVect structure. 
 * 3.  Clear the new state vector.
 * 4.  Use the default values for IPHC parameters, as given by RFC-2507.
 *
 * Parameters
 * Parameter       Description
 * iphcVectPtrPtr  Pointer to a IPHC state vector pointer.  This will be set 
 *                 to point to the new IPHC state vector if this routine is 
 *                 successful.
 * Return Value    Meaning
 * TM_ENOERROR     Successful
 * TM_ENOBUFS      No memory available for state structure.
 * TM_EINVAL       Invalid state vector pointer pointer.
 */
static int tfIphcInit(ttIphcVectPtrPtr iphcVectPtrPtr)
{
    ttIphcVectPtr vectPtr;
    int           errorCode;


/* 1.  ERROR CHECKING: All parameters are not null. */
#ifdef TM_ERROR_CHECKING
    if (iphcVectPtrPtr == TM_IPHC_VECT_NULL_PTRPTR)
    {
        errorCode = TM_EINVAL;
    }
    else
#endif /* TM_ERROR_CHECKING */
    {

/* 2.  Allocate a new ttIphcVect structure, if necessary.  */
        vectPtr = *iphcVectPtrPtr;
        if (vectPtr == TM_IPHC_VECT_NULL_PTR)
        {
            vectPtr = (ttIphcVectPtr) tm_get_raw_buffer( sizeof(ttIphcVect) );
        }
        if (vectPtr != TM_IPHC_VECT_NULL_PTR)
        {
/* 3.  Clear the new state vector. */
            tm_zero_struct(*vectPtr);

/* 4.  Use the default values for IPHC parameters, as given by RFC-2507. */
            vectPtr->iphcOpts.iphcoMaxTime     = TM_IPHC_MAX_TIME_DEF;
            vectPtr->iphcOpts.iphcoMaxPeriod   = TM_IPHC_MAX_PERIOD_DEF;
            vectPtr->iphcOpts.iphcoOutUdpSpace = TM_IPHC_CONTEXT_SPACE_DEF;
            vectPtr->iphcOpts.iphcoInUdpSpace  = TM_IPHC_CONTEXT_SPACE_DEF;
            vectPtr->iphcOpts.iphcoOutTcpSpace = TM_IPHC_CONTEXT_SPACE_DEF;
            vectPtr->iphcOpts.iphcoInTcpSpace  = TM_IPHC_CONTEXT_SPACE_DEF;
            vectPtr->iphcOpts.iphcoTcpErrRec   = TM_IPHC_TCP_ERR_REC_DEF;
            vectPtr->iphcOpts.iphcoMaxHeader   = TM_IPHC_MAX_HEADER_DEF;

            *iphcVectPtrPtr = vectPtr;
            errorCode       = TM_ENOERROR;
        }
        else
        {
            errorCode = TM_ENOBUFS;
        }        
    }

    return errorCode;
}


/*
 * tfIphcClose Function Description
 * This routine closes the IPHC module, deallocates the state vector and 
 * removes any associated timers.
 * 
 * 1.  ERROR CHECKING: all parameters are not null.
 * 2.  Free all UDP and TCP contexts, including IP header buffers.
 * 
 * Parameters
 * Parameter     Description
 * iphcVectPtr   Current IPHC state vector.
 * Return Value  Meaning
 * TM_ENOERROR   Successful
 * TM_EINVAL     Invalid state vector pointer.
 */
static int tfIphcClose(ttIphcVectPtr iphcVectPtr)
{
    int                 errorCode;
    ttIphcUdpContextPtr udpCtxPtr;
    ttIphcTcpContextPtr tcpCtxPtr;
    ttRawBufferPtr      origBufPtr;     

/* 1.  ERROR CHECKING: all parameters are not null. */
#ifdef TM_ERROR_CHECKING
    if (iphcVectPtr == TM_IPHC_VECT_NULL_PTR)
    {
        errorCode = TM_EINVAL;
    }
    else
#endif /* TM_ERROR_CHECKING */
    {

/* 2.  Free all UDP and TCP contexts. */
        if (iphcVectPtr->iphcUdpOutCtxPtr != TM_UDP_CTX_NULL_PTR)
        {
/*            
 * The list of outgoing contexts may have been rearranged, so find the
 * address of context zero, which is the original pointer that was allocated
 * and must now be freed
 * Also free IP header buffers from each context as we search the list.
 */ 
            udpCtxPtr = iphcVectPtr->iphcUdpOutCtxPtr;
            origBufPtr = TM_8BIT_NULL_PTR;
            while (udpCtxPtr != TM_UDP_CTX_NULL_PTR)
            {
                if (udpCtxPtr->uctxCid == 0)
                {
                    origBufPtr = (ttRawBufferPtr) udpCtxPtr;
                }

/* Free IP header buffer. */
                if (udpCtxPtr->uctxIpHeaderPtr != TM_8BIT_NULL_PTR)
                {
                    tm_free_raw_buffer(udpCtxPtr->uctxIpHeaderPtr);
                }
                
                udpCtxPtr = udpCtxPtr->uctxNextPtr;
            }
/* Free context list. */
            if (origBufPtr != TM_8BIT_NULL_PTR)
            {
                tm_free_raw_buffer(origBufPtr);
            }
        }

/* Free outgoing contexts and IP header buffers. */
        if (iphcVectPtr->iphcUdpInCtxPtr != TM_UDP_CTX_NULL_PTR)
        {
            udpCtxPtr = iphcVectPtr->iphcUdpInCtxPtr;
            while (udpCtxPtr != TM_UDP_CTX_NULL_PTR)
            {

/* Free IP header buffer. */
                if (udpCtxPtr->uctxIpHeaderPtr != TM_8BIT_NULL_PTR)
                {
                    tm_free_raw_buffer(udpCtxPtr->uctxIpHeaderPtr);
                }
                
                udpCtxPtr = udpCtxPtr->uctxNextPtr;
            }
/* Free context list. */
            tm_free_raw_buffer((ttRawBufferPtr) iphcVectPtr->iphcUdpInCtxPtr);
        }

        if (iphcVectPtr->iphcTcpOutCtxPtr != TM_TCP_CTX_NULL_PTR)
        {
/*            
 * The list of outgoing contexts may have been rearranged, so find the
 * address of context zero, which is the original pointer that was allocated
 * and must now be freed.
 * Also free IP header buffers from each context as we search the list.
 */
            tcpCtxPtr = iphcVectPtr->iphcTcpOutCtxPtr;
            origBufPtr = TM_8BIT_NULL_PTR;
            while (tcpCtxPtr != TM_TCP_CTX_NULL_PTR)
            {
                if (tcpCtxPtr->tctxCid == 0)
                {
                    origBufPtr = (ttRawBufferPtr) tcpCtxPtr;
                }
/* Free IP header buffer. */
                if (tcpCtxPtr->tctxIpHeaderPtr != TM_8BIT_NULL_PTR)
                {
                    tm_free_raw_buffer(tcpCtxPtr->tctxIpHeaderPtr);
                }
                tcpCtxPtr = tcpCtxPtr->tctxNextPtr;
            }

/* Free context list. */
            if (origBufPtr != TM_8BIT_NULL_PTR)
            {
                tm_free_raw_buffer(origBufPtr);
            }
        }

        if (iphcVectPtr->iphcTcpInCtxPtr != TM_TCP_CTX_NULL_PTR)
        {
            tcpCtxPtr = iphcVectPtr->iphcTcpInCtxPtr;
            while (tcpCtxPtr != TM_TCP_CTX_NULL_PTR)
            {

/* Free IP header buffer. */
                if (tcpCtxPtr->tctxIpHeaderPtr != TM_8BIT_NULL_PTR)
                {
                    tm_free_raw_buffer(tcpCtxPtr->tctxIpHeaderPtr);
                }
                
                tcpCtxPtr = tcpCtxPtr->tctxNextPtr;
            }
/* Free context list. */
            tm_free_raw_buffer((ttRawBufferPtr) iphcVectPtr->iphcTcpInCtxPtr);
        }

        errorCode = TM_ENOERROR;
    }

    return errorCode;
}


/*
 * tfIphcAllocCtx Function Description
 * Allocates a series of outgoing contexts for the specified protocol 
 * 
 * 1.  Determine the buffer size needed to maintain all necessary contexts.
 * 2.  Allocate memory for contexts.  Return TM_ENOBUFS if this allocation 
 *     fails.  
 * 3.  Setup context ID's and linked list of contexts.  Also allocate
 *     IP header buffers for each context.
 * 
 * Parameters
 * Parameter     Description
 * vectPtr       Pointer to the current IPHC state vector
 * proto         Either TM_IP_TCP or TM_IP_UDP
 * Return Value       Meaning
 * TM_ENOERROR        Successful
 * TM_EPROTONOSUPPORT Invalid protocol specified
 * TM_ENOBUFS         Not enough memory to allocate contexts
 */
static int tfIphcAllocCtx(ttIphcVectPtr          vectPtr,
                          tt8Bit                 proto,
                          tt16Bit                ctxSpace,
                          ttIphcUdpContextPtrPtr vectUdpCtxPtrPtr,
                          ttIphcTcpContextPtrPtr vectTcpCtxPtrPtr)
{
    ttIphcUdpContextPtr udpCtxPtr;
    ttIphcTcpContextPtr tcpCtxPtr;            
    ttIphcUdpContextPtr vectUdpCtxPtr;
    ttIphcTcpContextPtr vectTcpCtxPtr;            
    ttVoidPtr           voidCtxPtr;
    ttPktLen            allocSize;
    tt16Bit             ctxId;
    tt16Bit             maxHdrSize;
    int                 errorCode;

/* 1.  Determine the buffer size needed to maintain all necessary contexts. */
    if (proto == TM_IP_UDP)
    {
        allocSize = sizeof(ttIphcUdpContext) * ctxSpace;
    }
    else
    {
        if (proto == TM_IP_TCP)
        {
            allocSize = sizeof(ttIphcTcpContext) * ctxSpace;
        }
        else
        {
            errorCode = TM_EPROTONOSUPPORT;
            goto iphcAllocCtxExit;
        }
    }

/*
 * 2.  Allocate memory for contexts.  Return TM_ENOBUFS if this allocation 
 *     fails.  
 */
    voidCtxPtr = (ttVoidPtr) tm_get_raw_buffer(allocSize);
    if (voidCtxPtr == (ttVoidPtr) 0)
    {
        errorCode = TM_ENOBUFS;
        goto iphcAllocCtxExit;
    }


    tm_bzero(voidCtxPtr, allocSize);

    errorCode = TM_ENOERROR;

    maxHdrSize = vectPtr->iphcOpts.iphcoMaxHeader;
    
    if (proto == TM_IP_UDP)
    {
/* UDP */
        *vectUdpCtxPtrPtr = (ttIphcUdpContextPtr) voidCtxPtr;
        vectUdpCtxPtr     = (ttIphcUdpContextPtr) voidCtxPtr;

/*        
 * 3.  Setup context ID's and linked list of contexts.  Also allocate
 *     IP header buffers for each context.
 */     
        for (ctxId = 0; ctxId < ctxSpace; ctxId++)
        {
            udpCtxPtr = &vectUdpCtxPtr[ctxId];
            udpCtxPtr->uctxCid = ctxId;
            if (ctxId != (ctxSpace-1))
            {
                udpCtxPtr->uctxNextPtr = &(vectUdpCtxPtr[ctxId+1]);
            }
            else
            {
/* Last entry in list */
                udpCtxPtr->uctxNextPtr = (ttIphcUdpContextPtr) 0;
            }

            udpCtxPtr->uctxIpHeaderPtr = tm_get_raw_buffer(maxHdrSize);
            if (udpCtxPtr->uctxIpHeaderPtr == TM_8BIT_NULL_PTR)
            {
/* Allocation of header buffer failed - cleanup and return error. */
                tfIphcClose(vectPtr);
                errorCode = TM_ENOBUFS;
                goto iphcAllocCtxExit;
            }
        }

    }
    else        
    {
/* TCP */
        *vectTcpCtxPtrPtr = (ttIphcTcpContextPtr) voidCtxPtr;
        vectTcpCtxPtr = (ttIphcTcpContextPtr) voidCtxPtr;

/*
 * 3.  Setup context ID's and linked list of contexts.  Also allocate
 *     IP header buffers for each context.
 */     
        for (ctxId = 0; ctxId < ctxSpace; ctxId++)
        {
            tcpCtxPtr = &vectTcpCtxPtr[ctxId];
            tcpCtxPtr->tctxCid = ctxId;
            if (ctxId != (ctxSpace-1))
            {
                tcpCtxPtr->tctxNextPtr = &(vectTcpCtxPtr[ctxId+1]);
            }
            else
            {
/* Last entry in list */
                tcpCtxPtr->tctxNextPtr = (ttIphcTcpContextPtr) 0;                
            }

            tcpCtxPtr->tctxIpHeaderPtr = tm_get_raw_buffer(maxHdrSize);
            if (tcpCtxPtr->tctxIpHeaderPtr == TM_8BIT_NULL_PTR)
            {
/* Allocation of header buffer failed - cleanup and return error. */
                tfIphcClose(vectPtr);
                errorCode = TM_ENOBUFS;
                goto iphcAllocCtxExit;
            }
            
        }
    }
 
iphcAllocCtxExit:
    return errorCode;
}

static void tfCopyPacketToBuffer(ttPacketPtr packetPtr,
                                 tt8BitPtr   bufPtr,
                                 ttPktLen    dataLength)
{
    int byteIndex;
    int copyLength;

    tm_assert(tfCopyPacketToBuffer,
              dataLength <= packetPtr->pktChainDataLength);
    
    byteIndex = 0;
    while ( dataLength > 0 )
    {
        if (packetPtr->pktLinkDataLength >= dataLength)
        {
            copyLength = dataLength;
        }
        else
        {
            copyLength = packetPtr->pktLinkDataLength;
        }

        tm_bcopy(packetPtr->pktLinkDataPtr,
                 bufPtr+byteIndex,
                 copyLength);
        
        byteIndex  += copyLength;
        dataLength -= copyLength;
        
        if (dataLength > 0)
        {
            packetPtr = packetPtr->pktLinkNextPtr;
            tm_assert(tfCopyPacketToBuffer, packetPtr != TM_PACKET_NULL_PTR);
        }
    }
}

#else /* !TM_USE_IPHC && TM_USE_PPP*/
LINT_UNUSED_HEADER

/* To allow link for builds when TM_USE_IPHC is not defined */
int tvIphcDummy = 0;
#endif /* ! TM_USE_IPHC */

/***************** End Of File *****************/
