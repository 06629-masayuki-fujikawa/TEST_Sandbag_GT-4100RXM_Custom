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
 * Description: BOOTP Client protocol
 *
 * Filename: trbootp.c
 * Author: Odile
 * Date Created: 05/24/98
 * $Source: source/trbootp.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:47:07JST $
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

/* Define unreferenced macro for PC-LINT compile error suppression */
#ifdef TM_LINT
LINT_UNREF_MACRO(TM_TIMEOFFSET_TAG)
LINT_UNREF_MACRO(TM_TIMESERVER_TAG)
LINT_UNREF_MACRO(TM_NAMESERVER_TAG)
LINT_UNREF_MACRO(TM_LOGSERVER_TAG)
LINT_UNREF_MACRO(TM_QUOTESERVER_TAG)
LINT_UNREF_MACRO(TM_LPRSERVER_TAG)
LINT_UNREF_MACRO(TM_IMPRESSSERVER_TAG)
LINT_UNREF_MACRO(TM_RLPSERVER_TAG)
LINT_UNREF_MACRO(TM_HOSTNAME_TAG)
#endif /* TM_LINT */

/*
 *  BOOTP header (from RFC 1542)
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
 * |                           vend   (64)                         |
 * +---------------------------------------------------------------+
 */


/*
 * FROM RFC 1700        Assigned Numbers                October 1994
 * BOOTP AND DHCP PARAMETERS
 *
 * The Bootstrap Protocol (BOOTP) [RFC951] describes an IP/UDP
 * bootstrap protocol (BOOTP) which allows a diskless client machine to
 * discover its own IP address, the address of a server host, and the
 * name of a file to be loaded into memory and executed.  The Dynamic
 * Host Configuration Protocol (DHCP) [RFC1531] provides a framework for
 * automatic configuration of IP hosts.  The "DHCP Options and BOOTP
 * Vendor Information Extensions" [RFC2132] describes the additions to the
 * Bootstrap Protocol (BOOTP) which can also be used as options with the
 * Dynamic Host Configuration Protocol (DHCP).
 *
 * BOOTP Vendor Extensions and DHCP Options are listed below:
 *
 */

/*
 * Option FIELDS in the vendor information area (vendFields)
 */
/*
 *    14      Merit Dump File              Client to dump and name
 *                                         the file to dump it to
 *    15      Domain Name       N          The DNS domain name of the
 *                                         client
 *    16      Swap Server       N          Swap Server addeess
 *    17      Root Path         N          Path name for root disk
 *    18      Extension File    N          Path name for more BOOTP info
 *
 *    19      Forward On/Off    1          Enable/Disable IP Forwarding
 *    20      SrcRte On/Off     1          Enable/Disable Source Routing
 *    21      Policy Filter     N          Routing Policy Filters
 *    22      Max DG Assembly   2          Max Datagram Reassembly Size
 *    23      Default IP TTL    1          Default IP Time to Live
 *    24      MTU Timeout       4          Path MTU Aging Timeout
 *    25      MTU Plateau       N          Path MTU  Plateau Table
 *    26      MTU Interface     2          Interface MTU Size
 *    27      MTU Subnet        1          All Subnets are Local
 *    28      Broadcast Address 4          Broadcast Address
 *    29      Mask Discovery    1          Perform Mask Discovery
 *    30      Mask Supplier     1          Provide Mask to Others
 *    31      Router Discovery  1          Perform Router Discovery
 *    32      Router Request    4          Router Solicitation Address
 *    33      Static Route      N          Static Routing Table
 *    34      Trailers          1          Trailer Encapsulation
 *    35      ARP Timeout       4          ARP Cache Timeout
 *    36      Ethernet          1          Ethernet Encapsulation
 *    37      Default TCP TTL   1          Default TCP Time to Live
 *    38      Keepalive Time    4          TCP Keepalive Interval
 *    39      Keepalive Data    1          TCP Keepalive Garbage
 *    40      NIS Domain        N          NIS Domain Name
 *    41      NIS Servers       N          NIS Server Addresses
 *    42      NTP Servers       N          NTP Server Addresses
 *    43      Vendor Specific   N          Vendor Specific Information
 *    44      NETBIOS Name Srv  N          NETBIOS Name Servers
 *    45      NETBIOS Dist Srv  N          NETBIOS Datagram Distribution
 *    46      NETBIOS Note Type 1          NETBIOS Note Type
 *    47      NETBIOS Scope     N          NETBIOS Scope
 *    48      X Window Font     N          X Window Font Server
 *    49      X Window Manmager N          X Window Display Manager
 *    50      Address Request   4          Requested IP Address
 *    51      Address Time      4          IP Address Lease Time
 *    52      Overload          1          Overloaf "sname" or "file"
 *    53      DHCP Msg Type     1          DHCP Message Type
 *    54      DHCP Server Id    4          DHCP Server Identification
 *    55      Parameter List    N          Parameter Request List
 *    56      DHCP Message      N          DHCP Error Message
 *    57      DHCP Max Msg Size 2          DHCP Maximum Message Size
 *    58      Renewal Time      4          DHCP Renewal (T1) Time
 *    59      Rebinding Time    4          DHCP Rebinding (T2) Time
 *    60      Class Id          N          Class Identifier
 *    61      Client Id         N          Client Identifier
 *    62      Netware/IP Domain N          Netware/IP Domain Name
 *    63      Netware/IP Option N          Netware/IP sub Options
 *   64-127  Unassigned
 *   128-154 Reserved
 *   255     End               0          None
 */

/*
 * Local macros
 */

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *     0      Pad               0          None
 */
