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
 * Description: DHCP Client protocol
 *
 * Filename: trdhcp.c
 * Author: Odile
 * Date Created: 06/25/98
 * $Source: source/trdhcp.c $
 *
 * Modification History
 * $Revision: 6.0.2.14 $
 * $Date: 2012/03/06 10:09:05JST $
 * $Author: lfox $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>

#if defined(TM_USE_IPV4) && defined(TM_USE_DHCP)

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/* Define unreferenced macro for PC-LINT compile error suppression */
#ifdef TM_LINT
LINT_UNREF_MACRO(TM_DHCP_OVERLOAD_BOTH)
LINT_UNREF_MACRO(TM_DHCP_ERR_MSG)
LINT_UNREF_MACRO(TM_DHCP_MAX_MSG_SIZE)
LINT_UNREF_MACRO(TM_DHCP_MAXMSGSIZE)
LINT_UNREF_MACRO(TM_DHCPINFORM_END)
LINT_UNREF_MACRO(TM_DHCPINFORM)
LINT_UNREF_MACRO(TM_DHCPE_R_OFFER)
LINT_UNREF_MACRO(TM_DHCPE_R_ACK)
LINT_UNREF_MACRO(TM_DHCP_MSG_LENGTH)
LINT_UNREF_MACRO(TM_DHCPDISCOVER)
LINT_UNREF_MACRO(TM_DHCPREQUEST)
LINT_UNREF_MACRO(TM_DHCPRELEASE)
LINT_UNREF_MACRO(TM_DHCPDECLINE)
#endif /* TM_LINT */

/*
 *  DHCP header (from RFC 1542, modified for DHCP)
 *
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     op (1)    |   htype (1)   |   hlen (1)    |   hops (1)    |
 * +---------------+---------------+---------------+---------------+
 * |                            xid (4)                            |
 * +-------------------------------+-------------------------------+
 * |           secs (2)            |           flags (2)           |
 * +-------------------------------+-------------------------------+
 * |                           ciaddr (4)                          |
 * +---------------------------------------------------------------+
 * |                           yiaddr (4)                          |
 * +---------------------------------------------------------------+
 * |                           siaddr (4)                          |
 * +---------------------------------------------------------------+
 * |                           giaddr (4)                          |
 * +---------------------------------------------------------------+
 * |                                                               |
 * |                           chaddr (16)                         |
 * |                                                               |
 * |                                                               |
 * +---------------------------------------------------------------+
 * |                                                               |
 * |                           sname  (64)                         |
 * +---------------------------------------------------------------+
 * |                                                               |
 * |                           file   (128)                        |
 * +---------------------------------------------------------------+
 * |                                                               |
 * |                           options(312)                        |
 * +---------------------------------------------------------------+
 */

/*
 * Local macros, and local const variables.
 */

/* 
 * Number of seconds to wait until trying again a DHCP discover after
 * declining an IP address (because of conflict)
 */
#ifndef TM_DHCP_RESTART_DELAY
#define TM_DHCP_RESTART_DELAY  TM_UL(10000)
#endif /* TM_DHCP_RESTART_DELAY */

/* Flags for tfDhcpInit() */
#define TM_DHCP_START       0
#define TM_DHCP_RESTART     1
#define TM_DHCP_NEWOFFER    2

/* Maximum lease to allow is slightly over 49 days */
#define TM_DHCP_MAX_LEASE_TIME_SECONDS TM_UL(4294967) /* 49.7 days */
#define TM_DHCP_MIN_LEASE_TIME_SECONDS TM_UL(8)       /* 8 seconds */

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *    12      host name     variable       host name
 */
#define TM_DHCP_HOST_NAME   (tt8Bit)12         /* 0x0C */

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *    50      Address Request   4          Requested IP Address
 */
#define TM_DHCP_IP_ADDRESS (tt8Bit)50            /* 0x32 */
/* Requested IP address option */
static const tt8Bit  TM_CONST_QLF tlDhcpRequestedIp[2] = 
            { TM_DHCP_IP_ADDRESS, TM_IP_ADDRESS_LENGTH };

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *    51      Address Time      4          IP Address Lease Time
 */
#define TM_DHCP_LEASE_TIME (tt8Bit)51

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *    52      Overload          1          Overload "sname" or "file"
 */
#define TM_DHCP_OVERLOAD   (tt8Bit)52

#define TM_DHCP_OVERLOAD_FILE   1
#define TM_DHCP_OVERLOAD_SNAME  2
#define TM_DHCP_OVERLOAD_BOTH   (TM_DHCP_OVERLOAD_FILE|TM_DHCP_OVERLOAD_SNAME)

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *    54      DHCP Server Id    4          DHCP Server Identification
 */
#define TM_DHCP_SERVER_ID (tt8Bit)54           /* 0x36 */
/* Server IP address option */
static const tt8Bit  TM_CONST_QLF tlDhcpServerId[2] = 
                        {TM_DHCP_SERVER_ID, TM_IP_ADDRESS_LENGTH };

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *    55      Parameter List    N          Parameter Request List
 */
#define TM_DHCPGET_PARAMETER_LIST (tt8Bit)55  /* 0x37 */

/* 
 * Request DNS servers (6), and Default router (3) 
 * Request subnet mask (1), domain name (15 == 0xF), TFTP server name
 * (66 == 0x42) when TFTP server name field has been overloaded,
 * boot file name (67 == 0x43) when boot file name field has been overloaded.
 * Request boot file size (13 == 0xD), Request NetBios Name Server
 * (44 == 0x2C), Request host name (12 == 0x0C)
 */
#define TM_DHCP_PARAM_DNS_SERVER      (tt8Bit)6
#define TM_DHCP_PARAM_DEF_ROUTER      (tt8Bit)3
#define TM_DHCP_PARAM_SUBNET_MASK     (tt8Bit)1
#define TM_DHCP_PARAM_DOMAIN_NAME     (tt8Bit)15
#define TM_DHCP_PARAM_TFTP_SERVER     (tt8Bit)66
#define TM_DHCP_PARAM_BOOT_FILE_NAM   (tt8Bit)67
#define TM_DHCP_PARAM_BOOT_FILE_SIZ   (tt8Bit)13
#define TM_DHCP_PARAM_NBNS            (tt8Bit)44
#define TM_DHCP_PARAM_HOST_NAME       (tt8Bit)12

/* Number of items requested in the list (sizeof(tlDhcpParameterList)-2)) */
#define TM_DHCP_PARAM_LIST_SIZE       (tt8Bit)9

static const tt8Bit  TM_CONST_QLF tlDhcpParameterList[] = 
{ TM_DHCPGET_PARAMETER_LIST,   TM_DHCP_PARAM_LIST_SIZE, 
  TM_DHCP_PARAM_DNS_SERVER,    TM_DHCP_PARAM_DEF_ROUTER, 
  TM_DHCP_PARAM_SUBNET_MASK,   TM_DHCP_PARAM_DOMAIN_NAME,
  TM_DHCP_PARAM_TFTP_SERVER,   TM_DHCP_PARAM_BOOT_FILE_NAM,
  TM_DHCP_PARAM_BOOT_FILE_SIZ, TM_DHCP_PARAM_NBNS,
  TM_DHCP_PARAM_HOST_NAME};

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *    56      DHCP Message      N          DHCP Error Message
 */
#define TM_DHCP_ERR_MSG (tt8Bit)56

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *    57      DHCP Max Msg Size 2          DHCP Maximum Message Size
 */
#define TM_DHCP_MAX_MSG_SIZE (tt8Bit)57
#define TM_DHCP_MAXMSGSIZE   tm_const_htons(0x5702)

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *    58      Renewal Time      4          IP Address Renewal Time (T1)
 */

#define TM_DHCP_RENEWAL_TIME (tt8Bit)58

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *    59      Rebinding Time    4          IP Address Rebinding Time (T2)
 */
#define TM_DHCP_REBINDING_TIME (tt8Bit)59

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *    61      Client            n          Uniquely Identifies the client on
 *            Identifier                   the server
 */
#define TM_DHCP_CLIENT_ID (tt8Bit)61

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *    66      TFTP server       n          TFTP server name when 'sname'
 *            name                         has been overloaded
 */
#define TM_DHCP_TFTPS_NAME (tt8Bit)66

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *    67      BOOT file        n           Boot file name when 'file'
 *            name                         has been overloaded
 */
#define TM_DHCP_BOOTF_NAME (tt8Bit)67

/*
 * DHCP messages
 */

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *    53      DHCP Msg Type     1          DHCP Message Type
 */
#define TM_DHCP_MSG_TYPE   (tt8Bit)53     /* 0x35 */
/*
 * RFC 2132: 9.6. DHCP Message Type
 *   This option is used to convey the type of the DHCP message.  The code
 *  for this option is 53, and its length is 1.  Legal values for this
 *   option are:
 *
 *           Value   Message Type
 *          -----   ------------
 *            1     DHCPDISCOVER
 *            2     DHCPOFFER
 *            3     DHCPREQUEST
 *            4     DHCPDECLINE
 *            5     DHCPACK
 *            6     DHCPNAK
 *            7     DHCPRELEASE
 *            8     DHCPINFORM
 *
 *   Code   Len  Type
 *  +-----+-----+-----+
 *  |  53 |  1  | 1-9 |
 *  +-----+-----+-----+
 */

/*
 * DHCP message types (one byte)
 */
#define TM_DHCP_MSG_LENGTH    (tt8Bit)1

#define TM_DHCPDISCOVER       (tt8Bit)1
#define TM_DHCPOFFER          (tt8Bit)2
#define TM_DHCPREQUEST        (tt8Bit)3
#define TM_DHCPDECLINE        (tt8Bit)4
#define TM_DHCPACK            (tt8Bit)5
#define TM_DHCPNAK            (tt8Bit)6
#define TM_DHCPRELEASE        (tt8Bit)7
#define TM_DHCPINFORM         (tt8Bit)8

/*
 * DHCP Message including code, length, type, and End tag.
 * We add the END tag (255), to make it a 4 bytes, so that we can quickly
 * store a 4-byte constant in one instruction as opposed to copy bytes.
 */
#define TM_DHCPDISCOVER_END   tm_const_htonl(TM_UL(0x350101FF))
#define TM_DHCPREQUEST_END    tm_const_htonl(TM_UL(0x350103FF))
#define TM_DHCPDECLINE_END    tm_const_htonl(TM_UL(0x350104FF))
#define TM_DHCPRELEASE_END    tm_const_htonl(TM_UL(0x350107FF))
#define TM_DHCPINFORM_END     tm_const_htonl(TM_UL(0x350108FF))


/* DHCP FQDN option
 *
 *      Code   Len    Flags  RCODE1 RCODE2   Domain Name
 *     +------+------+------+------+------+------+--
 *     |  81  |   n  |      |      |      |       ...
 *     +------+------+------+------+------+------+--
 *
 *     Flags:
 *
 *      0 1 2 3 4 5 6 7
 *     +-+-+-+-+-+-+-+-+
 *     |  MBZ  |N|E|O|S|
 *     +-+-+-+-+-+-+-+-+
 *
 */
/* FQDN Option flag Bits: */
#define TM_DHCP_FQDN_FLAG_S     0x01
#define TM_DHCP_FQDN_FLAG_O     0x02
#define TM_DHCP_FQDN_FLAG_E     0x04
#define TM_DHCP_FQDN_FLAG_N     0x08
/* FQDN header offsets: */
#define TM_FQDN_RCODE1_OFFSET      0x01
#define TM_FQDN_RCODE2_OFFSET      0x02
#define TM_FQDN_DOMAINNAME_OFFSET  0x03

#define TM_FQDN_OPTION_HDR_LEN  6


/* The NetBIOS name server (NBNS) option
 *
 * RFC2132: NetBIOS over TCP/IP Name Server Option
 *
 * The NetBIOS name server (NBNS) option specifies a list of RFC
 * 1001/1002 [19] [20] NBNS name servers listed in order of preference.
 *
 * The code for this option is 44.  The minimum length of the option is
 * 4 octets, and the length must always be a multiple of 4.
 *
 *  Code   Len           Address 1              Address 2
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+----
 * |  44 |  n  |  a1 |  a2 |  a3 |  a4 |  b1 |  b2 |  b3 |  b4 | ...
 * +-----+-----+-----+-----+-----+-----+-----+-----+-----+-----+----
 *
 */
#define TM_DHCP_NBNS        (tt8Bit)44   /*  0x2C */


/*
 * Minimum amount of time left in T1, or T2 above which we are allowed to
 * retransmit. (60 seconds)
 */
#define TM_DHCPT_MIN_TX     TM_UL(60000)

/* Events in the DHCP client state machine */
/* User request a configuration */
#define TM_DHCPE_START         0
/* Receive an offer from the server */
#define TM_DHCPE_R_OFFER       1  /* Has to be 1 to match DHCPOFFER - 1 */
/* Retry timer */
#define TM_DHCPE_RETRY         2
/* Retry timeout */
#define TM_DHCPE_RTIMEOUT      3
/* Receive an ACK from the server */
#define TM_DHCPE_R_ACK         4 /* Has to be 4 to match DHCPACK - 1 */
/* Receive an NAK from the server */
#define TM_DHCPE_R_NAK         5  /* Has to be 5 to match DHCPNAK - 1 */

/* State machine no op index (no action) */
#define TM_MATRIX_NOOP            TM_DHCPA_LAST_INDEX
/* State machine Discard receive packet index (no action) */
#define TM_R_DISCARD              TM_MATRIX_NOOP

/* State machine error index (action is to set the corresponding error) */
#define TM_DHCPA_EALREADY         tm_dhcpa_err(TM_EALREADY)

/*
 * system error to state machine error index. Use TM_DHCPA_LAST_INDEX as base
 * of error
 */
#define tm_dhcpa_err(errorCode)  (tt8Bit)((int)((int)(errorCode-TM_ERR_BASE) \
                                                       + TM_DHCPA_LAST_INDEX))
/* State machine error index to system error */
#define tm_dhcpa_sys_err(index)  ((((int)(index))-TM_DHCPA_LAST_INDEX) \
                                                   + TM_ERR_BASE)



/* Macro to call a state function */
#define tm_call_state_function(stateEntryPtr, btEntryPtr, packetPtr) \
    (*(stateEntryPtr->dstaFunctPtr))(btEntryPtr, packetPtr)

/* Macro to declare a state function */
#define tm_state_function(dhcpStateFunction, btEntryPtr, packetPtr) \
    dhcpStateFunction(ttBtEntryPtr btEntryPtr, ttPacketPtr packetPtr)

/*
 * DSP accessor macros
 */
#ifdef TM_32BIT_DSP
#define TM_DSP_SHIFT 2
#endif /* TM_32BIT_DSP */

#ifdef TM_16BIT_DSP
#define TM_DSP_SHIFT 1
#endif /* TM_16BIT_DSP */

/* Get an 8-bit value at a particular byte offset */
#define tm_dsp_get_8bit(dataPtr, byteOffset) \
        tm_dsp_get_byte_ptr(dataPtr, byteOffset)

/* Increments a offset, ensuring that the offset is less than the word length */
#define tm_dsp_inc_byte_offset(byteOffset, incVal) \
    byteOffset = (byteOffset + incVal) % TM_DSP_BYTES_PER_WORD

#define tm_dsp_put_8bit(buffer,index,dataChar) \
{ \
    buffer[tm_packed_byte_count((index)+1)-1] &= \
        (~(0xff << ((TM_DSP_ROUND_PTR - \
        ((index) % TM_DSP_BYTES_PER_WORD)) << 3))); \
    buffer[tm_packed_byte_count((index)+1)-1] |= \
        (dataChar << ((TM_DSP_ROUND_PTR - \
        ((index) % TM_DSP_BYTES_PER_WORD)) << 3)); \
}

/*
 * Move the pointer to the next BOOTP option.
 */
#ifdef TM_DSP
#define tm_dhcp_next_option(optPtr, index, byteOffset) \
    optPtr += (index + byteOffset) >> (TM_DSP_BYTES_PER_WORD >> 1); \
    byteOffset  = (index + byteOffset) % TM_DSP_BYTES_PER_WORD
#else /* !TM_DSP */
#define tm_dhcp_next_option(optPtr, index, byteOffset) \
    (optPtr) += (index)
#endif /* TM_DSP */

/*
 * Get BOOTP option length
 */
#ifdef TM_DSP
#define tm_dhcp_get_opt_len(optPtr, index, byteOffset) \
tm_dsp_get_8bit((optPtr), ((index) + (byteOffset)))
#else /* !TM_DSP */
#define tm_dhcp_get_opt_len(optPtr, index, byteOffset) \
    *( (optPtr) + (index) )
#endif /* TM_DSP */


#ifdef TM_DSP
#define tm_dhcp_short_copy(srcPtr, dest, srcOffset, bytePos)             \
{ \
    dest = tm_dsp_get_8bit((srcPtr), ((srcOffset) + (bytePos))    ) << 8; \
    dest = tm_dsp_get_8bit((srcPtr), ((srcOffset) + (bytePos)) + 1);      \
}
#else /* !TM_DSP */
#define tm_dhcp_short_copy(srcPtr, dest, srcOffset, bytePos)          \
{ \
    ((tt8BitPtr)(&dest))[0] = (srcPtr)[srcOffset];                     \
    ((tt8BitPtr)(&dest))[1] = (srcPtr)[srcOffset+1];                   \
}
#endif /* TM_DSP */


#ifdef TM_DSP
#define tm_dhcp_long_copy(srcPtr, dest, srcOffset, bytePos) \
{ \
    dest  = (tt32Bit) \
            tm_dsp_get_8bit((srcPtr), ((srcOffset) + (bytePos))     ) << 24; \
    dest |= (tt32Bit) \
            tm_dsp_get_8bit((srcPtr), ((srcOffset) + (bytePos)) + 1 ) << 16; \
    dest |= (tt32Bit) \
            tm_dsp_get_8bit((srcPtr), ((srcOffset) + (bytePos)) + 2 ) << 8;  \
    dest |= (tt32Bit) \
            tm_dsp_get_8bit((srcPtr), ((srcOffset) + (bytePos)) + 3 );       \
}
#else /* !TM_DSP */
#define tm_dhcp_long_copy(srcPtr, dest, srcOffset, bytePos)           \
{ \
    ((tt8BitPtr)(&dest))[0] = (srcPtr)[srcOffset];                     \
    ((tt8BitPtr)(&dest))[1] = (srcPtr)[srcOffset+1];                   \
    ((tt8BitPtr)(&dest))[2] = (srcPtr)[srcOffset+2];                   \
    ((tt8BitPtr)(&dest))[3] = (srcPtr)[srcOffset+3];                   \
}
#endif /* TM_DSP */


#ifdef TM_DSP
#define tm_dhcp_get_char(srcPtr, srcOffset, bytePos) \
    tm_dsp_get_8bit((srcPtr), ((srcOffset) + (bytePos)))
#else /* !TM_DSP */
#define tm_dhcp_get_char(srcPtr, srcOffset, bytePos) \
    (srcPtr)[(srcOffset)]
#endif /* TM_DSP */

#ifdef TM_DSP
#define tm_dhcp_set_char(srcChar, destPtr, destOffset, bytePos) \
    tm_dsp_put_8bit((destPtr), ((destOffset)+(bytePos)), (srcChar))
#else /* !TM_DSP */
#define tm_dhcp_set_char(srcChar, destPtr, destOffset, bytePos) \
    (destPtr)[(destOffset)] = (srcChar)
#endif /* TM_DSP */

#ifdef TM_DSP
#define tm_dhcp_byte_copy(srcPtr, dest, srcOffset, length, bytePos) \
    tfMemCopyOffset( (int*) (srcPtr), (bytePos) + (srcOffset),      \
                     (int *) (&(dest)), 0, (length))
#else /* TM_DSP */
#define tm_dhcp_byte_copy(srcPtr, dest, srcOffset, length, bytePos) \
    tm_bcopy( &((srcPtr)[srcOffset]), (&(dest)), length )
#endif /* TM_DSP */

#ifdef TM_DSP
/* Copies data from an aligned buffer into a packet, with a possible offset */
#define tm_dhcp_byte_copy_to_pkt(srcPtr, destPtr, length, bytePos) \
    tfMemCopyOffset( (int*) srcPtr, 0, (int *) destPtr, bytePos, length)
#else /* TM_DSP */
#define tm_dhcp_byte_copy_to_pkt(srcPtr, destPtr, length, bytePos) \
    tm_bcopy( srcPtr, destPtr, length)
#endif /* TM_DSP */


#ifdef TM_DSP
#define tm_dhcp_copy_list_to_pkt(listPtr, destPtr, length, bytePos, tempCount)  \
{                                                                               \
    for (tempCount=0;tempCount<(length);tempCount++)                            \
    {                                                                           \
        tm_dsp_put_8bit((destPtr),                                              \
                        (tempCount)+(bytePos),                                  \
                        ((listPtr)[tempCount] & 0xff));                         \
    }                                                                           \
}
#else /* TM_DSP */
#define tm_dhcp_copy_list_to_pkt(listPtr, destPtr, length, bytePos, tempCount) \
    tm_bcopy( (listPtr), (destPtr), length )
#endif /* TM_DSP */

/*
 * Local types
 */

/* Transition matrix Entry for DHCP event/state processing */
typedef struct tsTransitionMatrixEntry {
    tt8Bit mtxTransition;
    tt8Bit mtxFunctionIndex;
} ttTransitionMatrixEntry;

