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
 * Description: TCP I/O functions
 * Filename: trtcpio.c
 * Author: Odile
 * Date Created: 12/10/08
 * $Source: source/trtcpio.c $
 *
 * Modification History
 * $Revision: 6.0.2.42 $
 * $Date: 2015/06/08 17:10:04JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#define TM_USE_TCP_IO

#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_TCP
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

#ifdef TM_USE_SSL
#include <trssl.h>
#endif /* TM_USE_SSL */

#ifdef TM_TCP_SACK
/* TCP Selective ACK block size (8bytes, 2 SQN) */
#define TM_SACK_BLOCK_LENGTH    8

#define TM_TCPO_SACK           (tt8Bit)0x05 /* SACK option */

/* SACK first word padding length */
#define TM_TCPO_SACK_FW_PADDED (tt16Bit)2
/* SACK first word length without padding */
#define TM_TCPO_SACK_FW_LENGTH (tt16Bit)2
/* SACK One SQN length (1 4-bytes sequence numbers) */
#define TM_TCPO_SACK_ONE_SQN   (tt16Bit)4
#endif /* TM_TCP_SACK */

/* 24 days in milliseconds:  24 * 24 * 60 * 60 * 1000 */
#define TM_24DAYS_MS                TM_UL(2073600000)

#define TM_DEVO_XSUM_CACHED     0x4000
#define tm_tcp_send_xsum_cached(offloadFlags)                          \
    tm_16bit_one_bit_set(offloadFlags, TM_DEVO_XSUM_CACHED)

/* Bits used for segment based event processing in TCP state machine */
#define TM_TCPE_FLAGS  ((tt8Bit)(TM_TCP_ACK|TM_TCP_RST|TM_TCP_SYN|TM_TCP_FIN))

/* Flag mask used to extract event */
#define tm_tcpe_flag_mask(flags)  (tt8Bit)(((tt8Bit)(flags)) & TM_TCPE_FLAGS)

/* Use dataTcpHdrPtr as temporary holder for back pointer to TCP header */
#define tm_tcp_set_hdr_ptr(packetPtr, tcphPtr) \
                      packetPtr->pktSharedDataPtr->dataTcpHdrPtr = tcphPtr;

/* BEGINNING of network headers */
#ifdef TM_TCP_SACK
/* TCP SACK option */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#ifndef TM_32BIT_DSP_BIG_ENDIAN
/* Default is Little Endian for 32BIT_DSP for backward compatibility */
typedef struct tsTcpSackOpt
{
    unsigned int        tcpoSLength : 8;   /* variable */
    unsigned int        tcpoSKind   : 8;   /* 5 */
/* 2 NOOP to make the structure word aligned */
    unsigned int        tcpoSNoop2  : 16;
/* variable length option. Maximum value is TM_TCP_SACK_SQNS (8) */
    ttSqn               tcpoSSqn[TM_TCP_SACK_SQNS];
} ttTcpSackOpt;
#else /* TM_32BIT_DSP_BIG_ENDIAN */
typedef struct tsTcpSackOpt
{
/* 2 NOOP to make the structure word aligned */
    unsigned int        tcpoSNoop2  : 16;
    unsigned int        tcpoSKind   : 8;   /* 5 */
    unsigned int        tcpoSLength : 8;   /* variable */
/* variable length option. Maximum value is TM_TCP_SACK_SQNS (8) */
    ttSqn               tcpoSSqn[TM_TCP_SACK_SQNS];
} ttTcpSackOpt;
#endif /* TM_32BIT_DSP_BIG_ENDIAN */
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
typedef struct tsTcpSackOpt
{
/* 2 NOOP to make the structure word aligned */
    tt16Bit             tcpoSNoop2;
    unsigned int        tcpoSKind   : 8;   /* 5 */
    unsigned int        tcpoSLength : 8;   /* variable */
/* variable length option. Maximum value is TM_TCP_SACK_SQNS (8) */
    ttSqn               tcpoSSqn[TM_TCP_SACK_SQNS];
} ttTcpSackOpt;
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
typedef struct tsTcpSackOpt
{
/* 2 NOOP to make the structure word aligned */
    tt16Bit             tcpoSNoop2;
    tt8Bit              tcpoSKind;   /* 5 */
    tt8Bit              tcpoSLength; /* variable */
/* variable length option. Maximum value is TM_TCP_SACK_SQNS (8) */
    ttSqn               tcpoSSqn[TM_TCP_SACK_SQNS];
} ttTcpSackOpt;
#endif /* !TM_DSP */

typedef ttTcpSackOpt TM_FAR * ttTcpSackOptPtr;
#endif /* TM_TCP_SACK */
/* END of network headers */

/*
 * tfTcpSendPacket() function description:
 * 1. Prevent re-entrancy (we release the lock when we send in the IP layer,
 *    and do not want to allow other threads access to this routine while
 *    we are looping and trying to send, except that we allow sending SYN,
 *    RST, Keep alive, or TIME WAIT packets as this will not affect the TCP
 *    algorithms):
 *    If a thread is in the process of sending a packet, we will just queue
 *    (or piggy back) our request to that thread, except if we are sending a
 *    SYN, RST, Keep alive, or TIME WAIT packet.
 * 2. If we are not in retransmit mode, disable the retransmit timer, while
 *    we are looping, and make sure that the timer interval will be
 *    re-initialized at the end of this routine.
 * 3. SYN, RESET, Keep alive and TIME WAIT packets have already been
 *    pre-formatted (in tfTcpCreateSendPacket()). Skip send queue lookup, and
 *    TCP header build for these packets described in step 4 below.
 * 4. Non pre-formatted packets, determine how much data (if any) we can
 *    send, segmentize the buffer and send it with a piggybacked
 *    acknowledgment (acknowledgment value = RCV.NXT) IP send:
 *    a. Compute maximum data that we are allowed to send: Never send more
 *       than the minimum of the congestion window and the receiver's
 *       advertised window (send window). This is our effective send window.
 *       Skip over any SEL ACKed segment in the send queue.
 *    b. Compute data from the send queue that has been sent but not
 *       acknowledged up to SND.NXT: unacknowledged send queue data length.
 *    c. If we are probing the receiver's window and if receiver's window
 *       is non zero, reset the probe window timer. If we are either
 *       sending urgent data or probing the receiver's window, and the
 *       receiver's window is zero, set our effective send window to 1,
 *       to allow one byte of data to be output. This will force the peer
 *       to send us an ACK, so that we can know if the window has opened.
 *    d. Compute data to send from the send queue, that will fit in the
 *       effective send window (including data sent but not yet acknowledged):
 *       send queue data length.
 *    e. If the send queue data length is less than the unacknowledged send
 *       queue data length, the peer's window has shrunk or FIN has not
 *       been acknowledged yet. We cannot send any new data. If the
 *       peer's window has dropped to zero, we need to cancel the retransmit
 *       timer, and add the probe window timer (done later).
 *    f. Else subtract the unacknowledged data from the allowed send queue
 *       data length. If it is more than the effective send MSS, reduce
 *       to that value, but remember to try and send more data in the loop.
 *       This is the amount of data we can send, in one IP send.
 *    g. If there is a FIN to be sent and no room for the FIN in that one
 *       segment, or there is more data in the send queue to be sent later
 *       we reset the FIN flag, to prevent us from sending it in
 *       this routine.
 *    h. Compute receive window that we are allowed to advertize.
 *    i. Sender silly window syndrome avoidance:
 *       . Sender Silly Window Syndrome rule #1 (RFC 1122 4.2.3.4).
 *         Data not full size segment.
 *       . Sender Silly Window Syndrome rule #2 (RFC 1122 4.2.3.4).
 *         Data to send and ((we are expecting an ACK and the user did not
 *         disable the Nagle algorithm), or the user does not want to push
 *         the data when emptying the send queue or we are not emptying the
 *         send queue).
 *       . Sender Silly Window Syndrome rule #3 (RFC 1122 4.2.3.4).
 *         Data to send and data not at least a fraction (1/4) of the biggest
 *         peer window
 *       . Sender Silly Window Syndrome rule #4 (RFC 1122 4.2.3.4).
 *         Data to send and we are not re-transmitting (timeout), or
 *         we are not sending urgent data or probing window (timeout)
 *    j. No data to send for reasons stated in i, check if we should send a
 *       segment for other reasons: need to send an ACK, urgent flag, a FIN,
 *       window update. Apply receiver silly window syndrome avoidance.
 *       If we cannot send anything and do have some data to send, and were
 *       not retransmitting or probing the window already, add the
 *       window probe timer, to try and send later, and then exit the routine.
 *    k. Allocate a TCP header.
 *    l. Add Time stamp option if any.
 *    m. Add Sel ACK options if any.
 *    n. Fill in the TCP header seqno, Ackno, control bits.
 *       If we send a pure ACK (no FIN, no data, no out of band probe), we
 *       need to set the sequence number to the highest sequence number
 *       we have sent so far, otherwise set it to the beginning of the data
 *       we are sending. If we are emptying the send queue, or user wants to
 *       push the data on every segment, set the PUSH bit.
 *       Ackno is Rcv.Nxt. Fill in control bits (ACK, PUSH, FIN).
 *   o.  Fill in window size in TCP header.
 *       Receiver silly window syndrome RFC 1122 section 4.2.3.3:
 *       Check if we should advertize a window update. The criteria for the
 *       advertizement is half as strict as the criteria for sending
 *       a gratuitious window update (see above).
 *       Do not advertize any increase in the receive window from the last
 *       update if the increase is less than a segment or less than a fraction
 *       of the maximum receive queue size.
 *       Use window scale option if any.
 *    p. Fill in urgent fields in TCP header
 *    q. Link in the data to send with the header. If we are not using Time
 *       stamp, time the data being sent (Vegas algorithm).
 *    r. Update SND.NXT, unless we are probing the remote zero window,
 *       or unless we are retransmitting. If we do retransmit update
 *       SND.NXT only if we have included data that has not been sent yet.
 *    s. Update maximum SND.NXT including out of window data (probe).
 *    t. Initialize total length of data being sent.
 *    u. update our advertized window, if we have increased it.
 * 5. Common code for pre-formatted packets (SYN, RESET, KEEP ALIVE) and
 *    all others.
 *    a. Point to IP header
 *    b. If TCP header is not recycled, find route (if not cached), fill in
 *       IP source and destination address, IP Tos, IP Ttl, TCP source port
 *       and destination port.
 *    c. Tcp checksum computation.
 *    d. Make the packet point to IP header
 *    e. Since we are sending an ACK and possibly a probe, reset those flags
 *    f. Remember our last Ack sent (for time stamp)
 *    g. If we have sent SYN/new data/FIN, turn on retransmission timer.
 *       Need to set it here, in case we are going through the loop again.
 *       If probe window timer is on, delete it.
 *    h. Send the packet via the IP layer. Socket lock will be released and
 *       regained inside the IP send routine.
 *    i. Check if we need to send more data (either because we determined that
 *       there is more data in the send queue, or another thread piggied back
 *       on this send thread. (Will go to step 4).
 * 6. Out of the TCP send loop
 *    a. Let other users have access to tfTcpSendPacket, by setting the
 *       send clear flag.
 *    b. (re)-initialization of retransmit timer interval either because we
 *       just enabled the retransmit timer, or because we are retransmitting
 *       some data in this routine.
 *    c. Reset the REINIT_TIMER flag to allow the retransmission timer to
 *       run (if in retransmit mode).
 *    d. If an error occured (could not duplicate data in the send queue, or
 *       there was no route to the destination), we need to free the packet.
 *       The header cannot be recycled.
 *
 * Parameter      description
 * tcpVectPtr     pointer to TCP state vector/socket entry
 *
 * Return values
 * TM_ENOERROR
 *
 */
#ifdef TM_RCM3200
/*
 * Rabbit 3000 processor requires 4K alignment of functions that produce
 * more than 4K of FARCODE.
 */
#pragma asm(".defseg FARCODE")
#pragma asm(".seg FARCODE")
#pragma asm(".align 1000h")
#endif /* TM_RCM3200 */

#ifdef TM_LINT
LINT_NULL_PTR_BEGIN(sendQPacketPtr)
LINT_NULL_PTR_BEGIN(packetPtr)
LINT_UNACCESS_SYM_BEGIN(window16)
#endif /* TM_LINT */
int tfTcpSendPacket (ttTcpVectPtr tcpVectPtr)
{
    ttPacketPtr         packetPtr;
    ttPacketPtr         sendQPacketPtr;
    ttPacketPtr         timSendQPacketPtr;
    ttPacketPtr         dataPacketPtr;
    ttPacketPtr         prevDataPacketPtr;
    tt8BitPtr           optPtr;
    tt8BitPtr           srcPtr;
    tt8BitPtr           destPtr;
#ifdef TM_USE_IPV4
    ttPseudoHeaderPtr   pshPtr;
    ttIpHeaderPtr       iphPtr;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    tt6PseudoHeaderPtr  psh6Ptr;
    tt6IpHeaderPtr      iph6Ptr;
    tt8Bit              saved6IpHops;
#endif /* TM_USE_IPV6 */
    ttTcpHeaderPtr      tcphPtr;
    ttSocketEntryPtr    socketPtr;
#ifdef TM_DEV_SEND_OFFLOAD
    ttDevSendOffloadPtr pktSendOffloadPtr;
#endif /* TM_DEV_SEND_OFFLOAD */
    ttSqn               sndNxt;
    ttSqn               seqNo;
    tt32Bit             dataLength;
    tt32Bit             unAckLength;
#ifdef TM_USE_TCP_PACKET
    tt32Bit             tempUnAckLength;
#endif /* TM_USE_TCP_PACKET */
    tt32Bit             tsVal;
    tt32Bit             currentTime;
    tt32Bit             effSndWindow32;
    ttS32Bit            advRecvSpace;
    tt32Bit             rcvWndAdvIncr;
    tt32Bit             maxRcvWindow32;
    tt32Bit             tcpLength;
    tt32Bit             cwnd;
    tt32Bit             sum;
    tt32Bit             sndWnd;
#ifdef TM_OPTIMIZE_SPEED
    ttPktLen            sendQPacketBytes;
#endif /* TM_OPTIMIZE_SPEED */
#ifdef TM_TCP_RFC2581
    ttS32Bit            idleTime;
#endif /* TM_TCP_RFC2581 */
#ifdef TM_TCP_RFC3042
    int                 segmentsToSend; /* [RFC3042].R2:2.10 */
#endif /* TM_TCP_RFC3042 */
#ifdef TM_TCP_SACK
    ttSackEntryPtr      sackBlockPtr;
    ttTcpSackOptPtr     tcpSackOptPtr;
    ttSqn               tcpSackSqn;
#ifdef TM_TCP_FACK
    tt32Bit             awnd;
#endif /* TM_TCP_FACK */
    tt16Bit             sackSqnIndex;
    tt16Bit             maxNumberSackSqns;
    tt16Bit             sackOptLength;
#ifdef TM_TCP_FACK
    tt8Bit              fackAlgorithm;
#endif /* TM_TCP_FACK */
#endif /* TM_TCP_SACK */
    ttPktLen            copyDataLength;
    ttPktLen            leftOverBytes;
#ifdef TM_DSP
    int                 destOffset;
    int                 srcOffset;
#endif /* TM_DSP */
    int                 errorCode;
#ifdef TM_USE_IPV4
    tt16Bit             tcp16Length;
    tt16Bit             pshLayer4Len; /*pshLayer4Len */
#endif /* TM_USE_IPV4 */
    tt16Bit             window16;
    tt16Bit             needSend;
    tt16Bit             urgentOffset;
    tt16Bit             tcpHeaderSize;
    tt16Bit             ipHdrLength;
    tt16Bit             pseudoXsum;
#ifdef TM_DEV_SEND_OFFLOAD
    tt16Bit             effSndMss;
    tt8Bit              needXsumUpdate;
#endif /* TM_DEV_SEND_OFFLOAD */
    tt8Bit              rcvWndScale;
    tt8Bit              byteOutOfWnd;
    tt8Bit              updateSndNxt;
    tt8Bit              checkUpdateSndNxt;
    tt8Bit              needFreePacket;
#ifdef TM_SNMP_MIB
    tt8Bit              snmpRetransFlg;
#endif /* TM_SNMP_MIB */
#define lOurRcvWindow32 effSndWindow32
#define lRcvAdv         effSndWindow32
#define lTempWindow32   maxRcvWindow32
#define lMaxSndNxt      maxRcvWindow32
#define l32UrgentOffset maxRcvWindow32

    errorCode = TM_ENOERROR;
    dataPacketPtr = (ttPacketPtr)0;
#ifdef TM_USE_IPV4
/* Initialize iphPtr to zero for compiler warning. */
    iphPtr = (ttIpHeaderPtr)0;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
/* Initialize iph6Ptr to zero for compiler warning. */
    iph6Ptr = (tt6IpHeaderPtr)0;
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_TCP_PACKET
    tempUnAckLength = (tt32Bit)0;
#endif /* TM_USE_TCP_PACKET */
    sum = (tt32Bit)0;
#ifdef TM_TCP_SACK
    tcpSackSqn = (ttSqn)0;
    sackSqnIndex = TM_16BIT_ZERO;
    sackOptLength = TM_16BIT_ZERO;
#endif /* TM_TCP_SACK */
#ifdef TM_SNMP_MIB
    snmpRetransFlg = (tt8Bit)0;
#endif /* TM_SNMP_MIB */
/*
 * 1. Prevent re-entrancy: we release the lock when we send in the IP layer,
 * and do not want to allow other threads access to this routine while
 * we are looping and trying to send, except that we allow sending SYN,
 * RST, Keep alive, or TIME WAIT packets as this will not affect the TCP
 * algorithms.
 */
/* SYN, RST, Keep alive, or TIME WAIT packet */
    packetPtr = tcpVectPtr->tcpsHeaderPacketPtr;
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    if (   (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags2,
                                  TM_TCPF2_SEND_CLEAR))
        && (packetPtr == TM_PACKET_NULL_PTR) )
    {
/*
 * 1. A thread is in the process of sending a packet, we will just queue
 * (or piggy back) our request to that thread, except if we are sending a
 * SYN, RST, Keep alive, or TIME WAIT packet.
 */
        tcpVectPtr->tcpsFlags2 |= TM_TCPF2_SEND_DATA;
/* Return */
        goto tcpSendPacketExit;
    }
    needFreePacket = TM_8BIT_ZERO;
    rcvWndScale = tcpVectPtr->tcpsRcvWndScale;
    if ( tm_16bit_bits_not_set( tcpVectPtr->tcpsFlags2,
                                TM_TCPF2_REXMIT_MODE) )
    {
/*
 * 2. If we are not in retransmit mode, set TM_TCPF2_REINIT_TIMER flag to
 * disable the retransmit timer, while we are looping, and make sure that
 * the timer interval will be re-initialized at the end of this routine.
 */
        tcpVectPtr->tcpsFlags2 |= TM_TCPF2_REINIT_TIMER;
    }
/*
 * RFC 2581: Re-starting Idle connections. ([RFC2581]R4.1:1)
 * Keep track of last Xmit time, i.e add a new TCP Vector field
 * tcpsLastXmitTime that will be intialized when a SYN is sent,
 * and, that will be updated every time a packet is sent.
 *   .   If sndUna == MaxSndNxt (all sent data has been ACKed)
 *    && if difference between current time, and last transmitted time is
 *        bigger than current rtt, reduce the congestion window to the
 *        restart window:
 *  RFC2581 alone:
 *    RW = IW
 *  RFC2414:
 *    RW = min(IW, cmnd), i.e.
 *   ..  If RFC2414 initial cw was used (i.e. tcpsInitialCwnd != 0)
 *      ... if cwnd > tcpsInitialCwnd then initialize cwnd
 *          with tcpsInitialCwnd
 *   .. else (tcpsInitialCwnd == 0)
 *       RW=IW (same as RFC2581 above).
 */
#ifdef TM_TCP_RFC2581
    if (    (tcpVectPtr->tcpsSndUna == tcpVectPtr->tcpsMaxSndNxt)
         && (socketPtr->socSendQueueBytes != TM_UL(0)) )
    {
        tm_kernel_set_critical;
        currentTime = tvTime;
        tm_kernel_release_critical;
        idleTime = (ttS32Bit)(currentTime - tcpVectPtr->tcpsLastXmitTime);
        if ((tt32Bit)idleTime > tcpVectPtr->tcpsRto)
        {
#ifdef TM_TCP_RFC2414
            if (tcpVectPtr->tcpsInitialCwnd != 0)
            {
                if (tcpVectPtr->tcpsCwnd > tcpVectPtr->tcpsInitialCwnd)
                {
                    tcpVectPtr->tcpsCwnd = tcpVectPtr->tcpsInitialCwnd;
                }
            }
            else
#endif /* TM_TCP_RFC2414 */
            {
                tcpVectPtr->tcpsCwnd = tm_tcp_iw(tcpVectPtr);
            }
        }
    }
#endif /* TM_TCP_RFC2581 */
#ifdef TM_TCP_RFC3042
/*
 * [RFC3042]R2:2.20
 * Initialize number of segments to send to 0.
 */
    segmentsToSend = 0; /* [RFC3042].R2:2.20 */
/* [RFC3042].R2:2.20. Not needed here. Added to avoid a compiler warning */
    cwnd = tcpVectPtr->tcpsCwnd;
#endif /* TM_TCP_RFC3042 */
    do
    {
#ifdef TM_TCP_RFC3042
/*
 * [RFC3042]R2:2.20
 * copy the congestion window in the local variable, and use it in the
 * loop instead.
 * [RFC3042]R2:2.50
 * if number of segments to send is 0 (end of SINGLE DUP algorithm):
 * set the cwnd to the TCP vector congestion window.
 *
 * Note that cwnd has to be set in the loop from the TCP vector, because
 * other threads could queue extra sending cycles to the loop.
 */
        if (segmentsToSend == 0) /* [RFC3042]R2:2.50 */
#endif /* TM_TCP_RFC3042 */
        {
            cwnd = tcpVectPtr->tcpsCwnd; /* [RFC3042].R2:2.20 */
        }
        tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2,
                          (TM_TCPF2_SEND_DATA|TM_TCPF2_SEND_CLEAR) );
#if defined(TM_TCP_SACK) && defined(TM_DEV_SEND_OFFLOAD)
        sackOptLength = (tt16Bit)0;