/* defined in trmacro.h */
/* #define TM_BOOT_PAD_TAG         (tt8Bit)0   */ /* padding fields */
/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *   255     End               0          None
 */
/* defined in trmacro.h */
/* #define TM_BOOT_END_TAG         (tt8Bit)255 */ /* end of usable data */
/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *     1      Subnet Mask       4          Subnet Mask Value
 */
/* #define TM_SUBNETMASK_TAG  (tt8Bit)1 */ /* defined in trmacro.h */
/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *     2      Time Offset       4          Time Offset in Seconds from UTC
 *     2      Time Offset       4          Time Offset in Seconds from UTC
 */
#define TM_TIMEOFFSET_TAG  (tt8Bit)2

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *     3      Router          N          N/4 Router addresses
 */
/*#define TM_ROUTER_TAG     (tt8Bit)3*/ /* defined in trmacro.h */

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *     4      Time Server       N          N/4 Timeserver addresses
 */
#define TM_TIMESERVER_TAG  (tt8Bit)4

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *     5      Name Server       N          N/4 IEN-116 Server addresses
 */
#define TM_NAMESERVER_TAG  (tt8Bit)5

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *     6      Domain Server     N          N/4 DNS Server IP addresses
 */
/*#define TM_DOMAINSERVER_TAG (tt8Bit)6 */ /* in trmacro.h */

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *     7      Log Server        N          N/4 Logging Server addresses
 */
/* IP addresses of N/4 MIT-LCS UDP log server */
#define TM_LOGSERVER_TAG   (tt8Bit)7 

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *     8      Quotes Server     N          N/4 Quotes Server addresses
 */
#define TM_QUOTESERVER_TAG (tt8Bit)8

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *     9      LPR Server        N          N/4 Printer Server addresses
 */
#define TM_LPRSERVER_TAG   (tt8Bit)9

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *    10      Impress Server    N          N/4 Impress Server addresses
 */
#define TM_IMPRESSSERVER_TAG (tt8Bit)10

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *   11      RLP Server        N          N/4 Ressource Location Protocol
 *                                        Server addresses
 */
#define TM_RLPSERVER_TAG   (tt8Bit)11

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *    12      Hostname          N         Hostname string. Name of the client
 */
#define TM_HOSTNAME_TAG    (tt8Bit)12

/*
 *    Tag     Name          Data Length    Meaning
 *    ---     ----          -----------    -------
 *    13      Boot File Size    2          Size of boot file in 512 byte
 *                                         chunks
 */
/*#define TM_BOOTFILESIZE_TAG (tt8Bit)13*/ /* in trmacro.h*/

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
    buffer[tm_packed_byte_count(index+1)-1] &= \
        (~(0xff << ((TM_DSP_ROUND_PTR - \
        (index % TM_DSP_BYTES_PER_WORD)) << 3))); \
    buffer[tm_packed_byte_count(index+1)-1] |= \
        (dataChar << ((TM_DSP_ROUND_PTR - \
        (index % TM_DSP_BYTES_PER_WORD)) << 3)); \
}

/*
 * Move the pointer to the next BOOTP option.
 */
#ifdef TM_DSP
#define tm_bootp_next_option(optPtr, index, byteOffset) \
    optPtr += (index + byteOffset) >> (TM_DSP_BYTES_PER_WORD >> 1); \
    byteOffset  = (index + byteOffset) % TM_DSP_BYTES_PER_WORD
#else /* !TM_DSP */
#define tm_bootp_next_option(optPtr, index, byteOffset) \
    (optPtr) += (index)
#endif /* TM_DSP */

/*
 * Get BOOTP option length
 */
#ifdef TM_DSP
#define tm_bootp_get_opt_len(optPtr, index, byteOffset) \
tm_dsp_get_8bit((optPtr), ((index) + (byteOffset)))
#else /* !TM_DSP */
#define tm_bootp_get_opt_len(optPtr, index, byteOffset) \
    *( (optPtr) + (index) )
#endif /* TM_DSP */



#ifdef TM_DSP
#define tm_bootp_short_copy(srcPtr, dest, srcOffset, bytePos)             \
{ \
    dest = tm_dsp_get_8bit((srcPtr), ((srcOffset) + (bytePos))    ) << 8; \
    dest = tm_dsp_get_8bit((srcPtr), ((srcOffset) + (bytePos)) + 1);      \
}
#else /* !TM_DSP */
#define tm_bootp_short_copy(srcPtr, dest, srcOffset, bytePos)          \
{ \
    ((tt8BitPtr)(&dest))[0] = (srcPtr)[srcOffset];                     \
    ((tt8BitPtr)(&dest))[1] = (srcPtr)[srcOffset+1];                   \
}
#endif /* TM_DSP */




#ifdef TM_DSP
#define tm_bootp_long_copy(srcPtr, dest, srcOffset, bytePos) \
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
#define tm_bootp_long_copy(srcPtr, dest, srcOffset, bytePos)           \
{ \
    ((tt8BitPtr)(&dest))[0] = (srcPtr)[srcOffset];                     \
    ((tt8BitPtr)(&dest))[1] = (srcPtr)[srcOffset+1];                   \
    ((tt8BitPtr)(&dest))[2] = (srcPtr)[srcOffset+2];                   \
    ((tt8BitPtr)(&dest))[3] = (srcPtr)[srcOffset+3];                   \
}
#endif /* TM_DSP */

#ifdef TM_DSP
#define tm_bootp_get_char(srcPtr, srcOffset, bytePos) \
    tm_dsp_get_8bit((srcPtr), ((srcOffset) + (bytePos)))
#else /* !TM_DSP */
#define tm_bootp_get_char(srcPtr, srcOffset, bytePos) \
    (srcPtr)[(srcOffset)]
