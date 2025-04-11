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
 *
 * Filename: tr7616e.h
 * Author: Sebastien Windal
 * Date Created:
 * $Source: include/tr7616e.h $
 *
 * Modification History
 * $Revision: 6.0.2.2 $
 * $Date: 2010/01/18 21:33:10JST $
 * $Author: odile $
 * $ProjectName: /home/mks/proj/tcpip.pj $
 * $ProjectRevision: 6.0.1.33 $
 */

#ifndef _TRSH7616_H_
#define _TRSH7616_H_

#include <trsocket.h>

//#define ETHERSIZE   1536
#define ETHERSIZE   100

#define BUFFSIZE    ( ETHERSIZE )

#define DSC_COUNT   10
#define PROMISC_OFF 0
#define PROMISC_ON  1

#define MULTIC_OFF  0
#define MULTIC_ON   1

#define ACT         0x80000000
#define DL_BIT      0x40000000
#define FP1         0x20000000
#define FP0         0x10000000
#define FE          0x08000000


#define EESR_RFCOF  0x01000000

#define EESR_ECI    0x00400000
#define EESR_TC     0x00200000
#define EESR_TDE    0x00100000

#define EESR_TFUF   0x00080000
#define EESR_FR     0x00040000
#define EESR_RDE    0x00020000
#define EESR_RFOF   0x00010000

#define EESR_ITF    0x00001000

#define EESR_CND    0x00000800
#define EESR_DLC    0x00000400
#define EESR_CD     0x00000200
#define EESR_TRO    0x00000100

#define EESR_RMAF   0x00000080
#define EESR_RFAR   0x00000020
#define EESR_RRF    0x00000010

#define EESR_RTLF   0x00000008
#define EESR_RTSF   0x00000004
#define EESR_PRE    0x00000002
#define EESR_CERF   0x00000001


#define EXC_INT_LAN     69
#define KNL_MASK_LEVEL  14

typedef struct tsEdmacDesc
{
    volatile unsigned long  uwStatus;
    unsigned short          uhBufSize;
    unsigned short          uhSize;
    unsigned char           *ubpBuf;
    struct tsEdmacDesc      *stpNext;
} ttEdmacDesc;


typedef struct tsSH7616Ether
{
    unsigned long           uwOpenFlag;
    ttUserInterface         hnInterfaceHandle;
    unsigned long           uwMacAddrHigh;
    unsigned long           uwMacAddrLow;
    unsigned long           uwMultiCastFlag;
    volatile unsigned long  uwEdmacEESR;
    volatile unsigned long  uwEdmacRDFAR;
    volatile unsigned long  uwSaveEdmacRDFAR;
    volatile unsigned long  uwEdmacTDFAR;
    volatile unsigned long  uwSaveEdmacTDFAR;
    ttEdmacDesc             *uhSendDescPtr;
    ttEdmacDesc             *uhRecvDescPtr;
    unsigned short          uhSendBufCount;
    unsigned short          uhRecvBufCount;
    unsigned short          uhIntErrRFCOF;
    unsigned short          uhIntErrECI;
    unsigned short          uhIntStaTC;
    unsigned short          uhIntErrTDE;
    unsigned short          uhIntErrTFUF;
    unsigned short          uhIntStaFR;
    unsigned short          uhIntErrRDE;
    unsigned short          uhIntErrRFOF;
    unsigned short          uhIntErrITF;
    unsigned short          uhIntErrCND;
    unsigned short          uhIntErrDLC;
    unsigned short          uhIntErrCD;
    unsigned short          uhIntErrTRO;
    unsigned short          uhIntStaRMAF;
    unsigned short          uhIntErrRRF;
    unsigned short          uhIntErrRTLF;
    unsigned short          uhIntErrRTSF;
    unsigned short          uhIntErrPRE;
    unsigned short          uhIntErrCERF;
    unsigned short          uhReserved;
} ttSH7616Ether;


/* external variables */
extern volatile ttEdmacDesc tvRcvDesc[DSC_COUNT];
extern volatile ttEdmacDesc tvSndDesc[DSC_COUNT];

extern char     RcvBuff[DSC_COUNT][BUFFSIZE + 16];
extern char     SndBuff[DSC_COUNT][BUFFSIZE + 16];

extern ttSH7616Ether  tvEther;


