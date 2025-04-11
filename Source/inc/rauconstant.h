#ifndef	___RAUCONSTANTH___
#define	___RAUCONSTANTH___
/************************************************************************************/
/*																					*/
/*		システム名　:　RAUシステム													*/
/*		ファイル名	:  rauconstant.h												*/
/*		機能		:  定数、マクロ定義ファイル										*/
/*																					*/
/************************************************************************************/

#include	"system.h"

#define		RAU_SET			1				/*	SET	*/
#define		RAU_CLR			0				/*	CLR	*/

/****	端末側状態遷移	****/
#define		S_T_IDLE				0		/* アイドル状態			*/
#define		S_T_SEND				1		/* 送信完了待ち状態		*/
#define		S_T_ACK					2		/* ＡＣＫ待ち状態		*/
#define		S_T_TEXT				3		/* テキスト受信中状態	*/
#define		S_T_BCC					4		/* ＢＣＣ受信待ち状態	*/

/****	ＨＯＳＴ側状態遷移	****/
#define		S_H_IDLE				0		/* アイドル状態			*/
#define		S_H_SEND				1		/* 送信完了待ち状態		*/
#define		S_H_ACK					2		/* ＡＣＫ待ち状態		*/
#define		S_H_SOH					3		/* ＳＯＨ受信待ち状態	*/
#define		S_H_TEXT				4		/* ＳＴＸ受信待ち状態	*/
#define		S_H_ETX					5		/* ＥＴＢ／ＥＴＸ受信待ち状態	*/
#define		S_H_CRC_1				6		/* ＣＲＣ上位バイト受信待ち状態 */
#define		S_H_CRC_2				7		/* ＣＲＣ下位バイト受信待ち状態 */

#define		S_H_BEFOR				10		/* モデム接続前状態 */
#define		S_H_BEFOR_ID			11		/* モデム接続前状態 */
#define		S_H_BEFOR_ID_OK			12		/* モデム接続前状態 */
#define		S_H_BEFOR_ID_NG			13		/* モデム接続前状態 */
#define		S_H_BEFOR_PW			14		/* モデム接続前状態 */
#define		S_H_BEFOR_PW_OK			15		/* モデム接続前状態 */
#define		S_H_BEFOR_PW_NG			16		/* モデム接続前状態 */
#define		S_H_BEFOR_TEL_SEND_WAIT	17		/* モデム接続前状態 */
#define		S_H_BEFOR_TEL_OK_WAIT	18		/* モデム接続前状態 */


/*****	各ワークＭＡＸ値	*****/
#define		RAU_DATA_MAX		1448			// 送受信最大サイズ
#define		RAU_RCV_MAX_T		500				/* 端末受信エリアＭＡＸサイズ		*/
#define		RAU_RCV_MAX_H		RAU_DATA_MAX	/* ＨＯＳＴ受信エリアＭＡＸサイズ	*/
#define		RAU_SEND_MAX_T		500				/* 端末送信エリアＭＡＸサイズ		*/
#define		RAU_SEND_MAX_H		RAU_DATA_MAX	/* ＨＯＳＴ送信エリアＭＡＸサイズ	*/
#define		RAU_NET_RES_LENGTH_MAX	29


/***  Hostからのﾎﾟｰﾘﾝｸﾞ受信監視ｱﾗｰﾑｴﾘｱ(最新の2件持ちます)  ***/
#define	HOST_NONE_POL_QUE_SIZE		2				/* 絶対に2の乗数にしてください */
#define	HOST_NONE_POL_HAPPEN		1
#define	HOST_NONE_POL_RESTORE		2
#define	HOST_NONE_POL_EVENT_NONE	0
#define	HOST_NONE_POL_ALM_TH		3000			/* ｱﾗｰﾑと判断するｽﾚｯｼｮﾙﾄﾞ(約 x100ms) */

/***  Termからのﾎﾟｰﾘﾝｸﾞ受信監視ｱﾗｰﾑｴﾘｱ	***/
#define	TERM_NONE_POL_ALM_TH		3000			/* ｱﾗｰﾑと判断するｽﾚｯｼｮﾙﾄﾞ(約 x100ms) */
#define	HOST_SCU_CHECK_TIMING		600				/* ｱﾗｰﾑをﾁｪｯｸする周期(約 x100ms) = 1分 */
#define	TEST_RCQ_SIZE	16
#define	TEST_HOST_NONE_POL_COUNTER_60S	((ushort)36000)
#define	TEST_ERROR_COUNT_LIMIT	((ushort)999)

/** 送信要求電文キュー **/
#define	RAU_IDB0_QUE_COUNT	16
#define	RAU_IDB0_QUE_SIZE	40

/*------------------------NT-NET対応------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*			T Y P E S														*/
/*--------------------------------------------------------------------------*/

