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
 * Description: Header info for driver for NE2000 and DP8390 Ethernet chips
 *
 * Filename: trne2000.h
 * Author: Jared
 * Date Created: 12/09/1999
 * $Source: include/trne2000.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:32JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */
/*
 * This file is a heavily modified, combined version of ne.c and 8390.c.
 * Written 1992-94 by Donald Becker.
 *
 * Copyright 1993 United States Government as represented by the
 * Director, National Security Agency.
 *
 * This software may be used and distributed according to the terms
 * of the GNU Public License, incorporated herein by reference.
 */

#ifndef _trne2k_h
#define _trne2k_h

/*
 * tfNE2kOpen()
 *
 *   Configure and start up the Ethernet interface.
 *   We take the address from the Ethernet board and set up the board.
 *   Then we store the interrupt address and enable the interrupt.
 *   Parameters:
 *       ttHandle   The user interface handle for this device.
 *   Returns:
 *       error:     TM_DEV_ERROR
 *       success:   TM_DEV_OKAY
 */
int tfNE2kOpen(ttUserInterface ttHandle);

/*
 * tfNE2kClose()
 *
 *   Removes the interrupt from the PIC and restore the old interrupt
 *   table. We could also shut down the chip, but it really doesn't seem
 *   necessary.
 *   Parameters:
 *       ttHandle   The user interface handle for this device.
 *   Returns:
 *       TM_DEV_OKAY
 */
int tfNE2kClose(ttUserInterface ttHandle);

/*
 * tfNE2kReceive()
 *
 *   This function receives a packet. It is called after we've received an
 *   interrupt indicating there is data to be handled.
 *
 *   Parameters:
 *       ttHandle        The handle to the user interface for this device
 *       dataPtr         We fill in this pointer with a pointer to the data
 *                       we have just received
 *       dataLength      We fill in this length with the length of the data
 *                       we have just received
 *       ttUserBufferPtr We fill out this buffer handle with the buffer handle
 *                       we get back when we allocate memory
 *   Returns:
 *       success:        TM_DEV_OKAY
 *       error:          TM_DEV_ERROR
 */
int tfNE2kReceive(ttUserInterface        ttHandle,
                  char TM_FAR * TM_FAR * dataPtr,
                  int  TM_FAR *          dataLength,
                  ttUserBufferPtr        bufHandlePtr);

/*
 * tfNE2kSend()
 *
 *   This function stores a packet in the NE2000's buffers for sending. If the
 *   chip is not currently busy sending anything, it also triggers a send. If
 *   the chip is busy, the packet send will be triggered in the ISR after all
 *   of the preceding packets in the NE2000's buffers have been sent.
 *
 *   Parameters:
 *       ttHandle   The handle to the user interface for this device
 *       dataPtr    The pointer to the data we're sending
 *       dataLength The length of the data we're sending
 *       flag       A flag indicating various things about the data to send
 *   Returns:
 *       success:        TM_DEV_OKAY
 *       error:          TM_DEV_ERROR
 */
int tfNE2kSend(ttUserInterface ttHandle,
               char TM_FAR *   dataPtr,
               int             dataLength,
               int             flag);


/* 
 * tfNE2kIoctl()
 *
 *   This function, in general, allows the user to talk directly to the
 *   chip, for any reason. We have implemented it here to allow the
 *   user to set multicast addresses. It should be noted that at the time
 *   this function was implemented, IGMP was not fully tested, therefore
 *   making it impossible to test this function. Use with caution!
 *
 *   TM_DEV_SET_MCAST_LIST:
 *     Adds a number of multicast entries (==optionLen) to the multicast hash 
 *     table.  This is done by 
 *         1. Compute a 32-bit CRC on the address.
 *         2. Extract the most significant 6 bits of the computed CRC.
 *         3. Set the bit coresponding to the above number in the multicast
 *            address filter (ie, decode the 6 bit value above into a 64 bit
 *            value: 50 -> 0x0002 0000 0000 0000) 
 *
 *   TM_DEV_SET_ALL_MCAST:
 *     Sets the chip to receive all multicast packets (ie, set the address
 *     filter to all ones).
 *
 *   Parameters:
 *       ttHandle   The handle to the user interface for this device
 *       flag       A flag what we want this function to do
 *       dataPtr    The pointer to the data being passed in
 *       dataLength The length of the data (in this case, how many multicast
 *                  addresses are being added).
 *   Returns:
 *       TM_DEV_OKAY
 */
int tfNE2kIoctl(ttUserInterface ttHandle,
                int             flag,
                void TM_FAR *   optionPtr,
                int             optionLen);


/*
 * tfNE2kGetPhyAddr()
 *
 *   This function retrieves the adapter's physical address.
 *
 *   Parameters:
 *       ttHandle        The handle to the user interface for this device
 *       physicalAddress A buffer into which to store the physical address
 *   Returns:
 *       TM_DEV_OKAY
 */
int tfNE2kGetPhyAddr(ttUserInterface ttHandle,
                     char TM_FAR *   physicalAddress);

#endif /* _trne2k_h */