#endif /* defined(TM_TCP_SACK) && defined(TM_DEV_SEND_OFFLOAD) */
        if (packetPtr == TM_PACKET_NULL_PTR)
        {
/*
 * Check if we need to send an ACK, or send a FIN, or probe a window, or
 * send urgent data
 */
            needSend = (tt16Bit)(tcpVectPtr->tcpsFlags2
                         &(TM_TCPF2_ACK|TM_TCPF2_SND_FIN|TM_TCPF2_PROBE_OOB));
            byteOutOfWnd = TM_8BIT_ZERO;
            sndWnd = tcpVectPtr->tcpsSndWnd;
            if (tcpVectPtr->tcpsSndUna != tcpVectPtr->tcpsSndWL2)
/*
 * Note: SND.WND is offset from SND.WL2, not from SND.UNA
 * SND.WND has not been updated to reflect the new SND.UNA. This could happen if the
 * peer ACKs new data within a retransmitted segment.
 * Recompute SND.WND from SND.UNA.
 */
            {
                unAckLength = tcpVectPtr->tcpsSndUna - tcpVectPtr->tcpsSndWL2;
                if (unAckLength < sndWnd)
                {
                    sndWnd = sndWnd - unAckLength;
                }
                else
/*
 * Window went to zero. unAckLength == sndWnd should be the only case we reach
 * here.
 */
                {
                    sndWnd = 0;
                }
            }
/* Update Send.Nxt variable (without checking) */
            updateSndNxt = TM_8BIT_YES;
/* Check if Send.Nxt variable should be updated */
            checkUpdateSndNxt = TM_8BIT_ZERO;
#ifdef TM_TCP_FACK
            if (    (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags,
                                          TM_TCPF_SEL_ACK))
                 && (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags2,
                                          TM_TCPF2_CONGESTED)) )
            {
/* If during recovery, use FACK */
                fackAlgorithm = TM_8BIT_YES;
            }
            else
            {
                fackAlgorithm = TM_8BIT_ZERO;
            }
#endif /* TM_TCP_FACK */
/*
 * 4.a Compute maximum data that we are allowed to send:
 * RFC 2001 (congestion avoidance):
 *  2.  The TCP output routine never sends more than the minimum of cwnd
 *      (congestion window) and the send window (receiver's advertised
 *       window):
 */
            if ( tm_16bit_one_bit_set( tcpVectPtr->tcpsFlags2,
                                       TM_TCPF2_REXMIT_TEMP) )
            {
/* 4.a retransmit one segment at suna */
                tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2,
                                  TM_TCPF2_REXMIT_TEMP );
                sndNxt = tcpVectPtr->tcpsSndUna;
/*
 * 4.b Compute data from the send queue that has been sent but not
 * acknowledged up to SND.NXT: unacknowledged send queue data length.
 */
                unAckLength = TM_UL(0);
                if (    (tcpVectPtr->tcpsState >= TM_TCPS_FIN_WAIT_1)
                     && (tcpVectPtr->tcpsState <= TM_TCPS_LAST_ACK) )
/*
 * In FIN_WAIT_1, CLOSING, LAST_ACK states, try and retransmit the FIN
 * as well if it will fit.
 */
                {
                    needSend |= TM_TCPF2_SND_FIN;
                }
/* Retransmit one segment */
                effSndWindow32 = tcpVectPtr->tcpsEffSndMss;
/* Data length that has been transmitted before */
                dataLength = tcpVectPtr->tcpsMaxSndNxt - sndNxt;
                if (effSndWindow32 > dataLength)
                {
/*
 * ANVL-CORE 17.18, 17.19:
 * Do not retransmit more that what was transmitted before (unless Nagle
 * algorithm is disabled).
 */
                    if ( tm_16bit_bits_not_set(
                                            tcpVectPtr->tcpsFlags,
                                            TM_TCP_PUSH_ALL|TM_TCPF_NODELAY) )
/* Nagle algorithm on */
                    {
/* Restrict what we want to send to what has been transmitted before */
                        effSndWindow32 = dataLength;
                    }
                }
                if (effSndWindow32 > sndWnd)
                {
/*
 * Make sure that we do not send data beyond advertized recv window.
 */
                    effSndWindow32 = sndWnd;
                }
#ifdef TM_TCP_FACK
                if (fackAlgorithm != TM_8BIT_ZERO)
                {
/*
 * If Fack algorithm applies, update sndNxt to sndUna, and loop to
 * see if we can send more data (i.e. if FACK algorithm allows it).
 */
                    tcpVectPtr->tcpsFlags2 |= TM_TCPF2_SEND_DATA;
                }
                else
#endif /* TM_TCP_FACK */
                {
                    updateSndNxt = TM_8BIT_ZERO;
                    checkUpdateSndNxt = TM_8BIT_YES;
                }
#ifdef TM_TCP_RFC3042
/* Undo limited Transmit algorithm if any. [RFC3042]R2:2.30*/
                segmentsToSend = 0;
                cwnd = tcpVectPtr->tcpsCwnd;
#endif /* TM_TCP_RFC3042 */
            }
            else
            {
                sndNxt = tcpVectPtr->tcpsSndNxt;
#ifdef TM_TCP_SACK
/* Skip over Sacked segments if any */
                sackBlockPtr =
                          tcpVectPtr->tcpsSackBlockPtr[TM_TCP_SACK_RECV_INDEX];
                while (sackBlockPtr != (ttSackEntryPtr)0)
                {
                    if (    tm_sqn_leq(sackBlockPtr->blocQLeftEdge, sndNxt)
                         && tm_sqn_gt(sackBlockPtr->blocQRightEdge, sndNxt) )
                    {
                        sndNxt = sackBlockPtr->blocQRightEdge;
                        break;
                    }
                    sackBlockPtr = sackBlockPtr->blocQNextPtr;
                }
#endif /* TM_TCP_SACK */
/*
 * 4.b Compute data from the send queue that has been sent but not
 * acknowledged up to SND.NXT: unacknowledged send queue data length.
 */
                unAckLength = sndNxt - tcpVectPtr->tcpsSndUna;
#ifdef TM_TCP_RFC3042
/*
 * Set congestion window according to the limited Transmit algorithm.
 * [RFC3042]R2:2
 *
 * [RFC3042]R2:2.40
 * if TM_TCPF2_SINGLE_DUP is set:
 *    . set cwnd to cwnd + 2 segments
 *    . reset the TM_TCPF2_SINGLE_DUP flag.
 *    . Set number of segments to send to 2.*
 */
                if (tm_16bit_one_bit_set(
                                     tcpVectPtr->tcpsFlags2,
                                     TM_TCPF2_SINGLE_DUP))
                {
                    cwnd = cwnd + (2 * tcpVectPtr->tcpsEffSndMss);
                    tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2,
                                      TM_TCPF2_SINGLE_DUP );
                    segmentsToSend = 2;
                }
#endif /* TM_TCP_RFC3042 */
/* 4.a smallest of congestion window and peer's receive window */
                if ( cwnd <= sndWnd )
                {
/*
 * congestion window is smallest.
 * effSndWindow32 includes unacked data.
 */
#ifdef TM_TCP_FACK
/* Apply FACK algorithm during recovery to see if we can send data */
                    if (fackAlgorithm != TM_8BIT_ZERO)
                    {
/*
 * Sender's estimate of the actual quantity of data outstanding in the
 * network
 */
                        awnd =   tcpVectPtr->tcpsRetranData
                               + tcpVectPtr->tcpsMaxSndNxt
                               - tcpVectPtr->tcpsSndFack;
/*
 * Congestion window with Data Smoothing:
 */
                        cwnd = tcpVectPtr->tcpsCwnd + (tt32Bit)tcpVectPtr->tcpsWinTrim;
                        if (awnd < cwnd)
                        {
/* Allow to send (cwnd - awnd) from sndNxt (sndNxt - sndUna == unAckLength) */
                            effSndWindow32 = cwnd - awnd + unAckLength;
                        }
                        else
                        {
/* Send nothing */
                            effSndWindow32 = unAckLength;
                        }
                        if (effSndWindow32 > sndWnd)
                        {
/* Cannot send more than peer advertized receive queue */
                            effSndWindow32 = sndWnd;
                        }
                    }
                    else
#endif /* TM_TCP_FACK */
                    {
                        effSndWindow32 = cwnd;
                    }
                }
                else
                {
/* 4.a peer's receive window. effSndWindow32 includes unacked data */
                    effSndWindow32 = sndWnd;
                }
            }
#ifdef TM_USE_TCP_PACKET
/*
 * Adjust the amount of data to send so that we respect the user packet
 * boundary
 */
            if (    tm_16bit_one_bit_set( tcpVectPtr->tcpsFlags,
                                          TM_TCPF_PACKET)
                 && (effSndWindow32 > unAckLength)
                 && (socketPtr->socSendQueueBytes > unAckLength) )
            {
#ifdef TM_OPTIMIZE_SPEED
                if (    (tcpVectPtr->tcpsSendQPacketPtr != (ttPacketPtr)0)
                     && (tm_sqn_geq(sndNxt, tcpVectPtr->tcpsSendQSqn)) )
                {
/* Get the send queue cached packet to send with its corresponding SQN */
                    sendQPacketPtr = tcpVectPtr->tcpsSendQPacketPtr;
                    tempUnAckLength = sndNxt - tcpVectPtr->tcpsSendQSqn;
                }
                else
#endif /* TM_OPTIMIZE_SPEED */
                {
                    sendQPacketPtr = socketPtr->socSendQueueNextPtr;
                    tempUnAckLength = unAckLength;
                }
                dataLength = effSndWindow32 - unAckLength;
                while (    (sendQPacketPtr != TM_PACKET_NULL_PTR)
                        && (tempUnAckLength
                                >= sendQPacketPtr->pktLinkDataLength) )
                {
                    tempUnAckLength -= sendQPacketPtr->pktLinkDataLength;
                    sendQPacketPtr =
                                  (ttPacketPtr)sendQPacketPtr->pktLinkNextPtr;
                }
                if (sendQPacketPtr != TM_PACKET_NULL_PTR)
                {
#ifdef TM_OPTIMIZE_SPEED
                    if (sendQPacketPtr != tcpVectPtr->tcpsSendQPacketPtr)
/* Cache sendQPacketPtr */
                    {
/* Cache the next packet to send with its corresponding SQN */
                        tcpVectPtr->tcpsSendQPacketPtr = sendQPacketPtr;
                        tcpVectPtr->tcpsSendQSqn = sndNxt - tempUnAckLength;
                    }
#endif /* TM_OPTIMIZE_SPEED */
/* What we can send */
                    dataLength = (tt32Bit)sendQPacketPtr->pktLinkDataLength;
                    if (effSndWindow32 < dataLength + unAckLength)
                    {
/* Do not send less than packet size */
                        effSndWindow32 = (tt32Bit)unAckLength;
                    }
                }
            }
#endif /* TM_USE_TCP_PACKET */
/* 4.c Check whether we need to reset the probe window timer */
            if (tm_16bit_one_bit_set(needSend, TM_TCPF2_PROBE_OOB))
            {
                if (effSndWindow32 == 0)
/*
 * 4.c If zero window and (window probe, or urgent data), we need to send
 * one byte (data or FIN whichever is first), to force the peer to send
 * us an ACK (that will allow us to verify whether the window has opened).
 * Remember that this byte cannot consume a sequence number.
 */
                {
                    effSndWindow32 = 1; /* allow one byte */
                    byteOutOfWnd = TM_8BIT_YES;
                    updateSndNxt = TM_8BIT_ZERO;
                }
                else
                {
/*
 * 4.c else out of band data with non zero window, or zero window opened in
 * which case we need to delete the window probe timer.
 */
                    if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags3,
                                             TM_TCPF3_PROBE_MODE))
                    {
                        tm_tcp_timer_suspend(tcpVectPtr->tcpsWndProbeTmPtr);
                        tm_tcp_maxrt_suspend(tcpVectPtr);
                        tcpVectPtr->tcpsFlags3 = (tt16Bit)
                                              (   tcpVectPtr->tcpsFlags3
                                                & ~(TM_TCPF3_PROBE_MODE) );
                        tcpVectPtr->tcpsReXmitCnt = 0;
                    }
                }
            }
/*
 * 4.d Compute data from the send queue, that will fit in the effective
 * effective send window (including data sent but not yet acknowledged):
 * send queue data length.
 */
            if (effSndWindow32 > socketPtr->socSendQueueBytes)
            {
                dataLength = socketPtr->socSendQueueBytes;
            }
            else
            {
                dataLength = effSndWindow32;
            }
            if (dataLength < unAckLength)
            {
/*
 * 4.e Deal with window shrinkage (or unacked FIN)
 */
                dataLength = 0;
                if (effSndWindow32 == 0)
                {
/*
 * 4.e If window dropped to zero, we need to reset SND.NXT to SND.UNA, and
 * if the window probe timer is not already on, delete retransmit timer,
 * and add zero window probe timer.
 */
                    tcpVectPtr->tcpsSndNxt = tcpVectPtr->tcpsSndUna;
                    sndNxt = tcpVectPtr->tcpsSndUna;
                    if (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags3,
                                              TM_TCPF3_PROBE_MODE))
                    {
/* 4.e turn off retransmit timer */
                        if (tcpVectPtr->tcpsFlags2 & TM_TCPF2_REXMIT_MODE)
                        {
                            tm_tcp_timer_suspend(tcpVectPtr->tcpsReXmitTmPtr);
                        }
                        tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2,
                                (TM_TCPF2_REXMIT_MODE|TM_TCPF2_REINIT_TIMER) );
                        tfTcpAddProbeTimer(tcpVectPtr);
                    }
                }
            }
            else
            {
/* 4.f subtract unacknowledged length from send queue data length */
                dataLength -= unAckLength;
/* 4.f Cannot send bigger than MSS or Offload MSS  */
                if (dataLength > (tt32Bit)(tcpVectPtr->tcpsOffloadDataSize))
                {
                    dataLength = tcpVectPtr->tcpsOffloadDataSize;
/* 4.f Remember to try and send more data */
                    tcpVectPtr->tcpsFlags2 |= TM_TCPF2_SEND_DATA;
                }
            }
/*
 * 4.g If there is a FIN to be sent and no room for the FIN in that one
 *     segment, or there is more data in the send queue to be sent later
 *     we reset the FIN flag, to prevent us from setting it in this segment.
 */
            if (    ( tm_16bit_one_bit_set(needSend, TM_TCPF2_SND_FIN) )
                 && (    ( dataLength >= effSndWindow32 )
                      || (   (dataLength + unAckLength)
                           < socketPtr->socSendQueueBytes ) ) )
            {
                tm_16bit_clr_bit( needSend, TM_TCPF2_SND_FIN );
            }
/*
 * 4.h Compute receive window that we are allowed to advertize:
 *  First compute Our total receive window
 */
/*#define lOurRcvWindow32 effSndWindow32 */
            lOurRcvWindow32 = tm_tcp_soc_recv_left(socketPtr, tcpVectPtr);
/* 4.h Next maximum receive window that we can advertize */
            if (rcvWndScale != TM_8BIT_ZERO)
            {
                maxRcvWindow32 = TM_TCP_MAX_SCALED_WND << rcvWndScale;
            }
            else
            {
                maxRcvWindow32 = TM_TCP_MAX_SCALED_WND;
            }
/* 4.h Next cannot advertize more than maximum */
/*#define lOurRcvWindow32 effSndWindow32 */
            if (lOurRcvWindow32 > maxRcvWindow32)
            {
                lOurRcvWindow32 = maxRcvWindow32;
            }
/*
 * 4.h Next what the peer knows about our receive window. Note that this
 * could be negative if peer sends data beyond our advertized receive window
 * and we accept that data.
 */
            advRecvSpace = (ttS32Bit)(   tcpVectPtr->tcpsRcvAdv
                                       - tcpVectPtr->tcpsRcvNxt );
/* 4.h Next prevent shrinkage of our receive window */
/*#define lOurRcvWindow32 effSndWindow32 */
            if ((ttS32Bit)lOurRcvWindow32 < advRecvSpace)
            {
                lOurRcvWindow32 = (tt32Bit)advRecvSpace;
            }
            rcvWndAdvIncr = lOurRcvWindow32 - (tt32Bit)advRecvSpace;
/*
 * 4.i Sender silly window syndrome avoidance:
 */
/*
 * 4.i Sender Silly Window Syndrome rule #1 (RFC 1122 4.2.3.4).
 * Data not full size segment.
 */
            if (dataLength < (tt32Bit)(tcpVectPtr->tcpsEffSndMss))
            {
                if (    (dataLength == 0)
/*
 * 4.i Sender Silly Window Syndrome rule #2 (RFC 1122 4.2.3.4).
 * Data to send and ((we are expecting an ACK and the user did not
 * disable the Nagle algorithm), or the user does not want to push
 * the data when emptying the send queue or we are not emptying the send
 * queue).
 */
                      || (   (    (     (tcpVectPtr->tcpsSndUna !=
                                                  tcpVectPtr->tcpsMaxSndNxt)
                                    && (   (tm_16bit_bits_not_set(
                                              tcpVectPtr->tcpsFlags,
                                              TM_TCP_PUSH_ALL|TM_TCPF_NODELAY))
#ifdef TM_USE_SSL
                                         && (tm_16bit_bits_not_set(
                                                 tcpVectPtr->tcpsSslFlags,
                                                 TM_SSLF_TCP_NO_DELAY))
#endif /* TM_USE_SSL */
                                       )
                                  )
                               || (tm_16bit_one_bit_set(
                                       tcpVectPtr->tcpsFlags,
                                       TM_TCPF_NOPUSH))
                               || (    (   (dataLength + unAckLength)
                                         < socketPtr->socSendQueueBytes )
#ifdef TM_USE_TCP_PACKET
                                    && ( tm_16bit_bits_not_set(
                                                 tcpVectPtr->tcpsFlags,
                                                 TM_TCPF_PACKET ) )
#endif /* TM_USE_TCP_PACKET */
                                   ) )
/*
 * 4.i Sender Silly Window Syndrome rule #3 (RFC 1122 4.2.3.4).
 * Data to send and data not at least a fraction (1/4) of the biggest
 * peer window
 */
                          && (dataLength <
                                 tcpVectPtr->tcpsMaxSndWnd / TM_TCP_SWS_FS)
/*
 * 4.i Sender Silly Window Syndrome rule #4 (RFC 1122 4.2.3.4).
 * Data to send and we are not re-transmitting (timeout)
 */
                          && (sndNxt == tcpVectPtr->tcpsMaxSndNxt)
/*
 * 4.i Sender Silly Window Syndrome rule #4 (RFC 1122 4.2.3.4).
 * Data to send and we are not sending urgent data or probing window (timeout)
 */
                          && (tm_16bit_bits_not_set( needSend,
                                                     TM_TCPF2_PROBE_OOB)) ) )
                {
/*
 * 4.j No data to send (or allowed to send). Check if we should send an ACK,
 *     a FIN, urgent flag.
 */
                    if (    (tm_16bit_bits_not_set(
                                            needSend,
/* 4.j no delay ACK, no FIN */
                                            TM_TCPF2_ACK|TM_TCPF2_SND_FIN))
/* 4.j no urgent data */
                         && (tm_sqn_lt( tcpVectPtr->tcpsSndUp,
                                        tcpVectPtr->tcpsSndUna)) )
                    {
/*
 * 4.j Receiver silly window syndrome RFC 1122 section 4.2.3.3:
 * Check if we should send a window update.
 * Send a window update for every other segment (as opposed to for every
 * segment described in RFC 1122), or when the increase in advertized
 * receive window reaches a fraction of the total receive queue as described
 * in RFC 1122.
 */
                        if (     ( rcvWndAdvIncr <
                                      (tt32Bit)(2*tcpVectPtr->tcpsEffSndMss) )
                              && ( (TM_TCP_SWS_FR * rcvWndAdvIncr) <
                                      socketPtr->socMaxRecvQueueBytes ) )
                        {
/*
 * 4.j Cannot send anything for the time being, and are not in retransmit mode
 * or probing the peer
 */
                            if (    (socketPtr->socSendQueueBytes
                                                             != TM_32BIT_ZERO)
                                 && (tm_16bit_bits_not_set(
                                                    tcpVectPtr->tcpsFlags2,
                                                      TM_TCPF2_CONGESTED
                                                    | TM_TCPF2_REXMIT_MODE) )
                                 && (tm_16bit_bits_not_set(
                                                     tcpVectPtr->tcpsFlags3,
                                                     TM_TCPF3_PROBE_MODE)) )
                            {
/*
 * 4.j Add window probe timer for either probing the window or recheck if
 * we can send data or window update later because of silly window syndrome,
 * or because the user wants NOPUSH.
 */
                                tfTcpAddProbeTimer(tcpVectPtr);
                            }
/* 4.j out of the loop */
                            break;
                        }
                    }
                }
            }
/* 4.k Allocate our TCP header */
            packetPtr = tfTcpGetTcpHeader(tcpVectPtr);
/*
 * Default TCP header size (with TS option if TS option has been negotiated)
 */
            tcpHeaderSize = tcpVectPtr->tcpsHeaderSize;
            if (packetPtr == TM_PACKET_NULL_PTR)
            {
                errorCode = TM_ENOBUFS;
/*
 * If we could not allocate a buffer to send a FIN, we are in danger
 * of having the socket stuck in FIN_WAIT_1, or LAST_ACK state forever. Set
 * the soft error so that we can check for that condition after
 * tfTcpSendPacket() returns.
 */
                if (    (tcpVectPtr->tcpsState == TM_TCPS_FIN_WAIT_1)
                     || (tcpVectPtr->tcpsState == TM_TCPS_LAST_ACK) )
/* If still in FIN_WAIT_1 or LAST_ACK state */
                {
                    tcpVectPtr->tcpsSoftError = TM_ENOBUFS;
                }
                if (tcpVectPtr->tcpsFlags2 == TM_TCPF2_REINIT_TIMER)
                {
                    tfSocketErrorRecord((int)socketPtr->socIndex, errorCode);
                    tfSocketNotify(socketPtr,
                                   TM_SELECT_READ|TM_SELECT_WRITE,
                                   TM_CB_SOCKET_ERROR,
                                   errorCode);
                }
/* 4.k out of the loop */
                break;
            }
            tcphPtr = (ttTcpHeaderPtr)packetPtr->pktLinkDataPtr;
/* 4.l Add time stamp options */
            if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_TS))
            {
/*
 * OL 5/24/2001
 * The optPtr = (tt8BitPtr)tcphPtr + TM_PAK_TCP_HDR_LEN assignment
 * has been cut into two different statements to circumvent an SDS compiler
 * loop optimization bug. With the loop optimization turned on, the
 * SDS compiler was moving this assignment outside of the loop, where
 * tcphPtr had not even been initialized!
 */
/* Point to the TCP header */
                optPtr = (tt8BitPtr)tcphPtr;
/* Point to options in the TCP header */
                optPtr = optPtr + TM_PAK_TCP_HDR_LEN;
/* 4.l Adding TS (inlined because in send path) */
                ((ttTcpAlgnTsOptPtr)optPtr)->tcpoAFirstWord =
                                                        TM_TCPO_TS_FIRST_WORD;
                tm_kernel_set_critical;
                tsVal = (tt32Bit)tvTime;
                tm_kernel_release_critical;
#ifdef TM_USE_SOCKET_IMPORT
                tsVal += tcpVectPtr->tcpsTsCurrentFixup;
#endif /* TM_USE_SOCKET_IMPORT */
                tm_htonl(tsVal, ((ttTcpAlgnTsOptPtr)optPtr)->tcpoAVal);
                tsVal = tcpVectPtr->tcpsTsRecent;
                tm_htonl(tsVal, ((ttTcpAlgnTsOptPtr)optPtr)->tcpoAEchoRpl);
/* 4.l End of adding TS */
            }
