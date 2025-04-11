/*[]----------------------------------------------------------------------[]
 *| System      : UT8500
 *| Module      : NT-NETバッファ管理モジュール
 *[]----------------------------------------------------------------------[]
 *| Author      : machida kei
 *| Date        : 2005.08.19
 *| Update      :
 *|		2005.09.07	全データクリアIF追加
 *|		2005.09.21	バッファFULL、ニアFULL状態を新設し、その発生/解除をOPEへ通知する GW841901
 *|		2006-01-17	machida.k	アイドル負荷軽減 (MH544401)
 *|		2006-01-19	machida.k	bug fix
 *[]----------------------------------------------------------------------[]
 *| Remark		:
 *|		データの2重登録防止のための停電保証は実装しない
 *|		NT-NETデータ中のシーケンシャルNo.を上位アプリケーションにおいて
 *|		チェックすることで保証するという方針のため
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
#include	"message.h"
#include	"common.h"
#include	"ntnet.h"
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
#include	"ntnet_def.h"
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
#include	"ope_def.h"
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
#include	"rau.h"
#include	"ntcom.h"
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
/*--------------------------------------------------------------------------*/
/*			C O N S T A N T S												*/
/*--------------------------------------------------------------------------*/

/* 送信バッファ識別コード(先頭から優先度順) */
enum {
	/* prior data-> */
	NTNET_SNDBUF_PRIOR,						/* その他の優先データ */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
//	NTNET_SNDBUF_SALE,						/* 精算データ(ID22orID33) */
//	/* <-prior data */
//	
//	/* normal data-> */
//	NTNET_SNDBUF_CAR_OUT,					/* 出庫データテーブル(ID21) */
//	NTNET_SNDBUF_CAR_IN,					/* 入庫データテーブル(ID20) */
//	NTNET_SNDBUF_TTOTAL,					/* T合計集計データ(ID30～38、ID41) */
//	NTNET_SNDBUF_COIN,						/* コイン金庫集計合計データ(ID131) */
//	NTNET_SNDBUF_NOTE,						/* 紙幣金庫集計合計データ(ID133) */
//	NTNET_SNDBUF_ERROR,						/* エラーデータ(ID120) */
//	NTNET_SNDBUF_ALARM,						/* アラームデータ(ID121) */
//	NTNET_SNDBUF_MONEY,						/* 金銭管理データ(ID126) */
//	NTNET_SNDBUF_TURI,						/* 釣銭管理集計データ(ID135) */
//	NTNET_SNDBUF_CAR_COUNT,					/* 駐車台数データ(ID236) */
//	NTNET_SNDBUF_AREA_COUNT,				/* 区画台数・満車データ(ID237) */
//	NTNET_SNDBUF_MONITOR,					/* モニタデータ(ID122) */
//	NTNET_SNDBUF_OPE_MONITOR,				/* 操作モニタデータ(ID123) */
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
	NTNET_SNDBUF_NORMAL,					/* その他の通常データ */
	/* <-normal data */
	
	NTNET_SNDBUF_MAX
};

#define	NTNET_SNDBUF_PRIOR_TOP	NTNET_SNDBUF_PRIOR
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
//#define	NTNET_SNDBUF_PRIOR_END	NTNET_SNDBUF_SALE
//
//#define	NTNET_SNDBUF_NORMAL_TOP	NTNET_SNDBUF_CAR_OUT
#define	NTNET_SNDBUF_PRIOR_END	NTNET_SNDBUF_PRIOR
#define	NTNET_SNDBUF_NORMAL_TOP	NTNET_SNDBUF_NORMAL
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
#define	NTNET_SNDBUF_NORMAL_END	NTNET_SNDBUF_NORMAL

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
#define	NTNET_SNDLOG_MAX	(sizeof(z_NtSndLogInfo)/sizeof(z_NtSndLogInfo[0]))
// GM849100(S) 名鉄協商コールセンター対応（960バイトを超えるため精算データを通常データで送信する）（GT-7700:GM747902参考）
//#define	NTNET_SNDLOG_PRIOR_TOP	0
//#define	NTNET_SNDLOG_PRIOR_END	0
//
//#define	NTNET_SNDLOG_NORMAL_TOP	(NTNET_SNDLOG_PRIOR_END + 1)
#define	NTNET_SNDLOG_NORMAL_TOP		0
// GM849100(E) 名鉄協商コールセンター対応（960バイトを超えるため精算データを通常データで送信する）（GT-7700:GM747902参考）
#define	NTNET_SNDLOG_NORMAL_END	NTNET_SNDLOG_MAX
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

/* 受信バッファ識別コード(先頭から優先度順) */
enum {
	NTNET_RCVBUF_BROADCAST,					/* 同報データ受信バッファ */
	NTNET_RCVBUF_PRIOR,						/* 優先データ受信バッファ */
	NTNET_RCVBUF_NORMAL,					/* 通常データ受信バッファ */
	NTNET_RCVBUF_MAX
};

/* 復電処理種別 */
typedef enum {
	NTBUF_FUKUDEN_NONE,						/* 復電処理無し */
	NTBUF_FUKUDEN_CLR,						/* バッファ初期化中 */
	NTBUF_FUKUDEN_DATAMOVE,					/* バッファ内データ位置移動中 */
}eNTBUF_FUKUDEN;

#define	NTBUF_PRIOR_DATASIZE_MAX		NTNET_BLKDATA_MAX	/* その他の優先データ */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//#define	NTBUF_SALE_DATASIZE_MAX			947					/* 精算データ(ID22orID33) */
//#define	NTBUF_CAR_OUT_DATASIZE_MAX		435					/* 出庫データテーブル(ID21) */
//#define	NTBUF_CAR_IN_DATASIZE_MAX		159					/* 入庫データテーブル(ID20) */
//#define	NTBUF_TTOTAL_DATASIZE_MAX		15591				/* T合計集計データ(ID35) */
//#define	NTBUF_COIN_DATASIZE_MAX			91					/* コイン金庫集計合計データ(ID131) */
//#define	NTBUF_NOTE_DATASIZE_MAX			91					/* 紙幣金庫集計合計データ(ID133) */
//#define	NTBUF_ERROR_DATASIZE_MAX		197					/* エラーデータ(ID120) */
//#define	NTBUF_ALARM_DATASIZE_MAX		197					/* アラームデータ(ID121) */
//#define	NTBUF_MONEY_DATASIZE_MAX		187					/* 金銭管理データ(ID126) */
//#define	NTBUF_CAR_COUNT_DATASIZE_MAX	225					/* 駐車台数データ(ID236) */
//#define	NTBUF_AREA_COUNT_DATASIZE_MAX	545					/* 区画台数・満車データ(ID237) */
//#define	NTBUF_MONITOR_DATASIZE_MAX		197					/* モニタデータ(ID122) */
//#define	NTBUF_OPE_MONITOR_DATASIZE_MAX	194					/* 操作モニタデータ(ID123) */
//#define	NTBUF_NORMAL_DATASIZE_MAX		24960	/* (960×26 */	/* その他の通常データ */
//#define	NTBUF_BROADCAST_DATASIZE_MAX	NTNET_BLKDATA_MAX	/* 同報データ受信バッファ */
//
//#define	NTBUF_NEARFULL_COUNT			6
//#define NTBUF_SALE_MEMPOOL_MAX			180000L
#define	NTBUF_NORMAL_DATASIZE_MAX		24960	/* (960×26 */	/* その他の通常データ */
#define	NTBUF_BROADCAST_DATASIZE_MAX	NTNET_BLKDATA_MAX	/* 同報データ受信バッファ */

#define	NTBUF_NOTSENDTOTALDATA			0xffff
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

/*--------------------------------------------------------------------------*/
/*			S T R U C T U R E S												*/
/*--------------------------------------------------------------------------*/

/* バッファ設定取得関数型 */
typedef	long (*NTBUF_SETTING_FUNC)(void);

/* 復電処理用データ */
typedef struct {
	eNTBUF_FUKUDEN		kind;				/* 復電処理種別 */
	
	/* for NTBUF_FUKUDEN_DATAMOVE */
	struct {
		long	rofs;						/* 移動対象データ先頭オフセット */
		long	count;						/* 移動対象データ数 */
		long	move;						/* 移動する距離(バイト数) */
	}datamove;
}t_NtBufFukuden;

/* バッファ管理 */
typedef struct {
	eNTBUF_SAVE			save;				/* データ保持方法種別 */
	long				wofs;				/* 書き込みオフセット */
	long				rofs;				/* 読み込みオフセット */
	uchar				*buffer;			/* バッファデータ本体 */
	long				buffer_size;		/* バッファ全体サイズ */
	ulong				datasize;			/* バッファに格納される1データの最大長 */
	short				id;					/* バッファに格納されるデータ種別 */
	NTBUF_SETTING_FUNC	setting;			/* バッファFULL時の動作設定取得関数ポインタ */
	t_NtBufFukuden		fukuden;			/* 復電処理用データ */
}t_NtBuf;

/* バッファ実領域 */
typedef struct {
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//	uchar	snd_pay[1];	/* 精算データテーブル(ID22 or 23) */
//	uchar	snd_car_out[1];				/* 出庫データテーブル(ID21) */
//	uchar	snd_car_in[1];				/* 入庫データテーブル(ID20) */
//	uchar	snd_ttotal[1];				/* T合計集計データ(ID30～38、41) */
//	uchar	snd_coin[1];				/* コイン金庫集計合計データ(ID131) */
//	uchar	snd_note[1];				/* 紙幣金庫集計合計データ(ID133) */
//	uchar	snd_err[1];					/* エラーデータ(ID120) */
//	uchar	snd_alarm[1];				/* アラームデータ(ID121) */
//	uchar	snd_money[1];				/* 金銭管理データ(ID126) */
//	uchar	snd_car_count[1];			/* 駐車台数データ(ID236) */
//	uchar	snd_area_count[1];			/* 区画台数データ(ID236) */
//	uchar	snd_monitor[1];				/* モニタデータ(ID122) */
//	uchar	snd_ope_monitor[1];			/* 操作モニタデータ(ID123) */
//	uchar	snd_normal[1];				/* 送信通常データ */
//	uchar	snd_prior[1];				/* 送信優先データ */
//	uchar	rcv_normal[1];				/* 受信通常データ */
//	uchar	rcv_prior[1];				/* 受信優先データ */
//	uchar	rcv_broadcast[1];			/* 受信同報データ */
	uchar	snd_normal[26000];			/* 送信通常データ */
	uchar	snd_prior[10000];			/* 送信優先データ */
	uchar	rcv_normal[26000];			/* 受信通常データ */
	uchar	rcv_prior[10000];			/* 受信優先データ */
	uchar	rcv_broadcast[1000];		/* 受信同報データ */
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
}t_NtBufPool;

/* バッファ初期化情報 */
typedef struct {
	eNTBUF_SAVE			save;				/* データ保持方法種別 */
	uchar				*pool;				/* バッファデータ本体メモリプール */
	ulong				size;				/* メモリプールサイズ */
	ulong				datasize;			/* バッファに格納される1データの最大長 */
	short				id;					/* バッファに格納されるデータ種別 */
	NTBUF_SETTING_FUNC	setting;			/* バッファFULL時の動作設定取得関数ポインタ */
}t_NtBufInfo;

/* バッファ内の各データに付加されるヘッダ情報 */
typedef struct {
	uchar	len[2];							/* データ長 */
	uchar	len_before0cut[2];				/* 0カット前のデータ長 */
	uchar	lower_terminal1;				/* 下位転送用端末No.(1) */
	uchar	lower_terminal2;				/* 下位転送用端末No.(2) */
	uchar	reserve;						/* 予備 */
}t_NtBufDataHeader;

/* NT-NETデータヘッダ情報 */
typedef struct {
	t_NtBufDataHeader	bufdata_header;		/* バッファデータヘッダ */
	t_NtNetDataHeader	ntdata_header;		/* NT-NETデータヘッダ */
}t_NtBufNtDataHeader;

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
///* FREEデータバッファ */
//typedef struct {
//	BOOL	isdata;							/* TRUE=データ有り */
//	uchar	buffer[FREEPKT_DATASIZE];		/* FREEデータ */
//}t_NtFreeBuf;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (backuped)									*/
/*--------------------------------------------------------------------------*/

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//#pragma section		_UNINIT2
#pragma section		_UNINIT4
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

/* バッファ実領域 */
static t_NtBufPool			z_NtBufPool;

/* 送信バッファ */
static t_NtBuf				z_NtSndBuf[NTNET_SNDBUF_MAX];

/* 受信バッファ */
static t_NtBuf				z_NtRcvBuf[NTNET_RCVBUF_MAX];

/* 全データクリア中 */
static BOOL					z_NtAllClr;

/* ニアフル電文受信 */
static ulong				z_NearFullBuff;
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//static ulong				z_PendingBuff;

// 合計ログデータ
static uchar				z_NtBufTotalLog[sizeof(SYUKEI)];
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

#pragma section

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (not backuped)								*/
/*--------------------------------------------------------------------------*/

extern BOOL	z_NTNET_AtInitial;

/* NT-NETデータヘッダ取得用ワーク領域 */
static t_NtBufNtDataHeader	z_NtBufNtDataHeaderWork;

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
///* FREEデータ送信バッファ */
//static t_NtFreeBuf			z_NtSndFreeBuf;
//
///* FREEデータ受信バッファ */
//static t_NtFreeBuf			z_NtRcvFreeBuf;
// 合計データIDリスト
// MH341110(S) A.Iiizumi 2017/12/18 ParkingWeb対応のNT-NETは車室毎集計は不要なため端末間では送信しないように修正 (共通改善№1391)
//static ushort	z_NtBufTotalIDList[] = {30, 31, 32, 33, 34, 35, 36, 41, 0};
static ushort	z_NtBufTotalIDList[9];
// MH341110(E) A.Iiizumi 2017/12/18 ParkingWeb対応のNT-NETは車室毎集計は不要なため端末間では送信しないように修正 (共通改善№1391)

// 合計ログデータ
static ushort	z_NtBufTotalDataIndex = NTBUF_NOTSENDTOTALDATA;

// 送信データ制御 0:優先データ 1:通常データ
t_Ntnet_SendDataCtrl	NTNET_SendCtrl[NTNET_SNDBUF_MAX];

// 優先NTNETデータバッファ
static	uchar	NtSendPriorData[1024];
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

/* 通信障害発生中フラグ */
static BOOL					z_NtCommErr;

/* バッファ情報 */
static t_NtBufState			z_NtBufState;

BOOL	z_NtBuf_SndDataExist;

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
const ushort c_NTBufTotalIDList_Normal[9] = {30, 31, 32, 33, 34, 36, 41, 0};
const ushort c_NTBufTotalIDList_ParkingWeb[9] = {30, 31, 32, 33, 34, 36, 41, 0, 0};
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/

#define	_ntbuf_IsNtData(b)	((b)->wofs != (b)->rofs)

#define	_ntbuf_GetPlaceSetting(s, p)	(((s) / (p)) % 10)

static void		ntbuf_InitNtBuffer(t_NtBuf *buffers, const t_NtBufInfo *bufinfo, int max, uchar clr);

static void		ntbuf_RearrangeNtBuffer(t_NtBuf *buffer);
static void		ntbuf_DiscardNtData(t_NtBuf *buffer, long delete_data_ofs, ushort delete_data_len);
static void		ntbuf_MoveNtData(t_NtBuf *buffer);
static void		ntbuf_ChkRcvBuf(t_NtBuf *buffers);
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//static BOOL		ntbuf_ChkSndBuf(t_NtBuf *buffers, t_NtFreeBuf *freebuf);
static ulong	ntbuf_GetBufCount(t_NtBuf *buffer);
static BOOL		ntbuf_ChkSndBuf(t_NtBuf *buffers);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

