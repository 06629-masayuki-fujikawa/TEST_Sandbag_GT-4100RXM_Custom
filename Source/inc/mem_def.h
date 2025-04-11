/*[]----------------------------------------------------------------------[]*/
/*| Memory Area Definition                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hashimoto                                             |*/
/*| Date        :  2001-10-26                                              |*/
/*[]------------------------------------- Copyright(C) 2000 AMANO Corp.---[]*/
#ifndef	___MEM_DEFH___
#define	___MEM_DEFH___

#include	"flashdef.h"
// MH810103 GG119202(S) 処理未了取引記録に再精算情報を印字する
//// MH321800(S) G.So ICクレジット対応
//#include	"suica_def.h"
//// MH321800(E) G.So ICクレジット対応
// MH810103 GG119202(E) 処理未了取引記録に再精算情報を印字する
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
#include	"pkt_def.h"
#include	"pktctrl.h"
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
extern	SWITCH_DATA	BootInfo;					// 起動時面選択情報

/*--------------------------------------------------------------------------*/
/*	設定ﾃﾞｰﾀ																*/
/*--------------------------------------------------------------------------*/
struct	Machine_rec {
	uchar			read_dat[12+9];									// ｻﾌﾞﾘｰﾄﾞﾃﾞｰﾀ
	uchar			sirial_no[12];									// ｼﾘｱﾙ№
	uchar			board_no[12];									// 基板情報
};

extern	struct	Machine_rec	MACHN_INF;								// 機器情報


/*--------------------------------------------------------------------------*/
/*	ｿﾌﾄｳｴｱﾊﾞｰｼﾞｮﾝ															*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	uchar			ver_devi;										// Device  Name
	uchar			ver_file;										// Machine Name
	uchar			ver_part[6];									// Parts No.
	uchar			ver_romn[2];									// Software Version No.00
} ver_rec;

extern	const	ver_rec		VERSNO;									// ROM Table
// GG120600(S) // Phase9 Versionを分ける
extern	const	ver_rec		VERSNO_BASE;							// Base Table
extern	const	ver_rec		VERSNO_RYOCAL;							// CalDll Table
// GG120600(E) // Phase9 Versionを分ける


/*--------------------------------------------------------------------------*/
/*	ﾊﾟｽﾜｰﾄﾞﾃｰﾌﾞﾙ															*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	uchar			pas_word[5];									// RAM  Password
	uchar			pas_trap;										// Trap Flag   00H : OK
																	//            0FFH : NG
	ushort			pas_onda;										// Power On Time Date
	ushort			pas_onhm;										//               hh:mm
	ushort			pas_ofda;										// Power Off Time Date
	ushort			pas_ofhm;										//                hh:mm
} pas_rec;

extern	const pas_rec		PASSDF;									// ROM Address
extern	pas_rec		PASSPT;											// RAM Address


/*--------------------------------------------------------------------------*/
/*	動作ﾌﾗｸﾞ																*/
/*--------------------------------------------------------------------------*/
typedef union{
	unsigned char	BYTE;								// Byte
	struct{
		unsigned char	YOBI:1;					// Bit 7	= 予備
		unsigned char	MESSAGEFULL:1;			// Bit 6	= メッセージバッファFULL
		unsigned char	PARAM_INVALID_CHK:1;			// Bit 5	= 起動時のパラメータ変更処理実施フラグ 0:起動時以外からのﾁｪｯｸ 1:起動時のﾁｪｯｸ
		unsigned char	PARAM_LOG_REC:1;				// Bit 4	= 起動時の設定更新履歴登録処理フラグ 0:登録予約なし 1:登録予約あり
		unsigned char	ILLEGAL_FLOATINGPOINT:1;		// Bit 3	= Excep_FloatingPoint用(浮動小数点例外)
		unsigned char	ILLEGAL_SUPERVISOR_INST:1;		// Bit 2	= Excep_SuperVisorInst用(特権命令例外)
		unsigned char	ILLEGAL_INSTRUCTION:1;			// Bit 1	= Excep_UndefinedInst用 (未定義命令例外)
		unsigned char	UNKOWN_VECT:1;					// Bit 0	= UnKown_Vect代表用領域
	} BIT;
} t_event_CtrlBitData;

#define		NMI_DAT_MAX	2048

typedef struct {
	ushort		flg_ocd;										// Open Close Receive Date
	ushort		flg_oct;										// Open Close Receive Time
	ushort		flg_ock;										// Open Close Kind (0=自動 1=営業 2=休業)
} RECEIVE_REC;

typedef	struct {
	RECEIVE_REC		receive_rec;
	ushort			nmi_mod;										// nmisave()/nmicler() Mode
	ushort			nmi_siz;										// Size
	void			*nmi_adr;										// Address
	uchar			nmi_dat[NMI_DAT_MAX];							// Save Data
	ushort			nmi_flg;										// nmi処理ﾌﾗｸﾞ
	uchar			car_full[4];										// NT-NET用満空ｽﾃｰﾀｽ

	ushort			memflg;
	t_event_CtrlBitData 	event_CtrlBitData;						// 割り込み制御用エラー登録
																	// BIT 0  ：UnKown_Vect代表用領域
																	// BIT 1  ：Excep_UndefinedInst用 (未定義命令例外)
																	// BIT 2  ：Excep_SuperVisorInst用(特権命令例外)
																	// BIT 3  ：Excep_FloatingPoint用(浮動小数点例外)
} flg_rec;

extern	flg_rec		FLAGPT;

#define	_MEMFLG_VAL		((ushort)0x2b54)			// '+T'

extern	ushort		clr_req;
#define		_CLR_HIFMEM		0x0001
#define		_CLR_CRNMEM		0x0010
#define		_CLR_ALLMEM		(_CLR_HIFMEM|_CLR_CRNMEM)

/*--------------------------------------------------------------------------*/
/* 状態監視																	*/
/*--------------------------------------------------------------------------*/
struct	AC_FLG {
	char			cycl_fg;										// 精算ｻｲｸﾙﾌﾗｸﾞ
	char			syusyu;											// 集計ｻｲｸﾙﾌﾗｸﾞ
	char			turi_syu;										// 釣銭管理ｻｲｸﾙﾌﾗｸﾞ(停電用)
// MH321800(S) G.So ICクレジット対応
	char			ec_alarm;										// 決済リーダ復電用フラグ
// MH321800(E) G.So ICクレジット対応
// MH810103 GG119202(S) 決済精算中止印字処理修正
	uchar			ec_deemed_fg;									// みなし決済復電用フラグ
// MH810103 GG119202(E) 決済精算中止印字処理修正
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
	uchar			ec_recv_deemed_fg;								// 直前取引データ受信フラグ（処理フラグ）
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
};

extern	struct	AC_FLG	ac_flg;


/*--------------------------------------------------------------------------*/
/* 定期券ﾃｰﾌﾞﾙ																*/
/*--------------------------------------------------------------------------*/
#define		PAS_MAX		12000										// Pass Card MAX

typedef union{
	uchar	BYTE;													// Byte
	struct{
		uchar	INV:1;												// Bit 7		0:有効, 1:無効
		uchar	PKN:2;												// Bit 5～6		利用中駐車場 0=基本, 1=拡張1, 2=拡張2, 3=拡張3
		uchar	RSV:1;												// Bit 4		予備
		uchar	STS:4;												// Bit 0～3		0:初期状態, 1:出庫中, 2,入庫中
	} BIT;
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は予備領域を使用すること!!!
} PAS_TBL;

// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)
//extern	PAS_TBL		pas_tbl[PAS_MAX];								// 定期券ｽﾃｰﾀｽﾃｰﾌﾞﾙ(12000)
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)

/*--------------------------------------------------------------------------*/
/* 定期券更新ﾃｰﾌﾞﾙ															*/
/*--------------------------------------------------------------------------*/
typedef union{
	uchar	BYTE;													// Byte
	struct{
		uchar	Bt67:2;												// Bit 6,7		定期4  0:更新許可, 1:更新不可
		uchar	Bt45:2;												// Bit 4,5		定期3  0:更新許可, 1:更新不可
		uchar	Bt23:2;												// Bit 2,3		定期2  0:更新許可, 1:更新不可
		uchar	Bt01:2;												// Bit 0,1		定期1  0:更新許可, 1:更新不可
	} BIT;
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は別途処理を記述しないと
	//	   バックアップ/リストアが正常終了しません。
} PAS_RENEWAL;

// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//extern	PAS_RENEWAL	pas_renewal[PAS_MAX/4];							// 定期券更新ﾃｰﾌﾞﾙ(12000)
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)

/*--------------------------------------------------------------------------*/
/* 定期券出庫時刻ﾃｰﾌﾞﾙ														*/
/*--------------------------------------------------------------------------*/
#define		PASS_EXTIMTBL_MAX	1000								// Pass Exit Time Tbl MAX

#pragma pack

typedef struct {
	ushort			PassId;											// 定期券ID
	ulong			ParkNo;											// 駐車場№
	date_time_rec	PassTime;										// 出庫時刻
} PASS_EXTBL;
#pragma unpack

typedef struct {
	ushort			Count;											// 定期券出庫時刻ﾃｰﾌﾞﾙ登録件数
	PASS_EXTBL		PassExTbl[PASS_EXTIMTBL_MAX];
} PASS_EXTIMTBL;

// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)
//extern	PASS_EXTIMTBL	pas_extimtbl;
// MH810100(E) K.Onodera  2019/12/26 車番チケットレス(不要処理削除)

/*--------------------------------------------------------------------------*/
/* 動作ｶｳﾝﾄ	 																*/
/*--------------------------------------------------------------------------*/
enum {
	MOV_PDWN_CNT = 0,													// 復電回数
	MOV_PRNT_CNT,														// ﾌﾟﾘﾝﾀ印字行数
	MOV_CUT_CNT,														// ﾌﾟﾘﾝﾀｶｯﾀｰ動作回数
	MOV_JPNT_CNT,														// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ印字行数
	MOV_COIN_RED,														// 硬貨読取枚数
	MOV_COIN_OUT,														// 硬貨払出枚数
	MOV_NOTE_RED,														// 紙幣読取枚数
	MOV_NOTE_BOX,														// 紙幣金庫抜取回数 MH544401 GW844900 紙幣金庫スタッカ開閉通知変更
// MH810100(S) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)	
//	READ_SHUT_CT,														// 磁気ﾘｰﾀﾞｰｼｬｯﾀｰ動作回数
//	READ_DO,															// 磁気ﾘｰﾀﾞｰ動作回数
//	READ_YO,															// 読取動作回数
//	READ_ER,															// 読取ｴﾗｰ回数
//	READ_WR,															// 書き込み回数
//	READ_VN,															// ﾍﾞﾘﾌｧｲNG回数
//	VPRT_DO,															// 磁気ﾘｰﾀﾞｰﾌﾟﾘﾝﾀ動作回数
// MH810100(E) Y.Yamauchi 2019/10/07 車番チケットレス(メンテナンス)
	LCD_LIGTH_CNT,														// LCDﾊﾞｯｸﾗｲﾄ点灯動作時間
// MH810100(S) Y.Yamauchi 2019/10/16 車番チケットレス(メンテナンス)
	QR_READER_CNT,														// QRﾘｰﾀﾞｰ動作時間
// MH810100(E) Y.Yamauchi 2019/10/16 車番チケットレス(メンテナンス)	
	MOV_CNT_MAX															// 動作ｶｳﾝﾄ数
};
extern	ulong	Mov_cnt_dat[MOV_CNT_MAX];								// 動作ｶｳﾝﾄ

#pragma pack
/*--------------------------------------------------------------------------*/
/* 追い番																	*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ulong			w;			// whole
	ulong			i;			// individual
} ST_OIBAN;
#pragma unpack
// IOLOG_DATAでpackしたST_OIBANを使用すると、アライメント2バイトと判断されるようになり
// エラーが出てしまうため、IOLOG_DATAではpackしていない同じ構造体ST_OIBAN2を使用する
typedef	struct {
	ulong			w;			// whole
	ulong			i;			// individual
} ST_OIBAN2;

/*--------------------------------------------------------------------------*/
/* 修正精算用ﾃﾞｰﾀｴﾘｱ														*/
/*--------------------------------------------------------------------------*/
struct	SYUSEI{														// 修正用ﾃﾞｰﾀ
	uchar			sei;											// 修正精算情報 0=無し/1=有り/2=無し(ｻｰﾋﾞｽﾀｲﾑ考慮)
	ulong			tryo;											// 元々の駐車料金(定期含まず)
	ulong			ryo;											// 駐車料金(定期料金)
	ulong			gen;											// 現金領収額
	ulong			tax;											// 消費税額
	ushort			iyear;											// 入車時刻
	uchar			imont;											//
	uchar			idate;											//
	uchar			ihour;											//
	uchar			iminu;											//
	ushort			oyear;											// 出車時刻
	uchar			omont;											//
	uchar			odate;											//
	uchar			ohour;											//
	uchar			ominu;											//
	uchar			ot_car;											// 0=ﾃﾞｰﾀなし
																	// 1=ﾌﾗｯﾌﾟ下降ﾗｸﾞﾀｲﾑ内
																	// 2=出庫
																	// 3=次車両入庫ﾌﾗｯﾌﾟ上昇ﾀｲﾑ内
																	// 4=次車両入庫ﾌﾗｯﾌﾟ上昇
																	// 5=ﾗｸﾞﾀｲﾑｱｯﾌﾟ後ﾌﾗｯﾌﾟ上昇(再入庫)
	uchar			infofg;											// ** 修正精算情報ﾌﾗｸﾞ **
																	// b8=(未使用)			0x80
																	// b7=領収証発行		0x40
																	// b6=ｱﾝﾁﾊﾟｽOFF			0x20
																	// b5=ｻｰﾋﾞｽﾀｲﾑ内出庫	0x10
																	// b4=(未使用)			0x08
																	// b3=(未使用)			0x04
																	// b2=(未使用)			0x02
																	// b1=全額割引			0x01

	ulong			sy_wari;										// 料金割引金額
	ulong			sy_time;										// 時間割引金額
	uchar			sy_wmai;										// 使用枚数
	uchar			tei_syu;										// 定期種別(1～15)
																	// b1～b6:定期券種別
																	// b7,b8 :駐車場№(0=基本、1～3=拡張1～3)
	ushort			tei_id;											// 個人ｺｰﾄﾞ(1～12000)
	ushort			tei_sd;											// 有効開始年月日ﾉｰﾏﾗｲｽﾞ
	ushort			tei_ed;											// 有効終了年月日ﾉｰﾏﾗｲｽﾞ
	uchar			syubetu;										// 料金種別
	ST_OIBAN		oiban;											// 精算追番(修正元の精算追番)
	ushort			bun_syu1;
	ushort			bun_syu2;										// 1の位		0:なし/1:通常/2:以上/3:不明
																	// 10の位		0:なし/1:ｴﾘｱ1/2:ｴﾘｱ2/3:ｴﾘｱ3
																	// 100～の位	0:なし/1～48:1～48分類目
};
extern	struct	SYUSEI	syusei[LOCK_MAX];

#define		SSS_ZENWARI		0x01									// 全額割引
#define		SSS_SVTIME		0x10									// ｻｰﾋﾞｽﾀｲﾑ内精算
#define		SSS_ANTIOFF		0x20									// ｱﾝﾁﾊﾟｽOFF
#define		SSS_RYOUSYU		0x40									// 領収証発行

/*--------------------------------------------------------------------------*/
/* 不正出庫用ﾃﾞｰﾀｴﾘｱ														*/
/*--------------------------------------------------------------------------*/
struct	FUSEI_D {													// 不正出庫車入出庫ﾃﾞｰﾀ
	ushort			iyear;											// 入車年
	uchar			imont;											// 入車月
	uchar			idate;											// 入車日
	uchar			ihour;											// 入車時
	uchar			iminu;											// 入車分
	ushort			oyear;											// 出車年
	uchar			omont;											// 出車月
	uchar			odate;											// 出車日
	uchar			ohour;											// 出車時
	uchar			ominu;											// 出車分
	ushort			t_iti;											// 駐車位置
	uchar			kyousei;										// 強制出庫ﾌﾗｸﾞ
																	// 0=通常の不正出庫
																	// 1=通常の強制出庫
																	// 2=修正精算の不正出庫(台数ｶｳﾝﾄなし、修正元の修正ﾊﾞｯﾌｧの入出庫時刻)
																	// 3=修正精算の不正出庫(台数ｶｳﾝﾄあり、修正元の現在の入出庫)
};

struct	FUSEI_SD {
	struct	FUSEI_D		fus_d[LOCK_MAX];							// 不正ﾃﾞｰﾀLOCK_MAX件分
	unsigned short		kensuu;										// 件数
};

extern	struct	FUSEI_SD	fusei;
extern	struct	FUSEI_SD	wkfus;									// 待避･ﾜｰｸｴﾘｱ
extern	struct	FUSEI_SD	TempFus;								// 不正・強制出庫ﾃﾞｰﾀ更新用ﾜｰｸｴﾘｱ（電源断時非保存）
extern	struct	FUSEI_SD	locktimeout;							// ロック閉・ﾌﾗｯﾌﾟ上昇タイマ内出庫データ格納
extern	struct	FUSEI_SD	wklocktimeout;							// 待避･ﾜｰｸｴﾘｱ


/*--------------------------------------------------------------------------*/
/* 各種集計数 																*/
/*--------------------------------------------------------------------------*/
// GG129000(S) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
// #define		WTIK_USEMAX			10									// ｻｰﾋﾞｽ券,掛売券,割引券,回数券使用可能最大枚数
#define		WTIK_USEMAX			11									// 割引券使用可能最大種別数（1枠は買上券用の予備）
// GG129000(E) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
#define		PKICHI_KU_CNT		26									// 駐車位置区画数（1～26=A～Z）
#define		RYOUKIN_SYU_CNT		12									// 料金種別数
#define		TEIKI_SYU_CNT		15									// 定期券種別数
#define		PKNO_SYU_CNT		4									// 駐車場Ｎｏ．種別数
#define		PKNO_WARI_CNT		2									// 店Ｎｏ．エリア確保
#define		SERVICE_SYU_CNT		15									// ｻｰﾋﾞｽ券種別数
#define		MISE_NO_CNT			100									// 店Ｎｏ．数
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
#define		T_MISE_NO_CNT		999									// 多店舗割引 店Ｎｏ．数
#define		T_MISE_SYU_CNT		15									// 多店舗割引 種別数
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5793 多店舗割引店№999対応
#define		WARIBIKI_SYU_CNT	100									// 割引種別数（割引券）
#define		SIHEI_SYU_CNT		4									// 紙幣種別数
#define		COIN_SYU_CNT		4									// ｺｲﾝ種別数
#define		IN_SIG_CNT			3									// 入庫信号数（信号１，２、３）
#define		BUNRUI_CNT			48									// 分類集計区分数
#define		SYUUKEI_SYU_CNT		8									// 集計種別数
// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
#define		DETAIL_SYU_MAX		2									// 明細情報セット可能数
// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応
// MH810100(S) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
#define		KAIMONO_WARISYU_MAX	10									// 買物割引種別最大数
// MH810100(E) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
// MH810103 GG119202(S) 処理未了取引記録に再精算情報を印字する
#define		EC_BRAND_MAX		10									// ブランド数最大値
#define		EC_BRAND_TOTAL_MAX	(EC_BRAND_MAX*2)					// 総ブランド数最大値
// MH810103 GG119202(E) 処理未了取引記録に再精算情報を印字する

/*--------------------------------------------------------------------------*/
/* ｺｲﾝ金庫集計																*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ushort			Kikai_no;										// 機械№
	ushort			Kakari_no;										// 係員№
	ST_OIBAN		Oiban;											// 追番
	date_time_rec	NowTime;										// 今回集計日時
	date_time_rec	OldTime;										// 前回集計日時
	ulong			Tryo;											// ｺｲﾝ金庫総入金額
	ulong			ryo[COIN_SYU_CNT];								// ｺｲﾝ金庫入金額（金種毎）
	ulong			cnt[COIN_SYU_CNT];								// ｺｲﾝ金庫入金数（金種毎）
} COIN_SYU_V01;
typedef struct {
	ushort			Kikai_no;										// 機械№
	ushort			Kakari_no;										// 係員№
	ST_OIBAN		Oiban;											// 追番
	date_time_rec	NowTime;										// 今回集計日時
	date_time_rec	OldTime;										// 前回集計日時
	ulong			Tryo;											// ｺｲﾝ金庫総入金額
	ulong			ryo[COIN_SYU_CNT];								// ｺｲﾝ金庫入金額（金種毎）
	ulong			cnt[COIN_SYU_CNT];								// ｺｲﾝ金庫入金数（金種毎）
	uchar			SeqNo;											// シーケンシャルNo.
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は別途処理を記述しないと
	//	   バックアップ/リストアが正常終了しません。
} COIN_SYU_V04;
typedef COIN_SYU_V04	COIN_SYU;

extern	COIN_SYU	coin_syu;
extern	COIN_SYU	cobk_syu;

/*--------------------------------------------------------------------------*/
/* 紙幣金庫集計																*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ushort			Kikai_no;										// 機械№
	ushort			Kakari_no;										// 係員№
	ST_OIBAN		Oiban;											// 追番
	date_time_rec	NowTime;										// 今回集計日時
	date_time_rec	OldTime;										// 前回集計日時
	ulong			Tryo;											// 紙幣金庫総入金額
	ulong			ryo[SIHEI_SYU_CNT];								// 紙幣金庫入金額（金種毎）
	ulong			cnt[SIHEI_SYU_CNT];								// 紙幣金庫入金数（金種毎）
} NOTE_SYU_V01;
typedef struct {
	ushort			Kikai_no;										// 機械№
	ushort			Kakari_no;										// 係員№
	ST_OIBAN		Oiban;											// 追番
	date_time_rec	NowTime;										// 今回集計日時
	date_time_rec	OldTime;										// 前回集計日時
	ulong			Tryo;											// 紙幣金庫総入金額
	ulong			ryo[SIHEI_SYU_CNT];								// 紙幣金庫入金額（金種毎）
	ulong			cnt[SIHEI_SYU_CNT];								// 紙幣金庫入金数（金種毎）
	uchar			SeqNo;											// シーケンシャルNo.
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は別途処理を記述しないと
	//	   バックアップ/リストアが正常終了しません。
} NOTE_SYU_V04;
typedef NOTE_SYU_V04	NOTE_SYU;

extern	NOTE_SYU	note_syu;
extern	NOTE_SYU	nobk_syu;


#pragma pack
/*--------------------------------------------------------------------------*/
/* 釣銭管理集計																*/
/*--------------------------------------------------------------------------*/
typedef struct {
	uchar			Year;											// 年
	uchar			Mon;											// 月
	uchar			Day;											// 日
	uchar			Hour;											// 時
	uchar			Min;											// 分
	uchar			Sec;											// 秒
} proc_date;

