/*
 * Copyright Notice:
 * Copyright Treck Incorporated  1997 - 2011
 * Copyright Zuken Elmic Japan   1997 - 2011
 * No portions or the complete contents of this file may be copied or
 * distributed in any form (including but not limited to printed or electronic
 * forms) without the expressed written consent of Treck Incorporated OR
 * Zuken Elmic.  Copyright laws and International Treaties protect the
 * contents of this file.  Unauthorized use is prohibited.
 * All rights reserved.
 *
 * Description: Stateless address auto-configuration for IPv6
 * IPv6 addresses are either manually configured, or auto-configured.
 * Auto-configuration is either stateless (i.e. generate addresses from the
 * interface ID and the link-local prefix, plus other prefixes that we discover
 * via Router Advertisements) or stateful (i.e. DHCPv6). Manually configured
 * IPv6 addresses are stored in the first TM_MAX_IPS_PER_IF slots of
 * ttDeviceEntry.dev6IpAddrArray, and auto-configured IPv6 addresses are stored
 * in the remaining TM_6_MAX_AUTOCONF_IPS_PER_IF slots. Stateless address
 * auto-configuration applies to hosts only. The only exception to this rule is
 * that routers can generate their own link-local addresses using stateless
 * address auto-configuration.
 * 
 * An IPv6 address that is in the process of being configured on an interface
 * is considered tentative, until Duplicate Address Detection confirms that a
 * different node isn't already using that address, at which point
 * tfDeviceStart is called to complete configuration of the address on the
 * interface. Duplicate Address Detection is initiated by calling
 * tf6DadConfigAddr for manually configured addresses, and
 * tf6DadAutoConfigPrefix for auto-configured addresses. The timer handler
 * tf6DadTimer handles retransmissions of Neighbor Solicitation probe messages
 * for Duplicate Address Detection.
 * 
 * Stateless address auto-configuration requires that an interface ID be set on
 * the interface, since stateless address auto-configuration uses the interface
 * ID as the host part of the IPv6 address. If no interface ID has been set on
 * the interface, then stateless address auto-configuration does not occur.
 * tf6ConfigInterfaceId is called to initiate stateless address
 * auto-configuration, which consists of forming a link-local scope IPv6
 * address from the interface ID ([RFC2462].R5.3:10), performing Duplicate
 * Address Detection on that address, and finally auto-configuring the
 * link-local address on the interface in one of the slots reserved for
 * auto-configured addresses. When the PPP link-layer closes or IPV6CP
 * renegotiates the interface ID, then the old interface ID must be
 * unconfigured, and all IPv6 addresses configured on the interface using
 * stateless address auto-configuration (i.e. using the interface ID) must be
 * unconfigured from the interface as well. tf6UnConfigInterfaceId is provided
 * for this purpose.
 * 
 * Before an interface can be considered "opened", it must have at least 1
 * link-local scope IPv6 address configured on it ([RFC2373].R2.1:10). Usually,
 * this link-local IP address is derived from the interface ID as part of
 * stateless address auto-configuration (i.e. tf6ConfigInterfaceId), however it
 * could also be manually configured on the interface. This process of
 * configuring the first link-local IPv6 address on the interface is somewhat
 * problematic, because we need to use it as the source IPv6 address to join
 * the solicited-node multicast group address corresponding to the interface ID
 * before we can complete the process of Duplicate Address Detection on the
 * link-local IPv6 address and configure it on the interface. In addition, if
 * Duplicate Address Detection on this link-local IPv6 address fails, we must
 * disable the IPv6 part of the interface. Disabling the IPv6 part of the
 * interface consists of stopping the process of stateless address
 * auto-configuration, and clearing the IPv6 device flag
 * TM_6_DEV_LINK_LOCAL_FLAG to indicate that no link-local scope IPv6 address
 * is configured or in the process of being configured on the interface. This
 * functionality is encapsulated in the internal API tf6DisableInterface (refer
 * to [IPV6SPEC]).
 * 
 * Every TM_6_PREFIX_TIMER_RES_MSEC milliseconds, tf6RtPrefixAgingTimer is
 * called to age preferred IPv6 addresses to deprecated to invalid. This aging
 * applies to all addresses on the interface, manually configured as well as
 * auto-configured ([RFC2462].R5.2:60). tf6RtPrefixAgingTimer locks the routing
 * tree so that it can walk and age all of the IPv6 local route entries in the
 * tree, and as a consequence of local route aging, transitions the state of
 * IPv6 address prefixes from preferred to deprecated (when
 * ttRteEntry.rte6HSPreferredLifetime becomes 0) to invalid (when
 * ttRteEntry.rteTtl becomes 0). The IPv6 addresses that have been manually and
 * auto-configured using the affected address prefixes go through the same
 * state transitions as the associated prefixes. If the user has registered a
 * callback using the dev6AddrNotifyFuncPtr parameter of tfNgConfigInterface,
 * then we notify the user when an address transitions state, however we want
 * to do this notification without any locks so that the user can call public
 * APIs in their callback function. Therefore, we cannot callback to the user
 * from tf6RtPrefixAgingTimer, because the routing tree is locked and must stay
 * locked until the tree walk has completed (since otherwise the routing tree
 * might change while we are in the middle of walking/traversing it, which
 * could cause problems). To workaround this, tf6RtPrefixAgingTimer creates a
 * new one-shot timer to do the user notification, and then notification occurs
 * in the context of another task without any locks. This timer is set to
 * expire immediately, and when it expires, it calls either
 * tf6RtDeprecatePrefixTimer or tf6RtInvalidatePrefixTimer to do all of the
 * work of deprecating or invalidating the address prefix and its associated
 * addresses, including deleting the local route entry from the routing table,
 * if the prefix has been invalidated.
 *
 * Filename: t6auto.c
 * Author: Ed Remmell
 * Date Created: 5/20/2002
 * $Source: source/t6auto.c $
 *
 * Modification History
 * $Revision: 6.0.2.5 $
 * $Date: 2010/11/24 03:56:05JST $
 * $Author: pcarney $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.11 $
 */

/*
 * Include
 */
#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_IPV6

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/* constant declarations and enumerations */

/* local function prototypes */
#ifdef TM_6_USE_PREFIX_DISCOVERY
/* Unconfigure the specified IPv6 address prefix from the specified
   interface. */
static void tf6UnConfigPrefix(
    ttDeviceEntryPtr devPtr, tt6IpAddressPtr prefixPtr );

/* Deprecate the specified IPv6 address prefix on the specified interface,
   which comprises deprecating all IPv6 addresses previously configured on
   the interface that have the specified address prefix. */
static void tf6DeprecatePrefix(
    ttDeviceEntryPtr devPtr, tt6IpAddressPtr prefixPtr );
#endif /* TM_6_USE_PREFIX_DISCOVERY */

#ifdef TM_6_USE_DAD
/* This function is called to create a new DAD entry for a tentative IPv6
   address, and insert the DAD entry into the linked-list of DAD entries
   chained off the interface. */
static tt6DadEntryPtr tf6DadAddEntry(
    ttDeviceEntryPtr devPtr, tt16Bit mHomeIndex );

/* This function is called during the process of Duplicate Address Detection
   when we have timed-out waiting for a solicited Neighbor Advertisement for
   our tentative address. This function retransmits the Neighbor Solicitation
   probe message up to (ttDeviceEntry.dev6DupAddrDetectTransmits - 1) times,
   each retransmission separated by ttDeviceEntry.dev6NeighSolicitReTxPeriod
   milliseconds ([RFC2462].R5.4.2:20). */
TM_NEARCALL static void tf6DadTimer(
    ttVoidPtr dadTimerPtr,
    ttGenericUnion userParm1,
    ttGenericUnion userParm2 );

/* This function starts the retransmission timer for a specified DAD entry. */
static void tf6DadStartTimer(
    ttDeviceEntryPtr devPtr,
    tt6DadEntryPtr dadEntryPtr );

/* This function stops the timer for a specified DAD entry. */
static void tf6DadStopTimer(
    ttDeviceEntryPtr devPtr, tt6DadEntryPtr dadEntryPtr );

/* Send a Neighbor Solicitation message for the purpose of Duplicate Address
   Detection. */
static int tf6DadCreateProbe(
    ttDeviceEntryPtr devPtr, tt16Bit mHomeIndex );
#endif /* TM_6_USE_DAD */

static int tf6DadDelayConfigAddr(
    ttDeviceEntryPtr devPtr,
    tt16Bit          mHomeIndex);

/* local variable definitions */

/* static const tables */

/* macro definitions */

/****************************************************************************
* FUNCTION: tf6ConfigInterfaceId
*
* PURPOSE: This function initiates stateless address auto-configuration, by
*   attempting to auto-configure a link-local IPv6 address on the specified
*   interface.
*
* PARAMETERS:
*   devPtr:
*       Pointer to the device/interface for which we want to initiate
*       stateless address auto-configuration.
*
* RETURNS:
*   0:
*       Success. A link-local IPv6 address is configured on the specified
*       interface.
*   TM_EINPROGRESS:
*       Duplicate Address Detection is in progress for a link-local IPv6
*       address on the interface.
*   TM_EPERM:
*       An interface ID must be set on the interface before initiating
*       stateless address auto-configuration.
*   TM_ENOBUFS:
*       Not enough memory available to process the request.
*
* NOTES:
*   An interface ID must already have been set on the interface before this
*   function is called. This function assumes that the device is locked.
*
****************************************************************************/
int tf6ConfigInterfaceId(
    ttDeviceEntryPtr devPtr )
{
/* local variables */
    tt6IpAddress                tempAddr;
#ifdef TM_6_USE_DAD
    tt6DadEntryPtr              dadEntryPtr;
#endif /* TM_6_USE_DAD */
    int                         errorCode;
    tt16Bit                     linkLocalMHomeIndex;
    tt16Bit                     mHomeIndex;

    tm_trace_func_entry(tf6ConfigInterfaceId);

    errorCode = TM_ENOERROR; /* assume success */
    
    tm_assert_is_locked(tf6ConfigInterfaceId, &(devPtr->devLockEntry) );

/* If an interface ID has not been set on the interface, then return
   TM_EPERM */
    if (!(devPtr->dev6Flags & TM_6_DEV_INTERFACE_ID_FLAG))
    {
        errorCode = TM_EPERM;
        goto exit6ConfigInterfaceId;
    }

/* If a link-local IPv6 address is configured or is pending configuration on
   the interface, then return 0 or TM_EINPROGRESS respectively */
    if (devPtr->dev6AddrCacheFlags & TM_6_DEV_LINK_LOCAL_FLAG)
    {
        mHomeIndex = devPtr->dev6LinkLocalMhomeIndex;
        if (!(devPtr->dev6IpAddrFlagsArray[mHomeIndex] & TM_6_DEV_IP_CONFIG))
        {
/* IPv6 address is pending configuration on the interface */
            tm_assert(tf6ConfigInterfaceId,
                      devPtr->dev6IpAddrFlagsArray[
                          mHomeIndex] &
                      TM_6_DEV_IP_CONF_STARTED);
            errorCode = TM_EINPROGRESS;
        }

        goto exit6ConfigInterfaceId;
    }

/* Initiate stateless address auto-configuration to configure a link-local
   IPv6 address on the interface */
/* Construct the link-local address to configure from the interface ID.
   ([RFC2462].R5.3:30) */
/* A link-local unicast IPv6 address is formed by prepending the well-known
   link-local prefix FE80::0 (of appropriate length) to the interface
   identifier. We implement a 64-bit interface identifier, so the link-local
   prefix is always 64 bits in length. ([RFC2373].R2.4:20) */
#ifdef TM_LITTLE_ENDIAN
    tempAddr.s6LAddr[0] = TM_UL(0x000080FE);
#else /* TM_BIG_ENDIAN */
    tempAddr.s6LAddr[0] = TM_UL(0xFE800000);
#endif /* TM_BIG_ENDIAN */
    tempAddr.s6LAddr[1] = TM_UL(0);
    tempAddr.s6LAddr[2] = devPtr->dev6InterfaceId[0];
    tempAddr.s6LAddr[3] = devPtr->dev6InterfaceId[1];


/* Auto-configure the link-local address on the interface */
    linkLocalMHomeIndex = (tt16Bit) TM_MAX_IPS_PER_IF;
    tm_assert(tf6ConfigInterfaceId,
              !(devPtr->dev6IpAddrFlagsArray[
                    linkLocalMHomeIndex] &
                (TM_6_DEV_IP_CONF_STARTED | TM_6_DEV_IP_CONFIG)));
    if (linkLocalMHomeIndex == devPtr->dev6MhomeUsedAutoEntries)
    {
        devPtr->dev6MhomeUsedAutoEntries++;
    }
    tm_6_ip_copy(
        &tempAddr, &devPtr->dev6IpAddrArray[linkLocalMHomeIndex]);
    devPtr->dev6PrefixLenArray[linkLocalMHomeIndex] = 64;

/* Initiate Duplicate Address Detection on this link-local address:
   ([RFC2462].R5.4:10) */
    devPtr->dev6IpAddrFlagsArray[linkLocalMHomeIndex]
        |= TM_6_DEV_IP_CONF_STARTED;
    devPtr->dev6AddrCacheFlags |= TM_6_DEV_LINK_LOCAL_FLAG;
    devPtr->dev6LinkLocalMhomeIndex = linkLocalMHomeIndex;

#ifdef TM_6_USE_DAD
    dadEntryPtr = TM_6_DAD_ENTRY_NULL_PTR;
    if (devPtr->dev6DupAddrDetectTransmits != 0)
    {
        dadEntryPtr = tf6DadAddEntry(devPtr, linkLocalMHomeIndex);

        if (dadEntryPtr == TM_6_DAD_ENTRY_NULL_PTR)
        {
            errorCode = TM_ENOBUFS;
            goto exit6ConfigInterfaceId;
        }
    }
#endif /* TM_6_USE_DAD */

    errorCode = tf6DadDelayConfigAddr(devPtr,
                                      linkLocalMHomeIndex);

exit6ConfigInterfaceId:
    tm_trace_func_exit_rc(tf6ConfigInterfaceId, errorCode);
    return errorCode;
} 

