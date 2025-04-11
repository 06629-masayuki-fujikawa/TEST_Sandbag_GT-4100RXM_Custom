#ifndef	___NTNET_DEFH___
#define	___NTNET_DEFH___
/*[]----------------------------------------------------------------------[]*/
/*| NT-NET関連ﾃﾞｰﾀ定義                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  R.Hara                                                  |*/
/*| Date        :  2005-08-05                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include "mem_def.h"
#include "rkn_cal.h"
#include "ope_def.h"
#include "ntnet.h"

#pragma pack

// MH810100(S) K.Onodera 2019/12/25 車番チケットレス(仮)
//#define		NTNET_MODEL_CODE		220
#define		NTNET_MODEL_CODE		581
// MH810100(E) K.Onodera 2019/12/25 車番チケットレス(仮)

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
#define	NTBUF_NEARFULL_COUNT			6
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

/*--------------------------------------------------------------------------*/
/*	車室問合せﾃﾞｰﾀ(ﾃﾞｰﾀ種別01)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			LockNo;											// 区画情報
} DATA_KIND_01;


/*--------------------------------------------------------------------------*/
/*	車室問合せ結果ﾃﾞｰﾀ(ﾃﾞｰﾀ種別02)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			LockNo;											// 区画情報
	ushort			Answer;											// 結果
//	flp_com			LkInfo;											// 車室情報
	uchar			LkInfo[48];										// 車室情報(flp_com構造体から最後の時間のﾒﾝﾊﾞｰ4ﾊﾞｲﾄ分引いた値)
	ushort			Yobi;
	ulong			PpcData;
	ushort			SvsTiket[15];
	ushort			KakeTiket[10];
	ushort			Rsv[20];										// 予備
} DATA_KIND_02;


/*--------------------------------------------------------------------------*/
/*	車室問合せ結果NGﾃﾞｰﾀ(ﾃﾞｰﾀ種別03)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			LockNo;											// 区画情報
	ushort			Answer;											// 結果
} DATA_KIND_03;


/*--------------------------------------------------------------------------*/
/*	ﾛｯｸ制御ﾃﾞｰﾀ(ﾃﾞｰﾀ種別04)													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			LockNo;											// 区画情報
	ushort			MoveMode;										// 処理区分
} DATA_KIND_04;


/*--------------------------------------------------------------------------*/
/*	ﾛｯｸ制御結果ﾃﾞｰﾀ(ﾃﾞｰﾀ種別05)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			LockNo;											// 区画情報
	ushort			LockSense;										// 車両検知状態
	ushort			LockState;										// ﾛｯｸ装置状態
} DATA_KIND_05;


/*--------------------------------------------------------------------------*/
/*	簡易車室情報ﾃｰﾌﾞﾙ(ﾃﾞｰﾀ種別12)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ulong			LockNo;											// 区画情報
	ushort			NowState;										// 現在ｽﾃｰﾀｽ
	ushort			Year;											// 入庫年
	uchar			Mont;											// 入庫月
	uchar			Date;											// 入庫日
	uchar			Hour;											// 入庫時
	uchar			Minu;											// 入庫分
	ushort			Syubet;											// 料金種別
	ulong			TyuRyo;											// 駐車料金
} LOCK_STATE;

typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			SMachineNo;										// 送信先端末機械№
	LOCK_STATE		LockState[OLD_LOCK_MAX];						// 車室情報
} DATA_KIND_12;


/*--------------------------------------------------------------------------*/
/*	入庫ﾃﾞｰﾀ(ﾃﾞｰﾀ種別20)													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ulong			ParkingNo;										// 定期券駐車場№
	ulong			PassID;											// 定期券ID
	uchar			Contract[8];									// 定期券契約№
	ushort			Syubet;											// 定期券種別
	ushort			State;											// 定期券ｽﾃｰﾀｽ
	ulong			Reserve1;										// 予備
	ulong			Reserve2;										// 予備
	ushort			MoveMode;										// 定期券回収／返却
	ushort			ReadMode;										// 定期券ﾘｰﾄﾞﾗｲﾄ／ﾘｰﾄﾞｵﾝﾘｰ
	ushort			SYear;											// 定期券開始年
	uchar			SMon;											// 定期券開始月
	uchar			SDate;											// 定期券開始日
	ushort			EYear;											// 定期券終了年
	uchar			EMon;											// 定期券終了月
	uchar			EDate;											// 定期券終了日
} PASS_DATA;

typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			InCount;										// 入庫追い番
	ushort			Syubet;											// 処理区分
	ushort			InMode;											// 入庫ﾓｰﾄﾞ
	ulong			LockNo;											// 区画情報
	ushort			CardType;										// 駐車券ﾀｲﾌﾟ
	ushort			CMachineNo;										// 駐車券機械№
	ulong			CardNo;											// 駐車券番号(発券追い番)
	date_time_rec2	InTime;											// 入庫年月日時分秒
	ushort			PassCheck;										// ｱﾝﾁﾊﾟｽﾁｪｯｸ
	ushort			CountSet;										// 在車ｶｳﾝﾄ
	PASS_DATA		PassData;										// 定期券ﾃﾞｰﾀ
	ushort			Dummy[32];										// ﾀﾞﾐｰ
} DATA_KIND_20;


/*--------------------------------------------------------------------------*/
/*	精算ﾃﾞｰﾀ(ﾃﾞｰﾀ種別22,23)													*/
/*--------------------------------------------------------------------------*/
#define		NTNET_DIC_MAX	25										// 割引数
#define		NTNET_SVS_M		1										// 割引種別 ｻｰﾋﾞｽ券(料金)
#define		NTNET_SVS_T		101										// 割引種別 ｻｰﾋﾞｽ券(時間)
#define		NTNET_KAK_M		2										// 割引種別 店割引(料金)
#define		NTNET_KAK_T		102										// 割引種別 店割引(時間)
#define		NTNET_TKAK_M	3										// 割引種別 多店舗(料金)
#define		NTNET_TKAK_T	103										// 割引種別 多店舗(時間)
#define		NTNET_WRI_M		4										// 割引種別 割引券(料金)
#define		NTNET_WRI_T		104										// 割引種別 割引券(時間)
#define		NTNET_FRE		5										// 割引種別 回数券
#define		NTNET_PRI_W		11										// 割引種別 割引ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ
#define		NTNET_PRI_S		12										// 割引種別 販売ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ
#define		NTNET_WRIKEY	40										// 割引種別 割引ｷｰ
#define		NTNET_KWRIKEY	41										// 割引種別 店割引ｷｰ
#define		NTNET_SYUBET	50										// 割引種別 種別割引
#define		NTNET_SUICA_1	31										// 割引種別 Suicaｶｰﾄﾞ番号
#define		NTNET_SUICA_2	32										// 割引種別 Suica支払額、残額
#define		NTNET_PASMO_0	33										// 割引種別 PASMO決済（集計用）
#define		NTNET_PASMO_1	35										// 割引種別 PASMOｶｰﾄﾞ番号
#define		NTNET_PASMO_2	36										// 割引種別 PASMO支払額、残額
#define		NTNET_EDY_0		32										// 割引種別 Edy集計ﾃﾞｰﾀ
#define		NTNET_EDY_1		33										// 割引種別 Edyｶｰﾄﾞ番号
#define		NTNET_EDY_2		34										// 割引種別 Edy支払額、残額
#define		NTNET_GENGAKU	90										// 割引種別 減額精算
#define		NTNET_FURIKAE	91										// 割引種別 振替精算
#define		NTNET_ICOCA_0	35										// 割引種別 ICOCAｶｰﾄﾞ番号(集計用)
#define		NTNET_ICOCA_1	61										// 割引種別 ICOCAｶｰﾄﾞ番号
#define		NTNET_ICOCA_2	62										// 割引種別 ICOCA支払額、残額
#define		NTNET_ICCARD_0	36										// 割引種別 IC-Cardｶｰﾄﾞ番号(集計用)
#define		NTNET_ICCARD_1	63										// 割引種別 IC-Cardｶｰﾄﾞ番号
#define		NTNET_ICCARD_2	64										// 割引種別 IC-Card支払額、残額
#define		NTNET_TCARD_0	38
#define		NTNET_TCARD_1	67
#define		NTNET_TCARD_2	68
// MH321800(S) G.So ICクレジット対応
#define		NTNET_WAON_0	37										// 割引種別 WAON（集計用）
#define		NTNET_WAON_1	65										// 割引種別 WAONｶｰﾄﾞ番号
#define		NTNET_WAON_2	66										// 割引種別 WAON支払額、残額

#define		NTNET_SAPICA_0	82										// 割引種別 SAPICA（集計用）
#define		NTNET_SAPICA_1	83										// 割引種別 SAPICAｶｰﾄﾞ番号
#define		NTNET_SAPICA_2	84										// 割引種別 SAPICA支払額、残額

#define		NTNET_NANACO_0	80										// 割引種別 nanaco（集計用）
#define		NTNET_NANACO_1	85										// 割引種別 nanacoｶｰﾄﾞ番号
#define		NTNET_NANACO_2	86										// 割引種別 nanaco支払額、残額

#define		NTNET_ID_0		34										// 割引種別 iD（集計用）
#define		NTNET_ID_1		37										// 割引種別 iDｶｰﾄﾞ番号
#define		NTNET_ID_2		38										// 割引種別 iD支払額、残額

#define		NTNET_QUICPAY_0	83										// 割引種別 QUICPay（集計用）
#define		NTNET_QUICPAY_1	81										// 割引種別 QUICPayｶｰﾄﾞ番号
#define		NTNET_QUICPAY_2	82										// 割引種別 QUICPay支払額、残額
// MH321800(E) G.So ICクレジット対応
// MH810105(S) MH364301 PiTaPa対応
#define		NTNET_PITAPA_0	81										// 割引種別 PiTaPa（集計用）
#define		NTNET_PITAPA_1	87										// 割引種別 PiTaPaｶｰﾄﾞ番号
#define		NTNET_PITAPA_2	88										// 割引種別 PiTaPa支払額
// MH810105(E) MH364301 PiTaPa対応
// MH810105(S) MH364301 QRコード決済対応
#define 	NTNET_QR		110										// 割引種別 QR決済
// MH810105(E) MH364301 QRコード決済対応

#define		NTNET_SYUSEI_1	92										// 割引種別 修正精算
#define		NTNET_SYUSEI_2	93										// 割引種別 修正精算
#define		NTNET_SYUSEI_3	94										// 割引種別 修正精算
#define		NTNET_CSVS_M	21										// 割引種別 精算中止ｻｰﾋﾞｽ券(料金)
#define		NTNET_CSVS_T	22										// 割引種別 精算中止ｻｰﾋﾞｽ券(時間)
#define		NTNET_CKAK_M	23										// 割引種別 精算中止店割引(料金)
#define		NTNET_CKAK_T	24										// 割引種別 精算中止店割引(時間)
#define		NTNET_CWRI_M	25										// 割引種別 精算中止割引券(料金)
#define		NTNET_CWRI_T	26										// 割引種別 精算中止割引券(時間)
#define		NTNET_CFRE		27										// 割引種別 精算中止回数券

// 不具合修正(S) K.Onodera 2016/12/08 #1585 振替元割引金額に無関係の値がセットされる
#define		NTNET_SECTION_WARI_MAX	999
// 不具合修正(E) K.Onodera 2016/12/08 #1585 振替元割引金額に無関係の値がセットされる
// MH322914(S) K.Onodera 2016/09/15 AI-V対応：振替精算
// MH810105(S) MH364301 QRコード決済対応
#define		NTNET_DEEMED_PAY		1021							// 割引種別 みなし決済
#define		NTNET_MIRYO_PAY			1022							// 割引種別 未了支払済み
// MH810105(E) MH364301 QRコード決済対応
#define		NTNET_KABARAI			2030							// 割引種別 過払い(現金以外)
#define		NTNET_FURIKAE_2			2031							// 割引種別 振替精算
#define		NTNET_FURIKAE_DETAIL	2032
#define		NTNET_FUTURE			2033							// 割引種別 後日精算予定額
#define		NTNET_AFTER_PAY			2034							// 割引種別 後日精算額(未払い回収額)
// MH322914(E) K.Onodera 2016/09/15 AI-V対応：振替精算
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
#define		NTNET_KAIMONO_GOUKEI	2001	// 割引種別 買物金額合計
#define		NTNET_SYUBET_TIME		150		// 種別割引(時間)
#define		NTNET_SHOP_DISC_AMT		6		// 買物割引(金額)
#define		NTNET_SHOP_DISC_TIME	106		// 買物割引(時間)
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
// GG129004(S) M.Fujikawa 2024/10/22 買物金額割引情報対応
#define		NTNET_KAIMONO_INFO		2002	// 買物情報
// GG129004(E) M.Fujikawa 2024/10/22 買物金額割引情報対応
// MH321800(S) D.Inaba ICクレジット対応
#define		NTNET_INQUIRY_NUM		8000							// 割引種別 決済リーダ問合せ番号
// MH321800(E) D.Inaba ICクレジット対応
// MH810105(S) MH364301 QRコード決済対応
#define		NTNET_MCH_NO_TOP_HALF		8001						// 割引種別 QRコード決済 Mch取引番号上位半分
#define		NTNET_MCH_NO_LOWER_HALF		8002						// 割引種別 QRコード決済 Mch取引番号下位半分
#define		NTNET_PAYTERMINAL_NO		8003						// 割引種別 QRコード決済 支払端末ID
#define		NTNET_DEAL_NO				8004						// 割引種別 QRコード決済 取引番号
#define		NTNET_MIRYO_UNKNOWN			8021						// 割引種別 未了支払不明
#define		NTNET_ID_APPROVAL_NO		8101						// 割引種別 ID承認番号
#define		NTNET_QUIC_PAY_APPROVAL_NO	8102						// 割引種別 QUICPAY承認番号
#define		NTNET_PITAPA_APPROVAL_NO	8103						// 割引種別 PITAPA承認番号
// MH810105(E) MH364301 QRコード決済対応

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（精算データ変更）
#define		NTNET_MEITETU_IN_MONEY		9200							// 名鉄個別割引種別：入金
#define		NTNET_MEITETU_OUT_MONEY		9201							// 名鉄個別割引種別：出金

#define		NTNET_SHABANINFO_1			60100						// 車番情報1
#define		NTNET_SHABANINFO_2			60101						// 車番情報2
#define		NTNET_SHABANINFO_3			60102						// 車番情報3
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（精算データ変更）

typedef	struct {
	ulong			ParkingNo;										// 駐車場№
	ushort			DiscSyu;										// 割引種別
	uchar			CardNo[20];										// 電子マネーカードNo.
} DISCOUNT_DATA2;
extern	DISCOUNT_DATA2	wk_DicData;

// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
typedef struct {
	ulong			ParkingNo;			// 駐車場№
	ushort			DiscSyu;			// 割引種別
	ushort			DiscNo;				// 割引区分
	ushort			DiscCount;			// 枚数
	ulong			Discount;			// 割引額(割引時間)
	ulong			DiscInfo1;			// 割引情報1
	union {
		struct {						// プリペイドカード以外の構造体
			ulong			DiscInfo2;	// 割引情報2
			ushort			MoveMode;	// 回収／返却
			ushort			DiscFlg;	// 割引済み
		} common;
		struct {						// プリペイドカード用構造体
			ulong			pay_befor;	// 支払前残額
			ushort			kigen_year;	// 有効期限年
			uchar			kigen_mon;	// 有効期限月
			uchar			kigen_day;	// 有効期限日
		} ppc_data;						// NT-NET精算データ送信前にクリアする
	} uDiscData;
} t_SeisanDiscountOld;
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]

typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			PayCount;										// 精算追い番
	ushort			PayMethod;										// 精算方法
	ushort			PayClass;										// 処理区分
	ushort			PayMode;										// 精算ﾓｰﾄﾞ
	ulong			LockNo;											// 区画情報
	ushort			CardType;										// 駐車券ﾀｲﾌﾟ
	ushort			CMachineNo;										// 駐車券機械№
	ulong			CardNo;											// 駐車券番号(発券追い番)
	date_time_rec2	OutTime;										// 出庫年月日時分秒
	ushort			KakariNo;										// 係員№
	ushort			OutKind;										// 精算出庫
	ushort			CountSet;										// 在車ｶｳﾝﾄ
	ushort			Before_pst;										// 前回％割引率
	ulong			BeforePwari;									// 前回％割引金額
	ulong			BeforeTime;										// 前回割引時間数
	ulong			BeforeTwari;									// 前回時間割引金額
	ulong			BeforeRwari;									// 前回料金割引金額
	ushort			ReceiptIssue;									// 領収証発行有無
	date_time_rec2	InTime;											// 入庫年月日時分秒
	date_time_rec2	PayTime;										// 事前精算年月日時分秒(CPS精算時ｾｯﾄ)
	ulong			TaxPrice;										// 課税対象額
	ulong			TotalPrice;										// 合計金額
	ulong			Tax;											// 消費税額
	ushort			Syubet;											// 料金種別
	ulong			Price;											// 駐車料金
	ulong			TotalDiscount;									// 総割引額
	ulong			CashPrice;										// 現金売上
	ulong			InPrice;										// 投入金額
	ulong			ChgPrice;										// 釣銭金額
	ulong			Fusoku;											// 釣銭払出不足金額
	ushort			FusokuFlg;										// 釣銭払出不足発生ﾌﾗｸﾞ
	ushort			PayObsFlg;										// 精算中障害発生ﾌﾗｸﾞ
	ushort			ChgOverFlg;										// 払戻上限額ｵｰﾊﾞｰ発生ﾌﾗｸﾞ
	ushort			PassCheck;										// ｱﾝﾁﾊﾟｽﾁｪｯｸ
	PASS_DATA		PassData;										// 定期券ﾃﾞｰﾀ
	ulong			PassRenewalPric;								// 定期券更新料金
	ushort			PassRenewalCondition;							// 定期券更新条件
	ushort			PassRenewalPeriod;								// 定期券更新期間
	date_time_rec2	BeforePayTime;									// 前回事前精算年月日時分秒（精算後券で精算時）
	uchar			MatchIP[8];										// ﾏｯﾁﾝｸﾞIPｱﾄﾞﾚｽ
	uchar			MatchVTD[8];									// ﾏｯﾁﾝｸﾞVTD/車番
	uchar			CreditCardNo[20];								// ｸﾚｼﾞｯﾄｶｰﾄﾞ会員№
	ulong			Credit_ryo;										// ｸﾚｼﾞｯﾄｶｰﾄﾞ利用金額
	ulong			CreditSlipNo;									// ｸﾚｼﾞｯﾄｶｰﾄﾞ伝票番号
	ulong			CreditAppNo;									// ｸﾚｼﾞｯﾄｶｰﾄﾞ承認番号
	uchar			CreditName[10];									// ｸﾚｼﾞｯﾄｶｰﾄﾞ会社名
	uchar			CreditDate[2];									// ｸﾚｼﾞｯﾄｶｰﾄﾞ有効期限(年月)
	ulong			CreditProcessNo;								// ｸﾚｼﾞｯﾄｶｰﾄﾞｾﾝﾀ処理追い番
	ulong			CreditReserve1;									// ｸﾚｼﾞｯﾄｶｰﾄﾞ予備1
	ulong			CreditReserve2;									// ｸﾚｼﾞｯﾄｶｰﾄﾞ予備2
	uchar			PayCalMax;										// 最大料金適用有無
	uchar			Reserve1[3];									// 予備1
	ulong			Reserve2;										// 予備2
	ulong			Reserve3;										// 予備3
	ulong			Reserve4;										// 予備4
	ulong			Reserve5;										// 予備5
	ulong			Reserve6;										// 予備6
	ulong			Reserve7;										// 予備7
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 	DISCOUNT_DATA	DiscountData[NTNET_DIC_MAX];					// 割引関連
	t_SeisanDiscountOld	DiscountData[NTNET_DIC_MAX];				// 割引関連
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
} DATA_KIND_22;

typedef	struct {
	ushort			FullNo1;										// 満車台数１
	ushort			CarCnt1;										// 現在台数１
	ushort			FullNo2;										// 満車台数２
	ushort			CarCnt2;										// 現在台数２
	ushort			FullNo3;										// 満車台数３
	ushort			CarCnt3;										// 現在台数３
	ushort			PascarCnt;										// 定期車両カウント
	ushort			Full[3];										// 駐車1～3満空状態
} PARKCAR_DATA1;
typedef	struct {
	ushort			GroupNo;										// ｸﾞﾙｰﾌﾟ番号
	ushort			State;											// 用途別駐車台数設定
	ushort			Full[3];										// 駐車1～3満空状態
	ushort			EmptyNo1;										// 空車台数１
	ushort			FullNo1;										// 満車台数１
	ushort			EmptyNo2;										// 空車台数２
	ushort			FullNo2;										// 満車台数２
	ushort			EmptyNo3;										// 空車台数３
	ushort			FullNo3;										// 満車台数３
} PARKCAR_DATA2;

