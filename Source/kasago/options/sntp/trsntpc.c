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
 * Description: SNTP Client (full mode) implementation
 *
 * Filename: trsntpc.c
 * Author: 
 * Date Created: 6/15/2010
 * $Source: source/trsntp.c $
 *
 * Modification History
 * $Revision: 6.0.2.4 $
 * $Date: 2015/07/21 17:15:01JST $
 * $Author: 
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_SNTP
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
 * tfSntpStart
 * Function Description
 *  This function is supported all modes.
 * 
 * Parameters
 * Parameter            Description
 * interfaceHandle      Interface handle as returned by tfAddInterface().
 * sntpcMode            Specifies the SNTP mode.
 *                       TM_SNTP_MODE_ANYCAST
                         TM_SNTP_MODE_CLIENT
                         TM_SNTP_MODE_BROARDCAST
 * blockingMode         Specifies whether the resolver should operate in 
 *                      blocking or non-blocking mode.
 *                       (TM_BLOCKING_ON or TM_BLOCKING_OFF)
 * ipAddrPtr            Pointer to structure set an IP address and an address 
 *                      family of SNTP server.
 * 
 * Returns
 * Value                Meaning 
 * ttUserSntpcHandle    SNTP handle Pointer
 * null                 failure. No SNTP handle allocated.
 */
ttUserSntpcHandle tfSntpStart(
                    ttUserInterface interfaceHandle,
                    int sntpcMode,
                    int blockingMode,
                    ttSockAddrStoragePtr ipAddrPtr )
{
    int                         errorCode;
    int                         socketDesc;
    int                         af_lock;
    ttSntpcSessionPtr           sntpcSessionPtr;
    ttDeviceEntryPtr            devPtr;
#ifdef TM_USE_REUSEPORT
    int                         tempInt;
#endif /* TM_USE_REUSEPORT*/
#ifdef TM_USE_SNTP_MULTICAST
#ifdef TM_USE_IPV4
    struct ip_mreq              linkScopeIpv4;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    struct ipv6_mreq            linkScopeIpv6;
#endif /* TM_USE_IPV6 */
#endif /* TM_USE_SNTP_MULTICAST */
    struct sockaddr_storage     sockname;

/* Initialize here, just to avoid compiler warnings. */
    errorCode = TM_ENOERROR;
    sntpcSessionPtr = (ttSntpcSessionPtr)0;
    devPtr = (ttDeviceEntryPtr) interfaceHandle;
/* 
 * Verify that the sockaddr_strage structer of server and 
 * specified server family are valid.
 */
    if( ipAddrPtr == (ttSockAddrStoragePtr)0
        || (    ipAddrPtr->ss_family != 0
#ifdef TM_USE_IPV4
            &&  ipAddrPtr->ss_family != AF_INET
#endif /* TM_USE_IPV4*/
#ifdef TM_USE_IPV6
            &&  ipAddrPtr->ss_family != AF_INET6
#endif /* TM_USE_IPV6 */
            )
        || (    blockingMode != TM_BLOCKING_OFF
            &&  blockingMode != TM_BLOCKING_ON )
        || (    sntpcMode != TM_SNTP_MODE_ANYCAST
            &&  sntpcMode != TM_SNTP_MODE_CLIENT
#ifdef TM_USE_SNTP_MULTICAST
            &&  sntpcMode != TM_SNTP_MODE_BROARDCAST
#endif /* TM_USE_SNTP_MULTICAST */
            )
      )
    {
        errorCode = TM_EINVAL;
        goto exitefUnicastStartSntp2;
    }

    errorCode = tfValidInterface(devPtr);
    if( errorCode != TM_ENOERROR)
    {
        errorCode = TM_EINVAL;
        goto exitefUnicastStartSntp2;
    }

/* Get buffer for SNTP session information */
    sntpcSessionPtr = devPtr->devSntpcSessionPtr;
    if ( sntpcSessionPtr == (ttSntpcSessionPtr) 0)
    {
        sntpcSessionPtr = (ttSntpcSessionPtr)
                        tm_get_raw_buffer(sizeof(ttSntpcSession));
        if (sntpcSessionPtr == (ttSntpcSessionPtr) 0)
        {
            errorCode = TM_EINVAL;
            goto exitefUnicastStartSntp2;
        }
        tm_bzero(sntpcSessionPtr, sizeof(ttSntpcSession));
        devPtr->devSntpcSessionPtr = (ttVoidPtr)sntpcSessionPtr;
        sntpcSessionPtr->devEntryPtr = devPtr;
        sntpcSessionPtr->sntpcMode   = sntpcMode;
        sntpcSessionPtr->socketDesc  = TM_SOCKET_ERROR;
    }

/* If the IP address of SNTP server is not set, set IPv4 address of the 
 * specified sntp server. 
 */
    if( ipAddrPtr->ss_family == 0 )
    {
        if (sntpcMode != TM_SNTP_MODE_CLIENT)
        {
            errorCode = TM_EINVAL;
            goto exitefUnicastStartSntp;
        }
#ifdef TM_USE_IPV4
        ipAddrPtr->addr.ipv4.sin_addr.s_addr = inet_addr(TM_SNTP1_NICT_JP);
        ipAddrPtr->ss_family = AF_INET;
        ipAddrPtr->ss_len    = sizeof(struct sockaddr_in);
#else /* TM_USE_IPV6 */
        (void)inet_pton(AF_INET6,
                        (const char *)TM_SNTP1_NICT_JP,
                        (void *)&(ipAddrPtr->addr.ipv6.sin6_addr));
        ipAddrPtr->ss_family = AF_INET6;
        ipAddrPtr->ss_len    = sizeof(struct sockaddr_in6);
#endif /* TM_USE_IPV4*/
    }

    /* Set the port number of sntp */
    ipAddrPtr->ss_port = htons((unsigned short)TM_SNTP_PORT);

    /* Create a socket with UDP mode. */
    if ( sntpcSessionPtr->socketDesc == TM_SOCKET_ERROR )
    {
        socketDesc = socket(ipAddrPtr->ss_family, SOCK_DGRAM, 0);
        if(socketDesc == TM_SOCKET_ERROR)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfSntpStart", 
                    "Failed to create a socket\n");
#endif /* TM_ERROR_CHECKING */
            errorCode = tfGetSocketError(socketDesc);
            goto exitefUnicastStartSntp;
        }
        sntpcSessionPtr->socketDesc = socketDesc;

        /* Set the blocking mode. */
        if(blockingMode == TM_BLOCKING_OFF)
        {
            af_lock = 1;
            sntpcSessionPtr->blockingMode = TM_BLOCKING_OFF;
        }
        else if(blockingMode == TM_BLOCKING_ON)
        {
            af_lock = 0;
            sntpcSessionPtr->blockingMode = TM_BLOCKING_ON;
        }
        errorCode = tfIoctl(socketDesc, FIONBIO, &af_lock);
        if (errorCode == TM_SOCKET_ERROR )
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfSntpStart", 
                    "Failed to specfy a blocking mode\n");
