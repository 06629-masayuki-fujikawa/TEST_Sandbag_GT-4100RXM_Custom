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
 * Description: DHCP User interface
 *
 * Filename: trdhcpui.c
 * Author: Odile
 * Date Created: 01/21/99
 * $Source: source/sockapi/trdhcpui.c $
 *
 * Modification History
 * $Revision: 6.0.2.4 $
 * $Date: 2010/06/17 07:26:31JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include <trsocket.h>
#include <trmacro.h>

#if (defined(TM_USE_DHCP) && defined(TM_USE_IPV4))

#include <trtype.h>
#include <trproto.h>
#include <trglobal.h>

/*
 * Local functions
 */

static int tfDhcpCommonSet( ttUserInterface interfaceHandle,
                            int             index,
                            int             flags,
                            ttUserIpAddress requestedIpAddress,
                            unsigned char   TM_FAR * clientIdPtr,
                            int             clientIdLength,
                            tt8Bit          indexType );

/*
 * tfDhcpUserStart function description:
 * Start Sending a DHCP request on the interface corresponding to
 * interface Handle. The index corresponds to a user DHCP request.
 * It has to be between 0 and tvMaxUserDhcpEntries - 1. (tvMaxUserDhcpEntries
 * default value is 0, and can be changed with a tfInitSetTreckOptions, with
 * option name TM_OPTION_DHCP_MAX_ENTRIES).
 * dhcpNotifyFunc is a user supplied call back function which will be called
 * when either the DHCP request has been successful, or has timed out, or
 * when a previously leased DHCP address has been cancelled by the server.
 *
 * Parameters
 * Parameter         Description
 * interfaceHandle   Ethernet interface handle
 * userIndex         User Index (between 0, and tvMaxUserDhcpEntries - 1)
 * dhcpNotifyFuncPtr Pointer to a function that will be called when
 *                   the DHCP request has completed, or timed out.
 * Returns
 * Value            Meaning
 * TM_ENOERROR      success. The DHCP notify function will not be called.
 * TM_EINPROGRESS   DHCP request/discover sent with no error.
 * TM_EALREADY      DHCP request/discover previously sent.
 * TM_EINVAL        Bad parameter
 * TM_EADDRNOTAVAIL Bad Requested IP address 
 *                  (when TM_DHCPF_REQUESTED_IPADDRESS is set)
 * TM_ENOBUFS       Not enough memory
 * other            as returned by device driver send function.
 */
int tfDhcpUserStart ( ttUserInterface         interfaceHandle,
                      int                     userIndex,
                      ttUserDhcpNotifyFuncPtr dhcpNotifyFuncPtr )
{
    int errorCode;
    ttDeviceEntryPtr devEntryPtr;

    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
/* Check validity of the device */
    errorCode = tfValidInterface(devEntryPtr);
    if ( errorCode == TM_ENOERROR )
    {
/* Lock the device */
        tm_call_lock_wait(&(devEntryPtr->devLockEntry));
/* Check that index is within range allocated */
        if (    (userIndex < 0)
             || (userIndex >= tm_context(tvMaxUserDhcpEntries)) )
        {
            errorCode = TM_EINVAL;
        }
        else
        {
            if (     devEntryPtr->devBootInfo[TM_BOOT_DHCP].bootRecvFuncPtr
                 == (ttDevBootRecvFuncPtr)0 )
            {
/* If tfUseDhcp() has not been called */
                devEntryPtr->devBootInfo[TM_BOOT_DHCP].bootRecvFuncPtr
                                             = tfDhcpRecvCB;
            }
            errorCode = tfBtInit( devEntryPtr,
                                  dhcpNotifyFuncPtr,
                                  userIndex,
                                  TM_BT_USER,
                                  TM_BOOT_DHCP );
/* Call tfDhcpStart */
/* Start the state machine */
            if (errorCode == TM_ENOERROR)
            {
/* So that tfIpIncomingPacket() let DHCP packets through */
                devEntryPtr->devFlag |= TM_DEV_IP_DHCP;
                errorCode = tfDhcpStart( devEntryPtr,
                                         userIndex,
                                         TM_BT_USER);
            }
        }
/* Unlock the device */
        tm_call_unlock(&(devEntryPtr->devLockEntry));
    }
    return (errorCode);
}

/*
 * tfDhcpUserRelease function description:
 * Cancel a DHCP request and/or a DHCP lease.
 *
 * Parameters 
 * Parameter         Description
 * Value             Meaning
 * interfaceHandle   Ethernet interface handle
 * userIndex         User Index (between 0, and tvMaxUserDhcpEntries - 1)
 *
 * Returns
 * TM_ENOERROR      success.
 * TM_EINVAL        bad parameter
 */
int tfDhcpUserRelease(ttUserInterface    interfaceHandle,
                      int                userIndex )
{
    ttDeviceEntryPtr devEntryPtr;
    int              errorCode;
    
    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
/* Check validity of the device */
    errorCode = tfValidInterface(devEntryPtr);
    if ( errorCode == TM_ENOERROR )
    {
/* Lock the device */
        tm_call_lock_wait(&(devEntryPtr->devLockEntry));
/* Check that index is within range allocated */
        if (    (userIndex < 0)
             || (userIndex >= (devEntryPtr->devBtUserMaxEntries)) )
        {
            errorCode = TM_EINVAL;
        }
        else
        {
/* Call tfDhcpStop to free any timer, and let the lease expire */
            errorCode = tfDhcpStop( devEntryPtr,
                                    userIndex,
                                    TM_BT_USER );
        }
/* Unlock the device */
        tm_call_unlock(&(devEntryPtr->devLockEntry));
    }
    return errorCode;
}

