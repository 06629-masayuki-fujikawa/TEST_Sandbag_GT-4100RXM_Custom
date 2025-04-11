#ifndef	___NTNETH___
#define	___NTNETH___
/*[]----------------------------------------------------------------------[]
 *| System      : UT8500
 *| Module      : NT-NET通信モジュール共通定義
 *[]----------------------------------------------------------------------[]
 *| Author      : machida kei
 *| Date        : 2005.08.16
 *| Update      :
 *|		2005.09.07	machida.k	NT-NETバッファデータ全クリアIF(NTBUF_AllClr)追加
 *|		2005.09.21	machida.k	NT-NETバッファ状態取得IF(NTBUF_GetBufState)追加
 *|								バッファFULL/ニアFULL対応のため NTNET_BUFSET_STATE_CHG 追加
 *|		2005-11-30	machida.k	STEP2対応
 *|		2006-01-17	machida.k	アイドル負荷軽減 (MH544401)
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*			C O N S T A N T S												*/
/*--------------------------------------------------------------------------*/

/* バッファFULL時の扱い */
#define	NTNET_BUFSET_DEL_OLD	0x00	/* 旧消去 */
#define	NTNET_BUFSET_DEL_NEW	0x01	/* 新消去 */
#define	NTNET_BUFSET_CANT_DEL	0x02	/* 休業 */

#define	NTNET_BUFSET_COMERR		0x80	/* 通信障害中のためバッファリングしない */
#define	NTNET_BUFSET_NORMAL		0xFF	/* バッファFULLでない */
#define	NTNET_BUFSET_STATE_CHG	0xA0	/* バッファ状態変化(FULLorニアFULL発生) */


// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
///* FREEデータサイズ */
//#define	FREEPKT_DATASIZE		500
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）

/* NT-NETブロックデータサイズ */
#define	NTNET_BLKDATA_MAX		960

/* ブロック数MAX */
#define	NTNET_BLKMAX_NORMAL		26		/* 通常データ */
#define	NTNET_BLKMAX_PRIOR		1		/* 優先データ */
#define	NTNET_BLKMAX_BROADCAST	1		/* 同報データ */

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
///* NTUPR_SetRcvPkt()戻り値 */
//typedef enum {
//	NTNET_RRSLT_NORMAL,					/* 正常完了 */
//	NTNET_RRSLT_BLKNO_INVALID,			/* ブロックNo.異常(1NTブロック受信の場合のみ) */
//	NTNET_RRSLT_BLKNO_VALID,			/* ブロックNo.異常(1NTブロック受信の場合のみ) */
//}eNTNET_RRSLT;
//
///* NTLWR_SendReq()戻り値 */
//typedef enum {
//	NTNET_SRSLT_NORMAL,					/* ARCブロック送受信結果＝00H、01H、86Hのとき */
//										/* FREEデータの場合は送信結果データ受信のとき */
//	NTNET_SRSLT_BUFFULL,				/* ARCブロック送受信結果＝82Hのとき */
//	NTNET_SRSLT_BLKNO,					/* ARCブロック送受信結果＝85Hのとき */
//	NTNET_SRSLT_SENDING,				/* データ送信中(リモートのみ)=90Hのとき  */
//	NTNET_SRSLT_RETRY_OVER				/* リトライオーバー発生 */
//}eNTNET_SRSLT;
//
///* NTUPR_SetRcvPkt()／NTLWR_SendReq() 引数 */
//typedef enum {
//	NTNET_FREEDATA,						/* FREEデータ */
//	NTNET_ERRDATA,						/* エラーデータ */
//	NTNET_NTDATA						/* NT-NETデータ */
//}eNTNET_DATA_KIND;
typedef enum {							// NTNET, NTComタスク間API結果
	NTNET_NORMAL,						// 正常
	NTNET_DEL_OLD,						// 最古データ上書き
	NTNET_ERR_BUFFERFULL,				// バッファフル
	NTNET_ERR_BLOCKOVER,				// NTNETブロックオーバー（優先、同報：1ブロック、通常、26ブロック以上のデータ）
	NTNET_ERR_OTHER,					// その他のエラー
} eNTNET_RESULT;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）

