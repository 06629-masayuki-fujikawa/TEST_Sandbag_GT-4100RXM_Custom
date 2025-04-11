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
 * Description:
 *  Common functions between 802.3 and Ethernet link-layers
 *
 * Filename: trethcom.c
 * Author: Ed Remmell
 * Date Created:
 * $Source: source/trethcom.c $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:47:21JST $
 * $Author: odile $
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

/* constant declarations and enumerations */

/* type definitions */

/* local function prototypes */

/* local variable definitions */

/* static const tables */

/* macro definitions */

#if ( defined(TM_IGMP) || defined(TM_USE_IPV6))
/* Reinitialize list of multicast addresses in Ethernet controller, this
 * calls the driverIoctl function with the flag TM_DEV_SET_MCAST_LIST
 */
int tfEtherReinitDriverMcast(ttUserInterface interfaceHandle)
{
    ttDeviceEntryPtr    devEntryPtr;
    int                 errorCode;

    devEntryPtr = (ttDeviceEntryPtr) interfaceHandle;
    errorCode = TM_ENOERROR; /* assume successful */

/* lock the device to serialize access to the list of multicast addresses */
    tm_call_lock_wait(&(devEntryPtr->devLockEntry));

    if (devEntryPtr->devLnkMcastEntries != TM_16BIT_ZERO)
    {
          errorCode = (*devEntryPtr->devIoctlFuncPtr)(
                                 interfaceHandle,
                                 TM_DEV_SET_MCAST_LIST,
                                 (ttVoidPtr)(devEntryPtr->devLnkMcastPtr),
                                 (int)devEntryPtr->devLnkMcastEntries );
    }

/* unlock the device */
    tm_call_unlock(&(devEntryPtr->devLockEntry));

    return errorCode;
}

/*
 * tfEtherSetMcastAddr Function description
 * Adds (command == TM_LNK_ADD_MCAST) or deletes
 * (command == TM_LNK_DEL_MCAST) a group address to a list of device enabled
 * multicast addresses. This function is called with the device locked.
 *
 * 0. Initialize ioctl command flag ioctlFlag to 0.
 * 1. If this is not a specific group command:
 * If command is TM_LNK_ADD_MCAST:
 * . increase devLnkMcastAllAddresses entry.
 * . if devLnkMcastAllAddresses is one, set ioctl command to
 *   TM_DEV_SET_ALL_MCAST flag.
 * else If command is TM_LNK_DEL_MCAST:
 * . decrease devLnkMcastAllEntries.
 * . if devLnkMcastAllEntries is zero, set ioctl command to
 *  TM_DEV_SET_MCAST_LIST flag.
 * 2. Else
 * 2.1 Check that the devLnkMcastPtr is non null. If non null devLnkMcastPtr
 *     points to an array of ethernet multicast addresses, followed by an
 *     array of owner counts for those addresses; check if the ethernet
 *     multicast address has already been added to the list.
 * 2.2 If command is TM_LNK_ADD_MCAST:
 * 2.2.1 If the ethernet multicast address has already been added to the list,
 *       increase an owner count for that ethernet entry, and exit.
 * 2.2.2 If not found in the list, check if there is room in the array
 * 2.2.3 If no room in the array or null devLnkMcastPtr, allocate
 *       with tm_get_raw_buffer(Length). Length should be
 *       tvEnetMcastBlockEntries * 8 (initialized to TM_BUF_Q1_SIZE) for a
 *       first allocation, double this for a second allocation etc..
 *       but limit number of successive allocations to a globally known
 *       value tvEnetMcastMaxAllocations initialized by default to 1.
 *       If succesfull, If current devLnkMcastPtr is non nil,
 *       bcopy its content to the new buffer, and free current devLnkMcastPtr.
 *       Increase devLnkMcastAllocations field. Set devLnkAllocMcastEntries
 *       to Length/8. Make devLnkMcastPtr point to the new buffer. Owner Count
 *       array is at offset devLnkAllocMcastEntries * sizeof(ttEnetAddress).
 *       If failed to allocate, exit with TM_ENOBUFS errorcode
 * 2.2.4 Add entry and increase devMcastEntries. Set ownerCount for the entry
 *       to 1. If devLnkMcastAll is zero, set the ioctl command to
 *       TM_DEV_SET_MCAST_LIST.
 * 2.3 Else If command is TM_LNK_DEL_MCAST:
 * 2.3.1 If the ethernet multicast address has already been added to the list,
 *       decrease an owner count for that ethernet entry. If owner count is
 *       zero, decrease devMcastEntries, remove entry from the list,
 *       and move following entries in both arrays down. If owner count is
 *       zero, and If devLnkMcastAll is zero, set the ioctl command to
 *       TM_DEV_SET_MCAST_LIST.
 * END of ELSE part from 2.3
 * END of ELSE part from 2.
 * 3. If ioctl command is set call driver specific ioctl function using
 *    ioctlFlag, devLnkMcastPtr, devLnkMcastEntries.
 * Note:
 * The zero IP host group address is used only for a multicast router to
 * tell the driver to enable reception of all multicast packets (only
 * needed for a multicast router. Not implemented here.) )
 * 
 * Parameters
 * Parameter    Description
 * devEntryPtr  Pointer to interface to which we want to add/remove the host
 *              group address membership.
 * enetAddrPtr  Pointer to Ethernet address to add/remove from this interface.
 * specGroupCmd Boolean indicating whether this is a specific (single multicast
 *              address) or general (all multicast addresses) command
 * command      One of TM_LNK_ADD_MCAST, or TM_LNK_DEL_MCAST
 * 
 * Return value     meaning
 * TM_ENOERROR      success
 * TM_ENOBUFS       Not enough memory to execute the command
 * TM_ENOENT        Entry to delete is not in the list
 * TM_EADDRNOTAVAIL Multicast bit not enabled, or address is not multicast IP
 */
