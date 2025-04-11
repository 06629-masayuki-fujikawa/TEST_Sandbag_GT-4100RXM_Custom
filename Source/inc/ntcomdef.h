// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
// MH364300 GG119A03 (S) NTNET通信制御対応（標準UT4000：MH341111流用）
	// UT4000の "ntcomdef.h"をベースにFT4000の差分を移植して対応
// MH364300 GG119A03 (E) NTNET通信制御対応（標準UT4000：MH341111流用）
#ifndef	___NTCOMDEFH___
#define	___NTCOMDEFH___
/*[]----------------------------------------------------------------------[]
 *|	filename: ntdef.h
 *[]----------------------------------------------------------------------[]
 *| summary	: NTタスク定義データヘッダ
 *| date	: 2013-02-22
 *| update	:
 *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
#include	"system.h"
#include	"common.h"
#include	"mem_def.h"


/*--------------------------------------------------------------------------*/
/*			C O N S T A N T S												*/
/*--------------------------------------------------------------------------*/

/*----------------------------------------- DATA --*/

#define	NT_BLKSIZE				1024		/* 1パケット長MAX */

#define	NT_NORMAL_BLKNUM_MAX	26			/* パケット保存数MAX */
#define	NT_PRIOR_BLKNUM_MAX		1			/* #007 */
#define	NT_BROADCAST_BLKNUM_MAX	1			/* 同報パケット保存数MAX */

#define	NT_ARC_ERRQUE_COUNT		20

typedef enum {
	NT_NORMAL,
	NT_ABNORMAL
}eNT_STS;

/* データ種別 */
typedef enum {
	NT_DATA_KIND_NORMAL_SND,
	NT_DATA_KIND_NORMAL_RCV,
	NT_DATA_KIND_PRIOR_SND,
	NT_DATA_KIND_PRIOR_RCV,
	NT_DATA_KIND_BROADCAST_SND,
	NT_DATA_KIND_BROADCAST_RCV
}eNT_DATA_KIND;

/* データセット関数の戻り値 */
typedef enum {
	NT_DATA_NORMAL,							/* 成功 */
	NT_DATA_BUFFERFULL,						/* バッファが一杯 or NT_NORMAL_BLKNUM_MAX待っても最終ブロックが来ない */
	NT_DATA_BLOCK_INVALID1,					/* ACK02 */
	NT_DATA_BLOCK_INVALID2,					/* ACK03 */
	NT_DATA_NO_MORE_TELEGRAM				/* バッファに既に電文有り */
}eNT_DATASET;

/* ブロックシーク関数のパラメータ */
typedef enum {
	NT_SEEK_SET,							/* 電文先頭を起点 */
	NT_SEEK_CUR,							/* 現在位置を起点 */
	NT_SEEK_END								/* 電文終端を起点 */
}eNT_SEEK;

/*----------------------------------------- PROT --*/

//#define	NT_TERMINALNUM_MAX		8			/* シリアル側の接続端末数MAX */

/* 主局/従局 */
//#define	NT_MASTER				0
#define	NT_SLAVE				1

/* 電文内でのオフセット */
#define	NT_OFS_DATA_SIZE_HI				5			/* 転送方向 */
#define	NT_OFS_DATA_SIZE_LOW			6			/* 転送方向 */
#define	NT_OFS_SEND_REQ_FLAG			7			/* 送信要求フラグ */
#define	NT_OFS_DIRECTION				7			/* 転送方向 */
#define	NT_OFS_TERMINAL_NO				8			/* 転送先 端末No. */
#define	NT_OFS_SLAVE_TERMINAL_NO1		9			/* 下位転送用端末No.(1) */
#define	NT_OFS_SLAVE_TERMINAL_NO2		10			/* 下位転送用端末No.(2) */
#define	NT_OFS_LINK_NO					11			/* 通信リンクNo. */
#define	NT_OFS_TERMINAL_STS				12			/* 端末ステータス */
#define	NT_OFS_BROADCASTED_FLAG			13			/* 同報受信済みフラグ */
#define	NT_OFS_DATASIZE_WITH_ZERO		14			/* ゼロカットする前のデータサイズ */
#define	NT_OFS_PACKET_MODE				16			/* パケット優先モード */
#define	NT_OFS_TELEGRAM_KIND			18			/* 電文種別(STX/ACK/NACK/ENQ/EOT) */
#define	NT_OFS_RESPONSE_DETAIL			19			/* 応答詳細 */
#define	NT_OFS_BLOCK_NO					20			/* ブロックNo. */
#define	NT_OFS_LAST_BLOCK_FLAG			21			/* 最終ブロック判定 */
#define	NT_OFS_SYSTEM_ID				22			/* システムID */
#define	NT_OFS_DATA_KIND				23			/* データ種別 */
#define	NT_OFS_BUFFERING_FLAG			24			/* データ保持フラグ */

