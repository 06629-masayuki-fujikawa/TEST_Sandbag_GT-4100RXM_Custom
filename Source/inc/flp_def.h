/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	ﾛｯｸ装置制御関連ﾍｯﾀﾞｰﾌｧｲﾙ																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author      : K.Akiba																					   |*/
/*| Date        : 2005-04-28																				   |*/
/*|																											   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#ifndef _FLP_DEF_H_
#define _FLP_DEF_H_

// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算DLLインターフェースの修正)
#include	"system.h"
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算DLLインターフェースの修正)
#include	"mem_def.h"
																				/*								*/
#define		LK_TYPE_AIDA1		1												/* 英田自転車					*/
#define		LK_TYPE_AIDA2		2												/* 英田ﾊﾞｲｸ						*/
#define		LK_TYPE_YOSHI		3												/* 吉増							*/
#define		LK_TYPE_KOMUZ		4												/* ｺﾑｽﾞ							*/
#define		LK_TYPE_HID			5												/* HID							*/
																				/* ﾌﾗｯﾌﾟ装置種別				*/
#define		LK_TYPE_AIDA_FLP	11												/* 英田製ﾌﾗｯﾌﾟ					*/
#define		LK_TYPE_HOUWA_FLP	12												/* 邦和製ﾌﾗｯﾌﾟ					*/
#define		LK_TYPE_EIKOU_FLP	13												/* 栄晃製ﾌﾗｯﾌﾟ					*/
#define		LK_TYPE_MEITO_FLP	14												/* ﾒｲﾄ製ﾌﾗｯﾌﾟ					*/
#define		LK_TYPE_SANICA_FLAP		15											/* サニカフラップ				*/
#define		LK_TYPE_CONTACT_FLAP	16											/* 接点フラップ					*/
#define		LK_TYPE_AIDA_FLP_CUSTOM			31									/* 英田製ﾌﾗｯﾌﾟ(停止3秒)			*/
#define		LK_TYPE_AIDA_FLP_CUSTOM_COLD	35									/* 英田製ﾌﾗｯﾌﾟ(停止3秒：寒冷地仕様)		*/

																				/* 装置種別						*/
#define		LK_KIND_ERR			0												/* 不明							*/
#define		LK_KIND_FLAP		1												/* ﾌﾗｯﾌﾟ装置					*/
#define		LK_KIND_LOCK		2												/* ﾛｯｸ装置						*/
#define		LK_KIND_INT_FLAP	3												/* 内蔵ﾌﾗｯﾌﾟ装置				*/

																				/* 装置No.範囲					*/
#define		FLAP_START_NO		CAR_START_INDEX+1								/* ﾌﾗｯﾌﾟ	開始No.				*/
#define		FLAP_END_NO			CAR_LOCK_MAX									/* ﾌﾗｯﾌﾟ	終了No.				*/
#define		LOCK_START_NO		BIKE_START_INDEX+1								/* ﾛｯｸ装置	開始No.				*/
#define		LOCK_END_NO			BIKE_START_INDEX+BIKE_LOCK_MAX					/* ﾛｯｸ装置	終了No.				*/
#define		INT_FLAP_START_NO	INT_CAR_START_INDEX+1							/* ﾌﾗｯﾌﾟ	開始No.				*/
#define		INT_FLAP_END_NO		INT_CAR_START_INDEX + INT_CAR_LOCK_MAX			/* ﾌﾗｯﾌﾟ	終了No.				*/
																				/*								*/
#define		LOCK_MAKER_CNT		6												/* ﾛｯｸ装置の種類数				*/
																				/*								*/
// フラップ状態データ
enum {								// ロック板状態
	FLP_LOCK_WAIT = '0',			// 待機中
	FLP_LOCK_DOWN,					// 下降中
	FLP_LOCK_UP,					// 上昇中
	FLP_LOCK_DOWN_ERR,				// 下降エラー
	FLP_LOCK_UP_ERR,				// 上昇エラー
	FLP_LOCK_FORCE_DOWN,			// 強制下降
	FLP_LOCK_FORCE_UP,				// 強制上昇
	FLP_LOCK_RESERVED,				// 予約（未使用）
	FLP_LOCK_INVALID,				// 不正ロック
};

