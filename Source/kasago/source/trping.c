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
 * Description: PING functions
 * Filename: trping.c
 * Author: Odile
 * Date Created: 09/09/98
 * $Source: source/trping.c $
 *
 * Modification History
 * $Revision: 6.0.2.7 $
 * $Date: 2016/03/29 15:43:39GMT $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/* Default (in seconds) interval between sending each ping packet */
#define TM_PING_DEFAULT_INTERVAL    1
/* Default (in milliseconds) interval between sending each ping packet */
#define TM_PING_DEFAULT_MS_INTERVAL TM_UL(1000)
/* Default PING data length */
#define TM_PING_DEFAULT_DATALENGTH  (tt16Bit) \
    ((tt16Bit)64 - (tt16Bit)tm_byte_count(TM_4PAK_ICMP_MIN_LEN))
/* Maximum ICMP data length */
#define TM_ICMP_MAX_DATALENGTH  (tt16Bit) \
    ((tt16Bit)0xFFFF - (tt16Bit)TM_4_IP_MIN_HDR_LEN)

/* Maximum PING data length */
#define TM_PING_MAX_DATALENGTH  (tt16Bit) \
    (TM_ICMP_MAX_DATALENGTH - (tt16Bit)tm_byte_count(TM_4PAK_ICMP_MIN_LEN))

/*
 * Maximum (at any given time) number of packets sent that we check
 * duplicates for.
 */
#define TM_PING_MAX_DUPS_CHK     (8 * 128) /* 1024 */

/*
 * bit number modulo TM_PING_MAX_DUPS_CHK
 * ( between 0 and TM_PING_MAX_DUPS_CHK-1 )
 */
#define tm_bit_modulo(icmpSeqNum) \
                    (tt16Bit)(icmpSeqNum % (tt16Bit)TM_PING_MAX_DUPS_CHK)
/*
 * map bit sequence number modulo TM_PING_MAX_DUPS_CHK to a byte offset in
 * array (yiels an offset value between 0 and TM_PING_MAX_DUPS_CHK/8 -1)
 */
#define tm_bit_to_byte_offset(bit1) \
                             (tt16Bit)(((tt16Bit)(bit1)) >> 3)
/* Bit mask in byte corresponding to a bit value */
#define tm_bit_mask_in_byte(bit1)    \
                (tt8Bit)(1 << (tt8Bit)(((tt16Bit)(bit1)) & 0x07))
/* Clear a bit using bit mask in byte1 */
#define tm_bit_clear(byte1, bitMask) ((tt8Bit) (byte1 & ~((tt8Bit)bitMask)))
/* Set a bit using bit mask in byte1 */
#define tm_bit_set(byte1, bitMask) ((tt8Bit)(byte1 | bitMask))
/* Test a bit using bit mask in byte1 */
#define tm_bit_test(byte1, bitMask) ((tt8Bit)(byte1 & bitMask))

/*
 * Set 8-bit value at an index in a packed buffer.
 */

#ifdef TM_DSP
#ifdef TM_LINT
LINT_EXPT_U_MACRO(tm_ping_put_char_index)
LINT_SHIFT_L_MACRO(tm_ping_put_char_index)
#endif /* TM_LINT */
#define tm_ping_put_char_index(buffer,index,dataChar)  \
{ \
    (buffer)[tm_packed_byte_count(index+1)-1] &= \
        (~(0xff << ((TM_DSP_ROUND_PTR - \
        (index % TM_DSP_BYTES_PER_WORD)) << 3))); \
    (buffer)[tm_packed_byte_count(index+1)-1] |= \
        ((dataChar) << ((TM_DSP_ROUND_PTR - \
        (index % TM_DSP_BYTES_PER_WORD)) << 3)); \
}

#ifdef TM_LINT
LINT_SHIFT_L_MACRO(tm_ping_get_char_index)
#endif /* TM_LINT */

#define tm_ping_get_char_index(buffer, index) \
    ((((buffer)[tm_packed_byte_count((index)+1)-1]) >> ((TM_DSP_ROUND_PTR - \
        ((index) % TM_DSP_BYTES_PER_WORD)) << 3)) & 0xff)

#else /* !TM_DSP */
#define tm_ping_put_char_index(buffer, index, dataChar) \
    ((buffer)[index] = dataChar)

#define tm_ping_get_char_index(buffer, index) \
    (buffer)[index]
#endif /* TM_DSP */

/*
 * Ping entry structure (pointed to by pointer in socket entry)
 */
typedef struct  tsPingEntry
{       
/* pointer to send timer structure */
    ttTimerPtr                   pgsTimerPtr;
/* Pointer given by the user to ping receive function call back */
    ttPingCBFuncPtr              pgsUserCBFuncPtr;
/* IP address of host that we are pinging */
    struct sockaddr_storage      pgsRemoteIpAddress;
/* Statistics info for the user */
    ttPingInfo                   pgsInfo;
/* unique identifier for this ping session */
    int                          pgsSocketDesc;
/* ICMP data length (user data length + ICMP header) */
    unsigned                     pgsIcmpDataLength;
/*
 * packed array indicating if we have received a packet
 * (for detecting duplicates)
 */
    tt8Bit                       pgsRecvd[TM_PING_MAX_DUPS_CHK/8];
} ttPingEntry;

typedef ttPingEntry TM_FAR * ttPingEntryPtr;

/*
 * ICMP header for an ICMP request or reply
 */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
typedef struct  tsIcmpPHeader
{       
    unsigned int icmChecksum : 16; /* ICMP checksum */
    unsigned int icmCode     :  8; /* ICMP code field */
    unsigned int icmType     :  8; /* ICMP type field */
    unsigned int icmSeqno    : 16; /* ICMP header request/reply sequence # */
    unsigned int icmIdent    : 16; /* ICMP header request/reply identifier */
} ttIcmpPHeader;
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
typedef struct  tsIcmpPHeader
{       
    unsigned int icmType     :8; /* ICMP type field */
    unsigned int icmCode     :8; /* ICMP code field */
    unsigned int icmChecksum;    /* ICMP checksum */
    unsigned int icmIdent;       /* ICMP header request/reply identifier */
    unsigned int icmSeqno;       /* ICMP header request/reply sequence number */
} ttIcmpPHeader; 
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
typedef struct  tsIcmpPHeader
{       
    tt8Bit  icmType; /* ICMP type field */
    tt8Bit  icmCode; /* ICMP code field */
    tt16Bit icmChecksum; /* ICMP checksum */
    tt16Bit icmIdent; /* ICMP header request/reply identifier */
    tt16Bit icmSeqno; /* ICMP header request/reply sequence number */
} ttIcmpPHeader;
#endif /* !TM_DSP */

typedef ttIcmpPHeader TM_FAR * ttIcmpPHeaderPtr;

/*
 * Local functions
 */