/*
 * Set the DHCP initial state (INIT, or INIT_REBOOT) prior to the user
 * calling tfDhcpUserStart(). Used by the user when the user 
 * wants to specify his/her own Client ID, of if the user wants to 
 * start in INIT_REBOOT state, or if the user wants to specify an IP address in
 * the DISCOVER phase.
 *
 * Also used by the user to set/reset the host name option when flags is set 
 * to TM_DHCPF_HOST_NAME. In that case if the clientIdPtr is null, and 
 * clientIdLength is zero the host name option is reset, otherwise it will
 * be set using the name pointed to by clientIdPtr.
 * The HOST Name option must be set separately from other DHCP options.
 *
 * Also used by the user when the user wants to use the DHCP FQDN option to 
 * set its domain name.
 * If flags is set to TM_DHCPF_FQDN_ENABLE and
 * clientIdPtr is NULL, the global FQDN (in tvFqdnStruct) will be used
 * instead.
 * Note that the TM_DHCPF_FQDN_ENABLE and TM_DHCPF_FQDN_DISABLE options cannot
 * be set at the same time as the other flags.
 *
 * Call common routine tfDchpCommonSet()
 *
 * Parameter         Description
 * interfaceHandle    Ethernet interface handle
 * userIndex          User Index (between 0, and tvMaxUserDhcpEntries - 1)
 * flags              0, or a combination of TM_DHCPF_INIT_REBOOT, 
 *                    TM_DHCPF_REQUESTED_IP_ADDRESS, 
 *                    TM_DHCPF_SUPPRESS_CLIENT_ID,
 *                    TM_DHCPF_FQDN_ENABLE,
 *                    TM_DHCPF_FQDN_DISABLE,
 *                    TM_DHCPF_FQDN_PARTIAL
 *                    TM_DHCPF_FQDN_FLAG_S_ZERO
 *                    TM_DHCPF_FQDN_FLAG_N_ONE
 *                    TM_DHCPF_HOST_NAME
 *                     
 * requestedIpAddress User requested IP address.
 * clientIdPtr        Pointer to client ID. When flags is set to
 *                    TM_DHCPF_FQDN_ENABLE, the FQDN domain name.
 *                    If flags is set to TM_DHCPF_FQDN_ENABLE and  it is
 *                    NULL, the global FQDN (in tvFqdnStruct) will be used
 *                    instead.
 * clientIdLength     Length of client ID.
 *
 * Returns
 * Value            Meaning
 * TM_ENOERROR      success.
 * TM_EINVAL        Bad parameter
 * TM_EPERM         Call can only be made in INIT state, i.e. prior
 *                  to calling tfDhcpUserStart or
 *                  tfOpenInterface/tfConfigInterface
 * TM_ENOBUFS       Not enough memory
 */
TM_PROTO_EXTERN int tfDhcpUserSet (ttUserInterface interfaceHandle,
                                   int             userIndex,
                                   int             flags,
                                   ttUserIpAddress requestedIpAddress,
                                   unsigned char   TM_FAR * clientIdPtr,
                                   int             clientIdLength)
{
    int errorCode;

    errorCode = tfDhcpCommonSet( interfaceHandle,
                                 userIndex,
                                 flags,
                                 requestedIpAddress, 
                                 clientIdPtr,
                                 clientIdLength,
                                 TM_BT_USER );
    return errorCode;
}


/*
 * Set the DHCP initial state (INIT, or INIT_REBOOT) prior to the user calling
 * tfOpenInterface/tfConfigInterface(). Used by the user when the user 
 * wants to specify his/her own Client ID, of if the user wants to 
 * start in INIT_REBOOT state, or if the user wants to specify an IP address in
 * the DISCOVER phase.
 * 
 * Also used by the user to set/reset the host name option when flags is set 
 * to TM_DHCPF_HOST_NAME. In that case if the clientIdPtr is null, and 
 * clientIdLength is zero the host name option is reset, otherwise it will
 * be set using the name pointed to by clientIdPtr.
 * The HOST Name option must be set separately from other DHCP options.
 *
 * Also used by the user when the user wants to use the DHCP FQDN option to 
 * set its domain name.
 * If flags is set to TM_DHCPF_FQDN_ENABLE and
 * clientIdPtr is NULL, the global FQDN (in tvFqdnStruct) will be used
 * instead.
 * Note that the TM_DHCPF_FQDN_ENABLE and TM_DHCPF_FQDN_DISABLE options cannot
 * be set at the same time as the other flags.
 *
 * Check that mhomeIndex is within a valid range.
 * Call common routine tfDchpCommonSet()
 *
 * Parameter         Description
 * interfaceHandle    Ethernet interface handle
 * mHomeIndex         multihome Index.
 * flags              0, or a combination of TM_DHCPF_INIT_REBOOT, 
 *                    TM_DHCPF_REQUESTED_IP_ADDRESS, 
 *                    TM_DHCPF_SUPPRESS_CLIENT_ID
 *                    TM_DHCPF_FQDN_ENABLE,
 *                    TM_DHCPF_FQDN_DISABLE,
 *                    TM_DHCPF_FQDN_PARTIAL
 *                    TM_DHCPF_FQDN_FLAG_S_ZERO
 *                    TM_DHCPF_FQDN_FLAG_N_ONE
 *                    TM_DHCPF_HOST_NAME
 *                     
 * requestedIpAddress User requested IP address.
 * clientIdPtr        Pointer to client ID. When flags is set to
 *                    TM_DHCPF_FQDN_ENABLE, the FQDN domain name.
 *                    If flags is set to TM_DHCPF_FQDN_ENABLE and  it is
 *                    NULL, the global FQDN (in tvFqdnStruct) will be used
 *                    instead.
 * clientIdLength     Length of client ID
 *
 * Returns
 * Value            Meaning
 * TM_ENOERROR      success.
 * TM_EINVAL        Bad parameter
 * TM_EPERM         Call can only be made in INIT state, i.e. prior
 *                  to calling tfDhcpUserStart or
 *                  tfOpenInterface/tfConfigInterface
 * TM_ENOBUFS       Not enough memory
 */
TM_PROTO_EXTERN int tfDhcpConfSet (ttUserInterface interfaceHandle,
                                   int             mHomeIndex,
                                   int             flags,
                                   ttUserIpAddress requestedIpAddress,
                                   unsigned char   TM_FAR * clientIdPtr,
                                   int             clientIdLength)
{
    int errorCode;

    if ( (mHomeIndex >= TM_MAX_IPS_PER_IF) || (mHomeIndex < 0) )
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        errorCode = tfDhcpCommonSet( interfaceHandle,
                                     mHomeIndex,
                                     flags, 
                                     requestedIpAddress, 
                                     clientIdPtr,
                                     clientIdLength,
                                     TM_BT_CONF );
    }
    return errorCode;
}

