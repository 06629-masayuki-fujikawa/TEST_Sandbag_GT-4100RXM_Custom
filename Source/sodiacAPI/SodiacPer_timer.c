/*************************************************************************/ /**
/*	\file	SodiacPer_timer.c
 *	\brief	Sodiac ペリフェラル タイマー for RL78/G13
 *	\date	CREATE : 17/Nov/2008 Arex.Co.Ltd,
 *	\date	Update : 2012/03/08  Arex.Co.Ltd,
 *	\version Sodiac 2.30
 **/ /*************************************************************************/

#include "iodefine.h"
#include "SodiacPer_timer.h"


/*************************************************************************/ /**
 *	PWMタイマー開始関数
 *	\date	CREATE : 17/Nov/2008 Y.Tamaki(Arex)
 *	\date	UPDATE : 2009/10/20	Iwasawa H.(Arex)
 **/ /*************************************************************************/
void sodiac_start_timer(void)
{
	/* 割り込み設定 */
	IEN(TPU0,TGI0A)=1;
	TPU0.TIER.BIT.TGIEA = 1;

	/* タイマーの設定 */
	TPUA.TSTR.BIT.CST0 = 1;		/* 動作開始 */
}

/*************************************************************************/ /**
 *	PWMタイマー停止関数
 *	\date	CREATE : 17/Nov/2008 Y.Tamaki(Arex)
 *	\date	UPDATE : 2009/10/20 Iwasawa H.(Arex)
 **/ /*************************************************************************/
void sodiac_stop_timer(void)
{
	/* タイマーカウント停止 */
	TPUA.TSTR.BIT.CST0 = 0;		/* 動作開始 */
	/* タイマー割り込み禁止 */
	TPU0.TIER.BIT.TGIEA = 0;
}

extern unsigned short	*GSODIAC_usaData;

/*************************************************************************/ /**
 *	PWMタイマー設定関数
 *	\param[in]	usTop   : タイマーに PWM 周期を設定する数値
 *	\param[in]	usData  : タイマーに 初期 デューティー比を設定する数値
 *	\param[in]	usCount : オーバーサンプリングの回数
 *	\date		CREATE : 17/Nov/2008 Y.Tamaki(Arex)
 *	\date		UPDATE : 2010/06/15 Iwasawa H.(Arex)
 *	\date		UPDATE : 2012/03/08 Iwasawa H.(Arex)
 **/ /*************************************************************************/
void sodiac_set_timer(unsigned short usTop, unsigned short usData, unsigned char usCount)
{
	/* DMAの設定 */
	DMAC0.DMCNT.BIT.DTE = 0;

	DMAC0.DMDAR = (unsigned long)&TPU0.TGRD; /* 音声データ転送先レジスタ */
	DMAC0.DMSAR = (unsigned long)GSODIAC_usaData; /* 音声バッファ */

	DMAC0.DMCRA = usCount;

	DMAC0.DMTMD.BIT.DCTG  = 1;	/* ペリフェラルから起動する */
	DMAC0.DMTMD.BIT.SZ    = 1;	/* 16bit転送 */
	DMAC0.DMTMD.BIT.DTS   = 2;	/* リピートブロック無 */
	DMAC0.DMTMD.BIT.MD    = 0;	/* ノーマルモード */

	DMAC0.DMINT.BIT.DTIE  = 1;	/* DMA終了割り込み許可 */

	DMAC0.DMAMD.BIT.SM    = 2;	/* 転送元インクリメント */
	DMAC0.DMAMD.BIT.SARA  = 0;
	DMAC0.DMAMD.BIT.DM    = 0;	/* 転送先固定 */
	DMAC0.DMAMD.BIT.DARA  = 0;

	DMAC0.DMCSL.BIT.DISEL = 0;	/* タイマー割り込みは行わない */

	DMAC0.DMCNT.BIT.DTE   = 1;	/* DMA0許可 */

	DMAC.DMAST.BIT.DMST   = 1;	/* (注)全体のシステムで設定してください 全DMA有効 */

	ICU.DMRSR0.BIT.DMRS   = VECT_TPU0_TGI0A;	/* TPU0A割り込みを起動要因に */
	IPR(DMAC,DMAC0I)=15;		/* 最高レベル */
	IEN(DMAC,DMAC0I)=1;			/* 割り込み有効 */

	/* 以下はPWMポートの設定 */
	/* 設定するポートを汎用入力に */
	PORT3.PDR.BIT.B3 = 0;
	PORT3.PMR.BIT.B3 = 0;

	MPC.PWPR.BIT.B0WI  = 0;		/* PFSWEの書き換え許可 */
	MPC.PWPR.BIT.PFSWE = 1;		/* PFSの書き換え許可 */
	MPC.PWPR.BIT.B0WI  = 1;		/* PFSWEの書き換え禁止 */

	/* 機能選択 */
	MPC.P33PFS.BIT.PSEL = 0x03;	/* TIOCD0 */

	MPC.PWPR.BIT.B0WI  = 0;		/* PFSWEの書き換え許可 */
	MPC.PWPR.BIT.PFSWE = 0;		/* PFSの書き換え禁止 */
	MPC.PWPR.BIT.B0WI  = 1;		/* PFSWEの書き換え禁止 */

	/* 機能実行 */
	PORT3.PMR.BIT.B3 = 1;		/* TIOCD0 */

	/* 以下はタイマー設定 */
	MSTP_TPU0 = 0;				/* タイマー起動 */
	TPU0.TCR.BIT.TPSC = 0;		/* クロック選択 PΦ1 */
	TPU0.TCR.BIT.CKEG = 1;		/* 立ち上がりエッジトリガ */
	TPU0.TCR.BIT.CCLR = 1;		/* クリア要因 TGRAコンペアマッチ */

	TPU0.TIORL.BIT.IOD = 5;		/* 初期値1 コンペアで0 */

	TPU0.TMDR.BIT.BFA = 0;		/* TGRCはノーマル */
	TPU0.TMDR.BIT.BFB = 0;		/* TGRDはノーマル */
	/* PWM周期設定  */
	TPU0.TGRA = usTop;
	/* 初期データ設定 */
	TPU0.TGRD = usData;

	TPU0.TMDR.BIT.MD = 3;		/* PWMモード2 */
}

/*************************************************************************/ /**
 *	PWMタイマー再設定関数
 *
 *	\param[in]	usTop   : タイマーに PWM 周期を設定する数値
 *	\param[in]	usData  : タイマーに 初期 デューティー比を設定する数値
 *	\param[in]	usCount : オーバーサンプリングの回数
 *	\date		CREATE : 2010/09/14
 *	\date		UPDATE : 2011/05/14 Iwasawa H.(Arex)
 *	\date		UPDATE : 2012/03/08 Iwasawa H.(Arex)
 **/ /*************************************************************************/
void sodiac_recycle_timer(unsigned short usTop, unsigned short usData, unsigned char usCount)
{
	TPU0.TGRA = usTop;
	TPU0.TGRD = usData;
}

/*************************************************************************/ /**
 *			All rights reserved, Copyright (C) Arex.Co.Ltd, 2008-2012		  *
 **/ /*************************************************************************/
