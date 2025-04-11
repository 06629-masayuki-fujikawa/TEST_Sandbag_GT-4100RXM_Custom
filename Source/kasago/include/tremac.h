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
 * Description: Header file for ZiLOG eZ80 F91 EMAC Ethernet driver
 *
 * Filename: tremac.h
 * Author: Qin Zhang
 * Date Created: 01/29/2003
 * $Source: include/tremac.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:22JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TREMAC_H_

#define _TREMAC_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TM_KERNEL_NONE_EZ80
/* Define keyword interrupt */
#define TM_INTERRUPT    interrupt
#else TM_KERNEL_NONE_EZ80
#define TM_INTERRUPT
#endif /* TM_KERNEL_NONE_EZ80 */

/* Constants for the Ioctl function */
#define TM_EMAC_IOCTL_REFILL         0x01
#define TM_EMAC_IOCTL_SEND_COMPLETE  0x02

/* Forward Definitions */
int  tfEmacOpen(ttUserInterface  emacHandle);
int  tfEmacSend(ttUserInterface  emacHandle,
                 char TM_FAR *   dataPtr,
                 int             len,
                 int             flag);
int  tfEmacClose(ttUserInterface emacHandle);

void TM_INTERRUPT tfEmacIsrHandler(void);

int  tfEmacReceive(ttUserInterface         emacHandle,
                    char TM_FAR * TM_FAR * dataPtr,
                    int  TM_FAR *          dataLength,
                    ttUserBufferPtr        bufHandlePtr);
int  tfEmacIoctl(ttUserInterface  emacHandle,
                  int             flag,
                  void TM_FAR *   optionPtr,
                  int             optionLen);
int  tfEmacGetPhysicalAddress(ttUserInterface  emacHandle,
                               char TM_FAR *   physicalAddress);

int tfEmacFreeRecvBuf( ttUserInterface  emacHandle,
                       char           * bufPtr );

#ifdef __cplusplus
}
#endif

#endif /* _TRLANCE_H_ */
