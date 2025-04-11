/*[]----------------------------------------------------------------------[]
 *| System      : UT8500
 *| Module      : NT-NET通信モジュール(下位レイヤー)
 *[]----------------------------------------------------------------------[]
 *| Author      : ogura.s
 *| Date        : 2005.08.22
 *| Update      :
 *|		2006-01-17	machida.k	アイドル負荷軽減 (MH544401)
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*			I N C L U D E													*/
/*--------------------------------------------------------------------------*/
#include	<string.h>
#include	<stdio.h>
#include	"system.h"
#include	"prm_tbl.h"
#include	"common.h"
#include	"message.h"
#include	"ntnet.h"

/*--------------------------------------------------------------------------*/
/*			C O N S T A N T S												*/
/*--------------------------------------------------------------------------*/
/* 共通設定パラメータ */
#define	PRM_TERMINAL_NO			CPrmSS[S_NTN][NTPRM_TERMINALNO]		/* 端末番号 */
#define	PRM_RESPONSE_TIME		CPrmSS[S_NTN][NTPRM_RESPONSE_TIME]	/* MAINモジュールからの応答待ち時間 */
#define	PRM_RETRY_COUNT			CPrmSS[S_NTN][NTPRM_RETRY]			/* MAINモジュールのリトライ回数 */

#define	SEND_INTERVAL			500

/* 送信制御用ﾀｲﾏｰ起動 */
#define	START_SNDCTRL_TIMER(t)	Lagtim(NTNETTCBNO, 1, (ushort)((t)/20))
/* 送信制御用ﾀｲﾏｰ解除 */
#define	STOP_SNDCTRL_TIMER()	Lagcan(NTNETTCBNO, 1)
/* 送信制御用ﾀｲﾑｱｳﾄｲﾍﾞﾝﾄ */
#define	TIMEOUT_SNDCTRL			TIMEOUT1

/* ARC制御用ﾀｲﾏｰ起動 */
#define	START_ARCCTRL_TIMER(t)	Lagtim(NTNETTCBNO, 2, (ushort)((t)/20))
/* ARC制御用ﾀｲﾏｰ解除 */
#define	STOP_ARCCTRL_TIMER()	Lagcan(NTNETTCBNO, 2)
/* ARC制御用ﾀｲﾑｱｳﾄｲﾍﾞﾝﾄ */
#define	TIMEOUT_ARCCTRL			TIMEOUT2

#define	CMDID_SNDDATA			'S'		/* 送信データID */
#define	CMDID_SNDRSLT			'r'		/* 送信結果データID */
#define	CMDID_RCVDATA			's'		/* 受信データID */
#define	CMDID_RCVRSLT			'R'		/* 受信結果データID */
#define	CMDID_ERRDATA			'e'		/* エラーデータ */
#define	CMDID_ERRRSLT			'E'		/* エラー結果データ */

/* ARCブロック送受信結果データにおける電文内でのオフセット */
#define	OFS_TERMINAL_NO			1			/* 端末No */
#define	OFS_ARCBLOCK_RESULT_ID	11			/* ARCブロック結果ID */
#define	OFS_RESULT_DATA			12			/* 結果 */

/* ARCブロック送受信結果データのデータ部のサイズ */
#define	ARC_BLOCK_RESULT_SIZE			15

#define	OFS_ARCBLOCK_RESULT_ID2	(5-2)			/* ARCブロック結果ID */
#define	OFS_RESULT_DATA2		(6-2)			/* 結果 */
#define	ARC_BLOCK_RESULT_SIZE2	7

/* ARC送信データの最大サイズ */
#define	ARC_1STBLK_SIZE		498
#define	ARC_2NDBLK_SIZE		500
#define	ARC_TOTAL_SIZE		(ARC_1STBLK_SIZE+ARC_2NDBLK_SIZE)

/* ARCNET管理層用イベント */
enum {
	EVT_NONE = 0,		/* 無し */
	EVT_SND_OK,			/* 送信正常終了 */
	EVT_RCV_NORMAL,		/* NTNET通常受信データ受信 */
	EVT_RCV_FREE,		/* FREEパケットデータ受信 */
	EVT_RCV_ERR,		/* エラーデータ受信 */
	EVT_RCV_BLKRST,		/* ARCブロック送受信結果データ受信イベント */
	EVT_TIMEOUT			/* タイマーイベント */
};

enum {
	_1STBLK = 0,	/* ARC第一ブロック */
	_2NDBLK			/* ARC第二ブロック */
};

/* 送信状態 */
typedef enum {
	SNDSTS_IDLE = 0,		/* 送信待機 */
	SNDSTS_WAIT_SNDBLK1,	/* 第1ﾌﾞﾛｯｸ送信待ち */
	SNDSTS_WAIT_SNDFIN1,	/* 第1ﾌﾞﾛｯｸ送信完了待ち */
	SNDSTS_WAIT_SNDBLK2,	/* 第2ﾌﾞﾛｯｸ送信待ち */
	SNDSTS_WAIT_SNDFIN2,	/* 第2ﾌﾞﾛｯｸ送信完了待ち */
	SNDSTS_WAIT_BLKRST		/* 送信結果データ受信待ち */
} eNTLWR_SNDSTS;

/* 受信状態 */
typedef enum {
	RCVSTS_WAIT_BLK1 = 0,	/* 第1ﾌﾞﾛｯｸ受信待ち */
	RCVSTS_WAIT_BLK2,		/* 第2ﾌﾞﾛｯｸ受信待ち */
	RCVSTS_WAIT_SNDRST		/* 受信結果データ送信待ち */
} eNTLWR_RCVSTS;

/* ARC制御-送信状態 */
typedef enum {
	ARCSTS_SND_IDLE = 0,	/* 待機中 */
	ARCSTS_WAIT_SNDFIN,		/* 送信完了待ち */
	ARCSTS_WAIT_RESND,		/* 再送待ち */
	ARCSTS_WAIT_SNDRST		/* 送信結果受信待ち */
} eNTLWR_ARCSTS;

