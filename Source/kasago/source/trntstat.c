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
 * Description: netstat tool that outputs the route and ARP table, tcp and
 *              udp sockets 
 * Filename: trntstat.c
 * Author: Bryan 
 * Date Created: 05/03/2002
 * $Source: source/trntstat.c $
 *
 * Modification History
 * $Revision: 6.0.2.7 $
 * $Date: 2014/12/17 14:26:26JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_NETSTAT

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/* 
 * copy the dst string to the src string, 
 * and move the dst string pointer forward by len 
 */
#define tm_nt_strncpy(dst, src, len) \
{\
    (void)tm_strncpy(dst, src, tm_strlen(src));\
    dst += len;\
}


/* String length of all the tahble feilds */
/* IP Address string length */
#ifdef TM_USE_IPV6
#define TM_NT_IP_ADDR_STR_LEN       INET6_ADDRSTRLEN
#else /* TM_USE_IPV6 */
#define TM_NT_IP_ADDR_STR_LEN       INET_ADDRSTRLEN
#endif /* TM_USE_IPV6 */


/* Address:port */
#define TM_NT_SOCK_ADDR_STR_LEN     TM_NT_IP_ADDR_STR_LEN + 6
/* Adderss/prefixLen */
#define TM_NT_KEY_IP_ADDR_STR_LEN   TM_NT_IP_ADDR_STR_LEN + 4
#define TM_NT_REFS_STR_LEN          5
#define TM_NT_MHOME_STR_LEN         6
#define TM_NT_IFACE_STR_LEN         9
#define TM_NT_MTU_STR_LEN           5
#define TM_NT_HOPS_STR_LEN          5
#define TM_NT_TTL_STR_LEN           12
#define TM_NT_FLAGS_STR_LEN         16

#define TM_NT_HWADDR_STR_LEN        18
#define TM_NT_HWTYPE_STR_LEN        8
#define TM_NT_HWADDRLEN_STR_LEN     4
#define TM_NT_INDEX_STR_LEN         6
#define TM_NT_FAMILY_STR_LEN        7
#define TM_NT_Q_STR_LEN             12
#define TM_NT_STATE_BACKLOG_STR_LEN 14
#define TM_NT_BACKLOG_STR_LEN       8
#define TM_NT_RTO_STR_LEN           11 
#define TM_NT_NUD_STATE_STR_LEN     6

#define TM_NT_LAST_SEQ_STR_LEN          8
#define TM_NT_LAST_USED_TIME_STR_LEN    13
#define TM_NT_LAST_UPDATED_TIME_STR_LEN 12 
#define TM_NT_HOME_REG_STR_LEN          8

#define TM_NT_DEVICE_STATUS_STR_LEN     7
#define TM_NT_ADDR_CONF_TYPE_STR_LEN    7
#define TM_NT_ADDR_CONF_STATUS_STR_LEN  12
#define TM_NT_ADDR_CONF_REG_STR_LEN     12
#define TM_NT_INDENT_STR_LEN            4

/* IKE and IPsec related stuff */
#define TM_NT_SELECTOR_STR_LEN  \
        (TM_NT_SOCK_ADDR_STR_LEN * 2 + 10)
#define TM_NT_PROTECT_PROTOCOL_STR_LEN  8
#define TM_NT_PROTECT_MODE_STR_LEN      6
#define TM_NT_HASH_ALGORITHM_STR_LEN    6
#define TM_NT_ENCRYPT_ALGOR_STR_LEN     6
#define TM_NT_INTEGER_STR_LEN          10
#define TM_NT_DH_STR_LEN                4
#define TM_NT_DPD_STR_LEN              14

/* NAT related macros */
/*
 * Some triggers will print as 1.2.3.4:1234
 * and others will print as 1.2.3.4:11111-22222
 * The remote addr will be 1.2.3.4:1234
 */
#define TM_NT_NAT_ADDR_PORT_STR_LEN         15 + 12 + 1
#define TM_NT_NAT_ADDR_PORT_REMT_STR_LEN    15 + 6 + 1
#define TM_NT_NAT_TYPE_STR_LEN              16 + 1
#define TM_NT_NAT_FLAGS_STRL_LEN            8 + 1

#ifdef TM_COMPACT_NETSTAT

#ifdef TM_USE_IPV6
#undef TM_NT_IP_ADDR_STR_LEN
#define TM_NT_IP_ADDR_STR_LEN       26
#endif /* TM_USE_IPV6 */

#undef  TM_NT_INDEX_STR_LEN
#define TM_NT_INDEX_STR_LEN         4

#undef  TM_NT_Q_STR_LEN
#define TM_NT_Q_STR_LEN             8

#undef  TM_NT_RTO_STR_LEN
#define TM_NT_RTO_STR_LEN           5

#endif /* TM_COMPACT_NETSTAT */


/* string length of a single ARP entry */
#define TM_NT_ARP_TABLE_STR_LEN     \
        TM_NT_IP_ADDR_STR_LEN       \
      + TM_NT_HWADDR_STR_LEN        \
      + TM_NT_HWTYPE_STR_LEN        \
      + TM_NT_HWADDRLEN_STR_LEN     \
      + TM_NT_REFS_STR_LEN          \
      + TM_NT_IFACE_STR_LEN         \
      + TM_NT_TTL_STR_LEN           \
      + TM_NT_NUD_STATE_STR_LEN

/* string length of a single routing entry */
#define TM_NT_RTE_TABLE_STR_LEN     \
        TM_NT_KEY_IP_ADDR_STR_LEN   \
      + TM_NT_IP_ADDR_STR_LEN       \
      + TM_NT_REFS_STR_LEN          \
      + TM_NT_MHOME_STR_LEN         \
      + TM_NT_IFACE_STR_LEN         \
      + TM_NT_MTU_STR_LEN           \
      + TM_NT_HOPS_STR_LEN          \
      + TM_NT_TTL_STR_LEN           \
      + TM_NT_FLAGS_STR_LEN

/* string length of a single TCP socket entry */
#define TM_NT_TCP_TABLE_STR_LEN     \
      + TM_NT_INDEX_STR_LEN         \
      + TM_NT_FAMILY_STR_LEN        \
      + TM_NT_Q_STR_LEN             \
      + TM_NT_Q_STR_LEN             \
      + TM_NT_SOCK_ADDR_STR_LEN     \
      + TM_NT_SOCK_ADDR_STR_LEN     \
      + TM_NT_REFS_STR_LEN          \
      + TM_NT_STATE_BACKLOG_STR_LEN \
      + TM_NT_RTO_STR_LEN

/* string length of a single UDP socket entry */
#define TM_NT_UDP_TABLE_STR_LEN     \
      + TM_NT_INDEX_STR_LEN         \
      + TM_NT_FAMILY_STR_LEN        \
      + TM_NT_Q_STR_LEN             \
      + TM_NT_Q_STR_LEN             \
      + TM_NT_SOCK_ADDR_STR_LEN     \
      + TM_NT_REFS_STR_LEN  

/* string length of a single Binding entry */
#define TM_NT_BINDING_TABLE_STR_LEN     \
        TM_NT_IP_ADDR_STR_LEN           \
      + TM_NT_IP_ADDR_STR_LEN           \
      + TM_NT_TTL_STR_LEN               \
      + TM_NT_LAST_SEQ_STR_LEN          \
      + TM_NT_LAST_USED_TIME_STR_LEN    \
      + TM_NT_LAST_UPDATED_TIME_STR_LEN \
      + TM_NT_HOME_REG_STR_LEN          \
      + TM_NT_MHOME_STR_LEN

/* string length of a single device entry */
#define TM_NT_DEVICE_TABLE_STR_LEN  \
        TM_NT_IFACE_STR_LEN         \
      + TM_NT_HWADDR_STR_LEN        \
      + TM_NT_DEVICE_STATUS_STR_LEN

/* string length of a single NAT entry */
#define TM_NT_NAT_TABLE_STR_LEN             \
        TM_NT_NAT_ADDR_PORT_STR_LEN         \
      + TM_NT_NAT_ADDR_PORT_STR_LEN         \
      + TM_NT_NAT_ADDR_PORT_REMT_STR_LEN    \
      + TM_NT_TTL_STR_LEN                   \
      + TM_NT_NAT_TYPE_STR_LEN              \
      + TM_NT_NAT_TYPE_STR_LEN              \
      + TM_NT_NAT_FLAGS_STRL_LEN

/* If there are addresses configured on the device
 * each one will be outputted at a separated line
 */
#define TM_NT_DEVICE_ADDR_INFO_STR_LEN  \
        TM_NT_INDENT_STR_LEN            \
      + TM_NT_MHOME_STR_LEN             \
      + TM_NT_ADDR_CONF_TYPE_STR_LEN    \
      + TM_NT_IP_ADDR_STR_LEN           \
      + TM_NT_ADDR_CONF_STATUS_STR_LEN  \
      + TM_NT_ADDR_CONF_REG_STR_LEN

/* Lifetime values are integers */
#define TM_NT_ISASTATE_ENTRY_TABLE_STR_LEN  \
        (TM_NT_IP_ADDR_STR_LEN *2)          \
      + TM_NT_ENCRYPT_ALGOR_STR_LEN         \
      + TM_NT_HASH_ALGORITHM_STR_LEN        \
      + TM_NT_PROTECT_PROTOCOL_STR_LEN      \
      + TM_NT_DH_STR_LEN                    \
      + (TM_NT_INTEGER_STR_LEN *2)          \
      + TM_NT_DPD_STR_LEN

/* Lifetime values are integers */
#define TM_NT_ISASTATE_SPD_TABLE_STR_LEN    \
        (TM_NT_IP_ADDR_STR_LEN *2)          \
      + TM_NT_HASH_ALGORITHM_STR_LEN        \
      + TM_NT_ENCRYPT_ALGOR_STR_LEN         \
      + TM_NT_PROTECT_PROTOCOL_STR_LEN      \
      + TM_NT_DH_STR_LEN                    \
      + (TM_NT_INTEGER_STR_LEN *2)          \
      + (TM_NT_DPD_STR_LEN *4)

/* one policy may have two protocol suite */
#define TM_NT_IPSEC_SPD_TABLE_STR_LEN        \
        (TM_NT_SELECTOR_STR_LEN * 2) + 10    \
      + (TM_NT_PROTECT_PROTOCOL_STR_LEN * 2) \
      + (TM_NT_PROTECT_MODE_STR_LEN * 2)     \
      + (TM_NT_HASH_ALGORITHM_STR_LEN * 2)   \
      + TM_NT_ENCRYPT_ALGOR_STR_LEN
     
/* SPI, lifetime seconds, lifetime kbytes are integers */
#define TM_NT_SADB_TABLE_STR_LEN   \
        TM_NT_IP_ADDR_STR_LEN * 2    \
      + TM_NT_PROTECT_PROTOCOL_STR_LEN \
      + TM_NT_INTEGER_STR_LEN \
      + TM_NT_PROTECT_MODE_STR_LEN    \
      + TM_NT_HASH_ALGORITHM_STR_LEN \
      + TM_NT_ENCRYPT_ALGOR_STR_LEN  \
      + TM_NT_INTEGER_STR_LEN \
      + TM_NT_INTEGER_STR_LEN

/*
 * Compile time error checking to ensure that memory violation doesn't occur.
 */
#if TM_NT_ARP_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN
#error "TM_NT_ENTRY_STR_LEN must be bigger than TM_NT_ARP_TABLE_STR_LEN"
#endif /* TM_NT_ARP_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN */

#if TM_NT_RTE_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN
#error "TM_NT_ENTRY_STR_LEN must be bigger than TM_NT_RTE_TABLE_STR_LEN"
#endif /* TM_NT_RTE_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN */

#if TM_NT_TCP_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN
#error "TM_NT_ENTRY_STR_LEN must be bigger than TM_NT_TCP_TABLE_STR_LEN"
#endif /* TM_NT_TCP_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN */

#if TM_NT_UDP_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN
#error "TM_NT_ENTRY_STR_LEN must be bigger than TM_NT_UDP_TABLE_STR_LEN"
#endif /* TM_NT_UDP_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN */

#if TM_NT_DEVICE_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN
#error "TM_NT_ENTRY_STR_LEN must be bigger than TM_NT_DEVICE_TABLE_STR_LEN"
#endif /* TM_NT_DEVICE_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN */

#if TM_NT_DEVICE_ADDR_INFO_STR_LEN >= TM_NT_ENTRY_STR_LEN
#error "TM_NT_ENTRY_STR_LEN must be bigger than TM_NT_DEVICE_ADDR_INFO_STR_LEN"
#endif /* TM_NT_DEVICE_ADDR_INFO_STR_LEN >= TM_NT_ENTRY_STR_LEN */

#if TM_NT_BINDING_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN
#error "TM_NT_ENTRY_STR_LEN must be bigger than TM_NT_BINDING_TABLE_STR_LEN"
#endif /* TM_NT_BINDING_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN */

#ifdef TM_USE_IPV6

#if TM_6_MAX_MHOME > TM_NT_MAX_IP_PER_IF
#error "TM_NT_MAX_IP_PER_IF is too small"
#endif /* all IPS_PER_IF > TM_NT_MAX_IP_PER_IF */

#else /* !TM_USE_IPV6 */

#if TM_MAX_IPS_PER_IF > TM_NT_MAX_IP_PER_IF
#error "TM_NT_MAX_IP_PER_IF must be bigger than or equal to TM_MAX_IPS_PER_IF"
#endif /*  TM_MAX_IPS_PER_IF > TM_NT_MAX_IP_PER_IF */

#endif /* !TM_USE_IPV6 */

#ifdef TM_USE_IKE

#if TM_NT_ISASTATE_ENTRY_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN
#error "TM_NT_ENTRY_STR_LEN must be bigger than TM_NT_ISASTATE_ENTRY_TABLE_STR_LEN"
#endif /* TM_NT_ISASTATE_ENTRY_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN */

#endif /* TM_USE_IKE */

#if (defined(TM_USE_IPSEC) || defined(TM_USE_IKEV1))

#if TM_NT_ISASTATE_SPD_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN
#error "TM_NT_ENTRY_STR_LEN must be bigger than TM_NT_ISASTATE_SPD_TABLE_STR_LEN"
#endif /* TM_NT_ISASTATE_SPD_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN */

#endif /* TM_USE_IPSEC || TM_USE_IKEV1 */

#ifdef TM_USE_IPSEC

#if TM_NT_IPSEC_SPD_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN
#error "TM_NT_ENTRY_STR_LEN must be bigger than TM_NT_IPSEC_SPD_TABLE_STR_LEN"
#endif /* TM_NT_IPSEC_SPD_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN */

#if TM_NT_SADB_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN

#error "TM_NT_ENTRY_STR_LEN must be bigger than TM_NT_SADB_TABLE_STR_LEN"
#endif /* TM_NT_SADB_TABLE_STR_LEN >= TM_NT_ENTRY_STR_LEN */

#endif /* TM_USE_IPSEC */

/* local types */
typedef int (*ttNtGetFuncEntryPtr)(ttSnmpCacheEntryPtr snmpCacheEntryPtr, 
                                   ttNtEntryUPtr       ntEntryPtr);

/* internal function prototypes */

static int tfNtEnumEntry(
    ttNtTableId         tableId,
    int                 cacheIndex,
    ttNtEntryCBFuncPtr  ntEntryCBFuncPtr,
    ttUserGenericUnion  genParam1,
    ttUserGenericUnion  genParam2);

static int tfNtTableToCacheIndex(int family, ttNtTableId tableId);

#ifdef TM_LOCK_NEEDED
static int tfNtCallBack( ttLockEntryPtr      lockEntryPtr, 
                         ttNtEntryCBFuncPtr  ntEntryCBFuncPtr,
                         ttNtEntryUPtr       ntEntryPtr,
                         ttUserGenericUnion  genParam1,
                         ttUserGenericUnion  genParam2 );
#endif /* TM_LOCK_NEEDED */

/* 
 * printf TTl value to a string, 
 * the result is "INF" if the ttl value is TM_RTE_INF.
 */
static char TM_FAR * tfNtTtlToStr(
    ttUser32Bit     ttl, 
    char TM_FAR *   buffer);

/* printf sockaddr_storage in the format of ipv4:port or [ipv6]:port */
static int tfNtSockAddrToStr(
    struct sockaddr_storage TM_FAR *    sockAddrPtr,
    char TM_FAR *                       buffer);

/* 
 * printf sockaddr_storage and prefix length in the format of 
 * ipaddress / prefix length
 */
static int tfNtSockAddrNPrefixLenToStr(
    struct sockaddr_storage TM_FAR *    sockAddrPtr,
    int                                 prefixLen,
    char TM_FAR *                       buffer);

/*
 * print V4 and V4 mapped address in dot decimal format 
 * print V6 address in the Hex Colon format 
 */
static int tfNtSockAddrToIpAddrStr(
    struct sockaddr_storage TM_FAR *    sockAddrPtr,
    char TM_FAR *                       buffer);
/*
 * store V4 and V4 mapped address as V4 address into sockaddr_storage struct
 * store V6 address as V6 address into sockaddr_storage struct
 */
static void tfNtIpAddrToSockAddr(
    ttIpAddressPtr              ipAddrPtr,
#if defined(TM_USE_IPV6) && defined(TM_USE_IPV4)
    tt16Bit                     prefixLength,
#endif /* TM_USE_IPV6 && TM_USE_IPV4 */
    struct sockaddr_storage TM_FAR *   sockAddrPtr);

/* get the string for NUD state */
static const char TM_FAR * tfNtGetNudStateStr(tt8Bit state);

#ifdef TM_USE_IPV4
static char * tfNtNatTypeToStr(ttUser8Bit type, char TM_FAR * buffer);
#endif /* TM_USE_IPV4 */

/* local variable */
static const char TM_FAR *  TM_CONST_QLF tlHwTypeArray[] = 
                {"ether", "tring", "tbus", "null"};


/* 
 * Function tfNtGetArpHeaderStr()
 * DESCRIPTION:
 *  Get the ARP table header as a string.
 * PARAMETERS
 *  buffer:     the user provided buffer for storing the result string
 *  *sizePtr: size of the user provided buffer, must be no shorting than
 *              TM_NT_ARP_TABLE_STR_LEN
 *  sizePtr:  user provided integer pointer to carry back the actual
 *              string length, ignored if NULL
 * RETURNS:
 *  TM_ENOERROR:    successful
 *  TM_ENINVAL:     buffer is NULL or *sizePtr is too small.
 */
char TM_FAR * tfNtGetArpHeaderStr(
    char TM_FAR *   buffer, 
    int  TM_FAR *   sizePtr)
{
    char TM_FAR *   bufferBase;
    
    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_ARP_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);
        tm_nt_strncpy(buffer, "IPAddress",  TM_NT_IP_ADDR_STR_LEN);
        tm_nt_strncpy(buffer, "HwAddress",  TM_NT_HWADDR_STR_LEN);
        tm_nt_strncpy(buffer, "HWtype",     TM_NT_HWTYPE_STR_LEN);
        tm_nt_strncpy(buffer, "HWL",        TM_NT_HWADDRLEN_STR_LEN);
        tm_nt_strncpy(buffer, "Refs",       TM_NT_REFS_STR_LEN);
        tm_nt_strncpy(buffer, "Iface",      TM_NT_IFACE_STR_LEN);
        tm_nt_strncpy(buffer, "State",      TM_NT_NUD_STATE_STR_LEN);
        tm_nt_strncpy(buffer, "TTL",        TM_NT_TTL_STR_LEN);
        buffer[0] = '\0';

        *sizePtr = TM_NT_ARP_TABLE_STR_LEN;
    }
    return bufferBase;
}


/* 
 * Function tfNtGetRteHeaderStr()
 * DESCRIPTION:
 *  Get the routing table header as a string.
 * PARAMETERS
 *  buffer:     the user provided buffer for storing the result string
 *  *sizePtr: size of the user provided buffer, must be no shorting than
 *              TM_NT_RTE_TABLE_STR_LEN
 *  sizePtr:  user provided integer pointer to carry back the actual
 *              string length, ignored if NULL
 * RETURNS:
 *  TM_ENOERROR:    successful
 *  TM_ENINVAL:     buffer is NULL or *sizePtr is too small.
 */
char TM_FAR * tfNtGetRteHeaderStr(
    char TM_FAR *   buffer, 
    int  TM_FAR *   sizePtr)
{
    char TM_FAR *   bufferBase;

    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_RTE_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);
/* IP address / prefix length */
        tm_nt_strncpy(buffer, "Destination", TM_NT_KEY_IP_ADDR_STR_LEN);
/* IP ADDRESS */
        tm_nt_strncpy(buffer, "Gw",      TM_NT_IP_ADDR_STR_LEN);
/*  clone prefix length */
        tm_nt_strncpy(buffer, "Refs",     TM_NT_REFS_STR_LEN);
        tm_nt_strncpy(buffer, "Mhome",   TM_NT_MHOME_STR_LEN);
        tm_nt_strncpy(buffer, "Iface",   TM_NT_IFACE_STR_LEN);
        tm_nt_strncpy(buffer, "mtu",     TM_NT_MTU_STR_LEN);
        tm_nt_strncpy(buffer, "hops",    TM_NT_HOPS_STR_LEN);
        tm_nt_strncpy(buffer, "ttl",     TM_NT_TTL_STR_LEN);
        tm_nt_strncpy(buffer, "flags",   TM_NT_FLAGS_STR_LEN);
        buffer[0] = '\0';

        *sizePtr = TM_NT_RTE_TABLE_STR_LEN;
    }
    return bufferBase;
}


/* 
 * Function tfNtGetTcpHeaderStr()
 * PARAMETERS
 * DESCRIPTION:
 *  Get the TCP socket table header as a string.
 *  buffer:     the user provided buffer for storing the result string
 *  *sizePtr: size of the user provided buffer, must be no shorting than
 *              TM_NT_TCP_TABLE_STR_LEN
 *  sizePtr:  user provided integer pointer to carry back the actual
 *              string length, ignored if NULL
 * RETURNS:
 *  TM_ENOERROR:    successful
 *  TM_ENINVAL:     buffer is NULL or *sizePtr is too small.
 */
char TM_FAR * tfNtGetTcpHeaderStr(
    char TM_FAR * buffer, 
    int  TM_FAR * sizePtr)
{
    char TM_FAR *   bufferBase;

    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_TCP_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);
        tm_nt_strncpy(buffer, "Idx",    TM_NT_INDEX_STR_LEN);
        tm_nt_strncpy(buffer, "family", TM_NT_FAMILY_STR_LEN);
        tm_nt_strncpy(buffer, "Recv-Q", TM_NT_Q_STR_LEN);
        tm_nt_strncpy(buffer, "Send-Q", TM_NT_Q_STR_LEN);
        tm_nt_strncpy(buffer, "LocalAddress",   TM_NT_SOCK_ADDR_STR_LEN);
        tm_nt_strncpy(buffer, "ForeignAddress", TM_NT_SOCK_ADDR_STR_LEN);
        tm_nt_strncpy(buffer, "Refs",   TM_NT_REFS_STR_LEN);
        tm_nt_strncpy(buffer, "State/Backlog",  TM_NT_STATE_BACKLOG_STR_LEN);
        tm_nt_strncpy(buffer, "RTO",    TM_NT_RTO_STR_LEN);
        buffer[0] = '\0';

        *sizePtr = TM_NT_TCP_TABLE_STR_LEN;
    }
    return bufferBase;
}


/* 
 * Function tfNtGetUdpHeaderStr()
 * DESCRIPTION:
 *  Get the UDP socket table header as a string.
 * PARAMETERS:
 *  buffer:     the user provided buffer for storing the result string
 *  *sizePtr: size of the user provided buffer, must be no shorting than
 *              TM_NT_UDP_TABLE_STR_LEN
 *  sizePtr:  user provided integer pointer to carry back the actual
 *              string length, ignored if NULL
 * RETURNS:
 *  TM_ENOERROR:    successful
 *  TM_ENINVAL:     buffer is NULL or *sizePtr is too small.
 */
char TM_FAR * tfNtGetUdpHeaderStr(
    char TM_FAR *   buffer, 
    int  TM_FAR *   sizePtr)
{
    char TM_FAR *   bufferBase;

    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_UDP_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);
        tm_nt_strncpy(buffer, "Index",  TM_NT_INDEX_STR_LEN);
        tm_nt_strncpy(buffer, "family", TM_NT_FAMILY_STR_LEN);
        tm_nt_strncpy(buffer, "RecvQ", TM_NT_Q_STR_LEN);
        tm_nt_strncpy(buffer, "SendQ", TM_NT_Q_STR_LEN);
        tm_nt_strncpy(buffer, "LocalAddress", TM_NT_SOCK_ADDR_STR_LEN);
        tm_nt_strncpy(buffer, "Refs",   TM_NT_REFS_STR_LEN);
        buffer[0] = '\0';

        *sizePtr = TM_NT_UDP_TABLE_STR_LEN;
    }
    return bufferBase;
}


/* 
 * Function tfNtGetBindingHeaderStr()
 * DESCRIPTION:
 *  Get the BINDING socket table header as a string.
 * PARAMETERS:
 *  buffer:     the user provided buffer for storing the result string
 *  *sizePtr: size of the user provided buffer, must be no shorting than
 *              TM_NT_BINDING_TABLE_STR_LEN
 *  sizePtr:  user provided integer pointer to carry back the actual
 *              string length, ignored if NULL
 * RETURNS:
 *  TM_ENOERROR:    successful
 *  TM_ENINVAL:     buffer is NULL or *sizePtr is too small.
 */
