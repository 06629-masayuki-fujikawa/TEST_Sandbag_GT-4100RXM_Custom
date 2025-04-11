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
 * Description: Type Definitions for IPv6
 *
 * Filename: t6type.h
 * Author: Ed Remmell
 * Date Created:
 * $Source: include/t6type.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:08JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _T6TYPE_H_
#define _T6TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TM_USE_IPV6

/* tt6IpFHEntry is a fragment head. */
typedef struct ts6IpFHEntry
{
/* Pointer to the next fragment head in the list. */
    struct ts6IpFHEntry TM_FAR      * ip6fHeadNextPtr;
/* Pointer to next packet in this fragment list */
    ttPacketPtr                       ip6fPacketNextPtr;
/* Pointer to the IP header in the first fragment. */
    tt8BitPtr                         ip6fIpHdrPtr;
/* Pointer to fragment header in the first fragment. */
    struct ts6FragmentExtHdr TM_FAR * ip6fOrigFragHdrPtr;
/* Source IP address */
    tt6IpAddress                      ip6fSrcAddr;
/* Destination IP address */
    tt6IpAddress                      ip6fDestAddr;
/* Fragment identification field */
    tt32Bit                           ip6fIdent;
/* Total size of received fragments */
    tt32Bit                           ip6fSize;
/* Fragment time to live. */
    tt8Bit                            ip6fTtl;
/* Incoming packet is too large to reassemble */
    tt8Bit                            ip6fTooBig;
/* Align to 32-bit boundary */    
    tt8Bit                            ip6fPadding[2];
} tt6IpFHEntry;
typedef tt6IpFHEntry TM_FAR * tt6IpFHEntryPtr;

/* tt6MldEntry is used to represent a multicast group that the user has
   joined on an interface. */
typedef struct ts6MldEntry
{
/* Pointer to the next multicast group entry in the list. */
    struct ts6MldEntry TM_FAR * mld6NextPtr;
/* Pointer to the prev multicast group entry in the list. */
    struct ts6MldEntry TM_FAR * mld6PrevPtr;
/* The IPv6 address of the multicast group. */
    tt6IpAddress                mld6McastGroup;
/* Pointer to a delay timer used to implement a random delay before sending a
   MLD Report message. 
   ([RFC2710].R4:100, [RFC2710].R4:110) */
    ttTimerPtr                  mld6DelayTimerPtr;
    tt16Bit                     mld6OwnerCount;
    tt8Bit                      mld6State;
#ifdef TM_6_USE_RFC3590
    tt16Bit                     mld6UseUnspecAddr;
#endif /* TM_6_USE_RFC3590 */
#ifdef TM_6_USE_MLDV2
/* Pointer to the MultiCast Address timer */
    ttTimerPtr                  mld6MCAddrTimerPtr;
/* Filter Mode either INCLUDE or EXCLUDE */
    tt8Bit                      filterMode;
/* Identifies the number of sources */
    tt16Bit                     mldNumberOfSources;
/* Pointer to the Source List */
    tt6IpAddressPtr             mld6sourceListPtr;
/* Variable for storing source list when we receive a 
 * Multicast Address and source specific query. 
 */
    tt6ListOfSourceAddrPtr      querySourceListPtr;
#endif /* TM_6_USE_MLDV2 */
} tt6MldEntry;
typedef tt6MldEntry    TM_FAR * tt6MldEntryPtr;
typedef tt6MldEntryPtr TM_FAR * tt6MldEntryPtrPtr;

#ifdef TM_6_USE_MLDV2
/* ts6MldMcAddrRec is used to represent a multicast address record in the 
   MLD report message */
typedef struct ts6MldMcAddrRec
{
/* Pointer to the next multicast group entry in the list. */
    struct ts6MldMcAddrRec TM_FAR * mld6McRecNextPtr;
/* Identifies the Record type of MLDv2 report message. */
    tt8Bit                      mldRecType;
/* Identifies the Auxilary Data length of MLDv2 report message. */
    tt8Bit                      mldAuxDataType;
/* Identifies the number of sources of MLDv2 report message. */
    tt16Bit                     mldNumberOfSources;
/* Identifies the Multicast Address of MLDv2 report message. */
    tt6IpAddress                mld6mcastAddr;
/* Identifies the Array of source addresses of MLDv2 report message. */
    tt6IpAddressPtr             mld6SourceAddrPtr;
/* Retransmission count for retransmitting state change reports. */
    tt8Bit                      mld6RetrCount;
} tt6MldMcAddrRec;
typedef tt6MldMcAddrRec TM_FAR * tt6MldMcAddrRecPtr;
#endif /* TM_6_USE_MLDV2 */

/* tt6DadEntry is used to represent a tentative address on which we are
   performing Duplicate Address Detection. */
typedef struct ts6DadEntry
{
/* Pointer to the next Duplicate Address Detection entry in the list. */
    struct ts6DadEntry TM_FAR * dad6NextPtr;
/* Pointer to the timer used to implement retransmission of the Neighbor
   Solicitation message for Duplicate Address Detection.
   ([RFC2462].R5.4.2:20) */
    ttTimerPtr                  dad6TimerPtr;
/* The interface of the tentative address on which we are performing
   Duplicate Address Detection. */
    ttDeviceEntryPtr            dad6DevPtr;
/* The multi-home index of the tentative address on which we are performing
   Duplicate Address Detection. */
    tt16Bit                     dad6MultiHomeIndex;
    tt8Bit                      dad6ReTxCount;
    tt8Bit                      dad6Filler;
} tt6DadEntry;
typedef tt6DadEntry TM_FAR * tt6DadEntryPtr;

/* This structure is used internally by tf6GetLocalIpAddress to keep track of
   the iteration through the list of multi-homed IPv6 addresses on the
   interface. This structure overlays the public API structure
   tt6LocalIpAddressCursor so that tf6GetLocalIpAddress can access internal
   state information. */
typedef struct ts6ImplLocalIpAddrCursor
{
/* The interface that the IPv6 addresses in resultsSet are homed on. */
    ttDeviceEntryPtr devPtr;
/* Points to where in resultsSet to start the next iteration. */
    tt6IpAddressPtr  addrListPtr;
/* On cursor initialization, caches the first TM_CURSOR_MAX_ROWS IPv6 addresses
   matching the search criteria. */
    tt6IpAddress     resultsSet[TM_CURSOR_MAX_ROWS];
/* Count of IPv6 addresses in resultsSet. */
    tt16Bit          rowCount;
    tt16Bit          pad;
} tt6ImplLocalIpAddrCursor;
typedef tt6ImplLocalIpAddrCursor TM_FAR * tt6ImplLocalIpAddrCursorPtr;


/* Network header: IPv6 Extension Header */
typedef struct ts6GenExtHdr
{
    tt8Bit        geNextHdr;
    tt8Bit        geExtLen;
    tt8Bit        geExtPad[2];
}tt6GenExtHdr;
typedef tt6GenExtHdr TM_FAR *  tt6GenExtHdrPtr;

/* Network header: IPv6 Hop-by-Hop Options Extension Header */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#ifndef TM_32BIT_DSP_BIG_ENDIAN
typedef struct ts6HopByHopExtHdr
{
/* The first 6 bytes of options (i.e. there may be more). This extension header
   is padded to a 64-bit boundary using the Pad1 and PadN options.
   ([RFC2460].R4:70, [RFC2460].R4.2:70) */
    unsigned int hop6Options : 16; /* 1st 16-bits of hop-by-hop options */
/* Length of this extension header in 8-byte units, not counting the first 8
   bytes. */
    unsigned int hop6HdrLen  : 8;
/* The protocol ID of the next IPv6 extension header following this one, or
   IPPROTO_NONE if the ULP payload is immediately following.
   ([RFC2460].R4:10) */
    unsigned int hop6NextHdr : 8;
    unsigned int hop6Options2; /* Next 32-bits of hop-by-hop options */
} tt6HopByHopExtHdr;
#else /* TM_32BIT_DSP_BIG_ENDIAN */
typedef struct ts6HopByHopExtHdr
{
/* The protocol ID of the next IPv6 extension header following this one, or
   IPPROTO_NONE if the ULP payload is immediately following.
   ([RFC2460].R4:10) */
    unsigned int hop6NextHdr : 8;
/* Length of this extension header in 8-byte units, not counting the first 8
   bytes. */
    unsigned int hop6HdrLen  : 8;
/* The first 6 bytes of options (i.e. there may be more). This extension header
   is padded to a 64-bit boundary using the Pad1 and PadN options.
   ([RFC2460].R4:70, [RFC2460].R4.2:70) */
    unsigned int hop6Options : 16; /* not used directly on 32-bit DSP */
} tt6HopByHopExtHdr;
#endif /* TM_32BIT_DSP_BIG_ENDIAN */
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
typedef struct ts6HopByHopExtHdr
{
/* The protocol ID of the next IPv6 extension header following this one, or
   IPPROTO_NONE if the ULP payload is immediately following.
   ([RFC2460].R4:10) */
    unsigned int hop6NextHdr : 8;
/* Length of this extension header in 8-byte units, not counting the first 8
   bytes. */
    unsigned int hop6HdrLen  : 8;
/* The first 6 bytes of options (i.e. there may be more). This extension header
   is padded to a 64-bit boundary using the Pad1 and PadN options.
   ([RFC2460].R4:70, [RFC2460].R4.2:70) */
    tt8Bit       hop6Options[6]; /* pad to 64-bit boundary */
} tt6HopByHopExtHdr;
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
typedef struct ts6HopByHopExtHdr
{
/* The protocol ID of the next IPv6 extension header following this one, or
   IPPROTO_NONE if the ULP payload is immediately following.
   ([RFC2460].R4:10) */
    tt8Bit hop6NextHdr;
/* Length of this extension header in 8-byte units, not counting the first 8
   bytes. */
    tt8Bit hop6HdrLen;
/* The first 6 bytes of options (i.e. there may be more). This extension header
   is padded to a 64-bit boundary using the Pad1 and PadN options.
   ([RFC2460].R4:70, [RFC2460].R4.2:70) */
    tt8Bit hop6Options[6]; /* pad to 64-bit boundary */
} tt6HopByHopExtHdr;
#endif /* TM_DSP */
typedef tt6HopByHopExtHdr TM_FAR * tt6HopByHopExtHdrPtr;

/* Network header: IPv6 Extension Header TLV Option */
/* Two of the extension headers, the Hop-by-Hop Options header and the
   Destination Options header, may carry one or more options in
   Type-Length-Value (TLV) format. */
typedef struct ts6ExtHdrOption
{
/* Identifies the type of the extension header option. The two highest-order
   bits of this field specify how to handle options that are not recognized,
   refer to TM_6_OPTION_ACTION_MASK. */
    tt8Bit eho6OptionType;
/* Length of the tlv6OptionData field in bytes. */
    tt8Bit eho6OptionLen;
/* The option data, if there is any. */
    tt8Bit eho6OptionData[256];
} tt6ExtHdrOption;
typedef tt6ExtHdrOption TM_FAR * tt6ExtHdrOptionPtr;