#endif /* TM_ERROR_CHECKING */
            tfClose(socketDesc);
            goto exitefUnicastStartSntp;
        }

#ifdef TM_USE_REUSEPORT
        tempInt = 1;
        setsockopt(socketDesc, SOL_SOCKET, SO_REUSEPORT, 
                (const char TM_FAR *)&tempInt, sizeof(int));
#endif /* TM_USE_REUSEPORT*/

        tm_bzero(&sockname, sizeof(struct sockaddr_storage));
        sockname.ss_family  = ipAddrPtr->ss_family;
        sockname.ss_port    = htons((unsigned short)TM_SNTP_PORT);
        sockname.ss_len     = sizeof(struct sockaddr_storage);

#ifdef TM_USE_SNTP_MULTICAST
        if(sntpcMode == TM_SNTP_MODE_BROARDCAST)
        {
#ifdef TM_USE_IPV4
            if(sockname.ss_family == AF_INET)
            {
                linkScopeIpv4.imr_multiaddr.s_addr = 
                                    inet_addr(TM_SNTP_MULTICAST_V4ADDR);
                tm_ip_copy( ipAddrPtr->addr.ipv4.sin_addr.s_addr,
                            linkScopeIpv4.imr_interface.s_addr );

                /* Call setsockopt() to set IPV4_JOIN_GROUP value for IPV4. */
                errorCode = setsockopt( socketDesc, 
                                        IP_PROTOIP, 
                                        IPO_ADD_MEMBERSHIP,
                                        (const char TM_FAR *)&linkScopeIpv4, 
                                        sizeof(linkScopeIpv4));
                if (errorCode == TM_SOCKET_ERROR )
                {
#ifdef TM_ERROR_CHECKING
                    tfKernelWarning("tfSntpStart", 
                            "Failed to join the multicast IPv4 address.\n");
#endif /* TM_ERROR_CHECKING */
                    tfClose(socketDesc);
                    goto exitefUnicastStartSntp;
                }
            }
            else
#endif /* TM_USE_IPV4 */
            {
#ifdef TM_USE_IPV6
                (void)inet_pton( AF_INET6,
                                 (const char *)TM_SNTP_MULTICAST_V6GLOBAL,
                                 (void *)&linkScopeIpv6.ipv6mr_multiaddr.ip6Addr );

                /* Call setsockopt() to set IPV6_JOIN_GROUP value for IPV6. */
                linkScopeIpv6.ipv6mr_interface = devPtr->devIndex;
                errorCode = setsockopt( socketDesc,
                                        IPPROTO_IPV6,
                                        IPV6_JOIN_GROUP,
                                        (const char *)&linkScopeIpv6, 
                                        sizeof( linkScopeIpv6 ) );
                if (errorCode == TM_SOCKET_ERROR )
                {
#ifdef TM_ERROR_CHECKING
                    tfKernelWarning("tfSntpStart", 
                            "Failed to join the multicast IPv6 address.\n");
#endif /* TM_ERROR_CHECKING */
                    tfClose(socketDesc);
                    goto exitefUnicastStartSntp;
                }
#endif /* TM_USE_IPV6 */
            }
            sntpcSessionPtr->stateFlag = TM_SNTP_STATUS_RECV;
        }
        else
