// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
// MH364300 GG119A03 (S) NTNET通信制御対応（標準UT4000：MH341111流用）
		// 本ファイルを追加			nt task(NT-NET通信処理部)
// MH364300 GG119A03 (E) NTNET通信制御対応（標準UT4000：MH341111流用）
/*[]----------------------------------------------------------------------[]
 *|	filename: ntcomsci.c
 *[]----------------------------------------------------------------------[]
 *| summary	: SCIドライバ
 *| author	: machida.k
 *| date	: 2005.05.31
 *| update	:
 *|		2005-12-20			machida.k	RAM共有化
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*			I N C L U D E													*/
/*--------------------------------------------------------------------------*/
#include	<string.h>
#include	<stdlib.h>
#include	"iodefine.h"
#include	"system.h"
#include	"common.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"ntnet.h"
#include	"ntcom.h"
#include	"ntcomdef.h"


/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S												*/
/*--------------------------------------------------------------------------*/
#define	NTCom_Enable_RS485Driver(bEnable)	(CP_NT_RTS=!bEnable)

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/


/*====================================================================================[PUBLIC]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComSci_Init()
 *[]----------------------------------------------------------------------[]
 *| summary	: ポート初期化
 *|			  (ポートの動作を開始させるには別途NTComSci_Start()のコールが必要)
 *| param	: RcvFunc - データ受信通知用のコールバック関数ポインタ
 *|			  StsFunc - データ受信エラー通知用コールバック関数ポインタ
 *|			  Speed (bps) 0=1200, 1=2400
 *|			  Dbits (Data bits length) 0=8bits, 1=7bits
 *|			  Sbits (Stop bits length) 0=1bit , 1=2bits
 *|			  Pbit  (Parity bit kind ) 0=none , 1=odd  ,2=even
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComSci_Init(T_NT_SCI_CALLBACK RcvFunc, T_NT_SCI_CALLBACK StsFunc,
				uchar Port, uchar Speed, uchar Dbits, uchar Sbits, uchar Pbit)
{

	/* 対象ポートのSCI動作停止 */
	NTComSci_Stop();

	NTCom_Enable_RS485Driver(0);				// (送信RS485ドライバ)トランスミッタを出力禁止

	/* パラメータ保存 */
	NTComSci_Ctrl.port_no = Port;
	NTComSci_Ctrl.speed	= Speed;
	NTComSci_Ctrl.databit	= Dbits;
	NTComSci_Ctrl.stopbit	= Sbits;
	NTComSci_Ctrl.parity	= Pbit;
	NTComSci_Ctrl.rcvfunc = RcvFunc;
	NTComSci_Ctrl.stsfunc = StsFunc;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComSci_Start()
 *[]----------------------------------------------------------------------[]
 *| summary	: ポート動作開始
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComSci_Start(void)
{
	volatile unsigned char	i;
	uchar	ists;

	ists = _di2();												/* disable interrupt */

	/* コントロールデータの初期化 */
	NTComSci_Ctrl.slen		= 0;
	NTComSci_Ctrl.scomplete	= TRUE;

	//  SCI initial
	MSTP_SCI9 = 0;							// SCI9 module stop mode release(SYSTEM.MSTPCRB.BIT.MSTPB27)
	SCI9.SEMR.BIT.ABCS = 0x01;			/* 調歩同期基本クロックセレクトBIT	*/
	SCI9.SCR.BYTE = 0x00;				// シリアル コントロール レジスタ 初期化クリア(割り込み禁止)

	_ei2(ists);												/* enable interrupt */

	SCI9.SMR.BIT.CM = 0;					// Async
	SCI9.SMR.BIT.MP = 0;					// Multi pro func : 0=Nouse, 1=Use

	/* データビット長 */
    if( NTComSci_Ctrl.databit == 1 ) {
        SCI9.SMR.BIT.CHR  = 1;    /* 7 bits */
    }
    else {
        SCI9.SMR.BIT.CHR  = 0;    /* 8 bits */
    }

    /* ストップビット長 */
    if( NTComSci_Ctrl.stopbit == 1 ) {
        SCI9.SMR.BIT.STOP = 1;    /* 2 bits */
    }
    else {
        SCI9.SMR.BIT.STOP = 0;    /* 1 bit */
    }

    /* パリティ */
    if( NTComSci_Ctrl.parity == 0 ) {
        SCI9.SMR.BIT.PE = 0;			// パリティなし
    }
    else {
        SCI9.SMR.BIT.PE = 1;			// パリティ有り
        if( NTComSci_Ctrl.parity == 1 ) {
            SCI9.SMR.BIT.PM = 1;    // 奇数
        }
        else {
            SCI9.SMR.BIT.PM = 0;    // 偶数
        }
    }

	/* 伝送速度 */
	switch( NTComSci_Ctrl.speed ){
	case 0:		/* 38400 bps */
		/* no break */
	default:
		SCI9.SMR.BIT.CKS = 0;
		SCI9.BRR = SCI_38400BPS_n0;
		break;
	case 1:		/* 19200 bps */
		SCI9.SMR.BIT.CKS = 0;
		SCI9.BRR = SCI_19200BPS_n1;
		break;
	case 2:		/* 9600 bps */
		SCI9.SMR.BIT.CKS = 0;
		SCI9.BRR = SCI_9600BPS_n1;
		break;
	case 3:		/* 4800 bps */
		SCI9.SMR.BIT.CKS = 0;
		SCI9.BRR = SCI_4800BPS_n1;
		break;
	}

	wait2us( 125L );						// 1bit time over wait
	i = SCI9.SSR.BYTE;					// Dummy Read
	SCI9.SSR.BYTE = 0xC4;				// Status register <- 0
	dummy_Read = SCI9.SSR.BYTE;			// Dummy Read
	SCI9.SCR.BYTE = 0x70;				// RIE&TE&RE Enable(RIE=1&送受信許可)

}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComSci_SndReq()
 *[]----------------------------------------------------------------------[]
 *| summary	: シリアル送信を開始する
 *|			  送信完了はNTComSci_IsSndCmp()で取得できる
 *| param	: buf - 送信データ
 *|			  len - 送信データ長
 *| return	: TRUE as success, FALSE as 送信ビジー
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComSci_SndReq(uchar *buf, ushort len)
{
	SCI9.SCR.BIT.TE = 1;
	if (SCI9.SSR.BIT.TEND) {
		/* 送信可能 */
		NTCom_Enable_RS485Driver(1);				/* (送信RS485ドライバ)トランスミッタを出力許可 */

		NTComSci_Ctrl.scomplete			= FALSE;	/* 送信完了フラグ初期化 */
		NTComSci_Ctrl.sdata				= buf;		/* 送信データ */
		NTComSci_Ctrl.scnt				= 0;		/* 送信完了データ数 */
		NTComSci_Ctrl.slen				= len;		/* 送信データ長 */

		SCI9.TDR			= buf[0];				/* 送信データ1byte目をセット */
		SCI9.SSR.BIT.TEND	= 0;
		dummy_Read = SCI9.SSR.BYTE;					// Dummy Read
		SCI9.SCR.BIT.TIE	= 1;					/* 送信割り込みイネーブル */

		return TRUE;
	}
	else {
		/* 送信不可 */
		SCI9.SCR.BIT.TEIE	= 0;
		SCI9.SCR.BIT.TIE	= 0;					/* 送信割り込みディセーブル */
		dummy_Read = SCI9.SSR.BYTE;					// Dummy Read
		NTComSci_Ctrl.scomplete	= TRUE;				/* 強制的に送信完了とする */

		return FALSE;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComSci_IsSndCmp()
 *[]----------------------------------------------------------------------[]
 *| summary	: シリアル送信完了を取得する
 *| param	: none
 *| return	: TRUE as 送信完了
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComSci_IsSndCmp(void)
{
	return NTComSci_Ctrl.scomplete;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComSci_Stop()
 *[]----------------------------------------------------------------------[]
 *| summary	: シリアル送受信停止
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComSci_Stop(void)
{
	_di();
	NTCom_Enable_RS485Driver(0);	/* (送信RS485ドライバ)トランスミッタを出力禁止 */

	SCI9.SCR.BIT.TIE  = 0;			/* 送信データエンプティ(TXI)割り込み禁止 */
	SCI9.SCR.BIT.RIE  = 0;			/* 受信割込み禁止 */
	SCI9.SCR.BIT.TE   = 0;			/* 送信動作禁止 */
	SCI9.SCR.BIT.RE   = 0;			/* 受信動作禁止 */
	SCI9.SCR.BIT.TEIE = 0;			/* 送信終了(TEI)割り込み禁止 */
	_ei();
}


/*==================================================================================[ISR (RX)]====*/


/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_Int_RXI()
 *[]----------------------------------------------------------------------[]
 *| summary	: １バイト受信完了割り込み処理
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTNET_Int_RXI(void)
{
	if((SCI9.SSR.BIT.ORER==0) && (SCI9.SSR.BIT.FER==0) && (SCI9.SSR.BIT.PER==0)) {
		NTComSci_Ctrl.rcvfunc(SCI9.RDR);
	}

	/* データ受信フラグクリア */
	SCI9.SSR.BYTE = 0;
}

/*==============================================================================[ISR (RX ERR)]====*/


/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_Int_ERI()
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信エラー割り込み処理
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTNET_Int_ERI(void)
{
    /* オーバーランエラー */
    if (SCI9.SSR.BIT.ORER) {
        NTComSci_Ctrl.stsfunc(NT_SCI_ERR_OVERRUN);
    }

    /* フレーミングエラー */
    if (SCI9.SSR.BIT.FER) {
        NTComSci_Ctrl.stsfunc(NT_SCI_ERR_FRAME);
    }

    /* パリティエラー */
    if (SCI9.SSR.BIT.PER) {
        NTComSci_Ctrl.stsfunc(NT_SCI_ERR_PARITY);
    }

    /* エラーをクリア */
    SCI9.SSR.BYTE &= 0xC7;
    dummy_Read = SCI9.SSR.BYTE;			// Dummy Read
}

/*==================================================================================[ISR (TX)]====*/


/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_Int_TXI()
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信データエンプティ割り込み処理
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTNET_Int_TXI(void)
{
	if (++NTComSci_Ctrl.scnt < NTComSci_Ctrl.slen) {
	/* 送信データ有り */
		SCI9.TDR			= NTComSci_Ctrl.sdata[NTComSci_Ctrl.scnt];
		SCI9.SSR.BIT.ORER= 0;							/* send start					*/
		SCI9.SSR.BIT.FER= 0;								/* 								*/
		SCI9.SSR.BIT.PER= 0;								/* 								*/
		dummy_Read = SCI9.SSR.BYTE;						// Dummy Read
	}
	else {
	/* 全データ送信完了 */
		SCI9.SCR.BIT.TIE	= 0;		/* 割り込みディセーブル */
		SCI9.SCR.BIT.TEIE = 1;				// TEND割り込み許可
	}
}

/*==============================================================================[ISR (TX END)]====*/


/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_Int_TEI()
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信終了割り込み処理
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTNET_Int_TEI(void)
{
	NTCom_Enable_RS485Driver(0);				/* (送信RS485ドライバ)トランスミッタを出力禁止 */

	SCI9.SCR.BIT.TEIE = 0;						/* 割り込みディセーブル */
	SCI9.SCR.BIT.TE = 0;
	NTComSci_Ctrl.scomplete			= TRUE;		/* 送信完了フラグON */
}

// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

