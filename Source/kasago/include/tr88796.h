/*
 * Description: Header info for driver for the Asix AX88796B Ethernet chip
 *
 * Filename: tr88796.h
 * Author: Lorne
 * Date Created: 2008/10/27
 * $Source: include/tr88796.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:16JST $
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

#ifndef _tr88796_h
#define _tr88796_h

/*
 * Structure Definitions
 */

/*
 * Device statistics.
 */
typedef struct ts88796Stats
{
	/* Interrupt events */
	unsigned long	intevent;
	/* Receive interrupts */
	unsigned long	rxint;
	/* Transmit interrupts */
	unsigned long	txint;
	/* Receive errors */
	unsigned long	rxerror;
	/* Transmit errors */
	unsigned long	txerror;
	/* Receive overruns */
	unsigned long	overrun;
	/* Packets Sent */
	unsigned long	transmit;
	/* Packets Received Intact */
	unsigned long	rxgood;
	/* CRC Errors */
	unsigned long	rxcrc;
	/* Frame Alignment Errors */
	unsigned long	rxalign;
	/* MII Errors */
	unsigned long	rxmii;
	/* Runt Packets */
	unsigned long	rxrunt;
	/* Multicast/Broadcast Address */
	unsigned long	rxmcbc;
	/* Missed Packets */
	unsigned long	rxmissed;
	/* Receiver disabled events */
	unsigned long	rxdisabled;
	/* Transmit collisions */
	unsigned long	txcollision;
	/* Transmit aborts */
	unsigned long	txabort;
	/* Transmit out-of-window collisions */
	unsigned long	txwindow;
	/* Transmit send timeout */
	unsigned long	txtimeout;
	/* Device not ready */
	unsigned long	dev_not_ready;
	/* Physical link not ready */
	unsigned long	phy_not_ready;
	/* Transmit queue overflow */
	unsigned long	txqovf;
	/* Receive queue overflow */
	unsigned long	rxqovf;
	/* No receive buffers available */
	unsigned long	rxnobuf;
	/* Mininum free transmit buffer space */
	unsigned int	txminfree;
} tt88796Stats;
typedef tt88796Stats TM_FAR * tt88796StatsPtr;

/*
 * tf88796Open()
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
int tf88796Open(ttUserInterface ttHandle);

/*
 * tf88796Close()
 *
 *   Removes the interrupt from the PIC and restore the old interrupt
 *   table. We could also shut down the chip, but it really doesn't seem
 *   necessary.
 *   Parameters:
 *       ttHandle   The user interface handle for this device.
 *   Returns:
 *       TM_DEV_OKAY
 */
int tf88796Close(ttUserInterface ttHandle);

/*
 * tf88796Receive()
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
int tf88796Receive(ttUserInterface       ttHandle,
                  char TM_FAR * TM_FAR * dataPtr,
                  int  TM_FAR *          dataLength,
                  ttUserBufferPtr        bufHandlePtr);

/*
 * tf88796Send()
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
int tf88796Send(ttUserInterface ttHandle,
               char TM_FAR *   dataPtr,
               int             dataLength,
               int             flag);


/* 
 * tf88796Ioctl()
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
int tf88796Ioctl(ttUserInterface ttHandle,
                int             flag,
                void TM_FAR *   optionPtr,
                int             optionLen);


/*
 * tf88796GetPhyAddr()
 *
 *   This function retrieves the adapter's physical address.
 *
 *   Parameters:
 *       ttHandle        The handle to the user interface for this device
 *       physicalAddress A buffer into which to store the physical address
 *   Returns:
 *       TM_DEV_OKAY
 */
int tf88796GetPhyAddr(ttUserInterface ttHandle,
                     char TM_FAR *   physicalAddress);

#endif /* _tr88796_h */
