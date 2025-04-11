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
 * Description: common socket interface for DHCP and BOOTP
 *
 * Filename: trbtdhcp.c
 * Author: Odile
 * Date Created: 06/24/98
 * $Source: source/trbtdhcp.c $
 *
 * Modification History
 * $Revision: 6.0.2.4 $
 * $Date: 2011/02/05 00:58:32JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>

#ifdef TM_USE_IPV4

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/*
 * Local variables
 */


/*
 * Maximum amount of time (in seconds) to wait before retransmitting a 
 * DHCP Discover or Request.
 */
#define TM_BOOT_MAX_DELAY       64

/* btFlags bits */
/* #define TM_BOOTF_SOCKET_OPENED  1 */ /* In trmacro.h */

/*
 * Local functions
 */

static void TM_CODE_FAR tfBtRecvCB(int  socketDescriptor,
                                   int  socketCBFlags);

static ttBtEntryPtr tfBtMatchByXid(ttBtEntryPtrPtr btEntryPtrPtr,
                                   int             max,
                                   tt32Bit         xid );

/*
 * Local macros
 */
#ifdef TM_DSP
#define tm_btdhcp_byte_copy(srcPtr, dest, srcOffset, length, bytePos) \
    tfMemCopyOffset( (int*) (srcPtr), (bytePos) + (srcOffset),        \
                     (int *) (&(dest)), 0, (length))
#else /* TM_DSP */
#define tm_btdhcp_byte_copy(srcPtr, dest, srcOffset, length, bytePos) \
    tm_bcopy( &((srcPtr)[srcOffset]), (&(dest)), length )
#endif /* TM_DSP */

#ifdef TM_DSP
#define tm_dsp_put_8bit(buffer,index,dataChar) \
{ \
    buffer[tm_packed_byte_count((index)+1)-1] &= \
        (~(0xff << ((TM_DSP_ROUND_PTR - \
        ((index) % TM_DSP_BYTES_PER_WORD)) << 3))); \
    buffer[tm_packed_byte_count((index)+1)-1] |= \
        (dataChar << ((TM_DSP_ROUND_PTR - \
        ((index) % TM_DSP_BYTES_PER_WORD)) << 3)); \
}
#define tm_btdhcp_set_char(srcChar, destPtr, destOffset, bytePos) \
    tm_dsp_put_8bit((destPtr), ((destOffset)+(bytePos)), (srcChar))
#else /* !TM_DSP */
#define tm_btdhcp_set_char(srcChar, destPtr, destOffset, bytePos) \
    (destPtr)[(destOffset)] = (srcChar)
#endif /* TM_DSP */

/*
 * tfBtOpenSocket function description:
 * Open the BOOT/DHCP socket. Handle concurrent use (BOOTP/DHCP/other devices).
 *
 * Set errorCode to no error, and initialize device entry pointer
 * If the BOOTP/DHCP socket has not yet been opened by this boot/dhcp entry:
 * 1. Lock the boot socket lock
 * 2. If BOOTP/DHCP socket is not opened
 * 2.1 Open a UDP socket
 * 2.2 Bind the UDP socket to the device (if TM_USE_STRONG_ESL is defined)
 * 2.3 Set the SO_REUSEPORT socket option in case a user wants to bind to
 *     the same port, or in case the user starts DHCP on multiple devices.
 * 2.4 Bind it to the UDP BOOTP client port
 * 2.5 register a receive call back function tfBtRecvCB() which will be
 *     called every time a BOOTP or DHCP packet is received for the
 *     BOOTP or DHCP client.
 * 2.6 If no error initialize the global socket descriptor. In STRONG_ESL
 *     case check for concurrent opening of the socket.
 * 2.7 Close the socket if any error occured
 * 3. If no error
 * 3.1 Mark the socket as opened for this BOOTP/DHCP entry
 * 3.2 Increase the opened socket count
 * 4. Unlock the boot socket lock
 * Return errorCode
 *
 * Parameters
 * btEntryPtr       Pointer to BOOT entry
 *
 * Return value     meaning
 * TM_ENOERROR      success
 * TM_EADDRINUSE    A UDP socket is already bound to the UDP port
 * TM_EMFILE        No more sockets
 */
int tfBtOpenSocket (ttBtEntryPtr btEntryPtr)
{
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr    devPtr;
#endif /* TM_USE_STRONG_ESL */
    ttSockAddrPtrUnion  tempSockAddr;
    struct sockaddr_in  address;
    int                 socketDescriptor;
    int                 errorCode;
#ifdef TM_USE_REUSEPORT
    int                 tempInt;
#endif /* TM_USE_REUSEPORT */

    errorCode = TM_ENOERROR;
#ifdef TM_USE_STRONG_ESL
    devPtr = btEntryPtr->btDevEntryPtr;
#endif /* TM_USE_STRONG_ESL */
    if (!tm_8bit_one_bit_set(btEntryPtr->btFlags, TM_BOOTF_SOCKET_OPENED))
    {
/* Lock the Boot Socket lock */
        tm_call_lock_wait(&tm_context(tvBootSocketLockEntry));
#ifdef TM_USE_STRONG_ESL
        if (devPtr->devBootSocketDescriptor == TM_SOCKET_ERROR)
#else /* !TM_USE_STRONG_ESL */
        if (tm_context(tvBootSocketDescriptor) == TM_SOCKET_ERROR) 
#endif /* !TM_USE_STRONG_ESL */
        {
#ifdef TM_USE_STRONG_ESL
            tm_assert(tfBtOpenSocket, devPtr->devBootSocketOpenCount == 0);
#else /* !TM_USE_STRONG_ESL */
            tm_assert(tfBtOpenSocket, tm_context(tvBootSocketOpenCount) == 0);
#endif /* !TM_USE_STRONG_ESL */
/* Socket not opened yet */
            tempSockAddr.sockInPtr = &address;
            socketDescriptor = socket(PF_INET, SOCK_DGRAM, IP_PROTOUDP);
            if (socketDescriptor == TM_SOCKET_ERROR)
            {
                errorCode = TM_SOCKET_ERROR;
            }
            else
            {
#ifdef TM_USE_STRONG_ESL
/* bind to the device */
                errorCode = setsockopt(socketDescriptor,
                                       SOL_SOCKET,
                                       SO_BINDTODEVICE,
                                       (char *)(devPtr->devNameArray),
                                       IFNAMSIZ);
                if (errorCode != TM_SOCKET_ERROR)
#endif /* TM_USE_STRONG_ESL */
                {
#ifdef TM_USE_REUSEPORT
                    tempInt = 1;
                    errorCode = setsockopt(socketDescriptor,
                                           SOL_SOCKET,
                                           SO_REUSEPORT,
                                           (const char TM_FAR *)&tempInt,
                                           sizeof(int) );
                    if (errorCode == TM_ENOERROR)
#endif /* TM_USE_REUSEPORT */
                    {
/* bind to the port */
                        address.sin_family = PF_INET;
/* network byte order */
                        address.sin_port = TM_BOOTPC_PORT;
                        tm_ip_copy(TM_IP_ZERO, address.sin_addr.s_addr);
                        errorCode = bind(socketDescriptor,
                                         tempSockAddr.sockPtr,
                                         sizeof(struct sockaddr_in));
                        if (errorCode == TM_ENOERROR)
                        {
/*
 * NOTE: We keep the TTL at its default value for a socket to allow
 * usage of relay agent
 */
                            errorCode = tfRegisterSocketCB(socketDescriptor,
                                                           tfBtRecvCB,
                                                           TM_CB_RECV);
                            if (errorCode == TM_ENOERROR)
                            {
#ifdef TM_USE_STRONG_ESL
                                devPtr->devBootSocketDescriptor =
                                                             socketDescriptor;
#else /* !TM_USE_STRONG_ESL */
                                tm_context(tvBootSocketDescriptor) =
                                                             socketDescriptor;
#endif /* TM_USE_STRONG_ESL */
                            }
                        }
                    }
                }
            }
            if (errorCode != TM_ENOERROR)
            {
                errorCode = tfGetSocketError(socketDescriptor);
                if (socketDescriptor != TM_SOCKET_ERROR)
                {
                    (void)tfClose(socketDescriptor);
                }
            }
        }
        if (errorCode == TM_ENOERROR)
        {
            btEntryPtr->btFlags = (tt8Bit)(  btEntryPtr->btFlags
                                           | TM_BOOTF_SOCKET_OPENED);
#ifdef TM_USE_STRONG_ESL
            devPtr->devBootSocketOpenCount++;
#else /* !TM_USE_STRONG_ESL */
            tm_context(tvBootSocketOpenCount)++;
#endif /* !TM_USE_STRONG_ESL */
        }
/* Unlock the Boot Socket lock */
        tm_call_unlock(&tm_context(tvBootSocketLockEntry));
    }
    else
    {
#ifdef TM_USE_STRONG_ESL
        tm_assert(tfBtOpenSocket,
                     (devPtr->devBootSocketOpenCount > 0)
                  && (devPtr->devBootSocketDescriptor != TM_SOCKET_ERROR));
#else /* TM_USE_STRONG_ESL */
        tm_assert(tfBtOpenSocket,
                     (tm_context(tvBootSocketOpenCount) > 0)
                  && (tm_context(tvBootSocketDescriptor) != TM_SOCKET_ERROR));
#endif /* TM_USE_STRONG_ESL */
    }
    return errorCode;
}

