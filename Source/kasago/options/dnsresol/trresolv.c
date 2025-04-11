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
 * Description:     DNS Resolver Application
 * Filename:        trresolv.c
 * Author:          Jason
 * Date Created:    7/21/00
 * $Source: source/trresolv.c $
 *
 * Modification History
 * $Revision: 6.0.2.24 $
 * $Date: 2013/02/27 02:40:57JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>


#ifndef TM_DISABLE_DNS_MAIL_RECS
#define TM_DNS_MAIL
#endif /* TM_DISABLE_DNS_MAIL_RECS */

/* Resource record query types */
#define TM_DNS_QTYPE_A          1
#define TM_DNS_QTYPE_CNAME      5
#define TM_DNS_QTYPE_REV        12
#define TM_DNS_QTYPE_MX         15
#define TM_DNS_QTYPE_AAAA       28
#ifdef TM_DNS_USE_SRV
#define TM_DNS_QTYPE_SRV        33
#endif /* TM_DNS_USE_SRV */

/* Cache search types */
#define TM_DNS_SEARCH_ID        0
#define TM_DNS_SEARCH_NAME      1

/* Cache search filters */
#define TM_DNS_LOOKUP_ALL       0   /* Search all records */
#define TM_DNS_LOOKUP_ACTIVE    1   /* Search only in progress entries */

/* DNS packet flags */
#define TM_DNS_HDR_QUERY        0x0000
#define TM_DNS_HDR_RESP         0x8000
#define TM_DNS_HDR_RECURSE      0x0100

/* DNS packet opcodes */
#define TM_DNS_OPCODE_QUERY     0x0000

/* DNS response code mask */
#define TM_DNS_RCODE_MASK       0x000f

/* DNS truncated code mask */
#define TM_DNS_TRUNC_MASK       0x0200

/* DNS class values */
#define TM_DNS_CLASS_IN         1

/* UDP port of the remote DNS server */
#define TM_DNS_SERVER_PORT      53


/*
 * Maximum length of the hostname used for IPv4 reverse lookups
 * (xxx.xxx.xxx.xxx.in-addr.arpa)
 */
#define TM_MAX_IN_ARPA_LENGTH   30

/*
 * Maximum length of the hostname used for IPv6 reverse lookups
 * (a.b.c.d...ip6.arpa (int))
 */
#define TM_6_MAX_IN_ARPA_LENGTH 72

/*
 * DNS cache entry flags
 */

/* Entry is complete: all info recieved, no more packets will be sent\recv */
#define TM_DNSF_COMPLETE        (tt16Bit)0x0001
/* New user flag: 1-reserve v6Entry till v4Entry is obtained;
   2- make sure return the entry once not matter correct or wrong errorCode */
#define TM_DNSF_USER            (tt16Bit)0x0002
/* IPv6 address to hostname lookup in the "IP6.ARPA" domain. */
#define TM_6_DNSF_IP6_ARPA      (tt16Bit)0x0004
/* While trying to perform a IPv6 address to hostname lookup, "IP6.ARPA" was
   used, but failed.  Revert to the "IP6.INT" domain. */
#define TM_6_DNSF_IP6_INT       (tt16Bit)0x0008


/* Default value for maximum number of cache entries */
#define TM_DNS_CACHE_SIZE       10

/* Default value for the number of times to retransmit DNS requests */
#define TM_DNS_RETRIES          3

/*
 * Default value for the amount of time (in seconds) to wait before
 * retransmitting a request.
 */
#define TM_DNS_TIMEOUT          5

#ifdef TM_32BIT_DSP
#define TM_DNS_HEADER_SIZE      3
#else /* ! TM_32BIT_DSP */
#define TM_DNS_HEADER_SIZE      12
#endif /* TM_32BIT_DSP */

/* Used to avoid an infinite loop with DNS label ponters */
#define TM_DNS_MAX_POINTER_COUNT 50

#ifdef TM_DNS_USE_SRV
#ifndef TM_DNS_SRV_HOSTS_PER_ENTRY
/* Maximum number of SRV hostnames per cache entry */
#define TM_DNS_SRV_HOSTS_PER_ENTRY 3
#endif /* TM_DNS_SRV_MAX_HOSTS */
#ifndef TM_DNS_SRV_ADDRS_PER_HOST
/* Maximum numbers of addrinfo structs per SRV hostname */
#define TM_DNS_SRV_ADDRS_PER_HOST  2
#endif /* TM_DNS_SRV_MAX_ADDRS */
#endif /* TM_DNS_USE_SRV */

#define TM_ADDRINFO_NULL_PTR     (struct addrinfo TM_FAR *)0
#ifdef TM_DNS_USE_SRV
#define TM_ADDRINFO_EXT_NULL_PTR (ttAddrInfoExtPtr)0
#endif /* TM_DNS_USE_SRV */

#ifdef TM_DNS_USE_SRV
/* We've already done an A-record query for this hostname */
#define TM_DNS_SRV_FLAGS_QRY_IP4 0x0001
/* We've already done an AAAA-record query for this hostname */
#define TM_DNS_SRV_FLAGS_QRY_IP6 0x0002
/* This entry has already been sorted */
#define TM_DNS_SRV_FLAGS_SORTED  0x0004
/* This entry has been deleted (but not removed yet) */
#define TM_DNS_SRV_FLAGS_DELETED 0x0008
#endif /* TM_DNS_USE_SRV */

#ifndef TM_DSP
/* Copy 2 bytes preserving byte order */
#define tm_2bytes_copy(srcPtr, destPtr)                                        \
{                                                                              \
    ((tt8BitPtr)(ttVoidPtr)(destPtr))[0] = ((tt8BitPtr)(ttVoidPtr)(srcPtr))[0];\
    ((tt8BitPtr)(ttVoidPtr)(destPtr))[1] = ((tt8BitPtr)(ttVoidPtr)(srcPtr))[1];\
}
/* Copy 4 bytes preserving byte order */
#define tm_4bytes_copy(srcPtr, destPtr)                                        \
{                                                                              \
    tm_2bytes_copy(srcPtr, destPtr);                                           \
    ((tt8BitPtr)(ttVoidPtr)(destPtr))[2] = ((tt8BitPtr)(ttVoidPtr)(srcPtr))[2];\
    ((tt8BitPtr)(ttVoidPtr)(destPtr))[3] = ((tt8BitPtr)(ttVoidPtr)(srcPtr))[3];\
}
#endif /* TM_DSP */

/*
 * Unaligned 16-bit copy from host to network array.
 * For DSP network array is unpacked.
 * Copy MSB in low memory (network/big endian order).
 * Copy LSB in high memory (network/big endian order).
 * This copy works on any CPU (big endian or little endian), because
 * the shift operation on ints works the same way on every CPU.
 */
#define tm_un_htons(hostShort, netPtr)                                      \
{                                                                           \
    netPtr[0] = (tt8Bit)(((hostShort) >> 8) & 0xFF);                                  \
    netPtr[1] = (tt8Bit)((hostShort) & 0xFF);                                         \
}
/*
 * Unaligned 16-bit copy from network array to host.
 * For DSP network array is unpacked.
 * Copy MSB from low memory (network/big endian order).
 * Copy LSB from high memory (network/big endian order).
 * This copy works on any CPU (big endian or little endian), because
 * the shift operation works the same way on every CPU.
 */
#define tm_un_ntohs(netPtr, hostShort)                                      \
{                                                                           \
    hostShort = (tt16Bit)(  (tt16Bit)(((tt16Bit)((netPtr)[0])) << 8)        \
                          | (tt16Bit)(((tt16Bit)(((netPtr)[1]))) & 0xFF));  \
}

/*
 * Unaligned 32-bit copy from host to network array.
 * For DSP network array is unpacked.
 * Copy MSB in low memory (network/big endian order).
 * Copy LSB in high memory (network/big endian order).
 * This copy works on any CPU (big endian or little endian), because
 * the shift operation on ints works the same way on every CPU.
 */
#define tm_un_htonl(hostLong, netPtr)                                       \
{                                                                           \
    netPtr[0] = ((hostLong) >> 24) & 0xFF;                                  \
    netPtr[1] = ((hostLong) >> 16) & 0xFF;                                  \
    netPtr[2] = ((hostLong) >> 8) & 0xFF;                                   \
    netPtr[3] = (hostLong) & 0xFF;                                          \
}
/*
 * Unaligned 32-bit copy from network array to host.
 * For DSP network array is unpacked.
 * Copy MSB from low memory (network/big endian order).
 * Copy LSB from high memory (network/big endian order).
 * This copy works on any CPU (big endian or little endian), because
 * the shift operation works the same way on every CPU.
 */
#define tm_un_ntohl(netPtr, hostLong)                                       \
{                                                                           \
    hostLong = (tt32Bit)(  (tt32Bit)(((tt32Bit)((netPtr)[0])) << 24)        \
                         | (tt32Bit)(((tt32Bit)((netPtr)[1])) << 16)        \
                         | (tt32Bit)(((tt32Bit)((netPtr)[2])) << 8)         \
                         | (tt32Bit)(((tt32Bit)(((netPtr)[3]))) & 0xFF));   \
}

typedef struct tsDnsHeader
{
/* Identifier used to match queries and replies */
    tt8Bit dnsId[2];
/*
 * Various flags: query/response, opcode, authoritative answer, truncation,
 * recursion desired/available, response code.
 */
    tt8Bit dnsFlags[2];
/* Number of query records */
    tt8Bit dnsQuestionCount[2];
/* Number of answer records */
    tt8Bit dnsAnswerCount[2];
/* Number of name server records */
    tt8Bit dnsNameServerCount[2];
/* Number of 'additional' type records */
    tt8Bit dnsAdditionalCount[2];
} ttDnsHeader;

typedef ttDnsHeader TM_FAR * ttDnsHeaderPtr;

/*
 * DNS Resource Record Header
 * NOTE: dnsRdLength is not really part of the header. It just contains the
 * length of what comes after the header.
 */
typedef struct tsDnsRRHeader
{
/* Resource record type (A, MX, etc) */
    tt8Bit dnsrType[2];
/* Resource record class (should always be 'IN') */
    tt8Bit dnsrClass[2];
/* Time that this record is allowed to be cached */
    tt8Bit dnsrTtl[4];
/* Length of record data that follows this header */
    tt8Bit dnsrRdLength[2];
} ttDnsRRHeader;

typedef ttDnsRRHeader TM_FAR * ttDnsRRHeaderPtr;


#define TM_DNS_RR_HDR_LEN   10


/*
 * Internal function prototypes
 */

/* Removes an entry from the DNS cache */
static int tfDnsCacheRemove(ttDnsCacheEntryPtr cacheEntryPtr);

/* Adds a new entry to the DNS cache */
static ttDnsCacheEntryPtr tfDnsCacheNew(int TM_FAR * retErrorCode);

/* Linearly searches the DNS cache for an entry with a matching field */
static ttDnsCacheEntryPtr tfDnsCacheLookup( tt8Bit    searchType,
                                            ttVoidPtr valuePtr,
                                            tt16Bit   queryType,
                                            tt8Bit    filter );

/*
 * Constructs a DNS query packet based on the hostname and type passed in, and
 * sends this packet to the specified DNS server.
 */
static int tfDnsSendRequest( ttDnsCacheEntryPtr cacheEntryPtr,
                             tt16Bit            queryType
#ifdef TM_USE_IPV6
                             , tt16Bit          flags
#endif /* TM_USE_IPV6 */
                             );

/*
 * Indicates that no response has been receieved from the DNS server; attempt
 * to retransmit request.
 */
TM_NEARCALL static void tfDnsTimeout (ttVoidPtr      timerBlockPtr,
                          ttGenericUnion userParm1,
                          ttGenericUnion userParm2);

/* Obtain the index of the next server to query */
static int tfDnsGetNextServer(ttGenericUnionPtr prevServersTried);

/*
 * Parses a response form the DNS server.  This routine extracts the error code
 * and the information (IP Address, for example) from the packet.
 */
static int tfDnsParseResponse( tt8BitPtr          responsePtr,
                               ttDnsCacheEntryPtr entryPtr );

/*
 * This function is called when there is a datagram to be received on our
 * socket; that is, we received a response from the DNS server.  We should
 * parse the response and place the information into our cache.  If we are in
 * blocking mode, this function should also post on the semaphore associated
 * with this cache entry, waking up the task waiting for the response.
 */
TM_NEARCALL static void tfDnsCallback(int sock, int event);

static int tfCopyAddrInfo(struct addrinfo TM_FAR * fromAddrInfoPtr,
                          struct addrinfo TM_FAR * toAddrInfoPtr);

/* Compare two compressed DNS labels */
static ttSChar tfDnsLabelAsciiCompare(  tt8BitPtr labelPtr,
                                        tt8BitPtr asciiPtr,
                                        tt8BitPtr pktDataPtr );

/* Return absolute length of compressed DNS label */
static tt16Bit tfDnsLabelLength(tt8BitPtr labelPtr);

/* Return expanded length of compressed DNS label */
static tt16Bit tfDnsExpLabelLength( tt8BitPtr labelPtr,
                                    tt8BitPtr pktDataPtr );

/*
 * Allocates and zeros out a new addrinfo structure, including the attached
 * sockaddr_storage structure.  This is used by getaddrinfo and internally by
 * the DNS resolver.
 */
static struct addrinfo TM_FAR * tfDnsAllocAddrInfo(void);

#ifdef TM_DNS_USE_SRV
/*
 * Allocates and zeros out a new extended addrinfo structure, including the
 * attached sockaddr_storage structure. This is used internally by the
 * DNS resolver.
 */
static ttAddrInfoExtPtr tfDnsAllocAddrInfoExt(void);
/* Frees memory used by extended addrinfo structure chain */
static void tfDnsFreeAddrInfoExt(ttAddrInfoExtPtr startAddrInfo);
/*
 * Get the next best address to use for the given SRV request. For the first
 * request, the sockaddr_storage pointed to by addr should be zeroed out
 */
static int tfDnsSrvGetNextHost(
        ttDnsCacheEntryPtr               entryPtr,
        struct sockaddr_storage TM_FAR * addr,
        tt32Bit                          sort);
/*
 * Get the highest-priority address (given the sort type requested by the
 * user) for the given extended addrinfo structure
 */
static int tfDnsSrvGetFirstHost(
        ttDnsCacheEntryPtr                entryPtr,
        ttAddrInfoExtPtr                  srvAddrInfoPtr,
        struct addrinfo TM_FAR * TM_FAR * firstHostPtrPtr,
        tt32Bit                           sortType);
/*
 * Copy data from an A or AAAA record cache entry into an addrinfo structure
 * stored on the given SRV extended addrinfo structure
 */
static int tfDnsSrvSaveRecordInfo(ttDnsCacheEntryPtr       entryPtr,
                                  struct addrinfo TM_FAR * addrInfoPtr,
                                  ttAddrInfoExtPtr         srvInfoPtr);
/*
 * Copy data from the given addrinfo structure to the user's sockaddr_storage
 * structure
 */
static void tfDnsSrvSaveUserAddrInfo(
        struct addrinfo         TM_FAR * addrInfoPtr,
        struct sockaddr_storage TM_FAR * userAddrPtr);
/* Get the next best host for the given SRV request */
static ttAddrInfoExtPtr tfDnsSrvGetNextSrv(ttDnsCacheEntryPtr entryPtr,
                                           ttAddrInfoExtPtr   srvInfoPtr);
/*
 * Find the best unsorted host for the given SRV request. The chosen host is
 * marked as sorted
 */
static ttAddrInfoExtPtr tfDnsSrvGetRandomSrv(ttDnsCacheEntryPtr entryPtr);
/*
 * Given a sockaddr_storage structure from the user, find the matching
 * addrinfo structure for the given cache entry
 */
static struct addrinfo TM_FAR * tfDnsSrvFindHostFromSockAddr(
        struct sockaddr_storage TM_FAR * addrPtr,
        ttDnsCacheEntryPtr               entryPtr,
        ttAddrInfoExtPtr        TM_FAR * srvInfoPtrPtr);
/* Delete the given host from the list associated with the given cache entry */
static ttAddrInfoExtPtr tfDnsSrvDeleteSrv(ttDnsCacheEntryPtr entryPtr,
                                          ttAddrInfoExtPtr   srvInfoPtr);
/* Delete the weakest SRV host associated with the given cache entry */
static void tfDnsSrvDeleteWeakest(ttDnsCacheEntryPtr entryPtr);
#endif /* TM_DNS_USE_SRV */

/* Open the socket, set the appropriate options, and register the callback */
static int tfDnsOpenSocket(void);

/* Close the socket */
static void tfDnsCloseSocket(ttDnsCacheEntryPtr dnsCacheEntryPtr);

/* Close the socket and mark the cache entry as complete */
static void tfDnsCloseSocketComplete(ttDnsCacheEntryPtr dnsCacheEntryPtr);

/* Generate a new random Transaction ID for the new request. */
static tt16Bit tfDnsGetNewTransId(void);

/* List of socket types with associated protocol for getaddrinfo() */
static const int TM_CONST_QLF tlSockTypes[] =
{
#ifdef TM_USE_TCP
    SOCK_STREAM, IPPROTO_TCP,
#endif /* TM_USE_TCP */
    SOCK_DGRAM, IPPROTO_UDP,
#if defined(TM_USE_RAW_SOCKET) || defined(TM_6_USE_RAW_SOCKET)
    SOCK_RAW, 0,
#endif /* TM_USE_RAW_SOCKET || TM_6_USE_RAW_SOCKET */
    0
};


extern	int stricmp(const char *d, const char *s);


/*
 * tfDnsInit
 *
 * Initializes the DNS resolver service.  This should be called only once when
 * the system is initialized.
 *
 * 1. ERROR CHECKING: If blocking doesn't equal TM_BLOCKING_ON or
 *    TM_BLOCKING_OFF, return TM_EINVAL.
 * 2. ERROR CHECKING: If tvDnsInitialized isn't set to
 *    TM_8BIT_NO then the resolver service has already been started,
 *    so return TM_EALREADY.
 * 3. Set all entries in DNS server list to zero.
 * 4. Initialize cache entry pointers to zero.
 * 5. Set defaults for option values.
 * 6. Clear DNS lock entry.
 * 7. Set tvDnsInitialized to TM_8BIT_YES.
 * 8. Initialize host table support.
 *
 * Parameter    Description
 * blockingMode Specifies whether the resolver should operate in blocking or
 *              non-blocking mode (TM_BLOCKING_ON or TM_BLOCKING_OFF)
 *
 * Returns
 * Value       Meaning
 * TM_EINVAL   blockingMode not set to either TM_BLOCKING_ON or TM_BLOCKING_OFF
 * TM_EALREADY The DNS resolver has already been started.
 * TM_ENOERROR Resolver started successfully.
 * TM_ENOMEM   Insufficient memory to complete the operation.
 */
int tfDnsInit(int blockingMode)
{
    int                 errorCode;
    tt8Bit              serverIdx;


    errorCode = TM_ENOERROR;

/*
 * 1. ERROR CHECKING: If tvDnsInitialized isn't set to
 *    TM_8BIT_NO then the resolver service has already been started,
 *    so return TM_EALREADY.
 */
    if (tm_context(tvDnsInitialized) == TM_8BIT_YES)
    {
        errorCode = TM_EALREADY;
    }

/*
 * 2. ERROR CHECKING: If blocking doesn't equal TM_BLOCKING_ON or
 *    TM_BLOCKING_OFF, return TM_EINVAL.
 */
    if (errorCode == TM_ENOERROR)
    {
        if (   (blockingMode == TM_BLOCKING_ON)
            || (blockingMode == TM_BLOCKING_OFF))
        {
            tm_context(tvDnsBlockingMode) = blockingMode;
        }
        else
        {
            errorCode = TM_EINVAL;
        }
    }

    if (errorCode == TM_ENOERROR)
    {
/* 3. Set all entries in DNS server list to zero. */
        for (serverIdx = 0; serverIdx < TM_DNS_MAX_SERVERS; serverIdx++)
        {
            tm_bzero(&(tm_context(tvDnsServerList[serverIdx])),
                     sizeof(ttDnsServerEntry));
        }

/* 4. Initialize cache entry pointer to zero. */
        tm_context(tvDnsCachePtr)     = (ttDnsCacheEntryPtr) 0;
        tm_context(tvDnsCacheEntries) = 0;

/* 5. Set defaults for option values */
        tm_context(tvDnsCacheSize)        = TM_DNS_CACHE_SIZE;
        tm_context(tvDnsCacheTtl)         = TM_DNS_MAX_TTL;
        tm_context(tvDnsRetries)          = TM_DNS_RETRIES;
        tm_context(tvDnsTimeout)          = TM_DNS_TIMEOUT;
        tm_context(tvDnsMaxNumberSockets) = TM_DNS_MAX_SOCKETS_ALLOWED;

/* 6. Clear DNS lock entry. */
#ifdef TM_LOCK_NEEDED
        tm_bzero( &tm_context(tvDnsLockEntry),
                    sizeof(tm_context(tvDnsLockEntry)) );
#endif /* TM_LOCK_NEEDED */

/* 7. Initialize host table support. */
        tfListInit(&tm_context(tvDnsHostTable));
        tm_context(tvDnsHostTableSearchFuncPtr)
                = (ttDnsHostTableSearchFuncPtr)0;
#ifdef TM_LOCK_NEEDED
        tm_bzero(&tm_context(tvDnsHostTableLockEntry), sizeof(ttLockEntry));
#endif /* TM_LOCK_NEEDED */

        {
/* 8. Set tvDnsInitialized to TM_8BIT_YES. */
            tm_context(tvDnsInitialized) = TM_8BIT_YES;
        }
    }

    return errorCode;
}

/*
 * tfDnsSetUserOption
 *
 * Function Description
 * Sets various options for the DNS resolver (cache size, timeouts, etc).
 *
 * Parameter      Description
 * optionType     Option to set: TM_DNS_OPTION_CACHE_SIZE,
 *                               TM_DNS_OPTION_RETRIES,
 *                               TM_DNS_OPTION_TIMEOUT,
 *                               TM_DNS_OPTION_CACHE_TTL,
 *                               TM_DNS_OPTION_BINDTODEVICE
 * optionValuePtr Pointer to the value for above option
 * optionLen      Length, in bytes, of the value pointed to by optionValuePtr
 *
 * Returns
 * Value          Meaning
 * TM_ENOPROTOOPT Unknown option type
 * TM_EINVAL      Invalid value for this option
 * TM_ENOERROR    DNS option set successfully.
 *
 */

int tfDnsSetUserOption(int    optionType,
                       void * optionValuePtr,
                       int    optionLen)
{
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr    devPtr;
#endif /* TM_USE_STRONG_ESL */
    tt32Bit             opt32U;
    int                 errorCode;
    int                 optIntS;

    errorCode = TM_ENOERROR;
    optIntS   = 0;
    opt32U    = 0;
#ifdef TM_USE_STRONG_ESL
    devPtr    = (ttDeviceEntryPtr)0;
#endif /* TM_USE_STRONG_ESL */

    switch (optionType)
    {
        case TM_DNS_OPTION_CACHE_SIZE:
        case TM_DNS_OPTION_RETRIES:
        case TM_DNS_OPTION_TIMEOUT:
            if (optionLen == (int)sizeof(int))
            {
                tm_bcopy(optionValuePtr, &optIntS, optionLen);
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;
        case TM_DNS_OPTION_CACHE_TTL:
        case TM_DNS_OPTION_MAX_SOCKETS_ALLOWED:
            if (optionLen == (int)sizeof(tt32Bit))
            {
                tm_bcopy(optionValuePtr, &opt32U, optionLen);
            }
            else
            {
                errorCode = TM_EINVAL;
            }
            break;
#ifdef TM_USE_STRONG_ESL
        case TM_DNS_OPTION_BINDTODEVICE:
            if (optionValuePtr == (void *)0)
            {
                errorCode = TM_EINVAL;
            }
            break;
#endif /* TM_USE_STRONG_ESL */
        default:
            errorCode = TM_ENOPROTOOPT;
    }

    if (errorCode == TM_ENOERROR)
    {
/* Lock the DNS global variables */
        tm_call_lock_wait(&tm_context(tvDnsLockEntry));

        switch (optionType)
        {

/* Maximum size of DNS cache */
            case TM_DNS_OPTION_CACHE_SIZE:
                if (optIntS > 0)
                {
                    tm_context(tvDnsCacheSize) = optIntS;
                }
                else
                {
                    errorCode = TM_EINVAL;
                }
                break;

/* Number of times to retransmit a DNS request */
            case TM_DNS_OPTION_RETRIES:
                if (optIntS >= 0)
                {
                    tm_context(tvDnsRetries) = optIntS;
                }
                else
                {
                    errorCode = TM_EINVAL;
                }
                break;

/* DNS timeout, in seconds */
            case TM_DNS_OPTION_TIMEOUT:
                if (optIntS >= 0)
                {
                    tm_context(tvDnsTimeout) = optIntS;
                }
                else
                {
                    errorCode = TM_EINVAL;
                }
                break;
/* DNS cache TTL, in seconds */
            case TM_DNS_OPTION_CACHE_TTL:
                if (opt32U <= (0xFFFFFFFFUL / 1000))
                {
                    tm_context(tvDnsCacheTtl) = opt32U;
                }
                else
                {
                    errorCode = TM_EINVAL;
                }
                break;
            case TM_DNS_OPTION_MAX_SOCKETS_ALLOWED:
                if (   (opt32U > 0)
                    && (opt32U <= tm_context(tvMaxNumberSockets)) )
                {
                    tm_context(tvDnsMaxNumberSockets) = opt32U;
                }
                else
                {
                    errorCode = TM_EINVAL;
                }
#ifdef TM_USE_STRONG_ESL
            case TM_DNS_OPTION_BINDTODEVICE:
                devPtr = (ttDeviceEntryPtr)
                                tfInterfaceNameToDevice(optionValuePtr);
                if (   (devPtr == (ttDeviceEntryPtr)0)
                    && (*((char *)(optionValuePtr)) != '\0') )
                {
                    errorCode = TM_EINVAL;
                }
                else
                {
                    tm_context(tvDnsDevPtr) = devPtr;
                }
                break;
#endif /* TM_USE_STRONG_ESL */
            default:
                break;
        }

        tm_call_unlock(&tm_context(tvDnsLockEntry));

    }

    return errorCode;
}

/*
 * tfDnsSetOption
 *
 * Wrapper for obsolete function
 */

int tfDnsSetOption(int optionType, int optionValue)
{
    int optVal;

    optVal = optionValue;

    return tfDnsSetUserOption(optionType,
                              &optVal,
                              sizeof(int));
}

/*
 * tfNgDnsSetServer Function Description
 *
 * Sets the address of the primary and secondary DNS server.  To set the
 * primary DNS server serverNumber should be set to TM_DNS_PRI_SERVER; for the
 * secondary server it should be set to TM_DNS_SEC_SERVER.  To remove a
 * previously set entry, set serverAddrPtr to NULL.
 *
 * Parameters
 * Parameter      Description
 * serverAddrPtr  Pointer to the address of the IPv4 or IPv6 address of the
 *                specified DNS server.
 * serverNumber   TM_DNS_PRI_SERVER, TM_DNS_SEC_SERVER orTM_DNS_TER_SERVER
 *
 * Returns
 * Value        Meaning
 * TM_EINVAL    Server number is not TM_DNS_PRI_SERVER, TM_DNS_SEC_SERVER or
 *              TM_DNS_TER_SERVER.
 *
 * 1. Verify that the specified server number is valid.
 * 2. Lock the DNS server list.
 * 3. If the address pointer is NULL, the user wants to reset this entry, so
 *    clear out the specified entry.
 * 4. Otherwise, the user is setting a new server address:
 *     4.1. Copy the address information into the DNS server list.
 *     4.2. Set the port number for this entry.
 * 5. Unlock the DNS server list.
 */
int tfNgDnsSetServer(struct sockaddr_storage TM_FAR * serverAddrPtr,
                     int                              serverNumber)
{
    ttDnsServerEntryPtr dnsServerEntryPtr;
    int                 errorCode;
#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
    tt4IpAddress        ip4Addr;
#endif /* TM_USE_IPV6 && TM_USE_IPV4 */

/* 1. Verify that the specified server number is valid. */
    if ( ( (serverNumber+1) > TM_DNS_MAX_SERVERS) || (serverNumber < 0) )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
/* 2. Lock the DNS server list. */
        tm_call_lock_wait(&tm_context(tvDnsLockEntry));

/*
 * 3. If the address pointer is NULL, the user wants to reset this entry, so
 *    clear out the specified entry.
 */
        dnsServerEntryPtr = &(tm_context(tvDnsServerList[serverNumber]));
        if (serverAddrPtr == (struct sockaddr_storage TM_FAR *) 0)
        {
            tm_bzero(dnsServerEntryPtr, sizeof(ttDnsServerEntry));
        }
        else
        {
/* 4. Otherwise, the user is setting a new server address: */
/*     4.1. Copy the address information into the DNS server list. */
            tm_bcopy(serverAddrPtr,
                     &(dnsServerEntryPtr->dseServerAddress),
                     sizeof(struct sockaddr_storage));
            dnsServerEntryPtr->dseFailures = 0;

#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
/* If both IPv6 and IPv4 are enabled, and the user is setting up an IPv4
   DNS server, we need to set the address as an IPv4-mapped IPv6 address
   so that it works properly with a PF_INET6 socket. */
            if (serverAddrPtr->ss_family == AF_INET)
            {
                tm_ip_copy(dnsServerEntryPtr->dseServerAddress.
                                            addr.ipv4.sin_addr.s_addr,
                           ip4Addr);
                tm_bzero(&(dnsServerEntryPtr->dseServerAddress),
                         sizeof(struct sockaddr_storage));

                dnsServerEntryPtr->dseServerAddress.ss_family = AF_INET6;
                tm_6_addr_to_ipv4_mapped(ip4Addr,
                                         &(dnsServerEntryPtr->
                                           dseServerAddress.
                                           addr.ipv6.sin6_addr));
                dnsServerEntryPtr->dseServerAddress.ss_len =
                                            sizeof(struct sockaddr_storage);
            }
#endif /* TM_USE_IPV6 && TM_USE_IPV4 */


/*     4.2. Set the port number for this entry. */
            tm_htons(TM_DNS_SERVER_PORT,
                     dnsServerEntryPtr->dseServerAddress.ss_port);
        }

/* 5. Unlock the DNS server list. */
        tm_call_unlock(&tm_context(tvDnsLockEntry));

        errorCode = TM_ENOERROR;
    }

    return errorCode;
}


/*
 * tfDnsSetServer
 *
 * Function Description
 * Sets the address of the primary and secondary DNS server.  To set the
 * primary DNS server serverNumber should be set to TM_DNS_PRI_SERVER; for the
 * secondary server it should be set to TM_DNS_SEC_SERVER.  To remove a
 * previously set entry, set serverIpAddr to zero.
 * DEPRECATED: use tfNgDnsSetServer
 *
 * Parameter     Description
 * serverIpAddr  IP address of the DNS server
 * serverNumber  Primary or secondary server
 *
 * Returns
 * Value        Meaning
 * TM_EINVAL    serverNumber is not TM_DNS_PRI_SERVER orTM_DNS_SEC_SERVER.
 * TM_ENOERROR  DNS server set successfully.
 *
 */
#ifdef TM_USE_IPV4
int tfDnsSetServer(ttUserIpAddress serverIpAddr, int serverNumber)
{
    struct sockaddr_storage serverAddr;
    int errorCode;

    errorCode = TM_ENOERROR;
    if ( serverIpAddr == (ttUserIpAddress)0)
    {
        errorCode = tfNgDnsSetServer((struct sockaddr_storage*)0,
                                     serverNumber);
    }
    else
    {
        tm_ip_copy(serverIpAddr, serverAddr.addr.ipv4.sin_addr.s_addr);
        serverAddr.ss_len = sizeof(struct sockaddr_in);
        serverAddr.ss_family = AF_INET;
        errorCode = tfNgDnsSetServer(&serverAddr, serverNumber);
    }
    return errorCode;
}

/*
 * tfDnsGetHostByName
 *
 * Function Description
 * Returns the IP address associated with the given hostname.  If operating in
 * non-blocking mode, tfDnsGetHostByName should be called repeatedly for the
 * same request until it returns any value other than TM_EWOULDBLOCK.
 *
 * 1. Call tfDnsGenerateQuery to build and send a A type query for the hostname.
 * 2. If no error is returned, set the IP address parameter to the value
 *    returned from the query; otherwise set it to zero.
 *
 * Parameter   Description
 * hostnameStr Hostname to resolve.
 * ipAddressPtr    Set to the IP address of the host.
 *
 * Returns
 * Value           Meaning
 * TM_EINVAL       Invalid host name string or IP address pointer.
 * TM_EPERM        tfDnsInit() has not yet been called.
 * TM_EWOULDBLOCK  DNS lookup in progress.  The user should continue to call
 *                 tfDnsGetHostByName with the same parameters until it returns
 *                 a value other than TM_EWOULDBLOCK.
 * TM_ENOERROR     DNS lookup successful, IP address stored in *ipAddressPtr.
 *
 */

int tfDnsGetHostByName( const char TM_FAR * hostnameStr,
                        ttUserIpAddressPtr  ipAddressPtr )
{
    ttSockAddrPtrUnion sockAddr;
    ttDnsCacheEntryPtr entryPtr;
    ttUserIpAddress    ipAddr;
    int                errorCode;


    if (    (ipAddressPtr == (tt4IpAddressPtr) 0)
         || (hostnameStr == (const char TM_FAR *) 0) )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
/* 0. Lock the DNS global variables */
        tm_call_lock_wait(&tm_context(tvDnsLockEntry));

/*
 * 1. Call tfDnsGenerateQuery to build and send a A type query for the hostname.
 */
        errorCode = tfDnsGenerateQuery( hostnameStr,
                                        TM_DNS_QTYPE_A,
                                        AF_INET,
                                        &entryPtr);

        if (errorCode == TM_ENOERROR)
        {
            tm_assert(tfDnsGetHostByName, entryPtr != 0);
/* it is possible that our query failed to return any address */
            errorCode = entryPtr->dnscErrorCode;
        }

        if ( errorCode == TM_ENOERROR )
        {
/*
 * 2. If no error is returned, set the IP address parameter to the value
 *    returned from the query; otherwise set it to zero.
 */
            tm_assert(tfDnsGetHostByName,
                      entryPtr->dnscAddrInfoPtr != TM_ADDRINFO_NULL_PTR);

            sockAddr.sockPtr = entryPtr->dnscAddrInfoPtr->ai_addr;
            tm_ip_copy( sockAddr.sockInPtr->sin_addr.s_addr, ipAddr);
/* return network byte order */
            *ipAddressPtr = ipAddr;
        }
        else
        {
            tm_ip_copy ( TM_IP_ZERO, *ipAddressPtr );
        }
/* Unlock the DNS global lock */
        tm_call_unlock(&tm_context(tvDnsLockEntry));
    }

    if (   (errorCode == TM_DNS_EANSWER)
        && (tm_context(tvDnsBlockingMode) == TM_BLOCKING_OFF))
    {
        errorCode = TM_EWOULDBLOCK;
    }
    return errorCode;
}

/*
 * tfDnsGetHostByAddr
 *
 * Returns the hostname associated with the given IP address.  If operating in
 * non-blocking mode, tfDnsGetHostByAddr should be called repeatedly for the
 * same request until it returns any value other than TM_EWOULDBLOCK.
 *
 *
 * 1.  Convert the IP address into a IN-ADDR.ARPA hostname format (10.2.3.4
 *     becomes "4.3.2.10.IN-ADDR.ARPA").
 * 2.  Call tfDnsGenerateQuery to build and send a reverse query.
 * 3.  If no error is returned set the hostname length parameter and copy the
 *     string returned from the query into the buffer provided by the user;
 *     otherwise return the error to the user.
 *
 * Parameter          Description
 * serverIpAddr       IP address to retrieve the hostname for.
 * hostnameStr        Buffer to place the retrieved hostname in.
 * hostnameStrLength  Size of the above buffer.
 *
 * Returns
 * Value           Meaning
 * TM_EINVAL       Invalid host name string or IP address pointer.
 * TM_EPERM        tfDnsInit() has not yet been called.
 * TM_EWOULDBLOCK  DNS lookup in progress.  The user should continue to call
 *                 tfDnsGetHostByAddr with the same parameters until it returns
 *                 a value other than TM_EWOULDBLOCK.
 * TM_ENOBUFS      Insufficient memory to complete the operation.
 * TM_ENOERROR     DNS lookup successful, hostname stored in hostnameStr.
 */