int tfEtherSetMcastAddr( ttDeviceEntryPtr devEntryPtr,
                         ttEnetAddress    enetAddress,
                         tt8Bit           specGroupCmd,
                         int              command)      
{
    ttEnetAddressPtr    enetMcastListPtr;
    tt16BitPtr          ownerCountPtr;
    unsigned            index;
    unsigned            enetEntries;
    int                 ioctlCommand;
    int                 errorCode;
    int                 numberAllocations;
    int                 numberEntriesToMove;
    tt8Bit              needAllocate;
    tt8Bit              addEntry;

    errorCode = TM_ENOERROR;
    ownerCountPtr = (tt16BitPtr)0;
    ioctlCommand = 0;
    if ( specGroupCmd == TM_8BIT_ZERO )
    {
/* No specific group address */
        if (command == TM_LNK_ADD_MCAST)
/* Enable reception of all multicast addresses */
        {
            devEntryPtr->devLnkMcastAllAddresses++;
            if (devEntryPtr->devLnkMcastAllAddresses == 1)
/* First time all multicast addressses are enabled */
            {
                ioctlCommand = TM_DEV_SET_ALL_MCAST;
            }
        }
        else
        {
            if (    (command == TM_LNK_DEL_MCAST)
                 && (devEntryPtr->devLnkMcastAllAddresses != 0) )
            {
                devEntryPtr->devLnkMcastAllAddresses--;
                if (devEntryPtr->devLnkMcastAllAddresses == 0)
                {
/*
 * No more enabling all ethernet addresses. Only enable reception of
 * specific multicast address in the list.
 */
                    ioctlCommand = TM_DEV_SET_MCAST_LIST;
                }
            }
        }
    }
    else
/* Specific group address */
    {
/* Valid multicast address, and device is multicast enabled */
        addEntry = TM_8BIT_ZERO;
        needAllocate = TM_8BIT_ZERO;
/* Pointer to list of Ethernet multicast entries */
        enetMcastListPtr = (ttEnetAddressPtr)
            (devEntryPtr->devLnkMcastPtr);
/* Number of Ethernet multicast entries */
        enetEntries = (unsigned)devEntryPtr->devLnkMcastEntries;
        index = 0;
        if (enetMcastListPtr != (ttEnetAddressPtr)0)
/* List of Ethernet multicast entries previously allocated */
        {
            ownerCountPtr = devEntryPtr->devLnkMcastOwnerCountPtr;
            if (ownerCountPtr == (tt16BitPtr)0)
            {
#ifdef TM_ERROR_CHECKING
                tfKernelError( "tfEtherMcast",
                               "Corrupted device Link Mcast Pointer");
#endif /* TM_ERROR_CHECKING */
            }
            else
            {
/*
* devLnkMcastPtr points to an array of ethernet multicast addresses,
* followed by an array of owner counts for those addresses; check if the
* ethernet multicast address has already been added to the list.
*/
                while ( index < enetEntries )
                {
                    if ( tm_ether_match( enetMcastListPtr[index],
                                         enetAddress ) )
                    {
                        break;
                    }
                    index++;
                }
                if (index < enetEntries)
/* Found one entry. Increase/decrease the owner count */
                {
                    if (command == TM_LNK_DEL_MCAST)
                    {
                        ownerCountPtr[index] = (tt16Bit)
                            (ownerCountPtr[index] - 1);
                        if (ownerCountPtr[index] == TM_16BIT_ZERO)
                        {
/* Delete entry and move all mcast entries and owner count entries down */
                            numberEntriesToMove = 
                                (int)(enetEntries - index - 1);
                            while (numberEntriesToMove != 0)
                            {
/* Avoid overlapping bcopy by copying up one entry at a time */
                                tm_bcopy( (enetMcastListPtr+index+1),
                                          (enetMcastListPtr+index),
                                          sizeof(ttEnetAddress));
                                ownerCountPtr[index] = ownerCountPtr[index+1];
                                index++;
                                numberEntriesToMove--;
                            }
                            if (devEntryPtr->devLnkMcastAllAddresses == 0)
                            {
/*
 * All ethernet addresses not enabled. Only enable reception of
 * specific multicast address in the list.
 */
                                ioctlCommand = TM_DEV_SET_MCAST_LIST;
                            }
                            devEntryPtr->devLnkMcastEntries = (tt16Bit)
                                (enetEntries - 1);
                        }
                    }
                    else
                    {
                        ownerCountPtr[index] = (tt16Bit)
                            (ownerCountPtr[index] + 1);
                    }
                }
                else
/* Did not find an entry in the list of Ethernet multicast addresses */
                {
                    if (command == TM_LNK_DEL_MCAST)
/* Entry to delete was not found */
                    {
                        errorCode = TM_ENOENT;
                    }
                    else
/* command == TM_LNK_ADD_MCAST. Need to add a brand new entry */
                    {
                        if (   enetEntries
                               < (unsigned)
                               devEntryPtr->devLnkMcastAllocEntries )
/* Room for it */
                        {
                            addEntry = TM_8BIT_YES;
                        }
                        else
                        {
/* Reallocation */
                            needAllocate = TM_LNK_MCAST_REALLOCATION;
                        }
                    } 
                }
            }
        }
        else
        {
/* Empty list of Ethernet Mcast addresses */
            if (command == TM_LNK_DEL_MCAST)
            {
/* ENOENT error */
                errorCode = TM_ENOENT;
            }
            else
            {
/* Allocate */
                needAllocate = TM_LNK_MCAST_ALLOCATION;
            }
        }
        if (needAllocate != TM_8BIT_ZERO)
/* No room for it. Allocate entries */
        {
            errorCode = TM_ENOBUFS;
            numberAllocations = (int)devEntryPtr->devLnkMcastAllocations;
            if (   numberAllocations
                   < tm_context(tvEnetMcastMaxAllocations) )
            {
                numberAllocations++;
                enetMcastListPtr =
                    (ttEnetAddressPtr) tm_get_raw_buffer(
                        (unsigned)tm_context(tvEnetMcastBlockEntries)
                        *  8 * (unsigned)numberAllocations );
                if (enetMcastListPtr != (ttEnetAddressPtr)0)
                {
                    errorCode = TM_ENOERROR;
                    devEntryPtr->devLnkMcastAllocEntries = (tt16Bit)
                        (   devEntryPtr->devLnkMcastAllocEntries
                            + (tt16Bit)tm_context(tvEnetMcastBlockEntries) );
                    devEntryPtr->devLnkMcastAllocations =
                        (tt8Bit)numberAllocations;
                    ownerCountPtr = (tt16BitPtr)(ttVoidPtr)
                        (enetMcastListPtr+
                         devEntryPtr->devLnkMcastAllocEntries);
                    if (needAllocate == TM_LNK_MCAST_REALLOCATION)
/* Need to copy current entries */
                    {
                        tm_bcopy( devEntryPtr->devLnkMcastPtr,
                                  enetMcastListPtr,
                                  enetEntries * sizeof(ttEnetAddress) );
                        tm_bcopy( devEntryPtr->devLnkMcastOwnerCountPtr,
                                  ownerCountPtr,
                                  enetEntries * sizeof(tt16Bit) );
                        
/* Free previously allocated block */
                        tm_free_raw_buffer( (ttRawBufferPtr)
                                            devEntryPtr->devLnkMcastPtr );
                    }
                    devEntryPtr->devLnkMcastPtr = enetMcastListPtr;
                    devEntryPtr->devLnkMcastOwnerCountPtr = ownerCountPtr;
                    addEntry = TM_8BIT_YES;
                }
            }
        }
        if (addEntry != TM_8BIT_ZERO)
        {
/* 
 * PC-LINT: If addEntry is non null, then enetMcastListPtr, 
 * and onwerCountPtr are non null 
 */
/* Room to copy the entry */
#ifdef TM_LINT
LINT_NULL_PTR_BEGIN(enetMcastListPtr)
#endif /* TM_LINT */

            tm_ether_copy( enetAddress,
                           (tt8BitPtr)(enetMcastListPtr+enetEntries) );
#ifdef TM_LINT
LINT_NULL_PTR_END(enetMcastListPtr)
#endif /* TM_LINT */

            devEntryPtr->devLnkMcastEntries++;
#ifdef TM_LINT
LINT_NULL_PTR_BEGIN(ownerCountPtr)
#endif /* TM_LINT */

            ownerCountPtr[enetEntries] = 1;
#ifdef TM_LINT
LINT_NULL_PTR_END(ownerCountPtr)
#endif /* TM_LINT */

            if (devEntryPtr->devLnkMcastAllAddresses == 0)
            {
/*
 * All ethernet addresses not enabled. Only enable reception of
 * specific multicast address in the list.
 */
                ioctlCommand = TM_DEV_SET_MCAST_LIST;
            }
        }
        
    }
    if (ioctlCommand != 0)
    {
        if ( devEntryPtr->devIoctlFuncPtr != (ttDevIoctlFuncPtr)0 )
        {
            errorCode = (*devEntryPtr->devIoctlFuncPtr)(
                                     (ttUserInterface)devEntryPtr,
                                     ioctlCommand,
                                     (ttVoidPtr)(devEntryPtr->devLnkMcastPtr),
                                     (int)devEntryPtr->devLnkMcastEntries );
        }
    }
    return errorCode;
}
#endif /* TM_IGMP || TM_USE_IPV6 */