#endif /* TM_USE_SNTP_MULTICAST */
        {
            sntpcSessionPtr->stateFlag = TM_SNTP_STATUS_SEND;
        }

        /* Bind the SNTP port number */
        errorCode = bind( socketDesc, 
                          (struct sockaddr *)&sockname, 
                          sizeof(sockname));
        if (errorCode == TM_SOCKET_ERROR )
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfSntpStart", 
                    "Failed to set the port number\n");
#endif /* TM_ERROR_CHECKING */
            tfClose(socketDesc);
            goto exitefUnicastStartSntp;
        }
    }

    /* Copy the server IP address to sntp Handle. */
    tm_bcopy( ipAddrPtr,
              &sntpcSessionPtr->ipAddr,
              sizeof(struct sockaddr_storage) );

exitefUnicastStartSntp:
    if ( (errorCode != TM_ENOERROR ) && (devPtr != (ttDeviceEntryPtr)0))
    {
        tfSntpFreeSession((ttUserSntpcHandle)sntpcSessionPtr);
        sntpcSessionPtr = (ttSntpcSessionPtr) 0;
        devPtr->devSntpcSessionPtr = (ttVoidPtr)0;
    }
exitefUnicastStartSntp2:
    return (ttUserSntpcHandle)sntpcSessionPtr;
}

/*
 * tfSntpExecute
 * Function Description
 *  
 * 
 * Parameters
 * Parameter                   Description
 * sntpcUserHandle             SNTP handle as returned by tfSntpStart().   
 * 
 * Returns
 * Value                       Meaning 
 * TM_ENOERROR                 Success
 * TM_EINVAL                   Invalid parameter
 * TM_EWOULDBLOCK              Getting time in progress.
 * TM_EPERM                    Can't set time to system.
 */
