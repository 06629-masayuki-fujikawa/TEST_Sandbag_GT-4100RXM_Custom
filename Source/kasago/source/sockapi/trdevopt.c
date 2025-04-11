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
 * Description: Device Driver options: tfInterfaceSetOptions()
 *
 * Filename: trdevopt.c
 * Author: Odile
 * Date Created: 01/18/00
 * $Source: source/sockapi/trdevopt.c $
 *
 * Modification History
 * $Revision: 6.0.2.4 $
 * $Date: 2010/09/22 05:42:10JST $
 * $Author: pcarney $
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
#ifdef TM_LOCK_NEEDED
#include <trglobal.h>
#else /* !TM_LOCK_NEEDED */
#if (       defined(TM_6_USE_DHCP)          \
         && defined(TM_6_DHCP_USE_FQDN)     \
         && defined(TM_ERROR_CHECKING)      \
         && defined(TM_USE_ANSI_LINE_FILE)  \
         && (defined(TM_KERNEL_VISUAL_X86) || defined(TM_KERNEL_WIN32_X86) \
             || defined(TM_MEMORY_LOGGING)))
#include <trglobal.h>
#endif /* 6_DHCP_FQDN && TM_ERROR_CHECKING && TM_USE_ANSI_LINE_FILE && TM_MEMORY_LOGGING */
#endif /* !TM_LOCK_NEEDED */

/*
 * tfInterfaceSetOptions function description:
 * Called by the user to set some interface options:
 * TM_DEV_OPTIONS_RECV_COPY
 *    Make tfRecvInterface() copy received driver buffers whose length
 *    is smaller than the option threshold to a newly allocated smaller
 *    buffer. This option cannot be used
 *    on a point to point device, as a point to point link layer receive
 *    function copies the receive buffer into a new buffer, to unstuf
 *    the bytes.
 * or
 * TM_DEV_OPTIONS_XMIT_TASK
 *    Turn on/off a transmit task for flow control between the TCP/IP stack
 *    and the device driver send function. Can only be changed when device
 *    is not opened (i.e not configured).
 */
