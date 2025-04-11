/*
 * Copyright Notice:
 * Copyright Zuken Elmic Japan   1997 - 2016
 * No portions or the complete contents of this file may be copied or
 * distributed in any form (including but not limited to printed or 
 * electronic forms) without the expressed written consent of 
 * Zuken Elmic. Copyright laws and International 
 * Treaties protect the contents of this file.  Unauthorized 
 * use is prohibited.
 * All rights reserved.
 *
 * Description: SNTP Client (only unicast mode) implementation
 *
 * Filename: trsntpcl.c
 * Author: 
 * Date Created: 6/15/2010
 * $Source: source/trsntpl.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2015/07/21 11:05:01JST $
 * $Author: 
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/* 
 * Header files
 */
#include <trsystem.h> 
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

#include <trsntp.h>

/*
 * Local function prototypes
 */

/* 
 * Internal function to fill destination socket address 
 * storage structure 
 */ 

/* local variable definitions */

/* static const tables */

/* macro definitions */

/*
 * tfSntpGetTimeByUnicast
 * Function Description
 *  This function does work only unicast mode.
 * 
 * Parameters
 * Parameter            Description
 * blockingMode         Specifies whether the resolver should operate in 
 *                      blocking or non-blocking mode.
 *                       (TM_BLOCKING_ON or TM_BLOCKING_OFF)
 * socketDescPtr        Pointer to SNTP socket.
 * stateFlagPtr         Status flas. 
 *                       (TM_SNTP_STATUS_SEND or TM_SNTP_STATUS_RECV)
 * ipAddrPtr            Pointer to structure set an IP address and an address.
 *                      family of SNTP server.
 * 
 * Returns
 * Value                Meaning
 * TM_ENOERROR          Success
 * TM_EINVAL            Invalid parameters passed to function
 * 
 */
int tfSntpGetTimeByUnicast(
                    int                  blockingMode,
                    int *                socketDescPtr,
                    int *                stateFlagPtr,
                    ttSockAddrStoragePtr ipAddrPtr )
{
    int                     errorCode;
    int                     errCode;
    int                     af_lock;
#ifdef TM_USE_REUSEPORT
    int                     tempInt;
#endif /* TM_USE_REUSEPORT*/
    int                     selret;
    int                     addressLength;
    tt32Bit                 cur_time;
    tt32Bit                 ntp_time;
    tt32Bit                 xmt_time;
    tt32Bit                 rch_time;
    tt32Bit                 days;
    tt32Bit                 ttt;
    struct sockaddr_storage sockname;
    ttNtpPacket             tlNtpPacket;
    fd_set                  rdps;
    struct timeval          waittime;
#ifndef TM_USE_SNTP_MSEC
    tt32Bit                 ttr;
    tt32Bit                 tta;
#else  /* TM_USE_SNTP_MSEC */
    tt32Bit                 cur_mtime;
    tt32Bit                 ntp_mtime;
    tt32Bit                 xmt_mtime;
    tt32Bit                 rch_mtime;
    tt32Bit                 mttt;
#endif /* TM_USE_SNTP_MSEC */

/* Initialize here, just to avoid compiler warnings. */
    errorCode = TM_ENOERROR;
    errCode   = 0;

/* 
 * Verify that the sockaddr_strage structer of server and 
 * specified server family are valid and so on.
 */
    if(        ( socketDescPtr == (int *)0 )
            || ( stateFlagPtr == (int *)0 )
            || ( ipAddrPtr == (ttSockAddrStoragePtr)0 )
            || (
#ifdef TM_USE_IPV4
                  ipAddrPtr->ss_family != AF_INET
#endif /* TM_USE_IPV4*/
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
                &&
#endif /* TM_USE_IPV4*/
                  ipAddrPtr->ss_family != AF_INET6
#endif /* TM_USE_IPV6 */
                )
            || (    blockingMode != TM_BLOCKING_OFF
                &&  blockingMode != TM_BLOCKING_ON )
        )
    {
        errorCode = TM_EINVAL;
        goto sntpGetTimeByUnicastExit;
    }

/* Transmission processing of SNTP request packet */
    if ( *stateFlagPtr == TM_SNTP_STATUS_SEND)
    {
        if( *socketDescPtr == TM_SOCKET_ERROR )
        {
            /* Set the port number of sntp */
            ipAddrPtr->ss_port = htons((unsigned short)TM_SNTP_PORT);

            /* Create a socket with UDP mode. */
#ifdef TM_USE_IPV4
            if( ipAddrPtr->ss_family == AF_INET )
            {
                ipAddrPtr->ss_len = sizeof(struct sockaddr_in);
            }
            else
#endif /* TM_USE_IPV4*/
            {
#ifdef TM_USE_IPV6
                ipAddrPtr->ss_len = sizeof(struct sockaddr_in6);
#endif /* TM_USE_IPV6 */
            }
            *socketDescPtr = 
                    socket(ipAddrPtr->ss_family, SOCK_DGRAM, 0);

            if(*socketDescPtr == TM_SOCKET_ERROR)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfSntpGetTimeByUnicast", 
                        "Failed to create a socket\n");
#endif /* TM_ERROR_CHECKING */
                errorCode = tfGetSocketError(*socketDescPtr);
                goto sntpGetTimeByUnicastExit;
            }

            /* Set the blocking mode. */
            if(blockingMode == TM_BLOCKING_OFF)
            {
                af_lock = 1;
            }
            else if(blockingMode == TM_BLOCKING_ON)
            {
                af_lock = 0;
            }
            errorCode = tfIoctl(*socketDescPtr, FIONBIO, &af_lock);
            if (errorCode == TM_SOCKET_ERROR )
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfSntpGetTimeByUnicast", 
                                "Failed to specfy a blocking mode\n");
