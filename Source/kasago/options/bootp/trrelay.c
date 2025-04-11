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
 * Description: BOOTP relay agent
 *
 * Filename: trrelay.c
 * Author: Odile
 * Date Created: 06/18/98
 * $Source: source/trrelay.c $
 *
 * Modification History
 * $Revision: 6.0.2.3 $
 * $Date: 2015/10/15 21:48:19JST $
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
 * Local macros
 */

 
#define TM_BOOT_BROADCAST_BIT         tm_const_htons(0x8000)


/* Maximum number of hops allowed for BOOTP relay agent */
#define TM_BOOT_MAX_HOPS              16


/*
 * Local types
 */

/*
 *  BOOTP/DHCP header (from RFC 1542 modified for DHCP/BOOTP)
 *
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     op (1)    |   htype (1)   |   hlen (1)    |   hops (1)    |
 * +---------------+---------------+---------------+---------------+
 * |                            xid (4)                            |
 * +-------------------------------+-------------------------------+
 * |           secs (2)            |           flags (2)           |
 * +-------------------------------+-------------------------------+
 * |                           ciaddr (4)                          |
 * +---------------------------------------------------------------+
 * |                           yiaddr (4)                          |
 * +---------------------------------------------------------------+
 * |                           siaddr (4)                          |
 * +---------------------------------------------------------------+
 * |                           giaddr (4)                          |
 * +---------------------------------------------------------------+
 * |                                                               |
 * |                           chaddr (16)                         |
 * |                                                               |
 * |                                                               |
 * +---------------------------------------------------------------+
 * |                                                               |
 * |                           sname  (64)                         |
 * +---------------------------------------------------------------+
 * |                                                               |
 * |                           file   (128)                        |
 * +---------------------------------------------------------------+
 * |                                                               |
 * |                           options(312)                        |
 * +---------------------------------------------------------------+
 */

/*
 * Local functions
 */

static void tfRelayRecvCB(int   socketDescriptor,
                          int   socFlags);


/*
 * tfStartBootRelayAgent description:
 * Start BOOTP relay agent. BOOTP/DHCP can be reached via interfaceHandle
 * at address ipAddress.
 * 1. We check that the interfaceHandle, and ipAddress are valid.
 * 2. Open the BOOTP relay agent socket descriptor and bind it to BOOTPS
 *    UDP port, if not already open. Save in global
 *    tvBootRelayAgentDescriptor
 * 3. register a receive call back function tfRelayRecvCB() which will be
 *    called every time a BOOTP or DHCP packet is received
 * 4. Save the parameters in tvBootServDevEntryPtr, and tvBootServIpAddress
 *    respectively for later use.
 * Parameters       meaning
 * interfaceHandle  Pointer to device entry pointer through which to reach
 *                  the device.
 * ipAddress        Ip address of the BootServ.
 *
 * Return value     meaning
 * TM_ENOERROR      success
 * TM_EALREADY      BOOTP relay agent has already been started
 * TM_EINVAL        Invalid interfaceHandle, or ipAddress
 * TM_EADDRINUSE    A UDP socket is already bound to the BOOTP BootServ port
 * TM_EMFILE        No more sockets
 */
