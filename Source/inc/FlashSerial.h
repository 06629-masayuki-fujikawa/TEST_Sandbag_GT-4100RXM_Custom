#ifndef _FLASHSERIAL_
#define _FLASHSERIAL_
//[]----------------------------------------------------------------------[]
///	@file		FlashSerial.h
///	@brief		シリアルフラッシュROM SPI関連定義ヘッダ
///	@date		2012/02/09
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]

//******************************************************************************
// Typedef definitions
//******************************************************************************
/** 受信バッファ管理テーブル **/
typedef struct {
    ushort  RcvReqCnt;					// 受信要求バイト数
    ushort  RcvCmpCnt;					// 受信完了バイト数
    ushort  RcvCmpFlg;					// 受信完了したフラグ 1=完了,0=未完了
} t_RspiRcvCtrl;

/** 送信バッファ管理テーブル **/
typedef struct {
    ushort  SndReqCnt;					// 送信要求バイト数
    ushort  SndCmpCnt;					// 送信完了バイト数
    ushort  ReadIndex;					// next send data (read) index
    ushort  SndCmpFlg;					// 送信完了したフラグ 1=完了,0=未完了
} t_RspiSndCtrl;

//******************************************************************************
// Macro definitions
//******************************************************************************

/* Status Register Bit Definitions */
#define SR_BSY			0x01
#define SR_BPL			0x08
#define SR_BP_SET(x)	(x<<2)	// BPL=0,BP3-0=x

#define FROM_SECTOR_SIZE	4096
// SPI FROM バッファサイズ(リードコマンド03H、コマンド1Byte、アドレス3Byte、データ4096(4K)byteを最大とするため)
#define RSPI_BUF_SIZE	(FROM_SECTOR_SIZE + 4) 

//******************************************************************************
// extern definitions
//******************************************************************************

// memdeta.c
extern	t_RspiRcvCtrl	Rspi_RcvCtrl;				// RSPI受信管理情報(割り込みハンドラ使用)
extern	t_RspiSndCtrl	Rspi_SndCtrl;				// RSPI送信管理情報(割り込みハンドラ使用)
extern	unsigned char	Rspi_RcvBuf[RSPI_BUF_SIZE];// RSPI受信バッファ(割り込みハンドラ使用)
extern	unsigned char	Rspi_SndBuf[RSPI_BUF_SIZE];// RSPI送信バッファ(割り込みハンドラ使用)

// FlashSerialSST.c
extern unsigned char From_Access_SemGet(unsigned char FlashSodiac);
extern void From_Access_SemFree(unsigned char FlashSodiac);
extern void SetRsten(void);
extern void SetRst(void);
extern unsigned char FlashReadStatus(void);
extern void FlashWriteStatus( unsigned char );
extern unsigned char  FlashErase ( const unsigned long, const unsigned char );
extern unsigned char  FlashReadData ( const unsigned long, unsigned char *, const unsigned long );
extern void FlashWriteData ( unsigned long, unsigned char const *);
extern void FlashJEDECReadId ( unsigned char * );

extern unsigned char FlashReadStatus_direct(void);
extern void FlashWriteStatus_direct(unsigned char );
extern unsigned char FlashErase_direct(const unsigned long , const unsigned char );
extern void FlashWriteData_direct(unsigned long , unsigned char const *);
extern unsigned char FlashReadData_direct(const unsigned long , unsigned char *, const unsigned long );
extern void FlashJEDECReadId_direct(unsigned char *);

// FlashSerial_RSPI.c
extern void Rspi_from_Init(void);
extern void Rspi_from_SPRI( void );
extern unsigned char Rspi_from_SndReq( unsigned short Length );
extern unsigned char Rspi_from_IsSndRcvCmp( void );
#endif