// フラップ書き込みデータ
enum {								// 書き込みデータ
	FLP_COMMAND_FORCE_OFF = 1,		// 強制OFF
	FLP_COMMAND_FORCE_ON = 2,		// 強制ONJ
	FLP_COMMAND_UP = 4,				// フラップ上昇
	FLP_COMMAND_DOWN = 5,			// フラップ下降
};
union	bit_reg {																/*								*/
	uchar	BYTE;																/*								*/
	struct	bt_tag {															/*								*/
		uchar	YOBI07	: 1 ;													/*								*/
		uchar	SYUUS	: 1 ;													/*								*/
		uchar	FURIK	: 1 ;													/*								*/
		uchar	FUKUG	: 1 ;													/*								*/
		uchar	YOBI03	: 1 ;													/*								*/
		uchar	YOBI02	: 1 ;													/*								*/
		uchar	YOBI01	: 1 ;													/*								*/
		uchar	LAGIN   : 1 ;													/*								*/
	} BIT;																		/*								*/
};																				/*								*/
typedef union bit_reg	FLP_BIT;												/*								*/

union	bits_reg {																/*								*/
	struct	bit_tag {															/*								*/
		ushort	b15	: 1 ;														/*								*/
		ushort	b14	: 1 ;														/*								*/
		ushort	b13	: 1 ;														/*								*/
		ushort	b12	: 1 ;														/*								*/
		ushort	b11	: 1 ;														/*								*/
		ushort	b10	: 1 ;														/*								*/
		ushort	b09	: 1 ;														/*								*/
		ushort	b08	: 1 ;														/*								*/
		ushort	b07	: 1 ;														/*								*/
		ushort	b06	: 1 ;														/*								*/
		ushort	b05	: 1 ;														/*								*/
		ushort	b04	: 1 ;														/*								*/
		ushort	b03	: 1 ;														/*								*/
		ushort	b02	: 1 ;														/*								*/
		ushort	b01	: 1 ;														/*								*/
		ushort	b00	: 1 ;														/*								*/
	} bits;																		/*								*/
	ushort	word;																/*								*/
};																				/*								*/
typedef union bits_reg	BITS;													/*								*/
																				/*								*/
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
union	bits_reg_byte {															/*								*/
	struct	bit_tag_byte {															/*								*/
		uchar	b07	: 1 ;														/*								*/
		uchar	b06	: 1 ;														/*								*/
		uchar	b05	: 1 ;														/*								*/
		uchar	b04	: 1 ;														/*								*/
		uchar	b03	: 1 ;														/*								*/
		uchar	b02	: 1 ;														/*								*/
		uchar	b01	: 1 ;														/*								*/
		uchar	b00	: 1 ;														/*								*/
	} BIT;																		/*								*/
	uchar	byte;																/*								*/
};																				/*								*/
typedef union bits_reg_byte	BIT;												/*								*/
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
/* 英田製フラップ 下降ロックエラー発生時のパタパタ状態保護処理用(n分m回リトライ用) */
typedef union{
	uchar  BYTE;
	struct{
		uchar	EXEC:1;					// Bit7　：1=リトライ処理実行中
		uchar	TO_MSG:1;				// Bit6  ：1=タイムアウトメッセージ送信済み（タイマ開始時に0）
		uchar	YOBI:4;					// Bit2-5：予備
		uchar	RETRY_START_ERR:1;		// Bit1　：1=E1141（リトライ処理開始エラー）発生中
		uchar	RETRY_OVER_ERR:1;		// Bit0　：1=E1140（リトライオーバーエラー）発生中
	}BIT;								// ※E1140,E1141発生状況はここにだけ登録する。
										//   表示は無い為 ERR_CHK[]には反映しない。
} flp_FLAG_INFO;

typedef struct{
	ushort			TimerCount;			// 下降指示実行インターバル時間（n分：x500ms)：f_info.BIT.EXEC==1 時のみ有効
	uchar  			RetryCount;			// リトライ実行回数（下降指示送信回数）（m回：0 ～ 03-0074⑤⑥）
	flp_FLAG_INFO 	f_info;
} t_flp_DownLockErrInfo;
extern	t_flp_DownLockErrInfo	flp_DownLockErrInfo[LOCK_MAX];

enum{
	FLAP_CTRL_MODE1=0,	// 下降済み(車両なし)のときのｲﾍﾞﾝﾄ処理
	FLAP_CTRL_MODE2,	// 下降済み(車両あり､FTｶｳﾝﾄ中)のときのｲﾍﾞﾝﾄ処理
	FLAP_CTRL_MODE3,	// 上昇動作中のときのｲﾍﾞﾝﾄ処理
	FLAP_CTRL_MODE4,	// 上昇済み(駐車中)のときのｲﾍﾞﾝﾄ処理
	FLAP_CTRL_MODE5,	// 下降動作中のときのｲﾍﾞﾝﾄ処理
	FLAP_CTRL_MODE6,	// 下降済み(車両あり､LTｶｳﾝﾄ中)のときのｲﾍﾞﾝﾄ処理
};
																				/*------------------------------*/