#ifdef TM_6_USE_MIP_CN
char TM_FAR * tfNtGetBindingHeaderStr(
    char TM_FAR *   buffer, 
    int  TM_FAR *   sizePtr)
{
    char TM_FAR *   bufferBase;

    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_BINDING_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);
        tm_nt_strncpy(buffer, "HomeAddress",  TM_NT_IP_ADDR_STR_LEN);
        tm_nt_strncpy(buffer, "CareOfAddress",  TM_NT_IP_ADDR_STR_LEN);
        tm_nt_strncpy(buffer, "Ttl", TM_NT_TTL_STR_LEN);
        tm_nt_strncpy(buffer, "LastSeq", TM_NT_LAST_SEQ_STR_LEN);
        tm_nt_strncpy(buffer, "LastSendTime", TM_NT_LAST_USED_TIME_STR_LEN);
        tm_nt_strncpy(buffer, "LastRecvTime", TM_NT_LAST_USED_TIME_STR_LEN);
        tm_nt_strncpy(buffer, "LastUpdTime", TM_NT_LAST_UPDATED_TIME_STR_LEN);
#ifdef TM_6_USE_MIP_HA
        tm_nt_strncpy(buffer, "HomeReg", TM_NT_HOME_REG_STR_LEN);
        tm_nt_strncpy(buffer, "mHome", TM_NT_MHOME_STR_LEN);
#endif /* TM_6_USE_MIP_HA*/
        buffer[0] = '\0';

        *sizePtr = TM_NT_BINDING_TABLE_STR_LEN;
    }
    return bufferBase;
}
#endif /* TM_6_USE_MIP_CN */


/* 
 * Function tfNtGetDeviceHeaderStr()
 * DESCRIPTION:
 *  Get the DEVICE table header as a string.
 * PARAMETERS:
 *  buffer:     the user provided buffer for storing the result string
 *  *sizePtr:   size of the user provided buffer, must be no shorting than
 *              TM_NT_DEVICE_TABLE_STR_LEN
 *  sizePtr:    user provided integer pointer to carry back the actual
 *              string length, ignored if NULL
 * RETURNS:
 *  TM_ENOERROR:    successful
 *  TM_ENINVAL:     buffer is NULL or *sizePtr is too small.
 */
char TM_FAR * tfNtGetDeviceHeaderStr(
    char TM_FAR *   buffer, 
    int  TM_FAR *   sizePtr)
{
    char TM_FAR *   bufferBase;

    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_DEVICE_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
        *sizePtr = TM_NT_DEVICE_TABLE_STR_LEN;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);
        tm_nt_strncpy(buffer, "Name",     TM_NT_IFACE_STR_LEN);
        tm_nt_strncpy(buffer, "PhysAddr", TM_NT_HWADDR_STR_LEN);
        tm_nt_strncpy(buffer, "Status", TM_NT_DEVICE_STATUS_STR_LEN);

        buffer[0] = '\0';

        *sizePtr =  TM_NT_DEVICE_TABLE_STR_LEN;
    }
    return bufferBase;
}


char TM_FAR * tfNtGetAddrConfHeaderStr(
    char TM_FAR *   buffer, 
    int  TM_FAR *   sizePtr)
{
    char TM_FAR *   bufferBase;

    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_DEVICE_ADDR_INFO_STR_LEN) )
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);
        tm_nt_strncpy(buffer, "Family",  TM_NT_FAMILY_STR_LEN);
        tm_nt_strncpy(buffer, "mHome",  TM_NT_MHOME_STR_LEN);
        tm_nt_strncpy(buffer, "Type",   TM_NT_ADDR_CONF_TYPE_STR_LEN);
        tm_nt_strncpy(buffer, "IPAddress", TM_NT_KEY_IP_ADDR_STR_LEN);
        tm_nt_strncpy(buffer, "Status", TM_NT_ADDR_CONF_STATUS_STR_LEN);
        buffer[0] = '\0';

        *sizePtr = TM_NT_DEVICE_ADDR_INFO_STR_LEN;
    }
    return bufferBase;
}

/* 
 * Function tfNtArpEntryToStr()
 * DESCRIPTION: 
 *  print the entry as s string to the user provided buffer
 * PARAMETERS
 *  ntArpEntryPtr
          : pointer to the entry to be converted to string
 *  buffer:     the user provided buffer for storing the result string
 *  *sizePtr: size of the user provided buffer, must be no shorting than
 *              TM_NT_ARP_TABLE_STR_LEN
 *  sizePtr:  user provided integer pointer to carry back the actual
 *              string length, ignored if NULL
 * RETURNS:
 *  TM_ENOERROR:    successful
 *  TM_ENINVAL:     ntArpEntryPtr is NULL, 
 *                  buffer is NULL, 
 *                  or *sizePtr is too small.
 */
char TM_FAR * tfNtArpEntryToStr(
    ttNtArpEntryPtr ntArpEntryPtr, 
    char TM_FAR *   buffer, 
    int  TM_FAR *   sizePtr)
{
    int             fieldLen;
    char TM_FAR *   bufferBase;

    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_ARP_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);

        tfNtSockAddrToIpAddrStr(&ntArpEntryPtr->ntArpSockAddr, buffer);
/* remove the '\0'; */
        buffer[tm_strlen(buffer)] = ' ';
        buffer += TM_NT_IP_ADDR_STR_LEN;

        tfNtHwAddrToStr(ntArpEntryPtr->ntArpHwAddress,
                        ntArpEntryPtr->ntArpHwLength,
                        buffer);
/* remove the '\0'; */
        buffer[tm_strlen(buffer)] = ' ';
        buffer += TM_NT_HWADDR_STR_LEN;

        tm_nt_strncpy(buffer,  
                      tlHwTypeArray[ntArpEntryPtr->ntArpHwType],
                      TM_NT_HWTYPE_STR_LEN);

        fieldLen = tm_sprintf(buffer, "%d", ntArpEntryPtr->ntArpHwLength);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_HWADDRLEN_STR_LEN;

        fieldLen = tm_sprintf(buffer, 
                              "%lu", 
                              ntArpEntryPtr->ntArpOwnerCount);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_REFS_STR_LEN;

        tm_nt_strncpy(buffer,  
                      ntArpEntryPtr->ntArpDeviceName,
                      TM_NT_IFACE_STR_LEN);

        tm_nt_strncpy(buffer,  
                      tfNtGetNudStateStr(ntArpEntryPtr->ntArpNudState),
                      TM_NT_NUD_STATE_STR_LEN);

        tfNtTtlToStr(ntArpEntryPtr->ntArpTtl, buffer);
        buffer += TM_NT_TTL_STR_LEN;

        buffer[0] = '\0';

        *sizePtr = TM_NT_ARP_TABLE_STR_LEN;
    }
    return bufferBase;
}


/* 
 * Function tfNtRteEntryToStr()
 * DESCRIPTION: 
 *  print the entry as s string to the user provided buffer
 * PARAMETERS
 *  ntRteEntryPtr
          : pointer to the entry to be converted to string
 *  buffer:     the user provided buffer for storing the result string
 *  *sizePtr: size of the user provided buffer, must be no shorting than
 *              TM_NT_RTE_TABLE_STR_LEN
 *  sizePtr:  user provided integer pointer to carry back the actual
 *              string length, ignored if NULL
 * RETURNS:
 *  TM_ENOERROR:    successful
 *  TM_ENINVAL:     ntArpEntryPtr is NULL, 
 *                  buffer is NULL, 
 *                  or *sizePtr is too small.
 */
char TM_FAR * tfNtRteEntryToStr(
    ttNtRteEntryPtr ntRteEntryPtr, 
    char TM_FAR *   buffer, 
    int  TM_FAR *   sizePtr)
{
    int             fieldLen;
    char TM_FAR *   bufferBase;

    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_RTE_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);

        tfNtSockAddrNPrefixLenToStr(
                            &ntRteEntryPtr->ntRteDestSockAddr, 
                            ntRteEntryPtr->ntRtePrefixLength, 
                            buffer);
        buffer += TM_NT_KEY_IP_ADDR_STR_LEN; 
/*
 * get the string for the gateway filed , it cold be
 * '*'  : if the entry is clonable
 * '-'  : if not clonable and TM_NT_RTE_LINK_LAYER is set 
 * gw ipaddress : otherwise
 */
        if (ntRteEntryPtr->ntRteFlags & TM_NT_RTE_CLONABLE)
        {
            buffer[0] = '*';
        }
        else if(ntRteEntryPtr->ntRteFlags & TM_NT_RTE_LINK_LAYER)
        {
            buffer[0] = '-';
        }
        else
        {
            tfNtSockAddrToIpAddrStr(
                    &ntRteEntryPtr->ntRteGwSockAddr, buffer);
            buffer[tm_strlen(buffer)] = ' ';
        }

        buffer += TM_NT_IP_ADDR_STR_LEN;;    

        fieldLen = tm_sprintf(buffer, 
                              "%lu", 
                              ntRteEntryPtr->ntRteOwnerCount);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_REFS_STR_LEN;

        fieldLen = tm_sprintf(buffer, 
                              "%lu", 
                              ntRteEntryPtr->ntRteMhomeIndex);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_MHOME_STR_LEN;

        tm_nt_strncpy(buffer,  
                      ntRteEntryPtr->ntRteDeviceName,
                      TM_NT_IFACE_STR_LEN);

        fieldLen = tm_sprintf(buffer, "%lu", ntRteEntryPtr->ntRteMtu);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_MTU_STR_LEN;

        fieldLen = tm_sprintf(buffer, "%lu", ntRteEntryPtr->ntRteHops);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_HOPS_STR_LEN;

        tfNtTtlToStr(ntRteEntryPtr->ntRteTtl, buffer);
        buffer += TM_NT_TTL_STR_LEN;

        tfNtRteFlagToStr(ntRteEntryPtr->ntRteFlags, buffer);
        buffer += TM_NT_FLAGS_STR_LEN;

        buffer[0] = '\0';

        *sizePtr = TM_NT_RTE_TABLE_STR_LEN;
    }
    return bufferBase;
}


/* 
 * Function tfNtTcpEntryToStr()
 * DESCRIPTION: 
 *  print the entry as s string to the user provided buffer
 * PARAMETERS
 *  ntTcpEntryPtr
          : pointer to the entry to be converted to string
 *  buffer:     the user provided buffer for storing the result string
 *  *sizePtr: size of the user provided buffer, must be no shorting than
 *              TM_NT_TCP_TABLE_STR_LEN
 *  sizePtr:  user provided integer pointer to carry back the actual
 *              string length, ignored if NULL
 * RETURNS:
 *  TM_ENOERROR:    successful
 *  TM_ENINVAL:     ntArpEntryPtr is NULL, 
 *                  buffer is NULL, 
 *                  or *sizePtr is too small.
 */
char TM_FAR * tfNtTcpEntryToStr(
    ttNtTcpEntryPtr ntTcpEntryPtr, 
    char TM_FAR *   buffer, 
    int  TM_FAR *   sizePtr)
{
    int             fieldLen;
    char TM_FAR *   bufferBase;

    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_TCP_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);

        if( ntTcpEntryPtr->ntTcpSockDesc == TM_SOC_NO_INDEX)
        {
            fieldLen = tm_sprintf(buffer, "%s", "*");
        }
        else
        {
            fieldLen = tm_sprintf(buffer, "%u", ntTcpEntryPtr->ntTcpSockDesc);
        }
        buffer[fieldLen] = ' ';
        buffer += TM_NT_INDEX_STR_LEN;

/*
 * V4 mapped address for a socket entry is stored as a V4 address 
 * in ntTcpEntryPtr->ntTcpLocalSockAddr 
 */
#ifdef TM_USE_IPV6
        if(ntTcpEntryPtr->ntTcpLocalSockAddr.ss_family == AF_INET6)
        {
            tm_nt_strncpy(buffer, "IPV6", TM_NT_FAMILY_STR_LEN);
        }
        else
        {
            tm_nt_strncpy(buffer, "IPV4", TM_NT_FAMILY_STR_LEN);
        }
#else /*TM_USE_IPV6*/
            tm_nt_strncpy(buffer, "IPV4", TM_NT_FAMILY_STR_LEN);
#endif /*TM_USE_IPV6*/



        fieldLen = tm_sprintf(buffer, 
                              "%lu/%lu ", 
                              ntTcpEntryPtr->ntTcpBytesInRecvQ,
                              ntTcpEntryPtr->ntTcpRecvQSize);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_Q_STR_LEN;

        fieldLen = tm_sprintf(buffer, 
                              "%lu/%lu",
                              ntTcpEntryPtr->ntTcpBytesInSendQ,
                              ntTcpEntryPtr->ntTcpSendQSize);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_Q_STR_LEN;

        tfNtSockAddrToStr(&ntTcpEntryPtr->ntTcpLocalSockAddr, buffer);
        buffer += TM_NT_SOCK_ADDR_STR_LEN;

        tfNtSockAddrToStr(&ntTcpEntryPtr->ntTcpPeerSockAddr, buffer);
        buffer += TM_NT_SOCK_ADDR_STR_LEN;

        fieldLen = tm_sprintf(buffer, 
                              "%lu", 
                              ntTcpEntryPtr->ntTcpOwnerCount);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_REFS_STR_LEN;

/* state/backlog */
        tm_strcpy(buffer, tfNtGetTcpStateStr(ntTcpEntryPtr->ntTcpState));
        if(ntTcpEntryPtr->ntTcpState == TM_NT_TCPS_LISTEN)
        {
/* print backlog only if the socket is a listening socket */
            tm_sprintf(buffer + tm_strlen(buffer), 
                       "/%u", 
                       ntTcpEntryPtr->ntTcpBackLog);
        }
        buffer[tm_strlen(buffer)] = ' ';
        buffer += TM_NT_STATE_BACKLOG_STR_LEN;

        fieldLen = tm_sprintf(buffer, 
                              "%lu", 
                              ntTcpEntryPtr->ntTcpRto);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_RTO_STR_LEN;
        buffer[0] = '\0';

        *sizePtr = TM_NT_TCP_TABLE_STR_LEN;
    }
    return bufferBase;
}

/* 
 * Function tfNtUdpEntryToStr()
 * DESCRIPTION: 
 *  print the entry as s string to the user provided buffer
 * PARAMETERS
 *  ntUdpEntryPtr
          : pointer to the entry to be converted to string
 *  buffer:     the user provided buffer for storing the result string
 *  *sizePtr: size of the user provided buffer, must be no shorting than
 *              TM_NT_UDP_TABLE_STR_LEN
 *  sizePtr:  user provided integer pointer to carry back the actual
 *              string length, ignored if NULL
 * RETURNS:
 *  TM_ENOERROR:    successful
 *  TM_ENINVAL:     ntArpEntryPtr is NULL, 
 *                  buffer is NULL, 
 *                  or *sizePtr is too small.
 */
char TM_FAR * tfNtUdpEntryToStr(
    ttNtUdpEntryPtr ntUdpEntryPtr, 
    char TM_FAR *   buffer, 
    int  TM_FAR *   sizePtr)
{
    int             fieldLen;
    char TM_FAR *   bufferBase;

    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_UDP_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);


        if( ntUdpEntryPtr->ntUdpSockDesc == TM_SOC_NO_INDEX)
        {
            fieldLen = tm_sprintf(buffer, "%d", -1);
        }
        else
        {
            fieldLen = tm_sprintf(buffer, "%u", ntUdpEntryPtr->ntUdpSockDesc);
        }


        fieldLen = tm_sprintf(buffer, "%u", ntUdpEntryPtr->ntUdpSockDesc);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_INDEX_STR_LEN;

/*
 * V4 mapped address for a socket entry is stored as a V4 address 
 * in ntUdpEntryPtr->ntUdpLocalSockAddr 
 */
        if(ntUdpEntryPtr->ntUdpLocalSockAddr.ss_family == AF_INET)
        {
            tm_nt_strncpy(buffer, "IPV4", TM_NT_FAMILY_STR_LEN);
        }
        else if(ntUdpEntryPtr->ntUdpLocalSockAddr.ss_family == AF_INET6)
        {
            tm_nt_strncpy(buffer, "IPV6", TM_NT_FAMILY_STR_LEN);
        }

        fieldLen = tm_sprintf(buffer, 
                              "%lu/%lu ", 
                              ntUdpEntryPtr->ntUdpBytesInRecvQ,
                              ntUdpEntryPtr->ntUdpRecvQSize);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_Q_STR_LEN;

        fieldLen = tm_sprintf(buffer, 
                              "%lu/%lu",
                              ntUdpEntryPtr->ntUdpBytesInSendQ,
                              ntUdpEntryPtr->ntUdpSendQSize);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_Q_STR_LEN;

        tfNtSockAddrToStr(&ntUdpEntryPtr->ntUdpLocalSockAddr, buffer);
        buffer += TM_NT_SOCK_ADDR_STR_LEN;

        fieldLen = tm_sprintf(buffer, 
                              "%u", 
                              ntUdpEntryPtr->ntUdpOwnerCount);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_REFS_STR_LEN;
        buffer[0] = '\0';

        *sizePtr = TM_NT_UDP_TABLE_STR_LEN;
    }
    return bufferBase;
}

#ifdef TM_USE_IKE

int tfNtGetIsaStateEntry(
    ttIsakmpStatePtr        isaStatePtr, 
    ttNtIsaStateEntryPtr    ntIsaStateEntryPtr)
{
    int                  errorCode;

    if (!isaStatePtr || !ntIsaStateEntryPtr)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        errorCode = TM_ENOERROR;
        tm_bzero(ntIsaStateEntryPtr, sizeof(ttNtIsaStateEntry));
        tm_bcopy(&isaStatePtr->isaPeerIp,
                 &ntIsaStateEntryPtr->ntIsaStateSource,
                 sizeof(struct sockaddr_storage));
        tm_bcopy(&isaStatePtr->isaPeerIp,
                 &ntIsaStateEntryPtr->ntIsaStateDestination,
                 sizeof(struct sockaddr_storage));
        
    }

    return errorCode;
}

/* 
 * Function tfNtGetIsaStateEntryHeaderStr()
 * DESCRIPTION:
 *  Get the IKE SA table header as a string.
 * PARAMETERS
 *  buffer:     the user provided buffer for storing the result string
 *  *sizePtr:   size of the user provided buffer, must be no shorter than
 *              TM_NT_ISASTATE_ENTRY_TABLE_STR_LEN
 * RETURNS:
 *  TM_ENOERROR:    successful
 *  TM_ENINVAL:     buffer is NULL or *sizePtr is too small.
 */

char TM_FAR * tfNtGetIsaStateEntryHeaderStr(
    char TM_FAR *   buffer, 
    int  TM_FAR *   sizePtr)
{

    char TM_FAR *   bufferBase;
    
    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_ISASTATE_ENTRY_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);
        tm_nt_strncpy(buffer, "Source IP",    TM_NT_IP_ADDR_STR_LEN);
        tm_nt_strncpy(buffer, "Dest. IP",     TM_NT_IP_ADDR_STR_LEN);
        tm_nt_strncpy(buffer, "Enc",          TM_NT_ENCRYPT_ALGOR_STR_LEN);
        tm_nt_strncpy(buffer, "Hash",         TM_NT_HASH_ALGORITHM_STR_LEN);
        tm_nt_strncpy(buffer, "Auth",         TM_NT_PROTECT_PROTOCOL_STR_LEN);
        tm_nt_strncpy(buffer, "DH",           TM_NT_DH_STR_LEN);
        tm_nt_strncpy(buffer, "Lifetime(s)",  TM_NT_INTEGER_STR_LEN);
        tm_nt_strncpy(buffer, "Lifetime(KB)", TM_NT_INTEGER_STR_LEN);
#ifdef TM_USE_IKE_DPD
        tm_nt_strncpy(buffer, "DPD Retries",  TM_NT_DPD_STR_LEN);
#endif /* TM_USE_IKE_DPD */
        buffer[0] = '\0';

        *sizePtr = TM_NT_ISASTATE_ENTRY_TABLE_STR_LEN;
    }
    return bufferBase;
}

char TM_FAR * tfNtIsaStateEntryToStr(
                ttNtIsaStateEntryPtr  ntIsaStateEntryPtr, 
                char TM_FAR *         buffer, 
                int  TM_FAR *         sizePtr)
{
    int             af;
    int             fieldLen;
    char TM_FAR *   bufferBase;

    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_IPSEC_SPD_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        fieldLen = 0;
        tm_memset(buffer, ' ', *sizePtr);
/* the source IP */
        af = ntIsaStateEntryPtr->ntIsaStateSource.ss_family;
#ifdef TM_USE_IPV4
        if(af == AF_INET)
        {
            inet_ntop(af,
                      &ntIsaStateEntryPtr->ntIsaStateSource.addr.ipv4.sin_addr,
                      buffer,
                      INET_ADDRSTRLEN);
            fieldLen = (int) tm_strlen(buffer);
        }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
        else
#endif /* TM_USE_IPV4 */
            if(af == AF_INET6)
        {
            inet_ntop(af,
                      &ntIsaStateEntryPtr->ntIsaStateSource.addr.ipv6.sin6_addr,
                      buffer,
                      INET6_ADDRSTRLEN);
            fieldLen = (int) tm_strlen(buffer);
        }
#endif /* TM_USE_IPV6*/
        else
        {
            fieldLen = tm_sprintf(buffer, "any, ");
        }
        buffer[fieldLen] = ' ';
/* the destination ip */
        buffer += TM_NT_IP_ADDR_STR_LEN;
        af = ntIsaStateEntryPtr->ntIsaStateSource.ss_family;
#ifdef TM_USE_IPV4
        if(af == AF_INET)
        {
            inet_ntop(af,
                      &ntIsaStateEntryPtr->ntIsaStateSource.addr.ipv4.sin_addr,
                      buffer,
                      INET_ADDRSTRLEN);
            fieldLen = (int) tm_strlen(buffer);
        }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
        else
#endif /* TM_USE_IPV4 */
            if(af == AF_INET6)
        {
            inet_ntop(af,
                      &ntIsaStateEntryPtr->ntIsaStateSource.addr.ipv6.sin6_addr,
                      buffer,
                      INET6_ADDRSTRLEN);
            fieldLen = (int) tm_strlen(buffer);
        }
#endif /* TM_USE_IPV6 */
        else
        {
            fieldLen = tm_sprintf(buffer, "any, ");
        }
        buffer[fieldLen] = ' ';

/* protection protocol */
        buffer += TM_NT_IP_ADDR_STR_LEN;
        if(ntIsaStateEntryPtr->ntIsaStateProtect == TM_IP_AH)
        {
            fieldLen = tm_sprintf(buffer, "ah");
        }
        else if(ntIsaStateEntryPtr->ntIsaStateProtect == TM_IP_ESP)
        {
            fieldLen = tm_sprintf(buffer, "esp");
        }
        buffer[fieldLen] = ' ';

/* protect mode */
        buffer += TM_NT_PROTECT_PROTOCOL_STR_LEN;
        if(ntIsaStateEntryPtr->ntIsaStateMode == SADB_MODE_TRANSPORT)
        {
            fieldLen = tm_sprintf(buffer, "trans");
        }
        else if(ntIsaStateEntryPtr->ntIsaStateMode == SADB_MODE_TUNNEL)
        {
            fieldLen = tm_sprintf(buffer, "tunnl");
        }
        else if(ntIsaStateEntryPtr->ntIsaStateMode == SADB_MODE_MIPV6TUNNEL)
        {
            fieldLen = tm_sprintf(buffer, "miptun");
        }
        buffer[fieldLen] = ' ';
/* hash algorithm */
        if(ntIsaStateEntryPtr->ntIsaStateHash
                                                == TM_IKE_MD5)
        {
            fieldLen = tm_sprintf(buffer, "md5");
        }
        else if(ntIsaStateEntryPtr->ntIsaStateHash
                                                == TM_IKE_SHA1)
        {
            fieldLen = tm_sprintf(buffer, "sha1");
        }
        else if(ntIsaStateEntryPtr->ntIsaStateHash
                                                == TM_IKE_SHA256)
        {
            fieldLen = tm_sprintf(buffer, "sha256");
        }
        else if(ntIsaStateEntryPtr->ntIsaStateHash
                                                == TM_IKE_SHA384)
        {
            fieldLen = tm_sprintf(buffer, "sha384");
        }
        else if(ntIsaStateEntryPtr->ntIsaStateHash
                                                == TM_IKE_SHA512)
        {
            fieldLen = tm_sprintf(buffer, "sha512");
        }
        else if(ntIsaStateEntryPtr->ntIsaStateHash
                                                == TM_IKE_TIGER)
        {
            fieldLen = tm_sprintf(buffer, "tiger");
        }
        else if(ntIsaStateEntryPtr->ntIsaStateHash
                                                == TM_IKE_RIPEMD)
        {
            fieldLen = tm_sprintf(buffer, "ripe");
        }
        else
        {
            fieldLen = tm_sprintf(buffer, "null?");
        }
        buffer[fieldLen] = ' ';

/* encryption algorithm */
        buffer = bufferBase + 2*TM_NT_IP_ADDR_STR_LEN 
                            + TM_NT_HASH_ALGORITHM_STR_LEN;
        if(ntIsaStateEntryPtr->ntIsaStateEncrypt
                                                == TM_IKE_DES_CBC)
        {
            fieldLen = tm_sprintf(buffer, "des");
        }
        else if(ntIsaStateEntryPtr->ntIsaStateEncrypt
                                                == TM_IKE_IDEA_CBC)
        {
            fieldLen = tm_sprintf(buffer, "idea");
        }
        else if(ntIsaStateEntryPtr->ntIsaStateEncrypt
                                                == TM_IKE_RC5_R16_B64_CBC)
        {
            fieldLen = tm_sprintf(buffer, "RC5_R16_B64");
        }
        else if(ntIsaStateEntryPtr->ntIsaStateEncrypt
                                                == TM_IKE_BLOWFISH_CBC)
        {
            fieldLen = tm_sprintf(buffer, "blwfh");
        }
        else if(ntIsaStateEntryPtr->ntIsaStateEncrypt
                                                == TM_IKE_3DES_CBC)
        {
            fieldLen = tm_sprintf(buffer, "3des");
        }
        else if(ntIsaStateEntryPtr->ntIsaStateEncrypt
                                                == TM_IKE_CAST_CBC)
        {
            fieldLen = tm_sprintf(buffer, "cast");
        }
        else if(ntIsaStateEntryPtr->ntIsaStateEncrypt
                                                == TM_IKE_AES_CBC)
        {
            fieldLen = tm_sprintf(buffer, "aes");
        }
        else if(ntIsaStateEntryPtr->ntIsaStateEncrypt
                                                == TM_IKE_NULL_ENCRYPT)
        {
            fieldLen = tm_sprintf(buffer, "null");
        }
        else if(ntIsaStateEntryPtr->ntIsaStateEncrypt
                                                == TM_IKE_TWOFISH_CBC)
        {
            fieldLen = tm_sprintf(buffer, "twofh");
        }
        
        buffer[fieldLen] = ' ';
/* seconds lifetime  */
        buffer += TM_NT_ENCRYPT_ALGOR_STR_LEN;
        fieldLen = tm_sprintf(buffer, "%lu", 
                                ntIsaStateEntryPtr->ntIsaStateTtlSeconds);
        buffer[fieldLen] = ' ';
/* kbytes */
        buffer += TM_NT_INTEGER_STR_LEN;
        fieldLen = tm_sprintf(buffer, "%lu", 
                                ntIsaStateEntryPtr->ntIsaStateTtlKbytes);
        buffer[fieldLen] = '\0';

        *sizePtr = (int) tm_strlen(bufferBase);
    }

    return bufferBase;
}