/****************************************************************************
* FUNCTION: tf6UnConfigInterfaceId
*
* PURPOSE: This function unconfigures the interface ID from the specified
*   interface, which includes unconfiguring all IPv6 addresses previously
*   configured on the interface using stateless address auto-configuration
*   (i.e. using the interface ID).
*
* PARAMETERS:
*   devPtr:
*       Pointer to the device/interface for which we want to unconfigure the
*       interface ID.
*
* RETURNS:
*   0:
*       Success
*   TM_ENOENT:
*       No interface ID was found on the specified interface.
*
* NOTES:
*   This function assumes that the device is locked.
*
****************************************************************************/
int tf6UnConfigInterfaceId(
    ttDeviceEntryPtr devPtr )
{
/* local variables */
    int                 errorCode;
    int                 mHomeIndex;
#ifdef TM_6_USE_DAD
    tt6DadEntryPtr      dadEntryPtr;
#endif /* TM_6_USE_DAD */

    tm_trace_func_entry(tf6UnConfigInterfaceId);

    errorCode = TM_ENOERROR; /* assume success */
    
    tm_assert_is_locked(tf6UnConfigInterfaceId, &(devPtr->devLockEntry));

/* If an interface ID has not been set on the interface, then return
   TM_ENOENT */
    if (!(devPtr->dev6Flags & TM_6_DEV_INTERFACE_ID_FLAG))
    {
        errorCode = TM_ENOENT;
        goto exit6UnConfigInterfaceId;
    }

/* Unconfigure all auto-configured addresses */
    for (mHomeIndex = TM_MAX_IPS_PER_IF;
         mHomeIndex <
             (TM_MAX_IPS_PER_IF + TM_6_MAX_AUTOCONF_IPS_PER_IF);
         mHomeIndex++)
    {
        if (devPtr->dev6IpAddrFlagsArray[mHomeIndex] &
            TM_6_DEV_IP_CONFIG)
        {
            errorCode = tfRemoveInterface(
                devPtr, (tt16Bit) mHomeIndex, PF_INET6);
        }
#ifdef TM_6_USE_DAD
        else if (devPtr->dev6IpAddrFlagsArray[mHomeIndex] &
                 TM_6_DEV_IP_CONF_STARTED)
        {
            dadEntryPtr = tf6DadFindEntry(
                devPtr, &devPtr->dev6IpAddrArray[mHomeIndex]);
            tf6DadFailed(
                devPtr, (tt16Bit) mHomeIndex, dadEntryPtr);
        }
#endif /* TM_6_USE_DAD */
    }

 exit6UnConfigInterfaceId:
    tm_trace_func_exit_rc(tf6UnConfigInterfaceId, errorCode);
    return errorCode;
}

/****************************************************************************
* FUNCTION: tf6DadConfigAddr
*
* PURPOSE: This function initiates manual configuration of the specified IPv6
*   address on the specified interface at the specified multi-home index. If
*   Duplicate Address Detection is enabled on the interface (i.e.
*   ttDeviceEntry.dev6DupAddrDetectTransmits is not 0), then the address will
*   be put in the state TM_6_DEV_IP_CONF_STARTED pending successful
*   completion of Duplicate Address Detection, and this API returns
*   TM_EINPROGRESS. If Duplicate Address Detection is disabled, then this API
*   completes configuration of the address on the interface, and returns 0.
*
* PARAMETERS:
*   devPtr
*       Pointer to the device that we are configuring the address on.
*   mHomeIndex
*       The multi-home index on the interface where we are configuring the
*       address.
*   ipAddrPtr
*       Pointer to the IPv6 address that we are manually configuring.
*   prefixLen
*       The length (in bits) of the subnet prefix part of the address
*       specified by ipAddrPtr.
*
* RETURNS:
*   0:
*       The address has been successfully configured.
*   TM_EINPROGRESS:
*       Configuration of the address has been initiated; Duplicate Address
*       Detection is in progress.
*   TM_EADDRINUSE:
*       Duplicate Address Detection failed, this address is already in use by a
*       different node.
*   TM_EINVAL:
*       Invalid value specified for prefixLen.
*   TM_EADDRNOTAVAIL:
*       Attempted to configure the device with a multicast address, the
*       unspecified address, the loopback address or an IPv4-only IPv6 address.
*   TM_EPERM:
*       An interface ID must be set on the interface before initiating
*       stateless address auto-configuration.
*   TM_ENOBUFS:
*       Not enough memory available to process the request.
*
* NOTES:
*   This function assumes that the device is locked.
*
****************************************************************************/
int tf6DadConfigAddr(
    ttDeviceEntryPtr devPtr,
    tt16Bit          mHomeIndex,
    tt6IpAddressPtr  ipAddrPtr,
    tt8Bit           prefixLen )
{
/* local variables */
    int                         errorCode;
#ifdef TM_6_USE_DAD
    tt6DadEntryPtr              dadEntryPtr;
    tt6IpAddressPtr             linkLocalAddrPtr;
    ttArpMapping                arpMapping;
    tt16Bit                     linkLocalMhomeIndex;
#if (defined(TM_6_USE_MIP_MN) && !defined(TM_6_MN_DISABLE_HOME_DETECT))
    tt16Bit                     vMhomeIndex;
    tt8Bit                      mnDeregHomeAddrStatus;
#endif /* TM_6_USE_MIP_MN && !TM_6_MN_DISABLE_HOME_DETECT */
#endif /* TM_6_USE_DAD */

    tm_trace_func_entry(tf6DadConfigAddr);

    errorCode = TM_ENOERROR; /* assume success */
#ifdef TM_6_USE_DAD
#if (defined(TM_6_USE_MIP_MN) && !defined(TM_6_MN_DISABLE_HOME_DETECT))
    mnDeregHomeAddrStatus = TM_8BIT_NO;
#endif /* TM_6_USE_MIP_MN && ! TM_6_MN_DISABLE_HOME_DETECT */
#endif /* TM_6_USE_DAD */

    tm_assert_is_locked(tf6DadConfigAddr, &(devPtr->devLockEntry) );

    if (devPtr->dev6IpAddrFlagsArray[mHomeIndex]
        & TM_6_DEV_IP_CONFIG)
    {
/* there is already an IPv6 address configured on this device at this
   multi-home index, so return success */
        goto exit6DadConfigAddr;
    }

    if (IN6_IS_ADDR_UNSPECIFIED(ipAddrPtr))
    {
/* when configured with an IP address that is all 0's, don't actually
   configure, but start auto-configuration */
        errorCode = tf6ConfigInterfaceId(devPtr);
        goto exit6DadConfigAddr;
    }
    else
    {
        errorCode = tf6ValidConfigAddr(ipAddrPtr, (int) prefixLen);
        if (errorCode != TM_ENOERROR)
        {
            goto exit6DadConfigAddr;
        }
    }

    if (devPtr->dev6IpAddrFlagsArray[mHomeIndex]
        & TM_6_DEV_IP_CONF_STARTED)
    {
/* there is already an IPv6 address pending configuration on this device at
   this multi-home index, so return TM_EINPROGRESS */
        errorCode = TM_EINPROGRESS;
        goto exit6DadConfigAddr;
    }

#ifdef TM_6_USE_DAD
#if (defined(TM_6_USE_MIP_MN) && !defined(TM_6_MN_DISABLE_HOME_DETECT))
/* check if we are attempting to configure our home address on the mobile
   interface while at home */
    if (tm_8bit_one_bit_set(
            tm_context(tv6MnVect).mns6Flags, TM_6_MNS_AT_HOME))
    {
        for (vMhomeIndex = 0;
             vMhomeIndex < TM_6_MAX_LOCAL_MHOME;
             vMhomeIndex++)
        {
/* if we are in the process of deregistering our home address: */
            if (tm_8bit_one_bit_set(
                    tm_context(tv6MnVect).mns6VirtHomeIfacePtr->
                    dev6IpAddrFlagsArray[vMhomeIndex],
                    TM_6_DEV_MIP_REGISTERED_FLAG)
                && tm_6_ip_match(
                    ipAddrPtr, &tm_6_ip_dev_addr(
                        tm_context(tv6MnVect).mns6VirtHomeIfacePtr,
                        vMhomeIndex)) )
            {
/* to support being able to send the deregistration BU with the source
   address set to our home address, we skip performing DAD on our
   home address when at home and in the process of deregistering */
                mnDeregHomeAddrStatus = TM_8BIT_YES;
                break;
            }
        }
    }
#endif /* TM_6_USE_MIP_MN && ! TM_6_MN_DISABLE_HOME_DETECT */
#endif /* TM_6_USE_DAD */

#ifdef TM_6_USE_DAD
#if (defined(TM_6_USE_MIP_MN) && !defined(TM_6_MN_DISABLE_HOME_DETECT))
    if (mnDeregHomeAddrStatus == TM_8BIT_NO)
#endif /* TM_6_USE_MIP_MN && ! TM_6_MN_DISABLE_HOME_DETECT */
    {
/* If there is an existing Neighbor Cache entry for the local IPv6
   address we are trying to configure, then delete it */
#ifdef TM_USE_STRONG_ESL
        arpMapping.arpmDevPtr = devPtr;
#endif /* TM_USE_STRONG_ESL */
        tm_6_ip_copy_dststruct(ipAddrPtr, arpMapping.arpm6IpAddress);
        arpMapping.arpmCommand = TM_6_ARP_DEL_BY_IP;
        tm_6_dev_scope_addr(&arpMapping.arpm6IpAddress, devPtr);
        (void) tfRtArpCommand(&arpMapping);
    }
#endif /* TM_6_USE_DAD */

/* Set the specified IP address and prefix length on the interface */
    tm_6_ip_copy(ipAddrPtr, &devPtr->dev6IpAddrArray[mHomeIndex]);
    devPtr->dev6PrefixLenArray[mHomeIndex] = prefixLen;

    errorCode = tf6ConfigInterfaceId(devPtr);
    if (errorCode == TM_EPERM)
    {
        if (    IN6_IS_ADDR_LINKLOCAL(ipAddrPtr)
            && !(devPtr->dev6AddrCacheFlags & TM_6_DEV_LINK_LOCAL_FLAG)
           )
        {
/* If the IPv6 address is link-local (i.e. IN6_IS_ADDR_LINKLOCAL) and no
   link-local IPv6 address is configured or pending configuration on the
   interface, then indicate that the interface has a link-local IPv6 address
   pending configuration */
            devPtr->dev6LinkLocalMhomeIndex = mHomeIndex;
            devPtr->dev6AddrCacheFlags |= TM_6_DEV_LINK_LOCAL_FLAG;
        }
        else
        {
            goto exit6DadConfigAddr;
        }
    }

    errorCode = TM_ENOERROR; /* assume success */
    devPtr->dev6IpAddrFlagsArray[mHomeIndex] |=
        TM_6_DEV_IP_CONF_STARTED;


#ifdef TM_6_USE_DAD
    dadEntryPtr = TM_6_DAD_ENTRY_NULL_PTR;
/* Perform Duplicate Address Detection on the IPv6 address
   ([RFC2462].R5.4:10) */
    if (   (devPtr->dev6DupAddrDetectTransmits != 0)
#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
        && (!(IN6_IS_ADDR_V4COMPAT(ipAddrPtr)))
#endif /* (defined(TM_USE_IPV6) && defined(TM_USE_IPV4)) */
#if (defined(TM_6_USE_MIP_MN) && !defined(TM_6_MN_DISABLE_HOME_DETECT))
        && (mnDeregHomeAddrStatus == TM_8BIT_NO)
#endif /* TM_6_USE_MIP_MN && ! TM_6_MN_DISABLE_HOME_DETECT */
        )
    {
        linkLocalMhomeIndex = devPtr->dev6LinkLocalMhomeIndex;
        linkLocalAddrPtr =
            &(devPtr->dev6IpAddrArray[linkLocalMhomeIndex]);

        if ((mHomeIndex >= TM_MAX_IPS_PER_IF)
            && (mHomeIndex
                < (TM_MAX_IPS_PER_IF + TM_6_MAX_AUTOCONF_IPS_PER_IF))
            && (devPtr->dev6Flags & TM_6_DEV_OPTIMIZE_DAD)
            && (  devPtr->dev6IpAddrFlagsArray[linkLocalMhomeIndex]
                & (TM_6_DEV_IP_CONF_STARTED | TM_6_DEV_IP_CONFIG))
            && (ipAddrPtr->s6LAddr[2] == linkLocalAddrPtr->s6LAddr[2])
            && (ipAddrPtr->s6LAddr[3] == linkLocalAddrPtr->s6LAddr[3]))
        {
/* if the address is being auto-configured, and the user wants to optimize
   Duplicate Address Detection, and both the address being auto-configured
   and the link-local address match on the low-order 64 bits, then skip
   Duplicate Address Detection ([RFC2462].R5.4:30) */
            errorCode = TM_ENOERROR;
        }
        else
        {
/* start Duplicate Address Detection on the IPv6 address */
            dadEntryPtr = tf6DadAddEntry(devPtr, mHomeIndex);

            if (dadEntryPtr == TM_6_DAD_ENTRY_NULL_PTR)
            {
                errorCode = TM_ENOBUFS;
                goto exit6DadConfigAddr;
            }
        }
    }
#endif /* TM_6_USE_DAD */

    errorCode = tf6DadDelayConfigAddr(devPtr,
                                      mHomeIndex);

exit6DadConfigAddr:
    tm_trace_func_exit_rc(tf6DadConfigAddr, errorCode);
    return errorCode;
}