/* Ping Recv call back function */
static void tfPingRecvCB(int socketDescriptor, int flags);
/* Timer thread Ping send function */
TM_NEARCALL static void tfPingTimerSendPacket (ttVoidPtr      timerBlockPtr,
                            ttGenericUnion userParm1,
                            ttGenericUnion userParm2);
/* Ping send function (common between timer thread and user) */
static int tfPingSendPacket( ttSocketEntryPtr  socketEntryPtr );


/*
 * tfNgPingOpenStart
 * Function Description
 * This function opens an ICMP socket, and starts sending PING echo request
 * to a remote host, and start gathering statistics on the connection to
 * the remote host, until tfPingClose() is called.  Once this function
 * returns, statistics can be retrieved at any time with
 * tfPingGetStatistics(). After tfPingClose() is called, the user can no
 * longer have access to the Ping statistics.
 *
 * 1) Check validity of parameters (check for null remote host name pointer,
 *    negative ping interval, datalength too big, and invalid inet address).
 *    If one of the parameters is invalid, set return error code to
 *    TM_SOCKET_ERROR (with associated errorCode of TM_EINVAL). Otherwise, if
 *    pingDataLength parameter or pingMilliSecondsInterval parameter had not 
 *    been set by the user, use default values.
 * 5) Open a PING ICMP socket with a call to
 *    socket(AF_INET, SOCK_RAW, IPPROTO_ICMP ). If no error, register
 *    a receive socket call back.
 * 6) If no error get associated socket entry pointer, check it out, and
 *    lock it.
 * 7) If no error, allocate a PING entry.
 * 6) If no error occurred, initialize Ping entry fields, store ping entry
 *    pointer in socket entry pointer, call
 *    tfTimerAdd to execute tfPingTimerSendPacket (which sends a single
 *    ICMP ping packet to the remote host) every pingMilliSecondsInterval 
 *    (in milliseconds).
 *    Store timer pointer in our ping entry.
 * 8) If no error occurred, send first ICMP echo request.
 * 9) Check in and unlock socket pointer.
 * 9) If any error occured, close PING socket if any had been opened (this
 *    will free any allocated PING entry or timer), store errorCode for
 *    TM_SOCKET_ERROR socket descriptor, (if not done so already).
 * 10) return (pingSocketDesc on success, TM_SOCKET_ERROR on failure).
 *
 * Parameters
 * Parameter          Description
 * remoteAddrPtr      Remote IPv4 or IPv6 address to gather ICMP statistics
 *                    from.
 * pingMilliSecondsInterval
 *                    Interval in milliseconds at which to retry sending a Ping
 *                    packet (if 0, use default of 1 second (or 1000 ms)).
 * pingDataLength     Length of ping data.  If zero, use default of 56 bytes.
 * pingUserCBFuncPtr  Callback function when packet is received.
 *
 * Returns
 * Value            Meaning
 * >= 0             Socket descriptor
 * TM_SOCKET_ERROR  Socket error - specific error can be retrieved with
 *                  tfGetSocketError.
 */