/* external functions */
extern int  tfSH7616DriverOpen(ttUserInterface);
extern int  tfSH7616EthDriverClose(ttUserInterface);
extern int  tfSH7616EthDriverRecv(ttUserInterface, char TM_FAR **, int TM_FAR *, ttUserBufferPtr);
extern int  tfSH7616EthDriverIoctl(ttUserInterface, int, void TM_FAR *, int);
extern int  tfSH7616EthDriverGetPhysAddr( ttUserInterface, char TM_FAR * );
extern int  tfSH7617DriverGetCtrlInfo( ttUserInterface, ttSH7616Ether * );
extern int  tfSH7616EthDriverPromiscuousMode( ttUserInterface, int );
extern int  tfSH7616ScatSend(ttUserInterface, ttUserPacketPtr);

extern void tfSH7616EthernetInit( void );
extern void tfSH7616EthernetOpen( void );
extern void tfSH7616EthernetClose( void );

extern int  tfSetMaskLevel( int );
extern void tfSH7616EtherIntEntry( void );
extern void tfSH7616InitPhy( void );



#define tm_sh7616_edmac_rr_set \
        if( EDMAC.EDRRR.LONG == 0x00000000 )    \
        {                                       \
            EDMAC.EDRRR.LONG = 0x00000001;      \
        }

#define tm_sh7616_edmac_tr_set \
        if( EDMAC.EDTRR.LONG == 0x00000000 )    \
        {                                       \
            EDMAC.EDTRR.LONG = 0x00000001;      \
        }


/* CPU/EtherC/EDMAC Register definitions */

#define MAP_IO      0xFFFFFE00
#define TOP_UBC     MAP_IO+0x100
#define TOP_INTC    MAP_IO+0x060
#define TOP_DMAC    MAP_IO+0x070
#define TOP_BSC     MAP_IO+0x1e0
#define TOP_SCI1    MAP_IO+0x0
#define TOP_CACHE   MAP_IO+0x92

/*************************/
/*         BSC           */
/*************************/
#define BCR1 (volatile long*)(TOP_BSC+0x00)         /* for SH7615   *//* FFFFFFE0 */
#define BCR2 (volatile long*)(TOP_BSC+0x04)         /* for SH7615   *//* FFFFFFE4 */
#define BCR3 (volatile long*)(TOP_BSC+0x1C)         /* for SH7615   *//* FFFFFFFC */
#define WCR1 (volatile long*)(TOP_BSC+0x08)         /* for SH7615   *//* FFFFFFE8 */
#define WCR2 (volatile long*)(TOP_BSC-0x20)         /* for SH7615   *//* FFFFFFC0 */
#define WCR3 (volatile long*)(TOP_BSC-0x1C)         /* for SH7615   *//* FFFFFFC4 */
#define MCR  (volatile long*)(TOP_BSC+0x0c)         /* for SH7615   *//* FFFFFFEC */
#define RTCSR (volatile long*)(TOP_BSC+0x10)        /* for SH7615   *//* FFFFFFF0 */
#define RTCNT (volatile long*)(TOP_BSC+0x14)        /* for SH7615   *//* FFFFFFF4 */
#define RTCOR (volatile long*)(TOP_BSC+0x18)        /* for SH7615   *//* FFFFFFF8 */

/********************/
/*        SDMR      */
/********************/
#define SDMR (*(volatile unsigned short *)0xffff0088)/* for SH7615 '99.10.17*/
/* Mode ---------> burst read/single write  */      /* for SH7615   */
/* CAS Latency --> 3                        */      /* for SH7615   */
/* Burst Length -> 4                        */      /* for SH7615   */

/********************/
/*        UBC       */
/********************/
#define BARA   (volatile long*)(TOP_UBC+0x00)
#define BAMRA  (volatile long*)(TOP_UBC+0x04)
#define BARAH  (volatile short*)(TOP_UBC+0x00)
#define BARAL  (volatile short*)(TOP_UBC+0x02)
#define BAMRAH (volatile short*)(TOP_UBC+0x04)
#define BAMRAL (volatile short*)(TOP_UBC+0x06)
#define BBRA   (volatile short*)(TOP_UBC+0x08)

#define BARB   (volatile long*)(TOP_UBC+0x20)
#define BAMRB  (volatile long*)(TOP_UBC+0x24)
#define BARBH  (volatile short*)(TOP_UBC+0x20)
#define BARBL  (volatile short*)(TOP_UBC+0x22)
#define BAMRBH (volatile short*)(TOP_UBC+0x24)
#define BAMRBL (volatile short*)(TOP_UBC+0x26)
#define BBRB   (volatile short*)(TOP_UBC+0x28)