#endif /* TM_DSP */

#ifdef TM_DSP
#define tm_bootp_set_char(srcChar, destPtr, destOffset, bytePos) \
    tm_dsp_put_8bit((destPtr), ((destOffset)+(bytePos)), srcChar)
#else /* !TM_DSP */
#define tm_bootp_set_char(srcChar, destPtr, destOffset, bytePos) \
    (destPtr)[(destOffset)] = (srcChar)
#endif /* TM_DSP */

#ifdef TM_DSP
#define tm_bootp_byte_copy(srcPtr, dest, srcOffset, length, bytePos) \
    tfMemCopyOffset( (int *) (srcPtr), (bytePos) + (srcOffset),      \
                     (int *) (&(dest)), 0, (length))
#else /* TM_DSP */
#define tm_bootp_byte_copy(srcPtr, dest, srcOffset, length, bytePos) \
    tm_bcopy( &((srcPtr)[srcOffset]), (&(dest)), length )
#endif /* TM_DSP */

#ifdef TM_DSP
#define tm_bootp_copy_list_to_pkt(listPtr, destPtr, length, bytePos, tempCount) \
{                                                                               \
    for (tempCount=0;tempCount<(length);tempCount++)                            \
    {                                                                           \
        tm_dsp_put_8bit((destPtr),                                              \
                        (tempCount)+(bytePos),                                  \
                        ((listPtr)[tempCount] & 0xff));                         \
    }                                                                           \
}
#else /* TM_DSP */
#define tm_bootp_copy_list_to_pkt(listPtr, destPtr, length, bytePos, tempCount) \
    tm_bcopy( (listPtr), (destPtr), length )
#endif /* TM_DSP */

/*
 * Local types
 */


/*
 * Local functions
 */
static void tfBootpInit (ttBtEntryPtr btEntryPtr);

static void tfBootpBtStop (ttBtEntryPtr btEntryPtr);

TM_NEARCALL static void tfBootpTimer (ttVoidPtr      timerBlockPtr,
                                      ttGenericUnion userParm1,
                                      ttGenericUnion userParm2);

TM_NEARCALL static int  tfBootpConfig( ttVoidPtr     voidPtr,
                                       int           flag,
                                       tt16Bit       multiHomeIndex );

static int  tfBootpSendPacket (ttBtEntryPtr btEntryPtr);

#ifdef TM_USE_AUTO_IP
static int tfBootpFinish ( ttUserInterface    interfaceHandle,
                           ttUserIpAddress    ipAddress,
                           int                errorCode,
                           ttUserGenericUnion bootpParam );
#endif /* TM_USE_AUTO_IP */

TM_NEARCALL static int tfBootpAddCustOpt( ttNodePtr      nodePtr,
                              ttGenericUnion genParam1);

/*
 * Initialize btEntry in INIT state, remove any timer from previous attempt
 * Free server options from previous attempt.
 * Initialize user set BOOTP option.
 * Send first BOOTP request.
 */
int tfBootpStart( ttDeviceEntryPtr devPtr,
                  int              index,
                  tt8Bit           type )
{
    ttBtEntryPtr btEntryPtr;
    int          errorCode;

    errorCode = TM_ENOENT;
    btEntryPtr = tfBtMap(devPtr, index, type, TM_BOOT_BOOTP);
    if (btEntryPtr != (ttBtEntryPtr)0)
    {
        tfBootpInit(btEntryPtr);
        errorCode = tfBtOpenSocket(btEntryPtr);
        if (errorCode == TM_ENOERROR)
        {
            errorCode = tfBootpSendPacket(btEntryPtr);
            if (errorCode == TM_ENOERROR)
            {
                errorCode = TM_EINPROGRESS;
            }
        }
    }
    return errorCode;
}

/*
 * Initialize btEntry in INIT state, remove any timer from previous attempt
 * Free server options from previous attempt, Discard previously discovered
 * values. 
 */
TM_PROTO_EXTERN int tfBootpStop( ttDeviceEntryPtr devPtr,
                                 int              index,
                                 tt8Bit           type )
{
    ttBtEntryPtr btEntryPtr;
    int          errorCode;

    errorCode = TM_EINVAL;
    btEntryPtr = tfBtMap(devPtr, index, type, TM_BOOT_BOOTP);
    if (btEntryPtr != (ttBtEntryPtr)0)
    {
        tfBootpBtStop(btEntryPtr);
        errorCode = TM_ENOERROR;
#ifdef TM_USE_STOP_TRECK
        if (tm_16bit_one_bit_set(devPtr->devFlag2, 
                                 TM_DEVF2_UNINITIALIZING))
        {
            tm_kernel_free(btEntryPtr);
            devPtr->devBtEntryTablePtr[type][TM_BOOT_BOOTP][index] =
                                                                 (ttBtEntryPtr)0;
        }
#endif /* TM_USE_STOP_TRECK */
    }
    return errorCode;
}

/*
 * Initialize btEntry in INIT state, remove any timer from previous attempt
 * Free server options from previous attempt, Discard previously discovered
 * values. Close the socket.
 */
static void tfBootpBtStop (ttBtEntryPtr btEntryPtr)
{
/* Discard previously discovered values. */
    tfBtInitUserEntry(btEntryPtr);
/* Remove timer from previous attempt */
    tfBtRemoveRetryTimer(btEntryPtr);
    btEntryPtr->btDhcpState = TM_DHCPS_INIT;
/* Close the socket */
    tfBtCloseSocket(btEntryPtr);
}

/*
 * Initialize btEntry in INIT state, remove any timer from previous attempt
 * Free server options from previous attempt.
 * Initialize user set BOOTP option.
 */