// MH586504 2009/05/08 (s) okuda 時計データミリ秒対応
/* 新時計データ(ID=119/229)電文情報 */
#define	NT_TMPKT_ID1					119			/* 時計データ 電文種別 */
#define	NT_TMPKT_ID2					229			/* 時計データ 電文種別 */
#define	NT_TMPKT_OLD_TYPE_PKT_DATA_LEN	33			/* 時計データ 旧電文長（伝送データ部長） */
#define	NT_TMPKT_NEW_TYPE_PKT_DATA_LEN	41			/* 時計データ 新電文長（伝送データ部長） */
#define	NT_TMPKT_NEW_TYPE_PKT_TOTAL_LEN	(NT_TMPKT_NEW_TYPE_PKT_DATA_LEN + 25)
													/* 時計データ 新電文長（総データ長：ヘッダ含む） */
#define	NT_OFS_DATA_TMPKT_HOSEI			58			/* 時計データ(ID=119/229) 補正値 */
#define	NT_OFS_DATA_TMPKT_FREETIMER		62			/* 時計データ(ID=119/229) 受信時フリータイマー値セットエリア */

// MH586504 2009/05/08 (e) okuda 時計データミリ秒対応

/* 転送方向 */
#define	NT_DIRECTION_TO_MASTER			0x00		/* 下位→上位 */
#define	NT_DIRECTION_TO_SLAVE			0x01		/* 上記→下位 */
/* 転送先 端末No. */
#define	NT_ALL_TERMINAL					0x00		/* 全端末へ(同報ではなく)送信するデータ */
#define	NT_TERMINAL_UNKOWN				0xF0		/* 端末No.不明(エラーデータのパラメータとしてのみ使用) */
#define	NT_BROADCAST					0xFF		/* 同報セレクティングのデータ */
/* 下位転送用端末No.(1) */
/* 下位転送用端末No.(2) */
/* 通信リンクNo. */
/* 端末ステータス */
#define	NT_BUFFER_NORMAL				0x00		/* 正常 */
#define	NT_NORMAL_BUFFER_FULL			0x01		/* bit0   :通常受信バッファFULLのため通常データ受信不可状態 */
#define	NT_PRIOR_BUFFER_FULL			0x02		/* bit1   :優先受信バッファFULLのため優先データ受信不可状態 */
#define	NT_ALL_BUFFER_FULL				0x03		/* bit1-2 :通常、優先受信バッファFULLのため、データ受信不可状態 */
/* 同報受信済みフラグ */
#define	NT_NOT_BROADCASTED				0x00		/* なし */
#define	NT_BROADCASTED					0x01		/* 前回の通信で同報セレクティングでデータ受信 */
#define	NT_BROADCASTED_CRC_ERR			0x81		/* 同報セレクティングデータがCRCエラー。(応答詳細は00Hで送信) */
/* ゼロカットする前のデータサイズ */
/* パケット優先モード */
#define	NT_NORMAL_DATA					0x00		/* 通常データ */
#define	NT_PRIOR_DATA					0x01		/* 優先データ */
/* 電文種別(STX/EOT/ENQ/ACK/NAC) */
#define	STX								0x02
#define	EOT								0x04
#define	ENQ								0x05
#define	ACK								0x06
#define	NAK								0x15
/* 応答詳細 */
#define	NT_RES_NORMAL					0x00		/* 正常 */
#define	NT_RES_CRC_RETRY_OVER			0x01		/* 受信STXブロックのCRC異常受信リトライオーバーのために強制ACK応答 */
#define	NT_RES_BLOCKNO_ERR1				0x02		/* STXのブロックNo.異常受信 (受信データ無効) */
#define	NT_RES_BLOCKNO_ERR2				0x03		/* STXのブロックNo.異常受信 (受信データ有効) */
#define	NT_RES_CRC_ERR					0x80		/* CRCエラー */
#define	NT_RES_BUFFER_FULL				0x81		/* 受信バッファFULL */
#define	NT_RES_SEQUENCE_ERR				0x82		/* シーケンス異常で強制切断 */