/*
 * tfBtCloseSocket function description:
 * Close the BOOT/DHCP socket. Handle concurrent use (BOOTP/DHCP/other devices).
 * Device unlocked while the socket is closed.
 *
 * Set errorCode to no error
 * If the BOOTP/DHCP socket has been opened by this boot/dhcp entry:
 * 1. Initialize local socket descriptor to TM_SOCKET_ERROR
 * 2. Lock the boot socket lock
 * 3. If boot open socket count is positive:
 * 3.1 Decrease the boot open socket count
 * 3.2 If boot open socket count is zero
 * 3.2.1 save the boot socket descriptor in local variable
 * 3.2.2 reset the boot socket descriptor in global variable.
 * 3.3 Reset flag indicating that the BOOTP/DHCP socket is opened by this
 *     bootp/dhcp entry
 * 3.4 UnLock the boot socket lock
 * 4. If local socket descriptor is not TM_SOCKET_ERROR
 * 4.1 Unlock the device
 * 4.2 close the socket
 * 4.3 check for error
 * 4.4 relock the device
 * Return errorCode
 *
 * Parameters
 * btEntryPtr       Pointer to BOOT entry
 *
 * Return value     meaning
 * TM_ENOERROR      success
 * Other            as returned by tfClose
 */
int tfBtCloseSocket (ttBtEntryPtr btEntryPtr)
{
    ttDeviceEntryPtr   devPtr = NULL;
    int                errorCode;
    int                socketDescriptor;

    errorCode = TM_ENOERROR;
    if (tm_8bit_one_bit_set(btEntryPtr->btFlags, TM_BOOTF_SOCKET_OPENED))
    {
        socketDescriptor = TM_SOCKET_ERROR;
        devPtr = btEntryPtr->btDevEntryPtr;
/* Lock the Boot Socket lock */
        tm_call_lock_wait(&tm_context(tvBootSocketLockEntry));
#ifdef TM_USE_STRONG_ESL
        tm_assert(tfBtCloseSocket,
                     (devPtr->devBootSocketOpenCount > 0)
                  && (devPtr->devBootSocketDescriptor != TM_SOCKET_ERROR));
        if (devPtr->devBootSocketOpenCount > 0)
        {
            devPtr->devBootSocketOpenCount--;
            if (devPtr->devBootSocketOpenCount == 0)
            {
                socketDescriptor = devPtr->devBootSocketDescriptor;
                devPtr->devBootSocketDescriptor = TM_SOCKET_ERROR;
            }
        }
#else /* TM_USE_STRONG_ESL */
        tm_assert(tfBtCloseSocket,
                     (tm_context(tvBootSocketOpenCount) > 0)
                  && (tm_context(tvBootSocketDescriptor) != TM_SOCKET_ERROR));
        if (tm_context(tvBootSocketOpenCount) > 0)
        {
            tm_context(tvBootSocketOpenCount)--;
            if (tm_context(tvBootSocketOpenCount) == 0)
            {
                socketDescriptor = tm_context(tvBootSocketDescriptor);
                tm_context(tvBootSocketDescriptor) = TM_SOCKET_ERROR;
            }
        }
#endif /* TM_USE_STRONG_ESL */
        btEntryPtr->btFlags = (tt8Bit)
                              (btEntryPtr->btFlags & ~TM_BOOTF_SOCKET_OPENED);
/* Unlock the Boot Socket Lock */
        tm_call_unlock(&tm_context(tvBootSocketLockEntry));
        if (socketDescriptor != TM_SOCKET_ERROR)
        {
/* Unlock the device */
            tm_call_unlock(&devPtr->devLockEntry);
            errorCode = tfClose(socketDescriptor);
            if (errorCode == TM_SOCKET_ERROR)
            {
                errorCode = tfGetSocketError(socketDescriptor);
            }
/* re-lock the device */
            tm_call_lock_wait(&devPtr->devLockEntry);
        }
    }
	if( devPtr == NULL ){
		;
	}

    return errorCode;
}

/*
 * tfBtAlloc function description:
 * Called by tfBtInit() below, or by tfDhcpCommonSet() when the user
 * wants to set DHCP parameters.
 * . Allocate a boot entry structure associated with the device if it has
 *   not been allocated yet.
 * . Initialize the boot entry structure with the correct type and
 *    index.
 *
 * Parameters       meaning
 * devPtr           Pointer to interface being configured.
 * index            multihome index for a config interface, or free user table
 *                  index if called from tfDhcpUserStart()
 * type             TM_BT_CONF, or TM_BT_USER. 
 *
 * Return value     meaning
 * Non null         btEntryPtr
 * zero             failure.
 */