static void tfBootpInit (ttBtEntryPtr btEntryPtr)
{
    tfBootpBtStop(btEntryPtr);
/* Initialize the host name option */
    if (btEntryPtr->btUserSetDhcpHostNameLength != (tt8Bit)0)
    {
        tm_bcopy(btEntryPtr->btUserSetDhcpHostNamePtr, 
                 btEntryPtr->btDhcpHostNamePtr,
                 (unsigned int)btEntryPtr->btUserSetDhcpHostNameLength);
    }
    btEntryPtr->btDhcpHostNameLength = 
                                btEntryPtr->btUserSetDhcpHostNameLength;
}

/*
 * tfBootpConfig() function description
 * tfBootpConfig function pointer is stored in the device structure
 * at devBootFuncPtr.
 * When the user wants to configure the interface with the BOOTP
 * client protocol, tfConfigInterface() is called, which in turns calls
 * the device link layer open function, which in turns calls the device
 * boot function: tfBootpConfig(). Note that tfConfigInterface() prevents
 * re_entrancy as long as the configuration as not completed or failed.
 * 1. If the BOOTP client UDP socket is not opened, open it.
 * 2. Send a BOOTP request. Retransmission will be handled by
 *    tfBootpSendPacket
 * Parameter        meaning
 * voidPtr          pointer to device entry
 * flag             Open, or close
 *
 * Return value     meaning
 * TM_EINPROGRESS   success
 * TM_EMFILE        Not enough sockets
 * TM_ENOBUFS       Not enough memory
 * other error code UDP send failed
 */
static int tfBootpConfig ( ttVoidPtr     voidPtr,
                           int           flag,
                           tt16Bit       multiHomeIndex )
{
    ttDeviceEntryPtr devPtr;
    tt8Bit           type;
    int              errorCode;

    devPtr = (ttDeviceEntryPtr)voidPtr;
    errorCode = TM_ENOERROR;
    if (flag == TM_LL_OPEN_STARTED)
/* Open */
    {
        if ( tm_8bit_one_bit_set(
                            tm_ip_dev_conf_flag(devPtr, multiHomeIndex),
                            TM_DEV_IP_BOOTP ) )
        {
            errorCode = tfBtInit( devPtr,
                                  TM_DHCP_NOTIFY_FUNC_NULL_PTR,
                                  (int)multiHomeIndex,
                                  TM_BT_CONF,
                                  TM_BOOT_BOOTP );
            if (errorCode == TM_ENOERROR)
            {
/* Initialize user structure; allocate, fill in and send a BOOT request */
                errorCode = tfBootpStart(devPtr, (int)multiHomeIndex,
                                         TM_BT_CONF);
            }
        }
        else
        {
/* Need to use tfUseDhcp with TM_DEV_IP_DHCP */
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
        errorCode = tfBootpStop(devPtr, multiHomeIndex, type);
        if (
#ifdef TM_USE_STOP_TRECK
             (flag == TM_LL_CLOSE_STARTED) &&
#endif /* TM_USE_STOP_TRECK */
             (errorCode == TM_ENOERROR) )
        {
            tm_8bit_clr_bit( tm_ip_dev_conf_flag(devPtr, multiHomeIndex),
                             (TM_DEV_IP_BOOTP | TM_DEV_IP_CONF_STARTED));
        }
    }
    return errorCode;
}

/*
 * tfUseBootp function description:
 * 1. Check that the device entry is valid.
 * 2. Check that the interface is not opened. 
 * 3. Initialize device boot function with tfBootpConfig()
 * 4. Initialize device recv call back function with tfBootpRecvCB()
 * 5. Initialize device BOOT user notify function with passed function pointer.
 *
 * Parameters       meaning
 * interfaceHandle  Pointer to interfae
 * devNotifyFuncPtr Pointer to a user notify function (for notification of
 *                  configuration completion)
 *
 * Return value     meaning
 * TM_ENOERROR      success
 * TM_EINVAL        Invalid link layer handle
 * TM_EADDRINUSE    A UDP socket is already bound to the BOOTP client port
 * TM_EMFILE        No more sockets
 * TM_ENOBUFS       Not enough available memory
 */
int tfUseBootp (ttUserInterface    interfaceHandle,
                ttDevNotifyFuncPtr notifyFuncPtr)
{
    ttDeviceEntryPtr devPtr;
    int              errorCode;

    devPtr = (ttDeviceEntryPtr)interfaceHandle;
    errorCode = tfValidInterface(devPtr);
    if (errorCode == TM_ENOERROR)
    {
        tm_call_lock_wait(&(devPtr->devLockEntry));
        devPtr->devBootInfoBootp.bootFuncPtr = tfBootpConfig;
/* receive call back from tfBootpRecvCB */
        devPtr->devBootInfoBootp.bootRecvFuncPtr = tfBootpRecvCB;
/* User notify */
        devPtr->devBootInfoBootp.bootNotifyFuncPtr = notifyFuncPtr;
        tm_call_unlock(&(devPtr->devLockEntry));
    }
    return errorCode;
}

/*
 * tfBootpTimer function description:
 * Retransmission timer, called by the timer interface to try and
 * and re-send a BOOTP request.
 * 1. Lock the device.
 * 2. If device not configured yet, and number of retries has not been
 *    reached send a BOOTP request and add this retry timer.
 * 3. If number of retries has been reached, reset the retry timer pointer,
 *    and notify the user of tfConfigInterface timeout.
 *
 * Parameters   Meaning
 * voidPtr      pointer to BOOT entry
 */
static void tfBootpTimer (ttVoidPtr      timerBlockPtr,
                          ttGenericUnion userParm1,
                          ttGenericUnion userParm2)
{
    ttDeviceEntryPtr    devPtr;
    ttBtEntryPtr        btEntryPtr;
#ifndef TM_SINGLE_INTERFACE_HOME
    tt16Bit             mhomeIndex;
#endif /* TM_SINGLE_INTERFACE_HOME */

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    
    btEntryPtr = (ttBtEntryPtr)userParm1.genVoidParmPtr;
    devPtr = btEntryPtr->btDevEntryPtr;
    tm_call_lock_wait(&(devPtr->devLockEntry));
    if (tm_timer_not_reinit((ttTimerPtr)timerBlockPtr))
    {
        btEntryPtr->btRetryTimerPtr = TM_TMR_NULL_PTR;
#ifndef TM_SINGLE_INTERFACE_HOME
        mhomeIndex = btEntryPtr->btIndex;
#endif /* TM_SINGLE_INTERFACE_HOME */
/*
 * and if we trying to configure using User BOOTP protocol
 * or using automatic configuration and device not configured yet
 */
        if (
#ifdef TM_USE_STRONG_ESL
                (devPtr->devBootSocketDescriptor != TM_SOCKET_ERROR)
#else /* TM_USE_STRONG_ESL */
                (tm_context(tvBootSocketDescriptor) != TM_SOCKET_ERROR)
#endif /* TM_USE_STRONG_ESL */
             && (    (btEntryPtr->btType == TM_BT_USER)
                  || (   ( tm_8bit_one_bit_set(
                            tm_ip_dev_conf_flag(devPtr, mhomeIndex),
                            TM_DEV_IP_BOOTP ) )
                       && ( tm_8bit_bits_not_set(
                            tm_ip_dev_conf_flag(devPtr, mhomeIndex),
                            TM_DEV_IP_CONFIG ) ) )
                )
           )
        {
            if (    (btEntryPtr->btRetries < devPtr->devBtMaxRetries)
                 || (devPtr->devBtMaxRetries == TM_DEV_BOOT_INF) )
            {
                (void)tfBootpSendPacket(btEntryPtr);
            }
            else
            {
                (void)tfBtFinishConfig(btEntryPtr, TM_ETIMEDOUT);
                btEntryPtr->btDhcpState = TM_DHCPS_INIT;
            }
        }
    }
    tm_call_unlock(&(devPtr->devLockEntry));
}


/*
 * tfBootpAddCustOpt() Function Description
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
static int tfBootpAddCustOpt(ttNodePtr      nodePtr,
                             ttGenericUnion genParam1)
{
    ttBtUserCustOptPtr optPtr;
    ttBtPktDataPtr     pdPtr;
#ifdef TM_DSP
    int             tempDspCount;
#endif /* TM_DSP */

    optPtr = (ttBtUserCustOptPtr)nodePtr;
    pdPtr  = (ttBtPktDataPtr)genParam1.genVoidParmPtr;

/* Set the option */
    tm_bootp_set_char(optPtr->buoBootOption,
                      pdPtr->bpdBootOptionPtr,
                      0,
                      pdPtr->bpdByteOffset);
    tm_bootp_next_option(pdPtr->bpdBootOptionPtr,
                         1,
                         pdPtr->bpdByteOffset);
/* Set the option length */
    tm_bootp_set_char(optPtr->buoDataLen,
                      pdPtr->bpdBootOptionPtr,
                      0,
                      pdPtr->bpdByteOffset);
    tm_bootp_next_option(pdPtr->bpdBootOptionPtr,
                         1,
                         pdPtr->bpdByteOffset);
/* Set the option data */
    tm_bootp_copy_list_to_pkt(optPtr->buoDataPtr,
                              pdPtr->bpdBootOptionPtr,
                              optPtr->buoDataLen,
                              pdPtr->bpdByteOffset,
                              tempDspCount);
    tm_bootp_next_option(pdPtr->bpdBootOptionPtr,
                         optPtr->buoDataLen,
                         pdPtr->bpdByteOffset);

    return TM_8BIT_NO;
}