/*
 * 4.m If SEL ACK is permitted, and there is any data in the reassemble queue,
 * build SEL ACK option, adjust datalength accordingly, and reset FIN if
 * datalength is reduced
 */
#ifdef TM_TCP_SACK
            if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_SEL_ACK))
            {
                if (    (    tcpVectPtr->tcpsSackBlockPtr
                                        [TM_TCP_SACK_SEND_INDEX]
                          != (ttSackEntryPtr)0 )
                     && ( tcpVectPtr->tcpsState >= TM_TCPS_SYN_RECEIVED ) )
                {
                    maxNumberSackSqns = (tt16Bit)
                                            tcpVectPtr->tcpsMaxSackSqns;
#ifdef TM_ERROR_CHECKING
                    if (    tcpVectPtr->tcpsReassemblePtr
                         == TM_PACKET_NULL_PTR )
                    {
                        tfKernelWarning(
                   "tfTcpSendPacket",
                   "Discrepancy between SEL ACK list, and reassemble list\n");
                    }
                    else
                    {
                        if (maxNumberSackSqns == (tt16Bit)0)
                        {
                            tfKernelWarning( "tfTcpSendPacket",
                                             "SEL ACK blocks not initialized!\n");
                        }
                        else
                        {
#endif /* TM_ERROR_CHECKING */
/*
 * Build SEL ACK option: scan the SEL ACK block queue, compute Sack option
 * Length header size.
 */
                            tcpSackOptPtr = (ttTcpSackOptPtr)
                                ((ttCharPtr)tcphPtr +
                                 tm_packed_byte_len(tcpHeaderSize));
                            tcpSackOptPtr->tcpoSNoop2 = TM_TCPO_NOOP2;
                            tcpSackOptPtr->tcpoSKind = TM_TCPO_SACK;
/*
 * Loop through the SEL ACK block queue and copy the SEL ACK blocks
 * in SEL ACK queue order: segment which triggered this ACK first (unless
 * that segment advanced the Ack Number field in the header), followed
 * by the most recently reported SACK blocks (based on first SACK blocks
 * in previous SACK options) that are not subsets of a SACK block
 * already included in the SACK option being constructed [RFC 2018].
 * All these RFC 2018 conditions are met by the way we insert our SEL ACK
 * blocks in our SEL ACK blocks queue.
 * Fill out as many SEL ACK blocks that can fit in the SEL ACK option
 * [RFC 2018].
 */
                            sackBlockPtr = tcpVectPtr->tcpsSackBlockPtr
                                                    [TM_TCP_SACK_SEND_INDEX];
/* Fill in the SEL ACK blocks */
                            for ( sackSqnIndex = 0;
                                     (sackSqnIndex < maxNumberSackSqns)
                                  && (sackBlockPtr != (ttSackEntryPtr)0);
                                  sackSqnIndex++)
                            {
/*
 * Variable length option: First and last sequence number of the
 * SEL ACK blocks (Left Edge, and Right Edge).
 */
/* Left Edge */
                                tcpSackSqn = sackBlockPtr->blocQLeftEdge;
                                tm_htonl( tcpSackSqn,
                                          tcpSackOptPtr->tcpoSSqn[sackSqnIndex]);
                                sackSqnIndex++;
/* Right Edge */
                                tcpSackSqn =  sackBlockPtr->blocQRightEdge;
                                tm_htonl( tcpSackSqn,
                                          tcpSackOptPtr->tcpoSSqn[sackSqnIndex]);
                                sackBlockPtr = sackBlockPtr->blocQNextPtr;
                            }
/* TM_TCPO_SACK_FW_LENGTH is first word length: (kind, length)*/
                            sackOptLength = (tt16Bit)
                                      (  TM_TCPO_SACK_FW_LENGTH
                                       + (sackSqnIndex * TM_TCPO_SACK_ONE_SQN));
/* Size of the option without padding */
                            tcpSackOptPtr->tcpoSLength = (tt8Bit)sackOptLength;
/* Size of the option with padding */
                            sackOptLength = (tt16Bit)
                                            (   sackOptLength
                                              + TM_TCPO_SACK_FW_PADDED );
                            tcpHeaderSize = (tt16Bit)(   tcpHeaderSize
                                                       + sackOptLength );
                            if (    ( dataLength != 0 )
                                 && (    dataLength
                                      >= (tt32Bit)(
                                              tcpVectPtr->tcpsOffloadDataSize
                                            - sackOptLength ) ) )
/* If adding the option, make us reduce the TCP send data length */
                            {
                                dataLength -= sackOptLength;
/*
 * If we cannot send all the data, adjust FIN bit (FIN has to be last in
 * the stream).
 */
                                tm_16bit_clr_bit( needSend, TM_TCPF2_SND_FIN );
                            }
/* Adjust TCP data offset to take into account the SACK option */
                            tcphPtr->tcpDataOffset =
                                        tm_tcp_set_data_offset(tcpHeaderSize);
/* Adjust TCP packet length to take into account the SACK option */
                            packetPtr->pktLinkDataLength = tcpHeaderSize;
                            packetPtr->pktChainDataLength = tcpHeaderSize;
#ifdef TM_ERROR_CHECKING
                        }
                    }
#endif /* TM_ERROR_CHECKING */
                }
            }
#endif /* TM_TCP_SACK */
/* 4.n build TCP header ACKNO, SEQNO, and control bits */
            if ( tm_16bit_one_bit_set(needSend, TM_TCPF2_SND_FIN))
            {
/* 4.n We are sending/retransmitting a FIN */
                tcphPtr->tcpControlBits = TM_TCP_ACK|TM_TCP_FIN|TM_TCP_PSH;
            }
            else
            {
                if (    (dataLength != TM_UL(0))
                     || tm_16bit_one_bit_set(needSend, TM_TCPF2_PROBE_OOB) )
                {
/*
 * 4.n Set PUSH bit if
 * 1) we are emptying the send queue
 * 2) or user wants PSH on every segment
 */
                    if (    ((dataLength + unAckLength) ==
                                       socketPtr->socSendQueueBytes)
                         || (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags,
                                                  TM_TCP_PUSH_ALL)) )
                    {
                        tcphPtr->tcpControlBits = TM_TCP_ACK|TM_TCP_PSH;
                    }
                    else
                    {
                        tcphPtr->tcpControlBits = TM_TCP_ACK;
                    }
                }
                else /* Pure ACK */
/*
 * 4.n Otherwise we are sending a pure ACK and we need to use sndNxt
 * because of the RFC793 acceptability test performed by the peer
 */
                {
                    tcphPtr->tcpControlBits = TM_TCP_ACK;
/* Use tcpsSndNxt for sequence number */
                    sndNxt = tcpVectPtr->tcpsSndNxt;
/* Do not update tcpsSndNxt to tcpsMaxSndNxt since we are sending a pure ACK */
                    updateSndNxt = TM_8BIT_ZERO;
                }
            }
            tm_htonl(sndNxt, tcphPtr->tcpSeqNo);
            seqNo = tcpVectPtr->tcpsRcvNxt;
            tm_htonl(seqNo, tcphPtr->tcpAckNo);
/*
 * 4.o TCP header window size:
 * Receiver silly window syndrome RFC 1122 section 4.2.3.3:
 * Check if we should advertize a window update. The criteria for the
 * advertizement is half as strict as the criteria for sending
 * a gratuitious window update (see above).
 * Do not advertize any increase in the receive window from the last
 * update if the increase is less than a segment or less than a fraction
 * of the maximum receive queue size.
 */
            if (   (rcvWndAdvIncr != 0)
                && (rcvWndAdvIncr < (tt32Bit)(tcpVectPtr->tcpsEffSndMss))
                && ( (2 * TM_TCP_SWS_FR * rcvWndAdvIncr) <
                                      socketPtr->socMaxRecvQueueBytes ) )
            {
                rcvWndAdvIncr = 0;
/*#define lOurRcvWindow32 effSndWindow32 */
/* No window update, but do not advertize less than previously advertized */
                if (advRecvSpace < 0)
                {
                    lOurRcvWindow32 = 0;
                }
                else
                {
                    lOurRcvWindow32 = (tt32Bit)advRecvSpace;
                }
            }
/* 4.o Window scale option. */
            if ( rcvWndScale != TM_8BIT_ZERO )
            {
/*#define lOurRcvWindow32 effSndWindow32 */
/*#define lTempWindow32   maxRcvWindow32*/
                lTempWindow32 = lOurRcvWindow32 >> rcvWndScale;
                window16 = (tt16Bit)(lTempWindow32);
            }
            else
            {
/*#define lOurRcvWindow32 effSndWindow32 */
                window16 = (tt16Bit)lOurRcvWindow32;
            }
            tm_htons(window16, tcphPtr->tcpWindowSize);
/* 4.p Check for urgent data to send */
            if (tm_sqn_geq(tcpVectPtr->tcpsSndUp, sndNxt))
            {
/*#define l32UrgentOffset maxRcvWindow32*/
                l32UrgentOffset = tcpVectPtr->tcpsSndUp - sndNxt;
                urgentOffset = (tt16Bit)l32UrgentOffset;
/*
 * If peer is a Berkeley system, make urgentPtr point to one byte past
 * last urgent data byte.
 */
                if ( tm_16bit_bits_not_set(
                                    tcpVectPtr->tcpsFlags,
                                    TM_TCPF_STDURG) )
                {
                    urgentOffset++;
                }
                tm_htons(urgentOffset, tcphPtr->tcpUrgentPtr);
                tcphPtr->tcpControlBits |= TM_TCP_URG;
            }
            else
            {
                tcphPtr->tcpUrgentPtr = TM_16BIT_ZERO; /* no urgent data */
/* Prevent wraparound of SND.UP, but allow retransmission of SndUp */
                if (tm_sqn_lt( tcpVectPtr->tcpsSndUp,
                               tcpVectPtr->tcpsSndUna - 1 ))
                {
                    tcpVectPtr->tcpsSndUp = tcpVectPtr->tcpsSndUna - 1;
                }
            }
/* 4.q. Link in the data to send with the header. */
            tcpLength = dataLength;
            if (tcpLength != TM_UL(0))
            {
#ifdef TM_OPTIMIZE_SPEED
                sendQPacketBytes = (ttPktLen)0;
#endif /* TM_OPTIMIZE_SPEED */
/* 4.q Find packetPtr at unAckLength offset */
#ifdef TM_OPTIMIZE_SPEED
                if (    (tcpVectPtr->tcpsSendQPacketPtr != (ttPacketPtr)0)
                     && (tm_sqn_geq(sndNxt, tcpVectPtr->tcpsSendQSqn)) )
                {
/* Get the send queue cached packet to send with its corresponding SQN */
                    sendQPacketPtr = tcpVectPtr->tcpsSendQPacketPtr;
                    unAckLength = sndNxt - tcpVectPtr->tcpsSendQSqn;
                }
                else
#endif /* TM_OPTIMIZE_SPEED */
                {
                    sendQPacketPtr = socketPtr->socSendQueueNextPtr;
                }
                while (   (sendQPacketPtr != TM_PACKET_NULL_PTR)
                       && (unAckLength >= sendQPacketPtr->pktLinkDataLength) )
                {
                    unAckLength -= sendQPacketPtr->pktLinkDataLength;
                    sendQPacketPtr =
                                (ttPacketPtr)sendQPacketPtr->pktLinkNextPtr;
                }
#ifdef TM_USE_TCP_PACKET
                if ( tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags,
                                          TM_TCPF_PACKET) )
                {
/*
 * BUG ID 11-686: NEW FEATURE
 * If user wants to preserve packet boundaries, make sure that we do not
 * coalesce different user packets. (Note that if TM_TCP_PACKET is set,
 * TM_TCP_PUSH_ALL is also set so that the push bit is set on every packet
 * that we send to force the remote side to push the data to the user
 * application. TM_TCP_PUSH_ALL implies TCP_NODELAY.)
 */
#ifdef TM_ERROR_CHECKING
                    if (sendQPacketPtr == TM_PACKET_NULL_PTR)
                    {
                        tfKernelError("tfTcpSendPacket",
                                      "Send queue corrupted");
                        tm_thread_stop;
                    }
#endif /* TM_ERROR_CHECKING */
                    if (   tcpLength
                         > (tt32Bit)(sendQPacketPtr->pktLinkDataLength) )
                    {
/* Restrict to the size of the link */
                        tcpLength = (tt32Bit)sendQPacketPtr->pktLinkDataLength;
/* Do try and send more data if there is room */
                        tcpVectPtr->tcpsFlags2 |= TM_TCPF2_SEND_DATA;
                        dataLength = tcpLength;
/*
 * Since we are not sending all the data, make sure that we remove the FIN,
 * if any.
 */
                        tm_8bit_clr_bit( tcphPtr->tcpControlBits, TM_TCP_FIN );
                    }
                    else
                    {
                        if (byteOutOfWnd != TM_8BIT_ZERO)
/* zero window probe */
                        {
/* Send a keep alive probe instead */
                            seqNo = tcpVectPtr->tcpsSndUna - 1;
                            tm_htonl(seqNo, tcphPtr->tcpSeqNo);
                            byteOutOfWnd = TM_8BIT_ZERO;
                            tcpLength = TM_UL(0);
                            goto tcpSendNoData;
                        }
                    }
                }
                else
#endif /* TM_USE_TCP_PACKET */
                {
#ifdef TM_OPTIMIZE_SPEED
/* Cache sendQPacketPtr */
                    if (sendQPacketPtr != tcpVectPtr->tcpsSendQPacketPtr)
                    {
/* Cache the next packet to send with its corresponding SQN */
                        tcpVectPtr->tcpsSendQPacketPtr = sendQPacketPtr;
                        tcpVectPtr->tcpsSendQSqn = sndNxt - unAckLength;
                    }
#endif /* TM_OPTIMIZE_SPEED */
                }
                prevDataPacketPtr = packetPtr;
                tm_kernel_set_critical;
                currentTime = tvTime;
                tm_kernel_release_critical;
/*
 * Check if we have enough room to copy the packet at the end of the
 * TCP header
 */
                leftOverBytes =   ((tt16Bit)(tm_byte_count(
                          packetPtr->pktSharedDataPtr->dataBufLastPtr
                        - packetPtr->pktLinkDataPtr))) - tcpHeaderSize;
                if (dataLength <= leftOverBytes)
                {
                    packetPtr->pktLinkDataLength += (ttPktLen)dataLength;
                    packetPtr->pktChainDataLength += (ttPktLen)dataLength;
                    destPtr =   packetPtr->pktLinkDataPtr
                              + tm_packed_byte_len(tcpHeaderSize);
                }
                else
                {
                    leftOverBytes = TM_16BIT_ZERO;
                    destPtr = (tt8BitPtr)0; /* Most common case */
                }
#ifdef TM_DSP
                destOffset = 0;
#endif /* TM_DSP */

                while (dataLength != TM_UL(0))
                {
#ifdef TM_ERROR_CHECKING
                    if (sendQPacketPtr == TM_PACKET_NULL_PTR)
                    {
                        tfKernelError("tfTcpSendPacket",
                                      "Send queue corrupted");
                        tm_thread_stop;
                    }
#endif /* TM_ERROR_CHECKING */
                    timSendQPacketPtr = sendQPacketPtr;
#ifdef TM_USE_QOS
                    if (dataPacketPtr != (ttPacketPtr)0)
/* Already chained/copied data. Check that the next data has same IP TOS */
                    {
                        if (sendQPacketPtr->pktIpTos != packetPtr->pktIpTos)
                        {
/* We are not sending dataLength bytes in this packet. Correct the length */
                            if (destPtr != (tt8BitPtr)0)
                            {
                                packetPtr->pktLinkDataLength -= (ttPktLen)dataLength;
                                packetPtr->pktChainDataLength -= (ttPktLen)dataLength;
                            }
                            tcpLength = (tt32Bit)(tcpLength - dataLength);
                            tm_8bit_clr_bit( tcphPtr->tcpControlBits,
                                              TM_TCP_FIN);
/* Do not append data with a different IP TOS */
                            dataPacketPtr = (ttPacketPtr)0;
/* Do try and send more data */
                            tcpVectPtr->tcpsFlags2 |= TM_TCPF2_SEND_DATA;
                            break; /* out of while(dataLength) loop */
                        }
                    }
                    else
/* First data in the packet. Save IP TOS in TCP header packet */
                    {
                        packetPtr->pktIpTos = sendQPacketPtr->pktIpTos;
                    }
#endif /* TM_USE_QOS */
                    if (destPtr == (tt8BitPtr)0)
                    {
/* 4.q Share the buffer. We duplicate the packet and do not copy it */
                        dataPacketPtr = tfDuplicateSharedBuffer(
                                                              sendQPacketPtr);
                        if (dataPacketPtr == TM_PACKET_NULL_PTR)
                        {
                            errorCode = TM_ENOBUFS;
/*
 * If we could not allocate a buffer to send a FIN, we are in danger
 * of having the socket stuck in FIN_WAIT_1, or LAST_ACK state forever. Set
 * the soft error so that we can check for that condition after
 * tfTcpSendPacket() returns.
 */
                            if (    (tcpVectPtr->tcpsState == TM_TCPS_FIN_WAIT_1)
                                 || (tcpVectPtr->tcpsState == TM_TCPS_LAST_ACK) )
/* If still in FIN_WAIT_1 or LAST_ACK state */
                            {
                                tcpVectPtr->tcpsSoftError = TM_ENOBUFS;
                            }
                            needFreePacket = TM_8BIT_YES;
                            goto tcpOutSendLoop; /* break out of 2 loops */
                        }
/*
 * Do not append to this buffer, since TCP has already started sending data
 * from it.
 */
                        sendQPacketPtr->pktUserFlags
                                                |= TM_PKTF_USER_DATA_SENT;
                        if (packetPtr->pktLinkExtraCount == 0)
                        {
                            tm_16bit_clr_bit( dataPacketPtr->pktFlags2,
                                              TM_PF2_NO_UNACK_DATA);
                            if (unAckLength != 0)
                            {
#ifdef TM_DSP
/* update pointer to the first word of data that has unsent bytes */
                                srcOffset =   unAckLength
                                        + dataPacketPtr->pktLinkDataByteOffset;
                                dataPacketPtr->pktLinkDataPtr +=
                                    tm_packed_byte_len((unsigned)srcOffset);
/* byte offset in word of start of packet data we are sending is (0-based
   index of first byte of un-sent data past the sent but un-acked data plus
   the byte offset of the original data) mod TM_DSP_BYTES_PER_WORD */
                                dataPacketPtr->pktLinkDataByteOffset =
                                    ((int)srcOffset) % TM_DSP_BYTES_PER_WORD;
#else /* ! TM_DSP */
                                dataPacketPtr->pktLinkDataPtr +=
                                                         (unsigned)unAckLength;
#endif /* ! TM_DSP */
                                dataPacketPtr->pktLinkDataLength -=
                                    (ttPktLen)unAckLength;
                                unAckLength = 0;
                            }
                            else
                            {
                                if (  (tm_16bit_bits_not_set(
                                            sendQPacketPtr->pktUserFlags,
                                            TM_PKTF_USER_DUPLICATED))
                                     || (sendQPacketPtr
                                           == socketPtr->socSendQueueNextPtr) )
/*
 * User zero copy buffer is not duplicated in the send queue, or we are the
 * first packet in the send queue
 */
                                {

                                    dataPacketPtr->pktFlags2 |=
                                                          TM_PF2_NO_UNACK_DATA;
                                }
                            }
                        }
/* 4.q Chain in new data */
                        prevDataPacketPtr->pktLinkNextPtr =
                                                      (ttVoidPtr)dataPacketPtr;
                        packetPtr->pktLinkExtraCount++;
                        tm_16bit_clr_bit( packetPtr->pktFlags2,
                                          TM_PF2_TRAILING_DATA );
                        if (dataLength > dataPacketPtr->pktLinkDataLength)
                        {
                            dataLength -= dataPacketPtr->pktLinkDataLength;
#ifdef TM_OPTIMIZE_SPEED
/* Keep track of send queue packets that we consume */
                            sendQPacketBytes +=
                                        sendQPacketPtr->pktLinkDataLength;
#endif /* TM_OPTIMIZE_SPEED */
                            sendQPacketPtr =
                                  (ttPacketPtr)sendQPacketPtr->pktLinkNextPtr;
                            prevDataPacketPtr = dataPacketPtr;
                        }
                        else
                        {
                            if (dataLength < dataPacketPtr->pktLinkDataLength)
                            {
                                packetPtr->pktFlags2 |= TM_PF2_TRAILING_DATA;
                            }
                            dataPacketPtr->pktLinkDataLength =
                                                         (ttPktLen)dataLength;
                            dataLength = TM_UL(0);
                        }
                    }
                    else
                    {
                        dataPacketPtr = sendQPacketPtr;
/*
 * Do not append to this buffer, since TCP has already started sending data
 * from it.
 */
                        sendQPacketPtr->pktUserFlags
                                                |= TM_PKTF_USER_DATA_SENT;
/* Unacknowledged data contained in this data packet */
                        copyDataLength =   dataPacketPtr->pktLinkDataLength
                                         - (ttPktLen)unAckLength;
#ifdef TM_DSP
/* DSP: Use tm_packed_byte_len: tm_packed_byte_count advances srcPtr one word
   too far if unAckLenght isn't even */
/* Offset of data in source data packet */
                        srcOffset =   unAckLength
                                    + dataPacketPtr->pktLinkDataByteOffset;
/* Pointer to word boundary where the source data starts */
                        srcPtr =   dataPacketPtr->pktLinkDataPtr
                                 + tm_packed_byte_len((unsigned)srcOffset);
/* Offset within the word of where the data starts */
                        srcOffset = srcOffset % TM_DSP_BYTES_PER_WORD;
#else /* !TM_DSP */
                        srcPtr =   dataPacketPtr->pktLinkDataPtr
                                 + ((unsigned)unAckLength);
#endif /* !TM_DSP */
                        if (dataLength > copyDataLength)
/* This data packet contains part of the data that we want to send. */
                        {
                            dataLength -= copyDataLength;
#ifdef TM_OPTIMIZE_SPEED
/* Keep track of send queue packets that we consume */
                            sendQPacketBytes +=
                                        sendQPacketPtr->pktLinkDataLength;
#endif /* TM_OPTIMIZE_SPEED */
                            sendQPacketPtr =
                                    (ttPacketPtr)dataPacketPtr->pktLinkNextPtr;
                            unAckLength = 0;
                        }
                        else
/* Data that we want to send is all included in this packet. */
                        {
                            copyDataLength = (ttPktLen)dataLength;
                            dataLength = TM_UL(0);
                        }
#ifdef TM_DSP
                        tfByteByByteCopy((int *) srcPtr,
                                         srcOffset,
                                         (int *) destPtr,
                                         destOffset,
                                         copyDataLength);
/* DSP: Use tm_packed_byte_len: tm_packed_byte_count advances destPtr one word
   too far if unAckLenght isn't even */
                        destOffset = destOffset + copyDataLength;
/* Pointer to word boundary where the end of data starts */
                        destPtr = destPtr + tm_packed_byte_len(destOffset);
/* Offset within the word of where the end of data starts */
                        destOffset = (destOffset) % TM_DSP_BYTES_PER_WORD;
#else /* !TM_DSP */
                        tm_bcopy(srcPtr, destPtr, copyDataLength);
                        destPtr = destPtr + copyDataLength;
#endif /* TM_DSP */
                    }
/* 4.q If time stamp is not on */
                    if (    (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags,
                                                   TM_TCPF_TS))
#ifdef TM_TCP_SEND_SYN_DATA
                         || (tcpVectPtr->tcpsState < TM_TCPS_SYN_RECEIVED )
#endif /* TM_TCP_SEND_SYN_DATA */
                       )
                    {
/*
 * 4.q Time individual packet if not retransmitted (Vegas algorithm)
 *     or if packet was dropped because of PATH MTU discovery.
 */
                        if (    tm_16bit_bits_not_set(
                                                 timSendQPacketPtr->pktFlags2,
                                                 TM_PF2_TCP_TIMED )
                             || tm_16bit_one_bit_set( tcpVectPtr->tcpsFlags2,
                                                      TM_TCPF2_DF_ERROR ) )
                        {
                            timSendQPacketPtr->pktTcpXmitTime = currentTime;
                            timSendQPacketPtr->pktFlags2 |= TM_PF2_TCP_TIMED;
                        }
                        else
                        {
                            if (sndNxt != tcpVectPtr->tcpsMaxSndNxt)
                            {
/*
 * Indicate that the packet is retransmitted (to ignore rtt computation (Karn
 * algorithm))
 */
                                if ( tm_16bit_one_bit_set(
                                                timSendQPacketPtr->pktFlags2,
                                                TM_PF2_TCP_TIMED ) )
                                {
                                    timSendQPacketPtr->pktFlags2
                                                     |= TM_PF2_TCP_REXMIT;
                                }
                            }
                        }
                    }
                }