ttBtEntryPtr tfBtAlloc ( ttDeviceEntryPtr        devPtr,
                         int                     index,
                         tt8Bit                  type,
                         tt8Bit                  bootIndex )
{
    ttBtEntryPtrPtr     btEntryPtrPtr;
    ttBtEntryPtr        btEntryPtr;
    ttBtOfferEntryPtr   btOfferEntryPtr;
    tt32Bit             xid;
    tt32Bit             typeIndex;
    unsigned int        maxEntries;
    unsigned int        allocSize;

    btEntryPtr = (ttBtEntryPtr)0;
    if (type == TM_BT_CONF)
    {
        maxEntries = TM_MAX_IPS_PER_IF;
    }
    else
    {
        maxEntries = (unsigned int)devPtr->devBtUserMaxEntries; 
    }
    btEntryPtrPtr = devPtr->devBtEntryTablePtr[type][bootIndex];
    if (btEntryPtrPtr == (ttBtEntryPtrPtr)0)
/* Allocate auto or user DHCP boot entries table  */
    {
        if (type == TM_BT_USER)
/* User */
        {
            if (maxEntries == 0)
/* Initialize max user entries on the device */
            {
                devPtr->devBtUserMaxEntries =
                                     (tt8Bit)tm_context(tvMaxUserDhcpEntries);
                maxEntries = (unsigned int)devPtr->devBtUserMaxEntries; 
            }
        }
        if ((maxEntries != 0) && ((unsigned int)index < maxEntries))
/* Check index is in range */
        {
            allocSize = sizeof(ttBtEntryPtr) * maxEntries;
            devPtr->devBtEntryTablePtr[type][bootIndex] = (ttBtEntryPtrPtr)
                    tm_kernel_malloc(allocSize);
            btEntryPtrPtr = devPtr->devBtEntryTablePtr[type][bootIndex];
            if (btEntryPtrPtr != (ttBtEntryPtrPtr)0)
            {
                tm_bzero((ttVoidPtr)btEntryPtrPtr, allocSize);
            }
        }
    }
    if (btEntryPtrPtr != (ttBtEntryPtrPtr)0)
    {
        if ((unsigned int)index < maxEntries)
/* index is in range */
        {
            btEntryPtr = tfBtMap(devPtr, index, type, bootIndex);
            if (btEntryPtr == (ttBtEntryPtr)0)
            {
/*
 * Once allocated this will never be freed, and will stay part of the device
 * entry. It is allocated here, as opposed to being part of the device
 * structure, so as not to make a device entry unnecessaryly bigger for
 * static configurations.
 */
                btEntryPtr = (ttBtEntryPtr)tm_kernel_malloc(
                                                (unsigned)sizeof(ttBtEntry));
                if (btEntryPtr == (ttBtEntryPtr)0)
                {
                    goto btAllocExit;
                }
/* We need a minimum of one entry in the BOOTP/DHCP offer cache */
                btOfferEntryPtr = tm_kernel_malloc(sizeof(ttBtOfferEntry));
                if (btOfferEntryPtr == (ttBtOfferEntryPtr)0)
                {
                    tm_kernel_free(btEntryPtr);
                    goto btAllocExit;
                }
                tm_bzero(btEntryPtr, sizeof(ttBtEntry));
                tm_bzero(btOfferEntryPtr, sizeof(ttBtOfferEntry));
                btEntryPtr->btDevEntryPtr = devPtr;
                btEntryPtr->btType = type;
                btEntryPtr->btIndex = (tt16Bit)index;
                btEntryPtr->btBootIndex = bootIndex;
/* Set up BOOTP/DHCP offer cache */
#ifdef TM_USE_DHCP_COLLECT
                btEntryPtr->btCollectSize = devPtr->devDhcpCollectSize;
                btEntryPtr->btCollectTime = devPtr->devDhcpCollectTime;
#endif /* TM_USE_DHCP_COLLECT */
                tfListInit(&btEntryPtr->btOfferEntryList);
                tfListAddToHead(&btEntryPtr->btOfferEntryList,
                                                (ttNodePtr)btOfferEntryPtr);
                btEntryPtr->btOfferEntryPtr = btOfferEntryPtr;
                btOfferEntryPtr->btEntryPtr = btEntryPtr;
/*
 * Initialize XID to a random number. Make sure that no two btEntryPtr have
 * the same XID. We use (user or conf) type, (DHCP or BOOTP) bootIndex, and
 * index to differentiate.
 */
                xid = (tt32Bit)tfGetRandom();
                typeIndex =
                        (tt32Bit)(((tt32Bit)(type << 1)) + (tt32Bit)bootIndex);
                typeIndex =
                    (tt32Bit)(((tt32Bit)(typeIndex << 16)) + (tt32Bit)index);
                xid = (xid & TM_UL(0xFFFC0000)) + (tt32Bit)typeIndex;
                btEntryPtr->btXid = xid;
                btEntryPtrPtr[index] = btEntryPtr;
/* Initialize the option lists */
                tfListInit(&(btEntryPtr->btUserSetCustOpts));
                tfListInit(&(btOfferEntryPtr->btUserServCustOpts));
                tfBtInitUserEntryNames(btEntryPtr);
            }
            else
            {
                if (    (btEntryPtr->btDhcpState == TM_DHCPS_INIT)
                     || (btEntryPtr->btDhcpState == TM_DHCPS_INITREBOOT) )
                {
                    if (btEntryPtr->btDhcpState == TM_DHCPS_INIT)
                    {
/* Discard previously discovered values */
                        tfBtInitUserEntry(btEntryPtr);
                    }
                    else
                    {
/* BOOTP 'secs' field */
                        btEntryPtr->btSeconds = TM_16BIT_ZERO;
                    }
/*
 * Number of retries for BOOTP requests, DHCP Discover, or DHCP requests in
 * rebooting or requesting state.
 */
                    btEntryPtr->btRetries = TM_8BIT_ZERO;
                }
            }
        }
    }
btAllocExit:
    return btEntryPtr;
}
/*
 * tfBtInit function description:
 * Called by tfBootpConfig() (respectively tfDhcpConfig()) when the user
 * issues tfConfigInterface() with a TM_DEV_IP_BOOTP flag (respectively
 * TM_DEV_IP_DHCP), or by tfDhcpUserStart().
 * 1. Allocate a boot entry structure associated with the device if it has
 *    not been allocated yet, and initialize the boot entry structure with
 *    the correct type and index.
 * 2. Initialize the call back function.
 *
 * Parameters       meaning
 * devPtr           Pointer to interface being configured.
 * index            multihome index for a config interface, or free user table
 *                  index if called from tfUserDhcpAddress()
 * bootIndex        Either BOOTP or DHCP.
 *
 * Return value     meaning
 * TM_ENOERROR      success
 * TM_EADDRINUSE    A UDP socket is already bound to the UDP port
 * TM_EMFILE        No more sockets
 * TM_ENOBUFS       not enough memory to allocate the boot structure.
 */
int tfBtInit ( ttDeviceEntryPtr        devPtr,
               ttUserDhcpNotifyFuncPtr dhcpNotifyFuncPtr,
               int                     index,
               tt8Bit                  type,
               tt8Bit                  bootIndex )
{
    ttBtEntryPtr     btEntryPtr;
    int              errorCode;

    btEntryPtr = tfBtAlloc(devPtr, index, type, bootIndex);
    if (btEntryPtr != (ttBtEntryPtr)0)
    {
        btEntryPtr->btDhcpNotifyFuncPtr = dhcpNotifyFuncPtr;
        errorCode = TM_ENOERROR;
    }
    else
    {
        errorCode = TM_ENOBUFS;
    }
    return errorCode;
}

/*
 * tfBtRecvCB() Function Description
 * BOOTPC UDP client socket receive call back function.
 * Called by the UDP/IP stack to signal reception of a packet.
 * 1. Call tfZeroCopyRecvFrom to retrieve incoming packet.
 * 2. Use packet device entry pointer to locate interface packet is for.
 * 3. Discard any truncated header, non reply packets, and packets whose
 *    BOOTP header physical address do not match ours.
 * 4. call the device BOOT/DHCP receive call back function registered by
 *    the BOOTP/DHCP client to give the packet to the appropriate protocol.
 *
 * Parameters:
 * socketDescriptor socketDescritor of the BOOT socket opened in
 *                  tfBtOpenSocket()
 * socketCBFlags    event flag (TM_CB_RECV always, since it is the only
 *                  flag we have registered for)
 *
 * No return value
 */