/*
 * tfBootpSendPacket() Function Description
 * 1. Get a BOOTP packet, pre-initialized with default values. Arrange
 *    for retransmission.
 * 2. Zero the options fields.
 * 2. Add boot end tag.
 * 3. Broadcast the BOOTP request.
 *
 * Parameters   Meaning
 * btEntryPtr   pointer to BOOT entry
 *
 * Return
 * TM_ENOERROR  no error
 * TM_ENOBUFS   no memory to allocate BOOTP header
 * other        from Send call
 */
static int tfBootpSendPacket (ttBtEntryPtr btEntryPtr)
{
    ttPacketPtr          packetPtr;
    ttBootHeaderPtr      bootHdrPtr;
    tt8BitPtr            bootOptionPtr;
    ttGenericUnion       listParam;
    ttBtPktData          pktData;
#ifdef TM_DSP
    ttPktLen             byteOffset;
    int                  tempDspCount;
#endif /* TM_DSP */
    int                  errorCode;

#ifdef TM_DSP
    byteOffset = 0;
#endif /* TM_DSP */

/*
 * We first allocate a zero copy BOOTP buffer of length corresponding to a
 * BOOTP send request
 */
    packetPtr = tfBtGetFillHeader (btEntryPtr,
                                   TM_BOOTP_HEADER_SIZE,
                                   tfBootpTimer);
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
        bootHdrPtr = (ttBootHeaderPtr)packetPtr->pktLinkDataPtr;
        bootOptionPtr = bootHdrPtr->bootOptions.optFields;

/* Host name option, if set by the user  */
        if (btEntryPtr->btDhcpHostNameLength != (tt8Bit)0)
        {
/* Add host name option */
            tm_bootp_set_char(
                        TM_HOSTNAME_TAG, bootOptionPtr, 0, byteOffset);
            tm_bootp_next_option(bootOptionPtr, 1, byteOffset);
/* host name option Length */
            tm_bootp_set_char(btEntryPtr->btDhcpHostNameLength,
                                bootOptionPtr, 0, byteOffset);
            tm_bootp_next_option(bootOptionPtr, 1, byteOffset);  
/* host name */
            tm_bootp_copy_list_to_pkt(
                btEntryPtr->btDhcpHostNamePtr,
                bootOptionPtr,
                (unsigned int)btEntryPtr->btDhcpHostNameLength,
                byteOffset, tempDspCount);

            tm_bootp_next_option(bootOptionPtr,
                            (unsigned)btEntryPtr->btDhcpHostNameLength,
                            byteOffset);
        }

/* Save the current offset */
        pktData.bpdBootOptionPtr = bootOptionPtr;
#ifdef TM_DSP
        pktData.bpdByteOffset    = byteOffset;
#endif /* TM_DSP */
        listParam.genVoidParmPtr  = &pktData;
/* Add all custom options to the packet */
        (void)tfListWalk(&(btEntryPtr->btUserSetCustOpts),
                         tfBootpAddCustOpt,
                         listParam);
/* Update the current offset */
#ifdef TM_DSP
        byteOffset = pktData.bpdByteOffset;
#endif /* TM_DSP */
        bootOptionPtr = pktData.bpdBootOptionPtr;

        tm_bootp_set_char(TM_BOOT_END_TAG, bootOptionPtr, 0, byteOffset);

/*
 * destination address is: IP limited Broadcast,
 *                         TM_BOOTPS_PORT (BOOTP UDP server port)
 */
        errorCode = tfBtSendPacket(packetPtr, TM_IP_LIMITED_BROADCAST);
    }
    else
    {
        errorCode = TM_ENOBUFS;
    }
    return errorCode;
}