/*
 * tfDhcpCommonSet function description
 * Set the DHCP initial state (INIT, or INIT_REBOOT), or set the FQDN option
 * prior to the user calling tfDhcpUserStart, or 
 * tfOpenInterface/tfConfigInterface
 * . If user specifies INIT_REBOOT state, the Requested IP address
 *   need to be specified as well.
 * . If user specifies INIT state, optionally the user can specify the IP 
 *   address, and or the CLIENT ID option. 
 * . If CLIENT ID is not specified, and not suppressed, the stack will pick 
 *   a unique CLIENT ID that will be the same across reboots provided that
 *   the user uses the same type of configuration and same index.
 * When flags is set to TM_DHCPF_HOST_NAME, tfDhcpCommonSet is used to 
 * set/reset the DHCP host name. The HOST Name option must be set separately 
 * from other DHCP options.
 * When flags is set to TM_DHCPF_FQDN_ENABLE, tfDhcpCommonSet is used to
 * set the FQDN domain name. The FQDN option must be set separately from other
 * DHCP options (The only other flag allowed with TM_DHCPF_FQDN_ENABLE is
 * TM_DHCPF_FQDN_PARTIAL, TM_DHCPF_FLAG_S, and TM_DHCPF_FLAG_N_ONE). If flags
 * is set to TM_DHCPF_FQDN_ENABLE and clientIdPtr is NULL, the global FQDN (in
 * tvFqdnStruct) will be used instead.
 *
 * 1. Check that the parameters are valid:
 * . flags has to be 0, or a combination of TM_DHCPF_INIT_REBOOT, 
 *                                    TM_DHCPF_REQUESTED_IP_ADDRESS, 
 *                                    TM_DHCPF_SUPPRESS_CLIENT_ID
 *                                    TM_DHCPF_FQDN_ENABLE,
 *                                    TM_DHCPF_FQDN_DISABLE,
 *                                    TM_DHCPF_FQDN_PARTIAL
 *                                    TM_DHCPF_FQDN_FLAG_S_ZERO
 *                                    TM_DHCPF_FQDN_FLAG_N_ONE
 *                                    TM_DHCPF_HOST_NAME
 * . Check that the requestedIpAddress field is valid, if 
 *   the TM_DHCPF_REQUESTED_IP_ADDRSS bit is set in the flags field.
 * . If user wants to set the state to INIT_REBOOT, then the IP address 
 *   parameter cannnot be zero. 
 * . If TM_DHCPF_FQDN_ENABLE is not set, and TM_DHCPF_HOST_NAME is not set,
 *   if client ID pointer is non null, the client Id option length should be 
 *   at least 2, and should not exceed TM_DHCP_CLIENTID_SIZE.
 * . If TM_DHCPF_HOST_NAME is set, and clientIdLength is not zero, then the
 *   client ID option length should be at least 1, and should not exceed
 *   TM_DHCP_HOSTNAME_SIZE.
 * . Check that the interface is valid
 * . Check that the user index is within range allocated (if called from
 *   tfDhcpUserSet)
 * 2. Allocate a btEntry corresponding to index, indexType
 * 3. Set DHCP state to requested state.
 * 4. Save requested IP address, and set requested IP address flag, if 
 *    requested IP address is non zero, otherwise reset the Requested IP 
 *    address flag.
 * 5. Save Client ID option, and set user Client ID option flag.
 * 6. If flags is set to FQDN_ENABLE, check no other flag is set except
 *    TM_DHCPF_FQDN_PARTIAL, TM_DHCPF_FQDN_FLAG_S_ZERO, 
 *    TM_DHCPF_FQDN_FLAG_N_ONE
 * 7. If it is not null, verify the domain name is correct by calling
 *    tfHostnameTextToBinary
 * 8. Copy the corrext domain name in btEntryPtr->btUserFqdnPtr (either the
 *    global one or the one passed), the correct length and set the flags
 *    properly
 *
 * Parameters
 * Parameter         Description
 * interfaceHandle    Ethernet interface handle
 * index              User Index (between 0, and tvMaxUserDhcpEntries - 1) if
 *                    indexType is TM_BT_CONF, otherwise mhomeIndex.
 * flags              0, or a combination of TM_DHCPF_INIT_REBOOT, 
 *                    TM_DHCPF_REQUESTED_IP_ADDRESS, 
 *                    TM_DHCPF_SUPPRESS_CLIENT_ID
 *                    TM_DHCPF_FQDN_ENABLE, TM_DHCPF_FQDN_DISABLE,
 *                    TM_DHCPF_FQDN_PARTIAL, TM_DHCPF_FQDN_FLAG_S_ZERO,
 *                    TM_DHCPF_FQDN_FLAG_N_ONE.
 *                    The FQDN option must be set separately from other DHCP
 *                    options (The only other flag allowed with
 *                    TM_DHCPF_FQDN_ENABLE is TM_DHCPF_FQDN_PARTIAL).
 *                    TM_DHCPF_FQDN_PARTIAL is ignored if clientIdPtr is
 *                    NULL).
 * requestedIpAddress User set IP address.
 * clientIdPtr        Pointer to client ID. When flags is set to
 *                    TM_DHCPF_FQDN_ENABLE, the FQDN domain name.
 *                    If flags is set to TM_DHCPF_FQDN_ENABLE and  it is
 *                    NULL, the global FQDN (in tvFqdnStruct) will be used
 *                    instead.
 * clientIdLength     Length of client ID
 * indexType          TM_BT_CONF, or TM_BT_USER
 * Returns
 * Value            Meaning
 * TM_ENOERROR      success.
 * TM_EINVAL        Bad parameter
 * TM_EADDRNOTAVAIL Bad Requested IP address 
 *                  (when TM_DHCPF_REQUESTED_IPADDRESS is set)
 * TM_EPERM         Call can only be made in INIT state, i.e. prior
 *                  to calling tfDhcpUserStart or
 *                  tfOpenInterface/tfConfigInterface
 * TM_ENOBUFS       Not enough memory
 */