static void TM_CODE_FAR tfBtRecvCB ( int  socketDescriptor,
                                     int  socketCBFlags )
{
    ttDeviceEntryPtr        devPtr;
    ttBtEntryPtr            btEntryPtr;
    ttPacketPtr             packetPtr;
    ttCharPtr               bufferPtr;
/* point to BOOTP/DHCP header */
    ttBootHeaderPtr         bootHdrPtr;
    ttDevBootRecvFuncPtr    btRecvFuncPtr;
    tt8BitPtr               etherPtrSrc;
    tt8BitPtr               etherPtrDest;
    int                     bufferLen;
    int                     addressLength;
    int                     i;
    int                     j;
    int                     maxEntries;
#ifdef TM_DSP
    int                     tempEtherAddr[6];
#endif /* TM_DSP */
#ifdef TM_KERNEL_WIN32_X86
    tt8Bit                  tempEtherSrcAddr[6];
#endif /* TM_KERNEL_WIN32_X86 */

    TM_UNREF_IN_ARG(socketCBFlags);
    addressLength = 0;
    devPtr = (ttDeviceEntryPtr)0; /* for compiler */
    packetPtr = TM_PACKET_NULL_PTR;
    bufferLen = tfZeroCopyRecvFrom(
            socketDescriptor,
            (ttUserMessage TM_FAR *)(ttVoidPtr)&packetPtr,
            (char TM_FAR * TM_FAR *)&bufferPtr,
            (int)TM_MAX_BOOT_PACKET,
            MSG_DONTWAIT,
            (struct sockaddr TM_FAR *)0,
            &addressLength);
    if (bufferLen == TM_SOCKET_ERROR)
    {
        goto btRecvExit;
    }

/*
 * RFC says to silently discard BOOTP header smaller than
 * TM_BOOTP_HEADER_SIZE, but Linux DHCP server sends DHCP offer
 * with a 272 bytes size ( smaller than the 300 bytes TM_BOOTP_HEADER_SIZE).
 * So instead of checking for a minimum length of TM_BOOTP_HEADER_SIZE (300
 * bytes), we will check for a minimum length of 244 bytes (the BOOTP
 * header with just a minimum of 8 bytes of option (4bytes for the magic
 * field, 4 bytes for the DHCP message type and end tag)).
 */
    if ( bufferLen < TM_BOOTP_MIN_HEADER_SIZE )
    {
        goto btRecvExit; /* return */
    }
    bootHdrPtr = (ttBootHeaderPtr)packetPtr->pktLinkDataPtr;
    devPtr = packetPtr->pktDeviceEntryPtr;
#ifdef TM_ERROR_CHECKING
    if (devPtr == (ttDeviceEntryPtr)0)
    {
        tfKernelError("tfBtRecvCB", "No device entry");
        goto btRecvExit;
    }
#endif /* TM_ERROR_CHECKING */
    tm_call_lock_wait(&(devPtr->devLockEntry));
/* Check that packet is for us */
    etherPtrSrc = &devPtr->devPhysAddrArr[0];
#ifdef TM_KERNEL_WIN32_X86
    tm_bcopy(etherPtrSrc, tempEtherSrcAddr, 6);
    tempEtherSrcAddr[0] = (tt8Bit)0x01; 
    etherPtrSrc = tempEtherSrcAddr;
#endif /* TM_KERNEL_WIN32_X86 */
    etherPtrDest = &bootHdrPtr->bootChaddr[0];

/* For DSP's, need to copy into temporary buffer for tm_ether_match below to
   work correctly. */
#ifdef TM_DSP
#ifdef TM_32BIT_DSP
    tempEtherAddr[0] = (etherPtrDest[0] >> 24) & 0xff;
    tempEtherAddr[1] = (etherPtrDest[0] >> 16) & 0xff;
    tempEtherAddr[2] = (etherPtrDest[0] >> 8 ) & 0xff;
    tempEtherAddr[3] = (etherPtrDest[0]      ) & 0xff;
    tempEtherAddr[4] = (etherPtrDest[1] >> 24) & 0xff;
    tempEtherAddr[5] = (etherPtrDest[1] >> 16) & 0xff;
    
    etherPtrDest = (tt8BitPtr) tempEtherAddr;
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
    tempEtherAddr[0] = (etherPtrDest[0] >> 8) & 0xff;
    tempEtherAddr[1] = (etherPtrDest[0]     ) & 0xff;
    tempEtherAddr[2] = (etherPtrDest[1] >> 8) & 0xff;
    tempEtherAddr[3] = (etherPtrDest[1]     ) & 0xff;
    tempEtherAddr[4] = (etherPtrDest[2] >> 8) & 0xff;
    tempEtherAddr[5] = (etherPtrDest[2]     ) & 0xff;
    
    etherPtrDest = (tt8BitPtr) tempEtherAddr;
#endif /* TM_16BIT_DSP */
#endif /* TM_DSP */
    
    if (   (bootHdrPtr->bootOp != TM_BOOTREPLY)
/* Silently discard non reply BOOTP headers */
        || (!tm_ether_match(etherPtrSrc, etherPtrDest)) )
/* Silently discard BOOTP header whose physical addresses do not match */
    {
        goto btRecvExit;
    }
/* Check if the packet is BOOTP or DHCP */
    btEntryPtr = (ttBtEntryPtr)0;
/* AUTO CONF table size */
    maxEntries = TM_MAX_IPS_PER_IF;
/* Map from Xid to btEntryPtr */
    for (i = 0;
         (i <= TM_BT_USER) && (btEntryPtr == (ttBtEntryPtr)0);
         i++)
    {
        for (j = 0;
             (j < TM_BOOT_MAX_TYPES) && (btEntryPtr == (ttBtEntryPtr)0);
             j++)
        {
            if (devPtr->devBtEntryTablePtr[i][j] != (ttBtEntryPtrPtr)0)
            {
                btEntryPtr = tfBtMatchByXid(
                                 devPtr->devBtEntryTablePtr[i][j],
                                 maxEntries,
                                 bootHdrPtr->bootXid );
            }
        }
/* User table size */
        maxEntries = devPtr->devBtUserMaxEntries;
    }
    if (btEntryPtr == (ttBtEntryPtr)0)
    {
/* No match. Packet is for another client on the local network. */
        goto btRecvExit;
    }
    btRecvFuncPtr = devPtr->devBootInfo[btEntryPtr->btBootIndex].
                                                bootRecvFuncPtr;
    if (btRecvFuncPtr != (ttDevBootRecvFuncPtr)0)
    {
        (*(btRecvFuncPtr))(packetPtr, btEntryPtr);
    }
    else
    {
#ifdef TM_ERROR_CHECKING
        tfKernelWarning("tfBtRecvCB", "No Device BOOT recv Function");
#endif /* TM_ERROR_CHECKING */
    }
btRecvExit:
    if ( devPtr != (ttDeviceEntryPtr)0 )
    {
        tm_call_unlock(&(devPtr->devLockEntry));
    }
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
/* Recv CB function called with socket unlocked */
        tfFreePacket((ttPacketPtr)packetPtr, TM_SOCKET_UNLOCKED);
    }
    return;
}

/*
 * tfBtGetFillHeader function description:
 * Called by BOOTP or DHCP when any requests are being sent on the network.
 * Allocate a BOOT/DHCP header packet, and fill in the BOOTP/DHCP header
 * fields (non including the options field). Some of the fields will be
 * overwritten later by DHCP depending on the DHCP state.
 * Assume Ethernet link layer for the time being. (We could check link layer
 * by looking it up in devPtr, but there is no need to do so now).
 * If a retryTimerFuncPtr is passed, arrange for that function to be
 * called later (exponential backoff time out).
 *
 * Parameters           meaning
 * devPtr               pointer to device
 * bootHeaderSize       Size of BOOTP/DHCP header.
 * retryTimerFuncPtr    Retry timer to be called in exponential backoff time.
 *
 * Return value
 * packetPtr        pointer to BOOTP/DHCP packet, null if failed.
 */