TM_PROTO_EXTERN int tfNgPingOpenStart(
#ifdef TM_USE_STRONG_ESL
    ttUserInterface                         interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
    const struct sockaddr_storage TM_FAR *  remoteAddrPtr,
    ttUser32Bit                             pingMilliSecondsInterval,
    int                                     pingDataLength,
    ttPingCBFuncPtr                         pingUserCBFuncPtr )
{
    ttPingEntryPtr      pingEntryPtr;
    ttSocketEntryPtr    socketEntryPtr;
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr    devPtr;
#endif /* TM_USE_STRONG_ESL */
    int                 pingSocketDesc;
    int                 errorCode;
    int                 retCode;
    ttGenericUnion      timerParm1;
    ttGenericUnion      timerParm2;
    tt32Bit             maxDataLength;
    int                 defaultDataLength;
    int                 socProtoType;
    unsigned int        icmpHdrLength;
#ifdef TM_USE_BSD_DOMAIN
    int                 af;
#endif /* TM_USE_BSD_DOMAIN */
    int                 addrFamily;
    ttIpPort            localPortNumber;
    
/* Check on parameters */
    pingEntryPtr = (ttPingEntryPtr)0; /* assume failure */
#ifdef TM_USE_STRONG_ESL
    devPtr = (ttDeviceEntryPtr)interfaceHandle;
#endif /* TM_USE_STRONG_ESL */
    pingSocketDesc = TM_SOCKET_ERROR; /* assume failure */
    retCode = TM_SOCKET_ERROR; /* assume failure */
    errorCode = TM_ENOERROR; /* error Code not yet known */

/* Only initialize to avoid compiler warnings */    
    maxDataLength     = 0;
    defaultDataLength = 0;
    socProtoType      = 0;
    icmpHdrLength     = 0;
    
/* Make sure the user specified a remote host */    
    if ( remoteAddrPtr != (ttConstSockAddrStoragePtr)0 )
    {
#ifdef TM_USE_IPV6
        addrFamily = AF_INET6;
#ifdef TM_USE_IPV4
        if (remoteAddrPtr->ss_family == AF_INET6)
        {
            if (IN6_IS_ADDR_V4MAPPED(&remoteAddrPtr->addr.ipv6.sin6_addr))
            {
                addrFamily = AF_INET;
            }
            else if ( tm_6_ip_zero(&remoteAddrPtr->addr.ipv6.sin6_addr) )
            {
                errorCode = TM_EINVAL;
            }
        }
        else
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
        {
            addrFamily = AF_INET;

            if ( tm_ip_zero(remoteAddrPtr->addr.ipv4.sin_addr.s_addr) )
            {
                errorCode = TM_EINVAL;
            }
        }
#endif /* TM_USE_IPV4 */

        if (errorCode == TM_ENOERROR)
        {
#ifdef TM_USE_IPV6
            if ( addrFamily == AF_INET6 )
            {
                maxDataLength     = TM_6_PING_MAX_DATALENGTH;
                defaultDataLength = TM_6_PING_DEFAULT_DATALENGTH;
                socProtoType      = IPPROTO_ICMPV6;
                icmpHdrLength     = TM_6PAK_MIN_ICMP_MSG_LEN;
            }
#ifdef TM_USE_IPV4
            else
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
            {
                maxDataLength     = TM_PING_MAX_DATALENGTH;
                defaultDataLength = TM_PING_DEFAULT_DATALENGTH;
                socProtoType      = IPPROTO_ICMP;
                icmpHdrLength     = TM_4PAK_ICMP_MIN_LEN;
            }
#endif /* TM_USE_IPV4 */
        }

/* Ping Data length cannot be bigger than TM_PING_MAX_DATALENGTH */
        if (((tt32Bit) pingDataLength) > maxDataLength)
        {
            errorCode = TM_EINVAL;
        }
    }
    else
    {
        errorCode = TM_EINVAL;
        goto exitNgPingOpenStart;
    }
    
    if (errorCode == TM_ENOERROR)
    {
/*
 * Use default values if the user did not set pingDataLength or
 * pingMilliSecondsInterval
 */
        if (pingDataLength == 0)
        {
            pingDataLength = defaultDataLength;
        }
        else
        {
            if ((unsigned)pingDataLength < tm_byte_count(sizeof(tt32Bit)))
            {
                pingDataLength = tm_byte_count(sizeof(tt32Bit));
            }
        }
        if (pingMilliSecondsInterval == (ttUser32Bit)0)
        {
            pingMilliSecondsInterval = TM_PING_DEFAULT_MS_INTERVAL;
        }

/* Open an ICMP socket */
/* when we are using a IPv4-mapped IPv6 address, this must use ICMP, which
   only works over IPv4 */
        pingSocketDesc = socket(addrFamily,
                                SOCK_RAW,
                                socProtoType);
        if (pingSocketDesc != TM_SOCKET_ERROR)
        {
/* Register call back function */
            errorCode = tfRegisterSocketCB( pingSocketDesc,
                                            tfPingRecvCB,
                                            TM_CB_RECV
                                            | TM_CB_SOCKET_ERROR );
            if (errorCode == TM_SOCKET_ERROR)
            {
/* Get errorCode so that we can store it for TM_SOCKET_ERROR */
                errorCode = tfGetSocketError(pingSocketDesc);
            }
            else
            {
#ifdef TM_USE_STRONG_ESL
                if ( tfValidInterface(devPtr) == TM_ENOERROR )
                {
                    errorCode = setsockopt(pingSocketDesc,
                                           SOL_SOCKET,
                                           SO_BINDTODEVICE,
                                           (char *)(devPtr->devNameArray),
                                           IFNAMSIZ);
                    if (errorCode != TM_ENOERROR)
                    {
                        errorCode = tfGetSocketError(pingSocketDesc);
                        goto exitNgPingOpenStart;
                    }
                }
#endif /* TM_USE_STRONG_ESL */
/* Map to socket entry pointer */
#ifdef TM_USE_BSD_DOMAIN
                af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
                socketEntryPtr = tfSocketCheckValidLock(  pingSocketDesc
#ifdef TM_USE_BSD_DOMAIN
                                                        , &af
#endif /* TM_USE_BSD_DOMAIN */
                                                       );
                if (socketEntryPtr == (ttSocketEntryPtr)0)
                {
                    errorCode = TM_EBADF;
                }
                else
                {
                    localPortNumber = tfRandPortInsert(socketEntryPtr,
                                                &socketEntryPtr->socTuple, 0);
                    if (localPortNumber == TM_16BIT_ZERO)
                    {
                        errorCode = TM_EMFILE;
                    }
                    else
                    {
/* Allocate a Ping entry */
                        pingEntryPtr = (ttPingEntryPtr)tm_get_raw_buffer(
                                                        sizeof(ttPingEntry));
                        if (pingEntryPtr == (ttPingEntryPtr)0)
                        {
                            errorCode = TM_ENOBUFS;
                        }
                        else
                        {
/* Initialize Ping entry, and store in socket entry pointer */
                            tm_bzero(pingEntryPtr, sizeof(ttPingEntry));
                            pingEntryPtr->pgsSocketDesc = pingSocketDesc;
                            pingEntryPtr->pgsIcmpDataLength = (unsigned)
                                ((unsigned)pingDataLength + 
                                 tm_byte_count(icmpHdrLength)); 
                            pingEntryPtr->pgsUserCBFuncPtr = pingUserCBFuncPtr;
                            tm_bcopy( remoteAddrPtr,
                                      &pingEntryPtr->pgsRemoteIpAddress,
                                      sizeof(struct sockaddr_storage));
                        
                            socketEntryPtr->socApiEntryPtr = pingEntryPtr;
/*
 * Add timer
 * No possible deadlock since tfTimerAdd use a different lock than
 * tfTimerExecute
 */
                            timerParm1.genVoidParmPtr =
                                (ttVoidPtr) pingEntryPtr;
                            timerParm2.genIntParm = pingSocketDesc;
                            pingEntryPtr->pgsTimerPtr =
                                tfTimerAdd( tfPingTimerSendPacket,
                                            timerParm1,
                                            timerParm2,
                                            (tt32Bit)pingMilliSecondsInterval,
                                            TM_TIM_AUTO);
                            if ( pingEntryPtr->pgsTimerPtr == (ttTimerPtr)0 )
                            {
                                errorCode = TM_ENOBUFS;
                            }
                            else
                            {
/* Send first packet */
                                errorCode = tfPingSendPacket(socketEntryPtr);
                                if ((errorCode == TM_ENOERROR)
#ifdef TM_USE_IPSEC
                                    || (errorCode == TM_IPSEC_PACKET_QUEUED)
#ifdef TM_USE_IKE
#if TM_IKE_PACKET_MAX_QUEUE_BYTES == 0
                                    || (errorCode == TM_EPOLICYDISCARD)
#endif /* TM_IKE_PACKET_MAX_QUEUE_BYTES == 0 */
#endif /* TM_USE_IKE */
#endif /*TM_USE_IPSEC*/
                                   )
                                {
/* Success */
                                    retCode = pingSocketDesc;
                                }
                            }
                        }
                    }
                    (void)tfSocketReturn( socketEntryPtr,
                                          pingSocketDesc, TM_ENOERROR, 0);
                }
            }
        }
        
    }

 exitNgPingOpenStart:
    if (retCode == TM_SOCKET_ERROR)
    {
/*
 * If we failed, clean up, will close the socket if it has been opened,
 * and free associated pingEntryPtr and timerPtr if any
 */
        if ( pingSocketDesc != TM_SOCKET_ERROR )
        {
            (void)tfPingClose(pingSocketDesc);
        }
        if (errorCode != TM_ENOERROR)
        {
/* Record the socket error for TM_SOCKET_ERROR */
            tfSocketErrorRecord(TM_SOCKET_ERROR, errorCode);
        }
    }
    return retCode;
}


#ifdef TM_USE_IPV4
/*
 * tfPingOpenStart() function description
 *
 * Parameters
 * Parameter         Description
 * remoteHostName    hostname to gather ICMP statistics from (dotted quad
 *                   notation)
 * pingInterval      Interval in seconds at which to retry sending a Ping
 *                   packet (If 0, use default of 1 second)
 * pingMilliSecondsInterval (replaces pingInterval, if 
 *                   TM_PING_MILLISECONDS_INTERVAL is defined)
 *                   Interval in milliseconds at which to retry sending a Ping
 *                   packet (If 0, use default of 1 second)
 * pingDataLength    Length of Ping data. If 0, default to 56 bytes.
 * pingUserCBFuncPtr CB function when packet is received
 *
 * Returns
 * Value            Meaning
 * 0                success
 * TM_SOCKET_ERROR  failure
 *
 * Error can be retrieved with tfGetSocketError(socketDesc)
 *
 * TM_EINVAL    Error in one of the parameters
 * Other        As returned by socket() and tfRegisterSocketCB()
 */