typedef	struct {
	ushort			FullNo1;										// 満車台数１
	ushort			CarCnt1;										// 現在台数１
	ushort			FullNo2;										// 満車台数２
	ushort			CarCnt2;										// 現在台数２
	ushort			FullNo3;										// 満車台数３
	ushort			CarCnt3;										// 現在台数３
	uchar			CarFullFlag;									// 台数・満空状態フラグ
	uchar			PascarCnt;										// 定期車両カウント
	ushort			Full[3];										// 駐車1～3満空状態
} PARKCAR_DATA11;
typedef	struct {
	ushort			FullSts1;										// 駐車１満空状態
	ushort			EmptyNo1;										// 空車台数１
	ushort			FullNo1;										// 満車台数１
	ushort			FullSts2;										// 駐車２満空状態
	ushort			EmptyNo2;										// 空車台数２
	ushort			FullNo2;										// 満車台数２
	ushort			FullSts3;										// 駐車３満空状態
	ushort			EmptyNo3;										// 空車台数３
	ushort			FullNo3;										// 満車台数３
} PARKCAR_DATA21;
// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
typedef	struct {
	ushort			FullNo1;										// 満車台数１
	ushort			CarCnt1;										// 現在台数１
	ushort			FullNo2;										// 満車台数２
	ushort			CarCnt2;										// 現在台数２
	ushort			FullNo3;										// 満車台数３
	ushort			CarCnt3;										// 現在台数３
	uchar			CarFullFlag;									// 台数・満空状態フラグ
	uchar			PascarCnt;										// 定期車両カウント
	ushort			Full[3];										// 駐車1～3満空状態
// 不具合修正(S) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
//	struct{	// 台数管理追番
//		uchar		CarCntYear;										// 年
//		uchar		CarCntMon;										// 月
//		uchar		CarCntDay;										// 日
//		uchar		CarCntHour;										// 時
//		uchar		CarCntMin;										// 分
//		ushort		CarCntSeq;										// 追番
//		uchar		Reserve1;										// 予備
//	} CarCntInfo;
	CAR_CNT_INFO	CarCntInfo;											// 台数管理追番
// 不具合修正(E) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
} PARKCAR_DATA14;
// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応
/*--------------------------------------------------------------------------*/
/*	入庫ﾃﾞｰﾀ(ﾃﾞｰﾀ種別54)													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	PARKCAR_DATA1	ParkData;										// 駐車台数
	ulong			InCount;										// 入庫追い番
	ushort			Syubet;											// 処理区分
	ushort			InMode;											// 入庫ﾓｰﾄﾞ
	ulong			LockNo;											// 区画情報
	ushort			CardType;										// 駐車券ﾀｲﾌﾟ
	ushort			CMachineNo;										// 駐車券機械№
	ulong			CardNo;											// 駐車券番号(発券追い番)
	date_time_rec2	InTime;											// 入庫年月日時分秒
	ushort			PassCheck;										// ｱﾝﾁﾊﾟｽﾁｪｯｸ
	ushort			CountSet;										// 在車ｶｳﾝﾄ
	PASS_DATA		PassData;										// 定期券ﾃﾞｰﾀ
	ushort			Dummy[32];										// ﾀﾞﾐｰ
} DATA_KIND_54;
typedef struct {
	ushort			MediaKind;										// 種別(媒体)	0～99
	uchar			MediaCardNo[30];								// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
	uchar			MediaCardInfo[16];								// カード情報	"0"～"9"、"A"～"Z"(英数字) 左詰め
} t_MediaInfo;		// 入庫／精算　媒体情報

// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
typedef struct {
	ushort			MediaKind;										// 種別(媒体)	0～99
	uchar			MediaCardNo[30];								// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
	uchar			MediaCardInfo[16];								// カード情報	"0"～"9"、"A"～"Z"(英数字) 左詰め
	ushort			Reserve1;										// 予備(サイズ)
} t_MediaInfo2;		// 入庫／精算　媒体情報
// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応

typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	PARKCAR_DATA11	ParkData;										// 駐車台数
	ulong			CenterSeqNo;									// センター追番
	ulong			InCount;										// 入庫追い番
	uchar			Syubet;											// 処理区分
	uchar			InMode;											// 入庫ﾓｰﾄﾞ
	uchar			CMachineNo;										// 駐車券機械№
	uchar			Reserve;										// 予備（サイズ調整用）
	ushort			FlapArea;										// フラップシステム	区画		0～99
	ushort			FlapParkNo;										// 					車室番号	0～9999
	uchar			PassCheck;										// ｱﾝﾁﾊﾟｽﾁｪｯｸ
	uchar			CountSet;										// 在車ｶｳﾝﾄ
	ulong			ParkingNo;										// 定期券　駐車場№	0～999999
	t_MediaInfo		Media[4];										// 入庫媒体情報1～4
} DATA_KIND_54_r10;
// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	PARKCAR_DATA14	ParkData;										// 駐車台数
	ulong			CenterSeqNo;									// センター追番
	ulong			InCount;										// 入庫追い番
	uchar			Syubet;											// 処理区分
	uchar			InMode;											// 入庫ﾓｰﾄﾞ
	uchar			CMachineNo;										// 駐車券機械№
	uchar			Reserve;										// 予備（サイズ調整用）
	ushort			FlapArea;										// フラップシステム	区画		0～99
	ushort			FlapParkNo;										// 					車室番号	0～9999
	uchar			PassCheck;										// ｱﾝﾁﾊﾟｽﾁｪｯｸ
	uchar			CountSet;										// 在車ｶｳﾝﾄ
	ulong			ParkingNo;										// 定期券　駐車場№	0～999999
// 不具合修正(S) K.Onodera 2016/12/09 連動評価指摘(0カット前のサイズが誤っている)
//	t_MediaInfo		Media[4];										// 入庫媒体情報1～4
	t_MediaInfo2	Media[4];										// 入庫媒体情報1～4
	ushort			DepositKind;									// 保証金区分
	ulong			DepositMoney;									// 保証金金額
	ulong			InMoney;										// 投入金額
	ulong			ChgMoney;										// 釣銭額
// 不具合修正(E) K.Onodera 2016/12/09 連動評価指摘(0カット前のサイズが誤っている)
} DATA_KIND_54_r13;
// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応

/*--------------------------------------------------------------------------*/
/*	精算ﾃﾞｰﾀ(ﾃﾞｰﾀ種別56,57)													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	PARKCAR_DATA1	ParkData;										// 駐車台数
	ulong			PayCount;										// 精算追い番
	ushort			PayMethod;										// 精算方法
	ushort			PayClass;										// 処理区分
	ushort			PayMode;										// 精算ﾓｰﾄﾞ
	ulong			LockNo;											// 区画情報
	ushort			CardType;										// 駐車券ﾀｲﾌﾟ
	ushort			CMachineNo;										// 駐車券機械№
	ulong			CardNo;											// 駐車券番号(発券追い番)
	date_time_rec2	OutTime;										// 出庫年月日時分秒
	ushort			KakariNo;										// 係員№
	ushort			OutKind;										// 精算出庫
	ushort			CountSet;										// 在車ｶｳﾝﾄ
	ushort			Before_pst;										// 前回％割引率
	ulong			BeforePwari;									// 前回％割引金額
	ulong			BeforeTime;										// 前回割引時間数
	ulong			BeforeTwari;									// 前回時間割引金額
	ulong			BeforeRwari;									// 前回料金割引金額
	ushort			ReceiptIssue;									// 領収証発行有無
	date_time_rec2	InTime;											// 入庫年月日時分秒
	date_time_rec2	PayTime;										// 事前精算年月日時分秒(CPS精算時ｾｯﾄ)
	ulong			TaxPrice;										// 課税対象額
	ulong			TotalPrice;										// 合計金額
	ulong			Tax;											// 消費税額
	ushort			Syubet;											// 料金種別
	ulong			Price;											// 駐車料金
	ulong			TotalDiscount;									// 総割引額
	ulong			CashPrice;										// 現金売上
	ulong			InPrice;										// 投入金額
	ulong			ChgPrice;										// 釣銭金額
	ulong			Fusoku;											// 釣銭払出不足金額
	ushort			FusokuFlg;										// 釣銭払出不足発生ﾌﾗｸﾞ
	ushort			PayObsFlg;										// 精算中障害発生ﾌﾗｸﾞ
	ushort			ChgOverFlg;										// 払戻上限額ｵｰﾊﾞｰ発生ﾌﾗｸﾞ
	ushort			PassCheck;										// ｱﾝﾁﾊﾟｽﾁｪｯｸ
	PASS_DATA		PassData;										// 定期券ﾃﾞｰﾀ
	ulong			PassRenewalPric;								// 定期券更新料金
	ushort			PassRenewalCondition;							// 定期券更新条件
	ushort			PassRenewalPeriod;								// 定期券更新期間
	date_time_rec2	BeforePayTime;									// 前回事前精算年月日時分秒（精算後券で精算時）

	uchar			MatchIP[8];										// ﾏｯﾁﾝｸﾞIPｱﾄﾞﾚｽ
	uchar			MatchVTD[8];									// ﾏｯﾁﾝｸﾞVTD/車番
	uchar			CreditCardNo[20];								// ｸﾚｼﾞｯﾄｶｰﾄﾞ会員№
	ulong			Credit_ryo;										// ｸﾚｼﾞｯﾄｶｰﾄﾞ利用金額
	ulong			CreditSlipNo;									// ｸﾚｼﾞｯﾄｶｰﾄﾞ伝票番号
	ulong			CreditAppNo;									// ｸﾚｼﾞｯﾄｶｰﾄﾞ承認番号
	uchar			CreditName[10];									// ｸﾚｼﾞｯﾄｶｰﾄﾞ会社名
	uchar			CreditDate[2];									// ｸﾚｼﾞｯﾄｶｰﾄﾞ有効期限(年月)
	ulong			CreditProcessNo;								// ｸﾚｼﾞｯﾄｶｰﾄﾞｾﾝﾀ処理追い番
	ulong			CreditReserve1;									// ｸﾚｼﾞｯﾄｶｰﾄﾞ予備1
	ulong			CreditReserve2;									// ｸﾚｼﾞｯﾄｶｰﾄﾞ予備2
	ulong			Reserve1;										// 予備1
	ulong			Reserve2;										// 予備2
	ulong			Reserve3;										// 予備3
	ulong			Reserve4;										// 予備4
	ulong			Reserve5;										// 予備5
	ulong			Reserve6;										// 予備6
	ulong			Reserve7;										// 予備7
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 	DISCOUNT_DATA	DiscountData[NTNET_DIC_MAX];					// 割引関連
	t_SeisanDiscountOld	DiscountData[NTNET_DIC_MAX];				// 割引関連
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
} DATA_KIND_56;
typedef	struct {
	uchar		In_Year;			// 入庫	年		00～99
	uchar		In_Mon;				// 		月		01～12
	uchar		In_Day;				// 		日		01～31
	uchar		In_Hour;			// 		時		00～23
	uchar		In_Min;				// 		分		00～59
	uchar		In_Sec;				// 		秒		00～59(磁気カードは0固定)
	uchar		Prev_Year;			// 前回精算	年	00～99 2000～2099
	uchar		Prev_Mon;			// 			月	01～12
	uchar		Prev_Day;			// 			日	01～31
	uchar		Prev_Hour;			// 			時	00～23
	uchar		Prev_Min;			// 			分	00～59
	uchar		Prev_Sec;			// 			秒	00～59(磁気カードは0固定)
} t_InPrevYMDHMS;		// 入庫/前回精算_YMDHMS

typedef	struct {
	uchar			MoneyKind_In;	// 金種有無	搭載金種の有無
									//			1bit目:10円、2bit目:50円、3bit目:100円、4bit目:500円
									//			5bit目:1000円、6bit目:2000円、7bit目:5000円、8bit目:10000円
	uchar			In_10_cnt;		// 投入金枚数(10円)		0～255
	uchar			In_50_cnt;		// 投入金枚数(50円)		0～255
	uchar			In_100_cnt;		// 投入金枚数(100円)	0～255
	uchar			In_500_cnt;		// 投入金枚数(500円)	0～255
	uchar			In_1000_cnt;	// 投入金枚数(1000円)	0～255
	uchar			In_2000_cnt;	// 投入金枚数(2000円)	0～255
	uchar			In_5000_cnt;	// 投入金枚数(5000円)	0～255
	uchar			In_10000_cnt;	// 投入金枚数(10000円)	0～255

	uchar			MoneyKind_Out;	// 金種有無	搭載金種の有無
									//			1bit目:10円、2bit目:50円、3bit目:100円、4bit目:500円
									//			5bit目:1000円、6bit目:2000円、7bit目:5000円、8bit目:10000円
	uchar			Out_10_cnt;		// 払出金枚数(10円)		0～255(払い出し不足分は含まない)
	uchar			Out_50_cnt;		// 払出金枚数(50円)		0～255(払い出し不足分は含まない)
	uchar			Out_100_cnt;	// 払出金枚数(100円)	0～255(払い出し不足分は含まない)
	uchar			Out_500_cnt;	// 払出金枚数(500円)	0～255(払い出し不足分は含まない)
	uchar			Out_1000_cnt;	// 払出金枚数(1000円)	0～255(払い出し不足分は含まない)
	uchar			Out_2000cnt;	// 払出金枚数(2000円)	0～255(払い出し不足分は含まない)
	uchar			Out_5000_cnt;	// 払出金枚数(5000円)	0～255(払い出し不足分は含まない)
	uchar			Out_10000_cnt;	// 払出金枚数(10000円)	0～255(払い出し不足分は含まない)
} t_MoneyInOut;		// 金銭情報(投入金枚数/払出金枚数)

typedef struct {
	ulong			ParkingNo;		// 駐車場No.
	ushort			Kind;			// 割引種別
	ushort			Group;			// 割引区分
	ushort			Callback;		// 回収枚数
	ulong			Amount;			// 割引額
	ulong			Info1;			// 割引情報1
	ulong			Info2;			// 割引情報2
} t_SeisanDiscount;

typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	PARKCAR_DATA11	ParkData;										// 駐車台数
	ulong			CenterSeqNo;									// センター追番
	ulong			PayCount;										// 精算追番(0～99999)
	uchar			PayMethod;										// 精算方法(0＝券なし精算/1＝駐車券精算/2＝定期券精算/3＝定期券併用精算/4＝紛失精算)
	uchar			PayClass;										// 処理区分(0＝精算/1＝再精算/2＝精算中止/3＝再精算中止, 8=精算前, 9=精算途中
	uchar			PayMode;										// 精算モード(0＝自動精算/1＝半自動精算/2＝手動精算/3＝精算なし/4＝遠隔精算)
																	//				20＝Mifareプリペイド精算
	uchar			CMachineNo;										// 駐車券機械№(入庫機械№)	0～255
	ushort			FlapArea;										// フラップシステム	区画		0～99
	ushort			FlapParkNo;										// 					車室番号	0～9999
	ushort			KakariNo;										// 係員№	0～9999
	uchar			OutKind;										// 精算出庫	0＝通常精算/1＝強制出庫/(2＝精算なし出庫)/3＝不正出庫/9＝突破出庫
																	//				10＝ゲート開放/20＝Mifareプリペイド精算出庫
																	//				97＝ロック開・フラップ上昇前未精算出庫/98＝ラグタイム内出庫
																	//				99＝サービスタイム内出庫
	uchar			CountSet;										// 在車カウント	0＝する(+1)/1＝しない/2＝する(-1)
																	// (データ種別が「22：事前精算」の時は「1=しない」となる)
	t_InPrevYMDHMS	InPrev_ymdhms;									// 入庫/前回精算_YMDHMS
	uchar			ReceiptIssue;									// 領収証発行有無	0＝領収証なし/1＝領収証あり
	uchar			Syubet;											// 料金種別			1～
	ulong			Price;											// 駐車料金			0～
	ulong			CashPrice;										// 現金売上			0～
	ulong			InPrice;										// 投入金額			0～
	ushort			ChgPrice;										// 釣銭金額			0～9999
	t_MoneyInOut	MoneyInOut;										// 金銭情報（投入金額／払戻金額）
	ushort			HaraiModoshiFusoku;								// 払戻不足額	0～9999
	uchar			Reserve1;										// 予備(サイズ調整用)	0
	uchar			PassCheck;										// アンチパスチェック	0＝チェックON/1＝チェックＯＦＦ/2＝強制ＯＦＦ
																	//						※定期を使用しない場合は「１＝チェックＯＦＦ」固定
	ulong			ParkingNo;										// 定期券　駐車場№	0～999999
	t_MediaInfo		Media[2];										// 精算媒体情報1, 2
	ushort			CardKind;										// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
																	//					3=Edy、4=PiTaPa、5=WAON　　90以降=個別対応
	uchar			settlement[64];									// 決済情報			(※3)　暗号化対象
	ulong			MoneyIn;										// 入金処理金額
	ulong			MoneyOut;										// 出金処理金額
	ulong			MoneyBack;										// 払戻金額
	ulong			MoneyFusoku;									// 受取り不足金額
	t_SeisanDiscount	SDiscount[NTNET_DIC_MAX];					// 割引
} DATA_KIND_56_r10;

// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	PARKCAR_DATA14	ParkData;										// 駐車台数
	ulong			CenterSeqNo;									// センター追番
	ulong			PayCount;										// 精算追番(0～99999)
	uchar			PayMethod;										// 精算方法(0＝券なし精算/1＝駐車券精算/2＝定期券精算/3＝定期券併用精算/4＝紛失精算/5＝定期券更新
																	//   10＝修正精算（修正元）/11＝修正精算（修正）/12=振替精算（振替先）/13=後日精算/20＝入庫時精算/30=入場（受付）券精算)
	uchar			PayClass;										// 処理区分(0＝精算/1＝再精算/2＝精算中止/3＝再精算中止, 8=精算前, 9=精算途中
	uchar			PayMode;										// 精算モード(0＝自動精算/1＝半自動精算/2＝手動精算/3＝精算なし/4＝遠隔精算)
																	//				20＝Mifareプリペイド精算
	uchar			CMachineNo;										// 駐車券機械№(入庫機械№)	0～255
	ushort			FlapArea;										// フラップシステム	区画		0～99
	ushort			FlapParkNo;										// 					車室番号	0～9999
	ushort			KakariNo;										// 係員№	0～9999
	uchar			OutKind;										// 精算出庫	0＝通常精算/1＝強制出庫/(2＝精算なし出庫)/3＝不正出庫/9＝突破出庫
																	//				10＝ゲート開放/20＝Mifareプリペイド精算出庫
																	//				97＝ロック開・フラップ上昇前未精算出庫/98＝ラグタイム内出庫
																	//				99＝サービスタイム内出庫
	uchar			CountSet;										// 在車カウント	0＝する(+1)/1＝しない/2＝する(-1)
																	// (データ種別が「22：事前精算」の時は「1=しない」となる)
	t_InPrevYMDHMS	InPrev_ymdhms;									// 入庫/前回精算_YMDHMS
	uchar			ReceiptIssue;									// 領収証発行有無	0＝領収証なし/1＝領収証あり
	uchar			Syubet;											// 料金種別			1～
	ulong			Price;											// 駐車料金			0～
	long			CashPrice;										// 現金売上			0～
	ulong			InPrice;										// 投入金額			0～
	ushort			ChgPrice;										// 釣銭金額			0～9999
	t_MoneyInOut	MoneyInOut;										// 金銭情報（投入金額／払戻金額）
	ushort			HaraiModoshiFusoku;								// 払戻不足額(現金)	0～9999
	ushort			CardFusokuType;									// 払出不足媒体種別(現金以外)
	ushort			CardFusokuTotal;								// 払出不足額(現金以外)
	ulong			SaleParkingNo;									// 売上先駐車場№(0～999999   ※特注用（標準では基本駐車場№をセット）
	uchar			Reserve1;										// 予備(サイズ調整用)	0
	uchar			PassCheck;										// アンチパスチェック	0＝チェックON/1＝チェックＯＦＦ/2＝強制ＯＦＦ
																	//						※定期を使用しない場合は「１＝チェックＯＦＦ」固定
	ulong			ParkingNo;										// 定期券　駐車場№	0～999999
	t_MediaInfo2	Media[2];										// 精算媒体情報1, 2
	ushort			CardKind;										// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
																	//					3=Edy、4=PiTaPa、5=WAON　　90以降=個別対応
	uchar			settlement[80];									// 決済情報			(※3)　暗号化対象
	ulong			TotalSale;										// 合計金額(物販)
	ulong			DeleteSeq;										// 削除データ追番(1～FFFFFFFFH　削除する精算データにセットしたセンター追番)
	ulong			Reserve2;										// 予備
	ulong			Reserve3;										// 予備
	t_SeisanDiscount	SDiscount[NTNET_DIC_MAX];					// 割引
} DATA_KIND_56_r14;
// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応
// MH321800(S) Y.Tanizaki ICクレジット対応
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	PARKCAR_DATA14	ParkData;										// 駐車台数
	ulong			CenterSeqNo;									// センター追番
	ulong			PayCount;										// 精算追番(0～99999)
	uchar			PayMethod;										// 精算方法(0＝券なし精算/1＝駐車券精算/2＝定期券精算/3＝定期券併用精算/4＝紛失精算/5＝定期券更新
																	//   10＝修正精算（修正元）/11＝修正精算（修正）/12=振替精算（振替先）/13=後日精算/20＝入庫時精算/30=入場（受付）券精算)
	uchar			PayClass;										// 処理区分(0＝精算/1＝再精算/2＝精算中止/3＝再精算中止, 8=精算前, 9=精算途中
	uchar			PayMode;										// 精算モード(0＝自動精算/1＝半自動精算/2＝手動精算/3＝精算なし/4＝遠隔精算)
																	//				20＝Mifareプリペイド精算
	uchar			CMachineNo;										// 駐車券機械№(入庫機械№)	0～255
	ushort			FlapArea;										// フラップシステム	区画		0～99
	ushort			FlapParkNo;										// 					車室番号	0～9999
	ushort			KakariNo;										// 係員№	0～9999
	uchar			OutKind;										// 精算出庫	0＝通常精算/1＝強制出庫/(2＝精算なし出庫)/3＝不正出庫/9＝突破出庫
																	//				10＝ゲート開放/20＝Mifareプリペイド精算出庫
																	//				97＝ロック開・フラップ上昇前未精算出庫/98＝ラグタイム内出庫
																	//				99＝サービスタイム内出庫
	uchar			CountSet;										// 在車カウント	0＝する(+1)/1＝しない/2＝する(-1)
																	// (データ種別が「22：事前精算」の時は「1=しない」となる)
	t_InPrevYMDHMS	InPrev_ymdhms;									// 入庫/前回精算_YMDHMS
	ulong			ParkTime;										// 駐車時間
	uchar			ReceiptIssue;									// 領収証発行有無	0＝領収証なし/1＝領収証あり
	uchar			Syubet;											// 料金種別			1～
	ulong			Price;											// 駐車料金			0～
	long			CashPrice;										// 現金売上			0～
	ulong			InPrice;										// 投入金額			0～
	ushort			ChgPrice;										// 釣銭金額			0～9999
	t_MoneyInOut	MoneyInOut;										// 金銭情報（投入金額／払戻金額）
	ushort			HaraiModoshiFusoku;								// 払戻不足額(現金)	0～9999
	ushort			CardFusokuType;									// 払出不足媒体種別(現金以外)
	ushort			CardFusokuTotal;								// 払出不足額(現金以外)
	ulong			SaleParkingNo;									// 売上先駐車場№(0～999999   ※特注用（標準では基本駐車場№をセット）
	ushort			MaxChargeApplyFlg;								// 最大料金適用フラグ
	ushort			MaxChargeApplyCnt;								// 最大料金適用回数
	uchar			MaxChargeSettingFlg;							// 最大料金設定有無
	uchar			PassCheck;										// アンチパスチェック	0＝チェックON/1＝チェックＯＦＦ/2＝強制ＯＦＦ
																	//						※定期を使用しない場合は「１＝チェックＯＦＦ」固定
	ulong			ParkingNo;										// 定期券　駐車場№	0～999999
	t_MediaInfo2	Media[2];										// 精算媒体情報1, 2
// GG129001(S) データ保管サービス対応（適格請求書情報有無をセットする）
	ushort			Invoice;										// 適格請求書情報有無
// GG129001(E) データ保管サービス対応（適格請求書情報有無をセットする）
// GG129001(S) データ保管サービス対応（登録番号をセットする）
	uchar			RegistNum[14];									// 登録番号（先頭1文字+13桁の数字）
// GG129001(E) データ保管サービス対応（登録番号をセットする）
// GG129001(S) データ保管サービス対応（課税対象額をセットする）
	ulong			TaxPrice;										// 課税対象額
// GG129001(E) データ保管サービス対応（課税対象額をセットする）
// GG129001(S) データ保管サービス対応（適用税率をセットする）
	ushort			TaxRate;										// 適用税率
// GG129001(E) データ保管サービス対応（適用税率をセットする）
	ushort			CardKind;										// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
																	//					3=Edy、4=PiTaPa、5=WAON　　90以降=個別対応
// MH810105(S) MH364301 QRコード決済対応
	uchar			PayTerminalClass;								// 決済端末区分
	uchar			Transactiontatus;								// 取引ステータス
// MH810105(E) MH364301 QRコード決済対応
	uchar			settlement[80];									// 決済情報			(※3)　暗号化対象
	ulong			TotalSale;										// 合計金額(物販)
	ulong			DeleteSeq;										// 削除データ追番(1～FFFFFFFFH　削除する精算データにセットしたセンター追番)
	uchar			Reserve;										// 予備
	uchar			CarNumInfoFlg;									// 車番情報有無
// GG124100(S) R.Endo 2021/12/23 車番チケットレス3.0 #6123 NT-NET精算データの車番情報セット
// 	uchar			LandTransOfficeName[12];						// 車番　陸運支局名
// 	uchar			ClassNum[9];									// 車番　分類番号
// 	uchar			Reserve1;										// 予備
// 	uchar			UsageCharacter[3];								// 車番　用途文字
// 	uchar			Reserve2;										// 予備
// 	uchar			SeqDesignNumber[12];							// 車番　一連指定番号
// 	uchar			Reserve3[10];									// 予備
	SHABAN_INFO		ShabanInfo;										// 車番情報
// GG124100(E) R.Endo 2021/12/23 車番チケットレス3.0 #6123 NT-NET精算データの車番情報セット
	t_SeisanDiscount	SDiscount[NTNET_DIC_MAX];					// 割引
// MH810105(S) MH364301 QRコード決済対応
//} DATA_KIND_56_r17;
} DATA_KIND_56_rXX;
// MH810105(E) MH364301 QRコード決済対応
// MH321800(E) Y.Tanizaki ICクレジット対応

// MH810102(S) R.Endo 2021/03/17 車番チケットレス フェーズ2.5 #5359 NT-NET精算データにセットする券種の変更
// 精算データ保持用
#define MACHINE_TKT_NO_SIZE (9)	// 駐車券機械№(入庫機械№)、駐車券番号サイズ

typedef struct {
	uchar			NormalFlag;										// 一時利用フラグ(0=定期利用,1=一時利用)
	uchar			MachineTktNo[MACHINE_TKT_NO_SIZE];				// 駐車券機械№(入庫機械№)(上位3桁、0～255)、駐車券番号(下位6桁、0～999999)
	ulong			ParkingNo;										// 定期券駐車場№
	ulong			id;												// 定期券ID(5桁、0～12000)
	uchar			syu;											// 定期券種別(2桁、1～15)
	uchar			pkno_syu;										// 定期券駐車場№種別(0=基本,1-3=拡張)
// GG132000(S) 精算完了／中止時のNT-NET通信の精算データにデータ項目を追加
	ushort			MaxFeeApplyFlg;									// 最大料金適用フラグ
	ushort			MaxFeeApplyCnt;									// 最大料金適用回数
	uchar			MaxFeeSettingFlg;								// 最大料金設定有無
// GG132000(E) 精算完了／中止時のNT-NET通信の精算データにデータ項目を追加
} ntNet_56_saveInf;

extern ntNet_56_saveInf ntNet_56_SaveData;
// MH810102(E) R.Endo 2021/03/17 車番チケットレス フェーズ2.5 #5359 NT-NET精算データにセットする券種の変更

/*--------------------------------------------------------------------------*/
/*	決済情報（電子マネー）
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			amount;
	uchar			card_id[20];
	ulong			card_zangaku;
	uchar			reserve[36];
} EMONEY;
// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
typedef struct {
	ulong			amount;
	uchar			card_id[20];
	ulong			card_zangaku;
// MH321800(E) D.Inaba ICクレジット対応
//	uchar			reserve[52];
	uchar			inquiry_num[16];
// MH810105(S) MH364301 QRコード決済対応
//	uchar			reserve[36];
	ulong			approbal_no;		// 承認番号
	uchar			reserve[32];
// MH810105(E) MH364301 QRコード決済対応
// MH321800(E) D.Inaba ICクレジット対応
} EMONEY_r14;

// MH810105(S) MH364301 QRコード決済対応
/*--------------------------------------------------------------------------*/
/*	決済情報（QRコード決済）
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			amount;				// 利用金額
	uchar			MchTradeNo[32];		// Mch取引番号
	uchar			PayTerminalNo[16];	// 支払端末ID
	uchar			DealNo[16];			// 取引番号
	uchar			PayKind;			// 決済ブランド
	uchar			dummy;
	uchar			reserve[10];
} QRCODE_rXX;
// MH810105(E) MH364301 QRコード決済対応

// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応
/*--------------------------------------------------------------------------*/
/*	決済情報（クレジット）
/*--------------------------------------------------------------------------*/
// MH321800(S) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
//typedef struct {
//	ulong			amount;
//	uchar			card_no[20];
//	uchar			cct_num[16];
//	uchar			kid_code[6];
//	ulong			app_no;
//	ulong			center_oiban;
//	uchar			reserve[10];
//} CREINFO;
// MH321800(E) T.Nagai ICクレジット対応 不要機能削除(センタークレジット)
// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
typedef struct {
	ulong			amount;				// 利用金額
	uchar			card_no[20];		// 会員番号
	uchar			cct_num[16];		// 端末識別番号
	uchar			kid_code[6];		// KIDコード
	ulong			app_no;				// 承認番号
	ulong			center_oiban;		// センタ処理通番
	uchar			ShopAccountNo[20];	// 加盟店取引番号
	ulong			slip_no;			// 伝票番号
	uchar			reserve[2];
} CREINFO_r14;
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
typedef struct {	// クレジット（ID56/57:精算データ用）
	uchar			CreditCardNo[20];								// ｸﾚｼﾞｯﾄｶｰﾄﾞ会員№
	ulong			Credit_ryo;										// ｸﾚｼﾞｯﾄｶｰﾄﾞ利用金額
	ulong			CreditSlipNo;									// ｸﾚｼﾞｯﾄｶｰﾄﾞ伝票番号
	ulong			CreditAppNo;									// ｸﾚｼﾞｯﾄｶｰﾄﾞ承認番号
	uchar			CreditName[10];									// ｸﾚｼﾞｯﾄｶｰﾄﾞ会社名
	uchar			CreditDate[2];									// ｸﾚｼﾞｯﾄｶｰﾄﾞ有効期限(年月)
	ulong			CreditProcessNo;								// ｸﾚｼﾞｯﾄｶｰﾄﾞｾﾝﾀ処理追い番
	uchar			term_id[16];									// 端末識別番号
} SETTLEMENT_CREDIT;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

typedef struct {	// 精算媒体（ID152:精算情報データ用）
	uchar			card_id[30];									// カード番号
	uchar			card_info[16];									// カード情報
	uchar			reserve[2];										// 空き
} SETTLEMENT_OUTMEDIA;
typedef union {
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
	SETTLEMENT_CREDIT	credit;										// クレジット（ID56/57:精算データ用）
	uchar				cardno[16];									// 電子マネー（ID56/57:精算データ用）
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
	SETTLEMENT_OUTMEDIA	media;
} SETTLEMENT_INFO;
// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応
/*--------------------------------------------------------------------------*/
/*	クレジットカードデータ
/*--------------------------------------------------------------------------*/
typedef struct {
	uchar			JIS2Data[69];									// JIS2ｶｰﾄﾞﾃﾞｰﾀ
	uchar			JIS1Data[37];									// JIS1ｶｰﾄﾞﾃﾞｰﾀ
	ushort			PayMethod;										// 支払方法
	ulong			DivCount;										// 分割回数
} CARDDAT;

/*--------------------------------------------------------------------------*/
/*	駐車台数データ(ﾃﾞｰﾀ種別58/59)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	PARKCAR_DATA2	ParkData;										// 駐車台数
	ulong			Reserve[4];										// 予備
} DATA_KIND_58;
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	PARKCAR_DATA21	ParkData;										// 駐車台数
	ulong			Reserve[4];										// 予備
} DATA_KIND_58_r10;
/*--------------------------------------------------------------------------*/
/*	センター用端末情報ﾃﾞｰﾀ(ﾃﾞｰﾀ種別65)										*/
/*--------------------------------------------------------------------------*/
typedef union {
	struct {
		ushort	b15	: 1 ;
		ushort	b14	: 1 ;
		ushort	b13	: 1 ;
		ushort	b12	: 1 ;
		ushort	b11	: 1 ;
		ushort	b10	: 1 ;
		ushort	b09	: 1 ;
		ushort	b08	: 1 ;
		ushort	b07	: 1 ;
		ushort	b06	: 1 ;
		ushort	b05	: 1 ;
		ushort	b04	: 1 ;
		ushort	b03	: 1 ;
		ushort	b02	: 1 ;
		ushort	b01	: 1 ;
		ushort	b00	: 1 ;
	} bits;
	ushort	word;
} REQ_BIT;
// GG120600(S) // Phase9 センター用端末情報データに受信電文リビジョンを追加
typedef struct {
	uchar		DataProto;			// 通信プロトコル(1=遠隔NT-NET/2=RealTime/3=DC-NET)
	uchar		DataSysID;			// システムID
	ushort		DataID;				// 受信電文データ種別
	ushort		DataRev;			// 受信電文リビジョン
}DATA_REV;
// GG120600(E) // Phase9 センター用端末情報データに受信電文リビジョンを追加
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	ulong			SMachineNo;										// 送信先端末機械№
	ushort			Result;											// 要求結果
	REQ_BIT			AcceptReq;										// 受付可能要求
	uchar			ProgramVer[12];									// ﾌﾟﾛｸﾞﾗﾑﾊﾞｰｼﾞｮﾝ
// MH810100(S) K.Onodera 2019/12/16 車番チケットレス(ParkingWebフェーズ9対応)
	uchar			BaseProgramVer[12];								// ﾍﾞｰｽﾌﾟﾛｸﾞﾗﾑﾊﾞｰｼﾞｮﾝ
	uchar			RyoCalDllVer[12];								// 料金計算dllﾊﾞｰｼﾞｮﾝ
// MH810100(E) K.Onodera 2019/12/16 車番チケットレス(ParkingWebフェーズ9対応)
	uchar			ModuleVer[15][12];								// ﾓｼﾞｭｰﾙﾊﾞｰｼﾞｮﾝ
// GG120600(S) // Phase9 センター用端末情報データに受信電文リビジョンを追加
	DATA_REV		DataRevs[100];									// 受信電文リビジョン
// GG120600(E) // Phase9 センター用端末情報データに受信電文リビジョンを追加
} DATA_KIND_65;