int tfStartBootRelayAgent( ttUserIpAddress ipAddress,
                           ttUserInterface interfaceHandle,
                           unsigned char   multiHomeIndex)
{
    ttDeviceEntryPtr    devPtr;
    ttSockAddrPtrUnion  tempSockAddr;
    struct sockaddr_in  addr;
    int                 errorCode;
    int                 socketDescriptor;

    devPtr = (ttDeviceEntryPtr)interfaceHandle;
    errorCode = tfValidInterface(devPtr);
    if ( ( errorCode != TM_ENOERROR) ||
#ifdef TM_SINGLE_INTERFACE_HOME
         ((tt8Bit)multiHomeIndex >= (tt8Bit)1) ||
#else /* TM_SINGLE_INTERFACE_HOME */
         ((tt16Bit)multiHomeIndex >= (devPtr)->devMhomeUsedEntries) ||
#endif /* TM_SINGLE_INTERFACE_HOME */
         ((tm_ip_dev_conf_flag((devPtr), multiHomeIndex)
                                          & TM_DEV_IP_CONFIG) == 0) ||
         tm_ip_zero(ipAddress) )
    {
        errorCode = TM_EINVAL;
        goto bootRelayExit;
    }
    if
#ifdef TM_USE_STRONG_ESL
        (devPtr->devBootRelayAgentDescriptor == TM_SOCKET_ERROR)
#else /* TM_USE_STRON_ESL */
        (tm_context(tvBootRelayAgentDescriptor) == TM_SOCKET_ERROR)
#endif /* TM_USE_STRONG_ESL */
    {
        errorCode = TM_ENOERROR;
        socketDescriptor = socket(PF_INET, SOCK_DGRAM, IP_PROTOUDP);
        if (socketDescriptor != TM_SOCKET_ERROR)
        {
#ifdef TM_USE_STRONG_ESL
            devPtr->devBootRelayAgentDescriptor = socketDescriptor;
/* bind to the device */
            tm_call_unlock(&devPtr->devLockEntry);
            errorCode = setsockopt(devPtr->devBootRelayAgentDescriptor,
                                   SOL_SOCKET,
                                   SO_BINDTODEVICE,
                                   (char *)(devPtr->devNameArray),
                                   IFNAMSIZ);
            tm_call_lock_wait(&devPtr->devLockEntry);
            if (   (errorCode != TM_SOCKET_ERROR)
                && (socketDescriptor == devPtr->devBootRelayAgentDescriptor) )
#endif /* TM_USE_STRONG_ESL */
            {
/* bind to the port */
                addr.sin_family = PF_INET;
                addr.sin_port = TM_BOOTPS_PORT; /* network byte order */
                tm_ip_copy(TM_IP_ZERO, addr.sin_addr.s_addr);
                tempSockAddr.sockInPtr = &addr;
                errorCode = bind(socketDescriptor ,
                                 tempSockAddr.sockPtr,
                                 sizeof(struct sockaddr_in));
                if (errorCode != TM_SOCKET_ERROR)
                {
                    errorCode = tfRegisterSocketCB(socketDescriptor,
                                                   tfRelayRecvCB,
                                                   TM_CB_RECV);
                }
            }
        }
        if (errorCode == TM_SOCKET_ERROR)
        {
            errorCode = tfGetSocketError(socketDescriptor);
            if (socketDescriptor != TM_SOCKET_ERROR)
            {
#ifdef TM_USE_STRONG_ESL
                devPtr->devBootRelayAgentDescriptor = TM_SOCKET_ERROR;
#endif /* TM_USE_STRONG_ESL */
                (void)tfClose(socketDescriptor);
            }
            goto bootRelayExit;
        }
#ifndef TM_USE_STRONG_ESL
        tm_context(tvBootRelayAgentDescriptor) = socketDescriptor;
#endif /* TM_USE_STRONG_ESL */
        tm_context(tvBootServDevEntryPtr) = (ttDeviceEntryPtr)interfaceHandle;
        tm_context(tvBootServMhomeIndex) = (tt16Bit) multiHomeIndex;
        tm_ip_copy(ipAddress, tm_context(tvBootServIpAddress));
    }
    else
    {
        errorCode = TM_EALREADY;
    }
bootRelayExit:
    return errorCode;
}

/*
 * Stop BOOTP relay agent.
 * . We close the BOOTP relay agent UDP socket.
 * No parameter
 *
 * Return value meaning
 * TM_EALREADY  BOOTP relay agent alredy stopped
 */
#ifdef TM_USE_STRONG_ESL
int tfStopBootRelayAgent (ttUserInterface interfaceHandle)
#else /* TM_USE_STRONG_ESL */
int tfStopBootRelayAgent (void)
#endif /* TM_USE_STRONG_ESL */
{
#ifdef TM_USE_STRONG_ESL
    ttDeviceEntryPtr devPtr;
#endif /* TM_USE_STRONG_ESL */
    int              errorCode;

#ifdef TM_USE_STRONG_ESL
    devPtr = (ttDeviceEntryPtr)interfaceHandle;
    if (devPtr->devBootRelayAgentDescriptor != TM_SOCKET_ERROR)
    {
        errorCode = tfClose(devPtr->devBootRelayAgentDescriptor);
        devPtr->devBootRelayAgentDescriptor = TM_SOCKET_ERROR;
    }
#else /* TM_USE_STRONG_ESL */
    if (tm_context(tvBootRelayAgentDescriptor) != TM_SOCKET_ERROR)
    {
        errorCode = tfClose(tm_context(tvBootRelayAgentDescriptor));
        tm_context(tvBootRelayAgentDescriptor) = TM_SOCKET_ERROR;
    }
#endif /* TM_USE_STRONG_ESL */
    else
    {
        errorCode = TM_EALREADY;
    }
    return errorCode;
}