int tfPingOpenStart (
#ifdef TM_USE_STRONG_ESL
                      ttUserInterface       interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                      char TM_FAR           *remoteHostName,
#ifdef TM_PING_MILLISECONDS_INTERVAL
                      ttUser32Bit           pingMilliSecondsInterval,
#else /* !TM_PING_MILLISECONDS_INTERVAL */
                      int                   pingInterval,
#endif /* !TM_PING_MILLISECONDS_INTERVAL */
                      int                   pingDataLength,
                      ttPingCBFuncPtr       pingUserCBFuncPtr)
{
    struct sockaddr_storage pingAddr;
    tt4IpAddress        remoteIpAddr;
#ifndef TM_PING_MILLISECONDS_INTERVAL
    ttUser32Bit         pingMilliSecondsInterval;
#endif /* !TM_PING_MILLISECONDS_INTERVAL */
    int                 errorCode;
    int                 retCode;

    retCode   = TM_SOCKET_ERROR;
    errorCode = TM_ENOERROR;
    
#ifndef TM_PING_MILLISECONDS_INTERVAL
/* Check for validity of pingInterval, and compute interval in milliseconds */
    if (pingInterval < 0)
    {
        errorCode = TM_EINVAL;
        goto exitPingOpenStart;
    }
    if (pingInterval == 0)
    {
        pingInterval = TM_PING_DEFAULT_INTERVAL;
    }
    pingMilliSecondsInterval = (ttUser32Bit)
                                    ((ttUser32Bit)pingInterval * TM_UL(1000));
#endif /* !TM_PING_MILLISECONDS_INTERVAL */
/*
 * Check validity of parameters (check for null remote host name pointer and
 * invalid inet address).
 */
    if ( remoteHostName == (char TM_FAR *)0 )
    {
        errorCode = TM_EINVAL;
        goto exitPingOpenStart;
    }
    else
    {
        remoteIpAddr = inet_aton(remoteHostName);
        if (tm_ip_zero(remoteIpAddr))
        {
            errorCode = TM_EINVAL;
            goto exitPingOpenStart;
        }
    }

/*
 * Create sockaddr_in structure with remote host address and the protocol
 * family set to IPv4.
 */ 
    pingAddr.addr.ipv4.sin_family = AF_INET;
    pingAddr.addr.ipv4.sin_len = sizeof(struct sockaddr_storage);
    tm_ip_copy( remoteIpAddr, pingAddr.addr.ipv4.sin_addr.s_addr );

    retCode = tfNgPingOpenStart(
#ifdef TM_USE_STRONG_ESL
                                 interfaceHandle,
#endif /* TM_USE_STRONG_ESL */
                                 &pingAddr,
                                 pingMilliSecondsInterval,
                                 pingDataLength,
                                 pingUserCBFuncPtr );

exitPingOpenStart:
    if (errorCode != TM_ENOERROR)
    {
/* Record the socket error for TM_SOCKET_ERROR */
        tfSocketErrorRecord(TM_SOCKET_ERROR, errorCode);
    }

    return retCode;
}
#endif /* TM_USE_IPV4 */

/*
 * tfPingRecvCB function description
 * This function is called whenever a ICMP packet is ready to be received.
 * It then processes the packet, checking to see if it's for one of
 * our user (ie - the ident field matches one ident field in the linked list
 * of ping entry). If it is, then record the time we received this packet,
 * and record all information.
 *
 * Internals
 * 1) If CB if for recv, get packet with tfZeroCopyRecvFrom. Check for error.
 * 2) Get associated socket entry pointer, lock it, and check it out.
 * 3) If successfull, get ping entry pointer, and ping info pointer.
 * 4) If we got a ping entry/info, and we received a packet
 *  4.1) BSD algorithm to check for duplicates:
 *       check if packet is a duplicate, by checking whether the ICMP
 *       sequence number bit is set in the received array. If it is not a
 *       duplicate packet set the bit in the array to indicate that we have
 *       received a packet corresponding to this sequence number.
 *  4.2) Keep track of the number of duplicates, and received packets in the
 *       ping info structure.
 *  4.3) Save tvTime as received time.
 *  4.4) Compute RTT from received time and sent time (in received data).
 *  4.5) Update pgiLastRtt, pgiSumRtt, pgiMaxRtt, pgiMinRtt, pgiAvgRtt in
 *       pgsInfo accordingly.
 * 5) If we got a ping entry/info, and we did not receive a packet, store
 *    socket CB error, or error occured while either receiving a packet in
 *    the Ping info entry.
 * 6) If we got a ping entry/info, save user function call back from ping
 *    entry.
 * 7) check in and unlock socket.
 * 8) Free zero copy buffer using tfFreeSharedBuffer.
 * 9) If the user provided a user function call back, call it, passing
 *    the socket descriptor to let the user know that an incoming
 *    ICMP echo reply came, or that an error occured.
 *
 * Parameters
 * Parameter         Description
 * socketDescriptor  ICMP socket
 * flags             TM_CB_RECV
 */