/* Network header: IPv6 Destination Options Extension Header */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#ifndef TM_32BIT_DSP_BIG_ENDIAN
typedef struct ts6DestExtHdr
{
/* The first 6 bytes of options (i.e. there may be more). This extension header
   is padded to a 64-bit boundary using the Pad1 and PadN options.
   ([RFC2460].R4:70, [RFC2460].R4.2:70) */
    unsigned int dst6Options : 16; /* not used directly on 32-bit DSP */
/* Length of this extension header in 8-byte units, not counting the first 8
   bytes. */
    unsigned int dst6HdrLen  : 8;
/* The protocol ID of the next IPv6 extension header following this one, or
   IPPROTO_NONE if the ULP payload is immediately following.
   ([RFC2460].R4:10) */
    unsigned int dst6NextHdr : 8;
} tt6DestExtHdr;
#else /* TM_32BIT_DSP_BIG_ENDIAN */
typedef struct ts6DestExtHdr
{
/* The protocol ID of the next IPv6 extension header following this one, or
   IPPROTO_NONE if the ULP payload is immediately following.
   ([RFC2460].R4:10) */
    unsigned int dst6NextHdr : 8;
/* Length of this extension header in 8-byte units, not counting the first 8
   bytes. */
    unsigned int dst6HdrLen  : 8;
/* The first 6 bytes of options (i.e. there may be more). This extension header
   is padded to a 64-bit boundary using the Pad1 and PadN options.
   ([RFC2460].R4:70, [RFC2460].R4.2:70) */
    unsigned int dst6Options : 16; /* not used directly on 32-bit DSP */
} tt6DestExtHdr;
#endif /* TM_32BIT_DSP_BIG_ENDIAN */
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
typedef struct ts6DestExtHdr
{
/* The protocol ID of the next IPv6 extension header following this one, or
   IPPROTO_NONE if the ULP payload is immediately following.
   ([RFC2460].R4:10) */
    unsigned int dst6NextHdr : 8;
/* Length of this extension header in 8-byte units, not counting the first 8
   bytes. */
    unsigned int dst6HdrLen  : 8;
/* The first 6 bytes of options (i.e. there may be more). This extension header
   is padded to a 64-bit boundary using the Pad1 and PadN options.
   ([RFC2460].R4:70, [RFC2460].R4.2:70) */
    tt8Bit dst6Options[6]; /* pad to 64-bit boundary */
} tt6DestExtHdr;
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
typedef struct ts6DestExtHdr
{
/* The protocol ID of the next IPv6 extension header following this one, or
   IPPROTO_NONE if the ULP payload is immediately following.
   ([RFC2460].R4:10) */
    tt8Bit dst6NextHdr;
/* Length of this extension header in 8-byte units, not counting the first 8
   bytes. */
    tt8Bit dst6HdrLen;
/* The first 6 bytes of options (i.e. there may be more). This extension header
   is padded to a 64-bit boundary using the Pad1 and PadN options.
   ([RFC2460].R4:70, [RFC2460].R4.2:70) */
    tt8Bit dst6Options[6]; /* pad to 64-bit boundary */
} tt6DestExtHdr;
#endif /* TM_DSP */
typedef tt6DestExtHdr TM_FAR * tt6DestExtHdrPtr;

/* Network header: IPv6 Routing Extension Header */
typedef struct ts6RoutingExtHdr
{
/* The protocol ID of the next IPv6 extension header following this one, or
   IPPROTO_NONE if the ULP payload is immediately following.
   ([RFC2460].R4:10) */
    tt8Bit rth6NextHdr;
/* Length of this extension header in 8-byte units, not counting the first 8
   bytes. */
    tt8Bit rth6HdrLen;
/* Specifies the specific type of routing extension header. Currently, only
   Type 0 is defined (i.e. tt6RoutingType0ExtHdr). */
    tt8Bit rth6Type;
/* Number of route segments remaining, i.e., number of explicitly listed
   intermediate nodes still to be visited before reaching the final
   destination. */
    tt8Bit rth6SegsLeft;
/* The routing extension header is a multiple of 8 bytes in length.
   ([RFC2460].R4:70) */
    tt8Bit rth6Pad[4]; /* pad to 64-bit boundary */
} tt6RoutingExtHdr;
typedef tt6RoutingExtHdr TM_FAR * tt6RoutingExtHdrPtr;

/* Network header: IPv6 Type 0 Routing Extension Header */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#ifndef TM_32BIT_DSP_BIG_ENDIAN
typedef struct ts6RoutingType0ExtHdr
{
/* Number of route segments remaining, i.e., number of explicitly listed
   intermediate nodes still to be visited before reaching the final
   destination. */
    unsigned int rth6SegsLeft : 8;
/* Specifies the specific type of routing extension header. Currently, only
   Type 0 is defined (i.e. tt6RoutingType0ExtHdr). */
    unsigned int rth6Type     : 8;
/* Length of this extension header in 8-byte units, not counting the first 8
   bytes. */
    unsigned int rth6HdrLen   : 8;
/* The protocol ID of the next IPv6 extension header following this one, or
   IPPROTO_NONE if the ULP payload is immediately following.
   ([RFC2460].R4:10) */
    unsigned int rth6NextHdr  : 8;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt32Bit rth6Reserved;
/* Array of addresses to visit, rth6SegsLeft indicates which is the next to be
   visited. */
    tt6IpAddrArr rth6AddressesToVisit;
} tt6RoutingType0ExtHdr;
#else /* TM_32BIT_DSP_BIG_ENDIAN */
typedef struct ts6RoutingType0ExtHdr
{
/* The protocol ID of the next IPv6 extension header following this one, or
   IPPROTO_NONE if the ULP payload is immediately following.
   ([RFC2460].R4:10) */
    unsigned int rth6NextHdr  : 8;
/* Length of this extension header in 8-byte units, not counting the first 8
   bytes. */
    unsigned int rth6HdrLen   : 8;
/* Specifies the specific type of routing extension header. Currently, only
   Type 0 is defined (i.e. tt6RoutingType0ExtHdr). */
    unsigned int rth6Type     : 8;
/* Number of route segments remaining, i.e., number of explicitly listed
   intermediate nodes still to be visited before reaching the final
   destination. */
    unsigned int rth6SegsLeft : 8;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt32Bit rth6Reserved;
/* Array of addresses to visit, rth6SegsLeft indicates which is the next to be
   visited. */
    tt6IpAddrArr rth6AddressesToVisit;
} tt6RoutingType0ExtHdr;
#endif /* TM_32BIT_DSP_BIG_ENDIAN */
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
typedef struct ts6RoutingType0ExtHdr
{
/* The protocol ID of the next IPv6 extension header following this one, or
   IPPROTO_NONE if the ULP payload is immediately following.
   ([RFC2460].R4:10) */
    unsigned int rth6NextHdr  : 8;
/* Length of this extension header in 8-byte units, not counting the first 8
   bytes. */
    unsigned int rth6HdrLen   : 8;
/* Specifies the specific type of routing extension header. Currently, only
   Type 0 is defined (i.e. tt6RoutingType0ExtHdr). */
    unsigned int rth6Type     : 8;
/* Number of route segments remaining, i.e., number of explicitly listed
   intermediate nodes still to be visited before reaching the final
   destination. */
    unsigned int rth6SegsLeft : 8;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt32Bit rth6Reserved;
/* Array of addresses to visit, rth6SegsLeft indicates which is the next to be
   visited. */
    tt6IpAddrArr rth6AddressesToVisit;
} tt6RoutingType0ExtHdr;
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
typedef struct ts6RoutingType0ExtHdr
{
/* The protocol ID of the next IPv6 extension header following this one, or
   IPPROTO_NONE if the ULP payload is immediately following.
   ([RFC2460].R4:10) */
    tt8Bit rth6NextHdr;
/* Length of this extension header in 8-byte units, not counting the first 8
   bytes. */
    tt8Bit rth6HdrLen;
/* Specifies the specific type of routing extension header. Currently, only
   Type 0 is defined (i.e. tt6RoutingType0ExtHdr). */
    tt8Bit rth6Type;
/* Number of route segments remaining, i.e., number of explicitly listed
   intermediate nodes still to be visited before reaching the final
   destination. */
    tt8Bit rth6SegsLeft;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt32Bit rth6Reserved;
/* Array of addresses to visit, rth6SegsLeft indicates which is the next to be
   visited. */
    tt6IpAddrArr rth6AddressesToVisit;
} tt6RoutingType0ExtHdr;
#endif /* !TM_DSP */
typedef tt6RoutingType0ExtHdr TM_FAR * tt6RoutingType0ExtHdrPtr;

/* Network header: IPv6 Fragment Extension Header */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#ifndef TM_32BIT_DSP_BIG_ENDIAN
typedef struct ts6FragmentExtHdr
{
/* Fragment offset, reserved bits and more-fragments flag bit:
   TM_6_EXTH_MORE_FRAGS_FLAG */
    unsigned int fra6OffsetFlags : 16;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    unsigned int fra6Reserved    : 8;
/* The protocol ID of the first IPv6 extension header of the Fragmentable Part
   of the original packet, or IPPROTO_NONE if the Fragmentable Part only
   consists of the ULP payload. ([RFC2460].R4.5:20) */
    unsigned int fra6NextHdr     : 8;
/* Identifies the packet that this fragment belongs to. ([RFC2460].R4.5:30) */
    tt32Bit fra6Ident;
} tt6FragmentExtHdr;
#else /* TM_32BIT_DSP_BIG_ENDIAN */
typedef struct ts6FragmentExtHdr
{
/* The protocol ID of the first IPv6 extension header of the Fragmentable Part
   of the original packet, or IPPROTO_NONE if the Fragmentable Part only
   consists of the ULP payload. ([RFC2460].R4.5:20) */
    unsigned int fra6NextHdr     : 8;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    unsigned int fra6Reserved    : 8;
/* Fragment offset, reserved bits and more-fragments flag bit:
   TM_6_EXTH_MORE_FRAGS_FLAG */
    unsigned int fra6OffsetFlags : 16;
/* Identifies the packet that this fragment belongs to. ([RFC2460].R4.5:30) */
    tt32Bit fra6Ident;
} tt6FragmentExtHdr;
#endif /* TM_32BIT_DSP_BIG_ENDIAN */
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
typedef struct ts6FragmentExtHdr
{
/* The protocol ID of the first IPv6 extension header of the Fragmentable Part
   of the original packet, or IPPROTO_NONE if the Fragmentable Part only
   consists of the ULP payload. ([RFC2460].R4.5:20) */
    unsigned int fra6NextHdr  : 8;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    unsigned int fra6Reserved : 8;
/* Fragment offset, reserved bits and more-fragments flag bit:
   TM_6_EXTH_MORE_FRAGS_FLAG */
    tt16Bit fra6OffsetFlags;
/* Identifies the packet that this fragment belongs to. ([RFC2460].R4.5:30) */
    tt32Bit fra6Ident;
} tt6FragmentExtHdr;

#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
typedef struct ts6FragmentExtHdr
{
/* The protocol ID of the first IPv6 extension header of the Fragmentable Part
   of the original packet, or IPPROTO_NONE if the Fragmentable Part only
   consists of the ULP payload. ([RFC2460].R4.5:20) */
    tt8Bit fra6NextHdr;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt8Bit fra6Reserved;
/* Fragment offset, reserved bits and more-fragments flag bit:
   TM_6_EXTH_MORE_FRAGS_FLAG */
    tt16Bit fra6OffsetFlags;
/* Identifies the packet that this fragment belongs to. ([RFC2460].R4.5:30) */
    tt32Bit fra6Ident;
} tt6FragmentExtHdr;
#endif /* !TM_DSP */
typedef tt6FragmentExtHdr TM_FAR * tt6FragmentExtHdrPtr;

