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
 * Filename: trtcpopt.c
 * Author: Odile
 * Date Created: 06/24/98
 * $Source: source/trtcpopt.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:48:48JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_TCP
#include <trtype.h>
#include <trproto.h>
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
#include <trssl.h>
#endif /* TM_USE_SSL_CLIENT || TM_USE_SSL_SERVER */

/*
 * tfTcpSetOption() function description:
 * This function lets the user set TCP level options for a
 * given connection.
 * Supported option names:
 *  TM_TCP_SEL_ACK*: Turn on/off selective acknowledgement TCP header option
 *  TM_TCP_WND_SCALE*: Turn on/off Window scaling TCP header option
 *  TM_TCP_TS*: Turn on/off Time stamp TCP header option
 *  TCP_NODELAY: Turn on/off disabling Nagle algorithm
 *  TCP_NOPUSH: Turn on/off non usage of Push bit
 *  TM_TCP_SLOW_START: Turn on/off slow start algorithm
 *  TM_TCP_PUSH_ALL: Turn on/off push on every segment.
 *    If content of cell pointed to by optionValuePtr is non zero, those
 *    options will be turned on, otherwise they will be turned off.
 *  TCP_MAXSEG*
 *    Content of cell pointed to by optionValuePtr contains the
 *    MSS for our connection that the user wants to use (instead of the
 *    default).
 *  TM_TCP_DELAY_ACK
 *    Content of cell pointed to by optionValuePtr contains the delay
 *    ACK time value for the connection that the user wants to use.
 *
 *
 * Options marked with an asterix can only be set prior to a connection
 * establishement. optionValuePtr point to an int, except for the
 * TM_TCP_DELAY_ACK optionName, in which case it points to an unsigned long.
 *
 * Parameter       description
 * tcpVectPtr      pointer to TCP state vector/socket entry
 * optionName      one of the option names described above
 * optionValuePtr  Pointer to cell as described above
 * optionLen       length of cell pointed to by optionValuePtr.
 *
 * Return value.
 * TM_ENOERROR    no error
 * TM_EPERM       if user tries and set an option that can only be set
 *                prior to a connection establishment, and connection has
 *                not been established
 * TM_EINVAL      error in one or more of the parameters
 * TM_ENOPROTOOPT unsupported option name
 *
 */