#define BARC   (volatile long*)(TOP_UBC+0x40)
#define BAMRC  (volatile long*)(TOP_UBC+0x44)
#define BBRC   (volatile short*)(TOP_UBC+0x48)
#define BRCR   (volatile long*)(TOP_UBC+0x30)

/********************/
/*       INTC       */
/********************/
#define IPRA (volatile short*)(TOP_INTC+0x82)
#define IPRB (volatile short*)(TOP_INTC+0x00)
#define IPRD (volatile short*)(0xfffffe40)

#define VCRA (volatile short*)(TOP_INTC+0x02)
#define VCRB (volatile short*)(TOP_INTC+0x04)
#define VCRC (volatile short*)(TOP_INTC+0x06)
#define VCRD (volatile short*)(TOP_INTC+0x08)
#define VCRE (volatile short*)(0xFFFFFE42)
#define VCRL (volatile short*)(0xfffffe50)

#define VCRWDT (volatile short*)(TOP_INTC+0x84)
#define ICR (volatile short*)(TOP_INTC+0x80)

/********************/
/*       DMAC       */
/********************/
#define SAR0 (volatile long*)(TOP_DMAC+0x110)
#define DAR0 (volatile long*)(TOP_DMAC+0x114)
#define DMAOR (volatile long*)(TOP_DMAC+0x140)
#define TCR0 (volatile long*)(TOP_DMAC+0x118)
#define CHCR0 (volatile long*)(TOP_DMAC+0x11c)
#define SAR1 (volatile long*)(TOP_DMAC+0x120)
#define DAR1 (volatile long*)(TOP_DMAC+0x124)
#define TCR1 (volatile long*)(TOP_DMAC+0x128)
#define CHCR1 (volatile long*)(TOP_DMAC+0x12c)
#define VCRDMA0 (volatile long*)(TOP_DMAC+0x130)
#define VCRDMA1 (volatile long*)(TOP_DMAC+0x138)
#define DRCR0 (volatile char*)(TOP_DMAC+0x1)
#define DRCR1 (volatile char*)(TOP_DMAC+0x2)

/*******************/
/*       SCI       */
/*******************/
/*#define SMR (volatile char*)(TOP_SCI1+0x00)
#define BRR (volatile char*)(TOP_SCI1+0x01)
#define SCR (volatile char*)(TOP_SCI1+0x02)
#define TDR (volatile char*)(TOP_SCI1+0x03)
#define SSR (volatile char*)(TOP_SCI1+0x04)
#define RDR (volatile char*)(TOP_SCI1+0x05)*/

/******************/
/*      SCIF1     */
/******************/
#define SCSMR1 (volatile char*)(0xFFFFFCC0)
#define SCBRR1 (volatile char*)(0xFFFFFCC2)
#define SCSCR1 (volatile char*)(0xFFFFFCC4)
#define SCFTDR1 (volatile char*)(0xFFFFFCC6)
#define SCSSR1 (volatile short*)(0xFFFFFCC8)
#define SCFRDR1 (volatile char*)(0xFFFFFCCC)
#define SCFCR1 (volatile char*)(0xFFFFFCCE)
#define SCFDR1 (volatile short*)(0xFFFFFCD0)

/******************/
/*       PFC      */
/******************/
#define PBCR (volatile short*)(0xFFFFFC88)

/******************/
/*      CACHE     */
/******************/
#define CCR (volatile char*)(TOP_CACHE+0x00)

/********************/
/* TL16C552 SCI CH0 */
/********************/
#define TL0RBR (volatile char*)(0x700000)
#define TL0THR (volatile char*)(0x700000)
#define TL0IER (volatile char*)(0x700002)
#define TL0IIR (volatile char*)(0x700004)
#define TL0FCR (volatile char*)(0x700004)
#define TL0LCR (volatile char*)(0x700006)
#define TL0MCR (volatile char*)(0x700008)
#define TL0LSR (volatile char*)(0x70000a)
#define TL0MSR (volatile char*)(0x70000c)
#define TL0SCR (volatile char*)(0x70000e)
#define TL0DLL (volatile char*)(0x700000)
#define TL0DLM (volatile char*)(0x700002)