/****************************************************************************
* FUNCTION: tfEtherCommonClose
*
* PURPOSE: Common close function between 802.3 and Ethernet link-layers
*
* PARAMETERS:
*   interfaceId: interface handle
*
* RETURNS:
*   TM_LL_OKAY
*
* NOTES:
*
****************************************************************************/
int tfEtherCommonClose(void TM_FAR *interfaceId)
{
#ifdef TM_USE_IPV6
    ttDeviceEntryPtr devEntryPtr;

    devEntryPtr = (ttDeviceEntryPtr)interfaceId;
    tm_16bit_clr_bit( devEntryPtr->dev6Flags,
                      (TM_6_LL_CONNECTING | TM_6_LL_CONNECTED) );
#ifdef TM_SNMP_MIB
#ifdef TM_USE_NG_MIB2
    tm_kernel_set_critical;
    tm_context(tvIpData).ipv6InterfaceTableLastChange = 
        tm_snmp_time_ticks(tvTime);
    tm_kernel_release_critical;
#endif /* TM_USE_NG_MIB2 */
#endif /* TM_SNMP_MIB */
#else /* !TM_USE_IPV6 */
    TM_UNREF_IN_ARG(interfaceId);
#endif /* !TM_USE_IPV6 */
    return TM_LL_OKAY;
}