#define	RAU_ARC_SNDQUE_COUNT		4	/* send queue count */
#define	RAU_ARC_ERRQUE_COUNT		20
#define	RAU_COM_ARCBUF_SIZE		(500)	/* ロングパケット対処 */

#define	RAU_BLKSIZE				1024		/* 1パケット長MAX */

#define	NT_NORMAL_BLKNUM_MAX	26			/* パケット保存数MAX */
#define	RAU_SEND_NTBLKNUM_MAX		32		// ※遠隔PCへの最大送信パケット数(32block) 2006.08.31:m-onouchi
#define	RAU_RECEIVE_NTBLKNUM_MAX	26		// ※遠隔PCからの最大受信パケット数(26block) 2006.08.31:m-onouchi
#define	NT_DOPANT_HEAD_SIZE		10			/* Dopa用 NT-NETのヘッダーサイズ */

#define	DOPA_HEAD_SIZE			26			// Ｄｏｐａパケットヘッダサイズ
#define	DOPA_DATA_SIZE			1420		// Ｄｏｐａパケットデータサイズ
#define	XPORT_DATA_SIZE			1420		// Etherパケットデータサイズ

//------------------------エラーコード----------------------------------------
#define	RAU_MODULE_CODE			0x41		// ＲＡＵモジュールコード

#define	RAU_ERR_MODEM_CONNECT_STATE		0x02	// モデム　接続・切断　ステータス
#define	RAU_ERR_DATA_CRC_ERROR			0x05	// エラーコード０５(バッファデータＣＲＣエラー)
#define	RAU_ERR_PASSWORD_BREAK			0x09	// エラーコード０９(パスワード破壊によりメモリ初期化)
#define	RAU_ERR_BLK_VALID				0x13	// エラーコード１９(ＮＴ－ＮＥＴブロック番号異常[今回有効])
#define	RAU_ERR_BLK_INVALID				0x14	// エラーコード２０(ＮＴ－ＮＥＴブロック番号異常[今回無効])
#define	RAU_ERR_ARCBLK_RETRY_OVER		0x30	// エラーコード４８(ＡＲＣＮＥＴ途中ブロック異常)
#define	RAU_ERR_BLK_OVER_ARC			0x32	// エラーコード５０(ＮＴ－ＮＥＴ受信ブロックオーバー)

#define	RAU_ERR_BUFF_FULL_20			0x3C	// エラーコード６０(入庫データバッファフル)
#define	RAU_ERR_BUFF_FULL_21			0x3D	// エラーコード６１(出庫データバッファフル)
#define	RAU_ERR_BUFF_FULL_22			0x3E	// エラーコード６２(事前精算データバッファフル)
#define	RAU_ERR_BUFF_FULL_23			0x3F	// エラーコード６３(出口精算データバッファフル)
#define	RAU_ERR_BUFF_FULL_30_41			0x40	// エラーコード６４(Ｔ合計集計データバッファフル)
#define	RAU_ERR_BUFF_FULL_120			0x41	// エラーコード６５(エラーデータバッファフル)
#define	RAU_ERR_BUFF_FULL_121			0x42	// エラーコード６６(アラームデータバッファフル)
#define	RAU_ERR_BUFF_FULL_122			0x43	// エラーコード６７(モニタデータバッファフル)
#define	RAU_ERR_BUFF_FULL_123			0x44	// エラーコード６８(操作モニタデータバッファフル)
#define	RAU_ERR_BUFF_FULL_131			0x45	// エラーコード６９(コイン金庫集計合計データバッファフル)
#define	RAU_ERR_BUFF_FULL_133			0x46	// エラーコード７０(紙幣金庫集計合計データバッファフル)
#define	RAU_ERR_BUFF_FULL_236			0x47	// エラーコード７１(駐車台数データバッファフル)
#define	RAU_ERR_BUFF_FULL_237			0x48	// エラーコード７２(区画台数・満車データバッファフル)
#define	RAU_ERR_BUFF_FULL_126			0x49	// エラーコード７３(金銭管理データバッファフル)

//------------------------ARCブロックチェック結果------------------------------

#define	RAU_ARCBLK_NORMAL				0x00		// 正常 
#define	RAU_ARCBLK_SEND_END				0x01		// ARCブロックナンバー異常 再送なし 
#define	RAU_ARCBLK_SEND_RETRY			0x81		// ARCブロックナンバー異常 再送要求 
#define	RAU_ARCBLK_INVALID				0x85		// NTブロックナンバー異常 今回無効 
#define	RAU_ARCBLK_VALID				0x86		// NTブロックナンバー異常 今回有効 
#define	RAU_ARCBLK_HOST_BUSY			0x90		// ホストからのコネクション中によりデータ受信不可 

#define	RAU_DATA_ID			0x		// 

// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//#define		RAU_ID_LOGIDLIST_COUNT		17			// ログIDリストデータ数
#define		RAU_ID_LOGIDLIST_COUNT		18			// ログIDリストデータ数
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)

