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
 * Description: Generic Device Send and Incoming routines
 *
 * Filename: trdevice.c
 * Author: Paul
 * Date Created: 11/10/97
 * $Source: source/trdevice.c $
 *
 * Modification History
 * $Revision: 6.0.2.39 $
 * $Date: 2012/10/09 14:50:58JST $
 * $Author: lfox $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>
#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/* Define unreferenced macro compile option for PC-LINT */
#ifdef TM_LINT
LINT_UNREF_MACRO(TM_DATA_WAITING)
LINT_UNREF_MACRO(TM_DATA_UNAVAILABLE)
#endif /* TM_LINT */

/* #define TM_STRCMP_EQUAL   0 */ /* Defined in trsocket.h */
/* #define TM_STRCMP_LESS   -1 */ /* Defined in trsocket.h */
/* #define TM_STRCMP_GREATER 1 */ /* Defined in trsocket.h */
#define TM_DATA_WAITING     1
#define TM_DATA_UNAVAILABLE 0

/*
 * Local macros
 */

#ifdef TM_DSP
/*
 * If first time through, must deal with ethernet header
 * copy enet header + data
 */
#define tm_pack_data_length( dataLength,lnkPacketPtr,oldPacketPtr,devPtr ) \
{                                                                          \
    if ( (lnkPacketPtr == oldPacketPtr) &&                                 \
         (tm_ll_has_ether(devPtr)) )                                       \
    {                                                                      \
        dataLength -= TM_ETHER_HEADER_SIZE;                                \
        dataLength  =   tm_packed_byte_count(dataLength)                   \
                      + TM_ETHER_HEADER_SIZE;                              \
    }                                                                      \
    else                                                                   \
    {                                                                      \
        dataLength = tm_packed_byte_count(dataLength);                     \
    }                                                                      \
}
#else /* !TM_DSP */
/* Do nothing macro */
#define tm_pack_data_length( dataLength, lnkPacketPtr, oldPacketPtr, devPtr )
#endif /* TM_DSP */

/*
 * Local functions
 */
#ifdef TM_USE_IPV4
static int tfCheckSetNetmask(ttDeviceEntryPtr devPtr,
                             tt16Bit          mHomeIndex);
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_DRV_ONE_SCAT_SEND
static void tfSendCompletePacketsInterfaceUnlock(
                                    ttDeviceEntryPtr devPtr,
                                    int              sendCompltPacketCount );
#endif /* TM_USE_DRV_ONE_SCAT_SEND */

/*
 * Free a Turbo Treck device buffer, or user owned device driver buffer
 * Called from tfRecvInterface(), when an
 * error occurs, to release the device driver buffers.
 */
static void tfFreeDrvRecvBuffer( ttDeviceEntryPtr devPtr,
                                 ttPacketPtr      packetPtr,
                                 char    TM_FAR * dataPtr );


static int tfDeviceOpen(
    ttDeviceEntryPtr devPtr,
    int              buffersPerFrameCount);

#ifdef TM_USE_STOP_TRECK
/* Free all allocated memory associated with a device, including the device */
static void tfDeviceFree(ttDeviceEntryPtr devPtr);
#endif /* TM_USE_STOP_TRECK */

#ifdef TM_USE_IPV6
/*
 * Check if same network address is configured on another multihome, and check
 * whether same sol node multicast address is needed elsewhere.
 * Re-add local address if same found.
 * Leave sol node multicast if not found.
 */
static void tf6CheckOtherMhomeConfig(ttDeviceEntryPtr devPtr,
                                     tt16Bit          mHomeIndex);

/* Reset configuration flag and addresses for this multihome entry */
static void tf6ResetMhomeConfig (ttDeviceEntryPtr devPtr,
                                 tt16Bit          mHomeIndex);
#endif /* TM_USE_IPV6 */

/*
 * Add an Interface
 */
ttUserInterface tfAddInterface (
                const char TM_FAR     * namePtr,
                ttUserLinkLayer         linkLayerHandle,
                ttDevOpenCloseFuncPtr   drvOpenFuncPtr,
                ttDevOpenCloseFuncPtr   drvCloseFuncPtr,
                ttDevSendFuncPtr        drvSendFuncPtr,
                ttDevRecvFuncPtr        drvRecvFuncPtr,
                ttDevFreeRecvFuncPtr    drvFreeRecvFuncPtr,
                ttDevIoctlFuncPtr       drvIoctlFuncPtr,
                ttDevGetPhysAddrFuncPtr drvGetPhysAddrFuncPtr,
                int  TM_FAR           * drvAddErrorPtr)
{
/* check the parameters */
    const char TM_FAR * charPtr;
    ttDeviceEntryPtr    listdevPtr;
    ttDeviceEntryPtr    devPtr;
#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
    ttIpAddrEntryPtr    oldSortedIpAddrArray;
#ifdef TM_USE_IPV6
    unsigned int        ipv6AllocSize;
#endif /* TM_USE_IPV6 */
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */
    unsigned int        allocSize;
#if (defined(TM_USE_STRONG_ESL) && defined(TM_USE_IPDUAL))
    unsigned int        devSize;
#endif /* TM_USE_STRONG_ESL && TM_USE_IPDUAL */
    int                 len;
#ifndef TM_SINGLE_INTERFACE_HOME
    int                 isEqual;
#endif /* TM_SINGLE_INTERFACE_HOME */
    int                 errorCode;
#ifdef TM_USE_IPV6
#ifdef TM_USE_STRONG_ESL
    int                 i;
#endif /* TM_USE_STRONG_ESL */
#endif /* TM_USE_IPV6 */
    tt16Bit             linkLayerProtocol;
#if (defined(TM_USE_STRONG_ESL) && defined(TM_USE_IPDUAL))
    tt8Bit              isLoopback;
#endif /* TM_USE_STRONG_ESL && TM_USE_IPDUAL */

    devPtr = TM_DEV_NULL_PTR;
    if (    (namePtr == (const char TM_FAR *)0)
         || (drvAddErrorPtr == (int TM_FAR *)0) )
    {
        errorCode = TM_EINVAL;
        goto addInterfaceFinish;
    }
/*
 * Check that linkLayerHandle point to a valid link layer entry
 */
    errorCode = tfValidLinkLayer((ttLinkLayerEntryPtr)linkLayerHandle);
    if (errorCode != TM_ENOERROR)
    {
        goto addInterfaceFinish;
    }
    linkLayerProtocol =
            ((ttLinkLayerEntryPtr)linkLayerHandle)->lnkLinkLayerProtocol;
/*
 * Check that send, recv function pointers are non null. Also check
 * that GetPhysAddr function pointer is non null for an Ethernet
 * interface.
 */
    if (    (    ( drvGetPhysAddrFuncPtr == (ttDevGetPhysAddrFuncPtr)0 )
              && ( linkLayerProtocol == TM_LINK_LAYER_ETHERNET ) )
         || (    (drvSendFuncPtr == (ttDevSendFuncPtr)0)
#ifdef TM_USE_DRV_ONE_SCAT_SEND
              && ((linkLayerProtocol) & TM_LINK_PT2PT_MASK)
#endif /* TM_USE_DRV_ONE_SCAT_SEND */
            )
         || (    (drvRecvFuncPtr == (ttDevRecvFuncPtr)0 )
#ifdef TM_USE_DRV_SCAT_RECV
              && ((linkLayerProtocol) & TM_LINK_PT2PT_MASK)
#endif /* TM_USE_DRV_SCAT_RECV */
            )
       )
    {
        errorCode = TM_EINVAL;
        goto addInterfaceFinish;
    }
/*
 * Check that namePtr length is no bigger than TM_MAX_DEVICE_NAME, and is
 * zero terminated
 */
    len = 0;
    charPtr = namePtr;
    while ((*charPtr != (char)0) && (len < TM_MAX_DEVICE_NAME))
    {
        charPtr++;
        len++;
    }
    if ( (len == 0) || (len >= TM_MAX_DEVICE_NAME) )
    {
        errorCode = TM_EINVAL;
        goto addInterfaceFinish;
    }
/* Lock the device list */
    tm_call_lock_wait(&tm_context(tvDeviceListLock));
    listdevPtr = tm_context(tvDeviceList);
/* Check for duplicate */
#ifdef TM_SINGLE_INTERFACE_HOME
    if (listdevPtr != TM_DEV_NULL_PTR)
    {
        errorCode = TM_EALREADY;
        goto addInterfaceUnlock;
    }
#else /* TM_SINGLE_INTERFACE_HOME */
    while (listdevPtr != TM_DEV_NULL_PTR)
    {
        isEqual = tm_strcmp( (ttCharPtr)(listdevPtr->devNameArray),
                             namePtr );
        if( isEqual == TM_STRCMP_EQUAL )
        {
            errorCode = TM_EALREADY;
            goto addInterfaceUnlock;
        }
        listdevPtr = listdevPtr->devNextDeviceEntry;
    }

/* check to make sure we haven't already added the maximum number of
   interfaces. */
    if (tm_context(tvAddedInterfaces) >= TM_MAX_NUM_IFS)
    {
        errorCode = TM_ENOSPC;
        goto addInterfaceUnlock;
    }
#endif /* !TM_SINGLE_INTERFACE_HOME */
/* If interface does not exist then malloc an entry */
    allocSize = (unsigned int)sizeof(ttDeviceEntry);
#if (defined(TM_USE_STRONG_ESL) && defined(TM_USE_IPDUAL))
    devSize = allocSize;
    if (tm_strcmp(namePtr, "LOOPBACK") == 0)
    {
        isLoopback = TM_8BIT_YES;
    }
    else
    {
        allocSize = (unsigned int)(allocSize + sizeof(ttRteEntry));
        isLoopback = TM_8BIT_NO;
    }
#endif /* TM_USE_STRONG_ESL && TM_USE_IPDUAL */
    devPtr = (ttDeviceEntryPtr)tm_kernel_malloc(allocSize);
    if (devPtr == TM_DEV_NULL_PTR)
    {
/* if the malloc fails then return an error */
        errorCode = TM_ENOBUFS;
        goto addInterfaceUnlock;
    }
    tm_bzero((ttVoidPtr)devPtr, allocSize);
#ifdef TM_MULTIPLE_CONTEXT
    devPtr->devContextPtr = tfGetCurrentContext();
#endif /* TM_MULTIPLE_CONTEXT */
#ifdef TM_USE_IPV4
#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
/* allocate the space for the sorted array of directed broadcast addresses
   on the interface. */
    allocSize = sizeof(ttIpAddrEntry) * TM_MAX_IPS_PER_IF;

    devPtr->devSortedDBroadIpAddrCache.sipcIpAddrArray =
        (ttIpAddrEntryPtr) tm_kernel_malloc(allocSize);
    if (devPtr->devSortedDBroadIpAddrCache.sipcIpAddrArray
        != TM_IP_ADDR_ENTRY_NULL_PTR)
    {
        tm_bzero(
            (ttVoidPtr) devPtr->
            devSortedDBroadIpAddrCache.sipcIpAddrArray,
            allocSize);
    }
#ifdef TM_ERROR_CHECKING
    else
    {
        tfKernelWarning(
            "tfAddInterface",
            "Failed allocating "
            "IP address cache for directed broadcast IPv4 addresses\n");
    }
#endif /* TM_ERROR_CHECKING */

#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */
#endif /* TM_USE_IPV4 */

/* Fill in the device entry */
    tm_strcpy((ttCharPtr)(devPtr->devNameArray), namePtr);
/* Initialize link layer fields */
    devPtr->devLinkLayerProtocolPtr =
                                (ttLinkLayerEntryPtr)linkLayerHandle;
    devPtr->devLinkLayerProtocol = linkLayerProtocol;
    devPtr->devLinkHeaderLength =
                ((ttLinkLayerEntryPtr)linkLayerHandle)->lnkHeaderLength;
    devPtr->devOpenFuncPtr = drvOpenFuncPtr;
    devPtr->devCloseFuncPtr = drvCloseFuncPtr;
    devPtr->devSendFuncPtr = drvSendFuncPtr;
    devPtr->devRecvFuncPtr = drvRecvFuncPtr;
    devPtr->devFreeRecvFuncPtr = drvFreeRecvFuncPtr;
    devPtr->devIoctlFuncPtr = drvIoctlFuncPtr;
    devPtr->devGetPhysAddrFuncPtr = drvGetPhysAddrFuncPtr;

#ifdef TM_USE_IPV4
    devPtr->devMtu = ((ttLinkLayerEntryPtr)linkLayerHandle)->lnkMtu;
/* Initialize DHCP/BOOTP options */
    devPtr->devBtInitTimeoutIntv = TM_BOOT_INIT_TIMEOUT;
    devPtr->devBtMaxRetries = TM_BOOT_RETRIES;
#ifdef TM_USE_STRONG_ESL
    devPtr->devBootSocketDescriptor = TM_SOCKET_ERROR;
    devPtr->devBootRelayAgentDescriptor = TM_SOCKET_ERROR;
#endif /* TM_USE_STRONG_ESL */

#endif /* TM_USE_IPV4 */
    if ( tm_ll_need_32bit_align(devPtr))
    {
        devPtr->devTxAlign = TM_ETHER_HW_TX_ALIGN;
    }
/*
 * Enforce this amount of data to be contiguous at the beginning of a
 * device driver scattered recv buffer.
 */
    devPtr->devRecvContiguousHdrLength = TM_DEV_DEF_RECV_CONT_HDR_LENGTH;

#ifdef TM_USE_IPV6
    tfListInit( &(devPtr->dev6DnsServerList) );
    tfListInit( &(devPtr->dev6DnsSearchList) );

#ifdef TM_6_USE_DHCP
    devPtr->dev6MhomeUsedDhcpEntries =
              TM_MAX_IPS_PER_IF
            + TM_6_MAX_AUTOCONF_IPS_PER_IF;
#endif /* TM_6_USE_DHCP*/

#ifdef TM_6_USE_MIP_HA
    devPtr->dev6MhomeUsedProxyEntries =
              TM_MAX_IPS_PER_IF
            + TM_6_MAX_AUTOCONF_IPS_PER_IF
            + TM_6_MAX_DHCP_IPS_PER_IA;
#endif /* TM_6_USE_MIP_HA */

    devPtr->dev6MhomeUsedAutoEntries =
              TM_MAX_IPS_PER_IF;

    devPtr->dev6HopLimit = TM_6_IP_DEF_HOPS;

#ifdef TM_6_PMTU_DISC
    devPtr->dev6Mtu = ((ttLinkLayerEntryPtr)linkLayerHandle)->lnkMtu;
#else /* ! TM_6_PMTU_DISC */
/* When IPv6 Path MTU Discovery is disabled, we must not exceed the IPv6
   minimum link MTU of 1280 bytes. ([RFC2460].R5:60) */
    devPtr->dev6Mtu = TM_6_PMTU_MINIMUM;
#endif /* ! TM_6_PMTU_DISC */

/*
 * Initialize the base reachable time to the default, and randomize the
 * reachable time. ([RFC2461].R6.3.4:70, [RFC2461].R6.3.4:80)
 */
    devPtr->dev6BaseReachableTime = TM_6_ND_DEF_REACHABLE_TIME;
    devPtr->dev6ReachableTime =
        (devPtr->dev6BaseReachableTime >> 1) +
        (tfGetRandom() % devPtr->dev6BaseReachableTime);

    devPtr->dev6NeighSolicitReTxPeriod = TM_6_ND_DEF_RETRANS_TIMER;
    tm_kernel_set_critical;
    devPtr->dev6LastReachRndTime       = tvTime;
    tm_kernel_release_critical;

/* Router discovery parameters */
    devPtr->dev6RtrSolInitDelay = TM_6_SOL_DEF_INIT_DELAY;
    devPtr->dev6RtrSolTimeout   = TM_6_SOL_DEF_TIMEOUT;
    devPtr->dev6RtrSolInitRetry = TM_6_SOL_DEF_RETRY;

/* The default value for DupAddrDetectTransmits is 1. ([RFC2462].R5.1:50) */
    devPtr->dev6DupAddrDetectTransmits = TM_6_DAD_DEF_XMITS;
#ifdef TM_USE_STRONG_ESL
/* Empty Default Router list */
    for (i = 0; i < TM_6_NUM_DEF_ROUTER_LISTS; i++)
    {
        tfListInit(&(devPtr->dev6DefaultRouterHead.drh6RouterList[i]));
    }
#endif /* TM_USE_STRONG_ESL */

#ifdef TM_6_USE_MLDV2
/* Set the Default Values */
    devPtr->dev6MldRobustVar = TM_6_MLD_ROBUST_VAR;
    devPtr->dev6MldQueryIntr = TM_6_MLD_QUERY_INTR;
    devPtr->dev6MldOVQPTimeout = 0;
    devPtr->dev6MldLastRcvTime = 0;
#endif /* TM_6_USE_MLDV2 */

#ifdef TM_6_USE_RFC3484
/* 2. If there is no error, then */
    isEqual = tm_strcmp(namePtr,"LOOPBACK");
    if ((errorCode == TM_ENOERROR)
        && (isEqual != TM_STRCMP_EQUAL) )
    {
/*   a. Initialize the policy table by calling the function
        tfPolicyTableInit(). */
        errorCode = tfPolicyTableInit(devPtr);
    }
#endif /* TM_6_USE_RFC3484 */
#endif /* TM_USE_IPV6 */

/* Initialize dialer fields (disabled) */
    devPtr->devDialerEntryPtr = TM_DIALER_ENTRY_NULL_PTR;
#ifdef TM_USE_DHCP_COLLECT
/* Initialize DHCP offer cache parameters */
    devPtr->devDhcpCollectSize = TM_DHCP_DEF_COLLECT_CACHE;
    devPtr->devDhcpCollectTime = TM_DHCP_DEF_COLLECT_TIMEOUT;
#endif /* TM_USE_DHCP_COLLECT */

/* Put it into the device list */
    devPtr->devNextDeviceEntry = tm_context(tvDeviceList);
    tm_context(tvDeviceList) = devPtr;
/* Increase number of added interfaces */
    tm_context(tvAddedInterfaces)++;
/* Interface number (used for SNMP) */
    devPtr->devIndex = (tt16Bit)(tm_context(tvAddedInterfaces) & 0xFFFF);

    tm_context(tvDevIndexToDevPtr)[devPtr->devIndex] = devPtr;

#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
/* sort IP addresses to improve performance of IP address lookup in the
   receive-path when many IP aliases are configured on a single interface.
   The SNMP Agent can use this to speed up non-exact match processing
   in tfIpAddrTableEntryGet. */
    allocSize =   sizeof(ttIpAddrEntry)
                * TM_MAX_IPS_PER_IF
                * tm_context(tvAddedInterfaces);
#ifdef TM_USE_IPV6
    ipv6AllocSize =   sizeof(ttIpAddrEntry)
                    * TM_6_MAX_MHOME
                    * tm_context(tvAddedInterfaces);
#endif /* TM_USE_IPV6 */

    if (tm_context(tvAddedInterfaces) == 1)
    {
/* need to do initialization of cache used to sort IP addresses
   configured on all interfaces */
#ifdef TM_USE_IPV4
        tm_call_lock_wait(&(tm_context(tv4SortedIpAddrCache).sipcLockEntry));
        tm_context(tv4SortedIpAddrCache).sipcIpAddrArray =
            (ttIpAddrEntryPtr) tm_malloc(allocSize);
        if (tm_context(tv4SortedIpAddrCache).sipcIpAddrArray
            != TM_IP_ADDR_ENTRY_NULL_PTR)
        {
            tm_bzero(
                (ttVoidPtr) tm_context(tv4SortedIpAddrCache).sipcIpAddrArray,
                allocSize);
        }
#ifdef TM_ERROR_CHECKING
        else
        {
            tfKernelWarning(
                "tfAddInterface",
                "Failed allocating "
                "IP address cache for unicast IPv4 addresses\n");
        }
#endif /* TM_ERROR_CHECKING */

        tm_call_unlock(&(tm_context(tv4SortedIpAddrCache).sipcLockEntry));
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
        tm_call_lock_wait(&(tm_context(tv6SortedIpAddrCache).sipcLockEntry));
        tm_context(tv6SortedIpAddrCache).sipcIpAddrArray =
            (ttIpAddrEntryPtr) tm_malloc(ipv6AllocSize);
        if (tm_context(tv6SortedIpAddrCache).sipcIpAddrArray
            != TM_IP_ADDR_ENTRY_NULL_PTR)
        {
            tm_bzero(
                (ttVoidPtr) tm_context(tv6SortedIpAddrCache).sipcIpAddrArray,
                ipv6AllocSize);
        }
#ifdef TM_ERROR_CHECKING
        else
        {
            tfKernelWarning(
                "tfAddInterface",
                "Failed allocating "
                "IP address cache for unicast IPv6 addresses\n");
        }
#endif /* TM_ERROR_CHECKING */

        tm_call_unlock(&(tm_context(tv6SortedIpAddrCache).sipcLockEntry));
#endif /* TM_USE_IPV6 */
    }
    else
    {
/* need to do realloc of global array used to sort IP addresses configured
   on all interfaces */
#ifdef TM_USE_IPV4
        tm_call_lock_wait(&(tm_context(tv4SortedIpAddrCache).sipcLockEntry));
        oldSortedIpAddrArray =
            tm_context(tv4SortedIpAddrCache).sipcIpAddrArray;
        if (oldSortedIpAddrArray != TM_IP_ADDR_ENTRY_NULL_PTR)
        {
            tm_context(tv4SortedIpAddrCache).sipcIpAddrArray =
                (ttIpAddrEntryPtr) tm_malloc(allocSize);
            if (tm_context(tv4SortedIpAddrCache).sipcIpAddrArray
                != TM_IP_ADDR_ENTRY_NULL_PTR)
            {
                tm_bzero((ttVoidPtr)
                         tm_context(tv4SortedIpAddrCache).sipcIpAddrArray,
                         allocSize);
                tm_bcopy(
                    oldSortedIpAddrArray,
                    tm_context(tv4SortedIpAddrCache).sipcIpAddrArray,
                    (sizeof(ttIpAddrEntry)
                      * tm_context(tv4SortedIpAddrCache).
                        sipcIpAddrArrayCount));
            }
#ifdef TM_ERROR_CHECKING
            else
            {
                tfKernelWarning(
                    "tfAddInterface",
                    "Failed reallocating "
                    "IP address cache for unicast IPv4 addresses\n");
            }
#endif /* TM_ERROR_CHECKING */

            tm_free(oldSortedIpAddrArray);
        }
        tm_call_unlock(&(tm_context(tv4SortedIpAddrCache).sipcLockEntry));
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
        tm_call_lock_wait(&(tm_context(tv6SortedIpAddrCache).sipcLockEntry));
        oldSortedIpAddrArray =
            tm_context(tv6SortedIpAddrCache).sipcIpAddrArray;
        if (oldSortedIpAddrArray != TM_IP_ADDR_ENTRY_NULL_PTR)
        {
            tm_context(tv6SortedIpAddrCache).sipcIpAddrArray =
                (ttIpAddrEntryPtr) tm_malloc(ipv6AllocSize);
            if (tm_context(tv6SortedIpAddrCache).sipcIpAddrArray
                != TM_IP_ADDR_ENTRY_NULL_PTR)
            {
                tm_bzero((ttVoidPtr)
                         tm_context(tv6SortedIpAddrCache).sipcIpAddrArray,
                         ipv6AllocSize);
                tm_bcopy(
                    oldSortedIpAddrArray,
                    tm_context(tv6SortedIpAddrCache).sipcIpAddrArray,
                    (sizeof(ttIpAddrEntry)
                     * tm_context(tv6SortedIpAddrCache).
                       sipcIpAddrArrayCount));
            }
#ifdef TM_ERROR_CHECKING
            else
            {
                tfKernelWarning(
                    "tfAddInterface",
                    "Failed reallocating "
                    "IP address cache for unicast IPv6 addresses\n");
            }
#endif /* TM_ERROR_CHECKING */

            tm_free(oldSortedIpAddrArray);
        }
        tm_call_unlock(&(tm_context(tv6SortedIpAddrCache).sipcLockEntry));
#endif /* TM_USE_IPV6 */
    }
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */
#if (defined(TM_USE_STRONG_ESL) && defined(TM_USE_IPDUAL))
    if (isLoopback == TM_8BIT_NO)
    {
/*
 * Initialize empty default gateway for the device.
 * Insert empty default gateway in the tree.
 */
       devPtr->dev4RejectDefaultGatewayPtr = (ttRteEntryPtr)
                                       (((ttCharPtr)devPtr) + devSize);
       (void)tf4RtInitInsertEmptyDefaultGateway(devPtr);
    }
#endif /* TM_USE_STRONG_ESL && TM_USE_IPDUAL */

#ifdef TM_6_USE_DHCP
#ifdef TM_6_DHCP_USE_FQDN
/* Save the default value for the FQDN flags field */
    devPtr->dev6DhcpFqdnFlags = TM_6_DHCP_FQDN_FLAGS_DEFAULT;
#endif /* TM_6_DHCP_USE_FQDN */
#endif /* TM_6_USE_DHCP */
/*
 * Create the receive event. Only created if TM_TASK_RECV is defined and if
 * either TM_TRECK_PREMPTIVE_KERNEL, or TM_TRECK_NON_PREEMPTIVE_KERNEL is
 * defined.
 */
    tm_kernel_create_recv_event((&(devPtr->devRecvEvent.eventUnion)));
/*
 * Create the sent event. Only created if TM_TASK_SENT is defined and if
 * either TM_TRECK_PREMPTIVE_KERNEL, or TM_TRECK_NON_PREEMPTIVE_KERNEL is
 * defined.
 */
    tm_kernel_create_sent_event((&(devPtr->devSentEvent.eventUnion)));
/*
 * Create the Transmit event. Only created if TM_TASK_XMIT is defined and if
 * either TM_TRECK_PREMPTIVE_KERNEL, or TM_TRECK_NON_PREEMPTIVE_KERNEL is
 * defined.
 */
    tm_kernel_create_xmit_event((&(devPtr->devXmitEvent.eventUnion)));

#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
    tm_kernel_set_critical;
    tm_context(tvIpData).ipIfStatsTableLastChange =
        tm_snmp_time_ticks(tvTime);
    tm_kernel_release_critical;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */

addInterfaceUnlock:
/* unlock the device list */
    tm_call_unlock(&tm_context(tvDeviceListLock));
addInterfaceFinish:
/* return the device pointer */
    if (drvAddErrorPtr != (int TM_FAR *)0)
    {
        *drvAddErrorPtr = errorCode;
    }
    return (ttUserInterface)devPtr;
}

/****************************************************************************
* FUNCTION: tfNgConfigInterface
*
* PURPOSE: This function is a replacement for tfConfigInterface to support
*   IPv6.
*
* NEW PARAMETERS:
*   prefixLen:
*       The length (in bits) of the subnet prefix part of the address
*       specified by ipAddrPtr.
*   ipv6Flags:
*       IPv6-specific interface/device flags:
*       TM_6_DEV_IP_FORW_ENB
*       TM_6_DEV_IP_FORW_MCAST_ENB
*       TM_6_DEV_MCAST_HW_LOOPBACK
*       TM_6_DEV_OPTIMIZE_DAD
*   dev6AddrNotifyFuncPtr:
*       The function to call to notify the user of IPv6-specific address
*       configuration events, or TM_6_DEV_ADDR_NOTIFY_FUNC_NULL_PTR if
*       notification is not desired.
*
* RETURNS:
*   TM_EINVAL:
*       invalid value specified for ipAddrPtr
*   TM_EINVAL:
*       multiHomeIndex was greater than or equal to TM_MAX_IPS_PER_IF.
*   TM_EINVAL:
*       Invalid value specified for prefixLen.
*   TM_EPERM:
*       User attempted to open the interface for IPv6, however there was no
*       way to configure a link-local scope IPv6 address on the interface.
*   TM_ENOSPC:
*       Failed to configure an IPv4-compatible IPv6 because a different IPv4
*       address was already configured in the IPv4 address list at the
*       specified multi-home index.
*   TM_EAFNOSUPPORT:
*       addrFamily was set to an invalid value for address family
*   TM_EINPROGRESS:
*       Duplicate Address Detection is in progress
*
* NOTES:
*   When the Treck stack is run in dual IP layer mode, and the specified
*   address is an IPv4-compatible IPv6 address, then it is configured on the
*   interface at the same multi-home index in both the IPv4 and IPv6 address
*   lists to support automatic tunneling.
*
****************************************************************************/
int tfNgConfigInterface(
    ttUserInterface            interfaceHandle,
    ttConstSockAddrStoragePtr  ipAddrPtr,
    int                        prefixLen,
    int                        flags,
    int                        ipv6Flags,
    int                        buffersPerFrameCount,
    tt6DevAddrNotifyFuncPtr    dev6AddrNotifyFuncPtr,
    unsigned int               mHomeIndex )
{
    int                         errorCode;

#if (!defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
/* avoid compiler warnings */
    TM_UNREF_IN_ARG(ipv6Flags);
    TM_UNREF_IN_ARG(dev6AddrNotifyFuncPtr);
#endif /* IPv4-only */

    errorCode = TM_ENOERROR; /* assume successful */
/* parameter validation */
    if (  (ipAddrPtr == (ttConstSockAddrStoragePtr)0)
        ||(mHomeIndex >=  TM_MAX_IPS_PER_IF))
    {
        errorCode = TM_EINVAL;
        goto exitNgConfigInterface;
    }

    switch (ipAddrPtr->ss_family)
    {
#ifdef TM_USE_IPV4
    case AF_INET:
        if ((ipAddrPtr->addr.ipv4.sin_addr.s_addr != TM_IP_ZERO)
            && (prefixLen > 32))
        {
            errorCode = TM_EINVAL;
        }
        else
        {
            errorCode = tf4ConfigInterface (
                interfaceHandle,
                ipAddrPtr->addr.ipv4.sin_addr.s_addr,
                tvRt32ContiguousBitsPtr[prefixLen],
                flags,
                buffersPerFrameCount,
                (tt16Bit) mHomeIndex );
        }
        break;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
    case AF_INET6:
        errorCode = tf6ConfigInterface(
                        interfaceHandle,
                        ipAddrPtr,
                        prefixLen,
                        flags,
                        ipv6Flags,
                        buffersPerFrameCount,
                        dev6AddrNotifyFuncPtr,
                        mHomeIndex );
    break;
#endif /* TM_USE_IPV6 */

    default:
        errorCode = TM_EAFNOSUPPORT;
        break;
    }

exitNgConfigInterface:
    return errorCode;
}

