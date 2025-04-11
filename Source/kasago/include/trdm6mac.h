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
 * Description: Texas Instruments TMS320DM641 EMAC device driver header file
 *
 * Filename: trdm6mac.h
 * Author: Jason
 * Date Created: 01/20/2004
 * $Source: include/trdm6mac.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:21JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TRDM6MAC_H_

#define _TRDM6MAC_H_

#ifdef __cplusplus
extern "C" {
#endif

/* IOCTL function type flags */
/* cleanup EMAC transmit frame descriptors and call tfSendCompleteInterface
   for packets that have completed transmit */
#define TM_EMAC_IOCTL_REFILL_TX         0x0001
/* refill empty EMAC receive frame descriptors with Treck packet buffers */
#define TM_EMAC_IOCTL_REFILL_RX         0x0002
/* check for and set full duplex or half duplex EMAC operation */
#define TM_EMAC_IOCTL_SET_DUPLEX        0x0004
/* IOCTL for backwards compatibility with previous version of this driver */
#define TM_EMAC_IOCTL_REFILL            (TM_EMAC_IOCTL_REFILL_TX | \
    TM_EMAC_IOCTL_REFILL_RX | TM_EMAC_IOCTL_SET_DUPLEX)

/* Driver API prototypes */
int tfDm642EmacOpen(ttUserInterface interfaceHandle);
int tfDm642EmacClose(ttUserInterface interfaceHandle);
int tfDm642EmacIoctl ( ttUserInterface interfaceHandle,
                       int             flag,
                       void TM_FAR *   optionPtr,
                       int             optionLen );
int tfDm642EmacSend ( ttUserInterface interfaceHandle,
                      char TM_FAR *   dataPtr,
                      int             dataLength,
                      int             flag );
#ifdef TM_USE_DRV_ONE_SCAT_SEND
int tfDm642EmacScatSend(
    ttUserInterface interfaceHandle,
    ttUserPacketPtr  uPacketPtr );
#endif /* TM_USE_DRV_ONE_SCAT_SEND */
int tfDm642EmacRecv(ttUserInterface        interfaceHandle,
                    char TM_FAR * TM_FAR * dataPtrPtr,
                    int  TM_FAR *          dataLengthPtr,
                    ttUserBufferPtr        bufHandlePtr);
int tfDm642EmacGetPhysicalAddress(ttUserInterface interfaceHandle,
                                  char TM_FAR *   physicalAddress);


#ifdef __cplusplus
}
#endif

#endif /* _TRDM6MAC_H_ */