int tfDnsGetHostByAddr( ttUserIpAddress ipAddr,
                        char TM_FAR *   hostnameStr,
                        int             hostnameStrLength )
{
    ttDnsCacheEntryPtr entryPtr;
    int                errorCode;
    ttCharPtr          arpaAddrStr;
    tt4IpAddress       reversedIpAddr;
    int                addrLength;

/* 0. Lock the DNS global variables */
    tm_call_lock_wait(&tm_context(tvDnsLockEntry));

    errorCode = TM_ENOERROR;

    if (hostnameStr == (char TM_FAR *) 0)
    {
        errorCode = TM_EINVAL;
    }

/*
 * 1.  Convert the IP address into a IN-ADDR.ARPA hostname format (10.2.3.4
 *     becomes "4.3.2.10.IN-ADDR.ARPA").
 */
    arpaAddrStr = (ttCharPtr)tm_get_raw_buffer((ttPktLen)TM_MAX_IN_ARPA_LENGTH);

    if (arpaAddrStr != (ttCharPtr) 0)
    {
/*
 * Reverse numerical IP address, convert it to ASCII and place at start of
 * string buffer.
 */

        reversedIpAddr = ( (ipAddr >> 24) & TM_UL(0x000000ff) ) |
                         ( (ipAddr >> 8)  & TM_UL(0x0000ff00) ) |
                         ( (ipAddr << 8)  & TM_UL(0x00ff0000) ) |
                         ( (ipAddr << 24) & TM_UL(0xff000000) );

        tfInetToAscii( reversedIpAddr, arpaAddrStr );
        addrLength = (int)tm_strlen( arpaAddrStr );

/* Copy the in-arpa domain specifier to end of reversed IP address */
        tm_strcpy( arpaAddrStr + addrLength, ".in-addr.arpa" );
    }
    else
    {
/* Not enough memory to allocate temporary string buffer for IN-ARPA hostname */
        errorCode = TM_ENOBUFS;
    }

/* 2.  Call tfDnsGenerateQuery to build and send a reverse query. */
    if (errorCode == TM_ENOERROR)
    {
        errorCode = tfDnsGenerateQuery( arpaAddrStr,
                                        TM_DNS_QTYPE_REV,
                                        AF_INET,
                                        &entryPtr);
/*
 * 3.  If no error is returned set the hostname length parameter and copy the
 *     string returned from the query into the buffer provided by the user;
 *     otherwise return the error to the user.
 */
        if (errorCode == TM_ENOERROR)
        {
            if (entryPtr->dnscErrorCode == TM_ENOERROR)
            {
                (void)tm_strncpy( hostnameStr,
                                  entryPtr->dnscRevHostnameStr,
                                  hostnameStrLength );
            }
            else
            {
                errorCode = entryPtr->dnscErrorCode;
            }
        }
    }

/* Free the temporary string buffer used to form the ARPA reverse query */
    if (arpaAddrStr != (ttCharPtr) 0)
    {
        tm_free_raw_buffer( (ttRawBufferPtr) arpaAddrStr );
    }
/* Unlock the DNS global lock */
    tm_call_unlock(&tm_context(tvDnsLockEntry));

    return errorCode;
}


/*
 * tfDnsGetNextMailHost
 *
 *
 * Returns the IP address of any additional mail server for this hostname.
 * If operating in non-blocking mode, tfDnsGetHostMX should be called repeatedly
 * for the same request until it returns any value other than TM_EWOULDBLOCK.
 *
 * 1. Call tfDnsGenerateQuery to build and send a MX type query.
 * 2. If no error was returned:
 *     2.1. Find the last MX record retrieved by the user, based on the
 *          preference and IP address of the last retrieved record.
 *     2.2. If there are no more MX records, return TM_DNS_EANSWER to the user.
 *     2.3. Otherwise, get the information from this addrinfo structure and
 *          return it to the user.
 *
 * Parameter       Description
 * hostnameStr     Hostname to resolve.
 * lastIpAddress   IP address of the last retrieved mail host for this host.
 * lastPreference  Preference of the last retrieved mail host for this host.
 * ipAddressPtr    Set to the IP address of the host.
 *
 * Returns
 * Value           Meaning
 * TM_EINVAL       Invalid host name string or IP address pointer.
 * TM_EPERM        tfDnsInit() has not yet been called.
 * TM_EWOULDBLOCK  DNS lookup in progress.  The user should continue to call
 *                 tfDnsGetHostByName with the same parameters until it returns
 *                 a value other than TM_EWOULDBLOCK.
 * TM_ENOBUFS      Insufficient memory to complete the operation.
 * TM_ENOERROR     DNS lookup successful, IP address stored in *ipAddressPtr.
 */
int tfDnsGetNextMailHost(const char TM_FAR *     hostnameStr,
                         ttUserIpAddress         lastIpAddress,
                         unsigned short          lastPreference,
                         ttUserIpAddressPtr      ipAddressPtr,
                         unsigned short TM_FAR * mxPrefPtr)
{
    struct addrinfo TM_FAR *    addrInfoPtr;
    struct addrinfo TM_FAR *    prevAddrInfoPtr;
    ttSockAddrPtrUnion          sockAddr;
    ttSockAddrPtrUnion          prevSockAddr;
    struct addrinfo             hints;
    int                         errorCode;

    addrInfoPtr = TM_ADDRINFO_NULL_PTR;
    errorCode = TM_ENOERROR;

    if (   (hostnameStr == (const char TM_FAR *)0)
        || (lastIpAddress == (ttUserIpAddress)0)
        || (ipAddressPtr == (ttUserIpAddressPtr)0)
        || (mxPrefPtr == (unsigned short TM_FAR *)0) )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        prevAddrInfoPtr = tfDnsAllocAddrInfo();
        if (prevAddrInfoPtr == TM_ADDRINFO_NULL_PTR)
        {
            errorCode = TM_ENOBUFS;
        }
        else
        {
            prevSockAddr.sockPtr = prevAddrInfoPtr->ai_addr;
            prevSockAddr.sockNgPtr->addr.ipv4.sin_addr.s_addr = lastIpAddress;
            prevSockAddr.sockNgPtr->ss_family = AF_INET;
            prevSockAddr.sockNgPtr->ss_len = sizeof(struct sockaddr_storage);

            tm_bzero(&hints, sizeof(struct addrinfo));
            hints.ai_family = AF_INET;

            errorCode = tfNgDnsGetNextMailHost(hostnameStr,
                                               prevAddrInfoPtr,
                                               lastPreference,
                                               &hints,
                                               &addrInfoPtr);
            freeaddrinfo(prevAddrInfoPtr);
        }
    }

    if (errorCode == TM_ENOERROR)
    {
        tm_assert(tfDnsGetNextMailHost,
                  (addrInfoPtr != TM_ADDRINFO_NULL_PTR));
        sockAddr.sockPtr = addrInfoPtr->ai_addr;
        *ipAddressPtr = sockAddr.sockNgPtr->addr.ipv4.sin_addr.s_addr;
        *mxPrefPtr = (unsigned short)addrInfoPtr->ai_mxpref;
    }

    if (addrInfoPtr != TM_ADDRINFO_NULL_PTR)
    {
        freeaddrinfo(addrInfoPtr);
    }

    return errorCode;
}

/*
 * tfDnsGetMailHost
 *
 * Returns the IP address of the primary mail server for this hostname.  If
 * operating in non-blocking mode, tfDnsGetHostMX should be called repeatedly
 * for the same request until it returns any value other than TM_EWOULDBLOCK.
 *
 * 1. Call tfDnsGenerateQuery to build and send a MX type query.
 * 2. If no error is returned get the MX info from the first addrinfo
 *    structure and return it to the user.
 * 3. Do not remove cache entry.  (Previously, MX records were not cached.
 *    This has changed with the updated IPv6 design).
 *
 * Parameter    Description
 * hostnameStr  Hostname to resolve.
 * ipAddressPtr Set to the IP address of the mail host.
 * mxPrefPtr    Set to the preference value of this mail host.
 *
 * Returns
 * Value           Meaning
 * TM_EINVAL       Invalid host name string or IP address pointer.
 * TM_EPERM        tfDnsInit() has not yet been called.
 * TM_EWOULDBLOCK  DNS lookup in progress.  The user should continue to call
 *                 tfDnsGetHostByName with the same parameters until it returns
 *                 a value other than TM_EWOULDBLOCK.
 * TM_ENOERROR     DNS lookup successful, IP address stored in *ipAddressPtr.
*/

int tfDnsGetMailHost(const char TM_FAR *      hostnameStr,
                     ttUserIpAddressPtr       ipAddressPtr,
                     unsigned short TM_FAR *  mxPrefPtr)
{
    struct addrinfo TM_FAR *    addrInfoPtr;
    ttSockAddrPtrUnion          sockAddr;
    struct addrinfo             hints;
    int                         errorCode;

    addrInfoPtr = TM_ADDRINFO_NULL_PTR;
    errorCode = TM_ENOERROR;

    if (   (hostnameStr == (const char TM_FAR *)0)
        || (ipAddressPtr == (ttUserIpAddressPtr)0)
        || (mxPrefPtr == (unsigned short TM_FAR *)0) )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        tm_bzero(&hints, sizeof(struct addrinfo));
        hints.ai_family = AF_INET;
        errorCode = tfNgDnsGetMailHost(hostnameStr,
                                       &hints,
                                       &addrInfoPtr);
    }

    if (errorCode == TM_ENOERROR)
    {
        tm_assert(tfDnsGetMailHost,
                  (addrInfoPtr != TM_ADDRINFO_NULL_PTR));
        sockAddr.sockPtr = addrInfoPtr->ai_addr;
        *ipAddressPtr = sockAddr.sockNgPtr->addr.ipv4.sin_addr.s_addr;
        *mxPrefPtr = (unsigned short)addrInfoPtr->ai_mxpref;
    }

    if (addrInfoPtr != TM_ADDRINFO_NULL_PTR)
    {
        freeaddrinfo(addrInfoPtr);
    }

    return errorCode;
}
#endif /* TM_USE_IPV4 */

/*
 * tfNgDnsGetMailHost
 *
 * Returns the IP address of the primary mail server for this hostname.
 * If operating in non-blocking mode, tfNgDnsGetMailHost should be called
 * repeatedly for the same request until it returns any value other than
 * TM_EWOULDBLOCK.
 *
 * 1. Call tfDnsGenerateQuery to build and send a MX type query.
 * 2. If no error is returned get the MX info from the first addrinfo
 *    structure and return it to the user.
 * 3. Do not remove cache entry.  (Previously, MX records were not cached.
 *    This has changed with the updated IPv6 design).
 *
 * Parameter    Description
 * hostnameStr  Hostname to resolve.
 * hintsPtr     A pointer to a structure indicating which types of addresses
 *              the caller is interested in.
 * resPtrPtr    A pointer to the address of an addrinfo structure to store the
 *              address information of the retrieved mail host.
 *
 * Returns
 * Value           Meaning
 * TM_EINVAL       Invalid host name string or IP address pointer.
 * EAI_FAMILY      Invalid address specified with hintsPtr.
 * TM_EPERM        tfDnsInit() has not yet been called.
 * TM_EWOULDBLOCK  DNS lookup in progress.  The user should continue to call
 *                 tfNgDnsGetMailHost with the same parameters until it returns
 *                 a value other than TM_EWOULDBLOCK.
 * TM_ENOERROR     DNS lookup successful, IP address stored in ipAddrPtr.
*/
int tfNgDnsGetMailHost(const char TM_FAR *               hostnameStr,
                       const struct addrinfo TM_FAR *    hintsPtr,
                       struct addrinfo TM_FAR * TM_FAR * resPtrPtr)
{
    ttSockAddrPtrUnion       sockAddr;
    ttSockAddrPtrUnion       mailServerSockAddr;
    ttDnsCacheEntryPtr       entryPtr;
    ttDnsCacheEntryPtr       mailServerEntryPtr;
    struct addrinfo TM_FAR * addrInfoPtr;
    struct addrinfo TM_FAR * retAddrInfoPtr;
    int                      errorCode;
    int                      desiredAddrFamily;

    entryPtr = (ttDnsCacheEntryPtr)0;
    errorCode = TM_ENOERROR;
    desiredAddrFamily = AF_UNSPEC;

    if (   (hostnameStr == (const char TM_FAR *)0)
        || (resPtrPtr == (struct addrinfo TM_FAR * TM_FAR *)0) )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        *resPtrPtr = TM_ADDRINFO_NULL_PTR;

/* 0. Lock the DNS global variables */
        tm_call_lock_wait(&tm_context(tvDnsLockEntry));

        if (hintsPtr != (const struct addrinfo TM_FAR *)0)
        {
            if (   (hintsPtr->ai_family != AF_INET)
                && (hintsPtr->ai_family != AF_INET6))
            {
                errorCode = EAI_FAMILY;
            }
            else
            {
                desiredAddrFamily = hintsPtr->ai_family;
            }
        }
        else
        {
/* If IPv6 is enabled then the default protocol is AF_INET6 */
#ifdef TM_USE_IPV6
            desiredAddrFamily = AF_INET6;
#else /* TM_USE_IPV6 */
            desiredAddrFamily = AF_INET;
#endif /* TM_USE_IPV6 */
        }

        retAddrInfoPtr = tfDnsAllocAddrInfo();
        if (retAddrInfoPtr == TM_ADDRINFO_NULL_PTR)
        {
            errorCode = TM_ENOBUFS;
        }

        if (errorCode == TM_ENOERROR)
        {
/* 1. Call tfDnsGenerateQuery to build and send a MX type query. */
            errorCode = tfDnsGenerateQuery(hostnameStr,
                                           TM_DNS_QTYPE_MX,
                                           desiredAddrFamily,
                                           &entryPtr);
        }

        if (errorCode == TM_ENOERROR)
        {
            tm_assert(tfNgDnsGetMailHost, entryPtr != (ttDnsCacheEntryPtr)0);
/* it is possible that our query failed to return any MX record */
            errorCode = entryPtr->dnscErrorCode;
        }

/*
 * 2. If no error is returned get the MX info from the first addrinfo
 *    structure and return it to the user.
 */
        if (errorCode == TM_ENOERROR)
        {
            tm_assert(tfNgDnsGetMailHost,
                      entryPtr->dnscAddrInfoPtr != TM_ADDRINFO_NULL_PTR);

            addrInfoPtr = entryPtr->dnscAddrInfoPtr;

            if (addrInfoPtr->ai_mxhostname != (char TM_FAR *)0)
            {
/* we need to lookup the IP address of the mail server */
                errorCode = tfDnsGenerateQuery(
                    addrInfoPtr->ai_mxhostname,
                    TM_DNS_QTYPE_A,
                    desiredAddrFamily,
                    &mailServerEntryPtr);

                if (errorCode == TM_ENOERROR)
                {
                    tm_assert(tfNgDnsGetMailHost, mailServerEntryPtr != 0);
/* it is possible that our query failed to return any address */
                    errorCode = mailServerEntryPtr->dnscErrorCode;
                }

                if (errorCode == TM_EWOULDBLOCK)
                {
                    goto NG_DNS_GET_MAIL_HOST_UNLOCK;
                }

                if (errorCode == TM_ENOERROR)
                {
                    tm_assert(tfNgDnsGetMailHost,
                              mailServerEntryPtr->dnscAddrInfoPtr
                                            != TM_ADDRINFO_NULL_PTR);

/* cleanup the mailserver hostname to indicate we have resolved it */
                    tm_free_raw_buffer(
                        (ttRawBufferPtr)addrInfoPtr->ai_mxhostname);
                    addrInfoPtr->ai_mxhostname = (char TM_FAR *)0;

/* copy the IP address information to the original MX query entry */
                    mailServerSockAddr.sockPtr =
                        mailServerEntryPtr->dnscAddrInfoPtr->ai_addr;
                    sockAddr.sockPtr = addrInfoPtr->ai_addr;
                    tm_sockaddr_storage_copy(mailServerSockAddr.sockNgPtr,
                                             sockAddr.sockNgPtr);
                    addrInfoPtr->ai_addr->sa_family =
                            mailServerSockAddr.sockPtr->sa_family;
                }
            }

            if (errorCode == TM_ENOERROR)
            {
/*
 * If no hintsPtr was specified, the user prefers mxPref over address family.
 * Therefore, we need to return the answer with the highest preference value
 * regardless of the address family. Note that we initially sort answers with
 * matching mxPref values such that IPv6 addresses are first.
 * If the user provided a hintsPtr, then we need to make sure that the address
 * family of the answer matches before returning it.
 */
                if (hintsPtr != (const struct addrinfo TM_FAR *)0)
                {
                    while (addrInfoPtr != TM_ADDRINFO_NULL_PTR)
                    {
                        if (addrInfoPtr->ai_addr->sa_family ==
                                                        desiredAddrFamily)
                        {
                            break;
                        }
                        addrInfoPtr = addrInfoPtr->ai_next;
                    }
                }
                if (addrInfoPtr == TM_ADDRINFO_NULL_PTR)
                {
                    errorCode = TM_DNS_EANSWER;
                }
                else
                {
                    (void)tfCopyAddrInfo(addrInfoPtr,
                                         retAddrInfoPtr);
                    retAddrInfoPtr->ai_mxpref = addrInfoPtr->ai_mxpref;
                    *resPtrPtr = retAddrInfoPtr;
                    errorCode = entryPtr->dnscErrorCode;
                }
            }
            else
            {
                errorCode = TM_DNS_EANSWER;
            }
        }
/*
 * 3. Do not remove cache entry.  (Previously, MX records were not cached.
 *    This has changed with the updated IPv6 design).
 */

NG_DNS_GET_MAIL_HOST_UNLOCK:
/* Clean up retAddrInfoPtr if we are not returning it */
        if (   (*resPtrPtr == TM_ADDRINFO_NULL_PTR)
            && (retAddrInfoPtr != TM_ADDRINFO_NULL_PTR) )
        {
            freeaddrinfo(retAddrInfoPtr);
        }

/* Unlock the DNS global lock */
        tm_call_unlock(&tm_context(tvDnsLockEntry));
    }

    return errorCode;
}

/*
 * tfNgDnsGetNextMailHost
 *
 *
 * Returns the IP address of any additional mail server for this hostname.
 * If operating in non-blocking mode, tfNgDnsGetNextMailHost should be called
 * repeatedly for the same request until it returns any value other than
 * TM_EWOULDBLOCK.
 *
 * 1. Call tfDnsGenerateQuery to build and send a MX type query.
 * 2. If no error was returned:
 *     2.1. Find the last MX record retrieved by the user, based on the
 *          preference and IP address of the last retrieved record.
 *     2.2. If there are no more MX records, return TM_DNS_EANSWER to the user.
 *     2.3. Otherwise, get the information from this addrinfo structure and
 *          return it to the user.
 *
 * Parameter       Description
 * hostnameStr     Hostname to resolve.
 * lastIpAddrPtr   IP address of the last retrieved mail host for this host.
 * lastPref        Preference of the last retrieved mail host for this host.
 * hintsPtr        A pointer to a structure indicating which types of addresses
 *                 the caller is interested in.
 * resPtrPtr       A pointer to the address of an addrinfo structure to store
 *                 address information of the retrieved mail host.
 *
 * Returns
 * Value           Meaning
 * TM_EINVAL       Invalid host name string or IP address pointer.
 * EAI_FAMILY      Invalid address specified with hintsPtr.
 * TM_EPERM        tfDnsInit() has not yet been called.
 * TM_EWOULDBLOCK  DNS lookup in progress.  The user should continue to call
 *                 tfNgDnsGetNextMailHost with the same parameters until it
 *                 returns a value other than TM_EWOULDBLOCK.
 * TM_ENOERROR     DNS lookup successful, IP address stored in ipAddrPtr.
 */
int tfNgDnsGetNextMailHost(const char TM_FAR *               hostnameStr,
                           struct addrinfo TM_FAR *          lastIpAddrPtr,
                           unsigned short                    lastPref,
                           const struct addrinfo TM_FAR *    hintsPtr,
                           struct addrinfo TM_FAR * TM_FAR * resPtrPtr)
{
    ttSockAddrPtrUnion       sockAddr;
    ttSockAddrPtrUnion       lastSockAddr;
    ttSockAddrPtrUnion       mailServerSockAddr;
    ttDnsCacheEntryPtr       entryPtr;
    ttDnsCacheEntryPtr       mailServerEntryPtr;
    struct addrinfo TM_FAR * addrInfoPtr;
    struct addrinfo TM_FAR * retAddrInfoPtr;
    int                      errorCode;
    int                      desiredAddrFamily;

    entryPtr = (ttDnsCacheEntryPtr)0;
    retAddrInfoPtr = TM_ADDRINFO_NULL_PTR;
    errorCode = TM_ENOERROR;
    desiredAddrFamily = AF_UNSPEC;

/* 0. Lock the DNS global variables */
    tm_call_lock_wait(&tm_context(tvDnsLockEntry));

    if (   (hostnameStr == (const char TM_FAR *)0)
        || (lastIpAddrPtr == TM_ADDRINFO_NULL_PTR)
        || (resPtrPtr == (struct addrinfo TM_FAR * TM_FAR *)0) )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        if (hintsPtr != (const struct addrinfo TM_FAR *)0)
        {
            if (   (hintsPtr->ai_family != AF_INET)
                && (hintsPtr->ai_family != AF_INET6) )
            {
                errorCode = EAI_FAMILY;
            }
            else
            {
                desiredAddrFamily = hintsPtr->ai_family;
            }
        }
        else
        {
/* If IPv6 is enabled then the default protocol is AF_INET6 */
#ifdef TM_USE_IPV6
            desiredAddrFamily = AF_INET6;
#else /* TM_USE_IPV6 */
            desiredAddrFamily = AF_INET;
#endif /* TM_USE_IPV6 */
        }
        lastSockAddr.sockPtr = lastIpAddrPtr->ai_addr;

        retAddrInfoPtr = tfDnsAllocAddrInfo();
        if (retAddrInfoPtr == TM_ADDRINFO_NULL_PTR)
        {
            errorCode = TM_ENOBUFS;
        }

        if (errorCode == TM_ENOERROR)
        {
/* 1.  Call tfDnsGenerateQuery to build and send a MX type query. */
            errorCode = tfDnsGenerateQuery(hostnameStr,
                                           TM_DNS_QTYPE_MX,
                                           desiredAddrFamily,
                                           &entryPtr);
        }

        if (errorCode == TM_ENOERROR)
        {
            tm_assert(tfNgDnsGetNextMailHost,entryPtr != (ttDnsCacheEntryPtr)0);
/* it is possible that our query failed to return any MX record */
            errorCode = entryPtr->dnscErrorCode;
        }

/* 2. If no error was returned: */
        if (errorCode == TM_ENOERROR)
        {
            tm_assert(tfNgDnsGetNextMailHost,
                      entryPtr->dnscAddrInfoPtr != TM_ADDRINFO_NULL_PTR);

            addrInfoPtr = entryPtr->dnscAddrInfoPtr;
/*
 *     2.1. Find the last MX record retrieved by the user, based on the
 *          preference and IP address of the last retrieved record.
 */
            while (addrInfoPtr != TM_ADDRINFO_NULL_PTR)
            {
                sockAddr.sockPtr = addrInfoPtr->ai_addr;
                if (    (tm_ip_match_sockaddr(*(sockAddr.sockNgPtr),
                                              *(lastSockAddr.sockNgPtr)))
                     && (addrInfoPtr->ai_mxpref == lastPref)
                     && (addrInfoPtr->ai_next != TM_ADDRINFO_NULL_PTR)
                     && (sockAddr.sockNgPtr->ss_family ==
                                lastSockAddr.sockNgPtr->ss_family) )
                {
                    addrInfoPtr = addrInfoPtr->ai_next;
                    break;
                }
                addrInfoPtr = addrInfoPtr->ai_next;
            }

/*
 * If no hintsPtr was specified, the user prefers mxPref over address family.
 * Therefore, we need to return the answer with the highest preference value
 * regardless of the address family. Note that we initially sort answers with
 * matching mxPref values such that IPv6 addresses are first.
 * If the user provided a hintsPtr, then we need to make sure that the address
 * family of the answer matches before returning it.
 */
            if (hintsPtr != (const struct addrinfo TM_FAR *)0)
            {
                while (addrInfoPtr != TM_ADDRINFO_NULL_PTR)
                {
                    sockAddr.sockPtr = addrInfoPtr->ai_addr;
                    if (sockAddr.sockNgPtr->ss_family == desiredAddrFamily)
                    {
                        break;
                    }
                    addrInfoPtr = addrInfoPtr->ai_next;
                }
            }

/* 2.2. If there are no more MX records, return TM_DNS_EANSWER to the user. */
            if (addrInfoPtr == TM_ADDRINFO_NULL_PTR)
            {
                errorCode = TM_DNS_EANSWER;
            }
            else
            {
/*
 * 2.3. Otherwise, get the information from this addrinfo structure and return
 *      it to the user.
 */
                if (addrInfoPtr->ai_mxhostname != (char TM_FAR *)0)
                {
/* we need to lookup the IP address of the mail server */
                    errorCode = tfDnsGenerateQuery(
                        addrInfoPtr->ai_mxhostname,
                        TM_DNS_QTYPE_A,
                        desiredAddrFamily,
                        &mailServerEntryPtr);

                    if (errorCode == TM_ENOERROR)
                    {
                        tm_assert(tfNgDnsGetNextMailHost,
                                  mailServerEntryPtr != 0);
/* it is possible that our query failed to return any address */
                        errorCode = mailServerEntryPtr->dnscErrorCode;
                    }

                    if (errorCode == TM_EWOULDBLOCK)
                    {
                        goto NG_DNS_GET_NEXT_MAIL_HOST_UNLOCK;
                    }

                    if (errorCode == TM_ENOERROR )
                    {
                        tm_assert(tfNgDnsGetNextMailHost,
                                  mailServerEntryPtr->dnscAddrInfoPtr
                                                != TM_ADDRINFO_NULL_PTR);

/* cleanup the mailserver hostname to indicate we have resolved it */
                        tm_free_raw_buffer(
                            (ttRawBufferPtr)addrInfoPtr->ai_mxhostname);
                        addrInfoPtr->ai_mxhostname = (char TM_FAR *) 0;

/* copy the IP address information to the original MX query entry */
                        mailServerSockAddr.sockPtr =
                        mailServerEntryPtr->dnscAddrInfoPtr->ai_addr;
                        sockAddr.sockPtr = addrInfoPtr->ai_addr;
                        tm_sockaddr_storage_copy(mailServerSockAddr.sockNgPtr,
                                                 sockAddr.sockNgPtr);
                        addrInfoPtr->ai_addr->sa_family =
                                mailServerSockAddr.sockPtr->sa_family;
                    }
                }

                if (errorCode == TM_ENOERROR)
                {
                    (void)tfCopyAddrInfo(addrInfoPtr,
                                         retAddrInfoPtr);
                    retAddrInfoPtr->ai_mxpref = addrInfoPtr->ai_mxpref;
                    *resPtrPtr = retAddrInfoPtr;
                    errorCode = entryPtr->dnscErrorCode;
                }
                else
                {
                    errorCode = TM_DNS_EANSWER;
                }
            }
        }
    }

NG_DNS_GET_NEXT_MAIL_HOST_UNLOCK:
/* Clean up retAddrInfoPtr */
    if (   (errorCode != TM_ENOERROR)
        && (retAddrInfoPtr != TM_ADDRINFO_NULL_PTR) )
    {
        freeaddrinfo(retAddrInfoPtr);
    }

/* Unlock the DNS global lock */
    tm_call_unlock(&tm_context(tvDnsLockEntry));

    return errorCode;
}

#ifdef TM_DNS_USE_SRV
/*
 * tfDnsGetSrvHost
 *
 * Return a sockaddr structure containing the best server for the given
 * service, protocol, and domain. If operating in non-blocking mode,
 * tfDnsGetSrvHost should be called repeatedly for the same request until it
 * returns a value other than TM_EWOULDBLOCK. This function can be called
 * repeatedly to retrieve multiple responses, sorted by priority and weight.
 * To get the next response, pass in a pointer to the sockaddr_storage
 * returned by the previous call.
 *
 * 1. Lock DNS global variables
 * 2. Validate parameters
 * 3. Translate request into SRV query format
 * 4. Call tfDnsGenerateQuery to build and send an SRV query
 * 5. Get the next best match, based on the sockaddr_storage passed in by the
 *    user
 * 6. Unlock the DNS global lock
 *
 * Parameter    Description
 * service      Pointer to a string containing the service (e.g., "http",
 *              "ftp", "telnet")
 * protocol     Pointer to a string containing the prootocol. Predefined
 *              values TM_DNS_SRV_PROTO_UDP and TM_DNS_SRV_PROTO_TCP
 *              supported
 * domain       Domain for request (e.g., "treck.com")
 * addrPtr      Pointer to a sockaddr_storage structure. The first time this
 *              function is called for a request, the structure must be zero'd
 *              out. The function will store the best result in the structure.
 *              For subsequent calls to get the next best result, the user
 *              must pass the previously returned structure back to this
 *              function.
 * flags        Used to specify the desired sort type. Valid values:
 *              TM_DNS_SRV_SORT_FAST
 *              TM_DNS_SRV_SORT_DUAL_PREF_IP4
 *              TM_DNS_SRV_SORT_DUAL_PREF_IP6
 *              TM_DNS_SRV_SORT_IP4_ONLY
 *              TM_DNS_SRV_SORT_IP6_ONLY
 *
 * Note on the cache entry structure used for SRV requests:
 * An entire SRV request, including all associated hosts and addresses, is
 * stored in a single DNS cache entry. The cache entry points to a list of
 * ttAddrInfoExt structures. Each ttAddrInfoExt structure represents an answer
 * returned by the SRV query. These are also referred to in the comments as
 * 'hosts'. Each host, in turn, has two linked lists of addrinfo structures,
 * one for IPv4 addresses, and another for IPv6 addresses. Two macros are used
 * to moderate the size of SRV requests, TM_DNS_SRV_HOSTS_PER_ENTRY and
 * TM_DNS_SRV_ADDRS_PER_HOST.
 *
 * Returns
 * Value              Meaning
 * TM_EINVAL          Invalid host name string or IP address pointer.
 * TM_EPERM           tfDnsInit() has not yet been called.
 * TM_EWOULDBLOCK     DNS lookup in progress.  The user should continue to call
 *                    tfDnsGetSrvHost with the same parameters until it returns
 *                    a value other than TM_EWOULDBLOCK.
 * TM_EPROTONOSUPPORT Asked for v6 addresses in a v4 only stack, or vice versa
 * TM_ENOMEM          Not enough memory to process request
 * TM_DNS_Exxx        DNS-specific error code based on server response
 * TM_ENOERROR        DNS SRV lookup successful, sockaddr stored in *addrPtr
 */

int tfDnsGetSrvHost(
    const char              TM_FAR * service,
    const char              TM_FAR * protocol,
    const char              TM_FAR * domainStr,
    struct sockaddr_storage TM_FAR * addrPtr,
    ttUser32Bit                      flags )
{
    ttDnsCacheEntryPtr entryPtr;
    char      TM_FAR * queryStr;
    tt32Bit            sort;
    int                errorCode;
    int                offset;
    int                serviceLen;
    int                domainLen;
    int                protoLen;

    errorCode = TM_ENOERROR;
    queryStr = (char TM_FAR *)0;

/* 1. Lock the DNS global variables */
    tm_call_lock_wait(&tm_context(tvDnsLockEntry));

/* 2. Validate parameters */
    if (   (service == (const char TM_FAR *)0)
        || (protocol == (const char TM_FAR *)0)
        || (addrPtr == (struct sockaddr_storage TM_FAR *)0)
        || (domainStr == (const char TM_FAR *)0))
    {
        errorCode = TM_EINVAL;
    }

    sort = flags & TM_DNS_SRV_SORT_MASK;

/* Make sure the user picked one (and only one) sort type */
    if (   (sort != TM_DNS_SRV_SORT_FAST)
        && (sort != TM_DNS_SRV_SORT_DUAL_PREF_IP4)
        && (sort != TM_DNS_SRV_SORT_DUAL_PREF_IP6)
        && (sort != TM_DNS_SRV_SORT_IP4_ONLY)
        && (sort != TM_DNS_SRV_SORT_IP6_ONLY))
    {
        errorCode = TM_EINVAL;
    }

#ifndef TM_USE_IPV4
/* If v4 isn't available, don't let the user ask for it */
    if (   (sort == TM_DNS_SRV_SORT_DUAL_PREF_IP4)
        || (sort == TM_DNS_SRV_SORT_DUAL_PREF_IP6)
        || (sort == TM_DNS_SRV_SORT_IP4_ONLY))
    {
        errorCode = TM_EPROTONOSUPPORT;
    }
#endif /* TM_USE_IPV4 */

#ifndef TM_USE_IPV6
/* If v6 isn't available, don't let the user ask for it */
    if (   (sort == TM_DNS_SRV_SORT_DUAL_PREF_IP6)
        || (sort == TM_DNS_SRV_SORT_DUAL_PREF_IP4)
        || (sort == TM_DNS_SRV_SORT_IP6_ONLY))
    {
        errorCode = TM_EPROTONOSUPPORT;
    }
#endif /* TM_USE_IPV6 */

/* 3. Translate request into SRV query format */
    if (errorCode == TM_ENOERROR)
    {
/* Get a buffer to hold the request string */
        serviceLen = (int)tm_strlen(service);
        domainLen = (int)tm_strlen(domainStr);
        protoLen = (int)tm_strlen(protocol);
        queryStr = tm_get_raw_buffer(serviceLen + domainLen + protoLen + 5);
        if (queryStr)
        {
/* Create the SRV request string: _service._protocol.domain.ext */
            offset = 0;
            queryStr[offset++] = '_';
            tm_memcpy(&queryStr[offset], service, serviceLen);
            offset += serviceLen;
            queryStr[offset++] = '.';
            queryStr[offset++] = '_';
            tm_memcpy(&queryStr[offset], protocol, protoLen);
            offset += protoLen;
            queryStr[offset++] = '.';
            tm_strncpy(&queryStr[offset], domainStr, domainLen + 1);
        }
        else
        {
            errorCode = TM_ENOMEM;
        }
    }

/* 4. Call tfDnsGenerateQuery to build and send an SRV query */
    entryPtr = (ttDnsCacheEntryPtr)0;
    if (errorCode == TM_ENOERROR)
    {
        errorCode = tfDnsGenerateQuery( queryStr,
                                        TM_DNS_QTYPE_SRV,
                                        AF_INET,
                                        &entryPtr);
    }

    if (errorCode == TM_ENOERROR)
    {
        tm_assert(tfDnsGetSrvHost, entryPtr != (ttDnsCacheEntryPtr)0);
/* It is possible that our query failed to return any SRV record */
        errorCode = entryPtr->dnscErrorCode;
    }

/*
 * 5. Get the next best match, based on the sockaddr_storage passed in
 *    by the user
 */
    if (errorCode == TM_ENOERROR)
    {
        tm_assert(tfDnsGetSrvHost,
                      entryPtr->dnscAddrInfoPtr != TM_ADDRINFO_NULL_PTR);

        if (entryPtr->dnscSrvSortedCount == 0)
        {
/*
 * If the SRV request has never been accessed before, make sure the user's
 * sockaddr_storage is zeroed out
 */
            tm_bzero(addrPtr, sizeof(struct sockaddr_storage));
        }

/* Get the next SRV host, based on the user's sockaddr_storage structure */
        errorCode = tfDnsSrvGetNextHost(entryPtr, addrPtr, sort);

        if (errorCode == TM_ENOERROR)
        {
            errorCode = entryPtr->dnscErrorCode;
        }
    }

/* 6. Unlock the DNS global lock */
    tm_call_unlock(&tm_context(tvDnsLockEntry));

/* Free any allocated memory */
    if (queryStr)
    {
        tm_free_raw_buffer(queryStr);
    }

    return errorCode;
}


/*
 * tfDnsSrvGetNextHost
 *
 * Get the next best address in the given cache entry, based on the previous
 * response returned to the user and the sort type requested. If the previous
 * response is empty (zero'd out), return the first response
 *
 * 1. Try to find a match for the previous response
 * 2. If a match is found, get the next address for the host
 * 3. If no address found, get the next SRV host
 * 4. Get the best address from the new SRV host
 * 5. Save the address to the user's structure
 *
 * Parameter    Description
 * entryPtr     Pointer to the DNS cache entry for this query
 * addrPtr      Pointer to the user's sockaddr_storage structure, into which
 *              the next address should be saved
 * sort         Sort type requested by the user. Valid values:
 *              TM_DNS_SRV_SORT_FAST
 *              TM_DNS_SRV_SORT_DUAL_PREF_IP4
 *              TM_DNS_SRV_SORT_DUAL_PREF_IP6
 *              TM_DNS_SRV_SORT_IP4_ONLY
 *              TM_DNS_SRV_SORT_IP6_ONLY
 *
 * Returns
 * Value              Meaning
 * TM_DNS_EANSWER     No more addresses found
 * TM_EWOULDBLOCK     No response yet, try again later
 * TM_ENOERROR        Next address found successfully
 */