#ifdef TM_USE_IPV6
int tf6ConfigInterface(
    ttUserInterface            interfaceHandle,
    ttConstSockAddrStoragePtr  ipAddrPtr,
    int                        prefixLen,
    int                        flags,
    int                        ipv6Flags,
    int                        buffersPerFrameCount,
    tt6DevAddrNotifyFuncPtr    dev6AddrNotifyFuncPtr,
    unsigned int               mHomeIndex )
{
    ttDeviceEntryPtr            devPtr;
    ttDialerEntryPtr            dialEntryPtr;
#ifdef TM_6_USE_MIP_MN
#ifndef TM_6_MN_DISABLE_HOME_DETECT
    ttUserInterface             mobileIfaceHandle;
    struct sockaddr_storage     currRtrSockAddr;
#endif /* ! TM_6_MN_DISABLE_HOME_DETECT */
#endif /* TM_6_USE_MIP_MN */
    struct sockaddr_storage     localIpAddr;
#ifdef TM_6_USE_DAD
    tt6IpAddress                tempAddr;
#endif /* TM_6_USE_DAD */
    int                         errorCode;

#ifdef TM_6_USE_MLD
    flags |= TM_DEV_MCAST_ENB; /* IPv6 MLD requires multicast */
#endif /* TM_6_USE_MLD */
    devPtr = (ttDeviceEntryPtr)interfaceHandle;

    tm_memcpy(&localIpAddr, ipAddrPtr, sizeof(struct sockaddr_storage));

/* validate the address we are trying to configure */
    if (!(IN6_IS_ADDR_UNSPECIFIED(&(ipAddrPtr->addr.ipv6.sin6_addr))))
    {
        errorCode = tf6ValidConfigAddr(
            &(ipAddrPtr->addr.ipv6.sin6_addr), prefixLen);
        if (errorCode != TM_ENOERROR)
        {
            goto exit6ConfigInterface;
        }
    }

#ifdef TM_6_USE_MIP_MN
    if (devPtr == tm_context(tv6MnVect).mns6VirtHomeIfacePtr)
    {
        if (tm_8bit_bits_not_set(tm_context(tv6MnVect).mns6Flags,
                                    TM_6_MNS_MOBILE_NODE_ACTIVE))
        {
/* the user must start Mobile IPv6 mobile node first */
            errorCode = TM_EPERM;
            goto exit6ConfigInterface;
        }
#ifndef TM_6_MN_DISABLE_HOME_DETECT
        else if (tm_context(tv6MnVect).mns6Flags & TM_6_MNS_AT_HOME)
        {
/* We allow the user to manually open the virtual home interface. We treat
this as an indication that the mobile node is operating on a foreign
link. */

/* lock the mobile node state vector */
            tm_call_lock_wait(&(tm_context(tv6MnVect).mns6LockEntry));
            mobileIfaceHandle = (ttUserInterface)
                tm_context(tv6MnVect).mns6MobileIfacePtr;
/* set status that we are operating on a foreign link */
            errorCode = tf6MnMoveFromHome();
/* unlock the mobile node state vector */
            tm_call_unlock(&(tm_context(tv6MnVect).mns6LockEntry));

/* If the user registered a MIPv6 Mobile Node event callback function when
they called tf6MnStartMobileIp, then notify the user of the event
TM_6_MNE_MOVE_FROM_HOME */
            if (tm_context(tv6MnVect).mns6UserNotifyFuncPtr
                != TM_6_MN_NOTIFY_FUNC_NULL_PTR)
            {
/* Initialize currRtrSockAddr to link-local scope address of current default
router */
                tf6SetSockAddrFromIpAddr(
                    &tm_context(tv6MnVect).mns6LinkLocalDefRtrAddr,
                    (ttDeviceEntryPtr) mobileIfaceHandle,
                    &currRtrSockAddr.addr.ipv6);

/* give the user the notification */
                (*(tm_context(tv6MnVect).mns6UserNotifyFuncPtr))(
                    mobileIfaceHandle,
                    &currRtrSockAddr,
                    TM_6_MNE_MOVE_FROM_HOME);
            }

            if (errorCode != TM_ENOERROR)
            {
                goto exit6ConfigInterface;
            }
        }
#endif /* ! TM_6_MN_DISABLE_HOME_DETECT */
    }
#endif /* TM_6_USE_MIP_MN */

/* Check interfaceHandle parameter */
    errorCode = tfValidInterface(devPtr);
    if (errorCode != TM_ENOERROR)
    {
        goto exit6ConfigInterface;
    }
/* Lock device entry AFTER verifying that the interface is valid */
    tm_call_lock_wait(&(devPtr->devLockEntry));

/* If the device is still closing, or the user is trying to restart DHCPv6
 * while its's still running, don't allow it to be opened again yet */
    if (   (devPtr->dev6Flags & TM_6_DEV_CLOSING)
#ifdef TM_6_USE_DHCP
        || (   (  ipv6Flags
                & (TM_6_DEV_USER_DHCP | TM_6_DEV_USER_DHCP_RTR_CTRLD))
            && (  devPtr->dev6Flags
                & (TM_6_DEV_USER_DHCP | TM_6_DEV_USER_DHCP_RTR_CTRLD)))
#endif /* TM_6_USE_DHCP */
       )
    {
        errorCode = TM_EPERM;
        goto exit6UnlockConfigInterface;
    }

#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
    if (IN6_IS_ADDR_V4COMPAT(
            &(localIpAddr.addr.ipv6.sin6_addr))
        && !(devPtr->dev6IpAddrFlagsArray[mHomeIndex]
                & (TM_6_DEV_IP_CONFIG
                | TM_6_DEV_IP_CONF_STARTED)))
    {
        if (prefixLen > TM_6_IP_V4COMPAT_PREFIX_LENGTH)
        {
/* if we are being asked to configure an IPv4-compatible IPv6 address, and
this interface/mHomeIndex index isn't configured yet for IPv6 */
            if (tm_8bit_bits_not_set(
                        tm_ip_dev_conf_flag(devPtr, mHomeIndex),
                        (TM_DEV_IP_CONFIG | TM_DEV_IP_CONF_STARTED)))
            {
/* tf4ConfigInterface assumes devLockEntry is unlocked */
                tm_call_unlock(&(devPtr->devLockEntry));
/* if it isn't configured for IPv4 either, then configure the embedded
IPv4 address first. */
                errorCode = tf4ConfigInterface (
                    interfaceHandle,
                    localIpAddr.addr.ipv6.sin6_addr.
                    ip6Addr.ip6U32[3],
                    tvRt32ContiguousBitsPtr[
                        prefixLen
                        - TM_6_IP_V4COMPAT_PREFIX_LENGTH],
                    flags,
                    buffersPerFrameCount,
                    (tt16Bit) mHomeIndex );
/* Re-lock devLockEntry */
                tm_call_lock_wait(&(devPtr->devLockEntry));
            }
            else if ((localIpAddr.addr.ipv6.sin6_addr.
                        ip6Addr.ip6U32[3] !=
                        tm_ip_dev_addr(devPtr, mHomeIndex)))
            {
/* if there is already a different IPv4 address configured in the IPv4
address list at the specified multi-home index, then return TM_ENOSPC. */
                errorCode = TM_ENOSPC;
            }
        }
        else
        {
/* When configuring an IPv4-compatible IPv6 address, the prefix length must
be greater than 96. */
            errorCode = TM_EINVAL;
        }
    }
#endif /* dual IP layer */

    if (errorCode != TM_ENOERROR)
    {
        goto exit6UnlockConfigInterface;
    }

    devPtr->dev6Flags |= (tt16Bit) (ipv6Flags & ~TM_6_DEV_IP_TEMPORARY);

    if (devPtr->dev6IpAddrFlagsArray[mHomeIndex]
        & TM_6_DEV_IP_CONF_STARTED)
    {
/* If this interface/mHomeIndex index is in the process of being configured,
then return TM_EINPROGRESS. */
        errorCode = TM_EINPROGRESS;
    }
/* If the link layer is in the process of connecting, can't configure another
address yet. */
    else if (tm_16bit_one_bit_set(devPtr->dev6Flags,
                                  TM_6_LL_CONNECTING))
    {
        errorCode = TM_EALREADY;
    }
    else if (!(devPtr->dev6IpAddrFlagsArray[mHomeIndex]
                & TM_6_DEV_IP_CONFIG))
/* If this interface/mHomeIndex index already configured, then return with no
error. */
    {
        if (tm_16bit_one_bit_set(ipv6Flags, TM_6_DEV_IP_TEMPORARY))
        {
/* This address is temporary */
            devPtr->dev6IpAddrFlagsArray[mHomeIndex] |=
                                                 TM_6_DEV_IPADDR_TEMPORARY;
        }
        devPtr->dev6AddrNotifyFuncPtr = dev6AddrNotifyFuncPtr;

/* open the device, if not already opened */
        if ( !(devPtr->devFlag & TM_DEV_OPENED) )
        {
            if (     (mHomeIndex == TM_16BIT_ZERO)
                 || !(tm_ll_is_pt2pt(devPtr) ) )
            {
                errorCode = tfDeviceOpen(
                    devPtr, buffersPerFrameCount);
            }
            else
            {
/*
 * Enforce usage of multi home index zero for first configuration, for pt2pt
 * interface
 */
                errorCode = TM_EINVAL;
            }
        }

        if (errorCode != TM_ENOERROR)
        {
            goto exit6UnlockConfigInterface;
        }

#ifdef TM_6_USE_DAD
/*
 * Check that we are not trying to configure the link-local stateless
 * address autoconfiguration address. If so, DAD will fail and cause
 * an assert(). Instead, we act as if the user passed us "::".
 */
    if (!(IN6_IS_ADDR_UNSPECIFIED(&(ipAddrPtr->addr.ipv6.sin6_addr))))
    {
#ifdef TM_LITTLE_ENDIAN
        tempAddr.s6LAddr[0] = TM_UL(0x000080FE);
#else /* TM_BIG_ENDIAN */
        tempAddr.s6LAddr[0] = TM_UL(0xFE800000);
#endif /* TM_BIG_ENDIAN */
        tempAddr.s6LAddr[1] = TM_UL(0);

/* We need the interface ID to build the address */
        if (tm_16bit_bits_not_set(devPtr->dev6Flags,
                                  TM_6_DEV_INTERFACE_ID_FLAG))
        {
/* If using Ethernet, we can generate the interface ID */
            if (tm_6_ll_is_lan(devPtr))
            {
                errorCode = tf6IntEui48SetInterfaceId(devPtr,
                                                      devPtr->devPhysAddrArr);
                if (errorCode != TM_ENOERROR)
                {
                    goto exit6UnlockConfigInterface;
                }
            }
        }

/* Either we already had an interface ID, or we were able to generate one */
        if (tm_16bit_one_bit_set(devPtr->dev6Flags,
                                 TM_6_DEV_INTERFACE_ID_FLAG))
        {
            tempAddr.s6LAddr[2] = devPtr->dev6InterfaceId[0];
            tempAddr.s6LAddr[3] = devPtr->dev6InterfaceId[1];
            if (tm_6_ip_match(&(localIpAddr.addr.ipv6.sin6_addr),
                              &tempAddr))
            {
                tm_bzero(&(localIpAddr.addr.ipv6.sin6_addr),
                         sizeof(struct in6_addr));
            }
        }
    }

/*
 * We don't want to configure this new prefix address if it is already
 * configured (via DHCPv6, manual IPv6 configuration, etc.)
 */
    if (!(tm_6_ip_zero(&(localIpAddr.addr.ipv6.sin6_addr))))
    {
        tm_6_ip_copy_structs(localIpAddr.addr.ipv6.sin6_addr, tempAddr);
        errorCode = tf6FindAddrInDevList(devPtr, &tempAddr);
        if (errorCode != TM_ENOERROR)
        {
            goto exit6UnlockConfigInterface;
        }
    }
#endif /* TM_6_USE_DAD */

/*
 * If the address we are being asked to configure is the IPv6 unspecified
 * address, this is not an error: we will open the link layer if it isn't
 * already open and start the process of stateless address
 * auto-configuration.
 */
        tm_6_ip_copy_structs(
            localIpAddr.addr.ipv6.sin6_addr,
            devPtr->dev6IpAddrArray[mHomeIndex]);
        devPtr->dev6PrefixLenArray[mHomeIndex] = (tt8Bit) prefixLen;

/* OR with user flags. */
        devPtr->devFlag |= (tt16Bit)flags;

/* If the link layer isn't open yet: */
        if (tm_16bit_bits_not_set(
                devPtr->dev6Flags, TM_6_LL_CONNECTED))
        {
/* 6. if we are using the dialer (and it hasn't already been executed),
*    then open the dialer
*/
            dialEntryPtr = devPtr->devDialerEntryPtr;
            if ((dialEntryPtr != TM_DIALER_ENTRY_NULL_PTR) &&
                (dialEntryPtr->dialOpenFuncPtr !=
                    (ttLnkOpenFuncPtr) 0) )
            {
                errorCode = (*(dialEntryPtr->dialOpenFuncPtr))
                    ( ((void TM_FAR *) interfaceHandle),
                        (tt16Bit) mHomeIndex, PF_INET6);
            }
            else
            {
/* 7. if we aren't using the dialer, then if the link-layer isn't already
connected then open the link-layer.
*/
                errorCode = (*(devPtr->
                                devLinkLayerProtocolPtr->
                                lnkOpenFuncPtr))
                    (((void TM_FAR *) interfaceHandle),
                        (tt16Bit) mHomeIndex, PF_INET6);

                if (errorCode == TM_ENOERROR)
                {
                    errorCode =
                            tf6FinishOpenInterface(devPtr, mHomeIndex);
                }
            }
        }
        else
        {
/*
 * The device is already open, the link layer is connected and this isn't
 * the first multihome to be configured on this interface.
 */
            errorCode = tf6FinishOpenInterface(devPtr, mHomeIndex);
        }
    }
#ifdef TM_6_USE_DAD
    else if (IN6_IS_ADDR_UNSPECIFIED(
                    &(localIpAddr.addr.ipv6.sin6_addr)))
    {
/* allow user to recover from a Duplicate Address Detection failure on the
auto-configured link-local scope address */
        if (!(devPtr->dev6IpAddrFlagsArray[mHomeIndex]
                & TM_6_DEV_IP_CONFIG))
        {
/*
 * mhomeIndex could have been configured earlier (i.e. mhome 0 when
 * configuring DHCPv6
 */
            errorCode = tf6ConfigInterfaceId(devPtr);
        }
    }
#endif /* TM_6_USE_DAD */

#ifdef TM_6_USE_DHCP
    if (   (errorCode == TM_ENOERROR)
        || (errorCode == TM_EINPROGRESS))
    {
/* Start DHCPv6 if TM_6_DEV_USER_DHCP is set in the ipv6flags*/
        if (ipv6Flags & TM_6_DEV_USER_DHCP)
        {
            if (devPtr->dev6BootFuncPtr == (ttDev6BootFuncPtr)0)
            {
                errorCode = TM_EPERM;
            }
/*
 * If DAD is still in progress, do not start DHCPv6 if the user has
 * requested that we delay until after the link-local address has finished DAD
 */
            else if (errorCode != TM_EINPROGRESS)
            {
                (*(devPtr->dev6BootFuncPtr))
                            (devPtr, ipv6Flags, TM_8BIT_YES);
            }
        }
/* Otherwise, if it's in router-controlled mode, solicit for routers */
        else if (ipv6Flags & TM_6_DEV_USER_DHCP_RTR_CTRLD)
        {
            tf6InitRtrSol(devPtr);
        }
    }
#endif/* TM_6_USE_DHCP */

exit6UnlockConfigInterface:
    tm_call_unlock(&(devPtr->devLockEntry));
exit6ConfigInterface:
    return errorCode;
}


#ifdef TM_6_USE_DAD
/*
 * tf6FindAddrInDevList
 *
 * Search through the addresses for a match in the configured and conf_started
 * states on the specified device.
 */
int tf6FindAddrInDevList(ttDeviceEntryPtr   devPtr,
                         tt6IpAddressPtr    tempAddrPtr)
{
    int     errorCode;
    int     i;

    errorCode = TM_ENOERROR;
    for (i = 0;
         i < (  TM_MAX_IPS_PER_IF
              + TM_6_MAX_AUTOCONF_IPS_PER_IF
              + TM_6_MAX_DHCP_SERVER_PER_IF);
         i++)
    {
        if (   (tm_8bit_one_bit_set(
                    devPtr->dev6IpAddrFlagsArray[i],
                    (TM_6_DEV_IP_CONFIG | TM_6_DEV_IP_CONF_STARTED)))
            && (tm_6_ip_match(tempAddrPtr,
                              &(devPtr->dev6IpAddrArray[i]))) )
        {
            errorCode = TM_EALREADY;
            break;
        }
    }

    return errorCode;
}
#endif /* TM_6_USE_DAD */
#endif /* TM_USE_IPV6 */

/*
 * Configure an interface with an IP address. Can be used for additional
 * IP Addresses on an Interface (Multihoming).
 * For first IP address configuration (first multi-home), use
 * tfOpenInterface instead.
 */
#ifdef TM_USE_IPV4
int tfConfigInterface ( ttUserInterface interfaceHandle,
                        ttUserIpAddress ipAddress,
                        ttUserIpAddress netMask,
                        int             flags,
                        int             buffersPerFrameCount,
                        unsigned char   mHomeIndex )
{
    return tf4ConfigInterface(
        interfaceHandle, ipAddress, netMask, flags, buffersPerFrameCount,
        (tt16Bit) mHomeIndex);
}

/* Configure an interface with an IP address, supports more than 256 IP
   aliases configured on an interface. Can be used for additional IP Addresses
   on an Interface (Multihoming). For first IP address configuration (first
   multi-home), use tfOpenInterface instead. */
int tf4ConfigInterface (
    ttUserInterface interfaceHandle,
    ttUserIpAddress ipAddress,
    ttUserIpAddress netMask,
    int             flags,
    int             buffersPerFrameCount,
    tt16Bit         mHomeIndex )
{
    ttDeviceEntryPtr    devPtr;
    int                 errorCode;
    tt16Bit             devFlag;
#ifdef TM_MOBILE_IP4
/* Pointer to Mobile IPv4 state vector */
    ttMobileVectPtr     mipVectPtr;
#endif /* TM_MOBILE_IP4 */
#ifdef TM_DEMO_FIXED_IP
    ttUserIpAddress     preXorIp;
    ttUserIpAddress     proXorIp;
    ttUserIpAddress     xorKey;
#endif /* TM_DEMO_FIXED_IP */
    tt8Bit              bootIndex;

    devPtr = (ttDeviceEntryPtr)interfaceHandle;
/*
 * Parameters check: check that mhome is within range allowed, and that
 * interface is valid
 */
    errorCode = tfValidInterface( devPtr );
    if( errorCode != TM_ENOERROR )
    {
        goto configInterfaceFinish;
    }
    errorCode = TM_EINVAL; /* assume failure */

    if( (int)mHomeIndex >= TM_MAX_IPS_PER_IF )
    {
        goto configInterfaceFinish;
    }
#ifdef TM_DEMO_FIXED_IP
    if(ipAddress != tm_const_htonl(TM_UL(0x7F000001)))
    {
        if( tm_ll_is_pt2pt(devPtr) )
        {
/* PPP IP address */
            preXorIp = TM_UL(0x66619297);
        }
        else
        {
/* Ethernet IP address */
            preXorIp = TM_UL(0x66613E6D);
        }
        xorKey = TM_UL(0xA6C95A67);
        proXorIp = preXorIp ^ xorKey;
        ipAddress = tm_const_htonl(proXorIp);
        preXorIp = TM_UL(0x5936A567);
        proXorIp = preXorIp ^ xorKey;
        netMask = tm_const_htonl(proXorIp);
    }
#endif /* TM_DEMO_FIXED_IP */
#ifdef TM_USE_DRV_ONE_SCAT_SEND
/* Double check that one send function is non null */
    if (     (devPtr->devSendFuncPtr == (ttDevSendFuncPtr)0)
         &&  (    devPtr->devOneScatSendFuncPtr
               == (ttDevOneScatSendFuncPtr)0) )
    {
        errorCode = TM_EPERM;
        goto configInterfaceFinish;
    }
#endif /* TM_USE_DRV_ONE_SCAT_SEND */
#ifdef TM_USE_DRV_SCAT_RECV
/* Double check that one recv function is non null */
    if (     (devPtr->devRecvFuncPtr == (ttDevRecvFuncPtr)0)
         &&  (    devPtr->devScatRecvFuncPtr
               == (ttDevScatRecvFuncPtr)0) )
    {
        errorCode = TM_EPERM;
        goto configInterfaceFinish;
    }
#endif /* TM_USE_DRV_SCAT_RECV */
    errorCode = TM_EADDRNOTAVAIL; /* assume failure */

/*
 * Parameter check: check that Ip address is not limited broadcast.
 */
    if (tm_ip_match(ipAddress, TM_IP_LIMITED_BROADCAST))
    {
/* Reject configuration of limited broadcast address */
        goto configInterfaceFinish;
    }
/* Reject class D and E addresses */
    if (tm_ip_is_class_d_e(ipAddress))
    {
        goto configInterfaceFinish;
    }
/*
 * Parameter check: check that flag is within range allowed
 */
    errorCode = TM_EINVAL; /* assume failure */

/* PRQA: QAC Message 277: The variable is used as a bit pattern */
/* PRQA S 277 L1 */
    if (tm_16bit_one_bit_set(flags, ~TM_DEV_USER_ALLOWED))
/* PRQA L:L1 */
    {
        goto configInterfaceFinish;
    }

/*
 * Parameter check: If scatter send is enabled at the device level,
 * then
 * 1. buffers per frame count should be bigger than 1.
 * 2. If user has turned on the interface xmit queue, then
 *    buffersPerFrameCount should be less or equal to the maximum xmit
 *    queue size.
 */
    if ( flags & TM_DEV_SCATTER_SEND_ENB )
    {
        if (buffersPerFrameCount < 1)
        {
            goto configInterfaceFinish;
        }
        if (tm_iface_xmit_queue_option(interfaceHandle))
        {
            if (   buffersPerFrameCount
                 > (int)devPtr->devXmitEntryPtr->xmitSlots )
            {
                goto configInterfaceFinish;
            }
        }
    }
    else
    {
/*
 * If scatter send is disabled at the device level, then buffers per frame
 * count should be 1.
 */
        if (buffersPerFrameCount != 1)
        {
            goto configInterfaceFinish;
        }
    }
/* lock the entry */
    tm_call_lock_wait(&(devPtr->devLockEntry));

#ifdef TM_USE_IPV6
/* If the device is still closing, don't allow it to be opened again yet */
    if (devPtr->dev6Flags & TM_6_DEV_CLOSING)
    {
        errorCode = TM_EPERM;
        goto configInterfaceUnlock;
    }
#endif /* TM_USE_IPV6 */

#ifdef TM_MOBILE_IP4
/*
 * If Mobile IPv4 is enabled and the user is configuring the first
 * multihome entry, check if the address is zero.  If it is,
 * configure the device with the home address of this node.
 */
    if ( tm_mobile_ip4_enb(devPtr) )
    {
        mipVectPtr = devPtr->devMobileIp4VectPtr;

/* Save the user's settings in the Mobile IP vector */
        tm_ip_copy( ipAddress, mipVectPtr->mipOrigDevAddr );
        tm_ip_copy( netMask, mipVectPtr->mipOrigDevNetmask );

        if ( (mHomeIndex == TM_16BIT_ZERO) &&
             tm_ip_match(ipAddress, TM_IP_ZERO) &&
             tm_ip_not_match(mipVectPtr->mipHomeAddr, TM_IP_ZERO) &&
             ( (flags & (TM_DEV_BOOT_FLAGS)) == 0) )
        {
            tm_ip_copy(mipVectPtr->mipHomeAddr, ipAddress);
            tm_ip_copy(TM_IP_HOST_MASK, netMask);
        }
    }
#endif /* TM_MOBILE_IP4 */

/* Enforce 0 IP address, and non pt2pt link layer for BOOTP or DHCP */
    if ( (flags & (TM_DEV_IP_BOOTP|TM_DEV_IP_DHCP)) != 0 )
    {
        if (    (tm_ip_not_zero(ipAddress))
             || (tm_ll_is_pt2pt(devPtr)) )
        {
            goto configInterfaceUnlock;
        }
        if ( (flags & (TM_DEV_IP_BOOTP|TM_DEV_IP_DHCP)) ==
                                     (TM_DEV_IP_BOOTP|TM_DEV_IP_DHCP) )
        {
/* User has to pick either BOOTP, or DHCP */
            goto configInterfaceUnlock;
        }
    }
    else
    {
        if (tm_ll_is_pt2pt(devPtr))
        {
            if (tm_ip_not_zero(ipAddress))
            {
                if ( tm_ip_match(ipAddress, devPtr->devPt2PtPeerIpAddr) )
                {
/* Cannot allow same IP address for our interface and for peer */
                    goto configInterfaceUnlock;
                }
            }
        }
        else
        {
/*
 * Disallow 0 IP address for non PPP/SLIP configurations, unless user
 * wants to get BOOTP like configuration from network.
 */
            if (    tm_ip_zero(ipAddress)
                 && ( (flags & TM_DEV_IP_USER_BOOT) == 0 ) )
            {
                goto configInterfaceUnlock;
            }
        }
    }
/*
 * If this interface/mHomeIndex index already configured, return with no error
 */
    if ( tm_8bit_one_bit_set( tm_ip_dev_conf_flag(devPtr, mHomeIndex),
                              TM_DEV_IP_CONFIG ) )
    {
        errorCode = TM_ENOERROR;
        goto configInterfaceUnlock;
    }
    errorCode = devPtr->devErrorCode;
    if (errorCode != TM_ENOERROR)
    {
        devPtr->devErrorCode = TM_ENOERROR;
        goto configInterfaceUnlock;
    }
    devFlag = devPtr->devFlag;
    if ( !(devFlag & TM_DEV_OPENED) )
/* If device not already opened */
    {
        if (     (mHomeIndex == TM_16BIT_ZERO)
             || !(tm_ll_is_pt2pt(devPtr) ) )
        {
            errorCode = tfDeviceOpen(devPtr, buffersPerFrameCount);
        }
        else
        {
/*
 * Enforce usage of multi home index zero for first configuration, for pt2pt
 * interface
 */
            errorCode = TM_EINVAL;
        }
    }
    else
    {
/* Device opened. Check whether connection is in progress */
        if ( tm_8bit_one_bit_set(
                        tm_ip_dev_conf_flag(devPtr, mHomeIndex),
                        (TM_DEV_IP_CONF_STARTED) ) )
        {
/* If this interface/mHomeIndex index is in the process of being configured,
 * then return TM_EINPROGRESS.
 */
            errorCode = TM_EINPROGRESS;
        }
        else
        {
/*
 * If the link layer is in the process of connecting, can't configure another
 * address yet.
 * If multi home is already configured, can't configure over it.
 */
            if (    (devFlag & TM_DEV_CONNECTING)
                 || ( tm_8bit_one_bit_set(
                            tm_ip_dev_conf_flag(devPtr, mHomeIndex),
                            (TM_DEV_IP_CONFIG) ) ) )
            {
                errorCode = TM_EALREADY;
            }
        }
    }
    if (errorCode != TM_ENOERROR)
    {
        goto configInterfaceUnlock;
    }
    tm_ip_copy((tt4IpAddress)ipAddress,
               tm_ip_dev_addr(devPtr, mHomeIndex));
    tm_ip_copy((tt4IpAddress)netMask,
               tm_ip_dev_mask(devPtr, mHomeIndex));
    tm_4_ip_dev_prefixLen(devPtr,mHomeIndex) =
        (tt8Bit) tf4NetmaskToPrefixLen(netMask);
    errorCode = tfCheckSetNetmask(devPtr, mHomeIndex);
    if (errorCode != TM_ENOERROR)
    {
        goto configInterfaceUnlock;
    }
/*
 * OR with user flags.
 * . Do not store TM_DEV_IP_USER_BOOT, TM_DEV_IP_BOOTP, TM_DEV_IP_DHCP on dev
 *   flag.
 */
    devPtr->devFlag |= (tt16Bit)(flags & ~(TM_DEV_BOOT_FLAGS));
/*
 * Store TM_DEV_IP_USER_BOOT/TM_DEV_IP_DHCP/TM_DEV_IP_BOOTP on conf flag
 * and indicate that configuration has started by setting
 * TM_DEV_IP_CONF_STARTED
 */
    tm_ip_dev_conf_flag(devPtr, mHomeIndex) |= (tt8Bit)
      ((flags & (TM_DEV_IP_USER_CONF_FLAGS)) | TM_DEV_IP_CONF_STARTED);
/* Start the Link Layer for this device. Device entry is locked */
    if (!(devFlag & TM_DEV_CONNECTED))
    {
/*
 * Not connected.
 */
        devPtr->devFlag |= TM_DEV_CONNECTING;

#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
        tm_kernel_set_critical;
        tm_context(tvIpData).ipv4InterfaceTableLastChange =
            tm_snmp_time_ticks(tvTime);
        tm_kernel_release_critical;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */

/*
 * If the dialer is enabled, rather than calling the link layer open function,
 * call the dialer.  When the dialer has completed, it will call the LL open
 * routine.
 */
        if (    (devPtr->devDialerEntryPtr != TM_DIALER_ENTRY_NULL_PTR)
             && (devPtr->devDialerEntryPtr->dialOpenFuncPtr !=
                                                      (ttLnkOpenFuncPtr) 0) )
        {
            errorCode = (*(devPtr->devDialerEntryPtr->
                           dialOpenFuncPtr))(
                               ((void TM_FAR *) interfaceHandle),
                               mHomeIndex,
                               PF_INET);
        }

        else
        {
/*
 * For PPP this call will attempt to make the connection, and when
 * the connection is made, IP address/netmask will be inserted in the
 * routing table.
 */

            errorCode = (*(devPtr->devLinkLayerProtocolPtr->
                          lnkOpenFuncPtr))( ((void TM_FAR *)interfaceHandle),
                                            mHomeIndex,
                                            PF_INET );
        }
        if (    (errorCode != TM_ENOERROR)
             && (errorCode != TM_EINPROGRESS)
             && (errorCode != TM_EALREADY) )
        {
/* Reset connecting flag */
            tm_16bit_clr_bit( devPtr->devFlag, TM_DEV_CONNECTING );
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
            tm_kernel_set_critical;
            tm_context(tvIpData).ipv4InterfaceTableLastChange =
                tm_snmp_time_ticks(tvTime);
            tm_kernel_release_critical;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
        }
    }
    if (devPtr->devFlag & TM_DEV_CONNECTED)
    {
/* Already connected */
#ifdef TM_SNMP_MIB
        tm_kernel_set_critical;
        devPtr->devLastChange = tvTime/10;
#ifdef TM_USE_NG_MIB2
        tm_context(tvIpData).ipv4InterfaceTableLastChange =
            tm_snmp_time_ticks(tvTime);
#endif /* TM_USE_NG_MIB2 */
        tm_kernel_release_critical;
#endif /* TM_SNMP_MIB */
        if (flags & (TM_DEV_IP_DHCP|TM_DEV_IP_BOOTP))
        {
/*
 * DHCP/BOOTP configuration: call BOOTP/DHCP configuration
 * if TM_DEV_IP_DHCP or TM_DEV_IP_BOOTP flag
 * is set it will attempt to send BOOTP or DHCP requests. When BOOTP/DHCP
 * replies are received, received IP address/netmask will be inserted in
 * the routing table.
 * Cannot do BOOTP and DHCP at the same time on the same multihome index,
 * otherwise one configured IP address could overwrite another one on the
 * device!!
 */
            bootIndex = TM_BOOT_BOOTP; /* assume BOOTP */
            if (flags & TM_DEV_IP_DHCP)
            {
                bootIndex = TM_BOOT_DHCP;
            }
            if (devPtr->devBootInfo[bootIndex].bootFuncPtr
                                                     != (ttDevBootFuncPtr)0)
            {
/*
 * Store TM_DEV_IP_DHCP/TM_DEV_IP_BOOTP on dev flag so that
 * tfIpIncomingPacket() lets incoming BOOTP/DHCP packets through.
 */
                devPtr->devFlag |= (tt16Bit)
                                (flags & (TM_DEV_IP_DHCP | TM_DEV_IP_BOOTP));
                errorCode =
                        (*(devPtr->devBootInfo[bootIndex].bootFuncPtr))
                            ( (ttUserInterface)devPtr,
                              TM_LL_OPEN_STARTED,
                              mHomeIndex );
            }
            else
            {
                errorCode = TM_EPERM;
                tm_ip_dev_conf_flag(devPtr, mHomeIndex) = TM_8BIT_ZERO;
            }
        }
        else
        {
/*
 * Already connected, and not BOOTP/DHCP.
 */
            if (!(flags & TM_DEV_IP_USER_BOOT))
/*
 * If user did not set TM_DEV_IP_USER_BOOT, just insert in routing table.
 */
            {
                errorCode = tfDeviceStart(devPtr, mHomeIndex,
                                          PF_INET);
            }
        }
     }
configInterfaceUnlock:
/* unlock the entry */
    tm_call_unlock(&(devPtr->devLockEntry));
configInterfaceFinish:
    return(errorCode);
}

/* convert an IPv4 netmask into a prefix length */
int tf4NetmaskToPrefixLen(ttUserIpAddress netMask)
{
#ifndef TM_LITTLE_ENDIAN
    auto tt4IpAddress   bittest; /* temporary */
#endif /* TM_BIG_ENDIAN */
    tt16Bit             prefixLen;

#ifdef TM_LITTLE_ENDIAN
    tm_ip_len_mask(netMask, prefixLen);
#else /* TM_BIG_ENDIAN */
    tm_ip_len_mask(netMask, prefixLen, bittest);
#endif /* TM_BIG_ENDIAN */

    return (int) prefixLen;
}
#endif /* TM_USE_IPV4 */

/* open the device */
static int tfDeviceOpen(
    ttDeviceEntryPtr devPtr,
    int              buffersPerFrameCount)
{
#if defined(TM_DEV_SEND_OFFLOAD) || defined(TM_DEV_RECV_OFFLOAD)
    ttDevIoctlOffloadStruct devIoctlOffload;
    int                     retCode;
#endif /* defined(TM_DEV_SEND_OFFLOAD) || defined(TM_DEV_RECV_OFFLOAD) */
    int                     errorCode;

    tm_assert_is_locked(tfDeviceOpen, &(devPtr->devLockEntry));
    errorCode = TM_ENOERROR; /* assume success */

    if (!(devPtr->devFlag & TM_DEV_OPENED))
    {
/* Initialize the extra scattered buffers count */
        devPtr->devScatterExtraCount = buffersPerFrameCount - 1;
/*
 * Lock the device driver lock before calling device open/GetPhysAddr.
 * Also protect devFlag (checked in tfIoctlInterface(), tfRecvInterface()).
 */
        tm_call_lock_wait(&(devPtr->devDriverLockEntry));
        if (devPtr->devOpenFuncPtr != (ttDevOpenCloseFuncPtr)0)
        {
/*
 * Open the Device itself, if not already opened.
 * Open the device will initialize driver and install ISR
 */
            errorCode = (*(devPtr->devOpenFuncPtr))(
                (ttUserInterface) devPtr);
        }
        if (errorCode == TM_ENOERROR)
        {
            if (( devPtr->devGetPhysAddrFuncPtr !=
                  (ttDevGetPhysAddrFuncPtr)0 )
                && (devPtr->
                    devLinkLayerProtocolPtr->lnkPhysAddrLen != 0))
            {
                devPtr->devPhysAddrLength = devPtr->
                    devLinkLayerProtocolPtr->lnkPhysAddrLen;
                (void)(*(devPtr->devGetPhysAddrFuncPtr))(
                    (ttUserInterface) devPtr,
                    (char TM_FAR *)(devPtr->devPhysAddrArr));

#ifdef TM_USE_IPV6
/* notify IPv6 that a link-layer address has been configured on the interface,
   therefore the interface supports IPv6 address resolution.
   ([RFC2461].R7.2.2:30, [RFC2461].R7.2.2:40, [RFC2461].R7.2.4:30,
   [RFC2461].R7.2.6:30, [RFC2461].R6.3.7:40) */
                devPtr->dev6Flags |= TM_6_DEV_LL_ADDRESS_FLAG;
#endif /* TM_USE_IPV6 */

/*
* Seed the random number generator with the last 4 bytes of the ethernet
* address (word #2, and word #3).
*/
                tfRandSeed(  (tt16Bit)
                             (   ((devPtr->devPhysAddrArr[2] & 0xff) << 8)
                                 | (devPtr->devPhysAddrArr[3] & 0xff) )
                             , (tt16Bit)
                             (   ((devPtr->devPhysAddrArr[4] & 0xff) << 8)
                                 | (devPtr->devPhysAddrArr[5] & 0xff) ) );
            }
            devPtr->devFlag |= TM_DEV_OPENED;

#ifdef TM_6_USE_DHCP
            if (    (tm_context(tv6DhcpDuidPtr) != (tt6DhcpDuidPtr)0)
                  &&(   tm_context(tv6DhcpDuidPtr)->duidType
                     == TM_6_DHCP_DUID_TYPE_NONE))
            {
/* User has called tf6UseDhcp earlier and has allocated the space for UDID-LL
 * let's store the DUID type and ID value now
 */
                tm_context(tv6DhcpDuidPtr)->duidType =
                         htons(TM_6_DHCP_DUID_TYPE_LL);
                tm_bcopy(devPtr->devPhysAddrArr,
                         &tm_context(tv6DhcpDuidPtr)->duIdValue.duidLl.llAddr,
                         devPtr->devPhysAddrLength);
           }
#endif/* TM_6_USE_DHCP */
        }
/* UnLock the device driver lock */
        tm_call_unlock(&(devPtr->devDriverLockEntry));
#if defined(TM_DEV_SEND_OFFLOAD) || defined(TM_DEV_RECV_OFFLOAD)
/* UnLock the device lock */
        tm_call_unlock(&(devPtr->devLockEntry));
        retCode = tfIoctlInterface( devPtr,
                                    TM_DEV_IOCTL_OFFLOAD_GET,
                                    &devIoctlOffload,
                                    sizeof(devIoctlOffload));
        if (retCode == TM_ENOERROR)
        {
            retCode = tfIoctlInterface( devPtr,
                                        TM_DEV_IOCTL_OFFLOAD_SET,
                                        &devIoctlOffload,
                                        sizeof(devIoctlOffload));
        }
/* Relock the device lock */
        tm_call_lock_wait(&(devPtr->devLockEntry));
#endif /* defined(TM_DEV_SEND_OFFLOAD) || defined(TM_DEV_RECV_OFFLOAD) */
    }

    return errorCode;
}