typedef struct {
// ※以下はushort値
	ushort			gen_mai;										// 現在（変更前）保有枚数
	ushort			ygen_mai;										// 現在（変更前）保有枚数(予蓄)
	ushort			zen_mai;										// 前回保有枚数
	ushort			yzen_mai;										// 前回保有枚数(予蓄)
	ushort			sin_mai;										// 新規設定枚数
	ushort			ysin_mai;										// 新規設定枚数(予蓄)
// ※以後は積算するのでulong値
	ulong			sei_nyu;										// 精算時入金枚数
	ulong			sei_syu;										// 精算時出金枚数
	ulong			ysei_syu;										// 精算時出金枚数(予蓄)
	ulong			hojyu;											// 釣銭補充枚数
	ulong			hojyu_safe;										// 釣銭補充時金庫搬送枚数
	ulong			kyosei;											// 強制払出枚数
	ulong			ykyosei;										// 強制払出枚数(予蓄)
} TURI_kan_rec_V01;

typedef struct {
	ushort			Kikai_no;										// 機械№
	ushort			Kakari_no;										// 係員№
	ST_OIBAN		Oiban;											// 追番
	date_time_rec	NowTime;										// 今回集計日時
	date_time_rec	OldTime;										// 前回集計日時
	proc_date		ProcDate;										// 処理日時
	date_time_rec	PayDate;										// 精算日時
	ulong			PayCount;										// 精算追い番
	uchar			PayClass;										// 処理区分
	short			safe_dt[4];										// Coin Safe Count(コイン金庫枚数)
	short			nt_safe_dt;										// Note Safe Count(紙幣金庫枚数)
	TURI_kan_rec_V01	turi_dat[4];									// 金銭ﾃﾞｰﾀ(4金種分)
	ushort			sub_tube;										// サブチューブ金種
} TURI_KAN_V01;

typedef struct {
	ushort			Kikai_no;										// 機械№
	ushort			Kakari_no;										// 係員№
	ST_OIBAN		Oiban;											// 追番
	date_time_rec	NowTime;										// 今回集計日時
	date_time_rec	OldTime;										// 前回集計日時
	proc_date		ProcDate;										// 処理日時
	date_time_rec	PayDate;										// 精算日時
	ulong			PayCount;										// 精算追い番
	uchar			PayClass;										// 処理区分
	short			safe_dt[4];										// Coin Safe Count(コイン金庫枚数)
	short			nt_safe_dt;										// Note Safe Count(紙幣金庫枚数)
	TURI_kan_rec_V01	turi_dat[4];								// 金銭ﾃﾞｰﾀ(4金種分)
	ushort			sub_tube;										// サブチューブ金種
	ulong			CenterSeqNo;									// センター追番
	ulong			dummy;											// 予備
} TURI_KAN_V02;
typedef struct {
// ※以下はushort値
	ushort			gen_mai;										// 現在（変更前）保有枚数
	ushort			ygen_mai;										// 現在（変更前）保有枚数(予蓄)
	ushort			zen_mai;										// 前回保有枚数
	ushort			yzen_mai;										// 前回保有枚数(予蓄)
	ushort			sin_mai;										// 新規設定枚数
	ushort			ysin_mai;										// 新規設定枚数(予蓄)
// ※以後は積算するのでulong値
	ulong			sei_nyu;										// 精算時入金枚数
	ulong			sei_syu;										// 精算時出金枚数
	ulong			ysei_syu;										// 精算時出金枚数(予蓄)
	ulong			hojyu;											// 釣銭補充枚数
	ulong			hojyu_safe;										// 釣銭補充時金庫搬送枚数
	ulong			kyosei;											// 強制払出枚数
	ulong			ykyosei;										// 強制払出枚数(予蓄)
	uchar			SeqNo;											// シーケンシャルNo.
} TURI_kan_rec_V04;
typedef TURI_kan_rec_V04	TURI_kan_rec;


typedef struct {
	ushort			Kikai_no;										// 機械№
	ushort			Kakari_no;										// 係員№
	ST_OIBAN		Oiban;											// 追番
	date_time_rec	NowTime;										// 今回集計日時
	date_time_rec	OldTime;										// 前回集計日時
	proc_date		ProcDate;										// 処理日時
	date_time_rec	PayDate;										// 精算日時
	ulong			PayCount;										// 精算追い番
	uchar			PayClass;										// 処理区分
	short			safe_dt[4];										// Coin Safe Count(コイン金庫枚数)
	short			nt_safe_dt;										// Note Safe Count(紙幣金庫枚数)
	TURI_kan_rec	turi_dat[4];									// 金銭ﾃﾞｰﾀ(4金種分)
	ushort			sub_tube;										// サブチューブ金種
	ulong			CenterSeqNo;									// センター追番
	uchar			SeqNo;											// シーケンシャルNo.
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は予備領域を使用すること!!!
	//	   メンバは最後尾に追加し、途中にメンバを挿入しないこと
	uchar			dummy[3];										// 予備
} TURI_KAN_V04;
typedef TURI_KAN_V04	TURI_KAN;

extern	TURI_KAN	turi_kan;
extern	TURI_KAN	turi_kwk;										// 編集用ﾜｰｸｴﾘｱ
extern	TURI_KAN	turi_log_wk;									// 金銭管理ログ読出しワークエリア

struct	turi_dat_rec{												// 釣銭管理ﾜｰｸ
	ushort			turi_in;										// 釣銭補充操作有無ﾌﾗｸﾞ
	short			coin_dsp[7];									// 表示用ﾜｰｸ
	short			dsp_ini[7];										// 表示用ﾜｰｸ初期値
	short			coin_sin[7];									// 新規枚数設定ﾜｰｸ
	short			incount[4];										// 入金ｺｲﾝ枚数ﾜｰｸ
	short			outcount[7];									// 出金ｺｲﾝ枚数ﾜｰｸ
	short			pay_safe[4];									// 精算時の金庫入金枚数
	short			forceout[7];									// 強制払出枚数ﾜｰｸ
};

extern	struct	turi_dat_rec	turi_dat;

#define		TURIKAN_DEFAULT_NOWAIT	0	// デフォルトセット待ち解除
#define		TURIKAN_DEFAULT_WAIT	1	// デフォルトセット待ち
extern	uchar	turi_kan_f_defset_wait;	
#define		TURIKAN_AUTOSTART_NOEXE	0	// 釣合せ未実行
#define		TURIKAN_AUTOSTART_EXE	1	// 釣合せ実行済み
extern	uchar	turi_kan_f_exe_autostart;	/* 釣合せ実行フラグ 1：実行済み、0：未実行*/

#pragma unpack

#pragma pack
/*--------------------------------------------------------------------------*/
/* 駐車位置別集計ｴﾘｱ														*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			Genuri_ryo;										// 現金売上額
	ushort			Seisan_cnt;										// 精算回数
	ushort			Husei_out_cnt;									// 不正出庫回数
	ulong			Husei_out_ryo;									// 不正出庫金額
	ushort			Kyousei_out_cnt;								// 強制出庫回数
	ulong			Kyousei_out_ryo;								// 強制出庫金額
	ushort			Uketuke_pri_cnt;								// 受付券発行回数
	ushort			Syuusei_seisan_cnt;								// 修正精算回数
	ulong			Syuusei_seisan_ryo;								// 修正精算払戻金額
} LOKTOTAL_DAT;

extern	LOKTOTAL_DAT	wkloktotal;									// 駐車位置別集計待避・ﾜｰｸｴﾘｱ

typedef struct {
	ushort			Kikai_no;										// 機械№
	ushort			Kakari_no;										// 係員№
	ST_OIBAN		Oiban;											// 追番
	date_time_rec	NowTime;										// 今回集計日時
	date_time_rec	OldTime;										// 前回集計日時
	LOKTOTAL_DAT	loktldat[LOCK_MAX];								// 駐車位置別集計(駐車位置毎)
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は別途処理を記述しないと
	//	   バックアップ/リストアが正常終了しません。
} LOKTOTAL;

struct	LOKTL{
	LOKTOTAL		tloktl;											// 駐車位置別集計(T集計用)
	LOKTOTAL		gloktl;											// 駐車位置別集計(GT集計用)
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は最後尾に追加すること!!!
	//	   途中にメンバを挿入しないこと
};

extern	struct	LOKTL	loktl;
extern	struct	LOKTL	loktlbk;									// 駐車位置別集計前回分
// MH321800(S) G.So ICクレジット対応
/*--------------------------------------------------------------------------*/
/* 決済リーダブランド毎集計ｴﾘｱ 												  */
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			sei_cnt;										// 精算＜件数＞
	ulong			sei_ryo;										//     ＜金額＞
	ulong			alm_cnt;										// アラーム取引＜件数＞
	ulong			alm_ryo;										//             ＜金額＞
} EC_SYU_DATA;
// MH321800(E) G.So ICクレジット対応
// MH322914 (s) kasiyama 2016/07/15 AI-V対応
/*--------------------------------------------------------------------------*/
/* 集計		 																*/
/*--------------------------------------------------------------------------*/
// 不明フラグ詳細理由
enum {
	fumei_LOST = 0,			// 0:紛失精算
//	fumei_FIRST_PASS,		// 1:初回定期
//	fumei_ANTIPASS_OFF,		// 2:アンチパスOFF
//	fumei_FORCE_OUT,		// 3:強制出庫
//	fumei_CARD_NG,			// 4:カード異常
	fumei_UNKNOWN_MAX,		// 不明詳細理由数
};
// MH322914 (e) kasiyama 2016/07/15 AI-V対応
/*--------------------------------------------------------------------------*/
/* 集計		 																*/
/*--------------------------------------------------------------------------*/
enum {
	_OBN_WHOLE,				// 通し追い番
	_OBN_PAYMENT,			// 精算追い番
	_OBN_DEPOSIT,			// 預り証追い番
	_OBN_CANCEL,			// 精算中止追い番
	_OBN_TURIKAN,			// 釣銭管理合計追い番
	_OBN_COIN_SAFE,			// コイン金庫合計追い番
	_OBN_NOTE_SAFE,			// 紙幣金庫合計追い番
	_OBN_KIYOUSEI,			// 強制出庫追番	
	_OBN_FUSEI,				// 不正出庫追番	
	_OBN_reserve,
	_OBN_T_TOTAL,			// T合計追い番
	_OBN_GT_TOTAL,			// GT合計追い番

	_OBN_AREA_MAX
};

// 仕様変更(S) K.Onodera 2016/11/02 割引集計データフォーマット対応
// 注：カード決済区分と合わせること
enum{
	MOD_TYPE_MONEY,
	MOD_TYPE_CREDIT,
	MOD_TYPE_EMONEY,
	MOD_TYPE_MAX
};
// 仕様変更(E) K.Onodera 2016/11/02 割引集計データフォーマット対応

typedef struct {
	ushort			Kikai_no;										// 機械№
	ushort			Kakari_no;										// 係員№
	ST_OIBAN		Oiban;											// 追番
	date_time_rec	NowTime;										// 今回集計日時
	date_time_rec	OldTime;										// 前回集計日時
	ulong			oiban_range[_OBN_AREA_MAX][2];					// 各種追い番範囲
	ulong			Uri_Tryo;										// 総売上額
	ulong			Genuri_Tryo;									// 総現金売上額
	ulong			Kakeuri_Tryo;									// 総掛売額
	ulong			Tax_Tryo;										// 総税金額
	ulong			Seisan_Tcnt;									// 総精算回数
	ulong			In_car_Tcnt;									// 総入庫台数
	ulong			In_car_cnt[IN_SIG_CNT];							// 入庫台数（入庫信号１、２）
	ulong			Out_car_Tcnt;									// 総出庫台数
	ulong			Out_car_cnt[IN_SIG_CNT];						// 出庫台数（入庫信号１、２）
	ulong			Now_car_Tcnt;									// 総現在駐車台数
	ulong			Now_car_cnt[IN_SIG_CNT];						// 現在駐車台数（入庫信号１、２）

	ulong			Hunsei_Tcnt;									// 紛失精算＜回数＞
	ulong			Hunsei_Tryo;									// 　　　　＜金額＞

	ulong			Rsei_cnt[RYOUKIN_SYU_CNT];						// 料金種別毎精算＜回数＞（料金種別Ａ～Ｌ）
	ulong			Rsei_ryo[RYOUKIN_SYU_CNT];						// 　　　　　　　＜金額＞（料金種別Ａ～Ｌ）

	ulong			Rtwari_cnt[RYOUKIN_SYU_CNT];					// 料金種別毎時間割引＜回数＞（料金種別Ａ～Ｌ）
	ulong			Rtwari_ryo[RYOUKIN_SYU_CNT];					// 　　　　　　　　　＜金額＞（料金種別Ａ～Ｌ）

	ulong			Ccrd_sei_cnt;									// ｸﾚｼﾞｯﾄｶｰﾄﾞ精算＜回数＞
	ulong			Ccrd_sei_ryo;									// 　　　　　　　＜金額＞

	ulong			Pcrd_use_cnt[PKNO_SYU_CNT];						// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用＜回数＞（基本、拡張１，２，３）
	ulong			Pcrd_use_ryo[PKNO_SYU_CNT];						// 　　　　　　　　＜金額＞（基本、拡張１，２，３）

	ulong			Ktik_use_cnt[PKNO_SYU_CNT];						// 回数券使用＜回数＞（基本、拡張１，２，３）
	ulong			Ktik_use_ryo[PKNO_SYU_CNT];						// 　　　　　＜金額＞（基本、拡張１，２，３）

	ulong			Stik_use_cnt[PKNO_SYU_CNT][SERVICE_SYU_CNT];	// ｻｰﾋﾞｽ券種別毎使用＜枚数＞（基本、拡張１，２，３）（ｻｰﾋﾞｽ券Ａ，Ｂ，Ｃ）
	ulong			Stik_use_ryo[PKNO_SYU_CNT][SERVICE_SYU_CNT];	// 　　　　　　　　	＜金額＞（基本、拡張１，２，３）（ｻｰﾋﾞｽ券Ａ，Ｂ，Ｃ）

	ulong			Mno_use_Tcnt;									// 店Ｎｏ．割引総ﾄｰﾀﾙ＜回数＞
	ulong			Mno_use_Tryo;									// 　　　　　　　　　＜金額＞
	ulong			Mno_use_cnt1[PKNO_SYU_CNT];						// 店Ｎｏ．割引ﾄｰﾀﾙ＜回数＞（基本、拡張１，２，３）
	ulong			Mno_use_ryo1[PKNO_SYU_CNT];						// 　　　　　　　　＜金額＞（基本、拡張１，２，３）
	ulong			Mno_use_cnt2[PKNO_SYU_CNT];						// 店Ｎｏ．1～100	ﾄｰﾀﾙ割引＜回数＞（基本、拡張１，２，３）
	ulong			Mno_use_ryo2[PKNO_SYU_CNT];						// 　　　　　　　	　　　　＜金額＞（基本、拡張１，２，３）
	ulong			Mno_use_cnt3[PKNO_SYU_CNT];						// 店Ｎｏ．101～255	ﾄｰﾀﾙ割引＜回数＞（基本、拡張１，２，３）
	ulong			Mno_use_ryo3[PKNO_SYU_CNT];						// 　　　　　　　　	　　　　＜金額＞（基本、拡張１，２，３）
	ulong			Mno_use_cnt4[PKNO_WARI_CNT][MISE_NO_CNT];		// 店Ｎｏ．毎割引＜回数＞（基本、拡張１）（店Ｎｏ．１～１００）
	ulong			Mno_use_ryo4[PKNO_WARI_CNT][MISE_NO_CNT];		// 　　　　　　　＜金額＞（基本、拡張１）（店Ｎｏ．１～１００）

	ulong			Wtik_use_Tcnt;									// 割引券使用総ﾄｰﾀﾙ＜回数＞
	ulong			Wtik_use_Tryo;									// 　　　　　　　　＜金額＞
	ulong			Wtik_use_cnt1[PKNO_SYU_CNT];					// 割引券使用ﾄｰﾀﾙ＜回数＞（基本、拡張１，２，３）
	ulong			Wtik_use_ryo1[PKNO_SYU_CNT];					// 　　　　　　　＜金額＞（基本、拡張１，２，３）
	ulong			Wtik_use_cnt2[PKNO_WARI_CNT][WARIBIKI_SYU_CNT];	// 割引券割引種別毎使用＜回数＞（基本、拡張１）（割引種別１～１００）
	ulong			Wtik_use_ryo2[PKNO_WARI_CNT][WARIBIKI_SYU_CNT];	// 　　　　　　　　　　＜金額＞（基本、拡張１）（割引種別１～１００）

	ulong			Teiki_use_cnt[PKNO_SYU_CNT][TEIKI_SYU_CNT];		// 定期券種別毎使用＜回数＞（基本、拡張１，２，３）（定期１～１５）
	ulong			Teiki_kou_cnt[PKNO_SYU_CNT][TEIKI_SYU_CNT];		// 定期券種別毎更新＜回数＞（基本、拡張１，２，３）（定期１～１５）
	ulong			Teiki_kou_ryo[PKNO_SYU_CNT][TEIKI_SYU_CNT];		// 　　　　　　　　＜金額＞（基本、拡張１，２，３）（定期１～１５）

	ulong			Ryosyuu_pri_cnt;								// 領収証発行回数
	ulong			Azukari_pri_cnt;								// 預り証発行回数
	ulong			Uketuke_pri_Tcnt;								// 受付券発行回数

	ulong			In_svst_seisan;									// サービスタイム内精算回数
	ulong			Apass_off_seisan;								// アンチパスＯＦＦ精算回数

	ulong			Harai_husoku_cnt;								// 払出不足＜回数＞
	ulong			Harai_husoku_ryo;								// 　　　　＜金額＞

	ulong			Kinko_Tryo;										// 金庫総入金額
	ulong			Note_Tryo;										// 紙幣金庫総入金額
	ulong			Note_ryo[SIHEI_SYU_CNT];						// 紙幣金庫入金額（金種毎）
	ulong			Note_cnt[SIHEI_SYU_CNT];						// 紙幣金庫入金数（金種毎）

	ulong			Coin_Tryo;										// ｺｲﾝ金庫総入金額
	ulong			Coin_ryo[COIN_SYU_CNT];							// ｺｲﾝ金庫入金額（金種毎）
	ulong			Coin_cnt[COIN_SYU_CNT];							// ｺｲﾝ金庫入金数（金種毎）
																	// ＜＜　金銭データ　＞＞
	ulong			tounyu;											// 精算投入金額　総額
	ulong			tou[5];											// 精算投入金額　各金種毎
	ulong			hojyu;											// 釣銭補充金額　総額
	ulong			hoj[4];											// 釣銭補充金額　各金種毎
	ulong			seisan;											// 精算払出金額　総額
	ulong			sei[4];											// 精算払出金額　各金種毎
	ulong			kyosei;											// 強制払出金額　総額
	ulong			kyo[4];											// 強制払出金種　各金種毎

	ulong			Sihei_out_Tryo;									// 紙幣払出金額
	ulong			Sihei_out_cnt[SIHEI_SYU_CNT];					// 　　　　枚数（金種毎）
	ulong			Sihei_now_Tryo;									// 紙幣払出機現在保有金額
	ulong			Sihei_now_cnt[SIHEI_SYU_CNT];					// 　　　　　　　　　枚数（金種毎）

	ulong			Junkan_goukei_cnt;								// 循環コイン合計回数
	ulong			Siheih_goukei_cnt;								// 紙幣払出機合計回数
	ulong			Ckinko_goukei_cnt;								// コイン金庫合計回数
	ulong			Skinko_goukei_cnt;								// 紙幣金庫合計回数

	ulong			Turi_modosi_ryo;								// 釣銭払戻額

	ulong			Seisan_chusi_cnt;								// 精算中止＜回数＞
	ulong			Seisan_chusi_ryo;								// 　　　　＜金額＞

	ulong			Husei_out_Tcnt;									// 不正出庫回数
	ulong			Husei_out_Tryo;									// 不正出庫金額

	ulong			Kyousei_out_Tcnt;								// 強制出庫回数
	ulong			Kyousei_out_Tryo;								// 強制出庫金額

	ulong			Syuusei_seisan_Tcnt;							// 修正精算回数
	ulong			Syuusei_seisan_Tryo;							// 修正精算払戻金額

//「分類集計データ」3分類対応
	ulong			Bunrui1_cnt[3][BUNRUI_CNT];						// 分類集計１：４８区分の台数（「入庫・精算 時刻別集計」時：入庫台数）
																	// 　　　　　　　　　　　　　（「入庫・出庫 時刻別集計」時：入庫台数）
	ulong			Bunrui1_ryo[3][BUNRUI_CNT];						// 分類集計１：４８区分の料金（「入庫・精算 時刻別集計」時：精算台数）
																	// 　　　　　　　　　　　　　（「入庫・出庫 時刻別集計」時：出庫台数）
	ulong			Bunrui1_cnt1[3];								// 分類集計１：区分以上の台数
	ulong			Bunrui1_ryo1[3];								// 分類集計１：区分以上の料金
	ulong			Bunrui1_cnt2[3];								// 分類集計１：区分不明の台数（「入庫・精算 時刻別集計」時：入庫台数）
																	// 　　　　　　　　　　　　　（「入庫・出庫 時刻別集計」時：入庫台数）
	ulong			Bunrui1_ryo2[3];								// 分類集計１：区分不明の料金（「入庫・精算 時刻別集計」時：精算台数）
																	// 　　　　　　　　　　　　　（「入庫・出庫 時刻別集計」時：出庫台数）
	ulong			Electron_sei_cnt;								// Suica精算＜回数＞
	ulong			Electron_sei_ryo;								// 　　　　 ＜金額＞
	ulong			Electron_psm_cnt;								// PASMO精算＜回数＞
	ulong			Electron_psm_ryo;								// 　　　　 ＜金額＞
	ulong			Electron_edy_cnt;								// Edy精算  ＜回数＞
	ulong			Electron_edy_ryo;								// 　　　　 ＜金額＞
	ulong			Electron_Arm_cnt;								// Edyｱﾗｰﾑ取引  ＜回数＞
	ulong			Electron_Arm_ryo;								// 　　　　     ＜金額＞

	ulong			Gengaku_seisan_cnt;								// 減額精算回数
	ulong			Gengaku_seisan_ryo;								// 減額精算金額
	ulong			Furikae_seisan_cnt;								// 振替精算回数
	ulong			Furikae_seisan_ryo;								// 振替精算金額

	ulong			Electron_ico_cnt;								// ICOCA精算＜回数＞
	ulong			Electron_ico_ryo;								// 　　　　 ＜金額＞

	ulong			Electron_icd_cnt;								// IC-CARD精算＜回数＞
	ulong			Electron_icd_ryo;								// 　　　　 ＜金額＞
	
	
	ulong			Syuusei_seisan_Mcnt;							// 修正精算未入金回数
	ulong			Syuusei_seisan_Mryo;							// 修正精算未入金額
	ulong			Lag_extension_cnt;								// ラグタイム延長回数
	LOKTOTAL_DAT	loktldat[SYU_LOCK_MAX];							// 駐車位置別集計(駐車位置毎:先頭から３０車室分)
	ulong			CenterSeqNo;									// センター追番（集計）
	// note: 集計データから複数のNT-NETデータが作成されるため、必要なシーケンシャルNo.を予め取得しておく
	//		 FT-4000では以下のデータを作成する
	//			データ名				ntnet,遠隔Phase1/遠隔Phase2
	//			集計基本データ			ID:30/42
	//			料金種別毎集計データ	ID:31/43
	//			分類集計データ			ID:32/44
	//			割引集計データ			ID:33/45
	//			定期集計データ			ID:34/46
	//			車室毎集計データ		ID:35/--
	//			金銭集計データ			ID:36/--
	//			集計終了データ			ID:41/53
	uchar			SeqNo[8];										// シーケンシャルNo.
// MH322914(S) K.Onodera 2016/10/11 AI-V対応：振替精算
//	ulong			dummy[13];										// 予備エリア(dummyは構造体の最後に配置すること！)
	ulong			Mno_use_cnt5;									// 店Ｎｏ．毎割引＜回数＞（基本、拡張１）（店Ｎｏ．１～１００）
	ulong			Mno_use_ryo5;									// 　　　　　　　＜金額＞（基本、拡張１）（店Ｎｏ．１～１００）
	ulong			Furikae_seisan_cnt2;							// 振替精算回数(ParkingWeb版)
	ulong			Furikae_seisan_ryo2;							// 振替精算金額(ParkingWeb版)
	ulong			Remote_seisan_cnt;								// 遠隔精算回数(ParkingWeb版)
	ulong			Remote_seisan_ryo;								// 遠隔精算金額(ParkingWeb版)
	ulong			Furikae_Card_cnt[MOD_TYPE_MAX];					// 振替精算における現金以外の過払い回数
	ulong			Furikae_CardKabarai[MOD_TYPE_MAX];				// 振替精算における現金以外の過払い額
// MH321800(S) G.So ICクレジット対応
//※決済リーダが処理できる上限まで集計エリアを追加します。
	EC_SYU_DATA		Ec_syu[EC_BRAND_TOTAL_MAX];							// 決済リーダ
// MH321800(E) G.So ICクレジット対応
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は予備領域を使用すること!!!
	//	   メンバは最後尾に追加し、途中にメンバを挿入しないこと
// MH321800(S) G.So ICクレジット対応
// MH810100(S) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
	ulong			Kaimono_use_cnt[KAIMONO_WARISYU_MAX+1];			// 買物割引①～⑩ + 閾値情報がない買物割引	＜回数＞
	ulong			Kaimono_use_ryo[KAIMONO_WARISYU_MAX+1];			// 				 							＜金額＞
// MH810100(E) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
	ulong			Ec_minashi_cnt;									// みなし決済精算＜回数＞
	ulong			Ec_minashi_ryo;									// 　　　　 ＜金額＞
// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
// ※集計領域拡張の機会に予備領域を100件確保する。（計：8120Byte）
//	ulong			dummy[1];										// 予備エリア(dummyは構造体の最後に配置すること！)
// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
//// MH810100(S) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
////	ulong			dummy[100];										// 予備エリア(dummyは構造体の最後に配置すること！)
//	ulong			dummy[78];										// 予備エリア(dummyは構造体の最後に配置すること！)
//// MH810100(E) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
	ulong			dummy[76];										// 予備エリア(dummyは構造体の最後に配置すること！)
// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
// MH321800(E) G.So ICクレジット対応
// MH322914(E) K.Onodera 2016/10/11 AI-V対応：振替精算
} SYUKEI;