/* Network header: ICMPv6 Header */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#ifndef TM_32BIT_DSP_BIG_ENDIAN
typedef struct ts6IcmpHeader
{
/* Checksum used to detect data corruption in the ICMPv6 message and parts of
   the IPv6 packet header. ([RFC2460].R8.1:50) */
    unsigned int icm6Checksum : 16;
/* Identifies the subtype of ICMPv6 message. */
    unsigned int icm6Code     : 8 ;
/* Identifies the type of ICMPv6 message. */
    unsigned int icm6Type     : 8 ;
    union {
/* Used when sending an ICMPv6 error message to set either icm6PktTooBigMtu or
   icm6ParmProbOffset to the error parameter value passed in by the caller. */
#define icm6ErrorParm icm6DataUnion.icm6Parm32
        tt32Bit icm6Parm32;
        struct {
            tt32Bit icm6Unused32;
        } icm6DestUnreachStruct;
        struct {
/* MTU of the next-hop link, as reported by the ICMPv6 Packet Too Big error
   message. */
#define icm6PktTooBigMtu icm6DataUnion.icm6PktTooBigStruct.icm6Mtu
            tt32Bit icm6Mtu;
        } icm6PktTooBigStruct;
        struct {
            tt32Bit icm6Unused32;
        } icm6TimeExceededStruct;
        struct {
/* Byte offset within the original/invoking packet where the error was
   detected, as reported by the ICMPv6 Parameter Problem error message. The
   Parameter Problem message is used when an IPv6 node finds a field in the
   IPv6 header or extension headers that prevents the processing of the
   packet. */
#define icm6ParmProbOffset icm6DataUnion.icm6ParmProbStruct.icm6Offset
            tt32Bit icm6Offset;
        } icm6ParmProbStruct;
        struct {
/* Sequence number in ICMPv6 Echo Request and Echo Reply messages. */
#define icm6EchoSeqno icm6DataUnion.icm6EchoStruct.icm6Seqno
            unsigned int icm6Seqno : 16;
/* Identifier in ICMPv6 Echo Request and Echo Reply messages. */
#define icm6EchoIdent icm6DataUnion.icm6EchoStruct.icm6Ident
            unsigned int icm6Ident : 16;
        } icm6EchoStruct;
        struct {
#define icm6MldReserved icm6DataUnion.icm6MldStruct.icm6Reserved
            unsigned int icm6Reserved         : 16;
/* Maximum response delay in ICMPv6 Multicast Listener Discovery messages,
   units are milliseconds. */
#define icm6MldMaxRespDelay icm6DataUnion.icm6MldStruct.icm6MaxRespDelayMsec
            unsigned int icm6MaxRespDelayMsec : 16;
        } icm6MldStruct;
        struct {
            tt32Bit icm6Reserved;
        } icm6RouterSolicitStruct;
        struct {
/* Router lifetime in ICMPv6 Router Advertisement message. */
#define icm6RouterAdvertLifetime \
    icm6DataUnion.icm6RouterAdvertStruct.icm6Lifetime
            unsigned int icm6Lifetime : 16;
/* Managed address configuration and Other stateful configuration flags in
   ICMPv6 Router Advertisement message:
   TM_6_ND_RA_MANAGED_FLAG
   TM_6_ND_RA_OTHER_FLAG */
#define icm6RouterAdvertFlags icm6DataUnion.icm6RouterAdvertStruct.icm6Flags
            unsigned int icm6Flags    : 8;
/* Current hop limit in ICMPv6 Router Advertisement message. */
#define icm6RouterAdvertHopLimit \
    icm6DataUnion.icm6RouterAdvertStruct.icm6HopLimit
            unsigned int icm6HopLimit : 8;
        } icm6RouterAdvertStruct;
        struct {
            tt32Bit icm6Reserved;
        } icm6NeighborSolicitStruct;
        struct {
            unsigned int icm6Reserved2 : 16;
            unsigned int icm6Reserved1 : 8;            
/* Router, Solicited and Override flags in ICMPv6 Neighbor Advertisement
   message:
   TM_6_ND_NA_ROUTER_FLAG
   TM_6_ND_NA_SOLICITED_FLAG
   TM_6_ND_NA_OVERRIDE_FLAG */
#define icm6NeighborAdvertFlags \
    icm6DataUnion.icm6NeighborAdvertStruct.icm6Flags
            unsigned int icm6Flags     : 8;
        } icm6NeighborAdvertStruct;
        struct {
            tt32Bit icm6Reserved;
        } icm6RedirectStruct;
    } icm6DataUnion;
} tt6IcmpHeader;
#else /* TM_32BIT_DSP_BIG_ENDIAN */
typedef struct ts6IcmpHeader
{
/* Identifies the type of ICMPv6 message. */
    unsigned int icm6Type     : 8 ;
/* Identifies the subtype of ICMPv6 message. */
    unsigned int icm6Code     : 8 ;
/* Checksum used to detect data corruption in the ICMPv6 message and parts of
   the IPv6 packet header. ([RFC2460].R8.1:50) */
    unsigned int icm6Checksum : 16;
    
    union {
/* Used when sending an ICMPv6 error message to set either icm6PktTooBigMtu or
   icm6ParmProbOffset to the error parameter value passed in by the caller. */
#define icm6ErrorParm icm6DataUnion.icm6Parm32
        tt32Bit icm6Parm32;
        struct {
            tt32Bit icm6Unused32;
        } icm6DestUnreachStruct;
        struct {
/* MTU of the next-hop link, as reported by the ICMPv6 Packet Too Big error
   message. */
#define icm6PktTooBigMtu icm6DataUnion.icm6PktTooBigStruct.icm6Mtu
            tt32Bit icm6Mtu;
        } icm6PktTooBigStruct;
        struct {
            tt32Bit icm6Unused32;
        } icm6TimeExceededStruct;
        struct {
/* Byte offset within the original/invoking packet where the error was
   detected, as reported by the ICMPv6 Parameter Problem error message. The
   Parameter Problem message is used when an IPv6 node finds a field in the
   IPv6 header or extension headers that prevents the processing of the
   packet. */
#define icm6ParmProbOffset icm6DataUnion.icm6ParmProbStruct.icm6Offset
            tt32Bit icm6Offset;
        } icm6ParmProbStruct;
        struct {
/* Identifier in ICMPv6 Echo Request and Echo Reply messages. */
#define icm6EchoIdent icm6DataUnion.icm6EchoStruct.icm6Ident
            unsigned int icm6Ident : 16;
/* Sequence number in ICMPv6 Echo Request and Echo Reply messages. */
#define icm6EchoSeqno icm6DataUnion.icm6EchoStruct.icm6Seqno
            unsigned int icm6Seqno : 16;
        } icm6EchoStruct;
        struct {
/* Maximum response delay in ICMPv6 Multicast Listener Discovery messages,
   units are milliseconds. */
#define icm6MldMaxRespDelay icm6DataUnion.icm6MldStruct.icm6MaxRespDelayMsec
            unsigned int icm6MaxRespDelayMsec : 16;
#define icm6MldReserved icm6DataUnion.icm6MldStruct.icm6Reserved
            unsigned int icm6Reserved         : 16;
        } icm6MldStruct;
        struct {
            tt32Bit icm6Reserved;
        } icm6RouterSolicitStruct;
        struct {
/* Current hop limit in ICMPv6 Router Advertisement message. */
#define icm6RouterAdvertHopLimit \
    icm6DataUnion.icm6RouterAdvertStruct.icm6HopLimit
            unsigned int icm6HopLimit : 8;
/* Managed address configuration and Other stateful configuration flags in
   ICMPv6 Router Advertisement message:
   TM_6_ND_RA_MANAGED_FLAG
   TM_6_ND_RA_OTHER_FLAG */
#define icm6RouterAdvertFlags icm6DataUnion.icm6RouterAdvertStruct.icm6Flags
            unsigned int icm6Flags    : 8;
/* Router lifetime in ICMPv6 Router Advertisement message. */
#define icm6RouterAdvertLifetime \
    icm6DataUnion.icm6RouterAdvertStruct.icm6Lifetime
            unsigned int icm6Lifetime : 16;
        } icm6RouterAdvertStruct;
        struct {
            tt32Bit icm6Reserved;
        } icm6NeighborSolicitStruct;
        struct {
/* Router, Solicited and Override flags in ICMPv6 Neighbor Advertisement
   message:
   TM_6_ND_NA_ROUTER_FLAG
   TM_6_ND_NA_SOLICITED_FLAG
   TM_6_ND_NA_OVERRIDE_FLAG */
#define icm6NeighborAdvertFlags \
    icm6DataUnion.icm6NeighborAdvertStruct.icm6Flags
            unsigned int icm6Flags     : 8;
            unsigned int icm6Reserved1 : 8;
            unsigned int icm6Reserved2 : 16;            
        } icm6NeighborAdvertStruct;
        struct {
            tt32Bit icm6Reserved;
        } icm6RedirectStruct;
    } icm6DataUnion;
} tt6IcmpHeader;
#endif /* TM_32BIT_DSP_BIG_ENDIAN */
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
/* This structure maps the ICMPv6 message header in network byte order. */
typedef struct ts6IcmpHeader
{
/* Identifies the type of ICMPv6 message. */
    unsigned int icm6Type : 8;
/* Identifies the subtype of ICMPv6 message. */
    unsigned int icm6Code : 8;
/* Checksum used to detect data corruption in the ICMPv6 message and parts of
   the IPv6 packet header. ([RFC2460].R8.1:50) */
    tt16Bit icm6Checksum;
    union {
/* Used when sending an ICMPv6 error message to set either icm6PktTooBigMtu or
   icm6ParmProbOffset to the error parameter value passed in by the caller. */
#define icm6ErrorParm icm6DataUnion.icm6Parm32
        tt32Bit icm6Parm32;
        struct {
            tt32Bit icm6Unused32;
        } icm6DestUnreachStruct;
        struct {
/* MTU of the next-hop link, as reported by the ICMPv6 Packet Too Big error
   message. */
#define icm6PktTooBigMtu icm6DataUnion.icm6PktTooBigStruct.icm6Mtu
            tt32Bit icm6Mtu;
        } icm6PktTooBigStruct;
        struct {
            tt32Bit icm6Unused32;
        } icm6TimeExceededStruct;
        struct {
/* Byte offset within the original/invoking packet where the error was
   detected, as reported by the ICMPv6 Parameter Problem error message. The
   Parameter Problem message is used when an IPv6 node finds a field in the
   IPv6 header or extension headers that prevents the processing of the
   packet. */
#define icm6ParmProbOffset icm6DataUnion.icm6ParmProbStruct.icm6Offset
            tt32Bit icm6Offset;
        } icm6ParmProbStruct;
        struct { 
/* Identifier in ICMPv6 Echo Request and Echo Reply messages. */
#define icm6EchoIdent icm6DataUnion.icm6EchoStruct.icm6Ident
            tt16Bit icm6Ident;
/* Sequence number in ICMPv6 Echo Request and Echo Reply messages. */
#define icm6EchoSeqno icm6DataUnion.icm6EchoStruct.icm6Seqno
            tt16Bit icm6Seqno;
        } icm6EchoStruct;
        struct { 
/* Maximum response delay in ICMPv6 Multicast Listener Discovery messages,
   units are milliseconds. */
#define icm6MldMaxRespDelay icm6DataUnion.icm6MldStruct.icm6MaxRespDelayMsec
            tt16Bit icm6MaxRespDelayMsec;
#define icm6MldReserved icm6DataUnion.icm6MldStruct.icm6Reserved
            tt16Bit icm6Reserved;
        } icm6MldStruct;
        struct {
            tt32Bit icm6Reserved;
        } icm6RouterSolicitStruct;
        struct {
/* Current hop limit in ICMPv6 Router Advertisement message. */
#define icm6RouterAdvertHopLimit \
    icm6DataUnion.icm6RouterAdvertStruct.icm6HopLimit
            unsigned int icm6HopLimit : 8;
/* Managed address configuration and Other stateful configuration flags in
   ICMPv6 Router Advertisement message:
   TM_6_ND_RA_MANAGED_FLAG
   TM_6_ND_RA_OTHER_FLAG */
#define icm6RouterAdvertFlags icm6DataUnion.icm6RouterAdvertStruct.icm6Flags
            unsigned int icm6Flags    : 8;
/* Router lifetime in ICMPv6 Router Advertisement message. */
#define icm6RouterAdvertLifetime \
    icm6DataUnion.icm6RouterAdvertStruct.icm6Lifetime
            tt16Bit icm6Lifetime;
        } icm6RouterAdvertStruct;
        struct {
            tt32Bit icm6Reserved;
        } icm6NeighborSolicitStruct;
        struct {
/* Router, Solicited and Override flags in ICMPv6 Neighbor Advertisement
   message:
   TM_6_ND_NA_ROUTER_FLAG
   TM_6_ND_NA_SOLICITED_FLAG
   TM_6_ND_NA_OVERRIDE_FLAG */
#define icm6NeighborAdvertFlags \
    icm6DataUnion.icm6NeighborAdvertStruct.icm6Flags
            unsigned int icm6Flags     : 8;
            unsigned int icm6Reserved1 : 8;
            tt16Bit icm6Reserved2;
        } icm6NeighborAdvertStruct;
        struct {
            tt32Bit icm6Reserved;
        } icm6RedirectStruct;
    } icm6DataUnion;
} tt6IcmpHeader;
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
typedef struct ts6IcmpHeader
{
/* Identifies the type of ICMPv6 message. */
    tt8Bit icm6Type;
/* Identifies the subtype of ICMPv6 message. */
    tt8Bit icm6Code;
/* Checksum used to detect data corruption in the ICMPv6 message and parts of
   the IPv6 packet header. ([RFC2460].R8.1:50) */
    tt16Bit icm6Checksum;
    union {
/* Used when sending an ICMPv6 error message to set either icm6PktTooBigMtu or
   icm6ParmProbOffset to the error parameter value passed in by the caller. */
#define icm6ErrorParm icm6DataUnion.icm6Parm32
        tt32Bit icm6Parm32;
        struct {
            tt32Bit icm6Unused32;
        } icm6DestUnreachStruct;
        struct {
/* MTU of the next-hop link, as reported by the ICMPv6 Packet Too Big error
   message. */
#define icm6PktTooBigMtu icm6DataUnion.icm6PktTooBigStruct.icm6Mtu
            tt32Bit icm6Mtu;
        } icm6PktTooBigStruct;
        struct {
            tt32Bit icm6Unused32;
        } icm6TimeExceededStruct;
        struct {
/* Byte offset within the original/invoking packet where the error was
   detected, as reported by the ICMPv6 Parameter Problem error message. The
   Parameter Problem message is used when an IPv6 node finds a field in the
   IPv6 header or extension headers that prevents the processing of the
   packet. */
#define icm6ParmProbOffset icm6DataUnion.icm6ParmProbStruct.icm6Offset
            tt32Bit icm6Offset;
        } icm6ParmProbStruct;
        struct { 
/* Identifier in ICMPv6 Echo Request and Echo Reply messages. */
#define icm6EchoIdent icm6DataUnion.icm6EchoStruct.icm6Ident
            tt16Bit icm6Ident;
/* Sequence number in ICMPv6 Echo Request and Echo Reply messages. */
#define icm6EchoSeqno icm6DataUnion.icm6EchoStruct.icm6Seqno
            tt16Bit icm6Seqno;
        } icm6EchoStruct;
        struct { 
/* Maximum response delay in ICMPv6 Multicast Listener Discovery messages,
   units are milliseconds. */
#define icm6MldMaxRespDelay icm6DataUnion.icm6MldStruct.icm6MaxRespDelayMsec
#define icm6MldReserved1 icm6DataUnion.icm6MldStruct.icm6MaxRespDelayMsec
            tt16Bit icm6MaxRespDelayMsec;
#define icm6MldNumberOfMcRecs icm6DataUnion.icm6MldStruct.icm6Reserved
#define icm6MldReserved icm6DataUnion.icm6MldStruct.icm6Reserved
            tt16Bit icm6Reserved;
        } icm6MldStruct;
        struct {
            tt32Bit icm6Reserved;
        } icm6RouterSolicitStruct;
        struct {
/* Current hop limit in ICMPv6 Router Advertisement message. */
#define icm6RouterAdvertHopLimit \
    icm6DataUnion.icm6RouterAdvertStruct.icm6HopLimit
            tt8Bit icm6HopLimit;
/* Managed address configuration and Other stateful configuration flags in
   ICMPv6 Router Advertisement message:
   TM_6_ND_RA_MANAGED_FLAG
   TM_6_ND_RA_OTHER_FLAG */
#define icm6RouterAdvertFlags icm6DataUnion.icm6RouterAdvertStruct.icm6Flags
            tt8Bit icm6Flags;
/* Router lifetime in ICMPv6 Router Advertisement message. */
#define icm6RouterAdvertLifetime \
    icm6DataUnion.icm6RouterAdvertStruct.icm6Lifetime
            tt16Bit icm6Lifetime;
        } icm6RouterAdvertStruct;
        struct {
            tt32Bit icm6Reserved;
        } icm6NeighborSolicitStruct;
        struct {
/* Router, Solicited and Override flags in ICMPv6 Neighbor Advertisement
   message:
   TM_6_ND_NA_ROUTER_FLAG
   TM_6_ND_NA_SOLICITED_FLAG
   TM_6_ND_NA_OVERRIDE_FLAG */
#define icm6NeighborAdvertFlags \
    icm6DataUnion.icm6NeighborAdvertStruct.icm6Flags
            tt8Bit icm6Flags;
            tt8Bit icm6Reserved1;
            tt16Bit icm6Reserved2;
        } icm6NeighborAdvertStruct;
        struct {
            tt32Bit icm6Reserved;
        } icm6RedirectStruct;
    } icm6DataUnion;
} tt6IcmpHeader;
#endif /* !TM_DSP */