/****************************************************************************
* FUNCTION: tfDeviceStart
*
* PURPOSE:
* This function is called when an interface is opened to complete
* configuration of an IP address on the interface, either directly from
* tfConfigInterface, from a link layer or from tf6FinishOpenInterface.  For
* instance, tfDeviceStart is called from PPP when IPCP or IPV6CP finishes and
* from DHCP when an IP address is acquired. Note that a link-local IPv6
* address must be assigned to the interface, and Duplicate Address Detection
* must have completed successfully on the address we are configuring before
* this function can be called for an IPv6 address. This function assumes that
* the device is locked.
*
* PARAMETERS:
*   devPtr:
*       Device on which to start this protocol.
*   mHomeIndex:
*       Multi-home index of IP address to add to this device. Note that when
*       protocolFamily is PF_INET6 (IPv6), this is the index into
*       ttDeviceEntry.dev6IpAddrArray, which stores both manually configured
*       and automatically configured IPv6 addresses.
*   protocolFamily:
*       Protocol family associated with the address that was just configured
*       on the device/interface (i.e. PF_INET, PF_INET6).
*
* RETURNS:
*   TM_EPERM:
*       The IPv6 interface could not be started because a link-local IPv6
*       address has not been assigned to it. This error code should never
*       occur.
*
* NOTES:
*
****************************************************************************/
int tfDeviceStart ( ttDeviceEntryPtr devPtr,
                    tt16Bit          mHomeIndex,
                    int              protocolFamily)
{
    int                        errorCode;
#ifdef TM_USE_IPV4
    int                        errCode;
#endif /* TM_USE_IPV4 */
    int                        is_pt2pt;
    tt8Bit                     configDestination;
#ifdef TM_USE_IPV6
    tt6IpAddressPtr            destIpAddrPtr;
    tt6IpAddressPtr            localIpAddrPtr;
    tt6IpAddress               scopeIpAddr;
#ifdef TM_6_USE_PREFIX_DISCOVERY
    tt6IpAddress               destIpNetAddr;
    int                        tempMaskIndex;
#endif /* TM_6_USE_PREFIX_DISCOVERY */
    int                        prefixLen;
    struct sockaddr_storage    destAddr;
    struct sockaddr_storage    gwAddr;
#ifdef TM_6_USE_PREFIX_DISCOVERY
    tt8Bit                     isOnLink;
#endif /* TM_6_USE_PREFIX_DISCOVERY */
    tt16Bit                    usedMhome;
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
    tt4IpAddress               destIpAddress;
    tt4IpAddress               ipAddress;
#endif /* TM_USE_IPV4 */
#ifdef TM_MOBILE_IP4
    ttMobileVectPtr            mipVectPtr;
#endif /* TM_MOBILE_IP4 */
#ifdef TM_DEMO_FIXED_IP
    ttUserIpAddress            preXorIp;
    ttUserIpAddress            proXorIp;
    ttUserIpAddress            xorKey;
#endif /* TM_DEMO_FIXED_IP */
#ifdef TM_6_USE_MIP_MN
#ifndef TM_6_MN_DISABLE_HOME_DETECT
    tt6MnStatus                 temp6MnStatus;
#endif /* TM_6_MN_DISABLE_HOME_DETECT */
#endif /* TM_6_USE_MIP_MN */

    tm_assert_is_locked( tfDeviceStart, &(devPtr->devLockEntry) );

#ifdef TM_USE_IPV4
    if (protocolFamily == PF_INET)
    {
/* Reset mhome conf flag TM_DEV_IP_CONF_STARTED */
        tm_8bit_clr_bit( tm_ip_dev_conf_flag(devPtr, mHomeIndex),
                         TM_DEV_IP_CONF_STARTED );
        errorCode = TM_ENOERROR;
        {
/* Copy parameter stored in devPtr */
            tm_ip_copy(tm_ip_dev_addr(devPtr, mHomeIndex), ipAddress);
            is_pt2pt = tm_ll_is_pt2pt(devPtr);
#ifdef TM_DEMO_FIXED_IP
            if( is_pt2pt )
            {
/* PPP IP address */
                preXorIp = TM_UL(0x66619297);
                xorKey = TM_UL(0xA6C95A67);
                proXorIp = preXorIp ^ xorKey;
                ipAddress = tm_const_htonl(proXorIp);
                tm_ip_copy(ipAddress, tm_ip_dev_addr(devPtr, mHomeIndex));
            }
#endif /* TM_DEMO_FIXED_IP */
            configDestination = TM_8BIT_YES;
            if ( is_pt2pt )
            {
                tm_ip_copy(tfDeviceDestIpAddress(devPtr), destIpAddress);
                if (mHomeIndex != TM_16BIT_ZERO)
                {
                    configDestination = TM_8BIT_ZERO;
                }
            }
            else
            {
                tm_ip_copy(ipAddress, destIpAddress);
            }
/*
 * Check on ipAddress and netMask parameters
 */
            errCode = tfCheckSetNetmask(devPtr, mHomeIndex);
            if (   (tm_ip_zero(ipAddress))
                || (errCode != TM_ENOERROR))
            {
                errorCode = TM_EADDRNOTAVAIL;
            }
            else
            {
                tfRandSeed( (tt16Bit) ((ipAddress >> 16) & 0x0ffff),
                            (tt16Bit) (ipAddress & 0x0ffff) );
                if (configDestination != TM_8BIT_ZERO)
                {
/* deviceEntry is locked */
                    errorCode = tf4RtAddLocal(devPtr,
                                              destIpAddress, /* key */
                                              tm_ip_dev_mask(devPtr,
                                                             mHomeIndex),
                                              ipAddress,
                                              mHomeIndex);
                }
            }
            if (errorCode == TM_ENOERROR)
            {
/* Ip address has been configured successfully */
                tm_ip_dev_conf_flag(devPtr,mHomeIndex) |= TM_DEV_IP_CONFIG;
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
#ifdef TM_SNMP_CACHE
                tfSnmpdCacheInsertPrefix(AF_INET, devPtr, mHomeIndex);
#endif /* TM_SNMP_CACHE */
                tm_kernel_set_critical;
                devPtr->dev4IpAddressCreated[mHomeIndex] =
                    tm_snmp_time_ticks(tvTime);
                tm_kernel_release_critical;
                devPtr->dev4IpAddressLastChanged[mHomeIndex] =
                    devPtr->dev4IpAddressCreated[mHomeIndex];
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
/* Add the IP address to our cache */
#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
                tf4InsertSortedIpAddr(
                    devPtr,
                    mHomeIndex,
                    &(tm_context(tv4SortedIpAddrCache)),
                    TM_8BIT_ZERO);

                if (    tm_4_ll_is_broadcast(devPtr)
                     && tm_ip_not_match(tm_ip_dev_mask(devPtr, mHomeIndex),
                                        TM_IP_HOST_MASK) )
                {
/* Add the directed broadcast address to our cache */
                    tf4InsertSortedIpAddr(
                        devPtr, mHomeIndex,
                        &(devPtr->devSortedDBroadIpAddrCache),
                        TM_8BIT_YES);
                }
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */

#ifndef TM_SINGLE_INTERFACE_HOME
/* Keep track of maximum configured multi home */
                if (mHomeIndex >= devPtr->devMhomeUsedEntries)
                {
                    devPtr->devMhomeUsedEntries = (tt16Bit)
                                                   ((int)mHomeIndex + 1);
                }
#endif /* !TM_SINGLE_INTERFACE_HOME */
#ifdef TM_IGMP
#ifndef TM_SINGLE_INTERFACE_HOME
                if (devPtr != tm_context(tvLoopbackDevPtr))
#endif /* !TM_SINGLE_INTERFACE_HOME */
                {
                    tfIgmpDevInit(devPtr, mHomeIndex);
                }
#endif /* TM_IGMP */
                if (devPtr->devFlag & TM_DEV_CONNECTING)
                {
/* Connection has been made */
                    devPtr->devFlag |= TM_DEV_CONNECTED;
/* Reset CONNECTING flag */
                    tm_16bit_clr_bit( devPtr->devFlag,
                                      TM_DEV_CONNECTING );
#ifdef TM_SNMP_MIB
                    tm_kernel_set_critical;
                    devPtr->devLastChange = tvTime/10;
                    tm_kernel_release_critical;
#endif /* TM_SNMP_MIB */
                }
/*
 * Add static route through loop back interface for our own IP address
 * for pt2pt networks. Ignore errors.
 */
#ifndef TM_SINGLE_INTERFACE_HOME
                if (   is_pt2pt
                    || (   !(tm_4_ll_is_broadcast(devPtr))
                        && (devPtr != tm_context(tvLoopbackDevPtr))
                        && (tm_ip_not_match(
                                tm_ip_dev_mask(devPtr, mHomeIndex),
                                TM_IP_HOST_MASK))))
                {
                    (void)tfAddStaticRoute(
                        (ttUserInterface)tm_context(tvLoopbackDevPtr),
                        ipAddress,
                        TM_IP_HOST_MASK,
                        (tt4IpAddress)
                        tm_const_htonl(TM_UL(0x7F000001)), 1 );
                }
                else
#endif /* !TM_SINGLE_INTERFACE_HOME */
                {
                    if (   tm_4_ll_is_lan(devPtr)
                        && (!(devPtr->devFlag2 & TM_DEVF2_NO_GRAT_ARP)))
                    {
                        (void)(*(devPtr->devLinkLayerProtocolPtr->
                                 lnkSendRequestFuncPtr))(
                                                       (ttVoidPtr)devPtr,
                                                       mHomeIndex,
                                                       ipAddress,
                                                       (ttVoidPtr)0);
/* Relock, because device unlocked in tfArpSendRequest */
                        tm_call_lock_wait(&(devPtr->devLockEntry));
                    }
                }
            }
            else
            {
/* Could not add to routing table, PPP should close the connection */
                tm_16bit_clr_bit( devPtr->devFlag, TM_DEV_CONNECTING );

#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
                tm_kernel_set_critical;
                tm_context(tvIpData).ipv4InterfaceTableLastChange =
                    tm_snmp_time_ticks(tvTime);
                tm_kernel_release_critical;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */

            }
        }

#ifdef TM_MOBILE_IP4
/* If Mobile IPv4 is enabled and not already open, open it now */
        if (tm_mobile_ip4_enb(devPtr))
        {
            mipVectPtr = devPtr->devMobileIp4VectPtr;

            if ( !(mipVectPtr->mipStatusFlags & TM_MIP_ST_OPEN) )
            {
                (*(mipVectPtr->mipConfigFuncPtr))
                    ( (ttUserInterface)devPtr,
                      TM_MIP_OPEN );
            }
        }
#endif /* TM_MOBILE_IP4 */
    }
    else
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
    if (protocolFamily == PF_INET6)
    {
        if (mHomeIndex >=  TM_6_MAX_MHOME)
/* For analyser benefit */
        {
            errorCode = TM_EINVAL;
        }
        else
        {
/* reset the configuration pending bit, so we aren't stuck in this state */
            tm_8bit_clr_bit( devPtr->dev6IpAddrFlagsArray[mHomeIndex],
                            TM_6_DEV_IP_CONF_STARTED );

#ifdef TM_6_USE_MIP_MN
            if (devPtr == tm_context(tv6MnVect).mns6VirtHomeIfacePtr)
            {
                errorCode = TM_ENOERROR;

                if (tm_8bit_bits_not_set(tm_context(tv6MnVect).mns6Flags,
                                         TM_6_MNS_MOBILE_NODE_ACTIVE))
                {
/* the user must start Mobile IPv6 mobile node first */
                    errorCode = TM_EPERM;
                    goto ExitDeviceStart;
                }
#ifndef TM_6_MN_DISABLE_HOME_DETECT
                else
                {
                    temp6MnStatus = tf6MnGetStatus();
                    if (temp6MnStatus != TM_6_MN_FOREIGN_NETWORK)
                    {
/* can only configure the virtual home interface when away from home */
                        errorCode = TM_EPERM;
                        goto ExitDeviceStart;
                    }
                }
#endif /* TM_6_MN_DISABLE_HOME_DETECT */
            }
#endif /* TM_6_USE_MIP_MN */

/* if the interface already has a link-local scope address (tentative or
   configured), or this is the loopback device */
            if (   (tm_6_link_local_cached(devPtr))
                || (tm_dev_is_loopback(devPtr)) )
            {
/*
 * Indicate that the address has been configured successfully on the interface:
 */
                devPtr->dev6IpAddrFlagsArray[mHomeIndex] |=
                    TM_6_DEV_IP_CONFIG;

#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
#ifdef TM_SNMP_CACHE
                tfSnmpdCacheInsertPrefix(AF_INET6, devPtr, mHomeIndex);
#endif /* TM_SNMP_CACHE */
                tm_kernel_set_critical;
                devPtr->dev6IpAddressCreated[mHomeIndex] =
                    tm_snmp_time_ticks(tvTime);
                tm_kernel_release_critical;
                devPtr->dev6IpAddressLastChanged[mHomeIndex] =
                    devPtr->dev6IpAddressCreated[mHomeIndex];
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */

#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
                tf6InsertSortedIpAddr(
                    devPtr,
                    mHomeIndex,
                    &(tm_context(tv6SortedIpAddrCache)));
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */

/*
 * Update source address cache on this interface - used to quickly pick an
 * appropriate source address for outgoing packets (i.e., one with a similar
 * scope, etc).
 */
                tf6UpdateCacheAddress( devPtr, mHomeIndex );

/* Seed random number generator with the IPv6 address. */
                localIpAddrPtr = &tm_6_ip_dev_addr(devPtr, mHomeIndex);
#ifdef TM_32BIT_DSP
                tfRandSeed( (localIpAddrPtr->ip6Addr.ip6U32[3] >> 16) & 0xff,
                            localIpAddrPtr->ip6Addr.ip6U32[3] & 0xff );
#else /* !TM_32BIT_DSP */
                tfRandSeed( localIpAddrPtr->ip6Addr.ip6U16[6],
                            localIpAddrPtr->ip6Addr.ip6U16[7] );
#endif /* TM_32BIT_DSP */

/*
 * Add local route to destination by calling tfRtAddLocal, which checks to
 * make sure that the local route entry doesn't already exist associated with
 * the same outgoing interface, and also checks in
 * ttRtRadixHeadEntry.rth6NonOnLinkPrefixList to see if the prefix is
 * non-on-link, in which case we don't want to add the local route in the
 * routing table. Destination is the peer's address in the case of a
 * point-to-point interface, and is the device's address in the case of a LAN
 * interface. Note that if the destination is local-scope, the scope ID needs
 * to be embedded in the IPv6 address that we use to add the local route.
 */
                tm_6_ip_copy( localIpAddrPtr, &scopeIpAddr );

/* Embed interface index or site ID in link-local or site-local addresses */
                tm_6_dev_scope_addr(&scopeIpAddr, devPtr);

                is_pt2pt = tm_ll_is_pt2pt(devPtr);
                configDestination = TM_8BIT_YES;
                if (is_pt2pt)
                {
/* the peer PPP IPv6 address is stored on the device with the scope ID
   already embedded */
                    destIpAddrPtr = &(devPtr->dev6Pt2PtPeerIpAddr);
                    if (mHomeIndex != (tt16Bit) TM_MAX_IPS_PER_IF)
                    {
/* this works because PPPv6 disables Duplicate Address Detection, so the
   link-local scope IPv6 address will always be successfully auto-configured
   before any other IPv6 address is configured on the interface. */
                        configDestination = TM_8BIT_ZERO;
                    }
                }
                else
                {
                    destIpAddrPtr = &scopeIpAddr;
                }

                prefixLen = devPtr->dev6PrefixLenArray[mHomeIndex];

/*
 * It is possible that the prefix is actually 'non-on-link'.  In other words,
 * since this prefix is not guaranteed to be on-link, we shouldn't add a
 * route corresponding to the prefix. Instead, we add a local route just for
 * the address being configured, by using a prefix length of 128 (all ones).
 */
#ifdef TM_6_USE_PREFIX_DISCOVERY
                isOnLink = TM_8BIT_YES;

/* Search the non-on-link list */
                tm_6_ip_copy(destIpAddrPtr,
                             &destIpNetAddr);
                tm_6_ip_net_prefix((&destIpNetAddr),
                                   prefixLen,
                                   tempMaskIndex);
                errorCode = tf6RtCheckNonOnLink(
                    &destIpNetAddr, (tt8Bit)prefixLen);
                if (errorCode == TM_ENOERROR)
                {
/*
 * Entry found in list; this address is 'not on-link' so don't add a local
 * route for this prefix.
 */
                    isOnLink  = TM_8BIT_ZERO;
                }
#endif /* TM_6_USE_PREFIX_DISCOVERY */

/*
 * Add a local routing entry only if this device is considered 'on-link',
 * and in the case of a PPP interface, only if this is the first address
 * configured (there can only be one route to our peer).
 */
                if (
#ifdef TM_6_USE_PREFIX_DISCOVERY
                    (isOnLink == TM_8BIT_YES) &&
#endif /* TM_6_USE_PREFIX_DISCOVERY */
#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
                    (!(IN6_IS_ADDR_V4COMPAT(destIpAddrPtr))) &&
#endif /* TM_USE_IPV6 && TM_USE_IPV4 */
                    (configDestination == TM_8BIT_YES))
                {

                    (void) tf6RtAddLocal(devPtr,
                                         destIpAddrPtr, /* key */
                                         prefixLen,
                                         &scopeIpAddr,
                                         mHomeIndex);
                }

                usedMhome = (tt16Bit) (mHomeIndex + 1);
/* Update the number of used multihome entries: */
                if (mHomeIndex < TM_MAX_IPS_PER_IF)
                {
                    if (mHomeIndex >= devPtr->dev6MhomeUsedEntries)
                    {
                        devPtr->dev6MhomeUsedEntries = usedMhome;
                    }
                }

                else if( mHomeIndex <   TM_MAX_IPS_PER_IF
                                      + TM_6_MAX_AUTOCONF_IPS_PER_IF)
                {
                    if (mHomeIndex >= devPtr->dev6MhomeUsedAutoEntries)
                    {
                        devPtr->dev6MhomeUsedAutoEntries = usedMhome;
                    }
                }

#ifdef TM_6_USE_DHCP
                else if( mHomeIndex <   TM_MAX_IPS_PER_IF
                                      + TM_6_MAX_AUTOCONF_IPS_PER_IF
                                      + TM_6_MAX_DHCP_IPS_PER_IA)
                {
                    if (mHomeIndex >= devPtr->dev6MhomeUsedDhcpEntries)
                    {
                        devPtr->dev6MhomeUsedDhcpEntries = usedMhome;
                    }
                }
#endif /*TM_6_USE_DHCP */

#ifdef TM_6_USE_MIP_HA
                else if( mHomeIndex <   TM_MAX_IPS_PER_IF
                                      + TM_6_MAX_AUTOCONF_IPS_PER_IF
                                      + TM_6_MAX_DHCP_IPS_PER_IA
                                      + TM_6_MAX_MIP_PROXY_IPS_PER_IF)
                {
                    if (mHomeIndex >= devPtr->dev6MhomeUsedProxyEntries)
                    {
                        devPtr->dev6MhomeUsedProxyEntries = usedMhome;
                    }
                }
#endif /*TM_6_USE_MIP_HA */
/*
 * If this interface is a point-to-point interface, a site-local,non-on-link
 * address or a non-LAN interface with a prefix that is not all ones.
 */
                if (    (is_pt2pt)
#ifdef TM_6_USE_PREFIX_DISCOVERY
                        || (isOnLink == TM_8BIT_ZERO)
#endif /* TM_6_USE_PREFIX_DISCOVERY */
                        || (    (!tm_6_ll_is_lan(devPtr))
                                && (prefixLen < TM_6_IP_HOST_PREFIX_LENGTH) ) )
                {
                    tm_6_ip_copy_dststruct( localIpAddrPtr,
                                            destAddr.addr.ipv6.sin6_addr );
                    destAddr.addr.ipv6.sin6_scope_id = 0;
                    destAddr.ss_len    = sizeof(struct sockaddr_in6);
                    destAddr.ss_family = AF_INET6;
#ifndef TM_6_USE_RFC3879
/* To depricate site local addresses do not set scope ID */
                    if ( IN6_IS_ADDR_SITELOCAL(localIpAddrPtr) )
                    {
                        destAddr.addr.ipv6.sin6_scope_id = devPtr->dev6SiteId;
                    }
                    else
#endif /* TM_6_USE_RFC3879 */
                    if ( IN6_IS_ADDR_LINKLOCAL(localIpAddrPtr) )
                    {
                        destAddr.addr.ipv6.sin6_scope_id =
                                            (u_long)devPtr->devIndex;
                    }

                    tm_6_ip_copy_structs( in6addr_loopback,
                                          gwAddr.addr.ipv6.sin6_addr );
                    gwAddr.addr.ipv6.sin6_scope_id = 0;
                    gwAddr.ss_len    = sizeof(struct sockaddr_in6);
                    gwAddr.ss_family = AF_INET6;

                    (void) tfNgAddStaticRoute(
                        (ttUserInterface)tm_context(tvLoopbackDevPtr),
                        &destAddr,
                        TM_6_IP_HOST_PREFIX_LENGTH,
                        &gwAddr,  /* loopback address */
                        1 );
                }
            }

            if (tm_6_link_local_cached(devPtr))
            {
                tf6DadConfigComplete( devPtr, mHomeIndex );
                errorCode = TM_ENOERROR;
            }
            else
            {
                if (tm_dev_is_loopback(devPtr))
                {
/*
 * Special case for loopback device, which doesn't have a link local address
 * and doesn't go through DAD.
 */
                    errorCode = TM_ENOERROR;
                }
                else
                {
/* this should never happen */
                    errorCode = TM_EPERM;
                }
            }
        }
    }
    else
#endif /* TM_USE_IPV6 */
    {
/* should never happen */
        errorCode = TM_EAFNOSUPPORT;
    }

#ifdef TM_6_USE_MIP_MN
ExitDeviceStart:
#endif /* TM_6_USE_MIP_MN */

    return errorCode;
}

/****************************************************************************
* FUNCTION: tfNgUnConfigInterface
*
* PURPOSE: This function is a replacement for tfUnConfigInterface to support
*   IPv6.
*
* NEW PARAMETERS:
*   addrFamily:
*       The address family (i.e. AF_INET for IPv4, AF_INET6 for IPv6) of the
*       address to remove from the interface.
*
* RETURNS:
*   TM_EAFNOSUPPORT:
*       addrFamily was set to an invalid value
*
* NOTES:
*   When addrFamily is set to AF_INET6, this API may be used to unconfigure
*   an auto-configured IPv6 address, as well as IPv6 addresses manually
*   configured on the interface by the user.
*
****************************************************************************/
int tfNgUnConfigInterface(
    ttUserInterface interfaceHandle,
    int             addrFamily,
    unsigned int    mHomeIndex )
{
#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
    ttDeviceEntryPtr    devPtr;
#endif /* dual IP layer */
    int                 errorCode;

    errorCode = TM_ENOERROR; /* assume success */

/* parameter validation */
    if (((addrFamily == AF_INET) && (mHomeIndex >= TM_MAX_IPS_PER_IF))
#ifdef TM_USE_IPV6
        || ((addrFamily == AF_INET6) && (mHomeIndex >= (TM_6_MAX_MHOME)))
#endif /* TM_USE_IPV6 */
        )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
        devPtr = (ttDeviceEntryPtr)interfaceHandle;
/* If there is an IPv4-compatible IPv6 address configured in the list of
   multi-homed IPv6 addresses on the interface at the specified multiHomeIndex,
   and the corresponding embedded IPv4 address is configured at the same
   multiHomeIndex in the list of multi-homed IPv4 addresses on the interface,
   then unconfigure the IPv4 address first, and set addrFamily to AF_INET6
   so that we next unconfigure the associated IPv4-compatible IPv6 address */
        if ((devPtr->dev6IpAddrFlagsArray[mHomeIndex]
             & TM_6_DEV_IP_CONFIG)
            && IN6_IS_ADDR_V4COMPAT(
                &(devPtr->dev6IpAddrArray[mHomeIndex])))
        {
            (void) tf4UnConfigInterface(
                interfaceHandle, (tt16Bit) mHomeIndex);
            addrFamily = AF_INET6;
        }
#endif /* dual IP layer */

        switch (addrFamily)
        {
#ifdef TM_USE_IPV4
        case AF_INET:
            errorCode =
                tf4UnConfigInterface(interfaceHandle, (tt16Bit) mHomeIndex);
            break;
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
        case AF_INET6:
            errorCode =
                tf6UnConfigInterface(interfaceHandle, mHomeIndex);
            break;
#endif /* TM_USE_IPV6 */

        default:
            errorCode = TM_EAFNOSUPPORT;
            break;
        }
    }

    return errorCode;
}

#ifdef TM_USE_IPV6
int tf6UnConfigInterface(
    ttUserInterface interfaceHandle,
    unsigned int    mHomeIndex )
{
    ttDeviceEntryPtr    devPtr;
    int                 errorCode;

    errorCode = TM_ENOERROR; /* assume success */
    devPtr = (ttDeviceEntryPtr)interfaceHandle;

/* Verify that an address of the specified addrFamily is configured on the
 * interface at the specified multiHomeIndex; if not, then return TM_ENOENT.
 */
    errorCode = tf6ValidConfigInterface(devPtr, (tt16Bit) mHomeIndex);

    switch(errorCode)
    {
    case TM_ENETDOWN:
        errorCode = TM_ENOENT;
        break;

    case TM_EINPROGRESS:
/* fall through */
    case TM_ENOERROR:
        tm_call_lock_wait(&(devPtr->devLockEntry));
#ifdef TM_6_USE_DHCP
        if (   (mHomeIndex >= TM_MAX_IPS_PER_IF + TM_6_MAX_AUTOCONF_IPS_PER_IF)
            && (  mHomeIndex
                < (  TM_MAX_IPS_PER_IF
                   + TM_6_MAX_AUTOCONF_IPS_PER_IF
                   + TM_6_MAX_DHCP_IPS_PER_IA)))
        {
/* start the process of releasing a DHCPv6 address */
            errorCode = tf6DhcpUnconfig(devPtr, (tt16Bit)mHomeIndex);
            if (errorCode != TM_ENOERROR)
/* DHCPv6 does not have this address configured */
            {
                errorCode =
                    tfRemoveInterface(devPtr, (tt16Bit)mHomeIndex, PF_INET6);
            }
        }
        else
#endif /* TM_6_USE_DHCP */
        {
/* remove the IPv6 address from the interface */
            errorCode =
                tfRemoveInterface(devPtr, (tt16Bit)mHomeIndex, PF_INET6);
        }
        tm_call_unlock(&(devPtr->devLockEntry));
        break;

    default:
        break;
    }

    return errorCode;
}
#endif /* TM_USE_IPV6 */

/* Remove an IP address from an interface */
#ifdef TM_USE_IPV4
int tfUnConfigInterface ( ttUserInterface interfaceHandle,
                          tt8Bit          mHomeIndex )
{
    return tf4UnConfigInterface(
        interfaceHandle, (tt16Bit) mHomeIndex);
}

/* Remove an IP address from an interface, supports more than 256 IP aliases
   configured on an interface. */
int tf4UnConfigInterface (
    ttUserInterface interfaceHandle,
    tt16Bit         mHomeIndex )
{
    int              errorCode;
    ttDeviceEntryPtr devPtr;

    devPtr = (ttDeviceEntryPtr)interfaceHandle;

    errorCode = tfValidInterface(devPtr);
    if (errorCode != TM_ENOERROR)
    {
        errorCode = TM_EINVAL;
    }
#ifndef TM_SINGLE_INTERFACE_HOME
    else if (mHomeIndex >= (TM_MAX_IPS_PER_IF))
    {
        errorCode = TM_ENOENT;
    }
#endif /* TM_SINGLE_INTERFACE_HOME */
    else
    {
        tm_call_lock_wait(&(devPtr->devLockEntry));
        errorCode = tfRemoveInterface(devPtr, mHomeIndex, PF_INET);
        tm_call_unlock(&(devPtr->devLockEntry));
    }
    return errorCode;
}
#endif /* TM_USE_IPV4 */


