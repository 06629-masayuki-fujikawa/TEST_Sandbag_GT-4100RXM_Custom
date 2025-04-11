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
 * Description: 8250 serial driver header file
 *
 * Filename: tr8250.h
 * Author: Jason
 * Date Created: 10/29/1999
 * $Source: include/tr8250.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:12JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TRLANCE_H_

#define _TRLANCE_H_

#ifdef __cplusplus
extern "C" {
#endif


/* Forward Definitions */
int  tf8250Open(ttUserInterface interfaceHandle);
int  tf8250Send(ttUserInterface interfaceHandle,
                char TM_FAR *   dataPtr,
                int             len,
                int             flag);
int  tf8250Close(ttUserInterface interfaceHandle);
void TM_INTERRUPT tf8250IsrHandler(void);
int  tf8250Receive(ttUserInterface         interfaceHandle,
                   char TM_FAR * TM_FAR *  dataPtr,
                   int  TM_FAR *           dataLength,
                   ttUserBufferPtr         bufHandlePtr);
int  tf8250Ioctl(ttUserInterface interfaceHandle,
                 int             flag,
                 void TM_FAR *   optionPtr,
                 int             optionLen);
int tf8250FreeReceiveBuffer(ttUserInterface interfaceHandle,
                            char TM_FAR *   dataPtr);


#ifdef __cplusplus
}
#endif

#endif /* _TRLANCE_H_ */
