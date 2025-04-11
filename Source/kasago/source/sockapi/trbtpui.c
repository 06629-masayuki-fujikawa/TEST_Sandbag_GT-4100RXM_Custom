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
 * Description: BOOTP User Interface
 *
 * Filename: trbtpui.c
 * Author: Odile
 * Date Created: 12/06/06
 * $Source: source/sockapi/trbtpui.c $
 *
 * Modification History
 * $Revision: 6.0.2.4 $
 * $Date: 2010/06/17 07:25:09JST $
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

static int tfBootpCommonSet(ttUserInterface interfaceHandle,
                            int             index,
                            int             flags,
                            unsigned char   TM_FAR * dataPtr,
                            int             dataLen,
                            int             type);

/*
 * tfBootpUserStart function description:
 * Start Sending a BOOTP request on the interface corresponding to
 * interface Handle. The index corresponds to a user BOOTP request.
 * It has to be between 0 and tvMaxUserDhcpEntries - 1. (tvMaxUserDhcpEntries
 * default value is 0, and can be changed with a tfInitSetTreckOptions, with
 * option name TM_OPTION_DHCP_MAX_ENTRIES).
 * bootpNotifyFunc is a user supplied call back function which will be called
 * when either the BOOTP request has been successful, or has timed out
 *
 * Parameters
 * Parameter         Description
 * interfaceHandle   Ethernet interface handle
 * userIndex         User Index (between 0, and tvMaxUserDhcpEntries - 1)
 * bootpNotifyFuncPtr Pointer to a function that will be called when
 *                   the BOOTP request has completed, or timed out.
 * Returns
 * Value            Meaning
 * TM_ENOERROR      success. The BOOTP notify function will not be called.
 * TM_EINPROGRESS   BOOTP request sent with no error.
 * TM_EALREADY      BOOTP request previously sent.
 * TM_EINVAL        Bad parameter
 * TM_ENOBUFS       Not enough memory
 * other            as returned by device driver send function.
 */
