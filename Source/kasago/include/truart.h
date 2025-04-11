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
 * Description: UART serial driver header file
 *
 * Filename: tr16550.h
 * Author: Bryan
 * Date Created: 6/10/2003
 * $Source: include/truart.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:58JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TRUART_H_

#define _TRUART_H_

#ifdef __cplusplus
extern "C" {
#endif

int tfUartOpen (
    ttUserInterface interfaceHandle);

int tfUartClose (
    ttUserInterface interfaceHandle);

int tfUartIoctl (
    ttUserInterface interfaceHandle,
    int             flag,
    void TM_FAR *   optionPtr,
    int             optionLen);

int tfUartSend( 
    ttUserInterface interfaceHandle,
    char TM_FAR *   dataPtr,
    int             dataLength,
    int             flag);

int tfUartReceive ( 
    ttUserInterface         interfaceHandle,
    char TM_FAR * TM_FAR * dataPtrPtr,
    int  TM_FAR *          dataLength,
    ttUserBufferPtr        bufHandlePtr);

void tfUartIsrHandler(void);

int tfUartFreeReceiveBuffer(
    ttUserInterface interfaceHandle,
    char TM_FAR *   dataPtr);

int tfUartGetPhysicalAddress(
    ttUserInterface interfaceHandle,
    char TM_FAR *   physicalAddress);


#ifdef __cplusplus
}
#endif

#endif /* _TRUART_H_ */