/*
 * NOTE duplicateSharedBuffer zeroes pktLinkNextPtr, so no need to zero
 * the last link.
 */
#ifdef TM_SNMP_MIB
/* tcpLength check for TM_TCP_PACKET (zero window probe) */
                if (    (sndNxt != tcpVectPtr->tcpsMaxSndNxt)
                     && (tcpLength != TM_UL(0)))
                {
/*
 * Number of segments retransmitted - that is, the number of TCP segments
 * transmitted containing one or more previously transmitted octets.
 * Here we send data below the maximum we have ever sent, so we are
 * re-transmitting.
 */
                    snmpRetransFlg = (tt8Bit)1;
                }
#endif /* TM_SNMP_MIB */
/* 4.r Update local Snd.Nxt (adding data length ) */
                sndNxt += tcpLength; /* data length */
#ifdef TM_OPTIMIZE_SPEED
/* Keep track of send queue packet bytes that we skip over */
                if (sendQPacketBytes != (ttPktLen)0)
                {
/*
 * sendQPacketPtr has moved. Cache new sendQPacketPtr with its corresponding
 * SQN
 */
                    tcpVectPtr->tcpsSendQPacketPtr = sendQPacketPtr;
                    tcpVectPtr->tcpsSendQSqn += sendQPacketBytes;
                }

#endif /* TM_OPTIMIZE_SPEED */
#ifdef TM_TCP_RFC2581
/*
 * RFC 2581: Re-starting Idle connections. ([RFC2581]R4.1:1)
 * Keep track of last data sent transmission time.
 */
                tcpVectPtr->tcpsLastXmitTime = currentTime;
#endif /* TM_TCP_RFC2581 */
#ifdef TM_USE_IPSEC
/* Tell IPSEC to not encrypt into the data buffers */
                if (leftOverBytes == TM_16BIT_ZERO)
                {
                    packetPtr->pktUserFlags |= TM_PKTF_USER_DATA_SENT;
                }
#endif /* TM_USE_IPSEC */
            }
            if ( tm_8bit_one_bit_set(tcphPtr->tcpControlBits, TM_TCP_FIN) )
/* 4.r Update local Snd.Nxt (adding FIN) */
            {
                sndNxt++; /* FIN */
                tcpVectPtr->tcpsFlags2 |= TM_TCPF2_FIN_SENT;
/*
 * ANVL-CORE 3.18, 3.20, 3.21, 3.22 (Do not retransmit the FIN on every
 * subsequent packets, unless retransmission occurs.)
 */
                tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2, TM_TCPF2_SND_FIN );
            }
            if (updateSndNxt != TM_8BIT_ZERO)
            {
/*
 * 4.r Not probing a zero window and not a congestion condition. Update.
 */
                tcpVectPtr->tcpsSndNxt = sndNxt;
            }
            else
            {
                if (checkUpdateSndNxt != TM_8BIT_ZERO)
                {
/*
 * 4.r Congestion condition. Retransmitting one segment at sndUna. Check
 * whether we have re-transmitted more than previously sent. If so, update.
 */
                    if (tm_sqn_gt(sndNxt, tcpVectPtr->tcpsSndNxt))
                    {
                        tcpVectPtr->tcpsSndNxt = sndNxt;
                    }
                }
            }
/*#define lMaxSndNxt maxRcvWindow32*/
/* 4.s Update Maximum send next including out of window data */
            lMaxSndNxt = tcpVectPtr->tcpsSndNxt + byteOutOfWnd;
            if (tm_sqn_gt(lMaxSndNxt, tcpVectPtr->tcpsMaxSndNxt))
            {
                tcpVectPtr->tcpsMaxSndNxt = lMaxSndNxt;
            }
#ifdef TM_TCP_FACK
            else
            {
                if (fackAlgorithm != TM_8BIT_ZERO)
                {
                    tcpVectPtr->tcpsRetranData += tcpLength;
                }
            }
#endif /* TM_TCP_FACK */
#ifdef TM_USE_TCP_PACKET
tcpSendNoData:
#endif /* TM_USE_TCP_PACKET */
/* 4.t Total packet length to be sent. */
            tcpLength = (tt32Bit)(tcpLength + tcpHeaderSize);
            packetPtr->pktChainDataLength = (ttPktLen)tcpLength;
/* 4.u If we increased our adversized window, update it */
/* ANVL-CORE 14.20: update tcpsRcvAdv if advRecvSpace is < 0 */
            if ((rcvWndAdvIncr != 0) || (advRecvSpace < 0))
            {
/* keep track of our biggest advertized receive window */
/*#define lOurRcvWindow32 effSndWindow32 */
/*#define lRcvAdv effSndWindow32*/
                lRcvAdv = tcpVectPtr->tcpsRcvNxt + lOurRcvWindow32;
                tcpVectPtr->tcpsRcvAdv = lRcvAdv;
            }
        }
        else
        {
/* 3. preformatted packets (i.e SYN, RESET, Keep alive, or TIME WAIT packet) */
            tcpVectPtr->tcpsHeaderPacketPtr = TM_PACKET_NULL_PTR;
            tcphPtr = (ttTcpHeaderPtr)packetPtr->pktLinkDataPtr;
            tcpLength = (tt32Bit)packetPtr->pktLinkDataLength;
#ifdef TM_DEV_SEND_OFFLOAD
            tcpHeaderSize = tcpVectPtr->tcpsHeaderSize;
#endif /* TM_DEV_SEND_OFFLOAD */
        }
#ifdef TM_SNMP_MIB
        if (snmpRetransFlg == (tt8Bit)1)
        {
/*
 * Number of segments retransmitted - that is, the number of TCP segments
 * transmitted containing one or more previously transmitted octets.
 * Here we send data below the maximum we have ever sent, so we are
 * re-transmitting.
 */
            tm_context(tvTcpData).tcpRetransSegs++;
        }
        else
        {
/*
 * Number of segments sent, including those on current connections
 * but excluding those containing only retransmitted octets.
 * Here we are transmitting more bytes than previously, since we
 * are increasing our MaxSndNxt.
 */
#ifdef TM_USE_NG_MIB2
            tm_64Bit_incr(tm_context(tvTcpData).tcpHCOutSegs);
#else /* TM_USE_NG_MIB2 */
            tm_context(tvTcpData).tcpOutSegs++;
#endif /* TM_USE_NG_MIB2 */
        }
#endif /* TM_SNMP_MIB */
/* 5. common code for preformatted packet and others */
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
        if (tcpVectPtr->tcpsNetworkLayer == TM_NETWORK_IPV4_LAYER)
#endif /* dual */
        {
/* 5.a Point to IP header */
            ipHdrLength = TM_4PAK_IP_MIN_HDR_LEN;
            iphPtr = (ttIpHeaderPtr)(((tt8BitPtr)tcphPtr) - ipHdrLength);
/* Initialize packet IP header length */
            packetPtr->pktIpHdrLen = (tt8Bit)ipHdrLength;
        }
#ifdef TM_USE_IPV6
        else
#endif /* dual layer */
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
        {
/* 5.a Point to IP header */
            ipHdrLength = TM_6PAK_IP_MIN_HDR_LEN;
            if (packetPtr->pktSharedDataPtr->dataFlags & TM_BUF_HDR_RECYCLED)
            {
                iph6Ptr = packetPtr->pkt6TxIphPtr;
            }
            else
            {
                iph6Ptr = (tt6IpHeaderPtr)(((tt8BitPtr)tcphPtr) - ipHdrLength);
            }
        }
#endif /* TM_USE_IPV6 */
/* Assume pseudo header has been cached (most common case) */
       pseudoXsum = tcpVectPtr->tcpsPseudoXsum;
/*
 * For IPsec, the recycled header maybe encrypted, not reliable.
 * For PPP the recycled header might be modified during PPP VJ compression.
 * In those cases, it is IPsec/PPP responsibility to prevent the buffer
 * from being recycled by resetting TM_BUF_TCP_HDR_BLOCK in dataFlags
 * as follows:
 * packetPtr->pktSharedDataPtr->dataFlags &= ~TM_BUF_TCP_HDR_BLOCK;
 */
        if (!(packetPtr->pktSharedDataPtr->dataFlags & TM_BUF_HDR_RECYCLED))
/*
 *  5.b If TCP header is not recycled, find route (if not cached), fill in
 *      IP source and destination address, IP Tos, IP Ttl, TCP source port
 *      and destination port.
 */
        {
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
            if (iphPtr != (ttIpHeaderPtr)0)
#endif /* dual */
            {
/* IPv4 protocol family, or IPV4 mapped IPV6 address, use IPv4 header */
                errorCode = tfIpDestToPacket(
                                  packetPtr,
#ifdef TM_4_USE_SCOPE_ID
                                  &socketPtr->socOurIfIpAddress,
                                  &socketPtr->socPeerIpAddress,
#else /* ! TM_4_USE_SCOPE_ID */
                                  tm_4_ip_addr(socketPtr->socOurIfIpAddress),
                                  tm_4_ip_addr(socketPtr->socPeerIpAddress),
#endif /* ! TM_4_USE_SCOPE_ID */
                                  socketPtr->socIpTos,
                                  &socketPtr->socRteCacheStruct,
                                  iphPtr );
                if (errorCode == TM_ENOERROR)
                {
/* Compute the pseudo header checksum  without the TCP header length */
                    if (!(tm_tcp_send_xsum_cached(
                                            tcpVectPtr->tcpsOffloadFlags)))
/* If pseudo header checksum has previously been cached, no need to recompute */
                    {
/*
 * Pseudo header pointer. Use iphPtr:
 */
                        pshPtr = (ttPseudoHeaderPtr)(
                            ((tt8BitPtr)iphPtr) +
                            (unsigned)(TM_4PAK_IP_MIN_HDR_LEN
                                       - TM_4PAK_PSEUDO_HDR_LEN) );
/*
 * Initialize pseudo header length with 0 (instead of tcplength.
 * Overlays IP header checksum
 */
                        pshPtr->pshLayer4Len = TM_16BIT_ZERO;
/* Initialize pseudo header zero field. Overlays TTL */
                        pshPtr->pshZero = TM_8BIT_ZERO;
/* Uncomplemented Pseudo header checksum */
                        pseudoXsum = tfPseudoHeaderChecksum(
                            (tt16BitPtr)(ttVoidPtr)pshPtr,
/* Pseudo header size */
                            (tt16Bit)tm_byte_count(TM_4PAK_PSEUDO_HDR_LEN));
/* Cache pseudo header checksum */
                        tcpVectPtr->tcpsOffloadFlags |= TM_DEVO_XSUM_CACHED;
                        tcpVectPtr->tcpsPseudoXsum = pseudoXsum;
                    }
/* Initialize IP TTL */
                    iphPtr->iphTtl = socketPtr->socIpTtl;
                }
            }
#ifdef TM_USE_IPV6
            else
#endif /* dual layer */
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
            {
/* IPv6 protocol family */
#ifdef TM_6_USE_MIP_RO
                if ((tm_context(tv6MipRoEnable) == TM_8BIT_NO)
#ifdef TM_6_USE_MIP_MN
                    || (tm_16bit_one_bit_set(
                            socketPtr->socOptions, TM_6_IPO_MN_USE_HA_TUNNEL))
#endif /* TM_6_USE_MIP_MN */
                    )
                {
/* this socket does not support route optimization, or route optimization
   is globally disabled, so bypass route optimization for this packet */
                    packetPtr->pktFlags |= TM_6_MIP_BYPASS_RO_FLAG;
                }
#endif /* TM_6_USE_MIP_RO */

                errorCode = tf6IpDestToPacket(
                                    packetPtr,
                                    &socketPtr->socOurIfIpAddress,
                                    &socketPtr->socPeerIpAddress,
                                    &socketPtr->socRteCacheStruct,
                                    iph6Ptr);
                if (errorCode == TM_ENOERROR)
                {
/* Compute the pseudo header checksum  without the TCP header length */
                    if (!(tm_tcp_send_xsum_cached(
                                            tcpVectPtr->tcpsOffloadFlags)))
/* If pseudo header checksum has previously been cached, no need to recompute */
                    {
/* Pseudo header checksum computation */
/*
 * Pseudo header pointer:
 * The pseudo-header overlays the IP header.
 */
                        psh6Ptr = (tt6PseudoHeaderPtr)((tt8BitPtr)iph6Ptr);
/* Save IP Hops limit as set by tf6IpDestToPacket */
                        saved6IpHops = iph6Ptr->iph6Hops;
/*
 * Initialize pseudo header length with 0 (will need to add length later.
 * Overlays flow label
 */
/* Pseudo header checksum only */
                        psh6Ptr->psh6Layer4Len = (tt32Bit)0;
/* Initialize pseudo header zero field. Overlays Payload, hops */
                        tm_6_zero_arr(psh6Ptr->psh6ZeroArr);
/* ULP field */
                        psh6Ptr->psh6Protocol = IPPROTO_TCP;

/* Uncomplemented Pseudo header checksum */
                        pseudoXsum = tfPseudoHeaderChecksum(
/* Compute the pseudo header checksum without the TCP length */
                                    (tt16BitPtr)(ttVoidPtr)psh6Ptr,
/* Pseudo header size */
                                    (tt16Bit)TM_6_IP_MIN_HDR_LEN);
/* Cache pseudo header checksum */
                        tcpVectPtr->tcpsOffloadFlags |= TM_DEVO_XSUM_CACHED;
                        tcpVectPtr->tcpsPseudoXsum = pseudoXsum;
                        iph6Ptr->iph6Hops = saved6IpHops;
                    }
/* Change disovered IPv6 hops if user had set it. */
/* Socket-level IPV6_UNICAST_HOPS.([IPV6REQ].R2.18:10) */
                    if (socketPtr->soc6HopLimit != -1)
                    {
/* User set value */
                        iph6Ptr->iph6Hops = (tt8Bit)socketPtr->soc6HopLimit;
                    }
                    iph6Ptr->iph6Nxt = IPPROTO_TCP;
/* Initialize IP header traffic class && flow label */
                    iph6Ptr->iph6Flow = socketPtr->soc6FlowInfo;
                }
            }
#endif /* TM_USE_IPV6 */
            if (errorCode != TM_ENOERROR)
            {
                if (errorCode == TM_EHOSTUNREACH)
                {
                    tcpVectPtr->tcpsSoftError = errorCode;
                    errorCode = TM_ENOERROR;
                }
                needFreePacket = TM_8BIT_YES;
                break;
            }
            tcphPtr->tcpSrcPort = socketPtr->socOurLayer4Port;
            tcphPtr->tcpDstPort = socketPtr->socPeerLayer4Port;
        }
#ifdef TM_DEV_SEND_OFFLOAD
        if (tm_tcp_send_offload(tcpVectPtr->tcpsOffloadFlags))
        {
            pktSendOffloadPtr =
                    &(packetPtr->pktSharedDataPtr->dataDevSendOffload);
#ifdef TM_TCP_SACK
/*
 * If SACK is enabled the effSndMss is reduced by the SACK option length,
 * because we added the sackOptLength to tcpHeaderSize
 */
            effSndMss = (tt16Bit)(tcpVectPtr->tcpsEffSndMss - sackOptLength);
#else /* TM_TCP_SACK */
            effSndMss = tcpVectPtr->tcpsEffSndMss;
#endif /* TM_TCP_SACK */
            if (    tm_tcp_send_seg_offload(tcpVectPtr->tcpsOffloadFlags)
                 && (tcpLength > (tt32Bit)(effSndMss + tcpHeaderSize)) )
            {
/* Segmentation offload */
                pktSendOffloadPtr->devoFlags = (tt16Bit)
                             (   (tcpVectPtr->tcpsOffloadFlags)
                               & (tt16Bit)(   TM_DEVO_TCP_SEGMENT_OFFLOAD_V4
                                           | TM_DEVO_TCP_SEGMENT_OFFLOAD_V6
                                           | TM_DEVO_TCP_CHKSUM_OFFLOAD
                                           | TM_DEVO_IP_CHKSUM_OFFLOAD ) );
/* Maximum TCP or UDP payload segment not including header */
                pktSendOffloadPtr->devoFramePayload = effSndMss;
/* TCP/IP or UDP/IP header length to be used in each frame */
                pktSendOffloadPtr->devoFrameHdrLen = tcpHeaderSize;
                pktSendOffloadPtr->devoPayload = (ttUser32Bit)
                                                    (tcpLength - tcpHeaderSize);
            }
            else
            {
/* Checksum offload only */
                if (tm_tcp_send_xsum_offload(tcpVectPtr->tcpsOffloadFlags))
                {
                    pktSendOffloadPtr->devoFlags = (tt16Bit)
                                         (    tcpVectPtr->tcpsOffloadFlags
                                           & (   TM_DEVO_TCP_CHKSUM_OFFLOAD
                                               | TM_DEVO_IP_CHKSUM_OFFLOAD ) );
/*
 * Offset to partial checksum
 */
                }
                else
                {
                    pktSendOffloadPtr = (ttDevSendOffloadPtr)0;
                }
            }
        }
        else
        {
            pktSendOffloadPtr = (ttDevSendOffloadPtr)0;
        }
        packetPtr->pktDevOffloadPtr = (ttVoidPtr)pktSendOffloadPtr;
#endif /* TM_DEV_SEND_OFFLOAD */
/* 5.c TCP Checksum computation */
/* 5.c Zero the tcp checksum for computation */
        tcphPtr->tcpChecksum =  TM_16BIT_ZERO;
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
        if (iphPtr != (ttIpHeaderPtr)0)
#endif /* dual */
        {
            tcp16Length = (tt16Bit)tcpLength;
            tm_htons(tcp16Length, pshLayer4Len); /*pshLayer4Len */
            sum = pshLayer4Len;
#ifdef TM_DEV_SEND_OFFLOAD
            needXsumUpdate = TM_8BIT_YES;
/* Compute the checksum */
            if (pktSendOffloadPtr != (ttDevSendOffloadPtr)0)
            {
/* Partial checksum */
                tcphPtr->tcpChecksum = tcpVectPtr->tcpsPseudoXsum;
                pktSendOffloadPtr->devoTucse = 0;
                pktSendOffloadPtr->devoTucss = (tt16Bit)
                         (   packetPtr->pktDeviceEntryPtr->devLinkHeaderLength
                           + TM_4_IP_MIN_HDR_LEN );
                pktSendOffloadPtr->devoFrameHdrLen = (tt16Bit)
                               (   pktSendOffloadPtr->devoFrameHdrLen
                                 + pktSendOffloadPtr->devoTucss );
                pktSendOffloadPtr->devoTucso = (tt16Bit)
                                       (   pktSendOffloadPtr->devoTucss
                                         + TM_TCP_BYTE_XSUM_OFFSET );
                if ((tm_tcp_send_seg_offload(pktSendOffloadPtr->devoFlags )))
                {
                    needXsumUpdate = TM_8BIT_NO;
                }
                if (pktSendOffloadPtr->devoFlags & TM_DEVO_IP_CHKSUM_OFFLOAD)
                {
/* IP header checksum */
/* offset to first byte to be included in Xsum */
                   pktSendOffloadPtr->devoIpcss =
                           packetPtr->pktDeviceEntryPtr->devLinkHeaderLength;
                   pktSendOffloadPtr->devoIpcso = (tt16Bit)
                                         (  pktSendOffloadPtr->devoIpcss
                                          + TM_IP_BYTE_XSUM_OFFSET );
/* Offset to last byte to be in included in Xsum */
                   pktSendOffloadPtr->devoIpcse = (tt16Bit)
                                              (   pktSendOffloadPtr->devoIpcss
                                                + (TM_4_IP_MIN_HDR_LEN - 1) );
                }
            }
            else
#endif /* TM_DEV_SEND_OFFLOAD */
            {
/* Full checksum */
                tcphPtr->tcpChecksum = tfPacketChecksum(
                                packetPtr,
                                (ttPktLen)tcpLength,
/* No Pseudo header */
                                (tt16BitPtr)0,
                                TM_16BIT_ZERO);
                tcphPtr->tcpChecksum = ~(tt16Bit)tcphPtr->tcpChecksum;
/* Add pseudo header */
                sum += pseudoXsum;
            }
/* 5.d Make the packet point to IP header */
            packetPtr->pktLinkDataPtr = (tt8BitPtr)iphPtr;
/*
 * 5.d Update the ChainDataLength and LinkDataLength, because we now point
 * to the IP header
 */
            ipHdrLength = tm_byte_count(ipHdrLength);
            packetPtr->pktChainDataLength += ipHdrLength;
            packetPtr->pktLinkDataLength += ipHdrLength;
        }
#ifdef TM_USE_IPV6
        else