/* NT-NETデータバッファ大別 */
typedef enum {
	NTNET_BUF_BROADCAST,				/* 同報データ(受信バッファのみ) */
	NTNET_BUF_PRIOR,					/* バッファリングデータではない優先データ */
	NTNET_BUF_BUFFERING,				/* バッファリングデータ(送信バッファのみ) */
	NTNET_BUF_NORMAL,					/* バッファリングデータではない通常データ */
	NTNET_BUF_INIT = 0xff				/* 初期値 */
}eNTNET_BUF_KIND;

/* バッファ毎のデータ保持情報 */
typedef enum {
	NTBUF_SAVE,							/* データ保持する */
	NTBUF_NOT_SAVE,						/* データ保持しない */
	NTBUF_SAVE_BY_DATA,					/* データ保持するかしないかは各データによる */
}eNTBUF_SAVE;

/* 転送先 端末No. */
#define	NTNET_BROADCAST			0xFF	/* 同報セレクティングのデータ */

/* パケット優先モード */
#define	NTNET_NORMAL_DATA		0x00	/* 通常データ */
#define	NTNET_PRIOR_DATA		0x01	/* 優先データ */

/* 電文種別 */
#define	STX						0x02

/* 最終ブロック判定 */
#define	NTNET_NOT_LAST_BLOCK	0x00	/* 中間ブロック */
#define	NTNET_LAST_BLOCK		0x01	/* 最終ブロック */

/* データ保持フラグ */
#define	NTNET_DO_BUFFERING		0x00	/* バッファリングする */
#define	NTNET_NOT_BUFFERING		0x01	/* バッファリングしない */

/* 設定パラメータ */
#define	NTPRM_TERMINALNO		3		/* ターミナルNo. */
#define	NTPRM_RESPONSE_TIME		19		/* MAINモジュールからの応答待ち時間 */
#define	NTPRM_RETRY				21		/* MAINモジュールのリトライ回数 */
#define	NTPRM_BUFFULL_TIMER		23		/* 再送要求時の送信Wait */
#define	NTPRM_BUFFULL_RETRY		24		/* 再送エラー判定回数 */
#define	NTPRM_BUFFULL			41		/* 送信データバッファFULL時の扱い */
/* 設定パラメータ41 各データに対応する位 */
#define	NTPRM_PLACE_SALE	1
#define	NTPRM_PLACE_CARIN	10
#define	NTPRM_PLACE_CAROUT	100
#define	NTPRM_PLACE_TTOTAL	1000
#define	NTPRM_PLACE_COIN	10000
#define	NTPRM_PLACE_NOTE	100000

/* NTNET_Err() 引数 */
#define	NTERR_EMERGE	1				/* 発生 */
#define	NTERR_RELEASE	0				/* 解除 */
#define	NTERR_ONESHOT	2				/* 発生＆解除 */

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
///* FREEデータ識別コード */
//#define	FREEDATA_SYMBOL			"FREE"
//
///* FREEデータ要求種別 */
//#define	FREEDATA_KIND_INITDAT	1		/* 初期設定データ要求 */
//#define	FREEDATA_KIND_IBKSTS	2		/* IBK状態要求 */
//#define	FREEDATA_KIND_DATCNT	3		/* 送信データ件数要求 */
//#define	FREEDATA_KIND_TERMSTS	4		/* 端末状態要求 */
//#define	FREEDATA_KIND_DATACLR	5		/* データクリア要求 */
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）

/* NT-NETバッファ状態 */
#define	NTBUF_BUFFER_NORMAL		0x00
#define	NTBUF_BUFFER_NEAR_FULL	0x01
#define	NTBUF_BUFFER_FULL		0x02

// MH322914(S) K.Onodera 2016/08/08 AI-V対応
#define		PIP_SYSTEM		16
// MH322914(E) K.Onodera 2016/08/08 AI-V対応
#define		REMOTE_SYSTEM	30
#define		REMOTE_DL_SYSTEM	40
#define		REMOTE_PC_TERMINAL_NO	99

#define		REMOTE_AUTO		0
#define		REMOTE_MANUAL	1

/*--------------------------------------------------------------------------*/
/*			S T R U C T U R E S												*/
/*--------------------------------------------------------------------------*/