ttPacketPtr tfBtGetFillHeader (ttBtEntryPtr     btEntryPtr,
                               int              bootHeaderSize,
                               ttTmCBFuncPtr    retryTimerFuncPtr)
{
    ttPacketPtr          packetPtr;
    ttBootHeaderPtr      bootHdrPtr;
    ttDeviceEntryPtr     devPtr;
    tt8BitPtr            etherPtrSrc;
    tt8BitPtr            etherPtrDest;
    tt32Bit              timerInterval;
    int                  retries;
    tt16Bit              delay;
    ttGenericUnion       timerParm1;
#ifdef TM_KERNEL_WIN32_X86
    tt8Bit               tempEtherSrcAddr[6];
#endif /* TM_KERNEL_WIN32_X86 */
   
    packetPtr = tfGetSharedBuffer(TM_MAX_SEND_HEADERS_SIZE,
                                  (ttPktLen)bootHeaderSize,
                                  TM_16BIT_ZERO);
    if (packetPtr != TM_PACKET_NULL_PTR)
    {
/* initialize the BOOTP/DHCP message to all 0's */
        tm_bzero(
            packetPtr->pktLinkDataPtr, tm_packed_byte_count(bootHeaderSize));

        devPtr = btEntryPtr->btDevEntryPtr;
/* Only needed for broadcast */
        packetPtr->pktDeviceEntryPtr = devPtr;
        if (btEntryPtr->btType == TM_BT_CONF)
        {
/* 
 * Automatic configuration, i.e. user called tfOpenInterface or 
 * tfConfigInterface. In that case btIndex contains the multi home
 * index.
 */
/* Used to fill in source address (for broadcasts) */
            packetPtr->pktMhomeIndex = btEntryPtr->btIndex;
        }
        else
        {
/* 
 * User configuration, i.e. tfDhcpUserStart() was called.
 * Indicate that source IP address has to be zero. 
 * DHCP will reset this flag, in RENEWING, REBINDING states.
 */
            packetPtr->pktFlags2 |= TM_PF2_ZERO_SRC_IP_ADDRESS;
        }
/* Indicates a zero copy buffer */
        packetPtr->pktUserFlags = TM_PKTF_USER_OWNS;
/* Indicate that pktDeviceEntryPtr is filled (for broadcast) */
        packetPtr->pktFlags |= TM_OUTPUT_DEVICE_FLAG;
/* Point to BOOTP/DHCP header */
        bootHdrPtr = (ttBootHeaderPtr)packetPtr->pktLinkDataPtr;

/* Fill in the BOOTP/DHCP header to be sent to the BOOTP server */

/*
 * RFC 951:
 *   'op' is set to '1', BOOTREQUEST.  'htype' is set to the hardware
 *   address type as assigned in the ARP section of the "Assigned
 *   Numbers" RFC.
 */
        bootHdrPtr->bootOp = TM_BOOTREQUEST;
        bootHdrPtr->bootHtype = TM_ARP_ETHER;
/*
 * RFC 951:  'chaddr' is filled in with the client's hardware address.
 */
        etherPtrSrc = &devPtr->devPhysAddrArr[0];
#ifdef TM_KERNEL_WIN32_X86
/* 
 * Modify the harware client address so that we get a DHCP address different
 * from Windows
 */
        tm_bcopy(etherPtrSrc, tempEtherSrcAddr, 6);
        tempEtherSrcAddr[0] = (tt8Bit)0x01; 
        etherPtrSrc = tempEtherSrcAddr;
#endif /* TM_KERNEL_WIN32_X86 */
        etherPtrDest = &bootHdrPtr->bootChaddr[0];

#ifdef TM_DSP
#ifdef TM_32BIT_DSP
/* Can't use tm_ether_copy on a DSP here because the address must be packed. */
        etherPtrDest[0] = (etherPtrSrc[0] << 24) |
                          (etherPtrSrc[1] << 16) |
                          (etherPtrSrc[2] << 8 ) |
                          (etherPtrSrc[3]      );
        etherPtrDest[1] = (etherPtrDest[1] & 0x0000ffff) | 
                          (etherPtrSrc[4] << 24)         |
                          (etherPtrSrc[5] << 16);
#endif /* TM_32BIT_DSP */
#ifdef TM_16BIT_DSP
/* Can't use tm_ether_copy on a DSP here because the address must be packed. */
        etherPtrDest[0] = (etherPtrSrc[0] << 8) | (etherPtrSrc[1]);
        etherPtrDest[1] = (etherPtrSrc[2] << 8) | (etherPtrSrc[3]);
        etherPtrDest[2] = (etherPtrSrc[4] << 8) | (etherPtrSrc[5]);
#endif /* TM_16BIT_DSP */
#else /* !TM_DSP */
        tm_ether_copy(etherPtrSrc, etherPtrDest);
#endif /* !TM_DSP */

/*
 * RFC 951:
 *  'hlen' is set to the length of the hardware address,
 *   e.g. '6' for 10mb ethernet. Hops is set to zero by clients (in bzero above).
 */
        bootHdrPtr->bootHlen = TM_ETHER_ADDRESS_LENGTH;
/*
 * RFC 951:
 * 'xid' is set to a 'random' transaction id. We do use a random number
 * but also, put an identifier in the entry to enable us to map to a
 * multihome index (for automatic configurations), or user DHCP request,
 * when a reply comes back.
 */
        bootHdrPtr->bootXid = btEntryPtr->btXid;
/*
 * RFC 951;
 *   'secs' is set to the
 *   number of seconds that have elapsed since the client has started
 *   booting (RFC 1542: i.e since starting sending the first BOOTP request).
 *   This will let the servers know how long a client has
 *   been trying.  As the number gets larger, certain servers may feel
 *   more 'sympathetic' towards a client they don't normally service.
 */
        retries = (int)btEntryPtr->btRetries;
        if (    (btEntryPtr->btDhcpState <= TM_DHCPS_SELECTING)
             && (retries >= 1) )
        {
/*
 *  If retransmitting BOOTP request, or DHCP discover, update the 'secs'
 *  field
 */
            delay = \
                (tt16Bit)(devPtr->devBtInitTimeoutIntv << (retries - 1));
/* Limit maximum retransmission delay to 64 seconds, check for overflow */
            if ((delay > TM_BOOT_MAX_DELAY) || (delay == TM_16BIT_ZERO))
            {
                delay = TM_BOOT_MAX_DELAY;
            }
            btEntryPtr->btSeconds = (tt16Bit) (   btEntryPtr->btSeconds
                                                + delay );
        }
        delay = (tt16Bit)btEntryPtr->btSeconds;
        tm_htons(delay, bootHdrPtr->bootSecs);
/*
 * RFC 1542: flags field is now a broadcast bit field.
 * Since we can handle receiving non broadcast packets on a non configured
 * interface, we set it to zero as recommended by RFC 1542 (in bzero above).
 */
/*
 * RFC 951: 'ciaddr' is set to the client's IP address if known, else 0.
 * RFC1542: Safest implementation is to use 0 for 'ciaddr' for BOOTP (in bzero above).
 * RFC2132: for DHCP value depends on the DHCP state (done in tfdhcp.c).
 */
/* 'yiaddr' to be filled in by server (zero in bzero above) */
/* Zero the siaddr field (in bzero above) */
/*
 * RFC 1542:
 *  A BOOTP client MUST set the 'giaddr' field to zero (0.0.0.0) in all
 *  BOOTREQUEST messages it generates (in bzero above).
 * RFC 2131:
 *  Same for DHCP.
 */
/*
 * RFC 951:
 *    If the client wishes to restrict booting to a particular server
 *    name, it may place a null-terminated string in 'sname'.  The name
 *    used should be any of the allowable names or nicknames of the
 *    desired host.
 *    We use the default null (in bzero above).
 */
/*
 * RFC 951:
 *    The client has several options for filling the 'file' name field.
 *    If left null, the meaning is 'I want to boot the default file for
 *    my machine'.  A null file name can also mean 'I am only interested
 *    in finding out client/server/gateway IP addresses, I dont care
 *    about file names'.
 *    The field can also be a 'generic' name such as 'unix' or
 *    'gateway'; this means 'boot the named program configured for my
 *    machine'.  Finally the field can be a fully directory qualified
 *    path name.
 *    Zero in bzero above.
 */
/*
 * RFC 2132: Optional vendor field (bootVend, or bootOptions):
 * When used with BOOTP/DHCP, the first four octets of the vendor information
 * field have been assigned to the "magic cookie" (as suggested in RFC
 * 951). This field identifies the mode in which the succeeding data is
 * to be interpreted.  The value of the magic cookie is the 4 octet
 * dotted decimal 99.130.83.99 (or hexadecimal number 63.82.53.63) in
 * network byte order.
 */
        bootHdrPtr->bootOptions.optMagic = TM_BOOT_MAGIC;
        if (retryTimerFuncPtr != (ttTmCBFuncPtr)0)
        {
/*
 * allocate a timer to retry (up to TM_BOOTRETRIES):
 *   RFC951:
 *    If no reply is received for a certain length of time, the client
 *     should retransmit the request.  The time interval must be chosen
 *     carefully so as not to flood the network.  Consider the case of a
 *     cable containing 100 machines that are just coming up after a
 *     power failure.  Simply retransmitting the request every four
 *     seconds will inundate the net.
 *
 *     As a possible strategy, you might consider backing off
 *     exponentially, similar to the way ethernet backs off on a
 *     collision.  So for example if the first packet is at time 0:00,
 *     the second would be at :04, then :08, then :16, then :32, then
 *     :64.  You should also randomize each time; this would be done
 *     similar to the ethernet specification by starting with a mask and
 *     'and'ing that with with a random number to get the first backoff.
 *     On each succeeding backoff, the mask is increased in length by one
 *     bit.  This doubles the average delay on each backoff.
 *
 *     After the 'average' backoff reaches about 60 seconds, it should be
 *     increased no further, but still randomized.
 */
            if (    (retries < devPtr->devBtMaxRetries)
                 || (devPtr->devBtMaxRetries == TM_DEV_BOOT_INF) )
            {
/* Delay for next retransmission */
                delay = 
                    (tt16Bit)(devPtr->devBtInitTimeoutIntv << retries);
/* Limit maximum retransmission delay to 64 seconds, check for overflow */
                if ((delay > TM_BOOT_MAX_DELAY) || (delay == TM_16BIT_ZERO))
                {
                    delay = TM_BOOT_MAX_DELAY;
                }
                btEntryPtr->btRetries++;
/* time to wait in milliseconds */
                timerInterval = (tt32Bit)delay*TM_UL(1000);
/* add random value between 0 and 1023 milliseconds */
                tm_kernel_set_critical;
                timerInterval = timerInterval + (tvTime & 0x3FF);
                tm_kernel_release_critical;
                timerParm1.genVoidParmPtr = (ttVoidPtr)btEntryPtr;
/* Adding a pre-allocated timer. Cannot fail */
                btEntryPtr->btRetryTimerPtr =
                                tfTimerAddExt(&btEntryPtr->btRetryTimer,
                                              retryTimerFuncPtr,
                                              (ttTmCUFuncPtr)0,
                                              timerParm1,
                                              /* unused */
                                              timerParm1, 
                                              timerInterval,
                                              0);
            } /* if retries */
        } /* if retryTimerFuncPtr */
    } /* if packetPtr */
    return packetPtr;
}


/*
 * tfBtFinishConfig function description
 * 1. If errorCode is non zero
 *    . finish configuring the local route, using configuration
 *      information collected from server reply. Store errorCode from
 *      local route configuration.
 *    . Add default gateway if one was provided.
 * 2. Notify user of tfConfigInterface completion, passing errorCode.
 *
 * Parameters       meaning
 * devPtr      pointer to device
 * errorCode        TM_ENOERROR, or TM_ETIMEDOUT
 *
 * Return value     meaning
 * TM_ENOERROR      no error
 * TM_ETIMEDOUT     configuration timed out
 * other            from tfDeviceStart()
 */