static int tfDnsSrvGetNextHost(
        ttDnsCacheEntryPtr               entryPtr,
        struct sockaddr_storage TM_FAR * addrPtr,
        tt32Bit                          sort)
{
    ttAddrInfoExtPtr         curSrvInfoPtr;
    struct addrinfo TM_FAR * curInfoPtr;
#if defined(TM_USE_IPV6) && defined(TM_USE_IPV4)
    tt32Bit                  tempSort;
#endif /* defined(TM_USE_IPV6) && defined(TM_USE_IPV4) */
    int                      errCode;

    curInfoPtr = TM_ADDRINFO_NULL_PTR;
    curSrvInfoPtr = (ttAddrInfoExtPtr)0;
    errCode = TM_ENOERROR;

/* 1. Try to find a match for the previous response */
    if (addrPtr->addr.ipSockAddr.sa_family != 0)
    {
        curInfoPtr = tfDnsSrvFindHostFromSockAddr(addrPtr,
                                                  entryPtr,
                                                  &curSrvInfoPtr);
        if (curInfoPtr)
        {
/* 2. If a match is found, get the next address for the host */
            curInfoPtr = curInfoPtr->ai_next;
#if defined(TM_USE_IPV6) && defined(TM_USE_IPV4)
            if (!curInfoPtr)
            {
                tempSort = 0;
/* If there are no more v4 addresses, check if the user wants v6 */
                if (   (addrPtr->addr.ipSockAddr.sa_family == AF_INET)
                    && (sort == TM_DNS_SRV_SORT_DUAL_PREF_IP4))
                {
                    tempSort = TM_DNS_SRV_SORT_IP6_ONLY;
                }
/* Else if there are no more v6 addresses, check if the user wants v4 */
                else if (   (addrPtr->addr.ipSockAddr.sa_family == AF_INET6)
                        && (sort == TM_DNS_SRV_SORT_DUAL_PREF_IP6))
                {
                    tempSort = TM_DNS_SRV_SORT_IP4_ONLY;
                }
/* If the user will accept a different address type, get it */
                if (tempSort)
                {
                    curSrvInfoPtr->aie_ownercount++;
                    entryPtr->dnscCount++;
                    (void)tfDnsSrvGetFirstHost(entryPtr,
                                               curSrvInfoPtr,
                                               &curInfoPtr,
                                               tempSort);
                    entryPtr->dnscCount--;
                    curSrvInfoPtr->aie_ownercount--;
                }
            }
#endif /* defined(TM_USE_IPV6) && defined(TM_USE_IPV4) */
        }
        else
        {
            errCode = TM_DNS_EANSWER;
        }
    }

    if (errCode == TM_ENOERROR)
    {
/* 3. If no address found, get the next SRV host */
        while (!curInfoPtr)
        {
/* Get the next SRV host */
            curSrvInfoPtr = tfDnsSrvGetNextSrv(entryPtr, curSrvInfoPtr);
            if (curSrvInfoPtr)
            {
/* Get the first address for the given SRV host */
                curSrvInfoPtr->aie_ownercount++;
                entryPtr->dnscCount++;
/* 4. Get the best address from the new SRV host */
                errCode = tfDnsSrvGetFirstHost(entryPtr,
                                               curSrvInfoPtr,
                                               &curInfoPtr,
                                               sort);
                entryPtr->dnscCount--;
                curSrvInfoPtr->aie_ownercount--;
            }
            else
            {
                break;
            }
            if (errCode == TM_EWOULDBLOCK)
            {
                goto dnsSrvGetNextHostExit;
            }
            else if (errCode != TM_ENOERROR)
            {
/*
 * If this SRV host can't be resolved, delete it. Note: Delete only occurs if
 * attempts have already been made to resolve v4 and v6 addresses for this
 * host, and nothing was returned
 */
#if defined(TM_USE_IPV6) && defined(TM_USE_IPV4)
/* Make sure this host can be deleted */
                if (   (curSrvInfoPtr->aie_extflags & TM_DNS_SRV_FLAGS_QRY_IP4)
                    && (curSrvInfoPtr->aie_extflags & TM_DNS_SRV_FLAGS_QRY_IP6)
                    && (!(curSrvInfoPtr->aie_ip4addrs))
                    && (!(curSrvInfoPtr->aie_ip6addrs)))
#endif /* defined(TM_USE_IPV6) && defined(TM_USE_IPV4) */
                {
                    curSrvInfoPtr = tfDnsSrvDeleteSrv(entryPtr,
                                                      curSrvInfoPtr);
                }
            }
        }
    }

    if (curInfoPtr == TM_ADDRINFO_NULL_PTR)
    {
        errCode = TM_DNS_EANSWER;
    }

/* 5. Save the address to the user's structure */
    if (errCode == TM_ENOERROR)
    {
        tfDnsSrvSaveUserAddrInfo(curInfoPtr, addrPtr);
    }

dnsSrvGetNextHostExit:

    return errCode;
}


/*
 * tfDnsSrvGetFirstHost
 *
 * Get the best address for a host, given the host and the sort type
 * requested. If the previous response is empty (zero'd out), return the
 * first response
 *
 * 1. Configure the sort
 * 2. Try to get an IPv4 address
 *   2A. Check if we need to send a query
 *     2Aa. Send an A record query
 *     2Ab. Copy the response to our cache entry
 *     2Ac. Remove the A record from the cache
 *   2B. If we don't have an address yet, check if one's already saved on the
 *       structure
 * 3. Try to get an IPv6 address
 *   3A. Check if we need to send a query
 *     3Aa. Send an AAAA record query
 *     3Ab. Copy the response to our cache entry
 *     3Ac. Remove the AAAA record from the cache
 *   3B. If we don't have an address yet, check if one's already saved on the
 *       structure
 * 4. Reconfigure the sort for a second try, if necessary
 * 5. Save the address we got for the caller
 *
 * Parameter       Description
 * entryPtr        Pointer to the cache entry containing srvAddrInfoPtr
 * srvAddrInfoPtr  Pointer to an extended addrinfo struct, containing the SRV
 *                 info and hostname
 * firstHostPtrPtr Pointer to a pointer, to be filled out with the best
 *                 address (addrinfo structure) for the given ttAddrInfoExtPtr
 * sortType        Address sort type requested by user. Valid values:
 *                   TM_DNS_SRV_SORT_FAST
 *                   TM_DNS_SRV_SORT_DUAL_PREF_IP4
 *                   TM_DNS_SRV_SORT_DUAL_PREF_IP6
 *                   TM_DNS_SRV_SORT_IP4_ONLY
 *                   TM_DNS_SRV_SORT_IP6_ONLY
 *
 * Returns
 * Value              Meaning
 * TM_DNS_EANSWER     No address found for the given ttAddrInfoExtPtr
 * TM_EPERM           tfDnsInit() has not yet been called.
 * TM_EWOULDBLOCK     No response yet, try again later
 * TM_DNS_Exxx        DNS-specific error code, based on server response
 * TM_ENOERROR        First address found successfully
 */

static int tfDnsSrvGetFirstHost(
        ttDnsCacheEntryPtr                entryPtr,
        ttAddrInfoExtPtr                  srvAddrInfoPtr,
        struct addrinfo TM_FAR * TM_FAR * firstHostPtrPtr,
        tt32Bit                           sortType)
{
    ttDnsCacheEntryPtr       newEntryPtr;
    struct addrinfo TM_FAR * retAddr;
    struct addrinfo TM_FAR * curAddr;
    int                      errCode;
#if defined(TM_USE_IPV6) && defined(TM_USE_IPV4)
    int                      i;
    int                      loopCount;
    tt8Bit                   ip6Pref;
#endif /* defined(TM_USE_IPV6) && defined(TM_USE_IPV4) */

    errCode = TM_DNS_EANSWER;
    retAddr = TM_ADDRINFO_NULL_PTR;
    newEntryPtr = (ttDnsCacheEntryPtr)0;

#if defined(TM_USE_IPV6) && defined(TM_USE_IPV4)
/* 1. Configure the sort */
    if (sortType == TM_DNS_SRV_SORT_DUAL_PREF_IP6)
    {
/*
 * We only need to do two loops if the user wants IPv6 preferred. Each loop
 * tries IPv4, then IPv6. So, for v6, we have to skip the v4 the first time
 * through
 */
        ip6Pref = TM_8BIT_YES;
        loopCount = 2;
        sortType = TM_DNS_SRV_SORT_IP6_ONLY;
    }
    else
    {
/* Any other sort, we can handle in one loop */
        ip6Pref = TM_8BIT_NO;
        loopCount = 1;
        if (sortType == TM_DNS_SRV_SORT_FAST)
        {
/* If the user wants a fast sort, check what we already have */
            if (srvAddrInfoPtr->aie_ip4addrs)
            {
                sortType = TM_DNS_SRV_SORT_IP4_ONLY;
            }
            else if (srvAddrInfoPtr->aie_ip6addrs)
            {
                sortType = TM_DNS_SRV_SORT_IP6_ONLY;
            }
        }
    }

    for (i = 0; i < loopCount; i++)
#endif /* defined(TM_USE_IPV6) && defined(TM_USE_IPV4) */
    {
#ifdef TM_USE_IPV4
/* 2. Try to get an IPv4 address */
        if (sortType != TM_DNS_SRV_SORT_IP6_ONLY)
        {
/* 2A. Check if we need to send a query */
            if (   (!retAddr)
                && (!(srvAddrInfoPtr->aie_extflags & TM_DNS_SRV_FLAGS_QRY_IP4))
                && (!srvAddrInfoPtr->aie_ip4addrs)
                && (srvAddrInfoPtr->aie_addrcount < TM_DNS_SRV_ADDRS_PER_HOST))
            {
/* 2Aa. Send an A record query */
                errCode = tfDnsGenerateQuery(srvAddrInfoPtr->aie_srvhostname,
                                            TM_DNS_QTYPE_A,
                                            AF_INET,
                                            &newEntryPtr);

                if (errCode == TM_EWOULDBLOCK)
                {
                    goto dnsSrvGetFirstHostExit;
                }

/* Check if this query was completed by another task while we were blocked */
                if (!(srvAddrInfoPtr->aie_extflags & TM_DNS_SRV_FLAGS_QRY_IP4))
                {
/* Mark this query as complete */
                    srvAddrInfoPtr->aie_extflags |= TM_DNS_SRV_FLAGS_QRY_IP4;

                    if (errCode == TM_ENOERROR)
                    {
                        errCode = newEntryPtr->dnscErrorCode;
                    }

/* 2Ab. Copy the response to our cache entry */
                    if (errCode == TM_ENOERROR)
                    {
                        curAddr = newEntryPtr->dnscAddrInfoPtr;
                        while (   curAddr
                               && (  srvAddrInfoPtr->aie_addrcount
                                   < TM_DNS_SRV_ADDRS_PER_HOST))
                        {
                            errCode = tfDnsSrvSaveRecordInfo(entryPtr,
                                                             curAddr,
                                                             srvAddrInfoPtr);
                            curAddr = curAddr->ai_next;
                        }
                        retAddr = srvAddrInfoPtr->aie_ip4addrs;
                    }
                }
/* 2Ac. Remove the A record from the cache */
                if (newEntryPtr)
                {
                    tfDnsCacheRemove(newEntryPtr);
                    newEntryPtr = (ttDnsCacheEntryPtr)0;
                }
            }

/*
 * 2B. If we don't have an address yet, check if one's already saved on the
 *     structure
 */
            if (!retAddr)
            {
                if (srvAddrInfoPtr->aie_ip4addrs)
                {
                    retAddr = srvAddrInfoPtr->aie_ip4addrs;
                    errCode = TM_ENOERROR;
                }
                else
                {
                    errCode = TM_DNS_EANSWER;
                }
            }

        }
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
/* 3. Try to get an IPv6 address */
        if (sortType != TM_DNS_SRV_SORT_IP4_ONLY)
        {
/* 3A. Check if we need to send a query */
            if (   (!retAddr)
                && (!(srvAddrInfoPtr->aie_extflags & TM_DNS_SRV_FLAGS_QRY_IP6))
                && (!srvAddrInfoPtr->aie_ip6addrs)
                && (srvAddrInfoPtr->aie_addrcount < TM_DNS_SRV_ADDRS_PER_HOST))
            {
/* 3Aa. Send an AAAA record query */
                errCode = tfDnsGenerateQuery(srvAddrInfoPtr->aie_srvhostname,
                                            TM_DNS_QTYPE_AAAA,
                                            AF_INET6,
                                            &newEntryPtr);

                if (errCode == TM_EWOULDBLOCK)
                {
                    goto dnsSrvGetFirstHostExit;
                }

/* Check if this query was completed by another task while we were blocked */
                if (!(  srvAddrInfoPtr->aie_extflags
                      & TM_DNS_SRV_FLAGS_QRY_IP6))
                {
/* Mark this query as complete */
                    srvAddrInfoPtr->aie_extflags |= TM_DNS_SRV_FLAGS_QRY_IP6;

                    if (errCode == TM_ENOERROR)
                    {
                        errCode = newEntryPtr->dnscErrorCode;
                    }

/* 3Ab. Copy the response to our cache entry */
                    if (errCode == TM_ENOERROR)
                    {
                        curAddr = newEntryPtr->dnscAddrInfoPtr;
                        while (   curAddr
                               && (  srvAddrInfoPtr->aie_addrcount
                                   < TM_DNS_SRV_ADDRS_PER_HOST))
                        {
                            errCode = tfDnsSrvSaveRecordInfo(entryPtr,
                                                             curAddr,
                                                             srvAddrInfoPtr);
                            curAddr = curAddr->ai_next;
                        }
                        retAddr = srvAddrInfoPtr->aie_ip6addrs;
                    }
                }

/* 3Ac. Remove the AAAA record from the cache */
                if (newEntryPtr)
                {
                    tfDnsCacheRemove(newEntryPtr);
                    newEntryPtr = (ttDnsCacheEntryPtr)0;
                }
            }

/*
 * 3B. If we don't have an address yet, check if one's already saved on the
 *     structure
 */
            if (!retAddr)
            {
                if (srvAddrInfoPtr->aie_ip6addrs)
                {
                    retAddr = srvAddrInfoPtr->aie_ip6addrs;
                    errCode = TM_ENOERROR;
                }
                else
                {
                    errCode = TM_DNS_EANSWER;
                }
            }
        }
#endif /* TM_USE_IPV6 */
#if defined(TM_USE_IPV6) && defined(TM_USE_IPV4)
/* 4. Reconfigure the sort for a second try, if necessary */
        if (ip6Pref == TM_8BIT_YES)
        {
/*
 * Only need to do this if the user has requested IPv6 preferred.
 * If we've found an address on the first time through the loop, we can
 * break. Otherwise, configure a v4 search for the second time through the
 * loop
 */
            if (retAddr)
            {
                break;
            }
            else
            {
                sortType = TM_DNS_SRV_SORT_IP4_ONLY;
            }
        }
#endif /* defined(TM_USE_IPV6) && defined(TM_USE_IPV4) */
    }

dnsSrvGetFirstHostExit:

/* 5. Save the address we got for the caller */
    *firstHostPtrPtr = retAddr;

    return errCode;
}


/*
 * tfDnsSrvSaveRecordInfo
 *
 * Duplicate an address (addrinfo struct) and save it to an SRV host
 * (ttAddrInfoExt)
 *
 * 1. Allocate a new addrinfo struct
 * 2. Copy the data from the given addrinfo to the new one
 * 3. Insert new addrinfo into list
 *
 * Parameter   Description
 * addrInfoPtr Pointer to addrinfo struct to duplicate
 * srvInfoPtr  Pointer to ttAddrInfoExt to store the new addrinfo struct on
 *
 * Returns
 * Value       Meaning
 * TM_ENOMEM   Not enough memory to complete operation
 * TM_ENOERROR Successfully copied the addrinfo
 */

static int tfDnsSrvSaveRecordInfo(ttDnsCacheEntryPtr       entryPtr,
                                  struct addrinfo TM_FAR * addrInfoPtr,
                                  ttAddrInfoExtPtr         srvInfoPtr)
{
    ttSockAddrPtrUnion       sockAddr;
    ttSockAddrPtrUnion       newSockAddr;
    struct addrinfo TM_FAR * newAddrInfoPtr;
    struct addrinfo TM_FAR * tempAddrInfoPtr;
    ttAddrInfoExtPtr         tempSrvInfoPtr;
    int                      errorCode;
#ifdef TM_USE_IPDUAL
    tt8Bit                   isV4;
#endif /* TM_USE_IPDUAL */

/* 1. Allocate a new addrinfo struct */
    newAddrInfoPtr = tfDnsAllocAddrInfo();
    if (newAddrInfoPtr != TM_ADDRINFO_NULL_PTR)
    {
        errorCode = TM_ENOERROR;
        sockAddr.sockPtr = addrInfoPtr->ai_addr;
        newSockAddr.sockPtr = newAddrInfoPtr->ai_addr;
        errorCode = tfCopyAddrInfo(addrInfoPtr, newAddrInfoPtr);
        if (errorCode != TM_ENOERROR)
        {
            tfKernelError("tfDnsSrvSaveRecordInfo",
                          "tfCopyAddrInfo failed!");
        }
#ifdef TM_USE_IPDUAL
        if (sockAddr.sockPtr->sa_family == AF_INET)
        {
            newSockAddr.sockInPtr->sin_port = srvInfoPtr->aie_port;
            isV4 = TM_8BIT_YES;
        }
        else
        {
            newSockAddr.sockIn6Ptr->sin6_port = srvInfoPtr->aie_port;
            isV4 = TM_8BIT_NO;
        }
#endif /* TM_USE_IPDUAL */

/* Make sure this entry doesn't already exist */
        tempAddrInfoPtr = tfDnsSrvFindHostFromSockAddr(newSockAddr.sockNgPtr,
                                                       entryPtr,
                                                       &tempSrvInfoPtr);
        if (tempAddrInfoPtr)
        {
/* This entry already exists, so discard the new entry */
            freeaddrinfo(newAddrInfoPtr);
            newAddrInfoPtr = TM_ADDRINFO_NULL_PTR;
        }

        if (newAddrInfoPtr)
        {
/* 3. Insert new addrinfo into list */
            srvInfoPtr->aie_addrcount++;
#ifdef TM_USE_IPDUAL
            if (isV4 == TM_8BIT_YES)
#endif /* TM_USE_IPDUAL */
#ifdef TM_USE_IPV4
            {
                newAddrInfoPtr->ai_next = srvInfoPtr->aie_ip4addrs;
                srvInfoPtr->aie_ip4addrs = newAddrInfoPtr;
            }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPDUAL
            else
#endif /* TM_USE_IPDUAL */
#ifdef TM_USE_IPV6
            {
                newAddrInfoPtr->ai_next = srvInfoPtr->aie_ip6addrs;
                srvInfoPtr->aie_ip6addrs = newAddrInfoPtr;
            }
#endif /* TM_USE_IPV6 */
        }
    }
    else
    {
        errorCode = TM_ENOMEM;
    }

    return errorCode;
}


/*
 * tfDnsSrvSaveUserAddrInfo
 *
 * Save address information to the user's sockaddr_storage struct
 *
 * 1. Copy the address information to the user's struct
 *
 * Parameter   Description
 * addrInfoPtr addrinfo struct containing the address to give to the user
 * userAddrPtr Pointer to the user's sockaddr_storage struct
 *
 * Returns
 * None
 */

static void tfDnsSrvSaveUserAddrInfo(
        struct addrinfo         TM_FAR * addrInfoPtr,
        struct sockaddr_storage TM_FAR * userAddrPtr)
{

/* 1. Copy the address information to the user's struct */
    tm_bcopy(addrInfoPtr->ai_addr,
             userAddrPtr,
             sizeof(struct sockaddr_storage));
}


/*
 * tfDnsSrvGetNextSrv
 *
 * Get the next best SRV answer
 *
 * 1. If no current SRV answer, ge tthe first one
 * 2. Else, get the next one in the list
 * 3. If this SRV answer is being deleted, get the next one
 * 4. If the SRV answer we found hasn't been sorted yet, do some sorting
 *
 * Parameter   Description
 * entryPtr    Cache entry for the SRV query we're working with
 * srvInfoPtr  Starting SRV answer for the get next
 *
 * Returns
 * Value       Meaning
 * 0           No next SRV answer found
 * Non-null    Pointer to the next SRV answer
 */

static ttAddrInfoExtPtr tfDnsSrvGetNextSrv(ttDnsCacheEntryPtr entryPtr,
                                           ttAddrInfoExtPtr   srvInfoPtr)
{
    ttAddrInfoExtPtr retVal;

    retVal = (ttAddrInfoExtPtr)0;

    if (srvInfoPtr == (ttAddrInfoExtPtr)0)
    {
/* 1. If no current SRV answer, ge tthe first one */
        retVal = (ttAddrInfoExtPtr)entryPtr->dnscAddrInfoPtr;
    }
    else
    {
/* 2. Else, get the next one in the list */
        retVal = (ttAddrInfoExtPtr)srvInfoPtr->aie_next;
    }

    while (retVal)
    {
/* 3. If this SRV answer is being deleted, get the next one */
        if (retVal->aie_extflags & TM_DNS_SRV_FLAGS_DELETED)
        {
            retVal = (ttAddrInfoExtPtr)retVal->aie_next;
        }
        else
        {
            break;
        }
    }

    if (retVal)
    {
/* 4. If the SRV answer we found hasn't been sorted yet, do some sorting */
        if (!(retVal->aie_extflags & TM_DNS_SRV_FLAGS_SORTED))
        {
            retVal = tfDnsSrvGetRandomSrv(entryPtr);
        }
    }

    return retVal;
}


/*
 * tfDnsSrvGetRandomSrv
 *
 * Find the first SRV answer that hasn't been sorted yet. If there are
 * other unsorted answers at the same priority, apply the weighted random
 * sorting algorithm to pick one.
 *
 * 1. Find the first unsorted answer
 * 2. Calculate the total weight of all answers at this priority
 * 3. Pick a randomly weighted host
 * 4. Reorder the list for the selected host
 *
 * Parameter   Description
 * entryPtr    Cache entry for the SRV query we're working with
 *
 * Returns
 * Value       Meaning
 * 0           No next SRV answer found
 * Non-null    Pointer to the chosen SRV answer
 */

static ttAddrInfoExtPtr tfDnsSrvGetRandomSrv(ttDnsCacheEntryPtr entryPtr)
{
    ttAddrInfoExtPtr startAddrInfoPtr;
    ttAddrInfoExtPtr beforeStartAddrInfoPtr;
    ttAddrInfoExtPtr curAddrInfoPtr;
    ttAddrInfoExtPtr prevAddrInfoPtr;
    tt32Bit          totalWeight;
    tt32Bit          runningTotal;
    tt32Bit          rand;
    tt16Bit          i;
    tt16Bit          priority;

    startAddrInfoPtr = (ttAddrInfoExtPtr)entryPtr->dnscAddrInfoPtr;
    beforeStartAddrInfoPtr = (ttAddrInfoExtPtr)0;
    curAddrInfoPtr = (ttAddrInfoExtPtr)0;

/* 1. Find the first unsorted answer */
    for (i = 0; i < entryPtr->dnscSrvSortedCount; i++)
    {
        beforeStartAddrInfoPtr = startAddrInfoPtr;
        startAddrInfoPtr = (ttAddrInfoExtPtr)startAddrInfoPtr->aie_next;
    }

    if (startAddrInfoPtr)
    {
/* Note that we've sorted another of the answers */
        entryPtr->dnscSrvSortedCount++;
        curAddrInfoPtr = startAddrInfoPtr;
/* Get the current priority */
        priority = (tt16Bit)curAddrInfoPtr->aie_srvpri;
        totalWeight = 0;
/* 2. Calculate the total weight of all answers at this priority */
        while (curAddrInfoPtr->aie_srvpri == priority)
        {
            totalWeight = totalWeight + curAddrInfoPtr->aie_srvweight;
            curAddrInfoPtr = (ttAddrInfoExtPtr)curAddrInfoPtr->aie_next;
            if (!curAddrInfoPtr)
            {
                break;
            }
        }
/* 3. Pick a randomly weighted host */
        curAddrInfoPtr = startAddrInfoPtr;
        prevAddrInfoPtr = beforeStartAddrInfoPtr;
        rand = tfGetRandom() % (totalWeight + 1);
        runningTotal = curAddrInfoPtr->aie_srvweight;
        while (runningTotal < rand)
        {
            prevAddrInfoPtr = curAddrInfoPtr;
            curAddrInfoPtr = (ttAddrInfoExtPtr)curAddrInfoPtr->aie_next;
            runningTotal = runningTotal + curAddrInfoPtr->aie_srvweight;
        }
        curAddrInfoPtr->aie_extflags |= TM_DNS_SRV_FLAGS_SORTED;

/* 4. Reorder the list for the selected host */
        if (prevAddrInfoPtr != beforeStartAddrInfoPtr)
        {
            prevAddrInfoPtr->aie_next = curAddrInfoPtr->aie_next;
            curAddrInfoPtr->aie_next =
                    (struct addrinfo TM_FAR *)startAddrInfoPtr;
            if (beforeStartAddrInfoPtr)
            {
                beforeStartAddrInfoPtr->aie_next =
                        (struct addrinfo TM_FAR *)curAddrInfoPtr;
            }
            else
            {
                entryPtr->dnscAddrInfoPtr =
                        (struct addrinfo TM_FAR *)curAddrInfoPtr;
            }
        }
    }

    return curAddrInfoPtr;
}


/*
 * tfDnsSrvFindHostFromSockAddr
 *
 * Match the data in the user's sockaddr_storage structure to one of the
 * addrinfo structs associated with the given cache entry. Return both the
 * address (addrinfo) and SRV host (ttAddrInfoExt)
 *
 * 1. Loop through the SRV hosts
 * 2. Set the appropriate list of addresses to search
 * 3. Loop through all the addresses for this host
 * 4. Check if the current address matches the user's data
 *
 * Parameter     Description
 * addrPtr       Pointer to the user's address data
 * entryPtr      The SRV query (cache entry) we're working with
 * srvInfoPtrPtr Pointer to save the SRV host data pointer (if found) to
 *
 * Returns
 * Value       Meaning
 * 0           No matching addrinfo found
 * Non-null    The matching addrinfo
 */

static struct addrinfo TM_FAR * tfDnsSrvFindHostFromSockAddr(
        struct sockaddr_storage TM_FAR * addrPtr,
        ttDnsCacheEntryPtr               entryPtr,
        ttAddrInfoExtPtr        TM_FAR * srvInfoPtrPtr)
{
    ttAddrInfoExtPtr         curSrvInfoPtr;
    struct addrinfo TM_FAR * curAddrInfoPtr;
    struct addrinfo TM_FAR * retVal;
    ttSockAddrPtrUnion       sockAddr;
    int                      compare;

    sockAddr.sockNgPtr = addrPtr;
    curSrvInfoPtr = (ttAddrInfoExtPtr)entryPtr->dnscAddrInfoPtr;
    *srvInfoPtrPtr = (ttAddrInfoExtPtr)0;
    retVal = TM_ADDRINFO_NULL_PTR;

/* 1. Loop through the SRV hosts */
    while (curSrvInfoPtr && (!retVal))
    {
/* Save the current SRV host pointer */
        *srvInfoPtrPtr = curSrvInfoPtr;
/* 2. Set the appropriate list of addresses to search */
#if defined(TM_USE_IPV6) && defined(TM_USE_IPV4)
        if (sockAddr.sockPtr->sa_family == AF_INET)
        {
            curAddrInfoPtr = curSrvInfoPtr->aie_ip4addrs;
        }
        else
        {
            curAddrInfoPtr = curSrvInfoPtr->aie_ip6addrs;
        }
#else /* defined(TM_USE_IPV6) && defined(TM_USE_IPV4) */
#ifdef TM_USE_IPV4
        curAddrInfoPtr = curSrvInfoPtr->aie_ip4addrs;
#else /* TM_USE_IPV4 */
        curAddrInfoPtr = curSrvInfoPtr->aie_ip6addrs;
#endif /* TM_USE_IPV4 */
#endif /* defined(TM_USE_IPV6) && defined(TM_USE_IPV4) */
/* 3. Loop through all the addresses for this host */
        while (curAddrInfoPtr)
        {
/* 4. Check if the current address matches the user's data */
            compare = tm_memcmp(addrPtr,
                                curAddrInfoPtr->ai_addr,
                                sizeof(struct sockaddr_storage));
            if (!compare)
            {
/* We have a match, return this to the user */
                retVal = curAddrInfoPtr;
                break;
            }
            else
            {
                curAddrInfoPtr = curAddrInfoPtr->ai_next;
            }
        }
        curSrvInfoPtr = (ttAddrInfoExtPtr)curSrvInfoPtr->aie_next;
    }

    return retVal;
}


/*
 * tfDnsSrvDeleteSrv
 *
 * Delete the given SRV host and all associated data. The delete only happens
 * if both v4 and v6 addresses have been queried and no response was recieved.
 * Ths function returns the SRV host previous to the one that was deleted (or,
 * if no delete occurs, it returns the one passed in)
 *
 * 1. Mark this host as deleted
 * 2. Find the previous host
 * 3. Don't actually delete unless nobody else is using this host
 * 4. Reorder the list of hosts
 * 5. Free the memory
 * 6. If we can't delete, we want to return the given host, rather than the
 *    previous
 *
 * Parameter     Description
 * entryPtr      The SRV query (cache entry) we're working with
 * srvInfoPtr    Pointer to SRV host to delete
 *
 * Returns
 * Value       Meaning
 * Non-null    The previous SRV host to the one that was deleted. If the
 *             delete isn't allowed because all addresses haven't been
 *             queried yet, the SRV host passed in is returned.
 * 0           The deleted SRV host was the first, no previous one exists
 */

static ttAddrInfoExtPtr tfDnsSrvDeleteSrv(ttDnsCacheEntryPtr entryPtr,
                                          ttAddrInfoExtPtr   srvInfoPtr)
{
    ttAddrInfoExtPtr prevSrvInfoPtr;
    ttAddrInfoExtPtr curSrvInfoPtr;

/* 1. Mark this host as deleted */
    srvInfoPtr->aie_extflags |= TM_DNS_SRV_FLAGS_DELETED;

    curSrvInfoPtr = (ttAddrInfoExtPtr)entryPtr->dnscAddrInfoPtr;
    prevSrvInfoPtr = (ttAddrInfoExtPtr)0;
/* 2. Find the previous host */
    while (curSrvInfoPtr != srvInfoPtr)
    {
        prevSrvInfoPtr = curSrvInfoPtr;
        curSrvInfoPtr = (ttAddrInfoExtPtr)curSrvInfoPtr->aie_next;
    }

/* 3. Don't actually delete unless nobody else is using this host */
    if (srvInfoPtr->aie_ownercount == 0)
    {
        entryPtr->dnscSrvHostCount--;
        if (srvInfoPtr->aie_extflags & TM_DNS_SRV_FLAGS_SORTED)
        {
            entryPtr->dnscSrvSortedCount--;
        }

/* 4. Reorder the list of hosts */
        if (prevSrvInfoPtr)
        {
            prevSrvInfoPtr->aie_next = curSrvInfoPtr->aie_next;
        }
        else
        {
            entryPtr->dnscAddrInfoPtr = curSrvInfoPtr->aie_next;
        }
        curSrvInfoPtr->aie_next = TM_ADDRINFO_NULL_PTR;
/* 5. Free the memory */
        tfDnsFreeAddrInfoExt(curSrvInfoPtr);
    }

    return prevSrvInfoPtr;
}
#endif /* TM_DNS_USE_SRV */

/*
 * INTERNAL FUNCTIONS
 */

/*
 * tfCopyAddrInfo
 */
static int tfCopyAddrInfo(struct addrinfo TM_FAR * fromAddrInfoPtr,
                          struct addrinfo TM_FAR * toAddrInfoPtr)
{
    ttSockAddrPtrUnion  fromSockAddr;
    ttSockAddrPtrUnion  toSockAddr;
    int                 errorCode;

    if (   (fromAddrInfoPtr == TM_ADDRINFO_NULL_PTR)
        || (toAddrInfoPtr == TM_ADDRINFO_NULL_PTR) )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        errorCode = TM_ENOERROR;
        fromSockAddr.sockPtr = fromAddrInfoPtr->ai_addr;
        toSockAddr.sockPtr = toAddrInfoPtr->ai_addr;
#ifdef TM_USE_IPDUAL
        if (fromSockAddr.sockPtr->sa_family == AF_INET)
#endif /* TM_USE_IPDUAL */
#ifdef TM_USE_IPV4
        {
/* 2. Copy the data from the given addrinfo to the new one */
            toSockAddr.sockInPtr->sin_len = fromSockAddr.sockInPtr->sin_len;
            toSockAddr.sockInPtr->sin_family = fromSockAddr.sockInPtr->sin_family;
            tm_ip_copy(fromSockAddr.sockInPtr->sin_addr.s_addr,
                       toSockAddr.sockInPtr->sin_addr.s_addr);
        }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPDUAL
        else
#endif /* TM_USE_IPDUAL */
#ifdef TM_USE_IPV6
        {
/* 2. Copy the data from the given addrinfo to the new one */
            toSockAddr.sockIn6Ptr->sin6_len = fromSockAddr.sockIn6Ptr->sin6_len;
            toSockAddr.sockIn6Ptr->sin6_family =
                                             fromSockAddr.sockIn6Ptr->sin6_family;
            tm_6_ip_copy(&(fromSockAddr.sockIn6Ptr->sin6_addr),
                         &(toSockAddr.sockIn6Ptr->sin6_addr));
        }
#endif /* TM_USE_IPV6 */
    }

    return errorCode;
}

/*
 * tfDnsLabelAsciiCompare
 *
 * Compares two DNS 'label' string (similar to 'strcmp', but labelPtr is
 * with the compressed DNS label format and asciiPtr is a normal string.
 * Assumes all pointer parameters are valid.
 *
 * Parameter    Description
 * labelPtr    First string to compare
 * asciiPtr    Other string to compare
 * pktDataPtr   Pointer to beginning fo DNS packet (needed for decompression)
 *
 * Returns
 * Value           Meaning
 * 0               Strings match
 * < 0             labelPtr < asciiPtr
 * > 0             labelPtr > asciiPtr
*/
static ttSChar tfDnsLabelAsciiCompare(  tt8BitPtr labelPtr,
                                        tt8BitPtr asciiPtr,
                                        tt8BitPtr pktDataPtr )
{

    tt16Bit index1;
    tt16Bit index2;
    ttSChar compareResult;
    tt8Bit  charCount;
    tt8Bit  char1;
    tt8Bit  char2;

    index1    = 0;
    index2    = 0;
    charCount = 0;
    compareResult = 0;
    if ( asciiPtr == (tt8BitPtr)0)
    {
        compareResult = 1;
        goto dnsLabelCompare;
    }
    if ( labelPtr == (tt8BitPtr)0)
    {
        compareResult = -1;
        goto dnsLabelCompare;
    }
    while ( (labelPtr[index1] != 0) && (asciiPtr[index2] != 0))
    {

        if (charCount > 0)
        {
/* Comparisons must be done case-insensitive */
            char1 = (tt8Bit)tm_toupper(labelPtr[index1]);
            char2 = (tt8Bit)tm_toupper(asciiPtr[index2]);
/* If the current characters don't match, return; otherwise move to next char */
            if ( char1 < char2 )
            {
                compareResult = -1;
                goto dnsLabelCompare;
            }
            else
            {
                if ( char1 > char2 )
                {
                    compareResult = 1;
                    goto dnsLabelCompare;
                }
                else
                {
                    index1++;
                    index2++;
                    charCount--;
                }
            }
        }
        else
        {
/* Back reference in the DNS label string; adjust current pointers */
            if (labelPtr[index1] & TM_DNS_POINTER_MASK)
            {
                labelPtr =   pktDataPtr
                           + ((labelPtr[index1] & ~TM_DNS_POINTER_MASK) << 8)
                           + labelPtr[index1+1];
                index1 = 0;
            }
/* Could be a back reference to another back reference. Check for it */
            if (!(labelPtr[index1] & TM_DNS_POINTER_MASK))
            {
                charCount = labelPtr[index1++];
            }

            if (asciiPtr[index2] == '.')
            {
                index2++;
            }
        }
    }

dnsLabelCompare:
    return compareResult;
}

/*
 * tfDnsLabelLength
 *
 * Returns the length of the (unexpanded) DNS label string.  Assumes that all
 * pointer paramters are valid.
 *
 * Parameter    Description
 * labelPtr     Pointer to label string to compute length for.
 *
 * Returns
 * Value           Meaning
 * length
 *
*/
static tt16Bit tfDnsLabelLength(tt8BitPtr labelPtr)
{
    tt16Bit length;


    length = 0;
    while (   (labelPtr[length] != 0)
           && (!(labelPtr[length] & TM_DNS_POINTER_MASK)))
    {
        length = (tt16Bit)((tt16Bit)((tt16Bit)labelPtr[length] + 1) + length);
    }
    if (labelPtr[length] & TM_DNS_POINTER_MASK)
    {
        length++;
    }

    length++;

    return length;

}


/*
 * tfDnsExpLabelLength
 *
 * Returns the length of given DNS label if it were expanded to ASCII
 * representation.  Assumes that all pointer parameters are valid. This
 * includes space for a terminating null character
 *
 * Parameter    Description
 * labelPtr     Pointer to DNS label to compute length of
 * pktDataPtr   Pointer to beginning of DNS packet (needed for decompression)
 *
 * Returns
 * Value           Meaning
 * length
 *
*/
static tt16Bit tfDnsExpLabelLength( tt8BitPtr labelPtr,
                                    tt8BitPtr pktDataPtr )
{

    tt8BitPtr oldLabelPtr;
    tt16Bit   offset;
    tt16Bit   asciiLen;

    asciiLen = 0;
    offset = 0;

    do
    {
/* Check if this is a pointer */
        if (labelPtr[offset] & TM_DNS_POINTER_MASK)
        {
            oldLabelPtr = labelPtr;
            labelPtr =   pktDataPtr
                       + ((labelPtr[offset] & ~TM_DNS_POINTER_MASK) << 8)
                       + labelPtr[offset+1];
/* Make sure we only point to a previous part of the packet */
            if (labelPtr >= oldLabelPtr)
            {
                asciiLen = 0;
                break;
            }
            offset = 0;
        }
        else
        {
/* Record the length for this part of the label */
            asciiLen = (tt16Bit)(asciiLen + (tt16Bit)labelPtr[offset] + 1);
            offset = (tt16Bit)(offset + (tt16Bit)labelPtr[offset] + 1);
        }
    } while (labelPtr[offset]);

    return asciiLen;
}





/*
 * tfDnsCallback
 *
 * This function is called when there is a datagram to be received on our
 * socket; that is, we received a response from the DNS server.  We should
 * parse the response and place the information into our cache.  If we are in
 * blocking mode, this function should also post on the semaphore associated
 * with this cache entry, waking up the task waiting for the response.
 *
 *
 * 1.  Read DNS response from socket.
 * 2.  Get ID of this packet.
 * 3.  Find the cache entry associated with this response by calling
 *     tfDnsCacheLookup (search only active entries by ID).
 * 4.  If no entries found, silently discard this packet.
 * 5.  Call tfDnsParseResponse to retrieve the information contained in this
 *     packet and place it into the cache entry, setting error code if
 *     necessary.
 * 6.  Close the socket and mark this cache entry as complete.
 * 7.  Remove timers for this cache entry.
 * 8.  Refresh the failure count for the server.
 * 9.  If operating in blocking mode, post on semaphore for this cache entry.
 *
 * Parameter   Description
 * sock        Socket descriptor being signaled
 * event       Callback event
 *
 * Returns
 * Value   Meaning
 * None
 *
 */