/****************************************************************************
* FUNCTION: tfRemoveInterface
*
* PURPOSE:
*   This function unconfigures an IP address from an interface.
*
* PARAMETERS:
*    devPtr:
*        Device on which to unconfigure the IP address.
*    mHomeIndex:
*        Multi-home index of IP address to unconfigure on this device. Note
*        that when protocolFamily is PF_INET6 (IPv6), this is the index into
*        ttDeviceEntry.dev6IpAddrArray, which stores both manually configured
*        and automatically configured IPv6 addresses.
*    protocolFamily:
*        Protocol family associated with the IP address to be unconfigured on
*        the device/interface (i.e. PF_INET, PF_INET6).
*
* RETURNS:
*   TM_ENOERROR:
*       Successful.
*   TM_ENOENT:
*       Interface was not configured at the specified multi-home index.
*
* NOTES:
*
****************************************************************************/
int tfRemoveInterface (ttDeviceEntryPtr devPtr,
                       tt16Bit          mHomeIndex,
                       int              protocolFamily)
{
    ttLinkLayerEntryPtr linkLayerEntryPtr;
#ifdef TM_6_USE_MIP_MN
    tt6MnHaBulEntryPtr  haBulPtr;
    ttGenericUnion      timerParm;
#endif /* TM_6_USE_MIP_MN */
#ifdef TM_USE_IPV4
    tt4IpAddress        destIpAddress;
    tt4IpAddress        ipAddress;
    tt4IpAddress        netMask;
    tt4IpAddress        gatewayIpAddress;
#endif /* TM_USE_IPV4 */
#ifdef TM_4_USE_SCOPE_ID
    tt6IpAddress        scopeDestIpAddr;
#ifndef TM_LITTLE_ENDIAN
    tt4IpAddress        tempBitTest;
#endif /* TM_LITTLE_ENDIAN */
#endif /* TM_4_USE_SCOPE_ID */
#ifdef TM_USE_IPV6
#ifdef TM_6_USE_DAD
    tt6DadEntryPtr      dadEntryPtr;
#endif /* TM_6_USE_DAD */
    tt6IpAddressPtr     localIpAddrPtr;
    tt6RtKeyCommand     rtKeyCommand;
    tt6IpAddress        scopeIpAddr;
#endif /* TM_USE_IPV6 */

#ifndef TM_SINGLE_INTERFACE_HOME
#ifdef TM_USE_IPV4
    int                 maxMhomeEntries;
    int                 maxConfMhomeEntry;
    int                 retCode;
    int                 i;
#endif /* TM_USE_IPV4 */
#endif /* ! TM_SINGLE_INTERFACE_HOME */
#ifdef TM_USE_IPV4
    int                 hops;
    int                 retCodeDefGwy;
#endif /* TM_USE_IPV4 */
    int                 errorCode;
#ifdef TM_USE_IPV6
    tt16Bit             prefixLen;
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
    tt8Bit              bootIndex;
    tt8Bit              devIpConfFlag;
#endif /* TM_USE_IPV4 */

    tm_assert_is_locked(tfRemoveInterface, &(devPtr->devLockEntry));

#ifdef TM_USE_SSDP
    tm_call_unlock(&(devPtr->devLockEntry));
    tfSsdpStopAdvertInterface(devPtr, mHomeIndex, protocolFamily);
/* relock the device */
    tm_call_lock_wait(&(devPtr->devLockEntry));
#endif  /* TM_USE_SSDP */

    errorCode = TM_ENOERROR;

#if (defined(TM_SNMP_MIB) && defined(TM_USE_NG_MIB2) && defined(TM_SNMP_CACHE))
    tfSnmpdCacheDeletePrefix((tt8Bit)protocolFamily, devPtr, mHomeIndex);
#endif /* (defined(TM_SNMP_MIB) && defined(TM_USE_NG_MIB2) && defined(TM_SNMP_CACHE)) */
#ifdef TM_USE_IPV4
    if (protocolFamily == PF_INET)
    {
        bootIndex = TM_BOOT_NONE;
        if ( tm_8bit_one_bit_set( tm_ip_dev_conf_flag(devPtr,
                                                      mHomeIndex),
                                  (TM_DEV_IP_DHCP) ) )
        {
            bootIndex = TM_BOOT_DHCP;
        }
        else if ( tm_8bit_one_bit_set( tm_ip_dev_conf_flag(devPtr,
                                                           mHomeIndex),
                                       (TM_DEV_IP_BOOTP) ) )
        {
            bootIndex = TM_BOOT_BOOTP;
        }
        devIpConfFlag = tm_ip_dev_conf_flag(devPtr, mHomeIndex);
        if (bootIndex != TM_BOOT_NONE)
        {
            if (devPtr->devBootInfo[bootIndex].bootFuncPtr
                                                     != (ttDevBootFuncPtr)0)
            {
                (void)(*(devPtr->devBootInfo[bootIndex].bootFuncPtr))
                                                      ( devPtr,
                                                        TM_LL_CLOSE_STARTED,
                                                        mHomeIndex );
            }
        }
        if ( tm_8bit_one_bit_set( devIpConfFlag, TM_DEV_IP_CONFIG) )
        {
/* remove the IP address from our cache */
#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
            tfRemoveSortedIpAddr(
                devPtr, mHomeIndex, &(tm_context(tv4SortedIpAddrCache)));

            if (    tm_4_ll_is_broadcast(devPtr)
                 && tm_ip_not_match(tm_ip_dev_mask(devPtr, mHomeIndex),
                                    TM_IP_HOST_MASK) )
            {
/* remove the directed broadcast address from our cache */
                tfRemoveSortedIpAddr(
                    devPtr, mHomeIndex,
                    &(devPtr->devSortedDBroadIpAddrCache));
            }
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */

/* If device/mHomeIndex index already configured, delete from routing table */
            tm_ip_copy(tm_ip_dev_addr(devPtr, mHomeIndex), ipAddress);
            tm_ip_copy(tm_ip_dev_mask(devPtr, mHomeIndex), netMask);
#ifdef TM_4_USE_SCOPE_ID
            tm_6_addr_to_ipv4_mapped(ipAddress, &scopeIpAddr);
/* Embed interface index or site ID in link-local or site-local addresses */
            tm_4_dev_scope_addr(&scopeIpAddr, devPtr);
/* Prefix length of the IPV4 address */
#ifdef TM_LITTLE_ENDIAN
            tm_ip_len_mask(netMask, prefixLen);
#else /* TM_BIG_ENDIAN */
            tm_ip_len_mask(netMask, prefixLen, tempBitTest);
#endif /* TM_BIG_ENDIAN */
#endif /* TM_4_USE_SCOPE_ID */

            if ( (tm_ll_is_pt2pt(devPtr)) )
            {
                if (mHomeIndex == TM_16BIT_ZERO)
                {
                    tm_ip_copy(
                        tfDeviceDestIpAddress(devPtr), destIpAddress);

#ifdef TM_4_USE_SCOPE_ID
                    tm_6_addr_to_ipv4_mapped(destIpAddress, &scopeDestIpAddr);
/* Embed interface index or site ID in link-local or site-local addresses */
                    tm_4_dev_scope_addr(&scopeDestIpAddr, devPtr);
#endif /* TM_4_USE_SCOPE_ID */

#ifdef TM_4_USE_SCOPE_ID
                    errorCode = tf6RtDelRoute(
#ifdef TM_USE_STRONG_ESL
                        devPtr,
#endif /* TM_USE_STRONG_ESL */
                        &scopeDestIpAddr,
                        (int) prefixLen + TM_6_IPV6_PREFIXLEN_INCREASE,
                        TM_RTE_LOCAL);
#else /* ! TM_4_USE_SCOPE_ID */
                    errorCode = tf4RtDelRoute(
#ifdef TM_USE_STRONG_ESL
                        devPtr,
#endif /* TM_USE_STRONG_ESL */
                        destIpAddress, netMask, TM_RTE_LOCAL );
#endif /* ! TM_4_USE_SCOPE_ID */
                }
            }
            else
            {
/*
 * BUG ID 1158:
 * If the loop back address on the local routing entry is ours, it needs to
 * be changed. So remove the local route unconditionally, and if there is
 * another configured IP address on the same subnet re-add it.
 */
#ifdef TM_USE_STRONG_ESL
                if (devPtr == tm_context(tvLoopbackDevPtr))
                {
/* No configured default gateway on the loop back interface */
                    retCodeDefGwy = TM_ENOENT;
                    tm_ip_copy(TM_IP_ZERO, gatewayIpAddress);
                    hops = 0;
                }
                else
#endif /* TM_USE_STRONG_ESL */
                {
                    retCodeDefGwy = tf4RtGetDefault(
#ifdef TM_USE_STRONG_ESL
                                             devPtr,
#endif /* TM_USE_STRONG_ESL */
                                             &gatewayIpAddress,
                                             &hops);
                }
#ifdef TM_4_USE_SCOPE_ID
                errorCode = tf6RtDelRoute(
#ifdef TM_USE_STRONG_ESL
                    devPtr,
#endif /* TM_USE_STRONG_ESL */
                    &scopeIpAddr,
                    (int) prefixLen + TM_6_IPV6_PREFIXLEN_INCREASE,
                    TM_RTE_LOCAL);
#else /* ! TM_4_USE_SCOPE_ID */
                errorCode = tf4RtDelRoute(
#ifdef TM_USE_STRONG_ESL
                                devPtr,
#endif /* TM_USE_STRONG_ESL */
                                ipAddress, netMask, TM_RTE_LOCAL );
#endif /* ! TM_4_USE_SCOPE_ID */
                if (tm_4_ll_is_lan(devPtr))
                {
                    linkLayerEntryPtr = devPtr->devLinkLayerProtocolPtr;
                    if (    (linkLayerEntryPtr != TM_LINK_NULL_PTR)
                         && (linkLayerEntryPtr->lnkResolveCleanFuncPtr !=
                                    (ttLnkResolveCleanFuncPtr)0) )
                    {
                        (void)(*(linkLayerEntryPtr->lnkResolveCleanFuncPtr))(
                                                   (ttVoidPtr)devPtr,
                                                   ipAddress,
                                                   netMask);
                    }
                }
#ifndef TM_SINGLE_INTERFACE_HOME
/* Bug ID 667:
 * the user could have configured multiple IP addresses on the interface each
 * on the same subnet. In that case, we only want to delete the local route
 * entry for the last of these IP addresses that they remove from the
 * interface.
 * BUG ID 1158:
 * If we do not remove the local route when multiple IP addresses are
 * configured on the same subnet, then it is possible that the local route
 * will use the IP address just removed as a loop back addres, and that will
 * cause problems. So it is better to remove the local route unconditionally,
 * and to re-add it with a configured IP address.
 */
                maxMhomeEntries = (int) devPtr->devMhomeUsedEntries;
                for ( i = 0; i < maxMhomeEntries; i++ )
                {
/* make sure we don't check the IP address we are removing */
                    if ((i != (int) mHomeIndex) &&
                        ( tm_8bit_one_bit_set( tm_ip_dev_conf_flag(devPtr,
                                                                   i),
                                               TM_DEV_IP_CONFIG) ) )
                    {
                        destIpAddress = tm_ip_dev_addr(devPtr, i);
                        if (    (netMask == tm_ip_dev_mask(devPtr, i))
                             && tm_ip_net_match(ipAddress,
                                               destIpAddress,
                                               netMask) )
                        {
/*
 * We found a subnet match on a different IP address on the same interface
 * BUG ID 1158: add it as local route.
 */
                            retCode = tf4RtAddLocal(devPtr,
                                                /* key: */
                                                destIpAddress,
                                                netMask,
                                                destIpAddress,
                                                (tt16Bit)i);
                            if (    (retCodeDefGwy == TM_ENOERROR)
                                 && (retCode == TM_ENOERROR))
/* Re-add the default gateway, as it could have been removed */
                            {
#ifdef TM_USE_IPV6
/*
* In dual IP layer mode, we must always have an IPv4 default gateway entry
* in the routing tree, even if it isn't configured. Replace the existing
* default gateway, if it is not configured.
*/
                                (void)tf4RtAddDefGw(devPtr,
                                                    gatewayIpAddress
#ifdef TM_USE_STRONG_ESL
                                                  , hops
#endif /* TM_USE_STRONG_ESL */
                                         );
#else /* ! dual IP layer */
                                (void)tf4RtAddRoute(
                                            devPtr, TM_IP_ZERO, TM_IP_ZERO,
                                            gatewayIpAddress, TM_16BIT_ZERO,
/*
 * Note that the add route function will set the mhome index from
 * the matching local route. So the passed value is irrelevant.
 */
                                            TM_16BIT_ZERO, hops,
                                            TM_RTE_INF,
                                            TM_RTE_STATIC | TM_RTE_INDIRECT,
                                            TM_8BIT_YES );
#endif /* ! dual IP layer */
                            }
                            break; /* out of the loop */
                        }
                    }
                }
#endif /* ! TM_SINGLE_INTERFACE_HOME */
            }

/* Remove static routes via loop back for our own IP address. Ignore errors.*/
#ifdef TM_4_USE_SCOPE_ID
            (void) tf6RtDelRoute(
#ifdef TM_USE_STRONG_ESL
                devPtr,
#endif /* TM_USE_STRONG_ESL */
                &scopeIpAddr, TM_6_IP_HOST_PREFIX_LENGTH, TM_RTE_STATIC);
#else /* ! TM_4_USE_SCOPE_ID */
            (void)tf4RtDelRoute(
#ifdef TM_USE_STRONG_ESL
                devPtr,
#endif /* TM_USE_STRONG_ESL */
                ipAddress, TM_IP_HOST_MASK, TM_RTE_STATIC
#ifndef TM_SINGLE_INTERFACE_HOME
                                 | TM_RTE_LINK_LAYER
#endif /* TM_SINGLE_INTERFACE_HOME */
                                 );
#endif /* ! TM_4_USE_SCOPE_ID */
#ifndef TM_SINGLE_INTERFACE_HOME
/* Keep track of highest configured multi home */
            if ((int)mHomeIndex + 1 == (int)devPtr->devMhomeUsedEntries)
            {
                if (mHomeIndex != TM_16BIT_ZERO)
                {
                    maxConfMhomeEntry = (tt16Bit)(mHomeIndex - 1);
                    do
                    {
                        if ( tm_8bit_one_bit_set(
                                      tm_ip_dev_conf_flag(devPtr,
                                                          maxConfMhomeEntry),
                                      TM_DEV_IP_CONFIG ) )
                        {
                            break;
                        }
                    }
/* PRQA: QAC Message 3416: It is processing intended by the loop count */
/* PRQA: QAC Message 3416: processing */
/* PRQA S 3416 L1 */
                    while ((tt16Bit)(maxConfMhomeEntry--)  != TM_16BIT_ZERO);
/* PRQA L:L1 */
                    devPtr->devMhomeUsedEntries = (tt16Bit)
                                                     (maxConfMhomeEntry + 1);
                }
                else
                {
                    devPtr->devMhomeUsedEntries = TM_16BIT_ZERO;
                }
            }
#endif /* TM_SINGLE_INTERFACE_HOME */
#ifdef TM_IGMP
#ifndef TM_SINGLE_INTERFACE_HOME
            if (    (devPtr != tm_context(tvLoopbackDevPtr))
                 && (devPtr->devMhomeUsedEntries == (tt16Bit)0))
#endif /* !TM_SINGLE_INTERFACE_HOME */
            {
                tfIgmpDevClose(devPtr);
            }
#endif /* TM_IGMP */
        }
        else
        {
            if ( tm_8bit_bits_not_set(devIpConfFlag, TM_DEV_IP_CONF_STARTED) )
            {
                errorCode = TM_ENOENT;
            }
        }
/* Reset configuration flag */
        tm_ip_dev_conf_flag(devPtr, mHomeIndex) = TM_8BIT_ZERO;
#ifdef TM_USE_VCHAN
/* Reinitialize virtual channel to 0 */
        tm_ip_dev_vchan(devPtr, mHomeIndex) = 0;
#endif /* TM_USE_VCHAN */
/* Clear IP address/netmask/dest address from this entry */
        tm_ip_copy(TM_IP_ZERO, tm_ip_dev_addr(devPtr, mHomeIndex));
        tm_ip_copy(TM_IP_ZERO, tm_ip_dev_mask(devPtr, mHomeIndex));
        tm_ip_copy(TM_IP_ZERO, tm_ip_dev_dbroad_addr(devPtr, mHomeIndex));
        tm_4_ip_dev_prefixLen(devPtr,mHomeIndex) = TM_8BIT_ZERO;
    }
    else
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
    if (protocolFamily == PF_INET6)
    {
        tf6DelayInitSolTimerRemove(devPtr, mHomeIndex);
        if (devPtr->dev6IpAddrFlagsArray[mHomeIndex] & TM_6_DEV_IP_CONFIG)
        {
#ifdef TM_OPTIMIZE_MANY_MHOMES
/* Remove the IPv6 address from our cache */
            tfRemoveSortedIpAddr(
                devPtr, mHomeIndex, &(tm_context(tv6SortedIpAddrCache)));
#endif /* TM_OPTIMIZE_MANY_MHOMES */

/* Local IP address */
            localIpAddrPtr = &(devPtr->dev6IpAddrArray[mHomeIndex]);
/* Delete local route from routing table */
/* If the local IP address we are unconfiguring is local-scope, the scope ID
   needs to be embedded in the IPv6 address that we use to delete the local
   route. */
            tm_6_ip_copy( localIpAddrPtr, &scopeIpAddr );

/* Embed interface index or site ID in link-local or site-local addresses */
            tm_6_dev_scope_addr(&scopeIpAddr, devPtr);
            prefixLen = (tt16Bit) devPtr->dev6PrefixLenArray[mHomeIndex];
            if (!(tm_ll_is_pt2pt(devPtr)))
            {
/* verify that we have a local route in the routing tree associated with
   this interface, if not then there is nothing to remove */
                rtKeyCommand.rkc6Command       = TM_6_RT_KEY_EXISTS_ON_DEV;
                rtKeyCommand.rkc6AddrPtr       = &scopeIpAddr;
                rtKeyCommand.rkc6PrefixLength  = (tt8Bit) prefixLen;
                rtKeyCommand.rkc6DevPtr        = devPtr;
/* unused */
                rtKeyCommand.rkc6EndpointAddrPtr = TM_6_IPADDR_NULL_PTR;
                rtKeyCommand.rkc6ValidLifetime = 0; /* unused */
#ifdef TM_6_USE_PREFIX_DISCOVERY
/* Do not include non-on link search */
                rtKeyCommand.rkc6Flags = TM_16BIT_ZERO;
#endif /* TM_6_USE_PREFIX_DISCOVERY */
                errorCode = tf6RtKeyCommand(&rtKeyCommand, TM_8BIT_YES);

                if (errorCode == TM_ENOERROR)
                {
/*
 * BUG ID 1158:
 * If the loop back address on the local routing entry is ours, it needs to
 * be changed. So remove the local route unconditionally, and if there is
 * another configured IP address on the same subnet re-add it.
 */
                    errorCode =
                        tf6RtDelRoute(
#ifdef TM_USE_STRONG_ESL
                            devPtr,
#endif /* TM_USE_STRONG_ESL */
                            &scopeIpAddr,
                            (int) prefixLen,
                            TM_RTE_LOCAL );
                    linkLayerEntryPtr = devPtr->devLinkLayerProtocolPtr;
                    if (    (linkLayerEntryPtr != TM_LINK_NULL_PTR)
                         && (linkLayerEntryPtr->lnk6ResolveCleanFuncPtr !=
                                    (tt6LnkResolveCleanFuncPtr)0) )
                    {
                        (void)(*(linkLayerEntryPtr->lnk6ResolveCleanFuncPtr))(
                                                       (ttVoidPtr)devPtr,
                                                       &scopeIpAddr,
                                                       (int) prefixLen);
                    }
                }
            }

/* Remove static routes via loop back for our own IP address. Ignore errors.*/
            (void)tf6RtDelRoute(
#ifdef TM_USE_STRONG_ESL
                                 devPtr,
#endif /* TM_USE_STRONG_ESL */
                                 &scopeIpAddr,
                                 TM_6_IP_HOST_PREFIX_LENGTH,
                                 TM_RTE_STATIC | TM_RTE_LINK_LAYER );

/* if there is another address configured on this interface with the same
   prefix, then we need to add back the local route */
/* Only leave the solicited-node multicast group if this is the last address
   on this interface with the same solicited-node multicast group, either
   manually configured or autoconfigured. ([RFC2461].R7.2.1:30) */

            tf6CheckOtherMhomeConfig(devPtr, mHomeIndex);

/* Reset configuration flag and addresses for this multihome entry */
            tf6ResetMhomeConfig (devPtr, mHomeIndex);

            if (!tm_6_dev_is_enabled(devPtr))
            {
                if ( (tm_ll_is_pt2pt(devPtr)) )
                {
/*
 * Only remove the route when we disable the interface for IPv6, which is
 * typically when the auto-configured link-local scope IPv6 address is
 * unconfigured.
 */
/* the peer PPP IPv6 address is stored on the device with the scope ID
   already embedded */
                    errorCode =
                        tf6RtDelRoute(
#ifdef TM_USE_STRONG_ESL
                            devPtr,
#endif /* TM_USE_STRONG_ESL */
                            &(devPtr->dev6Pt2PtPeerIpAddr),
                            (int)prefixLen,
                            TM_RTE_LOCAL );
                }
            }
            if (mHomeIndex < TM_MAX_IPS_PER_IF)
            {
/* Manually configured address */
                if (    (int)mHomeIndex + 1
                     == (int)devPtr->dev6MhomeUsedEntries)
                {
                    devPtr->dev6MhomeUsedEntries = mHomeIndex;
                }
            }

#ifdef TM_USE_IPV6
            else if (mHomeIndex <   TM_MAX_IPS_PER_IF
                                  + TM_6_MAX_AUTOCONF_IPS_PER_IF)
            {
/* Autoconfigured address */
                if (    (int)mHomeIndex + 1
                     == (int)devPtr->dev6MhomeUsedAutoEntries)
                {
                    devPtr->dev6MhomeUsedAutoEntries = mHomeIndex;
                }
            }
#endif /* TM_USE_IPV6 */

#ifdef TM_6_USE_DHCP
            else if (mHomeIndex <   TM_MAX_IPS_PER_IF
                                  + TM_6_MAX_AUTOCONF_IPS_PER_IF
                                  + TM_6_MAX_DHCP_IPS_PER_IA)
            {
/* DHCPv6 address address */
                if (    (int)mHomeIndex + 1
                     == (int)devPtr->dev6MhomeUsedDhcpEntries)
                {
                    devPtr->dev6MhomeUsedDhcpEntries = mHomeIndex;
                }
            }
#endif /* TM_6_USE_DHCP */

#ifdef TM_6_USE_MIP_HA
            else if (    (int)mHomeIndex + 1
                        == (int)devPtr->dev6MhomeUsedProxyEntries)
            {
                devPtr->dev6MhomeUsedProxyEntries = mHomeIndex;
            }
#endif /*TM_6_USE_MIP_HA */
            if (mHomeIndex == (tt16Bit) TM_MAX_IPS_PER_IF)
            {
/* unconfigure any remaining auto-configured IPv6 addresses, since the
   auto-configured link-local scope IPv6 address was just unconfigured */
                (void) tf6UnConfigInterfaceId(devPtr);
/* Reset the flag so we can send Router Solicitations again */
                tm_8bit_clr_bit( devPtr->dev6Flags,
                                 (   TM_6_DEV_INIT_DELAY_COMPLETE
                                   | TM_6_DEV_INIT_DELAY_STARTED ) );
            }
        }
        else if (devPtr->dev6IpAddrFlagsArray[mHomeIndex] &
                 TM_6_DEV_IP_CONF_STARTED)
        {
/* Configuration is pending for this IPv6 address. We need to clean up here */

#ifdef TM_6_USE_DAD
/* clean up the DAD entry and timer */
            dadEntryPtr = tf6DadFindEntry(
                devPtr, &devPtr->dev6IpAddrArray[mHomeIndex]);
            tf6DadRemoveEntry(devPtr, dadEntryPtr);
#endif /* TM_6_USE_DAD */

/* Only leave the solicited-node multicast group if this is the last address
   on this interface with the same solicited-node multicast group, either
   manually configured or autoconfigured. ([RFC2461].R7.2.1:30) */
            tf6CheckOtherMhomeConfig(devPtr, mHomeIndex);

/* Reset configuration flag and addresses for this multihome entry */
            tf6ResetMhomeConfig(devPtr, mHomeIndex);
        }
        else
        {
            errorCode = TM_ENOENT;
        }

#ifdef TM_6_USE_MIP_MN
/* deregister any active BUL entries for this home address */
        if ((devPtr == tm_context(tv6MnVect).mns6VirtHomeIfacePtr)
#ifdef TM_6_USE_MIP_HA
            && (mHomeIndex < TM_6_MAX_LOCAL_MHOME)
#endif /* TM_6_USE_MIP_HA */
            )
        {
/* we cannot lock the mobile node state vector after the device, because
   there are other places where we lock in the reverse order. So, unlock the
   device first */
            tm_call_unlock(&(devPtr->devLockEntry));

/* lock the mobile node state vector */
            tm_call_lock_wait(&(tm_context(tv6MnVect).mns6LockEntry));

            haBulPtr =
                tm_context(tv6MnVect).mns6BulPtr[mHomeIndex];
            if (haBulPtr != TM_6_MN_HA_BUL_NULL_PTR)
            {
                tm_assert(tfRemoveInterface,
                          (mHomeIndex == haBulPtr->hab6HomeAddrMhomeIndex));

/* if there is a failure timer currently running, stop it */
                if (haBulPtr->hab6UserFailureTmrPtr != TM_TMR_NULL_PTR)
                {
                    tm_timer_remove(haBulPtr->hab6UserFailureTmrPtr);
                    haBulPtr->hab6UserFailureTmrPtr = TM_TMR_NULL_PTR;
                }

/* tell the BUL entry we want to deregister */
                haBulPtr->hab6UserDeregStatus = TM_8BIT_YES;
                (void) tf6MnStateMachine(
                    haBulPtr,
                    TM_PACKET_NULL_PTR,
                    TM_6_MIP_USER_DEREGISTER_EVENT);

/* start the failure timer */
                timerParm.genVoidParmPtr = (ttVoidPtr) haBulPtr;
                haBulPtr->hab6UserFailureTmrPtr = tfTimerAdd(
                    tf6MnHaBulFailureTimer,
                    timerParm,
                    timerParm,
                    tm_context(tv6MnDeregTimeoutMsec),
                    0);
            }

/* unlock the mobile node state vector */
            tm_call_unlock(&(tm_context(tv6MnVect).mns6LockEntry));

/* relock the device */
            tm_call_lock_wait(&(devPtr->devLockEntry));
        }
#endif /* TM_6_USE_MIP_MN */
    }
    else
#endif /* TM_USE_IPV6 */
    {
/* should never happen */
        errorCode = TM_EAFNOSUPPORT;
    }

    return errorCode;
}

int tfCloseInterface(ttUserInterface interfaceHandle)
{
    ttDeviceEntryPtr    devPtr;
    int                 errorCode;

    devPtr = (ttDeviceEntryPtr)interfaceHandle;
/* Check parameter */
    errorCode = tfValidInterface(devPtr);
    if ( errorCode == TM_ENOERROR )
    {
/* Stop the Link Layer for this device */
#ifdef TM_6_USE_MIP_MN
#ifndef TM_6_MN_DISABLE_HOME_DETECT
        if (((ttDeviceEntryPtr) interfaceHandle)
            == tm_context(tv6MnVect).mns6VirtHomeIfacePtr)
        {
/* lock the mobile node state vector */
            tm_call_lock_wait(&(tm_context(tv6MnVect).mns6LockEntry));

/* force a return to home to close the virtual home interface */
            errorCode = tf6MnMoveToHome();

/* unlock the mobile node state vector */
            tm_call_unlock(&(tm_context(tv6MnVect).mns6LockEntry));
        }
        else
#endif /* ! TM_6_MN_DISABLE_HOME_DETECT */
#endif /* TM_6_USE_MIP_MN */
        {
            tm_call_lock_wait(&(devPtr->devLockEntry));
#ifdef TM_USE_IPV6
/* Remove the Router Solicitation timer */
            tf6RemoveRtrSol(devPtr);
#endif /* TM_USE_IPV6 */
/* Stop DHCPv6 if TM_6_DEV_USER_DHCP is set in the ipv6flags*/
#ifdef TM_6_USE_DHCP
            if (   (devPtr->dev6BootFuncPtr != (ttDev6BootFuncPtr)0)
                && (   (devPtr->dev6Flags & TM_6_DEV_USER_DHCP)
                    || (devPtr->dev6Flags & TM_6_DEV_USER_DHCP_RTR_CTRLD)))
            {
                devPtr->dev6Flags |= TM_6_DEV_CLOSING;
                (*(devPtr->dev6BootFuncPtr))(devPtr, 0, TM_8BIT_NO);
            }
            else
#endif /* TM_6_USE_DHCP */
            {
                errorCode = tfDeviceClose(devPtr);
            }
            tm_call_unlock(&(devPtr->devLockEntry));
        }
    }
    return errorCode;
}

/*
 * Receive a packet from the Interface
 */
int tfRecvInterface(ttUserInterface interfaceHandle)
{
    int                    errorCode;
    int                    retCode;
    int                    dataSize;
    char          TM_FAR * dataPtr;
    ttPacketPtr            packetPtr;
    ttPacketPtr            newPacketPtr;
    ttDeviceEntryPtr       devPtr;
#ifdef TM_DEV_RECV_OFFLOAD
    ttDevRecvOffloadStruct devRecvOffload;
#endif /* TM_DEV_RECV_OFFLOAD */
#if defined(TM_USE_DRV_SCAT_RECV) && defined(TM_NEED_ETHER_32BIT_ALIGNMENT)
    ttCharPtr               savDataPtr;
    ttPacketPtr             firstPacketPtr;
    int                     length;
    int                     allocLength;
    int                     needAlignment;
#endif /* TM_USE_DRV_SCAT_RECV && TM_NEED_ETHER_32BIT_ALIGNMENT */

#if defined(TM_USE_DRV_SCAT_RECV) && defined(TM_NEED_ETHER_32BIT_ALIGNMENT)
    firstPacketPtr = (ttPacketPtr)0;
    savDataPtr = (ttCharPtr)0; /* For compiler warning */
#endif /* TM_USE_DRV_SCAT_RECV && TM_NEED_ETHER_32BIT_ALIGNMENT */
    devPtr = (ttDeviceEntryPtr)interfaceHandle;
    errorCode = TM_ENOERROR;
    retCode = TM_ENOERROR;
/* Initialize the dataPtr to avoid warning */
    dataPtr = (char TM_FAR *)0;
    dataSize = 0; /* In case the user forgets to set it */
    packetPtr = (ttPacketPtr)0; /* In case the user forgets to set it */
#ifdef TM_DEV_RECV_OFFLOAD
/* moved here for compiler warning */
    devRecvOffload.devoRFlags = TM_16BIT_ZERO;
#endif /* TM_DEV_RECV_OFFLOAD */
/* Call the drivers receive routine */
/* Lock the send/receive to provide single threaded access to the recv */
    tm_lock_wait(&(devPtr->devDriverLockEntry));
    if ( tm_16bit_one_bit_set( devPtr->devFlag, TM_DEV_OPENED) )
    {
        retCode = (*(devPtr->devRecvFuncPtr))
                        ( interfaceHandle, &dataPtr, (int TM_FAR *)&dataSize,
                          (ttUserBufferPtr)(ttVoidPtr)&packetPtr
#ifdef TM_DEV_RECV_OFFLOAD
                          , &devRecvOffload
#endif /* TM_DEV_RECV_OFFLOAD */
                          );
    }
    else
    {
        retCode = TM_ENXIO;
    }
/* Unlock the send/receive */
    tm_unlock(&(devPtr->devDriverLockEntry));
    if (retCode != TM_ENOERROR)
    {
        goto recvInterfaceFinish;
    }
#ifdef TM_ERROR_CHECKING
/* Check on user device values */
    if (    ( dataPtr == (char TM_FAR *)0 )
         || ( dataSize == 0 )
         || (    (packetPtr != (ttPacketPtr)0)
/* Corrupted packet */
              && (!(packetPtr->pktUserFlags & TM_PKTF_USER_DEV_OWNS)) ) )
    {
        errorCode = TM_ENOBUFS;
#if (defined(TM_SNMP_MIB) || defined(TM_PPP_LQM))
        devPtr->devInErrors++;
#endif /* TM_SNMP_MIB or TM_PPP_LQM */
        goto recvInterfaceFinish;
    }
#endif /* TM_ERROR_CHECKING */
#ifdef TM_USE_INDRT
    tm_kernel_set_critical;
    if (devPtr->devRecvIsrCount != 0)
    {
        devPtr->devRecvIsrCount--;
        if (devPtr->devIndrtPosRecvIsr != 0)
/* There is an INDRT packet queued to the recv task */
        {
            devPtr->devIndrtPosRecvIsr--;
            if (devPtr->devIndrtPosRecvIsr == 0)
/*
 * We are receiving the INDRT packet. Just set and error, so that
 * packet can be freed, and exit
 */
            {
                errorCode = TM_EPERM;
                tm_kernel_release_critical;
                goto recvInterfaceFinish;
            }
        }
    }
    tm_kernel_release_critical;
#endif /* TM_USE_INDRT */
/*
 * Copy small packets to free large ethernet buffers, if user set this
 * option. Note that the user cannot set this option (with
 * tfInterfaceSetOptions) on a point to point device. It is checked in the
 * tfInterfaceSetOptions() function, so we do not have to check on that
 * here for each received packet.
 */
#if defined(TM_USE_DRV_SCAT_RECV) && defined(TM_NEED_ETHER_32BIT_ALIGNMENT)
#define tm_is_round(item) ( (((int)(item)) & TM_ROUND_SIZE) == 0 )
    needAlignment =
        (tm_ll_need_32bit_align(devPtr) && tm_is_round(dataPtr));
#endif /* TM_USE_DRV_SCAT_RECV && TM_NEED_ETHER_32BIT_ALIGNMENT */
    if (   ( ((tt16Bit) dataSize) <= devPtr->devRecvCopyThreshHold )
#if defined(TM_USE_DRV_SCAT_RECV) && defined(TM_NEED_ETHER_32BIT_ALIGNMENT)
         || (    (needAlignment)
/* Should be bigger than TM_DEV_DEF_RECV_CONT_HDR_LENGTH */
              && (dataSize <= TM_DEV_DEF_RECV_CONT_BUF_LENGTH)  )
#endif /* TM_USE_DRV_SCAT_RECV && TM_NEED_ETHER_32BIT_ALIGNMENT */
       )
    {
/*
 * Allocate a new packet to copy the data into.  TM_ETHER_IP_ALIGN specifies
 * the number of bytes that is necessary to align the IP header on a long word
 * boundary (usually two bytes).  This is needed because the Ethernet header
 * will be long word  aligned, but since the Ethernet header is 12 bytes long,
 * the IP header will be misaligned.
 */
        newPacketPtr = tfGetSharedBuffer(TM_ETHER_IP_ALIGN,
                                         (ttPktLen)dataSize,
                                         TM_16BIT_ZERO);
        if (newPacketPtr != TM_PACKET_NULL_PTR)
        {
            tm_bcopy(dataPtr,
                     newPacketPtr->pktLinkDataPtr,
                     tm_packed_byte_count(dataSize));
            tfFreeDrvRecvBuffer(devPtr, packetPtr, dataPtr);
            packetPtr = newPacketPtr;
            dataPtr = (char TM_FAR *)newPacketPtr->pktLinkDataPtr;
        }
    }
#if defined(TM_USE_DRV_SCAT_RECV) && defined(TM_NEED_ETHER_32BIT_ALIGNMENT)
    else
    {
        if (needAlignment)
        {
/*
 * We will make the Ethernet header start at a 2 bytes offset, and
 * therefore guarantee that the IP header/ARP header are on a 4-byte boundary
 * profided that tfKernelMalloc() returns pointers on a 4-byte boundary.
 */
            length = TM_DEV_DEF_RECV_CONT_HDR_LENGTH;
/* This should not be necessary */
            if (length > TM_DEV_DEF_RECV_CONT_BUF_LENGTH)
            {
                length = TM_DEV_DEF_RECV_CONT_BUF_LENGTH;
            }
            allocLength = TM_RECV_SCAT_MIN_INCR_BUF;
/* This should not be necessary */
            if (allocLength < length)
            {
                allocLength = length;
            }
            firstPacketPtr = tfGetSharedBuffer(TM_ETHER_IP_ALIGN,
                                               (ttPktLen)allocLength,
                                               TM_16BIT_ZERO);
            if ( firstPacketPtr == TM_PACKET_NULL_PTR)
            {
                errorCode = TM_ENOBUFS;
#if (defined(TM_SNMP_MIB) || defined(TM_PPP_LQM))
                devPtr->devInDiscards++;
#endif /* TM_SNMP_MIB or TM_PPP_LQM */
                goto recvInterfaceFinish;
            }
            else
            {
                tm_bcopy( dataPtr,
                          firstPacketPtr->pktLinkDataPtr,
                          (unsigned)length );
                firstPacketPtr->pktLinkDataLength = (ttPktLen)length;
                firstPacketPtr->pktChainDataLength = (ttPktLen)dataSize;
                savDataPtr = dataPtr;
                dataPtr += (unsigned)length;
                dataSize -= length;
            }
        }
    }
#endif /* TM_USE_DRV_SCAT_RECV && TM_NEED_ETHER_32BIT_ALIGNMENT */

/* Check to see if we are using the users buffer or our own */
    if (packetPtr == TM_PACKET_NULL_PTR)
    {
/* User owned buffer; create a packet/shared data area Zero Size*/
        newPacketPtr = tfGetSharedBuffer( 0,
                                          (ttPktLen)0,
                                          TM_BUF_USER_DEVICE_DATA );
        if ( newPacketPtr == TM_PACKET_NULL_PTR)
        {
            errorCode = TM_ENOBUFS;
#if (defined(TM_SNMP_MIB) || defined(TM_PPP_LQM))
#ifdef TM_USE_IPV4
            devPtr->dev4Ipv4Mib.ipIfStatsInDiscards++;
#ifdef TM_USE_NG_MIB2
            tm_context(tvDevIpv4Data).ipIfStatsInDiscards++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_USE_IPV4 */
#endif /* TM_SNMP_MIB or TM_PPP_LQM */
            goto recvInterfaceFinish;
        }
        else
        {
            packetPtr = newPacketPtr;
            packetPtr->pktSharedDataPtr->dataDeviceEntryPtr = devPtr;
            packetPtr->pktSharedDataPtr->dataBufFirstPtr = (tt8BitPtr)dataPtr;
            packetPtr->pktSharedDataPtr->dataBufLastPtr =
                ((tt8BitPtr)dataPtr)
                + tm_packed_byte_count((unsigned)dataSize);
#if defined(TM_USE_DRV_SCAT_RECV) && defined(TM_NEED_ETHER_32BIT_ALIGNMENT)
            if (firstPacketPtr != (ttPacketPtr)0)
            {
                packetPtr->pktSharedDataPtr->dataDeviceUserDataPtr = savDataPtr;
            }
            else
#endif /* TM_USE_DRV_SCAT_RECV && TM_NEED_ETHER_32BIT_ALIGNMENT */
            {
                packetPtr->pktSharedDataPtr->dataDeviceUserDataPtr = dataPtr;
            }
        }
    }
    else
    {
/* A Treck driver buffer */
        packetPtr->pktUserFlags = 0; /* User no longer owns the packet */
    }
/* In case the user has moved the beginning on us, or we allocated a header */
    packetPtr->pktLinkDataPtr = (tt8BitPtr)dataPtr;
    packetPtr->pktLinkDataLength = (ttPktLen)dataSize;
#if defined(TM_USE_DRV_SCAT_RECV) && defined(TM_NEED_ETHER_32BIT_ALIGNMENT)
    if (firstPacketPtr != (ttPacketPtr)0)
    {
        firstPacketPtr->pktLinkNextPtr = packetPtr;
        firstPacketPtr->pktLinkExtraCount = 1;
        packetPtr = firstPacketPtr;
    }
    else
#endif /* TM_USE_DRV_SCAT_RECV && TM_NEED_ETHER_32BIT_ALIGNMENT */
    {
        packetPtr->pktChainDataLength = (ttPktLen)dataSize;
    }
#ifdef TM_DEV_RECV_OFFLOAD
    if (devRecvOffload.devoRFlags != TM_16BIT_ZERO)
    {
        packetPtr->pktDevOffloadPtr = (ttVoidPtr)
                            &(packetPtr->pktSharedDataPtr->dataDevRecvOffload);
        tm_bcopy((ttVoidPtr)&devRecvOffload, packetPtr->pktDevOffloadPtr,
                  sizeof(devRecvOffload));
    }
#endif /* TM_DEV_RECV_OFFLOAD */
/* Call the drivers link layer recv routine */
    retCode = (*(devPtr->devLinkLayerProtocolPtr->lnkRecvFuncPtr))(
                                               (void TM_FAR *)interfaceHandle,
                                               (void TM_FAR *)packetPtr );


recvInterfaceFinish:
    if (errorCode != TM_ENOERROR)
/*
 * User set zero values for datasize, or dataPtr, or we could not
 * allocate a shared buffer to point to the user data
 */
    {
        if (dataPtr != (char TM_FAR *)0)
        {
            tfFreeDrvRecvBuffer(devPtr, packetPtr, dataPtr);
        }
#if defined(TM_USE_DRV_SCAT_RECV) && defined(TM_NEED_ETHER_32BIT_ALIGNMENT)
        if (firstPacketPtr != (ttPacketPtr)0)
        {
            tfFreeSharedBuffer(firstPacketPtr, TM_SOCKET_UNLOCKED);
        }
#endif /* TM_USE_DRV_SCAT_RECV && TM_NEED_ETHER_32BIT_ALIGNMENT */
        retCode = errorCode;
    }

    return retCode;
}