/* NT-NET通信データのヘッダ(データ部基本項目) */
typedef struct {
	uchar	system_id;					/* システムID */
	uchar	data_kind;					/* データ種別 */
	uchar	data_save;					/* データ保持フラグ */
	uchar	seq_no;						/* シーケンシャルNo. */
	uchar	parking_no[4];				/* 駐車場No. */
	uchar	machine_code[2];			/* 機種コード */
	uchar	terminal_no[4];				/* 端末機械No. */
	uchar	serial_no[6];				/* 端末シリアルNo. */
	uchar	year;						/* 処理年 */
	uchar	month;						/* 処理月 */
	uchar	day;						/* 処理日 */
	uchar	hour;						/* 処理時 */
	uchar	minute;						/* 処理分 */
	uchar	second;						/* 処理秒 */
}t_NtNetDataHeader;

/* 1NTNETブロックのヘッダー部 */
typedef struct {
//	uchar	sig[5];						/* "NTCOM" */
//	uchar	len;						/* データサイズ */
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
	uchar	sig[5];						/* "NTCOM" */
	uchar	len[2];						/* データサイズ */
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
	uchar	vect;						/* 転送方向 */
	uchar	terminal_no;				/* 端末No. */
	uchar	lower_terminal_no1;			/* 下位転送用端末No.(1) */
	uchar	lower_terminal_no2;			/* 下位転送用端末No.(2) */
	uchar	link_no;					/* パケット SEQ No. */
	uchar	status;						/* 端末ステータス */
	uchar	broadcast_rcvd;				/* 同報受信済みフラグ */
	uchar	len_before0cut[2];			/* ゼロカット前のデータサイズ */
	uchar	mode;						/* パケット優先モード */
	uchar	reserve;					/* 予備 */
	uchar	telegram_type;				/* STX */
	uchar	res_detail;					/* 応答詳細 */
}t_NtNetBlkHeader;

/* 1NT-NETブロックのデータ部 */
typedef struct {
	uchar	blk_no;						/* ID1 */
	uchar	is_lastblk;					/* ID2 */
	uchar	system_id;					/* ID3 */
	uchar	data_type;					/* ID4 */
	uchar	save;						/* データ保持フラグ */
	uchar	data[NTNET_BLKDATA_MAX];	/* データ部 */
}t_NtNetBlkData;

// (以下、ntnet_upper.cより移動)
typedef struct {
	t_NtNetBlkHeader	header;		/* ヘッダ部 */
	t_NtNetBlkData		data;		/* データ部 */
}t_NtNetBlk;

// (以下、新規作成)
typedef struct {
	uchar	packet_size[2];				/* パケットサイズ */
	uchar	len_before0cut[2];			/* ゼロカット前のデータサイズ */
	uchar	reserve;					/* 予備 */
}t_RemoteNtNetBlkHeader;

typedef struct {
	t_RemoteNtNetBlkHeader	header;		/* ヘッダ部 */
	t_NtNetBlkData		data;			/* データ部 */
}t_RemoteNtNetBlk;

/* 受信データ1件削除用のハンドル */
typedef struct {
	int				bufcode;			/* バッファNo. */
	long			rofs;				/* ハンドル取得時点でのバッファ読み出しオフセット */
}t_NtNet_ClrHandle;

/* バッファ状態(NTBUF_BUFFER_NORMAL/NTBUF_BUFFER_FULL/NTBUF_BUFFER_NEAR_FULL のOR情報 */
typedef struct {
	uchar	sale;							/* 精算データテーブル(ID22 or 23) */
	uchar	car_out;						/* 出庫データテーブル(ID21) */
	uchar	car_in;							/* 入庫データテーブル(ID20) */
	uchar	ttotal;							/* T合計集計データ(ID30～38、41) */
	uchar	coin;							/* コイン金庫集計合計データ(ID131) */
	uchar	note;							/* 紙幣金庫集計合計データ(ID133) */
}t_NtBufState;

