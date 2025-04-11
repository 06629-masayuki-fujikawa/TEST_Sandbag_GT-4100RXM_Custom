/*************************************************************************/ /**
 *	\file	SodiacPer_memory.c
 *	\brief	Sodiac ペリフェラル メモリ for RX630
 *	\author	Copyright(C) Arex Co. 2008-2012 All right Reserved.
 *	\author	株式会社アレックス 2008-2012
 *	\date	CREATE : 17/Nov/2008
 *	\date	UPDATE : 2010/01/21
 *	\date	UPDATE : 2010/06/15
 *	\date	UPDATE : 2012/02/14
 *	\version Sodiac 2.30
 **/ /*************************************************************************/

#include "iodefine.h"
#include "vect.h"
#include "SodiacPer_memory.h"
#include "system.h"

// DMA,SPIの割込み占有時間を短縮して他の割込み処理を稼動させる為[16]とする
//	重要 ------->	128 -> 16に変更するとAddress問題のﾌﾘｰｽﾞの発生頻度が高くなるので
//					Arex様の対応(sodiac_Q_AのNo13)が完了した時点で128 -> 16の対応を行う
#define	BUFFSIZE	(16)		/* 音声データキャッシュサイズ(byte) */


/*************************************************************************/ /**
 *	シリアル通信レジスタ
 *	実際の回路にあわせて変更してください
 **/ /*************************************************************************/
#define	SPI_OUT		RSPI0.SPDR	/* 送信バッファ */
#define	SPI_IN		RSPI0.SPDR	/* 受信バッファ */

union Long {
	unsigned char	ucaByte[4];
	unsigned long	ulLong;
};

unsigned long	g_lAddress=0xffffffffUL;

/* データキャッシュステート */
typedef enum {
	CS_IDLE = 0,				/**< 転送せず */
	CS_HEAD,					/**< ヘッダアクセス */
	CS_READ0,					/**< チャンネル0リード */
	CS_READ1,					/**< チャンネル1リード */
}	CASHESTATE;

static CASHESTATE		s_State;				/**< データキャッシュステート */
static unsigned char	s_cReadState;			/**< フラッシュリードコマンドステート */
static unsigned char	*s_pucData;				/**< データ格納ポインタ */
static unsigned short	s_usCount;				/**< 読み出しデータカウンタ */
static unsigned long	*s_ulFirstAddress[2]={	/**< バッファ中の先頭アドレス */
	&SodiacSoundAddress,&SodiacSoundAddress		/* 	音声データ上にＣＨ毎に保存領域を分けていないため */
												/*	同じアドレスを入れておく						 */
};

static unsigned long	s_ulStartAddress[2]={	/**< バッファ中の先頭アドレス */
	0xffffffffUL,0xffffffffUL
};

static unsigned char	s_ucaaDataBuff[2][BUFFSIZE];	/**< キャッシュバッファ */
static union Long		s_Long;
unsigned char	Sodiac_FROM_Get;


/*************************************************************************/ /**
 *	SPI割り込み処理
 *	\date	Create 2012/02/08	Iwasawa H.(AREX)
 **/ /*************************************************************************/
void	Sodiac_From_Rcv( void )
{
	unsigned char	a_ucData=SPI_IN; /* 受信データ読み取り */

	switch(s_cReadState){			/* コマンド送信ステート */
	case	0:					/* READコマンド送出終了 */
		SPI_OUT=s_Long.ucaByte[1];	/* アドレス b23～b16 */
		break;
	case	1:
		SPI_OUT=s_Long.ucaByte[2];	/* アドレス b15～b8 */
		break;
	case	2:
		SPI_OUT=s_Long.ucaByte[3];	/* アドレス b7～b0 */
		break;
	case	3:					/* 次からデータ読み出し */
		if(s_usCount == 1){
			RSPI0.SPCMD0.WORD = 0x2703;// SSL信号は転送完了で解除
		}
		SPI_OUT=0xFF;			/* ダミー */
		break;
	default:
		++g_lAddress;
		*s_pucData++ =a_ucData;	/* データ格納 */
		if(--s_usCount){
			if(s_usCount == 1){
				RSPI0.SPCMD0.WORD = 0x2703;// SSL信号は転送完了で解除
			}
			SPI_OUT=0xFF;		/* 次の読み込み */
		}else{
			/* 終了 */
			s_State=CS_IDLE;
			RSPI0.SPCR.BIT.SPRIE = 0;	// RSPI受信割り込み禁止
		}
		break;
	}
	if(s_cReadState<4)
		++s_cReadState;
	return;
}

/*************************************************************************/ /**
 *	\param[in]	ulAddress : 読み込みをはじめるアドレス
 *	\date		Update 2012/02/08	Iwasawa H.(AREX)
 **/ /*************************************************************************/
static void SetFlashAddress(unsigned long ulAddress)
{
	s_Long.ulLong=g_lAddress=ulAddress;
	s_cReadState=0;

	while(RSPI0.SPSR.BIT.IDLNF == 1){	}

	RSPI0.SPCMD0.WORD = 0x2783;			// 転送の途中より SSL信号はアクティブを保持
	RSPI0.SPCR.BIT.SPRIE = 1;			// RSPI受信割り込み許可
	SPI_OUT=0x03;						/* フラッシュ読み出し開始 */
}