// MH810105 GG119202(S) 処理未了取引集計仕様改善
//// MH321800(S) G.So ICクレジット対応
//#define	nanaco_sei_cnt		Ec_syu[0].sei_cnt
//#define	nanaco_sei_ryo		Ec_syu[0].sei_ryo
//#define	nanaco_alm_cnt		Ec_syu[0].alm_cnt
//#define	nanaco_alm_ryo		Ec_syu[0].alm_ryo
//#define	waon_sei_cnt		Ec_syu[1].sei_cnt
//#define	waon_sei_ryo		Ec_syu[1].sei_ryo
//#define	waon_alm_cnt		Ec_syu[1].alm_cnt
//#define	waon_alm_ryo		Ec_syu[1].alm_ryo
//#define	sapica_sei_cnt		Ec_syu[2].sei_cnt
//#define	sapica_sei_ryo		Ec_syu[2].sei_ryo
//#define	sapica_alm_cnt		Ec_syu[2].alm_cnt
//#define	sapica_alm_ryo		Ec_syu[2].alm_ryo
//#define	koutsuu_sei_cnt		Ec_syu[3].sei_cnt
//#define	koutsuu_sei_ryo		Ec_syu[3].sei_ryo
//#define	koutsuu_alm_cnt		Ec_syu[3].alm_cnt
//#define	koutsuu_alm_ryo		Ec_syu[3].alm_ryo
//#define	id_sei_cnt			Ec_syu[4].sei_cnt
//#define	id_sei_ryo			Ec_syu[4].sei_ryo
//#define	id_alm_cnt			Ec_syu[4].alm_cnt
//#define	id_alm_ryo			Ec_syu[4].alm_ryo
//#define	quicpay_sei_cnt		Ec_syu[5].sei_cnt
//#define	quicpay_sei_ryo		Ec_syu[5].sei_ryo
//#define	quicpay_alm_cnt		Ec_syu[5].alm_cnt
//#define	quicpay_alm_ryo		Ec_syu[5].alm_ryo
//// MH321800(E) G.So ICクレジット対応
// ブランド毎の支払回数・金額
#define	nanaco_sei_cnt		Ec_syu[0].sei_cnt
#define	nanaco_sei_ryo		Ec_syu[0].sei_ryo
#define	waon_sei_cnt		Ec_syu[1].sei_cnt
#define	waon_sei_ryo		Ec_syu[1].sei_ryo
#define	sapica_sei_cnt		Ec_syu[2].sei_cnt
#define	sapica_sei_ryo		Ec_syu[2].sei_ryo
#define	koutsuu_sei_cnt		Ec_syu[3].sei_cnt
#define	koutsuu_sei_ryo		Ec_syu[3].sei_ryo
#define	id_sei_cnt			Ec_syu[4].sei_cnt
#define	id_sei_ryo			Ec_syu[4].sei_ryo
#define	quicpay_sei_cnt		Ec_syu[5].sei_cnt
#define	quicpay_sei_ryo		Ec_syu[5].sei_ryo
// MH810105(S) MH364301 QRコード決済対応
#define qr_sei_cnt			Ec_syu[6].sei_cnt
#define qr_sei_ryo			Ec_syu[6].sei_ryo
// MH810105(E) MH364301 QRコード決済対応
// MH810105(S) MH364301 PiTaPa対応
#define	pitapa_sei_cnt		Ec_syu[7].sei_cnt
#define	pitapa_sei_ryo		Ec_syu[7].sei_ryo
// MH810105(E) MH364301 PiTaPa対応

// 処理未了取引集計（Ec_syuの0,1,3～5のalm_cnt,alm_ryoは使用しないこと）
#define	miryo_pay_ok_cnt	Ec_syu[2].alm_cnt	// 支払済み回数
#define	miryo_pay_ok_ryo	Ec_syu[2].alm_ryo	// 支払済み金額
#define	miryo_unknown_cnt	Ec_syu[6].alm_cnt	// 支払不明回数
#define	miryo_unknown_ryo	Ec_syu[6].alm_ryo	// 支払不明金額
// MH810105 GG119202(E) 処理未了取引集計仕様改善

#define	_SYU_HDR_SIZ	((ulong)&((SYUKEI*)NULL)->Uri_Tryo)
#define	_SYU_DAT_CNT	((sizeof(SYUKEI)-_SYU_HDR_SIZ)/sizeof(ulong))
extern	SYUKEI		skyprn;											// 集計印字エリア
extern	SYUKEI		wksky;											// 集計待避・ﾜｰｸｴﾘｱ

struct	SKY{														// 集計ｴﾘｱ
	SYUKEI			tsyuk;											// T集計
	SYUKEI			gsyuk;											// GT集計
	SYUKEI			msyuk;											// MT集計
	SYUKEI			fsyuk;											// 複数台数集計
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は最後尾に追加すること!!!
	//	   途中にメンバを挿入しないこと
};

extern	struct		SKY		sky;
extern	struct		SKY		skybk;									// 集計ｴﾘｱ(前回分)

typedef struct {
	ulong			In_car_Tcnt;									// 総入庫台数
	ulong			In_car_cnt[IN_SIG_CNT];							// 入庫台数（入庫信号１、２）
	ulong			Out_car_Tcnt;									// 総出庫台数
	ulong			Out_car_cnt[IN_SIG_CNT];						// 出庫台数（入庫信号１、２）
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は別途処理を記述しないと
	//	   バックアップ/リストアが正常終了しません。
} CAR_COUNT;

extern	CAR_COUNT		CarCount;									// 入出庫台数ｶｳﾝﾄｴﾘｱ
extern	CAR_COUNT		CarCount_W;									// 入出庫台数ｶｳﾝﾄﾜｰｸｴﾘｱ

// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
// リアルタイム情報用の日付切替基準の総売上額、総現金売上額
typedef struct {
	ulong			Uri_Tryo;		// 総売上額
	ulong			Genuri_Tryo;	// 総現金売上額
	date_time_rec	Date_Time;		// 日付切替基準時刻（現在の小計基準日）
	ulong			reserve[8];		// 予備
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は別途処理を記述しないと
	//	   バックアップ/リストアが正常終了しません。
}	DATE_SYOUKEI;

extern	DATE_SYOUKEI	Date_Syoukei;								//日付切替基準の小計
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
#pragma unpack


/*--------------------------------------------------------------------------*/
/* 動作履歴格納ｴﾘｱ															*/
/*--------------------------------------------------------------------------*/
#define		ERR_LOG_CNT			10									// ｴﾗｰﾛｸﾞ件数
// MH322916(S) A.Iiizumi 2018/08/20 長期駐車検出機能対応
//#define		ARM_LOG_CNT			10									// ｱﾗｰﾑﾛｸﾞ件数
#define		ARM_LOG_CNT			80									// ｱﾗｰﾑﾛｸﾞ件数
// MH322916(E) A.Iiizumi 2018/08/20 長期駐車検出機能対応
#define		OPE_LOG_CNT			10									// 操作履歴件数
#define		MON_LOG_CNT			10									// モニタ履歴件数
#define		RMON_LOG_CNT		10									// 遠隔監視件数
#define		FLP_LOG_CNT			10									// 不正・強制履歴件数
#define		SYUUKEI_DAY			101									// Ｓｕｉｃａ・Ｅｄｙ集計印字日数の最大日数（＋ work用（当日クリア分））
// MH321800(S) G.So ICクレジット対応
#define		SYUUKEI_DAY_EC		65									// 決済リーダ集計印字日数の最大日数（＋ work用（当日クリア分））
// MH321800(E) G.So ICクレジット対応
#define		IO_EVE_CNT			20									// 入出庫事象件数
#define		IO_LOG_CNT			70									// 入出庫ログ現在情報の件数
#define		RT_PAY_LOG_CNT		100									// 料金シミュレータ精算履歴件数

/*--------------------------------------------------------------------------*/
/* 複電履歴																	*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	date_time_rec	Pdw_Date;										// 停電日時
	date_time_rec	Pon_Date;										// 復電日時
	unsigned char	Pon_nmi;										// NMIﾌﾗｸﾞ
	unsigned char	Pon_kind;										// 復電種類
} Pon_log;

extern	Pon_log		Pon_log_work;									// 停復電ログワークバッファ

/*--------------------------------------------------------------------------*/
/* ｴﾗｰ履歴																	*/
/*--------------------------------------------------------------------------*/
#define	ERR_LOG_ASC_DAT_SIZE	36									// 35文字（ﾌﾟﾘﾝﾀ１行）+ 1
#define	ERR_LOG_DOOR_STS_F		0x01								// ドア開ビット
#define	ERR_LOG_RESEND_F		0x02								// ドア閉再送ビット

typedef	struct {
	date_time_rec	Date_Time;										// 発生日時
	uchar			Errsyu;											// ｴﾗｰ種別
	uchar			Errcod;											// ｴﾗｰｺｰﾄﾞ
	uchar			Errdtc;											// ｴﾗｰ発生/解除
	uchar			Errlev;											// ｴﾗｰﾚﾍﾞﾙ
	uchar			ErrDoor;										// b0:0=ドア閉,1=開／b1:再送 0=でない,1=である
	uchar			Errinf;											// ｴﾗｰ情報有無(0:なし、1:acsii、2:bin)
	ulong			ErrBinDat;										// ｴﾗｰ情報(binｺｰﾄﾞ)
	uchar			Errdat[ERR_LOG_ASC_DAT_SIZE];					// ｴﾗｰ情報(acsiiｺｰﾄﾞ)
	uchar			ErrSeqNo;										// シーケンシャルNo.
} Err_log;

struct	Err_log_rec {
	short			Err_wtp;										// エラーログ印字ライトポインタ
	Err_log			Err_log_dat[ERR_LOG_CNT];						// エラーログ印字バッファ
};
extern	struct	Err_log_rec		ERR_LOG_DAT;
extern	Err_log		Err_work;										// エラーログワークバッファ

/*--------------------------------------------------------------------------*/
/* ｱﾗｰﾑ履歴																	*/
/*--------------------------------------------------------------------------*/
#define	ALM_LOG_ASC_DAT_SIZE	36									// 35文字（ﾌﾟﾘﾝﾀ１行）+ 1

typedef	struct {
	date_time_rec	Date_Time;										// 発生日時
	uchar			Armsyu;											// ｱﾗｰﾑ種別
	uchar			Armcod;											// ｱﾗｰﾑｺｰﾄﾞ
	uchar			Armdtc;											// ｱﾗｰﾑ発生/解除
	uchar			Armlev;											// ｱﾗｰﾑﾚﾍﾞﾙ
	uchar			ArmDoor;										// b0:0=ドア閉,1=開／b1:再送 0=でない,1=である
	uchar			Arminf;											// ｱﾗｰﾑ情報有無
	ulong			ArmBinDat;										// ｱﾗｰﾑ情報(binｺｰﾄﾞ)
	uchar			Armdat[ALM_LOG_ASC_DAT_SIZE];					// ｱﾗｰﾑ情報(acsiiｺｰﾄﾞ)
	uchar			ArmSeqNo;										// シーケンシャルNo.
} Arm_log;

struct	Arm_log_rec {
	short			Arm_wtp;										// アラームログ印字ライトポインタ
	Arm_log			Arm_log_dat[ARM_LOG_CNT];						// アラームログ印字バッファ
};
extern	struct	Arm_log_rec		ARM_LOG_DAT;
extern	Arm_log	Arm_work;											// アラームログワークバッファ


/*--------------------------------------------------------------------------*/
/* 操作履歴																	*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	date_time_rec	Date_Time;										// 発生日時
	uchar			OpeKind;										// 操作種別
	uchar			OpeCode;										// 操作コード
	uchar			OpeLevel;										// 操作レベル
	ulong			OpeBefore;										// 変更前データ
	ulong			OpeAfter;										// 変更後データ
	uchar			OpeSeqNo;										// シーケンシャルNo.
} Ope_log;

struct	Ope_log_rec {
	short			Ope_count;										// 操作履歴件数
	short			Ope_wtp;										// 操作履歴ﾗｲﾄﾎﾟｲﾝﾀ
	Ope_log			Ope_work;										// 操作履歴ﾜｰｸﾊﾞｯﾌｧ
	Ope_log			Ope_log_dat[OPE_LOG_CNT];						// 操作履歴ﾊﾞｯﾌｧ
};

extern	struct	Ope_log_rec		OPE_LOG_DAT;
// MH321800(S) T.Nagai ICクレジット対応
//extern	Ope_log 		Ope_work;
// MH321800(E) T.Nagai ICクレジット対応

/*--------------------------------------------------------------------------*/
/* モニタ履歴																*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	date_time_rec	Date_Time;										// 発生日時
	uchar			MonKind;										// モニタ種別
	uchar			MonCode;										// モニタコード
	uchar			Rsv;											// 予約
	uchar			MonLevel;										// モニタレベル
	uchar			MonInfo[10];									// モニタ情報
	uchar			MonSeqNo;										// シーケンシャルNo.
} Mon_log;

struct	Mon_log_rec {
	short			Mon_count;										// モニタ履歴件数
	short			Mon_wtp;										// モニタ履歴ﾗｲﾄﾎﾟｲﾝﾀ
	Mon_log			Mon_work;										// モニタ履歴ﾜｰｸﾊﾞｯﾌｧ
	Mon_log			Mon_log_dat[MON_LOG_CNT];						// モニタ履歴ﾊﾞｯﾌｧ
};

extern	struct	Mon_log_rec		MON_LOG_DAT;
// MH321800(S) T.Nagai ICクレジット対応
//extern	Mon_log			Mon_work;
// MH321800(E) T.Nagai ICクレジット対応

/*--------------------------------------------------------------------------*/
/* 不正強制履歴																*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	date_time_rec	Date_Time;										// 発生日時
	date_time_rec	In_Time;										// 入庫日時
	ulong			WPlace;											// 駐車位置ﾃﾞｰﾀ
	uchar			Lok_inf;										// 動作情報 0:強制出庫
																	//          1:不正出庫
																	//          2:不正出庫(修正精算) GM333200
	uchar			ryo_syu;										// 料金種別
	long			fusei_fee;										// 不正出庫駐車料金
	short			dumy;											// not used
	ushort			Lok_No;											// ロック装置No
	ST_OIBAN		count;											// 追い番
	ushort			Kikai_no;										// 機械Ｎｏ
	uchar			Seisan_kind;									// 精算種別
}	flp_log;

struct	Flp_log_rec {
	short			Flp_wtp;										// 不正履歴ﾗｲﾄﾎﾟｲﾝﾀ
	flp_log			Flp_log_dat[FLP_LOG_CNT];						// 不正履歴ﾊﾞｯﾌｧ
};

extern	struct	Flp_log_rec		FLP_LOG_DAT;

/*--------------------------------------------------------------------------*/
/* 遠隔監視																	*/
/*--------------------------------------------------------------------------*/
typedef union {
	ulonglong		ullinstNo;
	ulong			ulinstNo[2];
} u_inst_no;

