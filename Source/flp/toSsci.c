/*[]----------------------------------------------------------------------[]*/
/*|		対子機シリアル通信用ドライバ									   |*/
/*|		・SCI CH-4 を使用します。										   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"common.h"
#include	"IFM.h"
#include	"toS.h"
#include	"toSdata.h"

/*----------------------------------*/
/*			value define			*/
/*----------------------------------*/
#define	IFMCOM_ERROR_COUNT_LIMIT	(ushort)9999	/* server no answer */


/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
#define	toS_Enable_RS485Driver(bEnable)	(CP_IF_RTS=!bEnable)

/***************************/
/***   receive process	 ***/
/***************************/
/*[]----------------------------------------------------------------------[]*/
/*|  [API]      toSsci_Init                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*|		対親機通信ポート初期化                                             |*/
/*|		・SCI ch-4の初期化                                                 |*/
/*|		・送受信動作  →許可                                               |*/
/*|		・受信割り込み→許可                                               |*/
/*|		・送信割り込み→禁止（その都度、許可/禁止を制御する）              |*/
/*[]----------------------------------------------------------------------[]*/
/*| input  : Speed (伝送速度)         4=2400,  2=9600, 1=19200, 0=38400bps |*/
/*|			 Dbits (データビット長)   0=8bits, 1=7bits                     |*/
/*|          Sbits (ストップビット長) 0=1bit , 1=2bits                     |*/
/*|          Pbit  (垂直パリティ)     0=無し , 1=奇数, 2=偶数              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  A.Iiizumi                                               |*/
/*| Date        :  2011-11-21                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toSsci_Init(unsigned char Speed,
					unsigned char Dbits,
					unsigned char Sbits,
					unsigned char Pbit)
{

	volatile unsigned char	i;
	unsigned long	ists;
	
	toS_Enable_RS485Driver(0);				// (送信RS485ドライバ)トランスミッタを出力禁止

	// 受信管理情報を初期化
	toS_RcvCtrl.RcvCnt      = 0;
	toS_RcvCtrl.OvfCount    = 0;
	toS_RcvCtrl.ComerrStatus = 0;

	// 送信管理情報を初期化
	toS_SndCtrl.SndReqCnt = 0;
	toS_SndCtrl.SndCmpCnt = 0;

	// 受信エラー管理情報を初期化
	toS_RcvErr.usOVRcnt = 0;				// オーバーランエラー発生回数をクリア
	toS_RcvErr.usFRMcnt = 0;				// フレーミングエラー発生回数をクリア
	toS_RcvErr.usPRYcnt = 0;				//     パリティエラー発生回数をクリア

	ists = _di2();							// 割り込み禁止

	//  SCI initial 
	MSTP_SCI4 = 0;							// SCI4 module stop mode release(SYSTEM.MSTPCRB.BIT.MSTPB27)
	P_SCI_L.SEMR.BIT.ABCS = 0x01;			/* 調歩同期基本クロックセレクトBIT	*/
	P_SCI_L.SCR.BYTE = 0x00;				// シリアル コントロール レジスタ 初期化クリア(割り込み禁止)

	_ei2( ists );
	
	P_SCI_L.SMR.BIT.CM = 0;					// Async
	P_SCI_L.SMR.BIT.MP = 0;					// Multi pro func : 0=Nouse, 1=Use

	// データ長(0:8Bit, 1:7Bit) 
	if( Dbits == 1 ){
		P_SCI_L.SMR.BIT.CHR  = 1;			// 7 bits 
	}else{
		P_SCI_L.SMR.BIT.CHR  = 0;			// 8 bits 
	}

	// ストップビット長(0:1Bit, 1:2Bit) 
	if( Sbits == 1 ){
		P_SCI_L.SMR.BIT.STOP = 1;			// 2 bits
	}else{
		P_SCI_L.SMR.BIT.STOP = 0;			// 1 bit
	}

	// パリティ(0:NONE, 1:Odd, 2:Even) 
	switch (Pbit) {
		case 0:
		default:
			P_SCI_L.SMR.BIT.PE = 0;			// パリティなし
			break;
		case 1:
			P_SCI_L.SMR.BIT.PE = 1;			// パリティ有り
			P_SCI_L.SMR.BIT.PM = 1;			// 奇数
			break;
		case 2:
			P_SCI_L.SMR.BIT.PE = 1;			// パリティ有り
			P_SCI_L.SMR.BIT.PM = 0;			// 偶数
			break;
	}

	// 伝送速度(bps) 
	switch (Speed) {
		case 1:
			P_SCI_L.SMR.BIT.CKS = 1;		// Clock select : 0=分周なし, 1=分周/4
			P_SCI_L.BRR = SCI_19200BPS_n1;
			break;
		case 2:
			P_SCI_L.SMR.BIT.CKS = 1;		// Clock select : 0=分周なし, 1=分周/4
			P_SCI_L.BRR = SCI_9600BPS_n1;
			break;
		default:// 0
			P_SCI_L.SMR.BIT.CKS = 0;		// Clock select : 0=分周なし, 1=分周/4
			P_SCI_L.BRR = SCI_38400BPS_n0;
			break;
	}

	wait2us( 125L );						// 1bit time over wait
	i = P_SCI_L.SSR.BYTE;					// Dummy Read
	P_SCI_L.SSR.BYTE = 0xC4;				// Status register <- 0
	dummy_Read = P_SCI_L.SSR.BYTE;			// Dummy Read
	P_SCI_L.SCR.BYTE = 0x70;				// RIE&TE&RE Enable(RIE=1&送受信許可)
}