typedef tt6IcmpHeader TM_FAR * tt6IcmpHeaderPtr;

/* Network header: ICMPv6 Multicast Listener Discovery */
typedef struct ts6MldMsg
{
/* The 32-bit ICMPv6 header, and MLD maximum response delay (i.e.
   mld6Hdr.icm6MldMaxRespDelay). */
    tt6IcmpHeader mld6Hdr;
/* For General Query messages, mld6Addr is set to 0 upon transmission. For
   Multicast-Address-Specific Query messages, mld6Addr is set to the specific
   IPv6 multicast address that is being queried. For Report or Done messages,
   mld6Addr is set to the specific IPv6 multicast address which the sender is
   listening on or is ceasing to listen on, respectively. */
    tt6IpAddrArr mld6Addr;
#ifdef TM_6_USE_MLDV2
/* Robustness Variable */
    tt8Bit          mld6QRV;
/* Query Interval in seconds */
    tt8Bit          mld6QQIC;
/* Number of Source Lists */
    tt16Bit         mld6NumOfSrcList;
/* This structure contains the number of source addresses and the list of source addresses.*/
    tt6IpAddressPtr mld6SourceArrayPtr;
#endif /* TM_6_USE_MLDV2 */
} tt6MldMsg;
typedef tt6MldMsg TM_FAR * tt6MldMsgPtr;

#ifdef TM_6_USE_MLDV2
typedef struct    ts6Mldv2ReportMsg
{
     tt8Bit         mld6RecType;
     tt8Bit         mld6AuxDataType;
     tt16Bit        mld6NumberOfSources;
     tt6IpAddrArr   mld6McAddr;
} tt6Mldv2ReportMsg;
typedef tt6Mldv2ReportMsg TM_FAR * tt6Mldv2ReportMsgPtr;
#endif /* TM_6_USE_MLDV2 */

/* Network header: ICMPv6 Router Solicitation */
typedef struct ts6RouterSolicitMsg
{
/* The 32-bit ICMPv6 header. */
    tt6IcmpHeader rs6Hdr;
} tt6RouterSolicitMsg;
typedef tt6RouterSolicitMsg TM_FAR * tt6RouterSolicitMsgPtr;

/* Network header: ICMPv6 Router Advertisement */
typedef struct ts6RouterAdvertMsg
{ 
/* The 32-bit ICMPv6 header, current hop limit (i.e.
   ra6Hdr.icm6RouterAdvertHopLimit), flags (i.e. ra6Hdr.icm6RouterAdvertFlags)
   and router lifetime (i.e. ra6Hdr.icm6RouterAdvertLifetime). */
    tt6IcmpHeader ra6Hdr;
/* The time, in milliseconds, that a node assumes a neighbor is reachable after
   having received a reachability confirmation. A value of zero means
   unspecified (by this router). */
    tt32Bit ra6ReachableTime;
/* The time, in milliseconds, between retransmitted Neighbor Solicitation
   messages. A value of zero means unspecified (by this router). */
    tt32Bit ra6RetxTimer;
} tt6RouterAdvertMsg;
typedef tt6RouterAdvertMsg TM_FAR * tt6RouterAdvertMsgPtr;

/* Network header: ICMPv6 Neighbor Solicitation/Advertisement */
typedef struct ts6NeighborMsg
{
/* The 32-bit ICMPv6 header. */
    tt6IcmpHeader nb6Hdr;
/* The IP address of the target of the solicitation. It MUST NOT be a multicast
   address. */
    tt6IpAddrArr  nb6Addr;
} tt6NeighborMsg;
typedef tt6NeighborMsg TM_FAR * tt6NeighborMsgPtr;


/* Network header: ICMPv6 Redirect */
typedef struct ts6Redirect
{
/* The 32-bit ICMPv6 header. */
    tt6IcmpHeader rd6Hdr;
/* An IP address that is a better first hop to use for the ICMP Destination
   Address. Will always be on-link. */
    tt6IpAddrArr rd6TargetAddr;
/* The IP address of the destination that is redirected to the target. */
    tt6IpAddrArr rd6DestAddr;
} tt6Redirect;
typedef tt6Redirect TM_FAR * tt6RedirectPtr;

/* Network header: ICMPv6 Neighbor Discovery Option */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#ifndef TM_32BIT_DSP_BIG_ENDIAN
typedef struct ts6NdOptionHeader
{
/* The first 6 bytes of the option data (not used directly on 32-bit DSP). */
    unsigned int ndh6OptionData : 16;
/* Identifies the type of Neighbor Discovery option. */
    unsigned int ndh6OptionType : 8;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    unsigned int ndh6OptionLen  : 8;
} tt6NdOptionHeader;
#else /* TM_32BIT_DSP_BIG_ENDIAN */
typedef struct ts6NdOptionHeader
{
/* Identifies the type of Neighbor Discovery option. */
    unsigned int ndh6OptionType : 8;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    unsigned int ndh6OptionLen  : 8;
/* The first 6 bytes of the option data (not used directly on 32-bit DSP). */
    unsigned int ndh6OptionData : 16;
} tt6NdOptionHeader;
#endif /* !TM_32BIT_DSP_BIG_ENDIAN */
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
typedef struct ts6NdOptionHeader
{
/* Identifies the type of Neighbor Discovery option. */
    unsigned int ndh6OptionType : 8;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    unsigned int ndh6OptionLen  : 8;
/* The first 6 bytes of the option data. */
    tt8Bit       ndh6OptionData[6];
} tt6NdOptionHeader;
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
typedef struct ts6NdOptionHeader
{
/* Identifies the type of Neighbor Discovery option. */
    tt8Bit ndh6OptionType;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    tt8Bit ndh6OptionLen;
/* The first 6 bytes of the option data. */
    tt8Bit ndh6OptionData[6];
} tt6NdOptionHeader;
#endif /* !TM_DSP */
typedef tt6NdOptionHeader TM_FAR * tt6NdOptionHeaderPtr;

/* Network header: ICMPv6 Prefix Information Option */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#ifndef TM_32BIT_DSP_BIG_ENDIAN
typedef struct ts6NdPrefixInfoOption
{
/* On-link and autonomous address-configuration flags:
   TM_6_ND_PI_ONLINK_FLAG
   TM_6_ND_PI_AUTO_FLAG */
    unsigned int ndp6Flags      : 8;
/* The number of leading bits in ndp6Prefix that are valid. */
    unsigned int ndp6PrefixLen  : 8;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    unsigned int ndp6OptionLen  : 8;
/* Identifies the type of Neighbor Discovery message. */
    unsigned int ndp6OptionType : 8;
/* The length of time in seconds (relative to the time the packet is sent) that
   the prefix is valid for the purpose of on-link determination. A value of all
   1's represents infinity. */
    tt32Bit ndp6ValidLifetime;
/* The length of time in seconds (relative to the time the packet is sent) that
   addresses generated from the prefix via stateless address autoconfiguration
   remain preferred. A value of all 1's represents infinity. */
    tt32Bit ndp6PreferredLifetime;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt32Bit ndp6Reserved;
/* An IP address or a prefix of an IP address. ndp6PrefixLen contains the
   number of valid leading bits in the prefix. */
    tt6IpAddrArr ndp6Prefix;
} tt6NdPrefixInfoOption;
#else /* TM_32BIT_DSP_BIG_ENDIAN */
typedef struct ts6NdPrefixInfoOption
{
/* Identifies the type of Neighbor Discovery message. */
    unsigned int ndp6OptionType : 8;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    unsigned int ndp6OptionLen  : 8;
/* The number of leading bits in ndp6Prefix that are valid. */
    unsigned int ndp6PrefixLen  : 8;
/* On-link and autonomous address-configuration flags:
   TM_6_ND_PI_ONLINK_FLAG
   TM_6_ND_PI_AUTO_FLAG */
    unsigned int ndp6Flags      : 8;
/* The length of time in seconds (relative to the time the packet is sent) that
   the prefix is valid for the purpose of on-link determination. A value of all
   1's represents infinity. */
    tt32Bit ndp6ValidLifetime;
/* The length of time in seconds (relative to the time the packet is sent) that
   addresses generated from the prefix via stateless address autoconfiguration
   remain preferred. A value of all 1's represents infinity. */
    tt32Bit ndp6PreferredLifetime;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt32Bit ndp6Reserved;
/* An IP address or a prefix of an IP address. ndp6PrefixLen contains the
   number of valid leading bits in the prefix. */
    tt6IpAddrArr ndp6Prefix;
} tt6NdPrefixInfoOption;
#endif /* !TM_32BIT_DSP_BIG_ENDIAN */
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
typedef struct ts6NdPrefixInfoOption
{
/* Identifies the type of Neighbor Discovery message. */
    unsigned int ndp6OptionType : 8;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    unsigned int ndp6OptionLen  : 8;
/* The number of leading bits in ndp6Prefix that are valid. */
    unsigned int ndp6PrefixLen  : 8;
/* On-link and autonomous address-configuration flags:
   TM_6_ND_PI_ONLINK_FLAG
   TM_6_ND_PI_AUTO_FLAG */
    unsigned int ndp6Flags      : 8;
/* The length of time in seconds (relative to the time the packet is sent) that
   the prefix is valid for the purpose of on-link determination. A value of all
   1's represents infinity. */
    tt32Bit ndp6ValidLifetime;
/* The length of time in seconds (relative to the time the packet is sent) that
   addresses generated from the prefix via stateless address autoconfiguration
   remain preferred. A value of all 1's represents infinity. */
    tt32Bit ndp6PreferredLifetime;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt32Bit ndp6Reserved;
/* An IP address or a prefix of an IP address. ndp6PrefixLen contains the
   number of valid leading bits in the prefix. */
    tt6IpAddrArr ndp6Prefix;
} tt6NdPrefixInfoOption;
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
typedef struct ts6NdPrefixInfoOption
{
/* Identifies the type of Neighbor Discovery message. */
    tt8Bit ndp6OptionType;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    tt8Bit ndp6OptionLen;
/* The number of leading bits in ndp6Prefix that are valid. */
    tt8Bit ndp6PrefixLen;
/* On-link, autonomous address-configuration and router address flags:
   TM_6_ND_PI_ONLINK_FLAG
   TM_6_ND_PI_AUTO_FLAG
   TM_6_ND_PI_RTR_ADDR_FLAG
*/
    tt8Bit ndp6Flags;
/* The length of time in seconds (relative to the time the packet is sent) that
   the prefix is valid for the purpose of on-link determination. A value of all
   1's represents infinity. */
    tt32Bit ndp6ValidLifetime;
/* The length of time in seconds (relative to the time the packet is sent) that
   addresses generated from the prefix via stateless address autoconfiguration
   remain preferred. A value of all 1's represents infinity. */
    tt32Bit ndp6PreferredLifetime;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt32Bit ndp6Reserved;
/* An IP address or a prefix of an IP address. ndp6PrefixLen contains the
   number of valid leading bits in the prefix. */
    tt6IpAddrArr ndp6Prefix;
} tt6NdPrefixInfoOption;
#endif /* !TM_DSP */
typedef tt6NdPrefixInfoOption TM_FAR * tt6NdPrefixInfoOptionPtr;

