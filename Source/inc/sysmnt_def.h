#ifndef	___SYSMNT_DEFH___
#define	___SYSMNT_DEFH___
/*[]----------------------------------------------------------------------[]
 *|	filename: sysmnt_def.h
 *[]----------------------------------------------------------------------[]
 *| summary	: システムメンテナンス共通定義
 *| author	: machida.k
 *| date	: 2005.07.01
 *| update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

#include "common.h"

/*=====================================================================================< sysmntsci.c >====*/

/*--------------------------------------------------------------------------*/
/*			D E F I N I T I O N S											*/
/*--------------------------------------------------------------------------*/

#define	STX						0x02
#define	ETX						0x03
#define	EOT						0x04
#define	ENQ						0x05
#define	ACK						0x06
#define	NAK						0x15

#define	SEL						0xFE
#define	POL						0xFF

/* 文字間タイムアウト */
#define	SYSMNT_SCI_C2C_TIMEOUT	2

#define	SYSMNT_TIMER_STOP		0x8000

/* SCIパケットヘッダ部サイズ */
#define	SYSMNT_SCI_HEADER_SIZE	9	/* stx+id1+id2+id3+id4 */

/* SCIパケットデータ部サイズ */
#define	SYSMNT_DATALEN_MAX		2048

/* 最大パケット長 */
#define	SYSMNT_SCI_PKTLEN_MAX	(SYSMNT_SCI_HEADER_SIZE+SYSMNT_DATALEN_MAX+1+2)	/* "+1+2" = etx+bcc */

/* バイナリ変換後の最大データ長 */

/* REMARK : ----------------------------
     AppServ_ConvParam2CSV_Init()は下記SYSMNT_DATALEN_MAXが1112以上で
     あることを前提としているため、"SYSMNT_DATALEN_MAX" がこれ以下とならぬよ
     う注意すること
 --------------------------------------- */

/* SCIエラー */
enum {
	SYSMNT_SCI_ERR_OVERRUN,
	SYSMNT_SCI_ERR_FRAME,
	SYSMNT_SCI_ERR_PARITY
};

typedef void (*SYSMNT_SCI_FUNC)(uchar);

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/

void	SYSMNTsci_Init(SYSMNT_SCI_FUNC RcvFunc, SYSMNT_SCI_FUNC StsFunc);
void	SYSMNTsci_Start(void);
BOOL	SYSMNTsci_SndReq(uchar *buf, int len);
BOOL	SYSMNTsci_IsSndCmp(void);
void	SYSMNTsci_Stop(void);
void	SYSMNTsci_PC_Start(void);

void	SYSMNT_Int_ERI( void );
void	SYSMNT_Int_RXI( void );
void	SYSMNT_Int_TXI( void );
/*	NOTE:
	SYSMNT_Int_xxxは１次のハンドラではないのでポート番号を関数名に持つ必要がない。
	既存ソースとの互換のため、以下のように再定義をしておく。
*/
#define	SYSMNT_Int_ERI2		SYSMNT_Int_ERI
#define	SYSMNT_Int_RXI2		SYSMNT_Int_RXI
#define	SYSMNT_Int_TXI2		SYSMNT_Int_TXI

/*=====================================================================================< sysmntcomdr.c >====*/

/*--------------------------------------------------------------------------*/
/*			D E F I N I T I O N S											*/
/*--------------------------------------------------------------------------*/

/* 送受信タイムアウト */
#define	_SYSMNT_COMM_TIMEOUT		5000	/* [msec] */

/* プロトコル定義 */
#define	_SYSMNT_NOT_LAST_BLK		0x00
#define	_SYSMNT_LAST_BLK			0x01
#define	_SYSMNT_UPLOAD				0xA1
#define	_SYSMNT_DOWNLOAD_REQ		0xA2
#define	_SYSMNT_DOWNLOAD			0xB1
#define	_SYSMNT_FILE_EXIST			0x00
#define	_SYSMNT_NO_FILE				0x81
#define	_SYSMNT_PC_ERR				0x8F
#define	_SYSMNT_DATA_VISIBLE		0x10
#define	_SYSMNT_DATA_INVISIBLE		0x00