int tfNtGetIsaStateSpdEntry(
    ttIkeEntryPtr               ikePolicyHeadPtr,
    int                         i,
    ttNtIsaStateSpdEntryPtr     ntIsaStateSpdEntryPtr)
{
    int                  errorCode;

    if (!ikePolicyHeadPtr || !ntIsaStateSpdEntryPtr)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        errorCode = TM_ENOERROR;
        tm_bzero(ntIsaStateSpdEntryPtr, sizeof(ttNtIsaStateSpdEntry));
        tm_bcopy(&ikePolicyHeadPtr->ikePlcyPtr->ikpSelector.selLocIp1,
                 &ntIsaStateSpdEntryPtr->ntIsaStateSpdSource,
                 sizeof(struct sockaddr_storage));
        tm_bcopy(&ikePolicyHeadPtr->ikePlcyPtr->ikpSelector.selRemtIp1, 
                 &ntIsaStateSpdEntryPtr->ntIsaStateSpdDestination,
                 sizeof(struct sockaddr_storage));

        ntIsaStateSpdEntryPtr->ntIsaStateSpdTransCount =
                                ikePolicyHeadPtr->ikeTransCount;

        ntIsaStateSpdEntryPtr->ntIsaStateSpdEncrypt =
                        ikePolicyHeadPtr->ikeTransDataPtr[i]->transEncryptAlg[0];
        ntIsaStateSpdEntryPtr->ntIsaStateSpdHash = 
                        ikePolicyHeadPtr->ikeTransDataPtr[i]->transHashAlg[0];
        ntIsaStateSpdEntryPtr->ntIsaStateSpdAuth = 
                        ikePolicyHeadPtr->ikeTransDataPtr[i]->transAuthMethod;
        ntIsaStateSpdEntryPtr->ntIsaStateSpdKeyLen =
                        ikePolicyHeadPtr->ikeTransDataPtr[i]->transKeyLength[0];
        ntIsaStateSpdEntryPtr->ntIsaStateSpdDhGroup =
                        ikePolicyHeadPtr->ikeTransDataPtr[i]->transDhgroup[0];
        ntIsaStateSpdEntryPtr->ntIsaStateTtlSeconds =
                        ikePolicyHeadPtr->ikeTransDataPtr[i]->transLifeSeconds;
        ntIsaStateSpdEntryPtr->ntIsaStateTtlKbytes =
                        ikePolicyHeadPtr->ikeTransDataPtr[i]->transLifeKbytes;

#ifdef TM_USE_IKE_DPD
        ntIsaStateSpdEntryPtr->ntIsaStateSpdDpdWorryMetric = 
                        ikePolicyHeadPtr->ikePlcyPtr->ikpDpdWorryMetric;
        ntIsaStateSpdEntryPtr->ntIsaStateSpdDpdReXmitDelay = 
                        ikePolicyHeadPtr->ikePlcyPtr->ikpDpdRetransDelay;
        ntIsaStateSpdEntryPtr->ntIsaStateSpdDpdReXmitLimit = 
                        ikePolicyHeadPtr->ikePlcyPtr->ikpDpdRetransLimit;
        ntIsaStateSpdEntryPtr->ntIsaStateSpdDpdMode = 
                        ikePolicyHeadPtr->ikePlcyPtr->ikpDpdMode;
#endif /* TM_USE_IKE_DPD */
    }

    return errorCode;
}

/* 
 * Function tfNtGetIsaStateSpdHeaderStr()
 * DESCRIPTION:
 *  Get the IKE SPD table header as a string.
 * PARAMETERS
 *  buffer:     the user provided buffer for storing the result string
 *  *sizePtr:   size of the user provided buffer, must be no shorter than
 *              TM_NT_ISASTATE_SPD_TABLE_STR_LEN
 * RETURNS:
 *  TM_ENOERROR:    successful
 *  TM_ENINVAL:     buffer is NULL or *sizePtr is too small.
 */

char TM_FAR * tfNtGetIsaStateSpdHeaderStr(
    char TM_FAR *   buffer, 
    int  TM_FAR *   sizePtr)
{

    char TM_FAR *   bufferBase;
    
    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_ISASTATE_SPD_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);
        tm_nt_strncpy(buffer, "Source IP",           TM_NT_IP_ADDR_STR_LEN);
        tm_nt_strncpy(buffer, "Dest. IP",            TM_NT_IP_ADDR_STR_LEN);
        tm_nt_strncpy(buffer, "Hash",         TM_NT_HASH_ALGORITHM_STR_LEN);
        tm_nt_strncpy(buffer, "Enc",           TM_NT_ENCRYPT_ALGOR_STR_LEN);
        tm_nt_strncpy(buffer, "Auth",       TM_NT_PROTECT_PROTOCOL_STR_LEN);
        tm_nt_strncpy(buffer, "DH",                       TM_NT_DH_STR_LEN);
        tm_nt_strncpy(buffer, "Life(s)",             TM_NT_INTEGER_STR_LEN);
        tm_nt_strncpy(buffer, "Life(KB)",            TM_NT_INTEGER_STR_LEN);
#ifdef TM_USE_IKE_DPD
        tm_nt_strncpy(buffer, "DPD WM(s)",               TM_NT_DPD_STR_LEN);
        tm_nt_strncpy(buffer, "DPD Delay(ms)",           TM_NT_DPD_STR_LEN);
        tm_nt_strncpy(buffer, "DPD Limit",               TM_NT_DPD_STR_LEN);
        tm_nt_strncpy(buffer, "DPD Mode",                TM_NT_DPD_STR_LEN);
#endif /* TM_USE_IKE_DPD */
        buffer[0] = '\0';

        *sizePtr = TM_NT_ISASTATE_SPD_TABLE_STR_LEN;
    }
    return bufferBase;
}

char TM_FAR * tfNtIsaStateSpdEntryToStr(
                ttNtIsaStateSpdEntryPtr ntIsaStateSpdEntryPtr,
                char TM_FAR *           buffer, 
                int  TM_FAR *           sizePtr)
{
    int             af;
    int             fieldLen;
    char TM_FAR *   bufferBase;

    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_ISASTATE_SPD_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        fieldLen = 0;
        tm_memset(buffer, ' ', *sizePtr);
/* the selector information */
        af = ntIsaStateSpdEntryPtr->ntIsaStateSpdSource.ss_family;
#ifdef TM_USE_IPV4
        if(af == AF_INET)
        {
            inet_ntop(af,
                      &ntIsaStateSpdEntryPtr->
                                    ntIsaStateSpdSource.addr.ipv4.sin_addr,
                      buffer,
                      INET_ADDRSTRLEN);
        }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
        else
#endif /* TM_USE_IPV4 */
            if(af == AF_INET6)
        {
            inet_ntop(af,
                      &ntIsaStateSpdEntryPtr->
                                ntIsaStateSpdSource.addr.ipv6.sin6_addr,
                      buffer,
                      INET6_ADDRSTRLEN);
        }
#endif /* TM_USE_IPV6 */
        else
        {
            fieldLen = tm_sprintf(buffer, "any\t");
        }

        if(af == AF_INET || af == AF_INET6)
        {
            fieldLen = (int) tm_strlen(buffer);
            buffer[fieldLen] = ' ';
            buffer += fieldLen;
            fieldLen = tm_sprintf(buffer, 
                          ":%d",
                          ntIsaStateSpdEntryPtr->ntIsaStateSpdSource.ss_port);
        }

        buffer[fieldLen] = ' ';
        buffer = bufferBase + TM_NT_IP_ADDR_STR_LEN;
        af = ntIsaStateSpdEntryPtr->ntIsaStateSpdDestination.ss_family;
#ifdef TM_USE_IPV4
        if(af == AF_INET)
        {
            inet_ntop(af,
                      &ntIsaStateSpdEntryPtr->
                                ntIsaStateSpdDestination.addr.ipv4.sin_addr,
                      buffer,
                      INET_ADDRSTRLEN);
        }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
        else
#endif /* TM_USE_IPV4 */
            if(af == AF_INET6)
        {
            inet_ntop(af,
                      &ntIsaStateSpdEntryPtr->
                                ntIsaStateSpdDestination.addr.ipv6.sin6_addr,
                      buffer,
                      INET6_ADDRSTRLEN);
        }
#endif /* TM_USE_IPV6 */
        else
        {
            fieldLen = tm_sprintf(buffer, "any\t");
        }

        if(af == AF_INET || af == AF_INET6)
        {
            fieldLen = (int) tm_strlen(buffer);
            buffer[fieldLen] = ' ';
            buffer += fieldLen;
            fieldLen = tm_sprintf(buffer, 
                              ":%d",
                              ntIsaStateSpdEntryPtr->
                                        ntIsaStateSpdDestination.ss_port);
        }
        buffer[fieldLen] = ' ';
        buffer = bufferBase + 2*TM_NT_IP_ADDR_STR_LEN;
/* hash algorithm */
        if(ntIsaStateSpdEntryPtr->ntIsaStateSpdHash
                                                == TM_IKE_MD5)
        {
            fieldLen = tm_sprintf(buffer, "md5");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdHash
                                                == TM_IKE_SHA1)
        {
            fieldLen = tm_sprintf(buffer, "sha1");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdHash
                                                == TM_IKE_SHA256)
        {
            fieldLen = tm_sprintf(buffer, "sha256");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdHash
                                                == TM_IKE_SHA384)
        {
            fieldLen = tm_sprintf(buffer, "sha384");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdHash
                                                == TM_IKE_SHA512)
        {
            fieldLen = tm_sprintf(buffer, "sha512");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdHash
                                                == TM_IKE_TIGER)
        {
            fieldLen = tm_sprintf(buffer, "tiger");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdHash
                                                == TM_IKE_RIPEMD)
        {
            fieldLen = tm_sprintf(buffer, "ripe");
        }
        else
        {
            fieldLen = tm_sprintf(buffer, "null?");
        }
        buffer[fieldLen] = ' ';

/* encryption algorithm */
        buffer = bufferBase + 2*TM_NT_IP_ADDR_STR_LEN 
                            + TM_NT_HASH_ALGORITHM_STR_LEN;
        if(ntIsaStateSpdEntryPtr->ntIsaStateSpdEncrypt
                                                == TM_IKE_DES_CBC)
        {
            fieldLen = tm_sprintf(buffer, "des");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdEncrypt
                                                == TM_IKE_IDEA_CBC)
        {
            fieldLen = tm_sprintf(buffer, "idea");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdEncrypt
                                                == TM_IKE_RC5_R16_B64_CBC)
        {
            fieldLen = tm_sprintf(buffer, "RC5_R16_B64");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdEncrypt
                                                == TM_IKE_BLOWFISH_CBC)
        {
            fieldLen = tm_sprintf(buffer, "blwfh");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdEncrypt
                                                == TM_IKE_3DES_CBC)
        {
            fieldLen = tm_sprintf(buffer, "3des");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdEncrypt
                                                == TM_IKE_CAST_CBC)
        {
            fieldLen = tm_sprintf(buffer, "cast");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdEncrypt
                                                == TM_IKE_AES_CBC)
        {
            fieldLen = tm_sprintf(buffer, "aes");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdEncrypt
                                                == TM_IKE_NULL_ENCRYPT)
        {
            fieldLen = tm_sprintf(buffer, "null");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdEncrypt
                                                == TM_IKE_TWOFISH_CBC)
        {
            fieldLen = tm_sprintf(buffer, "twofh");
        }
        
        buffer[fieldLen] = ' ';

/* protect mode */
        buffer = bufferBase + 2*TM_NT_IP_ADDR_STR_LEN 
                            + TM_NT_HASH_ALGORITHM_STR_LEN
                            + TM_NT_ENCRYPT_ALGOR_STR_LEN;
        if(ntIsaStateSpdEntryPtr->ntIsaStateSpdAuth
                                                == TM_IKE_PRESHARED_KEY)
        {
            fieldLen = tm_sprintf(buffer, "psk");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdAuth
                                                == TM_IKE_DSS_SIG)
        {
            fieldLen = tm_sprintf(buffer, "dss sig");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdAuth
                                                == TM_IKE_RSA_SIG)
        {
            fieldLen = tm_sprintf(buffer, "rsa sig");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdAuth
                                                == TM_IKE_RSA_ENC)
        {
            fieldLen = tm_sprintf(buffer, "rsa enc");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdAuth
                                                == TM_IKE_RSA_REVISED)
        {
            fieldLen = tm_sprintf(buffer, "rsa rev");
        }
        buffer[fieldLen] = ' ';

        buffer = bufferBase + 2*TM_NT_IP_ADDR_STR_LEN 
                            + TM_NT_HASH_ALGORITHM_STR_LEN
                            + TM_NT_ENCRYPT_ALGOR_STR_LEN
                            + TM_NT_PROTECT_PROTOCOL_STR_LEN;
        fieldLen = tm_sprintf(buffer, "%d", ntIsaStateSpdEntryPtr->
                                                ntIsaStateSpdDhGroup);
        buffer[fieldLen] = ' ';
/* lifetimes */
        buffer = bufferBase + 2*TM_NT_IP_ADDR_STR_LEN 
                            + TM_NT_HASH_ALGORITHM_STR_LEN
                            + TM_NT_ENCRYPT_ALGOR_STR_LEN
                            + TM_NT_PROTECT_PROTOCOL_STR_LEN
                            + TM_NT_DH_STR_LEN;
        fieldLen = tm_sprintf(buffer, "%d", ntIsaStateSpdEntryPtr->
                                                ntIsaStateTtlSeconds);
        buffer[fieldLen] = ' ';
        buffer = bufferBase + 2*TM_NT_IP_ADDR_STR_LEN 
                            + TM_NT_HASH_ALGORITHM_STR_LEN
                            + TM_NT_ENCRYPT_ALGOR_STR_LEN
                            + TM_NT_PROTECT_PROTOCOL_STR_LEN
                            + TM_NT_DH_STR_LEN
                            + TM_NT_INTEGER_STR_LEN;
        fieldLen = tm_sprintf(buffer, "%d", ntIsaStateSpdEntryPtr->
                                                ntIsaStateTtlKbytes);
        buffer[fieldLen] = ' ';

#ifdef TM_USE_IKE_DPD
        buffer = bufferBase + 2*TM_NT_IP_ADDR_STR_LEN 
                            + TM_NT_HASH_ALGORITHM_STR_LEN
                            + TM_NT_ENCRYPT_ALGOR_STR_LEN
                            + TM_NT_PROTECT_PROTOCOL_STR_LEN
                            + TM_NT_DH_STR_LEN
                            + 2*TM_NT_INTEGER_STR_LEN;
        fieldLen = tm_sprintf(buffer, "%d", ntIsaStateSpdEntryPtr->
                                                ntIsaStateSpdDpdWorryMetric);
        buffer[fieldLen] = ' ';
        buffer = bufferBase + 2*TM_NT_IP_ADDR_STR_LEN 
                            + TM_NT_HASH_ALGORITHM_STR_LEN
                            + TM_NT_ENCRYPT_ALGOR_STR_LEN
                            + TM_NT_PROTECT_PROTOCOL_STR_LEN
                            + TM_NT_DH_STR_LEN
                            + 2*TM_NT_INTEGER_STR_LEN
                            + TM_NT_DPD_STR_LEN;
        fieldLen = tm_sprintf(buffer, "%d", ntIsaStateSpdEntryPtr->
                                                ntIsaStateSpdDpdReXmitDelay);
        buffer[fieldLen] = ' ';
        buffer = bufferBase + 2*TM_NT_IP_ADDR_STR_LEN 
                            + TM_NT_HASH_ALGORITHM_STR_LEN
                            + TM_NT_ENCRYPT_ALGOR_STR_LEN
                            + TM_NT_PROTECT_PROTOCOL_STR_LEN
                            + TM_NT_DH_STR_LEN
                            + 2*TM_NT_INTEGER_STR_LEN
                            + 2*TM_NT_DPD_STR_LEN;
        fieldLen = tm_sprintf(buffer, "%d", ntIsaStateSpdEntryPtr->
                                                ntIsaStateSpdDpdReXmitLimit);
        buffer[fieldLen] = ' ';
        buffer = bufferBase + 2*TM_NT_IP_ADDR_STR_LEN 
                            + TM_NT_HASH_ALGORITHM_STR_LEN
                            + TM_NT_ENCRYPT_ALGOR_STR_LEN
                            + TM_NT_PROTECT_PROTOCOL_STR_LEN
                            + TM_NT_DH_STR_LEN
                            + 2*TM_NT_INTEGER_STR_LEN
                            + 3*TM_NT_DPD_STR_LEN;
        if(ntIsaStateSpdEntryPtr->ntIsaStateSpdDpdMode
                                                == TM_IKE_DPD_PERIODIC)
        {
            fieldLen = tm_sprintf(buffer, "periodic");
        }
        else if(ntIsaStateSpdEntryPtr->ntIsaStateSpdDpdMode
                                                == TM_IKE_DPD_ON_DEMAND)
        {
            fieldLen = tm_sprintf(buffer, "on-demand");
        }
#endif /* TM_USE_IKE_DPD */

        buffer[fieldLen] = '\0';
        *sizePtr = (int) tm_strlen(bufferBase);
    }

    return bufferBase;
}
#endif /* TM_USE_IKE */

#ifdef TM_USE_IPSEC

int tfNtGetSadbEntry(
    ttSadbRecordPtr         sadbRecordPtr, 
    ttNtSadbEntryPtr        ntSadbEntryPtr)
{
    ttTimerPtr timerPtr;
    tt32Bit    timeToLive;
    int        errorCode;

    if (!sadbRecordPtr || !ntSadbEntryPtr)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        tm_bcopy(&sadbRecordPtr->sadbSrcIpAddress,
            &ntSadbEntryPtr->ntSaSource,
            sizeof(struct sockaddr_storage));
        
        tm_bcopy(&sadbRecordPtr->sadbDstIpAddress,
            &ntSadbEntryPtr->ntSaDestination,
            sizeof(struct sockaddr_storage));

        if (sadbRecordPtr->sadbSAPtr)
        {
            ntSadbEntryPtr->ntSaEncrypt = 
                            sadbRecordPtr->sadbSAPtr->sadb_sa_encrypt;
            ntSadbEntryPtr->ntSaHash = sadbRecordPtr->sadbSAPtr->sadb_sa_auth;
            ntSadbEntryPtr->ntSaSpi = sadbRecordPtr->sadbSAPtr->sadb_sa_spi;
        }
        ntSadbEntryPtr->ntSaMode = sadbRecordPtr->sadbIpsecMode;
        ntSadbEntryPtr->ntSaProtect = sadbRecordPtr->sadbSaProtocol;
        
        if (tm_ipsec_is_vlifetime_inf(sadbRecordPtr->sadbRemainBytes))
        {
            tm_ipsec_set_vlifetime_to_inf(ntSadbEntryPtr->ntSaTtlKbytes);
        }
        else
        {
#ifdef TM_USE_IPSEC_64BIT_KB_LIFETIMES
            tm_user_64Bit_div32(sadbRecordPtr->sadbRemainBytes,
                                1000,
                                ntSadbEntryPtr->ntSaTtlKbytes);
#else /* TM_USE_IPSEC_64BIT_KB_LIFETIMES */
            ntSadbEntryPtr->ntSaTtlKbytes = 
                                sadbRecordPtr->sadbRemainBytes/1000;
#endif /* TM_USE_IPSEC_64BIT_KB_LIFETIMES */
        }
        timerPtr = (ttTimerPtr)sadbRecordPtr->sadbSoftTimerPtr;
        if (timerPtr != (ttTimerPtr)0)
        {
            timeToLive = tfTimerMsecLeft(timerPtr); 
        }
        else
        {
            timeToLive = TM_UL(0);
        }
        ntSadbEntryPtr->ntSaTtlSeconds = timeToLive / 1000;
        errorCode = TM_ENOERROR;
    }

    return errorCode;
}

int tfNtGetIpsecSpdEntry(
    ttPolicyEntryPtr        plcyPtr, 
    ttNtIpsecSpdEntryPtr    ntIpsecSpdEntryPtr)
{
    ttPolicyContentPtr   contentPtr;
    tt32Bit              flags;
    int                  errorCode;

    if (!plcyPtr || !ntIpsecSpdEntryPtr)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        errorCode = TM_ENOERROR;
        tm_bzero(ntIpsecSpdEntryPtr, sizeof(ttNtIpsecSpdEntry));
        tm_bcopy(&plcyPtr->plcySelector.selLocIp1,
                 &ntIpsecSpdEntryPtr->ntIpsecSpdLocalIp,
                 sizeof(struct sockaddr_storage));
        tm_bcopy(&plcyPtr->plcySelector.selRemtIp1,
                 &ntIpsecSpdEntryPtr->ntIpsecSpdRemoteIp,
                 sizeof(struct sockaddr_storage));
        ntIpsecSpdEntryPtr->ntIpsecSpdProtocol = 
                  plcyPtr->plcySelector.selProtocol;
        ntIpsecSpdEntryPtr->ntIpsecSpdIndex = plcyPtr->plcyIndex;
        contentPtr = plcyPtr->plcyContentPtr;
        
        if(tm_ipsec_plcyaction_nolog(plcyPtr->plcyAction)
           == TM_IPSEC_POLICY_BYPASS)
        {
            ntIpsecSpdEntryPtr->ntIpsecSpdProtect = TM_PFLAG_BYPASS;
        }
        else if(tm_ipsec_plcyaction_nolog(plcyPtr->plcyAction)
                == TM_IPSEC_POLICY_DISCARD)
        {
            ntIpsecSpdEntryPtr->ntIpsecSpdProtect = TM_PFLAG_DISCARD;
        }
        else if(contentPtr)
        {
            flags = contentPtr->pctRuleFlags;
            ntIpsecSpdEntryPtr->ntIpsecSpdProtect = (tt8Bit)(flags & 
                            (TM_PFLAG_AH | TM_PFLAG_ESP));
            ntIpsecSpdEntryPtr->ntIpsecSpdHash = contentPtr->pctPolicyContentInfo.pctiAuthAlg;
            ntIpsecSpdEntryPtr->ntIpsecSpdEncrypt = contentPtr
                                        ->pctPolicyContentInfo.pctiEncryptAlg;
            flags = contentPtr->pctRuleFlags;
            if(flags & TM_PFLAG_TRANSPORT)
            {
                ntIpsecSpdEntryPtr->ntIpsecSpdMode = SADB_MODE_TRANSPORT;
            }
            else if(flags & TM_PFLAG_TUNNEL)
            {
                if(flags & TM_PFLAG_MIPV6_HA_TUNNEL)
                {
                    ntIpsecSpdEntryPtr->ntIpsecSpdMode = SADB_MODE_MIPV6TUNNEL;
                }
                else
                {
                    ntIpsecSpdEntryPtr->ntIpsecSpdMode = SADB_MODE_TUNNEL;
                }
            }

            contentPtr = contentPtr->pctOuterContentPtr;
            if(contentPtr)
            {
                ntIpsecSpdEntryPtr->ntIpsecSpdOHash = contentPtr->pctPolicyContentInfo.pctiAuthAlg;
                flags = contentPtr->pctRuleFlags;
                if(flags & TM_PFLAG_TRANSPORT)
                {
                    ntIpsecSpdEntryPtr->ntIpsecSpdOMode = SADB_MODE_TRANSPORT;
                }
                else if(flags & TM_PFLAG_TUNNEL)
                {
                    ntIpsecSpdEntryPtr->ntIpsecSpdOMode = SADB_MODE_TUNNEL;
                }
                
                ntIpsecSpdEntryPtr->ntIpsecSpdOProtect = (tt8Bit)(flags & 
                    (TM_PFLAG_AH | TM_PFLAG_ESP));
            }
        }
    }

    return errorCode;
}