/* バッファ内のデータ数 */
typedef struct {
	ulong sndbuf_prior;				/* その他の優先データ残数 */
	ulong sndbuf_sale;				/* 精算データ残数(ID22orID33) */
	ulong sndbuf_ttotal;			/* T合計集計データ残数(ID30～38、ID41) */
	ulong sndbuf_coin;				/* コイン金庫集計合計データ残数(ID131) */
	ulong sndbuf_note;				/* 紙幣金庫集計合計データ残数(ID133) */
	ulong sndbuf_error;				/* エラーデータ残数(ID120) */
	ulong sndbuf_alarm;				/* アラームデータ残数(ID121) */
	ulong sndbuf_money;				/* 金銭管理データ残数(ID126) */
	ulong sndbuf_monitor;			/* モニタデータ残数(ID122) */
	ulong sndbuf_ope_monitor;		/* 操作モニタデータ残数(ID123) */
	ulong sndbuf_normal;			/* その他の通常データ残数 */
	ulong rcvbuf_broadcast;			/* 同報データ受信バッファ残数 */
	ulong rcvbuf_prior;				/* 優先データ受信バッファ残数 */
	ulong rcvbuf_normal;			/* 通常データ受信バッファ残数 */
	ulong sndbuf_incar;				/* 入庫データ残数(ID20) */
	ulong sndbuf_outcar;			/* 出庫データテーブル(ID21)    */
	ulong sndbuf_car_count;			/* 駐車台数データ残数(ID236)   */
	ulong sndbuf_area_count;		/* 区画台数・満車データ(ID237) */
	ulong sndbuf_turi;				/* 釣銭管理集計データ残数(ID135) */
	ulong sndbuf_rmon;				/* 遠隔監視データ残数(ID125) */
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	ulong sndbuf_lpark;				/* 長期駐車状態データ残数(ID61) */
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
}t_NtBufCount;

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
typedef struct {
	ushort				length;						// データ長
	uchar				*data;						// データ本体
	uchar				type;						// データ種別
	ushort				logid;						// ログID
	long				buffull_retry;				// IBKバッファFULLによる送信リトライ回数
	ulong				buffull_timer;				// IBKバッファFULLによる送信リトライ用タイマー
}t_Ntnet_SendDataCtrl;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/
/*------------------------------------------------------------ ntnet_lower.c ----*/

void	NTNET_Init(uchar clr);
void	NTNET_Err(int errcode, int occur);
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
///* アイドル負荷軽減 */
//#define	_NTNET_ChkDispatch()	(NTUPR_ChkDispatch() || _NTLWR_ChkDispatch())
//
///*------------------------------------------------------------ ntnet_lower.c ----*/
//
//void	NTLWR_Init(void);
//void	NTLWR_Main(MsgBuf *pmsg);
//BOOL	NTLWR_SendReq(uchar *data, ushort len, eNTNET_DATA_KIND kind);
//BOOL	NTLWR_IsSendComplete(eNTNET_SRSLT *result);
///* アイドル負荷軽減 */
//extern	BOOL	NtLwr_Dispatch;
//#define	_NTLWR_ChkDispatch()	(NtLwr_Dispatch)
//
///*------------------------------------------------------------ ntnet_upper.c ----*/
//
//void	NTUPR_Init(uchar clr);
//void	NTUPR_Main(void);
//eNTNET_RRSLT	NTUPR_SetRcvPkt(uchar *data, ushort len, eNTNET_DATA_KIND kind);
///* アイドル負荷軽減 */
//BOOL	NTUPR_ChkDispatch(void);
//
//uchar	NTUPR_SendingPacketKindGet( ushort UpperQueKind );
//
//void	NTUPR_Init2( void );
extern	void	NTNET_Main(void);
extern	eNTNET_RESULT	NTNET_SetReceiveData(const uchar* pNtData, ushort size, uchar type);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）

/*------------------------------------------------------------ ntnet_buffer.c ----*/

void	NTBUF_Init(uchar clr);
void	NTBUF_AllClr(void);
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
void	NTBUF_AllClr_startup(void);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
uchar	NTBUF_SetSendNtData(const void *data, ushort len, eNTNET_BUF_KIND bufkind);
ushort	NTBUF_GetRcvNtData(void *data, eNTNET_BUF_KIND bufkind);
void	NTBUF_ClrRcvNtData_Prepare(eNTNET_BUF_KIND bufkind, t_NtNet_ClrHandle *handle);
void	NTBUF_ClrRcvNtData_Exec(const t_NtNet_ClrHandle *handle);
void	NTBUF_CommErr(uchar sts);
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//void	NTBUF_DataClr(int req);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//ushort	NTBUF_GetSndNtData(void *data, eNTNET_BUF_KIND bufkind, ushort *UpperQueKind);
ushort	NTBUF_GetSndNtData(t_Ntnet_SendDataCtrl* pCtrl, eNTNET_BUF_KIND bufkind);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
uchar	NTBUF_SetRcvNtData(const void *data, ushort len, eNTNET_BUF_KIND bufkind);
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//void	NTBUF_ClrSndNtData_Prepare(const void *data, eNTNET_BUF_KIND bufkind, t_NtNet_ClrHandle *handle);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
void	NTBUF_ClrSndNtData_Exec(const t_NtNet_ClrHandle *handle);

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//uchar	NTBUF_SetSendFreeData(const void *data);
//uchar	NTBUF_GetRcvFreeData(void *data);
//uchar	NTBUF_SetRcvFreeData(const void *data);
//uchar	NTBUF_GetSndFreeData(void *data);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