/* COMDRステータス */
typedef enum {
	SYSMNT_COMDR_NO_PKT,				/* アイドル */
	SYSMNT_COMDR_PKT_ERR,				/* パケットフォーマット異常検出 */
	SYSMNT_COMDR_ERR_INVALID_LEN,		/* データ長異常検出 */
	SYSMNT_COMDR_ERR_SCI,				/* SCIエラー */
	SYSMNT_COMDR_BCC_ERR,				/* BCCエラー */
	SYSMNT_COMDR_PKT_RCVD				/* パケット受信完了 */
}eSYSMNT_COMDR_STS;

/*--------------------------------------------------------------------------*/
/*			S T R U C T U R E S												*/
/*--------------------------------------------------------------------------*/

/* 送信パケットタイプ */
enum {
	SYSMNT_TYPE_SEL,
	SYSMNT_TYPE_POL,
	SYSMNT_TYPE_STX
};

/* パケット(生データ) */
typedef struct {
	int		len;
	uchar	data[SYSMNT_SCI_PKTLEN_MAX];
}T_SYSMNT_SCI_PACKET;

/* パケットヘッダ(BINARY変換後) */
typedef struct {
	uchar	blkno;						/* ID1:データブロックNo. */
	uchar	blksts;						/* ID2:ブロックステータス */
	uchar	data_kind;					/* ID3:データ種別 */
	uchar	data_format;				/* ID4:データフォーマット */
}T_SYSMNT_PKT_HEADER;

/* パケットデータ(T_SYSMNT_PKT_HEADER::data_formatに従った変換後) */
typedef struct {
	int		len;						/* データ長 */
	uchar	data[SYSMNT_DATALEN_MAX];	/* データ */
}T_SYSMNT_PKT_DATA;

/* パケットデータ(アプリケーション用) */
typedef struct {
	uchar					type;		/* 電文先頭の制御コード */
	T_SYSMNT_PKT_HEADER		header;		/* ヘッダ部 */
	T_SYSMNT_PKT_DATA		data;		/* データ部 */
}T_SYSMNT_PKT;

// 料金計算テスト 使用カード情報データ構造
typedef struct {
	char			kind;				// カード種別
	short			info;				// カード情報
} t_Card_Info;

// 料金計算テスト 共通パラメータ設定起動制御データ構造
typedef struct {
	char			set;				// 共通パラメータ設定要求(OFF:設定要求なし/ON:設定要求あり)
	long			no;					// 共通パラメータアドレス
} t_param_Info;

// 料金計算テスト 制御データ構造
typedef struct {
	char			mode;				// 親画面モード
	char			sub_mode;			// 子画面モード
	short			pos;				// カーソル位置
	char			nrml_strt;			// 通常起動フラグ(OFF:共通パラメータからの起動/ON:通常起動)
	t_Card_Info		last_card;			// 最終使用カード情報
	t_param_Info	param;				// 共通パラメータ制御データ
} t_RT_Ctrl;

/*--------------------------------------------------------------------------*/
/*			E X T E R N A L   V A L I A B L E S								*/
/*--------------------------------------------------------------------------*/

extern ushort	SYSMNTcomdr_c2c_timer;
extern ushort	SYSMNTcomdr_sndcmp_timer;
extern	t_RT_Ctrl	rt_ctrl;			// 料金計算テスト制御データ

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/

void	SYSMNTcomdr_Open(void);
void	SYSMNTcomdr_Reset(void);
eSYSMNT_COMDR_STS	SYSMNTcomdr_GetRcvPacket(T_SYSMNT_PKT *buf);
BOOL	SYSMNTcomdr_SendPacket(T_SYSMNT_PKT *buf, int type);
BOOL	SYSMNTcomdr_SendByte(uchar c);
BOOL	SYSMNTcomdr_IsSendCmp(BOOL *timeout);
void	SYSMNTcomdr_Close(void);

/*=====================================================================================< systemmnt.c >====*/

/*--------------------------------------------------------------------------*/
/*			D E F I N I T I O N S											*/
/*--------------------------------------------------------------------------*/

/* Mifare発行画面 ページ数MAX */
#define	ISSMIF_PAGE_MAX	3