static uchar	ntbuf_SetNtData(t_NtBuf *buffer, const void *data, ushort len);
static void		ntbuf_DeleteOldestNtData(t_NtBuf *buffer, long write_len);
static void		ntbuf_DeleteLatestNtData(t_NtBuf *buffer, long write_len);
static void		ntbuf_WriteNtData(t_NtBuf *buffer, const void *data, ushort len, ushort len_0cut);

static ushort	ntbuf_GetNtData0CutLen(const uchar *data, ushort len);
static int		ntbuf_GetSndNtBufCode(const void *data, eNTNET_BUF_KIND buffer);
static int		ntbuf_GetRcvNtBufCode(eNTNET_BUF_KIND buffer);
static long		ntbuf_GetNtBufRemain(t_NtBuf *buffer);
static BOOL		ntbuf_ChkSndNtDataLen(int bufcode, ushort len);
static BOOL		ntbuf_ChkRcvNtDataLen(int bufcode, ushort len);

static long		ntbuf_ReadNtBuf(t_NtBuf *buffer, void *dst, long top, ulong cnt);
static long		ntbuf_WriteNtBuf(t_NtBuf *buffer, long dst_ofs, const void *data, ulong cnt);

static ushort	ntbuf_GetNtData(t_NtBuf *buffer, void *data, uchar f_Expand);

static void		ntbuf_NtDataClr(t_NtBuf *buffer);

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
//static uchar	ntbuf_SetFreeData(t_NtFreeBuf *buffer, const void *data);
//static uchar	ntbuf_GetFreeData(t_NtFreeBuf *buffer, void *dst);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）

static BOOL		ntbuf_ChkBufState(t_NtBuf *buffer);
static BOOL		ntbuf_SetBufState(t_NtBuf *buffer, uchar state);
static uchar	ntbuf_GetBufState(t_NtBuf *buffer);

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
//static long		ntbuf_GetSaleBufSetting(void);
//static long		ntbuf_GetCarInBufSetting(void);
//static long		ntbuf_GetCarOutBufSetting(void);
//static long		ntbuf_GetTTotalBufSetting(void);
//static long		ntbuf_GetCoinBufSetting(void);
//static long		ntbuf_GetNoteBufSetting(void);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
static long		ntbuf_GetDefaultSetting(void);

static void		ntbuf_W2Bcpy(uchar *bdst, ushort wsrc);

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//char ntbuf_IsSendData(uchar id);
//static void ntbuf_SetIBKBuffStopByID(uchar ID, ulong *bit);
ushort	NTBUF_ConvertLogToNTNETData(ushort logType, uchar* pLogData, uchar* pNtnetData);
ushort	NTBUF_ConvertTotalLogToNTNETData(ushort logType, ushort logID, uchar* pLogData, uchar* pNtnetData);
static	void	ntbuf_InitSendDataCtrl(void);
static	void	ntbuf_ClearSendDataCtrl(t_Ntnet_SendDataCtrl* pCtrl);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (const table)									*/
/*--------------------------------------------------------------------------*/
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
static const int	z_NtSndLogInfo[] = {	// ログ化されている送信バッファ(優先順)
	// 優先データ
	eLOG_PAYMENT,			// 精算データ(ID22orID33)
	// 通常データ
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（未使用ログ（入庫、出庫））
//	eLOG_ENTER,				// 入庫データテーブル(ID20)
//// MH364300 GG119A34(S) 改善連絡表No.83対応
//	eLOG_LEAVE,				// 出庫データテーブル(ID21)
//// MH364300 GG119A34(E) 改善連絡表No.83対応
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（未使用ログ（入庫、出庫））
	eLOG_TTOTAL,			// T合計集計データ(ID30～38、ID41)
// 不具合修正(S) K.Onodera 2016/12/26 #1701 端末間通信においてGT合計を実施しても集計データが送信されない
	eLOG_GTTOTAL,			// GT合計集計データ
// 不具合修正(E) K.Onodera 2016/12/26 #1701 端末間通信においてGT合計を実施しても集計データが送信されない
// MH341110(S) A.Iiizumi 2018/02/15 NT-NET端末間通信で駐車場センター形式の場合、釣銭管理合計(ID135)が精算の毎に送信される不具合対応(共通改善№1402)
	eLOG_MNYMNG_SRAM,		// 釣銭管理集計データ(ID135)
// MH341110(E) A.Iiizumi 2018/02/15 NT-NET端末間通信で駐車場センター形式の場合、釣銭管理合計(ID135)が精算の毎に送信される不具合対応(共通改善№1402)
	eLOG_COINBOX,			// コイン金庫集計合計データ(ID131)
	eLOG_NOTEBOX,			// 紙幣金庫集計合計データ(ID133)
	eLOG_ERROR,				// エラーデータ(ID120)
	eLOG_ALARM,				// アラームデータ(ID121)
	eLOG_MONEYMANAGE,		// 金銭管理データ(ID126)
	eLOG_PARKING,			// 駐車台数データ(ID236)
	eLOG_MONITOR,			// モニタデータ(ID122)
	eLOG_OPERATE,			// 操作モニタデータ(ID123)
};
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

/* 送信バッファ初期化用テーブル */
static const t_NtBufInfo	z_NtSndBufInfo[NTNET_SNDBUF_MAX] = {
	/* NTNET_SNDBUF_PRIOR */
	{
		NTBUF_NOT_SAVE,
		z_NtBufPool.snd_prior,
		sizeof(z_NtBufPool.snd_prior),
		NTBUF_PRIOR_DATASIZE_MAX,
		-1,
		ntbuf_GetDefaultSetting
	},
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
//	/* NTNET_SNDBUF_SALE */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_pay,
//		NTBUF_SALE_MEMPOOL_MAX,
//		NTBUF_SALE_DATASIZE_MAX,
//		22,
//		ntbuf_GetSaleBufSetting
//	},
//	/* NTNET_SNDBUF_CAR_OUT */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_car_out,
//		sizeof(z_NtBufPool.snd_car_out),
//		NTBUF_CAR_OUT_DATASIZE_MAX,
//		21,
//		ntbuf_GetCarOutBufSetting
//	},
//	/* NTNET_SNDBUF_CAR_IN */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_car_in,
//		sizeof(z_NtBufPool.snd_car_in),
//		NTBUF_CAR_IN_DATASIZE_MAX,
//		20,
//		ntbuf_GetCarInBufSetting
//	},
//	/* NTNET_SNDBUF_TTOTAL */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_ttotal,
//		sizeof(z_NtBufPool.snd_ttotal),
//		NTBUF_TTOTAL_DATASIZE_MAX,
//		41,
//		ntbuf_GetTTotalBufSetting
//	},
//	/* NTNET_SNDBUF_COIN */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_coin,
//		sizeof(z_NtBufPool.snd_coin),
//		NTBUF_COIN_DATASIZE_MAX,
//		131,
//		ntbuf_GetCoinBufSetting
//	},
//	/* NTNET_SNDBUF_NOTE */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_note,
//		sizeof(z_NtBufPool.snd_note),
//		NTBUF_NOTE_DATASIZE_MAX,
//		133,
//		ntbuf_GetNoteBufSetting
//	},
//	/* NTNET_SNDBUF_ERROR */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_err,
//		sizeof(z_NtBufPool.snd_err),
//		NTBUF_ERROR_DATASIZE_MAX,
//		120,
//		ntbuf_GetDefaultSetting
//	},
//	/* NTNET_SNDBUF_ALARM */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_alarm,
//		sizeof(z_NtBufPool.snd_alarm),
//		NTBUF_ALARM_DATASIZE_MAX,
//		121,
//		ntbuf_GetDefaultSetting
//	},
//	/* NTNET_SNDBUF_MONEY */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_money,
//		sizeof(z_NtBufPool.snd_money),
//		NTBUF_MONEY_DATASIZE_MAX,
//		126,
//		ntbuf_GetDefaultSetting
//	},
//	/* NTNET_SNDBUF_CAR_COUNT */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_car_count,
//		sizeof(z_NtBufPool.snd_car_count),
//		NTBUF_CAR_COUNT_DATASIZE_MAX,
//		236,
//		ntbuf_GetDefaultSetting
//	},
//	/* NTNET_SNDBUF_AREA_COUNT */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_area_count,
//		sizeof(z_NtBufPool.snd_area_count),
//		NTBUF_AREA_COUNT_DATASIZE_MAX,
//		237,
//		ntbuf_GetDefaultSetting
//	},
//	/* NTNET_SNDBUF_MONITOR */
//	{
//		NTBUF_SAVE,		
//		z_NtBufPool.snd_monitor,
//		sizeof(z_NtBufPool.snd_monitor),
//		NTBUF_MONITOR_DATASIZE_MAX,
//		122,
//		ntbuf_GetDefaultSetting
//	},
//	/* NTNET_SNDBUF_OPE_MONITOR */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_ope_monitor,
//		sizeof(z_NtBufPool.snd_ope_monitor),
//		NTBUF_OPE_MONITOR_DATASIZE_MAX,
//		123,
//		ntbuf_GetDefaultSetting
//	},
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
	/* NTNET_SNDBUF_NORMAL */
	{
		NTBUF_NOT_SAVE,	
		z_NtBufPool.snd_normal,
		sizeof(z_NtBufPool.snd_normal),
		NTBUF_NORMAL_DATASIZE_MAX,
		-1,
		ntbuf_GetDefaultSetting
	},
};

/* 受信バッファ初期化用テーブル */
static const t_NtBufInfo	z_NtRcvBufInfo[NTNET_RCVBUF_MAX] = {
	/* NTNET_RCVBUF_BROADCAST */
	{
		NTBUF_SAVE_BY_DATA,	
		z_NtBufPool.rcv_broadcast,	
		sizeof(z_NtBufPool.rcv_broadcast),	
		NTBUF_BROADCAST_DATASIZE_MAX,
		-1,
		ntbuf_GetDefaultSetting
	},
	/* NTNET_RCVBUF_PRIOR */
	{
		NTBUF_SAVE_BY_DATA,
		z_NtBufPool.rcv_prior,
		sizeof(z_NtBufPool.rcv_prior),
		NTBUF_PRIOR_DATASIZE_MAX,
		-1,
		ntbuf_GetDefaultSetting
	},
	/* NTNET_RCVBUF_NORMAL */
	{
		NTBUF_SAVE_BY_DATA,	
		z_NtBufPool.rcv_normal,	
		sizeof(z_NtBufPool.rcv_normal),
		NTBUF_NORMAL_DATASIZE_MAX,
		-1,
		ntbuf_GetDefaultSetting
	},
};

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
//const  long	z_NearFullBitEx[NTNET_SNDBUF_MAX] = {	// バッファ優先度順
//	0x00000000,						// NTNET_SNDBUF_PRIOR,			/* 優先データ一般 */
//	NTNET_BUFCTRL_REQ_SALE,			// NTNET_SNDBUF_SALE,			/* 精算データ(ID22/ID23) */
//	NTNET_BUFCTRL_REQ_OUTCAR,		// NTNET_SNDBUF_CAR_OUT,		/* 出庫データテーブル(ID21) */
//	NTNET_BUFCTRL_REQ_INCAR,		// NTNET_SNDBUF_CAR_IN,			/* 入庫データテーブル(ID20) */
//	NTNET_BUFCTRL_REQ_TTOTAL,		// NTNET_SNDBUF_TTOTAL,			/* T合計集計データ(ID30～38、ID41) */
//	NTNET_BUFCTRL_REQ_COIN,			// NTNET_SNDBUF_COIN,			/* コイン金庫集計合計データ(ID131) */
//	NTNET_BUFCTRL_REQ_NOTE,			// NTNET_SNDBUF_NOTE,			/* 紙幣金庫集計合計データ(ID133) */
//	NTNET_BUFCTRL_REQ_ERROR,		// NTNET_SNDBUF_ERROR,			/* エラーデータ(ID120) */
//	NTNET_BUFCTRL_REQ_ALARM,		// NTNET_SNDBUF_ALARM,			/* アラームデータ(ID121) */
//	NTNET_BUFCTRL_REQ_MONEY,		// NTNET_SNDBUF_MONEY,			/* 金銭管理データ(ID126) */
//	NTNET_BUFCTRL_REQ_TURI, 										/* 釣銭管理集計データ(ID135) */
//	NTNET_BUFCTRL_REQ_CAR_COUNT,	// NTNET_SNDBUF_CAR_COUNT,		/* 駐車台数データ(ID236) */
//	NTNET_BUFCTRL_REQ_AREA_COUNT,	// NTNET_SNDBUF_AREA_COUNT,		/* 区画台数・満車データ(ID237) */
//	NTNET_BUFCTRL_REQ_MONITOR,		// NTNET_SNDBUF_MONITOR,		/* モニタデータ(ID122) */
//	NTNET_BUFCTRL_REQ_OPE_MONITOR,	// NTNET_SNDBUF_OPE_MONITOR,	/* 操作モニタデータ(ID123) */
//	0x00000000,						// NTNET_SNDBUF_NORMAL,			/* 通常データ一般 */
//};
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）

/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/

/*====================================================================================[PUBLIC]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_Init
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NETバッファ管理モジュール初期化
 *|			  バッファ内の整合性を保つための停電保証処理も行う
 *| param	: clr - 1=停電保証データもクリア(このとき停電保証処理は行わない)
 *|			        0=停電保証データはクリアしない
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTBUF_Init(uchar clr)
{
	z_NtCommErr = FALSE;
	
	if (clr) {
		z_NtAllClr = FALSE;
	}
	else {
		if (z_NtAllClr) {
			/* NTBUF_AllClr()処理中に停電のため、全データクリア処理を補完 */
			clr = 1;
		}
	}
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
//	if (clr || (!_is_ntnet_remote())) {
//		z_NearFullBuff = 0;
//	}
//	z_PendingBuff = 0;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）

	/* バッファ初期化 */
	ntbuf_InitNtBuffer(z_NtSndBuf, z_NtSndBufInfo, NTNET_SNDBUF_MAX, clr);	/* 送信バッファ */
	ntbuf_InitNtBuffer(z_NtRcvBuf, z_NtRcvBufInfo, NTNET_RCVBUF_MAX, clr);	/* 受信バッファ */
	
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//	z_NtSndFreeBuf.isdata = FALSE;											/* FREEデータ送信バッファ */
//	z_NtRcvFreeBuf.isdata = FALSE;											/* FREEデータ受信バッファ */
	// 送信制御データの初期化
	ntbuf_InitSendDataCtrl();									// 送信制御データを初期化
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
	
	z_NtAllClr = FALSE;
	
	/* 受信バッファ内にあるデータ数分、OPEへメッセージ送信 */
	ntbuf_ChkRcvBuf(z_NtRcvBuf);
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//	z_NtBuf_SndDataExist = ntbuf_ChkSndBuf(z_NtSndBuf, &z_NtSndFreeBuf);
	z_NtBuf_SndDataExist = ntbuf_ChkSndBuf(z_NtSndBuf);
// MH341110(S) A.Iiizumi 2017/12/18 ParkingWeb対応のNT-NETは車室毎集計は不要なため端末間では送信しないように修正 (共通改善№1391)
// NOTE:ParkingWeb対応のNT-NETの端末間通信で車室毎集計送信するためには大きなRAM領域が必要になる。
// 仕様書には車室毎集計は定義されているが ParkingWebでは未使用（本データを使用する機器がいない）
// との記載より車室毎集計は送信しない設計とする。
	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) { // 端末間通信：センター仕様
		memcpy( &z_NtBufTotalIDList, &c_NTBufTotalIDList_ParkingWeb, sizeof( z_NtBufTotalIDList ) );
	}
	else {										// 端末間通信：標準仕様
		memcpy( &z_NtBufTotalIDList, &c_NTBufTotalIDList_Normal, sizeof( z_NtBufTotalIDList ) );
	}