static int tfDhcpCommonSet( ttUserInterface interfaceHandle,
                            int             index,
                            int             flags,
                            ttUserIpAddress requestedIpAddress,
                            unsigned char   TM_FAR * clientIdPtr,
                            int             clientIdLength,
                            tt8Bit          indexType )
{
    ttDeviceEntryPtr devEntryPtr;
    ttBtEntryPtr     btEntryPtr;
    int              errorCode;
#ifdef TM_USE_DHCP_FQDN
    tt8BitPtr        charTmpPtr;
    int              tmpLength;
#endif /* TM_USE_DHCP_FQDN */

/* Flags field should not have bits that are not defined set */
    if ( (flags & ~(TM_DHCPF_FLAGS)) != 0)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        errorCode = TM_ENOERROR;
#ifdef TM_USE_DHCP_FQDN
        if ( (flags & TM_DHCPF_FQDN_DISABLE) != 0)
        {
            if ( (flags & ~(TM_DHCPF_FQDN_DISABLE)) != 0 )
            {
                errorCode = TM_EINVAL;
            }
        }
        else if ( (flags & TM_DHCPF_FQDN_ENABLE) != 0)
        {
            if ( (flags & ~(TM_DHCPF_FQDN_FLAGS)) != 0 )
            {
                errorCode = TM_EINVAL;
            }
/* 
 * FQDN length should be at least 1 byte, and should not exceed 
 * TM_FDQN_MAX_LEN (255).
 */
            if (errorCode == TM_ENOERROR)
            {
                if (     (clientIdLength > TM_FQDN_MAX_LEN)
                     ||  (   (clientIdLength == 0)
                          && (clientIdPtr != (unsigned char TM_FAR *)0)) )
                {
                    errorCode = TM_EINVAL;
                }
            }
            if (errorCode == TM_ENOERROR) 
            {
                if (clientIdPtr != (unsigned char TM_FAR *)0)
                {
                    if (!(flags & TM_DHCPF_FQDN_ASCII))
                    {
/* Check the hostname is valid by attempting to convert it */
                        charTmpPtr = (tt8BitPtr)
                                    tm_kernel_malloc(TM_FQDN_MAX_LEN + 1);
                        if (charTmpPtr == (tt8BitPtr)0)
                        {
                            errorCode = TM_ENOBUFS;
                        }
                        else
                        {
                            errorCode = tfHostnameTextToBinary(charTmpPtr,
                                               &tmpLength,
                                               (ttCharPtr)clientIdPtr,
                                               clientIdLength,
                                               flags);
                            tm_kernel_free(charTmpPtr);
                            if (tmpLength > TM_FQDN_MAX_LEN)
                            {
/* Make sure the option length will fit in one byte */
                                errorCode = TM_EINVAL;
                            }
                        }
                    }
                }
            }
        }
        else
#endif /* TM_USE_DHCP_FQDN */
/* Host name option can only be set by itself */
        if (flags & TM_DHCPF_HOST_NAME)
        {
            if (    (flags != TM_DHCPF_HOST_NAME)
                 || (    (clientIdPtr == (unsigned char TM_FAR *)0) 
                      && (clientIdLength != 0) )
                 || (    (clientIdPtr != (unsigned char TM_FAR *)0)
                      && (    (clientIdLength < 1)
                           || (clientIdLength > TM_DHCP_HOSTNAME_SIZE) ) )
               )
            {
                errorCode = TM_EINVAL;
            }
        }
        else
        {
/* Check validity of requested IP address */
            if (
#ifdef TM_USE_DHCP_FQDN
                 (errorCode == TM_ENOERROR) &&
#endif /* TM_USE_DHCP_FQDN */
                 (flags & TM_DHCPF_REQUESTED_IP_ADDRESS) )
            {
                if (    tm_ip_is_class_d_e(requestedIpAddress)
                     || tm_ip_is_loop_back(requestedIpAddress)
                     || tm_ip_zero(requestedIpAddress) )
                {
                    errorCode = TM_EADDRNOTAVAIL;
                }
            }
/* 
 * In INIT_REBOOT state, need requested IP address 
 */
            if ((errorCode == TM_ENOERROR) && (flags & TM_DHCPF_INIT_REBOOT))
            {
                if (!(flags & TM_DHCPF_REQUESTED_IP_ADDRESS))
                {
                    errorCode = TM_EINVAL;
                }
            }
/* 
 * Client ID length should be at least 2 bytes long, and should not exceed 
 * TM_CLIENTID_SIZE
 */
            if (    (errorCode == TM_ENOERROR)
                 && !(flags & TM_DHCPF_SUPPRESS_CLIENT_ID) )
            {
                if (clientIdPtr != (unsigned char TM_FAR *)0)
                {
                    if (    (clientIdLength < 2) 
                         || (clientIdLength > TM_DHCP_CLIENTID_SIZE) )
                    {
                        errorCode = TM_EINVAL;
                    }
                }
            }
        }
    }
    if (errorCode == TM_ENOERROR)
    {
        devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
/* Check validity of the device */
        errorCode = tfValidInterface(devEntryPtr);
        if ( errorCode == TM_ENOERROR )
        {
            btEntryPtr = (ttBtEntryPtr)0;
/* Lock the device */
            tm_call_lock_wait(&(devEntryPtr->devLockEntry));
/* Check that index is within range allocated */
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
/* Get/Allocate btEntry corresponding to index, indexType on the device */
                btEntryPtr = tfBtAlloc(devEntryPtr, index, indexType,
                                       TM_BOOT_DHCP);
                if (btEntryPtr == (ttBtEntryPtr)0)
                {
                    errorCode = TM_ENOBUFS;
                }
                else
                {
                    if (btEntryPtr->btDhcpState != TM_DHCPS_INIT) 
                    {
#ifdef TM_USE_DHCP_FQDN
                        if (!(flags & TM_DHCPF_FQDN_ENABLE))
#endif /* TM_USE_DHCP_FQDN */
                        {
                            errorCode = TM_EPERM;
                        }
                    }
                }
            }
            if (errorCode == TM_ENOERROR)
            {
#ifdef TM_USE_DHCP_FQDN
                if (flags & TM_DHCPF_FQDN_DISABLE)
                {
                    if (btEntryPtr->btUserFqdnPtr != (char TM_FAR *)0)
                    {
                        if (btEntryPtr->btUserFqdnPtr !=
                                        tm_context(tvFqdnStruct).domainName)
                        {
/* Release the dynamically allocated FQDN storage */
                            tm_kernel_free(btEntryPtr->btUserFqdnPtr);
                        }
                        btEntryPtr->btUserFqdnPtr = (char TM_FAR *)0;
                    }
                    btEntryPtr->btUserFqdnLen = (tt8Bit)0;
/* Store the FQDN flags */
                    btEntryPtr->btFqdnFlags &= (tt8Bit)~TM_DHCPF_FQDN_ENABLE;
                }
                else if (flags & TM_DHCPF_FQDN_ENABLE)
                {
                    if (clientIdPtr == (unsigned char TM_FAR *)0)
                    {
/* Lock FQDN global structure */
                        tm_call_lock_wait(&tm_context(tvFqdnStructLock));
                        if (tm_context(tvFqdnStruct).domainNameLen == 0)
                        {
/* The default (global) FQDN must be set if the user did not
 * provide one in clientIdPtr
 */
                            errorCode = TM_EINVAL;
                        }
                        else
                        {
/* We will use Length, and flags in global structure when sending the option */
                            btEntryPtr->btUserFqdnPtr =
                                    tm_context(tvFqdnStruct).domainName;
                        }
/* UnLock FQDN global structure */
                        tm_call_unlock(&tm_context(tvFqdnStructLock));
                    }
                    else
                    {
                        if (    (btEntryPtr->btUserFqdnPtr ==
                                        tm_context(tvFqdnStruct).domainName)
                             || (btEntryPtr->btUserFqdnPtr == (char *) 0) )
                        {
/* allocate a buffer to store the FQDN domain name */
                            btEntryPtr->btUserFqdnPtr = 
                                    tm_kernel_malloc(clientIdLength + 1);
                        }
                        else if ((unsigned)btEntryPtr->btUserFqdnLen
                                                   < (unsigned)clientIdLength)
                        {
/* Re-allocate a buffer to store the FQDN domain name */
                            tm_kernel_free(btEntryPtr->btUserFqdnPtr);
                            btEntryPtr->btUserFqdnPtr =
                                        tm_kernel_malloc(clientIdLength + 1);
                        }
                        if (btEntryPtr->btUserFqdnPtr == (char *)0)
                        {
                            errorCode = TM_ENOBUFS;
                        }
                        else
                        {
                            btEntryPtr->btUserFqdnLen = (tt8Bit)clientIdLength;
                            tm_memcpy(btEntryPtr->btUserFqdnPtr,
                                      clientIdPtr,
                                      clientIdLength + 1);
/* Store the FQDN flags */
                            btEntryPtr->btFqdnFlags = (tt8Bit)
                                                (flags & TM_DHCPF_FQDN_FLAGS);
                        }
                    }
                    if (errorCode == TM_ENOERROR)
                    {
                        btEntryPtr->btFqdnFlags |= TM_DHCPF_FQDN_ENABLE;
                    }
                }
                else
#endif /* TM_USE_DHCP_FQDN */
                if (flags == TM_DHCPF_HOST_NAME)
                {
                    if (clientIdPtr != (unsigned char TM_FAR *)0)
                    {
/* Save user host name option */
                        tm_bcopy( clientIdPtr, 
                                  btEntryPtr->btUserSetDhcpHostNamePtr,
                                  clientIdLength );
                    }
                    btEntryPtr->btUserSetDhcpHostNameLength = 
                                                  (tt8Bit)clientIdLength;
                }
                else
                {
                    if (flags & TM_DHCPF_INIT_REBOOT)
                    {
                        btEntryPtr->btDhcpState = TM_DHCPS_INITREBOOT;
                    }
/* Reset previously set flags */
                    btEntryPtr->btUserSetFlags = TM_8BIT_ZERO; 
                    if (flags & TM_DHCPF_REQUESTED_IP_ADDRESS)
                    {
/* Save user requested IP address */
                        tm_ip_copy(requestedIpAddress, 
                                   btEntryPtr->btUserSetDhcpIpAddress);
                        btEntryPtr->btUserSetFlags |=
                                               TM_BT_USER_SET_IPADDRESS;
                    }
                    if (flags & TM_DHCPF_SUPPRESS_CLIENT_ID)
                    {
                        btEntryPtr->btUserSetFlags |=
                                               TM_BT_SUPPRESS_CLIENT_ID;
                    }
                    else
                    {
                        if (clientIdPtr != (unsigned char TM_FAR *)0)
                        {
/* Save user Client ID option */
                            tm_bcopy( clientIdPtr, 
                                      btEntryPtr->btUserSetDhcpClientIdPtr,
                                      clientIdLength );
                            btEntryPtr->btUserSetDhcpClientIdLength = 
                                      (tt8Bit)clientIdLength;
                            btEntryPtr->btUserSetFlags
                                              |= TM_BT_USER_SET_CLIENTID;
                        }
                    }
                }
            }
/* Unlock the device */
            tm_call_unlock(&(devEntryPtr->devLockEntry));
        }
    }
    return errorCode;
}