// IBWから受信した送信電文の識別結果
enum {
	RAU_ARC_NORMAL_PACKET,
	RAU_ARC_FREE_PACKET,
	RAU_ARC_BLOCK_CHECK_PACKET,
	RAU_ATC_UNKOWN_PACKET
};

enum {	// 送信シーケンスフラグ
	RAU_SND_SEQ_FLG_NORMAL,			// ０：ＨＯＳＴへのテーブル送信待機状態
	RAU_SND_SEQ_FLG_TABLE_SEND		// １：ＨＯＳＴへのテーブル送信開始状態(ＩＢＷからのテーブルデータ受信禁止)
};
enum {	// 受信シーケンスフラグ
	RAU_RCV_SEQ_FLG_NORMAL,			// ０：通常状態(ＩＢＷからの要求データ受信許可)
	RAU_RCV_SEQ_FLG_WAIT,			// １：ＨＯＳＴからの要求データ受信完了待ち状態(ＩＢＷからのスルーデータ受信禁止)
	RAU_RCV_SEQ_FLG_THROUGH_RCV,	// ２：ＨＯＳＴからの要求データ受信完了状態
	RAU_RCV_SEQ_FLG_THROUGH_SEND	// ３：ＨＯＳＴへのスルーデータ送信開始状態(ＩＢＷからのスルーデータ受信禁止)
};

// テーブルデータ設定時の戻り値 
typedef enum {
	RAU_DATA_NORMAL,							// 成功 
	RAU_DATA_BLOCK_INVALID1,					// ブロックナンバー異常によりデータ破棄（今回無効） 
	RAU_DATA_BLOCK_INVALID2,					// ブロックナンバー異常によりデータ破棄（今回有効） 
	RAU_DATA_CONNECTING_FROM_HOST,				// ホストからの接続中のため受け付け不可（データ送信要求） 
	RAU_DATA_BUFFERFULL							// バッファが一杯
}eRAU_TABLEDATA_SET;

enum {
	RAU_NEARFULL_NORMAL,
	RAU_NEARFULL_NEARFULL,
	RAU_NEARFULL_CANCEL		// 2:解除 #001
};

typedef struct {					/* send data queue 	*/
	ushort	Length;					/* data length 		*/
	uchar	Data[500];				/* data 			*/
} t_RauQue;

typedef t_RauQue T_RAU_ARCBLK;

typedef struct {
	ushort	Count; 		         	/* regist count */
	ushort	R_Index;	  	        /* next read offset index */
	ushort	W_Index;    	  	    /* next write offset index */
} t_RauQueCtrl;

typedef struct {
	uchar	BlkSts;					/* ARC block number */
	uchar	BlkNo;					/* ARC block status */
	ushort	Length;					/* data length 		*/
	uchar	Data[500];				/* data */
} t_RauSndQue;

typedef	struct {
	uchar	ModuleCode;				/* module code */
	uchar	ErrCode;				/* error code */
	uchar	f_Occur;				/* 1=occured , 0=release */
} t_RauErrCode;

typedef struct{
	uchar	OiBan;					/* sequence number */
	uchar 	ModuleCode;				/* module code */
	uchar 	ErrCode;				/* error code */
	uchar 	f_Occur;				/* 1=occuerd, 0=released */
} t_RauID65data;


/* バッファ */
typedef struct {
	short			len;							/* データ部サイズ */
	uchar			data[RAU_BLKSIZE];				/* バッファ本体 */
}T_RAU_BLKDATA;

/* ARCでのNT-NETブロックデータの受信 */
typedef struct {
	BOOL			waitblk;		/* 受信待ちブロックNo. */
	T_RAU_BLKDATA	blkdata;		/* 受信したブロックデータ */
	int				retry;			/* ブロックNo.異常連続発生回数 */
}T_RAU_ARC_RCVBLK;

typedef struct {
	ushort			len;
	uchar			data[1024];
}T_RAU_NTNETBLK;

#define	RAUARC_NODATA	-1

typedef struct {
	int				arcblknum;		/* 送信ARCブロック数(1～2:RAUARC_NODATA=データ無し) */
	int				snd_arcblknum;	/* 送信済みARCブロック数(0～2:2=全ARCブロック送信完了時) */
	int				ntblknum;		/* 送信NT-NETブロック数(1～26:RAUARC_NODATA=データ無し) */
	int				snd_ntblknum;	/* 送信済みNT-NETブロック数(0～26:26=全NT-NETブロック送信完了時) */
	T_RAU_NTNETBLK	ntblk;			/* 1NT-NETブロック取得用ワーク領域 */
	int				retry;			/* リトライ回数 */
	int				ntblkno_retry;	/* NT-NETブロックNo.異常による送信リトライ回数 */
	T_RAU_ARCBLK	arcblk[2];		/* 送信データ */
}T_RAU_ARC_SNDBLK;

#endif	/* ___RAUCONSTANTH___ */
