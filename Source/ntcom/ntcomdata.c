// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
// MH364300 GG119A03 (S) NTNET通信制御対応（標準UT4000：MH341111流用）
		// 本ファイルを追加			nt task(NT-NET通信処理部)
// MH364300 GG119A03 (E) NTNET通信制御対応（標準UT4000：MH341111流用）
/*[]----------------------------------------------------------------------[]
 *|	filename: NTComData.c
 *[]----------------------------------------------------------------------[]
 *| summary	: NTNET送受信データ管理機能
 *| author	: machida.k
 *| date	: 2005.06.07
 *| Update  : 2005-06-18	#001	J.Mitani	仕様変更により送信用電文のブロックチェック方法を変更
 *|		2005.07.05	J.Mitani	#002	停電保証処理追加
 *|		2005.09.13	machida.k	#003	停電保証データ仕様変更
 *|										(旧：優先データ以外を保証　→　新：データ保持フラグ=ONのデータを保証)
 *|								#004	停電保証処理追加
 *|								#005	送信用優先データバッファを端末数分確保する
 *|								#006	bug fix
 *|		2005-12-20	machida.k	RAM共有化
 *|		2006-01-11	machida.k	#007	bug fix(ブロックNo.異常と受信ブロック数オーバーのエラーコードを別にする)
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
#define	_NTComData_CheckBuffer(buf)	if ((buf)->fixed && (buf)->blknum <= 0) (buf)->fixed = FALSE	/* #004 */

static void			NTComData_ReconstructScheduler(T_NT_TELEGRAM_LIST *scheduler, T_NT_BUFFER *buffer, T_NT_FUKUDEN *fukuden);

static eNT_DATASET	NTComData_SetSndBlk(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk);
static eNT_DATASET	NTComData_SetRcvBlk(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk, BOOL *fixed);

static void			NTComData_SetRcvTeleSchedule(int terminal);
static int			NTComData_PeekRcvTeleSchedule(void);
/* #006-> */
static void			NTComData_DeleteRcvTeleSchedule(void);
/* <-#006 */

static BOOL			NTComData_AddBlk(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk);
static BOOL			NTComData_CreateTelegramInfo(T_NT_BUFFER *buffer, T_NT_TELEGRAM *telegram);
static BOOL			NTComData_GetBlkData(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk, int blk_no);
static void			NTComData_BufferClear(T_NT_BUFFER *buffer);

static BOOL			NTComData_SndBufClean(T_NT_BUFFER *buffer, eNT_STS terminal_failed, int terminal);
static BOOL			NTComData_ClrNotBufferingData(T_NT_BUFFER *buffer);							/* #003 */
static BOOL			NTComData_ChkBlkNo(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk);

static void			NTComData_CheckPowerFailure(void);			/* #002 */
static void			NTComData_Unpack0Cut(uchar *ucData);

/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/

/*====================================================================================[PUBLIC]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_Init()
 *[]----------------------------------------------------------------------[]
 *| summary	: データ管理モジュール内のデータ全てクリア
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_Clear(void)
{
//	int i;

	/* NTネットへの通常データ送信用バッファ */
	NTComData_InitBuffer(&NTCom_SndBuf_Normal,
						NTCom_BufMemPool.snd_pool_normal,
						_countof(NTCom_BufMemPool.snd_pool_normal));
	NTCom_SndBuf_Normal.kind = NT_DATA_KIND_NORMAL_SND;

	/* NTネットからの通常データ受信用バッファ */
	NTComData_InitBuffer(&NTCom_RcvBuf_Normal,
						NTCom_BufMemPool.rcv_pool_normal,
						_countof(NTCom_BufMemPool.rcv_pool_normal));
	NTCom_RcvBuf_Normal.kind = NT_DATA_KIND_NORMAL_RCV;

//	NTCom_RcvTeleSchedule.top		= 0;
	NTCom_RcvTeleSchedule.read		= 0;
//	NTCom_RcvTeleSchedule.telenum	= 0;
	NTCom_RcvTeleSchedule.write		= 0;

	/* NTネットへの優先データ送信用バッファ */
	NTComData_InitBuffer(&NTCom_SndBuf_Prior,
						NTCom_BufMemPool.snd_pool_prior,
						_countof(NTCom_BufMemPool.snd_pool_prior));
	NTCom_SndBuf_Prior.kind = NT_DATA_KIND_PRIOR_SND;

	/* NTネットからの優先データ受信用バッファ */
	NTComData_InitBuffer(&NTCom_RcvBuf_Prior,
					NTCom_BufMemPool.rcv_pool_prior,
					_countof(NTCom_BufMemPool.rcv_pool_prior));
	NTCom_RcvBuf_Prior.kind = NT_DATA_KIND_PRIOR_RCV;
/* NTネットへの同報データ送信用バッファ */
	NTComData_InitBuffer(&NTCom_SndBuf_Broadcast,
					NTCom_BufMemPool.snd_pool_broadcast,
					_countof(NTCom_BufMemPool.snd_pool_broadcast));
	NTCom_SndBuf_Broadcast.kind = NT_DATA_KIND_BROADCAST_SND;
/* NTネットからの同報データ受信用バッファ */
	NTComData_InitBuffer(&NTCom_RcvBuf_Broadcast,
					NTCom_BufMemPool.rcv_pool_broadcast,
					_countof(NTCom_BufMemPool.rcv_pool_broadcast));
	NTCom_RcvBuf_Broadcast.kind = NT_DATA_KIND_BROADCAST_RCV;

/* #004-> */
/* 停電保証処理制御データ */
	NTCom_Fukuden.scheduler		= NT_FUKUDEN_SCHEDULER_NONE;