#ifdef TM_USE_DHCP_FQDN
/* tfHostnameTextToBinary
 *
 * This function converts a host name from text to binary format. 
 *
 * Parameters       Description
 * hostNameBinPtr   The name in binary format (result of the conversion)
 * binLengthPtr     The length of the binary host name
 * hostNameTxtPtr   Pointer to the text ASCII hostname.
 * txtLength        The ASCII format domain name length 
 * flags            Indicates if the input text domain name is partial
 *                  or not  (bit TM_FQDNF_PARTIAL)
 * Returns
 * Value            Meaning
 * TM_ENOERROR      no error
 * TM_EINVAL        Invalid FQDN domain name
 */
TM_PROTO_EXTERN int tfHostnameTextToBinary(tt8BitPtr hostNameBinPtr,
                                           ttIntPtr  binLengthPtr,
                                           ttCharPtr hostNameTxtPtr,
                                           int       txtLength,
                                           int       flags)
{
    int       errorCode;
    int       tagLen;
    ttCharPtr tmpHostNameTxtPtr;

    if (hostNameTxtPtr == (ttCharPtr)0 || 
        hostNameBinPtr == (tt8BitPtr)0 || 
        binLengthPtr == (ttIntPtr)0 ||
        txtLength > TM_FQDN_MAX_LEN ||
        hostNameTxtPtr[txtLength-1] == '.' ||
        tm_strlen(hostNameTxtPtr) != (unsigned) txtLength)
    {
        errorCode = TM_EINVAL;
        goto hostNameTextToBinaryExit;
    }
    *binLengthPtr = 0;
    tmpHostNameTxtPtr = hostNameTxtPtr;
    tagLen = 0;
    while (txtLength >= 0)
    {
        if (*tmpHostNameTxtPtr == '.' || 
            *tmpHostNameTxtPtr == 0)
        {
            if (tagLen == 0)
            {
                errorCode = TM_EINVAL;
                goto hostNameTextToBinaryExit;
            }
/* We already scanned a whole tag, write it in the binary string.
 * In binary format, a tag starts by its length
 */ 
            *hostNameBinPtr = (tt8Bit)tagLen;
            hostNameBinPtr++;
            (*binLengthPtr)++;
            tm_memcpy(hostNameBinPtr, hostNameTxtPtr, tagLen);
            hostNameBinPtr += tagLen;
            *binLengthPtr += tagLen;
            if (*tmpHostNameTxtPtr == 0)
            {
                break;
            }
            hostNameTxtPtr += tagLen + 1;
            tagLen = 0;
        }
        else
        {
            tagLen++;
        }
        tmpHostNameTxtPtr++;
        txtLength--;
    }
    if (!(flags & TM_FQDNF_PARTIAL))
    {
/* Unless the hostname is partial, the binary string must end with a 0 */
        (*binLengthPtr) ++;
        *hostNameBinPtr = 0;
    }
    errorCode = TM_ENOERROR;
hostNameTextToBinaryExit:
    return errorCode;
}
#endif /* TM_USE_DHCP_FQDN */

#ifdef TM_USE_DHCP_FQDN
/* tfHostnameBinaryToText
 *
 * Parameter        Description
 * hostNameTxtPtr   Pointer to the text ASCII hostname. The result of
 *                  the conversion will be copied in it
 * txtLengthPtr     The ASCII format domain name length 
 * hostNameBinPtr   The name in binary format to be converted
 * binLength        The length of binary host name
 * flagsPtr         Modified to indicate if the output domain name is
 *                  partial or not (bit TM_FQDNF_PARTIAL)
 *
 * Returns
 * Value            Meaning
 *  TM_ENOERROR     no error
 *  TM_EINVAL       Invalid FQDN domain name
 */