void	NTBUF_SetRcvErrData(const void *data);

const t_NtBufState	*NTBUF_GetBufState(void);

void NTBUF_SetIBKNearFull(ulong mask, ulong occur);
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//void NTBUF_SetIBKNearFullByID(uchar ID);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
void NTBUF_SetIBKPendingByID(uchar ID);
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
void	NTBUF_EraseSendData(t_Ntnet_SendDataCtrl* pCtrl, BOOL force);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）


/* アイドル負荷軽減 */
extern	BOOL z_NtBuf_SndDataExist;
#define	_NTBUF_SndDataExist()	(z_NtBuf_SndDataExist)

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
//void	NTBUF_GetBufCount(t_NtBufCount *buf);
void	NTBUF_GetBufCount(t_NtBufCount *buf, BOOL isRemote);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

ushort	ntupr_NtNetDopaSndBufCheck_Del(uchar bufkind, uchar status);

void	NTNET_GetRevData( ushort msg, uchar * );
void	NTNET_RevData01( void );
void	NTNET_RevData02( void );
void	NTNET_RevData03( void );
void	NTNET_RevData04( void );
void	NTNET_RevData22( void );
void	NTNET_RevData100( void );
void	NTNET_RevData103( void );
void	NTNET_RevData109( void );
void	NTNET_RevData154( void );
void	NTNET_RevData156( void );							// 同期時刻設定要求
void	NTNET_RevData119( void );
void	NTNET_RevData143( void );
void	BasicDataMake( uchar knd, uchar keep );
void	NTNET_Snd_Data01( ulong op_lokno );
void	NTNET_Snd_Data02( ulong op_lokno, ushort pr_lokno, ushort ans );
void	NTNET_Snd_Data05( ulong op_lokno, uchar loksns, uchar lokst );
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（不要処理削除）
//void	NTNET_Snd_Data12( ulong MachineNo );
//void	NTNET_Snd_Data20( ushort pr_lokno );
//void	NTNET_Snd_Data20_frs( ushort pr_lokno, void *data );
//void	NTNET_Snd_Data22_LO( ushort pr_lokno, ushort paymethod, ushort payclass, ushort outkind );
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（不要処理削除）
void	NTNET_Snd_DataParkCarNumWeb( uchar kind );
void	NTNET_Snd_Data58( void );
void	NTNET_Snd_Data101( ulong MachineNo );
void	NTNET_Snd_Data104( ulong req );
void	NTNET_Snd_Data120( void );
void	NTNET_Snd_Data120_CL( void );
void	NTNET_Snd_Data121( void );
void	NTNET_Snd_Data121_CL( void );
void	NTNET_Snd_Data126( ulong MachineNo, ushort payclass );
void	NTNET_Snd_Data229( void );
void	NTNET_Snd_Data229_Exec( void );
void	NTNET_Snd_Data230( ulong MachineNo );
void	NTNET_Snd_Data231( ulong MachineNo );
void	NTNET_Snd_Data142( ulong pkno, ulong passid );
void	NTNET_Snd_DataFree( void );
void	NTNET_Snd_Data100( void );

void	NTNET_Snd_Data90(uchar type);