static int tf6DadDelayConfigAddr(
    ttDeviceEntryPtr devPtr,
    tt16Bit          mHomeIndex)
{
    tt6IpAddressPtr             ipAddrPtr;
    tt6IpAddress                tempAddr;
#if defined(TM_6_USE_PREFIX_DISCOVERY) || defined(TM_6_USE_NUD)
    ttLinkLayerEntryPtr         devLinkLayerProtocolPtr;
    ttTimerPtr                  timerPtr;
    tt32Bit                     initDelay;
    ttGenericUnion              timerParm1;
    ttGenericUnion              timerParm2;    
#endif /* defined(TM_6_USE_PREFIX_DISCOVERY) || defined(TM_6_USE_NUD) */
    int                         errorCode;

    ipAddrPtr = &(devPtr->dev6IpAddrArray[mHomeIndex]);
/* Add the group so that we can receive the multicast packets */
    tm_6_addr_to_sol_node(ipAddrPtr, &tempAddr);
    (void)tf6MldAddGroup(devPtr, &tempAddr
#ifdef TM_6_USE_MLDV2
                        ,
                        TM_6_MODE_IS_EXCLUDE, TM_6_SRC_ADDR_ENTRY_NULL_PTR
#endif /* TM_6_USE_MLDV2 */
                     );
#if defined(TM_6_USE_PREFIX_DISCOVERY) || defined(TM_6_USE_NUD)
/*      b. If the link-layer is now fully open (i.e. link-layer open function
    returned TM_ENOERROR), then start a one-shot timer to delay sending
    the initial Router/Neighbor Solicitation message as specified by
    requirements [RFC2461].R6.3.7:50 and [RFC2462].R5.4.2:30 (refer to
    tf6DelayInitSolTimer), specifying a random timer period in the range
    of 0 to TM_6_ND_MAX_RTR_SOLICIT_DLY. */
    if (   (tm_16bit_bits_not_set(devPtr->dev6Flags,
                                TM_6_DEV_INIT_DELAY_COMPLETE))
#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
         && (!(IN6_IS_ADDR_V4COMPAT(ipAddrPtr)))
#endif /* (defined(TM_USE_IPV6) && defined(TM_USE_IPV4)) */
         && (!(tm_ll_is_pt2pt(devPtr))) )
    {
        errorCode = TM_EINPROGRESS;

        if (devPtr->dev6InitDelayTimerPtrArr[mHomeIndex]
                                  == (ttTimerPtr)0)
        {
            timerPtr = devPtr->dev6InitDelayTimerPtrArr[0];
            if (timerPtr == (ttTimerPtr)0)
            {
                timerPtr = devPtr->dev6InitDelayTimerPtrArr
                                                [TM_MAX_IPS_PER_IF];
            }
            if (timerPtr != (ttTimerPtr)0)
            {
                initDelay = tfTimerMsecLeft(timerPtr);
            }
            else
            {
                devLinkLayerProtocolPtr = devPtr->devLinkLayerProtocolPtr;
                if (devLinkLayerProtocolPtr->lnk6McastFuncPtr
                    != TM_6_LL_MCAST_FUNC_NULL_PTR)
                {
                    (void)(*(devLinkLayerProtocolPtr->lnk6McastFuncPtr))(
                        (ttVoidPtr)devPtr, &in6addr_linklocal_allnodes,
                        TM_LNK_ADD_MCAST);
                }
/* Random time between 0, and MAX_RTR_SOLICITATION_DELAY (1 second) */
                initDelay = tfGetRandom() % TM_6_SOL_DEF_INIT_DELAY;
                if (initDelay < (TM_6_SOL_DEF_INIT_DELAY/2))
                {
/* Make sure we delay by at least 1/2 second */
                    initDelay = initDelay + TM_6_SOL_DEF_INIT_DELAY/2;
                }
            }
            timerParm1.genVoidParmPtr = (ttVoidPtr) devPtr;
            timerParm2.gen16BitParm = (tt16Bit) mHomeIndex;
            devPtr->dev6InitDelayTimerPtrArr[mHomeIndex] =
                                tfTimerAdd(
                                    tf6DelayInitSolTimer,
                                    timerParm1,
                                    timerParm2,
                                    initDelay,
                                    TM_TIM_AUTO);
        }
    }
    else
#endif /* (TM_6_USE_PREFIX_DISCOVERY) || defined(TM_6_USE_NUD) */
/*
* We should bypass the initial delay, and configure the device immediately.
* This flag could be set when configuring the loopback interface.
*/ 
    {
        errorCode = tf6DadFinishConfigAddr(devPtr,
                                           mHomeIndex);
    }
    return errorCode;
}