int tfHostnameBinaryToText(ttCharPtr    hostNameTxtPtr,
                           ttIntPtr     txtLengthPtr,
                           tt8BitPtr    hostNameBinPtr,
                           int          binLength,
                           ttIntPtr     flagsPtr)
{
    int errorCode;
    int tagLen;

    if (hostNameTxtPtr == (ttCharPtr)0 ||
        hostNameBinPtr == (tt8BitPtr)0 ||
        binLength > TM_FQDN_MAX_LEN ||
        txtLengthPtr == (ttIntPtr)0)
    {
        errorCode = TM_EINVAL;
        goto hostNameBinaryToTextExit;
    }
    *flagsPtr |= TM_FQDNF_PARTIAL;
    *txtLengthPtr = 0;

    while (binLength > 0)
    {
        tagLen = *hostNameBinPtr;
        hostNameBinPtr++;
        binLength--;
        if (tagLen == 0)
        {
            *flagsPtr &= ~TM_FQDNF_PARTIAL;
            if (binLength > 0)
            {
                errorCode = TM_EINVAL;
                goto hostNameBinaryToTextExit;
            }
        }
        else
        {
            if (binLength < tagLen)
            {
                errorCode = TM_EINVAL;
                goto hostNameBinaryToTextExit;
            }
            if (*txtLengthPtr != 0)
            {
                *hostNameTxtPtr = '.';
                hostNameTxtPtr++;
                (*txtLengthPtr)++;
            }
            tm_memcpy(hostNameTxtPtr, hostNameBinPtr, tagLen);
            hostNameTxtPtr += tagLen;
            *txtLengthPtr += tagLen;
            hostNameBinPtr += tagLen;
            binLength -= tagLen;
        }
    }
    *hostNameTxtPtr = 0;
    errorCode = TM_ENOERROR;

hostNameBinaryToTextExit:
    return errorCode;
}
#endif /* TM_USE_DHCP_FQDN */

#ifdef TM_USE_DHCP_FQDN
/*
 * Set the DHCP global FQDN domain name.
 * This name is the default FQDN domain name. It will be sent in the
 * DHCP FQDN option unless the user provided a domain name for a specific
 * interface/multi home index or user index using tfDhcpUSerSet or
 * tfDhcpConfSet.
 *
 *
 * Parameters:
 *      fqdnLen     the length of the domain name character
 *                  string (cannot be greater than 255)
 *      fqdnPtr     pointer to the domain name char string.
 *      flags       Set it to TM_DHCPF_FQDN_PARTIAL if the domain
 *                  name is partial.
 * 
 * Returns:
 *      TM_ENOERROR success
 *      TM_EINVAL   Invalid FQDN domain name
 *      TM_ENOBUFS  Out of memory
 */
TM_PROTO_EXTERN int tfUserSetFqdn(ttConstCharPtr fqdnPtr,
                                  int            fqdnLen,
                                  int            flags)
{
    tt8BitPtr       tmpCharPtr;
    int             length;
    int             errorCode;

    tmpCharPtr = (tt8BitPtr)0;

    if ((fqdnPtr == (ttConstCharPtr)0) || (fqdnLen > TM_FQDN_MAX_LEN))
    {
        errorCode = TM_EINVAL;
        goto userSetFqdnExit;
    }
    if (!(flags & TM_DHCPF_FQDN_ASCII))
    {
        tmpCharPtr = (tt8BitPtr)tm_kernel_malloc(TM_FQDN_MAX_LEN + 1);
        if (tmpCharPtr == (tt8BitPtr)0)
        {
            errorCode = TM_ENOBUFS;
            goto userSetFqdnExit;
        }
        errorCode = tfHostnameTextToBinary(tmpCharPtr, &length,
                                           (char *)fqdnPtr, fqdnLen, flags);
        tm_kernel_free(tmpCharPtr);
        if (length > TM_FQDN_MAX_LEN)
        {
/* Make sure the option length will fit in one byte */
            errorCode = TM_EINVAL;
        }
        if (errorCode != TM_ENOERROR)
        {
            goto userSetFqdnExit;
        }
    }
/* Need to bzero if not first call into this routine */
    tm_bzero(tm_context(tvFqdnStruct).domainName, TM_FQDN_MAX_LEN + 1);
/* Lock FQDN global structure */
    tm_call_lock_wait(&tm_context(tvFqdnStructLock));
    tm_memcpy(tm_context(tvFqdnStruct).domainName,
              fqdnPtr, fqdnLen + 1);
    tm_context(tvFqdnStruct).domainNameLen = (tt8Bit)fqdnLen;
/* Store the FQDN flags */
    tm_context(tvFqdnStruct).domainFlags = (tt8Bit)
                                                (flags & TM_DHCPF_FQDN_FLAGS);
/* UnLock FQDN global structure */
    tm_call_unlock(&tm_context(tvFqdnStructLock));
    errorCode = TM_ENOERROR;

userSetFqdnExit:
    return errorCode;
}
#endif /* TM_USE_DHCP_FQDN */

/*
 * Set custom DHCP configuration options prior to the user calling
 * tfDhcpUserStart(). This function is to set options that are not natively
 * supported by the DHCP client. For use with the DHCP User Controlled
 * Configuration API.
 * 
 * Parameter         Description
 * interfaceHandle    Ethernet interface handle
 * index              User index to set this option on
 * bootOption         DHCP option to set
 * dataPtr            Data to be sent in option
 * dataLen            Length of data pointed to by dataPtr
 *
 * Returns
 * See tfBtSetOption()
 */
int tfDhcpUserSetOption(
                  ttUserInterface interfaceHandle,
                  int             index,
                  ttUser8Bit      bootOption,
                  ttUserCharPtr   dataPtr,
                  ttUser8Bit      dataLen)
{
    return tfBtSetOption(interfaceHandle,
                         index,
                         bootOption,
                         dataPtr,
                         dataLen,
                         TM_BT_USER,
                         TM_BOOT_DHCP);
}

/*
 * Retrieve DHCP options sent by the server that aren't natively supported
 * by the DHCP client. Any options unrecognized by the client are stored in
 * a special list that is accessible through this funciton. For use with the
 * DHCP User Controlled Configuration API.
 * 
 * Parameter         Description
 * interfaceHandle    Ethernet interface handle
 * index              User index to get this option from
 * bootOption         DHCP option to retrieve
 * dataPtr            Buffer to store option data into
 * dataLenPtr         Pointer to the length of the data buffer. Must be set
 *                    by the user to the maximum number of bytes the buffer
 *                    can handle. It will be reset to the actual length of
 *                    data stored in the buffer.
 *
 * Returns
 * See tfBtGetOption()
 */
int tfDhcpUserGetOption(
                  ttUserInterface  interfaceHandle,
                  int              index,
                  ttUser8Bit       bootOption,
                  ttUserCharPtr    dataPtr,
                  ttUser8BitPtr    dataLenPtr)
{
    return tfBtGetOption(interfaceHandle,
                         index,
                         bootOption,
                         dataPtr,
                         dataLenPtr,
                         TM_BT_USER,
                         TM_BOOT_DHCP);
}

/*
 * Set custom DHCP configuration options prior to the user calling
 * tfOpenInterface/tfConfigInterface(). This function is to set options that
 * are not natively supported by the DHCP client. For use with the DHCP
 * Automatic Configuration API.
 * 
 * Parameter         Description
 * interfaceHandle    Ethernet interface handle
 * index              Multi-home index to set this option on
 * bootOption         DHCP option to set
 * dataPtr            Data to be sent in option
 * dataLen            Length of data pointed to by dataPtr
 *
 * Returns
 * See tfBtSetOption()
 */