int tfInterfaceSetOptions ( ttUserInterface interfaceHandle,
                            int             optionName,
                            void   TM_FAR * optionValuePtr,
                            int             optionLength)
{
    ttDeviceEntryPtr   devEntryPtr;
    int                errorCode;
    int                optionLengthFlag;
    int                optionValue;
#ifdef TM_MOBILE_IP4
    ttMipOptionFuncPtr mipOptionFuncPtr;
#endif /* TM_MOBILE_IP4 */

    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
    optionValue = 0; /* Compiler warning */
/* Check on parameters */

    errorCode = tfValidInterface( devEntryPtr );
    if( errorCode == TM_ENOERROR )
    {
        errorCode = TM_EINVAL; /* assume bad parameter */
#ifdef TM_MOBILE_IP4
        if ((unsigned) optionName >= TM_MIP_OPTION_BASE)
        {
            if ( (devEntryPtr->devMobileIp4VectPtr != (ttMobileVectPtr) 0) &&
                 (devEntryPtr->devMobileIp4VectPtr->mipOptionFuncPtr != 
                  (ttMipOptionFuncPtr) 0) )
            {
                mipOptionFuncPtr = 
                    devEntryPtr->devMobileIp4VectPtr->mipOptionFuncPtr;
                errorCode = (*mipOptionFuncPtr)(interfaceHandle,
                                    optionName,
                                    optionValuePtr,
                                    optionLength,
                                    TM_MIP_SET_OPTION);
            }
            else
            {
                errorCode = TM_ENOPROTOOPT;
            }

            goto interfaceSetOptFinish;
        }
#endif /* TM_MOBILE_IP4 */

/* Lock device */
        tm_call_lock_wait(&(devEntryPtr->devLockEntry));
/* Check on length of option */
        if (optionValuePtr != (void TM_FAR *)0)
        {
/* Extract option length flag part of the optionName */
            optionLengthFlag = optionName & (   TM_DEV_OFLAG_SHORT 
                                              | TM_DEV_OFLAG_UCHAR
                                              | TM_DEV_OFLAG_INT
                                              | TM_DEV_OFLAG_STRING);
/* Get the optionValue based on the option length flag part of the optionName */
            switch (optionLengthFlag)
            {
                case TM_DEV_OFLAG_SHORT:
                    if (optionLength == (int)sizeof(short))
                    {
                        errorCode = TM_ENOERROR;
                        optionValue = *((short TM_FAR *)optionValuePtr);
                    }
                    break;
                case TM_DEV_OFLAG_UCHAR:
                    if  (optionLength == (int)sizeof(unsigned char))
                    {
                        optionValue = *((unsigned char TM_FAR *)optionValuePtr);
                        errorCode = TM_ENOERROR;
                    }
                    break;
                case TM_DEV_OFLAG_INT:
                    if  (optionLength == (int)sizeof(int))
                    {
                        optionValue = *((int TM_FAR *)optionValuePtr);
                        errorCode = TM_ENOERROR;
                    }
                    break;
                case TM_DEV_OFLAG_STRING:
                    errorCode = TM_ENOERROR;
                default:
                    break;
            }
        }

/* End of check on length of option */
/* Check on value of option */
        if (    (errorCode == TM_ENOERROR)
             && (optionName & TM_DEV_OFLAG_ABSOLUTE)
             && (optionValue < 0) )
        {
            errorCode = TM_EINVAL;
        }
/* End of check on value of option. */
/* Check if option is allowed on the device */
        if (    ( errorCode == TM_ENOERROR )
             && ( optionName & TM_DEV_OFLAG_NOT_BYTE_STUFF )
             && ( tm_ll_is_byte_stuffing(devEntryPtr) ) )
/* Disallow changing some options if device requires byte stuffing */
        {
            errorCode = TM_EPERM;
        }
        if (    ( errorCode == TM_ENOERROR )
             && ( optionName & TM_DEV_OFLAG_NOT_OPEN )
             && ( tm_16bit_one_bit_set( devEntryPtr->devFlag,
                                        TM_DEV_OPENED) ) )
        {
/* Disallow changing some options after device has been configured */
            errorCode = TM_EPERM;
        }
        if (errorCode == TM_ENOERROR)
        {
/*
 * Lock device driver lock. OK to keep the device lock, since we never lock
 * the device lock after locking the device driver lock
 */
            tm_call_lock_wait(&(devEntryPtr->devDriverLockEntry));
            switch (optionName)
            {
#ifdef TM_USE_IPV6
                case TM_6_DEV_OPTIONS_DAD_XMITS:
                    devEntryPtr->dev6DupAddrDetectTransmits
                        = (unsigned char) optionValue;
                    break;
                case TM_6_DEV_OPTIONS_NO_INIT_DELAY:
                    if (optionValue > 0)
                    {
                        devEntryPtr->dev6Flags |= TM_6_DEV_INIT_DELAY_COMPLETE;
                    }
                    else
                    {
                        tm_16bit_clr_bit( devEntryPtr->dev6Flags,
                                          TM_6_DEV_INIT_DELAY_COMPLETE );
                    }
                    break;
                case TM_6_DEV_OPTIONS_NO_DHCP_CONF:
                    if (optionValue > 0)
                    {
                        devEntryPtr->devFlag2 |= TM_6_DEVF2_NO_DHCP_CONF;
                    }
                    else
                    {
                        tm_16bit_clr_bit( devEntryPtr->devFlag2,
                                          TM_6_DEVF2_NO_DHCP_CONF );
                    }
                    break;
#ifdef TM_6_USE_DHCP
#ifdef TM_6_DHCP_USE_FQDN
/* Save a domain name for use in the FQDN option */
                case TM_6_DEV_OPTIONS_DHCP_FQDN_FULL:
                case TM_6_DEV_OPTIONS_DHCP_FQDN_PART:
/* If we already have a domain name saved, remove it */
                    if (devEntryPtr->dev6DhcpFqdnDomain)
                    {
                        tm_free_raw_buffer(devEntryPtr->dev6DhcpFqdnDomain);
                        devEntryPtr->dev6DhcpFqdnDomain = (tt8BitPtr)0;
                        devEntryPtr->dev6DhcpFqdnDomainLen = 0;
                    }
/* If the user wants to configure a new name */
                    if (optionLength > 0)
                    {
/* Get a buffer for the new label */
                        devEntryPtr->dev6DhcpFqdnDomain =
                                tm_get_raw_buffer(optionLength + 2);
                        if (devEntryPtr->dev6DhcpFqdnDomain)
                        {
/* Store the new label in the buffer */
                            optionLength = 
                                    tfDnsAsciiToLabel(
                                          (ttConstCharPtr)optionValuePtr,
                                          optionLength,
                                          devEntryPtr->dev6DhcpFqdnDomain,
                                          optionLength + 1);
                            if (optionLength > 0)
                            {
                                if (   optionName
                                    == TM_6_DEV_OPTIONS_DHCP_FQDN_FULL)
                                {
/* If this is a full FQDN, add the trailing 0 length */
                                    devEntryPtr->
                                        dev6DhcpFqdnDomain[optionLength] = 0;
                                    optionLength++;
                                }
                                devEntryPtr->dev6DhcpFqdnDomainLen =
                                        (tt16Bit)optionLength;
                            }
                            else
                            {
/* If there was an error, free the buffer we allocated */
                                tm_free_raw_buffer(
                                        devEntryPtr->dev6DhcpFqdnDomain);
                                devEntryPtr->dev6DhcpFqdnDomain =
                                        (tt8BitPtr)0;
                                errorCode = TM_EINVAL;
                            }
                        }
                        else
                        {
                            errorCode = TM_ENOMEM;
                        }
                    }
                    break;
/* Configure the S-bit for DHCPv6 FQDN */
                case TM_6_DEV_OPTIONS_DHCP_FQDN_S_BIT:
                    if (!optionValue)
                    {
/* Clear the FQDN S-bit */
                        tm_8bit_clr_bit( devEntryPtr->dev6DhcpFqdnFlags,
                                         TM_6_DHCP_FQDN_FLAG_S );
                    }
                    else
                    {
/* Set the FQDN S-bit */
                        devEntryPtr->dev6DhcpFqdnFlags |=
                            TM_6_DHCP_FQDN_FLAG_S;
                    }
                    break;
/* Configure the N-bit for DHCPv6 FQDN */
                case TM_6_DEV_OPTIONS_DHCP_FQDN_N_BIT:
                    if (!optionValue)
                    {
/* Clear the FQDN N-bit */
                        tm_8bit_clr_bit( devEntryPtr->dev6DhcpFqdnFlags,
                                         TM_6_DHCP_FQDN_FLAG_N );
                    }
                    else
                    {
/* Set the FQDN N-bit */
                        devEntryPtr->dev6DhcpFqdnFlags |=
                            TM_6_DHCP_FQDN_FLAG_N;
                    }
                    break;
#endif /* TM_6_DHCP_USE_FQDN */
#endif /* TM_6_USE_DHCP */
/*
 * Disable IPv6 site-local and global address auto configuration
 */
                case TM_6_DEV_OPTIONS_NO_AUTOCONFIG:
                    if (optionValue == 1)
                    {
                        devEntryPtr->devFlag2 |= TM_6_DEVF2_NO_AUTOCONFIG;
                    }
                    else
                    {
                        tm_16bit_clr_bit( devEntryPtr->devFlag2,
                                          TM_6_DEVF2_NO_AUTOCONFIG );
                    }
                    break;
#ifdef TM_6_USE_FILTERING
                case TM_6_DEV_OPTIONS_FILTER:
                    if (optionValue == 1)
                    {
                        devEntryPtr->devFlag2 |= (tt16Bit)TM_6_DEVF2_FILTERING;
                    }
                    else
                    {
                        tm_16bit_clr_bit( devEntryPtr->devFlag2,
                                          TM_6_DEVF2_FILTERING );
                    }
                    break;
#endif /* TM_6_USE_FILTERING */
                case TM_6_DEV_OPTIONS_NO_DEST_UNREACH:
                    if (optionValue == 0)
                    {
                        tm_16bit_clr_bit( devEntryPtr->devFlag2,
                                          TM_6_DEVF2_NO_DEST_UNREACH );
                    }
                    else
                    {
                        devEntryPtr->devFlag2
                                    |= (tt16Bit)TM_6_DEVF2_NO_DEST_UNREACH;
                    }
                    break;
#endif /* TM_USE_IPV6 */

/*
 * User gives a threshold value, so that we try and copy receive driver
 * buffers whose size are smaller than the given threshold option value
 * to a new buffer.
 */
                case TM_DEV_OPTIONS_RECV_COPY:
                    devEntryPtr->devRecvCopyThreshHold = (tt16Bit)optionValue;
                    break;

                case TM_DEV_OPTIONS_TX_ALIGN:
                    devEntryPtr->devTxAlign = (tt16Bit)optionValue;
                    break;

                case TM_DEV_OPTIONS_XMIT_TASK:
/*
 * Use a transmit task for flow control between the TCP/IP stack and
 * a device driver
 */
                    if (optionValue > 0)
                    {
                        if (tm_iface_xmit_queue_option(interfaceHandle))
/* Disallow transmit task, if a transmit buffer queue is used */
                        {
                            errorCode = TM_EPERM;
                        }
                        else
                        {
/* Allow transmit task */
                            devEntryPtr->devFlag |= TM_DEV_XMIT_TASK;
                        }
                    }
                    else
                    {
/* Reset transmit task usage */
                        tm_16bit_clr_bit( devEntryPtr->devFlag,
                                          TM_DEV_XMIT_TASK );
                        devEntryPtr->devXmitTaskPacketPtr = (ttPacketPtr)0;
                    }
                    break;

/* Disable gratuitous ARP's */
                case TM_DEV_OPTIONS_NO_GRAT_ARP:
                    if (optionValue > 0)
                    {
                        devEntryPtr->devFlag2 |= TM_DEVF2_NO_GRAT_ARP;
                    }
                    else
                    {
                        tm_16bit_clr_bit( devEntryPtr->devFlag2,
                                          TM_DEVF2_NO_GRAT_ARP );
                    }
                    break;
/*
 * Maximum number of BOOTP/DHCP request retries.
 */
                case TM_DEV_OPTIONS_BOOT_RETRIES:
                    devEntryPtr->devBtMaxRetries = (unsigned char)optionValue;     
                    break;

/*
 * Initial timeout value for BOOTP/DHCP requests.  
 */
                case TM_DEV_OPTIONS_BOOT_TIMEOUT:
                    devEntryPtr->devBtInitTimeoutIntv
                        = (unsigned char)optionValue;                        
                    break;

#ifdef TM_USE_DRV_SCAT_RECV
/* 
 * For driver scattered recv only. Set the minimum number of
 * bytes at the head of a packet that have to be contiguous. 
 * If a scattered recv buffer is received with a first link length below that 
 * minimum, that minimum number of bytes is copied into a new buffer.
 * (Default value is TM_DEV_DEF_RECV_CONT_HDR_LENGTH) .
 */
                case TM_DEV_OPTIONS_SCAT_RECV_LENGTH:
                    devEntryPtr->devRecvContiguousHdrLength = 
                                                        (tt16Bit)optionValue;
                    break;
#endif /* TM_USE_DRV_SCAT_RECV */
/*
 * DHCP/BOOTP option: Number of ARP probes to send prior to configuring
 * the address (0 means default, -1 means no ARP probes should be sent)
 */
                case TM_DEV_OPTIONS_BOOT_ARP_RETRIES: 
                    devEntryPtr->devBtArpProbes = (int)optionValue;
                    break;
/*
 * DHCP/BOOTP option: Interval in seconds between ARP probes to be sent priot
 * to configuring the address (0 means default)
 */
                case TM_DEV_OPTIONS_BOOT_ARP_INTVL: 
                    devEntryPtr->devBtArpIntv = (tt8Bit)optionValue;
                    break;
/* 
 * DHCP/BOOTP option: number of seconds to wait after sending the
 * first ARP probe/Arp Request before finishing configuring
 * a DHCP/BOOTP address.
 * 0 means use default value.
 */
                case TM_DEV_OPTIONS_BOOT_ARP_TIMEOUT: 
                    devEntryPtr->devBtArpTimeout = (tt8Bit)optionValue;
                    break;
#ifdef TM_USE_DHCP
/* DHCP option: Do not send a RELEASE message when closing DHCP */
                case TM_DEV_OPTIONS_NO_DHCP_RELEASE:
                    if (optionValue > 0)
                    {
                        devEntryPtr->devFlag2 |= TM_DEVF2_NO_DHCP_RELEASE;
                    }
                    else
                    {
                        tm_16bit_clr_bit( devEntryPtr->devFlag2,
                                          TM_DEVF2_NO_DHCP_RELEASE );
                    }
                    break;
/* 
 * DHCP option: 
 * Pick which host name to send to the server in the
 * host name option and/or FQDN option.
 * 0 means pick user set values (default)
 * 1 means pick server set values
 */
                case TM_DEV_OPTIONS_BOOT_PK_HOST_NM:
                    if (optionValue == 1)
                    {
                        devEntryPtr->devFlag2 |= TM_DEVF2_SRVR_HOST_NAME;
                    }
                    else
                    {
                        tm_16bit_clr_bit( devEntryPtr->devFlag2,
                                          TM_DEVF2_SRVR_HOST_NAME );
                    }
                    break;
/* 
 * DHCP option: 
 * Pick which domain name to send to the server in the FQDN option
 * 0 means pick user set values (default)
 * 1 means pick server set values
 */
                case TM_DEV_OPTIONS_BOOT_PK_DOMAIN_NM:
                    if (optionValue == 1)
                    {
                        devEntryPtr->devFlag2 |= TM_DEVF2_SRVR_DOMAIN_NAME;
                    }
                    else
                    {
                        tm_16bit_clr_bit( devEntryPtr->devFlag2,
                                          TM_DEVF2_SRVR_DOMAIN_NAME );
                    }
                    break;
#endif /* TM_USE_DHCP */
#ifdef TM_USE_FILTERING
                case TM_DEV_OPTIONS_FILTER:
                    if (optionValue == 1)
                    {
                        devEntryPtr->devFlag2 |= (tt16Bit)TM_DEVF2_FILTERING;
                    }
                    else
                    {
                        tm_16bit_clr_bit( devEntryPtr->devFlag2,
                                          TM_DEVF2_FILTERING );
                    }
                    break;
#endif /* TM_USE_FILTERING */
#ifdef TM_USE_IGMP_FILTERING
                case TM_DEV_OPTIONS_FILTER_IGMP:
                    if (optionValue == 1)
                    {
                        devEntryPtr->devFlag2 |=
                                        (tt16Bit)TM_DEVF2_IGMP_FILTERING;
                    }
                    else
                    {
                        tm_16bit_clr_bit( devEntryPtr->devFlag2,
                                          TM_DEVF2_IGMP_FILTERING );
                    }
                    break;
#endif /* TM_USE_IGMP_FILTERING */
                case TM_DEV_OPTIONS_DISABLE_ICMP_RTR_ADVERT:
                    if (optionValue == 0)
                    {
                        tm_16bit_clr_bit( devEntryPtr->devFlag2,
                                          TM_DEVF2_DISABLE_ICMP_RTR_ADVERT);
                    }
                    else
                    {
                        devEntryPtr->devFlag2 |=
                             (tt16Bit)TM_DEVF2_DISABLE_ICMP_RTR_ADVERT;
                    }
                    break;
                case TM_DEV_OPTIONS_FORWARDING:
                    if (optionValue == 0)
                    {
                        tm_16bit_clr_bit( devEntryPtr->devFlag,
                                          TM_DEV_IP_FORW_ENB );
                    }
                    else
                    {
                        devEntryPtr->devFlag |= (tt16Bit)TM_DEV_IP_FORW_ENB;
                    }
                    break;
                case TM_DEV_OPTIONS_FORWARD_REFLECT:
                    if (optionValue == 0)
                    {
                        tm_16bit_clr_bit( devEntryPtr->devFlag2,
                                          TM_DEVF2_FORWARD_REFLECT );
                    }
                    else
                    {
                        devEntryPtr->devFlag2
                                    |= (tt16Bit)TM_DEVF2_FORWARD_REFLECT;
                    }
                    break;
                case TM_DEV_OPTIONS_IP_PROMISCUOUS:
                    if (optionValue == 0)
                    {
                        tm_16bit_clr_bit( devEntryPtr->devFlag,
                                          TM_DEV_IP_NO_CHECK );
                    }
                    else
                    {
                        devEntryPtr->devFlag |= (tt16Bit)TM_DEV_IP_NO_CHECK;
                    }
                    break;
#ifdef TM_USE_IGMPV3
                case TM_DEV_OPTIONS_NO_IGMPV2_RA:
                    if (optionValue == 0)
                    {
                        tm_16bit_clr_bit( devEntryPtr->devFlag2,
                                          TM_DEVF2_NO_IGMPV2_RA );
                    }
                    else
                    {
/* Do not check that an IGMPv2 query carry the router alert option */
                        devEntryPtr->devFlag2 |=
                                             (tt16Bit)TM_DEVF2_NO_IGMPV2_RA;
                    }
                    break;
#endif /* TM_USE_IGMPV3 */
#ifdef TM_USE_DHCP_COLLECT
                case TM_DEV_OPTIONS_DHCP_COLLECT_CACHE:
                    if (optionValue < 1)
                    {
                        errorCode = TM_EINVAL;
                    }
                    else
                    {
                        devEntryPtr->devDhcpCollectSize
                                                = (unsigned int)optionValue;
                    }
                    break;
                case TM_DEV_OPTIONS_DHCP_COLLECT_TIME:
                    devEntryPtr->devDhcpCollectTime
                                                = (unsigned int)optionValue;
                    break;
#endif /* TM_USE_DHCP_COLLECT */
                default:
                    break;
            }
/* UnLock device driver lock. */
            tm_call_unlock(&(devEntryPtr->devDriverLockEntry));
        }
/* UnLock device lock. */
        tm_call_unlock(&(devEntryPtr->devLockEntry));
    }

#ifdef TM_MOBILE_IP4

interfaceSetOptFinish:

#endif /* TM_MOBILE_IP4 */

    return errorCode;
}

