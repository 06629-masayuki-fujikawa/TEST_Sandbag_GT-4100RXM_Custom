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
 * Description: header file for Intel 80314 Gigabit Ethernet Device Driver
 * Filename: tr80314.h
 * Author: Jin Zhang and Ed Remmell
 * Date Created:
 * $Source: include/tr80314.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:11JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TR80314_H_
#define _TR80314_H_

#define TEST_TIMER

#define TM_80314_IOCTL_REFILL        0x0001 /* not used */
#define TM_80314_IOCTL_SEND_COMPLETE 0x0002

#define _Read_CPSR \
({ REGISTER unsigned _val_; \
   asm volatile("mrs\t%0, cpsr" : "=r" (_val_)); \
   _val_; \
})
#define _IRQ_DISABLE (1 << 7)
#define _FIQ_DISABLE (1 << 6)

int tf80314Initialize(ttUserInterface interfaceHandle,
                     int             portId);

int tf80314Open(ttUserInterface interfaceHandle);
int tf80314Close(ttUserInterface interfaceHandle);
int tf80314Ioctl(ttUserInterface interfaceHandle,
                 int             flag,
                 void TM_FAR *   optionPtr,
                 int             optionLen);
int tf80314Send(ttUserInterface interfaceHandle,
                char TM_FAR *   dataPtr,
                int             dataLength,
                int             flag);
int tf80314ScatSend(ttUserInterface interfaceHandle,
                    ttUserPacketPtr  uPacketPtr );
int tf80314Receive(ttUserInterface        interfaceHandle,
                   char TM_FAR * TM_FAR * dataPtr,
                   int  TM_FAR *          dataLength,
                   ttUserBufferPtr        bufHandlePtr);
int tf80314GetPhysAddr(ttUserInterface interfaceHandle,
                       char TM_FAR *   physicalAddress);

void tf80314Nanosleep(unsigned nanoseconds);
#endif /*_TR80314_H_*/

