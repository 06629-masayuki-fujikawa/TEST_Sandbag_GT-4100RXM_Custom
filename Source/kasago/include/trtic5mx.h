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
 * Description: Header file for device driver for TI's TMX320VC5510 DSP
 *  starter kit, used with LogicIO Ethernet daughtercard (Macronix MX98728EC
 *  GMAC). Adapted from example code provided by Texas Instruments.
 *
 * Filename: trtic5mx.h
 * Author: Ed Remmell, adapted from code written by Michael A. Denio of TI
 * Date Created:
 * $Source: include/trtic5mx.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:55JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TRTIC5MX_H_
#define _TRTIC5MX_H_

#ifdef __cplusplus
extern "C" {
#endif

int tfTiMxGetPhysAddr(
    ttUserInterface interfaceHandle,
    char TM_FAR *   physicalAddress);

int tfTiMxOpen(ttUserInterface interfaceHandle);

int tfTiMxClose(ttUserInterface interfaceHandle);

int tfTiMxIoctl(ttUserInterface interfaceHandle,
                int             flag,
                void TM_FAR *   optionPtr,
                int             optionLen);

int tfTiMxReceive(ttUserInterface        interfaceHandle,
                  char TM_FAR * TM_FAR * dataPtrPtr,
                  int  TM_FAR *          dataLengthPtr,
                  ttUserBufferPtr        bufHandlePtr);

int tfTiMxSend(ttUserInterface interfaceHandle,
               char TM_FAR *   dataPtr,
               int             dataLength,
               int             flag);

int tfTiMxScatSend(ttUserInterface interfaceHandle,
                   ttUserPacketPtr  uPacketPtr );

/* macro definitions */
/* MX98728 Macronix GMAC Register definitions, ETHC6000F Board */

/* Hardware Reset */
#define TM_GMAC_RESET_ON     0x408000
#define TM_GMAC_RESET_OFF    0x410000

/* GMAC_CONTROL [ 03 | 02 | 01 | 00 ]
      00    : NCRA    - Network Control Register A
      01    : NCRB    - Network Control Register B
      02    : TRA     - GMAC Test Register A
      03    : TRB     - GMAC Test Register B */
#define TM_GMAC_CONTROL      0x400000

/* GMAC_STATS   [ 07 | 06 | 05 | 04 ]
      04    : LTPS    - Last Transmitted Packet Status
      05    : LRPS    - Last Received Packet Status
      07/06 : MPCL    - Missed Packet Counter (16bit) */
#define TM_GMAC_STATS        0x400004

/* GMAC_ISR     [ 0B | 0A | 09 | 08 ]
      08    : IMR     - Interrupt Mask Register
      09    : IR      - Interrupt Register
      0B/0A : BP      - Boundary Page Pointer (16bit) */
#define TM_GMAC_ISR          0x400008

/* GMAC_TXRING1 [ 0F | 0E | 0D | 0C ]
      0D/0C : TLBP    - TX Low Boundary Page Pointer (16bit)
      0F/0E : TWP     - TX Write Page Pointer (16bit) */
#define TM_GMAC_TXRING1      0x40000c

/* GMAC_TXRING2 [ 13 | 12 | 11 | 10 ]
      11/10 : RES1    - Reserved (16bit)
      13/12 : TRP     - TX Read Page Pointer (16bit) */
#define TM_GMAC_TXRING2      0x400010

/* GMAC_RXRING1 [ 17 | 16 | 15 | 14 ]
      15/14 : RXINTT  - Receive Interrupt Timer (16bit)
      17/16 : RWP     - Receive Write Page Pointer (16bit) */
#define TM_GMAC_RXRING1      0x400014

/* GMAC_RXRING2 [ 1B | 1A | 19 | 18 ]
      19/18 : RRP     - Receive Read Page Pointer (16bit)
      1B/1A : RHBP    - RX High Boundary Page Pointer (16bit) */
#define TM_GMAC_RXRING2      0x400018

/* GMAC_IORING1 [ 1F | 1E | 1D | 1C ]
      1C    : EEIR    - EEPROM Interface Register
      1D    : BICT    - Bus Integrity Check Timer
      1F/1E : IORDP   - IO Data Port Page Pointer (16bit) */
#define TM_GMAC_IORING1      0x40001c

/* GMAC_ADDR1   [ 23 | 22 | 21 | 20 ]
      20    : PAR0    - Physical Address Byte 0
      21    : PAR1    - Physical Address Byte 1
      22    : PAR2    - Physical Address Byte 2
      23    : PAR3    - Physical Address Byte 3 */
#define TM_GMAC_ADDR1        0x400020

/* GMAC_ADDR2   [ 27 | 26 | 25 | 24 ]
      24    : PAR4    - Physical Address Byte 4
      25    : PAR5    - Physical Address Byte 5
      26    : MAR0    - Hash Table Byte 0
      27    : MAR1    - Hash Table Byte 1 */
#define TM_GMAC_ADDR2        0x400024

/* GMAC_ADDR3   [ 2B | 2A | 29 | 28 ]
      28    : MAR2    - Hash Table Byte 2
      29    : MAR3    - Hash Table Byte 3
      2A    : MAR4    - Hash Table Byte 4
      2B    : MAR5    - Hash Table Byte 5 */
#define TM_GMAC_ADDR3        0x400028

/* GMAC_ADDR4   [ 2F | 2E | 2D | 2C ]
      2C    : MAR6    - Hash Table Byte 6
      2D    : MAR7    - Hash Table Byte 7
      2E    : ANALOG  - Transceiver Control Register
      2F    : DINTVAL - DMA Interval Timer */
#define TM_GMAC_ADDR4        0x40002c

/* GMAC_CONFIG  [ 33 | 32 | 31 | 30 ]
      30    : NWAYC   - NWAY Configuration Register
      31    : NWAYS   - NWAY Status Register
      32    : GCA     - GMAC Configuration A Register
      33    : GCB     - GMAC Configuration B Register */
#define TM_GMAC_CONFIG       0x400030

/* GMAC_TWD     [ 37 | 36 | 35 | 34 ]
      37/34 : TWD     - Transmit Write Data Port (32bit) */
#define TM_GMAC_TWD          0x400034

/* GMAC_AUX1    [ 3B | 3A | 39 | 38 ]
      39/38 : RES2    - Reserved (16bit)
      3A    : HIPR    - Host Interface Protocol Register
      3B    : LPC     - Link Partner Code Register */
#define TM_GMAC_AUX1         0x400038

/* GMAC_AUX2    [ 3F | 3E | 3D | 3C ]
      3C    : DMASR   - DMA Status Register
      3D    : MISC1   - MISC Control Register 1
      3F/3E : TXFIFOCNT - TX FIFO Byte Count Register (16bit) */
#define TM_GMAC_AUX2         0x40003c

/* GMAC_RRD     [ 43 | 42 | 41 | 40 ]
      43/40 : RRD     - Read Data Port (32bit) */
#define TM_GMAC_RRD          0x400040    // Reg 40h/43h  RO

/* GMAC_ID      [ 47 | 46 | 45 | 44 ]
      45/44 : ID1     - ID Register ('M' << 8 | 'X')
      47/46 : ID2     - ID Register ("0001") */
#define TM_GMAC_ID           0x400044

/* GMAC_WRTXFIFOD [ 4B | 4A | 49 | 48 ]
      4B/48 : WRTXFIFOD - Write TX FIFO Data Port (32bit) */
#define TM_GMAC_WRTXFIFOD    0x400048    // Reg 48h/4Bh  WO

/* GMAC_IORD    [ 4F | 4E | 4D | 4C ]
      4F/4C : IORD    - IO Read Data Port (32bit) */
#define TM_GMAC_IORD         0x40004c    // Reg 4Ch/4Fh  RO

/* GMAC_AUX3    [ 53 | 52 | 51 | 50 ]
      50    : MISC2   - MISC Control Register 2
      51    : ???     - ???
      53/52 : HRPKTCNT - Host Receivce Packet Count Register (16bit) */
#define TM_GMAC_AUX3         0x400050

/* GMAC_FRAGCNT [ 57 | 56 | 55 | 54 ]
      56/54 : IORD    - Host DMA Fragment Counter (24bit)
      57    : ???     - ??? */
#define TM_GMAC_FRAGCNT      0x400054


/* GMAC Register Field Settings */

/* GMAC_CONTROL - NCRA/NCRB/TRA/TRB */
#define TM_GM_RESET        0x00000001  /* Reset (set, then clear) */
#define TM_GM_ST0          0x00000002  /* Transmit Command/Status */
#define TM_GM_ST1          0x00000004  /*     "" */
#define TM_GM_SR           0x00000008  /* Start Receive */
#define TM_GM_LB0          0x00000010  /* Loopback mode */
#define TM_GM_LB1          0x00000020  /*     "" */
#define TM_GM_INTMODE      0x00000040  /* Intmode (0=active low) */
#define TM_GM_INTCLK       0x00000080  /* Must be 0 */
#define TM_GM_PR           0x00000100  /* Promiscuous mode */
#define TM_GM_CA           0x00000200  /* "Pick-Off" mode when set */
#define TM_GM_PM           0x00000400  /* Pass all multicast */
#define TM_GM_PB           0x00000800  /* Pass bad frames */
#define TM_GM_AB           0x00001000  /* Accept broadcast */
#define TM_GM_HBD          0x00002000  /* Reserved (set to 0) */
#define TM_GM_RXINTC0      0x00004000  /* Rx packet INT threshhold */
#define TM_GM_RXINTC1      0x00008000  /*      "" */

/* GMAC_STATS - LTPS/LRPS/MPCL */
#define TM_GM_CC           0x0000000F  /* Collision count */
#define TM_GM_CRSLOST      0x00000010  /* Carrier Sense Lost */
#define TM_GM_UF           0x00000020  /* TX Fifo Underflow */
#define TM_GM_OWC          0x00000040  /* Out of window collision */
#define TM_GM_TERR         0x00000080  /* Transmit error */
#define TM_GM_BF           0x00000100  /* Rx packet buffer full */
#define TM_GM_CRC          0x00000200  /* Rx packet CRC error */
#define TM_GM_FAE          0x00000400  /* Frame alignment error */
#define TM_GM_FO           0x00000800  /* Fifo Overrun */
#define TM_GM_RW           0x00001000  /* Rx Watchdog */
#define TM_GM_MF           0x00002000  /* Multicast frame */
#define TM_GM_RF           0x00004000  /* Runt frame */
#define TM_GM_RERR         0x00008000  /* Receive Error */

/* GMAC_ISR - IMR/IR/BP */
#define TM_GM_FRAGIM       0x00000001  /* Fragment Counter Interrupt */
#define TM_GM_RIM          0x00000002  /* Rx Interrupt */
#define TM_GM_TIM          0x00000004  /* Tx Interrupt */
#define TM_GM_REIM         0x00000008  /* Rx Error Interrupt */
#define TM_GM_TEIM         0x00000010  /* Tx Error Interrupt */
#define TM_GM_FIFOEIM      0x00000020  /* Fifo Error Interrupt */
#define TM_GM_BUSEIM       0x00000040  /* Bus Error Interrupthhold */
#define TM_GM_RBFIM        0x00000080  /* Rx Buffer Full Interrupt */
#define TM_GM_FRAGI        0x00000100  /* Fragment Counter Interrupt */
#define TM_GM_RI           0x00000200  /* Rx Interrupt */
#define TM_GM_TI           0x00000400  /* Tx Interrupt */
#define TM_GM_REI          0x00000800  /* Rx Error Interrupt */
#define TM_GM_TEI          0x00001000  /* Tx Error Interrupt */
#define TM_GM_FIFOEI       0x00002000  /* Fifo Error Interrupt */
#define TM_GM_BUSEI        0x00004000  /* Bus Error Interrupthhold */
#define TM_GM_RBFI         0x00008000  /* Rx Buffer Full Interrupt */

/* GMAC_CONFIG - NWAYC/NWAYS/GCA/GCB */
#define TM_GM_FD           0x00000001  /* Full duplex mode */
#define TM_GM_PS100        0x00000002  /* Port select 100Mb */
#define TM_GM_ANE          0x00000004  /* Autonegotiate enable */
#define TM_GM_ANS0         0x00000008  /* Autonegotiate status */
#define TM_GM_ANS1         0x00000010  /*      " */
#define TM_GM_ANS2         0x00000020  /*      " */
#define TM_GM_NTTEST       0x00000040  /* Res. */
#define TM_GM_LTE          0x00000080  /* Link test enable (0=force link) */
#define TM_GM_LS10         0x00000100  /* Link is 10mbs */
#define TM_GM_LS100        0x00000200  /* Link is 100mbs */
#define TM_GM_LPNWAY       0x00000400  /* Link partner NWAY */
#define TM_GM_ANCLPT       0x00000800  /* Autonegotiation complete */
#define TM_GM_100TXF       0x00001000  /* NWAY 100 FD */
#define TM_GM_100TXH       0x00002000  /* NWAY 100 HD */
#define TM_GM_10TXF        0x00004000  /* NWAY 10  FD */
#define TM_GM_10TXH        0x00008000  /* NWAY 10  HD */
#define TM_GM_BPSCRM       0x00010000  /* Bypass srambler */
#define TM_GM_PBW          0x00020000  /* Use 16 bit transfer mode */
#define TM_GM_SLOWSRAM     0x00040000  /* User slower than 25nS SRAM */
#define TM_GM_ARXERRB      0x00080000  /* Accept RX packet with error */
#define TM_GM_MIISEL       0x00100000  /* Use external MII */
#define TM_GM_AUTOPUB      0x00200000  /* */
#define TM_GM_TXFIFOCNTEN  0x00400000  /* Enable the use of Tx FIFO cnt regs */
#define TM_GM_RES1         0x00800000  /* Reserved */
#define TM_GM_TTHD0        0x01000000  /* Transmit FIFO threshold */
#define TM_GM_TTHD1        0x02000000  /*      " */
#define TM_GM_RTHD0        0x04000000  /* Receive FIFO threshold */
#define TM_GM_RTHD1        0x08000000  /*      " */

/* GMAC_AUX1 - RES2/HIPR/LPC */
#define TM_GM_WRDYB        0x00010000  /* Write packet memory ready */
#define TM_GM_RRDYB        0x00020000  /* IO Read packet memory ready */
#define TM_GM_STIORD       0x00020000  /* IO Read packet memory ready */
#define TM_GM_DREQB        0x00040000  /* Rx packet data ready */

/* GMAC_AUX3  - MISC2/HRPKTCNT */
#define TM_GM_HBRLEN0      0x00000001  /* */
#define TM_GM_HBRLEN1      0x00000002  /* */
#define TM_GM_RUNTSIZE     0x00000004  /* Set to knock runts from 64 to 60 */
#define TM_GM_DREQBCTRL    0x00000008  /* DREQB timing */
#define TM_GM_RINTSEL      0x00000010  /* Set to IRQ on HOST dma, not local */
#define TM_GM_ITPSEL       0x00000020  /* */
#define TM_GM_A11A8EN      0x00000040  /* */
#define TM_GM_AUTORCVR     0x00000080  /* Set to enable autorecover on rx dma*/

/* EMIF CE2 addresses */
#define TM_EMIF_CE2_1       0x809
#define TM_EMIF_CE2_2       0x80A
#define TM_EMIF_CE2_3       0x80B

#ifdef __cplusplus
}
#endif

#endif /* #ifndef _TRTIC5MX_H_ */