/* ブロックNo. */
/* 最終ブロック判定 */
#define	NT_NOT_LAST_BLOCK				0x00		/* 中間ブロック */
#define	NT_LAST_BLOCK					0x01		/* 最終ブロック */
/* システムID */
/* データ種別 */
/* データ保持フラグ */
#define	NT_DO_BUFFERING					0x00		/* バッファリングする */
#define	NT_NOT_BUFFERING				0x01		/* バッファリングしない */


/*----------------------------------------- SCI --*/

/* SCI通信エラー */
enum {
	NT_SCI_ERR_OVERRUN,						/* オーバーランエラー */
	NT_SCI_ERR_FRAME,						/* フレーミングエラー */
	NT_SCI_ERR_PARITY						/* パリティエラー */
};

#define	NT_SCI_DATABIT					0	/* データビット長	0=8bits, 1=7bits */
#define	NT_SCI_STOPBIT					0	/* ストップビット長	0=1bit , 1=2bits */
#define	NT_SCI_PARITY					0	/* パリティ種別		0=none , 1=odd  ,2=even */

/*----------------------------------------- COMDR --*/

/* COMDRステータス */
typedef enum {
	NT_COMDR_NO_PKT,						/* アイドル */
	NT_COMDR_PKT_RCVING,					/* パケット受信中 */
	NT_COMDR_PKT_RCVD,						/* パケット受信完了 */
	NT_COMDR_ERR_SCI,						/* 通信エラー(オーバーランエラー/フレーミングエラー/パリティエラー) */
	NT_COMDR_ERR_INVALID_LEN,				/* パケットデータ長不正 */
	NT_COMDR_ERR_TIMEOUT					/* キャラクタ間タイムアウト */
}eNT_COMDR_STS;

/* パケット送信結果 */
typedef enum {
	NT_COMDR_SND_NORMAL,					/* 送信完了 */
	NT_COMDR_SND_TIMEOUT,					/* 送信タイムアウト発生 */
	NT_COMDR_SND_ERR						/* 送信不可 */
}eNT_COMDR_SND;

#define	NT_C2C_TIMINT			1			/* キャラクタ間タイムアウト監視タイマのインターバル */
#define NT_PKT_SIG				"NTCOM"		/* パケット先頭のシグネチャ */
#define	NT_PKT_HEADER_SIZE		7			/* シグネチャ＋データ長 */
#define	NT_PKT_CRC_SIZE			2			/* CRCデータサイズ */

/*----------------------------------------- TIMER --*/

#define	NT_TIM_1MS_CH_MAX		1			/* 1msタイマ登録数MAX */
#define	NT_TIM_10MS_CH_MAX		5			/* 10msタイマ登録数MAX */
#define	NT_TIM_100MS_CH_MAX		5			/* 100msタイマ登録数MAX */

#define	NT_TIM_UNIT_1MS			1
#define	NT_TIM_UNIT_10MS		10
#define	NT_TIM_UNIT_100MS		100

#define	NT_TIM_INVALID			0			/* タイマ未登録 */
#define	NT_TIM_STOP				1			/* タイマ停止中 */
#define	NT_TIM_START			2			/* タイマ動作中 */

