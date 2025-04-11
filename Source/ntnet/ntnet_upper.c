/*[]----------------------------------------------------------------------[]
 *| System      : UT8500
 *| Module      : NT-NET通信モジュール(上位レイヤー)
 *[]----------------------------------------------------------------------[]
 *| Author      : machida kei
 *| Date        : 2005.08.23
 *| Update      :
 *|		2005.09.01	machida.k	送信リトライが終了しない不具合の修正
 *|								送信リトライ回数を固定(設定パラメータは参照しない)とする
 *|								受信側はリトライ回数をチェックしない
 *|								IBKバッファFULL時における再送にWAITをかける
 *|		2006-01-17	machida.k	アイドル負荷軽減 (MH544401)
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*			I N C L U D E													*/
/*--------------------------------------------------------------------------*/

#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"prm_tbl.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnetauto.h"
#include	"mnt_def.h"

/*--------------------------------------------------------------------------*/
/*			C O N S T A N T S												*/
/*--------------------------------------------------------------------------*/

/* バッファコード */
enum {
	/* for send-> */
	NTUPR_SBUF_NORMAL,				/* 送信通常データ */
	NTUPR_SBUF_PRIOR,				/* 送信優先データ */
	/* <-for send */
	
	/* for receive-> */
	NTUPR_RBUF_NORMAL,				/* 受信通常データ */
	NTUPR_RBUF_PRIOR,				/* 受信優先データ */
	NTUPR_RBUF_BROADCAST,			/* 受信同報データ */
	/* <-for receive */
	
	NTUPR_BUF_MAX
};

/* 本レイヤーのステータス */
typedef enum {
	NTUPR_IDLE,						/* アイドル */
	NTUPR_SEND_FREE,				/* FREEデータ送信完了待ち */
	NTUPR_SEND_PRIOR,				/* 優先データ送信完了待ち */
	NTUPR_SEND_NORMAL				/* 通常データ送信完了待ち */
}eNTUPR_STS;

/* 復電処理用ステータス */
typedef enum {
	NTUPR_FUKUDEN_NONE,				/* 復電処理無し */
	NTUPR_FUKUDEN_DATAGET,			/* データ取得中 */
	NTUPR_FUKUDEN_DATACLR,			/* データ削除中 */
	NTUPR_FUKUDEN_HOLDING,			/* データ保持中 */
}eNTUPR_FUKUDEN_STS;

/* 送信リトライ回数 */
#define	NTUPR_SND_RETRY	3			/* #2 */

/*--------------------------------------------------------------------------*/
/*			S T R U C T U R E S												*/
/*--------------------------------------------------------------------------*/

/* 電文送受信用のバッファの実領域 */
typedef struct {
	uchar		snormal[26000];		/* 送信通常データ */
	uchar		sprior[1000];		/* 送信優先データ */
	uchar		rnormal[26000];		/* 受信通常データ */
	uchar		rprior[1000];		/* 受信優先データ */
	uchar		rbroadcast[1000];	/* 受信同報データ */
}t_NtUpr_TelePool;

/* NTUPR層用復電データ */
typedef struct {
	eNTUPR_FUKUDEN_STS	phase;		/* 復電処理ステータス */
	eNTNET_BUF_KIND		bufkind;	/* データ取得先バッファ */
	t_NtNet_ClrHandle	h;			/* データ削除用ハンドル */
}t_NtUpr_Fukuden;

/* NT-NET1電文管理 */
typedef struct {
	ushort				tele_len;			/* データ長 */
	uchar				*data;				/* データ本体 */
	ushort				blknum;				/* 送信/受信済みブロック数 */
	long				blkno_retry;		/* ブロックNo.異常による送信リトライ回数(送信バッファ時のみ使用) */
	long				buffull_retry;		/* IBKバッファFULLによる送信リトライ回数(送信バッファ時のみ使用) */
	ulong				buffull_timer;		/* IBKバッファFULLによる送信リトライ用タイマー(送信バッファ時のみ使用) */
	ushort				send_bytenum;		/* 送信済みデータ数(送信バッファ時のみ使用) */
	ushort				cur_send_bytenum;	/* 今回送信データ数(送信バッファ時のみ使用) */
	t_NtUpr_Fukuden		fukuden;			/* 復電処理用データ */
	ushort				UpperQueKind;		/* 送信ﾃﾞｰﾀ取得元ｷｭｰ種別（送信ﾊﾞｯﾌｧのみ）	*/
											/* "NTNET_SNDBUF_xxxx" の値を持つ。			*/
}t_NtUpr_Telegram;

/* FREEデータ送信制御 */
typedef struct {
	BOOL	isdata;					/* TRUE=データあり */
	uchar	data[FREEPKT_DATASIZE];	/* FREEデータ */
}t_NtUpr_FreeCtrl;

/* 制御データ構造 */
typedef struct {
	eNTUPR_STS			sts;		/* 本レイヤーのステータス */
	uchar				seqno;		/* 送信シーケンスNo. */
	uchar				rcv_seqno;	/* 受信シーケンスNo. */
	t_NtUpr_FreeCtrl	freedata;	/* FREEデータ送信制御データ */
}t_NtUpr_Ctrl;

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (backuped)									*/
/*--------------------------------------------------------------------------*/