/*--------------------------------------------------------------------------*/
/*	制御ﾃﾞｰﾀ応答(ﾃﾞｰﾀ種別100,101)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			SMachineNo;										// 送信先端末機械№
	uchar			ControlData[70];								// 各種制御
} DATA_KIND_100;


/*--------------------------------------------------------------------------*/
/*	管理ﾃﾞｰﾀ要求(ﾃﾞｰﾀ種別103,104)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			SMachineNo;										// 送信先端末機械№
	uchar			ControlData[70];								// 各種制御
} DATA_KIND_103;

#define	NTNET_MANDATA_CLOCK			0x00000001	// 時計データ
#define	NTNET_MANDATA_CTRL			0x00000002	// 制御データ
#define	NTNET_MANDATA_COMMON		0x00000004	// 共通設定データ
#define	NTNET_MANDATA_INVTKT		0x00000008	// 無効駐車券データ
#define	NTNET_MANDATA_PASSSTOP		0x00000010	// 定期券精算中止データ
#define	NTNET_MANDATA_PASSSTS		0x00000020	// 定期券ステータス
#define	NTNET_MANDATA_PASSCHANGE	0x00000040	// 定期券更新ステータス
#define	NTNET_MANDATA_PASSEXIT		0x00000080	// 定期券出庫時刻テーブル
#define	NTNET_MANDATA_LOCKINFO		0x00000100	// 車室パラメータ
#define	NTNET_MANDATA_LOCKMARKER	0x00000200	// ロック装置パラメータ
#define NTNET_MANDATA_SPECIALDAY	0x00000400	// 特別日設定データ
#define NTNET_MANDATA_PARKNUMCTL	0x00000800	// 駐車台数管理データ
#define	NTNET_MANDATA_PASSWORD		0x00002000	// パスワードデータ
#define	NTNET_MANDATA_NGKAKARIID	0x00004000	// 無効係員IDテーブル

/*--------------------------------------------------------------------------*/
/*	ﾃﾞｰﾀ要求2(ﾃﾞｰﾀ種別109)													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			SMachineNo;										// 送信先端末機械№
	uchar			ControlData[20];								// 各種制御
} DATA_KIND_109;

#define	NTNET_DATAREQ2_TSYOUKEI		0x00000001	// T小計データ
#define	NTNET_DATAREQ2_GTSYOUKEI	0x00000002	// GT小計データ
#define	NTNET_DATAREQ2_MTSYOUKEI	0x00000004	// MT小計データ
#define	NTNET_DATAREQ2_MSYOUKEI		0x00000008	// 複数小計データ
#define	NTNET_DATAREQ2_MGOUKEI		0x00000010	// 複数合計データ
#define	NTNET_DATAREQ2_COIN			0x00000020	// コイン金庫小計データ
#define	NTNET_DATAREQ2_NOTE			0x00000040	// 紙幣金庫小計データ
#define	NTNET_DATAREQ2_COUNT		0x00000080	// 動作カウントデータ
#define	NTNET_DATAREQ2_TERMINFO		0x00000100	// 端末情報データ
#define	NTNET_DATAREQ2_TERMSTS		0x00000200	// 端末状態データ
#define	NTNET_DATAREQ2_MONEY		0x00000400	// 金銭管理データ
#define	NTNET_DATAREQ2_SYASITU		0x00000800	// 簡易車室情報テーブル
#define	NTNET_DATAREQ2_NMSYOUKEI	0x00001000	// 現在複数小計データ

/*--------------------------------------------------------------------------*/
/*	センタ用データ要求(ﾃﾞｰﾀ種別154)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {						// 
	DATA_BASIC		DataBasic;			// 基本ﾃﾞｰﾀ
	ushort			FmtRev;				// フォーマットRev.№
	ulong			SMachineNo;			// 送信先端末機械№
	uchar			SeisanJyoutai_Req;	// 精算状態データ要求(0＝要求なし／1＝要求あり)
	uchar			RealTime_Req;		// リアルタイム情報要求(0＝要求なし／1＝要求あり)
	uchar			Reserve1;			// 予約1(0＝要求なし／1＝要求あり)
	uchar			Reserve2;			// 予約2(0＝要求なし／1＝要求あり)
	uchar			Reserve3;			// 予約3(0＝要求なし／1＝要求あり)
	uchar			Reserve4;			// 予約4(0＝要求なし／1＝要求あり)
	uchar			Reserve5;			// 予約5(0＝要求なし／1＝要求あり)
	uchar			Reserve6;			// 予約6(0＝要求なし／1＝要求あり)
	uchar			SynchroTime_Req;	// 同期時刻データ要求(0＝要求なし／1＝要求あり)
	uchar			TermInfo_Req;		// 端末情報データ要求(0＝要求なし／1＝要求あり)
} DATA_KIND_154;

/*--------------------------------------------------------------------------*/
/*	時計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別119,229)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			Year;											// Year
	uchar			Mon;											// Month
	uchar			Day;											// Day
	uchar			Hour;											// Hour
	uchar			Min;											// Minute
	ushort			Sec;											// Second
	ushort			MSec;											// Millisecond
	ulong			HOSEI_MSec;										// 補正値
	ulong			HOSEI_wk;										// 補正値 作業域
} DATA_KIND_119;


/*--------------------------------------------------------------------------*/
/*	ｴﾗｰﾃﾞｰﾀ(ﾃﾞｰﾀ種別120)													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	uchar			Errsyu;											// ｴﾗｰ種別
	uchar			Errcod;											// ｴﾗｰｺｰﾄﾞ
	uchar			Errdtc;											// ｴﾗｰｽﾃｰﾀｽ(発生/解除)
	uchar			Errlev;											// ｴﾗｰﾚﾍﾞﾙ
	uchar			Errdat1[10];									// ｴﾗｰ情報(ﾊﾞｲﾅﾘ)
	uchar			Errdat2[160];									// ｴﾗｰ情報(acsiiｺｰﾄﾞ)
} DATA_KIND_120;


/*--------------------------------------------------------------------------*/
/*	ｱﾗｰﾑﾃﾞｰﾀ(ﾃﾞｰﾀ種別121)													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	uchar			Armsyu;											// ｱﾗｰﾑ種別
	uchar			Armcod;											// ｱﾗｰﾑｺｰﾄﾞ
	uchar			Armdtc;											// ｱﾗｰﾑｽﾃｰﾀｽ(発生/解除)
	uchar			Armlev;											// ｱﾗｰﾑﾚﾍﾞﾙ
	uchar			Armdat1[10];									// ｱﾗｰﾑ情報(ﾊﾞｲﾅﾘ)
	uchar			Armdat2[160];									// ｱﾗｰﾑ情報(acsiiｺｰﾄﾞ)
} DATA_KIND_121;

/*--------------------------------------------------------------------------*/
/*	ｴﾗｰﾃﾞｰﾀ(ﾃﾞｰﾀ種別63)														*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	uchar			Errsyu;											// ｴﾗｰ種別
	uchar			Errcod;											// ｴﾗｰｺｰﾄﾞ
	uchar			Errdtc;											// ｴﾗｰｽﾃｰﾀｽ(発生/解除)
	uchar			Errlev;											// ｴﾗｰﾚﾍﾞﾙ
	uchar			ErrDoor;										// ﾄﾞｱ状態(0:close,1:open)
	uchar			Errdat[10];										// ｴﾗｰ情報(ﾊﾞｲﾅﾘ)
} DATA_KIND_63;


/*--------------------------------------------------------------------------*/
/*	ｱﾗｰﾑﾃﾞｰﾀ(ﾃﾞｰﾀ種別64)													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	uchar			Armsyu;											// ｱﾗｰﾑ種別
	uchar			Armcod;											// ｱﾗｰﾑｺｰﾄﾞ
	uchar			Armdtc;											// ｱﾗｰﾑｽﾃｰﾀｽ(発生/解除)
	uchar			Armlev;											// ｱﾗｰﾑﾚﾍﾞﾙ
	uchar			ArmDoor;										// ﾄﾞｱ状態(0:close,1:open)
	uchar			Armdat[10];										// ｱﾗｰﾑ情報(ﾊﾞｲﾅﾘ)
} DATA_KIND_64;


/*--------------------------------------------------------------------------*/
/*	金銭管理ﾃﾞｰﾀ(ﾃﾞｰﾀ種別126)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ushort			Mai;											// 保有枚数
	ushort			Money;											// 金額
} MONEY_DATA;

typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			SMachineNo;										// 送信先端末機械№
	ulong			PayCount;										// 精算追い番
	ushort			PayClass;										// 処理区分
	date_time_rec2	PayTime;										// 精算年月日時分秒
	ushort			KakariNo;										// 係員№
	MONEY_DATA		CoinSf[8];										// ｺｲﾝ金庫ﾃﾞｰﾀ
	ulong			CoinSfTotal;									// ｺｲﾝ金庫総額
	MONEY_DATA		NoteSf[8];										// 紙幣金庫ﾃﾞｰﾀ
	ulong			NoteSfTotal;									// 紙幣金庫総額
	MONEY_DATA		Coin[8];										// ｺｲﾝ循環ﾃﾞｰﾀ
	MONEY_DATA		CoinYotiku[2];									// ｺｲﾝ予蓄ﾃﾞｰﾀ
	MONEY_DATA		NoteChange[8];									// 紙幣払出しﾃﾞｰﾀ
} DATA_KIND_126;

/*--------------------------------------------------------------------------*/
/*	釣銭管理集計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別135)											*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ushort			Kind;											// 金種
	ushort			gen_mai;										// 現在（変更前）保有枚数
	ushort			zen_mai;										// 前回保有枚数
	ushort			sei_nyu;										// 精算時入金枚数
	ushort			sei_syu;										// 精算時出金枚数
	ushort			jyun_syu;										// 循環出金枚数
	ushort			hojyu;											// 釣銭補充枚数
	ushort			hojyu_safe;										// 釣銭補充時金庫搬送枚数
	ushort			turi_kyosei;									// 強制払出枚数(釣銭口)
	ushort			kin_kyosei;										// 強制払出枚数(金庫)
	ushort			sin_mai;										// 新規設定枚数
} TURI_DATA;

typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	ulong			CenterSeqNo;									// センター追番
	ulong			Oiban;											// 金銭管理合計追番
	ushort			PayClass;										// 処理区分
	ushort			KakariNo;										// 係員№
	TURI_DATA		turi_dat[4];									// 金銭ﾃﾞｰﾀ(4金種分)
	TURI_DATA		yturi_dat[5];									// 金銭ﾃﾞｰﾀ(5予蓄分)
} DATA_KIND_135;

/*--------------------------------------------------------------------------*/
/*	定期券問合せﾃﾞｰﾀ(ﾃﾞｰﾀ種別142)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			ParkingNo;										// 駐車場№
	ulong			PassID;											// 定期券ID
} DATA_KIND_142;


/*--------------------------------------------------------------------------*/
/*	定期券問合せ結果ﾃﾞｰﾀ(ﾃﾞｰﾀ種別143)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			Reserve;										// 予備
	ulong			ParkingNo;										// 駐車場№
	ulong			PassID;											// 定期券ID
	uchar			PassState;										// 定期券ｽﾃｰﾀｽ
	uchar			RenewalState;									// 更新ｽﾃｰﾀｽ
	date_time_rec	OutTime;										// 出庫年月日時分
	ulong			UseParkingNo;									// 利用中駐車場№
} DATA_KIND_143;


/*--------------------------------------------------------------------------*/
/*	端末情報ﾃﾞｰﾀ(ﾃﾞｰﾀ種別230)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			SMachineNo;										// 送信先端末機械№
	uchar			ProgramVer[12];									// ﾌﾟﾛｸﾞﾗﾑﾊﾞｰｼﾞｮﾝ
	uchar			ModuleVer[10][12];								// ﾓｼﾞｭｰﾙﾊﾞｰｼﾞｮﾝ
} DATA_KIND_230;


/*--------------------------------------------------------------------------*/
/*	端末状態ﾃﾞｰﾀ(ﾃﾞｰﾀ種別231)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			SMachineNo;										// 送信先端末機械№
	uchar			TerminalSt;										// 端末状態
	uchar			OpenClose;										// 営休業
	uchar			LC1_State;										// LC1状態
	uchar			LC2_State;										// LC2状態
	uchar			LC3_State;										// LC3状態
	uchar			Gate_State;										// ｹﾞｰﾄ状態
	uchar			CardNGType;										// NGｶｰﾄﾞ読取内容
} DATA_KIND_231;


/*--------------------------------------------------------------------------*/
/*	共通設定ﾃﾞｰﾀ(ﾃﾞｰﾀ種別80,208)											*/
/*--------------------------------------------------------------------------*/
#define	NTNET_PARAMDATA_MAX				998
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			SMachineNo;										// 送信先端末機械№
	ushort			uMode;											// 更新ﾓｰﾄﾞ
	ulong			ModelCode;										// 設定データの機種コード
	ushort			Status;											// ﾃﾞｰﾀｽﾃｰﾀｽ
	ushort			Segment;										// 設定ｾｸﾞﾒﾝﾄ
	ushort			TopAddr;										// 開始ｱﾄﾞﾚｽ
	ushort			DataCount;										// 設定ﾃﾞｰﾀ数
	ulong			Data[NTNET_PARAMDATA_MAX];						// 設定ﾃﾞｰﾀ
} DATA_KIND_80;

enum {
	_RPKERR_INVALID_MACHINENO = 140,
	_RPKERR_INVALID_MODELCODE,
	_RPKERR_INVALID_PARA_ADDR,
	_RPKERR_NO_PREPARE_COMMAND,
	_RPKERR_COMMAND_REJECT,

	_RPKERR_MAX
};

typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			SMachineNo;										// 送信先端末機械№
	uchar			ProcMode;										// 処理区分
	uchar			dummy_1;
	ulong			dummy_4;										// (設定データの機種コード)
	ushort			Status;											// ﾃﾞｰﾀｽﾃｰﾀｽ
	ushort			Segment;										// 設定ｾｸﾞﾒﾝﾄ
	ushort			TopAddr;										// 開始ｱﾄﾞﾚｽ
	ushort			DataCount;										// 設定ﾃﾞｰﾀ数
	long			Data[NTNET_PARAMDATA_MAX];						// 設定ﾃﾞｰﾀ
} RP_DATA_KIND_80;

#define	RP_PARAMDATA_MAX	30
#define	RP_DATA80_LEN		_offsetof(RP_DATA_KIND_80, Data[RP_PARAMDATA_MAX])

/*--------------------------------------------------------------------------*/
/*	設定ﾃﾞｰﾀ要求(ﾃﾞｰﾀ種別78,90)												*/
/*--------------------------------------------------------------------------*/
// リパーク専用フォーマットとする
typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	ulong			SMachineNo;								// 送信先端末機械№
	ulong			dummy_4;								// (設定データの機種コード)
	ushort			dummy_2;								// (ﾊﾟﾗﾒｰﾀ区分)
	ushort			Segment;								// 設定ｾｸﾞﾒﾝﾄ
	ushort			TopAddr;								// 開始ｱﾄﾞﾚｽ
	ushort			DataCount;								// 設定ﾃﾞｰﾀ数
	uchar			Reserve[72];							// 予備
} RP_DATA_KIND_78;


/*--------------------------------------------------------------------------*/
/*	定期精算中止テーブル(ﾃﾞｰﾀ種別91,219)									*/
/*--------------------------------------------------------------------------*/

typedef	struct {
	ulong			ParkingNo;										// 駐車場№
	ulong			PassID;											// 定期券ID
	uchar			PassKind;										// 定期券種別
	uchar			ChargeType;										// 料金種別
	ushort			Year;											// 入庫年
	uchar			Month;											// 入庫月
	uchar			Day;											// 入庫日
	uchar			Hour;											// 入庫時
	uchar			Min;											// 入庫分
	ushort			Sec;											// 入庫秒
	ulong			CouponRyo;										// 回数券領収額
	ushort			ShopNo;											// 店番号
	uchar			ServiceTicketA;									// サービス券Ａ
	uchar			ServiceTicketB;									// サービス券Ｂ
	uchar			ServiceTicketC;									// サービス券Ｃ
	uchar			UseCount;										// 使用枚数
	ulong			DiscountMoney;									// 割引金額
	ulong			DiscountTime;									// 割引時間
	ushort			DiscountRate;									// 割引率
} TEIKI_CHUSI;

typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			Dummy;											// 送信先端末機械№
	ushort			ProcMode;										// 処理区分
	TEIKI_CHUSI		TeikiChusi[TKI_CYUSI_MAX];						// 各種制御
} DATA_KIND_91, DATA_KIND_219;

/*--------------------------------------------------------------------------*/
/*	定期券ｽﾃｰﾀｽﾃｰﾌﾞﾙ(ﾃﾞｰﾀ種別93,221)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	PAS_TBL			PassTable[PAS_MAX];								// 定期券ｽﾃｰﾀｽﾃｰﾌﾞﾙ
} DATA_KIND_93;

/*--------------------------------------------------------------------------*/
/*	定期券更新ﾃｰﾌﾞﾙ(ﾃﾞｰﾀ種別94,222)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	PAS_RENEWAL		PassRenewal[PAS_MAX/4];							// 定期券更新ﾃｰﾌﾞﾙ
} DATA_KIND_94;

/*--------------------------------------------------------------------------*/
/*	定期券出庫時刻ﾃｰﾌﾞﾙ(ﾃﾞｰﾀ種別95,223)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	PASS_EXTBL		PassExTable[PASS_EXTIMTBL_MAX];					// 定期券出庫時刻ﾃｰﾌﾞﾙ
} DATA_KIND_95;

/*--------------------------------------------------------------------------*/
/*	車室パラメータ受信(ﾃﾞｰﾀ種別97,225)										*/
/*--------------------------------------------------------------------------*/

typedef	struct {
	ushort		lok_syu;											// ﾛｯｸ装置種別（0:無し　１～６）
	ushort		ryo_syu;											// 料金種別	（0:無し　１～１２）
	ushort		area;												// 区画(1～26)
	ulong		posi;												// 駐車位置№(1～9999)
	ushort		if_oya;												// 親IF盤№（１～１０）
	ushort		lok_no;												// 親IF盤に対する装置連番（１～９０）
} LOCK_INFO;

typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			SMachineNo;										// 送信先端末機械№
	ulong			ModelCode;										// 設定データの機種コード
	LOCK_INFO		LockInfo[OLD_LOCK_MAX];							// 各種制御
} DATA_KIND_97;

typedef DATA_KIND_97	DATA_KIND_225;

/*--------------------------------------------------------------------------*/
/*	設定受信完了通知ﾃﾞｰﾀ(ﾃﾞｰﾀ種別99)										*/
/*--------------------------------------------------------------------------*/
// 設定完了したﾃﾞｰﾀ
enum {
	NTNET_COMPLETE_TIME=0,								// 時計ﾃﾞｰﾀ
	NTNET_COMPLETE_CTRL,								// 制御ﾃﾞｰﾀ
	NTNET_COMPLETE_CPRM,								// 共通設定ﾃﾞｰﾀ
	NTNET_COMPLETE_PPRM,								// 個別設定ﾃﾞｰﾀ
	NTNET_COMPLETE_UPRM,								// ﾕｰｻﾞｰﾊﾟﾗﾒｰﾀﾃﾞｰﾀ
	NTNET_COMPLETE_RSV1,								// (現在駐車台数ﾃﾞｰﾀ)
	NTNET_COMPLETE_RSV2,								// (満車台数ﾃﾞｰﾀ)
	NTNET_COMPLETE_HEADER,								// 領収証ﾍｯﾀﾞｰﾃﾞｰﾀ
	NTNET_COMPLETE_FOOTER,								// 領収証ﾌｯﾀｰﾃﾞｰﾀ
	NTNET_COMPLETE_LOGO,								// ﾛｺﾞ印字ﾃﾞｰﾀ
	NTNET_COMPLETE_BMP,									// ﾋﾞｯﾄﾏｯﾌﾟﾃﾞｰﾀ
	NTNET_COMPLETE_MUKOU,								// 無効駐車券ﾃｰﾌﾞﾙﾃﾞｰﾀ
	NTNET_COMPLETE_RSV3,								// 予備
	NTNET_COMPLETE_TEIKI_TYUSHI,						// 定期券精算中止ﾃｰﾌﾞﾙﾃﾞｰﾀ
	NTNET_COMPLETE_RSV4,								// 予備
	NTNET_COMPLETE_TEIKI_STS,							// 定期券ｽﾃｰﾀｽﾃｰﾌﾞﾙ
	NTNET_COMPLETE_TEIKI_UPDATE,						// 定期券更新ﾃｰﾌﾞﾙ
	NTNET_COMPLETE_TEIKI_CAROUT,						// 定期券出庫時刻ﾃｰﾌﾞﾙ
	NTNET_COMPLETE_SPDAY,								// 特別日設定ﾃﾞｰﾀ
	NTNET_COMPLETE_SHASHITSU_PRM,						// 車室ﾊﾟﾗﾒｰﾀ設定
	NTNET_COMPLETE_LOCK_PRM,							// ﾛｯｸ装置ﾊﾟﾗﾒｰﾀ設定
	NTNET_COMPLETE_USRSET,								// ﾕｰｻﾞｰ設定ﾃﾞｰﾀ
	NTNET_COMPLETE_REQ_TCKT,							// (発券要求)
	NTNET_COMPLETE_RSV5,								// (区画満空制御ﾃﾞｰﾀ)
	NTNET_COMPLETE_RSV6,								// (区画営休業制御ﾃﾞｰﾀ)
	NTNET_COMPLETE_PRKCTRL,								// 現在台数管理ﾃﾞｰﾀ
	NTNET_COMPLETE_RSV7,								// 予備

	NTNET_COMPLETE_MAX=70
};

typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			SMachineNo;										// 送信先端末機械№
	uchar			CompleteInfo[NTNET_COMPLETE_MAX];				// 設定完了情報
} DATA_KIND_99;

/*--------------------------------------------------------------------------*/
/*	ﾃﾞｰﾀ要求2結果NG(ﾃﾞｰﾀ種別110)											*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;
	ulong			SMachineNo;
	uchar			ControlData[20];
	ushort			Result;
	ulong			ErrTerminal[32];
} DATA_KIND_110;

/*--------------------------------------------------------------------------*/
/*	センタ用データ要求結果NG(ﾃﾞｰﾀ種別155)									*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;
	ushort			FmtRev;				// フォーマットRev.№
	ulong			SMachineNo;
	uchar			SeisanJyoutai_Req;	// 精算状態データ要求(0＝要求なし／1＝要求あり)
	uchar			RealTime_Req;		// リアルタイム情報要求(0＝要求なし／1＝要求あり)
	uchar			Reserve1;			// 予約1(0＝要求なし／1＝要求あり)
	uchar			Reserve2;			// 予約2(0＝要求なし／1＝要求あり)
	uchar			Reserve3;			// 予約3(0＝要求なし／1＝要求あり)
	uchar			Reserve4;			// 予約4(0＝要求なし／1＝要求あり)
	uchar			Reserve5;			// 予約5(0＝要求なし／1＝要求あり)
	uchar			Reserve6;			// 予約6(0＝要求なし／1＝要求あり)
	uchar			SynchroTime_Req;	// 同期時刻データ要求(0＝要求なし／1＝要求あり)
	uchar			TermInfo_Req;		// 端末情報データ要求(0＝要求なし／1＝要求あり)
	ushort			Result;				// 結果(9=パラメータNG)
	ulong			ErrTerminal[32];
} DATA_KIND_155;

/*--------------------------------------------------------------------------*/
/*	定期券データ更新　定期券チェックデータ(ﾃﾞｰﾀ種別116,117)					*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;
	ushort			ProcMode;										// 処理区分
	ulong			ParkingNo;										// 駐車場№
	ulong			PassID;											// 定期券ID
	uchar			Status;											// ステータス
	uchar			UpdateStatus;									// 更新ステータス
	ushort			OutYear;										// 出庫年
	uchar			OutMonth;										// 出庫月
	uchar			OutDay;											// 出庫日
	uchar			OutHour;										// 出庫時
	uchar			OutMin;											// 出庫分
	ulong			UsingParkingNo;									// 使用中駐車場№
} DATA_KIND_116, DATA_KIND_117;

#define	NTNET_PASSUPDATE_STATUS			0x00000001
#define	NTNET_PASSUPDATE_UPDATESTATUS	0x00000002
#define	NTNET_PASSUPDATE_OUTTIME		0x00000004

enum {
	NTNET_PASSUPDATE_INITIAL = 0,									// 初期状態		0
	NTNET_PASSUPDATE_OUT,											// 出庫中		1
	NTNET_PASSUPDATE_IN,											// 入庫中		2
	NTNET_PASSUPDATE_INVALID,										// 無効			3
	NTNET_PASSUPDATE_ALL_INITIAL,									// 全初期化		4
	NTNET_PASSUPDATE_ALL_IN,										// 全入庫中		5
	NTNET_PASSUPDATE_ALL_OUT,										// 全出庫中		6
	NTNET_PASSUPDATE_ALL_OUT_FREE,									// 全出庫フリー	7
	NTNET_PASSUPDATE_ALL_IN_FREE,									// 全入庫フリー	8
	NTNET_PASSUPDATE_ALL_INVALID,									// 全無効		9
	NTNET_PASSUPDATE_ALL_VALID,										// 全有効		10
	NTNET_PASSUPDATE_ALL_INITIAL2									// 全初期化（有効定期のみ）11
};

/*--------------------------------------------------------------------------*/
/*	モニタデータ(ﾃﾞｰﾀ種別122)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	uchar			Monsyu;											// モニタ種別
	uchar			Moncod;											// モニタコード
	uchar			dummy;											// 予約
	uchar			Monlev;											// モニタレベル
	uchar			Mondat1[10];									// モニタ情報
	uchar			Mondat2[160];									// モニタメッセージ（予約）
} DATA_KIND_122;


/*--------------------------------------------------------------------------*/
/*	操作モニタデータ(ﾃﾞｰﾀ種別123)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	uchar			OpeMonsyu;										// 操作モニタ種別
	uchar			OpeMoncod;										// 操作モニタコード
	uchar			OpeMonlev;										// 操作モニタレベル
	uchar			OpeMondat1[4];									// 変更前データ
	uchar			OpeMondat2[4];									// 変更後データ
	uchar			OpeMondat3[160];								// 操作モニタメッセージ（予約）
} DATA_KIND_123;

/*--------------------------------------------------------------------------*/
/*	ﾛｯｸ装置ﾊﾟﾗﾒｰﾀ(ﾃﾞｰﾀ種別98/226)											*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ushort			in_tm;											/* 入庫車両検知ﾀｲﾏｰ				*/
	ushort			ot_tm;											/* 出庫車両検知ﾀｲﾏｰ				*/
	ushort			r_cnt;											/* ﾛｯｸ装置ﾘﾄﾗｲ回数				*/
	ushort			r_tim;											/* ﾛｯｸ装置ﾘﾄﾗｲ間隔				*/
	ushort			open_tm;										/* 開動作信号出力時間			*/
	ushort			clse_tm;										/* 閉動作信号出力時間			*/
} t_LockMaker_NtNet;

typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			SMachineNo;										// 送信先端末機械№
	ulong			ModelCode;										// 設定データの機種コード
	t_LockMaker_NtNet	LockMaker[6];								// ﾛｯｸ装置ﾊﾟﾗﾒｰﾀ
} DATA_KIND_98;

typedef DATA_KIND_98 DATA_KIND_226;

/*--------------------------------------------------------------------------*/
/*	集計基本ﾃﾞｰﾀ(ﾃﾞｰﾀ種別30/42/54/66/158/170/182/194)						*/
/*--------------------------------------------------------------------------*/

typedef struct {
	ulong			Num;											// 回数
	ulong			Amount;											// 金額
} t_SyuSub;

typedef struct {
	ushort			Num;											// 回数
	ulong			Amount;											// 金額
} t_SyuSub2;

typedef struct {
	t_SyuSub		CarOutIllegal;									// 不正出庫
	t_SyuSub		CarOutForce;									// 強制出庫
	ulong			AcceptTicket;									// 受付券発行回数
	t_SyuSub		ModifySettle;									// 修正精算
	ulong			Rsv[2];											// 予備1～2
} t_SyuSub_AllSystem;

typedef struct {
	t_SyuSub		CarOutIllegal;									// 不正出庫
	t_SyuSub		CarOutForce;									// 強制出庫
	ulong			AcceptTicket;									// 受付券発行回数
	t_SyuSub		ModifySettle;									// 修正精算
} t_SyuSub_AllSystem2;

typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
	date_time_rec2	NowTime;										// 今回集計
	date_time_rec2	LastTime;										// 前回集計
	ulong			SettleNum;										// 総精算回数
	ulong			Kakeuri;										// 総掛売額
	ulong			Cash;											// 総現金売上額
	ulong			Uriage;											// 総売上額
	ulong			Tax;											// 総消費税額
	ulong			Charge;											// 釣銭払戻額
	ulong			CoinTotalNum;									// コイン金庫合計回数
	ulong			NoteTotalNum;									// 紙幣金庫合計回数
	ulong			CyclicCoinTotalNum;								// 循環コイン合計回数
	ulong			NoteOutTotalNum;								// 紙幣払出機合計回数
	ulong			Rsv1[3];										// 予備
	ulong			CarOutWithoutPay;								// 精算なしモード時出庫台数
	ulong			SettleNumServiceTime;							// サービスタイム内精算回数
	ulong			CarOutLagTime;									// ラグタイム内出庫回数
	t_SyuSub		Shortage;										// 払出不足
	t_SyuSub		Cancel;											// 精算中止
	t_SyuSub		AutoSettle;										// 半自動精算
	t_SyuSub		ManualSettle;									// マニュアル精算
	ulong			AntiPassOffSettle;								// アンチパスOFF精算回数
	ulong			CarOutGateOpen;									// ゲート開放時出庫台数
	ulong			CarOutForce;									// 強制出庫台数
	t_SyuSub		LostSettle;										// 紛失精算
	ulong			ReceiptCallback;								// 領収証回収枚数
	ulong			ReceiptIssue;									// 領収証発行枚数
	ulong			WarrantIssue;									// 預り証発行枚数
	t_SyuSub_AllSystem	AllSystem;									// 全装置
	ulong			CarInTotal;										// 総入庫台数
	ulong			CarOutTotal;									// 総出庫台数
	ulong			CarIn1;											// 入庫1入庫台数
	ulong			CarOut1;										// 出庫1出庫台数
	ulong			CarIn2;											// 入庫2入庫台数
	ulong			CarOut2;										// 出庫2出庫台数
	ulong			CarIn3;											// 入庫3入庫台数
	ulong			CarOut3;										// 出庫3出庫台数
	ulong			MiyoCount;										// 未入金回数
	ulong			MiroMoney;										// 未入金額
	ulong			LagExtensionCnt;								// ラグタイム延長回数
	ulong			Rsv2[11];										// 予備1～11
} DATA_KIND_30;


typedef DATA_KIND_30 DATA_KIND_66;
typedef DATA_KIND_30 DATA_KIND_158;
typedef DATA_KIND_30 DATA_KIND_170;
typedef DATA_KIND_30 DATA_KIND_182;
typedef DATA_KIND_30 DATA_KIND_194;

/*--------------------------------------------------------------------------*/
/*	料金種別毎集計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別31/43/55/67/159/171/183/195)					*/
/*--------------------------------------------------------------------------*/

typedef struct {
	ushort			Kind;											// 料金種別
	t_SyuSub		Settle;											// 精算売上
	t_SyuSub		Discount;										// 割引
} t_SyuSub_RyokinKind;

typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
	t_SyuSub_RyokinKind	Kind[50];									// 種別01～50
} DATA_KIND_31;

typedef DATA_KIND_31 DATA_KIND_55;
typedef DATA_KIND_31 DATA_KIND_67;
typedef DATA_KIND_31 DATA_KIND_159;
typedef DATA_KIND_31 DATA_KIND_171;
typedef DATA_KIND_31 DATA_KIND_183;
typedef DATA_KIND_31 DATA_KIND_195;