/**************************/
/* TL16C552 Parallel port */
/**************************/
#define Para_r_data (volatile char*)(0x22900000)
#define Para_w_data (volatile char*)(0x22900000)
#define Para_r_status (volatile char*)(0x22900002)
#define Para_r_cont (volatile char*)(0x22900004)
#define Para_w_cont (volatile char*)(0x22900004)

/*******************/
/*       SW0       */
/*******************/
#define SW0 (volatile short*)(0x22100000)

/*******************/
/*     SDRAM       */
/*******************/
#define SD_BANK1 (volatile short*)(0x06000000)
#define SD_BANK2 (volatile short*)(0x06800000)
#define SD_BANK3 (volatile short*)(0x07000000)
#define SD_BANK4 (volatile short*)(0x07800000)

/* Free Running timer registers */
#define TIER    (volatile char *)(0xFFFFFE10)
#define FTCSR   (volatile char *)(0xFFFFFE11)
#define TIERFTCSR (volatile short *)(0xFFFFFE10)
#define FRC     (volatile short *)(0xFFFFFF12)
#define FRCH    (volatile char *)(FRC)
#define FRCL    (volatile char *)(FRC+1)
#define OCRA    (volatile short *)(0xFFFFFE14)
#define OCRAH   (volatile char *)(OCRA)
#define OCRAL   (volatile char *)(OCRA+0x01)
#define OCRB    (volatile short *)(0xFFFFFE14)
#define OCRBH   (volatile char *)(OCRA)
#define OCRBL   (volatile char *)(OCRA+0x01)
#define TCR     (volatile char *)(0xFFFFFF16)
#define TOCR    (volatile char *)(0xFFFFFF17)
#define FICR    (volatile short *)(0xFFFFFF18)
#define FICRH   (volatile char *)(FICR)
#define FICRL   (volatile char *)(FICR+1)


#define TSTR    (volatile char *)(0xFFFFFC40)
#define TGR0A   (volatile short*)(0xFFFFFC58)

/* 8-BIT LED REGISTER */
#define TM_8BIT_LED_RGSTR (volatile unsigned char *)(0x2200000)


/* Ethernet controller */
struct st_ec {                                          /* struct EtherC*/
             union {                                    /* ECMR         */
                   unsigned long LONG;                   /*  Long Access */
                   struct {                             /*  Bit  Access */
                          unsigned long      :19;        /*              */
                          unsigned int PRCEF: 1;        /*    PRCEF     */
                          unsigned int      : 2;        /*              */
                          unsigned int MPDE : 1;        /*    MPDE      */
                          unsigned int      : 2;        /*              */
                          unsigned int RE   : 1;        /*    RE        */
                          unsigned int TE   : 1;        /*    TE        */
                          unsigned int      : 1;        /*              */
                          unsigned int ILB  : 1;        /*    ILB       */
                          unsigned int ELB  : 1;        /*    ELB       */
                          unsigned int DM   : 1;        /*    DM        */
                          unsigned int PRM  : 1;        /*    PRM       */
                          }       BIT;                  /*              */
                   }            ECMR;                   /*              */
             union {                                    /* ECSR         */
                   unsigned long LONG;                   /*  Long Access */
                   struct {                             /*  Bit  Access */
                          unsigned long      :29;        /*              */
                          unsigned int LCHNG: 1;        /*    LCHNG     */
                          unsigned int MPR  : 1;        /*    MPR       */
                          unsigned int ICD  : 1;        /*    ICD       */
                          }       BIT;                  /*              */
                   }            ECSR;                   /*              */
             union {                                    /* ECSIPR       */
                   unsigned long LONG;                   /*  Long Access */
                   struct {                             /*  Bit  Access */
                          unsigned long        :29;      /*              */
                          unsigned int LCHNGIP: 1;      /*    LCHNGIP   */
                          unsigned int MPRIP  : 1;      /*    MPRIP     */
                          unsigned int ICDIP  : 1;      /*    ICDIP     */
                          }       BIT;                  /*              */
                   }            ECSIPR;                 /*              */
             union {                                    /* PIR          */
                   unsigned long LONG;                   /*  Long Access */
                   struct {                             /*  Bit  Access */
                          unsigned long    :28;          /*              */
                          unsigned int MDI: 1;          /*    MDI       */
                          unsigned int MDO: 1;          /*    MDO       */
                          unsigned int MMD: 1;          /*    MMD       */
                          unsigned int MDC: 1;          /*    MDC       */
                          }       BIT;                  /*              */
                   }            PIR;                    /*              */
             unsigned int       MAHR;                   /* MAHR         */
             unsigned int       MALR;                   /* MALR         */
             unsigned int       RFLR;                   /* RFLR         */
             union {                                    /* PSR          */
                   unsigned long LONG;                   /*  Long Access */
                   struct {                             /*  Bit  Access */
                          unsigned long     :31;         /*              */
                          unsigned int LMON: 1;         /*    LMON      */
                          }       BIT;                  /*              */
                   }            PSR;                    /*              */
             unsigned int       TROCR;                  /* TROCR        */
             unsigned int       CDCR;                   /* CDCR         */
             unsigned int       LCCR;                   /* LCCR         */
             unsigned int       CNDCR;                  /* CNDCR        */
             unsigned int       IFLCR;                  /* IFLCR        */
             unsigned int       CEFCR;                  /* CEFCR        */
             unsigned int       FRECR;                  /* FRECR        */
             unsigned int       TSFRCR;                 /* TSFRCR       */
             unsigned int       TLFRCR;                 /* TLFRCR       */
             unsigned int       RFCR;                   /* RFCR         */
             unsigned int       MAFCR;                  /* MAFCR        */
};


