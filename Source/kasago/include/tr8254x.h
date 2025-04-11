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
 * Description: Intel 8254X Device Driver
 * Filename: tr8254x.h
 * Author: jshang
 * Date Created: 1/14/2003
 * $Source: include/tr8254x.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:14JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */


/*
 * This following macros might need to be modified for your particular 
 * platform .
 */


/* Size of the 8254x transmit ring (number of packets) */
#define TM_8254X_CMD_RING_SIZE      32

/* Size of the 8254x receive ring (number of packets) */
#define TM_8254X_RECV_RING_SIZE     64

#define TM_I82544_MAX_PACKET_SIZE   16288


/* following definition is for directly read/write memory */

typedef unsigned char       CYG_BYTE;
typedef unsigned short      CYG_WORD16;
typedef ttUser32Bit         CYG_WORD32;
typedef unsigned long long  CYG_WORD64;

#define tm_readmem8(   _reg_, _val_ ) \
            ((CYG_BYTE)(_val_) = *((volatile CYG_BYTE *)(_reg_)))
#define tm_writemem8(  _reg_, _val_ ) \
            (*((volatile CYG_BYTE *)(_reg_)) = (CYG_BYTE)(_val_))
#define tm_readmem16(  _reg_, _val_ ) \
            ((CYG_WORD16)(_val_) = *((volatile CYG_WORD16 *)(_reg_)))
#define tm_writemem16( _reg_, _val_ ) \
            (*((volatile CYG_WORD16 *)(_reg_)) = (CYG_WORD16)(_val_))
#define tm_readmem32(  _reg_, _val_ ) \
            ((CYG_WORD32)(_val_) = *((volatile CYG_WORD32 *)(_reg_)))
#define tm_writemem32( _reg_, _val_ ) \
            (*((volatile CYG_WORD32 *)(_reg_)) = (CYG_WORD32)(_val_))
#define tm_readmem64(  _reg_, _val_ ) \
            ((CYG_WORD64)(_val_) = *((volatile CYG_WORD64 *)(_reg_)))
#define tm_writemem64( _reg_, _val_ ) \
            (*((volatile CYG_WORD64 *)(_reg_)) = (CYG_WORD64)(_val_))

#define tm_outl( _v_, _a_ ) tm_writemem32( _a_, _v_ )

static inline CYG_WORD32 tm_inl(CYG_WORD32 io_address)
 {   CYG_WORD32 _t_; tm_readmem32( io_address, _t_ ); return _t_;   }


#define TM_I82544_EECD     0x00010


/* ------------------------------------------------------------------------*/
/* Map from CPU-view addresses to PCI-bus master's view - however that is:*/


/* ------------------------------------------------------------------------ */                                                                     
/*                   82544 REGISTER OFFSETS                                 */
/* ------------------------------------------------------------------------ */

/* General registers */
#define TM_I82544_CTRL     0x00000
#define TM_I82544_STATUS   0x00008
#define TM_I82544_EECD     0x00010
#define TM_I82544_CTRL_EXT 0x00018
#define TM_I82544_MDIC     0x00020
#define TM_I82544_FCAL     0x00028
#define TM_I82544_FCAH     0x0002c
#define TM_I82544_FCT      0x00030
#define TM_I82544_VET      0x00038
#define TM_I82544_FCTTV    0x00170
#define TM_I82544_TXCW     0x00178
#define TM_I82544_RXCW     0x00180
#define TM_I82544_PBA      0x01000

/* Interrupt control registers */
#define TM_I82544_ICR      0x000c0
#define TM_I82544_ICS      0x000c8
#define TM_I82544_IMS      0x000d0
#define TM_I82544_IMC      0x000d8


/* ------------------------------------------------------------------------*/
/*               82544 DEVICE CONTROL WORD DEFNITIONS                      */
/* ------------------------------------------------------------------------*/

#define TM_I82544_CTRL_FD          (1<<0)
#define TM_I82544_CTRL_BEM         (1<<1)
#define TM_I82544_CTRL_LRST        (1<<3)
#define TM_I82544_CTRL_ASDE        (1<<5)
#define TM_I82544_CTRL_SLU         (1<<6)
#define TM_I82544_CTRL_ILOS        (1<<7)
#define TM_I82544_CTRL_SPEED       (3<<8)
#define TM_I82544_CTRL_FRCSPD      (1<<11)
#define TM_I82544_CTRL_FRCDPLX     (1<<12)
#define TM_I82544_CTRL_SWDPINSLO   (15<<18)
#define TM_I82544_CTRL_SWDPINSIO   (15<<22)
#define TM_I82544_CTRL_RST         (1<<26)
#define TM_I82544_CTRL_RFCE        (1<<27)
#define TM_I82544_CTRL_TFCE        (1<<28)
#define TM_I82544_CTRL_VME         (1<<30)
#define TM_I82544_CTRL_PHY_RST     (1<<31)