int tfDeviceClose (ttDeviceEntryPtr devPtr)
{
    ttDevXmitEntryPtr   devXmitPtr;
    ttPacketPtr         chainPacketPtr;
    ttLinkLayerEntryPtr linkLayerEntryPtr;
    int                 errorCode;
#ifndef TM_SINGLE_INTERFACE_HOME
    int                  i; /* could be negative */
#ifdef TM_USE_IPV4
    int                  maxIndex; /* could be negative */
#endif /* TM_USE_IPV4 */
#endif /* TM_SINGLE_INTERFACE_HOME */
#ifdef TM_MOBILE_IP4
    ttMobileVectPtr     mipVectPtr;
#endif /* TM_MOBILE_IP4 */

    errorCode = TM_ENOERROR;
/* If device is opened, or if we attempted to open it */
    if ( tm_16bit_one_bit_set( devPtr->devFlag, TM_DEV_OPENED ) )
    {
        linkLayerEntryPtr = devPtr->devLinkLayerProtocolPtr;
#ifdef TM_USE_IPV4
        if (    (linkLayerEntryPtr != TM_LINK_NULL_PTR)
             && (linkLayerEntryPtr->lnkResolveCleanFuncPtr !=
                        (ttLnkResolveCleanFuncPtr)0) )
        {
/* Remove all pending IPv4 ARP resolution entries */
            (void)(*(linkLayerEntryPtr->lnkResolveCleanFuncPtr))(
                                           (ttVoidPtr)devPtr,
                                           TM_IP_ZERO,
                                           (int)0);
        }
#ifdef TM_SINGLE_INTERFACE_HOME
        (void)tfRemoveInterface(devPtr, TM_16BIT_ZERO, PF_INET);
#else /* TM_SINGLE_INTERFACE_HOME */
        maxIndex = TM_MAX_IPS_PER_IF - 1;
        for ( i = maxIndex; i >= 0; i-- )
        {
/* Remove from routing table, if interface is configured  */
            (void)tfRemoveInterface(devPtr, (tt16Bit)i, PF_INET);
        }
#endif /* TM_SINGLE_INTERFACE_HOME */
#endif /* TM_USE_IPV4 */

#ifdef TM_USE_IPV6
        if (    (linkLayerEntryPtr != TM_LINK_NULL_PTR)
             && (linkLayerEntryPtr->lnk6ResolveCleanFuncPtr !=
                        (tt6LnkResolveCleanFuncPtr)0) )
        {
/* Remove all pending IPv6 ARP resolution entries */
            (void)(*(linkLayerEntryPtr->lnk6ResolveCleanFuncPtr))(
                                           (ttVoidPtr)devPtr,
                                           (tt6IpAddressPtr)0,
                                           (int)0);
        }
/* Remove all IPv6 addresses on this interface from the routing table */
        for ( i = (TM_6_MAX_MHOME - 1); i >= 0; i-- )
        {
            (void)tfRemoveInterface(devPtr, (tt16Bit)i, PF_INET6);
        }

#endif /* TM_USE_IPV6 */

#ifdef TM_MOBILE_IP4
/* Close Mobile IPv4 on this device */
        if (tm_mobile_ip4_enb(devPtr))
        {
            mipVectPtr = devPtr->devMobileIp4VectPtr;

            (*(mipVectPtr->mipConfigFuncPtr))
                    ( (ttUserInterface)devPtr,
                      TM_MIP_CLOSE );
        }
#endif /* TM_MOBILE_IP4 */

#ifdef TM_USE_IPV6
/* remove any auto-configured/discovered routing entries associated with the
   device. */
        tf6RtDeviceClose(devPtr);
        if (devPtr->devLinkLayerProtocolPtr->lnk6McastFuncPtr
                    != TM_6_LL_MCAST_FUNC_NULL_PTR)
        {
            (void)(*(devPtr->devLinkLayerProtocolPtr->lnk6McastFuncPtr))(
                        (ttVoidPtr)devPtr, &in6addr_linklocal_allnodes,
                        TM_LNK_DEL_MCAST);
        }
#endif /* TM_USE_IPV6 */
/* Close link layer */
        errorCode = (*(devPtr->devLinkLayerProtocolPtr->
                                lnkCloseFuncPtr))((void TM_FAR *)devPtr);
        if (errorCode == TM_ENOERROR)
        {

/*
 * Lock the device driver lock before calling device close. Also protect
 * devFlag (checked in tfIoctlInterface(), tfRecvInterface()).
 */
            tm_call_lock_wait(&(devPtr->devDriverLockEntry));
            if ( devPtr->devCloseFuncPtr != (ttDevOpenCloseFuncPtr)0 )
            {
                errorCode = (*(devPtr->devCloseFuncPtr))(
                                                (ttUserInterface)devPtr);
            }
/* Reset opened/connecting/connected flags + user flags */
            tm_16bit_clr_bit( devPtr->devFlag,
                              (  TM_DEV_OPENED
                               | TM_DEV_CONNECTING
                               | TM_DEV_CONNECTED
                               | TM_DEV_IP_BOOTP
                               | TM_DEV_IP_DHCP
                               | TM_DEV_USER_ALLOWED) );
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
#ifdef TM_USE_IPV4
            tm_kernel_set_critical;
            tm_context(tvIpData).ipv4InterfaceTableLastChange =
                tm_snmp_time_ticks(tvTime);
            tm_kernel_release_critical;
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
#ifdef TM_USE_IPV6
/* reset IPv6 user device flags, IPv6 link-layer device flags
   and IPv6 initial delay flag */
            tm_16bit_clr_bit( devPtr->dev6Flags,
                              (  TM_6_DEV_IP_FORW_ENB
                               | TM_6_DEV_IP_FORW_MCAST_ENB
                               | TM_6_DEV_MCAST_HW_LOOPBACK
                               | TM_6_DEV_OPTIMIZE_DAD
                               | TM_6_DEV_LL_ADDRESS_FLAG
                               | TM_6_USE_AUTO_IID
                               | TM_6_LL_CONNECTING
                               | TM_6_LL_CONNECTED
                               | TM_6_DEV_INIT_DELAY_COMPLETE) );
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
            tm_kernel_set_critical;
            tm_context(tvIpData).ipv6InterfaceTableLastChange =
                tm_snmp_time_ticks(tvTime);
            tm_kernel_release_critical;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */

/* reset dev6MhomeUsedAutoEntries because this is used to configure the
   initial link-local scope address on the interface */
            devPtr->dev6MhomeUsedAutoEntries =
                      TM_MAX_IPS_PER_IF;

#ifdef TM_6_USE_MIP_HA
            devPtr->dev6MhomeUsedProxyEntries =
                      TM_MAX_IPS_PER_IF
                    + TM_6_MAX_AUTOCONF_IPS_PER_IF
                    + TM_6_MAX_DHCP_IPS_PER_IA;
#endif /*TM_6_USE_MIP_HA */

#ifdef TM_6_USE_DHCP
            devPtr->dev6MhomeUsedDhcpEntries =
                      TM_MAX_IPS_PER_IF
                    + TM_6_MAX_AUTOCONF_IPS_PER_IF;

            tf6DhcpFreeDevMembers(devPtr);
#endif /* TM_6_USE_DHCP*/

#endif /* TM_USE_IPV6 */

#ifdef TM_SNMP_MIB
            tm_kernel_set_critical;
            devPtr->devLastChange = tvTime/10;
            tm_kernel_release_critical;
#endif /* TM_SNMP_MIB */
/* We will free queued send packets after unlocking. */
            chainPacketPtr = devPtr->devSendQueuePtr;
            devPtr->devSendQueueLastPtr = TM_PACKET_NULL_PTR;
            devPtr->devSendQueuePtr = TM_PACKET_NULL_PTR;
            devPtr->devXmitTaskPacketPtr = TM_PACKET_NULL_PTR;
            devXmitPtr = devPtr->devXmitEntryPtr;
            if (devXmitPtr != (ttDevXmitEntryPtr)0)
/* Re-initialize ring of transmit buffers */
            {
/* Read index in ring of buffers */
                devXmitPtr->xmitReadIndex = 0;
/* Write index in ring of buffers */
                devXmitPtr->xmitWriteIndex = 0;
/* No transmission error */
                devXmitPtr->xmitErrorFlag = TM_8BIT_ZERO;
/* No bytes queued or transmitted */
                devXmitPtr->xmitBytes = 0;
/* Ring of buffers initially empty */
                devXmitPtr->xmitFlag = TM_RING_EMPTY;
            }
/* Unlock device driver. Check if we need to call tfSendCompleteInterface() */
            tfDeviceUnlockSendComplete(devPtr);
            tfFreeChainPacket(chainPacketPtr, TM_SOCKET_UNLOCKED);

#ifdef TM_USE_IPV4
/* Clear the PPP Peer IPv4 address. */
            tm_ip_copy(TM_IP_ZERO, devPtr->devPt2PtPeerIpAddr);
#endif /* TM_USE_IPV4 */

        }
        devPtr->devErrorCode = TM_ENOERROR;
    }
    return errorCode;
}

/*
 * Prepare to Send a ttpacket to a device.
 * 1. Call the link layer send routine first to encapsulate the packet.
 *    Check for error return.
 * 2. If data is scattered, and device cannot support scattered data
 *    copy scattered data to one contiguous buffer. Free original scattered
 *    data. If cannot allocate a new buffer, error is set to TM_ENOBUFS.
 * 3. Lock the device driver lock, check on device being opened, otherwise
 *    error is set to TM_ENXIO.
 * 4. Queue the packet to the send queue.
 * 5. If user has enabled transmit task, initialize the task transmit packet
 *    pointer if not already set, then increase the transmit event count,
 *    unlock the device driver lock and post on the transmit task semaphore.
 * 6. otherwise, transmit task not enabled, call the postlink send functin,
 *    which will call the device driver send. For PPP the postlink send
 *    function is tfPppStuffPacket(), for SLIP it is tfSlipStuffPacket(), for
 *    all other link layers it is the common routine tfDeviceSendOnePacket()
 * 7. Unlock the device driver lock if not already done so in step 5.
 * 8. If an error occured, free the packet.
 * 9. Return error code.
 */
int tfDeviceSend ( ttPacketPtr packetPtr )
{
    tt8BitPtr           dataPtr;
    ttPacketPtr         lnkPacketPtr;
    ttPacketPtr         oldPacketPtr;
    ttDeviceEntryPtr    devPtr;
    ttLinkLayerEntryPtr devLinkLayerProtocolPtr;
    ttSharedDataPtr     oldSharedDataPtr;
    ttSharedDataPtr     sharedDataPtr;
    tt8BitPtr           startPtr;
#ifdef TM_USE_IPV6
    ttLnkPostFuncPtr    lnkPostFuncPtr;
#endif /* TM_USE_IPV6 */
    tt32Bit             dataLength;
#ifdef TM_ERROR_CHECKING
    tt32Bit             errorCheckDataSize;
#endif /* TM_ERROR_CHECKING */
    ttPktLen            headerSize;
    ttPktLen            packedHeaderSize;
#ifdef TM_DSP
    ttPktLen            extraLen;
    int                 destOffset;
#endif /* TM_DSP */
    int                 errorCode;
#if (defined(TM_SNMP_MIB) || defined(TM_PPP_LQM))
    tt16Bit             pktFlags;
#endif /* TM_SNMP_MIB or TM_PPP_LQM */
    tt16Bit             savXmitEventCount;
    tt16Bit             txAlign;
    tt8Bit              needUnlock;
    tt8Bit              needFreePacket;
    tt8Bit              needCopyPacket;
    tt8Bit              savXmitPending;

    devPtr = packetPtr->pktDeviceEntryPtr;
    devLinkLayerProtocolPtr = devPtr->devLinkLayerProtocolPtr;
#if (defined(TM_SNMP_MIB) || defined(TM_PPP_LQM))
/* Save pktFlags field for SNMP MIB variables update */
    pktFlags = packetPtr->pktFlags;
#endif /* TM_SNMP_MIB or TM_PPP_LQM */
/* Need to free the packet, if an error occurs before we send it */
    needFreePacket = TM_8BIT_YES;
#if (defined(TM_SNMP_MIB) || defined(TM_PPP_LQM))
#ifdef TM_USE_IPV4
    if (packetPtr->pktNetworkLayer == TM_NETWORK_IPV4_LAYER)
    {
        if ( tm_16bit_bits_not_set( pktFlags,
                                      TM_LL_BROADCAST_FLAG
                                    | TM_LL_MULTICAST_FLAG ) )
        {
            devPtr->devOutUcastPkts++;
        }
        else
        {
            if (tm_16bit_one_bit_set(pktFlags, TM_LL_BROADCAST_FLAG))
            {

#ifdef TM_USE_NG_MIB2
                tm_64Bit_incr(tm_context(tvDevIpv4Data).ipIfStatsHCOutBcastPkts);
#endif /* TM_USE_NG_MIB2 */
                tm_64Bit_incr(devPtr->dev4Ipv4Mib.ipIfStatsHCOutBcastPkts);
            }
#ifdef TM_USE_NG_MIB2
            else /* TT_LL_MULTICAST_FLAG */
            {

                tm_64Bit_incr(devPtr->dev4Ipv4Mib.ipIfStatsHCOutMcastPkts);
                tm_64Bit_incr(tm_context(tvDevIpv4Data).ipIfStatsHCOutMcastPkts);
                tm_64Bit_augm(devPtr->dev4Ipv4Mib.ipIfStatsHCOutMcastOctets,
                              packetPtr->pktChainDataLength);
                tm_64Bit_augm(tm_context(tvDevIpv4Data).ipIfStatsHCOutMcastOctets,
                              packetPtr->pktChainDataLength);
            }
#endif /* TM_USE_NG_MIB2 */
        }
    }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
    else
#endif /* TM_USE_IPV4 */
    if (packetPtr->pktNetworkLayer == TM_NETWORK_IPV6_LAYER)
    {
        if (tm_16bit_one_bit_set(pktFlags, TM_LL_MULTICAST_FLAG))
        {
#ifdef TM_USE_NG_MIB2
            tm_64Bit_incr(devPtr->dev6Ipv6Mib.ipIfStatsHCOutMcastPkts);
            tm_64Bit_incr(tm_context(tvDevIpv6Data).ipIfStatsHCOutMcastPkts);
            tm_64Bit_augm(devPtr->dev6Ipv6Mib.ipIfStatsHCOutMcastOctets,
                          packetPtr->pktChainDataLength);
            tm_64Bit_augm(tm_context(tvDevIpv6Data).ipIfStatsHCOutMcastOctets,
                          packetPtr->pktChainDataLength);
#endif /* TM_USE_NG_MIB2 */
        }
        else
        {
            devPtr->devOutUcastPkts++;
        }
    }
#endif /* TM_USE_IPV6 */
#endif /* TM_SNMP_MIB or TM_PPP_LQM */

#ifdef TM_BYPASS_ETHER_LL
    if ( tm_ll_is_enet(devPtr) )
    {
        errorCode = TM_ENOERROR;
    }
    else
#endif /* TM_BYPASS_ETHER_LL */
    {
/* An error at the link layer would cause us to free this packet */
        errorCode = (*(devLinkLayerProtocolPtr->lnkSendFuncPtr))(packetPtr);
    }
    if (errorCode == TM_ENOERROR)
    {
/* Check for aligment requirements */
        txAlign = devPtr->devTxAlign;
        if (    (txAlign > 2)
                && ( (   ((ttUserPtrCastToInt)(packetPtr->pktLinkDataPtr))
                         & ((ttUserPtrCastToInt)(txAlign - 1)) ) != 0 ) )
        {
/* We need to copy the packet to an aligned buffer */
            needCopyPacket = TM_8BIT_YES;
        }
        else
        {
            needCopyPacket = TM_8BIT_ZERO;
        }
#ifdef TM_DSP
/* if we do not do byte-stuffing in the PPP link-layer, then check to make
   sure that all buffers in the packet have pktLinkDataByteOffset set to 0 -
   if not, we need to copy the packet */
        if (!(tm_ll_is_byte_stuffing(devPtr)))
        {
            lnkPacketPtr = packetPtr;
            do
            {
                if (lnkPacketPtr->pktLinkDataByteOffset != 0)
                {
/* the device driver send function doesn't understand byte-offset within
   a word on the DSP - so, we need to copy the packet so byte-offset is 0 */
                    needCopyPacket = TM_8BIT_YES;
                    break;
                }
                lnkPacketPtr =
                    (ttPacketPtr)lnkPacketPtr->pktLinkNextPtr;
            }
            while (lnkPacketPtr != TM_PACKET_NULL_PTR);
        }
#endif /* TM_DSP */
/*
 * Check to see if we are contiguous, or that driver for non Pt2Pt link
 * layer supports scattered data (i.e TCP, or tfSocketScatteredSend())
 * with as many links as packet contains. All Pt2Pt devices have device
 * drivers that support scattered data.  Also, cannot send scattered data if
 * the user has configured a trailer, and there is trailing data in this
 * buffer - must copy into a new shared data buffer.
 */
        if (    (needCopyPacket == TM_8BIT_ZERO)
             && (    (    packetPtr->pktLinkDataLength
                       == packetPtr->pktChainDataLength )
                  || (    ( tm_16bit_one_bit_set( devPtr->devFlag,
                                                 TM_DEV_SCATTER_SEND_ENB) )
                      && (    packetPtr->pktLinkExtraCount
                           <= devPtr->devScatterExtraCount )
                      && (    (tm_context(tvPakPktSendTrailerSize) == 0)
                           || (tm_16bit_bits_not_set(packetPtr->pktFlags2,
                                                     TM_PF2_TRAILING_DATA))) )
                 || ( tm_ll_is_ppp(devPtr) ) ) )
        {
/*
 * Data alignment is as expected by the driver and either data is not
 * scattered, or device can send the scattered data
 */
            packetPtr->pktChainNextPtr = TM_PACKET_NULL_PTR;
        }
        else
        {
/*
 * Data is scattered, and exceeds device scatter count, or need to be aligned
 * at a certain boundary.
 */
#ifdef TM_ERROR_CHECKING
            if (    (packetPtr->pktLinkExtraCount == 0)
                 && (needCopyPacket == TM_8BIT_ZERO) )
            {
                tfKernelError("tfDeviceSend",
                "corrupted packet: 0 pktLinkExtraCount with scattered data!");
            }
#endif /* TM_ERROR_CHECKING */
/*
 * Device cannot handle scattered data, or data is not aligned at the correct
 * boundary. We must copy
 */
/* save the oldPacketPtr so that we can free it */
            oldPacketPtr = packetPtr;
            needCopyPacket = TM_8BIT_YES;
            if (    (oldPacketPtr->pktLinkExtraCount == 1)
/* If the user has configured a packet trailer, only try to prepend the TCP
   header if there is no trailing data in this buffer. */
                 && (    (tm_context(tvPakPktSendTrailerSize) == 0)
                      || (tm_16bit_bits_not_set(packetPtr->pktFlags2,
                                                TM_PF2_TRAILING_DATA)))
               )
            {
                packetPtr = (ttPacketPtr)oldPacketPtr->pktLinkNextPtr;
/* Check that it is safe to copy the header into the data area */
                if ( tm_16bit_one_bit_set(
                         packetPtr->pktFlags2, TM_PF2_NO_UNACK_DATA )
#ifdef TM_NO_COPY_OVER_USER_DATA
/* Check that the data area is not owned by the user */
                     && (!(  (packetPtr->pktSharedDataPtr->dataFlags)
                           & (TM_BUF_USER_DATA | TM_BUF_USER_DEVICE_DATA)))
#endif /* TM_NO_COPY_OVER_USER_DATA */
                     && !(tm_is_odd(
                              (ttUserPtrCastToInt)
                              (packetPtr->pktLinkDataPtr)) ) )
                {
/* Try and copy the TCP header at the beginning of the data packet */
                    headerSize = oldPacketPtr->pktLinkDataLength;
                    packedHeaderSize = headerSize;
/* For DSP pack the header size */
                    tm_pack_data_length( packedHeaderSize,
                                         oldPacketPtr,
                                         oldPacketPtr,
                                         devPtr );
                    if (    (unsigned)(packedHeaderSize)
                         <= ( (unsigned)
                              (  packetPtr->pktLinkDataPtr
                               - packetPtr->pktSharedDataPtr->dataBufFirstPtr)))
/* if there is room for it */
                    {
                        startPtr =
                            packetPtr->pktLinkDataPtr - packedHeaderSize;
                        if ( (txAlign <= 2)
                             || ( ( ((ttUserPtrCastToInt) startPtr)
                                    & ((ttUserPtrCastToInt)(txAlign - 1)))
                                  == 0) )
/*
 * Make sure we are aligned at devTxAlign if we need alignment other than at
 * 2 bytes boundary. Otherwise do not bother copying the header.
 */
                        {
                            oldPacketPtr->pktLinkNextPtr = (ttVoidPtr)0;
                            packetPtr->pktDeviceEntryPtr =
                                             oldPacketPtr->pktDeviceEntryPtr;
                            packetPtr->pktMhomeIndex =
                                             oldPacketPtr->pktMhomeIndex;
#if (defined(TM_USE_VCHAN) || defined(TM_USE_SNIFF))
                            packetPtr->pktNetworkLayer
                                = oldPacketPtr->pktNetworkLayer;
#endif /* (defined(TM_USE_VCHAN) || defined(TM_USE_SNIFF)) */
                            packetPtr->pktLinkDataLength += headerSize;
                            packetPtr->pktChainDataLength =
                                             packetPtr->pktLinkDataLength;
/* For DSP, update pointer, and bcopy using packed header size */
                            packetPtr->pktLinkDataPtr = startPtr;
                            tm_bcopy( oldPacketPtr->pktLinkDataPtr,
                                      packetPtr->pktLinkDataPtr,
                                      packedHeaderSize );
                            needCopyPacket = TM_8BIT_ZERO;
                        }
                    }
                }
            }
            if (needCopyPacket != TM_8BIT_ZERO)
            {
#ifdef TM_DSP
                if (tm_ll_has_ether(devPtr))
                {
                    extraLen = tm_byte_count(TM_ETHER_HEADER_SIZE) -
                        TM_ETHER_HEADER_SIZE;
                }
                else
                {
                    extraLen = 0;
                }
#endif /* TM_DSP */
                packetPtr = tfGetSharedBuffer(
                    TM_LL_MAX_HDR_LEN + txAlign,
                    oldPacketPtr->pktChainDataLength
#ifdef TM_DSP
                        + extraLen
#endif /* TM_DSP */
                    ,
                    TM_16BIT_ZERO );
                if (packetPtr == TM_PACKET_NULL_PTR)
                {
                    errorCode = TM_ENOBUFS;
/*
 * Packet will be freed at the end of the else statement (scattered data
 * exceeds the device scattered data count). No need to free at the end
 * of this routine.
 */
                    needFreePacket = TM_8BIT_ZERO;
#ifdef TM_SNMP_MIB
/* An error occured */
#ifdef TM_USE_IPV4
                    if (oldPacketPtr->pktNetworkLayer == TM_NETWORK_IPV4_LAYER)
                    {
#ifdef TM_USE_NG_MIB2
                        tm_context(tvDevIpv4Data).ipIfStatsOutDiscards++;
#endif /* TM_USE_NG_MIB2 */
                        devPtr->dev4Ipv4Mib.ipIfStatsOutDiscards++;
                    }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_NG_MIB2
#ifdef TM_USE_IPV4
                    else
#endif /* TM_USE_IPV4 */
                    if (oldPacketPtr->pktNetworkLayer == TM_NETWORK_IPV6_LAYER)
                    {
                        tm_context(tvDevIpv6Data).ipIfStatsOutDiscards++;
                        devPtr->dev6Ipv6Mib.ipIfStatsOutDiscards++;
                    }
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_USE_IPV6 */
#endif /* TM_SNMP_MIB */
                }
                else
                {
#ifdef TM_DSP
                    packetPtr->pktLinkDataLength -= extraLen;
                    packetPtr->pktChainDataLength -= extraLen;
#endif /* TM_DSP */
                    packetPtr->pktDeviceEntryPtr =
                                              oldPacketPtr->pktDeviceEntryPtr;
                    packetPtr->pktMhomeIndex = oldPacketPtr->pktMhomeIndex;
#if (defined(TM_USE_VCHAN) || defined(TM_USE_SNIFF) || defined(TM_SNMP_MIB))
                    packetPtr->pktNetworkLayer = oldPacketPtr->pktNetworkLayer;
#endif /* (defined(TM_USE_VCHAN) || defined(TM_USE_SNIFF) || defined(TM_SNMP_MIB)) */

/*
 * Copy dataSocketEntryPtr, dataFlags, and dataUserDataLength
 * for flow control with driver, and reset them in original packet.
 * This way the send queue will be updated only when we free the newly
 * allocated packet, not when we free the original packet in this
 * routine (tfSocketScatteredSend() only).
 */
                    oldSharedDataPtr = oldPacketPtr->pktSharedDataPtr;
                    if (oldSharedDataPtr->dataFlags & TM_BUF_USER_SEND)
                    {
                        sharedDataPtr = packetPtr->pktSharedDataPtr;
                        sharedDataPtr->dataFlags |= TM_BUF_USER_SEND;
                        sharedDataPtr->dataSocketEntryPtr =
                                        oldSharedDataPtr->dataSocketEntryPtr;
                        sharedDataPtr->dataUserDataLength =
                                        oldSharedDataPtr->dataUserDataLength;
                        tm_16bit_clr_bit( oldSharedDataPtr->dataFlags,
                                          TM_BUF_USER_SEND );
                        oldSharedDataPtr->dataSocketEntryPtr =
                                                         (ttSocketEntryPtr)0;
                    }
/* New start of data */
                    dataPtr = packetPtr->pktLinkDataPtr;
                    if (txAlign > 2)
                    {
                        if ((((ttUserPtrCastToInt) dataPtr)
                             & ((ttUserPtrCastToInt)(txAlign - 1))) != 0)
/* Make sure it is aligned if needed */
                        {
                            dataPtr = (tt8BitPtr)
                                (((ttUserPtrCastToInt) dataPtr)
                                 & ~((ttUserPtrCastToInt)(txAlign - 1)));
                            packetPtr->pktLinkDataPtr = dataPtr;
                        }
                    }
/* Copy the data */
                    lnkPacketPtr = oldPacketPtr;
#ifdef TM_ERROR_CHECKING
                    errorCheckDataSize = TM_UL(0);
#endif /* TM_ERROR_CHECKING */
#ifdef TM_DSP
                    destOffset = 0;
#endif /* TM_DSP */
                    do
                    {
                        dataLength = lnkPacketPtr->pktLinkDataLength;
#ifdef TM_ERROR_CHECKING
                        errorCheckDataSize += dataLength;
                        if (errorCheckDataSize > packetPtr->pktLinkDataLength)
/* Data is corrupted */
                        {
/* Newly allocated buffer will be freed at the end of this routine */
                            errorCode = TM_EFAULT;
                            tfKernelError("tfDeviceSend",
                                          "Copy of data is too long");
#ifdef TM_SNMP_MIB
/* An error occured */
#ifdef TM_USE_IPV4
                            if (packetPtr->pktNetworkLayer ==
                                        TM_NETWORK_IPV4_LAYER)
                            {
#ifdef TM_USE_NG_MIB2
                                tm_context(tvDevIpv4Data).
                                    ipIfStatsOutDiscards++;
#endif /* TM_USE_NG_MIB2 */
                                devPtr->dev4Ipv4Mib.
                                    ipIfStatsOutDiscards++;
                            }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_NG_MIB2
#ifdef TM_USE_IPV4
                            else
#endif /* TM_USE_IPV4 */
                            if (packetPtr->pktNetworkLayer ==
                                    TM_NETWORK_IPV6_LAYER)
                            {
                                tm_context(tvDevIpv6Data).
                                    ipIfStatsOutDiscards++;
                                devPtr->dev6Ipv6Mib.
                                    ipIfStatsOutDiscards++;
                            }
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_USE_IPV6 */
#endif /* TM_SNMP_MIB */
                            break;
                        }
#endif /* TM_ERROR_CHECKING */
#ifdef TM_DSP
/* Special DSP case to copy data into contiguous buffer: */
                        if ( (lnkPacketPtr == oldPacketPtr) &&
                             (tm_ll_has_ether(devPtr)) )
                        {
/* In the first ttPacket buffer in the chain, Ethernet header is stored as
   unpacked bytes */
                            dataLength -= TM_ETHER_HEADER_SIZE;
                            dataLength += tm_byte_count(TM_ETHER_HEADER_SIZE);
                        }

/* Special 8-bit byte copy for DSP: note that the source buffer of the copy is
   not guaranteed to be word-aligned (because of TCP un-acked data) */
                        tfMemCopyOffset(
                            (int *) lnkPacketPtr->pktLinkDataPtr,
                            lnkPacketPtr->pktLinkDataByteOffset,
                            (int *) dataPtr, destOffset,
                            dataLength );

/* Update word pointer and byte offset within that word for where we copy to
   next time */
                        destOffset = dataLength + destOffset;
                        dataPtr += tm_packed_byte_len((unsigned)destOffset);
                        destOffset = (destOffset) % TM_DSP_BYTES_PER_WORD;
#else /* !TM_DSP */
/* The normal (non-DSP) case for copying data into a contiguous buffer */
                        tm_bcopy( lnkPacketPtr->pktLinkDataPtr,
                                  dataPtr,
                                  dataLength );

                        dataPtr += (unsigned)dataLength;
#endif /* TM_DSP */

                        lnkPacketPtr =
                                    (ttPacketPtr)lnkPacketPtr->pktLinkNextPtr;
                    }
                    while (lnkPacketPtr != TM_PACKET_NULL_PTR);
                }
            }
#ifdef TM_DEV_SEND_OFFLOAD
            if (    (oldPacketPtr->pktDevOffloadPtr != (ttVoidPtr)0)
                 && (packetPtr != (ttPacketPtr)0) )
            {
                packetPtr->pktDevOffloadPtr = (ttVoidPtr)
                              &packetPtr->pktSharedDataPtr->dataDevSendOffload;
                tm_bcopy( oldPacketPtr->pktDevOffloadPtr,
                          packetPtr->pktDevOffloadPtr,
                          sizeof(ttDevSendOffloadStruct) );
            }
#endif /* TM_DEV_SEND_OFFLOAD */
/* Free user passed packet. */
            tfFreePacket(oldPacketPtr, TM_SOCKET_UNLOCKED);
        }
    }
#ifdef TM_SNMP_MIB
    else
    {
/* An error occured */
#ifdef TM_USE_IPV4
        if (packetPtr->pktNetworkLayer == TM_NETWORK_IPV4_LAYER)
        {
#ifdef TM_USE_NG_MIB2
            tm_context(tvDevIpv4Data).ipIfStatsOutDiscards++;
#endif /* TM_USE_NG_MIB2 */
            devPtr->dev4Ipv4Mib.ipIfStatsOutDiscards++;
        }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_NG_MIB2
#ifdef TM_USE_IPV4
        else
#endif /* TM_USE_IPV4 */
        if (packetPtr->pktNetworkLayer == TM_NETWORK_IPV6_LAYER)
        {
            tm_context(tvDevIpv6Data).ipIfStatsOutDiscards++;
            devPtr->dev6Ipv6Mib.ipIfStatsOutDiscards++;
        }
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_USE_IPV6 */
    }
#endif /* TM_SNMP_MIB */
/* Lock the send to provide single threaded access to the send */
    tm_lock_wait(&(devPtr->devDriverLockEntry));
    needUnlock = TM_8BIT_YES;
    if (errorCode == TM_ENOERROR)
    {
        if ( tm_16bit_one_bit_set( devPtr->devFlag, TM_DEV_OPENED) )
        {
#ifdef TM_SNMP_MIB
#ifdef TM_USE_IPV4
            if (packetPtr->pktNetworkLayer == TM_NETWORK_IPV4_LAYER)
            {
#ifdef TM_USE_NG_MIB2
                tm_64Bit_incr(devPtr->dev4Ipv4Mib.ipIfStatsHCOutTransmits);
                tm_64Bit_incr(tm_context(tvDevIpv4Data).ipIfStatsHCOutTransmits);
                tm_64Bit_augm(tm_context(tvDevIpv4Data).ipIfStatsHCOutOctets,
                              packetPtr->pktChainDataLength);
#endif /* TM_USE_NG_MIB2 */
                tm_64Bit_augm(devPtr->dev4Ipv4Mib.ipIfStatsHCOutOctets,
                              packetPtr->pktChainDataLength);
            }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_IPV4
            else
#endif /* TM_USE_IPV4 */
            if (packetPtr->pktNetworkLayer == TM_NETWORK_IPV6_LAYER)
            {
#ifdef TM_USE_NG_MIB2
                tm_64Bit_incr(devPtr->dev6Ipv6Mib.ipIfStatsHCOutTransmits);
                tm_64Bit_incr(tm_context(tvDevIpv6Data).ipIfStatsHCOutTransmits);
                tm_64Bit_augm(tm_context(tvDevIpv6Data).ipIfStatsHCOutOctets,
                              packetPtr->pktChainDataLength);
#endif /* TM_USE_NG_MIB2 */
                tm_64Bit_augm(devPtr->dev6Ipv6Mib.ipIfStatsHCOutOctets,
                              packetPtr->pktChainDataLength);
            }
#endif /* TM_USE_IPV6 */
#endif /* TM_SNMP_MIB */
            needFreePacket = TM_8BIT_ZERO;
#ifdef TM_USE_IPV6
            if (tm_6_ll_is_tunnel(devPtr))
            {
/* Save function before unlocking */
                lnkPostFuncPtr = devLinkLayerProtocolPtr->lnkPostFuncPtr;
/* Unlock the send/receive */
                tm_unlock(&(devPtr->devDriverLockEntry));
/* Tunnelling through another device. Do not use the device send queue */
                errorCode = (*(lnkPostFuncPtr))(packetPtr);
/* Lock the send/receive to provide single threaded access */
                tm_lock_wait(&(devPtr->devDriverLockEntry));
            }
            else
#endif /* TM_USE_IPV6 */
            {
/*
 * We are going to queue the packet to the send queue, and give the packet to
 * the driver send routine.
 */
#ifdef TM_SNMP_MIB
                devPtr->devOutQLen++;
#endif /* TM_SNMP_MIB */
                if (    devPtr->devSendQueueLastPtr
                     == TM_PACKET_NULL_PTR )
                {
/* No entries in the queue so make this packet the first and last entry */
                devPtr->devSendQueueLastPtr = packetPtr;
                devPtr->devSendQueuePtr = packetPtr;
                }
                else
                {
/* Other packets so chain to the end of the list */
/* Queue our packet to the end of the chain */
                    devPtr->devSendQueueLastPtr->pktChainNextPtr =
                                                                    packetPtr;
/* point to last queued packet */
                    devPtr->devSendQueueLastPtr = packetPtr;
                }
/*
 * Check to see if we have a Transmit task interface to the driver.
 * Option need to have been enabled successfully by the user.
 */
                if ( tm_16bit_one_bit_set( devPtr->devFlag,
                                           TM_DEV_XMIT_TASK) )
                {
                    if (devPtr->devXmitTaskPacketPtr == (ttPacketPtr)0)
/* Empty transmit queue */
                    {
                        devPtr->devXmitTaskPacketPtr = packetPtr;
                    }
/*
 * Use critical sections because tfCheckXmitInterface(), and
 * tfWaitXmitInterface use critical sections to protect this field instead
 * of the device driver lock entry.
 */
                    tm_kernel_set_critical;
                    savXmitEventCount = devPtr->devXmitEvent.eventCount;
                    devPtr->devXmitEvent.eventCount++;
                    savXmitPending = devPtr->devXmitEvent.eventPending;
                    tm_kernel_release_critical;
/*
 * Try and post only if the transmit task is pending/about to pend.
 */
                    if (    (savXmitPending != TM_8BIT_NO)
                         && (savXmitEventCount == (tt16Bit)0) )
                    {
/* Unlock the device driver lock before posting */
                        tm_unlock(&(devPtr->devDriverLockEntry));
/*
 * Only really posting if TM_TASK_XMIT is defined, and either
 * TM_TRECK_PREEMPTIVE_KERNEL, or TM_TRECK_NONPREMPTIVE_KERNEL is defined
 */
                        tm_kernel_post_xmit_event(
                                     &(devPtr->devXmitEvent.eventUnion));
/* No need to unlock the device driver lock at the end of this routine */
                        needUnlock = TM_8BIT_ZERO;
                    }
                }
                else
                {
/*
 * Call the post link layer routine, i.e either:
 * tfPppStuffPacket() for a PPP link layer, or
 * tfSlipStuffPacket() for a SLIP link layer, or
 * tfDeviceSendOnePacket() for any other link layer.
 * For each packet link for Ethernet, or for each send buffer for PPP, or
 * SLIP, then the post link layer routines call either the device
 * driver send function directly, if no device transmit queue is used,
 * or it calls tfDeviceSendBuffer() otherwise.
 * tfDeviceSendBuffer() calls the driver send function for the current
 * buffer only if the transmit queue is empty, otherwise it queues the
 * buffer to the device transmit queue.
 * If no device transmit queue is used, the post link layer function will
 * return the device driver error, if any, only if returned when the
 * last packet buffer was sent.
 * If a device transmit queue is used,
 * then the post link layer function will return the error as returned
 * from tfDeviceSendBuffer(), i.e TM_ENOBUFS, if no part of the packet
 * could be sent or queued, or TM_EIO if only part of the packet
 * (including at least the first piece, and the last piece) could be sent
 * or queued.
 */

/* write the packet into the pcap file*/
#ifdef TM_USE_SNIFF
#if (!defined(TM_KERNEL_WIN32_X86) || defined(TM_6_USE_IP_FORWARD) || \
        (defined(TM_USE_IPV4) && !defined(TM_SINGLE_INTERFACE_HOME)))
/* When TM_KERNEL_WIN32_X86 is defined and we are not forwarding, we always
   receive a copy of what we send, to avoid duplicated sniffing, we don't
   log */
                    if(packetPtr != TM_PACKET_NULL_PTR)
                    {
                        tfPcapWritePacket(packetPtr);
                    }
#endif /* !TM_KERNEL_WIN32_X86 or TM_6_USE_IP_FORWARD or ... */
#endif /* TM_USE_SNIFF */

                    devPtr->devSendQueueCurPtr = packetPtr;
#ifdef TM_USE_DRV_ONE_SCAT_SEND
                    if (devPtr->devOneScatSendFuncPtr
                                    != (ttDevOneScatSendFuncPtr)0)
                    {
/*
 * Bypass tfDeviceSendOnePacket, and call the device driver send function
 * only once with the scattered send buffer. Only supported on non Pt2Pt
 * interfaces.
 */
                        errorCode = (*(devPtr->devOneScatSendFuncPtr))(
                                            (ttUserInterface)devPtr,
                                            (ttUserPacketPtr)packetPtr );
                    }
                    else
#endif /* TM_USE_DRV_ONE_SCAT_SEND */
                    {
#ifndef TM_LOOP_TO_DRIVER
                        devPtr->devOrigDevPtr = packetPtr->pktOrigDevPtr;
#endif /* !TM_LOOP_TO_DRIVER */
                        errorCode =
                            (*(devLinkLayerProtocolPtr->lnkPostFuncPtr))(
                                                                   packetPtr);
                    }
                    if (errorCode != TM_ENOERROR)
                    {
/*
 * If no device transmit queue was used, and device driver send failed on
 * TM_USER_BUFFER_LAST, or, if a transmit queue was used, and we could not
 * queue any part of the packet to the device transmit queue (because it
 * was full) (errorCode == TM_ENOBUFS), then tfDeviceSendFailed() will
 * remove the packet from the send queue, and indicate that we need
 * to free the packet, by setting needFreePacket.
 * In all cases, tfDeviceSendFailed() will upate the SNMP MIB variables
 * errors.
 */
                        errorCode = tfDeviceSendFailed( devPtr,
                                                        packetPtr,
                                                        errorCode,
                                                        &needFreePacket);
                    }
                }
            }
        }
        else
        {
/* Device is closed. Can no longer send */
            errorCode = TM_ENXIO;
#ifdef TM_SNMP_MIB
#ifdef TM_USE_IPV4
            if (packetPtr->pktNetworkLayer == TM_NETWORK_IPV4_LAYER)
            {
#ifdef TM_USE_NG_MIB2
                tm_context(tvDevIpv4Data).ipIfStatsOutDiscards++;
#endif /* TM_USE_NG_MIB2 */
                devPtr->dev4Ipv4Mib.ipIfStatsOutDiscards++;
            }
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
#ifdef TM_USE_NG_MIB2
#ifdef TM_USE_IPV4
            else
#endif /* TM_USE_IPV4 */
            if (packetPtr->pktNetworkLayer == TM_NETWORK_IPV6_LAYER)
            {
                tm_context(tvDevIpv6Data).ipIfStatsOutDiscards++;
                devPtr->dev6Ipv6Mib.ipIfStatsOutDiscards++;
            }
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_USE_IPV6 */
#endif /* TM_SNMP_MIB */
        }
    }
    if (needUnlock != TM_8BIT_ZERO)
    {
/* Unlock the send */
/*
 * Unlock device driver. Check if we need to call tfSendCompleteInterface()
 * or tfSendCompletePacketsInterfaceUnlock().
 */
        if (    (devPtr->devSendCompltCount == 0)
#ifdef TM_USE_DRV_ONE_SCAT_SEND
             && (devPtr->devSendCompltPacketCount == 0)
#endif /* TM_USE_DRV_ONE_SCAT_SEND */
           )
        {
            tm_unlock(&(devPtr->devDriverLockEntry));
        }
        else
        {
/* Unlock device driver, call tfSendCompleteInterface() */
            tfDeviceUnlockSendComplete(devPtr);
        }
    }