static void tfDnsCallback(int sock, int event)
{
    int                errorCode;
    ttUserBuffer       bufHandle;
    char TM_FAR *      pktDataPtr;
    ttDnsHeaderPtr     dnsHdrPtr;
    tt16Bit            pktId;
    ttDnsCacheEntryPtr cacheEntryPtr;
#ifdef TM_USE_IPV6
    ttGenericUnion     timerParm1;
    ttPacketPtr        packetPtr;
#endif /* TM_USE_IPV6 */

/* Avoid compiler warnings */
    TM_UNREF_IN_ARG(event);


/* 1.  Read DNS response from socket. It is not necessary to get the address
 *     from tfZeroCopyRecvFrom - this information is never used as all DNS
 *     requests/responses are keyed on the ID.
 */
    errorCode = tfZeroCopyRecvFrom( sock,
                                    &bufHandle,
                                    &pktDataPtr,
                                    1460,
                                    0,
                                    (struct sockaddr TM_FAR *) 0,
                                    0 );

    if (errorCode >= (int)sizeof(ttDnsHeader))
    {
/*  Lock the DNS global variables */
        tm_call_lock_wait(&tm_context(tvDnsLockEntry));

/* 2.  Get ID of this packet. */
        dnsHdrPtr = (ttDnsHeaderPtr) pktDataPtr;
        tm_un_ntohs(dnsHdrPtr->dnsId, pktId);


/*
 * 3.  Find the cache entry associated with this response by calling
 *     tfDnsCacheLookup (search only active entries by ID).
 */

        cacheEntryPtr = tfDnsCacheLookup( TM_DNS_SEARCH_ID,
                                          (ttVoidPtr) &pktId,
                                          0,
                                          TM_DNS_LOOKUP_ACTIVE );


/* 4.  If no entries found, silently discard this packet. */
        if (cacheEntryPtr != (ttDnsCacheEntryPtr) 0)
        {
/*
 * Save the device index and site ID of the incoming interface; this is used
 * to return the scope to the user for link-local and site-local scope
 * addresses.
 */
#ifdef TM_USE_IPV6
            packetPtr = (ttPacketPtr) bufHandle;
            cacheEntryPtr->dnsc6DevIndex
                = packetPtr->pktDeviceEntryPtr->devIndex;
#ifndef TM_6_USE_RFC3879
/* site local address to be depricated, retrieval of Site ID can be removed */
            cacheEntryPtr->dnsc6SiteId
                = packetPtr->pktDeviceEntryPtr->dev6SiteId;
#endif /* TM_6_USE_RFC3879 */
#endif /* TM_USE_IPV6 */

/*
 * 5.  Call tfDnsParseResponse to retrieve the information contained in this
 *     packet and place it into the cache entry, setting error code if
 *     necessary.
 */
            errorCode = tfDnsParseResponse( (tt8BitPtr) pktDataPtr,
                                                cacheEntryPtr );

/* We're now done parsing the incoming DNS packet, so free it */
            (void)tfFreeZeroCopyBuffer(bufHandle);

            cacheEntryPtr->dnscErrorCode = errorCode;

/*
 * After parsing the response, if this response indicates that an IP6.ARPA
 * query failed, treat it like no response was received at all, since we need
 * to send another series of queries, this time in the IP6.INT domain:
 */
#ifdef TM_USE_IPV6
            if (    (errorCode != TM_ENOERROR)
                 && (cacheEntryPtr->dnscFlags & TM_6_DNSF_IP6_ARPA))
            {

/* If the original retry count is zero, our retries have already run out, so we
   need to start a new timer. */
                if (cacheEntryPtr->dnscTimerPtr == TM_TMR_NULL_PTR)
                {
                    timerParm1.genVoidParmPtr = (ttVoidPtr) cacheEntryPtr;
                    cacheEntryPtr->dnscTimerPtr =
                        tfTimerAdd( tfDnsTimeout,
                                    timerParm1,
                                    timerParm1, /* unused */
                                    (tt32Bit)tm_context(tvDnsTimeout) *
                                        TM_UL(1000),
                                    TM_TIM_AUTO );
                }

/* Reset the retry counter on this entry. */
                cacheEntryPtr->dnscRetriesLeft = tm_context(tvDnsRetries) + 1;

/* Set the flags on the cache entry to indicate that this is an IP6.INT
   query. */
                tm_16bit_clr_bit(cacheEntryPtr->dnscFlags, TM_6_DNSF_IP6_ARPA);
                cacheEntryPtr->dnscFlags |= TM_6_DNSF_IP6_INT;
            }
            else
#endif /* TM_USE_IPV6 */
            {
/* 6.  Close the socket and mark this cache entry as complete. */
                tfDnsCloseSocketComplete(cacheEntryPtr);

/* 7.  Remove timers for this cache entry. */
                if (cacheEntryPtr->dnscTimerPtr != (ttTimerPtr)0)
                {
                    tm_timer_remove( cacheEntryPtr->dnscTimerPtr );
                    cacheEntryPtr->dnscTimerPtr = (ttTimerPtr) 0;
                }

/* 8. Refresh the failure count for the server */
                tm_context(tvDnsServerList[cacheEntryPtr->dnscServerNum]).
                    dseFailures = 0;
#ifdef TM_PEND_POST_NEEDED
/*
 * 9. If using an RTOS:
 *    post on semaphore for this cache entry.
 */
/* increase entry count to avoid being removed by other threads */
                cacheEntryPtr->dnscCount++;

                tm_post(&cacheEntryPtr->dnscPendEntry,
                        &tm_context(tvDnsLockEntry), TM_ENOERROR);
/* decrease entry count */
                cacheEntryPtr->dnscCount--;
#endif /* TM_PEND_POST_NEEDED */
            }
        }
        else
        {
            (void)tfFreeZeroCopyBuffer(bufHandle);
        }
/* Unlock the DNS global lock */
        tm_call_unlock(&tm_context(tvDnsLockEntry));
    }
    else if (errorCode >= 0)
    {
        (void)tfFreeZeroCopyBuffer(bufHandle);
    }
}

/*
 * tfDnsParseResponse
 *
 * Parses a response form the DNS server.  This routine extracts the error code
 * and the information (IP Address, for example) from the packet.
 *
 *
 * 1.  ERROR CHECKING: Verify that this packet is a 'response' packet.
 * 2.  Get error code from RCODE field in header.  If error code is set
 *     (i.e., not zero) return error code to caller.
 * 3.  If ANCOUNT field in header is not greater than zero, this signifies an
 *     error condition (no response from server) so return error code
 *     (TM_DNS_EANSWER) to caller.
 * 4.  Since we may be storing more than one record per cache entry, and
 *     records have individual TTL values, the TTL value for the cache entry
 *     should be the shortest of all records stored in this cache entry.  We
 *     can determine this while searching through the answer section.  Since
 *     the TTL is initialized to the maximum value, which ensures that the
 *     TTL will never be larger than the configured maximum value, we only
 *     need to change it if the query string is a wildcard. If this is the
 *     case then we cannot cache it (RFC 1034 Section 4.3.3). To prevent
 *     caching of the answer and to make sure we ignore the TTL in the
 *     record, set the max TTL to 0.
 * 5.  Compute start of answer section (start of packet + header + all
 *     question entries).
 * 6.  For each RR in the answer section (ANCOUNT, or until we find an
 *     appropriate answer),
 *  6.1.    If CLASS of entry is IN...
 *      6.1.1. If this is a CNAME entry, save a pointer to the record; we'll
 *             actually copy it after processing the whole answer section, to
 *             make sure than we find an A/AAAA record.
 *      6.1.2. If this is an IPv6 hostname entry and query (AAAA record):
 *          6.1.2.1. Allocate a new addrinfo structure and attach it to the
 *                   cache entry.
 *          6.1.2.2. Store information from this record in the new addrinfo
 *                   structure.
 *      6.1.3. If this is an IPv4 hostname entry and query (A record):
 *          6.1.3.1. Allocate a new addrinfo structure and attach it to the
 *                   cache entry.
 *          6.1.3.2. Store information from this record in the new addrinfo
 *                   structure.
 *      6.1.4. If this is a mail host entry and query (MX):
 *          6.1.4.1. Allocate a new addrinfo structure and attach it to the
 *                   cache entry.
 *          6.1.4.2. Store information from this record in the new addrinfo
 *                   structure.
 *          6.1.4.3. For MX queries, addrinfo structures are sorted based on
 *                   preference, and then by hostname.  Find the appropriate
 *                   place for the new structure and insert it. If no MX
 *                   records have yet been saved, add this one as the first
 *                   entry.
 *              6.1.4.3.1. Loop through MX record entries:
 *              6.1.4.3.2. If the preference of the current entry is greater
 *                         than the new preference, the new entry should be
 *                         inserted immediately before the current entry.
 *              6.1.4.3.3. If the new and current preferences match, compare
 *                         the hostnames.  If the current hostname is "greater",
 *                         the new entry goes before the current one.
 *              6.1.4.3.4. Otherwise, save a pointer to the current entry and
 *                         move on to the next entry.
 *          6.1.4.4. If there was no previous entry, no MX records have been
 *                   save yet, so save this as the first one on the cache entry.
 *          6.1.4.5. Otherwise, if the new entry has not yet inserted, then
                     add the new entry after the previous one.
 *      6.1.5. If this is a reverse entry and query:
 *          6.1.5.1. Store information from this record in the cache entry.
 *          6.1.5.2. Stop processing since only one hostname (PTR record) is
 *                   needed.
 *      6.1.6. If the TTL for this record is shorter than the current value,
 *             save this entry's TTL as the current TTL to be used for the
 *             cache entry.  Only save the TTL if the current record is one
 *             that we're interested in for this query (A,CNAME,etc).
 *             TTL's received with the MSB set should be treated as 0
 *             (RFC 2181 Section 8).
 * 7. If this is a successful A or AAAA record query, and a CNAME record was
 *    found, allocate a new string buffer and copy the canonical name.
 * 8. Update the TTL and timestamp for the entry.  Use the shortest TTL of all
 *    records (addresses) saved on this cache entry, which was computed above.
 * 9. If the query type is MX we need to search through the additional records
 *    section to get an A record corresponding to the MX records retrieved
 *    above.  For each record in the additional section:
 *     9.1. If this is a hostname entry (A):
 *         9.1.1. Find matching addrinfo structure in the cache entry.
 *         9.1.2. If found, and no address set, copy address into structure.
 *         9.1.3. If found, and address already set, we need to create a copy
 *                of this addrinfo to keep additional information:
 *             9.1.3.1. Allocate new addrinfo structure and attach it after
 *                      the previously found entry.
 *             9.1.3.2. Store information from this record in the new addrinfo
 *                      structure.
 *     9.2. After searching through additional section, cleanup cache entry by
 *          removing any MX records that we couldn't find matching A/AAAA
 *          records for.  If no MX records remain after cleaning, return
 *          TM_DNS_EANSWER.
 * 10. If no records were found that matched our criteria, return
 *     TM_DNS_EANSWER.
 *
 * Parameter   Description
 * responsePtr Pointer to beginning of DNS response packet.
 * entryPtr    Pointer to the cache entry of the current request.
 *
 * Returns
 * Value   Meaning
 * TM_ENOERROR Information retrieved successfully
 * TM_EINVAL   Bad response pointer.
 * TM_EINVAL   Packet is not a 'response' packet
 *
 */
#ifdef TM_LINT
LINT_UNINIT_SYM_BEGIN(flags)
LINT_UNINIT_SYM_BEGIN(rrCount)
#endif /* TM_LINT */
static int tfDnsParseResponse(tt8BitPtr          responsePtr,
                              ttDnsCacheEntryPtr entryPtr)
{
    ttSockAddrPtrUnion       sockAddr;
    struct addrinfo TM_FAR * newAddrInfoPtr;
    struct addrinfo TM_FAR * addrInfoPtr;
    struct addrinfo TM_FAR * prevAddrInfoPtr;
    struct addrinfo TM_FAR * nextAddrInfoPtr;
#ifdef TM_DNS_USE_SRV
    struct addrinfo TM_FAR * tempAddrInfoPtr;
    ttAddrInfoExtPtr         srvAddrInfoPtr;
    ttAddrInfoExtPtr         prevSrvAddrInfoPtr;
    ttAddrInfoExtPtr         newSrvAddrInfoPtr;
    ttAddrInfoExtPtr         tempSrvInfoPtr;
#ifdef TM_USE_IPDUAL
    tt8Bit                   isV4;
#endif /* TM_USE_IPDUAL */
#endif /* TM_DNS_USE_SRV */
    ttDnsHeaderPtr           dnsHdrPtr;
    ttDnsRRHeaderPtr         rrHdrPtr;
    tt8BitPtr                questionPtr;
    tt8BitPtr                rDataPtr;
    tt8BitPtr                canonStringPtr;
    tt8BitPtr                answerPtr;
    tt8BitPtr                canonRecPtr;
    tt8BitPtr                additionalPtr;
    ttCharPtr                strChrResult;
#ifdef TM_USE_IPV4
    tt4IpAddress             rdIp4Address;
#endif /* TM_USE_IPV4 */
    struct sockaddr_storage  tempSockAddrStorage;
    int                      hostnameMatch;
    int                      errorCode;
#if (defined(TM_USE_IPV6) && defined(TM_DSP))
    int                      i;
#endif /* TM_USE_IPV6 && TM_DSP */
    tt32Bit                  ttl;
    tt32Bit                  currentMinTtl;
    tt16Bit                  aCount;
    tt16Bit                  qCount;
    tt16Bit                  qLength;
    tt16Bit                  length;
    tt16Bit                  addRecType;
    tt16Bit                  flags;
    tt16Bit                  rrCount;
    tt16Bit                  rdLength;
    tt16Bit                  rType;
    tt16Bit                  rClass;
    tt16Bit                  queryType;
    tt16Bit                  canonStringLen;
    tt8Bit                   entryFound;
    tt8Bit                   cnameFound;
    tt8Bit                   saveTtlFlag;
    tt8Bit                   matchedAdditionalRecs;


/* Initialize here, just to avoid compiler warnings. */
#ifdef TM_USE_IPV4
    additionalPtr  = (tt8BitPtr) 0;
    hostnameMatch   = 0;
#endif /* TM_USE_IPV4 */
    rrHdrPtr = (ttDnsRRHeaderPtr)0;
    dnsHdrPtr = (ttDnsHeaderPtr) responsePtr;
    errorCode = TM_ENOERROR;
    canonRecPtr = TM_8BIT_NULL_PTR;
    entryFound = TM_8BIT_ZERO;
    rDataPtr = (tt8BitPtr)0;
    newAddrInfoPtr = TM_ADDRINFO_NULL_PTR;
    currentMinTtl = tm_context(tvDnsCacheTtl);
#ifdef TM_USE_IPV4
    rdIp4Address = (tt4IpAddress)0;
#endif /* TM_USE_IPV4 */
    tm_bzero(&sockAddr, sizeof(ttSockAddrPtrUnion));

    tm_un_ntohs(dnsHdrPtr->dnsFlags, flags);

/* 1.  ERROR CHECKING: Verify that this packet is a 'response' packet. */

    if ( !(flags & TM_DNS_HDR_RESP))
    {
        errorCode = TM_EINVAL;
    }

/*
 * 2.  Get error code from RCODE field in header.  If error code is set
 *     (i.e., not zero) return error code to caller.
 */
    if ( ((flags & TM_DNS_RCODE_MASK) != 0) && (errorCode == TM_ENOERROR) )
    {
        errorCode = (flags & TM_DNS_RCODE_MASK) + TM_DNS_ERROR_BASE;
    }


/*
 * 3.  If ANCOUNT field in header is not greater than zero, this signifies an
 *     error condition (no response from server) so return error code
 *     (TM_DNS_EANSWER) to caller.
 *     If the response indicates no error and also has no answers, the response
 *     might be a referral, but RFC 1034 section 5.3.1 allows a resolver to not
 *     handle iterative queries/responses.
 */
    tm_un_ntohs( dnsHdrPtr->dnsAnswerCount, aCount );
    if ( (aCount == 0) && (errorCode == TM_ENOERROR) )
    {
        errorCode = TM_DNS_EANSWER;
    }

    if (errorCode != TM_ENOERROR)
    {
        goto dnsParseResponseExit;
    }

/* Check the "Truncated" flag, if it is set do not cache the answer */
    if (tm_16bit_one_bit_set(flags, TM_DNS_TRUNC_MASK))
    {
        currentMinTtl = 0;
    }
    else
    {
        currentMinTtl = tm_context(tvDnsCacheTtl);
    }

/*
 * 4.  Since we may be storing more than one record per cache entry, and
 *     records have individual TTL values, the TTL value for the cache entry
 *     should be the shortest of all records stored in this cache entry.  We
 *     can determine this while searching through the answer section.  Since
 *     the TTL is initialized to the maximum value, which ensures that the
 *     TTL will never be larger than the configured maximum value, we only
 *     need to change it if the query string is a wildcard. If this is the
 *     case then we cannot cache it (RFC 1034 Section 4.3.3). To prevent
 *     caching of the answer and to make sure we ignore the TTL in the
 *     record, set the max TTL to 0.
 */

    saveTtlFlag = TM_8BIT_ZERO;

    queryType = entryPtr->dnscQueryType;
    questionPtr = responsePtr + TM_DNS_HEADER_SIZE;

    strChrResult = tm_strchr(questionPtr, (int)'*');
    if (strChrResult != (char *)0)
    {
        currentMinTtl = 0;
    }

/*
 * 5.  Compute start of answer section (start of packet + header + all
 *     question entries).
 */
    qLength = 0;
    tm_un_ntohs( dnsHdrPtr->dnsQuestionCount, rrCount );
    for (qCount=0; qCount < rrCount; qCount++)
    {
        length = tfDnsLabelLength(questionPtr);

/* Move past the QTYPE and QCLASS fields */
        qLength = (tt16Bit) (qLength + length + 4);

        questionPtr += (length + 4);
    }

    answerPtr = questionPtr;

    cnameFound = TM_8BIT_ZERO;

/*
 * 6.  For each RR in the answer section (ANCOUNT, or until we find an
 *     appropriate answer),
 */
    tm_un_ntohs( dnsHdrPtr->dnsAnswerCount, rrCount);
    for (aCount = 0; aCount < rrCount; aCount++ )
    {

/*
 * Now that we know the name length and have saved a pointer to the start of
 * the RR header, move on to the next RR.
 */
        length = tfDnsLabelLength(answerPtr);
        rrHdrPtr = (ttDnsRRHeaderPtr)(ttVoidPtr)(answerPtr + length);

/*
 * Get type, class, and ttl from resource record header
 * (unaligned so we use tm_un_ntohs)
 */
        tm_un_ntohs(rrHdrPtr->dnsrType, rType);
        tm_un_ntohs(rrHdrPtr->dnsrClass, rClass);
        tm_un_ntohl(rrHdrPtr->dnsrTtl, ttl);
        if (ttl & TM_UL(0x80000000))
        {
            ttl = 0;
        }
/* Get length of what comes after the header */
        tm_un_ntohs(rrHdrPtr->dnsrRdLength, rdLength);

        answerPtr += length + TM_DNS_RR_HDR_LEN + rdLength;

/*  6.1.    If CLASS of entry is IN... */
        if ( rClass == TM_DNS_CLASS_IN )
        {
/*
 *      6.1.1. If this is a CNAME entry, save a pointer to the record; we'll
 *             actually copy it after processing the whole answer section, to
 *             make sure than we find an A/AAAA record.
 */
            if (rType == TM_DNS_QTYPE_CNAME)
            {
                rDataPtr = ((tt8BitPtr)(ttVoidPtr)rrHdrPtr) + TM_DNS_RR_HDR_LEN;
                canonRecPtr = rDataPtr;
                saveTtlFlag = TM_8BIT_YES;
                cnameFound = TM_8BIT_YES;
            }
            else

#ifdef TM_USE_IPV6
/*      6.1.2. If this is an IPv6 hostname entry and query (AAAA record): */
            if (    (queryType == TM_DNS_QTYPE_AAAA)
                 && (rType == TM_DNS_QTYPE_AAAA) )
            {

/*
 *          6.1.2.1. Allocate a new addrinfo structure and attach it to the
 *                   cache entry.
 */
                newAddrInfoPtr = tfDnsAllocAddrInfo();
                if (newAddrInfoPtr != TM_ADDRINFO_NULL_PTR)
                {
                    if (entryPtr->dnscAddrInfoPtr != TM_ADDRINFO_NULL_PTR)
                    {
                        newAddrInfoPtr->ai_next = entryPtr->dnscAddrInfoPtr;
                    }
                    entryPtr->dnscAddrInfoPtr = newAddrInfoPtr;

/*
 *          6.1.2.2. Store information from this record in the new addrinfo
 *                   structure.
 */
                    rDataPtr = ((tt8BitPtr)(ttVoidPtr)rrHdrPtr) + TM_DNS_RR_HDR_LEN;
                    sockAddr.sockPtr = newAddrInfoPtr->ai_addr;

#ifdef TM_DSP
/* Shrink from sixteen bytes(which are ints) to four 32-bit words */
                    for (i = 0; i < 4; i++)
                    {
                        tm_un_ntohl(rDataPtr + (i * 4),
                                    sockAddr.sockIn6Ptr->sin6_addr.ip6Addr.
                                     ip6U32[i]);
                    }
#else /* !TM_DSP */
/* use tm_bcopy because we want to keep network byte order */
                    tm_bcopy(rDataPtr,
                             sockAddr.sockIn6Ptr->sin6_addr.ip6Addr.ip6U8,
                             sizeof(sockAddr.sockIn6Ptr->sin6_addr));
#endif /* !TM_DSP */

                    sockAddr.sockIn6Ptr->sin6_len =
                        sizeof(struct sockaddr_in6);
                    sockAddr.sockIn6Ptr->sin6_family = AF_INET6;

                    entryFound = TM_8BIT_YES;
                    saveTtlFlag = TM_8BIT_YES;
                }
                else
                {
                    errorCode = TM_ENOBUFS;
                    goto dnsParseResponseExit;
                }

            }
            else
#endif /* TM_USE_IPV6 */


/*      6.1.3.  If this is an IPv4 hostname entry and query (A record): */
#ifdef TM_USE_IPV4
            if (    (queryType == TM_DNS_QTYPE_A)
                 && (rType == TM_DNS_QTYPE_A) )
            {
/*
 *          6.1.3.1. Allocate a new addrinfo structure and attach it to the
 *                   cache entry.
 */
                newAddrInfoPtr = tfDnsAllocAddrInfo();
                if (newAddrInfoPtr != TM_ADDRINFO_NULL_PTR)
                {
                    if (entryPtr->dnscAddrInfoPtr != TM_ADDRINFO_NULL_PTR)
                    {
                        newAddrInfoPtr->ai_next = entryPtr->dnscAddrInfoPtr;
                    }
                    entryPtr->dnscAddrInfoPtr = newAddrInfoPtr;

/*
 *          6.1.3.2. Store information from this record in the new addrinfo
 *                   structure.
 */
                    rDataPtr = ((tt8BitPtr)(ttVoidPtr)rrHdrPtr) + TM_DNS_RR_HDR_LEN;
#ifdef TM_DSP
/* Shrink from 4 bytes(which are ints) to one 32-bit */
                    tm_un_ntohl(rDataPtr, rdIp4Address);
#else /* !TM_DSP */
/* use copy because we want to keep network byte order */
                    tm_4bytes_copy(rDataPtr, &rdIp4Address);
#endif /* !TM_DSP */
                    sockAddr.sockPtr = newAddrInfoPtr->ai_addr;
                    tm_ip_copy(rdIp4Address,
                               sockAddr.sockInPtr->sin_addr.s_addr);

                    sockAddr.sockInPtr->sin_len =
                        sizeof(struct sockaddr_in);
                    sockAddr.sockInPtr->sin_family = AF_INET;

                    entryFound = TM_8BIT_YES;
                    saveTtlFlag = TM_8BIT_YES;
                }
                else
                {
/* Couldn't alloc memory, so exit.  The cache entry will be cleaned up when
   the error is returned to the user. */
                    errorCode = TM_ENOBUFS;
                    goto dnsParseResponseExit;
                }
            }
            else
#endif /* TM_USE_IPV4 */

/*      6.1.4. If this is a mail host entry and query (MX): */
            if (    (queryType == TM_DNS_QTYPE_MX)
                 && (rType == TM_DNS_QTYPE_MX) )
            {

/*          6.1.4.1. Allocate a new addrinfo structure. */
                newAddrInfoPtr = tfDnsAllocAddrInfo();
                if (newAddrInfoPtr == TM_ADDRINFO_NULL_PTR)
                {
/* Couldn't alloc memory, so exit.  The cache entry will be cleaned up when
   the error is returned to the user. */
                    errorCode = TM_ENOBUFS;
                    goto dnsParseResponseExit;
                }
/*
 *          6.1.4.2. Store information from this record in the new addrinfo
 *                   structure.
 */
                rDataPtr = ((tt8BitPtr)(ttVoidPtr)rrHdrPtr) + TM_DNS_RR_HDR_LEN;
                tm_un_ntohs(rDataPtr, newAddrInfoPtr->ai_mxpref);
                rDataPtr += 2;

                length = tfDnsExpLabelLength(rDataPtr, (tt8BitPtr) dnsHdrPtr);

                newAddrInfoPtr->ai_mxhostname = (char *)0;
/* If length is 0, the packet is bad */
                if (length > 0)
                {
                    newAddrInfoPtr->ai_mxhostname =
                        (char TM_FAR *) tm_get_raw_buffer(length);
                }
                if (newAddrInfoPtr->ai_mxhostname == 0)
                {
/* Couldn't alloc memory, so exit.  The cache entry will be cleaned up when
   the error is returned to the user. */
                    errorCode = TM_ENOBUFS;
                    goto dnsParseResponseExit;

                }

                tfDnsLabelToAscii(rDataPtr,
                                  (tt8BitPtr) newAddrInfoPtr->ai_mxhostname,
                                  (tt8BitPtr) dnsHdrPtr,
                                  TM_DNS_LABEL_FULL,
                                  0);

/*
 *          6.1.4.3. For MX queries, addrinfo structures are sorted based on
 *                   preference, and then by hostname.  Find the appropriate
 *                   place for the new structure and insert it. If no MX
 *                   records have yet been saved, add this one as the first
 *                   entry.
 *
 *              6.1.4.3.1. Loop through MX record entries:
 */
                addrInfoPtr = entryPtr->dnscAddrInfoPtr;
                prevAddrInfoPtr = TM_ADDRINFO_NULL_PTR;
                while (addrInfoPtr != TM_ADDRINFO_NULL_PTR)
                {

/*
 *              6.1.4.3.2. If the preference of the current entry is greater
 *                         than the new preference, the new entry should be
 *                         inserted immediately before the current entry.
 */
                    if (addrInfoPtr->ai_mxpref > newAddrInfoPtr->ai_mxpref)
                    {
                        break;
                    }
                    else if (addrInfoPtr->ai_mxpref == newAddrInfoPtr->ai_mxpref)

                    {

/*
 *              6.1.4.3.3. If the new and current preferences match, compare
 *                         the hostnames.  If the current hostname is "greater",
 *                         the new entry goes before the current one.
 */
                        hostnameMatch =
                            tm_stricmp(addrInfoPtr->ai_mxhostname,
                                       newAddrInfoPtr->ai_mxhostname);
                        if (hostnameMatch >= 0)
                        {
                            break;
                        }
                    }
/*
 *              6.1.4.3.4. Otherwise, save a pointer to the current entry and
 *                         move on to the next entry.
 */
                    prevAddrInfoPtr = addrInfoPtr;
                    addrInfoPtr     = addrInfoPtr->ai_next;
                }

/*
 *          6.1.4.4. If there was no previous entry, no MX records have been
 *                   save yet, so save this as the first one on the cache entry.
 */
                if (prevAddrInfoPtr == TM_ADDRINFO_NULL_PTR)
                {
                    entryPtr->dnscAddrInfoPtr = newAddrInfoPtr;
                }
                else
                {
/*          6.1.4.5. Otherwise, add the new entry after the previous one. */
                     prevAddrInfoPtr->ai_next = newAddrInfoPtr;
                }
                newAddrInfoPtr->ai_next = addrInfoPtr;

                entryFound = TM_8BIT_YES;
                saveTtlFlag = TM_8BIT_YES;
            }
            else

#ifdef TM_DNS_USE_SRV
/* If this is an SRV cache entry and query */
            if (    (queryType == TM_DNS_QTYPE_SRV)
                 && (rType == TM_DNS_QTYPE_SRV) )
            {

/* Allocate a new extended addrinfo structure. */
                newSrvAddrInfoPtr = tfDnsAllocAddrInfoExt();
                if (newSrvAddrInfoPtr == TM_ADDRINFO_EXT_NULL_PTR)
                {
/*
 * Couldn't alloc memory, so exit.  The cache entry will be cleaned up when
 * the error is returned to the user.
 */
                    errorCode = TM_ENOBUFS;
                    goto dnsParseResponseExit;
                }

/* Store information from this record in the new addrinfo structure. */
                rDataPtr = ((tt8BitPtr)(ttVoidPtr)rrHdrPtr) + TM_DNS_RR_HDR_LEN;
/* Save the priority */
                tm_un_ntohs(rDataPtr, newSrvAddrInfoPtr->aie_srvpri);

                rDataPtr += 2;
/* Save the weight */
                tm_un_ntohs(rDataPtr, newSrvAddrInfoPtr->aie_srvweight);

                rDataPtr += 2;
/* Save the port */
#ifdef TM_DSP
                tm_un_ntohs(rDataPtr, newSrvAddrInfoPtr->aie_port);
#else /* !TM_DSP */
/* Keep network byte order */
                tm_2bytes_copy(rDataPtr, &newSrvAddrInfoPtr->aie_port);
#endif /* !TM_DSP */

                rDataPtr += 2;
/* Get the expanded label length */
                length = tfDnsExpLabelLength(rDataPtr, (tt8BitPtr) dnsHdrPtr);

/* Allocate memory to save the hostname */
                newSrvAddrInfoPtr->aie_srvhostname = (char *)0;
                if (length > 0)
                {
                    newSrvAddrInfoPtr->aie_srvhostname =
                        (char *) tm_get_raw_buffer(length);
                }
                if (newSrvAddrInfoPtr->aie_srvhostname == 0)
                {
/*
 * Couldn't alloc memory, so exit.  The cache entry will be cleaned up when
 * the error is returned to the user.
 */
                    tfDnsFreeAddrInfoExt(newSrvAddrInfoPtr);
                    errorCode = TM_ENOBUFS;
                    goto dnsParseResponseExit;

                }

/* Translate the DNS label into ASCII hostname format */
                tfDnsLabelToAscii(
                        rDataPtr,
                        (tt8BitPtr) newSrvAddrInfoPtr->aie_srvhostname,
                        (tt8BitPtr) dnsHdrPtr,
                        TM_DNS_LABEL_FULL,
                        0);

/* Make sure this SRV doesn't already exist in the list */
                srvAddrInfoPtr = (ttAddrInfoExtPtr)entryPtr->dnscAddrInfoPtr;
                prevSrvAddrInfoPtr = TM_ADDRINFO_EXT_NULL_PTR;
                while (srvAddrInfoPtr != TM_ADDRINFO_EXT_NULL_PTR)
                {
                    if (   (   newSrvAddrInfoPtr->aie_port
                            == srvAddrInfoPtr->aie_port)
                        && (   tm_stricmp(newSrvAddrInfoPtr->aie_srvhostname,
                                          srvAddrInfoPtr->aie_srvhostname)
                            == 0))
                    {
/* This SRV already exists in the list, so discard the new one */
                        tfDnsFreeAddrInfoExt(newSrvAddrInfoPtr);
                        newSrvAddrInfoPtr = (ttAddrInfoExtPtr)0;
                        break;
                    }
                    srvAddrInfoPtr =
                            (ttAddrInfoExtPtr)srvAddrInfoPtr->aie_next;
                }

/* We may have just discarded the new SRV. If so, skip to the next */
                if (newSrvAddrInfoPtr == (ttAddrInfoExtPtr)0)
                {
                    continue;
                }
/*
 * For SRV queries, ttAddrInfoExt structures are sorted based on priority,
 * then randomly by weight.
 */
                srvAddrInfoPtr = (ttAddrInfoExtPtr)entryPtr->dnscAddrInfoPtr;
                prevSrvAddrInfoPtr = TM_ADDRINFO_EXT_NULL_PTR;
                while (srvAddrInfoPtr != TM_ADDRINFO_EXT_NULL_PTR)
                {

/*
 * Lowest priority goes first. Weight can be randomly ordered for now,
 * except 0 weight entries must come first in their priority
 */
                    if (   (newSrvAddrInfoPtr->aie_srvpri <
                            srvAddrInfoPtr->aie_srvpri)
                        || (   (newSrvAddrInfoPtr->aie_srvpri ==
                                srvAddrInfoPtr->aie_srvpri)
                            && (srvAddrInfoPtr->aie_srvweight != 0)))
                    {
                        break;
                    }
/*
 * Otherwise, save a pointer to the current entry and move on to the next
 * entry.
 */
                    prevSrvAddrInfoPtr = srvAddrInfoPtr;
                    srvAddrInfoPtr     =
                            (ttAddrInfoExtPtr)srvAddrInfoPtr->aie_next;
                }

/*
 * If there was no previous entry, no SRV records have been saved yet, so save
 * this as the first one on the cache entry.
 */
                if (prevSrvAddrInfoPtr == TM_ADDRINFO_EXT_NULL_PTR)
                {
                    entryPtr->dnscAddrInfoPtr =
                            (struct addrinfo TM_FAR *)newSrvAddrInfoPtr;
                }
                else
                {
/* Otherwise, add the new entry after the previous one. */
                     prevSrvAddrInfoPtr->aie_next =
                            (struct addrinfo TM_FAR *)newSrvAddrInfoPtr;
                }
                newSrvAddrInfoPtr->aie_next =
                        (struct addrinfo TM_FAR *)srvAddrInfoPtr;

/* If we've exceeded the max number of hosts, delete the weakest */
                if (  ++entryPtr->dnscSrvHostCount
                    > TM_DNS_SRV_HOSTS_PER_ENTRY)
                {
                    tfDnsSrvDeleteWeakest(entryPtr);
                }

                entryFound = TM_8BIT_YES;
                saveTtlFlag = TM_8BIT_YES;
            }
            else
#endif /* TM_DNS_USE_SRV */
/*      6.1.5. If this is a reverse entry and query: */
            if (    (queryType == TM_DNS_QTYPE_REV)
                 && (rType == TM_DNS_QTYPE_REV) )
            {

/*          6.1.5.1. Store information from this record in the cache entry. */
                rDataPtr = ((tt8BitPtr)(ttVoidPtr)rrHdrPtr) + TM_DNS_RR_HDR_LEN;
                length = tfDnsExpLabelLength( rDataPtr,
                                              (tt8BitPtr) dnsHdrPtr );

                entryPtr->dnscRevHostnameStr = (ttCharPtr)0;
                if (length > 0)
                {
                    entryPtr->dnscRevHostnameStr =
                        (ttCharPtr) tm_get_raw_buffer((ttPktLen)length);
                }

                if (entryPtr->dnscRevHostnameStr == 0)
                {
/* Couldn't alloc memory, so exit.  The cache entry will be cleaned up when
   the error is returned to the user. */
                    errorCode = TM_ENOBUFS;
                    goto dnsParseResponseExit;
                }

                (void)tfDnsLabelToAscii(rDataPtr,
                                  (tt8BitPtr) entryPtr->dnscRevHostnameStr,
                                  (tt8BitPtr) dnsHdrPtr,
                                  TM_DNS_LABEL_FULL,
                                  0);

/*
 *          6.1.5.2. Stop processing since only one hostname (PTR record) is
 *                   needed.
 */
                if (ttl < currentMinTtl)
                {
                    currentMinTtl = ttl;
                }
                entryFound = TM_8BIT_YES;
                break;
            }

/*
 *      6.1.6. If the TTL for this record is shorter than the current value,
 *             save this entry's TTL as the current TTL to be used for the
 *             cache entry.  Only save the TTL if the current record is one
 *             that we're interested in for this query (A,CNAME,etc).
 *             TTL's received with the MSB set should be treated as 0
 *             (RFC 2181 Section 8).
 */
            if ((saveTtlFlag == TM_8BIT_YES) && (currentMinTtl != 0))
            {
                if (ttl < currentMinTtl)
                {
                    currentMinTtl = ttl;
                }
                saveTtlFlag = TM_8BIT_ZERO;
            }

        }
    }

/*
 * 7. If this is a successful A or AAAA record query, and a CNAME record was
 *    found, allocate a new string buffer and copy the canonical name.
 */
    if (    (entryFound == TM_8BIT_YES)
         && (cnameFound == TM_8BIT_YES))
    {
        newAddrInfoPtr = entryPtr->dnscAddrInfoPtr;
        if (newAddrInfoPtr != TM_ADDRINFO_NULL_PTR)
        {
            canonStringLen = tfDnsExpLabelLength( canonRecPtr,
                                                  (tt8BitPtr)dnsHdrPtr );
            canonStringPtr = (tt8BitPtr)0;
            if (canonStringLen > 0)
            {
                canonStringPtr = (tt8BitPtr) tm_get_raw_buffer(canonStringLen);
            }
            if (canonStringPtr != 0)
            {
                tfDnsLabelToAscii( canonRecPtr,
                                   canonStringPtr,
                                   (tt8BitPtr) dnsHdrPtr,
                                   TM_DNS_LABEL_FULL,
                                   0);
                newAddrInfoPtr->ai_canonname = (char TM_FAR *) canonStringPtr;
            }
        }
    }

/*
 * 8. Update the TTL and timestamp for the entry.  Use the shortest TTL of all
 *    records (addresses) saved on this cache entry, which was computed above.
 */
    if (entryFound == TM_8BIT_YES)
    {
/*
 * currentMinTtl is initialized to the TTL set by the user, so the TTL will
 * never exceed the maximum.
 */
        entryPtr->dnscTtl = currentMinTtl * 1000;

        tm_kernel_set_critical;
        entryPtr->dnscTimestamp = tvTime;
        tm_kernel_release_critical;
    }

/*
 * 9. If the query type is MX we need to search through the additional records
 *    section to get an A record corresponding to the MX records retrieved
 *    above.  For each record in the additional section:
 */
    if (    (entryFound == TM_8BIT_YES)
         && (queryType == TM_DNS_QTYPE_MX) )
    {

/*
 * Move past any authority records to get to the additional records.
 */
        additionalPtr = answerPtr;
        tm_un_ntohs(dnsHdrPtr->dnsNameServerCount, rrCount);
        for (aCount=0; aCount < rrCount; aCount++)
        {
            length = tfDnsLabelLength(additionalPtr);
            rrHdrPtr = (ttDnsRRHeaderPtr)(ttVoidPtr)(additionalPtr + length);
/* Get length from resource record header (unaligned) */
            tm_un_ntohs(rrHdrPtr->dnsrRdLength, rdLength);

            length = (tt16Bit) (length + rdLength);

            additionalPtr += length + TM_DNS_RR_HDR_LEN;

        }

        matchedAdditionalRecs = 0;
        tm_un_ntohs(dnsHdrPtr->dnsAdditionalCount, rrCount);
        for (aCount = 0; aCount < rrCount; aCount++ )
        {

/*     9.1. If this is a hostname entry (A) or (AAAA): */
            length = tfDnsLabelLength(additionalPtr);
            rrHdrPtr = (ttDnsRRHeaderPtr)(ttVoidPtr)(additionalPtr + length);
            tm_un_ntohs(rrHdrPtr->dnsrRdLength, rdLength);
/* Get the record type */
            tm_un_ntohs(rrHdrPtr->dnsrType, addRecType);

            if (
#ifdef TM_USE_IPV4
                (addRecType != TM_DNS_QTYPE_A)
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPDUAL
                &&
#endif /* TM_USE_IPDUAL */
#ifdef TM_USE_IPV6
                (addRecType != TM_DNS_QTYPE_AAAA)
#endif /* TM_USE_IPV6 */
                )
            {
/* Unsupported type, skip it. */
                length = (tt16Bit)(length + rdLength);
                additionalPtr += length + TM_DNS_RR_HDR_LEN;
                continue;
            }

/*         9.1.1. Find matching addrinfo structure in the cache entry. */
            addrInfoPtr = entryPtr->dnscAddrInfoPtr;

            while (addrInfoPtr != TM_ADDRINFO_NULL_PTR)
            {
                hostnameMatch =
                    tfDnsLabelAsciiCompare(
                                      additionalPtr,
                                      (tt8BitPtr) addrInfoPtr->ai_mxhostname,
                                      (tt8BitPtr) dnsHdrPtr );
                if (hostnameMatch == 0)
                {
                    matchedAdditionalRecs++;
/*
 *         9.1.2. If found, and no address set, copy address into structure.
 *         9.1.3. If found, and address already set, we need to create a copy
 *                of this addrinfo to keep additional information:
 */
                    if (addrInfoPtr->ai_addr->sa_family != 0)
                    {
/*
 *             9.1.3.1. Allocate new addrinfo structure and attach it after
 *                      the previously found entry.
 */
                        newAddrInfoPtr = tfDnsAllocAddrInfo();
                        if (newAddrInfoPtr == TM_ADDRINFO_NULL_PTR)
                        {
/* Couldn't alloc memory, so exit.  The cache entry will be cleaned up when
   the error is returned to the user. */
                            errorCode = TM_ENOBUFS;
                            goto dnsParseResponseExit;
                        }
                        newAddrInfoPtr->ai_next = addrInfoPtr->ai_next;
                        addrInfoPtr->ai_next    = newAddrInfoPtr;

                        newAddrInfoPtr->ai_mxpref = addrInfoPtr->ai_mxpref;
                    }

/*
 *             9.1.3.2. Store information from this record in the new addrinfo
 *                      structure.
 */
                    rDataPtr = ((tt8BitPtr)(ttVoidPtr)rrHdrPtr) +
                                                        TM_DNS_RR_HDR_LEN;
                    tm_bzero(&tempSockAddrStorage,
                             sizeof(struct sockaddr_storage));
#ifdef TM_USE_IPV4
                    if (addRecType == TM_DNS_QTYPE_A)
                    {
                        tempSockAddrStorage.ss_family = AF_INET;
#ifdef TM_DSP
/* Shrink from 4 bytes(which are ints) to one 32-bit */
                        tm_un_ntohl(
                            rDataPtr,
                            tempSockAddrStorage.addr.ipv4.sin_addr.s_addr);
#else /* !TM_DSP */
/* use copy because we want to keep network byte order */
                        tm_4bytes_copy(
                            rDataPtr,
                            &(tempSockAddrStorage.addr.ipv4.sin_addr.s_addr));
#endif /* !TM_DSP */
                    }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPDUAL
                    else
#endif /* TM_USE_IPDUAL */
#ifdef TM_USE_IPV6
                    {
                        tempSockAddrStorage.ss_family = AF_INET6;
#ifdef TM_DSP
/* Shrink from sixteen bytes(which are ints) to four 32-bit words */
                        for (i = 0; i < 4; i++)
                        {
                            tm_un_ntohl(rDataPtr + (i * 4),
                                        tempSockAddrStorage.addr.ipv6.
                                                sin6_addr.ip6Addr.ip6U32[i]);
                        }
#else /* !TM_DSP */
/* use tm_bcopy because we want to keep network byte order */
                        tm_bcopy(rDataPtr,
                                 tempSockAddrStorage.addr.ipv6.
                                                sin6_addr.ip6Addr.ip6U8,
                                 sizeof(struct in6_addr));
#endif /* !TM_DSP */
                    }
#endif /* TM_USE_IPV6 */

                    if (addrInfoPtr->ai_addr->sa_family != 0)
                    {
                        sockAddr.sockPtr = newAddrInfoPtr->ai_addr;
                    }
                    else
                    {
                        sockAddr.sockPtr = addrInfoPtr->ai_addr;
                    }
                    tempSockAddrStorage.ss_len =
                                            sizeof(struct sockaddr_storage);
                    tm_sockaddr_storage_copy(&tempSockAddrStorage,
                                             sockAddr.sockNgPtr);
                }

                addrInfoPtr = addrInfoPtr->ai_next;
            }
            length = (tt16Bit) (length + rdLength);

            additionalPtr += length + TM_DNS_RR_HDR_LEN;
        }

/*
 *     9.2. After searching through additional section, cleanup cache entry by
 *          removing any MX records that we couldn't find matching A/AAAA
 *          records for.  If no MX records remain after cleaning, return
 *          TM_DNS_EANSWER.
 *
 * ECR modified for bug ID 1674:
 *      if there are no additional resource records, then we should do a
 *      name lookup to get the IP address of this mail server. This results
 *      from the use of a CNAME alias in the MX record, which is considered
 *      an invalid DNS configuration however it is sometimes used.
 */

        addrInfoPtr = entryPtr->dnscAddrInfoPtr;
        prevAddrInfoPtr = TM_ADDRINFO_NULL_PTR;

        while (addrInfoPtr != TM_ADDRINFO_NULL_PTR)
        {
/* save the pointer to the next addrInfo, since we may free this addrInfo */
           nextAddrInfoPtr = addrInfoPtr->ai_next;

/* if we had some additional resource records in the query response */
           if (matchedAdditionalRecs > 0)
           {
/* Also cleanup the hostnames.  There's no need for them anymore, since
   they're only used to arrange the MX records. */
               if (addrInfoPtr->ai_mxhostname != (char TM_FAR *) 0)
               {
                   tm_free_raw_buffer(
                       (ttRawBufferPtr)addrInfoPtr->ai_mxhostname);
                   addrInfoPtr->ai_mxhostname = (char TM_FAR *) 0;
               }

               if (addrInfoPtr->ai_addr->sa_family == 0)
               {
/* No additional record was found for this hostname.  Remove it from the list
   of MX records. */
                   if (prevAddrInfoPtr == TM_ADDRINFO_NULL_PTR)
                   {
                       entryPtr->dnscAddrInfoPtr = nextAddrInfoPtr;
                   }
                   else
                   {
                       prevAddrInfoPtr->ai_next = nextAddrInfoPtr;
                   }
/* unlink the addrInfo's that follow this one, so that we don't free them */
                   addrInfoPtr->ai_next = TM_ADDRINFO_NULL_PTR;
                   freeaddrinfo(addrInfoPtr);
               }
               else
               {
                   prevAddrInfoPtr = addrInfoPtr;
               }
           }
           else
           {
/* ECR modified for bug ID 1674:
        if there are no additional resource records, then we should do a
        name lookup to get the IP address of this mail server. This results
        from the use of a CNAME alias in the MX record, which is considered
        an invalid DNS configuration however it is sometimes used. */
               prevAddrInfoPtr = addrInfoPtr;
           }

            addrInfoPtr = nextAddrInfoPtr;
        }

        if (entryPtr->dnscAddrInfoPtr == TM_ADDRINFO_NULL_PTR)
        {
            errorCode = TM_DNS_EANSWER;
        }

    }
#ifdef TM_DNS_USE_SRV
    else
/* If this is a response to an SRV query */
    if (   (entryFound == TM_8BIT_YES)
        && (queryType == TM_DNS_QTYPE_SRV) )
    {
/* Move past any authority records to get to the additional records */
        additionalPtr = answerPtr;
        tm_un_ntohs(dnsHdrPtr->dnsNameServerCount, rrCount);
        for (aCount=0; aCount < rrCount; aCount++)
        {
            length = tfDnsLabelLength(additionalPtr);
            rrHdrPtr = (ttDnsRRHeaderPtr)(ttVoidPtr)(additionalPtr + length);

/* Get length from resource record header (unaligned) */
            tm_un_ntohs(rrHdrPtr->dnsrRdLength, rdLength);
            length = (tt16Bit) (length + rdLength);

            additionalPtr += length + TM_DNS_RR_HDR_LEN;
        }

/* Loop through the additional records */
        tm_un_ntohs( dnsHdrPtr->dnsAdditionalCount,rrCount);
        for (aCount = 0; aCount < rrCount; aCount++ )
        {
/* Move past the label */
            length = tfDnsLabelLength(additionalPtr);
            rrHdrPtr = (ttDnsRRHeaderPtr)(ttVoidPtr)(additionalPtr + length);

/* Get the record type */
            tm_un_ntohs(rrHdrPtr->dnsrType, addRecType);
/* Get the next record length */
            tm_un_ntohs(rrHdrPtr->dnsrRdLength, rdLength);

#ifndef TM_USE_IPDUAL
/* Make sure we have a record type we can handle */
#ifdef TM_USE_IPV4
            if (addRecType != TM_DNS_QTYPE_A)
            {
                additionalPtr += (tt16Bit)(length + rdLength) + TM_DNS_RR_HDR_LEN;
                continue;
            }
#else /* TM_USE_IPV4 */
            if (addRecType != TM_DNS_QTYPE_AAAA)
            {
                additionalPtr += (tt16Bit)(length + rdLength) + TM_DNS_RR_HDR_LEN;
                continue;
            }
#endif /* TM_USE_IPV4*/
#endif /* !TM_USE_IPDUAL */

/* Find matching ttAddrInfoExt structure in the cache entry. */
            srvAddrInfoPtr = (ttAddrInfoExtPtr)entryPtr->dnscAddrInfoPtr;
/* Could have two ports on the same host, so scan all entries */
            while (srvAddrInfoPtr != TM_ADDRINFO_EXT_NULL_PTR)
            {
                hostnameMatch =
                    tfDnsLabelAsciiCompare(
                                additionalPtr,
                                (tt8BitPtr) srvAddrInfoPtr->aie_srvhostname,
                                (tt8BitPtr) dnsHdrPtr );

                if (hostnameMatch == 0)
                {
/* We have a match, so allocate an addrinfo struct to store the address */
                    newAddrInfoPtr = tfDnsAllocAddrInfo();
                    if (newAddrInfoPtr == TM_ADDRINFO_NULL_PTR)
                    {
/* Couldn't alloc memory, but we've made it far enough to be OK. */
                        break;
                    }

                    sockAddr.sockPtr = newAddrInfoPtr->ai_addr;
                    rDataPtr = ((tt8BitPtr)(ttVoidPtr)rrHdrPtr) + TM_DNS_RR_HDR_LEN;

                    if (  srvAddrInfoPtr->aie_addrcount
                        < TM_DNS_SRV_ADDRS_PER_HOST)
                    {
/* Save the info in the record to the addrinfo structure */
#if defined(TM_USE_IPV6) && defined(TM_USE_IPV4)
                        if (addRecType == TM_DNS_QTYPE_A)
                        {
                            srvAddrInfoPtr->aie_extflags |=
                                    TM_DNS_SRV_FLAGS_QRY_IP4;
#ifdef TM_DSP
/* Shrink from 4 bytes(which are ints) to one 32-bit */
                            tm_un_ntohl(rDataPtr,
                                        sockAddr.sockInPtr->sin_addr.s_addr);
#else /* !TM_DSP */
/* use copy because we want to keep network byte order */
                            tm_4bytes_copy(rDataPtr,
                                       &(sockAddr.sockInPtr->sin_addr.s_addr));
#endif /* !TM_DSP */

                            sockAddr.sockInPtr->sin_len =
                                    sizeof(struct sockaddr_in);
                            sockAddr.sockInPtr->sin_family = AF_INET;
                            sockAddr.sockInPtr->sin_port =
                                                    srvAddrInfoPtr->aie_port;
                            isV4 = TM_8BIT_YES;
                        }
                        else
                        {
                            srvAddrInfoPtr->aie_extflags |=
                                    TM_DNS_SRV_FLAGS_QRY_IP6;
#ifdef TM_DSP
/* Shrink from sixteen bytes(which are ints) to four 32-bit words */
                            for (i = 0; i < 4; i++)
                            {
                                tm_un_ntohl(rDataPtr + (i * 4),
                                            sockAddr.sockIn6Ptr->sin6_addr.
                                                ip6Addr.ip6U32[i]);
                            }
#else /* !TM_DSP */
/* use tm_bcopy because we want to keep network byte order */
                            tm_bcopy(rDataPtr,
                                    sockAddr.sockIn6Ptr->
                                        sin6_addr.ip6Addr.ip6U8,
                                    sizeof(struct in6_addr));
#endif /* !TM_DSP */
                            sockAddr.sockIn6Ptr->sin6_len =
                                    sizeof(struct sockaddr_in6);
                            sockAddr.sockIn6Ptr->sin6_family = AF_INET6;
                            sockAddr.sockIn6Ptr->sin6_port =
                                                    srvAddrInfoPtr->aie_port;
                            isV4 = TM_8BIT_NO;
                        }

/* Make sure this entry doesn't already exist */
                        tempAddrInfoPtr = tfDnsSrvFindHostFromSockAddr(
                                                    sockAddr.sockNgPtr,
                                                    entryPtr,
                                                    &tempSrvInfoPtr);
                        if (tempAddrInfoPtr)
                        {
/* This entry already exists, so discard the new entry */
                            freeaddrinfo(newAddrInfoPtr);
                            newAddrInfoPtr = TM_ADDRINFO_NULL_PTR;
                        }

                        if (newAddrInfoPtr)
                        {
                            srvAddrInfoPtr->aie_addrcount++;
                            if (isV4 == TM_8BIT_YES)
                            {
                                newAddrInfoPtr->ai_next =
                                        srvAddrInfoPtr->aie_ip4addrs;
                                srvAddrInfoPtr->aie_ip4addrs = newAddrInfoPtr;
                            }
                            else
                            {
                                newAddrInfoPtr->ai_next =
                                        srvAddrInfoPtr->aie_ip6addrs;
                                srvAddrInfoPtr->aie_ip6addrs = newAddrInfoPtr;
                            }
                        }
#else /* defined(TM_USE_IPV6) && defined(TM_USE_IPV4) */
#ifdef TM_USE_IPV4
                        srvAddrInfoPtr->aie_extflags |=
                                TM_DNS_SRV_FLAGS_QRY_IP4;
#ifdef TM_DSP
/* Shrink from 4 bytes(which are ints) to one 32-bit */
                        tm_un_ntohl(rDataPtr,
                                    sockAddr.sockInPtr->sin_addr.s_addr);
#else /* !TM_DSP */
/* use copy because we want to keep network byte order */
                        tm_4bytes_copy(rDataPtr,
                                       &(sockAddr.sockInPtr->sin_addr.s_addr));
#endif /* !TM_DSP */
                        sockAddr.sockInPtr->sin_len =
                                sizeof(struct sockaddr_in);
                        sockAddr.sockInPtr->sin_family = AF_INET;
                        sockAddr.sockInPtr->sin_port =
                                                    srvAddrInfoPtr->aie_port;
/* Make sure this entry doesn't already exist */
                        tempAddrInfoPtr = tfDnsSrvFindHostFromSockAddr(
                                                    sockAddr.sockNgPtr,
                                                    entryPtr,
                                                    &tempSrvInfoPtr);
                        if (tempAddrInfoPtr)
                        {
/* This entry already exists, so discard the new entry */
                            freeaddrinfo(newAddrInfoPtr);
                            newAddrInfoPtr = TM_ADDRINFO_NULL_PTR;
                        }

                        if (newAddrInfoPtr)
                        {
                            srvAddrInfoPtr->aie_addrcount++;
                            newAddrInfoPtr->ai_next =
                                    srvAddrInfoPtr->aie_ip4addrs;
                            srvAddrInfoPtr->aie_ip4addrs = newAddrInfoPtr;
                        }
#else /* TM_USE_IPV4 */
                        srvAddrInfoPtr->aie_extflags |=
                                TM_DNS_SRV_FLAGS_QRY_IP6;
#ifdef TM_DSP
/* Shrink from sixteen bytes(which are ints) to four 32-bit words */
                        for (i = 0; i < 4; i++)
                        {
                            tm_un_ntohl(rDataPtr + (i * 4),
                                        sockAddr.sockIn6Ptr->sin6_addr.
                                            ip6Addr.ip6U32[i]);
                        }
#else /* !TM_DSP */
/* use tm_bcopy because we want to keep network byte order */
                        tm_bcopy(rDataPtr,
                                sockAddr.sockIn6Ptr->sin6_addr.ip6Addr.ip6U8,
                                sizeof(struct in6_addr));
#endif /* !TM_DSP */
                        sockAddr.sockIn6Ptr->sin6_len =
                                sizeof(struct sockaddr_in6);
                        sockAddr.sockIn6Ptr->sin6_family = AF_INET6;
                        sockAddr.sockIn6Ptr->sin6_port =
                                                    srvAddrInfoPtr->aie_port;
/* Make sure this entry doesn't already exist */
                        tempAddrInfoPtr = tfDnsSrvFindHostFromSockAddr(
                                                    sockAddr.sockNgPtr,
                                                    entryPtr,
                                                    &tempSrvInfoPtr);
                        if (tempAddrInfoPtr)
                        {
/* This entry already exists, so discard the new entry */
                            freeaddrinfo(newAddrInfoPtr);
                            newAddrInfoPtr = TM_ADDRINFO_NULL_PTR;
                        }

                        if (newAddrInfoPtr)
                        {
                            srvAddrInfoPtr->aie_addrcount++;
                            newAddrInfoPtr->ai_next =
                                    srvAddrInfoPtr->aie_ip6addrs;
                            srvAddrInfoPtr->aie_ip6addrs = newAddrInfoPtr;
                        }
#endif /* TM_USE_IPV4 */
#endif /* defined(TM_USE_IPV6) && defined(TM_USE_IPV4) */
                    }
                }
                srvAddrInfoPtr = (ttAddrInfoExtPtr)srvAddrInfoPtr->aie_next;
            }
            length = (tt16Bit) (length + rdLength);
            additionalPtr += length + TM_DNS_RR_HDR_LEN;
        }
    }
#endif /* TM_DNS_USE_SRV */

dnsParseResponseExit:

/*
 * 10.  If no records were found that matched our criteria, return
 *     TM_DNS_EANSWER.
 */
    if ( (entryFound == TM_8BIT_ZERO) && (errorCode == TM_ENOERROR) )
    {
        errorCode = TM_DNS_EANSWER;
    }

    return errorCode;
}
#ifdef TM_LINT
LINT_UNINIT_SYM_END(flags)
LINT_UNINIT_SYM_END(rrCount)
#endif /* TM_LINT */