#define	NT_TIM_1SHOT			1
#define	NT_TIM_CYCLE			2

/*----------------------------------------- MAIN&ARC --*/

#define	NT_ARC_SNDQUE_COUNT		4	/* send queue count */

#define	NT_COM_ARCBUF_SIZE		(500)	/* ロングパケット対処 */

#define	NT_MODULE_CODE			63		/* NTCom端末ﾓｼﾞｭｰﾙｺｰﾄﾞ 63=0x3F	*/

/* 7セグLED状態 */
typedef enum {
	NT_LED_IDLE = 1,
	NT_LED_SELPOLL
}eNT_LED;

/* IBWから受信した送信電文の識別結果 #003*/
enum {
	NT_ARC_NORMAL_PACKET,
	NT_ARC_FREE_PACKET,
	NT_ARC_BLOCK_CHECK_PACKET,
	NT_ATC_UNKOWN_PACKET
};

/* エラーコード(ARCNETの「エラーデータ」電文で送信) */
#define	NT_ERR_NO_RESPONSE			0x01	/* 通信不良 */
#define	NT_ERR_CRC_DATA				0x05	/* CRC16チェックNG(データ) */
#define	NT_ERR_CRC_CODE				0x06	/* CRC16チェックNG(コード) */
#define	NT_ERR_ACK01				0x07	/* CRC異常でデータ破棄 */
#define	NT_ERR_ACK02				0x08	/* ACK02リトライオーバーでデータ破棄 */
#define	NT_ERR_BROADCAST_FAIL		0x0A	/* 同報応答なし */
#define	NT_ERR_BLK_ARC2				0x13	/* ブロック番号異常(今回有効)(ARCNET) （NT－NET側のACK03　に相当）*/
#define	NT_ERR_BLK_ARC1				0x14	/* ブロック番号異常(今回無効)(ARCNET) （NT－NET側のACK02　に相当）*/
#define	NT_ERR_BLK_NT				0x15	/* ブロック番号異常(NT-NET) */
#define	NT_ERR_RETRY_OVER			0x1E	/* リトライ回数オーバー */
#define	NT_ERR_ARCBLK_RETRY_OVER	0x30	/* 途中ブロック異常 */
#define	NT_ERR_BLK_OVER_ARC			0x32	/* 受信ブロックオーバー(ARCNET) */
#define	NT_ERR_BLK_OVER_NT			0x33	/* 受信ブロックオーバー(NT-NET) */

/*----------------------------------------- MASTER --*/

/* ポーリング／セレクティング */
//typedef enum {
//	NT_COM_MODE_IDLE,					/* アイドル中 */
//	NT_COM_MODE_POLLING,				/* ポーリング中 */
//	NT_COM_MODE_SELECTING				/* セレクティング中 */
//}eNT_COM_POLLSEL;

/* 主局ステータス */
//typedef enum {
//	NT_COM_MASTER_IDOL,				/* アイドル  */
//	NT_COM_MASTER_ENQ_SEND,			/* ENQ送信後 */
//	NT_COM_MASTER_STX_SEND			/* STX送信後 */
//}eNT_COM_STS_MASTER;

/*----------------------------------------- SLAVE --*/

/* 従局ステータス */
typedef enum {
	NT_COM_SLAVE_IDOL,				/* アイドル (ENQ待ち)*/
	NT_COM_SLAVE_STX_SEND,			/* STX送信後 */
	NT_COM_SLAVE_ACK_SEND,			/* ACK送信後 */
	NT_COM_SLAVE_BROAD_CAST_WAIT	/* 同報受信待ち */
}eNT_COM_STS_SLAVE;

/* 従局ステータス */
enum {
	SEND_DISABLE = 0,		// データなし、主局バッファフル等で送信不可
	SEND_ENABLE_PRIOR,		// 優先データ送信可能
	SEND_ENABLE_NORMAL		// 通常データ送信可能
};






/*--------------------------------------------------------------------------*/
/*			T Y P E S														*/
/*--------------------------------------------------------------------------*/

#ifndef BOOL
#define BOOL	uchar
#define TRUE	1
#define	FALSE	0
#endif

