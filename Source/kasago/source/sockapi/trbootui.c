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
 * Description: Get BOOT entry interface
 *
 * Filename: trbootui.c
 * Author: Odile
 * Date Created: 01/23/99
 * $Source: source/sockapi/trbootui.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:42:47JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_IPV4

#include <trtype.h>
#include <trproto.h>
#ifdef TM_LOCK_NEEDED
#include <trglobal.h>
#endif /* TM_LOCK_NEEDED */

static ttUserBtEntryPtr tfGetBootEntry (ttUserInterface interfaceHandle,
                                        int             index,
                                        int             type,
                                        int             bootIndex );

/*
 * tfDhcpUserGetBootEntry function description:
 * Get a pointer to DHCP user BOOT entry (result of a successful
 * tfDhcpUserStart())
 * Parameters
 * Value             Meaning
 * interfaceHandle   Ethernet interface handle
 * userIndex         User Index (between 0, and tvMaxUserDhcpEntries - 1)
 * Return:
 * userBtEntryPtr    Pointer to a user boot entry as defined in trsocket.h
 * 0                 failure. No DHCP address bound.
 */
ttUserBtEntryPtr tfDhcpUserGetBootEntry( ttUserInterface interfaceHandle,
                                         int             index )
{
    return tfGetBootEntry (interfaceHandle, index, (int)(TM_BT_USER),
                           (int)TM_BOOT_DHCP );
}

/*
 * tfBootpUserGetBootEntry function description:
 * Get a pointer to BOOTP user BOOT entry (result of a successful
 * tfBootpUserStart())
 * Parameters
 * Value             Meaning
 * interfaceHandle   Ethernet interface handle
 * userIndex         User Index (between 0, and tvMaxUserDhcpEntries - 1)
 * Return:
 * userBtEntryPtr    Pointer to a user boot entry as defined in trsocket.h
 * 0                 failure. No DHCP address bound.
 */
ttUserBtEntryPtr tfBootpUserGetBootEntry( ttUserInterface interfaceHandle,
                                          int             index )
{
    return tfGetBootEntry (interfaceHandle, index, (int)(TM_BT_USER),
                           (int)TM_BOOT_BOOTP );
}

/*
 * tfConfGetBootEntry function description:
 * Get a pointer to DHCP/BOOTP Conf BOOT entry (obtained while doing a
 * tfConfigInterface with either TM_DEV_IP_BOOTP, or TM_DEV_IP_DHCP).
 * Parameters
 * Value             Meaning
 * interfaceHandle   Ethernet interface handle
 * multiHomeIndex    multihome index of the ethernet device
 * Return:
 * userBtEntryPtr    Pointer to a user boot entry as defined in trsocket.h
 * NULL              failure. No DHCP/BOOTP address bound.
 */
ttUserBtEntryPtr tfConfGetBootEntry( ttUserInterface interfaceHandle,
                                     unsigned char    multiHomeIndex )
{
    ttUserBtEntryPtr userBtEntryPtr;
    int              i;

    userBtEntryPtr = (ttUserBtEntryPtr)0;
    for ( i = 0;
          (i < TM_BOOT_MAX_TYPES) && (userBtEntryPtr == (ttUserBtEntryPtr)0);
          i++ )
    {
        userBtEntryPtr = tfGetBootEntry(interfaceHandle, 
                                        (int)multiHomeIndex, 
                                        (int)(TM_BT_CONF),
                                        i);
    }
    return userBtEntryPtr;
}

/*
 * tfGetBootEntry function description:
 * Get a pointer to a BOOT entry structure as a result of a
 * successful tfConfigInterface with flag TM_DEV_IP_DHCP or TM_DEV_IP_BOOTP
 * in which case index is the multihome index, and type is TM_BT_CONF;
 * or as result of a successful tfDhcpUserStart() or tfBootpUserStart()) in
 * which case index is a user index between 0 and tvMaxUserDhcpEntries - 1,
 * and type is TM_BT_USER.
 * Value             Meaning
 * interfaceHandle   Ethernet interface handle
 * userIndex         index (multi home index, or user index)
 * type              TM_BT_CONF, or TM_BT_USER
 * bootIndex         TM_BOOT_BOOTP, or TM_BOOT_DHCP
 * Return:
 * userBtEntryPtr    Pointer to a user boot entry as defined in trsocket.h
 * 0                 failure. No DHCP address bound.
 */
static ttUserBtEntryPtr tfGetBootEntry ( ttUserInterface interfaceHandle,
                                         int             index,
                                         int             type,
                                         int             bootIndex )
{
    ttUserBtEntryPtr userBtEntryPtr;
    ttBtEntryPtr     btEntryPtr;
    ttDeviceEntryPtr devPtr;
    int              errorCode;
    int              maxEntry;

    devPtr = (ttDeviceEntryPtr)interfaceHandle;

    userBtEntryPtr = (ttUserBtEntryPtr)0;
    errorCode = tfValidInterface(devPtr);
    if ( errorCode == TM_ENOERROR )
    {
/* Lock the device */
        tm_call_lock_wait(&(devPtr->devLockEntry));
/* Check that the index is valid (tfBtMap() does not check on that) */
        if (type == TM_BT_CONF)
        {
#ifdef TM_SINGLE_INTERFACE_HOME
            maxEntry = 1;
#else /* TM_SINGLE_INTERFACE_HOME */
#ifdef TM_USE_DHCP_COLLECT
/* If the user is controlling offer selection, we may need to access
 * entries that are not yet bound. */
            maxEntry = TM_MAX_IPS_PER_IF;
#else /* !TM_USE_DHCP_COLLECT */
            maxEntry = (int)(devPtr->devMhomeUsedEntries);
#endif /* !TM_USE_DHCP_COLLECT */
#endif /* TM_SINGLE_INTERFACE_HOME */
        }
        else
        {
            maxEntry = (int)(devPtr->devBtUserMaxEntries);
        }
        if (index < maxEntry)
        {
            btEntryPtr = tfBtMap(devPtr, index, (tt8Bit)type,
                                 (tt8Bit)bootIndex);
            if (btEntryPtr != (ttBtEntryPtr)0)
            {
                if (btEntryPtr->btDhcpState == TM_DHCPS_BOUND
#ifdef TM_USE_DHCP_COLLECT
/* The user may need to get the latest DHCP offer from within the offer
 * callback function. */
                ||  (   bootIndex == TM_BOOT_DHCP
                    &&  btEntryPtr->btDhcpState > TM_DHCPS_INIT
                    &&  devPtr->devOfferFuncPtr != (ttUserDhcpOfferCBFuncPtr)0
                    &&  tm_ip_not_zero(btEntryPtr->btYiaddr) )
#endif /* TM_USE_DHCP_COLLECT */
                   )
                {
                    userBtEntryPtr
                            = &btEntryPtr->btOfferEntryPtr->btUserEntry;
                }
            }
        }
/* Unlock the device */
        tm_call_unlock(&(devPtr->devLockEntry));
    }
    return userBtEntryPtr;
}

#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4BootuiDummy = 0;
#endif /* ! TM_USE_IPV4 */
