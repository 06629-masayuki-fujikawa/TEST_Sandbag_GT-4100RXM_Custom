/*
 * $Source: include/trlance.h $
 *
 * This LANCE driver was taken from the GNU sources for LINUX
 * and modified for the Treck TCP/IP protocol stack
 *
 *   Written 1993 by Donald Becker.
 *
 *   Copyright 1993 United States Government as represented by the
 *   Director, National Security Agency.  This software may be used and
 *   distributed according to the terms of the GNU Public License,
 *   incorporated herein by reference.
 *
 * Driver Modified by Paul and Jared at Treck Inc. 1999
 * Modification History
 * Date: $Date: 2010/01/18 21:33:26JST $
 * Author: Jared
 * Version:  $Revision: 6.0.2.2 $
 * Release:  $ProjectRevision: 6.0.1.33 $
 * Description: Driver for LANCE ethernet chips
 *
 * Developed on an AMD Net186 board (AMD PCNet-ISA II Ethernet controller
 * and AM186ES-40VC processor) using Borland BCC 5.0 compiler
 */
#ifndef _TRLANCE_H_

#define _TRLANCE_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The size of the receive and send descriptor rings (respectively). The
 * valid sizes for these rings are 1, 2, 4, 8, 16, 32, 64, and 128. They
 * can be set to different sizes.
 */
#define TM_LANCE_RING_SIZE_RX    8
#define TM_LANCE_RING_SIZE_TX    8

/* Constants for the Ioctl function */
#define TM_LANCE_IOCTL_REFILL         0x01
#define TM_LANCE_IOCTL_SEND_COMPLETE  0x02

/* Forward Definitions */
int  tfLanceOpen(ttUserInterface lanceHandle);
int  tfLanceSend(ttUserInterface lanceHandle,
                 char TM_FAR *   dataPtr,
                 int             len,
                 int             flag);
int  tfLanceClose(ttUserInterface lanceHandle);
void tfLanceIsrHandler0(void);
void tfLanceIsrHandler1(void);
/* Add more LANCE ISR handler here */

int  tfLanceReceive(ttUserInterface        interfaceHandle,
                    char TM_FAR * TM_FAR * dataPtr,
                    int  TM_FAR *          dataLength,
                    ttUserBufferPtr        bufHandlePtr);
int  tfLanceIoctl(ttUserInterface interfaceHandle,
                  int             flag,
                  void TM_FAR *   optionPtr,
                  int             optionLen);
int  tfLanceGetPhysicalAddress(ttUserInterface lanceHandle,
                               char TM_FAR *   physicalAddress);

#ifdef __cplusplus
}
#endif

#endif /* _TRLANCE_H_ */