int tf6DadFinishConfigAddr (
    ttDeviceEntryPtr devPtr,
    tt16Bit          mHomeIndex)
{
    tt6IpAddressPtr             ipAddrPtr;
#ifdef TM_6_USE_DAD
    tt6DadEntryPtr              dadEntryPtr;
    tt6IpAddress                scopeIpAddr;
#endif /* TM_6_USE_DAD */
    struct sockaddr_storage     tempSockAddr;
#ifdef TM_6_USE_MLD
    tt6IpAddress                tempAddr;
#endif /* TM_6_USE_MLD */
#ifdef TM_DEBUG_LOGGING
    char                        buffer[INET6_ADDRSTRLEN];
#endif /* TM_DEBUG_LOGGING */
    int                         errorCode;
    tt8Bit                      initDelay;

    errorCode = TM_ENOERROR;
    ipAddrPtr = &(devPtr->dev6IpAddrArray[mHomeIndex]);
    if (devPtr->dev6AddrNotifyFuncPtr
        != TM_6_DEV_ADDR_NOTIFY_FUNC_NULL_PTR)
    {
#ifdef TM_DEBUG_LOGGING
        inet_ntop(AF_INET6,
                  ipAddrPtr,
                  buffer,
                  INET6_ADDRSTRLEN);
        tm_debug_log2(
            "TM_6_DEV_ADDR_CONFIG_STARTED: mHome=%d, %s",
            mHomeIndex,
            buffer);
#endif /* TM_DEBUG_LOGGING */

/* if the user registered a callback, then notify the user of the event
   TM_6_DEV_ADDR_CONFIG_STARTED */
/* first, put the address in the format that the user expects */
        tf6SetSockAddrFromIpAddr(
            ipAddrPtr,
            devPtr, &tempSockAddr.addr.ipv6);

        tm_call_unlock(&(devPtr->devLockEntry));

        (*devPtr->dev6AddrNotifyFuncPtr)
            ((ttUserInterface) devPtr,
             (unsigned int) mHomeIndex,
             &tempSockAddr,
             TM_6_DEV_ADDR_CONFIG_STARTED);

        tm_call_lock_wait(&(devPtr->devLockEntry));
        if (    ((devPtr->devFlag & TM_DEV_OPENED) == 0)
             || (!(devPtr->dev6IpAddrFlagsArray[mHomeIndex]
                   & TM_6_DEV_IP_CONF_STARTED)) )
        {
/*
 * Recheck because device was unlocked.
 * Device has been closed, or mHome unconfigured, so dadEntryPtr has been freed
 */
            errorCode = TM_ESHUTDOWN;
        }
    }
    if (errorCode == TM_ENOERROR)
    {
/*
 * If this is the first address being configured on this interface, join the
 * all-nodes link-local multicast group and start sending initial router
 * solicitations.
 */ 
        if (   (tm_16bit_bits_not_set(devPtr->dev6Flags,
                                  TM_6_DEV_INIT_DELAY_COMPLETE))
#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
            && (!(IN6_IS_ADDR_V4COMPAT(ipAddrPtr)))
#endif /* (defined(TM_USE_IPV6) && defined(TM_USE_IPV4)) */
           )
        {
            devPtr->dev6Flags |= TM_6_DEV_INIT_DELAY_COMPLETE;
            initDelay = TM_8BIT_YES;
        }
        else
        {
            initDelay = TM_8BIT_NO;
        }
#ifdef TM_6_USE_MLD
#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
        if (!(IN6_IS_ADDR_V4COMPAT(ipAddrPtr)))
#endif /* (defined(TM_USE_IPV6) && defined(TM_USE_IPV4)) */
        {
/* Join the solicited-node multicast address corresponding to the IPv6
   address by calling tf6MldJoinGroup ([RFC2462].R5.4.2:10) */
            tm_6_addr_to_sol_node(ipAddrPtr, &tempAddr);
            (void)tf6MldJoinGroup(TM_8BIT_NO, devPtr, &tempAddr
#ifdef TM_6_USE_MLDV2
                        ,
                        TM_6_MODE_IS_EXCLUDE,
                        TM_6_SRC_ADDR_ENTRY_NULL_PTR,
                        (tt8Bit)0
#endif /* TM_6_USE_MLDV2 */
                    );
        }
#else /* !TM_6_USE_MLD */
/* Group already added in tf6DadDelayConfigAddr() */
#endif /* TM_6_USE_MLD */
#ifdef TM_6_USE_DAD
        dadEntryPtr = tf6DadFindEntry(devPtr, ipAddrPtr);
        if (dadEntryPtr != TM_6_DAD_ENTRY_NULL_PTR)
        {
            errorCode = TM_EINPROGRESS;
            if (mHomeIndex == TM_MAX_IPS_PER_IF)
            {
                tm_6_ip_copy(ipAddrPtr, &scopeIpAddr);
/* Embed interface index or site ID in link-local or site-local addresses */
                tm_6_dev_scope_addr(&scopeIpAddr, devPtr);
/* add the local route for the link-local scope prefix */
                (void) tf6RtAddLocal(
                    devPtr,
                    &scopeIpAddr,       /* key */
                    64,                 /* prefix length */
                    &scopeIpAddr,
                    mHomeIndex);
            }
/* Start the timer to wait for the solicited Neighbor Advertisement response */
            tf6DadStartTimer(devPtr, dadEntryPtr);
/* Send a Neighbor Solicitation for our tentative address */
            tf6DadCreateProbe(devPtr, mHomeIndex);
#ifdef TM_6_USE_PREFIX_DISCOVERY
            if (mHomeIndex == TM_MAX_IPS_PER_IF)
            {
                if (initDelay)
                {
                    tf6InitRtrSol(devPtr);
                }
            }
#endif /* TM_6_USE_PREFIX_DISCOVERY */
        }
        else
#endif /* TM_6_USE_DAD */
        {
/* Duplicate Address Detection is disabled */
#ifdef TM_6_USE_MLD
            if (!(devPtr->dev6IpAddrFlagsArray[mHomeIndex]
                  & TM_6_DEV_IP_CONF_STARTED))
/*
 * Recheck because device unlocked in tf6MldJoinGroup
 */
            {
/* mhome has been unconfigured */
                errorCode = TM_ESHUTDOWN;
            }
            else
#endif /* TM_6_USE_MLD */
            {
                errorCode = tfDeviceStart(devPtr, mHomeIndex, AF_INET6);
#ifdef TM_6_USE_PREFIX_DISCOVERY
                if (initDelay)
                {
                    tf6InitRtrSol(devPtr);
                }
#endif /* TM_6_USE_PREFIX_DISCOVERY */
            }
        }
    }
    return errorCode;
}

#ifdef TM_6_USE_PREFIX_DISCOVERY
/****************************************************************************
* FUNCTION: tf6DadAutoConfigPrefix
*
* PURPOSE: This function initiates stateless address auto-configuration of
*   the specified IPv6 address prefix on the specified interface. The address
*   prefix is assumed to be 64 bits in length ([RFC2373].R2.4:20,
*   [RFC2462].R5.5.3:80). If Duplicate Address Detection is enabled on the
*   interface (i.e. ttDeviceEntry.dev6DupAddrDetectTransmits is not 0), then
*   the address formed from the combination of the address prefix and the
*   interface ID is put in the state TM_6_DEV_IP_CONF_STARTED pending
*   successful completion of Duplicate Address Detection, and this API
*   returns TM_EINPROGRESS. If Duplicate Address Detection is disabled, then
*   this API completes configuration of the address on the interface, and
*   returns 0.
*
* PARAMETERS:
*   devPtr:
*       Pointer to the device that we are configuring the address on.
*   prefixPtr:
*       Pointer to the IPv6 address prefix that we are auto-configuring using
*       stateless address auto-configuration.
*
* RETURNS:
*   0:
*       The address has been successfully configured.
*   TM_EINPROGRESS:
*       Configuration of the address has been initiated; Duplicate Address
*       Detection is in progress.
*   TM_EADDRINUSE:
*       Duplicate Address Detection failed, this address is already in use by a
*       different node.
*   TM_ENOSPC:
*       No available slots in the IPv6 address list for auto-configuration.
*   TM_EPERM:
*       An interface ID must be set on the interface before attempting
*       stateless address auto-configuration. The link-local scope IPv6
*       address must be auto-configured first, before any prefix can
*       be auto-configured.
*
* NOTES:
*   This function assumes that the device is locked.
*
****************************************************************************/
int tf6DadAutoConfigPrefix(
    ttDeviceEntryPtr devPtr,
    tt6IpAddressPtr prefixPtr )
{
/* local variables */
    int                 errorCode;
    int                 mHomeIndex;
    tt6IpAddress        tempAddr;

    tm_trace_func_entry(tf6DadAutoConfigPrefix);

    errorCode = TM_ENOERROR; /* assume successful */

    tm_assert_is_locked(tf6DadAutoConfigPrefix, &(devPtr->devLockEntry) );

/* If an interface ID has not been set on the interface, or the link-local
   scope IPv6 address hasn't been successfully auto-configured or isn't in
   progress of being auto-configured (bug ID# 412), then return TM_EPERM.
   NOTE: we take advantage of the fact that the auto-configured
   link-local scope IPv6 address is always the first one auto-configured,
   therefore it must be in the slot TM_MAX_IPS_PER_IF. */
    if (!(devPtr->dev6Flags & TM_6_DEV_INTERFACE_ID_FLAG)
        || !(tm_8bit_one_bit_set(
                 devPtr->dev6IpAddrFlagsArray[TM_MAX_IPS_PER_IF],
                 (TM_6_DEV_IP_CONFIG | TM_6_DEV_IP_CONF_STARTED))))
    {
        errorCode = TM_EPERM;
        goto exit6DadAutoConfigPrefix;
    }

/* If the specified prefix is the link-local prefix, then ignore it and
   return 0. ([RFC2462].R5.5.3:60) */
    if (IN6_IS_ADDR_LINKLOCAL(prefixPtr))
    {
        goto exit6DadAutoConfigPrefix;
    }

    if (tm_context(tv6RtPrefixAgingTimerPtr) == TM_TMR_NULL_PTR)
    {
        tf6RtStartPrefixAgingTimer();
    }

/* If an IPv6 address with the same prefix is configured or is pending
   configuration on the interface in the slots reserved for auto-configured
   addresses, then return 0 or TM_EINPROGRESS respectively */
    for (mHomeIndex = TM_MAX_IPS_PER_IF;
         mHomeIndex < (TM_MAX_IPS_PER_IF + TM_6_MAX_AUTOCONF_IPS_PER_IF);
         mHomeIndex++)
    {
        if ((prefixPtr->ip6Addr.ip6U32[0]
             == devPtr->dev6IpAddrArray[mHomeIndex].ip6Addr.ip6U32[0])
            && (prefixPtr->ip6Addr.ip6U32[1]
                == devPtr->dev6IpAddrArray[
                    mHomeIndex].ip6Addr.ip6U32[1]))
        {
/* If we found a prefix match (Note: we assume prefix length is 64 bits) */
            if (devPtr->dev6IpAddrFlagsArray[mHomeIndex] &
                TM_6_DEV_IP_CONFIG)
            {
/* if it is configured, return TM_ENOERROR */
                goto exit6DadAutoConfigPrefix;
            }
            else
            {
                if (devPtr->dev6IpAddrFlagsArray[mHomeIndex] &
                    TM_6_DEV_IP_CONF_STARTED)
                {
/* else, if it is pending configuration, return TM_EINPROGRESS */
                    errorCode = TM_EINPROGRESS;
                    goto exit6DadAutoConfigPrefix;
                }
            }
        }
    }

/* Get the multi-home index of an available slot in the IPv6 address list
   for auto-configuration */
    for (mHomeIndex = TM_MAX_IPS_PER_IF;
         mHomeIndex < (TM_MAX_IPS_PER_IF + TM_6_MAX_AUTOCONF_IPS_PER_IF);
         mHomeIndex++)
    {
        if (tm_8bit_bits_not_set(
                devPtr->dev6IpAddrFlagsArray[mHomeIndex],
                (TM_6_DEV_IP_CONFIG | TM_6_DEV_IP_CONF_STARTED)))
        {
            break;
        }
    }

/* If there is no available slot, then return TM_ENOSPC. */
    if (mHomeIndex == (TM_MAX_IPS_PER_IF + TM_6_MAX_AUTOCONF_IPS_PER_IF))
    {
        errorCode = TM_ENOSPC;
        goto exit6DadAutoConfigPrefix;
    }

/* Construct the IPv6 address to auto-configure from the specified prefix
   and the interface ID. */
    tempAddr.s6LAddr[0] = prefixPtr->s6LAddr[0];
    tempAddr.s6LAddr[1] = prefixPtr->s6LAddr[1];
    tempAddr.s6LAddr[2] = devPtr->dev6InterfaceId[0];
    tempAddr.s6LAddr[3] = devPtr->dev6InterfaceId[1];

#ifdef TM_6_USE_DAD
/*
 * We don't want to configure this new prefix address if it is already
 * configured (via DHCPv6, manual IPv6 configuration, etc.)
 */
    errorCode = tf6FindAddrInDevList(devPtr, &tempAddr);
    if (errorCode != TM_ENOERROR)
    {
        goto exit6DadAutoConfigPrefix;
    }
#endif /* TM_6_USE_DAD */

/* ([RFC2462].R5.5.3:90) */
    errorCode = tf6DadConfigAddr(
        devPtr, (tt16Bit) mHomeIndex, &tempAddr, (tt8Bit) 64);

 exit6DadAutoConfigPrefix: 
    tm_trace_func_exit_rc(tf6DadAutoConfigPrefix, errorCode);
    return errorCode;
}
#endif /* TM_6_USE_PREFIX_DISCOVERY */