/*
 * tfDnsCacheInvalidate
 *
 * Public API to invalidate the DNS cache.
 *
 */
void tfDnsCacheInvalidate(void)
{
    ttDnsCacheEntryPtr cacheEntryPtr;

/* Lock the DNS global lock */
    tm_call_lock_wait(&tm_context(tvDnsLockEntry));

    cacheEntryPtr = tm_context(tvDnsCachePtr);
    if (cacheEntryPtr != (ttDnsCacheEntryPtr)0)
    {
        do
        {
            tfDnsCloseSocketComplete(cacheEntryPtr);
            cacheEntryPtr->dnscErrorCode = TM_DNS_EANSWER;
            cacheEntryPtr->dnscTtl = 0;
            cacheEntryPtr->dnscRetriesLeft = 0;
            cacheEntryPtr->dnscServerNum = TM_DNS_MAX_SERVERS - 1;
            if (cacheEntryPtr->dnscTimerPtr != (ttTimerPtr)0)
            {
                tm_timer_new_time(cacheEntryPtr->dnscTimerPtr, 0);
            }
            cacheEntryPtr = cacheEntryPtr->dnscNextEntryPtr;
        } while (cacheEntryPtr != tm_context(tvDnsCachePtr));
    }

/* Unlock the global lock */
    tm_call_unlock(&tm_context(tvDnsLockEntry));

    return;
}


/*
 * tfDnsCacheRemove
 *
 * Removes a DNS entry from the cache.
 *
 * 1.  If this entry is not complete (TM_DNSF_COMPLETE not set) or the
 *     information in this entry has not been retrieved by the user
 *     (TM_DNSF_USER not set) return from this routine with an error.
 * 2.  Remove this entry from the DNS cache list.
 * 3.  If this entry was first in the list, lock the DNS global variables,
 *     set tvDnsCachePtr to the new head of the list (if any) and unlock the
 *     DNS global variables.
 * 4.  Decrement the number of entries in the cache.
 * 5.  Deallocate memory associated with this entry:
 *  5.1.    Free timer.
 *  5.2.    Free hostname string.
 *  5.3.    Free request string.
 *  5.4.    Free reverse lookup hostname string.
 *  5.5.    Close the socket.
 *  5.6.    Free DNS entry structure.
 *
 * Parameter       Description
 * cacheEntryPtr   Pointer to the entry to remove from the cache
 *
 * Returns
 * Value           Meaning
 * TM_EINVAL       Invalid cache entry pointer.
 * TM_EINPROGRESS  Cannot remove a cache entry that is currently being used.
 *
 */

static int tfDnsCacheRemove(ttDnsCacheEntryPtr cacheEntryPtr)
{
    int                errorCode;
    ttDnsCacheEntryPtr prevEntryPtr;
    ttDnsCacheEntryPtr nextEntryPtr;

    errorCode = TM_ENOERROR;

#ifdef TM_ERROR_CHECKING
    if (cacheEntryPtr == (ttDnsCacheEntryPtr) 0)
    {
        errorCode = TM_EINVAL;
        goto dnsCacheRemoveExit;
    }
#endif /* TM_ERROR_CHECKING */

/*
 * 1.  If this entry is not complete (TM_DNSF_COMPLETE not set) or the
 *     information in this entry has been reserved by any API,
 *     return from this routine with an error.
 */
    if (   (cacheEntryPtr->dnscFlags & TM_DNSF_COMPLETE)
        && (cacheEntryPtr->dnscCount == 0) )
    {

/* 2.  Remove this entry from the DNS cache list. */
        prevEntryPtr = cacheEntryPtr->dnscPrevEntryPtr;
        nextEntryPtr = cacheEntryPtr->dnscNextEntryPtr;

        prevEntryPtr->dnscNextEntryPtr = nextEntryPtr;
        nextEntryPtr->dnscPrevEntryPtr = prevEntryPtr;

/*
 * 3.  If this entry was first in the list, lock the DNS global variables,
 *     set tvDnsCachePtr to the new head of the list (if any) and unlock the
 *     DNS global variables.
 */
        if (cacheEntryPtr == tm_context(tvDnsCachePtr))
        {

/* This is the last entry in the list; zero out the cache pointer */
            if (nextEntryPtr == tm_context(tvDnsCachePtr))
            {
                tm_context(tvDnsCachePtr) = (ttDnsCacheEntryPtr)0;
            }
            else
            {
                tm_context(tvDnsCachePtr) = nextEntryPtr;
            }
        }

/* 4.  Decrement the number of entries in the cache. */
        tm_context(tvDnsCacheEntries)--;


/* 5.  Deallocate memory associated with this entry: */
/* 5.1.    Free timer. */
        if (cacheEntryPtr->dnscTimerPtr != (ttTimerPtr) 0)
        {
            tm_timer_remove(cacheEntryPtr->dnscTimerPtr);
            cacheEntryPtr->dnscTimerPtr = (ttTimerPtr) 0;
        }

/* Free chain of address information structures, if present. */
        if (cacheEntryPtr->dnscAddrInfoPtr != TM_ADDRINFO_NULL_PTR)
        {
#ifdef TM_DNS_USE_SRV
/* SRV entries use a different structure, and must be free'd differently */
            if (cacheEntryPtr->dnscQueryType == TM_DNS_QTYPE_SRV)
            {
                tfDnsFreeAddrInfoExt(
                        (ttAddrInfoExtPtr)cacheEntryPtr->dnscAddrInfoPtr);
            }
            else
#endif /* TM_DNS_USE_SRV */
            {
                freeaddrinfo(cacheEntryPtr->dnscAddrInfoPtr);
            }
        }

/*  5.2.    Free hostname string. */
        if (cacheEntryPtr->dnscHostnameStr != (ttCharPtr) 0)
        {
            tm_free_raw_buffer((ttRawBufferPtr)cacheEntryPtr->dnscHostnameStr);
        }

/*  5.3.    Free request string. */
        if (cacheEntryPtr->dnscRequestStr != (ttCharPtr) 0)
        {
            tm_free_raw_buffer((ttRawBufferPtr)cacheEntryPtr->dnscRequestStr);
        }

/*  5.4     Free reverse lookup hostname string. */
        if (cacheEntryPtr->dnscRevHostnameStr != (ttCharPtr) 0)
        {
            tm_free_raw_buffer(
                           (ttRawBufferPtr)cacheEntryPtr->dnscRevHostnameStr);
        }

/*  5.5.    Close the socket. */
        tfDnsCloseSocketComplete(cacheEntryPtr);

/*  5.6.    Free DNS entry structure. */
        tm_free_raw_buffer((ttRawBufferPtr) cacheEntryPtr);

    }
    else
    {
        errorCode = TM_EINPROGRESS;
    }

#ifdef TM_ERROR_CHECKING
dnsCacheRemoveExit:
#endif /* TM_ERROR_CHECKING */
    return errorCode;
}


/*
 * tfDnsCacheNew
 *
 * Function Description
 * Adds a new entry to the DNS cache.  If this entry would cause the cache to be
 * too large (according to user's settings) we must remove an entry.  However,
 * since this routine is always called for to create an entry for a new request,
 * even if there is no space in the cache (i.e., it is 'filled' with in progress
 * requests) we will still add it to the list.
 *
 *
 * 1.   If there is not enough room in the cache...
 *  1.1.    Search through the cache backwards, starting with one past the
 *          current entry (which should be the oldest) looking for a cache entry
 *          that can be removed.
 *  1.2.    If one is found, call tfDnsCacheRemove to remove the old entry from
 *          the cache.
 * 2.  Allocate cache entry structure and insert it into the resolver's cache.
 * 3.  Increment the number of entries in the cache.
 *
 * Parameter   Description
 * None
 *
 * Returns
 * Value               Meaning
 * ttDnsCacheEntryPtr  pointer to new cache entry
 * 0                   couldn't create new cache entry.
 *
 */

static ttDnsCacheEntryPtr tfDnsCacheNew(int TM_FAR * retErrorCode)
{

    ttDnsCacheEntryPtr entryPtr;
    tt32Bit            servFlag;
    unsigned char      entryFound;
    int                errorCode;


    entryPtr = (ttDnsCacheEntryPtr) 0;

    if (tm_context(tvDnsInitialized) == TM_8BIT_YES)
    {
        errorCode = TM_ENOERROR;

/* 1.   If there is not enough room in the cache... */
        if ( (tm_context(tvDnsCacheEntries) >= tm_context(tvDnsCacheSize)) )
        {
/*
 * 1.1.    Search through the cache backwards, starting with one past the
 *          current entry (which should be the oldest) looking for a cache entry
 *          that can be removed.
 */

            entryPtr   = tm_context(tvDnsCachePtr);
            tm_assert(tfDnsCacheNew, entryPtr != (ttDnsCacheEntryPtr)0);
            entryFound = TM_8BIT_ZERO;
            do
            {
/* Move back to previous entry */
                entryPtr = entryPtr->dnscPrevEntryPtr;

/*
 * If the request has been completed and not any user is using the data, then
 * it is safe to remove this entry.
 */
                if (    (entryPtr->dnscFlags & TM_DNSF_COMPLETE)
                     && (entryPtr->dnscCount == 0 ))
                {
                    entryFound = TM_8BIT_YES;
                    break;
                }


            } while (entryPtr != tm_context(tvDnsCachePtr));


            if (entryFound == TM_8BIT_YES)
            {
/*
 *  1.3.    If one is found, call tfDnsCacheRemove to remove the old entry from
 *          the cache.
 */
                errorCode = tfDnsCacheRemove(entryPtr);
            }
            else
            {
/* If no entry was found to remove, the cache is still full. */
                 errorCode = TM_DNS_ECACHE_FULL;
            }
        }


        if (errorCode == TM_ENOERROR)
        {
/* 2. Allocate cache entry structure and insert it into the resolver's cache */

            entryPtr = (ttDnsCacheEntryPtr)
                tm_get_raw_buffer((ttPktLen)sizeof(ttDnsCacheEntry) );

            if (entryPtr != (ttDnsCacheEntryPtr) 0)
            {
                tm_bzero(entryPtr, sizeof(ttDnsCacheEntry));
                entryPtr->dnscServerNum =
                    tfDnsGetNextServer(&(entryPtr->dnscCurServerList));
                if (entryPtr->dnscServerNum == -1)
                {
                    entryPtr->dnscServerNum = 0;
                }
                servFlag = (tt32Bit)(1 << (entryPtr->dnscServerNum));
                entryPtr->dnscCurServerList.gen32bitParm |= servFlag;
                entryPtr->dnscRetriesLeft = tm_context(tvDnsRetries);
                entryPtr->dnscSocket = TM_SOCKET_ERROR;
/* Set entry count as 1 for new entry allocation */
                entryPtr->dnscCount = 1;

/* Insert new entry at head of list */
                if (tm_context(tvDnsCachePtr) == (ttDnsCacheEntryPtr) 0)
                {
                    entryPtr->dnscNextEntryPtr = entryPtr;
                    entryPtr->dnscPrevEntryPtr = entryPtr;
                }
                else
                {
                    entryPtr->dnscNextEntryPtr = tm_context(tvDnsCachePtr);
                    entryPtr->dnscPrevEntryPtr =
                        tm_context(tvDnsCachePtr)->dnscPrevEntryPtr;

                    (entryPtr->dnscPrevEntryPtr)->dnscNextEntryPtr = entryPtr;
                    tm_context(tvDnsCachePtr)->dnscPrevEntryPtr = entryPtr;
                }

                tm_context(tvDnsCachePtr) = entryPtr;

/* 3.  Increment the number of entries in the cache. */
                tm_context(tvDnsCacheEntries)++;

            }
            else
            {
                errorCode = TM_ENOMEM;
            }

        }


        if (errorCode != TM_ENOERROR)
        {
            entryPtr = (ttDnsCacheEntryPtr) 0;
        }
    }
    else
    {
/* tfDnsInit has not been called */
        errorCode = TM_EPERM;
    }

    *retErrorCode = errorCode;

    return entryPtr;
}

/*
 * tfDnsCacheLookup
 *
 * Linearly searches the DNS cache for an entry with a matching field (one of
 * the search types below and possibly the query type).  If one is found, return
 * a pointer to this entry, otherwise return NULL.  Subsets of entries are
 * searched according to flags below.  If no flags are set, all entries are
 * searched.
 *
 * Filter
 * Value                   Meaning
 * TM_DNS_LOOKUP_ALL       Search all entries.
 * TM_DNS_LOOKUP_ACTIVE    Only active (ie, in progress) entries are searched.
 *
 *
 * Search Types
 * Value               Meaning  (search based on...)(
 * TM_DNS_SEARCH_ID    Request ID field.
 * TM_DNS_SEARCH_NAME  Hostname and query type.
 *
 * Parameter   Description
 * searchType  Type of search (ID, hostname, etc) to perform.  See above.
 * valuePtr    generic pointer pointing to value appropriate for the type of
 *             search (tt16Bit value for ID, etc).
 * queryType   type of query
 * filter      What subset of the cache to search.
 *
 * Returns
 * Value               Meaning
 * 0                   No entry with matching hostname found, or invalid param.
 * ttDnsCacheEntryPtr  Pointer to matching entry.
 *
 */

static ttDnsCacheEntryPtr tfDnsCacheLookup( tt8Bit    searchType,
                                            ttVoidPtr valuePtr,
                                            tt16Bit   queryType,
                                            tt8Bit    filter)
{

    ttDnsCacheEntryPtr entryPtr;
    int                cacheEntries;
    int                errCode;
    tt8Bit             entryPassed;
    tt8Bit             entryActive;
    tt8Bit             entryComplete;
    ttDnsCacheEntryPtr foundEntryPtr;



    entryPtr     = tm_context(tvDnsCachePtr);
    cacheEntries = tm_context(tvDnsCacheEntries);
    foundEntryPtr     = (ttDnsCacheEntryPtr) 0;

#ifdef TM_ERROR_CHECKING
    if (valuePtr == (ttVoidPtr) 0)
    {
        goto dnsCacheLookupExit;
    }
#endif /* TM_ERROR_CHECKING */

    while ( cacheEntries && (foundEntryPtr == (ttDnsCacheEntryPtr) 0) )
    {
        cacheEntries--;
        entryPassed = TM_8BIT_YES;
/*
 * Check to see if this entry passes our 'filter' (only active entries, only
 * completed entries, or all entries.
 */
        if (filter != TM_DNS_LOOKUP_ALL)
        {
            entryActive   = (tt8Bit) (filter == TM_DNS_LOOKUP_ACTIVE);
            entryComplete = (tt8Bit)
                            ((entryPtr->dnscFlags & TM_DNSF_COMPLETE) > 0);

            if ((!entryActive) || entryComplete)
            {
                entryPassed = TM_8BIT_ZERO;
            }
        }

/*
 * If this entry passed our filter, check this entry according to the type
 * of the search.
 */
        if (entryPassed == TM_8BIT_YES)
        {
            switch (searchType)
            {

/* DNS request ID */
                case TM_DNS_SEARCH_ID:
                    if ( *((tt16BitPtr) valuePtr) == entryPtr->dnscRequestId )
                    {
                        foundEntryPtr = entryPtr;
                    }
                    break;

/* DNS hostname and query type */
                case TM_DNS_SEARCH_NAME:
                    if (entryPtr->dnscHostnameStr != (ttCharPtr) 0)
                    {
                        errCode = tm_stricmp((ttConstCharPtr) valuePtr,
                                             entryPtr->dnscHostnameStr);
                        if (    ( errCode == 0 )
                             && ( queryType == entryPtr->dnscQueryType ) )
                        {
                            foundEntryPtr = entryPtr;
                        }
                    }
                    break;

                default:
                    break;
            }
        }

        entryPtr = entryPtr->dnscNextEntryPtr;

    }

#ifdef TM_ERROR_CHECKING
dnsCacheLookupExit:
#endif /* TM_ERROR_CHECKING */


    return foundEntryPtr;
}


/*
 * tfDnsSendRequest
 *
 * Constructs a DNS query packet based on the hostname and type passed in, and
 * sends this packet to the specified DNS server.
 *
 * 1.  Get DNS server's IP address.
 * 2.  Allocate zero copy buffer for DNS query.
 * 3.  Formulate header (all other fields not used and set to zero):
 *          ID = current ID number (dnscRequestId)
 *          Query/Response = 0 (query)
 *          Opcode = 0 (standard query)
 *          Recursion Desired = 1 (yes)
 *          Question count = 1
 * 4.  Copy the request string (the DNS label formatted hostname, which was
 *     converted when this request was first generated) into the packet.
 * 5.  Set question type to queryType (host address, MX record, etc) and set
 *     type to 'IN', the Internet.
 * 6.  Send completed datagram to specified DNS server.
 *
 * Parameter     Description
 * hostnamePtr   Hostname to be resolved.
 * queryType     Type of query (A, MX, etc)
 * dnsServerNum  Index in server list (1st, 2nd, etc) of current DNS server.
 * requestId     Request ID of this query.
 *
 * Returns
 * Value   Meaning
 * tt16Bit ID of request sent
 *
 */