#ifdef TM_LINT
LINT_BOOLEAN_BEGIN
#endif /* TM_LINT */

    if (needFreePacket != TM_8BIT_ZERO)
#ifdef TM_LINT
LINT_BOOLEAN_END
#endif /* TM_LINT */

/*
 * 1. Error occured before we could call the device send function or
 * kick the transmit task, i.e:
 * link layer send function returned an error, or device is closed, or we
 * could not allocate a buffer to copy the data when scattered data could
 * not be sent, or data was corrupted in packet.
 * 2. Device driver send function returned an error.
 */
    {
        tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
    }
    return errorCode;
}

/*
 * Send one packet to the device. This is the link post send function
 * used by Ethernet, Loop back, or Transparent link layer. Packet has
 * already been queued by tfDeviceSend().
 * Function called from tfDeviceSend() if no Treck transmit task is used,
 * or from tfXmitInterface() if a Treck transmit task is used.
 */
int tfDeviceSendOnePacket (ttPacketPtr packetPtr)
{
    ttPacketPtr         oldPacketPtr;
    ttDevSendFuncPtr    devSendFuncPtr;
    ttDeviceEntryPtr    devPtr;
#ifdef TM_ERROR_CHECKING
    tt32Bit             errorCheckDataSize;
#endif /* TM_ERROR_CHECKING */
    int                 errorCode;

    devPtr = packetPtr->pktDeviceEntryPtr;
    devSendFuncPtr = devPtr->devSendFuncPtr;
#ifdef TM_ERROR_CHECKING
    errorCheckDataSize = packetPtr->pktChainDataLength;
#endif /* TM_ERROR_CHECKING */
    if (packetPtr->pktLinkDataLength != packetPtr->pktChainDataLength)
    {
        while ( packetPtr->pktLinkNextPtr != (ttVoidPtr)0 )
        {
#ifdef TM_ERROR_CHECKING
            if ( errorCheckDataSize < packetPtr->pktLinkDataLength )
            {
                tfKernelError("tfDeviceSend", "Send Too Much Scattered Data");
            }
            errorCheckDataSize -= packetPtr->pktLinkDataLength;
#endif /* TM_ERROR_CHECKING */
            oldPacketPtr = (ttPacketPtr)packetPtr->pktLinkNextPtr;
            (void)(*(devSendFuncPtr))(
                (ttUserInterface)devPtr,
                (char TM_FAR *)(packetPtr->pktLinkDataPtr),
                (int)(packetPtr->pktLinkDataLength),
                TM_USER_BUFFER_MORE);
            packetPtr = oldPacketPtr;
        }
    }
/*
 * If a device transmit queue is used, (*(devSendFuncPtr)) will really be
 * tfDeviceSendBuffer(). tfDeviceSendBuffer() will return TM_EIO
 * if the buffer could not be either sent to the driver or stored in
 * the interface transmit queue, because the transmit
 * queue was full, and could not be emptied. Note that in that case
 * no part of the packet could have been given to the driver, and the entire
 * packet has been removed from the send queue by (*devSendFuncPtr)() (i.e
 * tfDeviceSendBuffer()).
 */
    errorCode = (*(devSendFuncPtr))(
                                   (ttUserInterface)devPtr,
                                   (char TM_FAR *)(packetPtr->pktLinkDataPtr),
                                   (int)(packetPtr->pktLinkDataLength),
                                   TM_USER_BUFFER_LAST);
    return errorCode;
}

/*
 * This routine is used to call any other driver functions
 * (i.e. refresh receive pools)
 */
int tfIoctlInterface(ttUserInterface interfaceHandle, int flag,
                     void TM_FAR * optionPtr, int optionLen)
{
    ttDeviceEntryPtr     devPtr;
#if defined(TM_DEV_SEND_OFFLOAD) || defined(TM_DEV_RECV_OFFLOAD)
    ttDevIoctlOffloadPtr devIoctlOffloadPtr;
#endif /* defined(TM_DEV_SEND_OFFLOAD) || defined(TM_DEV_RECV_OFFLOAD) */
    int                  retCode;

    devPtr = (ttDeviceEntryPtr)interfaceHandle;
/* Lock the driver to provide single threaded access to the driver */
    tm_call_lock_wait(&(devPtr->devDriverLockEntry));
    if ( tm_16bit_one_bit_set( devPtr->devFlag, TM_DEV_OPENED) )
    {
        retCode = TM_ENOENT; /* Assume failure */
        if (flag & TM_DEV_IOCTL_EMPTY_XMIT_FLAG)
        {
            flag &= ~TM_DEV_IOCTL_EMPTY_XMIT_FLAG;
            if (devPtr->devEmptyXmitFuncPtr != (ttDevEmptyXmitFuncPtr)0)
            {
                retCode = (*(devPtr->devEmptyXmitFuncPtr))(
                                                            interfaceHandle);
            }
        }
        if (flag & TM_DEV_IOCTL_REFILL_POOL_FLAG)
        {
            flag &= ~TM_DEV_IOCTL_REFILL_POOL_FLAG;
            if (devPtr->devRefillPoolFuncPtr != (ttDevRefillPoolFuncPtr)0)
            {
                retCode = (*(devPtr->devRefillPoolFuncPtr))(
                                                            interfaceHandle);
            }
        }
        if (flag != 0)
        {
            if ( devPtr->devIoctlFuncPtr != (ttDevIoctlFuncPtr)0 )
            {
#if defined(TM_DEV_SEND_OFFLOAD) || defined(TM_DEV_RECV_OFFLOAD)
                if (   (   flag
                         & (   TM_DEV_IOCTL_OFFLOAD_SET
                             | TM_DEV_IOCTL_OFFLOAD_GET) )
                     && (optionLen != sizeof(ttDevIoctlOffloadStruct)) )
                {
                    retCode = TM_EINVAL;
                }
                else
#endif /* defined(TM_DEV_SEND_OFFLOAD) || defined(TM_DEV_RECV_OFFLOAD) */
                {
#if defined(TM_DEV_SEND_OFFLOAD) || defined(TM_DEV_RECV_OFFLOAD)
                    if (flag & TM_DEV_IOCTL_OFFLOAD_GET)
                    {
                        tm_bzero(optionPtr, sizeof(ttDevIoctlOffloadStruct));
                    }
#endif /* defined(TM_DEV_SEND_OFFLOAD) || defined(TM_DEV_RECV_OFFLOAD) */
                    retCode = (*(devPtr->devIoctlFuncPtr))(
                                interfaceHandle, flag, optionPtr, optionLen);
                }
#if defined(TM_DEV_SEND_OFFLOAD) || defined(TM_DEV_RECV_OFFLOAD)
                if (retCode == TM_ENOERROR)
                {
                    devIoctlOffloadPtr = (ttDevIoctlOffloadPtr)optionPtr;
                    if ( flag & (TM_DEV_IOCTL_OFFLOAD_SET) )
                    {
                        devPtr->devOffloadFlags =
                                devIoctlOffloadPtr->offloadFlags;
                    }
#ifdef TM_DEV_SEND_OFFLOAD
                    if ( flag & (TM_DEV_IOCTL_OFFLOAD_GET) )
                    {
                        devPtr->devOffloadMaxSize =
                             devIoctlOffloadPtr->offloadMaxSize;
                    }
#endif /* TM_DEV_SEND_OFFLOAD */
                }
#endif /* defined(TM_DEV_SEND_OFFLOAD) || defined(TM_DEV_RECV_OFFLOAD) */
            }
        }
    }
    else
    {
        retCode = TM_ENXIO;
    }
/* Unlock device driver. Check if we need to call tfSendCompleteInterface() */
    tfDeviceUnlockSendComplete(devPtr);
    return(retCode);
}

/*
 * tfValidInterface function description:
 *  Check that the Interface handle passed by the user is valid, i.e
 *  is in our list of interfaces.
 *
 * Parameter
 * devPtr           Interface hande passed by the user
 * Return
 * 0                success
 * TM_EINVAL        bad parameter
 */
int tfValidInterface (ttDeviceEntryPtr devPtr )
{
    int              errorCode;
    ttDeviceEntryPtr listdevPtr;

    errorCode = TM_EINVAL;
/* Lock the device list */
    tm_call_lock_wait(&tm_context(tvDeviceListLock));
    listdevPtr = tm_context(tvDeviceList);
#ifdef TM_SINGLE_INTERFACE_HOME
    if (    (listdevPtr != TM_DEV_NULL_PTR)
         && (listdevPtr == devPtr) )
    {
        errorCode = TM_ENOERROR;
    }
#else /* !TM_SINGLE_INTERFACE_HOME */
    while (listdevPtr != TM_DEV_NULL_PTR)
    {
        if (listdevPtr == devPtr)
        {
            errorCode = TM_ENOERROR;
            break;
        }
        listdevPtr = listdevPtr->devNextDeviceEntry;
    }
#endif /* !TM_SINGLE_INTERFACE_HOME */
    tm_call_unlock(&tm_context(tvDeviceListLock));
    return errorCode;
}

#ifdef TM_USE_IPV4
/*
 * tfValidConfigInterface function description:
 * Check that the Interface handle passed by the user is valid, i.e
 * is in our list of interfaces, and that the interface is configured
 * for the passed multihome index.
 *
 * Parameter
 * devPtr           Interface hande passed by the user
 * Return
 * 0                success
 * TM_EINVAL        bad parameter
 * TM_ENETDOWN      not configured
 */
int tfValidConfigInterface ( ttDeviceEntryPtr devPtr,
                             tt16Bit          mHomeIndex )
{
    int              errorCode;

    errorCode = tfValidInterface(devPtr);
    if (errorCode == TM_ENOERROR)
    {
        tm_call_lock_wait(&(devPtr->devLockEntry));
/* Check mHomeIndex parameter */
#ifdef TM_SINGLE_INTERFACE_HOME
        TM_UNREF_IN_ARG(mHomeIndex);
        if (tm_8bit_bits_not_set(
                            tm_ip_dev_conf_flag(devPtr, mHomeIndex),
                            TM_DEV_IP_CONFIG ) )
#else /* TM_SINGLE_INTERFACE_HOME */
        if (    (mHomeIndex >= devPtr->devMhomeUsedEntries)
             || (tm_8bit_bits_not_set(
                            tm_ip_dev_conf_flag(devPtr, mHomeIndex),
                            TM_DEV_IP_CONFIG ) ) )
#endif /* TM_SINGLE_INTERFACE_HOME */
        {
            errorCode = TM_ENETDOWN;
        }
        tm_call_unlock(&(devPtr->devLockEntry));
    }
    return errorCode;
}
#endif /* TM_USE_IPV4 */

/*
 * tfValidLinkLayer function description:
 *  Check that the link layer passed by the user is valid, i.e
 *  is in our list of link layers.
 *
 * Parameter
 * linkLayerHandle  Link layer handle passed by the user
 * Return
 * 0                success
 * TM_EINVAL        bad parameter
 */
int tfValidLinkLayer (ttLinkLayerEntryPtr linkLayerHandle)
{
    ttLinkLayerEntryPtr linkLayerEntryPtr;
    int                 errorCode;

    errorCode = TM_EINVAL;
    tm_call_lock_wait(&tm_context(tvLinkLayerListLock));
    linkLayerEntryPtr = tm_context(tvLinkLayerListPtr);
    while (linkLayerEntryPtr != TM_LINK_NULL_PTR)
    {
        if ((ttLinkLayerEntryPtr)linkLayerHandle == linkLayerEntryPtr)
        {
            errorCode = TM_ENOERROR;
            break;
        }
        linkLayerEntryPtr = linkLayerEntryPtr->lnkNextPtr;
    }
    tm_call_unlock(&tm_context(tvLinkLayerListLock));
    return errorCode;
}

#ifdef TM_USE_IPV4
/*
 * tfCheckSetNetmask function description
 * 1. Check that the netmask is contiguous.
 * 2. FOR LAN:
 *    . Check that Network mask is set for LAN. If not set (zero)
 *      we will silently use class mask.
 *    . Compute the  directed broadcast address, and store it in device.
 *    . If ipAddress matches the directed broadcast address, return error.
 * 3. FOR Pt2Pt
 *    . netMask should be all 1's, but we allow the user to set their own
 *      netMask (if different from 0).
 * 4. Store netmask (possibly modified) in device.
 *
 *    return errorCode.
 */
static int tfCheckSetNetmask( ttDeviceEntryPtr devPtr,
                              tt16Bit          mHomeIndex)
{
    tt4IpAddress     netMask;
    tt4IpAddress     ipAddress;
    tt4IpAddress     dBcastIpAddress;
    int              prefixLen;
    int              errorCode;

#ifdef TM_SINGLE_INTERFACE_HOME
    TM_UNREF_IN_ARG(mHomeIndex);
#endif /* TM_SINGLE_INTERFACE_HOME */
    errorCode = TM_ENOERROR;
    tm_ip_copy(tm_ip_dev_mask(devPtr, mHomeIndex), netMask);
    tm_ip_copy(tm_ip_dev_addr(devPtr, mHomeIndex), ipAddress);
    prefixLen = tf4NetmaskToPrefixLen(netMask);
    if (tm_ip_not_match(tvRt32ContiguousBitsPtr[prefixLen],
                        netMask))
/* Netmask should be contiguous */
    {
        errorCode = TM_EINVAL;
    }
    else if ( tm_4_ll_is_broadcast(devPtr) )
    {
        if (tm_ip_not_zero(ipAddress))
        {
            if (tm_ip_not_match(netMask, TM_IP_HOST_MASK))
            {
                if (tm_ip_zero(netMask))
/* If the user did not specify a netmask, silently change it to the class one */
                {
                    netMask = tm_ip_class_net_mask(ipAddress);
                    prefixLen = tf4NetmaskToPrefixLen(netMask);
                }
                tm_ip_copy(tm_ip_bld_d_broadcast(((tt4IpAddress)ipAddress),
                                                 ((tt4IpAddress)netMask)),
                           dBcastIpAddress);
                if (tm_ip_match(ipAddress, dBcastIpAddress))
                {
/* Reject configuration of directed broadcast address */
                    errorCode = TM_EADDRNOTAVAIL;
                }
                else
                {
/* Store directed broadcast address */
                    tm_ip_copy(dBcastIpAddress,
                               tm_ip_dev_dbroad_addr(devPtr,
                                                     mHomeIndex));
                }
            }
            else
            {
/* Netmask all ones. Store all ones for directed broadcast address */
                tm_ip_copy(TM_IP_ALL_ONES,
                           tm_ip_dev_dbroad_addr(devPtr, mHomeIndex));
            }
        }
    }
    else
/* PPP, SLIP, or loopback network mask is all ones */
    {
/* Allow the user to use a netmask different from all ones, if they wish to */
        if (tm_ip_zero(netMask))
        {
/* Unless the user did not specify a netMask */
            tm_ip_copy(TM_IP_HOST_MASK, netMask);
            prefixLen = (tt8Bit)32;
        }
    }
/* Update netmask */
    if (errorCode == TM_ENOERROR)
    {
        tm_ip_copy( (tt4IpAddress)netMask,
                    tm_ip_dev_mask(devPtr,mHomeIndex));
        tm_4_ip_dev_prefixLen(devPtr,mHomeIndex) = (tt8Bit)prefixLen;
    }
    return errorCode;
}

tt4IpAddress tfDeviceDestIpAddress(ttDeviceEntryPtr devPtr)
{
    tt4IpAddress destIpAddress;

/* Remote IP address for PT2PT connection is in slot 0. */
    tm_ip_copy(devPtr->devPt2PtPeerIpAddr, destIpAddress);
    if (tm_ip_zero(destIpAddress))
    {
/* If we don't know Pt2Pt other side's IP address, add one to our own */
        tm_ip_copy( tm_ip_dev_addr(devPtr, 0),
                    destIpAddress );
/*
 * Following broken in several statements so that they do not exceed
 * line size limit on some compilers.
 */
/* Host order */
        tm_ip_copy( ntohl(destIpAddress),
                    destIpAddress );
/* Add 1 to in in host order */
        tm_ip_copy( (destIpAddress + 1),
                    destIpAddress );
/* Back to network order */
        tm_ip_copy( htonl(destIpAddress),
                    destIpAddress );
    }
    return destIpAddress;
}
#endif /* TM_USE_IPV4 */

/*
 * Free a buffer either owned by the user (device driver buffer), or by
 * the Treck stack.
 * Called from tfRecvInterface().
 * Never called with the device driver lock on.
 */
static void tfFreeDrvRecvBuffer ( ttDeviceEntryPtr devPtr,
                                  ttPacketPtr      packetPtr,
                                  char    TM_FAR * dataPtr )
{
    if (    (packetPtr == (ttPacketPtr)0)
         && (devPtr->devFreeRecvFuncPtr != (ttDevFreeRecvFuncPtr)0) )
    {
/*
 * User owns the buffer. Free user buffer. Make the user device driver
 * free function re-entrant by setting the device driver lock
 */
        tm_lock_wait(&(devPtr->devDriverLockEntry));
        (void)(*(devPtr->devFreeRecvFuncPtr))(devPtr, dataPtr);
        tm_unlock(&(devPtr->devDriverLockEntry));
    }
    else
/* Treck owns the buffer (if any) */
    {
/* tfFreeDriverBuffer checks on zero pointer, and user ownership */
        (void)tfFreeDriverBuffer(packetPtr);
    }
}

/****************************************************************************
* FUNCTION: tfNgOpenInterface
*
* PURPOSE: This function is a replacement for tfOpenInterface to support IPv6.
*
* NEW PARAMETERS:
*   See tfNgConfigInterface
*
* RETURNS:
*   See tfNgConfigInterface
*
* NOTES:
*
****************************************************************************/
int tfNgOpenInterface(
    ttUserInterface interfaceHandle,
    const struct sockaddr_storage TM_FAR * ipAddrPtr,
    int prefixLen, int flags, int ipv6Flags, int buffersPerFrameCount,
    tt6DevAddrNotifyFuncPtr dev6AddrNotifyFuncPtr )
{
    return tfNgConfigInterface(
        interfaceHandle, ipAddrPtr, prefixLen, flags, ipv6Flags,
        buffersPerFrameCount, dev6AddrNotifyFuncPtr, (unsigned int) 0);
}

#ifdef TM_USE_IPV4
/*
 * Configure an interface with an IP address / IP netmask.
 * Can only be used for first IP address configuration on the interface.
 * For additional multi-home IP addresses, use tfConfigInterface().
 */
int tfOpenInterface ( ttUserInterface interfaceHandle,
                      ttUserIpAddress ipAddress,
                      ttUserIpAddress netMask,
                      int             flags,
                      int             buffersPerFrameCount )
{
    return tfConfigInterface(interfaceHandle,
                             ipAddress,
                             netMask,
                             flags,
                             buffersPerFrameCount,
                             (unsigned char)0);
}
#endif /* TM_USE_IPV4 */

/*
 * The send is completed (data is no longer being used by the driver).
 * Remove, and free the first packet in the device send queue.
 * Also, if the user is using either a transmit task, or a device driver
 * transmit ring, do some consitency checks, to make sure that we are not
 * freeing a packet that has not been transmitted yet.
 * VERY IMPORTANT:
 * . flag should be set to TM_DEV_SEND_COMPLETE_DRIVER if
 *   tfSendCompleteInterface is called from any device driver routine (
 *   in line send complete mechanism).
 * . flag should be set to TM_DEV_SEND_COMPLETE_APP if
 *   tfSendCompleteInterface is called from either a send task, or the main
 *   loop (tfWaitSentInterface(), or tfCheckSentInterface() mechanism).
 */
void tfSendCompleteInterface(ttUserInterface interfaceHandle,
                             int             devDriverLockFlag)
{
    ttPacketPtr       packetPtr;
    ttPacketPtr       nextPacketPtr;
    ttDeviceEntryPtr  devPtr;
#ifdef TM_USE_INDRT
    tt8Bit            processSendComplete;
#endif /* TM_USE_INDRT */

    devPtr = (ttDeviceEntryPtr)interfaceHandle;
    packetPtr = TM_PACKET_NULL_PTR;
#ifdef TM_USE_INDRT
    processSendComplete = TM_8BIT_YES;
    if (devPtr->devIndrtSendFuncPtr != (ttDevSendFuncPtr)0)
/* Device has been configured for INDRT, i.e tfUseIndrt() has been called. */
    {
        if (devDriverLockFlag & TM_DEV_SEND_COMPLETE_INDRT)
/* Called from INDRT (ISR) */
        {
/* Cannot process tfSendCompleteInterface at ISR level */
            processSendComplete = TM_8BIT_ZERO;
            if (devPtr->devOutstandingSends != 0)
            {
                devPtr->devOutstandingSends--;
#ifdef TM_ERROR_CHECKING
                if (devPtr->devIndrtPosSends == 0)
                {
                    tfKernelError("tfSendCompleteInterface",
                                  "No INDRT packet when there should be one!");
                }
#endif /* TM_ERROR_CHECKING */
                devPtr->devIndrtPosSends--;
                if (devPtr->devIndrtPosSends != 0)
                {
/*
 * Call from INDRT for a non INDRT packet, make sure tfSendCompleteInterface()
 * is called later.
 */
                    devPtr->devSendCompltCount++;
                }
/*
 * ELSE, INDRT packet, do nothing since INDRT packets are not really
 * queued to the device send queue.
 */
            }
#ifdef TM_ERROR_CHECKING
            else
            {
                tfKernelWarning("tfSendCompleteInterface",
                                "Too many calls\n");
            }
#endif /* TM_ERROR_CHECKING */
        }
        else
        {
/* Not called from INDRT. Assume we can process the send complete here. */
            if (     devDriverLockFlag
                 != (TM_DEV_SEND_COMPLETE_APP | TM_DEV_SEND_COMPLETE_DELAYED) )
/*
 * We have not been through this routine before, and have not updated the
 * INDRT variables yet.
 */
            {
                tm_kernel_set_critical;
                if (devPtr->devOutstandingSends != 0)
                {
                    devPtr->devOutstandingSends--;
                    if (devPtr->devIndrtPosSends != 0)
                    {
                        devPtr->devIndrtPosSends--;
                        if (devPtr->devIndrtPosSends == 0)
                        {
/*
 * Bypass tfSendCompleteInterface for an INDRT packet, since INDRT
 * packets are not really queued to the send quue
 */
                            processSendComplete = TM_8BIT_ZERO;
                        }
                    }
                    if (processSendComplete != TM_8BIT_ZERO)
/*
 * If we can process tfSendCompleteInterface(), and devSendCompltCount need
 * to be increased, do it here rather than below, since we need to
 * increase it in a critical section when INDRT is used.
 */
                    {
                        if (    (devDriverLockFlag != TM_DEV_SEND_COMPLETE_APP)
                             && (   devPtr->devFreeRecvFuncPtr
                                  != (ttDevFreeRecvFuncPtr)0) )
                        {
/* Increase devSendCompltCount in critical section, instead of below */
                            devPtr->devSendCompltCount++;
/* No need to increase devSendCompltCount below anymore */
                            processSendComplete = TM_8BIT_ZERO;
                        }
                    }
                }
                else
                {
#ifdef TM_ERROR_CHECKING
                    tfKernelWarning("tfSendCompleteInterface",
                                  "Spurious transmit done interrupt\n");
#endif /* TM_ERROR_CHECKING */
                    processSendComplete = TM_8BIT_ZERO;
                }
                tm_kernel_release_critical;
            }
            else
            {
/*
 * We have been through this routine before, and have already updated the
 * INDRT variables. Remove the SEND_COMPLETE_DELAYED flag. If set, it tells
 * us that the INDRT variables do not need to be updated, because they
 * had been updated before.
 */
                devDriverLockFlag = TM_DEV_SEND_COMPLETE_APP;
            }
        }
    }
    else
    {
/*
 * Remove the SEND_COMPLETE_DELAYED flag. Only needed if INDRT had been
 * configured for INDRT.
 */
        devDriverLockFlag &= ~TM_DEV_SEND_COMPLETE_DELAYED;
    }
    if (processSendComplete != TM_8BIT_ZERO)
#endif /* TM_USE_INDRT */
    {
        if (    (devDriverLockFlag == TM_DEV_SEND_COMPLETE_APP)
             || (devPtr->devFreeRecvFuncPtr == (ttDevFreeRecvFuncPtr)0) )
        {
            if (devDriverLockFlag == TM_DEV_SEND_COMPLETE_APP)
/*
 * If tfSendCompleteInterface is called from an application (i.e. send
 * complete task), and not from the driver, then the device driver is not
 * locked and we need to lock it.
 */
            {
                tm_lock_wait(&(devPtr->devDriverLockEntry));
            }
            if ( tm_16bit_one_bit_set( devPtr->devFlag, TM_DEV_OPENED) )
            {
                packetPtr = devPtr->devSendQueuePtr;
                if (packetPtr != TM_PACKET_NULL_PTR)
                {
#ifdef TM_ERROR_CHECKING
/*
 * We are checking that we are not freeing a packet that has not
 * been transmitted yet, i.e user calling tfSendCompleteInterface() too
 * many times, or calling tfSendCompleteInterface() for a packet that
 * it could not transmit.
 * We do a consistency check on the transmit task queue
 * if a transmit task is used. We cannot allow the user to free a
 * packet that has not been transmitted yet.
 */
/* Transmit task used, and transmit task queue non empty */
                    if (    (    devPtr->devXmitTaskPacketPtr
                              != TM_PACKET_NULL_PTR )
/* and Trying to free the packet at the head of the transmit task queue */
                         && ( packetPtr == devPtr->devXmitTaskPacketPtr) )
                    {
                        packetPtr = TM_PACKET_NULL_PTR;
                        tfKernelError("tfSendCompleteInterface",
                                      "freeing a packet that has" \
                                      "not been transmitted yet\n");
                    }
                    else
#endif /* TM_ERROR_CHECKING */
                    {
#ifdef TM_SNMP_MIB
                        devPtr->devOutQLen--;
#endif /* TM_SNMP_MIB */
/* Next packet in the device send queue */
                        nextPacketPtr = packetPtr->pktChainNextPtr;
/* If we have no more packets in the queue, then null the last packetPtr */
                        if (nextPacketPtr == TM_PACKET_NULL_PTR)
                        {
                            devPtr->devSendQueueLastPtr =
                                                            TM_PACKET_NULL_PTR;
                        }
/* Remove current packet from the device send queue */
                        devPtr->devSendQueuePtr = nextPacketPtr;
/*
 * Need to zero this field, just in case the send queue head pointer jumps
 * ahead of the transmit packet pointer. Could happen if user calls
 * tfSendCompleteInterface too many times.
 */
                        packetPtr->pktChainNextPtr = (ttPacketPtr)0;
/* Free the packet later with the lock off. */
                    }
                }
                if (    (devPtr->devEmptyXmitFuncPtr !=
                         (ttDevEmptyXmitFuncPtr)0)
                     && (!(devPtr->devFlag & TM_DEV_XMITING))
                     && tm_iface_xmit_queue_not_empty(
                                        devPtr->devXmitEntryPtr) )
/*
 * If there is a transmit queue, and we are not in the process of
 * transmitting, empty the ring of transmit buffers
 */
                {
                    (void)(*(devPtr->devEmptyXmitFuncPtr))
                        (interfaceHandle);
                }
            }
            if (devDriverLockFlag == TM_DEV_SEND_COMPLETE_APP)
            {
/* Unlock the device driver lock */
                tm_unlock(&(devPtr->devDriverLockEntry));
            }
            tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
        }
        else
        {
            devPtr->devSendCompltCount++;
        }
    }
}