/****************************************************************************
* FUNCTION: tf6DadConfigComplete
*
* PURPOSE: This function is called when Duplicate Address Detection has
*   successfully completed for our tentative address, and after tfDeviceStart
*   has completed the configuration of the address on the interface.
*
* PARAMETERS:
*   devPtr:
*       Pointer to the device that we configured the address on.
*   mHomeIndex:
*       The multi-home index on the interface where we configured the address.
*
* RETURNS:
*   Nothing
*
* NOTES:
*   This function assumes that the device is locked.
*
****************************************************************************/
void tf6DadConfigComplete(
    ttDeviceEntryPtr devPtr,
    tt16Bit mHomeIndex )
{
/* local variables */
    struct sockaddr_storage tempSockAddr;
#ifdef TM_6_USE_PREFIX_DISCOVERY
    tt6RtKeyCommand         rtKeyCommand;
    int                     errorCode;
#endif /* TM_6_USE_PREFIX_DISCOVERY */
#ifdef TM_6_USE_DAD
    tt6DadEntryPtr          dadEntryPtr;
#endif /* TM_6_USE_DAD */
#ifdef TM_DEBUG_LOGGING
    char                    buffer[INET6_ADDRSTRLEN];
#endif /* TM_DEBUG_LOGGING */
#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
#if defined(TM_6_USE_NUD) || defined(TM_USE_RFC3590)
    tt6IpAddressPtr         ipAddrPtr;
#endif /* defined(TM_6_USE_NUD) || defined(TM_USE_RFC3590) */
#endif /* (defined(TM_USE_IPV6) && defined(TM_USE_IPV4)) */

    tm_trace_func_entry(tf6DadConfigComplete);

    tm_assert_is_locked(tf6DadConfigComplete, &(devPtr->devLockEntry) );

#ifdef TM_6_USE_DAD
    dadEntryPtr = tf6DadFindEntry(
        devPtr, &devPtr->dev6IpAddrArray[mHomeIndex]);
    tf6DadRemoveEntry(devPtr, dadEntryPtr);
#endif /* TM_6_USE_DAD */

    if (devPtr->dev6AddrNotifyFuncPtr
        != TM_6_DEV_ADDR_NOTIFY_FUNC_NULL_PTR)
    {
#ifdef TM_DEBUG_LOGGING
        inet_ntop(AF_INET6,
                  &(devPtr->dev6IpAddrArray[mHomeIndex]),
                  buffer,
                  INET6_ADDRSTRLEN);
        tm_debug_log2(
            "TM_6_DEV_ADDR_CONFIG_COMPLETE: mHome=%d, %s",
            mHomeIndex,
            buffer);
#endif /* TM_DEBUG_LOGGING */

/* if the user registered a callback, then notify the user of the event
   TM_6_DEV_ADDR_CONFIG_COMPLETE */

/* first, put the address in the format that the user expects */
        tf6SetSockAddrFromIpAddr(
            &(devPtr->dev6IpAddrArray[mHomeIndex]),
            devPtr, &tempSockAddr.addr.ipv6);

        tm_call_unlock(&(devPtr->devLockEntry));

/* call the user's callback function */
        (*devPtr->dev6AddrNotifyFuncPtr)
            ((ttUserInterface) devPtr,
             (unsigned int) mHomeIndex,
             &tempSockAddr,
             TM_6_DEV_ADDR_CONFIG_COMPLETE);

        tm_call_lock_wait(&(devPtr->devLockEntry));
    }

#ifdef TM_6_USE_PREFIX_DISCOVERY
/* Check if the prefix is deprecated */
    rtKeyCommand.rkc6Command = TM_6_RT_KEY_GET_LIFETIMES;
    rtKeyCommand.rkc6AddrPtr = &(devPtr->dev6IpAddrArray[mHomeIndex]);
    rtKeyCommand.rkc6PrefixLength =
        devPtr->dev6PrefixLenArray[mHomeIndex];
#ifdef TM_6_USE_PREFIX_DISCOVERY
    rtKeyCommand.rkc6Flags = TM_16BIT_ZERO; /* on-link only */
#endif /* TM_6_USE_PREFIX_DISCOVERY */
    errorCode = tf6RtKeyCommand(&rtKeyCommand, TM_8BIT_YES);

    if ((errorCode == TM_ENOERROR) && (rtKeyCommand.rkc6PrefLifetime == 0))
    {
/* this prefix is deprecated, so deprecate this address */
        devPtr->dev6IpAddrFlagsArray[mHomeIndex] |=
            TM_6_DEV_IP_DEPRECATED_FLAG;

/*
 * Update source address cache on this interface - used to quickly pick an
 * appropriate source address for outgoing packets (i.e., one with a similar
 * scope, etc).
 */ 
        tf6UpdateCacheAddress(devPtr, (tt16Bit) mHomeIndex);

        if (devPtr->dev6AddrNotifyFuncPtr
            != TM_6_DEV_ADDR_NOTIFY_FUNC_NULL_PTR)
        {
/* if the user registered a callback, then notify the user of the event
   TM_6_DEV_ADDR_DEPRECATED */

/* first, put the address in the format that the user expects. It is possible
   that they modified it the first time we passed it to their callback, so
   reformat it. */
            tf6SetSockAddrFromIpAddr(
                &(devPtr->dev6IpAddrArray[mHomeIndex]),
                devPtr, &tempSockAddr.addr.ipv6);

            tm_call_unlock(&(devPtr->devLockEntry));

/* call the user's callback function */
            (*devPtr->dev6AddrNotifyFuncPtr)
                ((ttUserInterface) devPtr,
                 (unsigned int) mHomeIndex,
                 &tempSockAddr,
                 TM_6_DEV_ADDR_DEPRECATED);

            tm_call_lock_wait(&(devPtr->devLockEntry));
        }
    }
#endif /* TM_6_USE_PREFIX_DISCOVERY */

#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
#if defined(TM_6_USE_NUD) || defined(TM_USE_RFC3590)
    ipAddrPtr = &(devPtr->dev6IpAddrArray[mHomeIndex]);
#endif /* defined(TM_6_USE_NUD) || defined(TM_USE_RFC3590) */
#endif /* (defined(TM_USE_IPV6) && defined(TM_USE_IPV4)) */

#ifdef TM_6_USE_NUD
#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
    if (!(IN6_IS_ADDR_V4COMPAT(ipAddrPtr)))
#endif /* (defined(TM_USE_IPV6) && defined(TM_USE_IPV4)) */
    {
        tm_call_unlock(&(devPtr->devLockEntry));
/* Send initial unsolicited Neighbor Advertisement */
        tf6NdSendNeighAdvert(
            TM_PACKET_NULL_PTR, devPtr, mHomeIndex,
            &in6addr_linklocal_allnodes, 0);
        tm_call_lock_wait(&(devPtr->devLockEntry));
    }
#endif /* TM_6_USE_NUD */

#ifdef TM_6_USE_RFC3590
#if (defined(TM_USE_IPV6) && defined(TM_USE_IPV4))
    if (!(IN6_IS_ADDR_V4COMPAT(ipAddrPtr)))
#endif /* (defined(TM_USE_IPV6) && defined(TM_USE_IPV4)) */
/* If mHomeIndex is equal to TM_MAX_IPS_PER_IF */
    if (mHomeIndex == TM_MAX_IPS_PER_IF)
    {
/* Once a valid link-local address is available, a node SHOULD generate
   new MLD Report messages for all multicast addresses joined on the
   interface. ([RFC3590].R4:30) */
/* Call the function tf6InitMldSend() */
        tf6InitMldSend(devPtr);
    }
#endif /* TM_USE_RFC3590 */
    
    tm_trace_func_exit(tf6DadConfigComplete);
    return;
}

#ifdef TM_6_USE_DAD
/****************************************************************************
* FUNCTION: tf6DadFindEntry
*
* PURPOSE: This function is called by tf6NdIncomingPacket when a Neighbor
*   Solicitation or Neighbor Advertisement message is received to lookup the
*   DAD entry (if there is one) associated with the target address in the
*   message. This function assumes that the device is locked.
*
* PARAMETERS:
*   devPtr:
*       Pointer to the interface we want to check.
*   targetAddrPtr:
*       Pointer to the target address we want to look for on the specified
*       interface in the list of tentative addresses undergoing Duplicate
*       Address Detection.
*
* RETURNS:
*   NULL:
*       The specified target address was not undergoing Duplicate Address
*       Detection.
*   != NULL:
*       A pointer to the Duplicate Address Detection entry.
*
* NOTES:
*
****************************************************************************/
tt6DadEntryPtr tf6DadFindEntry(
    ttDeviceEntryPtr devPtr,
    tt6ConstIpAddressPtr targetAddrPtr )
{
/* local variables */
    tt6DadEntryPtr dadEntryPtr;

    tm_trace_func_entry(tf6DadFindEntry);

    tm_assert_is_locked(tf6DadFindEntry, &(devPtr->devLockEntry) );

/* Iterate through the linked-list of DAD entries on the interface (i.e.
   tDeviceEntry.dev6DadPtr) until we find a match on the target address,
   causing us to break out of the loop */
    for (dadEntryPtr = devPtr->dev6DadPtr;
         dadEntryPtr != TM_6_DAD_ENTRY_NULL_PTR;
         dadEntryPtr = dadEntryPtr->dad6NextPtr)
    {
        if (tm_6_ip_match(
                targetAddrPtr,
                &(devPtr->dev6IpAddrArray[
                      dadEntryPtr->dad6MultiHomeIndex])))
        {
            break;
        }
    }

    tm_trace_func_exit(tf6DadFindEntry);
    return dadEntryPtr;
}

/****************************************************************************
* FUNCTION: tf6DadAddEntry
*
* PURPOSE: This function is called to create a new DAD entry for a tentative
*   IPv6 address, and insert the DAD entry into the linked-list of DAD entries
*   chained off the interface.
*
* PARAMETERS:
*   devPtr:
*       Pointer to the interface on which we are configuring the tentative
*       address.
*   mHomeIndex:
*       The multi-home index on the interface where we are configuring the
*       tentative address.
*
* RETURNS:
*   NULL:
*       Could not allocate memory for the new DAD entry.
*   != NULL:
*       A pointer to the new Duplicate Address Detection entry.
*
* NOTES:
*   This function assumes that the device is locked.
*
****************************************************************************/
static tt6DadEntryPtr tf6DadAddEntry(
    ttDeviceEntryPtr devPtr,
    tt16Bit mHomeIndex )
{
/* local variables */
    tt6DadEntryPtr dadEntryPtr;

    tm_trace_func_entry(tf6DadAddEntry);

    tm_assert_is_locked(tf6DadAddEntry, &(devPtr->devLockEntry) );

/* Allocate a new DAD entry, and insert it at the head of the linked-list of
   DAD entries on the specified interface */
    dadEntryPtr = (tt6DadEntryPtr)tm_get_raw_buffer(sizeof(tt6DadEntry));

    if (dadEntryPtr != TM_6_DAD_ENTRY_NULL_PTR)
    {
        dadEntryPtr->dad6NextPtr = devPtr->dev6DadPtr;
        devPtr->dev6DadPtr = dadEntryPtr;

/* Initialize the new DAD entry */
        dadEntryPtr->dad6TimerPtr = TM_TMR_NULL_PTR;
        dadEntryPtr->dad6DevPtr = devPtr;
        dadEntryPtr->dad6MultiHomeIndex = mHomeIndex;
    }

    tm_trace_func_exit(tf6DadAddEntry);
    return dadEntryPtr;
}