#endif /* dual layer */
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
        {
            tm_htonl(tcpLength, sum);
/* Compute the checksum */
#ifdef TM_DEV_SEND_OFFLOAD
            needXsumUpdate = TM_8BIT_YES;
            if (pktSendOffloadPtr != (ttDevSendOffloadPtr)0)
            {
                if ((tm_tcp_send_seg_offload(pktSendOffloadPtr->devoFlags )))
                {
                    needXsumUpdate = TM_8BIT_NO;
                }
/* Partial checksum */
                tcphPtr->tcpChecksum = tcpVectPtr->tcpsPseudoXsum;
                pktSendOffloadPtr->devoTucse = 0;
                pktSendOffloadPtr->devoTucss = (tt16Bit)
                         (   packetPtr->pktDeviceEntryPtr->devLinkHeaderLength
/* OL: need to change this */
                           + TM_6_IP_MIN_HDR_LEN );
                pktSendOffloadPtr->devoFrameHdrLen = (tt16Bit)
                                         (   pktSendOffloadPtr->devoFrameHdrLen
                                           + pktSendOffloadPtr->devoTucss );
                pktSendOffloadPtr->devoTucso = (tt16Bit)
                                     (    pktSendOffloadPtr->devoTucss
                                        + TM_TCP_BYTE_XSUM_OFFSET );
/* Add TCP length to checksum */
            }
            else
#endif /* TM_DEV_SEND_OFFLOAD */
            {
/* Full checksum */
                tcphPtr->tcpChecksum = tfPacketChecksum(
                                             packetPtr,
                                             (ttPktLen)tcpLength,
/* No pseudo header */
                                             (tt16BitPtr)0,
                                             TM_16BIT_ZERO );
                tcphPtr->tcpChecksum = (tt16Bit)~tcphPtr->tcpChecksum;
/* Add pseudo header */
                sum += pseudoXsum;
            }
        }
#endif /* TM_USE_IPV6 */
#ifdef TM_DEV_SEND_OFFLOAD
        if (needXsumUpdate != TM_8BIT_NO)
#endif /* TM_DEV_SEND_OFFLOAD */
        {
            sum = sum + tcphPtr->tcpChecksum;
            tm_add_carries_sum(sum);
#ifdef TM_DEV_SEND_OFFLOAD
            if (pktSendOffloadPtr != (ttDevSendOffloadPtr)0)
            {
                tcphPtr->tcpChecksum = (tt16Bit)sum;
            }
            else
#endif /* TM_DEV_SEND_OFFLOAD */
            {
                if ((tt16Bit)sum == (tt16Bit)0xFFFF)
                {
                    tcphPtr->tcpChecksum = (tt16Bit)0;
                }
                else
                {
                    tcphPtr->tcpChecksum = (tt16Bit)~((tt16Bit)sum);
                }
            }
        }
/* 5.e Since we are sending an ACK and possibly a probe, reset those flags */
        if (tcpVectPtr->tcpsFlags2 & TM_TCPF2_DELAY_ACK)
        {
            tm_tcp_timer_suspend(tcpVectPtr->tcpsDelayAckTmPtr);
        }
        tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2,
                          (TM_TCPF2_DELAY_ACK|TM_TCPF2_ACK|TM_TCPF2_PROBE_OOB));
/* 5.f Remember our last Ack sent (for time stamp) */
        tcpVectPtr->tcpsLastAckSent = tcpVectPtr->tcpsRcvNxt;
/*
 * 5.g If we have sent SYN/new data/FIN, turn on retransmission timer.
 *     Need to set it here, in case we are going through the loop again.
 *     If probe window timer is on, delete it.
 */
        if (tcpVectPtr->tcpsSndNxt != tcpVectPtr->tcpsSndUna)
        {
            if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags3,
                                     TM_TCPF3_PROBE_MODE))
            {
                tm_tcp_timer_suspend(tcpVectPtr->tcpsWndProbeTmPtr);
                tm_tcp_maxrt_suspend(tcpVectPtr);
                tcpVectPtr->tcpsFlags3 = (tt16Bit)
                                              (   tcpVectPtr->tcpsFlags3
                                                & ~(TM_TCPF3_PROBE_MODE) );
                tcpVectPtr->tcpsReXmitCnt = 0;
            }
            if (tm_16bit_bits_not_set( tcpVectPtr->tcpsFlags2,
                                       TM_TCPF2_REXMIT_MODE ) )
            {
/* Make sure the retransmit timer is re-initialized when re-enabled */
                tcpVectPtr->tcpsFlags2 |=
                        (TM_TCPF2_REINIT_TIMER | TM_TCPF2_REXMIT_MODE);
            }
        }
#if TM_DEF_SEND_TRAILER_SIZE > 0
        if (packetPtr->pktLinkExtraCount == 0)
        {
/*
 * Do not recycle the TCP header, if the user might corrupt the
 * TCP header by sliding the packet.
 */
            packetPtr->pktSharedDataPtr->dataFlags &= ~TM_BUF_TCP_HDR_BLOCK;
        }
#endif /* TM_DEV_SEND_TRAILER_SIZE > 0 */
/* 5.h Send the packet */
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6 /* dual */
        if (iphPtr != (ttIpHeaderPtr)0)
#endif /* dual */
        {
#ifdef TM_USE_QOS
            iphPtr->iphTos = packetPtr->pktIpTos;
#endif /* TM_USE_QOS */
            errorCode = tfIpSendPacket(
                packetPtr,
                &socketPtr->socLockEntry
#ifdef TM_USE_IPSEC
                , (ttPktLenPtr)0
#ifdef TM_USE_IPSEC_TASK
                , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
#endif /* TM_USE_IPSEC */
                );
        }
#ifdef TM_USE_IPV6
        else
#endif /* dual */
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
        {
            errorCode = tf6IpSendPacket(packetPtr,
                                        &socketPtr->socLockEntry
#ifdef TM_USE_IPSEC_TASK
                                        , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
                                        );
        }
#endif /* TM_USE_IPV6 */

        packetPtr = TM_PACKET_NULL_PTR;
#ifdef TM_TCP_RFC3042
/*
 * [RFC3042]R2:2.60
 *  At the end of the do while loop, decrease the number of segments to
 *  send if it is not 0.
 */
        if (segmentsToSend)
        {
            segmentsToSend--;
        }
#endif /* TM_TCP_RFC3042 */
    }
/*
 * 5.i Check if we need to send more data (either because we determined that
 *     there is more data in the send queue, or another thread piggied back
 *     on this send thread. (Will go to step 4).
 */
    while (((tcpVectPtr->tcpsFlags2) & TM_TCPF2_SEND_DATA) != 0);
tcpOutSendLoop:
/* 6. Out of the TCP send loop. */
#ifdef TM_USE_SSL
/*
 * Reset the TM_SSLF_TCP_NO_DELAY flag.
 */
    tm_16bit_clr_bit( tcpVectPtr->tcpsSslFlags, TM_SSLF_TCP_NO_DELAY );
#endif /* TM_USE_SSL */
/*
 * 6.a Let other users have access to tfTcpSendPacket, by setting the
 *     send clear flag.
 */
    tcpVectPtr->tcpsFlags2 |= TM_TCPF2_SEND_CLEAR;
    if ( tm_16bit_all_bits_set( tcpVectPtr->tcpsFlags2,
                                  TM_TCPF2_REXMIT_MODE
                                | TM_TCPF2_REINIT_TIMER) )
    {
/*
 * 6.b (re)-initialization of retransmit timer interval either because we
 *     just enabled the retransmit timer, or because we are retransmitting
 *     some data in this routine.
 */
        if (tcpVectPtr->tcpsReXmitTmPtr != (ttTimerPtr)0)
        {
#ifdef TM_USE_TCP_REXMIT_CONTROL
            if (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags3,
                                      TM_TCPF3_REXMIT_PAUSE))
#endif /* TM_USE_TCP_REXMIT_CONTROL */
            {
                tm_timer_new_time( tcpVectPtr->tcpsReXmitTmPtr,
                                   tcpVectPtr->tcpsRto );
            }
        }
    }
/*
 * 6.c Reset the REINIT_TIMER flag to allow the retransmission timer to
 *     run (if in retransmit mode).
 *     Also reset the path MTU discovery error bit.
 */
    tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2,
                      (TM_TCPF2_REINIT_TIMER | TM_TCPF2_DF_ERROR) );
    if (needFreePacket != TM_8BIT_ZERO)
    {
/*
 *  6.d If an error occured (could not duplicate data in the send queue, or
 *     there was no route to the destination), we need to free the packet.
 *     The header cannot be recycled.
 */
        tm_16bit_clr_bit( packetPtr->pktSharedDataPtr->dataFlags,
                          TM_BUF_TCP_HDR_BLOCK );
        tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
    }

tcpSendPacketExit:
    if (errorCode != TM_EACCES)
    {
        errorCode = TM_ENOERROR;
    }
    return errorCode;

#undef lOurRcvWindow32
#undef lRcvAdv
#undef lTempWindow32
#undef lMaxSndNxt
#undef l32UrgentOffset
}
#ifdef TM_LINT
LINT_UNACCESS_SYM_END(window16)
LINT_NULL_PTR_END(sendQPacketPtr)
LINT_NULL_PTR_END(packetPtr)
#endif /* TM_LINT */

/*
 * tfTcpIncomingPacket() function description:
 * 1 Tcp header check and pointer adjustments.
 *   a. Save IP header pointer, and save TCP header pointer
 *   b. Make the packet point the TCP pseudo-header, and Check the TCP
 *      checksum
 *   c. Compute and check TCP header length.
 *   d. Make the packet point to the TCP user data, updating
 *      pktLinkDataLength, pktChainDataLength, and pktLinkDataPtr
 * 2. Target incoming socket. Check incoming TCP cache first, lock
 *    the cache. Lock the cached socket entry, and check for hit. If we have
 *    a hit, get ownership of the socket. If we do not have a hit, unlock the
 *    cached socket entry, and lookup the socket tree. If we find a hit, we
 *    already got socket ownership for the incoming thread, get ownership of
 *    the socket for the cache, purge the cache, and store the new socket
 *    entry in the cache. If we did not find a matching socket in the tree,
 *    use the static closed state vector to be able to use the common state
 *    machine code. Unlock the cache.
 * 3. Retrieve information from TCP header
 *    a. Retrieve information from TCP header in host byte order
 *    b. Retrieve TCP control bits (flags)
 *    c. segment unscaled window, (unscale only for non SYN segment)
 *    d. Next Check options for time stamp on non original SYN segment
 *       (non SYN segment, or duplicate SYN segment)
 *       (time stamp, if used would be on each data and ack segment)).
 *       We will check for original SYN segment options later on in the state
 *       machine. If options are present on non SYN segment or duplicate SYN
 *       segment, most common case first: check for 32-bit aligned timestamp
 *       option, otherwise, scan the option for the timestamp option. If the
 *       timestamp value is 0, change it to 1, since a value of 0 means
 *       an invalid timestamp.
 *    e. At this point we have extracted information from the TCP header, and
 *       a targeted TCP state vector/socket entry. Reset idle time, since we
 *       just got a TCP packet.
 *    f. Compute our advertized receive space
 *    g. Error checking: make sure that we did not advertize more than
 *       our real receive space.
 * 4. Header Prediction (Van Jacobson algorigthm):
 *    This algorithm allows to bypass the TCP state machine when the next
 *    in sequence expected segment comes from the network, and improves TCP
 *    throughput. (This algorithm is disabled if the user does not define
 *    TM_OPTIMIZE_SPEED.)
 *    First check that we are in established state with ACK bit or ACK|PUSH
 *    bits only, with either no time stamp or in order segment, and sequence
 *    number is the next expected receive segment, and peer's window has not
 *    changed (no window update), and we have not retransmitted data, and
 *    socket is not shutdown for read (header prediction condition). If this
 *    is true:
 *    a. If the starting sequence number of the received segment is less or
 *       equal to the left edge of the window (lastAckSent):
 *              SEG.SEQ <= Last.ACK.sent
 *       then the TSval from the segment is copied to TS.Recent;
 *       otherwise, the TSval is ignored. (update to RFC 1323)
 *    b. If data length is zero, check for Pure ACK for outstanding data:
 *       if new data is acknowledged (i.e if it advances the left edge of
 *       the send window) and ack number is valid (within the range of data
 *       sent) and we are not doing slow start or congestion avoidance
 *       If time stamp on use time stamp mechanism to update rtt, otherwise
 *       we will use the Vegas algorithm when we free the acked buffers.
 *       Free ACKed buffers, update send queue, update RTT (since we
 *       are advancing the left edge of the send window)
 *       Advance the left edge of the send window
 *       If all data has been acknowledged, cancel retransmit timer,
 *       Otherwise re-initialize the retransmit timer timeout, to time the
 *       next segment. Jump to the end of TCP incoming the routine where
 *       we check if we need to post on, or call back the user thread (for
 *       send notify/call back).
 *    c. Otherwise, segment contains some data. If segment does not ack
 *       any new data, reassemble queue is empty, and data fits in our receive
 *       queue, queue data in socket receive queue, set the read flag,
 *       so that we will post to the user thread, or call the receive user
 *       call back function upon exit of this routine. Jump to the end
 *       of the TCP incoming routine where we check if we need to post
 *       on, or call back the user thread (for read notify/call back).
 * 5. We reach this point if we either did not satisfy all the conditions
 *    of the TCP header algorithm, or it was disabled because TM_OPTIMIZE_SPEED
 *    is not defined.
 *    a. Retrieve Urgent pointer.  We had not retrieved it yet. (Did not
 *       need it for the header prediction code).
 * 6. PAWS check (if time stamp option is on):
 *       This is checked here, since this is common code for all
 *       combination states/events (except for RESETS), when TsOn is
 *       non zero.
 *       Protection Against Wrapped Sequence numbers protects against
 *       old duplicate non-SYN segments, and duplicate SYN segments
 *       received while there is a synchronized connection (RFC 1323).
 *       (Note that we checked the time stamp option for all packets
 *        satisfying these conditions)
 *       RST segments should be acceptable regardless of their
 *       timestamp (RFC 1323).
 *       Drop packets which satisfy the conditions stated above, and
 *       whose time stamp value is smaller than the most recent saved
 *       one, but if the connection had been idle for a long time, and the
 *       time stamp has wrapped around, just invalidate tcpsTsRecent, instead
 *       of dropping the segment. (With a 1 ms timestamp clock, the 32-bit
 *       timestamp will wrap its sign bit in 24.8 days.  So we need to
 *       check whether the last saved incoming segment time is older by
 *       more than 24 days than the current time, in which case we just
 *       invalidate the most recent incoming segment time, instead of
 *       dropping the segment (RFC 1323).)
 * 7. RFC793 Incoming segment acceptability test for state >= SYN_RECEIVED:
 *  This is checked here since this is common code for all states >=
 *  SYN_RECEIVED.
 *  If there is data (including SYN and FIN), we will first make sure that
 *  the segment fits between rcvNxt and the end of our receive window by
 *  trimming off any portions that lie outside the window (including SYN,
 *  URG, and FIN, PUSH), and checking the conditions mentionned above after
 *  having trimmed the data. If an incoming segment is not acceptable,
 *  an acknowledgment will be sent in reply (unless the RST bit is set), and
 *  the incoming segment will not be processed any further.
 *  If there is no data (including SYN and FIN) to begin with, we will
 *  keep processing to process valid ACKS, valid URGs, and Resets.
 * 8. Time stamp recording (for state >= SYN_RECEIVED):
 *  If the TSval from the segment is greater or equal to TS.Recent (checked
 *  in PAWS check above) and the starting sequence number of the received
 *  segment is less or equal to the left edge of the window (lastAckSent):
 *              SEG.SEQ <= Last.ACK.sent
 *  then the TSval from the segment is copied to TS.Recent;
 *  otherwise, the TSval is ignored. (update to RFC 1323)
 * 9. MAP control flags bits to TCP state machine events
 *    (Check on URG and PUSH done within state machine)
 * 10. Prepare all packet fields before going through the state machine
 *   a. Store the tcp header pointer in packet structure
 *   b. Store the TCP header length in packet structure
 *   c. Store timestamp values in packet structure.
 *   d. Store TCP header fields in host byte order back in tcp header.
 *   e. Store unscaled send window in packet structure.
 *   f. Store Modified control bits (we removed bits outside the receive
 *      window) back in control bits field in tcp header.
 * 11. Crank state machine once to process incoming segment (excluding FIN).
 *     State machine freed/queued the packet. If Fin is in order, and next
 *     to process, crank state machine with FIN event.
 * 12. Check if a read/write/exception event occured while we were processing
 *     the packet, in which case we need to post to / call back the user.
 * 13. Ack now/send data, if we found out that we needed to ACK, or the peer
 *     acked all of our data, or opened its receive window, or we have more
 *     data in the send queue.
 * 14. Unlock the socket
 * 15. Check if we need to free the packet (error occured earlier)
 *
 * Parameter      description
 * packetPtr      pointer to incoming data packet pointer with
 *                . packetPtr->pktChainDataLength containing the total
 *                  length of the packet
 *                . packetPtr->pktLinkDataLength containing the length
 *                  of the first scattered data area.
 *                . packetPtr->pktLinkDataPtr pointing to the IP header
 *
 * Return values
 * no return value
 */
#ifdef TM_RCM3200
/*
 * Rabbit 3000 processor requires 4K alignment of functions that produce
 * more than 4K of FARCODE.
 */
#pragma asm(".defseg FARCODE")
#pragma asm(".seg FARCODE")
#pragma asm(".align 1000h")
#endif /* TM_RCM3200 */