/* Mifare発行画面 各ページ毎の表示項目 */
enum {	/* page 0 */
	ISSMIF_CARD_TYPE,				/* カードタイプ */
	ISSMIF_PARKING_NO,				/* 駐車場No. */
	ISSMIF_TEIKI_NO,				/* 定期No. */
	ISSMIF_TEIKI_KIND,				/* 定期種別 */
	ISSMIF_INOUT_STS,				/* 入出庫ステータス */
	ISSMIF_CHARGE_KIND,				/* 料金種別 */
	ISSMIF_CURSORMAX_PAGE0
};
enum {	/* page 1 */
	ISSMIF_UPDATE_DATETIME,			/* 更新年月日時分 */
	ISSMIF_IN_DATETIME,				/* 入庫年月日時分 */
	ISSMIF_PAY_DATETIME,			/* 精算年月日時分 */
	ISSMIF_OUT_DATETIME,			/* 出庫年月日時分 */
	ISSMIF_START_DATE,				/* 有効開始年月日 */
	ISSMIF_END_DATE,				/* 有効終了年月日 */
	ISSMIF_CURSORMAX_PAGE1
};
enum {	/* page 2 */
	ISSMIF_AREA_INFO,				/* 区画情報 */
	ISSMIF_AREA_NO,					/* 区画No. */
	ISSMIF_PARTITION_NO,			/* 車室番号 */
	ISSMIF_RACK_TYPE,				/* ラックタイプ */
	ISSMIF_RACK_POS,				/* ラック位置 */
	ISSMIF_CURSORMAX_PAGE2
};

/* Mifare発行画面 t_IssMIF_Ctrl::mifare_status */
enum {
	ISSMIF_CARD_REQ,			/* "カードをかざしてください" */
	ISSMIF_WRITING,				/* "＝書込中です＝" */
	ISSMIF_OK,					/* "＝書込ＯＫです＝" */
	ISSMIF_WRITE_NG,			/* "＝書込ＮＧです＝" */
	ISSMIF_AUTH_NG,				/* "＝認証ＮＧです＝" */
	ISSMIF_EDIT,					/* データ編集中 */
	ISSMIF_USRMNT				/* ユーザメンテナンス券データ確認画面 */
};

/*--------------------------------------------------------------------------*/
/*			S T R U C T U R E S												*/
/*--------------------------------------------------------------------------*/

/* Mifare発行画面 データ保存用ワーク領域 */
typedef struct {
	ulong			data[ISSMIF_PAGE_MAX][ISSMIF_CURSORMAX_PAGE0];
	/* 以下はulongでオーバフローする数値データ */
	uchar			card_no[10];						/* カード番号 */
	uchar			in_datetime[10];					/* 入庫年月日時分 */
	uchar			pay_datetime[10];					/* 精算年月日時分 */
	uchar			out_datetime[10];					/* 出庫年月日時分 */
	uchar			ope_datetime[10];					/* 処理年月日時分 */
	uchar			update_datetime[10];				/* 更新年月日時分 */
}t_IssMIF_Data;

/* Mifare発行画面 制御データ */
typedef struct {
	int				operation;			/* 対Mifare処理状態 */
	int				page;				/* 表示ページ */
	int				cursor;				/* 現在カーソル位置 */
	ulong			cursor_val1;		/* カーソル位置の表示値 */
	uchar			cursor_val2[10];	/* ulongでオーバフロー(10桁)する数値データ */
	uchar			input;				/* カーソル移動後TENキー未入力＝0 */
	t_IssMIF_Data	*mifare;			/* Mifareデータバッファ */
}t_IssMIF_Ctrl;


/*--------------------------------------------------------------------------*/
/*			E X T E R N A L   V A L I A B L E S								*/
/*--------------------------------------------------------------------------*/

extern	ushort	SysMnt_RcvTimer;

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/
typedef void (*SYSMNT_MIFDSP_FUNC)(t_IssMIF_Ctrl *);

void	issmif_InitCtrl(t_IssMIF_Ctrl *ctrl, int ope);
void	issmif_GetAsciiDateTime(uchar *datetime, ushort year, uchar month, uchar day, uchar hour, uchar min);
void	issmif_DisplayPage0(t_IssMIF_Ctrl *ctrl);
void	issmif_DisplayPage1(t_IssMIF_Ctrl *ctrl);
void	issmif_DisplayPage2(t_IssMIF_Ctrl *ctrl);
void	issmif_DisplayPage3(t_IssMIF_Ctrl *ctrl);
void	issmif_DisplayDiscountPage(t_IssMIF_Ctrl *ctrl);
#define	_issmif_DisplayPage4	issmif_DisplayDiscountPage
#define	_issmif_DisplayPage5	issmif_DisplayDiscountPage
#define	_issmif_DisplayPage6	issmif_DisplayDiscountPage

ushort	SysMntLogoDataReg(void);					// logo登録
void	SysMntLogoDataRegDsp(const uchar *title);		// 表示

#endif	/* ___SYSMNT_DEFH___ */