typedef	struct {
	date_time_rec2	Date_Time;										// 発生日時
	u_inst_no		RmonInstNo;										// 命令番号
	ushort			RmonFuncNo;										// 端末機能番号
	ushort			RmonProcNo;										// 処理番号
	ushort			RmonProcInfoNo;									// 処理情報番号
	ushort			RmonStatusDetailNo;								// 状況詳細番号
	ushort			RmonInfo;										// 端末監視情報
	uchar			RmonStatusNo;									// 状況番号
	uchar			RmonSeqNo;										// シーケンシャルNo.
	date_time_rec	RmonStartTime1;									// 予定開始時刻1
	date_time_rec	RmonStartTime2;									// 予定開始時刻2
	uchar			RmonProgNo[12];									// プログラム部番・バージョン
// GG120600(S) // Phase9 遠隔監視データ変更
	uchar			RmonFrom;										// 命令要求元（0＝センター/1=端末）
	uchar			RmonUploadReq;									// 設定アップロード要求
// GG120600(E) // Phase9 遠隔監視データ変更
} Rmon_log;

struct	Rmon_log_rec {
	short			Rmon_count;										// 遠隔監視件数
	short			Rmon_wtp;										// 遠隔監視ﾗｲﾄﾎﾟｲﾝﾀ
	Rmon_log		Rmon_work;										// 遠隔監視ﾜｰｸﾊﾞｯﾌｧ
	Rmon_log		Rmon_log_dat[RMON_LOG_CNT];						// 遠隔監視ﾊﾞｯﾌｧ
};

extern	struct	Rmon_log_rec	RMON_LOG_DAT;

#pragma pack
/*--------------------------------------------------------------------------*/
/* 精算履歴(個別精算情報)													*/
/*--------------------------------------------------------------------------*/
// 定期券ﾃﾞｰﾀﾌｫｰﾏｯﾄ
typedef struct {
	uchar			syu;											// 定期券種別
	uchar			status;											// 定期券ｽﾃｰﾀｽ(読取時)
	ulong			id;												// 定期券id
	uchar			pkno_syu;										// 定期券駐車場Ｎｏ．種別
	uchar			update_mon;										// 更新月数
	ushort			s_year;											// 有効期限（開始：年）
	uchar			s_mon;											// 有効期限（開始：月）
	uchar			s_day;											// 有効期限（開始：日）
	ushort			e_year;											// 有効期限（終了：年）
	uchar			e_mon;											// 有効期限（終了：月）
	uchar			e_day;											// 有効期限（終了：日）
	uchar			t_tim[4];										// 処理月日時分
	uchar			update_rslt1;									// 定期更新精算時の更新結果			（　OK：更新成功　／　NG：更新失敗　）
	uchar			update_rslt2;									// 定期更新精算時のラベル発行結果	（　OK：正常発行　／　NG：発行不良　）
	uchar			Apass_off_seisan;								// 1=強制ｱﾝﾁﾊﾟｽOFF設定よりﾁｪｯｸしなかった
	ulong			ParkingNo;										// 駐車場Ｎｏ．
} teiki_use;

// ｻｰﾋﾞｽ券,掛売券,割引券,回数券ﾃﾞｰﾀﾌｫｰﾏｯﾄ
typedef struct {
	uchar			tik_syu;										// 割引券種別
																	// 0x01=ｻｰﾋﾞｽ券
																	// 0x02=掛売券
																	// 0x03=回数券
																	// 0x04=割引券
																	// 0x10=未収金（減額精算時に使用）
																	// 0x11=振替額（振替精算時に使用）
	uchar			pkno_syu;										// 駐車場Ｎｏ．種別
// MH322914(S) K.Onodera 2017/03/08 AI-V対応：遠隔精算割引
//	uchar			syubetu;										// 種別
	ushort			syubetu;										// 種別
// MH322914(E) K.Onodera 2017/03/08 AI-V対応：遠隔精算割引
	uchar			maisuu;											// 枚数
	ushort			mise_no;										// 店Ｎｏ．（割引券種別＝割引券の場合、券ﾃﾞｰﾀの店Ｎｏ．を格納）
	ulong			ryokin;											// 割引料金
	ulong			minute;											// 割引時間(UT8500未使用)
} wari_tiket;

// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
typedef struct {
	ulong			ppc_id;											// ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞＮｏ．
	ulong			pay_befor;										// 支払前残額
	ulong			pay_after;										// 支払後残額
	ushort			kigen_year;										// 有効期限年
	uchar			kigen_mon;										// 有効期限月
	uchar			kigen_day;										// 有効期限日
} ppc_tiket;

// ｸﾚｼﾞｯﾄｶｰﾄﾞﾃﾞｰﾀﾌｫｰﾏｯﾄ
typedef struct {
// MH321800(S) G.So ICクレジット対応(クレジットカード会社名（Shift JIS）24桁対応)
//	char			card_name[12];									// ｸﾚｼﾞｯﾄｶｰﾄﾞ会社略号
	char			card_name[24];									// ｸﾚｼﾞｯﾄｶｰﾄﾞ会社略号
// MH321800(E) G.So ICクレジット対応(クレジットカード会社名（Shift JIS）24桁対応)
	char			card_no[20];									// ｸﾚｼﾞｯﾄｶｰﾄﾞ会員番号
	ulong			slip_no;										// ｸﾚｼﾞｯﾄｶｰﾄﾞ伝票番号
	ulong			app_no;											// ｸﾚｼﾞｯﾄｶｰﾄﾞ承認番号
	ulong			pay_ryo;										// ｸﾚｼﾞｯﾄｶｰﾄﾞ利用金額
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//	date_time_rec2	CenterProcTime;									// ｸﾚｼﾞｯﾄｶｰﾄﾞｾﾝﾀｰ処理日時（秒あり）
//	ulong			CenterProcOiBan;								// ｸﾚｼﾞｯﾄｶｰﾄﾞｾﾝﾀｰ処理追番
//	char			CCT_Num[16];									// ｸﾚｼﾞｯﾄｶｰﾄﾞ端末識別番号
	char			CCT_Num[13];									// ｸﾚｼﾞｯﾄｶｰﾄﾞ端末識別番号
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
	uchar			cre_type;										// 0=ｸﾚｼﾞｯﾄ精算 1=iD精算
																	// 2=法人ｶｰﾄﾞ
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//	uchar			CreditDate[2];									// ｸﾚｼﾞｯﾄｶｰﾄﾞ有効期限 [0]=年 [1]=月
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
	uchar			kid_code[6];									// KIDｺｰﾄﾞ
	char			ShopAccountNo[20];								// ｸﾚｼﾞｯﾄｶｰﾄﾞ加盟店取引番号
	char			AppNoChar[6];									// 承認番号(英数字)
// MH321800(S) D.Inaba ICクレジット対応
	uchar			Identity_Ptrn;									// 本人確認パターン
// MH810105(S) MH364301 インボイス対応
//	uchar			dummy1;
// MH810105(E) MH364301 インボイス対応
// MH321800(E) D.Inaba ICクレジット対応
} credit_use;

enum {
	CREDIT_CARD = 0,
	CREDIT_HOJIN,
	_POSTPAY_MAX
};


#define	SUICA_USED	1
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#define	EDY_USED	2
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#define	PASMO_USED	3
#define	ICOCA_USED	4
#define	ICCARD_USED	5
// MH321800(S) G.So ICクレジット対応
enum {
// 以下決済リーダ
	EC_USED = 11,				// 決済リーダ使用
	EC_UNKNOWN_USED = EC_USED,	// [11]不明ブランド
	EC_ZERO_USED,				// [12]ブランド番号0
	EC_EDY_USED,				// [13]Edy
	EC_NANACO_USED,				// [14]nanaco
	EC_WAON_USED,				// [15]WAON
	EC_SAPICA_USED,				// [16]SAPICA
	EC_KOUTSUU_USED,			// [17]交通系ICカード
	EC_ID_USED,					// [18]iD
	EC_QUIC_PAY_USED,			// [19]QuicPay
	EC_CREDIT_USED,				// [20]クレジット
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
	EC_TCARD_USED,				// [21]Tカード
	EC_HOUJIN_USED,				// [22]法人
	EC_HOUSE_USED,				// [23]ハウスカード
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810105(S) MH364301 QRコード決済対応
	EC_QR_USED,					// [24]QRコード決済
// MH810105(E) MH364301 QRコード決済対応
// MH810105(S) MH364301 PiTaPa対応
	EC_PITAPA_USED,				// [25]PiTaPa
// MH810105(E) MH364301 PiTaPa対応
};
// MH321800(E) G.So ICクレジット対応

// MH810105(S) MH364301 QRコード決済対応
// QRブランドの支払種別
enum {
	QR_DUMMY,		// [00]
	ALIPAY,			// [01]
	DPAY,			// [02]
	LINEPAY,		// [03]
	PAYPAY,			// [04]
	RAKUTENPAY,		// [05]
	MICROPAY,		// [06]
	UPICPM,			// [07]
	AUPAY,			// [08]
	MERPAY,			// [09]
	ORIGAMIPAY,		// [10]
	GINKOPAY,		// [11]
	QUOPAY,			// [12]
	SGTDASH,		// [13]
	SGTGLB,			// [14]
	SMARTCODE,		// [15]
	JCOINPAY,		// [16]
	AMAZONPAY,		// [17]
	JKOPAY,			// [18]
	GLNPAY,			// [19]
	BANKPAY,		// [20]
	FAMIPAY,		// [21]
	ATONEPAY,		// [22]
	QR_PAY_KIND_MAX
};
// MH810105(E) MH364301 QRコード決済対応

typedef union{
	uchar	e_status;
	struct {
// MH321800(S) Y.Tanizaki ICクレジット対応
//		uchar YOBI				:7;		/* B1-7 = 予備 */				
// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
//		uchar YOBI				:4;		/* B4-7 = 予備 */
// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
//		uchar YOBI				:3;		/* B5-7 = 予備 */
// MH810105 GG119202(S) GT4710FX対応(決済リーダー対応/未了残高照会完了を記録する)(MH615702流用)
//		uchar YOBI				:2;		/* B6-7 = 予備 */
		uchar reserve			:1;		/* B7 : 予約(GT4710FXで使用)	*/
		uchar YOBI				:1;		/* B6 = 予備 */
// MH810105 GG119202(E) GT4710FX対応(決済リーダー対応/未了残高照会完了を記録する)(MH615702流用)
		uchar deemed_sett_fin	:1;		/* B5 = みなし決済による精算完了*/
// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
		uchar miryo_confirm		:1;		/* B4 = 未了確定 */
// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
		uchar brand_mismatch	:1;		/* B3 = ブランド不一致 */
		uchar deemed_receive	:1;		/* B2 = 決済精算中止(復決済) */			
		uchar deemed_settlement	:1;		/* B1 = みなし決済 */
// MH321800(E) Y.Tanizaki ICクレジット対応
		uchar pay_cansel_work	:1;		/* B0 = 電子マネー入金済みﾌﾗｸﾞ保持領域(印字用) */
	} BIT;
}t_E_STATUS;

// MH810105(S) MH364301 QRコード決済対応
typedef union{
	uchar	e_flag;
	struct {
		uchar YOBI				:6;		/* B1-6 = 予備 */
		uchar minashi_tim_pri	:1;		/* B4 = みなし集計時に発生時刻印字ﾌﾗｸﾞ */
		uchar deemSettleCancal	:1;		/* B0 = 未了支払不明（決済結果 1= 未受信 0= 受信）ﾌﾗｸﾞ */
	} BIT;
}t_E_FLAG;
// MH810105(E) MH364301 QRコード決済対応

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//typedef struct {
//	uchar	e_pay_kind;				// 電子決済種別 Edy:2
//	t_E_STATUS E_Status;			// 電子決済用各種ｽﾃｰﾀｽ
//	ulong	pay_ryo;				// 決済額
//	ulong	pay_befor;				// 決算前Suica残高
//	ulong	pay_after;				// 決済後Suica残高
//	uchar	Card_ID[8];				// ｶｰﾄﾞID (BCD 16桁)
//	ulong	deal_no;				// 取引通番(Hex)
//	ushort	card_deal_no;			// ｶｰﾄﾞ取引通番(Hex)
//	uchar	Terminal_ID[4];			// 上位端末ID (BCD 8桁)
//	ushort	IDm;					// Felica card ID
//} Edy_info;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

typedef struct {
	uchar	e_pay_kind;				// 電子決済種別 Suica:1
	t_E_STATUS E_Status;			// 電子決済用各種ｽﾃｰﾀｽ
	ulong	pay_ryo;				// 決済額
	ulong	pay_befor;				// 決算前Suica残高
	ulong	pay_after;				// 決済後Suica残高
	uchar	Card_ID[16];			// ｶｰﾄﾞID (Ascii 16桁)
	uchar	dmy2[4];				// 空き
} Suica_info;

// MH321800(S) G.So ICクレジット対応
// 取引データ
// MH810103 GG119202(S) 交通系ブランドの決済結果データフォーマット修正
typedef struct {
// 交通系IC
	uchar	SPRW_ID[13];			// SPRWID
	uchar	dmy1;
	uchar	Kamei[30];				// 加盟店名
	uchar	TradeKind;				// 取引種別
	uchar	dmy2;
} Ec_info_Koutsuu;
// MH810103 GG119202(E) 交通系ブランドの決済結果データフォーマット修正

typedef struct {
// Edy
// MH810103 MHUT40XX(S) Edy・WAON対応
//	ulong	DealNo;					// 取引通番
//	ulong	CardDealNo;				// カード取引通番
//	ulong	TerminalNo[2];			// 端末番号
	uchar	DealNo[10];				// Edy取引通番
	uchar	CardDealNo[5];			// カード取引通番
	uchar	dmy;
	uchar	TerminalNo[8];			// 上位端末ID
	uchar	Kamei[30];				// 加盟店名
// MH810103 MHUT40XX(E) Edy・WAON対応
} Ec_info_Edy;

typedef struct {
// nanaco
// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//	ulong	TerminalNo[3];			// 上位端末ID:10進 20桁
//	ulong	DealNo;					// 上位取引通番
	uchar	Kamei[30];				// 加盟店名
	uchar	DealNo[6];				// 端末取引通番
	uchar	TerminalNo[20];			// 上位端末ID
// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
} Ec_info_Nanaco;

typedef struct {
// WAON
	ulong	point;					// 今回ポイント
// MH810103 MHUT40XX(S) Edy・WAON対応
//	ulong	point_total;			// 累計ポイント
//	long	card_type;				// カード分類
//	ulong	period_point;			// ２年前までに獲得したポイント
//	ulong	period;					// ２年前までに獲得したポイントの有効期限
//	uchar	SPRW_ID[14];			// SPRWID(実際はAscii 13桁)
//	uchar	point_status;			// 利用可否
//	uchar	deal_code;				// 取引種別コード
	ulong	point_total[2];			// 累計ポイント
	uchar	SPRW_ID[13];			// SPRWID
	uchar	point_status;			// 累計ポイントメッセージ
									// 0=「累計WAONポイント数は、お近くのWAONステーションで確認ください。」
									// 1=「このWAONカードはポイント対象外です。」
									// 2=「JMB WAONはマイルがつきます。」
	uchar	Kamei[30];				// 加盟店名
// MH810103 MHUT40XX(E) Edy・WAON対応
} Ec_info_Waon;

typedef struct {
// SAPICA
	uchar	Terminal_ID[8];			// 端末ID (実際はAscii  7桁)
	ulong	Details_ID;				// 明細ID
} Ec_info_Sapica;

typedef struct {
// iD
// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//	uchar	Terminal_ID[8];			// 端末ID (実際はAscii  7桁)
//	ulong	Details_ID;				// 明細ID
	uchar	Kamei[30];				// 加盟店名
	uchar	Approval_No[7];			// 承認番号
	uchar	dmy1;
	uchar	TerminalNo[13];			// 上位端末ID
	uchar	dmy2;
// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
} Ec_info_Id;

typedef struct {
// QUICPay
// MH810103 MHUT40XX(S) nanaco・iD・QUICPay対応
//	uchar	Terminal_ID[8];			// 端末ID (実際はAscii  7桁)
//	ulong	Details_ID;				// 明細ID
	uchar	Kamei[30];				// 加盟店名
	uchar	Approval_No[7];			// 承認番号
	uchar	dmy1;
	uchar	TerminalNo[13];			// 上位端末ID
	uchar	dmy2;
// MH810103 MHUT40XX(E) nanaco・iD・QUICPay対応
} Ec_info_Quickpay;

// MH810105(S) MH364301 QRコード決済対応
typedef struct {
// QR
	uchar	PayKind;				// 支払種別
	uchar	dmy1;
	uchar	PayTerminalNo[20];		// 支払端末ID
	uchar	MchTradeNo[32];			// Mch取引番号
} Ec_info_Qr;
// MH810105(E) MH364301 QRコード決済対応

// MH810105(S) MH364301 PiTaPa対応
typedef	struct {
// PiTaPa
	uchar	Kamei[30];				// 加盟店名
	uchar	Approval_No[8];			// 承認番号
	uchar	TerminalNo[13];			// 上位端末ID
	uchar	Slip_No[5];				// 伝票番号
} Ec_info_Pitapa;
// MH810105(E) MH364301 PiTaPa対応

typedef struct {
	uchar	e_pay_kind;				// 電子決済種別 Sanden:11～
	t_E_STATUS E_Status;			// 電子決済用各種ｽﾃｰﾀｽ
	ulong	pay_ryo;				// 決済額
	ulong	pay_befor;				// 決算前残高
	ulong	pay_after;				// 決済後残高
	uchar	Card_ID[20];			// ｶｰﾄﾞID (実際はAscii 16or17桁)	※Card_IDはwk_DicData.CardNoのサイズに合わせる
// MH810103 GG119202(S) 電子マネー対応
//	uchar	inquiry_num[16];		// 問合せ番号
	uchar	inquiry_num[15];		// 問合せ番号
// MH810105(S) MH364301 QRコード決済対応
//	uchar	dummy;					// 問い合わせ番号と合わせて16byte
	t_E_FLAG	E_Flag;				// ﾌﾗｸﾞ管理(uchar)
// MH810105(E) MH364301 QRコード決済対応
// MH810103 GG119202(E) 電子マネー対応
	ulong	pay_datetime;			// 決済日時(Normalize)
// MH810105(S) MH364301 QRコード決済対応
//// MH810103 GG119202(S) 交通系ブランドの決済結果データフォーマット修正
//	uchar	Termserial_No[30];		// 端末識別番号(30桁)
//// MH810103 GG119202(E) 交通系ブランドの決済結果データフォーマット修正
	uchar	QR_Kamei[30];			// 加盟店名（QRコード決済）
// MH810105(E) MH364301 QRコード決済対応

	union brand {					// ブランド別取引データ
// MH810103 GG119202(S) 交通系ブランドの決済結果データフォーマット修正
		Ec_info_Koutsuu		Koutsuu;// 交通系IC
// MH810103 GG119202(E) 交通系ブランドの決済結果データフォーマット修正
		Ec_info_Edy			Edy;	// Edy
		Ec_info_Nanaco		Nanaco;	// nanaco
		Ec_info_Waon		Waon;	// WAON
		Ec_info_Sapica		Sapica;	// SAPICA
		Ec_info_Id			Id;		// iD
		Ec_info_Quickpay	Quickpay;	// QUICPay
// MH810105(S) MH364301 QRコード決済対応
		Ec_info_Qr			Qr;		// QR
// MH810105(E) MH364301 QRコード決済対応
// MH810105(S) MH364301 PiTaPa対応
		Ec_info_Pitapa		Pitapa;	// PiTaPa
// MH810105(E) MH364301 PiTaPa対応
	} Brand;
} Ec_info;
// MH321800(E) G.So ICクレジット対応

typedef union {
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	Edy_info	Edy;
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
	Suica_info	Suica;
// MH321800(S) G.So ICクレジット対応
	Ec_info		Ec;
// MH321800(E) G.So ICクレジット対応
} Electron_info;


typedef union{
	uchar	lw_status;
	struct {
		uchar YOBI		:2;		// B6,7	予備
		uchar PWARI		:2;		// B4,5	前回％割引
		uchar RWARI		:2;		// B2,3	前回料金割引
		uchar TWARI		:2;		// B0,1	前回時間割引
	} BIT;
}t_LW_STATUS;
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//typedef union{
//	unsigned char	BYTE;					// Byte
//	struct{
//		unsigned char	YOBI:6;				// 予備
//		unsigned char	T_Cancel:1;			// 1:Ｔカードキャンセル発生
//		unsigned char	T_Rate:1;			// 1:特別期間レート適用
//	} BIT;
//} Card_Receipt;
//typedef struct {
//	uchar			T_card_num[16];
//	ulong			T_point;
//	Card_Receipt	T_status;
//} T_CARD;
//
//#define T_POINT_MASK( a ) (a&0x7fffffff)
//#define T_CARD_USE( a ) (a&0x80000000)
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
typedef	struct {
	uchar			SystemID;										// ｼｽﾃﾑID
	uchar			DataKind;										// ﾃﾞｰﾀ種別
	uchar			DataKeep;										// ﾃﾞｰﾀ保持ﾌﾗｸﾞ
	uchar			SeqNo;											// ｼｰｹﾝｼｬﾙ№
	ulong			ParkingNo;										// 駐車場№
	ushort			ModelCode;										// 機種ｺｰﾄﾞ
	ulong			MachineNo;										// 機械№
	union {
		uchar		SerialNo[6];									// 端末ｼﾘｱﾙ№
		struct {
			ushort	encryptMode;									// 暗号化方式(0=暗号なし 1=AES方式)
			ushort	encryptKey;										// 暗号鍵番号
			uchar	Reserve[2];										// 予備
		} DT_BASIC;
	} CMN_DT;
	uchar			Year;											// 年
	uchar			Mon;											// 月
	uchar			Day;											// 日
	uchar			Hour;											// 時
	uchar			Min;											// 分
	uchar			Sec;											// 秒
} DATA_BASIC;