int tfBtFinishConfig ( ttBtEntryPtr btEntryPtr, int errorCode )
{
    ttDeviceEntryPtr        devPtr;
    ttDevNotifyFuncPtr      notifyFuncPtr;
    ttUserDhcpNotifyFuncPtr dhcpNotifyFuncPtr;
    tt4IpAddress            gatewayIpAddr;
    int                     index;
#ifdef TM_USE_IPV6
    struct sockaddr_storage temp6IpAddr;
#endif /* TM_USE_IPV6 */

/*
 * finish configuring the local route
 */
    devPtr = btEntryPtr->btDevEntryPtr;
    index = (int)btEntryPtr->btIndex;
/* Used for a tfConfigInterface() result */
    notifyFuncPtr = (ttDevNotifyFuncPtr)0;
/* Used for a tfDhcpUserStart() result */
    dhcpNotifyFuncPtr = btEntryPtr->btDhcpNotifyFuncPtr;
/*
 * Close the BOOTP/DHCP socket. Device unlocked/relocked there.
 */
    (void)tfBtCloseSocket(btEntryPtr);
    if (btEntryPtr->btType == TM_BT_CONF)
    {
        if (errorCode == TM_ENOERROR)
        {
            if (tm_8bit_bits_not_set(btEntryPtr->btLeaseFlags, TM_BTF_LEASE))
            {
                tm_ip_copy( btEntryPtr->btYiaddr, tm_ip_dev_addr( devPtr,
                                                                  index ) );
                tm_ip_copy( btEntryPtr->btNetMask, tm_ip_dev_mask( devPtr,
                                                                   index ) );
                tm_4_ip_dev_prefixLen(devPtr, index) = 
                    (tt8Bit) tf4NetmaskToPrefixLen(btEntryPtr->btNetMask);
                errorCode = tfDeviceStart( devPtr,
                                           (tt16Bit) index,
                                           PF_INET );

#ifdef TM_USE_IPV6
/*
 * When operating in dual IPv4/IPv6 mode, IPv4 addresses acquired via
 * DHCP/BOOTP should also be configured as IPv4-compatible IPv6 addresses to
 * be used for automatic tunneling ([RFC2893].R5.2:10). Only do this if there
 * is not already an IPv6 address configured at this multihome index and if
 * this is a global IPv4 address. ([RFC2893].R5.1:10).  Device is locked at
 * this point.
 */
                if (     (errorCode == TM_ENOERROR)
                     && !(devPtr->dev6IpAddrFlagsArray[index] & 
                             (TM_6_DEV_IP_CONFIG | TM_6_DEV_IP_CONF_STARTED))
                     && (tm_4_ip_is_global(btEntryPtr->btYiaddr))
                     && (tm_16bit_bits_not_set(devPtr->devFlag2,
                                               TM_6_DEVF2_NO_DHCP_CONF) ) )  
                {
                    tm_6_addr_to_ipv4_compat(btEntryPtr->btYiaddr, 
                                             &temp6IpAddr.addr.ipv6.sin6_addr); 
                    temp6IpAddr.ss_family = AF_INET6;
                    tm_call_unlock(&devPtr->devLockEntry);

/* Don't need to get error here, no way to report it to the user */
                    (void) tfNgConfigInterface(
                        devPtr,
                        &temp6IpAddr,
                        TM_6_IP_HOST_PREFIX_LENGTH, /* prefix len */
                        devPtr->devFlag & TM_DEV_USER_ALLOWED,
                        devPtr->dev6Flags,
                        1, /* doesn't matter here, device already open */
                        devPtr->dev6AddrNotifyFuncPtr,
                        (unsigned int)index);
                    tm_call_lock_wait(&devPtr->devLockEntry);
                }
#endif /* TM_USE_IPV6 */
            }
            
/*
 * default route use first ROUTER tag field for default gateway.
 * Don't care about error. Nobody to report it to.
 */
            if (errorCode == TM_ENOERROR)
            {
                tm_ip_copy(TM_IP_ZERO, gatewayIpAddr);
                if (tm_8bit_bits_not_set(btEntryPtr->btLeaseFlags,
                                         TM_BTF_LEASE))
                {
/* Fresh configuration */
                    tm_ip_copy(btEntryPtr->btDefRouter, gatewayIpAddr);
                }
                else
                {
/* Lease Renewal */
                    if (!tm_ip_match(btEntryPtr->btLeaseDefRouter,
                                     btEntryPtr->btDefRouter))
                    {
/* Default gateway has changed in new lease */
                        tm_ip_copy(btEntryPtr->btLeaseDefRouter, gatewayIpAddr);
/* 
 * Remove the old default gateway from the routing table
 */
                        (void)tfDelDefaultGateway(
#ifdef TM_USE_STRONG_ESL
                                                  btEntryPtr->btDevEntryPtr,
#endif /* TM_USE_STRONG_ESL */
                                                  btEntryPtr->btDefRouter);
                    }
                }
                if (tm_ip_not_zero(gatewayIpAddr))
                {
/* Add the default gateway into the routing table */
#ifdef TM_USE_IPV6
/* 
 * In dual IP layer mode, we must always have an IPv4 default gateway entry
 * in the routing tree, even if it isn't configured. Replace the existing
 * default gateway, if it is not configured.
 */
                    (void)tf4RtAddDefGw(devPtr, gatewayIpAddr
#ifdef TM_USE_STRONG_ESL
                                        , 1
#endif /* TM_USE_STRONG_ESL */
                                       );
#else /* ! dual IP layer */
                    (void)tf4RtAddRoute(
                        devPtr,
                        TM_IP_ZERO,
                        TM_IP_ZERO,
                        gatewayIpAddr,
                        TM_16BIT_ZERO,
                        TM_16BIT_ZERO,
                        1,
                        TM_RTE_INF,
                        TM_RTE_STATIC|TM_RTE_INDIRECT,
                        (tt8Bit)1);
#endif /* ! dual IP layer */
                }
            }
        }
        notifyFuncPtr = devPtr->devBootInfo[btEntryPtr->btBootIndex].
                                                            bootNotifyFuncPtr;
/*
 * If an error code was passed in occurred in tfDeviceStart, reset
 * the device flags.
 */
        if (    (errorCode != TM_ENOERROR)
             && (btEntryPtr->btDhcpState == TM_DHCPS_INIT)
             && (tm_8bit_bits_not_set(btEntryPtr->btLeaseFlags,
                                      TM_BTF_LEASE)) )
        {
/* Reset TM_DEV_IP_DHCP/TM_DEV_IP_CONFIG */
            tm_ip_dev_conf_flag(devPtr, index) = 0;
        }
        if (notifyFuncPtr == TM_DEV_NOTIFY_FUNC_NULL_PTR)
        {
            devPtr->devErrorCode = errorCode;
        }
        else
        {
/*
 * REVNOTE: Will add index to let the user know about which multi home
 * index
 */
            tm_call_unlock(&(devPtr->devLockEntry));
            (*(notifyFuncPtr))((ttUserInterface)devPtr, errorCode);
            tm_call_lock_wait(&(devPtr->devLockEntry));
        }
    }
    else
    {
        if (dhcpNotifyFuncPtr != TM_DHCP_NOTIFY_FUNC_NULL_PTR)
        {
            tm_call_unlock(&(devPtr->devLockEntry));
            (*(dhcpNotifyFuncPtr))( (ttUserInterface)devPtr,
                                    errorCode,
                                    index);
            tm_call_lock_wait(&(devPtr->devLockEntry));
        }
    }
    return errorCode;
}

/*
 * tfBtSendPacket function description:
 * called by BOOTP or DHCP when a packet need to be sent out.
 * 1. Initialize sendto destination structure
 * 2. Unlock the device
 * 3. Call tfZeroCopySendTo()
 * 4. Relock the device
 * Parameters       meaning
 * devPtr           pointer to device
 * packetPtr        pointer to packet to send out
 * destIpAddress    destination IP address of packet
 */