/* タイマユーザー用 */
typedef struct {
	ushort	timerid;								/* タイマーID */
	ushort	timeout;								/* タイムアウト */
	long	count;									/* [<0] = 未動作, [0] = タイムアウト, [>0] = タイムアウト監視中 */
}T_NT_TIMER;

/*----------------------------------------- MAIN --*/

/* 初期設定データ(ARCパケット形式) */
typedef struct {
	uchar			ExecMode;						/* 主局／従局 */
	uchar			TerminalNum;					/* 子機接続台数(主局時)／自端末No.(従局時) */
	uchar			Baudrate;						/* 通信BPS */
	uchar			Time_t1[2];						/* ENQ送信後の応答待ち時間(t1) */
	uchar			Time_t2[2];						/* 受信データ最大時間(t2) */
	uchar			Time_t3[2];						/* テキスト送信後の応答待ち時間(t3) */
	uchar			Time_t4[2];						/* 次局とのインターバル時間(t4) */
	uchar			Time_t5[2];						/* 文字間タイマー(t5) */
	uchar			Time_t6[2];						/* 同報ENQ後のデータ送信Wait時間(t6) */
	uchar			Time_t7[2];						/* 受信データ最大時間(t7) */
	uchar			Time_t8[2];						/* 同報処理前のWAIT時間(t8) */
	uchar			Time_t9[2];						/* 送信ウェイトタイマー(t9) */
	uchar			Retry;							/* データリトライ受信回数 */
	uchar			BlkSndUnit;						/* ブロック送信単位 */
	uchar			Time_LineMonitoring;			/* 通信回線監視タイマー */
	uchar			NoResNum[2];					/* 無応答エラー判定回数 */
	uchar			SkipCycle[2];					/* 無応答時スキップ周期 */
	uchar			Time_ResFromMain;				/* MAINモジュールからの受信応答待ち時間 */
}T_NT_INITIAL_DATA_FRAME;

/* 初期設定データ(IBKでの保存形式) */
typedef struct {
	uchar			ExecMode;						/* 主局／従局 */
	uchar			TerminalNum;					/* 子機接続台数(主局時)／自端末No.(従局時) */
	ushort			Baudrate;						/* 通信BPS */
	ushort			Time_t1;						/* ENQ送信後の応答待ち時間(t1) */
	ushort			Time_t2;						/* 受信データ最大時間(t2) */
	ushort			Time_t3;						/* テキスト送信後の応答待ち時間(t3) */
	ushort			Time_t4;						/* 次局とのインターバル時間(t4) */
	ushort			Time_t5;						/* 文字間タイマー(t5) */
	ushort			Time_t6;						/* 同報ENQ後のデータ送信Wait時間(t6) */
	ushort			Time_t7;						/* 受信データ最大時間(t7) */
	ushort			Time_t8;						/* 同報処理前のWAIT時間(t8) */
	ushort			Time_t9;						/* 送信ウェイトタイマー(t9) */
	uchar			Retry;							/* データリトライ受信回数 */
	uchar			BlkSndUnit;						/* ブロック送信単位 */
	uchar			Time_LineMonitoring;			/* 通信回線監視タイマー */
	ushort			NoResNum;						/* 無応答エラー判定回数 */
	ushort			SkipCycle;						/* 無応答時スキップ周期 */
	uchar			Time_ResFromMain;				/* MAINモジュールからの受信応答待ち時間 */
}T_NT_INITIAL_DATA;

//typedef struct {
//	uchar	Count;
//	uchar	ReadIdx;
//	uchar	WriteIdx;
//	uchar	dummy;
//} t_NTCom_RcvQueCtrl;

/*----------------------------------------- DATA --*/
/* データ管理機能 制御データ */
typedef struct {
	BOOL	terminal_exists;						/* 接続端末数 */
	eNT_STS	terminal_status;						/* 通信障害中の端末No.(なければ-1) */
}T_NTComData_Ctrl;