// GG124100(S) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// typedef	struct {
// 	ulong			ParkingNo;										// 駐車場№
// 	ushort			DiscSyu;										// 割引種別
// 	ushort			DiscNo;											// 割引区分
// 	ushort			DiscCount;										// 枚数
// 	ulong			Discount;										// 割引額(割引時間)
// 	ulong			DiscInfo1;										// 割引情報1
// 	union {
// 		struct {													// プリペイドカード以外の構造体
// 			ulong			DiscInfo2;								// 割引情報2
// 			ushort			MoveMode;								// 回収／返却
// 			ushort			DiscFlg;								// 割引済み
// 		} common;
// 		struct {													// プリペイドカード用構造体
// 			ulong			pay_befor;								// 支払前残額
// 			ushort			kigen_year;								// 有効期限年
// 			uchar			kigen_mon;								// 有効期限月
// 			uchar			kigen_day;								// 有効期限日
// 		} ppc_data;													// NT-NET精算データ送信前にクリアする
// 	} uDiscData;
// } DISCOUNT_DATA;
typedef	struct {
	ulong			ParkingNo;				// 駐車場№
	ushort			DiscSyu;				// 割引種別
	ushort			DiscNo;					// 割引区分
	ulong			Discount;				// 今回使用した割引金額
	ulong			DiscInfo1;				// 割引情報1
	union {
		struct {							// プリペイドカード以外の構造体
			ulong	DiscInfo2;				// 割引情報2
			ulong	PrevDiscount;			// 今回使用した前回精算までの割引金額
			ulong	PrevUsageDiscount;		// 前回精算までの使用済み割引金額
			uchar	PrevUsageDiscCount;		// 前回精算までの使用済み枚数
// GG124100(S) R.Endo 2022/08/19 車番チケットレス3.0 #6520 使用したサービス券の情報が領収証に印字されない
			uchar	FeeKindSwitchSetting;	// 種別切換先車種
			uchar	DiscRole;				// 割引役割
// GG124100(E) R.Endo 2022/08/19 車番チケットレス3.0 #6520 使用したサービス券の情報が領収証に印字されない
		} common;
		struct {							// プリペイドカード用構造体
			ulong	pay_befor;				// 支払前残額
			ushort	kigen_year;				// 有効期限年
			uchar	kigen_mon;				// 有効期限月
			uchar	kigen_day;				// 有効期限日
// GG124100(S) R.Endo 2022/08/19 車番チケットレス3.0 #6520 使用したサービス券の情報が領収証に印字されない
// 			uchar	Reserve[5];				// 予備
			uchar	Reserve[7];				// 予備
// GG124100(E) R.Endo 2022/08/19 車番チケットレス3.0 #6520 使用したサービス券の情報が領収証に印字されない
		} ppc_data;							// NT-NET精算データ送信前にクリアする
	} uDiscData;
	uchar			DiscCount;				// 今回使用した枚数
} DISCOUNT_DATA;
// GG124100(E) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-7700:GM747904参考）
typedef	struct {
	ulong			ParkingNo;										// 駐車場№（基本駐車場Ｎｏ固定）
	ushort			DiscSyu;										// 割引種別（２００：投入金、２０１：払出金）
	ushort			InOut10000;										// １００００円投入or払出枚数
	ushort			InOut5000;										// ５０００円投入or払出金額
	ushort			InOut2000;										// ２０００円投入or払出金額
	ushort			InOut1000;										// １０００円投入or払出金額
	ushort			InOut500;										// 　５００円投入or払出金額
	ushort			InOut100;										// 　１００円投入or払出金額
	ushort			InOut50;										// 　　５０円投入or払出金額
	ushort			InOut10;										// 　　１０円投入or払出金額
	ushort			MoveMode;										// 回収／返却　（現状はダミー）
	ushort			DiscFlg;										// 割引済み　（現状はダミー）
} IN_OUT_MONEY;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（GT-7700:GM747904参考）

// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
// 割引詳細(明細情報)
typedef struct{
	ulong			ParkingNo;										// 駐車場№
	ushort			DiscSyu;										// 種別
	union {
		struct{
			ushort	DiscNo;											// 割引区分
			ushort	DiscCount;										// 枚数
			ulong	Discount;										// 割引額(割引時間)
			ulong	DiscInfo1;										// 割引情報1
			ulong	DiscInfo2;										// 割引情報2
		} Common;
		struct{
			ulong	Pos;											// 区画情報
			ulong	OrgFee;											// 振替元駐車料金
			ulong	Total;											// 振替額
			ulong	Oiban;											// 振替元精算追番
		} Furikae;			// 2031(振替精算)
		struct{
			ushort	Mod;											// 媒体種別(設定可能な値は精算データのカード決済区分と同じ)
			ushort	Reserve1;										// 未使用
			ulong	FrkMoney;										// 振替額(現金)
			ulong	FrkCard;										// 振替額(現金以外)
			ulong	FrkDiscount;									// 振替額(割引)
		} FurikaeDetail;	// 2032(振替精算内訳)
		struct{
			ushort	Reserve1;										// 未使用
			ushort	Reserve2;										// 未使用
			ulong	Total;											// 支払予定額
			ulong	Reserve3;										// 未使用
			ulong	Reserve4;										// 未使用
		} Future;			// 2033(後日精算予定額)
		struct{
			ushort	Reserve1;										// 未使用
			ushort	Reserve2;										// 未使用
			ulong	Total;											// 支払予定額
			ulong	Reserve3;										// 未使用
			ulong	Reserve4;										// 未使用
		} AfterPay;			// 2034(後日精算額(未払い回収額))
	} uDetail;
} DETAIL_DATA;
// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応
// 不具合修正(S) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
typedef struct{	
	uchar		CarCntYear;											// 年
	uchar		CarCntMon;											// 月
	uchar		CarCntDay;											// 日
	uchar		CarCntHour;											// 時
	uchar		CarCntMin;											// 分
	ushort		CarCntSeq;											// 追番
	uchar		Reserve1;											// 予備
} CAR_CNT_INFO;	// 台数管理追番
// 不具合修正(E) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
// GG129000(S) H.Fujinaga 2023/02/18 ゲート式車番チケットレスシステム対応（QR駐車券対応）
typedef struct{	
	uchar		EntryMachineNo[3];									// 発券機番号
	uchar		ParkingTicketNo[6];									// 駐車券番号
	uchar		TicketLessMode[1];									// 車番チケットレス連動有無
} QRTICKET_INFO;	// QR駐車券情報
// GG129000(E) H.Fujinaga 2023/02/18 ゲート式車番チケットレスシステム対応（QR駐車券対応）

// 1精算情報ﾃﾞｰﾀﾌｫｰﾏｯﾄ
typedef struct {
	uchar			WFlag;											// 印字種別１（0:通常 1:復電）
	uchar			chuusi;											// 印字種別２（0:領収証 1:精算中止 2:未使用 3:修正精算領収証）
	ulong			WPlace;											// 駐車位置ﾃﾞｰﾀ
	ushort			Kikai_no;										// 機械Ｎｏ
	uchar			Seisan_kind;									// 精算種別
	ST_OIBAN		Oiban;											// 追番
	date_time_rec	TInTime;										// 入庫日時
	date_time_rec	TOutTime;										// 出庫日時（預り証の場合、発行時刻）
	ulong			WPrice;											// 駐車料金（定期更新領収証の場合、更新料金）
	uchar			syu;											// 種別
	teiki_use		teiki;											// 定期ﾃﾞｰﾀ
	ulong			ppc_chusi_ryo;									// 精算中止ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ料金
	ulong			BeforeTwari;									// 前回時間割引金額
	ulong			BeforeRwari;									// 前回料金割引金額
	ulong			BeforePwari;									// 前回％割引金額
	ulong			Wtax;											// 税金
	ulong			WInPrice;										// 入金額
	ulong			WChgPrice;										// 領収証の場合		：釣銭金額
																	// 精算中止の場合	：払戻金額
// 仕様変更(S) K.Onodera 2016/11/28 #1589 現金売上データは、入金額から釣銭と過払い金を引いた値とする
//	ulong			WTotalPrice;									// 領収金額
	long			WTotalPrice;									// 領収金額
// 仕様変更(E) K.Onodera 2016/11/28 #1589 現金売上データは、入金額から釣銭と過払い金を引いた値とする
	ulong			WFusoku;										// 領収証の場合		：釣銭払出不足金額
																	// 精算中止の場合	：払戻不足金額
	credit_use		credit;											// ｸﾚｼﾞｯﾄｶｰﾄﾞﾃﾞｰﾀ
// MH810105(S) MH364301 インボイス対応
	uchar			WTaxRate;										// 適用税率
// MH810105(E) MH364301 インボイス対応
	uchar			testflag;										// テスト印字用フラグ
																	// GM333200(修正精算変数の役割変更)
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//	ulong			MMPlace;										// 間違い精算：駐車位置		→新修正精算：修正元車室番号
//	ulong			MMPrice;										// 間違い精算：駐車料金		→新修正精算：修正元駐車料金(振替額)
//	ulong			MMInPrice;										// 間違い精算：入金額		→新修正精算：修正元入金額
//	ulong			MMTwari;										// 間違い精算：時間割引金額	→新修正精算：修正元払戻／支払掛
//	ulong			MMRwari;										// 間違い精算：料金割引金額	→新修正精算：修正元割引額(料金+時間)
//	ST_OIBAN		MMPwari;										// 修正元精算追番
//	uchar			MMSyubetu;										// 修正元料金種別
//	uchar			Zengakufg;										// 間違い精算：全額割引ﾌﾗｸﾞ	→新修正精算：下位b0bit：修正元全額割引(0:なし、1:あり)
//																	//										：下位b1bit：修正預り／掛け(0:預り、1:掛け)
//																	//										：下位b2ﾋﾞt：未払い(0:なし、1:あり)
	ulong			zenkai;											// 前回領収額
	uchar			CarSearchFlg;									// 車番/日時検索種別
	uchar			CarSearchData[6];								// 車番/日時検索データ
																	// 車番検索の場合：0～3に車番、4,5は0固定
																	// 日時検索の場合：年２バイト、それ以外は１バイト
	uchar			CarNumber[4];									// センター問い合わせ時の車番
	uchar			CarDataID[32];									// 車番データID(問合せに使用するID)
	date_time_rec	BeforeTPayTime;									// 前回精算日時
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
// 仕様変更(S) K.Onodera 2016/10/28 領収証フォーマット
	ushort			FRK_RetMod;										// 振替精算(ParkingWeb経由)振替過払い媒体
	ushort			FRK_Return;										// 振替精算(ParkingWeb経由)振替過払い金(払戻し)
// 仕様変更(E) K.Onodera 2016/10/28 領収証フォーマット
	Electron_info 	Electron_data;									// 電子決済精算情報
	t_LW_STATUS		PRTwari;										// 前回％、料金、時間割引フラグ
	ulong			ppc_chusi;										// 精算中止プリペイドデータ格納
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//	T_CARD			T_card;
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
	uchar			PayCalMax;										// 最大料金越えあり
	uchar			ReceiptIssue;									// 領収証発行有無
	uchar			PayMethod;										// 精算方法
	uchar			PayClass;										// 処理区分
	uchar			PayMode;										// 精算ﾓｰﾄﾞ
// MH810100(S) K.Onodera 2020/02/05 車番チケットレス(LCD_IF対応)
	uchar			CMachineNo;										// 駐車券機械№(入庫機械№)
// MH810100(S) K.Onodera 2020/02/05 車番チケットレス(LCD_IF対応)
	uchar			KakariNo;										// 係員№
	uchar			OutKind;										// 精算出庫
	uchar			CountSet;										// 在車ｶｳﾝﾄ
	uchar			PassCheck;										// ｱﾝﾁﾊﾟｽﾁｪｯｸ
	DATA_BASIC		DataBasic;										// NT-NET基本ﾃﾞｰﾀ
	DISCOUNT_DATA	DiscountData[WTIK_USEMAX];						// 割引ﾃﾞｰﾀ
// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
	DETAIL_DATA		DetailData[DETAIL_SYU_MAX];						// 割引データ(明細情報)
// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応
// GG129000(S) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
// 	ulong			SyuWariRyo;										// 種別割引料金
	// クラウド料金計算対応で種別割引を他の割引と同様に扱った結果として不要となった為、
	// DiscountDataの要素を1つ増やすため削減。
	// ※この削減は通常料金計算の種別割引に影響するが、前述のクラウド料金計算対応時に
	// 　通常料金計算はメンテ不要として変更されなかった為、もともと正常動作していない。
	// 　再び通常料金計算を使用する場合、通常料金計算でも「種別割引を他の割引と同様に扱う」対応が必要。
// GG129000(E) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
	ushort			ID;													// 22/56
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//	ushort			FullNo1;											// 満車台数１
//	ushort			CarCnt1;											// 現在台数１
//	ushort			FullNo2;											// 満車台数２
//	ushort			CarCnt2;											// 現在台数２
//	ushort			FullNo3;											// 満車台数３
//	ushort			CarCnt3;											// 現在台数３
//	ushort			PascarCnt;											// 定期車両カウント
//	ushort			Full[3];											// 駐車1～3満空状態
//// 不具合修正(S) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
//	CAR_CNT_INFO	CarCntInfo;											// 台数管理追番
//// 不具合修正(E) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
	uchar			in_coin[5];											// 金種毎入金枚数(10,50,100,500,1000)
	uchar			out_coin[5];										// 金種毎出金枚数(10,50,100,500,1000)
	ulong			CenterSeqNo;										// センター追番（精算）
	uchar			f_escrow;											// 精算データ、精算情報データで1000円の払戻し枚数をセットするフラグ
	uchar			SeqNo;											// シーケンシャルNo.
// MH322914(S) K.Onodera 2016/11/25 AI-V対応
//	uchar			reserve[48];									// 予備
// MH322914(E) K.Onodera 2016/11/25 AI-V対応
// MH810100(S) K.Onodera 2020/02/07 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
	ushort			MediaKind1;										// 種別(媒体)	0～99
	uchar			MediaCardNo1[32];								// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
	ushort			MediaKind2;										// 種別(媒体)	0～99
	uchar			MediaCardNo2[32];								// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
// MH810100(E) K.Onodera 2020/02/07 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
	// LOG_SECORNUM(0) = LogDatMax[0][1]/LogDatMax[0][0] = RECODE_SIZE/sizeof(Receipt_data) = (0x1000-6)/816 = 5…10
	// ベースファイルと同様に1セクタ当たり5個の個別精算情報が格納出来るよう16Byte単位の最大値「816Byte」の予備を確保
// MH810100(S) m.saito 2020/05/13 車番チケットレス(領収証印字：未精算出庫対応)
//	uchar			reserve[32];									// 予備
	uchar			shubetsu;										// 種別	(0=事前精算／1＝未精算出庫精算)
	date_time_rec	TUnpaidPayTime;									// 精算日時（未精算出庫用）
// MH810100(S) 2020/05/29 車番チケットレス(#4196)
//	uchar			reserve[25];									// 予備
// MH810100(e) 2020/05/29 車番チケットレス(#4196)
// MH810100(E) m.saito 2020/05/13 車番チケットレス(領収証印字：未精算出庫対応)
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
// MH810100(S) 2020/05/29 車番チケットレス(#4196)
// GG129000(S) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
// 	ulong			KaiWariDiff;									// 買物割引の前回からの差分
	// 未使用の為、DiscountDataの要素を1つ増やすため削減。
// GG129000(E) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
// MH810100(S) 2020/07/09 車番チケットレス(#4531 仕様変更 台数管理追番を台数管理をしなくても付与する)
//	uchar			reserve[21];									// 予備
	CAR_CNT_INFO	CarCntInfo;											// 台数管理追番
// MH810103 MHUT40XX(S) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
//	uchar			reserve[13];									// 予備
	uchar			EcResult;										// 決済結果
	uchar			EcErrCode[3];									// 詳細エラーコード
// GG124100(S) R.Endo 2021/12/23 車番チケットレス3.0 #6123 NT-NET精算データの車番情報セット
// 	uchar			reserve[9];
	SHABAN_INFO		ShabanInfo;										// 車番情報
	// LOG_SECORNUM(0) = LogDatMax[0][1]/LogDatMax[0][0] = RECODE_SIZE/sizeof(Receipt_data) = (0x1000-6)/1008 = 4…58
	// 1セクタ当たり4個の個別精算情報が格納出来るよう16Byte単位の最大値「1008Byte」の予備を確保
// GG124100(S) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 	uchar			reserve[103];
// GG124100(S) R.Endo 2022/08/19 車番チケットレス3.0 #6520 使用したサービス券の情報が領収証に印字されない
// 	uchar			reserve[63];
// GG129000(S) T.Nagai 2023/01/16 ゲート式車番チケットレスシステム対応（発券情報印字対応）
//	uchar			reserve[43];
	ulong			TicketNum;										// 駐車券番号（1～99999。0の場合値無し）
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
//	uchar			reserve[39];
	uchar			TInTime_Sec;									// 入庫秒
	uchar			TOutTime_Sec;									// 出庫秒
	uchar			BeforeTPayTime_Sec;								// 前回精算秒
	uchar			TUnpaidPayTime_Sec;								// 未精算秒
// GG129000(S) H.Fujinaga 2023/02/18 ゲート式車番チケットレスシステム対応（QR駐車券対応）
//	uchar			reserve[35];
	QRTICKET_INFO	QrTicketInfo;
// GG129000(S) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
// 	uchar			reserve[25];
	uchar			reserve[1];
// GG129000(E) R.Endo 2023/05/30 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
// GG129000(E) H.Fujinaga 2023/02/18 ゲート式車番チケットレスシステム対応（QR駐車券対応）
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
// GG129000(E) T.Nagai 2023/01/16 ゲート式車番チケットレスシステム対応（発券情報印字対応）
// GG124100(E) R.Endo 2022/08/19 車番チケットレス3.0 #6520 使用したサービス券の情報が領収証に印字されない
// GG124100(E) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// GG124100(E) R.Endo 2021/12/23 車番チケットレス3.0 #6123 NT-NET精算データの車番情報セット
// MH810103 MHUT40XX(E) DC=29ｈ→49ｈへ変更し、決済結果のステータスに応じてジャーナル印字を行う
// MH810100(E) 2020/07/09 車番チケットレス(#4531 仕様変更 台数管理追番を台数管理をしなくても付与する)
// MH810100(e) 2020/05/29 車番チケットレス(#4196)
// GG129001(S) データ保管サービス対応（課税対象額をセットする）
	ulong			WTaxPrice;										// 課税対象額
	ulong			WBillAmount;									// 請求額
// GG129001(E) データ保管サービス対応（課税対象額をセットする）
// GG129001(S) データ保管サービス対応（登録番号をセットする）
	uchar			RegistNum[14];									// 登録番号
// GG129001(E) データ保管サービス対応（登録番号をセットする）
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（精算データ変更）
	date_time_rec2	Before_Ts_Time;									// 前回Ｔ合計の時間	
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（精算データ変更）
} Receipt_data;
#pragma unpack

#define	_PAY_SEEK_LEN(a)	((ulong)&((Receipt_data*)NULL)->a)
#define	_PAY_DATA_LEN(a)	((ulong)sizeof(((Receipt_data*)NULL)->a))

extern	Receipt_data	PayData;									// 1精算情報,領収証印字ﾃﾞｰﾀ
extern	Receipt_data	PayDataBack;								// 1精算情報,領収証印字ﾃﾞｰﾀﾊﾞｯｸｱｯﾌﾟ
extern	Receipt_data	LabelPayData;								// 1精算情報,ﾗﾍﾞﾙ印字ﾃﾞｰﾀ(ﾜｰｸ)
struct Receipt_Data_Sub{
	wari_tiket		wari_data[25];							// ｻｰﾋﾞｽ券、掛売券、割引券データ
	ulong			pay_ryo;										// 回数券、プリペイド使用金額
};
extern struct Receipt_Data_Sub		PayData_Sub;					// 印字用詳細中止エリア
extern	uchar	Pay_Flg;											// 精算中止フラグ


typedef union{
	unsigned char	BYTE;					// Byte
	struct{
		unsigned char	YOBI:4;				// Bit 4-7 = 予備
		unsigned char	offset:4;			// Bit 0-3 = ｵﾌｾｯﾄ
	} BIT;
} t_PPC_Sub;

typedef struct {
	t_PPC_Sub	ppc_sub_data;
	ppc_tiket	ppc_data_detail[WTIK_USEMAX];
} t_PPC_DATA_DETAIL;

extern t_PPC_DATA_DETAIL	PPC_Data_Detail;
typedef	struct {
	uchar	year;
	uchar	mon;
	uchar	day;
	uchar	hour;
	uchar	min;
} t_TIME_INFO;
typedef struct {
	unsigned short 	Amount;				// 単位金額
	unsigned short 	Point;				// 単位ポイント
	t_TIME_INFO		start;				// 運用開始
	t_TIME_INFO		end;				// 運用終了
} t_T_Point_Time;

typedef struct {
	unsigned short Amount;				// 単位金額
	unsigned short Point;				// 単位ポイント
} t_T_Point_Base;
typedef struct {
	unsigned char  Tokubet;				// 期間レートﾌﾗｸﾞ
	unsigned short Amount;				// 単位金額
	unsigned short Point;				// 単位ポイント
	unsigned short disable;				// Tカード無効
} t_T_Point_Now;