/*--------------------------------------------------------------------------*/
/*	分類集計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別32/44/56/68/160/172/184/196)						*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
	ulong			Kind;											// 駐車分類集計の種類
	t_SyuSub		Group[BUNRUI_CNT];								// 分類1～48
	t_SyuSub		GroupTotal;										// 分類以上
	ulong			LostSettle;										// 紛失精算 台数
	ulong			AntiPassOffSettle;								// アンチパスOFF精算 台数
	ulong			CarOutForce;									// 強制出庫 台数
	ulong			FirstTimePass;									// 初回定期 台数
	t_SyuSub		Unknown;										// 分類不明
	uchar			Rsv[32];										// 予備
	ulong			Kind2;											// 駐車分類集計の種類_2
	t_SyuSub		Group2[BUNRUI_CNT];								// 分類1～48_2
	t_SyuSub		GroupTotal2;										// 分類以上_2
	ulong			LostSettle2;										// 紛失精算 台数_2
	ulong			AntiPassOffSettle2;								// アンチパスOFF精算 台数_2
	ulong			CarOutForce2;									// 強制出庫 台数_2
	ulong			FirstTimePass2;									// 初回定期 台数_2
	t_SyuSub		Unknown2;										// 分類不明_2
	uchar			Rsv2[32];										// 予備_2
	ulong			Kind3;											// 駐車分類集計の種類_3
	t_SyuSub		Group3[BUNRUI_CNT];								// 分類1～48_3
	t_SyuSub		GroupTotal3;										// 分類以上_3
	ulong			LostSettle3;										// 紛失精算 台数_3
	ulong			AntiPassOffSettle3;								// アンチパスOFF精算 台数_3
	ulong			CarOutForce3;									// 強制出庫 台数_3
	ulong			FirstTimePass3;									// 初回定期 台数_3
	t_SyuSub		Unknown3;										// 分類不明_3
	uchar			Rsv3[32];										// 予備_3
} DATA_KIND_32;

typedef DATA_KIND_32 DATA_KIND_68;
typedef DATA_KIND_32 DATA_KIND_160;
typedef DATA_KIND_32 DATA_KIND_172;
typedef DATA_KIND_32 DATA_KIND_196;

/*--------------------------------------------------------------------------*/
/*	割引集計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別33/45/57/69/161/173/185/197)						*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			ParkingNo;										// 駐車場No.
	ushort			Kind;											// 割引種別
	ushort			Group;											// 割引区分
	ulong			Num;											// 割引回数
	ulong			Callback;										// 回収枚数
	ulong			Amount;											// 割引額
	ulong			Info;											// 割引情報
	ulong			Rsv;											// 予備
} t_SyuSub_Discount;

typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
	t_SyuSub_Discount	Discount[500];								// 割引 001～500
} DATA_KIND_33;

typedef DATA_KIND_33 DATA_KIND_57;
typedef DATA_KIND_33 DATA_KIND_69;
typedef DATA_KIND_33 DATA_KIND_161;
typedef DATA_KIND_33 DATA_KIND_173;
typedef DATA_KIND_33 DATA_KIND_197;

/*--------------------------------------------------------------------------*/
/*	定期集計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別34/46/58/70/162/174/186/198)						*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			ParkingNo;										// 駐車場No.
	ushort			Kind;											// 種別
	ulong			Num;											// 回数
	ulong			Callback;										// 回収枚数
	t_SyuSub		Update;											// 更新
	ulong			Rsv;											// 予備
} t_SyuSub_Pass;

typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
	t_SyuSub_Pass	Pass[100];										// 定期券 001～100
} DATA_KIND_34;

typedef DATA_KIND_34 DATA_KIND_70;
typedef DATA_KIND_34 DATA_KIND_162;
typedef DATA_KIND_34 DATA_KIND_174;
typedef DATA_KIND_34 DATA_KIND_198;

/*--------------------------------------------------------------------------*/
/*	車室毎集計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別35/47/59/71/163/175/187/199)						*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			LockNo;											// 区画情報
	ulong			CashAmount;										// 現金売上
	ulong			Settle;											// 精算回数
	t_SyuSub		CarOutIllegal;									// 不正出庫
	t_SyuSub		CarOutForce;									// 強制出庫
	ulong			AcceptTicket;									// 受付券発行回数
	t_SyuSub		ModifySettle;									// 修正精算
	ulong			Rsv[2];											// 予備1～2
} t_SyuSub_Lock;

typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
	t_SyuSub_Lock	Lock[OLD_LOCK_MAX];								// 車室001～324
} DATA_KIND_35;

typedef DATA_KIND_35 DATA_KIND_47;
typedef DATA_KIND_35 DATA_KIND_59;
typedef DATA_KIND_35 DATA_KIND_71;
typedef DATA_KIND_35 DATA_KIND_163;
typedef DATA_KIND_35 DATA_KIND_175;
typedef DATA_KIND_35 DATA_KIND_199;

/*--------------------------------------------------------------------------*/
/*	金銭集計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別36/48/60/72/164/176/188/200)						*/
/*--------------------------------------------------------------------------*/

typedef struct {
	ushort			Kind;											// 金種
	ulong			Num;											// 枚数
} t_SyuSub_Coin;

typedef struct {
	ushort			Kind;											// 金種
	ushort			Num;											// 枚数
} t_SyuSub_Coin2;

typedef struct {
	ushort			Kind;											// 金種
	ulong			Num1;											// 枚数1
	ulong			Num2;											// 枚数2
} t_SyuSub_Note;

typedef struct {
	ushort			Kind;											// 金種
	ushort			Num1;											// 枚数1
	ushort			Num2;											// 枚数2
} t_SyuSub_Note2;

typedef struct {
	ushort			CoinKind;										// コイン金種
	ulong			Accept;											// 入金枚数
	ulong			Pay;											// 出金枚数
	ulong			ChargeSupply;									// 釣銭補充枚数
	ulong			SlotInventory;									// インベントリ枚数(取出口)
	ulong			CashBoxInventory;								// インベントリ枚数(金庫)
	ulong			Hold;											// 保有枚数
} t_SyuSub_Cycle;

typedef struct {
	ushort			CoinKind;										// コイン金種
	ulong			Pay;											// 出金枚数
	ulong			ChargeSupply;									// 釣銭補充枚数
	ulong			SlotInventory;									// インベントリ枚数(取出口)
	ulong			CashBoxInventory;								// インベントリ枚数(金庫)
	ulong			Hold;											// 保有枚数
} t_SyuSub_Stock;

typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
	ulong			Total;											// 金庫総入金額
	ulong			NoteTotal;										// 紙幣金庫総入金額
	ulong			CoinTotal;										// コイン金庫総入金額
	t_SyuSub_Coin	Coin[4];										// コイン1～4
	t_SyuSub_Coin	CoinRsv[4];										// コイン予備1～4
	t_SyuSub_Note	Note[4];										// 紙幣1～4
	t_SyuSub_Note	NoteRsv[4];										// 紙幣予備1～4
	ulong			CycleAccept;									// 循環部総入金額
	ulong			CyclePay;										// 循環部総出金額
	ulong			NoteAcceptTotal;								// 紙幣総入金額
	ulong			NotePayTotal;									// 紙幣総払出金額
	ulong			StockPayTotal;									// 予蓄部総出金額
	t_SyuSub_Cycle	Cycle[4];										// 循環1～4
	t_SyuSub_Cycle	CycleRsv[4];									// 循環予備1～4
	t_SyuSub_Stock	Stock[2];										// 予蓄部1～2
	t_SyuSub_Note	NotePay[4];										// 紙幣払出1～4
	t_SyuSub_Note	NotePayRsv[4];									// 紙幣払出予備1～4
} DATA_KIND_36;

typedef DATA_KIND_36 DATA_KIND_60;
typedef DATA_KIND_36 DATA_KIND_72;
typedef DATA_KIND_36 DATA_KIND_164;
typedef DATA_KIND_36 DATA_KIND_176;
typedef DATA_KIND_36 DATA_KIND_200;

/*--------------------------------------------------------------------------*/
/*	集計終了通知ﾃﾞｰﾀ(ﾃﾞｰﾀ種別41/53/65/77/169/181/193/205)					*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
	ulong			MachineNo[32];									// 複数集計した端末の機会No.-001～-032
} DATA_KIND_41;

typedef DATA_KIND_41 DATA_KIND_77;
typedef DATA_KIND_41 DATA_KIND_169;
typedef DATA_KIND_41 DATA_KIND_181;
typedef DATA_KIND_41 DATA_KIND_193;
typedef DATA_KIND_41 DATA_KIND_205;

// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
///*--------------------------------------------------------------------------*/
///*	センターPhase2集計基本ﾃﾞｰﾀ(ﾃﾞｰﾀ種別42)									*/
///*--------------------------------------------------------------------------*/
//typedef struct {
//	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
//	ushort			FmtRev;											// フォーマットRev.№
//	ushort			Type;											// 集計タイプ
//	ushort			KakariNo;										// 係員No.
//	ulong			CenterSeqNo;									// センター追番
//	ulong			SeqNo;											// 集計追番
//	ulong			StartSeqNo;										// 開始追番
//	ulong			EndSeqNo;										// 終了追番
//	date_time_rec3	LastTime;										// 前回集計
//	ulong			SettleNum;										// 総精算回数
//	ulong			Kakeuri;										// 総掛売額
//	ulong			Cash;											// 総現金売上額
//	ulong			Uriage;											// 総売上額
//	ulong			Tax;											// 総消費税額
//	ulong			Charge;											// 釣銭払戻額
//	ushort			CoinTotalNum;									// コイン金庫合計回数
//	ushort			NoteTotalNum;									// 紙幣金庫合計回数
//	ushort			CyclicCoinTotalNum;								// 循環コイン合計回数
//	ushort			NoteOutTotalNum;								// 紙幣払出機合計回数
//	ulong			CarOutWithoutPay;								// 精算なしモード時出庫台数
//	ulong			SettleNumServiceTime;							// サービスタイム内精算回数
//	ulong			CarOutLagTime;									// ラグタイム内出庫回数
//	t_SyuSub		Shortage;										// 払出不足 回数／金額
//	t_SyuSub		Cancel;											// 精算中止 回数／金額
//	t_SyuSub		AutoSettle;										// 半自動精算 回数／金額
//	t_SyuSub		ManualSettle;									// マニュアル精算 回数／金額
//	ulong			AntiPassOffSettle;								// アンチパスOFF精算回数
//	ulong			CarOutGateOpen;									// ゲート開放時出庫台数
//	ulong			CarOutForce;									// 強制出庫台数
//	t_SyuSub		LostSettle;										// 紛失精算 回数／金額
//	ulong			ReceiptCallback;								// 領収証回収枚数
//	ulong			ReceiptIssue;									// 領収証発行枚数
//	ulong			WarrantIssue;									// 預り証発行枚数
//	t_SyuSub_AllSystem2	AllSystem;									// 全装置
//	ulong			CarInTotal;										// 総入庫台数
//	ulong			CarOutTotal;									// 総出庫台数
//	ulong			CarIn1;											// 入庫1入庫台数
//	ulong			CarOut1;										// 出庫1出庫台数
//	ulong			CarIn2;											// 入庫2入庫台数
//	ulong			CarOut2;										// 出庫2出庫台数
//	ulong			CarIn3;											// 入庫3入庫台数
//	ulong			CarOut3;										// 出庫3出庫台数
//	ulong			MiyoCount;										// 未入金回数
//	ulong			MiroMoney;										// 未入金額
//	ulong			LagExtensionCnt;								// ラグタイム延長回数
//	ulong			Total;											// 金庫総入金額
//	ulong			NoteTotal;										// 紙幣金庫総入金額
//	ulong			CoinTotal;										// コイン金庫総入金額
//	t_SyuSub_Coin2	Coin[4];										// コイン1～4
//	t_SyuSub_Note2	Note[4];										// 紙幣1～4
//} DATA_KIND_42;
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
// 仕様変更(S) K.Onodera 2016/11/04 集計基本データフォーマット対応
/*--------------------------------------------------------------------------*/
/*	センターPhase7集計基本ﾃﾞｰﾀ(ﾃﾞｰﾀ種別42)									*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			CenterSeqNo;									// センター追番
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
	date_time_rec3	LastTime;										// 前回集計
	ulong			SettleNum;										// 総精算回数
	ulong			Kakeuri;										// 総掛売額
	ulong			Cash;											// 総現金売上額
	ulong			Uriage;											// 総売上額
	ulong			Tax;											// 総消費税額
	ulong			Excluded;										// 総売上対象外金額
	ulong			Charge;											// 釣銭払戻額
	ushort			CoinTotalNum;									// コイン金庫合計回数
	ushort			NoteTotalNum;									// 紙幣金庫合計回数
	ushort			CyclicCoinTotalNum;								// 循環コイン合計回数
	ushort			NoteOutTotalNum;								// 紙幣払出機合計回数
	ulong			CarOutWithoutPay;								// 精算なしモード時出庫台数
	ulong			SettleNumServiceTime;							// サービスタイム内精算回数
	ulong			CarOutLagTime;									// ラグタイム内出庫回数
	t_SyuSub		Shortage;										// 払出不足 回数／金額
	t_SyuSub		Cancel;											// 精算中止 回数／金額
	t_SyuSub		AutoSettle;										// 半自動精算 回数／金額
	t_SyuSub		ManualSettle;									// マニュアル精算 回数／金額
	ulong			AntiPassOffSettle;								// アンチパスOFF精算回数
	ulong			CarOutGateOpen;									// ゲート開放時出庫台数
	ulong			CarOutForce;									// 強制出庫台数
	t_SyuSub		LostSettle;										// 紛失精算 回数／金額
	ulong			ReceiptCallback;								// 領収証回収枚数
	ulong			ReceiptIssue;									// 領収証発行枚数
	ulong			WarrantIssue;									// 預り証発行枚数
	t_SyuSub_AllSystem2	AllSystem;									// 全装置
	ulong			CarInTotal;										// 総入庫台数
	ulong			CarOutTotal;									// 総出庫台数
	ulong			CarIn1;											// 入庫1入庫台数
	ulong			CarOut1;										// 出庫1出庫台数
	ulong			CarIn2;											// 入庫2入庫台数
	ulong			CarOut2;										// 出庫2出庫台数
	ulong			CarIn3;											// 入庫3入庫台数
	ulong			CarOut3;										// 出庫3出庫台数
	ulong			MiyoCount;										// 未入金回数
	ulong			MiroMoney;										// 未入金額
	ulong			LagExtensionCnt;								// ラグタイム延長回数
	ulong			SaleParkingNo;									// 売上先駐車場№(0～999999   ※特注用（標準では基本駐車場№をセット）
	ulong			FurikaeCnt;										// 振替回数
	ulong			FurikaeTotal;									// 振替額
	ulong			RemoteCnt;										// 遠隔精算回数
	ulong			RemoteTotal;									// 遠隔精算金額
	ulong			Total;											// 金庫総入金額
	ulong			NoteTotal;										// 紙幣金庫総入金額
	ulong			CoinTotal;										// コイン金庫総入金額
	t_SyuSub_Coin2	Coin[4];										// コイン1～4
	t_SyuSub_Note2	Note[4];										// 紙幣1～4
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//} DATA_KIND_42_r13;
} DATA_KIND_42;
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
// 仕様変更(E) K.Onodera 2016/11/04 集計基本データフォーマット対応

/*--------------------------------------------------------------------------*/
/*	センターPhase2料金種別毎集計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別43)							*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			CenterSeqNo;									// センター追番
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
	t_SyuSub_RyokinKind	Kind[50];									// 種別01～50
} DATA_KIND_43;

/*--------------------------------------------------------------------------*/
/*	センターPhase2割引集計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別45)									*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			ParkingNo;										// 駐車場No.
	ushort			Kind;											// 割引種別
	ushort			Group;											// 割引区分
	ulong			Num;											// 割引回数
	ulong			Callback;										// 回収枚数
	ulong			Amount;											// 割引額
	ulong			Info;											// 割引情報
} t_SyuSub_Discount2;

typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			CenterSeqNo;									// センター追番
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
	t_SyuSub_Discount2	Discount[500];								// 割引 001～500
} DATA_KIND_45;

/*--------------------------------------------------------------------------*/
/*	センターPhase2定期集計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別46)									*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			ParkingNo;										// 駐車場No.
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//	ushort			Kind;											// 種別
	uchar			Mode;											// 処理区分
	uchar			Kind;											// 種別
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
	ulong			Num;											// 回数
	ulong			Callback;										// 回収枚数
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//	t_SyuSub		Update;											// 更新
	t_SyuSub2		Update;											// 更新
	t_SyuSub2		Sale;											// 販売
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
} t_SyuSub_Pass2_Pay;

// 仕様変更(S) K.Onodera 2016/12/14 集計基本データフォーマット対応
typedef struct{
	uchar			syu;											// 種別
	ulong			num;											// 回数
	ulong			uri;											// 売上金額
} t_Teiki_Option;
// 仕様変更(E) K.Onodera 2016/12/14 集計基本データフォーマット対応

typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			CenterSeqNo;									// センター追番
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
// 仕様変更(S) K.Onodera 2016/12/14 集計基本データフォーマット対応
	t_Teiki_Option	Option[6];										// オプション情報
// 仕様変更(E) K.Onodera 2016/12/14 集計基本データフォーマット対応
	t_SyuSub_Pass2_Pay	Pass[100];									// 定期券 001～100
} DATA_KIND_46;

/*--------------------------------------------------------------------------*/
/*	センターPhase2集計終了通知ﾃﾞｰﾀ(ﾃﾞｰﾀ種別53)								*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			CenterSeqNo;									// センター追番
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
} DATA_KIND_53;

/*--------------------------------------------------------------------------*/
/*	ﾊﾟｽﾜｰﾄﾞﾃﾞｰﾀ(ﾃﾞｰﾀ種別84/212)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	uchar			PassWord[4];							// ﾊﾟｽﾜｰﾄﾞ
	uchar			PassWord2[4];							// ﾊﾟｽﾜｰﾄﾞ2
	uchar			Reserve[4];								// 予備
} DATA_KIND_84_DATA;
typedef	struct {
	DATA_BASIC			DataBasic;							// 基本ﾃﾞｰﾀ
	DATA_KIND_84_DATA	Data;								// ﾊﾟｽﾜｰﾄﾞﾃﾞｰﾀ
} DATA_KIND_84;

typedef DATA_KIND_84	DATA_KIND_212;

/*--------------------------------------------------------------------------*/
/*	係員ID状態データ、係員ID問合せデータ、係員保守データ(ﾃﾞｰﾀ種別254/255/151/108)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	ushort			Kubun;									// 処理区分 (未使用)
	ulong			ParkingNo;								// 駐車場№
	ushort			KakariNo;								// 係員No.
	uchar			Status;									// ステータス、予備
} DATA_KIND_254;

typedef DATA_KIND_254	DATA_KIND_255;
typedef DATA_KIND_254	DATA_KIND_151;
typedef DATA_KIND_254	DATA_KIND_108;

/*--------------------------------------------------------------------------*/
/*	特別日設定データ(ﾃﾞｰﾀ種別83/211)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ushort			sta_Mont;								// 開始月
	ushort			sta_Date;								// 開始日
	ushort			end_Mont;								// 終了月
	ushort			end_Date;								// 終了日
} t_splday_kikan;

typedef	struct {
	ushort			mont;									// 月
	ushort			day;									// 日
} t_splday_date;

typedef	struct {
	ushort			mont;									// 月
	ushort			week;									// 週
	ushort			yobi;									// 曜日
} t_splday_yobi;

typedef	struct {
	ushort			year;									// 年
	ushort			mont;									// 月
	ushort			day;									// 日
} t_splday_year;

typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	ushort			rsv1;									// 予備１
	ushort			rsv2;									// 予備２
	t_splday_kikan	kikan[3];								// 特別期間１～３
	t_splday_date	date[31];								// 特別日１～３１
	ushort			Shift[31];								// シフト１～３１
	t_splday_yobi	yobi[12];								// 特別曜日１～１２
	ushort			spl56;									// 5／6を特別日とする
	t_splday_year	year[6];								// 特別年月日１～６
} DATA_KIND_83;

typedef DATA_KIND_83	DATA_KIND_211;

#define		NTNET_SPLTRM_START		 1		// 特別期間の開始位置
#define		NTNET_SPLTRM_END		 6		// 特別期間の終了位置
#define		NTNET_SPLDAY_START		 9		// 特別日の開始位置
#define		NTNET_SPLDAY_END		39		// 特別日の終了位置
#define		NTNET_HMON_START		42		// ﾊｯﾋﾟｰﾏﾝﾃﾞｰの開始位置
#define		NTNET_HMON_END			47		// ﾊｯﾋﾟｰﾏﾝﾃﾞｰの終了位置
#define		NTNET_5_6				49		// 「5月6日を特別日とする」の位置
#define		NTNET_SPYEAR_START		50		// 特別年月日の開始位置
#define		NTNET_SPYEAR_END		55		// 特別年月日の終了位置
#define		NTNET_SPYOBI_START		56		// 特別曜日の開始位置
#define		NTNET_SPYOBI_END		67		// 特別曜日の終了位置

/*--------------------------------------------------------------------------*/
/*	設定ﾃﾞｰﾀ要求(ﾃﾞｰﾀ種別78,90)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	ulong			SMachineNo;								// 送信先端末機械№
	ulong			ModelCode;								// 設定データの機種コード
	ushort			PrmDiv;									// ﾊﾟﾗﾒｰﾀ区分
	ushort			Segment;								// 設定ｾｸﾞﾒﾝﾄ
	ushort			TopAddr;								// 開始ｱﾄﾞﾚｽ
	ushort			DataCount;								// 設定ﾃﾞｰﾀ数
	uchar			Reserve[72];							// 予備
} DATA_KIND_78;

/*--------------------------------------------------------------------------*/
/*	設定ﾃﾞｰﾀ送信要求(ﾃﾞｰﾀ種別114)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	ulong			SMachineNo;								// 送信先端末機械№
	SETUP_NTNETDATA	Req;									// 設定ﾃﾞｰﾀ
	uchar			Reserve[60];							// 予備
} DATA_KIND_114;

// リパーク専用フォーマットとする
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			SMachineNo;										// 送信先端末機械№
	uchar			reqSend;			// 送信要求：0(なし）、1（あり）、2（キャンセル）
	uchar			reqExec;			// 実行要求：0(なし）、1（あり）
	uchar			reserve[7];			// 予備
	uchar			reserve2[60];		// 予備
} RP_DATA_KIND_114;

/*--------------------------------------------------------------------------*/
/*	設定ﾃﾞｰﾀ送信応答(ﾃﾞｰﾀ種別115)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	ulong			SMachineNo;								// 送信元端末機械№
	uchar			ProcMode;								// 処理区分
	uchar			Reserve[8];								// 予備1～8
	uchar			Reserve2[60];							// 予備
} DATA_KIND_115;

// リパーク専用フォーマットとする
typedef	DATA_KIND_115	RP_DATA_KIND_115;

/*--------------------------------------------------------------------------*/
/*	コイン・紙幣金庫集計データ（データ種別130/131/132/133)					*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	short			Money;									// 金額
	long			Mai;									// 保有枚数
} MONEY_DATA2;

typedef	struct {
	DATA_BASIC		DataBasic;								// 基本データ
	ushort			Type;									// 集計タイプ
	ushort			KakariNo;								// 係員No.
	ulong			SeqNo;									// 集計追番
	ulong			StartSeqNo;								// 開始追番
	ulong			EndSeqNo;								// 終了追番
	union {
		MONEY_DATA2		Coin[8];							// コイン金庫データ
		MONEY_DATA2		Note[8];							// 紙幣金庫データ
	} Sf;
	ulong			SfTotal;								// 金庫総入金額
} DATA_KIND_130;

typedef DATA_KIND_130	DATA_KIND_132;

/*--------------------------------------------------------------------------*/
/*	動作カウントデータ（データ種別228)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// 基本データ
	ulong			MachineNo;								// 送信元端末機械№
	ulong			Count[100];								// 動作カウント
} DATA_KIND_228;

/*--------------------------------------------------------------------------*/
/*	駐車台数管理データ(ﾃﾞｰﾀ種別234/235)										*/
/*--------------------------------------------------------------------------*/
typedef struct {
	long			mode;									// 強制満空車ﾓｰﾄﾞ
	long			car_cnt;								// 現在台数
	long			ful_cnt;								// 満車台数
	long			kai_cnt;								// 満車解除台数
} PARK_NUM_CTRL_Sub;

typedef	struct {
	ulong			CurNum;									// 現在駐車台数
	ulong			NoFullNum;								// 満車解除台数
	ulong			FullNum;								// 満車台数
} PARK_NUM_CTRL;

typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	ushort			GroupNo;								// ｸﾞﾙｰﾌﾟ番号
	ulong			Reserve1;								// 予備１
	ulong			State;									// 用途別駐車台数設定
	PARK_NUM_CTRL	Data[3];								// 駐車台数管理ﾃﾞｰﾀ01～03
	ulong			Reserve[27];							// 予備
} DATA_KIND_234;

typedef DATA_KIND_234	DATA_KIND_235;

typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	ushort			GroupNo;								// ｸﾞﾙｰﾌﾟ番号
	ulong			CurNum;									// 現在駐車台数
	ulong			TkCurNum[15];							// 定期種別1～15現在駐車台数
	ulong			RyCurNum[20];							// 料金種別1～20現在駐車台数
	ulong			State;									// 用途別駐車台数設定
	ulong			Full[3];								// 駐車1～3満空状態
	ulong			EmptyNo1;								// 空車台数１
	ulong			FullNo1;								// 満車台数１
	ulong			EmptyNo2;								// 空車台数２
	ulong			FullNo2;								// 満車台数２
	ulong			EmptyNo3;								// 空車台数３
	ulong			FullNo3;								// 満車台数３
	ulong			Reserve[4];								// 予備
} DATA_KIND_236;

/*--------------------------------------------------------------------------*/
/*	遠隔定期券ｽﾃｰﾀｽﾃｰﾌﾞﾙ(ﾃﾞｰﾀ種別92)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	ushort			ProcMode;								// 処理区分
	PAS_TBL			PassTable[PAS_MAX];						// 定期券ｽﾃｰﾀｽﾃｰﾌﾞﾙ
} DATA_KIND_92;

/*--------------------------------------------------------------------------*/
/*	管理データ要求NG(ﾃﾞｰﾀ種別105)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	ulong			TermNo;									// 送信元ﾀｰﾐﾅﾙNo
	ushort			Result;									// 結果
} DATA_KIND_105;

/*--------------------------------------------------------------------------*/
/*	遠隔制御ﾃﾞｰﾀ(ﾃﾞｰﾀ種別240)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	ushort			GroupNo;								// グループNo
	uchar			ControlData[12];						// 各種制御
} DATA_KIND_240;

/*--------------------------------------------------------------------------*/
/*	遠隔ﾃﾞｰﾀ要求(ﾃﾞｰﾀ種別243)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	ushort			GroupNo;								// グループNo
	uchar			ControlData[20];						// 各種要求
} DATA_KIND_243;

/*--------------------------------------------------------------------------*/
/*	遠隔データ要求応答NG(ﾃﾞｰﾀ種別244)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	ushort			GroupNo;								// グループNo
	uchar			ControlData[20];						// 各種要求
	ushort			Result;									// 結果
	ulong			MachineNo[32];							// 障害端末のターミナルNo01～32機械No
} DATA_KIND_244;

/*--------------------------------------------------------------------------*/
/*	全車室情報データ(ﾃﾞｰﾀ種別245)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	ushort			GroupNo;								// グループNo
	LOCK_STATE		LockState[OLD_LOCK_MAX];				// 車室情報
} DATA_KIND_245;

/*--------------------------------------------------------------------------*/
/*	遠隔固有電文用基本ﾃﾞｰﾀ2													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	uchar			SystemID;										// ｼｽﾃﾑID
	uchar			DataKind;										// ﾃﾞｰﾀ種別
	uchar			DataKeep;										// ﾃﾞｰﾀ保持ﾌﾗｸﾞ
} DATA_BASIC_R;

/*--------------------------------------------------------------------------*/
/*	遠隔IBK制御ﾃﾞｰﾀ(ID60/80)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC_R	DataBasicR;								// 基本ﾃﾞｰﾀ
	uchar			Data[18];								// 要求ﾃﾞｰﾀ
} DATA_KIND_60_R;

/*--------------------------------------------------------------------------*/
/*	送信・ｸﾘｱ要求ﾃﾞｰﾀ(ID61・62/81・82)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC_R	DataBasicR;								// 基本ﾃﾞｰﾀ
	uchar			Data[20];								// 要求ﾃﾞｰﾀ
} DATA_KIND_61_R;

/*--------------------------------------------------------------------------*/
/*	ﾃｰﾌﾞﾙ件数ﾃﾞｰﾀ(ID63/83)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC_R	DataBasicR;								// 基本ﾃﾞｰﾀ
	ushort			Data[19];								// ﾃｰﾌﾞﾙ件数＆予備
} DATA_KIND_83_R;

/*--------------------------------------------------------------------------*/
/*	通信ﾁｪｯｸ要求／結果ﾃﾞｰﾀ(ID100/101)										*/
/*--------------------------------------------------------------------------*/
typedef struct {
	ulong			ParkingNo;								// 駐車場№
	ushort			ModelCode;								// 機種ｺｰﾄﾞ
	ulong			SMachineNo;								// 送信元№
	uchar			SerialNo[6];							// 端末ｼﾘｱﾙ№
	uchar			Year;									// 年
	uchar			Mon;									// 月
	uchar			Day;									// 日
	uchar			Hour;									// 時
	uchar			Min;									// 分
	uchar			Sec;									// 秒
	uchar			ChkNo;									// 通信ﾁｪｯｸNo.
} DATA_KIND_100R_sub;

typedef	struct {
	DATA_BASIC_R		DataBasicR;							// 基本ﾃﾞｰﾀ
	uchar				SeqNo;								// ｼｰｹﾝｼｬﾙ№
	DATA_KIND_100R_sub	sub;								// 識別用ｻﾌﾞﾃﾞｰﾀ
	uchar				Data[26];							// 要求：要求種別(1byte)
															//       ﾀｲﾑｱｳﾄ(1byte)
															//       予備(24byte)
															// 結果：結果ｺｰﾄﾞ(1byte)
															//       予備(25byte)
} DATA_KIND_100_R;