#ifdef TM_USE_DRV_ONE_SCAT_SEND
/*
 * The send is completed (data is no longer being used by the driver).
 * Remove, and free the given packet in the device send queue.
 * VERY IMPORTANT:
 * . flag should be set to TM_DEV_SEND_COMPLETE_DRIVER if
 *    tfSendCompletePacketInterface is called from any device driver routine
 *    (in line send complete mechanism).
 * . flag should be set to TM_DEV_SEND_COMPLETE_APP if
 *    tfSendCompletePacketInterface is called from either a send task, or the
 *    main loop (tfWaitSentInterface(), or tfCheckSentInterface() mechanism).
 */
void tfSendCompletePacketInterface(ttUserInterface interfaceHandle,
                                   ttUserPacketPtr packetUPtr,
                                   int             devDriverLockFlag)
{
    ttPacketPtr       packetPtr;
    ttPacketPtr       sendQPacketPtr;
    ttPacketPtr       nxtQPacketPtr;
    ttPacketPtr       prevQPacketPtr;
    ttDeviceEntryPtr  devPtr;
    tt8Bit            found;

    devPtr = (ttDeviceEntryPtr)interfaceHandle;
    packetPtr = (ttPacketPtr)packetUPtr;
    found = TM_8BIT_ZERO;
/*
 * Function called from application, or from device driver but not with
 * a device driver recv buffer
 */
    if (    (devDriverLockFlag == TM_DEV_SEND_COMPLETE_APP)
         || (devPtr->devFreeRecvFuncPtr == (ttDevFreeRecvFuncPtr)0) )
    {
        if (devDriverLockFlag == TM_DEV_SEND_COMPLETE_APP)
/*
 * If tfSendCompleteInterface is called from an application (i.e. send
 * complete task), and not from the driver, then the device driver is not
 * locked and we need to lock it.
 */
        {
            tm_lock_wait(&(devPtr->devDriverLockEntry));
        }
        if ( tm_16bit_one_bit_set( devPtr->devFlag, TM_DEV_OPENED) )
        {
            sendQPacketPtr = devPtr->devSendQueuePtr;
            prevQPacketPtr = TM_PACKET_NULL_PTR;
            while (sendQPacketPtr != TM_PACKET_NULL_PTR)
            {
/* Next packet in the device send queue */
                nxtQPacketPtr = sendQPacketPtr->pktChainNextPtr;
                if (sendQPacketPtr == packetPtr)
                {
#ifdef TM_SNMP_MIB
                    devPtr->devOutQLen--;
#endif /* TM_SNMP_MIB */
/* update last packetPtr, if pointed at the packet we are removing */
                    if (devPtr->devSendQueueLastPtr == packetPtr)
                    {
                        devPtr->devSendQueueLastPtr = prevQPacketPtr;
                    }
/* Remove current packet from the device send queue */
                    if (prevQPacketPtr == TM_PACKET_NULL_PTR)
                    {
                        devPtr->devSendQueuePtr = nxtQPacketPtr;
                    }
                    else
                    {
                        prevQPacketPtr->pktChainNextPtr = nxtQPacketPtr;
                    }
                    packetPtr->pktChainNextPtr = (ttPacketPtr)0;
/* Free the packet later with the lock off. */
                    found = TM_8BIT_YES;
                    break;
                }
                prevQPacketPtr = sendQPacketPtr;
                sendQPacketPtr = nxtQPacketPtr;
            }
            if (    (devPtr->devEmptyXmitFuncPtr !=
                     (ttDevEmptyXmitFuncPtr)0)
                 && (!(devPtr->devFlag & TM_DEV_XMITING))
                 && tm_iface_xmit_queue_not_empty(
                                        devPtr->devXmitEntryPtr) )
/*
 * If there is a transmit queue, and we are not in the process of
 * transmitting, empty the ring of transmit buffers
 */
            {
                (void)(*(devPtr->devEmptyXmitFuncPtr))(interfaceHandle);
            }
        }
        if (devDriverLockFlag == TM_DEV_SEND_COMPLETE_APP)
        {
/* Unlock the device driver lock */
            tm_unlock(&(devPtr->devDriverLockEntry));
        }
        if (found)
        {
            tfFreePacket(packetPtr, TM_SOCKET_UNLOCKED);
        }
    }
    else
    {
/*
 * tfSendCompletePacketInterface() is called from the driver, and
 * the buffer is a device driver buffer (ping echo reply for example, with
 * a device driver using its own buffers).
 * We cannot free the packet here, othwerwise a deadlock could occur.
 * Postpone freeing the packet.
 */
        packetPtr->pktUserFlags |= TM_PKTF_USER_DEV_SEND_CMPLT;
        devPtr->devSendCompltPacketCount++;
    }
}

/*
 * tfSendCompletePacketsInterfaceUnlock function description
 * Called from tfDeviceUnlockSendComplete (from either tfIoctlInterface(),
 * tfDeviceClose(), or tfDeviceSend()) with the device driver lock on,
 * when send complete on one of more packets have been postponed. Remove, and
 * free all the packets with the TM_PTKF_USER_DEV_SEND_CMPLT flag set, up
 * to packetCount. Unlock the device driver lock.
 */
static void tfSendCompletePacketsInterfaceUnlock( ttDeviceEntryPtr devPtr,
                                                  int              packetCount )
{
    ttPacketPtr       packetPtr;
    ttPacketPtr       sendQPacketPtr;
    ttPacketPtr       nxtQPacketPtr;
    ttPacketPtr       prevQPacketPtr;

    packetPtr = TM_PACKET_NULL_PTR; /* packet to free */
    if ( tm_16bit_one_bit_set( devPtr->devFlag, TM_DEV_OPENED) )
    {
        sendQPacketPtr = devPtr->devSendQueuePtr;
        prevQPacketPtr = TM_PACKET_NULL_PTR;
        while ((sendQPacketPtr != TM_PACKET_NULL_PTR) && (packetCount != 0))
        {
/* Next packet in the device send queue */
            nxtQPacketPtr = sendQPacketPtr->pktChainNextPtr;
            if (sendQPacketPtr->pktUserFlags & TM_PKTF_USER_DEV_SEND_CMPLT)
            {
#ifdef TM_SNMP_MIB
                devPtr->devOutQLen--;
#endif /* TM_SNMP_MIB */
/* update last packetPtr, if pointed at the packet we are removing */
                if (devPtr->devSendQueueLastPtr == sendQPacketPtr)
                {
                    devPtr->devSendQueueLastPtr = prevQPacketPtr;
                }
/* Remove current packet from the device send queue */
                if (prevQPacketPtr == TM_PACKET_NULL_PTR)
                {
                    devPtr->devSendQueuePtr = nxtQPacketPtr;
                }
                else
                {
                    prevQPacketPtr->pktChainNextPtr = nxtQPacketPtr;
                }
/* Free the packet later with the lock off, chain in all the packets to free. */
                sendQPacketPtr->pktChainNextPtr = packetPtr;
                packetPtr = sendQPacketPtr;
                packetCount--;
            }
            else
            {
/*
 * Update prevQPacketPtr, if we did not remove the current packet from the
 * queue.
 */
                prevQPacketPtr = sendQPacketPtr;
            }
            sendQPacketPtr = nxtQPacketPtr;
        }
        if (    (devPtr->devEmptyXmitFuncPtr != (ttDevEmptyXmitFuncPtr)0)
             && (!(devPtr->devFlag & TM_DEV_XMITING))
             && tm_iface_xmit_queue_not_empty(
                                        devPtr->devXmitEntryPtr) )
/*
 * If there is a transmit queue, and we are not in the process of
 * transmitting, empty the ring of transmit buffers
 */
        {
            (void)(*(devPtr->devEmptyXmitFuncPtr))(
                                               (ttUserInterface)devPtr);
        }
    }
/* Unlock the device driver lock */
    tm_unlock(&(devPtr->devDriverLockEntry));
    tfFreeChainPacket(packetPtr, TM_SOCKET_UNLOCKED);
}
#endif /* TM_USE_DRV_ONE_SCAT_SEND */

/*
 * Check to see if data has been sent and is waiting to be
 * freed.
 * When tfCheckSentInterface returns 0, the user
 * can then call tfSendCompleteInterface() so that the stack
 * can free the buffer holding the data that has been sent.
 * Returns TM_ENOERROR    if there is data sent, and ready to be freed.
 *         TM_EWOULDBLOCK if There is no data sent
 */
int tfCheckSentInterface(ttUserInterface interfaceHandle)
{
    return tfCheckEventInterface(
                        &((ttDeviceEntryPtr)interfaceHandle)->devSentEvent);
}

/*
 * tfNotifyInterfaceIsr() function description
 * Function Called by driver ISR routine.
 * Deprecates tfNotifyReceiveInterfaceIsr() and tfNotifySentInterfaceIsr()
 * VERY IMPORTANT: should be only called once per ISR.
 *               Replaces both tfNotifyRecvInterfaceIsr(),
 *               and tfNotifySentInterfaceIsr() which are now both deprecated.
 * Notify the user with the number of packets received in one ISR,
 * and the number of packets that have been transmitted by the chip.
 * . If number of packets received is non zero:
 *   Will cause:
 *    tfCheckReceiveInterface to return 0 (polling check method),
 *   or
 *    tfWaitReceiveInterface (task pending check method) to return.
 * . If number of packets transmitted by the chip is non zero:
 *    Will cause:
 *     tfCheckSentInterface to return 0 (polling check method),
 *    or
 *     tfWaitSentInterface (task pending check method) to return.
 *    But only if number of bytes that have been transmitted reaches a
 *    configurable threshold tvNotifySendCompLowWater.
 * Parmaeters:
 * interfaceHandle           Interface handle as returned by tfAddInterface()
 * numberRecvPackets         Count of Number of packets received in the ISR.
 *                           A zero value indicates that the ISR did not
 *                           get any recv packet notification
 * numberSendCompletePackets Total number of send complete packets. A zero
 *                           value indicates that the ISR did not get any
 *                           send complete notification.
 * totalNumberBytesSent      Total number of bytes sent in the send complete
 *                           packets.
 * Return
 * No return
 */
void tfNotifyInterfaceIsr ( ttUserInterface interfaceHandle,
                            int             numberRecvPackets,
                            int             numberSendCompletePackets,
                            ttUser32Bit     numberBytesSent,
                            ttUser32Bit     flag )
{
    ttDeviceEntryPtr devPtr;
    ttEventEntryPtr  recvEventEntryPtr;
    ttEventEntryPtr  sentEventEntryPtr;
#ifdef TM_MULTIPLE_CONTEXT
    ttUserContext    currentContext;
#endif /* TM_MULTIPLE_CONTEXT */
    tt32Bit          devBytesNotified;
    tt32Bit          devPacketsNotified;
    tt16Bit          savRecvEventCount;
    tt16Bit          savSentEventCount;
#ifdef TM_USE_INDRT
    int              indrtSend;
#endif /* TM_USE_INDRT */

    devPtr = (ttDeviceEntryPtr)interfaceHandle;
#ifdef TM_MULTIPLE_CONTEXT
    currentContext = tfGetCurrentContext();
    tfSetCurrentContext(devPtr->devContextPtr);
#endif /* TM_MULTIPLE_CONTEXT */
#ifdef TM_USE_INDRT
    if (    (numberSendCompletePackets != 0)
         && (devPtr->devIndrtSendFuncPtr != (ttDevSendFuncPtr)0) )
    {
        indrtSend = tfIndrtSentIsr(interfaceHandle, numberSendCompletePackets);
        if (indrtSend)
        {
            numberSendCompletePackets--;
        }
    }
#endif /* TM_USE_INDRT */
    TM_UNREF_IN_ARG(flag); /* Unused parameter */
    recvEventEntryPtr = &(devPtr->devRecvEvent);
    sentEventEntryPtr = &(devPtr->devSentEvent);
/*
 * Save numbers of recv event counts. This tells us whether the recv task
 * has backed up work.
 */
    savRecvEventCount = recvEventEntryPtr->eventCount;
/*
 * Save numbers of sent event counts. This tells us wheter the send complete
 * task has backed up work.
 */
    savSentEventCount = sentEventEntryPtr->eventCount;
    if (numberRecvPackets != 0)
    {
        recvEventEntryPtr->eventCount = (tt16Bit)(
                                             recvEventEntryPtr->eventCount
                                           + (tt16Bit)numberRecvPackets );
    }
    devPacketsNotified = 0;
    if (numberSendCompletePackets != 0)
    {
        devBytesNotified = devPtr->devBytesNotified + numberBytesSent;
        devPacketsNotified =   devPtr->devPacketsNotified
                             + (tt32Bit)numberSendCompletePackets;
        if (devBytesNotified > tm_context(tvNotifySendCompLowWater))
        {
            sentEventEntryPtr->eventCount = (tt16Bit)(
                                             sentEventEntryPtr->eventCount
                                           + (tt16Bit)devPacketsNotified );
            devPtr->devBytesNotified = 0;
            devPtr->devPacketsNotified = 0;
        }
        else
        {
            devPtr->devPacketsNotified = devPacketsNotified;
            devPtr->devBytesNotified = devBytesNotified;
/* Postpone notification until we reach the threshold */
            devPacketsNotified = 0;
        }
    }
/*
 * Since the OS could re-enable the interrupt when we post from the ISR,
 * we postpone posting to the tasks up to the end of this routine, so that
 * all our variable updates are protected from another ISR call.
 */
/*
 * Try and post only if we are notified of a received packet, and the recv
 * task is pending /about to pend.
 */
    if (    (numberRecvPackets != 0)
         && (recvEventEntryPtr->eventPending != TM_8BIT_NO)
         && (savRecvEventCount == (tt16Bit)0) )
    {
/*
 * Only really posting if TM_TASK_RECV is defined, and either
 * TM_TRECK_PREEMPTIVE_KERNEL, or TM_TRECK_NONPREMPTIVE_KERNEL is defined
 */
        tm_kernel_isr_post_recv_event(&(recvEventEntryPtr->eventUnion));
    }
/*
 * Try and post only if we are notified of a sent packet (and we reached
 * the threshold), and the send complete task is pending/about to pend.
 */
    if (    (devPacketsNotified != 0)
         && (sentEventEntryPtr->eventPending != TM_8BIT_NO)
         && (savSentEventCount == (tt16Bit)0) )
    {
/*
 * Only really posting if TM_TASK_SEND is defined, and either
 * TM_TRECK_PREEMPTIVE_KERNEL, or TM_TRECK_NONPREMPTIVE_KERNEL is defined
 */
        tm_kernel_isr_post_sent_event(&(sentEventEntryPtr->eventUnion));
    }
#ifdef TM_MULTIPLE_CONTEXT
    tfSetCurrentContext(currentContext);
#endif /* TM_MULTIPLE_CONTEXT */
}

/*
 * Check to see if data is waiting to be received
 * When tfCheckReceiveInterface returns 0, the user
 * can then call tfRecvInterface() so
 * that the stack can retrieve the data ready to be received.
 * Returns TM_ENOERROR    if there is data to be received,
 *         TM_EWOULDBLOCK if There is no data to be received
 */
int tfCheckReceiveInterface(ttUserInterface interfaceHandle)
{
    return tfCheckEventInterface(
                        &((ttDeviceEntryPtr)interfaceHandle)->devRecvEvent);
}

/*
 * Common routine called by either tfCheckSentInterface() or
 * tfCheckRecvInterface(), or tfCheckXmitInterface().
 */
int tfCheckEventInterface(ttEventEntryPtr eventEntryPtr)
{
    int    status;

    tm_kernel_set_critical;
    if (eventEntryPtr->eventCount != (tt16Bit)0)
    {
        eventEntryPtr->eventCount--;
        tm_kernel_release_critical;
        status = TM_ENOERROR;
    }
    else
    {
        tm_kernel_release_critical;
        status = TM_EWOULDBLOCK;
    }
    return status;
}

/*
 * Wait for incoming data, if there is none, then wait
 * When tfWaitReceiveInterface() returns, the user can then call
 * tfRecvInterface() so that the stack can retrieve the data ready to be
 * received.
 * VERY IMPORTANT:
 * Can only be used when there is a separate receive task, and TM_TASK_RECV
 * is defined in trsystem.h.
 */
#ifdef TM_RECV_PEND_ISR_POST_NEEDED
int tfWaitReceiveInterface(ttUserInterface interfaceHandle)
{
    return tfWaitEventInterface(
                        &(((ttDeviceEntryPtr)interfaceHandle)->devRecvEvent));
}
#endif /* TM_RECV_PEND_ISR_POST_NEEDED */

/*
 * Wait for data to be transmitted, if no data has been transmitted, then wait
 * When tfWaitSentInterface returns, the user can then call
 * tfSendCompleteInterface() so that the stack can free the buffer
 * holding the data that has been sent.
 * VERY IMPORTANT:
 * Can only be used when there is a separate send complete task, and
 * TM_TASK_SEND is defined in trsystem.h
 */
#ifdef TM_SENT_PEND_ISR_POST_NEEDED
int tfWaitSentInterface(ttUserInterface interfaceHandle)
{
    return tfWaitEventInterface(
                        &(((ttDeviceEntryPtr)interfaceHandle)->devSentEvent));
}
#endif /* TM_SENT_PEND_ISR_POST_NEEDED */


#ifdef TM_PEND_EVENT_NEEDED
/*
 * Common routine called by tfWaitReceiveInterface(), or tfWaitSentInterface()
 * or tfWaitXmitInterface().
 * While event count is 0, pend on the event.
 */
int tfWaitEventInterface(ttEventEntryPtr eventEntryPtr)
{
    int eventCount;
    for(;;)
    {
        tm_kernel_set_critical;
        eventCount = eventEntryPtr->eventCount;
        if (eventCount == 0)
        {
            eventEntryPtr->eventPending = TM_8BIT_YES; /* pending */
        }
        else
        {
            eventEntryPtr->eventPending = TM_8BIT_NO; /* not pending */
        }
        tm_kernel_release_critical;

        if (eventCount != 0)
        {
            break;
        }
        tm_kernel_pend_event(&eventEntryPtr->eventUnion);
    }

    tm_kernel_set_critical;
    eventEntryPtr->eventCount--;
    tm_kernel_release_critical;

    return TM_ENOERROR;
}
#endif /* TM_PEND_EVENT_NEEDED */


/*
 * tfDeviceSendFailed() function description.
 * Function called from tfDeviceSend() if no Treck transmit task is used,
 * or from tfXmitInterface() if a Treck transmit task is used,
 * when the link post send function returns an error.
 * If a device transmit queue is used, and the returned errorCode from
 * tfDeviceSendBuffer() is TM_ENOBUFS, then the device transmit
 * queue logic dropped a whole packet, and the Treck stack will
 * remove the packet from the send queue, and free it in this routine.
 * If a device transmit queue is used, and returns TM_EIO, the device
 * transmit queue interface guarantees that at least the first buffer was
 * sent, and the last buffer in the packet was sent or queued, and the
 * Treck stack will NOT remove the packet from the send queue in this routine.
 * If no device transmit queue is used, then the last buffer
 * in the packet was not sent by the driver, and the Treck
 * stack will remove the packet from the send queue, and free it in this
 * routine.
 * . If no device transmit queue is used (i.e device driver returned an error
 *   on the last buffer of the packet), or if the errorCode is TM_ENOBUFS
 *   (i.e the device transmit queue logic dropped a whole packet),
 *   dequeue and free the last packet queued to the device send queue.
 * . In all cases, update the interface SNMP MIB out error variables.
 * Parameters
 * devPtr      Pointer to device entry (as returned by tfAddInterface())
 * packetPtr        pointer to packet that did not get sent by the driver
 * errorCode        error returned by either tfDeviceSendBuffer() or the
 *                  device driver send function
 * Return
 * TM_EIO           Part of the packet got sent.
 * TM_ENOBUFS       Packet did not get sent at all.
 */
int tfDeviceSendFailed ( ttDeviceEntryPtr devPtr,
                         ttPacketPtr      packetPtr,
                         int              errorCode,
                         tt8BitPtr        needFreePacketPtr )
{
    ttPacketPtr queuePacketPtr;
#ifdef TM_ERROR_CHECKING
    int         kernelError;
#endif /* TM_ERROR_CHECKING */

/*
 * If device xmit queue is used, it returns TM_EIO for a partial copy
 * of the packet, or TM_ENOBUFS if it did not call the driver send routine
 * with any part of the packet. So if a device transmit queue is used
 * and the error code is not TM_ENOBUFS (i.e it is TM_EIO), then the
 * packet should not be dequeued, and freed here, but rather when
 * tfSendCompleteInterface() is called when the last part of the
 * buffer is sent.
 */
    if (    ( errorCode == TM_ENOBUFS)
         || ( devPtr->devXmitEntryPtr == (ttDevXmitEntryPtr)0) )
/*
 * Either we did not use the device xmit queue, and the device driver
 * returned an error on the last part of the packet, or we did not send any
 * part of a packet when using the device xmit queue
 */
    {
/*
 * Because of the device driver lock, we know that the packet that we
 * failed to send, should be at the end of the send queue, unless the
 * tfSendCompleteInterface() was called inside the device driver function
 * for the failed packet (device driver copies the packet and calls
 * tfSendCompleteInterface from the driver send function for the current
 * packet).
 */
        if (packetPtr == devPtr->devSendQueueLastPtr)
/* Our packet still at the end of the send queue */
        {
#ifdef TM_ERROR_CHECKING
            kernelError = TM_8BIT_ZERO;
#endif /* TM_ERROR_CHECKING */
/* Look for Previous to last packet in the send queue */
            queuePacketPtr = devPtr->devSendQueuePtr;
            if (queuePacketPtr == packetPtr)
            {
/* We were the only packet in the queue */
                devPtr->devSendQueuePtr = TM_PACKET_NULL_PTR;
                devPtr->devSendQueueLastPtr = TM_PACKET_NULL_PTR;
            }
            else
            {
#ifdef TM_ERROR_CHECKING
/*
 * Because sendQueueLastPtr is non null, then sendQueuePtr should also
 * be non null.
 */
                if (queuePacketPtr == (ttPacketPtr)0)
                {
                    tfKernelError("tfDeviceSendFailed",
                                  "Corrupted sendQueuePtr\n");
                    kernelError = TM_8BIT_YES;
                }
                else
#endif /* TM_ERROR_CHECKING */
                {
                    while (    (queuePacketPtr != (ttPacketPtr)0)
                            && (queuePacketPtr->pktChainNextPtr != packetPtr) )
                    {
                        queuePacketPtr = queuePacketPtr->pktChainNextPtr;
                    }
/* Make the previous to last packet in the queue, the last in the queue */
#ifdef TM_ERROR_CHECKING
/*
 * Because sendQueueLast is non null, there should be a packet in the
 * send queue pointing to it
 */
                    if (queuePacketPtr == (ttPacketPtr)0)
                    {
                        tfKernelError("tfDeviceSendFailed",
                                      "corrupted send Queue Chain\n");
                        kernelError = TM_8BIT_YES;
                    }
                    else
#endif /* TM_ERROR_CHECKING */
                    {
/* Make the previous packet in the send queue the last packet */
                        queuePacketPtr->pktChainNextPtr = TM_PACKET_NULL_PTR;
                        devPtr->devSendQueueLastPtr = queuePacketPtr;
                    }
                }
            }
            packetPtr->pktChainNextPtr = TM_PACKET_NULL_PTR;
#ifdef TM_SNMP_MIB
            devPtr->devOutQLen--;
#endif /* TM_SNMP_MIB */
#ifdef TM_ERROR_CHECKING
            if ( kernelError == TM_8BIT_ZERO )
#endif /* TM_ERROR_CHECKING */
            {
/* Indicate that the packet needs to be freed-up. */
                *needFreePacketPtr = TM_8BIT_YES;
            }
        }
        if ( devPtr->devXmitEntryPtr == (ttDevXmitEntryPtr)0 )
        {
/*
 * If the device driver send function failed, when no device transmit queue
 * is used, return TM_EIO
 */
            errorCode = TM_EIO;
        }
    }
/* Update SNMP MIB interface out errors */
#ifdef TM_SNMP_MIB
    if (errorCode == TM_ENOBUFS)
    {
#ifdef TM_USE_IPV4
        devPtr->dev4Ipv4Mib.ipIfStatsOutDiscards++;
#ifdef TM_USE_NG_MIB2
        tm_context(tvDevIpv4Data).ipIfStatsOutDiscards++;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_USE_IPV4 */
    }
    else
    {
        devPtr->devOutErrors++;
    }
#endif /* TM_SNMP_MIB */
    return errorCode;
}

/*
 * Called from the driver open function to query for the maximum number
 * of scattered buffers in one send.
 * Returns:
 * > 0              Maximum number of buffers in a send
 * TM_DEV_ERROR     Invalid interface handle
 */
int tfInterfaceScatterCount( ttUserInterface interfaceHandle )
{
  ttDeviceEntryPtr devPtr;
  int              retCode;

    devPtr = (ttDeviceEntryPtr)interfaceHandle;

    retCode = tfValidInterface( devPtr );
    if( retCode == TM_ENOERROR )
    {
        retCode = devPtr->devScatterExtraCount + 1;
    }
    else
    {
        retCode = TM_DEV_ERROR; /* assume failure */
    }
    return retCode;
}

/*
 * Common code to open a non point to point Link Layer device
 * Either insert new configuration in routing table or query
 * a DHCP/BOOTP server.
 */
int tfLinkOpen(void TM_FAR * interfaceId,
               tt16Bit       multiHomeIndex,
               int           protocolFamily)
{
    ttDeviceEntryPtr    devPtr;
    int                 errorCode;

    devPtr = (ttDeviceEntryPtr)interfaceId;
    errorCode = TM_ENOERROR; /* assume successful */

#ifndef TM_USE_IPV6
/* avoid compiler warnings */
    TM_UNREF_IN_ARG(protocolFamily);
    TM_UNREF_IN_ARG(multiHomeIndex);
#else /* TM_USE_IPV6 */
    if (protocolFamily == PF_INET6)
    {
/*
 * If the link type is Ethernet and an interface identifier has not already
 * been set on this device, form an IPv6 interface identifier from the
 * physical address.  Since the link is Ethernet, the physical address MUST
 * have been set already (in tfNgOpenInterface).  Call
 * tf6IntEui48SetInterfaceId to convert the 48-bit physical address to a
 * 64-bit interface identifier and then to save the ID in the device entry.
 * [RFC2464].R4:10, [RFC2464].R4:20
 */

        if ( tm_6_ll_is_lan(devPtr) &&
             tm_16bit_bits_not_set(devPtr->dev6Flags,
                                   TM_6_DEV_INTERFACE_ID_FLAG))
        {
            errorCode = tf6IntEui48SetInterfaceId(devPtr,
                                                  devPtr->devPhysAddrArr);
        }
        else
        {

/*
 * If the link type is the null link layer, the user may set the physical
 * address manually (not through drvGetPhyAddrFuncPtr) and must set the
 * interface identifier manually (unless the IPv6 address that the user is
 * manually configuring is a link-local scope address). If this has not been
 * done by this point, return error TM_EPERM.
 */
            if (tm_16bit_bits_not_set(devPtr->dev6Flags,
                                      TM_6_DEV_INTERFACE_ID_FLAG)
                && !IN6_IS_ADDR_LINKLOCAL(
                    &devPtr->dev6IpAddrArray[multiHomeIndex]))
            {
                errorCode = TM_EPERM;
            }
        }

        if (errorCode == TM_ENOERROR)
        {
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
            if (tm_16bit_one_bit_set(devPtr->dev6Flags,
                                     TM_6_LL_CONNECTING))
            {
                tm_kernel_set_critical;
                tm_context(tvIpData).ipv6InterfaceTableLastChange =
                    tm_snmp_time_ticks(tvTime);
                tm_kernel_release_critical;
            }
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
/*
 * Set TM_6_LL_CONNECTED and clear TM_6_LL_CONNECTING in dev6Flags since both
 * Ethernet and the null link-layer open immediately (unlike PPP, which much
 * wait for negotiation to finish).
 */
            tm_16bit_clr_bit( devPtr->dev6Flags, TM_6_LL_CONNECTING );
            devPtr->dev6Flags |= TM_6_LL_CONNECTED;
        }

    }
#ifdef TM_USE_IPV4
    else
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_IPV6 */
#ifdef TM_USE_IPV4
    {
/*
 * Set TM_DEV_CONNECTED and clear TM_DEV_CONNECTING in devFlag since both
 * Ethernet and the null link-layer open immediately (unlike PPP, which much
 * wait for negotiation to finish).
 */
        tm_16bit_clr_bit( devPtr->devFlag, TM_DEV_CONNECTING );
        devPtr->devFlag |= TM_DEV_CONNECTED;
    }
#endif /* TM_USE_IPV4 */

    return errorCode;
}

/*
 * Free a user device driver buffer (allocated with either tfGetEthernetBuffer
 * of tfGetDriverBuffer)
 */
int tfFreeDriverBuffer (ttUserBuffer userBuffer)
{
    int retCode;

/* PARAMETER CHECK */
    if (    (userBuffer != (ttUserBuffer)0)
         && ( (   ((ttPacketPtr)userBuffer)->pktUserFlags
                & TM_PKTF_USER_DEV_OWNS ) ) )
    {
        tm_16bit_clr_bit( ((ttPacketPtr)userBuffer)->pktUserFlags,
                          TM_PKTF_USER_DEV_OWNS );
/* Parameter is non null, and user device owns the packet */
        tfFreePacket((ttPacketPtr)userBuffer, TM_SOCKET_UNLOCKED);
        retCode = TM_ENOERROR;
    }
    else
    {
/* Parameter is null, or user did not own the packet */
        retCode = TM_DEV_ERROR;
    }
    return retCode;
}

/* Unlock device driver. Check if we need to call tfSendCompleteInterface() */
void tfDeviceUnlockSendComplete ( ttDeviceEntryPtr devPtr )
{
    int sendCompltCount;
#ifdef TM_USE_DRV_ONE_SCAT_SEND
    int sendCompltPacketCount;
#endif /* TM_USE_DRV_ONE_SCAT_SEND */

#ifdef TM_USE_INDRT
/*
 * If using INDRT, need to be critical, as devSendCompltCount could be
 * increased elsewhere in INDRT context, i.e. ISR level.
 */
    tm_kernel_set_critical;
#endif /* TM_USE_INDRT */
    sendCompltCount = devPtr->devSendCompltCount;
    devPtr->devSendCompltCount = 0;
#ifdef TM_USE_DRV_ONE_SCAT_SEND
    sendCompltPacketCount = devPtr->devSendCompltPacketCount;
    devPtr->devSendCompltPacketCount = 0;
#endif /* TM_USE_DRV_ONE_SCAT_SEND */
#ifdef TM_USE_INDRT
    tm_kernel_release_critical;
#endif /* TM_USE_INDRT */
#ifdef TM_USE_DRV_ONE_SCAT_SEND
    if (sendCompltPacketCount != 0)
    {
        tfSendCompletePacketsInterfaceUnlock( devPtr,
                                              sendCompltPacketCount );
    }
    else
    {
        tm_unlock(&(devPtr->devDriverLockEntry));
    }
#else /* !TM_USE_DRV_ONE_SCAT_SEND */
    tm_unlock(&(devPtr->devDriverLockEntry));
#endif /* !TM_USE_DRV_ONE_SCAT_SEND */
    while (sendCompltCount != 0)
    {
        tfSendCompleteInterface( (ttUserInterface)devPtr,
                                   TM_DEV_SEND_COMPLETE_APP
#ifdef TM_USE_INDRT
                                 | TM_DEV_SEND_COMPLETE_DELAYED
#endif /* TM_USE_INDRT */
                                 );
        sendCompltCount--;
    }
}


#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
/* APIs for sorting IP addresses to improve performance of IP address lookup
   in the receive-path when many IP aliases are configured on a single
   interface. */