#ifdef TM_LINT
LINT_UNACCESS_SYM_BEGIN(flags)
LINT_UNACCESS_SYM_BEGIN(queryType)
#endif /* TM_LINT */
static int tfDnsSendRequest( ttDnsCacheEntryPtr cacheEntryPtr,
                             tt16Bit            queryType
#ifdef TM_USE_IPV6
                             , tt16Bit          flags
#endif /* TM_USE_IPV6 */
                             )
{

    ttUserBuffer       bufHandle;
    ttDnsHeaderPtr     hdrPtr;
    char TM_FAR *      dataPtr;
    ttCharPtr          hostnamePtr;
    ttSockAddrPtrUnion serverSockAddr;
    int                errorCode;
    int                bufLength;
    int                dnsServerNum;
    tt16Bit            dnsflags;
    tt16Bit            requestId;

    errorCode = TM_ENOERROR;

    hostnamePtr = cacheEntryPtr->dnscRequestStr;
    dnsServerNum = cacheEntryPtr->dnscServerNum;
    requestId = cacheEntryPtr->dnscRequestId;

/*
 * 1.  Get DNS server's IP address.
 */
    serverSockAddr.sockNgPtr =
        &(tm_context(tvDnsServerList[dnsServerNum]).dseServerAddress);

/* 2.  Allocate zero copy buffer for DNS query. */
    bufLength =   (int)(tm_strlen(hostnamePtr) + 1
                + tm_byte_count(TM_DNS_HEADER_SIZE) + 4);
#ifdef TM_USE_IPV6
    if (flags & TM_6_DNSF_IP6_ARPA)
    {
        bufLength += 5; /* ".arpa" */
    }
    else if (flags & TM_6_DNSF_IP6_INT)
    {
        bufLength += 4; /* ".int" */
    }
#endif /* TM_USE_IPV6 */
    bufHandle = tfGetZeroCopyBuffer( tm_byte_count(bufLength), &dataPtr );

    if (bufHandle != (ttUserBuffer) 0)
    {

/*
 * 3.  Formulate header (all other fields not used and set to zero):
 */
        hdrPtr = (ttDnsHeaderPtr) dataPtr;

        tm_bzero(hdrPtr, tm_byte_count(TM_DNS_HEADER_SIZE));

        tm_un_htons(requestId, hdrPtr->dnsId);

/* Query packet, recursion desired */
/* Set opcode to either standard or inverse query */
        dnsflags = TM_DNS_HDR_RECURSE | TM_DNS_HDR_QUERY | TM_DNS_OPCODE_QUERY;

        tm_un_htons(dnsflags, hdrPtr->dnsFlags);

/* Only one question contained in this packet. */
        hdrPtr->dnsQuestionCount[1] = 1; /* LSB */

/*
 * 4.  Copy the request string (the DNS label formatted hostname, which was
 *     converted when this request was first generated) into the packet.
 */
        dataPtr += TM_DNS_HEADER_SIZE;
        tm_strcpy(dataPtr, hostnamePtr);

/*
 * 5.  Set question type to queryType (host address, MX record, etc) and set
 *     type to 'IN', the Internet.
 */
        dataPtr += tm_strlen(hostnamePtr) + 1;

/*
 * If this is an IPv6 reverse lookup (TM_6_DNSF_IP6_ARPA or TM_6_DNSF_IP6_INT
 * flags are set), append the appropriate suffix, either '.int' or
 * '.arpa'.
 */
#ifdef TM_USE_IPV6
        if (flags & TM_6_DNSF_IP6_ARPA)
        {
            *(dataPtr-1) = 4;
            tm_strcpy(dataPtr, "arpa");
            dataPtr += 5;
        }
        else if (flags & TM_6_DNSF_IP6_INT)
        {
            *(dataPtr-1) = 3;
            tm_strcpy(dataPtr, "int");
            dataPtr += 4;
        }
#endif /* TM_USE_IPV6 */
/* PRQA: QAC Message 3711: Implicit conversion: unsigned char to char. */
/* PRQA S 3711 L1 */
        tm_un_htons(queryType, dataPtr);
/* PRQA L:L1 */
        dataPtr += 2;
        tm_un_htons((tt16Bit)TM_DNS_CLASS_IN, dataPtr);
/* Unlock the global lock */
        tm_call_unlock(&tm_context(tvDnsLockEntry));
/*
 * The sockaddr structure stored in the DNS server list is completely filled
 * out (address, family, port, etc) so it is not necessary to know whether
 * this address is IPv4 or IPv6 - we can just pass the address structure
 * directly to tfZeroCopySendTo.
 */


/* 6.  Send completed datagram to specified DNS server. */
        if (cacheEntryPtr->dnscSocket == TM_SOCKET_ERROR)
        {
            cacheEntryPtr->dnscSocket = tfDnsOpenSocket();
        }
        if (cacheEntryPtr->dnscSocket == TM_SOCKET_ERROR)
        {
            errorCode = TM_EMFILE;
        }

        errorCode = tfZeroCopySendTo( cacheEntryPtr->dnscSocket,
                                      bufHandle,
                                      bufLength,
                                      0,
                                      serverSockAddr.sockPtr,
                                      serverSockAddr.sockPtr->sa_len );

        if (errorCode < 0)
        {
/* Return socket error code to caller */
            errorCode = tfGetSocketError(cacheEntryPtr->dnscSocket);
            if (errorCode == TM_EWOULDBLOCK)
            {
                (void)tfFreeZeroCopyBuffer(bufHandle);
            }
        }
        else
        {
            errorCode = TM_ENOERROR;
        }
/* Lock the DNS global lock */
        tm_call_lock_wait(&tm_context(tvDnsLockEntry));
    }
    else
    {
/* No memory available for zero copy buffer */
        errorCode = TM_ENOBUFS;
    }

    return errorCode;
}
#ifdef TM_LINT
LINT_UNACCESS_SYM_END(flags)
LINT_UNACCESS_SYM_END(queryType)
#endif /* TM_LINT */

/*
 *
 * tfDnsTimeout
 *
 * Indicates that no response has been receieved from the DNS server; attempt
 * to retransmit request.
 *
 *  1.  If dnscRetriesLeft is greater than zero. . .
 *      1.1.    Create and send DNS request by calling tfDnsSendRequest.  Set
 *              request ID number and query type in cache entry.
 *      1.2.    Start retransmission timer.
 *      1.3.    Decrement the number of retries remaining (dnscRetriesLeft).
 *  2.  If dnscRetriesLeft is zero, our retransmission limit has run out.
 *      2.1.    If the current DNS server is not the last in the list
 *          2.1.1.  Go on to try the next DNS server.
 *          2.1.2.  Restart this entry's retry count.
 *          2.1.3.  Create and send DNS request by calling tfDnsSendRequest.
 *                  Set request ID and query type in cache entry.
 *          2.1.4.  Start retransmission timer.
 *      2.2.    If DNS server list is exhausted set error code to ETIMEDOUT.
 *  3.  If an error occurred record it in the cache entry and post on this
 *      entry if necessary.
 *
 *
 * Parameter   Description
 * paramPtr    Points to cache entry associated with this timer.
 *
 * Returns
 * Value   Meaning
 * None
 *
 */

static void tfDnsTimeout (ttVoidPtr      timerBlockPtr,
                          ttGenericUnion userParm1,
                          ttGenericUnion userParm2)
{
    ttDnsCacheEntryPtr entryPtr;
    ttTimerPtr         timerPtr;
    tt32Bit            servFlag;
    int                errorCode;
    int                nextServerIndex;
    unsigned char      sendRequest;
#ifdef TM_USE_IPV6
    tt16Bit            flags;
#endif /* TM_USE_IPV6 */

    /* Avoid compiler warnings about unused parameters */
    TM_UNREF_IN_ARG(userParm2);
    timerPtr = (ttTimerPtr)timerBlockPtr;

    entryPtr  = (ttDnsCacheEntryPtr) userParm1.genVoidParmPtr;
    errorCode = TM_ENOERROR;

/* 0. Lock the DNS global lock */
    tm_call_lock_wait(&tm_context(tvDnsLockEntry));
    if (tm_timer_not_reinit(timerPtr))
    {

/*  1.  If dnscRetriesLeft is greater than zero. . . */
        if (entryPtr->dnscRetriesLeft > 0)
        {
/*
 *      1.1.    Create and send DNS request by calling tfDnsSendRequest.  Set
 *              request ID number and query type in cache entry.
 *      1.2.    Start retransmission timer.
 * (The shared code to actually perform these functions is at the end of the
 *  routine)
 */
            sendRequest = TM_8BIT_YES;



/*      1.3.    Decrement the number of retries remaining (dnscRetriesLeft). */
            entryPtr->dnscRetriesLeft--;

        }
        else
        {
/*  2.  If dnscRetriesLeft is zero, our retransmission limit has run out. */
            tm_context(tvDnsServerList[entryPtr->dnscServerNum]).dseFailures++;
/*      2.1.    If the current DNS server is not the last in the list... */
            nextServerIndex =
                tfDnsGetNextServer(&(entryPtr->dnscCurServerList));

            if (nextServerIndex != -1)
            {
/*          2.1.1.  Go on to try the next DNS server. */
                servFlag = (tt32Bit)(1 << nextServerIndex);
                entryPtr->dnscCurServerList.gen32bitParm |= servFlag;
                entryPtr->dnscServerNum = nextServerIndex;
                entryPtr->dnscRequestId = tfDnsGetNewTransId();
                tfDnsCloseSocket(entryPtr);
                entryPtr->dnscSocket = tfDnsOpenSocket();

/*          2.1.2.  Restart this entry's retry count. */
                entryPtr->dnscRetriesLeft = tm_context(tvDnsRetries);

/*
 *          2.1.3.  Create and send DNS request by calling tfDnsSendRequest.
 *                  Set request ID and query type in cache entry.
 *          2.1.4.  Start retransmission timer.
 * (The shared code to actually perform these functions is at the end of the
 *  routine)
 */
                sendRequest = TM_8BIT_YES;
/* set the new timeout */
                tm_timer_new_time(entryPtr->dnscTimerPtr,
                            (tt32Bit)tm_context(tvDnsTimeout) * TM_UL(1000));

            }
            else
            {
/*      2.2.    If DNS server list is exhausted set error code to ETIMEDOUT. */
                sendRequest = TM_8BIT_ZERO;
                errorCode = TM_ETIMEDOUT;
                tm_timer_remove(entryPtr->dnscTimerPtr);
                entryPtr->dnscTimerPtr = (ttTimerPtr) 0;
                tfDnsCloseSocketComplete(entryPtr);
            }
        }
/* increase entry count before possible send and post */
        entryPtr->dnscCount++;

        if (sendRequest == TM_8BIT_YES)
        {
#ifdef TM_USE_IPV6
            flags = (tt16Bit) (
                entryPtr->dnscFlags
                & (TM_6_DNSF_IP6_ARPA | TM_6_DNSF_IP6_INT));
#endif /* TM_USE_IPV6 */
/* Unlock the DNS global lock */

            errorCode = tfDnsSendRequest(entryPtr,
                                         entryPtr->dnscQueryType
#ifdef TM_USE_IPV6
                                         , flags
#endif /* TM_USE_IPV6 */
                                         );

            if (errorCode != TM_ENOERROR)
            {
                if (entryPtr->dnscTimerPtr != (ttTimerPtr)0)
                {
                     nextServerIndex =
                        tfDnsGetNextServer(&(entryPtr->dnscCurServerList));

                    if (nextServerIndex != -1)
/*
 * If send is not successful, set timeout to 0, so that this function is
 * called again immediately to try the next server.
 */
                    {
                        entryPtr->dnscRetriesLeft = 0;
                        tm_timer_new_time(entryPtr->dnscTimerPtr, TM_UL(0));
                        errorCode = TM_ENOERROR;
                    }
                    else
                    {
/* if there is no more server to try, return */
                        tm_timer_remove(entryPtr->dnscTimerPtr);
                        entryPtr->dnscTimerPtr = (ttTimerPtr) 0;
                    }
                }
            }
        }
/*
 *  3.  If an error occurred record it in the cache entry and post on this
 *      entry if necessary.
 */
        if (errorCode != TM_ENOERROR)
        {
            tfDnsCloseSocketComplete(entryPtr);
            entryPtr->dnscErrorCode = errorCode;
#ifdef TM_PEND_POST_NEEDED
            tm_post(&entryPtr->dnscPendEntry,
                    &tm_context(tvDnsLockEntry), errorCode);
#endif /* TM_PEND_POST_NEEDED */
        }
/* decrease entry count */
        entryPtr->dnscCount--;

    }
    tm_call_unlock(&tm_context(tvDnsLockEntry));
}

/*
 * tfDnsGetNextServer
 *
 * Obtains the index of the next server to query, checking that it has not
 * been queried before.
 *
 * Parameter            Description
 *  prevServersTried    A flags variable indicating which servers have
 *                      been previously queried.
 *
 * Returns
 * Value                Meaning
 * -1                   All servers have been previously queried.
 * >=0                  Index of the next server to query.
 */
static int tfDnsGetNextServer(ttGenericUnionPtr prevServersTriedPtr)
{
    ttDnsServerEntry    dnsServerEntry;
    tt32Bit             servFlag;
    int                 servIndex;
    int                 nextServerIndex;
    unsigned int        nextServerFailures;

    nextServerIndex = -1;
    nextServerFailures = 0;
    for (servIndex=0; servIndex<TM_DNS_MAX_SERVERS; servIndex++)
    {
        dnsServerEntry = tm_context(tvDnsServerList)[servIndex];
        if (dnsServerEntry.dseServerAddress.ss_family == 0)
        {
            continue;
        }

/* Don't bother checking further if we've already used this server */
        servFlag = (tt32Bit)(1 << servIndex);
        if (((prevServersTriedPtr->gen32bitParm) & servFlag) != 0)
        {
/* We have used this server before */
            continue;
        }

/* If the server is behaving, use it. */
        if (dnsServerEntry.dseFailures == 0)
        {
            nextServerIndex = servIndex;
            break;
        }
        else
        {
            if (   (nextServerIndex == -1)
                || (dnsServerEntry.dseFailures < nextServerFailures))
            {
                nextServerIndex = servIndex;
                nextServerFailures = dnsServerEntry.dseFailures;
            }
        }
    }

    return nextServerIndex;
}


/*
 * tfDnsGenerateQuery
 *
 * This routine attempts to retrieve information about a hostname and is called
 * once for each request from the user.  First, the cache is checked for valid
 * entries; if any are found it is returned to the caller.  If not, a DNS query
 * is generated and sent.
 *
 * 1.  Check cache for entry matching this hostname by calling tfDnsCacheLookup
 *     (search all entries by name).
 * 2.  If entry is found...
 *      2.1.    If this entry is complete (TM_DNSF_COMPLETE set)...
 *          2.1.1.  Check TTL value for this entry.  This is done according to
 *                  the following algorithm, where t is the current time, t0 is
 *                  the timestamp of when this entry was added and tD is when
 *                  the entry expires (t0 + TTL).
 *              2.1.1.1.    If t0 < tD (no overflow occurred), t must be greater
 *                          than t0 and less than tD.
 *              2.1.1.2.    If t0 > tD (overflow), t must either be greater than
 *                          t0 and tD or t must be less than t0 and tD.
 *              2.1.1.3.    If the TTL is 0, caching is prohibited.
 *                          See RFC 1034 section 3.6.
 *              2.1.1.4.    If the SRV host cannot be resolved.
 *          2.1.2.  If TTL has expired, remove entry from cache
 *                  (tfDnsCacheRemove) and make new request (go to 3.)
 *          2.1.3.  If an error occurred during this lookup and the user has
 *                  already retrieved this entry, remove it.
 *          2.1.4.  Otherwise, return the error code to the user.
 *      2.2.    If this entry is not complete (i.e., the request is in progress)
 *          2.2.1.  If in non-blocking mode, return TM_EWOULDBLOCK.
 *          2.2.2.  If in blocking mode, treat this as a new request and goto 3.
 *  3.  If no cache entry is found, make new request...
 *      3.1.    Create a new cache entry for this request (tfDnsCacheNew)
 *      3.2.    Allocate and set hostname string for this entry
 *      3.3.    Convert ASCII hostname to DNS label format and store the result
 *              in the cache entry.
 *      3.4.    Set the request ID number and query type in cache entry.
 *      3.5.    Create and start timer for this request (timer value set to
 *              tvDnsTimeout).
 *      3.6.    Create and send DNS request by calling tfDnsSendRequest.
 *      3.7.    Set this entry's dnscRetriesLeft to tvDnsRetries.
 *      3.8.   If in non-blocking mode, return TM_EWOULDBLOCK
 *      3.9.   If in blocking mode, pend on this entry
 *          3.9.1. Once pend wakes up, return info to the user
 *
 *
 * Parameter      Description
 * hostnameStr    Hostname to resolve.
 * queryType
 * addrFamily
 * entryPtrPtr
 *
 * Returns
 * Value            Meaning
 * TM_EINVAL        Invalid host name string or IP address pointer.
 * TM_EWOULDBLOCK   DNS lookup in progress.  The user should continue to call
 *                  tfDnsGetHostByName with the same parameters until it returns
 *                  a value other than TM_EWOULDBLOCK.
 * TM_ENOERROR      DNS lookup successful, IP address stored in *ipAddressPtr.
 *
 */

int tfDnsGenerateQuery( ttConstCharPtr        hostnameStr,
                        tt16Bit               queryType,
                        int                   addrFamily,
                        ttDnsCacheEntryPtrPtr entryPtrPtr )
{
    ttDnsCacheEntryPtr entryPtr;
    ttCharPtr          strChrResult;
    ttGenericUnion     timerParm1;
    int                errorCode;
    int                hostnameLength;
    int                labelLen;
    tt32Bit            currentTime;
    tt32Bit            timeStamp;
    tt32Bit            deadline;
#ifdef TM_USE_IPV6
    tt16Bit            flags;
#endif /* TM_USE_IPV6 */
    tt8Bit             validEntry;
    tt8Bit             needPend;

    addrFamily  = addrFamily;

    needPend    = TM_8BIT_YES;
    errorCode   = TM_ENOERROR; /* Compiler warning */
    entryPtr    = (ttDnsCacheEntryPtr)0;

    strChrResult = tm_strchr(hostnameStr, '.');
    if (strChrResult != (ttCharPtr)0)
    {
        labelLen = (int)(ttUserPtrCastToInt)
                        (strChrResult - (ttCharPtr)hostnameStr);
    }
    else
    {
        labelLen = (int)tm_strlen(hostnameStr);
    }
    if (labelLen > TM_DNS_MAX_LABEL_LEN)
    {
        errorCode = TM_EINVAL;
        goto dnsGenerateQueryExit;
    }
/*
 * 1.  Check cache for entry matching this hostname by calling tfDnsCacheLookup
 *     (search all entries by name).
 */
    while (needPend)
    {
        needPend = TM_8BIT_NO;
        entryPtr = tfDnsCacheLookup( TM_DNS_SEARCH_NAME,
                                     (ttVoidPtr) hostnameStr,
                                     queryType,
                                     TM_DNS_LOOKUP_ALL );


/* 2.  If entry is found... */
        if (entryPtr != (ttDnsCacheEntryPtr) 0)
        {

/*      2.1.    If this entry is complete (TM_DNSF_COMPLETE set)... */
            if (entryPtr->dnscFlags & TM_DNSF_COMPLETE)
            {
/*
 *          2.1.1.  Check TTL value for this entry.  This is done according to
 *                  the following algorithm, where t is the current time, t0 is
 *                  the timestamp of when this entry was added and tD is when
 *                  the entry expires (t0 + TTL).
 */
                tm_kernel_set_critical;
                currentTime = tvTime;
                tm_kernel_release_critical;

                timeStamp   = entryPtr->dnscTimestamp;
                deadline    = timeStamp + entryPtr->dnscTtl;
                validEntry = TM_8BIT_YES;

/*
 * Only check the TTL of the entry if the user has already retrieved the
 * information.  Otherwise entries with short (or zero) time to live values
 * might be removed before the user can get the data (especially in
 * non-blocking mode).
 */

/*
 *              2.1.1.1.    If t0 < tD (no overflow occurred), t must be greater
 *                          than t0 and less than tD.
 */

                if (    (timeStamp < deadline)
                    && ((currentTime < timeStamp)
                        || (currentTime >= deadline)))
                {
                    validEntry = TM_8BIT_ZERO;
                }
/*
 *              2.1.1.2.    If t0 > tD (overflow), t must either be greater than
 *                          t0 and tD or t must be less than t0 and tD.
 */
                if (   (timeStamp > deadline)
                    && (currentTime < timeStamp)
                    && (currentTime >= deadline))
                {
                    validEntry = TM_8BIT_ZERO;
                }
/*
 *              2.1.1.3.    If the TTL is 0, caching is prohibited.
 *                          See RFC 1034 section 3.6.
 */
                if (   (entryPtr->dnscTtl == 0)
                    && (tm_16bit_one_bit_set(entryPtr->dnscFlags, TM_DNSF_USER)))
                {
                    validEntry = TM_8BIT_ZERO;
                }
#ifdef TM_DNS_USE_SRV
/*
 *              2.1.1.4.    If the SRV host cannot be resolved.
 */
                if (   (queryType == TM_DNS_QTYPE_SRV)
                    && (entryPtr->dnscAddrInfoPtr == TM_ADDRINFO_NULL_PTR))
                {
                    validEntry = TM_8BIT_ZERO;
                }
#endif /* TM_DNS_USE_SRV */
/* If the entry TTL is expired or error complete, then resend requeust
 * for the last requeust in all requeusts
 */
/*
 *          2.1.2.  If TTL has expired, remove entry from cache
 *                  (tfDnsCacheRemove) and make new request (go to 3.)
 */
                if (validEntry == TM_8BIT_ZERO)
                {
/* In this path, dnscCount will be increased in the new cache */
                    if ( entryPtr->dnscCount == 0 )
                    {
                        (void)tfDnsCacheRemove(entryPtr);
                        entryPtr = (ttDnsCacheEntryPtr) 0;
                    }
                    else
                    {
/*
 * if another task is still checking the result, return the error and do not
 * spawn a new entry
 */
                        errorCode = TM_DNS_EANSWER;
                    }

                }
                else
                {
/*
 *          2.1.3.  If an error occurred during this lookup and no user is
 *                  using this entry, remove it and use a new entry for the query.
 */
                    if  ((entryPtr->dnscErrorCode != TM_ENOERROR)
                        && ( entryPtr->dnscCount == 0 )
                        && ( entryPtr->dnscFlags & TM_DNSF_USER))
                    {
/* In this path, dnscCount will be increased in the new cache */
                        (void)tfDnsCacheRemove(entryPtr);
                        entryPtr = (ttDnsCacheEntryPtr) 0;
                    }
                    else
                    {
/*
 *          2.1.4.  Otherwise, return the error code to the user.
 */
                        errorCode = entryPtr->dnscErrorCode;
/* return result once */
                        entryPtr->dnscFlags |= TM_DNSF_USER;
                    }

                }
            }
            else
            {
/*
 *      2.2.    If this entry is not complete (the request is in progress)
 */
#ifdef TM_PEND_POST_NEEDED
                if (tm_context(tvDnsBlockingMode) != TM_BLOCKING_OFF)
                {
/* If using an RTOS, pend on the request until we get a response or timeout*/
/* Increase entry count before we pend */
                    entryPtr->dnscCount++;
                    errorCode = tm_pend(&(entryPtr->dnscPendEntry),
                                        &tm_context(tvDnsLockEntry));
/* Decrease entry count because it is already protected by lock */
                    entryPtr->dnscCount--;
/* re-run from tfDnsCacheLookup */
                    needPend   = TM_8BIT_YES;
                }
                else
#endif /* TM_PEND_POST_NEEDED */
                {
/*          2.2.1.  If in non-blocking mode, return TM_EWOULDBLOCK. */
/* set the entry count as 1 for BLOCKING_OFF mode */
                    errorCode = TM_EWOULDBLOCK;
                }
            }
        }
    }
/* increase entry count */
    if (entryPtr != (ttDnsCacheEntryPtr) 0)
    {
        entryPtr->dnscCount++;
    }

/*  3.  If no cache entry is found, make new request... */
    if (entryPtr == (ttDnsCacheEntryPtr) 0)
    {
/*      3.1.    Create a new cache entry for this request (tfDnsCacheNew) */
        entryPtr = tfDnsCacheNew(&errorCode);
        if (entryPtr == (ttDnsCacheEntryPtr)0)
        {
/*
 * Exit the routine if adding a new entry fails.  The error code is set by
 * tfDnsCacheNew and this value is then returned from this function.
 */
            goto dnsGenerateQueryExit;
        }


/*      3.2.    Allocate and set hostname string for this entry */
        hostnameLength = (int)tm_strlen(hostnameStr);
        if (hostnameLength > TM_DNS_MAX_HOSTNAME_LEN)
        {
            errorCode = TM_EINVAL;
            goto dnsGenerateQueryExit;
        }
        entryPtr->dnscHostnameStr =
            (ttCharPtr) tm_get_raw_buffer((ttPktLen)hostnameLength + 1);

        if (entryPtr->dnscHostnameStr == (ttCharPtr) 0)
        {
            errorCode = TM_ENOMEM;
            goto dnsGenerateQueryExit;
        }

        tm_strcpy(entryPtr->dnscHostnameStr, hostnameStr);

/*
 *      3.3.    Convert ASCII hostname to DNS label format and store the result
 *              in the cache entry.
 */
        entryPtr->dnscRequestStr =
            (ttCharPtr) tm_get_raw_buffer((ttPktLen)hostnameLength + 2);

        if (entryPtr->dnscRequestStr == (ttCharPtr) 0)
        {
            errorCode = TM_ENOMEM;
            goto dnsGenerateQueryExit;
        }

/* Convert ASCII to DNS label. The length of the label string is actually
 * hostnameLength + 2 (allocated above), but tfDnsAsciiToLabel doesn't
 * fill in the terminating 0 length */
        labelLen = tfDnsAsciiToLabel((ttConstCharPtr)hostnameStr,
                                     hostnameLength,
                                     (tt8BitPtr)entryPtr->dnscRequestStr,
                                     hostnameLength + 1);
        entryPtr->dnscRequestStr[labelLen] = 0;

        entryPtr->dnscQueryType = queryType;

/*
 * If this is an IPv6 address to hostname lookup, set the TM_6_DNSF_IP6_ARPA
 * flag, so the query will be done in the IP6.ARPA domain first.
 */
#ifdef TM_USE_IPV6
        if (    (queryType == TM_DNS_QTYPE_REV)
             && (addrFamily == AF_INET6))
        {
            entryPtr->dnscFlags |= TM_6_DNSF_IP6_ARPA;
        }
#endif /* TM_USE_IPV6 */

/*
 *      3.4.   Set the request ID number and query type in cache entry.
 */
#ifdef TM_USE_IPV6
        flags = (tt16Bit) (
            entryPtr->dnscFlags & (TM_6_DNSF_IP6_ARPA | TM_6_DNSF_IP6_INT));
#endif /* TM_USE_IPV6 */

/*
 *      3.5.    Create and start timer for this request (timer value set to
 *              tvDnsTimeout). Timer should be started before sending the
 *              request.
 */
        timerParm1.genVoidParmPtr = (ttVoidPtr) entryPtr;
        entryPtr->dnscTimerPtr =
                tfTimerAdd( tfDnsTimeout,
                            timerParm1,
                            timerParm1, /* unused */
                            (tt32Bit)tm_context(tvDnsTimeout) * TM_UL(1000),
                            TM_TIM_AUTO );
/*
 * Initialize the dnscRequestId to be the current global Query ID.
 * Then increment the global value by the max number of servers
 * so we can avoid an overlap in Query IDs.
 */
        entryPtr->dnscRequestId = tfDnsGetNewTransId();

/*      3.6.    Create and send DNS request by calling tfDnsSendRequest. */
        errorCode = tfDnsSendRequest(entryPtr,
                                     queryType
#ifdef TM_USE_IPV6
                                     , flags
#endif /* TM_USE_IPV6 */
                                     );

        if (errorCode != TM_ENOERROR)
        {
/*
 * If sending is unsucessful, do not retry.
 * Instead, set the timeout timer to fire immediately
 * and it will automatically retry using the next DNS server.
 */
            entryPtr->dnscRetriesLeft = 0;
            if (entryPtr->dnscTimerPtr != (ttTimerPtr)0)
            {
                tm_timer_new_time(entryPtr->dnscTimerPtr, TM_UL(0));
            }
        }
        else
        {
/*      3.7.    Set this entry's dnscRetriesLeft to tvDnsRetries. */
            entryPtr->dnscRetriesLeft = tm_context(tvDnsRetries);
        }
        if (entryPtr->dnscFlags & TM_DNSF_COMPLETE)
        {
            errorCode = entryPtr->dnscErrorCode;
        }
        else
        {
/* we did not receive a response (or timeout) */
#ifdef TM_PEND_POST_NEEDED
            if (tm_context(tvDnsBlockingMode) != TM_BLOCKING_OFF)
            {
                errorCode = tm_pend(&(entryPtr->dnscPendEntry),
                                    &tm_context(tvDnsLockEntry));
/*          3.9.1. Once pend wakes up, return info to the user */
            }
            else
#endif /* TM_PEND_POST_NEEDED */
/*      3.8.   If in non-blocking mode, return TM_EWOULDBLOCK */
            {
                errorCode = TM_EWOULDBLOCK;
            }
        }
    }


dnsGenerateQueryExit:
    if (entryPtr != (ttDnsCacheEntryPtr)0)
    {
/* Decrease entry count by 1 */
        entryPtr->dnscCount--;
        if (errorCode != TM_EWOULDBLOCK)
        {
/* for BLOCKING_ON mode, return the entry once */
            entryPtr->dnscFlags |= TM_DNSF_USER;
        }
    }
    *entryPtrPtr = entryPtr;
    return errorCode;
}


/*
 * tfDnsAllocAddrInfo Function Description
 *
 * Allocates and zeros out a new addrinfo structure, including the attached
 * sockaddr_storage structure.  This is used by getaddrinfo and internally by
 * the DNS resolver.
 *
 * Parameters
 * None
 *
 * Returns
 * A pointer to the new structure if successful, NULL if the allocation fails.
 *
 * 1. Allocate a single raw buffer large enough to hold the addrinfo and
 *    sockaddr_storage structures.  This is allocated as a single buffer to
 *    conserve memory and so that it may be freed as a single buffer as well.
 * 2. If the allocation was successful
 *     2.1. Zero out the addrinfo and sockaddr_storage structures.
 *     2.2. Set the internal sockaddr pointer to point at the newly allocated
 *          sockaddr_storage structure, which immediately follows the addrinfo
 *          structure.
 */
static struct addrinfo TM_FAR * tfDnsAllocAddrInfo(void)
{
    struct addrinfo TM_FAR * addrInfoPtr;
    int                      allocSize;

/*
 * 1. Allocate a single raw buffer large enough to hold the addrinfo and
 *    sockaddr_storage structures.  This is allocated as a single buffer to
 *    conserve memory and so that it may be freed as a single buffer as well.
 */
    allocSize = sizeof(struct addrinfo) + sizeof(struct sockaddr_storage);
    addrInfoPtr = (struct addrinfo TM_FAR *) 
                                    tm_get_raw_buffer((ttPktLen)allocSize);

/* 2. If the allocation was successful */
    if (addrInfoPtr != TM_ADDRINFO_NULL_PTR)
    {

/*     2.1. Zero out the addrinfo and sockaddr_storage structures. */
        tm_bzero(addrInfoPtr, allocSize);

/*
 *     2.2. Set the internal sockaddr pointer to point at the newly allocated
 *          sockaddr_storage structure, which immediately follows the addrinfo
 *          structure.
 */
        addrInfoPtr->ai_addr = (struct sockaddr TM_FAR *) (addrInfoPtr + 1);
    }

    return addrInfoPtr;
}


#ifdef TM_DNS_USE_SRV
/*
 * tfDnsAllocAddrInfoExt Function Description
 *
 * Allocates and zeros out a new ttAddrInfoExt structure, including the
 * attached sockaddr_storage structure.  This is used internally by the
 * DNS resolver.
 *
 * 1. Allocate a single raw buffer large enough to hold the addrinfo and
 *    sockaddr_storage structures.  This is allocated as a single buffer to
 *    conserve memory and so that it may be freed as a single buffer as well.
 * 2. If the allocation was successful
 *     2.1. Zero out the addrinfo and sockaddr_storage structures.
 *     2.2. Set the internal sockaddr pointer to point at the newly allocated
 *          sockaddr_storage structure, which immediately follows the addrinfo
 *          structure.
 *
 * Parameters
 * None
 *
 * Returns
 * A pointer to the new structure if successful, NULL if the allocation fails.
 */

static ttAddrInfoExtPtr tfDnsAllocAddrInfoExt(void)
{
    ttAddrInfoExtPtr addrInfoPtr;
    int              allocSize;

/*
 * 1. Allocate a single raw buffer large enough to hold the ttAddrInfoExt and
 *    sockaddr_storage structures.  This is allocated as a single buffer to
 *    conserve memory and so that it may be freed as a single buffer as well.
 */
    allocSize = sizeof(ttAddrInfoExt) + sizeof(struct sockaddr_storage);
    addrInfoPtr = (ttAddrInfoExtPtr)tm_get_raw_buffer(allocSize);

/* 2. If the allocation was successful */
    if (addrInfoPtr != TM_ADDRINFO_EXT_NULL_PTR)
    {

/*     2.1. Zero out the addrinfo and sockaddr_storage structures. */
        tm_bzero(addrInfoPtr, allocSize);

/*
 *     2.2. Set the internal sockaddr pointer to point at the newly allocated
 *          sockaddr_storage structure, which immediately follows the
 *          ttAddrInfoExt structure.
 */
        addrInfoPtr->aie_addr = (struct sockaddr TM_FAR *) (addrInfoPtr + 1);
    }

    return addrInfoPtr;
}


/*
 * tfDnsFreeAddrInfoExt Function Description
 *
 * Free all memory associated with a chain of ttAddrInfoExt structures
 *
 * 1. Loop through each ttAddrInfoExt in the chain
 * 2. Free any associated addrinfo chains
 * 3. Free the ttAddrInfoExt chain
 *
 * Parameters
 * Name          Description
 * startAddrInfo The start of the ttAddrInfoExt chain to be free'd
 *
 * Returns
 * None
 */
static void tfDnsFreeAddrInfoExt(ttAddrInfoExtPtr startAddrInfo)
{
    ttAddrInfoExtPtr curAddrInfo;

    curAddrInfo = startAddrInfo;

/* 1. Loop through each ttAddrInfoExt in the chain */
    while (curAddrInfo)
    {
/* 2. Free any associated addrinfo chains */
#ifdef TM_USE_IPV4
        if (curAddrInfo->aie_ip4addrs)
        {
            freeaddrinfo(curAddrInfo->aie_ip4addrs);
        }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
        if (curAddrInfo->aie_ip6addrs)
        {
            freeaddrinfo(curAddrInfo->aie_ip6addrs);
        }
#endif /* TM_USE_IPV6 */
        curAddrInfo = (ttAddrInfoExtPtr)curAddrInfo->aie_next;
    }
/* 3. Free the ttAddrInfoExt chain */
    freeaddrinfo((struct addrinfo TM_FAR *)startAddrInfo);
}


/*
 * tfDnsSrvDeleteWeakest Function Description
 *
 * Delete the 'weakest' SRV host in a cache entry. The weakest host is defined
 * as the host with the lowest priority and lowest weight. Lowest priority is
 * the highest priority value. Lowest weight is the lowest weight value.
 *
 * 1. Loop through each ttAddrInfoExt in the chain
 * 2. Free any associated addrinfo chains
 * 3. Free the ttAddrInfoExt chain
 *
 * Parameters
 * Name          Description
 * startAddrInfo The start of the ttAddrInfoExt chain to be free'd
 *
 * Returns
 * None
 */
static void tfDnsSrvDeleteWeakest(ttDnsCacheEntryPtr entryPtr)
{
    ttAddrInfoExtPtr weakestSrvInfoPtr;
    ttAddrInfoExtPtr weakestPrevSrvInfoPtr;
    ttAddrInfoExtPtr curSrvInfoPtr;
    ttAddrInfoExtPtr prevSrvInfoPtr;
    tt16Bit          lowestPri;
    tt16Bit          lowestWeight;
    tt16Bit          currentPri;
    tt16Bit          currentWeight;

    weakestSrvInfoPtr = (ttAddrInfoExtPtr)0;
    weakestPrevSrvInfoPtr = (ttAddrInfoExtPtr)0;
    prevSrvInfoPtr = (ttAddrInfoExtPtr)0;
    curSrvInfoPtr = (ttAddrInfoExtPtr)entryPtr->dnscAddrInfoPtr;

    lowestPri = 0;
    lowestWeight = 0xFFFF;

/* 1. Loop through all of the hosts */
    while (curSrvInfoPtr)
    {
        currentPri = (tt16Bit)curSrvInfoPtr->aie_srvpri;
        currentWeight = (tt16Bit)curSrvInfoPtr->aie_srvweight;

/* If we have a host of weight 0, remember it */
        if (   (currentPri > lowestPri)
            || (   (currentPri == lowestPri)
                && (currentWeight <= lowestWeight)))
        {
            weakestSrvInfoPtr = curSrvInfoPtr;
            weakestPrevSrvInfoPtr = prevSrvInfoPtr;
            lowestPri = currentPri;
            lowestWeight = currentWeight;
        }
        if (!curSrvInfoPtr->aie_next)
        {
/* We're at the last SRV host, so remove the weakest one we found */
            if (weakestPrevSrvInfoPtr)
            {
                weakestPrevSrvInfoPtr->aie_next =
                        weakestSrvInfoPtr->aie_next;
            }
            else
            {
                entryPtr->dnscAddrInfoPtr =
                        weakestSrvInfoPtr->aie_next;
            }
            weakestSrvInfoPtr->aie_next = TM_ADDRINFO_NULL_PTR;
            tfDnsFreeAddrInfoExt(weakestSrvInfoPtr);
            break;
        }
        prevSrvInfoPtr = curSrvInfoPtr;
        curSrvInfoPtr = (ttAddrInfoExtPtr)curSrvInfoPtr->aie_next;
    }
    entryPtr->dnscSrvHostCount--;
}
#endif /* TM_DNS_USE_SRV */

