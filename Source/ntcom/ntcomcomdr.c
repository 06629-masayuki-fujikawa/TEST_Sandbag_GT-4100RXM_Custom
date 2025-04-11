// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
// MH364300 GG119A03 (S) NTNET通信制御対応（標準UT4000：MH341111流用）
		// 本ファイルを追加			nt task(NT-NET通信処理部)
// MH364300 GG119A03 (E) NTNET通信制御対応（標準UT4000：MH341111流用）
/*[]----------------------------------------------------------------------[]
 *|	filename: NTComComdr.c
 *[]----------------------------------------------------------------------[]
 *| summary	: SCIドライバ(1パケット抽出機能)
 *| author	: machida.k
 *| date	: 2005-06-10
 *| update	:
 *|				2005-12-20			machida.k	RAM共有化
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*			I N C L U D E													*/
/*--------------------------------------------------------------------------*/
#include	<string.h>
#include	<stdlib.h>
#include	"system.h"
#include	"common.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"ntnet.h"
#include	"ntcom.h"
#include	"ntcomdef.h"



/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/
static	void	NTComComdr_PortReset(void);
static	void	NTComComdr_Reset(void);

static	void	int_scircv(uchar rcvdata);
static	void	int_scierr(uchar errtype);
static	void	int_comdrtimer_c2c(void);
static	void	int_comdrtimer_sndcmp(void);

/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/