typedef	struct	flp_dt_com {													/* Flap Car Data				*/
																				/*------------------------------*/
	ushort		mode;															/* Flap Mode					*/
																				/*------------------------------*/
	BITS		nstat;															/*								*/
	BITS		ostat;															/*								*/
//																				/* Bit 15 reserve				*/
//																				/* Bit 14 reserve				*/
// 今回追加した14・15の扱いとしては、BIT15がBIT9と同じ意味合いを持つﾌﾗｸﾞになってしまうが、
// BIT9はﾌﾗｯﾌﾟ(ﾛｯｸ)からﾃﾞｰﾀを受信すると、値をｸﾘｱしてしまうので、遠隔からの強制出庫があったかどうかを
// 判定するために追加。
// BIT14は遠隔からの要求でﾛｯｸ制御結果を送信する必要があった場合にMAFに対して送信するのか、
// NT-NETに対して送信するのかを判定するために使用する。
																				/* Bit 15 遠隔からのフラップ操作要求	*/
																				/* Bit 14 強制出庫時のﾛｯｸ制御結果データ送信用 */
																				/* Bit 13 下降異常ﾌﾗｸﾞ			*/
																				/* Bit 12 上昇異常ﾌﾗｸﾞ			*/
																				/* Bit 11 reserve				*/
																				/* Bit 10 reserve				*/
																				/* Bit  9 0:通常 1:NT-NET		*/
																				/* Bit  8 0:接続あり 1:接続なし	*/
																				/* Bit  7 0:通常 1:ﾒﾝﾃﾅﾝｽ		*/
																				/* Bit  6 0:不正ｸﾘｱ 1:発生		*/
																				/* Bit  5 0:下降ﾛｯｸ解除 1:発生	*/
																				/* Bit  4 0:上昇ﾛｯｸ解除 1:発生	*/
																				/* Bit  3 0:強制出庫なし 1:あり	*/
																				/* Bit  2 0:下降動作 1:上昇動作	*/
																				/* Bit  1 0:下降済み 1:上昇済み	*/
																				/* Bit  0 0:車両なし 1:車両あり	*/
																				/*------------------------------*/
	ushort		ryo_syu;														/* 料金種別(ﾏﾙﾁ精算用)			*/
																				/*------------------------------*/
	ushort		year;															/* 入庫年						*/
																				/*------------------------------*/
	uchar		mont;															/* 入庫月						*/
																				/*------------------------------*/
	uchar		date;															/* 入庫日						*/
																				/*------------------------------*/
	uchar		hour;															/* 入庫時						*/
																				/*------------------------------*/
	uchar		minu;															/* 入庫分						*/
																				/*------------------------------*/
	ushort		passwd;															/* ﾊﾟｽﾜｰﾄﾞ						*/
																				/*------------------------------*/
	ushort		uketuke;														/* 受付券発行ﾌﾗｸﾞ				*/
																				/*------------------------------*/
	ushort		u_year;															/* 受付券発行年					*/
																				/*------------------------------*/
	uchar		u_mont;															/* 受付券発行月					*/
																				/*------------------------------*/
	uchar		u_date;															/* 受付券発行日					*/
																				/*------------------------------*/
	uchar		u_hour;															/* 受付券発行時					*/
																				/*------------------------------*/
	uchar		u_minu;															/* 受付券発行分					*/
																				/*------------------------------*/
	ushort		bk_syu;															/* 種別(中止,修正用)			*/
																				/*------------------------------*/
	ushort		s_year;															/* 精算完了年					*/
																				/*------------------------------*/
	uchar		s_mont;															/* 精算完了月					*/
																				/*------------------------------*/
	uchar		s_date;															/* 精算完了日					*/
																				/*------------------------------*/
	uchar		s_hour;															/* 精算完了時					*/
																				/*------------------------------*/
	uchar		s_minu;															/* 精算完了分					*/
																				/*------------------------------*/
	FLP_BIT		lag_to_in;														/* ﾗｸﾞﾀｲﾏｰﾀｲﾑｱｳﾄによる再入庫ﾌﾗｸﾞ*/
																				/*------------------------------*/
	uchar		issue_cnt;														/* 駐車証明書発行回数			*/
																				/*------------------------------*/
	ushort		bk_wmai;														/* 使用枚数(中止,修正用)		*/
																				/*------------------------------*/
	ulong		bk_wari;														/* 割引金額(中止,修正用)		*/
																				/*------------------------------*/
	ulong		bk_time;														/* 割引時間数(中止,修正用)		*/
																				/*------------------------------*/
	ushort		bk_pst;															/* ％割引率(中止,修正用)		*/
																				/*------------------------------*/
	short		in_chk_cnt;														/* 入庫判定カウンタ				*/
																				/*------------------------------*/
	long		timer;															/* Timer( 500ms unit )			*/
	uchar		car_fail;														/* 車室故障						*/
																				/* 0:車室故障無し状態			*/
																				/* 1:車室故障あり状態			*/
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
																				/* Bit  7 予備					*/
																				/* Bit  6 予備					*/
																				/* Bit  5 予備					*/
																				/* Bit  4 予備					*/
																				/* Bit  3 予備					*/
																				/* Bit  2 予備					*/
																				/* Bit  1 長期駐車2 0:なし 1:あり*/
	BIT			flp_state;														/* Bit  0 長期駐車1 0:なし 1:あり*/
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は予備領域を使用すること!!!
	//	   メンバは最後尾に追加し、途中にメンバを挿入しないこと
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
//	ulong		Dummy[10];														/* 将来拡張用の予備エリア   	*/
	uchar		Dummy1[3];														/* 将来拡張用の予備エリア   	*/
	ulong		Dummy[9];														/* 将来拡張用の予備エリア   	*/
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
																				/*------------------------------*/
}	flp_com;																	/* 13 Byte						*/
																				/*------------------------------*/
																				/*								*/