int tfBtSendPacket ( ttPacketPtr      packetPtr,
                     tt4IpAddress     destIpAddress)
{
#if (defined(TM_LOCK_NEEDED) || defined(TM_USE_STRONG_ESL))
    ttDeviceEntryPtr     devPtr;
#endif /* TM_LOCK_NEEDED || TM_USE_STRONG_ESL */
    int                  errorCode;
auto struct  sockaddr_in toAddress;
    int                  toAddressLength;
    ttSockAddrPtrUnion   tempSockAddr;

    tempSockAddr.sockInPtr = &toAddress;
    toAddressLength = sizeof(struct sockaddr);
    toAddress.sin_family = PF_INET;
/* Destination port is:    TM_BOOTPS_PORT (BOOTP UDP server port) */
    toAddress.sin_port = TM_BOOTPS_PORT; /* network byte order */
    tm_ip_copy(destIpAddress, toAddress.sin_addr.s_addr);

#if (defined(TM_LOCK_NEEDED) || defined(TM_USE_STRONG_ESL))
    devPtr = packetPtr->pktDeviceEntryPtr;
#endif /* TM_LOCK_NEEDED || TM_USE_STRONG_ESL */
    tm_call_unlock(&(devPtr->devLockEntry));
    errorCode = tfZeroCopySendTo(
#ifdef TM_USE_STRONG_ESL
                                 devPtr->devBootSocketDescriptor,
#else /* TM_USE_STRONG_ESL */
                                 tm_context(tvBootSocketDescriptor),
#endif /* TM_USE_STRONG_ESL */
        
                                 (ttUserMessage)packetPtr,
                                 (int)(packetPtr->pktChainDataLength),
                                 MSG_DONTWAIT,
                                 tempSockAddr.sockPtr,
                                 toAddressLength);
    if (errorCode != TM_SOCKET_ERROR)
    {
        errorCode = TM_ENOERROR;
    }
    else
    {
#ifdef TM_USE_STRONG_ESL
        errorCode = tfGetSocketError(devPtr->devBootSocketDescriptor);
#else /* TM_USE_STRONG_ESL */
        errorCode = tfGetSocketError(tm_context(tvBootSocketDescriptor));
#endif /* TM_USE_STRONG_ESL */
    }
    tm_call_lock_wait(&(devPtr->devLockEntry));
    return errorCode;
}

/*
 * tfBtRemoveRetryTimer function description:
 * Remove retry timer for either BOOTP or DHCP, reset the retries count
 *
 * Parameters   meaning
 * btEntryPtr   pointer to device boot structure
 *
 * No return value
 */
void tfBtRemoveRetryTimer (ttBtEntryPtr btEntryPtr)
{
    if (btEntryPtr->btRetryTimerPtr != TM_TMR_NULL_PTR)
    {
        tm_timer_remove(btEntryPtr->btRetryTimerPtr);
        btEntryPtr->btRetryTimerPtr = TM_TMR_NULL_PTR;
    }
    btEntryPtr->btRetries = TM_8BIT_ZERO;
}

/*
 * Map from device entry, index, type to btEntry pointer
 */
ttBtEntryPtr tfBtMap ( ttDeviceEntryPtr devPtr,
                       int              index,
                       tt8Bit           type,
                       tt8Bit           bootIndex)
{
    ttBtEntryPtrPtr btEntryPtrPtr;
    ttBtEntryPtr    btEntryPtr;

    btEntryPtrPtr = devPtr->devBtEntryTablePtr[type][bootIndex];
    if (btEntryPtrPtr != (ttBtEntryPtrPtr)0)
    {
        btEntryPtr = btEntryPtrPtr[index];
    }
    else
    {
        btEntryPtr = (ttBtEntryPtr)0;
    }
    return btEntryPtr;
} 

static ttBtEntryPtr tfBtMatchByXid ( ttBtEntryPtrPtr btEntryPtrPtr,
                                     int             max,
                                     tt32Bit         xid )
{
    ttBtEntryPtr    btEntryPtr;
    ttBtEntryPtr    resultBtEntryPtr;
    int             i;

    resultBtEntryPtr = (ttBtEntryPtr)0;
    for (i = 0; i < max; i++)
    {
        btEntryPtr = btEntryPtrPtr[i];
        if (btEntryPtr != (ttBtEntryPtr)0)
        {
            if (btEntryPtr->btXid == xid)
            {
                resultBtEntryPtr = btEntryPtr;
                break;
            }
        }
    }
    return resultBtEntryPtr;
}

/*
 * tfBtFindCustOpt() Function Description
 * Called by tfListWalk to find an option in the unknown options list
 *
 * Parameters   Meaning
 * nodePtr      Pointer to the current option
 * genParam1    Option to find
 *
 * Return
 * TM_8BIT_NO   This isn't the right option
 * TM_8BIT_YES  We found it
 */
int tfBtFindCustOpt(ttNodePtr      nodePtr,
                    ttGenericUnion genParam1)
{
    ttBtUserCustOptPtr optPtr;
    int                retVal;

    optPtr = (ttBtUserCustOptPtr)nodePtr;
    if (optPtr->buoBootOption == genParam1.gen8BitParm)
    {
        retVal = TM_8BIT_YES;
    }
    else
    {
        retVal = TM_8BIT_NO;
    }
    return retVal;
}

/*
 * tfBtSetOption() Function Description
 * Set custom DHCP or BOOTP configuration options prior to the start of
 * BOOTP/DHCP operation. This function is to set options that are not
 * natively supported by the BOOTP or DHCP client.
 *
 * 1. Verfy parameters
 * 2. Get a boot entry for this interface & index
 * 3. If this option has already been set, free it
 * 4. Add this option to the custom options list
 *
 * Parameter         Description
 * interfaceHandle    Ethernet interface handle
 * index              Index to set this option on
 * bootOption         Option to set
 * dataPtr            Data to be sent in option
 * dataLen            Length of data pointed to by dataPtr
 * indexType          type of 'index' parameter - user index or multi-home
 *                    index
 * bootIndex          DHCP or BOOTP
 *
 * Returns
 * TM_ENOERROR      Success
 * TM_EINVAL        Invalid parameters passed to function
 * TM_ENOBUFS       Didn't have memory available to complete operation
 */
int tfBtSetOption(ttUserInterface interfaceHandle,
                  int             index,
                  tt8Bit          bootOption,
                  ttUserCharPtr   dataPtr,
                  tt8Bit          dataLen,
                  tt8Bit          indexType,
                  tt8Bit          bootIndex)
{
    ttDeviceEntryPtr   devPtr;
    ttBtEntryPtr       btEntryPtr;
    ttBtUserCustOptPtr custOptPtr;
    ttGenericUnion     opt;
    int                errorCode;

    errorCode  = TM_ENOERROR;
    custOptPtr = (ttBtUserCustOptPtr)0;

    if (dataLen > 0 && !dataPtr)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        devPtr = (ttDeviceEntryPtr)interfaceHandle;
/* Check validity of the device */
        errorCode = tfValidInterface(devPtr);
        if ( errorCode == TM_ENOERROR )
        {
            btEntryPtr = (ttBtEntryPtr)0;
/* Lock the device */
            tm_call_lock_wait(&(devPtr->devLockEntry));
/* Check that index is within range */
            if (indexType == TM_BT_USER)
            {
                if (     (index < 0)
                     || (index >= tm_context(tvMaxUserDhcpEntries)) )
                {
                    errorCode = TM_EINVAL;
                }
            }
            if (errorCode == TM_ENOERROR)
            {
/* Get/Allocate btEntry corresponding to index, indexType, on the device */
                btEntryPtr = tfBtAlloc (devPtr, index, indexType,
                                        bootIndex);
                if (btEntryPtr == (ttBtEntryPtr)0)
                {
                    errorCode = TM_ENOBUFS;
                }
            }
            if (errorCode == TM_ENOERROR)
            {
/* If this option has already been set, free it */
                opt.gen8BitParm = bootOption;
                custOptPtr = (ttBtUserCustOptPtr)tfListWalk(
                                &(btEntryPtr->btUserSetCustOpts),
                                tfBtFindCustOpt,
                                opt);
                if (custOptPtr)
                {
                    tfListRemove(&(btEntryPtr->btUserSetCustOpts),
                                                (ttNodePtr)custOptPtr);
                    tm_free_raw_buffer(custOptPtr);
                }
                if (dataLen)
                {
/* Save the new option to the list */
                    custOptPtr = tm_get_raw_buffer(sizeof(ttBtUserCustOpt) +
                                                dataLen);
                    if (custOptPtr == (ttBtUserCustOptPtr)0)
                    {
                        errorCode = TM_ENOBUFS;
                    }
                    else
                    {
                        custOptPtr->buoDataPtr = (ttCharPtr)(custOptPtr + 1);
                        custOptPtr->buoDataLen = dataLen;
                        custOptPtr->buoBootOption = bootOption;
                        tm_bcopy(dataPtr,
                                custOptPtr->buoDataPtr,
                                dataLen);
                        tfListAddToTail(&(btEntryPtr->btUserSetCustOpts),
                                        (ttNodePtr)custOptPtr);
                    }
                }
            }
            tm_call_unlock(&(devPtr->devLockEntry));
        }
    }

    return errorCode;
}