#ifdef TM_LINT
LINT_UNINIT_SYM_BEGIN(segWnd)
LINT_UNINIT_SYM_BEGIN(urgentPtr)
#endif /* TM_LINT */
void  tfTcpIncomingPacket (ttPacketPtr packetPtr)
{
#ifdef TM_USE_IPV6
     tt6IpHeaderPtr        iph6Ptr;
     tt6PseudoHeaderPtr    psh6Ptr;
     tt6IpAddressPtr       ipv6SrcAddrPtr;
     tt6IpAddressPtr       ipv6DestAddrPtr;
#endif /* TM_USE_IPV6 */
     ttTupleDev            tupleDev;
#ifdef TM_USE_IPV4
     ttIpHeaderPtr         iphPtr;
     ttPseudoHeaderPtr     pshPtr;
#endif /* TM_USE_IPV4 */
     ttTcpHeaderPtr        tcphPtr;
     tt8BitPtr             tcpOptionPtr;
     ttTcpVectPtr          tcpVectPtr;
     ttTcpVectPtr          closedTcpVectPtr;
     ttSocketEntryPtr      socketPtr;
     ttSocketEntryPtr      cacheSocketPtr;
     ttSocketEntryPtr      purgeCacheSocketPtr;
     ttTcpTmWtVectPtr      tcpTmWtVectPtr;
#ifdef TM_ARP_UPDATE_ON_RECV
     ttRteEntryPtr         rtePtr;
#endif /* TM_ARP_UPDATE_ON_RECV */
#if (defined(TM_USE_NATT) && defined(TM_USE_IPSEC))
     ttSadbIntRecordPtr       sadbIntPtr;
     ttPseudoHeader           prvPsh;
#endif /* TM_USE_NATT && TM_USE_IPSEC */
#ifdef TM_DEV_RECV_OFFLOAD
     ttDevRecvOffloadPtr   pktDevRecvOffloadPtr;
#endif /* TM_DEV_RECV_OFFLOAD */
     tt32Bit               segUnScaledWnd; /* segment unscaled window */
     tt32Bit               tcpRecentAgePlus24;
     tt32Bit               currentTime;
     tt32Bit               rcvWnd;
     ttSqn                 segSeq; /* Segment seq no */
     ttSqn                 segAck; /* segment ack no*/
     ttSqn                 rcvNxt; /* Next expected seq no from remote */
     ttS32Bit              trim;
     ttS32Bit              advRecvSpace;
auto tt32Bit               tsVal;
auto tt32Bit               tsEcr;
#ifdef TM_TCP_SACK
auto ttSqn                 leftEdge;
auto ttSqn                 rightEdge;
     int                   maxSackBlockLength;
     int                   sackBlockLength;
#endif /* TM_TCP_SACK */
     tt32Bit               tcpLength;
#ifdef TM_OPTIMIZE_SPEED
     tt32Bit               bytesAcked;
     ttS32Bit              rtt;
     int                   errorCode;
#endif /* TM_OPTIMIZE_SPEED */
     int                   tcpOptionLen;
     int                   optionLen;
#ifdef TM_USE_IPV4
     tt16Bit               ipHdrLen;
     tt16Bit               tcp16Length;
#ifdef TM_USE_DRV_SCAT_RECV
     int                   contByteLength;
#endif /* TM_USE_DRV_SCAT_RECV */
#endif /* TM_USE_IPV4 */
#ifdef TM_DSP
     unsigned int          dataOffset;
#endif /* TM_DSP */
/* Amount of data to add to point to the TCP header */
     tt16Bit               checksum;
     tt16Bit               tcpHdrLen;
     tt16Bit               segWnd; /* segment window */
     tt16Bit               urgentPtr;  /* segment urgent pointer */
     tt16Bit               userTsOn;
     tt8Bit                option;
     tt8Bit                tcpCtlBits;
     tt8Bit                tsOn;
     tt8Bit                event;
     tt8Bit                idle24Days;
     tt8Bit                acceptableSeg;
     tt8Bit                tcpState;
#ifdef TM_USE_IPV6
#ifdef TM_ARP_UPDATE_ON_RECV
     int                   match;
#endif /* TM_ARP_UPDATE_ON_RECV */
#endif /* TM_USE_IPV6 */
#ifdef TM_DSP
     int                   bytePosition;
#endif /* TM_DSP */
#define lSocketCBFlags     tcpOptionLen
#define lSelectFlags       optionLen

#ifdef TM_TCP_SACK
    leftEdge = (ttSqn)0;
    rightEdge = (ttSqn)0;
#endif /* TM_TCP_SACK */
#ifdef TM_SNMP_MIB
/*
 * Number of segments received, including those received in error.
 * This count includes segments received on currently established
 * connections.
 */
#ifdef TM_USE_NG_MIB2
    tm_64Bit_incr(tm_context(tvTcpData).tcpHCInSegs);
#else /* TM_USE_NG_MIB2 */
    tm_context(tvTcpData).tcpInSegs++;
#endif /* !TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
    purgeCacheSocketPtr = (ttSocketEntryPtr)0;
#ifdef TM_USE_IPV4
    ipHdrLen = TM_16BIT_ZERO; /* for compiler warning */
    pshPtr = (ttPseudoHeaderPtr)0; /* for compiler warning */
#if (defined(TM_USE_NATT) && defined(TM_USE_IPSEC))
    sadbIntPtr = (ttSadbIntRecordPtr)0;
    tm_bzero(&prvPsh,sizeof(ttPseudoHeader));
#endif /* TM_USE_NATT && TM_USE_IPSEC */
#ifdef TM_USE_IPV6
    if (packetPtr->pktNetworkLayer == TM_NETWORK_IPV4_LAYER)
#endif /* dual */
    {
/* 1.a Save IP header pointer, and save TCP header pointer */
/* IP header length */
        ipHdrLen = (tt16Bit)packetPtr->pktIpHdrLen;
#ifdef TM_USE_DRV_SCAT_RECV
        contByteLength = tm_byte_count(ipHdrLen + TM_PAK_TCP_HDR_LEN);
/* If IP header + TCP header is not contiguous, make it contiguous */
        if (tm_pkt_hdr_is_not_cont(packetPtr, contByteLength))
        {
            packetPtr = tfContiguousSharedBuffer( packetPtr,
                                                  contByteLength );
            if (packetPtr == (ttPacketPtr)0)
            {
                goto tcpIncomingExit;
            }
        }
#endif /* TM_USE_DRV_SCAT_RECV */
/* Save pointer to IP header */
        iphPtr = (ttIpHeaderPtr)packetPtr->pktLinkDataPtr;
#ifdef TM_USE_IPV6
        iph6Ptr = (tt6IpHeaderPtr)0;
#endif /* dual */
/* 1.b Point to the TCP header for packet checksum */
        tm_pkt_ulp_hdr(packetPtr, ipHdrLen);
/* TCP length from packet/IP information */
        tcpLength = (tt32Bit)packetPtr->pktChainDataLength;
#ifdef TM_DEV_RECV_OFFLOAD
        pktDevRecvOffloadPtr = (ttDevRecvOffloadPtr)packetPtr->pktDevOffloadPtr;
        if (    (pktDevRecvOffloadPtr != (ttDevRecvOffloadPtr)0)
             && (   pktDevRecvOffloadPtr->devoRFlags
                   & (   TM_DEVOR_TCP_RECV_CHKSUM_VALIDATED ) ) )
        {
/* No need to validate the checksum */
            checksum = TM_16BIT_ZERO;
        }
        else
#endif /* TM_DEV_RECV_OFFLOAD */
        {
            if (!tm_context(tvTcpIncomingChecksumOn))
            {
/* We do not validate the checksum */
                checksum = TM_16BIT_ZERO;
            }
            else
            {
/* 1.b Point to the pseudo header for packet checksum */
                pshPtr = (ttPseudoHeaderPtr)(
                    ((tt8BitPtr)iphPtr) +
                    (unsigned)
                        (TM_4PAK_IP_MIN_HDR_LEN - TM_4PAK_PSEUDO_HDR_LEN) );
#if (defined(TM_USE_NATT) && defined(TM_USE_IPSEC))
/* Because a NAT change is done, dst of TCP pseudo header is a
 * local IP address. It is changed to a global IP address to use therefore.
 */
                sadbIntPtr = (ttSadbIntRecordPtr)packetPtr->pktSadbRecordPtr;
/* SA doesn't exist in the case of BYPASS and so on.Therefore, it is copied
 * only in the (except for 0) case that SA exists.But, it is copied on prvPsh
 * because received IP header is rewritten.
 */
                if (sadbIntPtr != (ttSadbIntRecordPtr)0)
                {
                    tm_bcopy(pshPtr, &prvPsh, sizeof(ttPseudoHeader));
                    tm_ip_copy(sadbIntPtr->sadbiNatOaDstAddr,
                           pshPtr->pshDstIpAddr);
                    tm_ip_copy(sadbIntPtr->sadbiNatOaSrcAddr,
                               pshPtr->pshSrcIpAddr);
                }
#endif /* TM_USE_NATT && TM_USE_IPSEC */
                tcp16Length = (tt16Bit)tcpLength;
                tm_htons(tcp16Length, pshPtr->pshLayer4Len); /* network order */
                pshPtr->pshZero = TM_8BIT_ZERO;
#ifdef TM_DEV_RECV_OFFLOAD
                if (    (pktDevRecvOffloadPtr != (ttDevRecvOffloadPtr)0)
                     && (   pktDevRecvOffloadPtr->devoRFlags
                          & TM_DEVOR_PACKET_RECV_CHKSUM_COMPUTED ) )
                {
/*
 * Checksum only the pseudo header and add to hardware computed checksum on
 * TCP header + data.
 */
                    checksum = tfDevoRecvPacketChecksum(
                                           pktDevRecvOffloadPtr,
                                           (tt16BitPtr)pshPtr,
                                           tm_byte_count(TM_4PAK_PSEUDO_HDR_LEN) );
                }
                else
#endif /* TM_DEV_RECV_OFFLOAD */
                {
/* Check the TCP checksum */
                    checksum = tfPacketChecksum(
                        packetPtr,
                        (ttPktLen)tcpLength,
                        (tt16BitPtr)(ttVoidPtr)pshPtr,
                        tm_byte_count(TM_4PAK_PSEUDO_HDR_LEN));
                }
            }
        }
#if (defined(TM_USE_NATT) && defined(TM_USE_IPSEC))
        packetPtr->pktSadbRecordPtr = (ttSadbRecordPtr)0;
#endif /* TM_USE_NATT && TM_USE_IPSEC */
    }
#ifdef TM_USE_IPV6
    else
#endif /* dual layer */
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    {
/* Dual IP layer. Initialize the iphPtr to zero. */
#ifdef TM_USE_IPV4
        iphPtr = (ttIpHeaderPtr)0;
#endif /* dual */
#ifdef TM_USE_DRV_SCAT_RECV
/* If TCP header is not contiguous, make it contiguous */
        if ( tm_pkt_hdr_is_not_cont( packetPtr,
                                     tm_byte_count(TM_PAK_TCP_HDR_LEN)) )
        {
            packetPtr = tfContiguousSharedBuffer(
                                          packetPtr,
                                          tm_byte_count(TM_PAK_TCP_HDR_LEN) );
            if (packetPtr == (ttPacketPtr)0)
            {
                goto tcpIncomingExit;
            }
        }
#endif /* TM_USE_DRV_SCAT_RECV */
        iph6Ptr = packetPtr->pkt6RxIphPtr;
        psh6Ptr = packetPtr->pkt6RxPseudoHdrPtr;
/*
 * Build the TCP pseudo-header from the received IPv6 packet header
 */
/* TCP length from packet/IP information */
        tcpLength = (tt32Bit)(packetPtr->pktChainDataLength);
#ifdef TM_DEV_RECV_OFFLOAD
        pktDevRecvOffloadPtr = (ttDevRecvOffloadPtr)packetPtr->pktDevOffloadPtr;
        if (    (pktDevRecvOffloadPtr != (ttDevRecvOffloadPtr)0)
             && (   pktDevRecvOffloadPtr->devoRFlags
                   & (   TM_DEVOR_TCP_RECV_CHKSUM_VALIDATED ) ) )
        {
/* No need to validate the checksum */
            checksum = TM_16BIT_ZERO;
        }
        else
#endif /* TM_DEV_RECV_OFFLOAD */
        {
            if (!tm_context(tvTcpIncomingChecksumOn))
            {
/* We do not validate the checksum */
                checksum = TM_16BIT_ZERO;
            }
            else
            {
/* Store the length of the TCP message in the pseudo header. */
                tm_htonl(tcpLength, psh6Ptr->psh6Layer4Len); /* network order */
                tm_6_zero_arr(psh6Ptr->psh6ZeroArr);
/* Restore because deleted by psh6Zero assignment */
                psh6Ptr->psh6Protocol  = IPPROTO_TCP;
#ifdef TM_DEV_RECV_OFFLOAD
                if (    (pktDevRecvOffloadPtr != (ttDevRecvOffloadPtr)0)
                     && (   pktDevRecvOffloadPtr->devoRFlags
                           & (TM_DEVOR_PACKET_RECV_CHKSUM_COMPUTED ) ) )
                {
/*
 * Checksum only the pseudo header and add to hardware computed checksum on
 * TCP header + data.
 */
                    checksum = tfDevoRecvPacketChecksum(
                                             pktDevRecvOffloadPtr,
                                             (tt16BitPtr)psh6Ptr,
                                             (tt16Bit)TM_6_IP_MIN_HDR_LEN );
                }
                else
#endif /* TM_DEV_RECV_OFFLOAD */
                {
/* Compute the TCP checksum on the TCP message and pseudo header */
                    checksum = tfPacketChecksum( packetPtr,
                                                 packetPtr->pktChainDataLength,
                                                 (tt16BitPtr)psh6Ptr,
                                                 (tt16Bit)TM_6_IP_MIN_HDR_LEN);
                }
            }
        }
    }
#endif /* TM_USE_IPV6 */
    if (checksum != TM_16BIT_ZERO)
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfTcpIncomingPacket",
                        "Checksum on incoming TCP packet failed");
#endif /* TM_ERROR_CHECKING */
#ifdef TM_SNMP_MIB
        tm_context(tvTcpData).tcpInErrs++;
#endif /* TM_SNMP_MIB */
        goto tcpIncomingExit; /* free packet and exit */
    }
#if (defined(TM_USE_NATT) && defined(TM_USE_IPSEC))
    else
    {
        if (sadbIntPtr != (ttSadbIntRecordPtr)0)
        {
/* NAT-OAi and NAT-OAr are copied and because Checksum was calculated,
 * it returns it.
 */
            tm_ip_copy(prvPsh.pshDstIpAddr,
                       pshPtr->pshDstIpAddr);
            tm_ip_copy(prvPsh.pshSrcIpAddr,
                       pshPtr->pshSrcIpAddr);
        }
    }
#endif /* TM_USE_NATT && TM_USE_IPSEC */
/* 1.c Save pointer to TCP header */
    tcphPtr = (ttTcpHeaderPtr)(packetPtr->pktLinkDataPtr);
/* 1.c TCP Header length in bytes */
    tcpHdrLen = tm_tcp_hdrLen(tcphPtr->tcpDataOffset);
/* Check validity of Header length */
    if (((tt32Bit)tcpHdrLen > tcpLength) || (tcpHdrLen < TM_BYTES_TCP_HDR))
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfTcpIncomingPacket",
                        "Option length too big or too small");
#endif /* TM_ERROR_CHECKING */
#ifdef TM_SNMP_MIB
        tm_context(tvTcpData).tcpInErrs++;
#endif /* TM_SNMP_MIB */
        goto tcpIncomingExit; /* free packet and exit */
    }
#ifdef TM_USE_DRV_SCAT_RECV
/* If TCP header is not contiguous, make it contiguous */
    if (    (tcpHdrLen != TM_BYTES_TCP_HDR)
         && tm_pkt_hdr_is_not_cont(packetPtr, tcpHdrLen) )
    {
        packetPtr = tfContiguousSharedBuffer( packetPtr,
                                              tcpHdrLen );
        if (packetPtr == (ttPacketPtr)0)
        {
            goto tcpIncomingExit;
        }
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
        if (iphPtr != (ttIpHeaderPtr)0)
#endif /* dual */
        {
            iphPtr = (ttIpHeaderPtr)(packetPtr->pktLinkDataPtr - ipHdrLen);
        }
#ifdef TM_USE_IPV6
        else
#endif /* dual layer */
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
        {
            iph6Ptr = packetPtr->pkt6RxIphPtr;
        }
#endif /* TM_USE_IPV6 */
        tcphPtr = (ttTcpHeaderPtr)(packetPtr->pktLinkDataPtr);
    }
#endif /* TM_USE_DRV_SCAT_RECV */
/*
 * 1.d Make the packet point to the TCP user data, updating pktLinkDataLength,
 *    pktChainDataLength, and pktLinkDataPtr
 */
/* data length (without TCP header) */
    tcpLength = (tt32Bit)(tcpLength - (tt32Bit)tcpHdrLen);
#ifdef TM_USE_TCP_REXMIT_TEST
    if (    (tm_context(tvTcpDropPackets) == TM_8BIT_YES)
         && ((tm_context(tvTcpDataOnly) == TM_8BIT_NO) ||
             (tcpLength != 0x00)))
    {
        goto tcpIncomingExit;
    }
#endif /* TM_USE_TCP_REXMIT_TEST */
/* Point to User data (after checksum) */
    packetPtr->pktLinkDataLength -= (ttPktLen)tcpHdrLen;
    packetPtr->pktLinkDataPtr =   ((tt8BitPtr)tcphPtr)
                                + tm_packed_byte_len(tcpHdrLen);
    packetPtr->pktChainDataLength = (ttPktLen)tcpLength;
/*
 * 2. Target incoming socket
 */
/* Copy local/remote port for socket look up */
    tupleDev.tudTuple.sotLocalIpPort = tcphPtr->tcpDstPort; /* local */
    tupleDev.tudTuple.sotRemoteIpPort = tcphPtr->tcpSrcPort; /* remote */
/* Copy and prepare local/remote IP address for socket lookup */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
    if (iphPtr != (ttIpHeaderPtr)0)
/* IPv4 header */
    {
/* Dual mode map to IPV6 address */
        tupleDev.tudAddressFamily = AF_INET;
/* pktSpecDestIpAddress already ipv4 to ipv6 mapped */
        ipv6DestAddrPtr = &(packetPtr->pktSpecDestIpAddress);
        tm_6_ip_copy_dststruct(ipv6DestAddrPtr,
                               tupleDev.tudTuple.sotLocalIpAddress);
/* Source iphSrcAddr IPv4 address => ipv6 mapped */
        ipv6SrcAddrPtr = &(tupleDev.tudTuple.sotRemoteIpAddress);
        tm_6_addr_to_ipv4_mapped(iphPtr->iphSrcAddr,
                                 ipv6SrcAddrPtr);
#ifdef TM_4_USE_SCOPE_ID
        ipv6DestAddrPtr = &(tupleDev.tudTuple.sotLocalIpAddress);
        tm_4_dev_scope_addr(ipv6DestAddrPtr, packetPtr->pktDestDevPtr);
        tm_4_dev_scope_addr(ipv6SrcAddrPtr, packetPtr->pktDeviceEntryPtr);
#endif /* TM_4_USE_SCOPE_ID */
    }
    else
#endif /* dual */
/* IPv6 header */
    {
/* Add scope to real IPV6 address */
        tupleDev.tudAddressFamily = AF_INET6;
        ipv6DestAddrPtr = &tm_6_ip_hdr_in6_addr(iph6Ptr->iph6DestAddr);
        ipv6SrcAddrPtr = &tm_6_ip_hdr_in6_addr(iph6Ptr->iph6SrcAddr);
        tm_6_dev_scope_addr(ipv6DestAddrPtr, packetPtr->pktDestDevPtr);
        tm_6_dev_scope_addr(ipv6SrcAddrPtr, packetPtr->pktDeviceEntryPtr);
        tm_6_ip_copy_dststruct(ipv6DestAddrPtr,
                               tupleDev.tudTuple.sotLocalIpAddress);
        tm_6_ip_copy_dststruct(ipv6SrcAddrPtr,
                               tupleDev.tudTuple.sotRemoteIpAddress);
    }
#else /* !TM_USE_IPV6 */
    tupleDev.tudAddressFamily = AF_INET;
    tm_ip_copy(iphPtr->iphDestAddr, tupleDev.tudTuple.sotLocalIpAddress);
    tm_ip_copy(iphPtr->iphSrcAddr, tupleDev.tudTuple.sotRemoteIpAddress);
#endif /* !TM_USE_IPV6 */
    tcpTmWtVectPtr = (ttTcpTmWtVectPtr)0;
    closedTcpVectPtr = (ttTcpVectPtr)0;
/*
 * Check incoming TCP cache first:
 * LOCK the cache
 */
    tm_lock_wait(&tm_context(tvTcpCacheLock));
    cacheSocketPtr = (ttSocketEntryPtr)(ttVoidPtr)(tm_context(tvTcpVectPtr));
    if (tm_context(tvTcpVectPtr) != TM_TCPVECT_NULL_PTR)
    {
/* LOCK the socket entry */
        tm_lock_wait(&(cacheSocketPtr->socLockEntry));
/*
 * Check that it is ours. Check on packet destination port, destination
 * IP address, source port and source Ip address.
 * Note that a listening socket will never get a cache
 * hit since we never store the peer info in a listening socket (this
 * enables us to cache the accepted socket, instead of the listening socket).
 */
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
        if (    tm_16bit_one_bit_set( cacheSocketPtr->socFlags,
                                      TM_SOCF_IN_TREE )
             && (tm_context(tvTcpVectPtr)->tcpsState != TM_TCPS_CLOSED)
             && (tm_tuple_equal(tupleDev.tudTuple, cacheSocketPtr->socTuple))
#ifdef TM_USE_STRONG_ESL
             && (    tm_8bit_bits_not_set(cacheSocketPtr->socFlags2,
                                          TM_SOCF2_BINDTODEVICE)
                  || (cacheSocketPtr->socRteCacheStruct.rtcDevPtr ==
                                           packetPtr->pktDeviceEntryPtr)
                )
#endif /* TM_USE_STRONG_ESL */
#ifdef TM_USE_IPDUAL
/* Prevent IPv4 sockets from receiving IPv6 packets; there may be an
 * IPv6-only socket available */
             && !(   cacheSocketPtr->socProtocolFamily == AF_INET
                  && tupleDev.tudAddressFamily != AF_INET )
/* Prevent IPv6-only sockets from receiving IPv4 packets; there may be
 * an IPv4 socket available */
             && !(   tm_16bit_one_bit_set( cacheSocketPtr->soc6Flags,
                                                        TM_6_SOCF_V6ONLY )
                  && tupleDev.tudAddressFamily != AF_INET6 )
#endif /* TM_USE_IPDUAL */
           )
/* PRQA L:L1 */
        {
/* Cache hit for socket. */
            tcpVectPtr = tm_context(tvTcpVectPtr);
            tcpState = tcpVectPtr->tcpsState;
            userTsOn =  (tt16Bit)(tcpVectPtr->tcpsFlags & TM_TCPF_TS);
            socketPtr = cacheSocketPtr;
/* owned by incoming thread */
            tm_socket_checkout(socketPtr);
        }
        else
        {
/* Not ours. Lookup socket/state vector in tree */
            tm_unlock(&(cacheSocketPtr->socLockEntry));
            goto lookupSocket;
        }
    }
    else
    {
/* Lookup socket in tree */
lookupSocket:
/*
 * If cache and queue of detached state vectors lookup failed, lookup the
 * socket tree. If lookup succeeds, we get a checked out locked socket
 * (owned by the incoming thread).
 */
#if (defined(TM_USE_STRONG_ESL) || defined(TM_IGMP) || defined(TM_6_USE_MLD))
        tupleDev.tudDevPtr = packetPtr->pktDeviceEntryPtr;
#endif /* TM_USE_STRONG_ESL || TM_IGMP || TM_6_USE_MLD */
        tupleDev.tudProtocolNumber = TM_IP_TCP;
        tcpVectPtr = (ttTcpVectPtr)(ttVoidPtr)tfSocketLookup(
            &tupleDev,
            TM_SOC_TREE_ALL_FLAG
            );
        if (    (tcpVectPtr != TM_TCPVECT_NULL_PTR)
/* If there is a socket at destination port (owned by incoming thread) */
             && (tm_16bit_bits_not_set(tcpVectPtr->tcpsSocketEntry.socFlags,
                                       TM_SOCF_LISTENING)))
/* and not a listening socket */
        {
/* TCP state */
            tcpState = tcpVectPtr->tcpsState;
            socketPtr = &(tcpVectPtr->tcpsSocketEntry);
            userTsOn =  (tt16Bit)(tcpVectPtr->tcpsFlags & TM_TCPF_TS);
            if (tcpVectPtr != tm_context(tvTcpVectPtr))
/*
 * If not already cached, try and cache the looked up socket
 */
            {
/*
 * If there is a cached socket that is different, remember to check it in
 * later when we no longer have a lock on the current socket.
 */
                purgeCacheSocketPtr = cacheSocketPtr;
/* Initialize the cache, with looked up socket */
                tm_context(tvTcpVectPtr) = tcpVectPtr;
/* Socket owned by cache */
                tm_socket_checkout(socketPtr);
            }
        }
        else
/* no socket, or listening socket */
        {
            if (tcpVectPtr == TM_TCPVECT_NULL_PTR)
            {
/*
 * If no socket/state vector use empty socket/close state vector,
 * so that we can use the state machine. Empty socket/close state vector
 * is locked, and checked out too, so that we can use common code to checkin
 * and unlock.
 */
                tcpVectPtr = tfTcpClosedVectGet();
                if (tcpVectPtr == TM_TCPVECT_NULL_PTR)
                {
                    goto tcpIncomingExit;
                }
                closedTcpVectPtr = tcpVectPtr;
            }
/* else listening socket */
/* Reset ACK that could have been previously set */
            socketPtr = &(tcpVectPtr->tcpsSocketEntry);
/* try and find a Time Wait vector  */
            tm_call_lock_wait(&tm_context(tvTcpTmWtTableLock));
            if (tm_context(tvTcpTmWtTableMembers) != 0)
            {
#ifdef TM_USE_STRONG_ESL
                tupleDev.tudDevPtr = packetPtr->pktDeviceEntryPtr;
#endif /* TM_USE_STRONG_ESL */
                tcpTmWtVectPtr = tfTcpTmWtLookup(&tupleDev);
            }
            else
            {
                tcpTmWtVectPtr = (ttTcpTmWtVectPtr)0;
            }
            if (tcpTmWtVectPtr == (ttTcpTmWtVectPtr)0)
            {
                tm_call_unlock(&tm_context(tvTcpTmWtTableLock));
/* TCP state */
                tcpState = tcpVectPtr->tcpsState;
                userTsOn = (tt16Bit)(tcpVectPtr->tcpsFlags & TM_TCPF_TS);
            }
            else
            {
/* TCP state */
                tcpState = TM_TCPS_TIME_WAIT;
                tfTcpTmWtTsInit(tcpTmWtVectPtr, tcpVectPtr);
                userTsOn = (tt16Bit)(tcpTmWtVectPtr->twsFlags & TM_TCPTWF_TS);
            }
        }
    }
/* UNLOCK the cache */
    tm_unlock(&tm_context(tvTcpCacheLock));
/* Save sockTuple in case it is needed in the recv TCP state machine */
    packetPtr->pktSharedDataPtr->dataSockTuplePtr = &(tupleDev.tudTuple);