/* Network header: ICMPv6 Advertisement Interval Option */
typedef struct ts6NdRtrAdvIntervalOption
{
/* Identifies the type of Neighbor Discovery message. */
    tt8Bit      nda6OptionType;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    tt8Bit      nda6OptionLen;
    tt16Bit     nda6Reserved;
/* The maximum time, in milliseconds, between successive unsolicited router
   Router Advertisement messages sent by this router on this network
   interface */
    tt32Bit     nda6RtrAdvInterval;
} tt6NdRtrAdvIntervalOption;
typedef tt6NdRtrAdvIntervalOption TM_FAR * tt6NdRtrAdvIntervalOptionPtr;

/* Network header: ICMPv6 Redirected Header Option */
typedef struct ts6NdRedirectHdrOption
{
/* Identifies the type of Neighbor Discovery message. */
    tt8Bit ndr6OptionType;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    tt8Bit ndr6OptionLen;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt16Bit ndr6Reserved1;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt32Bit ndr6Reserved2;
/* The original packet that is being redirected, truncated to ensure that the
   size of the Redirect message does not exceed the minimum link MTU. */
    tt6IpHeader ndr6IpHdr;
} tt6NdRedirectHdrOption;
typedef tt6NdRedirectHdrOption TM_FAR * tt6NdRedirectHdrOptionPtr;

/* Network header: ICMPv6 MTU Option */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#ifndef TM_32BIT_DSP_BIG_ENDIAN
typedef struct ts6NdMtuOption
{
/* Reserved. Set to 0 on transmit, ignored on receive. */
    unsigned int ndm6Reserved   : 16;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    unsigned int ndm6OptionLen  : 8;
/* Identifies the type of Neighbor Discovery message. */
    unsigned int ndm6OptionType : 8;
/* The recommended MTU for the link. */
    tt32Bit ndm6Mtu;
} tt6NdMtuOption;
#else /* TM_32BIT_DSP_BIG_ENDIAN */
typedef struct ts6NdMtuOption
{
/* Identifies the type of Neighbor Discovery message. */
    unsigned int ndm6OptionType : 8;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    unsigned int ndm6OptionLen  : 8;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    unsigned int ndm6Reserved   : 16;
/* The recommended MTU for the link. */
    tt32Bit ndm6Mtu;
} tt6NdMtuOption;
#endif /* !TM_32BIT_DSP_BIG_ENDIAN */
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
typedef struct ts6NdMtuOption
{
/* Identifies the type of Neighbor Discovery message. */
    unsigned int ndm6OptionType : 8;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    unsigned int ndm6OptionLen  : 8;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt16Bit ndm6Reserved;
/* The recommended MTU for the link. */
    tt32Bit ndm6Mtu;
} tt6NdMtuOption;
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
typedef struct ts6NdMtuOption
{
/* Identifies the type of Neighbor Discovery message. */
    tt8Bit ndm6OptionType;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    tt8Bit ndm6OptionLen;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt16Bit ndm6Reserved;
/* The recommended MTU for the link. */
    tt32Bit ndm6Mtu;
} tt6NdMtuOption;
#endif /* !TM_DSP */
typedef tt6NdMtuOption TM_FAR * tt6NdMtuOptionPtr;

/* Network header: ICMPv6 Recursive DNS Server Option */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#ifndef TM_32BIT_DSP_BIG_ENDIAN
typedef struct ts6NdRdnssOption
{
/* Reserved. Set to 0 on transmit, ignored on receive. */
    unsigned int ndrd6Reserved : 16;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    unsigned int ndrd6OptionLen : 8;
/* Identifies the type of Neighbor Discovery message. */
    unsigned int ndrd6OptionType : 8;
/* The maximum time, in seconds (relative to the time the packet is sent),
   over which this RDNSS address MAY be used for name resolution. */
    tt32Bit ndrd6Lifetime;
/* One or more 128-bit IPv6 addresses of the recursive DNS servers. */
    tt6IpAddrArr ndrdRdnssAddr;
} tt6NdRdnssOption;
#else /* TM_32BIT_DSP_BIG_ENDIAN */
typedef struct ts6NdRdnssOption
{
/* Identifies the type of Neighbor Discovery message. */
    unsigned int ndrd6OptionType : 8;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    unsigned int ndrd6OptionLen : 8;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    unsigned int ndrd6Reserved : 16;
/* The maximum time, in seconds (relative to the time the packet is sent),
   over which this RDNSS address MAY be used for name resolution. */
    tt32Bit ndrd6Lifetime;
/* One or more 128-bit IPv6 addresses of the recursive DNS servers. */
    tt6IpAddrArr ndrdRdnssAddr;
} tt6NdRdnssOption;
#endif /* !TM_32BIT_DSP_BIG_ENDIAN */
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
typedef struct ts6NdRdnssOption
{
/* Identifies the type of Neighbor Discovery message. */
    unsigned int ndrd6OptionType : 8;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    unsigned int ndrd6OptionLen : 8;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt16Bit ndrd6Reserved;
/* The maximum time, in seconds (relative to the time the packet is sent),
   over which this RDNSS address MAY be used for name resolution. */
    tt32Bit ndrd6Lifetime;
/* One or more 128-bit IPv6 addresses of the recursive DNS servers. */
    tt6IpAddrArr ndrdRdnssAddr;
} tt6NdRdnssOption;
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
typedef struct ts6NdRdnssOption
{
/* Identifies the type of Neighbor Discovery message. */
    tt8Bit ndrd6OptionType;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    tt8Bit ndrd6OptionLen;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt16Bit ndrd6Reserved;
/* The maximum time, in seconds (relative to the time the packet is sent),
   over which this RDNSS address MAY be used for name resolution. */
    tt32Bit ndrd6Lifetime;
/* One or more 128-bit IPv6 addresses of the recursive DNS servers. */
    tt6IpAddrArr ndrdRdnssAddr;
} tt6NdRdnssOption;
#endif /* !TM_DSP */
typedef tt6NdRdnssOption TM_FAR * tt6NdRdnssOptionPtr;

typedef struct ts6NdRdnss
{
    ttNode rdnssNode;
    ttTimerPtr timerPtr;
    tt6IpAddress rdnssAddress;
    tt32Bit rdnssLifetime;
} tt6NdRdnss;
typedef tt6NdRdnss * tt6NdRdnssPtr;

/* Network header: ICMPv6 DNS Search List Option */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#ifndef TM_32BIT_DSP_BIG_ENDIAN
typedef struct ts6NdDnsslOption
{
/* Reserved. Set to 0 on transmit, ignored on receive. */
    unsigned int ndsl6Reserved : 16;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    unsigned int ndsl6OptionLen : 8;
/* Identifies the type of Neighbor Discovery message. */
    unsigned int ndsl6OptionType : 8;
/* The maximum time, in seconds (relative to the time the packet is sent),
   over which this DNSSL domain name MAY be used for name resolution. */
    tt32Bit ndsl6Lifetime;
/* One or more domain names of DNS Search List that MUST be encoded using the
   technique described in Section 3.1 of [RFC1035]. */
    tt8Bit ndsl6Dnssl;
} tt6NdDnsslOption;
#else /* TM_32BIT_DSP_BIG_ENDIAN */
typedef struct ts6NdDnsslOption
{
/* Identifies the type of Neighbor Discovery message. */
    unsigned int ndsl6OptionType : 8;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    unsigned int ndsl6OptionLen : 8;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    unsigned int ndsl6Reserved : 16;
/* The maximum time, in seconds (relative to the time the packet is sent),
   over which this DNSSL domain name MAY be used for name resolution. */
    tt32Bit ndsl6Lifetime;
/* One or more domain names of DNS Search List that MUST be encoded using the
   technique described in Section 3.1 of [RFC1035]. */
    tt8Bit ndsl6Dnssl;
} tt6NdDnsslOption;
#endif /* !TM_32BIT_DSP_BIG_ENDIAN */
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
typedef struct ts6NdDnsslOption
{
/* Identifies the type of Neighbor Discovery message. */
    unsigned int ndsl6OptionType : 8;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    unsigned int ndsl6OptionLen : 8;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt16Bit ndsl6Reserved;
/* The maximum time, in seconds (relative to the time the packet is sent),
   over which this DNSSL domain name MAY be used for name resolution. */
    tt32Bit ndsl6Lifetime;
/* One or more domain names of DNS Search List that MUST be encoded using the
   technique described in Section 3.1 of [RFC1035]. */
    tt8Bit ndsl6Dnssl;
} tt6NdDnsslOption;
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
typedef struct ts6NdDnsslOption
{
/* Identifies the type of Neighbor Discovery message. */
    tt8Bit ndsl6OptionType;
/* Length of the Neighbor Discovery option (including the type and length
   fields) in 8-byte units. */
    tt8Bit ndsl6OptionLen;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt16Bit ndsl6Reserved;
/* The maximum time, in seconds (relative to the time the packet is sent),
   over which this DNSSL domain name MAY be used for name resolution. */
    tt32Bit ndsl6Lifetime;
/* One or more domain names of DNS Search List that MUST be encoded using the
   technique described in Section 3.1 of [RFC1035]. */
    tt8Bit ndsl6Dnssl;
} tt6NdDnsslOption;
#endif /* !TM_DSP */
typedef tt6NdDnsslOption TM_FAR * tt6NdDnsslOptionPtr;

typedef struct ts6NdDnssl
{
    ttNode dnsslNode;
    ttTimerPtr timerPtr;
    tt8Bit dnsslDomain[TM_6_DNSSL_MAX_DOMAIN_LEN];
    tt32Bit dnsslLifetime;
} tt6NdDnssl;
typedef tt6NdDnssl * tt6NdDnsslPtr;

/* Multi home function pointer. Used in all interface matching functions */
typedef  int  (*tt6MhomeFunctPtr)(ttDeviceEntryPtr devPtr,
                                  tt6ConstIpAddressPtr  ipAddr,
                                  tt16BitPtr mhomeIndexPtr);

#ifdef TM_6_USE_PREFIX_DISCOVERY
typedef void (*tt6PrefixFuncPtr)(
    ttDeviceEntryPtr devEntryPtr, tt6IpAddressPtr prefixPtr);
#endif /* TM_6_USE_PREFIX_DISCOVERY */


/*
 * Command structure used by tf6RtKeyCommand function.
 */ 