int tfBootpUserStart ( ttUserInterface        interfaceHandle,
                      int                     userIndex,
                      ttUserDhcpNotifyFuncPtr bootpNotifyFuncPtr )
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
/* Check that index is within range */
        if (    (userIndex < 0)
             || (userIndex >= tm_context(tvMaxUserDhcpEntries)) )
        {
            errorCode = TM_EINVAL;
        }
        else
        {
            if (     devEntryPtr->devBootInfo[TM_BOOT_BOOTP].bootRecvFuncPtr
                 == (ttDevBootRecvFuncPtr)0 )
            {
/* If tfUseBootp() has not been called */
                devEntryPtr->devBootInfo[TM_BOOT_BOOTP].bootRecvFuncPtr
                                             = tfBootpRecvCB;
            }
            errorCode = tfBtInit( devEntryPtr,
                                  bootpNotifyFuncPtr,
                                  userIndex,
                                  TM_BT_USER,
                                  TM_BOOT_BOOTP );
/* Call tfBootpStart */
/* Start the state machine */
            if (errorCode == TM_ENOERROR)
            {
/* So that tfIpIncomingPacket() let BOOTP packets through */
                devEntryPtr->devFlag |= TM_DEV_IP_BOOTP;
                errorCode = tfBootpStart(devEntryPtr,
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
 * tfBootpUserStop function description:
 * Stop a BOOTP request.
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
int tfBootpUserStop (ttUserInterface    interfaceHandle,
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
/*
 * Call tfBootpStop to free any timer, free server options, reset BOOT user
 * entry, etc..
 */
            errorCode = tfBootpStop(devEntryPtr,
                                    userIndex,
                                    TM_BT_USER );
        }
/* Unlock the device */
        tm_call_unlock(&(devEntryPtr->devLockEntry));
    }
    return errorCode;
}

/*
 * Set custom BOOTP configuration options prior to the user calling
 * tfBootpUserStart(). This function is to set options that are not natively
 * supported by the BOOTP client. For use with the BOOTP User Controlled
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
int tfBootpUserSetOption(
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
                         TM_BOOT_BOOTP);
}

/*
 * Retrieve BOOTP options sent by the server that aren't natively supported
 * by the BOOTP client. Any options unrecognized by the client are stored in
 * a special list that is accessible through this function. For use with the
 * BOOTP User Controlled Configuration API.
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
int tfBootpUserGetOption(
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
                         TM_BOOT_BOOTP);
}

/*
 * Set BOOTP configuration options prior to the user calling
 * tfOpenInterface/tfConfigInterface().
 * 
 * Used by the user to set/reset the host name option when flags is set 
 * to TM_BOOTPF_HOST_NAME. In that case if the dataPtr is null, and 
 * dataLength is zero the host name option is reset, otherwise it will
 * be set using the name pointed to by dataPtr.
 * The HOST Name option must be set separately from other BOOTP options.
 *
 * Check that mhomeIndex is within a valid range.
 *
 * Parameter         Description
 * interfaceHandle    Ethernet interface handle
 * mHomeIndex         Multi-home index to set this option on
 * flags              Valid Values:
 *                    TM_BOOTP_HOST_NAME
 * dataPtr            Data needed to set specified option
 * dataLen            Length of data pointed to by dataPtr
 *
 * Returns
 * Value            Meaning
 * TM_ENOERROR      success
 * TM_EINVAL        Bad parameter
 * TM_EPERM         Call can only be called prior to calling
 *                  tfOpenInterface/tfConfigInterface
 * TM_ENOBUFS       Not enough memory
 */
TM_PROTO_EXTERN int tfBootpConfSet (ttUserInterface interfaceHandle,
                                    int             mHomeIndex,
                                    int             flags,
                                    unsigned char   TM_FAR * dataPtr,
                                    int             dataLen)
{
    int errorCode;

    errorCode = tfBootpCommonSet( interfaceHandle,
                                 mHomeIndex,
                                 flags,
                                 dataPtr,
                                 dataLen,
                                 TM_BT_CONF );
    return errorCode;
}

/*
 * Set BOOTP configuration options prior to the user calling
 * tfBootpUserStart().
 * 
 * Used by the user to set/reset the host name option when flags is set 
 * to TM_BOOTPF_HOST_NAME. In that case if the dataPtr is null, and 
 * dataLength is zero the host name option is reset, otherwise it will
 * be set using the name pointed to by dataPtr.
 * The HOST Name option must be set separately from other BOOTP options.
 *
 * Check that mhomeIndex is within a valid range.
 *
 * Parameter         Description
 * interfaceHandle    Ethernet interface handle
 * mHomeIndex         Multi-home index to set this option on
 * flags              Valid Values:
 *                    TM_BOOTP_HOST_NAME
 * dataPtr            Data needed to set specified option
 * dataLen            Length of data pointed to by dataPtr
 *
 * Returns
 * Value            Meaning
 * TM_ENOERROR      success
 * TM_EINVAL        Bad parameter
 * TM_EPERM         Call can only be called prior to calling
 *                  tfOpenInterface/tfConfigInterface
 * TM_ENOBUFS       Not enough memory
 */
TM_PROTO_EXTERN int tfBootpUserSet (ttUserInterface interfaceHandle,
                                    int             index,
                                    int             flags,
                                    unsigned char   TM_FAR * dataPtr,
                                    int             dataLen)
{
    int errorCode;

    errorCode = tfBootpCommonSet(interfaceHandle,
                                 index,
                                 flags,
                                 dataPtr,
                                 dataLen,
                                 TM_BT_USER );
    return errorCode;
}

/*
 * Common routine callted by tfBootpUserSet() or tfBootpConfSet().
 * 
 * Used by the user to set/reset the host name option when flags is set 
 * to TM_BOOTPF_HOST_NAME. In that case if the dataPtr is null, and 
 * dataLength is zero the host name option is reset, otherwise it will
 * be set using the name pointed to by dataPtr.
 * The HOST Name option must be set separately from other BOOTP options.
 *
 * Check that mhomeIndex is within a valid range.
 *
 * Parameter         Description
 * interfaceHandle    Ethernet interface handle
 * mHomeIndex         Multi-home index to set this option on
 * flags              Valid Values:
 *                    TM_BOOTP_HOST_NAME
 * dataPtr            Data needed to set specified option
 * dataLen            Length of data pointed to by dataPtr
 *
 * Returns
 * Value            Meaning
 * TM_ENOERROR      success
 * TM_EINVAL        Bad parameter
 * TM_EPERM         Call can only be called prior to calling
 *                  tfOpenInterface/tfConfigInterface
 * TM_ENOBUFS       Not enough memory
 */
static int tfBootpCommonSet (ttUserInterface interfaceHandle,
                             int             index,
                             int             flags,
                             unsigned char   TM_FAR * dataPtr,
                             int             dataLen,
                             int             type)
{
    ttDeviceEntryPtr devEntryPtr;
    ttBtEntryPtr     btEntryPtr;
    int              errorCode;

/* Flags field should not have bits that are not defined set */
    if ( (flags & ~(TM_BOOTPF_FLAGS)) != 0)
    {
        errorCode = TM_EINVAL;
    }
    else
    {
        errorCode = TM_ENOERROR;
/* Host name option can only be set by itself */
        if (flags & TM_BOOTPF_HOST_NAME)
        {
            if (    (flags != TM_BOOTPF_HOST_NAME)
                 || (    (dataPtr == (unsigned char TM_FAR *)0) 
                      && (dataLen != 0) )
                 || (    (dataPtr != (unsigned char TM_FAR *)0)
                      && (    (dataLen < 1)
                           || (dataLen > TM_DHCP_HOSTNAME_SIZE) ) )
               )
            {
                errorCode = TM_EINVAL;
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
/* Get/Allocate btEntry corresponding to index, indexType on the device */
            btEntryPtr = tfBtAlloc(devEntryPtr, index, (tt8Bit)type,
                                   TM_BOOT_BOOTP);
            if (btEntryPtr == (ttBtEntryPtr)0)
            {
                errorCode = TM_ENOBUFS;
            }
            else
            {
                if (btEntryPtr->btDhcpState != TM_DHCPS_INIT) 
                {
                    errorCode = TM_EPERM;
                }
            }
            if (errorCode == TM_ENOERROR)
            {
                if (flags == TM_BOOTPF_HOST_NAME)
                {
                    if (dataPtr != (unsigned char TM_FAR *)0)
                    {
/* Save user host name option */
                        tm_bcopy( dataPtr, 
                                  btEntryPtr->btUserSetDhcpHostNamePtr,
                                  dataLen );
                    }
                    btEntryPtr->btUserSetDhcpHostNameLength = 
                                                  (tt8Bit)dataLen;
                }
            }
/* Unlock the device */
            tm_call_unlock(&(devEntryPtr->devLockEntry));
        }
    }
    return errorCode;
}

/*
 * Set custom BOOTP configuration options prior to the user calling
 * tfOpenInterface/tfConfigInterface(). This function is to set options
 * that are not natively supported by the BOOTP client.
 * 
 * Parameter          Description
 * interfaceHandle    Ethernet interface handle
 * index              Multi-home index to set this option on
 * bootOption         BootP option to set
 * dataPtr            Data to be sent in option
 * dataLen            Length of data pointed to by dataPtr
 *
 * Returns
 * See tfBtSetOption()
 */
int tfBootpConfSetOption(
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
                         TM_BOOT_BOOTP);
}

/*
 * Retrieve BOOTP options sent by the server that aren't natively supported
 * by the BOOTP client. Any options unrecognized by the client are stored in
 * a special list that is accessible through this funciton
 * 
 * Parameter         Description
 * interfaceHandle    Ethernet interface handle
 * index              Multi-home index to get this option from
 * bootOption         BootP option to retrieve
 * dataPtr            Buffer to store option data into
 * dataLenPtr         Pointer to the length of the data buffer. Must be set
 *                    by the user to the maximum number of bytes the buffer
 *                    can handle. It will be reset to the actual length of
 *                    data stored in the buffer.
 *
 * Returns
 * See tfBtGetOption()
 */
int tfBootpConfGetOption(
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
                         TM_BOOT_BOOTP);
}

#else /* !TM_USE_IPV4 */

#ifdef TM_LINT
LINT_UNUSED_HEADER
#endif /* TM_LINT */

/* To allow link for builds when TM_USE_IPV4 is not defined */
int tv4BtpuiDummy = 0;
#endif /* !TM_USE_IPV4 */
