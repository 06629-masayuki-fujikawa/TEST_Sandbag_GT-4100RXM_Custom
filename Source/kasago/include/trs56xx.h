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
 * Description:
 * Ethernet driver configuration parameters for Stretch S56XX
 *
 * Filename: tr56xx.h
 * Author: Jared August
 * Date Created: 8/26/2005
 * $Source: include/trs56xx.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:44JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#include "../source/kernel/none/s56xx/main.h"

/* How many buffers in the RX and TX driver rings, per Ethernet port */
#define TM_56XX_RX_RING_SIZE 12
#define TM_56XX_TX_RING_SIZE 8

/* Interrupt aggregation. This only matters when you don't define
 * TM_56XX_POLLED_MODE in main.h. AGG_COUNT is the number of interrupts to
 * wait before actually firing an interrupt. AGG_TIMEOUT is how long (in ms)
 * to wait after receiving a packet to fire an interrupt, in case AGG_COUNT
 * isn't reached in a reasonable amount of time */
#define TM_56XX_RX_ISR_AGG_COUNT    1
#define TM_56XX_RX_ISR_AGG_TIMEOUT  1
#define TM_56XX_TX_ISR_AGG_COUNT    1
#define TM_56XX_TX_ISR_AGG_TIMEOUT  1

/* This structure contains information for each driver buffer saved in the
 * buffer ring */
typedef struct
{
    ttUserBuffer  bufHandle;
    unsigned char *dataPtr;
    sx_uint32     bufLength;
} tt56xxBufRingInfo;

/* The information needed to control a single Ethernet port */
typedef struct
{
    unsigned char     phyAddr[6];
    sx_ppi            *ppi_handle;
    tt56xxBufRingInfo rxRingInfo[TM_56XX_RX_RING_SIZE];
    unsigned int      dirtyRxIndex;
    ttUserInterface   interfaceHandle;
} tt56xxPortInfo, * tt56xxPortInfoPtr;

/* Function definitions for use in tfAddInterface() */
int tf56xxOpen(ttUserInterface interfaceHandle);
int tf56xxClose(ttUserInterface interfaceHandle);
int tf56xxSend(ttUserInterface interfaceHandle,
			   char TM_FAR *   dataPtr,
			   int             dataLength,
			   int             flag);
int tf56xxRecv(ttUserInterface        interfaceHandle,
			   char TM_FAR * TM_FAR * dataPtr,
			   int TM_FAR *           dataLength,
			   ttUserBufferPtr        bufHandlePtr);
int tf56xxIoctl(ttUserInterface interfaceHandle,
				int             flag,
				void TM_FAR *   optionPtr,
				int             optionLen);
int tf56xxGetPhyAddr(ttUserInterface interfaceHandle,
					 char TM_FAR *   physicalAddress);