typedef struct {
	t_T_Point_Time	T_Rate_time;				// 期間レート
	t_T_Point_Base	T_Rate_base;				// 基本レート
} t_T_POINT_DATA;

#define BIN_KETA_MAX		16

typedef struct{
	uchar		Id_BusCode[2];			// IDマーク/業態マーク
	uchar		Company[4];				// 会社コード
	uchar 		BIN_Code[20];			// BIN体系
	uchar		keta;					// BIN体系桁数
} t_BIN_DEF;

//----------------------------------------------------------------------//
// T・法人カードのエリア拡張・追加・削除を行なう場合はFLASH書き込み時の	//
// サイズ指定をしている、FLT_TCARD_DATA_SIZE、FLT_HCARD_DATA_SIZEの		//
// 修正が必要になります。詳しくはfla_def.hを参照すること				//
//----------------------------------------------------------------------//
typedef struct {
	uchar		Id_BusCode[2];			// IDマーク/業態マーク
	uchar		Company[4];				// 会社コード
	uchar 		BIN_start[BIN_KETA_MAX];			// BIN体系開始
	uchar		BIN_end[BIN_KETA_MAX];			// BIN体系終了
	uchar		Field;					// 任意フィールド
	uchar		Partner_Code2[4];		// 提携先コード２
	uchar		Ptr2_Chk;				// 提携先コード２判定方式
	uchar		Member_Chk;				// 会員番号判定方式
	uchar		Reserve[10];			// 予備（将来拡張予約）
	uchar		keta;					// BIN体系桁数
} t_BIN_ADD;	

#define BIN_ADD_MAX			20
#define	BLOCK_MAX			2

// T及び法人カードの構造体を変更する場合には、以下のバージョン情報も更新すること
// FLASH内にバージョン情報として保持します。
#define	T_MEMBER_VERTION			1	// Tカードバージョン情報
#define	H_MEMBER_VERTION			1	// 法人カードバージョン情報

extern	Receipt_data	RT_PayData;									// 1精算情報,領収証印字ﾃﾞｰﾀ

struct	RT_pay_log_rec {
	short			RT_pay_count;									// 精算履歴件数(RAM)
	short			RT_pay_wtp;										// 精算履歴ﾗｲﾄﾎﾟｲﾝﾀ(RAM)
	Receipt_data	RT_pay_log_dat[RT_PAY_LOG_CNT];					// 精算履歴ﾊﾞｯﾌｧ(RAM)
};

extern	struct	RT_pay_log_rec		RT_PAY_LOG_DAT;

/*--------------------------------------------------------------------------*/
/* センター追番																*/
/*--------------------------------------------------------------------------*/
typedef enum {
	RAU_SEQNO_ENTER,									// センター追番（入庫）
	RAU_SEQNO_PAY,										// センター追番（精算）
	RAU_SEQNO_TOTAL,									// センター追番（集計）
	RAU_SEQNO_CHANGE,									// センター追番（釣銭管理集計）

	RAU_SEQNO_TYPE_COUNT,								// センター追番種別数
} RAU_SEQNO_TYPE;

extern	ulong	Rau_SeqNo[RAU_SEQNO_TYPE_COUNT];	// センター追番

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
/*--------------------------------------------------------------------------*/
/* センター追番(DC-NET通信													*/
/*--------------------------------------------------------------------------*/
typedef enum {
	DC_SEQNO_QR,										// QR確定・取消データ
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	DC_SEQNO_LANE,										// レーンモニタデータ
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

	// ↑↑　追加はここより上に行う
	DC_SEQNO_TYPE_COUNT,								// センター追番種別数
} DC_SEQNO_TYPE;

extern	ulong	DC_SeqNo[DC_SEQNO_TYPE_COUNT];		// センター追番
extern	ulong	DC_SeqNo_wk[DC_SEQNO_TYPE_COUNT];	// センター追番ワークエリア

/*--------------------------------------------------------------------------*/
/* センター追番(リアルタイム通信											*/
/*--------------------------------------------------------------------------*/
typedef enum {
	REAL_SEQNO_PAY,										// 精算データ
// MH810100(S) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
	REAL_SEQNO_PAY_AFTER,								// 精算データ(後日精算)
// MH810100(E) 2020/11/02 #4971 未精算出庫のセンター追番を分ける対応
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	REAL_SEQNO_RECEIPT,									// 領収証データ
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

	// ↑↑　追加はここより上に行う
	REAL_SEQNO_TYPE_COUNT,								// センター追番種別数
} REAL_SEQNO_TYPE;

extern	ulong	REAL_SeqNo[REAL_SEQNO_TYPE_COUNT];	// センター追番
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)

/*--------------------------------------------------------------------------*/
/* 精算中止履歴																*/
/*--------------------------------------------------------------------------*/
extern	Receipt_data	Cancel_pri_work;

#pragma pack
/*--------------------------------------------------------------------------*/
/* 集計履歴																	*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	SYUKEI			syukei_log;										// 集計ｴﾘｱ
	short			dumy;											// not used
} Syu_log;

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
// typedef struct {
// 	ushort			Kikai_no;										// 機械№
// 	ushort			Kakari_no;										// 係員№
// 	ST_OIBAN		Oiban;											// 追番
// 	date_time_rec	NowTime;										// 今回集計日時
// 	date_time_rec	OldTime;										// 前回集計日時
// 	LOKTOTAL_DAT	loktldat[LOCK_MAX-SYU_LOCK_MAX];				// 駐車位置別集計(駐車位置毎:３１車室以降)
// } LCKTTL_LOG;
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

extern	SYUKEI		TSYU_LOG_WK;									// Ｔ合計ログ(30車室分の集計を含む)登録用ワークエリア
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
// extern	LCKTTL_LOG	LCKT_LOG_WK;									// 車室毎集計ログ(31車室以降)登録用ワークエリア
// extern	LCKTTL_LOG	lckttl_wk;										// 車室毎集計ログ(31車室以降)編集用ワークエリア
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

#pragma unpack

/*--------------------------------------------------------------------------*/
/* 釣銭管理履歴																*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	TURI_KAN		Mny;											// 釣銭管理
	short			dumy;											// not used
} Mny_log;

/*--------------------------------------------------------------------------*/
/* ｺｲﾝ金庫集計履歴															*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	COIN_SYU		cin_mem;										// ｺｲﾝ金庫集計
	short			dumy;											// not used
} coin_log;

/*--------------------------------------------------------------------------*/
/* 紙幣金庫集計履歴															*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	NOTE_SYU		note_mem;										// 紙幣金庫集計
	short			dumy;											// not used
} note_log;

// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(QR確定・取消データ対応)
///*--------------------------------------------------------------------------*/
///* 入庫履歴																	*/
///*--------------------------------------------------------------------------*/
//#pragma pack
//typedef	struct {
//	ulong			ParkingNo;										// 定期券駐車場№
//	ulong			PassID;											// 定期券ID
//	uchar			Contract[8];									// 定期券契約№
//	ushort			Syubet;											// 定期券種別
//	ushort			State;											// 定期券ｽﾃｰﾀｽ
//	ulong			Reserve1;										// 予備
//	ulong			Reserve2;										// 予備
//	ushort			MoveMode;										// 定期券回収／返却
//	ushort			ReadMode;										// 定期券ﾘｰﾄﾞﾗｲﾄ／ﾘｰﾄﾞｵﾝﾘｰ
//	ushort			SYear;											// 定期券開始年
//	uchar			SMon;											// 定期券開始月
//	uchar			SDate;											// 定期券開始日
//	ushort			EYear;											// 定期券終了年
//	uchar			EMon;											// 定期券終了月
//	uchar			EDate;											// 定期券終了日
//} PASS_DATA_LOG;
//
//
//typedef	struct {
//	ulong			InCount;										// 入庫追い番
//	ushort			Syubet;											// 処理区分
//	ushort			InMode;											// 入庫ﾓｰﾄﾞ
//	ulong			LockNo;											// 区画情報
//	ushort			CardType;										// 駐車券ﾀｲﾌﾟ
//	ushort			CMachineNo;										// 駐車券機械№
//	ulong			CardNo;											// 駐車券番号(発券追い番)
//	date_time_rec2	InTime;											// 入庫年月日時分秒
//	ushort			PassCheck;										// ｱﾝﾁﾊﾟｽﾁｪｯｸ
//	ushort			CountSet;										// 在車ｶｳﾝﾄ
//	PASS_DATA_LOG	PassData;										// 定期券ﾃﾞｰﾀ
//	union {
//		ushort			Dummy[32];									// ﾀﾞﾐｰ
//		struct {
//			ushort	ID;				// 20/54
//			ushort	FullNo1;		// 満車台数１
//			ushort	CarCnt1;		// 現在台数１
//			ushort	FullNo2;		// 満車台数２
//			ushort	CarCnt2;		// 現在台数２
//			ushort	FullNo3;		// 満車台数３
//			ushort	CarCnt3;		// 現在台数３
//			ushort	PascarCnt;		// 定期車両カウント
//			ushort	Full[3];		// 駐車1～3満空状態
//			long	CenterSeqNo;	// センター追番（入庫）
//			uchar	SeqNo;			// シーケンシャルNo.
//			uchar	ReserveByte;	// 予備
//			proc_date	ProcDate;	// 処理日時
//// 不具合修正(S) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
////			ushort	Reserve[15];	// 予備
//			CAR_CNT_INFO CarCntInfo;// 台数管理追番
//			ushort	Reserve[11];	// 予備
//// 不具合修正(E) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
//		} DT_54;					// 
//	} CMN_DT;
//} enter_log;
//#pragma unpack
//extern	enter_log	Enter_data;
// MH810100(S) 2020/2/18 #3852 RT精算データのフォーマットがずれる(再評価)
#pragma pack
// MH810100(E) 2020/2/18 #3852 RT精算データのフォーマットがずれる(再評価)
/*--------------------------------------------------------------------------*/
/* RT精算データ																*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	unsigned long			ID;					// 入庫から精算完了までを管理するID
	unsigned long			CenterOiban;		// センター追番
	unsigned short			CenterOibanFusei;	// センター追番不正ﾌﾗｸﾞ
	uchar					shubetsu;			// 精算種別
	unsigned short			crd_info_rev_no;	// カード情報Rev.No.
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)
// 	stSendCardInfo_Rev10	crd_info;			// カード情報
// GG129004(S) M.Fujikawa 2024/12/04 AMS改善要望 カード情報Rev.12対応
//	stSendCardInfo_Rev11	crd_info;			// カード情報
	stSendCardInfo_RevXX	crd_info;			// カード情報
// GG129004(E) M.Fujikawa 2024/12/04 AMS改善要望 カード情報Rev.12対応
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	// 本構造体を変更する場合は必ず RTPay_log_date も修正する
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
} RTPay_log;
extern	RTPay_log		RTPay_Data;			// RT精算データログ

// LOGレコード日付用
typedef	struct {
	unsigned long		ID;					// 入庫から精算完了までを管理するID
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	unsigned long		CenterOiban;		// センター追番
	unsigned short		CenterOibanFusei;	// センター追番不正ﾌﾗｸﾞ
	uchar				shubetsu;			// 精算種別
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	unsigned short		crd_info_rev_no;	///< カード情報Rev.No.
	uchar				FormatNo[2];		// ﾌｫｰﾏｯﾄRev.No.	0～65535
	stDatetTimeYtoSec_t	dtTimeYtoSec;		// 年月日時分秒
} RTPay_log_date;

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
/*--------------------------------------------------------------------------*/
/* RT領収証データ															*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	unsigned long			ID;					// 入庫から精算完了までを管理するID
	unsigned long			CenterOiban;		// センター追番
	unsigned short			CenterOibanFusei;	// センター追番不正フラグ
	stReceiptInfo			receipt_info;		// 領収証情報

	// 本構造体を変更する場合は必ず RTReceipt_log_date も修正する
} RTReceipt_log;
extern	RTReceipt_log		RTReceipt_Data;		// RT領収証データログ

// LOGレコード日付用
typedef	struct {
	unsigned long		ID;					// 入庫から精算完了までを管理するID
	unsigned long		CenterOiban;		// センター追番
	unsigned short		CenterOibanFusei;	// センター追番不正フラグ
	ushort				FormatNo;			// フォーマットRev.No.
	stDatetTimeYtoSec_t	dtTimeYtoSec;		// 処理 年月日時分秒
} RTReceipt_log_date;
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

// MH810100(S) 2020/2/18 #3852 RT精算データのフォーマットがずれる(再評価)
#pragma unpack
// MH810100(E) 2020/2/18 #3852 RT精算データのフォーマットがずれる(再評価)
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応)
/*--------------------------------------------------------------------------*/
/* 駐車台数データ															*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	date_time_rec2	Time;			// 生成日時					(236)
	ushort			GroupNo;		// ｸﾞﾙｰﾌﾟ番号				(236, 58)
	ulong			CurNum;			// 現在駐車台数				(236, 58)
	ulong			TkCurNum[15];	// 定期種別1～15現在駐車台数(236)
	ulong			RyCurNum[20];	// 料金種別1～20現在駐車台数(236)
	ulong			State;			// 用途別駐車台数設定		(236, 58)
	ulong			Full[3];		// 駐車1～3満空状態			(236, 58)
	union {
		ulong		Reserve[10];	// 予備(236)
		struct {
			ushort	ID;				// 236/58
			ushort	FmtRev;			// フォーマットRev.№
			ushort	EmptyNo1;		// 空車台数１
			ushort	FullNo1;		// 満車台数１
			ushort	EmptyNo2;		// 空車台数２
			ushort	FullNo2;		// 満車台数２
			ushort	EmptyNo3;		// 空車台数３
			ushort	FullNo3;		// 満車台数３
			uchar	SeqNo;			// シーケンシャルNo.
			uchar	ReserveByte;	// 予備
			ushort	Reserve[11];	// 予備
		} DT_58;					// (58)
	} CMN_DT;
} ParkCar_log;
extern	ParkCar_log		ParkCar_data;								// 駐車台数データ
extern	ParkCar_log		ParkCar_data_Bk;							// 駐車台数データ（前回送信データ）
/*--------------------------------------------------------------------------*/
/* 長期駐車データ															*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ulong		LockNo;
	t_TIME_INFO	InCarTime;
	uchar		Reserve;
} parking_info;
typedef	struct {
	date_time_rec2	Time;											// 生成日時
	ushort			DeviceID_BK;
	ushort			Kind;
	ushort			ParkingTerm;
	ushort			ParkingNum;
	parking_info	ParkInfo[8];
	uchar			Reserve[10];
} LongPark_log;
extern	LongPark_log	LongParking_data;
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
typedef	struct {
	date_time_rec2	InTime;		// 入庫日時
	date_time_rec2	OutTime;	// 出庫日時
	ulong			LockNo;		// 内部処理用駐車位置番号
	date_time_rec2	ProcDate;	// 処理日時
	ushort			Ck_Time;	// 検出時間
	uchar			Knd;		// 発生/解除/全解除
	uchar			SeqNo;		// シーケンシャルNo(NT-NET電文に設定する)
	uchar			Reserve[2];
} LongPark_log_Pweb;
extern	LongPark_log_Pweb	LongParking_data_Pweb;
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
///*--------------------------------------------------------------------------*/
///* Rismイベントデータ														*/
///*--------------------------------------------------------------------------*/
//typedef struct {					// イベントログデータ
//	date_time_rec	Time;			// 発生日時
//	unsigned long	Code;			// イベントコード
//} RismEvent_log;
//extern	RismEvent_log	RismEvent_data;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

///*--------------------------------------------------------------------------*/
///* 係員無効データ   	    												*/
///*--------------------------------------------------------------------------*/
//
//#define	ADN_INV_MAX		1000										// 係員無効ﾃﾞｰﾀMAX数
//
//
//typedef struct {
//	short		count;												// 登録件数
//	short		wtp;												// 次書込み位置
//	ushort		aino[ADN_INV_MAX];									// 係員No
//} t_InvalidWriteData;
//
//extern t_InvalidWriteData Attend_Invalid_table;	
/*--------------------------------------------------------------------------*/
/* 係員有効データ   	    												*/
/*--------------------------------------------------------------------------*/

#define	ADN_VAILD_MAX		1000										// 係員有効ﾃﾞｰﾀMAX数

typedef struct {
	short		count;												// 登録件数
	short		wtp;												// 次書込み位置
	ushort		aino[ADN_VAILD_MAX];									// 係員No
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は別途処理を記述しないと
	//	   バックアップ/リストアが正常終了しません。
} t_ValidWriteData;

extern t_ValidWriteData Attend_Invalid_table;	



/*--------------------------------------------------------------------------*/
/* カード種別 																*/
/*--------------------------------------------------------------------------*/
#define		NG_CARD_PASS			1								// 定期
#define		NG_CARD_TICKET			2								// 駐車券
#define		NG_CARD_CREDIT			3								// クレジットカード
#define		NG_CARD_BUSINESS		4								// ビジネスカード
#define		NG_CARD_AMANO			5								// アマノ係員カード不正
#define		OK_CARD_AMANO			6								// アマノ係員カード正常

/*--------------------------------------------------------------------------*/
/* 不正ログ履歴データ（１件分）												*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ushort			Card_type;										// カード種別
	union card_no {													// カード番号
		ulong		card_s;											// 定期、駐車券、係員ｶｰﾄﾞ									
		uchar		card_c[10];										// ｸﾚｼﾞｯﾄ、ﾋﾞｼﾞﾈｽｶｰﾄﾞ(BCD右詰)
	}Card_No;
	date_time_rec	NowTime;										// 検出日時
} NGLOG_DATA;
extern	NGLOG_DATA	nglog_data;
/*--------------------------------------------------------------------------*/
/* 不正ログ履歴																*/
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* 入出庫イベント種別 														*/
/*--------------------------------------------------------------------------*/
enum{																/* イベント種別 */
	IOLOG_EVNT_NONE = 0,											//  0:イベント未発生
	IOLOG_EVNT_FLAP_UP_STA,											//  1:フラップ上昇開始
	IOLOG_EVNT_FLAP_UP_FIN,											//  2:フラップ上昇完了
	IOLOG_EVNT_FLAP_DW_STA,											//  3:フラップ下降開始
	IOLOG_EVNT_FLAP_DW_FIN,											//  4:フラップ下降完了
	IOLOG_EVNT_FLAP_UP_UFN,											//  5:フラップ上昇未完
	IOLOG_EVNT_FLAP_DW_UFN,											//  6:フラップ下降未完
	IOLOG_EVNT_LOCK_CL_STA,											//  7:駐輪ロック閉開始
	IOLOG_EVNT_LOCK_CL_FIN,											//  8:駐輪ロック閉完了
	IOLOG_EVNT_LOCK_OP_STA,											//  9:駐輪ロック開開始
	IOLOG_EVNT_LOCK_OP_FIN,											// 10:駐輪ロック開完了
	IOLOG_EVNT_LOCK_CL_UFN,											// 11:駐輪ロック閉未完
	IOLOG_EVNT_LOCK_OP_UFN,											// 12:駐輪ロック開未完
	IOLOG_EVNT_ENT_GARAGE,											// 13:入庫
	IOLOG_EVNT_OUT_GARAGE,											// 14:出庫
	IOLOG_EVNT_RENT_GARAGE,											// 15:再入庫
	IOLOG_EVNT_FORCE_STA,											// 16:強制出庫開始
	IOLOG_EVNT_FORCE_FIN,											// 17:強制出庫完了
	IOLOG_EVNT_OUT_ILLEGAL,											// 18:不正出庫
	IOLOG_EVNT_AJAST_STA,											// 19:精算開始
	IOLOG_EVNT_AJAST_FIN,											// 20:精算完了
	IOLOG_EVNT_AJAST_STP,											// 21:精算中止
	IOLOG_EVNT_OUT_ILLEGAL_START,									// 22:不正出庫発生
	IOLOG_EVNT_ERR_REDSTACK_R_Ge,									// 23:磁気リーダ リーダ内カード詰まり 発生
	IOLOG_EVNT_ERR_REDSTACK_R_Re,									// 24:磁気リーダ リーダ内カード詰まり 解除
	IOLOG_EVNT_ERR_REDSTACK_P_Ge,									// 25:磁気リーダ プリンター内カード詰まり 発生
	IOLOG_EVNT_ERR_REDSTACK_P_Re,									// 26:磁気リーダ プリンター内カード詰まり 解除
	IOLOG_EVNT_ERR_COINDISPENCEFAIL_Ge,								// 27:コイン払出異常 発生
	IOLOG_EVNT_ERR_COINDISPENCEFAIL_Re,								// 28:コイン払出異常 解除
	IOLOG_EVNT_ERR_COINSAFE,										// 29:硬貨返却コマンド送信後の停復電発生
	IOLOG_EVNT_ERR_NOTEJAM_Ge,										// 30:紙幣詰り 発生
	IOLOG_EVNT_ERR_NOTEJAM_Re,										// 31:紙幣詰り 解除
	IOLOG_EVNT_ERR_NOTEREJECT_Ge,									// 32:紙幣払出し異常 発生
	IOLOG_EVNT_ERR_NOTEREJECT_Re,									// 33:紙幣払出し異常 解除
	IOLOG_EVNT_ERR_NOTESAFE,										// 34:紙幣返却コマンド送信後の停復電発生
	IOLOG_EVNT_MAX													// xx:イベント数
};