/*[]----------------------------------------------------------------------[]*/
/*|  [INT]      toS_Int_RXI2                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|     RXI割り込みハンドラ                                                |*/
/*|		・受信データレジスタ・フル割り込み                                 |*/
/*|		・SYS\API_cstm.c にある本物のハンドラ：x_Int_RXI2()から呼び出される|*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toS_Int_RXI2( void )
{
	unsigned char   rcvdat;

	if((P_SCI_L.SSR.BIT.ORER==0) && (P_SCI_L.SSR.BIT.FER==0) && (P_SCI_L.SSR.BIT.PER==0)) {
		rcvdat = P_SCI_L.RDR;
toS_Sci_Rcv_10:
		if (TOS_SCI_RCVBUF_SIZE > toS_RcvCtrl.RcvCnt) {
			toS_RcvBuf[toS_RcvCtrl.RcvCnt++] = rcvdat;		// 受信バッファへ転送後、受信データ数カウントアップ
		} else {											// 受信バッファはオーバーフロー
			toS_RcvBuf[0] = rcvdat;							// 受信データは、バッファの先頭へ
			toS_RcvCtrl.RcvCnt     = 1;						// 受信データ長もチャラ
			if (0xffff != toS_RcvCtrl.OvfCount) {
				++toS_RcvCtrl.OvfCount;
			}
		}
		toScom_2msTimerStart(toS_RcvCtrl.usBetweenChar); 	// 文字間監視タイマー開始（再開）
		P_SCI_L.SSR.BYTE = 0;								// 割り込み要因のクリア
		dummy_Read = P_SCI_L.SSR.BYTE;						// Dummy Read
	}
	else{													// RDRF=0, ORER=1, FER=0, PER=0
		rcvdat = 0xff;
		goto toS_Sci_Rcv_10;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  [INT]      toS_Int_ERI2                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|     受信エラー割り込みハンドラ                                         |*/