#pragma section		_UNINIT2


/* 送受信バッファ実領域 */
t_NtUpr_TelePool			z_NtNetTelePool;

/* 送受信バッファ */
t_NtUpr_Telegram			z_NtUpr_Telegram[NTUPR_BUF_MAX];


#pragma section

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (not backuped)								*/
/*--------------------------------------------------------------------------*/

/* 送信制御データ */
static t_NtUpr_Ctrl		z_NtUpr_Ctrl;

/* 1ブロック送信用WORK */
static t_NtNetBlk		z_NtUpr_BlkWork;

char	z_NtRemoteAutoManual;
static	char	z_1minutePending;

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/

#define	_ntupr_BufSndReset(b)	(b)->blknum			= 0; \
								(b)->send_bytenum	= 0

#define	_ntupr_BufReset(b)	_ntupr_BufSndReset(b);	 \
							(b)->blkno_retry	= 0; \
							(b)->buffull_retry	= 0

#define	_ntupr_BufClr(b)	(b)->tele_len		= 0; \
							_ntupr_BufReset(b)

#define	_ntupr_NextSeqno(n)	(((n) >= 99) ? 1 : (n) + 1)

static void		ntupr_ChkSendData(t_NtUpr_Ctrl *ctrl, t_NtUpr_Telegram *telegram);
static void		ntupr_Send(t_NtUpr_Ctrl *ctrl, t_NtUpr_Telegram *telegram);
static eNTNET_RRSLT		ntupr_SetRcvNtBlk(t_NtUpr_Ctrl *ctrl, t_NtUpr_Telegram *telegram, uchar *data, ushort len);
static BOOL		ntupr_GetSndNtData(t_NtUpr_Ctrl *ctrl, t_NtUpr_Telegram *tele);
static void		ntupr_SetRcvFreeData(uchar *data);
static BOOL		ntupr_IsWaiting(t_NtUpr_Telegram *tele);
#define	_ntupr_SetRcvErrData(d)	NTBUF_SetRcvErrData(d)

static ushort	ntupr_Unpack(uchar *dst, t_NtNetBlk *ntblk, ushort blklen);
static ushort	ntupr_MkNtNetBlk(t_NtNetBlk *ntblk, t_NtUpr_Telegram *tele, uchar mode);
static int		ntupr_BufErrCode(eNTNET_BUF_KIND bufkind);

/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/