/* バッファ情報(TRUE=電文有り) */
typedef struct {
	BOOL			normal_s;						/* 通常データバッファ(NTネットから受信用) */
	BOOL			normal_r;						/* 通常データバッファ(NTネットへ送信用) */
	BOOL			prior_s;						/* 優先データバッファ(NTネットへ送信用) */
	BOOL			prior_r;						/* 優先データバッファ(NTネットから受信用) */
	BOOL			broadcast_s;					/* 同報データバッファ(NTネットへ送信用) */
	BOOL			broadcast_r;					/* 同報データバッファ(NTネットから受信用) */
}T_NT_BUF_STATUS;

/* 端末接続状態 */
//typedef struct {
//	uchar			sts[NT_TERMINALNUM_MAX];
//}T_NT_TERMINAL_STATUS;

/* バッファに含まれるブロック数 */
typedef struct {
	uchar			normal;
	uchar			prior;
}T_NT_BUF_COUNT;

/* バッファ */
typedef struct {
	short			len;							/* データ部サイズ */
	uchar			data[NT_BLKSIZE];				/* バッファ本体 */
}T_NT_BLKDATA;

/* データバッファ */
typedef struct {
	eNT_DATA_KIND	kind;							/* データ種別 */
	BOOL			fixed;							/* TRUE=電文確定 */
	int				blknum;							/* 有効ブロック数 */
	int				max;							/* リストの最大要素数 */
	T_NT_BLKDATA	*blkdata;						/* バッファ本体 */
}T_NT_BUFFER;

/* 電文情報(データ管理クラスのユーザでは値を変えないこと) */
typedef struct {
	eNT_DATA_KIND	kind;							/* データ種別 */
	int				terminal;						/* 端末番号 */
	int				blknum;							/* ブロック数 */
	int				curblk;							/* 現在ブロック位置 */
/* #006-> */
	uchar			saveflag;						/* データ保持フラグ */
/* <-#006 */
}T_NT_TELEGRAM;

/*  */
typedef struct {
//	#002	→
//	int				top;							/* リングバッファ先頭 */
//	int				telenum;						/* 電文数 */
	int				read;							/* リングバッファ先頭 */
	int				write;							/* リングバッファ終端 */
//	←	#002
	int				ring[1 + 1];					/* 電文所在情報(端末No.) */
}T_NT_TELEGRAM_LIST;


/* バッファメモリの実体 */
typedef struct {
	T_NT_BLKDATA	snd_pool_normal[NT_NORMAL_BLKNUM_MAX];			/* 通常データ送信用 */
	T_NT_BLKDATA	rcv_pool_normal[NT_NORMAL_BLKNUM_MAX];			/* 通常データ受信用 */
	T_NT_BLKDATA	snd_pool_prior[NT_PRIOR_BLKNUM_MAX];			/* 優先データ送信用 */
	T_NT_BLKDATA	rcv_pool_prior[NT_PRIOR_BLKNUM_MAX];			/* 優先データ受信用 */
	T_NT_BLKDATA	snd_pool_broadcast[NT_BROADCAST_BLKNUM_MAX];	/* 同報データ送信用 */
	T_NT_BLKDATA	rcv_pool_broadcast[NT_BROADCAST_BLKNUM_MAX];	/* 同報データ受信用 */
}T_NT_BUFMEM_POOL;

/* エラー情報 */
typedef struct {
	uchar			module;							/* モジュールコード */
	uchar			errcode;						/* エラーコード */
	uchar			occur;							/* 発生(1)/解除(0) */
	uchar			terminal;						/* 端末No. */
}T_NT_ERR;

/* エラー情報バッファ */
typedef struct {
	int				top;							/* リングバッファ先頭 */ 
	int				num;							/* 有効要素数 */
	T_NT_ERR		err[NT_ARC_ERRQUE_COUNT];		/* バッファ本体 */
}T_NT_ERR_INFO;