#ifdef TM_ARP_UPDATE_ON_RECV
/*
 * If this socket has an associated ARP entry, refresh the ARP cache entry.
 * This prevents the stack from sending ARP requests every 10 minutes
 * on an active ARP entry, i.e. an ARP entry for a destination IP address
 * that the stack is actively sending to, which is unnecessary if
 * there is incoming traffic from the peer, and if this check is enabled.
 */
    rtePtr = socketPtr->socRteCacheStruct.rtcRtePtr;
    if (    (rtePtr != TM_RTE_NULL_PTR)
         && tm_16bit_one_bit_set( rtePtr->rteFlags2, TM_RTE2_UP ))
    {
        if ( tm_16bit_one_bit_set (rtePtr->rteFlags, TM_RTE_INDIRECT) )
        {
            rtePtr = socketPtr->socRteCacheStruct.rtcRteClonedPtr;
        }
#ifdef TM_USE_IPDUAL
/*
 * In dual mode, the 6-4 tunnel default gateway does not point to any ARP
 * entry
 */
        if (rtePtr != TM_RTE_NULL_PTR)
#endif /* TM_USE_IPDUAL */
        {
            if (    tm_16bit_all_bits_set( rtePtr->rteFlags,
                                           (   TM_RTE_ARP | TM_RTE_CLONED
                                             | TM_RTE_LINK_LAYER) )
/* Don't modify static ARP entries */
                 && (    (rtePtr->rteTtl != TM_RTE_INF)
#ifdef TM_USE_IPV6
                      || (rtePtr->rte6HSNudState != TM_6_NUD_IPV4_STATE)
#endif /* TM_USE_IPV6 */
                    )
               )
            {
#ifdef TM_USE_IPV6
                tm_phys_addr_match(
                    packetPtr->pkt6PhysAddr,
                    rtePtr->rteHSPhysAddr,
                    rtePtr->rteHSPhysAddrLen,
                    match);
#endif /* TM_USE_IPV6 */

                if (
/*
 * Verify that the source Ethernet address of the incoming packet matches that
 * of the ARP cache entry.
 */
#ifdef TM_USE_IPV6
                    (match == rtePtr->rteHSPhysAddrLen)
#else /* ! TM_USE_IPV6 */
                    (tm_ether_match(rtePtr->rteHSEnetAdd,
                                    packetPtr->pktEthernetAddress))
#endif /* ! TM_USE_IPV6 */
                   )
                {
/* Use a critical section to prevent the timer from removing the entry */
                    tm_kernel_set_critical;
#ifdef TM_USE_IPV6
                    if (rtePtr->rte6HSNudState == TM_6_NUD_IPV4_STATE)
                    {
#endif /* TM_USE_IPV6 */
                        rtePtr->rteTtl = tm_context(tvArpTimeout);
#ifdef TM_USE_IPV6
                    }
                    rtePtr->rteHSLastReachedTickCount = tvTime;
#endif /* TM_USE_IPV6 */
                    tm_kernel_release_critical;
                }
            }
        }
    }
#endif /* TM_ARP_UPDATE_ON_RECV */

/* Get current time stamp */
    tm_kernel_set_critical;
    currentTime = tvTime;
    tm_kernel_release_critical;
/*
 * 3. Retrieve information from TCP header
 */
/* 3.a Retrieve information from TCP header in host byte order */
    tm_ntohl(tcphPtr->tcpSeqNo, segSeq);
    tm_ntohl(tcphPtr->tcpAckNo, segAck);
    tm_ntohs(tcphPtr->tcpWindowSize, segWnd);
/* 3.b Retrieve TCP control bits (flags) */
    tcpCtlBits = tcphPtr->tcpControlBits;
/* 3.c segment unscaled window, (unscale only for non SYN segment) */
    if (    (tcpVectPtr->tcpsSndWndScale != TM_8BIT_ZERO)
         && (tm_8bit_bits_not_set(tcpCtlBits, TM_TCP_SYN)) )
    {
        segUnScaledWnd = ((tt32Bit)segWnd) << tcpVectPtr->tcpsSndWndScale;
    }
    else
    {
        segUnScaledWnd = (tt32Bit)segWnd;
    }
/*
 * 3.d Next Check options for time stamp/SEL ACKS on non original SYN segment
 * (time stamp, if used would be on each data and ack segment)).
 * We will check for original SYN segment options later on in the state
 * machine.
 * Assume no time stamp
 */
    tsOn  = TM_8BIT_ZERO;
    tsVal = TM_32BIT_ZERO; /* to avoid compiler warning */
    tsEcr = TM_32BIT_ZERO; /* to avoid compiler warning */
    if (tcpHdrLen > TM_BYTES_TCP_HDR)
/* If options are present */
    {
/*
 * If SYN flag is not set or SYN segment is a duplicate, check for TIME
 * STAMP option.
 * Most common case first: check for 32-bit aligned timestamp option,
 * otherwise, scan the option for the timestamp option.
 */
        tcpOptionPtr = (tt8BitPtr)tcphPtr + TM_PAK_TCP_HDR_LEN;
        tcpOptionLen = (int)(tcpHdrLen - TM_BYTES_TCP_HDR);
        if (    (tm_8bit_bits_not_set(tcpCtlBits, TM_TCP_SYN))
             || (    (tm_8bit_one_bit_set(tcpCtlBits, TM_TCP_SYN))
                  && (tcpState >= TM_TCPS_SYN_RECEIVED) ) )
/* not a SYN segment, or duplicate SYN segment */
        {
            if (    (tcpOptionLen >= TM_TCPO_TS_NOOP_LENGTH)
                 && (((ttTcpAlgnTsOptPtr)tcpOptionPtr)->tcpoAFirstWord ==
                                            TM_TCPO_TS_FIRST_WORD))
            {
/*
 * Time stamp option is the first option and it is aligned.
 * We need to check that option has been negotiated, because of buggy remote
 * implementations. (Windows 2000 bug.)
 */
                if (userTsOn)
                {
                    tsOn = TM_8BIT_YES;
                    tm_ntohl( ((ttTcpAlgnTsOptPtr)tcpOptionPtr)->tcpoAVal,
                              tsVal );
                    tm_ntohl( ((ttTcpAlgnTsOptPtr)tcpOptionPtr)->tcpoAEchoRpl,
                              tsEcr );
#ifdef TM_USE_SOCKET_IMPORT
                    tsEcr -= tcpVectPtr->tcpsTsCurrentFixup;
#endif /* TM_USE_SOCKET_IMPORT */
                }
                tcpOptionLen -= TM_TCPO_TS_NOOP_LENGTH;
                tcpOptionPtr += tm_packed_byte_len(TM_TCPO_TS_NOOP_LENGTH);
            }

#ifdef TM_DSP
            bytePosition = (TM_DSP_BYTES_PER_WORD - 1);
#endif /* TM_DSP */

            while (tcpOptionLen > 0)
            {
/* Check for non-aligned time stamps, SEL ACKS */
/* Get option field */
                tm_tcp_get_option(option, tcpOptionPtr, 0, bytePosition);

                if (option == TM_TCPO_EOL)
                    break;
                if (option == TM_TCPO_NOOP)
                {
                    optionLen = 1;
                    goto tcpOptLoopEnd;
                }
/* Get option length */
                tm_tcp_get_option(optionLen, tcpOptionPtr, 1, bytePosition);
                if (optionLen <= 0)
                    break;
                if (option == TM_TCPO_TS)
                {
/*
 * We need to check that option has been negotiated, because of buggy remote
 * implementations. (Windows 2000 bug.)
 */
                    if (    (userTsOn)
                         && (optionLen == TM_TCPO_TS_LENGTH) )
                    {
                        tsOn = TM_8BIT_YES;
                        tm_endian_long_copy( tcpOptionPtr, tsVal, 2,
                                             bytePosition );
                        tm_endian_long_copy( tcpOptionPtr, tsEcr, 6,
                                             bytePosition );
                    }
                }
#ifdef TM_TCP_SACK
                else
                {
                    if (    (option == TM_TCPO_SACK)
                         && tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags,
                                                 TM_TCPF_SEL_ACK)
                       )
/* SACK option that has been negotiated */
                    {
                        sackBlockLength = 0;
                        maxSackBlockLength =   optionLen
                                             - (TM_SACK_BLOCK_LENGTH + 2);
                        while ( sackBlockLength <= maxSackBlockLength )
                        {
                            tm_endian_long_copy(tcpOptionPtr,
                                                leftEdge,
                                                (2 + sackBlockLength),
                                                bytePosition);
                            tm_endian_long_copy(tcpOptionPtr,
                                                rightEdge,
                                                (6 + sackBlockLength),
                                                bytePosition);


/*
 * Mark all TCP send queue segments that lie in the range given by
 * leftEdge and rightEdge as being SEL Ack'ed by the remote.
 * Note that we have to start at the beginning of the queue for each SACK
 * block since RFC 2018 makes no requirement on SEL ACK blocks order.
 */
                            if (    tm_sqn_geq( leftEdge,
                                                tcpVectPtr->tcpsSndUna)
                                 && tm_sqn_leq( rightEdge,
                                                tcpVectPtr->tcpsMaxSndNxt) )
                            {
                                tfTcpSackQueueBlock(
                                            tcpVectPtr, leftEdge,
                                            rightEdge, TM_TCP_SACK_RECV_INDEX);
                            }
/* End of Mark the TCP send queue segments */
                            sackBlockLength += TM_SACK_BLOCK_LENGTH;
                        }
                    }
                }
#endif /* TM_TCP_SACK */

tcpOptLoopEnd:
                tcpOptionLen -= optionLen;

#ifndef TM_DSP
                tcpOptionPtr += optionLen;
#endif /* TM_DSP */

            }
            if (tsVal == TM_UL(0))
/*0 means invalid tcpsTsRecent field for us*/
            {
/*set it to 1, so as not to mark as invalid */
                tsVal = TM_UL(1);
            }
        }
    } /* end of options processing */
/*
 * 3.e At this point we have extracted information from the TCP header, and
 * a targeted TCP state vector/socket entry. Reset idle time, since we
 * just got a TCP packet.
 */
    tcpVectPtr->tcpsIdleTime = 0; /* reset idle time */
    if (tcpVectPtr->tcpsKeepAliveTmPtr != (ttTimerPtr)0)
    {
/*
 * Just resetting the idle time is not good enough, because the idle time
 * could be updated by the keep alive timer about to expire. So make sure
 * that the keep alive timer does not expire right away.
 */
        tm_timer_new_time( tcpVectPtr->tcpsKeepAliveTmPtr,
                           ((tt32Bit)tcpVectPtr->tcpsIdleIntvTime)
                           * TM_UL(1000) );
    }
/* 3.f Receive window is Advertized receive space */
    advRecvSpace = (ttS32Bit)(   tcpVectPtr->tcpsRcvAdv
                               - tcpVectPtr->tcpsRcvNxt );
    if (advRecvSpace < 0)
    {
        rcvWnd  = TM_UL(0);
    }
    else
    {
        rcvWnd = (tt32Bit)advRecvSpace;
    }
    tcpVectPtr->tcpsRcvWnd = rcvWnd;
/*
 * 4. Header Prediction (Van Jacobson algorigthm): established state with ACK
 * bit or ACK|PUSH bits only, with either no time stamp or in order segment,
 * and sequence number is the next expected receive segment, and peer's
 * window has not changed (no window update), and we have not retransmitted
 * data, and socket is not shutdown for read.
 */
#ifdef TM_OPTIMIZE_SPEED
    if (    (tcpState == TM_TCPS_ESTABLISHED)
/* ANVL-CORE 11.20 check also for no reset */
         && (    (   tcpCtlBits
                   & (TM_TCP_SYN|TM_TCP_FIN|TM_TCP_URG|TM_TCP_ACK|TM_TCP_RST))
              == TM_TCP_ACK ) /* ACK or ACK|PSH */
/* No time stamp or in order segment */
         && ( !tsOn || (tm_ts_geq(tsVal, tcpVectPtr->tcpsTsRecent)) )
/* expected segment */
         && (tcpVectPtr->tcpsRcvNxt == segSeq)
/* no zero window */
         && (segUnScaledWnd)
/* no window update */
         && (tcpVectPtr->tcpsSndWnd == segUnScaledWnd)
/* No retransmission */
         && (tcpVectPtr->tcpsMaxSndNxt == tcpVectPtr->tcpsSndNxt)
/* Socket is not shutdown for read */
         && tm_16bit_bits_not_set( socketPtr->socFlags,
                                   TM_SOCF_NO_MORE_RECV_DATA ) )
    {
/*
 * 4.a If the starting sequence number of the received segment is less or
 * equal to the left edge of the window (lastAckSent):
 *              SEG.SEQ <= Last.ACK.sent
 * then the TSval from the segment is copied to TS.Recent;
 * otherwise, the TSval is ignored. (update to RFC 1323)
 */
        if (tsOn && tm_sqn_leq(segSeq, tcpVectPtr->tcpsLastAckSent))
        {
            tcpVectPtr->tcpsTsRecent = tsVal;
            tcpVectPtr->tcpsTsRecentAge = currentTime;
        }
        if (tcpLength == TM_UL(0))
/* no data */
        {
/*
 * 4.b Check for Pure ACK for outstanding data:
 * if new data is acknowledged (i.e if it advances the left edge of the send
 * window).
 * and ack number is valid (within the range of data sent)
 * and we are not doing slow start or congestion avoidance
 */
            if (    (tm_sqn_gt(segAck, tcpVectPtr->tcpsSndUna))
                 && (tm_sqn_leq(segAck, tcpVectPtr->tcpsMaxSndNxt))
/*
 * No congestion avoidance, or slow start
 */
/*
 * OL: 4/24/01 BUG ID 11-640
 * RFC 2525: Fix of "failure of window deflation after loss recovery".
 * + Adding this check will allow clearing of TM_TCPF2_CONGESTED flag,
 *   which will allow fast rexmit more than once.
 */
                 && (tm_16bit_bits_not_set( tcpVectPtr->tcpsFlags2,
                                            TM_TCPF2_CONGESTED ))
/* OL: 4/24/01 Vegas retransmission check not in effect */
                 && (tcpVectPtr->tcpsAcksAfterRexmit == TM_8BIT_ZERO)
                 && (tcpVectPtr->tcpsCwnd >= tcpVectPtr->tcpsSndWnd) )
            {
/* OL: 4/24/01. Added reset number of consecutive duplicate acks */
                tcpVectPtr->tcpsDupAcks = 0;
                if (tsOn)
                {
/* 4.b If time stamp on use time stamp mechanism to update rtt */
                    rtt = (ttS32Bit)(currentTime - tsEcr);
                }
                else
                {
/*
 * If time stamp is not on, we need to compute rtt from the saved
 * transmission time (Vegas round trip time computation).
 */
                    rtt = (ttS32Bit)( currentTime
                       - socketPtr->socSendQueueNextPtr->pktTcpXmitTime);
                }
                bytesAcked = segAck - tcpVectPtr->tcpsSndUna;
#ifdef TM_TCP_SACK
                if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_SEL_ACK))
                {
                    tfTcpSackDeQueueBlocks( tcpVectPtr,
                                            tcpVectPtr->tcpsSndUna,
                                            segAck,
                                            TM_TCP_SACK_RECV_INDEX );
                }
#endif /* TM_TCP_SACK */
/*
 * Valid ACK. Check if we need to update the send window variables:
 */
                tfTcpUpdateSendWindow(tcpVectPtr, segSeq, segAck,
                                      segUnScaledWnd);
/* 4.b Advance the left edge of the send window */
                tcpVectPtr->tcpsSndUna = segAck;
/*
 * 4.b free ACKed buffers, update send queue, update RTT (since we
 * are advancing the left edge of the send window)
 */
                tfTcpSendQueueFree( tcpVectPtr, bytesAcked, rtt, TM_8BIT_YES);
/* 4.b If all data has been acknowledged, cancel retransmit timer */
                if (tcpVectPtr->tcpsMaxSndNxt == tcpVectPtr->tcpsSndUna)
                {
/* 4.b disable retransmission timer */
                    if (tcpVectPtr->tcpsFlags2 & TM_TCPF2_REXMIT_MODE)
                    {
                        tm_tcp_timer_suspend(tcpVectPtr->tcpsReXmitTmPtr);
                        tcpVectPtr->tcpsFlags2 &= ~(TM_TCPF2_REXMIT_MODE);
                    }
                }
                else
                {
/*
 * 4.b Otherwise re-initialize the retransmit timer timeout, to time the
 * next segment.
 */
#ifdef TM_USE_TCP_REXMIT_CONTROL
                    if (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags3,
                                              TM_TCPF3_REXMIT_PAUSE))
#endif /* TM_USE_TCP_REXMIT_CONTROL */
                    {
                        tm_tcp_timer_new_time( tcpVectPtr->tcpsReXmitTmPtr,
                                               tcpVectPtr->tcpsRto );
                    }
                }
#ifdef TM_USE_TCP_INCOMING_CB
                tm_tcp_set_hdr_ptr(packetPtr, tcphPtr);
/* 10.b. Tcp header length */
                packetPtr->pktTcpHdrLen = (tt8Bit)tcpHdrLen;
                tfSockCBPacketNotify(socketPtr, packetPtr);
#endif /* TM_USE_TCP_INCOMING_CB */
                goto tcpIncomingFlags; /* Post and return */
            }
        }
        else
/* 4.c There is data */
        {
            if (   (tcpVectPtr->tcpsSndUna == segAck)
                && (tcpVectPtr->tcpsReassemblePtr == TM_PACKET_NULL_PTR)
                && (tcpLength <= rcvWnd) )
/*
 * 4.c segment contains some data, does not ack any new data, reassemble queue
 * is empty, data fits in our receive queue.
 */
            {
                tcpVectPtr->tcpsRcvNxt += tcpLength;
                if (    ( tcpVectPtr->tcpsDelayAckTime != TM_32BIT_ZERO )
#ifdef TM_TCP_ACK_PUSH
/* OL 3/29/01 BUG ID 11-631: workaround an Internet Explorer bug */
                     && ( tm_8bit_bits_not_set( tcpCtlBits,
                                                TM_TCP_PSH ) )
#endif /* TM_TCP_ACK_PUSH */
                   )
                {
                    tm_timer_new_time( tcpVectPtr->tcpsDelayAckTmPtr,
                                       tcpVectPtr->tcpsDelayAckTime);
                    tcpVectPtr->tcpsFlags2 |= TM_TCPF2_DELAY_ACK;
                }
                else
                {
                    tcpVectPtr->tcpsFlags2 |= TM_TCPF2_ACK;
                }
/*
 * Valid ACK. Check if we need to update the send window variables:
 */
                tfTcpUpdateSendWindow(tcpVectPtr, segSeq, segAck,
                                      segUnScaledWnd);
#ifdef TM_USE_TCP_INCOMING_CB
                tm_tcp_set_hdr_ptr(packetPtr, tcphPtr);
/* 10.b. Tcp header length */
                packetPtr->pktTcpHdrLen = (tt8Bit)tcpHdrLen;
                tfSockCBPacketNotify(socketPtr, packetPtr);
#endif /* TM_USE_TCP_INCOMING_CB */
/* 4.c Queue data to incoming socket */
                errorCode = tfSocketIncomingPacket( socketPtr,
                                                    packetPtr );
                if (errorCode == TM_ENOERROR)
                {
/*
 * 4.c update the flags so that we will post on the user thread, or
 *     call back the user call recv back routine at the end of this routine
 */
                    tcpVectPtr->tcpsSelectFlags |= TM_SELECT_READ;
                    tcpVectPtr->tcpsSocketCBFlags |= TM_CB_RECV;
                }
                packetPtr = (ttPacketPtr)0;
                goto tcpIncomingFlags; /* Post and return */
            }
        }
    }
#endif /* TM_OPTIMIZE_SPEED */
/*
 * 5. We did not satisfy all the conditions of the header prediction (or
 *    it was disabled
 */
/*
 *  5.a Retrieve Urgent pointer.  We had not retrieved it yet. (Was not
 *      needed for header prediction code)
 */
    tm_ntohs(tcphPtr->tcpUrgentPtr, urgentPtr);
/*
 * 6. PAWS check (if time stamp on):
 *     This is checked here, since this is common code for all
 *     combination states/events (except for RESETS), when TsOn is
 *     non zero.
 *     Protection Against Wrapped Sequence numbers protects against
 *     old duplicate non-SYN segments, and duplicate SYN segments
 *     received while there is a synchronized connection (RFC 1323).
 *     (Note that we checked the time stamp option for all packets
 *     satisfying these conditions)
 *     RST segments should be acceptable regardless of their
 *     timestamp (RFC 1323).
 *     Drop packets which satisfy the conditions stated above, and
 *     whose time stamp value is smaller than the most recent saved
 *     one.
 */
    if (    (tsOn != TM_8BIT_ZERO)
         && (tm_8bit_bits_not_set(tcpCtlBits, TM_TCP_RST))
         && (tcpVectPtr->tcpsTsRecent != TM_32BIT_ZERO)
         && (tm_ts_lt(tsVal, tcpVectPtr->tcpsTsRecent)) )
    {
/*
 * but if the connection had been idle for a long time, and the
 * time stamp has wrapped around, just invalidate tcpsTsRecent, instead
 * of dropping the segment.
 * With a 1 ms timestamp clock, the 32-bit timestamp will
 * wrap its sign bit in 24.8 days.  So we need to check whether
 * the last saved incoming segment time is older by more than
 * 24 days than the current time, in which case we just
 * invalidate the most recent incoming segment time, instead of
 * dropping the segment (RFC 1323).
 */
        tcpRecentAgePlus24 = tcpVectPtr->tcpsTsRecentAge + TM_24DAYS_MS;
        idle24Days = (tt8Bit)tm_ts_gt(currentTime, tcpRecentAgePlus24);
        if (idle24Days != TM_8BIT_ZERO)
        {
            tcpVectPtr->tcpsTsRecent = TM_UL(0); /* invalidate tcpsTsRecent */
        }
        else
        {
/* drop segment and ack it */
            tcpVectPtr->tcpsFlags2 |= TM_TCPF2_ACK;
            goto tcpIncomingFlags; /* ack and return */
        }
    }
/*
 * 7. RFC793 Incoming segment acceptability test for state >= SYN_RECEIVED:
 *    Segment Receive  Test
 *     Length  Window
 *    ------- -------  -------------------------------------------
 *
 *       0       0     SEG.SEQ = RCV.NXT
 *
 *       0      >0     RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND
 *
 *      >0       0     not acceptable
 *
 *      >0      >0     RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND
 *                  or RCV.NXT =< SEG.SEQ+SEG.LEN-1 < RCV.NXT+RCV.WND
 *
 *  Note that when the receive window is zero no segments should be
 *  acceptable except ACK segments.  Thus, it is possible for a TCP to
 *  maintain a zero receive window while transmitting data and receiving
 *  ACKs.  However, even when the receive window is zero, a TCP must
 *  process the RST and URG fields of all incoming segments.
 * End of RFC 793 note.
 */