/*--------------------------------------------------------------------------*/
/*	ニアフル通知データ(ID90)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC_R	DataBasicR;								// 基本ﾃﾞｰﾀ
	uchar			Data[20];								// ニアフル通知(13byte)
															//       予備(7byte)
} DATA_KIND_90_R;

typedef	struct {
	uchar		card_use;				// 利用可・不可
	ushort		Base_Amount;
	ushort		Base_Point;
	ushort		Time_Amount;
	ushort		Time_Point;
	t_TIME_INFO Start;
	t_TIME_INFO End;
	uchar		Reserve2[181];
} DATA_KIND_184_01;

typedef	struct {
	uchar		Reserve2[200];
} DATA_KIND_184_02;

typedef	struct {
	uchar		Id_BusCode[2];
	uchar		Company[4];
	uchar 		MemberNo_S[BIN_KETA_MAX];
	uchar 		MemberNo_E[BIN_KETA_MAX];
	uchar		Field;
	uchar		Partner_Code2[4];				// 提携先コード２
	uchar		Ptr2_Chk;						// 提携先コード２判定方式
	uchar		Member_Chk;						// 会員番号判定方式
	uchar		Reserve[10];					// 予備（将来拡張予約）
} W_CARD_INFO;

typedef	struct {
	uchar		kind;				// カード種別
	uchar		kubun;				// 制御指示
	W_CARD_INFO	W_Card;				// カードデータ
	uchar		Reserve2[143];
} DATA_KIND_184_03;

typedef	struct {
	uchar		kind;				// カード種別
	uchar		BlockNo;
	uchar		Reserve2[198];
} DATA_KIND_184_04;

typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	ushort			ReqKind;
	ushort			Reserve;
	union{
		DATA_KIND_184_01	Data01;
		DATA_KIND_184_02	Data02;
		DATA_KIND_184_03	Data03;
		DATA_KIND_184_04	Data04;
	}RcvData;
} DATA_KIND_184;


typedef	struct {
	uchar		Reserve[200];
} DATA_KIND_185_31;

typedef	struct {
	uchar		card_use;				// 利用可・不可
	ushort		Base_Amount;
	ushort		Base_Point;
	ushort		Time_Amount;
	ushort		Time_Point;
	t_TIME_INFO Start;
	t_TIME_INFO End;
	uchar		Reserve2[181];
} DATA_KIND_185_32;

typedef	struct {
	uchar		kind;				// カード種別
	uchar		Reserve[199];
} DATA_KIND_185_33;

typedef	struct {
	uchar		kind;				// カード種別
	uchar		BlockNo;
	W_CARD_INFO	W_Card[BLOCK_MAX];
	uchar		Reserve2[88];
} DATA_KIND_185_34;

typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	ushort			ReqKind;
	ushort			Result;
	union{
		DATA_KIND_185_31	Data01;
		DATA_KIND_185_32	Data02;
		DATA_KIND_185_33	Data03;
		DATA_KIND_185_34	Data04;
	}SndData;
} DATA_KIND_185;

typedef	struct {
	ulong		LockNo;
	t_TIME_INFO	InCarTime;
	uchar		Reserve;
} PARKING_INFO;

typedef	struct {
	DATA_BASIC		DataBasic;					// 基本ﾃﾞｰﾀ
	ushort			Kind;
	ushort			ParkingTerm;
	ushort			ParkingNum;
	PARKING_INFO	ParkInfo[8];
	uchar			Reserve[10];
} DATA_KIND_14;

/*--------------------------------------------------------------------------*/
/*	ｸﾚｼﾞｯﾄ問合せﾃﾞｰﾀ(ID148/149)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	uchar			DataKind;								// 電文種別
	uchar			DataOiban;								// 処理追番
	ulong			SaleOiban;								// 精算追番
	ushort			Year;									// 精算年
	uchar			Mon;									//     月
	uchar			Day;									//     日
	uchar			Hour;									//     時
	uchar			Min;									//     分
	ushort			Sec;									//     秒
	ulong			Slip_No;								// 伝票番号
	ulong			sale_ryo;								// 売上金額
	ushort			pay_kind;								// 支払方法
	ushort			share_count;							// 分割回数
	uchar			yobi[12];								// 予備
	uchar			jis2_data[69];							// JIS2ｶｰﾄﾞﾃﾞｰﾀ
	uchar			jis1_data[37];							// JIS1ｶｰﾄﾞﾃﾞｰﾀ
	uchar			card_no[16];							// 法人カード番号
	uchar			use_limit[4];							// 有効期限
	uchar			Type;									// 機能種別
	uchar			Partner_Code2[4];						// 提携先コード２
	uchar			Ptr2_Chk;								// 提携先コード２判定方式
	uchar			Reserve[3];								// 予備(将来拡張予約)
	uchar			yobi2[4];								// 予備
} DATA_KIND_148;

typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	uchar			DataKind;								// 電文種別
	uchar			DataOiban;								// 処理追番
	ulong			SaleOiban;								// 精算追番
	ushort			Year;									// 精算年
	uchar			Mon;									//     月
	uchar			Day;									//     日
	uchar			Hour;									//     時
	uchar			Min;									//     分
	ushort			Sec;									//     秒
	ulong			slip_no;								// 伝票番号
	ulong			sale_ryo;								// 売上金額
	ushort			pay_kind;								// 支払方法
	ushort			share_count;							// 分割回数
	uchar			inq_result1[2];							// 照会結果①
	uchar			inq_result2[2];							// 照会結果②
	ulong			app_no;									// 承認番号
	uchar			card_kind[2];							// ｶｰﾄﾞ種別
	uchar			company_code[4];						// 会社ｺｰﾄﾞ
	uchar			company_name[10];						// 会社名
	uchar			member_code[20];						// 会員番号
	uchar			member_name[10];						// 会員名
	uchar			card_use_limmit_y;						// ｶｰﾄﾞ有効期限（年）	
	uchar			card_use_limmit_m;						// ｶｰﾄﾞ有効期限（月）
	uchar			Tcard_use;								// Ｔカード併用
	uchar			yobi2[86];								// 予備
} DATA_KIND_149;

typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	uchar			ReqKind;								// 要求種別
	uchar			DL_Year;								// ダウンロード時刻（年）
	uchar			DL_Mon;									// ダウンロード時刻（月）
	uchar			DL_Day;									// ダウンロード時刻（日）
	uchar			DL_Hour;								// ダウンロード時刻（時）
	uchar			DL_Min;									// ダウンロード時刻（分）
	uchar			SW_Year;								// プログラム更新時刻（年）
	uchar			SW_Mon;									// プログラム更新時刻（月）
	uchar			SW_Day;									// プログラム更新時刻（日）
	uchar			SW_Hour;								// プログラム更新時刻（時）
	uchar			SW_Min;									// プログラム更新時刻（分）
	uchar			Prog_Info[15];							// メインプログラム情報
	uchar			Yobi[5];								// 予備エリア
} DATA_KIND_188;

typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	uchar			ErrCode;								// エラーコード
	uchar			ReqKind;								// 要求種別
	uchar			yobi[5];								// 予備
} DATA_KIND_189;


typedef	struct {
	DATA_BASIC		DataBasic;								// 基本ﾃﾞｰﾀ
	uchar			Series_Name[12];						// シリーズ名称
	uchar			Version[9];								// 精算機機能バージョン
	uchar			MachineNo;								// 精算機機械No
	ulong			DeviceID1;								// 精算機デバイスID1（代表）
	ulong			DeviceID2;								// 精算機デバイスID2
	ulong			DeviceID3;								// 精算機デバイスID3
	uchar			T_Card;									// Tカード
	uchar			Credit;									// クレジットカード
	uchar			Web_Money;								// 電子マネー
	uchar			Corporate;								// 法人カード
	uchar			Reserve[30];							// 予備（将来拡張予約）
} DATA_KIND_190;




/*--------------------------------------------------------------------------*/
/*	リアルタイム情報データ(ﾃﾞｰﾀ種別153)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {	// 精算機金銭情報
	ushort			ModelCode;		// 機種コード(001～999) 0=情報なし
	ushort			MachineNo;		// 送信元端末機械№(01～99)  0=情報なし
	ushort			Result;			// 要求結果(0＝正常、1＝通信障害中、2＝タイムアウト)
	ulong			Uriage;			// 総売上額					0～999999
	ulong			Cash;			// 総現金売上額				0～999999
// MH322917(S) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	uchar			Date_Uriage_use;// 日付切替時刻基準 使用有無 0＝未使用、1＝使用
	uchar			Reserve1;		// 予備(サイズ調整用)
	ulong			Date_Uriage;	// 日付切替時刻基準 総売上額      0～999999
	ulong			Date_Cash;		// 日付切替時刻基準 総現金売上額  0～999999
	uchar			Date_hour;		// 日付切替時刻基準 切替時 0～23
	uchar			Date_min;		// 日付切替時刻基準 切替分 0～12
// MH322917(E) A.Iiizumi 2018/08/30 リアルタイム情報 フォーマットRev11対応
	ulong			CoinTotal;		// コイン金庫総入金額			0～999999
	ulong			NoteTotal;		// 紙幣金庫総入金額			0～999999
	uchar			CoinReceive;	// コイン金庫収納率(%)
	uchar			Reserve;		// 予備(サイズ調整用)
	ushort			NoteReceive;	// 紙幣金庫収納枚数
	uchar			KinsenKanriFlag;// 金銭管理あり/なし			0＝あり/1＝なし
	uchar			TuriStatus;		// 釣銭切れ状態	0＝釣銭あり/1＝釣銭切れ
									// 	1bit目:10円、2bit目:50円、3bit目:100円、4bit目:500円
									// 	5bit目:1000円、6bit目:2000円、7bit目:5000円
	ushort			TuriMai_10;		// 釣銭情報	10円保有枚数	0～9999
	ushort			TuriMai_50;		// 			50円保有枚数	
	ushort			TuriMai_100;	// 			100円保有枚数	
	ushort			TuriMai_500;	// 			500円保有枚数	
	ushort			Yochiku1_shu;	// 予蓄1(SUB筒)	金種金額	0＝非搭載　10/50/100/500
	ushort			Yochiku1_mai;	// 				保有枚数	0～9999
	ushort			Yochiku2_shu;	// 予蓄2		金種金額	0＝非搭載　10/50/100/500/1000/2000/5000/10000
	ushort			Yochiku2_mai;	// 				保有枚数	0～9999
	ushort			Yochiku3_shu;	// 予蓄3		金種金額	0＝非搭載　10/50/100/500/1000/2000/5000/10000
	ushort			Yochiku3_mai;	// 				保有枚数	0～9999
	ushort			Yochiku4_shu;	// 予蓄4		金種金額	0＝非搭載　10/50/100/500/1000/2000/5000/10000
	ushort			Yochiku4_mai;	// 				保有枚数	0～9999
	ushort			Yochiku5_shu;	// 予蓄5		金種金額	0＝非搭載　10/50/100/500/1000/2000/5000/10000
	ushort			Yochiku5_mai;	// 				保有枚数	0～9999
} t_MacMoneyInfo;
typedef	struct {
	DATA_BASIC		DataBasic;		// 基本ﾃﾞｰﾀ
	ushort			FmtRev;			// フォーマットRev.№
	PARKCAR_DATA11	ParkData;		// 駐車台数
	t_MacMoneyInfo	MoneyInfo[32];	// 精算機金銭情報
} DATA_KIND_153;	// リアルタイム情報データ
/*--------------------------------------------------------------------------*/
/*	精算情報ﾃﾞｰﾀ(ﾃﾞｰﾀ種別152)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	uchar		KyugyoStatus;		//	機器状態_営休業		0：営業/1：休業
	uchar		Lc1Status;			//	機器状態_ＬＣ1状態	0：ＯＦＦ/1：ＯＮ
	uchar		Lc2Status;			//	機器状態_ＬＣ2状態	0：ＯＦＦ/1：ＯＮ
	uchar		Lc3Status;			//	機器状態_ＬＣ3状態	0：ＯＦＦ/1：ＯＮ
	uchar		GateStatus;			//	機器状態_ゲート状態	0：閉/1：開
	uchar		NgCardRead;			//	NGカード読取内容	1	30		0＝なし, 1～255＝NT-NETのアラーム種別02のコード内容と同じ
} t_KikiStatus;

typedef	struct {
	uchar		ErrCoinmech;		//	コインメック	エラー種別03のコード内容と同じ(レベルの高いものからセットする)
	uchar		ErrNoteReader;		//	紙幣リーダー	エラー種別04のコード内容と同じ(レベルの高いものからセットする)
	uchar		ErrGikiReader;		//	磁気リーダー	エラー種別01のコード内容と同じ(レベルの高いものからセットする)
	uchar		ErrNoteHarai;		//	紙幣払出機		エラー種別05のコード内容と同じ(レベルの高いものからセットする)
	uchar		ErrCoinJyunkan;		//	コイン循環		エラー種別06のコード内容と同じ(レベルの高いものからセットする)
	uchar		ErrNoteJyunkan;		//	紙幣循環		エラー種別14のコード内容と同じ(レベルの高いものからセットする)
} t_Erroring;		// 発生中エラー
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
//typedef	struct {
//	DATA_BASIC		DataBasic;			// 基本ﾃﾞｰﾀ
//	ushort			FmtRev;				// フォーマットRev.№
//	t_KikiStatus	Kiki;				// 機器状態
//	t_Erroring		Err;				// 発生中エラー
//	ulong			PayCount;			// 精算追番(0～99999)
//	uchar			PayMethod;			// 精算方法(0＝券なし精算/1＝駐車券精算/2＝定期券精算/3＝定期券併用精算/4＝紛失精算)
//	uchar			PayClass;			// 処理区分(0＝精算/1＝再精算/2＝精算中止/3＝再精算中止, 8=精算前, 9=精算途中
//	uchar			PayMode;			// 精算モード(0＝自動精算/1＝半自動精算/2＝手動精算/3＝精算なし/4＝遠隔精算)
//										//				20＝Mifareプリペイド精算
//	uchar			CMachineNo;			// 駐車券機械№(入庫機械№)	0～255
//	ushort			FlapArea;			// フラップシステム	区画		0～99
//	ushort			FlapParkNo;			// 					車室番号	0～9999
//	ushort			KakariNo;			// 係員№	0～9999
//	uchar			OutKind;			// 精算出庫	0＝通常精算/1＝強制出庫/(2＝精算なし出庫)/3＝不正出庫/9＝突破出庫
//										//				10＝ゲート開放/20＝Mifareプリペイド精算出庫
//										//				97＝ロック開・フラップ上昇前未精算出庫/98＝ラグタイム内出庫
//										//				99＝サービスタイム内出庫
//	uchar			CountSet;			// 在車カウント	0＝する(+1)/1＝しない/2＝する(-1)
//										// (データ種別が「22：事前精算」の時は「1=しない」となる)
//	t_InPrevYMDHMS	InPrev_ymdhms;		// 入庫/前回精算_YMDHMS
//	uchar			ReceiptIssue;		// 領収証発行有無	0＝領収証なし/1＝領収証あり
//	uchar			Syubet;				// 料金種別			1～
//	ulong			Price;				// 駐車料金			0～
//	ulong			CashPrice;			// 現金売上			0～
//	ulong			InPrice;			// 投入金額			0～
//	ushort			ChgPrice;			// 釣銭金額			0～9999
//	t_MoneyInOut	MoneyInOut;			// 金銭情報(投入金枚数/払出金枚数)
//	ushort			HaraiModoshiFusoku;	// 払戻不足額	0～9999
//	uchar			Reserve1;			// 予備(サイズ調整用)	0
//	uchar			AntiPassCheck;		// アンチパスチェック	0＝チェックON/1＝チェックＯＦＦ/2＝強制ＯＦＦ
//										//						※定期を使用しない場合は「１＝チェックＯＦＦ」固定
//	ulong			ParkNoInPass;		// 定期券　駐車場№	0～999999
//	t_MediaInfo		Media[2];			// 精算媒体情報1, 2
//	ushort			CardKind;			// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
//										//					3=Edy、4=PiTaPa、5=WAON　　90以降=個別対応
//	uchar			settlement[64];		// 決済情報			(※3)　暗号化対象
//	t_SeisanDiscount	SDiscount[NTNET_DIC_MAX];	// 割引
//} DATA_KIND_152;	// 精算情報ﾃﾞｰﾀ
typedef	struct {
	DATA_BASIC		DataBasic;			// 基本ﾃﾞｰﾀ
	ushort			FmtRev;				// フォーマットRev.№
	t_KikiStatus	Kiki;				// 機器状態
	t_Erroring		Err;				// 発生中エラー
	ulong			PayCount;			// 精算追番(0～99999)
	uchar			PayMethod;			// 精算方法(0＝券なし精算/1＝駐車券精算/2＝定期券精算/3＝定期券併用精算/4＝紛失精算)
	uchar			PayClass;			// 処理区分(0＝精算/1＝再精算/2＝精算中止/3＝再精算中止, 8=精算前, 9=精算途中
	uchar			PayMode;			// 精算モード(0＝自動精算/1＝半自動精算/2＝手動精算/3＝精算なし/4＝遠隔精算)
										//				20＝Mifareプリペイド精算
	uchar			CMachineNo;			// 駐車券機械№(入庫機械№)	0～255
	ushort			FlapArea;			// フラップシステム	区画		0～99
	ushort			FlapParkNo;			// 					車室番号	0～9999
	ushort			KakariNo;			// 係員№	0～9999
	uchar			OutKind;			// 精算出庫	0＝通常精算/1＝強制出庫/(2＝精算なし出庫)/3＝不正出庫/9＝突破出庫
										//				10＝ゲート開放/20＝Mifareプリペイド精算出庫
										//				97＝ロック開・フラップ上昇前未精算出庫/98＝ラグタイム内出庫
										//				99＝サービスタイム内出庫
	uchar			CountSet;			// 在車カウント	0＝する(+1)/1＝しない/2＝する(-1)
										// (データ種別が「22：事前精算」の時は「1=しない」となる)
	t_InPrevYMDHMS	InPrev_ymdhms;		// 入庫/前回精算_YMDHMS
	uchar			ReceiptIssue;		// 領収証発行有無	0＝領収証なし/1＝領収証あり
	uchar			Syubet;				// 料金種別			1～
	ulong			Price;				// 駐車料金			0～
	ulong			CashPrice;			// 現金売上			0～
	ulong			InPrice;			// 投入金額			0～
	ushort			ChgPrice;			// 釣銭金額			0～9999
	t_MoneyInOut	MoneyInOut;			// 金銭情報(投入金枚数/払出金枚数)
	ushort			HaraiModoshiFusoku;	// 払戻不足額	0～9999
	ushort			CardFusokuType;		// 払出不足媒体種別(現金以外)
	ushort			CardFusokuTotal;	// 払出不足額(現金以外)
	uchar			Reserve1;			// 予備(サイズ調整用)	0
	uchar			AntiPassCheck;		// アンチパスチェック	0＝チェックON/1＝チェックＯＦＦ/2＝強制ＯＦＦ
										//						※定期を使用しない場合は「１＝チェックＯＦＦ」固定
	ulong			ParkNoInPass;		// 定期券　駐車場№	0～999999
	t_MediaInfo2	Media[2];			// 精算媒体情報1, 2
	ushort			CardKind;			// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
										//					3=Edy、4=PiTaPa、5=WAON　　90以降=個別対応
// MH810105(S) MH364301 QRコード決済対応
	uchar			PayTerminalClass;	// 決済端末区分
	uchar			Transactiontatus;	// 取引ステータス
// MH810105(E) MH364301 QRコード決済対応
	uchar			settlement[80];		// 決済情報			(※3)　暗号化対象
// 不具合修正(S) K.Onodera 2016/11/24 #1580 精算情報データの内容がNT-NETフォーマット仕様書と一致していない
//	ulong			TotalSale;			// 合計金額(物販)
//	ulong			DeleteSeq;			// 削除データ追番(1～FFFFFFFFH　削除する精算データにセットしたセンター追番)
//	ulong			Reserve2;			// 予備
//	ulong			Reserve3;			// 予備
// 不具合修正(E) K.Onodera 2016/11/24 #1580 精算情報データの内容がNT-NETフォーマット仕様書と一致していない
	t_SeisanDiscount	SDiscount[NTNET_DIC_MAX];	// 割引
// MH810105(S) MH364301 QRコード決済対応
//} DATA_KIND_152_r12;	// 精算情報ﾃﾞｰﾀ
} DATA_KIND_152_rXX;	// 精算情報ﾃﾞｰﾀ
// MH810105(E) MH364301 QRコード決済対応
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
#define	NTNET_SYNCHRO_GET	0	// 同期時刻参照
#define	NTNET_SYNCHRO_SET	1	// 同期時刻設定
/*--------------------------------------------------------------------------*/
/*	同期時刻設定要求(ﾃﾞｰﾀ種別156)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {						// 
	DATA_BASIC		DataBasic;			// 基本ﾃﾞｰﾀ
	ushort			FmtRev;				// フォーマットRev.№
	ulong			SMachineNo;			// 送信先端末機械№
	uchar			Time_synchro_Req;	// 同期時刻設定要求(0＝要求なし／1＝要求あり)
	uchar			synchro_hour;		// 同期時刻(時)
	uchar			synchro_minu;		// 同期時刻(分)
	uchar			Reserve;			// 予備
	uchar			Reserve2[12];		// 予備2
} DATA_KIND_156;
/*--------------------------------------------------------------------------*/
/*	同期時刻設定要求 結果(ﾃﾞｰﾀ種別157)										*/
/*--------------------------------------------------------------------------*/
typedef	struct {						// 
	DATA_BASIC		DataBasic;			// 基本ﾃﾞｰﾀ
	ushort			FmtRev;				// フォーマットRev.№
	ulong			SMachineNo;			// 送信先端末機械№
	uchar			Time_synchro_Req;	// 同期時刻設定要求(0＝要求なし／1＝要求あり)
	uchar			synchro_hour;		// 同期時刻(時)
	uchar			synchro_minu;		// 同期時刻(分)
	uchar			Reserve;			// 予備
	uchar			Reserve2[12];		// 予備2
	ushort			Result;				// 結果(9=パラメータNG)
} DATA_KIND_157;


/*--------------------------------------------------------------------------*/
/*	ｸﾚｼﾞｯﾄ:送受信電文共通部(ﾃﾞｰﾀ種別136,137,138,139共通)					*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ushort			FmtRev;											// フォーマットRev.№
	ushort			DestInfo;										// 送信先情報
	uchar			DataIdCode1;									// 電文識別コード①
	uchar			DataIdCode2;									// 電文識別コード②
} CREDIT_COMMON;

/*==========================================================================*/
/*	ｸﾚｼﾞｯﾄ:送信電文															*/
/*==========================================================================*/
/*--------------------------------------------------------------------------*/
/*	開局ｺﾏﾝﾄﾞ:ｵﾝﾗｲﾝﾃｽﾄ														*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	CREDIT_COMMON	Common;											// ｸﾚｼﾞｯﾄ共通部
	ushort			SeqNo;											// 処理追い番
	date_time_rec2	Proc;											// 処理年月日時分秒
} DATA_KIND_136_01;

/*--------------------------------------------------------------------------*/
/*	ｸﾚｼﾞｯﾄ与信問合せﾃﾞｰﾀ													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	CREDIT_COMMON	Common;											// ｸﾚｼﾞｯﾄ共通部
	ushort			SeqNo;											// 処理追い番
	date_time_rec2	Pay;											// 精算年月日時分秒
	ulong			SlipNo;											// 端末処理通番（伝票№）
	ulong			Amount;											// 売上金額
	uchar			AppNo[6];										// 承認番号
	struct {
		uchar		JIS2Data[69];									// JIS2ｶｰﾄﾞﾃﾞｰﾀ
		uchar		JIS1Data[37];									// JIS1ｶｰﾄﾞﾃﾞｰﾀ
		ushort		PayMethod;										// 支払方法
		ulong		DivCount;										// 分割回数
	} Crypt;
} DATA_KIND_136_03;

/*--------------------------------------------------------------------------*/
/*	ｸﾚｼﾞｯﾄ売上依頼ﾃﾞｰﾀ														*/
/*--------------------------------------------------------------------------*/
typedef struct {
	uchar			MediaCardNo[30];								// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
	uchar			MediaCardInfo[16];								// カード情報	"0"～"9"、"A"～"Z"(英数字) 左詰め
	uchar			reserve[2];										// 予備
} t_MediaInfoCre;	// 入庫／精算　媒体情報
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	CREDIT_COMMON	Common;											// ｸﾚｼﾞｯﾄ共通部
	ushort			SeqNo;											// 処理追い番
	date_time_rec2	Pay;											// 精算年月日時分秒
	ulong			SlipNo;											// 端末処理通番（伝票№）
	ulong			Amount;											// 売上金額
	uchar			AppNo[6];										// 承認番号
	uchar			ShopAccountNo[20];								// 加盟店取引番号
	ushort			PayMethod;										// 支払方法
	ulong			DivCount;										// 分割回数
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
	t_MediaInfoCre	Media;											// 精算媒体情報1
} DATA_KIND_136_05;

/*--------------------------------------------------------------------------*/
/*	ﾃｽﾄｺﾏﾝﾄﾞ																*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	CREDIT_COMMON	Common;											// ｸﾚｼﾞｯﾄ共通部
	ushort			SeqNo;											// 処理追い番
	uchar			TestKind;										// ﾃｽﾄ種別
} DATA_KIND_136_07;

/*--------------------------------------------------------------------------*/
/*	ｸﾚｼﾞｯﾄ返品問合せﾃﾞｰﾀ													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	CREDIT_COMMON	Common;											// ｸﾚｼﾞｯﾄ共通部
	ushort			SeqNo;											// 処理追い番
	date_time_rec2	Proc;											// 処理年月日時分秒
	ulong			SlipNo;											// 端末処理通番（伝票№）
	uchar			AppNo[6];										// 返品対象 承認番号
	uchar			ShopAccountNo[20];								// 返品対象 加盟店取引番号
} DATA_KIND_136_09;

/*==========================================================================*/
/*	ｸﾚｼﾞｯﾄ:受信電文															*/
/*==========================================================================*/
/*--------------------------------------------------------------------------*/
/*	開局ｺﾏﾝﾄﾞ応答:ｵﾝﾗｲﾝﾃｽﾄ													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	CREDIT_COMMON	Common;											// ｸﾚｼﾞｯﾄ共通部
	ushort			SeqNo;											// 処理追い番
	date_time_rec2	Proc;											// 処理年月日時分秒
	ushort			Result1;										// 処理結果①
	ushort			Result2;										// 処理結果②
	ulong			SrcParkingNo;									// 要求元駐車場№
	ushort			SrcModelCode;									// 要求元機種ｺｰﾄﾞ
	ulong			SrcMachineNo;									// 要求元端末機械№
} DATA_KIND_137_02;

/*--------------------------------------------------------------------------*/
/*	ｸﾚｼﾞｯﾄ与信問合せ結果ﾃﾞｰﾀ												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	CREDIT_COMMON	Common;											// ｸﾚｼﾞｯﾄ共通部
	ushort			SeqNo;											// 処理追い番
	date_time_rec2	Pay;											// 精算年月日時分秒
	ulong			SlipNo;											// 端末処理通番（伝票№）
	ulong			Amount;											// 売上金額
	ushort			PayMethod;										// 支払方法
	ulong			DivCount;										// 分割回数
	ushort			Result1;										// 処理結果①
	ushort			Result2;										// 処理結果②
	uchar			AppNo[6];										// 承認番号
	uchar			ShopAccountNo[20];								// 加盟店取引番号
	uchar			KidCode[6];										// KIDｺｰﾄﾞ
	uchar			CardNo[16];										// ｸﾚｼﾞｯﾄｶｰﾄﾞ№
	uchar			TerminalId[13];									// 端末識別番号
	uchar			CompanyName[12];								// ｸﾚｼﾞｯﾄｶｰﾄﾞ会社名
} DATA_KIND_137_04;

/*--------------------------------------------------------------------------*/
/*	ｸﾚｼﾞｯﾄ売上依頼結果ﾃﾞｰﾀ													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	CREDIT_COMMON	Common;											// ｸﾚｼﾞｯﾄ共通部
	ushort			SeqNo;											// 処理追い番
	date_time_rec2	Pay;											// 精算年月日時分秒
	ulong			SlipNo;											// 端末処理通番（伝票№）
	ulong			Amount;											// 売上金額
	uchar			AppNo[6];										// 承認番号
	uchar			ShopAccountNo[20];								// 加盟店取引番号
	ushort			Result1;										// 処理結果①
	ushort			Result2;										// 処理結果②
} DATA_KIND_137_06;

/*--------------------------------------------------------------------------*/
/*	ﾃｽﾄｺﾏﾝﾄﾞ応答															*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	CREDIT_COMMON	Common;											// ｸﾚｼﾞｯﾄ共通部
	ushort			SeqNo;											// 処理追い番
	uchar			TestKind;										// ﾃｽﾄ種別
	ushort			Result1;										// 処理結果①
	ushort			Result2;										// 処理結果②
} DATA_KIND_137_08;

/*--------------------------------------------------------------------------*/
/*	ｸﾚｼﾞｯﾄ返品問合せ結果ﾃﾞｰﾀ												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	CREDIT_COMMON	Common;											// ｸﾚｼﾞｯﾄ共通部
	ushort			SeqNo;											// 処理追い番
	date_time_rec2	Proc;											// 処理年月日時分秒
	ulong			SlipNo;											// 端末処理通番（伝票№）
	uchar			AppNo[6];										// 返品対象 承認番号
	uchar			ShopAccountNo[20];								// 返品対象 加盟店取引番号
	ushort			Result1;										// 処理結果①
	ushort			Result2;										// 処理結果②
} DATA_KIND_137_0A;

// GG120600(S) // Phase9 センター用端末情報データに受信電文リビジョンを追加

// システムID：40
#define REMOTE_MNT_REQ								111				// 遠隔メンテナンス要求

// GG120600(E) // Phase9 センター用端末情報データに受信電文リビジョンを追加

typedef struct {
	ulong			InstNo1;										// 命令番号1
	ulong			InstNo2;										// 命令番号2
	uchar			ReqKind;										// 要求種別
	uchar			ProcKind;										// 処理種別
// GG120600(S) // Phase9 遠隔監視データ変更
	uchar			RmonFrom;										// 命令要求元（0＝センター/1=端末）
// GG120600(E) // Phase9 遠隔監視データ変更
	uchar			DL_Year;										// ダウンロード時刻（年）
	uchar			DL_Mon;											// ダウンロード時刻（月）
	uchar			DL_Day;											// ダウンロード時刻（日）
	uchar			DL_Hour;										// ダウンロード時刻（時）
	uchar			DL_Min;											// ダウンロード時刻（分）
	uchar			SW_Year;										// プログラム更新時刻（年）
	uchar			SW_Mon;											// プログラム更新時刻（月）
	uchar			SW_Day;											// プログラム更新時刻（日）
	uchar			SW_Hour;										// プログラム更新時刻（時）
	uchar			SW_Min;											// プログラム更新時刻（分）
	uchar			Prog_No[8];										// プログラム部番
	uchar			Prog_Ver[2];									// プログラムバージョン
} t_ProgDlReq;

typedef struct {
	uchar			DestIP[12];										// FTPサーバIPアドレス
	uchar			FTPCtrlPort[10];								// FTP制御ポート
	uchar			FTPUser[20];									// FTPユーザ
	uchar			FTPPasswd[20];									// FTPパスワード
	uchar			Reserve[2];
} t_FtpInfo;

typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	ulong			SMachineNo;										// 送信先ターミナル№
	t_ProgDlReq		ProgDlReq;
	t_FtpInfo		FtpInfo;
	uchar			Reserve[7];
} DATA_KIND_111_DL;

typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	ulong			SMachineNo;										// 送信元ターミナル№
	t_ProgDlReq		ProgDlReq;
	uchar			Reserve[8];
} DATA_KIND_118_DL;

typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	ulong			SMachineNo;										// 送信元ターミナル№
	ulong			InstNo1;										// 命令番号1
	ulong			InstNo2;										// 命令番号2
	ushort			FuncNo;											// 端末機能番号
	ushort			ProcNo;											// 処理番号
	ushort			ProcInfoNo;										// 処理情報番号
// GG120600(S) // Phase9 遠隔監視データ変更
//	uchar			Reserve;
	uchar			RmonFrom;										// 命令要求元0=センター/1=端末
	uchar			RmonUploadReq;									// 設定アップロード要求
// GG120600(E) // Phase9 遠隔監視データ変更
	uchar			StatusNo;										// 状況番号
	ushort			StatusDetailNo;									// 状況詳細番号
	ushort			MonitorInfo;									// 端末監視情報
	uchar			Start_Year1;									// 予定開始時刻1（年）
	uchar			Start_Mon1;										// 予定開始時刻1（月）
	uchar			Start_Day1;										// 予定開始時刻1（日）
	uchar			Start_Hour1;									// 予定開始時刻1（時）
	uchar			Start_Min1;										// 予定開始時刻1（分）
	uchar			Start_Year2;									// 予定開始時刻2（年）
	uchar			Start_Mon2;										// 予定開始時刻2（月）
	uchar			Start_Day2;										// 予定開始時刻2（日）
	uchar			Start_Hour2;									// 予定開始時刻2（時）
	uchar			Start_Min2;										// 予定開始時刻2（分）
	uchar			Prog_No[8];										// プログラム部番
	uchar			Prog_Ver[2];									// プログラムバージョン
} DATA_KIND_125_DL;

// MH322914 (s) kasiyama 2016/07/15 AI-V対応

// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
// ============================================================== //
//
//	端末間通信用フォーマット
//
// ============================================================== //

/*--------------------------------------------------------------------------*/
/*	精算ﾃﾞｰﾀ(ﾃﾞｰﾀ種別56,57)													*/
/*--------------------------------------------------------------------------*/
// GM849100(S) M.Fujikawa 2025/01/09 名鉄協商コールセンター対応（NT-NET端末間通信）
//#define		NTNET_DIC_MAX2	20										// 割引数（データ種別56/57）
#define		NTNET_MNY_MAX	2										// 投入／払出金額
#define		NTNET_DIC_MAX2	98										// 割引数（データ種別56/57）
// GM849100(E) M.Fujikawa 2025/01/09 名鉄協商コールセンター対応（NT-NET端末間通信）

