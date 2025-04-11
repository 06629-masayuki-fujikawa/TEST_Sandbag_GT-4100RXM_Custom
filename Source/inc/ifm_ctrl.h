#ifndef	_IFM_CTRL_H_
#define	_IFM_CTRL_H_

#include	"mem_def.h"

/*[]----------------------------------------------------------------------[]*/
/*| PARKiPRO対応                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : ART                                                      |*/
/*| Date        : 2007-02-28                                               |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/

#define IFM_DATA_MAX		250

#pragma pack
/*--------------------------------------------------------------------------*/
/*	IFM電文基本形式															*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ushort	mlen;
	uchar	ID[2];
	uchar	data[1];
} tIFMMSG;

/*--------------------------------------------------------------------------*/
/*	基本ﾃﾞｰﾀ(駐車場№～予約)												*/
/*--------------------------------------------------------------------------*/
// MH810100(S) K.Onodera 2020/01/07 車番チケットレス(機種コード)
//#define MODEL_CODE_FT4000	"0220"
#define MODEL_CODE_GT4100	"0581"
// MH810100(E) K.Onodera 2020/01/07 車番チケットレス(機種コード)

typedef	struct {
	uchar			ParkingNo[4];		// 駐車場№
	uchar			MachineNo[2];		// 機械№
	uchar			ModelCode[4];		// 機種コード
	uchar			Rslt[2];			// 予約(応答時のみ)
} tIFMMSG_BASIC;

// 車室情報
typedef struct {
	uchar			Area[2];			// 区画
	uchar			No[4];				// 車室番号
} t_ShedInfo;

/*----------*/
/*	要求用　*/
/*----------*/

/*--------------------------------------------------------------------------*/
/*	データ要求(データ種別30/31)												*/
/*--------------------------------------------------------------------------*/
/*--------------------*/
/*	要求系サブ構造体　*/
/*--------------------*/
// データ要求１
typedef struct {
	uchar			ReqID[2];			// 要求データID
	t_ShedInfo		CarInfo;			// 車室情報
	uchar			Reserve[14];		// 予備
} tREQ30;

// 現在時刻変更要求
typedef struct {
	uchar			Year[4];			// 年
	uchar			Mon[2];				// 月
	uchar			Day[2];				// 日
	uchar			Hour[2];			// 時
	uchar			Min[2];				// 分
	uchar			Sec[2];				// 秒
	uchar			Reserve[8];			// 予備
} tREQ40;

// フラップ・ロック制御要求
typedef struct {
	uchar			Reserve1[2];		// 予備１
	t_ShedInfo		CarInfo;			// 車室情報
	uchar			ProcMode[2];		// 処理区分
	uchar			Reserve2[12];		// 予備２
} tREQ41;

// 受付券発行要求
typedef struct {
	uchar			Reserve1[2];		// 予備１
	t_ShedInfo		CarInfo;			// 車室情報
	uchar			Reserve2[14];		// 予備２
} tREQ42;

// 振替精算実行可能チェック・振替精算要求
typedef struct {
	uchar			Reserve1[2];		// 予備１
	t_ShedInfo		CarInfo1;			// 振替元車室情報
	t_ShedInfo		CarInfo2;			// 振替先車室情報
	uchar			Reserve2[1];		// 予約
	uchar			PassUse;			// 定期利用有無（ID45には存在しない）
	uchar			Reserve3[6];		// 予備
} tREQ46;

typedef	struct {
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// 基本データ
										// ※応答は基本データの予約部に結果が入る
	union {
		tREQ30		r30;
		tREQ40		r40;
		tREQ41		r41;
		tREQ42		r42;
		tREQ46		r45;
		tREQ46		r46;
	} req;
} tIFMMSG_REQ;

typedef	tIFMMSG_REQ	tIFMMSG30;
typedef	tIFMMSG_REQ	tIFMMSG40;
typedef	tIFMMSG_REQ	tIFMMSG41;
typedef	tIFMMSG_REQ	tIFMMSG42;
typedef	tIFMMSG_REQ	tIFMMSG45;
typedef	tIFMMSG_REQ	tIFMMSG46;