static void tfPingRecvCB(int pingSocketDesc, int flags)
{
    struct sockaddr_storage fromAddress;
    ttPingInfoPtr       pingInfoPtr;
    ttPingEntryPtr      pingEntryPtr;
    ttSocketEntryPtr    socketEntryPtr;
    ttPacketPtr         packetPtr;
    ttCharPtr           bufferPtr;
    ttIcmpPHeaderPtr    icmphPtr;
    ttPingCBFuncPtr     pingUserCBFuncPtr;
    tt32Bit             currentTime;
    tt32Bit             pingTime;
    ttS32Bit            rtTime;
    int                 bufferLen;
    int                 errorCode;
    tt32Bit             maxDataLength;
    tt16Bit             icmpSeqNo;
    tt16Bit             byteOffset;
    tt8Bit              byteValue;
    tt8Bit              bitMask;
    unsigned int        icmpHdrLength;
    int                 fromAddressLength;
#ifdef TM_USE_BSD_DOMAIN
    int                 af;
#endif /* TM_USE_BSD_DOMAIN */

    pingUserCBFuncPtr = (ttPingCBFuncPtr)0;
    bufferPtr = (ttCharPtr)0;
    packetPtr = TM_PACKET_NULL_PTR;
    errorCode = TM_ENOERROR;
    fromAddressLength = sizeof(struct sockaddr_storage);
    maxDataLength = TM_ICMP_MAX_DATALENGTH;
    if (flags & TM_CB_RECV)
    {
        fromAddressLength = sizeof(fromAddress);
        bufferLen = tfZeroCopyRecvFrom(
                pingSocketDesc,
                (ttUserMessage TM_FAR *)(ttVoidPtr)&packetPtr,
                (ttCharPtr     TM_FAR *)&bufferPtr,
                (int)maxDataLength,
                MSG_DONTWAIT,
                (struct sockaddr TM_FAR *) &fromAddress,
                &fromAddressLength);
        if (bufferLen == TM_SOCKET_ERROR)
        {
            errorCode = tfGetSocketError(pingSocketDesc);
        }
    
        if (flags & TM_CB_SOCKET_ERROR)
        {
/* we received an ICMP error message in response to our Ping */
            bufferPtr = (ttCharPtr)0;
        }
    }
    else
    {
        tm_bzero(&fromAddress, sizeof(fromAddress));
    }

#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckValidLock(  pingSocketDesc
#ifdef TM_USE_BSD_DOMAIN
                                            , &af
#endif /* TM_USE_BSD_DOMAIN */
                                           );
    if (socketEntryPtr != (ttSocketEntryPtr)0)
    {
        pingEntryPtr = (ttPingEntryPtr)socketEntryPtr->socApiEntryPtr;
        if (pingEntryPtr != (ttPingEntryPtr)0)
        {
            pingInfoPtr = &pingEntryPtr->pgsInfo;
            if (bufferPtr != (ttCharPtr)0)
            {
                icmphPtr = (ttIcmpPHeaderPtr)bufferPtr;
                icmpSeqNo = (tt16Bit)icmphPtr->icmSeqno;
/*
 * Byte offset in the array corresponding to icmpSeqNo (modulo
 * TM_PING_MAX_DUPS_CHK)
 */
                byteOffset = tm_bit_to_byte_offset(icmpSeqNo); 
/*
 * Bit mask in the byte corresponding to icmpSeqNo (modulo
 * TM_PING_MAX_DUPS_CHK)
 */
                bitMask = tm_bit_mask_in_byte(icmpSeqNo); 
/* Byte value at byte offset in received array */
                byteValue = (tt8Bit)tm_ping_get_char_index(
                    pingEntryPtr->pgsRecvd, byteOffset);

/* BSD algorithm to check for duplicates: */
                if (tm_bit_test(byteValue, bitMask)) 
                {
                    pingInfoPtr->pgiDuplicated++;
                }
                else
                {
/*
 * Set bit for packet ID being sent, to mark it as already being received
 */
                    tm_ping_put_char_index( pingEntryPtr->pgsRecvd,
                                            byteOffset,
                                            tm_bit_set(byteValue, bitMask) );
                    pingInfoPtr->pgiReceived++;
                }

#ifdef TM_USE_IPV6
                icmpHdrLength = TM_6PAK_MIN_ICMP_MSG_LEN;
#ifdef TM_USE_IPV4
                if (fromAddress.ss_family == AF_INET6)
                {
                    if (IN6_IS_ADDR_V4MAPPED(&fromAddress.addr.ipv6.sin6_addr))
                    {
                        icmpHdrLength = TM_4PAK_ICMP_MIN_LEN;
                    }
                }
                else
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */
                {
#ifdef TM_USE_IPV4
                    icmpHdrLength = TM_4PAK_ICMP_MIN_LEN;
#endif /* TM_USE_IPV4 */
                }
                
                pingTime = *((tt32BitPtr)&bufferPtr[icmpHdrLength]); 
                tm_kernel_set_critical;
                currentTime = tvTime;
                tm_kernel_release_critical;
                rtTime = (ttS32Bit)currentTime - (ttS32Bit)pingTime;
                pingInfoPtr->pgiLastRtt = (ttUser32Bit)rtTime;
                if (   (pingInfoPtr->pgiMinRtt > (ttUser32Bit)rtTime)
                    || (pingInfoPtr->pgiMinRtt == TM_UL(0)) )
                {
                    pingInfoPtr->pgiMinRtt = (ttUser32Bit)rtTime;
                }
                if (pingInfoPtr->pgiMaxRtt < (ttUser32Bit)rtTime)
                {
                    pingInfoPtr->pgiMaxRtt = (ttUser32Bit)rtTime;
                }
                pingInfoPtr->pgiSumRtt += (ttUser32Bit)rtTime;
                pingInfoPtr->pgiAvgRtt =     pingInfoPtr->pgiSumRtt
                                           / (   pingInfoPtr->pgiReceived
                                               + pingInfoPtr->pgiDuplicated);
                tm_bcopy( &fromAddress,
                          &pingInfoPtr->pgiPeerSockAddr,
                          fromAddressLength );   
            }
            else
            {
                if (pingInfoPtr->pgiRecvErrorCode == TM_ENOERROR)
                {
                    if (flags & TM_CB_SOCKET_ERROR)
                    {
                        errorCode = socketEntryPtr->socRecvError;
                    }
                    pingInfoPtr->pgiRecvErrorCode = errorCode;
                }
            }
            if (pingEntryPtr->pgsUserCBFuncPtr != (ttPingCBFuncPtr)0)
            {
                pingUserCBFuncPtr = pingEntryPtr->pgsUserCBFuncPtr;
            }
        }
        (void)tfSocketReturn(socketEntryPtr, pingSocketDesc, TM_ENOERROR, 0);
    }

    if (packetPtr != TM_PACKET_NULL_PTR)
    {
/* Free the zero copy buffer */
        tfFreeSharedBuffer(packetPtr, TM_SOCKET_UNLOCKED);
    }
    if (pingUserCBFuncPtr != (ttPingCBFuncPtr)0)
    {
        (*pingUserCBFuncPtr)(pingSocketDesc);
    }
}


/*
 * tfPingClose Function Description
 * This function stops the ping requests for the passed socket
 * descriptor. The socket is closed, the associated ping Entry structure
 * is deallocated, and the associated timer deleted.
 *
 * Internals
 * 1) Get and lock socket entry pointer associated with pingSocketDesc.
 * 2) if successfull, save ping entry pointer and timer pointer.
 * 3) unlock the socket entry pointer.
 * 4) If there was no error getting the socket entry pointer,
 *    close the socket
 * 5) If timer pointer was non nil, call tfTimerLockedRemove to delete
 *    the timer. (Could not do it while the socket entry was locked)
 * 6) If Ping entry was non null, free the entry. Safe to do so now since
 *    the timer can no longer fire, and hence no longer access the pointer.
 *
 * Parameters
 * Parameter        Description
 * pingSocketDesc   Ping socket descriptor
 *
 * Returns
 * Value            Meaning
 * TM_SOCKET_ERROR  same as tfClose
 * 0                success
 */