/*
 * freeaddrinfo Function Description
 * Frees memory used by address information structure chain returned by
 * getaddrinfo.
 *
 * Parameters
 * Parameter    Description
 * addrInfoPtr  Pointer to first address info structure to be freed.
 *
 * Returns
 * None
 *
 * 1. Walk through the chain of addrinfo structures given by the user:
 *     1.1. If allocated, free canonical name string in first addrinfo.
 *     1.2. Get pointer to next addrinfo in chain.
 *     1.3. This points to a raw buffer containing both an addrinfo and a
 *          sockaddr_storage structure, so freeing this buffer frees both
 *          the addrinfo as well as the attached sockaddr_storage.
 */
void freeaddrinfo(struct addrinfo TM_FAR * addrInfoPtr)
{
    struct addrinfo TM_FAR * nextAddrInfoPtr;

/* 1. Walk through the chain of addrinfo structures given by the user: */
    while (addrInfoPtr != TM_ADDRINFO_NULL_PTR)
    {

/* 1.1. If allocated, free canonical name string */
        if (addrInfoPtr->ai_canonname != (char TM_FAR *) 0)
        {
            tm_free_raw_buffer((ttRawBufferPtr) addrInfoPtr->ai_canonname);
        }

/* 1.2. Get pointer to next addrinfo in chain. */
        nextAddrInfoPtr = addrInfoPtr->ai_next;

/*
 *     1.3. This points to a raw buffer containing both an addrinfo and a
 *          sockaddr_storage structure, so freeing this buffer frees both
 *          the addrinfo as well as the attached sockaddr_storage.
 */
        tm_free_raw_buffer((ttRawBufferPtr) addrInfoPtr);

        addrInfoPtr = nextAddrInfoPtr;
    }
}


/*
 * getaddrinfo Function Description
 *
 * Translates a node-name (hostname) to an address.  Similar to gethostbyname,
 * used for both IPv4 and IPv6.  Retrieving a service name or port number is
 * not supported in our implementation of getaddrinfo, so serviceName must be
 * set to NULL.
 *
 * Parameters
 * Parameter    Description
 * nodeName     String containing nodename to translate.
 * serviceName  Not currently used.
 * hintsPtr     Pointer to structure indicating which types of addresses the
 *              caller is interested in.
 * resPtrPtr    Set to a pointer to one or more addrinfo structures containing
 *              address information about the specified nodename.
 *
 * Returns
 * Value          Meaning
 * TM_EOPNOTSUPP  Specifying a service name is not supported.
 * TM_EPERM       tfDnsInit() has not yet been called.
 * EAI_NONAME     AI_NUMERICHOST was specified but nodeName was not a valid
 *                numeric address for the given family type.
 * EAI_BADFLAGS   Invalid flags specified in hint structure.
 * EAI_FAMILY     Invalid protocol family (not PF_INET,PF_INET6 or PF_UNSPEC)
 *                in hint structure.
 * EAI_NODATA     No addresses (of the given address family) associated with
 *                the nodename.
 * EAI_MEMORY     Not enough memory to allocate addrinfo structure(s) to be
 *                returned.
 *
 * 1. ERROR CHECKING:
 *      1.1. If resPtrPtr is NULL, return TM_EINVAL.
 *      1.2. If nodeName and serviceName are NULL, return EAI_NONAME.
 * 2. Validate the contents of the hints structure, if provided:
 *     2.1. ai_family must be either AF_UNSPEC, AF_INET or AF_INET6.  If
 *          ai_family is an unrecognized value or if the specified value is
 *          not supported, return EAI_FAMILY.
 *     2.2. ai_flags may include AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST,
 *          AI_NUMERICSERV, AI_ALL, AI_V4MAPPED and AI_ADDRCONFIG.
 *          If an unrecognized flag is included, return EAI_BADFLAGS.
 *     2.3. AI_NUMERICHOST flag requires a non-NULL nodeName.
 *          AI_NUMERICSERV flag requires a non-NULL serviceName.
 *     2.4. EAI_SOCKTYPE error if any of the following conditions fail:
 *          a. ai_socktype must be SOCK_STREAM, SOCK_DGRAM, SOCK_RAW or 0,
 *          b. ai_protocol must be IPPROTO_TCP or 0, if ai_socktype is
 *             SOCK_STREAM,
 *          c. ai_protocol must be IPPROTO_UDP or 0, if ai_socktype is
 *             SOCK_DGRAM.
 * 3. If a hint structure was not provided, use the defaults:
 *     3.1. The default flags should return IPv4 addresses only if no IPv6
 *          addresses were found, and should only return IPv4 addresses if
 *          IPv4 is configured and only return IPv6 addresses if IPv6 is
 *          configured.
 *     3.2. By default, accept any family, socket type or protocol.
 * 4. If necessary, determine which address families are configured on the
 *    local system.
 *     4.1. Assume that neither IPv4 nor IPv6 is configured, until we find a
 *          configured device.
 *     4.2. Lock the device list, and loop through the configured devices,
 *          ignoring the loopback interface.
 *     4.3. IPv6 is considered to be configured on a device if a link local
 *          address is configured.
 *          Exit loop if further search would be pointless.
 *     4.4. IPv4 is considered to be configured on a device if an IPv4 address
 *          is configured.
 *          Exit loop if further search would be pointless.
 * 5. Try numeric address resolution
 *     5.1. If nodeName is not provided, use a local address.
 *         5.1.1. If AI_PASSIVE flag is set, use "any" address for use
 *                with bind().
 *                Otherwise, use loopback address for use with
 *                connect() or sendto().
 *     5.2. If nodeName is provided, try to resolve it numerically.
 *         5.2.1. A scope identifier may be present at the end of the
 *                string following a "%" character. Save the scope id
 *                and remove it from the string before calling inet_pton.
 *         5.2.2. Try to resolve the address with inet_pton.
 *                If successful, Set a flag that we can check later.
 *                If unsuccessful and AI_NUMERICHOST is set, return
 *                EAI_NONAME.
 * 6. Try numeric service resolution
 *     6.1. If serviceName is provided, try and resolve it.
 *          Currently, only numeric values are supported.
 *          ServiceName is ignored for raw socket types.
 *     6.2. Try to convert the string to an unsigned short.
 *          If a non-digit character is found before the end of the string,
 *          then it is not a valid service name.
 *          If the serviceName is not valid and AI_NUMERICSERV is set, return
 *          EAI_NONAME.
 * 7. If we find a name, we will set the error code accordingly.
 *    Otherwise, error = EAI_NONAME.
 * 8. Try resolving AF_INET6 or AF_UNSPEC family as IPv6 address unless
 *    configuration precludes it.
 *     8.1. IPv6 addresses are excluded. Undo any previously resolved
 *          address.
 *     8.2. IPv6 addresses are acceptable.
 *         a. Skip the DNS query if the host name is a valid
 *            numerical address.
 *         b. If the host name is a valid IPv6 numerical address, indicate
 *            that the name was found.
 *     8.3. The host name is not a valid numerical address and the user
 *          wants IPv6 addresses.
 *          Try to resolve the name by DNS query, type AAAA.
 *     8.3.1. When operating in non-blocking mode, tfDnsGenerateQuery could
 *            return before finished.  Also, we may have experienced a
 *            problem contacting the name server, so there's no need to send
 *            an IPv4 request below.
 *            In these case, return immediately to the user. However, if we
 *            received a response from the name server that included an
 *            error, keep going, because this error may just indicate that
 *            the server can't handle IPv6 (TM_DNS_EANSWER, for instance).
 *     8.3.2. If tfDnsGenerateQuery returned successfully, there may still
 *            have been an error (for example, no answer to our record query)
 *            so get the error from the cache entry.
 *     8.3.3. Note: Don't copy the information out of the cache entry yet,
 *            since an IPv4 query may still need to be done below.  It's
 *            necessary to copy all information (both IPv4 and IPv6) at once,
 *            since the IPv4 query may not complete immediately, and in
 *            non-blocking mode we would return to the user.
 * 9. Try resolving as IPv4 address unless configuration precludes it.
 *     9.1. For AF_INET or AF_UNSPEC family, or
 *     9.2. For AF_INET6 family if AI_V4MAPPED selected, and either
 *             a. IPv6 query failed, or
 *             b. AI_ALL selected (get both IPv6 and IPv4 results).
 *     9.3. IPv4 addresses are excluded. Undo any previously resolved
 *          address.
 *     9.4. IPv4 addresses are acceptable.
 *         a. Skip the DNS query if the host name is a valid
 *            numerical address.
 *         b. If the host name is a valid IPv4 numerical address, indicate
 *            that the name was found.
 *     9.5. The host name is not a valid numerical address and the user
 *          wants IPv4 addresses.
 *          Try to resolve the name by DNS query, type A.
 *     9.6. add tvDnsCachesize by 1 in order to store v4Entry
 *     9.7. If tfDnsGenerateQuery returned successfully, there may still have
 *          been an error (for example, no answer to our record query) so get
 *          the error from the cache entry.
 *     9.8. restore the tvDnsCachesize by descresing 1.
 * 10. If no error has occurred yet, all of the relevant information
 *     is contained in the address structures or DNS cache entries.
 *     Allocate a series of addrinfo structures which will be returned to
 *     the user, and copy the relevant information to the new addrinfo's.
 * 11. If a IPv6 DNS cache entry exists, get the pointer to the address
 *     information so we can traverse the list and copy the information.
 * 12. For all valid addresses, numerically derived or queried.
 *   12.1. Allocate a new addrinfo entry.
 *   12.2. If memory allocation fails, first, clean up any previously
 *         allocated addrinfo structures, then return EAI_MEMORY to the user.
 *   12.3. Copy information into new addrinfo structure.
 *         For raw sockets, the protocol is provided by the user's
 *         protocol hint.
 *   12.4. If the DNS server returned a local scope address, set
 *         sin6_scope_id based on the incoming interface that the response
 *         was received on. (R2.5:60)
 *   12.5. Link new addrinfo to previous one in the chain.
 *   12.6. Move to next address in this cache entry.
 * 13. If a IPv4 DNS cache entry exists, get the pointer to the address
 *     information so we can traverse the list and copy the information.
 *     If the protocol type is IPv6, the addresses are returned as
 *     IPv4-mapped IPv6 addresses.
 * 14. For all valid addresses, numerically derived or queried.
 *   14.1. Allocate a new addrinfo entry.
 *   14.2. If memory allocation fails, first, clean up any previously
 *         allocated addrinfo structures, then return EAI_MEMORY to the user.
 *   14.3. Copy information into new addrinfo structure.
 *         For raw sockets, the protocol is provided by the user's
 *         protocol hint.
 *   14.4. If the user specified an address family of IPv6, addresses
 *         will be copied as IPv4-mapped IPv6 addresses.
 *   14.5. If the user specified an address family of IPv4, addresses
 *         will be presented in traditional IPv4 format.
 *   14.6. Link new addrinfo to previous one in the chain.
 *   14.7. Move to next address in this cache entry.
 * 15. If no addresses have been collected by this point, return EAI_NONAME.
 * 16. If the AI_CANONFLAG was set, copy the canonical name for this host into
 *     the first addrinfo structure being returned to the user.  The canonical
 *     name should be present on both the IPv6 as well as the IPv4 cache
 *     entries, but this isn't guaranteed (depends on the DNS server).  We
 *     check the IPv4 entry first and if it's not present, we check the IPv6
 *     entry.
 *   16.1. Allocate a string buffer to hold the canonical name.  Copy the
 *         canonical name into this buffer and attach it to the addrinfo
 *         structure.
 * 17. Indicate that the user has retrieved the information from the cache
 *     entries.  If retrieving both IPv6 and IPv4 addresses, it is important
 *     to do this only after both entries have been retrieved.  This prevents
 *     one entry from disappearing while waiting for the other to complete.
 * 18. Sort the addrinfo structures according to RFC3484
 *   18.1 Check whether the interface is proper
 *   18.2 Check whether the device entry pointer has been previously entered
 *   18.2.1 Sort the list of destination addresses by calling the function
               tf6SelectDestinationAddress()
 * 19. Expand the list of addresses to include all supported socket types and
 *     protocols that the user requested.
 *   19.1. For each valid socket type and associated protocol.
 *   19.1.1. Skip socket types and protocols that the user does not want.
 *   19.1.2. Allocate a new addrinfo entry if we are not updating an existing
 *           entry.
 *   19.1.2.1. If memory allocation fails, first, clean up any previously
 *             allocated addrinfo structures, then return EAI_MEMORY to the
 *             user.
 *   19.1.2.2. Duplicate the address and other information from the previous
 *             entry.
 *   19.1.2.3. Insert the new entry in the list after the current entry
 *             (pointed to by userAddrInfoPtr).
 *             Set userAddrInfoPtr to reflect the new "current" entry.
 *   19.1.3. Update the socket type and protocol data for this entry.
 *           For raw sockets, the protocol is provided by the user's
 *           protocol hint.
 *   19.1.4. Indicate that a new allocation will be needed next time
 *           around the loop.
 * 20. If no records for this protocol type were found, return EAI_NODATA.
 */
int getaddrinfo(const char TM_FAR *               nodeName,
                const char TM_FAR *               serviceName,
                const struct addrinfo TM_FAR *    hintsPtr,
                struct addrinfo TM_FAR * TM_FAR * resPtrPtr)
{
    struct addrinfo TM_FAR *        newAddrInfoPtr;
    struct addrinfo TM_FAR *        userAddrInfoPtr;
    struct addrinfo TM_FAR *        cacheAddrInfoPtr;
    ttSockAddrPtrUnion              sockAddr;
    ttSockAddrPtrUnion              cacheSockAddr;
    ttDeviceEntryPtr                listdevEntryPtr;
    ttCharPtr                       canonNamePtr;
    ttCharPtr                       stringEndPtr;
    ttConstIntPtr                   sockTypePtr;
#ifdef TM_USE_RFC3484
    struct addrinfo TM_FAR *        addrInfoPtr;
    ttDnsListEntryPtr               dnsListArrayPtr;
#endif /* TM_USE_RFC3484 */
#ifdef TM_USE_IPV6
    ttDnsCacheEntryPtr              v6EntryPtr;
    struct sockaddr_in6 TM_FAR *    ip6addrPtr;
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
    ttDnsCacheEntryPtr              v4EntryPtr;
    struct sockaddr_in TM_FAR *     ip4addrPtr;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    tt6IpAddress                    ip6Address;
    char                            ip6AddrStrBuf[INET6_ADDRSTRLEN];
    ttConstCharPtr                  ip6AddrPtr;
    ttCharPtr                       ip6ScopePtr;
    unsigned long                   scopeId;
    unsigned long                   scopeOffset;
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_RFC3484
    unsigned int                    localListCount;
    unsigned int                    j;
    unsigned int                    allocSize;
    int                             qsortRetVal;
#endif /* TM_USE_RFC3484 */
    int                             errorCode;
    int                             errCode;
    int                             hintsFamily;
    int                             hintsFlags;
    int                             hintsSockType;
    int                             hintsProtocol;
    int                             canonNameLen;
    unsigned short                  servicePort;
#ifdef TM_USE_IPV4
#ifndef TM_SINGLE_INTERFACE_HOME
    int                             i;
#endif /* !TM_SINGLE_INTERFACE_HOME */
    tt4IpAddress                    ip4Address;
    tt8Bit                          ipv4ConfigFlag;
    tt8Bit                          ip4AddressValid;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    tt8Bit                          ipv6ConfigFlag;
    tt8Bit                          ip6AddressValid;
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_RFC3484
    tt8Bit                          sortTestNum;
#endif /* TM_USE_RFC3484 */
    tt8Bit                          ipAddressValid;

    errorCode = TM_ENOERROR;

/* Default values */
/* Contents of variables ip4Address and ip6Address are not to be returned */
    ipAddressValid = TM_8BIT_NO;
    servicePort = 0;
#ifdef TM_USE_IPV4
/* Local IPv4 address suitable for use with connect() or sendto() */
    tm_ip_copy(TM_IP_LOOP_BACK_ADDR, ip4Address);
/* IPv4 addresses are configured and must be returned */
    ipv4ConfigFlag = TM_8BIT_YES;
/* Contents of variable ip4Address are not to be returned */
    ip4AddressValid = TM_8BIT_NO;
/* No IPv4 DNS cache entry */
    v4EntryPtr = (ttDnsCacheEntryPtr) 0;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
/* Local IPv6 address suitable for use with connect() or sendto() */
    tm_6_ip_copy_structs(in6addr_loopback, ip6Address);
/* IPv6 addresses are configured and must be returned */
    ipv6ConfigFlag = TM_8BIT_YES;
/* Contents of variable ip6Address are not to be returned */
    ip6AddressValid = TM_8BIT_NO;
    scopeId = 0;
/* No IPv6 DNS cache entry */
    v6EntryPtr = (ttDnsCacheEntryPtr) 0;
#endif /* TM_USE_IPV6 */

/* 0. Lock the DNS global variables */
    tm_call_lock_wait(&tm_context(tvDnsLockEntry));

/* 1. ERROR CHECKING: */
/*      1.1. If resPtrPtr is NULL, return TM_EINVAL. */
    if (resPtrPtr == (struct addrinfo TM_FAR * TM_FAR *)0)
    {
        errorCode = TM_EINVAL;
        goto getaddrinfoExit;
    }
/*      1.2. If nodeName and serviceName are NULL, return EAI_NONAME. */
    if (    (nodeName == (ttConstCharPtr)0)
         && (serviceName == (ttConstCharPtr)0) )
    {
        errorCode = EAI_NONAME;
        goto getaddrinfoExit;
    }

/* initilize the *resPtrPtr */
    *resPtrPtr = TM_ADDRINFO_NULL_PTR;

/* 2. Validate the contents of the hints structure, if provided: */
    if (hintsPtr != (const struct addrinfo TM_FAR *)0)
    {
        hintsFlags  = hintsPtr->ai_flags;
        hintsFamily = hintsPtr->ai_family;
        hintsSockType = hintsPtr->ai_socktype;
        hintsProtocol = hintsPtr->ai_protocol;

/*
 *     2.1. ai_family must be either AF_UNSPEC, AF_INET or AF_INET6.  If
 *          ai_family is an unrecognized value or if the specified value is
 *          not supported, return EAI_FAMILY.
 */
        if (    (hintsFamily != AF_UNSPEC)
#ifdef TM_USE_IPV4
             && (hintsFamily != AF_INET)
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
             && (hintsFamily != AF_INET6)
#endif /* TM_USE_IPV6 */
            )
        {
            errorCode = EAI_FAMILY;
            goto getaddrinfoExit;
        }

/*
 *     2.2. ai_flags may include AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST,
 *          AI_NUMERICSERV, AI_ALL, AI_V4MAPPED and AI_ADDRCONFIG.
 *          If an unrecognized flag is included, return EAI_BADFLAGS.
 */
        if (hintsFlags & ~TM_GETADDRINFO_FLAGS)
        {
            errorCode = EAI_BADFLAGS;
            goto getaddrinfoExit;
        }
/*
 *     2.3. AI_NUMERICHOST flag requires a non-NULL nodeName.
 *          AI_NUMERICSERV flag requires a non-NULL serviceName.
 */
        if (    (   (hintsFlags & AI_NUMERICHOST) != 0
                &&  nodeName == (ttConstCharPtr)0 )
            ||  (    (hintsFlags & AI_NUMERICSERV) != 0
                &&  serviceName == (ttConstCharPtr)0 ) )
        {
            errorCode = EAI_NONAME;
            goto getaddrinfoExit;
        }
/*
 *     2.4. EAI_SOCKTYPE error if any of the following conditions fail:
 *          a. ai_socktype must be SOCK_STREAM, SOCK_DGRAM, SOCK_RAW or 0,
 *          b. ai_protocol must be IPPROTO_TCP or 0, if ai_socktype is
 *             SOCK_STREAM,
 *          c. ai_protocol must be IPPROTO_UDP or 0, if ai_socktype is
 *             SOCK_DGRAM.
 */
        if (    hintsSockType != 0
#if defined(TM_USE_RAW_SOCKET) || defined(TM_6_USE_RAW_SOCKET)
            &&  hintsSockType != SOCK_RAW
#endif /* TM_USE_RAW_SOCKET || TM_6_USE_RAW_SOCKET */
#ifdef TM_USE_TCP
            &&  (   hintsSockType != SOCK_STREAM
                ||  (   hintsProtocol != IPPROTO_TCP
                    &&  hintsProtocol != 0 ) )
#endif /* TM_USE_TCP */
            &&  (   hintsSockType != SOCK_DGRAM
                ||  (   hintsProtocol != IPPROTO_UDP
                    &&  hintsProtocol != 0 ) ) )
        {
            errorCode = EAI_SOCKTYPE;
            goto getaddrinfoExit;
        }
    }
    else
    {
/*
 * 3. If a hint structure was not provided, use the defaults:
 */
/*
 *     3.1. The default flags should return IPv4 addresses only if no IPv6
 *          addresses were found, and should only return IPv4 addresses if
 *          IPv4 is configured and only return IPv6 addresses if IPv6 is
 *          configured.
 */
        hintsFlags = TM_GETADDRINFO_DEF_FLAGS;
/*
 *     3.2. By default, accept any family, socket type or protocol.
 */
        hintsFamily = AF_UNSPEC;
        hintsSockType = 0;
        hintsProtocol = 0;
    }

/*
 * 4. If necessary, determine which address families are configured on the
 *    local system.
 */
    if (hintsFlags & AI_ADDRCONFIG)
    {
/*
 *     4.1. Assume that neither IPv4 nor IPv6 is configured, until we find a
 *          configured device.
 */
#ifdef TM_USE_IPV4
        ipv4ConfigFlag = TM_8BIT_ZERO;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
        ipv6ConfigFlag = TM_8BIT_ZERO;
#endif /* TM_USE_IPV6 */

/*
 *     4.2. Lock the device list, and loop through the configured devices,
 *          ignoring the loopback interface.
 */
        tm_call_lock_wait(&tm_context(tvDeviceListLock));
        listdevEntryPtr = tm_context(tvDeviceList);
        while (listdevEntryPtr != TM_DEV_NULL_PTR)
        {

            if (listdevEntryPtr != tm_context(tvLoopbackDevPtr))
            {
/*
 *     4.3. IPv6 is considered to be configured on a device if a link local
 *          address is configured.
 *          Exit loop if further search would be pointless.
 */
#ifdef TM_USE_IPV6
                if (!ipv6ConfigFlag && tm_6_dev_is_enabled(listdevEntryPtr))
                {
                    ipv6ConfigFlag = TM_8BIT_YES;
#ifdef TM_USE_IPV4
                    if (ipv4ConfigFlag)
#endif /* TM_USE_IPV4 */
                    {
                        break;
                    }
                }
#endif /* TM_USE_IPV6 */

/*
 *     4.4. IPv4 is considered to be configured on a device if an IPv4 address
 *          is configured.
 *          Exit loop if further search would be pointless.
 */
#ifdef TM_USE_IPV4
                if (!ipv4ConfigFlag)
                {
#ifdef TM_SINGLE_INTERFACE_HOME
                    if (tm_ip_dev_conf_flag(listdevEntryPtr, 0))
                    {
                        ipv4ConfigFlag = TM_8BIT_YES;
                    }
#else /* !TM_SINGLE_INTERFACE_HOME */
                    for ( i = 0;
                          i < (int)listdevEntryPtr->devMhomeUsedEntries;
                          i++ )
                    {
                        if (tm_8bit_one_bit_set(
                                    tm_ip_dev_conf_flag(listdevEntryPtr, i),
                                    TM_DEV_IP_CONFIG) )
                        {
                            ipv4ConfigFlag = TM_8BIT_YES;
                            break;
                        }
                    }
#endif /* !TM_SINGLE_INTERFACE_HOME */
                    if (    ipv4ConfigFlag
#ifdef TM_USE_IPV6
                        &&  ipv6ConfigFlag
#endif /* TM_USE_IPV6 */
                        )
                    {
                        break;
                    }
                }
#endif /* TM_USE_IPV4 */

            }

            listdevEntryPtr = listdevEntryPtr->devNextDeviceEntry;

        }
        tm_call_unlock(&tm_context(tvDeviceListLock));

    }

/*
 * 5. Try numeric address resolution
 */
/*
 *      5.1. If nodeName is not provided, use a local address.
 */
    if (nodeName == (ttConstCharPtr)0)
    {
/*
 *          5.1.1. If AI_PASSIVE flag is set, use "any" address for use
 *                 with bind().
 *                 Otherwise, use loopback address for use with
 *                 connect() or sendto().
 */
        if (hintsFlags & AI_PASSIVE)
        {
#ifdef TM_USE_IPV6
            tm_6_ip_copy_structs(in6addr_any, ip6Address);
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
            tm_ip_copy(TM_IP_ZERO, ip4Address);
#endif /* TM_USE_IPV4 */
        }
#ifdef TM_USE_IPV6
        ip6AddressValid = TM_8BIT_YES;
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
        ip4AddressValid = TM_8BIT_YES;
#endif /* TM_USE_IPV4 */
        ipAddressValid = TM_8BIT_YES;
    }
/*
 *      5.2. If nodeName is provided, try to resolve it numerically.
 */
    else
    {
#ifdef TM_USE_IPV6
/*
 *          5.2.1. A scope identifier may be present at the end of the
 *                 string following a "%" character. Save the scope id
 *                 and remove it from the string before calling inet_pton.
 */
        ip6AddrPtr = nodeName;
        ip6ScopePtr = tm_strrchr(nodeName, '%');
        if (ip6ScopePtr != (ttConstCharPtr)0)
        {
/* Scope suffix character found. We need to copy the address portion of
 * the string (preceding the "%") to a separate buffer for numerical
 * analysis. Make sure it doesn't overflow the buffer. */
            scopeOffset = (unsigned long)(ip6ScopePtr - nodeName);
            if (scopeOffset + 1 < (unsigned long)sizeof(ip6AddrStrBuf))
            {
                tm_memcpy(ip6AddrStrBuf, nodeName, scopeOffset);
                ip6AddrStrBuf[scopeOffset] = '\0';
                ip6AddrPtr = ip6AddrStrBuf;
/* If there are more than 0 characters to the right of the "%" then we can
 * try to resolve the scope identifier.
 * First, try to convert the scope id to an unsigned long.
 * If a non-digit character is found before the end of the string, treat the
 * string as an interface name. */
                ++ip6ScopePtr;
                errCode = (int)tm_strlen(ip6ScopePtr);
                if (errCode > 0)
                {
                    scopeId = tm_strtoul(ip6ScopePtr, &stringEndPtr, 10);
                    if (stringEndPtr != (ttCharPtr)0 && *stringEndPtr != '\0')
                    {
                        scopeId = if_nametoindex(ip6ScopePtr);
                    }
                }
            }
        }
/*
 *          5.2.2. Try to resolve the address with inet_pton.
 *                 If successful, Set a flag that we can check later.
 *                 If unsuccessful and AI_NUMERICHOST is set, return
 *                 EAI_NONAME.
 */
        errCode = inet_pton(AF_INET6, ip6AddrPtr, &ip6Address);
        if (errCode == 1)
        {
            ip6AddressValid = TM_8BIT_YES;
            ipAddressValid = TM_8BIT_YES;
        }
        else
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
        errCode = inet_pton(AF_INET, nodeName, &ip4Address);
        if (errCode == 1)
        {
            ip4AddressValid = TM_8BIT_YES;
            ipAddressValid = TM_8BIT_YES;
        }
        else
#endif /* TM_USE_IPV4 */
        if (hintsFlags & AI_NUMERICHOST)
        {
            errorCode = EAI_NONAME;
            goto getaddrinfoExit;
        }
    }

/*
 * 6. Try numeric service resolution
 */
/*
 *      6.1. If serviceName is provided, try and resolve it.
 *           Currently, only numeric values are supported.
 *           ServiceName is ignored for raw socket types.
 */
    if (    serviceName != (ttConstCharPtr)0
#if defined(TM_USE_RAW_SOCKET) || defined(TM_6_USE_RAW_SOCKET)
        &&  hintsSockType != SOCK_RAW
#endif /* TM_USE_RAW_SOCKET || TM_6_USE_RAW_SOCKET */
        )
    {
/*
 *      6.2. Try to convert the string to an unsigned short.
 *           If a non-digit character is found before the end of the string,
 *           then it is not a valid service name.
 *           If the serviceName is not valid and AI_NUMERICSERV is set, return
 *           EAI_NONAME.
 */
        servicePort = (unsigned short)tm_strtoul(   serviceName,
                                                    &stringEndPtr,
                                                    10 );
        if (stringEndPtr != (ttCharPtr)0 && *stringEndPtr == '\0')
        {
            servicePort = ntohs(servicePort);
        }
        else if (hintsFlags & AI_NUMERICSERV)
        {
            errorCode = EAI_NONAME;
            goto getaddrinfoExit;
        }
        else
        {
            servicePort = 0;
        }
    }

/*
 * 7. If we find a name, we will set the error code accordingly.
 *    Otherwise, ...
 */
    errorCode = EAI_NONAME;

/*
 * 8. Try resolving AF_INET6 or AF_UNSPEC family as IPv6 address unless
 *    configuration precludes it.
 */
#ifdef TM_USE_IPV6
    if (    !(  (hintsFamily == AF_INET6 || hintsFamily == AF_UNSPEC)
            &&  ipv6ConfigFlag ) )
    {
/*
 *      8.1. IPv6 addresses are excluded. Undo any previously resolved
 *           address.
 */
        ip6AddressValid = TM_8BIT_NO;
    }
/*
 *      8.2. IPv6 addresses are acceptable.
 *          a. Skip the DNS query if the host name is a valid
 *             numerical address.
 *          b. If the host name is a valid IPv6 numerical address, indicate
 *             that the name was found.
 */
    else if (ipAddressValid)
    {
        if (ip6AddressValid)
        {
            errorCode = TM_ENOERROR;
        }
    }
    else
    {
/*
 *      8.3. The host name is not a valid numerical address and the user
 *           wants IPv6 addresses.
 *           Try to resolve the name by DNS query, type AAAA.
 */
        errorCode = tfDnsGenerateQuery( nodeName,
                                        TM_DNS_QTYPE_AAAA,
                                        AF_INET6,
                                        &v6EntryPtr );

/*
 *      8.3.1. When operating in non-blocking mode, tfDnsGenerateQuery could
 *             return before finished.  Also, we may have experienced a
 *             problem contacting the name server, so there's no need to send
 *             an IPv4 request below.
 *             In these case, return immediately to the user. However, if we
 *             received a response from the name server that included an
 *             error, keep going, because this error may just indicate that
 *             the server can't handle IPv6 (TM_DNS_EANSWER, for instance).
 */
        if ((errorCode != TM_ENOERROR) && (errorCode < TM_DNS_ERROR_BASE))
        {
            goto getaddrinfoExit;
        }

/*
 *      8.3.2. If tfDnsGenerateQuery returned successfully, there may still
 *             have been an error (for example, no answer to our record query)
 *             so get the error from the cache entry.
 */
        if (v6EntryPtr != (ttDnsCacheEntryPtr)0)
        {
            errorCode = v6EntryPtr->dnscErrorCode;
        }

/*
 *      8.3.3. Note: Don't copy the information out of the cache entry yet,
 *             since an IPv4 query may still need to be done below.  It's
 *             necessary to copy all information (both IPv4 and IPv6) at once,
 *             since the IPv4 query may not complete immediately, and in
 *             non-blocking mode we would return to the user.
 */
    }
#endif /* TM_USE_IPV6 */

/*
 * 9. Try resolving as IPv4 address unless configuration precludes it.
 */
#ifdef TM_USE_IPV4
/*      9.1. For AF_INET or AF_UNSPEC family */
    if (    !(  (   hintsFamily == AF_INET
                ||  hintsFamily == AF_UNSPEC
#ifdef TM_USE_IPV6
/*      9.2. For AF_INET6 family if AI_V4MAPPED selected, and either
 *              a. IPv6 query failed, or
 *              b. AI_ALL selected (get both IPv6 and IPv4 results). */
                ||  (   (hintsFlags & AI_V4MAPPED) != 0
                    &&  (   errorCode != TM_ENOERROR
                        ||  (hintsFlags & AI_ALL) != 0 ) )
#endif /* TM_USE_IPV6 */
                )
            &&  ipv4ConfigFlag ) )
    {
/*
 *      9.3. IPv4 addresses are excluded. Undo any previously resolved
 *           address.
 */
        ip4AddressValid = TM_8BIT_NO;
    }
/*
 *      9.4. IPv4 addresses are acceptable.
 *          a. Skip the DNS query if the host name is a valid
 *             numerical address.
 *          b. If the host name is a valid IPv4 numerical address, indicate
 *             that the name was found.
 */
    else if (ipAddressValid)
    {
        if (ip4AddressValid)
        {
            errorCode = TM_ENOERROR;
        }
    }
    else
    {
/*
 *      9.5. The host name is not a valid numerical address and the user
 *           wants IPv4 addresses.
 *           Try to resolve the name by DNS query, type A.
 */
#ifdef TM_USE_IPV6
        if (    (errorCode != TM_ENOERROR)
             && (v6EntryPtr != (ttDnsCacheEntryPtr)0))
        {
/* reserve the v6Entry */
            tm_16bit_clr_bit(v6EntryPtr->dnscFlags, TM_DNSF_USER);
        }
        if (v6EntryPtr != (ttDnsCacheEntryPtr)0)
        {
/* Keep the v6EntryPtr by increasing entry count */
            v6EntryPtr->dnscCount++;
/*      9.6. add tvDnsCachesize by 1 in order to store v4Entry */
            tm_context(tvDnsCacheSize)++;
        }
#endif /* TM_USE_IPV6 */

        errorCode = tfDnsGenerateQuery( nodeName,
                                        TM_DNS_QTYPE_A,
                                        AF_INET,
                                        &v4EntryPtr );

/*
 *      9.7. If tfDnsGenerateQuery returned successfully, there may still have
 *           been an error (for example, no answer to our record query) so get
 *           the error from the cache entry.
 */
        if (errorCode == TM_ENOERROR)
        {
#ifdef TM_USE_IPV6
            if (v6EntryPtr == (ttDnsCacheEntryPtr)0)
            {
#endif /* TM_USE_IPV6 */
                errorCode = v4EntryPtr->dnscErrorCode;
#ifdef TM_USE_IPV6
            }
            else if (v4EntryPtr->dnscErrorCode != TM_ENOERROR)
            {
                errorCode = v6EntryPtr->dnscErrorCode;
            }
#endif /* TM_USE_IPV6 */
        }
#ifdef TM_USE_IPV6
        if (v6EntryPtr != (ttDnsCacheEntryPtr)0)
        {
            if ( errorCode != TM_EWOULDBLOCK )
            {
                if (errorCode != TM_ENOERROR)
                {
                    errorCode = v6EntryPtr->dnscErrorCode;
                }
/* if v4Entry is obtained (wrong or correct ), set TM_DNSF_USER flag */
                v6EntryPtr->dnscFlags |= TM_DNSF_USER;
            }
            v6EntryPtr->dnscCount--;
/*      9.8 restore the tvDnsCachesize by descresing 1 */
            tm_context(tvDnsCacheSize)--;
        }
#endif /* TM_USE_IPV6 */
    }

#endif /* TM_USE_IPV4 */

/*
 * 10. If no error has occurred yet, all of the relevant information
 *     is contained in the address structures or DNS cache entries.
 *     Allocate a series of addrinfo structures which will be returned to
 *     the user, and copy the relevant information to the new addrinfo's.
 */
    if (errorCode != TM_ENOERROR)
    {
        goto getaddrinfoExit;
    }