/****************************************************************************
* FUNCTION: tfGetEthernetBuffer
*
* PURPOSE: Common get buffer function between 802.3 and Ethernet link-layers
*
* PARAMETERS:
*   userBufferPtr: A pointer to a ttUserBuffer variable that the user buffer
*       handle is stored
*
* RETURNS:
*   A char * to the beginning of the data area to store the received data
*   into or a NULL pointer if there is no memory to complete the operation
*
* NOTES:
*
****************************************************************************/
char TM_FAR *tfGetEthernetBuffer(ttUserBufferPtr userBufferPtr)
{
    ttPacketPtr  packetPtr;
    char TM_FAR *dataPtr;

/*
 * TM_ETHER_HW_ALIGN specifies what type of boundary (4 byte, 16 byte, etc)
 * the ethernet buffer is aligned on.  This is useful for devices that require
 * a DMA'ed receive buffer to be aligned on a certain boundary.
 *
 * TM_ETHER_IP_ALIGN is added to the Ethernet
 * buffer so that the TCP/IP header is aligned on a 4-byte boundary.
 * This makes the Ethernet buffer not aligned on a TM_ETHER_HW_ALIGN
 * (4-byte, 16-byte, etc.) boundary anymore.
 *
 * Device drivers that require that the Ethernet buffer be aligned on
 * TM_ETHER_HW_ALIGN boundary, then have to subract TM_ETHER_IP_ALIGN to the
 * pointer returned by this routine.
 */
    packetPtr = tfGetSharedBuffer(
#ifdef TM_DSP
                                    0,
                                    (ttPktLen)(TM_ETHER_MAX_PACKET_CRC),
#else /* !TM_DSP */
                                    TM_ETHER_HW_ALIGN,
                                    (ttPktLen)( TM_ETHER_MAX_PACKET_CRC +
                                                TM_ETHER_IP_ALIGN ),
#endif /* TM_DSP */
                                    TM_16BIT_ZERO );
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
/* Owned by the user device driver */
        packetPtr->pktUserFlags = TM_PKTF_USER_DEV_OWNS;
#ifndef TM_DSP
        if (   ((ttUserPtrCastToInt) packetPtr->pktLinkDataPtr)
               & (ttUserPtrCastToInt)(TM_ETHER_HW_ALIGN - 1) )
        {
/* Align the data area (according to TM_ETHER_HW_ALIGN) */
            packetPtr->pktLinkDataPtr = (tt8BitPtr)
                (((ttUserPtrCastToInt) packetPtr->pktLinkDataPtr)
                 & ~((ttUserPtrCastToInt) (TM_ETHER_HW_ALIGN - 1)) );
        }
/*
 * Offset of ethernet frame to allow IP area to be long word aligned.
 */
        packetPtr->pktLinkDataPtr += TM_ETHER_IP_ALIGN;
#endif /* !TM_DSP */
        dataPtr = (char TM_FAR *)packetPtr->pktLinkDataPtr;
        *userBufferPtr = (ttUserBuffer)packetPtr;
    }
    else
    {
        *userBufferPtr = (ttUserBuffer)0;
        dataPtr = (char TM_FAR *)0;
    }
    return(dataPtr);
}