/*
 * tfInterfaceGetOptions function description:
 * Called by the user to get the value of various interface options
 */
int tfInterfaceGetOptions ( ttUserInterface interfaceHandle,
                            int             optionName,
                            void   TM_FAR * optionValuePtr,
                            int             optionLength)
{
    ttDeviceEntryPtr   devEntryPtr;
    int                errorCode;
    int                optionLengthFlag;

    devEntryPtr = (ttDeviceEntryPtr)interfaceHandle;
/* Check on parameters */
    errorCode = tfValidInterface(devEntryPtr);
    if (errorCode  == TM_ENOERROR )
    {
        errorCode = TM_EINVAL; /* assume bad parameter */
/* Lock device */
        tm_call_lock_wait(&(devEntryPtr->devLockEntry));
/* Check on length of option */
        if (optionValuePtr != (void TM_FAR *)0)
        {
/* Extract option length flag part of the optionName */
            optionLengthFlag = optionName & (   TM_DEV_OFLAG_SHORT 
                                              | TM_DEV_OFLAG_UCHAR
                                              | TM_DEV_OFLAG_INT );
/* Get the optionValue based on the option length flag part of the optionName */
            switch (optionLengthFlag)
            {
                case TM_DEV_OFLAG_SHORT:
                    if (optionLength == (int)sizeof(short))
                    {
                        errorCode = TM_ENOERROR;
                    }
                    break;
                case TM_DEV_OFLAG_UCHAR:
                    if  (optionLength == (int)sizeof(unsigned char))
                    {
                        errorCode = TM_ENOERROR;
                    }
                    break;
                case TM_DEV_OFLAG_INT:
                    if  (optionLength == (int)sizeof(int))
                    {
                        errorCode = TM_ENOERROR;
                    }
                    break;
                default:
                    break;
            }
        }

/* End of check on length of option */
        if (errorCode == TM_ENOERROR)
        {
            switch (optionName)
            {
                case TM_DEV_OPTIONS_FORWARDING:
                    if (tm_4_dev_forward(devEntryPtr))
                    {
                        *(unsigned char TM_FAR *)optionValuePtr =
                                                            (unsigned char)1;
                    }
                    else
                    {
                        *(unsigned char TM_FAR *)optionValuePtr =
                                                            (unsigned char)0;
                    }
                    break;
                case TM_DEV_OPTIONS_FORWARD_REFLECT:
                    if (tm_dev_forward_reflect(devEntryPtr))
                    {
                        *(unsigned char TM_FAR *)optionValuePtr =
                                                            (unsigned char)1;
                    }
                    else
                    {
                        *(unsigned char TM_FAR *)optionValuePtr =
                                                            (unsigned char)0;
                    }
                    break;
                case TM_DEV_OPTIONS_IP_PROMISCUOUS:
                    if (tm_dev_no_check(devEntryPtr))
                    {
                        *(unsigned char TM_FAR *)optionValuePtr =
                                                            (unsigned char)1;
                    }
                    else
                    {
                        *(unsigned char TM_FAR *)optionValuePtr =
                                                            (unsigned char)0;
                    }
                    break;
#ifdef TM_USE_IGMPV3
                case TM_DEV_OPTIONS_NO_IGMPV2_RA:
                    if (tm_16bit_one_bit_set(devEntryPtr->devFlag2,
                                             TM_DEVF2_NO_IGMPV2_RA))
                    {
                        *(unsigned char TM_FAR *)optionValuePtr = (unsigned char)1;
                    }
                    else
                    {
/* Do not check that an IGMPv2 query carry the router alert option */
                        *(unsigned char TM_FAR *)optionValuePtr = (unsigned char)0;
                    }
                    break;
#endif /* TM_USE_IGMPV3 */
#ifdef TM_USE_IPV6
                case TM_6_DEV_OPTIONS_NO_DEST_UNREACH:
                    if (tm_6_dev_no_dest_unreach(devEntryPtr))
                    {
                        *(unsigned char TM_FAR *)optionValuePtr =
                                                            (unsigned char)1;
                    }
                    else
                    {
                        *(unsigned char TM_FAR *)optionValuePtr =
                                                            (unsigned char)0;
                    }
                    break;
#endif /* TM_USE_IPV6 */
                default:
                    break;
            }
        }
/* UnLock device lock. */
        tm_call_unlock(&(devEntryPtr->devLockEntry));
    }

    return errorCode;
}
