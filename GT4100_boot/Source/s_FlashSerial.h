#ifndef _S_FLASHSERIAL_
#define _S_FLASHSERIAL_
//[]----------------------------------------------------------------------[]
///	@file		s_FlashSerial.h
///	@brief		シリアルフラッシュROM SPI関連定義ヘッダ
///	@date		2012/03/29
///	@author		A.iiizumi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]

//******************************************************************************
// Typedef definitions
//******************************************************************************

//******************************************************************************
// Macro definitions
//******************************************************************************

/* Status Register Bit Definitions */
#define SR_BSY			0x01
#define SR_BPL			0x08
#define SR_BP_SET(x)	(x<<2)	// BPL=0,BP3-0=x

#define FROM_SECTOR_SIZE	4096
#define RSPI_BUF_SIZE	FROM_SECTOR_SIZE

//******************************************************************************
// extern definitions
//******************************************************************************

// memdeta.c
extern	unsigned char	Rspi_RcvBuf[RSPI_BUF_SIZE];// RSPI受信バッファ

// FlashSerial_RSPI_boot.c
extern void Rspi_from_Init(void);
extern unsigned char FlashReadStatus_direct(void);
#endif