#endif /* TM_ERROR_CHECKING */
                errorCode = tfGetSocketError(*socketDescPtr);
                goto sntpGetTimeByUnicastExit;
            }

#ifdef TM_USE_REUSEPORT
            tempInt = 1;
            setsockopt(*socketDescPtr, SOL_SOCKET, SO_REUSEPORT, 
                    (const char TM_FAR *)&tempInt, sizeof(int));
#endif /* TM_USE_REUSEPORT*/

            /* Bind the SNTP port number */
            tm_bzero(&sockname, sizeof(struct sockaddr_storage));
            sockname.ss_family = ipAddrPtr->ss_family;
            sockname.ss_port   = htons((unsigned short)TM_SNTP_PORT);
            sockname.ss_len    = sizeof(struct sockaddr_storage);

            errorCode = bind( *socketDescPtr, 
                              (struct sockaddr *)&sockname, 
                              sizeof(sockname));
            if (errorCode == TM_SOCKET_ERROR )
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfSntpGetTimeByUnicast", 
                                "Failed to set the port number\n");
#endif /* TM_ERROR_CHECKING */
                errorCode = tfGetSocketError(*socketDescPtr);
                goto sntpGetTimeByUnicastExit;
            }
        }

        /* Create SNTP request packet. */
        tm_bzero(&tlNtpPacket, sizeof(ttNtpPacket));

        /* Set SNTP version & mode as client*/
        tlNtpPacket.sntpVersionNember = TM_SNTP_VERSION;
        tlNtpPacket.sntpMode = TM_SNTP_MODE_CLIENT;

        /* Get the system time */
        xmt_time  = 0;
#ifndef TM_USE_SNTP_MSEC
        errorCode = tfKernelGetSystemTime(&days, (ttUser32Bit *)&xmt_time);
#else  /* TM_USE_SNTP_MSEC */
        errorCode = tfKernelGetSystemTimeMsec(&days, (ttUser32Bit *)&xmt_time,
                                                     (ttUser32Bit *)&xmt_mtime);
        tlNtpPacket.transmitTimestampFractions = htonl(xmt_mtime);