int tfTcpSetOption (ttTcpVectPtr     tcpVectPtr,
                    int              optionName,
                    ttConstCharPtr   optionValuePtr,
                    int              optionLength)
{
    tt16Bit         mss;
    int             optionValue;
    int             errorCode;
#if (defined(TM_USE_SSL_SERVER) || defined(TM_USE_SSL_CLIENT))
    ttSslSessionPtr sslSessionPtr;
#endif /* (defined(TM_USE_SSL_SERVER) || defined(TM_USE_SSL_CLIENT)) */

    errorCode = TM_ENOERROR;
    switch (optionName)
    {
/* ON/OFF flags */
#ifdef TM_TCP_SACK
        case TM_TCP_SEL_ACK:
#endif /* TM_TCP_SACK */
        case TM_TCP_WND_SCALE:
        case TM_TCP_TS:
/* These options can only be changed prior to our sending a SYN */
            if (tcpVectPtr->tcpsState >= TM_TCPS_LISTEN )
            {
                errorCode = TM_EPERM;
                goto setOptionExit;
            }
/* Fall through: execute common ON/OFF code */
        case TCP_NODELAY:
        case TCP_NOPUSH:
        case TM_TCP_SLOW_START:
        case TCP_STDURG:
#ifdef TM_USE_TCP_PACKET
        case TM_TCP_PACKET:
#endif /* TM_USE_TCP_PACKET */
            if (optionLength != (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                if (*((ttIntPtr)optionValuePtr) == 0) /* off */
                {
                    tcpVectPtr->tcpsFlags &= ~(tt16Bit)optionName;
                }
                else
                {
#if (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER))
#ifdef TM_USE_TCP_PACKET
                    if((optionName == TM_TCP_PACKET) &&
                             tm_16bit_one_bit_set(tcpVectPtr->tcpsSslFlags,
                             TM_SSLF_CLIENT | TM_SSLF_SERVER))
                    {
/* SSL is already enabled, we can't support TM_TCP_PACKET together */
                        errorCode = TM_EINVAL;
                    }
                    else
#endif /* TM_USE_TCP_PACKET */
#endif /* (defined(TM_USE_SSL_CLIENT) || defined(TM_USE_SSL_SERVER)) */
                    {
                        tcpVectPtr->tcpsFlags |= (tt16Bit)optionName;
                    }
                }
            }
            break;
        case TCP_MAXSEG:
            if (optionLength != (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                mss = (tt16Bit) (tt16Bit)(*((ttIntPtr)optionValuePtr));
/* Value can only be changed prior to our sending a SYN */
                if ( tcpVectPtr->tcpsState >= TM_TCPS_LISTEN )
                {
                        errorCode = TM_EPERM;
                }
                else
                {
                    tcpVectPtr->tcpsMSS = mss;
                    tm_16bit_clr_bit( tcpVectPtr->tcpsFlags,
                                      TM_TCPF_PMTU_DISCOVERY );
                }
            }
            break;
       case TCP_MAXRT:
            if (optionLength != (int)sizeof(int))
            {
                errorCode = TM_EINVAL;
            }
            else
            {
                optionValue = (*((ttIntPtr)optionValuePtr));
                if (optionValue == -1) /* wait forever */
                {
                    tcpVectPtr->tcpsFlags |= TM_TCPF_RT_FOREVER;
                }
                else
                {
                    if (optionValue != 0)
                    {
                        tm_16bit_clr_bit( tcpVectPtr->tcpsFlags,
                                          TM_TCPF_RT_FOREVER );
                        tcpVectPtr->tcpsFlags |= TCP_MAXRT;
                        tcpVectPtr->tcpsMaxRt =
                                             (tt32Bit)optionValue * TM_UL(1000);
                    }
                    else
                    {
/* Use default */
                        tm_16bit_clr_bit( tcpVectPtr->tcpsFlags,
                                          (TM_TCPF_RT_FOREVER | TCP_MAXRT) );
                    }
                }
            }
            break;
       case TCP_KEEPALIVE:
            if (optionLength != (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                optionValue = (*((ttIntPtr)optionValuePtr));
                if (optionValue < (tcpVectPtr->tcpsIdleIntvTime * 
                                            tcpVectPtr->tcpsKeepAliveProbeCnt))
                {
                    errorCode=TM_EINVAL;
                }
                else
                {
                    tcpVectPtr->tcpsKeepAliveTime = optionValue;
                }
            }
            break;
       case TM_TCP_DELAY_ACK:
            if (optionLength != (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                tcpVectPtr->tcpsDelayAckTime = (tt32Bit)
                                             (*((ttIntPtr)optionValuePtr));
            }
            break;
        case TM_TCP_MAX_REXMIT:
/* Change Maximum number of retransmissions (default 12) */
            if (optionLength != (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                tcpVectPtr->tcpsMaxReXmitCnt = (*((ttIntPtr)optionValuePtr));
            }
            break;
        case TM_TCP_KEEPALIVE_INTV:
/*
 * Change keep alive interval probes (default 75 seconds). Can only be
 * changed prior to our sending a SYN
 */
            if (tcpVectPtr->tcpsState >= TM_TCPS_LISTEN )
            {
                errorCode = TM_EPERM;
            }
            else
            {
                if (optionLength != (int)sizeof(int))
                {
                    errorCode=TM_EINVAL;
                }
                else
                {
                    optionValue = (*((ttIntPtr)optionValuePtr));
                    if ( (optionValue == 0) ||
                         (optionValue < 0) ||
/* Do not allow bigger interval than 10 minutes */
                         (optionValue > TM_TCP_KEEPALIVE_TIME/12) ||
                         (tcpVectPtr->tcpsKeepAliveTime < 
                            (optionValue * tcpVectPtr->tcpsKeepAliveProbeCnt)) )
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        tcpVectPtr->tcpsIdleIntvTime = optionValue;
                    }
                }
            }
            break;
        case TM_TCP_KEEPALIVE_CNT:
/*
 * Change keep alive probe count (default 8).
 */
            if (optionLength != (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                optionValue = (*((ttIntPtr)optionValuePtr));
                if ( (optionValue < 0) ||
                     (tcpVectPtr->tcpsKeepAliveTime < 
                               (tcpVectPtr->tcpsIdleIntvTime * optionValue)) )
                {
                    errorCode = TM_EINVAL;
                }
                else
                {
                    tcpVectPtr->tcpsKeepAliveProbeCnt = optionValue;
                }
            }
            break;
        case TM_TCP_FINWT2TIME:
/*
 * Change Amount of time to stay in FIN WAIT 2 when socket has been closed
 * (10 minutes) in seconds.
 */
            if (optionLength != (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                tcpVectPtr->tcpsFinWt2Time = (*((ttIntPtr)optionValuePtr));
            }
            break;
        case TM_TCP_2MSLTIME:
/*
 * Change TIME WAIT TIME (Default 2*Maximum segment life time (2*30 seconds))
 */
            if (optionLength != (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                tcpVectPtr->tcps2MSLTime = (*((ttIntPtr)optionValuePtr));
            }
            break;
        case TM_TCP_RTO_DEF:
/*
 * Change Default retransmission timeout value in milliseconds (default
 * 3,000 milliseconds)
 */
            if (optionLength != (int)sizeof(ttUser32Bit))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                tcpVectPtr->tcpsRtoDef = (*((tt32BitPtr)optionValuePtr));
            }
            break;
        case TM_TCP_RTO_MIN:
/*
 * Change minimum retransmission timeout value in milliseconds (default
 * 100 milliseconds)
 */
            if (optionLength != (int)sizeof(ttUser32Bit))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                tcpVectPtr->tcpsRtoMin = (*((tt32BitPtr)optionValuePtr));
            }
            break;
        case TM_TCP_RTO_MAX:
/*
 * Change maximum retransmission timeout value in milliseconds (default
 * 64,000 milliseconds)
 */
            if (optionLength != (int)sizeof(ttUser32Bit))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                tcpVectPtr->tcpsRtoMax = (*((tt32BitPtr)optionValuePtr));
            }
            break;
        case TM_TCP_PROBE_MIN:
/*
 * Change minimum zero window probe timeout value in milliseconds (default 500
 * milliseconds)
 */
            if (optionLength != (int)sizeof(ttUser32Bit))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                tcpVectPtr->tcpsProbeMin = (*((tt32BitPtr)optionValuePtr));
            }
            break;
        case TM_TCP_PROBE_MAX:
/*
 * Change maximum zero window probe timeout value in milliseconds (default
 * 60,000 milliseconds)
 */
            if (optionLength != (int)sizeof(ttUser32Bit))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                tcpVectPtr->tcpsProbeMax = (*((tt32BitPtr)optionValuePtr));
            }
            break;

#ifdef TM_USE_TCP_REXMIT_CONTROL
        case TM_TCP_REXMIT_CONTROL:
/* This allows the user to pause, resume, and reset the TCP rexmit timer */
            if (optionLength != (int)sizeof(int))
            {
                errorCode = TM_EINVAL;
            }
            else if (    ((*((ttIntPtr)optionValuePtr)) < 1)
                      || ((*((ttIntPtr)optionValuePtr)) > 3))
            {
                errorCode = TM_EINVAL;
            }
            else
            {
                errorCode = tfTcpControlRexmitTimer(tcpVectPtr, 
                                                *((ttIntPtr)optionValuePtr));
            }
            break;
#endif /* TM_USE_TCP_REXMIT_CONTROL */

#if (defined(TM_USE_SSL_SERVER) || defined(TM_USE_SSL_CLIENT))
#ifdef TM_USE_SSL_CLIENT
        case TM_TCP_SSL_CLIENT:
#endif /*TM_USE_SSL_CLIENT */
#ifdef TM_USE_SSL_SERVER
        case TM_TCP_SSL_SERVER:
#endif /*TM_USE_SSL_SERVER */

            if (optionLength != (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                if (*((ttIntPtr)optionValuePtr))
                {
#ifdef TM_USE_TCP_PACKET
                    if(tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags,
                                            TM_TCPF_PACKET ))
                    {
/* TCP_PACKET is already enabled, we can't support TM_TCP_SSL together */
                        errorCode = TM_EINVAL;
                    }
                    else
#endif /* TM_USE_TCP_PACKET */
                    {
                        if(tm_context(tvSslTlsPtr) == (ttVoidPtr)0)
                        {
                            errorCode = TM_EINVAL;
                        }
                        else
                        {
#ifdef TM_USE_SSL_SERVER
                            if(optionName == TM_TCP_SSL_SERVER)
                            {
                                tcpVectPtr->tcpsSslFlags |= TM_SSLF_SERVER;
                            }
#endif /*TM_USE_SSL_SERVER*/
#ifdef TM_USE_SSL_CLIENT
                            if(optionName == TM_TCP_SSL_CLIENT)
                            {
                                tcpVectPtr->tcpsSslFlags |= TM_SSLF_CLIENT;
                            }
#endif /*TM_USE_SSL_CLIENT*/
                            if(!tcpVectPtr->tcpsSslConnStatePtr)
                            {
/* BUG ID 1491 : tcpsSslConnStatePtr could be non-NULL if user sets this 
 * option multiple times. For example, user sets TM_TCP_SSL_SERVER option
 * for the listening socket, and then user sets this option on accepted 
 * socket again, which is unnecessory. 
 */
                                tcpVectPtr->tcpsSslConnStatePtr = 
                                       tm_get_raw_buffer
                                       (sizeof(ttSslConnectState));
                                if(!tcpVectPtr->tcpsSslConnStatePtr)
                                {
/* We don't know if this is going to be a listening socket or not. 
 * For a listening socket, we are going to free it when listen is called. 
 * And a new connect state will be allocated whenever you call 
 * tfSslServerCreateState inside tfTcpConReq
 */
                                    errorCode = TM_ENOBUFS;
                                }
                                else
                                {
                                    tm_bzero(tcpVectPtr->tcpsSslConnStatePtr, 
                                        sizeof(ttSslConnectState));
/* we set a default send threshold and send ceiling */
                                    tcpVectPtr->tcpsSslSendMinSize = 
                                        TM_SSL_SEND_DATA_MIN_SIZE;
                                    tcpVectPtr->tcpsSslSendMaxSize = 
                                        TM_SSL_SEND_DATA_MAX_SIZE;
                                }
                            }
                        }
                    }
                }
            }
            break;
        case TM_TCP_SSLSESSION:
            if(optionLength != (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                optionValue = (*((ttIntPtr)optionValuePtr));
                sslSessionPtr = tfSslGetSessionUsingIndex(optionValue);
                if(sslSessionPtr == (ttVoidPtr)0)
                {
/* we need to test if the mentioned sslsession already exists or not, and
 * we have only 16 bit maximum to store the session number
 */
                    errorCode = TM_EINVAL;
                }
                else
                {
                    tcpVectPtr->tcpsSslSession = (tt16Bit)(optionValue);
/* set this flag to indicate that we have done session-setting*/
                    tcpVectPtr->tcpsSslFlags |= TM_SSLF_SESSION_SET;
                    tfSslSessionReturn(sslSessionPtr);
                }
            }
            break;
        case TM_TCP_SSL_SEND_MIN_SIZE:
            if(optionLength != (int)sizeof(int))
            {
                errorCode = TM_EINVAL;
            }
            else
            {
                optionValue = (*((ttIntPtr)optionValuePtr));
#ifndef TM_INT_LE16
                if ((optionValue > 0xffff) || (optionValue < 0))
                {
/* we have only 16 bit maximum to store the minimum size */
                    errorCode = TM_EINVAL;
                }
                else
#endif /* TM_INT_LE16 */
                {
                    tcpVectPtr->tcpsSslSendMinSize = (tt16Bit)(optionValue);
                }
            }
            break;
/* each ssl record should contain at most that much user data. The default
 * value is our send queue size
 */
        case TM_TCP_SSL_SEND_MAX_SIZE:
            if(optionLength != (int)sizeof(int))
            {
                errorCode = TM_EINVAL;
            }
            else
            {
                optionValue = (*((ttIntPtr)optionValuePtr));
                if(
/* BUG ID 1494: We should not allow more than 0x4000 be set as this value */
                    (optionValue > 0x4000) ||
                    (optionValue < 100) )
                {
/* we have only 16 bit maximum to store the maximum size according to SSL RFCs,
 * and we don't want this max size to be too small either
 */
                    errorCode = TM_EINVAL;
                }
                else
                {
                    tcpVectPtr->tcpsSslSendMaxSize = (tt16Bit)(optionValue);
                }
            }
            break;
        case TM_TCP_SSL_USER_PROCESS:
            if (optionLength != (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                optionValue = (*((ttIntPtr)optionValuePtr));
                if (optionValue == 0)
                {
                    tm_16bit_clr_bit( tcpVectPtr->tcpsSslFlags, 
                                      TM_SSLF_USER_HANDSHK_PROCESS);
                }
                else
                {
/* Offload SSL handshake process to the user application task */
                    tcpVectPtr->tcpsSslFlags |= TM_SSLF_USER_HANDSHK_PROCESS;
                }
            }
            break;
#endif /* TM_USE_SSL_SERVER || TM_USE_SSL_CLIENT */
        case TM_TCP_PEND_ACCEPT_RECV_WND:
/*
 * Specify window size on listening socket. This will change to value
 * specified by SO_RCVBUF when accept() is called and new socket is spawned.
*/
            if (optionLength != (int)sizeof(ttUser32Bit))
            {
/* Check option length */
                errorCode = TM_EINVAL;
            }
            else
            {
                if (   (     (*((tt32BitPtr)optionValuePtr))
                           > tcpVectPtr->tcpsSocketEntry.socMaxRecvQueueBytes)
                     || (tcpVectPtr->tcpsState != TM_TCPS_LISTEN ) )
                {
/*
 * 1. User cannot increase the maximum queue size (to prevent shrinking of
 *    the recv window when we switch to the real recv window,
 *    (as recommended by RFC1122)).
 * 2. Option only allowed on listening socket
 */
                    errorCode = TM_EPERM;
                }
                else
                {
                    tcpVectPtr->tcpsMaxRecvQueueBytes =
                                            (*((tt32BitPtr)optionValuePtr));
                    tcpVectPtr->tcpsFlags3 |= TM_TCPF3_LISTEN_RECV_WINDOW;
                }
            }
            break;
#ifdef TM_USE_USER_PARAM
        case TM_TCP_USER_PARAM:
/*
 * Set the user-defined TCP connection specific data.
 */
            if (optionLength != (int)sizeof(ttUserGenericUnion))
            {
                errorCode = TM_EINVAL;
            }
            else
            {
                tm_bcopy(optionValuePtr, &tcpVectPtr->tcpsUserParam,
                                        sizeof(ttUserGenericUnion));
            }
            break;
#endif /* TM_USE_USER_PARAM */
        default:
            errorCode = TM_ENOPROTOOPT;
            break;
    }
setOptionExit:
    return errorCode;
}

/*
 * tfTcpGetOption() function description:
 * This function lets the user get TCP level options for a
 * given connection.
 * Supported option names:
 *  TM_TCP_SEL_ACK: Turn on/off selective acknowledgement TCP header option
 *  TM_TCP_WND_SCALE: Turn on/off Window scaling TCP header option
 *  TM_TCP_TS: Turn on/off Time stamp TCP header option
 *  TCP_NODELAY: Turn on/off disabling Nagle algorithm
 *  TCP_NOPUSH: Turn on/off non usage of Push bit when send queue is emptied.
 *  TM_TCP_SLOW_START: Turn on/off slow start algorithm
 *  TM_TCP_PUSH_ALL: Turn on/off setting push bit on every segment sent.
 *    If content of cell pointed to by optionValuePtr is non zero, those
 *    options are on, otherwise they are off.
 *  TCP_MAXSEG
 *    Content of cell pointed to by optionValuePtr contains the current
 *    MSS for our connection.
 *  TM_TCP_DELAY_ACK
 *    Content of cell pointed to by optionValuePtr contains the current delay
 *    ACK time value for the connection.
 *
 * optionValuePtr point to an int, except for the
 * TM_TCP_DELAY_ACK optionName, in which case it points to an unsigned long.
 *
 * Parameter       description
 * tcpVectPtr      pointer to TCP state vector/socket entry
 * optionName      one of the option names described above
 * optionValuePtr  Pointer to cell as described above
 * optionLenPtr    Pointer to length of cell pointed to by optionValuePtr.
 *
 * Return value.
 * TM_ENOERROR    no error
 * TM_EINVAL      error in one or more of the parameters
 * TM_ENOPROTOOPT unsupported option name
 *
 */
int tfTcpGetOption (ttTcpVectPtr   tcpVectPtr,
                    int            optionName,
                    ttCharPtr      optionValuePtr,
                    ttIntPtr       optionLengthPtr)
{
    int     errorCode;
    int     optionValue;

    errorCode = TM_ENOERROR;
    switch (optionName)
    {
/* ON/OFF flags */
#ifdef TM_TCP_SACK
        case TM_TCP_SEL_ACK:
#endif /* TM_TCP_SACK */
        case TM_TCP_WND_SCALE:
        case TM_TCP_TS:
        case TCP_NODELAY:
        case TCP_NOPUSH:
        case TM_TCP_SLOW_START:
        case TCP_STDURG:
        case TM_TCP_PUSH_ALL:
            if ((*optionLengthPtr) < (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                if (tcpVectPtr->tcpsFlags & optionName) /* on */
                {
                    *((ttIntPtr)optionValuePtr) = 1;
                }
                else /* off */
                {
                    *((ttIntPtr)optionValuePtr) = 0;
                }
                *optionLengthPtr = sizeof(int);
            }
            break;
         case TCP_MAXSEG:
            if ((*optionLengthPtr) < (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                *optionLengthPtr = sizeof(int);
                (*((ttIntPtr)optionValuePtr)) = (int)tcpVectPtr->tcpsMSS;
            }
            break;
         case TM_TCP_EFF_MAXSEG:
            if ((*optionLengthPtr) < (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                *optionLengthPtr = sizeof(int);
                (*((ttIntPtr)optionValuePtr)) = (int)tcpVectPtr->tcpsEffSndMss;
            }
            break;
       case TCP_MAXRT:
            if ((*optionLengthPtr) < (int)sizeof(int))
            {
                errorCode = TM_EINVAL;
            }
            else
            {
                if ( tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags,
                                          TM_TCPF_RT_FOREVER) )
                {
/* wait forever */
                    optionValue = -1;
                }
                else
                {
                    if ( tm_16bit_one_bit_set(tcpVectPtr->tcpsFlags,
                                              TCP_MAXRT) )
                    {
                        optionValue = (int)(tcpVectPtr->tcpsMaxRt/1000);
                    }
                    else
                    {
                        optionValue = 0;
                    }
                }
                *optionLengthPtr = sizeof(int);
                (*((ttIntPtr)optionValuePtr)) = optionValue;
            }
            break;
       case TCP_KEEPALIVE:
            if ((*optionLengthPtr) < (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                (*((ttIntPtr)optionValuePtr)) =
                                          (int)tcpVectPtr->tcpsKeepAliveTime;
                *optionLengthPtr = sizeof(int);
            }
            break;
       case TM_TCP_DELAY_ACK:
            if ((*optionLengthPtr) < (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                (*((ttIntPtr)optionValuePtr)) =
                                            (int)tcpVectPtr->tcpsDelayAckTime;
                *optionLengthPtr = sizeof(int);
            }
            break;
        case TM_TCP_MAX_REXMIT:
/* Get Maximum number of retransmissions (default 12) */
            if ((*optionLengthPtr) < (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                (*((ttIntPtr)optionValuePtr)) = tcpVectPtr->tcpsMaxReXmitCnt;
                *optionLengthPtr = sizeof(int);
            }
            break;
        case TM_TCP_KEEPALIVE_INTV:
/*
 * Get keep alive interval probes (default 75 seconds).
 */
            if ((*optionLengthPtr) < (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                (*((ttIntPtr)optionValuePtr)) = tcpVectPtr->tcpsIdleIntvTime;
                *optionLengthPtr = sizeof(int);
            }
            break;
        case TM_TCP_KEEPALIVE_CNT:
/*
 * Get keep alive probe count (default 8).
 */
            if ((*optionLengthPtr) < (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                (*((ttIntPtr)optionValuePtr)) =
                                            tcpVectPtr->tcpsKeepAliveProbeCnt;
                *optionLengthPtr = sizeof(int);
            }
            break;
        case TM_TCP_FINWT2TIME:
/*
 * Change Amount of time to stay in FIN WAIT 2 when socket has been closed
 * (10 minutes) in seconds.
 */
            if ((*optionLengthPtr) < (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                (*((ttIntPtr)optionValuePtr)) = tcpVectPtr->tcpsFinWt2Time;
                *optionLengthPtr = sizeof(int);
            }
            break;
        case TM_TCP_2MSLTIME:
/*
 * Change TIME WAIT TIME (Default 2*Maximum segment life time (2*30 seconds))
 */
            if ((*optionLengthPtr) < (int)sizeof(int))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                (*((ttIntPtr)optionValuePtr)) = tcpVectPtr->tcps2MSLTime;
                *optionLengthPtr = sizeof(int);
            }
            break;
        case TM_TCP_RTO_DEF:
/*
 * Change Default retransmission timeout value in milliseconds (default
 * 3,000 milliseconds)
 */
            if ((*optionLengthPtr) < (int)sizeof(ttUser32Bit))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                (*((tt32BitPtr)optionValuePtr)) = tcpVectPtr->tcpsRtoDef;
                *optionLengthPtr = sizeof(int);
            }
            break;
        case TM_TCP_RTO_MIN:
/*
 * Change minimum retransmission timeout value in milliseconds (default
 * 100 milliseconds)
 */
            if ((*optionLengthPtr) < (int)sizeof(ttUser32Bit))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                (*((tt32BitPtr)optionValuePtr)) = tcpVectPtr->tcpsRtoMin;
                *optionLengthPtr = sizeof(ttUser32Bit);
            }
            break;
        case TM_TCP_RTO_MAX:
/*
 * Change maximum retransmission timeout value in milliseconds (default
 * 64,000 milliseconds)
 */
            if ((*optionLengthPtr) < (int)sizeof(ttUser32Bit))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                (*((tt32BitPtr)optionValuePtr)) = tcpVectPtr->tcpsRtoMax;
                *optionLengthPtr = sizeof(ttUser32Bit);
            }
            break;
        case TM_TCP_PROBE_MIN:
/*
 * Change minimum zero window probe timeout value in milliseconds (default 500
 * milliseconds)
 */
            if ((*optionLengthPtr) < (int)sizeof(ttUser32Bit))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                (*((tt32BitPtr)optionValuePtr)) = tcpVectPtr->tcpsProbeMin;
                *optionLengthPtr = sizeof(ttUser32Bit);
            }
            break;
        case TM_TCP_PROBE_MAX:
/*
 * Change maximum zero window probe timeout value in milliseconds (default
 * 60,000 milliseconds)
 */
            if ((*optionLengthPtr) < (int)sizeof(ttUser32Bit))
            {
                errorCode=TM_EINVAL;
            }
            else
            {
                (*((tt32BitPtr)optionValuePtr)) = tcpVectPtr->tcpsProbeMax;
                *optionLengthPtr = sizeof(ttUser32Bit);
            }
            break;
        case TM_TCP_PEND_ACCEPT_RECV_WND:
/*
 * Specify window size on listening socket. This will change to value
 * specified by SO_RCVBUF when accept()is called and new socket is made
 * available to the user.
 */
            if ((*optionLengthPtr) < (int)sizeof(ttUser32Bit))
            {
                errorCode = TM_EINVAL;
            }
            else
            {
                (*((tt32BitPtr)optionValuePtr)) =
                                        tcpVectPtr->tcpsMaxRecvQueueBytes;
                *optionLengthPtr = sizeof(ttUser32Bit);
            }
            break;

#ifdef TM_USE_TCP_REXMIT_CONTROL
        case TM_TCP_REXMIT_CONTROL:
/* This allows the user to pause, resume, and reset the TCP rexmit timer */
            if ((*optionLengthPtr) < (int)sizeof(int))
            {
                errorCode = TM_EINVAL;
            }
            else if (tcpVectPtr->tcpsFlags3 & TM_TCPF3_REXMIT_PAUSE)
            {
                (*((ttIntPtr)optionValuePtr)) = (tt32Bit)1;
            }
            else
            {
                (*((ttIntPtr)optionValuePtr)) = (tt32Bit)2;
            }
            break;
#endif /* TM_USE_TCP_REXMIT_CONTROL */
#ifdef TM_USE_USER_PARAM
        case TM_TCP_USER_PARAM:
/*
 * Get the user-defined TCP connection specific data.
 */
            if (*optionLengthPtr < (int)sizeof(ttUserGenericUnion))
            {
                errorCode = TM_EINVAL;
            }
            else
            {
                tm_bcopy(&tcpVectPtr->tcpsUserParam, optionValuePtr,
                                        sizeof(ttUserGenericUnion));
                *optionLengthPtr = sizeof(ttUserGenericUnion);
            }
            break;
#endif /* TM_USE_USER_PARAM */
       default:
            errorCode = TM_ENOPROTOOPT;
            break;
    }
    return errorCode;
}
#else /* !TM_USE_TCP */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */


/* To allow link for builds when TM_USE_TCP is not defined */
int tvTcpOptDummy = 0;

#endif /* !TM_USE_TCP */