/* Pointer type to a transition matrix */
typedef ttTransitionMatrixEntry TM_CONST_QLF * ttTransitionMatrixEntryPtr;

/* state function type */
typedef int (*ttDhcpStateFunctPtr)(ttBtEntryPtr       btEntryPtr,
                                   ttPacketPtr        packetPtr);

/* State function table entry */
#ifdef TM_LINT
LINT_UNREF_MEMBER_BEGIN
#endif /* TM_LINT */

typedef struct tsDhcpStateFunctEntry {
    tt8Bit              dstaIndex;
    tt8Bit              dstaFiller[3];
    ttDhcpStateFunctPtr dstaFunctPtr;
} ttDhcpStateFunctEntry;
#ifdef TM_LINT
LINT_UNREF_MEMBER_END
#endif /* TM_LINT */


/* pointer type to a state function table */
typedef ttDhcpStateFunctEntry TM_CONST_QLF * ttDhcpStateFunctEntryPtr;

/*
 * Local functions
 */

static int tfDhcpConfig(ttVoidPtr voidPtr, int flag, tt16Bit multiHomeIndex);
static void tfDhcpBtStop(ttBtEntryPtr btEntryPtr, int errorCode);

static void tfDhcpScanOptions( ttPacketPtr     packetPtr,
                               ttBtEntryPtr    btEntryPtr,
                               int             action );

TM_NEARCALL static void tfDhcpRetryTimer (ttVoidPtr      timerBlockPtr,
                                          ttGenericUnion userParm1,
                                          ttGenericUnion userParm2);

TM_NEARCALL static void tfDhcpLeaseTimer (ttVoidPtr      timerBlockPtr,
                                          ttGenericUnion userParm1,
                                          ttGenericUnion userParm2);

TM_NEARCALL static void tfDhcpRenewTimer (ttVoidPtr      timerBlockPtr,
                                          ttGenericUnion userParm1,
                                          ttGenericUnion userParm2);

/* Function used to send DHCP DISCOVER/REQUEST messages with a delay */
static void tfDhcpRestartTimer (ttVoidPtr      timerBlockPtr,
                                ttGenericUnion userParm1,
                                ttGenericUnion userParm2);

static int tfDhcpMsgSend( ttBtEntryPtr     btEntryPtr,
                          ttTmCBFuncPtr    timerFuncPtr,
                          tt32Bit          dhcpMsgEnd );

static void tfDhcpDecline(ttBtEntryPtr btEntryPtr);

static int tfDhcpFinish( ttUserInterface    interfaceHandle,
                         ttUserIpAddress    ipAddress,
                         int                errorCode,
                         ttUserGenericUnion dhcpParam );

static int tfDhcpInit (ttBtEntryPtr btEntryPtr, int restart);

static void tfDhcpRemoveTimers (ttBtEntryPtr btEntryPtr);

static void tfDhcpOfferCleanup(ttBtEntryPtr btEntryPtr);

/* 
 * Called by tfListWalk to copy an option in the custom options list to
 * the outgoing packet.
 */
TM_NEARCALL static int tfDhcpAddCustOpt(ttNodePtr      nodePtr,
                            ttGenericUnion genParam1);

static int tfDhcpStateMachine( ttBtEntryPtr   btEntryPtr,
                               ttPacketPtr    packetPtr,
/* event (to be used as index in tlDhcpTransitionMatrix[][] */
                               tt8Bit         event );

static int tm_state_function(tfDhcpaInit, btEntryPtr, packetPtr);
static int tm_state_function(tfDhcpaDelayStart, btEntryPtr, packetPtr);
static int tm_state_function(tfDhcpaDiscover, btEntryPtr, packetPtr);
static int tm_state_function(tfDhcpaRequest, btEntryPtr, packetPtr);
static int tm_state_function(tfDhcpaCollect, btEntryPtr, packetPtr);
static int tm_state_function(tfDhcpaConfigure, btEntryPtr, packetPtr);
static int tm_state_function(tfDhcpaNewLease, btEntryPtr, packetPtr);
static int tm_state_function(tfDhcpaLease, btEntryPtr, packetPtr);
static int tm_state_function(tfDhcpaHalt, btEntryPtr, packetPtr);
static int tm_state_function(tfDhcpaRestart, btEntryPtr, packetPtr);
static int tm_state_function(tfDhcpaRcvInvalid, btEntryPtr, packetPtr);
static int tm_state_function(tfDhcpaTimeout, btEntryPtr, packetPtr);

#ifdef TM_DEBUG_LOGGING
static ttCharPtr tfDhcpMsgStr(tt32Bit msgEnd);
#endif /* TM_DEBUG_LOGGING */

/*
 * Initialized local variables
 */

/*
 * The following two tables are used by the DHCP state machine function
 * tfDhcpStateMachine().
 *
 * tlDhcpTransitionMatrix[][]
 * The transition matrix corresponds to the DHCP state-transition diagram
 * for DHCP clients pictured in figure 5 of RFC 2131, with retry timer,
 * and retry timer timeout added to it, but without the T1 and T2 timers
 * expirations. The transitions and actions for T1 and T2 timer expirations
 * are handled directly in tfDhcpLeaseTimer().
 * Given a causing event, and a DHCP state, the transition matrix yields a
 * next state to transition to, and an action index to be used in the state
 * function table below to obtain a list of functions to be called.
 * tlDhcpStateFuncTable[], the state function table gives a mapping between
 * an action procedure index and the list of DHCP state functions that need
 * to be called to take that action. The state functions corresponding to
 * an action procedure index are called sequentially one at a time.
 */
static const ttDhcpStateFunctEntry  TM_CONST_QLF tlDhcpStateFunctTable[]=
{
#define TM_DHCPA_START                                  (tt8Bit)0
    { TM_DHCPA_START, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO},
      tfDhcpaInit                                               },/* 0 */
    { TM_DHCPA_START, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO},
      tfDhcpaDelayStart                                          },/* 1 */
#define TM_DHCPA_DISCOVER                                     (tt8Bit)2
    { TM_DHCPA_DISCOVER, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO},
      tfDhcpaDiscover                                           },/* 2 */
#define TM_DHCPA_COLLECT                                      (tt8Bit)3
    { TM_DHCPA_COLLECT, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO},
      tfDhcpaCollect                                             },/* 3 */
    { TM_DHCPA_COLLECT, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO},
      tfDhcpaRequest                                             },/* 4 */
#define TM_DHCPA_REQUEST                                      (tt8Bit)5
    { TM_DHCPA_REQUEST, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO},
      tfDhcpaRequest                                             },/* 5 */
#define TM_DHCPA_CONFIGURE                                    (tt8Bit)6
    { TM_DHCPA_CONFIGURE, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO},
      tfDhcpaConfigure                                           },/* 6 */
    { TM_DHCPA_CONFIGURE, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO},
      tfDhcpaNewLease                                            },/* 7 */
#define TM_DHCPA_RESTART                                      (tt8Bit)8
    { TM_DHCPA_RESTART, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO},
      tfDhcpaRestart                                             },/* 8 */
    { TM_DHCPA_RESTART, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO},
      tfDhcpaDelayStart                                           },/* 9 */
#define TM_DHCPA_LEASE                                        (tt8Bit)10
    { TM_DHCPA_LEASE, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO},
      tfDhcpaLease                                               },/* 10 */
#define TM_DHCPA_HALT                                         (tt8Bit)11
    { TM_DHCPA_HALT, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO},
      tfDhcpaHalt                                                },/* 11 */
#define TM_DHCPA_TIMEOUT                                      (tt8Bit)12
    { TM_DHCPA_TIMEOUT, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO},
      tfDhcpaTimeout                                             },/* 12 */
#define TM_R_INVALID                                          (tt8Bit)13
    { TM_R_INVALID, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO},
      tfDhcpaRcvInvalid                                          },/* 13 */
#define TM_DHCPA_LAST_INDEX                                   (tt8Bit)14
    { TM_DHCPA_LAST_INDEX, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO},
      (ttDhcpStateFunctPtr)0                                     } /* 14 */
};

#define TM_DHCPA_CONFIGURE_LEASE \
                                (tt8Bit)(TM_DHCPA_LAST_INDEX + 1)

/*
 * The transition matrix tlDhcpTransitionMatrix[][] below yields an action
 * procedure index, and a new DHCP state given a current DHCP state and a
 * causing event. tlDhcpTransitionMatrix[][] is described in more details
 * above. States, and Causing events are defined and described above.
 */
static const ttTransitionMatrixEntry  TM_CONST_QLF tlDhcpTransitionMatrix
                            [TM_DHCPS_REBINDING+1][TM_DHCPE_R_NAK+1]=
{
    {/* State == TM_DHCPS_INIT */
/*      Event                 Next State         Action Procedure Index */
    {/* TM_DHCPE_START   */ TM_DHCPS_INIT,        TM_DHCPA_START          },
    {/* TM_DHCPE_R_OFFER */ TM_DHCPS_INIT,        TM_R_INVALID            },
    {/* TM_DHCPE_RETRY   */ TM_DHCPS_INIT,        TM_MATRIX_NOOP          },
/* Delay start timer expires */
    {/* TM_DHCPE_RTIMEOUT*/ TM_DHCPS_SELECTING,   TM_DHCPA_DISCOVER       },
    {/* TM_DHCPE_R_ACK   */ TM_DHCPS_INIT,        TM_R_INVALID            },
    {/* TM_DHCPE_R_NAK   */ TM_DHCPS_INIT,        TM_R_INVALID            }
    } ,
    {/* State == TM_DHCPS_SELECTING */
/*      Event                 Next State         Action Procedure Index */
    {/* TM_DHCPE_START   */ TM_DHCPS_SELECTING,   TM_DHCPA_EALREADY       },
/* Transition to TM_DHCPS_REQUESTING occurs when client makes a selection */
    {/* TM_DHCPE_R_OFFER */ TM_DHCPS_SELECTING,   TM_DHCPA_COLLECT        },
    {/* TM_DHCPE_RETRY   */ TM_DHCPS_SELECTING,   TM_DHCPA_DISCOVER       },
    {/* TM_DHCPE_RTIMEOUT*/ TM_DHCPS_INIT,        TM_DHCPA_TIMEOUT        },
    {/* TM_DHCPE_R_ACK   */ TM_DHCPS_SELECTING,   TM_R_INVALID            },
    {/* TM_DHCPE_R_NAK   */ TM_DHCPS_SELECTING,   TM_R_INVALID            }
    } ,
    {/* State == TM_DHCPS_REQUESTING */
/*      Event                 Next State         Action Procedure Index */
    {/* TM_DHCPE_START   */ TM_DHCPS_REQUESTING,  TM_DHCPA_EALREADY       },
    {/* TM_DHCPE_R_OFFER */ TM_DHCPS_REQUESTING,  TM_R_DISCARD            },
    {/* TM_DHCPE_RETRY   */ TM_DHCPS_REQUESTING,  TM_DHCPA_REQUEST        },
    {/* TM_DHCPE_RTIMEOUT*/ TM_DHCPS_INIT,        TM_DHCPA_TIMEOUT        },
/* record lease, set T1, T2 timers, cancel retry timer + configure */
    {/* TM_DHCPE_R_ACK   */ TM_DHCPS_BOUND,       TM_DHCPA_CONFIGURE      },
/* Discard offer, restart */
    {/* TM_DHCPE_R_NAK   */ TM_DHCPS_INIT,        TM_DHCPA_RESTART        }
    } ,
    {/* State == TM_DHCPS_INITREBOOT */
/*      Event                 Next State         Action Procedure Index */
    {/* TM_DHCPE_START   */ TM_DHCPS_INITREBOOT,  TM_DHCPA_START          },
    {/* TM_DHCPE_R_OFFER */ TM_DHCPS_INITREBOOT,  TM_R_INVALID            },
    {/* TM_DHCPE_RETRY   */ TM_DHCPS_INITREBOOT,  TM_MATRIX_NOOP          },
/* Delay start timer expires */
    {/* TM_DHCPE_RTIMEOUT*/ TM_DHCPS_REBOOTING,   TM_DHCPA_REQUEST        },
    {/* TM_DHCPE_R_ACK   */ TM_DHCPS_INITREBOOT,  TM_R_INVALID            },
    {/* TM_DHCPE_R_NAK   */ TM_DHCPS_INITREBOOT,  TM_R_INVALID            }
    } ,
    {/* State == TM_DHCPS_REBOOTING */
/*      Event                 Next State         Action Procedure Index */
    {/* TM_DHCPE_START   */ TM_DHCPS_REBOOTING,   TM_DHCPA_EALREADY       },
    {/* TM_DHCPE_R_OFFER */ TM_DHCPS_REBOOTING,   TM_R_INVALID            },
    {/* TM_DHCPE_RETRY   */ TM_DHCPS_REBOOTING,   TM_DHCPA_REQUEST        },
    {/* TM_DHCPE_RTIMEOUT*/ TM_DHCPS_INIT,        TM_DHCPA_TIMEOUT        },
    {/* TM_DHCPE_R_ACK   */ TM_DHCPS_BOUND,       TM_DHCPA_CONFIGURE      },
    {/* TM_DHCPE_R_NAK   */ TM_DHCPS_INIT,        TM_DHCPA_RESTART        }
    } ,
    {/* State == TM_DHCPS_BOUND */
/*      Event                 Next State         Action Procedure Index */
    {/* TM_DHCPE_START   */ TM_DHCPS_BOUND,       TM_MATRIX_NOOP          },
    {/* TM_DHCPE_R_OFFER */ TM_DHCPS_BOUND,       TM_R_DISCARD            },
    {/* TM_DHCPE_RETRY   */ TM_DHCPS_BOUND,       TM_MATRIX_NOOP          },
    {/* TM_DHCPE_RTIMEOUT*/ TM_DHCPS_BOUND,       TM_MATRIX_NOOP          },
    {/* TM_DHCPE_R_ACK   */ TM_DHCPS_BOUND,       TM_R_DISCARD            },
    {/* TM_DHCPE_R_NAK   */ TM_DHCPS_BOUND,       TM_R_DISCARD            }
    } ,
    {/* State == TM_DHCPS_RENEWING */
/*      Event                 Next State         Action Procedure Index */
    {/* TM_DHCPE_START   */ TM_DHCPS_RENEWING,    TM_MATRIX_NOOP          },
    {/* TM_DHCPE_R_OFFER */ TM_DHCPS_RENEWING,    TM_R_INVALID            },
    {/* TM_DHCPE_RETRY   */ TM_DHCPS_RENEWING,    TM_MATRIX_NOOP          },
    {/* TM_DHCPE_RTIMEOUT*/ TM_DHCPS_RENEWING,    TM_MATRIX_NOOP          },
/* record lease, set T1, T2 timers */
    {/* TM_DHCPE_R_ACK   */ TM_DHCPS_BOUND,       TM_DHCPA_LEASE          },
    {/* TM_DHCPE_R_NAK   */ TM_DHCPS_INIT,        TM_DHCPA_HALT           }
    } ,
    {/* State == TM_DHCPS_REBINDING */
/*      Event                 Next State         Action Procedure Index */
    {/* TM_DHCPE_START   */ TM_DHCPS_REBINDING,   TM_MATRIX_NOOP          },
    {/* TM_DHCPE_R_OFFER */ TM_DHCPS_REBINDING,   TM_R_INVALID            },
    {/* TM_DHCPE_RETRY   */ TM_DHCPS_REBINDING,   TM_MATRIX_NOOP          },
    {/* TM_DHCPE_RTIMEOUT*/ TM_DHCPS_REBINDING,   TM_MATRIX_NOOP          },
    {/* TM_DHCPE_R_ACK   */ TM_DHCPS_BOUND,       TM_DHCPA_LEASE          },
    {/* TM_DHCPE_R_NAK   */ TM_DHCPS_INIT,        TM_DHCPA_HALT           }
    }
};

/*
 * tfUseDhcp function description:
 * 1. Check that the device entry is valid.
 * 2. allocate array of BOOTP/DHCP entry pointers, if not already done
 * 3. Initialize device boot function with tfDhcpConfig()
 * 4. Initialize device recv call back function with tfDhcpRecvCB()
 * 5. Initialize device BOOT user notify function with passed function pointer.
 *
 * Parameters       meaning
 * interfaceHandle  Pointer to interface
 * devNotifyFuncPtr Pointer to a user notify function (for notification of
 *                  configuration completion)
 *
 * Return value     meaning
 * TM_ENOERROR      success
 * TM_EINVAL        Invalid link layer handle
 * TM_EPERM         Interface is already opened. 
 * TM_EADDRINUSE    A UDP socket is already bound to the BOOTP client port
 * TM_EMFILE        No more sockets
 * TM_ENOBUFS       Not enough available memory
 */
int tfUseDhcp (ttUserInterface    interfaceHandle,
               ttDevNotifyFuncPtr notifyFuncPtr)
{
    ttDeviceEntryPtr devPtr;
    int              errorCode;

    devPtr = (ttDeviceEntryPtr)interfaceHandle;
    errorCode = tfValidInterface(devPtr);
    if (errorCode == TM_ENOERROR)
    {
        tm_call_lock_wait(&(devPtr->devLockEntry));
        devPtr->devBootInfoDhcp.bootFuncPtr = tfDhcpConfig;
/* receive call back from tfBtRecvCB */
        devPtr->devBootInfoDhcp.bootRecvFuncPtr = tfDhcpRecvCB;
/* User notify */
        devPtr->devBootInfoDhcp.bootNotifyFuncPtr = notifyFuncPtr;
        tm_call_unlock(&(devPtr->devLockEntry));
    }
    return errorCode;
}

int tfDhcpStart ( ttDeviceEntryPtr devPtr, int index, tt8Bit type )
{
    ttBtEntryPtr btEntryPtr;
    int          errorCode;

/* Start DHCP State Machine with a send event */
    btEntryPtr = tfBtMap(devPtr, index, type, TM_BOOT_DHCP);
#ifdef TM_USE_DHCP_COLLECT
    if (btEntryPtr != (ttBtEntryPtr)0)
    {
/* Reload the default device settings for DHCP offer collection/selection.
 * These values were cleared when DHCP was unconfigured on this index.
 * If the user hasn't set them then load the latest default values from the
 * device entry. */
        if (btEntryPtr->btCollectSize == 0)
        {
            btEntryPtr->btCollectSize = devPtr->devDhcpCollectSize;
        }
        if (btEntryPtr->btCollectTime == 0)
        {
            btEntryPtr->btCollectTime = devPtr->devDhcpCollectTime;
        }
    }
#endif /* TM_USE_DHCP_COLLECT */
    errorCode = tfDhcpStateMachine( btEntryPtr,
                                    TM_PACKET_NULL_PTR,
                                    TM_DHCPE_START);
    if (    (errorCode == TM_ENOERROR)
         && (btEntryPtr->btDhcpState < TM_DHCPS_BOUND) )
    {
        errorCode = TM_EINPROGRESS;
    }
    return errorCode;
}

/*
 * DHCP offer cache list traversal support:
 *
 * tfDhcpOfferDelete        - Delete a node (offer).
 * tfDhcpOfferKeepNode      - Delete all but the specified node.
 * tfDhcpOfferDeleteServer  - Find and delete one or all server offers.
 * tfDhcpOfferDeleteServer2 - Find and delete one/all offers, skip first.
 * tfDhcpOfferFindServer    - Find node with specific server id.
 * tfDhcpOfferFindYiaddr    - Find node with a non-zero yiaddr field.
 */
static void tfDhcpOfferDelete(ttNodePtr nodePtr)
{
    ttBtOfferEntryPtr   offerPtr;
    ttBtEntryPtr        btEntryPtr;
    ttListPtr           listPtr;

    offerPtr = (ttBtOfferEntryPtr)nodePtr;
    btEntryPtr = offerPtr->btEntryPtr;
    listPtr = &btEntryPtr->btOfferEntryList;
/* Make this the current offer so standard functions will work */
    btEntryPtr->btOfferEntryPtr = offerPtr;
/* Free list of unhandled server options */
    tfBtDiscardServerNames(btEntryPtr);
/* Remove the entry from the list and free it */
    tfListRemove(listPtr, nodePtr);
    tm_kernel_free(nodePtr);
/* Set the head of the list as the current offer */
    btEntryPtr->btOfferEntryPtr
                = (ttBtOfferEntryPtr)listPtr->listHeadNode.nodeNextPtr;
}

TM_NEARCALL static int tfDhcpOfferKeepNode( ttNodePtr       nodePtr,
                                ttGenericUnion  genParam )
{
/* Delete any offers that do not match the specified server id */
    if (nodePtr != (ttNodePtr)genParam.genVoidParmPtr)
    {
        tfDhcpOfferDelete(nodePtr);
    }
    return TM_8BIT_NO;
}

TM_NEARCALL static int tfDhcpOfferDeleteServer( ttNodePtr       nodePtr,
                                    ttGenericUnion  genParam )
{
    int             stopFlag;

    stopFlag = TM_8BIT_NO;
    if (tm_ip_zero((ttUserIpAddress)genParam.gen32bitParm))
    {
/* Delete all offers */
        tfDhcpOfferDelete(nodePtr);
    }
    else if ( tm_ip_match((ttUserIpAddress)genParam.gen32bitParm,
                ((ttBtOfferEntryPtr)nodePtr)->btUserEntry.btuDhcpServerId) )
    {
/* Found a server id match; delete it and stop list traversal */
        tfDhcpOfferDelete(nodePtr);
        stopFlag = TM_8BIT_YES;
    }
    return stopFlag;
}