typedef struct {
	uchar			no;												// オプション代金番号
	uchar			reserve;										// 予備（調整用）
	ulong			ryo;											// オプション代金金額
} t_opt_price;

typedef struct {
	t_MoneyInOut	MoneyInOut;										// 金銭情報（投入金額／払戻金額）
// 不具合修正(S) K.Onodera 2016/12/20 #1684 端末間通信での精算データ(ID:57)で売上先駐車場№が正しい位置にセットされていない
//	ulong			TotalSale;										// 合計金額(物販)
//	ulong			DeleteSeq;										// 削除データ追番
//	ulong			SalesParkingNo;									// 売上先駐車場No.
	ulong			SalesParkingNo;									// 売上先駐車場No.
	ulong			TotalSale;										// 合計金額(物販)
	ulong			DeleteSeq;										// 削除データ追番
// 不具合修正(E) K.Onodera 2016/12/20 #1684 端末間通信での精算データ(ID:57)で売上先駐車場№が正しい位置にセットされていない
	ushort			FusokuCardKind;									// 払出し不足媒体種別(現金以外)
	ushort			FusokuCard;										// 払出し不足額(現金以外)
// GM849100(S) M.Fujikawa 2025/01/09 名鉄協商コールセンター対応（NT-NET端末間通信）
//	uchar			Reserve1[12];									// 予備1
	ulong			ParkingTime;									// 駐車時間
	ushort			MaxFeeFlag;										// 最大適用フラグ
	ushort			MaxFeeCount;									// 最大適用回数
	uchar			MaxFeeSetting;									// 最大料金設定有無
	uchar			Reserve1[3];									// 予備1
// GM849100(E) M.Fujikawa 2025/01/09 名鉄協商コールセンター対応（NT-NET端末間通信）
	ulong			PayCount;										// 精算追い番
	ushort			PayMethod;										// 精算方法
	ushort			PayClass;										// 処理区分
	ushort			PayMode;										// 精算ﾓｰﾄﾞ
	ulong			LockNo;											// 区画情報
	ushort			CardType;										// 駐車券ﾀｲﾌﾟ
	ushort			CMachineNo;										// 駐車券機械№
	ulong			CardNo;											// 駐車券番号(発券追い番)
	date_time_rec2	OutTime;										// 出庫年月日時分秒
	ushort			KakariNo;										// 係員№
	ushort			OutKind;										// 精算出庫
	ushort			CountSet;										// 在車ｶｳﾝﾄ
	ushort			Before_pst;										// 前回％割引率
	ulong			BeforePwari;									// 前回％割引金額
	ulong			BeforeTime;										// 前回割引時間数
	ulong			BeforeTwari;									// 前回時間割引金額
	ulong			BeforeRwari;									// 前回料金割引金額
// GM849100(S) M.Fujikawa 2025/01/09 名鉄協商コールセンター対応（NT-NET端末間通信）
//	ushort			ReceiptIssue;									// 領収証発行有無
	uchar			SealIssue;										//シール発行有無
	uchar			ReceiptIssue;									// 領収証発行有無
// GM849100(E) M.Fujikawa 2025/01/09 名鉄協商コールセンター対応（NT-NET端末間通信）
	date_time_rec2	InTime;											// 入庫年月日時分秒
	date_time_rec2	PayTime;										// 事前精算年月日時分秒(CPS精算時ｾｯﾄ)
	ulong			TaxPrice;										// 課税対象額
	ulong			TotalPrice;										// 合計金額
	ulong			Tax;											// 消費税額
	ushort			Syubet;											// 料金種別
	ulong			Price;											// 駐車料金
	ulong			TotalDiscount;									// 総割引額
	long			CashPrice;										// 現金売上
	ulong			InPrice;										// 投入金額
	ulong			ChgPrice;										// 釣銭金額
	ulong			Fusoku;											// 釣銭払出不足金額
	ushort			FusokuFlg;										// 釣銭払出不足発生ﾌﾗｸﾞ
	ushort			PayObsFlg;										// 精算中障害発生ﾌﾗｸﾞ
	ushort			ChgOverFlg;										// 払戻上限額ｵｰﾊﾞｰ発生ﾌﾗｸﾞ
	ushort			PassCheck;										// ｱﾝﾁﾊﾟｽﾁｪｯｸ
	PASS_DATA		PassData;										// 定期券ﾃﾞｰﾀ
	ulong			PassRenewalPric;								// 定期券更新料金
	ushort			PassRenewalCondition;							// 定期券更新条件
	ushort			PassRenewalPeriod;								// 定期券更新期間
	uchar			UpCount;										// 更新回数
	uchar			ReIssueCount;									// 再発行回数
	t_opt_price		Opt_Price[4];									// オプション代金
	date_time_rec2	BeforePayTime;									// 前回事前精算年月日時分秒（精算後券で精算時）
	uchar			MatchIP[8];										// ﾏｯﾁﾝｸﾞIPｱﾄﾞﾚｽ
	uchar			MatchVTD[8];									// ﾏｯﾁﾝｸﾞVTD/車番
	uchar			CreditIssue;									// ｸﾚｼﾞｯﾄｶｰﾄﾞ決済有無
	uchar			CredirReserve1;									// ｸﾚｼﾞｯﾄｶｰﾄﾞ予備1（調整用）
	uchar			CreditCardNo[20];								// ｸﾚｼﾞｯﾄｶｰﾄﾞ会員№
	ulong			Credit_ryo;										// ｸﾚｼﾞｯﾄｶｰﾄﾞ利用金額
	ulong			CreditSlipNo;									// ｸﾚｼﾞｯﾄｶｰﾄﾞ伝票番号
	ulong			CreditAppNo;									// ｸﾚｼﾞｯﾄｶｰﾄﾞ承認番号
	uchar			CreditName[10];									// ｸﾚｼﾞｯﾄｶｰﾄﾞ会社名
	uchar			CreditDate[2];									// ｸﾚｼﾞｯﾄｶｰﾄﾞ有効期限(年月)
	ulong			CreditProcessNo;								// ｸﾚｼﾞｯﾄｶｰﾄﾞｾﾝﾀ処理追い番
	uchar			term_id[16];									// 端末識別番号
	uchar			kid_code[6];									// KIDｺｰﾄﾞ
	ulong			Reserve2;										// 予備2
// MH364304(S) データ保管サービス対応（登録番号をセットする）
//	ulong			Reserve3;										// 予備3
//	ulong			Reserve4;										// 予備4
	ulong			RegistNum1;										// 登録番号1（下位9桁 BIN）
	ushort			RegistNum2;										// 登録番号2（上位4桁 BIN）
	uchar			RegistNum3;										// 登録番号3（先頭1文字）
// MH364304(S) データ保管サービス対応（適用税率をセットする）
	uchar			TaxRate;										// 適用税率
// MH364304(E) データ保管サービス対応（適用税率をセットする）
// MH364304(E) データ保管サービス対応（登録番号をセットする）
// MH364304(S) データ保管サービス対応（課税対象をセットする）
//	ulong			Reserve5;										// 予備5
	ulong			TaxableDiscount;								// 課税対象（b0～b19）
// MH364304(E) データ保管サービス対応（課税対象をセットする）
// MH364304(S) データ保管サービス対応（適格請求書情報有無をセットする）
//	ulong			Reserve6;										// 予備6
	ushort			Invoice;										// 適格請求書情報有無
	ushort			Reserve6;
// MH364304(E) データ保管サービス対応（適格請求書情報有無をセットする）
	ulong			Reserve7;										// 予備7
	ulong			Reserve8;										// 予備8
	t_MediaInfo2	Media[1];										// 出庫媒体種別
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（精算データ変更）（GT-7700:GM747904参考）
	date_time_rec2	Before_Ts_Time;									// 前回Ｔ合計の時間		計8byte
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（精算データ変更）（GT-7700:GM747904参考）
} t_SeisanData_T;

typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			CenterSeqNo;									// ｾﾝﾀｰ追番
	t_SeisanData_T	SeisanData;										// 精算データ本体
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（精算データ変更）（GT-7700:GM747904参考）
//	DISCOUNT_DATA	DiscountData[NTNET_DIC_MAX2];					// 割引関連
	IN_OUT_MONEY	MoneyData[NTNET_MNY_MAX];						// 投入・払出金額
	t_SeisanDiscountOld	DiscountData[NTNET_DIC_MAX2];				// 割引関連
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（精算データ変更）（GT-7700:GM747904参考）
} DATA_KIND_56_T;

/*--------------------------------------------------------------------------*/
/*	集計基本ﾃﾞｰﾀ(ﾃﾞｰﾀ種別42)												*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			CenterSeqNo;									// ｾﾝﾀｰ追番
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
	date_time_rec2	NowTime;										// 今回集計
	date_time_rec2	LastTime;										// 前回集計
	ulong			SettleNum;										// 総精算回数
	ulong			Kakeuri;										// 総掛売額
	ulong			Cash;											// 総現金売上額
	ulong			Uriage;											// 総売上額
	ulong			Tax;											// 総消費税額
	ulong			Charge;											// 釣銭払戻額
	ulong			CoinTotalNum;									// コイン金庫合計回数
	ulong			NoteTotalNum;									// 紙幣金庫合計回数
	ulong			CyclicCoinTotalNum;								// ホッパー合計回数
	ulong			NoteOutTotalNum;								// 紙幣払出機合計回数
	ulong			Uri_Tryo_Gai;									// 総売上外額
	ulong			SalesParkingNo;									// 売上先駐車場No.
	ulong			Rsv1;											// 予備
	ulong			CarOutWithoutPay;								// 精算なしモード時出庫台数
	ulong			SettleNumServiceTime;							// サービスタイム内精算回数
	ulong			CarOutLagTime;									// ラグタイム内出庫回数
	t_SyuSub		Shortage;										// 払出不足
	t_SyuSub		Cancel;											// 精算中止
	t_SyuSub		AutoSettle;										// 半自動精算
	t_SyuSub		ManualSettle;									// マニュアル精算
	ulong			AntiPassOffSettle;								// アンチパスOFF精算回数
	ulong			CarOutGateOpen;									// ゲート開放時出庫台数
	ulong			CarOutForce;									// 強制出庫台数
	t_SyuSub		LostSettle;										// 紛失精算
	ulong			ReceiptCallback;								// 領収証回収枚数
	ulong			ReceiptIssue;									// 領収証発行枚数
	ulong			WarrantIssue;									// 預り証発行枚数
	t_SyuSub_AllSystem	AllSystem;									// 全装置
	ulong			CarInTotal;										// 総入庫台数
	ulong			CarOutTotal;									// 総出庫台数
	ulong			CarIn1;											// 入庫1入庫台数
	ulong			CarOut1;										// 出庫1出庫台数
	ulong			CarIn2;											// 入庫2入庫台数
	ulong			CarOut2;										// 出庫2出庫台数
	ulong			CarIn3;											// 入庫3入庫台数
	ulong			CarOut3;										// 出庫3出庫台数
	ulong			MiyoCount;										// 未入金回数
	ulong			MiroMoney;										// 未入金額
	ulong			LagExtensionCnt;								// ラグタイム延長回数
	ulong			FurikaeCnt;										// 振替回数
	ulong			FurikaeTotal;									// 振替額
	ulong			RemoteCnt;										// 遠隔精算回数
	ulong			RemoteTotal;									// 遠隔精算金額
	ulong			Rsv2[6];										// 予備8～13
} DATA_KIND_42_T;

/*--------------------------------------------------------------------------*/
/*	料金種別毎集計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別43)											*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			CenterSeqNo;									// ｾﾝﾀｰ追番
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
	t_SyuSub_RyokinKind	Kind[50];									// 種別01～50
} DATA_KIND_43_T;

/*--------------------------------------------------------------------------*/
/*	分類集計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別44)												*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			CenterSeqNo;									// ｾﾝﾀｰ追番
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
	ulong			Kind;											// 駐車分類集計の種類
	t_SyuSub		Group[BUNRUI_CNT];								// 分類1～48
	t_SyuSub		GroupTotal;										// 分類以上
	ulong			LostSettle;										// 紛失精算 台数
	ulong			AntiPassOffSettle;								// アンチパスOFF精算 台数
	ulong			CarOutForce;									// 強制出庫 台数
	ulong			FirstTimePass;									// 初回定期 台数
	t_SyuSub		Unknown;										// 分類不明
	uchar			Rsv[32];										// 予備
	ulong			Kind2;											// 駐車分類集計の種類_2
	t_SyuSub		Group2[BUNRUI_CNT];								// 分類1～48_2
	t_SyuSub		GroupTotal2;									// 分類以上_2
	ulong			LostSettle2;									// 紛失精算 台数_2
	ulong			AntiPassOffSettle2;								// アンチパスOFF精算 台数_2
	ulong			CarOutForce2;									// 強制出庫 台数_2
	ulong			FirstTimePass2;									// 初回定期 台数_2
	t_SyuSub		Unknown2;										// 分類不明_2
	uchar			Rsv2[32];										// 予備_2
	ulong			Kind3;											// 駐車分類集計の種類_3
	t_SyuSub		Group3[BUNRUI_CNT];								// 分類1～48_3
	t_SyuSub		GroupTotal3;									// 分類以上_3
	ulong			LostSettle3;									// 紛失精算 台数_3
	ulong			AntiPassOffSettle3;								// アンチパスOFF精算 台数_3
	ulong			CarOutForce3;									// 強制出庫 台数_3
	ulong			FirstTimePass3;									// 初回定期 台数_3
	t_SyuSub		Unknown3;										// 分類不明_3
	uchar			Rsv3[32];										// 予備_3
} DATA_KIND_44_T;

/*--------------------------------------------------------------------------*/
/*	割引集計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別45)												*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			CenterSeqNo;									// ｾﾝﾀｰ追番
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
	t_SyuSub_Discount	Discount[500];								// 割引 001～500
} DATA_KIND_45_T;

/*--------------------------------------------------------------------------*/
/*	定期集計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別46)												*/
/*--------------------------------------------------------------------------*/
typedef struct {
	uchar			kind;											// オプション代金種別
	ulong			count;											// 回数
	ulong			sales;											// 売上金額
} t_SyuSub_Opt_Price;
typedef struct {
	DATA_BASIC			DataBasic;									// 基本ﾃﾞｰﾀ
	ulong				CenterSeqNo;								// ｾﾝﾀｰ追番
	ushort				Type;										// 集計タイプ
	ushort				KakariNo;									// 係員No.
	ulong				SeqNo;										// 集計追番
	ulong				StartSeqNo;									// 開始追番
	ulong				EndSeqNo;									// 終了追番
	t_SyuSub_Opt_Price	Opt_Price[6];								// オプション代金
	t_SyuSub_Pass		Pass[100];									// 定期券 001～100
} DATA_KIND_46_T;

/*--------------------------------------------------------------------------*/
/*	金銭集計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別48)												*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			CenterSeqNo;									// ｾﾝﾀｰ追番
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
	ulong			Total;											// 金庫総入金額
	ulong			NoteTotal;										// 紙幣金庫総入金額
	ulong			CoinTotal;										// コイン金庫総入金額
	t_SyuSub_Coin	Coin[4];										// コイン1～4
	t_SyuSub_Coin	CoinRsv[4];										// コイン予備1～4
	t_SyuSub_Note	Note[4];										// 紙幣1～4
	t_SyuSub_Note	NoteRsv[4];										// 紙幣予備1～4
	ulong			CycleAccept;									// 循環部総入金額
	ulong			CyclePay;										// 循環部総出金額
	ulong			NoteAcceptTotal;								// 紙幣総入金額
	ulong			NotePayTotal;									// 紙幣総払出金額
	ulong			StockPayTotal;									// 予蓄部総出金額
	t_SyuSub_Cycle	Cycle[4];										// 循環1～4
	t_SyuSub_Cycle	CycleRsv[4];									// 循環予備1～4
	t_SyuSub_Stock	Stock[2];										// 予蓄部1～2
	t_SyuSub_Note	NotePay[4];										// 紙幣払出1～4
	t_SyuSub_Note	NotePayRsv[4];									// 紙幣払出予備1～4
} DATA_KIND_48_T;

/*--------------------------------------------------------------------------*/
/*	集計終了通知ﾃﾞｰﾀ(ﾃﾞｰﾀ種別53)											*/
/*--------------------------------------------------------------------------*/
typedef struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			CenterSeqNo;									// ｾﾝﾀｰ追番
	ushort			Type;											// 集計タイプ
	ushort			KakariNo;										// 係員No.
	ulong			SeqNo;											// 集計追番
	ulong			StartSeqNo;										// 開始追番
	ulong			EndSeqNo;										// 終了追番
	ulong			MachineNo[32];									// 複数集計した端末の機械No.-001～-032
} DATA_KIND_53_T;