int tfSntpExecute( ttUserSntpcHandle sntpcUserHandle )
{
    int                     errorCode;
    int                     errCode;

    int                     socketDesc;
    ttSntpcSessionPtr       sntpcSessionPtr;
    ttSockAddrStoragePtr    ipAddrPtr;
    ttNtpPacket             tlNtpPacket;
    struct sockaddr_storage srcIpAddr;
    int                     addressLength;

    fd_set                  rdps;
    struct timeval          waittime;
    int                     selret;

    tt32Bit                 cur_time;
    tt32Bit                 ntp_time;
    tt32Bit                 xmt_time;
    tt32Bit                 rch_time;

    tt32Bit                 days;
    tt32Bit                 ttt;
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

    /* Verify that the specified sntp handle is valid. */
    if(sntpcUserHandle == (ttUserSntpcHandle)0)
    {
        errorCode = TM_EINVAL;
        goto exitefSntpExecute2;
    }
    sntpcSessionPtr = (ttSntpcSessionPtr)sntpcUserHandle;
    socketDesc = sntpcSessionPtr->socketDesc;
    ipAddrPtr = &sntpcSessionPtr->ipAddr;

/* Transmission processing of SNTP request packet */    
    if(sntpcSessionPtr->stateFlag == TM_SNTP_STATUS_SEND)
    {
        /* Initialize the NTP packet */
        tm_bzero(&tlNtpPacket, sizeof(ttNtpPacket));

        /* SNTP Version*/
        tlNtpPacket.sntpVersionNember = TM_SNTP_VERSION;

        switch(sntpcSessionPtr->sntpcMode)
        {
            case TM_SNTP_MODE_ANYCAST:
            case TM_SNTP_MODE_CLIENT:
                /* Client mode */
                tlNtpPacket.sntpMode = TM_SNTP_MODE_CLIENT;
                break;

            default:
                errorCode = TM_EINVAL;
                goto exitefSntpExecute;
        }

        /* Get the system time */
        xmt_time  = 0;
#ifndef TM_USE_SNTP_MSEC
        errorCode = tfKernelGetSystemTime(&days, (ttUser32Bit *)&xmt_time);
#else  /* TM_USE_SNTP_MSEC */
        errorCode = tfKernelGetSystemTimeMsec(&days, (ttUser32Bit *)&xmt_time,
                                                     (ttUser32Bit *)&xmt_mtime);
        tlNtpPacket.transmitTimestampFractions = htonl(xmt_mtime);
#endif /* TM_USE_SNTP_MSEC */

        xmt_time += TM_JAN_1970;
        tlNtpPacket.transmitTimestampSeconds = htonl(xmt_time);

        /* Send SNTP packet to the specified SNTP server. */
        errCode = sendto(   socketDesc,
                            (const char *)&tlNtpPacket, 
                            sizeof( tlNtpPacket ),
                            0,
                            (const struct sockaddr *)ipAddrPtr, 
                            sizeof(struct sockaddr_storage));

        if(errCode == TM_SOCKET_ERROR)
        {
            errorCode = tfGetSocketError(socketDesc);
#ifdef TM_ERROR_CHECKING
            if(errorCode != TM_EWOULDBLOCK)
            {
                tfKernelWarning("tfSntpExecute", 
                                "Failed to send to SNTP server.\n");
            }
#endif /* TM_ERROR_CHECKING */
            goto exitefSntpExecute;
        }

        /* Shift to the reception mode of state flag. */
        sntpcSessionPtr->stateFlag = TM_SNTP_STATUS_RECV;
    }

/* Reception processing of SNTP request packet */
    if(sntpcSessionPtr->stateFlag == TM_SNTP_STATUS_RECV)
    {
startefSntpExecute:
        /* Set the value of time out for select(). */
        waittime.tv_sec = 0;
        if(sntpcSessionPtr->blockingMode == TM_BLOCKING_ON)
        {
            /* Set the number of seconds for time out. */
            waittime.tv_sec = TM_SNTP_TIMEOUT;
        }
        waittime.tv_usec = 0;

        FD_ZERO(&rdps);
        FD_SET(socketDesc, &rdps);

        selret = select( FD_SETSIZE, &rdps, 
                         (fd_set *)0, (fd_set *)0, &waittime);
        if(selret == TM_SOCKET_ERROR)
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfSntpExecute", 
                            "Failed in the select().\n");
#endif /* TM_ERROR_CHECKING */
            errorCode = tfGetSocketError(selret);
            goto exitefSntpExecute;
        }

        if(selret == TM_ENOERROR)
        {
            if(     sntpcSessionPtr->blockingMode == TM_BLOCKING_ON
                &&  sntpcSessionPtr->sntpcMode != TM_SNTP_MODE_BROARDCAST )
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfSntpExecute", 
                                "Time out in the select().\n");
#endif /* TM_ERROR_CHECKING */
                errorCode = TM_ETIMEDOUT;
            }
            else
            {
                errorCode = TM_EWOULDBLOCK;
            }
            goto exitefSntpExecute;
        }

#ifdef TM_ERROR_CHECKING
        selret = FD_ISSET(socketDesc, &rdps);
        if(selret == 0)
        {
            tfKernelWarning("tfSntpExecute", 
                            "Received the packet from different socket.\n");
        }