#endif /* TM_USE_SNTP_MSEC */

        /* Set UTC time is reckoned from 0h 0m 0s UTC on 1 January 1900. */
        xmt_time += TM_JAN_1970;
        tlNtpPacket.transmitTimestampSeconds = htonl(xmt_time);

        /* Send SNTP packet to the specified SNTP server. */
        errCode = sendto( *socketDescPtr,
                          (const char *)&tlNtpPacket, 
                          sizeof( tlNtpPacket ),
                          0,
                          (const struct sockaddr *)ipAddrPtr, 
                          sizeof(struct sockaddr_storage));

        if(errCode == TM_SOCKET_ERROR)
        {
            errorCode = tfGetSocketError(*socketDescPtr);
#ifdef TM_ERROR_CHECKING
            if(errorCode != TM_EWOULDBLOCK)
            {
                tfKernelWarning("tfSntpGetTimeByUnicast", 
                                "Failed to send to SNTP server.\n");
            }
#endif /* TM_ERROR_CHECKING */
            goto sntpGetTimeByUnicastExit;
        }

        /* Shift to the reception mode of state flag. */
        *stateFlagPtr = TM_SNTP_STATUS_RECV;
    }

/* Reception processing of SNTP request packet */
    if(*stateFlagPtr == TM_SNTP_STATUS_RECV)
    {
sntpGetTimeByUnicastStart:
        /* Set the value of time out for select(). */
        waittime.tv_sec = 0;
        if ( blockingMode == TM_BLOCKING_ON)
        {
            /* Set the number of seconds for time out. */
            waittime.tv_sec = TM_SNTP_TIMEOUT;
        }
        waittime.tv_usec = 0;

        FD_ZERO(&rdps);
        FD_SET(*socketDescPtr, &rdps);

        selret = select( FD_SETSIZE, &rdps, 
                         (fd_set *)0, (fd_set *)0, &waittime);
        if(selret == TM_SOCKET_ERROR)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfSntpGetTimeByUnicast", 
                            "Failed in the select().\n");
#endif /* TM_ERROR_CHECKING */
            errorCode = tfGetSocketError(selret);
            goto sntpGetTimeByUnicastExit;
        }

        if (selret == TM_ENOERROR)
        {
            if ( blockingMode == TM_BLOCKING_ON )
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfSntpGetTimeByUnicast", 
                                "Time out in the select().\n");
#endif /* TM_ERROR_CHECKING */
                errorCode = TM_ETIMEDOUT;
            }
            else
            {
                errorCode = TM_EWOULDBLOCK;
            }
            goto sntpGetTimeByUnicastExit;
        }

#ifdef TM_ERROR_CHECKING
        if(FD_ISSET(*socketDescPtr, &rdps) == 0)
        {
            tfKernelWarning("tfSntpGetTimeByUnicast", 
                            "Received the packet from different socket.\n");
        }
#endif /* TM_ERROR_CHECKING */

        /* Receive the packet from the specified socket. */
        addressLength = sizeof(struct sockaddr_storage);
        errCode = recvfrom( *socketDescPtr,
                            (char *)&tlNtpPacket,
                            sizeof( tlNtpPacket ),
                            0,
                            (struct sockaddr *)&sockname,
                            &addressLength);

        if(errCode == TM_SOCKET_ERROR)
        {
            errorCode = tfGetSocketError(*socketDescPtr);
#ifdef TM_ERROR_CHECKING
            if(errorCode != TM_EWOULDBLOCK)
            {
                tfKernelWarning("tfSntpGetTimeByUnicast", 
                                "Failed in the recv().\n");
            }
#endif /* TM_ERROR_CHECKING */
            goto sntpGetTimeByUnicastExit;
        }

#ifdef TM_SNTP_ERROR_CHECKING
        if ( errCode < (sizeof( tlNtpPacket ) - TM_SNTP_KEY_DIGEST_SIZE)
            || (    tlNtpPacket.sntpVersionNember > TM_SNTP_VERSION
                ||  tlNtpPacket.sntpVersionNember < TM_NTP_OLDVERSION )
            )
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfSntpGetTimeByUnicast", 
                            "Failed by packet checking.\n");
#endif /* TM_ERROR_CHECKING */
            errorCode = TM_EINVAL;
            goto sntpGetTimeByUnicastExit;
        }
#endif /* TM_SNTP_ERROR_CHECKING */

        if(tlNtpPacket.sntpMode != TM_SNTP_MODE_SERVER)
        {
            goto sntpGetTimeByUnicastStart;
        }

        /* Convert to the number of second received from the SNTP server based 
         * on 1970/01/01. */
        ntp_time = ntohl(tlNtpPacket.transmitTimestampSeconds) - TM_JAN_1970;
        rch_time = ntohl(tlNtpPacket.receiveTimestampHigh)     - TM_JAN_1970;
        xmt_time = ntohl(tlNtpPacket.originateTimestampHigh)   - TM_JAN_1970;

        /* Retrieve the local time. */
        cur_time = 0;
