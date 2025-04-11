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
 * $Source: include/trmpc860.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:31JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */
/* 
 * HEADER FILE FOR Motorola MPC860 QUICC Sample Driver
 *
 * This device driver is derived from the Ethernet Bridge example from
 * Motorola.
 * It has been modified to interface to the Treck TCP/IP stack and to
 * provide readability.
 *
 * Since it is a derived work, it remains the property of Motorola Corp.
 *
 * We have left the definintions of the quicc registers as they
 * are in the QUICC user manual
 *
 * This is why they do not match TRECK coding standards.
 * Modification History
 * Date: $Date: 2010/01/18 21:33:31JST $
 * Author:
 * Version:  $Revision: 6.0.2.2 $
 * Release:  $ProjectRevision: 6.0.1.33 $
 * Description:
 */

#ifndef _TRMPC860_H_

#define _TRMPC860_H_

#ifdef __cplusplus
extern "C" {
#endif

 /* 24.576 MHz */
#define TM_MPC860_CLOCK_RATE 40000000
#define TM_MPC860_BAUD_RATE 19200


/*
 * Mpc860 Buffer Descriptors
 */
typedef struct tsMpc860BufDesc
{
    unsigned short bdStatus;
    unsigned short bdLength;
    void TM_FAR *  bdDataPtr;
}ttMpc860BufDesc;

typedef ttMpc860BufDesc TM_FAR * ttMpc860BufDescPtr;

/* Number of buffer descriptors */
#define TM_MPC860_ETHER_MAX_RECV_BUFD    (unsigned short)25      
#define TM_MPC860_ETHER_MAX_XMIT_BUFD    (unsigned short)50

#define TM_MPC860_SERIAL_MAX_RECV_BUFD    (unsigned short)5      
#define TM_MPC860_SERIAL_MAX_XMIT_BUFD    (unsigned short)5

#define TM_MPC860_FEC_MAX_RECV_BUFD    (unsigned short)25      
#define TM_MPC860_FEC_MAX_XMIT_BUFD    (unsigned short)50

/*
 * IOCTL Defines
 */
#define TM_MPC860_REFILL_SCC1   (int)0x0001
#define TM_MPC860_SEND_COMPLETE (int)0x0002
#define TM_MPC860_REFILL_SCC4   (int)0x0004
#define TM_MPC860_REFILL_FEC    (int)0x0008


/*
 * Function Prototypes
 */

/*
 * The Actual SCC1 ISR Wrapper Function
 * it must call the tfMpc860Scc1HandlerIsr function
 * This routine may be in "C" if it is supported
 * or assembly if not.  It is VERY RTOS dependent
 * so it cannot be provided with the driver
 *
 * An example of this routine in assembly for uC/OS:

*
* The Interrupt Routine for a CPU32 to call the Mpc860 Driver
*
_tfMpc860Scc1Isr
* Save the Registers
        MOVEM.L D0-D7/A0-A6,-(A7)

* uC/OS Call to tell RTOS we are in an ISR
        JSR     _OSIntEnter

* Call the handler
        JSR     _tfMpc860Scc1HandlerIsr

* uC/OS Call to tell the RTOS that we are leaving the ISR
        JSR     _OSIntExit

* Restore the registers
        MOVEM.L (A7)+,D0-D7/A0-A6

* Return from Exception (Interrupt)
        RTE     

 */

void tfMpc860Scc1Isr(void);
void tfMpc860Scc4Isr(void);

int tfMpc860GetPhyAddr(ttUserInterface   userInterface,
                        char           * addressPtr);

int tfMpc860Ioctl(ttUserInterface interfaceHandle, int flag,
                  void TM_FAR * optionPtr, int optionLen);

int tfMpc860EtherReceive(ttUserInterface   interfaceHandle,
                         char TM_FAR    ** dataPtrPtr,
                         int  TM_FAR     * lengthPtr,
                         ttUserBufferPtr   userBufferHandlePtr);

int tfMpc860EtherSend(ttUserInterface  interfaceHandle,
                      char TM_FAR    * bufferPtr,
                      int              len,
                      int              flag);

int tfMpc860SerialReceive(ttUserInterface   interfaceHandle,
                          char TM_FAR    ** dataPtrPtr,
                          int  TM_FAR     * lengthPtr,
                          ttUserBufferPtr   userBufferHandlePtr);

int tfMpc860SerialSend(ttUserInterface  interfaceHandle,
                       char TM_FAR    * bufferPtr,
                       int              len,
                       int              flag);
int tfMpc860EtherOpen(ttUserInterface interfaceHandle);

int tfMpc860EtherClose(ttUserInterface interfaceHandle);

void tfMpc860Scc1HandlerIsr(void);

int tfMpc860SerialOpen(ttUserInterface interfaceHandle);

int tfMpc860SerialClose(ttUserInterface interfaceHandle);

void tfMpc860Scc4HandlerIsr(void);

int tfMpc860EtherFreeRecvBuffer( ttUserInterface   interfaceHandle,
                                 char            * dataPtr);

int tfMpc860SerialFreeRecvBuffer(ttUserInterface   interfaceHandle,
                                 char            * dataPtr);

int tfMpc860FECOpen(ttUserInterface interfaceHandle);

int tfMpc860FECClose(ttUserInterface interfaceHandle);

void tfMpc860FECHandlerIsr(void);

int tfMpc860FECSend(ttUserInterface  interfaceHandle,
                    char TM_FAR    * bufferPtr,
                    int              len,
                    int              flag);

void tfMpc860FECIsr(void);

int tfMpc860FECReceive(ttUserInterface   interfaceHandle,
                       char TM_FAR    ** dataPtrPtr,
                       int  TM_FAR     * lengthPtr,
                       ttUserBufferPtr   userBufferHandlePtr);

int tfMpc860FECFreeRecvBuffer(ttUserInterface   interfaceHandle,
                              char            * dataPtr);

/*
 * Mpc860 Defines
 */
#define TM_MPC860_IMMR_LOC      0xff000000UL

/*
 * Interrupt Level for the CPIC on the 860
 */
#define TM_MPC860_CPIC_SIVEC     0x24000000                     /* Level 4 */
#define TM_MPC860_CPIC_LEVEL     4UL
#define TM_MPC860_CPIC_MASK      0x00400000UL
#define TM_MPC860_CIVR_IACK      1UL

/* Slot number (0-3) that the mpc resides in */
#define TM_MPC860_DARWIN_SERIAL_SLOT_NUM 3

/*
 * The CICR register is used to set the vector base address, the IRQ Level
 * set the priorities of the SCC's.  If it is not set correctly then the 
 * SCC's will NEVER interrupt.
 *
 * We set it up to the following default:
 *
 * Priority: Lowest to Highest
 * SCCd = SCC4
 * SCCc = SCC3
 * SCCb = SCC2
 * SCCa = SCC1
 *
 * IRQ Level = 4
 *
 * Highest Priority = Original Order 0b11111
 *
 * Interrupt Enable = SET
 */
#define TM_MPC860_CICR_DEFAULT  0x00E40F80UL|(TM_MPC860_CPIC_LEVEL<<13)
#define TM_MPC860_BRGC_RST      0x00010000UL
#define TM_MPC860_BRGC_DIV16    0x00000001UL
/*
 * Timer Interrupt Number
 */
#define TM_MPC860_TMR1_INT      0x00000019UL

/*
 * SCC(x) Interrupt Numbers
 */
#define TM_MPC860_SCCA_INT      0x0000001eUL
#define TM_MPC860_SCCB_INT      0x0000001dUL
#define TM_MPC860_SCCC_INT      0x0000001cUL
#define TM_MPC860_SCCD_INT      0x0000001bUL

/*
 * FEC Interrupt Numbers
 */
#define TM_MPC860_FEC_INT               0x2c000000UL
#define TM_MPC860_FEC_LEVEL     5UL
#define TM_MPC860_FEC_MASK      0x00100000UL

/*
 * Timer Interrupt Bit
 */
#define TM_MPC860_TMR1_INT_MASK 0x02000000UL

/* 
 * SCC(x) Interrupt Bit
 */
#define TM_MPC860_SCC1_INT_MASK 0x40000000UL
#define TM_MPC860_SCC2_INT_MASK 0x20000000UL
#define TM_MPC860_SCC3_INT_MASK 0x10000000UL
#define TM_MPC860_SCC4_INT_MASK 0x08000000UL

/* 
 * SCC(x) Commands
 */
#define TM_MPC860_INIT_RXTX_SCC1 (unsigned short)0x0001
#define TM_MPC860_INIT_RXTX_SCC2 (unsigned short)0x0041
#define TM_MPC860_INIT_RXTX_SCC3 (unsigned short)0x0081
#define TM_MPC860_INIT_RXTX_SCC4 (unsigned short)0x00C1

#define TM_MPC860_RESTART_TX (unsigned short)0x0600

/* 
 * Ethernet Defines 
 */
/* Perform 32 bit CRC */
#define TM_MPC860_ETHER_CRC_PRES         0xffffffffUL 
/* Comply with 32 bit CRC */    
#define TM_MPC860_ETHER_CRC_MASK         0xdebb20e3UL 
/* Zero for clarity */
#define TM_MPC860_ETHER_CRCEC            0x00000000UL
#define TM_MPC860_ETHER_ALEC             0x00000000UL
#define TM_MPC860_ETHER_DISFC            0x00000000UL
#define TM_MPC860_ETHER_PADS             0x00000000UL
/* 15 Collision Trys */
#define TM_MPC860_ETHER_RETRY_LIMIT      (unsigned short)0x000f 
/* Maxmimum Ethernet Frame Length = 1518 (DONT CHANGE) */
#define TM_MPC860_ETHER_MAX_FRAME_LEN    (unsigned short)0x05ee
/* Minimum Ethernet Frame Length = 64 (DONT CHANGE) */
#define TM_MPC860_ETHER_MIN_FRAME_LEN    (unsigned short)0x0040
/* Maximum Ethernet DMA count */
#define TM_MPC860_ETHER_MAX_DMA1_COUNT   TM_MPC860_ETHER_MAX_FRAME_LEN
#define TM_MPC860_ETHER_MAX_DMA2_COUNT   TM_MPC860_ETHER_MAX_FRAME_LEN
/* Clear the group addresses */
#define TM_MPC860_ETHER_GROUP_ADDR1      0x00000000UL
#define TM_MPC860_ETHER_GROUP_ADDR2      0x00000000UL        
#define TM_MPC860_ETHER_GROUP_ADDR3      0x00000000UL        
#define TM_MPC860_ETHER_GROUP_ADDR4      0x00000000UL        
/* Not Used */
#define TM_MPC860_ETHER_P_PER            0x00000000UL
/* Individual Hash table is not used */
#define TM_MPC860_ETHER_INDV_ADDR1       0x00000000UL
#define TM_MPC860_ETHER_INDV_ADDR2       0x00000000UL
#define TM_MPC860_ETHER_INDV_ADDR3       0x00000000UL
#define TM_MPC860_ETHER_INDV_ADDR4       0x00000000UL
/* Zeroed */
#define TM_MPC860_ETHER_T_ADDR_H         0x00000000UL 
#define TM_MPC860_ETHER_T_ADDR_M         0x00000000UL 
#define TM_MPC860_ETHER_T_ADDR_L         0x00000000UL 


/*
 * SCC Channel Numbers
 */
#define TM_MPC860_SCC1_CHANNEL 0
#define TM_MPC860_SCC2_CHANNEL 1
#define TM_MPC860_SCC3_CHANNEL 2
#define TM_MPC860_SCC4_CHANNEL 3

/*
 * SCC Parameter Ram 
 */
#define TM_MPC860_COMMAND_FLAG (unsigned short)0x0001
#define TM_MPC860_SCC1_RECV_BASE (unsigned short)0x0000  
#define TM_MPC860_SCC1_XMIT_BASE \
            TM_MPC860_SCC1_RECV_BASE + \
            (TM_MPC860_ETHER_MAX_RECV_BUFD * sizeof(struct tsMpc860BufDesc) )

#define TM_MPC860_SCC4_RECV_BASE \
            TM_MPC860_SCC1_XMIT_BASE + \
            (TM_MPC860_ETHER_MAX_XMIT_BUFD * sizeof(struct tsMpc860BufDesc) )

#define TM_MPC860_SCC4_XMIT_BASE \
            TM_MPC860_SCC4_RECV_BASE + \
            (TM_MPC860_SERIAL_MAX_RECV_BUFD * sizeof(struct tsMpc860BufDesc) )


/* Receive normal operation */ 
#define TM_MPC860_SCC1_RECV_FUNC_CODE    (unsigned short)0x18    
#define TM_MPC860_SCC4_RECV_FUNC_CODE    (unsigned short)0x15    
/* Transmit normal operation */ 
#define TM_MPC860_SCC1_XMIT_FUNC_CODE    (unsigned short)0x18 
#define TM_MPC860_SCC4_XMIT_FUNC_CODE    (unsigned short)0x15 
/* Max ethernet frame length (Divisable by 4)*/ 
#define TM_MPC860_FEC_MAX_RECV_BUF_LEN   (unsigned short)1520
#define TM_MPC860_SCC1_MAX_RECV_BUF_LEN  (unsigned short)1520

#define TM_MPC860_SCC4_MAX_RECV_BUF_LEN  (unsigned short)1520
#define TM_MPC860_SCC4_MAX_IDL16         (unsigned short)0x10        
#define TM_MPC860_SCC4_BREAK_COUNT       (unsigned short)3

#define TM_MPC860_SCC_FRAME_SYNC_ETHER   (unsigned short)0xd555


/*
 * Port A Registers
 */
#define TM_MPC860_PORTA_PA0  (unsigned short)0x8000
#define TM_MPC860_PORTA_PA1  (unsigned short)0x4000
#define TM_MPC860_PORTA_PA2  (unsigned short)0x2000
#define TM_MPC860_PORTA_PA3  (unsigned short)0x1000
#define TM_MPC860_PORTA_PA4  (unsigned short)0x0800
#define TM_MPC860_PORTA_PA5  (unsigned short)0x0400
#define TM_MPC860_PORTA_PA6  (unsigned short)0x0200
#define TM_MPC860_PORTA_PA7  (unsigned short)0x0100
#define TM_MPC860_PORTA_PA8  (unsigned short)0x0080
#define TM_MPC860_PORTA_PA9  (unsigned short)0x0040
#define TM_MPC860_PORTA_PA10 (unsigned short)0x0020
#define TM_MPC860_PORTA_PA11 (unsigned short)0x0010
#define TM_MPC860_PORTA_PA12 (unsigned short)0x0008
#define TM_MPC860_PORTA_PA13 (unsigned short)0x0004
#define TM_MPC860_PORTA_PA14 (unsigned short)0x0002
#define TM_MPC860_PORTA_PA15 (unsigned short)0x0001

/*
 * Port B Registers
 */
#define TM_MPC860_PORTB_PB0  0x80000000UL
#define TM_MPC860_PORTB_PB1  0x40000000UL
#define TM_MPC860_PORTB_PB2  0x20000000UL
#define TM_MPC860_PORTB_PB3  0x10000000UL
#define TM_MPC860_PORTB_PB4  0x08000000UL
#define TM_MPC860_PORTB_PB5  0x04000000UL
#define TM_MPC860_PORTB_PB6  0x02000000UL
#define TM_MPC860_PORTB_PB7  0x01000000UL
#define TM_MPC860_PORTB_PB8  0x00800000UL
#define TM_MPC860_PORTB_PB9  0x00400000UL
#define TM_MPC860_PORTB_PB10 0x00200000UL
#define TM_MPC860_PORTB_PB11 0x00100000UL
#define TM_MPC860_PORTB_PB12 0x00080000UL
#define TM_MPC860_PORTB_PB13 0x00040000UL
#define TM_MPC860_PORTB_PB14 0x00020000UL
#define TM_MPC860_PORTB_PB15 0x00010000UL
#define TM_MPC860_PORTB_PB16 0x00008000UL
#define TM_MPC860_PORTB_PB17 0x00004000UL
#define TM_MPC860_PORTB_PB18 0x00002000UL
#define TM_MPC860_PORTB_PB19 0x00001000UL
#define TM_MPC860_PORTB_PB20 0x00000800UL
#define TM_MPC860_PORTB_PB21 0x00000400UL
#define TM_MPC860_PORTB_PB22 0x00000200UL
#define TM_MPC860_PORTB_PB23 0x00000100UL
#define TM_MPC860_PORTB_PB24 0x00000080UL
#define TM_MPC860_PORTB_PB25 0x00000040UL
#define TM_MPC860_PORTB_PB26 0x00000020UL
#define TM_MPC860_PORTB_PB27 0x00000010UL
#define TM_MPC860_PORTB_PB28 0x00000008UL
#define TM_MPC860_PORTB_PB29 0x00000004UL
#define TM_MPC860_PORTB_PB30 0x00000002UL
#define TM_MPC860_PORTB_PB31 0x00000001UL


/*
 * Port C Registers
 */
#define TM_MPC860_PORTC_PC0  (unsigned short)0x8000
#define TM_MPC860_PORTC_PC1  (unsigned short)0x4000
#define TM_MPC860_PORTC_PC2  (unsigned short)0x2000
#define TM_MPC860_PORTC_PC3  (unsigned short)0x1000
#define TM_MPC860_PORTC_PC4  (unsigned short)0x0800
#define TM_MPC860_PORTC_PC5  (unsigned short)0x0400
#define TM_MPC860_PORTC_PC6  (unsigned short)0x0200
#define TM_MPC860_PORTC_PC7  (unsigned short)0x0100
#define TM_MPC860_PORTC_PC8  (unsigned short)0x0080
#define TM_MPC860_PORTC_PC9  (unsigned short)0x0040
#define TM_MPC860_PORTC_PC10 (unsigned short)0x0020
#define TM_MPC860_PORTC_PC11 (unsigned short)0x0010
#define TM_MPC860_PORTC_PC12 (unsigned short)0x0008
#define TM_MPC860_PORTC_PC13 (unsigned short)0x0004
#define TM_MPC860_PORTC_PC14 (unsigned short)0x0002
#define TM_MPC860_PORTC_PC15 (unsigned short)0x0001

/*
 * Port D Registers
 */
#define TM_MPC860_PORTD_PD0  (unsigned short)0x8000
#define TM_MPC860_PORTD_PD1  (unsigned short)0x4000
#define TM_MPC860_PORTD_PD2  (unsigned short)0x2000
#define TM_MPC860_PORTD_PD3  (unsigned short)0x1000
#define TM_MPC860_PORTD_PD4  (unsigned short)0x0800
#define TM_MPC860_PORTD_PD5  (unsigned short)0x0400
#define TM_MPC860_PORTD_PD6  (unsigned short)0x0200
#define TM_MPC860_PORTD_PD7  (unsigned short)0x0100
#define TM_MPC860_PORTD_PD8  (unsigned short)0x0080
#define TM_MPC860_PORTD_PD9  (unsigned short)0x0040
#define TM_MPC860_PORTD_PD10 (unsigned short)0x0020
#define TM_MPC860_PORTD_PD11 (unsigned short)0x0010
#define TM_MPC860_PORTD_PD12 (unsigned short)0x0008
#define TM_MPC860_PORTD_PD13 (unsigned short)0x0004
#define TM_MPC860_PORTD_PD14 (unsigned short)0x0002
#define TM_MPC860_PORTD_PD15 (unsigned short)0x0001

 

/*
 * SI Clock Route Register 
 */

/*
 * SCC1 Clock Sources
 */
#define TM_MPC860_SI_SCC1_GRANT    0x00000080UL
#define TM_MPC860_SI_SCC1_CONNECT  0x00000040UL
#define TM_MPC860_SI_SCC1_RCS_BRG1 0x00000000UL
#define TM_MPC860_SI_SCC1_RCS_BRG2 0x00000008UL
#define TM_MPC860_SI_SCC1_RCS_BRG3 0x00000010UL
#define TM_MPC860_SI_SCC1_RCS_BRG4 0x00000018UL
#define TM_MPC860_SI_SCC1_RCS_CLK1 0x00000020UL
#define TM_MPC860_SI_SCC1_RCS_CLK2 0x00000028UL
#define TM_MPC860_SI_SCC1_RCS_CLK3 0x00000030UL
#define TM_MPC860_SI_SCC1_RCS_CLK4 0x00000038UL
#define TM_MPC860_SI_SCC1_TCS_BRG1 0x00000000UL
#define TM_MPC860_SI_SCC1_TCS_BRG2 0x00000001UL
#define TM_MPC860_SI_SCC1_TCS_BRG3 0x00000002UL
#define TM_MPC860_SI_SCC1_TCS_BRG4 0x00000003UL
#define TM_MPC860_SI_SCC1_TCS_CLK1 0x00000004UL
#define TM_MPC860_SI_SCC1_TCS_CLK2 0x00000005UL
#define TM_MPC860_SI_SCC1_TCS_CLK3 0x00000006UL
#define TM_MPC860_SI_SCC1_TCS_CLK4 0x00000007UL

/*
 * SCC2 Clock Sources
 */
#define TM_MPC860_SI_SCC2_GRANT    0x00008000UL
#define TM_MPC860_SI_SCC2_CONNECT  0x00004000UL

#define TM_MPC860_SI_SCC2_RCS_BRG1 0x00000000UL
#define TM_MPC860_SI_SCC2_RCS_BRG2 0x00000800UL
#define TM_MPC860_SI_SCC2_RCS_BRG3 0x00001000UL
#define TM_MPC860_SI_SCC2_RCS_BRG4 0x00001800UL
#define TM_MPC860_SI_SCC2_RCS_CLK1 0x00002000UL
#define TM_MPC860_SI_SCC2_RCS_CLK2 0x00002800UL
#define TM_MPC860_SI_SCC2_RCS_CLK3 0x00003000UL
#define TM_MPC860_SI_SCC2_RCS_CLK4 0x00003800UL
#define TM_MPC860_SI_SCC2_TCS_BRG1 0x00000000UL
#define TM_MPC860_SI_SCC2_TCS_BRG2 0x00000100UL
#define TM_MPC860_SI_SCC2_TCS_BRG3 0x00000200UL
#define TM_MPC860_SI_SCC2_TCS_BRG4 0x00000300UL
#define TM_MPC860_SI_SCC2_TCS_CLK1 0x00000400UL
#define TM_MPC860_SI_SCC2_TCS_CLK2 0x00000500UL
#define TM_MPC860_SI_SCC2_TCS_CLK3 0x00000600UL
#define TM_MPC860_SI_SCC2_TCS_CLK4 0x00000700UL

/*
 * SCC3 Clock Sources
 */
#define TM_MPC860_SI_SCC3_GRANT    0x00800000UL
#define TM_MPC860_SI_SCC3_CONNECT  0x00400000UL

#define TM_MPC860_SI_SCC3_RCS_BRG1 0x00000000UL
#define TM_MPC860_SI_SCC3_RCS_BRG2 0x00080000UL
#define TM_MPC860_SI_SCC3_RCS_BRG3 0x00100000UL
#define TM_MPC860_SI_SCC3_RCS_BRG4 0x00180000UL
#define TM_MPC860_SI_SCC3_RCS_CLK5 0x00200000UL
#define TM_MPC860_SI_SCC3_RCS_CLK6 0x00280000UL
#define TM_MPC860_SI_SCC3_RCS_CLK7 0x00300000UL
#define TM_MPC860_SI_SCC3_RCS_CLK8 0x00380000UL
#define TM_MPC860_SI_SCC3_TCS_BRG1 0x00000000UL
#define TM_MPC860_SI_SCC3_TCS_BRG2 0x00010000UL
#define TM_MPC860_SI_SCC3_TCS_BRG3 0x00020000UL
#define TM_MPC860_SI_SCC3_TCS_BRG4 0x00030000UL
#define TM_MPC860_SI_SCC3_TCS_CLK5 0x00040000UL
#define TM_MPC860_SI_SCC3_TCS_CLK6 0x00050000UL
#define TM_MPC860_SI_SCC3_TCS_CLK7 0x00060000UL
#define TM_MPC860_SI_SCC3_TCS_CLK8 0x00070000UL

/*
 * SCC4 Clock Sources
 */
#define TM_MPC860_SI_SCC4_GRANT    0x80000000UL
#define TM_MPC860_SI_SCC4_CONNECT  0x40000000UL

#define TM_MPC860_SI_SCC4_RCS_BRG1 0x00000000UL
#define TM_MPC860_SI_SCC4_RCS_BRG2 0x08000000UL
#define TM_MPC860_SI_SCC4_RCS_BRG3 0x10000000UL
#define TM_MPC860_SI_SCC4_RCS_BRG4 0x18000000UL
#define TM_MPC860_SI_SCC4_RCS_CLK5 0x20000000UL
#define TM_MPC860_SI_SCC4_RCS_CLK6 0x28000000UL
#define TM_MPC860_SI_SCC4_RCS_CLK7 0x30000000UL
#define TM_MPC860_SI_SCC4_RCS_CLK8 0x38000000UL
#define TM_MPC860_SI_SCC4_TCS_BRG1 0x00000000UL
#define TM_MPC860_SI_SCC4_TCS_BRG2 0x01000000UL
#define TM_MPC860_SI_SCC4_TCS_BRG3 0x02000000UL
#define TM_MPC860_SI_SCC4_TCS_BRG4 0x03000000UL
#define TM_MPC860_SI_SCC4_TCS_CLK5 0x04000000UL
#define TM_MPC860_SI_SCC4_TCS_CLK6 0x05000000UL
#define TM_MPC860_SI_SCC4_TCS_CLK7 0x06000000UL
#define TM_MPC860_SI_SCC4_TCS_CLK8 0x07000000UL


/*
 * Transmit Buffer Descriptor Flags
 */
#define TM_MPC860_XMIT_READY     (unsigned short)0x8000  /* ready bit */
/* short frame padding */
#define TM_MPC860_XMIT_PAD       (unsigned short)0x4000
#define TM_MPC860_XMIT_WRAP      (unsigned short)0x2000  /* wrap bit */
#define TM_MPC860_XMIT_INTR      (unsigned short)0x1000  /* interrupt on completion */
#define TM_MPC860_XMIT_LAST      (unsigned short)0x0800  /* last in frame */
/* transmit CRC (when last) */
#define TM_MPC860_XMIT_CRC       (unsigned short)0x0400

/* 
 * Transmit Error Conditions
 */
#define TM_MPC860_XMIT_ETHER_ERROR   (unsigned short)0x00c2
/* defer indication */
#define TM_MPC860_XMIT_E_DEFER       (unsigned short)0x0200
#define TM_MPC860_XMIT_E_HEARTB      (unsigned short)0x0100  /* heartbeat */
/* error: late collision */
#define TM_MPC860_XMIT_E_LATEC       (unsigned short)0x0080
/* error: retransmission limit */
#define TM_MPC860_XMIT_E_LIMIT       (unsigned short)0x0040
#define TM_MPC860_XMIT_E_COUNT       (unsigned short)0x003c  /* retry count */
/* error: underrun */
#define TM_MPC860_XMIT_E_UNDERRUN    (unsigned short)0x0002
/* carier sense lost */
#define TM_MPC860_XMIT_E_CARRIER     (unsigned short)0x0001

/*
 * Receive Buffer Descriptor Flags
 */
#define TM_MPC860_RECV_EMPTY     (unsigned short)0x8000  /* buffer empty */
#define TM_MPC860_RECV_WRAP      (unsigned short)0x2000  /* wrap bit */
/* interrupt on reception */
#define TM_MPC860_RECV_INTR      (unsigned short)0x1000
/* last BD in frame */
#define TM_MPC860_RECV_LAST      (unsigned short)0x0800
/* first BD in frame */
#define TM_MPC860_RECV_FIRST     (unsigned short)0x0400

/*
 * Receive Error Conditions 
 */
#define TM_MPC860_RECV_ERROR     (unsigned short)0x00ff
#define TM_MPC860_RECV_E_TOOLONG (unsigned short)0x0020  /* frame too long */
/* non-octet aligned */
#define TM_MPC860_RECV_E_NOTBYTE (unsigned short)0x0010
#define TM_MPC860_RECV_E_SHORT   (unsigned short)0x0008  /* short frame */
/* receive CRC error */
#define TM_MPC860_RECV_E_CRC     (unsigned short)0x0004
#define TM_MPC860_RECV_E_OVERRUN (unsigned short)0x0002  /* receive overrun */
#define TM_MPC860_RECT_E_COLL    (unsigned short)0x0001  /* collision */


/*
 * Ethernet Interrupts
 */
/* graceful stop complete */
#define TM_MPC860_INTR_ETHER_STOP    (unsigned short)0x0080
/* transmit error */
#define TM_MPC860_INTR_ETHER_E_XMIT  (unsigned short)0x0010
/* receive frame */
#define TM_MPC860_INTR_ETHER_RECV_F  (unsigned short)0x0008
/* busy condition */
#define TM_MPC860_INTR_ETHER_BUSY    (unsigned short)0x0004
/* transmit buffer */
#define TM_MPC860_INTR_ETHER_XMIT_B  (unsigned short)0x0002
/* receive buffer */
#define TM_MPC860_INTR_ETHER_RECV_B  (unsigned short)0x0001

/*
 * FEC Interupts
 */
/* receive frame */
#define TM_MPC860T_INTR_ETHER_RECV_F  0x02000000UL

/* transmit frame */
#define TM_MPC860T_INTR_ETHER_XMIT_F  0x08000000UL


/*
 * Serial Interrupts
 */
/* busy condition */
#define TM_MPC860_INTR_SERIAL_BUSY   (unsigned short)0x0004
/* transmit buffer */
#define TM_MPC860_INTR_SERIAL_XMIT_B (unsigned short)0x0002
/* receive buffer */
#define TM_MPC860_INTR_SERIAL_RECV_B (unsigned short)0x0001


/* 
 * Ethernet mode register 
 */
/* individual address mode */
#define TM_MPC860_ETHER_IND_ADDR      (unsigned short)0x1000
/* Enable CRC */
#define TM_MPC860_ETHER_ENABLE_CRC    (unsigned short)0x0800
/* Loop Back Mode */
#define TM_MPC860_ETHER_LOOPBACK      (unsigned short)0x0040
/* # of ignored bits 22 */
#define TM_MPC860_ETHER_NBITS_IGNORED (unsigned short)0x000a
#define TM_MPC860_ETHER_PROMISCUOUS   (unsigned short)0x0200 /* promiscuous */
/* broadcast address */
#define TM_MPC860_ETHER_BROADCAST     (unsigned short)0x0100


/* Uart Mode Register */
#define TM_MPC860_UART_FLOW           (unsigned short)0x8000
#define TM_MPC860_STOP_2              (unsigned short)0x4000
#define TM_MPC860_CHAR_6              (unsigned short)0x1000
#define TM_MPC860_CHAR_7              (unsigned short)0x2000
#define TM_MPC860_CHAR_8              (unsigned short)0x3000

/*****************************************************************
        General SCC mode register (GSMR)
*****************************************************************/
/* GSMR LOW */
/* SCC modes */
#define TM_MPC860_HDLC_PORT                0x0UL
#define TM_MPC860_HDLC_BUS                 0x1UL
#define TM_MPC860_APPLE_TALK               0x2UL
#define TM_MPC860_SS_NO7                   0x3UL
#define TM_MPC860_UART                     0x4UL
#define TM_MPC860_PROFI_BUS                0x5UL
#define TM_MPC860_ASYNC_HDLC               0x6UL
#define TM_MPC860_V14                      0x7UL
#define TM_MPC860_BISYNC_PORT              0x8UL
#define TM_MPC860_DDCMP_PORT               0x9UL
#define TM_MPC860_ETHERNET_PORT            0xcUL

#define TM_MPC860_ENABLE_XMIT       0x00000010UL
#define TM_MPC860_ENABLE_RECV       0x00000020UL

#define TM_MPC860_PREAMBLE_PAT_10   0x00080000UL
#define TM_MPC860_PREAMBLE_48       0x00800000UL
#define TM_MPC860_XMIT_CLOCK_INV    0x10000000UL
#define TM_MPC860_UART_XMIT_SAMPLE  0x00020000UL 
#define TM_MPC860_UART_RECV_SAMPLE  0x00008000UL 

/* GSMR HIGH */
#define TM_MPC860_TRANSPARENT_CRC   0x00008000UL
#define TM_MPC860_TRANSPARENT_RECV  0x00001000UL
#define TM_MPC860_TRANSPARENT_XMIT  0x00000800UL
#define TM_MPC860_CTS_SAMPLE        0x00000080UL
#define TM_MPC860_CD_SAMPLE         0x00000100UL
#define TM_MPC860_UART_RECV_FIFO_1  0x00000020UL

/* 
 * SDMA Defines
 */
#define TM_MPC860_SDMA_ARB_ID   (unsigned short)0x0001


/*
 * UART defines 
 */
#define TM_MPC860_UART_REJECT_CHAR 0x4000
#define TM_MPC860_UART_LAST_CHAR   0x8000


/* 
 * MPC860T FEC specific defines 
 */

/* Multicast hash table defines */
#define TM_MPC860T_HASH_TABLE_HIGH          0x00000000UL
#define TM_MPC860T_HASH_TABLE_LOW           0x00000000UL

/* Ethernet Control register, bit definitions */
#define TM_MPC860T_ECNTRL_ETHERNETENABLE    0x02
#define TM_MPC860T_ECNTRL_ETHERNETRESET     0x01

/* Interrupt Event register, bit definitions */
/*   also good for interrupt mask register  */
#define TM_MPC860T_IEVENT_HEARTBEATERROR    0x80000000UL
#define TM_MPC860T_IEVENT_BABBLINGRECEIVER  0x40000000UL
#define TM_MPC860T_IEVENT_BABBLINGTRANSMIT  0x20000000UL
#define TM_MPC860T_IEVENT_GRACEFULSTOPACK   0x10000000UL
#define TM_MPC860T_IEVENT_TRANSMITFRAMEINT  0x08000000UL
#define TM_MPC860T_IEVENT_TRANSMITBUFINT    0x04000000UL
#define TM_MPC860T_IEVENT_RECEIVEFRAMEINT   0x02000000UL
#define TM_MPC860T_IEVENT_RECEIVEBUFINT     0x01000000UL
#define TM_MPC860T_IEVENT_MIIINT            0x00800000UL
#define TM_MPC860T_IEVENT_ETHERNETBUSERROR  0x00400000UL

/* Receive Control */
#define TM_MPC860T_PROMISCUOUS              0x00000008
#define TM_MPC860T_MIIMODE                  0x00000004
#define TM_MPC860T_RDT                      0x00000002
#define TM_MPC860T_INTERNALLOOPBACK         0x00000001

/* Transmit Control */
#define TM_MPC860T_FULLDUPLEXENABLE         0x00000004
#define TM_MPC860T_HEARTBEATCONTROL         0x00000002
#define TM_MPC860T_GRACEFULTRANSMITSTOP     0x00000001

#define TM_MPC860T_DATABO0                  0x40000000UL
#define TM_MPC860T_DATABO1                  0x20000000UL
#define TM_MPC860T_DESCBO0                  0x10000000UL
#define TM_MPC860T_DESCBO1                  0x08000000UL
#define TM_MPC860T_FC1                      0x04000000UL
#define TM_MPC860T_FC2                      0x02000000UL
#define TM_MPC860T_FC3                      0x01000000UL

/* activate  recieve buffers */
#define TM_MPC860T_BD_ACTIVE                0x01000000UL

/* FEC interrupt vectors */
#define TM_MPC860T_IVEC_NONE                0
#define TM_MPC860T_IVEC_NONTIMECRITICAL     1
#define TM_MPC860T_IVEC_TRANSMITINT         2
#define TM_MPC860T_IVEC_RECEIVEINT          3
#define TM_MPC860T_IVEC_LEVEL               29


/* FEC interrupt level */
#define TM_MPC860T_FEC_LEVEL                5


/*
 * tbase and rbase registers
 */
#define TM_MPC860_XMIT_BD_ADDR(quicc,pram)                              \
            ( (ttMpc860BufDescPtr)(quicc->udata_bd_ucode + pram->tbase) )

#define TM_MPC860_RECV_BD_ADDR(quicc,pram)                              \
            ( (ttMpc860BufDescPtr)(quicc->udata_bd_ucode + pram->rbase) )

#define TTBD_ADDR(quicc,pram)                                           \
            ( (ttMpc860BufDescPtr)(quicc->udata_bd_ucode + pram->tbptr) )


/*****************************************************************
        HDLC parameter RAM
*****************************************************************/

struct hdlc_pram
{
/*
 * SCC parameter RAM
 */
    unsigned short  rbase;          /* RX BD base address */
    unsigned short  tbase;          /* TX BD base address */
    unsigned char   rfcr;           /* Rx function code */
    unsigned char   tfcr;           /* Tx function code */
    unsigned short  mrblr;          /* Rx buffer length */
    unsigned long   rstate;         /* Rx internal state */
    unsigned long   rptr;           /* Rx internal data pointer */
    unsigned short  rbptr;          /* rb BD Pointer */
    unsigned short  rcount;         /* Rx internal byte count */
    unsigned long   rtemp;          /* Rx temp */
    unsigned long   tstate;         /* Tx internal state */
    unsigned long   tptr;           /* Tx internal data pointer */
    unsigned short  tbptr;          /* Tx BD pointer */
    unsigned short  tcount;         /* Tx byte count */
    unsigned long   ttemp;          /* Tx temp */
    unsigned long   rcrc;           /* temp receive CRC */
    unsigned long   tcrc;           /* temp transmit CRC */

/*
 * HDLC specific parameter RAM
 */
    unsigned char   RESERVED1[4];   /* Reserved area */
    unsigned long   c_mask;         /* CRC constant */
    unsigned long   c_pres;         /* CRC preset */
    unsigned short  disfc;          /* discarded frame counter */
    unsigned short  crcec;          /* CRC error counter */
    unsigned short  abtsc;          /* abort sequence counter */
    unsigned short  nmarc;          /* nonmatching address rx cnt */
    unsigned short  retrc;          /* frame retransmission cnt */
    unsigned short  mflr;           /* maximum frame length reg */
    unsigned short  max_cnt;        /* maximum length counter */
    unsigned short  rfthr;          /* received frames threshold */
    unsigned short  rfcnt;          /* received frames count */
    unsigned short  hmask;          /* user defined frm addr mask */
    unsigned short  haddr1;         /* user defined frm address 1 */
    unsigned short  haddr2;         /* user defined frm address 2 */
    unsigned short  haddr3;         /* user defined frm address 3 */
    unsigned short  haddr4;         /* user defined frm address 4 */
    unsigned short  tmp;            /* temp */
    unsigned short  tmp_mb;         /* temp */
};


/*****************************************************************
        ASYNC HDLC parameter RAM
*****************************************************************/

struct async_hdlc_pram
{
/*
 * SCC parameter RAM
 */
    unsigned short  rbase;          /* RX BD base address */
    unsigned short  tbase;          /* TX BD base address */
    unsigned char   rfcr;           /* Rx function code */
    unsigned char   tfcr;           /* Tx function code */
    unsigned short  mrblr;          /* Rx buffer length */
    unsigned long   rstate;         /* Rx internal state */
    unsigned long   rptr;           /* Rx internal data pointer */
    unsigned short  rbptr;          /* rb BD Pointer */
    unsigned short  rcount;         /* Rx internal byte count */
    unsigned long   rtemp;          /* Rx temp */
    unsigned long   tstate;         /* Tx internal state */
    unsigned long   tptr;           /* Tx internal data pointer */
    unsigned short  tbptr;          /* Tx BD pointer */
    unsigned short  tcount;         /* Tx byte count */
    unsigned long   ttemp;          /* Tx temp */
    unsigned long   rcrc;           /* temp receive CRC */
    unsigned long   tcrc;           /* temp transmit CRC */

/*
 * ASYNC HDLC specific parameter RAM
 */
    unsigned char   RESERVED1[4];   /* Reserved area */
    unsigned long   c_mask;         /* CRC constant */
    unsigned long   c_pres;         /* CRC preset */
    unsigned short  bof;            /* begining of flag character */
    unsigned short  eof;            /* end of flag character */
    unsigned short  esc;            /* control escape character */
    unsigned char   RESERVED2[4];   /* Reserved area */
    unsigned short  zero;           /* zero */
    unsigned char   RESERVED3[2];   /* Reserved area */
    unsigned short  rfthr;          /* received frames threshold */
    unsigned char   RESERVED4[4];   /* Reserved area */
    unsigned long   txctl_tbl;      /* Tx ctl char mapping table */
    unsigned long   rxctl_tbl;      /* Rx ctl char mapping table */
    unsigned short  nof;            /* Number of opening flags */
};


/*****************************************************************
        UART parameter RAM
*****************************************************************/

/*
 * bits in uart control characters table
 */
#define CC_INVALID      0x8000          /* control character is valid */
#define CC_REJ          0x4000          /* don't store char in buffer */
#define CC_CHAR         0x00ff          /* control character */

/* UART */
struct uart_pram
{
/*
 * SCC parameter RAM
 */
    unsigned short  rbase;          /* RX BD base address */
    unsigned short  tbase;          /* TX BD base address */
    unsigned char   rfcr;           /* Rx function code */
    unsigned char   tfcr;           /* Tx function code */
    unsigned short  mrblr;          /* Rx buffer length */
    unsigned long   rstate;         /* Rx internal state */
    unsigned long   rptr;           /* Rx internal data pointer */
    unsigned short  rbptr;          /* rb BD Pointer */
    unsigned short  rcount;         /* Rx internal byte count */
    unsigned long   rx_temp;        /* Rx temp */
    unsigned long   tstate;         /* Tx internal state */
    unsigned long   tptr;           /* Tx internal data pointer */
    unsigned short  tbptr;          /* Tx BD pointer */
    unsigned short  tcount;         /* Tx byte count */
    unsigned long   ttemp;          /* Tx temp */
    unsigned long   rcrc;           /* temp receive CRC */
    unsigned long   tcrc;           /* temp transmit CRC */

/*
 * UART specific parameter RAM
 */
    unsigned char   RESERVED1[8];   /* Reserved area */
    unsigned short  max_idl;        /* maximum idle characters */
    unsigned short  idlc;           /* rx idle counter (internal) */
    unsigned short  brkcr;          /* break count register */

    unsigned short  parec;          /* Rx parity error counter */
    unsigned short  frmer;          /* Rx framing error counter */
    unsigned short  nosec;          /* Rx noise counter */
    unsigned short  brkec;          /* Rx break character counter */
    unsigned short  brkln;          /* Reaceive break length */

    unsigned short  uaddr1;         /* address character 1 */
    unsigned short  uaddr2;         /* address character 2 */
    unsigned short  rtemp;          /* temp storage */
    unsigned short  toseq;          /* Tx out of sequence char */
    unsigned short  cc[8];          /* Rx control characters */
    unsigned short  rccm;           /* Rx control char mask */
    unsigned short  rccr;           /* Rx control char register */
    unsigned short  rlbc;           /* Receive last break char */
};



/*****************************************************************
        BISYNC parameter RAM
*****************************************************************/

struct bisync_pram
{
 /*
 * SCC parameter RAM
 */
    unsigned short  rbase;          /* RX BD base address */
    unsigned short  tbase;          /* TX BD base address */
    unsigned char   rfcr;           /* Rx function code */
    unsigned char   tfcr;           /* Tx function code */
    unsigned short  mrblr;          /* Rx buffer length */
    unsigned long   rstate;         /* Rx internal state */
    unsigned long   rptr;           /* Rx internal data pointer */
    unsigned short  rbptr;          /* rb BD Pointer */
    unsigned short  rcount;         /* Rx internal byte count */
    unsigned long   rtemp;          /* Rx temp */
    unsigned long   tstate;         /* Tx internal state */
    unsigned long   tptr;           /* Tx internal data pointer */
    unsigned short  tbptr;          /* Tx BD pointer */
    unsigned short  tcount;         /* Tx byte count */
    unsigned long   ttemp;          /* Tx temp */
    unsigned long   rcrc;           /* temp receive CRC */
    unsigned long   tcrc;           /* temp transmit CRC */

/*
 * BISYNC specific parameter RAM
 */
    unsigned char   RESERVED1[4];   /* Reserved area */
    unsigned long   crcc;           /* CRC Constant Temp Value */
    unsigned short  prcrc;          /* Preset Receiver CRC-16/LRC */
    unsigned short  ptcrc;          /* Preset Transmitter CRC-16/LRC */
    unsigned short  parec;          /* Receive Parity Error Counter */
    unsigned short  bsync;          /* BISYNC SYNC Character */
    unsigned short  bdle;           /* BISYNC DLE Character */
    unsigned short  cc[8];          /* Rx control characters */
    unsigned short  rccm;           /* Receive Control Character Mask */
};

/*****************************************************************
        IOM2 parameter RAM
        (overlaid on tx bd[5] of SCC channel[2])
*****************************************************************/
struct iom2_pram
{
    unsigned short  ci_data;        /* ci data */
    unsigned short  monitor_data;   /* monitor data */
    unsigned short  tstate;         /* transmitter state */
    unsigned short  rstate;         /* receiver state */
};

/*****************************************************************
        SPI/SMC parameter RAM
        (overlaid on tx bd[6,7] of SCC channel[2])
*****************************************************************/

#define SPI_R   0x8000          /* Ready bit in BD */

struct spi_pram
{
    unsigned short  rbase;          /* Rx BD Base Address */
    unsigned short  tbase;          /* Tx BD Base Address */
    unsigned char   rfcr;           /* Rx function code */
    unsigned char   tfcr;           /* Tx function code */
    unsigned short  mrblr;          /* Rx buffer length */
    unsigned long   rstate;         /* Rx internal state */
    unsigned long   rptr;           /* Rx internal data pointer */
    unsigned short  rbptr;          /* rb BD Pointer */
    unsigned short  rcount;         /* Rx internal byte count */
    unsigned long   rtemp;          /* Rx temp */
    unsigned long   tstate;         /* Tx internal state */
    unsigned long   tptr;           /* Tx internal data pointer */
    unsigned short  tbptr;          /* Tx BD pointer */
    unsigned short  tcount;         /* Tx byte count */
    unsigned long   ttemp;          /* Tx temp */
};

struct smc_uart_pram {
    unsigned short  rbase;          /* Rx BD Base Address */
    unsigned short  tbase;          /* Tx BD Base Address */
    unsigned char   rfcr;           /* Rx function code */
    unsigned char   tfcr;           /* Tx function code */
    unsigned short  mrblr;          /* Rx buffer length */
    unsigned long   rstate;         /* Rx internal state */
    unsigned long   rptr;           /* Rx internal data pointer */
    unsigned short  rbptr;          /* rb BD Pointer */
    unsigned short  rcount;         /* Rx internal byte count */
    unsigned long   rtemp;          /* Rx temp */
    unsigned long   tstate;         /* Tx internal state */
    unsigned long   tptr;           /* Tx internal data pointer */
    unsigned short  tbptr;          /* Tx BD pointer */
    unsigned short  tcount;         /* Tx byte count */
    unsigned long   ttemp;          /* Tx temp */
    unsigned short  max_idl;        /* Maximum IDLE Characters */
    unsigned short  idlc;           /* Temporary IDLE Counter */
    unsigned short  brkln;          /* Last Rx Break Length */
    unsigned short  brkec;          /* Rx Break Condition Counter */
    unsigned short  brkcr;          /* Break Count Register (Tx) */
    unsigned short  r_mask;         /* Temporary bit mask */
};

struct smc_trnsp_pram {
    unsigned short  rbase;          /* Rx BD Base Address */
    unsigned short  tbase;          /* Tx BD Base Address */
    unsigned char   rfcr;           /* Rx function code */
    unsigned char   tfcr;           /* Tx function code */
    unsigned short  mrblr;          /* Rx buffer length */
    unsigned long   rstate;         /* Rx internal state */
    unsigned long   rptr;           /* Rx internal data pointer */
    unsigned short  rbptr;          /* rb BD Pointer */
    unsigned short  rcount;         /* Rx internal byte count */
    unsigned long   rtemp;          /* Rx temp */
    unsigned long   tstate;         /* Tx internal state */
    unsigned long   tptr;           /* Tx internal data pointer */
    unsigned short  tbptr;          /* Tx BD pointer */
    unsigned short  tcount;         /* Tx byte count */
    unsigned long   ttemp;          /* Tx temp */
    unsigned short  reserved[5];    /* Reserved */
};

struct centronics_pram
{
    unsigned short  rbase;          /* Rx BD Base Address */
    unsigned short  tbase;          /* Tx BD Base Address */
    unsigned char   fcr;            /* function code */
    unsigned char   smask;          /* Status Mask */
    unsigned short  mrblr;          /* Rx buffer length */
    unsigned long   rstate;         /* Rx internal state */
    unsigned long   rptr;           /* Rx internal data pointer */
    unsigned short  rbptr;          /* rb BD Pointer */
    unsigned short  rcount;         /* Rx internal byte count */
    unsigned long   rtemp;          /* Rx temp */
    unsigned long   tstate;         /* Tx internal state */
    unsigned long   tptr;           /* Tx internal data pointer */
    unsigned short  tbptr;          /* Tx BD pointer */
    unsigned short  tcount;         /* Tx byte count */
    unsigned long   ttemp;          /* Tx temp */
    unsigned short  max_sl;         /* Maximum Silence period */
    unsigned short  sl_cnt;         /* Silence Counter */
    unsigned short  char1;          /* CONTROL char 1 */
    unsigned short  char2;          /* CONTROL char 2 */
    unsigned short  char3;          /* CONTROL char 3 */
    unsigned short  char4;          /* CONTROL char 4 */
    unsigned short  char5;          /* CONTROL char 5 */
    unsigned short  char6;          /* CONTROL char 6 */
    unsigned short  char7;          /* CONTROL char 7 */
    unsigned short  char8;          /* CONTROL char 8 */
    unsigned short  rccm;           /* Rx Control Char Mask */
    unsigned short  rccr;           /* Rx Char Control Register */
};

struct idma_pram
{
    unsigned short  ibase;          /* IDMA BD Base Address */
    unsigned short  ibptr;  /* IDMA buffer descriptor pointer */
    unsigned long   istate; /* IDMA internal state */
    unsigned long   itemp;  /* IDMA temp */
};

struct ethernet_pram
{
/*
 * SCC parameter RAM
 */
    unsigned short  rbase;          /* RX BD base address */
    unsigned short  tbase;          /* TX BD base address */
    unsigned char   rfcr;           /* Rx function code */
    unsigned char   tfcr;           /* Tx function code */
    unsigned short  mrblr;          /* Rx buffer length */
    unsigned long   rstate;         /* Rx internal state */
    unsigned long   rptr;           /* Rx internal data pointer */
    unsigned short  rbptr;          /* rb BD Pointer */
    unsigned short  rcount;         /* Rx internal byte count */
    unsigned long   rtemp;          /* Rx temp */
    unsigned long   tstate;         /* Tx internal state */
    unsigned long   tptr;           /* Tx internal data pointer */
    unsigned short  tbptr;          /* Tx BD pointer */
    unsigned short  tcount;         /* Tx byte count */
    unsigned long   ttemp;          /* Tx temp */
    unsigned long   rcrc;           /* temp receive CRC */
    unsigned long   tcrc;           /* temp transmit CRC */

/*
 * ETHERNET specific parameter RAM
 */
    unsigned long   c_pres;         /* preset CRC */
    unsigned long   c_mask;         /* constant mask for CRC */
    unsigned long   crcec;          /* CRC error counter */
    unsigned long   alec;           /* alighnment error counter */
    unsigned long   disfc;          /* discard frame counter */
    unsigned short  pads;           /* short frame PAD characters */
    unsigned short  ret_lim;        /* retry limit threshold */
    unsigned short  ret_cnt;        /* retry limit counter */
    unsigned short  mflr;           /* maximum frame length reg */
    unsigned short  minflr;         /* minimum frame length reg */
    unsigned short  maxd1;          /* maximum DMA1 length reg */
    unsigned short  maxd2;          /* maximum DMA2 length reg */
    unsigned short  maxd;           /* rx max DMA */
    unsigned short  dma_cnt;        /* rx dma counter */
    unsigned short  max_b;          /* max bd byte count */
    unsigned short  gaddr1;         /* group address filter 1 */
    unsigned short  gaddr2;         /* group address filter 2 */
    unsigned short  gaddr3;         /* group address filter 3 */
    unsigned short  gaddr4;         /* group address filter 4 */
    unsigned long   tbuf0_data0;    /* save area 0 - current frm */
    unsigned long   tbuf0_data1;    /* save area 1 - current frm */
    unsigned long   tbuf0_rba0;
    unsigned long   tbuf0_crc;
    unsigned short  tbuf0_bcnt;
    unsigned short  paddr_h;        /* physical address (MSB) */
    unsigned short  paddr_m;        /* physical address */
    unsigned short  paddr_l;        /* physical address (LSB) */
    unsigned short  p_per;          /* persistence */
    unsigned short  rfbd_ptr;       /* rx first bd pointer */
    unsigned short  tfbd_ptr;       /* tx first bd pointer */
    unsigned short  tlbd_ptr;       /* tx last bd pointer */
    unsigned long   tbuf1_data0;    /* save area 0 - next frame */
    unsigned long   tbuf1_data1;    /* save area 1 - next frame */
    unsigned long   tbuf1_rba0;
    unsigned long   tbuf1_crc;
    unsigned short  tbuf1_bcnt;
    unsigned short  tx_len;         /* tx frame length counter */
    unsigned short  iaddr1;         /* individual address filter 1*/
    unsigned short  iaddr2;         /* individual address filter 2*/
    unsigned short  iaddr3;         /* individual address filter 3*/
    unsigned short  iaddr4;         /* individual address filter 4*/
    unsigned short  boff_cnt;       /* back-off counter */
    unsigned short  taddr_h;        /* temp address (MSB) */
    unsigned short  taddr_m;        /* temp address */
    unsigned short  taddr_l;        /* temp address (LSB) */
};

struct transparent_pram
{
/*
 * SCC parameter RAM
 */
    unsigned short  rbase;          /* RX BD base address */
    unsigned short  tbase;          /* TX BD base address */
    unsigned char   rfcr;           /* Rx function code */
    unsigned char   tfcr;           /* Tx function code */
    unsigned short  mrblr;          /* Rx buffer length */
    unsigned long   rstate;         /* Rx internal state */
    unsigned long   rptr;           /* Rx internal data pointer */
    unsigned short  rbptr;          /* rb BD Pointer */
    unsigned short  rcount;         /* Rx internal byte count */
    unsigned long   rtemp;          /* Rx temp */
    unsigned long   tstate;         /* Tx internal state */
    unsigned long   tptr;           /* Tx internal data pointer */
    unsigned short  tbptr;          /* Tx BD pointer */
    unsigned short  tcount;         /* Tx byte count */
    unsigned long   ttemp;          /* Tx temp */
    unsigned long   rcrc;           /* temp receive CRC */
    unsigned long   tcrc;           /* temp transmit CRC */

/*
 * TRANSPARENT specific parameter RAM
 */
    unsigned long   crc_p;          /* CRC Preset */
    unsigned long   crc_c;          /* CRC constant */
};

struct timer_pram
{
/*
 * RISC timers parameter RAM
 */
    unsigned short  tm_base;        /* RISC timer table base adr */
    unsigned short  tm_ptr;         /* RISC timer table pointer */
    unsigned short  r_tmr;          /* RISC timer mode register */
    unsigned short  r_tmv;          /* RISC timer valid register */
    unsigned long   tm_cmd;         /* RISC timer cmd register */
    unsigned long   tm_cnt;         /* RISC timer internal cnt */
};


struct i2c_pram
{
/*
 * I2C parameter RAM
 */
    unsigned short  rbase;          /* RX BD base address */
    unsigned short  tbase;          /* TX BD base address */
    unsigned char   rfcr;           /* Rx function code */
    unsigned char   tfcr;           /* Tx function code */
    unsigned short  mrblr;          /* Rx buffer length */
    unsigned long   rstate;         /* Rx internal state */
    unsigned long   rptr;           /* Rx internal data pointer */
    unsigned short  rbptr;          /* rb BD Pointer */
    unsigned short  rcount;         /* Rx internal byte count */
    unsigned long   rtemp;          /* Rx temp */
    unsigned long   tstate;         /* Tx internal state */
    unsigned long   tptr;           /* Tx internal data pointer */
    unsigned short  tbptr;          /* Tx BD pointer */
    unsigned short  tcount;         /* Tx byte count */
    unsigned long   ttemp;          /* Tx temp */
};

struct scc_regs 
{
   unsigned long    scc_gsmrl;    /* SCC Gen mode (LOW) */
   unsigned long    scc_gsmrh;    /* SCC Gen mode (HIGH) */
   unsigned short   scc_psmr;     /* protocol specific mode register */
   unsigned char    RESERVED29[0x2];        /* Reserved area */
   unsigned short   scc_todr;     /* SCC transmit on demand */
   unsigned short   scc_dsr;      /* SCC data sync reg */
   unsigned short   scc_scce;     /* SCC event reg */
   unsigned char    RESERVED30[0x2];        /* Reserved area */
   unsigned short   scc_sccm;     /* SCC mask reg */
   unsigned char    RESERVED31[0x1];        /* Reserved area */
   unsigned char    scc_sccs;     /* SCC status reg */
   unsigned char    RESERVED32[0x8];        /* Reserved area */

};


/************************************************************************
*
*                     MPC860 MEMORY MAP
*
************************************************************************/
typedef struct tsMpc860
{
/*-----------------------------------*/
/* BASE + 0x0000: INTERNAL REGISTERS */
/*-----------------------------------*/

/*-----*/
/* SIU */
/*-----*/
   
   unsigned long    siu_mcr;     /* module configuration reg */
   unsigned long    siu_sypcr;   /* System protection cnt */
   unsigned char    RESERVED58[0x6];
   unsigned short   siu_swsr;    /* sw service */
   unsigned long    siu_sipend;  /* Interrupt pend reg */
   unsigned long    siu_simask;  /* Interrupt mask reg */
   unsigned long    siu_siel;    /* Interrupt edge level mask reg */
   unsigned long    siu_sivec;   /* Interrupt vector */
   unsigned long    siu_tesr;    /* Transfer error status */
   unsigned char    RESERVED1[0xc];   /* Reserved area */
   unsigned long    sdma_sdcr;   /* SDMA configuration reg */
   unsigned char    RESERVED55[0x4c];

/*--------*/
/* PCMCIA */
/*--------*/

   unsigned long    pcmcia_pbr0;      /* PCMCIA Base Reg: Window 0 */
   unsigned long    pcmcia_por0;      /* PCMCIA Option Reg: Window 0 */
   unsigned long    pcmcia_pbr1;      /* PCMCIA Base Reg: Window 1 */
   unsigned long    pcmcia_por1;      /* PCMCIA Option Reg: Window 1 */
   unsigned long    pcmcia_pbr2;      /* PCMCIA Base Reg: Window 2 */
   unsigned long    pcmcia_por2;      /* PCMCIA Option Reg: Window 2 */
   unsigned long    pcmcia_pbr3;      /* PCMCIA Base Reg: Window 3 */
   unsigned long    pcmcia_por3;      /* PCMCIA Option Reg: Window 3 */
   unsigned long    pcmcia_pbr4;      /* PCMCIA Base Reg: Window 4 */
   unsigned long    pcmcia_por4;      /* PCMCIA Option Reg: Window 4 */
   unsigned long    pcmcia_pbr5;      /* PCMCIA Base Reg: Window 5 */
   unsigned long    pcmcia_por5;      /* PCMCIA Option Reg: Window 5 */
   unsigned long    pcmcia_pbr6;      /* PCMCIA Base Reg: Window 6 */
   unsigned long    pcmcia_por6;      /* PCMCIA Option Reg: Window 6 */
   unsigned long    pcmcia_pbr7;      /* PCMCIA Base Reg: Window 7 */
   unsigned long    pcmcia_por7;      /* PCMCIA Option Reg: Window 7 */
   unsigned char    RESERVED2[0x20];  /* Reserved area */
   unsigned long    pcmcia_pgcra;     /* PCMCIA Slot A Control  Reg */
   unsigned long    pcmcia_pgcrb;     /* PCMCIA Slot B Control  Reg */
   unsigned long    pcmcia_pscr;      /* PCMCIA Status Reg */
   unsigned char    RESERVED2a[0x4];  /* Reserved area */
   unsigned long    pcmcia_pipr;      /* PCMCIA Pins Value Reg */
   unsigned char    RESERVED2b[0x4];  /* Reserved area */
   unsigned long    pcmcia_per;       /* PCMCIA Enable Reg */
   unsigned char    RESERVED2c[0x4];  /* Reserved area */

/*------*/
/* MEMC */
/*------*/

   unsigned long    memc_br0;    /* base register 0 */
   unsigned long    memc_or0;    /* option register 0 */
   unsigned long    memc_br1;    /* base register 1 */
   unsigned long    memc_or1;    /* option register 1 */
   unsigned long    memc_br2;    /* base register 2 */
   unsigned long    memc_or2;    /* option register 2 */
   unsigned long    memc_br3;    /* base register 3 */
   unsigned long    memc_or3;    /* option register 3 */
   unsigned long    memc_br4;    /* base register 3 */
   unsigned long    memc_or4;    /* option register 3 */
   unsigned long    memc_br5;    /* base register 3 */
   unsigned long    memc_or5;    /* option register 3 */
   unsigned long    memc_br6;    /* base register 3 */
   unsigned long    memc_or6;    /* option register 3 */
   unsigned long    memc_br7;    /* base register 3 */
   unsigned long    memc_or7;    /* option register 3 */
   unsigned char    RESERVED3[0x24];        /* Reserved area */
   unsigned long    memc_mar;    /* Memory address */
   unsigned long    memc_mcr;    /* Memory command */
   unsigned char    RESERVED4[0x4];     /* Reserved area */
   unsigned long    memc_mamr;   /* Machine A mode */
   unsigned long    memc_mbmr;   /* Machine B mode */
   unsigned short   memc_mstat;  /* Memory status */
   unsigned short   memc_mptpr;  /* Memory preidic timer prescalar */
   unsigned long    memc_mdr;    /* Memory data */
   unsigned char    RESERVED5[0x80];        /* Reserved area */

/*---------------------------*/
/* SYSTEM INTEGRATION TIMERS */
/*---------------------------*/

   unsigned short   simt_tbscr;       /* Time base stat&ctr */
   unsigned char    RESERVED100[0x2];       /* Reserved area */
   unsigned long    simt_tbreff0;     /* Time base reference 0 */
   unsigned long    simt_tbreff1;     /* Time base reference 1 */
   unsigned char    RESERVED6[0x14];        /* Reserved area */
   unsigned short   simt_rtcsc;       /* Realtime clk stat&cntr 1 */
   unsigned char    RESERVED110[0x2];       /* Reserved area */
   unsigned long    simt_rtc;         /* Realtime clock */
   unsigned long    simt_rtsec;       /* Realtime alarm seconds */
   unsigned long    simt_rtcal;       /* Realtime alarm */
   unsigned char    RESERVED56[0x10];       /* Reserved area */
   unsigned long    simt_piscr;       /* PIT stat&ctrl */
   unsigned long    simt_pitc;        /* PIT counter */
   unsigned long    simt_pitr;        /* PIT */
   unsigned char    RESERVED7[0x34];        /* Reserved area */

/*---------------*/
/* CLOCKS, RESET */
/*---------------*/
   
   unsigned long    clkr_sccr;        /* System clk cntrl */
   unsigned long    clkr_plprcr;      /* PLL reset&ctrl */
   unsigned long    clkr_rsr;         /* reset status */
   unsigned char    RESERVED8a[0x74];           /* Reserved area */

/*--------------------------------*/
/* System Integration Timers Keys */
/*--------------------------------*/

   unsigned long    simt_tbscrk;      /* Timebase Status&Ctrl Key */
   unsigned long    simt_tbreff0k;    /* Timebase Reference 0 Key */
   unsigned long    simt_tbreff1k;    /* Timebase Reference 1 Key */
   unsigned long    simt_tbk;         /* Timebase and Decrementer Key */
   unsigned char    RESERVED66b[0x10];          /* Reserved area */
   unsigned long    simt_rtcsck;      /* Real-Time Clock Status&Ctrl Key */

   unsigned long    simt_rtck;        /* Real-Time Clock Key */
   unsigned long    simt_rtseck;      /* Real-Time Alarm Seconds Key */
   unsigned long    simt_rtcalk;      /* Real-Time Alarm Key */
   unsigned char    RESERVED66c[0x10];          /* Reserved area */
   unsigned long    simt_piscrk;      /* Periodic Interrupt Status&Ctrl Key */
   unsigned long    simt_pitck;       /* Periodic Interrupt Count Key */
   unsigned char    RESERVED66d[0x38];          /* Reserved area */
        
/*----------------------*/
/* Clock and Reset Keys */
/*----------------------*/

   unsigned long    clkr_sccrk;   /* System Clock Control Key */
   unsigned long    clkr_plprcrk; /* PLL, Low Power and Reset Control Key */
   unsigned long    clkr_rsrk;    /* Reset Status Key */
   unsigned char    RESERVED66e[0x4d4];         /* Reserved area */
              
/*-----*/
/* I2C */
/*-----*/
   
   unsigned char    i2c_i2mod;    /* i2c mode */
   unsigned char    RESERVED59[3];
   unsigned char    i2c_i2add;    /* i2c address */
   unsigned char    RESERVED60[3];
   unsigned char    i2c_i2brg;    /* i2c brg */
   unsigned char    RESERVED61[3];
   unsigned char    i2c_i2com;    /* i2c command */
   unsigned char    RESERVED62[3];
   unsigned char    i2c_i2cer;    /* i2c event */
   unsigned char    RESERVED63[3];
   unsigned char    i2c_i2cmr;    /* i2c mask */
   unsigned char    RESERVED10[0x8b];       /* Reserved area */

/*-----*/
/* DMA */
/*-----*/

   unsigned char    RESERVED11[0x4];        /* Reserved area */
   unsigned long    dma_sdar;     /* SDMA address reg */
   unsigned char    RESERVED12[0x2];        /* Reserved area */
   unsigned char    dma_sdsr;     /* SDMA status reg */
   unsigned char    RESERVED13[0x3];        /* Reserved area */
   unsigned char    dma_sdmr;     /* SDMA mask reg */
   unsigned char    RESERVED14[0x1];        /* Reserved area */                        
   unsigned char    dma_idsr1;    /* IDMA1 status reg */
   unsigned char    RESERVED15[0x3];        /* Reserved area */
   unsigned char    dma_idmr1;    /* IDMA1 mask reg */
   unsigned char    RESERVED16[0x3];        /* Reserved area */
   unsigned char    dma_idsr2;    /* IDMA2 status reg */
   unsigned char    RESERVED17[0x3];        /* Reserved area */
   unsigned char    dma_idmr2;    /* IDMA2 mask reg */
   unsigned char    RESERVED18[0x13];       /* Reserved area */

/*--------------------------*/
/* CPM Interrupt Controller */
/*--------------------------*/

   unsigned short   intr_civr;    /* CP interrupt vector reg */
   unsigned char    RESERVED19[0xe];        /* Reserved area */
   unsigned long    intr_cicr;    /* CP interrupt configuration reg */
   unsigned long    intr_cipr;    /* CP interrupt pending reg */
   unsigned long    intr_cimr;    /* CP interrupt mask reg */
   unsigned long    intr_cisr;    /* CP interrupt in-service reg */

/*----------*/
/* I/O port */
/*----------*/

   unsigned short   pio_padir;    /* port A data direction reg */
   unsigned short   pio_papar;    /* port A pin assignment reg */
   unsigned short   pio_paodr;    /* port A open drain reg */
   unsigned short   pio_padat;    /* port A data register */
   unsigned char    RESERVED20[0x8];        /* Reserved area */
   unsigned short   pio_pcdir;    /* port C data direction reg */
   unsigned short   pio_pcpar;    /* port C pin assignment reg */
   unsigned short   pio_pcso;     /* port C special options */
   unsigned short   pio_pcdat;    /* port C data register */
   unsigned short   pio_pcint;    /* port C interrupt cntrl reg */
   unsigned char    RESERVED64[6];
   unsigned short   pio_pddir;    /* port D Data Direction reg */
   unsigned short   pio_pdpar;    /* port D pin assignment reg */
   unsigned char    RESERVED65[2];
   unsigned short   pio_pddat;    /* port D data reg */
   unsigned char    RESERVED21[0x8];        /* Reserved area */

/*-----------*/
/* CPM Timer */
/*-----------*/

   unsigned short   timer_tgcr;   /* timer global configuration reg                                                               */
   unsigned char    RESERVED22[0xe];        /* Reserved area */
   unsigned short   timer_tmr1;   /* timer 1 mode reg */
   unsigned short   timer_tmr2;   /* timer 2 mode reg */
   unsigned short   timer_trr1;   /* timer 1 referance reg */
   unsigned short   timer_trr2;   /* timer 2 referance reg */
   unsigned short   timer_tcr1;   /* timer 1 capture reg */
   unsigned short   timer_tcr2;   /* timer 2 capture reg */
   unsigned short   timer_tcn1;   /* timer 1 counter reg */
   unsigned short   timer_tcn2;   /* timer 2 counter reg */
   unsigned short   timer_tmr3;   /* timer 3 mode reg */
   unsigned short   timer_tmr4;   /* timer 4 mode reg */
   unsigned short   timer_trr3;   /* timer 3 referance reg */
   unsigned short   timer_trr4;   /* timer 4 referance reg */
   unsigned short   timer_tcr3;   /* timer 3 capture reg */
   unsigned short   timer_tcr4;   /* timer 4 capture reg */
   unsigned short   timer_tcn3;   /* timer 3 counter reg */
   unsigned short   timer_tcn4;   /* timer 4 counter reg */
   unsigned short   timer_ter1;   /* timer 1 event reg */
   unsigned short   timer_ter2;   /* timer 2 event reg */
   unsigned short   timer_ter3;   /* timer 3 event reg */
   unsigned short   timer_ter4;   /* timer 4 event reg */
   unsigned char    RESERVED23[0x8];        /* Reserved area */

/*----*/
/* CP */
/*----*/

   unsigned short   cp_cr;        /* command register */
   unsigned char    RESERVED24[0x2];        /* Reserved area */
   unsigned short   cp_rccr;      /* main configuration reg */
   unsigned char    RESERVED25;                     /* Reserved area */
   unsigned char    cp_resv1;                       /* Reserved reg */
   unsigned long    cp_resv2;                       /* Reserved reg */
   unsigned short   cp_rctr1;     /* ram break register 1 */
   unsigned short   cp_rctr2;     /* ram break register 2 */
   unsigned short   cp_rctr3;     /* ram break register 3 */
   unsigned short   cp_rctr4;     /* ram break register 4 */
   unsigned char    RESERVED26[0x2];        /* Reserved area */
   unsigned short   cp_rter;      /* RISC timers event reg */
   unsigned char    RESERVED27[0x2];        /* Reserved area */
   unsigned short   cp_rtmr;      /* RISC timers mask reg */
   unsigned char    RESERVED28[0x14];       /* Reserved area */

/*-----*/
/* BRG */
/*-----*/

   unsigned long    brgc1;        /* BRG1 configuration reg */
   unsigned long    brgc2;        /* BRG2 configuration reg */
   unsigned long    brgc3;        /* BRG3 configuration reg */
   unsigned long    brgc4;        /* BRG4 configuration reg */

/*---------------*/
/* SCC registers */
/*---------------*/

   struct scc_regs  scc_regs[4];
   
/*-----*/
/* SMC */
/*-----*/

   struct smc_regs 
   
   {
       unsigned char    RESERVED34[0x2];        /* Reserved area */
       unsigned short   smc_smcmr;    /* SMC mode reg */
       unsigned char    RESERVED35[0x2];        /* Reserved area */
       unsigned char    smc_smce;     /* SMC event reg */
       unsigned char    RESERVED36[0x3];        /* Reserved area */
       unsigned char    smc_smcm;     /* SMC mask reg */
       unsigned char    RESERVED37[0x5];        /* Reserved area */

   } smc_regs[2];


/*-----*/
/* SPI */
/*-----*/

   unsigned short   spi_spmode;   /* SPI mode reg */
   unsigned char    RESERVED38[0x4];        /* Reserved area */
   unsigned char    spi_spie;     /* SPI event reg */
   unsigned char    RESERVED39[0x3];        /* Reserved area */
   unsigned char    spi_spim;     /* SPI mask reg */
   unsigned char    RESERVED40[0x2];        /* Reserved area */
   unsigned char    spi_spcom;    /* SPI command reg */
   unsigned char    RESERVED41[0x4];        /* Reserved area */

/*-----*/
/* PIP */
/*-----*/

   unsigned short   pip_pipc;     /* pip configuration reg */
   unsigned char    RESERVED42[0x2];        /* Reserved area */
   unsigned short   pip_ptpr;     /* pip timing parameters reg */
   unsigned long    pip_pbdir;    /* port b data direction reg */
   unsigned long    pip_pbpar;    /* port b pin assignment reg */
   unsigned char    RESERVED43[0x2];        /* Reserved area */
   unsigned short   pip_pbodr;    /* port b open drain reg */
   unsigned long    pip_pbdat;    /* port b data reg */
   unsigned char    RESERVED44[0x18];       /* Reserved area */


/*------------------*/
/* Serial Interface */
/*------------------*/

   unsigned long    si_simode;      /* SI mode register */
   unsigned char    si_sigmr;       /* SI global mode register */
   unsigned char    RESERVED45;                     /* Reserved area */
   unsigned char    si_sistr;       /* SI status register */
   unsigned char    si_sicmr;       /* SI command register */
   unsigned char    RESERVED46[0x4];                /* Reserved area */
   unsigned long    si_sicr;        /* SI clock routing */
   unsigned long    si_sirp;        /* SI ram pointers */
   unsigned char    RESERVED47[0x10c];      /* Reserved area */
   unsigned char    si_siram[0x200]; /* SI routing ram */
/*   unsigned char    RESERVED48[0x1200]; Reserved area  (now used for 860T) */

/*-------------------------*/
/* 860T-Specific Registers */
/*-------------------------*/
   unsigned long   t_addr_low;                  /* lower 32-bits of station address */
   unsigned long   t_addr_high;                 /* upper 16-bits of station address */
   unsigned long   t_hash_table_high;           /* upper 32-bits of hash table */
   unsigned long   t_hash_table_low;            /* lower 32-bits of hash table */
   unsigned long   t_r_des_start;                       /* beginning of rx descriptor ring */
   unsigned long   t_x_des_start;               /* beginning of tx descriptor ring */
   unsigned long   t_r_buff_size;               /* rx buffer size */
   unsigned char   RESERVED76a[0x24];           /* Reserved area */
   unsigned long   t_ecntrl;                    /* ethernet control register */
   unsigned long   t_ievent;                    /* interrupt event register */
   unsigned long   t_imask;                     /* interrupt mask register */
   unsigned long   t_ivec;                      /* int level and vector status */
   unsigned long   t_r_des_active;              /* rx ring updated flag */
   unsigned long   t_x_des_active;              /* tx ring updated flag */
   unsigned char   RESERVED76b[0x28];           /* Reserved area */
   unsigned long   t_mii_data;                  /* mii data register */
   unsigned long   t_mii_speed;                 /* mii speed control register */
   unsigned char   RESERVED76c[0x44];           /* Reserved area */
   unsigned long   t_r_bound;                   /* end of RAM (read-only) */
   unsigned long   t_r_fstart;                  /* rx FIFO start address */
   unsigned char   RESERVED76d[0x18];           /* Reserved area */
   unsigned long   t_x_fstart;                  /* tx FIFO start address */
   unsigned char   RESERVED76e[0x44];           /* Reserved area */
   unsigned long   t_fun_code;                  /* fec SDMA function code */
   unsigned char   RESERVED76f[0x0C];           /* Reserved area */
   unsigned long   t_r_cntrl;               /* rx control register */
   unsigned long   t_r_hash;                    /* rx hash register */
   unsigned char   RESERVED76g[0x38];           /* Reserved area */
   unsigned long   t_x_cntrl;                   /* tx control register */
   unsigned char   RESERVED76h[0x78];           /* Reserved area */
   unsigned char   RESERVED77[0x1000];

/*---------------------------------*/
/* BASE + 0x2000: user data memory */
/*---------------------------------*/

/* user data bd's or Ucode (small)  */
   unsigned char    udata_bd_ucode[0x200];
/* user data bd's or Ucode (medium) */
   unsigned char    udata_bd_ucode2[0x200];
/* user data bd's or Ucode (large)  */
   unsigned char    udata_bd_ucode3[0x400];
   unsigned char    udata_bd[0x700];    /* user data bd's*/
   unsigned char    ucode_ext[0x100];   /* Ucode Extension ram*/
   unsigned char    RESERVED49[0x0c00];     /* Reserved area */


/*-----------------------------------------------------------------------*/
/* BASE + 0x3c00: PARAMETER RAM. This main union defines 4 memory blocks */
/* of an identical size. See the Parameter RAM definition in the MPC860  */
/* user's manual.                                                        */
/*-----------------------------------------------------------------------*/

/*------------------------*/
/* Base + 0x3C00 (page 1) */
/*      + 0x3D00 (page 2) */
/*      + 0x3E00 (page 3) */
/*      + 0x3F00 (page 4) */
/*------------------------*/

   union 
      
   {
      struct page_of_pram 
       
      {
/*------------------------------------------------------------*/
/* scc parameter area - 1st memory block (protocol dependent) */
/*------------------------------------------------------------*/

         union 
           
         {
            struct hdlc_pram         h;
            struct uart_pram         u;
            struct bisync_pram       b;
            struct transparent_pram  t;
            struct async_hdlc_pram   a;
            unsigned char            RESERVED50[0x80];

         } pscc;                

/*----------------------------------------------------------------*/
/* Other protocol areas for the rest of the memory blocks in each */
/* page.                                                          */
/*----------------------------------------------------------------*/

         union 
                 
         {
/*---------------------------------------------------------------*/
/* This structure defines the rest of the blocks on the 1st page */
/*---------------------------------------------------------------*/ 
         
            struct 
         
            {
               struct i2c_pram  i2c;     /* I2C   */
               struct idma_pram idma1;   /* IDMA1 */

            } i2c_idma;

/*---------------------------------------------------------------*/
/* This structure defines the rest of the blocks on the 2nd page */
/*---------------------------------------------------------------*/
 
            struct 
         
            {
               struct spi_pram      spi;     /* SPI    */
               struct timer_pram    timer;   /* Timers */
               struct idma_pram     idma2;   /* IDMA2  */

            } spi_timer_idma;

/*---------------------------------------------------------------*/
/* This structure defines the rest of the blocks on the 3rd page */
/*---------------------------------------------------------------*/
 
            struct 
         
            {
               union 
          
               {
                  struct smc_uart_pram  u1;   /* SMC1 */
                  struct smc_trnsp_pram t1;  /* SMC1 */
/* declare full block */
                  unsigned char         RESERVED58[0x40];

               } psmc1;

               unsigned char    dsp1_param1[0x40];

            } smc_dsp1;


/*---------------------------------------------------------------*/
/* This structure defines the rest of the blocks on the 4th page */
/*---------------------------------------------------------------*/ 
 
            struct 
         
            {
               union 
          
               {
                  struct smc_uart_pram   u2;   /* SMC2 */
                  struct smc_trnsp_pram  t2;  /* SMC2 */
                  struct centronics_pram c;  /* Uses SM2's space */
/* declare full block */
                  unsigned char          RESERVED59[0x40];

               } psmc2;

               unsigned char    dsp2_param[0x40];

            } smc_dsp2; 

            unsigned char       RESERVED56[0x80];    /* declare full block */

         } pothers;

      } scc;

/*---------------------------------------------------------------*/
/* When selecting Ethernet as protocol for an SCC, this protocol */
/* uses a complete page of Parameter RAM memory.                 */
/*---------------------------------------------------------------*/

      struct ethernet_pram      enet_scc; 

/*--------------------------------------------------------*/
/* declaration to guarantee a page of memory is allocated */
/*--------------------------------------------------------*/

      unsigned char   RESERVED60[0x100]; 

   } pram[4]; /* end of union */
} ttMpc860;

typedef ttMpc860 *ttMpc860Ptr;

/************************************************************************
*                       DUAL PORT RAM DEFINITIONS
************************************************************************/


#define pipe    smce2   /* pip event register at same address as smce2 */
#define pipm    smcm2   /* pip mask register at same address as smcm2 */

/************************************************************************
*                       SIPEND REGISTER - SIU INTERRUPT PENDING
************************************************************************/
#define SIPEND_LV7              0x00010000UL
#define SIPEND_IRQ7             0x00020000UL
#define SIPEND_LV6              0x00040000UL
#define SIPEND_IRQ6             0x00080000UL
#define SIPEND_LV5              0x00100000UL
#define SIPEND_IRQ5             0x00200000UL
#define SIPEND_LV4              0x00400000UL
#define SIPEND_IRQ4             0x00800000UL
#define SIPEND_LV3              0x01000000UL
#define SIPEND_IRQ3             0x02000000UL
#define SIPEND_LV2              0x04000000UL
#define SIPEND_IRQ2             0x08000000UL
#define SIPEND_LV1              0x10000000UL
#define SIPEND_IRQ1             0x20000000UL
#define SIPEND_LV0              0x40000000UL
#define SIPEND_IRQ0             0x80000000UL



/************************************************************************
*                               SIMASK REGISTER - SIU INTERRUPT MASK                                    *
************************************************************************/
#define SIMASK_LVM7             0x00010000UL
#define SIMASK_IRM7             0x00020000UL
#define SIMASK_LVM6             0x00040000UL
#define SIMASK_IRM6             0x00080000UL
#define SIMASK_LVM5             0x00100000UL
#define SIMASK_IRM5             0x00200000UL
#define SIMASK_LVM4             0x00400000UL
#define SIMASK_IRM4             0x00800000UL
#define SIMASK_LVM3             0x01000000UL
#define SIMASK_IRM3             0x02000000UL
#define SIMASK_LVM2             0x04000000UL
#define SIMASK_IRM2             0x08000000UL
#define SIMASK_LVM1             0x10000000UL
#define SIMASK_IRM1             0x20000000UL
#define SIMASK_LVM0             0x40000000UL
#define SIMASK_IRM0             0x80000000UL


/************************************************************************
*                     SIEL REGISTER - SIU INTERRUPT LEVEL
************************************************************************/
#define SIEL_WM7                0x00010000UL
#define SIEL_ED7                0x00020000UL
#define SIEL_WM6                0x00040000UL
#define SIEL_ED6                0x00080000UL
#define SIEL_WM5                0x00100000UL
#define SIEL_ED5                0x00200000UL
#define SIEL_WM4                0x00400000UL
#define SIEL_ED4                0x00800000UL
#define SIEL_WM3                0x01000000UL
#define SIEL_ED3                0x02000000UL
#define SIEL_WM2                0x04000000UL
#define SIEL_ED2                0x08000000UL
#define SIEL_WM1                0x10000000UL
#define SIEL_ED1                0x20000000UL
#define SIEL_WM0                0x40000000UL
#define SIEL_ED0                0x80000000UL


/************************************************************************
*                   SIVEC REGISTER - SIU INTERRUPT VECTOR CODE
************************************************************************/
#define SIVEC_LV7               0x3C000000UL   /* Level 7 */
#define SIVEC_IRQ7              0x38000000UL   /* IRQ 7 */
#define SIVEC_LV6               0x34000000UL   /* Level 6 */
#define SIVEC_IRQ6              0x30000000UL   /* IRQ 6 */
#define SIVEC_LV5               0x2C000000UL   /* Level 5 */
#define SIVEC_IRQ5              0x28000000UL   /* IRQ 5 */
#define SIVEC_LV4               0x24000000UL   /* Level 4 */
#define SIVEC_IRQ4              0x20000000UL   /* IRQ 4 */
#define SIVEC_LV3               0x1C000000UL   /* Level 3 */
#define SIVEC_IRQ3              0x18000000UL   /* IRQ 3 */
#define SIVEC_LV2               0x14000000UL   /* Level 2 */
#define SIVEC_IRQ2              0x10000000UL   /* IRQ 2 */
#define SIVEC_LV1               0x0C000000UL   /* Level 1 */
#define SIVEC_IRQ1              0x08000000UL   /* IRQ 1 */
#define SIVEC_LV0               0x04000000UL   /* Level 0 */
#define SIVEC_IRQ0              0x00000000UL   /* IRQ 0 */


/************************************************************************
*              PISCR REGISTER - PERIODIC INTERRUPT STATUS AND CONTROL
************************************************************************/
#define PISCR_PTE               0x00010000UL
#define PISCR_PITF              0x00020000UL
#define PISCR_PIE               0x00040000UL
#define PISCR_PS                0x00800000UL
#define PISCR_PIRQ7             0x01000000UL
#define PISCR_PIRQ6             0x02000000UL
#define PISCR_PIRQ5             0x04000000UL
#define PISCR_PIRQ4             0x08000000UL
#define PISCR_PIRQ3             0x10000000UL
#define PISCR_PIRQ2             0x20000000UL
#define PISCR_PIRQ1             0x40000000UL
#define PISCR_PIRQ0             0x80000000UL


/* Control character structure for UART */

typedef struct tsMpc860ControlChars {
    unsigned char reject;   /* reject the character */
    unsigned char ch;      /* control character */
} ttMpc860ControlChars;

#define TM_MPC860_NUM_CONTROL_CHARS 0

/*
ttMpc860ControlChars tlMpc860ControlChars[] = 
{
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0},
    {0,0}
};
*/


/*
 * Darwin specific macros 
 */

/* USI termination macros */
#define TM_DARWIN_USI_TERM_RS423   0x00
#define TM_DARWIN_USI_TERM_RS530A  0x10
#define TM_DARWIN_USI_TERM_X21     0x30
#define TM_DARWIN_USI_TERM_V35     0x40
#define TM_DARWIN_USI_TERM_RS530   0x50
#define TM_DARWIN_USI_TERM_RS232   0x60
#define TM_DARWIN_USI_TERM_V11     0x70

/* USI mode macros */
#define TM_DARWIN_USI_RS423        0x00
#define TM_DARWIN_USI_RS530A       0x01
#define TM_DARWIN_USI_X21          0x03
#define TM_DARWIN_USI_V35          0x04
#define TM_DARWIN_USI_RS530        0x05
#define TM_DARWIN_USI_RS232        0x06
#define TM_DARWIN_USI_V11          0x07

/* USI loopback macros */
#define TM_DARWIN_USI_NO_LOOPBACK  0x08

#ifdef __cplusplus
}
#endif

#endif /* _TRMPC860_H_ */