/*====================================================================================[PUBLIC]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComComdr_Init()
 *[]----------------------------------------------------------------------[]
 *| summary	: NTComComdrモジュール初期化
 *| param	: timeout_c2c - キャラクタ間タイムアウト値 [msec]
 *|			  speed (bps) 0=1200, 1=2400
 *|			  dbit  (Data bits length) 0=8bits, 1=7bits
 *|			  sbit  (Stop bits length) 0=1bit , 1=2bits
 *|			  pbit  (Parity bit kind ) 0=none , 1=odd  ,2=even
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComComdr_Init(ushort timeout_c2c, uchar port, uchar speed, uchar dbit, uchar sbit, uchar pbit)
{
	/* SCIインタフェース初期化(割り込み無効) */
	NTComSci_Init(int_scircv, int_scierr, port, speed, dbit, sbit, pbit);
	/* タイムアウト値保存 */
	NTComComdr_Ctrl.timer_c2c.timeout	= (timeout_c2c + 1) / NT_C2C_TIMINT;
	/* 制御データ初期化 */
	NTComComdr_Reset();
	/* タイマークリエイト＆スタート */
	NTComComdr_Ctrl.timer_c2c.timerid	= NTComTimer_Create(1, NT_C2C_TIMINT, int_comdrtimer_c2c, TRUE);
	NTComTimer_Start(NTComComdr_Ctrl.timer_c2c.timerid);
	NTComComdr_Ctrl.timer_sndcmp.timerid	= NTComTimer_Create(10, 10, int_comdrtimer_sndcmp, TRUE);
	NTComTimer_Start(NTComComdr_Ctrl.timer_sndcmp.timerid);
	/* SCI割り込み有効化 */
	NTComSci_Start();
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComComdr_Reset()
 *[]----------------------------------------------------------------------[]
 *| summary	: NTComComdrモジュールリセット
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComComdr_Reset(void)
{
	volatile unsigned long	ist;			// 現在の割込受付状態

	ist = _di2();
	NTComComdr_Ctrl.status			= NT_COMDR_NO_PKT;
	NTComComdr_Ctrl.r_rcvlen			= 0;
	NTComComdr_Ctrl.r_datalen			= 0;
	NTComComdr_Ctrl.timer_c2c.count	= -1;
	NTComComdr_Ctrl.timer_sndcmp.count	= 0;
	_ei2(ist);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComComdr_GetRcvPacket()
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信パケット取得
 *| param	: buf - 受信パケットバッファ	<OUT>
 *|					(戻り値がNTCOM_SCI_PKT_RCVDのとき有効)
 *| return	: COMDRステータス
 *[]----------------------------------------------------------------------[]*/
eNT_COMDR_STS	NTComComdr_GetRcvPacket(T_NT_BLKDATA *buf)
{
	eNT_COMDR_STS	ret;

	ret = (eNT_COMDR_STS)NTComComdr_Ctrl.status;

	switch (ret) {
	case NT_COMDR_NO_PKT:					/* アイドル */
		break;
	case NT_COMDR_PKT_RCVING:				/* パケット受信中 */
		if (NTComComdr_Ctrl.timer_c2c.count >= NTComComdr_Ctrl.timer_c2c.timeout) {
			/* キャラクタ間タイムアウト発生 */
			ret = NT_COMDR_ERR_TIMEOUT;
			NTComComdr_Reset();
		}
		break;
	case NT_COMDR_PKT_RCVD:					/* パケット受信完了 */
		memcpy(buf->data, &NTComComdr_Ctrl.r_buf, NTComComdr_Ctrl.r_rcvlen);
		buf->len = NTComComdr_Ctrl.r_rcvlen;
		NTComComdr_Reset();
		break;
	default:								/* 受信エラー発生 */
		NTComComdr_Reset();
		break;
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComComdr_SendPacket()
 *[]----------------------------------------------------------------------[]
 *| summary	: パケット送信(送信完了かタイムアウトまでリターンしない))
 *| param	: buf - 送信パケット	<IN>
 *|			  wait - 送信開始待ち時間				[msec]
 *|			  timeout - 送信完了待ちタイムアウト	[msec]
 *| return	: 送信結果
 *[]----------------------------------------------------------------------[]*/
eNT_COMDR_SND	NTComComdr_SendPacket(T_NT_BLKDATA *buf, int wait, int timeout)
{
	/* 送信開始待ち */
	xPause1ms(wait);

	/* ドライバに送信を要求 */
	if (!NTComSci_SndReq(buf->data, buf->len)) {
		NTComComdr_PortReset();
		return NT_COMDR_SND_ERR;			/* 送信不可 */
	}
	/* タイムアウト設定 */
	NTComComdr_Ctrl.timer_sndcmp.count	= _To10msVal(timeout);

	/* 送信完了待ち */
	while (NTComComdr_Ctrl.timer_sndcmp.count > 0) {
		taskchg( IDLETSKNO );
		NTCom_TimoutChk();// タイマーチェック処理
		if (NTComSci_IsSndCmp()){
			NTComComdr_Ctrl.timer_sndcmp.count = 0;
			return NT_COMDR_SND_NORMAL;		/* 送信完了 */
		}
	}
	NTComComdr_PortReset();

	return NT_COMDR_SND_TIMEOUT;			/* タイムアウト */
}

/*====================================================================================[PRIVATE]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComComdr_PortReset()
 *[]----------------------------------------------------------------------[]
 *| summary	: SCIポートリセット
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComComdr_PortReset(void)
{
	/* SCI割り込み無効化 */
	NTComSci_Stop();
	/* COMDR制御データ初期化 */
	NTComComdr_Reset();
	/* SCIポートリセット＆割り込み有効化 */
	NTComSci_Start();
}

/*====================================================================================[INTERRUPTS]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: int_scircv()
 *[]----------------------------------------------------------------------[]
 *| summary	: 1バイト受信割り込みハンドラ
 *| param	: rcvdata - 受信した1バイト
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	int_scircv(uchar rcvdata)
{
	/* キャラクタ間タイムアウト監視開始 */
	NTComComdr_Ctrl.timer_c2c.count = 0;

	if (NTComComdr_Ctrl.status == NT_COMDR_NO_PKT) {
		NTComComdr_Ctrl.status = NT_COMDR_PKT_RCVING;
	}

	if (NTComComdr_Ctrl.status == NT_COMDR_PKT_RCVING) {

		if (NTComComdr_Ctrl.r_rcvlen < 7) {
		/* パケット先頭7バイト(5バイトのシグネチャ＋2バイトのデータ長)受信フェーズ */
			if (NTComComdr_Ctrl.r_rcvlen < 5) {
			/* 電文先頭のシグネチャチェック */
				if (rcvdata != NT_PKT_SIG[NTComComdr_Ctrl.r_rcvlen]) {
					/* 受信パケットを1から検出しなおす */
					if (rcvdata == NT_PKT_SIG[0]) {
						/* パケットの先頭データかもしれないので */
						NTComComdr_Ctrl.r_buf[0] = rcvdata;
						NTComComdr_Ctrl.r_rcvlen = 1;
					}
					else {
						NTComComdr_Ctrl.r_rcvlen = 0;
					}
					return;
				}
			}
			NTComComdr_Ctrl.r_buf[NTComComdr_Ctrl.r_rcvlen++] = rcvdata;
			if (NTComComdr_Ctrl.r_rcvlen >= 7) {
			/* データ長取得 */
				NTComComdr_Ctrl.r_datalen = NT_MakeWord(&NTComComdr_Ctrl.r_buf[5]);
				if (NTComComdr_Ctrl.r_datalen >= NT_BLKSIZE - NT_PKT_CRC_SIZE) {
					NTComComdr_Ctrl.status = NT_COMDR_ERR_INVALID_LEN;
				}
				/* パケットデータ&CRCデータ受信フェーズへ */
			}
		}
		else {
		/* パケットデータ&CRCデータ受信フェーズ */
			NTComComdr_Ctrl.r_buf[NTComComdr_Ctrl.r_rcvlen++] = rcvdata;
			if (NTComComdr_Ctrl.r_rcvlen >= NTComComdr_Ctrl.r_datalen + NT_PKT_CRC_SIZE) {
				/* 1パケット受信完了 */
				NTComComdr_Ctrl.timer_c2c.count = -1;
				NTComComdr_Ctrl.status = NT_COMDR_PKT_RCVD;
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: int_scierr()
 *[]----------------------------------------------------------------------[]
 *| summary	: 通信エラー割り込みハンドラ
 *| param	: errtype - 発生した通信エラー
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	int_scierr(uchar errtype)
{
	switch (errtype) {
	case NT_SCI_ERR_OVERRUN:
		NTCom_ComErr.OVRcnt++;
		break;
	case NT_SCI_ERR_FRAME:
		NTCom_ComErr.FRMcnt++;
		break;
	case NT_SCI_ERR_PARITY:
		NTCom_ComErr.PRYcnt++;
	default:
		break;
	}
	NTComComdr_Ctrl.status = NT_COMDR_ERR_SCI;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: int_comdrtimer_c2c()
 *[]----------------------------------------------------------------------[]
 *| summary	: キャラクター間タイムアウト監視用タイマー割り込みハンドラ
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	int_comdrtimer_c2c(void)
{
	if (NTComComdr_Ctrl.timer_c2c.count >= 0) {
		if (NTComComdr_Ctrl.timer_c2c.count < 0x7fffffff) {
			NTComComdr_Ctrl.timer_c2c.count++;
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: int_comdrtimer_sndcmp()
 *[]----------------------------------------------------------------------[]
 *| summary	: 100msタイマー割り込みハンドラ
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	int_comdrtimer_sndcmp(void)
{
	if (NTComComdr_Ctrl.timer_sndcmp.count > 0) {
		NTComComdr_Ctrl.timer_sndcmp.count--;
	}
}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