#define TM_I82544_CTRL_PHY_RESET           (1<<18)
#define TM_I82544_CTRL_PHY_RESET_DIR       (1<<22)
#define TM_I82544_CTRL_MDIO                (1<<20)
#define TM_I82544_CTRL_MDIO_DIR            (1<<24)
#define TM_I82544_CTRL_MDC                 (1<<21)
#define TM_I82544_CTRL_MDC_DIR             (1<<25)

#define TM_I82544_CTRL_EXT_PHY_RESET4      (1<<4)
#define TM_I82544_CTRL_EXT_PHY_RESET_DIR4  (1<<8)

#define TM_PHY_ADDRESS 1


/* Receive registers */
#define TM_I82544_RCTL     0x00100
#define TM_I82544_FCRTL    0x02160
#define TM_I82544_FCRTH    0x02168
#define TM_I82544_RDBAL    0x02800
#define TM_I82544_RDBAH    0x02804
#define TM_I82544_RDLEN    0x02808
#define TM_I82544_RDH      0x02810
#define TM_I82544_RDT      0x02818
#define TM_I82544_RDTR     0x02820
#define TM_I82544_RXDCTL   0x02828
#define TM_I82544_RXCSUM   0x05000
#define TM_I82544_MTA      0x05200
#define TM_I82544_RAT      0x05400
#define TM_I82544_VFTA     0x05600

#define TM_I82544_RCTL_EN  (1<<1)
#define TM_I82544_RCTL_SBP (1<<2)
#define TM_I82544_RCTL_UPE (1<<3)
#define TM_I82544_RCTL_MPE (1<<4)
#define TM_I82544_RCTL_BAM (1<<15)
#define TM_I82544_RCTL_SECRC (1<<26)

/* Transmit registers */
#define TM_I82544_TCTL     0x00400
#define TM_I82544_TIPG     0x00410
#define TM_I82544_TBT      0x00448
#define TM_I82544_AIT      0x00458
#define TM_I82544_TXDMAC   0x03000
#define TM_I82544_TDBAL    0x03800
#define TM_I82544_TDBAH    0x03804
#define TM_I82544_TDLEN    0x03808
#define TM_I82544_TDH      0x03810
#define TM_I82544_TDT      0x03818
#define TM_I82544_TIDV     0x03820
#define TM_I82544_TXDCTL   0x03828
#define TM_I82544_TSPMT    0x03830


#define TM_I82544_TCTL_EN  (1<<1)
#define TM_I82544_TCTL_PSP (1<<3)


/* ------------------------------------------------------------------------ */
/*               82544 DEVICE STATUS WORD DEFNITIONS                        */
/* ------------------------------------------------------------------------ */

#define TM_I82544_STATUS_FD        0x0001
#define TM_I82544_STATUS_LU        0x0002
#define TM_I82544_STATUS_TXOFF     0x0010
#define TM_I82544_STATUS_TBIMODE   0x0020
#define TM_I82544_STATUS_SPEED     0x00C0
#define TM_I82544_STATUS_ASDV      0x0300
#define TM_I82544_STATUS_PCI_SPD   0x0800
#define TM_I82544_STATUS_BUS64     0x1000
#define TM_I82544_STATUS_PCIX_MODE 0x2000
#define TM_I82544_STATUS_PCIXSPD   0xC000


/* ------------------------------------------------------------------------*/
/*               RECEIVE DESCRIPTORS                                       */
/* ------------------------------------------------------------------------*/

#define TM_I82544_RD_BUFFER        0
#define TM_I82544_RD_LENGTH        8
#define TM_I82544_RD_PKTXSUM       10
#define TM_I82544_RD_STATUS        12
#define TM_I82544_RD_ERRORS        13
#define TM_I82544_RD_SPECIAL       14
#define TM_I82544_RD_SIZE          16

#define TM_I82544_RD_STATUS_DD     (1<<0)
#define TM_I82544_RD_STATUS_EOP    (1<<1)
#define TM_I82544_RD_STATUS_IXSM   (1<<2)
#define TM_I82544_RD_STATUS_VP     (1<<3)
#define TM_I82544_RD_STATUS_TCPCS  (1<<5)
#define TM_I82544_RD_STATUS_IPCS   (1<<6)
#define TM_I82544_RD_STATUS_PIF    (1<<7)