TM_NEARCALL static int tfDhcpOfferDeleteServer2(    ttNodePtr       nodePtr,
                                        ttGenericUnion  genParam )
{
    ttBtOfferEntryPtr   offerPtr;
    int                 stopFlag;

    offerPtr = (ttBtOfferEntryPtr)nodePtr;
/* Skip the first entry in the list */
    if ( offerPtr->btEntryPtr->btOfferEntryList.listHeadNode.nodeNextPtr
                                                            == nodePtr )
    {
        stopFlag = TM_8BIT_NO;
    }
    else
    {
        stopFlag = tfDhcpOfferDeleteServer(nodePtr, genParam);
    }
    return stopFlag;
}

TM_NEARCALL static int tfDhcpOfferFindServer(   ttNodePtr       nodePtr,
                                    ttGenericUnion  genParam )
{
/* Return TRUE if the server id matches what we are looking for */
    return tm_ip_match( (ttUserIpAddress)genParam.gen32bitParm,
            ((ttBtOfferEntryPtr)nodePtr)->btUserEntry.btuDhcpServerId );
}

TM_NEARCALL static int tfDhcpOfferFindYiaddr(   ttNodePtr       nodePtr,
                                    ttGenericUnion  genParam )
{
    TM_UNREF_IN_ARG(genParam);
/* Return TRUE if the the yiaddr field is valid */
    return tm_ip_not_zero(
            ((ttBtOfferEntryPtr)nodePtr)->btUserEntry.btuYiaddr );
}

/*
 * Delete entries in the DHCP offer cache.
 *
 * Parameters
 * btEntryPtr   pointer to boot (for Dhcp) entry.
 * serverId     IP address of server that uniquely identifies an entry in
 *              the cache of offers. Set to TM_IP_ZERO for operation
 *              specific meaning.
 * operation    TM_DHCP_OFFER_DELETE: delete the specified offer or all
 *              offers if serverId = TM_IP_ZERO.
 *              TM_DHCP_OFFER_DELETE2: delete the specified offer, start
 *              search from second entry in the cache.
 *              TM_DHCP_OFFER_KEEP: delete all offers EXCEPT the one
 *              specified. If serverId = TM_IP_ZERO, find the first offer
 *              with a non-zero yiaddr and delete the rest.
 *
 * Return value     meaning
 * TM_ENOERROR      Success
 * TM_ENOENT        No entry found
 */
int tfDhcpDeleteOffers( ttBtEntryPtr    btEntryPtr,
                        tt4IpAddress    serverId,
                        int             operation )
{
    ttListPtr       listPtr;
    ttListCBFuncPtr listFuncPtr;
    ttGenericUnion  genParam;
    ttNodePtr       offerPtr;
    int             errorCode;

    errorCode = TM_ENOERROR;
    genParam.gen32bitParm = (ttUser32Bit)serverId;
    listPtr = &btEntryPtr->btOfferEntryList;

    if (operation == TM_DHCP_OFFER_KEEP)
    {
        listFuncPtr = tm_ip_not_zero(serverId)
/* Delete all offers EXCEPT the one with the specified serverId */
                    ? tfDhcpOfferFindServer
/* Delete all offers EXCEPT the first one with a non-zero yiaddr */
                    : tfDhcpOfferFindYiaddr;
        offerPtr = tfListWalk(listPtr, listFuncPtr, genParam);
        if (offerPtr == TM_NODE_NULL_PTR)
        {
/* Entry not found */
            errorCode = TM_ENOENT;
            if (tm_ip_zero(serverId))
            {
/* Non-zero yiaddr search failed. That's okay--keep any offer. */
                offerPtr = (ttNodePtr)btEntryPtr->btOfferEntryPtr;
            }
        }
        if (offerPtr != TM_NODE_NULL_PTR)
        {
/* Delete all entries EXCEPT the offer identified */
            genParam.genVoidParmPtr = (ttVoidPtr)offerPtr;
            tfListWalk(listPtr, tfDhcpOfferKeepNode, genParam);
        }
    }
    else
    {
        listFuncPtr = (operation == TM_DHCP_OFFER_DELETE)
/* Delete the specified offer or all offers if serverId = 0 */
                    ? tfDhcpOfferDeleteServer
/* Same, but start search from the second entry */
                    : tfDhcpOfferDeleteServer2;
        offerPtr = tfListWalk(listPtr, listFuncPtr, genParam);
        if (offerPtr == TM_NODE_NULL_PTR && tm_ip_not_zero(serverId))
        {
            errorCode = TM_ENOENT;
        }
    }
    return errorCode;
}

/*
 * Stop the DHCP protocol on an interface/multihome, or for a user index.
 * 1. Map to btEntryPtr.
 * 2. Remove retry timer, and lease timer
 * 3. Transition to the correct state.
 */
int tfDhcpStop ( ttDeviceEntryPtr devPtr, int index, tt8Bit type )
{
    ttBtEntryPtr btEntryPtr;

    btEntryPtr = tfBtMap(devPtr, index, type, TM_BOOT_DHCP);
    if (btEntryPtr != (ttBtEntryPtr)0)
    {
        tfDhcpBtStop(btEntryPtr, 0);
#ifdef TM_USE_DHCP_COLLECT
/* Reset DHCP offer cache settings */
        tfDhcpOfferCleanup(btEntryPtr);
#endif /* TM_USE_DHCP_COLLECT */
#ifdef TM_USE_STOP_TRECK
        if (tm_16bit_one_bit_set(devPtr->devFlag2, 
                                 TM_DEVF2_UNINITIALIZING))
        {
#ifdef TM_USE_DHCP_FQDN
            if (   (btEntryPtr->btUserFqdnPtr != (ttCharPtr)0)
                && (btEntryPtr->btUserFqdnPtr !=
                        tm_context(tvFqdnStruct).domainName))
            {
                tm_kernel_free(btEntryPtr->btUserFqdnPtr);
                btEntryPtr->btUserFqdnPtr = (ttCharPtr)0;
            }
#endif /* TM_USE_DHCP_FQDN */
/* Delete the offer cache for this entry */
            tfDhcpDeleteOffers(btEntryPtr, TM_IP_ZERO, TM_DHCP_OFFER_DELETE);
/* Delete the entry */
            tm_kernel_free(btEntryPtr);
            devPtr->devBtEntryTablePtr[type][TM_BOOT_DHCP][index] =
                                                             (ttBtEntryPtr)0;
        }
#endif /* TM_USE_STOP_TRECK */
    }
    return TM_ENOERROR;
}

static void tfDhcpBtStop (ttBtEntryPtr btEntryPtr, int errorCode)
{
    ttDeviceEntryPtr devPtr;

    TM_UNREF_IN_ARG(errorCode);

    devPtr = btEntryPtr->btDevEntryPtr;
    tfDhcpRemoveTimers(btEntryPtr);
    if (btEntryPtr->btDhcpState >= TM_DHCPS_BOUND)
    {
/* Cancel collision detection */
        (void)tfCancelCollisionDetection(
#ifdef TM_USE_STRONG_ESL
                                          btEntryPtr->btDevEntryPtr,
#endif /* TM_USE_STRONG_ESL */
                                          btEntryPtr->btYiaddr);
/*
 * Send a RELEASE message to the server. If we are in BOUND state,
 * move to renewing state so that the logic in tfDhcpMsgSend() will
 * set the right ciaddr, and unicast destination. In rebinding state
 * do as usual (ciaddr, and broadcast destination.)
 */
        if (btEntryPtr->btDhcpState == TM_DHCPS_BOUND)
        {
            btEntryPtr->btDhcpState = TM_DHCPS_RENEWING;
        }
        if ( tm_16bit_bits_not_set(devPtr->devFlag2,
                                   (   TM_DEVF2_NO_DHCP_RELEASE
                                     | TM_DEVF2_UNINITIALIZING )) )
/*
 * If user did not turn off sending DHCP Release messages on the device,
 * and we are not in the process of un-initializing
 */
        {
            (void)tfBtOpenSocket(btEntryPtr);
            (void)tfDhcpMsgSend(btEntryPtr, (ttTmCBFuncPtr)0,
                                TM_DHCPRELEASE_END);
        }
    }
    if (btEntryPtr->btType == TM_BT_CONF)
    { 
        tm_8bit_clr_bit( tm_ip_dev_conf_flag(devPtr, btEntryPtr->btIndex),
                         (TM_DEV_IP_DHCP | TM_DEV_IP_CONF_STARTED) );
    }
/*
 * Discard previously discovered values. Re-initialize user entry.
 * Close the socket.
 */
    tfBtInitUserEntry(btEntryPtr);
    btEntryPtr->btDhcpState = TM_DHCPS_INIT;
    tfBtCloseSocket(btEntryPtr);
    return;
}

/*
 * tfDhcpConfig() function description
 * tfDhcpConfig function pointer is stored in the device driver structure
 * at devBootFuncPtr.
 * When the user wants to configure the interface with the BOOTP or DHCP
 * client protocol, tfConfigInterface() is called, which in turns calls
 * the device link layer open function, which in turns calls the interface
 * boot function: tfDhcpConfig(). Note that tfConfigInterface() prevents
 * re_entrancy as long as the configuration as not completed or failed.
 *
 * 1. If the BOOTP client UDP socket is not opened, open it.
 * 2. Start the DHCP state machine.
 * Parameter        meaning
 * voidPtr          pointer to device entry
 * flag             open/close
 *
 * Return value     meaning
 * TM_EINPROGRESS   success
 * TM_EMFILE        Not enough sockets
 * TM_ENOBUFS       Not enough memory
 * other error code UDP send failed
 */
static int tfDhcpConfig (ttVoidPtr voidPtr, int flag, tt16Bit multiHomeIndex)
{
    ttDeviceEntryPtr devPtr;
    int              errorCode;
    tt8Bit           type;

    devPtr = (ttDeviceEntryPtr)voidPtr;
    if (flag == TM_LL_OPEN_STARTED)
/* open */
    {
        if ( tm_8bit_one_bit_set(
                            tm_ip_dev_conf_flag(devPtr, multiHomeIndex),
                            TM_DEV_IP_DHCP ) )
        {
            errorCode = tfBtInit( devPtr,
                                  TM_DHCP_NOTIFY_FUNC_NULL_PTR,
                                  (int)multiHomeIndex,
                                  TM_BT_CONF,
                                  TM_BOOT_DHCP );
            if (errorCode == TM_ENOERROR)
            {
                errorCode = tfDhcpStart( devPtr,
                                         (int)multiHomeIndex,
                                         TM_BT_CONF );
            }
        }
        else
        {
/* Need to use tfUseBootp with TM_DEV_IP_BOOTP */
            errorCode = TM_EPERM;
        }
    }
    else
/* close */
    {
#ifdef TM_USE_STOP_TRECK
        if (flag == TM_LL_USER_STOP)
        {
            type = TM_BT_USER;
        }
        else
#endif /* TM_USE_STOP_TRECK */
        {
            type = TM_BT_CONF;
        }
        errorCode = tfDhcpStop( devPtr,
                                (int)multiHomeIndex,
                                type );
    }
    return errorCode;
}

#ifdef TM_USE_DHCP_COLLECT
/*
 * tfDhcpOfferCleanup - Reset the offer cache state information prior to
 *                      unconfiguring DHCP.
 *
 * Common code for tfDhcpStop and tfDhcpCollectTimer.
 */
static void tfDhcpOfferCleanup(ttBtEntryPtr btEntryPtr)
{
/* Delete all cached offers, leave one remaining */
    tfDhcpDeleteOffers(btEntryPtr, TM_IP_ZERO, TM_DHCP_OFFER_DELETE2);
/* Clear the yiaddr of the remaining offer to invalidate it */
    tm_ip_copy(TM_IP_ZERO, btEntryPtr->btYiaddr);
/* Reset elements that are not persistent across interface close and reopen */
    tm_ip_copy(TM_IP_ZERO, btEntryPtr->btPrefAddr);
    tm_8bit_clr_bit(btEntryPtr->btFlags, TM_BOOTF_EXCLUSIVE_ADDRESS);
    btEntryPtr->btCollectSize = 0;
    btEntryPtr->btCollectTime = 0;
}
#endif /* TM_USE_DHCP_COLLECT */

/*
 * tfDhcpRetryTimer function description:
 * Called by the timer when retry interval has expired during rebooting,
 * selecting, or requesting states.
 * 1. Lock interface
 * 2. If number of retries has not been reached,
 *      . start the state machine with a retry timer event.
 *    else
 *      . start the state machine with a retry timeout event.
 * 4. Unlock interface
 *
 * Parameters       meaning
 * voidPtr          pointer to boot (for Dhcp) entry
 *
 * RETURN
 * no return value
 */
static void tfDhcpRetryTimer (ttVoidPtr      timerBlockPtr,
                              ttGenericUnion userParm1,
                              ttGenericUnion userParm2)
{
    ttDeviceEntryPtr    devPtr;
    ttBtEntryPtr        btEntryPtr;
    tt8Bit              event;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    
    btEntryPtr = (ttBtEntryPtr)userParm1.genVoidParmPtr;
    devPtr = btEntryPtr->btDevEntryPtr;
    tm_call_lock_wait(&(devPtr->devLockEntry));
    if (tm_timer_not_reinit((ttTimerPtr)timerBlockPtr))
    {
/* Single shot timer */
        btEntryPtr->btRetryTimerPtr = TM_TMR_NULL_PTR;
/* If BOOT client socket descriptor open */
        if
#ifdef TM_USE_STRONG_ESL
           (devPtr->devBootSocketDescriptor != TM_SOCKET_ERROR)
#else /* TM_USE_STRONG_ESL */
           (tm_context(tvBootSocketDescriptor) != TM_SOCKET_ERROR)
#endif /* TM_USE_STRONG_ESL */
        {
/*
 * and if we trying to configure using DHCP protocol
 * and device not configured yet
 */
            if (    (btEntryPtr->btRetries < devPtr->devBtMaxRetries)
                 || (devPtr->devBtMaxRetries == TM_DEV_BOOT_INF) )
            {
                event = (tt8Bit)TM_DHCPE_RETRY;
            }
            else
            {
                event = (tt8Bit)TM_DHCPE_RTIMEOUT;
            }
            (void)tfDhcpStateMachine(btEntryPtr,
                                     TM_PACKET_NULL_PTR,
                                     event);
        }
    }
    tm_call_unlock(&(devPtr->devLockEntry));
}

/*
 * tfDhcpLeaseTimer function description:
 * Called by the timer when lease timer interval has expired. Handles
 * state transition, and retries when T1 or T2 has expired.
 * 1. Lock interface
 * 2. Check that timer has not been removed, updated, and that we are in
 *    a valid state for the lease (BOUND, RENEWING, or REBINDING)
 * 2. In bound state, T1 expired, update T1, move to
 *    RENEWING state, and execute following renewing state code (in 3. below).
 * 3. In Renewing state, update T2.
 *    . if T2 has expired, move to rebinding state, and execute rebinding
 *      state code (in 4. below).
 *    . else if T2 has not expired:
 *       . update lease time.
 *       . If T2 half time is bigger than 60 seconds,
 *         re-initialize the timer to T2 half time.
 *       . else if T2 half time is less than 60 seconds, re-initialize the
 *         timer to T2.
 * 4. In rebinding state, update lease time.
 *   . update lease time.
 *   . If lease time has expired, move to init state.
 *   . else if Lease time has not expired:
 *       . If lease half time is bigger than 60 seconds,
 *         re-initialize the timer to lease half time.
 *       . else if lease half time is less than 60 seconds, re-initialize the
 *         timer to lease time.
 * 5. . If we moved to init state, halt the network
 *    . else update the timer for the next timeout, and send a DHCP request.
 * 6. Unlock interface
 * Parameters       meaning
 * voidPtr          pointer to boot (for Dhcp) entry
 *
 * RETURN
 * no return value
 */
static void tfDhcpLeaseTimer (ttVoidPtr      timerBlockPtr,
                              ttGenericUnion userParm1,
                              ttGenericUnion userParm2)
{
#if (defined(TM_LOCK_NEEDED) || defined(TM_USE_STRONG_ESL))
    ttDeviceEntryPtr    devPtr;
#endif /* TM_LOCK_NEEDED || TM_USE_STRONG_ESL */
    ttBtEntryPtr        btEntryPtr;
    ttTimerPtr          dhcpLeaseTimerPtr;
    tt32Bit             leaseTxTimeout;
    tt32Bit             nextLeaseTxTimeout;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    
    btEntryPtr = (ttBtEntryPtr) userParm1.genVoidParmPtr;
#if (defined(TM_LOCK_NEEDED) || defined(TM_USE_STRONG_ESL))
    devPtr = btEntryPtr->btDevEntryPtr;
#endif /* TM_LOCK_NEEDED || TM_USE_STRONG_ESL */
    tm_call_lock_wait(&(devPtr->devLockEntry));
    dhcpLeaseTimerPtr = btEntryPtr->btDhcpLeaseTimerPtr;
/*
 * check whether the timer has not been
 * re-initialized/removed between the call back function and obtaining
 * the device lock.
 */
    if (    (tm_timer_not_reinit((ttTimerPtr)timerBlockPtr))
         && (dhcpLeaseTimerPtr == (ttTimerPtr)timerBlockPtr))
    {
        if (btEntryPtr->btDhcpState == TM_DHCPS_BOUND)
/* In bound state we need to open the BOOTP/DHCP socket */
        {
            (void)tfBtOpenSocket(btEntryPtr);
        }
/* If BOOT client socket descriptor open
 * and we are in bound, renewing or rebinding state.
 */
        if (
#ifdef TM_USE_STRONG_ESL
                (devPtr->devBootSocketDescriptor != TM_SOCKET_ERROR)
#else /* TM_USE_STRONG_ESL */
                (tm_context(tvBootSocketDescriptor) != TM_SOCKET_ERROR)
#endif /* TM_USE_STRONG_ESL */
             && (btEntryPtr->btDhcpState >= TM_DHCPS_BOUND) )
        {
/* DHCP lease timeout */
            leaseTxTimeout = tm_tmr_interval(dhcpLeaseTimerPtr);
/* Next DHCP lease timeout */
            nextLeaseTxTimeout = TM_UL(0);
/* Update leaseTime/T1/T2 */
            if (btEntryPtr->btDhcpState == TM_DHCPS_BOUND)
            {
/*
 * In bound state, T1 expired, update T1, T2, and lease time, move to
 * RENEWING state
 */
                btEntryPtr->btDhcpT1 = TM_UL(0);
                btEntryPtr->btDhcpState = TM_DHCPS_RENEWING;
/* execute renewing state code below */
            }
            if (btEntryPtr->btDhcpState == TM_DHCPS_RENEWING)
            {
/* In Renewing state, update T2, and lease time  */
                if (leaseTxTimeout >= btEntryPtr->btDhcpT2)
                {
                    btEntryPtr->btDhcpT2 = TM_UL(0);
/* T2 expired: state transition */
                    btEntryPtr->btDhcpState = TM_DHCPS_REBINDING;
/* Execute REBINDING state code below */
                }
                else
                {
/* T2 has not expired yet, no state transition */
                    btEntryPtr->btDhcpT2 -= leaseTxTimeout;
                    btEntryPtr->btDhcpLeaseTime -= leaseTxTimeout;
                    nextLeaseTxTimeout = btEntryPtr->btDhcpT2/2;
                    if ( nextLeaseTxTimeout < TM_DHCPT_MIN_TX )
                    {
                        nextLeaseTxTimeout = btEntryPtr->btDhcpT2;
                    }
                }
            }
            if (btEntryPtr->btDhcpState == TM_DHCPS_REBINDING)
            {
/* In Rebinding state, update lease time */
                if (leaseTxTimeout >= btEntryPtr->btDhcpLeaseTime)
                {
/* Lease expired, move back to init state */
                    btEntryPtr->btDhcpLeaseTime = TM_UL(0);
                    btEntryPtr->btDhcpState = TM_DHCPS_INIT;
/* Execute INIT state code below */
                }
                else
                {
                    btEntryPtr->btDhcpLeaseTime -= leaseTxTimeout;
                    nextLeaseTxTimeout = btEntryPtr->btDhcpLeaseTime/2;
                    if (nextLeaseTxTimeout < TM_DHCPT_MIN_TX)
                    {
                        nextLeaseTxTimeout = btEntryPtr->btDhcpLeaseTime;
                    }
                }
            }
            if (btEntryPtr->btDhcpState == TM_DHCPS_INIT)
            {
/* Back to init state */
                (void)tfDhcpaHalt(btEntryPtr, TM_PACKET_NULL_PTR);
            }
            else
            {
/* update timer timeout */
                tm_timer_new_time(btEntryPtr->btDhcpLeaseTimerPtr,
                                   nextLeaseTxTimeout);
                (void)tfDhcpaRequest(btEntryPtr, TM_PACKET_NULL_PTR);
            }
        } /* If in bound, renewing, rebinding state */
        else
        {
            tm_timer_remove(dhcpLeaseTimerPtr);
            btEntryPtr->btDhcpLeaseTimerPtr = TM_TMR_NULL_PTR;
        }
    } /* if dhcpLeaseTimerPtr */
    tm_call_unlock(&(devPtr->devLockEntry));
}