/*====================================================================================[PUBLIC]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: NTUPR_Init
 *[]----------------------------------------------------------------------[]
 *| summary	: 機能を初期化
 *| param	: clr - 1=停電保証データもクリア
 *|			        0=停電保証データはクリアしない
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTUPR_Init(uchar clr)
{
	int i;
	
	/* 制御データのクリア */
	z_NtUpr_Ctrl.sts				= NTUPR_IDLE;
	z_NtUpr_Ctrl.freedata.isdata	= FALSE;
	z_NtUpr_Ctrl.seqno				= 0;
	z_NtUpr_Ctrl.rcv_seqno			= 0;										/* 受信ｼｰｹﾝｽNo.ｸﾘｱ */
	
	z_1minutePending = (char)-1;

	/* バッファメモリ割り当て */
	z_NtUpr_Telegram[NTUPR_SBUF_NORMAL].data	= z_NtNetTelePool.snormal;		/* 送信通常データ */
	z_NtUpr_Telegram[NTUPR_SBUF_PRIOR].data		= z_NtNetTelePool.sprior;		/* 送信優先データ */
	z_NtUpr_Telegram[NTUPR_RBUF_NORMAL].data	= z_NtNetTelePool.rnormal;		/* 受信通常データ */
	z_NtUpr_Telegram[NTUPR_RBUF_PRIOR].data		= z_NtNetTelePool.rprior;		/* 受信優先データ */
	z_NtUpr_Telegram[NTUPR_RBUF_BROADCAST].data	= z_NtNetTelePool.rbroadcast;	/* 受信同報データ */
	
	/* 送受信バッファデータのクリア */
	for (i = 0; i < NTUPR_BUF_MAX; i++) {
		if (clr) {
			_ntupr_BufClr(&z_NtUpr_Telegram[i]);
			z_NtUpr_Telegram[i].fukuden.phase = NTUPR_FUKUDEN_NONE;
		}
		else {
			/* 復電処理 */
			if (z_NtUpr_Telegram[i].fukuden.phase != NTUPR_FUKUDEN_NONE) {
				ntupr_GetSndNtData(&z_NtUpr_Ctrl, &z_NtUpr_Telegram[i]);
			}
			/* バッファ初期化 */
			if (z_NtUpr_Telegram[i].tele_len > 0
				&& ((t_NtNetDataHeader*)z_NtUpr_Telegram[i].data)->data_save == NTNET_DO_BUFFERING) {
				/* データ保持フラグ＝ONのデータは停電保証対象 */
					_ntupr_BufReset(&z_NtUpr_Telegram[i]);
			}
			else {
				_ntupr_BufClr(&z_NtUpr_Telegram[i]);
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTUPR_SetRcvPkt
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信データセット
 *| param	: data - 受信データ
 *|			  len - 受信データ長
 *|			  kind - 受信データの種類
 *| return	: ブロックNo.チェック結果
 *[]----------------------------------------------------------------------[]*/
eNTNET_RRSLT	NTUPR_SetRcvPkt(uchar *data, ushort len, eNTNET_DATA_KIND kind)
{
	eNTNET_RRSLT	result;
	
	result = NTNET_RRSLT_NORMAL;
	switch (kind) {
	case NTNET_NTDATA:		/* NT-NETデータ受信 */
		result = ntupr_SetRcvNtBlk(&z_NtUpr_Ctrl, z_NtUpr_Telegram, data, len);
		break;
	case NTNET_FREEDATA:	/* FREEデータ受信 */
		ntupr_SetRcvFreeData(data);
		break;
	case NTNET_ERRDATA:		/* エラーデータ受信 */
		_ntupr_SetRcvErrData(data);
		break;
	}
	
	return result;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTUPR_ChkDispatch
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET通信モジュール 上位レイヤー ディスパッチチェック
 *| param	: none
 *| return	: TRUE = Dispatch要因あり
 *[]----------------------------------------------------------------------[]*/
BOOL	NTUPR_ChkDispatch(void)
{
	eNTNET_SRSLT	dummy;
	
	switch (z_NtUpr_Ctrl.sts) {
	case NTUPR_IDLE:				/* アイドル */
		if (z_NtUpr_Telegram[NTUPR_SBUF_NORMAL].tele_len > 0	/* 送信中の通常データあり */
			|| z_NtUpr_Telegram[NTUPR_SBUF_PRIOR].tele_len > 0	/* 送信中の優先データあり */
			|| _NTBUF_SndDataExist()) {							/* バッファモジュールに送信データあり */
			return TRUE;
		}
		break;
	case NTUPR_SEND_FREE:			/* FREEデータ送信完了待ち */
		/* no break */
	case NTUPR_SEND_PRIOR:			/* 優先データ送信完了待ち */
		/* no break */
	case NTUPR_SEND_NORMAL:			/* 通常データ送信完了待ち */
		if (NTLWR_IsSendComplete(&dummy)) {
			return TRUE;
		}
		break;
	default:
		/* このパスは通らないはず */
		break;
	}
	if ((int)z_1minutePending >= 0) {
		if (z_1minutePending != CLK_REC.minu) {
			// 1分経過→ペンディング解除
			NTBUF_SetIBKPendingByID(0xff);
			z_1minutePending = (char)-1;
			return TRUE;
		}
	}
	return FALSE;
}


/*[]----------------------------------------------------------------------[]
 *|	name	: NTUPR_Main
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET通信モジュール 上位レイヤーメイン
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTUPR_Main(void)
{
	/* 送信のスケジューリング＆実行 */
	ntupr_Send(&z_NtUpr_Ctrl, z_NtUpr_Telegram);
}

/*====================================================================================[PRIVATE]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_Send
 *[]----------------------------------------------------------------------[]
 *| summary	: NTNETデータ送信オペレーション
 *| param	: ctrl - 制御データ
 *|			  telegram - 本モジュールで管理する送受信バッファ
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntupr_Send(t_NtUpr_Ctrl *ctrl, t_NtUpr_Telegram *telegram)
{
	t_NtUpr_Telegram *tele;
	eNTUPR_STS		sts;
	eNTNET_SRSLT	result;
	ushort			len;
	
	switch (ctrl->sts) {
	case NTUPR_IDLE:		/*--------------------------------  IDLE ----  */
		/* 送信データの有無をチェック */
		ntupr_ChkSendData(ctrl, telegram);
		
		if (ctrl->freedata.isdata) {
			/* FREEデータ送信 */
			NTLWR_SendReq(ctrl->freedata.data, sizeof(ctrl->freedata.data), NTNET_FREEDATA);
			ctrl->sts = NTUPR_SEND_FREE;
		}
		else {
			len = 0;
			if (telegram[NTUPR_SBUF_PRIOR].tele_len > 0) {
				if (!ntupr_IsWaiting(&telegram[NTUPR_SBUF_PRIOR])) {
			/* 優先データ送信 */
					len = ntupr_MkNtNetBlk(&z_NtUpr_BlkWork, &telegram[NTUPR_SBUF_PRIOR], NTNET_PRIOR_DATA);
					sts = NTUPR_SEND_PRIOR;
				}
			}
			else if (telegram[NTUPR_SBUF_NORMAL].tele_len > 0) {
				if (!ntupr_IsWaiting(&telegram[NTUPR_SBUF_NORMAL])) {
			/* 通常データ送信 */
					len = ntupr_MkNtNetBlk(&z_NtUpr_BlkWork, &telegram[NTUPR_SBUF_NORMAL], NTNET_NORMAL_DATA);
					sts = NTUPR_SEND_NORMAL;
				}
			}
			if (len > 0) {
				if (NTLWR_SendReq((uchar*)&z_NtUpr_BlkWork, len, NTNET_NTDATA)) {
					ctrl->sts = sts;
				}
			}
		}
		break;
	case NTUPR_SEND_FREE:	/*----------------------- FREEデータ送信後 ----*/
		if (NTLWR_IsSendComplete(&result)) {
			ctrl->freedata.isdata = FALSE;
			ctrl->sts = NTUPR_IDLE;
		}
		break;
	case NTUPR_SEND_PRIOR:	/*----------------------- 優先データ送信後 ----*/
		/* no break */
	case NTUPR_SEND_NORMAL:	/*----------------------- 通常データ送信後 ----*/
		if (ctrl->sts == NTUPR_SEND_PRIOR) {
			tele = &telegram[NTUPR_SBUF_PRIOR];
		}
		else {	/* ctrl->sts == NTUPR_SEND_NORMAL */
			tele = &telegram[NTUPR_SBUF_NORMAL];
		}
		
		if (NTLWR_IsSendComplete(&result)) {
			// 遠隔『制御』電文の場合、リトライはせずに通知のみ
			if (z_NtUpr_BlkWork.data.system_id == REMOTE_SYSTEM) {
				// OK/NGに関わらず電文は削除
				// 通知
				if (z_NtRemoteAutoManual == REMOTE_AUTO) {
					ntautoPostResult(z_NtUpr_BlkWork.data.data_type, (uchar)result);
				} else {
					// メンテ通知
					IBKCtrl_SetRcvData_manu(z_NtUpr_BlkWork.data.data_type, (uchar)result);
				}
				// 送信完了=電文廃棄
				_ntupr_BufClr(tele);
				ctrl->sts = NTUPR_IDLE;
				break;
			}
			switch (result) {
			case NTNET_SRSLT_NORMAL:		/* 正常完了 */
				/* IBKバッファFULLによるリトライ回数クリア */
				tele->buffull_retry = 0;
				tele->send_bytenum += tele->cur_send_bytenum;
				if (tele->send_bytenum >= tele->tele_len) {
					_ntupr_BufClr(tele);	/* 送信完了=電文廃棄 */
					if (tele->fukuden.phase == NTUPR_FUKUDEN_HOLDING) {
						tele->fukuden.phase = NTUPR_FUKUDEN_DATACLR;
						ntupr_GetSndNtData(ctrl, tele);		// ここでバッファから削除
					}
				}
				break;
			case NTNET_SRSLT_BUFFULL:		/* IBKバッファFULL */
				// データは保持したまま送信停止/ニアフル解除で送信再開
				if (_is_ntnet_remote()) {
					// buffer層にニアフル通知
					NTBUF_SetIBKNearFullByID(z_NtUpr_BlkWork.data.data_type);
					// 送信中止、ワークエリアの電文廃棄
					_ntupr_BufClr(tele);					// バッファ内の元データは維持
					break;
				}
				/* 一定時間のWAIT後、同じデータを再送 */
				if (tele->buffull_retry < CPrmSS[S_NTN][NTPRM_BUFFULL_RETRY]) {
					if (++tele->buffull_retry >= CPrmSS[S_NTN][NTPRM_BUFFULL_RETRY]) {
						NTNET_Err(ERR_NTNET_BUFFULL_RETRY, NTERR_ONESHOT);	/* バッファFULLによる再送回数超過 */
					}
				}
				_ntupr_BufSndReset(tele);
				tele->buffull_timer = LifeTimGet();
				break;
			case NTNET_SRSLT_RETRY_OVER:	/* リトライ回数オーバー */
				NTNET_Err(ERR_NTNET_ARCBLKNO_INVALID, NTERR_ONESHOT);	/* ARCブロックNo.異常発生 */
				/* no break */
			case NTNET_SRSLT_BLKNO:			/* NTブロックNo.異常 */
				/* IBKバッファFULLによるリトライ回数クリア */
				tele->buffull_retry = 0;
				if (tele->blkno_retry++ < NTUPR_SND_RETRY) {
				/* 先頭ブロックから再送 */
					_ntupr_BufSndReset(tele);
				}
				else {
					_ntupr_BufClr(tele);
					if (tele->fukuden.phase == NTUPR_FUKUDEN_HOLDING) {
						tele->fukuden.phase = NTUPR_FUKUDEN_DATACLR;
						ntupr_GetSndNtData(ctrl, tele);		// ここでバッファから削除
					}
					NTNET_Err(ERR_NTNET_DISCARD_SENDDATA, NTERR_ONESHOT);	/* リトライオーバーによりデータ破棄 */
				}
				break;
			case NTNET_SRSLT_SENDING:			/* データ送信中(リモートのみ) */
				// 該当データを1分歩進ごとに再送
				// 該当バッファペンディング通知
				NTBUF_SetIBKPendingByID(z_NtUpr_BlkWork.data.data_type);
				z_1minutePending = CLK_REC.minu;
				// 電文破棄
				_ntupr_BufClr(tele);					// バッファ内の元データは維持
				break;
			}
			ctrl->sts = NTUPR_IDLE;
		}
		break;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_ChkSendData
 *[]----------------------------------------------------------------------[]
 *| summary	: 本モジュールで管理するバッファに送信データがあるかチェック。
 *|			  なければ上位モジュールからデータを取得する。
 *| param	: ctrl - 制御データ
 *|			  telegram - 本モジュールで管理する送受信バッファ
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntupr_ChkSendData(t_NtUpr_Ctrl *ctrl, t_NtUpr_Telegram *telegram)
{
	t_NtUpr_Telegram *tele;
	uchar nomal_chk;
	
	if (ctrl->sts != NTUPR_IDLE) {
		return;
	}
	
	/* FREEデータ */
	ctrl->freedata.isdata = NTBUF_GetSndFreeData(ctrl->freedata.data);
	if (ctrl->freedata.isdata) {
		return;
	}
	/* 優先データ */
	tele = &telegram[NTUPR_SBUF_PRIOR];
	nomal_chk = ( _is_ntnet_remote() && telegram[NTUPR_SBUF_NORMAL].tele_len > 0 );
	if( tele->tele_len <= 0 && !nomal_chk ){
		/* 手元にデータがないのでバッファから新しいデータをもらう */
		tele->fukuden.bufkind = NTNET_BUF_PRIOR;
		tele->fukuden.phase = NTUPR_FUKUDEN_DATAGET;
		if (ntupr_GetSndNtData(ctrl, tele)) {
			return;
		}
	}
	
	/* 通常データ */
	tele = &telegram[NTUPR_SBUF_NORMAL];
	if (tele->tele_len <= 0) {
		/* 手元にデータがないのでバッファから新しいデータをもらう */
		tele->fukuden.bufkind = NTNET_BUF_NORMAL;
		tele->fukuden.phase = NTUPR_FUKUDEN_DATAGET;
		if (ntupr_GetSndNtData(ctrl, tele)) {
			return;
		}
	}
	
	/* 送信可能なデータ無し */
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_GetSndNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: 上位モジュールからデータ取得＆削除
 *| param	: ctrl - 制御データ
 *|			  telegram - 本モジュールで管理する送受信バッファ
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
BOOL	ntupr_GetSndNtData(t_NtUpr_Ctrl *ctrl, t_NtUpr_Telegram *tele)
{
	BOOL ret = FALSE;
	
	switch (tele->fukuden.phase) {
	case NTUPR_FUKUDEN_DATAGET:
	/* データ取得 */
		tele->tele_len = NTBUF_GetSndNtData(tele->data, tele->fukuden.bufkind, &(tele->UpperQueKind) );
		if (tele->tele_len <= 0) {
			break;	/* データ無し */
		}
		/* 送信制御データ初期化 */
		_ntupr_BufReset(tele);
		/* データ削除準備 */
		NTBUF_ClrSndNtData_Prepare(tele->data, tele->fukuden.bufkind, &tele->fukuden.h);
		/* シーケンスNo.セット */
		if (tele->data[0] != REMOTE_SYSTEM) {
			((t_NtNetDataHeader*)(tele->data))->seq_no = ctrl->seqno;
			ctrl->seqno = (uchar)_ntupr_NextSeqno(ctrl->seqno);
		}
		if (_is_ntnet_remote() && tele->data[0] != REMOTE_SYSTEM) {
			tele->fukuden.phase = NTUPR_FUKUDEN_HOLDING;
			tele->UpperQueKind = 0xFF;	// データ件数2重カウントの防止
			break;						// 削除しない
		}
		tele->fukuden.phase = NTUPR_FUKUDEN_DATACLR;
		/* no break */
	case NTUPR_FUKUDEN_DATACLR:
	/* データ削除 */
		NTBUF_ClrSndNtData_Exec(&tele->fukuden.h);
		
		tele->fukuden.phase = NTUPR_FUKUDEN_NONE;
		ret = TRUE;
		break;
	default:
	case NTUPR_FUKUDEN_HOLDING:
		break;
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_SetRcvNtBlk
 *[]----------------------------------------------------------------------[]
 *| summary	: ブロックNo.チェックをして受信データをバッファリング
 *|			  最終ブロック受信で、上位バッファにデータをセットする
 *| param	: ctrl - 制御データ
 *|			  telegram - 本モジュールで管理する送受信バッファ
 *|			  data - 受信NT-NETデータ
 *|			  len - dataのバイト長
 *| return	: ブロック番号チェック結果
 *[]----------------------------------------------------------------------[]*/
eNTNET_RRSLT	ntupr_SetRcvNtBlk(t_NtUpr_Ctrl *ctrl, t_NtUpr_Telegram *telegram, uchar *data, ushort len)
{
	eNTNET_RRSLT	ret;
	t_NtNetBlk		*ntblk;
	t_NtUpr_Telegram	*tele;
	int				bufkind;

	ret = NTNET_RRSLT_NORMAL;
	
	if (len > 0) {
		ntblk = (t_NtNetBlk*)data;
		if (ntblk->header.terminal_no == NTNET_BROADCAST
			|| ntblk->header.mode == NTNET_PRIOR_DATA) {
		/* 同報 or 優先データ */
			if (ntblk->header.terminal_no == NTNET_BROADCAST) {
			/* 同報データ */
				tele	= &telegram[NTUPR_RBUF_BROADCAST];
				bufkind	= NTNET_BUF_BROADCAST;
			}
			else {
			/* 優先データ */
				tele	= &telegram[NTUPR_RBUF_PRIOR];
				bufkind	= NTNET_BUF_PRIOR;
			}
			if (ntblk->data.blk_no == 1 && ntblk->data.is_lastblk == NTNET_LAST_BLOCK) {
				_ntupr_BufClr(tele);
				tele->data[0] = ntblk->data.system_id;	/* ID3 */
				tele->data[1] = ntblk->data.data_type;	/* ID4 */
				tele->data[2] = ntblk->data.save;		/* データ保持フラグ */
				tele->tele_len = 3;
				tele->tele_len += ntupr_Unpack(&tele->data[tele->tele_len], ntblk, len);

				/* ============= 受信シーケンスNo.チェック(NTNET電文) ================= */
				/* 0又は前回受信時のシーケンスNo.以外、且つ99以下なら受信。その他は破棄 */
				/* ==================================================================== */
				if(	(((t_NtNetDataHeader*)(tele->data))->seq_no	==	0 					||
					((t_NtNetDataHeader*)(tele->data))->seq_no	!=	ctrl->rcv_seqno)	&&
					((t_NtNetDataHeader*)(tele->data))->seq_no	<	100
					){
					/* シーケンスNo.更新 */
					ctrl->rcv_seqno = ((t_NtNetDataHeader*)(tele->data))->seq_no;

					/* 精算機取り込み */
					/* 上位バッファモジュールに書き込み */
					if (NTBUF_SetRcvNtData(tele->data, tele->tele_len, (eNTNET_BUF_KIND)bufkind) != NTNET_BUFSET_NORMAL) {
						NTNET_Err(ntupr_BufErrCode((eNTNET_BUF_KIND)bufkind), NTERR_ONESHOT);	/* バッファオーバーライト発生 */
					}
				}

			}
			else {
				ret = NTNET_RRSLT_BLKNO_INVALID;
			}
		}
		else if (ntblk->header.mode == NTNET_NORMAL_DATA) {
		/* 通常データ */
			/* ブロックNo.をチェック */
			tele = &telegram[NTUPR_RBUF_NORMAL];
			if (ntblk->data.blk_no == 1 && tele->blknum > 0) {
				_ntupr_BufClr(tele);
				ret = NTNET_RRSLT_BLKNO_VALID;		/* ブロックNo.異常(今回有効) */
				NTNET_Err(ERR_NTNET_NTBLKNO_VALID, NTERR_ONESHOT);	/* ブロックNo.異常発生 */
			}
			else if (ntblk->data.blk_no != tele->blknum + 1) {
				_ntupr_BufClr(tele);
				ret = NTNET_RRSLT_BLKNO_INVALID;	/* ブロックNo.異常(今回無効) */
				NTNET_Err(ERR_NTNET_NTBLKNO_INVALID, NTERR_ONESHOT);	/* ブロックNo.異常発生 */
			}
			else if (ntblk->data.blk_no == NTNET_BLKMAX_NORMAL
					&& ntblk->data.is_lastblk == NTNET_NOT_LAST_BLOCK) {
				_ntupr_BufClr(tele);
				ret = NTNET_RRSLT_BLKNO_INVALID;	/* ブロックNo.異常(今回無効) */
				NTNET_Err(ERR_NTNET_RECV_BLK_OVER, NTERR_ONESHOT);	/* ブロックNo.異常発生 */
			}
			
			if (ret != NTNET_RRSLT_BLKNO_INVALID) {
			/* データ保存 */
				if (ntblk->data.blk_no == 1) {
				/* 先頭ブロック */
					tele->data[0] = ntblk->data.system_id;
					tele->data[1] = ntblk->data.data_type;
					tele->data[2] = ntblk->data.save;
					tele->tele_len = 3;
				}
				tele->tele_len += ntupr_Unpack(&tele->data[tele->tele_len], ntblk, len);
				tele->blknum++;
				if (ntblk->data.is_lastblk == NTNET_LAST_BLOCK) {

					/* ============= 受信シーケンスNo.チェック(NTNET電文) ================= */
					/* 0又は前回受信時のシーケンスNo.以外、且つ99以下なら転送。その他は破棄 */
					/* ==================================================================== */
					if(	((t_NtNetDataHeader*)(tele->data))->system_id == REMOTE_SYSTEM) {
						/* 上位バッファモジュールに書き込み */
						// この場合、seq_noは存在しない
						// また、優先バッファとする
						if (NTBUF_SetRcvNtData(tele->data, tele->tele_len, NTNET_BUF_PRIOR) != NTNET_BUFSET_NORMAL) {
							NTNET_Err(ntupr_BufErrCode(NTNET_BUF_PRIOR), NTERR_ONESHOT);	/* バッファオーバーライト通知 */
						}
					}
					else	// 以下通常電文↓
					if(	(((t_NtNetDataHeader*)(tele->data))->seq_no	==	0 					||
						((t_NtNetDataHeader*)(tele->data))->seq_no	!=	ctrl->rcv_seqno)	&&
						((t_NtNetDataHeader*)(tele->data))->seq_no	<	100
						){
						/* シーケンスNo.更新 */
						ctrl->rcv_seqno = ((t_NtNetDataHeader*)(tele->data))->seq_no;

						/* NETDOPA_MAF電文用システムIDの場合MAFへ転送 */
						/* 上位バッファモジュールに書き込み */
						if (NTBUF_SetRcvNtData(tele->data, tele->tele_len, NTNET_BUF_NORMAL) != NTNET_BUFSET_NORMAL) {
							NTNET_Err(ntupr_BufErrCode(NTNET_BUF_NORMAL), NTERR_ONESHOT);	/* バッファオーバーライト通知 */
						}
					}
					_ntupr_BufClr(tele);
				}
			}
		}
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_SetRcvFreeData
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信したFREEデータを上位バッファにセットする
 *| param	: data - 受信FREEデータ
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntupr_SetRcvFreeData(uchar *data)
{
	if (!NTBUF_SetRcvFreeData(data)) {
		NTNET_Err(ERR_NTNET_FREE_RCVBUF, NTERR_ONESHOT);	/* バッファオーバーライト通知 */
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_IsWaiting
 *[]----------------------------------------------------------------------[]
 *| summary	: IBKバッファFULLによる送信リトライ間隔の監視
 *| param	: tele - 送信したい電文データ
 *| return	: TRUE - 送信WAIT中
 *[]----------------------------------------------------------------------[]*/
BOOL	ntupr_IsWaiting(t_NtUpr_Telegram *tele)
{
	if (tele->buffull_retry <= 0) {
		return FALSE;		/* WAIT中でない */
	}
	if (LifePastTimGet(tele->buffull_timer) >= _To10msVal((ulong)CPrmSS[S_NTN][NTPRM_BUFFULL_TIMER])) {
		return FALSE;		/* WAIT完了 */
	}
	
	return TRUE;			/* WAIT中 */
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_Unpack
 *[]----------------------------------------------------------------------[]
 *| summary	: 0カット圧縮を解凍してdstにコピー
 *| param	: dst - 解凍後のデータコピー先
 *|			  ntblk - 受信したNT-NETブロック
 *|			  blklen - ntblkの有効データ長
 *| return	: dstにコピーしたサイズ
 *[]----------------------------------------------------------------------[]*/
ushort	ntupr_Unpack(uchar *dst, t_NtNetBlk *ntblk, ushort blklen)
{
	ushort len_before0cut, len_after0cut;

	len_after0cut	= blklen - (ushort)_offsetof(t_NtNetBlk, data.data);
	len_before0cut	= _MAKEWORD(ntblk->header.len_before0cut);
	
	/* 0カット後のデータをコピー */
	memcpy(dst, ntblk->data.data, (ulong)len_after0cut);
	/* 0カット分の伸長 */
	while (len_after0cut < len_before0cut) {
		dst[len_after0cut++] = 0;
	}
	
	return len_before0cut;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_MkNtNetBlk
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NETブロックデータ作成
 *| param	: ntblk - NT-NETブロックデータ作成先				<OUT>
 *|			  tele - 作成するブロックデータの元データ
 *|			  mode - パケット優先モード
 *| return	: ntblkの有効データ長
 *[]----------------------------------------------------------------------[]*/
ushort	ntupr_MkNtNetBlk(t_NtNetBlk *ntblk, t_NtUpr_Telegram *tele, uchar mode)
{
	ushort	datalen;
	uchar	blksts;

	if (tele->tele_len <= 0) {
		return 0;
	}
	
	if (tele->blknum <= 0) {
		/* 1ブロック目 */
		tele->send_bytenum = 3;		/* ID3,ID4,データ保持フラグ */
	}
	/* 今回送信データ長算出 */
	if (tele->tele_len - tele->send_bytenum > NTNET_BLKDATA_MAX) {
		datalen = NTNET_BLKDATA_MAX;
		blksts = NTNET_NOT_LAST_BLOCK;
	}
	else {
		datalen = tele->tele_len - tele->send_bytenum;
		blksts = NTNET_LAST_BLOCK;
	}
	
	/* NT-NETデータ組み上げ */
	memset(ntblk, 0, sizeof(t_NtNetBlk));
	ntblk->header.terminal_no		= (uchar)CPrmSS[S_NTN][NTPRM_TERMINALNO];
	ntblk->header.len_before0cut[0]	= (uchar)(datalen >> 8);
	ntblk->header.len_before0cut[1]	= (uchar)datalen;
	ntblk->header.mode				= mode;
	ntblk->header.telegram_type		= STX;
	ntblk->data.blk_no				= (uchar)++tele->blknum;
	ntblk->data.is_lastblk			= blksts;
	ntblk->data.system_id			= tele->data[0];
	ntblk->data.data_type			= tele->data[1];
	ntblk->data.save				= tele->data[2];
	// 遠隔制御電文の場合、データ保持フラグの最上位ビットが自動/手動を示す
	if (ntblk->data.system_id == REMOTE_SYSTEM) {
		// 遠隔電文送信フラグに自動/手動をセット
		if (ntblk->data.save & 0x80) {
			z_NtRemoteAutoManual = REMOTE_MANUAL;
		} else {
			z_NtRemoteAutoManual = REMOTE_AUTO;
		}
		ntblk->data.save &= 0x7f;
	}
	memcpy((char*)ntblk->data.data, &tele->data[tele->send_bytenum], (ulong)datalen);
	/* 今回送信データ長を保存 */
	tele->cur_send_bytenum = datalen;
	
	/* 0カットしたデータ長を算出 */
	while (datalen > 0) {
		if (ntblk->data.data[datalen-1] != 0) {
			break;
		}
		datalen--;
	}

	return (ushort)(datalen + _offsetof(t_NtNetBlk, data.data));
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_BufErrCode
 *[]----------------------------------------------------------------------[]
 *| summary	: バッファ種別コード⇒エラーコードへの変換
 *| param	: bufkind - バッファ種別
 *| return	: バッファオーバーライトエラーコード
 *[]----------------------------------------------------------------------[]*/
int		ntupr_BufErrCode(eNTNET_BUF_KIND bufkind)
{
	switch (bufkind) {
	case NTNET_BUF_BROADCAST:					/* 同報データ */
		return ERR_NTNET_BROADCAST_RCVBUF;
	case NTNET_BUF_PRIOR:						/* 優先データ */
		return ERR_NTNET_PRIOR_RCVBUF;
	default:	/* case NTNET_BUF_NORMAL: */	/* 通常データ */
		return ERR_NTNET_NORMAL_RCVBUF;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_BufErrCode
 *[]----------------------------------------------------------------------[]
 *| summary	: 現在送信バッファにある未送信パケットがあれば
 *|			  どのキューにあった電文かを返す。
 *|			  （指定したキュー種から抜き取ったパケットが未送信か否かを返す）
 *| param	: キュー種
 *|			  0～10 = 未送信パケットがそれ以前に保存されていたキュー種
 *|			  ("NTNET_SNDBUF_xxxx" で定義されている値)
 *| return	: 0 = 指定したキュー種パケットは未送信ではない
 *|			  1 = 指定したキュー種パケットは未送信である
 *[]----------------------------------------------------------------------[]*/
uchar	NTUPR_SendingPacketKindGet( ushort UpperQueKind )
{
	// 優先送信バッファ
	if( (0 != z_NtUpr_Telegram[ NTUPR_SBUF_PRIOR ].tele_len) &&					// 未送信ﾃﾞｰﾀあり
		(UpperQueKind == z_NtUpr_Telegram[ NTUPR_SBUF_PRIOR ].UpperQueKind) )	// 指定されたキュー種のﾃﾞｰﾀである
	{
		return	(uchar)1;
	}

	// 通常送信バッファ
	if( (0 != z_NtUpr_Telegram[ NTUPR_SBUF_NORMAL ].tele_len) &&				// 未送信ﾃﾞｰﾀあり
		(UpperQueKind == z_NtUpr_Telegram[ NTUPR_SBUF_NORMAL ].UpperQueKind) )	// 指定されたキュー種のﾃﾞｰﾀである
	{
		return	(uchar)1;
	}

	return	(uchar)0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTUPR_Init2
 *[]----------------------------------------------------------------------[]
 *| summary	: 運転途中で行う機能初期化（メンテからのオールクリア）
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTUPR_Init2( void )
{
	int i;
	
	/* 制御データのクリア */
	z_NtUpr_Ctrl.sts				= NTUPR_IDLE;
	z_NtUpr_Ctrl.freedata.isdata	= FALSE;
	z_NtUpr_Ctrl.seqno				= 0;
	z_NtUpr_Ctrl.rcv_seqno			= 0;										/* 受信ｼｰｹﾝｽNo.ｸﾘｱ */
	
	/* 送受信バッファデータのクリア */
	for (i = 0; i < NTUPR_BUF_MAX; i++) {
		_ntupr_BufClr(&z_NtUpr_Telegram[i]);
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_NtNetDopaSndBufCheck_Del
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信ﾊﾞｯﾌｧ状態ﾁｪｯｸ/ｸﾘｱ
 *| param	: bufkind 1:優先送信ﾊﾞｯﾌｧ 0:通常送信ﾊﾞｯﾌｧ
 *|			  status  1:送信ﾊﾞｯﾌｧのﾁｪｯｸ 0:送信ﾊﾞｯﾌｧのｸﾘｱ
 *|
 *| return	: 送信ﾊﾞｯﾌｧに未送信ﾃﾞｰﾀがない場合：0  
 *|			  送信ﾊﾞｯﾌｧに未送信ﾃﾞｰﾀがある場合：1
 *|			  送信ﾊﾞｯﾌｧｸﾘｱの場合(常に)		 ：0
 *[]----------------------------------------------------------------------[]*/
ushort		ntupr_NtNetDopaSndBufCheck_Del(uchar bufkind, uchar status)
{
	ushort res = 0;
	if(bufkind) {
		if(status){
			if(z_NtUpr_Telegram[NTUPR_SBUF_PRIOR].tele_len  > 0)
				res = 1;
		}
	}else{
		if(status){
			if(z_NtUpr_Telegram[NTUPR_SBUF_NORMAL].tele_len  > 0)
				res = 1;
		}else{
			_ntupr_BufClr(&z_NtUpr_Telegram[NTUPR_SBUF_PRIOR]);
			_ntupr_BufClr(&z_NtUpr_Telegram[NTUPR_SBUF_NORMAL]);
		}
	}
		
	return res;
}