int tfPingClose (int pingSocketDesc)
{
    ttSocketEntryPtr socketEntryPtr;
    ttPingEntryPtr   pingEntryPtr;
    ttTimerPtr       timerPtr;
    int              retCode;
#ifdef TM_USE_BSD_DOMAIN
    int              af;
#endif /* TM_USE_BSD_DOMAIN */

    timerPtr = (ttTimerPtr)0;
    pingEntryPtr = (ttPingEntryPtr)0;
    retCode = TM_SOCKET_ERROR;
/*
 * Retrieve socket Entry pointer, to get access to timer and ping entry
 * pointers
 */
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckValidLock(  pingSocketDesc
#ifdef TM_USE_BSD_DOMAIN
                                            , &af
#endif /* TM_USE_BSD_DOMAIN */
                                           );
    if (socketEntryPtr != (ttSocketEntryPtr)0)
    {
        pingEntryPtr = (ttPingEntryPtr)socketEntryPtr->socApiEntryPtr;
        if (pingEntryPtr != (ttPingEntryPtr)0)
        {
/* Disable timer on it to stop gathering information on it */
            if (pingEntryPtr->pgsTimerPtr != (ttTimerPtr)0)
            {
                timerPtr = pingEntryPtr->pgsTimerPtr;
            }
            socketEntryPtr->socApiEntryPtr = (ttPingEntryPtr)0;
        }
        (void)tfSocketReturn(socketEntryPtr, pingSocketDesc, TM_ENOERROR, 0);
        retCode = tfClose(pingSocketDesc);
/* Clean up */
        if (pingEntryPtr != (ttPingEntryPtr)0)
        {
/* Free Timer and PING entry */
            if (timerPtr != (ttTimerPtr)0)
            {
/*
 * remove the timer
 */
                tfTimerRemove(timerPtr);
            }
            tm_free_raw_buffer((ttRawBufferPtr)pingEntryPtr);
        }
    }
    else
    {
        retCode = tfSocketReturn(socketEntryPtr, pingSocketDesc, TM_EBADF, 0);
    }
    return retCode;
}

/*
 * tfPingGetStatistics Function Description
 * This function returns a pointer to the statistics gathered since
 * previous call to tfPingOpenStart
 *
 * 1) Map socket descriptor to socket pointer, lock and check out.
 * 2) If successfull, copy data from the PingEntry to pingInfoPtr.
 * 3) check in and unlock socket pointer.
 *
 * Parameters
 * Parameter       Description
 * pingSocketDesc  Ping socket descriptor
 * pingInfoPtr     pointer to a structure that will be filled with
 *                 relevant information
 *
 * Returns
 * Value           Meaning
 * 0               success
 * TM_SOCKET_ERROR failure
 *
 * User can retrieve error code by calling tfGetSocketError(pingSocketDesc)
 * TM_EINVAL       bad pingInfoPtr, or no Ping entry
 * Other           socket error
 */
int tfPingGetStatistics ( int           pingSocketDesc,
                          ttPingInfoPtr pingInfoPtr )
{
    ttPingEntryPtr   pingEntryPtr;
    ttSocketEntryPtr socketEntryPtr;
    int              errorCode;
    int              retCode;
#ifdef TM_USE_BSD_DOMAIN
    int              af;
#endif /* TM_USE_BSD_DOMAIN */

    retCode = TM_SOCKET_ERROR; /* assume failure */
/* Lock */
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckValidLock(  pingSocketDesc
#ifdef TM_USE_BSD_DOMAIN
                                            , &af
#endif /* TM_USE_BSD_DOMAIN */
                                           );
    if (socketEntryPtr != (ttSocketEntryPtr)0)
    {
        errorCode = TM_ENOERROR; /* error Code not yet known */
        pingEntryPtr = (ttPingEntryPtr)socketEntryPtr->socApiEntryPtr;
        if (     (pingEntryPtr == (ttPingEntryPtr)0)
              || (pingInfoPtr == (ttPingInfoPtr)0) )
        {
            errorCode = TM_EINVAL;
        }
        else
        {
            tm_bcopy( &pingEntryPtr->pgsInfo, pingInfoPtr,
                      sizeof(ttPingInfo));
/* Reset error codes in socket Ping info structure */
            pingEntryPtr->pgsInfo.pgiSendErrorCode = TM_ENOERROR;
            pingEntryPtr->pgsInfo.pgiRecvErrorCode = TM_ENOERROR;
            retCode = TM_ENOERROR; /* no error */
        }
    }
    else
    {
        errorCode = TM_EBADF;
    }
/* Unlock, save errorCode */
    return tfSocketReturn( socketEntryPtr, pingSocketDesc, errorCode,
                           retCode );
}



/*
 * Function Description
 * This function is called from the timer execute thread.
 * We lock the Ping, and then call tfPingTimerSendPacket()
 *
 * Internals
 * 1) Map from ping Entry pointer to socket Entry Pointer, and
 *    lock the socket entry.
 * 2) Call tfPingSendPacket if socket entry corresponds to ping entry.
 * 3) If tfPingSendPacket returned an error, store it in the
 *    Ping entry structure.
 * 3) unlock the socket entry
 *
 * Parameters
 * Parameter       Description
 * voidPtr         a pointer to a PING entry
 *
 * Returns
 * Value   Meaning
 * None
 */
static void tfPingTimerSendPacket(ttVoidPtr      timerBlockPtr,
                                  ttGenericUnion userParm1,
                                  ttGenericUnion userParm2)
{
    ttSocketEntryPtr    socketEntryPtr;
    ttPingInfoPtr       pingInfoPtr;
    int                 pingSocketDesc;
    int                 errorCode;
#ifdef TM_USE_BSD_DOMAIN
    int                 af;
#endif /* TM_USE_BSD_DOMAIN */

    pingSocketDesc = userParm2.genIntParm;
#ifdef TM_USE_BSD_DOMAIN
    af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
    socketEntryPtr = tfSocketCheckValidLock(  pingSocketDesc
#ifdef TM_USE_BSD_DOMAIN
                                            , &af
#endif /* TM_USE_BSD_DOMAIN */
                                           );
    if (socketEntryPtr != TM_SOCKET_ENTRY_NULL_PTR)
    {
/* verify that we haven't been removed in another task */
        if (tm_timer_not_reinit((ttTimerPtr) timerBlockPtr))
        {
            errorCode = TM_ENOERROR;
            if ( (socketEntryPtr->socApiEntryPtr == userParm1.genVoidParmPtr) )
            {
                pingInfoPtr =
                    &((ttPingEntryPtr)(userParm1.genVoidParmPtr))->pgsInfo;
                errorCode = tfPingSendPacket(socketEntryPtr);
                if (    (errorCode != TM_ENOERROR)
#ifdef TM_USE_IPSEC
                        && (errorCode != TM_IPSEC_PACKET_QUEUED)
#endif /*TM_USE_IPSEC*/
                        && (pingInfoPtr->pgiSendErrorCode == TM_ENOERROR) )
                {
                    pingInfoPtr->pgiSendErrorCode = errorCode;
                }
            }
        }

        (void)tfSocketReturn(socketEntryPtr, pingSocketDesc, TM_ENOERROR, 0);
    }
    return;
}