int tfDhcpConfSetOption(
                  ttUserInterface interfaceHandle,
                  int             index,
                  ttUser8Bit      bootOption,
                  ttUserCharPtr   dataPtr,
                  ttUser8Bit      dataLen)
{
    return tfBtSetOption(interfaceHandle,
                         index,
                         bootOption,
                         dataPtr,
                         dataLen,
                         TM_BT_CONF,
                         TM_BOOT_DHCP);
}

/*
 * Retrieve DHCP options sent by the server that aren't natively supported
 * by the DHCP client. Any options unrecognized by the client are stored in
 * a special list that is accessible through this funciton. For use with the
 * DHCP Automatic Configuration API.
 * 
 * Parameter         Description
 * interfaceHandle    Ethernet interface handle
 * index              Multi-home index to get this option from
 * bootOption         DHCP option to retrieve
 * dataPtr            Buffer to store option data into
 * dataLenPtr         Pointer to the length of the data buffer. Must be set
 *                    by the user to the maximum number of bytes the buffer
 *                    can handle. It will be reset to the actual length of
 *                    data stored in the buffer.
 *
 * Returns
 * See tfBtGetOption()
 */
int tfDhcpConfGetOption(
                  ttUserInterface  interfaceHandle,
                  int              index,
                  ttUser8Bit       bootOption,
                  ttUserCharPtr    dataPtr,
                  ttUser8BitPtr    dataLenPtr)
{
    return tfBtGetOption(interfaceHandle,
                         index,
                         bootOption,
                         dataPtr,
                         dataLenPtr,
                         TM_BT_CONF,
                         TM_BOOT_DHCP);
}

#ifdef TM_USE_DHCP_COLLECT
/*
 * Control operational attributes of DHCP on an interface.
 *
 * Parameter        Description
 * interfaceHandle  Interface handle as returned by tfAddInterface().
 * multiHomeIndex   Index of IPv4, DHCP configured address.
 * attrName         Name of attribute.
 * attrValuePtr     Pointer to user variable containing attribute value.
 * attrLength       Length of the attribute value in bytes.
 * btType           DHCP entry type (TM_BT_CONF or TM_BT_USER).
 *
 * Returns:
 *  TM_ENOERROR     Success
 *  TM_EINVAL       Invalid parameter
 *  TM_EPERM        Operation not permitted in current state
 *  TM_ENOENT       Entry not found in cache
 */
static int tfDhcpSetAttributes( ttUserInterface interfaceHandle,
                                unsigned int    multiHomeIndex,
                                int             attrName,
                                void   TM_FAR * attrValuePtr,
                                int             attrLength,
                                tt8Bit          btType )
{
    ttDeviceEntryPtr    devPtr;
    ttBtEntryPtr        btEntryPtr;
    int                 errorCode;
    int                 errCode;
    ttIntPtr            intPtr;
    tt4IpAddress        serverId;

    errorCode = TM_EINVAL;
    devPtr  = (ttDeviceEntryPtr)interfaceHandle;
    errCode = tfValidInterface(devPtr);
    if (    attrValuePtr != TM_VOID_NULL_PTR
        &&  errCode == TM_ENOERROR )
    {
/* Lock device */
        tm_call_lock_wait(&(devPtr->devLockEntry));
/*
 * Check that the multi-home index is within range, and get/allocate a
 * bt entry.
 */
        btEntryPtr = tfBtAlloc( devPtr,
                                (int)multiHomeIndex,
                                btType,
                                TM_BOOT_DHCP );
        if (btEntryPtr != (ttBtEntryPtr)0)
        {
            switch (attrName)
            {
/* Legacy mode of operation: unconditionally select the first DHCP offer
 * received. This is the default mode. It is the only mode supported by
 * previous Treck versions. */
            case TM_DHCP_ATTR_LEGACY_MODE:
                if (btEntryPtr->btDhcpState != TM_DHCPS_INIT)
                {
                    errorCode = TM_EPERM;
                }
                else
                {
                    btEntryPtr->btCollectSize = 1;
                    devPtr->devOfferFuncPtr = (ttUserDhcpOfferCBFuncPtr)0;
                    tm_ip_copy(TM_IP_ZERO, btEntryPtr->btPrefAddr);
                    errorCode = TM_ENOERROR;
                }
                break;

/* Preferred IP address: select any offer that matches a user-defined
 * IP address. If no matching offer is received, select any other offer. */
            case TM_DHCP_ATTR_PREF_ADDRESS:
                if (btEntryPtr->btDhcpState != TM_DHCPS_INIT)
                {
                    errorCode = TM_EPERM;
                }
                else
                {
                    if ((unsigned)attrLength == sizeof(tt4IpAddress))
                    {
                        btEntryPtr->btCollectSize = 1;
                        devPtr->devOfferFuncPtr = (ttUserDhcpOfferCBFuncPtr)0;
                        tm_ip_copy( *(tt4IpAddressPtr)attrValuePtr,
                                                btEntryPtr->btPrefAddr );
                        tm_8bit_clr_bit( btEntryPtr->btFlags,
                                            TM_BOOTF_EXCLUSIVE_ADDRESS );
                        errorCode = TM_ENOERROR;
                    }
                }
                break;

/* Exclusive IP address: select any offer that matches a user-defined
 * IP address. If no matching offer is received, cancel DHCP. */
            case TM_DHCP_ATTR_EXCL_ADDRESS:
                if (btEntryPtr->btDhcpState != TM_DHCPS_INIT)
                {
                    errorCode = TM_EPERM;
                }
                else
                {
                    if ((unsigned)attrLength == sizeof(tt4IpAddress))
                    {
                        btEntryPtr->btCollectSize = 1;
                        devPtr->devOfferFuncPtr = (ttUserDhcpOfferCBFuncPtr)0;
                        tm_ip_copy( *(tt4IpAddressPtr)attrValuePtr,
                                                btEntryPtr->btPrefAddr );
                        btEntryPtr->btFlags
                                    |= (tt8Bit)TM_BOOTF_EXCLUSIVE_ADDRESS;
                        errorCode = TM_ENOERROR;
                    }
                }
                break;

/* Select an offer by server id. An offer with a matching server id must
 * reside in the offer collection cache. Treck will send a DHCPREQUEST
 * for the matching entry and end the offer collection phase. */
            case TM_DHCP_ATTR_CACHE_SELECT:
                if (btEntryPtr->btDhcpState != TM_DHCPS_SELECTING)
                {
                    errorCode = TM_EPERM;
                }
                else
                {
                    if ((unsigned)attrLength == sizeof(tt4IpAddress))
                    {
                        tm_ip_copy(*(tt4IpAddressPtr)attrValuePtr, serverId);
                        if (tm_ip_not_zero(serverId))
                        {
/* Remove all entries in the offer cache except for the one matching
 * the selected server. */
                            errorCode = tfDhcpDeleteOffers(
                                        btEntryPtr,
                                        serverId,
                                        TM_DHCP_OFFER_KEEP );
                            if (errorCode == TM_ENOERROR)
                            {
                                errorCode = tfDhcpRequestOffer(btEntryPtr);
                            }
                        }
                    }
                }
                break;

/* Remove an offer by server id. An offer with a matching server id must
 * reside in the offer collection cache. The matching entry will be dropped
 * from the cache unless it is the only offer in the cache. */
            case TM_DHCP_ATTR_CACHE_DROP:
                if (btEntryPtr->btDhcpState != TM_DHCPS_SELECTING)
                {
                    errorCode = TM_EPERM;
                }
                else
                {
                    if ((unsigned)attrLength == sizeof(tt4IpAddress))
                    {
                        tm_ip_copy(*(tt4IpAddressPtr)attrValuePtr, serverId);
                        if (tm_ip_not_zero(serverId))
                        {
                            if (btEntryPtr->btOfferEntryList.listCount > 1)
                            {
/* There is more than one offer in the cache, so we can try to remove the
 * specified offer. */
                                errorCode = tfDhcpDeleteOffers(
                                            btEntryPtr,
                                            serverId,
                                            TM_DHCP_OFFER_DELETE );
                            }
                            else if (   tm_ip_match(serverId,
                                                btEntryPtr->btDhcpServerId)
                                    &&  tm_ip_not_zero(btEntryPtr->btYiaddr) )
                            {
/* There is only one offer in the cache and it matches the specified
 * server id, so we can invalidate the entry by clearing the client IP
 * address field. */
                                tm_ip_copy(TM_IP_ZERO, btEntryPtr->btYiaddr);
                            }
                            else
                            {
/* No such offer exists in the cache */
                                errorCode = TM_ENOENT;
                            }
                        }
                    }
                }
                break;

/* Set the maximum number of DHCP offers to collect (min = 1). */
            case TM_DHCP_ATTR_COLLECT_CACHE:
/* It doesn't make sense to have more than one entry unless the user
 * callback option is used. */
                if (btEntryPtr->btDhcpState != TM_DHCPS_INIT)
                {
                    errorCode = TM_EPERM;
                }
                else
                {
                    intPtr = (ttIntPtr)attrValuePtr;
                    if ((unsigned)attrLength == sizeof(int) && *intPtr >= 1)
                    {
                        btEntryPtr->btCollectSize = (unsigned int)*intPtr;
                        errorCode = TM_ENOERROR;
                    }
                }
                break;

/* Set the maximum time in seconds to collect DHCP offers. */
            case TM_DHCP_ATTR_COLLECT_TIME:
                if (btEntryPtr->btDhcpState != TM_DHCPS_INIT)
                {
                    errorCode = TM_EPERM;
                }
                else
                {
                    intPtr = (ttIntPtr)attrValuePtr;
                    if ((unsigned)attrLength == sizeof(int) && *intPtr >= 0)
                    {
                        btEntryPtr->btCollectTime = (unsigned int)*intPtr;
                        errorCode = TM_ENOERROR;
                    }
                }
                break;

            default:
                ;
            }
        }
/* Unlock device */
        tm_call_unlock(&(devPtr->devLockEntry));
    }
    return errorCode;
}