#if defined(TM_USE_IPV4) || defined(TM_6_USE_DAD)
tt8Bit tfEtherMatchOurs(ttDeviceEntryPtr devPtr, tt8BitPtr enetAddressPtr)
{
    tt8Bit  match;

    match = TM_8BIT_NO;
#ifdef TM_SINGLE_INTERFACE_HOME
    if ( tm_ether_match(enetAddressPtr, devPtr->devPhysAddrArr) )
    {
/* Ethernet address is ours */
        match = TM_8BIT_YES;
    }
#else /* TM_SINGLE_INTERFACE_HOME */
/* Scan all devices to see if we originated the ARP request/probe/reply */
/* LOCK the device list */
    tm_lock_wait(&tm_context(tvDeviceListLock));
    devPtr = tm_context(tvDeviceList);
    while (devPtr != TM_DEV_NULL_PTR)
    {
        if (    tm_4_ll_is_lan(devPtr)
             && tm_16bit_one_bit_set(devPtr->devFlag, TM_DEV_OPENED) )
        {
            if ( tm_ether_match(enetAddressPtr, devPtr->devPhysAddrArr) )
            {
/* Ethernet address is ours */
                match = TM_8BIT_YES;
                break; /* out of the loop */
            }
        }
        devPtr = devPtr->devNextDeviceEntry;
    }
/* UNLOCK the device list */
    tm_unlock(&tm_context(tvDeviceListLock));
#endif /* !TM_SINGLE_INTERFACE_HOME */
    return match;
}
#endif /* TM_USE_IPV4 || TM_6_USE_DAD */

/***************** End Of File *****************/