/*--------------------------------------------------------------------------*/
/*			S T R U C T U R E S												*/
/*--------------------------------------------------------------------------*/
/* ARCNET電文：通常送信データ */
typedef struct {
	uchar	SeqNo;			/* シーケンシャルNo */
	uchar	FuncID;			/* 宛先機能ID */
	uchar	BlkNo;			/* ARCブロックNo */
	uchar	CmdID;			/* データ種別 */
	uchar	Size[2];		/* データサイズ */
	uchar	Oiban;			/* データ追番 */
	uchar	BlkSts;			/* ARCブロックステータス */
	struct {
		uchar	rsv1;			/* 予備 */
		uchar	rsv2;			/* 予備 */
		uchar	data[498];		/* 実際の送信データ */
	} SndData;				/* 送信データ */
} t_ArcSndData;

/* ARCNET電文：送信結果データ */
typedef struct {
	uchar	SeqNo;			/* シーケンシャルNo */
	uchar	FuncID;			/* 宛先機能ID */
	uchar	brank;			/* 空き */
	uchar	CmdID;			/* データ種別 */
	uchar	Oiban;			/* データ追番 */
	uchar	rsv1;			/* 予備 */
	uchar	Result;			/* 結果 */
	uchar	rsv2;			/* 予備 */
} t_ArcSndResult;

/* ARCNET電文：通常受信データ */
typedef struct {
	uchar	SeqNo;			/* シーケンシャルNo */
	uchar	FuncID;			/* 宛先機能ID */
	uchar	BlkNo;			/* ARCブロックNo */
	uchar	CmdID;			/* データ種別 */
	uchar	Size[2];		/* データサイズ */
	uchar	Oiban;			/* データ追番 */
	uchar	BlkSts;			/* ARCブロックステータス */
	struct {
		uchar	NodeID;			/* ノードID */
		uchar	ComPrt;			/* 通信ポート */
		uchar	data[498];		/* 実際の受信データ */
	} RcvData;				/* 受信データ */
} t_ArcRcvData;

/* ARCNET電文：受信結果データ */
typedef struct {
	uchar	SeqNo;			/* シーケンシャルNo */
	uchar	FuncID;			/* 宛先機能ID */
	uchar	brank;			/* 空き */
	uchar	CmdID;			/* データ種別 */
	uchar	Oiban;			/* データ追番 */
	uchar	rsv1;			/* 予備 */
	uchar	Result;			/* 結果 */
	uchar	rsv2;			/* 予備 */
} t_ArcRcvResult;

/* ARCNET電文：エラーデータ */
typedef struct {
	uchar	SeqNo;			/* シーケンシャルNo */
	uchar	FuncID;			/* 宛先機能ID */
	uchar	brank;			/* 空き */
	uchar	CmdID;			/* データ種別 */
	uchar	Oiban;			/* データ追番 */
	uchar	TarmNo;			/* ターミナルNo */
	uchar	Module;			/* モジュールコード */
	uchar	Err;			/* エラーコード */
	uchar	OnOff;			/* 発生／解除 */
	uchar	rsv[7];			/* 予備 */
} t_ArcErrData;

/* ARCNET電文：エラー結果データ */
typedef struct {
	uchar	SeqNo;			/* シーケンシャルNo */
	uchar	FuncID;			/* 宛先機能ID */
	uchar	brank;			/* 空き */
	uchar	CmdID;			/* データ種別 */
	uchar	Oiban;			/* データ追番 */
	uchar	rsv1;			/* 予備 */
	uchar	Rrsult;			/* 結果 */
	uchar	rsv2;			/* 予備 */
} t_ArcErrResult;

/* 送信制御構造体 */
typedef struct {
	eNTLWR_SNDSTS		status;			/* 送信状態 */
	uchar				retry;			/* リトライ回数 */
	eNTNET_DATA_KIND	kind;			/* 送信種別 */
	eNTNET_SRSLT		result;			/* 送信結果 */
	uchar				buff[2][508];	/* 送信バッファ */
	ushort				size[2];		/* 送信バッファ格納済みサイズ */
} t_SndCtrl;

/* 受信制御構造体 */
typedef struct {
	eNTLWR_RCVSTS		status;			/* 受信状態 */
	uchar				retry;			/* リトライ回数 */
	uchar				buff[1000];		/* 受信NTパケット記憶用バッファ */
	ushort				size;			/* 受信バッファ格納済みサイズ */
	uchar				blk_rslt[23];	/* ARCﾌﾞﾛｯｸ受信結果データ送信用バッファ */
	ushort				rslt_size;		/* ARCﾌﾞﾛｯｸ受信結果データ送信用バッファデータ長 */
} t_RcvCtrl;

/* ARC制御構造体 */
typedef struct {
	eNTLWR_ARCSTS		status;		/* 送信状態 */
	uchar				*snd_data;	/* 送信データ */
	ushort				snd_size;	/* 送信データサイズ */
	uchar				snd_oiban;	/* 送信データ追い番 */
	uchar				rcv_oiban;	/* 受信データ追い番 */
	uchar				err_oiban;	/* エラーデータ追い番 */
} t_ArcCtrl;


/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S												*/
/*--------------------------------------------------------------------------*/
/* NT下位レイヤー制御データ */
static struct {
	t_ArcCtrl	arc_ctrl;			/* ARC制御データ */
	t_SndCtrl	snd_ctrl;			/* 送信処理制御データ */
	t_RcvCtrl	rcv_ctrl;			/* 受信処理制御データ */
} NtLwrCtrl;

/* NT下位レイヤー ディスパッチフラグ */
BOOL	NtLwr_Dispatch;

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/
static void		ntlwr_rcv(uchar event, ulong msgprm);
static void		ntlwr_snd(uchar event, ulong msgprm);
static void		make_arcblk_rrslt(uchar	rslt);

static uchar	ntlwr_arc(ushort event, ulong msgprm);
static BOOL		send_arc_packet(uchar *pdata, ushort size);

static BOOL		ntlwr_MakeDispatchFlag(void);