/* 
 * Function tfNtGetIpsecSpdHeaderStr()
 * DESCRIPTION:
 *  Get the IPsec SPD table header as a string.
 * PARAMETERS
 *  buffer:     the user provided buffer for storing the result string
 *  *sizePtr:   size of the user provided buffer, must be no shorter than
 *              TM_NT_IPSEC_SPD_TABLE_STR_LEN
 *  sizePtr:    user provided integer pointer to carry back the actual
 *              string length, ignored if NULL
 * RETURNS:
 *  TM_ENOERROR:    successful
 *  TM_ENINVAL:     buffer is NULL or *sizePtr is too small.
 */

char TM_FAR * tfNtGetIpsecSpdHeaderStr(
    char TM_FAR *   buffer, 
    int  TM_FAR *   sizePtr)
{

    char TM_FAR *   bufferBase;
    
    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_IPSEC_SPD_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);
        tm_nt_strncpy(buffer, 
                      "Idx, Selector(local ip:port,remote ip:port, protocol),"
                      "Inner policy, Outer policy,"
                      "LEAST PREFERRED SPD ENTRY FIRST !!! ",
                      2* TM_NT_SELECTOR_STR_LEN + 10);
        tm_nt_strncpy(buffer, "Protect", TM_NT_PROTECT_PROTOCOL_STR_LEN);
        tm_nt_strncpy(buffer, "Mode",    TM_NT_PROTECT_MODE_STR_LEN);
        tm_nt_strncpy(buffer, "Hash",    TM_NT_HASH_ALGORITHM_STR_LEN);
        tm_nt_strncpy(buffer, "Enc",     TM_NT_ENCRYPT_ALGOR_STR_LEN);
        tm_nt_strncpy(buffer, "Protect", TM_NT_PROTECT_PROTOCOL_STR_LEN);
        tm_nt_strncpy(buffer, "Mode",    TM_NT_PROTECT_MODE_STR_LEN);
        tm_nt_strncpy(buffer, "Hash",    TM_NT_HASH_ALGORITHM_STR_LEN);
        buffer[0] = '\0';

        *sizePtr = TM_NT_IPSEC_SPD_TABLE_STR_LEN;
    }
    return bufferBase;
}


char TM_FAR * tfNtGetIpsecSadbHeaderStr(
    char TM_FAR *   buffer, 
    int  TM_FAR *   sizePtr)
{
    char TM_FAR *   bufferBase;
    
    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_SADB_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);
        tm_nt_strncpy(buffer, "Source IP Address",TM_NT_IP_ADDR_STR_LEN);
        tm_nt_strncpy(buffer, "Destination IP Address",TM_NT_IP_ADDR_STR_LEN);
        tm_nt_strncpy(buffer, "Protect", TM_NT_PROTECT_PROTOCOL_STR_LEN);
        tm_nt_strncpy(buffer, "Mode",TM_NT_PROTECT_MODE_STR_LEN);
        tm_nt_strncpy(buffer, "SPI", TM_NT_INTEGER_STR_LEN);
        tm_nt_strncpy(buffer, "Hash",    TM_NT_HASH_ALGORITHM_STR_LEN);
        tm_nt_strncpy(buffer, "Enc",     TM_NT_ENCRYPT_ALGOR_STR_LEN);
        tm_nt_strncpy(buffer, "TTL-sec", TM_NT_INTEGER_STR_LEN);
        tm_nt_strncpy(buffer, "TTL-kb",  TM_NT_INTEGER_STR_LEN);
        buffer[0] = '\0';

        *sizePtr = TM_NT_SADB_TABLE_STR_LEN;
    }
    return bufferBase;
}


char TM_FAR * tfNtSadbEntryToStr(
                ttNtSadbEntryPtr      ntSadbEntryPtr, 
                char TM_FAR *         buffer, 
                int  TM_FAR *         sizePtr)
{
    int             af;
    int             fieldLen;
    char TM_FAR *   bufferBase;

    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_IPSEC_SPD_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        fieldLen = 0;
        tm_memset(buffer, ' ', *sizePtr);
/* the source IP */
        af = ntSadbEntryPtr->ntSaSource.ss_family;
#ifdef TM_USE_IPV4
        if(af == AF_INET)
        {
            inet_ntop(af,
                      &ntSadbEntryPtr->ntSaSource.addr.ipv4.sin_addr,
                      buffer,
                      INET_ADDRSTRLEN);
            fieldLen = (int) tm_strlen(buffer);
        }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
        else
#endif /* TM_USE_IPV4 */
            if(af == AF_INET6)
        {
            inet_ntop(af,
                      &ntSadbEntryPtr->ntSaSource.addr.ipv6.sin6_addr,
                      buffer,
                      INET6_ADDRSTRLEN);
            fieldLen = (int) tm_strlen(buffer);
        }
#endif /* TM_USE_IPV6*/
        else
        {
            fieldLen = tm_sprintf(buffer, "any, ");
        }
        buffer[fieldLen] = ' ';
/* the destination ip */
        buffer += TM_NT_IP_ADDR_STR_LEN;
        af = ntSadbEntryPtr->ntSaDestination.ss_family;
#ifdef TM_USE_IPV4
        if(af == AF_INET)
        {
            inet_ntop(af,
                      &ntSadbEntryPtr->ntSaDestination.addr.ipv4.sin_addr,
                      buffer,
                      INET_ADDRSTRLEN);
            fieldLen = (int) tm_strlen(buffer);
        }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
        else
#endif /* TM_USE_IPV4 */
            if(af == AF_INET6)
        {
            inet_ntop(af,
                      &ntSadbEntryPtr->ntSaDestination.addr.ipv6.sin6_addr,
                      buffer,
                      INET6_ADDRSTRLEN);
            fieldLen = (int) tm_strlen(buffer);
        }
#endif /* TM_USE_IPV6 */
        else
        {
            fieldLen = tm_sprintf(buffer, "any, ");
        }
        buffer[fieldLen] = ' ';

/* protection protocol */
        buffer += TM_NT_IP_ADDR_STR_LEN;
        if(ntSadbEntryPtr->ntSaProtect == TM_IP_AH)
        {
            fieldLen = tm_sprintf(buffer, "ah");
        }
        else if(ntSadbEntryPtr->ntSaProtect == TM_IP_ESP)
        {
            fieldLen = tm_sprintf(buffer, "esp");
        }
        buffer[fieldLen] = ' ';

/* protect mode */
        buffer += TM_NT_PROTECT_PROTOCOL_STR_LEN;
        if(ntSadbEntryPtr->ntSaMode == SADB_MODE_TRANSPORT)
        {
            fieldLen = tm_sprintf(buffer, "trans");
        }
        else if(ntSadbEntryPtr->ntSaMode == SADB_MODE_TUNNEL)
        {
            fieldLen = tm_sprintf(buffer, "tunnl");
        }
        else if(ntSadbEntryPtr->ntSaMode == SADB_MODE_MIPV6TUNNEL)
        {
            fieldLen = tm_sprintf(buffer, "miptun");
        }
        buffer[fieldLen] = ' ';
/* SPI */
        buffer += TM_NT_PROTECT_MODE_STR_LEN;
        fieldLen = tm_sprintf(buffer, "%lu", ntSadbEntryPtr->ntSaSpi);
        buffer[fieldLen] = ' ';
/* hash algorithm */
        buffer += TM_NT_INTEGER_STR_LEN;
        if(ntSadbEntryPtr->ntSaHash == SADB_AALG_NULL)
        {
            fieldLen = tm_sprintf(buffer, "null");
        }
        else if(ntSadbEntryPtr->ntSaHash == SADB_AALG_MD5HMAC)
        {
            fieldLen = tm_sprintf(buffer, "md5");
        }
        else if(ntSadbEntryPtr->ntSaHash == SADB_AALG_SHA1HMAC)
        {
            fieldLen = tm_sprintf(buffer, "sha1");
        }
        else if(ntSadbEntryPtr->ntSaHash == SADB_AALG_SHA256HMAC)
        {
            fieldLen = tm_sprintf(buffer, "sha2-256");
        }
        else if(ntSadbEntryPtr->ntSaHash == SADB_AALG_SHA384HMAC)
        {
            fieldLen = tm_sprintf(buffer, "sha2-384");
        }
        else if(ntSadbEntryPtr->ntSaHash == SADB_AALG_SHA512HMAC)
        {
            fieldLen = tm_sprintf(buffer, "sha2-512");
        }
        else if(ntSadbEntryPtr->ntSaHash == SADB_AALG_RIPEMDHMAC)
        {
            fieldLen = tm_sprintf(buffer, "ripe");
        }
        else if(ntSadbEntryPtr->ntSaHash == SADB_AALG_AESXCBCMAC)
        {
            fieldLen = tm_sprintf(buffer, "aes_xcbc");
        }
        buffer[fieldLen] = ' ';

/* encryption algorithm */
        buffer += TM_NT_HASH_ALGORITHM_STR_LEN;
        if(ntSadbEntryPtr->ntSaEncrypt == SADB_EALG_NULL)
        {
            fieldLen = tm_sprintf(buffer, "null");
        }
        else if(ntSadbEntryPtr->ntSaEncrypt == SADB_EALG_DESCBC)
        {
            fieldLen = tm_sprintf(buffer, "des");
        }
        else if(ntSadbEntryPtr->ntSaEncrypt == SADB_EALG_3DESCBC)
        {
            fieldLen = tm_sprintf(buffer, "3des");
        }
        else if(ntSadbEntryPtr->ntSaEncrypt == SADB_EALG_CAST128CBC)
        {
            fieldLen = tm_sprintf(buffer, "cast");
        }
        else if(ntSadbEntryPtr->ntSaEncrypt == SADB_EALG_AESCBC)
        {
            fieldLen = tm_sprintf(buffer, "aes_cbc");
        }
        else if(ntSadbEntryPtr->ntSaEncrypt == SADB_EALG_AESCTR)
        {
            fieldLen = tm_sprintf(buffer, "aes_ctr");
        }
#if (defined(TM_USE_VIPER) || defined(TM_USE_VIPER_TEST))        
        else if(ntSadbEntryPtr->ntSaEncrypt == SADB_EALG_AESCCM8)
        {
            fieldLen = tm_sprintf(buffer, "aes_ccm_8");
        }        
        else if(ntSadbEntryPtr->ntSaEncrypt == SADB_EALG_AESCCM12)
        {
            fieldLen = tm_sprintf(buffer, "aes_ccm_12");
        }        
        else if(ntSadbEntryPtr->ntSaEncrypt == SADB_EALG_AESCCM16)
        {
            fieldLen = tm_sprintf(buffer, "aes_ccm_16");
        }
#endif /* (defined(TM_USE_VIPER) || defined(TM_USE_VIPER_TEST)) */
        else if(ntSadbEntryPtr->ntSaEncrypt == SADB_EALG_TWOFISHCBC)
        {
            fieldLen = tm_sprintf(buffer, "twofh");
        }
        else if(ntSadbEntryPtr->ntSaEncrypt == SADB_EALG_BLOWFISHCBC)
        {
            fieldLen = tm_sprintf(buffer, "blwfh");
        }
        buffer[fieldLen] = ' ';
/* seconds lifetime  */
        buffer += TM_NT_ENCRYPT_ALGOR_STR_LEN;
        if (ntSadbEntryPtr->ntSaTtlSeconds)
        {
            fieldLen = tm_sprintf(buffer, "%lu", 
                            ntSadbEntryPtr->ntSaTtlSeconds);
        }
        else
        {
            fieldLen = tm_sprintf(buffer, "INF");
        }
        buffer[fieldLen] = ' ';
/* kbytes */
        buffer += TM_NT_INTEGER_STR_LEN;
        if (tm_ipsec_is_vlifetime_inf(ntSadbEntryPtr->ntSaTtlKbytes))
        {
            fieldLen = tm_sprintf(buffer, "INF");
        }
        else
        {
#ifdef TM_USE_IPSEC_64BIT_KB_LIFETIMES
            fieldLen = tm_sprintf(
                buffer, "0x%8x%8x",
                tm_user_64Bit_hi32(ntSadbEntryPtr->ntSaTtlKbytes),
                tm_user_64Bit_lo32(ntSadbEntryPtr->ntSaTtlKbytes));
#else /* TM_USE_IPSEC_64BIT_KB_LIFETIMES */
            fieldLen = tm_sprintf(buffer, "%lu", ntSadbEntryPtr->ntSaTtlKbytes);
#endif /* TM_USE_IPSEC_64BIT_KB_LIFETIMES */
        }
        buffer[fieldLen] = '\0';

        *sizePtr = (int) tm_strlen(bufferBase);
    }

    return bufferBase;
}

char TM_FAR * tfNtIpsecSpdEntryToStr(
                ttNtIpsecSpdEntryPtr  ntIpsecSpdEntryPtr, 
                char TM_FAR *         buffer, 
                int  TM_FAR *         sizePtr)
{
    int             fieldLen;
    char TM_FAR *   bufferBase;

    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_IPSEC_SPD_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        fieldLen = 0;
        tm_memset(buffer, ' ', *sizePtr);
/* the policy index */
        fieldLen = tm_sprintf(buffer, "%2d ", 
                                ntIpsecSpdEntryPtr->ntIpsecSpdIndex);
        buffer += fieldLen;
/* the selector information */
        tfNtSockAddrToStr(&ntIpsecSpdEntryPtr->ntIpsecSpdLocalIp, buffer);
        buffer += TM_NT_SOCK_ADDR_STR_LEN;
        tfNtSockAddrToStr(&ntIpsecSpdEntryPtr->ntIpsecSpdRemoteIp, buffer);
        buffer += TM_NT_SOCK_ADDR_STR_LEN;

        if(ntIpsecSpdEntryPtr->ntIpsecSpdProtocol == 0)
        {
            fieldLen = tm_sprintf(buffer, "any");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdProtocol == IPPROTO_UDP)
        {
            fieldLen = tm_sprintf(buffer, "udp");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdProtocol == IPPROTO_TCP)
        {
            fieldLen = tm_sprintf(buffer, "tcp");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdProtocol == IPPROTO_ICMP)
        {
            fieldLen = tm_sprintf(buffer, "icmp");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdProtocol == IPPROTO_ICMPV6)
        {
            fieldLen = tm_sprintf(buffer, "icmp6");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdProtocol == IPPROTO_MH)
        {
            fieldLen = tm_sprintf(buffer, "mh");
        }
        buffer[fieldLen] = ' ';
/* protect method : ah esp bypass, discard */

        buffer = bufferBase + 2*TM_NT_SELECTOR_STR_LEN + 10; 
        if(ntIpsecSpdEntryPtr->ntIpsecSpdProtect & TM_PFLAG_AH)
        {
            fieldLen = tm_sprintf(buffer, "ah");
        }
        else if (ntIpsecSpdEntryPtr->ntIpsecSpdProtect & TM_PFLAG_ESP)
        {
            fieldLen = tm_sprintf(buffer, "esp");
        }
        else if (ntIpsecSpdEntryPtr->ntIpsecSpdProtect & TM_PFLAG_BYPASS)
        {
            fieldLen = tm_sprintf(buffer, "bypass");
            buffer[fieldLen] = '\0';
            goto IPSEC_SPDSTR_RETURN;
        }
        else if (ntIpsecSpdEntryPtr->ntIpsecSpdProtect & TM_PFLAG_DISCARD)
        {
            fieldLen = tm_sprintf(buffer, "discard");
            buffer[fieldLen] = '\0';
            goto IPSEC_SPDSTR_RETURN;
        }
        buffer[fieldLen] = ' ';
/* protect mode */
        buffer += TM_NT_PROTECT_PROTOCOL_STR_LEN;
        if(ntIpsecSpdEntryPtr->ntIpsecSpdMode == SADB_MODE_TRANSPORT)
        {
            fieldLen = tm_sprintf(buffer, "trans");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdMode == SADB_MODE_TUNNEL)
        {
            fieldLen = tm_sprintf(buffer, "tunnl");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdMode == SADB_MODE_MIPV6TUNNEL)
        {
            fieldLen = tm_sprintf(buffer, "miptun");
        }
        buffer[fieldLen] = ' ';

/* hash algorithm */
        buffer += TM_NT_PROTECT_MODE_STR_LEN;
        if(ntIpsecSpdEntryPtr->ntIpsecSpdHash == SADB_AALG_NULL)
        {
            fieldLen = tm_sprintf(buffer, "null");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdHash == SADB_AALG_MD5HMAC)
        {
            fieldLen = tm_sprintf(buffer, "md5");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdHash == SADB_AALG_SHA1HMAC)
        {
            fieldLen = tm_sprintf(buffer, "sha1");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdHash == SADB_AALG_SHA256HMAC)
        {
            fieldLen = tm_sprintf(buffer, "sha2-256");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdHash == SADB_AALG_SHA384HMAC)
        {
            fieldLen = tm_sprintf(buffer, "sha2-384");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdHash == SADB_AALG_SHA512HMAC)
        {
            fieldLen = tm_sprintf(buffer, "sha2-512");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdHash == SADB_AALG_RIPEMDHMAC)
        {
            fieldLen = tm_sprintf(buffer, "ripe");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdHash == SADB_AALG_AESXCBCMAC)
        {
            fieldLen = tm_sprintf(buffer, "aes_xcbc");
        }
        buffer[fieldLen] = ' ';

/* encryption algorithm */
        buffer += TM_NT_HASH_ALGORITHM_STR_LEN;
        if(ntIpsecSpdEntryPtr->ntIpsecSpdEncrypt == SADB_EALG_NULL)
        {
            fieldLen = tm_sprintf(buffer, "null");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdEncrypt == SADB_EALG_DESCBC)
        {
            fieldLen = tm_sprintf(buffer, "des");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdEncrypt == SADB_EALG_3DESCBC)
        {
            fieldLen = tm_sprintf(buffer, "3des");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdEncrypt == SADB_EALG_CAST128CBC)
        {
            fieldLen = tm_sprintf(buffer, "cast");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdEncrypt == SADB_EALG_AESCBC)
        {
            fieldLen = tm_sprintf(buffer, "aes_cbc");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdEncrypt == SADB_EALG_AESCTR)
        {
            fieldLen = tm_sprintf(buffer, "aes_ctr");
        }
#if (defined(TM_USE_VIPER) || defined(TM_USE_VIPER_TEST))        
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdEncrypt == SADB_EALG_AESCCM8)
        {
            fieldLen = tm_sprintf(buffer, "aes_ccm_8");
        }        
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdEncrypt == SADB_EALG_AESCCM12)
        {
            fieldLen = tm_sprintf(buffer, "aes_ccm_12");
        }        
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdEncrypt == SADB_EALG_AESCCM16)
        {
            fieldLen = tm_sprintf(buffer, "aes_ccm_16");
        }
#endif /* (defined(TM_USE_VIPER) || defined(TM_USE_VIPER_TEST)) */
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdEncrypt == SADB_EALG_TWOFISHCBC)
        {
            fieldLen = tm_sprintf(buffer, "twofh");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdEncrypt == SADB_EALG_BLOWFISHCBC)
        {
            fieldLen = tm_sprintf(buffer, "blwfh");
        }
        if(!ntIpsecSpdEntryPtr->ntIpsecSpdOProtect)
        {
            buffer[fieldLen] = '\0';
            goto IPSEC_SPDSTR_RETURN;
        }
        buffer[fieldLen] = ' ';
/* outer policy content */
        buffer += TM_NT_ENCRYPT_ALGOR_STR_LEN;
        if(ntIpsecSpdEntryPtr->ntIpsecSpdOProtect & TM_PFLAG_AH)
        {
            fieldLen = tm_sprintf(buffer, "ah");
        }
        else if (ntIpsecSpdEntryPtr->ntIpsecSpdOProtect & TM_PFLAG_ESP)
        {
            fieldLen = tm_sprintf(buffer, "esp");
        }
        buffer[fieldLen] = ' ';

/* protect mode */
        buffer += TM_NT_PROTECT_PROTOCOL_STR_LEN;

        buffer += TM_NT_PROTECT_PROTOCOL_STR_LEN;
        if(ntIpsecSpdEntryPtr->ntIpsecSpdOMode == SADB_MODE_TRANSPORT)
        {
            fieldLen = tm_sprintf(buffer, "trans");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdOMode == SADB_MODE_TUNNEL)
        {
            fieldLen = tm_sprintf(buffer, "tunnl");
        }
        buffer[fieldLen] = ' ';

/* hash algorithm */
        buffer += TM_NT_PROTECT_MODE_STR_LEN;
        if(ntIpsecSpdEntryPtr->ntIpsecSpdOHash == SADB_AALG_NULL)
        {
            fieldLen = tm_sprintf(buffer, "null");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdOHash == SADB_AALG_MD5HMAC)
        {
            fieldLen = tm_sprintf(buffer, "md5");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdOHash == SADB_AALG_SHA1HMAC)
        {
            fieldLen = tm_sprintf(buffer, "sha1");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdOHash == SADB_AALG_SHA256HMAC)
        {
            fieldLen = tm_sprintf(buffer, "sha2-256");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdOHash == SADB_AALG_SHA384HMAC)
        {
            fieldLen = tm_sprintf(buffer, "sha2-384");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdOHash == SADB_AALG_SHA512HMAC)
        {
            fieldLen = tm_sprintf(buffer, "sha2-512");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdOHash == SADB_AALG_RIPEMDHMAC)
        {
            fieldLen = tm_sprintf(buffer, "ripe");
        }
        else if(ntIpsecSpdEntryPtr->ntIpsecSpdOHash == SADB_AALG_AESXCBCMAC)
        {
            fieldLen = tm_sprintf(buffer, "aes_xcbc");
        }

        buffer[fieldLen] = '\0';

        *sizePtr = (int) tm_strlen(bufferBase);
    }

IPSEC_SPDSTR_RETURN:
    return bufferBase;
}


#endif /* TM_USE_IPSEC */

/* 
 * Function tf6NtBindingEntryToStr()
 * DESCRIPTION: 
 *  print the entry as s string to the user provided buffer
 * PARAMETERS
 *  ntBindingEntryPtr
          : pointer to the entry to be converted to string
 *  buffer:     the user provided buffer for storing the result string
 *  *sizePtr: size of the user provided buffer, must be no shorting than
 *              TM_NT_Binding_TABLE_STR_LEN
 *  sizePtr:  user provided integer pointer to carry back the actual
 *              string length, ignored if NULL
 * RETURNS:
 *  TM_ENOERROR:    successful
 *  TM_ENINVAL:     ntArpEntryPtr is NULL, 
 *                  buffer is NULL, 
 *                  or *sizePtr is too small.
 */
#ifdef TM_6_USE_MIP_CN
char TM_FAR * tf6NtBindingEntryToStr(
    tt6NtBindingEntryPtr ntBindingEntryPtr, 
    char TM_FAR *   buffer, 
    int  TM_FAR *   sizePtr)
{
    int             fieldLen;
    char TM_FAR *   bufferBase;

    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_BINDING_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);

        inet_ntop(AF_INET6, 
                  &ntBindingEntryPtr->nt6BindHomeAddr, 
                  buffer, 
                  INET6_ADDRSTRLEN);
        buffer[tm_strlen(buffer)] = ' ';
        buffer += TM_NT_IP_ADDR_STR_LEN;

        inet_ntop(AF_INET6, 
                  &ntBindingEntryPtr->nt6BindCoAddr, 
                  buffer, 
                  INET6_ADDRSTRLEN);
        buffer[tm_strlen(buffer)] = ' ';
        buffer += TM_NT_IP_ADDR_STR_LEN;

        fieldLen = tm_sprintf(buffer, 
                              "%d",
                              ntBindingEntryPtr->nt6BindLifetime);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_TTL_STR_LEN;

        fieldLen = tm_sprintf(buffer, 
                              "%d",
                              ntBindingEntryPtr->nt6BindLastSeq);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_LAST_SEQ_STR_LEN;

        fieldLen = tm_sprintf(buffer, 
                              "%d",
                              ntBindingEntryPtr->nt6BindLastSendTime);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_LAST_USED_TIME_STR_LEN;

        fieldLen = tm_sprintf(buffer, 
                              "%d",
                              ntBindingEntryPtr->nt6BindLastRecvTime);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_LAST_USED_TIME_STR_LEN;

        fieldLen = tm_sprintf(buffer, 
                              "%d",
                              ntBindingEntryPtr->nt6BindLastUpdatedTime);


#ifdef TM_6_USE_MIP_HA
        buffer[fieldLen] = ' ';
        buffer += TM_NT_LAST_UPDATED_TIME_STR_LEN;

        if (ntBindingEntryPtr->nt6BindHomeReg != 0)
        {
            fieldLen = tm_sprintf(buffer, "yes");
        }
        else
        {
            fieldLen = tm_sprintf(buffer, "no");
        }
        buffer[fieldLen] = ' ';
        buffer += TM_NT_HOME_REG_STR_LEN;

        tm_sprintf(buffer, "%d", ntBindingEntryPtr->nt6BindProxyMHome);
#endif /* TM_6_USE_MIP_HA*/

        *sizePtr = (int) tm_strlen(bufferBase);
    }
    return bufferBase;
}
#endif /* TM_6_USE_MIP_CN */



/* 
 * Function tfNtDeviveEntryToStr()
 * DESCRIPTION: 
 *  print the entry as s string to the user provided buffer
 * PARAMETERS
 *  ntDeviceEntryPtr
          : pointer to the entry to be converted to string
 *  buffer:     the user provided buffer for storing the result string
 *  *sizePtr: size of the user provided buffer, must be no shorting than
 *              TM_NT_Device_TABLE_STR_LEN
 *  sizePtr:  user provided integer pointer to carry back the actual
 *              string length, ignored if NULL
 * RETURNS:
 *  TM_ENOERROR:    successful
 *  TM_ENINVAL:     ntArpEntryPtr is NULL, 
 *                  buffer is NULL, 
 *                  or *sizePtr is too small.
 */