// MH341110(E) A.Iiizumi 2017/12/18 ParkingWeb対応のNT-NETは車室毎集計は不要なため端末間では送信しないように修正 (共通改善№1391)
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_AllClr
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NETバッファ内のデータを全て削除
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTBUF_AllClr(void)
{
	z_NtAllClr = TRUE;		/* 停電保証のチェックポイント */
	
	/* バッファ初期化 */
	ntbuf_InitNtBuffer(z_NtSndBuf, z_NtSndBufInfo, NTNET_SNDBUF_MAX, 1);	/* NT-NETデータ送信バッファ */
	ntbuf_InitNtBuffer(z_NtRcvBuf, z_NtRcvBufInfo, NTNET_RCVBUF_MAX, 1);	/* NT-NETデータ受信バッファ */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//	z_NtSndFreeBuf.isdata = FALSE;											/* FREEデータ送信バッファ */
//	z_NtRcvFreeBuf.isdata = FALSE;											/* FREEデータ受信バッファ */
//	
//	NTUPR_Init2();															/* Upperﾓｼﾞｭｰﾙ送受信ﾌｪｰｽﾞとﾊﾞｯﾌｧｸﾘｱ */

	// ログ初期化
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（未使用ログ（入庫、出庫））
//	Ope_Log_UnreadToRead(eLOG_ENTER, eLOG_TARGET_NTNET);		// 入庫
//// MH364300 GG119A34(S) 改善連絡表No.83対応
//	Ope_Log_UnreadToRead(eLOG_LEAVE, eLOG_TARGET_NTNET);		// 出庫
//// MH364300 GG119A34(E) 改善連絡表No.83対応
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（未使用ログ（入庫、出庫））
	Ope_Log_UnreadToRead(eLOG_PAYMENT, eLOG_TARGET_NTNET);		// 精算
	Ope_Log_UnreadToRead(eLOG_TTOTAL, eLOG_TARGET_NTNET);		// T合計
	Ope_Log_UnreadToRead(eLOG_ERROR, eLOG_TARGET_NTNET);		// エラー
	Ope_Log_UnreadToRead(eLOG_ALARM, eLOG_TARGET_NTNET);		// アラーム
	Ope_Log_UnreadToRead(eLOG_MONITOR, eLOG_TARGET_NTNET);		// モニタ
	Ope_Log_UnreadToRead(eLOG_OPERATE, eLOG_TARGET_NTNET);		// 操作モニタ
	Ope_Log_UnreadToRead(eLOG_COINBOX, eLOG_TARGET_NTNET);		// コイン金庫集計
	Ope_Log_UnreadToRead(eLOG_NOTEBOX, eLOG_TARGET_NTNET);		// 紙幣金庫集計
	Ope_Log_UnreadToRead(eLOG_PARKING, eLOG_TARGET_NTNET);		// 駐車台数
	Ope_Log_UnreadToRead(eLOG_MONEYMANAGE, eLOG_TARGET_NTNET);	// 金銭管理(SRAM)
	Ope_Log_UnreadToRead(eLOG_MNYMNG_SRAM, eLOG_TARGET_NTNET);	// 釣銭管理
// MH341110(S) A.Iiizumi 2017/12/25 動作チェック-NT-NETチェックのデータクリア処理でGT合計を初期化しない不具合修正 (共通改善№1396)
	Ope_Log_UnreadToRead(eLOG_GTTOTAL, eLOG_TARGET_NTNET);		// GT合計
// MH341110(E) A.Iiizumi 2017/12/25 動作チェック-NT-NETチェックのデータクリア処理でGT合計を初期化しない不具合修正 (共通改善№1396)

	// 送信中集計データIDインデックスクリア
	z_NtBufTotalDataIndex = NTBUF_NOTSENDTOTALDATA;
	
	// 送信制御データにあるリトライデータをクリア
	ntbuf_InitSendDataCtrl();									// 送信制御データを初期化
	
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

	z_NtAllClr = FALSE;		/* 停電保証処理完了のチェックポイント */
}
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_AllClr_startup
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NETバッファ内のデータを全て削除(起動時用)
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTBUF_AllClr_startup(void)
{
	z_NtAllClr = TRUE;		/* 停電保証のチェックポイント */
	
	/* バッファ初期化 */
	ntbuf_InitNtBuffer(z_NtSndBuf, z_NtSndBufInfo, NTNET_SNDBUF_MAX, 1);	/* NT-NETデータ送信バッファ */
	ntbuf_InitNtBuffer(z_NtRcvBuf, z_NtRcvBufInfo, NTNET_RCVBUF_MAX, 1);	/* NT-NETデータ受信バッファ */
	// NOTE：起動時はログを送信済みにする必要がないためNTBUF_AllClr()のようなログ送信済み処理は実施しない
	//       ログの初期化が必要な場合別ロジックで実施される

	// 送信中集計データIDインデックスクリア
	z_NtBufTotalDataIndex = NTBUF_NOTSENDTOTALDATA;
	
	// 送信制御データにあるリトライデータをクリア
	ntbuf_InitSendDataCtrl();									// 送信制御データを初期化
	z_NtAllClr = FALSE;		/* 停電保証処理完了のチェックポイント */
}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_SetSendNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信NTNETデータをバッファに書込む
 *| param	: data - 書き込み対象となるデータ					<IN>
 *|			  len - dataのバイト長
 *|			  bufkind - バッファ種別
 *|					   (NTNET_BUF_BUFFERING/NTNET_BUF_PRIOR/NTNET_BUF_NORMAL)
 *|	return	: NTNET_BUFSET_NORMAL=正常終了
 *|			  NTNET_BUFSET_DEL_OLD=最古データを消去
 *|			  NTNET_BUFSET_DEL_NEW=最新データを消去
 *|			  NTNET_BUFSET_CANT_DEL=バッファFULLだが設定が"休業"のため消去不可
 *|			  NTNET_BUFSET_COMERR=IBKで通信障害発生中のためセットしない
 *|			  NTNET_BUFSET_STATE_CHG=バッファ状態変化(書込みは完了)
 *[]----------------------------------------------------------------------[]
 *| remark	:
 *|		dataが指し示すデータイメージのフォーマットは次の通り。
 *|			『NT-NET 通信フォーマット仕様書』の「5.データフォーマット詳細」に
 *|			記載されたデータフォーマット。(0カット等の圧縮を行っていないもの)
 *|			データ項目中の「シーケンスNo.」についてはNT-NETタスクからの送信時に
 *|			作成するので、不定でよい。
 *[]----------------------------------------------------------------------[]*/