// MH810100(S) K.Onodera  2019/12/26 車番チケットレス(QR確定・取消データ対応)
///*--------------------------------------------------------------------------*/
///* 入出庫ログ履歴データ（１件分）											*/
///*--------------------------------------------------------------------------*/
//typedef struct {
//	date_time_rec	NowTime;										// 発生日時
//	ushort			Event_type;										// 入出庫イベント種別
//	union io_inf {													// 入出庫詳細情報
//		uchar		io_c[4];										// 未使用
//		ushort		io_s[2];										// 未使用
//		ST_OIBAN2	io_l;											// 追い番（精算中止、完了）／券番号（精算開始）
//	}IO_INF;
//} IOLOG_DATA;
//
///*--------------------------------------------------------------------------*/
///* 入出庫ログ履歴															*/
///*--------------------------------------------------------------------------*/
//typedef	struct {
//	IOLOG_DATA		iolog_data[IO_EVE_CNT];							// 入出庫ログ履歴データ（20件）
//	ushort			is_rec:2;										// 入出庫ログ記録中(0:未実施/1:記録中/2:記録完了)
//	ushort			room:14;										// 入出庫ログ車室番号
//} IoLog_Data;
//
//// MH322917(S) A.Iiizumi 2018/11/22 注意事項のコメント追加
//// 注意事項：Io_log_rec構造体は入出庫ログ書き込みのSRAM領域よりメンバ変更しないこと
//// 本エリアの変更が必要な場合はバージョンアップでログをログバージョン「LOG_VERSION」をアップデートし、
//// opetask()の中でLOG_VERSIONにあった全ログクリア処理を追加すること
//// MH322917(E) A.Iiizumi 2018/11/22 注意事項のコメント追加
//struct	Io_log_rec {
//	ushort			iolog_count;									// 入出庫ログ履歴件数
//	ushort			iolog_wtp;										// 入出庫ログ履歴ﾗｲﾄﾎﾟｲﾝﾀ
//	ushort			LockIndex;
//	IoLog_Data		iolog_list[IO_LOG_CNT];							// 入出庫ログ履歴ﾊﾞｯﾌｧ
//};
//extern	struct	Io_log_rec	IO_LOG_REC;
//extern	IoLog_Data	Io_log_wk;
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(QR確定・取消データ対応)
/*--------------------------------------------------------------------------*/
/* Suica通信履歴															*/
/*--------------------------------------------------------------------------*/
// バッファサイズ変更
#define	SUICA_LOG_MAXSIZE			48000	// 通信ﾛｸﾞﾊﾞｯﾌｧｻｲｽﾞ
#define	SUICA_LOG_NEAR_PRINT_SIZE	4000	// 直近のログの印字サイズ

typedef union{
	unsigned char	BYTE;					// Byte
	struct{
		unsigned char	YOBI:6;				// Bit 2-7 = 予備
		unsigned char	log_Write_Start:1;	// Bit 1 = ﾛｸﾞ開始ﾌﾗｸﾞ
		unsigned char	write_flag:1;		// Bit 0 = 書込みﾌﾗｸﾞ
	} BIT;
} t_Suica_log;

struct suica_log_rec{
	ushort	log_wpt;						// log書込みﾎﾟｲﾝﾀﾞ
	short	log_time_old;					// 最古の書込みﾃﾞｰﾀ保持領域
	char	log_Buf[SUICA_LOG_MAXSIZE];		// LOGﾃﾞｰﾀ格納領域
	char	cNull;
	t_Suica_log	Suica_log_event;			// suicaﾛｸﾞｲﾍﾞﾝﾄ
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は予備領域を使用すること!!!
	//	   途中にメンバを挿入しないこと
};

extern	struct	suica_log_rec	SUICA_LOG_REC;
extern	struct	suica_log_rec	SUICA_LOG_REC_FOR_ERR;	// 決済異常が発生した時のログ情報

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//extern ulong	Edy_OpeDealNo;				// Main管理用Edy取引通番
//
//#define	EDY_ARMLOG_MAX	20
//
//typedef struct {
//	uchar				Kikai_no;									// 機械Ｎｏ（自番号0,1-20）
//	uchar				TD_Kikai_no;								// 発券機 機械番号（1～20。0の場合値無し）
//	ulong				TicketNum;									// 駐車券番号（1～99999。0の場合値無し）
//	uchar				syubet;										// 種別
//	teiki_use			teiki;										// 定期ﾃﾞｰﾀ
//	date_time_rec		TOutTime;									// 発行時刻
//	uchar				Edy_CardNo[8];								// Suica ID
//	ulong				Pay_Ryo;									// 実際の決済額
//	ulong				Pay_Befor;									// 決算前Suica残高
//	ulong				Pay_After;									// 決済後Suica残高
//	ulong				Edy_Deal_num;								// 取引通番
//	ushort				Card_Deal_num;								// ｶｰﾄﾞ取引通番
//	uchar				High_Terminal_ID[4];						// 上位端末ID
//} edy_arm_log;
//
//struct Edy_Arm_Log{
//	short		log_cnt;											// ログ履歴件数
//	ushort		log_wpt;											// log書込みﾎﾟｲﾝﾀﾞ
//	edy_arm_log	log_Buf[EDY_ARMLOG_MAX];							// LOGﾃﾞｰﾀ格納領域
//};
//extern	edy_arm_log		Edy_Arm_log_work;									// 停復電ログワークバッファ
//
//
//#define	EDY_SHIMELOG_MAX	40
//
//typedef struct {
//	date_time_rec		ShimeTime;									// 締め実施時刻
//	uchar				High_Terminal_ID[4];						// 上位端末ID
//	ulong				Electron_edy_cnt;							// Edy精算  ＜回数＞
//	ulong				Electron_edy_ryo;							// 　　　　 ＜金額＞
//	ulong				Electron_Arm_cnt;							// Edyｱﾗｰﾑ取引  ＜回数＞
//	ulong				Electron_Arm_ryo;							// 　　　　     ＜金額＞
//	uchar				Shime_OKorNG;								// 締め結果（0:OK/1:NG）
//	uchar				fg_DataAri;									// EDY_SHIME_LOG_NOW でのみ使用するﾃﾞｰﾀありﾌﾗｸﾞ（1=あり）
//																	// （停電時LOG Edy締めLOG登録用）
//																	// EMから精算回数&金額受信～LOG登録までの間1となる。
//} edy_shime_log;
//
//struct Edy_Shime_Log{
//	short		log_cnt;											// ログ履歴件数
//	ushort		log_wpt;											// log書込みﾎﾟｲﾝﾀﾞ
//	edy_shime_log	log_Buf[EDY_SHIMELOG_MAX];						// LOGﾃﾞｰﾀ格納領域
//};
//
//extern	edy_shime_log	EDY_SHIME_LOG_NOW;
//
//extern	uchar	Edy_High_Terminal_ID[4];							// 上位端末ID保存ｴﾘｱ
//
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#define		CRE_SALENG_MAX			10				/* クレジット売上拒否最大件数		*/
#define		CRE_SHOP_ACCOUNTBAN_MAX	20				/* クレジット取引番号最大文字数		*/
typedef struct {
	uchar			MediaCardNo[30];								// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
	uchar			MediaCardInfo[16];								// カード情報	"0"～"9"、"A"～"Z"(英数字) 左詰め
	uchar			reserve[2];										// 予備

} t_MediaInfoCre2;	// 入庫／精算　媒体情報
struct	DATA_BK{									/*									*/
	unsigned long	ryo;							/* ﾊﾞｯｸｱｯﾌﾟ料金						*/
	date_time_rec	time;							/* 精算時刻							*/
	ulong			slip_no;						/* ｸﾚｼﾞｯﾄｶｰﾄﾞ伝票番号				*/
	char			AppNoChar[6];					// ｸﾚｼﾞｯﾄｶｰﾄﾞ承認番号(英数字)
	char			shop_account_no[20];			/* ｸﾚｼﾞｯﾄｶｰﾄﾞ加盟店取引番号			*/
	uchar			CMachineNo;										// 駐車券機械№(入庫機械№)	0～255
	uchar			PayMethod2;										// 精算方法(0＝券なし精算/1＝駐車券精算/2＝定期券精算/3＝定期券併用精算/4＝紛失精算)
	uchar			PayClass;										// 処理区分(0＝精算/1＝再精算/2＝精算中止/3＝再精算中止, 8=精算前, 9=精算途中
	uchar			PayMode;										// 精算モード(0＝自動精算/1＝半自動精算/2＝手動精算/3＝精算なし/4＝遠隔精算)
	ushort			FlapArea;										// フラップシステム	区画		0～99
	ushort			FlapParkNo;										// 					車室番号	0～9999
	ulong			Price;											// 駐車料金			0～
	ulong			OptionPrice;									// その他料金		0～
	ulong			Discount;										// 割引額(合算)
	ulong			CashPrice;										// 現金売上			0～
	ulong			PayCount;										// 精算追番(0～99999)
	ushort			MediaKind;										// 種別(媒体)	0～99
	t_MediaInfoCre2	Media;											// 精算媒体情報1
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は予備領域を使用すること!!!
	//	   途中にメンバを挿入しないこと
};													/*									*/
													/*									*/
typedef struct {									/* ｴﾗｰ解除ﾃﾞｰﾀ ｴﾘｱ					*/
	char	ken;									/* 売上拒否件数						*/
	struct	DATA_BK back[CRE_SALENG_MAX];			/*									*/
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は予備領域を使用すること!!!
	//	   途中にメンバを挿入しないこと
} CRE_SALENG;										/* 　　　　　　　　　　　　　　　　 */
extern	CRE_SALENG	cre_saleng;						// 売上拒否データ１０件分
													/*									*/
//
// 売上収集チェック制御用エリア
//
typedef struct {
	short			UmuFlag;						// 売上済？フラグ(A3送信時にON)
	struct	DATA_BK	back;							// 売上情報
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は予備領域を使用すること!!!
	//	   途中にメンバを挿入しないこと
} CRE_URIAGE;
extern CRE_URIAGE	cre_uriage;
extern	ulong		cre_slipno;						// クレジット端末処理通番

// MH321800(S) G.So ICクレジット対応 不要機能削除(CCT)
//extern struct clk_rec	ClkLastSend;				/* カード情報要求最終送信時刻	*/
//
// MH321800(E) G.So ICクレジット対応 不要機能削除(CCT)
//
// クレジット利用明細用エリア
//
typedef struct {
	date_time_rec	PayTime;										// 精算日時
	char			card_name[12];									// ｸﾚｼﾞｯﾄｶｰﾄﾞ会社略号
	char			card_no[20];									// ｸﾚｼﾞｯﾄｶｰﾄﾞ会員番号
	ulong			slip_no;										// ｸﾚｼﾞｯﾄｶｰﾄﾞ伝票番号
	char			AppNoChar[6];									// ｸﾚｼﾞｯﾄｶｰﾄﾞ承認番号(英数字)
	char			shop_account_no[20];							// ｸﾚｼﾞｯﾄｶｰﾄﾞ加盟店取引番号
	ulong			pay_ryo;										// ｸﾚｼﾞｯﾄｶｰﾄﾞ利用金額
} meisai_log;
extern meisai_log	meisai_work;		// 利用明細ワーク

typedef	struct {
	ushort		event1;			/* control event */
	ushort		status1;		/* control status */
	ulong		event2;			/* data event to send */
	ulong		status2;		/* data status to send */
} NTNET_AUTO_TRANCTRL;

extern	NTNET_AUTO_TRANCTRL	ntTranCtrl;

typedef	struct {
	struct	clk_rec		last0;		/* last batch send time */
	struct	clk_rec		last1;		/* last transaction(number of parked cars) time */
} NTNET_AUTO_TRANTIME;

extern	NTNET_AUTO_TRANTIME	ntTranTime;

extern	short	NT_pcars_timer;

typedef struct {
	date_time_rec	SyuTime;										// 集計日時
	ushort			cnt;											// 総利用回数
	ulong			ryo;											// 総利用金額
} syu_dat;

typedef struct {
	date_time_rec	NowTime;										// 今回集計時間
	date_time_rec	OldTime;										// 前回集計時間

	uchar			ptr;											// 最新の格納位置
	uchar			cnt;											// Suica/Edy集計の格納数

	syu_dat			now;											// 当日の集計
	syu_dat			bun[SYUUKEI_DAY];								// 過去40日分の集計（＋ work用（当日クリア分））
} syuukei_info;

// MH321800(S) G.So ICクレジット対応
typedef struct {
	date_time_rec	SyuTime;										// 集計日時
// MH810105 GG119202(S) 処理未了取引集計仕様改善
//// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
//	// cnt[EC_BRAND_TOTAL_MAX - 1] と ryo[EC_BRAND_TOTAL_MAX - 1]にみなし決済の回数と金額を格納する
//// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
// MH810105 GG119202(E) 処理未了取引集計仕様改善
	ushort			cnt[EC_BRAND_TOTAL_MAX];						// 総利用回数
	ulong			ryo[EC_BRAND_TOTAL_MAX];						// 総利用金額
	ushort			cnt2[EC_BRAND_TOTAL_MAX];						// 総アラーム回数
	ulong			ryo2[EC_BRAND_TOTAL_MAX];						// 総アラーム金額
	ulong			unknownTimeCnt;									// 決済日時不明集計回数
} syu_dat2;

// MH810105 GG119202(S) 処理未了取引集計仕様改善
// みなし決済集計
#define	sp_minashi_cnt			cnt2[EC_SAPICA_USED-EC_EDY_USED]
#define	sp_minashi_ryo			ryo2[EC_SAPICA_USED-EC_EDY_USED]

// 処理未了取引集計（0～2,4～6のcnt2,ryo2は使用しないこと）
#define	sp_miryo_pay_ok_cnt		cnt2[EC_CREDIT_USED-EC_EDY_USED]
#define	sp_miryo_pay_ok_ryo		ryo2[EC_CREDIT_USED-EC_EDY_USED]
#define	sp_miryo_unknown_cnt	cnt2[EC_TCARD_USED-EC_EDY_USED]
#define	sp_miryo_unknown_ryo	ryo2[EC_TCARD_USED-EC_EDY_USED]
// MH810105 GG119202(E) 処理未了取引集計仕様改善
typedef struct {
	date_time_rec	NowTime;										// 今回集計時間
	date_time_rec	OldTime;										// 前回集計時間

	uchar			ptr;											// 最新の格納位置
	uchar			cnt;											// 決済リーダ集計の格納数

	syu_dat2		next;											// 取引データ			：翌日の集計
	syu_dat2		now;											// 取引データ			：当日の集計
	syu_dat2		bun[SYUUKEI_DAY_EC];							// 取引データ			：過去64日分の集計（＋ work用（当日クリア分））
} syuukei_info2;
// MH321800(E) G.So ICクレジット対応

struct Syuukei_log_rec {
	syuukei_info	sca_inf;										// Ｓｕｉｃａ集計情報
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//	syuukei_info	edy_inf;										// Ｅｄｙ集計情報
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
// MH321800(S) G.So ICクレジット対応
	syuukei_info2	ec_inf;											// ＥＣ集計情報
// MH321800(E) G.So ICクレジット対応
	// 注：本構造体はバックアップ/リストア対象であるため、
	//	   メンバを追加する場合は別途処理を記述しないと
	//	   バックアップ/リストアが正常終了しません。
};

extern	struct Syuukei_log_rec	Syuukei_sp;
extern	syu_dat bk_syu_dat;
// MH321800(S) G.So ICクレジット対応
extern	syu_dat2 bk_syu_dat_ec;
// MH810105(S) MH364301 未了残高照会タイムアウト時の動作仕様変更
//typedef struct {
//	uchar			Brand;											// 復電用アラーム取引ブランド
//	ulong			Ryo;											// 復電用アラーム取引額
//} EMoneyAlarmFukuden;
//extern	EMoneyAlarmFukuden	EcAlarm;								// 決済リーダ復電用アラーム取引データ
// MH810105(E) MH364301 未了残高照会タイムアウト時の動作仕様変更
typedef struct {
// MH810103 GG118807_GG118907(S) 「未了残高照会完了」時にも未了確定をカウントする
//	ulong				WPlace;										// 車室番号
//// MH810100(S) S.Nishimoto 2020/08/20 車番チケットレス(#4602:決済精算中止（復）で車番情報が印刷されない)
//	uchar				CarSearchFlg;								// 車番/日時検索種別
//	uchar				CarSearchData[6];							// 車番/日時検索データ
//	uchar				CarNumber[4];								// 車番(精算)
//	uchar				CarDataID[32];								// 車番データID
//// MH810100(E) S.Nishimoto 2020/08/20 車番チケットレス(#4602:決済精算中止（復）で車番情報が印刷されない)
//	uchar				update_mon;									// 更新月数
//	uchar				syu;										// 車種
//	ulong				WPrice;										// 駐車料金（定期更新領収証の場合、更新料金）
//	ulong				PayPrice;									// みなし決済額
//	date_time_rec		TInTime;									// 入庫日時
//	ushort				EcDeemedBrandNo;							// みなし決済発生ブランドNo
//// GG119202(S) ICクレジットみなし決済印字フォーマット変更対応
////	date_time_rec		EcDeemedDate;								// みなし決済発生日時
//	date_time_rec		TOutTime;									// 出庫日時
//// GG119202(E) ICクレジットみなし決済印字フォーマット変更対応
// MH810100(S) S.Nishimoto 2020/08/20 車番チケットレス(#4602:決済精算中止（復）で車番情報が印刷されない)
	uchar				CarSearchFlg;								// 車番/日時検索種別
	uchar				CarSearchData[6];							// 車番/日時検索データ
	uchar				CarNumber[4];								// 車番(精算)
	uchar				CarDataID[32];								// 車番データID
// MH810100(E) S.Nishimoto 2020/08/20 車番チケットレス(#4602:決済精算中止（復）で車番情報が印刷されない)
	ulong				PayPrice;									// みなし決済額
	ushort				EcDeemedBrandNo;							// みなし決済発生ブランドNo
	uchar				MiryoFlg;									// 1:未了中, 2:未了残高照会中
	uchar				dummy[25];
// MH810103 GG118807_GG118907(E) 「未了残高照会完了」時にも未了確定をカウントする
} EcDeemedFukudenLog;
extern	EcDeemedFukudenLog	EcDeemedLog;							// みなし決済復電ログデータ
extern	Receipt_data	EcRecvDeemedData;							// 決済精算中止(復決済)データ
// MH321800(E) G.So ICクレジット対応
// MH810105 GG119202(S) 決済リーダバージョンアップ中の復電対応
extern	uchar			EcReaderVerUpFlg;							// 決済リーダバージョンアップ中フラグ
// MH810105 GG119202(E) 決済リーダバージョンアップ中の復電対応

extern	const	uchar	Mif_Read_Sector[8];							/* 読出ｾｸﾀ指定ｴﾘｱ */
extern	const	uchar	Mif_Read_AccessBlock[8];					/* ｱｸｾｽﾌﾞﾛｯｸ指定ｴﾘｱ	*/
extern	const	uchar	Mif_W_DataSpecify[3][8];					/* 書込みﾃﾞｰﾀ指定ｴﾘｱ */
extern	const	uchar	Mif_W_Verify[3][8]; 						/* 書込みﾍﾞﾘﾌｧｲｴﾘｱ */
extern	const	uchar	Mif_AccessKey[8]; 							/* ｱｸｾｽ鍵ｴﾘｱ */
#define	SUICA_QUE_MAX_SIZE 256
#define	SUICA_QUE_MAX_COUNT 10

extern	ushort		CLK_REC_msec;		// 時計ミリ秒値

typedef struct{
	unsigned short	readpt;
	unsigned short	writept;
	unsigned short	count;
	unsigned char	rcvdata[SUICA_QUE_MAX_COUNT][SUICA_QUE_MAX_SIZE];
}t_SUICA_RCV_QUE;

extern t_SUICA_RCV_QUE	suica_rcv_que;

extern	ushort	LongTermParkingPrevTime[2];
/* 遠隔料金設定ログ */
#define	REMOTELOG_COUNT_MAX		1000
typedef	struct {
	date_time_rec	rcv_Time;									// 設定日時
	unsigned short	addr;										// 設定アドレス
	unsigned long	rcv_data;									// 設定データ
} t_Change_data;

extern t_Change_data	remote_Change_data;

/* 種別毎に設定を分ける項目(46-0001・46-0002) */
enum {
	ROCK_CLOSE_TIMER = 0,
	FLAP_UP_TIMER,
	FLP_ROCK_INTIME,			// ロック閉・フラップ上昇・ラグタイム内制御
	LAG_PAY_PTN,				// ラグタイムアップ後の料金
	WARI_PAY_PTN,				// 時間割引/定期後の料金
	TYUU_DAISUU_MODE,			// 駐車台数管理方式
	IN_CHK_TIME,				// 入庫判定タイム
	
	SYUSET_MAX,
};

