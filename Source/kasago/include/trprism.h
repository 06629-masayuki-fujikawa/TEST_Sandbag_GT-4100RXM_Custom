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
 * Description: PRISM 802.11b device driver 
 *
 * Filename: trprism.h
 * Author: Jason Schmidlapp
 * Date Created: 4/9/2003
 * $Source: include/trprism.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:38JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TRPRISM_H_
#define _TRPRISM_H_

#ifdef __cplusplus
extern "C" {
#endif

    
/* constant declarations and enumerations */

/* type definitions */

/* global function prototypes */

/*
 * Initializes the PRISM 802.11b controller.  Get the relevant PCI
 * information, intialize the chip and enable interrupts.
 */ 
int tfPrismOpen(ttUserInterface interfaceHandle);

/* Returns the MAC address of this 802.11b interface. */
int tfPrismGetPhysAddr(ttUserInterface interfaceHandle,
                       char TM_FAR *   physicalAddress);

/* Submits an Ethernet frame to the PRISM controller for transmission. */
int tfPrismSend(ttUserInterface interfaceHandle,
                char TM_FAR *   dataPtr,
                int             dataLength,
                int             flag);

/* Pass a packet that was received from the PRISM controller in the ISR up to
   the stack. */
int tfPrismReceive(ttUserInterface        interfaceHandle,
                   char TM_FAR * TM_FAR * dataPtrPtr,
                   int  TM_FAR *          dataLengthPtr,
                   ttUserBufferPtr        bufHandlePtr);

/* Close the PRISM 802.11b interface. */
int tfPrismClose(ttUserInterface interfaceHandle);

/*
 * Controls various options on the PRISM controller as well as issuing certain
 * 802.11b related commands.
 */ 
int tfPrismIoctl(ttUserInterface interfaceHandle,
                 int             flag,
                 void TM_FAR *   optionPtr,
                 int             optionLen);

/* Processes PRISM interrupts, including the received frame interrupt. */
void tfPrismHandlerIsr(void);
    
/* global variable extern's */

/* macro definitions */
#define TM_PRISM_IOCTL_GET_PORT_STATUS  0x0001
#define TM_PRISM_IOCTL_GET_SSID         0x0002
#define TM_PRISM_IOCTL_GET_BSSID        0x0003
#define TM_PRISM_IOCTL_GET_COMM_QUALITY 0x0004
#define TM_PRISM_IOCTL_GET_TX_RATE      0x0005
#define TM_PRISM_IOCTL_SET_PORT_TYPE    0x0006
#define TM_PRISM_IOCTL_SET_IBSS_CHAN    0x0007
#define TM_PRISM_IOCTL_SET_SSID         0x0008
#define TM_PRISM_IOCTL_SET_OWN_SSID     0x0009
#define TM_PRISM_IOCTL_SET_WEP_KEY      0x000a
#define TM_PRISM_IOCTL_SET_WEP_ENABLE   0x000b

    
/* Infrastructure mode - use an access point. */
#define TM_PRISM_INFRASTRUCTURE_MODE 0
/* Ad-hoc mode - don't use access point. */
#define TM_PRISM_AD_HOC_MODE         1
/* Auto-port mode - use an AP if present, otherwise use ad-hoc mode. */
#define TM_PRISM_AUTO_PORT_MODE      2

/*
 * Port status 
 */
#define TM_PRISM_PORT_DISABLED      1  /* Port is disabled.                 */
#define TM_PRISM_PORT_SEARCHING     2  /* searching for initial connection  */
#define TM_PRISM_PORT_CONN_IBSS     3  /* ad-hoc mode: connected to IBSS    */
#define TM_PRISM_PORT_CONN_ESS      4  /* infrastr. mode: connected to BSS  */
#define TM_PRISM_PORT_OUT_OF_RANGE  5  /* Out of range, in ESS              */
#define TM_PRISM_PORT_CONN_WDS      6  /* shouldn't occur (not implemented) */
#define TM_PRISM_PORT_HOST_AP       8  /* should never occur, since this
                                          is a station only implementation. */
    
/*
 * Link status
 */
#define TM_PRISM_CONN              1
#define TM_PRISM_DISCONN           2
#define TM_PRISM_AP_CHANGE         3
#define TM_PRISM_AP_OUT_OF_RANGE   4
#define TM_PRISM_AP_IN_RANGE       5
#define TM_PRISM_ASSC_FAIL         6
    
#ifdef __cplusplus
}
#endif

#endif /* #ifndef _TRPRISM_H_ */