/*
 * tfBootpRecvCB() Function Description
 * Process an recvCB packet for the BOOTP client
 * 1. Lock the device
 * 2. Retrieve BOOTP information from the BOOTP header
 * 3. Remove the retry timer.
 * 4. Finish configuring the device.
 * 5. Unlock the device
 *
 * Parameters:
 * socketDescriptor socketDescritor of the BOOT socket opened in
 *                  tfBootpConfig()
 * socketCBFlags    event flag (TM_CB_RECV always, since it is the only
 *                  flag we have registered for)
 *
 * No return value
 */
void tfBootpRecvCB ( ttPacketPtr  packetPtr,
                     ttBtEntryPtr btEntryPtr )
{
/* point to BOOT header */
    ttBootHeaderPtr     bootHdrPtr;
    tt8BitPtr           optionPtr;
    tt8BitPtr           lastOptionPtr;
    ttBtUserCustOptPtr  custOptPtr;
#ifdef TM_USE_AUTO_IP
    ttDeviceEntryPtr    devPtr;
    ttUserGenericUnion  bootpParam;
#endif /* TM_USE_AUTO_IP */
    int                 errorCode;
    int                 optionLength;
    int                 index;
    tt16Bit             bootFileSize;
    tt8Bit              optionName;
#ifdef TM_DSP
    unsigned int        byteOffset; 
#endif /* TM_DSP */
 
    bootFileSize = (tt16Bit)0;
    if (btEntryPtr->btDhcpState != TM_DHCPS_BOUND)
/* If we are not currently processing or have not processed a BOOTP request */
    {
/* Discard previously discovered values */
        tfBtDiscardServerNames(btEntryPtr);
        bootHdrPtr = (ttBootHeaderPtr)packetPtr->pktLinkDataPtr;
/* 
 * Gather information from the vendor specific area, if any. Check that
 * message is not a DHCP message.
 */
/* check RFC2132 compliant BOOTP vendor field */
        if (bootHdrPtr->bootOptions.optMagic == TM_BOOT_MAGIC)
        {
            optionPtr = bootHdrPtr->bootOptions.optFields;
/* Allow for options bigger than 64 bytes */
            lastOptionPtr =
                ((tt8BitPtr)bootHdrPtr) +
                tm_packed_byte_count((int)packetPtr->pktChainDataLength - 1);

#ifdef TM_DSP
            byteOffset = 0;
#endif /* TM_DSP */        
            while (   (tm_bootp_get_char(optionPtr,0,byteOffset)
                   != TM_BOOT_END_TAG)
                      && (optionPtr <= lastOptionPtr) )
            {
                optionName = tm_bootp_get_char(optionPtr, 0, byteOffset);
                switch(optionName)
                {
/* Fixed length field */
                case TM_BOOT_PAD_TAG:
                    tm_bootp_next_option(optionPtr, 1, byteOffset);
                    break;
                case TM_DHCP_MSG_TYPE:
                    goto bootpRecvCBExit;
/* TFTP boot file size (for TFTP download) */
                case TM_BOOTFILESIZE_TAG:
                    optionLength = tm_bootp_get_opt_len(optionPtr, 
                                                        TM_BOOT_TAGCODE_SIZE, 
                                                        byteOffset);
/* Copy boot file size from &optionPtr[TM_BOOT_TAG_OFFSET] */
                    if (optionLength == 2)
                    {
                        tm_bootp_short_copy( optionPtr,
                                             bootFileSize,
                                             TM_BOOT_TAG_OFFSET,
                                             byteOffset);
                        btEntryPtr->btBootFileSize = ntohs(bootFileSize);
                    }
/* point to next tag */
                    tm_bootp_next_option(optionPtr, 
                                         TM_BOOT_TAGCODELEN_SIZE, 
                                         byteOffset);
                    break;
/* Domain Name Servers (BIND) */
                case TM_DOMAINSERVER_TAG:
                    optionLength = tm_bootp_get_opt_len(optionPtr, 
                                                        TM_BOOT_TAGCODE_SIZE, 
                                                        byteOffset);
/* copy first 2 domain name servers from &optionPtr[TM_BOOT_TAG_OFFSET] */
                    if ( optionLength >= TM_IP_ADDRESS_LENGTH )
                    {
                        tm_bootp_long_copy( optionPtr,
                                            btEntryPtr->btDns1ServerIpAddress,
                                            TM_BOOT_TAG_OFFSET,
                                            byteOffset );
                    }
                    if ( optionLength >= (2 * TM_IP_ADDRESS_LENGTH) )
                    {
                        tm_bootp_long_copy( optionPtr,
                                            btEntryPtr->btDns2ServerIpAddress,
                                            TM_BOOT_TAG_OFFSET + 
                                                TM_IP_ADDRESS_LENGTH,
                                            byteOffset );
                    }
/* point to next tag */
                    tm_bootp_next_option( optionPtr,
                                          optionLength+TM_BOOT_TAGCODELEN_SIZE,
                                          byteOffset );
                    break;
/* Domain Name */
                case TM_DOMAINNAME_TAG:
/* Host Name */
                case TM_HOSTNAME_TAG:
                    if (optionName == TM_DOMAINNAME_TAG)
                    {
                        index = TM_BT_DOMAIN_INDEX;
                    }
                    else
                    {
                        index = TM_BT_RXHNAME_INDEX;
                    }
                    optionLength = tm_bootp_get_opt_len(optionPtr, 
                                                        TM_BOOT_TAGCODE_SIZE, 
                                                        byteOffset);
                    
                    tfBtBootCopy(  btEntryPtr
                                 , optionPtr
                                 , TM_BOOT_TAG_OFFSET
#ifdef TM_DSP
                                   + byteOffset
#endif /* !TM_DSP */
                                 , optionLength
                                 , (int)(ttUserPtrCastToInt)
                                                (lastOptionPtr - optionPtr)
                                 , index);
/* point to next tag */
                    tm_bootp_next_option( optionPtr,
                                          optionLength+TM_BOOT_TAGCODELEN_SIZE,
                                          byteOffset );
                    break;
                case TM_SUBNETMASK_TAG: /* Subnet Mask */
                    optionLength = tm_bootp_get_opt_len(optionPtr, 
                                                        TM_BOOT_TAGCODE_SIZE, 
                                                        byteOffset);
/* optionLength should be 4 bytes */
/* copy subnet mask from &optionPtr[TM_BOOT_TAG_OFFSET], set flag */
                    if (optionLength == TM_IP_ADDRESS_LENGTH)
                    {
                        tm_bootp_long_copy( optionPtr,
                                            btEntryPtr->btNetMask,
                                            TM_BOOT_TAG_OFFSET,
                                            byteOffset );
                    }
/* point to next tag */
                    tm_bootp_next_option( optionPtr,
                                          optionLength+TM_BOOT_TAGCODELEN_SIZE,
                                          byteOffset );
                    break;
                case TM_ROUTER_TAG: /* routers */
                    optionLength = tm_bootp_get_opt_len(optionPtr, 
                                                        TM_BOOT_TAGCODE_SIZE, 
                                                        byteOffset);
/* copy routing entries, set flag */
                    if (optionLength >= TM_IP_ADDRESS_LENGTH)
                    {
                        tm_bootp_long_copy( optionPtr,
                                            btEntryPtr->btDefRouter,
                                            TM_BOOT_TAG_OFFSET,
                                            byteOffset );
                    }
/* point to next tag */
                    tm_bootp_next_option( optionPtr,
                                          optionLength+TM_BOOT_TAGCODELEN_SIZE,
                                          byteOffset );
                    break;
                case TM_IPFORWARD_TAG:
/* Enable/disable IP forwarding at the IP level (tvIpForward) */
                    optionLength = tm_bootp_get_opt_len(optionPtr, 
                                                        TM_BOOT_TAGCODE_SIZE, 
                                                        byteOffset);
                    if (optionLength == 1)
                    {
                        tm_context(tvIpForward) = tm_bootp_get_char(
                            optionPtr, TM_BOOT_TAG_OFFSET, byteOffset );
                    }
/* point to next tag */
                    tm_bootp_next_option( optionPtr,
                                          optionLength+TM_BOOT_TAGCODELEN_SIZE,
                                          byteOffset );
                    break;
                default:
                    optionLength = tm_bootp_get_opt_len(optionPtr,
                                                        TM_BOOT_TAGCODE_SIZE,
                                                        byteOffset);
/* Unknown option - save it in the custom list */
                    custOptPtr = 
                        (ttBtUserCustOptPtr)tm_get_raw_buffer( 
                                        sizeof(ttBtUserCustOpt) +
                                        (ttPktLen)optionLength);
                    if (custOptPtr != (ttBtUserCustOptPtr)0)
                    {
                        custOptPtr->buoDataPtr    = (ttCharPtr)(custOptPtr + 1);
                        custOptPtr->buoDataLen    = (tt8Bit)optionLength;
                        custOptPtr->buoBootOption = optionName;
                        tm_bootp_byte_copy(optionPtr,
                                           *(custOptPtr->buoDataPtr),
                                           TM_BOOT_TAG_OFFSET,
                                           optionLength,
                                           byteOffset);
                        tfListAddToTail(&(btEntryPtr->btOfferEntryPtr
                                        ->btUserServCustOpts),
                                        (ttNodePtr)custOptPtr);
                    }
/* pass over variable tag data */
                    tm_bootp_next_option( optionPtr, 
                                          optionLength+TM_BOOT_TAGCODELEN_SIZE,
                                          byteOffset );
                    break;
                } /* switch */
            } /* while */
        }/* RFC 2132 if */
        if (tm_ip_zero(bootHdrPtr->bootYiaddr))
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfBootpRecvCB", "Bad Client IP address");
#endif /* TM_ERROR_CHECKING */
            goto bootpRecvCBExit;
        }
/* TFTP server name (for TFTP download) */
        optionLength = (int)tm_strlen(&bootHdrPtr->bootSname[0]);
#ifdef TM_DSP
        optionLength = tm_byte_count(optionLength);
#endif /* TM_DSP */
        if (optionLength > TM_BOOTSNAME_SIZE)
        {
            optionLength = TM_BOOTSNAME_SIZE;
        }
        (void)tfBtBootCopy(btEntryPtr, &bootHdrPtr->bootSname[0], 0,
                           optionLength,
                           tm_packed_byte_count(TM_BOOTSNAME_SIZE),
                           TM_BT_SNAME_INDEX);
/* TFTP boot file name (for TFTP download) */
        optionLength = (int)tm_strlen(&bootHdrPtr->bootFile[0]);
        if (optionLength > TM_BOOTFILENAME_SIZE)
        {
            optionLength = TM_BOOTFILENAME_SIZE;
        }
        (void)tfBtBootCopy(btEntryPtr, &bootHdrPtr->bootFile[0], 0,
                           optionLength,
                           tm_packed_byte_count(TM_BOOTFILENAME_SIZE),
                           TM_BT_FNAME_INDEX);
/* copy my ip address (Yiaddr) in btEntry */
        tm_ip_copy( bootHdrPtr->bootYiaddr,
                    btEntryPtr->btYiaddr);
/* TFTP server name (for TFTP download) */
        tm_ip_copy( bootHdrPtr->bootSiaddr,
                    btEntryPtr->btBootSIpAddress);
/*
 * Finish configuring the local route
 */
        tfBtRemoveRetryTimer(btEntryPtr);
/* Assume success. Prevent other configurations, */
/* and let the user take a pick at the BOOT entry (tfConfGetBootEntry()) */
        btEntryPtr->btDhcpState = TM_DHCPS_BOUND; 
#ifdef TM_USE_AUTO_IP
        devPtr = btEntryPtr->btDevEntryPtr;
        if (    (tm_4_ll_is_lan(devPtr))
             && (devPtr->devBtArpProbes >= 0) )
        {
            bootpParam.genVoidParmPtr = (ttVoidPtr)btEntryPtr;
/*
 * Make sure that there is not a collision detection started on that IP 
 * address 
 */
            (void)tfCancelCollisionDetection(
#ifdef TM_USE_STRONG_ESL
                                             devPtr,
#endif /* TM_USE_STRONG_ESL */
                                             bootHdrPtr->bootYiaddr);
/* Register the call back function for that IP address with the stack */
            errorCode = tfUseCollisionDetection(
#ifdef TM_USE_STRONG_ESL
                                                 devPtr,
#endif /* TM_USE_STRONG_ESL */
                                                 bootHdrPtr->bootYiaddr,
                                                 tfBootpFinish, bootpParam );
            if (errorCode == TM_ENOERROR)
            {
                errorCode = tfStartArpSend(
                    devPtr, 
                    bootHdrPtr->bootYiaddr,
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
                                                     bootHdrPtr->bootYiaddr);
                    errorCode = tfBtFinishConfig(btEntryPtr, TM_ENOERROR);
                }
            }
            else
            {
/* Collision in ARP table */
                (void)tfBtFinishConfig(btEntryPtr, errorCode);
            }
        }
        else
        {
            errorCode = tfBtFinishConfig(btEntryPtr, TM_ENOERROR);
        }