/* <-#004 */
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_Start()
 *[]----------------------------------------------------------------------[]
 *| summary	: データ管理モジュール初期化
 *|			  terminal - 初期設定データ
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_Start(void)
{
	BOOL	ret;

	/* 通常バッファ停電保証処理 */
	NTComData_CheckPowerFailure();		/* #004 関数末尾より移動 */

	/* 端末接続情報 */
	NTComData_Ctrl.terminal_exists = FALSE;
	/* NTCom_InitData.ExecMode == NT_SLAVE */
	NTComData_Ctrl.terminal_exists = TRUE;

	/* 端末異常情報 */
	NTComData_Ctrl.terminal_status = NT_NORMAL;

// #003,#004->
	/* 停電保証対象外の送受信データをクリア */
	/* 通常データバッファ */
	ret = NTComData_ClrNotBufferingData(&NTCom_SndBuf_Normal);	// NT-NETへの送信用
	if(ret) {
		if(NTComOpeRecvNormalDataBuf.fixed) {
			NTComOpeRecvNormalDataBuf.fixed = FALSE;
		}
	}
	NTComData_ClrNotBufferingData(&NTCom_RcvBuf_Normal);	// NT-NETからの受信用

	/* 通常受信データのARCNETへの送信スケジューラ(停電保証対象外となり無効になったデータ分を削除する) */
	NTCom_Fukuden.scheduler = NT_FUKUDEN_SCHEDULER_START;
	NTComData_ReconstructScheduler(&NTCom_RcvTeleSchedule, &NTCom_RcvBuf_Normal, &NTCom_Fukuden);

	/* 優先データバッファ */
/* #005-> */
	ret = NTComData_ClrNotBufferingData(&NTCom_SndBuf_Prior);			// NT-NETへの送信用
	if(ret) {
		if(NTComOpeRecvPriorDataBuf.fixed) {
			NTComOpeRecvPriorDataBuf.fixed = FALSE;
		}
	}
/* <-#005 */
	NTComData_ClrNotBufferingData(&NTCom_RcvBuf_Prior);			// NT-NETからの受信用

	/* 同報データバッファ */
	NTComData_ClrNotBufferingData(&NTCom_SndBuf_Broadcast);		// NT-NETへの送信用
	NTComData_ClrNotBufferingData(&NTCom_RcvBuf_Broadcast);		// NT-NETからの受信用
// <-#003,#004

	/* エラー情報 */
	NTCom_Err.top = 0;
	NTCom_Err.num = 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_SetSndBlock()
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信バッファにデータセット
 *| param	: blk - バッファにセットするブロックデータ		<IN/OUT>
 *| return	: eNT_DATASET型
 *[]----------------------------------------------------------------------[]*/
eNT_DATASET	NTComData_SetSndBlock(T_NT_BLKDATA *blk)
{
	int terminal;

	terminal = blk->data[NT_OFS_TERMINAL_NO];

	if (terminal == NT_BROADCAST) {
	/* 同報データ */
		return NTComData_SetSndBlk(&NTCom_SndBuf_Broadcast, blk);
	}
	else if (blk->data[NT_OFS_PACKET_MODE] == NT_PRIOR_DATA) {
	/* 優先データ */
/* #005-> */
		if (terminal == NT_ALL_TERMINAL) {
			/* 全端末分の電文を作成 */
			if (NTComData_Ctrl.terminal_exists) {
				if (NTCom_SndBuf_Prior.fixed) {
					/* ひとつでもバッファフルであればこの電文は保存しない */
					return NT_DATA_NO_MORE_TELEGRAM;
				}
			}
			if (NTComData_Ctrl.terminal_exists) {
				blk->data[NT_OFS_TERMINAL_NO] = 1;
				NTComData_SetSndBlk(&NTCom_SndBuf_Prior, blk);
			}
			return NT_DATA_NORMAL;
		}
		else {
			return NTComData_SetSndBlk(&NTCom_SndBuf_Prior, blk);
		}

/* <-#005 */
	}
	else {	/* blk->data[NT_OFS_PACKET_MODE] == NT_NORMAL_DATA */
	/* 通常データ */
		if (terminal == NT_ALL_TERMINAL) {
			/* 電文を作成 */
			if (NTComData_Ctrl.terminal_exists) {
				if (NTCom_SndBuf_Normal.fixed) {
					/* ひとつでもバッファフルであればこの電文は保存しない */
					return NT_DATA_NO_MORE_TELEGRAM;
				}
			}
			if (NTComData_Ctrl.terminal_exists) {
				blk->data[NT_OFS_TERMINAL_NO] = 1;
				NTComData_SetSndBlk(&NTCom_SndBuf_Normal, blk);
			}
			return NT_DATA_NORMAL;
		}
		else {
			return NTComData_SetSndBlk(&NTCom_SndBuf_Normal, blk);
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_PeekSndTele_Normal()
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信可能な通常データ電文の情報を取得
 *| param	: terminal - 宛先端末番号
 *|						(従局動作時は自端末No.)
 *|			  telegram - 電文情報				<OUT>
 *| return	: TRUE - 電文有り
 *|			  FALSE - 電文なし
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_PeekSndTele_Normal(int terminal, T_NT_TELEGRAM *telegram)
{
	if (terminal < 1) {
		return FALSE;
	}

	return NTComData_CreateTelegramInfo(&NTCom_SndBuf_Normal, telegram);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_PeekSndTele_Prior()
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信可能な優先データ電文の情報を取得
 *| param	: terminal - 宛先端末番号
 *|						(従局動作時は自端末No.)
 *|			  telegram - 電文情報				<OUT>
 *| return	: TRUE - 電文有り
 *|			  FALSE - 電文なし
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_PeekSndTele_Prior(int terminal, T_NT_TELEGRAM *telegram)
{
	if (terminal < 1) {
		return FALSE;
	}

	return NTComData_CreateTelegramInfo(&NTCom_SndBuf_Prior, telegram);
}
/* <-#005 */

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_PeekSndTele_Broadcast()
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信可能な同報データ電文の情報を取得
 *| param	: telegram - 電文情報				<OUT>
 *| return	: TRUE - 電文有り
 *|			  FALSE - 電文なし
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_PeekSndTele_Broadcast(T_NT_TELEGRAM *telegram)
{
	return NTComData_CreateTelegramInfo(&NTCom_SndBuf_Broadcast, telegram);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_SetRcvBlock()
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信バッファにデータセット
 *| param	: blk - ブロックデータ				<IN>
 *| return	: eNT_DATASET型
 *[]----------------------------------------------------------------------[]*/
eNT_DATASET	NTComData_SetRcvBlock(T_NT_BLKDATA *blk)
{
	int		terminal;
	BOOL	fixed;
	eNT_DATASET ret;

	terminal = blk->data[NT_OFS_TERMINAL_NO];

	if (terminal == NT_BROADCAST) {
		/* 同報データ */
		ret = NTComData_SetRcvBlk(&NTCom_RcvBuf_Broadcast, blk, &fixed);
	}
	else if (blk->data[NT_OFS_PACKET_MODE] == NT_PRIOR_DATA) {
		/* 優先データ */
		ret = NTComData_SetRcvBlk(&NTCom_RcvBuf_Prior, blk, &fixed);
	}
	else {	/* blk->data[NT_OFS_PACKET_MODE] == NT_NORMAL_DATA */
		/* 通常データ */
		ret = NTComData_SetRcvBlk(&NTCom_RcvBuf_Normal, blk, &fixed);
		if (fixed) {
			NTComData_SetRcvTeleSchedule(terminal);
		}
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_PeekRcvTele_Normal()
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信完了した通常データ電文の情報を取得
 *| param	: telegram - 電文情報				<OUT>
 *| return	: TRUE - 電文有り
 *|			  FALSE - 電文なし
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_PeekRcvTele_Normal(T_NT_TELEGRAM *telegram)
{
	int terminal;

	terminal = NTComData_PeekRcvTeleSchedule();
	if (0 < terminal) {
		return NTComData_CreateTelegramInfo(&NTCom_RcvBuf_Normal, telegram);
	}

	return FALSE;		/* #006 */
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_PeekRcvTele_Prior()
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信完了した優先データ電文の情報を取得
 *| param	: telegram - 電文情報				<OUT>
 *| return	: TRUE - 電文有り
 *|			  FALSE - 電文なし
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_PeekRcvTele_Prior(T_NT_TELEGRAM *telegram)
{
	return NTComData_CreateTelegramInfo(&NTCom_RcvBuf_Prior, telegram);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_PeekRcvTele_Broadcast()
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信完了した同報データ電文の情報を取得
 *| param	: telegram - 電文情報				<OUT>
 *| return	: TRUE - 電文有り
 *|			  FALSE - 電文なし
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_PeekRcvTele_Broadcast(T_NT_TELEGRAM *telegram)
{
	return NTComData_CreateTelegramInfo(&NTCom_RcvBuf_Broadcast, telegram);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_GetRecvBlock()
 *[]----------------------------------------------------------------------[]
 *| summary	: ブロックポインタの位置にあるデータを取得する
 *| param	: telegram - 同報データ電文情報		<IN/OUT>
 *|			  blk - ブロックデータ				<OUT>
 *| return	: [0 ≦ X] : 該当電文の残りブロック数(0=残りブロックなし)
 *|			  [0 ＞ X] : パラメータエラー
 *[]----------------------------------------------------------------------[]*/
int		NTComData_GetRecvBlock(T_NT_TELEGRAM *telegram, T_NTCom_SndWork *sndWork)
{
	uchar i;
	union {
		uchar	uc[2];
		ushort	us;
	}u;

	memset(sndWork, 0, sizeof(T_NTCom_SndWork));

	switch (telegram->kind) {
	case NT_DATA_KIND_NORMAL_RCV:
		sndWork->len = 0;
		if(NTComOpeSendNormalDataBuf.count == 0) {
			return -1;
		}
		for(i=0; NTComOpeSendNormalDataBuf.count > 0; i++) {
			u.uc[0] = NTComOpeSendNormalDataBuf.ntDataBuf[NTComOpeSendNormalDataBuf.readIndex][14];
			u.uc[1] = NTComOpeSendNormalDataBuf.ntDataBuf[NTComOpeSendNormalDataBuf.readIndex][15];
			if(i == 0) {
				u.us+= 3;
				memcpy(sndWork->sendData, &NTComOpeSendNormalDataBuf.ntDataBuf[NTComOpeSendNormalDataBuf.readIndex][22], u.us);
			}
			else {
				memcpy(&sndWork->sendData[sndWork->len], &NTComOpeSendNormalDataBuf.ntDataBuf[NTComOpeSendNormalDataBuf.readIndex][25], u.us);
			}
			sndWork->len += u.us;

			++NTComOpeSendNormalDataBuf.readIndex;
			if(NTComOpeSendNormalDataBuf.readIndex == NT_NORMAL_BLKNUM_MAX) {
				NTComOpeSendNormalDataBuf.readIndex = 0;
			}
			--NTComOpeSendNormalDataBuf.count;				// データ格納数減算
		}
		break;
	case NT_DATA_KIND_PRIOR_RCV:
		u.uc[0] = NTCom_RcvBuf_Prior.blkdata[telegram->curblk].data[14];
		u.uc[1] = NTCom_RcvBuf_Prior.blkdata[telegram->curblk].data[15];
		sndWork->len = u.us;
		memcpy(sndWork->sendData,
				&NTCom_RcvBuf_Prior.blkdata[telegram->curblk].data,
				NTCom_RcvBuf_Prior.blkdata[telegram->curblk].len);
		break;
	case NT_DATA_KIND_BROADCAST_RCV:
		u.uc[0] = NTCom_RcvBuf_Broadcast.blkdata[telegram->curblk].data[14];
		u.uc[1] = NTCom_RcvBuf_Broadcast.blkdata[telegram->curblk].data[15];
		sndWork->len = u.us;
		sndWork->len = NTCom_RcvBuf_Broadcast.blkdata[telegram->curblk].len;
		memcpy(sndWork->sendData,
				&NTCom_RcvBuf_Broadcast.blkdata[telegram->curblk].data,
				NTCom_RcvBuf_Broadcast.blkdata[telegram->curblk].len);
		break;
	default:
		return -1;
	}

	return telegram->blknum - telegram->curblk;
}

/*[]----------------------------------------------------------------------[]*
 *|             NTComData_Unpack0Cut()
 *[]----------------------------------------------------------------------[]*
 *|	0カットされたデータを復元する｡
 *[]----------------------------------------------------------------------[]*
 *|	patam	void
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTComData_Unpack0Cut(uchar *ucData)
{

	ushort uiBefore0Cut;
	ushort uiAfter0Cut;

	union {
		uchar	uc[2];
		ushort	us;
	}u;

	u.uc[0] = ucData[5];
	u.uc[1] = ucData[6];

	// 0カット後のサイズを取得
	uiAfter0Cut = u.us - sizeof(t_NtNetBlkHeader) + 5;

	u.uc[0] = ucData[14];
	u.uc[1] = ucData[15];

	// 0カット前のサイズを取得
	uiBefore0Cut = u.us;

	while (uiBefore0Cut > uiAfter0Cut) {
		ucData[uiAfter0Cut + sizeof(t_NtNetBlkHeader) + 5] = 0x00;
		uiAfter0Cut++;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_GetSendBlock()
 *[]----------------------------------------------------------------------[]
 *| summary	: ブロックポインタの位置にあるデータを取得する
 *| param	: telegram - 同報データ電文情報		<IN/OUT>
 *|			  blk - ブロックデータ				<OUT>
 *| return	: [0 ≦ X] : 該当電文の残りブロック数(0=残りブロックなし)
 *|			  [0 ＞ X] : パラメータエラー
 *[]----------------------------------------------------------------------[]*/
int		NTComData_GetSendBlock(T_NT_TELEGRAM *telegram, T_NT_BLKDATA *blk)
{
	BOOL ret;

	switch (telegram->kind) {
	case NT_DATA_KIND_NORMAL_SND:
		ret = NTComData_GetBlkData(&NTCom_SndBuf_Normal, blk, telegram->curblk);
		break;
	case NT_DATA_KIND_PRIOR_SND:
		ret = NTComData_GetBlkData(&NTCom_SndBuf_Prior, blk, telegram->curblk);	/* #005 */
		break;
	case NT_DATA_KIND_BROADCAST_SND:
		ret = NTComData_GetBlkData(&NTCom_SndBuf_Broadcast, blk, telegram->curblk);
		break;
	default:
		return -1;
	}

	if (ret) {
		return telegram->blknum - telegram->curblk;
	}

	return -1;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_SeekBlock()
 *[]----------------------------------------------------------------------[]
 *| summary	: ブロックポインタを移動する
 *| param	: telegram - 同報データ電文情報		<IN/OUT>
 *|			  offset - originからのオフセット
 *|			  origin - 初期位置
 *|							NT_SEEK_SET - 電文先頭
 *|							NT_SEEK_CUR - 現在位置
 *|							NT_SEEK_END - 電文終端
 *| return	: [0 ≦ X] : 該当電文の残りブロック数(0=残りブロックなし)
 *|			  [0 ＞ X] : パラメータエラー
 *[]----------------------------------------------------------------------[]*/
int		NTComData_SeekBlock(T_NT_TELEGRAM *telegram, int offset, eNT_SEEK origin)
{
	switch (origin) {
	case NT_SEEK_SET:
		telegram->curblk = offset;
		break;
	case NT_SEEK_CUR:
		telegram->curblk += offset;
		break;
	case NT_SEEK_END:
		telegram->curblk = telegram->blknum + offset;
		break;
	default:
		return -1;
	}
	if (telegram->curblk < 0) {
		telegram->curblk = 0;
	}
	else if (telegram->curblk > telegram->blknum) {
		telegram->curblk = telegram->blknum;
	}

	return telegram->blknum - telegram->curblk;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_DeleteTelegram()
 *[]----------------------------------------------------------------------[]
 *| summary	: バッファから電文データを削除する
 *| param	: telegram - 削除対象となる電文		<IN>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_DeleteTelegram(T_NT_TELEGRAM *telegram)
{
	switch (telegram->kind) {
	case NT_DATA_KIND_NORMAL_SND:		/* 通常データバッファ(送信)の電文 */
		NTComData_BufferClear(&NTCom_SndBuf_Normal);
		NTComOpeRecvNormalDataBuf.fixed = FALSE;
		break;
	case NT_DATA_KIND_NORMAL_RCV:		/* 通常データバッファ(受信)の電文 */
		NTComData_BufferClear(&NTCom_RcvBuf_Normal);
		NTComData_DeleteRcvTeleSchedule();	/* スケジューラの情報もクリア */
		break;
	case NT_DATA_KIND_PRIOR_SND:		/* 優先データバッファ(送信)の電文 */
		NTComData_BufferClear(&NTCom_SndBuf_Prior);		/* #005 */
		NTComOpeRecvPriorDataBuf.fixed = FALSE;
		break;
	case NT_DATA_KIND_PRIOR_RCV:		/* 優先データバッファ(受信)の電文 */
		NTComData_BufferClear(&NTCom_RcvBuf_Prior);
		break;
	case NT_DATA_KIND_BROADCAST_SND:	/* 同報データバッファ(送信)の電文 */
		NTComData_BufferClear(&NTCom_SndBuf_Broadcast);
		break;
	case NT_DATA_KIND_BROADCAST_RCV:	/* 同報データバッファ(受信)の電文 */
		NTComData_BufferClear(&NTCom_RcvBuf_Broadcast);
		break;
	default:
		break;
	}
	/* 電文情報構造体を無効化 */
	NTComData_ResetTelegram(telegram);
}

/* #004-> */
/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_IsValidTelegram()
 *[]----------------------------------------------------------------------[]
 *| summary	: パラメータのtelegramが有効データに対応するものがどうかをチェックする
 *| param	: telegram - チェック対象		<IN/OUT>
 *| return	: TRUE - 有効
 *|			  FALSE - 無効
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_IsValidTelegram(T_NT_TELEGRAM *telegram)
{
	T_NT_BUFFER *buffer;

	if (telegram->terminal <= 0) {
		if (telegram->terminal != NT_BROADCAST
			|| (telegram->kind != NT_DATA_KIND_BROADCAST_SND && telegram->kind != NT_DATA_KIND_BROADCAST_RCV)) {
			NTComData_ResetTelegram(telegram);
			return FALSE;					// 端末番号異常
		}
	}

	switch (telegram->kind) {
	case NT_DATA_KIND_NORMAL_SND:
		buffer = &NTCom_SndBuf_Normal;
		break;
	case NT_DATA_KIND_NORMAL_RCV:
		buffer = &NTCom_RcvBuf_Normal;
		break;
	case NT_DATA_KIND_PRIOR_SND:
		buffer = &NTCom_SndBuf_Prior;
		break;
	case NT_DATA_KIND_PRIOR_RCV:
		buffer = &NTCom_RcvBuf_Prior;
		break;
	case NT_DATA_KIND_BROADCAST_SND:
		buffer = &NTCom_SndBuf_Broadcast;
		break;
	case NT_DATA_KIND_BROADCAST_RCV:
		buffer = &NTCom_RcvBuf_Broadcast;
		break;
	default:
		NTComData_ResetTelegram(telegram);
		return FALSE;					// 電文種別異常
	}

	if (!buffer->fixed) {
		NTComData_ResetTelegram(telegram);
		return FALSE;					// telegramが示すバッファに確定電文無し
	}

	if (telegram->curblk < 0) {
		return FALSE;					// 無効telegaram
	}

	return TRUE;
}
/* <-#004 */

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_SetTerminalStatus()
 *[]----------------------------------------------------------------------[]
 *| summary	: 端末接続状態変化通知
 *| param	: terminal - 通信障害が発生した端末の端末No.
 *|						(従局動作時は自端末No.)
 *|			  sts - eNT_STS型
 *| return	: TRUE as success
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_SetTerminalStatus(int terminal, eNT_STS sts, T_NT_TELEGRAM *telegram_normal, T_NT_TELEGRAM *telegram_prior)
{
	if (terminal <= 0) {
		return FALSE;
	}

	NTComData_Ctrl.terminal_status = sts;
	if (sts == NT_ABNORMAL) {
		/* バッファ再構成 */
		/* 通常データ送信バッファ */
		if (NTComData_SndBufClean(&NTCom_SndBuf_Normal, NTComData_Ctrl.terminal_status, terminal)) {
			if (telegram_normal != NULL) {
				NTComData_ResetTelegram(telegram_normal);	/* バッファクリアされたら電文情報もリセット */
				if(NTComOpeRecvNormalDataBuf.fixed) {
					NTComOpeRecvNormalDataBuf.fixed = FALSE;
				}
			}
		}
		/* 優先データ送信バッファ */
		if (NTComData_SndBufClean(&NTCom_SndBuf_Prior, NTComData_Ctrl.terminal_status, terminal)) {	/* #005 */
			if (telegram_prior != NULL) {
				NTComData_ResetTelegram(telegram_prior);	/* バッファクリアされたら電文情報もリセット */
				if(NTComOpeRecvPriorDataBuf.fixed) {
					NTComOpeRecvPriorDataBuf.fixed = FALSE;
				}
			}
		}
		/* 同報データはやらない(通信異常発生時はエラーデータ(同報応答無し)を発生させる仕様のため) */
	}

	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_GetBufferStatus()
 *[]----------------------------------------------------------------------[]
 *| summary	: バッファ状態取得
 *| param	: bufsts - バッファ状態				<OUT>
 *|			  terminal - 通常データバッファの状態が欲しい端末No.
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_GetBufferStatus(T_NT_BUF_STATUS *bufsts, int terminal)
{
	/* 通常データバッファ(送信用) */
	bufsts->normal_s		= NTCom_SndBuf_Normal.fixed;
	/* 通常データバッファ(受信用) */
	bufsts->normal_r		= NTCom_RcvBuf_Normal.fixed;
	/* 優先データバッファ(送信用) */
	bufsts->prior_s			= NTCom_SndBuf_Prior.fixed;		/* #005 */
	/* 優先データバッファ(受信用) */
	bufsts->prior_r			= NTCom_RcvBuf_Prior.fixed;
	/* 同報データバッファ(送信用) */
	bufsts->broadcast_s		= NTCom_SndBuf_Broadcast.fixed;
	/* 同報データバッファ(受信用) */
	bufsts->broadcast_r		= NTCom_RcvBuf_Broadcast.fixed;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_GetSndBufferCount()
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信バッファに含まれるブロック数を取得する
 *| param	: bufcnt - バッファに含まれるブロック数			<OUT>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_GetSndBufferCount(T_NT_BUF_COUNT *bufcnt)
{

	bufcnt->prior = 0;
	/* 通常データバッファのブロック数 */
	bufcnt->normal = NTCom_SndBuf_Normal.blknum;
	/* 優先データバッファのブロック数 */
	bufcnt->prior += NTCom_SndBuf_Prior.blknum;
/* <-#005 */
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_GetRcvBufferCount()
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信バッファに含まれるブロック数を取得する
 *| param	: bufcnt - バッファに含まれるブロック数			<OUT>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_GetRcvBufferCount(T_NT_BUF_COUNT *bufcnt)
{

	/* 通常データバッファのブロック数 */
	bufcnt->normal = NTCom_RcvBuf_Normal.blknum;

	/* 優先データバッファのブロック数 */
	bufcnt->prior = NTCom_RcvBuf_Prior.blknum;
}


/*====================================================================================[PRIVATE]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_InitBuffer()
 *[]----------------------------------------------------------------------[]
 *| summary	: 電文リストの初期化
 *| param	: buffer - データバッファ							<OUT>
 *|			  mempool  - データバッファで使用するバッファメモリ	<IN>
 *|			  mempool_size - mempoolのサイズ(配列要素数)
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_InitBuffer(T_NT_BUFFER *buffer, T_NT_BLKDATA *mempool, int mempool_size)
{
	/* メモリクリア */
	memset(mempool, 0, mempool_size);
	/* メモリアロケート */
	buffer->blkdata	= mempool;
	buffer->max		= mempool_size;
	/* バッファ制御データ初期化 */
	buffer->fixed	= FALSE;
	buffer->blknum	= 0;
}

/* #004-> */
/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_ReconstructScheduler()
 *[]----------------------------------------------------------------------[]
 *| summary	: 通常受信データのARCNETへの送信スケジューラの再構成
 *| param	: scheduler - 送信スケジューラ					<IN/OUT>
 *|			  buffer - 受信通常データバッファ				<IN>
 *|			  fukuden - 復電処理制御データ					<IN/OUT>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_ReconstructScheduler(T_NT_TELEGRAM_LIST *scheduler, T_NT_BUFFER *buffer, T_NT_FUKUDEN *fukuden)
{
	int ringsize, i, read;

	switch (fukuden->scheduler) {
	case NT_FUKUDEN_SCHEDULER_START:
	/* スケジューラからワークへ、有効データのみを書き出し */
		ringsize = 0;
		read = scheduler->read;
		while (read != scheduler->write) {
			if (buffer->fixed) {
//				fukuden->scheduler_work[ringsize++] = scheduler->ring[read];
				fukuden->scheduler_work = scheduler->ring[read];
			}
			read = _offset(read, 1, _countof(scheduler->ring));
		}
		fukuden->scheduler_worksize = ringsize;
		fukuden->scheduler = NT_FUKUDEN_SCHEDULER_MADE;
		/* no break */
	case NT_FUKUDEN_SCHEDULER_MADE:
	/* ワークからスケジューラへの書き戻し */
		for (i = 0; i < fukuden->scheduler_worksize; i++) {
//			scheduler->ring[i] = fukuden->scheduler_work[i];
			scheduler->ring[i] = fukuden->scheduler_work;
		}
		scheduler->read = 0;
		scheduler->write = fukuden->scheduler_worksize;
		fukuden->scheduler = NT_FUKUDEN_SCHEDULER_NONE;
		break;
	default:
		break;
	}
}
/* <-#004 */

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_SetSndBlk()
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信ブロック保存
 *| param	: buffer - 保存先バッファ			<IN/OUT>
 *|			  blk - 保存ブロックデータ			<IN>
 *| return	: eNT_DATASET型
 *| note	: #001
 *[]----------------------------------------------------------------------[]*/
eNT_DATASET	NTComData_SetSndBlk(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk)
{
	eNT_DATASET ret;
	int terminal;

	ret = NT_DATA_NORMAL;

	if (buffer->fixed) {
		return NT_DATA_NO_MORE_TELEGRAM;
	}

	if (!NTComData_ChkBlkNo(buffer, blk)) {
	/* ブロックNo.異常 */
		NTComData_BufferClear(buffer);
		if (blk->data[NT_OFS_BLOCK_NO] == 1) {
			/* 今来たブロックデータのブロックNo.が1であれば、
			   電文先頭のデータとしてあらためてバッファ保存される */
			ret = NT_DATA_BLOCK_INVALID2;	/* データ有効なのでリターンしない */
		}
		else {
			return NT_DATA_BLOCK_INVALID1;			/* ブロックデータ無効 */
		}
	}

	/* バッファに保存 */
	NTComData_AddBlk(buffer, blk);	/* バッファ残量のチェックはここではなく下記タイミングで行う
										(27ブロック目受信時ではなく最終ブロックでない26ブロック目受信でエラーとする仕様) */

	/* 最終ブロックのデータであれば電文リストに電文情報を作成 */
	if (blk->data[NT_OFS_LAST_BLOCK_FLAG] == NT_LAST_BLOCK) {
		buffer->fixed = TRUE;			/* 電文確定! */

		terminal = blk->data[NT_OFS_TERMINAL_NO];
		if (terminal != NT_ALL_TERMINAL && terminal != NT_BROADCAST) {
			/* 故障端末宛てデータ削除 */
			NTComData_SndBufClean(buffer, NTComData_Ctrl.terminal_status, terminal);
		}
	}
	else {
		/* バッファ残量チェック */
		if (buffer->blknum >= buffer->max) {
			NTComData_BufferClear(buffer);
// #007->
//			return NT_DATA_BLOCK_INVALID1;
			return NT_DATA_BUFFERFULL;
// <-#007
		}
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_SetRcvBlk()
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信ブロック保存
 *| param	: buffer - 保存先バッファ			<IN>
 *|			  blk - 保存するブロックデータ		<IN/OUT>
 *|			  fixed - 1電文受信完了検出時、TRUE
 *| return	: eNT_DATASET型
 *[]----------------------------------------------------------------------[]*/
eNT_DATASET	NTComData_SetRcvBlk(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk, BOOL *fixed)
{
//	int i;
	eNT_DATASET	ret = NT_DATA_NORMAL;
	union {
		uchar	uc[2];
		ushort	us;
	}u;

	if (buffer->fixed) {
		return NT_DATA_NO_MORE_TELEGRAM;
	}

	*fixed = FALSE;

	/* ブロックNo.異常 */
	if (!NTComData_ChkBlkNo(buffer, blk)) {
		/* ブロックNo.異常 */
		NTComData_BufferClear(buffer);
		if (blk->data[NT_OFS_BLOCK_NO] == 1) {
			/* 今来たブロックデータのブロックNo.が1であれば、
			   電文先頭のデータとしてバッファ保存する */
			ret = NT_DATA_BLOCK_INVALID2;			/* データ有効なのでリターンしない */
		}
		else {
			return NT_DATA_BLOCK_INVALID1;
		}
	}
	/* バッファに保存 */
	NTComData_AddBlk(buffer, blk);	/* バッファ残量のチェックはここではなく下記タイミングで行う
										(27ブロック目受信時ではなく最終ブロックでない26ブロック目受信でエラーとする仕様) */
	if( (blk->data[NT_OFS_PACKET_MODE] == NT_NORMAL_DATA) &&
		(blk->data[NT_OFS_TERMINAL_NO] != 0xFF) ) {
		// パケットデータサイズ取得
		u.uc[0] = blk->data[5];
		u.uc[1] = blk->data[6];
		memset(&NTComOpeSendNormalDataBuf.ntDataBuf[NTComOpeSendNormalDataBuf.writeIndex], 0, sizeof(T_NT_BLKDATA));
		memcpy(&NTComOpeSendNormalDataBuf.ntDataBuf[NTComOpeSendNormalDataBuf.writeIndex], blk->data, u.us);
		// 0カットされたデータを復元する
		NTComData_Unpack0Cut(&NTComOpeSendNormalDataBuf.ntDataBuf[NTComOpeSendNormalDataBuf.writeIndex][0]);
		NTComOpeSendNormalDataBuf.writeIndex++;
		NTComOpeSendNormalDataBuf.count++;
	}

	/* 最終ブロックのデータであれば電文リストに電文情報を作成 */
	if (blk->data[NT_OFS_LAST_BLOCK_FLAG] == NT_LAST_BLOCK) {
		buffer->fixed = TRUE;			/* 電文確定! */
		*fixed = TRUE;
	}
	else {
		/* バッファ残量チェック */
		if (buffer->blknum >= buffer->max) {
			NTComData_BufferClear(buffer);
// #007->
//			return NT_DATA_BLOCK_INVALID1;
			return NT_DATA_BUFFERFULL;
// <-#007
		}
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_SetRcvTeleSchedule()
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NETから受信完了した通常データ電文の情報を、IBW送信スケジューラに追加する
 *| param	: terminal - 受信した通常データ電文の送信元端末No.
 *|						(従局動作時は自端末No.)
 *| return	: none
 *|	update	:	#002
 *[]----------------------------------------------------------------------[]*/
void	NTComData_SetRcvTeleSchedule(int terminal)
{
	NTCom_RcvTeleSchedule.ring[NTCom_RcvTeleSchedule.write] = terminal;
	NTCom_RcvTeleSchedule.write = _offset(NTCom_RcvTeleSchedule.write, 1, _countof(NTCom_RcvTeleSchedule.ring));
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_PeekRcvTeleSchedule()
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NETから受信完了した通常データ電文の順番が記録されているIBW送信スケジューラから、
 *|			  次にIBWへ送信すべき電文が保存されているバッファを取得する
 *| param	: terminal - 受信した通常データ電文の送信元端末No.
 *|						(従局動作時は自端末No.)
 *| return	: 正常時：端末No.
 *|			  異常時：-1
 *|	update	:	#002
 *[]----------------------------------------------------------------------[]*/
int		NTComData_PeekRcvTeleSchedule(void)
{
	int terminal;

	if (NTCom_RcvTeleSchedule.read == NTCom_RcvTeleSchedule.write) {
		return -1;
	}

	terminal = NTCom_RcvTeleSchedule.ring[NTCom_RcvTeleSchedule.read];

	return terminal;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_DeleteRcvTeleSchedule()
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NETから受信完了した通常データ電文の順番が記録されているIBW送信スケジューラから、最後尾のデータを削除
 *| param	: none
 *| return	: none
 *|	update	:	#002
 *[]----------------------------------------------------------------------[]*/
void	NTComData_DeleteRcvTeleSchedule(void)
{
	if (NTCom_RcvTeleSchedule.read == NTCom_RcvTeleSchedule.write) {
		return;
	}

	NTCom_RcvTeleSchedule.read = _offset(NTCom_RcvTeleSchedule.read, 1, _countof(NTCom_RcvTeleSchedule.ring));
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NT_IsNewTypeTimeSetPacket()
 *[]----------------------------------------------------------------------[]
 *| summary	: 新形式の時計データ電文か否かの判定関数
 *| param	: blk - 保存するデータ				<IN>
 *| return	: TRUE is BINGO
 *[]----------------------------------------------------------------------[]*/
BOOL	NT_IsNewTypeTimeSetPacket( T_NT_BLKDATA *blk )
{
	union {
		ushort	us;
		uchar	uc[2];
	} u;

	if( (blk->data[NT_OFS_LAST_BLOCK_FLAG] == NT_LAST_BLOCK) &&
		(blk->data[NT_OFS_BLOCK_NO] == 1) &&						// 1ブロック電文で かつ
		((blk->data[NT_OFS_DATA_KIND] == NT_TMPKT_ID1) || (blk->data[NT_OFS_DATA_KIND] == NT_TMPKT_ID2)) )
																	// 時計データ電文の場合
	{
		u.uc[0] = blk->data[NT_OFS_DATASIZE_WITH_ZERO];
		u.uc[1] = blk->data[NT_OFS_DATASIZE_WITH_ZERO+1];			// 0カット前のデータサイズget
		if( u.us >= NT_TMPKT_NEW_TYPE_PKT_DATA_LEN ){				// 新時計データ電文（補正値付き）であれば
			return TRUE;
		}
	}
	return FALSE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_AddBlk()
 *[]----------------------------------------------------------------------[]
 *| summary	: リスト最後尾にデータを保存する
 *| param	: buffer - 保存先バッファリスト	<IN/OUT>
 *|			  blk - 保存するデータ				<IN>
 *| return	: TRUE as success
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_AddBlk(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk)
{
	union {
		ushort	us;
		uchar	uc[2];
	} u;
	ulong		wkul;
	short		AddLength;		// 時計更新要求電文が0カットされて短い場合、補足する長さ

	/* バッファ残量チェック */
	if (buffer->blknum >= buffer->max) {
		return FALSE;
	}

	// 送受信 両電文は本関数で背引用バッファにコピーされる
	// ここで時計データ電文（ID=119，229）であれば受信時刻として
	// ライフタイマー（フリーランニングタイマ）値をセットする

	if( TRUE == NT_IsNewTypeTimeSetPacket(blk) ) {						// 新形式の時計データ電文
		wkul = LifeTim1msGet();
		memcpy( &blk->data[NT_OFS_DATA_TMPKT_FREETIMER], &wkul, 4 );	// 電文に現在のライフタイマー値をセット(*1)

		/* 電文末尾にデータをセットしたので、電文が0カットされていたらデータ長を末尾までの値にセットする */
		if( blk->len < NT_TMPKT_NEW_TYPE_PKT_TOTAL_LEN ){
			AddLength = NT_TMPKT_NEW_TYPE_PKT_TOTAL_LEN - blk->len;		// 短い分を算出
			if( AddLength > 4 ){										// 末尾4byteは *1でセットしているので、上書きしないようにする
				memset( &blk->data[blk->len], 0, (AddLength - 4) );		// データ長を41byteにして、不足分に 0 をセットする。
			}

			blk->len = NT_TMPKT_NEW_TYPE_PKT_TOTAL_LEN;					// 引数値Length変更

			u.us = NT_TMPKT_NEW_TYPE_PKT_TOTAL_LEN;						// 電文ヘッダ内 パケットデータサイズset
			blk->data[NT_OFS_DATA_SIZE_HI ] = u.uc[0];
			blk->data[NT_OFS_DATA_SIZE_LOW] = u.uc[1];
		}
	}

	/* ブロックデータコピー */
	memcpy(&buffer->blkdata[buffer->blknum], blk, sizeof(T_NT_BLKDATA));
	buffer->blknum++;

	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_CreateTelegramInfo()
 *[]----------------------------------------------------------------------[]
 *| summary	: 電文情報取得
 *| param	: buffer - バッファ					<IN>
 *|			  telegram - 電文情報				<OUT>
 *| return	: TRUE - 電文有り
 *|			  FALSE - 電文なし
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_CreateTelegramInfo(T_NT_BUFFER *buffer, T_NT_TELEGRAM *telegram)
{
	if (!buffer->fixed) {
		return FALSE;
	}
	telegram->curblk	= -1;		/* invalid */
	telegram->kind		= buffer->kind;
	telegram->blknum	= buffer->blknum;
	telegram->terminal	= buffer->blkdata[0].data[NT_OFS_TERMINAL_NO];
	telegram->saveflag	= buffer->blkdata[0].data[NT_OFS_BUFFERING_FLAG];		/* #003 */
	telegram->curblk	= 0;		/* valid */

	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_GetBlkData()
 *[]----------------------------------------------------------------------[]
 *| summary	: ブロックポインタの位置にあるデータを取得する
 *|			  データ取得完了後、ブロックデータポインタを1進める
 *| param	: buffer - バッファ					<IN/OUT>
 *|			  blk - ブロックデータ				<OUT>
 *|			  blk_no - データとして欲しいブロックのブロックNo.
 *| return	: TRUE as success
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_GetBlkData(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk, int blk_no)
{
	if (blk_no < 0 || buffer->blknum <= blk_no) {
		return FALSE;		/* no more block */
	}

	memcpy(blk, &buffer->blkdata[blk_no], sizeof(T_NT_BLKDATA));

	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_BufferClear()
 *[]----------------------------------------------------------------------[]
 *| summary	: バッファクリア
 *| param	: buffer - ブロックデータをクリアするバッファ		<IN/OUT>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_BufferClear(T_NT_BUFFER *buffer)
{
//	buffer->fixed	= FALSE;
	buffer->blknum	= 0;
	buffer->fixed	= FALSE;	/* #004 */
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_SndBufClean()
 *[]----------------------------------------------------------------------[]
 *| summary	: データ保持フラグがOFFの故障端末宛て電文を削除する
 *| param	: buffer - バッファ
 *|			  terminal_failed - 通信異常端末情報
 *|			  terminal - 故障端末No.
 *| return	: TRUE - バッファクリアが行われた
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_SndBufClean(T_NT_BUFFER *buffer, eNT_STS terminal_failed, int terminal)
{
	if (buffer->fixed) {
		if (terminal_failed == NT_ABNORMAL) {
			// 該当端末は通信異常中
			if (buffer->blkdata[0].data[NT_OFS_TERMINAL_NO] == terminal) {
				// バッファ内データが該当端末
				return NTComData_ClrNotBufferingData(buffer);		// #003
			}
		}
	}
	return FALSE;	/* 未確定の電文は最終ブロック受信を待って削除 */
}

// #003->
/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_SndBufClean()
 *[]----------------------------------------------------------------------[]
 *| summary	: 該当バッファ内に含まれるデータのデータ保持フラグがOFFであれば、電文削除
 *| param	: buffer - バッファ
 *| return	: TRUE - バッファクリアが行われた
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_ClrNotBufferingData(T_NT_BUFFER *buffer)
{
	if (buffer->blknum > 0) {
		if (buffer->blkdata[0].data[NT_OFS_BUFFERING_FLAG] == NT_NOT_BUFFERING) {
			NTComData_BufferClear(buffer);
			return TRUE;
		}
	}

	return FALSE;
}
// <-#003

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_ChkBlkNo()
 *[]----------------------------------------------------------------------[]
 *| summary	: ブロックデータのブロックNo.チェック
 *|			  buffer内のデータとblkでブロックNo.が連続しているかをチェックする
 *| param	: buffer - チェックするバッファ
 *|			  blk - バッファの最後に付加される予定のブロックデータ
 *| return	: TRUE - ブロックNo.正常
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_ChkBlkNo(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk)
{
	int		i;
	uchar	prv_no;

	if (buffer->blknum <= 0) {
		if (blk->data[NT_OFS_BLOCK_NO] != 1) {
			return FALSE;
		}
	}
	else {
		prv_no = 0;
		for (i = 0; i < buffer->blknum; i++) {
			if (prv_no + 1 != buffer->blkdata[i].data[NT_OFS_BLOCK_NO]) {
				return FALSE;
			}
			prv_no++;
		}
		if (prv_no + 1 != blk->data[NT_OFS_BLOCK_NO]) {
			return FALSE;
		}
	}

	return TRUE;
}

/* #002-> */
/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_CheckPowerFailure()
 *[]----------------------------------------------------------------------[]
 *| summary	: 停電保証処理
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_CheckPowerFailure(void)
{
	int ofs, cnt, cnt_max;

/* #004-> */
// 各バッファ内データ削除中かをチェック
	/* 通常データバッファ */
	_NTComData_CheckBuffer(&NTCom_SndBuf_Normal);	/* NTネットへの送信用 */
	_NTComData_CheckBuffer(&NTCom_RcvBuf_Normal);	/* NTネットからの受信用 */

	/* 優先データバッファ */
	_NTComData_CheckBuffer(&NTCom_SndBuf_Prior);		/* NTネットへの送信用 */

	_NTComData_CheckBuffer(&NTCom_RcvBuf_Prior);			/* NTネットからの受信用 */
	/* 同報データバッファ */
	_NTComData_CheckBuffer(&NTCom_SndBuf_Broadcast);		/* NTネットへの送信用 */
	_NTComData_CheckBuffer(&NTCom_RcvBuf_Broadcast);		/* NTネットからの受信用 */

// 受信通常データ用のスケジューラをチェック
	if (NTCom_Fukuden.scheduler != NT_FUKUDEN_SCHEDULER_NONE) {
		NTComData_ReconstructScheduler(&NTCom_RcvTeleSchedule, &NTCom_RcvBuf_Normal, &NTCom_Fukuden);
	}

/* <-#004 */

// 受信バッファ⇔受信データ送信スケジューラ 間の不整合をチェック
	// 受信バッファ側から不整合をチェック
	if (NTCom_RcvBuf_Normal.fixed) {
		// 通常電文受信バッファに確定した電文が存在したら｡
		ofs = NTCom_RcvTeleSchedule.read;

		if (NTCom_RcvTeleSchedule.read <= NTCom_RcvTeleSchedule.write) {
			cnt_max = NTCom_RcvTeleSchedule.write - NTCom_RcvTeleSchedule.read;
		} else {
			cnt_max = NTCom_RcvTeleSchedule.write - NTCom_RcvTeleSchedule.read + 1;
		}

		for (cnt = 0; cnt < cnt_max; cnt++) {
			if (NTCom_RcvTeleSchedule.ring[ofs] == 1) {
				// 通常バッファの電文がすでに登録されていたら、次のバッファをチェックする｡
				break;
			}
			ofs = _offset(ofs, 1, _countof(NTCom_RcvTeleSchedule.ring));
		}
		if (cnt >= cnt_max) {
			// 電文登録途中で停電発生のケースへの対応
			// 通信相手から再送されるので、受信前の状態にロールバック
			NTComData_BufferClear(&NTCom_RcvBuf_Normal);
		}
	}

	// 送信スケジュール側から不整合をチェック
	if (NTCom_RcvTeleSchedule.read != NTCom_RcvTeleSchedule.write) {
		// スケジューラーに有効なデータがあれば
//		ofs = _termno2ofs(NTCom_RcvTeleSchedule.ring[NTCom_RcvTeleSchedule.read]);
		if (!NTCom_RcvBuf_Normal.fixed) {
			// 電文削除途中で停電発生のケースへの対応
			// スケジュールされている端末の受信バッファが空だったら、スケジューラからも削除する｡
			NTComData_DeleteRcvTeleSchedule();
		}
	}
}
/* <-#002 */

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_ResetTelegram()
 *[]----------------------------------------------------------------------[]
 *| summary	: 電文情報リセット処理
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_ResetTelegram(T_NT_TELEGRAM *telegram)
{
	telegram->curblk = -1;

	if(telegram->kind == NT_DATA_KIND_PRIOR_SND) {
		if(NTComOpeRecvPriorDataBuf.fixed) {
			NTComOpeRecvPriorDataBuf.fixed = FALSE;
		}
	}
	if(telegram->kind == NT_DATA_KIND_NORMAL_SND) {
		if(NTComOpeRecvNormalDataBuf.fixed) {
			NTComOpeRecvNormalDataBuf.fixed = FALSE;
		}
	}
}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