#ifdef TM_USE_DHCP_COLLECT
/*
 * tfDhcpCollectTimer function description:
 * Called by the timer at the end of the selecting state.
 * 1. Lock interface
 * 2. If the user has provided an offer callback function:
 *      a. call it.
 *      b. if the user does not select an offer then signal timeout.
 * 3. Else if the user has provided a non-exclusive preferred IP address
 *    and an entry exists in offer cache:
 *      a. if there is a valid offer in the cache then select it.
 *      b. else signal timeout.
 * 4. Else signal timeout.
 * 5. Unlock interface
 *
 * Parameters       meaning
 * voidPtr          pointer to boot (for Dhcp) entry
 *
 * RETURN
 * no return value
 */
TM_NEARCALL static void tfDhcpCollectTimer(
                                ttVoidPtr      timerBlockPtr,
                                ttGenericUnion userParm1,
                                ttGenericUnion userParm2 )
{
    ttDeviceEntryPtr            devPtr;
    ttBtEntryPtr                btEntryPtr;
    ttUserDhcpOfferCBFuncPtr    userOfferFuncPtr;
    int                         errorCode;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);

    btEntryPtr = (ttBtEntryPtr)userParm1.genVoidParmPtr;
    devPtr = btEntryPtr->btDevEntryPtr;
    tm_call_lock_wait(&(devPtr->devLockEntry));
    if (tm_timer_not_reinit((ttTimerPtr)timerBlockPtr))
    {
/* Single shot timer */
        btEntryPtr->btRetryTimerPtr = TM_TMR_NULL_PTR;
        if (btEntryPtr->btDhcpState == TM_DHCPS_SELECTING)
        {
            userOfferFuncPtr = devPtr->devOfferFuncPtr;
            if (userOfferFuncPtr != (ttUserDhcpOfferCBFuncPtr)0)
            {
/* Unlock the device */
                tm_call_unlock(&(devPtr->devLockEntry));
/* Call the user to select an offer */
                (*userOfferFuncPtr)(
                        (ttUserInterface)btEntryPtr->btDevEntryPtr,
                        (unsigned)btEntryPtr->btIndex,
                        TM_DHCP_EVENT_TIMEOUT,
                        TM_IP_ZERO,
                        TM_IP_ZERO,
                        devPtr->devOfferParam );
/* Lock the device */
                tm_call_lock_wait(&(devPtr->devLockEntry));
            }
            else if (   tm_ip_not_zero(btEntryPtr->btPrefAddr)
                    &&  tm_8bit_bits_not_set(btEntryPtr->btFlags,
                                            TM_BOOTF_EXCLUSIVE_ADDRESS) )
            {
/* The user specified a preferred IP address but is willing to accept
 * another offer.
 * Find the first "real" offer (i.e. yiaddr not zero). Remove all other
 * entries. */
                errorCode = tfDhcpDeleteOffers( btEntryPtr,
                                                TM_IP_ZERO,
                                                TM_DHCP_OFFER_KEEP );
                if (errorCode == TM_ENOERROR)
                {
/* Request the remaining offer */
                    tfDhcpRequestOffer(btEntryPtr);
                }
            }
/* If the user has not made a selection, signal timeout condition. */
            if (btEntryPtr->btDhcpState == TM_DHCPS_SELECTING)
            {
                tfDhcpOfferCleanup(btEntryPtr);
                tfDhcpStateMachine( btEntryPtr,
                                    TM_PACKET_NULL_PTR,
                                    TM_DHCPE_RTIMEOUT );
            }
        }
    }
    tm_call_unlock(&(devPtr->devLockEntry));
}
#endif /* TM_USE_DHCP_COLLECT */

/*
 * tfDhcpStateMachine() function description.
 *
 * State machine main function.
 * Given a causing event, and the DHCP current state,
 * get an action procedure index and a new DHCP state from the local
 * variable array tlDhcpTransitionMatrix[][]. Transition to the new state,
 * and then call all state functions
 * corresponding to the action procedure index in the local variable
 * array tlDhcpStateFunctTable[]. If any of the
 * state functions in the list returns an error, return immediately, and do
 * not call the remaining state functions in the action procedure index list.
 *
 * Special values for action procedure index:
 * TM_DHCPA_LAST_INDEX (== TM_MATRIX_NOOP): no state function is called,
 *                                         just a DHCP state transition
 * index > TM_DHCPA_LAST_INDEX: no state function is called, no DHCP state
 *                             transition either. Return
 *                             error corresponding to index value.
 *
 *
 * Parameters  description
 * btEntryPtr  pointer to boot (for Dhcp) entry
 * packetPtr   DHCP RecvCB packet pointer to be passed to the
 *             DHCP state function. (Null for non RecvCB packet events)
 * event       Causing Event (to be used as second index in
 *             tvDhcpTransitionMatrix[][])
 *
 * Return value
 * error
 */
static int tfDhcpStateMachine (ttBtEntryPtr       btEntryPtr,
                               ttPacketPtr        packetPtr,
/* event (to be used as index in tlDhcpTransitionMatrix[][] */
                               tt8Bit             event)
{
    ttTransitionMatrixEntryPtr matrixEntryPtr;
    ttDhcpStateFunctEntryPtr   dhcpStateFunctEntryPtr;
    int                        errCode;
    tt8Bit                     curState;
    tt8Bit                     index;

/* Current state */
    curState = btEntryPtr->btDhcpState;
/* Point to transition matrix for the current state, event pair */
    matrixEntryPtr = (ttTransitionMatrixEntryPtr)
                                     &tlDhcpTransitionMatrix[curState][event];
/* Index into state function table */
    index = matrixEntryPtr->mtxFunctionIndex;
/* New state */
    btEntryPtr->btDhcpState = matrixEntryPtr->mtxTransition;
/* Check whether there is any function to call */
    if (index < TM_DHCPA_LAST_INDEX)
    {
/* State function table entry pointer */
        dhcpStateFunctEntryPtr =(ttDhcpStateFunctEntryPtr)
                                                &tlDhcpStateFunctTable[index];
/* Call all functions corresponding to index */
        do
        {
/* Call state function */
            errCode = tm_call_state_function(dhcpStateFunctEntryPtr,
                                             btEntryPtr,
                                             packetPtr);
/* If state function returned an error */
            if (errCode != 0)
            {
/* Stop any further processing in the state machine */
                break;
            }
/* Point to next entry in state function table */
            dhcpStateFunctEntryPtr++;
        } while (dhcpStateFunctEntryPtr->dstaIndex == index);
    }
    else
    {
/* No function to call */
        if (index == TM_MATRIX_NOOP)
        {
/* No error. */
            errCode = TM_ENOERROR;
        }
        else
        {
/*
 * Index is coded socket error value. Retrieve system error.
 */
            errCode = tm_dhcpa_sys_err(index);
        }
    }
    return errCode;
}

/*
 * tfDhcpRecvCB() Function Description
 * 1. Lock the interface
 * 2. Process an RecvCB packet for the DHCP client. We cannot assume
 *    any particular order in the options. Scan the option fields
 *    to get the DHCP message type and the Server ID. Check for overloaded
 *    options. If message type and server ID are valid, then start the state
 *    machine with the Message type.
 * 3. Unlock the interface.
 *
 * Parameters:
 * socketDescriptor socketDescritor of the BOOT socket opened in
 *                  tfDhcpConfig() or tfUseDhcp().
 * socketCBFlags    event flag (TM_CB_RECV always, since it is the only
 *                  flag we have registered for)
 *
 * No return value
 */
void tfDhcpRecvCB ( ttPacketPtr  packetPtr,
                    ttBtEntryPtr btEntryPtr )
{
    ttBootHeaderPtr bootHdrPtr;
   
/* point to packet DHCP header */
    bootHdrPtr = (ttBootHeaderPtr)packetPtr->pktLinkDataPtr;
/*
 * RFC 2131 specifies that the option length should be at least 312 bytes,
 * but since windows NT sends DHCP packets with option lengths smaller
 * than 312 bytes, we will not check on that.
 */
/* gather information from the option area */

/* check RFC2132 compliant BOOTP vendor field */
    if (bootHdrPtr->bootOptions.optMagic == TM_BOOT_MAGIC)
    {
/*
 * Scan the option for DHCP message, and server ID. We cannot assume that
 * the recvCB message TAGS are word aligned, and we cannot assume any option
 * order. If message is OK, call the state machine.
 */
        tfDhcpScanOptions(packetPtr, btEntryPtr, TM_DHCPA_LAST_INDEX);
    }/* RFC 2132 if */
    return;
}

/*
 * tfDhcpAddCustOpt() Function Description
 * Called by tfListWalk to copy an option in the custom options list to
 * the outgoing packet.
 *
 * Parameters   Meaning
 * nodePtr      Pointer to the option to be set
 * genParam1    Pointer to a structure containing the offset of the current
 *              option in the packet
 *
 * Return
 * TM_8BIT_NO
 */
static int tfDhcpAddCustOpt(ttNodePtr      nodePtr,
                            ttGenericUnion genParam1)
{
    ttBtUserCustOptPtr optPtr;
    ttBtPktDataPtr     pdPtr;
#ifdef TM_DSP
    int             tempDspCount;
#endif /* TM_DSP */

    optPtr = (ttBtUserCustOptPtr)nodePtr;
    pdPtr  = (ttBtPktDataPtr)genParam1.genVoidParmPtr;

    if (    (optPtr->buoBootOption != TM_DHCPGET_PARAMETER_LIST)
         || (pdPtr->bpdMsgEnd == TM_DHCPREQUEST_END)
         || (pdPtr->bpdMsgEnd == TM_DHCPDISCOVER_END)
       )
/* GET_PARAMETER_LIST option only added on discover or request messages */
    {
/* Set the option */
        tm_dhcp_set_char(optPtr->buoBootOption,
                         pdPtr->bpdBootOptionPtr,
                         0,
                         pdPtr->bpdByteOffset);
        tm_dhcp_next_option(pdPtr->bpdBootOptionPtr,
                            1,
                            pdPtr->bpdByteOffset);
/* Set the option length */
        tm_dhcp_set_char(optPtr->buoDataLen,
                         pdPtr->bpdBootOptionPtr,
                         0,
                         pdPtr->bpdByteOffset);
        tm_dhcp_next_option(pdPtr->bpdBootOptionPtr,
                            1,
                            pdPtr->bpdByteOffset);
/* Set the option data */
        tm_dhcp_copy_list_to_pkt(optPtr->buoDataPtr,
                                 pdPtr->bpdBootOptionPtr,
                                 optPtr->buoDataLen,
                                 pdPtr->bpdByteOffset,
                                 tempDspCount);
        tm_dhcp_next_option(pdPtr->bpdBootOptionPtr,
                            optPtr->buoDataLen,
                            pdPtr->bpdByteOffset);
    }
    return TM_8BIT_NO;
}

/*
 * tfDhcpMsgSend function description:
 * Called whenever a DHCP message type need to be send/broadcast to a
 * server.
 * Note for a decline message, state is requesting (caller sets state
 *                                                  to requesting.)
 *      for a release message, state is renewing, or rebinding (caller sets
                                 state to renewing, if state not  rebinding.)
 * Send a DHCP Message:
 *  .  Use common routine to allocate and fill in DHCP header.
 *  .  Finish filling the DHCP header depending on the DHCP state:
 *     0. Renewing state:   DHCP message
 *                          Use leased address for Ciaddr field.
 *                          send is unicast to server IP address.
 *                          server ID option for release message
 *     1. Rebinding state:  DHCP message with no option.
 *                          Use leased address for Ciaddr field.
 *                          send is broadcast.
 *                          server ID option for release message.
 *     2. Selecting state:  DHCP message with requested IP option (only if
 *                          user set a requested IP address).
 *                          Use 0 for ciaddr field.
 *                          send is broadcast.
 *     3. Rebooting state:  DHCP message with requested IP option.
 *                          Use 0 for ciaddr field.
 *                          send is broadcast.
 *     4. Requesting state: DHCP message with requested IP option,
 *                          server ID option (catches decline messages).
 *                          Use 0 for ciaddr field.
 *                          send is broadcast.
 *  . Add options in the same order as windows:
 *      1. Client ID option (in all messages)
 *      2. Requested IP option (if needed. See above)
 *      3. Server Id option (if needed. See above)
 *      4. host name option (in discover and requests messages)
 *      5. FQDN option (in requests messages only)
 *      6. Parameter list option (in discover and requests messages)
 * Parameters       Meaning
 * btEntryPtr       pointer to boot (for Dhcp) entry
 * timerFuncPtr     pointer to timer function to add (if any)
 * dhcpMsgPtr       Pointer to dhcp Message without end tag
 * dhcpMsgEnd       DHCP message type to send with end tag
 *
 * Return
 * TM_ENOERROR      no error
 * TM_ENOBUFS       not enough buffer to send message.
 * Other            as returned by send function.
 */