struct st_edmac {                                       /* struct E-DMAC*/
                union {                                 /* EDMR         */
                      unsigned long LONG;                /*  Long Access */
                      struct {                          /*  Bit  Access */
                             unsigned long    :26;       /*              */
                             unsigned int DL : 2;       /*    DL        */
                             unsigned int    : 3;       /*              */
                             unsigned int SWR: 1;       /*    SWR       */
                             }     BIT;                 /*              */
                      }         EDMR;                   /*              */
                union {                                 /* EDTRR        */
                      unsigned long LONG;                /*  Long Access */
                      struct {                          /*  Bit  Access */
                             unsigned long   :31;        /*              */
                             unsigned int TR: 1;        /*    TR        */
                             }     BIT;                 /*              */
                      }         EDTRR;                  /*              */
                union {                                 /* EDRRR        */
                      unsigned long LONG;                /*  Long Access */
                      struct {                          /*  Bit  Access */
                             unsigned long   :31;        /*              */
                             unsigned int RR: 1;        /*    RR        */
                             }     BIT;                 /*              */
                      }         EDRRR;                  /*              */
                void           *TDLAR;                  /* TDLAR        */
                void           *RDLAR;                  /* RDLAR        */
                union {                                 /* EESR         */
                      unsigned int LONG;                /*  Long Access */
                      struct {                          /*  Bit  Access */
                             unsigned int      :7;      /*              */
                             unsigned int RFCOF:1;      /*    RFCOF     */
                             unsigned int      :1;      /*              */
                             unsigned int ECI  :1;      /*    ECI       */
                             unsigned int TC   :1;      /*    TC        */
                             unsigned int TDE  :1;      /*    TDE       */
                             unsigned int TFUF :1;      /*    TFUF      */
                             unsigned int FR   :1;      /*    FR        */
                             unsigned int RDE  :1;      /*    RDE       */
                             unsigned int RFOF :1;      /*    RFOF      */
                             unsigned int      :3;      /*              */
                             unsigned int ITF  :1;      /*    ITF       */
                             unsigned int CND  :1;      /*    CND       */
                             unsigned int DLC  :1;      /*    DLC       */
                             unsigned int CD   :1;      /*    CD        */
                             unsigned int TRO  :1;      /*    TRO       */
                             unsigned int RMAF :1;      /*    RMAF      */
                             unsigned int      :2;      /*              */
                             unsigned int RRF  :1;      /*    RRF       */
                             unsigned int RTLF :1;      /*    RTLF      */
                             unsigned int RTSF :1;      /*    RTSF      */
                             unsigned int PRE  :1;      /*    PRE       */
                             unsigned int CERF :1;      /*    CERF      */
                             }     BIT;                 /*              */
                      }         EESR;                   /*              */
                union {                                 /* EESIPR       */
                      unsigned int LONG;                /*  Long Access */
                      struct {                          /*  Bit  Access */
                             unsigned int        :7;    /*              */
                             unsigned int RFCOFIP:1;    /*    RFCOFIP   */
                             unsigned int        :1;    /*              */
                             unsigned int ECIIP  :1;    /*    ECIIP     */
                             unsigned int TCIP   :1;    /*    TCIP      */
                             unsigned int TDEIP  :1;    /*    TDEIP     */
                             unsigned int TFUFIP :1;    /*    TFUFIP    */
                             unsigned int FRIP   :1;    /*    FRIP      */
                             unsigned int RDEIP  :1;    /*    RDEIP     */
                             unsigned int RFOFIP :1;    /*    RFOFIP    */
                             unsigned int        :3;    /*              */
                             unsigned int ITFIP  :1;    /*    ITFIP     */
                             unsigned int CNDIP  :1;    /*    CNDIP     */
                             unsigned int DLCIP  :1;    /*    DLCIP     */
                             unsigned int CDIP   :1;    /*    CDIP      */
                             unsigned int TROIP  :1;    /*    TROIP     */
                             unsigned int RMAFIP :1;    /*    RMAFIP    */
                             unsigned int        :2;    /*              */
                             unsigned int RRFIP  :1;    /*    RRFIP     */
                             unsigned int RTLFIP :1;    /*    RTLFIP    */
                             unsigned int RTSFIP :1;    /*    RTSFIP    */
                             unsigned int PREIP  :1;    /*    PREIP     */
                             unsigned int CERFIP :1;    /*    CERFIP    */
                             }     BIT;                 /*              */
                      }         EESIPR;                 /*              */
                union {                                 /* TRSCER       */
                      unsigned long LONG;                /*  Long Access */
                      struct {                          /*  Bit  Access */
                             unsigned long       :19;    /*              */
                             unsigned int ITFCE : 1;    /*    ITFCE     */
                             unsigned int CNDCE : 1;    /*    CNDCE     */
                             unsigned int DLCCE : 1;    /*    DLCCE     */
                             unsigned int CDCE  : 1;    /*    CDCE      */
                             unsigned int TROCE : 1;    /*    TROCE     */
                             unsigned int RMAFCE: 1;    /*    RMAFCE    */
                             unsigned int       : 2;    /*              */
                             unsigned int RRFCE : 1;    /*    RRFCE     */
                             unsigned int RTLFCE: 1;    /*    RTLFCE    */
                             unsigned int RTSFCE: 1;    /*    RTSFCE    */
                             unsigned int PRECE : 1;    /*    PRECE     */
                             unsigned int CERFCE: 1;    /*    CERFCE    */
                             }     BIT;                 /*              */
                      }         TRSCER;                 /*              */
                unsigned int    RMFCR;                  /* RMFCR        */
                unsigned int    TFTR;                   /* TFTR         */
                union {                                 /* FDR          */
                      unsigned long LONG;                /*  Long Access */
                      struct {                          /*  Bit  Access */
                             unsigned long    :23;       /*              */
                             unsigned int TFD: 1;       /*    TFD       */
                             unsigned int    : 7;       /*              */
                             unsigned int RFD: 1;       /*    RFD       */
                             }     BIT;                 /*              */
                      }         FDR;                    /*              */
                union {                                 /* RCR          */
                      unsigned long LONG;                /*  Long Access */
                      struct {                          /*  Bit  Access */
                             unsigned long    :31;       /*              */
                             unsigned int RNC: 1;       /*    RNC       */
                             }     BIT;                 /*              */
                      }         RCR;                    /*              */
                union {                                 /* EDOCR        */
                      unsigned long LONG;                /*  Long Access */
                      struct {                          /*  Bit  Access */
                             unsigned long    :29;       /*              */
                             unsigned int FEC: 1;       /*    FEC       */
                             unsigned int AEC: 1;       /*    AEC       */
                             unsigned int EDH: 1;       /*    EDH       */
                             }     BIT;                 /*              */
                      }         EDOCR;                  /*              */
                char            wk1[12];                /*              */
                void           *RBWAR;                  /* RBWAR        */
                void           *RDFAR;                  /* RDFAR        */
                char            wk2[4];                 /*              */
                void           *TBRAR;                  /* TBRAR        */
                void           *TDFAR;                  /* TDFAR        */
};                    


#define EC    (*(volatile struct st_ec *) 0xFFFFFD60)   /* EtherCAddress */
#define EDMAC (*(volatile struct st_edmac *)0xFFFFFD00) /* E-DMACAddress */

#endif /* _TRSH7616_H_ */