/****************************************************************************
* FUNCTION: tfRemoveSortedIpAddr
*
* PURPOSE: Remove IPv4 address (unicast or directed broadcast) or IPv6
*   unicast address from sorted cache of IP addresses
*
* PARAMETERS:
*   removeDevPtr:
*       pointer to the device/interface that IP address is configured on,
*       this is the IP address to remove from the sorted array.
*   removeMHomeIndex:
*       multi-home index where the IP address is configured on the interface,
*       this is the IP address to remove from the sorted array.
*   ipAddrCachePtr:
*       control block for sorted IP address array.
*
* RETURNS:
*   Nothing
*
* NOTES:
*   Supports both IPv4 and IPv6.
*
****************************************************************************/
void tfRemoveSortedIpAddr(
    ttDeviceEntryPtr removeDevPtr, tt16Bit removeMHomeIndex,
    ttSortedIpAddrCachePtr ipAddrCachePtr)
{
    tt32Bit loopIndex, sortedArrayCount, entriesToMoveUp;
    ttIpAddrEntryPtr deletionPointPtr, tempIpAddrEntryPtr, sortedArray;
    ttDeviceEntryPtr devPtr;
    tt16Bit mHomeIndex;
    tt16Bit devIndex; /* 1-based index, same as what SNMP uses */

    tm_call_lock_wait(&(ipAddrCachePtr->sipcLockEntry));

    sortedArray = ipAddrCachePtr->sipcIpAddrArray;
    if (sortedArray != TM_IP_ADDR_ENTRY_NULL_PTR)
    {
        sortedArrayCount = ipAddrCachePtr->sipcIpAddrArrayCount;

        if (sortedArrayCount > 0)
        {
/* sortedArrayCount should not be zero here, but we don't want to fail in any
   case */
            deletionPointPtr = TM_IP_ADDR_ENTRY_NULL_PTR;
            for (loopIndex = 0; loopIndex < sortedArrayCount; loopIndex++)
            {
/* iterate through the sorted array to find the deletion point. IP addresses
   are sorted in ascending order. */
                devIndex = sortedArray[loopIndex].devIndex;
                tm_assert(tfRemoveSortedIpAddr, (devIndex > 0));
                tm_assert(
                    tfRemoveSortedIpAddr,
                    (devIndex <= tm_context(tvAddedInterfaces)));
                devPtr = tm_context(tvDevIndexToDevPtr)[devIndex];
                tm_assert(
                    tfRemoveSortedIpAddr, (devPtr != TM_DEV_NULL_PTR));
                mHomeIndex = sortedArray[loopIndex].mHomeIndex;

                if ((removeDevPtr == devPtr) &&
                    (removeMHomeIndex == mHomeIndex))
                {
/* we found the deletion point */
                    deletionPointPtr = &sortedArray[loopIndex];
                    break;
                }
            }

            tm_assert(tfRemoveSortedIpAddr,
                      (deletionPointPtr != TM_IP_ADDR_ENTRY_NULL_PTR));
            if (deletionPointPtr != TM_IP_ADDR_ENTRY_NULL_PTR)
            {
/* delete the IP address entry from the array. */
                deletionPointPtr->devIndex = TM_16BIT_ZERO;
                deletionPointPtr->mHomeIndex = TM_16BIT_ZERO;

                tm_assert(
                    tfRemoveSortedIpAddr, (sortedArrayCount > loopIndex));
                entriesToMoveUp = sortedArrayCount - (loopIndex + 1);
/* move IP address entries after the deletion point up one in the array. */
                tempIpAddrEntryPtr = deletionPointPtr;
                for (loopIndex = 0; loopIndex < entriesToMoveUp; loopIndex++)
                {
                    tm_bcopy(
                        tempIpAddrEntryPtr + 1,
                        tempIpAddrEntryPtr,
                        sizeof(ttIpAddrEntry));
                    tempIpAddrEntryPtr++;
                }
                tm_assert(
                    tfRemoveSortedIpAddr,
                    (tempIpAddrEntryPtr
                     == &sortedArray[(sortedArrayCount - 1)]));

/* decrement the count of IP address entries in the sorted array */
                ipAddrCachePtr->sipcIpAddrArrayCount--;
/* Modify the cache serial number for cache lookup refresh */
                ipAddrCachePtr->sipcSerialNum++;
            }
        }
    }

    tm_call_unlock(&(ipAddrCachePtr->sipcLockEntry));
    return;
}

#ifdef TM_USE_IPV4
/****************************************************************************
* FUNCTION: tf4InsertSortedIpAddr
*
* PURPOSE: Insert IPv4 address (unicast or directed broadcast) in sorted
*   order into array of IP addresses. Used to optimize lookup of IP addresses
*   in the receive-path when many IP aliases are configured on the interface.
*
* PARAMETERS:
*   insertDevPtr:
*       pointer to the device/interface that IP address is configured on,
*       this is the IP address to be inserted into the sorted array.
*   insertMHomeIndex:
*       multi-home index where the IP address is configured on the interface,
*       this is the IP address to be inserted into the sorted array.
*   ipAddrCachePtr:
*       control block for sorted IP address array.
*   dBroadFlag:
*       set to TM_8BIT_YES for directed broadcast addresses, otherwise set
*       to TM_8BIT_ZERO.
*
* RETURNS:
*   Nothing
*
* NOTES:
*   We do not allow a 0 IP address to be inserted.
*
****************************************************************************/
void tf4InsertSortedIpAddr(
    ttDeviceEntryPtr insertDevPtr, tt16Bit insertMHomeIndex,
    ttSortedIpAddrCachePtr ipAddrCachePtr, tt8Bit dBroadFlag)
{
    tt32Bit loopIndex, sortedArrayCount, entriesToMoveDown;
    ttIpAddrEntryPtr insertionPointPtr, tempIpAddrEntryPtr, sortedArray;
    tt4IpAddress devIpAddr, ipAddrToInsert;
    ttDeviceEntryPtr devPtr;
    tt16Bit mHomeIndex;
    tt16Bit devIndex; /* 1-based index, same as what SNMP uses */

    tm_call_lock_wait(&(ipAddrCachePtr->sipcLockEntry));

    sortedArray = ipAddrCachePtr->sipcIpAddrArray;
    if (sortedArray != TM_IP_ADDR_ENTRY_NULL_PTR)
    {
        sortedArrayCount = ipAddrCachePtr->sipcIpAddrArrayCount;
        tm_assert(
            tf4InsertSortedIpAddr,
            (sortedArrayCount
             <= (tt32Bit)
             (tm_context(tvAddedInterfaces) * TM_MAX_IPS_PER_IF)));

        if (dBroadFlag == TM_8BIT_YES)
        {
            tm_ntohl(
                tm_ip_dev_dbroad_addr(
                    insertDevPtr, insertMHomeIndex), ipAddrToInsert);
        }
        else
        {
            tm_ntohl(
                tm_ip_dev_addr(
                    insertDevPtr, insertMHomeIndex), ipAddrToInsert);
        }

/* we do not allow a 0 IP address to be inserted */
        if (ipAddrToInsert != 0)
        {
            insertionPointPtr = TM_IP_ADDR_ENTRY_NULL_PTR;
            for (loopIndex = 0; loopIndex < sortedArrayCount; loopIndex++)
            {
/* iterate through the sorted array to find the insertion point. IP addresses
   are sorted in ascending order. */
                devIndex = sortedArray[loopIndex].devIndex;
                tm_assert(tf4InsertSortedIpAddr, (devIndex > 0));
                tm_assert(
                    tf4InsertSortedIpAddr,
                    (devIndex <= tm_context(tvAddedInterfaces)));
                devPtr = tm_context(tvDevIndexToDevPtr)[devIndex];
                tm_assert(tf4InsertSortedIpAddr,
                          (devPtr != TM_DEV_NULL_PTR));
                mHomeIndex = sortedArray[loopIndex].mHomeIndex;
                tm_assert(tf4InsertSortedIpAddr,
                          (mHomeIndex < TM_MAX_IPS_PER_IF));

                if (dBroadFlag == TM_8BIT_YES)
                {
                    tm_ntohl(
                        tm_ip_dev_dbroad_addr(devPtr, mHomeIndex),
                        devIpAddr);
                }
                else
                {
                    tm_ntohl(
                        tm_ip_dev_addr(devPtr, mHomeIndex), devIpAddr);
                }
                tm_assert(tf4InsertSortedIpAddr, (devIpAddr != 0));

                if (ipAddrToInsert < devIpAddr)
                {
/* we found the insertion point */
                    insertionPointPtr = &sortedArray[loopIndex];
                    break;
                }
            }

            if (insertionPointPtr != TM_IP_ADDR_ENTRY_NULL_PTR)
            {
                entriesToMoveDown = sortedArrayCount - loopIndex;
                tm_assert(tf4InsertSortedIpAddr, (entriesToMoveDown > 0));
/* move IP address entries after the insertion point down one in the array. */
                tm_assert(tf4InsertSortedIpAddr, (sortedArrayCount > 0));
                tempIpAddrEntryPtr = &sortedArray[sortedArrayCount];
                for (loopIndex = 0; loopIndex < entriesToMoveDown; loopIndex++)
                {
                    tempIpAddrEntryPtr--;
                    tm_bcopy(
                        tempIpAddrEntryPtr,
                        tempIpAddrEntryPtr + 1,
                        sizeof(ttIpAddrEntry));
                }
                tm_assert(tf4InsertSortedIpAddr,
                          (tempIpAddrEntryPtr == insertionPointPtr));
            }
            else
            {
/* insert the new IP address entry as the last entry in the array. */
                insertionPointPtr = &sortedArray[sortedArrayCount];
            }
/* insert the new IP address entry at the insertion point */
            insertionPointPtr->devIndex
                = (tt16Bit) insertDevPtr->devIndex;
            insertionPointPtr->mHomeIndex = insertMHomeIndex;

/* increment the count of IP address entries in the sorted array */
            ipAddrCachePtr->sipcIpAddrArrayCount++;
/* Modify the cache serial number for cache lookup refresh */
            ipAddrCachePtr->sipcSerialNum++;
        }
    }

    tm_call_unlock(&(ipAddrCachePtr->sipcLockEntry));
    return;
}

/* #define TM_DEBUG_SORTED_IP_ADDR_LOOKUP */
/****************************************************************************
* FUNCTION: tf4LookupSortedIpAddr
*
* PURPOSE: Lookup IPv4 (unicast or directed broadcast) address in sorted
*   array of IP addresses.
*
* PARAMETERS:
*   ipAddr:
*       IPv4 address to lookup in the sorted array
*   scopeDevIndex:
*       1-based device index (refer to ttDeviceEntry.devIndex): specify 0
*       if you don't want to restrict scope of the lookup to a specific
*       device, otherwise specify the device index of the device you want
*       to perform the lookup on.
*   ipAddrCachePtr:
*       control block for sorted IP address array.
*   dBroadFlag:
*       set to TM_8BIT_YES for directed broadcast addresses, otherwise set
*       to TM_8BIT_ZERO.
*
* RETURNS:
*   == TM_IP_ADDR_ENTRY_NULL_PTR:
*       IPv6 address was not found in array.
*   != TM_IP_ADDR_ENTRY_NULL_PTR:
*       pointer to ttIpAddrEntry structure, with devIndex and mHomeIndex
*       populated with the information for the found IPv4 address
*
* NOTES:
*   Implements uniform binary search algorithm as described by the Knuth
*   book "Sorting and Searching":
*   Given a table of records R1, R2, ..., Rn, whose keys are in increasing
*   order K1 < K2 < ... < Kn, this algorithm searches a given argument K. If
*   N is even, the algorithm will sometimes refer to a dummy key K0 which
*   should be set to negative infinity (or any value less than the least
*   possible key value - in this case, 0, since we don't allow insertion of a
*   0 IP address in the cache). We assume that N >= 1.
*
****************************************************************************/
ttIpAddrEntryPtr tf4LookupSortedIpAddr(
    tt4IpAddress ipAddr, int scopeDevIndex,
    ttSortedIpAddrCachePtr ipAddrCachePtr, tt8Bit dBroadFlag)
{
    ttIpAddrEntryPtr startSearchPoint, midSearchPoint, foundEntry;
    tt32Bit          N, m, nextM, ceilingAdjustment;
    int              i;
    tt4IpAddress     devIpAddr, searchIpAddr;
    ttDeviceEntryPtr devPtr;
    tt16Bit          mHomeIndex;
    tt16Bit          devIndex; /* 1-based index, same as what SNMP uses */

    foundEntry = TM_IP_ADDR_ENTRY_NULL_PTR; /* assume not found */
    tm_lock_wait(&(ipAddrCachePtr->sipcLockEntry));

    startSearchPoint = ipAddrCachePtr->sipcIpAddrArray;
    N = ipAddrCachePtr->sipcIpAddrArrayCount;

    if ((startSearchPoint != TM_IP_ADDR_ENTRY_NULL_PTR) && (N > 0))
    {
        tm_ntohl(ipAddr, searchIpAddr);

/* U1. [Initialize.] Set i = ceiling(N/2), m = floor(N/2) */
        ceilingAdjustment = N & 1;
        nextM = N >> 1; /* see first statement in do loop */
        i = nextM + ceilingAdjustment;

        do
        {
            m = nextM; /* initialization: m = floor(N/2) */
            ceilingAdjustment = m & 1;
            nextM = m >> 1;
#ifdef TM_DEBUG_SORTED_IP_ADDR_LOOKUP
            tm_assert(tf4LookupSortedIpAddr, (i <= (int) N));
#endif /* TM_DEBUG_SORTED_IP_ADDR_LOOKUP */
            midSearchPoint = &(startSearchPoint[i - 1]);

            devIndex = midSearchPoint->devIndex;
#ifdef TM_DEBUG_SORTED_IP_ADDR_LOOKUP
            tm_assert(tf4LookupSortedIpAddr, (devIndex > 0));
            tm_assert(
                tf4LookupSortedIpAddr,
                (devIndex <= tm_context(tvAddedInterfaces)));
#endif /* TM_DEBUG_SORTED_IP_ADDR_LOOKUP */

            devPtr = tm_context(tvDevIndexToDevPtr)[devIndex];
#ifdef TM_DEBUG_SORTED_IP_ADDR_LOOKUP
            tm_assert(tf4LookupSortedIpAddr, (devPtr != TM_DEV_NULL_PTR));
#endif /* TM_DEBUG_SORTED_IP_ADDR_LOOKUP */
            mHomeIndex = midSearchPoint->mHomeIndex;
#ifdef TM_DEBUG_SORTED_IP_ADDR_LOOKUP
            tm_assert(tf4LookupSortedIpAddr, (mHomeIndex < TM_MAX_IPS_PER_IF));
#endif /* TM_DEBUG_SORTED_IP_ADDR_LOOKUP */

            if (dBroadFlag == TM_8BIT_YES)
            {
                tm_ntohl(
                    tm_ip_dev_dbroad_addr(devPtr, mHomeIndex), devIpAddr);
            }
            else
            {
                tm_ntohl(
                    tm_ip_dev_addr(devPtr, mHomeIndex), devIpAddr);
            }
#ifdef TM_DEBUG_SORTED_IP_ADDR_LOOKUP
            tm_assert(tf4LookupSortedIpAddr, (devIpAddr != 0));
#endif /* TM_DEBUG_SORTED_IP_ADDR_LOOKUP */

/* U2. [Compare.] If K < Ki, got to U3; if K > Ki, go to U4; and if K == Ki,
   the algorithm terminates successfully. */
            if (searchIpAddr < devIpAddr)
            {
/* U3. [Decrease i]. (We have pinpointed the search to an interval which
   contains either m or (m - 1) records; i points just to the right of this
   interval.) If m == 0, the algorithm terminates unsuccessfully. Otherwise,
   set i = (i - ceiling(m/2)); then set m = floor(m/2) and return to U2. */
                if (m == 0)
                {
                    break; /* not found */
                }
                i = i - (nextM + ceilingAdjustment);
            }
            else if (searchIpAddr > devIpAddr)
            {
/* U4. [Increase i]. (We have pinpointed the search to an interval which
   contains either m or (m - 1) records; i points just to the left of this
   interval.) If m == 0, the algorithm terminates unsuccessfully. Otherwise
   set i = (i + ceiling(m/2)); then set m = floor(m/2) and return to U2. */
                if (m == 0)
                {
                    break; /* not found */
                }
                i = i + (nextM + ceilingAdjustment);
            }
            else
            {
/* the algorithm terminates successfully */
                if ((scopeDevIndex == 0) || (devIndex == scopeDevIndex))
                {
                    foundEntry = midSearchPoint;
                    break;
                }
                else
                {
/* iterate through matching keys (in both directions) to find match on
   devIndex */
                    /* TBD */
                    break; /* for now, not found */
                }
            }
        }
        while (i > 0);
    }

    tm_unlock(&(ipAddrCachePtr->sipcLockEntry));
    return foundEntry;
}
#endif /* TM_USE_IPV4 */
#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */

#ifdef TM_USE_STRONG_ESL
/*
 * Given a device name return the corresponding interface handle.
 */
ttUserInterface tfInterfaceNameToDevice(ttUserConstCharPtr namePtr)
{
    ttDeviceEntryPtr devPtr;

    tm_call_lock_wait(&tm_context(tvDeviceListLock));
    devPtr = tm_context(tvDeviceList);
    while (devPtr != TM_DEV_NULL_PTR)
    {
        if (    tm_strcmp((ttCharPtr)(devPtr->devNameArray), namePtr)
             == TM_STRCMP_EQUAL )
        {
            break;
        }
        devPtr = devPtr->devNextDeviceEntry;
    }
    tm_call_unlock(&tm_context(tvDeviceListLock));
    return (ttUserInterface)devPtr;
}

/*
 * Given an interface handle, store a null terminated name in namePtr up to
 * nameLen.
 */
int tfInterfaceName(ttUserInterface interfaceHandle,
                    ttUserCharPtr   namePtr,
                    int             nameLen)
{
    ttDeviceEntryPtr devPtr;
    int              errorCode;

    errorCode = tfValidInterface(interfaceHandle);
    if (errorCode == TM_ENOERROR)
    {
        devPtr = (ttDeviceEntryPtr)interfaceHandle;
        namePtr = tm_strncpy(namePtr, &devPtr->devNameArray[0], nameLen);
        if (namePtr == (ttUserCharPtr)0)
        {
            errorCode = TM_ENOBUFS;
        }
    }
    return errorCode;
}
#endif /* TM_USE_STRONG_ESL */

#ifdef TM_USE_IPV6
/*
 * Check if same network address is configured on another multihome, and check
 * whether same sol node multicast address is needed elsewhere.
 * Re-add local address if same found.
 * Leave sol node multicast if not found.
 */
static void tf6CheckOtherMhomeConfig(ttDeviceEntryPtr devPtr,
                                     tt16Bit          mHomeIndex)
{
    tt6IpAddressPtr     localIpAddrPtr;
    tt6IpAddress        scopeIpAddr;
    tt6IpAddress        tempAddr;
    int                 networkMatch;
    int                 netMatchTemp;
    int                 maskIndex;
#ifndef TM_SINGLE_INTERFACE_HOME
    int                 maxMhomeProxyEntries;
    int                 maxMhomeEntries;
    int                 i;
#endif /* ! TM_SINGLE_INTERFACE_HOME */
    tt16Bit             prefixLen;
    tt8Bit              networkFound;
    tt8Bit              solNodeFound;
    tt8Bit              checkNetwork;

    localIpAddrPtr = &(devPtr->dev6IpAddrArray[mHomeIndex]);
    prefixLen = (tt16Bit) devPtr->dev6PrefixLenArray[mHomeIndex];
    solNodeFound = TM_8BIT_ZERO;
    networkFound = TM_8BIT_ZERO;
    if (devPtr->dev6IpAddrFlagsArray[mHomeIndex] & TM_6_DEV_IP_CONF_STARTED)
    {
/* Do not check for a configured duplicate network address */
       checkNetwork = TM_8BIT_NO;
    }
    else
    {
/* Check for a configured duplicate network address */
       checkNetwork = TM_8BIT_YES;
    }
/*
 * Check configured addresses.
 * Select maximum entries as the number of used entries plus one because
 * we may be called during address configuration prior to incrementing
 * the number of used entries.
 */
#ifdef TM_6_USE_MIP_HA
    maxMhomeProxyEntries = (int)devPtr->dev6MhomeUsedProxyEntries + 1;
#else /* ! TM_6_USE_MIP_HA */
#ifdef TM_6_USE_DHCP
    maxMhomeProxyEntries = (int)devPtr->dev6MhomeUsedDhcpEntries + 1;
#else /* ! TM_6_USE_DHCP */
    maxMhomeProxyEntries = (int)devPtr->dev6MhomeUsedAutoEntries + 1;
#endif /* ! TM_6_USE_DHCP */
#endif /* ! TM_6_USE_MIP_HA */
    if (maxMhomeProxyEntries > (  TM_MAX_IPS_PER_IF
                                + TM_6_MAX_AUTOCONF_IPS_PER_IF
                                + TM_6_MAX_DHCP_IPS_PER_IA
                                + TM_6_MAX_MIP_PROXY_IPS_PER_IF))
    {
        maxMhomeProxyEntries =   TM_MAX_IPS_PER_IF
                               + TM_6_MAX_AUTOCONF_IPS_PER_IF
                               + TM_6_MAX_DHCP_IPS_PER_IA
                               + TM_6_MAX_MIP_PROXY_IPS_PER_IF;
    }
    maxMhomeEntries = (int)devPtr->dev6MhomeUsedEntries + 1;
    if (maxMhomeEntries > TM_MAX_IPS_PER_IF)
    {
        maxMhomeEntries = TM_MAX_IPS_PER_IF;
    }
    for (i = 0; i < maxMhomeProxyEntries;)
    {
/* make sure we don't check the IP address we are removing */
        if ((i != (int) mHomeIndex) &&
            (devPtr->dev6IpAddrFlagsArray[i] &
             TM_6_DEV_IP_CONFIG))
        {
            if (solNodeFound == TM_8BIT_ZERO)
/* Check for duplicate solicited node */
            {
                if (tm_6_is_addr_same_sol_node(
                                localIpAddrPtr,
                                &(devPtr->dev6IpAddrArray[i])))
                {
/* we found a solicited-node multicast address match on a different IP
   address on the same interface */
                    solNodeFound = TM_8BIT_YES;

                    if (   (checkNetwork == TM_8BIT_NO)
                        || (networkFound == TM_8BIT_YES) )
                    {
                        break;
                    }
                }
            }
            if (    (checkNetwork != TM_8BIT_NO)
                 && (networkFound == TM_8BIT_ZERO) )
/* Check for duplicate network address */
            {
                if (prefixLen == (tt16Bit) devPtr->dev6PrefixLenArray[i])
                {
                    tm_6_ip_net_match_prefix(
                                networkMatch,
                                localIpAddrPtr,
                                &(devPtr->dev6IpAddrArray[i]),
                                prefixLen,
                                maskIndex,
                                netMatchTemp);

                    if (networkMatch)
                    {
/* We found a subnet match on a different IP address on the same interface */
                        networkFound = TM_8BIT_YES;

                        if (!(tm_ll_is_pt2pt(devPtr)))
                        {
/*
 * We found a subnet match on a different IP address on the same interface
 * BUG ID 1158: add it as local route.
 */
/* If the other matching IP address is local-scope, the scope ID
   needs to be embedded in the IPv6 address that we use to add the local
   route. */
                            tm_6_ip_copy(
                                        &(devPtr->dev6IpAddrArray[i]),
                                        &scopeIpAddr );

/* Embed interface index or site ID in link-local or site-local addresses */
                            tm_6_dev_scope_addr(
                                        &scopeIpAddr,
                                        devPtr);

                            (void) tf6RtAddLocal(
                                        devPtr,
                                        &scopeIpAddr, /* key */
                                        (int) prefixLen,
                                        &scopeIpAddr,
                                        (tt16Bit) i);
                        }
                        if (solNodeFound == TM_8BIT_YES)
                        {
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            i++;
            if ((i < TM_MAX_IPS_PER_IF) && (i >= maxMhomeEntries))
            {
/* advance to auto-configured entries */
                i = TM_MAX_IPS_PER_IF;
            }

            continue;
        }

        i++;
    }
    if (solNodeFound == TM_8BIT_ZERO)
    {
/* No other addresses are configured with the same solicited-node multicast
   address. Leave the solicited-node multicast group.
   ([RFC2461].R7.2.1:30, [RFC2710].R4:170, [RFC2710].R5:110) */
        tm_6_addr_to_sol_node(localIpAddrPtr, &tempAddr);
#ifdef TM_6_USE_MLD
        (void) tf6MldLeaveGroup(TM_8BIT_NO, devPtr, &tempAddr
#ifdef TM_6_USE_MLDV2
            ,
            TM_6_MODE_IS_INCLUDE, TM_6_SRC_ADDR_ENTRY_NULL_PTR
#endif /* TM_6_USE_MLDV2 */
            );
#else /* !TM_6_USE_MLD */
        (void) tf6MldDeleteGroup(devPtr, &tempAddr);
#endif /* !TM_6_USE_MLD */
    }
}

/* Reset configuration flag and addresses for this multihome entry */
static void tf6ResetMhomeConfig (ttDeviceEntryPtr devPtr,
                                 tt16Bit          mHomeIndex)
{
#ifdef TM_6_USE_MIP_MN
    devPtr->dev6IpAddrFlagsArray[mHomeIndex]
                &= TM_6_DEV_MIP_REGISTERED_FLAG;
#else /* ! TM_6_USE_MIP_MN */
    devPtr->dev6IpAddrFlagsArray[mHomeIndex] = 0;
#endif /* ! TM_6_USE_MIP_MN */
    tf6UpdateCacheAddress( devPtr, mHomeIndex );
#ifdef TM_USE_VCHAN
/* Reinitialize virtual channel to 0 */
    tm_6_ip_dev_vchan(devPtr, mHomeIndex) = 0;
#endif /* TM_USE_VCHAN */
    devPtr->dev6PrefixLenArray[mHomeIndex] = 0;
#ifndef TM_6_USE_MIP_MN
/* clear the IPv6 address on the interface to all 0's */
    tm_6_ip_copy_structs(in6addr_any, devPtr->dev6IpAddrArray[mHomeIndex]);
#endif /* ! TM_6_USE_MIP_MN */
}
#endif /* TM_USE_IPV6 */

#ifdef TM_USE_STOP_TRECK
void tfDeviceListClose (int flags)
{
    ttDeviceEntryPtr    devPtr;
    ttDeviceEntryPtr    nextDevPtr;
#ifdef TM_USE_STOP_TRECK
#ifdef TM_USE_IPV4
    int                 i;
    int                 j;
    int                 k;
    int                 maxIndex;
    int                 flag;
#endif /* TM_USE_IPV4 */
#endif /* TM_USE_STOP_TRECK */

#ifndef TM_USE_STOP_TRECK
    TM_UNREF_IN_ARG(flags);
#endif /* !TM_USE_STOP_TRECK */
/* Lock the device list */
    tm_call_lock_wait(&tm_context(tvDeviceListLock));
    devPtr = tm_context(tvDeviceList);
#ifdef TM_SINGLE_INTERFACE_HOME
    if (devPtr != TM_DEV_NULL_PTR)
#else /* !TM_SINGLE_INTERFACE_HOME */
    while (devPtr != TM_DEV_NULL_PTR)
#endif /* !TM_SINGLE_INTERFACE_HOME */
    {
        nextDevPtr = devPtr->devNextDeviceEntry;
        tm_call_unlock(&tm_context(tvDeviceListLock));
#ifdef TM_USE_STOP_TRECK
        if (flags & TM_DEVF_UNINITIALIZING)
        {
            tm_call_lock_wait(&(devPtr->devLockEntry));
/* Uninitializing the device. Do not send anything. */
            devPtr->devFlag2 = (tt16Bit)
                    (devPtr->devFlag2 | TM_DEVF2_UNINITIALIZING);
#ifdef TM_USE_IPV4
            for (i = 0; i < TM_BOOT_MAX_TYPES; i++)
            {
                if (devPtr->devBootInfo[i].bootFuncPtr
                                                     != (ttDevBootFuncPtr)0)
/* BOOTP or DHCP */
                {
                    for (j = 0; j <= TM_BT_USER; j++)
/* CONF, or USER */
                    {
                        if (j == TM_BT_CONF)
                        {
                            maxIndex = TM_MAX_IPS_PER_IF;
                            flag = TM_LL_CLOSE_STARTED;
                        }
                        else
                        {
                            maxIndex = devPtr->devBtUserMaxEntries;
                            flag = TM_LL_USER_STOP;
                        }
                        for (k = 0; k < maxIndex; k++)
                        {
/*
 * CONF or User Stop function
 */
                            (void)(*(devPtr->devBootInfo[i].bootFuncPtr))
                                    ( (ttUserInterface)devPtr,
                                      flag,
                                      (tt16Bit)k );
                        }
                    }
                }
            }
#ifdef TM_IGMP
            tfIgmpDeInit(devPtr);
#endif /* TM_IGMP */
#endif /* TM_USE_IPV4 */
/*
 * Call the link layer close function while the device UNINITIALIZING flag
 * is set, since the device could have already been closed, so that
 * we can free PPP buffers for example.
 */
            (void)(*(devPtr->devLinkLayerProtocolPtr->
                                lnkCloseFuncPtr))((void TM_FAR *)devPtr);
            tm_call_unlock(&(devPtr->devLockEntry));
        }
#endif /* TM_USE_STOP_TRECK */
        tfCloseInterface(devPtr);
        tm_call_lock_wait(&tm_context(tvDeviceListLock));
#ifdef TM_USE_STOP_TRECK
        if (flags & TM_DEVF_UNINITIALIZING)
        {
/* Remove the entry */
            tm_context(tvDeviceList) = nextDevPtr;
/* Move it to the free list */
/* Put it into the device list */
            devPtr->devNextDeviceEntry = tm_context(tvDeviceToFreeList);
            tm_context(tvDeviceToFreeList) = devPtr;
        }
#endif /* TM_USE_STOP_TRECK */
#ifndef TM_SINGLE_INTERFACE_HOME
        devPtr = nextDevPtr;
#endif /* !TM_SINGLE_INTERFACE_HOME */
    }
    tm_call_unlock(&tm_context(tvDeviceListLock));
    return;
}
#endif /* TM_USE_STOP_TRECK */

#ifdef TM_USE_STOP_TRECK
void tfDeviceListFree (
#ifdef TM_MULTIPLE_CONTEXT
                ttUserContext contextHandle
#else /* !TM_MULTIPLE_CONTEXT */
                void
#endif /* TM_MULTIPLE_CONTEXT */
                      )
{
    ttDeviceEntryPtr    devPtr;

/* Lock the device list */
    tm_call_lock_wait(&tm_context_var(tvDeviceListLock));
    devPtr = tm_context_var(tvDeviceToFreeList);
#ifdef TM_SINGLE_INTERFACE_HOME
    if (devPtr != TM_DEV_NULL_PTR)
#else /* TM_SINGLE_INTERFACE_HOME */
    while (devPtr != TM_DEV_NULL_PTR)
#endif /* TM_SINGLE_INTERFACE_HOME */
    {
/* Remove the entry */
        tm_context_var(tvDeviceToFreeList) = devPtr->devNextDeviceEntry;
        tm_call_unlock(&tm_context_var(tvDeviceListLock));
        tfDeviceFree(devPtr);
        tm_call_lock_wait(&tm_context_var(tvDeviceListLock));
#ifndef TM_SINGLE_INTERFACE_HOME
/* Next Entry */
        devPtr = tm_context_var(tvDeviceToFreeList);
#endif /* !TM_SINGLE_INTERFACE_HOME */
    }
    tm_call_unlock(&tm_context_var(tvDeviceListLock));
    return;
}

static void tfDeviceFree(ttDeviceEntryPtr devPtr)
{
#ifdef TM_USE_IPV4
    int i;
    int j;
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV4
    for (i = 0; i <= TM_BT_USER; i++)
    {
        for (j = 0; j < TM_BOOT_MAX_TYPES ; j++)
        {
            if (devPtr->devBtEntryTablePtr[i][j] != (ttBtEntryPtrPtr)0)
            {
                if ((i != TM_BT_USER) || (j != TM_BOOT_BOOTP))
/* USER BOOTP table of btEntry pointers is not allocated separately */
                {
/* One allocation for the BT_USER tables */
                    tm_kernel_free(devPtr->devBtEntryTablePtr[i][j]);
                }
                devPtr->devBtEntryTablePtr[i][j] = (ttBtEntryPtrPtr)0;
            }
        }
    }
#endif /* TM_USE_IPV4 */
#if (defined(TM_OPTIMIZE_MANY_MHOMES) && !defined(TM_SINGLE_INTERFACE_HOME))
#ifdef TM_USE_IPV4
/* free the space for the sorted array of directed broadcast addresses
   on the interface. */

    if (devPtr->devSortedDBroadIpAddrCache.sipcIpAddrArray !=
                                                     (ttIpAddrEntryPtr)0)
    {
        tm_kernel_free(devPtr->devSortedDBroadIpAddrCache.sipcIpAddrArray);
        devPtr->devSortedDBroadIpAddrCache.sipcIpAddrArray =
                                                      (ttIpAddrEntryPtr)0;
    }
/* Free sorted IPv4 address cache */
    tm_call_lock_wait(&(tm_context(tv4SortedIpAddrCache).sipcLockEntry));
    if (tm_context(tv4SortedIpAddrCache).sipcIpAddrArray != (ttIpAddrEntryPtr)0)
    {
        tm_free(tm_context(tv4SortedIpAddrCache).sipcIpAddrArray);
        tm_context(tv4SortedIpAddrCache).sipcIpAddrArray =
                                                TM_IP_ADDR_ENTRY_NULL_PTR;
    }
    tm_call_unlock(&(tm_context(tv4SortedIpAddrCache).sipcLockEntry));
#endif /* TM_USE_IPV4 */
#ifdef TM_USE_IPV6
    tm_call_lock_wait(&(tm_context(tv6SortedIpAddrCache).sipcLockEntry));
/* Free sorted IPv6 address cache */
    if (tm_context(tv6SortedIpAddrCache).sipcIpAddrArray = (ttIpAddrEntryPtr)0)
    {
        tm_free(tm_context(tv6SortedIpAddrCache).sipcIpAddrArray);
        tm_context(tv6SortedIpAddrCache).sipcIpAddrArray =
                                                     TM_IP_ADDR_ENTRY_NULL_PTR;
    }
    tm_call_unlock(&(tm_context(tv6SortedIpAddrCache).sipcLockEntry));
#endif /* TM_USE_IPV6 */

#endif /* TM_OPTIMIZE_MANY_MHOMES and not TM_SINGLE_INTERFACE_HOME */
    if (devPtr->devLnkMcastPtr != (ttVoidPtr)0)
    {
        tm_free_raw_buffer(devPtr->devLnkMcastPtr);
        devPtr->devLnkMcastPtr = (ttVoidPtr)0;
    }
    tm_kernel_free(devPtr);
    return;
}
#endif /* TM_USE_STOP_TRECK */