void	NTNET_RevData80( void );
void	NTNET_RevData91( void );
void	NTNET_RevData93( void );
void	NTNET_RevData94( void );
void	NTNET_RevData95( void );
void	NTNET_RevData97( void );
void	NTNET_RevData98( void );
void	NTNET_RevData110( void );
void	NTNET_RevData116( void );
void	NTNET_RevData117( void );
void	NTNET_Snd_Data99(ulong MachineNo, int CompleteKind);
void	NTNET_Snd_Data101_2( void );
void	NTNET_Snd_Data109( ulong req, char value );
void	NTNET_Snd_Data116( uchar ProcMode, ulong PassId, ulong ParkingId, uchar UseParkingKind, uchar Status, uchar UpdateStatus, date_time_rec *ExitTime );
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（不要処理削除）
//void	NTNET_Snd_Data122( uchar kind, uchar code, uchar level, uchar *info, uchar *message );
//void	NTNET_Snd_Data123( uchar kind, uchar code, uchar level, uchar *before, uchar *after, uchar *message );
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（不要処理削除）
void	NTNET_Snd_Data208(ulong mac, ushort sts, ushort seg);
void	_NTNET_Snd_Data208(ulong mac, ushort sts, ushort seg, ushort addr, ushort count);
void	NTNET_RevData83( void );
void	NTNET_Snd_Data84(ulong MachineNo, uchar ng);
void	NTNET_Snd_Data219(uchar Mode, struct TKI_CYUSI *Data);
void	NTNET_Snd_Data221( void );
void	NTNET_Snd_Data222( void );
void	NTNET_Snd_Data223( void );
void	NTNET_Snd_Data225( ulong MachineNo );
void	NTNET_Snd_Data226( ulong MachineNo );
void	NTNET_RevData66( void );
void	NTNET_RevData77( void );
void	NTNET_RevData194( void );
void	NTNET_RevData205( void );
void	NTNET_Snd_TGOUKEI( void );
void	NTNET_Snd_TSYOUKEI( void );
void	NTNET_RevData78( void );
void	NTNET_RevData114( void );
void	NTNET_RevData234( void );
char	NTNET_is234StateValid(void);

void	NTNET_RevData92(void);
void	NTNET_RevData240(void);
// MH810100(S) K.Onodera 2019/12/19 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//void	NTNET_RevData243(void);
// MH810100(E) K.Onodera 2019/12/19 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

void	NTNET_RevData80_R(void);
void	NTNET_RevData90_R(void);
void	NTNET_RevData116_R(void);

void	NTNET_Snd_Data115( ulong MachineNo, uchar ProcMode );
void	NTNET_Snd_Data130( void );
void	NTNET_Snd_Data132( void );
void	NTNET_Snd_Data211( void );
void	NTNET_Snd_Data211_Exec( void );
// GM849100(S) M.Fujikawa 2025/01/15 名鉄協商コールセンター対応（NT-NET端末間通信）
//void	NTNET_Snd_Data228( ulong MachineNo );
void	NTNET_Snd_Data228( ulong MachineNo, uchar from );
// GM849100(E) M.Fujikawa 2025/01/15 名鉄協商コールセンター対応（NT-NET端末間通信）
void	NTNET_Snd_Data235(char bReq);
void	NTNET_Snd_Data235_Exec(char bReq);
void	NTNET_Snd_Data131(void);
void	NTNET_Snd_Data133(void);
void	NTNET_Snd_DataParkCarNum(uchar kind);
void	NTNET_Snd_Data236(void);
void	NTNET_Snd_Data105(ulong MachineNo, ushort code);
void	NTNET_Snd_Data110(ulong MachineNo, ushort code);
void	NTNET_Snd_Data155(ulong MachineNo, ushort code);	// センタ用データ要求NG送信
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
//void	NTNET_Snd_Data152(void);							// 精算状態データ
// MH810105(S) MH364301 QRコード決済対応
//void	NTNET_Snd_Data152_r12(void);						// 精算状態データ
void	NTNET_Snd_Data152_rXX(void);						// 精算状態データ
// MH810105(E) MH364301 QRコード決済対応
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
void	NTNET_Snd_Data153(void);							// リアルタイム情報
void	NTNET_Snd_Data157(ulong MachineNo, ushort code, uchar setget);	// 同期時刻設定要求 結果
// MH810100(S) K.Onodera 2019/12/19 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//void	NTNET_Snd_Data238(void);
//void	NTNET_Snd_Data244(ushort code);
// MH810100(E) K.Onodera 2019/12/19 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
void	NTNET_Snd_Data245(ushort GroupNo);
void	BasicDataMake_R( uchar knd, uchar keep );
void	NTNET_Snd_Data60_R(ulong req);
void	NTNET_Snd_DataTblCtrl(uchar id, ulong req, uchar bAuto);
void	NTNET_Snd_Data100_R(uchar mode, uchar chk_num);
void	NTNET_RevData83_R(void);
void	NTNET_Rev_Data101_R(void);

void	NTNET_ClrSetup(void);
void	NTNET_RevData78( void );