#endif /* TM_ERROR_CHECKING */

        /* Receive the packet from the specified socket. */
        addressLength = sizeof(struct sockaddr_storage);
        errCode = recvfrom( socketDesc, 
                            (char *)&tlNtpPacket, 
                            sizeof( tlNtpPacket ), 
                            0,
                            (struct sockaddr *)&srcIpAddr,
                            &addressLength);

        if(errCode == TM_SOCKET_ERROR)
        {
            errorCode = tfGetSocketError(socketDesc);
#ifdef TM_ERROR_CHECKING
            if(errorCode != TM_EWOULDBLOCK)
            {
                tfKernelWarning("tfSntpExecute", 
                                "Failed in the recv().\n");
            }
#endif /* TM_ERROR_CHECKING */
            goto exitefSntpExecute;
        }

#ifdef TM_SNTP_ERROR_CHECKING
        if ( errCode < (sizeof( tlNtpPacket ) - TM_SNTP_KEY_DIGEST_SIZE)
            || (    tlNtpPacket.sntpVersionNember > TM_SNTP_VERSION
                ||  tlNtpPacket.sntpVersionNember < TM_NTP_OLDVERSION )
            )
        {
#ifdef TM_ERROR_CHECKING
            tfKernelWarning("tfSntpExecute", 
                            "Failed by packet checking.\n");
#endif /* TM_ERROR_CHECKING */
            errorCode = TM_EINVAL;
            goto exitefSntpExecute;
        }
#endif /* TM_SNTP_ERROR_CHECKING */

        switch(sntpcSessionPtr->sntpcMode)
        {
        case TM_SNTP_MODE_ANYCAST:
        case TM_SNTP_MODE_CLIENT:
            /* Client mode */
            if(tlNtpPacket.sntpMode != TM_SNTP_MODE_SERVER)
            {
                goto startefSntpExecute;
            }
            break;
#ifdef TM_USE_SNTP_MULTICAST
        case TM_SNTP_MODE_BROARDCAST:
            /* Multicast mode */
            if(tlNtpPacket.sntpMode != TM_SNTP_MODE_BROARDCAST)
            {
                goto startefSntpExecute;
            }
            break;
#endif /* TM_USE_SNTP_MULTICAST */
        default:
            break;
        }

        if (sntpcSessionPtr->sntpcMode == TM_SNTP_MODE_ANYCAST)
        {
            tm_bcopy( &srcIpAddr,
                      &sntpcSessionPtr->ipAddr,
                      sizeof(struct sockaddr_storage) );
            sntpcSessionPtr->sntpcMode = TM_SNTP_MODE_CLIENT;
            errorCode = TM_ENOERROR;
            goto exitefSntpExecute;
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
    }
exitefSntpExecute:
    if((errorCode != TM_EWOULDBLOCK)
        && (sntpcSessionPtr->sntpcMode != TM_SNTP_MODE_BROARDCAST))
    {
        /* Clear the state flag. */
        sntpcSessionPtr->stateFlag = TM_SNTP_STATUS_SEND;
    }
exitefSntpExecute2:
    return errorCode;
}

/*
 * tfSntpCloseSocket
 * Function Description
 *  This function close a socket opend by tfSntpStart(). When SNTP is 
 * running as multicast mode, it send LEAVE message.
 * 
 * Parameters
 * Parameter                   Description
 * sntpcUserHandle             SNTP handle as returned by tfSntpStart().   
 * 
 * Returns
 * Value                       Meaning 
 * TM_ENOERROR                 Success
 * TM_EINVAL                   Invalid parameter
 */