typedef struct ts6RtKeyCommand
{
/* Pointer to routing key address. Set by the caller. */
    tt6IpAddressPtr     rkc6AddrPtr;
/* Tunnel endpoint address. */
    tt6IpAddressPtr     rkc6EndpointAddrPtr;
/* TM_6_RT_KEY_EXISTS_ON_DEV check for match on device pointer */
    ttDeviceEntryPtr    rkc6DevPtr;
/* Valid lifetime of prefix */    
    tt32Bit             rkc6ValidLifetime;
/* Preferred lifetime of prefix. */    
    tt32Bit             rkc6PrefLifetime;
/* Flags: non on-link */    
#define TM_6_RKC_NON_ONLINK  1
    tt16Bit             rkc6Flags;
/* Prefix length of prefix (rkc6AddrPtr). Set by the caller. */
    tt8Bit              rkc6PrefixLength;
/* Routing key command.  Set by the caller. */    
    tt8Bit              rkc6Command;
} tt6RtKeyCommand;

typedef tt6RtKeyCommand TM_FAR * tt6RtKeyCommandPtr;

/* ICMP message handling for errors within tunnels. */
typedef void (*tt6TunnelIcmpErrorFuncPtr)(
    ttPacketPtr packetPtr,
    tt8BitPtr   origIpHdrPtr);

#if ( defined(TM_6_USE_MIP_MN) || defined (TM_6_USE_MIP_HA) )
/* Network header: ICMPv6 Dynamic Home Agent Address Discovery */
typedef struct ts6DhaadMsg
{
    tt8Bit          dhaadMsgType;       /* ICMP type */
    tt8Bit          dhaadMsgCode;       /* ICMP code */
    tt16Bit         dhaadMsgChecksum;   /* ICMP checksume */
    tt16Bit         dhaadMsgId;         /* DHAAD identifier */
    tt16Bit         dhaadMsgReserved;   /* DHAAD reserved */
    tt6IpAddrArr    dhaadMsgHaAddrList; /* DHAAD home agent addr list */
} tt6DhaadMsg;
typedef tt6DhaadMsg TM_FAR * tt6DhaadMsgPtr;
#define  TM_6_DHAAD_MSG_NULL_PTR (tt6DhaadMsgPtr)0

/* Network header: ICMPv6 Mobile Prefix Discovery */
typedef struct ts6MobilePrefixMsg
{
    tt8Bit          mobilePrefixMsgType;       /* ICMP type */
    tt8Bit          mobilePrefixMsgCode;       /* ICMP code */
    tt16Bit         mobilePrefixMsgChecksum;   /* ICMP checksume */
    tt16Bit         mobilePrefixMsgId;         /* MOBILE_PREFIX identifier */
    tt16Bit         mobilePrefixMsgReserved;   /* MOBILE_PREFIX reserved */
    tt8Bit          mobilePrefixMsgOpts[4];     /* MOBILE_PREFIX opts list */
} tt6MobilePrefixMsg;
typedef tt6MobilePrefixMsg TM_FAR * tt6MobilePrefixMsgPtr;
#define  TM_6_MOBILE_PREFIX_MSG_NULL_PTR (tt6MobilePrefixMsgPtr)0

#endif /* MN or HA */


#ifdef TM_6_USE_MIP_MN

/* states for tf6MnStateMachine and tf6MnRrStateMachine */
typedef enum te6MnsState
{
/* Idle: there is no existing mobility binding, registration is not in
   progress, and all return routability and binding-related messaging is
   silently ignored. */
    TM_6_MIP_IDLE,
/* WaitA: the mobile node has sent a Binding Update, and is only waiting for
   the solicited Binding Acknowledgement message to arrive. There is no
   existing mobility binding. */
    TM_6_MIP_WAIT_BIND_ACK,
/* WaitD: the mobility node has sent a de-registration Binding Update, and is
   only waiting for the solicited Binding Acknowledgement message to arrive. */
    TM_6_MIP_DEREG_WAIT_BIND_ACK,
/* Bound: the mobile node has established a mobility binding with the home
   agent or correspondent node. */
    TM_6_MIP_BOUND,
/* WaitAR: the mobile node has an existing mobility binding, which is being
   refreshed with a Binding Update. The mobile node is waiting for the
   solicited Binding Acknowledgement. */
    TM_6_MIP_REFRESH_WAIT_BIND_ACK,

/*** END OF STATES FOR HOME REGISTRATION BUL ENTRIES ***/

/* RRInit: the mobile node has initiated the return routability procedure but
   has not yet completed it, and has no existing route optimization mobility
   binding for the specified correspondent node. */
    TM_6_MIP_INIT_HOTI_COTI,
/* RRDel: the mobile node intends to send a de-registration Binding Update
   later but is first waiting for a home keygen token before this can be done.
   ([ISSUE185].R11.6.1:30) */
    TM_6_MIP_DEREG_NEED_HOME_KEYGEN,
/* RRRedo: the mobile node has an existing route optimization mobility
   binding and has initiated the return routability procedure in order to
   refresh it with the specified correspondent node. */
    TM_6_MIP_REDO_HOTI_COTI,
/* The specified correspondent node does not support route optimization.
   ([MIPV6_18++].R11.7.5:50, [MIPV6_18++].R11.9:10) */
    TM_6_MIP_DISABLED 
} tt6MnsState;

/* events for tf6MnStateMachine and tf6MnRrStateMachine */
typedef enum te6MnsEvent
{
/* Received valid Binding Acknowledgement message from the home agent or
   correspondent node with status: 135 Sequence number out of window
   ([MIPV6_18++].R8.5:40) */
    TM_6_MIP_BIND_ACK_BAD_SEQNO_EVENT,
/* Received valid Binding Acknowledgement message from the home agent or
   correspondent node with status: 1 Accepted but prefix discovery necessary
   ([MIPV6_18++].R8.5:40) */
    TM_6_MIP_BIND_ACK_DEPRECATED_EVENT,
/* Received valid Binding Acknowledgement message from the home agent or
   correspondent node with a non-recoverable failure status, including:
   128 Reason unspecified
   129 Administratively prohibited
   130 Insufficient resources
   131 Home registration not supported
   132 Not home subnet
   133 Not home agent for this mobile node
   134 Duplicate Address Detection failed
   ([MIPV6_18++].R8.5:40) */
    TM_6_MIP_BIND_ACK_HARD_FAIL_EVENT,
/* Received valid Binding Acknowledgement message from the home agent or
   correspondent node with a successful status (< 128) other than:
   1 Accepted but prefix discovery necessary
   ([MIPV6_18++].R8.5:40) */
    TM_6_MIP_BIND_ACK_SUCCESS_EVENT,
/* Received valid Binding Refresh Request message from the correspondent node.
   ([MIPV6_18++].R8.5:60) */
    TM_6_MIP_BIND_REFRESH_REQ_EVENT,
/* Failure timer (i.e. tt6MnHaBulEntry.hab6UserFailureTmrPtr) associated with
   this binding update list entry has expired. */
    TM_6_MIP_FAILURE_TIMER_EVENT,
/* Notification of mobile node L3 movement away from home, a new primary
   care-of address has already been set in the binding update list entry that
   needs to be registered with the home agent or correspondent node. */
    TM_6_MIP_L3_MOVE_NOT_HOME_EVENT,
/* Notification of mobile node returning home. */
    TM_6_MIP_RETURN_HOME_EVENT,
/* Retransmit timer associated with this binding update list entry has
   expired. */
    TM_6_MIP_RETX_TIMER_EVENT,
/* User request to deregister a mobility binding, refer to
   tf6MnRegisterBinding and tf6MnStopMobileIp. */
    TM_6_MIP_USER_DEREGISTER_EVENT,
/* User request to register a mobility binding, refer to
   tf6MnRegisterBinding. */
    TM_6_MIP_USER_REGISTER_EVENT,

/*** END OF EVENTS FOR HOME REGISTRATION BUL ENTRIES ***/

/* Received valid Binding Acknowledgement message from the correspondent node
   with status one of:
   136 Expired home nonce index
   137 Expired care-of nonce index
   138 Expired nonces
   ([MIPV6_18++].R8.5:40) */
    TM_6_MIP_BIND_ACK_BAD_NONCE_EVENT,
/* Received valid Binding Error message from the correspondent node with
   status: 2 Unrecognized MH Type value
   ([MIPV6_20].R8.5:120) */
    TM_6_MIP_BIND_ERR_BAD_MH_TYPE_EVENT,
/* Received valid Binding Error message from the correspondent node with
   status: 1 Unknown binding for Home Address destination option
   ([MIPV6_20].R8.5:120) */
    TM_6_MIP_BIND_ERR_NO_BINDING_EVENT,
/* Received valid Care-of Test message from the correspondent node */
    TM_6_MIP_COT_EVENT,
/* Notification by associated home registration binding update list entry
   that this binding must be deregistered. */
    TM_6_MIP_DEREGISTER_EVENT ,
/* Received valid Home Test message from the correspondent node. */
    TM_6_MIP_HOT_EVENT,
/* ICMPv6 Parameter Problem error code 1:
   Mobility Header protocol not supported. ([MIPV6_20].R8.5:130) */
    TM_6_MIP_ICMP_PARM_PROB_1_EVENT
} tt6MnsEvent;


#ifdef TM_6_USE_MIP_RO
/* Mobile IPv6 mobile node route optimization binding update list entry
  ([MIPV6_18++].R11.1:20, [MIPV6_18++].R8.5:50) */
typedef struct ts6MnRrBulEntry
{
/* ttNode MUST be the first field. Supports insertion of this structure into
   a generic linked-list ttList. */
    ttNode              rrb6RrBulNode;
#define rrb6RrBulNextPtr(rrBulPtr) \
    ((tt6MnRrBulEntryPtr)(ttVoidPtr)(rrBulPtr->rrb6RrBulNode.nodeNextPtr))
#define rrb6RrBulPrevPtr(rrBulPtr) \
    ((tt6MnRrBulEntryPtr)(ttVoidPtr)(rrBulPtr->rrb6RrBulNode.nodePrevPtr))

/* Retransmission timer used to schedule retransmission of BUs, HoTI and
   CoTI messages. */
    ttTimerPtr          rrb6ReTxTimerPtr;
/* Failure timer used to kill deregistration when the CN does not respond */
    ttTimerPtr          rrb6FailureTimerPtr;
/* Correspondent node address associated with this binding. */
    struct in6_addr     rrb6CnAddr;
/* Primary care-of address associated with this binding. */
    struct in6_addr     rrb6CareOfAddr;
/* Mobile cookie value used in the HoTI and CoTI messages. */
    tt8Bit              rrb6MnCookie[TM_6_MIP_COOKIE_LEN];
/* Home keygen token received from the correspondent node in the HoT message */
    tt8Bit              rrb6HotKeygen[TM_6_MIP_KEYGEN_LEN];
/* Care-of keygen token received from the correspondent node in the
   CoT message */
    tt8Bit              rrb6CotKeygen[TM_6_MIP_KEYGEN_LEN];
/* Granted lifetime of this binding. Specified by the correspondent node
   in a solicited BA. Used to quickly determine if the binding is in the
   "registered" state, in which case it is non-zero. */
    tt32Bit             rrb6LifetimeMsec;
/* Latched millisecond tick count (tvTime) of the last time we sent a BU for
   this binding. */
    tt32Bit             rrb6BuSentTime;
/* Latched millisecond tick count (tvTime) of the last time we received a
   solicited BA for this binding. */
    tt32Bit             rrb6BuAckTime;
/* Latched millisecond tick count (tvTime) of the last time we used this
   binding in the data path to route optimize application data. */
    tt32Bit             rrb6LastUsedTime;
/* Latched millisecond tick count (tvTime) of the last time we stored a home
   keygen token for this binding. Used to determine if we can reuse a stored
   home keygen token when we execute a L3 move or deregister. Refer to
   TM_6_MIP_MIN_TOKEN_LIFE_MSEC
   ([ISSUE103].R5.2.7:60, [MIPV6_18++].R11.6.1:20) */
    tt32Bit             rrb6HomeKeygenStoredTime;
/* Sequence number value we used in the last BU that we sent to the
   correspondent node, in host byte order. ([MIPV6_18++].R11.7.2:190) */
    tt16Bit             rrb6BuSentSeq;
/* Number of BU or HoTI/CoTI retransmissions that we've done since we last
   stopped and restarted the retransmission timer (rrb6ReTxTimerPtr). */
    tt16Bit             rrb6ReTxCount;
/* Home nonce index received from the correspondent node. */
    tt16Bit             rrb6HoNonceIndex;
/* Care-of nonce index received from the correspondent node. */
    tt16Bit             rrb6CoNonceIndex;
/* Multi-home index of the associated home address on the virtual home
   interface. */
    tt16Bit             rrb6HomeAddrMhomeIndex;
/* State of this route optimization BUL entry. This is managed by
   tf6MnRrStateMachine. */
    tt8Bit              rrb6State;
/* The state of the return routability procedure for this binding, refer to
   the tt6MnRrStatus enumeration type. */
    tt8Bit              rrb6MnRrStatus;
} tt6MnRrBulEntry; 
typedef tt6MnRrBulEntry TM_FAR * tt6MnRrBulEntryPtr;
#endif /* TM_6_USE_MIP_RO */