char TM_FAR * tfNtDeviceEntryToStr(
    ttNtDevEntryPtr ntDevEntryPtr, 
    char TM_FAR *   buffer, 
    int  TM_FAR *   sizePtr)
{
    char TM_FAR *   bufferBase;
    unsigned int    isLen;

    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_DEVICE_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);

        tm_nt_strncpy(
            buffer, 
            ntDevEntryPtr->ntDevName,  
            TM_NT_IFACE_STR_LEN);

        tfNtHwAddrToStr(
            (tt8BitPtr)ntDevEntryPtr->ntDevPhyAddr, 
            ntDevEntryPtr->ntDevPhyAddrLen,
            buffer);
        isLen = tm_strlen(buffer);
        if (isLen == 0)
        {
            tm_strcpy(buffer, "NA");
        }

        buffer[tm_strlen(buffer)] = ' ';

        buffer += TM_NT_HWADDR_STR_LEN;

        switch(ntDevEntryPtr->ntDevStatus)

        {
        case TM_NT_DEVICE_STATUS_CLOSED:
            tm_nt_strncpy(buffer, "closed", TM_NT_DEVICE_STATUS_STR_LEN);
            break;
        case TM_NT_DEVICE_STATUS_OPEN:
            tm_nt_strncpy(buffer, "open", TM_NT_DEVICE_STATUS_STR_LEN);
            break;
        case TM_NT_DEVICE_STATUS_CONNECTING:
            tm_nt_strncpy(buffer, "connecting", TM_NT_DEVICE_STATUS_STR_LEN);
            break;
        case TM_NT_DEVICE_STATUS_CONNECTED:
            tm_nt_strncpy(buffer, "connected", TM_NT_DEVICE_STATUS_STR_LEN);
            break;
        default:
            break;
        }
        buffer += TM_NT_DEVICE_STATUS_STR_LEN;
        buffer[0] = 0;

        *sizePtr = (int) tm_strlen(bufferBase);
    }
    return bufferBase;
}


/* print the addr conf entry as s string to the user provided buffer*/
#ifdef TM_USE_IPV6
char TM_FAR * tf6NtAddrConfEntryToStr(
    tt6NtAddrEntryPtr nt6AddrEntryPtr, 
    char TM_FAR *    buffer, 
    int  TM_FAR *    sizePtr)
{
    int             fieldLen;
    char TM_FAR *   bufferBase;



    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_DEVICE_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        fieldLen = 0;
        tm_memset(buffer, ' ', *sizePtr);

        tm_strcpy(buffer, "IPv6");
        buffer[tm_strlen(buffer)] = ' ';
        buffer += TM_NT_FAMILY_STR_LEN;

        fieldLen = tm_sprintf(buffer, "%d", nt6AddrEntryPtr->nt6AddrMHome);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_MHOME_STR_LEN;

        switch (nt6AddrEntryPtr->nt6AddrType)
        {
        case TM_NT_ADDR_CONF_TYPE_NONE:
            bufferBase[0] = 0;
            return bufferBase;
    
        case TM_NT_ADDR_CONF_TYPE_MANUAL:
            tm_nt_strncpy(buffer, "manual", TM_NT_ADDR_CONF_TYPE_STR_LEN);
            break;

        case TM_NT_ADDR_CONF_TYPE_AUTO:
            tm_nt_strncpy(buffer, "auto", TM_NT_ADDR_CONF_TYPE_STR_LEN);
            break;

        case TM_NT_ADDR_CONF_TYPE_DHCPV6:
            tm_nt_strncpy(buffer, "dhcpv6", TM_NT_ADDR_CONF_TYPE_STR_LEN);
            break;

        case TM_NT_ADDR_CONF_TYPE_PROXY:
            tm_nt_strncpy(buffer, "proxy", TM_NT_ADDR_CONF_TYPE_STR_LEN);
            break;

        default:
            bufferBase[0] = 0;
            return bufferBase;
        }

        /* IP address / prefix length */
        inet_ntop(
                AF_INET6, 
                &nt6AddrEntryPtr->nt6AddrAddr, 
                buffer,
                INET6_ADDRSTRLEN);

        tm_sprintf(buffer + tm_strlen(buffer), "/%d", 
                   nt6AddrEntryPtr->nt6AddrPrefixLen);
        *(buffer + tm_strlen(buffer)) = ' ';
        buffer += TM_NT_KEY_IP_ADDR_STR_LEN; 

        switch(nt6AddrEntryPtr->nt6AddrStatus )
        {
        case TM_NT_ADDR_STATUS_CONFIGURED:
            tm_nt_strncpy(buffer,"configured",TM_NT_ADDR_CONF_STATUS_STR_LEN);
            break;
        case TM_NT_ADDR_STATUS_CONFIGURING:
            tm_nt_strncpy(buffer,"configuring",TM_NT_ADDR_CONF_STATUS_STR_LEN);
            break;
        case TM_NT_ADDR_STATUS_DEPRECATED:
            tm_nt_strncpy(buffer,"deprecated",TM_NT_ADDR_CONF_STATUS_STR_LEN);
            break;
        case TM_NT_ADDR_STATUS_UNKOWN:
            tm_nt_strncpy(buffer,"unkown",TM_NT_ADDR_CONF_STATUS_STR_LEN);
        default:
            break;
        }

        if(nt6AddrEntryPtr->nt6AddrStatus & TM_NT_ADDR_CONF_REG_STR_LEN)
        {
            tm_nt_strncpy(buffer, "registered", TM_NT_ADDR_CONF_REG_STR_LEN);
        }
        
        buffer[0] = '\0';

        *sizePtr = (int) tm_strlen(bufferBase);
    }
    return bufferBase;
}
#endif /* TM_USE_IPV6*/


/* print the addr conf entry as s string to the user provided buffer*/
#ifdef TM_USE_IPV4
char TM_FAR * tfNtAddrConfEntryToStr(
    ttNtAddrEntryPtr ntAddrEntryPtr, 
    char TM_FAR *    buffer, 
    int  TM_FAR *    sizePtr)
{
    int             fieldLen;
    char TM_FAR *   bufferBase;

    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_DEVICE_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        fieldLen = 0;
        tm_memset(buffer, ' ', *sizePtr);

        tm_strcpy(buffer, "IPv4");
        buffer[tm_strlen(buffer)] = ' ';
        buffer += TM_NT_FAMILY_STR_LEN;

        fieldLen = tm_sprintf(buffer, "%d", ntAddrEntryPtr->ntAddrMHome);
        buffer[fieldLen] = ' ';
        buffer += TM_NT_MHOME_STR_LEN;

        switch (ntAddrEntryPtr->ntAddrType)
        {
        case TM_NT_ADDR_CONF_TYPE_NONE:
            bufferBase[0] = 0;
            return bufferBase;
    
        case TM_NT_ADDR_CONF_TYPE_MANUAL:
            tm_nt_strncpy(buffer, "manual",   TM_NT_ADDR_CONF_TYPE_STR_LEN);
            break;

        case TM_NT_ADDR_CONF_TYPE_BOOTP:
            tm_nt_strncpy(buffer, "bootp", TM_NT_ADDR_CONF_TYPE_STR_LEN);
            break;

        case TM_NT_ADDR_CONF_TYPE_DHCP:
            tm_nt_strncpy(buffer, "dhcp", TM_NT_ADDR_CONF_TYPE_STR_LEN);
            break;

        default:
            bufferBase[0] = 0;
            return bufferBase;
        }

        /* IP address / prefix length */
        inet_ntop(
                AF_INET, 
                &ntAddrEntryPtr->ntAddrAddr, 
                buffer,
                INET_ADDRSTRLEN);

        tm_sprintf(buffer + tm_strlen(buffer), "/%d", 
                   ntAddrEntryPtr->ntAddrPrefixLen);
        *(buffer + tm_strlen(buffer)) = ' ';
        buffer += TM_NT_KEY_IP_ADDR_STR_LEN; 

        switch(ntAddrEntryPtr->ntAddrStatus )
        {
        case TM_NT_ADDR_STATUS_CONFIGURED:
            tm_nt_strncpy(buffer,"configured",TM_NT_ADDR_CONF_STATUS_STR_LEN);
            break;
        case TM_NT_ADDR_STATUS_CONFIGURING:
            tm_nt_strncpy(buffer,"configuring",TM_NT_ADDR_CONF_STATUS_STR_LEN);
            break;
        case TM_NT_ADDR_STATUS_UNKOWN:
            tm_nt_strncpy(buffer,"unkown",TM_NT_ADDR_CONF_STATUS_STR_LEN);
        default:
            break;
        }

       
        buffer[0] = '\0';

        *sizePtr = (int) tm_strlen(bufferBase);
    }
    return bufferBase;
}
#endif /* TM_USE_IPV4 */


/*
 * tfNetStat() function description
 * This function walks through tvSnmpdCaches, calls ntEntryCBFuncPtr for each 
 * entry found with the entry, genParam1 and genParam, 
 * it continues if ntEntryCBFuncPtr returns TM_ENOERROR and 
 * returns when ntEntryCBFuncPtr returns other values
 *
 * Parameters
 * Parameter        Description
 * tableId   the cached table to output, current support values are
 *                  TM_NT_TABLE_RTE,  TM_NT_TABLE_ARP, TM_NT_TABLE_UDP
 *                  and TM_NT_TABLE_TCP
 * ntEntryCBFuncPtr    Call back function for printing each of the routing entry,
 *                  must implements the ttNtEntryCB() interface.
 * genParam1        Generic parameter, For passing other user information 
 *                  information on to the callback functions.
 * genParam2        Generic parameter, For passing other user information 
 *                  information on to the callback functions.
 *
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Completed successfully
 * TM_ENOENT        Call back function failed
 * TM_EINVAL        One of the passed in parameter is invalid
 *
 */
int tfNetStat(
    ttNtTableId         tableId,
    ttNtEntryCBFuncPtr  ntEntryCBFuncPtr,
    ttUserGenericUnion  genParam1,
    ttUserGenericUnion  genParam2)
{
    int                 errorCode;

    errorCode = TM_ENOERROR;
    
    if (ntEntryCBFuncPtr == (ttNtEntryCBFuncPtr)0)
    {
/*  ntEntryCBFuncPtr must not be NULL */
        errorCode = TM_EINVAL;
    }
    else
    {
/*
 * output the entries
 */
#ifdef TM_USE_IPV4
        errorCode = tfNtEnumEntry(
                        tableId,                                 
                        tfNtTableToCacheIndex(AF_INET, tableId),
                        ntEntryCBFuncPtr,  
                        genParam1,
                        genParam2);

#if (defined(TM_USE_IKEV1) || defined(TM_USE_IPSEC))
        if (
#ifdef TM_USE_IKEV1
               (tableId == TM_NT_TABLE_ISASTATE_ENTRY)
            || (tableId == TM_NT_TABLE_ISASTATE_SPD)
#ifdef TM_USE_IPSEC
            ||
#endif /* TM_USE_IKEV1 && TM_USE_IPSEC */
#endif /* TM_USE_IKEV1 */
#ifdef TM_USE_IPSEC
               (tableId == TM_NT_TABLE_IPSEC_SPD)
            || (tableId == TM_NT_TABLE_SADB)
#endif /* TM_USE_IPSEC */
            )
        {
/* we don't have to call again */
            goto NTST_RETURN;
        }
#endif /* TM_USE_IKEV1 || TM_USE_IPSEC */
#endif /* TM_USE_IPV4 */


#ifdef TM_USE_IPV6
        errorCode = tfNtEnumEntry(
                        tableId,                                 
                        tfNtTableToCacheIndex(AF_INET6, tableId), 
                        ntEntryCBFuncPtr,  
                        genParam1,
                        genParam2);
#endif /* TM_USE_IPV6 */
    }

#if (defined(TM_USE_IPV4) && (defined(TM_USE_IKE) || defined(TM_USE_IPSEC)))
NTST_RETURN:
#endif /* TM_USE_IPV4 && (TM_USE_IKE || TM_USE_IPSEC) */
    return errorCode;
}


/* walk though the list and output the entries */    
static int tfNtEnumEntry(
    ttNtTableId         tableId,
    int                 cacheIndex,
    ttNtEntryCBFuncPtr  ntEntryCBFuncPtr,
    ttUserGenericUnion  genParam1,
    ttUserGenericUnion  genParam2)
{

    ttSnmpdCacheHeadPtr snmpdCacheHeadPtr;
    ttSnmpCacheEntryPtr snmpCacheEntryPtr;
    ttNtEntryUPtr       ntEntryPtr;
    ttNtGetFuncEntryPtr getFuncEntryPtr; 
    ttDeviceEntryPtr    devPtr;
#ifdef TM_USE_IPV4
    ttNatTriggerPtr     trigPtr;
    ttNtNatEntryPtr     ntNatHeadPtr;
    ttNtNatEntryPtr     ntNatCurPtr;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IKE
    ttIkeGlobalPtr      gIkePtr;
    ttIsakmpStatePtr    isaStatePtr;
    ttIkeEntryPtr       gIkePolicyHeadPtr;
#endif /* TM_USE_IKE */
#ifdef TM_USE_IPSEC
    ttIpsecEntryPtr     gIpsecPtr;
    ttPolicyEntryPtr    plcyPtr;
    ttSadbRecordPtr     sadbPtr;
    ttSadbRecordPtr     checkSadbPtr;
    int                 plcyCount;
#endif /* TM_USE_IPSEC */
#if (defined(TM_USE_IKE) || defined(TM_USE_IPSEC))
    int                 j;
    int                 n;
#endif /* (defined(TM_USE_IKE) || defined(TM_USE_IPSEC)) */
#ifdef TM_6_USE_MIP_CN
    ttNodePtr           nodePtr;
    tt32Bit             i;
    unsigned int        listCount;
#endif /* TM_6_USE_MIP_CN */
    int                 errorCode;
#ifdef TM_USE_IPV4
    tt16Bit             numNatTriggers;
#endif /* TM_USE_IPV4 */
#ifdef TM_6_USE_MIP_CN
    tt8Bit              temp8Bit;
#endif /* TM_6_USE_MIP_CN */

/* start from the first route entry */
    snmpdCacheHeadPtr = &(tm_context(tvSnmpdCaches)[cacheIndex]);

#ifdef TM_6_USE_MIP_CN
    listCount = 0;
#endif /* TM_6_USE_MIP_CN */

    ntEntryPtr = tm_get_raw_buffer(sizeof(ttNtEntryU));
                   
    if (ntEntryPtr != (ttNtEntryUPtr)0)
    {
        errorCode = TM_ENOERROR;
        tm_bzero(ntEntryPtr, sizeof(ttNtEntryU));
        getFuncEntryPtr = (ttNtGetFuncEntryPtr)0;
        if (tableId <= TM_NT_TABLE_UDP)
        {
            switch (tableId)
            {
                case TM_NT_TABLE_RTE:
                    getFuncEntryPtr = tfNtGetRteEntry;
                    break;
                case TM_NT_TABLE_ARP:
                    getFuncEntryPtr = tfNtGetArpEntry;
                    break;
                case TM_NT_TABLE_TCP:
                    getFuncEntryPtr = tfNtGetTcpEntry;
                    break;
                case TM_NT_TABLE_UDP:
                    getFuncEntryPtr = tfNtGetUdpEntry;
                    break;
                default:
                    errorCode = TM_EINVAL;
                    break;
            }
            while (errorCode == TM_ENOERROR)
            {
/* Lock the cache (to lock out SNMP cache changes) */
#ifdef TM_LOCK_NEEDED
                tm_call_lock_wait(&(snmpdCacheHeadPtr->snmpcLockEntry));
#endif /* TM_LOCK_NEEDED */
                snmpdCacheHeadPtr->snmpcNextNodePtr =
                      snmpdCacheHeadPtr->snmpcList.listHeadNode.nodeNextPtr;
                while (snmpdCacheHeadPtr->snmpcNextNodePtr !=
                                  &(snmpdCacheHeadPtr->snmpcList.listHeadNode))
                {
                    snmpCacheEntryPtr = (ttSnmpCacheEntryPtr)
                                        snmpdCacheHeadPtr->snmpcNextNodePtr;
                    snmpdCacheHeadPtr->snmpcNextNodePtr =
                             snmpdCacheHeadPtr->snmpcNextNodePtr->nodeNextPtr;
                    if (snmpCacheEntryPtr != (ttSnmpCacheEntryPtr)0)
                    {
                        errorCode = (*getFuncEntryPtr)(snmpCacheEntryPtr,
                                                       ntEntryPtr);
                    }
                    else
                    {
                        errorCode = TM_EFAULT;
                    }
                    if (errorCode != TM_ENOERROR)
                    {
                        break;
                    }
#ifdef TM_LOCK_NEEDED
                    errorCode = tfNtCallBack(
                                     &(snmpdCacheHeadPtr->snmpcLockEntry),
                                     ntEntryCBFuncPtr,
                                     ntEntryPtr,
                                     genParam1, genParam2);
#else /* !TM_LOCK_NEEDED */
                    errorCode = (*ntEntryCBFuncPtr)(ntEntryPtr,
                                                    genParam1, 
                                                    genParam2);
#endif /* !TM_LOCK_NEEDED */
                    if (errorCode != TM_ENOERROR)
                    {
                        break;
                    }
                }
/* UnLock the SNMP cache */
#ifdef TM_LOCK_NEEDED
                tm_call_unlock(&(snmpdCacheHeadPtr->snmpcLockEntry));
#endif /* TM_LOCK_NEEDED */
                break;
            }
        }
        else
        {
            switch (tableId)
            {
#ifdef TM_6_USE_MIP_CN
            case TM_NT_TABLE_BINDING:
                if (cacheIndex != -1)
                {
#ifdef TM_LOCK_NEEDED
                    tm_call_lock_wait(&tm_context(tv6BindingLock));
#endif /* TM_LOCK_NEEDED */
                    nodePtr = 
                        tm_context(tv6MipBindingList).listHeadNode.nodeNextPtr;
                    listCount = tm_context(tv6MipBindingList).listCount;
                    for (i=0; i < listCount; i++)
                    {
                        tfNtGetBindingEntry(
                                    (tt6BindingEntryPtr)nodePtr, 
                                    &ntEntryPtr->nt6BindingEntry);

                        nodePtr = nodePtr->nodeNextPtr;
#ifdef TM_LOCK_NEEDED
                        errorCode = tfNtCallBack(
                                     &tm_context(tv6BindingLock),
                                     ntEntryCBFuncPtr,
                                     ntEntryPtr,
                                     genParam1, genParam2);
#else /* !TM_LOCK_NEEDED */
                        errorCode = (*ntEntryCBFuncPtr)(ntEntryPtr,
                                                        genParam1, 
                                                        genParam2);
#endif /* !TM_LOCK_NEEDED */
                        if (errorCode != TM_ENOERROR)
                        {
                            break;
                        }
                        temp8Bit = tfListMember(&tm_context(tv6MipBindingList),
                                                nodePtr);
                        if (!temp8Bit)
                        {
                            break;
                        }
                    }
#ifdef TM_LOCK_NEEDED
                    tm_call_unlock(&tm_context(tv6BindingLock));
#endif /* TM_LOCK_NEEDED */
                }
            break;
#endif /* TM_6_USE_MIP_CN */

        case TM_NT_TABLE_DEVICE:
                if (cacheIndex != -1)
                {
#ifdef TM_LOCK_NEEDED
                    tm_call_lock_wait(&tm_context(tvDeviceListLock));
#endif /* TM_LOCK_NEEDED */
                    devPtr = tm_context(tvDeviceList);
                    while (devPtr != TM_DEV_NULL_PTR)
                    {
                        tfNtGetDeviceEntry(
                                    devPtr, 
                                    &ntEntryPtr->ntDevEntry);

                        devPtr = devPtr->devNextDeviceEntry;
#ifdef TM_LOCK_NEEDED
                        errorCode = tfNtCallBack(
                                     &tm_context(tvDeviceListLock),
                                     ntEntryCBFuncPtr,
                                     ntEntryPtr,
                                     genParam1, genParam2);
#else /* !TM_LOCK_NEEDED */
                        errorCode = (*ntEntryCBFuncPtr)(ntEntryPtr,
                                                        genParam1, 
                                                        genParam2);
#endif /* !TM_LOCK_NEEDED */
                        if (errorCode != TM_ENOERROR)
                        {
                            break;
                        }
/*
 * Devices are never deleted. So no need to check that device is stil
 * in the list after call to user.
 */
                    }
#ifdef TM_LOCK_NEEDED
                    tm_call_unlock(&tm_context(tvDeviceListLock));
#endif /* TM_LOCK_NEEDED */
                }
                break;
#ifdef TM_USE_IKE
            case TM_NT_TABLE_ISASTATE_ENTRY:
#ifdef TM_LOCK_NEEDED
                tm_call_lock_wait(&(tm_context(tvIkeLockEntry)));
#endif /* TM_LOCK_NEEDED */
                gIkePtr = (ttIkeGlobalPtr)tm_context(tvIkePtr);
                if (gIkePtr == TM_IKE_GLOBAL_NULL_PTR)
                {
#ifdef TM_LOCK_NEEDED
                    tm_call_unlock(&(tm_context(tvIkeLockEntry)));
#endif /* TM_LOCK_NEEDED */
                    errorCode = TM_EIKENOTSTARTED;
                    break;
                }
                
                for(j = 0; j < TM_IKE_STATETABLE_SIZE; j ++)
                {
                    isaStatePtr = gIkePtr->ikeSaTable[j];
                    while (isaStatePtr != TM_ISAKMP_STATE_NULL_PTR)
                    {
                        if (!(isaStatePtr->isaFlags & TM_IKESTATE_FLAG_PHASE2))
                        {
                            tfNtGetIsaStateEntry(isaStatePtr,
                                                &ntEntryPtr->ntIsaStateEntry);
#ifdef TM_LOCK_NEEDED
                            errorCode = tfNtCallBack(
                                                &(tm_context(tvIkeLockEntry)),
                                                ntEntryCBFuncPtr,
                                                ntEntryPtr,
                                                genParam1, genParam2);
#else /* !TM_LOCK_NEEDED */
                            errorCode = (*ntEntryCBFuncPtr)(ntEntryPtr,
                                                            genParam1, 
                                                            genParam2);
#endif /* !TM_LOCK_NEEDED */
                            if (errorCode != TM_ENOERROR)
                            {
                                break;
                            }
/* Recheck because of call back function */
                            gIkePtr = (ttIkeGlobalPtr)tm_context(tvIkePtr);
                            if (gIkePtr == TM_IKE_GLOBAL_NULL_PTR)
                            {
                                break;
                            }
                        }
                        isaStatePtr = isaStatePtr->isaHashNext;
                    }
                }
#ifdef TM_LOCK_NEEDED
                tm_call_unlock(&(tm_context(tvIkeLockEntry)));
#endif /* TM_LOCK_NEEDED */
                break;
            case TM_NT_TABLE_ISASTATE_SPD:
#ifdef TM_LOCK_NEEDED
                tm_call_lock_wait(&(tm_context(tvIkeLockEntry)));
#endif /* TM_LOCK_NEEDED */
                gIkePtr = (ttIkeGlobalPtr)tm_context(tvIkePtr);
                if (gIkePtr == TM_IKE_GLOBAL_NULL_PTR)
                {
#ifdef TM_LOCK_NEEDED
                    tm_call_unlock(&(tm_context(tvIkeLockEntry)));
#endif /* TM_LOCK_NEEDED */
                    errorCode = TM_EIKENOTSTARTED;
                    break;
                }

                gIkePolicyHeadPtr = (ttIkeEntryPtr)gIkePtr->ikePolicyHeadPtr;
                while (gIkePolicyHeadPtr != TM_IKE_ENTRY_NULL_PTR)
                {
                    plcyCount = gIkePolicyHeadPtr->ikeTransCount;
                    for(j=0; j<plcyCount; j++)
                    {
                        tfNtGetIsaStateSpdEntry(gIkePolicyHeadPtr,
                                                j,
                                                &ntEntryPtr->ntIsaStateSpdEntry);
#ifdef TM_LOCK_NEEDED
                        errorCode = tfNtCallBack(
                                            &(tm_context(tvIkeLockEntry)),
                                            ntEntryCBFuncPtr,
                                            ntEntryPtr,
                                            genParam1, genParam2);
#else /* !TM_LOCK_NEEDED */
                        errorCode = (*ntEntryCBFuncPtr)(ntEntryPtr,
                                                        genParam1, 
                                                        genParam2);
#endif /* !TM_LOCK_NEEDED */
                        if (errorCode != TM_ENOERROR)
                        {
                            break;
                        }
/* Recheck because of call back function */
                        gIkePtr = (ttIkeGlobalPtr)tm_context(tvIkePtr);
                        if (gIkePtr == TM_IKE_GLOBAL_NULL_PTR)
                        {
                            break;
                        }
                    }
                    gIkePolicyHeadPtr = gIkePolicyHeadPtr->ikeNextPtr;
                }
#ifdef TM_LOCK_NEEDED
                tm_call_unlock(&(tm_context(tvIkeLockEntry)));
#endif /* TM_LOCK_NEEDED */
                break;
#endif /* TM_USE_IKE */
#ifdef TM_USE_IPSEC
            case TM_NT_TABLE_SADB:
#ifdef TM_LOCK_NEEDED
                tm_call_lock_wait(&(tm_context(tvIpsecLockEntry)));
#endif /* TM_LOCK_NEEDED */
                gIpsecPtr = (ttIpsecEntryPtr)tm_context(tvIpsecPtr);
                if (gIpsecPtr != TM_IPSEC_ENTRY_NULL_PTR)
                {
                    errorCode = TM_ENOERROR;
                    n = tm_direction_to_index(TM_IPSEC_INBOUND);
                    do
                    {
                        j = 0;
                        do
                        {
                            sadbPtr = gIpsecPtr->ipsecRecordHash[n][j];
                            while(sadbPtr)
                            {
                                tfNtGetSadbEntry(sadbPtr,
                                    &ntEntryPtr->ntSadbEntry);
                                checkSadbPtr = sadbPtr->sadbNextSameHashPtr;
#ifdef TM_LOCK_NEEDED
                                errorCode = tfNtCallBack(
                                                &(tm_context(tvIpsecLockEntry)),
                                                ntEntryCBFuncPtr,
                                                ntEntryPtr,
                                                genParam1, genParam2);
#else /* !TM_LOCK_NEEDED */
                                errorCode = (*ntEntryCBFuncPtr)(ntEntryPtr,
                                                                genParam1, 
                                                                genParam2);
#endif /* !TM_LOCK_NEEDED */
                                if (errorCode != TM_ENOERROR)
                                {
                                    break; /* out of inner loop */
                                }
/* Recheck because of call back function */
                                gIpsecPtr = (ttIpsecEntryPtr)
                                            tm_context(tvIpsecPtr);
                                if (gIpsecPtr == TM_IPSEC_ENTRY_NULL_PTR)
                                {
                                    errorCode = TM_ENOENT; /* out of outer loop */
                                    break; /* out of inner loop */
                                }
                                sadbPtr = gIpsecPtr->ipsecRecordHash[n][j];
                                while (sadbPtr)
                                {
                                    if (sadbPtr == checkSadbPtr)
                                    {
                                        break; /* next still in list */
                                    }
                                    sadbPtr = sadbPtr->sadbNextSameHashPtr;
                                }
                            }
                            j++;
                        } while (    (j < TM_IPSEC_SA_TABLE_SIZE)
                                  && (errorCode == TM_ENOERROR));
                        n++;
                    } while(   (n < tm_direction_to_index(TM_IPSEC_OUTBOUND))
                            && (errorCode == TM_ENOERROR));
                }
                else /* gIpsecPtr != TM_IPSEC_ENTRY_NULL_PTR */
                {
                    errorCode = TM_EIPSECNOTINITIALIZED;
                }
#ifdef TM_LOCK_NEEDED
                tm_call_unlock(&(tm_context(tvIpsecLockEntry)));
#endif /* TM_LOCK_NEEDED */
                break;
            case TM_NT_TABLE_IPSEC_SPD:
#ifdef TM_LOCK_NEEDED
                tm_call_lock_wait(&(tm_context(tvIpsecLockEntry)));
#endif /* TM_LOCK_NEEDED */
                gIpsecPtr = (ttIpsecEntryPtr)tm_context(tvIpsecPtr);
                if (gIpsecPtr != TM_IPSEC_ENTRY_NULL_PTR)
                {
                    plcyCount = gIpsecPtr->ipsecPlcyIndex;
                    for(j = 0; j < plcyCount; j ++)
                    {
                        errorCode =
                                tfIpsecPolicyCKLockedQueryByIndex(j, &plcyPtr);
                        if (errorCode == TM_ENOERROR)
                        {
                            tfNtGetIpsecSpdEntry(plcyPtr,
                                    &ntEntryPtr->ntIpsecSpdEntry);
/* Unget the policy pointer gotten by tfIpsecPolicyCKLockedQueryByIndex() */
                            tfPolicyFree(plcyPtr);
#ifdef TM_LOCK_NEEDED
                            errorCode = tfNtCallBack(
                                             &(tm_context(tvIpsecLockEntry)),
                                             ntEntryCBFuncPtr,
                                             ntEntryPtr,
                                             genParam1, genParam2);
#else /* !TM_LOCK_NEEDED */
                            errorCode = (*ntEntryCBFuncPtr)(ntEntryPtr,
                                                            genParam1, 
                                                            genParam2);
#endif /* !TM_LOCK_NEEDED */
                            if (errorCode != TM_ENOERROR)
                            {
                                break;
                            }
/* Recheck because of call back function */
                            gIpsecPtr = (ttIpsecEntryPtr)
                                                tm_context(tvIpsecPtr);
                            if (gIpsecPtr == TM_IPSEC_ENTRY_NULL_PTR)
                            {
                                break;
                            }
                        }
                    }
                }
                else /* gIpsecPtr != TM_IPSEC_ENTRY_NULL_PTR */
                {
                    errorCode = TM_EIPSECNOTINITIALIZED;
                }
#ifdef TM_LOCK_NEEDED
                tm_call_unlock(&(tm_context(tvIpsecLockEntry)));
#endif /* TM_LOCK_NEEDED */
                break;
#endif /* TM_USE_IPSEC */
#ifdef TM_USE_IPV4
            case TM_NT_TABLE_NAT:
                if (cacheIndex != -1)
                {
#ifdef TM_LOCK_NEEDED
                    tm_call_lock_wait(&tm_context(tvDeviceListLock));
#endif /* TM_LOCK_NEEDED */
                    devPtr = tm_context(tvDeviceList);
                    while (devPtr != TM_DEV_NULL_PTR)
                    {
                        if (devPtr->devNatNumTriggers == 0)
                        {
                            devPtr = devPtr->devNextDeviceEntry;
                            continue;
                        }
                        tm_call_lock_wait(&(devPtr->devNatLockEntry));
                        numNatTriggers = devPtr->devNatNumTriggers;
                        ntNatHeadPtr = (ttNtNatEntryPtr)0;
                        if (numNatTriggers > 0)
                        {
/* Allocate memory for the NAT trigger cache */
                            ntNatHeadPtr=
                                (ttNtNatEntryPtr)tm_get_raw_buffer(
                                    (numNatTriggers * sizeof(ttNtNatEntry)));
                            if (ntNatHeadPtr == (ttNtNatEntryPtr)0)
                            {
                                errorCode = TM_ENOBUFS;
                                break;
                            }

/* Build the NAT trigger cache */
                            ntNatCurPtr = ntNatHeadPtr;
                            for (trigPtr = devPtr->devNatTriggerHead;
                                 trigPtr != (ttNatTriggerPtr)0;
                                 trigPtr = trigPtr->ntrNextPtr)
                            {
                                tm_bzero(ntNatCurPtr, sizeof(ttNtNatEntry));
                                tfNtGetNatEntry(trigPtr, ntNatCurPtr);
                                ntNatCurPtr++;
                            }
                        }
                        tm_call_unlock(&(devPtr->devNatLockEntry));

                        devPtr = devPtr->devNextDeviceEntry;

                        if (ntNatHeadPtr != (ttNtNatEntryPtr)0)
                        {
                            ntNatCurPtr = ntNatHeadPtr;
                            while (numNatTriggers > 0)
                            {
                                tm_memcpy(&(ntEntryPtr->ntNatEntry),
                                          ntNatCurPtr,
                                          sizeof(ttNtNatEntry));
#ifdef TM_LOCK_NEEDED
                                errorCode = tfNtCallBack(
                                        &tm_context(tvDeviceListLock),
                                        ntEntryCBFuncPtr,
                                        ntEntryPtr,
                                        genParam1,
                                        genParam2);
#else /* !TM_LOCK_NEEDED */
                                errorCode = (*ntEntryCBFuncPtr)(ntEntryPtr,
                                                                genParam1, 
                                                                genParam2);
#endif /* !TM_LOCK_NEEDED */
                                if (errorCode != TM_ENOERROR)
                                {
                                    break;
                                }
/*
 * Devices are never deleted. So no need to check that device is stil
 * in the list after call to user.
 */
                                ntNatCurPtr++;
                                numNatTriggers--;
                            }
                            tm_free_raw_buffer(ntNatHeadPtr);
                        }
                    }
#ifdef TM_LOCK_NEEDED
                    tm_call_unlock(&tm_context(tvDeviceListLock));
#endif /* TM_LOCK_NEEDED */
                }
                break;
#endif /* TM_USE_IPV4 */
            default:
                errorCode = TM_EINVAL;
            }
        }
        tm_free_raw_buffer(ntEntryPtr);
    }
    else
    {
        errorCode = TM_ENOBUFS;
    }
    return errorCode; 
}