/*
 * Function Description
 * This function is called from either the timer execute thread or from
 * tfPingOpenStart() by the user with the socket entry checked out and
 * locked.
 * We fill an ICMP packet with the appropriate information
 * (including checksum, unique number and timestamp) and then sends it
 * to the remote host.
 *
 * Internals
 * 1) allocate zero copy buffer with tfGetZeroCopyBuffer.
 * 2) If sucessfull:
 *  2.1 BSD algorithm to check for duplicates:
 *      Increase number of transmitted packets. Compute
 *      a unique ICMP seq number modulo TM_PING_MAX_DUPS_CHK, and clear
 *      corresponding bit in TM_PING_MAX_DUPS_CHK bit received array (to
 *      be set upon receiving the echo reply so that we can keep track of
 *     duplicated echo replies).
 * 2.2 Use socket random port for ICMP identification number
 * 2.3 Fill in ICMP header fields, and add a time stamp in the data portion.
 * 2.4 unlock the Ping prior to sending the packet
 * 2.5 Compute checksum using tfPacketChecksum, and set the corresponding
 *     field in the ICMP header.
 * 2.6 Send data using tfZeroCopySendto.
 * 2.7 If error occured, retrieve errorCode
 * 2.8 re-lock the socket entry
 * 3) return errorCode
 *
 * Parameters
 * Parameter       Description
 * voidPtr         a pointer to a PING entry
 *
 * Returns
 * Value      Meaning
 * 0          Success
 * TM_ENOBUFS no buffer
 * Other      as retrieved from failed tfZeroCopySendTo()
 */