static BOOL		ntlwr_SendRemoteData(uchar *data, ushort len);
static ushort	ntlwr_Read_Remote_1stBlk(uchar *data, ushort len);

/*--------------------------------------------------------------------------*/
/*			I N T E R F A S E     F U N C T I O N S							*/
/*--------------------------------------------------------------------------*/
/*[]----------------------------------------------------------------------[]
 *|	name	: NTLWR_Init
 *[]----------------------------------------------------------------------[]
 *| summary	: 下位レイヤー初期化処理
 *| param	: void
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTLWR_Init(void)
{
	/* ゼロクリア */
	memset(&NtLwrCtrl, 0, sizeof(NtLwrCtrl));

	NtLwrCtrl.arc_ctrl.status = ARCSTS_SND_IDLE;		/* ARC制御-送信状態 */
	NtLwrCtrl.arc_ctrl.snd_oiban = 0xff;				/* ARC制御-送信データ追い番 */
	NtLwrCtrl.arc_ctrl.rcv_oiban = 0xff;				/* ARC制御-受信データ追い番 */
	NtLwrCtrl.arc_ctrl.err_oiban = 0xff;				/* ARC制御-エラーデータ追い番 */

	NtLwrCtrl.snd_ctrl.status = SNDSTS_IDLE;			/* 送信制御-状態 */

	NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_BLK1;		/* 受信制御-状態 */
	NtLwr_Dispatch = FALSE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NtArc_SendReq
 *[]----------------------------------------------------------------------[]
 *| summary	: データ送信要求
 *| param	: uchar            *data - 送信データ
 *| 		: ushort           len   - 送信データ長
 *| 		: eNTNET_DATA_KIND kind  - 送信データ種別
 *| return	: TRUE  - 要求受付
 *| 		: FALSE - 要求受付不可