/* MIPv6 mobile node home registration binding update list entry
   ([MIPV6_18++].R11.1:20, [MIPV6_18++].R8.5:50) */
typedef struct ts6MnHaBulEntry
{
#ifdef TM_6_USE_MIP_RO
/* List of route optimization binding update list entries associated with
   this home registration binding update list entry. Kept in order by most
   recent use in the data path, with the most recently used entry at the head
   of the list. */
    ttList              hab6RrBulList;
#define hab6RrBulHeadPtr(haBulPtr)  \
    ((tt6MnRrBulEntryPtr)(ttVoidPtr) \
                         haBulPtr->hab6RrBulList.listHeadNode.nodeNextPtr)
#endif /* TM_6_USE_MIP_RO */

/* Retransmission timer used to schedule retransmission of BUs */
    ttTimerPtr          hab6ReTxTimerPtr;
/* The function to call to notify the user of Mobile IPv6 binding events.
   Refer to the mnBindingNotifyFuncPtr parameter of tf6MnRegisterBinding. */
    tt6MnBindingNotifyFuncPtr hab6UserNotifyFuncPtr;
/* Optional failure timer, period is hab6UserFailureMsec. */
    ttTimerPtr          hab6UserFailureTmrPtr;
/* Primary care-of address associated with this binding. */
    struct in6_addr     hab6CareOfAddr;
/* Optional failure timer period: specifies how long we keep trying to
   register or deregister this binding without receiving a response from the
   home agent. Specified by the user, refer to the timeoutSec parameter of
   tf6MnRegisterBinding. */
    tt32Bit             hab6UserFailureMsec;
/* The granted lifetime of this binding. Specified by the home agent in a
   solicited BA. */
    tt32Bit             hab6LifetimeMsec;
/* The optional re-registration period specified by the home agent in the
   Binding Refresh Advice option of the BA. */
    tt32Bit             hab6RefreshPeriodMsec;
/* Latched millisecond tick count (tvTime) of the last time we sent a BU for
   this binding. */
    tt32Bit             hab6BuSentTime;
/* Latched millisecond tick count (tvTime) of the last time we received a
   solicited BA for this binding. Used to determine when the granted lifetime
   has expired. */
    tt32Bit             hab6BuAckTime;
/* The sequence number value we used in the last BU that we sent to the home
   agent. ([MIPV6_18++].R11.7.1:80) */
    tt16Bit             hab6BuSentSeq;
/* The user-specified lifetime for this binding, refer to the
   maxHaBindingLifetimeSec parameter of tf6MnRegisterBinding. */
    tt16Bit             hab6UserReqLifetime;
/* The number of BU retransmissions that we've done since we last stopped
   and restarted the retransmission timer (hab6ReTxTimerPtr). */
    tt16Bit             hab6ReTxCount;
/* The multi-home index of the associated home address on the virtual home
   interface. */
    tt16Bit             hab6HomeAddrMhomeIndex;
/* The state of this home registration BUL entry. This is managed by
   tf6MnStateMachine. */
    tt8Bit              hab6State;
/* Boolean status indicates if the user has requested that we deregister
   this binding (refer to tf6MnRegisterBinding and tf6MnStopMobileIp). */
    tt8Bit              hab6UserDeregStatus;
/* An 8-bit unsigned integer status code received in the Binding
   Acknowledgement message. This is passed to the user's notify function
   that they registered for notification of binding events. */
    tt8Bit              hab6UserBindAckStatus;
    tt8Bit              padding;
} tt6MnHaBulEntry;
typedef tt6MnHaBulEntry TM_FAR * tt6MnHaBulEntryPtr;

/* Mobile IPv6 mobile node state vector */
typedef struct ts6MnVect
{
#ifdef TM_LOCK_NEEDED
/* lock used to serialize access to this data structure and the associated
   binding update list (mns6BulPtr). */
    ttLockEntry         mns6LockEntry;
#endif /* TM_LOCK_NEEDED*/

#ifdef TM_PEND_POST_NEEDED
/* Semaphore used in blocking mode */
    ttPendEntry         mns6StopPendEntry;
#endif /* TM_PEND_POST_NEEDED */

/* The user-specified home agent address. Refer to the homeAgentAddrPtr
   parameter of tf6MnStartMobileIp. */
    struct in6_addr     mns6HomeAgentAddr;

/* The link-local scope address of the current default router with the
   scope ID embedded, discovered on the mobile interface. This is the source
   address for Router Advertisement messages, used to implement
   [MIPV6_18++].R11.5.1:60.
   NOTE: we always access this field in a critical section, so we can avoid
   having to lock the MN state vector. */
    struct in6_addr     mns6LinkLocalDefRtrAddr;

#ifdef TM_6_USE_MIP_RA_RTR_ADDR
/* Global scope addresses of the current default router, discovered from
   the Router Address option in received Router Advertisements */
    struct in6_addr     mns6GlobalDefRtrAddrs[TM_6_MN_RA_RTR_ADDR_MAX_CNT];
/* Count of global scope router addresses stored in mns6GlobalDefRtrAddrs,
   protected by critical section so we can avoid locking the MN state vector */
    int                 mns6GlobalDefRtrAddrsCount;
#endif /* TM_6_USE_MIP_RA_RTR_ADDR */

/* Pointers to mobile node binding update lists (one for each home address),
   indexed using the multi-home index of the associated home address on the
   virtual home interface. Points to the home registration BUL entries, which
   contain a ttList of route optimization BUL entries.
   ([MIPV6_18++].R11.1:10, [MIPV6_18++].R8.5:50) */
    tt6MnHaBulEntryPtr  mns6BulPtr[TM_6_MAX_LOCAL_MHOME];
/* Pointer to the virtual home interface, used to keep track of mobile node
   home addresses when the mobile node is attached to a foreign link. */
    ttDeviceEntryPtr    mns6VirtHomeIfacePtr;
/* Pointer to the current mobile interface, specified by the user calling
   tf6MnStartMobileIp or tf6MnMoveNotify. */
    ttDeviceEntryPtr    mns6MobileIfacePtr;
/* The function to call to notify the user of Mobile IPv6 Mobile Node
   events. Refer to the mnNotifyFuncPtr parameter of tf6MnStartMobileIp. */
    tt6MnNotifyFuncPtr  mns6UserNotifyFuncPtr;
/* User-specified IPv6 address prefix that the MN uses to detect when it is
   operating on the home network versus on a foreign network. Refer to the
   homePrefixPtr parameter of tf6MnStartMobileIp. */
    struct in6_addr     mns6HomePrefix;
/* When MIPv6 state bit TM_6_MNS_REG_PRIMARY_COA is set, this stores the
   primary care-of address most recently registered with the home agent
   using the API tf6MnRegisterBinding. */
    struct in6_addr     mns6RegPrimaryCoa;
/* When MIPv6 state bit TM_6_MNS_PREV_PRIMARY_COA is set, this stores the
   previous primary care-of address registered with the home agent.
   ([MIPV6_18++].R11.5.3:30) */
    struct in6_addr     mns6PrevPrimaryCoa;
/* Timer used to implement the Elmic option TM_6_OPTION_MN_BEACON_THRESHOLD.
   ([MIPV6_18++].R11.5.1:60) */
    ttTimerPtr          mns6BeaconTimerPtr;
/* Advertisement Interval used to monitor Router Advertisement beacons sent by
   the current default router */
    tt32Bit             mns6BeaconIntervalMsec;
    
#ifdef TM_USE_IKE
/* IKE socket descriptor per home address */
    int                 mns6IkeSd[TM_6_MAX_LOCAL_MHOME];
#endif /* TM_USE_IKE */

#ifdef TM_6_USE_PREFIX_DISCOVERY
/* Timer that we use to retransmit the Mobile Prefix Solicitation message
   when we are waiting for a reply, refer to tf6MipPrefixSolicitReTxTimer. */
    ttTimerPtr          mns6PrefixSolicitTimerPtr;
/* 16-bit ICMP identifier of the last sent Mobile Prefix Solicitation message*/
    tt16Bit             mns6PrefixSolicitIdent;
/* The current backoff retransmission peroid */
    tt16Bit             mns6PrefixSolicitRetranPeriod;
#endif /* ! TM_6_USE_PREFIX_DISCOVERY */

/* User-specified home prefix length, refer to the homePrefixLen parameter
   of tf6MnStartMobileIp. */
    tt8Bit              mns6HomePrefixLen;
/* Mobile node state flags */
    tt8Bit              mns6Flags;
    tt8Bit              mns6Padding[2];
} tt6MnVect;

/* DHAAD state machine data */
typedef struct ts6DhaadInfo
{
    tt6IpAddress                dhaadHomePrefix;
    tt8Bit                      dhaadHomePrefixLen;
    tt8Bit                      dhaadState;
    tt8Bit                      dhaadRetriesCount;
    tt8Bit                      dhaadNumberOfAddrs;
    ttPendEntry                 dhaadPendEntry;
    ttLockEntry                 dhaadLockEntry;
    ttTimerPtr                  dhaadRetryTimerPtr;
    tt16Bit                     dhaadId;    /* identifier in the REQ and REP */
    tt16Bit                     dhaadAddrBufferLen;
    struct sockaddr_storage *   dhaadHomeAgentAddrPtr;
} tt6DhaadInfo;
typedef tt6DhaadInfo TM_FAR * tt6DhaadInfoPtr;
#define TM_6_DHAAD_ENRY_NULL_PTR (tt6DhaadInfoPtr)0;

#endif /* TM_6_USE_MIP_MN */


#ifdef TM_6_USE_MIP_CN
/* the binding cache entry */
typedef struct ts6BindingEntry
{
    ttNode              bind6Node;
    tt6IpAddress        bind6HomeAddr;       /* home address */
    tt6IpAddress        bind6CoAddr;         /* care of address */
    tt32Bit             bind6Lifetime;       /* life time in msecs */
    tt32Bit             bind6LastSendTime;   /* msecs, for replace policy */
    tt32Bit             bind6LastRecvTime;   /* msecs, for replace policy */
    tt32Bit             bind6LastUpdatedTime;/* msecs, time last updated*/
    tt16Bit             bind6LastNonceIndex;
    tt16Bit             bind6LastSeq;        /* last seq received */
/* dest unreach ICMPs recved since bind6LastSendTime */
    tt32Bit             bind6IcmpErrRecved;
#ifdef TM_6_USE_MIP_HA  
    tt16Bit             bind6HomeReg ;      /* for HA only */
    tt16Bit             bind6ProxyMHome;    /* the mHome of the proxy addr */
    ttDeviceEntryPtr    bind6TunnelDevPtr;  /* virtual tunnel device */
#endif /* TM_6_USE_MIP_HA */     
} tt6BindingEntry;
typedef tt6BindingEntry    TM_FAR * tt6BindingEntryPtr;
typedef tt6BindingEntryPtr TM_FAR * tt6BindingEntryPtrPtr;
#endif /* TM_6_USE_MIP_CN */


#if (defined(TM_6_USE_MIP_MN) || defined(TM_6_USE_MIP_CN))

/* Network header: IPv6 Mobility Header */
typedef struct ts6MipHeader
{
    tt8Bit      mh6PayloadProto; /* payload proto */
    tt8Bit      mh6HeaderLen;    /* header length */
    tt8Bit      mh6Type;         /* MH type */
    tt8Bit      mh6Reserved;
} tt6MipHeader;
typedef tt6MipHeader TM_FAR * tt6MipHeaderPtr;

/* Network header: IPv6 Mobility Header */
typedef struct ts6MipMsg
{
/* The common mobility header */
    tt6MipHeader        mm6Mh;    
    tt16Bit             mm6Checksum;
    tt8Bit              padding[2];
} tt6MipMsg;
typedef tt6MipMsg TM_FAR * tt6MipMsgPtr;


