/*
 * Copyright Notice:
 * Copyright Zuken Elmic Japan   1997 - 2011
 * No portions or the complete contents of this file may be copied or
 * distributed in any form (including but not 
 * limited to printed or electronic
 * forms) without the expressed written consent of Zuken Elmic.
 * Copyright laws and International Treaties protect the
 *
 * contents of this file.  Unauthorized use is prohibited.
 * All rights reserved.
 *
 * Description:    All sntp variables and functions  
 * This file is intended for the user to include in 
 * their SNTP applications.
 * This file SHOULD NOT be modified.
 * Filename:        trsntp.h
 * $Source: include/trsntp.h $
 *
 */

#ifndef _TR_SNTP_H_
#define _TR_SNTP_H_ 

#ifdef __cplusplus
extern "C" {
#endif

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>

/* macro definitions */
/* #define TM_USE_SNTP_DIGEST */ /* not supported */

#ifndef TM_USE_SNTP
#ifdef TM_SNTP_ERROR_CHECKING
#undef TM_SNTP_ERROR_CHECKING
#endif /* TM_SNTP_ERROR_CHECKING */
#ifdef TM_USE_SNTP_MULTICAST
#undef TM_USE_SNTP_MULTICAST
#endif /* TM_USE_SNTP_MULTICAST */
#endif /* ! TM_USE_IPV4 */

/*
 * SNTP Common Structures
 */

typedef struct  tsSntpcSession
{
    int                     socketDesc;
    int                     blockingMode;
    int                     stateFlag;
    int                     sntpcMode;
#ifdef TM_USE_SNTP
    ttDeviceEntryPtr        devEntryPtr;
#endif /* TM_USE_SNTP */
/* When SNTP is running by unicast mode, this ip address is stored an IP 
 * address of SNTP server. When SNTP is running by multicast address 
 * this is stored multicast address.
 */
    struct sockaddr_storage ipAddr;
} ttSntpcSession;
typedef ttSntpcSession TM_FAR *   ttSntpcSessionPtr;

typedef void TM_FAR * ttUserSntpcHandle;

/* 
 * SNTP Header 
 */
#ifdef TM_LITTLE_ENDIAN
typedef struct tsNtpPacket{
    union {
        struct {
            unsigned int sntpus4Md  : 3;
            unsigned int sntpus4VN  : 3;
            unsigned int sntpus4LI  : 2;
            unsigned int sntpus4Stm : 8;
            unsigned int sntpus4PI  : 8;
            unsigned int sntpus4Pcn : 8;
        } sntp4Hdr1Struct;

        int ControlWord;
    } sntp4HdrUnion;

    tt32Bit rootDelay;
    tt32Bit rootDispersion;
    tt32Bit referenceIdentifier;
    tt32Bit referenceTimestampHigh;
    tt32Bit referenceTimestampLow;
    tt32Bit originateTimestampHigh;
    tt32Bit originateTimestampLow;
    tt32Bit receiveTimestampHigh;
    tt32Bit receiveTimestampLow;
    tt32Bit transmitTimestampSeconds;
    tt32Bit transmitTimestampFractions;
#ifdef TM_USE_SNTP_DIGEST
    tt32Bit keyIdentifier;
    tt32Bit messageDigest[4];
#endif /* TM_USE_SNTP_DIGEST */
} ttNtpPacket;
#else /* TM_BIG_ENDIAN */
typedef struct tsNtpPacket{
    union {
        struct {
            unsigned int sntpus4LI  : 2;
            unsigned int sntpus4VN  : 3;
            unsigned int sntpus4Md  : 3;
            unsigned int sntpus4Stm : 8;
            unsigned int sntpus4PI  : 8;
            unsigned int sntpus4Pcn : 8;
        } sntp4Hdr1Struct;

        int ControlWord;
    } sntp4HdrUnion;

    tt32Bit rootDelay;
    tt32Bit rootDispersion;
    tt32Bit referenceIdentifier;
    tt32Bit referenceTimestampHigh;
    tt32Bit referenceTimestampLow;
    tt32Bit originateTimestampHigh;
    tt32Bit originateTimestampLow;
    tt32Bit receiveTimestampHigh;
    tt32Bit receiveTimestampLow;
    tt32Bit transmitTimestampSeconds;
    tt32Bit transmitTimestampFractions;
#ifdef TM_USE_SNTP_DIGEST
    tt32Bit keyIdentifier;
    tt32Bit messageDigest[4];
#endif /* TM_USE_SNTP_DIGEST */
} ttNtpPacket;

#endif /* TM_LITTLE_ENDIAN */
#define sntpLeapId          sntp4HdrUnion.sntp4Hdr1Struct.sntpus4LI
#define sntpVersionNember   sntp4HdrUnion.sntp4Hdr1Struct.sntpus4VN
#define sntpMode            sntp4HdrUnion.sntp4Hdr1Struct.sntpus4Md
#define sntpStatum          sntp4HdrUnion.sntp4Hdr1Struct.sntpus4Stm
#define sntpPollInterval    sntp4HdrUnion.sntp4Hdr1Struct.sntpus4PI
#define sntpPrecision       sntp4HdrUnion.sntp4Hdr1Struct.sntpus4Pcn

#define sntpCtrWrd          sntp4HdrUnion.ControlWord

/*
 * SNTP Macro definitions
 */
#ifdef TM_USE_SNTP_DIGEST
#define TM_SNTP_KEY_DIGEST_SIZE 20
#else /* TM_USE_SNTP_DIGEST */
#define TM_SNTP_KEY_DIGEST_SIZE 0
#endif /* TM_USE_SNTP_DIGEST */

/* local variable definitions */
/* Default value for the amount of time (in seconds) to wait before 
 * retransmitting a request.*/
#define TM_SNTP_TIMEOUT 3

/* SNTP port number */
#define TM_SNTP_PORT   123

/*  */
#define TM_TIMEDIFF     32400UL         /* Time Differences in seconds */
#define TM_JAN_1985     473353200UL     /* 1985-1970 in seconds */
#define TM_JAN_1970     2208988800UL    /* 1970-1900 in seconds */

/* Version */
#define TM_NTP_OLDVERSION       1
#define TM_SNTP_VERSION         4

/* Mode */
#define TM_SNTP_MODE_ANYCAST       0
#define TM_SNTP_MODE_ACTIVE        1
#define TM_SNTP_MODE_PASSIVE       2
#define TM_SNTP_MODE_CLIENT        3
#define TM_SNTP_MODE_SERVER        4
#define TM_SNTP_MODE_BROARDCAST    5

/* SNTP server address */
#ifdef TM_USE_IPV4
#define TM_SNTP1_NICT_JP    "133.243.230.51"
#else /* TM_USE_IPV6 */ 
#define TM_SNTP1_NICT_JP    "2001:e38:2020:123"
#endif /* TM_USE_IPV4 */

/* Multicast IPv4 address for SNTP */
#define TM_SNTP_MULTICAST_V4ADDR    "224.0.1.1"

#define TM_SNTP_MULTICAST_V6LINK    "ff02::101"
#define TM_SNTP_MULTICAST_V6SITE    "ff05::101"
#define TM_SNTP_MULTICAST_V6ORG     "ff08::101"
#define TM_SNTP_MULTICAST_V6GLOBAL  "ff0e::101"

/* state */
#define TM_SNTP_STATUS_SEND  0
#define TM_SNTP_STATUS_RECV  1

/*
 * SNTP Functions prototypes
 */
#ifdef TM_USE_SNTP
ttUserSntpcHandle tfSntpStart(
        ttUserInterface      interfaceHandle,
        int                  sntpcMode,
        int                  blockingMode,
        ttSockAddrStoragePtr ipAddrPtr);

#ifdef TM_USE_SNTP_MULTICAST
int tfSntpSetRecvMode(ttUserSntpcHandle sntpcUserHandle);
#endif /* TM_USE_SNTP_MULTICAST */

int tfSntpExecute(ttUserSntpcHandle sntpcUserHandle);

int tfSntpCloseSocket(ttUserSntpcHandle sntpcUserHandle);

int tfSntpFreeSession (ttUserSntpcHandle sntpcUserHandle);
#endif /* ! TM_USE_SNTP */

int tfKernelSetSystemTime(
        ttUser32BitPtr  secondsPtr, 
        void *          voidPtr,
        int             size);

int tfSntpGetTimeByUnicast(
        int                  blockingMode,
        int *                socketDescPtr,
        int *                stateFlagPtr,
        ttSockAddrStoragePtr ipAddrPtr );

#ifdef __cplusplus
}
#endif

#endif /* _TR_SNTP_H_ */