static int tfDhcpMsgSend ( ttBtEntryPtr     btEntryPtr,
                           ttTmCBFuncPtr    timerFuncPtr,
                           tt32Bit          dhcpMsgEnd)
{
    ttBootHeaderPtr bootHdrPtr;
    ttPacketPtr     packetPtr;
    tt8BitPtr       bootOptionPtr;
    ttNodePtr       nodePtr;
    tt8BitPtr       hostNamePtr;
#ifdef TM_USE_DHCP_FQDN
    tt8BitPtr       fqdnPtr;
#endif /* TM_USE_DHCP_FQDN */
    ttGenericUnion  listParam;
    tt4IpAddress    destIpAddress;
    ttPktLen        dhcpMsgLength;
    ttBtPktData     pktData;
    int             state;
    int             errorCode;
#ifdef TM_USE_DHCP_FQDN
    int             fqdnLen;
    unsigned int    fqdnFlags;
    unsigned int    domainNameLength;
    unsigned int    allocLength;
#endif /* TM_USE_DHCP_FQDN */
#ifdef TM_DSP
    unsigned int    byteOffset;
    unsigned int    tempDspCount;
#endif /* TM_DSP */
    tt8Bit          addServerIdOption;
    tt8Bit          addRequestIpOption;
    tt8Bit          hostNameLength;
#ifdef TM_USE_DHCP_FQDN
    tt8Bit          fqdnOptionFlags;
    tt8Bit          fqdnAllocated;
#endif /* TM_USE_DHCP_FQDN */

    errorCode = TM_ENOERROR;
#ifdef TM_DSP
    byteOffset = 0;
#endif /* TM_DSP */
    addServerIdOption = TM_8BIT_ZERO;
    addRequestIpOption = TM_8BIT_ZERO;
/* Allocate and fill in a DHCP header */
    packetPtr = tfBtGetFillHeader(btEntryPtr,
                                  TM_DHCP_HEADER_SIZE,
                                  timerFuncPtr);
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
        state = btEntryPtr->btDhcpState;
        bootHdrPtr = (ttBootHeaderPtr)packetPtr->pktLinkDataPtr;
        bootOptionPtr = bootHdrPtr->bootOptions.optFields;
/*
 * ciaddr already initialized to zero in tfBtGetFillHeader().
 */
/* Message is broadcast except in renewing state. */
        tm_ip_copy(TM_IP_LIMITED_BROADCAST, destIpAddress);
/* Copy DHCP message type, with END tag */
        bootHdrPtr->bootOptions.optDhcpMsg = dhcpMsgEnd;
/* Point to end tag */
        tm_dhcp_next_option(bootOptionPtr, 3, byteOffset);
/* We will add additional options (if any) + client ID option + End tag below */
        if (    (state == TM_DHCPS_RENEWING)
             || (state == TM_DHCPS_REBINDING) )
        {
            if (btEntryPtr->btType == TM_BT_USER)
            {
/* Reset zero IP address requirement */
                tm_16bit_clr_bit( packetPtr->pktFlags2, 
                                  TM_PF2_ZERO_SRC_IP_ADDRESS);
            }
            if (state == TM_DHCPS_RENEWING)
            {
/*
 * In RENEWING state:
 * . send is unicast to server Ip address (set here).
 */
                tm_ip_copy(btEntryPtr->btDhcpServerId, destIpAddress);
            }
/*
 * In REBINDING state:
 * . send is broadcast (default))
 */
/*
 * In REBINDING state or RENEWING state:
 * . Use leased address for Ciaddr field (set here).
 */
            tm_ip_copy(btEntryPtr->btYiaddr, bootHdrPtr->bootCiaddr);
/*
 * In REBINDING, RENEWING state:
 * If sending a release message:
 * . Add DHCP server ID option
 */
            if (dhcpMsgEnd == TM_DHCPRELEASE_END)
            {
/* DHCP Release Message. Add DHCP server ID option. */
                addServerIdOption = TM_8BIT_YES;
            }
        }
        else
        {
/*
 * In REQUESTING, REBOOTING, or SELECTING state
 * . send is broadcast (default) .
 * . Ciaddr is 0 (default).
 */
            if (    (state != TM_DHCPS_SELECTING)
                 || (tm_8bit_one_bit_set( btEntryPtr->btUserSetFlags,
                                          TM_BT_USER_SET_IPADDRESS )) )
            {
/*
 * In REQUESTING or REBOOTING state:
 * . DHCP message with requested IP option 
 * In Selecting state (Discover)
 *  . DHCP message with requested IP option only if user has set a requested 
 *    IP address.
 */
                addRequestIpOption = TM_8BIT_YES;
            }
            if (state == TM_DHCPS_REQUESTING)
            {
/*
 * In REQUESTING state:
 * . Add server ID option
 * (This will also catch DECLINE messages)
 */
                addServerIdOption = TM_8BIT_YES;
            }
        } /* else: (REQUESTING, REBOOTING, SELECTING) */
/* 
 * Add options in the same order as windows:
 * 1. Client ID option (in all messages)
 * 2. Request IP option (if needed. See above)
 * 3. Server Id option (if needed. See above)
 * 4. host name option (in discover and requests messages)
 * 5. FQDN option (in requests messages only)
 * 6. Parameter list option (in discover and requests messages)
 */
        if ( tm_8bit_bits_not_set( btEntryPtr->btUserSetFlags,
                                   TM_BT_SUPPRESS_CLIENT_ID ) )
        {
/* Add the Client ID option as set in tfDhcpInit() */
            tm_dhcp_set_char(TM_DHCP_CLIENT_ID, bootOptionPtr, 0, byteOffset);
            tm_dhcp_next_option(bootOptionPtr, 1, byteOffset);

            tm_dhcp_set_char(
                btEntryPtr->btDhcpClientIdLength, bootOptionPtr, 0, byteOffset);
            tm_dhcp_next_option(bootOptionPtr, 1, byteOffset);            

            tm_dhcp_copy_list_to_pkt(
                btEntryPtr->btDhcpClientIdPtr,
                bootOptionPtr,
                (unsigned int)btEntryPtr->btDhcpClientIdLength,
                byteOffset, tempDspCount);

            tm_dhcp_next_option(bootOptionPtr,
                                (unsigned)btEntryPtr->btDhcpClientIdLength,
                                byteOffset);
        }
        if (addRequestIpOption != TM_8BIT_ZERO)
        {
/* requested IP option */
            tm_dhcp_copy_list_to_pkt(
                    tlDhcpRequestedIp, bootOptionPtr, 2,
                    byteOffset, tempDspCount);
            tm_dhcp_next_option(bootOptionPtr, 2, byteOffset);

            tm_dhcp_byte_copy_to_pkt( (tt8BitPtr)&btEntryPtr->btYiaddr,
                                      bootOptionPtr,
                                      TM_IP_ADDRESS_LENGTH,
                                      byteOffset );
            tm_dhcp_next_option(
                    bootOptionPtr, TM_IP_ADDRESS_LENGTH, byteOffset);
        }
        if (addServerIdOption != TM_8BIT_ZERO)
        {
/* Server ID option */
            tm_dhcp_copy_list_to_pkt(tlDhcpServerId,
                                     bootOptionPtr,
                                     2,
                                     byteOffset, tempDspCount);
            tm_dhcp_next_option(bootOptionPtr, 2, byteOffset);
/* server IP address */
            tm_dhcp_byte_copy_to_pkt((tt8BitPtr)&btEntryPtr->btDhcpServerId,
                                     bootOptionPtr,
                                     TM_IP_ADDRESS_LENGTH,
                                     byteOffset);
            tm_dhcp_next_option(bootOptionPtr,
                                TM_IP_ADDRESS_LENGTH, byteOffset);
        }
/* Host name as set by the user */
        hostNameLength = btEntryPtr->btDhcpHostNameLength;
        hostNamePtr = btEntryPtr->btDhcpHostNamePtr;
        if (    (dhcpMsgEnd == TM_DHCPREQUEST_END)
             || (dhcpMsgEnd == TM_DHCPDISCOVER_END) )
        {
/* Host name option, in discover and request messages. */
            if (    (dhcpMsgEnd == TM_DHCPREQUEST_END)
                 && (btEntryPtr->btDhcpRxHostNameLength != 0)
                 && (tm_16bit_one_bit_set(btEntryPtr->btDevEntryPtr->devFlag2,
                                          TM_DEVF2_SRVR_HOST_NAME) )
               )
            {
/* Host name as set by the server */
                hostNamePtr = btEntryPtr->btDhcpRxHostNamePtr;
                hostNameLength = (tt8Bit)btEntryPtr->btDhcpRxHostNameLength;
            }
            if (hostNameLength != (tt8Bit)0)
            {
/* Add host name option */
                tm_dhcp_set_char(
                            TM_DHCP_HOST_NAME, bootOptionPtr, 0, byteOffset);
                tm_dhcp_next_option(bootOptionPtr, 1, byteOffset);
/* host name option Length */
                tm_dhcp_set_char(hostNameLength,
                                 bootOptionPtr, 0, byteOffset);
                tm_dhcp_next_option(bootOptionPtr, 1, byteOffset);  
/* host name */
                tm_dhcp_copy_list_to_pkt(
                    hostNamePtr,
                    bootOptionPtr,
                    (unsigned int)hostNameLength,
                    byteOffset, tempDspCount);

                tm_dhcp_next_option(bootOptionPtr,
                                (unsigned)hostNameLength,
                                byteOffset);
            }
        }
#ifdef TM_USE_DHCP_FQDN
        if (dhcpMsgEnd == TM_DHCPREQUEST_END)
/* Send FQDN option in all DHCP request messages */
        {
            fqdnFlags = btEntryPtr->btFqdnFlags;
            fqdnAllocated = TM_8BIT_NO;
            fqdnPtr = (tt8BitPtr)0;
            fqdnLen = 0;
            domainNameLength = 0;
            if (   tm_16bit_all_bits_set(btEntryPtr->btDevEntryPtr->devFlag2,
                                         TM_DEVF2_SRVR_HOST_NAME |
                                         TM_DEVF2_SRVR_DOMAIN_NAME)
                 && (btEntryPtr->btServerFqdnLen != 0) ) 
/* Use FQDN from server */
            {
                fqdnPtr = btEntryPtr->btServerAFqdn;
                fqdnLen = btEntryPtr->btServerFqdnLen; 
                fqdnFlags = TM_DHCPF_FQDN_ASCII | TM_DHCPF_FQDN_ENABLE;
            }
            else
            {
                if (    (tm_16bit_one_bit_set(
                                          btEntryPtr->btDevEntryPtr->devFlag2,
                                          TM_DEVF2_SRVR_DOMAIN_NAME))
/*
 * Build FQDN option using domain name as set by server
 */
                     && (btEntryPtr->btDomainName[0] != '\0')
                     && (hostNameLength != (tt8Bit)0)
                   )
                {
                    domainNameLength = (int)tm_btdhcp_name_length(btEntryPtr,
                                                           TM_BT_DOMAIN_INDEX);
                    allocLength = (unsigned int)hostNameLength
/* room for '.', and '\0': */
                                 + domainNameLength + 2;
                    if (allocLength > TM_FQDN_MAX_LEN + 1)
                    {
                        allocLength = TM_FQDN_MAX_LEN + 1;
                        if (hostNameLength >= TM_FQDN_MAX_LEN)
                        {
                            hostNameLength = TM_FQDN_MAX_LEN;
                            domainNameLength = 0;
                        }
                        else
                        {
                            domainNameLength = TM_FQDN_MAX_LEN - 1
                                              - (unsigned int)hostNameLength;
/* So the user notices the truncation */
                            btEntryPtr->btDomainName[domainNameLength] = '\0';
                            tm_btdhcp_name_length(btEntryPtr,
                           TM_BT_DOMAIN_INDEX) = (ttUser16Bit)domainNameLength;
                        }
                    }
                    fqdnPtr = (tt8BitPtr)tm_kernel_malloc(allocLength);
                    if (fqdnPtr != (tt8BitPtr)0)
                    {
                        fqdnAllocated = TM_8BIT_YES;
                        fqdnFlags = TM_DHCPF_FQDN_ASCII | TM_DHCPF_FQDN_ENABLE;
                        tm_bcopy(hostNamePtr, fqdnPtr,
                                 (unsigned int)hostNameLength);
                        if (hostNameLength != TM_FQDN_MAX_LEN)
                        {
                            fqdnPtr[(unsigned int)hostNameLength] = '.';
                            if (domainNameLength)
                            {
                                tm_bcopy(btEntryPtr->btDomainName,
                                  &fqdnPtr[(unsigned int)hostNameLength +1],
                                  (unsigned int)(domainNameLength));
                            }
                        }
                        fqdnLen = (int)(allocLength - 1);
                        fqdnPtr[fqdnLen] = '\0';
                    }
                }
            }
            if (fqdnFlags & TM_DHCPF_FQDN_ENABLE)
            {
/* 
 * Lock FQDN global structure 
 * (In case btUserFqdnPtr == tm_context(tvFqdnStruct).domainName))
 */
                tm_call_lock_wait(&tm_context(tvFqdnStructLock));
                if (fqdnLen == 0)
                {
                    if (btEntryPtr->btUserFqdnPtr ==
                                        tm_context(tvFqdnStruct).domainName)
                    {
/* Global FQDN */
                        fqdnLen = (int)(unsigned int)
                                        tm_context(tvFqdnStruct).domainNameLen;
                        fqdnFlags = (unsigned int)
                                        tm_context(tvFqdnStruct).domainFlags;
                    }
                    else
                    {
/* Interface FQDN */
                        fqdnLen = (int)(unsigned int)btEntryPtr->btUserFqdnLen;
                        fqdnFlags = (unsigned int)btEntryPtr->btFqdnFlags;
                    }
                    if (!(fqdnFlags & TM_DHCPF_FQDN_ASCII))
                    {
                        fqdnPtr = (tt8BitPtr)tm_kernel_malloc(
                                                   TM_FQDN_MAX_LEN +
                                                   TM_FQDN_DOMAINNAME_OFFSET);
                        if (fqdnPtr != (tt8BitPtr)0)
                        {
/*
 * tfHostnameTextToBinary has already been called before to check the
 * domain name is valid. If it fails now, it means the memory is corrupted.
 */
                            (void)tfHostnameTextToBinary(
                                    fqdnPtr,
                                    &fqdnLen,
                                    btEntryPtr->btUserFqdnPtr,
                                    fqdnLen,
                                    (int)fqdnFlags);
                            fqdnAllocated = TM_8BIT_YES;
                        }
                    }
                    else
                    {
                        fqdnPtr = (tt8BitPtr)btEntryPtr->btUserFqdnPtr;
                    }
                }
                if (fqdnPtr != (tt8BitPtr)0)
                {
/* FQDN option ID */
                    tm_dhcp_set_char(TM_FQDN_TAG, bootOptionPtr, 0, byteOffset);
                    tm_dhcp_next_option(bootOptionPtr, 1, byteOffset);
/* FQDN Option Length */
                    tm_dhcp_set_char((tt8Bit)(fqdnLen +
                                                TM_FQDN_DOMAINNAME_OFFSET),
                                     bootOptionPtr, 0, byteOffset);
                    tm_dhcp_next_option(bootOptionPtr, 1, byteOffset);  
/* FQDN option flags */
                    if (!(fqdnFlags & TM_DHCPF_FQDN_ASCII))
                    {
                        fqdnOptionFlags = (tt8Bit)TM_DHCP_FQDN_FLAG_E;
                    }
                    else
                    {
                        fqdnOptionFlags = TM_8BIT_ZERO;
                    }
                    if (fqdnFlags & TM_DHCPF_FQDN_FLAG_N_ONE)
/* RFC 4702: If N is 1, S must be 0 */
                    {
                        fqdnOptionFlags = (tt8Bit)
                                (fqdnOptionFlags | TM_DHCP_FQDN_FLAG_N);
                    }
                    else if (!(fqdnFlags & TM_DHCPF_FQDN_FLAG_S_ZERO))
                    {
                        fqdnOptionFlags = (tt8Bit)
                                (fqdnOptionFlags | TM_DHCP_FQDN_FLAG_S);
                    }
                    tm_dhcp_set_char(fqdnOptionFlags,
                                     bootOptionPtr, 0, byteOffset);
                    tm_dhcp_next_option(bootOptionPtr, 1, byteOffset);  
/* FQDN option Rcode 1 */
                    tm_dhcp_set_char(0,
                                    bootOptionPtr, 0, byteOffset);
                    tm_dhcp_next_option(bootOptionPtr, 1, byteOffset);  
/* FQDN option Rcode 2 */
                    tm_dhcp_set_char(0,
                                    bootOptionPtr, 0, byteOffset);
                    tm_dhcp_next_option(bootOptionPtr, 1, byteOffset);  
/* FQDN domain name */
                    tm_dhcp_byte_copy_to_pkt(fqdnPtr,
                                             bootOptionPtr,
                                             fqdnLen,
                                             byteOffset);
                    if (fqdnAllocated != TM_8BIT_NO)
                    {
                        tm_kernel_free(fqdnPtr);
                    }
                    tm_dhcp_next_option(bootOptionPtr,
                                        fqdnLen,
                                        byteOffset);
                }
/* UnLock FQDN global structure */
                tm_call_unlock(&tm_context(tvFqdnStructLock));
            }
        }
#endif /* TM_USE_DHCP_FQDN */
        if (    (dhcpMsgEnd == TM_DHCPREQUEST_END)
             || (dhcpMsgEnd == TM_DHCPDISCOVER_END) )
/* Send parameter list option in all discover and request messages */
        {
            listParam.gen8BitParm = TM_DHCPGET_PARAMETER_LIST;
/* allow user to override parameter request list */
            nodePtr = tfListWalk(
                    &(btEntryPtr->btUserSetCustOpts),
                    tfBtFindCustOpt,
                    listParam);
            if (nodePtr == TM_NODE_NULL_PTR)
/* User is not overwriting the parameter list */
            {
                tm_dhcp_copy_list_to_pkt(
                        tlDhcpParameterList, bootOptionPtr,
                        sizeof(tlDhcpParameterList),
                        byteOffset, tempDspCount);
#ifdef TM_ERROR_CHECKING
                if ( tlDhcpParameterList[1] != 
                                        (sizeof(tlDhcpParameterList) - 2) )
                {
                    tfKernelWarning("tfDhcpMsgSend", 
                      "tlDhcpParameterList size not initialized correctly\n");
/* Fix it */
                    tm_dhcp_set_char(
                            sizeof(tlDhcpParameterList) - 2,
                            bootOptionPtr, 1,
                            byteOffset);
                }
#endif /* TM_ERROR_CHECKING */
                tm_dhcp_next_option(
                        bootOptionPtr,
                        sizeof(tlDhcpParameterList),
                        byteOffset);
            }
        }

/* Store the current offset */
        pktData.bpdBootOptionPtr = bootOptionPtr;
#ifdef TM_DSP
        pktData.bpdByteOffset    = byteOffset;
#endif /* TM_DSP */
        pktData.bpdMsgEnd        = dhcpMsgEnd;
        listParam.genVoidParmPtr = &pktData;
/* Write all custom options to the packet */
        (void)tfListWalk(&(btEntryPtr->btUserSetCustOpts),
                         tfDhcpAddCustOpt,
                         listParam);
/* Get the new offset */
#ifdef TM_DSP
        byteOffset = pktData.bpdByteOffset;
#endif /* TM_DSP */
        bootOptionPtr = pktData.bpdBootOptionPtr;

        tm_dhcp_set_char(TM_BOOT_END_TAG, bootOptionPtr, 0, byteOffset);
        dhcpMsgLength =
            ((ttPktLen)(bootOptionPtr - packetPtr->pktLinkDataPtr)) + 1;
#ifdef TM_DSP
        dhcpMsgLength = tm_byte_count(dhcpMsgLength);
#else /* !TM_DSP */
        if (dhcpMsgLength & TM_ROUND_SIZE)
        {
            dhcpMsgLength = (ttPktLen)(  (dhcpMsgLength + TM_ROUND_SIZE)
                                       & TM_ROUND_MASK_SIZE );
        }
#endif /* !TM_DSP */
        if (dhcpMsgLength < (ttPktLen)300)
/*
 * Some DHCP servers will discard packets with DHCP sizes smaller than 300
 * bytes
 */
        {
            dhcpMsgLength = (ttPktLen)300;
        }
        packetPtr->pktChainDataLength = dhcpMsgLength; 
        packetPtr->pktLinkDataLength = dhcpMsgLength; 
        tm_debug_log2("tfDhcpMsgSend '%s', length %d\n", 
                      tfDhcpMsgStr(dhcpMsgEnd), (int)dhcpMsgLength);
#ifdef  TM_TRECK_DEMO
        if (dhcpMsgEnd == TM_DHCPRELEASE_END)
        {
/* Just to check that we do send a release message, upon exit */
            tfKernelWarning("tfDhcpMsgSend", "DHCP RELEASE");
        }
#endif /* TM_TRECK_DEMO */
/*
 * destination address is based on state. See above.
 */
        errorCode = tfBtSendPacket( packetPtr, destIpAddress );
    }
    else
    {
        errorCode = TM_ENOBUFS;
    }
    return errorCode;
}

/*
 * tfDhcpaInit function description:
 * Called in init state, when user wants to configure the interface,
 * or Called when we want to send the first DHCP request in REBOOTING state.
 *
 * Initialize the boot entry
 *
 * Parameters       Meaning
 * btEntryPtr       pointer to boot (for Dhcp) entry
 * packetPtr        null pointer
 *
 * Return
 * TM_ENOERROR
 */
static int tm_state_function (tfDhcpaInit, btEntryPtr, packetPtr)
{
    int errorCode;

    TM_UNREF_IN_ARG(packetPtr);
    if (btEntryPtr->btDhcpRestartTimerPtr == TM_TMR_NULL_PTR)
    {
/* Remove DHCP timers */
        tfDhcpRemoveTimers(btEntryPtr);
/* Close the socket */
        tfBtCloseSocket(btEntryPtr);
        (void)tfDhcpInit(btEntryPtr, TM_DHCP_START);
        errorCode = TM_ENOERROR;
    }
    else
/* Already started */
    {
        errorCode = TM_EALREADY;
    }
    return errorCode;
}

/*
 * tfDhcpaDelayStart function description:
 * Called in init state, when user wants to configure the interface, or
 * when a previous server offer was nacked by the server.
 * or Called when we want to send the first DHCP request in INIT_REBOOT state.
 * Try and delay sending the message with a random delay timer
 *
 * Parameters       Meaning
 * btEntryPtr       pointer to boot (for Dhcp) entry
 * packetPtr        null pointer
 *
 * Return
 * TM_EINPROGRESS   delayed timer started
 * Other            as returned by socket/bind function.
 */
static int tm_state_function (tfDhcpaDelayStart, btEntryPtr, packetPtr)
{
    ttGenericUnion  timerParm1;
    ttGenericUnion  timerParm2;
    tt32Bit         timeout;
    int             errorCode;

    TM_UNREF_IN_ARG(packetPtr);
    if (btEntryPtr->btDhcpRestartTimerPtr == TM_TMR_NULL_PTR)
    {
/*
 * open the socket now, so that the user gets the error back if we fail in
 * the TM_DHCPA_START case.
 * in the TM_DHCPA_RESTART case, we also want to open the socket now, so that
 * we stay in the init state if we fail to open the socket.
 */
        errorCode = tfBtOpenSocket(btEntryPtr);
        if (errorCode == TM_ENOERROR)
        {
            timeout = tfGetRandom();
            timeout %= TM_UL(9001);
            timeout += TM_UL(1000);
            timerParm1.genVoidParmPtr = (ttVoidPtr)btEntryPtr;
            timerParm2.gen8BitParm = TM_DHCPE_RTIMEOUT;
            btEntryPtr->btDhcpRestartTimerPtr =
                            tfTimerAddExt(&btEntryPtr->btDhcpRestartTimer,
                                          tfDhcpRestartTimer,
                                          (ttTmCUFuncPtr)0,
                                          timerParm1,
                                          timerParm2,
                                          timeout,
                                          0 );
            errorCode = TM_EINPROGRESS;
        }
    }
    else
    {
        errorCode = TM_EALREADY;
    }
    return errorCode;
}

/*
 * tfDhcpaDiscover function description:
 * Called in init state, when user wants to configure the interface, or
 * when a previous server offer was nacked by the server.
 * . Broadcast a DHCP discover message. Use tfDhcpMsgSend() routine.
 *
 * Parameters       Meaning
 * btEntryPtr       pointer to boot (for Dhcp) entry
 * packetPtr        null pointer
 *
 * Return
 * TM_ENOERROR      no error
 * TM_ENOBUFS       not enough buffer to send message.
 * Other            as returned by send function.
 */
static int tm_state_function (tfDhcpaDiscover, btEntryPtr, packetPtr)
{
    int errorCode;

    TM_UNREF_IN_ARG(packetPtr);
    errorCode = tfDhcpMsgSend(btEntryPtr, tfDhcpRetryTimer,
                              TM_DHCPDISCOVER_END);
    return errorCode;
}

/*
 * tfDhcpaRequest function description:
 * Send a DHCP request:
 *  . If we have not reached the bound state yet, use an exponential
 *    backoff timer.
 *  . Use common routine to send the DHCP message request.
 *
 * Parameters       Meaning
 * btEntryPtr       pointer to boot (for Dhcp) entry
 * packetPtr        null pointer
 *
 * Return
 * TM_ENOERROR      no error
 * TM_ENOBUFS       not enough buffer to send message.
 * Other            as returned by send function.
 */
static int tm_state_function (tfDhcpaRequest, btEntryPtr, packetPtr)
{
    ttTmCBFuncPtr   retryTimerFuncPtr;
    int             state;
    int             errorCode;

    TM_UNREF_IN_ARG(packetPtr);
    state = btEntryPtr->btDhcpState;
    if (state >= TM_DHCPS_BOUND)
    {
/* No retry timer */
        retryTimerFuncPtr = (ttTmCBFuncPtr)0;
    }
    else
    {
        retryTimerFuncPtr = tfDhcpRetryTimer;
    }
    tm_kernel_set_critical;
/* Time stamp when we send DHCP request */
    btEntryPtr->btDhcpRequestTimeStamp = tvTime;
    tm_kernel_release_critical;
    errorCode = tfDhcpMsgSend(btEntryPtr,
                              retryTimerFuncPtr,
                              TM_DHCPREQUEST_END );

    return errorCode;
}

/*
 * Send a DHCP request.
 * Called in the selecting state.
 */
int tfDhcpRequestOffer(ttBtEntryPtr btEntryPtr)
{
/* Remove the Discover/Collect retry timer */
    tfBtRemoveRetryTimer(btEntryPtr);
/* Get the state machine to send out a request for the selected entry */
    btEntryPtr->btDhcpState = TM_DHCPS_REQUESTING;
    return tfDhcpStateMachine(  btEntryPtr,
                                TM_PACKET_NULL_PTR,
                                TM_DHCPE_RETRY );
}

/*
 * tfDhcpaCollect function description:
 * Called when receiving a DHCP offer from the server, in selecting state.
 * . Retrieve offered IP address from the server.
 * . If offered IP address is not invalid,
 *     . remove the Discover retry timer,
 *     . DHCP state transition to REQUESTING
 *     . return no error to the state machine, to cause the state machine
 *       to send a DHCP request.
 * . If offered IP address is invalid, return TM_EINVAL to the state machine.
 *
 * Parameters       Meaning
 * btEntryPtr       pointer to boot (for Dhcp) entry
 * packetPtr        pointer to recvCB packet
 *
 * Return
 * TM_ENOERROR      no error, state machine can call tfDhcpaRequest
 * TM_EINVAL        Invalide offered address. Stop the state machine.
 */