/* #005-> */
/* 停電保証データ */
typedef enum {
	NT_FUKUDEN_SCHEDULER_NONE,						/* 復電処理無し */
	NT_FUKUDEN_SCHEDULER_START,						/* スケジューラのコピー作成中 */
	NT_FUKUDEN_SCHEDULER_MADE						/* スケジューラのコピー書き戻し中 */
}eNT_FUKUDEN_SCHEDULER;
typedef struct {
	eNT_FUKUDEN_SCHEDULER	scheduler;				/* 通常受信データのARCNETへの送信スケジューラの再構成中 */
	int						scheduler_work;			/* スケジューラの再構成用ワーク */
	int						scheduler_worksize;		/* scheduler_ring内の有効データ数 */
}T_NT_FUKUDEN;
/* <-#005 */

/*----------------------------------------- TIMER --*/

/* タイマ制御 */
typedef struct {
	uchar	sts;			/* タイマステータス(NTCOM_TIM_CYCLE/NTCOM_TIM_1SHOT/NTCOM_TIM_INVALID/NTCOM_TIM_STOP) */
	void	(*func)(void);	/* タイムアウト時にコールされる関数 */
	ushort	time;			/* タイマ初期値 */
	ushort	cnt;			/* タイマ現在値 */
	BOOL	cyclic;			/* TRUE=サイクリックタイマ, FALSE=1SHOTタイマ */
}T_NT_TIM_CTRL;

/* コールバック関数 */
typedef void (*T_NT_SCI_CALLBACK)(uchar);

/*----------------------------------------- SCI --*/

/* SCI制御 */
typedef struct {
	/* ---- port information ---- */
	uchar				port_no;			/* ポート番号 */
/* ART:machida (s) 05-12-20 */
//	struct st_sci		*pport;				/* ポート制御レジスタ先頭アドレス */
	volatile struct st_sci	*pport;			/* ポート制御レジスタ先頭アドレス */
/* ART:machida (e) 05-12-20 */
	uchar				speed;				/* ボーレート */
	uchar				databit;			/* データビット数 */
	uchar				stopbit;			/* ストップビット数 */
	uchar				parity;				/* パリティ */
	/* ---- for sending ---- */
	uchar				*sdata;				/* 送信データ */
	ushort				slen;				/* 送信データ長 */
	ushort				scnt;				/* 送信済みデータ数 */
	BOOL				scomplete;			/* 送信完了フラグ (1=complete, 0=yet) */
	/* ---- callback functions ---- */
	T_NT_SCI_CALLBACK	rcvfunc;			/* データ受信通知用コールバック関数ポインタ */
	T_NT_SCI_CALLBACK	stsfunc;			/* データ受信エラー通知用コールバック関数ポインタ */
}T_NT_SCI_CTRL;

/*----------------------------------------- COMDR --*/

/* COMDR制御 */
typedef struct {
	uchar		status;				/* COMDR機能ステータス */
	int			r_rcvlen;			/* 受信電文長 */
	int			r_datalen;			/* 有効電文長 */
	uchar		r_buf[NT_BLKSIZE];	/* 受信バッファ */
	T_NT_TIMER	timer_c2c;			/* キャラクタ間タイムアウト */
	T_NT_TIMER	timer_sndcmp;		/* 送信完了タイムアウト */
}T_NT_COMDR_CTRL;

/*----------------------------------------- MAIN&ARC --*/

typedef struct {					/* queue control data */
	uchar	Count;
	uchar	ReadIdx;
	uchar	WriteIdx;
	uchar	dummy;
} t_NT_RcvQueCtrl;

typedef struct {
	ushort	Count; 		         	/* regist count */
	ushort	R_Index;	  	        /* next read offset index */
	ushort	W_Index;    	  	    /* next write offset index */
} t_QueCtrl;

typedef struct {
	uchar	BlkSts;					/* ARC block number */
	uchar	BlkNo;					/* ARC block status */
	ushort	Length;					/* data length 		*/
	uchar	Data[500];				/* data */
} t_SndQue;

/* error inform data for IBW queue data style */
typedef	struct {
	uchar	ModuleCode;				/* module code */
	uchar	ErrCode;				/* error code */
	uchar	f_Occur;				/* 1=occured , 0=release */
	uchar	TerminalNo;				/* number of terminal, error occurred on */
} t_ErrCode;