#else /* TM_USE_AUTO_IP */
        errorCode = tfBtFinishConfig(btEntryPtr, TM_ENOERROR);
#endif /* TM_USE_AUTO_IP */
        if (errorCode != TM_ENOERROR)
/* Configuration failed. Allow a new configuration to take place */
        {
            btEntryPtr->btDhcpState = TM_DHCPS_INIT;
        }
    }
bootpRecvCBExit:
    return;
}

#ifdef TM_USE_AUTO_IP
static int tfBootpFinish ( ttUserInterface    interfaceHandle,
                           ttUserIpAddress    ipAddress,
                           int                errorCode,
                           ttUserGenericUnion bootpParam )
{
    ttBtEntryPtr     btEntryPtr;
    ttDeviceEntryPtr devPtr;

    btEntryPtr = (ttBtEntryPtr)bootpParam.genVoidParmPtr;
    devPtr = (ttDeviceEntryPtr)interfaceHandle;
    (void)tfCancelCollisionDetection(
#ifdef TM_USE_STRONG_ESL
                                     interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                                     ipAddress);
/* Lock the device */
    tm_call_lock_wait(&(devPtr->devLockEntry));
    errorCode = tfBtFinishConfig(btEntryPtr, errorCode);
    if (errorCode != TM_ENOERROR)
    {
        btEntryPtr->btDhcpState = TM_DHCPS_INIT;
    }
/* UnLock the device */
    tm_call_unlock(&(devPtr->devLockEntry));
    return TM_ENOERROR;
}
#endif /* TM_USE_AUTO_IP */

#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */


/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4BootpDummy = 0;
#endif /* ! TM_USE_IPV4 */