static int tm_state_function(tfDhcpaCollect, btEntryPtr, packetPtr)
{
    ttDeviceEntryPtr    devPtr;
    ttBootHeaderPtr     bootHdrPtr;
    tt4IpAddress        yiaddr;
    int                 errorCode;
    tt8Bit              needScanOptions;
    tt8Bit              needSendRequest;
#ifdef TM_USE_DHCP_COLLECT
    ttUserDhcpOfferCBFuncPtr    userOfferFuncPtr;
    tt4IpAddress                serverId;
    ttGenericUnion              timerParm1;
#endif /* TM_USE_DHCP_COLLECT */

    needScanOptions = TM_8BIT_YES;
    needSendRequest = TM_8BIT_YES;
    devPtr = btEntryPtr->btDevEntryPtr;

/* Point to packet DHCP header */
    bootHdrPtr = (ttBootHeaderPtr)packetPtr->pktLinkDataPtr;
/* Retrieve the IP address that the server is offering */
    tm_ip_copy(bootHdrPtr->bootYiaddr, yiaddr);
    if (tm_ip_not_zero(yiaddr))
    {
#ifdef TM_USE_DHCP_COLLECT
/* Start the offer collection timeout timer upon receipt of the first
 * offer. */
        if (    tm_ip_zero(btEntryPtr->btYiaddr)
            &&  btEntryPtr->btOfferEntryList.listCount == 1 )
        {
/* Remove the Discover/Collect retry timer */
            tfBtRemoveRetryTimer(btEntryPtr);
/* Set a time limit for offer collection */
            timerParm1.genVoidParmPtr = (ttVoidPtr)btEntryPtr;
            btEntryPtr->btRetryTimerPtr = tfTimerAddExt(
                        &btEntryPtr->btRetryTimer,
                        tfDhcpCollectTimer,
                        (ttTmCUFuncPtr)0,
                        timerParm1,
                        timerParm1,   /* unused */
                        btEntryPtr->btCollectTime * TM_MSECS_PER_SECOND,
                        0 );
        }
        tm_ip_copy(yiaddr, btEntryPtr->btYiaddr);
        userOfferFuncPtr = devPtr->devOfferFuncPtr;
        if (userOfferFuncPtr != (ttUserDhcpOfferCBFuncPtr)0)
        {
/* Extract all the information from the server so the user can view it
 * in the callback function. */
            tfDhcpScanOptions(packetPtr, btEntryPtr, TM_DHCPA_CONFIGURE);
            needScanOptions = TM_8BIT_NO;
/* If tfDhcpScanOptions gave us a ServerId, we can remove any other cache
 * entries with the same ServerId. */
            if (btEntryPtr->btOfferEntryList.listCount > 1)
            {
                tm_ip_copy(btEntryPtr->btDhcpServerId, serverId);
                if (tm_ip_not_zero(serverId))
                {
                    tfDhcpDeleteOffers( btEntryPtr,
                                        serverId,
                                        TM_DHCP_OFFER_DELETE2 );
                }
            }
/* Unlock the device */
            tm_call_unlock(&(devPtr->devLockEntry));
/* Call the user to decide what to do with the current offer */
            errorCode = (*userOfferFuncPtr)(
                    (ttUserInterface)btEntryPtr->btDevEntryPtr,
                    (unsigned)btEntryPtr->btIndex,
                    TM_DHCP_EVENT_OFFER,
                    (ttUserIpAddress)btEntryPtr->btDhcpServerId,
                    (ttUserIpAddress)yiaddr,
                    devPtr->devOfferParam );
/* Lock the device */
            tm_call_lock_wait(&(devPtr->devLockEntry));
/* The user may have done something to change the state, like select
 * an arbitrary offer from the cache. */
            if (btEntryPtr->btDhcpState != TM_DHCPS_SELECTING)
            {
                needSendRequest = TM_8BIT_NO;
            }
            else
            {
                if (errorCode == TM_DHCP_OFFER_IGNORE)
                {
/* Clear the client's IP address to invalidate the cache entry */
                    tm_ip_copy(TM_IP_ZERO, btEntryPtr->btYiaddr);
                }
                else if (errorCode == TM_DHCP_OFFER_SELECT)
                {
/* User selected the current offer; move to the requesting state. */
                    btEntryPtr->btDhcpState = TM_DHCPS_REQUESTING;
                }
/* Note: We do nothing if the user returns TM_DHCP_OFFER_COLLECT since
 * the offer is already in the cache. */
            }
        }
        else if (tm_ip_not_zero(btEntryPtr->btPrefAddr))
        {
/* Check if the offered address matches the user's preferred address */
            if (tm_ip_match(yiaddr, btEntryPtr->btPrefAddr))
            {
/* The current offer is the one we have been waiting for; move to the
 * requesting state. */
                btEntryPtr->btDhcpState = TM_DHCPS_REQUESTING;
            }
        }
        else
/* Legacy mode . . . */
#endif /* TM_USE_DHCP_COLLECT */
        {
/* We use the first received offer; move to requesting state */
            btEntryPtr->btDhcpState = TM_DHCPS_REQUESTING;
        }
        if (needScanOptions)
        {
            tfDhcpScanOptions(packetPtr, btEntryPtr, TM_DHCPA_COLLECT);
            tm_ip_copy(yiaddr, btEntryPtr->btYiaddr);
        }
        if (needSendRequest && btEntryPtr->btDhcpState == TM_DHCPS_REQUESTING)
        {
/* Remove the Discover/Collect retry timer */
            tfBtRemoveRetryTimer(btEntryPtr);
/*
 * A no error return code, will cause the state machine to call
 * tfDhcpaRequest.
 */
            errorCode = TM_ENOERROR;
        }
        else
        {
/*
 * Either the user callback has changed state or we are remaining in the
 * TM_DHCPS_SELECTING state.
 * Set a non-zero error return code to prevent the state machine from
 * calling tfDhcpaRequest.
 */
            errorCode = -1;
        }
    }
    else
    {
/* Ignore invalid offered address */
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfDhcpCollect", "Invalid offered IP address");
#endif /* TM_ERROR_CHECKING */
        errorCode = TM_EINVAL;
    }
    return errorCode;
}

/*
 * tfDhcpaConfigure function description:
 * Called when receiving a DHCP ACK from the server, in requesting state.
 * . Remove the retry timer.
 * . Check that the IP address is not in use, by querying the ARP table.
 * . If not in use
 *   . Retrieve all options sent by the server, checking for overloaded
 *     options.
 *   . Configure the IP address.
 *   . If the configuration of the IP address did not fail
 *     return TM_ENOERROR to the state machine, so that lease times can
 *     be recorded, and T1 and T2 timers started.
 *   . If the configuration of the IP address failed, go back to the init
 *     state, and return errorCode to the state machine to stop any further
 *     processing.
 * . If in use, send a DECLINE message, go back to
 *   the init state, and send a discover message, return error to the state
 *   machine, to stop any further processing.
 *
 * Parameters       Meaning
 * btEntryPtr       pointer to boot (for Dhcp) entry
 * packetPtr        pointer to recvCB packet
 *
 * Return
 * TM_ENOERROR      no error, state machine can call tfDhcpaLease
 * TM_EINVAL        Invalid offered address. Stop the state machine.
 */
static int tm_state_function(tfDhcpaConfigure, btEntryPtr, packetPtr)
{
    ttBootHeaderPtr     bootHdrPtr;
    ttDeviceEntryPtr    devPtr;
    tt4IpAddress        bootYiaddr;
    ttUserGenericUnion  dhcpParam;
    int                 errorCode;

    bootHdrPtr = (ttBootHeaderPtr)packetPtr->pktLinkDataPtr;
/* Remove the Request retry timer */
    tfBtRemoveRetryTimer(btEntryPtr);
/* In case the server changed IP address on us */
    tm_ip_copy(bootHdrPtr->bootYiaddr, bootYiaddr);
    tm_ip_copy(bootYiaddr, btEntryPtr->btYiaddr);
/*
 * Extract all information from the server
 * We cannot assume that any field in the recvCB message is
 * aligned on a 4-byte boundary.
 */
    tfDhcpScanOptions(packetPtr, btEntryPtr, TM_DHCPA_CONFIGURE);
    devPtr = btEntryPtr->btDevEntryPtr;
    if (    (tm_4_ll_is_lan(devPtr))
         && (devPtr->devBtArpProbes >= 0) )
    {
        dhcpParam.genVoidParmPtr = (ttVoidPtr)btEntryPtr;
/*
 * Make sure that there is not a collision detection started on that IP 
 * address 
 */
        (void)tfCancelCollisionDetection(
#ifdef TM_USE_STRONG_ESL
                                          devPtr,
#endif /* TM_USE_STRONG_ESL */
                                          bootYiaddr);
/* Register the call back function for that IP address with the stack */
        errorCode = tfUseCollisionDetection(
#ifdef TM_USE_STRONG_ESL
                                             devPtr,
#endif /* TM_USE_STRONG_ESL */
                                             bootYiaddr,
                                             tfDhcpFinish, dhcpParam );
        if (errorCode != TM_ENOERROR)
        {
/* Found a collision in the ARP cache */
            tfDhcpDecline(btEntryPtr);
/* 
 * Do not finish the configuration below, and stop any further processing 
 * in the current state machine, because errorCode is non zero.
 */
        }
        else
        {
/* Start sending ARP probes for that IP address */
            errorCode = tfStartArpSend(
                    devPtr,
                    bootYiaddr,
                    devPtr->devBtArpProbes,
                    (tt32Bit)((tt32Bit)devPtr->devBtArpIntv * TM_UL(1000)),
                    (tt32Bit)((tt32Bit)devPtr->devBtArpTimeout * TM_UL(1000)));
            if (errorCode != TM_ENOERROR)
            {
/* We could not start sending probles. We will not check this. */
                (void)tfCancelCollisionDetection(
#ifdef TM_USE_STRONG_ESL
                                                  devPtr,
#endif /* TM_USE_STRONG_ESL */
                                                  bootYiaddr);
/* Finish the configuration below */
                errorCode = TM_ENOERROR;
            }
            else
            {
/* Stop any further processing in the current state machine */
/* Do not finish the configuration below */
/* Keep socket opened */
                errorCode = TM_ENOENT;
            }
        }
    }
    else
    {
/* Finish the configuration below */
        errorCode = TM_ENOERROR;
    }
/* configure IP address */
    if (errorCode == TM_ENOERROR)
    {
/* Finish configuration. Close the socket. */
        errorCode = tfBtFinishConfig(btEntryPtr, TM_ENOERROR);
        if (errorCode != TM_ENOERROR)
        {
/* The configuration failed. Stop DHCP and go back to the INIT state */
            tfDhcpBtStop(btEntryPtr, errorCode);
/* 
 * Stop any further processing in the current state machine, because errorCode
 * is non zero.
 */
        }
    }
/* If errorCode is zero, tfDhcpaNewLease will be called by the state machine */
    return errorCode;
}

static int tfDhcpFinish ( ttUserInterface    interfaceHandle,
                          ttUserIpAddress    ipAddress,
                          int                errorCode,
                          ttUserGenericUnion dhcpParam )
{
    ttBtEntryPtr     btEntryPtr;
    ttDeviceEntryPtr devPtr;

    (void)tfCancelCollisionDetection(
#ifdef TM_USE_STRONG_ESL
                                      interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                                      ipAddress);
    btEntryPtr = (ttBtEntryPtr)dhcpParam.genVoidParmPtr;
    devPtr = (ttDeviceEntryPtr)interfaceHandle;
/* Lock the device */
    if (devPtr != (ttDeviceEntryPtr)0)
    {
        tm_call_lock_wait(&(devPtr->devLockEntry));
    }
/* To let the user take a pick at the BOOT entry (tfConfGetBootEntry()) */
    if (errorCode == TM_ENOERROR)
    {
/* Enter the lease */
        (void)tfDhcpaNewLease(btEntryPtr, (ttPacketPtr)0);
/* Configure IP address. Close the socket. */
        errorCode = tfBtFinishConfig(btEntryPtr, TM_ENOERROR);
        if (errorCode != TM_ENOERROR)
        {
/* The configuration failed. Stop DHCP and go back to the INIT state */
            tfDhcpBtStop(btEntryPtr, errorCode);
        }
    }
    else
    {
        tfDhcpDecline(btEntryPtr);
    }
/* UnLock the device */
    if (devPtr != (ttDeviceEntryPtr)0)
    {
        tm_call_unlock(&(devPtr->devLockEntry));
    }
    return TM_ENOERROR;
}

/*
 * tfDhcpDecline function description
 */
static void tfDhcpDecline(ttBtEntryPtr btEntryPtr)
{
    ttGenericUnion  timerParm1;
    ttGenericUnion  timerParm2;
    int             errorCode;

#ifdef TM_ERROR_CHECKING
    tfKernelWarning("tfDhcpDecline", "Declining Acked IP address");
#endif /* TM_ERROR_CHECKING */
/*
 * Send a DECLINE message to the server (in requesting state, so that
 * the ciaddr field is 0, and that the message is broadcast (since
 * we are not configured)), and that the Requested IP option is set.
 */
    btEntryPtr->btDhcpState = TM_DHCPS_REQUESTING;
    (void)tfDhcpMsgSend( btEntryPtr, (ttTmCBFuncPtr)0,
                         TM_DHCPDECLINE_END );
/* Remove DHCP timers */
    tfDhcpRemoveTimers(btEntryPtr);
/* Close the socket */
    tfBtCloseSocket(btEntryPtr);
/* Discard previously discovered values */
    errorCode = tfDhcpInit(btEntryPtr, TM_DHCP_RESTART);
    if (    (errorCode == TM_ENOERROR)
         && (btEntryPtr->btDhcpRestartTimerPtr == TM_TMR_NULL_PTR) )
    {
/* Restart the whole process in 10 seconds */
        btEntryPtr->btDhcpState = TM_DHCPS_INIT;
        timerParm1.genVoidParmPtr = (ttVoidPtr)btEntryPtr;
        timerParm2.gen8BitParm = TM_DHCPE_START;
        btEntryPtr->btDhcpRestartTimerPtr =
                                tfTimerAddExt(&btEntryPtr->btDhcpRestartTimer,
                                              tfDhcpRestartTimer,
                                              (ttTmCUFuncPtr)0,
                                              timerParm1,
                                              timerParm2,
                                              TM_DHCP_RESTART_DELAY,
                                              0 );
    }
    return;
}

/*
 * One-shot timer to kick the DHCP state machine to send a
 * DHCP DISCOVER/REQUEST message after a delay.
 */
static void tfDhcpRestartTimer (ttVoidPtr      timerBlockPtr,
                                ttGenericUnion userParm1,
                                ttGenericUnion userParm2)
{
    ttBtEntryPtr btEntryPtr;
#ifdef TM_LOCK_NEEDED
    ttDeviceEntryPtr    devPtr;
#endif /* TM_LOCK_NEEDED */

/* 
 * Send a Discover message.
 */
    btEntryPtr = (ttBtEntryPtr)userParm1.genVoidParmPtr;
#ifdef TM_LOCK_NEEDED
    devPtr = btEntryPtr->btDevEntryPtr;
#endif /* TM_LOCK_NEEDED */
    tm_call_lock_wait(&(devPtr->devLockEntry));
    if (tm_timer_not_reinit((ttTimerPtr)timerBlockPtr))
    {
        btEntryPtr->btDhcpRestartTimerPtr = (ttTimerPtr)0;
        (void)tfDhcpStateMachine( btEntryPtr,
                                  TM_PACKET_NULL_PTR,
                                  userParm2.gen8BitParm);
    }
    tm_call_unlock(&(devPtr->devLockEntry));
}

/*
 * tfDhcpaNewLease function description:
 * Called when receiving a DHCP ACK from the server, in BOUND, Renewing,
 * or rebinding state.
 *   . Retrieve all lease/T1/T2 options sent by the server, checking for
 *     overloaded options. Convert from seconds to milliseconds. Remove
 *     time elapsed since last Request was sent.
 *   . If Lease Time is not infinite, and T1, T2 and Lease time seem valid,
 *     start or update lease timer so that it expires in T1.
 *
 * Parameters       Meaning
 * btEntryPtr       pointer to boot (for Dhcp) entry
 * packetPtr        pointer to recvCB packet
 *
 * Return
 * TM_ENOERROR      no error
 */
static int tm_state_function(tfDhcpaNewLease, btEntryPtr, packetPtr)
{
/* point to packet DHCP header */
    ttS32Bit            timeElapsed;
    ttGenericUnion      timerParm1;
    int                 errorCode;
 
    TM_UNREF_IN_ARG(packetPtr);
    errorCode = TM_ENOERROR;

    tm_kernel_set_critical;
/* We stored tvTime when we sent DHCP request */
    timeElapsed = (ttS32Bit)(tvTime - btEntryPtr->btDhcpRequestTimeStamp);
    tm_kernel_release_critical;

/*
 * Lease, T1 and T2 sanity and default processing is designed to work
 *      with any lease time.  We don't want to be too picky.
 *      We would rather work with a DHCP server if at all possible.
 *
 *      RFC1541 specified a minimum lease time of 60 seconds. RFC2141 removed
 *      that requirement, i.e. there is no mimimum lease requirement period.
 *      But the RFC also says: 
 *      "T1 MUST be earlier than T2, which, in turn, MUST be earlier than the 
 *       time at which the client's lease will expire." So it would make
 *       sense to enforce a minimum lease time of 8 seconds, if the DHCP
 *       server sets the lease time to zero.
 *
 *      A maximum lease time of 49 days is enforced because we convert it to
 *      a 32 bit millisecond value for use with our timers.  Don't shorten
 *      an infinite lease! We also enforce a minimum of 60 seconds so the 
 *      default T1 and T2 values make sense all though the value could be as
 *      small as 8 seconds without a problem.
 *
 *       Use default T1 (1/2 of lease time) and T2 (7/8 of lease time) as 
 *       specified by the RFC 2131 if any of the following conditions exist:
 * 
 *         T1 or T2 is not specified or
 *         T1 or T2 is greater than lease time or
 *         T1 is greater than T2
 *
 *       All lease time, T1 and T2 processing is done in seconds before 
 *       converting to milliseconds to avoid overflow in compute 7/8ths of the 
 *       lease time.
 */
  
    if (tm_8bit_one_bit_set(btEntryPtr->btLeaseFlags, TM_BTF_LEASE_TIME))
    {
/* Lease time is usable */
        if (btEntryPtr->btDhcpLeaseTime != TM_DHCPT_INF_LEASE_TIME)
        {
/* Enforce maximum lease time. */
            if (btEntryPtr->btDhcpLeaseTime > TM_DHCP_MAX_LEASE_TIME_SECONDS)
            {
               btEntryPtr->btDhcpLeaseTime = TM_DHCP_MAX_LEASE_TIME_SECONDS;
            }

/* Enforce minmum lease time, even for a zero lease time. */
            if (btEntryPtr->btDhcpLeaseTime < TM_DHCP_MIN_LEASE_TIME_SECONDS)
            {
/* If the DHCP server sets the lease time to zero, execute that code as well */
                btEntryPtr->btDhcpLeaseTime = TM_DHCP_MIN_LEASE_TIME_SECONDS;
            }
       
            if (!tm_8bit_one_bit_set(btEntryPtr->btLeaseFlags, TM_BTF_T1) ||
                !tm_8bit_one_bit_set(btEntryPtr->btLeaseFlags, TM_BTF_T2) ||
                btEntryPtr->btDhcpT1 > btEntryPtr->btDhcpLeaseTime ||
                btEntryPtr->btDhcpT2 > btEntryPtr->btDhcpLeaseTime ||
                btEntryPtr->btDhcpT1 > btEntryPtr->btDhcpT2 )
            {
/* Use RFC 2131 defaults */
                btEntryPtr->btDhcpT1 = btEntryPtr->btDhcpLeaseTime >> 1 ;
                btEntryPtr->btDhcpT2 = (btEntryPtr->btDhcpLeaseTime * 7) >> 3;
            }
    
/* 
 * Convert T1, T2 and lease time to milliseconds and remove the already elapsed
 * time since the request was sent. Don't go negative. 
 */
            if (btEntryPtr->btDhcpT1*TM_UL(1000) < (tt32Bit)timeElapsed)
            {
                btEntryPtr->btDhcpT1 = 0;
            }
            else
            {
                btEntryPtr->btDhcpT1 = 
                        btEntryPtr->btDhcpT1*TM_UL(1000) - (tt32Bit)timeElapsed;
            }
            if (btEntryPtr->btDhcpT2*TM_UL(1000) < (tt32Bit)timeElapsed)
            {
                btEntryPtr->btDhcpT2 = 0;
            }
            else
            {
                btEntryPtr->btDhcpT2 = 
                         btEntryPtr->btDhcpT2*TM_UL(1000) - (tt32Bit)timeElapsed;
            }
            if (btEntryPtr->btDhcpLeaseTime*TM_UL(1000) < (tt32Bit)timeElapsed)
            {
                btEntryPtr->btDhcpLeaseTime = 0;
            }
            else
            {
                btEntryPtr->btDhcpLeaseTime = 
                 btEntryPtr->btDhcpLeaseTime*TM_UL(1000) - (tt32Bit)timeElapsed;
            }
            if (btEntryPtr->btDhcpRenewTimerPtr != TM_TMR_NULL_PTR)
            {
                tm_timer_remove(btEntryPtr->btDhcpRenewTimerPtr);
                btEntryPtr->btDhcpRenewTimerPtr = TM_TMR_NULL_PTR;
            }
            if ( btEntryPtr->btDhcpLeaseTimerPtr == TM_TMR_NULL_PTR )
            {
/* Add lease timer */
                timerParm1.genVoidParmPtr = (ttVoidPtr)btEntryPtr;
                btEntryPtr->btDhcpLeaseTimerPtr =
                                    tfTimerAddExt(&btEntryPtr->btDhcpLeaseTimer,
                                                  tfDhcpLeaseTimer,
                                                  (ttTmCUFuncPtr)0,
                                                  timerParm1,
                                                  timerParm1, /* unused */
                                                  btEntryPtr->btDhcpT1,
                                                  TM_TIM_AUTO );
            }
            else
            {
/* If lease timer has already been added, update its timeout */
                tm_timer_new_time(btEntryPtr->btDhcpLeaseTimerPtr,
                                  btEntryPtr->btDhcpT1);
            }
       }
    }
/* Reset flags for next lease renewal */
    tm_8bit_clr_bit( btEntryPtr->btLeaseFlags,
                                (TM_BTF_LEASE_TIME |TM_BTF_T1 | TM_BTF_T2));
    return errorCode;
}

static int tm_state_function(tfDhcpaLease, btEntryPtr, packetPtr)
{
/* point to packet DHCP header */
    int                 errorCode;
 
    tfDhcpScanOptions(packetPtr, btEntryPtr, TM_DHCPA_LEASE);
    errorCode = tfDhcpaNewLease(btEntryPtr, packetPtr);
/* Lease renewed */
    btEntryPtr->btLeaseFlags |= TM_BTF_LEASE;
/* Notify user of lease renewal. Close socket */
    (void)tfBtFinishConfig(btEntryPtr, TM_ENOERROR);
/* Copy new default router */
    tm_ip_copy(btEntryPtr->btLeaseDefRouter, btEntryPtr->btDefRouter);
/* Reset lease default router */
    tm_ip_copy(TM_IP_ZERO, btEntryPtr->btLeaseDefRouter);
/* Reset flags for next lease renewal */
    btEntryPtr->btLeaseFlags = (tt8Bit)0;
    return errorCode;
}