/*|		・SYS\API_cstm.c にある本物のハンドラ：x_Int_ERI2()から呼び出される|*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toS_Int_ERI2(void)
{
/*
*	オーバーランエラー：
*		・「シフトレジスタ」へ受信したデータを「データレジスタ」へ転送する前に、
*		  次の受信データが「シフトレジスタ」へ入ってきた。
*		・「データレジスタ」への転送後、受信データフル割り込みを発生させるので
*		  その割り込みハンドラがうまく機能していない場合に発生する。
*
*	フレーミングエラー：
*		・スタートビット＋データ長＋パリティ＋ストップビット＝フレーム、および
*		  ビットレートが、送信側と受信側で一致していない場合に発生する。
*
*	パリティエラー：
*		・SCIが行う水平パリティ検出にて異常があった(通信異常)や、
*		  パリティが、送信側と受信側で一致していない場合に発生する。
*/
	if( P_SCI_L.SSR.BIT.ORER ) {
		toS_RcvCtrl.ComerrStatus |= 0x01;	/* over run set	*/
		if( toS_RcvErr.usOVRcnt < IFMCOM_ERROR_COUNT_LIMIT ){
			toS_RcvErr.usOVRcnt++;			/* over run counter inc */
		}
	}
	if( P_SCI_L.SSR.BIT.FER ) {
		toS_RcvCtrl.ComerrStatus |= 0x02;	/* frame error set		*/
		if( toS_RcvErr.usFRMcnt < IFMCOM_ERROR_COUNT_LIMIT ){
			toS_RcvErr.usFRMcnt++;			/* frame error counter inc */
		}
	}
	if( P_SCI_L.SSR.BIT.PER ) {
		toS_RcvCtrl.ComerrStatus |= 0x04;	/* parity error set		*/
		if( toS_RcvErr.usPRYcnt < IFMCOM_ERROR_COUNT_LIMIT ){
			toS_RcvErr.usPRYcnt++;			/* parity error counter inc */
		}
	}

	/*（全部まとめて）エラーフラグクリア */
	P_SCI_L.SSR.BYTE &= 0xc7;
	dummy_Read = P_SCI_L.SSR.BYTE;			// Dummy Read
}


/***************************/
/***	send process	 ***/
/***************************/
/*[]----------------------------------------------------------------------[]*/
/*|  [API]      toSsci_SndReq                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*|		送信開始                                                           |*/
/*|		・送信したいデータは、toM_SndBuf[]へ既にセットされている前提です。 |*/
/*[]----------------------------------------------------------------------[]*/
/*| 引数 :	 送信したいデータ長が渡されます。							   |*/
/*| 戻り値 : 送信開始結果												   |*/
/*|				1=正常終了（送信開始できた）                               |*/
/*|				0=異常終了（送信開始できなかった）                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :  2005-05-12   早瀬・改良                                 |*/
/*| 				・戻り値をBoolean型にする                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	toSsci_SndReq( unsigned short Length )
{
	/*「０バイト送れ！」で要求されたときのセーフティ機能 */
	if (Length == 0) {
		toS_SndCtrl.SndCmpFlg = 1;				/* 送信していないけど送信完了扱い(完了フラグ←ON) */
		return (uchar)1;						/* 正常終了（送信開始できた）を返す */
	}

	P_SCI_L.SCR.BIT.TE = 1;
	/* 送信できる？（送信データレジスタ・エンプティ？）*/
	if( 0 != P_SCI_L.SSR.BIT.TEND ){
		toS_Enable_RS485Driver(1);				/* (送信RS485ドライバ)トランスミッタを出力許可 */

		toS_SndCtrl.SndCmpFlg = 0;				/* 送信開始(完了フラグ←OFF) */
		toS_SndCtrl.SndReqCnt = Length;			/* 送信要求データ数をセット */
		toS_SndCtrl.SndCmpCnt = 0;				/* 送信完了データ数をリセット */
		toS_SndCtrl.ReadIndex = 1;				/* 最初の１文字転送をこれからやるので */

		P_SCI_L.TDR = toS_SndBuf[0];			// 最初の１文字を送信バッファから転送
		dummy_Read = P_SCI_L.SSR.BYTE;			// Dummy Read
		P_SCI_L.SCR.BIT.TIE = 1;				// TXI割り込み許可
		return (uchar)1;						/* 正常終了（送信開始できた）を返す */
	}
	/* 送信できない（送信データレジスタが空ではない）*/
	else {
		toS_SndCtrl.SndCmpFlg = 1;				/* 送信できないけど送信完了扱い(完了フラグ←ON) */
		P_SCI_L.SCR.BIT.TEIE = 0;				// TEND割り込み禁止 
		P_SCI_L.SCR.BIT.TIE  = 0;				//  TXI割り込み禁止 
		dummy_Read = P_SCI_L.SSR.BYTE;			// Dummy Read
		return (uchar)0;                        /* 異常終了（送信開始できなかった）を返す */
	}
	return (uchar)1;
}