typedef struct {					/* send data queue 	*/
	ushort	Length;					/* data length 		*/
	uchar	Data[500];				/* data(ロングパケット対応: 256 -> 500) */
} t_Que;


typedef t_Que T_NT_ARCBLK;

/* ARCでのNT-NETブロックデータの受信 */
//typedef struct {
//	BOOL			waitblk;		/* 受信待ちブロックNo. */
//	T_NT_BLKDATA	blkdata;		/* 受信したブロックデータ */
//	int				retry;			/* ブロックNo.異常連続発生回数 */
//}T_NT_ARC_RCVBLK;

/* #004 -> */
typedef struct {
	int				ntblkno_retry;	/* NT-NETブロックNo.異常による送信リトライ回数 */
	T_NT_TELEGRAM	telegram;
}T_NT_SND_TELEGRAM;
/* <- #004 */

typedef struct {					/* communication error data */
	uchar	Full;
	uchar	Comer;
	ushort	OVRcnt;
	ushort	FRMcnt;
	ushort	PRYcnt;
	uchar	dummy;
} t_NT_ComErr;

//typedef union {
typedef struct {
//	T_NT_BLKDATA	ntblk;
//	uchar			arcblk[NT_COM_ARCBUF_SIZE];
	ushort			len;
	uchar			sendData[26000];
}T_NTCom_SndWork;


/*----------------------------------------- SLAVE --*/

// 従局管理用構造体
typedef struct {
	eNT_COM_STS_SLAVE	status;			// 従局ステータス
	eNT_STS	master_status;				// 主局ステータス
	uchar	broad_cast_received;		// 同報データ受信済み
	uchar	block_send_count;			// ブロック送信数
	uchar	block_receive_count;		// ブロック受信数
	uchar	terminal_num;					// 主局時：接続されるターミナル数	従局時：自局番号
	BOOL	receive_packet_crc_status;		// 受信したパケットのCRCエラー状態
	uchar	receive_crc_err_count;			// CRCエラーの受信回数｡（stx）
	uchar	receive_crc_err_count_ctrl;		// 制御電文CRCエラーの受信回数｡（stx 以外）
	uchar	last_terminal_status;			// 一回前に送信した端末状態。

	uchar	send_retry_count;				// 送信リトライ回数
	uchar	ack02_receive_count;			// Ack02を受信した回数	（指定回数受信で電文削除）	#001
	uchar	cur_seq_no;						// シーケンスＮｏ．
	uchar	timeout_kind;					// タイムアウト種別

	T_NT_TIMER	timer_10m;				// 10m sec タイマー
	BOOL timeout_10m;					// 10m sec タイマー　タイムアウト
	T_NT_TIMER	timer_100m;				// 100m sec タイマー
	BOOL timeout_100m;					// 100m sec タイマー　タイムアウト
}T_NTCOM_SLAVE_CTRL;


/*----------------------------------------- BUF --*/
// NTComタスク通常受信バッファ（対OPE層）
typedef struct {
	uchar	ntDataBuf[NT_NORMAL_BLKNUM_MAX][NT_BLKSIZE];	// NT-NETデータバッファ
	ushort	readIndex;										// 読込みインデックス
	ushort	writeIndex;										// 書込みインデックス
	ushort	count;											// 格納データ数
	BOOL	fixed;											// TRUE:送信中 FALSE:待機中
}T_NTCOM_NORMAL_DATABUF;

// NTComタスク優先受信バッファ（対OPE層）
typedef struct {
	uchar	ntDataBuf[NT_PRIOR_BLKNUM_MAX][NT_BLKSIZE];		// NT-NETデータバッファ
	ushort	readIndex;										// 読込みインデックス
	ushort	writeIndex;										// 書込みインデックス
	ushort	count;											// 格納データ数
	BOOL	fixed;											// TRUE:送信中 FALSE:待機中
}T_NTCOM_PRIOR_DATABUF;


#endif	/* ___NTCOMDEFH___ */
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304参考）