/*
 * tfDhcpaRestart function description
 * Called when receiving a NAK in selecting state, or rebooting state.
 * . Remove retry timer
 * . Return TM_ENOERROR to the state machine, to allow call to
 *   tfDhcpaDiscover
 * Parameters       Meaning
 * btEntryPtr       pointer to boot (for Dhcp) entry
 * packetPtr        pointer to recvCB packet
 *
 * Return
 * TM_ENOERROR      no error, state machine can call tfDhcpaDiscover
 */
static int tm_state_function(tfDhcpaRestart, btEntryPtr, packetPtr)
{
    int errorCode;

    TM_UNREF_IN_ARG(packetPtr);

/* Remove DHCP timers */
    tfDhcpRemoveTimers(btEntryPtr);
/* Notify the user we received a NAK. Close the socket */
    (void)tfBtFinishConfig(btEntryPtr, TM_EPERM);
/* Discard previously discovered values */
    errorCode = tfDhcpInit(btEntryPtr, TM_DHCP_RESTART);
/*
 * return no error, will cause the state machine to call
 * tfDhcpaDelayStart from init state
 */
    return errorCode;
}

/*
 * tfDhcpaHalt function description
 * Called when receiving a NAK in renewing state, or rebinding state
 * or when lease expires in rebinding state.
 * . Remove lease timer.
 * . unconfigure the interface 
 * . Notify the user of the error.
 * . Return TM_ENOERROR
 * Parameters       Meaning
 * btEntryPtr       pointer to boot (for Dhcp) entry
 * packetPtr        pointer to recvCB packet
 *
 * Return
 * TM_ENOERROR      no error
 */
static int tm_state_function(tfDhcpaHalt, btEntryPtr, packetPtr)
{
    int errorCode;

/* Remove DHCP timers */
    tfDhcpRemoveTimers(btEntryPtr);
/* Unconfigure the interface */
    if (btEntryPtr->btType == TM_BT_CONF)
    {
        (void)tfRemoveInterface(
            btEntryPtr->btDevEntryPtr, btEntryPtr->btIndex, PF_INET);
    }
/* Let the user know about it */
/* If we have a NULL packet pointer, we timed out */
    if (packetPtr == TM_PACKET_NULL_PTR)
    {
        errorCode = TM_ETIMEDOUT;
    }
/* otherwise we received a NAK */
    else
    {
        errorCode = TM_EPERM;
    }
/* Notify user of the eror. Close the socket. */
    (void)tfBtFinishConfig(btEntryPtr, errorCode);
/*
 * Discard previously discovered values. Re-initialize user entry.
 */
    tfBtInitUserEntry(btEntryPtr);
    return TM_ENOERROR;
}

/*
 * tfDhcpaRcvInvalid function description
 * Called when receiving an invalid packet from the network for the
 * current DHCP state.
 * Give warning to the user.
 *
 * Parameters       Meaning
 * btEntryPtr       pointer to boot (for Dhcp) entry
 * packetPtr        pointer to recvCB packet
 *
 * Return
 * TM_ENOERROR      no error.
 */
static int tm_state_function(tfDhcpaRcvInvalid, btEntryPtr, packetPtr)
{

    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(btEntryPtr);
#ifdef TM_ERROR_CHECKING
    tfKernelWarning("tfDhcpaRcvInvalid", "DHCP invalid message for state");
#endif /* TM_ERROR_CHECKING */
    return TM_ENOERROR;
}

/*
 * tfDhcpaTimeout function description
 * Called when retry timer expires (in SELECTING/REQUESTING/REBOOTING state)
 * . Move back to init state.
 * . Reset retry timer pointer
 * . Let the user know that tfConfigInterface timed out.
 * Parameters       Meaning
 * btEntryPtr       pointer to boot (for Dhcp) entry
 * packetPtr        pointer to recvCB packet
 *
 * Return
 * TM_ENOERROR      no error.
 */
static int tm_state_function(tfDhcpaTimeout, btEntryPtr, packetPtr)
{

    TM_UNREF_IN_ARG(packetPtr);
/* Back to init state */
    btEntryPtr->btDhcpState = TM_DHCPS_INIT;
    tfDhcpRemoveTimers(btEntryPtr);
/* Let the user know about the timeout. Close the socket */
    (void)tfBtFinishConfig(btEntryPtr, TM_ETIMEDOUT);
    return TM_ENOERROR;
}

/*
 * Scan the options, and copy the options appropriate for the action, and
 * state. Look up all options, even the overloaded ones. We do not
 * assume any order in the options. This is why we have to scan all the
 * options to make sure we got the ones we need.
 * . If action is configure (TM_DHCPA_CONFIGURE), copy all the
 *   configuration parameters in the boot structure.
 * . If action is lease, copy all the lease parameters in the boot structure.
 * . If we just received the message (action == TM_DHCPA_LAST_INDEX),
 *   check on msgType, and serverID; If OK, call the state machine with
 *   the msgType mapped to event.
 */
static void tfDhcpScanOptions ( ttPacketPtr     packetPtr,
                                ttBtEntryPtr    btEntryPtr,
                                int             action )
{
    ttBootHeaderPtr     bootHdrPtr;
    tt8BitPtr           optionPtr;
    tt8BitPtr           lastOptionPtr;
    ttBtUserCustOptPtr  custOptPtr;
    tt32Bit             leaseTime;
    tt4IpAddress        dhcpServerId;
    tt4IpAddress        defRouter;
    int                 optionLength;
    int                 nameLength;
    tt16Bit             bootFileSize;
    tt8Bit              dhcpMsgType;
    tt8Bit              dhcpOverload;
    tt8Bit              dhcpFileOverloaded;
    tt8Bit              dhcpSnameOverloaded;
    tt8Bit              optionName;
    int                 idx;
    int                 localAction;
    int                 index;
    int                 offset;
#ifdef TM_DSP
    unsigned int        byteOffset;
#endif /* TM_DSP */
#ifdef TM_USE_DHCP_FQDN
    int                 retCode;
    int                 i;
#endif /* TM_USE_DHCP_FQDN */
#ifdef TM_USE_DHCP_COLLECT
    ttBtOfferEntryPtr   offerPtr;
#endif /* TM_USE_DHCP_COLLECT */
    tt8Bit              hasLeaseOption;

    bootFileSize = (tt16Bit)0;
    leaseTime = (tt32Bit)0;
    hasLeaseOption = TM_8BIT_NO;
    bootHdrPtr = (ttBootHeaderPtr)packetPtr->pktLinkDataPtr;
    localAction = action;
    switch (action)
    {
        case TM_DHCPA_LEASE:
        case TM_DHCPA_CONFIGURE:
/* Common code for LEASE, CONFIGURE */
            localAction = TM_DHCPA_CONFIGURE_LEASE;
/* Zero out the DNS server fields */
            btEntryPtr->btDns1ServerIpAddress = (tt32Bit)0;
            btEntryPtr->btDns2ServerIpAddress = (tt32Bit)0;
            /* Fallthrough */
        case TM_DHCPA_COLLECT:
/* Common code for LEASE, CONFIGURE, and COLLECT */
/* Discard previously discovered values */
            tfBtDiscardServerNames(btEntryPtr);
            break;
#ifdef TM_USE_DHCP_COLLECT
        case TM_DHCPA_LAST_INDEX:
/*
 * We received a new offer.
 * We can't save anything in the user boot entry until we get the proper
 * entry set up as the current entry. Otherwise, we might corrupt an offer
 * that was previously received.
 * If we are collecting DHCP server offers and we are caching more than one
 * offer, we need to allocate a new cache entry and select it as the current
 * entry (head of list) unless:
 *  1) The currently selected entry is available (has no client address), or
 *  2) The cache is already full.
 * In either case, we just use (reuse) the current entry.
 */
            if (    btEntryPtr->btDhcpState == TM_DHCPS_SELECTING
                &&  tm_ip_not_zero(btEntryPtr->btYiaddr)
                &&  btEntryPtr->btOfferEntryList.listCount
                                            < btEntryPtr->btCollectSize )
            {
/* If the allocation fails, just use (reuse) the current entry. */
                offerPtr = tm_kernel_malloc(sizeof(ttBtOfferEntry));
                if (offerPtr != (ttBtOfferEntryPtr)0)
                {
/* Zero the block */
                    tm_bzero(offerPtr, sizeof(ttBtOfferEntry));
/* Link it into the cache at the head of the list (the current item) */
                    tfListAddToHead(&btEntryPtr->btOfferEntryList,
                                                    (ttNodePtr)offerPtr);
                    btEntryPtr->btOfferEntryPtr = offerPtr;
                    offerPtr->btEntryPtr = btEntryPtr;
/* Initialize the block */
                    tfListInit(&(offerPtr->btUserServCustOpts));
                    tfBtInitUserEntryNames(btEntryPtr);
                    tfDhcpInit(btEntryPtr, TM_DHCP_NEWOFFER);
                }
            }
            break;
#endif /* TM_USE_DHCP_COLLECT */
        default:
            break;
    }
/*
 * Scan the option for DHCP message, and server ID. We cannot assume that
 * the recvCB message TAGS are word aligned.
 */
    optionPtr = bootHdrPtr->bootOptions.optFields;
/* Allow for options bigger than 312 bytes */
    lastOptionPtr =   ((tt8BitPtr)bootHdrPtr)
                    + tm_packed_byte_count(packetPtr->pktChainDataLength - 1);
/* Assume no server id */
    tm_ip_copy(TM_IP_ZERO, dhcpServerId);
/* assume no overload */
    dhcpFileOverloaded = TM_8BIT_ZERO;
    dhcpSnameOverloaded = TM_8BIT_ZERO;
    dhcpOverload = TM_8BIT_ZERO;
    dhcpMsgType = TM_8BIT_ZERO;
    do
    {
        if (dhcpOverload & TM_DHCP_OVERLOAD_FILE)
        {
/* Server overloads file field first */
            dhcpFileOverloaded = TM_8BIT_YES;
            tm_8bit_clr_bit( dhcpOverload, TM_DHCP_OVERLOAD_FILE);
            optionPtr = (tt8BitPtr)&bootHdrPtr->bootFile[0];
            lastOptionPtr =
                  (tt8BitPtr)&bootHdrPtr->bootFile[
                      tm_packed_byte_count(TM_BOOTFILENAME_SIZE)-1];
/* Can't just use bootFile[TM_BOOTFILENAME_SIZE] because this generates
   compiler warning about out of range subscript. */
            lastOptionPtr++;
        }
        else
        {
/* Server overloads sname field second */
            if (dhcpOverload & TM_DHCP_OVERLOAD_SNAME)
            {
                dhcpSnameOverloaded = TM_8BIT_YES;
                tm_8bit_clr_bit( dhcpOverload, TM_DHCP_OVERLOAD_SNAME);
                optionPtr = (tt8BitPtr)&bootHdrPtr->bootSname[0];
                lastOptionPtr =
                      (tt8BitPtr)&bootHdrPtr->bootSname[
                          tm_packed_byte_count(TM_BOOTSNAME_SIZE)-1];
/* Can't just use bootFile[TM_BOOTSNAME_SIZE] because this generates
   compiler warning about out of range subscript. */
                lastOptionPtr++;
            }
        }
#ifdef TM_DSP
        byteOffset = 0;
#endif /* TM_DSP */
        while (    (tm_dhcp_get_char(optionPtr,0,byteOffset) != TM_BOOT_END_TAG)
                && (optionPtr <= lastOptionPtr) )
        {
            if ( tm_dhcp_get_char(optionPtr, 0, byteOffset) == TM_BOOT_PAD_TAG)
            {
/* Fixed length field */
                tm_dhcp_next_option(optionPtr, 1, byteOffset);                
            }
            else
            {
/* variable length data */
                optionLength = tm_dhcp_get_opt_len( optionPtr,
                                                    TM_BOOT_TAGCODE_SIZE,
                                                    byteOffset );

                optionName = tm_dhcp_get_char(optionPtr, 0, byteOffset);
                switch (optionName)
                {
                    case TM_DHCP_MSG_TYPE:
                        if (    (localAction == TM_DHCPA_LAST_INDEX)
                             && (optionLength == 1) )
                        {
                            dhcpMsgType = tm_dhcp_get_char( optionPtr,
                                                            TM_BOOT_TAG_OFFSET,
                                                            byteOffset );
                        }
/* point to next tag */
                        break;
                    case TM_DHCP_OVERLOAD:
                        if (optionLength == 1)
                        {
                            dhcpOverload = tm_dhcp_get_char( optionPtr,
                                                             TM_BOOT_TAG_OFFSET,
                                                             byteOffset );
                        }
/* point to next tag */
                        break;
                    case TM_DHCP_SERVER_ID:
/* Option Length should be 4 */
                        if (optionLength == TM_IP_ADDRESS_LENGTH)
                        {
                            tm_dhcp_long_copy( optionPtr,
                                               dhcpServerId,
                                               TM_BOOT_TAG_OFFSET,
                                               byteOffset );
                            if (    (    btEntryPtr->btDhcpState
                                      == TM_DHCPS_SELECTING)
                                 || (    btEntryPtr->btDhcpState
                                      == TM_DHCPS_REBINDING )
                                 || (action == TM_DHCPA_CONFIGURE)
                               )
                            {
/* Copy only if we are in the selecting phase, or the rebinding phase or configuring */
                                tm_ip_copy( dhcpServerId,
                                            btEntryPtr->btDhcpServerId );
                            }
#ifndef TM_OPTIMIZE_SIZE
/* This is not really necessary. Avoid extra CPU cycles */
                            if (    (action == TM_DHCPA_LAST_INDEX)
                                 && (dhcpMsgType != TM_8BIT_ZERO)
                                 && hasLeaseOption )
                            {
/* We just need the server ID and lease option, so exit the 2 loops */
                                goto scanOptionsExit; /* out of 2 loops */
                            }
#endif /* !TM_OPTIMIZE_SIZE */
                        }
                        break;
                    case TM_DHCP_LEASE_TIME:
                        if (optionLength == 4)
                        {
                            hasLeaseOption = TM_8BIT_YES;
#ifndef TM_OPTIMIZE_SIZE
/* This is not really necessary. Avoid extra CPU cycles */
                            if (    (action == TM_DHCPA_LAST_INDEX)
                                 && (dhcpMsgType != TM_8BIT_ZERO)
                                 && tm_ip_not_zero(dhcpServerId) )
                            {
/* We just need the server ID and lease option, so exit the 2 loops */
                                goto scanOptionsExit; /* out of 2 loops */
                            }
#endif /* !TM_OPTIMIZE_SIZE */
                            if (localAction == TM_DHCPA_CONFIGURE_LEASE)
                            {
/* Cannot assume 4-byte boundary */
                                tm_dhcp_long_copy( optionPtr,
                                                   leaseTime,
                                                   TM_BOOT_TAG_OFFSET,
                                                   byteOffset );
                                btEntryPtr->btLeaseFlags |= TM_BTF_LEASE_TIME;
                                tm_htonl(leaseTime,
                                         btEntryPtr->btDhcpLeaseTime);
                            }
                        }
                        break;
                    case TM_DHCP_RENEWAL_TIME:
                        if (    (localAction == TM_DHCPA_CONFIGURE_LEASE) 
                             && (optionLength == 4) )
                        {
/* Cannot assume 4-byte boundary */
                            tm_dhcp_long_copy( optionPtr,
                                               leaseTime,
                                               TM_BOOT_TAG_OFFSET,
                                               byteOffset );
                            btEntryPtr->btLeaseFlags |= TM_BTF_T1;
                            tm_htonl(leaseTime, btEntryPtr->btDhcpT1);
                        }
                        break;
                    case TM_DHCP_REBINDING_TIME:
                        if (    (localAction == TM_DHCPA_CONFIGURE_LEASE) 
                             && (optionLength == 4) )
                        {
/* Cannot assume 4-byte boundary */
                            tm_dhcp_long_copy( optionPtr, 
/* Cannot assume 4-byte boundary */
                                               leaseTime,
                                               TM_BOOT_TAG_OFFSET,
                                               byteOffset );
                            btEntryPtr->btLeaseFlags |= TM_BTF_T2;
                            tm_htonl(leaseTime, btEntryPtr->btDhcpT2);
                        }
                        break;
/* TFTP server name (for TFTP download) */
                    case TM_DHCP_TFTPS_NAME:
                        /* fallthrough */
/* TFTP boot file name (for TFTP download) */
                    case TM_DHCP_BOOTF_NAME:
                         if (localAction == TM_DHCPA_COLLECT)
                         {
/* not interested in copying these options when collecting */
                             break;
                         }
/* Host name provided by server */
                    case TM_DHCP_HOST_NAME:
                        /* fallthrough */
                    case TM_DOMAINNAME_TAG: /* Domain Name */
                        /* fallthrough */
#ifdef TM_USE_DHCP_FQDN
/* FQDN name provided by server */
                    case TM_FQDN_TAG:
#endif /* TM_USE_DHCP_FQDN */
                        if ( (    (localAction == TM_DHCPA_CONFIGURE_LEASE)
                               || (localAction == TM_DHCPA_COLLECT) ) )
                        {
                            offset = TM_BOOT_TAG_OFFSET;
                            nameLength = optionLength;
                            switch (optionName)
                            {
                                case TM_DHCP_TFTPS_NAME:
                                    index = TM_BT_SNAME_INDEX;
                                    dhcpSnameOverloaded = TM_8BIT_YES;
                                    break;
                                case TM_DHCP_BOOTF_NAME:
                                    dhcpFileOverloaded = TM_8BIT_YES;
                                    index = TM_BT_FNAME_INDEX;
                                    break;
                                case TM_DOMAINNAME_TAG: /* Domain Name */
                                    index = TM_BT_DOMAIN_INDEX;
                                    break;
                                case TM_DHCP_HOST_NAME:
                                    index = TM_BT_RXHNAME_INDEX;
                                    break;
#ifdef TM_USE_DHCP_FQDN
                                case TM_FQDN_TAG:
/* Temporary index */
                                    for (i = 0; i < 3; i++)
                                    {
                                        if (nameLength >= 1)
                                        {
                                            if (btEntryPtr->btOfferEntryPtr
                                                    ->btUserEntry.
                                                    btuFqdnRcvOffset == i)
                                            {
                                                 btEntryPtr->btOfferEntryPtr
                                                     ->btUserEntry.
                                                     btuFqdnRcvOffset++; 
                                                 nameLength--;
                                                 btEntryPtr->btOfferEntryPtr
                                                     ->btUserEntry.
                                                     btuFqdnArray[i] = 
                                                    tm_dhcp_get_char(
                                                                  optionPtr,
                                                                  offset,
                                                                  byteOffset);
                                                 offset++;
                                            }
                                         }
                                    }
                                    if ((btEntryPtr->btOfferEntryPtr
                                            ->btUserEntry.btuFqdnOptionFlags
                                                & TM_DHCP_FQDN_FLAG_E))
                                    {
/* Binary */
                                        index = TM_BT_FQDN_BINDEX;
                                    }
                                    else
                                    {
/* Ascii */
                                        index = TM_BT_FQDN_AINDEX;
                                    }
                                    if (nameLength != 0)
                                    {
                                        btEntryPtr->btFqdnStatus 
                                                        |= TM_FQDNF_NAMEREPLY;
                                    }
                                    break;
#endif /* TM_USE_DHCP_FQDN */
                                default:
                                    index = 0; /* not reached */
                                    break;

                            }
                            tfBtBootCopy(  btEntryPtr
                                         , optionPtr
                                         , offset
#ifdef TM_DSP
                                           + byteOffset
#endif /* !TM_DSP */
                                         , nameLength
/* maxSize: includes offset */
                                         , (int)(ttUserPtrCastToInt)
                                                 (lastOptionPtr - optionPtr)
                                         , index);
                        }
                        break;
/* TFTP boot file size (for TFTP download) */
                    case TM_BOOTFILESIZE_TAG:
/* Copy boot file size from &optionPtr[TM_BOOT_TAG_OFFSET] */
                        if (    (localAction == TM_DHCPA_CONFIGURE_LEASE)
                             && (optionLength == 2) )
                        {
                            tm_dhcp_short_copy( optionPtr,
                                                bootFileSize,
                                                TM_BOOT_TAG_OFFSET,
                                                byteOffset );
                            btEntryPtr->btBootFileSize = ntohs(bootFileSize);
                        }
                        break;
                    case TM_DOMAINSERVER_TAG: /* Domain Name Servers (BIND) */
/* copy first 2 domain name servers from &optionPtr[TM_BOOT_TAG_OFFSET] */
                        if (localAction == TM_DHCPA_CONFIGURE_LEASE) 
                        {
                            if ( optionLength >= TM_IP_ADDRESS_LENGTH )
                            {
                                tm_dhcp_long_copy(
                                    optionPtr,
                                    btEntryPtr->btDns1ServerIpAddress,
                                    TM_BOOT_TAG_OFFSET,
                                    byteOffset );
                            }
                            if ( optionLength >= (2 * TM_IP_ADDRESS_LENGTH) )
                            {
                                tm_dhcp_long_copy(
                                    optionPtr,
                                    btEntryPtr->btDns2ServerIpAddress,
                                    TM_BOOT_TAG_OFFSET+TM_IP_ADDRESS_LENGTH,
                                    byteOffset );
                            }
                        }
                        break;
                    case TM_SUBNETMASK_TAG: /* Subnet Mask */
/* 
 * Length should be 4 bytes. Check only when configuring. Ignore when renewing
 * the lease. Copy subnet mask from &optionPtr[TM_BOOT_TAG_OFFSET]
 */
                        if (    (action == TM_DHCPA_CONFIGURE)
                             && (optionLength == TM_IP_ADDRESS_LENGTH) )
                        {
                            tm_dhcp_long_copy( optionPtr,
                                               btEntryPtr->btNetMask,
                                               TM_BOOT_TAG_OFFSET,
                                               byteOffset );
                        }
                        break;
                    case TM_ROUTER_TAG: /* routers */
/* copy routing entries, set flag */
                        if (    (localAction == TM_DHCPA_CONFIGURE_LEASE)
                             && (optionLength >= TM_IP_ADDRESS_LENGTH) )
                        {
                            defRouter = TM_IP_ZERO; /* compiler warning */
                            tm_dhcp_long_copy( optionPtr,
                                               defRouter,
                                               TM_BOOT_TAG_OFFSET,
                                               byteOffset );
                            if (action == TM_DHCPA_LEASE)
                            {
                                tm_ip_copy(defRouter,
                                           btEntryPtr->btLeaseDefRouter);
                            }
                            else
                            {
                                tm_ip_copy(defRouter, btEntryPtr->btDefRouter);
                                tm_ip_copy(defRouter,
                                                btEntryPtr->btLeaseDefRouter);
                            }
                        }
                        break;
                    case TM_IPFORWARD_TAG:
/* Enable/disable IP forwarding at the IP level (tvIpForward) */
                        if (    (localAction == TM_DHCPA_CONFIGURE_LEASE)
                             && (optionLength == 1) )
                        {
                            tm_context(tvIpForward) =
                                tm_dhcp_get_char( optionPtr,
                                                  TM_BOOT_TAG_OFFSET,
                                                  byteOffset );
                        }
                        break;
                    case TM_NBNS_TAG:
/* NetBios name server(s) */
                        if (localAction == TM_DHCPA_CONFIGURE_LEASE) 
                        {
/* Copy a maximum number of TM_DHCP_NBNS_NUM_SERVER name servers into the
   btNetBiosNameServers array. */
                            for (idx = 0;
                                 idx < TM_DHCP_NBNS_NUM_SERVER &&
                                            idx < (int)(optionLength /
                                                    TM_IP_ADDRESS_LENGTH);
                                 idx++)
                            {
                                tm_dhcp_long_copy(
                                    optionPtr,
                                    btEntryPtr->btNetBiosNameServers[idx],
                                    TM_BOOT_TAG_OFFSET +
                                        idx * TM_IP_ADDRESS_LENGTH,
                                    byteOffset );
                            }
                            btEntryPtr->btNetBiosNumNameServers
                                                    = (unsigned char)idx;
                        }
                        break;
                    default:
/* This is an unrecognized option. Save it to the unrecognized option list */
                        if (localAction == TM_DHCPA_CONFIGURE_LEASE)
                        {
                            custOptPtr = 
                                (ttBtUserCustOptPtr)tm_get_raw_buffer(
                                             sizeof(ttBtUserCustOpt) +
                                             (ttPktLen)optionLength);
                            if (custOptPtr != (ttBtUserCustOptPtr)0)
                            {
                                custOptPtr->buoDataPtr    = (ttCharPtr)
                                                            (custOptPtr + 1);
                                custOptPtr->buoDataLen    = (tt8Bit)
                                                            optionLength;
                                custOptPtr->buoBootOption = optionName;
                                tm_dhcp_byte_copy(optionPtr,
                                                *(custOptPtr->buoDataPtr),
                                                TM_BOOT_TAG_OFFSET,
                                                optionLength,
                                                byteOffset);
                                tfListAddToTail(
                                            &(btEntryPtr->btOfferEntryPtr
                                                ->btUserServCustOpts),
                                            (ttNodePtr)custOptPtr);
                            }
                        }
                        break;
                } /* switch */
/* pass over variable data, point to next tag */
                tm_dhcp_next_option( optionPtr,
                                     optionLength + TM_BOOT_TAGCODELEN_SIZE,
                                     byteOffset );
            } /* else */
        } /* while */
    } /* do */
    while (dhcpOverload != TM_8BIT_ZERO);
#ifdef TM_USE_DHCP_FQDN
    if (    (    (btEntryPtr->btServerFqdnBLen != TM_16BIT_ZERO)
              || (btEntryPtr->btServerFqdnBLen != TM_16BIT_ZERO) )
         && (btEntryPtr->btServerFqdnLen == TM_16BIT_ZERO))
    {
        if (!(btEntryPtr->btOfferEntryPtr->btUserEntry.btuFqdnOptionFlags
                                                    & TM_DHCP_FQDN_FLAG_S))
        {
            btEntryPtr->btFqdnStatus |= TM_FQDNF_NOT_SUPPORTED;
        }
        if ((btEntryPtr->btOfferEntryPtr->btUserEntry.btuFqdnOptionFlags
                                                    & TM_DHCP_FQDN_FLAG_E))
        {
/* Binary */
            btEntryPtr->btServerAFqdn =
                        tm_get_raw_buffer(btEntryPtr->btServerFqdnBLen + 1);
            if (btEntryPtr->btServerAFqdn != (tt8BitPtr)0)
            {
/* Copy from Binary to Ascii */
                retCode = tfHostnameBinaryToText(
                                      (ttCharPtr)btEntryPtr->btServerAFqdn,
                                      &btEntryPtr->btServerFqdnLen,
                                      (tt8BitPtr)btEntryPtr->btServerBFqdn,
                                      (int)btEntryPtr->btServerFqdnBLen,
                                      &btEntryPtr->btFqdnStatus);
                tm_free_raw_buffer(btEntryPtr->btServerAFqdn);
                tm_btdhcp_name_init(btEntryPtr, TM_BT_FQDN_AINDEX);
                if (retCode != TM_ENOERROR)
                {
                    btEntryPtr->btFqdnStatus |= TM_FQDNF_MALFORMED;
                }
            }
        }
        else
        {
            btEntryPtr->btServerFqdnLen = (int)btEntryPtr->btServerFqdnALen;
        }
    }
#endif /* TM_USE_DHCP_FQDN */
    if (localAction == TM_DHCPA_CONFIGURE_LEASE) 
    {
        if (dhcpFileOverloaded == TM_8BIT_ZERO)
        {
            optionLength = (int)tm_strlen(&bootHdrPtr->bootFile[0]);
#ifdef TM_DSP
            optionLength = tm_byte_count(optionLength);
#endif /* TM_DSP */
            if (optionLength > TM_BOOTFILENAME_SIZE)
            {
                optionLength = TM_BOOTFILENAME_SIZE;
            }
            tfBtBootCopy( btEntryPtr,
                          &bootHdrPtr->bootFile[0],
                          0,
                          optionLength,
                          tm_packed_byte_count(TM_BOOTFILENAME_SIZE),
                          TM_BT_FNAME_INDEX);
        }
        if (dhcpSnameOverloaded == TM_8BIT_ZERO)
        {
            optionLength = (int)tm_strlen(&bootHdrPtr->bootSname[0]);
#ifdef TM_DSP
            optionLength = tm_byte_count(optionLength);
#endif /* TM_DSP */
            if (optionLength > TM_BOOTSNAME_SIZE)
            {
                optionLength = TM_BOOTSNAME_SIZE;
            }
            tfBtBootCopy( btEntryPtr,
                          &bootHdrPtr->bootSname[0],
                          0,
                          optionLength,
                          tm_packed_byte_count(TM_BOOTSNAME_SIZE),
                          TM_BT_SNAME_INDEX);
        }
/* copy second phase BOOT server address (Siaddr) */
        tm_ip_copy( bootHdrPtr->bootSiaddr,
                    btEntryPtr->btBootSIpAddress );
    }
#ifndef TM_OPTIMIZE_SIZE
scanOptionsExit:
#endif /* !TM_OPTIMIZE_SIZE */
    if (action == TM_DHCPA_LAST_INDEX)
    {
        if (   (   (dhcpMsgType == TM_DHCPACK)
                && ((btEntryPtr->btDhcpState == TM_DHCPS_REBOOTING)
                    ? tm_ip_not_zero(dhcpServerId)
                    : tm_ip_match(dhcpServerId, btEntryPtr->btDhcpServerId))
                && hasLeaseOption
               )
            || (   (dhcpMsgType == TM_DHCPOFFER)
                && tm_ip_not_zero(dhcpServerId)
                && hasLeaseOption
               )
            || (   (dhcpMsgType == TM_DHCPNAK)
                && ((btEntryPtr->btDhcpState == TM_DHCPS_REBOOTING)
                    ? tm_ip_not_zero(dhcpServerId)
                    : tm_ip_match(dhcpServerId, btEntryPtr->btDhcpServerId))
                && !hasLeaseOption
               )
           )
        {
/*
 * Only valid DHCP messages are DHCPOFFER, DHCPACK, or DHCPNAK, with
 * a valid server ID option.
 * We only accept DHCPACK and DHCPNAK from our selected server, except if we
 * are in the REBOOTING state, in which case, we have not selected a server.
 * IP address lease time option requirements, according to RFC-2131:
 *  1) DHCPOFFER must include a valid IP address lease time option.
 *  2) DHCPACK must include a valid IP address lease time option unless it is
 *     in response to a DHCPINFORM, in which case it must not include an IP
 *     address lease time option.
 *  3) DHCPNAK must not include an IP address lease time option.
 */
            (void)tfDhcpStateMachine( btEntryPtr,
                                      packetPtr,
/* Event matches msgType - 1 */
                                      (tt8Bit)((int)dhcpMsgType - 1));
        }
#ifdef TM_ERROR_CHECKING
#ifndef TM_AUTO_TEST
        else
        {
/* 
 * If dhcpMsgType is zero, then we probably received a BOOTP message, so
 * we should ignore the error.
 */
            if (dhcpMsgType != TM_8BIT_ZERO)
            {
                tfKernelWarning("tfDhcpRecvCB",
                                "invalid DHCP message for client");
            }
        }
#endif /* !TM_AUTO_TEST */
#endif /* TM_ERROR_CHECKING */
    }
    return;
}