/*	ｴﾗｰﾃﾞｰﾀ(ﾃﾞｰﾀ種別63)														*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	uchar			Errsyu;											// ｴﾗｰ種別
	uchar			Errcod;											// ｴﾗｰｺｰﾄﾞ
	uchar			Errdtc;											// ｴﾗｰｽﾃｰﾀｽ(発生/解除)
	uchar			Errlev;											// ｴﾗｰﾚﾍﾞﾙ
	uchar			ErrDoor;										// ﾄﾞｱ状態(0:close,1:open)
	uchar			Errdat1[10];									// ｴﾗｰ情報(ﾊﾞｲﾅﾘ)
	uchar			Errdat2[160];									// ｴﾗｰ情報(acsiiｺｰﾄﾞ)
} DATA_KIND_63_T;

/*--------------------------------------------------------------------------*/
/*	ｱﾗｰﾑﾃﾞｰﾀ(ﾃﾞｰﾀ種別64)													*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	uchar			Armsyu;											// ｱﾗｰﾑ種別
	uchar			Armcod;											// ｱﾗｰﾑｺｰﾄﾞ
	uchar			Armdtc;											// ｱﾗｰﾑｽﾃｰﾀｽ(発生/解除)
	uchar			Armlev;											// ｱﾗｰﾑﾚﾍﾞﾙ
	uchar			ArmDoor;										// ﾄﾞｱ状態(0:close,1:open)
	uchar			Armdat1[10];									// ｱﾗｰﾑ情報(ﾊﾞｲﾅﾘ)
	uchar			Armdat2[160];									// ｱﾗｰﾑ情報(acsiiｺｰﾄﾞ)
} DATA_KIND_64_T;

/*--------------------------------------------------------------------------*/
/*	釣銭管理集計ﾃﾞｰﾀ(ﾃﾞｰﾀ種別135)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			CenterSeqNo;									// センター追番
	ulong			Oiban;											// 金銭管理合計追番
	ushort			PayClass;										// 処理区分
	ushort			KakariNo;										// 係員№
	TURI_DATA		turi_dat[4];									// 金銭ﾃﾞｰﾀ(4金種分)
	TURI_DATA		yturi_dat[5];									// 金銭ﾃﾞｰﾀ(5予蓄分)
	uchar			Reserve[32];									// 予備
} DATA_KIND_135_T;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
// GM849100(S) M.Fujikawa 2025/01/10 名鉄協商コールセンター対応（NT-NET端末間通信）
/*--------------------------------------------------------------------------*/
/*	端末情報ﾃﾞｰﾀ(ﾃﾞｰﾀ種別230)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ushort			Status;								// ユニット状態
	uchar			Err_md;								// エラー種別（モジュールＮｏ）
	uchar			Err_no;								// エラー番号
} UNIT_DATA;

/*--------------------------------------------------------------------------*/
/*	端末情報ﾃﾞｰﾀ(ﾃﾞｰﾀ種別230)												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ulong			SMachineNo;										// 送信先端末機械№
	uchar			ProgramVer[12];									// ﾌﾟﾛｸﾞﾗﾑﾊﾞｰｼﾞｮﾝ
	UNIT_DATA		UnitInfo[10];
																	// [0]: リーダー１
																	// [1]: リーダー２
																	// [2]: 発券ユニット（なし）
																	// [3]: ジャーナルプリンタ
																	// [4]: レシートプリンタ
																	// [5]: コインメック
																	// [6]: 紙幣リーダー
																	// [7]: ゲート装置
																	// [8]: オプションユニット
	uchar			Dummy[80];
} DATA_KIND_230_T;
// GM849100(E) M.Fujikawa 2025/01/10 名鉄協商コールセンター対応（NT-NET端末間通信）

// システムID：16
#define PIP_REQ_KIND_RECEIPT_AGAIN			1			// 領収書再発行要求
#define PIP_RES_KIND_RECEIPT_AGAIN			2			// 領収証再発行応答

#define PIP_REQ_KIND_FURIKAE				3			// 振替精算要求
#define PIP_RES_KIND_FURIKAE				4			// 振替精算応答

#define PIP_REQ_KIND_RECEIVE_TKT			5			// 受付券発行要求
#define PIP_RES_KIND_RECEIVE_TKT			6			// 受付券発行応答

#define PIP_REQ_KIND_REMOTE_CALC			7			// 遠隔精算要求
#define PIP_RES_KIND_REMOTE_CALC_ENTRY		8			// 遠隔精算入庫時刻指定精算応答
#define PIP_RES_KIND_REMOTE_CALC_FEE		9			// 遠隔精算金額指定精算応答

#define PIP_REQ_KIND_OCCUR_ERRALM			10			// 発生中エラーアラーム要求
#define PIP_RES_KIND_OCCUR_ERRALM			11			// 発生中エラーアラーム応答

// 遠隔精算要求種別
#define	PIP_REMOTE_TYPE_CALC				1
#define	PIP_REMOTE_TYPE_CALC_TIME			2
#define	PIP_REMOTE_TYPE_CALC_FEE			3

// 振替精算要求種別
#define	PIP_FURIKAE_TYPE_INFO				1
#define	PIP_FURIKAE_TYPE_CHK				2
#define	PIP_FURIKAE_TYPE_GO					3

// 振替精算ステータス
enum {
	PIP_FRK_STS_IDLE = 0,							// アイドル状態
	PIP_FRK_STS_ACP,								// 振替精算 受付
	PIP_FRK_STS_MAX,
};

// 結果
#define PIP_RES_RESULT_OK					0			// 成功
#define PIP_RES_RESULT_NG_DENY				11			// 受付拒否（受付不可状態）
#define PIP_RES_RESULT_NG_PARAM				12			// パラメータ不良
#define PIP_RES_RESULT_NG_NO_CAR			13			// 指定車室無し
#define PIP_RES_RESULT_NG_BUSY				14			// ビジー（クレジット問い合わせ中など、一時的に受け付けられない場合
#define PIP_RES_RESULT_NG_OTHER				19			// その他エラー
#define PIP_RES_RESULT_NG_CHANGE_NEXT		21			// 要求受付完了し、その後の処理への移行が失敗
#define PIP_RES_RESULT_NG_CHANGE_PAY		22			// 料金計算完了後、精算中状態に移行失敗
#define PIP_RES_RESULT_OK_STOP_USER			23			// 要求実行完了（利用者による中止終了）
#define PIP_RES_RESULT_OK_STOP				24			// 要求実行完了（精算機が実行不可条件検知による中止終了）
#define PIP_RES_RESULT_NG_FURIKAE_SRC		25			// 振替元条件エラー
#define PIP_RES_RESULT_NG_FURIKAE_DST		26			// 振替先条件エラー

// UNION PIP_CTRLの種別
enum{
	UNI_KIND_FURIKAE = 1,
	UNI_KIND_REMOTE_TIME,
	UNI_KIND_REMOTE_FEE,
};
// ============================================================ //
//			構造体定義
// ============================================================ //
// PiP共通部分
typedef	struct {
	ushort			FmtRev;					// フォーマットRev.№
	ulong			SMachineNo;				// 送信先ターミナル№
	ulong			CenterSeqNo;			// センター追番
} PIP_COMMON;

// 割引情報
typedef struct{
	ulong			ParkNo;					// 割引駐車場№
	ushort			DiscSyubet;				// 割引種別
	ushort			DiscType;				// 割引区分
	ushort			DiscCount;				// 割引使用枚数
	ulong			DiscPrice;				// 割引金額
	ulong			DiscInfo1;				// 割引情報１
	ulong			DiscInfo2;				// 割引情報２
} PIP_Discount;

// 媒体情報
typedef struct{
	ushort		Kind;						// 種別
	uchar		CardNo[30];					// カード番号
	uchar		CardInfo[16];				// カード情報
	ushort		Reserve;					// 予備(サイズ調整用)
} PIP_Media;

#define	PIP_DISCOUNT_MAX					25

// 車室情報
typedef struct{
	ushort			SrcArea;				// 区画
	ulong			SrcNo;					// 車室
	date_time_rec2	Date1;					// 精算/入庫年月日時分秒
	date_time_rec2	Date2;					// 入庫/料金計算年月日時分秒
	ushort			SrcFeeKind;				// 料金種別
	ulong			SrcFee;					// 駐車料金
	ulong			SrcFeeDiscount;			// 割引金額
	ushort			SrcStatus;				// ステータス
} PIP_FlpInfo;

// オプション情報
typedef struct{
	ushort		kind;						// 種別
	ulong		Amount;						// 金額
} PIP_Option;

#define	PIP_OPTION_MAX						4
// ============================================================ //
//			要求コマンド構造体定義
// ============================================================ //

// 振替精算要求
typedef struct {
	DATA_BASIC		DataBasic;				// 基本ﾃﾞｰﾀ
	PIP_COMMON		Common;					// Park I Pro共通データ
	ushort			ReqKind;				// 要求種別
	ushort			SrcArea;				// 振替元区画
	ulong			SrcNo;					// 振替元車室番号
	ushort			DestArea;				// 振替先区画
	ulong			DestNo;					// 振替先車室番号
	ushort			SrcPassUse;				// 振替元定期利用有無
} DATA_KIND_16_03;

// 遠隔精算要求
typedef struct {
	DATA_BASIC		DataBasic;				// 基本ﾃﾞｰﾀ
	PIP_COMMON		Common;					// Park I Pro共通データ
	uchar			ReqKind;				// 要求種別
// 仕様変更(S) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
//	uchar			Syubet;					// 料金種別
	uchar			Type;					// 遠隔精算種別(0=出庫精算/1=後日精算)
	ushort			Syubet;					// 料金種別
// 仕様変更(E) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
	ushort			Area;					// 区画
	ulong			ulNo;					// 車室番号
	date_time_rec2	InTime;					// 入庫年月日時分秒
	ulong			Price;					// 駐車料金			0～
// 仕様変更(S) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
	ulong			FutureFee;				// 後日精算予定額
// 仕様変更(E) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
	PIP_Discount	stDiscount;				// 割引情報
} DATA_KIND_16_07;


// 受付券発行要求
typedef struct {
	DATA_BASIC		DataBasic;				// 基本ﾃﾞｰﾀ
	PIP_COMMON		Common;					// Park I Pro共通データ
	ushort			Area;					// 区画
	ulong			ulNo;					// 車室番号
} DATA_KIND_16_05;

// 領収証再発行要求
typedef struct {
	DATA_BASIC		DataBasic;				// 基本ﾃﾞｰﾀ
	PIP_COMMON		Common;					// Park I Pro共通データ
	ushort			PayInfoKind;			// 精算情報種別
	ulong			PayCount;				// 精算追番
	uchar			PayMethod;				// 精算方法
	uchar			PayClass;				// 処理区分
	uchar			PayMode;				// 精算ﾓｰﾄﾞ
	uchar			CMachineNo;				// 駐車券機械№(入庫機械№※フラップ式はゼロ)
	ushort			Area;					// 区画
	ushort			No;						// 車室番号
	ushort			KakariNo;				// 係員№
	uchar			OutKind;				// 精算出庫
	uchar			CountSet;				// 在車ｶｳﾝﾄ
	uchar			OutTime[6];				// 精算年月日時分秒
	uchar			InTime[6];				// 入庫年月日時分秒
	uchar			PreOutTime[6];			// 前回精算年月日時分秒
	uchar			ReceiptIssue;			// 領収証発行有無
	uchar			Syubet;					// 料金種別
	ulong			TotalFee;				// 駐車料金/合計金額
	ulong			TotalMoney;				// 現金売上データ
	ulong			InMoney;				// 投入金額
	ushort			Change;					// 釣銭額
	struct{	// 金銭情報;
		uchar		InMoneyBit;				// 投入金金種有無
		uchar		InMoneyCnt[8];
	} InMoneyInfo;
	struct{	// 払出情報;
		uchar		OutMoneyBit;			// 払出金金種有無
		uchar		OutMoneyCnt[8];
	} OutMoneyInfo;
	ushort			Fusoku;					// 払戻し不足額
// 仕様変更(S) K.Onodera 2016/11/02 領収証再発行要求フォーマット対応
	ushort			CardFusokuType;			// 払戻し不足媒体(現金以外)
	ushort			CardFusoku;				// 払戻し不足額(現金以外)
// 仕様変更(E) K.Onodera 2016/11/02 領収証再発行要求フォーマット対応
	ushort			Antipas;				// アンチパスチェック(0=チェックON/1=チェックOFF/2=強制OFF)※定期使用なしは1固定
	PIP_Media		MainMedia;				// メイン媒体情報
	PIP_Media		SubMedia;				// サブ媒体情報
	ushort			CardPaymentKind;		// カード決済区分(0=なし/1=クレジット/2=交通系/3=Edy/4=PiTaPa/5=WAON/6=nanaco/7=SPICA)
// 仕様変更(S) K.Onodera 2016/11/02 領収証再発行要求フォーマット対応
//	uchar			CardPaymentInfo[64];	// 決済情報※暗号化対象
//	struct{	// タイムレジ情報
//		ulong		InMoney;				// 入金処理金額
//		ulong		OutMoney;				// 出金処理金額
//		ulong		ReturnMoney;			// 払戻し金額
//		ulong		FusokuMoney;			// 受け取り不足金額
//	} TimeRegInfo;
	uchar			CardPaymentInfo[80];	// 決済情報※暗号化対象
	ulong			TotalSale;				// 合計金額(物販)
	ulong			DeleteSeq;				// 削除データ追番
// 仕様変更(E) K.Onodera 2016/11/02 領収証再発行要求フォーマット対応
	ulong			SalesDestination;		// 売上先駐車場№※特注用で、標準は基本駐車場№をセット
	struct{	// 定期券情報
		ulong		ParkNo;					// 駐車場№
		ulong		Id;						// ID
		ushort		Kind;					// 種別
		uchar		Status;					// ステータス
		uchar		ReadOnly;				// リードオンリー(0=リードライト/1=リードライト)
		uchar		ReturnKind;				// 返却/回収
		uchar		UpdataStartDate[3];		// 更新後の有効開始年月日
		uchar		StartDate[3];			// 有効開始年月日
		uchar		EndDate[3];				// 有効終了年月日
		ulong		UpdateFee;				// 更新料金
		uchar		UpdateFlag;				// 0=新規/1=更新
		uchar		UpdateCnt;				// 更新回数
		uchar		UpdateTerm;				// 更新期間
	} PassInfo;
	uchar			ReissueCnt;				// 再発行回数
// 仕様変更(S) K.Onodera 2016/11/02 領収証再発行要求フォーマット対応
	uchar			card_name[12];			// クレジットカード会社名
// 仕様変更(E) K.Onodera 2016/11/02 領収証再発行要求フォーマット対応
	PIP_Option		stOption[PIP_OPTION_MAX];
	PIP_Discount	stDiscount[PIP_DISCOUNT_MAX];
} DATA_KIND_16_01;

// 発生中エラーアラーム要求
typedef struct {
	DATA_BASIC		DataBasic;				// 基本ﾃﾞｰﾀ
	PIP_COMMON		Common;					// Park I Pro共通データ
	ushort			ReqFlg;					// 要求エラーアラームフラグ(0=エラー/1=アラーム)
} DATA_KIND_16_10;


// ============================================================ //
//			応答コマンド構造体定義
// ============================================================ //

// 受付券発行応答
typedef struct {
	DATA_BASIC		DataBasic;				// 基本ﾃﾞｰﾀ
	PIP_COMMON		Common;					// Park I Pro共通データ
	ushort			Result;					// 結果
	ushort			Area;					// 区画
	ulong			ulNo;					// 車室番号
} DATA_KIND_16_06;

// 領収証再発行応答
typedef struct {
	DATA_BASIC		DataBasic;				// 基本ﾃﾞｰﾀ
	PIP_COMMON		Common;					// Park I Pro共通データ
	ushort			Type;					// 精算情報種別(0=精算/1=定期更新)
	ushort			Result;					// 結果
	ulong			PayCount;				// 精算追い番
	uchar			PayMethod;				// 精算方法
	uchar			PayClass;				// 処理区分
	uchar			PayMode;				// 精算ﾓｰﾄﾞ
	uchar			CMachineNo;				// 駐車券機械№
	ushort			Area;					// 区画
	ushort			No;						// 車室番号
	ushort			KakariNo;				// 係員№
// MH810105(S) MH364301 領収証再発行応答のフォーマット間違いを修正
//	ushort			OutKind;				// 精算出庫
//	ushort			CountSet;				// 在車ｶｳﾝﾄ
	uchar			OutKind;				// 精算出庫
	uchar			CountSet;				// 在車ｶｳﾝﾄ
// MH810105(E) MH364301 領収証再発行応答のフォーマット間違いを修正
	uchar			OutTime[6];				// 精算年月日時分秒
	uchar			InTime[6];				// 入庫年月日時分秒
	uchar			PreOutTime[6];			// 前回精算年月日時分秒
// MH810105(S) MH364301 領収証再発行応答のフォーマット間違いを修正
//	ushort			ReceiptIssue;			// 領収証発行有無
	uchar			ReceiptIssue;			// 領収証発行有無
// MH810105(E) MH364301 領収証再発行応答のフォーマット間違いを修正
	uchar			Syubet;					// 料金種別
	ulong			TotalFee;				// 駐車料金/合計金額
	ulong			TotalMoney;				// 現金売上データ
	ulong			InMoney;				// 投入金額
	ushort			Change;					// 釣銭額
	struct{	// 金銭情報;
		uchar		InMoneyBit;				// 投入金金種有無
		uchar		InMoneyCnt[8];
	} InMoneyInfo;
	struct{	// 払出情報;
		uchar		OutMoneyBit;			// 投入金金種有無
		uchar		OutMoneyCnt[8];
	} OutMoneyInfo;
	ushort			Fusoku;					// 払戻し不足額
// 仕様変更(S) K.Onodera 2016/11/02 領収証再発行要求フォーマット対応
	ushort			CardFusokuType;			// 払戻し不足媒体(現金以外)
	ushort			CardFusoku;				// 払戻し不足額(現金以外)
// 仕様変更(E) K.Onodera 2016/11/02 領収証再発行要求フォーマット対応
	ushort			Antipas;				// アンチパスチェック(0=チェックON/1=チェックOFF/2=強制OFF)※定期使用なしは1固定
	PIP_Media		MainMedia;				// メイン媒体情報
	PIP_Media		SubMedia;				// サブ媒体情報
	ushort			CardPaymentKind;		// カード決済区分(0=なし/1=クレジット/2=交通系/3=Edy/4=PiTaPa/5=WAON/6=nanaco/7=SPICA)
// 仕様変更(S) K.Onodera 2016/11/02 領収証再発行要求フォーマット対応
//	uchar			CardPaymentInfo[64];	// 決済情報※暗号化対象
//	struct{	// タイムレジ情報
//		ulong		InMoney;				// 入金処理金額
//		ulong		OutMoney;				// 出金処理金額
//		ulong		ReturnMoney;			// 払戻し金額
//		ulong		FusokuMoney;			// 受け取り不足金額
//	} TimeRegInfo;
	uchar			CardPaymentInfo[80];	// 決済情報※暗号化対象
	ulong			TotalSale;				// 合計金額(物販)
	ulong			DeleteSeq;				// 削除データ追番
// 仕様変更(E) K.Onodera 2016/11/02 領収証再発行要求フォーマット対応
	ulong			SalesDestination;		// 売上先駐車場№※特注用で、標準は基本駐車場№をセット
	struct{	// 定期券情報
		ulong		ParkNo;					// 駐車場№
		ulong		Id;						// ID
		ushort		Kind;					// 種別
		uchar		Status;					// ステータス
		uchar		ReadOnly;				// リードオンリー(0=リードライト/1=リードライト)
		uchar		ReturnKind;				// 返却/回収
		uchar		UpdataStartDate[3];		// 更新後の有効開始年月日
		uchar		StartDate[3];			// 有効開始年月日
		uchar		EndDate[3];				// 有効終了年月日
		ulong		UpdateFee;				// 更新料金
		uchar		UpdateFlag;				// 0=新規/1=更新
		uchar		UpdateCnt;				// 更新回数
		uchar		UpdateTerm;				// 更新期間
	} PassInfo;
	uchar			ReissueCnt;				// 再発行回数
// 仕様変更(S) K.Onodera 2016/11/02 領収証再発行要求フォーマット対応
	uchar			card_name[12];			// クレジットカード会社名
// 仕様変更(E) K.Onodera 2016/11/02 領収証再発行要求フォーマット対応
	PIP_Option		stOption[PIP_OPTION_MAX];
	PIP_Discount	stDiscount[PIP_DISCOUNT_MAX];
	
} DATA_KIND_16_02;

typedef struct {
	uchar			ucKind;					// 種別
	uchar			ucCoce;					// コード
	uchar			ucLevel;				// レベル
	uchar			ucDoor;					// ドア状態
	uchar			ucInfo[10];				// エラーアラーム情報
}PIP_ErrAlarm;

#define PIP_OCCUR_ERROR_ALARM_MAX			60

// 発生中エラー応答テーブル
typedef struct {
	DATA_BASIC		DataBasic;				// 基本ﾃﾞｰﾀ
	PIP_COMMON		Common;					// Park I Pro共通データ
	ushort			Result;					// 結果
	ushort			Kind;					// 応答エラーアラームフラグ（0=エラー／1=アラーム）
	ushort			Count;					// 発生中エラーアラーム総数
	PIP_ErrAlarm	stErrAlm[PIP_OCCUR_ERROR_ALARM_MAX];
} DATA_KIND_16_11;

// 振替精算応答
typedef struct{
	DATA_BASIC		DataBasic;				// 基本ﾃﾞｰﾀ
	PIP_COMMON		Common;					// Park I Pro共通データ
	ushort			RcvKind;				// 応答種別
	ushort			Result;					// 結果
	uchar			OpenClose;				// 営休業状況
	uchar			MntMode;				// 精算機動作モード状況(0=通常/1=メンテ)
	uchar			ErrOccur;				// エラー発生状況(0=なし/1=あり)
	uchar			AlmOccur;				// アラーム発生状況(0=なし/1=あり)
	ushort			PayState;				// 精算処理状態
	ushort			PassUse;				// 振替元定期利用有無
	PIP_FlpInfo		stSrcInfo;				// 振替元車室情報
	PIP_FlpInfo		stDestInfo;				// 振替先車室情報
	ulong			PassParkingNo;			// 定期券駐車場番号
	ulong			PassID;					// 定期券ID
	ushort			PassKind;				// 定期券種別
	long			Remain;					// 支払残額
} DATA_KIND_16_04;

// 遠隔精算入庫時刻指定精算応答
typedef struct {
	DATA_BASIC		DataBasic;				// 基本ﾃﾞｰﾀ
	PIP_COMMON		Common;					// Park I Pro共通データ
	ushort			RcvKind;				// 応答種別
	ushort			Result;					// 結果
	ushort			Area;					// 区画
	ulong			ulNo;					// 車室番号
	date_time_rec2	PayTime;				// 料金計算年月日時分秒
	date_time_rec2	InTime;					// 入庫年月日時分秒
	uchar			ParkTimeDay;			// 駐車時間日
	uchar			ParkTimeHour;			// 駐車時間時
	uchar			ParkTimeMin;			// 駐車時間分
	uchar			ParkTimeSec;			// 駐車時間秒
	ulong			ParkTotalSec;			// 駐車総時間秒数
	ushort			Syubet;					// 料金種別
	ulong			Price;					// 駐車料金			0～
// 仕様変更(S) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
	ulong			FutureFee;				// 後日精算予定額
// 仕様変更(E) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
	PIP_Discount	stDiscount;				// 割引情報
} DATA_KIND_16_08;

// 遠隔精算金額指定精算応答
typedef struct {
	DATA_BASIC		DataBasic;				// 基本ﾃﾞｰﾀ
	PIP_COMMON		Common;					// Park I Pro共通データ
	ushort			Result;					// 結果
// 仕様変更(S) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
	ushort			Type;					// 遠隔精算種別
// 仕様変更(E) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
	ushort			Area;					// 区画
	ulong			ulNo;					// 車室番号
	ushort			Syubet;					// 料金種別
	ulong			Price;					// 駐車料金			0～
// 仕様変更(S) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
	ulong			FutureFee;			// 後日精算予定額
// 仕様変更(E) K.Onodera 2016/10/25 遠隔精算フォーマット変更対応
	PIP_Discount	stDiscount;				// 割引情報
} DATA_KIND_16_09;

// MH322914 (e) kasiyama 2016/07/15 AI-V対応
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(電文対応)
/*--------------------------------------------------------------------------*/
/*	長期駐車情報データ(ﾃﾞｰﾀ種別61)											*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	DATA_BASIC		DataBasic;										// 基本ﾃﾞｰﾀ
	ushort			FmtRev;											// フォーマットRev.№
	uchar			dtc;											// ステータス(発生/解除/全解除)
	uchar			Reserve;										// 予備（サイズ調整用）
	proc_date		InTime;											// 入庫日時
	proc_date		OutTime;										// 出庫日時
	ulong			Ck_Time;										// 長期駐車検出時間
	t_MediaInfo2	Media[4];										// 入庫媒体情報1～4
} DATA_KIND_61_r10;

#define LONGPARK_LOG_SET		0// 発生
#define LONGPARK_LOG_RESET		1// 解除
#define LONGPARK_LOG_ALL_RESET	2// 全解除

#define LONGPARK_LOG_NON		0// 指定なし
#define LONGPARK_LOG_PAY		1// 精算
#define LONGPARK_LOG_KYOUSEI_FUSEI	2// 強制/不正
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(電文対応)
#pragma unpack

/*--------------------------------------------------------------------------*/
/*	Freeﾃﾞｰﾀ																*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	uchar			FreeData[500];									// FreeData
} DATA_KIND_Free;


typedef	union {
	DATA_BASIC		DataBasic;
	DATA_KIND_01	SData01;
	DATA_KIND_02	SData02;
	DATA_KIND_05	SData05;
	DATA_KIND_12	SData12;
	DATA_KIND_20	SData20;
	DATA_KIND_22	SData22;
	DATA_KIND_54	SData54;
	DATA_KIND_56	SData56;
	DATA_KIND_58	SData58;
	DATA_KIND_58_r10	SData58_r10;
	DATA_KIND_100	SData101;
	DATA_KIND_103	SData104;
	DATA_KIND_120	SData120;
	DATA_KIND_121	SData121;
	DATA_KIND_126	SData126;
	DATA_KIND_142	SData142;
	DATA_KIND_119	SData229;
	DATA_KIND_230	SData230;
	DATA_KIND_231	SData231;
	DATA_KIND_99	SData99;
	DATA_KIND_109	SData109;
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
//	DATA_KIND_152	SData152;	// 精算情報データ
// MH810105(S) MH364301 QRコード決済対応
//	DATA_KIND_152_r12	SData152_r12;	// 精算情報データ
	DATA_KIND_152_rXX	SData152_r12;	// 精算情報データ
// MH810105(E) MH364301 QRコード決済対応
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
	DATA_KIND_153	SData153;	// リアルタイム情報
	DATA_KIND_154	SData154;	// センター用データ要求
	DATA_KIND_116	SData116;
	DATA_KIND_122	SData122;
	DATA_KIND_123	SData123;
	DATA_KIND_80	SData208;
	RP_DATA_KIND_80	RP_SData208;
	DATA_KIND_219	SData219;
	DATA_KIND_93	SData221;
	DATA_KIND_94	SData222;
	DATA_KIND_95	SData223;
	DATA_KIND_225	SData225;
	DATA_KIND_226	SData226;
	DATA_KIND_30	SData30;
	DATA_KIND_31	SData31;
	DATA_KIND_32	SData32;
	DATA_KIND_33	SData33;
	DATA_KIND_34	SData34;
	DATA_KIND_35	SData35;
	DATA_KIND_36	SData36;
	DATA_KIND_41	SData41;
	DATA_KIND_Free	SDataFree;
	RP_DATA_KIND_115	SData115;
	DATA_KIND_130	SData130;
	DATA_KIND_132	SData132;
	DATA_KIND_211	SData211;
	DATA_KIND_228	SData228;
	DATA_KIND_235	SData235;

	DATA_KIND_78	SData90;

	DATA_KIND_236	SData236;
	DATA_KIND_105	SData105;
	DATA_KIND_110	SData110;
	DATA_KIND_155	SData155;
	DATA_KIND_157	SData157;
	DATA_KIND_244	SData244;
	DATA_KIND_245	SData245;
	DATA_BASIC_R	DataBasicR;
	DATA_KIND_60_R	SData60_R;
	DATA_KIND_61_R	SData61_R;
	DATA_KIND_83_R	SData63_R;
	DATA_KIND_100_R	SData100_R;
	DATA_KIND_143	SData143;
	DATA_KIND_212	SData212;
	DATA_KIND_255	SData255;
	DATA_KIND_108	SData108;
	DATA_KIND_185	SData185;
	DATA_KIND_14	SData14;
	DATA_KIND_148	SData148;
	DATA_KIND_189	SData189;
	DATA_KIND_190	SData190;
	DATA_KIND_135	SData135;
	DATA_KIND_42	SData42;
	DATA_KIND_43	SData43;
	DATA_KIND_45	SData45;
	DATA_KIND_46	SData46;
	DATA_KIND_53	SData53;
	DATA_KIND_65	SData65;

	DATA_KIND_136_01	SData136_01;	// 開局ｺﾏﾝﾄﾞ:ｵﾝﾗｲﾝﾃｽﾄ
	DATA_KIND_136_03	SData136_03;	// ｸﾚｼﾞｯﾄ与信問合せﾃﾞｰﾀ
	DATA_KIND_136_05	SData136_05;	// ｸﾚｼﾞｯﾄ売上依頼ﾃﾞｰﾀ
	DATA_KIND_136_07	SData136_07;	// ﾃｽﾄｺﾏﾝﾄﾞ
	DATA_KIND_136_09	SData136_09;	// ｸﾚｼﾞｯﾄ返品問合せﾃﾞｰﾀ
	DATA_KIND_118_DL	SData118_DL;
// MH322914 (s) kasiyama 2016/07/15 AI-V対応
	DATA_KIND_16_02		SData16_02;		// 領収証再発行応答
	DATA_KIND_16_04		SData16_04;		// 振替精算応答
	DATA_KIND_16_06		SData16_06;		// 受付券発行応答
	DATA_KIND_16_08		SData16_08;		// 遠隔精算入庫時刻指定精算応答
	DATA_KIND_16_09		SData16_09;		// 遠隔精算金額指定精算応答
	DATA_KIND_16_11		SData16_11;		// 発生中エラー応答テーブル
// MH322914 (e) kasiyama 2016/07/15 AI-V対応
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(電文対応)
	DATA_KIND_61_r10	SData61_r10;
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(電文対応)
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
	DATA_KIND_56_T	SData56_T;
	DATA_KIND_63_T	SData63_T;
	DATA_KIND_64_T	SData64_T;
	DATA_KIND_230_T	SData230_T;
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

} SEND_NTNET_DT;

extern	SEND_NTNET_DT	SendNtnetDt;								// NE-NET送信ﾊﾞｯﾌｧ(ﾜｰｸ)


typedef	union {
	DATA_BASIC		DataBasic;
	DATA_KIND_01	RData01;
	DATA_KIND_02	RData02;
	DATA_KIND_03	RData03;
	DATA_KIND_04	RData04;
	DATA_KIND_22	RData22;
	DATA_KIND_100	RData100;
	DATA_KIND_103	RData103;
	DATA_KIND_109	RData109;
	DATA_KIND_154	RData154;
	DATA_KIND_156	RData156;
	DATA_KIND_110	RData110;
	DATA_KIND_155	RData155;
	DATA_KIND_116	RData116;
	DATA_KIND_117	RData117;
	DATA_KIND_119	RData119;
	DATA_KIND_143	RData143;
	DATA_KIND_80	RData80;
	RP_DATA_KIND_80	RP_RData80;
	RP_DATA_KIND_78	RP_RData78;
	DATA_KIND_91	RData91;
	DATA_KIND_93	RData93;
	DATA_KIND_94	RData94;
	DATA_KIND_95	RData95;
	DATA_KIND_97	RData97;
	DATA_KIND_98	RData98;
	DATA_KIND_66	RData66;
	DATA_KIND_67	RData67;
	DATA_KIND_68	RData68;
	DATA_KIND_69	RData69;
	DATA_KIND_70	RData70;
	DATA_KIND_72	RData72;
	DATA_KIND_77	RData77;
	DATA_KIND_78	RData78;
	DATA_KIND_83	RData83;
	RP_DATA_KIND_114	RData114;
	DATA_KIND_234	RData234;
	DATA_KIND_236	RData236;
	DATA_KIND_92	RData92;
	DATA_KIND_240	RData240;
	DATA_KIND_243	RData243;
	DATA_BASIC_R	DataBasicR;
	DATA_KIND_83_R	RData83_R;
	DATA_KIND_100_R	RData101_R;
	DATA_KIND_60_R	RData60_R;
	DATA_KIND_90_R	RData90_R;
	DATA_KIND_142	RData142;
	DATA_KIND_84	RData84;
	DATA_KIND_151	RData151;
	DATA_KIND_254	RData254;
	DATA_KIND_184	RData184;
	DATA_KIND_149	RData149;
	DATA_KIND_188	RData188;

	DATA_KIND_137_02	RData137_02;	// 開局ｺﾏﾝﾄﾞ応答:ｵﾝﾗｲﾝﾃｽﾄ
	DATA_KIND_137_04	RData137_04;	// ｸﾚｼﾞｯﾄ与信問合せ結果ﾃﾞｰﾀ
	DATA_KIND_137_06	RData137_06;	// ｸﾚｼﾞｯﾄ売上依頼結果ﾃﾞｰﾀ
	DATA_KIND_137_08	RData137_08;	// ﾃｽﾄｺﾏﾝﾄﾞ応答
	DATA_KIND_137_0A	RData137_0A;	// ｸﾚｼﾞｯﾄ返品問合せ結果ﾃﾞｰﾀ
	DATA_KIND_111_DL	RData111_DL;
// MH322914 (s) kasiyama 2016/07/15 AI-V対応
	DATA_KIND_16_01		RData16_01;		// 領収証再発行要求
	DATA_KIND_16_03		RData16_03;		// 振替精算要求
	DATA_KIND_16_05		RData16_05;		// 受付券発行要求
	DATA_KIND_16_07		RData16_07;		// 遠隔精算要求
	DATA_KIND_16_10		RData16_10;		// 発生中エラーアラーム要求
// MH322914 (e) kasiyama 2016/07/15 AI-V対応

} RECV_NTNET_DT;

extern	RECV_NTNET_DT	RecvNtnetDt;								// NE-NET受信ﾊﾞｯﾌｧ(ﾜｰｸ)

extern	DATA_KIND_58	SData58_bk;

extern	short		NTNetID100ResetFlag;							// 制御データリセット指示実行フラグ
extern	short		NTNetTotalEndFlag;								// 集計データ最終電文受信フラグ
extern	short		NTNetTotalEndError;								// データ要求２NG受信
extern	NTNETCTRL_FUKUDEN_PARAM	NtNet_FukudenParam;					// 車室パラメータ/ロック種別パラメータ 行進用

extern	ulong		NTNetDataCont[4];								// 各ﾃﾞｰﾀ追い番
																	// 0=入庫追い番
																	// 1=出庫追い番
																	// 2=精算追い番
																	// 3=精算中止

typedef struct {
	ulong			MachineNo;										// 前回受信した機械№
	ulong			PayCount;										// 前回受信した精算追い番
	ushort			PayMethod;										// 前回受信した精算方法
	ushort			PayClass;										// 前回受信した処理区分
	ushort			PayMode;										// 前回受信した精算ﾓｰﾄﾞ
	ulong			LockNo;											// 前回受信した区画情報
	date_time_rec2	OutTime;										// 前回受信した精算年月日時分秒
} RECV_MULTI_DT22;

extern	RECV_MULTI_DT22	RcvDt22;
extern	date_time_rec2	NTNetTTotalTime;
extern	date_time_rec2	NTNetTime_152;
extern	date_time_rec2	NTNetTime_152_wk;

// 精算情報データ保持用
typedef struct	{								// 入庫日時
	short	year;
	char	mon;
	char	day;
	char 	week;
	char	hour;
	char	min;
// GG129000(S) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
	char	sec;
// GG129000(E) T.Nagai 2023/01/23 ゲート式車番チケットレスシステム対応（入庫時刻に秒をセットする）
} NTNET_CAR_IN_TIM;																	/*　　　　　　　　　　　　　　　　　　　　　　*/

typedef	struct {
	ulong			w;			// whole
	ulong			i;			// individual
} NTNET152_ST_OIBAN;

enum {
	NTNET_152_OIBAN = 0,								// 精算追番
	NTNET_152_PAYMETHOD,								// 精算方法
	NTNET_152_PAYMODE,									// 精算モード
// MH810100(S) K.Onodera 2020/02/05 車番チケットレス(LCD_IF対応)
	NTNET_152_CMACHINENO,								// 駐車券機械№(入庫機械№)
// MH810100(E) K.Onodera 2020/02/05 車番チケットレス(LCD_IF対応)
	NTNET_152_WPLACE,									// 駐車位置ﾃﾞｰﾀ
	NTNET_152_KAKARINO,									// 係員№
	NTNET_152_OUTKIND,									// 精算出庫
	NTNET_152_COUNTSET,									// 在車カウント
	NTNET_152_CARINTIME,								// 入庫日時
	NTNET_152_RECEIPTISSUE,								// 領収証発行有無
	NTNET_152_SYUBET,									// 料金種別
	NTNET_152_PRICE,									// 駐車料金
	NTNET_152_CASHPRICE,								// 現金売上
	NTNET_152_INPRICE,									// 投入金額
	NTNET_152_CHGPRICE,									// 釣銭金額
// 不具合修正(S) K.Onodera 2016/12/08 #1642 振替過払い金(現金による払い戻し)が、 精算情報データの釣銭額に加算されない
	NTNET_152_KABARAI,									// 振替過払い金(払い戻す額)
// 不具合修正(E) K.Onodera 2016/12/08 #1642 振替過払い金(現金による払い戻し)が、 精算情報データの釣銭額に加算されない
	NTNET_152_INCOIN,									// 金種毎入金枚数
	NTNET_152_OUTCOIN,									// 金種毎出金枚数
	NTNET_152_FESCROW,									// 精算データ、精算情報データで1000円の払戻し枚数をセットするフラグ
	NTNET_152_HARAIMODOSHIFUSOKU,						// 払戻不足額
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
	NTNET_152_CARD_FUSOKU_TYPE,							// 払戻不足媒体
	NTNET_152_CARD_FUSOKU,								// 払戻不足額(現金以外)
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
	NTNET_152_ANTIPASSCHECK,							// アンチパスチェック
	NTNET_152_PARKNOINPASS,								// 定期券 駐車場№
	NTNET_152_PKNOSYU,									// 定期券駐車場Ｎｏ．種別
	NTNET_152_TEIKIID,									// 定期券id
	NTNET_152_TEIKISYU,									// 定期券種別
// MH810100(S) K.Onodera 2020/02/17 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
	NTNET_152_MEDIAKIND1,								// 種別(媒体)
	NTNET_152_MEDIACARDNO1,								// カード番号
	NTNET_152_MEDIACARDINFO1,							// カード番号
	NTNET_152_MEDIAKIND2,								// 種別(媒体)
	NTNET_152_MEDIACARDNO2,								// カード番号
	NTNET_152_MEDIACARDINFO2,							// カード番号
// MH810100(E) K.Onodera 2020/02/17 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
	NTNET_152_ECARDKIND,								// カード決済区分
	NTNET_152_EPAYRYO,									// 電子マネー決済額
	NTNET_152_ECARDID,									// ｶｰﾄﾞID
	NTNET_152_EPAYAFTER,								// 決済後Suica残高
// MH321800(S) D.Inaba ICクレジット対応
	NTNET_152_ECINQUIRYNUM,								// 問合せ番号
// MH321800(E) D.Inaba ICクレジット対応
	NTNET_152_CPAYRYO,									// クレジット決済額
	NTNET_152_CCARDNO,									// クレジットカード会員番号
	NTNET_152_CCCTNUM,									// 端末識別番号
	NTNET_152_CKID,										// ＫＩＤコード
	NTNET_152_CAPPNO,									// 承認番号
	NTNET_152_CCENTEROIBAN,								// センター処理通番
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
	NTNET_152_CTRADENO,									// 加盟店取引番号
	NTNET_152_SLIPNO,									// 伝票番号
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
	NTNET_152_DPARKINGNO,								// 割引 駐車場№
	NTNET_152_DSYU,										// 割引 割引種別
	NTNET_152_DNO,										// 割引 割引区分
	NTNET_152_DCOUNT,									// 割引 枚数
	NTNET_152_DISCOUNT,									// 割引 割引額(割引時間)
	NTNET_152_DINFO1,									// 割引 割引情報1
	NTNET_152_DINFO2,									// 割引 割引情報2
// MH810105(S) MH364301 QRコード決済対応
	NTNET_152_TRANS_STS,								// 取引ステータス
	NTNET_152_QR_MCH_TRADE_NO,							// Mch取引番号
	NTNET_152_QR_PAY_TERM_ID,							// 支払端末ID
	NTNET_152_QR_PAYKIND,								// QR決済ブランド
// MH810105(E) MH364301 QRコード決済対応
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
	NTNET_152_PREVDISCOUNT,								// 割引 今回使用した前回精算までの割引金額
	NTNET_152_PREVUSAGEDISCOUNT,						// 割引 前回精算までの使用済み割引金額
	NTNET_152_PREVUSAGEDCOUNT,							// 割引 前回精算までの使用済み枚数
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// GG124100(S) R.Endo 2022/08/19 車番チケットレス3.0 #6520 使用したサービス券の情報が領収証に印字されない
	NTNET_152_FEEKINDSWITCHSETTING,						// 割引 種別切換先車種
	NTNET_152_DROLE,									// 割引 割引役割
// GG124100(E) R.Endo 2022/08/19 車番チケットレス3.0 #6520 使用したサービス券の情報が領収証に印字されない
};