/*--------------------------------------------------------------------------*/
/*	領収証再発行要求(データ種別43)											*/
/*--------------------------------------------------------------------------*/
// 精算情報
typedef struct {
	uchar			Method[2];			// 精算方法
	uchar			PayClass[2];		// 処理区分
	uchar			Mode[2];			// 精算モード
	uchar			OutKind[2];			// 精算出庫
	uchar			Receipt;			// 領収証出力
	uchar			Empty;				// 空き
	uchar			PayCount[6];		// 精算／精算中止追番
	uchar			tmPay[8];			// 精算時刻（MMDDHHMM）
	t_ShedInfo		CarInfo;			// 車室情報
	uchar			ParkingNo[4];		// 定期券駐車場番号
	uchar			PassKind[2];		// 定期券種別
	uchar			PassID[6];			// 定期券ID（契約番号）
	uchar			AntiPassChk;		// アンチパスチェック
	uchar			StockCnt;			// 在庫カウント
	uchar			FareKind[2];		// 料金種別
	uchar			tmEnter[8];			// 入庫時刻
	uchar			Fare[6];			// 駐車料金
} t_PayInfo;

// 金額情報
typedef struct {
	uchar			Cash[6];			// 現金売上げ額
	uchar			Entered[6];			// 投入金額
	uchar			Change[6];			// 釣銭額
	uchar			Shortage[6];		// 払出し不足額
	uchar			CreditCard[6];		// クレジットカード利用金額
} t_SumInfo;

typedef	struct {
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// 基本データ
	uchar			Serial[2];			// データ追番
	t_PayInfo		PayInfo;			// 精算情報
	t_SumInfo		SumInfo;			// 金額情報
	uchar			dummy[10];			// 予約＆予備
} tIFMMSG43;

/*--------------------------------------------------------------------------*/
/*	減額精算要求(データ種別44)												*/
/*--------------------------------------------------------------------------*/
// 金額情報２
typedef struct {
	uchar			Rest[6];			// 支払残額
	uchar			Entered[6];			// 投入金額
	uchar			Change[6];			// 釣銭額
	uchar			Shortage[6];		// 払出し不足額
	uchar			Discount[6];		// 駐車料金割引額
} t_SumInfo2;

typedef	struct {
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// 基本データ
	uchar			SerNo[2];			// データ追番
	t_PayInfo		PayInfo;			// 精算情報
	t_SumInfo2		SumInfo;			// 金額情報
	uchar			dummy[10];			// 予約＆予備
} tIFMMSG44;

/*--------------------------------------------------------------------------*/
/*	IBKエラー通知(データ種別B0)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ushort			mlen;
	uchar			ID[2];

	uchar			MachineNo[2];		// 機械№		(="01")
	uchar			ModelCode[2];		// 機種コード	(="00")
	uchar			ModuleCode[2];		// モジュールコード	(="55" or "12")
	uchar			ErrorCode[2];		// エラーコード	(="01"～"99")
	uchar			ErrorOccur[2];		// 発生／解除	(="01"～"03")
	uchar			dummy[8];
} tIFMMSGB0;

/*----------*/
/*	応答用	*/
/*----------*/

/*--------------------------------------------------------------------------*/
/*	IBKメモリクリア要求(データ種別57)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ushort			mlen;
	uchar			ID[2];

	uchar			dummy1[3];			// 予約
	uchar			req;				// 通知（='2'～IBKイニシャライズのみ）
	uchar			dummy2;				// 予約
} tIFMMSG57;

/*--------------------------------------------------------------------------*/
/*	データ要求NG(データ種別60/61)											*/
/*--------------------------------------------------------------------------*/
typedef	tIFMMSG_REQ	tIFMMSG60;

enum {
	_IFM_ANS_SUCCESS,
	_IFM_ANS_REFUSED,
	_IFM_ANS_INVALID_PARAM,
	_IFM_ANS_FROM_NG,
	_IFM_ANS_TO_NG,
	_IFM_ANS_NOT_FOUND,
	_IFM_ANS_IMPOSSIBLE,
	_IFM_ANS_NOW_BUSY,
	_IFM_ANS_reserved2,
	_IFM_ANS_OTHER_ERR,
	_IFM_ANS_DONE = 20,
	_IFM_ANS_CANCEL,
	_IFM_ANS_ABNORMAL
};

/*--------------------------------------------------------------------------*/
/*	駐車場情報(データ種別62)												*/
/*--------------------------------------------------------------------------*/
typedef struct {
	uchar			CurTime[14];		// 精算機現在時刻データ
	uchar			Stat;				// 営休業状況
	uchar			Mode;				// 精算機動作モード状況
	uchar			Err;				// エラー発生状況
	uchar			Alarm;				// アラーム発生状況
	uchar			Full1;				// 満車１状況
	uchar			Full2;				// 満車２状況
	uchar			Full3;				// 満車３状況
	uchar			Reserve1;			// 予備１
	uchar			Reserve2[7];		// 予備２
	uchar			ShedNum[3];			// 車室数
} tM_STATUS;

typedef struct {
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// 基本データ
	tM_STATUS		Stat;				// 精算機情報
	t_ShedInfo		CarInfo[324];		// 車室情報
	uchar			Reserve[6];			// 予備
} tIFMMSG62;