static int tfPingSendPacket( ttSocketEntryPtr  socketEntryPtr )
{
    struct sockaddr_storage TM_FAR * remoteAddrPtr;
    ttPingEntryPtr      pingEntryPtr;
    ttPingInfoPtr       pingInfoPtr;
#ifdef TM_USE_IPV4
    ttIcmpPHeaderPtr    icmphPtr;
#endif /* TM_USE_IPV4 */
    ttPacketPtr         packetPtr;
    tt16BitPtr          pshPtr;
#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
    struct sockaddr_storage ipv4SockAddr;
#endif /* TM_USE_IPV4 && TM_USE_IPV6 */
#ifdef TM_USE_IPV6
    tt6IcmpHeaderPtr    icmp6hPtr;
    tt6PseudoHeader     pseudoHdr;
    int                 addrFamily;
#endif /* TM_USE_IPV6 */
    int                 errorCode;
    int                 retCode;
    int                 pingSocketDesc;
    unsigned int        icmpHdrLength;
    tt32Bit             currentTime;
    ttPktLen            icmpDataLength;
    tt16Bit             icmpSeqNo;
    tt16Bit             byteOffset;
    tt16Bit             pshLength;
    tt16Bit             checksum;
#ifdef TM_4_USE_SCOPE_ID
    tt16Bit             peerDevIndex;
#endif /* TM_4_USE_SCOPE_ID */
    ttIpPort            ourLayer4Port;
    tt8Bit              byteValue;
    tt8Bit              bitMask;
    tt8Bit              freePacket;

    errorCode = TM_ENOERROR;
    pingEntryPtr = (ttPingEntryPtr)socketEntryPtr->socApiEntryPtr;
    pingSocketDesc = pingEntryPtr->pgsSocketDesc;
    pingInfoPtr = &pingEntryPtr->pgsInfo;
    remoteAddrPtr = &pingEntryPtr->pgsRemoteIpAddress;
#ifdef TM_USE_IPV4
    icmphPtr = (ttIcmpPHeaderPtr) 0; /* fix compiler warning */
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    icmp6hPtr = (tt6IcmpHeaderPtr) 0; /* fix compiler warning */
#endif /* TM_USE_IPV6 */
    freePacket = TM_8BIT_NO;

#ifdef TM_USE_IPV6
    addrFamily = AF_INET6;
#ifdef TM_USE_IPV4
    if (remoteAddrPtr->ss_family == AF_INET6)
    {
        if (IN6_IS_ADDR_V4MAPPED(&remoteAddrPtr->addr.ipv6.sin6_addr))
        {
            addrFamily = AF_INET;
/* The socket address family must match the protocol family stored on the
   socket. Since ICMP can only be used over IPv4, and ICMPv6 only over IPv6,
   when the user specifies IPv4-mapped IPv6 format with ping we need to
   translate the IPv6 format socket address to an IPv4 format address */
            tm_bzero(&ipv4SockAddr, sizeof(ipv4SockAddr));
            ipv4SockAddr.addr.ipv4.sin_family = AF_INET;
            ipv4SockAddr.addr.ipv4.sin_len = sizeof(ipv4SockAddr);
            ipv4SockAddr.addr.ipv4.sin_addr.s_addr = 
                tm_4_ip_addr(remoteAddrPtr->addr.ipv6.sin6_addr);
            remoteAddrPtr = &ipv4SockAddr;
#ifdef TM_SNMP_MIB
/* Number of ICMP Echo (request) messages sent. */
            tm_context(tvIcmpData).icmpOutEchos++;
#endif /* TM_SNMP_MIB */
        }
#ifdef TM_SNMP_MIB
        else
        {
            tm_context(tvIcmpv6Data).icmpv6OutEchos++;
        }
#endif /* TM_SNMP_MIB */
    }
    else
    {
        addrFamily = AF_INET;
#ifdef TM_SNMP_MIB
        tm_context(tvIcmpData).icmpOutEchos++;
#endif /* TM_SNMP_MIB */
    }
#endif /* TM_USE_IPV4 */
#else  /* TM_USE_IPV6 */
#ifdef TM_SNMP_MIB
    tm_context(tvIcmpData).icmpOutEchos++;
#endif /* TM_SNMP_MIB */
#endif /* TM_USE_IPV6 */

/* User data + ICMP header length */
    icmpDataLength = (ttPktLen)pingEntryPtr->pgsIcmpDataLength;
/*
 * We first allocate a zero copy ICMP buffer of length corresponding to an
 * ICMP send request (user data + ICMP header length)
 */
    packetPtr =
        tfGetSharedBuffer(
            TM_MAX_SEND_HEADERS_SIZE, icmpDataLength, TM_16BIT_ZERO);
    
    if (packetPtr == (ttPacketPtr)0)
    {
#ifdef TM_SNMP_MIB
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
        if (addrFamily == AF_INET)
        {
            tm_context(tvIcmpData).icmpOutErrors++;
        }
        else
#endif /* TM_USE_IPV4 */
        {
            tm_context(tvIcmpv6Data).icmpv6OutErrors++;
        }
#else  /* TM_USE_IPV6 */
        tm_context(tvIcmpData).icmpOutErrors++;
#endif /* TM_USE_IPV6 */
#endif /* TM_SNMP_MIB */
        errorCode = TM_ENOBUFS;
    }
    else
    {
        tm_bzero(packetPtr->pktLinkDataPtr, icmpDataLength);
/* Mark packet as zero copy packet */
        packetPtr->pktUserFlags = TM_PKTF_USER_OWNS;
        icmpSeqNo = (tt16Bit)pingInfoPtr->pgiTransmitted;
        pingInfoPtr->pgiTransmitted++;
/*
 * Limit our ICMP sequence number between 0 and TM_PING_MAX_DUPS_CHK-1,
 * since we only keep track of the last TM_PING_MAX_DUPS_CHK sent PING
 * ICMP echo request at any given time.
 */
        icmpSeqNo = tm_bit_modulo(icmpSeqNo); /* modulo TM_PING_MAX_DUPS_CHK */
        {
/*
 * BSD algorithm to check for duplicates:
 * Each bit in the TM_PING_MAX_DUPS_CHK/8 bytes (or TM_PING_MAX_DUPS_CHK
 * bits) pgsRecvd array correspond to one sequence number being sent
 * (betwen 0, and TM_PING_MAX_DUPS_CHK-1). The bit position
 * in the array is given by the value of the ICMP sequence number.
 * We clear the bit corresponding to icmpSeqNo upon sending an ICMP echo
 * request, it will be set upon receiving the echo reply, so that we can
 * check for duplicates echo replies later on.
 */
/* Byte offset in the array corresponding to icmpSeqNo */
            byteOffset = tm_bit_to_byte_offset(icmpSeqNo); 
/* Bit mask in the byte corresponding to icmpSeqNo */
            bitMask = tm_bit_mask_in_byte(icmpSeqNo); 
/* Byte value at byte offset in received array */
            byteValue = (tt8Bit)tm_ping_get_char_index(pingEntryPtr->pgsRecvd,
                                               byteOffset);

/*
 * Clear bit for packet ID being sent in received array. That bit will be
 * set upon receiving the corresponding echo reply.
 */
            tm_ping_put_char_index( pingEntryPtr->pgsRecvd,
                                    byteOffset,
                                    tm_bit_clear(byteValue, bitMask) );
                                        
        }

        ourLayer4Port = socketEntryPtr->socOurLayer4Port;

/* Unlock */
        tm_unlock(&(socketEntryPtr->socLockEntry));

        pshPtr = (tt16BitPtr) 0; /* initialized here for compiler warning */
        pshLength = 0; /* initialized here for compiler warning */
#ifdef TM_USE_IPV6
        icmpHdrLength = TM_6PAK_MIN_ICMP_MSG_LEN;
        if (addrFamily == AF_INET6)
        {
            icmp6hPtr = (tt6IcmpHeaderPtr)packetPtr->pktLinkDataPtr;

/* ICMPv6 type ECHO request */            
            icmp6hPtr->icm6Type = TM_6_ICMP_TYPE_ECHO_REQUEST;
            icmp6hPtr->icm6Code = 0;

/*
 * ICMPv6 sequence number (modulo TM_PING_MAX_DUPS_CHK) to check for
 * duplicates
 */
            icmp6hPtr->icm6EchoSeqno = icmpSeqNo;
/* ICMP identification number (Port) */            
            icmp6hPtr->icm6EchoIdent = ourLayer4Port;
            icmp6hPtr->icm6Checksum  = 0; 
            
            pseudoHdr.psh6Layer4Len =
                htonl((tt32Bit) packetPtr->pktChainDataLength);
/* ICMPv6 includes a pseudo header for checksum calculation (below) */
            tm_6_zero_arr(pseudoHdr.psh6ZeroArr);
            pseudoHdr.psh6Protocol  = IPPROTO_ICMPV6;
            pshPtr = (tt16BitPtr) &pseudoHdr;
            pshLength = sizeof(pseudoHdr);
        }
        else
#endif /* TM_USE_IPV6 */
        {
#ifdef TM_4_USE_SCOPE_ID
            if (pingEntryPtr->pgsRemoteIpAddress.ss_family == AF_INET6)
            {
/* IPv4-mapped IPv6 address case */                
                if (IN4_IS_ADDR_LINKLOCAL(
                        remoteAddrPtr->addr.ipv4.sin_addr.s_addr))
                {
                    peerDevIndex = (tt16Bit)
                        pingEntryPtr->pgsRemoteIpAddress.addr.ipv6.sin6_scope_id;
                    if (peerDevIndex
                        > ((int)tm_context(tvAddedInterfaces) & 0xFFFF))
                    {
/* scope ID is bogus */                        
                        errorCode = TM_EINVAL;
                        freePacket = TM_8BIT_YES;
                        goto exitPingSendPacket;
                    }

/* set outgoing interface on packet from scope ID */
                    packetPtr->pktDeviceEntryPtr =
                        tm_context(tvDevIndexToDevPtr)[peerDevIndex];
                    packetPtr->pktMhomeIndex = TM_16BIT_ZERO;
                    packetPtr->pktFlags |= TM_OUTPUT_DEVICE_FLAG;
                }
            }
#endif /* TM_4_USE_SCOPE_ID */

#ifdef TM_USE_IPV4
            icmphPtr = (ttIcmpPHeaderPtr)packetPtr->pktLinkDataPtr;
            
/* ICMP type ECHO request */
            icmphPtr->icmType = TM_ICMP_TYPE_ECHO_REQ;
            icmphPtr->icmCode = 0; /* ICPM code 0 */
/*
 * ICMP sequence number (modulo TM_PING_MAX_DUPS_CHK) to check for
 * duplicates
 */
            icmphPtr->icmSeqno = icmpSeqNo;
/* ICMP identification number (Port) */
            icmphPtr->icmIdent = ourLayer4Port;

            icmphPtr->icmChecksum = 0;
            icmpHdrLength = TM_4PAK_ICMP_MIN_LEN;
/* No pseudo header for ICMPv4 */            
#endif /* TM_USE_IPV4 */
        }

        tm_kernel_set_critical;
        currentTime = tvTime;
        tm_kernel_release_critical;
        
        *((tt32BitPtr)&(packetPtr->pktLinkDataPtr[icmpHdrLength]))
                                                        = currentTime; 
/* Compute the ICMP checksum */
        checksum = tfPacketChecksum( packetPtr,
                                     icmpDataLength,
                                     pshPtr,
                                     pshLength);

#ifdef TM_USE_IPV6
        if (addrFamily == AF_INET6)
        {
            icmp6hPtr->icm6Checksum = checksum;
        }
        else
#endif /* TM_USE_IPV6 */
        {
#ifdef TM_USE_IPV4
            icmphPtr->icmChecksum = checksum;            
#endif /* TM_USE_IPV4 */
        }
        
        retCode = tfZeroCopySendTo( pingSocketDesc,
                                    packetPtr,
                                    (int) icmpDataLength,
                                    MSG_DONTWAIT,
                                    (struct sockaddr TM_FAR *) remoteAddrPtr,
                                    remoteAddrPtr->ss_len );
/* relock */
        tm_lock_wait(&socketEntryPtr->socLockEntry);
        if (retCode == TM_SOCKET_ERROR)
        {
/* Map TM_SOCKET_ERROR to error code */
            errorCode = tfGetSocketError(pingSocketDesc);
            if (errorCode == TM_EWOULDBLOCK)
            {
                freePacket = TM_8BIT_YES;
            }
        }
    }
#ifdef TM_4_USE_SCOPE_ID
exitPingSendPacket:    
#endif /* TM_4_USE_SCOPE_ID */

    if (freePacket == TM_8BIT_YES)
    {
        tfFreeSharedBuffer(packetPtr, TM_SOCKET_LOCKED);
    }

    return errorCode;
}
