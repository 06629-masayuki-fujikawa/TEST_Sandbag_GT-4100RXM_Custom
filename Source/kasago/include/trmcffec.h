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
 * Description: Header file for Motorola Coldfire FEC sample driver
 *
 * Work originally from Motorola development team
 * Since this file is a derived work, it remains the property of Motorola Corp.
 *
 * With modifications by Quadros Systems, Inc. as needed
 * modifications include:
 * - PITn offsets should be 0x10000 instead of 0x1000
 * - MCF5282_SCM_PACR_ACCESSCTRL{0|1}+ macros missing arg x
 * - additional macros for RAMBAR CPU register bit definitions
 * - correction: MCF5282_SCM_RAMBAR is at IPSBAR+8 not IPSBAR+4
 * - correction: removed MCF5282_SCM_FLASHBAR definition
 * - wrapping use of args x with (x) for safety
 *
 * $Source: include/trmcffec.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:30JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _MCF5282_H /* { */
#define _MCF5282_H

#ifdef __cplusplus
extern "C" {
#endif

/* NOTE: TM_MCF5282_RX_BUF_LEN must be evenly divisible by 16 */
#ifdef TM_USE_DRV_SCAT_RECV
/* Not recommended to use smaller than 256. The last buffer will also
 include the CRC. */
#define TM_MCF5282_RX_BUF_LEN           400
/* ethernet receive buffer descriptors */
#define TM_MCF5282_ETHER_MAX_RECV_BUFD  25
#else /* ! TM_USE_DRV_SCAT_RECV */
/* Enough room to guarantee a complete packet per buffer including CRC */
#define TM_MCF5282_RX_BUF_LEN           1520
/* ethernet receive buffer descriptors */
#define TM_MCF5282_ETHER_MAX_RECV_BUFD  8
#endif /* ! TM_USE_DRV_SCAT_RECV */

/* ethernet transmit buffer descriptors */
#define TM_MCF5282_ETHER_MAX_XMIT_BUFD  32

/* device driver function prototypes */
void tfMcf5282IntRxHandler(void);
int tfMcf5282Open(ttUserInterface interfaceHandle);
int tfMcf5282Close(ttUserInterface interfaceHandle);
int tfMcf5282GetPhyAddr(
    ttUserInterface userInterface, ttCharPtr address);
/* IOCTL function type flags */    
#define TM_MCF5282_IOCTL_REFILL_RX 0x0001
#define TM_MCF5282_SEND_COMPLETE   0x0002
int tfMcf5282Ioctl(
    ttUserInterface interfaceHandle, int flag,
    ttVoidPtr optionPtr, int optionLen);
int tfMcf5282Receive(
    ttUserInterface interfaceHandle,
    ttCharPtrPtr    dataPtrPtr,
    ttIntPtr        lengthPtr,
    ttUserBufferPtr userBufferHandlePtr);
#ifdef TM_USE_DRV_SCAT_RECV  
int tfMcf5282ScatRecv (
    ttUserInterface interfaceHandle,
    ttDruBlockPtrPtr uDevBlockPtrPtr,
    ttIntPtr        uDevBlockCountPtr,
    ttIntPtr        flagPtr);
#endif /* TM_USE_DRV_SCAT_RECV */
int tfMcf5282Send(
    ttUserInterface interfaceHandle,
    ttCharPtr       bufferPtr,
    int             len,
    int             flag);
#ifdef TM_USE_DRV_ONE_SCAT_SEND
int tfMcf5282ScatSend(
    ttUserInterface interfaceHandle, ttUserPacketPtr  uPacketPtr );
#endif /* TM_USE_DRV_ONE_SCAT_SEND */

/* from Quadros m5282evb.h */
extern unsigned char __IPSBAR[];

/* from Quadros sysinit.h */
int checkICR(int n, int level, int priority);
void writeICR(int n, int level, int priority);

/*********************************************************************
*
* System Control Module (SCM)
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_SCM_IPSBAR   (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0000]))
#define MCF5282_SCM_RAMBAR   (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0008]))

#define MCF5282_SCM_CRSR     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0010]))
#define MCF5282_SCM_CWCR     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0011]))
#define MCF5282_SCM_LPICR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0012]))
#define MCF5282_SCM_CWSR     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0013]))
#define MCF5282_SCM_DMAREQC  (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0014]))
#define MCF5282_SCM_MPARK    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x001C]))
#define MCF5282_SCM_MPR      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0020]))

#define MCF5282_SCM_PACR0    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0024]))
#define MCF5282_SCM_PACR1    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0025]))
#define MCF5282_SCM_PACR2    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0026]))
#define MCF5282_SCM_PACR3    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0027]))
#define MCF5282_SCM_PACR4    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0028]))
#define MCF5282_SCM_PACR5    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x002A]))
#define MCF5282_SCM_PACR6    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x002B]))
#define MCF5282_SCM_PACR7    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x002C]))
#define MCF5282_SCM_PACR8    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x002E]))

#define MCF5282_SCM_GPACR0   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0030]))
#define MCF5282_SCM_GPACR1   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0031]))

/* Bit level definitions and macros */
#define MCF5282_SCM_IPSBAR_BA(x)            ((x) & 0xC0000000)
#define MCF5282_SCM_IPSBAR_V                (0x00000001)

#define MCF5282_SCM_RAMBAR_BA(x)            ((x) & 0xFFFF0000)
#define MCF5282_SCM_RAMBAR_BDE              (0x00000200)

/* RAMBAR *register* definitions - not *RAMBAR* in IPSBAR */
#define RAMBAR_BA(x)                ((x) & 0xFFFF0000)

#define RAMBAR_PRI(x)               ((x) << 10) /* 2-bit priority; hi/lo 32k banks */
#define   RAMBAR_PRI_CPU_CPU          0 /* CPU hi, CPU lo */
#define   RAMBAR_PRI_CPU_DMA          1 /* CPU hi, DMA lo */
#define   RAMBAR_PRI_DMA_CPU          2 /* DMA hi, CPU lo */
#define   RAMBAR_PRI_DMA_DMA          3 /* DMA hi, DMA lo */

#define RAMBAR_SPV(x)               ((x) << 9) /* Secondary Port Valid (allows DMA) */
#define   RAMBAR_SPV_DISABLE          0
#define   RAMBAR_SPV_ENABLE           1

#define RAMBAR_WP(x)                ((x) << 8) /* Write Protect */
#define   RAMBAR_WP_RW                0 /* Read + Write */
#define   RAMBAR_WP_R                 1 /* Read */

/* Address Space Masks (5) - 0=enable, 1=disable SRAM accesses */
#define RAMBAR_CI(x)                ((x) << 5) /* CPU space/interrupt acknowledge cycle */
#define RAMBAR_SC(x)                ((x) << 4) /* Supervisor Code address */
#define RAMBAR_SD(x)                ((x) << 3) /* Supervisor Data address */
#define RAMBAR_UC(x)                ((x) << 2) /* User Code address */
#define RAMBAR_UD(x)                ((x) << 1) /* User Data address */

#define RAMBAR_V                    (0x00000001)

#define MCF5282_SCM_CRSR_EXT                (0x80)
#define MCF5282_SCM_CRSR_CWDR               (0x20)

#define MCF5282_SCM_CWCR_CWE                (0x80)
#define MCF5282_SCM_CWCR_CWRI               (0x40)
#define MCF5282_SCM_CWCR_CWT(x)             (((x) & 0x03) << 3)
#define MCF5282_SCM_CWCR_CWTA               (0x04)
#define MCF5282_SCM_CWCR_CWTAVAL            (0x02)
#define MCF5282_SCM_CWCR_CWTIC              (0x01)

#define MCF5282_SCM_LPICR_ENBSTOP           (0x80)
#define MCF5282_SCM_LPICR_XLPM_IPL(x)      (((x) & 0x07) << 4)

#define MCF5282_SCM_CWSR_SEQ1               (0x55)
#define MCF5282_SCM_CWSR_SEQ2               (0xAA)

#define MCF5282_SCM_DMAREQC_DMAC3(x)        (((x) & 0x000F) << 12)
#define MCF5282_SCM_DMAREQC_DMAC2(x)        (((x) & 0x000F) <<  8)
#define MCF5282_SCM_DMAREQC_DMAC1(x)        (((x) & 0x000F) <<  4)
#define MCF5282_SCM_DMAREQC_DMAC0(x)        (((x) & 0x000F) <<  0)
#define MCF5282_SCM_DMAREQC_DMATIMER0       (0x4)
#define MCF5282_SCM_DMAREQC_DMATIMER1       (0x5)
#define MCF5282_SCM_DMAREQC_DMATIMER2       (0x6)
#define MCF5282_SCM_DMAREQC_DMATIMER3       (0x7)
#define MCF5282_SCM_DMAREQC_UART0           (0x8)
#define MCF5282_SCM_DMAREQC_UART1           (0x9)
#define MCF5282_SCM_DMAREQC_UART2           (0xA)

#define MCF5282_SCM_MPARK_M2_P_EN           (0x02000000)
#define MCF5282_SCM_MPARK_BCR24BIT          (0x01000000)
#define MCF5282_SCM_MPARK_M3_PRTY(x)        (((x) & 0x03) << 22)
#define MCF5282_SCM_MPARK_M2_PRTY(x)        (((x) & 0x03) << 20)
#define MCF5282_SCM_MPARK_M0_PRTY(x)        (((x) & 0x03) << 18)
#define MCF5282_SCM_MPARK_M1_PRTY(x)        (((x) & 0x03) << 16)
#define MCF5282_SCM_MPARK_FIXED             (0x00040000)
#define MCF5282_SCM_MPARK_TIMEOUT           (0x00020000)
#define MCF5282_SCM_MPARK_PRK_LAST          (0x00010000)
#define MCF5282_SCM_MPARK_LCKOUT_TIME(x)    (((x) & 0x000F) << 8)

#define MCF5282_SCM_MPARK_MX_PRTY_FIRST     (0x3)
#define MCF5282_SCM_MPARK_MX_PRTY_SECOND    (0x2)
#define MCF5282_SCM_MPARK_MX_PRTY_THIRD     (0x1)
#define MCF5282_SCM_MPARK_MX_PRTY_FOURTH    (0x0)

#define MCF5282_SCM_MPR_MPR(x)              (((x) & 0x0F))
#define MCF5282_SCM_MPR_DMA                   (1 << 2)
#define MCF5282_SCM_MPR_FEC                   (1 << 3)

#define MCF5282_SCM_PACR_LOCK1                (0x80)
#define MCF5282_SCM_PACR_ACCESSCTRL1(x)     (((x) & 0x07) << 4)
#define MCF5282_SCM_PACR_LOCK0                (0x08)
#define MCF5282_SCM_PACR_ACCESSCTRL0(x)     (((x) & 0x07) << 0)
#define MCF5282_SCM_PACR_RW_NA                (0x0)
#define MCF5282_SCM_PACR_R_NA                 (0x1)
#define MCF5282_SCM_PACR_R_R                  (0x2)
#define MCF5282_SCM_PACR_RW_RW                (0x4)
#define MCF5282_SCM_PACR_RW_R                 (0x5)
#define MCF5282_SCM_PACR_NA_NA                (0x7)

#define MCF5282_SCM_GPACR_LOCK                (0x80)
#define MCF5282_SCM_GPACR_ACCESSCTRL(x)     (((x) & 0x0F))

#define MCF5282_SCM_GPACR_ACCESSCTRL_RW_NA   (0x00)
#define MCF5282_SCM_GPACR_ACCESSCTRL_R_NA    (0x01)
#define MCF5282_SCM_GPACR_ACCESSCTRL_R_R     (0x02)
#define MCF5282_SCM_GPACR_ACCESSCTRL_RW_RW   (0x04)
#define MCF5282_SCM_GPACR_ACCESSCTRL_RW_R    (0x05)
#define MCF5282_SCM_GPACR_ACCESSCTRL_NA_NA   (0x07)
#define MCF5282_SCM_GPACR_ACCESSCTRL_RWE_NA  (0x08)
#define MCF5282_SCM_GPACR_ACCESSCTRL_RE_NA   (0x09)
#define MCF5282_SCM_GPACR_ACCESSCTRL_RE_RE   (0x0A)
#define MCF5282_SCM_GPACR_ACCESSCTRL_E_NA    (0x0B)
#define MCF5282_SCM_GPACR_ACCESSCTRL_RWE_RWE (0x0C)
#define MCF5282_SCM_GPACR_ACCESSCTRL_RWE_RE  (0x0D)
#define MCF5282_SCM_GPACR_ACCESSCTRL_RWE_E   (0x0F)

/*********************************************************************
*
* SDRAM Controller Module
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_SDRAMC_DCR   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x0040]))

#define MCF5282_SDRAMC_DACR0 (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0048]))
#define MCF5282_SDRAMC_DMR0  (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x004C]))

#define MCF5282_SDRAMC_DACR1 (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0050]))
#define MCF5282_SDRAMC_DMR1  (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0054]))

/* Bit level definitions and macros */
#define MCF5282_SDRAMC_DCR_NAM              (0x2000)
#define MCF5282_SDRAMC_DCR_COC              (0x1000)
#define MCF5282_SDRAMC_DCR_IS               (0x0800)
#define MCF5282_SDRAMC_DCR_RTIM_3           (0x0000)
#define MCF5282_SDRAMC_DCR_RTIM_6           (0x0200)
#define MCF5282_SDRAMC_DCR_RTIM_9           (0x0400)
#define MCF5282_SDRAMC_DCR_RC(x)            ((x) & 0x01FF)

#define MCF5282_SDRAMC_DACR_BASE(x)         ((x) & 0xFFFC0000)
#define MCF5282_SDRAMC_DACR_RE              (0x00008000)
#define MCF5282_SDRAMC_DACR_CASL(x)         (((x) & 0x03) << 12)
#define MCF5282_SDRAMC_DACR_CBM(x)          (((x) & 0x07) <<  8)
#define MCF5282_SDRAMC_DACR_PS_32           (0x00000000)
#define MCF5282_SDRAMC_DACR_PS_8            (0x00000010)
#define MCF5282_SDRAMC_DACR_PS_16           (0x00000020)
#define MCF5282_SDRAMC_DACR_IMRS            (0x00000040)
#define MCF5282_SDRAMC_DACR_IP              (0x00000008)

#define MCF5282_SDRAMC_DMR_BAM_4G           (0xFFFC0000)
#define MCF5282_SDRAMC_DMR_BAM_2G           (0x7FFC0000)
#define MCF5282_SDRAMC_DMR_BAM_1G           (0x3FFC0000)
#define MCF5282_SDRAMC_DMR_BAM_1024M        (0x3FFC0000)
#define MCF5282_SDRAMC_DMR_BAM_512M         (0x1FFC0000)
#define MCF5282_SDRAMC_DMR_BAM_256M         (0x0FFC0000)
#define MCF5282_SDRAMC_DMR_BAM_128M         (0x07FC0000)
#define MCF5282_SDRAMC_DMR_BAM_64M          (0x03FC0000)
#define MCF5282_SDRAMC_DMR_BAM_32M          (0x01FC0000)
#define MCF5282_SDRAMC_DMR_BAM_16M          (0x00FC0000)
#define MCF5282_SDRAMC_DMR_BAM_8M           (0x007C0000)
#define MCF5282_SDRAMC_DMR_BAM_4M           (0x003C0000)
#define MCF5282_SDRAMC_DMR_BAM_2M           (0x001C0000)
#define MCF5282_SDRAMC_DMR_BAM_1M           (0x000C0000)
#define MCF5282_SDRAMC_DMR_BAM_1024K        (0x000C0000)
#define MCF5282_SDRAMC_DMR_BAM_512K         (0x00040000)
#define MCF5282_SDRAMC_DMR_BAM_256K         (0x00000000)
#define MCF5282_SDRAMC_DMR_WP               (0x00000100)
#define MCF5282_SDRAMC_DMR_CI               (0x00000040)
#define MCF5282_SDRAMC_DMR_AM               (0x00000020)
#define MCF5282_SDRAMC_DMR_SC               (0x00000010)
#define MCF5282_SDRAMC_DMR_SD               (0x00000008)
#define MCF5282_SDRAMC_DMR_UC               (0x00000004)
#define MCF5282_SDRAMC_DMR_UD               (0x00000002)
#define MCF5282_SDRAMC_DMR_V                (0x00000001)

/*********************************************************************
*
* Chip Select Module
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_CS0_CSAR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x0080]))
#define MCF5282_CS0_CSMR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0084]))
#define MCF5282_CS0_CSCR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x008A]))

#define MCF5282_CS1_CSAR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x008C]))
#define MCF5282_CS1_CSMR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0090]))
#define MCF5282_CS1_CSCR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x0096]))

#define MCF5282_CS2_CSAR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x0098]))
#define MCF5282_CS2_CSMR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x009C]))
#define MCF5282_CS2_CSCR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x00A2]))

#define MCF5282_CS3_CSAR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x00A4]))
#define MCF5282_CS3_CSMR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x00A8]))
#define MCF5282_CS3_CSCR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x00AE]))

#define MCF5282_CS4_CSAR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x00B0]))
#define MCF5282_CS4_CSMR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x00B4]))
#define MCF5282_CS4_CSCR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x00BA]))

#define MCF5282_CS5_CSAR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x00BC]))
#define MCF5282_CS5_CSMR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x00C0]))
#define MCF5282_CS5_CSCR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x00C6]))

#define MCF5282_CS6_CSAR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x00C8]))
#define MCF5282_CS6_CSMR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x00CC]))
#define MCF5282_CS6_CSCR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x00D2]))

#define MCF5282_CS_CSAR(x)  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x0080 + ((x) * 0x0C)]))
#define MCF5282_CS_CSMR(x)  (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0084 + ((x) * 0x0C)]))
#define MCF5282_CS_CSCR(x)  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x008A + ((x) * 0x0C)]))

/* Bit level definitions and macros */
#define MCF5282_CS_CSAR_BA(a)       (uint16)(((a) & 0xFFFF0000) >> 16)

#define MCF5282_CS_CSMR_BAM_4G      (0xFFFF0000)
#define MCF5282_CS_CSMR_BAM_2G      (0x7FFF0000)
#define MCF5282_CS_CSMR_BAM_1G      (0x3FFF0000)
#define MCF5282_CS_CSMR_BAM_1024M   (0x3FFF0000)
#define MCF5282_CS_CSMR_BAM_512M    (0x1FFF0000)
#define MCF5282_CS_CSMR_BAM_256M    (0x0FFF0000)
#define MCF5282_CS_CSMR_BAM_128M    (0x07FF0000)
#define MCF5282_CS_CSMR_BAM_64M     (0x03FF0000)
#define MCF5282_CS_CSMR_BAM_32M     (0x01FF0000)
#define MCF5282_CS_CSMR_BAM_16M     (0x00FF0000)
#define MCF5282_CS_CSMR_BAM_8M      (0x007F0000)
#define MCF5282_CS_CSMR_BAM_4M      (0x003F0000)
#define MCF5282_CS_CSMR_BAM_2M      (0x001F0000)
#define MCF5282_CS_CSMR_BAM_1M      (0x000F0000)
#define MCF5282_CS_CSMR_BAM_1024K   (0x000F0000)
#define MCF5282_CS_CSMR_BAM_512K    (0x00070000)
#define MCF5282_CS_CSMR_BAM_256K    (0x00030000)
#define MCF5282_CS_CSMR_BAM_128K    (0x00010000)
#define MCF5282_CS_CSMR_BAM_64K     (0x00000000)
#define MCF5282_CS_CSMR_WP          (0x00000100)
#define MCF5282_CS_CSMR_AM          (0x00000040)
#define MCF5282_CS_CSMR_CI          (0x00000020)
#define MCF5282_CS_CSMR_SC          (0x00000010)
#define MCF5282_CS_CSMR_SD          (0x00000008)
#define MCF5282_CS_CSMR_UC          (0x00000004)
#define MCF5282_CS_CSMR_UD          (0x00000002)
#define MCF5282_CS_CSMR_V           (0x00000001)

#define MCF5282_CS_CSCR_WS(x)       (((x) & 0x0F) << 10)
#define MCF5282_CS_CSCR_AA          (0x0100)
#define MCF5282_CS_CSCR_PS_8        (0x0040)
#define MCF5282_CS_CSCR_PS_16       (0x0080)
#define MCF5282_CS_CSCR_PS_32       (0x0000)
#define MCF5282_CS_CSCR_BEM         (0x0020)
#define MCF5282_CS_CSCR_BSTR        (0x0010)
#define MCF5282_CS_CSCR_BSTW        (0x0008)

/*********************************************************************
*
* Direct Memory Access (DMA) Module
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_DMA0_SAR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0100]))
#define MCF5282_DMA0_DAR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0104]))
#define MCF5282_DMA0_DCR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0108]))
#define MCF5282_DMA0_BCR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x010C]))
#define MCF5282_DMA0_DSR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0110]))

#define MCF5282_DMA1_SAR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0140]))
#define MCF5282_DMA1_DAR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0144]))
#define MCF5282_DMA1_DCR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0148]))
#define MCF5282_DMA1_BCR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x014C]))
#define MCF5282_DMA1_DSR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0150]))

#define MCF5282_DMA2_SAR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0180]))
#define MCF5282_DMA2_DAR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0184]))
#define MCF5282_DMA2_DCR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0188]))
#define MCF5282_DMA2_BCR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x018C]))
#define MCF5282_DMA2_DSR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0190]))

#define MCF5282_DMA3_SAR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x01C0]))
#define MCF5282_DMA3_DAR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x01C4]))
#define MCF5282_DMA3_DCR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x01C8]))
#define MCF5282_DMA3_BCR    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x01CC]))
#define MCF5282_DMA3_DSR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x01D0]))

#define MCF5282_DMA_SAR(x)  (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0100 + ((x) * 0x40)]))
#define MCF5282_DMA_DAR(x)  (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0104 + ((x) * 0x40)]))
#define MCF5282_DMA_DCR(x)  (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0108 + ((x) * 0x40)]))
#define MCF5282_DMA_BCR(x)  (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x010C + ((x) * 0x40)]))
#define MCF5282_DMA_DSR(x)  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0110 + ((x) * 0x40)]))

/* Bit level definitions and macros */
#define MCF5282_DMA_DCR_INT                 (0x80000000)
#define MCF5282_DMA_DCR_EEXT                (0x40000000)
#define MCF5282_DMA_DCR_CS                  (0x20000000)
#define MCF5282_DMA_DCR_AA                  (0x10000000)
#define MCF5282_DMA_DCR_BWC_DMA             (0x00000000)
#define MCF5282_DMA_DCR_BWC_512             (0x02000000)
#define MCF5282_DMA_DCR_BWC_1024            (0x04000000)
#define MCF5282_DMA_DCR_BWC_2048            (0x06000000)
#define MCF5282_DMA_DCR_BWC_4096            (0x08000000)
#define MCF5282_DMA_DCR_BWC_8192            (0x0A000000)
#define MCF5282_DMA_DCR_BWC_16384           (0x0C000000)
#define MCF5282_DMA_DCR_BWC_32768           (0x0E000000)
#define MCF5282_DMA_DCR_SINC                (0x00400000)
#define MCF5282_DMA_DCR_SSIZE_LONG            (0x00000000)
#define MCF5282_DMA_DCR_SSIZE_BYTE            (0x00100000)
#define MCF5282_DMA_DCR_SSIZE_WORD            (0x00200000)
#define MCF5282_DMA_DCR_SSIZE_LINE            (0x00300000)
#define MCF5282_DMA_DCR_DINC                (0x00080000)
#define MCF5282_DMA_DCR_DSIZE_LONG            (0x00000000)
#define MCF5282_DMA_DCR_DSIZE_BYTE            (0x00020000)
#define MCF5282_DMA_DCR_DSIZE_WORD            (0x00040000)
#define MCF5282_DMA_DCR_START               (0x00010000)
#define MCF5282_DMA_DCR_AT                  (0x00008000)

#define MCF5282_DMA_DSR_CE                  (0x40)
#define MCF5282_DMA_DSR_BES                 (0x20)
#define MCF5282_DMA_DSR_BED                 (0x10)
#define MCF5282_DMA_DSR_REQ                 (0x04)
#define MCF5282_DMA_DSR_BSY                 (0x02)
#define MCF5282_DMA_DSR_DONE                (0x01)

/*********************************************************************
*
* Universal Asychronous Receiver/Transmitter (UART) Module
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_UART0_UMR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0200]))
#define MCF5282_UART0_USR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0204]))
#define MCF5282_UART0_UCSR   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0204]))
#define MCF5282_UART0_UCR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0208]))
#define MCF5282_UART0_URB    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x020C]))
#define MCF5282_UART0_UTB    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x020C]))
#define MCF5282_UART0_UIPCR  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0210]))
#define MCF5282_UART0_UACR   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0210]))
#define MCF5282_UART0_UISR   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0214]))
#define MCF5282_UART0_UIMR   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0214]))
#define MCF5282_UART0_UBG1   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0218]))
#define MCF5282_UART0_UBG2   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x021C]))
#define MCF5282_UART0_UIP    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0234]))
#define MCF5282_UART0_UOP1   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0238]))
#define MCF5282_UART0_UOP0   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x023C]))

#define MCF5282_UART1_UMR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0240]))
#define MCF5282_UART1_USR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0244]))
#define MCF5282_UART1_UCSR   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0244]))
#define MCF5282_UART1_UCR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0248]))
#define MCF5282_UART1_URB    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x024C]))
#define MCF5282_UART1_UTB    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x024C]))
#define MCF5282_UART1_UIPCR  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0250]))
#define MCF5282_UART1_UACR   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0250]))
#define MCF5282_UART1_UISR   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0254]))
#define MCF5282_UART1_UIMR   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0254]))
#define MCF5282_UART1_UBG1   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0258]))
#define MCF5282_UART1_UBG2   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x025C]))
#define MCF5282_UART1_UIP    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0274]))
#define MCF5282_UART1_UOP1   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0278]))
#define MCF5282_UART1_UOP0   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x027C]))

#define MCF5282_UART2_UMR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0280]))
#define MCF5282_UART2_USR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0284]))
#define MCF5282_UART2_UCSR   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0284]))
#define MCF5282_UART2_UCR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0288]))
#define MCF5282_UART2_URB    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x028C]))
#define MCF5282_UART2_UTB    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x028C]))
#define MCF5282_UART2_UIPCR  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0290]))
#define MCF5282_UART2_UACR   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0290]))
#define MCF5282_UART2_UISR   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0294]))
#define MCF5282_UART2_UIMR   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0294]))
#define MCF5282_UART2_UBG1   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0298]))
#define MCF5282_UART2_UBG2   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x029C]))
#define MCF5282_UART2_UIP    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x02B4]))
#define MCF5282_UART2_UOP1   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x02B8]))
#define MCF5282_UART2_UOP0   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x02BC]))

#define MCF5282_UART_UMR(x)     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0200 + ((x) * 0x40)]))
#define MCF5282_UART_USR(x)     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0204 + ((x) * 0x40)]))
#define MCF5282_UART_UCSR(x)    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0204 + ((x) * 0x40)]))
#define MCF5282_UART_UCR(x)     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0208 + ((x) * 0x40)]))
#define MCF5282_UART_URB(x)     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0xx20C + ((x) * 0x40)]))
#define MCF5282_UART_UTB(x)     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x020C + ((x) * 0x40)]))
#define MCF5282_UART_UIPCR(x)   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0210 + ((x) * 0x40)]))
#define MCF5282_UART_UACR(x)    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0210 + ((x) * 0x40)]))
#define MCF5282_UART_UISR(x)    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0214 + ((x) * 0x40)]))
#define MCF5282_UART_UIMR(x)    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0214 + ((x) * 0x40)]))
#define MCF5282_UART_UBG1(x)    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0218 + ((x) * 0x40)]))
#define MCF5282_UART_UBG2(x)    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x021C + ((x) * 0x40)]))
#define MCF5282_UART_UIP(x)     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0234 + ((x) * 0x40)]))
#define MCF5282_UART_UOP1(x)    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0238 + ((x) * 0x40)]))
#define MCF5282_UART_UOP0(x)    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x023C + ((x) * 0x40)]))

/* Bit level definitions and macros */
#define MCF5282_UART_UMR1_RXRTS              (0x80)
#define MCF5282_UART_UMR1_RXIRQ              (0x40)
#define MCF5282_UART_UMR1_ERR                (0x20)
#define MCF5282_UART_UMR1_PM_MULTI_ADDR      (0x1C)
#define MCF5282_UART_UMR1_PM_MULTI_DATA      (0x18)
#define MCF5282_UART_UMR1_PM_NONE            (0x10)
#define MCF5282_UART_UMR1_PM_FORCE_HI        (0x0C)
#define MCF5282_UART_UMR1_PM_FORCE_LO        (0x08)
#define MCF5282_UART_UMR1_PM_ODD             (0x04)
#define MCF5282_UART_UMR1_PM_EVEN            (0x00)
#define MCF5282_UART_UMR1_BC_5               (0x00)
#define MCF5282_UART_UMR1_BC_6               (0x01)
#define MCF5282_UART_UMR1_BC_7               (0x02)
#define MCF5282_UART_UMR1_BC_8               (0x03)

#define MCF5282_UART_UMR2_CM_NORMAL          (0x00)
#define MCF5282_UART_UMR2_CM_ECHO            (0x40)
#define MCF5282_UART_UMR2_CM_LOCAL_LOOP      (0x80)
#define MCF5282_UART_UMR2_CM_REMOTE_LOOP     (0xC0)
#define MCF5282_UART_UMR2_TXRTS              (0x20)
#define MCF5282_UART_UMR2_TXCTS              (0x10)
#define MCF5282_UART_UMR2_STOP_BITS_1        (0x07)
#define MCF5282_UART_UMR2_STOP_BITS_15       (0x08)
#define MCF5282_UART_UMR2_STOP_BITS_2        (0x0F)
#define MCF5282_UART_UMR2_STOP_BITS(a)       ((a) & 0x0f)

#define MCF5282_UART_USR_RB                  (0x80)
#define MCF5282_UART_USR_FE                  (0x40)
#define MCF5282_UART_USR_PE                  (0x20)
#define MCF5282_UART_USR_OE                  (0x10)
#define MCF5282_UART_USR_TXEMP               (0x08)
#define MCF5282_UART_USR_TXRDY               (0x04)
#define MCF5282_UART_USR_FFULL               (0x02)
#define MCF5282_UART_USR_RXRDY               (0x01)

#define MCF5282_UART_UCSR_RCS_SYS_CLK        (0xD0)
#define MCF5282_UART_UCSR_RCS_DTIN16         (0xE0)
#define MCF5282_UART_UCSR_RCS_DTIN           (0xF0)
#define MCF5282_UART_UCSR_TCS_SYS_CLK        (0x0D)
#define MCF5282_UART_UCSR_TCS_DTIN16         (0x0E)
#define MCF5282_UART_UCSR_TCS_DTIN           (0x0F)

#define MCF5282_UART_UCR_NONE                (0x00)
#define MCF5282_UART_UCR_STOP_BREAK          (0x70)
#define MCF5282_UART_UCR_START_BREAK         (0x60)
#define MCF5282_UART_UCR_RESET_BKCHGINT      (0x50)
#define MCF5282_UART_UCR_RESET_ERROR         (0x40)
#define MCF5282_UART_UCR_RESET_TX            (0x30)
#define MCF5282_UART_UCR_RESET_RX            (0x20)
#define MCF5282_UART_UCR_RESET_MR            (0x10)
#define MCF5282_UART_UCR_TX_DISABLED         (0x08)
#define MCF5282_UART_UCR_TX_ENABLED          (0x04)
#define MCF5282_UART_UCR_RX_DISABLED         (0x02)
#define MCF5282_UART_UCR_RX_ENABLED          (0x01)

#define MCF5282_UART_UIPCR_COS               (0x10)
#define MCF5282_UART_UIPCR_CTS               (0x01)

#define MCF5282_UART_UACR_IEC                (0x01)

#define MCF5282_UART_UISR_COS                (0x80)
#define MCF5282_UART_UISR_ABC                (0x40)
#define MCF5282_UART_UISR_RXFIFO             (0x20)
#define MCF5282_UART_UISR_TXFIFO             (0x10)
#define MCF5282_UART_UISR_RXFTO              (0x08)
#define MCF5282_UART_UISR_DB                 (0x04)
#define MCF5282_UART_UISR_RXRDY              (0x02)
#define MCF5282_UART_UISR_TXRDY              (0x01)

#define MCF5282_UART_UIMR_COS                (0x80)
#define MCF5282_UART_UIMR_DB                 (0x04)
#define MCF5282_UART_UIMR_FFULL              (0x02)
#define MCF5282_UART_UIMR_RXRDY              (0x02) /* alias */
#define MCF5282_UART_UIMR_TXRDY              (0x01)

#define MCF5282_UART_UIP_CTS                 (0x01)

#define MCF5282_UART_UOP_RTS                 (0x01)

/*********************************************************************
*
* Inter-IC (I2C) Module
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_I2C_I2ADR   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0300]))
#define MCF5282_I2C_I2FDR   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0304]))
#define MCF5282_I2C_I2CR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0308]))
#define MCF5282_I2C_I2SR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x030C]))
#define MCF5282_I2C_I2DR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0310]))

/* Bit level definitions and macros */
#define MCF5282_I2C_I2ADR_ADDR(x)        (((x) & 0x7F) << 0x01)

#define MCF5282_I2C_I2FDR_IC(x)          (((x) & 0x3F))

#define MCF5282_I2C_I2CR_IEN             (0x80)
#define MCF5282_I2C_I2CR_IIEN            (0x40)
#define MCF5282_I2C_I2CR_MSTA            (0x20)
#define MCF5282_I2C_I2CR_MTX             (0x10)
#define MCF5282_I2C_I2CR_TXAK            (0x08)
#define MCF5282_I2C_I2CR_RSTA            (0x04)

#define MCF5282_I2C_I2SR_ICF             (0x80)
#define MCF5282_I2C_I2SR_IAAS            (0x40)
#define MCF5282_I2C_I2SR_IBB             (0x20)
#define MCF5282_I2C_I2SR_IAL             (0x10)
#define MCF5282_I2C_I2SR_SRW             (0x04)
#define MCF5282_I2C_I2SR_IIF             (0x02)
#define MCF5282_I2C_I2SR_RXAK            (0x01)

/*********************************************************************
*
* Queued Serial Peripheral Interface (QSPI) Module
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_QSPI_QMR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x0340]))
#define MCF5282_QSPI_QDLYR  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x0344]))
#define MCF5282_QSPI_QWR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x0348]))
#define MCF5282_QSPI_QIR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x034C]))
#define MCF5282_QSPI_QAR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x0350]))
#define MCF5282_QSPI_QDR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x0354]))
#define MCF5282_QSPI_QCR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x0354]))

/* Bit level definitions and macros */
#define MCF5282_QSPI_QMR_MSTR                 (0x8000)
#define MCF5282_QSPI_QMR_DOHIE                (0x4000)
#define MCF5282_QSPI_QMR_BITS_16              (0x0000)
#define MCF5282_QSPI_QMR_BITS_8               (0x2000)
#define MCF5282_QSPI_QMR_BITS_9               (0x2400)
#define MCF5282_QSPI_QMR_BITS_10              (0x2800)
#define MCF5282_QSPI_QMR_BITS_11              (0x2C00)
#define MCF5282_QSPI_QMR_BITS_12              (0x3000)
#define MCF5282_QSPI_QMR_BITS_13              (0x3400)
#define MCF5282_QSPI_QMR_BITS_14              (0x3800)
#define MCF5282_QSPI_QMR_BITS_15              (0x3C00)
#define MCF5282_QSPI_QMR_CPOL                 (0x0200)
#define MCF5282_QSPI_QMR_CPHA                 (0x0100)
#define MCF5282_QSPI_QMR_BAUD(x)              (((x) & 0x00FF))

#define MCF5282_QSPI_QDLYR_SPE                (0x80)
#define MCF5282_QSPI_QDLYR_QCD(x)             (((x) & 0x007F) << 8)
#define MCF5282_QSPI_QDLYR_DTL(x)             (((x) & 0x00FF))

#define MCF5282_QSPI_QWR_HALT                 (0x8000)
#define MCF5282_QSPI_QWR_WREN                 (0x4000)
#define MCF5282_QSPI_QWR_WRTO                 (0x2000)
#define MCF5282_QSPI_QWR_CSIV                 (0x1000)
#define MCF5282_QSPI_QWR_ENDQP(x)             (((x) & 0x000F) << 8)
#define MCF5282_QSPI_QWR_CPTQP(x)             (((x) & 0x000F) << 4)
#define MCF5282_QSPI_QWR_NEWQP(x)             (((x) & 0x000F))

#define MCF5282_QSPI_QIR_WCEFB                (0x8000)
#define MCF5282_QSPI_QIR_ABRTB                (0x4000)
#define MCF5282_QSPI_QIR_ABRTL                (0x1000)
#define MCF5282_QSPI_QIR_WCEFE                (0x0800)
#define MCF5282_QSPI_QIR_ABRTE                (0x0400)
#define MCF5282_QSPI_QIR_SPIFE                (0x0100)
#define MCF5282_QSPI_QIR_WCEF                 (0x0008)
#define MCF5282_QSPI_QIR_ABRT                 (0x0004)
#define MCF5282_QSPI_QIR_SPIF                 (0x0001)

#define MCF5282_QSPI_QAR_ADDR(x)              (((x) & 0x003F))

#define MCF5282_QSPI_QDR_COMMAND(x)           (((x) & 0xFF00))

#define MCF5282_QSPI_QCR_DATA(x)              (((x) & 0x00FF) << 8)
#define MCF5282_QSPI_QCR_CONT                 (0x8000)
#define MCF5282_QSPI_QCR_BITSE                (0x4000)
#define MCF5282_QSPI_QCR_DT                   (0x2000)
#define MCF5282_QSPI_QCR_DSCK                 (0x1000)
#define MCF5282_QSPI_QCR_CS                   ((x) & 0x000F) << 8)

/*********************************************************************
*
* DMA Timer Module
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_TIMER0_DTMR   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x0400]))
#define MCF5282_TIMER0_DTXMR  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0402]))
#define MCF5282_TIMER0_DTER   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0403]))
#define MCF5282_TIMER0_DTRR   (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0404]))
#define MCF5282_TIMER0_DTCR   (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0408]))
#define MCF5282_TIMER0_DTCN   (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x040C]))

#define MCF5282_TIMER1_DTMR   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x0440]))
#define MCF5282_TIMER1_DTXMR  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0442]))
#define MCF5282_TIMER1_DTER   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0443]))
#define MCF5282_TIMER1_DTRR   (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0444]))
#define MCF5282_TIMER1_DTCR   (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0448]))
#define MCF5282_TIMER1_DTCN   (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x044C]))

#define MCF5282_TIMER2_DTMR   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x0480]))
#define MCF5282_TIMER2_DTXMR  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0482]))
#define MCF5282_TIMER2_DTER   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0483]))
#define MCF5282_TIMER2_DTRR   (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0484]))
#define MCF5282_TIMER2_DTCR   (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0488]))
#define MCF5282_TIMER2_DTCN   (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x048C]))

#define MCF5282_TIMER3_DTMR   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x04C0]))
#define MCF5282_TIMER3_DTXMR  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x04C2]))
#define MCF5282_TIMER3_DTER   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x04C3]))
#define MCF5282_TIMER3_DTRR   (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x04C4]))
#define MCF5282_TIMER3_DTCR   (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x04C8]))
#define MCF5282_TIMER3_DTCN   (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x04CC]))

#define MCF5282_TIMER_DTMR(x) (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x0400 + ((x) * 0x40)]))
#define MCF5282_TIMER_DTXMR(x) (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0402 + ((x) * 0x40)]))
#define MCF5282_TIMER_DTER(x) (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0403 + ((x) * 0x40)]))
#define MCF5282_TIMER_DTRR(x) (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0404 + ((x) * 0x40)]))
#define MCF5282_TIMER_DTCR(x) (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0408 + ((x) * 0x40)]))
#define MCF5282_TIMER_DTCN(x) (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x040C + ((x) * 0x40)]))

/* Bit level definitions and macros */
#define MCF5282_TIMER_DTMR_PS(a)              (((a) & 0x00FF) << 8)
#define MCF5282_TIMER_DTMR_CE_ANY             (0x00C0)
#define MCF5282_TIMER_DTMR_CE_FALL            (0x0080)
#define MCF5282_TIMER_DTMR_CE_RISE            (0x0040)
#define MCF5282_TIMER_DTMR_CE_NONE            (0x0000)
#define MCF5282_TIMER_DTMR_OM                 (0x0020)
#define MCF5282_TIMER_DTMR_ORRI               (0x0010)
#define MCF5282_TIMER_DTMR_FRR                (0x0008)
#define MCF5282_TIMER_DTMR_CLK_DTIN           (0x0006)
#define MCF5282_TIMER_DTMR_CLK_DIV16          (0x0004)
#define MCF5282_TIMER_DTMR_CLK_DIV1           (0x0002)
#define MCF5282_TIMER_DTMR_CLK_STOP           (0x0000)
#define MCF5282_TIMER_DTMR_RST                (0x0001)

#define MCF5282_TIMER_DTXMR_DMAEN             (0x80)
#define MCF5282_TIMER_DTXMR_MODE16            (0x01)

#define MCF5282_TIMER_DTER_REF                (0x02)
#define MCF5282_TIMER_DTER_CAP                (0x01)

/*********************************************************************
*
* Interrupt Controller (INTC) Module
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_INTC0_IPRH    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C00]))
#define MCF5282_INTC0_IPRL    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C04]))
#define MCF5282_INTC0_IMRH    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C08]))
#define MCF5282_INTC0_IMRL    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C0C]))
#define MCF5282_INTC0_INTFRCH (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C10]))
#define MCF5282_INTC0_INTFRCL (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C14]))
#define MCF5282_INTC0_IRLR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C18]))
#define MCF5282_INTC0_IACKLPR (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C19]))
#define MCF5282_INTC0_ICR1    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C41]))
#define MCF5282_INTC0_ICR2    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C42]))
#define MCF5282_INTC0_ICR3    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C43]))
#define MCF5282_INTC0_ICR4    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C44]))
#define MCF5282_INTC0_ICR5    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C45]))
#define MCF5282_INTC0_ICR6    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C46]))
#define MCF5282_INTC0_ICR7    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C47]))
#define MCF5282_INTC0_ICR8    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C48]))
#define MCF5282_INTC0_ICR9    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C49]))
#define MCF5282_INTC0_ICR10   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C4A]))
#define MCF5282_INTC0_ICR11   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C4B]))
#define MCF5282_INTC0_ICR12   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C4C]))
#define MCF5282_INTC0_ICR13   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C4D]))
#define MCF5282_INTC0_ICR14   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C4E]))
#define MCF5282_INTC0_ICR15   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C4F]))
#define MCF5282_INTC0_ICR17   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C51]))
#define MCF5282_INTC0_ICR18   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C52]))
#define MCF5282_INTC0_ICR19   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C53]))
#define MCF5282_INTC0_ICR20   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C54]))
#define MCF5282_INTC0_ICR21   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C55]))
#define MCF5282_INTC0_ICR22   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C56]))
#define MCF5282_INTC0_ICR23   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C57]))
#define MCF5282_INTC0_ICR24   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C58]))
#define MCF5282_INTC0_ICR25   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C59]))
#define MCF5282_INTC0_ICR26   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C5A]))
#define MCF5282_INTC0_ICR27   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C5B]))
#define MCF5282_INTC0_ICR28   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C5C]))
#define MCF5282_INTC0_ICR29   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C5D]))
#define MCF5282_INTC0_ICR30   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C5E]))
#define MCF5282_INTC0_ICR31   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C5F]))
#define MCF5282_INTC0_ICR32   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C60]))
#define MCF5282_INTC0_ICR33   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C61]))
#define MCF5282_INTC0_ICR34   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C62]))
#define MCF5282_INTC0_ICR35   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C63]))
#define MCF5282_INTC0_ICR36   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C64]))
#define MCF5282_INTC0_ICR37   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C65]))
#define MCF5282_INTC0_ICR38   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C66]))
#define MCF5282_INTC0_ICR39   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C67]))
#define MCF5282_INTC0_ICR40   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C68]))
#define MCF5282_INTC0_ICR41   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C69]))
#define MCF5282_INTC0_ICR42   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C6A]))
#define MCF5282_INTC0_ICR43   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C6B]))
#define MCF5282_INTC0_ICR44   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C6C]))
#define MCF5282_INTC0_ICR45   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C6D]))
#define MCF5282_INTC0_ICR46   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C6E]))
#define MCF5282_INTC0_ICR47   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C6F]))
#define MCF5282_INTC0_ICR48   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C70]))
#define MCF5282_INTC0_ICR49   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C71]))
#define MCF5282_INTC0_ICR50   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C72]))
#define MCF5282_INTC0_ICR51   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C73]))
#define MCF5282_INTC0_ICR52   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C74]))
#define MCF5282_INTC0_ICR53   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C75]))
#define MCF5282_INTC0_ICR54   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C76]))
#define MCF5282_INTC0_ICR55   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C77]))
#define MCF5282_INTC0_ICR56   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C78]))
#define MCF5282_INTC0_ICR57   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C79]))
#define MCF5282_INTC0_ICR58   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C7A]))
#define MCF5282_INTC0_ICR59   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C7B]))
#define MCF5282_INTC0_ICR60   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C7C]))
#define MCF5282_INTC0_ICR61   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C7D]))
#define MCF5282_INTC0_ICR62   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0C7E]))
#define MCF5282_INTC0_SWIACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0CE0]))
#define MCF5282_INTC0_L1IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0CE4]))
#define MCF5282_INTC0_L2IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0CE8]))
#define MCF5282_INTC0_L3IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0CEC]))
#define MCF5282_INTC0_L4IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0CF0]))
#define MCF5282_INTC0_L5IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0CF4]))
#define MCF5282_INTC0_L6IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0CF8]))
#define MCF5282_INTC0_L7IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0CFC]))

#define MCF5282_INTC1_IPRH    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D00]))
#define MCF5282_INTC1_IPRL    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D04]))
#define MCF5282_INTC1_IMRH    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D08]))
#define MCF5282_INTC1_IMRL    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D0C]))
#define MCF5282_INTC1_INTFRCH (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D10]))
#define MCF5282_INTC1_INTFRCL (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D14]))
#define MCF5282_INTC1_IRLR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D18]))
#define MCF5282_INTC1_IACKLPR (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D19]))
#define MCF5282_INTC1_ICR08   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D48]))
#define MCF5282_INTC1_ICR09   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D49]))
#define MCF5282_INTC1_ICR10   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D4A]))
#define MCF5282_INTC1_ICR11   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D4B]))
#define MCF5282_INTC1_ICR12   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D4C]))
#define MCF5282_INTC1_ICR13   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D4D]))
#define MCF5282_INTC1_ICR14   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D4E]))
#define MCF5282_INTC1_ICR15   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D4F]))
#define MCF5282_INTC1_ICR16   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D50]))
#define MCF5282_INTC1_ICR17   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D51]))
#define MCF5282_INTC1_ICR18   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D52]))
#define MCF5282_INTC1_ICR19   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D53]))
#define MCF5282_INTC1_ICR20   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D54]))
#define MCF5282_INTC1_ICR21   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D55]))
#define MCF5282_INTC1_ICR22   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D56]))
#define MCF5282_INTC1_ICR23   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D57]))
#define MCF5282_INTC1_ICR24   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D58]))
#define MCF5282_INTC1_ICR25   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D59]))
#define MCF5282_INTC1_ICR26   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0D5A]))
#define MCF5282_INTC1_SWIACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0DE0]))
#define MCF5282_INTC1_L1IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0DE4]))
#define MCF5282_INTC1_L2IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0DE8]))
#define MCF5282_INTC1_L3IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0DEC]))
#define MCF5282_INTC1_L4IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0DF0]))
#define MCF5282_INTC1_L5IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0DF4]))
#define MCF5282_INTC1_L6IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0DF8]))
#define MCF5282_INTC1_L7IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0DFC]))

/* Bit level definitions and macros */
#define MCF5282_INTC_IPRH_INT63          (0x80000000)
#define MCF5282_INTC_IPRH_INT62          (0x40000000)
#define MCF5282_INTC_IPRH_INT61          (0x20000000)
#define MCF5282_INTC_IPRH_INT60          (0x10000000)
#define MCF5282_INTC_IPRH_INT59          (0x08000000)
#define MCF5282_INTC_IPRH_INT58          (0x04000000)
#define MCF5282_INTC_IPRH_INT57          (0x02000000)
#define MCF5282_INTC_IPRH_INT56          (0x01000000)
#define MCF5282_INTC_IPRH_INT55          (0x00800000)
#define MCF5282_INTC_IPRH_INT54          (0x00400000)
#define MCF5282_INTC_IPRH_INT53          (0x00200000)
#define MCF5282_INTC_IPRH_INT52          (0x00100000)
#define MCF5282_INTC_IPRH_INT51          (0x00080000)
#define MCF5282_INTC_IPRH_INT50          (0x00040000)
#define MCF5282_INTC_IPRH_INT49          (0x00020000)
#define MCF5282_INTC_IPRH_INT48          (0x00010000)
#define MCF5282_INTC_IPRH_INT47          (0x00008000)
#define MCF5282_INTC_IPRH_INT46          (0x00004000)
#define MCF5282_INTC_IPRH_INT45          (0x00002000)
#define MCF5282_INTC_IPRH_INT44          (0x00001000)
#define MCF5282_INTC_IPRH_INT43          (0x00000800)
#define MCF5282_INTC_IPRH_INT42          (0x00000400)
#define MCF5282_INTC_IPRH_INT41          (0x00000200)
#define MCF5282_INTC_IPRH_INT40          (0x00000100)
#define MCF5282_INTC_IPRH_INT39          (0x00000080)
#define MCF5282_INTC_IPRH_INT38          (0x00000040)
#define MCF5282_INTC_IPRH_INT37          (0x00000020)
#define MCF5282_INTC_IPRH_INT36          (0x00000010)
#define MCF5282_INTC_IPRH_INT35          (0x00000008)
#define MCF5282_INTC_IPRH_INT34          (0x00000004)
#define MCF5282_INTC_IPRH_INT33          (0x00000002)
#define MCF5282_INTC_IPRH_INT32          (0x00000001)

#define MCF5282_INTC_IPRL_INT31          (0x80000000)
#define MCF5282_INTC_IPRL_INT30          (0x40000000)
#define MCF5282_INTC_IPRL_INT29          (0x20000000)
#define MCF5282_INTC_IPRL_INT28          (0x10000000)
#define MCF5282_INTC_IPRL_INT27          (0x08000000)
#define MCF5282_INTC_IPRL_INT26          (0x04000000)
#define MCF5282_INTC_IPRL_INT25          (0x02000000)
#define MCF5282_INTC_IPRL_INT24          (0x01000000)
#define MCF5282_INTC_IPRL_INT23          (0x00800000)
#define MCF5282_INTC_IPRL_INT22          (0x00400000)
#define MCF5282_INTC_IPRL_INT21          (0x00200000)
#define MCF5282_INTC_IPRL_INT20          (0x00100000)
#define MCF5282_INTC_IPRL_INT19          (0x00080000)
#define MCF5282_INTC_IPRL_INT18          (0x00040000)
#define MCF5282_INTC_IPRL_INT17          (0x00020000)
#define MCF5282_INTC_IPRL_INT16          (0x00010000)
#define MCF5282_INTC_IPRL_INT15          (0x00008000)
#define MCF5282_INTC_IPRL_INT14          (0x00004000)
#define MCF5282_INTC_IPRL_INT13          (0x00002000)
#define MCF5282_INTC_IPRL_INT12          (0x00001000)
#define MCF5282_INTC_IPRL_INT11          (0x00000800)
#define MCF5282_INTC_IPRL_INT10          (0x00000400)
#define MCF5282_INTC_IPRL_INT9           (0x00000200)
#define MCF5282_INTC_IPRL_INT8           (0x00000100)
#define MCF5282_INTC_IPRL_INT7           (0x00000080)
#define MCF5282_INTC_IPRL_INT6           (0x00000040)
#define MCF5282_INTC_IPRL_INT5           (0x00000020)
#define MCF5282_INTC_IPRL_INT4           (0x00000010)
#define MCF5282_INTC_IPRL_INT3           (0x00000008)
#define MCF5282_INTC_IPRL_INT2           (0x00000004)
#define MCF5282_INTC_IPRL_INT1           (0x00000002)

#define MCF5282_INTC_IMRH_INT63          (0x80000000)
#define MCF5282_INTC_IMRH_INT62          (0x40000000)
#define MCF5282_INTC_IMRH_INT61          (0x20000000)
#define MCF5282_INTC_IMRH_INT60          (0x10000000)
#define MCF5282_INTC_IMRH_INT59          (0x08000000)
#define MCF5282_INTC_IMRH_INT58          (0x04000000)
#define MCF5282_INTC_IMRH_INT57          (0x02000000)
#define MCF5282_INTC_IMRH_INT56          (0x01000000)
#define MCF5282_INTC_IMRH_INT55          (0x00800000)
#define MCF5282_INTC_IMRH_INT54          (0x00400000)
#define MCF5282_INTC_IMRH_INT53          (0x00200000)
#define MCF5282_INTC_IMRH_INT52          (0x00100000)
#define MCF5282_INTC_IMRH_INT51          (0x00080000)
#define MCF5282_INTC_IMRH_INT50          (0x00040000)
#define MCF5282_INTC_IMRH_INT49          (0x00020000)
#define MCF5282_INTC_IMRH_INT48          (0x00010000)
#define MCF5282_INTC_IMRH_INT47          (0x00008000)
#define MCF5282_INTC_IMRH_INT46          (0x00004000)
#define MCF5282_INTC_IMRH_INT45          (0x00002000)
#define MCF5282_INTC_IMRH_INT44          (0x00001000)
#define MCF5282_INTC_IMRH_INT43          (0x00000800)
#define MCF5282_INTC_IMRH_INT42          (0x00000400)
#define MCF5282_INTC_IMRH_INT41          (0x00000200)
#define MCF5282_INTC_IMRH_INT40          (0x00000100)
#define MCF5282_INTC_IMRH_INT39          (0x00000080)
#define MCF5282_INTC_IMRH_INT38          (0x00000040)
#define MCF5282_INTC_IMRH_INT37          (0x00000020)
#define MCF5282_INTC_IMRH_INT36          (0x00000010)
#define MCF5282_INTC_IMRH_INT35          (0x00000008)
#define MCF5282_INTC_IMRH_INT34          (0x00000004)
#define MCF5282_INTC_IMRH_INT33          (0x00000002)
#define MCF5282_INTC_IMRH_INT32          (0x00000001)

#define MCF5282_INTC_IMRL_INT31          (0x80000000)
#define MCF5282_INTC_IMRL_INT30          (0x40000000)
#define MCF5282_INTC_IMRL_INT29          (0x20000000)
#define MCF5282_INTC_IMRL_INT28          (0x10000000)
#define MCF5282_INTC_IMRL_INT27          (0x08000000)
#define MCF5282_INTC_IMRL_INT26          (0x04000000)
#define MCF5282_INTC_IMRL_INT25          (0x02000000)
#define MCF5282_INTC_IMRL_INT24          (0x01000000)
#define MCF5282_INTC_IMRL_INT23          (0x00800000)
#define MCF5282_INTC_IMRL_INT22          (0x00400000)
#define MCF5282_INTC_IMRL_INT21          (0x00200000)
#define MCF5282_INTC_IMRL_INT20          (0x00100000)
#define MCF5282_INTC_IMRL_INT19          (0x00080000)
#define MCF5282_INTC_IMRL_INT18          (0x00040000)
#define MCF5282_INTC_IMRL_INT17          (0x00020000)
#define MCF5282_INTC_IMRL_INT16          (0x00010000)
#define MCF5282_INTC_IMRL_INT15          (0x00008000)
#define MCF5282_INTC_IMRL_INT14          (0x00004000)
#define MCF5282_INTC_IMRL_INT13          (0x00002000)
#define MCF5282_INTC_IMRL_INT12          (0x00001000)
#define MCF5282_INTC_IMRL_INT11          (0x00000800)
#define MCF5282_INTC_IMRL_INT10          (0x00000400)
#define MCF5282_INTC_IMRL_INT9           (0x00000200)
#define MCF5282_INTC_IMRL_INT8           (0x00000100)
#define MCF5282_INTC_IMRL_INT7           (0x00000080)
#define MCF5282_INTC_IMRL_INT6           (0x00000040)
#define MCF5282_INTC_IMRL_INT5           (0x00000020)
#define MCF5282_INTC_IMRL_INT4           (0x00000010)
#define MCF5282_INTC_IMRL_INT3           (0x00000008)
#define MCF5282_INTC_IMRL_INT2           (0x00000004)
#define MCF5282_INTC_IMRL_INT1           (0x00000002)
#define MCF5282_INTC_IMRL_MASKALL        (0x00000001)

#define MCF5282_INTC_INTFRCH_INT63       (0x80000000)
#define MCF5282_INTC_INTFRCH_INT62       (0x40000000)
#define MCF5282_INTC_INTFRCH_INT61       (0x20000000)
#define MCF5282_INTC_INTFRCH_INT60       (0x10000000)
#define MCF5282_INTC_INTFRCH_INT59       (0x08000000)
#define MCF5282_INTC_INTFRCH_INT58       (0x04000000)
#define MCF5282_INTC_INTFRCH_INT57       (0x02000000)
#define MCF5282_INTC_INTFRCH_INT56       (0x01000000)
#define MCF5282_INTC_INTFRCH_INT55       (0x00800000)
#define MCF5282_INTC_INTFRCH_INT54       (0x00400000)
#define MCF5282_INTC_INTFRCH_INT53       (0x00200000)
#define MCF5282_INTC_INTFRCH_INT52       (0x00100000)
#define MCF5282_INTC_INTFRCH_INT51       (0x00080000)
#define MCF5282_INTC_INTFRCH_INT50       (0x00040000)
#define MCF5282_INTC_INTFRCH_INT49       (0x00020000)
#define MCF5282_INTC_INTFRCH_INT48       (0x00010000)
#define MCF5282_INTC_INTFRCH_INT47       (0x00008000)
#define MCF5282_INTC_INTFRCH_INT46       (0x00004000)
#define MCF5282_INTC_INTFRCH_INT45       (0x00002000)
#define MCF5282_INTC_INTFRCH_INT44       (0x00001000)
#define MCF5282_INTC_INTFRCH_INT43       (0x00000800)
#define MCF5282_INTC_INTFRCH_INT42       (0x00000400)
#define MCF5282_INTC_INTFRCH_INT41       (0x00000200)
#define MCF5282_INTC_INTFRCH_INT40       (0x00000100)
#define MCF5282_INTC_INTFRCH_INT39       (0x00000080)
#define MCF5282_INTC_INTFRCH_INT38       (0x00000040)
#define MCF5282_INTC_INTFRCH_INT37       (0x00000020)
#define MCF5282_INTC_INTFRCH_INT36       (0x00000010)
#define MCF5282_INTC_INTFRCH_INT35       (0x00000008)
#define MCF5282_INTC_INTFRCH_INT34       (0x00000004)
#define MCF5282_INTC_INTFRCH_INT33       (0x00000002)
#define MCF5282_INTC_INTFRCH_INT32       (0x00000001)

#define MCF5282_INTC_INTFRCL_INT31       (0x80000000)
#define MCF5282_INTC_INTFRCL_INT30       (0x40000000)
#define MCF5282_INTC_INTFRCL_INT29       (0x20000000)
#define MCF5282_INTC_INTFRCL_INT28       (0x10000000)
#define MCF5282_INTC_INTFRCL_INT27       (0x08000000)
#define MCF5282_INTC_INTFRCL_INT26       (0x04000000)
#define MCF5282_INTC_INTFRCL_INT25       (0x02000000)
#define MCF5282_INTC_INTFRCL_INT24       (0x01000000)
#define MCF5282_INTC_INTFRCL_INT23       (0x00800000)
#define MCF5282_INTC_INTFRCL_INT22       (0x00400000)
#define MCF5282_INTC_INTFRCL_INT21       (0x00200000)
#define MCF5282_INTC_INTFRCL_INT20       (0x00100000)
#define MCF5282_INTC_INTFRCL_INT19       (0x00080000)
#define MCF5282_INTC_INTFRCL_INT18       (0x00040000)
#define MCF5282_INTC_INTFRCL_INT17       (0x00020000)
#define MCF5282_INTC_INTFRCL_INT16       (0x00010000)
#define MCF5282_INTC_INTFRCL_INT15       (0x00008000)
#define MCF5282_INTC_INTFRCL_INT14       (0x00004000)
#define MCF5282_INTC_INTFRCL_INT13       (0x00002000)
#define MCF5282_INTC_INTFRCL_INT12       (0x00001000)
#define MCF5282_INTC_INTFRCL_INT11       (0x00000800)
#define MCF5282_INTC_INTFRCL_INT10       (0x00000400)
#define MCF5282_INTC_INTFRCL_INT9        (0x00000200)
#define MCF5282_INTC_INTFRCL_INT8        (0x00000100)
#define MCF5282_INTC_INTFRCL_INT7        (0x00000080)
#define MCF5282_INTC_INTFRCL_INT6        (0x00000040)
#define MCF5282_INTC_INTFRCL_INT5        (0x00000020)
#define MCF5282_INTC_INTFRCL_INT4        (0x00000010)
#define MCF5282_INTC_INTFRCL_INT3        (0x00000008)
#define MCF5282_INTC_INTFRCL_INT2        (0x00000004)
#define MCF5282_INTC_INTFRCL_INT1        (0x00000002)

#define MCF5282_INTC_IRLR_IRQ7           (0x80)
#define MCF5282_INTC_IRLR_IRQ6           (0x40)
#define MCF5282_INTC_IRLR_IRQ5           (0x20)
#define MCF5282_INTC_IRLR_IRQ4           (0x10)
#define MCF5282_INTC_IRLR_IRQ3           (0x08)
#define MCF5282_INTC_IRLR_IRQ2           (0x04)
#define MCF5282_INTC_IRLR_IRQ1           (0x02)

#define MCF5282_INTC_ICR_IL(x)           (((x) & 0x07) << 3) /* level (IPL) */
#define MCF5282_INTC_ICR_IP(x)           (((x) & 0x07) << 0) /* priority */

/*********************************************************************
*
* Global Interrupt Acknowledge Cycle (GIAC) Registers
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_GIAC_GSWIACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0FE0]))
#define MCF5282_GIAC_GL1IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0FE4]))
#define MCF5282_GIAC_GL2IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0FE8]))
#define MCF5282_GIAC_GL3IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0FEC]))
#define MCF5282_GIAC_GL4IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0FF0]))
#define MCF5282_GIAC_GL5IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0FF4]))
#define MCF5282_GIAC_GL6IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0FF8]))
#define MCF5282_GIAC_GL7IACK  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x0FFC]))

/* Bit level definitions and macros */

/* To do - add bit level definintions */

/*********************************************************************
*
* Fast Ethernet Controller (FEC) Module
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_FEC_EIR       (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1004]))
#define MCF5282_FEC_EIMR      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1008]))
#define MCF5282_FEC_RDAR      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1010]))
#define MCF5282_FEC_TDAR      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1014]))
#define MCF5282_FEC_ECR       (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1024]))
#define MCF5282_FEC_MMFR      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1040]))
#define MCF5282_FEC_MSCR      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1044]))
#define MCF5282_FEC_MIBC      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1064]))
#define MCF5282_FEC_RCR       (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1084]))
#define MCF5282_FEC_TCR       (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x10C4]))
#define MCF5282_FEC_PALR      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x10E4]))
#define MCF5282_FEC_PAUR      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x10E8]))
#define MCF5282_FEC_OPD       (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x10EC]))
#define MCF5282_FEC_IAUR      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1118]))
#define MCF5282_FEC_IALR      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x111C]))
#define MCF5282_FEC_GAUR      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1120]))
#define MCF5282_FEC_GALR      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1124]))
#define MCF5282_FEC_TFWR      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1144]))
#define MCF5282_FEC_FRBR      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x114C]))
#define MCF5282_FEC_FRSR      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1150]))
#define MCF5282_FEC_ERDSR     (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1180]))
#define MCF5282_FEC_ETDSR     (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1184]))
#define MCF5282_FEC_EMRBR     (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1188]))

#define MCF5282_FEC_RMON_T_DROP         (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1200]))
#define MCF5282_FEC_RMON_T_PACKETS      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1204]))
#define MCF5282_FEC_RMON_T_BC_PKT       (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1208]))
#define MCF5282_FEC_RMON_T_MC_PKT       (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x120C]))
#define MCF5282_FEC_RMON_T_CRC_ALIGN    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1210]))
#define MCF5282_FEC_RMON_T_UNDERSIZE    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1214]))
#define MCF5282_FEC_RMON_T_OVERSIZE     (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1218]))
#define MCF5282_FEC_RMON_T_FRAG         (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x121C]))
#define MCF5282_FEC_RMON_T_JAB          (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1220]))
#define MCF5282_FEC_RMON_T_COL          (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1224]))
#define MCF5282_FEC_RMON_T_P64          (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1228]))
#define MCF5282_FEC_RMON_T_P65TO127     (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x122C]))
#define MCF5282_FEC_RMON_T_P128TO255    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1230]))
#define MCF5282_FEC_RMON_T_P256TO511    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1234]))
#define MCF5282_FEC_RMON_T_P512TO1023   (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1238]))
#define MCF5282_FEC_RMON_T_P1024TO2047  (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x123C]))
#define MCF5282_FEC_RMON_T_P_GTE2048    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1240]))
#define MCF5282_FEC_RMON_T_OCTETS       (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1244]))
#define MCF5282_FEC_IEEE_T_DROP         (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1248]))
#define MCF5282_FEC_IEEE_T_FRAME_OK     (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x124C]))
#define MCF5282_FEC_IEEE_T_1COL         (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1250]))
#define MCF5282_FEC_IEEE_T_MCOL         (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1254]))
#define MCF5282_FEC_IEEE_T_DEF          (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1258]))
#define MCF5282_FEC_IEEE_T_LCOL         (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x125C]))
#define MCF5282_FEC_IEEE_T_EXCOL        (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1260]))
#define MCF5282_FEC_IEEE_T_MACERR       (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1264]))
#define MCF5282_FEC_IEEE_T_CSERR        (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1268]))
#define MCF5282_FEC_IEEE_T_SQE          (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x126C]))
#define MCF5282_FEC_IEEE_T_FDXFC        (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1270]))
#define MCF5282_FEC_IEEE_T_OCTETS_OK    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1274]))
#define MCF5282_FEC_RMON_R_PACKETS      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1284]))
#define MCF5282_FEC_RMON_R_BC_PKT       (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1288]))
#define MCF5282_FEC_RMON_R_MC_PKT       (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x128C]))
#define MCF5282_FEC_RMON_R_CRC_ALIGN    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1290]))
#define MCF5282_FEC_RMON_R_UNDERSIZE    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1294]))
#define MCF5282_FEC_RMON_R_OVERSIZE     (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1298]))
#define MCF5282_FEC_RMON_R_FRAG         (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x129C]))
#define MCF5282_FEC_RMON_R_JAB          (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x12A0]))
#define MCF5282_FEC_RMON_R_RESVD_0      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x12A4]))
#define MCF5282_FEC_RMON_R_P64          (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x12A8]))
#define MCF5282_FEC_RMON_R_P65T0127     (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x12AC]))
#define MCF5282_FEC_RMON_R_P128TO255    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x12B0]))
#define MCF5282_FEC_RMON_R_P256TO511    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x12B4]))
#define MCF5282_FEC_RMON_R_P512TO1023   (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x12B8]))
#define MCF5282_FEC_RMON_R_P1024TO2047  (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x12BC]))
#define MCF5282_FEC_RMON_R_GTE2048      (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x12C0]))
#define MCF5282_FEC_RMON_R_OCTETS       (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x12C4]))
#define MCF5282_FEC_IEEE_R_DROP         (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x12C8]))
#define MCF5282_FEC_IEEE_R_FRAME_OK     (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x12CC]))
#define MCF5282_FEC_IEEE_R_CRC          (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x12D0]))
#define MCF5282_FEC_IEEE_R_ALIGN        (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x12D4]))
#define MCF5282_FEC_IEEE_R_MACERR       (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x12D8]))
#define MCF5282_FEC_IEEE_R_FDXFC        (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x12DC]))
#define MCF5282_FEC_IEEE_R_OCTETS_OK    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x12E0]))

/* Bit level definitions and macros */
#define MCF5282_FEC_EIR_HBERR               (0x80000000)
#define MCF5282_FEC_EIR_BABR                (0x40000000)
#define MCF5282_FEC_EIR_BABT                (0x20000000)
#define MCF5282_FEC_EIR_GRA                 (0x10000000)
#define MCF5282_FEC_EIR_TXF                 (0x08000000)
#define MCF5282_FEC_EIR_TXB                 (0x04000000)
#define MCF5282_FEC_EIR_RXF                 (0x02000000)
#define MCF5282_FEC_EIR_RXB                 (0x01000000)
#define MCF5282_FEC_EIR_MII                 (0x00800000)
#define MCF5282_FEC_EIR_EBERR               (0x00400000)
#define MCF5282_FEC_EIR_LC                  (0x00200000)
#define MCF5282_FEC_EIR_RL                  (0x00100000)
#define MCF5282_FEC_EIR_UN                  (0x00080000)

#define MCF5282_FEC_EIMR_HBERR              (0x80000000)
#define MCF5282_FEC_EIMR_BABR               (0x40000000)
#define MCF5282_FEC_EIMR_BABT               (0x20000000)
#define MCF5282_FEC_EIMR_GRA                (0x10000000)
#define MCF5282_FEC_EIMR_TXF                (0x08000000)
#define MCF5282_FEC_EIMR_TXB                (0x04000000)
#define MCF5282_FEC_EIMR_RXF                (0x02000000)
#define MCF5282_FEC_EIMR_RXB                (0x01000000)
#define MCF5282_FEC_EIMR_MII                (0x00800000)
#define MCF5282_FEC_EIMR_EBERR              (0x00400000)
#define MCF5282_FEC_EIMR_LC                 (0x00200000)
#define MCF5282_FEC_EIMR_RL                 (0x00100000)
#define MCF5282_FEC_EIMR_UN                 (0x00080000)

#define MCF5282_FEC_RDAR_R_DES_ACTIVE       (0x01000000)

#define MCF5282_FEC_TDAR_X_DES_ACTIVE       (0x01000000)

#define MCF5282_FEC_ECR_ETHER_EN            (0x00000002)
#define MCF5282_FEC_ECR_RESET               (0x00000001)

#define MCF5282_FEC_MMFR_ST                 (0x40000000)
#define MCF5282_FEC_MMFR_OP_RD              (0x20000000)
#define MCF5282_FEC_MMFR_OP_WR              (0x10000000)
#define MCF5282_FEC_MMFR_PA(x)              (((x) & 0x1F) << 23)
#define MCF5282_FEC_MMFR_RA(x)              (((x) & 0x1F) << 18)
#define MCF5282_FEC_MMFR_TA                 (0x00020000)
#define MCF5282_FEC_MMFR_DATA(x)            (((x) & 0xFFFF))

#define MCF5282_FEC_MSCR_DIS_PREAMBLE       (0x00000008)
#define MCF5282_FEC_MSCR_MII_SPEED(x)       (((x) & 0x1F) << 1)

#define MCF5282_FEC_MIBC_MIB_DISABLE        (0x80000000)
#define MCF5282_FEC_MIBC_MIB_IDLE           (0x40000000)

#define MCF5282_FEC_RCR_MAX_FL(x)           (((x) & 0x07FF) << 16)
#define MCF5282_FEC_RCR_FCE                 (0x00000020)
#define MCF5282_FEC_RCR_BC_REJ              (0x00000010)
#define MCF5282_FEC_RCR_PROM                (0x00000008)
#define MCF5282_FEC_RCR_MII_MODE            (0x00000004)
#define MCF5282_FEC_RCR_DRT                 (0x00000002)
#define MCF5282_FEC_RCR_LOOP                (0x00000001)

#define MCF5282_FEC_TCR_RFC_PAUSE           (0x00000010)
#define MCF5282_FEC_TCR_TFC_PAUSE           (0x00000008)
#define MCF5282_FEC_TCR_FDEN                (0x00000004)
#define MCF5282_FEC_TCR_HBC                 (0x00000002)
#define MCF5282_FEC_TCR_GTS                 (0x00000001)

#define MCF5282_FEC_PALR_BYTE0(x)           (((x) & 0xFF) << 24)
#define MCF5282_FEC_PALR_BYTE1(x)           (((x) & 0xFF) << 16)
#define MCF5282_FEC_PALR_BYTE2(x)           (((x) & 0xFF) <<  8)
#define MCF5282_FEC_PALR_BYTE3(x)           (((x) & 0xFF) <<  0)

#define MCF5282_FEC_PAUR_BYTE4(x)           (((x) & 0xFF) << 24)
#define MCF5282_FEC_PAUR_BYTE5(x)           (((x) & 0xFF) << 16)

#define MCF5282_FEC_OPD_PAUSE_DUR(x)        (((x) & 0xFFFF))

#define MCF5282_FEC_TFWR_X_WMRK_64          (0x00000001)
#define MCF5282_FEC_TFWR_X_WMRK_128         (0x00000002)
#define MCF5282_FEC_TFWR_X_WMRK_192         (0x00000003)

#define MCF5282_FEC_EMRBR_R_BUF_SIZE(x)     (((x) & 0x7F) <<  4)

#define MCF5282_FEC_TxBD_R                  0x8000
#define MCF5282_FEC_TxBD_INUSE              0x4000
#define MCF5282_FEC_TxBD_TO1                0x4000
#define MCF5282_FEC_TxBD_W                  0x2000
#define MCF5282_FEC_TxBD_TO2                0x1000
#define MCF5282_FEC_TxBD_L                  0x0800
#define MCF5282_FEC_TxBD_TC                 0x0400
#define MCF5282_FEC_TxBD_DEF                0x0200
#define MCF5282_FEC_TxBD_HB                 0x0100
#define MCF5282_FEC_TxBD_LC                 0x0080
#define MCF5282_FEC_TxBD_RL                 0x0040
#define MCF5282_FEC_TxBD_UN                 0x0002
#define MCF5282_FEC_TxBD_CSL                0x0001

#define MCF5282_FEC_RxBD_E                  0x8000
#define MCF5282_FEC_RxBD_INUSE              0x4000
#define MCF5282_FEC_RxBD_R01                0x4000
#define MCF5282_FEC_RxBD_W                  0x2000
#define MCF5282_FEC_RxBD_R02                0x1000
#define MCF5282_FEC_RxBD_L                  0x0800
#define MCF5282_FEC_RxBD_M                  0x0100
#define MCF5282_FEC_RxBD_BC                 0x0080
#define MCF5282_FEC_RxBD_MC                 0x0040
#define MCF5282_FEC_RxBD_LG                 0x0020
#define MCF5282_FEC_RxBD_NO                 0x0010
#define MCF5282_FEC_RxBD_CR                 0x0004
#define MCF5282_FEC_RxBD_OV                 0x0002
#define MCF5282_FEC_RxBD_TR                 0x0001

/*********************************************************************
*
* General Purpose I/O (GPIO) Module
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_GPIO_PORTA      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100000]))
#define MCF5282_GPIO_PORTB      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100001]))
#define MCF5282_GPIO_PORTC      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100002]))
#define MCF5282_GPIO_PORTD      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100003]))
#define MCF5282_GPIO_PORTE      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100004]))
#define MCF5282_GPIO_PORTF      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100005]))
#define MCF5282_GPIO_PORTG      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100006]))
#define MCF5282_GPIO_PORTH      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100007]))
#define MCF5282_GPIO_PORTJ      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100008]))
#define MCF5282_GPIO_PORTDD     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100009]))
#define MCF5282_GPIO_PORTEH     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10000A]))
#define MCF5282_GPIO_PORTEL     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10000B]))
#define MCF5282_GPIO_PORTAS     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10000C]))
#define MCF5282_GPIO_PORTQS     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10000D]))
#define MCF5282_GPIO_PORTSD     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10000E]))
#define MCF5282_GPIO_PORTTC     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10000F]))
#define MCF5282_GPIO_PORTTD     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100010]))
#define MCF5282_GPIO_PORTUA     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100011]))

#define MCF5282_GPIO_DDRA       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100014]))
#define MCF5282_GPIO_DDRB       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100015]))
#define MCF5282_GPIO_DDRC       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100016]))
#define MCF5282_GPIO_DDRD       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100017]))
#define MCF5282_GPIO_DDRE       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100018]))
#define MCF5282_GPIO_DDRF       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100019]))
#define MCF5282_GPIO_DDRG       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10001A]))
#define MCF5282_GPIO_DDRH       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10001B]))
#define MCF5282_GPIO_DDRJ       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10001C]))
#define MCF5282_GPIO_DDRDD      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10001D]))
#define MCF5282_GPIO_DDREH      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10001E]))
#define MCF5282_GPIO_DDREL      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10001F]))
#define MCF5282_GPIO_DDRAS      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100020]))
#define MCF5282_GPIO_DDRQS      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100021]))
#define MCF5282_GPIO_DDRSD      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100022]))
#define MCF5282_GPIO_DDRTC      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100023]))
#define MCF5282_GPIO_DDRTD      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100024]))
#define MCF5282_GPIO_DDRUA      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100025]))

#define MCF5282_GPIO_PORTAP     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100028]))
#define MCF5282_GPIO_PORTBP     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100029]))
#define MCF5282_GPIO_PORTCP     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10002A]))
#define MCF5282_GPIO_PORTDP     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10002B]))
#define MCF5282_GPIO_PORTEP     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10002C]))
#define MCF5282_GPIO_PORTFP     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10002D]))
#define MCF5282_GPIO_PORTGP     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10002E]))
#define MCF5282_GPIO_PORTHP     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10002F]))
#define MCF5282_GPIO_PORTJP     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100030]))
#define MCF5282_GPIO_PORTDDP    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100031]))
#define MCF5282_GPIO_PORTEHP    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100032]))
#define MCF5282_GPIO_PORTELP    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100033]))
#define MCF5282_GPIO_PORTASP    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100034]))
#define MCF5282_GPIO_PORTQSP    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100035]))
#define MCF5282_GPIO_PORTSDP    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100036]))
#define MCF5282_GPIO_PORTTCP    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100037]))
#define MCF5282_GPIO_PORTTDP    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100038]))
#define MCF5282_GPIO_PORTUAP    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100039]))

#define MCF5282_GPIO_SETA       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100028]))
#define MCF5282_GPIO_SETB       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100029]))
#define MCF5282_GPIO_SETC       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10002A]))
#define MCF5282_GPIO_SETD       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10002B]))
#define MCF5282_GPIO_SETE       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10002C]))
#define MCF5282_GPIO_SETF       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10002D]))
#define MCF5282_GPIO_SETG       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10002E]))
#define MCF5282_GPIO_SETH       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10002F]))
#define MCF5282_GPIO_SETJ       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100030]))
#define MCF5282_GPIO_SETDD      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100031]))
#define MCF5282_GPIO_SETEH      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100032]))
#define MCF5282_GPIO_SETEL      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100033]))
#define MCF5282_GPIO_SETAS      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100034]))
#define MCF5282_GPIO_SETQS      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100035]))
#define MCF5282_GPIO_SETSD      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100036]))
#define MCF5282_GPIO_SETTC      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100037]))
#define MCF5282_GPIO_SETTD      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100038]))
#define MCF5282_GPIO_SETUA      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100039]))

#define MCF5282_GPIO_CLRA       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10003C]))
#define MCF5282_GPIO_CLRB       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10003D]))
#define MCF5282_GPIO_CLRC       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10003E]))
#define MCF5282_GPIO_CLRD       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10003F]))
#define MCF5282_GPIO_CLRE       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100040]))
#define MCF5282_GPIO_CLRF       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100041]))
#define MCF5282_GPIO_CLRG       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100042]))
#define MCF5282_GPIO_CLRH       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100043]))
#define MCF5282_GPIO_CLRJ       (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100044]))
#define MCF5282_GPIO_CLRDD      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100045]))
#define MCF5282_GPIO_CLREH      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100046]))
#define MCF5282_GPIO_CLREL      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100047]))
#define MCF5282_GPIO_CLRAS      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100048]))
#define MCF5282_GPIO_CLRQS      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100049]))
#define MCF5282_GPIO_CLRSD      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10004A]))
#define MCF5282_GPIO_CLRTC      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10004B]))
#define MCF5282_GPIO_CLRTD      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10004C]))
#define MCF5282_GPIO_CLRUA      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10004D]))

#define MCF5282_GPIO_PBCDPAR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100050]))
#define MCF5282_GPIO_PFPAR      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100051]))
#define MCF5282_GPIO_PEPAR      (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x100052]))
#define MCF5282_GPIO_PJPAR      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100054]))
#define MCF5282_GPIO_PSDPAR     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100055]))
#define MCF5282_GPIO_PASPAR     (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x100056]))
#define MCF5282_GPIO_PEHLPAR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100058]))
#define MCF5282_GPIO_PQSPAR     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x100059]))
#define MCF5282_GPIO_PTCPAR     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10005A]))
#define MCF5282_GPIO_PTDPAR     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10005B]))
#define MCF5282_GPIO_PUAPAR     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x10005C]))

/* Bit level definitions and macros */
#define MCF5282_GPIO_PORTx7                  (0x80)
#define MCF5282_GPIO_PORTx6                  (0x40)
#define MCF5282_GPIO_PORTx5                  (0x20)
#define MCF5282_GPIO_PORTx4                  (0x10)
#define MCF5282_GPIO_PORTx3                  (0x08)
#define MCF5282_GPIO_PORTx2                  (0x04)
#define MCF5282_GPIO_PORTx1                  (0x02)
#define MCF5282_GPIO_PORTx0                  (0x01)
#define MCF5282_GPIO_PORTx(x)                (0x01 << x)

#define MCF5282_GPIO_DDRx7                   (0x80)
#define MCF5282_GPIO_DDRx6                   (0x40)
#define MCF5282_GPIO_DDRx5                   (0x20)
#define MCF5282_GPIO_DDRx4                   (0x10)
#define MCF5282_GPIO_DDRx3                   (0x08)
#define MCF5282_GPIO_DDRx2                   (0x04)
#define MCF5282_GPIO_DDRx1                   (0x02)
#define MCF5282_GPIO_DDRx0                   (0x01)
#define MCF5282_GPIO_DDRx(x)                 (0x01 << x)

#define MCF5282_GPIO_PORTxP7                 (0x80)
#define MCF5282_GPIO_PORTxP6                 (0x40)
#define MCF5282_GPIO_PORTxP5                 (0x20)
#define MCF5282_GPIO_PORTxP4                 (0x10)
#define MCF5282_GPIO_PORTxP3                 (0x08)
#define MCF5282_GPIO_PORTxP2                 (0x04)
#define MCF5282_GPIO_PORTxP1                 (0x02)
#define MCF5282_GPIO_PORTxP0                 (0x01)
#define MCF5282_GPIO_PORTxP(x)               (0x01 << x)

#define MCF5282_GPIO_SETx7                   (0x80)
#define MCF5282_GPIO_SETx6                   (0x40)
#define MCF5282_GPIO_SETx5                   (0x20)
#define MCF5282_GPIO_SETx4                   (0x10)
#define MCF5282_GPIO_SETx3                   (0x08)
#define MCF5282_GPIO_SETx2                   (0x04)
#define MCF5282_GPIO_SETx1                   (0x02)
#define MCF5282_GPIO_SETx0                   (0x01)
#define MCF5282_GPIO_SETx(x)                 (0x01 << x)

#define MCF5282_GPIO_CLRx7                   (0x80)
#define MCF5282_GPIO_CLRx6                   (0x40)
#define MCF5282_GPIO_CLRx5                   (0x20)
#define MCF5282_GPIO_CLRx4                   (0x10)
#define MCF5282_GPIO_CLRx3                   (0x08)
#define MCF5282_GPIO_CLRx2                   (0x04)
#define MCF5282_GPIO_CLRx1                   (0x02)
#define MCF5282_GPIO_CLRx0                   (0x01)
#define MCF5282_GPIO_CLRx(x)                 (0x01 << x)

#define MCF5282_GPIO_PBCDPAR_PBPA            (0x80)
#define MCF5282_GPIO_PBCDPAR_PCDPA           (0x40)

#define MCF5282_GPIO_PEPAR_PEPA7             (0x4000)
#define MCF5282_GPIO_PEPAR_PEPA6             (0x1000)
#define MCF5282_GPIO_PEPAR_PEPA5             (0x0400)
#define MCF5282_GPIO_PEPAR_PEPA4             (0x0100)
#define MCF5282_GPIO_PEPAR_PEPA3             (0x0040)
#define MCF5282_GPIO_PEPAR_PEPA2             (0x0010)
#define MCF5282_GPIO_PEPAR_PEPA1(x)          (((x) & 0x3) << 2)
#define MCF5282_GPIO_PEPAR_PEPA0(x)          (((x) & 0x3) << 0)

#define MCF5282_GPIO_PFPAR_PFPA7             (0x80)
#define MCF5282_GPIO_PFPAR_PFPA6             (0x40)
#define MCF5282_GPIO_PFPAR_PFPA5             (0x20)

#define MCF5282_GPIO_PJPAR_PJPA7             (0x80)
#define MCF5282_GPIO_PJPAR_PJPA6             (0x40)
#define MCF5282_GPIO_PJPAR_PJPA5             (0x20)
#define MCF5282_GPIO_PJPAR_PJPA4             (0x10)
#define MCF5282_GPIO_PJPAR_PJPA3             (0x08)
#define MCF5282_GPIO_PJPAR_PJPA2             (0x04)
#define MCF5282_GPIO_PJPAR_PJPA1             (0x02)
#define MCF5282_GPIO_PJPAR_PJPA0             (0x01)
#define MCF5282_GPIO_PJPAR_PJPA(x)           (0x01 << x)

#define MCF5282_GPIO_PSDPAR_PSDPA            (0x80)

#define MCF5282_GPIO_PASPAR_PASPA5(x)        (((x) & 0x3) << 10)
#define MCF5282_GPIO_PASPAR_PASPA4(x)        (((x) & 0x3) <<  8)
#define MCF5282_GPIO_PASPAR_PASPA3(x)        (((x) & 0x3) <<  6)
#define MCF5282_GPIO_PASPAR_PASPA2(x)        (((x) & 0x3) <<  4)
#define MCF5282_GPIO_PASPAR_PASPA1(x)        (((x) & 0x3) <<  2)
#define MCF5282_GPIO_PASPAR_PASPA0(x)        (((x) & 0x3) <<  0)

#define MCF5282_GPIO_PEHLPAR_PEHPA           (0x80)
#define MCF5282_GPIO_PEHLPAR_PELPA           (0x40)

#define MCF5282_GPIO_PQSPAR_PQSPA6           (0x40)
#define MCF5282_GPIO_PQSPAR_PQSPA5           (0x20)
#define MCF5282_GPIO_PQSPAR_PQSPA4           (0x10)
#define MCF5282_GPIO_PQSPAR_PQSPA3           (0x08)
#define MCF5282_GPIO_PQSPAR_PQSPA2           (0x04)
#define MCF5282_GPIO_PQSPAR_PQSPA1           (0x02)
#define MCF5282_GPIO_PQSPAR_PQSPA0           (0x01)
#define MCF5282_GPIO_PQSPAR_PQSPA(x)         (0x01 << x)

#define MCF5282_GPIO_PTCPAR_PTCPA3(x)        (((x) & 0x3) << 6)
#define MCF5282_GPIO_PTCPAR_PTCPA2(x)        (((x) & 0x3) << 4)
#define MCF5282_GPIO_PTCPAR_PTCPA1(x)        (((x) & 0x3) << 2)
#define MCF5282_GPIO_PTCPAR_PTCPA0(x)        (((x) & 0x3) << 0)

#define MCF5282_GPIO_PTDPAR_PTDPA3(x)        (((x) & 0x3) << 6)
#define MCF5282_GPIO_PTDPAR_PTDPA2(x)        (((x) & 0x3) << 4)
#define MCF5282_GPIO_PTDPAR_PTDPA1(x)        (((x) & 0x3) << 2)
#define MCF5282_GPIO_PTDPAR_PTDPA0(x)        (((x) & 0x3) << 0)

#define MCF5282_GPIO_PUAPAR_PUAPA3           (0x08)
#define MCF5282_GPIO_PUAPAR_PUAPA2           (0x04)
#define MCF5282_GPIO_PUAPAR_PUAPA1           (0x02)
#define MCF5282_GPIO_PUAPAR_PUAPA0           (0x01)

/*********************************************************************
*
* Reset Controller Module
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_RESET_RCR        (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x110000]))
#define MCF5282_RESET_RSR        (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x110001]))

/* Bit level definitions and macros */
#define MCF5282_RESET_RCR_SOFTRST      (0x80)
#define MCF5282_RESET_RCR_FRCRSTOUT    (0x40)
#define MCF5282_RESET_RCR_LVDF         (0x10)
#define MCF5282_RESET_RCR_LVDIE        (0x08)
#define MCF5282_RESET_RCR_LVDRE        (0x04)
#define MCF5282_RESET_RCR_LVDE         (0x01)

#define MCF5282_RESET_RSR_LVD          (0x40)
#define MCF5282_RESET_RSR_SOFT         (0x20)
#define MCF5282_RESET_RSR_WDR          (0x10)
#define MCF5282_RESET_RSR_POR          (0x08)
#define MCF5282_RESET_RSR_EXT          (0x04)
#define MCF5282_RESET_RSR_LOC          (0x02)
#define MCF5282_RESET_RSR_LOL          (0x01)
#define MCF5282_RESET_RSR_ALL          (0x7F)

/*********************************************************************
*
* Chip Configuration Module (CCM)
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_CCM_CCR       (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x110004]))
#define MCF5282_CCM_RCON      (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x110008]))
#define MCF5282_CCM_CIR       (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x11000A]))

/* Bit level definitions and macros */
#define MCF5282_CCM_CCR_LOAD                (0x8000)
#define MCF5282_CCM_CCR_MODE(x)             (((x) & 0x0007) << 8)
#define MCF5282_CCM_CCR_SZEN                (0x0040)
#define MCF5282_CCM_CCR_PSTEN               (0x0020)
#define MCF5282_CCM_CCR_BME                 (0x0008)
#define MCF5282_CCM_CCR_BMT(x)              (((x) & 0x0007))

/*********************************************************************
*
* Power Management Module (PMM)
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_PMM_LPICR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x000012]))
#define MCF5282_PMM_LPCR     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x110007]))

/* Bit level definitions and macros */
#define MCF5282_PMM_LPICR_ENBSTOP            (0x80)
#define MCF5282_PMM_LPICR_XLMP_IPL(x)        (((x) & 0x07) << 4)

#define MCF5282_PMM_LPCR_LPMD_STOP           (0xC0)
#define MCF5282_PMM_LPCR_LPMD_WAIT           (0x80)
#define MCF5282_PMM_LPCR_LPMD_DOZE           (0x40)
#define MCF5282_PMM_LPCR_LPMD_RUN            (0x00)
#define MCF5282_PMM_LPCR_STPMD(x)            (((x) & 0x03) << 3)
#define MCF5282_PMM_LPCR_LVDSE               (0x02)

/*********************************************************************
*
* Clock Module
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_CLOCK_SYNCR        (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x120000]))
#define MCF5282_CLOCK_SYNSR        (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x120002]))

/* Bit level definitions and macros */
#define MCF5282_CLOCK_SYNCR_LOLRE            (0x8000)
#define MCF5282_CLOCK_SYNCR_MFD(x)           (((x) & 0x0007) << 12)
#define MCF5282_CLOCK_SYNCR_LOCRE            (0x0800)
#define MCF5282_CLOCK_SYNCR_RFD(x)           (((x) & 0x0007) << 8)
#define MCF5282_CLOCK_SYNCR_LOCEN            (0x0080)
#define MCF5282_CLOCK_SYNCR_DISCLK           (0x0040)
#define MCF5282_CLOCK_SYNCR_FWKUP            (0x0020)
#define MCF5282_CLOCK_SYNCR_STPMD(x)         (((x) & 0x0003) << 2)

#define MCF5282_CLOCK_SYNSR_PLLMODE          (0x80)
#define MCF5282_CLOCK_SYNSR_PLLSEL           (0x40)
#define MCF5282_CLOCK_SYNSR_PLLREF           (0x20)
#define MCF5282_CLOCK_SYNSR_LOCKS            (0x10)
#define MCF5282_CLOCK_SYNSR_LOCK             (0x08)
#define MCF5282_CLOCK_SYNSR_LOCS             (0x04)

/*********************************************************************
*
* Edge Port (EPORT) Module
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_EPORT_EPPAR   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x130000]))
#define MCF5282_EPORT_EPDDR   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x130002]))
#define MCF5282_EPORT_EPIER   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x130003]))
#define MCF5282_EPORT_EPDR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x130004]))
#define MCF5282_EPORT_EPPDR   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x130005]))
#define MCF5282_EPORT_EPFR    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x130006]))

/* Bit level definitions and macros */
#define MCF5282_EPORT_EPPAR_EPPA7_LEVEL     (0x0000)
#define MCF5282_EPORT_EPPAR_EPPA7_RISING    (0x4000)
#define MCF5282_EPORT_EPPAR_EPPA7_FALLING   (0x8000)
#define MCF5282_EPORT_EPPAR_EPPA7_BOTHEDGE  (0xC000)
#define MCF5282_EPORT_EPPAR_EPPA6_LEVEL     (0x0000)
#define MCF5282_EPORT_EPPAR_EPPA6_RISING    (0x1000)
#define MCF5282_EPORT_EPPAR_EPPA6_FALLING   (0x2000)
#define MCF5282_EPORT_EPPAR_EPPA6_BOTHEDGE  (0x3000)
#define MCF5282_EPORT_EPPAR_EPPA5_LEVEL     (0x0000)
#define MCF5282_EPORT_EPPAR_EPPA5_RISING    (0x0400)
#define MCF5282_EPORT_EPPAR_EPPA5_FALLING   (0x0800)
#define MCF5282_EPORT_EPPAR_EPPA5_BOTHEDGE  (0x0C00)
#define MCF5282_EPORT_EPPAR_EPPA4_LEVEL     (0x0000)
#define MCF5282_EPORT_EPPAR_EPPA4_RISING    (0x0100)
#define MCF5282_EPORT_EPPAR_EPPA4_FALLING   (0x0200)
#define MCF5282_EPORT_EPPAR_EPPA4_BOTHEDGE  (0x0300)
#define MCF5282_EPORT_EPPAR_EPPA3_LEVEL     (0x0000)
#define MCF5282_EPORT_EPPAR_EPPA3_RISING    (0x0040)
#define MCF5282_EPORT_EPPAR_EPPA3_FALLING   (0x0080)
#define MCF5282_EPORT_EPPAR_EPPA3_BOTHEDGE  (0x00C0)
#define MCF5282_EPORT_EPPAR_EPPA2_LEVEL     (0x0000)
#define MCF5282_EPORT_EPPAR_EPPA2_RISING    (0x0010)
#define MCF5282_EPORT_EPPAR_EPPA2_FALLING   (0x0020)
#define MCF5282_EPORT_EPPAR_EPPA2_BOTHEDGE  (0x0030)
#define MCF5282_EPORT_EPPAR_EPPA1_LEVEL     (0x0000)
#define MCF5282_EPORT_EPPAR_EPPA1_RISING    (0x0004)
#define MCF5282_EPORT_EPPAR_EPPA1_FALLING   (0x0008)
#define MCF5282_EPORT_EPPAR_EPPA1_BOTHEDGE  (0x000C)

#define MCF5282_EPORT_EPDDR_EPDD7            (0x80)
#define MCF5282_EPORT_EPDDR_EPDD6            (0x40)
#define MCF5282_EPORT_EPDDR_EPDD5            (0x20)
#define MCF5282_EPORT_EPDDR_EPDD4            (0x10)
#define MCF5282_EPORT_EPDDR_EPDD3            (0x08)
#define MCF5282_EPORT_EPDDR_EPDD2            (0x04)
#define MCF5282_EPORT_EPDDR_EPDD1            (0x02)
#define MCF5282_EPORT_EPDDR_EPDD(x)            (0x01 << x)

#define MCF5282_EPORT_EPIER_EPIE7            (0x80)
#define MCF5282_EPORT_EPIER_EPIE6            (0x40)
#define MCF5282_EPORT_EPIER_EPIE5            (0x20)
#define MCF5282_EPORT_EPIER_EPIE4            (0x10)
#define MCF5282_EPORT_EPIER_EPIE3            (0x08)
#define MCF5282_EPORT_EPIER_EPIE2            (0x04)
#define MCF5282_EPORT_EPIER_EPIE1            (0x02)
#define MCF5282_EPORT_EPIER_EPIE(x)            (0x01 << x)

#define MCF5282_EPORT_EPDR_EPD7              (0x80)
#define MCF5282_EPORT_EPDR_EPD6              (0x40)
#define MCF5282_EPORT_EPDR_EPD5              (0x20)
#define MCF5282_EPORT_EPDR_EPD4              (0x10)
#define MCF5282_EPORT_EPDR_EPD3              (0x08)
#define MCF5282_EPORT_EPDR_EPD2              (0x04)
#define MCF5282_EPORT_EPDR_EPD1              (0x02)
#define MCF5282_EPORT_EPDR_EPD(x)              (0x01 << x)

#define MCF5282_EPORT_EPPDR_EPPD7            (0x80)
#define MCF5282_EPORT_EPPDR_EPPD6            (0x40)
#define MCF5282_EPORT_EPPDR_EPPD5            (0x20)
#define MCF5282_EPORT_EPPDR_EPPD4            (0x10)
#define MCF5282_EPORT_EPPDR_EPPD3            (0x08)
#define MCF5282_EPORT_EPPDR_EPPD2            (0x04)
#define MCF5282_EPORT_EPPDR_EPPD1            (0x02)
#define MCF5282_EPORT_EPPDR_EPPD(x)            (0x01 << x)

#define MCF5282_EPORT_EPFR_EPF7              (0x80)
#define MCF5282_EPORT_EPFR_EPF6              (0x40)
#define MCF5282_EPORT_EPFR_EPF5              (0x20)
#define MCF5282_EPORT_EPFR_EPF4              (0x10)
#define MCF5282_EPORT_EPFR_EPF3              (0x08)
#define MCF5282_EPORT_EPFR_EPF2              (0x04)
#define MCF5282_EPORT_EPFR_EPF1              (0x02)
#define MCF5282_EPORT_EPFR_EPF(x)              (0x01 << x)

/*********************************************************************
*
* Watchdog Timer Module (WTM)
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_WTM_WCR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x140000]))
#define MCF5282_WTM_WMR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x140002]))
#define MCF5282_WTM_WCNTR  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x140004]))
#define MCF5282_WTM_WSR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x140006]))

/* Bit level definitions and macros */
#define MCF5282_WTM_WCR_WAIT      (0x0008)
#define MCF5282_WTM_WCR_DOZE      (0x0004)
#define MCF5282_WTM_WCR_HALTED    (0x0002)
#define MCF5282_WTM_WCR_EN        (0x0001)

#define MCF5282_WTM_WSR_SEQ1      (0x5555)
#define MCF5282_WTM_WSR_SEQ2      (0xAAAA)

/*********************************************************************
*
* Programmable Interrupt Timer (PIT) Module
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_PIT0_PCSR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x150000]))
#define MCF5282_PIT0_PMR     (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x150002]))
#define MCF5282_PIT0_PCNTR   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x150004]))

#define MCF5282_PIT1_PCSR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x160000]))
#define MCF5282_PIT1_PMR     (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x160002]))
#define MCF5282_PIT1_PCNTR   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x160004]))

#define MCF5282_PIT2_PCSR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x170000]))
#define MCF5282_PIT2_PMR     (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x170002]))
#define MCF5282_PIT2_PCNTR   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x170004]))

#define MCF5282_PIT3_PCSR    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x180000]))
#define MCF5282_PIT3_PMR     (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x180002]))
#define MCF5282_PIT3_PCNTR   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x180004]))

/* wld - was 0x1000 - now 0x10000 */
#define MCF5282_PIT_PCSR(x)  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x150000 + (0x10000 * (x))]))
#define MCF5282_PIT_PMR(x)   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x150002 + (0x10000 * (x))]))
#define MCF5282_PIT_PCNTR(x) (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x150004 + (0x10000 * (x))]))

/* Bit level definitions and macros */
#define MCF5282_PIT_PCSR_PRE(x)      (((x) & 0x000F) << 8)
#define MCF5282_PIT_PCSR_DOZE        (0x0040)
#define MCF5282_PIT_PCSR_HALTED      (0x0020)
#define MCF5282_PIT_PCSR_OVW         (0x0010)
#define MCF5282_PIT_PCSR_PIE         (0x0008)
#define MCF5282_PIT_PCSR_PIF         (0x0004)
#define MCF5282_PIT_PCSR_RLD         (0x0002)
#define MCF5282_PIT_PCSR_EN          (0x0001)

/*********************************************************************
*
* Queued Analog to Digital Converter (QADC) Module
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_QADC_QADCMCR  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x190000]))
#define MCF5282_QADC_PORTQA   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x190006]))
#define MCF5282_QADC_PORTQB   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x190007]))
#define MCF5282_QADC_DDRQA    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x190008]))
#define MCF5282_QADC_DDRQB    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x190009]))
#define MCF5282_QADC_QACR0    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x19000A]))
#define MCF5282_QADC_QACR1    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x19000C]))
#define MCF5282_QADC_QACR2    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x19000E]))
#define MCF5282_QADC_QASR0    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x190010]))
#define MCF5282_QADC_QASR1    (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x190012]))
#define MCF5282_QADC_CCW(x)   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x190200 + ((x) * 2)]))
#define MCF5282_QADC_RJURR(x) (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x190280 + ((x) * 2)]))
#define MCF5282_QADC_LJSRR(x) (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x190300 + ((x) * 2)]))
#define MCF5282_QADC_LJURR(x) (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x190380 + ((x) * 2)]))

/* Bit level definitions and macros */
#define MCF5282_QADC_QADCMCR_QSTOP          (0x8000)
#define MCF5282_QADC_QADCMCR_QDBG           (0x4000)
#define MCF5282_QADC_QADCMCR_SUPV           (0x0080)

#define MCF5282_QADC_PORTQA_PQA4            (0x10)
#define MCF5282_QADC_PORTQA_PQA3            (0x08)
#define MCF5282_QADC_PORTQA_PQA1            (0x02)
#define MCF5282_QADC_PORTQA_PQA0            (0x01)
#define MCF5282_QADC_PORTQA_AN56            (0x10)
#define MCF5282_QADC_PORTQA_AN55            (0x08)
#define MCF5282_QADC_PORTQA_ETRIG2          (0x10)
#define MCF5282_QADC_PORTQA_ETRIG1          (0x08)
#define MCF5282_QADC_PORTQA_AN53            (0x02)
#define MCF5282_QADC_PORTQA_AN52            (0x01)
#define MCF5282_QADC_PORTQA_MA1             (0x02)
#define MCF5282_QADC_PORTQA_MA0             (0x01)

#define MCF5282_QADC_PORTQB_PQB3            (0x08)
#define MCF5282_QADC_PORTQB_PQB2            (0x04)
#define MCF5282_QADC_PORTQB_PQB1            (0x02)
#define MCF5282_QADC_PORTQB_PQB0            (0x01)
#define MCF5282_QADC_PORTQB_AN3             (0x08)
#define MCF5282_QADC_PORTQB_AN2             (0x04)
#define MCF5282_QADC_PORTQB_AN1             (0x02)
#define MCF5282_QADC_PORTQB_AN0             (0x01)
#define MCF5282_QADC_PORTQB_ANZ             (0x08)
#define MCF5282_QADC_PORTQB_ANY             (0x04)
#define MCF5282_QADC_PORTQB_ANX             (0x02)
#define MCF5282_QADC_PORTQB_ANW             (0x01)

#define MCF5282_QADC_DDRQA_DDQA4            (0x10)
#define MCF5282_QADC_DDRQA_DDQA3            (0x08)
#define MCF5282_QADC_DDRQA_DDQA1            (0x02)
#define MCF5282_QADC_DDRQA_DDQA0            (0x01)

#define MCF5282_QADC_DDRQB_DDQB3            (0x08)
#define MCF5282_QADC_DDRQB_DDQB2            (0x04)
#define MCF5282_QADC_DDRQB_DDQB1            (0x02)
#define MCF5282_QADC_DDRQB_DDQB0            (0x01)

#define MCF5282_QADC_QACR0_MUX              (0x8000)
#define MCF5282_QADC_QACR0_TRG              (0x1000)
#define MCF5282_QADC_QACR0_QPR(x)           (((x) & 0x007F))

#define MCF5282_QADC_QACRx_CIE              (0x8000)
#define MCF5282_QADC_QACRx_PIE              (0x4000)
#define MCF5282_QADC_QACRx_SSE              (0x2000)
#define MCF5282_QADC_QACRx_MQ(x)            (((x) & 0x001F) << 8)
#define MCF5282_QADC_QACRx_RESUME           (0x0080)
#define MCF5282_QADC_QACRx_BQ(x)            (((x) & 0x007F))

#define MCF5282_QADC_QASR0_CF1              (0x8000)
#define MCF5282_QADC_QASR0_PF1              (0x4000)
#define MCF5282_QADC_QASR0_CF2              (0x2000)
#define MCF5282_QADC_QASR0_PF2              (0x1000)
#define MCF5282_QADC_QASR0_TOR1             (0x0800)
#define MCF5282_QADC_QASR0_TOR2             (0x0400)

#define MCF5282_QADC_CCW_P                  (0x0200)
#define MCF5282_QADC_CCW_BYP                (0x0100)
#define MCF5282_QADC_CCW_IST(x)             (((x) & 0x0003) << 14)
#define MCF5282_QADC_CCW_CHAN(x)            (((x) & 0x003F))

/*********************************************************************
*
* General Purpose Timer (GPT) Module
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_GPTA_GPTIOS     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A0000]))
#define MCF5282_GPTA_GPTCFORC   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A0001]))
#define MCF5282_GPTA_GPTOC3M    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A0002]))
#define MCF5282_GPTA_GPTOC3D    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A0003]))
#define MCF5282_GPTA_GPTCNT     (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A0004]))
#define MCF5282_GPTA_GPTSCR1    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A0006]))
#define MCF5282_GPTA_GPTTOV     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A0008]))
#define MCF5282_GPTA_GPTCTL1    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A0009]))
#define MCF5282_GPTA_GPTCTL2    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A000B]))
#define MCF5282_GPTA_GPTIE      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A000C]))
#define MCF5282_GPTA_GPTSCR2    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A000D]))
#define MCF5282_GPTA_GPTFLG1    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A000E]))
#define MCF5282_GPTA_GPTFLG2    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A000F]))
#define MCF5282_GPTA_GPTC0      (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A0010]))
#define MCF5282_GPTA_GPTC1      (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A0012]))
#define MCF5282_GPTA_GPTC2      (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A0014]))
#define MCF5282_GPTA_GPTC3      (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A0016]))
#define MCF5282_GPTA_GPTPACTL   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A0018]))
#define MCF5282_GPTA_GPTPAFLG   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A0019]))
#define MCF5282_GPTA_GPTPACNT   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A001A]))
#define MCF5282_GPTA_GPTPORT    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A001D]))
#define MCF5282_GPTA_GPTDDR     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1A001E]))

#define MCF5282_GPTB_GPTIOS     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B0000]))
#define MCF5282_GPTB_GPTCFORC   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B0001]))
#define MCF5282_GPTB_GPTOC3M    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B0002]))
#define MCF5282_GPTB_GPTOC3D    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B0003]))
#define MCF5282_GPTB_GPTCNT     (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B0004]))
#define MCF5282_GPTB_GPTSCR1    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B0006]))
#define MCF5282_GPTB_GPTTOV     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B0008]))
#define MCF5282_GPTB_GPTCTL1    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B0009]))
#define MCF5282_GPTB_GPTCTL2    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B000B]))
#define MCF5282_GPTB_GPTIE      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B000C]))
#define MCF5282_GPTB_GPTSCR2    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B000D]))
#define MCF5282_GPTB_GPTFLG1    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B000E]))
#define MCF5282_GPTB_GPTFLG2    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B000F]))
#define MCF5282_GPTB_GPTC0      (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B0010]))
#define MCF5282_GPTB_GPTC1      (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B0012]))
#define MCF5282_GPTB_GPTC2      (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B0014]))
#define MCF5282_GPTB_GPTC3      (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B0016]))
#define MCF5282_GPTB_GPTPACTL   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B0018]))
#define MCF5282_GPTB_GPTPAFLG   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B0019]))
#define MCF5282_GPTB_GPTPACNT   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B001A]))
#define MCF5282_GPTB_GPTPORT    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B001D]))
#define MCF5282_GPTB_GPTDDR     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1B001E]))

/* Bit level definitions and macros */
#define MCF5282_GPT_GPTIOS_IOS3              (0x08)
#define MCF5282_GPT_GPTIOS_IOS2              (0x04)
#define MCF5282_GPT_GPTIOS_IOS1              (0x02)
#define MCF5282_GPT_GPTIOS_IOS0              (0x01)

#define MCF5282_GPT_GPTCFORC_FOC3            (0x08)
#define MCF5282_GPT_GPTCFORC_FOC2            (0x04)
#define MCF5282_GPT_GPTCFORC_FOC1            (0x02)
#define MCF5282_GPT_GPTCFORC_FOC0            (0x01)

#define MCF5282_GPT_GPTOC3M_OC3M3            (0x08)
#define MCF5282_GPT_GPTOC3M_OC3M2            (0x04)
#define MCF5282_GPT_GPTOC3M_OC3M1            (0x02)
#define MCF5282_GPT_GPTOC3M_OC3M0            (0x01)

#define MCF5282_GPT_GPTOC3M_OC3D(x)          (((x) & 0x04))

#define MCF5282_GPT_GPTSCR1_GPTEN            (0x80)
#define MCF5282_GPT_GPTSCR1_TFFCA            (0x10)

#define MCF5282_GPT_GPTTOV3                  (0x08)
#define MCF5282_GPT_GPTTOV2                  (0x04)
#define MCF5282_GPT_GPTTOV1                  (0x02)
#define MCF5282_GPT_GPTTOV0                  (0x01)

#define MCF5282_GPT_GPTCTL_OMOL3(x)          (((x) & 0x03) << 6)
#define MCF5282_GPT_GPTCTL_OMOL2(x)          (((x) & 0x03) << 4)
#define MCF5282_GPT_GPTCTL_OMOL1(x)          (((x) & 0x03) << 2)
#define MCF5282_GPT_GPTCTL_OMOL0(x)          (((x) & 0x03) << 0)

#define MCF5282_GPT_GPTCTL2_EDG3(x)          (((x) & 0x03) << 6)
#define MCF5282_GPT_GPTCTL2_EDG2(x)          (((x) & 0x03) << 4)
#define MCF5282_GPT_GPTCTL2_EDG1(x)          (((x) & 0x03) << 2)
#define MCF5282_GPT_GPTCTL2_EDG0(x)          (((x) & 0x03) << 0)

#define MCF5282_GPT_GPTIE_C3I                (0x08)
#define MCF5282_GPT_GPTIE_C2I                (0x04)
#define MCF5282_GPT_GPTIE_C1I                (0x02)
#define MCF5282_GPT_GPTIE_C0I                (0x01)

#define MCF5282_GPT_GPTSCR2_TOI              (0x80)
#define MCF5282_GPT_GPTSCR2_PUPT             (0x20)
#define MCF5282_GPT_GPTSCR2_RDPT             (0x10)
#define MCF5282_GPT_GPTSCR2_TCRE             (0x08)
#define MCF5282_GPT_GPTSCR2_PR(x)            (((x) & 0x07))

#define MCF5282_GPT_GPTFLG1_C3F              (0x08)
#define MCF5282_GPT_GPTFLG1_C2F              (0x04)
#define MCF5282_GPT_GPTFLG1_C1F              (0x02)
#define MCF5282_GPT_GPTFLG1_C0F              (0x01)

#define MCF5282_GPT_GPTFLG2_TOF              (0x80)
#define MCF5282_GPT_GPTFLG2_C3F              (0x08)
#define MCF5282_GPT_GPTFLG2_C2F              (0x04)
#define MCF5282_GPT_GPTFLG2_C1F              (0x02)
#define MCF5282_GPT_GPTFLG2_C0F              (0x01)

#define MCF5282_GPT_GPTPACTL_PAE             (0x40)
#define MCF5282_GPT_GPTPACTL_PAMOD           (0x20)
#define MCF5282_GPT_GPTPACTL_PEDGE           (0x10)
#define MCF5282_GPT_GPTPACTL_CLK_PACLK       (0x04)
#define MCF5282_GPT_GPTPACTL_CLK_PACLK256    (0x08)
#define MCF5282_GPT_GPTPACTL_CLK_PACLK65536  (0x0C)
#define MCF5282_GPT_GPTPACTL_CLK(x)          (((x) & 0x03) << 2)
#define MCF5282_GPT_GPTPACTL_PAOVI           (0x02)
#define MCF5282_GPT_GPTPACTL_PAI             (0x01)

#define MCF5282_GPT_GPTPAFLG_PAOVF           (0x02)
#define MCF5282_GPT_GPTPAFLG_PAIF            (0x01)

#define MCF5282_GPT_GPTPORT_PORTT3           (0x08)
#define MCF5282_GPT_GPTPORT_PORTT2           (0x04)
#define MCF5282_GPT_GPTPORT_PORTT1           (0x02)
#define MCF5282_GPT_GPTPORT_PORTT0           (0x01)

#define MCF5282_GPT_GPTDDR_DDRT3             (0x08)
#define MCF5282_GPT_GPTDDR_DDRT2             (0x04)
#define MCF5282_GPT_GPTDDR_DDRT1             (0x02)
#define MCF5282_GPT_GPTDDR_DDRT0             (0x01)

/*********************************************************************
*
* FlexCAN Module
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_FLEXCAN_CANMCR      (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0000]))
#define MCF5282_FLEXCAN_CANCTRL0    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0006]))
#define MCF5282_FLEXCAN_CANCTRL1    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0007]))
#define MCF5282_FLEXCAN_PRESDIV     (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0008]))
#define MCF5282_FLEXCAN_CANCTRL2    (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0009]))
#define MCF5282_FLEXCAN_TIMER       (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C000A]))
#define MCF5282_FLEXCAN_RXGMASK     (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0010]))
#define MCF5282_FLEXCAN_RX14MASK    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0014]))
#define MCF5282_FLEXCAN_RX15MASK    (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0018]))
#define MCF5282_FLEXCAN_ESTAT       (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0020]))
#define MCF5282_FLEXCAN_IMASK       (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0022]))
#define MCF5282_FLEXCAN_IFLAG       (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0024]))
#define MCF5282_FLEXCAN_RXECTR      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0026]))
#define MCF5282_FLEXCAN_TXECTR      (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0027]))
#define MCF5282_FLEXCAN_MBUF0_CTRL  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0080]))
#define MCF5282_FLEXCAN_MBUF0_IDH   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0082]))
#define MCF5282_FLEXCAN_MBUF0_IDL   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0084]))
#define MCF5282_FLEXCAN_MBUF0_BYTE0 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0086]))
#define MCF5282_FLEXCAN_MBUF0_BYTE1 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0087]))
#define MCF5282_FLEXCAN_MBUF0_BYTE2 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0088]))
#define MCF5282_FLEXCAN_MBUF0_BYTE3 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0089]))
#define MCF5282_FLEXCAN_MBUF0_BYTE4 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C008A]))
#define MCF5282_FLEXCAN_MBUF0_BYTE5 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C008B]))
#define MCF5282_FLEXCAN_MBUF0_BYTE6 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C008C]))
#define MCF5282_FLEXCAN_MBUF0_BYTE7 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C008D]))

#define MCF5282_FLEXCAN_MBUF1_CTRL  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0090]))
#define MCF5282_FLEXCAN_MBUF1_IDH   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0092]))
#define MCF5282_FLEXCAN_MBUF1_IDL   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0094]))
#define MCF5282_FLEXCAN_MBUF1_BYTE0 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0096]))
#define MCF5282_FLEXCAN_MBUF1_BYTE1 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0097]))
#define MCF5282_FLEXCAN_MBUF1_BYTE2 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0098]))
#define MCF5282_FLEXCAN_MBUF1_BYTE3 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0099]))
#define MCF5282_FLEXCAN_MBUF1_BYTE4 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C009A]))
#define MCF5282_FLEXCAN_MBUF1_BYTE5 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C009B]))
#define MCF5282_FLEXCAN_MBUF1_BYTE6 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C009C]))
#define MCF5282_FLEXCAN_MBUF1_BYTE7 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C009D]))

#define MCF5282_FLEXCAN_MBUF2_CTRL  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00A0]))
#define MCF5282_FLEXCAN_MBUF2_IDH   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00A2]))
#define MCF5282_FLEXCAN_MBUF2_IDL   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00A4]))
#define MCF5282_FLEXCAN_MBUF2_BYTE0 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00A6]))
#define MCF5282_FLEXCAN_MBUF2_BYTE1 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00A7]))
#define MCF5282_FLEXCAN_MBUF2_BYTE2 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00A8]))
#define MCF5282_FLEXCAN_MBUF2_BYTE3 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00A9]))
#define MCF5282_FLEXCAN_MBUF2_BYTE4 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00AA]))
#define MCF5282_FLEXCAN_MBUF2_BYTE5 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00AB]))
#define MCF5282_FLEXCAN_MBUF2_BYTE6 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00AC]))
#define MCF5282_FLEXCAN_MBUF2_BYTE7 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00AD]))

#define MCF5282_FLEXCAN_MBUF3_CTRL  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00B0]))
#define MCF5282_FLEXCAN_MBUF3_IDH   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00B2]))
#define MCF5282_FLEXCAN_MBUF3_IDL   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00B4]))
#define MCF5282_FLEXCAN_MBUF3_BYTE0 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00B6]))
#define MCF5282_FLEXCAN_MBUF3_BYTE1 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00B7]))
#define MCF5282_FLEXCAN_MBUF3_BYTE2 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00B8]))
#define MCF5282_FLEXCAN_MBUF3_BYTE3 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00B9]))
#define MCF5282_FLEXCAN_MBUF3_BYTE4 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00BA]))
#define MCF5282_FLEXCAN_MBUF3_BYTE5 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00BB]))
#define MCF5282_FLEXCAN_MBUF3_BYTE6 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00BC]))
#define MCF5282_FLEXCAN_MBUF3_BYTE7 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00BD]))

#define MCF5282_FLEXCAN_MBUF4_CTRL  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00C0]))
#define MCF5282_FLEXCAN_MBUF4_IDH   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00C2]))
#define MCF5282_FLEXCAN_MBUF4_IDL   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00C4]))
#define MCF5282_FLEXCAN_MBUF4_BYTE0 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00C6]))
#define MCF5282_FLEXCAN_MBUF4_BYTE1 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00C7]))
#define MCF5282_FLEXCAN_MBUF4_BYTE2 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00C8]))
#define MCF5282_FLEXCAN_MBUF4_BYTE3 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00C9]))
#define MCF5282_FLEXCAN_MBUF4_BYTE4 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00CA]))
#define MCF5282_FLEXCAN_MBUF4_BYTE5 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00CB]))
#define MCF5282_FLEXCAN_MBUF4_BYTE6 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00CC]))
#define MCF5282_FLEXCAN_MBUF4_BYTE7 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00CD]))

#define MCF5282_FLEXCAN_MBUF5_CTRL  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00D0]))
#define MCF5282_FLEXCAN_MBUF5_IDH   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00D2]))
#define MCF5282_FLEXCAN_MBUF5_IDL   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00D4]))
#define MCF5282_FLEXCAN_MBUF5_BYTE0 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00D6]))
#define MCF5282_FLEXCAN_MBUF5_BYTE1 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00D7]))
#define MCF5282_FLEXCAN_MBUF5_BYTE2 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00D8]))
#define MCF5282_FLEXCAN_MBUF5_BYTE3 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00D9]))
#define MCF5282_FLEXCAN_MBUF5_BYTE4 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00DA]))
#define MCF5282_FLEXCAN_MBUF5_BYTE5 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00DB]))
#define MCF5282_FLEXCAN_MBUF5_BYTE6 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00DC]))
#define MCF5282_FLEXCAN_MBUF5_BYTE7 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00DD]))

#define MCF5282_FLEXCAN_MBUF6_CTRL  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00E0]))
#define MCF5282_FLEXCAN_MBUF6_IDH   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00E2]))
#define MCF5282_FLEXCAN_MBUF6_IDL   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00E4]))
#define MCF5282_FLEXCAN_MBUF6_BYTE0 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00E6]))
#define MCF5282_FLEXCAN_MBUF6_BYTE1 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00E7]))
#define MCF5282_FLEXCAN_MBUF6_BYTE2 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00E8]))
#define MCF5282_FLEXCAN_MBUF6_BYTE3 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00E9]))
#define MCF5282_FLEXCAN_MBUF6_BYTE4 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00EA]))
#define MCF5282_FLEXCAN_MBUF6_BYTE5 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00EB]))
#define MCF5282_FLEXCAN_MBUF6_BYTE6 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00EC]))
#define MCF5282_FLEXCAN_MBUF6_BYTE7 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00ED]))

#define MCF5282_FLEXCAN_MBUF7_CTRL  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00F0]))
#define MCF5282_FLEXCAN_MBUF7_IDH   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00F2]))
#define MCF5282_FLEXCAN_MBUF7_IDL   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00F4]))
#define MCF5282_FLEXCAN_MBUF7_BYTE0 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00F6]))
#define MCF5282_FLEXCAN_MBUF7_BYTE1 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00F7]))
#define MCF5282_FLEXCAN_MBUF7_BYTE2 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00F8]))
#define MCF5282_FLEXCAN_MBUF7_BYTE3 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00F9]))
#define MCF5282_FLEXCAN_MBUF7_BYTE4 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00FA]))
#define MCF5282_FLEXCAN_MBUF7_BYTE5 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00FB]))
#define MCF5282_FLEXCAN_MBUF7_BYTE6 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00FC]))
#define MCF5282_FLEXCAN_MBUF7_BYTE7 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00FD]))

#define MCF5282_FLEXCAN_MBUF8_CTRL  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0100]))
#define MCF5282_FLEXCAN_MBUF8_IDH   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0102]))
#define MCF5282_FLEXCAN_MBUF8_IDL   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0104]))
#define MCF5282_FLEXCAN_MBUF8_BYTE0 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0106]))
#define MCF5282_FLEXCAN_MBUF8_BYTE1 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0107]))
#define MCF5282_FLEXCAN_MBUF8_BYTE2 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0108]))
#define MCF5282_FLEXCAN_MBUF8_BYTE3 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0109]))
#define MCF5282_FLEXCAN_MBUF8_BYTE4 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C010A]))
#define MCF5282_FLEXCAN_MBUF8_BYTE5 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C010B]))
#define MCF5282_FLEXCAN_MBUF8_BYTE6 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C010C]))
#define MCF5282_FLEXCAN_MBUF8_BYTE7 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C010D]))

#define MCF5282_FLEXCAN_MBUF9_CTRL  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0110]))
#define MCF5282_FLEXCAN_MBUF9_IDH   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0112]))
#define MCF5282_FLEXCAN_MBUF9_IDL   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0114]))
#define MCF5282_FLEXCAN_MBUF9_BYTE0 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0116]))
#define MCF5282_FLEXCAN_MBUF9_BYTE1 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0117]))
#define MCF5282_FLEXCAN_MBUF9_BYTE2 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0118]))
#define MCF5282_FLEXCAN_MBUF9_BYTE3 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0119]))
#define MCF5282_FLEXCAN_MBUF9_BYTE4 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C011A]))
#define MCF5282_FLEXCAN_MBUF9_BYTE5 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C011B]))
#define MCF5282_FLEXCAN_MBUF9_BYTE6 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C011C]))
#define MCF5282_FLEXCAN_MBUF9_BYTE7 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C011D]))

#define MCF5282_FLEXCAN_MBUF10_CTRL (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0120]))
#define MCF5282_FLEXCAN_MBUF10_IDH  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0122]))
#define MCF5282_FLEXCAN_MBUF10_IDL  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0124]))
#define MCF5282_FLEXCAN_MBUF10_BYTE0 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0126]))
#define MCF5282_FLEXCAN_MBUF10_BYTE1 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0127]))
#define MCF5282_FLEXCAN_MBUF10_BYTE2 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0128]))
#define MCF5282_FLEXCAN_MBUF10_BYTE3 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0129]))
#define MCF5282_FLEXCAN_MBUF10_BYTE4 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C012A]))
#define MCF5282_FLEXCAN_MBUF10_BYTE5 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C012B]))
#define MCF5282_FLEXCAN_MBUF10_BYTE6 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C012C]))
#define MCF5282_FLEXCAN_MBUF10_BYTE7 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C012D]))

#define MCF5282_FLEXCAN_MBUF11_CTRL  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0130]))
#define MCF5282_FLEXCAN_MBUF11_IDH   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0132]))
#define MCF5282_FLEXCAN_MBUF11_IDL   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0134]))
#define MCF5282_FLEXCAN_MBUF11_BYTE0 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0136]))
#define MCF5282_FLEXCAN_MBUF11_BYTE1 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0137]))
#define MCF5282_FLEXCAN_MBUF11_BYTE2 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0138]))
#define MCF5282_FLEXCAN_MBUF11_BYTE3 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0139]))
#define MCF5282_FLEXCAN_MBUF11_BYTE4 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C013A]))
#define MCF5282_FLEXCAN_MBUF11_BYTE5 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C013B]))
#define MCF5282_FLEXCAN_MBUF11_BYTE6 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C013C]))
#define MCF5282_FLEXCAN_MBUF11_BYTE7 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C013D]))

#define MCF5282_FLEXCAN_MBUF12_CTRL  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0140]))
#define MCF5282_FLEXCAN_MBUF12_IDH   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0142]))
#define MCF5282_FLEXCAN_MBUF12_IDL   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0144]))
#define MCF5282_FLEXCAN_MBUF12_BYTE0 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0146]))
#define MCF5282_FLEXCAN_MBUF12_BYTE1 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0147]))
#define MCF5282_FLEXCAN_MBUF12_BYTE2 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0148]))
#define MCF5282_FLEXCAN_MBUF12_BYTE3 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0149]))
#define MCF5282_FLEXCAN_MBUF12_BYTE4 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C014A]))
#define MCF5282_FLEXCAN_MBUF12_BYTE5 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C014B]))
#define MCF5282_FLEXCAN_MBUF12_BYTE6 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C014C]))
#define MCF5282_FLEXCAN_MBUF12_BYTE7 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C014D]))

#define MCF5282_FLEXCAN_MBUF13_CTRL  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0150]))
#define MCF5282_FLEXCAN_MBUF13_IDH   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0152]))
#define MCF5282_FLEXCAN_MBUF13_IDL   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0154]))
#define MCF5282_FLEXCAN_MBUF13_BYTE0 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0156]))
#define MCF5282_FLEXCAN_MBUF13_BYTE1 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0157]))
#define MCF5282_FLEXCAN_MBUF13_BYTE2 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0158]))
#define MCF5282_FLEXCAN_MBUF13_BYTE3 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0159]))
#define MCF5282_FLEXCAN_MBUF13_BYTE4 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C015A]))
#define MCF5282_FLEXCAN_MBUF13_BYTE5 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C015B]))
#define MCF5282_FLEXCAN_MBUF13_BYTE6 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C015C]))
#define MCF5282_FLEXCAN_MBUF13_BYTE7 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C015D]))

#define MCF5282_FLEXCAN_MBUF14_CTRL  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0160]))
#define MCF5282_FLEXCAN_MBUF14_IDH   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0162]))
#define MCF5282_FLEXCAN_MBUF14_IDL   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0164]))
#define MCF5282_FLEXCAN_MBUF14_BYTE0 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0166]))
#define MCF5282_FLEXCAN_MBUF14_BYTE1 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0167]))
#define MCF5282_FLEXCAN_MBUF14_BYTE2 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0168]))
#define MCF5282_FLEXCAN_MBUF14_BYTE3 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0169]))
#define MCF5282_FLEXCAN_MBUF14_BYTE4 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C016A]))
#define MCF5282_FLEXCAN_MBUF14_BYTE5 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C016B]))
#define MCF5282_FLEXCAN_MBUF14_BYTE6 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C016C]))
#define MCF5282_FLEXCAN_MBUF14_BYTE7 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C016D]))

#define MCF5282_FLEXCAN_MBUF15_CTRL  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0170]))
#define MCF5282_FLEXCAN_MBUF15_IDH   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0172]))
#define MCF5282_FLEXCAN_MBUF15_IDL   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0174]))
#define MCF5282_FLEXCAN_MBUF15_BYTE0 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0176]))
#define MCF5282_FLEXCAN_MBUF15_BYTE1 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0177]))
#define MCF5282_FLEXCAN_MBUF15_BYTE2 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0178]))
#define MCF5282_FLEXCAN_MBUF15_BYTE3 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0179]))
#define MCF5282_FLEXCAN_MBUF15_BYTE4 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C017A]))
#define MCF5282_FLEXCAN_MBUF15_BYTE5 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C017B]))
#define MCF5282_FLEXCAN_MBUF15_BYTE6 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C017C]))
#define MCF5282_FLEXCAN_MBUF15_BYTE7 (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C017D]))

#define MCF5282_FLEXCAN_MBUF0_BYTE(x)  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0086 + (x)]))
#define MCF5282_FLEXCAN_MBUF1_BYTE(x)  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0096 + (x)]))
#define MCF5282_FLEXCAN_MBUF2_BYTE(x)  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00A6 + (x)]))
#define MCF5282_FLEXCAN_MBUF3_BYTE(x)  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00B6 + (x)]))
#define MCF5282_FLEXCAN_MBUF4_BYTE(x)  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00C6 + (x)]))
#define MCF5282_FLEXCAN_MBUF5_BYTE(x)  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00D6 + (x)]))
#define MCF5282_FLEXCAN_MBUF6_BYTE(x)  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00E6 + (x)]))
#define MCF5282_FLEXCAN_MBUF7_BYTE(x)  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C00F6 + (x)]))
#define MCF5282_FLEXCAN_MBUF8_BYTE(x)  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0106 + (x)]))
#define MCF5282_FLEXCAN_MBUF9_BYTE(x)  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0116 + (x)]))
#define MCF5282_FLEXCAN_MBUF10_BYTE(x) (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0126 + (x)]))
#define MCF5282_FLEXCAN_MBUF11_BYTE(x) (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0136 + (x)]))
#define MCF5282_FLEXCAN_MBUF12_BYTE(x) (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0146 + (x)]))
#define MCF5282_FLEXCAN_MBUF13_BYTE(x) (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0156 + (x)]))
#define MCF5282_FLEXCAN_MBUF14_BYTE(x) (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0166 + (x)]))
#define MCF5282_FLEXCAN_MBUF15_BYTE(x) (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0176 + (x)]))

#define MCF5282_FLEXCAN_MBUF_BYTE(x, y) \
                        (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1C0086 + (0x10 * (x)) + (y)]))

/* Bit level definitions and macros */
#define MCF5282_FLEXCAN_CANMCR_STOP            (0x8000)
#define MCF5282_FLEXCAN_CANMCR_FRZ             (0x4000)
#define MCF5282_FLEXCAN_CANMCR_HALT            (0x1000)
#define MCF5282_FLEXCAN_CANMCR_NOTRDY          (0x0800)
#define MCF5282_FLEXCAN_CANMCR_WAKEMSK         (0x0400)
#define MCF5282_FLEXCAN_CANMCR_SOFTRST         (0x0200)
#define MCF5282_FLEXCAN_CANMCR_FRZACK          (0x0100)
#define MCF5282_FLEXCAN_CANMCR_SUPV            (0x0080)
#define MCF5282_FLEXCAN_CANMCR_SELFWAKE        (0x0040)
#define MCF5282_FLEXCAN_CANMCR_APS             (0x0020)

#define MCF5282_FLEXCAN_CANCTRL0_BOFFMSK          (0x80)
#define MCF5282_FLEXCAN_CANCTRL0_ERRMSK           (0x40)
#define MCF5282_FLEXCAN_CANCTRL0_RXMODE           (0x04)
#define MCF5282_FLEXCAN_CANCTRL0_TXMODE_CMOSPOS   (0x00)
#define MCF5282_FLEXCAN_CANCTRL0_TXMODE_CMOSNEG   (0x01)
#define MCF5282_FLEXCAN_CANCTRL0_TXMODE_OPENDRAIN (0x02)

#define MCF5282_FLEXCAN_CANCTRL1_SAMP             (0x80)
#define MCF5282_FLEXCAN_CANCTRL1_TSYNC            (0x20)
#define MCF5282_FLEXCAN_CANCTRL1_LBUF             (0x10)
#define MCF5282_FLEXCAN_CANCTRL1_LOM              (0x08)
#define MCF5282_FLEXCAN_CANCTRL1_PROPSEG(x)       (((x) & 0x07))

#define MCF5282_FLEXCAN_CANCTRL2_RJW(x)       (((x) & 0x03) << 6)
#define MCF5282_FLEXCAN_CANCTRL2_PSEG1(x)     (((x) & 0x07) << 3)
#define MCF5282_FLEXCAN_CANCTRL2_PSEG2(x)     (((x) & 0x07) << 0)

#define MCF5282_FLEXCAN_ESTAT_BITERR(x)       (((x) & 0x03) << 14)
#define MCF5282_FLEXCAN_ESTAT_ACKERR          (0x2000)
#define MCF5282_FLEXCAN_ESTAT_CRCERR          (0x1000)
#define MCF5282_FLEXCAN_ESTAT_FORMERR         (0x0800)
#define MCF5282_FLEXCAN_ESTAT_STUFFERR        (0x0400)
#define MCF5282_FLEXCAN_ESTAT_TXWARN          (0x0200)
#define MCF5282_FLEXCAN_ESTAT_RXWARN          (0x0100)
#define MCF5282_FLEXCAN_ESTAT_IDLE            (0x0080)
#define MCF5282_FLEXCAN_ESTAT_TXRX            (0x0040)
#define MCF5282_FLEXCAN_ESTAT_FCS(x)          (((x) & 0x03) << 4)
#define MCF5282_FLEXCAN_ESTAT_BOFFINT         (0x0004)
#define MCF5282_FLEXCAN_ESTAT_ERRINT          (0x0002)
#define MCF5282_FLEXCAN_ESTAT_WAKEINT         (0x0001)

#define MCF5282_FLEXCAN_IMASK_BUF15M        (0x8000)
#define MCF5282_FLEXCAN_IMASK_BUF14M        (0x4000)
#define MCF5282_FLEXCAN_IMASK_BUF13M        (0x2000)
#define MCF5282_FLEXCAN_IMASK_BUF12M        (0x1000)
#define MCF5282_FLEXCAN_IMASK_BUF11M        (0x0800)
#define MCF5282_FLEXCAN_IMASK_BUF10M        (0x0400)
#define MCF5282_FLEXCAN_IMASK_BUF9M         (0x0200)
#define MCF5282_FLEXCAN_IMASK_BUF8M         (0x0100)
#define MCF5282_FLEXCAN_IMASK_BUF7M         (0x0080)
#define MCF5282_FLEXCAN_IMASK_BUF6M         (0x0040)
#define MCF5282_FLEXCAN_IMASK_BUF5M         (0x0020)
#define MCF5282_FLEXCAN_IMASK_BUF4M         (0x0010)
#define MCF5282_FLEXCAN_IMASK_BUF3M         (0x0008)
#define MCF5282_FLEXCAN_IMASK_BUF2M         (0x0004)
#define MCF5282_FLEXCAN_IMASK_BUF1M         (0x0002)
#define MCF5282_FLEXCAN_IMASK_BUF0M         (0x0001)

#define MCF5282_FLEXCAN_IFLAG_BUF15I        (0x8000)
#define MCF5282_FLEXCAN_IFLAG_BUF14I        (0x4000)
#define MCF5282_FLEXCAN_IFLAG_BUF13I        (0x2000)
#define MCF5282_FLEXCAN_IFLAG_BUF12I        (0x1000)
#define MCF5282_FLEXCAN_IFLAG_BUF11I        (0x0800)
#define MCF5282_FLEXCAN_IFLAG_BUF10I        (0x0400)
#define MCF5282_FLEXCAN_IFLAG_BUF9I         (0x0200)
#define MCF5282_FLEXCAN_IFLAG_BUF8I         (0x0100)
#define MCF5282_FLEXCAN_IFLAG_BUF7I         (0x0080)
#define MCF5282_FLEXCAN_IFLAG_BUF6I         (0x0040)
#define MCF5282_FLEXCAN_IFLAG_BUF5I         (0x0020)
#define MCF5282_FLEXCAN_IFLAG_BUF4I         (0x0010)
#define MCF5282_FLEXCAN_IFLAG_BUF3I         (0x0008)
#define MCF5282_FLEXCAN_IFLAG_BUF2I         (0x0004)
#define MCF5282_FLEXCAN_IFLAG_BUF1I         (0x0002)
#define MCF5282_FLEXCAN_IFLAG_BUF0I         (0x0001)

/*********************************************************************
*
* ColdFire Flash Module (CFM)
*
*********************************************************************/

/* Read/Write access macros for general use */
#define MCF5282_CFM_CFMMCR   (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1D0000]))
#define MCF5282_CFM_CFMCLKD  (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1D0002]))
#define MCF5282_CFM_CFMSEC   (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1D0008]))
#define MCF5282_CFM_CFMPROT  (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1D0010]))
#define MCF5282_CFM_CFMSACC  (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1D0014]))
#define MCF5282_CFM_CFMDACC  (*(tt32BitPtr)(ttVoidPtr)(&__IPSBAR[0x1D0018]))
#define MCF5282_CFM_CFMUSTAT (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1D0020]))
#define MCF5282_CFM_CFMCMD   (*(tt8BitPtr)(ttVoidPtr)(&__IPSBAR[0x1D0024]))
#define MCF5282_CFM_CFMDISU  (*(tt16BitPtr)(ttVoidPtr)(&__IPSBAR[0x1D0042]))

/* Bit level definitions and macros */
#define MCF5282_CFM_FLASHBAR_BA(a)             ((a) & 0xFFF8000)
#define MCF5282_CFM_FLASHBAR_WP                (0x00000100)
#define MCF5282_CFM_FLASHBAR_CI                (0x00000020)
#define MCF5282_CFM_FLASHBAR_SC                (0x00000010)
#define MCF5282_CFM_FLASHBAR_SD                (0x00000008)
#define MCF5282_CFM_FLASHBAR_UC                (0x00000004)
#define MCF5282_CFM_FLASHBAR_UD                (0x00000002)
#define MCF5282_CFM_FLASHBAR_V                 (0x00000001)

#define MCF5282_CFM_CFMMCR_LOCK                (0x0400)
#define MCF5282_CFM_CFMMCR_PVIE                (0x0200)
#define MCF5282_CFM_CFMMCR_AEIE                (0x0100)
#define MCF5282_CFM_CFMMCR_CBEIE               (0x0080)
#define MCF5282_CFM_CFMMCR_CCIE                (0x0040)
#define MCF5282_CFM_CFMMCR_KEYACC              (0x0020)

#define MCF5282_CFM_CFMCLKD_DIVLD              (0x80)
#define MCF5282_CFM_CFMCLKD_PRDIV8             (0x40)
#define MCF5282_CFM_CFMCLKD_DIV(x)             (((x) & 0x3F))

#define MCF5282_CFM_CFMSEC_KEYEN               (0x80000000)
#define MCF5282_CFM_CFMSEC_SECSTAT             (0x40000000)
#define MCF5282_CFM_CFMSEC_SEC(x)              (((x) & 0xFFFF))

#define MCF5282_CFM_CFMUSTAT_CBEIF             (0x80)
#define MCF5282_CFM_CFMUSTAT_CCIF              (0x40)
#define MCF5282_CFM_CFMUSTAT_PVIOL             (0x20)
#define MCF5282_CFM_CFMUSTAT_ACCERR            (0x10)
#define MCF5282_CFM_CFMUSTAT_BLANK             (0x04)

#define MCF5282_CFM_CFMCMD_CMD(x)              (((x) & 0x7F))

#define MCF5282_CFM_CFMCMD_CMD_ERASE_VERIFY    (0x05)
#define MCF5282_CFM_CFMCMD_CMD_PROGRAM_32BIT   (0x20)
#define MCF5282_CFM_CFMCMD_CMD_PAGE_ERASE      (0x40)
#define MCF5282_CFM_CFMCMD_CMD_MASS_ERASE      (0x41)

#ifdef __cplusplus
}
#endif

#endif /* } _MCF5282_H */

/* end of file - trmcffec.h */