/*
 * tfRelayRecvCB() Function Description
 * Process an incoming packet for the BOOTP relay agent.
 *
 * Parameters:
 * socketDescriptor socketDescritor of the BOOT socket opened in
 *                  tfStartBootRelayAgent()
 * socFlags         event flag (TM_CB_RECV always, since it is the only
 *                  flag we have registered for)
 *
 * No return value
 */
static void TM_CODE_FAR tfRelayRecvCB(int  socketDescriptor,
                                      int  socketCBFlags)
{
     ttDeviceEntryPtr   deviceEntryPtr;
     int                bufferLen;
     ttPacketPtr        packetPtr;
/* point to BOOT header */
     ttCharPtr          bufferPtr;
     struct sockaddr_in address;
     int                addressLength;
#ifndef TM_SINGLE_INTERFACE_HOME
auto tt16Bit            mhomeIndex;
#endif /* TM_SINGLE_INTERFACE_HOME */
     tt8Bit             needFreePacket;
     ttBootHeaderPtr    bootHdrPtr;
#ifdef TM_ERROR_CHECKING
     int                errorCode;
#endif /* TM_ERROR_CHECKING */
     ttSockAddrPtrUnion tempSockAddr;

    TM_UNREF_IN_ARG(socketCBFlags);
    addressLength = sizeof(struct sockaddr);
    needFreePacket = TM_8BIT_YES;
    tempSockAddr.sockInPtr = &address;
    bufferLen = tfZeroCopyRecvFrom(
            socketDescriptor,
            (ttUserMessage TM_FAR *)(ttVoidPtr)&packetPtr,
            (char TM_FAR * TM_FAR *)&bufferPtr,
            (int)TM_MAX_BOOT_PACKET,
            0,
            tempSockAddr.sockPtr,
            &addressLength);
#ifdef TM_ERROR_CHECKING
    if (bufferLen == TM_SOCKET_ERROR)
    {
        tfKernelError("tfRelayRecvCB", "Could not recv packet");
        goto relayRecvExit;
    }
#endif /* TM_ERROR_CHECKING */
    bootHdrPtr = (ttBootHeaderPtr)bufferPtr;
/* packet for BOOTP relay agent */
    if (bootHdrPtr->bootOp == TM_BOOTREQUEST)
    {
/* BOOTP relay agent to forward a request */
/*
 * RFC 1542:
 *  The relay agent MUST silently discard BOOTREQUEST messages whose
 *  'hops' field exceeds the value 16.  A configuration option SHOULD be
 *  provided to set this threshold to a smaller value if desired by the
 *  network manager.  The default setting for a configurable threshold
 *  SHOULD be 4.
 */
        if (bootHdrPtr->bootHops >= TM_BOOT_MAX_HOPS)
        {
            goto relayRecvExit;
        }
        deviceEntryPtr = packetPtr->pktDeviceEntryPtr;
/*
 *  RFC 1542: it may be desirable to send to the
 *  limited broadcast address (255.255.255.255) on specific
 *  physical interfaces.  However, if the BOOTREQUEST message was
 *  received as a broadcast, the relay agent MUST NOT rebroadcast
 *  the BOOTREQUEST on the physical interface from whence it came.
 */
        if (    (deviceEntryPtr == tm_context(tvBootServDevEntryPtr))
             && (tm_16bit_one_bit_set( packetPtr->pktFlags,
                                         TM_LL_BROADCAST_FLAG
                                       | TM_IP_BROADCAST_FLAG ))
             && (    (tm_ip_match(tm_context(tvBootServIpAddress),
                                  TM_IP_LIMITED_BROADCAST))
                  || (tm_ip_match(tm_context(tvBootServIpAddress),
                                  tm_ip_dev_dbroad_addr(
                                        tm_context(tvBootServDevEntryPtr),
                                        tm_context(tvBootServMhomeIndex))))) )
        {
            goto relayRecvExit;
        }
#ifdef TM_ERROR_CHECKING
        if (deviceEntryPtr == (ttDeviceEntryPtr)0)
        {
            tfKernelError("tfRelayRecvCB",
                          "Null incoming device entry Ptr");
            goto relayRecvExit;
        }
#endif /* TM_ERROR_CHECKING */
/*
 * RFC 1542:
 *  If the relay agent does decide to relay the request, it MUST examine
 *  the 'giaddr' ("gateway" IP address) field.  If this field is zero,
 *  the relay agent MUST fill this field with the IP address of the
 *  interface on which the request was received.  If the interface has
 *  more than one IP address logically associated with it, the relay
 *  agent SHOULD choose one IP address associated with that interface and
 *  use it consistently for all BOOTP messages it relays.  If the
 *  'giaddr' field contains some non-zero value, the 'giaddr' field MUST
 *  NOT be modified.  The relay agent MUST NOT, under any circumstances,
 *  fill the 'giaddr' field with a broadcast address as is suggested in
 *  [1] (Section 8, sixth paragraph).
 */
        if (tm_ip_zero(bootHdrPtr->bootGiaddr))
        {
            tm_ip_copy( tm_ip_dev_addr(deviceEntryPtr, 0),
                        bootHdrPtr->bootGiaddr);
        }
/*
 *  The value of the 'hops' field MUST be incremented.
 *  All other BOOTP fields MUST be preserved intact.
 */
        bootHdrPtr->bootHops++;
/*
 * At this point, the request is relayed to its new destination (or
 * destinations).  This destination MUST be configurable.  Further, this
 * destination configuration SHOULD be independent of the destination
 * configuration for any other so-called "broadcast forwarders" (e.g.,
 * for the UDP-based TFTP, DNS, Time, etc.  protocols).
 */
        address.sin_port = TM_BOOTPS_PORT;
        tm_ip_copy(tm_context(tvBootServIpAddress), address.sin_addr.s_addr);
/* Fill output interface */
        packetPtr->pktDeviceEntryPtr = tm_context(tvBootServDevEntryPtr);
        packetPtr->pktMhomeIndex = tm_context(tvBootServMhomeIndex);
        packetPtr->pktFlags = TM_OUTPUT_DEVICE_FLAG;
        packetPtr->pktUserFlags = TM_PKTF_USER_OWNS;
    }
    else
    {
/* BOOTP relay agent to forward a reply */
/*
 * RFC 1542:
 * BOOTP relay agents relay BOOTREPLY messages only to BOOTP clients.
 * It is the responsibility of BOOTP BootServs to send BOOTREPLY messages
 * directly to the relay agent identified in the 'giaddr' field.
 * Therefore, a relay agent may assume that all BOOTREPLY messages it
 * receives are intended for BOOTP clients on its directly-connected
 * networks.
 * The 'giaddr' field can be used to identify the logical interface from
 * which the reply must be sent (i.e., the host or router interface
 * connected to the same network as the BOOTP client).  If the content
 * of the 'giaddr' field does not match one of the relay agent's
 * directly-connected logical interfaces, the BOOTREPLY messsage MUST be
 * silently discarded.
 */
/* Get Device/multi-home index of address match for Giaddr */
#ifdef TM_SINGLE_INTERFACE_HOME
        deviceEntryPtr = tfIfaceConfig(0);
        if ( deviceEntryPtr != (ttDeviceEntryPtr)0 )
        {
            if ( !tm_ip_match( bootHdrPtr->bootGiaddr,
                               tm_ip_dev_addr(deviceEntryPtr, 0)) )
            {
                deviceEntryPtr = (ttDeviceEntryPtr)0;
            }
        }
#else /* TM_SINGLE_INTERFACE_HOME */
        deviceEntryPtr = tfIfaceMatch (bootHdrPtr->bootGiaddr,
                                       tfMhomeAddrMatch,
                                       &mhomeIndex);
#endif /* TM_SINGLE_INTERFACE_HOME */
        if (deviceEntryPtr == (ttDeviceEntryPtr)0)
        {
            goto relayRecvExit;
        }
/*
 *  RFC 1542: The relay agent SHOULD examine the newly-defined BROADCAST
 *  flag (see Sections 2.2 and 3.1.1 for more information). If this flag
 *  is set to 1, the reply SHOULD be sent as an IP broadcast using the IP
 *  limited broadcast address 255.255.255.255 as the IP destination address
 *  and the link-layer broadcast address as the link-layer destination
 *  address.  If the BROADCAST flag is cleared (0), the reply SHOULD be
 *  sent as an IP unicast to the IP address specified by the 'yiaddr'
 *  field and the link-layer address specified in the 'chaddr' field.  If
 *  unicasting is not possible, the reply MAY be sent as a broadcast, in
 *  which case it SHOULD be sent to the link-layer broadcast address
 *  using the IP limited broadcast address 255.255.255.255 as the IP
 *  destination address.
 */
        if ( tm_16bit_one_bit_set(bootHdrPtr->bootFlags,
                                  TM_BOOT_BROADCAST_BIT) )
        {
            tm_ip_copy(TM_IP_LIMITED_BROADCAST, address.sin_addr.s_addr);

        }
        else
        {
/*
 * RFC 1542: The 'htype', 'hlen', and 'chaddr' fields supply the link-layer
 * hardware type, hardware address length, and hardware address of the
 * client as defined in the ARP protocol [4] and the Assigned Numbers
 * document [6].
 */
            if ( (bootHdrPtr->bootHtype != TM_ARP_ETHER) ||
                 (bootHdrPtr->bootHlen != TM_ETHER_ADDRESS_LENGTH) )
            {
/* We only forward ETHERNET requests */
                goto relayRecvExit;
            }
/*
 * Cannot use ARP, since the booting client does not know its IP address
 * yet. So we copy the client ethernet address and bypass IP routing/ARP
 * by setting the TM_ETHERNET_ADDRESS_FLAG in pktFlags (below).
 */
            tm_bcopy(&bootHdrPtr->bootChaddr[0],
                     &packetPtr->pktSharedDataPtr->dataEthernetAddress[0],
                     TM_ETHER_ADDRESS_LENGTH);
/*
 * RFC 1542: The 'yiaddr' field is the IP address of the client, as
 * assigned by the BOOTP BootServ.
 */
            tm_ip_copy(bootHdrPtr->bootYiaddr, address.sin_addr.s_addr);
            packetPtr->pktFlags =   TM_OUTPUT_DEVICE_FLAG
                                  | TM_ETHERNET_ADDRESS_FLAG;
            packetPtr->pktUserFlags = TM_PKTF_USER_OWNS;
        }
        address.sin_port = TM_BOOTPC_PORT;
/* Fill output interface */
        packetPtr->pktDeviceEntryPtr = deviceEntryPtr;
#ifndef TM_SINGLE_INTERFACE_HOME
        packetPtr->pktMhomeIndex = mhomeIndex;
#endif /* TM_SINGLE_INTERFACE_HOME */
    }
    needFreePacket = TM_8BIT_ZERO;
    tempSockAddr.sockInPtr = &address;

#ifdef TM_ERROR_CHECKING
    errorCode =
#endif /* TM_ERROR_CHECKING */

                tfZeroCopySendTo(
#ifdef TM_USE_STRONG_ESL
                                 deviceEntryPtr->devBootRelayAgentDescriptor,
#else /* TM_USE_STRONG_ESL */
                                 tm_context(tvBootRelayAgentDescriptor),
#endif /* TM_USE_STRONG_ESL */
                                 (ttUserMessage)packetPtr,
                                 bufferLen,
                                 MSG_DONTWAIT,
                                 tempSockAddr.sockPtr,
                                 addressLength);
#ifdef TM_ERROR_CHECKING
    if (errorCode == TM_SOCKET_ERROR)
    {
        tfKernelWarning("tfRelayRecvCB",
                        "Relay agent could not forward boot request");
    }
#endif /* TM_ERROR_CHECKING */
relayRecvExit:
    if (needFreePacket != TM_8BIT_ZERO)
    {
/* Recv CB function called with socket unlocked */
        tfFreePacket((ttPacketPtr)packetPtr, TM_SOCKET_UNLOCKED);
    }
}

#else /* ! TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4RelayDummy = 0;
#endif /* ! TM_USE_IPV4 */