/*
 * tfDhcpInit function description
 * . Called from tfDhcpaInit() with the TM_DHCP_START flag.
 * . called from tfDhcpDecline() (when we found a conflict) or
 *   from tfDhcpaRestart() (when we got a NAK) with the TM_DHCP_RESTART flag
 * . If flag is RESTART, make sure that we do not use the user requested
 *   IP address, and discard the previously discovered values.
 * . Initialize the Client ID option (from the user, or using our own if
 *   user has not set one.)
 * . Initialize the requested IP address, if set by the user (and not
 *   reset by us.)
 * 
 */
static int tfDhcpInit (ttBtEntryPtr btEntryPtr, int flag)
{
    ttDeviceEntryPtr    devPtr;
    int                 errorCode;

/*
 * Assume no error
 */
    errorCode = TM_ENOERROR;
    if (flag == TM_DHCP_RESTART)
    {
/* Do not use requested user IP address, since it was declined/NAKed */
        tm_8bit_clr_bit( btEntryPtr->btUserSetFlags,
                         TM_BT_USER_SET_IPADDRESS);
/*
 * Discard previously discovered values. Re-initialize user entry.
 */
        tfBtInitUserEntry(btEntryPtr);
        if (    (btEntryPtr->btType == TM_BT_CONF)
             && ( (tm_ip_dev_conf_flag(
                                   btEntryPtr->btDevEntryPtr,
                                   btEntryPtr->btIndex) & TM_DEV_IP_DHCP) == 0 )
           )
        {
/* User has closed the interface in the call back */
            errorCode = TM_ESHUTDOWN;
        }
    }
    if (errorCode == TM_ENOERROR)
    {
/* Initialize the client ID option */
        if ( tm_8bit_one_bit_set( btEntryPtr->btUserSetFlags,
                                  TM_BT_USER_SET_CLIENTID ) )
        {
            tm_bcopy(btEntryPtr->btUserSetDhcpClientIdPtr, 
                     btEntryPtr->btDhcpClientIdPtr,
                     (unsigned int)btEntryPtr->btUserSetDhcpClientIdLength);
            btEntryPtr->btDhcpClientIdLength = 
                                btEntryPtr->btUserSetDhcpClientIdLength;
        }
        else
        {
            if ( tm_8bit_bits_not_set( btEntryPtr->btUserSetFlags,
                                       TM_BT_SUPPRESS_CLIENT_ID ) )
            {
/* Use the hardware address + btType + btIndex */
                btEntryPtr->btDhcpClientIdPtr[0] = TM_8BIT_ZERO;/* Hardware type */
                devPtr = btEntryPtr->btDevEntryPtr;
/* Add the harware address */
                tm_bcopy(devPtr->devPhysAddrArr, 
                     &btEntryPtr->btDhcpClientIdPtr[1],
                     TM_ETHER_ADDRESS_LENGTH);
/* ADD the type (BT_CONF, or BT_USER) */
                btEntryPtr->btDhcpClientIdPtr[TM_ETHER_ADDRESS_LENGTH + 1] = 
                        btEntryPtr->btType;
/* ADD the index */
                tm_bcopy(
                    (tt8BitPtr)&btEntryPtr->btIndex,
                    &btEntryPtr->btDhcpClientIdPtr[TM_ETHER_ADDRESS_LENGTH + 2],
                    2 );
                btEntryPtr->btDhcpClientIdLength = 
                                        (tt8Bit)(TM_ETHER_ADDRESS_LENGTH + 4);
            }
        }
/* Initialize the host name option */
        if (btEntryPtr->btUserSetDhcpHostNameLength != (tt8Bit)0)
        {
            tm_bcopy(btEntryPtr->btUserSetDhcpHostNamePtr, 
                     btEntryPtr->btDhcpHostNamePtr,
                     (unsigned int)btEntryPtr->btUserSetDhcpHostNameLength);
        }
        btEntryPtr->btDhcpHostNameLength = 
                                btEntryPtr->btUserSetDhcpHostNameLength;
/* Initialize the Requested IP address */
        if ( tm_8bit_one_bit_set( btEntryPtr->btUserSetFlags, 
                                  TM_BT_USER_SET_IPADDRESS ) )
        {
            tm_ip_copy(btEntryPtr->btUserSetDhcpIpAddress,
                       btEntryPtr->btYiaddr);
        }
    }
    return errorCode;
}

/*
 * tfDhcpRenewTimer function description
 * Called when a DHCP request sent to renew a lease (from tfDhcpRenewLease)
 * times out.  This routine will either retransmit a REQUEST or will notify
 * the user that the lease renewal has timed out.
 *
 * Return
 * None
 *
 */
static void tfDhcpRenewTimer (ttVoidPtr      timerBlockPtr,
                              ttGenericUnion userParm1,
                              ttGenericUnion userParm2)
{
    ttTimerPtr         dhcpRenewTimerPtr;
    ttDeviceEntryPtr   devPtr;
    ttBtEntryPtr       btEntryPtr;
    ttDevNotifyFuncPtr notifyFuncPtr;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);
    
    btEntryPtr = (ttBtEntryPtr)userParm1.genVoidParmPtr;
    devPtr = btEntryPtr->btDevEntryPtr;
    tm_call_lock_wait(&(devPtr->devLockEntry));
    dhcpRenewTimerPtr = btEntryPtr->btDhcpRenewTimerPtr;
    if (   (tm_timer_not_reinit((ttTimerPtr)timerBlockPtr))
         && (dhcpRenewTimerPtr == (ttTimerPtr)timerBlockPtr))
    {
        if (btEntryPtr->btDhcpState == TM_DHCPS_RENEWING)
        {
            if (btEntryPtr->btDhcpRenewRetries > 1)
            {
/* Retransmit DHCP REQUEST */
                btEntryPtr->btDhcpRenewRetries--;
                (void)tfDhcpaRequest(btEntryPtr, TM_PACKET_NULL_PTR);
            }
            else
            {
/* No retries left - notify user that lease renewal has timed out */
                notifyFuncPtr = devPtr->devBootInfo[TM_BOOT_DHCP].
                                                            bootNotifyFuncPtr;
/* Server failed to respond - remove timer */
                tm_timer_remove(dhcpRenewTimerPtr);
                btEntryPtr->btDhcpRenewTimerPtr = TM_TMR_NULL_PTR;
                if (notifyFuncPtr != (ttDevNotifyFuncPtr)0)
                {
                    tm_call_unlock(&(devPtr->devLockEntry));
                    (*(notifyFuncPtr))( (ttUserInterface)devPtr,
                                        TM_EHOSTDOWN);
                    tm_call_lock_wait(&(devPtr->devLockEntry));
                }
            }
        }
        else
        {
/* DHCP lease has been renewed - remove timer */
            tm_timer_remove(dhcpRenewTimerPtr);
            btEntryPtr->btDhcpRenewTimerPtr = TM_TMR_NULL_PTR;
        }
    }
    tm_call_unlock(&(devPtr->devLockEntry));
}

/*
 * tfDhcpRenewLease function description
 * Forces DCHP to renew the lease for a particular address.
 * . Get boot entry for interface/mhome.
 *
 *
 * Return
 * TM_ENOERROR      No error.
 * TM_EINVAL        No DHCP boot entry found for this interface/mhome.
 * TM_EPERM         Can't renew lease because no lease has been obtained.
 * TM_EALREADY      Lease renewal already in progress
 */
int tfDhcpRenewLease(ttUserInterface interfaceHandle,
                     int             mHomeIndex,
                     ttUser32Bit     timeoutMilliSecs,
                     unsigned char   retries)
{
    ttDeviceEntryPtr devPtr;
    ttBtEntryPtr     btEntryPtr;
    int              errorCode;
    tt8Bit           dhcpState;
    ttGenericUnion   timerParm1;

    devPtr = (ttDeviceEntryPtr)interfaceHandle;
    tm_call_lock_wait(&(devPtr->devLockEntry));
/* Get boot entry for interface/mhome. */
    btEntryPtr = tfBtMap( devPtr,
                          mHomeIndex,
                          TM_BT_CONF,
                          TM_BOOT_DHCP );

    errorCode = TM_ENOERROR;
    if (btEntryPtr != (ttBtEntryPtr) 0)
    {

        dhcpState = btEntryPtr->btDhcpState;

/* No lease has been obtained yet */
        if ( (dhcpState != TM_DHCPS_BOUND) &&
             (dhcpState != TM_DHCPS_RENEWING) )
        {
            errorCode = TM_EPERM;
        }

/* Renewal already in progress */
        if ( btEntryPtr->btDhcpRenewTimerPtr != TM_TMR_NULL_PTR )
        {
            errorCode = TM_EALREADY;
        }

        if ( errorCode == TM_ENOERROR )
        {
            btEntryPtr->btDhcpRenewRetries = retries;
            timerParm1.genVoidParmPtr = (ttVoidPtr)btEntryPtr;
            btEntryPtr->btDhcpRenewTimerPtr =
                        tfTimerAddExt( &btEntryPtr->btDhcpRenewTimer,
                                       tfDhcpRenewTimer,
                                       (ttTmCUFuncPtr)0,
                                       timerParm1,
                                       timerParm1, /* unused */
                                       timeoutMilliSecs,
                                       TM_TIM_AUTO);

            btEntryPtr->btDhcpState = TM_DHCPS_RENEWING;

/* Send first DHCP REQUEST */
            (void)tfDhcpaRequest(btEntryPtr, TM_PACKET_NULL_PTR);
        }
    }
    else
    {
/* No boot entry */
        errorCode = TM_EINVAL;
    }

    tm_call_unlock(&(devPtr->devLockEntry));

    return errorCode;
}

static void tfDhcpRemoveTimers (ttBtEntryPtr btEntryPtr)
{
    int i;

    btEntryPtr->btRetries = TM_8BIT_ZERO;
    btEntryPtr->btDhcpRenewRetries = 0;
    for (i = 0; i < TM_BT_NUM_TIMERS; i++)
    {
        if (btEntryPtr->btTimerPtr[i] != (ttTimerPtr)0)
        {
            tm_timer_remove(btEntryPtr->btTimerPtr[i]);
            btEntryPtr->btTimerPtr[i] = (ttTimerPtr)0;
        }
    }
}

#ifdef TM_DEBUG_LOGGING

static ttCharPtr tfDhcpMsgStr(tt32Bit msgEnd)
{
    ttCharPtr retCharPtr;

    switch(msgEnd)
    {
        case TM_DHCPDISCOVER_END:
            retCharPtr = "DHCP DISCOVER";
            break;
        case TM_DHCPREQUEST_END:
            retCharPtr = "DHCP REQUEST";
            break;
        case TM_DHCPDECLINE_END:
            retCharPtr = "DHCP DECLINE";
            break;
        case TM_DHCPRELEASE_END:
            retCharPtr = "DHCP RELEASE";
            break;
        case TM_DHCPINFORM_END:
            retCharPtr = "DHCP INFORM";
            break;
        default:
            retCharPtr = "DHCP UNKNOWN MSG";
            break;
    }
    return retCharPtr;
}
#endif /* TM_DEBUG_LOGGING */

#else /* !TM_USE_IPV4  || !TM_USE_DHCP */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV4 or TM_USE_DHCP is not defined */
int tv4DhcpDummy = 0;
#endif /* !TM_USE_IPV4 || !TM_USE_DHCP */
