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
 * Description: TCP functions
 * Filename: trtcp.c
 * Author: Odile
 * Date Created: 01/26/98
 * $Source: source/trtcp.c $
 *
 * Modification History
 * $Revision: 6.0.2.54 $
 * $Date: 2013/08/23 13:53:01JST $
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

/*
 * Local macros
 */
#ifdef TM_USE_TCP_SMALL_IW_DIRECT
#define TM_TCPF_INITIAL_WINDOW    TM_TCPF_SLOW_START
#else /* TM_USE_TCP_SMALL_IW_DIRECT */
#define TM_TCPF_INITIAL_WINDOW    (TM_TCPF_INDIRECT | TM_TCPF_SLOW_START)
#endif /* TM_USE_TCP_SMALL_IW_DIRECT */

/* Backward compatibility */
#ifndef TM_TCP_MAX_SCALED_WND
/* Maximum unscaled window */
#define TM_TCP_MAX_SCALED_WND  TM_UL(65535)
#endif /* TM_TCP_MAX_SCALED_WND */

/* Define PC-lint compile options */
#ifdef TM_LINT
LINT_UNREF_MACRO( TM_TCP_MSS_OPT_SIZE )
LINT_UNREF_MACRO( TM_TCP_ALIGN_TS_OPT_SIZE )
LINT_UNREF_MACRO( TM_TCP_WNDSC_OPT_SIZE )
LINT_UNREF_MACRO( TM_TCP_SACK_PERMIT_OPT_SIZE )
LINT_UNREF_MACRO( TM_TCP_MAX_CONNECT_HDR )
LINT_UNREF_MACRO( TM_TCPS_INVALID )
LINT_UNREF_MACRO( TM_TCPF_PUSH_ALL )
LINT_UNREF_MACRO( TM_TCPF_USER_SET )
LINT_UNREF_MACRO( tm_tcp_acceptable_ack )
LINT_UNREF_MACRO( tm_tcp_packet_fully_acked )
#endif /* TM_LINT */

/* TCP header size (without options) in bytes (in trmacro.h) */
/*#define TM_BYTES_TCP_HDR       20 *//*  TCP header without options */

/* TCP options sizes in bytes */
#define TM_TCP_MSS_OPT_SIZE             4
#define TM_TCP_ALIGN_TS_OPT_SIZE       12
#define TM_TCP_WNDSC_OPT_SIZE           4
#define TM_TCP_SACK_PERMIT_OPT_SIZE     4

#define TM_TCP_MAX_CONNECT_HDR (TM_BYTES_TCP_HDR            +       \
                                TM_TCP_MSS_OPT_SIZE         +       \
                                TM_TCP_ALIGN_TS_OPT_SIZE    +       \
                                TM_TCP_WNDSC_OPT_SIZE       +       \
                                TM_TCP_SACK_PERMIT_OPT_SIZE)

/* Use dataTcpHdrPtr as temporary holder for back pointer to TCP header */
#define tm_tcp_hdr_ptr(packetPtr) \
                      (packetPtr->pktSharedDataPtr->dataTcpHdrPtr)

/*
 * TCP options
 */

/*
 * Option kind values
 */
/* #define TM_TCPO_EOL         (tt8Bit)0x00  *//* End of option list. */
/* #define TM_TCPO_NOOP        (tt8Bit)0x01  *//* No-Operation. */
#define TM_TCPO_MSS            (tt8Bit)0x02    /* Maximum Segment Size. */
#define TM_TCPO_WNDSC          (tt8Bit)0x03    /* Window Scale */
#define TM_TCPO_SACK_PERMIT    (tt8Bit)0x04    /* SACK permit (in SYN) */
/* #define TM_TCPO_SACK        (tt8Bit)0x05  *//* SACK option */
/* #define TM_TCPO_TS          (tt8Bit)0x08  *//* Time Stamp */

/* #define TM_TCPO_NOOP2          (tt16Bit)0x0101 *//* No-Operation twice */

/*
 * Option lengths
 */
#define TM_TCPO_MSS_LENGTH     (tt16Bit)4 /* Maximum Segment Size length. */
#define TM_TCPO_WNDSC_LENGTH   (tt16Bit)3 /* Window Scale */
#define TM_TCPO_SACKP_LENGTH   (tt16Bit)2 /* Selective ACK permit length. */
/* SACK first word padding length */
/* #define TM_TCPO_SACK_FW_PADDED (tt16Bit)2 */
/* SACK first word length without padding */
/* #define TM_TCPO_SACK_FW_LENGTH (tt16Bit)2 */
/* SACK One SQN length (1 4-bytes sequence numbers) */
/* #define TM_TCPO_SACK_ONE_SQN   (tt16Bit)4 */
/* #define TM_TCPO_TS_LENGTH      (tt16Bit)10 *//* Time Stamp length. */
/* Time Stamp length with NOOP fillers. */
/*#define TM_TCPO_TS_NOOP_LENGTH (tt16Bit)12 */ /* in trmacro.h */

/*
 * First word of aligned Time Stamp Option:
 * TM_TCPO_NOOP, TM_TCPO_NOOP, TM_TCPO_TS, TM_TCPO_TS_LENGTH
 * Note that we expand we use 2 different macros for LITTLE_ENDIAN
 * and BIG_ENDIAN, because some compilers cannot handle tm_const_htonl()
 * on this value.
 */
/* #ifdef TM_LITTLE_ENDIAN */
/* #define TM_TCPO_TS_FIRST_WORD TM_UL(0x0A080101) *//* LITTLE ENDIAN */
/* #else *//* !TM_LITTLE_ENDIAN */
/* #define TM_TCPO_TS_FIRST_WORD TM_UL(0x0101080A) *//* BIG ENDIAN */
/* #endif *//* TM_LITTLE_ENDIAN */

/*
 *  Maximum number of SEL ACK sqns that will fit in the TCP header.
 *  Has to be a multiple of two, since there are 2 SQNs per SACK block.
 *
 * (   (   (   TM_MAX_TCP_HDR_LEN
 *           - (TM_TCPO_SACK_FW_LENGTH + TM_TCPO_SACK_FW_PADDED))
 *       - tcpVectPtr->tcpsHeaderSize )
 *   / ( TM_TCPO_SACK_ONE_SQN) )
 */
/* Maximum number of sqns in SACK option when no time stamp option is used */
/* #define TM_TCP_SACK_SQNS          8 */ /* in trmacro.h */
/* Maximum number of sqns in SACK option when time stamp option is used */
#define TM_TCP_TS_SACK_SQNS       6

/* maximum value for window scale */
#define TM_TCP_MAX_WNDSC       (tt8Bit)14

/* Compute our receive window scale based on our receive queue max size */
#define tm_tcp_rcv_wnd_scale(rcvWndScale, maxRecvQueueBytes) \
{ \
    while (   (rcvWndScale < TM_TCP_MAX_WNDSC) \
           && ((TM_TCP_MAX_SCALED_WND << rcvWndScale) < maxRecvQueueBytes)) \
    { \
          rcvWndScale++; \
    } \
}

/*
 * State of the TCP connection
 */

/*#define TM_TCPS_CLOSED       0 */ /* defined in trmacro.h */
/*#define TM_TCPS_LISTEN       1 */ /* defined in trmacro.h */
/*#define TM_TCPS_SYN_SENT     2 */ /* defined in trmacro.h */
/*#define TM_TCPS_SYN_RECEIVED 3 */ /* defined in trmacro.h */
/*#define TM_TCPS_ESTABLISHED  4 */ /* defined in trmacro.h */
/*#define TM_TCPS_CLOSE_WAIT   5 */ /* defined in trmacro.h */
/*#define TM_TCPS_FIN_WAIT_1   6 */ /* defined in trmacro.h */
/*#define TM_TCPS_CLOSING      7 */ /* defined in trmacro.h */
/*#define TM_TCPS_LAST_ACK     8 */ /* defined in trmacro.h */
/*#define TM_TCPS_FIN_WAIT_2   9 */ /* defined in trmacro.h */
/*#define TM_TCPS_TIME_WAIT    10*/ /* defined in trmacro.h */

/* invalid state */
/*#define TM_TCPS_INVALID      20*//* defined in trmacro.h */

/*
 * TCP events
 */

/* #define TM_TCPE_ACTIVE_OPEN         0 */ /* defined in trmacro.h */
/* #define TM_TCPE_PASSIVE_OPEN        1 */ /* defined in trmacro.h */
/* #define TM_TCPE_SEG_SYN             2 */ /* SYN bit on, ACK off */
/* #define TM_TCPE_SEG_SYN_ACK         3 */ /* SYN and ACK bits on */
/* #define TM_TCPE_SEG_ACK             4 */ /* ACK bit on, SYN,FIN,RST off */
/* #define TM_TCPE_SEG_FIN             5 */ /* FIN on, ACK on */
/* #define TM_TCPE_SEG_RST             6 */ /* RST bit on */
/* #define TM_TCPE_SEG_INVALID         7 */ /* Invalid control bits field */
#define TM_TCPE_RCV_USER               8   /* User receive call */
#define TM_TCPE_REXMIT                 9   /* Retransmission timeout */
#define TM_TCPE_CLOSE                  10  /* User close */
#define TM_TCPE_ABORT                  11  /* User abort */

/*
 * TCP flags
 */

/*
 * Options/flags defined in trsocket.h and set in tcpsFlags
 */

/* Turn ON/OFF Nagle Algorithm (in trmacro.h) */
/*#define TM_TCPF_NODELAY       (tt16Bit)TCP_NODELAY *//* 0x01 */
/* 0x02 is used internally here (not set by the user) */
#define TM_TCPF_PASSIVE       (tt16Bit)0x02        /* accept socket */
/* Turn OFF push (in trmacro.h) */
/* #define TM_TCPF_NOPUSH        (tt16Bit)TCP_NOPUSH *//* 0x04 */
/* Urgent pointer points to last byte of urgent data (RFC 1122) (in trmacro.h) */
/*#define TM_TCPF_STDURG        (tt16Bit)TCP_STDURG  *//* 0x08 */
/* Use connection timer to abort the connection */
#define TM_TCPF_MAXRT         (tt16Bit)TCP_MAXRT   /* 0x10 */
/* Retransmit forever, even connection requests */
/* #define TM_TCPF_RT_FOREVER    (tt16Bit)0x020 */ /* in trmacro.h */

/*
 * Treck Extended options/flags defined in trsocket.h and set in tcpsFlags
 */

/* Turn ON/OFF Selective Acknolwlegements (in trmacro.h) */
/* #define TM_TCPF_SEL_ACK        (tt16Bit)TM_TCP_SEL_ACK */ /* 0x0040 */
/* Turn ON/OFF Big Window Scaling */
#define TM_TCPF_WND_SCALE      (tt16Bit)TM_TCP_WND_SCALE  /* 0x0080 */
/* Turn ON/OFF Time stamp (in trmacro.h) */
/* #define TM_TCPF_TS             (tt16Bit)TM_TCP_TS    *//* 0x0100 */
/* Options resettable by the remote side if not set in its SYN */
#define TM_TCPF_OPTIONS        (tt16Bit)(   TM_TCPF_SEL_ACK \
                                          | TM_TCPF_WND_SCALE \
                                          | TM_TCPF_TS)
/* Options settable by the user */
#define TM_TCPF_USER_SET \
        (tt16Bit) (   TCP_NODELAY | TCP_NOPUSH | TCP_STDURG \
                    | TM_TCP_SLOW_START \
                    | TM_TCPF_OPTIONS )

/* Turn ON/OFF Slow Start */
#define TM_TCPF_SLOW_START     (tt16Bit)TM_TCP_SLOW_START /* 0x0200 */
#define TM_TCPF_PUSH_ALL       (tt16Bit)TM_TCP_PUSH_ALL   /* 0x0400 */
/* TM_TCPF_PACKET in trmacro.h */
/*#define TM_TCPF_PACKET         (tt16Bit)TM_TCP_PACKET*/ /* 0x0400 */
/*
 *  Additional TCP flags internal to this module and set in tcpsFlags.
 */
#define TM_TCPF_SOCKETENTRYPTR TM_BUF_SOCKETENTRYPTR    /* 0x800 */
#define TM_TCPF_RECYCLE_HDR    TM_BUF_TCP_HDR_BLOCK     /* 0x1000 */
#define TM_TCPF_INDIRECT       (tt16Bit)0x2000 /* Route is indirect */
#define TM_TCPF_WAIT_SND_FIN   (tt16Bit)0x4000 /* Wait to send our FIN */
/*
 * User set the MSS: Do not use route MTU; Do not allow path MTU discovery.
 */
/* #define TM_TCPF_PMTU_DISCOVERY  (tt16Bit)0x8000 */ /* in trmacro.h */

/*
 * Additional TCP flags set in tcpsFlags2 (not enough room in tcpsFlags)
 */
/* FIN has been received (in trmacro.h) */
/* #define TM_TCPF2_RCV_FIN       (tt16Bit)0x01   */
/* try and send FIN (in trmacro.h) */
/* #define TM_TCPF2_SND_FIN       (tt16Bit)0x02 */
/* Our FIN has been sent (in trmacro.h) */
/* #define TM_TCPF2_FIN_SENT      (tt16Bit)0x04   */
/* network congested (in trmacro.h) */
/* #define TM_TCPF2_CONGESTED     (tt16Bit)0x08 */
#define TM_TCPF2_OOBDATA_IN    (tt16Bit)0x10   /* one byte OOB data copied */
/* OOB data copied to user */
/* #define TM_TCPF2_OOBDATA_OUT   (tt16Bit)0x20  */
/* Zero window probe or Urgent data (defined in trmacro.h) */
/* #define TM_TCPF2_PROBE_OOB   (tt16Bit)0x40 */
/* nobody is sending (defined in trmacro.h) */
/* #define TM_TCPF2_SEND_CLEAR    (tt16Bit)0x80 */
/* tfTcpSendPacket() need to reset retransmit timer interval (in trmacro.h) */
/* #define TM_TCPF2_REINIT_TIMER  (tt16Bit)0x100 */
/* Rexmit mode on (in trmacro.h) */
/* #define TM_TCPF2_REXMIT_MODE   (tt16Bit)0x200 */
/* Rexmit one segment at temporary sequence number (in trmacro.h) */
/* #define TM_TCPF2_REXMIT_TEMP   (tt16Bit)0x400 */
/* Send an ACK without delay (defined in trmacro.h) */
/* #define TM_TCPF2_ACK           (tt16Bit)0x800*/
/* Tell the delay timer to send an ACK (in trmacro.h) */
/* #define TM_TCPF2_DELAY_ACK     (tt16Bit)0x1000 */
/* try and send data (defined in trmacro.h) */
/*#define TM_TCPF2_SEND_DATA   (tt16Bit)0x2000 */
/* Don't fragment error (in trmacro.h) */
/* #define TM_TCPF2_DF_ERROR      (tt16Bit)0x4000 */
/* RFC3042 2 dup ACKs (in trmacro.h) */
/* #define TM_TCPF2_SINGLE_DUP    (tt16Bit)0x8000 */
/* User has set a listening recv window (defined in trmacro.h) */
/* #define TM_TCPF3_LISTEN_RECV_WINDOW  (tt16Bit)1 */
/* probing a zero window (in trmacro.h) */
/* #define TM_TCPF3_PROBE_MODE    (tt16Bit)0x0002 */

/*
 * TCP Timer array indices
 */
#ifdef TM_LINT
LINT_ZERO_MACRO(TM_TCPTM_DELAYACK)
#endif /* TM_LINT */
/* #define TM_TCPTM_DELAYACK     ( TM_TCPTM_LINGER - 6 ) *//* 0 */
/* #define TM_TCPTM_CONN         ( TM_TCPTM_LINGER - 5 ) *//* 1 */
/* #define TM_TCPTM_PROBE        ( TM_TCPTM_LINGER - 4 ) *//* 2 */
/* #define TM_TCPTM_REXMIT       ( TM_TCPTM_LINGER - 3 ) *//* 3 */
/* #define TM_TCPTM_KEEPALIVE    ( TM_TCPTM_LINGER - 2 ) *//* 4 */
#define TM_TCPTM_FINWT2       ( TM_TCPTM_LINGER - 1 ) /* 5 */

/* #ifdef TM_TCP_SACK */
/*
 * Index in tcpSackBlockPtr[2]
 * Queue of Sel Ack Blocks corresponding to recv data.
 * Queued when we store data in the reassembly queue.
 * Dequeued when data is moved to the recv queue.
 */
/* #define TM_TCP_SACK_SEND_INDEX  0 */
/*
 * Index in tcpSackBlockPtr[2]
 * Queue of Sel Ack Blocks corresponding to sent data.
 * Queued when we recv SEL ACKs from the remote.
 * Dequeued when data is acked.
 */
/* #define TM_TCP_SACK_RECV_INDEX 1 */
/* #endif *//* TM_TCP_SACK */

/*
 * Tcp state vector Timers
 */
/* Delay ACK timer */
/* #define tcpsDelayAckTmPtr   tcpsTmPtr[TM_TCPTM_DELAYACK] */
#define tcpsDelayAckTm      tcpsTm[TM_TCPTM_DELAYACK]
/* Connection timer */
#define tcpsConnTmPtr       tcpsTmPtr[TM_TCPTM_CONN]
#define tcpsConnTm          tcpsTm[TM_TCPTM_CONN]
/* Keep alive timer */
/* #define tcpsKeepAliveTmPtr  tcpsTmPtr[TM_TCPTM_KEEPALIVE] */
#define tcpsKeepAliveTm     tcpsTm[TM_TCPTM_KEEPALIVE]
/* retransmit timer */
/* #define tcpsReXmitTmPtr     tcpsTmPtr[TM_TCPTM_REXMIT] */
#define tcpsReXmitTm        tcpsTm[TM_TCPTM_REXMIT]
/* Zero window probe timer (in trmacro.h) */
/* #define tcpsWndProbeTmPtr   tcpsTmPtr[TM_TCPTM_PROBE] */
#define tcpsWndProbeTm      tcpsTm[TM_TCPTM_PROBE]
/* FIN Wait 2 Timer */
#define tcpsFinWt2TmPtr     tcpsTmPtr[TM_TCPTM_FINWT2]
#define tcpsFinWt2Tm        tcpsTm[TM_TCPTM_FINWT2]

/*
 * socket close linger timer
 */
#ifdef TM_PEND_POST_NEEDED
#define tcpsLingerTmPtr     tcpsTmPtr[TM_TCPTM_LINGER]
#define tcpsLingerTm        tcpsTm[TM_TCPTM_LINGER]
#endif /* TM_PEND_POST_NEEDED */


/*
 * State function Indices
 */

/* State machine no op index (no action) */
#define TM_MATRIX_NOOP           TM_TCPM_LAST_INDEX

/* State machine error indices (action is to set the corresponding error) */
#define TM_TCPM_EADDRINUSE       tm_tcpm_err(TM_EADDRINUSE)
#define TM_TCPM_ENOTCONN         tm_tcpm_err(TM_ENOTCONN)
#define TM_TCPM_ESHUTDOWN        tm_tcpm_err(TM_ESHUTDOWN)
#define TM_TCPM_EPERM            tm_tcpm_err(TM_EPERM)
#define TM_TCPM_EALREADY         tm_tcpm_err(TM_EALREADY)
#define TM_TCPM_EISCONN          tm_tcpm_err(TM_EISCONN)
#define TM_TCPM_EINPROGRESS      tm_tcpm_err(TM_EINPROGRESS)
#define TM_TCPM_EOPNOTSUPP       tm_tcpm_err(TM_EOPNOTSUPP)

#ifdef TM_ERROR_CHECKING

/* Re-initialize an automatic timer with a new interval of call back */
/* #define tm_tcp_timer_new_time(timerPtr, newTime)        \*/
/*    tfTcpTimerNewTime(timerPtr, newTime); */

/* Suspend an automatic timer (in trmacro.h) */
/* #define tm_tcp_timer_suspend(timerPtr)                  \ */
/*    tfTcpTimerSuspend(timerPtr) */

#ifdef TM_USE_TCP_REXMIT_CONTROL
/* Unsuspend an automatic timer */
#define tm_tcp_timer_resume(timerPtr)                   \
    tfTcpTimerResume(timerPtr)
#endif /* TM_USE_TCP_REXMIT_CONTROL */

#else /* TM_ERROR_CHECKING */

/* Re-initialize an automatic timer with a new interval of call back */
/* #define tm_tcp_timer_new_time(timerPtr, newTime)         \ */
/*    tm_timer_new_time(timerPtr, newTime); */

/* Suspend an automatic timer (in trmacro.h) */
/* #define tm_tcp_timer_suspend(timerPtr)                  \ */
/*    tm_timer_suspend(timerPtr) */

#ifdef TM_USE_TCP_REXMIT_CONTROL
#define tm_tcp_timer_resume(timerPtr)                   \
    tm_timer_resume(timerPtr)
#endif /* TM_USE_TCP_REXMIT_CONTROL */

#endif /* TM_ERROR_CHECKING */

/*
 * Socket error to state machine error index. Use TM_TCPM_LAST_INDEX as base
 * of error. Conversion to make it fit in an 8 bit cell.
 */
#define tm_tcpm_err(errorCode)  (tt8Bit)((int)((int)(errorCode-TM_ERR_BASE) \
                                                        + TM_TCPM_LAST_INDEX))
/* State machine error index to socket error */
#define tm_tcpm_sock_err(index)  ((((int)(index))-TM_TCPM_LAST_INDEX) \
                                                + TM_ERR_BASE)


/* Macro to call a state function */
#define tm_call_state_function(stateEntryPtr, tcpVectPtr, packetPtr, nState) \
    (*(stateEntryPtr->tstaFunctPtr))(tcpVectPtr, packetPtr, nState)

/* Macro to declare a state function */
#define tm_state_function(tcpStateFunction, tcpVectPtr, packetPtr, nState) \
    tcpStateFunction(ttTcpVectPtr tcpVectPtr, \
                     ttPacketPtr  packetPtr, \
                     tt8Bit       nState)

/*
 * From TCP header length in bytes to TCP header lower 4 bits of data offset
 * in 32-bit words. (header length divided by 4 and moved to lower 4 bits of
 * 8-bit tcpDataOffset), so shift left 2 (left 4 - 2 right))
 */
/* #define tm_tcp_set_data_offset(hdrLen) ((tt8Bit)(((tt8Bit)hdrLen) << 2)) */

/* tm_tcp_iw() in trmacro.h */
/* RFC 2001/RFC 2581 Initial window: IW (intial value of congestion window) */
/* #ifdef TM_TCP_RFC2581 */
/* RFC 2581: Initial congestion window set at TWO MSS. [RFC2581]R3.1:1 */
/*#define tm_tcp_iw(tcpVectPtr)   (2 * tcpVectPtr->tcpsEffSndMss) */
/* #else *//* !TM_TCP_RFC2581 */
/* RFC 2001: Initial congestion window set at one MSS */
/*#define tm_tcp_iw(tcpVectPtr)   tcpVectPtr->tcpsEffSndMss */
/*#endif *//* TM_TCP_RFC2581 */

/* TCP loss window (to be used for slow start after a retransmission time out)*/
#define tm_tcp_lw(tcpVectPtr)   tcpVectPtr->tcpsEffSndMss

/*
 * (a) Determining that an acknowledgment refers to some sequence
 *     number sent but not yet acknowledged:
 * A new acknowledgment (called an "acceptable ack"), is one for which
 * the following inequality holds: sndUna < segAck <= SndNxt.
 */
#define tm_tcp_acceptable_ack(sndUna, segAck, SndNxt) \
        (tm_sqn_lt(sndUna, segAck) && tm_sqn_leq(segAck, SndNxt))


/*
 * If SND.UNA =< SEG.ACK =< SND.NXT then the ACK is valid (although
 * it does not ack any new sent data)
 */
#define tm_tcp_valid_ack(sndUna, segAck, sndNxt) \
        (tm_sqn_leq(sndUna, segAck) && tm_sqn_leq(segAck, sndNxt))

/*
 * (b) Determining that all sequence numbers occupied by a segment
 *     have been acknowledged (e.g., to remove the segment from a
 *     retransmission queue).
 * A segment on the send queue is fully acknowledged if the sum
 * of its sequence number and link length is less or equal than the
 * acknowledgment value in the incoming segment.
 */
#define tm_tcp_packet_fully_acked(sndUna, pktLinkLength, segAck) \
            tm_sqn_leq(sndUna+pktLinkLength, segAck)

/*
 * Round Trip Time(RTT) scale (8) => shift 3 (used to compute a scaled
 * smoothed round trip time).
 */
#define TM_TCP_RTT_SCALE_SHIFT  3
/*
 * RTT variance scale (4) => shift 2 (used to compute a scaled smoothed
 * round trip variance).
 */
#define TM_TCP_RTTVAR_SCALE_SHIFT  2

/*
 * Retransmission timeout is Scaled Smooth round trip time / 8 + Scaled
 * smoothed mean deviation estimator
 */
#define tm_rto(srtt, rttvar) \
     (tt32Bit)(((tt32Bit)(srtt) >> TM_TCP_RTT_SCALE_SHIFT) + (tt32Bit)rttvar)

/*
 * Exponential back off array: tlExpnBackOff[i] == 1 >> i
 */

static const int  TM_CONST_QLF tlExpnBackOff[] = { 1, 2, 4, 8, 16, 32};

#define tm_expn_backoff(retryFactor) \
{ \
    if (retryFactor < 6) \
    { \
        retryFactor = tlExpnBackOff[retryFactor]; \
    } \
    else \
    { \
        retryFactor = 64; \
    } \
}

/*
 * Initialize all receive sequence numbers with initial receive sequence
 * number (+1) received from the peer. Rcv.Up should point to Rcv.Nxt-1
 */
#define tm_tcp_init_rcv_sqn(tcpVectPtr) \
{ \
    (tcpVectPtr)->tcpsRcvUp = (tcpVectPtr)->tcpsIrs; \
    (tcpVectPtr)->tcpsRcvNxt = (tcpVectPtr)->tcpsRcvAdv = \
                                                 (tcpVectPtr)->tcpsIrs + 1; \
}

/*
 * Initialize all send sequence numbers with our own initial sequence
 * number used to send our SYN.
 */
#ifdef TM_TCP_FACK
#define tm_tcp_init_send_sqn(tcpVectPtr) \
{ \
    (tcpVectPtr)->tcpsSndUp = (tcpVectPtr)->tcpsIss - 1; \
    (tcpVectPtr)->tcpsSndUna = (tcpVectPtr)->tcpsIss; \
    (tcpVectPtr)->tcpsSndFack = (tcpVectPtr)->tcpsIss; \
    (tcpVectPtr)->tcpsSndNxt = (tcpVectPtr)->tcpsIss; \
    (tcpVectPtr)->tcpsMaxSndNxt = (tcpVectPtr)->tcpsIss; \
}
#else /* !TM_TCP_FACK */
#define tm_tcp_init_send_sqn(tcpVectPtr) \
{ \
    (tcpVectPtr)->tcpsSndUp = (tcpVectPtr)->tcpsIss - 1; \
    (tcpVectPtr)->tcpsSndUna = (tcpVectPtr)->tcpsIss; \
    (tcpVectPtr)->tcpsSndNxt = (tcpVectPtr)->tcpsIss; \
    (tcpVectPtr)->tcpsMaxSndNxt = (tcpVectPtr)->tcpsIss; \
}
#endif /* !TM_TCP_FACK */

/* Round up 'value' to a multiple of 'unit' */
#define tm_roundup(value, unit)    \
                    ( ( ((unit) + (value) - 1) / (unit) ) * (unit) )

/*
 * Given a Tcp state vector and a timer index, remove corresponding
 * Tcp state vector/socket entry timer
 */
#define tm_tcp_timer_remove(tcpVectPtr, timerIndex) \
{ \
    if (tcpVectPtr->tcpsTmPtr[timerIndex] != TM_TMR_NULL_PTR) \
    { \
        tm_timer_remove(tcpVectPtr->tcpsTmPtr[timerIndex]); \
        tcpVectPtr->tcpsTmPtr[timerIndex] = TM_TMR_NULL_PTR; \
    } \
}

/* Suspend max connection timeout timer (in trmacro.h) */
/* #define tm_tcp_maxrt_suspend(tcpVectPtr) \ */
/* { \ */
/*    if (tcpVectPtr->tcpsTmPtr[TM_TCPTM_CONN] != TM_TMR_NULL_PTR) \ */
/*    { \ */
/*        tm_timer_suspend(tcpVectPtr->tcpsTmPtr[TM_TCPTM_CONN]); \ */
/*    } \ */
/*} */



#ifndef TM_DSP
#ifdef TM_LITTLE_ENDIAN

#define tm_endian_short_copy(srcPtr, dest, srcOffset)                  \
{                                                                      \
    ((tt8BitPtr)(&dest))[1] = (srcPtr)[srcOffset];                     \
    ((tt8BitPtr)(&dest))[0] = (srcPtr)[srcOffset+1];                   \
}

#else /* !TM_LITTLE_ENDIAN */

#define tm_endian_short_copy(srcPtr, dest, srcOffset)                  \
{                                                                      \
    ((tt8BitPtr)(&dest))[0] = (srcPtr)[srcOffset];                     \
    ((tt8BitPtr)(&dest))[1] = (srcPtr)[srcOffset+1];                   \
}

#endif /* TM_LITTLE_ENDIAN */
#endif /* TM_DSP */



#define tm_soc_recv_q_bytes(socketPtr) \
        (socketPtr->socRecvQueueBytes)

#define tm_soc_send_q_bytes(socketPtr) \
        (socketPtr->socSendQueueBytes)


#ifdef TM_USE_SSL
#define tm_tcp_soc_recv_q_bytes(socketPtr, tcpVectPtr) \
   (   tm_soc_recv_q_bytes(socketPtr)                  \
     + tcpVectPtr->tcpsSslRecvQBytes )
#define tm_tcp_soc_send_q_bytes(socketPtr, tcpVectPtr) \
   (   tm_soc_send_q_bytes(socketPtr)                  \
     + tcpVectPtr->tcpsSslSendQBytes )
#else /* !TM_USE_SSL */
#define tm_tcp_soc_recv_q_bytes(socketPtr, tcpVectPtr) \
   tm_soc_recv_q_bytes(socketPtr)
#define tm_tcp_soc_send_q_bytes(socketPtr, tcpVectPtr) \
   tm_soc_send_q_bytes(socketPtr)
#endif /* !TM_USE_SSL */

/*
 * Local types
 */

/*
 * Network headers
 */

/* TCP Maximum Segment Size option */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#ifndef TM_32BIT_DSP_BIG_ENDIAN
/* Default is Little Endian for 32BIT_DSP for backward compatibility */
typedef struct tsTcpMssOpt
{
    unsigned int tcpoMSS     : 16;
    unsigned int tcpoMLength : 8;
    unsigned int tcpoMKind   : 8;
} ttTcpMssOpt;
#else /* TM_32BIT_DSP_BIG_ENDIAN */
typedef struct tsTcpMssOpt
{
    unsigned int tcpoMKind   : 8;
    unsigned int tcpoMLength : 8;
    unsigned int tcpoMSS     : 16;
} ttTcpMssOpt;
#endif /* TM_32BIT_DSP_BIG_ENDIAN */
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
typedef struct tsTcpMssOpt
{
    unsigned int tcpoMKind   : 8;
    unsigned int tcpoMLength : 8;
    tt16Bit      tcpoMSS;
} ttTcpMssOpt;
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
typedef struct tsTcpMssOpt
{
    tt8Bit   tcpoMKind;
    tt8Bit   tcpoMLength;
    tt16Bit  tcpoMSS;
} ttTcpMssOpt;
#endif /* !TM_DSP */

/* ALigned TCP Time stamp option */
/* typedef struct tsTcpAlgnTsOpt */
/* { */
/*    tt32Bit  tcpoAFirstWord; *//* NOOP, NOOP, Kind, Length */
/*    tt32Bit  tcpoAVal;       *//* time stamp value */
/*    tt32Bit  tcpoAEchoRpl;   *//* time stamp echo reply */
/*} ttTcpAlgnTsOpt; */

/* Non aligned TCP Time stamp option */
#ifdef TM_DSP
/* C3X cannot really be unaligned */
typedef struct tsTcpTsOpt
{
    unsigned int tcpoTKind   : 8;
    unsigned int tcpoTLength : 8;
    unsigned int tcpoTNoop   : 8;
    tt32Bit  tcpoTVal;       /* time stamp value */
    tt32Bit  tcpoTEchoRpl;   /* time stamp echo reply */
} ttTcpTsOpt;
#else /* !TM_DSP */
#ifdef TM_LINT
LINT_UNREF_MEMBER_BEGIN
#endif /* TM_LINT */
typedef struct tsTcpTsOpt
{
    tt8Bit   tcpoTKind;
    tt8Bit   tcpoTLength;
    tt8Bit   tcpoTVal[sizeof(tt32Bit)];       /* time stamp value */
    tt8Bit   tcpoTEchoRpl[sizeof(tt32Bit)];   /* time stamp echo reply */
} ttTcpTsOpt;
#ifdef TM_LINT
LINT_UNREF_MEMBER_END
#endif /* TM_LINT */
#endif /* TM_DSP */

/* TCP window scale option */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#ifndef TM_32BIT_DSP_BIG_ENDIAN
/* Default is Little Endian for 32BIT_DSP for backward compatibility */
typedef struct tsTcpWndScOpt
{
    unsigned int tcpoWndSc   : 8;
    unsigned int tcpoWLength : 8;  /* 3 */
    unsigned int tcpoWKind   : 8;  /* 3 */
    unsigned int tcpoWNoop   : 8;
} ttTcpWndScOpt;
#else /* TM_32BIT_DSP_BIG_ENDIAN */
typedef struct tsTcpWndScOpt
{
    unsigned int tcpoWNoop   : 8;
    unsigned int tcpoWKind   : 8;  /* 3 */
    unsigned int tcpoWLength : 8;  /* 3 */
    unsigned int tcpoWndSc   : 8;

} ttTcpWndScOpt;
#endif /* TM_32BIT_DSP_BIG_ENDIAN */
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
typedef struct tsTcpWndScOpt
{
    unsigned int tcpoWNoop   : 8;
    unsigned int tcpoWKind   : 8;  /* 3 */
    unsigned int tcpoWLength : 8;  /* 3 */
    unsigned int tcpoWndSc   : 8;

} ttTcpWndScOpt;
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
typedef struct tsTcpWndScOpt
{
    tt8Bit   tcpoWNoop;
    tt8Bit   tcpoWKind;    /* 3 */
    tt8Bit   tcpoWLength;  /* 3 */
    tt8Bit   tcpoWndSc;
} ttTcpWndScOpt;
#endif /* !TM_DSP */

/* TCP SACK permitted option */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
#ifndef TM_32BIT_DSP_BIG_ENDIAN
/* Default is Little Endian for 32BIT_DSP for backward compatibility */
typedef struct tsTcpSackPermitOpt
{
    unsigned int tcpoSPLength : 8;  /* 2 */
    unsigned int tcpoSPKind   : 8;  /* 4 */
    unsigned int tcpoSPNoop1  : 8;  /* to make the structure word aligned */
    unsigned int tcpoSPNoop2  : 8;  /* to make the structure word aligned */
} ttTcpSackPermitOpt;
#else /* TM_32BIT_DSP_BIG_ENDIAN */
typedef struct tsTcpSackPermitOpt
{
    unsigned int tcpoSPNoop1  : 8;  /* to make the structure word aligned */
    unsigned int tcpoSPNoop2  : 8;  /* to make the structure word aligned */
    unsigned int tcpoSPKind   : 8;  /* 4 */
    unsigned int tcpoSPLength : 8;  /* 2 */
} ttTcpSackPermitOpt;
#endif /* TM_32BIT_DSP_BIG_ENDIAN */
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
typedef struct tsTcpSackPermitOpt
{
    unsigned int tcpoSPNoop1  : 8;  /* to make the structure word aligned */
    unsigned int tcpoSPNoop2  : 8;  /* to make the structure word aligned */
    unsigned int tcpoSPKind   : 8;  /* 4 */
    unsigned int tcpoSPLength : 8;  /* 2 */
} ttTcpSackPermitOpt;
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
typedef struct tsTcpSackPermitOpt
{
    tt16Bit  tcpoSPNoop2;  /* 2 NOOP to make the structure word aligned */
    tt8Bit   tcpoSPKind;   /* 4 */
    tt8Bit   tcpoSPLength; /* 2 */
} ttTcpSackPermitOpt;
#endif /* !TM_DSP */
/* END of network headers */

/* Pointer type to network headers */
/* typedef ttTcpAlgnTsOpt     TM_FAR * ttTcpAlgnTsOptPtr; */
typedef ttTcpWndScOpt      TM_FAR * ttTcpWndScOptPtr;
typedef ttTcpMssOpt        TM_FAR * ttTcpMssOptPtr;
typedef ttTcpSackPermitOpt TM_FAR * ttTcpSackPermitOptPtr;

/* Transition matrix Entry for TCP event/state processing */
typedef struct tsTransitionMatrixEntry
{
    tt8Bit mtxTransition;
    tt8Bit mtxFunctionIndex;
} ttTransitionMatrixEntry;

/* Pointer type to a transition matrix */
typedef ttTransitionMatrixEntry TM_CONST_QLF * ttTransitionMatrixEntryPtr;

/* state function type */
typedef int (*ttTcpStateFunctPtr)(ttTcpVectPtr   tcpVectPtr,
                                  ttPacketPtr    packetPtr,
                                  tt8Bit         newState);

/* State function table entry */
#ifdef TM_LINT
LINT_UNREF_MEMBER_BEGIN
#endif /* TM_LINT */
typedef struct tsTcpStateFunctEntry
{
    tt8Bit             tstaIndex;
    tt8Bit             tstaFiller[3];
    ttTcpStateFunctPtr tstaFunctPtr;
} ttTcpStateFunctEntry;
#ifdef TM_LINT
LINT_UNREF_MEMBER_END
#endif /* TM_LINT */

/* pointer type to a state function table */
typedef ttTcpStateFunctEntry TM_CONST_QLF * ttTcpStateFunctEntryPtr;

#ifdef TM_USE_STRONG_ESL
/* Structure for tuple/device comparison */
struct tsTwlTupleDev
{
    ttSockTuplePtr      twlTuplePtr;
    ttDeviceEntryPtr    twlDevPtr;
};

typedef struct tsTwlTupleDev TM_FAR * ttTwlTupleDevPtr;
#endif /* TM_USE_STRONG_ESL */


/*
 * Local functions
 */

/* Update round trip time */
static void tfTcpUpdateRtt( ttTcpVectPtr tcpVectPtr,
                            tt32Bit      newRtt,
                            tt8Bit       computeRtoFlag );
/* Initialize MSS and Windows */
/* int tfTcpSetMssWnd(ttTcpVectPtr tcpVectPtr); */
/* Initialize/Change MSS */
static int tfTcpSetMss(ttTcpVectPtr tcpVectPtr);

/*
 * Get a TCP header from the connection TCP header recycle list. If
 * the connection TCP header recycle list is empty get a new TCP header
 * from memory.
 */
TM_PROTO_EXTERN ttPacketPtr tfTcpGetTcpHeader(ttTcpVectPtr tcpVectPtr);
/* Generate SYN options after the TCP header */
static tt16Bit tfTcpGenSynOptions(ttTcpVectPtr tcpVectPtr,
                                  ttPacketPtr packetPtr);
/* Add Time Stamp option */
static void tfTcpGenTsOption(ttTcpVectPtr tcpVectPtr,
                             tt8BitPtr    optPtr);
/* Send SYN, SYN_ACK, RST, and Keep alive probes */
static int tfTcpCreateSendPacket(ttTcpVectPtr tcpVectPtr,
                                 ttPacketPtr  packetPtr,
                                 ttSqn        seq,
                                 ttSqn        ack,
                                 tt8Bit       ctlBits);
/*
 * Process received SYN options (from the peer). Initialize receive sequence
 * numbers. Also check routing entry and set MSS from peer.
 */
static int tfTcpRcvSynOptions(ttPacketPtr packetPtr,
                              tt16Bit tcpsFlags,
                              ttTcpVectPtr tcpVectPtr);
/* Get an initial sequence number (when we send a SYN) */
static void tfTcpGetIss (ttTcpVectPtr tcpVectPtr);
/* Compute the interval between successive zero window probes */
static tt8Bit tfTcpProbeWndInterval (ttTcpVectPtr tcpVectPtr);
/* Reassemble received packets in the reassemble queue */
static tt8Bit tfTcpReassemblePacket(ttTcpVectPtr tcpVectPtr,
                                    ttPacketPtr  packetPtr);
/* Process incoming Urgent Data (from the peer) */
static tt8Bit tfTcpRcvUrgentData(ttTcpVectPtr tcpVectPtr,
                                 ttPacketPtr  packetPtr);
/* Queue Out of byte data to the socket */
static int tfSocketOobIncomingPacket(ttTcpVectPtr tcpVectPtr,
                                     ttPacketPtr  packetPtr);
/*
 * Head trim a packet, preserving/updating sequence number, and urgent offset
 */
static void tfTcpPacketHeadTrim (ttPacketPtr packetPtr,
                                 ttPktLen    overlap);

/*
 * if prev, and current packet not fragmented, check if we can append to
 * the prev packet. Update urgent pointer if needed.
 */
static tt8Bit tfTcpAppendPacket(ttPacketPtr packetPtr,
                                ttPacketPtr prevPacketPtr);

#ifdef TM_ERROR_CHECKING
#ifdef TM_USE_TCP_REXMIT_CONTROL
static void tfTcpTimerResume (ttTimerPtr timerPtr);
#endif /* TM_USE_TCP_REXMIT_CONTROL */
#endif /* TM_ERROR_CHECKING */
/* Add the MaxRt timer */
static void tfTcpAddMaxRtTmr (ttTcpVectPtr tcpVectPtr);
/* Delay ACK timer */
static void tfTcpTmDelayAck (ttVoidPtr      timerBlockPtr,
                             ttGenericUnion userParm1,
                             ttGenericUnion userParm2);
/* Connection time out timer */
TM_NEARCALL static void tfTcpTmConnOut (ttVoidPtr      timerBlockPtr,
                            ttGenericUnion userParm1,
                            ttGenericUnion userParm2);
/* Zero window probe interval timer */
static void tfTcpTmProbeWnd (ttVoidPtr      timerBlockPtr,
                             ttGenericUnion userParm1,
                             ttGenericUnion userParm2);
/* Retransmission timer */
TM_NEARCALL static void tfTcpTmReXmit (ttVoidPtr      timerBlockPtr,
                           ttGenericUnion userParm1,
                           ttGenericUnion userParm2);
/* Keep alive timer. Also keep track of idle time on the connection */
static void tfTcpTmKeepAlive (ttVoidPtr      timerBlockPtr,
                              ttGenericUnion userParm1,
                              ttGenericUnion userParm2);
/* Fin Wait 2 state time out timer */
static void tfTcpTmFinWt2Out (ttVoidPtr      timerBlockPtr,
                              ttGenericUnion userParm1,
                              ttGenericUnion userParm2);
/* 2MSL time out timer */
static void tfTcpTm2MslOut (ttVoidPtr      timerBlockPtr,
                            ttGenericUnion userParm1,
                            ttGenericUnion userParm2);

/* Socket linger time out timer */
#ifdef TM_PEND_POST_NEEDED
static void tfTcpTmLinger (ttVoidPtr      timerBlockPtr,
                           ttGenericUnion userParm1,
                           ttGenericUnion userParm2);
#endif /* TM_PEND_POST_NEEDED */
/* Free all timers, between minIndex, and maxIndex */
static void tfTcpFreeTimers(ttTcpVectPtr tcpVectPtr, tt8Bit minIndex,
                                                     tt8Bit maxIndex);

/* Clean up function called by the timer execute before freeing a timer */
static void tfTcpTmrCleanup(ttVoidPtr      timerBlockPtr,
                            tt8Bit         flags);

/*
 * Initialize connection send variables, and connection time out
 * and retransmission timers
 */
static void tfTcpInitConTmr(ttTcpVectPtr tcpVectPtr);

/* Initialize socket entry routing cache */
static int tfTcpSetRtc(ttTcpVectPtr tcpVectPtr);

/* Complete a user initiated close */
static int tfTcpCompleteClose(ttTcpVectPtr tcpVectPtr);

/*
 * Remove entry from the half connected queue, send a reset, and delete it.
 */
static void tfTcpRemoveResetConReq(ttTcpVectPtr conReqTcpVectPtr, int index);

/*
 * Insert a TCP vector in the listening TCP vector connection queue, as given
 * by the queue index.
 */
static void tfTcpConQueueInsert(ttTcpVectPtr listenTcpVectPtr,
                                ttTcpVectPtr tcpVectPtr,
                                int          queueIndex);

/*
 * Remove a TCP vector from the listening TCP vector connection queue, as given
 * by the queue index.
 */
static tt8Bit tfTcpConQueueRemove(ttTcpVectPtr tcpVectPtr, int queueIndex);

/* Post FIN transmission: make sure FIN will be retransmitted */
static void tfTcpPostSendFin (ttTcpVectPtr tcpVectPtr);

#ifdef TM_TCP_SACK


static ttSackEntryPtr tfTcpSackRemoveBlock( ttTcpVectPtr   tcpVectPtr,
                                            ttSackEntryPtr sackBlockPtr,
                                            int            sackBlockIndex );

/*
 * Free all SEL ACK blocks from TCP state vector queue of SEL ACK blocks,
 * for a given queue (either queue of SEL ACK blocks taht we send, or
 * SEL ACK blocks that we receive).
 */
static void tfTcpSackFreeQueue(ttTcpVectPtr tcpVectPtr, int sackIndex);

#ifdef TM_TCP_FACK
/*
 * If new data is acknowledged:
 *   Update wintrim (adjustment to congestion window during recovery).
 *   Update Forward-most data (highest sqn + 1) held by the receiver.
 * If retransmitted data is acknowledged:
 *   adjust retransmitted bytes during recovery (retransData != 0).
 */
static void tfTcpSackUpdateFack(ttTcpVectPtr tcpVectPtr,
                                ttSqn        leftEdgeSeqNo,
                                ttSqn        rightEdgeSeqNo,
                                tt32Bit      ackedBytes);
#endif /* TM_TCP_FACK */
#endif /* TM_TCP_SACK */

#ifdef TM_DEV_SEND_OFFLOAD
static void tfTcpOffloadInit(ttTcpVectPtr tcpVectPtr);
#endif /* TM_DEV_SEND_OFFLOAD */

/* Check if we need to update the send window variables */
TM_PROTO_EXTERN void tfTcpUpdateSendWindow(ttTcpVectPtr tcpVectPtr,
                                           tt32Bit      seqNo,
                                           tt32Bit      segAck,
                                           tt32Bit      unscaledWnd);

#ifdef TM_USE_SSL
static int tfTcpSslClose(ttTcpVectPtr tcpVectPtr, int flags);
#endif /* TM_USE_SSL */

/* Return a pointer to the ttList where the matching time wait vector is */
static ttListPtr tfTcpTmWtGetListPtr (ttSockTuplePtr sockTuplePtr);
/* Grab ISS from Time Wait and close the TCP time wait vector */
static ttSqn tfTcpTmWtGetIssClose(ttTcpTmWtVectPtr tcpTmWtVectPtr);
/* TCP Time Wait table Hash list walk CB function */
static int tfTcpTmWtTupleEqualCB(ttNodePtr nodePtr, ttGenericUnion genParam);
/* Time Wait TCP Vector Timer Clean up function */
static void tfTcpTmWtTmrCleanup(ttVoidPtr       timerBlockPtr,
                                tt8Bit          flags);

/*
 * State functions (actions)
 */

/*
 * Close a TCP state vector (action taken when TCP state vector transition to
 * closed state)
 */
static int tm_state_function (tfTcpVectClose, tcpVectPtr, packetPtr,
                              newState);
/* Send a FIN to the peer */
static int tm_state_function (tfTcpSendFin, tcpVectPtr, packetPtr, newState);
/* Re-transmit a FIN */
static int tm_state_function (tfTcpReSendFin, tcpVectPtr, packetPtr,
                              newState);
/* Send Data */
static int tm_state_function (tfTcpSendData, tcpVectPtr, packetPtr, newState);
/* Send a SYN */
static int tm_state_function (tfTcpSendSyn, tcpVectPtr, packetPtr, newState);
/* Re-transmit a SYN */
static int tm_state_function (tfTcpReSendSyn, tcpVectPtr, packetPtr,
                              newState);
/* Receive a SYN from the peer */
static int tm_state_function (tfTcpRcvSyn, tcpVectPtr, packetPtr, newState);
/* Insert SYN data (data send along with a SYN) in reassemble queue */
static int tm_state_function (tfTcpSynData, tcpVectPtr, packetPtr, newState);
/* Send a SYN-ACK */
static int tm_state_function (tfTcpSendSynAck, tcpVectPtr, packetPtr,
                              newState);
/* Free a packet */
static int tm_state_function (tfTcpFreePacket, tcpVectPtr, packetPtr,
                              newState);
/* Delete the re-transmission timer */
static int tm_state_function (tfTcpDelReXmit, tcpVectPtr, packetPtr,
                              newState);
/* Invalid segment from the peer */
static int tm_state_function (tfTcpSegInvalid, tcpVectPtr, packetPtr,
                              newState);
/* Connection request from the peer */
static int tm_state_function (tfTcpConReq, listenTcpVectPtr, packetPtr,
                              newState);
/* Check that we received a valid ACK of our ISS (SYN) from the peer */
static int tm_state_function (tfTcpCheckValidIssAck, tcpVectPtr, packetPtr,
                              newState);
/* Transition to establish state */
static int tm_state_function (tfTcpEstablish, tcpVectPtr, packetPtr,
                              newState);
/* Transition to listen state */
static int tm_state_function (tfTcpListen, tcpVectPtr, packetPtr, newState);
/* Connection refused by peer, plus free packet */
static int tm_state_function (tfTcpNotifyConnResetFree, tcpVectPtr,
                              packetPtr, newState);
/* Connection reset by peer, notify the user, free packet */
static int tm_state_function (tfTcpNotifyResetFree, tcpVectPtr, packetPtr,
                              newState);
/* Connection reset by us, because of a bad segment, notify the user */
static int tm_state_function (tfTcpNotifyReset, tcpVectPtr, packetPtr,
                              newState);
/* Send a reset to the user using sequence number 0 and acking peer's data */
static int tm_state_function (tfTcpSendRstSeq0,  tcpVectPtr,  packetPtr,
                              newState);
/* Send a reset to the peer (2 other different cases) */
static int tm_state_function (tfTcpSendRst, tcpVectPtr, packetPtr, newState);
/* Receive an ACK from the peer */
static int tm_state_function (tfTcpRcvAck, tcpVectPtr, packetPtr, newState);
/* Receive an ACK from the peer and complete user initiated close */
static int tm_state_function (tfTcpRcvAckComplt, tcpVectPtr, packetPtr,
                              newState);
/* Check that ACK is for the FIN we had sent */
static int tm_state_function (tfTcpCheckAckFin, tcpVectPtr, packetPtr,
                              newState);
/* Process a FIN received from the peer */
static int tm_state_function (tfTcpRcvFin, tcpVectPtr, packetPtr,
                              newState);
/* Start 2MSL time out timer */
static int tm_state_function (tfTcpStart2MslTmr, tcpVectPtr, packetPtr,
                              newState);
/*
 *    FROM RFC 793:
 *
 *   Current Segment Variables
 *
 *      SEG.SEQ - segment sequence number
 *      SEG.ACK - segment acknowledgment number
 *      SEG.LEN - segment length
 *      SEG.WND - segment window
 *      SEG.UP  - segment urgent pointer
 *
 *  A connection progresses through a series of states during its
 *  lifetime.  The states are:  LISTEN, SYN-SENT, SYN-RECEIVED,
 *  ESTABLISHED, FIN-WAIT-1, FIN-WAIT-2, CLOSE-WAIT, CLOSING, LAST-ACK,
 *  TIME-WAIT, and the fictional state CLOSED.  CLOSED is fictional
 *  because it represents the state when there is no TCB, and therefore,
 *  no connection.  Briefly the meanings of the states are:
 *
 *    LISTEN - represents waiting for a connection request from any remote
 *    TCP and port.
 *
 *    SYN-SENT - represents waiting for a matching connection request
 *    after having sent a connection request.
 *
 *    SYN-RECEIVED - represents waiting for a confirming connection
 *    request acknowledgment after having both received and sent a
 *    connection request.
 *
 *    ESTABLISHED - represents an open connection, data received can be
 *    delivered to the user.  The normal state for the data transfer phase
 *    of the connection.
 *
 *    FIN-WAIT-1 - represents waiting for a connection termination request
 *    from the remote TCP, or an acknowledgment of the connection
 *    termination request previously sent.
 *
 *    FIN-WAIT-2 - represents waiting for a connection termination request
 *    from the remote TCP.
 *
 *    CLOSE-WAIT - represents waiting for a connection termination request
 *    from the local user.
 *
 *    CLOSING - represents waiting for a connection termination request
 *    acknowledgment from the remote TCP.
 *
 *    LAST-ACK - represents waiting for an acknowledgment of the
 *    connection termination request previously sent to the remote TCP
 *    (which includes an acknowledgment of its connection termination
 *    request).
 *
 *    TIME-WAIT - represents waiting for enough time to pass to be sure
 *    the remote TCP received the acknowledgment of its connection
 *    termination request.
 *
 *    CLOSED - represents no connection state at all.
 *
 *  A TCP connection progresses from one state to another in response to
 *  events.  The events are the user calls, OPEN, SEND, RECEIVE, CLOSE,
 *  ABORT, and STATUS; the incoming segments, particularly those
 *  containing the SYN, ACK, RST and FIN flags; and timeouts.
 *
 *  The state diagram in figure 6 illustrates only state changes, together
 *  with the causing events and resulting actions, but addresses neither
 *  error conditions nor actions which are not connected with state
 *  changes.  In a later section, more detail is offered with respect to
 *  the reaction of the TCP to events.
 *
 * END of from RFC 793.
 *
 * Time stamps description FROM RFC 1323:
 *   (1)  The connection state is augmented with two 32-bit slots:
 *        TS.Recent holds a timestamp to be echoed in TSecr whenever a
 *        segment is sent, and Last.ACK.sent holds the ACK field from
 *        the last segment sent.  Last.ACK.sent will equal RCV.NXT
 *        except when ACKs have been delayed.
 *   (2)  If Last.ACK.sent falls within the range of sequence numbers
 *        of an incoming segment:
 *
 *           SEG.SEQ <= Last.ACK.sent < SEG.SEQ + SEG.LEN
 *
 *        then the TSval from the segment is copied to TS.Recent;
 *        otherwise, the TSval is ignored.
 *
 *   (3)  When a TSopt is sent, its TSecr field is set to the current
 *        TS.Recent value.
 *
 * END of Time Stamps description from RFC 1323.
 */


/*
 * Initialized local variables
 */

/*
 * The following two tables are used by the TCP state machine function
 * tfTcpStateMachine().
 *
 * tlTcpTransitionMatrix[][]
 * The transition matrix not only corresponds to the TCP state diagram
 * pictured in figure 6 of RFC 793, but also addresses error conditions,
 * and actions which are not connected with state changes. Given a causing
 * event, and a TCP state, the transition matrix yields a next state to
 * transition to and an action index to be used in the state function table
 * below to obtain a list of functions to be called.
 *
 * tlTcpStateFuncTable[], the state function table gives a mapping between
 * an action procedure index and the list of TCP state functions that need
 * to be called to take that action. The state functions corresponding to
 * an action procedure index are called sequentially one at a time, and
 * the new TCP state is passed as a parameter to them. TCP state transition
 * to that new TCP state occurs in the indicated (in comments) state function.
 */
static const ttTcpStateFunctEntry  TM_CONST_QLF tlTcpStateFunctTable[]=
{
#define TM_TCPM_FREE_PACKET                                 0
#define TM_TCPMF_FREE_PACKET    \
    (tt8Bit)TM_TCPM_FREE_PACKET, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
    {TM_TCPMF_FREE_PACKET,       tfTcpFreePacket      }, /* 0 */
#define TM_TCPM_SND_RST_SEQ0                                1
#define TM_TCPMF_SND_RST_SEQ0    \
    (tt8Bit)TM_TCPM_SND_RST_SEQ0, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
    {TM_TCPMF_SND_RST_SEQ0,      tfTcpSendRstSeq0     }, /* 1 */
#define TM_TCPM_SND_RST                                     2
#define TM_TCPMF_SND_RST    \
    (tt8Bit)TM_TCPM_SND_RST, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
    {TM_TCPMF_SND_RST,           tfTcpSendRst         }, /* 2 */
#define TM_TCPM_SNDRST_DELTCB                               3
#define TM_TCPMF_SNDRST_DELTCB    \
    (tt8Bit)TM_TCPM_SNDRST_DELTCB, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
    {TM_TCPMF_SNDRST_DELTCB,     tfTcpSendRst         }, /* 3 */
/* State transition */
    {TM_TCPMF_SNDRST_DELTCB,     tfTcpVectClose       }, /* 4 */
#define TM_TCPM_RCV_ACKOFSYN                                5
#define TM_TCPMF_RCV_ACKOFSYN    \
    (tt8Bit)TM_TCPM_RCV_ACKOFSYN, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
/* State transition */
    {TM_TCPMF_RCV_ACKOFSYN,      tfTcpCheckValidIssAck}, /* 5 */
    {TM_TCPMF_RCV_ACKOFSYN,      tfTcpFreePacket      }, /* 6 */
#define TM_TCPM_RCV_SYN_ACK                                 7
#define TM_TCPMF_RCV_SYN_ACK    \
    (tt8Bit)TM_TCPM_RCV_SYN_ACK, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
/* State transition */
    {TM_TCPMF_RCV_SYN_ACK,       tfTcpCheckValidIssAck}, /* 7 */
/* state transition */
    {TM_TCPMF_RCV_SYN_ACK,       tfTcpRcvSyn          }, /* 8 */
    {TM_TCPMF_RCV_SYN_ACK,       tfTcpEstablish       }, /* 9 */
#define TM_TCPM_SND_DATA                                   10
#define TM_TCPMF_SND_DATA    \
    (tt8Bit)TM_TCPM_SND_DATA, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
    {TM_TCPMF_SND_DATA,          tfTcpSendData        },/* 10 */
#define TM_TCPM_SND_SYN_1                                  11
#define TM_TCPMF_SND_SYN_1    \
    (tt8Bit)TM_TCPM_SND_SYN_1, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
/* State transition */
    {TM_TCPMF_SND_SYN_1,         tfTcpSendSyn         },/* 11 */
#define TM_TCPM_SND_SYN_R                                  12
#define TM_TCPMF_SND_SYN_R    \
    (tt8Bit)TM_TCPM_SND_SYN_R, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
    {TM_TCPMF_SND_SYN_R,         tfTcpReSendSyn       },/* 12 */
#define TM_TCPM_SIMULT_OPEN                                13
#define TM_TCPMF_SIMULT_OPEN    \
    (tt8Bit)TM_TCPM_SIMULT_OPEN, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
/* state transition */
    {TM_TCPMF_SIMULT_OPEN,       tfTcpRcvSyn          },/* 13 */
    {TM_TCPMF_SIMULT_OPEN,       tfTcpSynData         },/* 14 */
    {TM_TCPMF_SIMULT_OPEN,       tfTcpSendSynAck      },/* 15 */
    {TM_TCPMF_SIMULT_OPEN,       tfTcpFreePacket      },/* 16 */
#define TM_TCPM_CON_REQ                                    17
#define TM_TCPMF_CON_REQ    \
    (tt8Bit)TM_TCPM_CON_REQ, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
/* State transition */
    {TM_TCPMF_CON_REQ,           tfTcpConReq          },/* 17 */
#define TM_TCPM_SND_SYN_ACK                                18
#define TM_TCPMF_SND_SYN_ACK    \
    (tt8Bit)TM_TCPM_SND_SYN_ACK, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
    {TM_TCPMF_SND_SYN_ACK,       tfTcpSendSynAck      },/* 18 */
#define TM_TCPM_ACCEPT_NOTIFY                              19
#define TM_TCPMF_ACCEPT_NOTIFY    \
    (tt8Bit)TM_TCPM_ACCEPT_NOTIFY, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
/* State transition */
    {TM_TCPMF_ACCEPT_NOTIFY,     tfTcpCheckValidIssAck},/* 19 */
    {TM_TCPMF_ACCEPT_NOTIFY,     tfTcpEstablish       },/* 20 */
#define TM_TCPM_CONN_REFUSED                               21
#define TM_TCPMF_CONN_REFUSED    \
    (tt8Bit)TM_TCPM_CONN_REFUSED, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
    {TM_TCPMF_CONN_REFUSED,      tfTcpNotifyConnResetFree}, /* 21 */
/* state transition */
    {TM_TCPMF_CONN_REFUSED,      tfTcpVectClose       },/* 22 */
#define TM_TCPM_CONN_RESET                                 23
#define TM_TCPMF_CONN_RESET    \
    (tt8Bit)TM_TCPM_CONN_RESET, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
    {TM_TCPMF_CONN_RESET,        tfTcpNotifyResetFree },/* 23 */
/* state transition */
    {TM_TCPMF_CONN_RESET,        tfTcpVectClose       },/* 24 */
/* Bad segment caused us to abort the connection */
#define TM_TCPM_BAD_SEG_ABORT                              25
#define TM_TCPMF_BAD_SEG_ABORT    \
    (tt8Bit)TM_TCPM_BAD_SEG_ABORT, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
    {TM_TCPMF_BAD_SEG_ABORT,     tfTcpSendRst         },/* 25 */
    {TM_TCPMF_BAD_SEG_ABORT,     tfTcpNotifyReset     },/* 26 */
/* state transition */
    {TM_TCPMF_BAD_SEG_ABORT,     tfTcpVectClose       },/* 27 */
#define TM_TCPM_SND_FIN                                    28
#define TM_TCPMF_SND_FIN    \
    (tt8Bit)TM_TCPM_SND_FIN, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
/* state transition */
    {TM_TCPMF_SND_FIN,           tfTcpSendFin         },/* 28 */
#define TM_TCPM_DEL_TCB                                    29
#define TM_TCPMF_DEL_TCB    \
    (tt8Bit)TM_TCPM_DEL_TCB, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
/* state transition */
    {TM_TCPMF_DEL_TCB,           tfTcpVectClose       },/* 29 */
#define TM_TCPM_DEL_TCB_FREE                               30
#define TM_TCPMF_DEL_TCB_FREE    \
    (tt8Bit)TM_TCPM_DEL_TCB_FREE, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
/* state transition */
    {TM_TCPMF_DEL_TCB_FREE,      tfTcpVectClose       },/* 30 */
    {TM_TCPMF_DEL_TCB_FREE,      tfTcpFreePacket      },/* 31 */
#define TM_TCPM_SEG_INVALID                                32
#define TM_TCPMF_SEG_INVALID    \
    (tt8Bit)TM_TCPM_SEG_INVALID, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
    {TM_TCPMF_SEG_INVALID,       tfTcpSegInvalid      },/* 32 */
#define TM_TCPM_RCV_ACK                                    33
#define TM_TCPMF_RCV_ACK    \
    (tt8Bit)TM_TCPM_RCV_ACK, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
    {TM_TCPMF_RCV_ACK,           tfTcpRcvAck          },/* 33 */
#define TM_TCPM_RCV_ACK_CMPLT                              34
#define TM_TCPMF_RCV_ACK_CMPLT    \
    (tt8Bit)TM_TCPM_RCV_ACK_CMPLT, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
/* tfTcpRcvAck queue or free the packet */
    {TM_TCPMF_RCV_ACK_CMPLT,     tfTcpRcvAck          },/* 34 */
    {TM_TCPMF_RCV_ACK_CMPLT,     tfTcpRcvAckComplt    },/* 35 */
#define TM_TCPM_RCVACKFIN_2MSL                             36
#define TM_TCPMF_RCVACKFIN_2MSL    \
    (tt8Bit)TM_TCPM_RCVACKFIN_2MSL, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
/* tfTcpRcvAck queue or free the packet */
    {TM_TCPMF_RCVACKFIN_2MSL,    tfTcpRcvAck          },/* 36 */
/* state transition, packet already freed or queued */
    {TM_TCPMF_RCVACKFIN_2MSL,    tfTcpCheckAckFin     },/* 37 */
    {TM_TCPMF_RCVACKFIN_2MSL,    tfTcpStart2MslTmr    },/* 38 */
#define TM_TCPM_RCVACKFINCLOSE                             39
#define TM_TCPMF_RCVACKFINCLOSE    \
    (tt8Bit)TM_TCPM_RCVACKFINCLOSE, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
/* tfTcpRcvAck queue or free the packet */
    {TM_TCPMF_RCVACKFINCLOSE,    tfTcpRcvAck          },/* 39 */
/* state transition */
    {TM_TCPMF_RCVACKFINCLOSE,    tfTcpCheckAckFin     },/* 40 */
/* state transition */
    {TM_TCPMF_RCVACKFINCLOSE,    tfTcpVectClose       },/* 41 */
#define TM_TCPM_RCVACKFINCMPLT                             42
#define TM_TCPMF_RCVACKFINCMPLT    \
    (tt8Bit)TM_TCPM_RCVACKFINCMPLT, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
/* tfTcpRcvAck queue or free the packet */
    {TM_TCPMF_RCVACKFINCMPLT,    tfTcpRcvAck          },/* 42 */
/* state transition */
    {TM_TCPMF_RCVACKFINCMPLT,    tfTcpCheckAckFin     },/* 43 */
    {TM_TCPMF_RCVACKFINCMPLT,    tfTcpRcvAckComplt    },/* 44 */
#define TM_TCPM_RCV_FIN                                    45
#define TM_TCPMF_RCV_FIN    \
    (tt8Bit)TM_TCPM_RCV_FIN, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
/* State transition */
    {TM_TCPMF_RCV_FIN,           tfTcpRcvFin          },/* 45 */
#define TM_TCPM_RCV_FIN_2MSL                               46
#define TM_TCPMF_RCV_FIN_2MSL    \
    (tt8Bit)TM_TCPM_RCV_FIN_2MSL, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
/* State transition */
    {TM_TCPMF_RCV_FIN_2MSL,      tfTcpRcvFin          },/* 46 */
    {TM_TCPMF_RCV_FIN_2MSL,      tfTcpStart2MslTmr    },/* 47 */
#define TM_TCPM_DEL_REXMIT                                 48
#define TM_TCPMF_DEL_REXMIT    \
    (tt8Bit)TM_TCPM_DEL_REXMIT, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
    {TM_TCPMF_DEL_REXMIT,        tfTcpDelReXmit       },/* 48 */
#define TM_TCPM_LISTEN                                     49
#define TM_TCPMF_LISTEN        \
    (tt8Bit)TM_TCPM_LISTEN, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
    {TM_TCPMF_LISTEN,            tfTcpListen          },/* 49 */
#define TM_TCPM_SND_FIN_R                                  50
#define TM_TCPMF_SND_FIN_R     \
    (tt8Bit)TM_TCPM_SND_FIN_R, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
    {TM_TCPMF_SND_FIN_R,         tfTcpReSendFin       },/* 50 */
#define TM_TCPM_LAST_INDEX                                 51
#define TM_TCPMF_LAST_INDEX    \
    (tt8Bit)TM_TCPM_LAST_INDEX, {TM_8BIT_ZERO, TM_8BIT_ZERO, TM_8BIT_ZERO}
    {TM_TCPMF_LAST_INDEX,        (ttTcpStateFunctPtr)0} /* 51 */
};

/*
 * The transition matrix tlTcpTransitionMatrix[][] below yields an action
 * procedure index, and a new TCP state given a current TCP state and a
 * causing event. tlTcpTransitionMatrix[][] is described in more details
 * above. States, and Causing events are defined and described above.
 */
static const ttTransitionMatrixEntry  TM_CONST_QLF
                              tlTcpTransitionMatrix[TM_TCPS_TIME_WAIT + 1]
                                                   [TM_TCPE_ABORT + 1]      =
{
    {/* State == TM_TCPS_CLOSED */
/*      Event                    Next State          Action Procedure Index */
    {/* TM_TCPE_ACTIVE_OPEN  */ TM_TCPS_SYN_SENT,     TM_TCPM_SND_SYN_1     },
    {/* TM_TCPE_PASSIVE_OPEN */ TM_TCPS_LISTEN,       TM_TCPM_LISTEN        },
    {/* TM_TCPE_SEG_SYN      */ TM_TCPS_CLOSED,       TM_TCPM_SND_RST_SEQ0  },
    {/* TM_TCPE_SEG_SYN_ACK  */ TM_TCPS_CLOSED,       TM_TCPM_SND_RST       },
    {/* TM_TCPE_SEG_ACK      */ TM_TCPS_CLOSED,       TM_TCPM_SND_RST       },
/* Ignore FIN in closed state */
    {/* TM_TCPE_SEG_FIN      */ TM_TCPS_CLOSED,       TM_MATRIX_NOOP        },
    {/* TM_TCPE_SEG_RST      */ TM_TCPS_CLOSED,       TM_TCPM_FREE_PACKET   },
/* Segment with ACK bit off */
/*
 * ANVL-CORE 1.4
 * ANVL sends a FIN with no ACK, and data with not ACK to test closed state
 * response. Send a RESET with Seq 0, since there is no ACK.
 */
    {/* TM_TCPE_SEG_INVALID  */ TM_TCPS_CLOSED,       TM_TCPM_SND_RST_SEQ0  },
    {/* TM_TCPE_RCV_USER     */ TM_TCPS_CLOSED,       TM_TCPM_ENOTCONN      },
    {/* TM_TCPE_REXMIT       */ TM_TCPS_CLOSED,       TM_TCPM_ENOTCONN      },
/* User issued a close, and state vector is closed */
     {/* TM_TCPE_CLOSE       */ TM_TCPS_CLOSED,       TM_TCPM_DEL_TCB       },
/*
 * User isssued a close (with event changed to abort in tfTcpClose), and
 * state vector is closed
 */
     {/* TM_TCPE_ABORT       */ TM_TCPS_CLOSED,       TM_TCPM_DEL_TCB       }
    } ,
    {/* State == TM_TCPS_LISTEN */
/*      Event                    Next State          Action Procedure Index */
/* Do not allow connect on a listening socket */
    {/* TM_TCPE_ACTIVE_OPEN  */ TM_TCPS_LISTEN,       TM_TCPM_EOPNOTSUPP    },
    {/* TM_TCPE_PASSIVE_OPEN */ TM_TCPS_LISTEN,       TM_MATRIX_NOOP        },
    {/* TM_TCPE_SEG_SYN      */ TM_TCPS_SYN_RECEIVED, TM_TCPM_CON_REQ       },
    {/* TM_TCPE_SEG_SYN_ACK  */ TM_TCPS_LISTEN,       TM_TCPM_SND_RST       },
/* Any ACK received in the listening state, causes us to send a Reset */
    {/* TM_TCPE_SEG_ACK      */ TM_TCPS_LISTEN,       TM_TCPM_SND_RST       },
/* Ignore FIN in listen state */
    {/* TM_TCPE_SEG_FIN      */ TM_TCPS_LISTEN,       TM_MATRIX_NOOP        },
    {/* TM_TCPE_SEG_RST      */ TM_TCPS_LISTEN,       TM_TCPM_FREE_PACKET   },
/* Ignore FIN (with no ACK) in listen state */
    {/* TM_TCPE_SEG_INVALID  */ TM_TCPS_LISTEN,       TM_TCPM_SEG_INVALID   },
/* Allow for select read (accept) */
    {/* TM_TCPE_RCV_USER     */ TM_TCPS_LISTEN,       TM_MATRIX_NOOP        },
    {/* TM_TCPE_REXMIT       */ TM_TCPS_LISTEN,       TM_TCPM_ENOTCONN      },
    {/* TM_TCPE_CLOSE        */ TM_TCPS_CLOSED,       TM_TCPM_DEL_TCB       },
    {/* TM_TCPE_ABORT        */ TM_TCPS_CLOSED,       TM_TCPM_DEL_TCB       }
    } ,
    {/* State == TM_TCPS_SYN_SENT */
/*      Event                    Next State          Action Procedure Index */
    {/* TM_TCPE_ACTIVE_OPEN  */ TM_TCPS_SYN_SENT,     TM_TCPM_EINPROGRESS   },
    {/* TM_TCPE_PASSIVE_OPEN */ TM_TCPS_SYN_SENT,     TM_TCPM_EADDRINUSE    },
    {/* TM_TCPE_SEG_SYN      */ TM_TCPS_SYN_RECEIVED, TM_TCPM_SIMULT_OPEN   },
    {/* TM_TCPE_SEG_SYN_ACK  */ TM_TCPS_ESTABLISHED,  TM_TCPM_RCV_SYN_ACK   },
    {/* TM_TCPE_SEG_ACK      */ TM_TCPS_SYN_SENT,     TM_TCPM_RCV_ACKOFSYN  },
/* Ignore FIN in SYN-SENT state */
    {/* TM_TCPE_SEG_FIN      */ TM_TCPS_SYN_SENT,     TM_MATRIX_NOOP        },
    {/* TM_TCPE_SEG_RST      */ TM_TCPS_CLOSED,       TM_TCPM_CONN_REFUSED  },
    {/* TM_TCPE_SEG_INVALID  */ TM_TCPS_SYN_SENT,     TM_TCPM_SEG_INVALID   },
/* Let the user pend on recv. No error */
    {/* TM_TCPE_RCV_USER     */ TM_TCPS_SYN_SENT,     TM_MATRIX_NOOP        },
    {/* TM_TCPE_REXMIT       */ TM_TCPS_SYN_SENT,     TM_TCPM_SND_SYN_R     },
    {/* TM_TCPE_CLOSE        */ TM_TCPS_CLOSED,       TM_TCPM_DEL_TCB       },
    {/* TM_TCPE_ABORT        */ TM_TCPS_CLOSED,       TM_TCPM_DEL_TCB       }
    } ,
    {/* State == TM_TCPS_SYN_RECEIVED */
/*      Event                    Next State          Action Procedure Index */
    {/* TM_TCPE_ACTIVE_OPEN  */ TM_TCPS_SYN_RECEIVED, TM_TCPM_EINPROGRESS   },
    {/* TM_TCPE_PASSIVE_OPEN */ TM_TCPS_SYN_RECEIVED, TM_TCPM_EADDRINUSE    },
/* Segment caused us to abort the connection */
    {/* TM_TCPE_SEG_SYN,     */ TM_TCPS_CLOSED,       TM_TCPM_BAD_SEG_ABORT },
/* Duplicate SYN already removed in tfTcpIncomingPacket, so this is illegal */
    {/* TM_TCPE_SEG_SYN_ACK, */ TM_TCPS_CLOSED,       TM_TCPM_BAD_SEG_ABORT },
/* accept or simultaneous open */
    {/* TM_TCPE_SEG_ACK      */ TM_TCPS_ESTABLISHED,  TM_TCPM_ACCEPT_NOTIFY },
/*
 * This transition only occurs, if we did not get an ACK of our SYN, since
 * the ACK would have moved us to the established state first.
 */
    {/* TM_TCPE_SEG_FIN      */ TM_TCPS_CLOSE_WAIT,   TM_TCPM_RCV_FIN       },
    {/* TM_TCPE_SEG_RST      */ TM_TCPS_CLOSED,       TM_TCPM_CONN_REFUSED  },
    {/* TM_TCPE_SEG_INVALID  */ TM_TCPS_SYN_RECEIVED, TM_TCPM_SEG_INVALID   },
/* Let the user pend on recv. No error */
    {/* TM_TCPE_RCV_USER     */ TM_TCPS_SYN_RECEIVED, TM_MATRIX_NOOP        },
    {/* TM_TCPE_REXMIT       */ TM_TCPS_SYN_RECEIVED, TM_TCPM_SND_SYN_ACK   },
    {/* TM_TCPE_CLOSE        */ TM_TCPS_FIN_WAIT_1,   TM_TCPM_SND_FIN       },
    {/* TM_TCPE_ABORT        */ TM_TCPS_CLOSED,       TM_TCPM_SNDRST_DELTCB }
    } ,
    {/* State == TM_TCPS_ESTABLISHED */
/*      Event                    Next State          Action Procedure Index */
    {/* TM_TCPE_ACTIVE_OPEN  */ TM_TCPS_ESTABLISHED,  TM_TCPM_EISCONN       },
    {/* TM_TCPE_PASSIVE_OPEN */ TM_TCPS_ESTABLISHED,  TM_TCPM_EADDRINUSE    },
/* Segment caused us to abort the connection */
    {/* TM_TCPE_SEG_SYN,     */ TM_TCPS_CLOSED,       TM_TCPM_BAD_SEG_ABORT },
/* Duplicate SYN already removed in tfTcpIncomingPacket, so this illegal */
    {/* TM_TCPE_SEG_SYN_ACK, */ TM_TCPS_CLOSED,       TM_TCPM_BAD_SEG_ABORT },
    {/* TM_TCPE_SEG_ACK      */ TM_TCPS_ESTABLISHED,  TM_TCPM_RCV_ACK       },
    {/* TM_TCPE_SEG_FIN      */ TM_TCPS_CLOSE_WAIT,   TM_TCPM_RCV_FIN       },
    {/* TM_TCPE_SEG_RST      */ TM_TCPS_CLOSED,       TM_TCPM_CONN_RESET    },
    {/* TM_TCPE_SEG_INVALID  */ TM_TCPS_ESTABLISHED,  TM_TCPM_SEG_INVALID   },
/* Let the user pend on recv. No error */
    {/* TM_TCPE_RCV_USER     */ TM_TCPS_ESTABLISHED,  TM_MATRIX_NOOP        },
    {/* TM_TCPE_REXMIT       */ TM_TCPS_ESTABLISHED,  TM_TCPM_SND_DATA      },
    {/* TM_TCPE_CLOSE        */ TM_TCPS_FIN_WAIT_1,   TM_TCPM_SND_FIN       },
    {/* TM_TCPE_ABORT        */ TM_TCPS_CLOSED,       TM_TCPM_SNDRST_DELTCB }
    } ,
    {/* State == TM_TCPS_CLOSE_WAIT */
/*      Event                    Next State          Action Procedure Index */
    {/* TM_TCPE_ACTIVE_OPEN, */ TM_TCPS_CLOSE_WAIT,   TM_TCPM_EISCONN       },
    {/* TM_TCPE_PASSIVE_OPEN */ TM_TCPS_CLOSE_WAIT,   TM_TCPM_EADDRINUSE    },
    {/* TM_TCPE_SEG_SYN      */ TM_TCPS_CLOSED,       TM_TCPM_BAD_SEG_ABORT },
    {/* TM_TCPE_SEG_SYN_ACK  */ TM_TCPS_CLOSED,       TM_TCPM_BAD_SEG_ABORT },
    {/* TM_TCPE_SEG_ACK      */ TM_TCPS_CLOSE_WAIT,   TM_TCPM_RCV_ACK       },
    {/* TM_TCPE_SEG_FIN      */ TM_TCPS_CLOSE_WAIT,   TM_MATRIX_NOOP        },
    {/* TM_TCPE_SEG_RST      */ TM_TCPS_CLOSED,       TM_TCPM_CONN_RESET    },
    {/* TM_TCPE_SEG_INVALID  */ TM_TCPS_CLOSE_WAIT,   TM_TCPM_SEG_INVALID   },
/* Remote site has already sent a FIN. No more data to be expected */
    {/* TM_TCPE_RCV_USER     */ TM_TCPS_CLOSE_WAIT,   TM_TCPM_ESHUTDOWN     },
    {/* TM_TCPE_REXMIT       */ TM_TCPS_CLOSE_WAIT,   TM_TCPM_SND_DATA      },
    {/* TM_TCPE_CLOSE        */ TM_TCPS_LAST_ACK,     TM_TCPM_SND_FIN       },
    {/* TM_TCPE_ABORT        */ TM_TCPS_CLOSED,       TM_TCPM_SNDRST_DELTCB }
    } ,
    {/* State == TM_TCPS_FIN_WAIT_1 */
/*      Event                    Next State          Action Procedure Index */
    {/* TM_TCPE_ACTIVE_OPEN  */ TM_TCPS_FIN_WAIT_1,   TM_TCPM_ESHUTDOWN     },
    {/* TM_TCPE_PASSIVE_OPEN */ TM_TCPS_FIN_WAIT_1,   TM_TCPM_EADDRINUSE    },
    {/* TM_TCPE_SEG_SYN      */ TM_TCPS_CLOSED,       TM_TCPM_BAD_SEG_ABORT },
    {/* TM_TCPE_SEG_SYN_ACK  */ TM_TCPS_CLOSED,       TM_TCPM_BAD_SEG_ABORT },
/* Check if ack is ack of FIN for transition */
    {/* TM_TCPE_SEG_ACK      */ TM_TCPS_FIN_WAIT_2,   TM_TCPM_RCVACKFINCMPLT},
    {/* TM_TCPE_SEG_FIN      */ TM_TCPS_CLOSING,      TM_TCPM_RCV_FIN       },
    {/* TM_TCPE_SEG_RST      */ TM_TCPS_CLOSED,       TM_TCPM_CONN_RESET    },
    {/* TM_TCPE_SEG_INVALID  */ TM_TCPS_FIN_WAIT_1,   TM_TCPM_SEG_INVALID   },
/* Let the user pend on recv. No error */
    {/* TM_TCPE_RCV_USER     */ TM_TCPS_FIN_WAIT_1,   TM_MATRIX_NOOP        },
    {/* TM_TCPE_REXMIT       */ TM_TCPS_FIN_WAIT_1,   TM_TCPM_SND_FIN_R     },
    {/* TM_TCPE_CLOSE        */ TM_TCPS_FIN_WAIT_1,   TM_TCPM_ESHUTDOWN     },
    {/* TM_TCPE_ABORT        */ TM_TCPS_CLOSED,       TM_TCPM_SNDRST_DELTCB }
    } ,
    {/* State == TM_TCPS_CLOSING */
/*      Event                    Next State          Action Procedure Index */
    {/* TM_TCPE_ACTIVE_OPEN, */ TM_TCPS_CLOSING,      TM_TCPM_ESHUTDOWN     },
    {/* TM_TCPE_PASSIVE_OPEN */ TM_TCPS_CLOSING,      TM_TCPM_EADDRINUSE    },
    {/* TM_TCPE_SEG_SYN      */ TM_TCPS_CLOSED,       TM_TCPM_BAD_SEG_ABORT },
    {/* TM_TCPE_SEG_SYN_ACK  */ TM_TCPS_CLOSED,       TM_TCPM_BAD_SEG_ABORT },
/* Check if ack is ack of FIN for transition */
    {/* TM_TCPE_SEG_ACK      */ TM_TCPS_TIME_WAIT,    TM_TCPM_RCVACKFIN_2MSL},
    {/* TM_TCPE_SEG_FIN      */ TM_TCPS_CLOSING,      TM_MATRIX_NOOP        },
    {/* TM_TCPE_SEG_RST      */ TM_TCPS_CLOSED,       TM_TCPM_DEL_TCB_FREE  },
    {/* TM_TCPE_SEG_INVALID  */ TM_TCPS_CLOSING,      TM_TCPM_SEG_INVALID   },
    {/* TM_TCPE_RCV_USER     */ TM_TCPS_CLOSING,      TM_TCPM_ESHUTDOWN     },
    {/* TM_TCPE_REXMIT       */ TM_TCPS_CLOSING,      TM_TCPM_SND_FIN_R     },
    {/* TM_TCPE_CLOSE        */ TM_TCPS_CLOSING,      TM_TCPM_ESHUTDOWN     },
    {/* TM_TCPE_ABORT        */ TM_TCPS_CLOSED,       TM_TCPM_DEL_TCB       }
    } ,
    {/* State == TM_TCPS_LAST_ACK */
/*      Event                    Next State          Action Procedure Index */
    {/* TM_TCPE_ACTIVE_OPEN  */ TM_TCPS_LAST_ACK,     TM_TCPM_ESHUTDOWN     },
    {/* TM_TCPE_PASSIVE_OPEN */ TM_TCPS_LAST_ACK,     TM_TCPM_EADDRINUSE    },
    {/* TM_TCPE_SEG_SYN      */ TM_TCPS_CLOSED,       TM_TCPM_BAD_SEG_ABORT },
    {/* TM_TCPE_SEG_SYN_ACK  */ TM_TCPS_CLOSED,       TM_TCPM_BAD_SEG_ABORT },
    {/* TM_TCPE_SEG_ACK      */ TM_TCPS_CLOSED,       TM_TCPM_RCVACKFINCLOSE},
    {/* TM_TCPE_SEG_FIN      */ TM_TCPS_LAST_ACK,     TM_MATRIX_NOOP        },
    {/* TM_TCPE_SEG_RST      */ TM_TCPS_CLOSED,       TM_TCPM_DEL_TCB_FREE  },
    {/* TM_TCPE_SEG_INVALID  */ TM_TCPS_LAST_ACK,     TM_TCPM_SEG_INVALID   },
/* Remote site has already sent a FIN. No more data to be expected */
    {/* TM_TCPE_RCV_USER     */ TM_TCPS_LAST_ACK,     TM_TCPM_ESHUTDOWN     },
    {/* TM_TCPE_REXMIT       */ TM_TCPS_LAST_ACK,     TM_TCPM_SND_FIN_R     },
    {/* TM_TCPE_CLOSE        */ TM_TCPS_LAST_ACK,     TM_TCPM_ESHUTDOWN     },
    {/* TM_TCPE_ABORT        */ TM_TCPS_CLOSED,       TM_TCPM_DEL_TCB       }
    } ,
    {/* State == TM_TCPS_FIN_WAIT_2 */
/*      Event                    Next State          Action Procedure Index */
    {/* TM_TCPE_ACTIVE_OPEN  */ TM_TCPS_FIN_WAIT_2,   TM_TCPM_ESHUTDOWN     },
    {/* TM_TCPE_PASSIVE_OPEN */ TM_TCPS_FIN_WAIT_2,   TM_TCPM_EADDRINUSE    },
    {/* TM_TCPE_SEG_SYN      */ TM_TCPS_CLOSED,       TM_TCPM_BAD_SEG_ABORT },
    {/* TM_TCPE_SEG_SYN_ACK  */ TM_TCPS_CLOSED,       TM_TCPM_BAD_SEG_ABORT },
/* RecvAck + if send queue empty, call close complete */
    {/* TM_TCPE_SEG_ACK      */ TM_TCPS_FIN_WAIT_2,   TM_TCPM_RCV_ACK_CMPLT },
    {/* TM_TCPE_SEG_FIN      */ TM_TCPS_TIME_WAIT,    TM_TCPM_RCV_FIN_2MSL  },
    {/* TM_TCPE_SEG_RST      */ TM_TCPS_CLOSED,       TM_TCPM_CONN_RESET    },
    {/* TM_TCPE_SEG_INVALID  */ TM_TCPS_FIN_WAIT_2,   TM_TCPM_SEG_INVALID   },
/* Let the user pend on recv. No error */
    {/* TM_TCPE_RCV_USER     */ TM_TCPS_FIN_WAIT_2,   TM_MATRIX_NOOP        },
/* Our FIN has already been acked */
    {/* TM_TCPE_REXMIT       */ TM_TCPS_FIN_WAIT_2 ,  TM_TCPM_SND_DATA      },
    {/* TM_TCPE_CLOSE        */ TM_TCPS_FIN_WAIT_2,   TM_TCPM_ESHUTDOWN     },
    {/* TM_TCPE_ABORT        */ TM_TCPS_CLOSED,       TM_TCPM_SNDRST_DELTCB }
    } ,
    {/* State == TM_TCPS_TIME_WAIT */
/*
 * NOTES: 1. incoming segment events are NOOP.
 *        2. User events coming from the socket are true events.
 * Note1: the TCP vector is no longer in the tree in the time wait state.
 * So the incoming segment events are NOOP in this table, as they will be used
 * on the time wait vector (which will be found) instead. So The actions and
 * transitions are done with the time wait vector instead in the
 * tfTcpTmWtProcess() function.
 * NOTE2: The user initiated events will find the socket in the array, and
 * will execute these actions and transitions.
 */
/*      Event                    Next State          Action Procedure Index */
    {/* TM_TCPE_ACTIVE_OPEN  */ TM_TCPS_TIME_WAIT,    TM_TCPM_ESHUTDOWN     },
    {/* TM_TCPE_PASSIVE_OPEN */ TM_TCPS_TIME_WAIT,    TM_TCPM_EADDRINUSE    },
    {/* TM_TCPE_SEG_SYN      */ TM_TCPS_TIME_WAIT,    TM_MATRIX_NOOP        },
    {/* TM_TCPE_SEG_SYN_ACK  */ TM_TCPS_TIME_WAIT,    TM_MATRIX_NOOP        },
    {/* TM_TCPE_SEG_ACK      */ TM_TCPS_TIME_WAIT,    TM_MATRIX_NOOP        },
    {/* TM_TCPE_SEG_FIN      */ TM_TCPS_TIME_WAIT,    TM_MATRIX_NOOP        },
    {/* TM_TCPE_SEG_RST      */ TM_TCPS_TIME_WAIT,    TM_MATRIX_NOOP        },
    {/* TM_TCPE_SEG_INVALID  */ TM_TCPS_TIME_WAIT,    TM_MATRIX_NOOP        },
/* Remote site has already sent a FIN. No more data to be expected */
    {/* TM_TCPE_RCV_USER     */ TM_TCPS_TIME_WAIT,    TM_TCPM_ESHUTDOWN     },
/* Retransmission timer should not be running in TIME_WAIT state. */
    {/* TM_TCPE_REXMIT       */ TM_TCPS_TIME_WAIT,    TM_TCPM_DEL_REXMIT    },
/* Only affects the tcp vector, not the time wait vector */
    {/* TM_TCPE_CLOSE        */ TM_TCPS_CLOSED,       TM_TCPM_DEL_TCB       },
/* Only affects the tcp vector, not the time wait vector */
    {/* TM_TCPE_ABORT        */ TM_TCPS_CLOSED,       TM_TCPM_DEL_TCB       }
    }
};

/*
 * Tcp functions
 */
#define static

/*
 * tfTcpInit() Function Description
 * This functions is called by tfInit() to initialize the TCP
 * context variables:
 * . Initialize tvTcpTimerTime.
 * No parameter.
 * No return value.
 */
void tfTcpInit (void)
{
#ifdef tvTime
    tm_kernel_set_critical;
    tm_context(tvTcpTimerTime) = tvTime;
    tm_kernel_release_critical;
#endif /* tvTime */
#ifdef TM_USE_IPV4
#ifndef TM_USE_PREDICTABLE_IPID
    tfGetRandomBytes((tt8BitPtr)&tm_context(tvTcpIpId),
                     sizeof(tm_context(tvTcpIpId)));
#endif /* !TM_USE_PREDICTABLE_IPID */
#endif /* TM_USE_IPV4 */
    return;
}

/*
 * Allocate a closed TCP vector (to allow usage of state functions when
 * no matching TCP vector is found)
 */
ttTcpVectPtr tfTcpClosedVectGet (void)
{
    ttSocketEntryPtr socketPtr;
    ttTcpVectPtr     tcpVectPtr;

    tcpVectPtr = (ttTcpVectPtr)(ttVoidPtr)tm_recycle_alloc(sizeof(ttTcpVect),
                                                           TM_RECY_TCPVEC);
    if (tcpVectPtr != (ttTcpVectPtr)0)
    {
/* State is closed */
        socketPtr = &(tcpVectPtr->tcpsSocketEntry);
/* Socket owned by incoming thread + 1 to prevent tfSocketFree call */
        socketPtr->socOwnerCount = 2;
        socketPtr->socIndex = TM_SOC_NO_INDEX;
#ifdef TM_USE_IPV4
/* Field used when sending a RESET when no socket is found */
        socketPtr->socIpTos = tm_context(tvIpDefTos);
#endif /* TM_USE_IPV4 */
        tm_call_lock_wait(&(socketPtr->socLockEntry));
    }
    return tcpVectPtr;
}

/* Free a closed TCP vector */
void tfTcpClosedVectFree (ttTcpVectPtr tcpVectPtr)
{
/* Reset routing entry cache */
    tfRtCacheUnGet(&(tcpVectPtr->tcpsSocketEntry.socRteCacheStruct));
/* Free entry */
    tm_recycle_free(&(tcpVectPtr->tcpsSocketEntry), TM_RECY_TCPVEC);
    return;
}

/*
 * tfTcpAbort() Function Description
 * Called by the user to abort a connection, or by a timer function when
 * a time out occurs, or when the user closes a listening socket, to abort
 * all the pending connections.
 * Set the tcpsAbortError with soft error if soft error is set and errorCode
 * is TM_ETIMEDOUT, or with passed errorCode otherwise
 * Crank state machine with an ABORT event.
 * This will send a Reset to peer if state allows,
 * delete state vector from the tree, and notify the user if tcpsAbortError
 * is non zero (done in tfTcpVectClose()).
 *
 * Parameter   Description
 *
 * tcpVectPtr  Tcp State Vector pointer
 * errorCode   Error code associated with the abort
 *
 * Returns
 * Value        Meaning
 * Different error values as returned from the state machine
 */
int tfTcpAbort (ttTcpVectPtr   tcpVectPtr,
                int            errorCode)
{
/* If soft error set and errorCode is timed out, use soft error */
    if ((tcpVectPtr->tcpsSoftError != 0) && (errorCode == TM_ETIMEDOUT))
    {
        tcpVectPtr->tcpsAbortError = tcpVectPtr->tcpsSoftError;
    }
/* Otherwise use error code */
    else
    {
        tcpVectPtr->tcpsAbortError = errorCode;
    }
    errorCode = tfTcpStateMachine( tcpVectPtr,
                                   TM_PACKET_NULL_PTR,
                                   TM_TCPE_ABORT );
    return errorCode;
}

/*
 * tfTcpVectClose() Function Description
 * All transitions to closed state should call tfTcpVectClose()
 * Called from the state machine function call loop or directly from
 * tfTcpStart2MslTmr
 *  1. . free all queued send buffers
 *     . free TCP recycled headers.
 *     . free TCP timers
 *     . Remove the tcp state vector from the socket tree.
 *     . if socket is a listening socket, abort or close all accepted
 *       connections
 *     . reset socket routing entry cache
 *     . reset incoming TCP socket cache
 *     . notify the user if hard error or timeout with soft error occurred.
 *  2. Calls tfTcpCompleteClose to finish a user initialited socket close
 *     (if any).
 *
 * Parameter   description
 * tcpVectPtr  pointer to TCP state vector/socket entry
 * packetPtr   Null pointer
 * newState    new TCP state (TM_TCP_CLOSED always)
 *
 * Returns
 * Value        Meaning
 * TM_ENOERROR  no error
 */
static int tm_state_function (tfTcpVectClose, tcpVectPtr, packetPtr, newState)
{
    ttTcpVectPtr            conReqTcpVectPtr;
    ttSocketEntryPtr        listenSocketPtr;
    ttSocketEntryPtr        socketPtr;
    int                     selectFlags;
    int                     socketCBFlags;
    int                     errorCode;
    int                     queueIndex;
    tt8Bit                  cacheReset;
    tt8Bit                  needUnlock;

    TM_UNREF_IN_ARG(packetPtr);
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
#ifdef TM_SNMP_MIB
/*
 * Number of times TCP connections have made a direct transition to the
 * CLOSED state from either the SYN-SENT state or the SYN-RCVD state,
 * plus the number of times TCP connections have made a direct transition
 * to the LISTEN state from the SYN-RCVD state.
 */
    if (    (tcpVectPtr->tcpsState == TM_TCPS_SYN_SENT)
         || (tcpVectPtr->tcpsState == TM_TCPS_SYN_RECEIVED) )
    {
        tm_context(tvTcpData).tcpAttemptFails++;
    }
/*
 *  number of times TCP connections have made a direct
 *  transition to the CLOSED state from either the ESTABLISHED
 *  state or the CLOSE-WAIT state."
 */
    if (    (tcpVectPtr->tcpsState == TM_TCPS_ESTABLISHED)
         || (tcpVectPtr->tcpsState == TM_TCPS_CLOSE_WAIT) )
    {
        tm_context(tvTcpData).tcpEstabResets++;
/*
 * number of TCP connections for which the current state
 * is either ESTABLISHED or CLOSE-WAIT. We are moving to
 * CLOSED state.
 */
        tm_context(tvTcpData).tcpCurrEstab--;
    }
#endif /* TM_SNMP_MIB */
    tcpVectPtr->tcpsState = newState;
/* Do not allow any more recycling of tcp headers */
    tm_16bit_clr_bit( tcpVectPtr->tcpsFlags,
                      (TM_TCPF_RECYCLE_HDR|TM_TCPF_SOCKETENTRYPTR) );
/* Socket no longer connected */
    tm_16bit_clr_bit( socketPtr->socFlags, TM_SOCF_CONNECTED );
/* Can no longer queue send data */
    tm_16bit_set_bit( socketPtr->socFlags, TM_SOCF_NO_MORE_SEND_DATA );
/*
 * Do not try and ACK or/and send more data at the end of
 * tfTcpIncomingPacket
 */
    tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2,
                      (  TM_TCPF2_ACK
                       | TM_TCPF2_SEND_DATA
                       | TM_TCPF2_REXMIT_TEMP
                       | TM_TCPF2_REINIT_TIMER
                       | TM_TCPF2_DF_ERROR) );
/* Free all the packets in the send queue (if any) */
    tfFreePacket( socketPtr->socSendQueueNextPtr,
                  TM_SOCKET_LOCKED);
    socketPtr->socSendQueueBytes = TM_32BIT_ZERO;
    socketPtr->socSendQueueNextPtr = TM_PACKET_NULL_PTR;
    socketPtr->socSendQueueLastPtr = TM_PACKET_NULL_PTR;
#ifdef TM_TCP_SACK
/* Free the send queue SACKED blocks */
    tfTcpSackFreeQueue(tcpVectPtr, TM_TCP_SACK_RECV_INDEX);
#endif /* TM_TCP_SACK */
/* Free the TCP headers */
    tfTcpPurgeTcpHeaders(tcpVectPtr);
/* Free all the TCP state vector timers */
    tfTcpFreeTimers(tcpVectPtr, TM_TCPTM_DELAYACK, TM_TCPTM_LINGER);
/* Reset options */
    socketPtr->socOptions = 0;
    if (tm_16bit_one_bit_set(socketPtr->socFlags, TM_SOCF_IN_TREE))
    {
/* Remove entry from the socket tree */
        (void)tfSocketTreeDelete(socketPtr, TM_16BIT_ZERO);
    }
    if (socketPtr->socFlags & TM_SOCF_LISTENING)
    {
/*
 * If socket is a listening socket:
 * Abort all the connection request sockets that have not been accepted yet.
 * Need to do this with the listening socket lock on, otherwise the
 * connection queues could get corrupted.
 * Reset the back pointer from the connection request sockets to this socket
 * first to avoid deadlock (re-locking the listening socket lock) in
 * tfTcpVectClose called from tfTcpAbort for a connection request socket.
 */
        for (queueIndex = 0; queueIndex <= TM_TCP_ACCEPT_QUEUE; queueIndex++)
        {
            conReqTcpVectPtr =
                        tcpVectPtr->tcpsConQueue[queueIndex].tcpConReqNextPtr;
            while (conReqTcpVectPtr != tcpVectPtr)
            {
                tfTcpRemoveResetConReq(conReqTcpVectPtr, queueIndex);
/*
 * Point to next entry in list. Use head of queue, because previous entry has
 * been removed.
 */
                conReqTcpVectPtr =
                        tcpVectPtr->tcpsConQueue[queueIndex].tcpConReqNextPtr;
            }
        }
    }
/* Reset routing entry cache */
    tfRtCacheUnGet(&socketPtr->socRteCacheStruct);
/* Save this pointer before we unlock the socket */
    listenSocketPtr = socketPtr->socListenSocketPtr;
    if (listenSocketPtr != (ttSocketEntryPtr)0)
    {
/*
 * Increase the number of threads that need access to the listening socket.
 * This is to prevent the listening socket owner count from being decreased,
 * when the back pointer from this socket to the listening socket is reset
 * if the listening socket is closed after our socket is unlocked.
 * (This could also be viewed as defering the listening socket owner count
 *  increase until we acquire the listening socket lock.)
 */
        tcpVectPtr->tcpsThreadCount++;
    }
    errorCode = tcpVectPtr->tcpsAbortError;
    if (    (errorCode != TM_ENOERROR)
         || (!(socketPtr->socFlags & TM_SOCF_LISTENING)) )
    {
        socketCBFlags = tcpVectPtr->tcpsSocketCBFlags;
        tcpVectPtr->tcpsSocketCBFlags = 0;
        selectFlags =   tcpVectPtr->tcpsSelectFlags;
        tcpVectPtr->tcpsSelectFlags = 0;
/* Notify non listening socket of TCP vector close */
        if (!(socketPtr->socFlags & TM_SOCF_LISTENING))
        {
            socketCBFlags |= TM_CB_TCPVECT_CLOSE;
        }
        if (errorCode != TM_ENOERROR)
        {
/* If abort with error, notify user about the error */
            tcpVectPtr->tcpsAbortError = TM_ENOERROR;
            selectFlags |= (TM_SELECT_READ | TM_SELECT_WRITE);
            if ( (errorCode == TM_ECONNREFUSED) || (errorCode == TM_ECONNRESET) )
            {
                socketCBFlags |= TM_CB_RESET|TM_CB_SOCKET_ERROR;
            }
            else
            {
                socketCBFlags |= TM_CB_SOCKET_ERROR;
            }
            if (tm_8bit_one_bit_set( socketPtr->socFlags2,
                                     TM_SOCF2_CONNECTING ) )
            {
/* Set the connect error code */
                tfSocketErrorRecord((int)socketPtr->socIndex, errorCode);
            }
        }
/* Notify of error and/or TCP vector close */
        tfSocketNotify( socketPtr,
                        selectFlags,
                        socketCBFlags,
                        errorCode );
    }
/*
 * Reset the incoming socket cache. Need to unlock to avoid deadlock with
 * cache lock. This is safe since we have removed the entry from the tree,
 * and the cache hit logic checks that the entry is still in the tree.
 */
    tm_call_unlock(&socketPtr->socLockEntry);
    tm_call_lock_wait(&tm_context(tvTcpCacheLock));
    if (tcpVectPtr == tm_context(tvTcpVectPtr))
    {
        cacheReset = TM_8BIT_YES;
/* Reset the cache Tcp state vector pointer */
        tm_context(tvTcpVectPtr) = TM_TCPVECT_NULL_PTR;
    }
    else
    {
        cacheReset = TM_8BIT_ZERO;
    }
    tm_call_unlock(&tm_context(tvTcpCacheLock));
/*
 * Check if we need to check in a listening socket, and remove our socket
 * from the queue of pending connection request. Need to check
 * with current socket unlocked to avoid deadlock with listening socket lock.
 */
    if (listenSocketPtr != (ttSocketEntryPtr)0)
    {
        needUnlock = TM_8BIT_YES;
        tm_call_lock_wait(&listenSocketPtr->socLockEntry);
        tm_call_lock_wait(&socketPtr->socLockEntry);
/*
 * Remove from connection request queue, unless already done between
 * our socket unlock, and listening socket lock
 */
        if (listenSocketPtr == socketPtr->socListenSocketPtr)
        {
/*
 * Both locks are needed to prevent race conditions (for tcpsThreadCount,
 * and for socListenSocketPtr) when we call this function.
 */
/* Assume the socket is in the ready to be accepted queue */
            queueIndex = TM_TCP_ACCEPT_QUEUE;
            if (tcpVectPtr->tcpsSynQueue.tcpConReqNextPtr != tcpVectPtr)
            {
/* If in SYN queue instead */
                queueIndex = TM_TCP_SYN_QUEUE;
            }
            tfTcpRemoveConReq(tcpVectPtr, queueIndex, 1);
        }
/*
 * Decrease listening socket ownership count, since our increasing
 * tcpsThreadCount prevented its decrease in both cases of our calling
 * tfTcpRemoveConReq just above, or a thread closing the listening socket
 * and calling it, and therefore resetting the socListenSocketPtr field.
 */
        --(listenSocketPtr->socOwnerCount);
/*
 * Decrease thread count with both socket and listening socket locks on
 * to prevent race conditions.
 * tcpsThreadCount has to be decreased with the listening socket lock on,
 * and before we check whether we need to free the listening socket.
 */
        tcpVectPtr->tcpsThreadCount--;
        tm_call_unlock(&socketPtr->socLockEntry);
/*
 * Check ownership count on listening socket, since we just decreased it,
 * and free it if ownership count dropped to 0.
 */
        if (listenSocketPtr->socOwnerCount == 0)
        {
            tfSocketFree(listenSocketPtr);
            needUnlock = TM_8BIT_ZERO;
        }
        if (needUnlock)
        {
/* Listening socket has not been freed. Unlock it. */
            tm_call_unlock(&(listenSocketPtr)->socLockEntry);
        }
    }
    tm_call_lock_wait(&socketPtr->socLockEntry);
    if (cacheReset != TM_8BIT_ZERO)
    {
        socketPtr->socOwnerCount--;
    }
/* Finish user initiated close (if any) */
    (void)tfTcpCompleteClose(tcpVectPtr);
    return TM_ENOERROR;
}

/*
 * tfTcpSendFin() Function Description
 * 1) For states above SYN_RECEIVED state, try and send a FIN:
 *  since we cannot send the FIN right away if there is any queued
 *  data, we just set a flag to let the tfTcpSendPacket know that a
 *  FIN should be sent with the last packet.
 * 2)  SYN-RECEIVED STATE
 *     RFC793: If no SENDs have been issued and there is no pending data
 *     to send, then form a FIN segment and send it, and enter FIN-WAIT-1
 *     state; otherwise queue for processing after entering ESTABLISHED state.
 * Parameter   description
 * tcpVectPtr  pointer to TCP state vector/socket entry
 * packetPtr   Null pointer
 * newState    new TCP state for state transition
 *
 * Returns
 * Value        Meaning
 * TM_ENOERROR  no error
 */
static int tm_state_function (tfTcpSendFin, tcpVectPtr, packetPtr, newState)
{
    ttSocketEntryPtr  socketPtr;
    tt32Bit           queuedBytes;
    tt8Bit            sendFin;
#ifdef TM_USE_SSL
    int               errorCode;
#endif /* TM_USE_SSL */

    TM_UNREF_IN_ARG(packetPtr);
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    sendFin = TM_8BIT_YES;
    if (    (tcpVectPtr->tcpsState == TM_TCPS_SYN_RECEIVED)
#ifdef TM_USE_SSL_CLIENT
/*
 * On SSL client postpone sending the FIN until SSL is connected if
 * data is queued.
 */
         || (    (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                       TM_SSLF_SEND_ENABLED))
              && (tm_16bit_bits_not_set(tcpVectPtr->tcpsSslFlags,
                                       TM_SSLF_ESTABLISHED) )
              && (tcpVectPtr->tcpsSslSendQBytes != TM_32BIT_ZERO) )
#endif /* TM_USE_SSL_CLIENT */
       )
    {
        queuedBytes = socketPtr->socSendQueueBytes;
#ifdef TM_USE_SSL
        if (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                TM_SSLF_SEND_ENABLED))
        {
            queuedBytes = queuedBytes + tcpVectPtr->tcpsSslSendQBytes;
        }
#endif /* TM_USE_SSL */
        if (queuedBytes != TM_32BIT_ZERO)
        {
/* ANVL-CORE 1.19 */
/*
 * In SYN-RECEIVED state, and no data has been queued for sending/sent,
 * postone sending the FIN until we reach the established state.
 */
            tcpVectPtr->tcpsFlags |= TM_TCPF_WAIT_SND_FIN;
            sendFin = TM_8BIT_ZERO;
        }
    }
    if (sendFin != TM_8BIT_ZERO)
    {
/*
 * Send an immediate FIN, unless (state is TM_TCPS_SYN_RECEIVED and
 * data has been sent/queued for sending (RFC793)).
 */
#ifdef TM_SNMP_MIB
        if (tcpVectPtr->tcpsState != TM_TCPS_SYN_RECEIVED)
        {
/*
 * number of TCP connections for which the current state
 * is either ESTABLISHED or CLOSE-WAIT. We are moving to
 * FIN_WAIT_1 state or LAST_ACK.
 */
            tm_context(tvTcpData).tcpCurrEstab--;
        }
#endif /* TM_SNMP_MIB */
/* Transition to the new state */
        tcpVectPtr->tcpsState = newState;
        tcpVectPtr->tcpsFlags2 |= TM_TCPF2_SND_FIN;
#ifdef TM_USE_SSL
        errorCode = tfTcpSslClose(tcpVectPtr, 1); /* abort on failure */
        if (errorCode == TM_ENOERROR)
#endif /* TM_USE_SSL */
        {
            (void)tfTcpSendPacket(tcpVectPtr);
            tfTcpPostSendFin(tcpVectPtr);
        }
    }
    return TM_ENOERROR;
}

#ifdef TM_USE_SSL
static int tfTcpSslClose(ttTcpVectPtr tcpVectPtr, int flags)
{
    int               errorCode;

    errorCode = TM_ENOERROR;
/* User will no longer be able to process handshake */
    tm_16bit_clr_bit( tcpVectPtr->tcpsSslFlags,
                      TM_SSLF_USER_HANDSHK_PROCESS );
/*
 * Note check on null tcpsSslConnStatePtr not necessary (non null if
 * TM_SSLF_SEND_ENABLED is set
 */
    if (    (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                  TM_SSLF_SEND_ENABLED))
         && (tcpVectPtr->tcpsSslConnStatePtr != (ttVoidPtr)0) )
    {
        errorCode = tfSslClose(tcpVectPtr->tcpsSslConnStatePtr, 0);
        if ( (errorCode != TM_ENOERROR) && (flags != 0))
        {
            (void)tfTcpAbort(tcpVectPtr, errorCode);
        }
    }
    return errorCode;
}
#endif /* TM_USE_SSL */

/*
 * tfTcpReSendFin() Function Description
 * Called in FIN_WAIT_1, CLOSING, and LAST_ACK states, when a retransmission
 * timeout occurs. Retransmit the FIN.
 * Parameter   description
 * tcpVectPtr  pointer to TCP state vector/socket entry
 * packetPtr   Null pointer
 * newState    new TCP state for state transition
 *
 * Returns
 * Value        Meaning
 * TM_ENOERROR  no error
 */
static int tm_state_function (tfTcpReSendFin, tcpVectPtr, packetPtr, newState)
{
    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(newState);

    tcpVectPtr->tcpsFlags2 |= TM_TCPF2_SND_FIN;
    (void)tfTcpSendPacket(tcpVectPtr);
    tfTcpPostSendFin(tcpVectPtr);
    return TM_ENOERROR;
}

/* Post FIN transmission: make sure FIN will be retransmitted */
static void tfTcpPostSendFin (ttTcpVectPtr tcpVectPtr)
{
    if (   (tcpVectPtr->tcpsState == TM_TCPS_FIN_WAIT_1)
         ||(tcpVectPtr->tcpsState == TM_TCPS_LAST_ACK) )
/* If still in FIN_WAIT_1 or LAST_ACK state */
    {
        if (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags2,
                                  TM_TCPF2_REXMIT_MODE))
/* Not in retransmit mode */
        {
            if (    (tcpVectPtr->tcpsSoftError == TM_EHOSTUNREACH)
                 || (tcpVectPtr->tcpsSoftError == TM_ENOBUFS) )
/* Remote is unreachable, or we failed to allocate a buffer */
            {
                tfTcpAbort(tcpVectPtr, tcpVectPtr->tcpsSoftError);
            }
            else
            {
                if (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags3,
                                          TM_TCPF3_PROBE_MODE))
/* not probing */
                {
                    if (tcpVectPtr->tcpsReXmitTmPtr != (ttTimerPtr)0)
                    {
/* Make sure that the FIN gets retransmitted */
#ifdef TM_USE_TCP_REXMIT_CONTROL
                        if (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags3,
                                                  TM_TCPF3_REXMIT_PAUSE))
#endif /* TM_USE_TCP_REXMIT_CONTROL */
                        {
                            tm_timer_new_time( tcpVectPtr->tcpsReXmitTmPtr,
                                               tcpVectPtr->tcpsRto );
                        }
                        tcpVectPtr->tcpsFlags2 |= TM_TCPF2_REXMIT_MODE;
                        tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2,
                                          TM_TCPF2_REINIT_TIMER );
                    }
                }
            }
        }
    }
    return;
}

/*
 * tfTcpCompleteClose() function description:
 * Complete a socket close, making it unaccessible from the user.
 * Called from 5 different possible paths:
 * 1. in FIN_WAIT_2 when retransmission queue is empty.
 * 2. state transition to TIME_WAIT
 * 3. state transition to CLOSED (tfTcpVectClose() is called, and calls
 *    tfTcpCompleteClose()).
 * 4. Close USER event when the state vector is already closed
 *    (tfTcpVectClose() is called, and calls tfTcpCompleteClose()).
 * 5. Close USER event when tfTcpCompleteClose() had already been called
 *    previously in cases 1, 2, or 3 above (as a result of a shutdown(), or
 *    reset), to make sure that the user is notified, and that the TCP
 *    vector/socket entry is removed from the socket array and therefore
 *    freed (TM_SOCF_CLOSING logic below).
 *
 * TM_SOCF_CLOSING on means that the user issued a close call that has
 * not completed yet, so it is ok to finish closing the socket here, and it
 * gives us re-entrancy protection.
 * TM_SOCF_OPEN on means that the socket is still open (in the array).
 *
 * 1) If TM_SOCF_OPEN is set, the socket is still accessible by the user
 *  1.1  we reset the TM_SOCF_CONNECTED, TM_SOCF_CONNECTING flags. We set a
 *       TM_SOCF_DISCONNECTED flag so that we know that this function has been
 *      called.
 *  1.2 If both TM_SOCF_OPEN, and TM_SOCF_CLOSING are set, we complete the user
 *      close:
 *   . we reset the socket options.
 *   . we remove socket entry from the array, i.e reset the TM_SOCF_OPEN flag
 *   . we call the close complete call back function
 *   . If the linger timer pointer is non null, the user has issued
 *     a close, and is waiting: we free the linger timer, and post
 *     on the close thread
 * 2) Else If TM_SOCF_OPEN was not set, that means that the socket had already
 *    completed the user close call, we return a TM_ESHUTDOWN error.
 * 3) If the socket is no longer in the array (i.e. no longer accessible
 *    by the user):
 *   . we flush the socket receive, TCP reassemble queues, SEL ACK blocks.
 *
 * Parameter   description
 * tcpVectPtr  pointer to TCP state vector/socket entry
 * packetPtr   Null pointer
 * newState    new TCP state, unused. No state transition in this function
 *
 * Returns
 * Value        Meaning
 * TM_ENOERROR  no error
 * TM_ESHUTDOWN socket close had already been completed
 */
static int tfTcpCompleteClose (ttTcpVectPtr tcpVectPtr )
{
    ttSocketEntryPtr        socketPtr;
    int                     selectFlags;
    int                     socketCBFlags;
    int                     errorCode;
#ifdef TM_USE_SSL
    ttSslConnectStatePtr    connStatePtr;
#endif /* TM_USE_SSL */

    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
/* No state transition in this routine (done prior) */
    if (tm_16bit_one_bit_set(socketPtr->socFlags, TM_SOCF_OPEN))
    {
/* socket no longer connected/connecting */
         tm_16bit_clr_bit( socketPtr->socFlags, TM_SOCF_CONNECTED );
         tm_8bit_clr_bit( socketPtr->socFlags2, TM_SOCF2_CONNECTING );
/* So that we know that this function has already been called */
         socketPtr->socFlags |= TM_SOCF_DISCONNECTED;
/* Socket is open (in the array) */
        if ( tm_16bit_one_bit_set( socketPtr->socFlags,
                                   TM_SOCF_CLOSING ) )
        {
/*
 * User has issued a close. Complete the close now.
 */
            selectFlags = tcpVectPtr->tcpsSelectFlags;
            socketCBFlags = tcpVectPtr->tcpsSocketCBFlags;
/* Call socket CB function for close complete event, if set by user */
            socketCBFlags |= TM_CB_CLOSE_COMPLT;
/*
 * Do not notify about this state machine earlier recv/send at the end
 * of tfTcpIncomingPacket
 */
            tcpVectPtr->tcpsSelectFlags = 0;
            tcpVectPtr->tcpsSocketCBFlags = 0;
            tfSocketNotify( socketPtr,
                            selectFlags,
                            socketCBFlags,
                            0 );
/*
 * Remove socket entry from the array (will reset TM_SOCF_OPEN and
 * TM_SOCF_CLOSING)
 */
            tfSocketArrayDelete(socketPtr);
#ifdef TM_PEND_POST_NEEDED /* save code space */
/*
 * If the linger timer pointer is non null, the user has issued
 * a close and is waiting for close completion:
 * Free the linger timer, and post on the close/shutdown thread
 */
            if (tcpVectPtr->tcpsLingerTmPtr != TM_TMR_NULL_PTR)
            {
                tm_timer_remove(tcpVectPtr->tcpsLingerTmPtr);
                tcpVectPtr->tcpsLingerTmPtr = TM_TMR_NULL_PTR;
                tm_post(&socketPtr->socClosePendEntry,
                        &socketPtr->socLockEntry,
                        0);
            }
#endif /* TM_PEND_POST_NEEDED */
        }
        errorCode = TM_ENOERROR;
    }
    else
    {
        errorCode = TM_ESHUTDOWN;
    }
    if (tm_16bit_bits_not_set(socketPtr->socFlags, TM_SOCF_OPEN))
/* User no longer has access to the socket */
    {
/*
 * Flush receive, reassemble queues, SEL ACK blocks (in case data arrived
 * on the reassemble queue after the close or read shutdown), or user never
 * did an accept.
 */
        tfTcpShutRead(tcpVectPtr);
#ifdef TM_USE_SSL
        connStatePtr = (ttSslConnectStatePtr)(tcpVectPtr->tcpsSslConnStatePtr);
        if (connStatePtr != (ttSslConnectStatePtr)0)
        {
/* if user doesn't call tfSslConnectUserClose before this call, then
 * we must take care of it. We have to close SSL first
 * because this connection state is also stored in its corresponding
 * SSL session. You have to let the session know that this connection
 * is already closed
 */
            if(tcpVectPtr->tcpsSslFlags & TM_SSLF_ESTABLISHED)
            {
/*
 * only after the SSL state has been established, can it be found
 * in the active connect list
 * note that session may be closed before tfTcpCompleteClose is called and
 * the SSL connection has to be finished so that we can find it in the list
 */
                tfSslConnectNodeRemove(connStatePtr);
            }
/*
 * Free this SSL connection state and clear it from the tcpVectPtr
 */
            tfSslClearConnState(connStatePtr, tcpVectPtr, 0);
        }
#endif /* TM_USE_SSL */
    }
    return errorCode;
}

/*
 * tfTcpUpdateRtt() function description.
 * Given a new round trip time sample (in milliseconds), update the
 * connection scaled smooth round trip time, scaled round trip time
 * variance, and retransmission timeout if computeRtoFlag is on.
 * Reset the retransmit count, remove the connection timeout timer
 * if appropriate.
 *
 * Parameter       description
 * tcpVectPtr      pointer to TCP state vector/socket entry
 * newRtt          new round trip time sample in milliseconds
 * computeRtoFlag  if off, we do not want to recompute the RTO
 *                 (Karn Algorithm)
 *
 * No return value.
 */
static void tfTcpUpdateRtt ( ttTcpVectPtr tcpVectPtr,
                             tt32Bit      newRtt,
                             tt8Bit       computeRtoFlag )
{
    ttS32Bit delta;
    tt32Bit  rto;

    if (computeRtoFlag != TM_8BIT_ZERO)
    {
        if (newRtt == 0)
        {
/*
 * clock granularity (tvTimerTickLength). Assume tvTimerTickLength-1 instead
 */
            newRtt = tvTimerTickLength - 1;
        }
        if (tcpVectPtr->tcpsScSmRtt == 0)
        {
/* Scaled Smooth RTT never computed yet*/
/* Use scaled computed round trip time for initial value */
            tcpVectPtr->tcpsScSmRtt = (ttS32Bit)
                                        (newRtt << TM_TCP_RTT_SCALE_SHIFT);
/* Use scaled half computed round trip time for initial value */
            tcpVectPtr->tcpsScRttVar = (ttS32Bit)
                                  (newRtt << (TM_TCP_RTTVAR_SCALE_SHIFT -1));
        }
        else
        {
/*
 * Difference between computed round trip time and unscaled smooth round trip
 * time
 */
            delta =  (ttS32Bit) (   newRtt
                                  - (    (tt32Bit)tcpVectPtr->tcpsScSmRtt
                                      >> TM_TCP_RTT_SCALE_SHIFT ) );
/* Update Scaled smoothed round trip time: SmRtt = SmRtt + delta/8,
 * or since ScSmRtt == 8*SmRtt: ScSmRtt = ScSmRtt+ delta
 */
            tcpVectPtr->tcpsScSmRtt += delta;
/* Scaled smoothed round trip time cannot be negative or 0 */
            if (tcpVectPtr->tcpsScSmRtt <= 0)
            {
/* REVNOTE: Clock granularity (tvTimerTickLength) */
                tcpVectPtr->tcpsScSmRtt = 1;
            }
/*
 * Update scaled RTT variance:
 * rttvar = rttvar + (|delta| - rttvar)/4 , or since ScRttVar == 4*rttvar:
 * ScRttVar = ScRttVar + |delta| - ScRttVar/4
 */
            if (delta < 0)
            {
                delta = -delta; /* absolute value */
            }
            delta -= (ttS32Bit)(    (tt32Bit)tcpVectPtr->tcpsScRttVar
                                 >> TM_TCP_RTTVAR_SCALE_SHIFT );
            tcpVectPtr->tcpsScRttVar += delta;
/* Scaled Rtt variance cannot be negative or 0 */
            if (tcpVectPtr->tcpsScRttVar <= 0)
            {
/* clock granularity (tvTimerTickLength) */
                tcpVectPtr->tcpsScRttVar = 1;
            }
        }
/*
 * Retransmission timeout is Scaled Smooth round trip time / 8 + Scaled
 * smoothed mean deviation estimator
 */
        rto = tm_rto(tcpVectPtr->tcpsScSmRtt, tcpVectPtr->tcpsScRttVar);
        tm_bound(rto, tcpVectPtr->tcpsRtoMin, tcpVectPtr->tcpsRtoMax);
        tcpVectPtr->tcpsRto = (tt32Bit)rto;
    }
/* Reset retransmission count */
    tcpVectPtr->tcpsReXmitCnt = 0;
    if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_MAXRT))
    {
        tm_tcp_maxrt_suspend(tcpVectPtr);
    }
/* Discard any ICMP error message, since we have received an ACK */
    tcpVectPtr->tcpsSoftError = 0;
    return;
}

/*
 * tfTcpOpen() Function Description
 * Called from connect, or listen.
 * 1. Double check that the socketEntry has not been used for a previous
 *    connection. (The user has to close the socket and open a new one
 *    prior to re-connecting.)
 * 2. If no error, call state machine error = tfTcpStateMachine(tcpVector,
 *    TM_PACKET_NULL_PTR, event); tfTcpStateMachine
 *    will store next state for both active and passive open.
 *    . For an active open tfTcpSendSyn() is called. It will insert the
 *      socket in the connection-oriented tree, send a SYN and start a timer.
 *    . For a passive open tfTcpListen() is called. It will set the
 *      flag.
 * 3. If socket is blocking wait for establishement of the connection,
 *    otherwise return TM_EINPROGRESS.
 * 4. Check whether the associated TCP vector state did not transition to the
 *    closed state (i.e. whether the connection did not get reset when calling
 *    connect() in loop back mode.)
 * 5. return error to the user.
 *
 * Parameter         description
 *
 * socketPtr    Pointer to socket entry
 * event             Indicate passive/active open
 *
 * Return value
 * TM_ENOERROR       no error
 * TM_EINPROGRESS    if connect is non blocking, and connection has not
 *                   been established yet.
 * TM_ESHUTDOWN      if trying to connect on a shutdown connection
 * TM_EADDRINUSE     User is trying to connect/listen on a socket that has
 *                   been used for a prior connection.
 */
int tfTcpOpen (ttTcpVectPtr tcpVectPtr, tt8Bit event)
{
    ttSocketEntryPtr    socketPtr;
    int                 errorCode;
#ifdef TM_USE_SOCKET_IMPORT
    ttUserTcpConPtr     userTcpConPtr;
#endif /* TM_USE_SOCKET_IMPORT */

    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
#ifdef TM_USE_SOCKET_IMPORT
    userTcpConPtr = tcpVectPtr->tcpsUserTcpConPtr;
#endif /* TM_USE_SOCKET_IMPORT */
/*
 * Double check that the socketEntry has not been used for a previous
 * connection. (The user has to close the socket and open a new one
 * prior to re-connecting.)
 */
    if ( tm_16bit_one_bit_set( socketPtr->socFlags,
                               TM_SOCF_NO_MORE_SEND_DATA ) )
    {
/* State transition to closed from a previous connection. */
        errorCode = TM_ESHUTDOWN;
    }
    else
    {
/*
 * Call State machine. Keep socket locked for both passive open and
 * active open.
 */
        errorCode = tfTcpStateMachine( tcpVectPtr,
                                       TM_PACKET_NULL_PTR,
                                       event );
        if (errorCode == TM_ENOERROR)
        {
/* If user issued a connect, try and wait for connection establishment */
            if (event == TM_TCPE_ACTIVE_OPEN)
            {
#ifdef TM_USE_SOCKET_IMPORT
/* Allow wait if this socket is not being created as an imported socket */
                if (userTcpConPtr == (ttUserTcpConPtr)0)
#endif /* TM_USE_SOCKET_IMPORT */
                {
                    while (    (tcpVectPtr->tcpsState == TM_TCPS_SYN_SENT)
                            && (errorCode == TM_ENOERROR) )
                    {
/* and socket is blocking, wait for established state */
                        if (tm_16bit_bits_not_set(socketPtr->socFlags,
                                                  TM_SOCF_NONBLOCKING))
                        {
                            errorCode = tm_pend(&socketPtr->socSendPendEntry,
                                                &socketPtr->socLockEntry);
                        }
/*
 * if socket non blocking let the user know that connect has not completed
 * yet
 */
                        else
                        {
                            errorCode = TM_EINPROGRESS;
                        }
                    }
                }
/*
 * If TCP transitionned to the close state, check for error on socket.
 * This could happen when a loop back connect fails (connection refused).
 */
                if ( tm_16bit_one_bit_set( socketPtr->socFlags,
                                           TM_SOCF_NO_MORE_SEND_DATA ) )
                {
                    errorCode = socketPtr->socSendError;
                    if (errorCode != TM_ENOERROR)
                    {
                        socketPtr->socSendError = TM_ENOERROR;
                    }
                }
            }
        }
    }
    return errorCode;
}

/*
 * tfTcpRecv() Function Description
 * Called by the socket recv calls, when no data is available in
 * the receive queue.
 * Call the state machine to find out whether a USER receive call
 * is allowed given the state of the connection.
 *
 * Parameter        description
 * socketPtr   Pointer to socket entry
 *
 * Return value:
 * TM_ENOERROR      User receive call is allowed
 * TM_ENOTCONN      connection establishment has not started yet
 * TM_ESHUTDOWN     connection is shutdown for read
 */
int tfTcpRecv (ttTcpVectPtr tcpVectPtr)
{
    int errorCode;

    errorCode = tfTcpStateMachine(tcpVectPtr, TM_PACKET_NULL_PTR,
                                  TM_TCPE_RCV_USER);
    if (errorCode == TM_ENOERROR)
    {
        if tm_16bit_one_bit_set( tcpVectPtr->tcpsSocketEntry.socFlags,
                                 TM_SOCF_NO_MORE_RECV_DATA )
        {
            errorCode = TM_ESHUTDOWN;
        }
    }
    return errorCode;
}

/*
 * tfTcpRecvCmplt() Function Description
 * Called by the socket recv calls when a receive has completed, or
 * when a receive call back buffer has been freed to let the TCP code
 * knows that some bytes have been received by the user.
 * We update the receive queue size by the number of bytes received by
 * the user, and update the socket out of band mark.
 * We then check whether we need to/can send more data. If the user
 * does not want delay ACK, we send/piggyback a window update.
 *
 * Parameter         description
 * socketPtr    Pointer to socket entry
 * copyCount         number of bytes received by the user
 *
 * No return value:
 */
void tfTcpRecvCmplt (ttTcpVectPtr tcpVectPtr, ttPktLen copyCount, int flags)
{
    ttSocketEntryPtr        socketPtr;

    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
/* Update receive queue size */
    socketPtr->socRecvQueueBytes -= (tt32Bit)copyCount;
    if (socketPtr->socOobMark != TM_UL(0))
    {
/* Update out of band mark */
        socketPtr->socOobMark -= (tt32Bit)copyCount;
        if (socketPtr->socOobMark == TM_UL(0))
        {
            socketPtr->socFlags |= TM_SOCF_RCVATMARK;
        }
    }
    else
    {
        tm_16bit_clr_bit( socketPtr->socFlags, TM_SOCF_RCVATMARK );
    }
    if (    (flags == 0)
         && (    (tcpVectPtr->tcpsState == TM_TCPS_ESTABLISHED)
              || (tcpVectPtr->tcpsState == TM_TCPS_FIN_WAIT_1)
              || (tcpVectPtr->tcpsState == TM_TCPS_FIN_WAIT_2)) )
    {
/*
 * Only send a window update if the peer can send more data (i.e.
 * peer did not send a FIN.)
 */
        (void)tfTcpSendPacket(tcpVectPtr);
    }
    return;
}

/*
 * tfTcpSendData() Function Description
 * Called by the retransmission timer (through the state machine), when
 * sending data is allowed for the state of the connection.
 * Call tfTcpSendPacket() to retransmit some data.
 *
 * Parameter      description
 * tcpVectPtr     pointer to TCP state vector/socket entry
 * packetPtr      Null pointer
 * newState       new TCP state, unused. No state transition in this function
 */
static int tm_state_function (tfTcpSendData, tcpVectPtr, packetPtr, newState)
{
    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(newState);
    return tfTcpSendPacket(tcpVectPtr);
}

/*
 * tfTcpRcvSynOptions() function description:
 * Process received SYN options (from the peer). Initialize receive sequence
 * numbers. Also check routing entry and set MSS from peer. Initialize
 * congestion window, and slow start threshold.
 * Parameter     description
 * packetPtr     pointer to incoming data packet pointer (as prepared in
 *               tfTcpIncomingPacket step 10).
 * tcpsFlags     our tcp options (what we allow)
 * tcpVectPtr    Pointer to targeted TCP state vector (including socket entry)
 *
 * Return values
 * return error value if any, TM_ENOERROR otherwise.
 */
static int tfTcpRcvSynOptions (ttPacketPtr  packetPtr,
                               tt16Bit      tcpsFlags,
                               ttTcpVectPtr tcpVectPtr)
{
    ttTcpHeaderPtr tcphPtr;
    tt8BitPtr      tcpOptionPtr;
    int            tcpOptionLen;
    int            optionLen;
    int            errorCode;
    tt16Bit        tcpHdrLen;
    tt16Bit        mss;
    tt8Bit         option;
#ifdef TM_DSP
    int            bytePosition;
#ifdef TM_TCP_SACK
    int            i;
#endif /* TM_TCP_SACK */
#endif /* TM_DSP */

    tcphPtr = tm_tcp_hdr_ptr(packetPtr);
    tcpHdrLen = packetPtr->pktTcpHdrLen;
/* Reset TCP options to be set by peer (assume peer has not set them) */
    tcpVectPtr->tcpsFlags = tcpsFlags;
    tm_16bit_clr_bit( tcpVectPtr->tcpsFlags, TM_TCPF_OPTIONS );
/* Assume minimum TCP header size */
    tcpVectPtr->tcpsHeaderSize = TM_BYTES_TCP_HDR;
#ifdef TM_TCP_SACK
    tcpVectPtr->tcpsMaxSackSqns = (tt8Bit)TM_TCP_SACK_SQNS;
#endif /* TM_TCP_SACK */
    if (tcpHdrLen > (tt16Bit)TM_BYTES_TCP_HDR)
/*
 * Options are present. Note that there is no guarantee on word alignment
 * for options when a TCP header come from a peer. This is the reason
 * we copy one byte at a time.
 */
    {
        tcpOptionPtr = (tt8BitPtr)tcphPtr + TM_PAK_TCP_HDR_LEN;
        tcpOptionLen = (int)tcpHdrLen - TM_BYTES_TCP_HDR;

#ifdef TM_DSP
        bytePosition = (TM_DSP_BYTES_PER_WORD - 1);
#endif /* TM_DSP */

        while (tcpOptionLen > 0)
        {
/* Get option field */
            tm_tcp_get_option(option, tcpOptionPtr, 0, bytePosition);

            if (option == TM_TCPO_EOL)
                break;
            if (option == TM_TCPO_NOOP)
            {
                optionLen = 1;
                goto tcpSynOptLoopEnd;
            }
/* Get option length */
            tm_tcp_get_option(optionLen, tcpOptionPtr, 1, bytePosition);

            if (optionLen <= 0)
                break;
            switch (option)
            {
                case TM_TCPO_MSS:
                    if (optionLen != TM_TCPO_MSS_LENGTH)
                    {
/* move to next option, continue */
                        goto tcpSynOptLoopEnd;
                    }
/* Copy mss in host byte order */
#ifdef TM_DSP
/* Get high order byte of MSS */
                    tm_tcp_get_option(mss, tcpOptionPtr, 0, bytePosition);
                    mss = (mss << 8) & 0xff00;

/* Get lower order byte of MSS  and OR with high order byte */
                    tm_tcp_get_option(option, tcpOptionPtr, 0, bytePosition);
                    mss |= option;

#else /* !TM_DSP */
                    mss = TM_16BIT_ZERO; /* to avoid compiler warning */
                    tm_endian_short_copy(tcpOptionPtr, mss, 2);
#endif /* TM_DSP */
                    tcpVectPtr->tcpsPeerMss = mss;
                    break;
                case TM_TCPO_WNDSC:
                    if (optionLen != TM_TCPO_WNDSC_LENGTH)
                    {
/* move to next option, continue */
                        goto tcpSynOptLoopEnd;
                    }
                    if (tm_16bit_one_bit_set(tcpsFlags, TM_TCPF_WND_SCALE))
                    {
                        tcpVectPtr->tcpsFlags |= TM_TCPF_WND_SCALE;
                        tm_tcp_get_option(option,
                                          tcpOptionPtr,
                                          2,
                                          bytePosition);

                        if (option > TM_TCP_MAX_WNDSC)
                        {
                            option = TM_TCP_MAX_WNDSC;
                        }
                        tcpVectPtr->tcpsSndWndScale = option;
                    }
                    break;
#ifdef TM_TCP_SACK
                case TM_TCPO_SACK_PERMIT:
                    if (optionLen != TM_TCPO_SACKP_LENGTH)
                    {
/* move to next option, continue */
                        goto tcpSynOptLoopEnd;
                    }
                    if (tm_16bit_one_bit_set(tcpsFlags, TM_TCPF_SEL_ACK))
                    {
                        tcpVectPtr->tcpsFlags |= TM_TCPF_SEL_ACK;
                    }
#ifdef TM_DSP
                    for (i = 0; i < TM_TCPO_SACKP_LENGTH - 2; i++) {
                        if (--bytePosition < 0) {
                            bytePosition = (TM_DSP_BYTES_PER_WORD - 1);
                            tcpOptionPtr++;
                        }
                    }
#endif /* TM_DSP */
                    break;
#endif /* TM_TCP_SACK */
                case TM_TCPO_TS:
                    if (optionLen != TM_TCPO_TS_LENGTH)
                    {
/* move to next option, continue */
                        goto tcpSynOptLoopEnd;
                    }
                    if (tm_16bit_one_bit_set(tcpsFlags, TM_TCPF_TS))
                    {
                        tcpVectPtr->tcpsFlags |= TM_TCPF_TS;
/* Tcp header with time stamp option on every packet (except reset)*/
                        tcpVectPtr->tcpsHeaderSize += TM_TCPO_TS_NOOP_LENGTH;
#ifdef TM_TCP_SACK
/* Maximum number of SQN SEL ACK */
                        tcpVectPtr->tcpsMaxSackSqns =
                                              (tt8Bit)TM_TCP_TS_SACK_SQNS;
#endif /* TM_TCP_SACK */
/* Collect most recent time value from peer*/
                        tm_endian_long_copy( tcpOptionPtr,
                                             tcpVectPtr->tcpsTsRecent,
                                             2,
                                             bytePosition );
                        tm_kernel_set_critical;
/* Time of arrival of time value from peer */
                        tcpVectPtr->tcpsTsRecentAge = tvTime;
                        tm_kernel_release_critical;
/* Collect time stamp echo reply (our time value echoed by the remote) */
                        packetPtr->pktSharedDataPtr->dataFlags |=
                                                            TM_BUF_TCP_TS_ON;
                        tm_endian_long_copy( tcpOptionPtr,
                                             packetPtr->pktTcpTsEcr,
                                             6,
                                             bytePosition );
                    }
                    break;
                default:
                    break;
            }

tcpSynOptLoopEnd:
            tcpOptionLen -= optionLen;

#ifndef TM_DSP
            tcpOptionPtr += optionLen;
#endif /* TM_DSP */

        }
    }
/* Set MSS, receive window and send window to multiple of (MSS - options) */
    errorCode = tfTcpSetMssWnd(tcpVectPtr);
/* Initialize receive sequence numbers */
    tcpVectPtr->tcpsIrs = tcphPtr->tcpSeqNo;
    tm_tcp_init_rcv_sqn(tcpVectPtr);
    tcpVectPtr->tcpsSndWnd = packetPtr->pktTcpUnscaledWnd;
/*
 * RFC 2001 (congestion avoidance):
 *   1.  Initialization for a given connection sets cwnd to one segment
 *      and ssthresh to 65535 bytes.
 * RFC 2581 update:
 * 1. IW (i.e. Initial Congestion Window) set to 2 MSS.
 * 2. The initial value of ssthresh MAY be arbitrarily high (for example,
 *    some implementations use the size of the advertised window), but it
 *    may be reduced in response to congestion.
 */
    if (tm_16bit_all_bits_set(tcpVectPtr->tcpsFlags, TM_TCPF_INITIAL_WINDOW))
    {
/*
 * If user did not turn off slow start and did not def out full initial window
 * on direct routes
 */
        tcpVectPtr->tcpsCwnd = tm_tcp_iw(tcpVectPtr);
    }
    else
    {
/* If either network is local or user turned off slow start */
        tcpVectPtr->tcpsCwnd = TM_TCP_MAX_SCALED_WND << TM_TCP_MAX_WNDSC;
    }
/* Maximum possible send window (with Window scale added to RFC 2001) */
    tcpVectPtr->tcpsSsthresh = TM_TCP_MAX_SCALED_WND << TM_TCP_MAX_WNDSC;
    return errorCode;
}

/*
 * tfTcpSendSyn function description:
 * Called from the TCP state machine when an ACTIVE OPEN event occurs.
 * Send a SYN segment:
 * 1. Check that we have a routing entry and that address is valid
 *    for TCP. Set the local IP address, if user had not bound the
 *    socket to an IP address.
 * 2. If no error, try and re-insert the socket as a connection-oriented
 *    socket, so that it can be accessed by tfSocketLookup, when incoming data
 *    comes from the network:
 *      . If we cannot insert as a connection-oriented socket:
 *       .. Try and lookup for the duplicate socket, and check if it is
 *          in time wait state. If it is, get its sequence number, and
 *          close it.
 *       .. Try to re-insert as a connection-oriented socket.
 * 3. If we re-inserted the socket in the tree successfully:
 *   . Transition to the new state (SYN_SENT)
 *   . Select initial MSS
 *   . An inititial send sequence number is selected (tcpsIss), if
 *     not done so before.
 *   . All send sequence numbers are initialized to tcpsIss
 *   . Add timers for the connection request
 *   . A syn segment of the form SEQ=ISS CTL=SYN with options is sent.
 * Parameter     description
 * tcpVectPtr    Pointer to TCP state vector (including socket entry)
 * packetPtr     Null pointer.
 * newState      new TCP state to transition to.
 *
 * Return values
 * return error value if any, TM_ENOERROR otherwise.
 */
static int tm_state_function (tfTcpSendSyn, tcpVectPtr, packetPtr, newState)
{
    ttRteEntryPtr      rtePtr;
    ttRtCacheEntryPtr  rtcPtr;
    ttDeviceEntryPtr   devPtr;
    ttSocketEntryPtr   socketPtr;
    ttTcpTmWtVectPtr   tcpTmWtVectPtr;
    ttTupleDev         tupleDev;
#ifdef TM_USE_IPV6
    tt6IpAddressPtr    ipv6OurAddrPtr;
    tt6IpAddressPtr    ipv6PeerAddrPtr;
    ttDeviceEntryPtr   srcDevPtr;
    int                addScopeId;
#endif /* TM_USE_IPV6 */
#ifdef TM_4_USE_SCOPE_ID
    int                addV4MappedScopeId;
#endif /* TM_4_USE_SCOPE_ID */
    ttSqn              newIss;
    int                errorCode;
    int                retCode;
    tt16Bit            reuseFlag;
#if defined(TM_USE_IPV6) && defined (TM_USE_IPV4)
    tt8Bit             initializedIPv4Address;
    tt8Bit             checkPeerIpv4Address;
#endif /* dual */
#ifdef TM_USE_SOCKET_IMPORT
    ttUserTcpConPtr    userTcpConPtr;
#endif /* TM_USE_SOCKET_IMPORT */

    errorCode = TM_ENOERROR;
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
#ifdef TM_USE_SOCKET_IMPORT
    userTcpConPtr = tcpVectPtr->tcpsUserTcpConPtr;
#endif /* TM_USE_SOCKET_IMPORT */
#ifdef TM_USE_IPV6
    tm_bzero(&tupleDev, sizeof(tupleDev));
#endif /* TM_USE_IPV6 */
/* Local port */
    tupleDev.tudTuple.sotLocalIpPort = socketPtr->socOurLayer4Port;
/* Copy remote port from user area */
    tupleDev.tudTuple.sotRemoteIpPort =
                             tcpVectPtr->tcpsSockAddrInPtr->sin_port;
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
    initializedIPv4Address = TM_8BIT_ZERO;
    checkPeerIpv4Address = TM_8BIT_ZERO;
#endif /* dual */
    addScopeId = 0;
#ifdef TM_4_USE_SCOPE_ID
    addV4MappedScopeId = 0;
#endif /* TM_4_USE_SCOPE_ID */
#ifdef TM_USE_IPV4
    if (socketPtr->socProtocolFamily == AF_INET6)
#endif /* dual */
    {
/* AF_INET6 */
/*
 *  User passed parameters. Point to user area.
 */
        ipv6PeerAddrPtr = &(tcpVectPtr->tcpsSockAddrIn6Ptr->sin6_addr);
        tm_6_ip_copy_dststruct(ipv6PeerAddrPtr,
                               tupleDev.tudTuple.sotRemoteIpAddress);
        ipv6PeerAddrPtr = &tupleDev.tudTuple.sotRemoteIpAddress;
        if (tm_6_addr_is_ipv4_mapped(ipv6PeerAddrPtr))
        {
#ifdef TM_4_USE_SCOPE_ID
            addV4MappedScopeId =
                IN4_IS_ADDR_LINKLOCAL(ipv6PeerAddrPtr->s6LAddr[3]);
#endif /* TM_4_USE_SCOPE_ID */
#ifdef TM_USE_IPV4 /* dual */
            if (    tm_6_ip_not_zero(&(socketPtr->socOurIfIpAddress))
                 && !tm_6_addr_is_ipv4_mapped((&socketPtr->socOurIfIpAddress)))
/* Non zero IPv6 source address */
            {
                if (tm_6_ip_match((&socketPtr->socOurIfIpAddress),
                                  (&in6addr_loopback)) )
                {
/* If source IP address is IPv6 loopback, change to IPv4 loopback */
                    initializedIPv4Address = TM_8BIT_YES;
                    tm_ip_copy(
                        tm_const_htonl(TM_UL(0x7F000001)),
                        tm_4_ip_addr(tupleDev.tudTuple.sotLocalIpAddress) );
                }
                else
                {
/* Don't allow connection from IPv6 to IPv4 addresses */
                    errorCode = TM_EHOSTUNREACH;
                }
            }
            checkPeerIpv4Address = TM_8BIT_YES;
#else /* IPv6 only */
            errorCode = TM_EHOSTUNREACH;
#endif /* IPv6 only */
        }
        else
        {
            addScopeId = IN6_IS_ADDR_LINKLOCAL(ipv6PeerAddrPtr);
/* Save the flow label and traffic class values from the user */
            socketPtr->soc6FlowInfo =
                        tcpVectPtr->tcpsSockAddrIn6Ptr->sin6_flowinfo;
        }
    }
#ifdef TM_USE_IPV4
    else
#endif /* dual */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
    {
/* AF_INET */
/*
 *  User passed parameters. Copy from user area.
 */
/* Copy from user area */
        tm_ip_copy( (tt4IpAddress)
                    (tcpVectPtr->tcpsSockAddrInPtr->sin_addr.s_addr),
                    tm_4_ip_addr(tupleDev.tudTuple.sotRemoteIpAddress));
#ifdef TM_USE_IPV6
        checkPeerIpv4Address = TM_8BIT_YES;
        ipv6PeerAddrPtr = &(tupleDev.tudTuple.sotRemoteIpAddress);
        tm_6_addr_add_ipv4_map_field(ipv6PeerAddrPtr);
#endif /* dual */
    }
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
    ipv6OurAddrPtr = &(tupleDev.tudTuple.sotLocalIpAddress);
#ifdef TM_USE_IPV4
    if (checkPeerIpv4Address == TM_8BIT_ZERO)
#endif /* dual */
/* IPv6 address */
    {
/* Check validity of destination address */
        if (    (IN6_IS_ADDR_MULTICAST(ipv6PeerAddrPtr))
             || (tm_6_ip_zero(ipv6PeerAddrPtr)) )
        {
            errorCode = TM_EADDRNOTAVAIL;
        }
        else
        {
            tcpVectPtr->tcpsNetworkLayer = TM_NETWORK_IPV6_LAYER;
        }
    }
#ifdef TM_USE_IPV4
    else
#endif /* dual */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
    {
/* Check validity of destination address */
        if (    (tm_ip_match(tm_4_ip_addr(tupleDev.tudTuple.sotRemoteIpAddress),
                             TM_IP_LIMITED_BROADCAST))
             || (tm_ip_is_multicast(
                         tm_4_ip_addr(tupleDev.tudTuple.sotRemoteIpAddress)))
             || (tm_ip_zero(
                         tm_4_ip_addr(tupleDev.tudTuple.sotRemoteIpAddress)))
           )
        {
            errorCode = TM_EADDRNOTAVAIL;
        }
#ifdef TM_USE_IPV6
        else
        {
            tcpVectPtr->tcpsNetworkLayer = TM_NETWORK_IPV4_LAYER;
        }
#endif /* dual */
    }
#endif /* TM_USE_IPV4 */
/* Get info from routing table */
    if (errorCode == TM_ENOERROR)
    {
#ifdef TM_4_USE_SCOPE_ID
        if (addV4MappedScopeId)
        {
            tm_4_embed_scope_id( ipv6PeerAddrPtr,
                                 tcpVectPtr->tcpsSockAddrIn6Ptr->sin6_scope_id);
        }
#endif /* TM_4_USE_SCOPE_ID */
#ifdef TM_USE_IPV6
#ifndef TM_SINGLE_INTERFACE
/* Add scope if needed */
        if (addScopeId)
        {
            tm_6_embed_scope_id( ipv6PeerAddrPtr,
                                 tcpVectPtr->tcpsSockAddrIn6Ptr->sin6_scope_id);
        }
#endif /* TM_SINGLE_INTERFACE */
/* Point to ipv6PeerAddrPtr */
        tcpVectPtr->tcpsSockAddrPeer6Ptr = ipv6PeerAddrPtr;
#endif /* TM_USE_IPV6 */
        errorCode = tfTcpSetRtc(tcpVectPtr);
    }
    if (errorCode == TM_ENOERROR)
    {
        rtcPtr = &(socketPtr->socRteCacheStruct);
        rtePtr = rtcPtr->rtcRtePtr;
        devPtr = rtePtr->rteOrigDevPtr;
#ifdef TM_USE_IPV4
/* Do not allow connection to a directed broadcast (only when we connect) */
#ifdef TM_USE_IPV6
        if (checkPeerIpv4Address == TM_8BIT_YES)
#endif /* dual */
        {
#ifdef TM_USE_IPV6
            if (initializedIPv4Address == TM_8BIT_ZERO)
#endif /* dual */
            {
                tm_ip_copy(tm_4_ip_addr(socketPtr->socOurIfIpAddress),
                           tm_4_ip_addr(tupleDev.tudTuple.sotLocalIpAddress));
            }
            if (   (tm_4_ll_is_lan(devPtr))
                && (tm_ip_match(
                        tm_4_ip_addr(tupleDev.tudTuple.sotRemoteIpAddress),
                        tm_ip_dev_dbroad_addr(devPtr,
                                              rtePtr->rteOrigMhomeIndex))) )
            {
                tfRtCacheUnGet(rtcPtr);
                errorCode = TM_EADDRNOTAVAIL;
            }
            else
            {
/*
 * Initialize source Ip address from outgoing interface
 * unless the socket was set by the user to another IP address.
 */
                if (     (tm_ip_zero(tm_4_ip_addr(
                                        tupleDev.tudTuple.sotLocalIpAddress)))
                     || (   (tm_16bit_bits_not_set( socketPtr->socFlags,
                                                   TM_SOCF_BIND )
                            )
/*
 * BUG ID 203: if socket source IP address has been assigned by calling
 * setsockopt with option IPO_SRCADDR, we should use the assigned IP
 * address.
 */
#ifdef TM_USE_STRONG_ESL
                         && (tm_8bit_bits_not_set( socketPtr->socFlags2,
                                                    TM_SOCF2_IP_SRCADDR
                                                  | TM_SOCF2_BINDTODEVICE
                                                 )
                          )
#else /* !TM_USE_STRONG_ESL */
                         && (tm_8bit_bits_not_set( socketPtr->socFlags2,
                                                    TM_SOCF2_IP_SRCADDR
                                                 )
                          )
#endif /* TM_USE_STRONG_ESL */
                       )
                   )
/*
 * BUG ID 11-670: On the loop back interface, only initialize source IP address
 * with peer Ip address if the socket was not set by the user to another IP
 * address.
 */
                {
                    if (devPtr == tm_context(tvLoopbackDevPtr))
                    {
                        tm_ip_copy(
                            tm_4_ip_addr(tupleDev.tudTuple.sotRemoteIpAddress),
                            tm_4_ip_addr(tupleDev.tudTuple.sotLocalIpAddress));
                    }
                    else
                    {
                        tm_ip_copy(
                            tm_ip_dev_addr(devPtr, rtePtr->rteOrigMhomeIndex),
                            tm_4_ip_addr(tupleDev.tudTuple.sotLocalIpAddress));
                    }
                }
#ifdef TM_USE_IPV6
                tm_6_addr_add_ipv4_map_field(ipv6OurAddrPtr);
#ifdef TM_4_USE_SCOPE_ID
                tm_4_dev_scope_addr(ipv6OurAddrPtr, devPtr);
#endif /* TM_4_USE_SCOPE_ID */
#endif /* dual */
            }
        }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
        else
#endif /* dual */
        {
/*
 * Initialize source Ip address from outgoing interface
 * unless the socket was set by the user to another IP address.
 */
            if ( tm_6_ip_zero(&(socketPtr->socOurIfIpAddress)) )
            {
                errorCode = tf6SelectSourceAddress( devPtr,
                           ipv6OurAddrPtr,
                           &(rtcPtr->rtcSrcDevPtr),
                           &(rtcPtr->rtcSrcMhomeIndex),
                           ipv6PeerAddrPtr,
                           &(rtcPtr->rtcDestIpAddrType),
                           TM_6_EMBED_SCOPE_ID_FLAG);
                if (   (errorCode == TM_ENOERROR)
                    && (!IN6_IS_ADDR_LINKLOCAL(ipv6OurAddrPtr)))
                {
                    socketPtr->socRteCacheStruct.rtcSrcMhomeSet = TM_8BIT_YES;
                }
            }
            else
            {
                tm_6_ip_copy(&(socketPtr->socOurIfIpAddress), ipv6OurAddrPtr);
                if (!IN6_IS_ADDR_LINKLOCAL(ipv6OurAddrPtr))
                {
                    srcDevPtr = tf6IfaceMatch(
                            ipv6OurAddrPtr,
                            tf6MhomeAddrMatch,
                            &(socketPtr->socRteCacheStruct.rtcSrcMhomeIndex));
                    if (srcDevPtr != (ttDeviceEntryPtr)0)
                    {
                        socketPtr->socRteCacheStruct.rtcSrcMhomeSet =
                                                             TM_8BIT_YES;
                        socketPtr->socRteCacheStruct.rtcSrcDevPtr = srcDevPtr;
                    }
                }
            }
        }
#endif /* TM_USE_IPV6 */
/* Reset pointer to user data */
        tcpVectPtr->tcpsSockAddrUPtr = (ttSockAddrPtrUnionPtr)0;
        if (errorCode == TM_ENOERROR)
        {
            if (tm_16bit_one_bit_set(socketPtr->socFlags,
                                     TM_SOCF_IN_TREE))
            {
/* Tree flag OR'ed with so reuse addr option */
                reuseFlag = (tt16Bit)
                          (   (socketPtr->socOptions & SO_REUSEADDR)
                            | TM_SOC_TREE_CO_FLAG );
            }
            else
            {
/* Pick random port */
                reuseFlag = (tt16Bit)(  TM_SOC_TREE_CO_FLAG
                                      | TM_SOC_TREE_RANDOM_PORT);
            }
/* Insert/Re-insert as a connection-oriented socket. */
            retCode = tfSocketTreeReInsert(socketPtr,
                                           &(tupleDev.tudTuple),
                                           reuseFlag );
            if (retCode != TM_SOC_RB_OKAY)
            {
                errorCode = TM_EADDRINUSE;
            }
            else
            {
/* Socket successfully inserted in the connection-oriented tree. */
                if (tm_16bit_one_bit_set(socketPtr->socFlags,
                                         TM_SOCF_NO_MORE_SEND_DATA))
                {
                    errorCode = TM_ESHUTDOWN;
                }
                else
                {
                    errorCode = TM_ENOERROR;
/*
 * check for a previous socket in the TCP time wait state
 */
/* Lock */
                    tm_call_lock_wait(&tm_context(tvTcpTmWtTableLock));
                    if (tm_context(tvTcpTmWtTableMembers) != 0)
                    {
#ifdef TM_USE_STRONG_ESL
                        tupleDev.tudDevPtr = devPtr;
#endif /* TM_USE_STRONG_ESL */
                        tcpTmWtVectPtr = tfTcpTmWtLookup(&(tupleDev));
                    }
                    else
                    {
                        tcpTmWtVectPtr = (ttTcpTmWtVectPtr)0;
                    }
                    if (tcpTmWtVectPtr != (ttTcpTmWtVectPtr)0)
/* If there is a TCP vector in time wait state: */
                    {
/* get its sequence number and close the vector. */
                        newIss = tfTcpTmWtGetIssClose(tcpTmWtVectPtr);
                        tcpVectPtr->tcpsIss = newIss;
                    }
                    else
                    {
                        tfTcpGetIss(tcpVectPtr);
                    }
#ifdef TM_USE_SOCKET_IMPORT
/* Use the ISS from the imported socket */
                    if (userTcpConPtr != (ttUserTcpConPtr)0)
                    {
                        tcpVectPtr->tcpsIss = userTcpConPtr->uconIss;
                    }
#endif /* TM_USE_SOCKET_IMPORT */
/* Unlock */
                    tm_call_unlock(&tm_context(tvTcpTmWtTableLock));
#if defined(TM_USE_IPV6) && defined (TM_USE_IPV4)
                    if (initializedIPv4Address != TM_8BIT_ZERO)
                    {
/* Change the socket to an IPv4 socket */
                        socketPtr->socProtocolFamily = AF_INET;
                    }
#endif /* dual */
                    tcpVectPtr->tcpsState = newState;
                    socketPtr->socFlags2 |= TM_SOCF2_CONNECTING;
#ifdef TM_SNMP_MIB
/*
 * Number of times TCP connections have made a direct
 * transition to the SYN-SENT state from the CLOSED state.
 */
                    tm_context(tvTcpData).tcpActiveOpens++;
#endif /* TM_SNMP_MIB */
/*
 * Get the MSS from the interface, and modify socMaxRecvQueueBytes,
 * and socMaxSendQueueBytes to round them up to a multiple of MSS
 * (Only when we connect)
 */
                    (void)tfTcpSetMssWnd(tcpVectPtr);
/*
 * Get a brand new initial sequence number (unless we already got one from
 * a time wait state TCP vector).
 */
/* Initialize connection send variables, and timeout timers */
                    tfTcpInitConTmr(tcpVectPtr);
#ifdef TM_USE_IPDUAL
#ifdef TM_USE_IPSEC
                    if (rtePtr->rteDevPtr != tm_context(tv6Ipv4DefGwTunnelPtr))
/* Do not allow recycling of tcp headers on IPv6 over IPv4 tunnels. */
#endif /* TM_USE_IPSEC */
#endif /* TM_USE_IPDUAL */
                    {
                        tcpVectPtr->tcpsFlags |=
                            TM_TCPF_RECYCLE_HDR | TM_TCPF_SOCKETENTRYPTR;
                    }
#ifdef TM_USE_SOCKET_IMPORT
                    if (userTcpConPtr != (ttUserTcpConPtr)0)
                    {
/* Socket is being created in the established state.
 * Pretend we just sent a packet to avoid starting with a reduced
 * window size. */
                        tm_kernel_set_critical;
                        tcpVectPtr->tcpsLastXmitTime = tvTime;
                        tm_kernel_release_critical;
                    }
                    else
#endif /* TM_USE_SOCKET_IMPORT */
                    {
/*
 * Generate options and send packet
 * ignore error from tfTcpCreateSendPacket, and let the retransmit logic do
 * its work.
 */
                        errorCode = tfTcpCreateSendPacket(tcpVectPtr,
                                                packetPtr,
                                                tcpVectPtr->tcpsSndUna,
                                                (ttSqn)0,
                                                TM_TCP_SYN);
                        if (errorCode != TM_EACCES)
                        {
                            errorCode = TM_ENOERROR;
                        }
                    }
                }
            }
        }
    }
    else
    {
/* Reset pointer to user data */
        tcpVectPtr->tcpsSockAddrUPtr = (ttSockAddrPtrUnionPtr)0;
    }
    return errorCode;
}

/*
 * tfTcpListen function description:
 * Called from the TCP state machine when a PASSIVE OPEN event occurs,
 * and the TCP state is closed.
 * . If no bind had been previously called, try and insert in the
 *   connection-less tree using a random port.
 * . If no error, transition to the new state (listen).
 * Parameter     description
 * tcpVectPtr    Pointer to TCP state vector (including socket entry)
 * packetPtr     Null pointer.
 * newState      new TCP state to transition to.
 *
 * Return values
 * return error value if any, TM_ENOERROR otherwise.
 */
static int tm_state_function (tfTcpListen, tcpVectPtr, packetPtr, newState)
{
    ttSocketEntryPtr        socketPtr;

    TM_UNREF_IN_ARG(packetPtr); /* unused argument */
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
/* State transition */
    tcpVectPtr->tcpsState = newState;
/* Allow connection requests from peer */
    socketPtr->socOptions |= SO_ACCEPTCONN;
    return TM_ENOERROR;
}

/*
 * tfTcpReSendSyn function description:
 * Called from the TCP state machine when a timeout occurs.
 * Re-Send a SYN segment:
 * A syn segment of the form SEQ=ISS CTL=SYN with options is sent.
 * Parameter     description
 * tcpVectPtr    Pointer to TCP state vector (including socket entry)
 * packetPtr     Null pointer.
 * newState      ignored.
 *
 * Return values
 * TM_ENOERROR   no error
 */
static int tm_state_function (tfTcpReSendSyn, tcpVectPtr, packetPtr, newState)
{
    TM_UNREF_IN_ARG(newState);
    (void)tfTcpCreateSendPacket(tcpVectPtr,
                                packetPtr,
                                tcpVectPtr->tcpsSndUna,
                                (ttSqn)0,
                                TM_TCP_SYN);
    return TM_ENOERROR;
}


/*
 * tfTcpGenSynOptions function description:
 * Called from tfTcpCreateSendPacket() when a SYN, or SYN_ACK packet is
 * generated. The MSS option is added always. Time stamp option, Window
 * scale options, and/or SEL ACK options are added if allowed.
 *
 * Return values
 * Tcp header size
 */
#ifdef TM_LINT
LINT_UNACCESS_SYM_BEGIN(mss)
#endif /* TM_LINT */
static tt16Bit tfTcpGenSynOptions (ttTcpVectPtr tcpVectPtr,
                                   ttPacketPtr packetPtr)
{
    tt8BitPtr       optPtr;
    tt16Bit         tcpFlags;
    tt16Bit         tcpHdrSize;
    tt16Bit         mss;

    tcpFlags = tcpVectPtr->tcpsFlags;
/* Header size with no options */
    tcpHdrSize = TM_BYTES_TCP_HDR;
    {
/* Point to the options area of the TCP header */
        optPtr = packetPtr->pktLinkDataPtr + TM_PAK_TCP_HDR_LEN;
/* send the MSS option */
        ((ttTcpMssOptPtr)optPtr)->tcpoMKind = TM_TCPO_MSS;
        ((ttTcpMssOptPtr)optPtr)->tcpoMLength = TM_TCPO_MSS_LENGTH;
        mss = tcpVectPtr->tcpsMSS;
        tm_htons(mss, ((ttTcpMssOptPtr)optPtr)->tcpoMSS);
/* Point to next option */
        optPtr += tm_packed_byte_len(TM_TCPO_MSS_LENGTH);
        tcpHdrSize += TM_TCPO_MSS_LENGTH;
        if (tm_16bit_one_bit_set(tcpFlags, TM_TCPF_WND_SCALE))
        {
/* Generate aligned TCP window scale option */
            ((ttTcpWndScOptPtr)optPtr)->tcpoWNoop = TM_TCPO_NOOP;
            ((ttTcpWndScOptPtr)optPtr)->tcpoWKind = TM_TCPO_WNDSC;
            ((ttTcpWndScOptPtr)optPtr)->tcpoWLength = TM_TCPO_WNDSC_LENGTH;
            ((ttTcpWndScOptPtr)optPtr)->tcpoWndSc =
                                                tcpVectPtr->tcpsRcvWndScale;

            optPtr += tm_packed_byte_len(TM_TCPO_WNDSC_LENGTH + 1);
            tcpHdrSize += TM_TCPO_WNDSC_LENGTH + 1;
        }
#ifdef TM_TCP_SACK
        if (tm_16bit_one_bit_set(tcpFlags, TM_TCPF_SEL_ACK))
        {
/* Generate Selective ACK permit option */
#ifdef TM_USE_DSP
            ((ttTcpSackPermitOptPtr)optPtr)->tcpoSPNoop1 = TM_TCPO_NOOP;
            ((ttTcpSackPermitOptPtr)optPtr)->tcpoSPNoop2 = TM_TCPO_NOOP;
#else /* !TM_USE_DSP */
            ((ttTcpSackPermitOptPtr)optPtr)->tcpoSPNoop2 = TM_TCPO_NOOP2;
#endif /* !TM_USE_DSP */
            ((ttTcpSackPermitOptPtr)optPtr)->tcpoSPKind = TM_TCPO_SACK_PERMIT;
            ((ttTcpSackPermitOptPtr)optPtr)->tcpoSPLength =
                                                         TM_TCPO_SACKP_LENGTH;
/* aligned size */
            optPtr += tm_packed_byte_len(TM_TCPO_SACKP_LENGTH + 2);
            tcpHdrSize += TM_TCPO_SACKP_LENGTH + 2; /* aligned size */
        }
#endif /* TM_TCP_SACK */
        if (tm_16bit_one_bit_set(tcpFlags, TM_TCPF_TS))
        {
/* Adding TS */
            tfTcpGenTsOption(tcpVectPtr, optPtr);
            tcpHdrSize += TM_TCPO_TS_NOOP_LENGTH;
/* Note: optionPtr should be updated if we add more options */
/* End of adding TS */
        }
    }
    return tcpHdrSize;
}
#ifdef TM_LINT
LINT_UNACCESS_SYM_END(mss)
#endif /* TM_LINT */

static void tfTcpGenTsOption (ttTcpVectPtr tcpVectPtr,
                              tt8BitPtr    optPtr )
{
    tt32Bit         tsVal;

/* Adding TS: */
    ((ttTcpAlgnTsOptPtr)optPtr)->tcpoAFirstWord = TM_TCPO_TS_FIRST_WORD;
    tm_kernel_set_critical;
    tsVal = (tt32Bit)tvTime;
    tm_kernel_release_critical;
#ifdef TM_USE_SOCKET_IMPORT
    tsVal += tcpVectPtr->tcpsTsCurrentFixup;
#endif /* TM_USE_SOCKET_IMPORT */
    tm_htonl(tsVal, ((ttTcpAlgnTsOptPtr)optPtr)->tcpoAVal);
    tsVal = tcpVectPtr->tcpsTsRecent;
    tm_htonl(tsVal, ((ttTcpAlgnTsOptPtr)optPtr)->tcpoAEchoRpl);
    return;
}

/*
 * tfTcpFreePacket function description:
 * Called from the TCP state machine when an incoming packet need to be freed.
 * If the packet has not been put into the reassembly queue, it is freed.
 * Parameter     description
 * tcpVectPtr    ignored
 * packetPtr     pointer to packet to be freed up.
 * newState      ignored.
 * Return values
 * TM_ENOERROR   no error
 */
static int tm_state_function (tfTcpFreePacket, tcpVectPtr, packetPtr,
                              newState)
{
    TM_UNREF_IN_ARG(tcpVectPtr);
    TM_UNREF_IN_ARG(newState);
/* free packet */
    if (tm_16bit_bits_not_set(packetPtr->pktSharedDataPtr->dataFlags,
                              TM_BUF_TCP_IN_QUEUE))
    {
        tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
    }
    return TM_ENOERROR;
}

/*
 * tfTcpDelReXmit function description:
 * Called from the TCP state machine when all retransmission timers
 * need to be removed.
 * Parameter     description
 * tcpVectPtr    pointer to TCP state vector.
 * packetPtr     ignored.
 * newState      ignored.
 * Return values
 * TM_ENOERROR   no error
 */
static int tm_state_function (tfTcpDelReXmit, tcpVectPtr, packetPtr,
                              newState)
{
#ifdef TM_ERROR_CHECKING
    tfKernelError("tfTcpDelReXmit",
                  "Retransmission timer should already have been deleted");
#endif /* TM_ERROR_CHECKING */
    TM_UNREF_IN_ARG(newState);
    TM_UNREF_IN_ARG(packetPtr);
    tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2, TM_TCPF2_REXMIT_MODE );
    tm_16bit_clr_bit( tcpVectPtr->tcpsFlags3, TM_TCPF3_PROBE_MODE );
    tm_tcp_timer_remove(tcpVectPtr, TM_TCPTM_REXMIT);
    tm_tcp_timer_remove(tcpVectPtr, TM_TCPTM_CONN);
    tm_tcp_timer_remove(tcpVectPtr, TM_TCPTM_PROBE);
    return TM_ENOERROR;
}

/*
 * tfTcpSegInvalid function description:
 * Called from the TCP state machine when an incoming packet is invalid.
 * We free the packet.
 * Parameter     description
 * tcpVectPtr    ignored
 * packetPtr     pointer to packet to be freed up.
 * newState      ignored.
 * Return values
 * TM_ENOERROR   no error
 */
static int tm_state_function (tfTcpSegInvalid, tcpVectPtr, packetPtr,
                              newState)
{
    TM_UNREF_IN_ARG(newState);
    TM_UNREF_IN_ARG(tcpVectPtr);

    /* free packet */
    tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
    return TM_ENOERROR;
}

/*
 * tfTcpConnReq function description:
 * Called from the TCP state machine when an incoming connection request
 * arrives.
 * 1. Check valid destination address
 * 2. Check whether we can accept the connection (socket
 *    accepting connections, backlog not full, and room to allocate a
 *    socket/tcp vector entry).
 *    If we cannot, send a reset and exit the routine.
 * 3. Create a new accept socket
 * 4. Initialize with peer address/port, local address/port
 * 5. Process options
 * 6. set ISS
 * 7. Insert new accept socket in the tree
 * 8. Initialize connection send variables, and timeout timers
 * 9. Deal with SYN data if any (put in reassembly queue)
 * 10.Send a SYN-ACK
 * Parameter    description
 * tcpVectPtr   pointer to targeted TCP state vector (including socket entry).
 * packetPtr    pointer to incoming data packet pointer (as prepared in
 *              tfTcpIncomingPacket step 10).
 * newState     new TCP state to transition to.
 * Return values
 * return error value if any, TM_ENOERROR otherwise.
 */
static int tm_state_function (tfTcpConReq, listenTcpVectPtr, packetPtr,
                              newState)
{
    ttSocketEntryPtr      listenSocketPtr;
    ttTcpVectPtr          tcpVectPtr;
    ttSocketEntryPtr      socketPtr;
    ttTcpVectPtr          conReqTcpVectPtr;
    ttSharedDataPtr       pktShrDataPtr;
#ifdef TM_USE_IPV6
    tt6IpHeaderPtr        iph6Ptr;
    ttDeviceEntryPtr      devPtr;
#endif /* TM_USE_IPV6 */
    int                   errorCode;
    int                   retCode;
    int                   isZero;
    tt16Bit               maxBackLog;
    tt16Bit               synFloodCount;
    tt8Bit                needFreePacket;
    tt8Bit                needRelockListen; /* BUG ID 1497 */
    tt8Bit                needBackLogRestore;

    needFreePacket = TM_8BIT_YES;
    needBackLogRestore = TM_8BIT_NO;
    needRelockListen = TM_8BIT_NO; /* BUG ID 1497 */
    listenSocketPtr = &(listenTcpVectPtr->tcpsSocketEntry);
    pktShrDataPtr = packetPtr->pktSharedDataPtr;
#ifdef TM_USE_IPV6
    iph6Ptr = packetPtr->pkt6RxIphPtr;
#ifdef TM_USE_IPV4
    if (iph6Ptr != (tt6IpHeaderPtr)0)
#endif /* dual */
    {
        isZero = (    tm_6_ip_zero(
                          &tm_6_ip_hdr_in6_addr(iph6Ptr->iph6DestAddr))
                   || tm_6_ip_zero(
                          &tm_6_ip_hdr_in6_addr(iph6Ptr->iph6SrcAddr)) );
    }
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
    else
#endif /* dual */
    {
        isZero = (    tm_ip_zero(tm_4_ip_addr(
                                    pktShrDataPtr->dataSpecDestIpAddress))
                   || tm_ip_zero(pktShrDataPtr->dataSrcIpAddress) );
    }
#endif /* TM_USE_IPV4 */
    if (    tm_16bit_one_bit_set(   packetPtr->pktFlags,
                                    TM_IP_BROADCAST_FLAG
                                  | TM_IP_MULTICAST_FLAG
                                  | TM_LL_BROADCAST_FLAG
                                  | TM_LL_MULTICAST_FLAG )
         || isZero )
    {
/* Ignore packets with illegal destination address, source address */
        errorCode = TM_EDESTADDRREQ; /* Destination address required */
        goto conReqExitNoUnlock;
    }
    maxBackLog = listenSocketPtr->socMaxBackLog;
    synFloodCount = listenTcpVectPtr->tcpsSynFloodCount;
    if (    ( tm_16bit_bits_not_set( listenSocketPtr->socOptions,
                                     SO_ACCEPTCONN ) ) )
    {
/* Cannot accept the connection. Refuse it (i.e. send a reset) */
        errorCode = TM_ECONNREFUSED;
        goto conReqExitNoUnlock;
    }
    if (   (listenSocketPtr->socBackLog >= maxBackLog)
/*
 * SYN flood attack prevention: Allow con req through if the backlog is fully
 * in half connected connections.
 */
         && (synFloodCount != maxBackLog) )
    {
/*
 * BUG ID 1741: According to Stevens Vol. 2 p 455 "When this occurs, TCP
 * ignores incoming connection requests."
 * errorCode must be something other than TM_ECONNREFUSED or TM_ENOERROR
 */
        errorCode = TM_EPERM;
        goto conReqExitNoUnlock;
    }
/*
 * Count this connection request in the back log.
 */
    listenSocketPtr->socBackLog++;
/* Increase the SYN flood count. */
    listenTcpVectPtr->tcpsSynFloodCount++;
    needBackLogRestore = TM_8BIT_YES;
/*
 * . If either we have reached our maximum SYN flood count,
 *      or we do not have any sockets available, and we have more
 *         than half of the backlog in half connectected connections,
 *   drop the first one in the half connected connection request. Do so before
 *   allocating a socket, or we might fail if we have reached the maximum
 *   number of allowed sockets.
 */
    if (    (synFloodCount == maxBackLog)
         || (     (    tm_context(tvNumberOpenSockets)
                    == tm_context(tvMaxNumberSockets) )
              &&  (synFloodCount > maxBackLog/2) ) )
    {
/*
 * Remove first entry from the half connected queue, send a reset,
 * and delete it.
 */
        conReqTcpVectPtr =
            listenTcpVectPtr->tcpsConQueue[TM_TCP_SYN_QUEUE].tcpConReqNextPtr;
        if (conReqTcpVectPtr != listenTcpVectPtr)
        {
/* Will decrease tcpsSynFloodCount */
            tfTcpRemoveResetConReq(conReqTcpVectPtr, TM_TCP_SYN_QUEUE);
            if (listenTcpVectPtr->tcpsState != TM_TCPS_LISTEN)
            {
/*
 * listening socket was closed while unlocked in tfTcpRemoveResetConReq:
 * send a reset.
 */
                errorCode = TM_ECONNREFUSED;
                goto conReqExitNoUnlock;
            }
        }
        else
        {
/* Could not remove an entry from the half connected queue. Bail out */
            errorCode = TM_EPERM;
            goto conReqExitNoUnlock;
        }
    }
/*
 * lets allocate a socket entry. Owner count initialized to 1, and socket
 * locked in tfSocketAllocate(), entry owned by us
 */
    tcpVectPtr = (ttTcpVectPtr)(ttVoidPtr)tfSocketAllocate(
                                                  (tt8Bit)IP_PROTOTCP,
                                                  &errorCode );
    if (tcpVectPtr == (ttTcpVectPtr)0)
    {
/* The allocation failed. Refuse the connection */
        errorCode = TM_ECONNREFUSED;
        goto conReqExitNoUnlock;
    }
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
/*
 * Socket is locked, and checked out.
 * Copy all informations needed from the listening socket
 */

/* Copy socket options (except accept) */
    socketPtr->socOptions = listenSocketPtr->socOptions;
    tm_16bit_clr_bit( socketPtr->socOptions, SO_ACCEPTCONN );
/* Copy blocking status of listening socket */
    tm_16bit_clr_bit( socketPtr->socFlags, TM_SOCF_NONBLOCKING );
    socketPtr->socFlags = (tt16Bit)(   listenSocketPtr->socFlags
                                     & (TM_SOCF_MSG_WAIT_FLAGS ) );
/* Copy all settable options from the listening socket */
    socketPtr->socMaxRecvQueueBytes = listenSocketPtr->socMaxRecvQueueBytes;
    socketPtr->socMaxSendQueueBytes= listenSocketPtr->socMaxSendQueueBytes;
    tcpVectPtr->tcpsMSS = listenTcpVectPtr->tcpsMSS;
    tcpVectPtr->tcpsDelayAckTime = listenTcpVectPtr->tcpsDelayAckTime;
    socketPtr->socLowRecvQueueBytes = listenSocketPtr->socLowRecvQueueBytes;
    socketPtr->socLowSendQueueBytes = listenSocketPtr->socLowSendQueueBytes;
    socketPtr->socSendAppendThreshold =
                                      listenSocketPtr->socSendAppendThreshold;
    socketPtr->socRecvCopyFraction = listenSocketPtr->socRecvCopyFraction;
    socketPtr->socLingerTime = listenSocketPtr->socLingerTime;
#ifdef TM_USE_IPV4
    socketPtr->socIpTos = listenSocketPtr->socIpTos;
    socketPtr->socIpTtl = listenSocketPtr->socIpTtl;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    socketPtr->soc6HopLimit = listenSocketPtr->soc6HopLimit;
    socketPtr->soc6FlowInfo = listenSocketPtr->soc6FlowInfo;
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_STRONG_ESL
    if (tm_8bit_one_bit_set(listenSocketPtr->socFlags2,
                             TM_SOCF2_BINDTODEVICE))
/* Inherit bind to device from listening socket */
    {
        socketPtr->socRteCacheStruct.rtcDevPtr =
                         listenSocketPtr->socRteCacheStruct.rtcDevPtr;
        socketPtr->socFlags2 |= TM_SOCF2_BINDTODEVICE;
    }
#ifdef TM_USE_STRICT_STRONG_ESL
    else
    {
/* Bind to incoming device */
        socketPtr->socFlags2 |= TM_SOCF2_BINDTODEVICE;
        socketPtr->socRteCacheStruct.rtcDevPtr = packetPtr->pktDeviceEntryPtr;
    }
#endif /* TM_USE_STRICT_STRONG_ESL */
#endif /* TM_USE_STRONG_ESL */
    tcpVectPtr->tcpsMaxRt = listenTcpVectPtr->tcpsMaxRt;
    tcpVectPtr->tcpsEffSndMss =  listenTcpVectPtr->tcpsEffSndMss;
    tcpVectPtr->tcpsFinWt2Time = listenTcpVectPtr->tcpsFinWt2Time;
    tcpVectPtr->tcps2MSLTime = listenTcpVectPtr->tcps2MSLTime;
    tcpVectPtr->tcpsKeepAliveTime = listenTcpVectPtr->tcpsKeepAliveTime;
    tcpVectPtr->tcpsMaxReXmitCnt = listenTcpVectPtr->tcpsMaxReXmitCnt;
    tcpVectPtr->tcpsIdleIntvTime = listenTcpVectPtr->tcpsIdleIntvTime;
    tcpVectPtr->tcpsKeepAliveProbeCnt =
                                listenTcpVectPtr->tcpsKeepAliveProbeCnt;
    tcpVectPtr->tcpsRtoDef = listenTcpVectPtr->tcpsRtoDef;
    tcpVectPtr->tcpsRtoMin = listenTcpVectPtr->tcpsRtoMin;
    tcpVectPtr->tcpsRtoMax = listenTcpVectPtr->tcpsRtoMax;
    tcpVectPtr->tcpsProbeMin = listenTcpVectPtr->tcpsProbeMin;
    tcpVectPtr->tcpsProbeMax = listenTcpVectPtr->tcpsProbeMax;
#ifdef TM_USE_USER_PARAM
/* Inherit the user-defined TCP connection specific data that might have
 * been stored in the packet by the device driver. */
    tm_bcopy(&packetPtr->pktUserStruct.pktuUserParam,
             &tcpVectPtr->tcpsUserParam, sizeof(ttUserGenericUnion));
#endif /* TM_USE_USER_PARAM */
#ifdef TM_USE_SSL_SERVER
/* If it is a ssl socket, we need to copy the SSL information from
 * listenTcpVectPtr to tcpVectPtr
 */
    if (tm_16bit_one_bit_set(listenTcpVectPtr->tcpsSslFlags,TM_SSLF_SERVER))
    {
        tcpVectPtr->tcpsSslFlags = listenTcpVectPtr->tcpsSslFlags;
        tcpVectPtr->tcpsSslSession = listenTcpVectPtr->tcpsSslSession;
        tcpVectPtr->tcpsSslSendMinSize = listenTcpVectPtr->tcpsSslSendMinSize;
        tcpVectPtr->tcpsSslSendMaxSize = listenTcpVectPtr->tcpsSslSendMaxSize;
        errorCode = tfSslServerCreateState(tcpVectPtr);
        if(errorCode != TM_ENOERROR)
        {
            errorCode = TM_ECONNREFUSED;
            goto conReqExit;
        }
    }
#endif /* TM_USE_SSL_SERVER */
/*
 * set the socProtocolFamily properly, so that accept() would pass back
 * a sockaddr struct with the right family
 */
    socketPtr->socProtocolFamily = listenSocketPtr->socProtocolFamily;
/* copy socket 4-tuple (to check routing entry, and for socket lookup) */
    tm_bcopy(packetPtr->pktSharedDataPtr->dataSockTuplePtr,
             &socketPtr->socTuple,
             sizeof(ttSockTuple));
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
    if (iph6Ptr != (tt6IpHeaderPtr)0)
#endif /* dual */
    {
        tcpVectPtr->tcpsNetworkLayer = TM_NETWORK_IPV6_LAYER;
        if (!IN6_IS_ADDR_LINKLOCAL(&tm_6_ip_hdr_in6_addr(
                                                    iph6Ptr->iph6DestAddr)))
        {
            devPtr = tf6IfaceMatch(
                        &tm_6_ip_hdr_in6_addr(iph6Ptr->iph6DestAddr),
                        tf6MhomeAddrMatch,
                        &(socketPtr->socRteCacheStruct.rtcSrcMhomeIndex));
            if (devPtr == (ttDeviceEntryPtr)0)
            {
                goto conReqExit;
            }
            socketPtr->socRteCacheStruct.rtcSrcMhomeSet = TM_8BIT_YES;
            socketPtr->socRteCacheStruct.rtcSrcDevPtr = devPtr;
        }
    }
#ifdef TM_USE_IPV4
    else
    {
        tcpVectPtr->tcpsNetworkLayer = TM_NETWORK_IPV4_LAYER;
    }
#endif /* dual */
#endif /* TM_USE_IPV6 */

/*
 * Process received SYN options from the peer. Initialize receive sequence
 * numbers. Also check routing entry and set MSS from peer.
 */
    errorCode = tfTcpRcvSynOptions(packetPtr,
                                   (tt16Bit)(   listenTcpVectPtr->tcpsFlags
                                              | TM_TCPF_PASSIVE ),
                                   tcpVectPtr);
    if (errorCode != TM_ENOERROR) /* no routing entry */
    {
/* No return path to the sender, refuse the connection */
        errorCode = TM_ECONNREFUSED;
        goto conReqExit;
    }
/* Initial sequence number */
    if (listenTcpVectPtr->tcpsIss != (ttSqn)0)
    {
/*
 * Reuse ISS from old connection now in TIME WAIT that we copied to the
 * listening socket in tfTcpTmWtGetIssClose()
 */
        tcpVectPtr->tcpsIss = listenTcpVectPtr->tcpsIss;
        listenTcpVectPtr->tcpsIss = (ttSqn)0; /* reset it */
    }
    else
    {
/* Otherwise (no old connection in time wait state) issue a new ISS */
        tcpVectPtr->tcpsIss = (ttSqn)tfGetRandom();
    }
    tcpVectPtr->tcpsState = newState;
#ifdef TM_SNMP_MIB
/*
 * Number of times TCP connections have made a direct
 * transition to the SYN-RCVD state from the LISTEN state.
 */
    tm_context(tvTcpData).tcpPassiveOpens++;
#endif /* TM_SNMP_MIB */
/* Unlock the listening socket to prevent deadlock with socket root lock */
    tm_call_unlock(&listenSocketPtr->socLockEntry);
/* Insert in the tree */
    retCode = tfSocketTreeInsert(socketPtr,
                                 &(socketPtr->socTuple),
                                 (tt16Bit)(   (   socketPtr->socOptions
                                                & SO_REUSEADDR )
                                            | TM_SOC_TREE_CO_FLAG ) );
/*
 * Initialize connection send variables, and timeout timers, before unlocking
 * the socket.
 */
    tfTcpInitConTmr(tcpVectPtr);
/* Unlock the socket before locking the listening socket to prevent deadlock */
    tm_call_unlock(&socketPtr->socLockEntry);
/* Lock the listening socket */
    tm_call_lock_wait(&listenSocketPtr->socLockEntry);
/* Lock the socket */
    tm_call_lock_wait(&socketPtr->socLockEntry);
    if (tcpVectPtr->tcpsState == TM_TCPS_CLOSED)
    {
/* Connection was reset while we did not have the lock */
        errorCode = TM_ECONNRESET;
    }
    else if (    (retCode != TM_SOC_RB_OKAY)
              || (listenTcpVectPtr->tcpsState != TM_TCPS_LISTEN) )
    {
/*
 * Could not insert in the tree (should not happen), or listening socket
 * was closed while unlocked: send a reset.
 */
        errorCode = TM_ECONNREFUSED;
        needRelockListen = TM_8BIT_YES;
/* UnLock the listening socket */
        tm_call_unlock(&listenSocketPtr->socLockEntry);
/*
 * Make sure socket is removed from the tree if needed, timers are freed, and
 * routing cache (set in tfTcpRcvSynOptions) is reset.
 */
        tfTcpVectClose(tcpVectPtr, TM_PACKET_NULL_PTR, TM_TCPS_CLOSED);
    }
    else
    {
/* Back pointer to listening socket */
        socketPtr->socListenSocketPtr = listenSocketPtr;
/* The accepted socket has a pointer to the listening socket */
        tm_socket_checkout(listenSocketPtr);
/*
 * Insert in SYN connection request queue. We use a doubly link circular list.
 * (Circular doubly list is the most efficient kind of list here
 * since the head (listening vector) never changes, and is itself a tcp
 * vector, and since we always want to insert at the end, and want the
 * ability to remove a connection request (when closing) without to search
 * through the list.)
 */
        tfTcpConQueueInsert(listenTcpVectPtr, tcpVectPtr, TM_TCP_SYN_QUEUE);
/*
 * Deal with SYN data (put in reassemble queue).
 * NOTE: need to call down instead of letting the state machine do
 * it, since the state machine owns the listening socket, not the
 * accept socket
 */
        (void)tfTcpSynData(tcpVectPtr, packetPtr, newState);
        if (tm_16bit_one_bit_set(pktShrDataPtr->dataFlags, TM_BUF_TCP_IN_QUEUE))
        {
            needFreePacket = TM_8BIT_ZERO;
        }
/* BUD ID 1497 */
        needRelockListen = TM_8BIT_YES;
/* UnLock the listening socket */
        tm_call_unlock(&listenSocketPtr->socLockEntry);
/* END of BUD ID 1497 */
/* Send a SYN-ACK, socketPtr will be unlocked and relocked there. */
        (void)tfTcpSendSynAck(tcpVectPtr, packetPtr, newState);
    }
conReqExit:
    tm_socket_checkin_call_unlock(socketPtr);
    if (needRelockListen != TM_8BIT_NO) /* BUD ID 1497 */
    {
/* ReLock the listening socket */
        tm_call_lock_wait(&listenSocketPtr->socLockEntry);
    }
    if (errorCode != TM_ENOERROR)
    {
/*
 * We allocated a socket, but could not use it. Decrease number of
 * open sockets (that was increased in the tfSocketAllocate() function)
 */
        tfSocketDecreaseOpenSockets();
    }
conReqExitNoUnlock:
    if (errorCode != TM_ENOERROR)
    {
        if (errorCode == TM_ECONNREFUSED)
        {
/* connection refused by us. Let the peer know */
            (void)tfTcpSendRstSeq0(listenTcpVectPtr, packetPtr,
                                   listenTcpVectPtr->tcpsState);
            needFreePacket = TM_8BIT_ZERO;
        }
        if (needBackLogRestore)
        {
/* Update backlog */
            listenSocketPtr->socBackLog--;
            listenTcpVectPtr->tcpsSynFloodCount--;
        }
    }
    if (needFreePacket)
    {
        tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
    }
    return errorCode;
}

/*
 * tfTcpCheckValidIssAck function description:
 * Called from the TCP state machine when an incoming ACK of a SYN arrives
 * If SEG.ACK =< ISS, or SEG.ACK > SND.NXT, send a reset
 *
 *           <SEQ=SEG.ACK><CTL=RST>
 *
 *         and discard the segment.  Return.
 * Otherwise transition to new state.
 * Parameter    description
 * tcpVectPtr   pointer to targeted TCP state vector (including socket entry).
 * packetPtr    pointer to incoming data packet pointer (as prepared in
 *              tfTcpIncomingPacket step 10).
 * newState     new TCP state to transition to.
 * Return values
 * return error value if any, TM_ENOERROR otherwise.
 */
static int tm_state_function (tfTcpCheckValidIssAck, tcpVectPtr, packetPtr,
                              newState)
{
    ttTcpHeaderPtr tcphPtr;
    ttSqn          segAck;
    int            errorCode;

    TM_UNREF_IN_ARG(newState);
    tcphPtr = tm_tcp_hdr_ptr(packetPtr);
/*
 * If SEG.ACK =< ISS, or SEG.ACK > SND.NXT, send a reset
 *
 *           <SEQ=SEG.ACK><CTL=RST>
 *
 *         and discard the segment.  Return.
 */

    segAck = tcphPtr->tcpAckNo;
    if (    tm_sqn_leq(segAck,tcpVectPtr->tcpsIss)
         || tm_sqn_gt(segAck, tcpVectPtr->tcpsMaxSndNxt) )
    {
/* Rst will free packet */
        (void)tfTcpSendRst(tcpVectPtr, packetPtr, tcpVectPtr->tcpsState);
        errorCode = TM_EINVAL;
    }
    else
    {
        if (    (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_PASSIVE)
             && (tcpVectPtr->tcpsSocketEntry.socListenSocketPtr
                                             == TM_SOCKET_ENTRY_NULL_PTR) ) )
/*
 * Passive socket and
 * Another task is in the process of sending a RESET on this socket, but has
 * been pre-empted by this one
 */
        {
            errorCode = TM_EINVAL;
            tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
        }
        else
        {
/* Otherwise transition to new state. */
            tcpVectPtr->tcpsState = newState;
            errorCode = TM_ENOERROR;
        }
    }
    return errorCode;
}

/*
 * tfTcpEstablish function description:
 * Called from the TCP state machine when an incoming TCP packet makes us
 * transition to the TCP established state.
 * RFC 793:
 * SND.UNA should be advanced to equal SEG.ACK and any segments on the
 * retransmission queue which are thereby acknowledged should be removed.
 * Change the connection state to ESTABLISHED, the following variables
 * must be set:
 *                   SND.WND <- SEG.WND
 *                   SND.WL1 <- SEG.SEQ
 *                   SND.WL2 <- SEG.ACK
 *
 *       and form an ACK segment:
 *         <SEQ=SND.NXT><ACK=RCV.NXT><CTL=ACK>
 *
 *   and send it.  Data or controls which were queued for
 *   transmission may be included.  If there are other controls or
 *   text in the segment then continue processing at the sixth step
 *   below where the URG bit is checked, otherwise return.
 * End of RFC 793.
 * 1. Update Tcp state vector send window variables
 * 2. Get first estimate of peer's receive window
 * 3. Socket is now connected
 * 4. Connection is established, turn off connection timeout.
 * 5. Add Keep alive timer
 * 6. free ACKed buffers, update send queue, update RTT (since we
 *    are advancing the left edge of the send window)
 *    If time stamp on use time stamp mechanism to update rtt.
 *    Otherwise use our time that we saved when sending the SYN.
 * 7. Advance the left edge of the send window
 * 8. If retransmit timer moved our sndNxt back, change it to segAck
 * 9. If all data has been acknowledged, cancel retransmit timer
 * 10.If we had postponed sending the FIN until we reach the established
 *    state, send it now
 * 11.Notify accept (passive) or connect (active)
 * 12.Check for SYN data
 * 13.Check urgent data (RFC 793 sixth step)
 * 14.Process incoming data (if any) (RFC 793 seventh step)
 * 15.Process incoming FIN (if any) (done at the end of tfTcpIncomingPacket)
 * Note: notification of incoming data (read), Urgent data (exception) done
 *       at the end of tfTcpIncomingPacket()
 * Parameter    description
 * tcpVectPtr   pointer to targeted TCP state vector (including socket entry).
 * packetPtr    pointer to incoming data packet pointer (as prepared in
 *              tfTcpIncomingPacket step 10).
 * newState     new TCP state to transition to.
 * Return values
 * return error value if any, TM_ENOERROR otherwise.
 */
#ifdef TM_USE_SOCKET_IMPORT
/* Public entry for initializing imported sockets */
void tfTcpEstablishSocImp(ttTcpVectPtr tcpVectPtr)
{
    tfTcpEstablish(tcpVectPtr, TM_PACKET_NULL_PTR, TM_TCPS_ESTABLISHED);
}
#endif /* TM_USE_SOCKET_IMPORT */
#ifdef TM_LINT
LINT_NULL_PTR_BEGIN(listenSocketPtr)
#endif /* TM_LINT */
static int tm_state_function (tfTcpEstablish, tcpVectPtr, packetPtr, newState)
{
    ttTcpHeaderPtr              tcphPtr;
    ttSocketEntryPtr            socketPtr;
    ttSocketEntryPtr            listenSocketPtr;
    ttTcpVectPtr                listenTcpVectPtr;
    ttPacketPtr                 dataPacketPtr;
#ifdef TM_USE_IPV4
    ttIpHeaderPtr               iphPtr;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPSEC
#ifdef TM_USE_IPDUAL
    ttRteEntryPtr               rtePtr;
#endif /* TM_USE_IPDUAL */
#endif /* TM_USE_IPSEC */
    tt32Bit                     bytesAcked;
    ttSqn                       segAck;
#ifdef TM_TCP_SEND_SYN_DATA
    tt32Bit                     tsEcr;
    ttS32Bit                    rtt;
#endif /* TM_TCP_SEND_SYN_DATA */
    tt8Bit                      needFreePacket;
    tt8Bit                      tcpCtlBits;
    tt8Bit                      urgentData;
    tt8Bit                      needUnlockListeningSocket;
    tt8Bit                      dataInPacket;
    tt8Bit                      removed;
#ifdef TM_TCP_SEND_SYN_DATA
    tt8Bit                      computeRtoFlag;
#endif /* TM_TCP_SEND_SYN_DATA */
    ttGenericUnion              timerParm1;
#ifdef TM_USE_SOCKET_IMPORT
    ttUserTcpConPtr             userTcpConPtr;
#endif /* TM_USE_SOCKET_IMPORT */

#ifdef TM_SNMP_MIB
/*
 * number of TCP connections for which the current state
 * is either ESTABLISHED or CLOSE-WAIT."
 */
    tm_context(tvTcpData).tcpCurrEstab++;
#endif /* TM_SNMP_MIB */
    TM_UNREF_IN_ARG(newState);
#ifdef TM_USE_SOCKET_IMPORT
/* Creating a socket in the established state. There is no packet. The
 * necessary TCP state information is provided by the user. */
    userTcpConPtr = tcpVectPtr->tcpsUserTcpConPtr;
    if (userTcpConPtr != (ttUserTcpConPtr)0)
    {
        tcphPtr = (ttTcpHeaderPtr)0;
        segAck = userTcpConPtr->uconSndUna;
    }
    else
#endif /* TM_USE_SOCKET_IMPORT */
    {
        tcphPtr = tm_tcp_hdr_ptr(packetPtr);
        segAck = tcphPtr->tcpAckNo;
    }
/* Allow re recycling of tcp headers */
    tcpVectPtr->tcpsFlags |= TM_TCPF_RECYCLE_HDR | TM_TCPF_SOCKETENTRYPTR;
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
#ifdef TM_USE_IPV4
    if (
#ifdef TM_USE_SOCKET_IMPORT
            packetPtr != TM_PACKET_NULL_PTR &&
#endif /* TM_USE_SOCKET_IMPORT */
            packetPtr->pktNetworkLayer == TM_NETWORK_IPV4_LAYER )
    {
        iphPtr = (ttIpHeaderPtr)
                (((ttCharPtr)(ttVoidPtr)tcphPtr) - packetPtr->pktIpHdrLen);
        socketPtr->socIpConTos = iphPtr->iphTos;
    }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPSEC
#ifdef TM_USE_IPDUAL
    rtePtr = socketPtr->socRteCacheStruct.rtcRtePtr;
    if (    (rtePtr != (ttRteEntryPtr)0)
         && (rtePtr->rteDevPtr == tm_context(tv6Ipv4DefGwTunnelPtr)) )
    {
        tm_16bit_clr_bit( tcpVectPtr->tcpsFlags,
                          ( TM_TCPF_RECYCLE_HDR | TM_TCPF_SOCKETENTRYPTR));
    }
#endif /* TM_USE_IPDUAL */
#endif /* TM_USE_IPSEC */
/* Update Tcp state vector send window variables */
#ifdef TM_USE_SOCKET_IMPORT
    if (userTcpConPtr != (ttUserTcpConPtr)0)
    {
/* Creating a socket in the established state. There is no packet. The
 * necessary TCP state information is provided by the user. */
        tcpVectPtr->tcpsSndWnd = userTcpConPtr->uconSndWnd;
        tcpVectPtr->tcpsSndWL1 = userTcpConPtr->uconRcvNxt;
        tcpVectPtr->tcpsSndWL2 = userTcpConPtr->uconSndUna;
    }
    else
#endif /* !TM_USE_SOCKET_IMPORT */
    {
        tcpVectPtr->tcpsSndWnd = packetPtr->pktTcpUnscaledWnd;
        tcpVectPtr->tcpsSndWL1 = tcphPtr->tcpSeqNo;
        tcpVectPtr->tcpsSndWL2 = tcphPtr->tcpAckNo;
    }
/* Get first estimate of peer's receive window */
    tcpVectPtr->tcpsMaxSndWnd = tcpVectPtr->tcpsSndWnd;
/* Socket is now connected */
    socketPtr->socFlags |= TM_SOCF_CONNECTED;
/*
 * Connection is established, turn off connection timeout.
 */
    tm_tcp_timer_remove(tcpVectPtr, TM_TCPTM_CONN);
/* Add Keep alive timer */
    timerParm1.genVoidParmPtr = (ttVoidPtr)tcpVectPtr;
    tm_socket_checkout(socketPtr);
    tcpVectPtr->tcpsKeepAliveTmPtr =
                   tfTimerAddExt( &tcpVectPtr->tcpsKeepAliveTm,
                                  tfTcpTmKeepAlive,
                                  tfTcpTmrCleanup,
                                  timerParm1,
                                  timerParm1, /* unused */
                                  ((tt32Bit)tcpVectPtr->tcpsIdleIntvTime) *
                                    TM_UL(1000),
                                  TM_TIM_AUTO);
/* Add delay ACK timer if used in automatic mode */
    if (tcpVectPtr->tcpsDelayAckTime != TM_UL(0))
    {
        timerParm1.genVoidParmPtr = (ttVoidPtr)tcpVectPtr;
        tm_socket_checkout(socketPtr);
        tcpVectPtr->tcpsDelayAckTmPtr =
                               tfTimerAddExt( &tcpVectPtr->tcpsDelayAckTm,
                                              tfTcpTmDelayAck,
                                              tfTcpTmrCleanup,
                                              timerParm1,
                                              timerParm1, /* unused */
                                              tcpVectPtr->tcpsDelayAckTime,
                                              TM_TIM_AUTO | TM_TIM_SUSPENDED );
    }
#ifdef TM_TCP_RFC2581
    if (tcpVectPtr->tcpsCwnd == (tt32Bit)tm_tcp_lw(tcpVectPtr))
    {
/* If SYN was retransmitted, set the initial window back to 2 segments. */
        tcpVectPtr->tcpsCwnd = tm_tcp_iw(tcpVectPtr);
    }
#endif /* TM_TCP_RFC2581 */
#ifdef TM_TCP_RFC2414
    if (    (tm_16bit_all_bits_set(tcpVectPtr->tcpsFlags,
                                   TM_TCPF_INITIAL_WINDOW) )
/*
 * If user did not turn off slow start and did not def out full initial window
 * on direct routes
 * And SYN did not get retransmitted
 */
         && (tcpVectPtr->tcpsReXmitCnt == 0) )
    {
/* min((4*MSS), max(2*MSS, 4380)) */
        tcpVectPtr->tcpsCwnd = 2 * tcpVectPtr->tcpsEffSndMss;
        if (tcpVectPtr->tcpsCwnd < TM_UL(4380))
        {
            tcpVectPtr->tcpsCwnd = 4 * tcpVectPtr->tcpsEffSndMss;
            if (tcpVectPtr->tcpsCwnd > TM_UL(4380))
            {
                tcpVectPtr->tcpsCwnd = TM_UL(4380);
            }
        }
        tcpVectPtr->tcpsInitialCwnd = tcpVectPtr->tcpsCwnd;
    }
#endif /* TM_TCP_RFC2414 */
/*
 * free ACKed buffers, update send queue, update RTT (if we
 * are ACKing data)
 */
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
/* Advance the left edge of the send window */
    tcpVectPtr->tcpsSndUna = segAck;
/* If retransmit timer moved our sndNxt back, change it to segAck */
    if (tm_sqn_lt(tcpVectPtr->tcpsSndNxt, segAck))
    {
        tcpVectPtr->tcpsSndNxt = segAck;
    }
    if (bytesAcked > 1) /* Acking real data */
    {
#ifdef TM_TCP_SEND_SYN_DATA
        computeRtoFlag = TM_8BIT_YES;
        if (    tm_16bit_one_bit_set( packetPtr->pktSharedDataPtr->dataFlags,
                                      TM_BUF_TCP_TS_ON )
/* BUG ID 11-671: Work around Windows 2000 time samp option bug. */
             && (packetPtr->pktTcpTsEcr != TM_UL(0)) )
        {
/* If time stamp on, use time stamp mechanism to update rtt */
            tsEcr = packetPtr->pktTcpTsEcr;
        }
        else
        {
/*
 * If time stamp is not on, we need to compute rtt from the saved
 * transmission time (Vegas round trip time computation).
 */
            tsEcr = socketPtr->socSendQueueNextPtr->pktTcpXmitTime;
            if (    tm_16bit_bits_not_set(
                                    socketPtr->socSendQueueNextPtr->pktFlags2,
                                    TM_PF2_TCP_TIMED )
/* Packet has not been timed */
                 || tm_16bit_one_bit_set(
                                    socketPtr->socSendQueueNextPtr->pktFlags2,
                                    TM_PF2_TCP_REXMIT ) )
/* Ignore rtt if packet has been re-transmitted (KARN algorithm) */
            {
                computeRtoFlag = TM_8BIT_ZERO;
            }
        }
        tm_kernel_set_critical;
        rtt = (ttS32Bit)(tvTime - tsEcr);
        tm_kernel_release_critical;
        tfTcpSendQueueFree(tcpVectPtr, bytesAcked-1, rtt, computeRtoFlag);
#else /* !TM_TCP_SEND_SYN_DATA */
        tfTcpSendQueueFree(tcpVectPtr, bytesAcked-1, TM_L(0), TM_8BIT_ZERO);
#endif /* !TM_TCP_SEND_SYN_DATA */
    }
    else /* Only Acking the SYN */
    {
/*
 * Do not update the RTO with the RTT of our SYN, since we are not ACKing
 * any data. This will prevent us from adjusting our RTO downwards and prevent
 * unnecessary retransmissions on slow links. (On a slow link the SYN
 * will travel much faster than packets with real data.)
 */
        tfTcpUpdateRtt(tcpVectPtr, TM_UL(0), TM_8BIT_ZERO);
    }
/* If all data has been acknowledged, cancel retransmit timer */
    if (tcpVectPtr->tcpsMaxSndNxt == tcpVectPtr->tcpsSndUna)
    {
/* disable retransmission timer */
        tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2,
                          TM_TCPF2_REXMIT_MODE );
        tm_tcp_timer_suspend(tcpVectPtr->tcpsReXmitTmPtr);
    }
#ifdef TM_USE_SOCKET_IMPORT
    if (tcpVectPtr->tcpsUserTcpConPtr != (ttUserTcpConPtr)0)
    {
/* Skip packet parsing, since there is no packet */
        tcpVectPtr->tcpsRcvUp = tcpVectPtr->tcpsRcvNxt-1;
        goto tcpEstablishSocImp1;
    }
#endif /* TM_USE_SOCKET_IMPORT */
/* Assume no data in packet */
    dataPacketPtr = TM_PACKET_NULL_PTR;
    needFreePacket = TM_8BIT_YES;
    dataInPacket = TM_8BIT_ZERO;
    tcpCtlBits = tcphPtr->tcpControlBits;
    if (tm_8bit_one_bit_set(tcpCtlBits, TM_TCP_SYN))
    {
        (void)tfTcpSynData(tcpVectPtr, packetPtr, newState);
        tcpVectPtr->tcpsFlags2 |= TM_TCPF2_ACK; /* ACK SYN now */
        if (tm_16bit_one_bit_set(packetPtr->pktSharedDataPtr->dataFlags,
                                 TM_BUF_TCP_IN_QUEUE))
        {
/* Packet already queued in reassemble queue */
            needFreePacket = TM_8BIT_ZERO;
        }
    }
    else
    {
/*
 * RFC 793 sixth step: Check urgent data.
 */
        if (tm_8bit_one_bit_set(tcpCtlBits, TM_TCP_URG))
        {
            urgentData = tfTcpRcvUrgentData(tcpVectPtr, packetPtr);
        }
        else
        {
            urgentData = TM_8BIT_ZERO;
        }
        if (urgentData == TM_8BIT_ZERO)
        {
/* If no urgent data */
            if (tm_sqn_lt(tcpVectPtr->tcpsRcvUp, tcpVectPtr->tcpsRcvNxt-1))
            {
/* and if not in urgent mode: update Rcv.UP to catch up to Rcv.Nxt-1*/
                tcpVectPtr->tcpsRcvUp = tcpVectPtr->tcpsRcvNxt-1;
            }
        }
        if (    (packetPtr->pktChainDataLength != TM_UL(0))
             || (tcpCtlBits & TM_TCP_FIN) )
/* if there is data or a FIN in this packet, process data */
        {
            dataPacketPtr = packetPtr;
            needFreePacket = TM_8BIT_ZERO;
/* BUG ID 1487 */
            if (packetPtr->pktChainDataLength != TM_UL(0))
            {
                dataInPacket = TM_8BIT_YES;
            }
/* END BUG ID 1487 */
        }
    }
/*
 * RFC 793 seventh step process the segment text:
 * Copy data from packet and reassemble queue to receive queue,
 * update rcvNxt.
 * Note upon return from tfTcpReassemblePacket, dataPacketPtr can no
 * longer be accessed.
 */
    tcpCtlBits = tfTcpReassemblePacket(tcpVectPtr, dataPacketPtr);
    if (needFreePacket != TM_8BIT_ZERO)
    {
        tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
    }
    if (tcpCtlBits & TM_TCP_FIN)
/* FIN is next byte to process */
    {
/* Tell tfTcpIncomingPacket to process the SEG_FIN event */
        tcpVectPtr->tcpsFlags2 |= TM_TCPF2_RCV_FIN;
    }
/* Notify accept or connect */
    if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_PASSIVE))
    {
/* TCP Server */
/* BUG ID 1488 && BUG ID 1487 */
        if (tm_16bit_one_bit_set(tcpVectPtr->tcpsSocketCBFlags, TM_CB_RECV))
/* Data was piggied back on the client SYN, and/or ACK of the 3-way handshake */
        {
/*
 * BUG ID 1488: Undo socket notification flags to prevent double
 * notification on accepted socket.
 */
            tm_16bit_clr_bit( tcpVectPtr->tcpsSocketCBFlags, TM_CB_RECV );
            tm_16bit_clr_bit( tcpVectPtr->tcpsSelectFlags, TM_SELECT_READ );
/* End BUG ID 1488 */
/* BUG ID 1487 */
            if (dataInPacket)
/* Data in the ACK segment */
            {
/* BUG ID 1487: ACK needs to be delayed */
                if (    (tcpVectPtr->tcpsDelayAckTmPtr != TM_TMR_NULL_PTR)
#ifdef TM_TCP_ACK_PUSH
/* OL 3/29/01 BUG ID 11-631: workaround an Internet Explorer bug */
                     && (tm_8bit_bits_not_set(tcpCtlBits, TM_TCP_PSH))
#endif /* TM_TCP_ACK_PUSH */
                   )
                {
/*
 * Note that the delay ACK timer has jut been started in this function in
 * the suspended state. Unsuspend the timer.
 */
                    tm_timer_unsuspend(tcpVectPtr->tcpsDelayAckTmPtr);
/* Turn off TM_TCPF2_ACK, and turn on TM_TCPF2_DELAY_ACK */
                    tcpVectPtr->tcpsFlags2
                                        ^= (TM_TCPF2_DELAY_ACK | TM_TCPF2_ACK);
                }
            }
            else
/*
 * All data was acknowledged (SYN data), or FIN.
 * (FIN: State machine will take care of FIN next.)
 */
            {
                tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2, TM_TCPF2_ACK );
            }
        }
/* End BUG ID 1487 */
        needUnlockListeningSocket = TM_8BIT_ZERO;
/* listening socket */
        listenSocketPtr = socketPtr->socListenSocketPtr;
/*
 * Increase the number of threads that need access to the listening socket.
 * This is to prevent the listening socket owner count from being decreased,
 * when the back pointer from this socket to the listening socket is reset
 * if the listening socket is closed (and hence the connection request
 * removed) after our socket is unlocked below, or if the connection is reset
 * after our socket is unlocked below.
 */
        tcpVectPtr->tcpsThreadCount++;
/* To avoid deadlock with locking listening socket */
        tm_call_unlock(&socketPtr->socLockEntry);
#ifdef TM_ERROR_CHECKING
        if (listenSocketPtr == TM_SOCKET_ENTRY_NULL_PTR)
        {
            tfKernelError("tfTcpEstablish", "Corrupted passive socket");
        }
        else
#endif /* TM_ERROR_CHECKING */
        {
            tm_call_lock_wait(&listenSocketPtr->socLockEntry);
            needUnlockListeningSocket = TM_8BIT_YES;
            tm_call_lock_wait(&socketPtr->socLockEntry);
/*
 * Decrease the threadCount (with connection socket
 * && listening socket lock on), to prevent race conditions.
 * tcpsThreadCount has to be decreased with the listening socket lock on,
 * and before we check whether we need to free the listening socket.
 * Connection socket lock only necessary if more than one recv interface
 * task were to process incoming data for the same connection (very unlikely).
 */
            tcpVectPtr->tcpsThreadCount--;
/* Move TCP vector from half connected queue, to fully connected queue. */
/* Remove from half connected queue */
            removed = tfTcpConQueueRemove(tcpVectPtr, TM_TCP_SYN_QUEUE);
            if (removed)
/* If TCP Vector was in half connected queue. */
            {
                listenTcpVectPtr = (ttTcpVectPtr)(ttVoidPtr)listenSocketPtr;
/* Update SYN flood count */
                listenTcpVectPtr->tcpsSynFloodCount--;
/* Insert in ready to be accepted queue. */
                tfTcpConQueueInsert(listenTcpVectPtr, tcpVectPtr,
                                    TM_TCP_ACCEPT_QUEUE);
            }
/*
 * socketPtr needs to be unlocked before the listening socket, to
 * prevent deadlock. (listenSocketPtr is unlocked (and relocked)
 * in tfSocketNotify(), or listenSocketPtr is unlocked in
 * tfSocketFree()).
 */
            if (     socketPtr->socListenSocketPtr
                  == listenSocketPtr )
            {
                tm_call_unlock(&socketPtr->socLockEntry);
/* Listening socket has not been closed:
 * increase listen socket owner count since listen socket is unlocked in
 * tfSocketNotify(). We will decrease it in common code below.
 */
                (listenSocketPtr->socOwnerCount)++;
                tfSocketNotify(listenSocketPtr, TM_SELECT_READ,
                               TM_CB_ACCEPT, 0);
            }
            else
            {
                tm_call_unlock(&socketPtr->socLockEntry);
/*
 * else
 * Listening socket has been closed, or the connection has been reset.
 * We will also decrease the listening socket ownership count since our
 * increasing tcpsThreadCount prevented its decrease in tfTcpRemoveConReq().
 * Common code below.
 */
            }
            --(listenSocketPtr->socOwnerCount);
/*
 * Check ownership count on listening socket, since we just decreased it,
 * and free it if ownership count dropped to 0.
 */
            if (listenSocketPtr->socOwnerCount == 0)
            {
                tfSocketFree(listenSocketPtr);
                needUnlockListeningSocket = TM_8BIT_ZERO;
            }
        }
        if (needUnlockListeningSocket)
        {
/* Listening socket has not been freed. Unlock it. */
            tm_call_unlock(&(listenSocketPtr)->socLockEntry);
        }
        tm_call_lock_wait(&socketPtr->socLockEntry);
    }
    else
/* connect socket (open, or simultaneous open) */
    {
#ifdef TM_USE_SOCKET_IMPORT
tcpEstablishSocImp1:
#endif /* TM_USE_SOCKET_IMPORT */
        tcpVectPtr->tcpsSelectFlags |= TM_SELECT_WRITE;
        tcpVectPtr->tcpsSocketCBFlags |= TM_CB_CONNECT_COMPLT;
        tm_8bit_clr_bit( socketPtr->socFlags2, TM_SOCF2_CONNECTING );
/* ANVL-CORE 4.21. */
/* Reset the connect error code */
        tfSocketErrorRecord((int)socketPtr->socIndex, TM_ENOERROR);
    }
    if (    (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_WAIT_SND_FIN))
#ifdef TM_USE_SSL_CLIENT
         && (    (tm_16bit_bits_not_set(tcpVectPtr->tcpsSslFlags,
                                        TM_SSLF_SEND_ENABLED))
              || (tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                                       TM_SSLF_ESTABLISHED))
              || (tcpVectPtr->tcpsSslSendQBytes == TM_32BIT_ZERO) )
/*
 * On SSL, postpone sending the FIN until SSL is connected if
 * data is queued.
 */
#endif /* TM_USE_SSL_CLIENT */
       )
    {
/*
 * If we had postponed sending the FIN until we reach the established state,
 * send it now.
 */
        tm_16bit_clr_bit( tcpVectPtr->tcpsFlags, TM_TCPF_WAIT_SND_FIN );
        tcpVectPtr->tcpsState = TM_TCPS_FIN_WAIT_1;
/* ANVL 6.30 add TM_TCPF2_SEND_DATA to force TCP send call */
        tcpVectPtr->tcpsFlags2 |= (TM_TCPF2_SND_FIN | TM_TCPF2_SEND_DATA);
#ifdef TM_SNMP_MIB
/*
 * number of TCP connections for which the current state
 * is either ESTABLISHED or CLOSE-WAIT."
 */
        tm_context(tvTcpData).tcpCurrEstab--;
#endif /* TM_SNMP_MIB */
    }
    if (socketPtr->socSendQueueBytes != TM_32BIT_ZERO)
/* ANVL-CORE 5.25. Send data queued during SYN-RECEIVED state. */
    {
        tcpVectPtr->tcpsFlags2 |= TM_TCPF2_SEND_DATA;
    }
    return TM_ENOERROR;
}
#ifdef TM_LINT
LINT_NULL_PTR_END(listenSocketPtr)
#endif /* TM_LINT */

/*
 * tfTcpSynData function description:
 * Called from the TCP state machine when processing an incoming SYN TCP
 * packet to check for SYN data.
 * Put received SYN data (if any) in reassembly queue (including urgent
 * data and FIN) up to receive queue size.
 * Parameter    description
 * tcpVectPtr   pointer to targeted TCP state vector (including socket entry).
 * packetPtr    pointer to incoming data packet pointer (as prepared in
 *              tfTcpIncomingPacket step 10).
 * newState     new TCP state to transition to.
 * Return values
 * TM_ENOERROR.
 */
static int tm_state_function (tfTcpSynData, tcpVectPtr, packetPtr, newState)
{
    ttTcpHeaderPtr      tcphPtr;
    ttSocketEntryPtr    socketPtr;
    ttPacketPtr         oldPacketPtr;
    tt32Bit             rcvWnd;
    tt32Bit             tcpLength;
    tt8Bit              finByte;

    TM_UNREF_IN_ARG(newState);
    tcphPtr = tm_tcp_hdr_ptr(packetPtr);
    if (tcphPtr->tcpControlBits & TM_TCP_FIN)
    {
        finByte = (tt8Bit)1;
    }
    else
    {
        finByte = TM_8BIT_ZERO;
    }
    tcpLength = packetPtr->pktChainDataLength + finByte;
    if (tcpLength != TM_UL(0))
    {
        socketPtr = &(tcpVectPtr->tcpsSocketEntry);
        rcvWnd = tm_tcp_soc_recv_left(socketPtr, tcpVectPtr);
/* Tail trim data sent with the SYN and not within the receive window */
#ifdef TM_ERROR_CHECKING
        if (rcvWnd == TM_UL(0))
        {
            tfKernelError("tfTcpSynData",
                          "Receive queue full, or not initialized");
            tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
        }
        else
#endif /* TM_ERROR_CHECKING */
        {
            if (tcpLength > rcvWnd)
            {
                if (finByte != TM_8BIT_ZERO)
                {
                    rcvWnd--;
                    tm_8bit_clr_bit( tcphPtr->tcpControlBits, TM_TCP_FIN );
                }
                tfPacketTailTrim(packetPtr,
                                 (ttPktLen)rcvWnd,
                                 TM_SOCKET_LOCKED);
            }
            tcphPtr->tcpSeqNo++; /* over SYN byte */
            if (tm_8bit_one_bit_set(tcphPtr->tcpControlBits, TM_TCP_URG))
            {
                tcphPtr->tcpUrgentPtr++; /* over SYN byte */
/* If urgent data, process it */
                (void)tfTcpRcvUrgentData(tcpVectPtr, packetPtr);
            }
            oldPacketPtr = tcpVectPtr->tcpsReassemblePtr;
/* Should be first in reassembly queue, but just in case */
            if (oldPacketPtr != TM_PACKET_NULL_PTR)
            {
                tfFreePacket(oldPacketPtr, TM_SOCKET_LOCKED);
            }
            tcpVectPtr->tcpsReassemblePtr = packetPtr;
            packetPtr->pktSharedDataPtr->dataFlags |= TM_BUF_TCP_IN_QUEUE;
        }
    }
    return TM_ENOERROR;
}

/*
 * tfTcpSendSynAck function description
 * Called from the TCP state machine when we need to send a SYN-ACK.
 * Generate options and send packet.
 * Ignore error from tfTcpCreateSendPacket, and let the retransmit logic do
 * its work.
 * Parameter     description
 * tcpVectPtr    pointer to TCP state vector.
 * packetPtr     ignored.
 * newState      ignored.
 * Return values
 * TM_ENOERROR   no error
 */
static int tm_state_function (tfTcpSendSynAck, tcpVectPtr, packetPtr,
                              newState)
{
    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(newState);
/*
 * Generate options and send packet.
 * Ignore error from tfTcpCreateSendPacket, and let the retransmit logic do
 * its work.
 */
     (void)tfTcpCreateSendPacket(tcpVectPtr,
                                 TM_PACKET_NULL_PTR,
                                 tcpVectPtr->tcpsSndUna,
                                 tcpVectPtr->tcpsRcvNxt,
                                 TM_TCP_SYN|TM_TCP_ACK);
    return TM_ENOERROR;
}

/*
 * tfTcpRcvSyn function description:
 * Called from the TCP state machine when processing an incoming SYN TCP
 * packet.
 * process SYN options, initialize receive sequence numbers.
 * Parameter    description
 * tcpVectPtr   pointer to targeted TCP state vector (including socket entry).
 * packetPtr    pointer to incoming data packet pointer (as prepared in
 *              tfTcpIncomingPacket step 10).
 * newState     new TCP state to transition to.
 * Return values
 * TM_ENOERROR.
 */
static int tm_state_function (tfTcpRcvSyn, tcpVectPtr, packetPtr, newState)
{
    tcpVectPtr->tcpsState = newState;
/* process SYN options, initialize receive sequence numbers */
    (void)tfTcpRcvSynOptions(packetPtr,
                             tcpVectPtr->tcpsFlags,
                             tcpVectPtr);
    return TM_ENOERROR;
}

/*
 * tfTcpNotifyConnResetFree function description:
 * Called from the TCP state machine when we receive a RST in SYN_SENT state
 * or in SYN_RECEIVED state:
 * . In SYN_SENT ignore the RST if there is no ACK or the ACK is not
 *   acceptable, otherwise signal the user.
 * . In SYN_RECEIVED state, signal the user only if the open was active.
 * (Note signal occurs, in next state machine function call (tfTcpVectClose)))
 * . Free the incoming packet.
 * Parameter    description
 * tcpVectPtr   pointer to targeted TCP state vector (including socket entry).
 * packetPtr    pointer to incoming data packet pointer (as prepared in
 *              tfTcpIncomingPacket step 10).
 * newState     no state transition.
 * Return values
 * TM_ENOERROR.
 */
static int tm_state_function (tfTcpNotifyConnResetFree, tcpVectPtr,
                              packetPtr, newState)
{
    ttTcpHeaderPtr      tcphPtr;
    ttSqn               segAck;
    int                 errorCode;

    TM_UNREF_IN_ARG(newState);
    tcphPtr = tm_tcp_hdr_ptr(packetPtr);
    errorCode = TM_ENOERROR;
    if (tcpVectPtr->tcpsState == TM_TCPS_SYN_SENT)
/* SYN_SENT state, check if the ACK is acceptable */
    {
/* If SND.UNA =< SEG.ACK =< SND.NXT then the Reset ACK is acceptable. */
        segAck = tcphPtr->tcpAckNo;
        if (    tm_8bit_one_bit_set(tcphPtr->tcpControlBits, TM_TCP_ACK)
             && tm_tcp_valid_ack(tcpVectPtr->tcpsSndUna,
                                 segAck, tcpVectPtr->tcpsMaxSndNxt) )
        {
/* Notify pending connect (done in tfTcpVectClose) */
            tcpVectPtr->tcpsAbortError = TM_ECONNREFUSED;
        }
        else
        {
/* No ACK, or invalid ACK, ignore the reset, do not transition to CLOSED */
            errorCode = TM_EINVAL; /* invalid Reset ack, ignore */
        }
    }
    else
/* SYN_RECEIVED state */
    {
/* Notify pending connect for an active open */
        if (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags, TM_TCPF_PASSIVE))
        {
/* Notify pending connect (done in tfTcpVectClose) */
            tcpVectPtr->tcpsAbortError = TM_ECONNREFUSED;
        }
    }
    tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
    return errorCode;
}

/*
 * tfTcpNotifyReset function description:
 * Called from the TCP state machine when we receive a bad segment in a TCP
 * state >= SYN_RECEIVED.
 * . Set the abort errror to TM_ECONNRESET to signal the user.
 * (Note signal occurs, in next state machine function call (tfTcpVectClose)))
 * Parameter    description
 * tcpVectPtr   pointer to targeted TCP state vector (including socket entry).
 * packetPtr    pointer to incoming data packet pointer (as prepared in
 *              tfTcpIncomingPacket step 10).
 * newState     no state transition.
 * Return values
 * TM_ENOERROR.
 */
static int tm_state_function (tfTcpNotifyReset, tcpVectPtr, packetPtr,
                              newState)
{
    TM_UNREF_IN_ARG(newState);
    TM_UNREF_IN_ARG(packetPtr);
    tcpVectPtr->tcpsAbortError = TM_ECONNRESET;
    return TM_ENOERROR;
}

/*
 * tfTcpNotifyResetFree function description:
 * Called from the TCP state machine when we receive a RST in a TCP state
 * >= established.
 * . Set the abort errror to TM_ECONNRESET to signal the user.
 * (Note signal occurs, in next state machine function call (tfTcpVectClose)))
 * . Free the packet
 * Parameter    description
 * tcpVectPtr   pointer to targeted TCP state vector (including socket entry).
 * packetPtr    pointer to incoming data packet pointer (as prepared in
 *              tfTcpIncomingPacket step 10).
 * newState     no state transition.
 * Return values
 * TM_ENOERROR.
 */
static int tm_state_function (tfTcpNotifyResetFree, tcpVectPtr, packetPtr,
                              newState)
{
    TM_UNREF_IN_ARG(newState);
    tcpVectPtr->tcpsAbortError = TM_ECONNRESET;
    tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
#ifdef TM_USE_SSL
/*
 * If RST is received but we didn't get SSL Close_notify before, we MUST not resume
 * this session. If this session is already cached, we need to remove it later
 * ( Note check on null tcpsSslConnStatePtr not necessary (non null if
 *   TM_SSLF_RECV_ENABLED is set))
 */
    if (    (tcpVectPtr->tcpsSslFlags & TM_SSLF_RECV_ENABLED)
         && (tcpVectPtr->tcpsSslConnStatePtr != (ttVoidPtr)0) )
    {
        ((ttSslConnectStatePtr)tcpVectPtr->tcpsSslConnStatePtr)
                      ->scsRunFlags |= TM_SSL_RUNFLAG_IMCOMP_CLOSE;
    }
#endif /* TM_USE_SSL */
    return TM_ENOERROR;
}

/*
 * tfTcpSendRstSeq0 function description:
 * Called from the state machine when a SYN segment is received in the
 * TCP closed state, or when a listening server refuses a connection in
 * tfTcpConReq().
 * . Send a reset of the form SeQ=0 ACK=SegSeq+SegLen CTL=RST,ACK
 * . No state transition.
 * Parameter    description
 * tcpVectPtr   pointer to TCP state vector (including socket entry).
 * packetPtr    pointer to incoming data packet pointer (as prepared in
 *              tfTcpIncomingPacket step 10).
 * newState     no state transition.
 * Return values
 * TM_ENOERROR.
 */
static int tm_state_function (tfTcpSendRstSeq0,  tcpVectPtr,  packetPtr,
                              newState)
{
    ttTcpHeaderPtr      tcphPtr;
    ttSqn               ackNo;
    tt8Bit              tcpCtlBits;

    TM_UNREF_IN_ARG(newState);
    tcphPtr = tm_tcp_hdr_ptr(packetPtr);
    ackNo = tcphPtr->tcpSeqNo;
/* Ack data length */
    ackNo += packetPtr->pktChainDataLength;
    tcpCtlBits = tcphPtr->tcpControlBits;
/* Ack SYN and FIN bits if any */
    if (tm_8bit_one_bit_set(tcpCtlBits, TM_TCP_FIN|TM_TCP_SYN))
    {
/* At least one of them is set */
        ackNo++;
        if (tm_8bit_all_bits_set(tcpCtlBits, TM_TCP_FIN|TM_TCP_SYN))
        {
/* Both of them are set */
            ackNo++;
        }
    }
    (void)tfTcpCreateSendPacket(tcpVectPtr, packetPtr,
                                (ttSqn)0, ackNo, TM_TCP_RST|TM_TCP_ACK);
    return TM_ENOERROR;
}

/*
 * tfTcpSendRst function description:
 * Called from the state machine to abort the connetion or when the user
 * wants to abort the connection.
 * if PacketPtr is non null Send a reset of the form SeQ=SegAck CTL=RST
 * Otherwise (ABORT) send a reset of the form SeQ=SndNxt CTL=RST
 * Parameter    description
 * tcpVectPtr   pointer to TCP state vector (including socket entry).
 * packetPtr    pointer to incoming data packet pointer (as prepared in
 *              tfTcpIncomingPacket step 10), or null.
 * newState     new state to transition to.
 * Return values
 * TM_ENOERROR.
 */
static int tm_state_function (tfTcpSendRst, tcpVectPtr, packetPtr, newState)
{
    ttSqn  seqNo;

    TM_UNREF_IN_ARG(newState);
    if (packetPtr != TM_PACKET_NULL_PTR)
/* Incoming segment makes us send a reset */
    {
        seqNo = tm_tcp_hdr_ptr(packetPtr)->tcpAckNo;
    }
    else
/* called by ABORT */
    {
        seqNo = tcpVectPtr->tcpsSndNxt;
    }
#ifdef TM_USE_SSL
    tfTcpSslClose(tcpVectPtr, 0); /* no abort on failure */
#endif /* TM_USE_SSL */
#ifdef TM_USE_STOP_TRECK
    if (tm_8bit_bits_not_set(tcpVectPtr->tcpsSocketEntry.socFlags2,
                             TM_SOCF2_UNINITIALIZING))
#endif /* TM_USE_STOP_TRECK */
    {
        (void)tfTcpCreateSendPacket(tcpVectPtr, packetPtr,
                                    seqNo, (ttSqn)0, TM_TCP_RST);
    }
    return TM_ENOERROR;
}

/*
 * tfTcpRcvAck() function description
 * This function is a state function called from the TCP state machine
 * started in tfTcpIncomingPacket() when the state of the connection is
 * between ESTABLISHED and FIN_WAIT_2 and when the header prediction code
 * fails.
 * We first check on the validity of the ACK field of the received packet.
 * . If the ACK is a duplicate, we check on Vegas retransmission trigger,
 *   for fast retransmission. If Vegas retransmission occurs, We also
 *   perform RFC 2001 congestion avoidance, and fast recovery.
 * . If we are acking new data, we check whether we are getting the ACK
 *   of our FIN (if we had sent one), we free the ACKed buffers, update
 *   send queue, update RTT (since we are advancing the left edge of the
 *   send window). We then perform RFC 2001, congestion avoidance, and
 *   fast recovery algorithm. Then, if all data has been acknowledged,
 *   we cancel the retransmission timer, otherwise we check on the other
 *   Vegas retransmission trigger (for first and second ACK after a
 *   retransmission has occured), to determine whether we need to
 *   retransmit a segment.
 * If state of the connection is Established, FIN_WAIT_1, or FIN_WAIT_2,
 * we then check if the segment contains urgent data, and/or text data,
 * and/or FIN, queue the in order data to the socket, and set a flag
 * to queue a FIN to the state machine if a FIN is in order, and next to be
 * processed.
 *
 * Parameters
 * tcpVectPtr   pointer to targeted TCP state vector (including socket entry).
 * packetPtr    pointer to incoming data packet pointer (as prepared in
 *              tfTcpIncomingPacket step 10).
 * newState     New Tcp State to transition to. Ignored in this routine.
 * Return value:
 * Error code if further processing of the packet in the state machine
 * is not allowed, 0 otherwise.
 */
static int tm_state_function (tfTcpRcvAck, tcpVectPtr, packetPtr, newState)
{
    ttTcpHeaderPtr              tcphPtr;
    ttPacketPtr                 sendQPacketPtr;
    ttSocketEntryPtr            socketPtr;
    tt32Bit                     ackLength;
    tt32Bit                     dataLength;
    tt32Bit                     windowSize;
    tt32Bit                     currentTime;
    tt32Bit                     packetTime;
    tt32Bit                     ssthresh;
    ttSqn                       segAck;
    ttS32Bit                    rtt;
    int                         errorCode;
    tt8Bit                      needFreePacket;
    tt8Bit                      curState;
    tt8Bit                      urgentData;
    tt8Bit                      tcpCtlBits;
    tt8Bit                      computeRtoFlag;
#define lSendSegSize   windowSize
#define lCwnd          windowSize

    TM_UNREF_IN_ARG(newState);
    tcphPtr = tm_tcp_hdr_ptr(packetPtr);
    tcpCtlBits = tcphPtr->tcpControlBits;
    segAck = tcphPtr->tcpAckNo;
    needFreePacket = TM_8BIT_YES;
    errorCode = TM_ENOERROR;
    dataLength = packetPtr->pktChainDataLength;
    tm_kernel_set_critical;
    currentTime = tvTime;
    tm_kernel_release_critical;
    computeRtoFlag = TM_8BIT_YES;
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
/* If time stamp is on, compute round trip time */
    if ( tm_16bit_one_bit_set( packetPtr->pktSharedDataPtr->dataFlags,
                               TM_BUF_TCP_TS_ON ) )
    {
        packetTime = packetPtr->pktTcpTsEcr;
        rtt = (ttS32Bit)(currentTime - packetTime);
    }
    else
    {
/* time stamp not on, compute round trip time using packet stamp */
        sendQPacketPtr = socketPtr->socSendQueueNextPtr;
        if (sendQPacketPtr != TM_PACKET_NULL_PTR)
        {
            packetTime = sendQPacketPtr->pktTcpXmitTime;
            rtt = (ttS32Bit)(currentTime - packetTime);
            if ( tm_16bit_one_bit_set( sendQPacketPtr->pktFlags2,
                                       TM_PF2_TCP_REXMIT ) )
            {
/* Ignore rtt if packet has been re-transmitted (KARN algorithm) */
                computeRtoFlag = TM_8BIT_ZERO;
            }
        }
        else
/* Duplicate ACK for a SYN, or a FIN and no outstanding data */
        {
/* +1, to prevent congestion window shrinkage */
            packetTime = tcpVectPtr->tcpsCwndAge + 1;
/*
 * + 1, just to force retransmission of FIN/SYN, if appropriate.
 * (tfTcpSenQueueFree() not called)
 */
            rtt = (ttS32Bit)(tcpVectPtr->tcpsRto + 1);
        }
    }
/* If we are not acking any new data (not acceptable) */
    if (tm_sqn_leq(segAck, tcpVectPtr->tcpsSndUna))
    {
/*
 * Check if the ACK is a duplicate:
 * if ACK is as big as any previous ACK (sndUna) (valid ACK)
 */
        if (    (segAck == tcpVectPtr->tcpsSndUna)
/*
 * and if all of our data has not been acknowledged, and we are not probing
 * a zero window (retransmit flag on).
 */
             && (tm_16bit_one_bit_set( tcpVectPtr->tcpsFlags2,
                                       TM_TCPF2_REXMIT_MODE))
/* and peer is not sending any new data (including FIN) */
            &&  (dataLength == TM_UL(0))
            &&  (tm_8bit_bits_not_set(tcpCtlBits, TM_TCP_FIN))
/* and if peer is not sending a window update */
            &&  (packetPtr->pktTcpUnscaledWnd == tcpVectPtr->tcpsSndWnd) )
        {
/*
 * Use Vegas retransmission algorithm:
 * Vegas treats the receipt of certain ACK's (duplicate ACKs) as a trigger
 * to check if a timeout should happen:
 * Compute time difference between current time and time of relevant
 * segment (first) in send queue. If bigger > retransmission time then
 * retransmit:
 */
            tcpVectPtr->tcpsDupAcks++;
            if ( tm_16bit_bits_not_set( tcpVectPtr->tcpsFlags2,
                                        TM_TCPF2_CONGESTED ) )
            {
                if (    ((tt32Bit)rtt > tcpVectPtr->tcpsRto)
/* Added also check for 3 duplicate ACKS in case Rto becomes too big */
                     || (tcpVectPtr->tcpsDupAcks >= 3)
#ifdef TM_TCP_FACK
/* Added check for Forward ACK beyond 3 segments */
                     || (    (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags,
                                                   /* Not needed */
                                                   TM_TCPF_SEL_ACK) )
                          && (   (tt32Bit)(   tcpVectPtr->tcpsSndFack
                                            - tcpVectPtr->tcpsSndUna )
                               > (tt32Bit)(3 * tcpVectPtr->tcpsEffSndMss) ) )
#endif /* TM_TCP_FACK */
                     )
                {
/*
 * RFC 2001 (congestion avoidance, and fast recovery) modified with
 * Vegas retransmission trigger algorithm:
 *   1. if the RTT computed as the difference between the arrival time of
 *      the duplicate ACK, and the sent time of the missing (first) segment
 *      in the send queue is bigger than the RTO (Vegas trigger), or we
 *      have received a third duplicate ACK, set
 *      ssthresh to one-half the current congestion window, cwnd, but no less
 *      than two segments.  Retransmit the missing segment.  Set cwnd to
 *      ssthresh plus the segment size times the number of duplicate ACKS.
 *      This inflates the congestion window by the number of segments that
 *      have left the network and which the other end has cached (dupAcks).
 * ( Note that since we use the Vegas retransmission trigger or
 *  the third duplicate ACK in a row of RFC 2001, we count the number of
 *  duplicate ACKS which may or may not be 3, and have replaced the fixed
 *  value (3) with the actual number of duplicate ACKS (dupAcks)).
 */
                    tcpVectPtr->tcpsAcksAfterRexmit = (tt8Bit)1;
/*
 * We use one-half of the current window size (the minimum of cwnd and
 * the receiver's advertised window), instead of one-half of cwnd:
 */
                    if ( tm_16bit_one_bit_set( tcpVectPtr->tcpsFlags,
                                                  TM_TCPF_SLOW_START ) )
/* If user did not turn off slow start */
                    {
/*
 * RFC 2581 ssthresh = max(FlightSize /2, 2*SMSS) where flight size is
 * outsdanding data in the network, i.e. miminum of send window and
 * congestion window.
 */
                        windowSize = tcpVectPtr->tcpsSndWnd;
                        if (windowSize > tcpVectPtr->tcpsCwnd)
                        {
                            windowSize = tcpVectPtr->tcpsCwnd;
                        }
                        windowSize = windowSize / 2;
/* No less than 2 segments */
                        ssthresh = 2 * tcpVectPtr->tcpsEffSndMss;
                        if (windowSize > ssthresh)
                        {
                            ssthresh = windowSize;
                        }
/*
 * RFC 2581 Retransmit the lost segment and set cwnd to ssthresh plus 3*SMSS.
 * This artificially "inflates" the congestion window by the number
 * of segments (three) that have left the network and which the
 * receiver has buffered. (Same as RFC 2001).
 *
 * Set the congestion window so that we can fill the pipe (the remote
 * side sends a duplicate ACK for every out of order packet received, so
 * mss*dupAcks is the number of out of order bytes that the remote has
 * cached already. We set our congestion window to slow start threshold
 * + number of cached bytes by the remote).
 */
                        lCwnd =   ssthresh + (   tcpVectPtr->tcpsEffSndMss
                                               * tcpVectPtr->tcpsDupAcks );
                        if (   tm_ts_geq( packetTime,
                                          tcpVectPtr->tcpsCwndAge )
                             && (lCwnd < tcpVectPtr->tcpsCwnd) )
/*
 * If packet was originally transmitted after the last decrease in
 * congestion window, and we are decreasing the window. (If a packet got
 * lost before the last window decrease, it does not mean that the current
 * shrinked window is at fault, so we do not need to decrease the window
 * again.)
 */
                        {
                            tcpVectPtr->tcpsSsthresh = ssthresh;
#ifdef TM_TCP_FACK
/* FACK Data smoothing */
                            if ( tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags,
                                                      TM_TCPF_SEL_ACK) )
                            {
/* Value to be added to congestion window during recovery phase */
                                tcpVectPtr->tcpsWinTrim = (ttS32Bit)
                                      ((   tcpVectPtr->tcpsMaxSndNxt
                                         - tcpVectPtr->tcpsSndFack ) / 2);
                            }
#endif /* TM_TCP_FACK */
                            tcpVectPtr->tcpsCwnd = lCwnd;
                            tcpVectPtr->tcpsCwndAge = currentTime;
                        }
                    }
/*
 * Retransmit missing segment only (segAck which is tcpVectPtr->tcpsSndUna)
 * Force re-initialization of retransmit timer interval (in tfTcpSendPacket)
 * Set the congested flag.
 */
                    tcpVectPtr->tcpsFlags2 |= (   TM_TCPF2_REINIT_TIMER
                                                | TM_TCPF2_REXMIT_TEMP
                                                | TM_TCPF2_CONGESTED );
#ifdef TM_TCP_FACK
/* Highest SQN (+1) sent to the peer at the time of congestion */
                    tcpVectPtr->tcpsCongSndNxt = tcpVectPtr->tcpsMaxSndNxt;
/* Initialize count of retransmitted data during recovery phase. */
                    tcpVectPtr->tcpsRetranData = TM_UL(0);
#endif /* TM_TCP_FACK */
                }
#ifdef TM_TCP_RFC3042
                else
                {
/*
 * [RFC3042]R2:1
 * If number of duplicate ACKS stored in tcpsDupAcks is 2, set the
 * TM_TCPF_SEND_DATA flag that indicates that we want to send data, and
 * a new flag that will indicate that we want to send 2 segments beyond
 * cwnd at sndNxt, i.e. TM_TCPF2_SINGLE_DUP.
 */
                     if (tcpVectPtr->tcpsDupAcks == 2)
                     {
                        tcpVectPtr->tcpsFlags2 = (tt16Bit)
                             (   tcpVectPtr->tcpsFlags2
                               | (TM_TCPF2_SEND_DATA | TM_TCPF2_SINGLE_DUP) );
                     }
                }
#endif /* TM_TCP_RFC3042 */
            }
            else
            {
/*
 * RFC 2001 (fast recovery):
 *   2.  Each time another duplicate ACK arrives, increment cwnd by the
 *      segment size.  This inflates the congestion window for the
 *      additional segment that has left the network.  Transmit a
 *      packet, if allowed by the new value of cwnd.
 * End of RFC 2001.
 * RFC 2581 (fast recovery): same as 2001 except transmit a segment
 *      if allowed by the new value of cwnd and receiver's advertised window.
 * (Explanation: we already determined that the network was congested, and
 * retransmitted the missing segment. Just send more data ahead to fill the
 * pipe.)
 */
                if (   tm_16bit_one_bit_set( tcpVectPtr->tcpsFlags,
                                           TM_TCPF_SLOW_START )
#ifdef TM_TCP_FACK
                    && (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags,
                                              TM_TCPF_SEL_ACK))
#endif /* TM_TCP_FACK */
                    )
                {
/* If user did not turn off slow start, && not using FACK algorithm
 * (FACK algorithm keeps cwnd constant during recovery).
 */
                    tcpVectPtr->tcpsCwnd += tcpVectPtr->tcpsEffSndMss;
                }
/* Try and send more data (at the end of tfTcpIncomingPacket()) */
                if (socketPtr->socSendQueueBytes != TM_32BIT_ZERO)
                {
                    tcpVectPtr->tcpsFlags2 |= TM_TCPF2_SEND_DATA;
                }
            }
        }
        else
        {
/* Not a duplicate ACK, and not acking new data */
            tcpVectPtr->tcpsDupAcks = 0;
/* Do not get out of recovery mode yet. */
        }
    }
/* else: we are acking new data (acceptable) */
    else
    {
/* Check that it is valid */
        if (tm_sqn_gt(segAck, tcpVectPtr->tcpsMaxSndNxt))
        {
/* Invalid ACK, Ack and exit */
            tcpVectPtr->tcpsFlags2 |= TM_TCPF2_ACK;
            errorCode = TM_EINVAL;
            goto rcvAckExit;
        }
/* length of ACKED data (could include ACK of our SYN, and/or ACK of our FIN) */
        ackLength = segAck - tcpVectPtr->tcpsSndUna;
/* ANVL-CORE 1.19 */
        if (ackLength > socketPtr->socSendQueueBytes)
/* End of ANVL-CORE 1.19 */
/*
 * Either:
 * 1. ANVL-CORE 1.19: case of FIN_WAIT_1 state (directly from SYN_RECEIVED)
 *    our SYN or SYN+FIN is being ACKed
 * or
 * 2. FIN is being acked
 */
        {
            if (    (tcpVectPtr->tcpsSndUna == tcpVectPtr->tcpsIss)
/*
 * ANVL-CORE 1.19: (case of FIN_WAIT_1 state (directly from SYN_RECEIVED))
 * ACK seems to be an ACK of our SYN, since our Snd.Una is still at ISS.
 * Make sure that this is not a case of SQN wrap around, by checking that the
 * socket is not yet connected.
 */
                 && (tm_16bit_bits_not_set(socketPtr->socFlags,
                                           TM_SOCF_CONNECTED)) )
            {
/* Socket is now connected */
                socketPtr->socFlags |= TM_SOCF_CONNECTED;
/*
 * Connection is established, turn off connection timeout.
 */
                tm_tcp_timer_remove(tcpVectPtr, TM_TCPTM_CONN);
                ackLength--;
/* End of ANVL-CORE 1.19 */
            }
            if (    (tm_16bit_one_bit_set( tcpVectPtr->tcpsFlags2,
                                           TM_TCPF2_FIN_SENT ))
                 && (ackLength > socketPtr->socSendQueueBytes) )
/* Our FIN is being acked  */
            {
                ackLength = socketPtr->socSendQueueBytes;
                tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2,
                                  (  TM_TCPF2_SND_FIN
                                   | TM_TCPF2_FIN_SENT
                                   | TM_TCPF2_SEND_DATA) );
            }
        }
#ifdef TM_TCP_SACK
        if ( tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_SEL_ACK) )
        {
            tfTcpSackDeQueueBlocks( tcpVectPtr,
                                    tcpVectPtr->tcpsSndUna,
                                    segAck,
                                    TM_TCP_SACK_RECV_INDEX );
        }
#endif /* TM_TCP_SACK */
/* Advance the left edge of the send window */
        tcpVectPtr->tcpsSndUna = segAck;
/* If retransmit timer moved our sndNxt back, change it to segAck */
        if (tm_sqn_lt(tcpVectPtr->tcpsSndNxt, segAck))
        {
            tcpVectPtr->tcpsSndNxt = segAck;
        }
/*
 * Free ACKed buffers, update send queue, update RTT (since we
 * are advancing the left edge of the send window). Check for zero
 * value which could occur if we get just the ACK of our FIN.
 */
        if (ackLength != 0)
        {
            tfTcpSendQueueFree(tcpVectPtr, ackLength, rtt, computeRtoFlag);
        }
        if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags2, TM_TCPF2_CONGESTED))
        {
/*
 * RFC 2001 (fast retransmit and congestion avoidance):
 *   3.  When the next ACK arrives that acknowledges new data, set cwnd
 *      to ssthresh (the value set in step 1).  This ACK should be the
 *      acknowledgment of the retransmission from step 1, one round-trip
 *      time after the retransmission.  Additionally, this ACK should
 *      acknowledge all the intermediate segments sent between the lost
 *      packet and the receipt of the first duplicate ACK.  This step is
 *      congestion avoidance, since TCP is down to one-half the rate it
 *      was at when the packet was lost.
 * FACK: but partial ACK should not get us out of recovery mode, and into
 *       congestion avoidance yet.
 */
#ifdef TM_TCP_FACK
            if (    (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags,
                                           TM_TCPF_SEL_ACK)) /* needed */
                 || (tm_sqn_geq(tcpVectPtr->tcpsSndUna,
                                tcpVectPtr->tcpsCongSndNxt))
/*
 * Fack algorithm:
 * Non partial ACK (sndUna bigger than maximum sqn when congestion occured)
 * ==> OK to get out of recovery mode and into congestion avoidance
 */
               )
#endif /* TM_TCP_FACK */
            {
                if ( tm_16bit_one_bit_set( tcpVectPtr->tcpsFlags,
                                           TM_TCPF_SLOW_START ) )
/* If user did not turn off slow start */
                {
                    if (tcpVectPtr->tcpsCwnd > tcpVectPtr->tcpsSsthresh)
                    {
/* Set back congestion window to slow start threshold */
                        tcpVectPtr->tcpsCwnd = tcpVectPtr->tcpsSsthresh;
                        tcpVectPtr->tcpsCwndAge = currentTime;
                    }
                }
/* If congested earlier, reset flag => out of recovery mode */
                tm_16bit_clr_bit(tcpVectPtr->tcpsFlags2, TM_TCPF2_CONGESTED);
            }
        }
/* OL 4/24/01: moved re-init of dup acks out of previous if statement. */
/* Reset number of consecutive duplicate acks. */
        tcpVectPtr->tcpsDupAcks = 0;
/*
 * RFC 2001 (congestion avoidance):
 *  4. When new data is acknowledged by the other end, increase cwnd,
 *     but the way it increases depends on whether TCP is performing
 *     slow start or congestion avoidance.
 *
 *     If cwnd is less than or equal to ssthresh, TCP is in slow start;
 *     otherwise TCP is performing congestion avoidance.  Slow start
 *     continues until TCP is halfway to where it was when congestion
 *     occurred (since it recorded half of the window size that caused
 *     the problem in step 2), and then congestion avoidance takes over.
 *
 *     Slow start has cwnd begin at one segment, and be incremented by
 *     one segment every time an ACK is received.  As mentioned earlier,
 *     this opens the window exponentially:  send one segment, then two,
 *     then four, and so on.  Congestion avoidance dictates that cwnd be
 *     incremented by segsize*segsize/cwnd each time an ACK is received,
 *     where segsize is the segment size and cwnd is maintained in bytes.
 *     This is a linear growth of cwnd, compared to slow start's
 *     exponential growth.  The increase in cwnd should be at most one
 *     segment each round-trip time (regardless how many ACKs are
 *     received in that RTT), whereas slow start increments cwnd by the
 *     number of ACKs received in a round-trip time.
 */
        if (   tm_16bit_one_bit_set( tcpVectPtr->tcpsFlags,
                                     TM_TCPF_SLOW_START )
/* If user did not turn off slow start */
#ifdef TM_TCP_FACK
/* && not using FACK algorithm
 * (Fack algorithm: need to keep cwnd constant during recovery)
 */
            && (    (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags2,
                                           TM_TCPF2_CONGESTED))
                 || (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags,
                                           TM_TCPF_SEL_ACK)) )
#endif /* TM_TCP_FACK */
           )
        {
/* assume slow start */
            lSendSegSize = tcpVectPtr->tcpsEffSndMss;
            if (tcpVectPtr->tcpsCwnd > tcpVectPtr->tcpsSsthresh)
            {
/* congestion avoidance, but not slow start */
                lSendSegSize =   lSendSegSize
                               * lSendSegSize/tcpVectPtr->tcpsCwnd;
/*
 * RFC 2581: during congestion avoidance, If the formula above fails
 * to increase the congestion window, it should be increased by 1.
 * [RFC2581]R3.1:2
 */
#ifdef TM_TCP_RFC2581
                if (lSendSegSize == 0)
                {
                    lSendSegSize = 1;
                }
#endif /* TM_TCP_RFC2581 */
            }
            lSendSegSize = tcpVectPtr->tcpsCwnd + lSendSegSize;
            tcpVectPtr->tcpsCwnd = TM_TCP_MAX_SCALED_WND <<
                                                 tcpVectPtr->tcpsSndWndScale;
            if (tcpVectPtr->tcpsCwnd > lSendSegSize)
            {
                tcpVectPtr->tcpsCwnd = lSendSegSize;
            }
        }
/*
 * If all data has been acknowledged, cancel retransmit timer but only if we
 * do not have a pending FIN to send.
 */
        if (tcpVectPtr->tcpsMaxSndNxt == segAck)
        {
/* disable retransmission timer? */
            if (tcpVectPtr->tcpsFlags2 & TM_TCPF2_REXMIT_MODE)
            {
                if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags2,
                                         (  TM_TCPF2_SND_FIN
                                          | TM_TCPF2_FIN_SENT)))
/* pending FIN to send. */
                {
#ifdef TM_USE_TCP_REXMIT_CONTROL
                   if (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags3,
                                             TM_TCPF3_REXMIT_PAUSE))
#endif /* TM_USE_TCP_REXMIT_CONTROL */
                   {
/*
 * Do not suspend the TCP retransmission timer to make sure the socket does
 * not stay indefinitely in the FIN_WAIT_1 state which would cause us
 * to leak this socket, and re-start the timeout to the RTO to quickly
 * transmit the FIN once we have finished sending all queued data.
 */
                        tm_tcp_timer_new_time(tcpVectPtr->tcpsReXmitTmPtr,
                                              tcpVectPtr->tcpsRto);
                   }
                }
                else
                {
/* disable retransmission timer */
                    tm_tcp_timer_suspend(tcpVectPtr->tcpsReXmitTmPtr);
                    tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2, TM_TCPF2_REXMIT_MODE);
                }
            }
/* Check whether we can send more data */
            if (    (socketPtr->socSendQueueBytes != TM_32BIT_ZERO)
/* ANVL-CORE 3.20 */
                 || (tm_16bit_one_bit_set( tcpVectPtr->tcpsFlags2,
                                           TM_TCPF2_SND_FIN)) )
            {
                tcpVectPtr->tcpsFlags2 |= TM_TCPF2_SEND_DATA;
            }
/* Reset Vegas retransmission check */
            tcpVectPtr->tcpsAcksAfterRexmit = TM_8BIT_ZERO;
/* Compute a fresh Cwnd Time Stamp */
            tcpVectPtr->tcpsCwndAge = currentTime;
#ifdef TM_TCP_FACK
/* Necessary if non-full size packets have been sent */
            tcpVectPtr->tcpsRetranData = TM_32BIT_ZERO;
#endif /* TM_TCP_FACK */
        }
        else
        {
/*
 * Vegas retransmission:
 * If we are not probing a zero window,
 * If tcpsAcksAfterRexmit is 1 or 2 (first or second ACK after a
 * retransmission), add 1 to it, and check the time difference between
 * current time, and time of next packet to re-send. If bigger than rtt,
 * retransmit it.
 */
            if (    (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags2,
                                          TM_TCPF2_REXMIT_MODE))
#ifdef TM_TCP_FACK
                 && (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags,
                                           TM_TCPF_SEL_ACK)) /* needed */
#endif /* TM_TCP_FACK */
                 && (    (tcpVectPtr->tcpsAcksAfterRexmit == (tt8Bit)1)
                      || (tcpVectPtr->tcpsAcksAfterRexmit == (tt8Bit)2) ) )
            {
                tcpVectPtr->tcpsAcksAfterRexmit++;
                if ((tt32Bit)rtt > tcpVectPtr->tcpsRto)
                {
/*
 * Retransmit missing segment only (segAck which is tcpVectPtr->tcpsSndUna)
 * Force re-initialization of retransmit timer interval (in tfTcpSendPacket)
 */
                    tcpVectPtr->tcpsFlags2 |= (TM_TCPF2_REINIT_TIMER |
                                               TM_TCPF2_REXMIT_TEMP);
                }
            }
/*
 * Some data has been acknowledged, but more data need to be acknowledged.
 * Re-initialize the retransmit timer timeout value for next segment.
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
    }
/*
 * Valid ACK. Check if we need to update the send window variables:
 */
    tfTcpUpdateSendWindow(tcpVectPtr, tcphPtr->tcpSeqNo, segAck,
                          packetPtr->pktTcpUnscaledWnd);
/*
 * Processing for Established, FIN_WAIT_1 and FIN_WAIT_2 only.
 * Process urgent data, and segment text (sixth and seventh steps).
 */
    curState = tcpVectPtr->tcpsState;
/*
 * NOTE: Check on state, rather than having an extra function
 * called from the state machine for each of theses states? Check on
 * state is faster but a little bit bigger in code space here. Calls from
 * the state machine makes the table bigger.
 */
    if (    (curState == TM_TCPS_ESTABLISHED)
         || (curState == TM_TCPS_FIN_WAIT_1)
         || (curState == TM_TCPS_FIN_WAIT_2) )
    {
/* RFC 793 sixth step: check urgent data */
        if (tm_8bit_one_bit_set(tcpCtlBits, TM_TCP_URG))
        {
            urgentData = tfTcpRcvUrgentData(tcpVectPtr, packetPtr);
        }
        else
        {
            urgentData = TM_8BIT_ZERO;
        }
        if (urgentData == TM_8BIT_ZERO)
        {
/* no urgent data */
            if (tm_sqn_lt(tcpVectPtr->tcpsRcvUp, tcpVectPtr->tcpsRcvNxt-1))
            {
/* and if not in urgent mode: update Rcv.UP to catch up to Rcv.Nxt-1*/
                tcpVectPtr->tcpsRcvUp = tcpVectPtr->tcpsRcvNxt-1;
            }
        }
/*
 * RFC 793 seventh step: process segment text:
 * Copy data from packet and reassemble queue to receive queue, update rcvNxt
 */
        if (    (dataLength != TM_UL(0))
             || (tm_8bit_one_bit_set(tcpCtlBits, TM_TCP_FIN)) )
        {
            if ( tm_16bit_one_bit_set(socketPtr->socFlags,
                                      TM_SOCF_NO_MORE_RECV_DATA) )
            {
/*
 * Incoming data/FIN for TCP socket when socket is shutdown for read.
 */
                if (    (dataLength != TM_UL(0))
/*
 * [RFC2525.2.16].1
 * TCP half-duplex Close:
 * RFC 1122 says that if new data is received after CLOSE is called,
 * TCP should send a RESET, if the user could no longer read the data.
 * RFC 2525 problem 2.16: failure to send a RST after Half Duplex Close.
 * ANVL-CORE-11.21: Test is done with shutdown, instead of close.
 * So we need to test for both close, and shutdown, making sure that the
 * send queue is empty before we send a RESET, so that we do not loose
 * the data we want to send.
 */
                     && (   (tm_16bit_bits_not_set( socketPtr->socFlags,
                                                    TM_SOCF_OPEN) )
                         || (    tm_16bit_one_bit_set (socketPtr->socFlags,
                                   TM_SOCF_CLOSING | TM_SOCF_NO_MORE_SEND_DATA)
                              && (socketPtr->socSendQueueBytes == TM_UL(0))) ) )
                {
                    errorCode = TM_ECONNABORTED;
                    needFreePacket = TM_8BIT_ZERO;
                    (void)tfTcpStateMachine( tcpVectPtr, packetPtr,
                                             TM_TCPE_ABORT);
                    goto rcvAckExit;
                }
/*
 * Otherwise:
 *    USER READ shutdown, and receiving a FIN with no data
 * or USER READ shutdown, and data but no USER CLOSE or WRITE shutdown
 * or USER READ shudown, and data, USER CLOSE or WRITE shutdown, but send
 *    queue not empty yet:
 *      ACK the data if next in sequence (tcpsRcvNxt update + ACK logic below),
 *      but do not queue it, i.e. drop the data (needFreePacket == TM_8BIT_YES
 *      by default).
 */
                if (tcphPtr->tcpSeqNo != tcpVectPtr->tcpsRcvNxt)
                {
/* Not in sequence, do not ack new data, reset the FIN bit if any */
                    dataLength = TM_UL(0); /* Do not ACK new data */
                    tcpCtlBits = 0; /* Do not process the FIN */
/* duplicate ACK */
                    tcpVectPtr->tcpsFlags2 |= TM_TCPF2_ACK;
                }
            }
            else
/*
 * No USER READ SHUTDOWN
 * Queue the data if next in sequence.
 */
            {
                if (    (tcphPtr->tcpSeqNo == tcpVectPtr->tcpsRcvNxt)
                     && (    tcpVectPtr->tcpsReassemblePtr
                          == TM_PACKET_NULL_PTR) )
                {
#ifdef TM_ERROR_CHECKING
                    if (tcpVectPtr->tcpsRcvWnd < dataLength)
                    {
                        tfKernelError("tfTcpRcvAck",
                         "Trimmed incoming data bigger than receive queue!!");
                    }
#endif /* TM_ERROR_CHECKING */
/*
 * If urgent data is in this packet, and urgent data is not to be kept
 * in line
 */
                    if (    tm_16bit_one_bit_set(
                                      packetPtr->pktSharedDataPtr->dataFlags,
                                      TM_BUF_TCP_OOB_BYTE )
                         && (tm_16bit_bits_not_set(
                                                socketPtr->socOptions,
                                                SO_OOBINLINE)) )
                    {
/* Extract out of band byte, and queue rest of the data to the socket */
                        errorCode = tfSocketOobIncomingPacket(tcpVectPtr,
                                                              packetPtr);
                    }
                    else
                    {
                        errorCode = tfSocketIncomingPacket( socketPtr,
                                                            packetPtr );
                    }
/* Note: errorCode always TM_ENOERROR for TCP */
                    if (errorCode == TM_ENOERROR)
                    {
                        tcpVectPtr->tcpsSelectFlags |= TM_SELECT_READ;
                        tcpVectPtr->tcpsSocketCBFlags |= TM_CB_RECV;
                    }
#ifdef TM_USE_SSL
                    else
                    {
/* SSL processing: no data for the user yet. */
                        if (errorCode == TM_EWOULDBLOCK)
                        {
                            errorCode = TM_ENOERROR;
                        }
                    }
#endif /* TM_USE_SSL */
                }
                else
                {
/*
 * In reassemble queue (including possibly FIN).
 * (Duplicate ACK set in tfTcpReassemblePacket())
 */
                    tcpCtlBits = tfTcpReassemblePacket(tcpVectPtr, packetPtr);
                    dataLength = TM_UL(0); /* Do not ACK new data */
                }
                needFreePacket = TM_8BIT_ZERO;
            }
/* If any data accepted/queued */
            if (dataLength != TM_UL(0))
            {
/* Update RcvNxt */
                tcpVectPtr->tcpsRcvNxt += dataLength;
/* Ack the data */
                if (    ( tcpVectPtr->tcpsDelayAckTime != TM_32BIT_ZERO )
                     && ( tcpVectPtr->tcpsDelayAckTmPtr != TM_TMR_NULL_PTR )
#ifdef TM_TCP_ACK_PUSH
/* OL 3/29/01 BUG ID 11-631: workaround an Internet Explorer bug */
                     && ( tm_8bit_bits_not_set( tcpCtlBits,
                                                TM_TCP_PSH ) )
#endif /* TM_TCP_ACK_PUSH */
                   )
                {
                    tm_timer_new_time( tcpVectPtr->tcpsDelayAckTmPtr,
                                       tcpVectPtr->tcpsDelayAckTime );
                    tcpVectPtr->tcpsFlags2 |= TM_TCPF2_DELAY_ACK;
                }
                else
                {
/* User does not want delay ACK, or delay ACK timer has been deleted */
                    tcpVectPtr->tcpsFlags2 |= TM_TCPF2_ACK;
                }
            }
            if (tcpCtlBits & TM_TCP_FIN)
/* Done with the ACK. if FIN is next byte to process */
            {
/* tell tfTcpIncomingPacket to process the SEG_FIN event */
                tcpVectPtr->tcpsFlags2 |= TM_TCPF2_RCV_FIN;
            }
        }
    }
rcvAckExit:
    if (needFreePacket != TM_8BIT_ZERO)
    {
        tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
    }
    return errorCode;
#undef lSendSegSize
#undef lCwnd
}

/*
 * tfTcpRcvAckComplt() function description
 * Called from the state machine in FIN_WAIT2 when the ACK of our FIN is
 * received.
 * If retransmission queue is empty:
 *  . complete the user close call, if user has shudown for read.
 *  . Free TCP timers except for idle count timer, and delay ACK, and
 *    Add FIN wait 2 timer if not already done so.
 * Parameters
 * tcpVectPtr   pointer to targeted TCP state vector (including socket entry).
 * packetPtr    Invalid (packetPtr has been either queued or freed up).
 * newState     Ignored in this routine (transition if any already occurred).
 * Return value:
 * TM_ENOERROR
 */
static int tm_state_function (tfTcpRcvAckComplt, tcpVectPtr, packetPtr,
                              newState)
{
    ttSocketEntryPtr        socketPtr;
    ttGenericUnion          timerParm1;

    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(newState);
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
/* If retransmission queue is empty */
    if (socketPtr->socSendQueueNextPtr == TM_PACKET_NULL_PTR)
    {
/*
 * Mark the socket as disconnected, and complete a user close call if user
 * had closed the socket.
 */
        (void)tfTcpCompleteClose(tcpVectPtr);
/* Free some TCP timers and add FIN wait 2 timer (if not already done so) */
        if (tcpVectPtr->tcpsFinWt2TmPtr == TM_TMR_NULL_PTR)
        {
/*
 * Free all the TCP state vector timers except for the idle count timer,
 * and delay ACK.
 */
            tfTcpFreeTimers(tcpVectPtr, TM_TCPTM_CONN, TM_TCPTM_REXMIT);
            timerParm1.genVoidParmPtr = (ttVoidPtr)tcpVectPtr;
            tm_socket_checkout(socketPtr);
            tcpVectPtr->tcpsFinWt2TmPtr =
                     tfTimerAddExt( &tcpVectPtr->tcpsFinWt2Tm,
                                    tfTcpTmFinWt2Out,
                                    tfTcpTmrCleanup,
                                    timerParm1,
                                    timerParm1, /* unused */
                                    ((tt32Bit)tcpVectPtr->tcpsFinWt2Time) *
                                        TM_UL(1000),
                                    TM_TIM_AUTO);
        }
    }
    return TM_ENOERROR;
}


/*
 * tfTcpCheckAckFin() function description
 * Called from the state machine in FIN_WAIT1, LAST_ACK, or CLOSING when
 * an ACK is received. We check whether the ACK is the ACK or our FIN.
 * If so we transition to the new state.
 * Parameters
 * tcpVectPtr   pointer to targeted TCP state vector (including socket entry).
 * packetPtr    Invalid (packetPtr has been either queued or freed up).
 * newState     New state to transition to if ACK is ACK of our FIN.
 * Return value:
 * TM_ENOERROR  if ACK is ACK of our FIN
 * TM_EINVAL    otherwise
 */
static int tm_state_function (tfTcpCheckAckFin, tcpVectPtr, packetPtr,
                              newState)
{
    int errorCode;

    TM_UNREF_IN_ARG(packetPtr);
/*
 * At this point the packet has either been freed or queued to the user.
 * If our FIN is acked then transition to new state otherwise return error
 */
    if ( tm_16bit_bits_not_set( tcpVectPtr->tcpsFlags2,
                                TM_TCPF2_SND_FIN | TM_TCPF2_FIN_SENT ) )
    {
        tcpVectPtr->tcpsState = newState;
        errorCode = TM_ENOERROR;
    }
    else
    {
/* Not ACK of FIN. No further processing in the state machine */
        errorCode = TM_EINVAL;
    }
    return errorCode;
}

/*
 * tfTcpRcvFin function description:
 * Called from the TCP state machine, when processing an incoming FIN
 * segment and when the TCP state is either SYN_RECEIVED, ESTABLISHED,
 * FIN_WAIT1, or FIN_WAIT2.
 * ANVL-CORE 10.24
 * Also called from the TCP state machine, when processing extra FINS,
 * i.e. FIN with a sqn == sqn+1 of previous FIN, in CLOSING, LAST-ACK
 * and TIME-WAIT state.
 * Process incoming FIN. (We had already determined that the segment
 * was valid, and the FIN withing the window.)
 * . Mark EOF in incoming stream
 * . transition to new state.
 * . Update Rcv.Nxt
 * . We will ack the FIN later (at the end of tfTcpIncomingPacket).
 * . Let the user know about the remote close.
 * Parameters
 * tcpVectPtr   pointer to targeted TCP state vector (including socket entry).
 * packetPtr    Invalid (packetPtr has been either queued or freed up).
 * newState     New Tcp State to transition to.
 * Return value:
 * TM_ENOERROR
 */
static int tm_state_function (tfTcpRcvFin, tcpVectPtr, packetPtr,
                              newState)
{
    ttSocketEntryPtr        socketPtr;

    TM_UNREF_IN_ARG(packetPtr); /* Invalid, i.e. either queued up, or freed */
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    socketPtr->socFlags |= TM_SOCF_REOF; /* received EOF mark */
    tcpVectPtr->tcpsRcvNxt++;
    tcpVectPtr->tcpsFlags2 |= TM_TCPF2_ACK;
    tcpVectPtr->tcpsSelectFlags |= TM_SELECT_READ;
    tcpVectPtr->tcpsSocketCBFlags |= TM_CB_REMOTE_CLOSE;
/*
 * Because of ANVL-CORE 10.24 change:
 * Transition to the new state after checking to restart the timer, to make
 * sure that we do not restart a timer that has not already been started
 */
    tcpVectPtr->tcpsState = newState;

#ifdef TM_USE_SSL
/*
 * If FIN is received but we didn't get SSL Close_notify before, we MUST not resume
 * this session. If this session is already cached, we need to remove it later
 * ( Note check on null tcpsSslConnStatePtr not necessary (non null if
 *   TM_SSLF_RECV_ENABLED is set))
 */
    if (    (tcpVectPtr->tcpsSslFlags & TM_SSLF_RECV_ENABLED)
         && (tcpVectPtr->tcpsSslConnStatePtr != (ttVoidPtr)0) )
    {
        ((ttSslConnectStatePtr)tcpVectPtr->tcpsSslConnStatePtr)
                      ->scsRunFlags |= TM_SSL_RUNFLAG_IMCOMP_CLOSE;
    }
#endif /* TM_USE_SSL */
    return TM_ENOERROR;
}

/*
 * tfTcpStart2MslTmr() function description
 * Called from the state machine when we transition to the TIME_WAIT state.
 * . Allocate a new TIME WAIT vector (smaller than the TCP vector)
 * . Initialize it
 * . start time wait timer
 * . Insert the new TIME WAIT vector in the Time Wait hash table
 * . Send an ACK if set during TCP state machine
 * . Close the TCP vector (it will complete the user close)
 * Parameters
 * tcpVectPtr   pointer to targeted TCP state vector (including socket entry).
 * packetPtr    Invalid (packetPtr has been either queued or freed up).
 * newState     ignored (transition already occured)
 * Return value:
 * TM_ENOERROR
 */
static int tm_state_function (tfTcpStart2MslTmr, tcpVectPtr, packetPtr,
                              newState)
{
    ttGenericUnion      timerParm1;
#if (defined(TM_USE_STRONG_ESL) || defined(TM_SNMP_CACHE))
    ttSocketEntryPtr    socketPtr;
#endif /* TM_USE_STRONG_ESL || TM_SNMP_CACHE */
    ttTcpTmWtVectPtr    tcpTmWtVectPtr;
    ttListPtr           listPtr;
    tt32Bit             mslTime;
    tt32Bit             minMslTime;

    TM_UNREF_IN_ARG(packetPtr);
    TM_UNREF_IN_ARG(newState);
    if (tm_context(tvTcpTmWtTableMembers) < tm_context(tvMaxTcpTmWtAllocCount))
/* Time Wait Vector table not full. (If full, drop the time wait vector ) */
    {
/* Allocate a TIME WAIT TCP VECTOR */
        tcpTmWtVectPtr = (ttTcpTmWtVectPtr)(ttVoidPtr)
                                tm_get_raw_buffer(sizeof(ttTcpTmWtVect));
        if (tcpTmWtVectPtr != (ttTcpTmWtVectPtr)0)
        {
            tm_bzero(tcpTmWtVectPtr, sizeof(ttTcpTmWtVect));
/* Our max SND.NXT */
            tcpTmWtVectPtr->twsMaxSndNxt = tcpVectPtr->tcpsMaxSndNxt;
/* Next expected peer seq no */
            tcpTmWtVectPtr->twsRcvNxt = tcpVectPtr->tcpsRcvNxt;
#ifdef TM_USE_IPV6
/* Network layer */
            tcpTmWtVectPtr->twsNetworkLayer = tcpVectPtr->tcpsNetworkLayer;
#endif /* TM_USE_IPV6 */
/* Socket 4-tuple */
            tm_bcopy(&(tcpVectPtr->tcpsSocketEntry.socTuple),
                     &(tcpTmWtVectPtr->twsTupleDev.tudTuple),
                     sizeof(ttSockTuple));
#if (defined(TM_USE_STRONG_ESL) || defined(TM_SNMP_CACHE))
            socketPtr = &(tcpVectPtr->tcpsSocketEntry);
#endif /* TM_USE_STRONG_ESL || TM_SNMP_CACHE */
#ifdef TM_USE_STRONG_ESL
            if (tm_8bit_one_bit_set(socketPtr->socFlags2,
                                    TM_SOCF2_BINDTODEVICE))
/* Copy bound device from socket */
            {
                tcpTmWtVectPtr->twsTupleDev.tudDevPtr =
                                         socketPtr->socRteCacheStruct.rtcDevPtr;
            }
#endif /* TM_USE_STRONG_ESL */
            if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_TS))
/* Copy Time Stamp option variables if TS on */
            {
                tcpTmWtVectPtr->twsFlags |= TM_TCPTWF_TS;
                tcpTmWtVectPtr->twsTsRecent = tcpVectPtr->tcpsTsRecent;
                tcpTmWtVectPtr->twsTsRecentAge = tcpVectPtr->tcpsTsRecentAge;
            }
#ifdef TM_SNMP_CACHE
            tcpTmWtVectPtr->twsTupleDev.tudAddressFamily =
                                         socketPtr->socProtocolFamily;
            tfSnmpdCacheInsertTmWtVect(tcpTmWtVectPtr);
#endif /* TM_SNMP_CACHE */
/*
 * Compute 2MSL time (time to stay in the time wait state) in milliseconds,
 * given the current tcps2MSLTime.
 * It is given by the tcps2MSLTime field in seconds.
 * If user set it below the minimum trip time, set it to the minimum round
 * trip time.
 */
            mslTime = (tt32Bit)(tcpVectPtr->tcps2MSLTime) * TM_UL(1000);
/* Compute minimum of minimum RTO, and computed RTO */
            minMslTime = tcpVectPtr->tcpsRtoMin;
            if (minMslTime > tcpVectPtr->tcpsRto)
            {
                minMslTime = tcpVectPtr->tcpsRto;
            }
            if (mslTime <  minMslTime)
/* If 2MSL is below minimum round trip, default to minimum round trip */
            {
                mslTime = minMslTime;
            }
            timerParm1.genVoidParmPtr = (ttVoidPtr)tcpTmWtVectPtr;
            tm_call_lock_wait(&tm_context(tvTcpTmWtTableLock));
            tcpTmWtVectPtr->tws2MslTmPtr = tfTimerAddExt(
                                                     &(tcpTmWtVectPtr->tws2MslTm),
                                                      tfTcpTm2MslOut,
                                                      tfTcpTmWtTmrCleanup,
                                                      timerParm1,
                                                      timerParm1, /* unused */
                                                      mslTime,
                                                      TM_TIM_AUTO);
            listPtr = tfTcpTmWtGetListPtr(&(tcpTmWtVectPtr->twsTupleDev.tudTuple));
/* add it to the beginning of the list */
            tfListAddToHead(listPtr,
                            &(tcpTmWtVectPtr->twsNode));
/* Number of time wait vectors in the table */
            tm_context(tvTcpTmWtTableMembers)++;
            if (listPtr->listCount == 1)
            {
/* Remember the number of non empty hash buckets (for diagnosis) */
                tm_context(tvTcpTmWtTableHashBucketFilled)++;
            }
            if (listPtr->listCount > tm_context(tvTcpTmWtTableHighDepth))
            {
/* Remember the highest list depth for diagnosis */
                tm_context(tvTcpTmWtTableHighDepth) = listPtr->listCount;
            }
            tm_call_unlock(&tm_context(tvTcpTmWtTableLock));
        }
    }
#ifdef TM_ERROR_CHECKING
    else
    {
        tfKernelWarning("tfTcpStart2MslTmr",
                        "Time Wait Vector Table Full");
    }
#endif /* TM_ERROR_CHECKING */
    if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags2, TM_TCPF2_ACK))
    {
        (void)tfTcpCreateSendPacket(tcpVectPtr, TM_PACKET_NULL_PTR,
                                    tcpVectPtr->tcpsSndNxt,
                                    tcpVectPtr->tcpsRcvNxt,
                                    TM_TCP_ACK);
    }
/* Close the TCP vector, but keep the vector in the time wait state */
    tfTcpVectClose(tcpVectPtr, TM_PACKET_NULL_PTR, TM_TCPS_TIME_WAIT);
    return TM_ENOERROR;
}

/*
 * tfTcpCreateSendPacket function description:
 * This function handles sending SYN, SYN_ACK, RST, Time Wait ACKs, and
 * Keep alive probes
 * Send a packet with seqNo = seq, AckNo = ack, and ControlBits = ctlbits.
 * Generate options for SYN packets, RST, keep alive probes, none for RST.
 * If packetPtr is non null for a RESET, use the packetPtr, reversing
 * source and destination port, and source and destination addresses
 * (because our vector might be closed). If packetPtr is null, create a
 * new packet.
 * parameters   Description
 * tcpVectPtr   pointer to TCP state vector
 * packetPtr    pointer to incoming reset segment, or null pointer
 * seq          Sequence number for new TCP segment
 * ack          Ack number for new TCP segment
 * ctlBits      TCP control bits for new TCP segment
 */
#ifdef TM_LINT
LINT_UNACCESS_SYM_BEGIN(window16)
#endif /* TM_LINT */
static int tfTcpCreateSendPacket (ttTcpVectPtr tcpVectPtr,
                                  ttPacketPtr  packetPtr,
                                  ttSqn        seq,
                                  ttSqn        ack,
                                  tt8Bit       ctlBits)
{
    ttTcpHeaderPtr      tcphPtr;
    ttSharedDataPtr     pktShrDataPtr;
#ifdef TM_USE_IPV4
    ttPseudoHeaderPtr   pshPtr;
    ttIpHeaderPtr       iphPtr;
#endif /* TM_USE_IPV4 */
    ttSocketEntryPtr    socketPtr;
#ifdef TM_USE_IPV4
    tt16Bit             ipHdrLength;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    tt6IpHeaderPtr      prevIph6Ptr;
    tt6IpHeaderPtr      iph6Ptr;
    tt6PseudoHeaderPtr  psh6Ptr;
    tt8Bit              saved6IpHops;
#endif /* TM_USE_IPV6 */
    tt32Bit             ourRcvWindow32;
    tt32Bit             maxRcvWindow32;
    int                 errorCode;
    ttIpPort            srcPort;
    tt16Bit             tcpHdrSize;
    tt16Bit             window16;
    tt8Bit              replyPacket;
    tt8Bit              rcvWndScale;
#define lTempWindow32   maxRcvWindow32

    replyPacket = TM_8BIT_ZERO;
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    if (packetPtr == TM_PACKET_NULL_PTR)
    {
/* Allocate a new TCP header */
        packetPtr = tfTcpGetTcpHeader(tcpVectPtr);
    }
    else
    {
        if (tm_8bit_one_bit_set(ctlBits, TM_TCP_RST))
        {
            if (packetPtr->pktLinkNextPtr != (ttVoidPtr)0)
            {
                tfFreePacket( (ttPacketPtr)packetPtr->pktLinkNextPtr,
                              TM_SOCKET_LOCKED );
                packetPtr->pktLinkNextPtr = (ttVoidPtr)0;
                packetPtr->pktLinkExtraCount = 0;
            }
            replyPacket = TM_8BIT_YES;
            packetPtr->pktLinkDataPtr = (tt8BitPtr)tm_tcp_hdr_ptr(packetPtr);
        }
#ifdef TM_ERROR_CHECKING
        else
        {
            tfKernelError("tfTcpCreateSendPacket",
                          "non Reset non null packet");
        }
#endif /* TM_ERROR_CHECKING */
    }
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
        if (tm_8bit_one_bit_set(ctlBits, TM_TCP_SYN))
        {
/* BUG ID 1328: Allow piggybacking on next segments */
            tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2, TM_TCPF2_REXMIT_TEMP );

            tcpHdrSize = tfTcpGenSynOptions(tcpVectPtr, packetPtr);
            tm_kernel_set_critical;
#ifdef TM_TCP_RFC2581
/*
 * RFC 2581: Re-starting Idle connections. ([RFC2581]R4.1:1)
 * Keep track of last data sent transmission time
 */
            tcpVectPtr->tcpsLastXmitTime = tvTime;
#endif /* TM_TCP_RFC2581 */
            tm_kernel_release_critical;
            tcpVectPtr->tcpsSndNxt = tcpVectPtr->tcpsSndUna + 1;
            if (tm_sqn_gt(tcpVectPtr->tcpsSndNxt, tcpVectPtr->tcpsMaxSndNxt))
            {
                tcpVectPtr->tcpsMaxSndNxt = tcpVectPtr->tcpsSndNxt;
            }
        }
        else
        {
/* RESET, Time Wait ACKs, Keep Alive */
            if (    tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_TS)
                 && (tm_8bit_bits_not_set(ctlBits, TM_TCP_RST)) )
            {
/* Time Wait ACKs, or Keep alive and time stamp option on: Add TS */
                tfTcpGenTsOption(
                            tcpVectPtr,
                            packetPtr->pktLinkDataPtr + TM_PAK_TCP_HDR_LEN);
                tcpHdrSize = TM_BYTES_TCP_HDR + TM_TCPO_TS_NOOP_LENGTH;
/* End of adding TS */
            }
            else
            {
                tcpHdrSize = TM_BYTES_TCP_HDR;
            }
        }
/* SYN packets/Reset packet/Time Wait ACKs/keep alive probe have no data */
        packetPtr->pktChainDataLength = (ttPktLen)tcpHdrSize;
        packetPtr->pktLinkDataLength = (ttPktLen)tcpHdrSize;
        tcphPtr = (ttTcpHeaderPtr)packetPtr->pktLinkDataPtr;
        tcphPtr->tcpDataOffset = tm_tcp_set_data_offset(tcpHdrSize);
        tcphPtr->tcpControlBits = ctlBits;
        tm_htonl(seq, tcphPtr->tcpSeqNo);
        tm_htonl(ack, tcphPtr->tcpAckNo);
/* Resetting the connection, no more peer incoming data allowed */
        if (tm_8bit_one_bit_set(ctlBits, TM_TCP_RST))
        {
            tcphPtr->tcpWindowSize = 0;
            ourRcvWindow32 = TM_UL(0);
#ifdef TM_SNMP_MIB
            tm_context(tvTcpData).tcpOutRsts++;
#endif /* TM_SNMP_MIB */
        }
        else
        {
            ourRcvWindow32 = tm_tcp_soc_recv_left(socketPtr, tcpVectPtr);
/* Window scale option. Can only scale on non SYN packets */
            if (tm_8bit_bits_not_set(ctlBits, TM_TCP_SYN))
            {
                rcvWndScale = tcpVectPtr->tcpsRcvWndScale;
            }
            else
            {
                rcvWndScale = (tt8Bit)0;
            }
            maxRcvWindow32 = TM_TCP_MAX_SCALED_WND << rcvWndScale;
/* Cannot advertize more than maximum */
            if (ourRcvWindow32 > maxRcvWindow32)
            {
                ourRcvWindow32 = maxRcvWindow32;
            }
            tcpVectPtr->tcpsRcvAdv = tcpVectPtr->tcpsRcvNxt + ourRcvWindow32;
/*#define lTempWindow32 maxRcvWindow32*/
            lTempWindow32 = ourRcvWindow32 >> rcvWndScale;
            window16 = (tt16Bit)lTempWindow32;
            tm_htons(window16, tcphPtr->tcpWindowSize);
        }
        tcphPtr->tcpUrgentPtr = TM_16BIT_ZERO; /* no urgent data */
/* ACK is being piggybacked or does not need to be sent */
        tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2, TM_TCPF2_ACK );
        if (replyPacket != TM_8BIT_ZERO)
        {
            pktShrDataPtr = packetPtr->pktSharedDataPtr;
/* exchange source and destination ports */
            srcPort = tcphPtr->tcpSrcPort;
            tcphPtr->tcpSrcPort = tcphPtr->tcpDstPort;
            tcphPtr->tcpDstPort = srcPort;
/* Zero the tcp checksum for computation */
            tcphPtr->tcpChecksum =  TM_16BIT_ZERO;
#ifdef TM_USE_IPV6 /* dual mode */
            prevIph6Ptr = packetPtr->pkt6RxIphPtr;
#endif /* TM_USE_IPV6 */
#ifdef TM_DEV_RECV_OFFLOAD
/* If recv checksum was offloaded, reset offload pointer */
            packetPtr->pktDevOffloadPtr = (ttVoidPtr)0;
#endif /* TM_DEV_RECV_OFFLOAD */
#if TM_DEF_SEND_TRAILER_SIZE > 0
/*
 * Do not recycle the TCP header, if the user might corrupt the
 * TCP header by sliding the packet.
 */
            tm_16bit_clr_bit( packetPtr->pktSharedDataPtr->dataFlags,
                              TM_BUF_TCP_HDR_BLOCK );
#endif /* TM_DEV_SEND_TRAILER_SIZE > 0 */
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6 /* dual mode */
            if (prevIph6Ptr == (tt6IpHeaderPtr)0)
#endif /* dual */
            {
/* IPv4 packet */
/*
 * BUG ID 11-669. If remote uses IP header options, do not use incoming
 * IP header length. We never send IP header options.
 */
                ipHdrLength = TM_4PAK_IP_MIN_HDR_LEN;
                packetPtr->pktIpHdrLen = (tt8Bit)ipHdrLength;
/* Point to new IP header (without options) */
                iphPtr = (ttIpHeaderPtr)((tt8BitPtr)tcphPtr -
                                                      packetPtr->pktIpHdrLen);
/* Exchange source and destination Ip addresses. Find return route. */
#ifdef TM_USE_STRONG_ESL
                socketPtr->socRteCacheStruct.rtcDevPtr =
                                             packetPtr->pktDeviceEntryPtr;
#endif /* TM_USE_STRONG_ESL */
                errorCode = tfIpDestToPacket( packetPtr,
#ifdef TM_4_USE_SCOPE_ID
/* source */            &(pktShrDataPtr->dataSpecDestIpAddress),
/* destination */       &(pktShrDataPtr->dataSockTuplePtr->sotRemoteIpAddress),
#else /* ! TM_4_USE_SCOPE_ID */
/* source */            tm_4_ip_addr(pktShrDataPtr->dataSpecDestIpAddress),
/* destination */       pktShrDataPtr->dataSrcIpAddress,
#endif /* ! TM_4_USE_SCOPE_ID */
/* initialize iphTos */ socketPtr->socIpTos,
                        &socketPtr->socRteCacheStruct,
                        iphPtr);
                if (errorCode == TM_ENOERROR)
                {
/*
 * For Checksum computation.
 */
                    pshPtr = (ttPseudoHeaderPtr)(
                        ((tt8BitPtr)iphPtr) +
                        (unsigned)(TM_4PAK_IP_MIN_HDR_LEN
                                   - TM_4PAK_PSEUDO_HDR_LEN) );
/* Set Pseudo header zero field (overlaying TTL) to 0 */
                    pshPtr->pshZero = TM_8BIT_ZERO;
/*
 * Initialize pseudo header length (pshLayer4Len). Overlays IP header
 * checksum
 */
                    tm_htons(tcpHdrSize, pshPtr->pshLayer4Len);
/* ULP protocol (6 for TCP) */
                    pshPtr->pshProtocol = TM_IP_TCP;
/* The packet already points to the TCP header. */
/* Compute the checksum */
                    tcphPtr->tcpChecksum = tfPacketChecksum(
                                     packetPtr,
                                     (ttPktLen)tcpHdrSize,
                                     (tt16BitPtr)(ttVoidPtr)pshPtr,
                                     tm_byte_count(TM_4PAK_PSEUDO_HDR_LEN));
                    iphPtr->iphTtl = tm_context(tvIpDefTtl);
                    iphPtr->iphFlagsFragOff = TM_16BIT_ZERO;
/* Point to IP header */
                    packetPtr->pktLinkDataPtr = (tt8BitPtr)iphPtr;
                    ipHdrLength = tm_byte_count(packetPtr->pktIpHdrLen);
                    packetPtr->pktChainDataLength += ipHdrLength;
                    packetPtr->pktLinkDataLength += ipHdrLength;
#ifdef TM_SNMP_MIB
/*
 * Number of segments sent, including those on current connections
 * but excluding those containing only retransmitted octets.
 * Here we are sending a reset. So we increase tcpOutSegs.
 */
#ifdef TM_USE_NG_MIB2
                    tm_64Bit_incr(tm_context(tvTcpData).tcpHCOutSegs);
#else /* TM_USE_NG_MIB2 */
                    tm_context(tvTcpData).tcpOutSegs++;
#endif /* !TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
/* Send the Reset packet. Cache unlocked by tfIpSendPacket() */
                    errorCode = tfIpSendPacket(
                        packetPtr,
#ifdef TM_LOCK_NEEDED
                        &socketPtr->socLockEntry
#else /* !TM_LOCK_NEEDED */
                        (ttLockEntryPtr)0
#endif /* !TM_LOCK_NEEDED */
#ifdef TM_USE_IPSEC
                        , (ttPktLenPtr)0
#ifdef TM_USE_IPSEC_TASK
                        , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
#endif /* TM_USE_IPSEC */
                        );
                }
                else
                {
                    tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
                }
            }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4 /* dual mode */
            else
#endif /* dual */
            {
                iph6Ptr = (tt6IpHeaderPtr)(   (tt8BitPtr)tcphPtr
                                            - TM_6PAK_IP_MIN_HDR_LEN );

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
/* Note scope ID already embedded in tf6TcpIncomingPacket */
                tm_6_ip_copy_structs(
                               tm_6_ip_hdr_in6_addr(prevIph6Ptr->iph6SrcAddr),
                               packetPtr->pktSpecDestIpAddress);
                errorCode = tf6IpDestToPacket( packetPtr,
/* source */                &(tm_6_ip_hdr_in6_addr(prevIph6Ptr->iph6DestAddr)),
/* destination */           &packetPtr->pktSpecDestIpAddress,
                            &socketPtr->socRteCacheStruct,
                            iph6Ptr);

                if (errorCode == TM_ENOERROR)
                {
/*
 * Pseudo header pointer:
 */
/* The packet already points to the TCP header. */
                    psh6Ptr = (tt6PseudoHeaderPtr)((tt8BitPtr)iph6Ptr);
/* Save IP Hops limit as set by tf6IpDestToPacket */
                    saved6IpHops = iph6Ptr->iph6Hops;
/* Initialize pseudo header length. Overlays flow label */
/* Network order */
                    tm_htons(tcpHdrSize, psh6Ptr->psh616BitLayer4Len);
/* network order */
                    psh6Ptr->psh616Bit0Layer4Len = TM_16BIT_ZERO;
/* Initialize pseudo header zero field. Overlays Payload, hops */
                    tm_6_zero_arr(psh6Ptr->psh6ZeroArr);
/* ULP field */
                    psh6Ptr->psh6Protocol = IPPROTO_TCP;
/* Compute the checksum */
                    tcphPtr->tcpChecksum = tfPacketChecksum(
                                packetPtr,
                                (ttPktLen)tcpHdrSize,
                                (tt16BitPtr)(ttVoidPtr)psh6Ptr,
/* Pseudo header size */
                                (tt16Bit)TM_6_IP_MIN_HDR_LEN);
/* Initialize IPv6 hops from value saved earlier */
                    iph6Ptr->iph6Hops = saved6IpHops;
                    iph6Ptr->iph6Nxt = IPPROTO_TCP;
/* Initialize IP header traffic class && flow label. */
                    iph6Ptr->iph6Flow = socketPtr->soc6FlowInfo;

#ifdef TM_SNMP_MIB
/*
 * Number of segments sent, including those on current connections
 * but excluding those containing only retransmitted octets.
 * Here we are sending a reset. So we increase tcpOutSegs.
 */
#ifdef TM_USE_NG_MIB2
                    tm_64Bit_incr(tm_context(tvTcpData).tcpHCOutSegs);
#else /* TM_USE_NG_MIB2 */
                    tm_context(tvTcpData).tcpOutSegs++;
#endif /* !TM_USE_NG_MIB2 */
#endif /* !TM_SNMP_MIB */
/* Send the Reset packet. Cache unlocked by tf6IpSendPacket() */
                    errorCode = tf6IpSendPacket(
                            packetPtr,
                            &socketPtr->socLockEntry
#ifdef TM_USE_IPSEC_TASK
                            , (ttIpsecTaskListEntryPtr)0
#endif /* TM_USE_IPSEC_TASK */
                            );
                }
                else
                {
                    tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
                }
            }
#endif /* TM_USE_IPV6 */
        }
        else
        {
            tcpVectPtr->tcpsHeaderPacketPtr = packetPtr;
/* TcpSendPacket will add Src/Destination ports, length and checksum */
            errorCode = tfTcpSendPacket(tcpVectPtr);
        }

    }
    else
    {
        errorCode = TM_ENOBUFS;
    }
    return errorCode;
}
#ifdef TM_LINT
LINT_UNACCESS_SYM_END(window16)
#endif /* TM_LINT */


/*
 * tfTcpStateMachine() function description.
 *
 * State machine main function.
 * Given a causing event, and the TCP state vector current state,
 * get an action procedure index and a new TCP state from the local
 * variable array tlTcpTransitionMatrix[][]. Then call all state functions
 * corresponding to the action procedure index in the local variable
 * array tlTcpStateFunctTable[], passing to each state function the new
 * TCP state (for the TCP state transition) as a parameter. If any of the
 * state functions in the list returns an error, return immediately, and do
 * not call the remaining state functions in the action procedure index list.
 * TCP state transition (if any) occurs in one of the state functions in
 * the list (as indicated in the comments in the local array variable
 * tlTcpStateFunctTable[]) unless an error occurs before that particular
 * state function is being called.
 *
 * Special values for action procedure index:
 * TM_TCPM_LAST_INDEX (== TM_MATRIX_NOOP): no state function is called,
 *                                         just a TCP state transition
 * index > TM_TCPM_LAST_INDEX: no state function is called, no TCP state
 *                             transition either. Return socket
 *                             error corresponding to index value.
 *
 *
 * Parameters  description
 * tcpVectPtr  TCP state vector pointer to be passed to the
 *             TCP state function. Also used to extract current state which
 *             is used as first index in tlTcpTransitionMatrix[][].
 * packetPtr   TCP incoming packet pointer to be passed to the
 *             TCP state function.
 * event       Causing Event (to be used as second index in
 *             tlTcpTransitionMatrix[][])
 *
 * Return value
 * error
 */
int tfTcpStateMachine (ttTcpVectPtr   tcpVectPtr,
                       ttPacketPtr    packetPtr,
/* event (to be used as index in tlTcpTransitionMatrix[][] */
                       tt8Bit         event)
{
    ttTransitionMatrixEntryPtr matrixEntryPtr;
    ttTcpStateFunctEntryPtr    tcpStateFunctEntryPtr;
    int                        errCode;
    tt8Bit                     curState;
    tt8Bit                     newState;
    tt8Bit                     index;

/* Save current state */
    curState = tcpVectPtr->tcpsState;
/* point to transition matrix for the current state, event pair */
    matrixEntryPtr = (ttTransitionMatrixEntryPtr)
                                      &tlTcpTransitionMatrix[curState][event];
    newState = matrixEntryPtr->mtxTransition;
/* Index into state function table */
    index = matrixEntryPtr->mtxFunctionIndex;
    tm_debug_log4(
        "tfTcpStateMachine socket %d, socFlags 0x%x, tcpFlags 0x%x, "
        "tcpFlags2 0x%x",
        (int)tcpVectPtr->tcpsSocketEntry.socIndex,
        (int)tcpVectPtr->tcpsSocketEntry.socFlags,
        (int)tcpVectPtr->tcpsFlags, (int)tcpVectPtr->tcpsFlags2);
    tm_debug_log4(
        "----------------- index %d, curState %d, newState %d, event %d",
        (int)index, (int)curState, (int)newState, (int)event);
/* Check whether there is any function to call */
    if ((unsigned)index < TM_TCPM_LAST_INDEX)
    {
/* State function table entry pointer */
        tcpStateFunctEntryPtr =(ttTcpStateFunctEntryPtr)
                                                 &tlTcpStateFunctTable[index];
/* Call all functions corresponding to index */
        do
        {
/* Call state function */
            tm_debug_log1("----------------- calling func 0x%x",
                          tcpStateFunctEntryPtr);
            errCode = tm_call_state_function(tcpStateFunctEntryPtr,
                                             tcpVectPtr,
                                             packetPtr,
                                             newState);
/* If state function returned an error */
            if (errCode != 0)
            {
                break;
            }
/* Point to next entry in state function table */
            tcpStateFunctEntryPtr++;
        } while (tcpStateFunctEntryPtr->tstaIndex == index);
    }
    else
    {
/* No function to call */
        if ((unsigned)index == TM_MATRIX_NOOP)
        {
/* Just a state transition */
            tm_debug_log0("----------------- state transition only");
            tcpVectPtr->tcpsState = newState;
            errCode = TM_ENOERROR;
        }
        else
        {
/*
 * Index is coded socket error value. Retrieve socket error.
 * No state transition
 */
            errCode = tm_tcpm_sock_err(index);
            tm_debug_log1("----------------- socket error return only %d",
                          errCode);
        }
    }
    tm_debug_log1("tfTcpStateMachine returns %d", errCode);
    return errCode;
}


/*
 * Free TCP timers between minIndex, and maxIndex (not including maxIndex)
 */
static void tfTcpFreeTimers ( ttTcpVectPtr tcpVectPtr,
                              tt8Bit       minIndex,
                              tt8Bit       maxIndex )
{
    ttTimerPtr    tmPtr;
    int           index;

    for (index = minIndex; ((tt8Bit) index) < maxIndex; index++)
    {

        tmPtr = tcpVectPtr->tcpsTmPtr[index];
        if (tmPtr != TM_TMR_NULL_PTR)
        {
            if (index == TM_TCPTM_DELAYACK)
            {
                tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2,
                                  TM_TCPF2_DELAY_ACK );
                tcpVectPtr->tcpsDelayAckTime = TM_UL(0);
            }
            if (index == TM_TCPTM_REXMIT)
            {
                tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2,
                                  TM_TCPF2_REXMIT_MODE );
            }
            if (index == TM_TCPTM_PROBE)
            {
                tm_16bit_clr_bit( tcpVectPtr->tcpsFlags3,
                                  TM_TCPF3_PROBE_MODE);
            }
            tcpVectPtr->tcpsTmPtr[index] = TM_TMR_NULL_PTR;
            tm_timer_remove(tmPtr);
        }
    }
    return;
}

/*
 * tfTcpTmrCleanup()
 * Clean up function called by the timer execute before freeing a timer
 * or by tfTimerRemove().
 */
static void tfTcpTmrCleanup(ttVoidPtr       timerBlockPtr,
                            tt8Bit          flags)
{
    ttSocketEntryPtr socketPtr;

    socketPtr = (ttSocketEntryPtr)
            (tm_tmr_arg1((ttTimerPtr)timerBlockPtr).genVoidParmPtr);
    if (!(flags & TM_TIMER_CLEANUP_LOCKED))
    {
/* Lock the socket */
        tm_call_lock_wait(&socketPtr->socLockEntry);
/* Check it in. This will release the timer ownership of the socket. */
        tm_socket_checkin_call_unlock(socketPtr);
    }
    else
    {
        socketPtr->socOwnerCount--;
    }
    return;
}

/*
 * TCP keep Alive timer. Fired every tcpVectPtr->tcpsIdleIntvTime (75s).
 * 1. Update idle time.
 * 2. If user has not issued a close yet (state <= CLOSE WAIT), and if
 *    Keep alive option is on, if idle time has reached tcpVectPtr->
 *    tcpsKeepAliveTime (2 hours), and if tcpVectPtr->tcpsKeepAliveProbeCnt
 *    (8) probes have already been sent with no response, abort the
 *    connection, else send probe
 */
static void tfTcpTmKeepAlive (ttVoidPtr      timerBlockPtr,
                              ttGenericUnion userParm1,
                              ttGenericUnion userParm2)
{
    ttTcpVectPtr       tcpVectPtr;
    ttSocketEntryPtr   socketPtr;
    int                keepAliveTime;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);

    tcpVectPtr = (ttTcpVectPtr)userParm1.genVoidParmPtr;
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    tm_call_lock_wait(&socketPtr->socLockEntry);
/* Keep track of idle time on the connection */
    if (tcpVectPtr->tcpsKeepAliveTmPtr == (ttTimerPtr)timerBlockPtr)
    {
#ifdef TM_USE_TCP_REXMIT_CONTROL
        if (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags3,
                                  TM_TCPF3_REXMIT_PAUSE))
#endif /* TM_USE_TCP_REXMIT_CONTROL */
        {
            tcpVectPtr->tcpsIdleTime += tcpVectPtr->tcpsIdleIntvTime;
        }
        if (    (socketPtr->socOptions & SO_KEEPALIVE)
             && (tcpVectPtr->tcpsState <= TM_TCPS_CLOSE_WAIT) )
        {
            keepAliveTime = tcpVectPtr->tcpsKeepAliveTime;
            if ( (tcpVectPtr->tcpsIdleTime) >= keepAliveTime )
            {
                keepAliveTime += tcpVectPtr->tcpsIdleIntvTime *
                                 tcpVectPtr->tcpsKeepAliveProbeCnt;
                if ( tcpVectPtr->tcpsIdleTime >= keepAliveTime )
                {
                    (void)tfTcpAbort(tcpVectPtr, TM_ETIMEDOUT);
                }
                else
                {
/*
 * Keep alive probe.
 * Create and send a TCP message with zero length at sndUna-1.
 * NOTE: sndUna-1 instead of sndNxt-1 to make sure that we
 * are outside of the peer's window
 */
                    (void)tfTcpCreateSendPacket(tcpVectPtr,
                                                TM_PACKET_NULL_PTR,
                                                tcpVectPtr->tcpsSndUna-1,
                                                tcpVectPtr->tcpsRcvNxt,
                                                TM_TCP_ACK);
                }
            }
        }
    }
    tm_call_unlock(&socketPtr->socLockEntry);
    return;
}


/*
 * Called when connection request timed out. Abort the connection.
 * Also need to notify user
 */
static void tfTcpTmConnOut (ttVoidPtr      timerBlockPtr,
                            ttGenericUnion userParm1,
                            ttGenericUnion userParm2)
{
    ttTcpVectPtr        tcpVectPtr;
    ttSocketEntryPtr    socketPtr;
    tt32Bit             timeOutPeriod;
    tt32Bit             idleTime;
    tt8Bit              abort;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);

    tcpVectPtr = (ttTcpVectPtr)userParm1.genVoidParmPtr;
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    if (socketPtr != (ttSocketEntryPtr)0)
    {
        tm_call_lock_wait(&socketPtr->socLockEntry);
    }
    if (    (tcpVectPtr->tcpsConnTmPtr == (ttTimerPtr)timerBlockPtr)
         && (tm_timer_not_reinit((ttTimerPtr)timerBlockPtr))
#ifdef TM_USE_TCP_REXMIT_CONTROL
/* and if the retransmission timer is not paused */
         && tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags3,
                                  TM_TCPF3_REXMIT_PAUSE)
#endif /* TM_USE_TCP_REXMIT_CONTROL */
        )

    {
        abort = TM_8BIT_YES; /* assume connection timed out */
        timeOutPeriod = tcpVectPtr->tcpsMaxRt;
        if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags3, TM_TCPF3_PROBE_MODE))
/* We are probing a zero window */
        {
            idleTime = ((tt32Bit)(tcpVectPtr->tcpsIdleTime)*TM_UL(1000));
            if (idleTime < timeOutPeriod)
            {
/* Got a response from probes during connection time out period, do not ABORT */
                abort = TM_8BIT_ZERO;
/* New connection timer count down started at time of last response */
                timeOutPeriod = (tt32Bit)(timeOutPeriod - idleTime);
            }
        }
        if (abort == TM_8BIT_YES)
        {
            tm_timer_remove(tcpVectPtr->tcpsConnTmPtr);
            tcpVectPtr->tcpsConnTmPtr = TM_TMR_NULL_PTR;
            (void)tfTcpAbort(tcpVectPtr, TM_ETIMEDOUT);
        }
        else
        {
            tm_timer_new_time(tcpVectPtr->tcpsConnTmPtr, timeOutPeriod);
        }
    }
    if (socketPtr != (ttSocketEntryPtr)0)
    {
        tm_call_unlock(&socketPtr->socLockEntry);
    }
    return;
}

/*
 * Add probe window timer
 * NOTE: Called from tfTcpSendPacket() only
 */
void tfTcpAddProbeTimer (ttTcpVectPtr tcpVectPtr)
{
    ttGenericUnion      timerParm1;
    ttSocketEntryPtr    socketPtr;
#ifdef TM_USE_TCP_REXMIT_CONTROL
    tt8Bit              timerMode;
#endif /* TM_USE_TCP_REXMIT_CONTROL */

#ifdef TM_USE_TCP_REXMIT_CONTROL
    if (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags3,
                              TM_TCPF3_REXMIT_PAUSE))
    {
        timerMode = TM_TIM_AUTO;
    }
    else
    {
        timerMode = TM_TIM_SUSPENDED | TM_TIM_AUTO;
    }
#endif /* TM_USE_TCP_REXMIT_CONTROL */

    if (tcpVectPtr->tcpsWndProbeTmPtr != (ttTimerPtr)0)
    {
#ifdef TM_USE_TCP_REXMIT_CONTROL
        if (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags3,
                                  TM_TCPF3_REXMIT_PAUSE))
#endif /* TM_USE_TCP_REXMIT_CONTROL */
        {
            tm_timer_new_time(tcpVectPtr->tcpsWndProbeTmPtr,
                              tcpVectPtr->tcpsProbeMin);
        }

    }
    else
    {
        timerParm1.genVoidParmPtr = (ttVoidPtr) tcpVectPtr;
        socketPtr = &(tcpVectPtr->tcpsSocketEntry);
        tm_socket_checkout(socketPtr);
        tcpVectPtr->tcpsWndProbeTmPtr =
                                  tfTimerAddExt( &tcpVectPtr->tcpsWndProbeTm,
                                                 tfTcpTmProbeWnd,
                                                 tfTcpTmrCleanup,
                                                 timerParm1,
                                                 timerParm1, /* unused */
                                                 tcpVectPtr->tcpsProbeMin,
#ifdef TM_USE_TCP_REXMIT_CONTROL
                                                 timerMode
#else /* !TM_USE_TCP_REXMIT_CONTROL */
                                                 TM_TIM_AUTO
#endif /* TM_USE_TCP_REXMIT_CONTROL */
                                                 );
    }
    tcpVectPtr->tcpsFlags3 |= TM_TCPF3_PROBE_MODE;
    tcpVectPtr->tcpsReXmitCnt = 0;
    (void)tfTcpProbeWndInterval(tcpVectPtr);
    return;
}

/*
 * Add connection time out timer if required by the user (with TCP_MAXRT
 * socket option), and we just starting probing.
 * Compute probe window interval.
 */
static tt8Bit tfTcpProbeWndInterval (ttTcpVectPtr tcpVectPtr)
{
    tt32Bit probeWndInterval;
    int     retryFactor;
    int     reXmitCnt;
    tt8Bit  abort;

    probeWndInterval = tm_rto(tcpVectPtr->tcpsScSmRtt,
                              tcpVectPtr->tcpsScRttVar);
/*
 * Add a connection time out timer if user wants one and we just started
 * probing
 */
    tfTcpAddMaxRtTmr(tcpVectPtr);
    reXmitCnt = tcpVectPtr->tcpsReXmitCnt++;
    retryFactor = reXmitCnt;
/* exponential backoff time interval for next probe */
    tm_expn_backoff(retryFactor);
    probeWndInterval = probeWndInterval * (tt32Bit)retryFactor;
    tm_bound(probeWndInterval,
             tcpVectPtr->tcpsProbeMin,
             tcpVectPtr->tcpsProbeMax);
    abort = TM_8BIT_ZERO;
    if ( reXmitCnt >= tcpVectPtr->tcpsMaxReXmitCnt )
    {
        if (    (((tt32Bit)(tcpVectPtr->tcpsIdleTime)*TM_UL(1000)) >=
                                                         probeWndInterval)
             && (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags,
                                       TM_TCPF_RT_FOREVER)) )
        {
            abort = TM_8BIT_YES;
        }
        else
        {
/* Avoid drifting of reXmitCnt */
            tcpVectPtr->tcpsReXmitCnt = tcpVectPtr->tcpsMaxReXmitCnt;
        }
    }
#ifdef TM_USE_TCP_REXMIT_CONTROL
    if (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags3,
                              TM_TCPF3_REXMIT_PAUSE))
#endif /* TM_USE_TCP_REXMIT_CONTROL */
    {
        tm_tcp_timer_new_time(tcpVectPtr->tcpsWndProbeTmPtr, probeWndInterval);
    }

    return abort;
}

/*
 * Probe window timeout timer
 */
static void tfTcpTmProbeWnd (ttVoidPtr      timerBlockPtr,
                             ttGenericUnion userParm1,
                             ttGenericUnion userParm2)
{
    ttTcpVectPtr        tcpVectPtr;
    ttSocketEntryPtr    socketPtr;
    tt8Bit              abort;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);

    tcpVectPtr = (ttTcpVectPtr)userParm1.genVoidParmPtr;
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    if (socketPtr != (ttSocketEntryPtr)0)
    {
        tm_call_lock_wait(&socketPtr->socLockEntry);
    }
    if (    (tcpVectPtr->tcpsWndProbeTmPtr == (ttTimerPtr)timerBlockPtr)
         && (tm_timer_not_reinit((ttTimerPtr)timerBlockPtr))
         && (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags3,
                                  TM_TCPF3_PROBE_MODE))
#ifdef TM_USE_TCP_REXMIT_CONTROL
/* and if the retransmission timer is not paused */
         && (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags3,
                                   TM_TCPF3_REXMIT_PAUSE))
#endif /* TM_USE_TCP_REXMIT_CONTROL */
                                   )
    {
#ifdef TM_ERROR_CHECKING
        if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags2,
                                 TM_TCPF2_REXMIT_MODE))
        {
            tfKernelError("tfTcpTmProbeWnd",
                          "Probing window, and rexmit timer on!");

        }
#endif /* TM_ERROR_CHECKING */
/* Recompute probe window time interval, and set timer interval with it */
        abort = tfTcpProbeWndInterval(tcpVectPtr);
        if (abort == TM_8BIT_ZERO)
        {
/* End of probe window time interval computation and setting */
            tcpVectPtr->tcpsFlags2 |= (TM_TCPF2_PROBE_OOB);
/* tfTcpSendPacket will send a 1 byte packet */
            (void)tfTcpStateMachine( tcpVectPtr,
                                     TM_PACKET_NULL_PTR,
                                     TM_TCPE_REXMIT);
        }
        else
        {
/* tfTcpAbort will abort the connetion */
            (void)tfTcpAbort(tcpVectPtr, TM_ETIMEDOUT);
        }
    }
    if (socketPtr != (ttSocketEntryPtr)0)
    {
        tm_call_unlock(&socketPtr->socLockEntry);
    }
    return;
}

/*
 * Delay Ack timer.
 * Called to transmit a delay ACK. Piggy back data.
 */
static void tfTcpTmDelayAck (ttVoidPtr      timerBlockPtr,
                             ttGenericUnion userParm1,
                             ttGenericUnion userParm2)
{
    ttTcpVectPtr        tcpVectPtr;
    ttSocketEntryPtr    socketPtr;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);

    tcpVectPtr = (ttTcpVectPtr)userParm1.genVoidParmPtr;
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    if (socketPtr != (ttSocketEntryPtr)0)
    {
        tm_call_lock_wait(&socketPtr->socLockEntry);
    }
/*
 * Check whether the timer has been re-initialized/removed/suspended
 * between the call back function and obtaining the TCP vector lock.
 * If timer has not been reinitialized:
 */
    if (    (tcpVectPtr->tcpsDelayAckTmPtr == (ttTimerPtr)timerBlockPtr)
         && (tm_timer_not_reinit((ttTimerPtr)timerBlockPtr))
         && (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags2,
                                  TM_TCPF2_DELAY_ACK)) )
    {
/* force ACK */
        tcpVectPtr->tcpsFlags2 |= (TM_TCPF2_ACK);
        (void)tfTcpSendPacket(tcpVectPtr);
    }
    if (socketPtr != (ttSocketEntryPtr)0)
    {
        tm_call_unlock(&socketPtr->socLockEntry);
    }
    return;
}

/*
 * Fin wait 2 timeout timer
 */
static void tfTcpTmFinWt2Out (ttVoidPtr      timerBlockPtr,
                              ttGenericUnion userParm1,
                              ttGenericUnion userParm2)
{
    ttTcpVectPtr        tcpVectPtr;
    ttSocketEntryPtr    socketPtr;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);

    tcpVectPtr = (ttTcpVectPtr)userParm1.genVoidParmPtr;
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    if (socketPtr != (ttSocketEntryPtr)0)
    {
        tm_call_lock_wait(&socketPtr->socLockEntry);
    }
    if (tcpVectPtr->tcpsFinWt2TmPtr == (ttTimerPtr)timerBlockPtr)
    {
        if ( (tcpVectPtr->tcpsIdleTime) >= tcpVectPtr->tcpsFinWt2Time -
                                           tcpVectPtr->tcpsIdleIntvTime)
        {
/*
 * Abort the connection if idle for tvTcpFinWt2Time - Idle interval
 * (used to increment idleTime) which is 600s-75s.
 * tfTcpAbort will remove timer.
 */
            (void)tfTcpAbort(tcpVectPtr, TM_ETIMEDOUT);
        }
        else
        {
/* Otherwise check again in tvTcpIdleIntvTime (75 s) */
            tm_tcp_timer_new_time(
                         tcpVectPtr->tcpsFinWt2TmPtr,
                         ((tt32Bit)tcpVectPtr->tcpsIdleIntvTime)*TM_UL(1000));
        }
    }
    if (socketPtr != (ttSocketEntryPtr)0)
    {
        tm_call_unlock(&socketPtr->socLockEntry);
    }
    return;
}

/*
 * Retransmission timeout timer
 */
static void tfTcpTmReXmit (ttVoidPtr      timerBlockPtr,
                           ttGenericUnion userParm1,
                           ttGenericUnion userParm2)
{
    ttTcpVectPtr        tcpVectPtr;
    ttSocketEntryPtr    socketPtr;
    tt32Bit             reXmitTime;
    int                 retryFactor;
    int                 reXmitCnt;
    int                 maxReXmitCnt;
    tt16Bit             flags;
#define lWindowSize     reXmitTime

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);

    tcpVectPtr = (ttTcpVectPtr) userParm1.genVoidParmPtr;
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    tm_call_lock_wait(&socketPtr->socLockEntry);
    flags = tcpVectPtr->tcpsFlags;
/*
 * Check whether the timer has been re-initialized
 * between the call back function and obtaining the TCP vector lock.
 * If timer has not been reinitialized:
 */
    if (    (tcpVectPtr->tcpsReXmitTmPtr == (ttTimerPtr)timerBlockPtr)
         && (tm_timer_not_reinit((ttTimerPtr)timerBlockPtr)) )
    {
/* If timer is enabled, and not in the process of being re-initialized */
        if (    tm_16bit_one_bit_set( tcpVectPtr->tcpsFlags2,
                                      TM_TCPF2_REXMIT_MODE)
             && tm_16bit_bits_not_set( tcpVectPtr->tcpsFlags2,
                                       TM_TCPF2_REINIT_TIMER)
#ifdef TM_USE_TCP_REXMIT_CONTROL
/* and if the retransmission timer is not paused */
             && tm_16bit_bits_not_set( tcpVectPtr->tcpsFlags3,
                                       TM_TCPF3_REXMIT_PAUSE)
#endif /* TM_USE_TCP_REXMIT_CONTROL */
                                       )
        {
/* increase retransmit count */
            tcpVectPtr->tcpsReXmitCnt++;
            reXmitCnt = tcpVectPtr->tcpsReXmitCnt;
            maxReXmitCnt = tcpVectPtr->tcpsMaxReXmitCnt;
/*
 * Add a connection time out timer if user wants one and we just started
 * retransmitting
 */
            tfTcpAddMaxRtTmr(tcpVectPtr);
            if (    (reXmitCnt <= maxReXmitCnt)
                 || (tm_16bit_one_bit_set(flags, TM_TCPF_RT_FOREVER)) )
            {
                if (reXmitCnt > maxReXmitCnt)
                {
/* If we wait forever, do not allow the retransmission count to drift up */
                    tcpVectPtr->tcpsReXmitCnt = tcpVectPtr->tcpsMaxReXmitCnt;
                }
                reXmitTime = tm_rto(tcpVectPtr->tcpsScSmRtt,
                                    tcpVectPtr->tcpsScRttVar);
                if ( reXmitCnt > (int)((unsigned int)maxReXmitCnt >> 2) )
                {
                    if (tm_16bit_one_bit_set(flags, TM_TCPF_INDIRECT))
                    {
/*
 * NOTE: We also reset when we add a new route entry.
 */
                        tfRtCacheReset(&socketPtr->socRteCacheStruct,
                                       TM_RTE_INDIRECT);
                        tfTcpPurgeTcpHeaders(tcpVectPtr);
                    }
/*
 * Save rto in rttvar before resetting scaled smooth round trip time, to
 * keep last estimate around for the next computation of retransmit time,
 * since we are zeroing the scaled smoothed round trip time.
 */
                    tcpVectPtr->tcpsScRttVar = (ttS32Bit)reXmitTime;
/*
 * reset scaled smoothed round trip time, so that we can get a fresh
 * new estimate
 */
                    tcpVectPtr->tcpsScSmRtt = TM_UL(0);
                }
                retryFactor = tcpVectPtr->tcpsReXmitCnt;
/* exponential backoff time interval for next retransmit */
                tm_expn_backoff(retryFactor);
                reXmitTime = reXmitTime * (tt32Bit)retryFactor;
                tm_bound(reXmitTime,
                         tcpVectPtr->tcpsRtoMin,
                         tcpVectPtr->tcpsRtoMax);
                tcpVectPtr->tcpsRto = reXmitTime;
                if ( tm_16bit_one_bit_set( tcpVectPtr->tcpsFlags,
                                           TM_TCPF_SLOW_START ) )
/* If user did not turn off slow start */
                {
/*
 * RFC 2001:
 *   3.  When congestion occurs (indicated by a timeout),
 *      one-half of the current window size (the
 *      minimum of cwnd and the receiver's advertised window), but at
 *      least two segments is saved in ssthresh.  Additionally, (since the
 *      congestion is indicated by a timeout), cwnd is set to one segment
 *      (i.e., slow start).
 */
                    lWindowSize = tcpVectPtr->tcpsSndWnd;
                    if (lWindowSize > tcpVectPtr->tcpsCwnd)
                    {
                        lWindowSize = tcpVectPtr->tcpsCwnd;
                    }
                    lWindowSize = lWindowSize / 2;
                    tcpVectPtr->tcpsSsthresh =
                                            2 * tcpVectPtr->tcpsEffSndMss;
                    if (lWindowSize > tcpVectPtr->tcpsSsthresh)
                    {
                        tcpVectPtr->tcpsSsthresh = lWindowSize;
                    }
/* send unacknowledged data */
                    tcpVectPtr->tcpsSndNxt = tcpVectPtr->tcpsSndUna;
/* Slow start. Use Loss Window */
                    tcpVectPtr->tcpsCwnd = tm_tcp_lw(tcpVectPtr);
                    tm_kernel_set_critical;
                    tcpVectPtr->tcpsCwndAge = tvTime;
                    tm_kernel_release_critical;
/*
 * ANVL-CORE 17.18, 17.19:
 * Set the TM_TCPF2_REXMIT_TEMP flag (below) to go through the
 * tfTcpSendPacket() new check for not transmitting new data with
 * retransmitted packets unless we can send a full segment.
 */
                }
/*
 * Force re-initialization of retransmit timer interval (in tfTcpSendPacket)
 * Just retransmit one segment (loss window).
 */
                tcpVectPtr->tcpsFlags2 |= (tt16Bit)
                                                 (   TM_TCPF2_REINIT_TIMER
                                                   | TM_TCPF2_REXMIT_TEMP );
/* If congested earlier, reset flag */
                tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2,
                                  TM_TCPF2_CONGESTED );
/* Reset number of consecutive duplicate acks */
                tcpVectPtr->tcpsDupAcks = 0;
#ifdef TM_TCP_SACK
/*
 * RFC 2018 states:
 * After a retransmit timeout the data sender SHOULD turn off all of the
 * SACKed bits, since the timeout might indicate that the data receiver
 * has reneged.
 */
                tfTcpSackFreeQueue(tcpVectPtr, TM_TCP_SACK_RECV_INDEX);
#endif /* TM_TCP_SACK */
                (void)tfTcpStateMachine( tcpVectPtr,
                                         TM_PACKET_NULL_PTR,
                                         TM_TCPE_REXMIT);
            }
            else
            {
/* tfTcpAbort will remove timer */
                (void)tfTcpAbort(tcpVectPtr, TM_ETIMEDOUT);
            }
        }
    }
    tm_call_unlock(&socketPtr->socLockEntry);
#undef lWindowSize
    return;
}

/*
 * 2MSL timeout TIMER
 */
static void tfTcpTm2MslOut (ttVoidPtr      timerBlockPtr,
                            ttGenericUnion userParm1,
                            ttGenericUnion userParm2)
{
    ttTcpTmWtVectPtr    tcpTmWtVectPtr;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);

    tcpTmWtVectPtr = (ttTcpTmWtVectPtr)userParm1.genVoidParmPtr;
    tm_call_lock_wait(&tm_context(tvTcpTmWtTableLock));
    if (tcpTmWtVectPtr->tws2MslTmPtr == (ttTimerPtr)timerBlockPtr)
    {
        tfTcpTmWtVectClose(tcpTmWtVectPtr);
    }
    tm_call_unlock(&tm_context(tvTcpTmWtTableLock));
    return;
}


/*
 * Free all recycled headers for a given state vetor
 */
void tfTcpPurgeTcpHeaders (ttTcpVectPtr tcpVectPtr)
{
    ttPacketPtr     packetPtr;
    ttPacketPtr     nextPacketPtr;

    packetPtr = tcpVectPtr->tcpsRecycleHdrPtr;
    while (packetPtr != TM_PACKET_NULL_PTR)
    {
        nextPacketPtr = (ttPacketPtr)packetPtr->pktLinkNextPtr;
/*
 * Reset TM_BUF_TCP_HDR_BLOCK flag so that tfFreeSharedBuffer does not
 * put back this packet on the tcp vector recycle list. (The other flag
 * reset is not necessary, but it does not cost us anything.)
 */
        tm_16bit_clr_bit( packetPtr->pktSharedDataPtr->dataFlags,
                          (TM_BUF_TCP_HDR_BLOCK|TM_BUF_HDR_RECYCLED) );
        tfFreeSharedBuffer(packetPtr, TM_SOCKET_LOCKED);
        packetPtr = nextPacketPtr;
    }
    tcpVectPtr->tcpsRecycleHdrPtr = TM_PACKET_NULL_PTR;
    tcpVectPtr->tcpsRecycleHdrCount = TM_16BIT_ZERO;
    return;
}

/*
 * tfTcpSendQeueFree() function description:
 * free ACKed buffers, update send queue, turn on TM_SELECT_WRITE, and
 * TM_CB_SEND_COMPLT to allow posting on socket send pend queue (done
 * in tfSocketNotify() at the end of tfTcpIncomingPacket()), update RTT
 * (since we are advancing the left edge of the send window) always if
 * time stamp is on, but, if time stamp is not on, update RTT only if
 * packet has not been re-transmitted (KARN's algorithm).
 * Parameters:
 * tcpVectPtr      Pointer to socket entry/tcp state vector
 * bytesAcked      Number of bytes being acked
 * rtt             Round trip time already calculated only if time stamp is on
 * computeRtoFlag  if off, we do not want to recompute the RTO
 *                 (Karn Algorithm)
 */
void tfTcpSendQueueFree (ttTcpVectPtr tcpVectPtr,
                         tt32Bit      bytesAcked,
                         ttS32Bit     rtt,
                         tt8Bit       computeRtoFlag)
{
    ttPacketPtr         packetPtr;
    ttPacketPtr         dumpPacketPtr;
    ttSocketEntryPtr    socketPtr;
#ifdef TM_DSP
    int                 dataOffset;
#endif /* TM_DSP */

    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
#ifdef TM_ERROR_CHECKING
    if (    (socketPtr->socSendQueueBytes < bytesAcked)
         || (socketPtr->socSendQueueNextPtr == TM_PACKET_NULL_PTR) )
    {
        tfKernelError("tfTcpSendQueueFree", "Socket send queue corrupted!");
    }
#endif /* TM_ERROR_CHECKING */
    socketPtr->socSendQueueBytes -= bytesAcked;
    tcpVectPtr->tcpsSendCompltBytes += bytesAcked;
/*
 * Post for any pending thread (send or select) (before we unlock).
 * (Done at the end of tfTcpIncomingPacket().)
 */
#ifdef TM_USE_SSL
    if (    (tm_16bit_bits_not_set(tcpVectPtr->tcpsSslFlags,
                                    TM_SSLF_SEND_ENABLED))
         || (tm_16bit_all_bits_set(tcpVectPtr->tcpsSslFlags,
                                   TM_SSLF_SEND_ENABLED | TM_SSLF_ESTABLISHED))
        )
#endif /* TM_USE_SSL */
    {
        if (    (    socketPtr->socMaxSendQueueBytes
                  -  socketPtr->socSendQueueBytes )
                                 >= socketPtr->socLowSendQueueBytes )
        {
            tcpVectPtr->tcpsSelectFlags |= TM_SELECT_WRITE;
            tcpVectPtr->tcpsSocketCBFlags |=   TM_CB_SEND_COMPLT
                                             | TM_CB_WRITE_READY;
        }
        else
        {
            tcpVectPtr->tcpsSocketCBFlags |= TM_CB_SEND_COMPLT;
        }
    }
    packetPtr = socketPtr->socSendQueueNextPtr;
    while ( (bytesAcked != TM_UL(0)) && (packetPtr != TM_PACKET_NULL_PTR) )
    {
        if ((tt32Bit) packetPtr->pktLinkDataLength > bytesAcked)
        {
/*
 * If bytesAcked is less than size of first. Update first and break out
 * of the loop
 */
#ifdef TM_DSP
            dataOffset = bytesAcked + packetPtr->pktLinkDataByteOffset;
            packetPtr->pktLinkDataPtr +=
                (unsigned)tm_packed_byte_len(dataOffset);
/* keep track of the 8-bit byte index (into the word) that has been ACK'ed */
            packetPtr->pktLinkDataByteOffset =
                    (dataOffset) % TM_DSP_BYTES_PER_WORD;
#else /* ! TM_DSP */
            packetPtr->pktLinkDataPtr += (unsigned)bytesAcked;
#endif /* TM_DSP */
            packetPtr->pktLinkDataLength -= (ttPktLen)bytesAcked;
#ifdef TM_OPTIMIZE_SPEED
            if (packetPtr == tcpVectPtr->tcpsSendQPacketPtr)
            {
/* Update cached send queue SQN */
                tcpVectPtr->tcpsSendQSqn = (ttSqn)
                                      (tcpVectPtr->tcpsSendQSqn + bytesAcked);
            }
#endif /* TM_OPTIMIZE_SPEED */
            if (tcpVectPtr->tcpsSndUna == tcpVectPtr->tcpsMaxSndNxt)
            {
/*
 * All sent data in the packet has been acknowledged, and there is more
 * data to send and not yet sent. Make sure the time flag is reset to
 * allow a fresh computation of rtt.
 */
                tm_16bit_clr_bit( packetPtr->pktFlags2,
                                  (TM_PF2_TCP_TIMED|TM_PF2_TCP_REXMIT) );
            }
            break;
        }
        else
        {
/*
 * If bytesAcked is more than size of first, update bytesAcked. If
 * there is a next shared buffer, free first and move to next.
 */
            bytesAcked -= packetPtr->pktLinkDataLength;
            dumpPacketPtr = packetPtr;
            packetPtr = (ttPacketPtr)packetPtr->pktLinkNextPtr;
#ifdef TM_OPTIMIZE_SPEED
            if (dumpPacketPtr == tcpVectPtr->tcpsSendQPacketPtr)
            {
/* Cache the next packet to send with its corresponding SQN */
                tcpVectPtr->tcpsSendQPacketPtr = packetPtr;
                tcpVectPtr->tcpsSendQSqn =   tcpVectPtr->tcpsSendQSqn
                                           + dumpPacketPtr->pktLinkDataLength;
            }
#endif /* TM_OPTIMIZE_SPEED */
            tfFreeSharedBuffer(dumpPacketPtr, TM_SOCKET_LOCKED);
        }
    }
/* update send queue */
    socketPtr->socSendQueueNextPtr = packetPtr;
#ifdef TM_ERROR_CHECKING
    if (packetPtr == TM_PACKET_NULL_PTR)
    {
        socketPtr->socSendQueueLastPtr = TM_PACKET_NULL_PTR;
    }
#endif /* TM_ERROR_CHECKING */
    tfTcpUpdateRtt(tcpVectPtr, (tt32Bit)rtt, computeRtoFlag);
    return;
}

/*
 * Called from the ICMP layer, when an ICMP error message is received from
 * the network.
 *
 * tcpVectPtr   Pointer to the TCP vector.
 * tlPtr        Pointer to the TCP header of the TCP packet encapsulated
 *              in the ICMP message.
 * errorCode    The network error code.
 * icmpCode     The ICMP error code.
 */
void tfTcpIcmpErrInput ( ttTcpVectPtr    tcpVectPtr,
                         ttTlHeaderPtr   tlPtr,
                         int             errorCode,
                         tt8Bit          icmpCode )
{
    ttSocketEntryPtr    socketPtr;
    ttTcpHeaderPtr      tcphPtr;
    ttSqn               seqNo;
#ifdef TM_PMTU_DISC
#ifdef TM_TCP_RFC2414
    tt16Bit             savEffSndMss;
#endif /* TM_TCP_RFC2414 */
#endif /* TM_PMTU_DISC */

#ifndef TM_USE_IPV4
    TM_UNREF_IN_ARG(icmpCode);
#endif /* TM_USE_IPV4 */

    if (tlPtr != (ttTlHeaderPtr)0)
    {
        tcphPtr = (ttTcpHeaderPtr)(ttVoidPtr)tlPtr;
        tm_ntohl(tcphPtr->tcpSeqNo, seqNo);
        if (    tm_sqn_lt(seqNo, tcpVectPtr->tcpsSndUna)
             || tm_sqn_geq(seqNo, tcpVectPtr->tcpsMaxSndNxt))
        {
            goto icmpErrorExit;
        }
    }
#ifdef TM_USE_IPV4
    if (errorCode == TM_EXT_TTLTRANSIT)
    {
        tfTcpPurgeTcpHeaders(tcpVectPtr);
    }
#endif /* TM_USE_IPV4 */
    if ((errorCode == TM_EHOSTUNREACH)
#ifdef TM_USE_IPV4
         && (icmpCode >= TM_ICMP_CODE_PROTO)
         && (icmpCode <= TM_ICMP_CODE_FRAG)
#endif /* TM_USE_IPV4 */
       )
    {
        (void)tfTcpAbort(tcpVectPtr, errorCode);
        goto icmpErrorExit;
    }
/* If connection not established, or error is soft */
    if (   (tcpVectPtr->tcpsState != TM_TCPS_ESTABLISHED)
        || (   (errorCode != TM_EHOSTUNREACH)
            && (errorCode != TM_ENETUNREACH)
            && (errorCode != TM_EHOSTDOWN) ) )
    {
/*
 * If trying to connect and retransmission has already taken place and
 * soft error has already occured before
 */
        if (    ( tcpVectPtr->tcpsState < TM_TCPS_ESTABLISHED )
             && ( tcpVectPtr->tcpsReXmitCnt >
                    (int)((unsigned int)(tcpVectPtr->tcpsMaxReXmitCnt) >> 2) )
             && ( tcpVectPtr->tcpsSoftError != 0 ) )
        {
            socketPtr = &(tcpVectPtr->tcpsSocketEntry);
/*
 * Connect error (write error), but notify any reader just in case connect
 * was non blocking
 */
            if (tm_8bit_one_bit_set( socketPtr->socFlags2,
                                     TM_SOCF2_CONNECTING ) )
            {
/* Set the connect error code */
                tfSocketErrorRecord((int)socketPtr->socIndex, errorCode);
            }
            tfSocketNotify(socketPtr,
                           TM_SELECT_READ|TM_SELECT_WRITE,
                           TM_CB_SOCKET_ERROR,
                           errorCode);
            goto icmpErrorExit;
        }
        else
        {
            if (errorCode == TM_EXT_SRCQNCH)
            {
/* Attempt slow start, if allowed by user */
                if ( tm_16bit_one_bit_set( tcpVectPtr->tcpsFlags,
                                           TM_TCPF_SLOW_START ) )
                {
/* RFC 2581 upate: change IW to 2 times MSS.  [RFC2581]R3.1:1  */
                    tcpVectPtr->tcpsCwnd = tm_tcp_iw(tcpVectPtr);
                }
                goto icmpErrorExit;
            }
            else
            {
#ifdef TM_PMTU_DISC
                if (errorCode == TM_EMSGSIZE)
                {
#ifdef TM_TCP_RFC2414
/* Save current MSS */
                    savEffSndMss = tcpVectPtr->tcpsEffSndMss;
#endif /* TM_TCP_RFC2414 */
/* Re-adjust our MSS according to the new Path MTU estimate */
                    (void)tfTcpSetMss(tcpVectPtr);
#ifdef TM_TCP_RFC2414
                    if ( tm_16bit_all_bits_set( tcpVectPtr->tcpsFlags,
                                                TM_TCPF_INITIAL_WINDOW ) )
                    {
/* Re-adjust the congestion window accorging to the new Path MTU estimate */
                        if (    (tcpVectPtr->tcpsInitialCwnd == tcpVectPtr->tcpsCwnd)
                             || (savEffSndMss == TM_16BIT_ZERO) )
                        {
                            tcpVectPtr->tcpsCwnd = 2 * tcpVectPtr->tcpsEffSndMss;
                            if (   (tcpVectPtr->tcpsCwnd < TM_UL(4380))
                                 || (savEffSndMss == TM_16BIT_ZERO) )
                            {
                                tcpVectPtr->tcpsCwnd =
                                                    4 * tcpVectPtr->tcpsEffSndMss;
                                if (tcpVectPtr->tcpsCwnd > TM_UL(4380))
                                {
                                    tcpVectPtr->tcpsCwnd = TM_UL(4380);
                                }
                            }
                            tcpVectPtr->tcpsInitialCwnd = tcpVectPtr->tcpsCwnd;
                        }
                        else
                        {
                             tcpVectPtr->tcpsCwnd =
                                (tcpVectPtr->tcpsCwnd * tcpVectPtr->tcpsEffSndMss)
                              / savEffSndMss;
                        }
                    }
#endif /* TM_TCP_RFC2414 */
/*
 * Set a flag to indicate that we should re-time all packets that we will
 * retransmit, because we know for sure that the packets have been dropped
 * and hence we will not get an ACK for the already sent packets.
 */
                    tcpVectPtr->tcpsFlags2 = (tt16Bit)
                                 (tcpVectPtr->tcpsFlags2 | TM_TCPF2_DF_ERROR);
/* Error indicates that our packets were dropped. Resend all TCP data */
                    tcpVectPtr->tcpsSndNxt = tcpVectPtr->tcpsSndUna;
                    (void)tfTcpSendPacket(tcpVectPtr);
                }
                else
#endif /* TM_PMTU_DISC */
                {
                    tcpVectPtr->tcpsSoftError = errorCode;
                }
            }
        }
    }
icmpErrorExit:
    return;
}

/*
 * Get a TCP Header from the TCP state vector's recycle list of TCP headers
 * to avoid filling in the header, over and over
 * with fields that do not change on the connection
 * If the connection TCP headers recycle list is empty,
 * allocate a new TCP header and initialize TCP and IP header fields.
 */
ttPacketPtr tfTcpGetTcpHeader (ttTcpVectPtr tcpVectPtr)
{
    ttPacketPtr         packetPtr;
#ifdef TM_USE_IPV4
    ttIpHeaderPtr       iphPtr;
#endif /* !TM_USE_IPV4 */
    ttRtCacheEntryPtr   socCacheRtcPtr;
    ttRteEntryPtr       packetRtePtr;
    ttSocketEntryPtr    socketPtr;
    int                 maxLlpHeaderSize;
    tt16Bit             tcpHeaderSize;
    tt16Bit             recycleAllowed;
#if (defined(TM_6_USE_MIP_MN) && defined(TM_6_USE_MIP_RO))
    tt6MnRrBulEntryPtr  rrBulPtr;
    tt32Bit             elapsedTickCount;
#endif /* TM_6_USE_MIP_MN && TM_6_USE_MIP_RO */
#ifdef TM_USE_IPV6
    int                 tempInt;
#endif /* TM_USE_IPV6 */
    ttUser8BitPtr       enetDestAddrPtr;

    packetPtr = TM_PACKET_NULL_PTR;
    tcpHeaderSize = tcpVectPtr->tcpsHeaderSize;
    packetPtr = tcpVectPtr->tcpsRecycleHdrPtr;
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    socCacheRtcPtr = &socketPtr->socRteCacheStruct;
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
#if (defined(TM_6_USE_MIP_MN) && defined(TM_6_USE_MIP_RO))
        if (socCacheRtcPtr->rtcRtePtr != TM_RTE_NULL_PTR)
        {
/* If we have a route optimization BUL cached, check if the lifetime has
   expired. If it has, then we must not route optimize this packet.
   ([MIPV6_18++].R11.1:30) */
/* lock the mobile node state vector */
            tm_lock_wait(&(tm_context(tv6MnVect).mns6LockEntry));

            rrBulPtr = (tt6MnRrBulEntryPtr)
                socketPtr->socRteCacheStruct.rtc6MnRrBulPtr;
/* if the socket has cached a pointer to a BUL entry, then we need to check
   for an expired binding */
            if (rrBulPtr != TM_6_MN_RR_BUL_NULL_PTR)
            {
                if (rrBulPtr->rrb6LifetimeMsec != 0)
                {
/* we have an active binding, but is it really??? Verify that the lifetime
   hasn't expired. */
                    tm_kernel_set_critical;
                    elapsedTickCount = (tvTime - rrBulPtr->rrb6BuAckTime);
                    tm_kernel_release_critical;

                    if (elapsedTickCount >= rrBulPtr->rrb6LifetimeMsec)
                    {
/* This binding has expired, we cannot use it for route optimization. */
/* Set the binding lifetime to 0, so that we know that this BUL entry can't be
   used for route optimization. Note that it must stay in the BOUND state,
   because we have a socket lock so it is unsafe to call
   tf6MnRrResetRouteCaches. */
                        rrBulPtr->rrb6LifetimeMsec = 0;
                    }
                }

/* NOTE: the following code cannot be an "else" block for the previous "if"
   block, it is separate to support multiple TCP sockets sharing the same BUL
   entry, in which case the first to send will set the BUL lifetime to 0. The
   other sockets still need to have their route caches reset and recycled TCP
   headers purged later when they send. If we could have called
   tf6MnRrResetRouteCaches from this context, it would have cleaned up all
   associated sockets and then this logic would be simpler. */
                if (rrBulPtr->rrb6LifetimeMsec == 0)
                {
/* uncache the cached route pointer so that we purge recycled TCP headers */
                    tfRtCacheUnGet(socCacheRtcPtr);
/* the following line of code is very important, since otherwise we would
   keep executing this code for every single TCP packet we send on this
   socket until the cached BUL is either refreshed or removed */
                    socCacheRtcPtr->rtc6MnRrBulPtr = TM_VOID_NULL_PTR;
                }
            }

/* unlock the mobile node state vector */
            tm_unlock(&(tm_context(tv6MnVect).mns6LockEntry));
        }
#endif /* TM_6_USE_MIP_MN && TM_6_USE_MIP_RO */

/* Check whether we can use the recycled header */
        packetRtePtr = packetPtr->pktSharedDataPtr->dataRtePtr;
        if (    (socCacheRtcPtr->rtcRtePtr == TM_RTE_NULL_PTR)
/* If cache route pointer has been reset */
             || tm_16bit_bits_not_set(socCacheRtcPtr->rtcRtePtr->rteFlags2, TM_RTE2_UP)
/* or cache points to a route entry that has been removed */
             || (packetRtePtr == TM_RTE_NULL_PTR)
/* or packet routing entry has been reset */
             || (    (packetRtePtr != socCacheRtcPtr->rtcRtePtr)
                  && (packetRtePtr != socCacheRtcPtr->rtcRteClonedPtr) )
/*
 * or packet routing entry does not match (either socket cache entry, or ARP
 * cache entry).
 */
             || (    tm_16bit_one_bit_set(packetRtePtr->rteFlags, TM_RTE_ARP)
                  && tm_16bit_bits_not_set( packetRtePtr->rteFlags,
                                            TM_RTE_LINK_LAYER) )
/* or if packetRtePtr points to an Arp entry, it is not valid */
#ifdef TM_USE_IPV6
             || (    (tcpVectPtr->tcpsNetworkLayer == TM_NETWORK_IPV6_LAYER)
                  && (socCacheRtcPtr->rtcSrcMhomeSet)
                  && (    !(   (socCacheRtcPtr->rtcSrcDevPtr->
                                   dev6IpAddrFlagsArray[
                                            socCacheRtcPtr->rtcSrcMhomeIndex])
                              & TM_6_DEV_IP_CONFIG)
                        ||  !(tm_6_ip_match(
                                  &(socketPtr->socTuple.sotLocalIpAddress),
                                  &tm_6_ip_dev_addr(
                                            socCacheRtcPtr->rtcSrcDevPtr,
                                            socCacheRtcPtr->rtcSrcMhomeIndex)))
                     )
                )
/* or IPV6 source address lifetime has expired (non link local addresses) */
#endif /* TM_USE_IPV6 */
           )
        {
/* cannot use the recycled header */
            packetPtr = TM_PACKET_NULL_PTR;
/*
 * Since the first TCP header could not be re-used, and the TCP headers
 * are LIFO stored, the other ones in the list are not re-usable
 * either. So purge all the TCP headers.
 */
            tfTcpPurgeTcpHeaders(tcpVectPtr);
/* Re-initialize the TCP routing cache and the MSS, if need be */
            (void)tfTcpSetMss(tcpVectPtr);
        }
        else
        {
            if (tm_16bit_one_bit_set(packetRtePtr->rteFlags, TM_RTE_ARP))
            {
/*
 * Copy the ARP mapping unconditionally in case the ARP entry had been
 * updated with a new physical address.
 * Note this code assumes that the recycled header points to the link layer
 * header (ttEtherHeader). That way the code works for both BYPASS_ETHER_LL,
 * and not BYPASS_ETHER_LL. Another way would be to modify trip.c/t6ip.c, and
 * always copy the ethernet destination in the shared data structure even if
 * we by pass the ethernet link layer, and use that value instead.
 */
                enetDestAddrPtr = (tt8BitPtr) (ttVoidPtr)
                    &(((ttEtherHeaderPtr)
                       (ttVoidPtr) packetPtr->pktLinkDataPtr)->
                      ethDstAddr);
#ifdef TM_USE_IPV6
                    tm_6_phys_addr_copy(
                        packetRtePtr->rteHSPhysAddr,
                        enetDestAddrPtr,
                        packetRtePtr->rteHSPhysAddrLen,
                        tempInt);
#else /* ! TM_USE_IPV6 */
                    tm_ether_copy(
                        packetRtePtr->rteHSEnetAdd, enetDestAddrPtr);
#endif /* ! TM_USE_IPV6 */
            }
        }
    }
    if (packetPtr == TM_PACKET_NULL_PTR)
    {
/*
 * tcpsFlags tells us if recycling is allowed. (Recycle is disallowed
 * when the TCP state is TimeWait or closed)
 */
        recycleAllowed = (tt16Bit)(
                               (TM_TCPF_RECYCLE_HDR | TM_TCPF_SOCKETENTRYPTR)
                             & tcpVectPtr->tcpsFlags);
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
        if (tcpVectPtr->tcpsNetworkLayer == TM_NETWORK_IPV6_LAYER)
#endif /* dual */
        {
/* DSP: Byte value */
            maxLlpHeaderSize = TM_IP_DEF_MAX_HDR_LEN + TM_LL_MAX_HDR_LEN;
        }
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
        else
#endif /* dual */
        {
/* DSP: Byte value */
            maxLlpHeaderSize = TM_4_IP_MIN_HDR_LEN + TM_LL_MAX_HDR_LEN;
        }
#endif /* TM_USE_IPV4 */
        packetPtr = tfGetSharedBuffer(
            maxLlpHeaderSize, (ttPktLen)TM_MAX_TCP_HDR_LEN, recycleAllowed );
/*
 * Fill in default TCP/IP header fields
 */
        if (packetPtr == TM_PACKET_NULL_PTR)
        {
            goto getTcpHeaderExit;
        }
#ifdef TM_USE_USER_PARAM
/*
 * Copy the user-defined TCP connection specific data to the packet so that
 * it can be accessed by a device driver function that has been registered
 * via tfUseInterfaceOneScatSend() call.
 */
        tm_bcopy(   &tcpVectPtr->tcpsUserParam,
                    &packetPtr->pktUserStruct.pktuUserParam,
                    sizeof(ttUserGenericUnion) );
#endif /* TM_USE_USER_PARAM */
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
        if (tcpVectPtr->tcpsNetworkLayer == TM_NETWORK_IPV4_LAYER)
#endif /* dual */
        {
            packetPtr->pktIpHdrLen = TM_4PAK_IP_MIN_HDR_LEN; /* packed */
            iphPtr = (ttIpHeaderPtr)(   packetPtr->pktLinkDataPtr
/* packed IPv4 minimum header length */
                                      - TM_4PAK_IP_MIN_HDR_LEN );
            iphPtr->iphFlagsFragOff = TM_16BIT_ZERO;
#ifdef TM_PMTU_DISC
            if (    (tm_16bit_all_bits_set(
                                 tcpVectPtr->tcpsFlags,
                                 TM_TCPF_INDIRECT | TM_TCPF_PMTU_DISCOVERY) )
#ifdef TM_USE_TCP_PACKET
                 && (tm_16bit_bits_not_set( tcpVectPtr->tcpsFlags,
                                            TM_TCPF_PACKET ))
#endif /* TM_USE_TCP_PACKET */
               )
/* Cannot use PMTU discovery if TM_TCP_PACKET option is set */
            {
/* Re-initialize the routing cache/MSS, if need be */
                (void)tfTcpSetMss(tcpVectPtr);
                if (    ( socCacheRtcPtr->rtcRtePtr != (ttRteEntryPtr)0 )
                     && ( tm_16bit_one_bit_set(
                                     socCacheRtcPtr->rtcRtePtr->rteFlags,
                                     TM_RTE_INDIRECT ) )
                     && ( tm_16bit_bits_not_set(
                                     socCacheRtcPtr->rtcRtePtr->rteFlags,
                                     TM_RTE_NO_PMTU_DISC ) ) )
                {
/* Path MTU discovery */
                    iphPtr->iphFlagsFragOff = TM_IP_DONT_FRAG;
                }
            }
#endif /* TM_PMTU_DISC */
            iphPtr->iphUlp = TM_IP_TCP;
        }
#endif /* TM_USE_IPV4 */
        if (recycleAllowed != TM_16BIT_ZERO)
        {
/* Allow recycling */
            packetPtr->pktSharedDataPtr->dataSocketEntryPtr = socketPtr;
/* Checkout since packet has pointer to socket */
            tm_socket_checkout(socketPtr);
        }

/* Needed for ARP refresh in the send path when using recycled headers */
        if ( tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_INDIRECT) )
        {
            packetPtr->pktFlags2 |= TM_PF2_INDIRECT;
        }
    }
    else
    {
/* Remove first recycled header from the recycle list */
        tcpVectPtr->tcpsRecycleHdrPtr = (ttPacketPtr)packetPtr->pktLinkNextPtr;
        tcpVectPtr->tcpsRecycleHdrCount = (tt16Bit)
                                        (tcpVectPtr->tcpsRecycleHdrCount - 1);
/* set the recycle flags only (i.e get rid of all other flags)*/
        packetPtr->pktSharedDataPtr->dataFlags = (tt16Bit)
         (TM_BUF_SOCKETENTRYPTR | TM_BUF_TCP_HDR_BLOCK | TM_BUF_HDR_RECYCLED);
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
        if (tcpVectPtr->tcpsNetworkLayer == TM_NETWORK_IPV6_LAYER)
#endif /* dual */
        {
/* DSP: Packed value. */
            maxLlpHeaderSize =
                TM_PAK_IP_DEF_MAX_HDR_LEN + TM_PAK_LL_MAX_HDR_LEN;
            if (packetPtr->pkt6TxIphPtr == (tt6IpHeaderPtr)0)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning(
                    "tfTcpGetTcpHeader",
                    "Found a recycled header without IPv6 pointer");
#endif /* TM_ERROR_CHECKING */
/* cannot assume anything on the connection */
                tm_16bit_clr_bit(
                                packetPtr->pktSharedDataPtr->dataFlags,
                                (TM_BUF_TCP_HDR_BLOCK|TM_BUF_HDR_RECYCLED) );
            }
        }
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
        else
#endif /* dual */
        {
/* Packed value. */
            maxLlpHeaderSize =
                TM_4PAK_IP_MIN_HDR_LEN + TM_PAK_LL_MAX_HDR_LEN;
        }
#endif /* TM_USE_IPV4 */
/* Point to TCP header */
        packetPtr->pktLinkDataPtr =
                                packetPtr->pktSharedDataPtr->dataBufFirstPtr
                              + maxLlpHeaderSize; /* packed value */
/*
 * We need to re-initialize pktLinkNextPtr, pktLinkExtraCount,
 * pktLinkDataLength and pktChainDataLength since they get changed by the
 * TCP/IP/link layer.
 */
        packetPtr->pktLinkNextPtr = (ttVoidPtr)0;
        packetPtr->pktLinkExtraCount = 0;
/* Re-initialize pktChainNextPtr, in case we need to queue for ARP resolution */
        packetPtr->pktChainNextPtr = (ttPacketPtr)0;
    }
/*
 * TCP Header size, and packet length without SEL ACKs. To be updated when
 * sending SEL ACKS, or SYN/RST/Keep Alive.
 */
    ((ttTcpHeaderPtr)packetPtr->pktLinkDataPtr)->tcpDataOffset =
                                        tm_tcp_set_data_offset(tcpHeaderSize);
    packetPtr->pktLinkDataLength = tcpHeaderSize;
    packetPtr->pktChainDataLength = tcpHeaderSize;
#ifdef TM_DEV_SEND_OFFLOAD
    packetPtr->pktDevOffloadPtr = (ttVoidPtr)0;
#endif /* TM_DEV_SEND_OFFLOAD */
#ifdef TM_USE_QOS
#ifdef TM_USE_IPV4
    packetPtr->pktIpTos = socketPtr->socIpTos;
    packetPtr->pktIpTosSet = 1;
#else /* !TM_USE_IPV4 */
    packetPtr->pktIpTos = 0;
    packetPtr->pktIpTosSet = 0;
#endif /* !TM_USE_IPV4 */
#endif /* TM_USE_QOS */
getTcpHeaderExit:
    return(packetPtr);
}

/*
 * tfTcpVectInit() function description
 * Called by the socket interface, when the user opens a socket to
 * initialize the TCP state vector
 *  Fill in default values in state vector.
 *
 * Parameters
 * tcpVectPtr    Pointer to TCP state vector entry/socket entry
 *
 * No return value
 */
void tfTcpVectInit (ttTcpVectPtr tcpVectPtr)
{
    ttSocketEntryPtr        socketPtr;
    int                     queueIndex;

    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    socketPtr->socLingerTime = tm_context(tvDefLingerTime); /* in secs */
    socketPtr->socOptions |= SO_LINGER;
    socketPtr->socSendAppendThreshold =
                                    tm_context(tvDefSendAppendThreshold);
    socketPtr->socRecvCopyFraction =
                                    tm_context(tvDefRecvAppendFraction);
/*
 * Empty circular lists of connection requests (half connected, and
 * fully connected)
 */
    for (queueIndex = 0; queueIndex <= TM_TCP_ACCEPT_QUEUE; queueIndex++)
    {
        tcpVectPtr->tcpsConQueue[queueIndex].tcpConReqPrevPtr = tcpVectPtr;
        tcpVectPtr->tcpsConQueue[queueIndex].tcpConReqNextPtr = tcpVectPtr;
    }
/* Minimum retransmission time out */
    if (tm_context(tvTcpMinRtoTime) < (tt32Bit)tvTimerTickLength)
    {
        tm_context(tvTcpMinRtoTime) = (tt32Bit)tvTimerTickLength;
    }
    if (tm_context(tvTcpMinProbeWndTime) < (tt32Bit)tvTimerTickLength)
    {
        tm_context(tvTcpMinProbeWndTime) = (tt32Bit)tvTimerTickLength;
    }
    if (tm_context(tvTcpKeepAliveTime) < TM_TCP_KEEPALIVE_TIME)
    {
        tm_context(tvTcpKeepAliveTime) = TM_TCP_KEEPALIVE_TIME;
    }
/*
 * Amount of time before a connection is broken, when TCP start retramsit
 * connection request, or data (the later only if enabled by option).
 */
    tcpVectPtr->tcpsMaxRt = tm_context(tvTcpConnTime);
/* Max retransmission count */
    tcpVectPtr->tcpsMaxReXmitCnt = tm_context(tvTcpMaxReXmitCnt);
/* min Tcp zero window probe time (in milliseconds) */
    tcpVectPtr->tcpsProbeMin = tm_context(tvTcpMinProbeWndTime);
/* max Tcp zero window probe time (in milliseconds) */
    tcpVectPtr->tcpsProbeMax = tm_context(tvTcpMaxProbeWndTime);
/*
 * Amount of time to stay in FIN WAIT 2 when socket has been closed
 * (10 minutes) in seconds.
 */
     tcpVectPtr->tcpsFinWt2Time = tm_context(tvTcpFinWt2Time);
/*
 * 2*Maximum segment life time (2*30 seconds) (TIME WAIT time) in
 * milliseconds
 */
    tcpVectPtr->tcps2MSLTime = tm_context(tvTcp2MSLTime);
/* Keep alive timer (2 hours) in seconds */
    tcpVectPtr->tcpsKeepAliveTime = tm_context(tvTcpKeepAliveTime);
/* Keep alive Probe Interval/Idle update time (75s) in seconds. */
    tcpVectPtr->tcpsIdleIntvTime = tm_context(tvTcpIdleIntvTime);
/* Keep alive Max Probe count (8) */
    tcpVectPtr->tcpsKeepAliveProbeCnt = tm_context(tvTcpKeepAliveProbeCnt);
    tcpVectPtr->tcpsFlags = (tt16Bit)(   tm_context(tvTcpDefOptions)
#ifdef TM_USE_IPSEC
                                       | TM_TCPF_RECYCLE_HDR
                                       | TM_TCPF_SOCKETENTRYPTR
#endif /* TM_USE_IPSEC */
                                       | TM_TCPF_PMTU_DISCOVERY );
    tcpVectPtr->tcpsFlags2 |= TM_TCPF2_SEND_CLEAR; /* OK to send */
    tcpVectPtr->tcpsDelayAckTime = tm_context(tvTcpDelayAckTime);
    tcpVectPtr->tcpsMSS = tm_context(tvTcpDefMss);
    if (tcpVectPtr->tcpsMSS != TM_16BIT_ZERO)
    {
        tcpVectPtr->tcpsMaxRecycleHdrCount = (tt16Bit)
                 (tt32Bit)(tm_context(tvDefSendQueueBytes)
                                              / (tt32Bit)tcpVectPtr->tcpsMSS);
    }
/* Scaled smoothed round trip time (scale factor is 8)*/
    tcpVectPtr->tcpsScSmRtt = TM_L(0); /* no calculation yet */
/* Default Tcp retransmission time (milliseconds) */
    tcpVectPtr->tcpsRtoDef = tm_context(tvTcpDefRtoTime);
/* min Tcp retransmission time (in milliseconds) */
    tcpVectPtr->tcpsRtoMin = tm_context(tvTcpMinRtoTime);
/* max Tcp retransmission time (in milliseconds) */
    tcpVectPtr->tcpsRtoMax = tm_context(tvTcpMaxRtoTime);
/* tcpsScRttVar, and tcpsRto first computation in tfTcpInitConTmr */
/*
 * Assume maximum TCP header size for a connection request.
 * This value will be updated upon receiving the peer's TCP options
 */
/* Maximum TCP header with options */
    tcpVectPtr->tcpsHeaderSize = TM_MAX_TCP_HDR_LEN;
    return;
}

/*
 * Close the TCP connection. Called from either a user close
 */
int tfTcpClose (ttTcpVectPtr tcpVectPtr)
{
    ttSocketEntryPtr        socketPtr;
    int                     errorCode;
#ifdef TM_PEND_POST_NEEDED
    ttGenericUnion          timerParm1;
    tt8Bit                  lingerOn;
#endif /* TM_PEND_POST_NEEDED */
    tt8Bit                  event;

    errorCode = TM_ENOERROR;
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
#ifdef TM_PEND_POST_NEEDED
    lingerOn = TM_8BIT_ZERO;
    if (tm_16bit_one_bit_set(socketPtr->socOptions, SO_LINGER))
/* Note that linger on with timeout of zero is now handled in tfClose() */
    {
/* only allow linger if socket is blocking */
        if (    tm_16bit_bits_not_set( socketPtr->socFlags,
                                       TM_SOCF_NONBLOCKING )
             && (tvPendPostEnabled != TM_8BIT_ZERO) )
        {
/*
 * Linger option on with non zero timeout value and blocking socket (this is
 * the default setting for a socket (default timeout value is 60 seconds)).
 */
            lingerOn = TM_8BIT_YES;
        }
    }
#endif /* TM_PEND_POST_NEEDED */
    if (    (tm_tcp_soc_recv_q_bytes(socketPtr, tcpVectPtr) == TM_32BIT_ZERO)
         || (tm_tcp_soc_send_q_bytes(socketPtr, tcpVectPtr) != TM_32BIT_ZERO) )
    {
/* No pending data, or we are not done sending our data. Normal close. */
        event = TM_TCPE_CLOSE;
/* Read shutdown */
        tfTcpShutRead(tcpVectPtr);
    }
    else
    {
/*
 * [RFC2525.2.17].1
 * Pending recv data, and no queued send data. Send a RESET.
 * (RFC 1122 + RFC 2525)
 */
/*
 * TCP half-duplex Close:
 * RFC 1122 says that if any data is present in the receive queue when
 * we close the connection, we need to send a RESET, if the user could
 * no longer read the data.
 * RFC 2525 problem 2.17: failure to RST on close with data pending.
 * (Note that if we have pending data in the send queue we would not
 *  send a RESET, so that our queued data can still be sent.)
 */
        event = TM_TCPE_ABORT;
    }
#ifdef TM_USE_SSL
/* User will no longer be able to process handshakes, etc.. */
    tm_16bit_clr_bit(tcpVectPtr->tcpsSslFlags, TM_SSLF_USER_HANDSHK_PROCESS);
#endif /* TM_USE_SSL */
/* Crank the state machine with the close or abort event */
    errorCode = tfTcpStateMachine( tcpVectPtr,
                                   TM_PACKET_NULL_PTR,
                                   event );
/*
 * If tfTcpCompleteClose() has already been called other than in the state
 * machine call above, make sure we call it again, so that we can signal
 * the user, and free the socket entry from the socket array.
 */
    if (    (tcpVectPtr->tcpsState >= TM_TCPS_ESTABLISHED)
         && (tm_16bit_one_bit_set( socketPtr->socFlags,
                                   TM_SOCF_DISCONNECTED)) )
    {
#ifdef TM_PEND_POST_NEEDED
/*
 * If TM_SOCF_DISCONNECTED is set, and state is bigger than
 * TM_TCPS_ESTABLISHED, then tfTcpCompleteClose() has already
 * been called, i.e. the TCP state has already changed to TIME_WAIT
 * or FIN_WAIT_2 with an empty send queue.
 */
        lingerOn = TM_8BIT_ZERO;
#endif /* TM_PEND_POST_NEEDED */
        (void)tfTcpCompleteClose(tcpVectPtr);
    }
#ifdef TM_PEND_POST_NEEDED
/*
 * If linger is on.
 * We need to wait until the TCP state changes to either CLOSED or TIME_WAIT
 * or FIN_WAIT_2 with an empty send queue, or up until the timeout value,
 * whichever comes first.
 */
    if (lingerOn)
    {
        if (tcpVectPtr->tcpsState != TM_TCPS_CLOSED)
        {
            timerParm1.genVoidParmPtr = (ttVoidPtr)socketPtr;
            tm_socket_checkout(socketPtr);
            tcpVectPtr->tcpsLingerTmPtr =
                    tfTimerAddExt( &tcpVectPtr->tcpsLingerTm,
                                   tfTcpTmLinger,
                                   tfTcpTmrCleanup,
                                   timerParm1,
                                   timerParm1, /* unused */
                                   (tt32Bit)socketPtr->socLingerTime *
                                    TM_UL(1000),
                                   0);
            errorCode = tm_pend( &socketPtr->socClosePendEntry,
                                 &socketPtr->socLockEntry );
        }
    }
#endif /* TM_PEND_POST_NEEDED */
    return errorCode;
}

#ifdef TM_PEND_POST_NEEDED
/*
 * Socket linger timeout timer
 */
static void tfTcpTmLinger (ttVoidPtr      timerBlockPtr,
                           ttGenericUnion userParm1,
                           ttGenericUnion userParm2)
{
    ttTcpVectPtr        tcpVectPtr;
    ttSocketEntryPtr    socketPtr;

/* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    TM_UNREF_IN_ARG(timerBlockPtr);

    tcpVectPtr = (ttTcpVectPtr) userParm1.genVoidParmPtr;
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    tm_call_lock_wait(&socketPtr->socLockEntry);
    if (tcpVectPtr->tcpsLingerTmPtr == (ttTimerPtr)timerBlockPtr)
    {
        tcpVectPtr->tcpsLingerTmPtr = TM_TMR_NULL_PTR;
        tm_post(&socketPtr->socClosePendEntry,
                &socketPtr->socLockEntry,
                TM_ETIMEDOUT);
    }
    tm_call_unlock(&socketPtr->socLockEntry);
    return;
}
#endif /* TM_PEND_POST_NEEDED */

/*
 * User cannot recv any more data
 * Flush receive and reassembly queues and SEL ACK block entries.
 */
void tfTcpShutRead (ttTcpVectPtr tcpVectPtr)
{
    ttSocketEntryPtr        socketPtr;

    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    socketPtr->socFlags |= TM_SOCF_NO_MORE_RECV_DATA;

    if (socketPtr->socReceiveQueueNextPtr != (ttPacketPtr)0)
/* Free items in the TCP recv queue */
    {
        tfFreePacket(socketPtr->socReceiveQueueNextPtr,
                     TM_SOCKET_LOCKED);
        socketPtr->socReceiveQueueNextPtr = TM_PACKET_NULL_PTR;
        socketPtr->socReceiveQueueLastPtr = TM_PACKET_NULL_PTR;
        socketPtr->socRecvQueueBytes = TM_32BIT_ZERO;
    }
    if (tcpVectPtr->tcpsReassemblePtr != (ttPacketPtr)0)
/* Free items in the TCP reassembly queue */
    {
        tfFreeChainPacket(tcpVectPtr->tcpsReassemblePtr, TM_SOCKET_LOCKED);
        tcpVectPtr->tcpsReassemblePtr = TM_PACKET_NULL_PTR;
    }
#ifdef TM_TCP_SACK
    tfTcpSackFreeQueue(tcpVectPtr, TM_TCP_SACK_SEND_INDEX);
#endif /* TM_TCP_SACK */
    return;
}

/*
 * Write Shutdown:
 * Send a FIN on the TCP connection if appropriate. User can no longer
 * send data.
 */
int  tfTcpShutWrite(ttTcpVectPtr tcpVectPtr)
{
    ttSocketEntryPtr    socketPtr;
    int                 errorCode;

    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
/* User can no longer send data */
    socketPtr->socFlags |= TM_SOCF_NO_MORE_SEND_DATA;
/* Crank the state machine with the close event */
    errorCode = tfTcpStateMachine( tcpVectPtr,
                                   TM_PACKET_NULL_PTR,
                                   TM_TCPE_CLOSE );
    return errorCode;
}

/*
 * tfTcpSetMssWnd Function Description:
 * 1. We get the MTU from the route (done in tfTcpSetMss())
 * 2. We take the smallest of our MSS and the peer MSS (done in tfTcpSetMss())
 * 3. We round up the send and queue sizes to a multiple of MSS (minus
 *    headers).
 *
 * Parameters
 * tcpVectPtr    Pointer to TCP state vector entry/socket entry
 *
 * Return value:
 * 0 on success
 * TM_EHOSTUNREACH if no route to host.
 *
 * This function is called either when
 * a. We are attempting to connect (sending a SYN request) (tfTcpSendSyn())
 *    (state == CLOSED (we have postponed the state transition until
 *    after this routine).
 * b. We have received a SYN-ACK (peer sending a SYN and acking our SYN
 *    request) (TM_TCPM_RCV_SYN_ACK) (tfTcpRcvSynOptions())
 *    (state == ESTABLISHED)
 * c. We have received a simultaneous SYN from the remote
 *    (TM_TCPM_SIMULT_OPEN) (tfTcpRcvSynOptions()) (state == SYN_RECEIVED)
 * d. We have received a first SYN from the remote (peer sending a
 *    connection request) (TM_TCPM_CON_REQUEST) (tfTcpRcvSynOptions())
 *    (state == LISTEN (state transition postponed until after
 *    tfTcpRcvSynOptions() returns successfully to make sure we have a
 *    routing entry to the remote)
 * Note that a SYN has been sent by us in cases b. and c., but a SYN
 * has not yet been sent by us in cases a. and d.
 */
int tfTcpSetMssWnd (ttTcpVectPtr tcpVectPtr)
{
    ttSocketEntryPtr    socketPtr;
    tt32Bit             maxQueueBytes;
    int                 errorCode;
    tt16Bit             mss;
    tt8Bit              rcvWndScale;
#ifdef TM_USE_SOCKET_IMPORT
    ttUserTcpConPtr     userTcpConPtr;
#endif /* TM_USE_SOCKET_IMPORT */

    errorCode = tfTcpSetMss(tcpVectPtr);
    if (errorCode != TM_ENOERROR)
    {
        goto outTcpSetMssWnd;
    }
    mss = tcpVectPtr->tcpsEffSndMss;
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    maxQueueBytes = socketPtr->socMaxSendQueueBytes;
    if (maxQueueBytes >= (tt32Bit)mss)
    {
/*
 * Adjust max send size to make it a multiple of EffSndMss
 */
        if (mss == TM_16BIT_ZERO)
        {
            tcpVectPtr->tcpsMaxRecycleHdrCount = TM_16BIT_ZERO;
        }
        else
        {
            socketPtr->socMaxSendQueueBytes =
                               tm_roundup(maxQueueBytes, (tt32Bit)mss);
            tcpVectPtr->tcpsMaxRecycleHdrCount = (tt16Bit)
                (tt32Bit)(socketPtr->socMaxSendQueueBytes / (tt32Bit)mss);
        }
    }
/*
 * Try and adjust the max receive window to also make it a multiple of
 * EffSndMss, but see restriction below
 */
    maxQueueBytes = socketPtr->socMaxRecvQueueBytes;
/*
 * Round up because we cannot shrink the receive window.
 * ANVL-PERF 2.23 note:
 * Rounding the receive window to a multiple of MSS is more efficient with TCP
 * because of the TCP Nagle algorithm (trying to send full size segments.)
 * But note that ANVL-PERF 2.23 does not expect us to change the recv queue
 * size.
 */
#ifdef TM_USE_SOCKET_IMPORT
    userTcpConPtr = tcpVectPtr->tcpsUserTcpConPtr;
    if (userTcpConPtr != (ttUserTcpConPtr)0)
    {
        rcvWndScale = userTcpConPtr->uconRcvWndScale;

/* When creating a socket in the established state, grow our receive
 * window to match that of the imported socket. */
        if (maxQueueBytes < userTcpConPtr->uconRcvWnd)
        {
            maxQueueBytes = userTcpConPtr->uconRcvWnd;
        }
    }
    else
#endif /* TM_USE_SOCKET_IMPORT */
    {
        rcvWndScale = 0;
    }
    if (mss != TM_16BIT_ZERO)
    {
        maxQueueBytes = tm_roundup(maxQueueBytes, (tt32Bit)mss);
        if ( tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_WND_SCALE) )
        {
#ifdef TM_USE_SOCKET_IMPORT
            if (userTcpConPtr != (ttUserTcpConPtr)0)
            {
/* Clip receive window size at maximum since the receive window scale is
 * not adjustable--it must match the imported socket window scale. */
                if (maxQueueBytes > (TM_TCP_MAX_SCALED_WND << rcvWndScale))
                {
                    maxQueueBytes = (TM_TCP_MAX_SCALED_WND << rcvWndScale);
                }
            }
            else
#endif /* TM_USE_SOCKET_IMPORT */
            {
/* Compute the receive window scale, based on the round up window size */
                tm_tcp_rcv_wnd_scale(rcvWndScale, maxQueueBytes);
            }
        }
        else
        {
/*
 * If we cannot scale, cannot have a queue size bigger than the max scaled
 * window (which fits in 16 bits).
 */
            if (maxQueueBytes > TM_TCP_MAX_SCALED_WND)
            {
                maxQueueBytes = TM_TCP_MAX_SCALED_WND;
            }
        }
    }
/* Restriction:
 * If we have already sent our SYN, and window scale is in effect and,
 * changing the window size would change the receive scale window, we cannot
 * change the receive window (since we had sent our receive window scale
 * to the remote in our SYN). Otherwise we can go ahead and change it:
 */
    if (    tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags, TM_TCPF_WND_SCALE)
         || (tcpVectPtr->tcpsState < TM_TCPS_SYN_RECEIVED)
         || (rcvWndScale == tcpVectPtr->tcpsRcvWndScale) )
    {
/* Initialize receive window */
        tcpVectPtr->tcpsRcvWndScale = rcvWndScale;
        socketPtr->socMaxRecvQueueBytes = maxQueueBytes;
        tcpVectPtr->tcpsRcvWnd = maxQueueBytes;
        if ( tm_16bit_one_bit_set( tcpVectPtr->tcpsFlags3,
                                   TM_TCPF3_LISTEN_RECV_WINDOW ) )
        {
/*
 * User modified recv window for sockets that have not been accepted yet.
 * Note that TM_TCPF3_LISTEN_RECV_WINDOW can only be set on a server. So
 * we could only be here when called from a new connection request comes in
 * (via tfTcpIncomingPacket, tfTcpConReq, tfTcpRcvSynOptions), i.e. with
 * tcpsState == 0, i.e. tcpsState < TM_TCPS_SYN_RECEIVED.
 */
/* Save the real recv window */
            maxQueueBytes = socketPtr->socMaxRecvQueueBytes;
            if (tcpVectPtr->tcpsMaxRecvQueueBytes < maxQueueBytes)
            {
/*
 * If it will not cause us to shrink the recv window later on, use pre-accept
 * recv window set by the user.
 */
                socketPtr->socMaxRecvQueueBytes =
                                            tcpVectPtr->tcpsMaxRecvQueueBytes;
            }
/*
 * tcpsMaxRecvQueueBytes now contains the real recv window size that will
 * be restored on the socket once accept is called.
 */
            tcpVectPtr->tcpsMaxRecvQueueBytes = maxQueueBytes;
        }
    }
outTcpSetMssWnd:
    return errorCode;
}

/*
 * tfTcpSetMss Function Description:
 * 1. We get the MTU from the route
 * 2. We take the smallest of our MSS, the peer MSS, and maximum send queue
 *
 * Parameters
 * tcpVectPtr    Pointer to TCP state vector entry/socket entry
 *
 * Return value:
 * 0 on success
 * TM_EHOSTUNREACH if no route to host.
 *
 * This function is called either when
 * a. We are attempting to connect (sending a SYN request) (tfTcpSendSyn())
 *    (state == CLOSED (we have postponed the state transition until
 *    after this routine).
 * b. We have received a SYN-ACK (peer sending a SYN and acking our SYN
 *    request) (TM_TCPM_RCV_SYN_ACK) (tfTcpRcvSynOptions())
 *    (state == ESTABLISHED)
 * c. We have received a simultaneous SYN from the remote
 *    (TM_TCPM_SIMULT_OPEN) (tfTcpRcvSynOptions()) (state == SYN_RECEIVED)
 * d. We have received a first SYN from the remote (peer sending a
 *    connection request) (TM_TCPM_CON_REQUEST) (tfTcpRcvSynOptions())
 *    (state == LISTEN (state transition postponed until after
 *    tfTcpRcvSynOptions() returns successfully to make sure we have a
 *    routing entry to the remote)
 * Note that a SYN has been sent by us in cases b. and c., but a SYN
 * has not yet been sent by us in cases a. and d.
 *
 * e. We have received an ICMP message size error, because we have
 *    attempted path MTU discovery, and our message was too big.
 * f. We could not get a recycled TCP header.
 */
static int tfTcpSetMss ( ttTcpVectPtr   tcpVectPtr )
{
    ttRteEntryPtr       rtePtr;
    ttSocketEntryPtr    socketPtr;
#ifdef TM_USE_IPSEC
    ttSASockHashListPtr salistPtr;
#endif /* TM_USE_IPSEC*/
    tt32Bit             maxQueueBytes;
#ifdef TM_USE_IPSEC
    unsigned int        sock;
#endif /* TM_USE_IPSEC*/
    int                 errorCode;
    tt16Bit             mss;
    tt16Bit             peerMss;
    tt16Bit             headerSize;
#ifdef TM_USE_IPSEC
    tt16Bit             ipsecOverHead;
#endif /* TM_USE_IPSEC*/
#ifdef TM_USE_SOCKET_IMPORT
    ttUserTcpConPtr     userTcpConPtr;
#endif /* TM_USE_SOCKET_IMPORT */

    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
/*
 * If user did not overwrite the default compute the MTU from the route,
 * (otherwise just use the MSS from the user even if it is bigger than the
 *  MTU, since this is what the user wants.)
 */
    errorCode = TM_ENOERROR;
/*
 * Rte cache structure not set, when we receive a first SYN from the remote
 * (case #d above), or when we receive a path MTU error message (case #e
 * above).
 */
    errorCode = tfTcpSetRtc(tcpVectPtr);
    if (errorCode != TM_ENOERROR)
    {
        goto outTcpSetMss;
    }
#ifdef TM_USE_SOCKET_IMPORT
/* User provides MSS for imported sockets created in established state */
    userTcpConPtr = tcpVectPtr->tcpsUserTcpConPtr;
    if (userTcpConPtr != (ttUserTcpConPtr)0)
    {
        mss = userTcpConPtr->uconMss;
    }
    else
#endif /* TM_USE_SOCKET_IMPORT */
    {
        mss = (tt16Bit)0;
        if (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags,
                                                TM_TCPF_PMTU_DISCOVERY))
        {
/* User set the MSS */
            mss = tcpVectPtr->tcpsMSS;
        }
    }
    if (mss < TM_TCP_MIN_MSS)
    {
/* Compute the MSS from the route IP MTU */
        rtePtr = socketPtr->socRteCacheStruct.rtcRtePtr;
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
        if (tcpVectPtr->tcpsNetworkLayer == TM_NETWORK_IPV6_LAYER)
#endif /* dual */
        {
#if (defined(TM_6_USE_MIP_MN) || defined(TM_6_USE_MIP_CN) || defined(TM_USE_IPSEC))
/* TM_PAK_IP_DEF_MAX_HDRL_LEN takes into accounts tunnels */
            headerSize = (tt16Bit)tm_byte_count(
                TM_PAK_IP_DEF_MAX_HDR_LEN + TM_PAK_TCP_HDR_LEN);
#else /* !MIP && !IPSEC */
            headerSize = (tt16Bit)tm_byte_count(
                TM_6PAK_IP_MIN_HDR_LEN + TM_PAK_TCP_HDR_LEN);
#ifdef TM_USE_IPDUAL
            if (tm_context(tv6Ipv4DefGwTunnelPtr) != (ttDeviceEntryPtr)0)
/* Take into account IPv6 over IPv4 tunnel */
            {
                headerSize = (tt16Bit)
                              ( headerSize +
                                (tt16Bit)tm_byte_count(
                                  TM_4PAK_IP_MIN_HDR_LEN)
                              );
            }
#endif /* TM_USE_IPDUAL */
#endif /* !MIP && !IPSEC */
        }
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
        else
#endif /* dual */
        {
             headerSize = (tt16Bit)tm_byte_count(
                                TM_4PAK_IP_MIN_HDR_LEN + TM_PAK_TCP_HDR_LEN);
        }
#endif /* TM_USE_IPV4 */
/* Need to protect the route MTU, since it could be changed dynamically
   due to an IPv6 router advertisement. */
#ifdef TM_USE_IPV6
        tm_kernel_set_critical;
#endif /* TM_USE_IPV6 */
        mss = (tt16Bit)rtePtr->rteMtu;
#ifdef TM_USE_IPV6
        tm_kernel_release_critical;
#endif /* TM_USE_IPV6 */

        mss = (tt16Bit)(mss - headerSize);
        if (mss < TM_TCP_MIN_MSS)
        {
            mss = TM_TCP_MIN_MSS;
        }
    }
/* Take the smallest of our MSS and the peer MSS (peer MSS from peer SYN) */
    peerMss = tcpVectPtr->tcpsPeerMss;
    if (    (peerMss != TM_16BIT_ZERO)
         && (peerMss < mss) )
    {
        mss = peerMss;
/*
 * Protect ourselves from MSS that are smaller than the maximum TCP header
 * size (from the remote)
 */
        if (mss < TM_TCP_MIN_MSS)
        {
            mss = TM_TCP_MIN_MSS;
        }
    }
    tcpVectPtr->tcpsMSS = mss;
/*
 * Compute the max amount of data in a segment, i.e we subtract from
 * the mss the length of the options that we know will be set in every
 * segment (Time stamp qualifies, SEL ACK does not)).
 */
    if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_TS))
    {
        mss = (tt16Bit)(tcpVectPtr->tcpsMSS - TM_TCPO_TS_NOOP_LENGTH);
    }
    else
    {
        mss = tcpVectPtr->tcpsMSS;
    }
/* EffSndMss (MSS without option) cannot be bigger than send queue size */
    maxQueueBytes = socketPtr->socMaxSendQueueBytes;
    if (maxQueueBytes < (tt32Bit)mss)
    {
        mss = (tt16Bit)maxQueueBytes;
    }
    tcpVectPtr->tcpsEffSndMss = mss;
    if (mss != TM_16BIT_ZERO)
    {
        tcpVectPtr->tcpsMaxRecycleHdrCount = (tt16Bit)
                (tt32Bit)(socketPtr->socMaxSendQueueBytes / (tt32Bit)mss);
    }
    else
    {
        tcpVectPtr->tcpsMaxRecycleHdrCount = TM_16BIT_ZERO;
    }
#ifdef TM_USE_IPSEC
    tm_call_lock_wait(&(tm_context(tvIpsecLockEntry)));
    if (tm_context(tvIpsecPtr))
    {

        sock = tcpVectPtr->tcpsSocketEntry.socIndex;

        if (sock != TM_SOC_NO_INDEX)
        {
            salistPtr = ((ttIpsecEntryPtr)(tm_context(tvIpsecPtr)))->
                ipsecSocketCache[sock % TM_IPSEC_SOCKSA_CACHE_SIZE];
            if( salistPtr != (ttSASockHashList*)0 )
            {
                while (    (salistPtr != (ttSASockHashListPtr)0)
                        && (salistPtr->sshSock != sock) )
                {
                    salistPtr = salistPtr->sshHashNextPtr;
                }
                if(salistPtr != (ttSASockHashListPtr)0)
                {
                    ipsecOverHead = (tt16Bit)
                        (salistPtr->sshSadbPtr->sadbPolicyPtr->plcyHdrLen +
                         salistPtr->sshSadbPtr->sadbPolicyPtr->plcyTrailerLen);
                    if (tcpVectPtr->tcpsEffSndMss > ipsecOverHead)
                    {
                        tcpVectPtr->tcpsEffSndMss = (tt16Bit)
                            (tcpVectPtr->tcpsEffSndMss - ipsecOverHead);
                    }
                    else
                    {
                        tcpVectPtr->tcpsEffSndMss = TM_16BIT_ZERO;
                    }
                }
            }
        }
    }
    tm_call_unlock(&(tm_context(tvIpsecLockEntry)));
#endif /* TM_USE_IPSEC */
#ifdef TM_DEV_SEND_OFFLOAD
    tfTcpOffloadInit(tcpVectPtr);
#endif /* TM_DEV_SEND_OFFLOAD */

outTcpSetMss:
    return errorCode;
}

/*
 * Get routing entry corresponding to peer address
 */
static int tfTcpSetRtc (ttTcpVectPtr tcpVectPtr)
{
    ttRteEntryPtr     rtePtr;
    ttRtCacheEntryPtr rtcPtr;
    ttSocketEntryPtr  socketPtr;
#ifdef TM_USE_IPV6
    tt6IpAddressPtr   ip6DestAddrPtr;
#endif /* TM_USE_IPV6 */
    int               errorCode;

    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    rtcPtr = &socketPtr->socRteCacheStruct;
    rtePtr = rtcPtr->rtcRtePtr;
    if (    (rtePtr != TM_RTE_NULL_PTR)
         && (tm_16bit_bits_not_set(rtePtr->rteFlags2, TM_RTE2_UP)) )
    {
/*
 * Current route is no longer valid, release ownership of the
 * corresponding routing entry (empty cache).
 */
        tfRtCacheUnGet(rtcPtr);
    }
    if (rtcPtr->rtcRtePtr == TM_RTE_NULL_PTR)
    {
#ifdef TM_USE_IPV6
        if (tcpVectPtr->tcpsSockAddrUPtr != (ttSockAddrPtrUnionPtr)0)
        {
/*
 *  Use passed parameters, if socket not already loaded from a previous
 *  connect
 */
            ip6DestAddrPtr = tcpVectPtr->tcpsSockAddrPeer6Ptr;
        }
        else
        {
            ip6DestAddrPtr = &(socketPtr->socPeerIpAddress);
        }
        tm_6_ip_copy_dststruct(ip6DestAddrPtr, rtcPtr->rtcDestIpAddr);
#else /* ! TM_USE_IPV6 */
        if (tcpVectPtr->tcpsSockAddrUPtr != (ttSockAddrPtrUnionPtr)0)
        {
/* Copy from user area */
            tm_ip_copy((tt4IpAddress)
                  (tcpVectPtr->tcpsSockAddrInPtr->sin_addr.s_addr),
                  rtcPtr->rtcDestIpAddr );
        }
        else
        {
            tm_ip_copy(socketPtr->socPeerIpAddress,
                       rtcPtr->rtcDestIpAddr);
        }
#endif /* ! TM_USE_IPV6 */
        errorCode = tfRtGet(rtcPtr);
        if (errorCode != TM_ENOERROR)
        {
/* Not a valid entry */
            goto outTcpSetRtc;
        }
/* Valid routing entry */
        rtePtr = rtcPtr->rtcRtePtr;
#ifdef TM_USE_IPDUAL
#ifdef TM_USE_IPSEC
        if (rtePtr->rteDevPtr == tm_context(tv6Ipv4DefGwTunnelPtr))
        {
/* Do not allow recycling of tcp headers */
            tm_16bit_clr_bit( tcpVectPtr->tcpsFlags,
                              (TM_TCPF_RECYCLE_HDR | TM_TCPF_SOCKETENTRYPTR));
        }
#endif /* TM_USE_IPSEC */
#endif /* TM_USE_IPDUAL */
        if ( tm_16bit_one_bit_set(rtePtr->rteFlags, TM_RTE_INDIRECT) )
        {
            tcpVectPtr->tcpsFlags |= TM_TCPF_INDIRECT;
        }
#ifdef TM_DEV_SEND_OFFLOAD
        tfTcpOffloadInit(tcpVectPtr);
#endif /* TM_DEV_SEND_OFFLOAD */
    }
    else
    {
        errorCode = TM_ENOERROR;
    }
outTcpSetRtc:
    return errorCode;
}

/*
 * Get an initial ISS for a new connection
 * Compute TCP ISN based on the following formula:
 * ISN = M + R(t) + F(sip, sport, dip, dport)
 * Where M is the increasing tvTcpIss, R(t) is a random number, and
 * F(..) is a function of source IP address, source IP port, destination
 * IP address, and destination IP port.
 */
static void tfTcpGetIss (ttTcpVectPtr tcpVectPtr)
{
#ifdef TM_USE_TCP_128BIT_RANDOM_ISS
    ttMd5Ctx        mdContext; /* 88 bytes */
    tt32Bit         outDigest[4];
#else /* !TM_USE_TCP_128BIT_RANDOM_ISS */
    tt32BitPtr      tuplePtr;
    tt32Bit         iss;
#endif /* !TM_USE_TCP_128BIT_RANDOM_ISS */
    unsigned int    i;

    if (tm_context(tvTcpSecretInit) == TM_8BIT_NO)
    {
#ifdef TM_USE_TCP_128BIT_RANDOM_ISS
/* Initialize 128-bit random number R(t) */
        for (i = 0; i < (sizeof(tm_context(tvTcpSecret))/sizeof(tt32Bit)); i++)
        {
            tm_context(tvTcpSecret)[i] = tfGetRandom();
        }
#ifndef TM_OPTIMIZE_SIZE
/* Copy random number in the corresponding MD5 context */
        tfMd5Init(&tm_context(tvTcpSecretMdContext));
        tfMd5Update(&tm_context(tvTcpSecretMdContext),
                    (tt8BitPtr)(ttVoidPtr)tm_context(tvTcpSecret),
                    (ttPktLen)sizeof(tm_context(tvTcpSecret)),
                    (ttPktLen)0);
#endif /* TM_OPTIMIZE_SIZE */
#else /* !TM_USE_TCP_128BIT_RANDOM_ISS */
        tm_context(tvTcpSecretIss) = tfGetRandom();
#endif /* !TM_USE_TCP_128BIT_RANDOM_ISS */
        tm_context(tvTcpSecretInit) = TM_8BIT_YES;
    }
#ifdef TM_USE_TCP_128BIT_RANDOM_ISS
#ifdef TM_OPTIMIZE_SIZE
/* Copy random number in the local MD5 context */
    tfMd5Init(&mdContext);
    tfMd5Update(&mdContext,
                (tt8BitPtr)(ttVoidPtr)tm_context(tvTcpSecret),
                (ttPktLen)sizeof(tm_context(tvTcpSecret)),
                (ttPktLen)0);
#else /* !TM_OPTIMIZE_SIZE */
/* Copy random number MD5 context in local variable */
    mdContext = tm_context(tvTcpSecretMdContext);
#endif /* TM_OPTIMIZE_SIZE */
/* Update local MD5 context using F(sip, sport, dip, dport) */
    tfMd5Update((ttVoidPtr)&mdContext,
                (tt8BitPtr)(ttVoidPtr)&(tcpVectPtr->tcpsSocketEntry.socTuple),
                (ttPktLen)sizeof(ttSockTuple),
                (ttPktLen)0);
/* Hash R(t) + F(sip, sport, dip, dport) in local outDigest */
    tfMd5Final((tt8BitPtr)outDigest, (ttVoidPtr)&mdContext);
#else /* !TM_USE_TCP_128BIT_RANDOM_ISS */
/* Random number + F(sip, sport, dip, dport) in local iss */
    tuplePtr = (tt32BitPtr)(ttVoidPtr)&(tcpVectPtr->tcpsSocketEntry.socTuple),
    iss = tm_context(tvTcpSecretIss);
    for (i = 0; i < sizeof(ttSockTuple)/sizeof(tt32Bit); i++)
    {
        iss += *(tuplePtr);
        tuplePtr++;
    }
#endif /* !TM_USE_TCP_128BIT_RANDOM_ISS */
/* Increase global TCP ISS */
/* Increase tvTcpIss by 250 every millisecond (i.e 1 every 4 us)
 * tvTime - tvTcpTimerTime is time elapsed since last call to this function
 */
    tm_kernel_set_critical;
    tm_context(tvTcpIss) += (tvTime - tm_context(tvTcpTimerTime)) *
                                                            TM_TCP_ISS_INCR;
/* Make sure we add at least one tick length */
    tm_context(tvTcpIss) += (TM_TCP_ISS_INCR * (tvTimerTickLength));
/* Update tvTcpTimerTime */
    tm_context(tvTcpTimerTime) = tvTime;
    tm_kernel_release_critical;
/*
 * New ISS is global ISS + hashed random number and F(sip, sport, dip, dport)
 * M + (R(t) + F(sip, sport, dip, dport))
 */
#ifdef TM_USE_TCP_128BIT_RANDOM_ISS
    tcpVectPtr->tcpsIss = tm_context(tvTcpIss) + outDigest[0];
#else /* !TM_USE_TCP_128BIT_RANDOM_ISS */
/*
 * New ISS is global ISS + random number + F(sip, sport, dip, dport)
 * M + (R(t) + F(sip, sport, dip, dport))
 */
    tcpVectPtr->tcpsIss = tm_context(tvTcpIss) + iss;
#endif /* !TM_USE_TCP_128BIT_RANDOM_ISS */
    return;
}

/*
 * Initialize connection send variables and establishment timers
 */
static void tfTcpInitConTmr (ttTcpVectPtr tcpVectPtr)
{
    ttSocketEntryPtr    socketPtr;
    tt32Bit             tcpMinRtoTime;
    tt32Bit             tcpMaxRtoTime;
    tt32Bit             currentTime;
    tt32Bit             rto;
    ttGenericUnion      timerParm1;
#ifdef TM_USE_TCP_REXMIT_CONTROL
    tt8Bit              timerMode;
#endif /* TM_USE_TCP_REXMIT_CONTROL */
#ifdef TM_USE_SOCKET_IMPORT
    ttUserTcpConPtr     userTcpConPtr;
#endif /* TM_USE_SOCKET_IMPORT */

#ifdef TM_USE_TCP_REXMIT_CONTROL
        if (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags3,
                                  TM_TCPF3_REXMIT_PAUSE))
        {
            timerMode = TM_TIM_AUTO;
        }
        else
        {
            timerMode = TM_TIM_SUSPENDED | TM_TIM_AUTO;
        }
#endif /* TM_USE_TCP_REXMIT_CONTROL */

/* Initialize send variables. Make sndNxt point to next sequence number */
    tm_tcp_init_send_sqn(tcpVectPtr);
#ifdef TM_USE_SOCKET_IMPORT
/* If this socket is being created from an imported socket, replace the
 * TCP send state information with the values from the imported socket. */
    userTcpConPtr = tcpVectPtr->tcpsUserTcpConPtr;
    if (userTcpConPtr != (ttUserTcpConPtr)0)
    {
        tcpVectPtr->tcpsSndUp = userTcpConPtr->uconSndUna - 1;
        tcpVectPtr->tcpsSndUna = userTcpConPtr->uconSndUna;
        tcpVectPtr->tcpsSndNxt = userTcpConPtr->uconSndUna;
        tcpVectPtr->tcpsMaxSndNxt = userTcpConPtr->uconSndUna;
    }
#endif /* TM_USE_SOCKET_IMPORT */
    tm_kernel_set_critical;
    currentTime = tvTime;
    tm_kernel_release_critical;
/* Fresh congestion window time stamp */
    tcpVectPtr->tcpsCwndAge = currentTime;
/*
 * Scaled smoothed mean deviation estimator should be initialized
 * with default rto time (3s).
 */
    tcpVectPtr->tcpsScRttVar = (ttS32Bit)(tcpVectPtr->tcpsRtoDef);
/*
 * Retransmission timeout is Scaled Smooth round trip time / 8 + Scaled
 * smoothed mean deviation estimator
 */
    rto = tcpVectPtr->tcpsRtoDef;
    tcpMinRtoTime = tcpVectPtr->tcpsRtoMin;
    tcpMaxRtoTime = tcpVectPtr->tcpsRtoMax;
    tm_bound(rto, tcpMinRtoTime, tcpMaxRtoTime);
    tcpVectPtr->tcpsRto = rto;
    timerParm1.genVoidParmPtr = (ttVoidPtr)tcpVectPtr;
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    tm_socket_checkout(socketPtr);
    tcpVectPtr->tcpsReXmitTmPtr = tfTimerAddExt( &tcpVectPtr->tcpsReXmitTm,
                                                 tfTcpTmReXmit,
                                                 tfTcpTmrCleanup,
                                                 timerParm1,
                                                 timerParm1, /* unused */
                                                 tcpVectPtr->tcpsRto,
#ifdef TM_USE_TCP_REXMIT_CONTROL
                                                 timerMode
#else /* !TM_USE_TCP_REXMIT_CONTROL */
                                                 TM_TIM_AUTO
#endif /* TM_USE_TCP_REXMIT_CONTROL */
                                                 );
/* turn on retransmission */
    tcpVectPtr->tcpsFlags2 |= (TM_TCPF2_REXMIT_MODE);
    timerParm1.genVoidParmPtr = (ttVoidPtr) tcpVectPtr;
    tm_socket_checkout(socketPtr);
    tcpVectPtr->tcpsConnTmPtr = tfTimerAddExt( &tcpVectPtr->tcpsConnTm,
                                               tfTcpTmConnOut,
                                               tfTcpTmrCleanup,
                                               timerParm1,
                                               timerParm1, /* unused */
                                               tcpVectPtr->tcpsMaxRt,
#ifdef TM_USE_TCP_REXMIT_CONTROL
                                               timerMode
#else /* !TM_USE_TCP_REXMIT_CONTROL */
                                               TM_TIM_AUTO
#endif /* TM_USE_TCP_REXMIT_CONTROL */
                                               );
    return;
}

/*
 * Put a packet in the reassembly queue, and try and reassemble the
 * next in sequence receive packet if any to queue into the receive
 * queue.
 * If SEL Ack is enabled, insert a SEL ACK block corresponding to the new
 * segment at head of the SEL ACK blocks queue, collapsing with existing
 * intersecting SEL ACK blocks if needed. At the end of the routine, if
 * some segments are moved to the socket receive queue, remove the
 * corresponding SEL ACK block entries.
 */
static tt8Bit tfTcpReassemblePacket ( ttTcpVectPtr tcpVectPtr,
                                      ttPacketPtr  packetPtr )
{
    ttPacketPtr         nextPacketPtr;
    ttPacketPtr         prevPacketPtr;
    ttPacketPtr         dumpPacketPtr;
    ttSocketEntryPtr    socketPtr;
    ttSqn               segSeqNo;
    ttSqn               endSegSeqNo;
#ifdef TM_TCP_SACK
    ttSqn               leftEdgeSeqNo;
#endif /* TM_TCP_SACK */
    ttS32Bit            overlap;
    int                 errorCode;
    tt8Bit              tcpCtlBits;
    tt8Bit              needAck;
    tt8Bit              appended;

/*
 * Loop through all the packets already queued, to find the
 * right spot to insert this packet (between prevPacketPtr
 * and nextPacketPtr). Our beginning sequence number should be strictly
 * less than the nextPacket sequence number.
 */
    tcpCtlBits = TM_8BIT_ZERO;
    needAck = TM_8BIT_YES;
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
        nextPacketPtr = tcpVectPtr->tcpsReassemblePtr;
        prevPacketPtr = TM_PACKET_NULL_PTR;
/* Begin sequence number of our packet */
        segSeqNo = tm_tcp_hdr_ptr(packetPtr)->tcpSeqNo;
/* End sequence number of our packet (one past last) */
        endSegSeqNo = segSeqNo + packetPtr->pktChainDataLength;
#ifdef TM_TCP_SACK
/*
 * If SEL Ack is enabled, insert a SEL ACK block corresponding to the new
 * segment at head of the SEL ACK blocks queue, collapsing with existing
 * intersecting SEL ACK blocks if needed.
 */
        if ( tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_SEL_ACK) )
        {
            tfTcpSackQueueBlock( tcpVectPtr, segSeqNo, endSegSeqNo,
                                 TM_TCP_SACK_SEND_INDEX );
        }
#endif /* TM_TCP_SACK */
        while (nextPacketPtr != TM_PACKET_NULL_PTR)
        {
            if ( tm_sqn_lt(segSeqNo,
                           tm_tcp_hdr_ptr(nextPacketPtr)->tcpSeqNo) )
            {
                break;
            }
            prevPacketPtr = nextPacketPtr;
            nextPacketPtr = nextPacketPtr->pktChainNextPtr;
        }
/* Check for overlap of data between previous packet and our packet */
        if (prevPacketPtr != TM_PACKET_NULL_PTR)
        {
            overlap = (ttS32Bit)
                      (  tm_tcp_hdr_ptr(prevPacketPtr)->tcpSeqNo
                       + (tt32Bit)prevPacketPtr->pktChainDataLength
                       - segSeqNo);
/* If there are overlap bytes */
            if (overlap > TM_L(0))
            {
                if ( (tt32Bit)overlap >=
                                     (tt32Bit)packetPtr->pktChainDataLength )
/* All of our packet data are already included in previous packet */
                {
                    tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
                    goto tcpReassExit;
                }
                tfTcpPacketHeadTrim(packetPtr, (ttPktLen)overlap);
            }
            else
            {
                if (    (overlap == TM_L(0))
                     && tm_16bit_bits_not_set(
                        tcpVectPtr->tcpsFlags, TM_TCPF_PACKET)
                   )
/*
 * Contiguous packets. Check if we can append to the prev packet.
 */
                {
                    appended = tfTcpAppendPacket(packetPtr, prevPacketPtr);
                    if (appended)
                    {
/* packet appended to previous. Free it. We are done. */
                        tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
                        goto tcpReassExit;
                    }
                }
            }
        }
/* Check for overlap of data between our packet and next packet(s) */
        while (nextPacketPtr != TM_PACKET_NULL_PTR)
        {
/* overlap computation */
            overlap = (ttS32Bit)(   endSegSeqNo
                                 -  tm_tcp_hdr_ptr(nextPacketPtr)->tcpSeqNo );
            if (overlap < TM_L(0)) /* no overlap */
            {
                break;
            }
            else
            {
                if (    (overlap == TM_L(0))
                     && tm_16bit_bits_not_set(
                        tcpVectPtr->tcpsFlags, TM_TCPF_PACKET)
                   )
                {
/*
 * Contiguous packets. Check if we can append the next packet.
 */
                    appended = tfTcpAppendPacket(nextPacketPtr, packetPtr);
                    if (!appended)
                    {
                        break;
                    }
                    endSegSeqNo = segSeqNo + packetPtr->pktChainDataLength;
                }
                else
                {
                    if ( (tt32Bit)overlap <
                                  (tt32Bit)nextPacketPtr->pktChainDataLength )
                    {
                        tfTcpPacketHeadTrim(nextPacketPtr, (ttPktLen)overlap);
                        break;
                    }
                }
            }
/* overlap covers the next packet, or we copied it: remove it from the queue */
            dumpPacketPtr = nextPacketPtr;
            if (prevPacketPtr != TM_PACKET_NULL_PTR)
            {
                prevPacketPtr->pktChainNextPtr =
                                        nextPacketPtr->pktChainNextPtr;
            }
            else
            {
                tcpVectPtr->tcpsReassemblePtr =
                                        nextPacketPtr->pktChainNextPtr;
            }
/* Point to next packet */
            nextPacketPtr = nextPacketPtr->pktChainNextPtr;
/* Free the removed packet */
            tfFreePacket(dumpPacketPtr, TM_SOCKET_LOCKED);
        }
/* We have found the right spot. Insert the packet */
        packetPtr->pktChainNextPtr = nextPacketPtr;
        if (prevPacketPtr == TM_PACKET_NULL_PTR)
        {
/* at head of queue */
            tcpVectPtr->tcpsReassemblePtr = packetPtr;
        }
        else
        {
            prevPacketPtr->pktChainNextPtr = packetPtr;
        }
    }
/* Check if there is contiguous data to give to the user */
    nextPacketPtr = tcpVectPtr->tcpsReassemblePtr;
    prevPacketPtr = TM_PACKET_NULL_PTR;
/* If there is data in the queue, is it the data we expect? */
    if (    (nextPacketPtr == TM_PACKET_NULL_PTR)
         || tm_tcp_hdr_ptr(nextPacketPtr)->tcpSeqNo != tcpVectPtr->tcpsRcvNxt)
    {
        if ( packetPtr == (ttPacketPtr)0 )
/* Called from tfTcpEstablish() with no data */
        {
            needAck = TM_8BIT_ZERO;
        }
        goto tcpReassExit;
    }
/* At least some data in sequence order */
#ifdef TM_TCP_SACK
    leftEdgeSeqNo = tcpVectPtr->tcpsRcvNxt;
#endif /* TM_TCP_SACK */
    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    do
    {
        tcpVectPtr->tcpsRcvNxt += nextPacketPtr->pktChainDataLength;
        tcpCtlBits = tm_tcp_hdr_ptr(nextPacketPtr)->tcpControlBits;
        prevPacketPtr = nextPacketPtr;
        nextPacketPtr = nextPacketPtr->pktChainNextPtr;
        tcpVectPtr->tcpsReassemblePtr = nextPacketPtr;
        prevPacketPtr->pktChainNextPtr = TM_PACKET_NULL_PTR;
/*
 * If urgent data is in this packet, and urgent data is not to be kept
 * in line
 */
        if (    tm_16bit_one_bit_set(
                                   prevPacketPtr->pktSharedDataPtr->dataFlags,
                                   TM_BUF_TCP_OOB_BYTE )
             && (tm_16bit_bits_not_set( socketPtr->socOptions,
                                        SO_OOBINLINE)) )
        {
/* Extract out of band byte, and queue rest of the data to the socket */
            errorCode = tfSocketOobIncomingPacket(tcpVectPtr, prevPacketPtr);
        }
        else
        {
            errorCode = tfSocketIncomingPacket(socketPtr, prevPacketPtr);
        }
        if (errorCode == TM_ENOERROR)
        {
            tcpVectPtr->tcpsSelectFlags |= TM_SELECT_READ;
            tcpVectPtr->tcpsSocketCBFlags |= TM_CB_RECV;
        }
    }
    while (   (nextPacketPtr != TM_PACKET_NULL_PTR)
           && (tm_tcp_hdr_ptr(nextPacketPtr)->tcpSeqNo ==
                                                    tcpVectPtr->tcpsRcvNxt));
#ifdef TM_TCP_SACK
    if ( tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_SEL_ACK) )
    {
        tfTcpSackDeQueueBlocks( tcpVectPtr,
                                leftEdgeSeqNo,
                                tcpVectPtr->tcpsRcvNxt,
                                TM_TCP_SACK_SEND_INDEX );
    }
#endif /* TM_TCP_SACK */
tcpReassExit:
    if (needAck != TM_8BIT_ZERO)
    {
        tcpVectPtr->tcpsFlags2 |= TM_TCPF2_ACK; /* ACK now */
    }
    return tcpCtlBits;
}

#ifdef TM_ERROR_CHECKING
/*
 * Re-initialize an automatic timer with a new interval of call back
 */
void tfTcpTimerNewTime (ttTimerPtr timerPtr, tt32Bit newTime)
{
    if (timerPtr == TM_TMR_NULL_PTR)
    {
        tfKernelError("tfTcpTimerNewTime",
                      "Null timer pointer");
    }
    else
    {
        tm_timer_new_time(timerPtr, newTime);
    }
    return;
}
/*
 * Re-initialize an automatic timer with a new interval of call back
 */
void tfTcpTimerSuspend(ttTimerPtr timerPtr)
{
    if (timerPtr == TM_TMR_NULL_PTR)
    {
        tfKernelError("tfTcpTimerSuspend",
                      "Null timer pointer");
    }
    else
    {
        tm_timer_suspend(timerPtr);
    }
    return;
}

#ifdef TM_USE_TCP_REXMIT_CONTROL
/*
 * Resume a time that has been suspended
 */
static void tfTcpTimerResume(ttTimerPtr timerPtr)
{
    if (timerPtr == TM_TMR_NULL_PTR)
    {
        tfKernelError("tfTcpTimerResume",
                      "NULL timer pointer");
    }
    else
    {
        tm_timer_resume(timerPtr);
    }
    return;
}
#endif /* TM_USE_TCP_REXMIT_CONTROL */
#endif /* TM_ERROR_CHECKING */


/*
 * Receive Urgent Data
 */
static tt8Bit tfTcpRcvUrgentData ( ttTcpVectPtr tcpVectPtr,
                                   ttPacketPtr  packetPtr )
{
    ttTcpHeaderPtr      tcphPtr;
    ttSocketEntryPtr    socketPtr;
    ttSqn               segUp;
    tt8Bit              urgentData;

    tcphPtr = tm_tcp_hdr_ptr(packetPtr);
    urgentData = TM_8BIT_YES;
/*
 * The urgent pointer points to the sequence number of the LAST octet in a
 * sequence of urgent data (see RFC 1122).
 * TCP_STDURG TCP option need to be reset by the user if peer is Berkeley
 * since Berkeley systems erroneously set the urgent pointer to
 * one byte past the last byte of out of band data.
 */
    if (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags, TM_TCPF_STDURG))
    {
        if (tcphPtr->tcpUrgentPtr != (tt16Bit)0)
        {
            tcphPtr->tcpUrgentPtr--;
        }
#ifdef TM_ERROR_CHECKING
        else
        {
/*
 * Give a warning, if user think that remote side is berkeley, but remote
 * side behaves like RFC 1122.
 */
            tfKernelWarning("tfTcpRcvUrgentData",
                            "Peer follows RFC 1122, but user reset STD URG");
        }
#endif /* TM_ERROR_CHECKING */
    }
/*
 * Compute segUp:
 */
    segUp = tcphPtr->tcpSeqNo + tcphPtr->tcpUrgentPtr;
/*
 * RFC 793:
 *        If the URG bit is set, RCV.UP <- max(RCV.UP,SEG.UP), and signal
 *        the user that the remote side has urgent data if the urgent
 *        pointer (RCV.UP) is in advance of the data consumed.  If the
 *        user has already been signaled (or is still in the "urgent
 *        mode") for this continuous sequence of urgent data, do not
 *        signal the user again.
 *
 */
    if (tm_sqn_lt(tcpVectPtr->tcpsRcvUp, segUp))
    {
        socketPtr = &(tcpVectPtr->tcpsSocketEntry);
/* If new urgent data coming in */
        tcpVectPtr->tcpsRcvUp = segUp;
/*
 * Compute out of band mark (offset to last byte of urgent data)
 */
        socketPtr->socOobMark = socketPtr->socRecvQueueBytes
                                     + segUp
                                     - tcpVectPtr->tcpsRcvNxt;
        if (socketPtr->socOobMark == 0)
        {
/* If last byte of out of band data is at the head of the receive queue */
            socketPtr->socFlags |= TM_SOCF_RCVATMARK;
        }
        else
        {
            tm_16bit_clr_bit( socketPtr->socFlags, TM_SOCF_RCVATMARK );
        }
/* Notify user that out of band data is here or coming */
        tcpVectPtr->tcpsSelectFlags |= TM_SELECT_EXCEPTION;
        tcpVectPtr->tcpsSocketCBFlags |= TM_CB_RECV_OOB;
/*
 * Out of band data not copied yet, new out of band data for the user
 * to retrieve
 */
        tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2,
                          (TM_TCPF2_OOBDATA_IN | TM_TCPF2_OOBDATA_OUT) );
    }
    if (tcphPtr->tcpUrgentPtr < (tt16Bit)packetPtr->pktChainDataLength)
    {
/*
 * Urgent data in this packet.
 * Mark the packet, so that urgent data is extracted before the packet
 * is being queued to the user. Cannot extract it yet, in case we
 * have to go through tfTcpReassemblePacket(), so that we can keep
 * the number of bytes consistant with the sequence numbers, and also so
 * that we don't clobber bytes in the reassembly queue when data is
 * retransmitted from the peer and overlaps with data already queued in
 * the reassembly queue.
 */
        packetPtr->pktSharedDataPtr->dataFlags |= TM_BUF_TCP_OOB_BYTE;
    }
    return urgentData;
/*
 * RFC 1122 says that the urgent pointer points to the sequence number
 * of the last octet in a sequence of urgent data:
 *
 *         4.2.2.4  Urgent Pointer: RFC-793 Section 3.1
 *
 *            The second sentence is in error: the urgent pointer points
 *            to the sequence number of the LAST octet (not LAST+1) in a
 *            sequence of urgent data.  The description on page 56 (last
 *            sentence) is correct.
 *
 *            A TCP MUST support a sequence of urgent data of any length.
 *
 *            A TCP MUST inform the application layer asynchronously
 *            whenever it receives an Urgent pointer and there was
 *            previously no pending urgent data, or whenever the Urgent
 *            pointer advances in the data stream.  There MUST be a way
 *            for the application to learn how much urgent data remains to
 *            be read from the connection, or at least to determine
 *            whether or not more urgent data remains to be read.
 * End of RFC 1122
 */
}

/*
 * Process out of band incoming data
 */
static int tfSocketOobIncomingPacket ( ttTcpVectPtr tcpVectPtr,
                                       ttPacketPtr packetPtr )
{
    ttPacketPtr             nextPacketPtr;
    tt8BitPtr               dataPtr;
    ttPktLen                copyLength;
#ifndef TM_DSP
    int                     i;
#endif /* TM_DSP */
    int                     errorCode;
    tt16Bit                 urgentOffset;
    tt8Bit                  oobByte;

/* Pull out of the stream the one byte of urgent data */
    nextPacketPtr = packetPtr;
    urgentOffset = tm_tcp_hdr_ptr(packetPtr)->tcpUrgentPtr;
    do
    {
        if (nextPacketPtr->pktLinkDataLength > (ttPktLen)urgentOffset)
        {

#ifdef TM_DSP
            dataPtr = (tt8BitPtr)(   nextPacketPtr->pktLinkDataPtr
                                   + tm_packed_byte_len(urgentOffset) );
/* Check to see if the urgent pointer points to word aligned data */
            if ((urgentOffset & TM_DSP_ROUND_PTR) == 0) {

                oobByte =   ( *dataPtr >> ((TM_DSP_BYTES_PER_WORD-1) << 3) )
                          & 0xff;

            } else {
#ifdef TM_32BIT_DSP
                switch (urgentOffset & TM_DSP_ROUND_PTR) {
                    case 1:
                       oobByte = (*dataPtr >> 16) & 0xff;
                       break;

                    case 2:
                       oobByte = (*dataPtr >> 8) & 0xff;
                       break;

                    case 3:
                       oobByte = *dataPtr & 0xff;
                       break;
                }
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
                oobByte = *dataPtr & 0xff;
#endif /* TM_16BIT_DSP */

            }
            copyLength =   nextPacketPtr->pktLinkDataLength
                         - (ttPktLen)urgentOffset
                         - (ttPktLen)1;
            if (copyLength != (ttPktLen)0)
            {
                if((urgentOffset & TM_DSP_ROUND_PTR) == TM_DSP_ROUND_PTR)
                {
                    tfByteByByteCopy( (int*) dataPtr + 1,
                                      0,
                                      (int*) dataPtr,
                                      (TM_DSP_BYTES_PER_WORD - 1),
                                      copyLength );
                }
                else
                {
                    tfByteByByteCopy((int*)dataPtr,
                                      (urgentOffset & TM_DSP_ROUND_PTR) + 1,
                                      (int*) dataPtr,
                                      urgentOffset & TM_DSP_ROUND_PTR,
                                     copyLength);
                }
            }
#else /* !TM_DSP */
            dataPtr = (tt8BitPtr)(nextPacketPtr->pktLinkDataPtr +
                                  urgentOffset);
            oobByte = *dataPtr;
            copyLength =   nextPacketPtr->pktLinkDataLength
                         - (ttPktLen)urgentOffset
                         - (ttPktLen)1;
/* Avoid overlapping bcopy by copying up one byte at a time */
            for (i = 0; i < (int)copyLength; i++)
            {
                dataPtr[i] = dataPtr[i + 1];
            }
#endif /* !TM_DSP */
            nextPacketPtr->pktLinkDataLength--;
/* Out of band byte removed from the stream */
            packetPtr->pktChainDataLength--;
/* Copy out of band data in TCP state vector */
            tcpVectPtr->tcpsOobByte = oobByte;
            tcpVectPtr->tcpsFlags2 |= TM_TCPF2_OOBDATA_IN;
            break;
        }
        urgentOffset = (tt16Bit) ((tt16Bit)urgentOffset -
                                  (tt16Bit)nextPacketPtr->pktLinkDataLength);
        nextPacketPtr = (ttPacketPtr)nextPacketPtr->pktLinkNextPtr;
    }
    while (nextPacketPtr != TM_PACKET_NULL_PTR);
    if (packetPtr->pktChainDataLength == 0)
    {
        errorCode = TM_ENOBUFS;
        tfFreePacket(packetPtr, TM_SOCKET_LOCKED);
    }
    else
    {
        errorCode = tfSocketIncomingPacket(&(tcpVectPtr->tcpsSocketEntry),
                                           packetPtr);
    }
    return errorCode;
}

/*
 * Head trim a packet, preserving/updating sequence number, urgent offset,
 * and control bits
 */
static void tfTcpPacketHeadTrim (ttPacketPtr packetPtr,
                                 ttPktLen    overlap)
{
    ttTcpHeaderPtr tcphPtr;

    tfPacketHeadTrim(packetPtr, overlap);
    tcphPtr = tm_tcp_hdr_ptr(packetPtr);
    tcphPtr->tcpSeqNo = tcphPtr->tcpSeqNo + (tt32Bit)overlap;
    if ( tm_16bit_one_bit_set( packetPtr->pktSharedDataPtr->dataFlags,
                               TM_BUF_TCP_OOB_BYTE ) )
    {
        if ((ttPktLen)tcphPtr->tcpUrgentPtr >= overlap)
        {
            tcphPtr->tcpUrgentPtr = (tt16Bit)(   tcphPtr->tcpUrgentPtr
                                               - (tt16Bit)overlap );
        }
        else
        {
            tm_16bit_clr_bit( packetPtr->pktSharedDataPtr->dataFlags,
                              TM_BUF_TCP_OOB_BYTE );
        }
    }
    return;
}

/*
 * User receives out of band data
 */
int tfTcpRecvOobData (ttTcpVectPtr tcpVectPtr, char TM_FAR * bufferPtr,
                      int flags)
{
    ttSocketEntryPtr    socketPtr;
    int                 errorCode;

    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
/*
 * If there is out of band data:
 * If out of band data offset is non zero or we are at out of band mark, and
 * if out of band data is not in line and
 * if out of band data has not already been read
 */
    if (    (    (socketPtr->socOobMark != 0)
              || tm_16bit_one_bit_set(socketPtr->socFlags,
                                      TM_SOCF_RCVATMARK) )
         && tm_16bit_bits_not_set(socketPtr->socOptions, SO_OOBINLINE)
         && tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags2,
                                  TM_TCPF2_OOBDATA_OUT) )
    {
/*
 * There is out of band data. If out of band has been copied in the
 * state vector, give it to the user.
 */
        if ( tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags2,
                                  TM_TCPF2_OOBDATA_IN) )
        {

            *bufferPtr = (char)tcpVectPtr->tcpsOobByte;
            if ((flags & MSG_PEEK) == 0)
            {
/* Turn off OOBDATA_IN, turn on OOBDATA_OUT */
                tcpVectPtr->tcpsFlags2 ^=
                                     TM_TCPF2_OOBDATA_IN|TM_TCPF2_OOBDATA_OUT;
            }
            errorCode = TM_ENOERROR;
        }
        else
        {
/*
 * If out of band data has not been extracted yet from the stream, tell the
 * user it is coming soon
 */
            errorCode = TM_EWOULDBLOCK;
        }
    }
    else
/* else if there is no out of band data, return an error */
    {
        errorCode = TM_EINVAL;
    }
    return errorCode;
}

/*
 * Add a connection timeout timer if required by the user with the TCP_MAXRT
 * socket option.
 */
static void tfTcpAddMaxRtTmr (ttTcpVectPtr tcpVectPtr)
{
    ttSocketEntryPtr    socketPtr;
    ttGenericUnion      timerParm1;
#ifdef TM_USE_TCP_REXMIT_CONTROL
    tt8Bit              timerMode;
#endif /* TM_USE_TCP_REXMIT_CONTROL */

#ifdef TM_USE_TCP_REXMIT_CONTROL
    if (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags3,
                              TM_TCPF3_REXMIT_PAUSE))
    {
        timerMode = TM_TIM_AUTO;
    }
    else
    {
        timerMode = TM_TIM_SUSPENDED | TM_TIM_AUTO;
    }
#endif /* TM_USE_TCP_REXMIT_CONTROL */

    if (    (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_MAXRT))
         && (tcpVectPtr->tcpsReXmitCnt == 1) )
    {
#ifdef TM_USE_TCP_REXMIT_CONTROL
/* if we recently reset, we don't need to mess with the ConnTm */
        if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags3,
                                 TM_TCPF3_REXMIT_RESET))
        {
            tcpVectPtr->tcpsFlags3 &= ~TM_TCPF3_REXMIT_RESET;
        }
        else
#endif /* TM_USE_TCP_REXMIT_CONTROL */
        {
            if (tcpVectPtr->tcpsConnTmPtr == TM_TMR_NULL_PTR)
            {
                timerParm1.genVoidParmPtr = (ttVoidPtr)tcpVectPtr;
                socketPtr = &(tcpVectPtr->tcpsSocketEntry);
                tm_socket_checkout(socketPtr);
                tcpVectPtr->tcpsConnTmPtr = tfTimerAddExt(
                                                    &tcpVectPtr->tcpsConnTm,
                                                    tfTcpTmConnOut,
                                                    tfTcpTmrCleanup,
                                                    timerParm1,
                                                    timerParm1, /* unused */
                                                    tcpVectPtr->tcpsMaxRt,
#ifdef TM_USE_TCP_REXMIT_CONTROL
                                                    timerMode
#else /* !TM_USE_TCP_REXMIT_CONTROL */
                                                    TM_TIM_AUTO
#endif /* TM_USE_TCP_REXMIT_CONTROL */
                                                         );

            }
            else
            {
                tm_timer_new_time(tcpVectPtr->tcpsConnTmPtr, tcpVectPtr->tcpsMaxRt);
            }
        }
    }
    return;
}

/*
 * Remove a connection request from the listening queue of non accepted
 * connection requests (either half connected (index == 0), or fully
 * connected (index == 1).
 * Always called with the listening socket locked, and with socketPtr locked.
 * Called from accept(), or tfTcpVectClose(), or tfTcpRemoveResetConReq().
 */
void tfTcpRemoveConReq (ttTcpVectPtr tcpVectPtr, int index, int closing)
{
    ttSocketEntryPtr listenSocketPtr;
    ttTcpVectPtr     listenTcpVectPtr;
    ttSocketEntryPtr socketPtr;
    unsigned int     count;
    tt8Bit           removed;

    removed = tfTcpConQueueRemove(tcpVectPtr, index);
    if (removed)
    {
        socketPtr = &(tcpVectPtr->tcpsSocketEntry);
/* Back pointer to listening socket */
        listenSocketPtr = socketPtr->socListenSocketPtr;
/* Reset back pointer to listen socket */
        socketPtr->socListenSocketPtr = (ttSocketEntryPtr)0;
#ifdef TM_ERROR_CHECKING
        if (listenSocketPtr == (ttSocketEntryPtr)0)
        {
            tfKernelError(
              "tfTcpRemoveConReq",
              "Corruped connection request back pointer to listening socket");
        }
        else
#endif /* TM_ERROR_CHECKING */
        {
/* Allow more incoming connections */
            listenSocketPtr->socBackLog--;
            if (index == TM_TCP_SYN_QUEUE)
            {
                listenTcpVectPtr = (ttTcpVectPtr)(ttVoidPtr)listenSocketPtr;
                listenTcpVectPtr->tcpsSynFloodCount--;
            }
            count = tcpVectPtr->tcpsThreadCount;
            if (count >= 1)
            {
/*
 * If more than one thread has ownership via tcpsThreadCount,
 * increase the listening socket ownership count. If only one thread just
 * transfer the back pointer listening socket ownership count to the thread,
 * so do not increase or decrease it.
 * Explanation: this is to prevent a closed listening socket from
 * disappearing while we are processing a connection request establishment,
 * or closing a connection request.
 */
                if (count > 1)
                {
                    listenSocketPtr->socOwnerCount = (unsigned int)
                                    (   listenSocketPtr->socOwnerCount
                                      + (unsigned int)(count - 1) );
                }
            }
            else
            {
/*
 * No thread has ownership. listening socket ownership count down since we
 * just reset the pointer.
 */
                listenSocketPtr->socOwnerCount--;
            }
        }
/* We are closing a connection request, decrease number of open connections */
        if (closing)
        {
            tfSocketDecreaseOpenSockets();
        }
    }
#ifdef TM_ERROR_CHECKING
    else
    {
        tfKernelError( "tfTcpRemoveConReq",
                       "Corruped connection request entry");
    }
#endif /* TM_ERROR_CHECKING */
    return;
}

/*
 * Remove entry from the half connected queue, send a reset, and delete it.
 */
static void tfTcpRemoveResetConReq(ttTcpVectPtr conReqTcpVectPtr, int index)
{
    ttSocketEntryPtr        conReqSocketEntryPtr;
    ttSocketEntryPtr        listenSocketPtr;

    conReqSocketEntryPtr = &(conReqTcpVectPtr->tcpsSocketEntry);
    tm_call_lock_wait(&conReqSocketEntryPtr->socLockEntry);
/* Check out to get ownership */
    tm_socket_checkout(conReqSocketEntryPtr);
    listenSocketPtr = conReqSocketEntryPtr->socListenSocketPtr;
/*
 * Remove entry from the current (half connected, or ready to be accepted)
 * queue. Zero back pointer to listening socket.
 */
    tfTcpRemoveConReq(conReqTcpVectPtr, index, 1);
/* Unlock the listening socket */
    if (listenSocketPtr != (ttSocketEntryPtr)0)
    {
        tm_call_unlock(&listenSocketPtr->socLockEntry);
    }
/* Socket has not been accepted yet (not in the array) */
    (void)tfTcpAbort( conReqTcpVectPtr, 0 ); /* no user to notify */
    tm_socket_checkin_call_unlock(conReqSocketEntryPtr);
/* Relock the listening socket */
    if (listenSocketPtr != (ttSocketEntryPtr)0)
    {
        tm_call_lock_wait(&listenSocketPtr->socLockEntry);
    }
    return;
}

/*
 * Insert TCP vector in doubly linked connection list (either queue of
 * half connected TCP vectors (queueIndex == 0), or queue of fully connected,
 * but not yet accepted TCP vectors (queueIndex == 1).
 */
static void tfTcpConQueueInsert(ttTcpVectPtr listenTcpVectPtr,
                                ttTcpVectPtr tcpVectPtr,
                                int          index)
{
    tcpVectPtr->tcpsConQueue[index].tcpConReqNextPtr = listenTcpVectPtr;
    tcpVectPtr->tcpsConQueue[index].tcpConReqPrevPtr =
                       listenTcpVectPtr->tcpsConQueue[index].tcpConReqPrevPtr;
    listenTcpVectPtr->tcpsConQueue[index].tcpConReqPrevPtr->
                       tcpsConQueue[index].tcpConReqNextPtr = tcpVectPtr;
    listenTcpVectPtr->tcpsConQueue[index].tcpConReqPrevPtr = tcpVectPtr;
    return;
}

/*
 * Remove TCP vector from doubly linked connection list (either queue of
 * half connected TCP vectors (queueIndex == 0), or queue of fully connected,
 * but not yet accepted TCP vectors (queueIndex == 1).
 */
static tt8Bit tfTcpConQueueRemove(ttTcpVectPtr tcpVectPtr, int index)
{
    ttTcpVectPtr     prevTcpVectPtr;
    ttTcpVectPtr     nextTcpVectPtr;
    tt8Bit           removed;

    prevTcpVectPtr = tcpVectPtr->tcpsConQueue[index].tcpConReqPrevPtr;
    if (prevTcpVectPtr != tcpVectPtr)
    {
/* If in the list */
        nextTcpVectPtr = tcpVectPtr->tcpsConQueue[index].tcpConReqNextPtr;
#ifdef TM_ERROR_CHECKING
        if (nextTcpVectPtr == tcpVectPtr)
        {
            tfKernelError( "tfTcpRemoveConReq",
                           "Corruped connection request entry");
        }
#endif /* TM_ERROR_CHECKING */
        prevTcpVectPtr->tcpsConQueue[index].tcpConReqNextPtr = nextTcpVectPtr;
        nextTcpVectPtr->tcpsConQueue[index].tcpConReqPrevPtr = prevTcpVectPtr;
/* Out of the list */
        tcpVectPtr->tcpsConQueue[index].tcpConReqPrevPtr = tcpVectPtr;
        tcpVectPtr->tcpsConQueue[index].tcpConReqNextPtr = tcpVectPtr;
        removed = TM_8BIT_YES;
    }
    else
    {
        removed = TM_8BIT_NO;
    }
    return removed;
}

/*
 * tfTcpAppendPacket()
 * if prev, and current packet not fragmented, check if we can append to
 * the prev packet. Update urgent pointer if needed.
 */
static tt8Bit tfTcpAppendPacket(ttPacketPtr packetPtr,
                                ttPacketPtr prevPacketPtr)
{
    ttTcpHeaderPtr      tcphPtr;
    ttTcpHeaderPtr      prevTcphPtr;
    ttPktLen            extraAllocatedBytes;
#ifdef TM_DSP
    unsigned int        destOffset;
#endif /* TM_DSP */
    tt8Bit              appended;

    appended = TM_8BIT_NO;
    if ( (    prevPacketPtr->pktChainDataLength
           == prevPacketPtr->pktLinkDataLength) &&
         (    packetPtr->pktChainDataLength
           == packetPtr->pktLinkDataLength)
       )
/*
 * Contiguous packets, prev, and current packet not fragmented.
 * Check if we can append to the prev packet.
 */
    {
/* compute room left at the end previous packet */
        extraAllocatedBytes = (ttPktLen)
                        (tm_byte_count(
                            prevPacketPtr->pktSharedDataPtr->dataBufLastPtr
                            - prevPacketPtr->pktLinkDataPtr ));
        extraAllocatedBytes = (ttPktLen)
                        (   (extraAllocatedBytes
                             - prevPacketPtr->pktLinkDataLength)
#ifdef TM_DSP
                             - prevPacketPtr->pktLinkDataByteOffset
#endif /* TM_DSP */
                            );
        if (extraAllocatedBytes >= packetPtr->pktChainDataLength)
        {
/* All of our packet data can now be included in the previous packet */
#ifdef TM_DSP
            destOffset = (prevPacketPtr->pktLinkDataLength
                          + prevPacketPtr->pktLinkDataByteOffset);
            tfMemCopyOffset( (int *) packetPtr->pktLinkDataPtr,
                             packetPtr->pktLinkDataByteOffset,
/* destination word pointer for copy */
                            (int *) (prevPacketPtr->pktLinkDataPtr
                                     + tm_packed_byte_len(destOffset)),
/* destination 8-bit byte offset for copy */
                            destOffset % TM_DSP_BYTES_PER_WORD,
                            packetPtr->pktLinkDataLength );
#else /* ! TM_DSP */
            tm_bcopy( packetPtr->pktLinkDataPtr,
                        prevPacketPtr->pktLinkDataPtr
                      + prevPacketPtr->pktLinkDataLength,
                      (unsigned)packetPtr->pktLinkDataLength);
#endif /* ! TM_DSP */
            tcphPtr = tm_tcp_hdr_ptr(packetPtr);
            prevTcphPtr = tm_tcp_hdr_ptr(prevPacketPtr);
            if (   tm_16bit_one_bit_set(packetPtr->pktSharedDataPtr->dataFlags,
                                        TM_BUF_TCP_OOB_BYTE )
                && tm_16bit_bits_not_set(
                                     prevPacketPtr->pktSharedDataPtr->dataFlags,
                                     TM_BUF_TCP_OOB_BYTE )
               )
            {
                prevTcphPtr->tcpUrgentPtr = (tt16Bit)
                            (   tcphPtr->tcpUrgentPtr
                              + (tt16Bit)prevPacketPtr->pktChainDataLength);
            }
            if (tm_8bit_one_bit_set(tcphPtr->tcpControlBits,
                                    TM_TCP_FIN|TM_TCP_PSH))
            {
                prevTcphPtr->tcpControlBits |= (tt8Bit)
                        (tcphPtr->tcpControlBits & (TM_TCP_FIN|TM_TCP_PSH));
            }
            prevPacketPtr->pktChainDataLength += packetPtr->pktChainDataLength;
            prevPacketPtr->pktLinkDataLength += packetPtr->pktLinkDataLength;
            appended = TM_8BIT_YES;
        }
    }
    return appended;
}

#ifdef TM_TCP_SACK
/*
 * tfTcpSackQueueBlock function description:
 * Insert a SEL ACK block at the head of the TCP state vector list of
 * SEL ACK blocks (that we send in SEL ACK options).
 * 1. Find all currently SEL ACK queued blocks that intersect with the
 *    new SEL ACK block. For each found intersecting SEL ACK block,
 *    do save the biggest range of sequence numbers in our left edge and
 *    right edge parameters, if applicable, and remove the intersecting block.
 * 2. Allocate a new SEL ACK block
 * 3. Initialize with left edge, and right edge parameters.
 * 4. Insert at head of SEL ACK blocks queue.
 * Parameters:
 * tcpVectPtr        Pointer to Tcp state vector
 * leftEdgeSeqNo     Left edge sequence number of the range
 * RightEdgeSeqNo    Right edge sequence number of the range
 * No return value
 */

void
tfTcpSackQueueBlock ( ttTcpVectPtr tcpVectPtr,
                      ttSqn        leftEdgeSeqNo,
                      ttSqn        rightEdgeSeqNo,
                      int          sackBlockIndex )
{
    ttSackEntryPtr    sackBlockPtr;
#ifdef TM_TCP_FACK
    ttSqn             overlapLeftEdge;
    ttSqn             overlapRightEdge;
    tt32Bit           ackedBytes;
    tt8Bit            overlapCheck;
#endif /* TM_TCP_FACK */

    sackBlockPtr = tcpVectPtr->tcpsSackBlockPtr[sackBlockIndex];
#ifdef TM_TCP_FACK
    overlapLeftEdge = (ttSqn)0; /* for compiler warning */
    overlapRightEdge = (ttSqn)0; /* for compiler warning */
    if (    (sackBlockIndex == TM_TCP_SACK_RECV_INDEX)
         && (tcpVectPtr->tcpsRetranData != 0) )
    {
        ackedBytes = rightEdgeSeqNo - leftEdgeSeqNo;
        overlapCheck = TM_8BIT_YES;
    }
    else
    {
        overlapCheck = TM_8BIT_ZERO;
        ackedBytes = TM_32BIT_ZERO;
    }
#endif /* TM_TCP_FACK */
/* Find all intersecting SEL ACK blocks */
    while ( sackBlockPtr != (ttSackEntryPtr)0 )
    {
        if (    tm_sqn_leq(leftEdgeSeqNo, sackBlockPtr->blocQRightEdge)
             && tm_sqn_geq(rightEdgeSeqNo, sackBlockPtr->blocQLeftEdge) )
/* Non empty intersection with the new block */
        {
#ifdef TM_TCP_FACK
            if (overlapCheck != TM_8BIT_ZERO)
            {
                overlapLeftEdge = sackBlockPtr->blocQLeftEdge;
                overlapRightEdge = sackBlockPtr->blocQRightEdge;
            }
#endif /* TM_TCP_FACK */
            if (tm_sqn_gt(leftEdgeSeqNo, sackBlockPtr->blocQLeftEdge))
/* If existing block starts before our left edge, make it our left edge */
            {
#ifdef TM_TCP_FACK
                if (overlapCheck != TM_8BIT_ZERO)
                {
                    overlapLeftEdge = leftEdgeSeqNo;
                }
#endif /* TM_TCP_FACK */
                leftEdgeSeqNo = sackBlockPtr->blocQLeftEdge;
            }
            if (tm_sqn_lt(rightEdgeSeqNo, sackBlockPtr->blocQRightEdge))
/* If existing block ends after our right edge, make it our right edge */
            {
#ifdef TM_TCP_FACK
                if (overlapCheck != TM_8BIT_ZERO)
                {
                    overlapRightEdge = rightEdgeSeqNo;
                }
#endif /* TM_TCP_FACK */
                rightEdgeSeqNo = sackBlockPtr->blocQRightEdge;
            }
#ifdef TM_TCP_FACK
            if (overlapCheck != TM_8BIT_ZERO)
            {
                ackedBytes -= overlapRightEdge - overlapLeftEdge;
            }
#endif /* TM_TCP_FACK */
/* Delete existing SEL ACK block, return next sack block in the queue */
            sackBlockPtr = tfTcpSackRemoveBlock( tcpVectPtr,
                                                 sackBlockPtr,
                                                 sackBlockIndex );
        }
        else
        {
/* Next block in the queue */
            sackBlockPtr = sackBlockPtr->blocQNextPtr;
        }
    }
/* Allocate a new SEL ACK block */
    sackBlockPtr = (ttSackEntryPtr)tm_get_raw_buffer(
                                               (unsigned)sizeof(ttSackEntry));
    if (sackBlockPtr != (ttSackEntryPtr)0)
/* Initialize, and Insert new SEL ACK block at head of the queue */
    {
        sackBlockPtr->blocQLeftEdge = leftEdgeSeqNo;
        sackBlockPtr->blocQRightEdge = rightEdgeSeqNo;
        sackBlockPtr->blocQPrevPtr = (ttSackEntryPtr)0;
/* Append current first block in queue (if any), to the new block  */
        sackBlockPtr->blocQNextPtr =
                                tcpVectPtr->tcpsSackBlockPtr[sackBlockIndex];
        if (tcpVectPtr->tcpsSackBlockPtr[sackBlockIndex] != (ttSackEntryPtr)0)
        {
            tcpVectPtr->tcpsSackBlockPtr[sackBlockIndex]->blocQPrevPtr =
                                                                 sackBlockPtr;
        }
/* New block at head of queue */
        tcpVectPtr->tcpsSackBlockPtr[sackBlockIndex] = sackBlockPtr;
    }
#ifdef TM_ERROR_CHECKING
    else
    {
        tfKernelWarning("tfTcpSackQueueBlock",
                        "No memory to allocate a SEL ACK block");
    }
#endif /* TM_ERROR_CHECKING */
#ifdef TM_TCP_FACK
    if (sackBlockIndex == TM_TCP_SACK_RECV_INDEX)
    {
        tfTcpSackUpdateFack(tcpVectPtr, leftEdgeSeqNo, rightEdgeSeqNo,
                            ackedBytes);
    }
#endif /* TM_TCP_FACK */
    return;
}

/*
 * tfTcpSackDeQueueBlocks function description:
 * Dequeue from the SEL ACK tcp vector block queue (used for SEL ACK options),
 * and free all the SEL ACK blocks that lie entirely within the sequence
 * number range given by the leftEdgeSeqNo parameter, and rightEdgeSeqNo
 * parameter. If there is a partial intersection, just make the block
 * sequence numbers range smaller.
 * Parameters:
 * tcpVectPtr        Pointer to Tcp state vector
 * leftEdgeSeqNo     Left edge sequence number of the range
 * RightEdgeSeqNo    Right edge sequence number of the range
 * No return value
 */
void
tfTcpSackDeQueueBlocks ( ttTcpVectPtr tcpVectPtr,
                         ttSqn        leftEdgeSeqNo,
                         ttSqn        rightEdgeSeqNo,
                         int          sackBlockIndex )
{
    ttSackEntryPtr    sackBlockPtr;
#ifdef TM_TCP_FACK
    tt32Bit           ackedBytes;
    tt8Bit            overlapCheck;
#endif /* TM_TCP_FACK */

    sackBlockPtr = tcpVectPtr->tcpsSackBlockPtr[sackBlockIndex];
#ifdef TM_TCP_FACK
    if (    (sackBlockIndex == TM_TCP_SACK_RECV_INDEX)
         && (tcpVectPtr->tcpsRetranData != 0) )
    {
        ackedBytes = rightEdgeSeqNo - leftEdgeSeqNo;
        overlapCheck = TM_8BIT_YES;
    }
    else
    {
        overlapCheck = TM_8BIT_ZERO;
        ackedBytes = TM_32BIT_ZERO;
    }
#endif /* TM_TCP_FACK */
/*
 * Remove, and free all SEL ACK blocks inside the range
 */
    while (sackBlockPtr != (ttSackEntryPtr)0)
    {
        if (    tm_sqn_geq(sackBlockPtr->blocQLeftEdge, leftEdgeSeqNo)
             && tm_sqn_leq(sackBlockPtr->blocQRightEdge, rightEdgeSeqNo) )
/* Found a block which lies inside the range */
        {
#ifdef TM_TCP_FACK
            if (overlapCheck != TM_8BIT_ZERO)
            {
                ackedBytes -=
                    sackBlockPtr->blocQRightEdge - sackBlockPtr->blocQLeftEdge;
            }
#endif /* TM_TCP_FACK */
/* Delete existing SEL ACK block, return next sack block in the queue */
            sackBlockPtr = tfTcpSackRemoveBlock(tcpVectPtr, sackBlockPtr,
                                                sackBlockIndex);
        }
        else
        {
            if (    tm_sqn_leq(leftEdgeSeqNo, sackBlockPtr->blocQRightEdge)
                 && tm_sqn_geq(rightEdgeSeqNo, sackBlockPtr->blocQLeftEdge) )
/* Partial intersection with the block. block gets smaller */
            {
#ifdef TM_ERROR_CHECKING
                if (    tm_sqn_gt(leftEdgeSeqNo, sackBlockPtr->blocQLeftEdge)
                     && tm_sqn_lt( rightEdgeSeqNo,
                                   sackBlockPtr->blocQRightEdge) )
                 {
/* Range lies strictly inside a block. This should never happen */
                     tfKernelWarning("tfTcpSackDeQueueBlocks",
                                     "Lost bytes in reassemble queue\n");
                }
#endif /* TM_ERROR_CHECKING */
                if (tm_sqn_leq(leftEdgeSeqNo, sackBlockPtr->blocQLeftEdge))
/*
 * If range left edge is before our block left edge, all the Sequence
 * numbers between the block left edge, and the range right edge need
 * to be deleted: store right edge sqn in the block left edge.
 */
                {
                    sackBlockPtr->blocQLeftEdge = rightEdgeSeqNo;
                }
                else
                {
#ifdef TM_ERROR_CHECKING
                    if (tm_sqn_geq( rightEdgeSeqNo,
                                    sackBlockPtr->blocQRightEdge))
/*
 * If range right edge is beyond our block right edge, all the Sequence
 * numbers between the range left edge, and block right edge need
 * to be deleted: store left edge sqn in the block right edge.
 * This means that we are deleting sequence numbers at the end of our
 * block leaving some sequence numbers at the top. This should never happen.
 */
                    {
                         tfKernelWarning("tfTcpSackDeQueueBlocks",
                                         "gap in reassemble queue\n");
                         sackBlockPtr->blocQRightEdge = leftEdgeSeqNo;
                    }
#endif /* TM_ERROR_CHECKING */
                }
            }
/* Point to next block in queue */
            sackBlockPtr = sackBlockPtr->blocQNextPtr;
        }
    }
#ifdef TM_TCP_FACK
    if (sackBlockIndex == TM_TCP_SACK_RECV_INDEX)
    {
        tfTcpSackUpdateFack(tcpVectPtr, leftEdgeSeqNo, rightEdgeSeqNo,
                            ackedBytes);
    }
#endif /* TM_TCP_FACK */
    return;
}

#ifdef TM_TCP_FACK
/*
 * If new data is acknowledged:
 * . Update wintrim (data smoothing): adjustment to congestion window during
 *   recovery (winTrim != 0).
 * . Update Forward-most data (highest sqn + 1) held by the receiver.
 * If retransmitted data is acknowledged:
 * . adjust retransmitted bytes during recovery (retransData != 0).
 */
static void tfTcpSackUpdateFack (ttTcpVectPtr tcpVectPtr,
                                 ttSqn        leftEdgeSeqNo,
                                 ttSqn        rightEdgeSeqNo,
                                 tt32Bit      ackedBytes)
{
    ttS32Bit reduction;

/* Update adjustment to congestion window during recovery */
    if (tm_sqn_lt(tcpVectPtr->tcpsSndFack, rightEdgeSeqNo))
    {
        if (tcpVectPtr->tcpsWinTrim != TM_L(0))
/* In recovery: adjust winTrim (data smoothing) */
        {
            reduction = (ttS32Bit)((rightEdgeSeqNo -
                                    tcpVectPtr->tcpsSndFack) / 2);
            if (reduction < tcpVectPtr->tcpsWinTrim)
            {
                tcpVectPtr->tcpsWinTrim -= reduction;
            }
            else
            {
                tcpVectPtr->tcpsWinTrim = TM_L(0);
            }
        }
        if (ackedBytes != TM_32BIT_ZERO)
        {
            if (tm_sqn_lt(leftEdgeSeqNo, tcpVectPtr->tcpsSndFack))
/* Acking some retransmitted bytes */
            {
/* Remove from acked bytes count the non retransmitted bytes */
                ackedBytes -= (rightEdgeSeqNo - tcpVectPtr->tcpsSndFack);
            }
            else
            {
/* Acking new bytes */
                ackedBytes = TM_32BIT_ZERO;
            }
        }
/* Forward-most data (highest sqn + 1) held by the receiver */
        tcpVectPtr->tcpsSndFack = rightEdgeSeqNo;
    }
    if (ackedBytes != TM_32BIT_ZERO)
/*
 * In recovery: adjust amount of retransmitted data by the number of bytes
 * Acked
 */
    {
        if (ackedBytes < tcpVectPtr->tcpsRetranData)
        {
            tcpVectPtr->tcpsRetranData -= ackedBytes;
        }
        else
        {
            tcpVectPtr->tcpsRetranData = TM_UL(0);
        }
    }
    return;
}
#endif /* TM_TCP_FACK */

/*
 * tfTcpSackRemoveBlock function description:
 * Dequeue one TCP SACK block from the given SEL ACK tcp vector block queue
 * and recycle it.
 * Parameters:
 * tcpVectPtr        Pointer to Tcp state vector
 * sackBlockPtr      Pointer to TCP Sack Block to remove
 * sackBlockIndex    which tcp vector block queue.
 * Return value
 * Next TCP SACK block in the given queue
 */
static ttSackEntryPtr tfTcpSackRemoveBlock( ttTcpVectPtr   tcpVectPtr,
                                            ttSackEntryPtr sackBlockPtr,
                                            int            sackBlockIndex )
{
    ttSackEntryPtr    recycleSackBlockPtr;

/* Recycle this block */
    recycleSackBlockPtr = sackBlockPtr;
/*
 * Remove the block to recycle from the TCP SEL ACK blocks queue
 */
/* previous block in queue*/
    sackBlockPtr = recycleSackBlockPtr->blocQPrevPtr;
    if (sackBlockPtr != (ttSackEntryPtr)0)
    {
        sackBlockPtr->blocQNextPtr = recycleSackBlockPtr->blocQNextPtr;
    }
    else
    {
        tcpVectPtr->tcpsSackBlockPtr[sackBlockIndex] =
                                    recycleSackBlockPtr->blocQNextPtr;
    }
/* Point to next block in queue */
    sackBlockPtr = recycleSackBlockPtr->blocQNextPtr;
    if (sackBlockPtr != (ttSackEntryPtr)0)
    {
        sackBlockPtr->blocQPrevPtr = recycleSackBlockPtr->blocQPrevPtr;
    }
/*
 * Recycle SEL ACK block
 */
    tm_free_raw_buffer(recycleSackBlockPtr);
    return sackBlockPtr;
}

static void tfTcpSackFreeQueue(ttTcpVectPtr tcpVectPtr, int sackIndex)
{
    ttSackEntryPtr    sackBlockPtr;
    ttSackEntryPtr    recycleSackBlockPtr;

    sackBlockPtr = tcpVectPtr->tcpsSackBlockPtr[sackIndex];
    tcpVectPtr->tcpsSackBlockPtr[sackIndex] = (ttSackEntryPtr)0;
    while (sackBlockPtr != (ttSackEntryPtr)0)
    {
/*
 * Recycle SEL ACK block
 */
        recycleSackBlockPtr = sackBlockPtr;
/* Point to next block in queue */
        sackBlockPtr = sackBlockPtr->blocQNextPtr;
        tm_free_raw_buffer(recycleSackBlockPtr);
    }
    return;
}

#endif /* TM_TCP_SACK */

#ifdef TM_DEV_SEND_OFFLOAD
static void tfTcpOffloadInit(ttTcpVectPtr tcpVectPtr)
{
    ttSocketEntryPtr socketPtr;
    ttDeviceEntryPtr devPtr;
    ttRteEntryPtr    rtePtr;
    int              mtu;

    socketPtr = &(tcpVectPtr->tcpsSocketEntry);
    rtePtr = socketPtr->socRteCacheStruct.rtcRtePtr;
    tm_assert(tfTcpOffloadInit, rtePtr != (ttRteEntryPtr)0);
    devPtr = rtePtr->rteOrigDevPtr;
    tm_assert(tfTcpOffloadInit, devPtr != (ttDeviceEntryPtr)0);
    tcpVectPtr->tcpsOffloadFlags = devPtr->devOffloadFlags;
    /* Default value if we do not offload */
    tcpVectPtr->tcpsOffloadDataSize = tcpVectPtr->tcpsEffSndMss;
    if (tm_tcp_send_offload(tcpVectPtr->tcpsOffloadFlags))
    {
/* IP MTU without minimum TCP/IP header */
        mtu =   tcpVectPtr->tcpsMSS
              + devPtr->devLinkHeaderLength;
#ifdef TM_USE_IPV4
#ifdef TM_USE_IPV6
        if (tcpVectPtr->tcpsNetworkLayer == TM_NETWORK_IPV4_LAYER)
#endif /* dual */
        {
/* Only need IPv4 offload flag */
            tm_16bit_clr_bit( tcpVectPtr->tcpsOffloadFlags,
                              TM_DEVO_TCP_SEGMENT_OFFLOAD_V6 );
            if (tm_tcp_send_seg_offload(tcpVectPtr->tcpsOffloadFlags))
            {
/* Full IP MTU: add minimum TCP/IP header */
                mtu += (   tm_byte_count( TM_PAK_TCP_HDR_LEN)
                         + TM_4_IP_MIN_HDR_LEN );
                if (devPtr->devOffloadMaxSize <= (tt16Bit)mtu)
                {
/* Do not bother offloading if we fit in the MTU */
                    tm_16bit_clr_bit( tcpVectPtr->tcpsOffloadFlags,
                                      TM_DEVO_TCP_SEGMENT_OFFLOAD_V4 );
                }
                else
                {
/*
 * If segmentation offload is on, then IP checksum offload should be on as
 * well
 */
                    tcpVectPtr->tcpsOffloadFlags
                                          |= TM_DEVO_IP_CHKSUM_OFFLOAD;
/* Used as EffSndMss for offloading */
                    tcpVectPtr->tcpsOffloadDataSize =  (tt16Bit)
                            (   devPtr->devOffloadMaxSize
                              - mtu
                              + tcpVectPtr->tcpsEffSndMss );
                }
            }
        }
#ifdef TM_USE_IPV6
        else
#endif /* dual layer */
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
        {
/* Only need IPv6 offload flag, and do not need IP checksum offload */
            tm_16bit_clr_bit( tcpVectPtr->tcpsOffloadFlags,
                              (  TM_DEVO_TCP_SEGMENT_OFFLOAD_V4
                               | TM_DEVO_IP_CHKSUM_OFFLOAD) );
            if (tm_tcp_send_seg_offload(tcpVectPtr->tcpsOffloadFlags))
            {
/* Full IP MTU */
                mtu += (   tm_byte_count( TM_PAK_TCP_HDR_LEN)
                         + TM_6_IP_MIN_HDR_LEN );
                if (devPtr->devOffloadMaxSize <= (tt16Bit)mtu)
                {
                    tm_16bit_clr_bit( tcpVectPtr->tcpsOffloadFlags,
                                      TM_DEVO_TCP_SEGMENT_OFFLOAD_V6 );
                }
                else
                {
                    tcpVectPtr->tcpsOffloadDataSize =
                            (   devPtr->devOffloadMaxSize
                              - mtu
                              + tcpVectPtr->tcpsEffSndMss );
                }
            }
        }
#endif /* TM_USE_IPV6 */
    }
    return;
}
#endif /* TM_DEV_SEND_OFFLOAD */

/*
 * Compute checksum on pseudo header
 * Note the checksum is returned un-complemented.
 * NOTE: Called from tfTcpSendPacket() only.
 */
tt16Bit tfPseudoHeaderChecksum( tt16BitPtr  pshPtr,
                                tt16Bit     pshLength )
{
    tt32Bit             sum;

/* Checksum only the pseudo header */
    sum = (tt32Bit)0;
#ifdef TM_DSP
#ifdef TM_16BIT_DSP
    while (pshLength >= 2)
    {
        sum += *pshPtr++ & 0xFFFF;
        pshLength -= 2;
    }
#endif /* TM_16BIT_DSP */
#ifdef TM_32BIT_DSP
    while (pshLength >= 4)
    {
        sum += (*pshPtr >> 16) & 0xffff;
        sum += *pshPtr & 0xffff;
        pshPtr++;
        pshLength -= 4;
    }
#endif /* TM_32BIT_DSP */
#else /* !TM_DSP */
    while (pshLength >= 1)
    {
        sum += *pshPtr++;
        pshLength -= 2;
    }
#endif /* !TM_DSP */
    tm_add_carries_sum(sum);
    return (tt16Bit)sum;
}

/* Check if we need to update the send window variables */
void tfTcpUpdateSendWindow (ttTcpVectPtr tcpVectPtr, tt32Bit seqNo,
                            tt32Bit segAck, tt32Bit unscaledWnd)
{
/* if peer is sending new data */
    if (   tm_sqn_lt(tcpVectPtr->tcpsSndWL1, seqNo)
/* or if peer is acking new data */
        || (    (tcpVectPtr->tcpsSndWL1 == seqNo)
             && tm_sqn_lt(tcpVectPtr->tcpsSndWL2, segAck))
/* or if peer is sending a pure window update */
        || (    (tcpVectPtr->tcpsSndWL2 == segAck)
             && (unscaledWnd > tcpVectPtr->tcpsSndWnd) ) )
    {
/* Update Tcp state vector send window variables */
        tcpVectPtr->tcpsSndWnd = unscaledWnd;
        tcpVectPtr->tcpsSndWL1 = seqNo;
        tcpVectPtr->tcpsSndWL2 = segAck;
        if (tcpVectPtr->tcpsMaxSndWnd < tcpVectPtr->tcpsSndWnd)
        {
/* Highest send window seen so far (peer receive window) */
            tcpVectPtr->tcpsMaxSndWnd = tcpVectPtr->tcpsSndWnd;
        }
        tcpVectPtr->tcpsFlags2 |= TM_TCPF2_SEND_DATA;
    }
    return;
}

/*
 * tfTcpTmWtGetIssClose function description:
 * Grab the time wait vector last sequence number (plus some) as our new
 * ISS
 * Close the Time Wait vector.
 *
 * Called either:
 * 1. when processing an incoming SYN targeting a tcp state vector
 *    in time wait state when the sequence number of the incoming segment
 *    is from a new connection (bigger than, or equal to the expected
 *    sequence number from the old connection) (RFC 1122):
 * or
 * 2. when sending a SYN for a new connection when there is a duplicate
 *    TCP vector in the time wait state for the same 4-tuple.
 * Try and re-use the ISS of the previous connection in time wait state
 * (RFC 1122).
 *
 * . use the time wait vector last sequence number + some as our new ISS,
 * . close the time wait TCP state vector/socket,
 */
static ttSqn tfTcpTmWtGetIssClose(ttTcpTmWtVectPtr tcpTmWtVectPtr)
{
    ttSqn              newIss;
/*
 * Get Time Wait ISS + R(t). Clear random number MSB so that we do not wrap
 * around, and make sure we add at least 32768
 */
    newIss =   tcpTmWtVectPtr->twsMaxSndNxt
             + ((tfGetRandom() & 0x7FFFFFFF) | 0x8000);
    if (newIss == TM_UL(0))
    {
        newIss++;
    }
/*
 * Close it so nobody else can use its ISS, and so that we do not
 * grab it again.
 */
    (void)tfTcpTmWtVectClose(tcpTmWtVectPtr);
    return newIss;
}

void tfTcpTmWtTsInit(ttTcpTmWtVectPtr tcpTmWtVectPtr,
                     ttTcpVectPtr     tcpVectPtr)
{
    if (tm_8bit_one_bit_set(tcpTmWtVectPtr->twsFlags, TM_TCPTWF_TS))
    {
        if (tcpVectPtr->tcpsState != TM_TCPS_LISTEN)
        {
            tcpVectPtr->tcpsFlags = (tt16Bit)
                                    (tcpVectPtr->tcpsFlags | TM_TCPF_TS);
        }
        tcpVectPtr->tcpsTsRecent = tcpTmWtVectPtr->twsTsRecent;
        tcpVectPtr->tcpsTsRecentAge = tcpTmWtVectPtr->twsTsRecentAge;
        tcpVectPtr->tcpsLastAckSent = tcpTmWtVectPtr->twsRcvNxt;
    }
}

/*
 * tfTcpTmWtProcess Function Description
 * Process incoming packet targeting a TCP vector in the time wait state.
 * Parameters
 * tcpTmWtVectPtr pointer to targeted TCP Time Wait state vector.
 * tcpVectPtr     pointer to corresponding listening socket, or closed vector
 * packetPtr      pointer to incoming packet
 * event          TCP state machine event
 * Returns
 * packetPtr      incoming packet Pointer
 * null           incoming packet pointer has been freed
 */
ttPacketPtr tfTcpTmWtProcess(ttTcpTmWtVectPtr tcpTmWtVectPtr,
                             ttTcpVectPtr     tcpVectPtr,
                             ttPacketPtr      packetPtr,
                             tt8Bit           event)
{
    ttTcpHeaderPtr      tcphPtr;
    ttPacketPtr         sendPacketPtr;
    ttSocketEntryPtr    listenSocketPtr;
    ttSqn               seqNo;
    ttSqn               ackNo;
    tt8Bit              ctlBits;

    tcphPtr = tm_tcp_hdr_ptr(packetPtr);
    if (tm_8bit_one_bit_set(tcpTmWtVectPtr->twsFlags, TM_TCPTWF_TS))
    {
        tcpTmWtVectPtr->twsTsRecent = tcpVectPtr->tcpsTsRecent;
        tcpTmWtVectPtr->twsTsRecentAge = tcpVectPtr->tcpsTsRecentAge;
    }
    if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags2, TM_TCPF2_ACK))
/*
 * Note: Use ctlBits, instead of TM_TCPF2_ACK, because tcpVectPtr
 * could be unlocked in the tfTcpStateMachine() call below.
 */
    {
        ctlBits = TM_TCP_ACK;
/* Reset ACK */
        tm_16bit_clr_bit( tcpVectPtr->tcpsFlags2, TM_TCPF2_ACK);
    }
    else
    {
        ctlBits = TM_8BIT_ZERO;
    }
    switch (event)
    {
        case TM_TCPE_SEG_SYN:
/* TCP NEW ISS */
/*
 * If the sequence number of the incoming segment is from a new
 * connection (bigger than, or equal to the expected sequence number from the
 * old connection) (not a duplicate).
 * ANVL-CORE 11.22 check seqno >= rcvNxt (instead of seqno > rcvNxt)
 */
            if (tm_sqn_geq(tcphPtr->tcpSeqNo, tcpTmWtVectPtr->twsRcvNxt))
/*
 * RFC 1122 4.2.2.13 allows to close a vector in TIME WAIT state if a new
 * connection request arrives provided the ISS is increased to not overlap.
 */
            {
                if (tcpVectPtr->tcpsState == TM_TCPS_LISTEN)
/*
 * ANVL-CORE 8.28: only close the TCP Time Wait vector if the listening socket
 * had not been closed.
 */
                {
/* Grab ISS from Time Wait and close the TCP time wait vector */
/* listening socket is locked and checked out */
/* save newIss onto listening socket */
                    tcpVectPtr->tcpsIss = tfTcpTmWtGetIssClose(tcpTmWtVectPtr);
/* Unlock the Time Wait table */
                    tm_call_unlock(&tm_context(tvTcpTmWtTableLock));
                    tcpTmWtVectPtr = (ttTcpTmWtVectPtr)0;
/*
 * Now crank the state machine with the tcpVectPtr. It will
 * free the packet.
 * NOTE: tcpVectPtr unlocked/relocked.
 */
                    (void)tfTcpStateMachine(tcpVectPtr,
                                            packetPtr,
                                            TM_TCPE_SEG_SYN);
/* Prevent the caller from freeing packet */
                    packetPtr = (ttPacketPtr)0;
/* Do not send below */
                    ctlBits = TM_8BIT_ZERO;
                }
                else
                {
/* ANVL-CORE 8.28 OTW SYN with no listening vector, Ack it */
                    ctlBits = TM_TCP_ACK;
                }
            }
            break;
        case TM_TCPE_SEG_ACK:
/*
 * Incoming ACK in TIME_WAIT state
 *
 * If invalid ACK, Ack with proper SQN, ACK. (ANVL-CORE 3.22)
 * else
 * If seqNo is rcvNxt - 1 (duplicate FIN), restart 2MSL timer
 */
            if (tm_sqn_gt(tcphPtr->tcpAckNo, tcpTmWtVectPtr->twsMaxSndNxt))
/* ANVL-CORE 3.22 */
            {
/* Invalid ACK, Ack */
                ctlBits = TM_TCP_ACK;
            }
            else
            {
/*
 * If seqNo is rcvNxt - 1 (duplicate FIN), restart 2MSL timer.
 */
                if (tcphPtr->tcpSeqNo == tcpTmWtVectPtr->twsRcvNxt-1)
                {
                    tm_tcp_timer_new_time(
                            tcpTmWtVectPtr->tws2MslTmPtr,
                            tm_tmr_interval(tcpTmWtVectPtr->tws2MslTmPtr));
                }
            }
            break;
        case TM_TCPE_SEG_SYN_ACK:
/* Bad Segment Abort: incoming segment makes us send a reset */
            ctlBits = TM_TCP_RST;
            break;
        case TM_TCPE_SEG_RST:
/* Ignore the Reset (RFC 1337) */
            ctlBits = TM_8BIT_ZERO;
            break;
        default:
            break;
    }
    if (ctlBits)
    {
/* Common code for sending a TCP packet */
        if (tcpVectPtr->tcpsState == TM_TCPS_LISTEN)
        {
            listenSocketPtr = &(tcpVectPtr->tcpsSocketEntry);
            tcpVectPtr = tfTcpClosedVectGet();
            if (tcpVectPtr != (ttTcpVectPtr)0)
            {
                tfTcpTmWtTsInit(tcpTmWtVectPtr, tcpVectPtr);
            }
        }
        else
        {
            listenSocketPtr = (ttSocketEntryPtr)0;
        }
        if (tcpVectPtr != (ttTcpVectPtr)0)
        {
/* Copy socket tuple, TCP SND variables, offload flags, headerSize */
            tm_bcopy(&(tcpTmWtVectPtr->twsTupleDev.tudTuple),
                     &(tcpVectPtr->tcpsSocketEntry.socTuple),
                     sizeof(ttSockTuple));
            tcpVectPtr->tcpsSndNxt = tcpTmWtVectPtr->twsMaxSndNxt;
            tcpVectPtr->tcpsSndUna = tcpTmWtVectPtr->twsMaxSndNxt;
#ifdef TM_USE_IPV6
            tcpVectPtr->tcpsNetworkLayer = tcpTmWtVectPtr->twsNetworkLayer;
#endif /* TM_USE_IPV6 */
            if (tm_8bit_one_bit_set(tcpTmWtVectPtr->twsFlags, TM_TCPTWF_TS))
            {
                tcpVectPtr->tcpsHeaderSize =  TM_BYTES_TCP_HDR
                                            + TM_TCPO_TS_NOOP_LENGTH;
            }
            else
            {
                tcpVectPtr->tcpsHeaderSize = TM_BYTES_TCP_HDR;
            }
#ifdef TM_USE_STRONG_ESL
            if (tcpTmWtVectPtr->twsTupleDev.tudDevPtr != (ttDeviceEntryPtr)0)
            {
                tcpVectPtr->tcpsSocketEntry.socRteCacheStruct.rtcDevPtr =
                                   tcpTmWtVectPtr->twsTupleDev.tudDevPtr;
            }
#endif /* TM_USE_STRONG_ESL */
/* End of copy */
            if (ctlBits == TM_TCP_RST)
            {
/* Delete and Free TCB */
                (void)tfTcpTmWtVectClose(tcpTmWtVectPtr);
                seqNo = tm_tcp_hdr_ptr(packetPtr)->tcpAckNo;
                ackNo = (ttSqn)0;
                sendPacketPtr = packetPtr;
/* Packet freed in send */
                packetPtr = TM_PACKET_NULL_PTR;
            }
            else
            {
                seqNo = tcpTmWtVectPtr->twsMaxSndNxt;
                ackNo = tcpTmWtVectPtr->twsRcvNxt;
                sendPacketPtr = TM_PACKET_NULL_PTR;
            }
            tm_call_unlock(&tm_context(tvTcpTmWtTableLock));
            tcpTmWtVectPtr = (ttTcpTmWtVectPtr)0;
            if (listenSocketPtr != (ttSocketEntryPtr)0)
            {
                tm_call_unlock(&listenSocketPtr->socLockEntry);
            }
            (void)tfTcpCreateSendPacket(tcpVectPtr, sendPacketPtr,
                                        seqNo, ackNo, ctlBits);
            if (listenSocketPtr != (ttSocketEntryPtr)0)
            {
                tm_call_lock_wait(&listenSocketPtr->socLockEntry);
                tm_call_unlock(&(tcpVectPtr->tcpsSocketEntry.socLockEntry));
                tfTcpClosedVectFree(tcpVectPtr);
            }
        }
    }
    if (tcpTmWtVectPtr != (ttTcpTmWtVectPtr)0)
/* Common code for unlocking the Time Wait table lock */
    {
/* Unlock the time wait vector table */
        tm_call_unlock(&tm_context(tvTcpTmWtTableLock));
    }
    return packetPtr;
}

/* Time Wait TCP Vector Close function */
void tfTcpTmWtVectClose(ttTcpTmWtVectPtr tcpTmWtVectPtr)
{
    ttTimerPtr  timerPtr;
    ttListPtr   listPtr;

#ifdef TM_SNMP_CACHE
    tfSnmpdCacheDeleteTmWtVect(tcpTmWtVectPtr);
#endif /* TM_SNMP_CACHE */
    listPtr = tfTcpTmWtGetListPtr(&(tcpTmWtVectPtr->twsTupleDev.tudTuple));
/* Number of time wait vectors in the table */
    tm_context(tvTcpTmWtTableMembers)--;
    (void)tfListRemove(listPtr, &(tcpTmWtVectPtr->twsNode));
    if (listPtr->listCount == 0)
    {
/* Number of hash buckets filled */
        tm_context(tvTcpTmWtTableHashBucketFilled)--;
    }
    if (tcpTmWtVectPtr->tws2MslTmPtr != (ttTimerPtr)0)
    {
        timerPtr = tcpTmWtVectPtr->tws2MslTmPtr;
        tcpTmWtVectPtr->tws2MslTmPtr = (ttTimerPtr)0;
        tfTimerRemove(timerPtr);
/* Time Wait TCP vector will be freed in timer cleanup function */
    }
    return;
}

/*
 * tfTcpTmWtTmrCleanup()
 * Time Wait TCP Vector Timer Clean up function:
 * Clean up function called by the timer execute before freeing a timer
 * or by tfTimerRemove().
 * Called after the Timer Wait TCP vector has been removed from the table.
 */
static void tfTcpTmWtTmrCleanup(ttVoidPtr       timerBlockPtr,
                                tt8Bit          flags)
{
    ttTcpTmWtVectPtr tcpTmWtVectPtr;

    TM_UNREF_IN_ARG(flags);
    tcpTmWtVectPtr = (ttTcpTmWtVectPtr)
            (tm_tmr_arg1((ttTimerPtr)timerBlockPtr).genVoidParmPtr);
    tm_free_raw_buffer(tcpTmWtVectPtr);
    return;
}

/* Return a pointer to the ttList where the matching time wait vector is */
static ttListPtr tfTcpTmWtGetListPtr (ttSockTuplePtr sockTuplePtr)
{
    ttListPtr        listPtr;
    tt32Bit          hashValue;

    hashValue = tfSocketHash((ttCharPtr)(ttVoidPtr)sockTuplePtr,
                             sizeof(ttSockTuple),
                             tm_context(tvTcpTmWtTableHashSize));
    listPtr = &(tm_context(tvTcpTmWtTableListPtr)[hashValue]);
    return listPtr;
}

/* TCP Time Wait table Table lookup */
ttTcpTmWtVectPtr tfTcpTmWtLookup(ttTupleDevPtr tupleDevPtr)
{
    ttTcpTmWtVectPtr tcpTmWtVectPtr;
    ttListPtr        listPtr;
    ttGenericUnion   genParam;

    listPtr = tfTcpTmWtGetListPtr(&(tupleDevPtr->tudTuple));
    if (listPtr->listCount == 0)
    {
        tcpTmWtVectPtr = (ttTcpTmWtVectPtr)0;
    }
    else
    {
        genParam.genVoidParmPtr = tupleDevPtr;
        tcpTmWtVectPtr = (ttTcpTmWtVectPtr)(ttVoidPtr)
             tfListWalk(listPtr,
                        tfTcpTmWtTupleEqualCB,
                        genParam);
    }
    return tcpTmWtVectPtr;
}

/* TCP Time Wait table Hash list walk CB function */
static int tfTcpTmWtTupleEqualCB(
    ttNodePtr       nodePtr,
    ttGenericUnion  genParam)
{
    ttTcpTmWtVectPtr    tcpTmWtVectPtr;
    ttTupleDevPtr       tupleDevPtr;
    int                 retCode;

    tcpTmWtVectPtr = (ttTcpTmWtVectPtr)(ttVoidPtr)nodePtr;
    tupleDevPtr = (ttTupleDevPtr)genParam.genVoidParmPtr;
/* PRQA: QAC Message 3416: The calling function doesn't have the side */
/* PRQA: QAC Message 3416: effect for the comparison processing */
/* PRQA S 3416 L1 */
    retCode = tm_tupleptr_equal(
                        &(tupleDevPtr->tudTuple),
                        &(tcpTmWtVectPtr->twsTupleDev.tudTuple));
/* PRQA L:L1 */
#ifdef TM_USE_STRONG_ESL
    if (    (retCode == TM_8BIT_YES)
         && (tcpTmWtVectPtr->twsTupleDev.tudDevPtr != (ttDeviceEntryPtr)0)
       )
    {
         if (tcpTmWtVectPtr->twsTupleDev.tudDevPtr != tupleDevPtr->tudDevPtr)
         {
            retCode = TM_8BIT_NO;
         }
    }
#else /* !TM_USE_STRONG_ESL */
#endif /* !TM_USE_STRONG_ESL */
    return retCode;
}

#ifdef TM_6_USE_MIP_MN
/****************************************************************************
* FUNCTION: tfTcpBackOnLink
*
* PURPOSE:
*   This function is used to notify TCP of movement so that TCP can more
*   quickly recover from exponential backoff on send affecting TCP
*   retransmissions. TCP recovery essentially consists of immediately
*   sending 3 duplicate ACKS to the peer to trigger the peer fast
*   retransmit algorithm.
*
* PARAMETERS:
*   devPtr:
*       Points to interface on which the TCP sockets send/recv data.
*
* RETURNS:
*   Nothing
*
* NOTES:
*   This function is called by Mobile IPv6 Mobile Node move detection when
*   the MN detects that it was off-link and has now come back on-link, to
*   recover from the MIPv6 scenario "TCP Incorrectly Detects Congestion When
*   Off-Link":
*
*  During a period of link outage, which can occur when the MN moves from an
*  old link to a new link, TCP may attempt to transmit data packets, in which
*  case it will not receive the expected TCP acknowledgements from the peer.
*  Generally, this causes TCP to assume that routers are dropping the packets
*  it sends due to congestion, in which case it starts an exponential backoff
*  for retransmitting unacknowledged TCP data to give the network a chance to
*  recover from the congestion condition. As a consequence, TCP data transfer
*  in the send direction becomes very slow since the send window decreases to
*  a single segment, also TCP becomes quite unresponsive when the link is
*  finally back up (i.e. TCP may delay many seconds before retransmitting)
*  since the TCP retransmission timer period has increased exponentially.
*
****************************************************************************/
void tfTcpBackOnLink(ttDeviceEntryPtr devPtr)
{
    ttSocketEntryPtr    socketPtr;
    ttTcpVectPtr        tcpVectPtr;
    ttRteEntryPtr       rtePtr;
    int                 index;
    int                 i;
#ifdef TM_USE_BSD_DOMAIN
    int                 af;
#endif /* TM_USE_BSD_DOMAIN */

/*
 * We assume that the device driver will drop the packets if it is off link,
 * and return an error to the stack.
 */

/* For each TCP socket that would send/recv on the specified interface (i.e.
 * reference the routing entry device pointer), send 3 duplicate ACKs.
 * send 3 duplicate ACKs to activate the remote Fast Retransmit algorithm.
 * Also a TCP sender would piggyback data on the ACKs.
 */
    for ( index = 0;
          (tt32Bit)index < tm_context(tvMaxNumberSockets);
          index++ )
    {
#ifdef TM_USE_BSD_DOMAIN
        af = TM_BSD_FAMILY_INET;
#endif /* TM_USE_BSD_DOMAIN */
        socketPtr = tfSocketCheckValidLock( index
#ifdef TM_USE_BSD_DOMAIN
                                                , &af
#endif /* TM_USE_BSD_DOMAIN */
                                               );

        if (socketPtr != (ttSocketEntryPtr)0)
        {
            if (socketPtr->socProtocolNumber == IP_PROTOTCP)
            {
                tcpVectPtr = (ttTcpVectPtr)socketPtr;
                if (tcpVectPtr->tcpsState == TM_TCPS_ESTABLISHED)
                {
/* Refresh the cache if need be */
                    (void)tfTcpSetRtc(tcpVectPtr);
                    rtePtr = socketPtr->socRteCacheStruct.rtcRtePtr;
                    if (    (rtePtr != TM_RTE_NULL_PTR)
                         && (rtePtr->rteOrigDevPtr == devPtr) )
                    {
                        for (i = 0; i < 3; i++)
                        {
                            tcpVectPtr->tcpsFlags2 |= TM_TCPF2_ACK;
                            (void)tfTcpSendPacket(tcpVectPtr);
                            if (tcpVectPtr->tcpsState != TM_TCPS_ESTABLISHED)
                            {
                                break;
                            }
                        }
                    }
                }
            }
            (void)tfSocketReturn(socketPtr, index, TM_ENOERROR, 0);
        }
    }
    return;
}
#endif /* TM_6_USE_MIP_MN */
#ifdef TM_USE_SSL_CLIENT
void tfTcpSslConnected(ttTcpVectPtr tcpVectPtr)
{
    int errorCode;

    if (    (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags, TM_TCPF_WAIT_SND_FIN))
         && (    (tcpVectPtr->tcpsState == TM_TCPS_ESTABLISHED)
              || (tcpVectPtr->tcpsState == TM_TCPS_CLOSE_WAIT) )
         && (tm_16bit_all_bits_set(tcpVectPtr->tcpsSslFlags,
                                   TM_SSLF_SEND_ENABLED | TM_SSLF_ESTABLISHED))
       )
    {
/*
 * If we had postponed sending the FIN until we reach the established state,
 * send it now
 */
#ifdef TM_SNMP_MIB
/*
 * number of TCP connections for which the current state
 * is either ESTABLISHED or CLOSE-WAIT. We are moving to
 * FIN_WAIT_1 state or LAST_ACK.
 */
        tm_context(tvTcpData).tcpCurrEstab--;
#endif /* TM_SNMP_MIB */
        tm_16bit_clr_bit(tcpVectPtr->tcpsFlags, TM_TCPF_WAIT_SND_FIN);
        tcpVectPtr->tcpsState = TM_TCPS_FIN_WAIT_1;
        tcpVectPtr->tcpsFlags2 |= TM_TCPF2_SND_FIN;
        errorCode = tfTcpSslClose(tcpVectPtr, 1); /* abort on failure */
        if (errorCode == TM_ENOERROR)
        {
            (void)tfTcpSendPacket(tcpVectPtr);
        }
    }
    return;
}
#endif /* TM_USE_SSL_CLIENT */

#ifdef TM_USE_TCP_REXMIT_CONTROL
/****************************************************************************
* FUNCTION: tfTcpControlRexmitTimer
*
* PURPOSE:
*   This function is used when setsockopt is called with the option
*   TM_TCP_REXMIT_CONTROL to control the TCP Retransmission Timer.
*
* PARAMETERS:
*   tcpVectPtr:
*       Points to the TCP control block on which data is sent for the socket
*   function
*       1 = Pause, 2 = Resume, 3 = Reset
*
* RETURNS:
*   TM_ENOERROR when the function succeeds
*   TM_EINVAL if the parameters are not valid
*
* NOTES:
*   Function control passes through setsockopt and tfTcpSetOption
*
****************************************************************************/
int tfTcpControlRexmitTimer(ttTcpVectPtr tcpVectPtr,
                            int          fxn)
{
    int                 errorCode;
    tt32Bit             rto;
    tt32Bit             tcpMinRtoTime;
    tt32Bit             tcpMaxRtoTime;
    tt32Bit             probeWndInterval;

    errorCode = TM_ENOERROR;
    if (tcpVectPtr == (ttTcpVectPtr)0)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        switch(fxn)
        {
            case 1: /* Pause */
/* make sure the timer isn't already paused */
                if (tm_16bit_bits_not_set(tcpVectPtr->tcpsFlags3,
                                        TM_TCPF3_REXMIT_PAUSE))
                {
                    tcpVectPtr->tcpsFlags3 |= TM_TCPF3_REXMIT_PAUSE;

/* suspend the connection timeout timer */
                    if (tcpVectPtr->tcpsConnTmPtr != TM_TMR_NULL_PTR)
                    {
                        tcpVectPtr->tcpsConnTmLeftAtPause =
                            tfTimerMsecLeft(tcpVectPtr->tcpsConnTmPtr);
                        tm_tcp_timer_suspend(tcpVectPtr->tcpsConnTmPtr);
                    }

/* if we have an active retransmission timer, suspend it */
                    if (    (tcpVectPtr->tcpsReXmitTmPtr != (ttTimerPtr)0)
                        && (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags2,
                                                TM_TCPF2_REXMIT_MODE)))
                    {
                        tm_tcp_timer_suspend(tcpVectPtr->tcpsReXmitTmPtr);
                    }
                    else if (   (tcpVectPtr->tcpsWndProbeTmPtr != (ttTimerPtr)0)
                             && (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags3,
                                                      TM_TCPF3_PROBE_MODE)))
                    {
                        tm_tcp_timer_suspend(tcpVectPtr->tcpsWndProbeTmPtr);
                    }
                }
                break;

            case 2: /* Resume */
            case 3: /* Reset */
/* make sure the timer is paused */
                if (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags3,
                                        TM_TCPF3_REXMIT_PAUSE))
                {
                    tcpVectPtr->tcpsFlags3 &= ~TM_TCPF3_REXMIT_PAUSE;

/* compute all new reset values for the rexmit timer and probe timer */
                    if (fxn == 3)
                    {
                        tcpVectPtr->tcpsIdleTime = 0; /* reset idle time */

/* reset retransmission count */
                        tcpVectPtr->tcpsReXmitCnt = 0;

/* reset rexmit timer to TM_TCP_RTO_DEF */
                        tcpVectPtr->tcpsScSmRtt = (ttS32Bit)TM_UL(0);
                        tcpVectPtr->tcpsScRttVar = (ttS32Bit)(tcpVectPtr->tcpsRtoDef);

/* recompute initial retransmission timeout */
                        rto = tcpVectPtr->tcpsRtoDef;
                        tcpMinRtoTime = tcpVectPtr->tcpsRtoMin;
                        tcpMaxRtoTime = tcpVectPtr->tcpsRtoMax;
                        tm_bound(rto, tcpMinRtoTime, tcpMaxRtoTime);
                        tcpVectPtr->tcpsRto = rto;

/* recompute probe window interval */
                        probeWndInterval = tm_rto(tcpVectPtr->tcpsScSmRtt,
                                                  tcpVectPtr->tcpsScRttVar);
                        tm_bound(probeWndInterval,
                                tcpVectPtr->tcpsProbeMin,
                                tcpVectPtr->tcpsProbeMax);

/* if we have a rexmit timer valid, set the new time and next packet */
                        if (tcpVectPtr->tcpsReXmitTmPtr != (ttTimerPtr)0)
                        {
                            tm_timer_new_time(tcpVectPtr->tcpsReXmitTmPtr, rto);
                            tcpVectPtr->tcpsSndNxt = tcpVectPtr->tcpsSndUna;
                        }

/* if we have a probe timer, set the new time */
                        if (tcpVectPtr->tcpsWndProbeTmPtr != (ttTimerPtr)0)
                        {
                            tm_timer_new_time(tcpVectPtr->tcpsWndProbeTmPtr,
                                              tcpVectPtr->tcpsProbeMin);
                        }
/* reset connection timeout timer */
                        if (tcpVectPtr->tcpsConnTmPtr != TM_TMR_NULL_PTR)
                        {
                            tm_timer_new_time(tcpVectPtr->tcpsConnTmPtr,
                                              tcpVectPtr->tcpsMaxRt);
                        }
/* remember that we just reset - keeps us from messing up ConnTm */
                        tcpVectPtr->tcpsFlags3 |= TM_TCPF3_REXMIT_RESET;
                    }

/* figure out which to resume - only one is valid at a time */
                    if (    (tcpVectPtr->tcpsReXmitTmPtr != (ttTimerPtr)0)
                         && (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags2,
                                                  TM_TCPF2_REXMIT_MODE)))
                    {
                        tm_tcp_timer_resume(tcpVectPtr->tcpsReXmitTmPtr);
                        tfTcpStateMachine(tcpVectPtr, TM_PACKET_NULL_PTR, TM_TCPE_REXMIT);
                    }

/* not retransmitting, probing - resume the probe timer */
                    else if (   (tcpVectPtr->tcpsWndProbeTmPtr != (ttTimerPtr)0)
                         && (tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags3,
                                                  TM_TCPF3_PROBE_MODE)))
                    {
                        tm_tcp_timer_resume(tcpVectPtr->tcpsWndProbeTmPtr);
                    }

/* always resume the active connection timeout timer */
                    if (tcpVectPtr->tcpsConnTmPtr != TM_TMR_NULL_PTR)
                    {
                        if (fxn == 2)
                        {
                            tm_timer_new_time(
                                tcpVectPtr->tcpsConnTmPtr,
                                tcpVectPtr->tcpsConnTmLeftAtPause);
                        }
                        tm_tcp_timer_resume(tcpVectPtr->tcpsConnTmPtr);
                    }
                }
                break;

            default: /* Invalid */
                errorCode = TM_EINVAL;
                break;
        }
    }
    return errorCode;
}
#endif /* TM_USE_TCP_REXMIT_CONTROL */

#else /* !TM_USE_TCP */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */


/* To allow link for builds when TM_USE_TCP is not defined */
int tvTcpDummy = 0;

#endif /* !TM_USE_TCP */