uchar	NTBUF_SetSendNtData(const void *data, ushort len, eNTNET_BUF_KIND bufkind)
{
	int		bufcode;
	uchar	ret;
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
//	uchar *dt;
//
//	return	NTNET_BUFSET_NORMAL;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）

	/* NT-NET通信無し時はここで停止する */
	if( _not_ntnet() ){		/* NT-NET通信無し */
		return	NTNET_BUFSET_NORMAL;
	}

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
//	// 送信不要ならセットせずに終了
//	dt = (uchar*)data;
//	if (dt[0] != REMOTE_SYSTEM) {
//		if (ntbuf_IsSendData(dt[1]) == 0) {
//			return NTNET_BUFSET_NORMAL;
//		}
//	}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）

	/* 該当バッファを取得 */
	bufcode = ntbuf_GetSndNtBufCode(data, bufkind);
	if (bufcode < 0 || NTNET_SNDBUF_MAX <= bufcode) {
		return NTNET_BUFSET_NORMAL;		/* このパスは通らないはず */
	}
	/* データ長チェック */
	if (!ntbuf_ChkSndNtDataLen(bufcode, len)) {
		return NTNET_BUFSET_NORMAL;		/* このパスは通らないはず */
	}
	/* 通信障害状態をチェック */
	if (z_NtCommErr) {
		if (z_NtSndBuf[bufcode].save == NTBUF_NOT_SAVE) {
			return NTNET_BUFSET_COMERR;	/* 通信障害中のためセットしない */
		}
	}
	/* バッファ状態をチェック */
	if (z_NtSndBuf[bufcode].setting() == NTNET_BUFSET_CANT_DEL) {
		if (ntbuf_GetBufState(&z_NtSndBuf[bufcode]) & NTBUF_BUFFER_FULL) {
			return NTNET_BUFSET_CANT_DEL;	/* バッファFULL時の動作設定が「休業」かつ、バッファFULL */
		}
	}
	/* バッファにデータをセット */
	ret = ntbuf_SetNtData(&z_NtSndBuf[bufcode], data, len);
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
//	if ((ret != NTNET_BUFSET_CANT_DEL) &&
//		((z_NearFullBitEx[bufcode] & z_NearFullBuff) == 0) &&
//		((z_NearFullBitEx[bufcode] & z_PendingBuff) == 0)) {
	if (ret != NTNET_BUFSET_CANT_DEL) {
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
		z_NtBuf_SndDataExist = TRUE;
	}
	if (ret == NTNET_BUFSET_NORMAL) {
		if (ntbuf_ChkBufState(&z_NtSndBuf[bufcode])) {
			ret = NTNET_BUFSET_STATE_CHG;
		}
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_GetRcvNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信NTNETデータの取得
 *| param	: data - データをコピー先となるワークバッファ	<OUT>
 *|			  bufkind - バッファ種別
 *|					   (NTNET_BUF_BROADCAST/NTNET_BUF_PRIOR/NTNET_BUF_NORMAL)
 *| return	: 引数dataにコピーしたバイト数
 *|			  0＝指定バッファにデータなし
 *[]----------------------------------------------------------------------[]*/
ushort	NTBUF_GetRcvNtData(void *data, eNTNET_BUF_KIND bufkind)
{
	ushort	ret;
	int		bufcode;
	
	ret = 0;
	
	/* 該当バッファを取得 */
	bufcode = ntbuf_GetRcvNtBufCode(bufkind);
	if (0 <= bufcode && bufcode < NTNET_RCVBUF_MAX) {
		ret = ntbuf_GetNtData(&z_NtRcvBuf[bufcode], data, 1);	// 0ｶｯﾄ分を伸張してGet
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_ClrRcvNtData_Prepare
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信データ1件をクリア(チェックポイント処理)
 *| param	: bufkind - データを削除するバッファ
 *|					   (NTNET_BUF_BROADCAST/NTNET_BUF_PRIOR/NTNET_BUF_NORMAL)
 *|			  handle - データ削除ハンドル					<OUT>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTBUF_ClrRcvNtData_Prepare(eNTNET_BUF_KIND bufkind, t_NtNet_ClrHandle *handle)
{
	int bufcode = ntbuf_GetRcvNtBufCode(bufkind);
	
	if (0 <= bufcode && bufcode < NTNET_RCVBUF_MAX) {
		handle->bufcode	= bufcode;
		handle->rofs	= z_NtRcvBuf[bufcode].rofs;
	}
	else {
		handle->bufcode = NTNET_RCVBUF_MAX;		/* このパスは通らないはず */
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_ClrRcvNtData_Exec
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信データ1件をクリア(クリア実行処理)
 *| param	: handle - データ削除ハンドル					<IN>
 *|					   (NTBUF_ClrRcvData_Check()で作成されたものであること)
 *| return	: none
 *[]----------------------------------------------------------------------[]
 *|	remark	:
 *|		停電発生時におけるデータの2重削除防止のため、本関数はNTBUF_ClrRcvData_Prepareと
 *|		セットで使用する。以下は使用例。
 *|		【通常時】
 *|			// 停電保証データ->
 *|			t_NtNet_ClrHandle	h;
 *|			uchar				phase = 0;
 *|			// <-停電保証データ
 *|			
 *|			// (※１)
 *|			
 *|			phase = 1;		// --------------------- 削除チェックポイント開始
 *|			// (※２)
 *|			NTBUF_ClrRcvData_Prepare(NTNET_BUF_PRIOR, &h);
 *|			
 *|			phase = 2;		// --------------------- 削除開始
 *|			// (※３)
 *|			NTBUF_ClrRcvData_Exec(&h)
 *|			
 *|			phase = 3;		// --------------------- 削除完了
 *|		
 *|		【停電復帰時、phase=0】
 *|			上記"※１"地点から処理
 *|		
 *|		【停電復帰時、phase=1】
 *|			上記"※２"地点から処理
 *|		
 *|		【停電復帰時、phase=2】
 *|			上記"※３"地点から処理
 *|		
 *|		【停電復帰時、phase=3】
 *|			復電処理なし
 *|		
 *[]----------------------------------------------------------------------[]*/
void	NTBUF_ClrRcvNtData_Exec(const t_NtNet_ClrHandle *handle)
{
	if (0 <= handle->bufcode && handle->bufcode < NTNET_RCVBUF_MAX) {
		if (z_NtRcvBuf[handle->bufcode].rofs == handle->rofs) {
			ntbuf_NtDataClr(&z_NtRcvBuf[handle->bufcode]);
		}
		else {
			/* 削除済み */
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_CommErr
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET通信異常発生/解除通知
 *| param	: sts - IBKからのエラーデータ"01H"を受け取ったときの発生(01H)/解除(00H)
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTBUF_CommErr(uchar sts)
{
	int i;
	
	if (sts == 1) {
		/* 送信NT-NETデータのうち、データ保持フラグがOFFのバッファを全てクリア */
		z_NtCommErr = TRUE;
		for (i = 0; i < _countof(z_NtSndBuf); i++) {
			if (z_NtSndBuf[i].save == NTBUF_NOT_SAVE) {
				z_NtSndBuf[i].rofs = 
				z_NtSndBuf[i].wofs = 0;
			}
		}
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//		z_NtBuf_SndDataExist = ntbuf_ChkSndBuf(z_NtSndBuf, &z_NtSndFreeBuf);
		z_NtBuf_SndDataExist = ntbuf_ChkSndBuf(z_NtSndBuf);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
	}
	else {	/* sts == 0 */
		z_NtCommErr = FALSE;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_SetRcvNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信NTNETデータをバッファに書込む
 *| param	: data - 書き込み対象となるデータ					<IN>
 *|			  len - dataのバイト長
 *|			  bufkind - バッファ種別
 *|					   (NTNET_BUF_BROADCAST/NTNET_BUF_PRIOR/NTNET_BUF_NORMAL)
 *|	return	: NTNET_BUFSET_NORMAL=正常終了
 *|			  NTNET_BUFSET_DEL_OLD=最古データを消去
 *[]----------------------------------------------------------------------[]*/
uchar	NTBUF_SetRcvNtData(const void *data, ushort len, eNTNET_BUF_KIND bufkind)
{
	uchar	ret;
	int		bufcode;
	
	/* 該当バッファを取得 */
	bufcode = ntbuf_GetRcvNtBufCode(bufkind);
	if (bufcode < 0 || NTNET_RCVBUF_MAX <= bufcode) {
		return NTNET_BUFSET_NORMAL;		/* このパスは通らないはず */
	}
	/* データ長チェック */
	if (!ntbuf_ChkRcvNtDataLen(bufcode, len)) {
		return NTNET_BUFSET_NORMAL;		/* このパスは通らないはず */
	}
	/* バッファにデータをセット */
	ret = ntbuf_SetNtData(&z_NtRcvBuf[bufcode], data, len);
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
	if(ret == NTNET_BUFSET_DEL_NEW || ret == NTNET_BUFSET_DEL_OLD) {	// バッファフルのためデータ削除
		switch(bufkind) {
		case NTNET_BUF_PRIOR:
			NTNET_Err(ERR_NTNET_PRIOR_RCVBUF, NTERR_ONESHOT);		// 優先
			break;
		case NTNET_BUF_NORMAL:
			NTNET_Err(ERR_NTNET_NORMAL_RCVBUF, NTERR_ONESHOT);		// 通常
			break;
		case NTNET_BUF_BROADCAST:
			NTNET_Err(ERR_NTNET_BROADCAST_RCVBUF, NTERR_ONESHOT);	// 同報
			break;
		default:
			break;
		}
	}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

	/* OPEへNTNETデータ受信を通知 */
	queset(OPETCBNO, (ushort)IBK_NTNET_DAT_REC, 0, NULL);
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_GetSndNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信NTNETデータをバッファから取得
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
// *| param	: data - 送信NT-NETデータコピー先
 *| param	: pCtrl - 送信制御データ
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
 *|			  bufkind - バッファ種別
 *|					   (NTNET_BUF_PRIOR/NTNET_BUF_NORMAL)
 *| return	: 引数dataにコピーしたバイト数
 *|			  0＝指定バッファにデータなし
 *[]----------------------------------------------------------------------[]*/
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//ushort	NTBUF_GetSndNtData(void *data, eNTNET_BUF_KIND bufkind, ushort *UpperQueKind)
ushort	NTBUF_GetSndNtData(t_Ntnet_SendDataCtrl *pCtrl, eNTNET_BUF_KIND bufkind)
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
{
	int		i;
	ushort	ret;
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//	t_NtBuf	*buffer;
	ret = 0;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
	
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
//	buffer = NULL;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
	switch (bufkind) {
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//	case NTNET_BUF_PRIOR:
//		/* 優先度順にデータのあるバッファを検索 */
//		for (i = NTNET_SNDBUF_PRIOR_TOP; i <= NTNET_SNDBUF_PRIOR_END; i++) {
//			if (_ntbuf_IsNtData(&z_NtSndBuf[i])) {
//			// 指定のバッファがニアフル状態(IBK)なら読み出さずに次のバッファをサーチ
//				if ((z_NearFullBitEx[i] & z_NearFullBuff) ||
//					(z_NearFullBitEx[i] & z_PendingBuff))
//					continue;
//				buffer	= &z_NtSndBuf[i];
//				*UpperQueKind = (ushort)i;
//				break;
//			}
//		}
	case NTNET_BUF_PRIOR:	// バッファリングデータではない優先データ
		// 優先度順にデータのあるバッファを検索
		// 優先データバッファ
		if (_ntbuf_IsNtData(&z_NtSndBuf[NTNET_SNDBUF_PRIOR])) {
			ret = ntbuf_GetNtData(&z_NtSndBuf[NTNET_SNDBUF_PRIOR], pCtrl->data, 1);	// 送信バッファ内のデータ1件をゼロカット伸張してコピーする。
			ntbuf_NtDataClr(&z_NtSndBuf[NTNET_SNDBUF_PRIOR]);	// データをバッファから削除
			pCtrl->logid = eLOG_MAX;							// ログデータではない
			break;
		}
		
// GM849100(S) 名鉄協商コールセンター対応（960バイトを超えるため精算データを通常データで送信する）（GT-7700:GM747902参考）
//		// 精算データログ
//		if (Ope_Log_UnreadCountGet(eLOG_PAYMENT, eLOG_TARGET_NTNET)) {			// NT-NET未読精算LOGあり
//				// データを未送信状態のまま取得
//			if (Ope_Log_TargetDataVoidRead(eLOG_PAYMENT, RAU_LogData, eLOG_TARGET_NTNET, TRUE)) {	
//				// ログデータをNTNETデータに変換する
//				ret = NTNET_ConvertLogToNTNETData(eLOG_PAYMENT, RAU_LogData, pCtrl->data);
//// MH364300 GG119A23(S) // GG122600(S) 直前取引内容ログが登録されるとリセットを繰り返す
//				if (ret == 0) {								// 空読み対象なら送信済にする
//					Ope_Log_TargetVoidReadPointerUpdate(eLOG_PAYMENT, eLOG_TARGET_NTNET);
//					ret = 0;								// 送信対象データなし
//				}
//// MH364300 GG119A23(E) // GG122600(E) 直前取引内容ログが登録されるとリセットを繰り返す
//				pCtrl->logid = eLOG_PAYMENT;
//			}
//		}
// GM849100(E) 名鉄協商コールセンター対応（960バイトを超えるため精算データを通常データで送信する）（GT-7700:GM747902参考）
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
		break;
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//	case NTNET_BUF_NORMAL:
//		/* 優先度順にデータのあるバッファを検索 */
//		for (i = NTNET_SNDBUF_NORMAL_TOP; i <= NTNET_SNDBUF_NORMAL_END; i++) {
//			if (_ntbuf_IsNtData(&z_NtSndBuf[i])) {
//			// 指定のバッファがニアフル状態(IBK)なら読み出さずに次のバッファをサーチ
//				if ((z_NearFullBitEx[i] & z_NearFullBuff) ||
//					(z_NearFullBitEx[i] & z_PendingBuff))
//					continue;
//				buffer	= &z_NtSndBuf[i];
//				*UpperQueKind = (ushort)i;
//				break;
//			}
//		}
	case NTNET_BUF_NORMAL:	// バッファリングデータではない通常データ
		// 優先度順にデータのあるバッファを検索
		// ログデータ
		if(z_NtBufTotalDataIndex == NTBUF_NOTSENDTOTALDATA) {			// 集計データ未送信状態
			for (i = NTNET_SNDLOG_NORMAL_TOP; i < NTNET_SNDLOG_NORMAL_END; i++) {
				if(eLOG_TTOTAL != z_NtSndLogInfo[i] && eLOG_GTTOTAL != z_NtSndLogInfo[i]) {	// 集計データ以外
					if (Ope_Log_UnreadCountGet(z_NtSndLogInfo[i], eLOG_TARGET_NTNET)) {	// NT-NET未読精算LOGあり
						// データを未送信状態のまま取得
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（遠隔と端末間通信のバッファを別にする）
//						if (Ope_Log_TargetDataVoidRead(z_NtSndLogInfo[i], RAU_LogData, eLOG_TARGET_NTNET, TRUE)) {
						if (Ope_Log_TargetDataVoidRead(z_NtSndLogInfo[i], NTNET_LogData, eLOG_TARGET_NTNET, TRUE)) {
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（遠隔と端末間通信のバッファを別にする）
							// ログデータをNTNETデータに変換する
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（遠隔と端末間通信のバッファを別にする）
//							ret = NTNET_ConvertLogToNTNETData(z_NtSndLogInfo[i], RAU_LogData, pCtrl->data);
							ret = NTNET_ConvertLogToNTNETData(z_NtSndLogInfo[i], NTNET_LogData, pCtrl->data);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（遠隔と端末間通信のバッファを別にする）
							if(ret == 0) {								// 送信レベル未満等の空読み対象なら送信済にする
								Ope_Log_TargetVoidReadPointerUpdate(z_NtSndLogInfo[i], eLOG_TARGET_NTNET);
								ret = 0;								// 送信対象データなし
							}
							pCtrl->logid = z_NtSndLogInfo[i];
							break;
						}
					}
				}
				else {																	// 集計データ
// MH364300 GG119A14(S) 改善連絡表No.11対応
//// MH364300 GG119A11(S) // GM569000 (S) ParkingWeb端末間（フェーズ２）通信対応
//					if( (prm_get(COM_PRM, S_NTN, 121, 1, 1) == 0) && 
//								(eLOG_GTTOTAL == z_NtSndLogInfo[i]) ) {
//						continue;		// 通常のＮＴＮＥＴの場合、ＧＴ集計（合計）は送信しない
//					}
//// MH364300 GG119A11(E) // GM569000 (E) ParkingWeb端末間（フェーズ２）通信対応
// MH364300 GG119A14(E) 改善連絡表No.11対応
					if (Ope_Log_UnreadCountGet(z_NtSndLogInfo[i], eLOG_TARGET_NTNET)) {	// NT-NET未読精算LOGあり
						// データを未送信状態のまま取得
						if (Ope_Log_TargetDataVoidRead(z_NtSndLogInfo[i], z_NtBufTotalLog, eLOG_TARGET_NTNET, TRUE)) {
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（精算データ、エラーデータのみする）
							if( prm_get(COM_PRM, S_SSS, 1, 1, 1) == 1 ) {		// 名鉄協商仕様NT-NET端末間通信
								Ope_Log_TargetVoidReadPointerUpdate(z_NtSndLogInfo[i], eLOG_TARGET_NTNET);
								z_NtBufTotalDataIndex = NTBUF_NOTSENDTOTALDATA;	// 集計データ未送信状態
								ret = 0;// 送信対象データなし
								break;
							}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（精算データ、エラーデータのみする）
// MH341110(S) A.Iiizumi 2017/12/20 NT-NET端末間通信で既存形式の場合、GT合計を送信する不具合修正 (共通改善№1389)
							if( ( prm_get(COM_PRM, S_NTN, 121, 1, 1) == 0 )&&(eLOG_GTTOTAL == z_NtSndLogInfo[i]) ){
								// 既存形式の場合、GT合計は送信しない仕様なので空読みして送信済にする
								// 送信済みに変更する場合でもOpe_Log_TargetDataVoidRead→Ope_Log_TargetVoidReadPointerUpdateの手順を踏むこと
								Ope_Log_TargetVoidReadPointerUpdate(z_NtSndLogInfo[i], eLOG_TARGET_NTNET);
								z_NtBufTotalDataIndex = NTBUF_NOTSENDTOTALDATA;// 集計データ未送信状態
								ret = 0;// 送信対象データなし
								break;
							}
// MH341110(E) A.Iiizumi 2017/12/20 NT-NET端末間通信で既存形式の場合、GT合計を送信する不具合修正 (共通改善№1389)
							// ログデータをNTNETデータに変換する
							// 集計データはID30～36, 41に分割するため専用のバッファに格納する
							z_NtBufTotalDataIndex = 0;
							pCtrl->logid = z_NtSndLogInfo[i];
							ret = NTNET_ConvertTotalLogToNTNETData(pCtrl->logid,
																		z_NtBufTotalIDList[z_NtBufTotalDataIndex],
																		z_NtBufTotalLog, pCtrl->data);
							++z_NtBufTotalDataIndex;
							break;
						}
					}
				}
			}
		}
		else {															// 集計データ送信中
			if(0 != z_NtBufTotalIDList[z_NtBufTotalDataIndex]) {		// 送信データあり
				ret = NTNET_ConvertTotalLogToNTNETData(pCtrl->logid,
														z_NtBufTotalIDList[z_NtBufTotalDataIndex],
														z_NtBufTotalLog, pCtrl->data);
				++z_NtBufTotalDataIndex;								// 集計データIDインデックス更新
				if(0 == z_NtBufTotalIDList[z_NtBufTotalDataIndex]) {	// 集計終了通知データ処理済
					z_NtBufTotalDataIndex = NTBUF_NOTSENDTOTALDATA;
				}
			}
			else {														// 集計データ送信処理終了
				z_NtBufTotalDataIndex = NTBUF_NOTSENDTOTALDATA;
			}
			break;
		}

		// 通常データバッファ
		if (_ntbuf_IsNtData(&z_NtSndBuf[NTNET_SNDBUF_NORMAL])) {
			ret = ntbuf_GetNtData(&z_NtSndBuf[NTNET_SNDBUF_NORMAL], pCtrl->data, 1);	// 送信バッファ内のデータ1件をゼロカット伸張してコピーする。
			ntbuf_NtDataClr(&z_NtSndBuf[NTNET_SNDBUF_NORMAL]);			// データをバッファから削除
			pCtrl->logid = eLOG_MAX;									// ログデータではない
			break;
		}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
		break;
	default:
		break;
	}
	
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
//	ret = 0;
//	if (buffer != NULL) {
//	/* データコピー */
//		ret = ntbuf_GetNtData(buffer, data, 1);					// 0ｶｯﾄ分を伸張してGet
//																// 送信直前にもう一度0カットする
//	}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
	
	return ret;
}

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
///*[]----------------------------------------------------------------------[]
// *|	name	: NTBUF_ClrSndNtData_Prepare
// *[]----------------------------------------------------------------------[]
// *| summary	: 送信データ1件をクリア(チェックポイント処理)
// *| param	: data - 削除するデータ
// *|			  bufkind - データを削除するバッファ
// *|					   (NTNET_BUF_BUFFERING/NTNET_BUF_PRIOR/NTNET_BUF_NORMAL)
// *|			  handle - データ削除ハンドル					<OUT>
// *| return	: none
// *[]----------------------------------------------------------------------[]*/
//void	NTBUF_ClrSndNtData_Prepare(const void *data, eNTNET_BUF_KIND bufkind, t_NtNet_ClrHandle *handle)
//{
//	int bufcode = ntbuf_GetSndNtBufCode(data, bufkind);
//	
//	if (0 <= bufcode && bufcode < NTNET_SNDBUF_MAX) {
//		handle->bufcode	= bufcode;
//		handle->rofs	= z_NtSndBuf[bufcode].rofs;
//		{
//			ulong	fofs;
//			fofs = (ulong)z_NtSndBuf[bufcode].rofs + _offsetof(t_NtBufDataHeader, reserve);
//			if (fofs >= z_NtSndBuf[bufcode].buffer_size)
//				fofs -= z_NtSndBuf[bufcode].buffer_size;
//			z_NtSndBuf[bufcode].buffer[fofs] = 0xaa;		/* set prepared flag */
//		}
//	}
//	else {
//		handle->bufcode = NTNET_SNDBUF_MAX;		/* このパスは通らないはず */
//	}
//}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_ClrSndNtData_Exec
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信データ1件をクリア(クリア実行処理)
 *| param	: handle - データ削除ハンドル					<IN>
 *|					   (NTBUF_ClrSndData_Check()で作成されたものであること)
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTBUF_ClrSndNtData_Exec(const t_NtNet_ClrHandle *handle)
{
	if (0 <= handle->bufcode && handle->bufcode < NTNET_SNDBUF_MAX) {
		if (z_NtSndBuf[handle->bufcode].rofs == handle->rofs) {
			ulong	fofs;
			fofs = (ulong)z_NtSndBuf[handle->bufcode].rofs + _offsetof(t_NtBufDataHeader, reserve);
			if (fofs >= z_NtSndBuf[handle->bufcode].buffer_size)
				fofs -= z_NtSndBuf[handle->bufcode].buffer_size;
			/* check  prepared flag */
			if (z_NtSndBuf[handle->bufcode].buffer[fofs] != 0xaa)
				return;		/* 削除済み */
			ntbuf_NtDataClr(&z_NtSndBuf[handle->bufcode]);
			/* バッファ状態をチェック */
			if (ntbuf_ChkBufState(&z_NtSndBuf[handle->bufcode])) {
				if (!z_NTNET_AtInitial) {
				/* OPEへバッファ状態変化を通知 */
					queset(OPETCBNO, (ushort)IBK_NTNET_BUFSTATE_CHG, sizeof(z_NtSndBuf[handle->bufcode].id), &z_NtSndBuf[handle->bufcode].id);
				}
			}
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//			z_NtBuf_SndDataExist = ntbuf_ChkSndBuf(z_NtSndBuf, &z_NtSndFreeBuf);
			z_NtBuf_SndDataExist = ntbuf_ChkSndBuf(z_NtSndBuf);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
		}
		else {
			/* 削除済み */
		}
	}
}

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
///*[]----------------------------------------------------------------------[]
// *|	name	: NTBUF_SetSendFreeData
// *[]----------------------------------------------------------------------[]
// *| summary	: 送信FREEデータをバッファに書き込む
// *| param	: data - 書き込み対象となるデータ					<IN>
// *| return	: 1＝正常終了
// *|			  0＝バッファオーバーライト発生
// *[]----------------------------------------------------------------------[]
// *| remark	:
// *|		dataには、送信データ部の500byte＜識別コード("FREE")～FREEDATA＞が
// *|		セットされていること
// *[]----------------------------------------------------------------------[]*/
//uchar	NTBUF_SetSendFreeData(const void *data)
//{
//	uchar ret = ntbuf_SetFreeData(&z_NtSndFreeBuf, data);
//	
//	z_NtBuf_SndDataExist = TRUE;
//	
//	return ret;
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: NTBUF_GetRcvFreeData
// *[]----------------------------------------------------------------------[]
// *| summary	: 受信FREEデータバッファからデータを取得する
// *| param	: data - データコピー先					<OUT>
// *| return	: 1＝正常終了
// *|			  0＝データ無し
// *[]----------------------------------------------------------------------[]
// *| remark	: データコピー完了後、バッファ内のFREEデータはクリアされる
// *[]----------------------------------------------------------------------[]*/
//uchar	NTBUF_GetRcvFreeData(void *data)
//{
//	return ntbuf_GetFreeData(&z_NtRcvFreeBuf, data);
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: NTBUF_SetRcvFreeData
// *[]----------------------------------------------------------------------[]
// *| summary	: 送信FREEデータをバッファに書き込む
// *| param	: data - 書き込み対象となるデータ					<IN>
// *| return	: 1＝正常終了
// *|			  0＝バッファオーバーライト発生
// *[]----------------------------------------------------------------------[]
// *| remark	:
// *|		dataには、送信データ部の500byte＜識別コード("FREE")～FREEDATA＞が
// *|		セットされていること
// *[]----------------------------------------------------------------------[]*/
//uchar	NTBUF_SetRcvFreeData(const void *data)
//{
//	uchar ret = ntbuf_SetFreeData(&z_NtRcvFreeBuf, data);
//	
//	/* OPEへFREEデータ受信を通知 */
//	queset(OPETCBNO, (ushort)IBK_NTNET_FREE_REC, 0, NULL);
//	
//	return ret;
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: NTBUF_GetSndFreeData
// *[]----------------------------------------------------------------------[]
// *| summary	: 受信FREEデータバッファからデータを取得する
// *| param	: data - データコピー先					<OUT>
// *| return	: 1＝正常終了
// *|			  0＝データ無し
// *[]----------------------------------------------------------------------[]
// *| remark	: データコピー完了後、バッファ内のFREEデータはクリアされる
// *[]----------------------------------------------------------------------[]*/
//uchar	NTBUF_GetSndFreeData(void *data)
//{
//
//	uchar ret = ntbuf_GetFreeData(&z_NtSndFreeBuf, data);
//	
//	if (ret) {
//		z_NtBuf_SndDataExist = ntbuf_ChkSndBuf(z_NtSndBuf, &z_NtSndFreeBuf);
//	}
//	
//	return ret;
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: NTBUF_SetRcvErrData
// *[]----------------------------------------------------------------------[]
// *| summary	: 受信したエラーデータをOPEに通知する
// *| param	: data - 受信したエラーデータ
// *|					ターミナルNo.～予備の11バイト
// *| return	: none
// *[]----------------------------------------------------------------------[]*/
//void	NTBUF_SetRcvErrData(const void *data)
//{
//	uchar errdata[10];
//	
//	/* OPEへエラーデータを通知 */
//	memcpy(errdata, &((uchar*)data)[1], sizeof(errdata));		/* CRMは従局固定なのでターミナルNo.は今のところ未使用 */
//	queset(OPETCBNO, (ushort)IBK_NTNET_ERR_REC, sizeof(errdata), errdata);
//}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_GetBufState
 *[]----------------------------------------------------------------------[]
 *| summary	: バッファ状態を返す
 *| param	: none
 *| return	: バッファ状態データ
 *[]----------------------------------------------------------------------[]*/
const t_NtBufState	*NTBUF_GetBufState(void)
{
	return &z_NtBufState;
}


/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_SetIBKNearFull
 *[]----------------------------------------------------------------------[]
 *| summary	: バッファ毎のニアフル状態をセット
 *| param	: 
 *| return	: 
 *[]----------------------------------------------------------------------[]*/
void NTBUF_SetIBKNearFull(ulong mask, ulong occur)
{
	z_NearFullBuff &= ~mask;	// 変化のあった場所を特定
	z_NearFullBuff |= occur;	// 発生/解除の変更
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
//
//	// 送信バッファ状態を更新
//	z_NtBuf_SndDataExist = ntbuf_ChkSndBuf(z_NtSndBuf, &z_NtSndFreeBuf);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
}

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
///*[]----------------------------------------------------------------------[]
// *|	name	: NTBUF_SetIBKNearFull
// *[]----------------------------------------------------------------------[]
// *| summary	: バッファ毎のニアフル状態をセット
// *| param	: 
// *| return	: 
// *[]----------------------------------------------------------------------[]*/
//void NTBUF_SetIBKNearFullByID(uchar ID)
//{
//	ntbuf_SetIBKBuffStopByID(ID, &z_NearFullBuff);
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: NTBUF_SetIBKNearFull
// *[]----------------------------------------------------------------------[]
// *| summary	: バッファ毎のペンディング状態をセット
// *| param	: 
// *| return	: 
// *[]----------------------------------------------------------------------[]*/
//void NTBUF_SetIBKPendingByID(uchar ID)
//{
//	if (ID == 0xff) {
//		// 0xff指定なら全ペンディング解除
//		z_PendingBuff = 0;
//	} else {
//		ntbuf_SetIBKBuffStopByID(ID, &z_PendingBuff);
//	}
//}
//
//void ntbuf_SetIBKBuffStopByID(uchar ID, ulong *bit)
//{
//	// ID情報からビット情報を作成
//	switch (ID) {
//	case 20:		// 入庫データテーブル(ID20)
//	case 54:
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_CAR_IN];
//		break;
//	case 21:		// 出庫データテーブル(ID21)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_CAR_OUT];
//		break;
//	case 22:		// 精算データ(ID22/ID23)
//	case 23:
//	case 56:
//	case 57:
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_SALE];
//		break;
//	case 120:		// エラーデータ(ID120)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_ERROR];
//		break;
//	case 121:		// アラームデータ(ID121)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_ALARM];
//		break;
//	case 122:		// モニタデータ(ID122)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_MONITOR];
//		break;
//	case 123:		// 操作モニタデータ(ID123)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_OPE_MONITOR];
//		break;
//	case 131:		// コイン金庫集計合計データ(ID131)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_COIN];
//		break;
//	case 133:		// 紙幣金庫集計合計データ(ID133)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_NOTE];
//		break;
//	case 236:		// 駐車台数データ(ID236)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_CAR_COUNT];
//		break;
//	case 237:		// 区画台数・満車データ(ID237)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_AREA_COUNT];
//		break;
//	case 126:		// 金銭管理データ(ID126)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_MONEY];
//		break;
//	default:		// T合計集計データ(ID30～38、ID41)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_TTOTAL];
//		break;
//	}
//
//	// 送信バッファ状態を更新
//	z_NtBuf_SndDataExist = ntbuf_ChkSndBuf(z_NtSndBuf, &z_NtSndFreeBuf);
//}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）


/*====================================================================================[PRIVATE]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_InitNtBuffer
 *[]----------------------------------------------------------------------[]
 *| summary	: バッファ初期化
 *| param	: buffers - 初期化対象バッファ
 *|			  bufinfo - バッファ初期化情報テーブル
 *|			  max - バッファ数
 *|			  clr - 1=停電保証データもクリア(このとき停電保証処理は行わない)
 *|			        0=停電保証データはクリアしない
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_InitNtBuffer(t_NtBuf *buffers, const t_NtBufInfo *bufinfo, int max, uchar clr)
{
	int i;
	
	for (i = 0; i < max; i++) {
		buffers[i].save			= bufinfo[i].save;
		buffers[i].buffer		= bufinfo[i].pool;
		buffers[i].buffer_size	= bufinfo[i].size;
		buffers[i].setting		= bufinfo[i].setting;
		buffers[i].datasize		= bufinfo[i].datasize + 3 + sizeof(t_NtBufDataHeader);	/* "+3"=ID3,ID4,データ保持フラグ */
		buffers[i].id			= bufinfo[i].id;
		if (clr) {
			buffers[i].fukuden.kind	= NTBUF_FUKUDEN_CLR;
			buffers[i].wofs = 0;
			buffers[i].rofs = 0;
			buffers[i].fukuden.kind	= NTBUF_FUKUDEN_NONE;
		}
		else {
			/* 復電処理 */
			switch (buffers[i].fukuden.kind) {
			case NTBUF_FUKUDEN_CLR:
				buffers[i].wofs = 0;
				buffers[i].rofs = 0;
				buffers[i].fukuden.kind	= NTBUF_FUKUDEN_NONE;
				break;
			case NTBUF_FUKUDEN_DATAMOVE:
				ntbuf_MoveNtData(&buffers[i]);
				buffers[i].fukuden.kind = NTBUF_FUKUDEN_NONE;
				break;
			case NTBUF_FUKUDEN_NONE:
				/* no break */
			default:
				/* do nothing */
				break;
			}
			/* バッファ再構成 */
			switch (buffers[i].save) {
			case NTBUF_SAVE:
				/* do nothing */
				break;
			case NTBUF_SAVE_BY_DATA:
				ntbuf_RearrangeNtBuffer(&buffers[i]);
				break;
			case NTBUF_NOT_SAVE:
				/* no break */
			default:
				/* このバッファは毎回クリアされるので停電保証不要 */
				buffers[i].wofs = 0;
				buffers[i].rofs = 0;
				break;
			}
		}
		/* バッファ状態データ作成 */
		ntbuf_ChkBufState(&buffers[i]);
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_RearrangeNtBuffer
 *[]----------------------------------------------------------------------[]
 *| summary	: バッファ再構成
 *|			  データ保持フラグがOFFのデータをバッファから削除する
 *| param	: buffer - データ削除対象バッファ
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_RearrangeNtBuffer(t_NtBuf *buffer)
{
	long rofs, rofs_next;
	
	rofs_next = buffer->rofs;
	
	while (rofs_next != buffer->wofs) {
		/* データヘッダ読み込み */
		rofs = rofs_next;
		ntbuf_ReadNtBuf(buffer, &z_NtBufNtDataHeaderWork, rofs, sizeof(z_NtBufNtDataHeaderWork));
		rofs_next = _offset(rofs, _MAKEWORD(z_NtBufNtDataHeaderWork.bufdata_header.len), buffer->buffer_size);
		if (z_NtBufNtDataHeaderWork.ntdata_header.data_save == NTNET_NOT_BUFFERING) {
		/* データ保持フラグ=OFF なのでこのデータを捨てる */
			ntbuf_DiscardNtData(buffer, rofs, _MAKEWORD(z_NtBufNtDataHeaderWork.bufdata_header.len));
			rofs_next = buffer->rofs;	/* 最初から再検索 */
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_DiscardNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: 指定されたNTNETデータをバッファから捨てる(捨てたデータ長分、後ろのデータを詰める)
 *| param	: buffer - データ削除対象バッファ
 *|			  delete_data_ofs - 削除対象データ先頭のバッファ内オフセット
 *|			  delete_data_len - 削除対象データのバイト長
 *| return	: none
 *[]----------------------------------------------------------------------[]
 *| remark	:
 *|			読み込みオフセット								書き込みオフセット
 *|				  ↓												↓
 *|		┌────┬──────────┬─────────┬───┬────┐
 *|		│ (空き) │　　　　Ａ　　　　　｜　　　　Ｂ　　　　│　Ｃ　│ (空き) │
 *|		└────┴──────────┴─────────┴───┴────┘
 *|			  						｜
 *|			  						｜データ保持フラグが、Ａ，Ｃ＝ＯＮ、Ｂ＝ＯＦＦの場合
 *|			  						｜Ｂのデータを削除して、その分Ａを詰める
 *|			  						↓
 *|		┌──────────────┬──────────┬───┬────┐
 *|		│　　　　　　(空き)　　　　　│　　　　Ａ　　　　　│　Ｃ　│ (空き) │
 *|		└──────────────┴──────────┴───┴────┘
 *|									  ↑							↑
 *|							読み込みオフセット				書き込みオフセット
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_DiscardNtData(t_NtBuf *buffer, long delete_data_ofs, ushort delete_data_len)
{
	ushort	len;
	long	count, ofs;
	
	if (delete_data_ofs != buffer->wofs) {
		/* 削除用データ準備 */
		buffer->fukuden.datamove.move	= delete_data_len;	/* 削除するデータのサイズ */
		buffer->fukuden.datamove.rofs	= buffer->rofs;		/* 移動対象とするデータの先頭 */
		count = 0;
		ofs = buffer->rofs;
		while (ofs != delete_data_ofs) {
			ntbuf_ReadNtBuf(buffer, &len, ofs, sizeof(len));
			ofs = _offset(ofs, len, buffer->buffer_size);
			count += len;
		}
		buffer->fukuden.datamove.count	= count;			/* 移動対象となるデータのサイズ */
		
		/* データの削除 */
		buffer->fukuden.kind = NTBUF_FUKUDEN_DATAMOVE;	/* 復電処理のチェックポイント */
		ntbuf_MoveNtData(buffer);
		buffer->fukuden.kind = NTBUF_FUKUDEN_NONE;		/* 停電保証対象エリア終了 */
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_MoveNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: 同バッファ内でのデータ移動
 *| param	: buffer - データ移動対象バッファ
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_MoveNtData(t_NtBuf *buffer)
{
	long	src_ofs, dst_ofs;
	
	while (buffer->fukuden.datamove.count > 0) {
		src_ofs = _offset(buffer->fukuden.datamove.rofs, buffer->fukuden.datamove.count-1, buffer->buffer_size);
		dst_ofs = _offset(src_ofs, buffer->fukuden.datamove.move, buffer->buffer_size);
		buffer->buffer[dst_ofs] = buffer->buffer[src_ofs];
		buffer->fukuden.datamove.count--;
		WACDOG;		/* 念のため */
	}
	
	buffer->rofs = _offset(buffer->fukuden.datamove.rofs, buffer->fukuden.datamove.move, buffer->buffer_size);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_ChkRcvBuf
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NETデータ受信バッファをチェックし、見つかったデータ数分の
 *|			  IBK_NTNET_DAT_RECメッセージをOPEへ送信する
 *| param	: buffers - NT-NETデータ受信バッファ
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_ChkRcvBuf(t_NtBuf *buffers)
{
	ushort	i, len;
	long	rofs;
	
	for (i = 0; i < NTNET_RCVBUF_MAX; i++) {
		rofs = buffers[i].rofs;
		while (rofs != buffers[i].wofs) {
			ntbuf_ReadNtBuf(&buffers[i], &len, rofs, sizeof(len));
			rofs = _offset(rofs, len, buffers[i].buffer_size);
			queset(OPETCBNO, (ushort)IBK_NTNET_DAT_REC, 0, NULL);	/* OPEへNTNETデータ有りを通知 */
		}
	}
}

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_GetBufCount
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NETデータバッファ内の残データ数を算出する
 *| param	: buffer - NT-NETデータバッファ
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static	ulong	ntbuf_GetBufCount(t_NtBuf *buffer)
{
	ushort	len;
	long	rofs;
	ulong	count = 0;
	
	rofs = buffer->rofs;
	while (rofs != buffer->wofs) {
		ntbuf_ReadNtBuf(buffer, &len, rofs, sizeof(len));
		rofs = _offset(rofs, len, buffer->buffer_size);
		count++;
		WACDOG;		/* 念のため */
	}

	return count;
}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_GetBufCount
 *[]----------------------------------------------------------------------[]
 *| summary	: バッファ毎の残データ件数を返す
 *| param	: buf - 残データ件数データ						<OUT>
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
 *| 		  isRemote - TRUE : 遠隔 FALSE : NT-NET
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//void	NTBUF_GetBufCount(t_NtBufCount *buf)
//{
//	buf->sndbuf_prior		= 	0UL;
//	buf->sndbuf_sale		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_PAYMENT, eLOG_TARGET_REMOTE);
//	if(prm_get(COM_PRM, S_NTN, 121, 1, 1) == 0) {
//		buf->sndbuf_ttotal		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_TTOTAL, eLOG_TARGET_REMOTE);
//	}
//	else {
//		// センター対応Phase2以降はT合計とGT合計の合計とする
//		buf->sndbuf_ttotal		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_TTOTAL, eLOG_TARGET_REMOTE);
//		buf->sndbuf_ttotal		+= 	(ulong)Ope_Log_UnreadCountGet(eLOG_GTTOTAL, eLOG_TARGET_REMOTE);
//	}
//	buf->sndbuf_coin		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_COINBOX, eLOG_TARGET_REMOTE);
//	buf->sndbuf_note		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_NOTEBOX, eLOG_TARGET_REMOTE);
//	buf->sndbuf_error		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_ERROR, eLOG_TARGET_REMOTE);
//	buf->sndbuf_alarm		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_ALARM, eLOG_TARGET_REMOTE);
//	buf->sndbuf_money		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);
//	buf->sndbuf_turi		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_MNYMNG_SRAM, eLOG_TARGET_REMOTE);
//	buf->sndbuf_monitor		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_MONITOR, eLOG_TARGET_REMOTE);
//	buf->sndbuf_ope_monitor	= 	(ulong)Ope_Log_UnreadCountGet(eLOG_OPERATE, eLOG_TARGET_REMOTE);
//	buf->sndbuf_normal		= 	0UL;
//// MH810100(S) K.Onodera 2019/11/15 車番チケットレス(RT精算データ対応)
////	buf->sndbuf_incar		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_ENTER, eLOG_TARGET_REMOTE);
//	buf->sndbuf_incar		= 	0UL;
//// MH810100(E) K.Onodera 2019/11/15 車番チケットレス(RT精算データ対応)
//	buf->sndbuf_outcar		= 	0UL;
//	buf->sndbuf_car_count	= 	(ulong)Ope_Log_UnreadCountGet(eLOG_PARKING, eLOG_TARGET_REMOTE);
//	buf->sndbuf_area_count	= 	0UL;
//	buf->sndbuf_rmon		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_REMOTE_MONITOR, eLOG_TARGET_REMOTE);
//// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//	buf->sndbuf_lpark		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_LONGPARK_PWEB, eLOG_TARGET_REMOTE);
//// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//	buf->rcvbuf_broadcast	= 	0UL;
//	buf->rcvbuf_prior		= 	0UL;
//	buf->rcvbuf_normal		= 	0UL;
//	return ;
//}
void	NTBUF_GetBufCount(t_NtBufCount *buf, BOOL isRemote)
{
	ushort	logTarget;									// 対象のログターゲット
	ushort	index;
	
	logTarget = isRemote == TRUE ? eLOG_TARGET_REMOTE : eLOG_TARGET_NTNET;
	buf->sndbuf_sale		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_PAYMENT, logTarget);
// MH341110(S) A.Iiizumi 2017/12/28 動作チェック-NT-NETチェックのデータクリア処理でGT合計の未送信件数を正しく取得できない不具合修正 (共通改善№1396)
//	if(FALSE == isRemote) {								// NT-NET
//		// NT-NETはT合計のみの件数
//		buf->sndbuf_ttotal		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_TTOTAL, logTarget);
//	}
//	else {
// MH341110(E) A.Iiizumi 2017/12/28 動作チェック-NT-NETチェックのデータクリア処理でGT合計の未送信件数を正しく取得できない不具合修正 (共通改善№1396)
		if(prm_get(COM_PRM, S_NTN, 121, 1, 1) == 0) {
			buf->sndbuf_ttotal		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_TTOTAL, logTarget);
		}
		else {
			// センター対応Phase2以降はT合計とGT合計の合計とする
			buf->sndbuf_ttotal		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_TTOTAL, logTarget);
			buf->sndbuf_ttotal		+= 	(ulong)Ope_Log_UnreadCountGet(eLOG_GTTOTAL, logTarget);
		}
// MH341110(S) A.Iiizumi 2017/12/28 動作チェック-NT-NETチェックのデータクリア処理でGT合計の未送信件数を正しく取得できない不具合修正 (共通改善№1396)
//	}
// MH341110(E) A.Iiizumi 2017/12/28 動作チェック-NT-NETチェックのデータクリア処理でGT合計の未送信件数を正しく取得できない不具合修正 (共通改善№1396)
	buf->sndbuf_coin		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_COINBOX, logTarget);
	buf->sndbuf_note		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_NOTEBOX, logTarget);
	buf->sndbuf_error		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_ERROR, logTarget);
	buf->sndbuf_alarm		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_ALARM, logTarget);
	buf->sndbuf_money		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, logTarget);
	buf->sndbuf_turi		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_MNYMNG_SRAM, logTarget);
	buf->sndbuf_monitor		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_MONITOR, logTarget);
	buf->sndbuf_ope_monitor	= 	(ulong)Ope_Log_UnreadCountGet(eLOG_OPERATE, logTarget);
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（未使用ログ（入庫））
//	buf->sndbuf_incar		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_ENTER, logTarget);
	buf->sndbuf_incar		= 	0UL;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（未使用ログ（入庫））
// MH364300 GG119A34(S) 改善連絡表No.83対応
//	buf->sndbuf_outcar		= 	0UL;
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（未使用ログ（出庫））
//	buf->sndbuf_outcar		= 	IS_SEND_OUTCAR_DATA ? (ulong)Ope_Log_UnreadCountGet(eLOG_LEAVE, logTarget) : 0UL;
	buf->sndbuf_outcar		= 	0UL;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（未使用ログ（出庫））
// MH364300 GG119A34(E) 改善連絡表No.83対応
	buf->sndbuf_car_count	= 	(ulong)Ope_Log_UnreadCountGet(eLOG_PARKING, logTarget);
	buf->sndbuf_area_count	= 	0UL;
	buf->sndbuf_rmon		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_REMOTE_MONITOR, logTarget);
// MH364300 GG119A08(S) // MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	buf->sndbuf_lpark		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_LONGPARK_PWEB, eLOG_TARGET_REMOTE);
// MH364300 GG119A08(E) // MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)

	if(FALSE == isRemote) {								// NT-NETはバッファのデータ数を取得する
		buf->sndbuf_prior		= 	ntbuf_GetBufCount(&z_NtSndBuf[NTNET_SNDBUF_PRIOR]);		// 送信優先データバッファ
		buf->sndbuf_normal		= 	ntbuf_GetBufCount(&z_NtSndBuf[NTNET_SNDBUF_NORMAL]);	// 送信通常データバッファ
		buf->rcvbuf_broadcast	= 	ntbuf_GetBufCount(&z_NtRcvBuf[NTNET_RCVBUF_BROADCAST]);	// 受信同報データバッファ
		buf->rcvbuf_prior		= 	ntbuf_GetBufCount(&z_NtRcvBuf[NTNET_RCVBUF_PRIOR]);		// 受信優先データバッファ
		buf->rcvbuf_normal		= 	ntbuf_GetBufCount(&z_NtRcvBuf[NTNET_RCVBUF_NORMAL]);	// 受信通常データバッファ
		
		// 送信制御バッファ(NTNET)にあるデータを加算する
		for(index = 0; index < NTNET_SNDBUF_MAX; ++index) {
			if(NTNET_SendCtrl[index].length != 0 && 		// 送信制御データに送信データあり
			   NTNET_SendCtrl[index].logid == eLOG_MAX) {	// 送信データはログ以外(優先、通常)
				// 送信制御データに送信データがある場合は件数に加算する
				if(index == NTNET_SNDBUF_PRIOR) {		// 優先データ
					++buf->sndbuf_prior;
				}
				else {									// 通常データ
					++buf->sndbuf_normal;
				}
			}
		}
		// 送信バッファ(NTCOM)にある優先データを加算する
		switch(NTCom_GetSendPriorDataID()) {			// 優先送信データ
		case 0:											// データなし
			// 何もしない
			break;
		case 22:										// 精算(事前)
		case 23:										// 精算(出口)
// MH364300 GG119A11(S) // GM569000 (S) ParkingWeb端末間（フェーズ２）通信対応
		case 56:										// 精算(事前)（端末間データ）
		case 57:										// 精算(出口)（端末間データ）
// MH364300 GG119A11(E) // GM569000 (E) ParkingWeb端末間（フェーズ２）通信対応
			++buf->sndbuf_sale;
			break;
		default:										// ログデータ以外の優先データ
			++buf->sndbuf_prior;
			break;
		}

		// 送信バッファ(NTCOM)にある通常データを加算する
		switch(NTCom_GetSendNormalDataID()) {			// 通常送信データ
		case 0:											// データなし
			// 何もしない
			break;
		case 20:										// 入庫
// MH364300 GG119A11(S) // GM569000 (S) ParkingWeb端末間（フェーズ２）通信対応
		case 54:										// 入庫（端末間データ）
// MH364300 GG119A11(E) // GM569000 (E) ParkingWeb端末間（フェーズ２）通信対応
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（未使用ログ（入庫））
//			++buf->sndbuf_incar;
			// 何もしない
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（未使用ログ（入庫））
			break;
// MH364300 GG119A34(S) 改善連絡表No.83対応
		case 21:										// 出庫
		case 55:										// 出庫（端末間データ）
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（未使用ログ（出庫））
//			if (IS_SEND_OUTCAR_DATA) {
//				++buf->sndbuf_outcar;
//			}
			// 何もしない
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（未使用ログ（出庫））
			break;
// MH364300 GG119A34(E) 改善連絡表No.83対応
		case 41:										// 集計終了通知
// MH341110(S) A.Iiizumi 2017/12/28 動作チェック-NT-NETチェックのデータクリア処理でGT合計の未送信件数を正しく取得できない不具合修正 (共通改善№1396)
		case 53:										// 集計終了通知 センター対応Phase2以降
// MH341110(E) A.Iiizumi 2017/12/28 動作チェック-NT-NETチェックのデータクリア処理でGT合計の未送信件数を正しく取得できない不具合修正 (共通改善№1396)
			++buf->sndbuf_ttotal;
			break;
		case 120:										// エラー
// MH364300 GG119A11(S) // GM569000 (S) ParkingWeb端末間（フェーズ２）通信対応
		case 63:										// エラー（端末間データ）
// MH364300 GG119A11(E) // GM569000 (E) ParkingWeb端末間（フェーズ２）通信対応
			++buf->sndbuf_error;
			break;
		case 121:										// アラーム
// MH364300 GG119A11(S) // GM569000 (S) ParkingWeb端末間（フェーズ２）通信対応
		case 64:										// アラーム（端末間データ）
// MH364300 GG119A11(E) // GM569000 (E) ParkingWeb端末間（フェーズ２）通信対応
			++buf->sndbuf_alarm;
			break;
		case 122:										// モニタ
			++buf->sndbuf_monitor;
			break;
		case 123:										// 操作モニタ
			++buf->sndbuf_ope_monitor;
			break;
		case 126:										// 金銭管理(SRAM)
			++buf->sndbuf_money;
			break;
		case 131:										// コイン金庫集計
			++buf->sndbuf_coin;
			break;
		case 133:										// 紙幣金庫集計
			++buf->sndbuf_note;
			break;
		case 236:										// 駐車台数
			++buf->sndbuf_car_count;
			break;
// MH364300 GG119A11(S) // GM569000 (S) ParkingWeb端末間（フェーズ２）通信対応
		case 135:										// 釣銭管理集計データ(SRAM)
			++buf->sndbuf_turi;
			break;
// MH364300 GG119A11(E) // GM569000 (E) ParkingWeb端末間（フェーズ２）通信対応
		default:										// ログデータ以外の通常データ
			++buf->rcvbuf_normal;
			break;
		}
	}
	else {												// NT-NET以外は対象外
		buf->sndbuf_prior		= 	0UL;
		buf->sndbuf_normal		= 	0UL;
		buf->rcvbuf_broadcast	= 	0UL;
		buf->rcvbuf_prior		= 	0UL;
		buf->rcvbuf_normal		= 	0UL;
	}
	return ;
}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_ChkSndBuf
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NETデータ送信バッファ上のデータ有無をチェック
 *| param	: buffers - NT-NETデータ送信バッファ
 *| return	: TRUE - データあり
 *[]----------------------------------------------------------------------[]*/
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//BOOL	ntbuf_ChkSndBuf(t_NtBuf *buffers, t_NtFreeBuf *freebuf)
BOOL	ntbuf_ChkSndBuf(t_NtBuf *buffers)
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
{
	ushort	i;
	
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
//	if (freebuf->isdata) {
//		return TRUE;
//	}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）

	for (i = 0; i < NTNET_SNDBUF_MAX; i++) {
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//		if ((_ntbuf_IsNtData(&buffers[i])) &&
//			((z_NearFullBitEx[i] & z_NearFullBuff) == 0) &&
//			((z_NearFullBitEx[i] & z_PendingBuff) == 0)) {
		if (_ntbuf_IsNtData(&buffers[i])) {
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
			return TRUE;
		}
	}
	
	return FALSE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_SetNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: データをバッファに書込み
 *| param	: buffer - データをセットするバッファ
 *|			  data - 書き込み対象となるデータ					<IN>
 *|			  len - dataのバイト長
 *|	return	: NTNET_BUFSET_NORMAL=正常終了
 *|			  NTNET_BUFSET_DEL_OLD=最古データを消去
 *|			  NTNET_BUFSET_DEL_NEW=最新データを消去
 *|			  NTNET_BUFSET_CANT_DEL=バッファFULLだが設定が"休業"のため消去不可
 *[]----------------------------------------------------------------------[]*/
uchar	ntbuf_SetNtData(t_NtBuf *buffer, const void *data, ushort len)
{
	ushort	write_len, len_0cut;
	uchar	ret;
	
	ret = NTNET_BUFSET_NORMAL;
	
	len_0cut = ntbuf_GetNtData0CutLen(data, len);
	write_len = (ushort)(len_0cut + sizeof(t_NtBufDataHeader));
	/* バッファ空き容量のチェック */
	if (ntbuf_GetNtBufRemain(buffer) < write_len) {
		/* バッファFULL時の設定取得 */
		ret = (uchar)buffer->setting();
		switch (ret) {
		case NTNET_BUFSET_DEL_NEW:		/* 新消去 */
			ntbuf_DeleteLatestNtData(buffer, (long)write_len);
			break;
		case NTNET_BUFSET_CANT_DEL:		/* 休業 */
			/* do nothing */
			break;
		case NTNET_BUFSET_DEL_OLD:		/* 旧消去 */
			/* no break */
		default:
			ret = NTNET_BUFSET_DEL_OLD;
			ntbuf_DeleteOldestNtData(buffer, (long)write_len);
			break;
		}
	}
	
	if (ret != NTNET_BUFSET_CANT_DEL) {
	/* バッファ書込み */
		ntbuf_WriteNtData(buffer, data, len, len_0cut);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_GetNtData0CutLen
 *[]----------------------------------------------------------------------[]
 *| summary	: 0カット後のデータサイズ取得
 *| param	: data - サイズ算出対象データ
 *|			  len - データ長
 *|	return	: 0カット後のデータサイズ
 *[]----------------------------------------------------------------------[]*/
ushort	ntbuf_GetNtData0CutLen(const uchar *data, ushort len)
{
	while (len > 0) {
		if (data[len-1] != 0) {
			break;
		}
		len--;
	}
	
	return len;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_DeleteOldestNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: write_len分のバッファ空き領域を確保する(データ削除は最古データから順に行う)
 *| param	: buffer - データを削除するバッファ
 *|			  len - 必要な空きサイズ
 *|	return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_DeleteOldestNtData(t_NtBuf *buffer, long write_len)
{
	ushort len;
	
	while (ntbuf_GetNtBufRemain(buffer) < write_len) {
		ntbuf_ReadNtBuf(buffer, &len, buffer->rofs, sizeof(len));
		buffer->rofs = _offset(buffer->rofs, len, buffer->buffer_size);
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_DeleteLatestNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: write_len分のバッファ空き領域を確保する(データ削除は最新データから順に行う)
 *| param	: buffer - データを削除するバッファ
 *|			  len - 必要な空きサイズ
 *|	return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_DeleteLatestNtData(t_NtBuf *buffer, long write_len)
{
	ushort	len;
	long	ofs;
	
	while (ntbuf_GetNtBufRemain(buffer) < write_len) {
		/* 最新データを検索 */
		ofs = buffer->rofs;
		len = 0;
		while (_offset(ofs, len, buffer->buffer_size) != buffer->wofs) {
			ofs = _offset(ofs, len, buffer->buffer_size);
			ntbuf_ReadNtBuf(buffer, &len, ofs, sizeof(len));
		}
		buffer->wofs = ofs;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_WriteNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: データ書込み
 *| param	: buffer - データをセットするバッファ
 *|			  data - 書き込み対象となるデータ					<IN>
 *|			  len - dataのバイト長
 *|			  len_0cut - 0カット後のデータサイズ
 *|	return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_WriteNtData(t_NtBuf *buffer, const void *data, ushort len_before0cut, ushort len_after0cut)
{
	t_NtBufDataHeader header;
	long	ofs;
	
	ofs = buffer->wofs;

	/* データヘッダ書込み */
	ntbuf_W2Bcpy(header.len, (ushort)(len_after0cut + sizeof(t_NtBufDataHeader)));	/* バッファデータ長 */
	ntbuf_W2Bcpy(header.len_before0cut, len_before0cut);							/* 0カット前のデータ長 */
	header.lower_terminal1	= 0;													/* 下位転送用端末No.(1) */
	header.lower_terminal2	= 0;													/* 下位転送用端末No.(2) */
	header.reserve			= 0;													/* 予備 */
	ofs = ntbuf_WriteNtBuf(buffer, ofs, &header, sizeof(header));
	/* データ書込み */
	ofs = ntbuf_WriteNtBuf(buffer, ofs, data, (ulong)len_after0cut);

	/* 書き込みオフセット更新 */
	buffer->wofs = ofs;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_GetNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NETデータ読み込み
 *| param	: buffer - データの読み込み元バッファ
 *|			  data - データコピー先領域						<OUT>
 *|			  f_Expand - 1=0カット分を伸長して返す，0=0カットのまま返す
 *|	return	: 取得したデータのデータ長
 *|			  0＝データ無し
 *[]----------------------------------------------------------------------[]*/
ushort	ntbuf_GetNtData(t_NtBuf *buffer, void *data, uchar f_Expand)
{
	t_NtBufDataHeader header;
	ushort	ret, len, len_before0cut;
	long	ofs;
	
	ret = 0;
	if (_ntbuf_IsNtData(buffer)) {
		/* データヘッダからデータ長を取得 */
		ofs = ntbuf_ReadNtBuf(buffer, &header, buffer->rofs, sizeof(t_NtBufDataHeader));
		len = _MAKEWORD(header.len);
		len -= sizeof(t_NtBufDataHeader);
		/* データ読み込み */
		ntbuf_ReadNtBuf(buffer, data, ofs, (ulong)len);
		/* 0カット分の伸長 */
		if( f_Expand ){
			len_before0cut = _MAKEWORD(header.len_before0cut);
			while (len < len_before0cut) {
				((uchar*)data)[len++] = 0;
			}
		}
		ret = len;
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_NtDataClr
 *[]----------------------------------------------------------------------[]
 *| summary	: バッファから最古データを1件削除
 *| param	: buffer - データの削除対象バッファ
 *|	return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_NtDataClr(t_NtBuf *buffer)
{
	ushort len;
	
	if (_ntbuf_IsNtData(buffer)) {
		ntbuf_ReadNtBuf(buffer, &len, buffer->rofs, sizeof(len));
		buffer->rofs = _offset(buffer->rofs, len, buffer->buffer_size);
	}
}

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
///*[]----------------------------------------------------------------------[]
// *|	name	: ntbuf_SetFreeData
// *[]----------------------------------------------------------------------[]
// *| summary	: FREEデータをバッファに書き込む
// *| param	: buffer - データをセットするバッファ
// *|			  data - 書き込み対象となるデータ					<IN>
// *| return	: 1＝正常終了
// *|			  0＝バッファオーバーライト発生
// *[]----------------------------------------------------------------------[]*/
//uchar	ntbuf_SetFreeData(t_NtFreeBuf *buffer, const void *data)
//{
//	uchar ret;
//	
//	if (buffer->isdata) {
//		ret = 0;	/* overwrite! */
//	}
//	else {
//		ret = 1;
//	}
//	
//	memcpy(buffer->buffer, data, sizeof(buffer->buffer));
//	buffer->isdata = TRUE;	/* データ有り */
//	
//	return ret;
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: ntbuf_GetFreeData
// *[]----------------------------------------------------------------------[]
// *| summary	: FREEデータをコピーし、バッファから削除
// *| param	: buffer - データの読み込み元するバッファ
// *|			  data - コピー先領域					<OUT>
// *| return	: 1＝正常終了
// *|			  0＝データ無し
// *[]----------------------------------------------------------------------[]*/
//uchar	ntbuf_GetFreeData(t_NtFreeBuf *buffer, void *data)
//{
//	uchar ret;
//	
//	if (!buffer->isdata) {
//		ret = 0;	/* データ無し */
//	}
//	else {
//		ret = 1;
//		memcpy(data, buffer->buffer, sizeof(buffer->buffer));
//		buffer->isdata = FALSE;		/* データ削除 */
//	}
//	
//	return ret;
//}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_GetSndNtBufCode
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NETデータ送信バッファコードの取得
 *| param	: data - 書き込み対象となるデータ					<IN>
 *|			  buffer - バッファ種別
 *|					   (NTNET_BUF_BUFFERING/NTNET_BUF_PRIOR/NTNET_BUF_NORMAL)
 *| return	: 引数dataにコピーしたバイト数
 *|			  0＝指定バッファにデータなし
 *[]----------------------------------------------------------------------[]*/
int	ntbuf_GetSndNtBufCode(const void *data, eNTNET_BUF_KIND buffer)
{
	int		ret;
	
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
//	switch (((t_NtNetDataHeader*)data)->data_kind) {
//	case 22:	/* no break */
//	case 23:
//	case 56:
//	case 57:
//	/* 精算データテーブル */
//		ret = NTNET_SNDBUF_SALE;
//		break;
//	case 21:
//	/* 出庫データテーブル */
//		ret = NTNET_SNDBUF_CAR_OUT;
//		break;
//	case 20:
//	case 54:
//	/* 入庫データテーブル */
//		ret = NTNET_SNDBUF_CAR_IN;
//		break;
//	case 30:	/* no break */
//	case 31:	/* no break */
//	case 32:	/* no break */
//	case 33:	/* no break */
//	case 34:	/* no break */
//	case 35:	/* no break */
//	case 36:	/* no break */
//	case 37:	/* no break */
//	case 38:	/* no break */
//	case 41:	/* no break */
//	/* T合計集計データ */
//		ret = NTNET_SNDBUF_TTOTAL;
//		break;
//	case 131:
//	/* コイン金庫集計合計データ */
//		ret = NTNET_SNDBUF_COIN;
//		break;
//	case 133:
//	/* 紙幣金庫集計合計データ */
//		ret = NTNET_SNDBUF_NOTE;
//		break;
//	case 120:
//	/* エラーデータ */
//		ret = NTNET_SNDBUF_ERROR;
//		break;
//	case 121:
//	/* アラームデータ */
//		ret = NTNET_SNDBUF_ALARM;
//		break;
//	case 126:
//	/* 金銭管理データ */
//		ret = NTNET_SNDBUF_MONEY;
//		break;
//	case 236:
//	/* 駐車台数データ */
//		ret = NTNET_SNDBUF_CAR_COUNT;
//		break;
//	case 237:
//	/* 区画台数・満車データ */
//		ret = NTNET_SNDBUF_AREA_COUNT;
//		break;
//	case 122:
//	/* モニタデータ */
//		ret = NTNET_SNDBUF_MONITOR;
//		break;
//	case 123:
//	/* 操作モニタデータ */
//		ret = NTNET_SNDBUF_OPE_MONITOR;
//		break;
//	default:
//		switch (buffer) {
//		case NTNET_BUF_PRIOR:
//		/* バッファリングデータではない優先データ */
//			ret = NTNET_SNDBUF_PRIOR;
//			break;
//		case NTNET_BUF_NORMAL:
//		/* バッファリングデータではない通常データ */
//			ret = NTNET_SNDBUF_NORMAL;
//			break;
//		default:
//			ret = -1;	/* このパスは通らないはず */
//			break;
//		}
//	}
	switch (buffer) {
	case NTNET_BUF_PRIOR:
	/* バッファリングデータではない優先データ */
		ret = NTNET_SNDBUF_PRIOR;
		break;
	case NTNET_BUF_NORMAL:
		/* バッファリングデータではない通常データ */
		ret = NTNET_SNDBUF_NORMAL;
		break;
	default:
		ret = -1;	/* このパスは通らないはず */
		break;
	}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_GetRcvNtBufCode
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NETデータ受信バッファコードの取得
 *| param	: buffer - バッファ種別
 *|					   (NTNET_BUF_BROADCAST/NTNET_BUF_PRIOR/NTNET_BUF_NORMAL)
 *| return	: 引数dataにコピーしたバイト数
 *|			  0＝指定バッファにデータなし
 *[]----------------------------------------------------------------------[]*/
int	ntbuf_GetRcvNtBufCode(eNTNET_BUF_KIND buffer)
{
	int ret;
	
	switch (buffer) {
	case NTNET_BUF_BROADCAST:
		ret = NTNET_RCVBUF_BROADCAST;
		break;
	case NTNET_BUF_PRIOR:
		ret = NTNET_RCVBUF_PRIOR;
		break;
	case NTNET_BUF_NORMAL:
		ret = NTNET_RCVBUF_NORMAL;
		break;
	default:
		ret = -1;
		break;
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_GetNtBufRemain
 *[]----------------------------------------------------------------------[]
 *| summary	: バッファ残容量取得
 *| param	: buffer - 残容量取得したいバッファ
 *| return	: バッファ残容量
 *[]----------------------------------------------------------------------[]*/
long	ntbuf_GetNtBufRemain(t_NtBuf *buffer)
{
	long rofs;
	
	if (buffer->wofs == buffer->rofs) {
		return buffer->buffer_size - 1;
	}
	
	rofs = buffer->rofs;
	if (buffer->wofs > buffer->rofs) {
		rofs += buffer->buffer_size;
	}
		
	return rofs - buffer->wofs - 1;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_ChkSndNtDataLen
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信データ長チェック
 *| param	: bufcode - データ格納先バッファ
 *|			  len - データ長
 *| return	: TRUE - OK
 *[]----------------------------------------------------------------------[]*/
BOOL	ntbuf_ChkSndNtDataLen(int bufcode, ushort len) 
{
	ushort limit;
	
	if (NTNET_SNDBUF_PRIOR_TOP <= bufcode && bufcode <= NTNET_SNDBUF_PRIOR_END) {
		limit = NTNET_BLKDATA_MAX + 3;		/* 優先データの最大データ長("+3"=ID3,ID4,データ保持フラグ) */
	}
	else {
		limit = NTNET_BLKDATA_MAX * 26 + 3;	/* 通常データの最大データ長("+3"=ID3,ID4,データ保持フラグ) */
	}
	
	if (len > limit) {
		return FALSE;
	}
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_ChkRcvNtDataLen
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信データ長チェック
 *| param	: bufcode - データ格納先バッファ
 *|			  len - データ長
 *| return	: TRUE - OK
 *[]----------------------------------------------------------------------[]*/
BOOL	ntbuf_ChkRcvNtDataLen(int bufcode, ushort len) 
{
	ushort limit;
	
	switch (bufcode) {
	case NTNET_RCVBUF_NORMAL:
		limit = NTNET_BLKDATA_MAX * 26 + 3;	/* 通常データの最大データ長("+3"=ID3,ID4,データ保持フラグ) */
		break;
	case NTNET_RCVBUF_BROADCAST:	/* no break */
	case NTNET_RCVBUF_PRIOR:		/* no break */
	default:
		limit = NTNET_BLKDATA_MAX + 3;		/* 優先,同報データの最大データ長("+3"=ID3,ID4,データ保持フラグ) */
		break;
	}
	
	if (len > limit) {
		return FALSE;
	}
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_ReadNtBuf
 *[]----------------------------------------------------------------------[]
 *| summary	: バッファからのデータ読み込み
 *| param	: buffer - データ読み込み対象バッファ
 *|			  dst - データ読込先領域							<OUT>
 *|			  top - 読み込み元データ先頭
 *|			  cnt - 読み込みデータ数
 *| return	: 読み込み実行後のtop位置
 *[]----------------------------------------------------------------------[]*/
long	ntbuf_ReadNtBuf(t_NtBuf *buffer, void *dst, long top, ulong cnt)
{
	ulong rcnt;
	uchar *cdst = (uchar*)dst;
	
	if (top + cnt >= (ulong)buffer->buffer_size) {
		rcnt = buffer->buffer_size - top;
		memcpy(cdst, &buffer->buffer[top], rcnt);
		cdst += rcnt;
		cnt -= rcnt;
		top = 0;
	}
	memcpy(cdst, &buffer->buffer[top], cnt);
	
	return (long)(top + cnt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_WriteNtBuf
 *[]----------------------------------------------------------------------[]
 *| summary	: データ書込み
 *| param	: buffer - データをセットするバッファ
 *|			  dst_ofs - データ書き込み開始オフセット
 *|			  data - 書き込み対象となるデータ					<IN>
 *|			  cnt - データ書き込みサイズ
 *|	return	: none
 *[]----------------------------------------------------------------------[]*/
long	ntbuf_WriteNtBuf(t_NtBuf *buffer, long dst_ofs, const void *data, ulong cnt)
{
	const uchar *csrc = (const uchar*)data;
	ulong wcnt;
	
	if (dst_ofs + cnt >= (ulong)buffer->buffer_size) {
		wcnt = buffer->buffer_size - dst_ofs;
		memcpy(&buffer->buffer[dst_ofs], csrc, wcnt);
		dst_ofs	= 0;
		csrc	+= wcnt;
		cnt		-= wcnt;
	}
	
	memcpy(&buffer->buffer[dst_ofs], csrc, cnt);
	
	return (long)(dst_ofs + cnt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_ChkBufState
 *[]----------------------------------------------------------------------[]
 *| summary	: バッファ状態チェック
 *|			  バッファ状態に変化があればバッファ状態データを更新する
 *| param	: buffer - チェック対象バッファ
 *| return	: TRUE - バッファ状態変化
 *|			  FALSE - バッファ状態変化無し
 *[]----------------------------------------------------------------------[]*/
BOOL	ntbuf_ChkBufState(t_NtBuf *buffer)
{
	long	remain;
	uchar	state;
	
	if (buffer->setting == ntbuf_GetDefaultSetting) {
		return FALSE;	/* 常に最古データに上書きのバッファなので状態監視の必要なし */
	}
	
	state = NTBUF_BUFFER_NORMAL;
	remain = ntbuf_GetNtBufRemain(buffer);
	if (remain < buffer->datasize) {
	/* バッファFULL */
		if (buffer->id == 22) {
		/* 精算データテーブルの場合、ニアFULLも発生中とする */
			state = (NTBUF_BUFFER_FULL|NTBUF_BUFFER_NEAR_FULL);
		}
		else {
			state = NTBUF_BUFFER_FULL;
		}
	}
	else if (remain < buffer->datasize * NTBUF_NEARFULL_COUNT) {
		if (buffer->id == 22) {
	/* ニアFULL(精算データテーブルでのみ発生) */
			state = NTBUF_BUFFER_NEAR_FULL;
		}
	}
	
	/* バッファ状態データセット(変化があればバッファ状態データ更新) */
	if (ntbuf_SetBufState(buffer, state)) {
		return TRUE;
	}
	
	return FALSE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_SetBufState
 *[]----------------------------------------------------------------------[]
 *| summary	: バッファ状態データセット
 *| param	: buffer - 対象バッファ
 *|			  state - 現在のバッファ状態(NTBUF_BUFFER_NORMAL/NTBUF_BUFFER_FULL/NTBUF_BUFFER_NEAR_FULL)
 *| return	: TRUE - バッファ状態変化
 *|			  FALSE - バッファ状態変化無し
 *[]----------------------------------------------------------------------[]*/
BOOL	ntbuf_SetBufState(t_NtBuf *buffer, uchar state)
{
	uchar	*dst;
	
	switch (buffer->id) {
	case 22:	/* 精算データテーブル */
		dst = &z_NtBufState.sale;
		break;
	case 21:	/* 出庫データテーブル */
		dst = &z_NtBufState.car_out;
		break;
	case 20:	/* 入庫データテーブル */
		dst = &z_NtBufState.car_in;
		break;
	case 41:	/* T合計集計データ */
		dst = &z_NtBufState.ttotal;
		break;
	case 131:	/* コイン金庫集計合計データ */
		dst = &z_NtBufState.coin;
		break;
	case 133:	/* 紙幣金庫集計合計データ */
		dst = &z_NtBufState.note;
		break;
	default:	/* バッファ残量監視の必要が無いデータ(バッファFULL時は常に最古データに上書き) */
		dst = NULL;
		break;
	}
	
	if (dst != NULL) {
		if (*dst != state) {
			*dst = state;
			return TRUE;
		}
	}
	
	return FALSE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_GetBufState
 *[]----------------------------------------------------------------------[]
 *| summary	: バッファ状態データ取得
 *| param	: buffer - 対象バッファ
 *| return	: 対象バッファのバッファ状態
 *[]----------------------------------------------------------------------[]*/
uchar	ntbuf_GetBufState(t_NtBuf *buffer)
{
	switch (buffer->id) {
	case 22:	/* 精算データテーブル */
		return z_NtBufState.sale;
	case 21:	/* 出庫データテーブル */
		return z_NtBufState.car_out;
	case 20:	/* 入庫データテーブル */
		return z_NtBufState.car_in;
	case 41:	/* T合計集計データ */
		return z_NtBufState.ttotal;
	case 131:	/* コイン金庫集計合計データ */
		return z_NtBufState.coin;
	case 133:	/* 紙幣金庫集計合計データ */
		return z_NtBufState.note;
	default:	/* バッファ残量監視の必要が無いデータ */
		return NTBUF_BUFFER_NORMAL;
	}
}

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
///*[]----------------------------------------------------------------------[]
// *|	name	: ntbuf_GetSaleBufSetting
// *[]----------------------------------------------------------------------[]
// *| summary	: バッファFULL時の動作設定取得(精算データテーブル)
// *| param	: none
// *| return	: バッファFULL時の動作設定
// *[]----------------------------------------------------------------------[]*/
//long	ntbuf_GetSaleBufSetting(void)
//{
//	return _ntbuf_GetPlaceSetting(CPrmSS[S_NTN][NTPRM_BUFFULL], NTPRM_PLACE_SALE);
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: ntbuf_GetCarInBufSetting
// *[]----------------------------------------------------------------------[]
// *| summary	: バッファFULL時の動作設定取得(入庫データテーブル)
// *| param	: none
// *| return	: バッファFULL時の動作設定
// *[]----------------------------------------------------------------------[]*/
//long	ntbuf_GetCarInBufSetting(void)
//{
//	return _ntbuf_GetPlaceSetting(CPrmSS[S_NTN][NTPRM_BUFFULL], NTPRM_PLACE_CARIN);
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: ntbuf_GetCarInBufSetting
// *[]----------------------------------------------------------------------[]
// *| summary	: バッファFULL時の動作設定取得(出庫データテーブル)
// *| param	: none
// *| return	: バッファFULL時の動作設定
// *[]----------------------------------------------------------------------[]*/
//long	ntbuf_GetCarOutBufSetting(void)
//{
//	return _ntbuf_GetPlaceSetting(CPrmSS[S_NTN][NTPRM_BUFFULL], NTPRM_PLACE_CAROUT);
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: ntbuf_GetTTotalBufSetting
// *[]----------------------------------------------------------------------[]
// *| summary	: バッファFULL時の動作設定取得(T合計集計データ)
// *| param	: none
// *| return	: バッファFULL時の動作設定
// *[]----------------------------------------------------------------------[]*/
//long	ntbuf_GetTTotalBufSetting(void)
//{
//	return _ntbuf_GetPlaceSetting(CPrmSS[S_NTN][NTPRM_BUFFULL], NTPRM_PLACE_TTOTAL);
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: ntbuf_GetCoinBufSetting
// *[]----------------------------------------------------------------------[]
// *| summary	: バッファFULL時の動作設定取得(コイン金庫集計合計データ)
// *| param	: none
// *| return	: バッファFULL時の動作設定
// *[]----------------------------------------------------------------------[]*/
//long	ntbuf_GetCoinBufSetting(void)
//{
//	return _ntbuf_GetPlaceSetting(CPrmSS[S_NTN][NTPRM_BUFFULL], NTPRM_PLACE_COIN);
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: ntbuf_GetNoteBufSetting
// *[]----------------------------------------------------------------------[]
// *| summary	: バッファFULL時の動作設定取得(紙幣金庫集計合計データ)
// *| param	: none
// *| return	: バッファFULL時の動作設定
// *[]----------------------------------------------------------------------[]*/
//long	ntbuf_GetNoteBufSetting(void)
//{
//	return _ntbuf_GetPlaceSetting(CPrmSS[S_NTN][NTPRM_BUFFULL], NTPRM_PLACE_NOTE);
//}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_GetDefaultSetting
 *[]----------------------------------------------------------------------[]
 *| summary	: バッファFULL時の動作設定取得(デフォルト)
 *| param	: none
 *| return	: バッファFULL時の動作設定
 *[]----------------------------------------------------------------------[]*/
long	ntbuf_GetDefaultSetting(void)
{
	return NTNET_BUFSET_DEL_OLD;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_W2Bcpy
 *[]----------------------------------------------------------------------[]
 *| summary	: WORD値をBYTE×2にコピー
 *| param	: bdst - コピー先バイト配列
 *|			  wsrc - コピー元WORD値
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_W2Bcpy(uchar *bdst, ushort wsrc)
{
	*bdst++ = (uchar)(wsrc >> 8);
	*bdst = (uchar)wsrc;
}

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
///*[]----------------------------------------------------------------------[]
// *|	name	: ntbuf_IsSendData
// *[]----------------------------------------------------------------------[]
// *| summary	: 遠隔NT-NETなら各データの送信可否をチェック、送信不要ならセットせずに終了
// *| param	: ID:データID
// *| return	: 1:OK / 0:NG
// *[]----------------------------------------------------------------------[]*/
//char ntbuf_IsSendData(uchar id)
//{
//	uchar	send[12];
//	char	ret = 1;
//	int		i;
//
//	if (_is_ntnet_remote()) {
//		for (i = 0; i < 6; i++) {
//			send[i] = (uchar)prm_get( COM_PRM,S_NTN,61,1,(char)(6-i) );
//		}
//		for (i = 0; i < 6; i++) {
//			send[i+6] = (uchar)prm_get( COM_PRM,S_NTN,62,1,(char)(6-i) );
//		}
//		switch (id) {
//		case 20:	// 入庫データ
//			if (send[0]) ret = 0;
//			break;
//		case 22:	// 
//		case 23:	// 精算データ
//			if (send[1]) ret = 0;
//			break;
//		case 120:	// エラーデータ
//			if (send[3]) ret = 0;
//			break;
//		case 121:	// アラームデータ
//			if (send[4]) ret = 0;
//			break;
//		case 122:	// モニタデータ
//			if (send[5]) ret = 0;
//			break;
//		case 123:	// 操作モニタデータ
//			if (send[6]) ret = 0;
//			break;
//		case 131:	// コイン金庫集計データ
//			if (send[7]) ret = 0;
//			break;
//		case 133:	// 紙幣金庫集計データ
//			if (send[8]) ret = 0;
//			break;
//		case 236:	// 駐車台数データ
//			if (send[9]) ret = 0;
//			break;
//		case 126:	// 金銭管理データ
//			if (send[11]) ret = 0;
//			break;
//		default:	// 集計データ
//			if ((id >= 30 && id <= 38) || id == 41) {
//				if (send[2]) ret = 0;
//			}
//			break;
//		}
//	}
//	return ret;
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: NTBUF_DataClr
// *[]----------------------------------------------------------------------[]
// *| summary	: NT-NETバッファ内の指定データを削除
// *| param	: req  削除要求データID
// *| return	: none
// *[]----------------------------------------------------------------------[]*/
//static const int buf_order[] = {
//	NTNET_SNDBUF_CAR_IN,			// 入庫データテーブル
//	NTNET_SNDBUF_TTOTAL,			// T合計集計データ
//	NTNET_SNDBUF_SALE,				// 精算データテーブル
//	NTNET_SNDBUF_COIN,				// コイン金庫集計合計データ
//	NTNET_SNDBUF_ERROR,				// エラーデータ
//	NTNET_SNDBUF_NOTE,				// 紙幣金庫集計合計データ
//	NTNET_SNDBUF_ALARM,				// アラームデータ
//	NTNET_SNDBUF_CAR_COUNT,			// 駐車台数データ
//	NTNET_SNDBUF_MONITOR,			// モニタデータ
//	NTNET_SNDBUF_MONEY,				// 金銭管理データ
//	NTNET_SNDBUF_OPE_MONITOR,		// 操作モニタデータ
//};
//
//void	NTBUF_DataClr(int req)
//{
//	int no;
//	
//	if (req == -1) {
//		ntbuf_InitNtBuffer(z_NtSndBuf, z_NtSndBufInfo, NTNET_SNDBUF_MAX, 1);	// NT-NETデータ送信バッファ初期化
///
//	}
//	else {
//		no = buf_order[req];
//		z_NtSndBuf[no].save        = z_NtSndBufInfo[no].save;
//		z_NtSndBuf[no].buffer      = z_NtSndBufInfo[no].pool;
//		z_NtSndBuf[no].buffer_size = z_NtSndBufInfo[no].size;
//		z_NtSndBuf[no].setting     = z_NtSndBufInfo[no].setting;
//		z_NtSndBuf[no].datasize    = z_NtSndBufInfo[no].datasize + 3 + sizeof(t_NtBufDataHeader);  // "+3"=ID3,ID4,データ保持フラグ
//		z_NtSndBuf[no].id          = z_NtSndBufInfo[no].id;
//		
//		// 停電保証データもクリア
//		z_NtSndBuf[no].fukuden.kind = NTBUF_FUKUDEN_CLR;
//		z_NtSndBuf[no].wofs = 0;
//		z_NtSndBuf[no].rofs = 0;
//		z_NtSndBuf[no].fukuden.kind = NTBUF_FUKUDEN_NONE;
//
//		// バッファ状態データ作成
//		ntbuf_ChkBufState(&z_NtSndBuf[no]);
//	}
//}
/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_InitSendDataCtrl
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信制御データ初期化処理
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static	void	ntbuf_InitSendDataCtrl(void)
{
	// 送信制御データの初期化
	memset(&NTNET_SendCtrl, 0, sizeof(NTNET_SendCtrl));
	NTNET_SendCtrl[0].data = NtSendPriorData;			// 優先NTNETデータバッファ
	NTNET_SendCtrl[0].type = NTNET_PRIOR_DATA;			// 優先データ(通信部へ渡す種別)
	NTNET_SendCtrl[0].logid = eLOG_MAX;					// 送信対象ログ eLOG_MAX:ログデータ以外
	
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（遠隔と端末間通信のバッファを別にする）
//	NTNET_SendCtrl[1].data = RAU_NtLogData;				// 通常NTNETデータバッファ
	NTNET_SendCtrl[1].data = NTNET_NtLogData;			// 通常NTNETデータバッファ
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（遠隔と端末間通信のバッファを別にする）
	NTNET_SendCtrl[1].type = NTNET_NORMAL_DATA;			// 通常データ(通信部へ渡す種別)
	NTNET_SendCtrl[1].logid = eLOG_MAX;					// 送信対象ログ eLOG_MAX:ログデータ以外
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_ClearSendDataCtrl
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信制御データクリア処理
 *| param	: pCtrl		: クリアする送信制御データ
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static	void	ntbuf_ClearSendDataCtrl(t_Ntnet_SendDataCtrl* pCtrl)
{
	if(pCtrl) {
		pCtrl->length = 0;								// 送信データ長
		pCtrl->logid = eLOG_MAX;						// 送信対象ログ eLOG_MAX:ログデータ以外
		pCtrl->buffull_retry = 0;						// 送信リトライ回数
		pCtrl->buffull_timer = 0;						// 送信リトライタイマ
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_EraseSendData
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信データを削除する
 *|				ログの場合は送信済にする
 *|				送信バッファデータの場合は何もしない
 *| param	: pCtrl		: 削除する送信データの制御データ
 *|			  force		: 強制削除フラグ(T合計、GT合計のみ有効)
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTBUF_EraseSendData(t_Ntnet_SendDataCtrl* pCtrl, BOOL force)
{
	ushort	totalLogID = eLOG_MAX;
	
	if(pCtrl->logid != eLOG_MAX) {						// 送信データはログデータ
		// ログデータの場合は送信バッファに格納した時点で送信済にする
		if(pCtrl->logid != eLOG_TTOTAL && pCtrl->logid != eLOG_GTTOTAL) {	// T合計, GT合計以外
			Ope_Log_TargetVoidReadPointerUpdate(pCtrl->logid, eLOG_TARGET_NTNET);
		}
		else {											// T合計, GT合計
			if(z_NtBufTotalDataIndex == NTBUF_NOTSENDTOTALDATA || force) {
				// 終了通知(ID:41)まで処理済または、強制削除の場合は送信済にする
				Ope_Log_TargetVoidReadPointerUpdate(pCtrl->logid, eLOG_TARGET_NTNET);
				if(force) {
					z_NtBufTotalDataIndex = NTBUF_NOTSENDTOTALDATA;
				}
			}
			else {										// T, GT合計続きのデータあり
				totalLogID = pCtrl->logid;
			}
		}
	}
	else {												// ログデータ以外
		// ログデータ以外なら既にバッファから削除されている
	}
	
	ntbuf_ClearSendDataCtrl(pCtrl);						// 送信制御データを初期化
	if(totalLogID == eLOG_TTOTAL || totalLogID == eLOG_GTTOTAL) {	// T合計, GT合計以外
		pCtrl->logid = totalLogID;						// 待避したログIDを設定する
	}
}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
