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
 * Description: 16550 serial driver header file
 *
 * Filename: tr16550.h
 * Author: Jason
 * Date Created: 10/29/1999
 * $Source: include/tr16550.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:09JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TR16550_H_

#define _TR16550_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Forward Definitions */
int  tf16550Open(ttUserInterface interfaceHandle);
int  tf16550Send(ttUserInterface interfaceHandle,
                 char TM_FAR *   dataPtr,
                 int             len,
                 int             flag);
int  tf16550Close(ttUserInterface interfaceHandle);
void TM_INTERRUPT tf16550IsrHandler(void);
int  tf16550Receive(ttUserInterface         interfaceHandle,
                    char TM_FAR * TM_FAR *  dataPtr,
                    int  TM_FAR *           dataLength,
                    ttUserBufferPtr         bufHandlePtr);
int  tf16550Ioctl(ttUserInterface interfaceHandle,
                  int             flag,
                  void TM_FAR *   optionPtr,
                  int             optionLen);
int tf16550FreeReceiveBuffer(ttUserInterface interfaceHandle,
                             char TM_FAR *   dataPtr);


#ifdef __cplusplus
}
#endif

#endif /* _TR16550_H_ */