#ifdef TM_LOCK_NEEDED
static int tfNtCallBack ( ttLockEntryPtr      lockEntryPtr, 
                          ttNtEntryCBFuncPtr  ntEntryCBFuncPtr,
                          ttNtEntryUPtr       ntEntryPtr,
                          ttUserGenericUnion  genParam1,
                          ttUserGenericUnion  genParam2 )
{
    int errorCode;

#ifdef TM_LOCK_NEEDED
    tm_call_unlock(lockEntryPtr);
#endif /* TM_LOCK_NEEDED */
    errorCode = (*ntEntryCBFuncPtr)(ntEntryPtr,
                                    genParam1, 
                                    genParam2);
#ifdef TM_LOCK_NEEDED
    tm_call_lock_wait(lockEntryPtr);
#endif /* TM_LOCK_NEEDED */
    return errorCode;
}
#endif /* TM_LOCK_NEEDED */

static int tfNtTableToCacheIndex(int family, ttNtTableId tableId)
{
    int index;
    index = -1;
#ifdef TM_USE_IPV4
    if (family == AF_INET)
    {
        switch(tableId)
        {
            case TM_NT_TABLE_RTE:
                index = TM_SNMPC_ROUTE_INDEX;
                break;
            case TM_NT_TABLE_ARP:
                index = TM_SNMPC_ARP_INDEX;
                break;
            case TM_NT_TABLE_UDP:
                index = TM_SNMPC_UDP_INDEX;
                break;
            case TM_NT_TABLE_TCP:
                index = TM_SNMPC_TCP_INDEX;
                break;
            case TM_NT_TABLE_DEVICE:
            case TM_NT_TABLE_NAT:
                index = 1;
                break;
            default:
                index = -1;
        }
    }
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
    if (family == AF_INET6)
    {
        switch(tableId)
        {
            case TM_NT_TABLE_RTE:
                index = TM_SNMPC6_ROUTE_INDEX;
                break;
            case TM_NT_TABLE_ARP:
                index = TM_SNMPC6_ARP_INDEX;
                break;
            case TM_NT_TABLE_UDP:
                index = TM_SNMPC6_UDP_INDEX;
                break;
            case TM_NT_TABLE_TCP:
                index = TM_SNMPC6_TCP_INDEX;
                break;
#ifdef TM_6_USE_MIP_CN
            case TM_NT_TABLE_BINDING:
                index = 1;
                break;
#endif /* TM_6_USE_MIP_CN */
#ifndef TM_USE_IPV4
            case TM_NT_TABLE_DEVICE:
                index = 1;
                break;
#endif /* TM_USE_IPV4 */
            default:
                index = -1;
        }
    }
#endif /* TM_USE_IPV6 */
    return index;
}

/*
 * tfNtGetArpEntry() function description
 * extract information from a ttRteEntry, do necessary conversions and store 
 * it a ttNtArpEntry structure, which is more preseantation oriented, and it 
 * used to pass to the netstat call back functions.
 *
 * Parameters
 * Parameter        Description
 * snmpCacheEntryPtr pointer to an ARP SNMP cache entry to get information from
 * ntEntryPtr        pointer to ttNtEntry to store the converted infomation
 *
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Completed successfully
 *
 */
int tfNtGetArpEntry(ttSnmpCacheEntryPtr snmpCacheEntryPtr, 
                    ttNtEntryUPtr       ntEntryPtr)
{
    ttNtArpEntryPtr ntArpEntryPtr;
    ttRteEntryPtr   rtePtr;
#ifdef TM_USE_IPV6
    ttTimeStruct    curTime;
#endif /* TM_USE_IPV6 */

    rtePtr = (ttRteEntryPtr)snmpCacheEntryPtr->sceTableEntryPtr;
    ntArpEntryPtr = &(ntEntryPtr->ntArpEntry);

/* extract and convert */
    ntArpEntryPtr->ntArpHwType = TM_NT_HWTYPE_NULL;
    if (tm_4_ll_is_lan(rtePtr->rteDevPtr))
    {
        ntArpEntryPtr->ntArpHwType = TM_NT_HWTYPE_ETHERNET;
    }
    tfNtIpAddrToSockAddr(&rtePtr->rteLeafNode.rtnLSKey,
#if defined(TM_USE_IPV6) && defined(TM_USE_IPV4)
                         rtePtr->rteLeafNode.rtnPrefixLength,
#endif /* TM_USE_IPV6 && TM_USE_IPV4 */
                         &ntArpEntryPtr->ntArpSockAddr);
    ntArpEntryPtr->ntArpInterfaceHandle     
            = (ttUserInterface)(rtePtr->rteDevPtr);
    tm_memcpy(ntArpEntryPtr->ntArpHwAddress, 
              rtePtr->rteHostGwayUnion.HSEnetAdd,
              TM_MAX_PHYS_ADDR);
    ntArpEntryPtr->ntArpHwLength 
            = sizeof(rtePtr->rteHostGwayUnion.HSEnetAdd);
    tm_memcpy(ntArpEntryPtr->ntArpDeviceName, 
              rtePtr->rteDevPtr->devNameArray, 
              TM_MAX_DEVICE_NAME);
    ntArpEntryPtr->ntArpOwnerCount  = rtePtr->rteOwnerCount;
#ifdef TM_USE_IPV6
    ntArpEntryPtr->ntArpNudState = rtePtr->rte6HSNudState;
    if(rtePtr->rte6HSNudState == TM_6_NUD_REACHABLE_STATE)
    {
        tm_tmr_get_current_time(curTime);
        ntArpEntryPtr->ntArpTtl =
                rtePtr->rteDevPtr->dev6ReachableTime
                    - (curTime.timMsecs
                       - rtePtr->rteHSLastReachedTickCount);
    }
    else
#endif /* TM_USE_IPV6 */
    {
        ntArpEntryPtr->ntArpTtl = rtePtr->rteTtl;
    }
    return TM_ENOERROR;
}

/*
 * tfNtGetRteEntry() function description
 * extract information from a ttRteEntry, do necessary conversions and store 
 * it a ttNtRteEntry structure, which is more preseantation oriented, and it 
 * used to pass to the netstat call back functions.
 *
 * Parameters
 * Parameter        Description
 * snmpCacheEntryPtr pointer to a route SNMP cache entry to get information from
 * ntEntryPtr        pointer to ttNtEntry to store the converted infomation
 *
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Completed successfully
 *
 */
int tfNtGetRteEntry(ttSnmpCacheEntryPtr snmpCacheEntryPtr, 
                    ttNtEntryUPtr       ntEntryPtr)
{
    ttRteEntryPtr   rtePtr;
    ttNtRteEntryPtr ntRteEntryPtr;

    rtePtr = (ttRteEntryPtr)snmpCacheEntryPtr->sceTableEntryPtr;
    ntRteEntryPtr = &(ntEntryPtr->ntRteEntry);
/* verify parameters */

/* extract and convert */
    tfNtIpAddrToSockAddr(&rtePtr->rteDest,
#if defined(TM_USE_IPV6) && defined(TM_USE_IPV4)
                         rtePtr->rteLeafNode.rtnPrefixLength,
#endif /* TM_USE_IPV6 && TM_USE_IPV4 */
                         &ntRteEntryPtr->ntRteDestSockAddr);
#ifdef TM_USE_IPV6
    ntRteEntryPtr->ntRtePrefixLength = 
                 (tt8Bit)rtePtr->rteLeafNode.rtnPrefixLength;
#else /* TM_USE_IPV6 */
    ntRteEntryPtr->ntRtePrefixLength = 
        (tt8Bit)tf4NetmaskToPrefixLen(rtePtr->rteLeafNode.rtnLSMask);
#endif /* TM_USE_IPV6 */

    ntRteEntryPtr->ntRteOwnerCount      = rtePtr->rteOwnerCount;
    ntRteEntryPtr->ntRteMhomeIndex      = rtePtr->rteMhomeIndex;
    tm_memcpy(ntRteEntryPtr->ntRteDeviceName, 
              rtePtr->rteDevPtr->devNameArray, 
              TM_MAX_DEVICE_NAME);
    ntRteEntryPtr->ntRteInterfaceHandle 
            = (ttUserInterface)(rtePtr->rteDevPtr);

    tfNtIpAddrToSockAddr(&rtePtr->rteHostGwayUnion.GSGateway,
#if defined(TM_USE_IPV6) && defined(TM_USE_IPV4)
                         TM_6_IP_HOST_PREFIX_LENGTH, 
#endif /* TM_USE_IPV6 && TM_USE_IPV4 */
                         &ntRteEntryPtr->ntRteGwSockAddr);

    tm_memcpy(ntRteEntryPtr->ntRteHwAddress, 
              rtePtr->rteHostGwayUnion.HSEnetAdd,
              TM_MAX_PHYS_ADDR);
#ifdef TM_USE_IPV6
    ntRteEntryPtr->ntRteClonePrefixLength  
            = rtePtr->rteClonePrefixLength;
#else /* TM_USE_IPV6 */
    ntRteEntryPtr->ntRteClonePrefixLength
            = (tt8Bit)tf4NetmaskToPrefixLen(rtePtr->rteCloneMask);
#endif /* TM_USE_IPV6 */
    ntRteEntryPtr->ntRteMtu             = rtePtr->rteMtu;
    ntRteEntryPtr->ntRteHops            = rtePtr->rteHops;
    ntRteEntryPtr->ntRteTtl             = rtePtr->rteTtl;
    ntRteEntryPtr->ntRteFlags           = 0;
    if (rtePtr->rteFlags2 & TM_RTE2_UP)
    {
        ntRteEntryPtr->ntRteFlags |= TM_NT_RTE_UP;
    }
    if (rtePtr->rteFlags2 & TM_RTE2_HOST)
    {
        ntRteEntryPtr->ntRteFlags |= TM_NT_RTE_HOST;
    }
    if (rtePtr->rteFlags & TM_RTE_INDIRECT)
    {
        ntRteEntryPtr->ntRteFlags |= TM_NT_RTE_GW;
    }
    if (rtePtr->rteFlags & TM_RTE_ROUTE_DYNAMIC)
    {
        ntRteEntryPtr->ntRteFlags |= TM_NT_RTE_DYNAMIC;
    }
    if (rtePtr->rteFlags & TM_RTE_REJECT)
    {
        ntRteEntryPtr->ntRteFlags |= TM_NT_RTE_REJECT;
    }
    if (rtePtr->rteFlags & TM_RTE_REDIRECT)
    {
        ntRteEntryPtr->ntRteFlags |= TM_NT_RTE_REDIRECT;
    }
    if (rtePtr->rteFlags & TM_RTE_REJECT)
    {
        ntRteEntryPtr->ntRteFlags |= TM_NT_RTE_REJECT;
    }
    if (rtePtr->rteFlags & TM_RTE_CLONE)
    {
        ntRteEntryPtr->ntRteFlags |= TM_NT_RTE_CLONABLE;
    }
    if (rtePtr->rteFlags & TM_RTE_CLONED)
    {
        ntRteEntryPtr->ntRteFlags |= TM_NT_RTE_CLONED;
    }
    if (rtePtr->rteFlags & TM_RTE_STATIC)
    {
        ntRteEntryPtr->ntRteFlags |= TM_NT_RTE_STATIC;
    }
    if (rtePtr->rteFlags & TM_RTE_LINK_LAYER)
    {
        ntRteEntryPtr->ntRteFlags |= TM_NT_RTE_LINK_LAYER;
    }
    return TM_ENOERROR;
}

/*
 * tfNtGetTcpEntry() function description
 * extract information from a ttSocketEntry, do necessary conversions and store 
 * it a ttNtTcpEntry structure, which is more preseantation oriented, and it 
 * used to pass to the netstat call back functions.
 *
 * Parameters
 * Parameter        Description
 * snmpCacheEntryPtr pointer to a TCP vector/socket SNMP cache entry to get
 *                   information from
 * ntEntryPtr        pointer to ttNtEntry to store the converted infomation
 *
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Completed successfully
 *
 */
int tfNtGetTcpEntry(ttSnmpCacheEntryPtr snmpCacheEntryPtr, 
                    ttNtEntryUPtr       ntEntryPtr)
{
    ttSocketEntryPtr socketPtr;
    ttNtTcpEntryPtr  ntTcpEntryPtr;
    ttTupleDevPtr    tupleDevPtr;
    ttTcpVectPtr     tcpVectPtr;
    ttTcpTmWtVectPtr tcpTmWtVectPtr;

    ntTcpEntryPtr = &(ntEntryPtr->ntTcpEntry);
    tupleDevPtr = (ttTupleDevPtr)(snmpCacheEntryPtr->sceParmPtr);
    if (snmpCacheEntryPtr->sceTableEntryType != TM_SNMPC_TCP_TIME_WAIT_TYPE)
    {
        tcpVectPtr = (ttTcpVectPtr)(snmpCacheEntryPtr->sceTableEntryPtr);
        socketPtr = &(tcpVectPtr->tcpsSocketEntry);
        ntTcpEntryPtr->ntTcpRecvQSize       = socketPtr->socMaxRecvQueueBytes;
        ntTcpEntryPtr->ntTcpSendQSize       = socketPtr->socMaxSendQueueBytes;
        ntTcpEntryPtr->ntTcpBackLog         = socketPtr->socBackLog;
        ntTcpEntryPtr->ntTcpOwnerCount      = socketPtr->socOwnerCount;       
        ntTcpEntryPtr->ntTcpSockDesc        = socketPtr->socIndex;
        ntTcpEntryPtr->ntTcpFlags           = tcpVectPtr->tcpsFlags;           

        if (tcpVectPtr->tcpsState != TM_TCPS_LISTEN)
        {
            ntTcpEntryPtr->ntTcpBytesInRecvQ    = socketPtr->socRecvQueueBytes;   
            ntTcpEntryPtr->ntTcpBytesInSendQ    = socketPtr->socSendQueueBytes;   
            ntTcpEntryPtr->ntTcpRto             = tcpVectPtr->tcpsRto;
            ntTcpEntryPtr->ntTcpReXmitCnt       = (tt32Bit)tcpVectPtr->tcpsReXmitCnt;
            ntTcpEntryPtr->ntTcpSndUna          = tcpVectPtr->tcpsSndUna;         
            ntTcpEntryPtr->ntTcpSndNxt          = tcpVectPtr->tcpsSndNxt;         
            ntTcpEntryPtr->ntTcpMaxSndNxt       = tcpVectPtr->tcpsMaxSndNxt;      
            ntTcpEntryPtr->ntTcpIss             = tcpVectPtr->tcpsIss;            
            ntTcpEntryPtr->ntTcpIrs             = tcpVectPtr->tcpsIrs;            
            ntTcpEntryPtr->ntTcpSndWL1          = tcpVectPtr->tcpsSndWL1;         
            ntTcpEntryPtr->ntTcpSndWL2          = tcpVectPtr->tcpsSndWL2;         
            ntTcpEntryPtr->ntTcpMaxSndWnd       = tcpVectPtr->tcpsMaxSndWnd;      
            ntTcpEntryPtr->ntTcpRcvNxt          = tcpVectPtr->tcpsRcvNxt;         
            ntTcpEntryPtr->ntTcpRcvWnd          = tcpVectPtr->tcpsRcvWnd;         
            ntTcpEntryPtr->ntTcpRcvAdv          = tcpVectPtr->tcpsRcvAdv;         
            ntTcpEntryPtr->ntTcpCwnd            = tcpVectPtr->tcpsCwnd;           
            ntTcpEntryPtr->ntTcpSsthresh        = tcpVectPtr->tcpsSsthresh;       
            ntTcpEntryPtr->ntTcpDupAck          = tcpVectPtr->tcpsDupAcks;     
            ntTcpEntryPtr->ntTcpAcksAfterRexmit = tcpVectPtr->tcpsAcksAfterRexmit;
        }
/* 
 * internal TCP state constants are not acceeable to the user,
 * mapping to the user acceable constants here 
 */
        switch(tcpVectPtr->tcpsState)
        {
            case TM_TCPS_CLOSED:
                ntTcpEntryPtr->ntTcpState = TM_NT_TCPS_CLOSED;
                break;
            case TM_TCPS_LISTEN:
                ntTcpEntryPtr->ntTcpState = TM_NT_TCPS_LISTEN;
                break;
            case TM_TCPS_SYN_SENT:    
                ntTcpEntryPtr->ntTcpState = TM_NT_TCPS_SYN_SENT;    
                break;
            case TM_TCPS_SYN_RECEIVED:
                ntTcpEntryPtr->ntTcpState = TM_NT_TCPS_SYN_RECEIVED;
                break;
            case TM_TCPS_ESTABLISHED: 
                ntTcpEntryPtr->ntTcpState = TM_NT_TCPS_ESTABLISHED;  
                break;
            case TM_TCPS_CLOSE_WAIT:  
                ntTcpEntryPtr->ntTcpState = TM_NT_TCPS_CLOSE_WAIT;  
                break;
            case TM_TCPS_FIN_WAIT_1:  
                ntTcpEntryPtr->ntTcpState = TM_NT_TCPS_FIN_WAIT_1;  
                break;
            case TM_TCPS_CLOSING:     
                ntTcpEntryPtr->ntTcpState = TM_NT_TCPS_CLOSING;     
                break;
            case TM_TCPS_LAST_ACK:    
                ntTcpEntryPtr->ntTcpState = TM_NT_TCPS_LAST_ACK;     
                break;
            case TM_TCPS_FIN_WAIT_2:  
                ntTcpEntryPtr->ntTcpState = TM_NT_TCPS_FIN_WAIT_2;  
                break;
                break;
            case TM_TCPS_INVALID:     
                ntTcpEntryPtr->ntTcpState = TM_NT_TCPS_INVALID;      
                break;
            default:
                ntTcpEntryPtr->ntTcpState = TM_NT_TCPS_INVALID;      
        }
    }
    else
    {
        tcpTmWtVectPtr =
                 (ttTcpTmWtVectPtr)(snmpCacheEntryPtr->sceTableEntryPtr);
        ntTcpEntryPtr->ntTcpState = TM_NT_TCPS_TIME_WAIT;   
        ntTcpEntryPtr->ntTcpOwnerCount = 1;       
        ntTcpEntryPtr->ntTcpSockDesc   = TM_SOC_NO_INDEX;
        ntTcpEntryPtr->ntTcpSndUna     = tcpTmWtVectPtr->twsMaxSndNxt;
        ntTcpEntryPtr->ntTcpSndNxt     = tcpTmWtVectPtr->twsMaxSndNxt;        
        ntTcpEntryPtr->ntTcpMaxSndNxt  = tcpTmWtVectPtr->twsMaxSndNxt;
/* Next expected peer seq no */
        ntTcpEntryPtr->ntTcpRcvNxt     = tcpTmWtVectPtr->twsRcvNxt;
        ntTcpEntryPtr->ntTcpSndWL1     = tcpTmWtVectPtr->twsRcvNxt;         
        ntTcpEntryPtr->ntTcpSndWL2     = tcpTmWtVectPtr->twsMaxSndNxt;         
    }
/* the local sock addr information */    
    tfNtIpAddrToSockAddr(&(tupleDevPtr->tudTuple.sotLocalIpAddress),
#if defined(TM_USE_IPV6) && defined(TM_USE_IPV4)
                         TM_6_IP_HOST_PREFIX_LENGTH, 
#endif /* TM_USE_IPV6 && TM_USE_IPV4 */
                         &ntTcpEntryPtr->ntTcpLocalSockAddr);
    ntTcpEntryPtr->ntTcpLocalSockAddr.ss_port =
                                          tupleDevPtr->tudTuple.sotLocalIpPort;
/* the Peer sock addr information */    
    tfNtIpAddrToSockAddr(&(tupleDevPtr->tudTuple.sotRemoteIpAddress),
#if defined(TM_USE_IPV6) && defined(TM_USE_IPV4)
                         TM_6_IP_HOST_PREFIX_LENGTH, 
#endif /* TM_USE_IPV6 && TM_USE_IPV4 */
                         &ntTcpEntryPtr->ntTcpPeerSockAddr);
    ntTcpEntryPtr->ntTcpPeerSockAddr.ss_port =
                                         tupleDevPtr->tudTuple.sotRemoteIpPort;
    return TM_ENOERROR;
}