/*
 * Control operational attributes of DHCP automatic configuration on
 * an interface.
 *
 * Parameter        Description
 * interfaceHandle  Interface handle as returned by tfAddInterface().
 * multiHomeIndex   Index of IPv4, DHCP configured address.
 * attrName         Name of attribute.
 * attrValuePtr     Pointer to user variable containing attribute value.
 * attrLength       Length of the attribute value in bytes.
 *
 * Returns:
 *  TM_ENOERROR     Success
 *  TM_EINVAL       Invalid parameter
 *  TM_EPERM        Operation not permitted in current state
 *  TM_ENOENT       Entry not found in cache
 */
int tfDhcpConfSetAttributes(    ttUserInterface interfaceHandle,
                                unsigned int    multiHomeIndex,
                                int             attrName,
                                void   TM_FAR * attrValuePtr,
                                int             attrLength )
{
    return tfDhcpSetAttributes( interfaceHandle,
                                multiHomeIndex,
                                attrName,
                                attrValuePtr,
                                attrLength,
                                TM_BT_CONF );
}

/*
 * Control operational attributes of DHCP user-controlled configuration on
 * an interface.
 *
 * Parameter        Description
 * interfaceHandle  Interface handle as returned by tfAddInterface().
 * multiHomeIndex   Index of IPv4, DHCP configured address.
 * attrName         Name of attribute.
 * attrValuePtr     Pointer to user variable containing attribute value.
 * attrLength       Length of the attribute value in bytes.
 *
 * Returns:
 *  TM_ENOERROR     Success
 *  TM_EINVAL       Invalid parameter
 *  TM_EPERM        Operation not permitted in current state
 *  TM_ENOENT       Entry not found in cache
 */
int tfDhcpUserSetAttributes(    ttUserInterface interfaceHandle,
                                unsigned int    multiHomeIndex,
                                int             attrName,
                                void   TM_FAR * attrValuePtr,
                                int             attrLength )
{
    return tfDhcpSetAttributes( interfaceHandle,
                                multiHomeIndex,
                                attrName,
                                attrValuePtr,
                                attrLength,
                                TM_BT_USER );
}

/*
 * Register user function that receives notification of DHCP offers.
 *
 * Parameter                Description
 * interfaceHandle          Interface handle as returned by tfAddInterface().
 * userDhcpOfferCBFuncPtr   Pointer to a user function to receive DHCP offer
 *                          notification.
 * userParam                User-defined parameter.
 *
 * Returns:
 *  TM_ENOERROR     Success
 *  TM_EINVAL       Invalid parameter
 */
int tfRegisterDhcpOfferCB(
    ttUserInterface             interfaceHandle,
    ttUserDhcpOfferCBFuncPtr    userDhcpOfferCBFuncPtr,
    ttUserGenericUnion          userParam )
{
    ttDeviceEntryPtr devPtr;
    int              errorCode;

    devPtr = (ttDeviceEntryPtr)interfaceHandle;
    errorCode = tfValidInterface(devPtr);
    if (errorCode == TM_ENOERROR)
    {
        tm_call_lock_wait(&(devPtr->devLockEntry));
        devPtr->devOfferFuncPtr = userDhcpOfferCBFuncPtr;
        devPtr->devOfferParam = userParam;
        tm_call_unlock(&(devPtr->devLockEntry));
    }
    return errorCode;
}
#endif /* TM_USE_DHCP_COLLECT */

#else /* !TM_USE_DHCP || !TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_DHCP or TM_USE_IPV4 is not defined */
int tv4DhcpuiDummy = 0;

#endif /* !TM_USE_DHCP || !TM_USE_IPV4 */