    userAddrInfoPtr = TM_ADDRINFO_NULL_PTR;

#ifdef TM_USE_IPV6
/*
 * 11. If a IPv6 DNS cache entry exists, get the pointer to the address
 *     information so we can traverse the list and copy the information.
 */
    if (v6EntryPtr != (ttDnsCacheEntryPtr)0)
    {
        cacheAddrInfoPtr = v6EntryPtr->dnscAddrInfoPtr;
    }
    else
    {
        cacheAddrInfoPtr = TM_ADDRINFO_NULL_PTR;
    }

/*
 * 12. For all valid addresses, numerically derived or queried.
 */
    while (ip6AddressValid || cacheAddrInfoPtr != TM_ADDRINFO_NULL_PTR)
    {

/*   12.1. Allocate a new addrinfo entry. */
        newAddrInfoPtr = tfDnsAllocAddrInfo();

/*
 *   12.2. If memory allocation fails, first, clean up any previously
 *           allocated addrinfo structures, then return EAI_MEMORY to the user.
 */
        if (newAddrInfoPtr == TM_ADDRINFO_NULL_PTR)
        {
            if (*resPtrPtr != TM_ADDRINFO_NULL_PTR)
            {
                freeaddrinfo(*resPtrPtr);
            }
            errorCode = EAI_MEMORY;
            goto getaddrinfoExit;
        }

/*
 *   12.3. Copy information into new addrinfo structure.
 *         For raw sockets, the protocol is provided by the user's
 *         protocol hint.
 */
        sockAddr.sockPtr = newAddrInfoPtr->ai_addr;
        ip6addrPtr = sockAddr.sockIn6Ptr;

        ip6addrPtr->sin6_len = sizeof(struct sockaddr_in6);
        ip6addrPtr->sin6_family = AF_INET6;
        ip6addrPtr->sin6_port = servicePort;

        newAddrInfoPtr->ai_flags = hintsFlags;
        newAddrInfoPtr->ai_family = AF_INET6;
        newAddrInfoPtr->ai_addrlen = sizeof(struct sockaddr_in6);

        if (ip6AddressValid)
        {
            tm_6_ip_copy_structs(ip6Address, ip6addrPtr->sin6_addr);
            ip6addrPtr->sin6_scope_id = scopeId;
        }
        else /* if (cacheAddrInfoPtr != TM_ADDRINFO_NULL_PTR) */
        {
            cacheSockAddr.sockPtr = cacheAddrInfoPtr->ai_addr;
            tm_6_ip_copy_structs(cacheSockAddr.sockIn6Ptr->sin6_addr,
                                            ip6addrPtr->sin6_addr);

/*
 *   12.4. If the DNS server returned a local scope address, set
 *         sin6_scope_id based on the incoming interface that the response
 *         was received on. (R2.5:60)
 */
            if (IN6_IS_ADDR_LINKLOCAL(&ip6addrPtr->sin6_addr))
            {
                ip6addrPtr->sin6_scope_id = v6EntryPtr->dnsc6DevIndex;
            }
#ifndef TM_6_USE_RFC3879
            else
            {
                if (IN6_IS_ADDR_SITELOCAL(&ip6addrPtr->sin6_addr))
                {
                    ip6addrPtr->sin6_scope_id = v6EntryPtr->dnsc6SiteId;
                }
            }
#endif/* TM_6_USE_RFC3879 */
        }

/*   12.5. Link new addrinfo to previous one in the chain. */
        if (userAddrInfoPtr == TM_ADDRINFO_NULL_PTR)
        {
            userAddrInfoPtr = newAddrInfoPtr;
            *resPtrPtr = userAddrInfoPtr;
        }
        else
        {
            userAddrInfoPtr->ai_next = newAddrInfoPtr;
            userAddrInfoPtr = newAddrInfoPtr;
        }

/*   12.6. Move to next address in this cache entry. */
        if (ip6AddressValid)
        {
            ip6AddressValid = TM_8BIT_NO;
        }
        else if (cacheAddrInfoPtr != TM_ADDRINFO_NULL_PTR)
        {
            cacheAddrInfoPtr = cacheAddrInfoPtr->ai_next;
        }
    }
#endif /* TM_USE_IPV6 */


/*
 * 13. If a IPv4 DNS cache entry exists, get the pointer to the address
 *     information so we can traverse the list and copy the information.
 *     If the protocol type is IPv6, the addresses are returned as
 *     IPv4-mapped IPv6 addresses.
 */
#ifdef TM_USE_IPV4
    if (v4EntryPtr != (ttDnsCacheEntryPtr)0)
    {
        cacheAddrInfoPtr = v4EntryPtr->dnscAddrInfoPtr;
    }
    else
    {
        cacheAddrInfoPtr = TM_ADDRINFO_NULL_PTR;
    }

/*
 * 14. For all valid addresses, numerically derived or queried.
 */
    while (ip4AddressValid || cacheAddrInfoPtr != TM_ADDRINFO_NULL_PTR)
    {

/*   14.1. Allocate a new addrinfo entry. */
        newAddrInfoPtr = tfDnsAllocAddrInfo();

/*
 *   14.2. If memory allocation fails, first, clean up any previously
 *         allocated addrinfo structures, then return EAI_MEMORY to the user.
 */
        if (newAddrInfoPtr == TM_ADDRINFO_NULL_PTR)
        {
            if (*resPtrPtr != TM_ADDRINFO_NULL_PTR)
            {
                freeaddrinfo(*resPtrPtr);
            }
            errorCode = EAI_MEMORY;
            goto getaddrinfoExit;
        }

/*
 *   14.3. Copy information into new addrinfo structure.
 *         For raw sockets, the protocol is provided by the user's
 *         protocol hint.
 */

        newAddrInfoPtr->ai_flags = hintsFlags;
/*
 *   14.4. If the user specified an address family of IPv6, addresses
 *         will be copied as IPv4-mapped IPv6 addresses.
 */
#ifdef TM_USE_IPV6
        if (hintsFamily == AF_INET6)
        {
            sockAddr.sockPtr = newAddrInfoPtr->ai_addr;
            ip6addrPtr = sockAddr.sockIn6Ptr;

            ip6addrPtr->sin6_len = sizeof(struct sockaddr_in6);
            ip6addrPtr->sin6_family = AF_INET6;
            ip6addrPtr->sin6_port = servicePort;

            newAddrInfoPtr->ai_family = AF_INET6;
            newAddrInfoPtr->ai_addrlen
                                = sizeof(struct sockaddr_in6);

            if (ip4AddressValid)
            {
                tm_6_addr_to_ipv4_mapped(ip4Address,
                                        &ip6addrPtr->sin6_addr);
                ip6addrPtr->sin6_scope_id = scopeId;
            }
            else /* if (cacheAddrInfoPtr != TM_ADDRINFO_NULL_PTR) */
            {
                cacheSockAddr.sockPtr = cacheAddrInfoPtr->ai_addr;
                tm_6_addr_to_ipv4_mapped(
                        cacheSockAddr.sockInPtr->sin_addr.s_addr,
                        &ip6addrPtr->sin6_addr );
            }
        }
        else
#endif /* TM_USE_IPV6 */

/*
 *   14.5. If the user specified an address family of IPv4, addresses
 *         will be presented in traditional IPv4 format.
 */
        /* if ((hintsFamily == AF_INET) || (hintsFamily == AF_UNSPEC)) */
        {
            sockAddr.sockPtr = newAddrInfoPtr->ai_addr;
            ip4addrPtr = sockAddr.sockInPtr;

            ip4addrPtr->sin_len = sizeof(struct sockaddr_in);
            ip4addrPtr->sin_family = AF_INET;
            ip4addrPtr->sin_port = servicePort;

            newAddrInfoPtr->ai_family = AF_INET;
            newAddrInfoPtr->ai_addrlen
                                = sizeof(struct sockaddr_in);

            if (ip4AddressValid)
            {
                tm_ip_copy(ip4Address, ip4addrPtr->sin_addr.s_addr);
            }
            else /* if (cacheAddrInfoPtr != TM_ADDRINFO_NULL_PTR) */
            {
                cacheSockAddr.sockPtr = cacheAddrInfoPtr->ai_addr;
                tm_ip_copy(
                        cacheSockAddr.sockInPtr->sin_addr.s_addr,
                        ip4addrPtr->sin_addr.s_addr);
            }
        }

/*   14.6. Link new addrinfo to previous one in the chain. */
        if (userAddrInfoPtr == TM_ADDRINFO_NULL_PTR)
        {
            userAddrInfoPtr = newAddrInfoPtr;
            *resPtrPtr = userAddrInfoPtr;
        }
        else
        {
            userAddrInfoPtr->ai_next = newAddrInfoPtr;
            userAddrInfoPtr = newAddrInfoPtr;
        }

/*   14.7. Move to next address in this cache entry. */
        if (ip4AddressValid)
        {
            ip4AddressValid = TM_8BIT_NO;
        }
        else if (cacheAddrInfoPtr != TM_ADDRINFO_NULL_PTR)
        {
            cacheAddrInfoPtr = cacheAddrInfoPtr->ai_next;
        }
    }
#endif /* TM_USE_IPV4 */

/*
 * 15. If no addresses have been collected by this point, return EAI_NONAME.
 */
    if (*resPtrPtr == TM_ADDRINFO_NULL_PTR)
    {
        errorCode = EAI_NONAME;
        goto getaddrinfoExit;
    }

/*
 * 16. If the AI_CANONFLAG was set, copy the canonical name for this host into
 *     the first addrinfo structure being returned to the user.  The canonical
 *     name should be present on both the IPv6 as well as the IPv4 cache
 *     entries, but this isn't guaranteed (depends on the DNS server).  We
 *     check the IPv4 entry first and if it's not present, we check the IPv6
 *     entry.
 */
    if ( hintsFlags & AI_CANONNAME )
    {
#ifdef TM_USE_IPV6
        if (    (v6EntryPtr != (ttDnsCacheEntryPtr)0)
             && (v6EntryPtr->dnscAddrInfoPtr != TM_ADDRINFO_NULL_PTR)
             && (v6EntryPtr->dnscAddrInfoPtr->ai_canonname !=
                 (char TM_FAR *) 0) )
        {
            canonNamePtr = v6EntryPtr->dnscAddrInfoPtr->ai_canonname;
        }
        else
#endif /* TM_USE_IPV6 */
        {
#ifdef TM_USE_IPV4
            if (    (v4EntryPtr != (ttDnsCacheEntryPtr)0)
                 && (v4EntryPtr->dnscAddrInfoPtr !=
                     TM_ADDRINFO_NULL_PTR)
                 && (v4EntryPtr->dnscAddrInfoPtr->ai_canonname !=
                     (char TM_FAR *) 0))
            {
                canonNamePtr = v4EntryPtr->dnscAddrInfoPtr->ai_canonname;
            }
            else
#endif /* TM_USE_IPV4 */
            {
                canonNamePtr = (char TM_FAR *)0;
            }
        }

/*
 *     16.1. Allocate a string buffer to hold the canonical name.  Copy the
 *           canonical name into this buffer and attach it to the addrinfo
 *           structure.
 */
        if (canonNamePtr != (char TM_FAR *)0)
        {
            canonNameLen = (int) (tm_strlen(canonNamePtr) + 1);
            (*resPtrPtr)->ai_canonname =
                (char TM_FAR *) tm_get_raw_buffer((ttPktLen)canonNameLen);
            if ((*resPtrPtr)->ai_canonname != (char TM_FAR *)0)
            {
                (void)tm_strncpy((*resPtrPtr)->ai_canonname,
                                 canonNamePtr,
                                 canonNameLen);
            }
        }
    }
/*
 * 17. Indicate that the user has retrieved the information from the cache
 *     entries.  If retrieving both IPv6 and IPv4 addresses, it is important
 *     to do this only after both entries have been retrieved.  This prevents
 *     one entry from disappearing while waiting for the other to complete.
 */
#ifdef TM_USE_IPV6
#ifdef TM_6_USE_RFC3484
/* 
 * 18. Sort the addrinfo structures according to RFC3484
 */
    if ((*resPtrPtr != TM_ADDRINFO_NULL_PTR) && (errorCode == TM_ENOERROR)
        && (v6EntryPtr != (ttDnsCacheEntryPtr)0) )
    {
/* 18.1 Check whether the interface is proper */
        tm_assert( getaddrinfo,
            (v6EntryPtr->dnsc6DevIndex <= tm_context(tvAddedInterfaces)));
/* 18.2 Check whether the device entry pointer has been previously entered */
        listdevEntryPtr 
            = tm_context(tvDevIndexToDevPtr)[v6EntryPtr->dnsc6DevIndex];
        if (!( (v6EntryPtr->dnscAddrInfoPtr == TM_ADDRINFO_NULL_PTR)
#ifdef TM_USE_IPV4
            && (v4EntryPtr != (ttDnsCacheEntryPtr)0)
            && (v4EntryPtr->dnscAddrInfoPtr != TM_ADDRINFO_NULL_PTR) 
            && (tm_8bit_one_bit_set( tm_ip_dev_conf_flag(listdevEntryPtr, 0),
                                     TM_DEV_IP_CONFIG))
#endif /* TM_USE_IPV4 */
           ) )
        {
/*      18.2.1 Sort the list of destination addresses by calling the function
               tf6SelectDestinationAddress() */
            errorCode = tf6SelectDestinationAddress(listdevEntryPtr,
                                                    resPtrPtr);
            if (errorCode != TM_ENOERROR)
            {
                errorCode = TM_ENOERROR;
            }
        }
    }
#endif /* TM_6_USE_RFC3484 */
#endif /* TM_USE_IPV6 */

/*
 * 19. Expand the list of addresses to include all supported socket types and
 *     protocols that the user requested.
 */
    for (   userAddrInfoPtr = *resPtrPtr;
            userAddrInfoPtr != TM_ADDRINFO_NULL_PTR;
            userAddrInfoPtr = userAddrInfoPtr->ai_next )
    {
/*
 * newAddrInfoPtr will be NULL whenever we need to allocate a new entry.
 * For each existing entry, we set newAddrInfoPtr to point to the entry
 * and just update the socket type and protocol information in the existing
 * entry.
 * userAddrInfoPtr will be updated to reflect the current entry as new
 * entries are inserted into the list.
 */
        newAddrInfoPtr = userAddrInfoPtr;
/*
 *   19.1. For each valid socket type and associated protocol.
 */
        for (sockTypePtr = tlSockTypes; *sockTypePtr; sockTypePtr += 2)
        {
/*
 *   19.1.1. Skip socket types and protocols that the user does not want.
 */
            if (    (   hintsSockType != 0
                    &&  hintsSockType != sockTypePtr[0] )
                ||  (   hintsProtocol != 0
                    &&  sockTypePtr[1] != 0
                    &&  hintsProtocol != sockTypePtr[1] ) )
            {
                continue;
            }
/*
 *   19.1.2. Allocate a new addrinfo entry if we are not updating an existing
 *           entry.
 */
            if (newAddrInfoPtr == TM_ADDRINFO_NULL_PTR)
            {
                newAddrInfoPtr = tfDnsAllocAddrInfo();
/*
 *   19.1.2.1. If memory allocation fails, first, clean up any previously
 *             allocated addrinfo structures, then return EAI_MEMORY to the
 *             user.
 */
                if (newAddrInfoPtr == TM_ADDRINFO_NULL_PTR)
                {
                    if (*resPtrPtr != TM_ADDRINFO_NULL_PTR)
                    {
                        freeaddrinfo(*resPtrPtr);
                    }
                    errorCode = EAI_MEMORY;
                    goto getaddrinfoExit;
                }
/*
 *   19.1.2.2. Duplicate the address and other information from the previous
 *             entry.
 */
                tm_bcopy(   userAddrInfoPtr->ai_addr,
                            newAddrInfoPtr->ai_addr,
                            userAddrInfoPtr->ai_addrlen );
                newAddrInfoPtr->ai_flags   = userAddrInfoPtr->ai_flags;
                newAddrInfoPtr->ai_family  = userAddrInfoPtr->ai_family;
                newAddrInfoPtr->ai_addrlen = userAddrInfoPtr->ai_addrlen;
/*
 *   19.1.2.3. Insert the new entry in the list after the current entry
 *             (pointed to by userAddrInfoPtr).
 *             Set userAddrInfoPtr to reflect the new "current" entry.
 */
                newAddrInfoPtr->ai_next    = userAddrInfoPtr->ai_next;
                userAddrInfoPtr->ai_next   = newAddrInfoPtr;
                userAddrInfoPtr            = newAddrInfoPtr;
            }
/*
 *   19.1.3. Update the socket type and protocol data for this entry.
 *           For raw sockets, the protocol is provided by the user's
 *           protocol hint.
 */
            newAddrInfoPtr->ai_socktype = sockTypePtr[0];
            newAddrInfoPtr->ai_protocol
                        = sockTypePtr[1] ? sockTypePtr[1] : hintsProtocol;
/*
 *   19.1.4. Indicate that a new allocation will be needed next time
 *           around the loop.
 */
            newAddrInfoPtr = TM_ADDRINFO_NULL_PTR;
        }
    }


/* 20. If no records for this protocol type were found, return EAI_NODATA. */
/* 20.1 Always decrease entry count unless its return code is EWOULDBLOCK */
getaddrinfoExit:

/* Unlock the DNS global variables */
    tm_call_unlock(&tm_context(tvDnsLockEntry));

    return errorCode;
}


/*
 * getnameinfo Function Description
 *
 * Translates an address into it's corresponding nodename (hostname).  Similar
 * to gethostbyaddr.  Retrieving a service name or port number is not
 * supported in our implementation of getnameinfo, so serviceName must be set
 * to NULL.
 *
 * Parameters
 * Parameter          Description
 * addressPtr         Pointer to address to translate.
 * addressLength      Length of address structure.
 * hostname           Buffer to copy nodename into on completion.
 * hostnameLength     Size of hostname buffer.
 * serviceName        Not used.
 * serviceNameLength  Not used.
 * flags              NI_NUMERICHOST
 *
 * Returns
 * Value        Meaning
 * EAI_NONAME   NI_NUMERICHOST flag was set, but couldn't convert binary
 *              address into string.
 * EAI_FAMILY   Unrecognized or unsupported address family.
 * EAI_MEMORY   Memory allocation failure
 * EAI_FAIL     Non-recoverable error occurred.
 * EAI_NONAME   No entry found for this address.
 *
 *
 * 1. ERROR CHECKING: verify that the hostname or serviceName storage
 *    exists and that a socket address has been provided.
 *     1.1. Verify the address family and address length.
 * 2. Resolve service name.
 *    Currently, the numeric form of the service address is always returned,
 *    so there is no need to check if the NI_NUMERICSERV or NI_DGRAM flags
 *    are set.
 * 3. If the NI_NUMERICHOST flag is set, the user simply wants the IP address
 *    to be returned in numeric form (no DNS lookup should be performed).
 *     3.1. Call inet_ntop to convert the binary address into a numeric
 *          address string.
 *     3.2. If inet_ntop fails, return EAI_NONAME to the user.
 * 4. If the address type is IPv6, create IP6.ARPA hostname from the specified
 *    address ([R2.5:50]):
 *     4.1. Do IPv4 lookup on IPv4-mapped and -compatible IPv6 addresses.
 *          [RFC3493].
 *     4.2. Return EAI_NONAME, if the IPv6 unspecified address is given
 *          [RFC3493].
 *     4.3. Loop through IPv6 address, starting at the least significant byte.
 *     4.4. Output the IP6.ARPA address, nibble by nibble using sprintf:
 *     4.5. Append the "IP6" suffix to the nibble string created above.
 *     4.6. Do not append ARPA or INT suffix - this will be done by the sending
 *          routine.  "ARPA" will be used first - if this fails, "INT" will be
 *          used.
 * 5. If the address type is IPv4 or IPv4-mapped or -compatible IPv6 address,
 *    create in-addr.arpa hostname from the specified address.
 * 6. Search the cache for an entry with name information for this address.
 *    If not found in the cache, generate and send a new query.
 * 7. If no error occurred, copy the hostname information to the user's
 *    buffer.  Copy as much as possible, but if there is not enough space in
 *    the user's buffer, return EAI_OVERFLOW.
 * 8. If an error was returned form tfDnsGenerateQuery:
 *     8.1. If a memory allocation error occurred, return EAI_MEMORY to the
 *          user.
 *     8.2. If there was no DNS entry for this address and the NI_NAMEREQD
 *          flag is set, return EAI_NONAME. Otherwise, return the numeric
 *          form. [RFC3493]
 *     8.3. If there was no DNS response and the NI_NAMEREQD flag is not set,
 *          return the numeric form. [RFC3493]
 *     8.4. If another error occurred, return EAI_FAIL to the user.
 *
 */
int getnameinfo( const struct sockaddr TM_FAR * addressPtr,
                 int           addressLength,
                 char TM_FAR * hostname,
                 int           hostnameLength,
                 char TM_FAR * serviceName,
                 int           serviceNameLength,
                 int           flags )
{

    int                      errorCode;
    int                      family;
    ttSockAddrPtrUnion       sockAddr;
    ttCharPtr                revAddrPtr;
    ttVoidPtr                voidAddrPtr;
    ttDnsCacheEntryPtr       entryPtr;
    int                      i;
    int                      j;
    tt16Bit                  serviceNum;
    int                      tmpLen;
#ifdef TM_USE_IPV4
    tt4IpAddress             ip4Addr;
    tt4IpAddress             reversedIp4Addr;
    int                      addrLength;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    ttCharPtr                stringPtr;
    tt6IpAddressPtr          ip6AddrPtr;
    tt8Bit                   nibble;
#endif /* TM_USE_IPV6 */

    errorCode  = TM_ENOERROR;
    revAddrPtr = (ttCharPtr) 0;

#ifdef TM_USE_IPV4
    addrLength = 0;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    nibble = (tt8Bit)0;
#endif /* TM_USE_IPV6 */

/* 0. Lock the DNS global variables */
    tm_call_lock_wait(&tm_context(tvDnsLockEntry));


/*
 * 1. ERROR CHECKING: verify that the hostname or serviceName storage
 *    exists and that a socket address has been provided.
 */
    if (hostnameLength <= 0)
    {
        hostname = (ttCharPtr)0;
    }
    if (serviceNameLength <= 0)
    {
        serviceName = (ttCharPtr)0;
    }
    if ( (  (hostname == (ttCharPtr)0) && (serviceName == (ttCharPtr)0) )
         || (addressPtr == TM_SOCKADDR_NULL_PTR) )
    {
        errorCode = TM_EINVAL;
        goto getnameinfoExit;
    }

    family = addressPtr->sa_family;
    sockAddr.sockPtr = (struct sockaddr TM_FAR *) addressPtr;
/*
 *     1.1. Verify the address family and address length.
 */
#ifdef TM_USE_IPV6
    ip6AddrPtr = &sockAddr.sockIn6Ptr->sin6_addr;
    if (   (family == AF_INET6)
        && ((unsigned int)addressLength >= sizeof(struct sockaddr_in6)) )
    {
        serviceNum = ntohs(sockAddr.sockIn6Ptr->sin6_port);
        voidAddrPtr = (ttVoidPtr) ip6AddrPtr;
    }
    else
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
    if (   (family == AF_INET)
        && ((unsigned int)addressLength >= sizeof(struct sockaddr_in)) )
    {
        serviceNum = ntohs(sockAddr.sockInPtr->sin_port);
        voidAddrPtr = (ttVoidPtr) &sockAddr.sockInPtr->sin_addr.s_addr;
    }
    else
#endif /* TM_USE_IPV4 */
    {
/* Bad address family (not IPv4 or IPv6, or protocol not enabled) */
        errorCode = EAI_FAMILY;
        goto getnameinfoExit;
    }

/*
 * 2. Resolve service name.
 *    Currently, the numeric form of the service address is always returned,
 *    so there is no need to check if the NI_NUMERICSERV or NI_DGRAM flags
 *    are set.
 */
    if (serviceName != (ttCharPtr)0)
    {
/* Fill from the end of the user's buffer backwards, being careful not to
 * overflow. */
        i = serviceNameLength;
        serviceName[--i] = '\0';
        do
        {
            if (i == 0)
            {
                errorCode = EAI_OVERFLOW;
                goto getnameinfoExit;
            }
            serviceName[--i] = (char)('0' + (serviceNum % 10));
            serviceNum /= 10;
        } while (serviceNum != TM_16BIT_ZERO);

/* Left justify the string in the user's buffer */
        if (i > 0)
        {
            for (j = 0; i < serviceNameLength; ++i, ++j)
            {
                serviceName[j] = serviceName[i];
            }
        }
    }

/*
 * The remainder of the function deals with the hostname. If no storage was
 * provided, skip to the end.
 */
    if (hostname == (ttCharPtr)0)
    {
        goto getnameinfoExit;
    }

/*
 * 3. If the NI_NUMERICHOST flag is set, the user simply wants the IP address
 *    to be returned in numeric form (no DNS lookup should be performed).
 */
    if (flags & NI_NUMERICHOST)
    {
getnameinfoNumeric:
/*
 *     3.1. Call inet_ntop to convert the binary address into a numeric
 *          address string.
 */
/* We just save the return from inet_ntop to check for success... */
        voidAddrPtr = (ttVoidPtr)
            inet_ntop(family, voidAddrPtr, hostname, hostnameLength );

/*     3.2. If inet_ntop fails, return EAI_NONAME to the user. */
        if (voidAddrPtr != TM_VOID_NULL_PTR)
        {
            errorCode = TM_ENOERROR;
        }
        else
        {
            errorCode = EAI_NONAME;
        }

        goto getnameinfoExit;
    }

/*
 * 4. If the address type is IPv6, create IP6.ARPA hostname from the specified
 *    address ([R2.5:50]):
 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
    if (    family == AF_INET6
/*
 *     4.1. Do IPv4 lookup on IPv4-mapped and -compatible IPv6 addresses.
 *          [RFC3493].
 */
        &&  !IN6_IS_ADDR_V4MAPPED(ip6AddrPtr)
        &&  !IN6_IS_ADDR_V4COMPAT(ip6AddrPtr) )
#endif /* TM_USE_IPV4 */
    {
/*
 *     4.2. Return EAI_NONAME, if the IPv6 unspecified address is given
 *          [RFC3493].
 */
        if (IN6_IS_ADDR_UNSPECIFIED(ip6AddrPtr))
        {
            errorCode = EAI_NONAME;
            goto getnameinfoExit;
        }

        stringPtr = (ttCharPtr) tm_get_raw_buffer(TM_6_MAX_IN_ARPA_LENGTH);
        if (stringPtr == (ttCharPtr)0)
        {
            errorCode = EAI_MEMORY;
            goto getnameinfoExit;
        }
        revAddrPtr = stringPtr;

/*     4.3. Loop thru IPv6 address, starting at the least significant byte. */
        for ( i=(TM_6_IP_ADDRESS_LENGTH-1); i>=0; i--)
        {

/*     4.4. Output the IP6.ARPA address, nibble by nibble */

/* We could do this via sprintf, but would rather not have to include this
   module. */
#ifdef TM_DSP
            nibble = tm_dsp_get_byte_larr(ip6AddrPtr->s6LAddr, i) & 0x0f;
#else /* !TM_DSP */
            nibble = (tt8Bit) (ip6AddrPtr->s6_addr[i] & 0x0f);
#endif /* TM_DSP */
            if (nibble < 10)
            {
                *stringPtr++ = (char) (nibble + '0');
            }
            else
            {
                *stringPtr++ = (char) (nibble - 10 + 'A');
            }
            *stringPtr++ = '.';

#ifdef TM_DSP
            nibble = (tm_dsp_get_byte_larr(ip6AddrPtr->s6LAddr,i) >> 4) & 0x0f;
#else /* !TM_DSP */
            nibble = (tt8Bit) ((ip6AddrPtr->s6_addr[i] >> 4) & 0x0f);
#endif /* TM_DSP */
            if (nibble < 10)
            {
                *stringPtr++ = (char) (nibble + '0');
            }
            else
            {
                *stringPtr++ = (char) (nibble - 10 + 'A');
            }
            *stringPtr++ = '.';
        }

/*     4.5. Append the "IP6" suffix to the nibble string created above. */
        *stringPtr++ = 'i';
        *stringPtr++ = 'p';
        *stringPtr++ = '6';
        *stringPtr++ = '\0';

/*
 *     4.6. Do not append ARPA or INT suffix - this will be done by the sending
 *          routine.  "ARPA" will be used first - if this fails, "INT" will be
 *          used.
 */
    }
#ifdef TM_USE_IPV4
    else
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
    {
/*
 * 5. If the address type is IPv4 or IPv4-mapped or -compatible IPv6 address,
 *    create in-addr.arpa hostname from the specified address.
 */
/* Allocate working buffer to create the in-addr.arpa hostname in. */
        revAddrPtr = (ttCharPtr) tm_get_raw_buffer(TM_MAX_IN_ARPA_LENGTH);
        if (revAddrPtr == (ttCharPtr)0)
        {
            errorCode = EAI_MEMORY;
            goto getnameinfoExit;
        }

#ifdef TM_USE_IPV6
        if (family == AF_INET6)
        {
            ip4Addr = tm_4_ip_addr(*ip6AddrPtr);
        }
        else
#endif /* TM_USE_IPV6 */
        {
            ip4Addr = sockAddr.sockInPtr->sin_addr.s_addr;
        }
        reversedIp4Addr = ( (ip4Addr >> 24) & TM_UL(0x000000ff) ) |
            ( (ip4Addr >> 8)  & TM_UL(0x0000ff00) ) |
            ( (ip4Addr << 8)  & TM_UL(0x00ff0000) ) |
            ( (ip4Addr << 24) & TM_UL(0xff000000) );

        tfInetToAscii( reversedIp4Addr, revAddrPtr );
        addrLength = (int)tm_strlen( revAddrPtr );
        tm_strcpy( revAddrPtr + addrLength, ".in-addr.arpa" );
    }
#endif /* TM_USE_IPV4 */


/*
 * 6. Search the cache for an entry with name information for this address.
 *    If not found in the cache, generate and send a new query.
 */

     errorCode =
         tfDnsGenerateQuery(revAddrPtr, TM_DNS_QTYPE_REV, family, &entryPtr);

/*
 * If tfDnsGenerateQuery returned successfully, there may still have been
 * an error (for example, no answer to our record query) so get the error
 * from the cache entry.
 */
     if (errorCode == TM_ENOERROR)
     {
         tm_assert(getnameinfo, entryPtr != (ttDnsCacheEntryPtr)0);
         errorCode = entryPtr->dnscErrorCode;
     }

     tm_free_raw_buffer((ttRawBufferPtr) revAddrPtr);

/*
 * 7. If no error occurred, copy the hostname information to the user's
 *    buffer.  Copy as much as possible, but if there is not enough space in
 *    the user's buffer, return EAI_OVERFLOW.
 */
     if (errorCode == TM_ENOERROR)
     {
         (void)tm_strncpy(hostname,
                            entryPtr->dnscRevHostnameStr, hostnameLength);
         tmpLen = (int)tm_strlen(entryPtr->dnscRevHostnameStr);
         if (tmpLen > hostnameLength)
         {
             errorCode = EAI_OVERFLOW;
         }

/* 8. If an error was returned form tfDnsGenerateQuery: */
     }
     else
     {
/*
 *     8.1. If a memory allocation error occurred, return EAI_MEMORY to the
 *          user.
 */
         if ( (errorCode == TM_ENOBUFS) || (errorCode == TM_ENOMEM) )
         {
             errorCode = EAI_MEMORY;
         }
         else if (    (errorCode == TM_DNS_EANSWER)
                   || (errorCode == TM_DNS_ENAME_ERROR) )
         {
/*
 *     8.2. If there was no DNS entry for this address and the NI_NAMEREQD
 *          flag is set, return EAI_NONAME. Otherwise, return the numeric
 *          form. [RFC3493]
 */
             if (flags & NI_NAMEREQD)
             {
                errorCode = EAI_NONAME;
             }
             else
             {
                goto getnameinfoNumeric;
             }
         }
         else if (errorCode == TM_ETIMEDOUT)
         {
/*
 *     8.3. If there was no DNS response and the NI_NAMEREQD flag is not set,
 *          return the numeric form. [RFC3493]
 */
             if ((flags & NI_NAMEREQD) == 0)
             {
                goto getnameinfoNumeric;
             }
         }
         else if (errorCode != TM_EWOULDBLOCK)
         {
/*     8.4. If another error occurred, return EAI_FAIL to the user. */
             errorCode = EAI_FAIL;
         }
     }

getnameinfoExit:
/* Unlock the DNS global variables */
    tm_call_unlock(&tm_context(tvDnsLockEntry));

    return errorCode;
}

/****************************************************************************
* FUNCTION: tfDnsOpenSocket
*
* PURPOSE:
*   Open the socket, set the appropriate options, and register the callback:
*     1. Create a UDP socket if there is room.
*     2. Set the blocking mode for the resolver.
*     3. Register receive callback for the DNS socket to tfDnsCallback.
*
* PARAMETERS:
*   (none)
*
* RETURNS:
*   int
*     The new socket descriptor.
*
****************************************************************************/

static int tfDnsOpenSocket(void)
{
    int     newSocket;
#ifdef TM_USE_STRONG_ESL
    int     errorCode;
#endif /* TM_USE_STRONG_ESL */
#ifdef TM_DSP
    int    optValue;
#endif /* TM_DSP */

/* 1. Create a UDP socket if there is room. */
    if (tm_context(tvDnsNumSocksAllocated) == tm_context(tvDnsMaxNumberSockets))
    {
        newSocket = TM_SOCKET_ERROR;
    }
    else
    {
#ifdef TM_USE_IPV6
        newSocket = socket(PF_INET6, SOCK_DGRAM, 0);
#else /* !TM_USE_IPV6 */
        newSocket = socket(PF_INET, SOCK_DGRAM, 0);
#endif /* TM_USE_IPV6 */
    }

#ifdef TM_USE_STRONG_ESL
    if (   (newSocket != TM_SOCKET_ERROR)
        && (tm_context(tvDnsDevPtr) != (ttDeviceEntryPtr)0) )
    {
        errorCode = setsockopt(
                            newSocket,
                            SOL_SOCKET,
                            SO_BINDTODEVICE,
                            (char *)((tm_context(tvDnsDevPtr))->devNameArray),
                            IFNAMSIZ);
        if (errorCode != TM_ENOERROR)
        {
            (void)tfClose(newSocket);
            newSocket = TM_SOCKET_ERROR;
        }
    }
#endif /* TM_USE_STRONG_ESL */

    if (newSocket != TM_SOCKET_ERROR)
    {
#ifdef TM_DSP
        optValue = 1;
        (void)setsockopt(newSocket,
                         SOL_SOCKET,
                         SO_UNPACKEDDATA,
                         (const char TM_FAR *)&optValue,
                         sizeof(int));
#endif /* TM_DSP */
/* 2. Set the blocking mode for the resolver. */
        (void)tfBlockingState(newSocket, tm_context(tvDnsBlockingMode));

/* 3. Register receive callback for the DNS socket to tfDnsCallback. */
        (void)tfRegisterSocketCB(newSocket,
                                 tfDnsCallback,
                                 TM_CB_RECV);
        tm_context(tvDnsNumSocksAllocated)++;
    }

    return newSocket;
}

/****************************************************************************
* FUNCTION: tfDnsCloseSocket
*
* PURPOSE:
*   Close the socket.
*
* PARAMETERS:
*   dnsCacheEntryPtr:
*       The DNS cache entry to close the socket for.
*
* RETURNS:
*   (none)
*
****************************************************************************/
static void tfDnsCloseSocket(ttDnsCacheEntryPtr dnsCacheEntryPtr)
{
    if (dnsCacheEntryPtr->dnscSocket != TM_SOCKET_ERROR)
    {
        (void)tfClose(dnsCacheEntryPtr->dnscSocket);
        tm_context(tvDnsNumSocksAllocated)--;
        dnsCacheEntryPtr->dnscSocket = TM_SOCKET_ERROR;
    }

    return;
}

/****************************************************************************
* FUNCTION: tfDnsCloseSocketComplete
*
* PURPOSE:
*   Close the socket and mark the cache entry as complete.
*
* PARAMETERS:
*   dnsCacheEntryPtr:
*       The DNS cache entry to close the socket for.
*
* RETURNS:
*   (none)
*
****************************************************************************/
static void tfDnsCloseSocketComplete(ttDnsCacheEntryPtr dnsCacheEntryPtr)
{
    tfDnsCloseSocket(dnsCacheEntryPtr);

/* Mark this cache entry as complete. */
    dnsCacheEntryPtr->dnscFlags |= TM_DNSF_COMPLETE;

    return;
}

/****************************************************************************
* FUNCTION: tfDnsGetNewTransId
*
* PURPOSE:
*   Generate a new random Transaction ID for the new request.
*
* PARAMETERS:
*   (none)
*
* RETURNS:
*   tt16Bit
*     The newly generated Transaction ID.
*
****************************************************************************/
static tt16Bit tfDnsGetNewTransId(void)
{
    ttDnsCacheEntryPtr dnsCacheEntryPtr;
    int         i;
    tt16Bit     newTransId;
    tt8Bit      duplicateTransId;

    duplicateTransId = TM_8BIT_YES;

    do
    {
        tfGetRandomBytes((tt8BitPtr)&newTransId, sizeof(newTransId));

/* First check this value against the value in all of the cache entries */
        dnsCacheEntryPtr = tfDnsCacheLookup(TM_DNS_SEARCH_ID,
                                            (ttVoidPtr)&newTransId,
                                            0,
                                            TM_DNS_LOOKUP_ALL);
        if (dnsCacheEntryPtr != (ttDnsCacheEntryPtr)0)
        {
            duplicateTransId = TM_8BIT_YES;
            continue;
        }

/* Next check it against the backlog */
        for (i=0; i<TM_DNS_TRANS_ID_BACKLOG_SIZE; i++)
        {
            if (tm_context(tvDnsTransId)[i] == newTransId)
            {
                duplicateTransId = TM_8BIT_YES;
                break;
            }
            else
            {
                duplicateTransId = TM_8BIT_NO;
            }
        }
    }
    while (duplicateTransId == TM_8BIT_YES);

    if (tm_context(tvDnsCurrentTransId) == TM_DNS_TRANS_ID_BACKLOG_SIZE)
    {
        tm_context(tvDnsCurrentTransId) = 0;
    }
    tm_context(tvDnsTransId)[tm_context(tvDnsCurrentTransId)] = newTransId;
    tm_context(tvDnsCurrentTransId)++;

    return newTransId;
}

#ifdef TM_USE_STOP_TRECK
void tfDnsDeInit(
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* !TM_MULTIPLE_CONTEXT */
                )
{
    tm_call_lock_wait(&tm_context_var(tvDnsLockEntry));
    while (tm_context_var(tvDnsCachePtr) != (ttDnsCacheEntryPtr)0)
    {
        (void)tfDnsCacheRemove(tm_context_var(tvDnsCachePtr));
    }
    tm_call_unlock(&tm_context_var(tvDnsLockEntry));
}
#endif /* TM_USE_STOP_TRECK */