/****************************************************************************
* FUNCTION: tf6DadRemoveEntry
*
* PURPOSE: This function is called to unlink the specified DAD entry from the
*   linked-list of DAD entries chained off the interface, stop the associated
*   DAD timer if there is any, and recycle the DAD entry. This function
*   assumes that the device is locked.
*
* PARAMETERS:
*   devPtr:
*       Pointer to the interface that has the DAD entry.
*   dadEntryPtr:
*       Pointer to the DAD entry to unlink.
*
* RETURNS:
*   Nothing
*
* NOTES:
*
****************************************************************************/
void tf6DadRemoveEntry(
    ttDeviceEntryPtr devPtr,
    tt6DadEntryPtr dadEntryPtr )
{
/* local variables */
    tt6DadEntryPtr prevDadEntryPtr, currDadEntryPtr;

    tm_trace_func_entry(tf6DadRemoveEntry);

    tm_assert_is_locked(tf6DadRemoveEntry, &(devPtr->devLockEntry) );

    if (dadEntryPtr != TM_6_DAD_ENTRY_NULL_PTR)
    {
/* Iterate through the linked-list of DAD entries on the interface
   until we find a match */
        prevDadEntryPtr = TM_6_DAD_ENTRY_NULL_PTR;
        for (currDadEntryPtr = devPtr->dev6DadPtr;
             currDadEntryPtr != TM_6_DAD_ENTRY_NULL_PTR;
             currDadEntryPtr = currDadEntryPtr->dad6NextPtr)
        {
            if (currDadEntryPtr == dadEntryPtr)
            {
                break;
            }

            prevDadEntryPtr = currDadEntryPtr;
        }

        if (currDadEntryPtr != TM_6_DAD_ENTRY_NULL_PTR)
        {
/* we found a match, unlink the DAD entry from the list: */
            if (prevDadEntryPtr == TM_6_DAD_ENTRY_NULL_PTR)
            {
                devPtr->dev6DadPtr = currDadEntryPtr->dad6NextPtr;
            }
            else
            {
                prevDadEntryPtr->dad6NextPtr = currDadEntryPtr->dad6NextPtr;
            }
        }

        tf6DadStopTimer(devPtr, dadEntryPtr);
        tm_free_raw_buffer((ttRawBufferPtr) dadEntryPtr);
    }
 
    tm_trace_func_exit(tf6DadRemoveEntry);
    return;
}

/****************************************************************************
* FUNCTION: tf6DadFailed
*
* PURPOSE: 
*   This function is called when Duplicate Address Detection has failed for
*   our tentative address. If the address is the only link-local IPv6 address
*   on the interface, then the IPv6 part of the interface is disabled. This
*   function assumes that the device is locked.
*
* PARAMETERS:
*   devPtr:
*       Pointer to the device that we were configuring the address on.
*   mHomeIndex:
*       The multi-home index on the interface where we were configuring the
*       address.
*   dadEntryPtr:
*       Pointer to the DAD entry. NULL if there is no associated DAD entry.
*
* RETURNS:
*   Nothing
*
* NOTES:
*
****************************************************************************/
void tf6DadFailed(
    ttDeviceEntryPtr devPtr,
    tt16Bit          mHomeIndex,
    tt6DadEntryPtr   dadEntryPtr)
{
/* local variables */
    int                     errorCode;
    struct sockaddr_storage tempSockAddr;
#ifdef TM_6_USE_DAD
    tt6IpAddress            tempAddr;
    tt6IpAddressPtr         localIpAddrPtr;
#endif /* TM_6_USE_DAD */
#ifdef TM_DEBUG_LOGGING
    char                    buffer[INET6_ADDRSTRLEN];
#endif /* TM_DEBUG_LOGGING */

    tm_trace_func_entry(tf6DadFailed);

    tm_assert_is_locked(tf6DadFailed, &(devPtr->devLockEntry) );

    tf6DelayInitSolTimerRemove(devPtr, mHomeIndex);
    if (mHomeIndex == TM_MAX_IPS_PER_IF)
    {
        tf6RemoveRtrSol(devPtr);
    }
    tf6DadRemoveEntry(devPtr, dadEntryPtr);

    if (devPtr->dev6IpAddrFlagsArray[mHomeIndex] &
        (TM_6_DEV_IP_CONF_STARTED | TM_6_DEV_IP_CONFIG))
    {
        if (mHomeIndex == TM_MAX_IPS_PER_IF)
        {
            tm_6_ip_copy(&(devPtr->dev6IpAddrArray[mHomeIndex]),
                         &tempAddr);
/* Embed interface index or site ID in link-local or site-local addresses */
            tm_6_dev_scope_addr(&tempAddr, devPtr);
/* Delete the local route for the link-local scope prefix */
            (void)tf6RtDelRoute(
#ifdef TM_USE_STRONG_ESL
                            devPtr,
#endif /* TM_USE_STRONG_ESL */
                            &tempAddr,
                            (int)64,
                            TM_RTE_LOCAL );
/* Leave associated multicast group */
            localIpAddrPtr = &(devPtr->dev6IpAddrArray[mHomeIndex]);
            tm_6_addr_to_sol_node(localIpAddrPtr, &tempAddr);
#ifdef TM_6_USE_MLD
            (void) tf6MldLeaveGroup(TM_8BIT_NO, devPtr, &tempAddr
#ifdef TM_6_USE_MLDV2
                    ,
                    TM_6_MODE_IS_INCLUDE,
                    TM_6_SRC_ADDR_ENTRY_NULL_PTR);
#else /*TM_6_USE_MLDV2*/
                    );
#endif /* TM_6_USE_MLDV2 */
#else /* !TM_6_USE_MLD */
            (void) tf6MldDeleteGroup(devPtr, &tempAddr);
#endif /* !TM_6_USE_MLD */
        }
/* Remove the address from the interface ([RFC2462].R5.4.5:10) */
        tm_8bit_clr_bit(devPtr->dev6IpAddrFlagsArray[mHomeIndex],
                        (TM_6_DEV_IP_CONFIG | TM_6_DEV_IP_CONF_STARTED));

        tf6UpdateCacheAddress( devPtr, (tt16Bit) mHomeIndex );
#if (TM_6_MAX_AUTOCONF_IPS_PER_IF > 0)
/* If the address is the link-local IPv6 address auto-configured using the
   interface ID, then disable IPv6 auto-configuration on the interface
   ([RFC2462].R5.4.5:30) */
        if (mHomeIndex == (tt16Bit) TM_MAX_IPS_PER_IF)
        {
/* unconfigure any remaining auto-configured IPv6 addresses, since the
   auto-configured link-local scope IPv6 address was just unconfigured */
            (void) tf6UnConfigInterfaceId(devPtr);
        }
#endif /* (TM_6_MAX_AUTOCONF_IPS_PER_IF > 0) */

        if (   devPtr->dev6AddrNotifyFuncPtr
            != TM_6_DEV_ADDR_NOTIFY_FUNC_NULL_PTR)
        {
#ifdef TM_DEBUG_LOGGING
            inet_ntop(AF_INET6,
                      &(devPtr->dev6IpAddrArray[mHomeIndex]),
                      buffer,
                      INET6_ADDRSTRLEN);
            tm_debug_log2(
                "TM_6_DEV_ADDR_CONFIG_FAILED: mHome=%d, %s",
                mHomeIndex,
                buffer);
#endif /* TM_DEBUG_LOGGING */

/* the user registered a callback */

/* first, put the address in the format that the user expects */
            tf6SetSockAddrFromIpAddr(
                &(devPtr->dev6IpAddrArray[mHomeIndex]),
                devPtr, &(tempSockAddr.addr.ipv6));

            tm_call_unlock(&(devPtr->devLockEntry));

/* notify the user of the event TM_6_DEV_ADDR_CONFIG_FAILED.
   ([RFC2462].R5.4.5:20) */
            (*devPtr->dev6AddrNotifyFuncPtr)
                ((ttUserInterface) devPtr,
                 (unsigned int) mHomeIndex,
                 &tempSockAddr,
                 TM_6_DEV_ADDR_CONFIG_FAILED);

            tm_call_lock_wait(&(devPtr->devLockEntry));
        }

#if (TM_6_MAX_AUTOCONF_IPS_PER_IF > 0)
        if ((mHomeIndex == (tt16Bit) TM_MAX_IPS_PER_IF)
            && (devPtr->dev6Flags & TM_6_USE_AUTO_IID))
        {
/* if the user has specified for us to attempt to recover a DAD failure
   on the auto-configured link-local scope IPv6 address, then we do
   that here. */
            errorCode = 
                tf6SetRandomLockedInterfaceId(devPtr);
            if(errorCode == TM_ENOERROR)
            {
/* start DAD on the new auto-configured link-local scope IPv6 address */
                tf6ConfigInterfaceId(devPtr);
            }
        }
#endif /* (TM_6_MAX_AUTOCONF_IPS_PER_IF > 0) */
    }

    tm_trace_func_exit(tf6DadFailed);
    return;
}

/****************************************************************************
* FUNCTION: tf6DadDupDetected
*
* PURPOSE: This function is called when we detect that a different node is
*   using an IPv6 address that we have already configured on the interface.
*   ([RFC2462].R5.4.4:10)
*
* PARAMETERS:
*   devPtr:
*       Pointer to the device that the address is configured on.
*   mHomeIndex:
*       The multi-home index of the address.
*
* RETURNS:
*   Nothing
*
* NOTES:
*   This function assumes that the device is locked.
*
****************************************************************************/
void tf6DadDupDetected(
    ttDeviceEntryPtr devPtr,
    tt16Bit mHomeIndex )
{
/* local variables */
    struct sockaddr_storage tempSockAddr;
#ifdef TM_DEBUG_LOGGING
    char                    buffer[INET6_ADDRSTRLEN];
#endif /* TM_DEBUG_LOGGING */

    tm_trace_func_entry(tf6DadDupDetected);

    tm_assert_is_locked(tf6DadDupDetected, &(devPtr->devLockEntry));

    tf6DelayInitSolTimerRemove(devPtr, mHomeIndex);

    if (mHomeIndex == TM_MAX_IPS_PER_IF)
    {
        tf6RemoveRtrSol(devPtr);
    }

    if (devPtr->dev6AddrNotifyFuncPtr
        != TM_6_DEV_ADDR_NOTIFY_FUNC_NULL_PTR)
    {
#ifdef TM_DEBUG_LOGGING
        inet_ntop(AF_INET6,
                  &(devPtr->dev6IpAddrArray[mHomeIndex]),
                  buffer,
                  INET6_ADDRSTRLEN);
        tm_debug_log2(
            "TM_6_DEV_ADDR_DUP_DETECTED: mHome=%d, %s",
            mHomeIndex,
            buffer);
#endif /* TM_DEBUG_LOGGING */

/* the user registered a callback */

/* first, put the address in the format that the user expects */
        tf6SetSockAddrFromIpAddr(
            &(devPtr->dev6IpAddrArray[mHomeIndex]),
            devPtr, &(tempSockAddr.addr.ipv6));

            tm_call_unlock(&(devPtr->devLockEntry));

/* notify the user of the event TM_6_DEV_ADDR_DUP_DETECTED. */
            (*devPtr->dev6AddrNotifyFuncPtr)
                ((ttUserInterface) devPtr,
                (unsigned int) mHomeIndex,
                &tempSockAddr,
                TM_6_DEV_ADDR_DUP_DETECTED);

            tm_call_lock_wait(&(devPtr->devLockEntry));
        }

    tm_trace_func_exit(tf6DadDupDetected);
    return;
}