/* 検査モード情報格納エリア */
typedef struct{
	unsigned char	dir_chkresult[5];	// ディレクトリ
	unsigned char	fn_chkresult[30]; 	// 結果情報ファイル名
	unsigned char	pt_no;				// モードナンバー
	unsigned char	Chk_mod;			// チェック中の項目
	unsigned char	Chk_str1[15];		// 固定文字列①
	unsigned char	Chk_str2[15];		// 固定文字列②
} t_Chk_info;
extern	t_Chk_info Chk_info;
/* 検査結果格納エリア */
typedef struct{
	unsigned short	Kakari_no;
	unsigned long	Chk_no;
	unsigned short	Card_no;
	date_time_rec	Chk_date;
	unsigned short	ptn;
	unsigned char set[8];
	unsigned char moj[16];
	unsigned char key_disp[2];
	unsigned char mag[4];
	unsigned char led_shut[2];
	unsigned char sw[8];
	unsigned char r_print[6];
	unsigned char j_print[5];
	unsigned char ann[3];
	unsigned char sig[1];
	unsigned char coin[3];
	unsigned char note[3];
} t_Chk_result;
extern t_Chk_result Chk_result;
#define		CHK_RESULT01		6								/* 検査結果01設定数			*/
#define		CHK_RESULT02		3								/* 検査結果02設定数			*/
#define		CHK_RESULT03		4								/* 検査結果03設定数			*/
#define		CHK_RESULT04		15								/* 検査結果04設定数			*/
#define		CHK_RESULT05		8								/* 検査結果05設定数			*/
#define		CHK_RESULT06		7								/* 検査結果06設定数			*/
#define		CHK_RESULT07		5								/* 検査結果07設定数			*/
#define		CHK_RESULT08		6								/* 検査結果08設定数			*/
#define		CHK_RESULT09		3								/* 検査結果09設定数			*/
#define		CHK_RESULT10		8								/* 検査結果10設定数			*/
#define		CHK_RESULT11		8								/* 検査結果11設定数			*/
#define		CHK_RESULT12		2								/* 検査結果12設定数			*/

#define		CHK_RESULT_NUMMAX	13
typedef struct{
	date_time_rec	Chk_date;		// 検査日
	unsigned short	Kakari_no;		// 社員番号
	unsigned char	Machine_No[6];		// 製品号機
	unsigned char	Model[8];			// モデル？
	unsigned char	System;			// システム構成
	unsigned char	Sub_Money;		// サブチューブ金種
} t_Chk_Res01;

typedef struct{
	// ソフトウェアバージョン
	unsigned char Version[3][8];
} t_Chk_Res02;

typedef struct{
	unsigned char	Dip_sw[3][4];
	unsigned char	Dip_sw6;
} t_Chk_Res03;

typedef struct{
	unsigned char	res_dat[15];
} t_Chk_Res04;

typedef struct{
	unsigned char	Mojule[27][8];				// モジュールナンバー
} t_Chk_Res05;

typedef struct{
	unsigned char	res_dat[7];
} t_Chk_Res06;

typedef struct{
	unsigned char	res_dat[5];
} t_Chk_Res07;

typedef struct{
	unsigned char	res_dat[6];
} t_Chk_Res08;

typedef struct{
	unsigned char	res_dat[3];
} t_Chk_Res09;

typedef struct{
	unsigned char	res_dat[8];
} t_Chk_Res10;

typedef struct{
	unsigned char	res_dat[8];
} t_Chk_Res11;

typedef struct{
	unsigned char	res_dat[2];
} t_Chk_Res12;

typedef struct{
	char	Chk_Res00;
	t_Chk_Res01		Chk_Res01;
	t_Chk_Res02		Chk_Res02;
	t_Chk_Res03		Chk_Res03;
	t_Chk_Res04		Chk_Res04;
	t_Chk_Res05		Chk_Res05;
	t_Chk_Res06		Chk_Res06;
	t_Chk_Res07		Chk_Res07;
	t_Chk_Res08		Chk_Res08;
	t_Chk_Res09		Chk_Res09;
	t_Chk_Res10		Chk_Res10;
	t_Chk_Res11		Chk_Res11;
	t_Chk_Res12		Chk_Res12;
} t_Chk_res_ftp;

extern t_Chk_res_ftp Chk_res_ftp;
extern	char	*Chk_result_p[];												/* ﾊﾟﾗﾒｰﾀｾｯｼｮﾝﾃｰﾌﾞﾙ				*/
/*--------------------------------------------------------------------------*/
/* FTPワークエリア															*/
/*--------------------------------------------------------------------------*/
extern	char	ETC_cache[65536];
extern	char	FTP_buff[];
extern	char	SCR_buff[8192];
extern	ushort		xSTACK_TBL[TSKMAX];							/* タスク毎のスタックサイズテーブル */
extern	ulong		xSTK_RANGE_BTM;								/* スタック終了アドレス */

#define	ENCKEY_NUM	(1)											/* 暗号化キーの個数 */
extern	uchar	Encryption_Key[ENCKEY_NUM][16];					/* 暗号化キー */
extern	uchar	ryo_test_flag;




// SNTP固定パラメータ
#define	_SNTP_PORTNO			123
#define	_SNTP_TOUT_IBK			5		// 5 sec
#define	_SNTP_RESPONSE_TOUT		10		// 10 sec
#define	_SNTP_WAI_MAX			5		// 1+5 times
#define	_SNTP_RETRY_MAX			2		// 1+2 times
#define	_SNTP_REQ_MAX			3		// 3 times
#define	_SNTP_RETRY_INT			60L		// 60 min
#define	_SNTP_d_LIMIT			1600	// 1600 msec (ルート遅延閾値)
#define	_SNTP_UPPER_LIMIT		600		// 600 sec (誤差上限時間)
#define	_SNTP_LOWER_LIMIT		500		// 500 msec (誤差下限時間)


/*--------------------------------------------------------------------------*/
/* 遠隔ダウンロードワークエリア												*/
/*--------------------------------------------------------------------------*/
// MH810100(S) Y.Yamauchi 20191120 車番チケットレス(遠隔ダウンロード)
//#define TEMP_BUFF_MAX			65536		// 64K
#define TEMP_BUFF_MAX			(128*1024)		// 128K
// MH810100(E) Y.Yamauchi 20191120 車番チケットレス(遠隔ダウンロード)
extern uchar	g_TempUse_Buffer[TEMP_BUFF_MAX];
// MH810100(S) Y.Yamauchi 20191120 車番チケットレス(遠隔ダウンロード)
extern uchar	g_TempUse_Buffer2[TEMP_BUFF_MAX];
// MH810100(E) Y.Yamauchi 20191120 車番チケットレス(遠隔ダウンロード)
// GG120600(S) // Phase9 設定変更通知対応
typedef enum{
	FTP_REQ_NONE,			// 0:要求なし
	FTP_REQ_NORMAL,			// 1:FTP要求あり
	FTP_REQ_WITH_POWEROFF,	// 2:FTP要求あり(電断要)
	PRM_CHG_REQ_NONE = 10,		// 0:変更なし
	PRM_CHG_REQ_NORMAL,			// 1:変更あり
} eFTP_REQ_TYPE;
extern eFTP_REQ_TYPE 	g_PrmChgFlg;						// パラメータ変更有無
// GG120600(E) // Phase9 設定変更通知対応

/*--------------------------------------------------------------------------*/
/* 時刻自動補正情報履歴														*/
/*--------------------------------------------------------------------------*/
enum {
	_T_DELAY,						// 補正遅延中
	_T_WATCH,						// 補正時刻監視中
	_T_BUSY,						// SNTP応答待ち
	_T_LAG,							// msec調整中
};

enum {
	_SNTP_RESULT_CODE = 0x5300,
	SNTP_LIMIT_OVER,				// 補正上限超過				：●
	SNTP_NORMAL_REVISE,				// 時刻補正実施				：○
	SNTP_NORMAL_SKIP,				// 時刻補正未実施			：◎
	SNTP_DELAYED_REVISE,			// 遅延あり・時刻補正実施	：△
	SNTP_DELAYED_SKIP,				// 遅延あり・時刻補正未実施	：▽
	SNTP_DELAYED_RETRY,				// 遅延あり・再処理			：▲
	SNTP_BADDATA,					// 無効データ				：×
	SNTP_NOTREADY_WAIT,				// 通信待機					：＋
	SNTP_GUARD_TIME,				// 端末が時刻更新禁止時間帯	：※
	SNTP_TIMEOUT,					// 応答タイムアウト			：－
};

typedef struct {
	uchar		type;				// 実施タイプ
	uchar		reqno;				// 実施回数
	ushort		fExec;				// 実施結果
	date_time_rec2	Req_Date;		// 時刻自動補正要求日時
	date_time_rec2	Rsp_Date;		// 時刻自動補正実行日時
	ulong		d;					// ルート遅延(ms)
	long		t;					// 端末時差(ms)
} NTP_log;

typedef struct {
	uchar		mode;				// 1=有効, 0=無効
	uchar		state;				// 状態
	uchar		stop;				// 1=停止
	uchar		req_adj;			// 1=補正要求
	uchar		method;				// 時刻取得方式={0:周期、1:時刻}
	uchar		waicnt;				// PPP接続待ち回数
	uchar		trycnt;				// トライ回数
	uchar		reqcnt;				// 要求回数(ルート遅延NG用)
	uchar		skip_adj;			// 補正省略
	ulong		delay;				// 遅延時間
	NTP_log		log;				// 1件分ログ
	struct clk_rec	tmpClk;
	ushort		tmpMsec;
	short		lag_count;			// 時刻補正調整カウンタ(msec)
	char		tmp[38];
} t_SYSTIME_ADJ;
extern t_SYSTIME_ADJ	SysTimeAdj;
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
/*--------------------------------------------------------------------------*/
/* 長期駐車検出機能関連														*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ushort		time1;// 検出時間1 設定情報
	ushort		time2;// 検出時間2 設定情報
	ushort		cng_count;// 車室検索用のカウンタ
	uchar		f_prm_cng;// 設定変更検出フラグ
	
} t_longpark_prm_cng;
// 長期駐車データ使用時、途中で設定が変更された時、現在発生している長期駐車を一旦解除し
// 新しい設定での長期駐車状態に更新するための情報
extern t_longpark_prm_cng LongPark_Prmcng;
#define	LONG_PARK_TIME1_DEF		48	// 検出時間1のデフォルト(03-0135③④⑤⑥=0000の時048で動作させるための設定値)
// 長期駐車チェック(アラームデータ検出)でドア閉時にセンターに対して送信する処理の判定用
extern ushort LongParkCheck_resend_count;// 車室検索用のカウンタ
extern uchar f_LongParkCheck_resend;// 判定フラグ
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
// MH321800(S) Y.Tanizaki ICクレジット対応
extern uchar f_ec_reader_waiting_wakeup;
// MH321800(E) Y.Tanizaki ICクレジット対応
// GG120600(S) // Phase9 設定変更通知対応
extern void mnt_SetFtpFlag( eFTP_REQ_TYPE req );
extern eFTP_REQ_TYPE mnt_GetRemoteFtpFlag( void );
extern	uchar	remotedl_work_update_get( void );
// GG120600(E) // Phase9 設定変更通知対応

// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
typedef struct {
	ushort				id;					// QRコードID
	ushort				rev;				// QRコードフォーマットRev.
	ushort				enc_type;			// QRコードエンコードタイプ
	ushort				info_size;			// QRデータ情報(パースデータ)サイズ
	lcdbm_rsp_QR_com_u	QR_data;			// 買物金額～予備
// GG129000(S) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
	ushort				qr_type;			// QRコードフォーマットタイプ
// GG129000(E) H.Fujinaga 2023/01/10 ゲート式車番チケットレスシステム対応（個別共通フォーマットQR割引券対応）
} tBarcodeDetail;

typedef struct {
	tBarcodeDetail		Barcode;			// バーコード情報
	ushort	RowSize;						// 生データサイズ
	char	RowData[BAR_DATASIZE];			// 読取したバーコード生データ
} tMediaDetail;
// MH810100(S) Y.Yamaichi 2020/1/23 車番チケットレス(メンテナンス）
extern	tMediaDetail	MediaDetail;			// 割引媒体情報
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
// extern	stDiscount2_t	DiscountBackUpArea[ONL_MAX_DISC_NUM];
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6343 クラウド料金計算対応(センター問合せRev.No.2)
extern	void	lcdbm_QR_data_res( uchar result );
// MH810100(E) Y.Yamaichi 2020/1/23 車番チケットレス(メンテナンス）
	
#define BARCODE_USE_MAX			99			// 1認証操作中に使用可能なバーコード枚数
typedef	struct	{							// 1操作中の割引媒体重複チェック用データ
//	stDiscount		stDis[5];				// 掛売情報
	DATE_YMDHMS		Certif_tim;				// 認証確定時刻/取消時刻
//	ulong	ulAmount;						// 累積買物金額
//	uchar	ucDiscountMethod;				// 割引方法
	uchar	bar_count;						// バーコード使用枚数
// MH810103(S) R.Endo 2021/05/31 車番チケットレス フェーズ2.2 #5669 QR買上券レシート読込み枚数上限の判定不具合
	uchar	amount_count;					// QR買上券使用枚数
// MH810103(E) R.Endo 2021/05/31 車番チケットレス フェーズ2.2 #5669 QR買上券レシート読込み枚数上限の判定不具合
//	uchar	media_count;					// 割引媒体使用枚数
	tMediaDetail	data[BARCODE_USE_MAX];	// 1認証操作で使用した割引数
// MH810100(S) 2020/08/31 #4776 【連動評価指摘事項】精算中の停電後、復電時に送るQR取消データの値が一部欠落して「0」となっている（No.02-0052）
	struct{
		ulong			Query_ParkingNo;		// 問い合わせカード 駐車場№
		ushort			Query_Kind;				// 問い合わせカード 種別
		uchar			Query_CardNo[32];		// 問い合わせカード カード番号
		uchar			Passkind;				// 定期種別
		uchar			MemberKind;				// 会員種別
		ulong			InParkNo;				// 入庫駐車場№
		DATE_YMDHMS		InDate;					// 入庫年月日時分秒
	} QueryResult;
// MH810100(E) 2020/08/31 #4776 【連動評価指摘事項】精算中の停電後、復電時に送るQR取消データの値が一部欠落して「0」となっている（No.02-0052）
} t_UseMedia;
extern	t_UseMedia	g_UseMediaData;

// ■LCD_IF_受信ﾃﾞｰﾀ実体定義				// _INTINRAMに配置するのでinitializeは不要
// MH810100(S) Y.Yamauchi 2020/03/12 車番チケットレス(#4019 QRリーダバージョン確認画面で「→」押下すると、画面を切り替えるたびに取得した値が消える)
//typedef union {
typedef struct {
// MH810100(E) Y.Yamauchi 2020/03/12 車番チケットレス(#4019 QRリーダバージョン確認画面で「→」押下すると、画面を切り替えるたびに取得した値が消える)
	lcdbm_rsp_error_t			lcdbm_rsp_error;			// エラー通知
	lcdbm_rsp_notice_ope_t		lcdbm_rsp_notice_ope;		// 操作通知
	lcdbm_rsp_QR_ctrl_res_t		lcdbm_rsp_QR_ctrl_res;		// QRﾘｰﾀﾞ制御応答(ﾒﾝﾃﾅﾝｽ)
	lcdbm_rsp_QR_rd_rslt_t lcdbm_rsp_QR_rd_rslt;			// QR読取結果(ﾒﾝﾃﾅﾝｽ)
	lcdbm_rsp_rt_con_rslt_t		lcdbm_rsp_rt_con_rslt;		// ﾘｱﾙﾀｲﾑ通信疎通結果(ﾒﾝﾃﾅﾝｽ)
	lcdbm_rsp_dc_con_rslt_t		lcdbm_rsp_dc_con_rslt;		// DC-NET通信疎通結果(ﾒﾝﾃﾅﾝｽ)
	lcdbm_lcd_disconnect_t		lcdbm_lcd_disconnect;		// LCDBM_OPCD_RESET_NOT/LCD_COMMUNICATION_CUTTING受信時の共通ﾒｯｾｰｼﾞ	LCD_DISCONNECT
} LcdRecv_t;

// ■LCD_IF_送信ﾃﾞｰﾀ実体定義				// _INTINRAMに配置するのでinitializeは不要
typedef union {
	ushort 						lcdbm_LCDbrightness;		// LCD輝度設定					PKTcmd_brightness()
	uchar 						lcdbm_audio_volume;			// ｱﾅｳﾝｽ音量設定				PKTcmd_audio_volume()
	lcdbm_cmd_audio_start_t 	lcdbm_cmd_audio_start;		// ｱﾅｳﾝｽ開始要求				PKTcmd_audio_start()
	uchar 						lcdbm_audio_stop_channel;	// ｱﾅｳﾝｽ終了要求(放送終了ﾁｬﾈﾙ)	PKTcmd_audio_end()
	uchar 						lcdbm_audio_stop_method;	// ｱﾅｳﾝｽ終了要求(中断方法)		PKTcmd_audio_end()
	uchar 						lcdbm_BUZ_volume;			// ﾌﾞｻﾞｰ音量設定				PKTcmd_beep_volume()
	uchar 						lcdbm_BUZ_beep;				// ﾌﾞｻﾞｰ鳴動要求				PKTcmd_beep_start()
// MH810103(s) 電子マネー対応 営休業通知修正
//	uchar 						lcdbm_opn_cls;				// 営休業通知					PKTcmd_notice_opn()
//	uchar 						lcdbm_opn_cls_reason;		// 休業理由ｺｰﾄﾞ					PKTcmd_notice_opn()
	lcdbm_cmd_notice_opn_t		lcdbm_opn_cls;				// 営休業通知					PKTcmd_notice_opn()
// MH810103(e) 電子マネー対応 営休業通知修正
	lcdbm_rsp_notice_ope_t		lcdbm_cmd_notice_ope;		// 操作通知						PKTcmd_notice_ope()
} LcdSendUnion_t;

extern	LcdRecv_t				LcdRecv;
extern	LcdSendUnion_t			LcdSend;

extern	lcdbm_rsp_in_car_info_t	lcdbm_rsp_in_car_info_recv;			// 入庫情報(ｶｰﾄﾞ情報)PKTtaskでﾃﾞｰﾀをsetする
extern	lcdbm_rsp_in_car_info_t	lcdbm_rsp_in_car_info_main;			// 入庫情報(ｶｰﾄﾞ情報)
extern	lcdbm_rsp_QR_data_t		lcdbm_rsp_QR_data_recv;				// QRﾃﾞｰﾀ=PKTtaskでﾃﾞｰﾀをsetする	LCD_QR_DATA
extern	unsigned char			lcdbm_ICC_Settlement_Status;		// 決済状態ﾃﾞｰﾀ					LCD_ICC_SETTLEMENT_STS
extern	unsigned char			lcdbm_ICC_Settlement_Result;		// 決済結果情報					LCD_ICC_SETTLEMENT_RSLT
extern	uchar					lcdbm_Flag_ResetRequest;			// ﾘｾｯﾄ要求有無判定ﾌﾗｸﾞ(op_mod02の状態でリセット通知/起動通知で<- ON)
extern	uchar					lcdbm_Flag_QRuse_ICCuse;			// QR_利用中, 利用確認中ﾌﾗｸﾞ(QRﾘｰﾀﾞ利用, ICｸﾚｼﾞｯﾄ利用の排他制御)
extern	uchar					lcdbm_Counter_QR_StartStopRetry;	// QRﾘｰﾀﾞ開始/停止応答OK待ちﾘﾄﾗｲｶｳﾝﾀ
extern	uchar					lcdbm_Last_QR_StartStopInfo;		// QRﾘｰﾀﾞ開始/停止応答の最後に送信した応答(開始応答=0/停止応答=1)
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
extern	uchar					lcdbm_Flag_RemotePay_ICCuse;		// 決済リーダ停止確認フラグ(遠隔精算)
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
//////////////////////////////////////////////////////////////////////////
extern	stDiscount_t			m_stDisc;							// DISCOUNT_DATA3 + DiscParkNo
extern	short					m_nDiscPage;						// stDiscountInfo[]のindex(0-24)
extern	uchar					lcdbm_QRans_InCar_status;			// 
extern	ulong					g_OpeSequenceID;					// ID(入庫から精算完了までを管理するID)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// MH810100(S) 2020/08/28 #4780 種別切替QR適用後に現金を投入し、QR割引券を読み取ると割引が適用されない
extern	uchar					g_checkKirikae;								// 種別切替チェック済みフラグ
// MH810100(E) 2020/08/28 #4780 種別切替QR適用後に現金を投入し、QR割引券を読み取ると割引が適用されない
// MH810100(S) 2020/09/09 #4820 【連動評価指摘事項】ICクレジット精算完了直後に停電→復電させた場合、リアルタイム精算データの精算処理の年月日時分秒が反映されずに送信される（No.83）
extern struct clk_rec			g_PayStartTime;			// 精算開始入時刻(精算が開始された時の現在時刻)
// MH810100(E) 2020/09/09 #4820 【連動評価指摘事項】ICクレジット精算完了直後に停電→復電させた場合、リアルタイム精算データの精算処理の年月日時分秒が反映されずに送信される（No.83）
// GG124100(S) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応
extern	uchar					g_calcMode;							// 料金計算モード(0:通常料金計算モード/1:クラウド料金計算モード)
// GG124100(E) R.Endo 2022/06/02 車番チケットレス3.0 #6343 クラウド料金計算対応
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
extern	ulong					LaneStsSeqNo;						// レーンモニタデータの状態一連番号
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129000(S) H.Fujinaga 2023/02/24 ゲート式車番チケットレスシステム対応（遠隔精算対応）
extern	uchar	PayInfoChange_StateCheck(void);
// GG129000(E) H.Fujinaga 2023/02/24 ゲート式車番チケットレスシステム対応（遠隔精算対応）

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（遠隔と端末間通信のバッファを別にする）
// 標準の端末間通信では遠隔のバッファを使用しているが、併用可能にするためバッファを別にする
// サイズは遠隔と同じとする
#define	NTNET_LOG_WORK_SIZE		20000							// NT-NET端末間通信用ログデータ
extern uchar	NTNET_LogData[NTNET_LOG_WORK_SIZE];
extern uchar	NTNET_NtLogData[NTNET_LOG_WORK_SIZE];
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（遠隔と端末間通信のバッファを別にする）

#endif	/* ___MEM_DEFH___ */