struct	FLPCTL_rec {															/*								*/
																				/*------------------------------*/
	uchar		Comd_knd;														/* ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ種別				*/
	uchar		Comd_cod;														/* ﾒｯｾｰｼﾞｺﾏﾝﾄﾞｺｰﾄﾞ				*/
	ushort		Room_no;														/* 車室№						*/
	ushort		Ment_flg;														/* 0=ﾒﾝﾃﾅﾝｽ, 1=NT-NET			*/
	short		Flp_mv_tm[LOCK_MAX];											/* ﾌﾗｯﾌﾟ板(ﾛｯｸ装置)動作監視ﾀｲﾏｰ	*/
	short		Flp_uperr_tm[LOCK_MAX];											/* 上昇ﾛｯｸ(閉異常)ｴﾗｰ判定ﾀｲﾏｰ	*/
	short		Flp_dwerr_tm[LOCK_MAX];											/* 下降ﾛｯｸ(開異常)ｴﾗｰ判定ﾀｲﾏｰ	*/
	flp_com		flp_work;														/* Write Data Work				*/
																				/*------------------------------*/
};																				/*								*/
#define	_FLP_LAGIN		0x0f
#define	_FLP_FUKUGEN	0x10
#define	_FLP_FURIKAE	0x20
																				/*								*/
extern	struct FLPCTL_rec	FLPCTL;												/*								*/
																				/*								*/
typedef	struct	flp_dt_rec {													/*								*/
																				/*------------------------------*/
	flp_com		flp_data[LOCK_MAX];												/* Flap Car Data				*/
																				/*------------------------------*/
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は予備領域を使用すること!!!
	//	   メンバは最後尾に追加し、途中にメンバを挿入しないこと
	ushort		yobi;															/* 予備							*/
																				/*------------------------------*/
}	flp_rec;																	/*								*/
																				/*								*/
extern	flp_rec	FLAPDT;															/*								*/

//掛売券ﾃﾞｰﾀﾌｫｰﾏｯﾄ(精算中止用)
typedef struct {
	ushort			mise_no;										// 店Ｎｏ．
	uchar			maisuu;											// 枚数
} kake_tiket;

//精算中止情報 エリアＢ
typedef struct {
	date_time_rec	TInTime;										// 入庫日時
	ulong			WPlace;											// フラップ番号		2Byte→4Byteに変更
	uchar			sev_tik[15];									// サービス券券種毎の使用枚数
	kake_tiket		kake_data[5];									// 掛売券店No,使用枚数
	ulong			ppc_chusi_ryo;									// 精算中止ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ料金
	uchar			syu;											// 種別
	uchar			yobi[30];										// 予備		32→30変更
} flp_com_sub;

extern flp_com_sub FLAPDT_SUB[11];												/*0～9：精算中止ｴﾘｱ　10：ﾏﾙﾁ精算用中止ｴﾘｱ*/
extern	uchar	Flap_Sub_Flg;													/* Bエリアフラグ				*/
extern	uchar	Flap_Sub_Num;													/* Bエリア要素数				*/
																				/*								*/
