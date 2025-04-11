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
 * Description: SMC9000 Ethernet driver header file
 *
 * Filename: trsmc.h
 * Author: Jason
 * Date Created: 2/16/2000
 * $Source: include/trsmc.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:46JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TRSMC_H_

#define _TRSMC_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initialize and start the SMC device, setup interrupts, install ISR
 * handler and initialize data structures.
 */
int  tfSmcOpen(ttUserInterface interfaceHandle);

/* 
 * Attempt to send a packet to the SMC.  If there is no on-chip memory available
 * and the Treck transmit queue is enabled, this routine will return immediately
 * and queue the packet.  Otherwise this routine will spin until memory is
 * available.
 */
int  tfSmcSend(ttUserInterface interfaceHandle,
               char TM_FAR *   dataPtr,
               int             len,
               int             flag);

#ifdef TM_USE_DRV_ONE_SCAT_SEND
/* tfScmScatSend Scatter send function.*/
int tfSmcScatSend(ttUserInterface interfaceHandle,
                  ttUserPacketPtr  uPacketPtr );
#endif /* TM_USE_DRV_ONE_SCAT_SEND*/

/* Free any used memory and shut down the device. */
int  tfSmcClose(ttUserInterface interfaceHandle);

/* ISR to handle received packets and transmit errors */
void TM_INTERRUPT tfSmcIsrHandler(void);

/* Pass a packet which was recieved in the ISR to the Treck stack. */
int  tfSmcReceive(ttUserInterface         interfaceHandle,
                  char TM_FAR * TM_FAR *  dataPtr,
                  int  TM_FAR *           dataLength,
                  ttUserBufferPtr         bufHandlePtr);

/* Adds and removes entries in the SMC's multicast table */
int  tfSmcIoctl(ttUserInterface interfaceHandle,
                int             flag,
                void TM_FAR *   optionPtr,
                int             optionLen);

/* Returns the physical address associated with this device */
int tfSmcGetPhysAddr(ttUserInterface interfaceHandle,
                     char TM_FAR *   physicalAddress);

#ifdef __cplusplus
}
#endif

#endif /* _TRSMC_H_ */