/* Network header: IPv6 Mobility Header Binding Refresh Request */
typedef struct ts6MipBrrMsg
{
/* The common mobility header */
    tt6MipHeader        brr6Mh;    
    tt16Bit             brr6Checksum;
/*
 * 16-bit field reserved for future use.  The value MUST be
 * initialized to zero by the sender, and MUST be ignored by the receiver
 */
    tt16Bit             brr6Reserved;
} tt6MipBrrMsg;
typedef tt6MipBrrMsg TM_FAR * tt6MipBrrMsgPtr;

/* Network header: IPv6 Mobility Header Home Test Init */
typedef struct ts6MipHotiMsg
{
/* The common mobility header */
    tt6MipHeader        hoti6Mh;    
    tt16Bit             hoti6Checksum;
/*
 * 16-bit field reserved for future use.  The value MUST be
 * initialized to zero by the sender, and MUST be ignored by the receiver
 */
    tt16Bit             hoti6Reserved;
/* 64-bit field which contains a random value, the home init cookie.*/
    tt8Bit              hoti6HoCookie[8];
} tt6MipHotiMsg;
typedef tt6MipHotiMsg TM_FAR * tt6MipHotiMsgPtr;

/* Network header: IPv6 Mobility Header Care-of Test Init */
typedef struct ts6MipCotiMsg
{
/* The common mobility header */
    tt6MipHeader        coti6Mh;    
    tt16Bit             coti6Checksum;
/*
 * 16-bit field reserved for future use.  The value MUST be
 * initialized to zero by the sender, and MUST be ignored by the receiver
 */
    tt16Bit             coti6Reserved;
/* 64-bit field which contains a random value, the care-of init cookie */
    tt8Bit              coti6CoCookie[8];
} tt6MipCotiMsg;
typedef tt6MipCotiMsg TM_FAR * tt6MipCotiMsgPtr;

/* Network header: IPv6 Mobility Header Home Test */
typedef struct ts6MipHotMsg
{
/* The common mobility header */
    tt6MipHeader        hot6Mh;    
    tt16Bit             hot6Checksum;
/*
 * This field will be echoed back by the mobile node to the
 * correspondent node in a subsequent Binding Update.
 */
    tt16Bit             hot6HoNonceIndex;
/* 64-bit field which contains the home init cookie.*/
    tt8Bit              hot6HoCookie[8];
/*
 * This field contains the 64 bit home keygen token used in the
 * return routability procedure.
 */
    tt8Bit              hot6HoKeygen[8];
} tt6MipHotMsg;
typedef tt6MipHotMsg TM_FAR * tt6MipHotMsgPtr;

/* Network header: IPv6 Mobility Header Care-of Test */
typedef struct ts6MipCotMsg
{
/* The common mobility header */
    tt6MipHeader        cot6Mh;    
    tt16Bit             cot6Checksum;
/*
 * This value will be echoed back by the mobile node to the
 * correspondent node in a subsequent Binding Update.
 */
    tt16Bit             cot6CoNonceIndex;
/* 64-bit field which contains the care-of init cookie.*/
    tt8Bit              cot6CoCookie[8];
/*
 * This field contains the 64 bit care-of keygen token used in the
 * return routability procedure.
 */
    tt8Bit              cot6CoKeygen[8];
} tt6MipCotMsg;
typedef tt6MipCotMsg TM_FAR * tt6MipCotMsgPtr;

/* Network header: IPv6 Mobility Header Binding Update */
typedef struct ts6MipBuMsg
{
/* The common mobility header */
    tt6MipHeader        bu6Mh;    
    tt16Bit             bu6Checksum;
/*
 * A 16-bit number used by the receiving node to sequence Binding
 * Updates and by the sending node to match a returned Binding
 * Acknowledgement with this Binding Update.
 */
    tt16Bit             bu6Seq;
/* flags, one of AHLK */
    tt8Bit              bu6Flags;
/*
 * These fields are unused.  They MUST be initialized to zero by
 * the sender and MUST be ignored by the receiver.
 */
    tt8Bit              bu6Reserved;
/* 
 * 16-bit unsigned integer.  The number of time units remaining
 * before the binding MUST be considered expired.A value of zero
 * indicates that the Binding Cache entry for the mobile node MUST
 * be deleted.  One time unit is 4 seconds.
 */
    tt16Bit             bu6Lifetime;
} tt6MipBuMsg;
typedef tt6MipBuMsg TM_FAR * tt6MipBuMsgPtr;

/* Network header: IPv6 Mobility Header Binding Acknowledgement */
typedef struct ts6MipBaMsg
{
/* The common mobility header */
    tt6MipHeader        ba6Mh;    
    tt16Bit             ba6Checksum;
/*
 * 8-bit unsigned integer indicating the disposition of the
 * Binding Update.  Values of the Status field less than 128
 * indicate that the Binding Update was accepted by the receiving
 * node.  Values greater than or equal to 128 indicate that
 * the Binding Update was rejected by the receiving node.
 */
    tt8Bit              ba6Status;
/*
 * These fields are unused.  They MUST be initialized to zero by
 * the sender and MUST be ignored by the receiver.
 */
/* flags, K bit flag, this includes the 7 bytes reserverd as  well. */
    tt8Bit              ba6Flags;
/*
 * The Sequence Number in the Binding Acknowledgement is
 * copied from the Sequence Number field in the Binding Update.
 * It is used by the mobile node in matching this Binding
 * Acknowledgement with an outstanding Binding Update.
 */
    tt16Bit             ba6Seq; 
/*
 * The granted lifetime, in time units of 4 seconds, for which
 * this node SHOULD retain the entry for this mobile node in its
 * Binding Cache.  A value of all one bits (0xffff) indicates
 * infinity.
 * 
 * The value of this field is undefined if the Status field
 * indicates that the Binding Update was rejected.
 */
    tt16Bit             ba6Lifetime;
} tt6MipBaMsg;
typedef tt6MipBaMsg TM_FAR * tt6MipBaMsgPtr;

/* Network header: IPv6 Mobility Header Binding Error Message */
typedef struct ts6MipBeMsg
{
/* The common mobility header */
    tt6MipHeader        be6Mh;    
    tt16Bit             be6Checksum;
    tt8Bit              be6Status;
    tt8Bit              be6Reserved;
    tt6IpAddrArr        be6HomeAddr;
} tt6MipBeMsg;
typedef tt6MipBeMsg TM_FAR * tt6MipBeMsgPtr;


/* Network header: IPv6 Mobility Option */
typedef struct ts6MipOption
{
    tt8Bit              mo6Type;
    tt8Bit              mo6Len;
    tt8Bit              mo6Data[6];
} tt6MipOption;
typedef tt6MipOption TM_FAR * tt6MipOptionPtr;

/* Network header: IPv6 Mobility Option Alternate Care-of Address */
typedef struct ts6MipOptionAltCoa
{
    tt8Bit              mo6Type;
    tt8Bit              mo6Len;
    tt8Bit              mo6AltCoa[TM_6_IP_ADDRESS_LENGTH];
    tt8Bit              padding[2];
} tt6MipOptionAltCoa;
typedef tt6MipOptionAltCoa TM_FAR * tt6MipOptionAltCoaPtr;

/* Network header: IPv6 Mobility Option Nonce Indices */
typedef struct ts6MipOptionNonceIndex
{
    tt8Bit              mo6Type;
    tt8Bit              mo6Len;
    tt16Bit             mo6HomeNonceIndex;
    tt16Bit             mo6CoNonceIndex;
    tt8Bit              padding[2];
} tt6MipOptionNonceIndex;
typedef tt6MipOptionNonceIndex TM_FAR * tt6MipOptionNonceIndexPtr;

/* Network header: IPv6 Mobility Option Binding Authorization Data */
typedef struct ts6MipOptionAuthData
{
    tt8Bit              mo6Type;
    tt8Bit              mo6Len;
    tt8Bit              mo6Auth[12];
} tt6MipOptionAuthData;
typedef tt6MipOptionAuthData TM_FAR * tt6MipOptionAuthDataPtr;

/* Network header: IPv6 Mobility Option Binding Refresh Advice */
typedef struct ts6MipOptionRefAdvice
{
    tt8Bit              mo6Type;
    tt8Bit              mo6Len;
    tt16Bit             mo6RefreshInterval;
} tt6MipOptionRefAdvice;
typedef tt6MipOptionRefAdvice TM_FAR * tt6MipOptionRefAdvicePtr;

typedef void (* ttPacketFuncPtr)(ttPacketPtr packetPtr);

#endif /* TM_6_USE_MIP_MN || TM_6_USE_MIP_CN */

#ifdef TM_6_USE_ROUTING_HEADER

/* Network header: IPv6 Type 2 Routing Extension Header */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#ifndef TM_32BIT_DSP_BIG_ENDIAN
typedef struct ts6RoutingType2ExtHdr
{
/* Number of route segments remaining, 1 */
    unsigned int        rth6SegsLeft : 8;
/* Specifies the specific type of routing extension header. 2. */
    unsigned int        rth6Type     : 8;
/* Length of this extension header in 8-byte units, not counting the first 8
   bytes. */
    unsigned int        rth6HdrLen   : 8;
/* The protocol ID of the next IPv6 extension header following this one, or
   IPPROTO_NONE if the ULP payload is immediately following.
   ([RFC2460].R4:10) */
    unsigned int        rth6NextHdr  : 8;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt32Bit             rth6Reserved;
/* The Home Address of the destination Mobile Node. */
    tt6IpAddrArr        rth6HomeAddr;
} tt6RoutingType2ExtHdr;
#else /* TM_32BIT_DSP_BIG_ENDIAN */
typedef struct ts6RoutingType2ExtHdr
{
/* The protocol ID of the next IPv6 extension header following this one, or
   IPPROTO_NONE if the ULP payload is immediately following.
   ([RFC2460].R4:10) */
    unsigned int        rth6NextHdr  : 8;
/* Length of this extension header in 8-byte units, not counting the first 8
   bytes. */
    unsigned int        rth6HdrLen   : 8;
/* Specifies the specific type of routing extension header. 2. */
    unsigned int        rth6Type     : 8;
/* Number of route segments remaining, 1 */
    unsigned int        rth6SegsLeft : 8;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt32Bit             rth6Reserved;
/* The Home Address of the destination Mobile Node. */
    tt6IpAddrArr        rth6HomeAddr;
} tt6RoutingType2ExtHdr;
#endif /* TM_32BIT_DSP_BIG_ENDIAN */
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
typedef struct ts6RoutingType2ExtHdr
{
/* The protocol ID of the next IPv6 extension header following this one, or
   IPPROTO_NONE if the ULP payload is immediately following.
   ([RFC2460].R4:10) */
    unsigned int        rth6NextHdr  : 8;
/* Length of this extension header in 8-byte units, not counting the first 8
   bytes. */
    unsigned int        rth6HdrLen   : 8;
/* Specifies the specific type of routing extension header. 2. */
    unsigned int        rth6Type     : 8;
/* Number of route segments remaining, 1 */
    unsigned int        rth6SegsLeft : 8;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt32Bit             rth6Reserved;
/* The Home Address of the destination Mobile Node. */
    tt6IpAddrArr        rth6HomeAddr;
} tt6RoutingType2ExtHdr;
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
typedef struct ts6RoutingType2ExtHdr
{
/* The protocol ID of the next IPv6 extension header following this one, or
   IPPROTO_NONE if the ULP payload is immediately following.
   ([RFC2460].R4:10) */
    tt8Bit              rth6NextHdr;
/* Length of this extension header in 8-byte units, not counting the first 8
   bytes. */
    tt8Bit              rth6HdrLen;
/* Specifies the specific type of routing extension header. 2. */
    tt8Bit              rth6Type;
/* Number of route segments remaining, 1 */
    tt8Bit              rth6SegsLeft;
/* Reserved. Set to 0 on transmit, ignored on receive. */
    tt32Bit             rth6Reserved;
/* The Home Address of the destination Mobile Node. */
    tt6IpAddrArr        rth6HomeAddr;
} tt6RoutingType2ExtHdr;
#endif /* !TM_DSP */
typedef tt6RoutingType2ExtHdr TM_FAR * tt6RoutingType2ExtHdrPtr;

#endif /* TM_6_USE_ROUTING_HEADER */

#endif /* TM_USE_IPV6 */

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _T6TYPE_H_ */