/****************************************************************************
* FUNCTION: tf6DadTimer
*
* PURPOSE: This function is called during the process of Duplicate Address
*   Detection when we have timed-out waiting for a solicited Neighbor
*   Advertisement for our tentative address. This function retransmits the
*   Neighbor Solicitation probe message up to
*   (ttDeviceEntry.dev6DupAddrDetectTransmits - 1) times, each retransmission
*   separated by ttDeviceEntry.dev6NeighSolicitReTxPeriod milliseconds
*   ([RFC2462].R5.4.2:20). If Duplicate Address Detection is successful, this
*   function calls tfDeviceStart to complete configuration of the address on
*   the interface.
*
* PARAMETERS:
*   dadTimerPtr:
*       Pointer to the Duplicate Address Detection timer.
*   userParm1:
*       Pointer to the interface.
*   userParm2:
*       Pointer to the DAD entry.
*
* RETURNS:
*   Nothing
*
* NOTES:
*
****************************************************************************/
static void tf6DadTimer(
    ttVoidPtr dadTimerPtr,
    ttGenericUnion userParm1,
    ttGenericUnion userParm2 )
{
/* local variables */
    tt6DadEntryPtr      dadEntryPtr;
    ttDeviceEntryPtr    devPtr;
    tt16Bit             mHomeIndex;

    tm_trace_func_entry(tf6DadTimer);

/* Extract the device pointer from userParm1 */
    devPtr = (ttDeviceEntryPtr) userParm1.genVoidParmPtr;
/* Extract the DAD entry pointer from userParm2 */
    dadEntryPtr = (tt6DadEntryPtr) userParm2.genVoidParmPtr;

    tm_call_lock_wait(&(devPtr->devLockEntry));

/* verify that we haven't been removed in another task */
    if (tm_timer_not_reinit((ttTimerPtr) dadTimerPtr))
    {
        mHomeIndex = dadEntryPtr->dad6MultiHomeIndex;

        if (dadEntryPtr->dad6ReTxCount == TM_8BIT_ZERO)
        {
            dadEntryPtr = tf6DadFindEntry(
                devPtr, &devPtr->dev6IpAddrArray[mHomeIndex]);
            tf6DadRemoveEntry(devPtr, dadEntryPtr);
    
/* duplicate address detection completed successfully, complete configuration
   of the address on the interface. */
            (void)tfDeviceStart(devPtr, mHomeIndex, AF_INET6);

#ifdef TM_6_USE_DHCP
/* Start DHCPv6 if applicable */
            if (tm_16bit_one_bit_set(devPtr->dev6Flags,
                                     TM_6_DEV_USER_DHCP))
            {
                (*(devPtr->dev6BootFuncPtr))
                            (devPtr, devPtr->dev6Flags, TM_8BIT_YES);
            }
#endif /* TM_6_USE_DHCP */
        }
        else
        {
/* retransmit the Neighbor Solicitation for our tentative address */
            tf6DadCreateProbe(devPtr, mHomeIndex);

/* decrement the retransmission count for the next time this auto-timer
   fires */
            dadEntryPtr->dad6ReTxCount--;
        }
    }

    tm_call_unlock(&(devPtr->devLockEntry));

    tm_trace_func_exit(tf6DadTimer);
    return;
}

/****************************************************************************
* FUNCTION: tf6DadStartTimer
*
* PURPOSE: 
*   This function starts the retransmission timer for a specified DAD entry.
*   This function assumes that the device is locked.
*
* PARAMETERS:
*   devPtr:
*       Pointer to the interface.
*   mHomeIndex:
*       The multi-home index of the tentative address upon which we are
*       performing Duplicate Address Detection, passed as the second user
*       parameter to the timer call back.
*   dadEntryPtr:
*       Pointer to the DAD entry.
*
* RETURNS:
*   Nothing
*
* NOTES:
*
****************************************************************************/
static void tf6DadStartTimer(
    ttDeviceEntryPtr devPtr,
    tt6DadEntryPtr dadEntryPtr )
{
/* local variables */
    ttGenericUnion timerParm1;
    ttGenericUnion timerParm2;

    tm_trace_func_entry(tf6DadStartTimer);

    tm_assert_is_locked(tf6DadStartTimer, &(devPtr->devLockEntry) );

    if (dadEntryPtr != TM_6_DAD_ENTRY_NULL_PTR)
    {
        tm_assert(
            tf6DadStartTimer,
            (dadEntryPtr->dad6TimerPtr == TM_TMR_NULL_PTR));
        tm_assert(
            tf6DadStartTimer,
            (devPtr->dev6NeighSolicitReTxPeriod != 0));
        tm_assert(
            tf6DadStartTimer,
            (devPtr->dev6DupAddrDetectTransmits > 0));

        dadEntryPtr->dad6ReTxCount =
            (tt8Bit) (devPtr->dev6DupAddrDetectTransmits - 1);
        timerParm1.genVoidParmPtr = (ttVoidPtr) devPtr;
        timerParm2.genVoidParmPtr = (ttVoidPtr) dadEntryPtr;

        dadEntryPtr->dad6TimerPtr = tfTimerAdd(
            tf6DadTimer,
            timerParm1, /* interface pointer */
            timerParm2, /* DAD entry pointer */
            devPtr->dev6NeighSolicitReTxPeriod,
            TM_TIM_AUTO);
    }

    tm_trace_func_exit(tf6DadStartTimer);
    return;
}

/****************************************************************************
* FUNCTION: tf6DadStopTimer
*
* PURPOSE: This function stops the timer for a specified DAD entry.
*
* PARAMETERS:
*   devPtr:
*       Pointer to the interface.
*   dadEntryPtr:
*       Pointer to the DAD entry for which we want to stop the timer.
*
* RETURNS:
*   Nothing
*
* NOTES:
*   This function assumes that the device is locked.
*
****************************************************************************/
static void tf6DadStopTimer(
    ttDeviceEntryPtr devPtr,
    tt6DadEntryPtr dadEntryPtr )
{
/* local variables */
    ttTimerPtr timerPtr;

    TM_UNREF_IN_ARG(devPtr);

    tm_trace_func_entry(tf6DadStopTimer);

    tm_assert_is_locked(tf6DadStopTimer, &(devPtr->devLockEntry) );

    if (dadEntryPtr != TM_6_DAD_ENTRY_NULL_PTR)
    {
        timerPtr = dadEntryPtr->dad6TimerPtr;
        if (timerPtr != TM_TMR_NULL_PTR)
        {
            tm_timer_remove(timerPtr);
            dadEntryPtr->dad6TimerPtr = TM_TMR_NULL_PTR;
        }
    }

    tm_trace_func_exit(tf6DadStopTimer);
    return;
}

/****************************************************************************
* FUNCTION: tf6DadCreateProbe
*
* PURPOSE: Send a Neighbor Solicitation message for the purpose of Duplicate
*   Address Detection.
*
* PARAMETERS:
*   devPtr:
*       Pointer to the device that the tentative address is stored on. This
*       is the device that we will send the Neighbor Solicitation message out
*       on.
*   mHomeIndex:
*       The multi-home index of the address on which we are performing
*       Duplicate Address Detection.
*
* RETURNS:
*   0:
*       Success
*   TM_ENOBUF:
*       Not enough memory to allocate a packet.
*   TM_ENETDOWN:
*       Network is down.
*
* NOTES:
*   This function assumes that the device is locked.
*
****************************************************************************/
static int tf6DadCreateProbe(
    ttDeviceEntryPtr devPtr,
    tt16Bit mHomeIndex )
{
/* local variables */
    int                 errorCode;
    tt6IpAddress        destAddr;

    tm_trace_func_entry(tf6DadCreateProbe);

    tm_assert_is_locked(tf6DadCreateProbe, &(devPtr->devLockEntry));

/* Set the destination IP address (destAddrPtr) of the Neighbor Solicitation
   message to the solicited-node multicast address of the target address
   ([RFC2462].R5.4.2:20) */
    tm_6_addr_to_sol_node(
        &(devPtr->dev6IpAddrArray[mHomeIndex]), &destAddr);

    tm_call_unlock(&(devPtr->devLockEntry));
/* The source IP address (srcAddrPtr) of the Neighbor Solicitation
   message is the IPv6 unspecified address (i.e. all 0's).
   ([RFC2462].R5.4.2:20) */
    errorCode = tf6NdSendNeighSolicit(
        devPtr,
        &(devPtr->dev6IpAddrArray[mHomeIndex]),
        &in6addr_any,
        &destAddr);
    tm_call_lock_wait(&(devPtr->devLockEntry));

    tm_trace_func_exit_rc(tf6DadCreateProbe, errorCode);
    return errorCode;
}
#endif /* TM_6_USE_DAD */

#ifdef TM_6_USE_PREFIX_DISCOVERY
/* common code for all prefix timer handlers */
void tf6PrefixTimerCommon(
    ttGenericUnion userParm1,
    ttGenericUnion userParm2,
    tt6PrefixFuncPtr prefixFuncPtr)
{
/* local variables */
    tt6IpAddress        prefix;
    ttDeviceEntryPtr    devPtr;

    prefix.s6LAddr[0] = userParm1.gen32bitParm;
    prefix.s6LAddr[1] = userParm2.gen32bitParm;

/* a link-local scope IPv6 address is never invalidated ([RFC2462].R5.3:50) */
    tm_assert(tf6PrefixTimerCommon, (!IN6_IS_ADDR_LINKLOCAL(&prefix)));

    tm_kernel_set_critical;
    devPtr = tm_context(tvDeviceList);
    tm_kernel_release_critical;

    while (devPtr != TM_DEV_NULL_PTR)
    {
/* TBD: site-local could involve multiple interfaces, so we need to iterate
   through all of the interfaces looking for a match on site ID */

/* Because of a potential deadlock, we can't lock the device while we have
   a lock on the device list, so we can't lock the device list here but
   intead we use critical sections around accesses to device pointers
   stored in the device list.
   NOTE: THIS CODE MUST BE REVISITED IF WE EVER DECIDE TO REMOVE A DEVICE
   FROM THE DEVICE LIST. */

        tm_call_lock_wait(&(devPtr->devLockEntry));
        (*prefixFuncPtr)(devPtr, &prefix);
        tm_call_unlock(&(devPtr->devLockEntry));

        tm_kernel_set_critical;
        devPtr = devPtr->devNextDeviceEntry;
        tm_kernel_release_critical;
    }

    return;
}

/****************************************************************************
* FUNCTION: tf6RtInvalidatePrefixTimer
*
* PURPOSE: This function is the address invalidation timer specified by
*   requirement [RFC2461].R6.3.4:130. This function invalidates an address
*   prefix, which causes all of the configured addresses having that prefix
*   to be unconfigured, and also causes the local route entry for that prefix
*   to be removed from the routing tree.
*
* PARAMETERS:
*   invalidateTimerPtr:
*       Pointer to the address invalidation timer that just expired.
*   userParm1:
*       High-order 32 bits of the prefix to invalidate
*   userParm2:
*       Low-order 32 bits of the prefix to invalidate
*
* RETURNS:
*   Nothing
*
* NOTES:
*
****************************************************************************/
void tf6RtInvalidatePrefixTimer(
    ttVoidPtr invalidateTimerPtr,
    ttGenericUnion userParm1,
    ttGenericUnion userParm2 )
{
/* local variables */

    tm_trace_func_entry(tf6RtInvalidatePrefixTimer);
    TM_UNREF_IN_ARG(invalidateTimerPtr); /* compiler warning */

    tf6PrefixTimerCommon(
        userParm1, userParm2, tf6UnConfigPrefix);

    tm_trace_func_exit(tf6RtInvalidatePrefixTimer);
    return;
}