/*--------------------------------------------------------------------------*/
/*	駐車場情報・小(データ種別63)											*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// 基本データ
	tM_STATUS		Stat;				// 精算機情報
	uchar			Reserve[6];			// 予備
} tIFMMSG63;

/*--------------------------------------------------------------------------*/
/*	全車室状況(データ種別64)												*/
/*--------------------------------------------------------------------------*/
typedef struct {
	uchar			Area[2];			// 区画
	uchar			No[4];				// 車室番号
	uchar			Mode;				// 処理モード
	uchar			Stat1;				// ステータス１
	uchar			Stat2;				// ステータス２
	uchar			Stat3;				// ステータス３
	uchar			ParkTime[8];		// 入庫時刻（MMDDHHMM）
} t_ShedStat;

typedef struct {
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// 基本データ
	tM_STATUS		Stat;				// 精算機情報
	t_ShedStat		ShedStat[324];		// 車室状況
	uchar			Reserve[6];			// 予備
} tIFMMSG64;

/*--------------------------------------------------------------------------*/
/*	１車室状況(データ種別65)												*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// 基本データ
	tM_STATUS		Stat;				// 精算機情報
	t_ShedStat		ShedStat;			// 車室状況
	uchar			Reserve[12];		// 予備
} tIFMMSG65;

/*--------------------------------------------------------------------------*/
/*	精算状況・精算データ(データ種別66/67)									*/
/*--------------------------------------------------------------------------*/
// 割引詳細
typedef struct {
	uchar			ParkingNo[3];		// 駐車場№
	uchar			kind[3];			// 割引種別
	uchar			div[3];				// 割引区分
	uchar			used[3];			// 使用枚数
	uchar			sum[6];				// 割引額
	uchar			inf1[6];			// 割引情報１
	uchar			inf2[6];			// 割引情報２
} t_DiscountInfo;

typedef struct {
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// 基本データ
	uchar			Serial[2];			// 精算処理状態／データ追番
	t_PayInfo		PayInfo;			// 精算情報
	t_SumInfo		SumInfo;			// 金額情報
	t_DiscountInfo	DscntInfo[25];		// 割引詳細
	uchar			Reserve[4];			// 予備
} tIFMMSG_PAY;

typedef	tIFMMSG_PAY	tIFMMSG66;
typedef	tIFMMSG_PAY	tIFMMSG67;

/*--------------------------------------------------------------------------*/
/*	エラーデータ(データ種別68)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {	// error data
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// 基本データ
	uchar			Reserve1[2];		// 予備１
	uchar			ModuleCode[4];		// モジュールコード
	uchar			ErrCode[4];			// エラーコード
	uchar			Stat[2];			// 発生＆解除
	uchar			Time[8];			// 月日時分
	uchar			Reserve2[2];		// 予備２
} tIFMMSG68;

/*--------------------------------------------------------------------------*/
/*	応答汎用(データ種別70～74)												*/
/*--------------------------------------------------------------------------*/
typedef	tIFMMSG40	tIFMMSG70;
typedef	tIFMMSG41	tIFMMSG71;
typedef	tIFMMSG42	tIFMMSG72;
typedef	tIFMMSG43	tIFMMSG73;
typedef	tIFMMSG44	tIFMMSG74;

/*--------------------------------------------------------------------------*/
/*	振替精算チェック(データ種別75/76)										*/
/*--------------------------------------------------------------------------*/
typedef struct {
	uchar			FailedPaid[6];		// 振替元支払額
	uchar			Switchable[6];		// 振替可能額
	uchar			OrgFare[6];			// 振替先駐車料金
	uchar			OrgArrear[6];		// 振替先支払残額
	uchar			ActArrear[6];		// 振替後支払残額
	
} tSWP_INFO;

typedef	struct {	// error data
	ushort			mlen;
	uchar			ID[2];
	
	tIFMMSG_BASIC	Basic;				// 基本データ
	uchar			PayMode[2];			// 精算処理状態
	t_ShedInfo		CarInfo1;			// 振替元車室情報
	t_ShedInfo		CarInfo2;			// 振替先車室情報
	uchar			Reserve1;			// 予約
	uchar			PassUse;			// 定期利用有無
	tSWP_INFO		SwInfo;				// 振替金額情報
	uchar			Reserve2[8];		// 予備
} tIFMMSG_SWP;

typedef	tIFMMSG_SWP	tIFMMSG75;
typedef	tIFMMSG_SWP	tIFMMSG76;


#pragma unpack

