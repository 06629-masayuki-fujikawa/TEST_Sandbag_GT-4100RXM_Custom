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
 * Description: Treck PCI interface and abstractions
 * Filename: trpci.h
 * Author: Jason
 * Date Created: 3/14/2000
 * $Source: include/trpci.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:35JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

/* 'Standard' PCI ports for the typical PC */
#define TM_PCI_CONF_ADDR_PORT   0x0CF8
#define TM_PCI_CONF_DATA_PORT   0x0CFC


/*
 * PCI configuration registers are accessed through 2 32-bit 'ports', an address
 * port and a data port.  On a Intel x86 platform, these ports are usually 
 * access through the normal inpx commands.  However, on a PPC, for example,
 * these 'ports' are access through memory mapped registers.  The abstraaction
 * below simply needs the processor to write/read the specified value to/from
 * the specified port.  The examples shown below are for an x86.  For another
 * platform the tm_out_dword might look like:
 *      #define tm_out_dword(tp_value,tp_port) *(tp_port) = tp_value
 *
 * This should be the only section of this file that requires modification.
 */

#define tm_pci_out_word(tp_value,tp_port)    _outpw(tp_port,tp_value)

/* Write out a byte to specified port address */
#define tm_pci_out_byte(tp_value,tp_port)    _outp(tp_port,tp_value)

/* Write out a dword to specified port address */
#define tm_pci_out_dword(tp_value,tp_port)   _outpd(tp_port, tp_value)                  

/* Read in a word from specified port address */
#define tm_pci_in_word(tp_port)  _inpw(tp_port)

/* Read in a byte from specified port address */
#define tm_pci_in_byte(tp_port)  _inp(tp_port)

/* Read in a dword from specified port address */
#define tm_pci_in_dword(tp_port) _inpd(tp_port)



/* Writes a byte (8-bit) to a PCI configuration block */
#define tm_pci_write_conf_byte(bus,dev,func,dword,data) \
        tm_pci_out_dword(0x80000000UL | (bus << 16) | (dev << 11) | \
            (func << 8) | (dword << 2), TM_PCI_CONF_ADDR_PORT); \
        tm_pci_out_byte(data,TM_PCI_CONF_DATA_PORT);                             

/* Writes a word (16-bit) to a PCI configuration block */
#define tm_pci_write_conf_word(bus,dev,func,dword,data) \
        tm_pci_out_dword(0x80000000UL | (bus << 16) | (dev << 11) | \
            (func << 8) | (dword << 2), TM_PCI_CONF_ADDR_PORT); \
        tm_pci_out_word(data,TM_PCI_CONF_DATA_PORT);                               

/* Writes a dword (32-bit) to a PCI configuration block */
#define tm_pci_write_conf_dword(bus,dev,func,dword,data) \
        tm_pci_out_dword(0x80000000UL | (bus << 16) | (dev << 11) | \
            (func << 8) | (dword << 2), TM_PCI_CONF_ADDR_PORT); \
        tm_pci_out_dword(data,TM_PCI_CONF_DATA_PORT);                          

/* Reads a byte (8-bit) from a PCI configuration block */
#define tm_pci_read_conf_byte(bus,dev,func,dword,data) \
        tm_pci_out_dword(0x80000000UL | (bus << 16) | (dev << 11) | \
            (func << 8) | (dword << 2), TM_PCI_CONF_ADDR_PORT); \
        data = (unsigned char) tm_pci_in_byte(TM_PCI_CONF_DATA_PORT);              

/* Reads a word (16-bit) from a PCI configuration block */
#define tm_pci_read_conf_word(bus,dev,func,dword,data) \
        tm_pci_out_dword(0x80000000UL | (bus << 16) | (dev << 11) | \
            (func << 8) | (dword << 2), TM_PCI_CONF_ADDR_PORT); \
        data = (unsigned short) tm_pci_in_word(TM_PCI_CONF_DATA_PORT);             

/* Reads a dword (32-bit) from a PCI configuration block */
#define tm_pci_read_conf_dword(bus,dev,func,dword,data) \
        tm_pci_out_dword(0x80000000UL | (bus << 16) | (dev << 11) | \
            (func << 8) | (dword << 2), TM_PCI_CONF_ADDR_PORT); \
        data = (unsigned long) tm_pci_in_dword(TM_PCI_CONF_DATA_PORT)

/* 
 * Offsets into PCI Configuration structure 
 */

/* Device ID and vendor ID */
#define TM_PCI_VENDDEV_ID     0

/* Status and command registers */
#define TM_PCI_STAT_CMD_REG   1

/* Class Code and revision */
#define TM_PCI_CLASS_REV      2

/* BIST, Header Type, Latency Timer, Cache Line Size */
#define TM_PCI_HEAD_LAT_CACHE 3

/* Base Address 0 */
#define TM_PCI_BASE_ADDR_0    4
#define TM_PCI_BASE_ADDR_1    5

/* CardBus CIS Pointer */
#define TM_PCI_CARDBUS_CIS    10

/* Subsystem ID and subsystem vendor ID */
#define TM_PCI_SUBSYS_ID      11

/* Expansion ROM Base Address */
#define TM_PCI_ROM_BASE_ADDR  12

/* Reserved & Capabilities Pointer */
#define TM_PCI_CAPAB_PTR      13

/* Reserved */
#define TM_PCI_RESERVE        14

/* Max_Lat, Min_Gnt, Interrupt Pin, Interrupt Line */
#define TM_PCI_INT_LINE       15
#define TM_PCI_INT_PIN        15
#define TM_PCI_MIN_GNT        15
#define TM_PCI_MAX_LAT        15


/* Retrieves the specified PCI device's bus, device and function ID's */
int tfPciFindDevice(unsigned short  deviceId,
                    unsigned short  vendorId,
                    unsigned char * bus,
                    unsigned char * device,
                    unsigned char * func);