#ifndef TM_USE_SNTP_MSEC
        errorCode = tfKernelGetSystemTime(&days, (ttUser32Bit *)&cur_time);

        /* Calcurate the delay time. */
        ttt = (rch_time - xmt_time);
        ttr = (ntp_time - cur_time);
        tta = (ttt + ttr);
        if ((0 < (ttS32Bit)ttt) && (0 < (ttS32Bit)ttr))
        {
            tta = tta >> 1;
        }
        else
        {
            tta = (ttS32Bit)tta >> 1;
        }

        /* Calcurate the current time. */
        cur_time += tta;

        /* Set the clock */
        errorCode = tfKernelSetSystemTime( (void *)&cur_time,
                                           &tlNtpPacket,
                                           sizeof(tlNtpPacket));

#ifdef TM_KERNEL_WIN32_X86
        fprintf(stderr,"system clock offset = %d (0x%08x)\n", tta, tta);
#endif /* TM_KERNEL_WIN32_X86 */
#else  /* TM_USE_SNTP_MSEC */
        ntp_mtime = ntohl(tlNtpPacket.transmitTimestampFractions);
        rch_mtime = ntohl(tlNtpPacket.receiveTimestampLow);
        xmt_mtime = ntohl(tlNtpPacket.originateTimestampLow);
        cur_mtime = 0;
        errorCode = tfKernelGetSystemTimeMsec(&days, (ttUser32Bit *)&cur_time,
                                                     (ttUser32Bit *)&cur_mtime);

        /* Calcurate the delay time. */
        tm_64bit_sub(rch_time, rch_mtime, xmt_time, xmt_mtime);
        tm_64bit_sub(ntp_time, ntp_mtime, cur_time, cur_mtime);
        ttt = rch_time;
        tm_64bit_add(rch_time, rch_mtime, ntp_time, ntp_mtime);

        if ((0 < (ttS32Bit)ttt) && (0 < (ttS32Bit)ntp_time))
        {
            tm_64bit_div(rch_time, rch_mtime, ttt, mttt);
        }
        else
        {
            tm_64bit_sdiv(rch_time, rch_mtime, ttt, mttt);
        }

        /* Calcurate the current time. */
        tm_64bit_add(cur_time, cur_mtime, ttt, mttt);

        /* Set the clock */
        errorCode = tfKernelSetSystemTimeMsec((ttUser32Bit *)&cur_time,
                                              (ttUser32Bit *)&cur_mtime,
                                              &tlNtpPacket, sizeof(tlNtpPacket));

#ifdef TM_KERNEL_WIN32_X86
        if ((ttt & 0x80000000) == 0)
        {
            fprintf(stderr,"system clock offset = %d (0x%08x).%03d (0x%08x)\n",
                            ttt, ttt, mttt / (0xFFFFFFFF / 1000), mttt);
        }
        else
        {
            fprintf(stderr,"system clock offset = -%d (0x%08x).%03d (0x%08x)\n",
                            ~ttt, ttt, ~mttt / (0xFFFFFFFF / 1000), mttt);
        }
#endif /* TM_KERNEL_WIN32_X86 */
#endif /* TM_USE_SNTP_MSEC */
        /* Clear the stateFlag */
        *stateFlagPtr = TM_SNTP_STATUS_SEND;

        /* Close the socket */
        tfClose(*socketDescPtr);
        *socketDescPtr = TM_SOCKET_ERROR;
    }

sntpGetTimeByUnicastExit:
    if (   errorCode != TM_ENOERROR
        && errorCode != TM_EWOULDBLOCK
        && (   ( socketDescPtr != (int *)0 )
            && ( *socketDescPtr != TM_SOCKET_ERROR ) )
       )
    {
        tfClose(*socketDescPtr);
        *socketDescPtr = TM_SOCKET_ERROR;
    }

    return errorCode;
}