#define TM_I82544_RD_ERRORS_RXE    (1<<7)
#define TM_I82544_RD_ERRORS_IPE    (1<<6)
#define TM_I82544_RD_ERRORS_TCPE   (1<<5)
#define TM_I82544_RD_ERRORS_CXE    (1<<4)
#define TM_I82544_RD_ERRORS_RSV    (1<<3)
#define TM_I82544_RD_ERRORS_SEQ    (1<<2)
#define TM_I82544_RD_ERRORS_SE     (1<<1)
#define TM_I82544_RD_ERRORS_CE     (1<<0)

/* ------------------------------------------------------------------------ */
/*               TRANSMIT DESCRIPTORS                                       */
/* ------------------------------------------------------------------------ */

/* Currently we only use the legacy Tx descriptor */

#define TM_I82544_TD_BUFFER        0
#define TM_I82544_TD_LENGTH        8
#define TM_I82544_TD_CSO           10
#define TM_I82544_TD_CMD           11
#define TM_I82544_TD_STATUS        12
#define TM_I82544_TD_CSS           13
#define TM_I82544_TD_SPECIAL       14

#define TM_I82544_TD_CMD_EOP       (1<<0)
#define TM_I82544_TD_CMD_IFCS      (1<<1)
#define TM_I82544_TD_CMD_IC        (1<<2)
#define TM_I82544_TD_CMD_RS        (1<<3)
#define TM_I82544_TD_CMD_RPS       (1<<4)
#define TM_I82544_TD_CMD_DEXT      (1<<5)
#define TM_I82544_TD_CMD_VLE       (1<<6)
#define TM_I82544_TD_CMD_IDE       (1<<7)

#define TM_I82544_TD_STATUS_DD     (1<<0)
#define TM_I82544_TD_STATUS_EC     (1<<1)
#define TM_I82544_TD_STATUS_LC     (1<<2)
#define TM_I82544_TD_STATUS_TU     (1<<3)

/* following definition are for TCP segmentation and checksum */

#define TM_I82544_RXCSUM_IPOFL     (1<<8)
#define TM_I82544_RXCSUM_TUOFL     (1<<9)
/* 82546 support ipv6 */
#define TM_I82544_RXCSUM_IPV6OFL   (1<<10)

/* TCP/UDP context transmit descriptor layout type = 0000 */
#define TM_I82544_TD_CON_IPCSS      0
#define TM_I82544_TD_CON_IPCSO      1
#define TM_I82544_TD_CON_IPCSE      2
#define TM_I82544_TD_CON_TUCSS      4
#define TM_I82544_TD_CON_TUCSO      5
#define TM_I82544_TD_CON_TUCSE      6
#define TM_I82544_TD_CON_PAYLEN     8
#define TM_I82544_TD_CON_DTYP       10 /*20-23 */
#define TM_I82544_TD_CON_TUCMD      11
#define TM_I82544_TD_CON_STA        12
#define TM_I82544_TD_CON_HDRLEN     13
#define TM_I82544_TD_CON_MSS        14

#define TM_I82544_TD_CON_TUCMD_TCP  (1<<0)
#define TM_I82544_TD_CON_TUCMD_IP   (1<<1)
#define TM_I82544_TD_CON_TUCMD_TSE  (1<<2)
#define TM_I82544_TD_CON_TUCMD_RS   (1<<3)
#define TM_I82544_TD_CON_TUCMD_RSV  (1<<4)
#define TM_I82544_TD_CON_TUCMD_DEXT (1<<5)
#define TM_I82544_TD_CON_TUCMD_RSV2 (1<<6)
#define TM_I82544_TD_CON_TUCMD_IDE  (1<<7)

/* TCP/UDP data transmit descripor, type = 0001 */
#define TM_I82544_TD_DAT_ADDRESS    0
#define TM_I82544_TD_DAT_DTALEN     8
#define TM_I82544_TD_DAT_DTYP       10 /* 20 - 23*/
#define TM_I82544_TD_DAT_DCMD       11
#define TM_I82544_TD_DAT_STA        12
#define TM_I82544_TD_DAT_POPTS      13
#define TM_I82544_TD_DAT_SPECIAL    14

#define TM_I82544_TD_DAT_DCMD_TSE   (1<<2)
#define TM_I82544_TD_DAT_POPTS_IXSM (1<<0)
#define TM_I82544_TD_DAT_POPTS_TXSM (1<<1)


/* ------------------------------------------------------------------------*/
/*                      DEVICES AND PACKET QUEUES                          */
/* ------------------------------------------------------------------------*/
/* get the number 1518 form ecos code */
#define TM_MAX_RX_PACKET_SIZE  1518        // maximum Rx packet size
#define TM_MAX_TX_PACKET_SIZE  1518        // maximum Tx packet size
/* descriptor size */
#define TM_I82544_DESC_SIZE         16