/*  7. This is checked here since this is common code for all states >=
 *  SYN_RECEIVED.
 *  If there is data (including SYN and FIN), we will first make sure that
 *  the segment fits between rcvNxt and the end of our receive window by
 *  trimming off any portions that lie outside the window (including SYN,
 *  URG, and FIN, PUSH), and checking the conditions mentionned above after
 *  having trimmed the data. If an incoming segment is not acceptable,
 *  an acknowledgment will be sent in reply (unless the RST bit is set), and
 *  the incoming segment will not be processed any further.
 *  If there is no data (including SYN and FIN) to begin with, we will
 *  keep processing to process valid ACKS, valid URGs, and Resets.
 */
    if (tcpState >= TM_TCPS_SYN_RECEIVED)
    {
/* Assume segment is acceptable */
        acceptableSeg = TM_8BIT_YES;
/* What segment we expect next from the other side */
        if (tcpTmWtVectPtr != (ttTcpTmWtVectPtr)0)
        {
            rcvNxt = tcpTmWtVectPtr->twsRcvNxt;
        }
        else
        {
            rcvNxt = tcpVectPtr->tcpsRcvNxt;
        }
/*
 * If there are bytes in the segment (including FIN and SYN), let us trim
 * all the bytes (including FIN and SYN) outside the acceptable range,
 * as described in RFC 793 incoming segment acceptability test
 */
/* First, let us first compute the Segment length adding FIN and SYN bits */
        if (tm_8bit_one_bit_set(tcpCtlBits, TM_TCP_FIN|TM_TCP_SYN))
        {
/* At least one of them is set */
            tcpLength++;
            if (tm_8bit_all_bits_set(tcpCtlBits, TM_TCP_FIN|TM_TCP_SYN))
            {
/* Both of them are set */
                tcpLength++;
            }
        }
/*
 * Second, let us trim the data outside the acceptable range, and check
 * for Keep Alive Probes.
 */
        if (tcpLength > TM_UL(0)) /* Data, and/or FIN, and/or SYN */
        {
            trim = (ttS32Bit)(rcvNxt - segSeq);
            if (trim > TM_L(0))
/*
 * Head trim. This includes duplicate SYN and FIN
 * Duplicate data (data before rcvNxt, no need to check on our receive
 * window). One of our ACK was lost or remote is sending a Keep alive.
 */
            {
                if ( (tt32Bit)trim >= tcpLength )
                {
/* All data is duplicate data, or Keep Alive Probe */
                    tcpVectPtr->tcpsFlags2 |= TM_TCPF2_ACK;
/*
 * Remove all data from segment, including FIN and SYN. Note that we
 * remove the data from the segment, but we allow further processing,
 * since we could have a valid ACK with this duplicate data. (For example
 * the peer could have missed one of our ACK, but acking further data that
 * he has received from us after the ACK that he missed. Could also be
 * a duplicate FIN because the remote missed our ACK, in which case
 * we would want to process it to restart the 2MSL timeout for example
 * if we were in TIMEWAIT state). Also if we were to self-connect, this
 * code will be executed, and we need to process the ACK to go to the
 * established state.
 */
                    tcpLength = TM_UL(0);
/* BUG ID 1497: land attack change */
/*
 * Note that these 2 lines of code will prevent TCP simultaneous TCP open, and
 * in particular will prevent the connection of a single TCP socket to itself.
 */
                    if (    (tcpState != TM_TCPS_SYN_RECEIVED)
                         || (!(tm_8bit_all_bits_set(tcpCtlBits,
                                                TM_TCP_ACK | TM_TCP_SYN))) )
/* End of BUG ID 1497: land attack change */
                    {
                        tm_8bit_clr_bit( tcpCtlBits,
                              (TM_TCP_FIN|TM_TCP_SYN|TM_TCP_URG|TM_TCP_PSH) );
                    }
/*
 * else receiving a SYN-ACK in SYN received state. This is either due to
 * a land attack where a peer sends a SYN with a spoofed source IP address
 * to our own IP address, which causes us to send a SYN-ACK to ourselves, or
 * due to a TCP socket connecting to itself (simultaneous open).
 * Keep the SYN-ACK (i.e do not reset the SYN bit), so that the state machine
 * aborts closes the connection to ourselves.
 * Note: if the attacker had used an SQN different from our ISS (most
 * cases), the state machine would have handled the attack properly without
 * the above change, i.e. it would have sent a reset, upon getting this
 * SYN-ACK segment without the SYN bit, because it would have failed the
 * ISS ACK check.
 */
                }
                else
                {
/* Some of the data is duplicate data. Trim it */
                    tcpLength = (tt32Bit)(tcpLength - (tt32Bit)trim);
/*
 * After the trim, tcpLength could still include a FIN, but no longer
 * includes a SYN
 * Next update segSeq.
 */
                    segSeq = rcvNxt;
/* Update send window (since it is relative to segSeq) */
                    if (segUnScaledWnd >= (tt32Bit)trim)
                    {
/* based off updated seq */
                        segUnScaledWnd -= (tt32Bit)trim;
                    }
                    else
                    {
                        segUnScaledWnd = TM_UL(0);
                    }
/* Update urgent pointer (since it is relative to segSeq) */
                    if (tm_8bit_one_bit_set(tcpCtlBits, TM_TCP_URG))
                    {
                        if (urgentPtr > (tt16Bit)trim)
                        {
                            urgentPtr = (tt16Bit)(urgentPtr - (tt16Bit)trim);
                        }
                        else
                        {
/* Ugent data is duplicate */
                            tm_8bit_clr_bit( tcpCtlBits, TM_TCP_URG );
                        }
                    }
/* If SYN was on, remove it since it is a duplicate */
                    if (tm_8bit_one_bit_set(tcpCtlBits, TM_TCP_SYN))
                    {
/* trim was including the SYN byte, remove it so that we can update dataPtr */
                        trim--;
                        tm_8bit_clr_bit( tcpCtlBits, TM_TCP_SYN );
                    }
                    if (   (tt32Bit)trim
                         > (tt32Bit)packetPtr->pktLinkDataLength )
                    {
#ifdef TM_ERROR_CHECKING
                        if (packetPtr->pktLinkDataLength ==
                                                packetPtr->pktChainDataLength)
                        {
                            tfKernelError("tfTcpIncomingPacket",
                                          "corrupted packet");
                        }
                        else
#endif /* TM_ERROR_CHECKING */
                        {
/* tfPacketHeadTrim will keep the TCP header around */
                            tfPacketHeadTrim(packetPtr, (ttPktLen)trim);
                        }
                    }
                    else
                    {
#ifdef TM_DSP
                        dataOffset = (unsigned int)
                           (packetPtr->pktLinkDataByteOffset + trim);
/* Pointer points at word boundary */
                        packetPtr->pktLinkDataPtr +=
                                    tm_packed_byte_len(dataOffset);
/* Keep track of offset of pointer within a word */
                        packetPtr->pktLinkDataByteOffset =
                                    ((int)dataOffset) % TM_DSP_BYTES_PER_WORD;
#else /* !TM_DSP */
                        packetPtr->pktLinkDataPtr += trim;
#endif /* !TM_DSP */
/*
 * Subract trim, instead of storing tcpLength because tcpLength could
 * include a FIN, but trim does not (our tcpLength (before we subtracted)
 * trim was bigger than trim therefore the FIN byte (which would be the
 * last byte in the sequence) is not a duplicate and trim does not include it)
 */
                        packetPtr->pktChainDataLength -= (ttPktLen)trim;
/*
 * Subtract instead of copying pktChainDataLength, since this will also
 * work for scattered data
 */
                        packetPtr->pktLinkDataLength -= (ttPktLen)trim;
                    }
                }
            } /* end of duplicate data */
/* next Trim excess bytes off the right edge */
            if (tcpLength > TM_UL(0)) /* tail trim */
            {
/*
 * (one past Last sequence number of the incoming segment) -
 * (end of our receive window (one past last expected sequence number))
 * is the amount to trim the packet if > 0.
 * Note with a receive window of zero this will trim all incoming data
 */
                trim = (ttS32Bit)((segSeq + tcpLength) - (rcvNxt + rcvWnd));
                if (trim > TM_L(0))
                {
/*
 * The remote side is unaware of our receive window size, or he is probing
 * our zero receive window, or he is starting a new connection.
 *
 * Some data beyond the right edge of the receive window. No more FIN/PUSH.
 * Check for SYN below.
 */
                    tm_8bit_clr_bit( tcpCtlBits,  TM_TCP_FIN|TM_TCP_PSH );
                    if ( (tt32Bit)trim >= tcpLength )
                    {
/*
 * Note that this if statement will always be true for a zero receive window
 * but we will check for the special acceptability case for receive window
 * below
 */
                        tcpLength = TM_UL(0);
/*
 * Segment lies entirely outside of our receive window, or receive window
 * is zero.
 *
 * Side step special case of new connection when we are in TIME WAIT state.
 * (Further processing of such a segment will be done using the time wait
 * tcp vector (cf. tfTcpNewIss()).)
 */
                        if (    (tcpState != TM_TCPS_TIME_WAIT)
/* ANVL-CORE 8.28: Only side step if it is SYN segment, not a SYN-ACK */
                             || (tcpCtlBits != TM_TCP_SYN)
                             || (segSeq == rcvNxt) )
                        {
/* BUG ID 1848: land attack change */
                            if (    (tcpState != TM_TCPS_SYN_RECEIVED)
                                 || (!(tm_8bit_all_bits_set(tcpCtlBits,
                                                        TM_TCP_ACK | TM_TCP_SYN))) )
/* End of BUG ID 1848: land attack change */
                            {
/*
 * Not a potential land attack and
 * Segment lies entirely beyond the right edge of our receive window,
 * (including the case when the receive window is zero), and segment is
 * not a new SYN connection when we are in time wait state. We need to ACK
 * immediately. Furthermore except for the special case when the sqn is
 * within our rcv window, the segment is not acceptable.
 */
                                tcpVectPtr->tcpsFlags2 |= TM_TCPF2_ACK;
                                if (segSeq != rcvNxt)
/* Most common case is segSeq == rcvNxt, which is acceptable */
                                {
                                    if  (    (tm_sqn_lt(segSeq, rcvNxt))
                                          || (tm_sqn_geq(segSeq, rcvNxt+rcvWnd)) )
                                    {
                                        tcpVectPtr->tcpsFlags2 |= TM_TCPF2_ACK;
                                        acceptableSeg = TM_8BIT_ZERO;
                                    }
                                }
/*
 * else: segSeq within rcv window: the segment is a zero window
 * probe from peer. Data not acceptable, but we will process ACK/URG/RST
 * in state machine.
 */
                            }
                        }
                    }
                    else /* trim < tcpLength */
                    {
/*
 * Trim data from the end. Compute the trimmed data length. If there is a
 * FIN this will remove the FIN byte from tcpLength.
 */
                        tcpLength = (tt32Bit)(tcpLength - (tt32Bit)trim);
/*
 * Note at this point, if the packet includes a SYN, it is in the window
 * and is therefore erroneous, and the state machine will reset the
 * connection and get rid of the packet. So no need to check for SYN even
 * if it affects our length.
 */
                        if (packetPtr->pktLinkDataLength !=
                                                packetPtr->pktChainDataLength)
                        {
                            tfPacketTailTrim(packetPtr, (ttPktLen)tcpLength,
                                                        TM_SOCKET_LOCKED);
                        }
                        else
                        {
/* NOTE: could either subtract trim, or assign tcpLength */
                            packetPtr->pktChainDataLength =
                                                          (ttPktLen)tcpLength;
                            packetPtr->pktLinkDataLength =
                                                          (ttPktLen)tcpLength;
                        }
                    }
                }
            }
/*
 * Drop all data from packet when packet has been trimmed all the way
 */
            if (    (tcpLength == TM_UL(0))
                 && (packetPtr->pktChainDataLength != (ttPktLen)0) )
            {
/* Update packet for tcpLength == TM_UL(0) */
                if (packetPtr->pktLinkDataLength !=
                                            packetPtr->pktChainDataLength)
                {
/* Drop scattered data beyond first link */
                    tfFreePacket( (ttPacketPtr)packetPtr->pktLinkNextPtr,
                                  TM_SOCKET_LOCKED );
                    packetPtr->pktLinkNextPtr = (ttVoidPtr)0;
                }
                packetPtr->pktChainDataLength = (ttPktLen)0;
                packetPtr->pktLinkDataLength = (ttPktLen)0;
            }
        }
        else
        {
/* ANVL-CORE 3.2 + 8.21-8.29  */
/*
 * 7. RFC793 Incoming segment acceptability test for state >= SYN_RECEIVED
 *    when segment length is zero.
 *    This includes Keep Alive probes (segSeq == rcvNxt -1).
 *
 *    Segment Receive  Test
 *     Length  Window
 *    ------- -------  -------------------------------------------
 *       0       0     SEG.SEQ = RCV.NXT
 *
 *       0      >0     RCV.NXT =< SEG.SEQ < RCV.NXT+RCV.WND
 */
            if (segSeq != rcvNxt)
/* Most common case is segSeq == rcvNxt, which is acceptable */
            {
                if  (    (tm_sqn_lt(segSeq, rcvNxt))
                      || (tm_sqn_geq(segSeq, rcvNxt+rcvWnd)) )
                {
                    tcpVectPtr->tcpsFlags2 |= TM_TCPF2_ACK;
                    acceptableSeg = TM_8BIT_ZERO;
                }
            }
        }
/* 7. END of acceptability tests */
/*
 * Process valid acks, valid URGS, and Resets as required by RFC 793.
 */
        if (acceptableSeg == TM_8BIT_ZERO) /* not acceptable */
        {
            if (tm_8bit_one_bit_set(tcpCtlBits, TM_TCP_RST))
            {
/* 7. Drop invalid reset segment without ACKING (reset ACK bit) */
                tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2, TM_TCPF2_ACK );
            }
            goto tcpIncomingFlags; /* check if we need to Ack, and return */
        }
/*
 * 8. Time stamp recording:
 * If the TSval from the segment is greater or equal to TS.Recent (checked
 * in PAWS check above) and the starting sequence number of the received
 * segment is less or equal to the left edge of the window (lastAckSent):
 *              SEG.SEQ <= Last.ACK.sent
 * then the TSval from the segment is copied to TS.Recent;
 * otherwise, the TSval is ignored. (update to RFC 1323)
 */
        if (    tsOn
             && tm_sqn_leq(segSeq, tcpVectPtr->tcpsLastAckSent))
        {
            tcpVectPtr->tcpsTsRecent = tsVal;
            tcpVectPtr->tcpsTsRecentAge = currentTime;
        }
/* 7,8 End of state >= SYN_RECEIVED processing */
    }

/*
 * 9. MAP control flags bits to TCP state machine events
 *
 * Most common case first: ACK bit is set without FIN, SYN, RST
 */
    if (tm_tcpe_flag_mask(tcpCtlBits) == TM_TCP_ACK)
/* ACK set, FIN, SYN, RST not set */
    {
        event = TM_TCPE_SEG_ACK;
    }
    else
    {
        if (tm_8bit_one_bit_set(tcpCtlBits, TM_TCP_RST))
        {
            event = TM_TCPE_SEG_RST;
        }
        else
        {
            if (tm_8bit_one_bit_set(tcpCtlBits, TM_TCP_SYN))
            {
/*
 * Note SYN and FIN could be set together, but in that case we postpone
 * dealing with the FIN until we scan the reassemble queue when we reach
 * the established state. So no need to check here.
 */
                if (tm_8bit_one_bit_set(tcpCtlBits, TM_TCP_ACK))
                {
/* SYN and ACK */
                    event = TM_TCPE_SEG_SYN_ACK;
                }
                else
                {
/* SYN and no ACK */
                    event = TM_TCPE_SEG_SYN;
/*
 * BUG ID 1498 update: We now discard any SYN segment that have the FIN, or PSH,
 * or URG bit set if there is no data. If TM_USE_TCP_CERT_VU464113 is defined,
 * then we discard those segments even when there is data.
 * Note that discarding a SYN-FIN segment with data will not allow T/TCP to work.
 */
                    if (   (tm_8bit_one_bit_set(tcpCtlBits,
                                                 TM_TCP_FIN
                                                |TM_TCP_PSH
                                                |TM_TCP_URG))
#ifndef TM_USE_TCP_CERT_VU464113
                         && (tcpLength == 0)
#endif /* !TM_USE_TCP_CERT_VU464113 */
                       )
                    {
                        event = TM_TCPE_SEG_INVALID;
                    }
/* END BUG ID 1498 */
                }
            }
            else
            {
                if (tm_8bit_all_bits_set(tcpCtlBits, TM_TCP_FIN|TM_TCP_ACK))
                {
/*
 * Process the ACK first. Fin will be dealt with later if next to be
 * processed (not in reassemble queue).
 */
                    event = TM_TCPE_SEG_ACK;
                }
                else
                {
/* Invalid control bit(s) */
                    event = TM_TCPE_SEG_INVALID;
                }
            }
        }
    }
/*
 * Check on URG and PUSH done within state machine
 *
 * 10. Prepare all packet fields before going through the state machine
 * a. Store the tcp header pointer in packet structure
 * b. Store the TCP header length in packet structure
 * c. Store timestamp values in packet structure.
 * d. Store TCP header fields in host byte order back in tcp header.
 * e. Store unscaled send window in packet structure.
 * f. Store Modified control bits (we removed bits outside the receive
 *    window) back in control bits field in tcp header.
 *
 */
/*
 * 10.a. Tcp header pointer
 */
    tm_tcp_set_hdr_ptr(packetPtr, tcphPtr);
/* 10.b. Tcp header length */
    packetPtr->pktTcpHdrLen = (tt8Bit)tcpHdrLen;
#ifdef TM_USE_TCP_INCOMING_CB
    if (    (tcpTmWtVectPtr == (ttTcpTmWtVectPtr)0)
         && (socketPtr->socIndex != TM_SOC_NO_INDEX) )
    {
        tfSockCBPacketNotify(socketPtr, packetPtr);
    }
#endif /* TM_USE_TCP_INCOMING_CB */
/* 10.c. Time stamp */
    if (tsOn)
    {
/* Segment TS Echo reply */
        packetPtr->pktTcpTsEcr = tsEcr;
        packetPtr->pktSharedDataPtr->dataFlags |= TM_BUF_TCP_TS_ON;
    }
/*
 * 10.d. Store seg fields in host byte order (even for Big Endian, since some
 * of these values might have been changed in this function).
 */
    tcphPtr->tcpSeqNo = segSeq;
    tcphPtr->tcpAckNo =  segAck;
    tcphPtr->tcpUrgentPtr = urgentPtr;
/* 10.e. segment unscaled window */
    packetPtr->pktTcpUnscaledWnd = segUnScaledWnd;
/* 10.f. Modified control bits (we removed bits outside the receive window) */
    tcphPtr->tcpControlBits = tcpCtlBits;
/* 11. Crank state machine once to process incoming segment (excluding FIN) */
    if (tcpTmWtVectPtr != (ttTcpTmWtVectPtr)0)
    {
        packetPtr = tfTcpTmWtProcess(tcpTmWtVectPtr, tcpVectPtr,
                                     packetPtr, event);
/* TCP Time Wait table unlocked */
        tcpTmWtVectPtr = (ttTcpTmWtVectPtr)0;
    }
    else
    {
        (void)tfTcpStateMachine(tcpVectPtr, packetPtr, event);
/* 11. State machine freed/queued the packet */
         packetPtr = (ttPacketPtr)0;
/*
 * 11. If Fin is in order, and next to process, crank state machine with FIN
 *     event.
 */
        if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags2, TM_TCPF2_RCV_FIN))
        {
/*
 * Packet has either been freed by first state machine or queued to the
 * user, so use null packet.
 */
            tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2, TM_TCPF2_RCV_FIN );
            (void)tfTcpStateMachine(tcpVectPtr,
                                    TM_PACKET_NULL_PTR,
                                    TM_TCPE_SEG_FIN);
        }
    }
tcpIncomingFlags:
/*
 * 12 Check if a read/write/exception event occured while we were processing
 *    the packet, in which case we need to post to / call back the user.
 */
/* #define lSelectFlags       optionLen */
    lSelectFlags = tcpVectPtr->tcpsSelectFlags;
/* #define lSocketCBFlags     tcpOptionLen */
    lSocketCBFlags = tcpVectPtr->tcpsSocketCBFlags;
/* Check if we need to post/call back */
    if ( (lSelectFlags != 0) || (lSocketCBFlags != 0) )
    {
        if (lSocketCBFlags & TM_CB_SEND_COMPLT)
        {
/* If send queue is non empty, send more data */
            if (socketPtr->socSendQueueBytes != TM_32BIT_ZERO)
            {
                tcpVectPtr->tcpsFlags2 |= TM_TCPF2_SEND_DATA;
            }
        }
/*
 * OL 5/30/01 BUG ID 11-641.
 * Removed unneeded errorCode TM_ESHUTDOWN assignment on end of file.
 * It was intended to cause read to return an error on end of file, but it
 * was also causing send to fail when we were notifying for both read
 * (end of file) and write (TM_CB_SEND_COMPLT) at the same time.
 * When the user tries and reads after an end of file notification, the
 * TM_ESHUTDOWN error will still be returned, because tfTcpRecv() is called
 * by the user recv functions, and tfTcpRecv() will return TM_ESHUTDOWN
 * from the state machine run.
 */
        tcpVectPtr->tcpsSelectFlags = 0;
        tcpVectPtr->tcpsSocketCBFlags = 0;
        tfSocketNotify( socketPtr,
                        lSelectFlags,
                        lSocketCBFlags,
                        TM_ENOERROR );
    }
    if ( tm_16bit_one_bit_set( tcpVectPtr->tcpsFlags2,
                                 TM_TCPF2_ACK
                               | TM_TCPF2_SEND_DATA
                               | TM_TCPF2_REXMIT_TEMP ) )
    {
/*
 * 13. Ack now/send data, if we found out that we needed to ACK, or the peer
 *     acked all of our data, or opened its receive window, or we have more
 *     data in the send queue.
 */
        if (tcpState != TM_TCPS_TIME_WAIT)
        {
            (void)tfTcpSendPacket(tcpVectPtr);
        }
        else
        {
            if (tcpTmWtVectPtr != (ttTcpTmWtVectPtr)0)
            {
                packetPtr = tfTcpTmWtProcess(tcpTmWtVectPtr, tcpVectPtr,
                                             packetPtr, 0);
/* TCP Time Wait table unlocked */
                tcpTmWtVectPtr = (ttTcpTmWtVectPtr)0;
            }
        }
    }
/* 14. Unlock the socket */
    tm_socket_checkin_unlock(socketPtr);
    if (tcpTmWtVectPtr != (ttTcpTmWtVectPtr)0)
    {
/* Unlock the time wait vector table */
        tm_call_unlock(&tm_context(tvTcpTmWtTableLock));
    }
    if (purgeCacheSocketPtr != (ttSocketEntryPtr)0)
    {
/*
 * Previously cached socket was replaced, check it in.
 */
        tm_lock_wait(&(purgeCacheSocketPtr->socLockEntry));
        tm_socket_checkin_unlock(purgeCacheSocketPtr);
    }
    if (closedTcpVectPtr != (ttTcpVectPtr)0)
    {
        tfTcpClosedVectFree(closedTcpVectPtr);
    }
tcpIncomingExit:
/* 15. Check if we need to free the packet (error occured earlier) */
    if (packetPtr != (ttPacketPtr)0)
    {
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    }
    return;
#undef lSocketCBFlags
#undef lSelectFlags
}
#ifdef TM_LINT
LINT_UNINIT_SYM_END(urgentPtr)
LINT_UNINIT_SYM_END(segWnd)
#endif /* TM_LINT */

#else /* !TM_USE_TCP */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_TCP is not defined */
int tvTcpioDummy = 0;

#endif /* !TM_USE_TCP */