/*[]----------------------------------------------------------------------[]*/
/*|  [INT]      toS_Int_TXI2                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		送信（送信データレジスタ空）割り込みハンドラ                       |*/
/*|		・SYS\API_cstm.c にある本物のハンドラ：x_Int_TXI2()から呼び出される|*/
/*|		・送信データレジスタへ転送可能となったとき、呼び出されます。       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toS_Int_TXI2( void )
{
	++toS_SndCtrl.SndCmpCnt;					/* 送信完了データ数カウント（アップ）*/

	/* まだ送信データが残っている？*/
	if (toS_SndCtrl.SndReqCnt > toS_SndCtrl.SndCmpCnt) {
		P_SCI_L.TDR = toS_SndBuf[ toS_SndCtrl.ReadIndex++];	// 送信バッファから転送 
		P_SCI_L.SSR.BIT.ORER= 0;							/* send start					*/
		P_SCI_L.SSR.BIT.FER= 0;								/* 								*/
		P_SCI_L.SSR.BIT.PER= 0;								/* 								*/
		dummy_Read = P_SCI_L.SSR.BYTE;						// Dummy Read
	}
	/* もう全て送信データを転送した。けど出し切った訳じゃない */
	else {
		P_SCI_L.SCR.BIT.TIE  = 0;				//（自身の）TXI割り込み禁止 
		P_SCI_L.SCR.BIT.TEIE = 1;				// TEND割り込み許可 

/* →許可することで出し切った判定は次の「TEND割り込み」に任せる */
	}
}

/*[]----------------------------------------------------------------------[]*/
/*|  [INT]      toS_Int_TEI2                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|		送信終了（TEND=トランスミット・エンド）割り込みハンドラ            |*/
/*|		・シフトレジスタから外部へ送信し切ったとき、呼び出されます。       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toS_Int_TEI2( void )
{
	/* ここに来たら「全て送信データを出し切った」ということ */
	toS_Enable_RS485Driver(0);				/* (送信RS485ドライバ)トランスミッタを出力禁止 */
	P_SCI_L.SCR.BIT.TEIE = 0;				//（自身の）TEND割り込み禁止
	P_SCI_L.SCR.BIT.TE = 0;
	toS_SndCtrl.SndCmpFlg = 1;				/* 送信完了した(完了フラグ←ON) */
}


/*[]----------------------------------------------------------------------[]*/
/*|  [API]      toSsci_IsSndCmp                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*|		送信完了判定                                                       |*/
/*|		・送信完了したか？否かを返します。                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| 戻り値 : 送信完了結果（Boolean）									   |*/
/*|				1=送信完了												   |*/
/*|				0=送信未完了											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
unsigned char	toSsci_IsSndCmp( void )
{
	if (1 == toS_SndCtrl.SndCmpFlg) {
		return (unsigned char)1;
	}
	return (unsigned char)0;
}


/*[]----------------------------------------------------------------------[]*/
/*|  [INT]      toSsci_Stop		                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|         SCI send & receive stop			                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toSsci_Stop( void )
{
	_di();
	toS_Enable_RS485Driver(0);		/* (送信RS485ドライバ)トランスミッタを出力禁止 */
	P_SCI_L.SCR.BYTE = 0;			// 全ビットリセット
	dummy_Read = P_SCI_L.SSR.BYTE;	// Dummy Read
	_ei();
}