typedef	struct {																/*								*/
	ulong		LockNo;															/* 区画情報						*/
	ushort		Answer;															/* 結果(0=正常,1=未接続)		*/
	flp_com		lock_mlt;														/* 車室情報						*/
	ulong			ppc_chusi_ryo_mlt;											/* 精算中止ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ料金		*/
	uchar			sev_tik_mlt[15];											/* サービス券券種毎の使用枚数	*/
	kake_tiket		kake_data_mlt[5];											/* 掛売券店No,使用枚数			*/
} lock_multi;																	/*								*/
																				/*								*/
extern	lock_multi	LOCKMULTI;													/* Flap Car Data(ﾏﾙﾁ精算用)		*/
																				/*								*/
extern	ulong	UketukeNoBackup[LOCK_MAX];										/* 受付券発行追番ﾊﾞｯｸｱｯﾌﾟ		*/
																				/*								*/
extern	uchar	SvsTime_Syu[LOCK_MAX];											/* 精算時の料金種別（種別毎ｻｰﾋﾞｽﾀｲﾑの選択に使用）	*/

extern	uchar	Lock_Kind;														/* ﾛｯｸ装置種別（ﾌﾗｯﾌﾟ／ﾛｯｸ装置）*/
																				/*								*/
extern	uchar	DownLockFlag[LOCK_MAX];

// 不具合修正(S) K.Onodera 2016/12/09 #1582 振替元のフラップが上昇中に電源を切ると、振替先のフラップが下降しなくなる
extern	ushort	FurikaeDestFlapNo;												// 振替先フラップ№(振替先フラップ下降指示中停電に対応)
// 不具合修正(E) K.Onodera 2016/12/09 #1582 振替元のフラップが上昇中に電源を切ると、振替先のフラップが下降しなくなる
// MH322916(S) A.Iiizumi 2018/05/16 長期駐車検出機能対応
extern	uchar	LongParkingFlag[LOCK_MAX];										//長期駐車状態管理フラグ 0:長期駐車状態なし、1:長期駐車状態あり
// MH322916(E) A.Iiizumi 2018/05/16 長期駐車検出機能対応
/*** function prototype ***/													/*								*/
																				/*								*/
/* fcmain.c */																	/*								*/
extern	void	fcmain( void );													/*								*/
extern	char	flp_faz1( ushort );												/*								*/
extern	char	flp_faz2( ushort );												/*								*/
extern	char	flp_faz3( ushort );												/*								*/
extern	char	flp_faz4( ushort );												/*								*/
extern	char	flp_faz5( ushort );												/*								*/
extern	char	flp_faz6( ushort );												/*								*/
extern	ushort	FlpMsg( void );													/*								*/
extern	void	FlpSet( ushort, uchar );										/*								*/
extern	void	flpst_pc( char, char );											/*								*/
extern	void	CarTimer( char, char );											/*								*/
extern	short	InChkTimer( void );
extern	long	LockTimer( void );												/*								*/
extern	long	LagTimer( ushort );												/*								*/
extern	void	FmvTimer( ushort, short );										/*								*/
extern	char	Sens_Chk( char );												/*								*/
extern	char	CarSen_Ck( char );												/*								*/
extern	void	CarTimMng( void );												/*								*/
extern	void	SrvTimMng( void );												/*								*/
extern	void	LkErrTimMng( void );											/*								*/
extern	void	LkTimChk( void );												/*								*/
extern	void	NTNET_Snd_Data05_Sub( void );									/*								*/
extern	uchar	LkKind_Get( ushort );											/*								*/
extern	uchar	m_mode_chk_psl( ushort no );
extern	uchar	m_mode_chk_all( uchar kind );
extern	void Flapdt_sub_clear(ushort no);
char	ope_Furikae_start(ushort no);
char	ope_Furikae_stop(ushort no, char bCancel);
void	ope_Furikae_fukuden(void);
extern	void	ope_SyuseiStart(ushort no);

extern	uchar	flp_DownLock_DownSendEnableCheck( void );
extern	uchar	flp_DownLock_DownSendEnableCheck2( ushort no );
extern	void	flp_DownLock_ErrSet( ushort no, char ErrNo, char knd );
extern	ulong	flp_ErrBinDataEdit( ushort no );
extern	void	flp_DownLock_Initial( void );
extern	void	flp_DownLock_RetryTimerStart( ushort no );
extern	void	flp_DownLock_RetryStop( ushort no );
extern	uchar	flp_DownLock_DownSend( ushort no );
extern	void	flp_DownLock_FlpSet( ushort no );
extern	void	flp_DownLock_lk_err_chk( ushort no, char ErrNo, char kind );
extern	uchar	flp_err_search( ushort, ulong* );
#endif	// _FLP_DEF_H_