/*
 * tfNtGetUdpEntry() function description
 * extract information from a ttSocketEntry, do necessary conversions and store 
 * it a ttNtUdpEntry structure, which is more preseantation oriented, and it 
 * used to pass to the netstat call back functions.
 *
 * Parameters
 * Parameter        Description
 * snmpCacheEntryPtr pointer to a UDP socket SNMP cache entry to get
 *                   information from
 * ntEntryPtr        pointer to ttNtEntry to store the converted infomation
 *
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Completed successfully
 *
 */
int tfNtGetUdpEntry(ttSnmpCacheEntryPtr snmpCacheEntryPtr, 
                    ttNtEntryUPtr       ntEntryPtr)
{
    ttSocketEntryPtr    socketPtr;
    ttTupleDevPtr       tupleDevPtr;
    ttNtUdpEntryPtr     ntUdpEntryPtr;

    ntUdpEntryPtr = &(ntEntryPtr->ntUdpEntry);
    socketPtr = (ttSocketEntryPtr)snmpCacheEntryPtr->sceTableEntryPtr;
    tupleDevPtr = (ttTupleDevPtr)(snmpCacheEntryPtr->sceParmPtr);

    ntUdpEntryPtr->ntUdpBytesInRecvQ = socketPtr->socRecvQueueBytes;
    ntUdpEntryPtr->ntUdpBytesInSendQ = socketPtr->socSendQueueBytes;   
    ntUdpEntryPtr->ntUdpRecvQSize    = socketPtr->socMaxRecvQueueBytes;
    ntUdpEntryPtr->ntUdpSendQSize    = socketPtr->socMaxSendQueueBytes;

/* the local sock addr information */    
    tfNtIpAddrToSockAddr(&socketPtr->socOurIfIpAddress,
#if defined(TM_USE_IPV6) && defined(TM_USE_IPV4)
                         TM_6_IP_HOST_PREFIX_LENGTH, 
#endif /* TM_USE_IPV6 && TM_USE_IPV4 */
                         &ntUdpEntryPtr->ntUdpLocalSockAddr);
    ntUdpEntryPtr->ntUdpLocalSockAddr.ss_port
                                         = socketPtr->socOurLayer4Port;
    
    ntUdpEntryPtr->ntUdpOwnerCount   = (tt16Bit)socketPtr->socOwnerCount;
    ntUdpEntryPtr->ntUdpSockDesc     = socketPtr->socIndex;
/* the local sock addr information */    
    tfNtIpAddrToSockAddr(&(tupleDevPtr->tudTuple.sotLocalIpAddress),
#if defined(TM_USE_IPV6) && defined(TM_USE_IPV4)
                         TM_6_IP_HOST_PREFIX_LENGTH, 
#endif /* TM_USE_IPV6 && TM_USE_IPV4 */
                         &ntUdpEntryPtr->ntUdpLocalSockAddr);
    ntUdpEntryPtr->ntUdpLocalSockAddr.ss_port
                                        = tupleDevPtr->tudTuple.sotLocalIpPort;
    return TM_ENOERROR;
}

/*
 * tfNtGetBindingEntry() function description
 * extract information from a ttBindingEntry, do necessary conversions and store 
 * it a tt6NtBindingEntry structure, which is more preseantation oriented, and it 
 * used to pass to the netstat call back functions.
 *
 * Parameters
 * Parameter        Description
 * bindingPtr       pointer to a socket entry to get information from, 
 *                  the socket entry must actually be a ttBindingVect, which is 
 *                  ttSocketEntry + Binding specific fields.
 * ntBindingPtr     pointer to tt6NtBindingEntry to store the converted infomation
 *
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Completed successfully
 * TM_EINVAL        One of the passed in pointers is null
 *
 */
#ifdef TM_6_USE_MIP_CN
int tfNtGetBindingEntry(
    tt6BindingEntryPtr   bindingPtr, 
    tt6NtBindingEntryPtr ntBindingPtr)
{
    int errorCode;

    errorCode = TM_ENOERROR;
/* verity parameters */
    if (   bindingPtr  == TM_6_BINDING_NULL_PTR
        || ntBindingPtr == (tt6NtBindingEntryPtr)0)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        ntBindingPtr->nt6BindHomeAddr        = bindingPtr->bind6HomeAddr;    
        ntBindingPtr->nt6BindCoAddr          = bindingPtr->bind6CoAddr;      
        ntBindingPtr->nt6BindLifetime        = bindingPtr->bind6Lifetime;    
        ntBindingPtr->nt6BindLastSeq         = bindingPtr->bind6LastSeq;     
        ntBindingPtr->nt6BindLastSendTime    = bindingPtr->bind6LastSendTime;
        ntBindingPtr->nt6BindLastRecvTime    = bindingPtr->bind6LastRecvTime;
        ntBindingPtr->nt6BindLastUpdatedTime = bindingPtr->bind6LastUpdatedTime;
#ifdef TM_6_USE_MIP_HA
        ntBindingPtr->nt6BindHomeReg         = bindingPtr->bind6HomeReg;
        ntBindingPtr->nt6BindProxyMHome      = bindingPtr->bind6ProxyMHome;
#endif /* TM_6_USE_MIP_HA */

    }
    return errorCode;
}
#endif /* TM_6_USE_MIP_CN */



/*
 * tfNtGetDeviceEntry() function description
 * extract information from a ttDeviceEntry, do necessary conversions and store 
 * it a tt6NtDevEntry structure, which is more preseantation oriented, and it 
 * used to pass to the netstat call back functions.
 *
 * Parameters
 * Parameter    Description
 * devPtr       pointer to a socket entry to get information from, 
 * ntDevPtr     pointer to tt6NtDevEntry to store the converted infomation
 *
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Completed successfully
 * TM_EINVAL        One of the passed in pointers is null
 *
 */
int tfNtGetDeviceEntry(
    ttDeviceEntryPtr   devPtr, 
    ttNtDevEntryPtr    ntDevPtr)
{
    int     errorCode;
#if (defined(TM_USE_IPV6) || !defined(TM_SINGLE_INTERFACE_HOME))
    tt16Bit i;  /* address index */
#endif /* multiple addresses */
#ifdef TM_USE_IPV4
#ifndef TM_SINGLE_INTERFACE_HOME
    tt8Bit addrType;
#endif /* !TM_SINGLE_INTERFACE_HOME */
#endif /* TM_USE_IPV4 */

    errorCode = TM_ENOERROR;

    if (   devPtr  == (ttDeviceEntryPtr)0
        || ntDevPtr == (ttNtDevEntryPtr)0)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        tm_memcpy(  
            ntDevPtr->ntDevName, 
            devPtr->devNameArray, 
            TM_MAX_DEVICE_NAME);

        tm_memcpy(  
            ntDevPtr->ntDevPhyAddr, 
            devPtr->devPhysAddrArr,
            devPtr->devPhysAddrLength);

        if (devPtr->devFlag & TM_DEV_OPENED)
        {
            ntDevPtr->ntDevStatus = TM_NT_DEVICE_STATUS_OPEN;
        }
        else
        {
            ntDevPtr->ntDevStatus = TM_NT_DEVICE_STATUS_CLOSED;
        }
        if (devPtr->devFlag & TM_DEV_CONNECTING)
        {
            ntDevPtr->ntDevStatus = TM_NT_DEVICE_STATUS_CONNECTING;
        }
        if (devPtr->devFlag & TM_DEV_CONNECTED)
        {
            ntDevPtr->ntDevStatus = TM_NT_DEVICE_STATUS_CONNECTED;
        }

        ntDevPtr->ntDevPhyAddrLen = devPtr->devPhysAddrLength;

#ifdef TM_USE_IPV4
/* copy the IPv4 addresses over */
#ifdef TM_SINGLE_INTERFACE_HOME
            tm_ip_copy(
                devPtr->devIpAddr,
                ntDevPtr->ntDevAddrConfArray[0].ntAddrAddr.s_addr);

            ntDevPtr->ntDevAddrConfArray[0].ntAddrPrefixLen = 
                devPtr->dev4PrefixLen;
#else /* ! TM_SINGLE_INTERFACE_HOME */
        for(i=0; i < TM_NT_MAX_IP_PER_IF; i++)
        {
            if (i >= TM_MAX_IPS_PER_IF)
            {
                tm_bzero(
                    &ntDevPtr->ntDevAddrConfArray[i], 
                    sizeof(ttNtAddrEntry));

                continue;
            }
            tm_ip_copy(
                devPtr->devIpAddrArray[i],  
                ntDevPtr->ntDevAddrConfArray[i].ntAddrAddr.s_addr);

            ntDevPtr->ntDevAddrConfArray[i].ntAddrMHome = (tt8Bit)i;
            ntDevPtr->ntDevAddrConfArray[i].ntAddrPrefixLen = 
                devPtr->dev4PrefixLenArray[i];

            if (tm_ip_dev_conf_flag(devPtr, i) & TM_DEV_IP_CONFIG)
            {
                ntDevPtr->ntDevAddrConfArray[i].ntAddrStatus =
                    TM_NT_ADDR_STATUS_CONFIGURED;
                if (tm_8bit_one_bit_set(tm_ip_dev_conf_flag(devPtr, i),
                                        TM_DEV_IP_DHCP) )
                {
                    addrType = TM_NT_ADDR_CONF_TYPE_DHCP;
                }
                else if (tm_8bit_one_bit_set(tm_ip_dev_conf_flag(devPtr, i),
                                             TM_DEV_IP_BOOTP) )
                {
                    addrType = TM_NT_ADDR_CONF_TYPE_BOOTP;
                }
                else
                {
                    addrType = TM_NT_ADDR_CONF_TYPE_MANUAL;
                }
            }
            else
            {
                addrType = TM_NT_ADDR_CONF_TYPE_NONE;
            }
            ntDevPtr->ntDevAddrConfArray[i].ntAddrType = addrType;
        }
#endif /* ! TM_SINGLE_INTERFACE_HOME */
#endif /* TM_USE_IPV4 */


#ifdef TM_USE_IPV6
/* copy the IPv6 address information over */
        for (i=0; i < TM_6_MAX_MHOME; i++)
        {
            if (!(devPtr->dev6IpAddrFlagsArray[i]
                  & (TM_6_DEV_IP_CONFIG | TM_6_DEV_IP_CONF_STARTED)))
            {
/* if this address isn't configured or pending configuration, then skip it */
                ntDevPtr->ntDev6AddrConfArray[i].nt6AddrType = 
                    TM_NT_ADDR_CONF_TYPE_NONE;
                continue;
            }

            if (i < devPtr->dev6MhomeUsedEntries)
            {
                ntDevPtr->ntDev6AddrConfArray[i].nt6AddrType = 
                    TM_NT_ADDR_CONF_TYPE_MANUAL;
            }
            else if (i <  TM_MAX_IPS_PER_IF)
            {
                ntDevPtr->ntDev6AddrConfArray[i].nt6AddrType = 
                    TM_NT_ADDR_CONF_TYPE_NONE;
                continue;
            }
            else if (i < devPtr->dev6MhomeUsedAutoEntries)
            {
                ntDevPtr->ntDev6AddrConfArray[i].nt6AddrType = 
                    TM_NT_ADDR_CONF_TYPE_AUTO;
            }
            else if (i <  TM_MAX_IPS_PER_IF 
                        + TM_6_MAX_AUTOCONF_IPS_PER_IF)
            {
                ntDevPtr->ntDev6AddrConfArray[i].nt6AddrType = 
                    TM_NT_ADDR_CONF_TYPE_NONE;
                continue;
            }
#ifdef TM_6_USE_DHCP
            else if (i < devPtr->dev6MhomeUsedDhcpEntries)
            {
                ntDevPtr->ntDev6AddrConfArray[i].nt6AddrType = 
                    TM_NT_ADDR_CONF_TYPE_DHCPV6;
            }
#endif /* TM_6_USE_DHCP */
#ifdef TM_6_USE_MIP_HA
            else if (i < devPtr->dev6MhomeUsedProxyEntries)
            {
                ntDevPtr->ntDev6AddrConfArray[i].nt6AddrType = 
                    TM_NT_ADDR_CONF_TYPE_PROXY;
            }
#endif /* TM_6_USE_MIP_HA */
            else
            {
                ntDevPtr->ntDev6AddrConfArray[i].nt6AddrType = 
                    TM_NT_ADDR_CONF_TYPE_NONE;
                continue;
            }

            ntDevPtr->ntDev6AddrConfArray[i].nt6AddrMHome = (tt8Bit)i;

            tm_6_ip_copy(
                &devPtr->dev6IpAddrArray[i], 
                &ntDevPtr->ntDev6AddrConfArray[i].nt6AddrAddr);

            ntDevPtr->ntDev6AddrConfArray[i].nt6AddrPrefixLen = 
                devPtr->dev6PrefixLenArray[i];

            if (devPtr->dev6IpAddrFlagsArray[i] & TM_6_DEV_IP_DEPRECATED_FLAG)
            {
                ntDevPtr->ntDev6AddrConfArray[i].nt6AddrStatus = 
                    TM_NT_ADDR_STATUS_DEPRECATED;
            }
            else if (devPtr->dev6IpAddrFlagsArray[i] & TM_6_DEV_IP_CONF_STARTED)
            {
                ntDevPtr->ntDev6AddrConfArray[i].nt6AddrStatus = 
                    TM_NT_ADDR_STATUS_CONFIGURING;
            }
            else if (devPtr->dev6IpAddrFlagsArray[i] & TM_6_DEV_IP_CONFIG)
            {
                ntDevPtr->ntDev6AddrConfArray[i].nt6AddrStatus = 
                    TM_NT_ADDR_STATUS_CONFIGURED;
            }
            else
            {
                ntDevPtr->ntDev6AddrConfArray[i].nt6AddrStatus = 
                    TM_NT_ADDR_STATUS_UNKOWN;
            }
        }
#endif /* TM_USE_IPV6 */

    }
    return errorCode;
}

/*
 * tfNtRteFlagToStr() function description
 * Interpret the route flags and represent it is a char buffer for outputting.
 *
 * Parameters
 * Parameter        Description
 * flags            in: the route flags
 * buffer           value result: string representation of the flags, 
 *                  not to be allocated to be at least TM_NT_FLAGS_STR_LEN bytes.
 * Returns          the string representation
 */
char TM_FAR * tfNtRteFlagToStr(ttUser16Bit flags, char TM_FAR * buffer)
{
    int             flagsIndex;
    char TM_FAR *   flagCharPtr;
    char TM_FAR *   bufferBase;

    flagCharPtr = "UHGDJRCLS";
    bufferBase = buffer;

    tm_memset(buffer, 0, TM_NT_FLAGS_STR_LEN);

    for (flagsIndex=1; flagsIndex<= TM_NT_RTE_STATIC; flagsIndex*=2)
    {
        if (flags & flagsIndex)
        {
            *buffer++ = *flagCharPtr++;
        }
        else
        {
            *buffer++ = ' ';
            flagCharPtr++;
        }
    }    
    return bufferBase;
}

/*
 * tfNtHwAddrToStr() function description
 * convert a HW address into a colon seprated Hex string representation
 *
 * Parameters
 * Parameter        Description
 * hwAddrPtr        in: the HW address to be converted
 * hwAddrLen        in: the length of the HW address
 * buffer           value result: buffer for storinf the converted string, 
 *                  must be allocated to be at least hwAddrLen bytes 
 * Returns          the string representation of the HW address
 */
char TM_FAR * tfNtHwAddrToStr(
    ttUser8BitPtr   hwAddrPtr, 
    int             hwAddrLen, 
    char TM_FAR *   buffer)
{
    int            i;
    char TM_FAR *  bufferBase;

    if (   hwAddrPtr == TM_8BIT_NULL_PTR
        || buffer == (char TM_FAR *) 0 )
    {
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        if (hwAddrLen <= 0)
        {
            buffer[0] = 0;
            bufferBase = (char TM_FAR *)0;
        }
        else
        {
            bufferBase = buffer;
            tm_sprintf(buffer, "%02X", *hwAddrPtr++);
            buffer +=2;
            for (i=1; i< hwAddrLen; i++)
            {
                tm_sprintf(buffer, ":%02X", *hwAddrPtr++);
                buffer +=3;
            }
        }
    }
    return bufferBase;
}

/*
 * tfNtTtlToStr() function description
 * convert a TTL value into string representation,
 * the result is "INF" if the ttl value is TM_RTE_INF.
 * otherwise it woold be the integer value of ttl.
 *
 * Parameters
 * Parameter        Description
 * ttl              in: the TTL value to be converted
 * buffer           value result: buffer for storing the converted string, 
 *                  must be allocated to be at least TM_NT_TTL_STR_LEN bytes 
 * Returns          the string representation of the TTL value
 */
static char TM_FAR * tfNtTtlToStr(ttUser32Bit ttl, char TM_FAR * buffer)
{
    if (ttl != TM_RTE_INF)
    {
        tm_sprintf(buffer, "%ld", ttl);
    }
    else
    {
        tm_strcpy(buffer, "INF");
    }
    buffer[tm_strlen(buffer)] = ' ';
    return buffer;
}

/*
 * tfNtGetTcpStateStr() function description
 * convert a TCP state value into string representation,
 *
 * Parameters
 * Parameter        Description
 * ttl              in: the TCP state value to be converted
 *                  must be allocated to be at least TM_NT_TTL_STR_LEN bytes 
 * Returns          the string representation of the TCP  state
 */
const char TM_FAR * tfNtGetTcpStateStr(ttUser8Bit state)
{
    char TM_FAR * strPtr;
    switch(state)
    {
        case TM_TCPS_CLOSED:
            strPtr = "CLOSED";
            break;
        case TM_TCPS_LISTEN:
            strPtr = "LISTEN";
            break;
        case TM_TCPS_SYN_SENT:    
            strPtr = "SYN_SENT";    
            break;
        case TM_TCPS_SYN_RECEIVED:
            strPtr = "SYN_RECV";
            break;
        case TM_TCPS_ESTABLISHED: 
            strPtr = "ESTABLISHED";  
            break;
        case TM_TCPS_CLOSE_WAIT:  
            strPtr = "CLOSE_WAIT";  
            break;
        case TM_TCPS_FIN_WAIT_1:  
            strPtr = "FIN_WAIT1";  
            break;
        case TM_TCPS_CLOSING:     
            strPtr = "CLOSING";     
            break;
        case TM_TCPS_LAST_ACK:    
            strPtr = "LAST_ACK";     
            break;
        case TM_TCPS_FIN_WAIT_2:  
            strPtr = "FIN_WAIT2";  
            break;
        case TM_TCPS_TIME_WAIT:   
            strPtr = "TIME_WAIT";   
            break;
        case TM_TCPS_INVALID:     
            strPtr = "INVALID";      
            break;
        default:
            strPtr = "INVALID";      
    }
    return strPtr;
}

/* printf sockaddr_storage in the format of ipv4:port or [ipv6]:port */
static int tfNtSockAddrToStr(
    struct sockaddr_storage TM_FAR *    sockAddrPtr,
    char TM_FAR *                       buffer)
{
    int len;

#ifdef TM_USE_IPV6
    if (sockAddrPtr->ss_family == AF_INET6)
    {
        buffer[0] = '[';
        buffer ++;
        buffer += tfNtSockAddrToIpAddrStr(sockAddrPtr, buffer);
        buffer[0] = ']';
        buffer ++;
    }
    else
#endif /* TM_USE_IPV6 */
    {
        buffer += tfNtSockAddrToIpAddrStr(sockAddrPtr, buffer);
    }
    tm_sprintf(buffer, ":%u", ntohs(sockAddrPtr->ss_port));
    len = (int) tm_strlen(buffer);
    buffer[len]=' ';
    return len;
}

/* 
 * print the address/prefix combination into the buffer and 
 * return the number of char printed 
 */
static int tfNtSockAddrNPrefixLenToStr(
    struct sockaddr_storage TM_FAR *    sockAddrPtr,
    int                                 prefixLen,
    char TM_FAR *                       buffer)
{
    int len;

#ifdef TM_USE_IPV6
    if (sockAddrPtr->ss_family == AF_INET) 
    {
        if (prefixLen >= TM_6_IP_V4COMPAT_PREFIX_LENGTH)
        {
            prefixLen -= TM_6_IP_V4COMPAT_PREFIX_LENGTH;
        }
        else
        {
/* Default IPv4 gateway */
            prefixLen = 0;
        }
    }
#endif /* TM_USE_IPV6 */

    len =  tfNtSockAddrToIpAddrStr(sockAddrPtr, buffer);
    len += tm_sprintf(buffer + len, "/%d", prefixLen);
    buffer[len] = ' ';
    return len;
}

static int tfNtSockAddrToIpAddrStr(
    struct sockaddr_storage TM_FAR *    sockAddrPtr,
    char TM_FAR *                       buffer)
{
    *buffer = '\0'; /* in case ss_family is bogus */
#ifdef TM_USE_IPV6
    if (sockAddrPtr->ss_family == AF_INET6)
    {
        tm_6_dev_unscope_addr(&sockAddrPtr->addr.ipv6.sin6_addr);
        inet_ntop(AF_INET6,
                  &sockAddrPtr->addr.ipv6.sin6_addr,
                  buffer,
                  INET6_ADDRSTRLEN);
    }
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
    if (sockAddrPtr->ss_family == AF_INET)
    {
        inet_ntop(AF_INET,
                  &sockAddrPtr->addr.ipv4.sin_addr,
                  buffer,
                  INET_ADDRSTRLEN);
    }
#endif /* TM_USE_IPV4 */
    return (int) tm_strlen(buffer);
}

static void tfNtIpAddrToSockAddr(
    ttIpAddressPtr              ipAddrPtr,
#if defined(TM_USE_IPV6) && defined(TM_USE_IPV4)
    tt16Bit                     prefixLength,
#endif /* TM_USE_IPV6 && TM_USE_IPV4 */
    struct sockaddr_storage TM_FAR *   sockAddrPtr)
{
    
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
/* dual mode */
    if (    IN6_IS_ADDR_V4MAPPED(ipAddrPtr)
         || (    (prefixLength == TM_6_DEF_4GWAY_PREFIX_LENGTH)
             && (tm_6_ip_zero(ipAddrPtr)) ) )
    {
/* 
 * this is a V4 mapped address, or IPv4 default gateway let's get the
 * V4 part only 
 */
        sockAddrPtr->addr.ipv4.sin_addr.s_addr = ipAddrPtr->ip6Addr.ip6U32[3];
        sockAddrPtr->ss_family = AF_INET;
    }
    else
#endif /* TM_USE_IPV4 */
/* V6 only */
    {
        tm_6_ip_copy_dststruct(ipAddrPtr, sockAddrPtr->addr.ipv6.sin6_addr);
        sockAddrPtr->ss_family = AF_INET6;
    }
#else /* TM_USE_IPV6 */
/* V4 only */
        sockAddrPtr->addr.ipv4.sin_addr.s_addr = *ipAddrPtr;
        sockAddrPtr->ss_family = AF_INET;
#endif /* TM_USE_IPV6 */
}