/*************************************************************************/ /**
 *	\param[in]	ulAddress : 読み出しアドレス
 *	\retval		読みだされたデータ(1byte)
 *	\date		Create 2012/02/08	Iwasawa H.(AREX)
 **/ /*************************************************************************/
unsigned char sodiac_dataread(int iChannel,unsigned long ulAddress)
{
	unsigned long	a_ulOffset;
	unsigned long	ist;		// 現在の割込受付状態

	_ei();
	ulAddress += *s_ulFirstAddress[iChannel];
	if( 0xffffffffUL != s_ulStartAddress[iChannel])
	{
		a_ulOffset = ulAddress - s_ulStartAddress[iChannel];
	}
	else
	{
		a_ulOffset = 0;
	}

	/* この範囲内ならデータが存在する */
	if((s_ulStartAddress[iChannel]<=ulAddress)&&(a_ulOffset<BUFFSIZE)){
		/* 自分のチャンネルを読み出し中 */
		if(((CS_READ0==s_State)&&(0==iChannel))||
		   ((CS_READ1==s_State)&&(1==iChannel))){
			while(g_lAddress<ulAddress){ /* 読み出したいところまで読まれるまで待つ */
			}
		}
		return	s_ucaaDataBuff[iChannel][a_ulOffset];
	}
	/* バッファにデータがなかったので読み込みを開始 */

	/* まだデータ読み込みがされていないかバッファ外 */
	while(CS_IDLE!=s_State);
	while(RSPI0.SPSR.BIT.IDLNF == 1);
	ist = _di2();
	s_State=(0==iChannel)?CS_READ0:CS_READ1;
	s_usCount=BUFFSIZE;
	s_pucData=s_ucaaDataBuff[iChannel];
	s_ulStartAddress[iChannel]=ulAddress;

	SetFlashAddress(ulAddress);

	_ei2(ist);
	while(g_lAddress<=ulAddress); 		/* 読み出したいところまで読まれるまで待つ */

	// 書き込み権開放
	return	s_ucaaDataBuff[iChannel][0];
}

/*************************************************************************/ /**
 *	\param[in]	ulAddress : 読み出しアドレス
 *	\retval		読みだされたデータ(1byte)
 *	\date		Create 2012/02/08	Iwasawa H.(AREX)
 **/ /*************************************************************************/
unsigned char sodiac_headerread(unsigned long ulAddress)
{
	unsigned char	a_ucData;
	unsigned long	ist;		// 現在の割込受付状態

	while(CS_IDLE!=s_State);
	while(RSPI0.SPSR.BIT.IDLNF == 1);
	ist = _di2();
	s_State=CS_HEAD;
	s_usCount=1;
	s_pucData=&a_ucData;

	ulAddress += *s_ulFirstAddress[0];
	SetFlashAddress(ulAddress);
	_ei2(ist);
	while(CS_IDLE!=s_State); /* 読み出しが終わるまで待つ */

	return	a_ucData;
}

/*************************************************************************/ /**
 *	\param[out]	vpPt      : データ格納バッファ
 *	\param[in]	ulAddress : 読み出しアドレス
 *	\param[in]	iSize     : データ読み出しサイズ
 *	\retval		D_SODIAC_E_OK : 成功
 *	\date		Create 2012/02/08	Iwasawa H.(AREX)
 **/ /*************************************************************************/
D_SODIAC_E sodiac_nread(int iChannel, void *vpPt, unsigned long ulAddress, int iSize)
{
	unsigned char	*a_pucData;

	if(!vpPt)
		return	D_SODIAC_E_PAR;	/* 無効ポインタ */

	s_ulStartAddress[iChannel]	= 0xffffffffUL;					// 初期値に戻す
	a_pucData=(unsigned char *)vpPt;
	for(;0<iSize;--iSize){
		*a_pucData++ =sodiac_dataread(iChannel, ulAddress++);
	}

	return	D_SODIAC_E_OK;
}

/*************************************************************************/ /**
 *	\param[out]	vpPt      : データ格納バッファ
 *	\param[in]	ulAddress : 読み出しアドレス
 *	\param[in]	iSize     : データ読み出しサイズ
 *	\retval		D_SODIAC_E_OK : 成功
 *	\date		Create 2012/02/08	Iwasawa H.(AREX)
 **/ /*************************************************************************/
D_SODIAC_E sodiac_hnread(void *vpPt, unsigned long ulAddress, int iSize)
{
	unsigned long	ist;		// 現在の割込受付状態

	if(!vpPt)
		return	D_SODIAC_E_PAR;	/* 無効ポインタ */

	while(CS_IDLE!=s_State);
	while(RSPI0.SPSR.BIT.IDLNF == 1);
	ist = _di2();
	s_State=CS_HEAD;
	s_usCount=iSize;
	s_pucData=vpPt;

	ulAddress += *s_ulFirstAddress[0];
	SetFlashAddress(ulAddress);
	_ei2(ist);
	while(CS_IDLE!=s_State); /* 読み出しが終わるまで待つ */

	return	D_SODIAC_E_OK;
}

/*************************************************************************/ /**
 *			All rights reserved, Copyright (C) Arex.Co.Ltd, 2008-2012		  *
 **/ /*************************************************************************/