int tfSntpCloseSocket(ttUserSntpcHandle sntpcUserHandle)
{
    int                     errorCode;
    ttSntpcSessionPtr       sntpcSessionPtr;
#ifdef TM_USE_SNTP_MULTICAST
    ttDeviceEntryPtr            devPtr;
#ifdef TM_USE_IPV4
    struct ip_mreq              linkScopeIpv4;
    ttSockAddrStoragePtr        ipAddrPtr;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    struct ipv6_mreq            linkScopeIpv6;
#endif /* TM_USE_IPV6 */
#endif /* TM_USE_SNTP_MULTICAST */

    errorCode = TM_ENOERROR;

    if(sntpcUserHandle == (ttUserSntpcHandle)0)
    {
        errorCode = TM_EINVAL;
        goto exittfCloseSntpSocket;
    }

    sntpcSessionPtr = (ttSntpcSessionPtr)sntpcUserHandle;

    if(sntpcSessionPtr->socketDesc == TM_SOCKET_ERROR)
    {
        errorCode = TM_EINVAL;
        goto exittfCloseSntpSocket;
    }

#ifdef TM_USE_SNTP_MULTICAST
    devPtr = sntpcSessionPtr->devEntryPtr;
    if( sntpcSessionPtr->sntpcMode == TM_SNTP_MODE_BROARDCAST )
    {
#ifdef TM_USE_IPV4
        if (sntpcSessionPtr->ipAddr.addr.ipv4.sin_family == AF_INET )
        {
            linkScopeIpv4.imr_multiaddr.s_addr = 
                            inet_addr(TM_SNTP_MULTICAST_V4ADDR);
            ipAddrPtr = &sntpcSessionPtr->ipAddr;
            tm_ip_copy( ipAddrPtr->addr.ipv4.sin_addr.s_addr,
                        linkScopeIpv4.imr_interface.s_addr );
            /* Call setsockopt() to leave the IPv4 responder socket from
             * multicast group */
            errorCode = setsockopt( sntpcSessionPtr->socketDesc,
                                    IP_PROTOIP, 
                                    IPO_DROP_MEMBERSHIP,
                                    (const char TM_FAR *)&linkScopeIpv4,
                                    sizeof(linkScopeIpv4));

            if(errorCode != TM_ENOERROR)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfSntpCloseSocket", 
                    "Calling setsockopt with IPO_DROP_MEMBERSHIP\n");
#endif /* TM_ERROR_CHECKING */
                errorCode = TM_EINVAL;
            }
        }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
        if( sntpcSessionPtr->ipAddr.addr.ipv6.sin6_family == AF_INET6 )
        {
            (void)inet_pton( AF_INET6,
                             (const char *)TM_SNTP_MULTICAST_V6GLOBAL,
                             (void *)&linkScopeIpv6.ipv6mr_multiaddr.ip6Addr );
            /* Call setsockopt() to leave the IPv6 responder socket from
             * multicast group */
            linkScopeIpv6.ipv6mr_interface = devPtr->devIndex;
            errorCode = setsockopt( sntpcSessionPtr->socketDesc, 
                                    IPPROTO_IPV6, 
                                    IPV6_LEAVE_GROUP, 
                                    (const char *)&linkScopeIpv6, 
                                    sizeof(linkScopeIpv6));
            if(errorCode != TM_ENOERROR)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelWarning("tfSntpCloseSocket", 
                    "Calling setsockopt failed with IPV6_LEAVE_GROUP\n");
#endif /* TM_ERROR_CHECKING */
                errorCode = TM_EINVAL;
            }
        }
#endif /* TM_USE_IPV6 */
    }
#endif /* TM_USE_SNTP_MULTICAST */

    tfClose(sntpcSessionPtr->socketDesc);
    sntpcSessionPtr->socketDesc = TM_SOCKET_ERROR;

exittfCloseSntpSocket:
    return errorCode;    
}

/*
 * tfSntpFreeSession
 * Function Description
 *  This function free a SNTP handle allocated by tfSntpStart().
 * 
 * Parameters
 * Parameter                   Description
 * sntpcUserHandle             SNTP handle as returned by tfSntpStart().     
 * 
 * Returns
 * Value                       Meaning 
 * TM_ENOERROR                 Success
 * TM_EINVAL                   Invalid parameter
 */
int tfSntpFreeSession (ttUserSntpcHandle sntpcUserHandle)
{
    int                         result;
    ttSntpcSessionPtr           sntpcSessionPtr;
    ttDeviceEntryPtr            devPtr;

    sntpcSessionPtr = (ttSntpcSessionPtr)sntpcUserHandle;
    if (sntpcSessionPtr != (ttSntpcSessionPtr)0)
    {
        /* Free SNTP session */
        devPtr = sntpcSessionPtr->devEntryPtr;
        devPtr->devSntpcSessionPtr = (ttVoidPtr)0;
        tm_free_raw_buffer((ttRawBufferPtr) sntpcSessionPtr);
        result = TM_ENOERROR;
    }
    else
    {
        result = TM_EINVAL;
    }

    return (result);
}
#else /* ! TM_USE_SNTP */

/* To allow link for builds when TM_USE_SNTP is not defined */
int tvSntpDummy = 0;

#endif /* ! TM_USE_SNTP */