void	NTNET_RevData150( void );
void	NTNET_RevData254( void );
void	NTNET_RevData84( void );
void	NTNET_Snd_Data108(ushort KakariNo, uchar Status);

void	NTNET_Snd_Data190( void );
void	NTNET_RevData188( void *);
void	NTNET_Snd_Data189( uchar err_code, uchar data );
void	NTNET_Ans_Data99(uchar SysID, ulong MachineNo, int CompleteKind, uchar CompleteInfo);
void	NTNET_Ans_Data115(uchar SysID, ulong MachineNo, uchar ng);
void	NTNET_Ans_Data208(uchar SysID, ulong MachineNo, ushort addr, ushort count, uchar okng);
ulong	OrgCal( ushort Index );
void	regist_mon(ushort code, ushort error, ushort param, ushort num);
int		check_remote_addr(ushort kind, ushort addr, ushort count);
int		update_param(void);
uchar	cyushi_chk( ushort no );
// MH322914 (s) kasiyama 2016/07/15 AI-V対応
void	NTNET_Snd_Data16_04( ushort rslt );
void	NTNET_Snd_Data16_08( ushort rslt );
void	NTNET_Snd_Data16_09( ushort rslt );
void	PiP_FurikaeEnd( void );
void	PiP_FurikaeStart( void );
ushort PiP_GetFurikaeSts( void );
void	PiP_FurikaeInit( void );
// MH322914 (e) kasiyama 2016/07/15 AI-V対応

#define NTNET_Snd_Data61_R(req, sauto)	NTNET_Snd_DataTblCtrl(61, req, sauto)
#define NTNET_Snd_Data62_R(req)	NTNET_Snd_DataTblCtrl(62, req, 0)

extern t_NtBufCount IBK_BufCount;
extern uchar		IBK_ComChkResult;
extern uchar		stNtBufCountOffset[];

#define REMOTE_IBKCTRL_MAX				3
#define	REMOTE_IBKCTRL_TBLNUM			0x00000001		// テーブル件数データ
#define	REMOTE_IBKCTRL_SET1				0x00000002		// 遠隔IBK設定データ１要求
#define	REMOTE_IBKCTRL_SET2				0x00000004		// 遠隔IBK設定データ２要求

// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//#define NTNET_BUFCTRL_REQ_MAX			15
#define NTNET_BUFCTRL_REQ_MAX			16
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
#define	NTNET_BUFCTRL_REQ_INCAR			0x00000001		// 入庫データ
#define	NTNET_BUFCTRL_REQ_OUTCAR		0x00000002		// 出庫データ
#define	NTNET_BUFCTRL_REQ_SALE			0x00000004		// 精算データ
#define	NTNET_BUFCTRL_REQ_TTOTAL		0x00000008		// T合計集計データ
#define	NTNET_BUFCTRL_REQ_ERROR			0x00000010		// エラーデータ
#define	NTNET_BUFCTRL_REQ_ALARM			0x00000020		// アラームデータ
#define	NTNET_BUFCTRL_REQ_MONITOR		0x00000040		// モニタデータ
#define	NTNET_BUFCTRL_REQ_OPE_MONITOR	0x00000080		// 操作モニタデータ
#define	NTNET_BUFCTRL_REQ_COIN			0x00000100		// コイン金庫集計合計データ
#define	NTNET_BUFCTRL_REQ_NOTE			0x00000200		// 紙幣金庫集計合計データ
#define	NTNET_BUFCTRL_REQ_CAR_COUNT		0x00000400		// 駐車台数データ
#define	NTNET_BUFCTRL_REQ_AREA_COUNT	0x00000800		// 区画台数・満車データ
#define	NTNET_BUFCTRL_REQ_MONEY			0x00001000		// 金銭管理データ
#define	NTNET_BUFCTRL_REQ_TURI 			0x00002000		// 釣銭管理集計データ
#define	NTNET_BUFCTRL_REQ_RMON 			0x00004000		// 遠隔監視データ
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//#define	NTNET_BUFCTRL_REQ_ALL		0x00007fff		// 全データ
#define	NTNET_BUFCTRL_REQ_LONG_PARK		0x00008000		// 長期駐車状態データ
// MH810100(S) Y.Yamauchi 20191030 車番チケットレス(メンテナンス)
//#define	NTNET_BUFCTRL_REQ_ALL			0x0000ffff		// 全データ
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//#define	NTNET_BUFCTRL_REQ_ALL			0x0000fffe		// 入庫データ以外の全データ
#define	NTNET_BUFCTRL_REQ_ALL			0x000073fc		// 入庫/出庫/駐車台数/区画台数・満車/長期駐車状態データ以外の全データ
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
// MH810100(E) Y.Yamauchi 20191030 車番チケットレス(メンテナンス)
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
// Phase1には釣銭管理がないため、全データは以下の値となる
// MH810100(S) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//#define	NTNET_BUFCTRL_REQ_ALL_PHASE1	0x00001fff		// 全データ(Phase1)
#define	NTNET_BUFCTRL_REQ_ALL_PHASE1	0x000013fc		// 入庫/出庫/駐車台数/区画台数・満車データ以外の全データ(Phase1)
// MH810100(E) K.Onodera 2019/12/23 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