static const char TM_FAR * tfNtGetNudStateStr(tt8Bit state)
{
#ifdef TM_USE_IPV6    
    switch(state)
    {
        case TM_6_NUD_REACHABLE_STATE:
            return "RCHBL";
        case TM_6_NUD_STALE_STATE:
            return "STALE";
        case TM_6_NUD_DELAY_STATE:
            return "DELAY";
        case TM_6_NUD_PROBE_STATE:
            return "PROBE";
        case TM_6_NUD_IPV4_STATE:
            return "IPV4";
        case TM_6_NUD_CLONED_STATE:
            return "STATC";
        default:
            return "INVAL";
    }
#else /* TM_USE_IPV6 */
    TM_UNREF_IN_ARG(state);
    return "IPV4";
#endif /* TM_USE_IPV6 */
}

#ifdef TM_ERROR_CHECKING
#ifdef TM_DEBUG_LOGGING
int tfxNtArpEntryCBLog(
    ttNtEntryUPtr       ntEntryUPtr,
    ttUserGenericUnion  genParam1,
    ttUserGenericUnion  genParam2)
{
    ttCharPtr buffer;
    int       entryStrLen;

    TM_UNREF_IN_ARG(genParam1);
    TM_UNREF_IN_ARG(genParam2);
    entryStrLen = TM_NT_ENTRY_STR_LEN;

    buffer = (ttCharPtr)tm_get_raw_buffer(TM_NT_ENTRY_STR_LEN);
    
    if (buffer != (char TM_FAR *) 0)
    {
        tfNtArpEntryToStr(&ntEntryUPtr->ntArpEntry,
                          buffer,
                          &entryStrLen);
        tm_debug_log0(buffer);

        tm_free_raw_buffer((ttRawBufferPtr)(ttVoidPtr)buffer);
    }

/* 
 * return TM_ENOERROR   to ask tfNetStat to call back with more entries 
 * return other codes to ask tfNetStat to stop calling back
 */
    return TM_ENOERROR;
}

void tfxLogArpTable(const char * msg)
{
    ttCharPtr           buffer;
    ttCharPtr           arpStrPtr;
    int                 headerStrLen;
    ttUserGenericUnion  genUnion;

    genUnion.gen8BitParm = TM_8BIT_ZERO;;

/* print the ARP table using netstat tools */
    
    buffer = (ttCharPtr)tm_get_raw_buffer(TM_NT_ENTRY_STR_LEN);
    
    if (buffer != (ttCharPtr)0)
    {
        tm_sprintf(buffer, "\n%s: -- ARP table --", msg);
        tm_debug_log0(buffer);

        tm_sprintf(
            buffer, "tvRteLimboCount = %d", tm_context(tvRteLimboCount) );
        tm_debug_log0(buffer);

        headerStrLen = TM_NT_ENTRY_STR_LEN;
        arpStrPtr = tfNtGetArpHeaderStr(buffer, &headerStrLen);
        if (    ( arpStrPtr != (ttCharPtr)0 )
/* 
 * The stack calls tfLogAllocBuf with TM_DEF_MAX_LOG_MSG_LEN, so we
 * can use this macro for comparison. Otherwise we would have to change it to
 *             && ( headerStrLen < 
 *                       (int)tm_context(tvLogCtrlBlkPtr)->lcbBufEndPadLen
 *                     - (int)(sizeof(ttLogMsgHdr) - TM_LOG_MSG_HDR_PAD_LEN) ) )
 */
             && ( headerStrLen < TM_DEF_MAX_LOG_MSG_LEN) )
        {
/* If ARP netstat message does not exceed maximum log message size */
            tm_debug_log0(buffer);
            (void) tfNetStat(
                TM_NT_TABLE_ARP, tfxNtArpEntryCBLog, genUnion, genUnion);
        }
        tm_free_raw_buffer((ttRawBufferPtr)(ttVoidPtr)buffer);
    }
}

int tfxNtRteEntryCBLog(
    ttNtEntryUPtr       ntEntryUPtr,
    ttUserGenericUnion  genParam1,
    ttUserGenericUnion  genParam2)
{
    ttCharPtr buffer;
    int       entryStrLen;

    TM_UNREF_IN_ARG(genParam1);
    TM_UNREF_IN_ARG(genParam2);
    entryStrLen = TM_NT_ENTRY_STR_LEN;

    buffer = (ttCharPtr)tm_get_raw_buffer(TM_NT_ENTRY_STR_LEN);
    
    if (buffer != (char TM_FAR *) 0)
    {
        tfNtRteEntryToStr(&ntEntryUPtr->ntRteEntry,
                          buffer,
                          &entryStrLen);
        tm_debug_log0(buffer);

        tm_free_raw_buffer((ttRawBufferPtr)(ttVoidPtr)buffer);
    }

/* 
 * return TM_ENOERROR   to ask tfNetStat to call back with more entries 
 * return other codes to ask tfNetStat to stop calling back
 */
    return TM_ENOERROR;
}

void tfxLogRteTable(const char * msg)
{
    ttCharPtr           buffer;
    ttCharPtr           arpStrPtr;
    int                 headerStrLen;
    ttUserGenericUnion  genUnion;

    genUnion.gen8BitParm = TM_8BIT_ZERO;;

/* print the routing table using netstat tools */
    
    buffer = (ttCharPtr)tm_get_raw_buffer(TM_NT_ENTRY_STR_LEN);
    
    if (buffer != (ttCharPtr)0)
    {
        tm_sprintf(buffer, "\n%s: -- Routing table --", msg);
        tm_debug_log0(buffer);

        tm_sprintf(
            buffer, "tvRteLimboCount = %d", tm_context(tvRteLimboCount) );
        tm_debug_log0(buffer);

        headerStrLen = TM_NT_ENTRY_STR_LEN;
        arpStrPtr = tfNtGetRteHeaderStr(buffer, &headerStrLen);
        if (    ( arpStrPtr != (ttCharPtr)0 )
/* 
 * The stack calls tfLogAllocBuf with TM_DEF_MAX_LOG_MSG_LEN, so we
 * can use this macro for comparison. Otherwise we would have to change it to
 *             && ( headerStrLen < 
 *                       (int)tm_context(tvLogCtrlBlkPtr)->lcbBufEndPadLen
 *                     - (int)(sizeof(ttLogMsgHdr) - TM_LOG_MSG_HDR_PAD_LEN) ) )
 */
             && ( headerStrLen < TM_DEF_MAX_LOG_MSG_LEN) )
        {
/* If ARP netstat message does not exceed maximum log message size */
            tm_debug_log0(buffer);
            (void) tfNetStat(
                TM_NT_TABLE_RTE, tfxNtRteEntryCBLog, genUnion, genUnion);
        }
        tm_free_raw_buffer((ttRawBufferPtr)(ttVoidPtr)buffer);
    }
}

/* log a routing entry */
void tfxLogRteEntry(const char * msg, ttRteEntryPtr rtePtr)
{
    ttCharPtr           buffer;
    ttNtEntryUPtr       ntEntryPtr;
    ttNtRteEntryPtr     ntRteEntryPtr;
#ifdef TM_LOCK_NEEDED
    ttSnmpdCacheHeadPtr snmpdCacheHeadPtr;
#endif /* TM_LOCK_NEEDED */
    int                 entryStrLen;
    int                 prefixLen; /* length of msg and space delimiter */

    if (rtePtr->rteDevPtr != TM_DEV_NULL_PTR)
    {
        if (rtePtr->rteSnmpCacheEntry.sceTableEntryPtr == (ttVoidPtr)0)
        {
            rtePtr->rteSnmpCacheEntry.sceTableEntryPtr = (ttVoidPtr)rtePtr;
        }
        ntEntryPtr = (ttNtEntryUPtr)tm_get_raw_buffer(
                                          sizeof(ttNtRteEntry));
        if (ntEntryPtr != (ttNtEntryUPtr)0)
        {
            ntRteEntryPtr = &(ntEntryPtr->ntRteEntry);
#ifdef TM_LOCK_NEEDED
            if (rtePtr->rteSnmpCacheEntry.sceTableIndex != TM_SNMPC_NO_INDEX)
            {
                snmpdCacheHeadPtr = &(tm_context(tvSnmpdCaches)[
                                    rtePtr->rteSnmpCacheEntry.sceTableIndex]);
                tm_call_lock_wait(&(snmpdCacheHeadPtr->snmpcLockEntry));
            }
            else
            {
                snmpdCacheHeadPtr = (ttSnmpdCacheHeadPtr)0;
            }
#endif /* TM_LOCK_NEEDED */
            if (tfNtGetRteEntry(&(rtePtr->rteSnmpCacheEntry), ntEntryPtr)
                                                             == TM_ENOERROR)
            {
                entryStrLen = TM_NT_ENTRY_STR_LEN;
                prefixLen = (int)tm_strlen(msg) + 1;

                buffer = (ttCharPtr)tm_get_raw_buffer(
                    TM_NT_ENTRY_STR_LEN + prefixLen);
                if (buffer != (ttCharPtr)0)
                {
                    tm_strcpy(buffer, msg);
                    tm_strcat(buffer, " ");
                    tfNtRteEntryToStr(
                        ntRteEntryPtr, buffer + prefixLen, &entryStrLen);
                    tm_debug_log0(buffer);

                    tm_free_raw_buffer((ttRawBufferPtr)(ttVoidPtr)buffer);
                }
            }
#ifdef TM_LOCK_NEEDED
            if (snmpdCacheHeadPtr != (ttSnmpdCacheHeadPtr)0)
            {
                tm_call_unlock(&(snmpdCacheHeadPtr->snmpcLockEntry));
            }
#endif /* TM_LOCK_NEEDED */

            tm_free_raw_buffer((ttRawBufferPtr)(ttVoidPtr)ntRteEntryPtr);
        }
    }
}
#endif /* TM_DEBUG_LOGGING */
#endif /* TM_ERROR_CHECKING */


#ifdef TM_USE_IPV4
/* 
 * Function tfNtGetNatHeaderStr()
 * DESCRIPTION:
 *  Get the NAT table header as a string.
 * PARAMETERS:
 *  buffer:     the user provided buffer for storing the result string
 *  *sizePtr:   size of the user provided buffer, must be no shorting than
 *              TM_NT_NAT_TABLE_STR_LEN
 *  sizePtr:    user provided integer pointer to carry back the actual
 *              string length, ignored if NULL
 * RETURNS:
 *  TM_ENOERROR:    successful
 *  TM_ENINVAL:     buffer is NULL or *sizePtr is too small.
 */
char TM_FAR * tfNtGetNatHeaderStr(
    char TM_FAR *   buffer, 
    int  TM_FAR *   sizePtr)
{
    char TM_FAR *   bufferBase;
    
    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_NAT_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);
        tm_nt_strncpy(buffer, "Private IP",     TM_NT_NAT_ADDR_PORT_STR_LEN);
        tm_nt_strncpy(buffer, "Public IP",      TM_NT_NAT_ADDR_PORT_STR_LEN);
        tm_nt_strncpy(buffer,
                      "Remt IP/PASV Ports",
                      TM_NT_NAT_ADDR_PORT_REMT_STR_LEN);
        tm_nt_strncpy(buffer, "TTL",            TM_NT_TTL_STR_LEN);
        tm_nt_strncpy(buffer, "Type",           TM_NT_NAT_TYPE_STR_LEN);
        tm_nt_strncpy(buffer, "Parent Type",    TM_NT_NAT_TYPE_STR_LEN);
        tm_nt_strncpy(buffer, "Flags",          TM_NT_NAT_FLAGS_STRL_LEN);

        buffer[0] = '\0';

        *sizePtr = TM_NT_NAT_TABLE_STR_LEN;
    }
    return bufferBase;
}


/*
 * tfNtGetNatEntry() function description
 * Extract information from a ttNatEntry, do necessary conversions and store 
 * it a ttNtDevEntry structure, which is more preseantation oriented, and it 
 * used to pass to the netstat call back functions.
 *
 * Parameters
 * Parameter    Description
 * devPtr       pointer to a socket entry to get information from, 
 * ntDevPtr     pointer to ttNtDevEntry to store the converted infomation
 *
 * Returns
 * Value            Meaning
 * TM_ENOERROR      Completed successfully
 *
 */
void tfNtGetNatEntry(ttNatTriggerPtr    trigPtr,
                     ttNtNatEntryPtr    ntNatPtr)
{
    ntNatPtr->ntNatIpPrivate = trigPtr->ntrIpPrivate;
    ntNatPtr->ntNatIpPublic = trigPtr->ntrIpPublic;
    ntNatPtr->ntNatIpRemote = trigPtr->ntrIpRemote;
    ntNatPtr->ntNatIpPrivatePort = trigPtr->ntrPortPrivate;
    ntNatPtr->ntNatIpPublicPort = trigPtr->ntrPortPublic;
    ntNatPtr->ntNatIpRemotePort = trigPtr->ntrPortRemote;
    ntNatPtr->ntNatTtl = trigPtr->ntrTtl;
    ntNatPtr->ntNatType = trigPtr->ntrType;
    if (trigPtr->ntrSpawnPtr != (ttNatTriggerPtr)0)
    {
        ntNatPtr->ntNatSpawnType = trigPtr->ntrSpawnPtr->ntrType;
    }
    ntNatPtr->ntNatFlags = trigPtr->ntrFlag;

    return;
}


/* 
 * Function tfNtNatEntryToStr()
 * DESCRIPTION: 
 *  print the entry as s string to the user provided buffer
 * PARAMETERS
 *  ntNatEntryPtr
          : pointer to the entry to be converted to string
 *  buffer:     the user provided buffer for storing the result string
 *  *sizePtr: size of the user provided buffer, must be no shorting than
 *              TM_NT_RTE_TABLE_STR_LEN
 *  sizePtr:  user provided integer pointer to carry back the actual
 *              string length, ignored if NULL
 * RETURNS:
 *  TM_ENOERROR:    successful
 *  TM_ENINVAL:     ntArpEntryPtr is NULL, 
 *                  buffer is NULL, 
 *                  or *sizePtr is too small.
 */
char TM_FAR * tfNtNatEntryToStr(
    ttNtNatEntryPtr ntNatEntryPtr, 
    char TM_FAR *   buffer, 
    int  TM_FAR *   sizePtr)
{
    char TM_FAR *   bufferBase;
    int             tempBufLen;


    if(  (buffer == (char TM_FAR *)0)
       ||(sizePtr == (int TM_FAR *)0)
       ||(*sizePtr <= TM_NT_NAT_TABLE_STR_LEN))
    {
/* invalid input parameter(s) */
        bufferBase = (char TM_FAR *)0;
    }
    else
    {
        bufferBase = buffer;
        tm_memset(buffer, ' ', *sizePtr);
        switch (ntNatEntryPtr->ntNatType)
        {
        case TM_NTRTYPE_STATIC:
            inet_ntop(AF_INET,
                      &(ntNatEntryPtr->ntNatIpPrivate),
                      buffer,
                      INET_ADDRSTRLEN);
            buffer[tm_strlen(buffer)] = ' ';
            buffer += TM_NT_NAT_ADDR_PORT_STR_LEN;
            inet_ntop(AF_INET,
                      &(ntNatEntryPtr->ntNatIpPublic),
                      buffer,
                      INET_ADDRSTRLEN);
            buffer[tm_strlen(buffer)] = ' ';
            buffer += TM_NT_NAT_ADDR_PORT_STR_LEN;
            buffer += TM_NT_NAT_ADDR_PORT_REMT_STR_LEN;
            break;
        case TM_NTRTYPE_DYNAMIC:
        case TM_NTRTYPE_DYNAMIC_OUTBOUND:
        case TM_NTRTYPE_TCPSESSION:
        case TM_NTRTYPE_UDPSESSION:
        case TM_NTRTYPE_ICMPIDSESSION:
            buffer[0] = '*';
            buffer += TM_NT_NAT_ADDR_PORT_STR_LEN;
            inet_ntop(AF_INET,
                      &(ntNatEntryPtr->ntNatIpPublic),
                      buffer,
                      INET_ADDRSTRLEN);
            buffer[tm_strlen(buffer)] = ' ';
            buffer += TM_NT_NAT_ADDR_PORT_STR_LEN;
            if (ntNatEntryPtr->ntNatIpRemote != 0)
            {
                inet_ntop(AF_INET,
                          &(ntNatEntryPtr->ntNatIpRemote),
                          buffer,
                          INET_ADDRSTRLEN);
                tempBufLen = (int)tm_strlen(buffer);
                buffer += tempBufLen;
                if (ntNatEntryPtr->ntNatIpRemotePort != 0)
                {
                    tempBufLen += tm_sprintf(buffer,
                                             ":%u",
                                             ntNatEntryPtr->ntNatIpRemotePort);
                    buffer += tm_strlen(buffer);
                    buffer[0] = ' ';
                }
                else
                {
                    buffer[tm_strlen(buffer)] = ' ';
                }
                buffer += TM_NT_NAT_ADDR_PORT_REMT_STR_LEN - tempBufLen;
            }
            else
            {
                buffer += TM_NT_NAT_ADDR_PORT_REMT_STR_LEN;
            }
            break;
        case TM_NTRTYPE_NAPT:
            buffer[0] = '*';
            buffer += TM_NT_NAT_ADDR_PORT_STR_LEN;
            inet_ntop(AF_INET,
                      &(ntNatEntryPtr->ntNatIpPublic),
                      buffer,
                      INET_ADDRSTRLEN);
            tempBufLen = (int)tm_strlen(buffer);
            buffer += tempBufLen;
            tempBufLen += tm_sprintf(buffer,
                                     ":%u",
                                     ntNatEntryPtr->ntNatIpPublicPortMin);
            buffer += tm_strlen(buffer);
            tempBufLen += tm_sprintf(buffer,
                                     "-%u",
                                     ntNatEntryPtr->ntNatIpPublicPortMax);
            buffer += tm_strlen(buffer);
            buffer[0] = ' ';
            buffer += (TM_NT_NAT_ADDR_PORT_STR_LEN - tempBufLen);
            buffer += TM_NT_NAT_ADDR_PORT_REMT_STR_LEN;
            break;
        case TM_NTRTYPE_INNERTCPSERVER:
        case TM_NTRTYPE_INNERUDPSERVER:
            inet_ntop(AF_INET,
                      &(ntNatEntryPtr->ntNatIpPrivate),
                      buffer,
                      INET_ADDRSTRLEN);
            tempBufLen = (int)tm_strlen(buffer);
            buffer += tempBufLen;
            tempBufLen += tm_sprintf(buffer,
                                     ":%u",
                                     ntohs(ntNatEntryPtr->ntNatIpPrivatePort));
            buffer += tm_strlen(buffer);
            buffer[0] = ' ';
            buffer += (TM_NT_NAT_ADDR_PORT_STR_LEN - tempBufLen);
            inet_ntop(AF_INET,
                      &(ntNatEntryPtr->ntNatIpPublic),
                      buffer,
                      INET_ADDRSTRLEN);
            tempBufLen = (int)tm_strlen(buffer);
            buffer += tempBufLen;
            tempBufLen += tm_sprintf(buffer,
                                     ":%u",
                                     ntohs(ntNatEntryPtr->ntNatIpPublicPort));
            buffer += tm_strlen(buffer);
            buffer[0] = ' ';
            buffer += (TM_NT_NAT_ADDR_PORT_STR_LEN - tempBufLen);
            buffer += TM_NT_NAT_ADDR_PORT_REMT_STR_LEN;
            break;
        case TM_NTRTYPE_INNERFTPSERVER:
            inet_ntop(AF_INET,
                      &(ntNatEntryPtr->ntNatIpPrivate),
                      buffer,
                      INET_ADDRSTRLEN);
            tempBufLen = (int)tm_strlen(buffer);
            buffer += tempBufLen;
            tempBufLen += tm_sprintf(buffer, ":21");
            buffer += tm_strlen(buffer);
            buffer[0] = ' ';
            buffer += (TM_NT_NAT_ADDR_PORT_STR_LEN - tempBufLen);
            inet_ntop(AF_INET,
                      &(ntNatEntryPtr->ntNatIpPublic),
                      buffer,
                      INET_ADDRSTRLEN);
            tempBufLen = (int)tm_strlen(buffer);
            buffer += tempBufLen;
            tempBufLen += tm_sprintf(buffer, ":21");
            buffer += tm_strlen(buffer);
            buffer[0] = ' ';
            buffer += (TM_NT_NAT_ADDR_PORT_STR_LEN - tempBufLen);
            tempBufLen = tm_sprintf(buffer,
                                    "%u",
                                    ntNatEntryPtr->ntNatIpPublicPortMin);
            buffer += tm_strlen(buffer);
            tempBufLen += tm_sprintf(buffer,
                                     "-%u",
                                     ntNatEntryPtr->ntNatIpPublicPortMax);
            buffer += tm_strlen(buffer);
            buffer[0] = ' ';
            buffer += (TM_NT_NAT_ADDR_PORT_REMT_STR_LEN - tempBufLen);
            break;
        default:
            break;
        }

        tfNtTtlToStr(ntNatEntryPtr->ntNatTtl, buffer);
        buffer += TM_NT_TTL_STR_LEN;

        buffer = tfNtNatTypeToStr(ntNatEntryPtr->ntNatType, buffer);
        if (ntNatEntryPtr->ntNatSpawnType != 0)
        {
            buffer = tfNtNatTypeToStr(ntNatEntryPtr->ntNatSpawnType, buffer);
        }
        else
        {
            buffer += TM_NT_NAT_TYPE_STR_LEN;
        }

        tfNtNatFlagToStr(ntNatEntryPtr->ntNatFlags, buffer);
        buffer += TM_NT_NAT_FLAGS_STRL_LEN;

        buffer[0] = '\0';

        *sizePtr = TM_NT_NAT_TABLE_STR_LEN;
    }

    return bufferBase;
}


/*
 * tfNtTypeToStr() function description
 * Interpret the NAT trigger type and represent it is a char buffer for outputting.
 *
 * Parameters
 * Parameter        Description
 * flags            in: the NAT trigger type
 * buffer           value result: string representation of the type
 */
static char * tfNtNatTypeToStr(ttUser8Bit type, char TM_FAR * buffer)
{
    switch (type)
    {
    case TM_NTRTYPE_STATIC:
        tm_nt_strncpy(buffer, "Static", TM_NT_NAT_TYPE_STR_LEN);
        break;
    case TM_NTRTYPE_DYNAMIC:
        tm_nt_strncpy(buffer, "Dynamic", TM_NT_NAT_TYPE_STR_LEN);
        break;
    case TM_NTRTYPE_DYNAMIC_OUTBOUND:
        tm_nt_strncpy(buffer, "Dynamic Outbound", TM_NT_NAT_TYPE_STR_LEN);
        break;
    case TM_NTRTYPE_TCPSESSION:
        tm_nt_strncpy(buffer, "TCP Session", TM_NT_NAT_TYPE_STR_LEN);
        break;
    case TM_NTRTYPE_UDPSESSION:
        tm_nt_strncpy(buffer, "UDP Session", TM_NT_NAT_TYPE_STR_LEN);
        break;
    case TM_NTRTYPE_ICMPIDSESSION:
        tm_nt_strncpy(buffer, "ICMP ID Session", TM_NT_NAT_TYPE_STR_LEN);
        break;
    case TM_NTRTYPE_NAPT:
        tm_nt_strncpy(buffer, "NAPT", TM_NT_NAT_TYPE_STR_LEN);
        break;
    case TM_NTRTYPE_INNERTCPSERVER:
        tm_nt_strncpy(buffer, "Inner TCP Server", TM_NT_NAT_TYPE_STR_LEN);
        break;
    case TM_NTRTYPE_INNERUDPSERVER:
        tm_nt_strncpy(buffer, "Inner UDP Server", TM_NT_NAT_TYPE_STR_LEN);
        break;
    case TM_NTRTYPE_INNERFTPSERVER:
        tm_nt_strncpy(buffer, "Inner FTP Server", TM_NT_NAT_TYPE_STR_LEN);
        break;
    default:
        tm_nt_strncpy(buffer, "Unknown", TM_NT_NAT_TYPE_STR_LEN);
        break;
    }
    return buffer;
}


/*
 * tfNtNatFlagToStr() function description
 * Interpret the NAT trigger flags and represent it is a char buffer for outputting.
 *
 * Parameters
 * Parameter        Description
 * flags            in: the NAT flags
 * buffer           value result: string representation of the flags, 
 *                  not to be allocated to be at least TM_NT_FLAGS_STR_LEN bytes.
 * Returns          the string representation
 */
char TM_FAR * tfNtNatFlagToStr(ttUser8Bit flags, char TM_FAR * buffer)
{
    char TM_FAR *   bufferBase;

    bufferBase = buffer;

    tm_memset(buffer, 0, TM_NT_NAT_FLAGS_STRL_LEN);

    if (flags & TM_NTRFLAG_ACTIVE)
    {
        *buffer++ = 'A';
        tm_8bit_clr_bit( flags, TM_NTRFLAG_ACTIVE);
    }
    else
    {
        *buffer++ = ' ';
    }
    if (flags & TM_NTRFLAG_TCPSEQ)
    {
        *buffer++ = 'T';
        tm_8bit_clr_bit( flags, TM_NTRFLAG_TCPSEQ);
    }
    else
    {
        *buffer++ = ' ';
    }
    if (flags & TM_NTRFLAG_TCPFIN)
    {
        *buffer++ = 'F';
        tm_8bit_clr_bit( flags, TM_NTRFLAG_TCPFIN);
    }
    else
    {
        *buffer++ = ' ';
    }

    return bufferBase;
}
#endif /* TM_USE_IPV4 */


#else /* !TM_USE_NETSTAT */
#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

int tlNstatoDummy = 0;
#endif /* !TM_USE_NETSTAT */