#define CYG_PRI_CFG_BAR_MEM_MASK                  0xfffffff0l
#define TM_VIRT_TO_BUS(_x_) \
  (((ttUser32Bit)(_x_) & 0x1fffffff) | hal_pci_inbound_window_base)
#define BUS_TO_VIRT(_x_) \
  (((ttUser32Bit)(_x_) & hal_pci_inbound_window_mask) | 0xC0000000)



/*
 * Receive buffer descriptor.  Used to describe one buffer of a frame. A frame
 * could consist of one or more of these buffers, but in this driver will only
 * consist of a single buffer per frame.
 */
typedef struct tsRecvBufDesc
{

/* Physical adddress pointer to buffer's data */
    char TM_FAR *                 bufDataPtr;
/* 
 * Treck buffer handle (not present in normal 8254x descriptor - must remain 
 * at end of structure) 
 */
    ttUserBuffer                  bufHandle;

} ttRecvBufDesc;

typedef ttRecvBufDesc TM_FAR * ttRecvBufDescPtr;


typedef struct ts8254xDescriptor 
{
    unsigned char desRing[TM_I82544_DESC_SIZE];
} tt8254xDescriptor;

typedef tt8254xDescriptor *tt8254xDescriptorPtr;

/*
 * Various state information for this interface
 */
typedef struct ts8254xInfo
{
/* plusing 2 is for dcache flush, because Xscale cache line 
 * size is 32
 */ 
    unsigned char     dummpy[32];
    tt8254xDescriptor i8254xCmdRing[TM_8254X_CMD_RING_SIZE+2];        
    tt8254xDescriptor i8254xRecvDesc[TM_8254X_RECV_RING_SIZE+2];
/* Base memory address for the 8254x */
    unsigned char TM_FAR * i8254xMemAddrPtr;
/* Base I/O address for the 8254x */
    ttUser32Bit i8254xIoAddr;
/* Interrupt line */
    unsigned char i8254xIntLine;
/* send ring variables */
    int i8254xCmdRingTailIndex;
    int i8254xCmdRingDirtyIndex;

    int i8254xCmdRingStatus[TM_8254X_CMD_RING_SIZE];
/* Current buffer number for current frame */
    int i8254xScatterIndex;

/* 
 * Receive frame ring, buffer descriptors and indexes
 */
    ttRecvBufDesc   i8254xRecvBufDesc[TM_8254X_RECV_RING_SIZE];

    int i8254xRecvRingIndex;
    int i8254xRecvRingDirtyIndex;  
    int i8254xRecvRingIsrIndex;  

/* Treck interface handle for this device */
    ttUserInterface i8254xInterfaceHandle;

#if defined(TM_DEV_SEND_OFFLOAD) || defined(TM_DEV_RECV_OFFLOAD)
/* enable receive checksum and packet checksum calculation */
    unsigned char i8254xRecvCheckSumEnable;
    unsigned char i8254xRecvCheckSumCompute;
/* cached variables for offload */
    unsigned char i8254xCacheDcmd;
    unsigned char i8254xCachePopts;
    int           i8254xCacheDevoFlags;
    ttPktLen      i8254xCacheChainLength;
    unsigned char i8254xTcpContextEnable;
#endif /* TM_DEV_SEND_OFFLOAD || TM_DEV_RECV_OFFLOAD */

} tt8254xInfo;

typedef tt8254xInfo *tt8254xInfoPtr;


#define TM_8254X_IOCTL_REFILL        0x0001
#define TM_8254X_IOCTL_SEND_COMPLETE 0x0002

/*
 * Driver API prototypes
 */
int tf8254xOpen(ttUserInterface interfaceHandle);
int tf8254xClose(ttUserInterface interfaceHandle);
int tf8254xIoctl(ttUserInterface interfaceHandle,
                 int             flag,
                 void TM_FAR *   optionPtr,
                 int             optionLen);
int tf8254xSend(ttUserInterface interfaceHandle,
                char TM_FAR *   dataPtr,
                int             dataLength,
                int             flag);
int tf8254xReceive(ttUserInterface        interfaceHandle,
                   char TM_FAR * TM_FAR * dataPtr,
                   int  TM_FAR *          dataLength,
                   ttUserBufferPtr        bufHandlePtr
#ifdef TM_DEV_RECV_OFFLOAD
                 , ttDevRecvOffloadPtr    devoRPtr
#endif /* TM_DEV_RECV_OFFLOAD */
                 );

int tf8254xGetPhysAddr(ttUserInterface interfaceHandle,
                       char TM_FAR *   physicalAddress);

int tf8254xScatDrvSend ( ttUserInterface  interfaceHandle,
                        ttUserPacketPtr  uPacketPtr );