/****************************************************************************
* FUNCTION: tf6UnConfigPrefix
*
* PURPOSE: This function unconfigures the specified IPv6 address prefix from
*   the specified interface, which comprises unconfiguring all IPv6 addresses
*   previously configured on the interface that have the specified address
*   prefix, as well as removing the local route entry for that prefix from the
*   routing tree or the list of non-on-link prefixes (i.e.
*   ttRtRadixHeadEntry.rth6NonOnLinkPrefixList), and any associated Neighbor
*   Cache entries from the Neighbor Cache. This function is called when the
*   invalidation timer expires for the specified IPv6 address prefix (refer to
*   tf6RtPrefixAgingTimer). The address prefix is assumed to be 64 bits in
*   length ([RFC2373].R2.4:20, [RFC2462].R5.5.3:80). This function assumes that
*   the device is locked.
*
* PARAMETERS:
*   devPtr:
*       Pointer to the device/interface for which we want to unconfigure a
*       specified address prefix.
*   prefixPtr:
*       Pointer to the IPv6 address prefix that we are unconfiguring.
*
* RETURNS:
*   Nothing
*
* NOTES:
*
****************************************************************************/
static void tf6UnConfigPrefix(
    ttDeviceEntryPtr devPtr,
    tt6IpAddressPtr prefixPtr )
{
/* local variables */
    int         maxMhomeAutoEntries;
    int         maxMhomeEntries;
    int         i;
    struct sockaddr_storage tempSockAddr;

    tm_trace_func_entry(tf6UnConfigPrefix);

    tm_assert_is_locked(tf6UnConfigPrefix, &(devPtr->devLockEntry));

    maxMhomeAutoEntries = (int) devPtr->dev6MhomeUsedAutoEntries;
    maxMhomeEntries = (int) devPtr->dev6MhomeUsedEntries;
    for (i = 0; i < maxMhomeAutoEntries;)
    {
        if (devPtr->dev6IpAddrFlagsArray[i] & TM_6_DEV_IP_CONFIG)
        {
            if ((prefixPtr->s6LAddr[0] ==
                 devPtr->dev6IpAddrArray[i].s6LAddr[0])
                && (prefixPtr->s6LAddr[1] ==
                    devPtr->dev6IpAddrArray[i].s6LAddr[1]))
            {
/*
 * One of the prefix address timed out, so we need to send a Router Solicitation
 * to attempt to obtain another prefix address.
 */
                (void)tf6SendRtrSol(devPtr);
            }
        }

/* tf6SendRtrSol() unlocks and re-locks the device so we must check it again */
        if (devPtr->dev6IpAddrFlagsArray[i] & TM_6_DEV_IP_CONFIG)
        {
            if ((prefixPtr->s6LAddr[0] ==
                 devPtr->dev6IpAddrArray[i].s6LAddr[0])
                && (prefixPtr->s6LAddr[1] ==
                    devPtr->dev6IpAddrArray[i].s6LAddr[1]))
            {
/* first, put the address in the format that the user expects */
                tf6SetSockAddrFromIpAddr(
                    &(devPtr->dev6IpAddrArray[i]),
                    devPtr, &(tempSockAddr.addr.ipv6));

/* if we match on the prefix, then unconfigure the IPv6 address */
                (void)tfRemoveInterface(devPtr, (tt16Bit) i, PF_INET6);

                if (devPtr->dev6AddrNotifyFuncPtr
                    != TM_6_DEV_ADDR_NOTIFY_FUNC_NULL_PTR)
                {
/* the user registered a callback */
                    tm_call_unlock(&(devPtr->devLockEntry));
                    
/* notify the user of the event TM_6_DEV_ADDR_INVALIDATED. */
                    (*devPtr->dev6AddrNotifyFuncPtr)
                        ((ttUserInterface) devPtr,
                         (unsigned int) i,
                         &tempSockAddr,
                         TM_6_DEV_ADDR_INVALIDATED);
                    
                    tm_call_lock_wait(&(devPtr->devLockEntry));
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

    tm_trace_func_exit(tf6UnConfigPrefix);
    return;
}

/****************************************************************************
* FUNCTION: tf6RtDeprecatePrefixTimer
*
* PURPOSE: This function deprecates an address prefix, which causes all of
*   the configured addresses having that prefix to also become deprecated.
*
* PARAMETERS:
*   deprecateTimerPtr:
*       Pointer to the timer that just expired.
*   userParm1:
*       High-order 32 bits of the prefix to deprecate
*   userParm2:
*       Low-order 32 bits of the prefix to deprecate
*
* RETURNS:
*   Nothing
*
* NOTES:
*
****************************************************************************/
void tf6RtDeprecatePrefixTimer(
    ttVoidPtr deprecateTimerPtr,
    ttGenericUnion userParm1,
    ttGenericUnion userParm2 )
{
/* local variables */

    tm_trace_func_entry(tf6RtDeprecatePrefixTimer);

    TM_UNREF_IN_ARG(deprecateTimerPtr); /* compiler warning */
    tf6PrefixTimerCommon(
        userParm1, userParm2, tf6DeprecatePrefix);

    tm_trace_func_exit(tf6RtDeprecatePrefixTimer);
    return;
}

/****************************************************************************
* FUNCTION: tf6DeprecatePrefix
*
* PURPOSE: This function deprecates the specified IPv6 address prefix on the
*   specified interface, which comprises deprecating all IPv6 addresses
*   previously configured on the interface that have the specified address
*   prefix. This function is called when the invalidation timer expires for
*   the specified IPv6 address prefix (refer to tf6RtPrefixAgingTimer).
*
* PARAMETERS:
*   devPtr:
*       Pointer to the device/interface for which we want to deprecate a
*       specified address prefix.
*   prefixPtr:
*       Pointer to the IPv6 address prefix that we are deprecating
*
* RETURNS:
*   Nothing
*
* NOTES:
*   The address prefix is assumed to be 64 bits in length
*   ([RFC2373].R2.4:20, [RFC2462].R5.5.3:80). This function assumes that the
*   device is locked.
*
****************************************************************************/
static void tf6DeprecatePrefix(
    ttDeviceEntryPtr devPtr,
    tt6IpAddressPtr prefixPtr )
{
/* local variables */
    int         maxMhomeAutoEntries;
    int         maxMhomeEntries;
    int         i;
    struct sockaddr_storage tempSockAddr;

    tm_trace_func_entry(tf6DeprecatePrefix);

    tm_assert_is_locked(tf6DeprecatePrefix, &(devPtr->devLockEntry));

    maxMhomeAutoEntries = (int) devPtr->dev6MhomeUsedAutoEntries;
    maxMhomeEntries = (int) devPtr->dev6MhomeUsedEntries;
    for (i = 0; i < maxMhomeAutoEntries;)
    {
        if (devPtr->dev6IpAddrFlagsArray[i] & TM_6_DEV_IP_CONFIG)
        {
            if ((prefixPtr->s6LAddr[0] ==
                 devPtr->dev6IpAddrArray[i].s6LAddr[0])
                && (prefixPtr->s6LAddr[1] ==
                    devPtr->dev6IpAddrArray[i].s6LAddr[1]))
            {
/* if we match on the prefix, then transition state to deprecated */
                devPtr->dev6IpAddrFlagsArray[i] |=
                    TM_6_DEV_IP_DEPRECATED_FLAG;

/*
 * Update source address cache on this interface - used to quickly pick an
 * appropriate source address for outgoing packets (i.e., one with a similar
 * scope, etc).
 */ 
                tf6UpdateCacheAddress(devPtr, (tt16Bit) i);

                if (devPtr->dev6AddrNotifyFuncPtr
                    != TM_6_DEV_ADDR_NOTIFY_FUNC_NULL_PTR)
                {
/* the user registered a callback */

/* first, put the address in the format that the user expects */
                    tf6SetSockAddrFromIpAddr(
                        &(devPtr->dev6IpAddrArray[i]),
                        devPtr, &(tempSockAddr.addr.ipv6));

                    tm_call_unlock(&(devPtr->devLockEntry));

/* notify the user of the event TM_6_DEV_ADDR_DEPRECATED. */
                    (*devPtr->dev6AddrNotifyFuncPtr)
                        ((ttUserInterface) devPtr,
                         (unsigned int) i,
                         &tempSockAddr,
                         TM_6_DEV_ADDR_DEPRECATED);

                    tm_call_lock_wait(&(devPtr->devLockEntry));
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

    tm_trace_func_exit(tf6DeprecatePrefix);
    return;
}

/****************************************************************************
* FUNCTION: tf6UnDeprecatePrefix
*
* PURPOSE: This function undeprecates the specified IPv6 address prefix on
*   the specified interface, which comprises transitioning from deprecated to
*   preferred all IPv6 addresses previously configured on the interface that
*   have the specified address prefix. This function is called by Prefix
*   Discovery when a Router Advertisement containing a Prefix Information
*   option for the specified prefix is received which indicates that the
*   prefix is now preferred.
*
* PARAMETERS:
*   devPtr:
*       Pointer to the device/interface for which we want to undeprecate
*       (i.e. make preferred) a specified address prefix.
*   prefixPtr:
*       Pointer to the IPv6 address prefix that we are undeprecating.
*
* RETURNS:
*   Nothing
*
* NOTES:
*   The address prefix is assumed to be 64 bits in length
*   ([RFC2373].R2.4:20, [RFC2462].R5.5.3:80). This function assumes that the
*   device is locked.
*
****************************************************************************/
void tf6UnDeprecatePrefix(
    ttDeviceEntryPtr devPtr,
    tt6IpAddressPtr prefixPtr )
{
/* local variables */
    int         maxMhomeAutoEntries;
    int         maxMhomeEntries;
    int         i;
    struct sockaddr_storage tempSockAddr;

    tm_trace_func_entry(tf6UnDeprecatePrefix);

    tm_assert_is_locked(tf6UnDeprecatePrefix, &(devPtr->devLockEntry));

    maxMhomeAutoEntries = (int) devPtr->dev6MhomeUsedAutoEntries;
    maxMhomeEntries = (int) devPtr->dev6MhomeUsedEntries;
    for (i = 0; i < maxMhomeAutoEntries;)
    {
        if (devPtr->dev6IpAddrFlagsArray[i] & TM_6_DEV_IP_CONFIG)
        {
            if ((prefixPtr->s6LAddr[0] ==
                 devPtr->dev6IpAddrArray[i].s6LAddr[0])
                && (prefixPtr->s6LAddr[1] ==
                    devPtr->dev6IpAddrArray[i].s6LAddr[1]))
            {
/* if we match on the prefix, then transition state to preferred */
                tm_8bit_clr_bit( devPtr->dev6IpAddrFlagsArray[i],
                                 TM_6_DEV_IP_DEPRECATED_FLAG );

/*
 * Update source address cache on this interface - used to quickly pick an
 * appropriate source address for outgoing packets (i.e., one with a similar
 * scope, etc).
 */ 
                tf6UpdateCacheAddress(devPtr, (tt16Bit) i);

                if (devPtr->dev6AddrNotifyFuncPtr
                    != TM_6_DEV_ADDR_NOTIFY_FUNC_NULL_PTR)
                {
/* the user registered a callback */

/* first, put the address in the format that the user expects */
                    tf6SetSockAddrFromIpAddr(
                        &(devPtr->dev6IpAddrArray[i]),
                        devPtr, &(tempSockAddr.addr.ipv6));

                    tm_call_unlock(&(devPtr->devLockEntry));

/* notify the user of the event TM_6_DEV_ADDR_DEPRECATED. */
                    (*devPtr->dev6AddrNotifyFuncPtr)
                        ((ttUserInterface) devPtr,
                         (unsigned int) i,
                         &tempSockAddr,
                         TM_6_DEV_ADDR_PREFERRED);

                    tm_call_lock_wait(&(devPtr->devLockEntry));
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

    tm_trace_func_exit(tf6UnDeprecatePrefix);
    return;
}
#endif /* TM_6_USE_PREFIX_DISCOVERY */

#else /* ! TM_USE_IPV6 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV6 is not defined */
int tv6AutoDummy = 0;
#endif /* ! TM_USE_IPV6 */

/***************** End Of File *****************/