*[]----------------------------------------------------------------------[]*/
BOOL	NTLWR_SendReq(uchar *data, ushort len, eNTNET_DATA_KIND kind)
{
	t_ArcSndData	*pdata;
	ushort			size;
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	/* 送信データ設定済み？ */
	if (NtLwrCtrl.snd_ctrl.size[_1STBLK] != 0)
		return FALSE;

	/* 送信電文種別登録 */
	NtLwrCtrl.snd_ctrl.kind = kind;

	/* NT-NET電文 */
	if (kind == NTNET_NTDATA) {
			if (_is_ntnet_remote()) {
				return ntlwr_SendRemoteData(data, len);
			}

		/* データ長確認 */
		if (len > ARC_TOTAL_SIZE)
			return FALSE;

		/* 送信データ登録 */
		if (len > ARC_1STBLK_SIZE) {
			/* ２ブロック目をセットする */
			pdata = (t_ArcSndData *)NtLwrCtrl.snd_ctrl.buff[_2NDBLK];
			size = (ushort)(len - ARC_1STBLK_SIZE);
			memcpy(&pdata->SndData, &data[ARC_1STBLK_SIZE], (size_t)size);
			NtLwrCtrl.snd_ctrl.size[_2NDBLK] = (ushort)(8 + size);	/* ﾍｯﾀﾞ+ﾃﾞｰﾀ */

			/* ヘッダ部作成(ｼｰｹﾝｼｬﾙﾅﾝﾊﾞｰ・宛先機能IDは、ここでは設定しない */
			pdata->BlkNo = 2;
			pdata->CmdID = CMDID_SNDDATA;
			u.us = size;
			pdata->Size[0] = u.uc[0];
			pdata->Size[1] = u.uc[1];
			pdata->BlkSts = NTNET_LAST_BLOCK;

			len = ARC_1STBLK_SIZE;
		}
		/* １ブロック目をセット */
		pdata = (t_ArcSndData *)NtLwrCtrl.snd_ctrl.buff[_1STBLK];
		size = len;
		pdata->SndData.rsv1 = 0;
		pdata->SndData.rsv2 = 0;
		memcpy(pdata->SndData.data, data, (size_t)size);
		NtLwrCtrl.snd_ctrl.size[_1STBLK] = (ushort)(8 + 2 + size);	/* ﾍｯﾀﾞ+予備+ﾃﾞｰﾀ */

		/* ヘッダ部作成(ｼｰｹﾝｼｬﾙNo・宛先機能ID・追番は、ここでは設定しない */
		pdata->BlkNo = 1;
		pdata->CmdID = CMDID_SNDDATA;
		u.us = (ushort)(2 + size);				/* 予備+ﾃﾞｰﾀ */
		pdata->Size[0] = u.uc[0];
		pdata->Size[1] = u.uc[1];
		/* 2ﾌﾞﾛｯｸ目を設定していれば、中間ﾌﾞﾛｯｸ */
		pdata->BlkSts = (NtLwrCtrl.snd_ctrl.size[_2NDBLK]) ? NTNET_NOT_LAST_BLOCK : NTNET_LAST_BLOCK;
	}
	/* FREEデータ(エラーデータを送信することはない) */
	else {	/* if (kind == NTNET_FREEDATA) */
		/* データ長確認 */
		if (len > ARC_2NDBLK_SIZE)
			return FALSE;

		/* １ブロック目をセット */
		pdata = (t_ArcSndData *)NtLwrCtrl.snd_ctrl.buff[_1STBLK];
		size = len;
		memcpy(&pdata->SndData, data, (size_t)size);
		NtLwrCtrl.snd_ctrl.size[_1STBLK] = (ushort)(8 + size);	/* ﾍｯﾀﾞ+ﾃﾞｰﾀ */

		/* ヘッダ部作成(ｼｰｹﾝｼｬﾙNo・宛先機能ID・追番は、ここでは設定しない */
		pdata->BlkNo = 1;
		pdata->CmdID = CMDID_SNDDATA;
		u.us = size;
		pdata->Size[0] = u.uc[0];
		pdata->Size[1] = u.uc[1];
		pdata->BlkSts = NTNET_LAST_BLOCK;
	}
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntlwr_SendRemoteData
 *[]----------------------------------------------------------------------[]
 *| summary	: データフォーマット変更(通常→遠隔)
 *| param	: 
 *| return	: 
 *[]----------------------------------------------------------------------[]*/
static	BOOL	ntlwr_SendRemoteData(uchar *data, ushort len)
{
	t_ArcSndData	*pdata;
	ushort			size;
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	t_NtNetBlk			*ntblk;
	t_RemoteNtNetBlk	*r_blk;
	uchar				*r_data;
	ushort				r_len;

	ntblk = (t_NtNetBlk *)data;
	// ２ブロック目先頭
	r_data = data;
	r_data += sizeof(pdata->SndData);
	r_data += (ushort)(sizeof(t_NtNetBlkHeader) - sizeof(t_RemoteNtNetBlkHeader));
	// フォーマット修正後の長さ
	len -= (ushort)(sizeof(t_NtNetBlkHeader) - sizeof(t_RemoteNtNetBlkHeader));
	r_len = len;

	/* NT-NET電文 */
	/* データ長確認 */
	if (len > (sizeof(pdata->SndData)*2))
		return FALSE;

	/* 送信データ登録 */
	if (len > sizeof(pdata->SndData)) {
		/* ２ブロック目をセットする */
		pdata = (t_ArcSndData *)NtLwrCtrl.snd_ctrl.buff[_2NDBLK];
		size = (ushort)(len - sizeof(pdata->SndData));
		memcpy(&pdata->SndData, r_data, (size_t)size);
		NtLwrCtrl.snd_ctrl.size[_2NDBLK] = (ushort)(8 + size);	/* ﾍｯﾀﾞ+ﾃﾞｰﾀ */

		/* ヘッダ部作成(ｼｰｹﾝｼｬﾙﾅﾝﾊﾞｰ・宛先機能IDは、ここでは設定しない */
		pdata->BlkNo = 2;
		pdata->CmdID = CMDID_SNDDATA;
		u.us = size;
		pdata->Size[0] = u.uc[0];
		pdata->Size[1] = u.uc[1];
		pdata->BlkSts = NTNET_LAST_BLOCK;

		len = sizeof(pdata->SndData);
	}
	/* １ブロック目をセット */
	pdata = (t_ArcSndData *)NtLwrCtrl.snd_ctrl.buff[_1STBLK];
	size = len;
	r_blk = (t_RemoteNtNetBlk*)&pdata->SndData;
	u.us = (ushort)r_len;
	r_blk->header.packet_size[0] = u.uc[0];
	r_blk->header.packet_size[1] = u.uc[1];
	r_blk->header.len_before0cut[0] = ntblk->header.len_before0cut[0];
	r_blk->header.len_before0cut[1] = ntblk->header.len_before0cut[1];
	r_blk->header.reserve = ntblk->header.reserve;
	memcpy(&r_blk->data, &ntblk->data, (size_t)size-sizeof(t_RemoteNtNetBlkHeader));
	NtLwrCtrl.snd_ctrl.size[_1STBLK] = (ushort)(8 + size);	/* ﾍｯﾀﾞ+ﾃﾞｰﾀ */

	/* ヘッダ部作成(ｼｰｹﾝｼｬﾙNo・宛先機能ID・追番は、ここでは設定しない */
	pdata->BlkNo = 1;
	pdata->CmdID = CMDID_SNDDATA;
	u.us = (ushort)(size);				/* ﾃﾞｰﾀ */
	pdata->Size[0] = u.uc[0];
	pdata->Size[1] = u.uc[1];
	/* 2ﾌﾞﾛｯｸ目を設定していれば、中間ﾌﾞﾛｯｸ */
	pdata->BlkSts = (NtLwrCtrl.snd_ctrl.size[_2NDBLK]) ? NTNET_NOT_LAST_BLOCK : NTNET_LAST_BLOCK;
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntlwr_Read_Remote_1stBlk
 *[]----------------------------------------------------------------------[]
 *| summary	: データフォーマット変更(遠隔→通常)
 *| param	: 
 *| return	: 
 *[]----------------------------------------------------------------------[]*/
static	ushort	ntlwr_Read_Remote_1stBlk(uchar *data, ushort len)
{
	t_NtNetBlk			*n_blk;
	t_RemoteNtNetBlk	*r_blk;
	ushort	n_len;

	n_blk = (t_NtNetBlk*)NtLwrCtrl.rcv_ctrl.buff;
	r_blk = (t_RemoteNtNetBlk*)data;
	n_len = len;

	// ヘッダ(upperで参照しているもののみセット)
	n_blk->header.vect = 0;
	n_blk->header.terminal_no = 0;
	n_blk->header.lower_terminal_no1 = 0;
	n_blk->header.lower_terminal_no2 = 0;
	n_blk->header.link_no = 0;
	n_blk->header.status = 0;
	n_blk->header.broadcast_rcvd = 0;
	n_blk->header.len_before0cut[0] = r_blk->header.len_before0cut[0];
	n_blk->header.len_before0cut[1] = r_blk->header.len_before0cut[1];
	n_blk->header.mode = NTNET_NORMAL_DATA;
	n_blk->header.reserve = n_blk->header.reserve;
	n_blk->header.telegram_type = STX;
	n_blk->header.res_detail = 0;
	n_len += (sizeof(t_NtNetBlkHeader) - sizeof(t_RemoteNtNetBlkHeader));

	// データ
	memcpy((uchar*)(&n_blk->data), (uchar*)(&r_blk->data), len - sizeof(t_RemoteNtNetBlkHeader));

	return n_len;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NtArc_IsSendCmplete
 *[]----------------------------------------------------------------------[]
 *| summary	: データ送信結果取得
 *| param	: eNTNET_SRSLT *result - 送信結果格納領域
 *| return	: TRUE  - 送信完了
 *| 		: FALSE - 送信未完了
 *[]----------------------------------------------------------------------[]*/
BOOL	NTLWR_IsSendComplete(eNTNET_SRSLT *result)
{
	/* 送信データあり？ */
	if (NtLwrCtrl.snd_ctrl.size[_1STBLK] != 0)
		return FALSE;
	*result = NtLwrCtrl.snd_ctrl.result;
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTLWR_Main
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET通信モジュール 下位レイヤーメイン処理
 *| param	: MsgBuf *pmsb - 受信メッセージ
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTLWR_Main(MsgBuf *pmsb)
{
	ushort	arc_evt;
	uchar	nt_evt;

	ulong	msgprm;
	
	if (pmsb != NULL) {
		memcpy(&msgprm, pmsb->msg.data, sizeof(msgprm));
		arc_evt	= pmsb->msg.command;
	}
	else {
		msgprm	= 0;
		arc_evt	= 0;
	}

	/* ARC制御 */
	nt_evt = ntlwr_arc(arc_evt, msgprm);

	/* 受信処理 */
	ntlwr_rcv(nt_evt, msgprm);
	/* 送信処理 */
	ntlwr_snd(nt_evt, msgprm);

	NtLwr_Dispatch = ntlwr_MakeDispatchFlag();
}

/*--------------------------------------------------------------------------*/
/*			S T A T I C   F U N C T I O N S									*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*			ARCNET管理層													*/
/*--------------------------------------------------------------------------*/
/*[]----------------------------------------------------------------------[]
 *|	name	: ntlwr_rcv
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信制御処理
 *| param	: uchar event - 受信･送信制御用イベント
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static void	ntlwr_rcv(uchar event, ulong msgprm)
{
	t_ArcRcvData	*prcv;
	t_ArcErrData	*perr;
	eNTNET_RRSLT	ret;
	uchar			rslt;
	ushort	len;

	perr = 0;
	prcv = 0;
	switch (NtLwrCtrl.rcv_ctrl.status) {
/*--- 第１アークブロック受信待ち ---*/
	case RCVSTS_WAIT_BLK1:
/*--- 第２アークブロック受信待ち ---*/
	case RCVSTS_WAIT_BLK2:
		/* NTNET通常受信データ受信 */
		if (event == EVT_RCV_NORMAL) {

			/* ブロック番号確認 */
			if (prcv->BlkNo == 1) {
				/* ブロックステータス確認 */
				if (prcv->BlkSts == NTNET_NOT_LAST_BLOCK) {
					/* 受信データ記憶(NodeID,ComNoは省略する) */
					len = (ushort)_MAKEWORD(prcv->Size);
					if (_is_ntnet_remote()) {
						// 遠隔→通常形式へ変換してコピー
						len	= ntlwr_Read_Remote_1stBlk((uchar*)&prcv->RcvData, len);
					}
					else {
						len -= 2;
						memcpy(NtLwrCtrl.rcv_ctrl.buff, &prcv->RcvData.data, (size_t)(len));
					}
					NtLwrCtrl.rcv_ctrl.size = len;
					/* 第２アークブロック受信待ち状態へ */
					NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_BLK2;
					NtLwrCtrl.rcv_ctrl.retry = 0;
				}
				else if (prcv->BlkSts == NTNET_LAST_BLOCK) {
					/* データ受信完了通知(NodeID,ComNoは省略する) */
					len = (ushort)_MAKEWORD(prcv->Size);
					if (_is_ntnet_remote()) {
						// 遠隔→通常形式へ変換してコピー
						len	= ntlwr_Read_Remote_1stBlk((uchar*)&prcv->RcvData, len);
						ret = NTUPR_SetRcvPkt(NtLwrCtrl.rcv_ctrl.buff, len, NTNET_NTDATA);
					} else {
						ret = NTUPR_SetRcvPkt(prcv->RcvData.data, (ushort)(len-2), NTNET_NTDATA);
					}
					/* Arcブロック受信結果データ作成 */
					if (ret == NTNET_RRSLT_NORMAL) {
						rslt = 0x00;	/* 正常 */
					}
					else if (ret == NTNET_RRSLT_BLKNO_INVALID) {
						rslt = 0x85;	/* NTﾌﾞﾛｯｸNo異常(今回無効) */
					}
					else {	/* if (ret == NTNET_RRSLT_BLKNO_VALID) */
						rslt = 0x86;	/* NTﾌﾞﾛｯｸNo異常(今回有効) */
					}
					make_arcblk_rrslt(rslt);

					/* Arcブロック受信結果データ送信要求 */
					if (send_arc_packet(NtLwrCtrl.rcv_ctrl.blk_rslt,
										NtLwrCtrl.rcv_ctrl.rslt_size) == FALSE) {
						/*  Arcブロック受信結果データ送信待ち状態へ */
						NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_SNDRST;
					}
					else {
						/* 第１アークブロック受信待ち状態へ */
						NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_BLK1;
					}
					NtLwrCtrl.rcv_ctrl.retry = 0;
				}
			}
			else if (prcv->BlkNo == 2) {
				/* ブロックステータス確認 */
				if (prcv->BlkSts == NTNET_LAST_BLOCK) {
					/* 第１アークブロック受信待ち */
					if (NtLwrCtrl.rcv_ctrl.status == RCVSTS_WAIT_BLK1) {
						/* Arcブロック受信結果データ作成 */
						if (NtLwrCtrl.rcv_ctrl.retry < PRM_RETRY_COUNT+3) {
							rslt = 0x81;	/* ARCﾌﾞﾛｯｸNo異常(再送要求) */
							NtLwrCtrl.rcv_ctrl.retry++;
						}
						else {
							rslt = 0x01;	/* ARCﾌﾞﾛｯｸNo異常(再送なし) */
							NtLwrCtrl.rcv_ctrl.retry = 0;
						}
						make_arcblk_rrslt(rslt);
	
						/* Arcブロック受信結果データ送信要求 */
						if (send_arc_packet(NtLwrCtrl.rcv_ctrl.blk_rslt,
											NtLwrCtrl.rcv_ctrl.rslt_size) == FALSE) {
							/*  Arcブロック受信結果データ送信待ち状態へ */
							NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_SNDRST;
						}
					}
					/* 第２アークブロック受信待ち */
					else {	/* if (NtLwrCtrl.rcv_ctrl.status == RCVSTS_WAIT_BLK2) */
						/* データ受信完了通知 */
						memcpy(&NtLwrCtrl.rcv_ctrl.buff[NtLwrCtrl.rcv_ctrl.size], &prcv->RcvData, (size_t)_MAKEWORD(prcv->Size));
						NtLwrCtrl.rcv_ctrl.size += _MAKEWORD(prcv->Size);
						ret = NTUPR_SetRcvPkt(NtLwrCtrl.rcv_ctrl.buff, NtLwrCtrl.rcv_ctrl.size, NTNET_NTDATA);	
						/* Arcブロック受信結果データ作成 */
						if (ret == NTNET_RRSLT_NORMAL) {
							rslt = 0x00;	/* 正常 */
						}
						else if (ret == NTNET_RRSLT_BLKNO_INVALID) {
							rslt = 0x85;	/* NTﾌﾞﾛｯｸNo異常(今回無効) */
						}
						else {	/* if (ret == NTNET_RRSLT_BLKNO_VALID) */
							rslt = 0x86;	/* NTﾌﾞﾛｯｸNo異常(今回有効) */
						}
						make_arcblk_rrslt(rslt);
	
						/* Arcブロック受信結果データ送信要求 */
						if (send_arc_packet(NtLwrCtrl.rcv_ctrl.blk_rslt,
											NtLwrCtrl.rcv_ctrl.rslt_size) == FALSE) {
							/*  Arcブロック受信結果データ送信待ち状態へ */
							NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_SNDRST;
						}
						else {
							/* 第１アークブロック受信待ち状態へ */
							NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_BLK1;
						}
					}
				}
			}
		}
		/* FREEデータ受信 */
		else if (event == EVT_RCV_FREE) {
			/* データ受信完了通知 */
			NTUPR_SetRcvPkt((uchar *)&prcv->RcvData, _MAKEWORD(prcv->Size), NTNET_FREEDATA);
			/* 第１アークブロック受信待ち状態へ */
			NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_BLK1;
			NtLwrCtrl.rcv_ctrl.retry = 0;
		}
		/* エラーデータ受信 */
		else if (event == EVT_RCV_ERR) {
			/* データ受信完了通知 */
			NTUPR_SetRcvPkt(&perr->TarmNo, 11, NTNET_ERRDATA);	/* ﾀｰﾐﾅﾙNoから11バイトを上位に通知する */
			/* 第１アークブロック受信待ち状態へ */
			NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_BLK1;
			NtLwrCtrl.rcv_ctrl.retry = 0;
		}
		break;

/*--- Arcブロック受信結果データ送信待ち ---*/
	default:	/* case RCVSTS_WAIT_SNDRST: */
		/* Arcブロック受信結果データ送信要求 */
		if (send_arc_packet(NtLwrCtrl.rcv_ctrl.blk_rslt, NtLwrCtrl.rcv_ctrl.rslt_size) == TRUE) {
			/* 第１アークブロック受信待ち状態へ */
			NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_BLK1;
		}
		break;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntlwr_snd
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信制御処理
 *| param	: uchar event - 受信･送信制御用イベント
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static void	ntlwr_snd(uchar event, ulong msgprm)
{
	uchar	result;

	result = 0;
	switch (NtLwrCtrl.snd_ctrl.status) {
/*--- 送信アイドル ---*/
	case SNDSTS_IDLE:
		/* 送信データ有り？ */
		if (NtLwrCtrl.snd_ctrl.size[_1STBLK] == 0)
			break;
		NtLwrCtrl.snd_ctrl.retry = 0;
		NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_SNDBLK1;

/*--- 第1ﾌﾞﾛｯｸ-送信待ち ---*/
	case SNDSTS_WAIT_SNDBLK1:
		/* 第１ブロック送信 */
		if (send_arc_packet(NtLwrCtrl.snd_ctrl.buff[_1STBLK], NtLwrCtrl.snd_ctrl.size[_1STBLK]) == TRUE) {
			NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_SNDFIN1;	/* 送信状態=第1ﾌﾞﾛｯｸﾃﾞｰﾀ送信結果待ち */
		}
		break;

/*--- 第1ﾌﾞﾛｯｸ-送信完了待ち ---*/
	case SNDSTS_WAIT_SNDFIN1:
		/* 送信完了イベント */
		if (event == EVT_SND_OK) {
			if (NtLwrCtrl.snd_ctrl.size[_2NDBLK] == 0) {
				/* NTﾃﾞｰﾀ送信時 */
				if (NtLwrCtrl.snd_ctrl.kind == NTNET_NTDATA) {
					START_SNDCTRL_TIMER(PRM_RESPONSE_TIME*1000);	/* ARCﾌﾞﾛｯｸ結果ﾃﾞｰﾀ受信ﾀｲﾏｰ起動 */
					NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_BLKRST;	/* 送信状態=Arcﾌﾞﾛｯｸ受信結果ﾃﾞｰﾀ受信待ち */
				}
				/* FREEﾃﾞｰﾀ送信時 */
				else {	/* if (NtLwrCtrl.snd_ctrl.kind == NTNET_FREEDATA) */
					NtLwrCtrl.snd_ctrl.size[_1STBLK] = 0;				/* 送信データ開放 */
					NtLwrCtrl.snd_ctrl.size[_2NDBLK] = 0;
					NtLwrCtrl.snd_ctrl.result = NTNET_SRSLT_NORMAL;		/* 送信結果=正常 */
					NtLwrCtrl.snd_ctrl.status = SNDSTS_IDLE;			/* 送信状態=アイドル */
				}
			}
			else {
				/* 第２ブロック送信 */
				if (send_arc_packet(NtLwrCtrl.snd_ctrl.buff[_2NDBLK], NtLwrCtrl.snd_ctrl.size[_2NDBLK]) == TRUE) {
					NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_SNDFIN2;	/* 送信状態=第2ﾌﾞﾛｯｸﾃﾞｰﾀ送信完了待ち */
				}
				else {
					NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_SNDBLK2;	/* 送信状態=第2ﾌﾞﾛｯｸﾃﾞｰﾀ送信待ち */
				}
			}
		}
		break;

/*--- 第2ﾌﾞﾛｯｸ-送信待ち ---*/
	case SNDSTS_WAIT_SNDBLK2:
		/* 第２ブロック送信 */
		if (send_arc_packet(NtLwrCtrl.snd_ctrl.buff[_2NDBLK], NtLwrCtrl.snd_ctrl.size[_2NDBLK]) == TRUE) {
			NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_SNDFIN2;	/* 送信状態=第2ﾌﾞﾛｯｸﾃﾞｰﾀ送信完了待ち */
		}
		break;

/*--- 第2ﾌﾞﾛｯｸ-送信完了待ち ---*/
	case SNDSTS_WAIT_SNDFIN2:
		/* 送信完了イベント */
		if (event == EVT_SND_OK) {
			START_SNDCTRL_TIMER(PRM_RESPONSE_TIME*1000);		/* ARCﾌﾞﾛｯｸ結果ﾃﾞｰﾀ受信ﾀｲﾏｰ起動 */
			NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_BLKRST;		/* 送信状態=Arcﾌﾞﾛｯｸ受信結果ﾃﾞｰﾀ受信待ち */
		}
		break;

/*--- Arcﾌﾞﾛｯｸ結果ﾃﾞｰﾀ受信待ち ---*/
	default:	/* case SNDSTS_WAIT_BLKRST: */
		/* Arcﾌﾞﾛｯｸ結果ﾃﾞｰﾀ受信ｲﾍﾞﾝﾄ */
		if (event == EVT_RCV_BLKRST) {
			STOP_SNDCTRL_TIMER();	/* ARCﾌﾞﾛｯｸ結果ﾃﾞｰﾀ受信ﾀｲﾏｰ解除 */
			/* 結果確認 */

			switch (result) {
			case 0x00:	/* 正常 */
			case 0x01:	/* ARCﾌﾞﾛｯｸNo異常-再送なし */
			case 0x86:	/* NTﾌﾞﾛｯｸNo異常-今回有効 */
				NtLwrCtrl.snd_ctrl.size[_1STBLK] = 0;				/* 送信データ開放 */
				NtLwrCtrl.snd_ctrl.size[_2NDBLK] = 0;
				NtLwrCtrl.snd_ctrl.result = NTNET_SRSLT_NORMAL;		/* 送信結果=正常 */
				NtLwrCtrl.snd_ctrl.status = SNDSTS_IDLE;			/* 送信状態=アイドル */
				break;
			case 0x82:	/* ﾊﾞｯﾌｧﾌﾙ */
				NtLwrCtrl.snd_ctrl.size[_1STBLK] = 0;				/* 送信データ開放 */
				NtLwrCtrl.snd_ctrl.size[_2NDBLK] = 0;
				NtLwrCtrl.snd_ctrl.result = NTNET_SRSLT_BUFFULL;	/* 送信結果=バッファフル */
				NtLwrCtrl.snd_ctrl.status = SNDSTS_IDLE;			/* 送信状態=アイドル */
				break;
			case 0x85:	/* NTﾌﾞﾛｯｸNo異常-今回無効 */
				NtLwrCtrl.snd_ctrl.size[_1STBLK] = 0;				/* 送信データ開放 */
				NtLwrCtrl.snd_ctrl.size[_2NDBLK] = 0;
				NtLwrCtrl.snd_ctrl.result = NTNET_SRSLT_BLKNO;		/* 送信結果=NTﾌﾞﾛｯｸNo異常 */
				NtLwrCtrl.snd_ctrl.status = SNDSTS_IDLE;			/* 送信状態=アイドル */
				break;
			case 0x90:	/* データ送信中 */
				NtLwrCtrl.snd_ctrl.size[_1STBLK] = 0;				/* 送信データ開放 */
				NtLwrCtrl.snd_ctrl.size[_2NDBLK] = 0;
				NtLwrCtrl.snd_ctrl.result = NTNET_SRSLT_SENDING;	/* 送信結果=NTﾌﾞﾛｯｸNo異常 */
				NtLwrCtrl.snd_ctrl.status = SNDSTS_IDLE;			/* 送信状態=アイドル */
				break;
			case 0x81:	/* ARCﾌﾞﾛｯｸNo異常-再送要求 */
			default:
				/* リトライ処理 */
				if (NtLwrCtrl.snd_ctrl.retry < PRM_RETRY_COUNT) {
					/* 第1ﾌﾞﾛｯｸﾃﾞｰﾀ再送 */
					if (send_arc_packet(NtLwrCtrl.snd_ctrl.buff[_1STBLK], NtLwrCtrl.snd_ctrl.size[_1STBLK]) == TRUE) {
						NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_SNDFIN1;	/* 送信状態=第1ﾌﾞﾛｯｸﾃﾞｰﾀ送信結果待ち */
					}
					else {
						NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_SNDBLK1;	/* 送信状態=第1ﾌﾞﾛｯｸﾃﾞｰﾀ送信待ち */
					}
					NtLwrCtrl.snd_ctrl.retry++;
				}
				else {
					NtLwrCtrl.snd_ctrl.size[_1STBLK] = 0;				/* 送信データ開放 */
					NtLwrCtrl.snd_ctrl.size[_2NDBLK] = 0;
					NtLwrCtrl.snd_ctrl.result = NTNET_SRSLT_RETRY_OVER;	/* 送信結果=リトライアウト */
					NtLwrCtrl.snd_ctrl.status = SNDSTS_IDLE;			/* 送信状態=アイドル */
				}
				break;
			}
		}
		/* ﾀｲﾑｱｳﾄｲﾍﾞﾝﾄ */
		else if (event == EVT_TIMEOUT){
			/* ARCﾌﾞﾛｯｸ結果待ちﾀｲﾑｱｳﾄ時は第1ﾌﾞﾛｯｸから再送する */
			if (send_arc_packet(NtLwrCtrl.snd_ctrl.buff[_1STBLK], NtLwrCtrl.snd_ctrl.size[_1STBLK]) == TRUE) {
				NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_SNDFIN1;	/* 送信状態=第1ﾌﾞﾛｯｸﾃﾞｰﾀ送信結果待ち */
			}
			else {
				NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_SNDBLK1;	/* 送信状態=第1ﾌﾞﾛｯｸﾃﾞｰﾀ送信待ち */
			}
		}
		break;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: make_arcblk_rrslt
 *[]----------------------------------------------------------------------[]
 *| summary	: Arcブロック受信結果データ作成
 *| param	: uchar rslt - 結果データ
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static void	make_arcblk_rrslt(uchar	rslt)
{
	t_ArcSndData	*psnd;
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	memset(NtLwrCtrl.rcv_ctrl.blk_rslt, 0, sizeof(NtLwrCtrl.rcv_ctrl.blk_rslt));
	psnd = (t_ArcSndData *)NtLwrCtrl.rcv_ctrl.blk_rslt;
	psnd->BlkNo = 1;												/* ARCﾌﾞﾛｯｸNo */
	psnd->CmdID = CMDID_SNDDATA;									/* 宛先機能ID */
	NtLwrCtrl.rcv_ctrl.rslt_size = 8;								// ARCヘッダサイズ
	if (_is_ntnet_remote()) {
		NtLwrCtrl.rcv_ctrl.rslt_size += ARC_BLOCK_RESULT_SIZE2;
		u.us = ARC_BLOCK_RESULT_SIZE2;								/* データサイズ */
		psnd->Size[0] = u.uc[0];
		psnd->Size[1] = u.uc[1];
		psnd->BlkSts = NTNET_LAST_BLOCK;							/* ARCﾌﾞﾛｯｸｽﾃｰﾀｽ */
		psnd->SndData.data[OFS_ARCBLOCK_RESULT_ID2] = 0xff;			/* ARCブロック結果ID */
		psnd->SndData.data[OFS_RESULT_DATA2] = rslt;				/* 結果 */
		return;
	}

	NtLwrCtrl.rcv_ctrl.rslt_size += ARC_BLOCK_RESULT_SIZE;
	u.us = ARC_BLOCK_RESULT_SIZE;									/* データサイズ */
	psnd->Size[0] = u.uc[0];
	psnd->Size[1] = u.uc[1];
	psnd->BlkSts = NTNET_LAST_BLOCK;								/* ARCﾌﾞﾛｯｸｽﾃｰﾀｽ */
	psnd->SndData.data[OFS_TERMINAL_NO] = (uchar)PRM_TERMINAL_NO;	/* 端末No */
	psnd->SndData.data[OFS_ARCBLOCK_RESULT_ID] = 0xff;				/* ARCブロック結果ID */
	psnd->SndData.data[OFS_RESULT_DATA] = rslt;						/* 結果 */
}


/*--------------------------------------------------------------------------*/
/*			ARCNET通信層													*/
/*--------------------------------------------------------------------------*/
/*[]----------------------------------------------------------------------[]
 *|	name	: check_ArcPacket_length
 *[]----------------------------------------------------------------------[]
 *| summary	: ARCショートパケット長調整
 *| param	: prm 受信データ
 *|           expected_len 設定パケット長
 *| return	: チェック結果
 *[]----------------------------------------------------------------------[]*/
int check_ArcPacket_length(ulong prm, ushort expected_len)
{
	return 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntlwr_arc
 *[]----------------------------------------------------------------------[]
 *| summary	: ARC送受信管理
 *| param	: ushort event : メッセージイベント
 *| return	: 受信･送信制御用イベント
 *[]----------------------------------------------------------------------[]*/
static uchar	ntlwr_arc(ushort event, ulong msgprm)
{
	uchar			ret;
	ret = EVT_NONE;
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: send_arc_packet
 *[]----------------------------------------------------------------------[]
 *| summary	: ARCパケット送信要求
 *| param	: uchar  *pdata - 送信データ
 *| 		: ushort size   - 送信データサイズ
 *| return	: TRUE  - 送信可
 *| 		: FALSE - 送信不可
 *[]----------------------------------------------------------------------[]*/
static BOOL	send_arc_packet(uchar *pdata, ushort size)
{
	if (NtLwrCtrl.arc_ctrl.snd_size)
		return FALSE;

	/* 送信データ登録 */
	NtLwrCtrl.arc_ctrl.snd_data = pdata;
	NtLwrCtrl.arc_ctrl.snd_size = size;

	/* 追い番セット */
	if (++NtLwrCtrl.arc_ctrl.snd_oiban > 99)
		NtLwrCtrl.arc_ctrl.snd_oiban = 1;
	((t_ArcSndData *)pdata)->Oiban = NtLwrCtrl.arc_ctrl.snd_oiban;
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntlwr_MakeDispatchFlag
 *[]----------------------------------------------------------------------[]
 *| summary	: NTLWRモジュール ディスパッチ要因チェック
 *| param	: none
 *| return	: TRUE  - 内部ディスパッチ要因あり
 *[]----------------------------------------------------------------------[]*/
BOOL	ntlwr_MakeDispatchFlag(void)
{
	/* ARC制御モジュールをチェック */
	if (NtLwrCtrl.arc_ctrl.status == ARCSTS_SND_IDLE && NtLwrCtrl.arc_ctrl.snd_size > 0) {
		return TRUE;
	}
	/* 受信系統制御モジュールをチェック */
	if (NtLwrCtrl.rcv_ctrl.status == RCVSTS_WAIT_SNDRST) {
		return TRUE;
	}
	/* 送信系統制御モジュールをチェック */
	switch (NtLwrCtrl.snd_ctrl.status) {
	case SNDSTS_WAIT_SNDBLK1:	/* 第1ﾌﾞﾛｯｸ送信待ち */
		/* no break */
	case SNDSTS_WAIT_SNDBLK2:	/* 第2ﾌﾞﾛｯｸ送信待ち */
		return TRUE;
	default:	/* SNDSTS_WAIT_SNDFIN1/SNDSTS_WAIT_SNDFIN2/SNDSTS_WAIT_BLKRST */
		break;
	}
	
	return FALSE;
}
