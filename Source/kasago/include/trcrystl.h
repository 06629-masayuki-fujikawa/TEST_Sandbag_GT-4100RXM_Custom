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
 * Description: Crystal LAN (CS8900) driver header file
 *
 * Filename: trcrystl.h
 * Author: Jason
 * Date Created: 11/29/1999
 * $Source: include/trcrystl.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:20JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TRCRYSTAL_H_

#define _TRCRYSTAL_H_

#ifdef __cplusplus
extern "C" {
#endif

/* IOCTL function type flags */
#define TM_CRYSTAL_IOCTL_REFILL         TM_DEV_IOCTL_REFILL_POOL_FLAG

/* Driver API prototypes */
int  tfCrystalOpen(ttUserInterface interfaceHandle);
int  tfCrystalSend(ttUserInterface interfaceHandle,
                 char TM_FAR *   dataPtr,
                 int             len,
                 int             flag);
int  tfCrystalClose(ttUserInterface interfaceHandle);

#ifdef __BORLANDC__
void TM_INTERRUPT tfCrystalIsrHandler(void);
#else
void tfCrystalIsrHandler(void);
#endif

/* NOTE: Can use tfPoolRecvBuffer instead in tfAddInterface */
int  tfCrystalReceive(ttUserInterface         interfaceHandle,
                    char TM_FAR * TM_FAR *  dataPtr,
                    int  TM_FAR *           dataLength,
                    ttUserBufferPtr         bufHandlePtr);
int  tfCrystalIoctl(ttUserInterface interfaceHandle,
                  int             flag,
                  void TM_FAR *   optionPtr,
                  int             optionLen);
int tfCrystalGetPhysicalAddress(ttUserInterface interfaceHandle,
                                char TM_FAR *   physicalAddress);


#ifdef __cplusplus
}
#endif

#endif /* _TRCRYSTAL_H_ */