/*
 * tfBtGetOption() Function Description
 * When the DHCP or BOOTP client accepts a lease, it saves any unrecognized
 * options on a special list. This function retrieves the options from that
 * list.
 *
 * 1. Verfy parameters
 * 2. Get a boot entry for this interface & index
 * 3. Find the option
 * 4. Copy the option to the user buffer
 *
 * Parameter         Description
 * interfaceHandle    Ethernet interface handle
 * index              Index to retrieve this option from
 * bootOption         Option to retrieve
 * dataPtr            Buffer to store option data
 * dataLenPtr         Pointer to the length of the buffer pointed to by
 *                    dataPtr. This function resets it to the length of the
 *                    data actually stored in the bufer.
 * indexType          type of 'index' parameter - user index or multi-home
 *                    index
 * bootIndex          DHCP or BOOTP
 *
 * Returns
 * TM_ENOERROR      Success
 * TM_EINVAL        Invalid parameters passed to function
 * TM_ENOBUFS       Didn't have memory available to complete operation
 * TM_EMSGSIZE      Not eough room in buffer to store option data. If this
 *                  happens, the length needed is stored in dataLenPtr
 * TM_ENOENT        The option was not found
 * 
 */
int tfBtGetOption(ttUserInterface  interfaceHandle,
                  int              index,
                  tt8Bit           bootOption,
                  ttUserCharPtr    dataPtr,
                  tt8BitPtr        dataLenPtr,
                  tt8Bit           indexType,
                  tt8Bit           bootIndex)
{
    ttDeviceEntryPtr   devPtr;
    ttBtEntryPtr       btEntryPtr;
    ttBtUserCustOptPtr custOptPtr;
    ttGenericUnion     opt;
    int                errorCode;
#ifdef TM_DSP
    int                byteOffset;

    byteOffset = 0;
#endif /* TM_DSP */

    errorCode = TM_ENOERROR;

    if (dataPtr == 0 || *dataLenPtr == 0)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        devPtr = (ttDeviceEntryPtr)interfaceHandle;
/* Check validity of the device */
        errorCode = tfValidInterface(devPtr);
        if ( errorCode == TM_ENOERROR )
        {
            btEntryPtr = (ttBtEntryPtr)0;
/* Lock the device */
            tm_call_lock_wait(&(devPtr->devLockEntry));
/* Check that index is within range allocated */
            if (indexType == TM_BT_USER)
            {
                if (     (index < 0)
                     || (index >= (devPtr->devBtUserMaxEntries)) )
                {
                    errorCode = TM_EINVAL;
                }
            }
            if (errorCode == TM_ENOERROR)
            {
/* Get/Allocate btEntry corresponding to index, indexType on the device */
                btEntryPtr = tfBtAlloc (devPtr, index, indexType,
                                        bootIndex);
                if (btEntryPtr == (ttBtEntryPtr)0)
                {
                    errorCode = TM_ENOBUFS;
                }
            }
            if (errorCode == TM_ENOERROR)
            {
/* Find the requested option */
                opt.gen8BitParm = bootOption;
                custOptPtr = (ttBtUserCustOptPtr)tfListWalk(
                                &(btEntryPtr->btOfferEntryPtr
                                            ->btUserServCustOpts),
                                tfBtFindCustOpt,
                                opt);
                if (custOptPtr)
                {
/* If there's enough room to store the buffer, do so */
                    if (*dataLenPtr >= custOptPtr->buoDataLen)
                    {
                        tm_btdhcp_byte_copy(custOptPtr->buoDataPtr,
                                            *dataPtr,
                                            0,
                                            custOptPtr->buoDataLen,
                                            byteOffset);
                    }
                    else
                    {
                        errorCode = TM_EMSGSIZE;
                    }
                    *dataLenPtr = custOptPtr->buoDataLen;
                }
                else
                {
/* Else the option wasn't found */
                    errorCode = TM_ENOENT;
                }
            }
            tm_call_unlock(&(devPtr->devLockEntry));
        }
    }
    return errorCode;
}

void tfBtInitUserEntryNames (ttBtEntryPtr btEntryPtr)
{
    int              i;

    for (i = 0; i < TM_BTU_NAMES_MAX_INDEX; i++)
    {
/* Point to null */
        tm_btdhcp_name_init(btEntryPtr, i);
    }
}

/* Initialize user BOOT entry */
void tfBtInitUserEntry (ttBtEntryPtr btEntryPtr)
{

/* Free allocated space used to store server values before reinitialization */
    tfBtDiscardServerNames(btEntryPtr);
    tm_bzero(&btEntryPtr->btOfferEntryPtr->btUserEntry,
                                                sizeof(ttUserBtEntry));
    tfBtInitUserEntryNames(btEntryPtr);
}

/* Free server acquired values */
void tfBtDiscardServerNames (ttBtEntryPtr btEntryPtr)
{
    int i;

/* Free the server options we received */
    tfListFree(&(btEntryPtr->btOfferEntryPtr->btUserServCustOpts));
    for (i = 0; i < TM_BTU_NAMES_MAX_INDEX; i++)
    {
        if (tm_btdhcp_name_is_allocated(btEntryPtr, i))
        {
/* Free */
            tm_btdhcp_free_name(btEntryPtr, i);
/* Point to null */
            tm_btdhcp_name_init(btEntryPtr, i);
        }
    }
#ifdef TM_USE_DHCP_FQDN
    tm_bzero(btEntryPtr->btOfferEntryPtr->btUserEntry.btuFqdnArray,
             sizeof(btEntryPtr->btOfferEntryPtr->btUserEntry.btuFqdnArray));
    btEntryPtr->btOfferEntryPtr->btUserEntry.btuServerFqdnLen = 0;
#endif /* TM_USE_DHCP_FQDN */
}

/* Copy from BOOT header to user entry fields */
int tfBtBootCopy(ttBtEntryPtr       btEntryPtr,
                 tt8BitPtr          srcPtr,
                 int                srcOffset,
                 int                optionLength, /* unpacked */
                 int                maxSize, /* packed */
                 int                index
                 )
{
    tt8BitPtr         namePtr;
    unsigned int      prevNameLength;
    unsigned int      allocSize;
    int               errorCode;

    errorCode = TM_ENOERROR;
    if (optionLength != 0)
    {
/* maxSize includes the srcOffset */
        if ((optionLength + srcOffset) <= tm_byte_count(maxSize))
        {
            allocSize = (unsigned)(optionLength + 1); /* includes final '\0' */
            prevNameLength = 0;
            if (tm_btdhcp_name_is_allocated(btEntryPtr, index) )
            {
                prevNameLength = tm_btdhcp_name_length(btEntryPtr, index);
                allocSize =  allocSize + prevNameLength;
            }
            namePtr = tm_get_raw_buffer(allocSize);
            if (namePtr != (tt8BitPtr)0)
            {
                if (prevNameLength != 0)
                {
                    tm_bcopy( tm_btdhcp_name(btEntryPtr, index),
                              namePtr,
                              tm_packed_byte_count(prevNameLength));
                    tm_free_raw_buffer(tm_btdhcp_name(btEntryPtr, index));
                }
#ifdef TM_DSP
                tfMemCopyOffset( (int*) (srcPtr), srcOffset,
                                 (int*) (namePtr), prevNameLength,
                                 optionLength);
#else /* !TM_DSP */
                tm_bcopy( &srcPtr[srcOffset],
                          &namePtr[prevNameLength],
                          optionLength);
#endif /* !TM_DSP */
                if (prevNameLength != 0)
                {
                    optionLength = (int)(allocSize - 1);
                }
                tm_btdhcp_set_char('\0', 
                                   namePtr, 
                                   optionLength, 0) ;
                tm_btdhcp_name(btEntryPtr, index) = namePtr;
                tm_btdhcp_name_length(btEntryPtr, index) = (ttUser16Bit)
                                                                optionLength;
            }
            else
            {
                errorCode = TM_ENOBUFS;
            }
        }
        else
        {
            errorCode = TM_EINVAL;
        }
    }
    return errorCode;
}

#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */


/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4BtdhcpDummy = 0;
#endif /* ! TM_USE_IPV4 */