typedef struct {
	NTNET152_ST_OIBAN	Oiban;		// 精算追番(0～99999)
	uchar		PayMethod;			// 精算方法(0＝券なし精算/1＝駐車券精算/2＝定期券精算/3＝定期券併用精算/4＝紛失精算)
	uchar		PayMode;			// 精算モード(0＝自動精算/1＝半自動精算/2＝手動精算/3＝精算なし/4＝遠隔精算)
// MH810100(S) K.Onodera 2020/02/05 車番チケットレス(LCD_IF対応)
	uchar		CMachineNo;			// 駐車券機械№(入庫機械№)	0～255
// MH810100(E) K.Onodera 2020/02/05 車番チケットレス(LCD_IF対応)
// MH810100(S) K.Onodera 2020/02/05 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	ulong		WPlace;				// 駐車位置ﾃﾞｰﾀ
// MH810100(E) K.Onodera 2020/02/05 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	uchar		KakariNo;			// 係員№	0～9999
	uchar		OutKind;			// 精算出庫	0＝通常精算/1＝強制出庫/(2＝精算なし出庫)/3＝不正出庫/9＝突破出庫
	uchar		CountSet;			// 在車カウント	0＝する(+1)/1＝しない/2＝する(-1)
	NTNET_CAR_IN_TIM	carInTime;	// 入庫日時
	uchar			ReceiptIssue;		// 領収証発行有無	0＝領収証なし/1＝領収証あり
	uchar			Syubet;				// 料金種別			1～
	ulong			Price;				// 駐車料金			0～
	ulong			CashPrice;			// 現金売上			0～
	ulong			InPrice;			// 投入金額			0～
	ulong			ChgPrice;			// 釣銭金額			0～9999
// 不具合修正(S) K.Onodera 2016/12/08 #1642 振替過払い金(現金による払い戻し)が、 精算情報データの釣銭額に加算されない
	ushort			FrkReturn;			// 振替過払い金(払い戻す額)
// 不具合修正(E) K.Onodera 2016/12/08 #1642 振替過払い金(現金による払い戻し)が、 精算情報データの釣銭額に加算されない
	uchar			in_coin[5];											// 金種毎入金枚数(10,50,100,500,1000)
	uchar			out_coin[5];										// 金種毎出金枚数(10,50,100,500,1000)
	uchar			f_escrow;			// 精算データ、精算情報データで1000円の払戻し枚数をセットするフラグ
	ulong			HaraiModoshiFusoku;	// 払戻不足額	0～9999
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
	ushort			CardFusokuType;		// 払出不足媒体種別(現金以外)
	ushort			CardFusokuTotal;	// 払出不足額(現金以外)
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
	uchar			AntiPassCheck;		// アンチパスチェック	0＝チェックON/1＝チェックＯＦＦ/2＝強制ＯＦＦ
	ulong			ParkNoInPass;		// 定期券　駐車場№	0～999999
	uchar			pkno_syu;			// 定期券駐車場Ｎｏ．種別
	ulong			teiki_id;			// 定期券id
	uchar			teiki_syu;			// 定期券種別
// MH810100(S) K.Onodera 2020/02/05 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
	ushort			MediaKind1;			// 種別(媒体)	0～99
	uchar			MediaCardNo1[30];	// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
	uchar			MediaCardInfo1[16];	// カード情報	"0"～"9"、"A"～"Z"(英数字) 左詰め
	ushort			MediaKind2;			// 種別(媒体)	0～99
	uchar			MediaCardNo2[30];	// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
	uchar			MediaCardInfo2[16];	// カード情報	"0"～"9"、"A"～"Z"(英数字) 左詰め
// MH810100(E) K.Onodera 2020/02/05 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
	uchar			e_pay_kind;			// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
	ulong			e_pay_ryo;			// 電子マネー決済額
// MH321800(S) G.So ICクレジット対応
//	uchar			e_Card_ID[16];		// ｶｰﾄﾞID (Ascii 16桁)
	uchar			e_Card_ID[20];		// ｶｰﾄﾞID (Ascii 20桁)
// MH321800(E) G.So ICクレジット対応
	ulong			e_pay_after;		// 決済後Suica残高
// MH321800(S) D.Inaba ICクレジット対応
	uchar			e_inquiry_num[16];	// 問合せ番号
// MH321800(E) D.Inaba ICクレジット対応
	uchar			c_Card_No[20];		// ｸﾚｼﾞｯﾄｶｰﾄﾞ会員番号
	ulong			c_pay_ryo;			// ｸﾚｼﾞｯﾄｶｰﾄﾞ利用金額
	uchar			c_cct_num[16];		// ｸﾚｼﾞｯﾄｶｰﾄﾞ端末識別番号
	uchar			c_kid_code[6];		// KIDｺｰﾄﾞ
	ulong			c_app_no;
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
	uchar			c_trade_no[20];		// 加盟店取引番号
	ulong			c_slipNo;			// 伝票番号
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
	DISCOUNT_DATA	DiscountData[WTIK_USEMAX];						// 割引ﾃﾞｰﾀ
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
	DETAIL_DATA		DetailData[DETAIL_SYU_MAX];						// 明細データ
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
// MH810105(S) MH364301 QRコード決済対応
	uchar			e_Transactiontatus;	// 取引ステータス
	uchar			qr_MchTradeNo[32];	// Mch取引番号
	uchar			qr_PayTermID[16];	// 支払端末ID
	uchar			qr_PayKind;			// QR決済ブランド
// MH810105(E) MH364301 QRコード決済対応
} ntNet_152_saveInf;

typedef union {
	NTNET152_ST_OIBAN	Oiban;		// 精算追番(0～99999)
	uchar		PayMethod;			// 精算方法(0＝券なし精算/1＝駐車券精算/2＝定期券精算/3＝定期券併用精算/4＝紛失精算)
	uchar		PayMode;			// 精算モード(0＝自動精算/1＝半自動精算/2＝手動精算/3＝精算なし/4＝遠隔精算)
// MH810100(S) K.Onodera 2020/02/05 車番チケットレス(LCD_IF対応)
	uchar		CMachineNo;			// 駐車券機械№(入庫機械№)	0～255
// MH810100(E) K.Onodera 2020/02/05 車番チケットレス(LCD_IF対応)
// MH810100(S) K.Onodera 2020/02/05 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	ulong		WPlace;				// 駐車位置ﾃﾞｰﾀ
// MH810100(E) K.Onodera 2020/02/05 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	uchar		KakariNo;			// 係員№	0～9999
	uchar		OutKind;			// 精算出庫	0＝通常精算/1＝強制出庫/(2＝精算なし出庫)/3＝不正出庫/9＝突破出庫
	uchar		CountSet;			// 在車カウント	0＝する(+1)/1＝しない/2＝する(-1)
	NTNET_CAR_IN_TIM	carInTime;	// 入庫日時
	uchar			ReceiptIssue;		// 領収証発行有無	0＝領収証なし/1＝領収証あり
	uchar			Syubet;				// 料金種別			1～
	ulong			Price;				// 駐車料金			0～
	ulong			CashPrice;			// 現金売上			0～
	ulong			InPrice;			// 投入金額			0～
	ulong			ChgPrice;			// 釣銭金額			0～9999
// 不具合修正(S) K.Onodera 2016/12/08 #1642 振替過払い金(現金による払い戻し)が、 精算情報データの釣銭額に加算されない
	ushort			FrkReturn;			// 振替過払い金(払い戻す額)
// 不具合修正(E) K.Onodera 2016/12/08 #1642 振替過払い金(現金による払い戻し)が、 精算情報データの釣銭額に加算されない
	uchar			in_coin[5];			// 金種毎入金枚数(10,50,100,500,1000)
	uchar			out_coin[5];			// 金種毎出金枚数(10,50,100,500,1000)
	uchar			f_escrow;			// 精算データ、精算情報データで1000円の払戻し枚数をセットするフラグ
	ulong			HaraiModoshiFusoku;	// 払戻不足額	0～9999
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
	ushort			CardFusokuType;		// 払出不足媒体種別(現金以外)
	ushort			CardFusokuTotal;	// 払出不足額(現金以外)
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
	uchar			AntiPassCheck;		// アンチパスチェック	0＝チェックON/1＝チェックＯＦＦ/2＝強制ＯＦＦ
	ulong			ParkNoInPass;		// 定期券　駐車場№	0～999999
	uchar			pkno_syu;			// 定期券駐車場Ｎｏ．種別
	ulong			teiki_id;			// 定期券id
	uchar			teiki_syu;			// 定期券種別
// MH810100(S) K.Onodera 2020/02/17 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
	ushort			MediaKind1;			// 種別(媒体)	0～99
	uchar			MediaCardNo1[30];	// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
	uchar			MediaCardInfo1[16];	// カード情報	"0"～"9"、"A"～"Z"(英数字) 左詰め
	ushort			MediaKind2;			// 種別(媒体)	0～99
	uchar			MediaCardNo2[30];	// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
	uchar			MediaCardInfo2[16];	// カード情報	"0"～"9"、"A"～"Z"(英数字) 左詰め
// MH810100(E) K.Onodera 2020/02/17 車番チケットレス(ParkingWeb精算データ精算媒体(車番)対応)
	uchar			e_pay_kind;			// カード決済区分	0=なし、1=クレジット、2=交通系電子マネー(Suica,PASMO,ICOCA等)
	ulong			e_pay_ryo;			// 電子マネー決済額
// MH321800(S) G.So ICクレジット対応
//	uchar			e_Card_ID[16];		// ｶｰﾄﾞID (Ascii 16桁)
	uchar			e_Card_ID[20];		// ｶｰﾄﾞID (Ascii 20桁)
// MH321800(E) G.So ICクレジット対応
	ulong			e_pay_after;		// 決済後Suica残高
// MH321800(S) D.Inaba ICクレジット対応
	uchar			e_inquiry_num[16];	// 問合せ番号
// MH321800(E) D.Inaba ICクレジット対応
	uchar			c_Card_No[20];		// ｸﾚｼﾞｯﾄｶｰﾄﾞ会員番号
	ulong			c_pay_ryo;			// ｸﾚｼﾞｯﾄｶｰﾄﾞ利用金額
	uchar			c_cct_num[16];		// ｸﾚｼﾞｯﾄｶｰﾄﾞ端末識別番号
	uchar			c_kid_code[6];		// KIDｺｰﾄﾞ
	ulong			c_app_no;
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
	uchar			c_trade_no[20];		// 加盟店取引番号
	ulong			c_slipNo;			// 伝票番号
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
	DISCOUNT_DATA	DiscountData;		// 割引ﾃﾞｰﾀ
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
	DETAIL_DATA		DetailData;			// 明細データ
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
// MH810105(S) MH364301 QRコード決済対応
	uchar			e_Transactiontatus;	// 取引ステータス
	uchar			qr_MchTradeNo[32];	// Mch取引番号
	uchar			qr_PayTermID[16];	// 支払端末ID
	uchar			qr_PayKind;			// QR決済ブランド
// MH810105(E) MH364301 QRコード決済対応
} NTNET_152_U_SAVEINF;

extern ntNet_152_saveInf	ntNet_152_SaveData;

/*--------------------------------------------------------------------------*/
/*	要求ﾃﾞｰﾀ受信時ﾊﾞｯｸｱｯﾌﾟｴﾘｱ												*/
/*--------------------------------------------------------------------------*/
typedef	struct {
	ushort			ReceiveFlg;										// 要求受信済みﾌﾗｸﾞ
	ulong			ParkingNo;										// 駐車場№
	ushort			ModelCode;										// 機種ｺｰﾄﾞ
	ulong			MachineNo;										// 機械№
	uchar			SerialNo[6];									// 端末ｼﾘｱﾙ№
	uchar			Year;											// 年
	uchar			Mon;											// 月
	uchar			Day;											// 日
	uchar			Hour;											// 時
	uchar			Min;											// 分
	uchar			Sec;											// 秒
} RECEIVE_BACKUP;

extern	RECEIVE_BACKUP	RecvBackUp;									// NE-NET要求ﾃﾞｰﾀ受信時ﾊﾞｯｸｱｯﾌﾟｴﾘｱ

#define	REMOTE_BUFF_SIZE	1800				// 遠隔設定変更受信ﾊﾞｯﾌｧｻｲｽﾞ

typedef struct {
	int		prepare;		// =1：準備指令受信
	int		bufcnt;			// ＝バッファ済み設定データ数
	int		bufofs;			// ＝受信バッファ先頭
	char	buffer[REMOTE_BUFF_SIZE];	// 受信バッファ
} NTNET_TMP_BUFFER;

extern	NTNET_TMP_BUFFER tmp_buffer;


extern	DATA_KIND_126	Ntnet_Prev_SData126;						// 前回送信 金銭管理データ
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
extern	TURI_KAN	turi_kan_bk;									// 前回ログ作成 金銭管理データ
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間と遠隔を併用する）
// 名鉄協商コールセンター対応で使用する端末間通信送受信データ
// RECV_NTNET_DTを端末間通信用に定義するとメモリが不足するため、名鉄協商コールセンター対応で
// 受信するデータIDのみ定義する
typedef	union {
	DATA_BASIC		DataBasic;
	DATA_KIND_109	RData109;
} RECV_NTNET_TERM_DT;

extern RECV_NTNET_TERM_DT	RecvNtnetTermDt;						// 端末間通信用受信データバッファ
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（端末間と遠隔を併用する）

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/
extern	int		ntnet_decision_credit(credit_use *p);
extern	ushort	NTNET_Edit_Data22( Receipt_data *p_RcptDat, DATA_KIND_22 *p_NtDat );
extern	ushort	NTNET_Edit_Data22_SK( Receipt_data *p_RcptDat, DATA_KIND_22 *p_NtDat );
extern	void	NTNET_Snd_Data22_FusDel( ushort pr_lokno, ushort paymethod, ushort payclass, ushort outkind );
extern void	Make_Log_Enter( unsigned short );
extern void	Make_Log_Enter_frs( unsigned short , void * );
// MH810100(S) K.Onodera 2020/01/15 車番チケットレス(RT精算データ対応)
//extern unsigned short	NTNET_Edit_Data20( enter_log *, DATA_KIND_20 * );
// MH810100(S) K.Onodera 2020/01/15 車番チケットレス(RT精算データ対応)
extern unsigned short	NTNET_Edit_Data120( Err_log *, DATA_KIND_120 * );
extern unsigned short	NTNET_Edit_Data121( Arm_log *, DATA_KIND_121 * );
extern unsigned short	NTNET_Edit_Data63( Err_log *, DATA_KIND_63 * );
extern unsigned short	NTNET_Edit_Data64( Arm_log *, DATA_KIND_64 * );
extern unsigned short	NTNET_Edit_Data122( Mon_log *, DATA_KIND_122 * );
extern unsigned short	NTNET_Edit_Data123( Ope_log *, DATA_KIND_123 * );
extern unsigned short	NTNET_Edit_Data131( COIN_SYU *, DATA_KIND_130 * );
extern unsigned short	NTNET_Edit_Data133( NOTE_SYU *, DATA_KIND_132 * );
extern void	Make_Log_ParkCarNum( void );
extern unsigned short	NTNET_Edit_Data236( ParkCar_log *, DATA_KIND_236 * );
extern unsigned short	NTNETDOPA_Edit_Data14( LongPark_log *, DATA_KIND_14 * );
// MH322914 (s) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
//extern void	Make_Log_MnyMng( ushort );
extern BOOL	Make_Log_MnyMng( ushort );
// MH322914 (e) kasiyama 2016/07/13 テーブルデータの件数が不正に加算される[共通バグNo.1221](MH341106)
extern unsigned short	NTNET_Edit_Data126( TURI_KAN *, DATA_KIND_126 * );
extern unsigned short	NTNET_Edit_Data135( TURI_KAN *, DATA_KIND_135 * );
extern	void	Make_Log_TGOUKEI( void );
#define	NTNET_Edit_Data30( syu, ntdat )		NTNET_Edit_SyukeiKihon( syu, 30, ntdat )
#define	NTNET_Edit_Data31( syu, ntdat )		NTNET_Edit_SyukeiRyokinMai( syu, 31, ntdat )
#define	NTNET_Edit_Data32( syu, ntdat )		NTNET_Edit_SyukeiBunrui( syu, 32, ntdat )
#define	NTNET_Edit_Data33( syu, ntdat )		NTNET_Edit_SyukeiWaribiki( syu, 33, ntdat )
#define	NTNET_Edit_Data34( syu, ntdat )		NTNET_Edit_SyukeiTeiki( syu, 34, ntdat )
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
// #define	NTNET_Edit_Data35( syu, ntdat )		NTNET_Edit_SyukeiShashitsuMai( syu, 35, ntdat )
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
#define	NTNET_Edit_Data36( syu, ntdat )		NTNET_Edit_SyukeiKinsen( syu, 36, ntdat )
#define	NTNET_Edit_Data41( syu, ntdat )		NTNET_Edit_SyukeiSyuryo( syu, 41, ntdat )
#define	NTNET_Edit_Data158( syu, ntdat )	NTNET_Edit_SyukeiKihon( syu, 158, ntdat )
#define	NTNET_Edit_Data159( syu, ntdat )	NTNET_Edit_SyukeiRyokinMai( syu, 159, ntdat )
#define	NTNET_Edit_Data160( syu, ntdat )	NTNET_Edit_SyukeiBunrui( syu, 160, ntdat )
#define	NTNET_Edit_Data161( syu, ntdat )	NTNET_Edit_SyukeiWaribiki( syu, 161, ntdat )
#define	NTNET_Edit_Data162( syu, ntdat )	NTNET_Edit_SyukeiTeiki( syu, 162, ntdat )
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
// #define	NTNET_Edit_Data163( syu, ntdat )	NTNET_Edit_SyukeiShashitsuMai( syu, 163, ntdat )
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
#define	NTNET_Edit_Data164( syu, ntdat )	NTNET_Edit_SyukeiKinsen( syu, 164, ntdat )
#define	NTNET_Edit_Data169( syu, ntdat )	NTNET_Edit_SyukeiSyuryo( syu, 169, ntdat )
extern	unsigned short	NTNET_Edit_SyukeiKihon( SYUKEI *, ushort, DATA_KIND_30 * );
extern	unsigned short	NTNET_Edit_SyukeiRyokinMai( SYUKEI *, ushort, DATA_KIND_31 * );
extern	unsigned short	NTNET_Edit_SyukeiBunrui( SYUKEI *, ushort, DATA_KIND_32 * );
extern	unsigned short	NTNET_Edit_SyukeiWaribiki( SYUKEI *, ushort, DATA_KIND_33 * );
extern	unsigned short	NTNET_Edit_SyukeiTeiki( SYUKEI *, ushort, DATA_KIND_34 * );
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
// extern	unsigned short	NTNET_Edit_SyukeiShashitsuMai( SYUKEI *, ushort, DATA_KIND_35 * );
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
extern	unsigned short	NTNET_Edit_SyukeiKinsen( SYUKEI *, ushort, DATA_KIND_36 * );
extern	unsigned short	NTNET_Edit_SyukeiSyuryo( SYUKEI *, ushort, DATA_KIND_41 * );
// MH810100(S) K.Onodera 2019/11/15 車番チケットレス (RT精算データ対応)
//extern	unsigned char	NTNET_Edit_isData20_54(enter_log 		*p_RcptDat);	//	入庫フォーマット判定(20/54)
// MH810100(E) K.Onodera 2019/11/15 車番チケットレス (RT精算データ対応)
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
extern	unsigned short	NTNET_Edit_SyukeiKihon_T( SYUKEI *, ushort, DATA_KIND_42_T * );
extern	unsigned short	NTNET_Edit_SyukeiRyokinMai_T( SYUKEI *, ushort, DATA_KIND_43_T * );
extern	unsigned short	NTNET_Edit_SyukeiBunrui_T( SYUKEI *, ushort, DATA_KIND_44_T * );
extern	unsigned short	NTNET_Edit_SyukeiWaribiki_T( SYUKEI *, ushort, DATA_KIND_45_T * );
extern	unsigned short	NTNET_Edit_SyukeiTeiki_T( SYUKEI *, ushort, DATA_KIND_46_T * );
extern	unsigned short	NTNET_Edit_SyukeiKinsen_T( SYUKEI *, ushort, DATA_KIND_48_T * );
extern	unsigned short	NTNET_Edit_SyukeiSyuryo_T( SYUKEI *, ushort, DATA_KIND_53_T * );
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）

extern	unsigned char	NTNET_Edit_isData22_56(Receipt_data 	*p_RcptDat);	//	精算フォーマット判定(22/56)
extern	unsigned char	NTNET_Edit_isData236_58(ParkCar_log 	*p_RcptDat);	//	駐車台数フォーマット判定(236/58)
// MH810100(S) K.Onodera 2019/11/15 車番チケットレス (RT精算データ対応)
////	新入庫NT-NETフォーマット設定(20ﾌｫｰﾏｯﾄにFmtRev, PARKCAR_DATA1を付加)
//extern	unsigned short	NTNET_Edit_Data54(enter_log *p_RcptDat,	DATA_KIND_54 	*p_NtDat );
////	新入庫NT-NETフォーマット設定(20ﾌｫｰﾏｯﾄにFmtRev, PARKCAR_DATA11を付加)
//extern	unsigned short	NTNET_Edit_Data54_r10(enter_log *p_RcptDat,	DATA_KIND_54_r10	*p_NtDat );
//// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
//extern	unsigned short	NTNET_Edit_Data54_r13(enter_log *p_RcptDat,	DATA_KIND_54_r13	*p_NtDat );
//// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応
// MH810100(E) K.Onodera 2019/11/15 車番チケットレス (RT精算データ対応)
// MH810105(S) MH364301 QRコード決済対応
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
extern	unsigned short	NTNET_Edit_Data56_T( Receipt_data *p_RcptDat, DATA_KIND_56_T *p_NtDat );
extern	unsigned short	NTNET_Edit_Data63_T( Err_log *, DATA_KIND_63_T * );
extern	unsigned short	NTNET_Edit_Data64_T( Arm_log *, DATA_KIND_64_T * );
extern unsigned short	NTNET_Edit_Data135_T( TURI_KAN *, DATA_KIND_135_T * );
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
////	新精算NT-NETフォーマット設定(22ﾌｫｰﾏｯﾄにFmtRev, PARKCAR_DATA1を付加)	// 参照 = NTNET_Edit_Data22
//extern	unsigned short	NTNET_Edit_Data56(Receipt_data *p_RcptDat,	DATA_KIND_56 	*p_NtDat );
////	新精算NT-NETフォーマット設定(22ﾌｫｰﾏｯﾄにFmtRev, PARKCAR_DATA1を付加)	// 参照 = NTNET_Edit_Data22_SK
//extern	unsigned short	NTNET_Edit_Data56_SK(Receipt_data 	*p_RcptDat,	DATA_KIND_56 	*p_NtDat );
////	新精算NT-NETフォーマット設定(22ﾌｫｰﾏｯﾄにFmtRev, PARKCAR_DATA11を付加)	// 参照 = NTNET_Edit_Data22
// MH810105(E) MH364301 QRコード決済対応
// MH321800(S) Y.Tanizaki ICクレジット対応
//extern	unsigned short	NTNET_Edit_Data56_r10(Receipt_data *p_RcptDat,	DATA_KIND_56_r10	*p_NtDat );
//// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
//extern	unsigned short	NTNET_Edit_Data56_r14( Receipt_data *p_RcptDat, DATA_KIND_56_r14 *p_NtDat );
//// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応
// MH810105(S) MH364301 QRコード決済対応
//extern	unsigned short	NTNET_Edit_Data56_r17( Receipt_data *p_RcptDat, DATA_KIND_56_r17 *p_NtDat );
extern	unsigned short	NTNET_Edit_Data56_rXX( Receipt_data *p_RcptDat, DATA_KIND_56_rXX *p_NtDat );
// MH810105(E) MH364301 QRコード決済対応
// MH321800(E) Y.Tanizaki ICクレジット対応
//	WEB用駐車台数用NT-NETフォーマット設定(236ﾌｫｰﾏｯﾄとは別に58ﾌｫｰﾏｯﾄを作成)	// 参照 = NTNET_Edit_Data236
extern	unsigned short	NTNET_Edit_Data58(ParkCar_log		*p_RcptDat,	DATA_KIND_58 	*p_NtDat );
//	WEB用駐車台数用NT-NETフォーマット設定(236ﾌｫｰﾏｯﾄとは別に58_r10ﾌｫｰﾏｯﾄを作成)	// 参照 = NTNET_Edit_Data236
extern	unsigned short	NTNET_Edit_Data58_r10(ParkCar_log	*p_RcptDat,	DATA_KIND_58_r10	*p_NtDat_r10 );
// GG129000(S) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
//extern	unsigned short	NTNET_Edit_SyukeiKihon_r10( SYUKEI *syukei, ushort Type, DATA_KIND_42 *p_NtDat );
//// 仕様変更(S) K.Onodera 2016/11/04 集計基本データフォーマット対応
//extern	unsigned short	NTNET_Edit_SyukeiKihon_r13( SYUKEI *syukei, ushort Type, DATA_KIND_42_r13 *p_NtDat );
//// 仕様変更(E) K.Onodera 2016/11/04 集計基本データフォーマット対応
//// 仕様変更(S) K.Onodera 2016/12/14 集計基本データフォーマット対応
////extern	unsigned short	NTNET_Edit_SyukeiRyokinMai_r10( SYUKEI *syukei, ushort Type, DATA_KIND_43 *p_NtDat );
////extern	unsigned short	NTNET_Edit_SyukeiRyokinMai_r10( SYUKEI *syukei, ushort Type, DATA_KIND_43 *p_NtDat );
////extern	unsigned short	NTNET_Edit_SyukeiWaribiki_r10( SYUKEI *syukei, ushort Type, DATA_KIND_45 *p_NtDat );
////extern	unsigned short	NTNET_Edit_SyukeiTeiki_r10( SYUKEI *syukei, ushort Type, DATA_KIND_46 *p_NtDat );
////extern	unsigned short	NTNET_Edit_SyukeiSyuryo_r10( SYUKEI *syukei, ushort Type, DATA_KIND_53 *p_NtDat );
//extern	unsigned short	NTNET_Edit_SyukeiRyokinMai_r13( SYUKEI *syukei, ushort Type, DATA_KIND_43 *p_NtDat );
//extern	unsigned short	NTNET_Edit_SyukeiWaribiki_r13( SYUKEI *syukei, ushort Type, DATA_KIND_45 *p_NtDat );
//extern	unsigned short	NTNET_Edit_SyukeiTeiki_r13( SYUKEI *syukei, ushort Type, DATA_KIND_46 *p_NtDat );
//extern	unsigned short	NTNET_Edit_SyukeiSyuryo_r13( SYUKEI *syukei, ushort Type, DATA_KIND_53 *p_NtDat );
//// 仕様変更(E) K.Onodera 2016/12/14 集計基本データフォーマット対応
extern	unsigned short	NTNET_Edit_SyukeiKihon_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_42 *p_NtDat );
extern	unsigned short	NTNET_Edit_SyukeiRyokinMai_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_43 *p_NtDat );
extern	unsigned short	NTNET_Edit_SyukeiWaribiki_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_45 *p_NtDat );
extern	unsigned short	NTNET_Edit_SyukeiTeiki_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_46 *p_NtDat );
extern	unsigned short	NTNET_Edit_SyukeiSyuryo_rXX( SYUKEI *syukei, ushort Type, DATA_KIND_53 *p_NtDat );
// GG129000(E) M.Fujikawa 2023/09/26 ゲート式車番チケットレスシステム対応　集計データのフォーマットRev.NoがRev.15になっていない　不具合#7132
extern	void	NTNET_Data152Save(void *saveData, uchar saveDataCategory);
extern	void NTNET_Data152_DiscDataSave(void *saveData, uchar saveDataCategory, uchar saveIndex);
extern	void NTNET_Data152_DiscDataClear(void);
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
extern	void NTNET_Data152_DetailDataSave(void *saveData, uchar saveDataCategory, uchar saveIndex);
extern	void NTNET_Data152_DetailDataClear(void);
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
extern	void NTNET_Data152_SaveDataClear(void);
extern	void	NTNET_CtrlRecvData( void );
extern	void NTNET_Data152_SaveDataUpdate(void);
uchar	NTNET_Snd_Data136_01( uchar ReSend );
uchar	NTNET_Snd_Data136_03( void );
uchar	NTNET_Snd_Data136_05( uchar ReSend );
uchar	NTNET_Snd_Data136_07( uchar ReSend );
uchar	NTNET_Snd_Data136_09( uchar ReSend );
extern	void NTNET_CtrlRecvData_DL( void );
extern	void NTNET_RevData111_DL(void);
extern	void NTNET_Snd_Data118_DL(t_ProgDlReq *pDlReq);
extern	unsigned short NTNET_Edit_Data125_DL(Rmon_log *p_RcptDat, DATA_KIND_125_DL *p_NtDat);
// MH322914(S) K.Onodera 2016/09/16 AI-V対応：振替精算
extern	Receipt_data* GetFurikaeSrcReciptData( void );
// MH322914(E) K.Onodera 2016/09/16 AI-V対応：振替精算
// 不具合修正(S) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
extern void SetVehicleCountDate( ulong val );
extern ulong GetVehicleCountDate( void );
extern void AddVehicleCountSeqNo( void );
extern void ClrVehicleCountSeqNo( void );
extern ushort GetVehicleCountSeqNo( void );
// 不具合修正(E) K.Onodera 2016/11/30 #1586 振替元精算データの取消データで、台数管理追番の年月日時分が送信時の年月日時分になっている
// MH322917(S) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
extern void Make_Log_LongParking_Pweb( unsigned long pr_lokno, unsigned short time, uchar knd, uchar knd2);
extern unsigned short NTNET_Edit_Data61( LongPark_log_Pweb *p_RcptDat, DATA_KIND_61_r10 *p_NtDat );
// MH322917(E) A.Iiizumi 2018/08/31 長期駐車検出機能の拡張対応(ログ登録)
#endif	/* ___NTNET_DEFH___ */