/*--------------------------------------------------------------------------*/
/*	受信バッファ															*/
/*--------------------------------------------------------------------------*/
typedef	union {
	tIFMMSG		buff;
	tIFMMSG30	msg30;
	tIFMMSG40	msg40;
	tIFMMSG41	msg41;
	tIFMMSG42	msg42;
	tIFMMSG43	msg43;
	tIFMMSG44	msg44;
	tIFMMSG45	msg45;
	tIFMMSG46	msg46;
	tIFMMSGB0	msgB0;
	char		dummy[sizeof(tIFMMSG)-1+IFM_DATA_MAX];
} tIFM_RCV_BUF;

/*--------------------------------------------------------------------------*/
/*	送信キュー																*/
/*--------------------------------------------------------------------------*/
typedef	union {
	tIFMMSG		buff;
	tIFMMSG67	msg67;			// 精算データ
	tIFMMSG68	msg68;			// エラーデータ
	tIFMMSG57	msg57;
	tIFMMSG74	msg74;			// 減額精算要求応答
	tIFMMSG76	msg76;			// 振替精算応答
	tIFMMSG60	msg60;			// データ要求NG
	tIFMMSG62	msg62;			// 駐車場情報
	tIFMMSG63	msg63;			// 駐車場情報（小）
	tIFMMSG64	msg64;			// 全車室情報
	tIFMMSG65	msg65;			// １車室情報
	tIFMMSG66	msg66;			// 精算処理状況
	tIFMMSG70	msg70;			// 現在時刻変更応答
	tIFMMSG71	msg71;			// フラップロック制御応答
	tIFMMSG72	msg72;			// 受付券発行応答
	tIFMMSG73	msg73;			// 領収証発行応答
	tIFMMSG75	msg75;			// 振替精算事前チェック応答
} tIFM_SND_BUF;

typedef	struct {
	ushort		blk_num;			// ブロック数
	ushort		last_blk_size;		// 最終ブロックサイズ
	ushort		lb_revised_size;	// 最終ブロック補正サイズ
	ushort		snd_blk;
//	ushort		src_que;
} tIFM_SND_CTR;

/*--------------------------------------------------------------------------*/
/*	精算データテーブル														*/
/*--------------------------------------------------------------------------*/
typedef struct {
	unsigned char	COUNT;						// 電文数(０～２０)
	unsigned char	RIND;						// 最古電文位置(０～１９)
	unsigned char	CIND;						// 書き込み電文位置(０～１９)
	struct {
		unsigned short	LEN;					// 電文長(ＳＴＸ～ＢＣＣ)
		unsigned char	BUFF[259 * 4];			// データバッファ(２５９×４)
	} DATA[20];									// ２０件
} t_PIP_PayTable;								// 精算データテーブル

/*--------------------------------------------------------------------------*/
/*	エラーデータテーブル													*/
/*--------------------------------------------------------------------------*/
typedef struct {
	unsigned char	COUNT;						// 電文数(０～２０)
	unsigned char	RIND;						// 最古電文位置(０～１９)
	unsigned char	CIND;						// 書き込み電文位置(０～１９)
	struct {
		unsigned short	LEN;					// 電文長(ＳＴＸ～ＢＣＣ)
		unsigned char	BUFF[67];				// データバッファ
	} ERROR[20];								// フラップ・ロック式(２０件)
} t_PIP_ErrTable;								// エラーデータテーブル
extern	t_PIP_PayTable	PAYTBL;		// 精算データテーブル
extern	t_PIP_ErrTable	ERRTBL;		// エラーデータテーブル

/*--------------------------------------------------------------------------*/
/*	プロトタイプ															*/
/*--------------------------------------------------------------------------*/
void	IFM_Init(int clr);
void	IFM_Snd_ErrorClear(void);
void	IFM_Snd_Error(void);
extern	void	IFM_Snd_Payment(const Receipt_data *dat);
extern	void	IFM_Accumulate_Payment_Init();
extern	void	IFM_Accumulate_Error_Init();
void	IFM_Snd_AccumulateError(uchar count);
void	IFM_Snd_AccumulatePayment(uchar count);
void	IFM_Snd_GengakuAns(int sts);
void	IFM_Snd_FurikaeAns(int sts, struct VL_FRS *frs);

extern	ushort	pipcomm;
#define	_is_pip()		(pipcomm)
#define	_is_not_pip()	(!pipcomm)
// MH322914(S) K.Onodera 2016/10/12 AI-V対応
#define	_is_Normal_pip()		(pipcomm == 1)
#define	_is_ParkingWeb_pip()	(pipcomm == 2)
// MH322914(E) K.Onodera 2016/10/12 AI-V対応

#endif