void	NTNET_Snd_Data63_R(void);
void	NTNET_Snd_Data117_R(uchar mode, char *data);

void	mc10exec( void );

void	NTNET_Snd_Data143(ulong ParkingNo, ulong PassID);	// 定期券問合せ結果データ
void	NTNET_RevData142(void);								// 定期券問合せデータ

typedef	struct {
	uchar		DataKind;
	void		*Src;
} NTNETCTRL_FUKUDEN_PARAM;

extern const	uchar	NTNET_SYSTEM_ID[10];

void	NTNET_UpdateParam(NTNETCTRL_FUKUDEN_PARAM *fukuden);
short	NTNET_isTotalEndReceived( ushort *result );
ulong NTNET_GetParkingNo(uchar ParkingKind);
enum {
	PKOFS_SEARCH_LOCAL,
	PKOFS_SEARCH_MASTER
};
uchar NTNET_GetParkingKind(ulong ParkingId, int search);

extern uchar	Ntnet_Remote_Comm;
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間通信は個別設定を参照する）
extern uchar	Ntnet_Term_Comm;							// NT-NET(名鉄協商コールセンター)使用設定
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間通信は個別設定を参照する）
#define	_ntnet_by_hif		0x10
#define	_is_ntnet_remote()	((Ntnet_Remote_Comm & 0x0f) == 2)
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間通信は個別設定を参照する）
//#define	_is_ntnet_normal()	((Ntnet_Remote_Comm & 0x0f) == 1)
#define	_is_ntnet_normal()	((Ntnet_Term_Comm & 0x0f) == 1)
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間通信は個別設定を参照する）
#define	_is_ntnet			_is_ntnet_normal
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間通信は個別設定を参照する）
//#define	_not_ntnet()		(Ntnet_Remote_Comm == 0)
#define	_not_ntnet()		(Ntnet_Remote_Comm == 0 && Ntnet_Term_Comm == 0)
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間通信は個別設定を参照する）
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間通信は個別設定を参照する）
//#define	_is_ibk_ntnet()		(Ntnet_Remote_Comm == 1)
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間通信は個別設定を参照する）
#define	_is_ibk_ntnet_remote()		(Ntnet_Remote_Comm == 2)
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間と遠隔を併用する）
#define NTNET_TARGET_MAX	2						// 端末間, 遠隔
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間と遠隔を併用する）

extern	void	Remote_Cal_Data_Restor( void );

extern 	char	z_NtRemoteAutoManual;
extern	void NTNET_RAUResult_Send(uchar systemID, uchar dataType, uchar result);

extern	void	NTNET_Snd_Data65( ulong MachineNo );
extern	uchar	GetNtDataSeqNo(void);
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
// 送信データ制御 0:優先データ 1:通常データ
extern	t_Ntnet_SendDataCtrl	NTNET_SendCtrl[2];

extern	ushort	NTNET_ConvertLogToNTNETData(ushort logType, uchar* pLogData, uchar* pNtnetData);
extern	ushort	NTNET_ConvertTotalLogToNTNETData(ushort logType, ushort logID, uchar* pLogData, uchar* pNtnetData);
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
// GM849100(S) M.Fujikawa 2025/01/10 名鉄協商コールセンター対応（NT-NET端末間通信）
void	NTNET_Snd_Data230_T( ulong MachineNo );
// GM849100(E) M.Fujikawa 2025/01/10 名鉄協商コールセンター対応（NT-NET端末間通信）

#endif	/* ___NTNETH___ */

