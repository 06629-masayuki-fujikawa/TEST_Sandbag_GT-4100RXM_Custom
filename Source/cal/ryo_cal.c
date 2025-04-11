/*[]----------------------------------------------------------------------------------------------[]*/
/*| 料金計算処理メイン																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara																		   |*/
/*| Date		: 2001-11-02																	   |*/
/*[]------------------------------------------------------------- Copyright(C) 2000 AMANO Corp.---[]*/
																	/********************************/
#include	<string.h>												/*								*/
#include	"system.h"												/*								*/
#include	"mem_def.h"												/*								*/
#include	"pri_def.h"												/*								*/
#include	"rkn_def.h"												/* 全デファイン統括				*/
#include	"rkn_cal.h"												/* 料金関連データ				*/
#include	"rkn_fun.h"												/* 全サブルーチン宣言			*/
#include	"tbl_rkn.h"												/*								*/
#include	"ope_def.h"												/*								*/
#include	"LKmain.h"												/*								*/
#include	"prm_tbl.h"												/*								*/
#include	"common.h"												/*								*/
#include	"ntnet.h"												/*								*/
#include	"ntnet_def.h"											/*								*/
#include	"flp_def.h"												/*								*/
#include	"Suica_def.h"											/*								*/
#include	"cre_ctrl.h"
#include	"raudef.h"

// GG120600(S) // Phase9 Versionを分ける
/*[]----------------------------------------------------------------------[]*/
/*| 料金計算モジュール 部番                                                |*/
/*[]----------------------------------------------------------------------[]*/
// センター用端末情報データに料金計算dllバージョンをセットする仕様より新規に定義する
// 料金計算dllバージョンは基準となっているメインプログラムのバージョンをセットする
// メインプログラムのバージョンアップで料金計算エンジンの変更があった場合のみ料金計算dllバージョンもアップされる仕様とする
// 以下変更する際の例を記す
// 料金計算エンジンの変更 プログラムバージョン 料金計算dllバージョン
// なし                   GG120600             MH322918
const	ver_rec		VERSNO_RYOCAL = {
	'0',' ','M','H','8','1','7','8','0','0'				// MH817800
};
// GG120600(E) // Phase9 Versionを分ける
	
																	/*								*/
const uchar	CHG_PKN[4][4] = {										/*								*/
	{ RY_SKC, RY_SKC_K, RY_SKC_K2, RY_SKC_K3 },						/* ｻｰﾋﾞｽ券						*/
	{ RY_KAK, RY_KAK_K, RY_KAK_K2, RY_KAK_K3 },						/* 掛売券						*/
	{ RY_WBC, RY_WBC_K, RY_WBC_K2, RY_WBC_K3 },						/* 割引券						*/
	{ RY_FRE, RY_FRE_K, RY_FRE_K2, RY_FRE_K3 },						/* 回数券						*/
};																	/*								*/
																	/*								*/
	DISCOUNT_DATA2	wk_DicData;
																	/*								*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 料金計算処理メイン																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: ryo_cal( r_knd, num )															   |*/
/*| PARAMETER	: r_knd	; 料金計算種別															   |*/
/*|						;   0:駐車券															   |*/
/*|						;   1:定期券															   |*/
/*|						;   2:ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ														   |*/
/*|						;   3:ｻｰﾋﾞｽ券															   |*/
/*|						;   4:中止券(未使用)													   |*/
/*|						;   6:掛売駐車券(未使用)												   |*/
/*|						;   7:遠隔精算(金額指定)												   |*/
// MH810105(S) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
//// MH321800(S) hosoda ICクレジット対応 (アラーム取引)
///*|						;   9:相殺割引															   |*/
//// MH321800(E) hosoda ICクレジット対応 (アラーム取引)
// MH810105(E) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
/*|						;  50:不正出庫															   |*/
/*|						; 100:修正精算															   |*/
/*|						; 110:料金計算シミュレータ												   |*/
/*|						; 111:料金計算シミュレータ(遠隔精算入庫時刻指定精算)					   |*/
/*|				  num	; 内部処理用駐車位置番号(1～324)										   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(AMANO)																	   |*/
/*| Date		: 2001-11-02																	   |*/
/*| Update		: 4500EX T.Nakayama																   |*/
/*|       		: ｻｰﾋﾞｽ券,拡張ｻｰﾋﾞｽ券,定期券対応												   |*/
/*|       		: 精算中止後のサービス券対応の移植(FCR-P30033 TF4800N DH917004)					   |*/
/*[]------------------------------------------------------------- Copyright(C) 2000 AMANO Corp.---[]*/
void	ryo_cal( char r_knd, ushort num )							/*								*/
{																	/*								*/
	ulong	sot_tax = 0L;											/*								*/
	ulong	dat = 0L;												/*								*/
	struct	REQ_RKN		*rhs_p;										/*								*/
	short	calcreqnum	;											/* 券毎料金計算要求数			*/
	char	cnt;													/*								*/
	char	i;														/*								*/
	uchar	wrcnt = 0;												/*								*/
// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
	uchar	DetailCnt = 0;
// 仕様変更(S) K.Onodera 2016/12/05 振替額変更
//	ulong	OrgDiscount = 0;
// 仕様変更(E) K.Onodera 2016/12/05 振替額変更
// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応
	uchar	pricnt = 0;												/*								*/
	ushort	wk_dat;													/*								*/
	ushort	wk_dat2;												/*								*/
	ulong	wk_dat3;												/*								*/
// MH810100(S) K.Onodera 2020/02/27 車番チケットレス(買物割引対応)
	ushort	wk_dat4;												/*								*/
// MH810100(E) K.Onodera 2020/02/27 車番チケットレス(買物割引対応)
// MH810100(S) 2020/09/09 #4755 店割引の割引種類対応（【連動評価指摘事項】NT-NET精算データに割引種別2,102の割引情報の値が格納されない(02-0045)）
	ushort	wk_dat5;												/*								*/
// MH810100(E) 2020/09/09 #4755 店割引の割引種類対応（【連動評価指摘事項】NT-NET精算データに割引種別2,102の割引情報の値が格納されない(02-0045)）
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算処理の変更)
	ushort	wk_dat6;												/*								*/
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
	ushort	wk_dat7;
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
	short	wk_adr;													/* アドレス						*/
	short	wk_ses;													/* パラメータセッション№		*/
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算処理の変更)
// MH321800(S) 割引種別8000のセット方法を修正
//	ushort	wk_DiscSyu1;											/* 割引種別：Suicaｶｰﾄﾞ番号		*/
//	ushort	wk_DiscSyu2;											/* 割引種別：Suica支払額、残額	*/
// MH321800(E) 割引種別8000のセット方法を修正
	uchar	chg_flg = 0;
// MH810100(S) K.Onodera 2020/01/27 車番チケットレス(割引済み対応)
	uchar	DiscStatus;
// MH810100(S) K.Onodera 2020/01/27 車番チケットレス(割引済み対応)
	struct clk_rec *pTime;


// MH810100(S) K.Onodera 2019/12/05 車番チケットレス(料金計算)
//	long	svtm,uptm;
// MH810100(E) K.Onodera 2019/12/05 車番チケットレス(料金計算)
	ulong	wk_pay_ryo;				//再精算用プリペイド金額格納エリア

	flp_com	*pflp;
// MH810100(S) K.Onodera 2019/12/05 車番チケットレス(料金計算)
//	uchar	fusei_cal=0;
// MH810100(E) K.Onodera 2019/12/05 車番チケットレス(料金計算)

	struct	CAR_TIM	Adjustment_Beginning;

// MH810105(S) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
//// MH321800(S) hosoda ICクレジット対応 (アラーム取引)
////	アラーム取引後などの残額を相殺して出庫させる
////	計算上はサービス券または掛売券の割引に付けて精算データを作り集計する
//	f_sousai = 0;
//	if (r_knd == 9) {
//		f_sousai = 1;		// 相殺する
//		r_knd = 3;			// サービス券または掛売券
//	}
//// MH321800(E) hosoda ICクレジット対応 (アラーム取引)
// MH810105(E) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
	memcpy( &Adjustment_Beginning, &car_ot, sizeof( Adjustment_Beginning ) );

	WACDOG;// RX630はWDGストップが出来ない仕様のためWDGリセットにする

	if( r_knd == 50 ){												/* 不正出庫による料金計算？		*/
		r_knd = 0;													/* ﾌﾗｸﾞを精算開始時に合わせる	*/
// MH810100(S) K.Onodera 2019/12/05 車番チケットレス(料金計算)
//		fusei_cal = 1;												/* 不正料金計算用のﾌﾗｸﾞをセット	*/
// MH810100(E) K.Onodera 2019/12/05 車番チケットレス(料金計算)
	}
	if( r_knd == 110 ){												/* 料金計算シミュレータによる料金計算？ */
		r_knd = 0;													/* ﾌﾗｸﾞを精算開始時に合わせる	*/
		chg_flg = 1;												/* 料金計算シミュレータ用に変換	*/
	}
// MH322914(S) K.Onodera 2016/08/09 AI-V対応：遠隔精算(入庫時刻指定)
	if( r_knd == 111 ){												/* 料金計算シミュレータによる料金計算？ */
		r_knd = 0;													/* ﾌﾗｸﾞを精算開始時に合わせる	*/
	}
// MH322914(E) K.Onodera 2016/08/09 AI-V対応：遠隔精算(入庫時刻指定)

// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
#if 1	// ここから：クラウド料金計算のfee_init(cal_cloud/cal_cloud.c)に相当。
		// ※修正する際はfee_init側も確認すること。
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
// 不具合修正(S) K.Onodera 2016/12/08 #1645 領収証のプリペイドカード残額が正しく印字されない
//	if((( r_knd == 0 )||( r_knd == 100 ))&&							/* 駐車券or修正精算				*/
// MH810100(S) K.Onodera 2019/12/05 車番チケットレス(料金計算)
//	if((( r_knd == 0 )||( r_knd == 100 )||( r_knd == 7 ))&&			/* 駐車券or修正精算				*/
	if((( r_knd == 0 )||( r_knd == 100 )||( r_knd == 7 )||( r_knd == 8 ))&&
// MH810100(E) K.Onodera 2019/12/05 車番チケットレス(料金計算)
// 不具合修正(E) K.Onodera 2016/12/08 #1645 領収証のプリペイドカード残額が正しく印字されない
		(ryo_buf.credit.pay_ryo == 0))
	{																/*								*/
		memset( req_rhs, 0, sizeof( struct REQ_RKN )*150 );			/* 現在計算後格納ｴﾘｱｸﾘｱ			*/
		memset( &ryo_buf, 0, sizeof( struct RYO_BUF ) );			/* 料金ﾊﾞｯﾌｧｸﾘｱｰ				*/
		ryo_buf.ryo_flg = 0;										/* 通常料金ｾｯﾄ					*/
		ryo_buf.pkiti = num;										/* 駐車位置番号ｾｯﾄ				*/
		memset( &PayData, 0, sizeof( Receipt_data ) );				/* 精算情報,領収証印字ﾃﾞｰﾀ		*/
// MH810102(S) R.Endo 2021/03/17 車番チケットレス フェーズ2.5 #5359 NT-NET精算データにセットする券種の変更
		memset( &ntNet_56_SaveData, 0, sizeof(ntNet_56_SaveData) );	/* 精算データ保持				*/
// MH810102(E) R.Endo 2021/03/17 車番チケットレス フェーズ2.5 #5359 NT-NET精算データにセットする券種の変更
		NTNET_Data152_SaveDataClear();
		memset( &PPC_Data_Detail, 0, sizeof( PPC_Data_Detail ));	/* ﾌﾟﾘﾍﾟ詳細印字ｴﾘｱｸﾘｱ			*/	
		ntnet_nmax_flg = 0;											/*　最大料金ﾌﾗｸﾞｸﾘｱ	*/
		if( FLAPDT.flp_data[num-1].lag_to_in.BIT.LAGIN == ON ){
			ryo_buf.lag_tim_over = 1;								/* ラグタイムオーバーフラグセット */
		}
// MH810100(S) 2020/06/02 【連動評価指摘事項】再精算時に、残額以上かつ駐車料金未満のQR割引を行うと請求額が不正な値となる(No.02-0014)
		if(ryo_buf.lag_tim_over == 1){
			ryo_buf.zenkai = lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.GenkinFee;
			// さらに、決済金額を足す
			for( cnt=0; cnt<ONL_MAX_SETTLEMENT; cnt++ ){
				// 決済区分あり？
				if( lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.stSettlement[cnt].CargeType ){
					ryo_buf.zenkai += lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.stSettlement[cnt].CargeFee;
				}
			}
// MH810100(S) 2020/06/30 【連動評価指摘事項】再精算時の割引適応方法の変更
			ryo_buf.zenkaiWari = 0;
			for( cnt=0; cnt<ONL_MAX_DISC_NUM; cnt++ ){
				// 駐車場№があって種別が999以下
				if( lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscParkNo && 
// MH810100(S) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
// 50/150もは、飛ばす。（必ず計算されてしまうため）
					lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu != NTNET_SYUBET_TIME &&
					lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu != NTNET_SYUBET &&
// MH810100(E) 2020/09/07 #4813 【検証課指摘事項】種別割引を付与した場合にリアルタイム通信の精算データに種別割引が格納されない（No.81）
					lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[cnt].DiscSyu <= NTNET_SECTION_WARI_MAX){
					// 前回割引額(支払額を除く)の合計を出す
					ryo_buf.zenkaiWari = ryo_buf.zenkaiWari + lcdbm_rsp_in_car_info_main.crd_info.dtCalcInfo.stDiscountInfo[cnt].UsedDisc;
				}
			}
// MH810100(E) 2020/06/30 【連動評価指摘事項】再精算時の割引適応方法の変更
		}
// MH810100(E) 2020/06/02 【連動評価指摘事項】再精算時に、残額以上かつ駐車料金未満のQR割引を行うと請求額が不正な値となる(No.02-0014)

		memset( &OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo) );	/* NT-NET精算ﾃﾞｰﾀ付加情報ｸﾘｱ	*/
		memset( PayData.DiscountData, 0, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );/* 精算ﾃﾞｰﾀ割引情報作成ｴﾘｱｸﾘｱ	*/
		NTNET_Data152_DiscDataClear();
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
		NTNET_Data152_DetailDataClear();
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応

		if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
			PayInfo_Class = 9;										// 精算情報データ用処理区分を待避
// MH810100(S) K.Onodera 2020/02/20 車番チケットレス(未精算出庫対応)
			if( lcdbm_rsp_in_car_info_main.shubetsu == 1 ){	// 後日精算
				NTNetTime_152_wk.Year = car_ot_f.year;				// 出庫年
				NTNetTime_152_wk.Mon  = car_ot_f.mon;				// 出庫月
				NTNetTime_152_wk.Day  = car_ot_f.day;				// 出庫日
				NTNetTime_152_wk.Hour = car_ot_f.hour;				// 出庫時
				NTNetTime_152_wk.Min  = car_ot_f.min;				// 出庫分
				NTNetTime_152_wk.Sec  = 0;							// 出庫秒
			}else{
// MH810100(E) K.Onodera 2020/02/20 車番チケットレス(未精算出庫対応)
// MH810100(S)
//			// 精算開始時の時刻をセット
//			NTNetTime_152_wk.Year = CLK_REC.year;					// 年
//			NTNetTime_152_wk.Mon  = CLK_REC.mont;					// 月
//			NTNetTime_152_wk.Day  = CLK_REC.date;					// 日
//			NTNetTime_152_wk.Hour = CLK_REC.hour;					// 時
//			NTNetTime_152_wk.Min  = CLK_REC.minu;					// 分
//			NTNetTime_152_wk.Sec  = (ushort)CLK_REC.seco;			// 秒
			// 精算開始時刻を取得
			pTime = GetPayStartTime();
			// 精算開始時の時刻をセット
			NTNetTime_152_wk.Year = pTime->year;					// 年
			NTNetTime_152_wk.Mon  = pTime->mont;					// 月
			NTNetTime_152_wk.Day  = pTime->date;					// 日
			NTNetTime_152_wk.Hour = pTime->hour;					// 時
			NTNetTime_152_wk.Min  = pTime->minu;					// 分
			NTNetTime_152_wk.Sec  = (ushort)pTime->seco;			// 秒
// MH810100(E)
// MH810100(S) K.Onodera 2020/02/20 車番チケットレス(未精算出庫対応)
			}
// MH810100(E) K.Onodera 2020/02/20 車番チケットレス(未精算出庫対応)
		}

// MH810103 GG119202(S) 不要機能削除(センタークレジット)
//		if( RAU_Credit_Enabale != 0 ) {
//			cre_ctl.PayStartTime.Year = car_ot_f.year;				// 精算開始年
//			cre_ctl.PayStartTime.Mon = car_ot_f.mon;				// 精算開始月
//			cre_ctl.PayStartTime.Day = car_ot_f.day;				// 精算開始日
//			cre_ctl.PayStartTime.Hour = car_ot_f.hour;				// 精算開始時
//			cre_ctl.PayStartTime.Min = car_ot_f.min;				// 精算開始分
//			cre_ctl.PayStartTime.Sec = 0;							// 精算開始秒
//		}
// MH810103 GG119202(E) 不要機能削除(センタークレジット)

		memset( card_use, 0, sizeof( card_use ) );					/* 1精算ｶｰﾄﾞ毎使用枚数ﾃｰﾌﾞﾙ		*/
		memset( card_use2, 0, sizeof( card_use2 ) );				/* 1精算ｻｰﾋﾞｽ券毎使用枚数ﾃｰﾌﾞﾙ	*/
		CardUseSyu = 0;												/* 1精算の割引種類の件数ｸﾘｱ		*/
		discount_tyushi = 0;										/*　割引額　　　　　　　　　　　*/
		memset(&PayData_Sub,0,sizeof(struct Receipt_Data_Sub));		/*	詳細中止ｴﾘｱｸﾘｱ				*/
		Flap_Sub_Num = 0;											/*	詳細中止ｸﾘｱ					*/
		if( r_knd == 0 ){											/* 駐車券						*/
			vl_carchg( num, chg_flg );								/*								*/
// 不具合修正(S) K.Onodera 2016/12/08 #1645 領収証のプリペイドカード残額が正しく印字されない
		}else if( r_knd == 7 ){
			;
// 不具合修正(E) K.Onodera 2016/12/08 #1645 領収証のプリペイドカード残額が正しく印字されない
// MH810100(S) K.Onodera 2019/12/05 車番チケットレス(料金計算)
		}else if( r_knd == 8 ){
			// 領収証用に車番データをセット
			PayData.CarSearchFlg = vl_car_no.CarSearchFlg;			// 車番/日時検索種別
			memcpy( PayData.CarSearchData, vl_car_no.CarSearchData, sizeof(PayData.CarSearchData) );
			memcpy( PayData.CarNumber, vl_car_no.CarNumber, sizeof(PayData.CarNumber) );
			memcpy( PayData.CarDataID, vl_car_no.CarDataID, sizeof(PayData.CarDataID) );
// MH810100(E) K.Onodera 2019/12/05 車番チケットレス(料金計算)
		}else{														/*								*/
#if	SYUSEI_PAYMENT		// SYUSEI_PAYMENT
			vl_scarchg( num );										/* 修正精算						*/
			if( syusei[num-1].infofg & SSS_ZENWARI ){				/* 全額割引使用していた?		*/
				ryo_buf.mis_zenwari = 1;							/* 全額割引ﾌﾗｸﾞON				*/
			}														/*								*/
#endif		// SYUSEI_PAYMENT
		}															/*								*/
	}																/*								*/
// 不具合修正(S) K.Onodera 2016/12/08 #1645 領収証のプリペイドカード残額が正しく印字されない
//// MH322914(S) K.Onodera 2016/08/08 AI-V対応：遠隔精算(精算金額指定)
//	// 遠隔精算？
//	else if( r_knd == 7 ){
//		memset( req_rhs, 0, sizeof( struct REQ_RKN )*150 );			/* 現在計算後格納ｴﾘｱｸﾘｱ			*/
//		memset( &ryo_buf, 0, sizeof( struct RYO_BUF ) );			/* 料金ﾊﾞｯﾌｧｸﾘｱｰ				*/
//		ryo_buf.ryo_flg = 0;										/* 通常料金ｾｯﾄ					*/
//		ryo_buf.pkiti = num;										/* 駐車位置番号ｾｯﾄ				*/
//		memset( &PayData, 0, sizeof( Receipt_data ) );				/* 精算情報,領収証印字ﾃﾞｰﾀ		*/
//	}
//// MH322914(E) K.Onodera 2016/08/08 AI-V対応：遠隔精算(精算金額指定)
// 不具合修正(E) K.Onodera 2016/12/08 #1645 領収証のプリペイドカード残額が正しく印字されない
// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
#endif	// ここまで：クラウド料金計算のfee_init(cal_cloud/cal_cloud.c)に相当。
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
																	/*								*/
																	/*								*/
// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
#if 1	// ここから：クラウド料金計算のryo_buf_n_set(cal_cloud/cal_cloud.c)に相当。
		// ※修正する際はryo_buf_n_set側も確認すること。
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
	ryo_buf_n.require = ryo_buf.zankin;								/*								*/
	ryo_buf_n.dis     = ryo_buf.waribik;							/*								*/
	ryo_buf_n.nyu     = ryo_buf.nyukin;								/*								*/
// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
#endif	// ここまで：クラウド料金計算のryo_buf_n_set(cal_cloud/cal_cloud.c)に相当。
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
																	/*								*/
	tol_dis = ryo_buf.waribik;										/*								*/
																	/*								*/
	ec09();															/* VL料金ﾊﾟﾗﾒｰﾀｾｯﾄ				*/
	if( prm_get( COM_PRM,S_STM,1,1,1 ) == 0 ){						/* 全共通ｻｰﾋﾞｽﾀｲﾑ,ｸﾞﾚｰｽﾀｲﾑ		*/
		se_svt.stim = (short)CPrmSS[S_STM][2];						/* ｻｰﾋﾞｽﾀｲﾑget					*/
		se_svt.gtim = (short)CPrmSS[S_STM][3];						/* ｸﾞﾚｰｽﾀｲﾑget					*/
	}else{															/* 種別毎ｻｰﾋﾞｽﾀｲﾑ				*/
		se_svt.stim = (short)CPrmSS[S_STM][5+(3*(rysyasu-1))];		/* 種別毎ｻｰﾋﾞｽﾀｲﾑget			*/
		se_svt.gtim = (short)CPrmSS[S_STM][6+(3*(rysyasu-1))];		/* 種別毎ｸﾞﾚｰｽﾀｲﾑget			*/
	}																/*								*/

	if( num != 0xffff ){
		SvsTime_Syu[num-1] = rysyasu;								/* 精算時の料金種別ｾｯﾄ（ﾗｸﾞﾀｲﾑ処理用）*/
	}

	if( num != 0xffff ){
		pflp = &FLAPDT.flp_data[num-1];
	}else{
		pflp = &LOCKMULTI.lock_mlt;
	}

	if( pflp->lag_to_in.BIT.LAGIN == ON ){
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_無料時間内のﾗｸﾞﾀｲﾑｵｰﾊﾞは無料とする事_削除
//		se_svt.stim = 0;											/* Yes→ｻｰﾋﾞｽﾀｲﾑを無効とする	*/
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)_無料時間内のﾗｸﾞﾀｲﾑｵｰﾊﾞは無料とする事_削除
	}
	else
	{
// MH322914 (s) kasiyama 2016/07/12 ロック閉タイムをサービスタイムまで延長する[共通バグNo.1211](MH341106)
//		if(	( pflp->mode == FLAP_CTRL_MODE3 ) ||							// 上昇動作中または
//			( pflp->mode == FLAP_CTRL_MODE4 ) ||							// 上昇済み
//			( fusei_cal == 1 ))												// 不正出庫による料金計算実施？
//		{
//			svtm = (long)se_svt.stim;
//			svtm *= 60;
//
//			if( num >= LOCK_START_NO)
//			{
//				uptm = Carkind_Param(ROCK_CLOSE_TIMER, (char)(LockInfo[num - 1].ryo_syu), 5,1);
//			}
//			else
//			{
//				uptm = Carkind_Param(FLAP_UP_TIMER, (char)(LockInfo[num - 1].ryo_syu), 5,1);
//			}
//			uptm = (long)(((uptm/100)*60) + (uptm%100));
//
//
//			if( svtm == uptm )											
//			{
//				if( prm_get( COM_PRM,S_TYP,62,1,2 ) ) { 				// 修正精算機能あり設定
//					if(OPECTL.Pay_mod == 2){							// 修正精算中
//						se_svt.stim = 0;								// ｻｰﾋﾞｽﾀｲﾑ無効
//					}else{												// 通常精算中
//						if( num != 0xffff ){							// 相互精算でない
//							if( syusei[num-1].sei != 2 ){				// 復元車室でない
//								se_svt.stim = 0;						// ｻｰﾋﾞｽﾀｲﾑ無効
//							}											//
//						}else{											// 相互精算のとき
//							se_svt.stim = 0;							// ｻｰﾋﾞｽﾀｲﾑ無効
//						}
//					}													//
//				}else{													// 修正精算機能なし設定であれば復元車室なし
//					se_svt.stim = 0;									// ｻｰﾋﾞｽﾀｲﾑ無効
//				}
//			}
//		}
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)
//		if ( prm_get( COM_PRM, S_TYP, 68, 1, 5 ) == 0 ) {
//			// ロック閉タイマーとサービスタイムが同じ時間の時にサービスタイムをロック閉までとする
//			if(	( pflp->mode == FLAP_CTRL_MODE3 ) ||							// 上昇動作中または
//				( pflp->mode == FLAP_CTRL_MODE4 ) ||							// 上昇済み
//				( fusei_cal == 1 ))												// 不正出庫による料金計算実施？
//			{
//				svtm = (long)se_svt.stim;
//				svtm *= 60;
//	
//				if( num >= LOCK_START_NO)
//				{
//					uptm = Carkind_Param(ROCK_CLOSE_TIMER, (char)(LockInfo[num - 1].ryo_syu), 5,1);
//				}
//				else
//				{
//					uptm = Carkind_Param(FLAP_UP_TIMER, (char)(LockInfo[num - 1].ryo_syu), 5,1);
//				}
//				uptm = (long)(((uptm/100)*60) + (uptm%100));
//	
//	
//				if( svtm == uptm )											
//				{
//					if( prm_get( COM_PRM,S_TYP,62,1,2 ) ) { 				// 修正精算機能あり設定
//						if(OPECTL.Pay_mod == 2){							// 修正精算中
//							se_svt.stim = 0;								// ｻｰﾋﾞｽﾀｲﾑ無効
//						}else{												// 通常精算中
//							if( num != 0xffff ){							// 相互精算でない
//								if( syusei[num-1].sei != 2 ){				// 復元車室でない
//									se_svt.stim = 0;						// ｻｰﾋﾞｽﾀｲﾑ無効
//								}											//
//							}else{											// 相互精算のとき
//								se_svt.stim = 0;							// ｻｰﾋﾞｽﾀｲﾑ無効
//							}
//						}													//
//					}else{													// 修正精算機能なし設定であれば復元車室なし
//						se_svt.stim = 0;									// ｻｰﾋﾞｽﾀｲﾑ無効
//					}
//				}
//			}
//		}
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)
// MH322914 (e) kasiyama 2016/07/12 ロック閉タイムをサービスタイムまで延長する[共通バグNo.1211](MH341106)
	}

	et02();															/* 料金計算						*/
	for( calcreqnum =  0 ;											/* 料金計算要求数				*/
		  calcreqnum < (short)req_crd_cnt	;						/* ex.各券毎に料金計算			*/
		  calcreqnum ++					)							/* 処理数が異なる				*/
	{																/*								*/
		memcpy( &req_rkn, req_crd + calcreqnum,						/* 料金計算要求複写				*/
				sizeof(req_rkn)				) ;						/* ex.2 件目以降→要求域		*/
		et02()	;													/* 料金計算						*/
	}																/*								*/
																	/*								*/
	if( tki_flg != OFF )											/* 時間帯定期後時間割引 		*/
	{																/* ｷｰ処理なら					*/
		if( tki_ken ){												/*								*/
			ryo_buf.dis_fee = 0L;									/* 前回料金割引額ｸﾘｱ			*/
			ryo_buf.dis_tim = 0L;									/* 前回時間割引額ｸﾘｱ			*/
			ryo_buf.fee_amount = 0L;								/* 料金割引額ｸﾘｱ				*/
			ryo_buf.tim_amount = 0L;								/* 割引時間数ｸﾘｱ				*/
			wk_pay_ryo = PayData_Sub.pay_ryo;
			memset(&PayData_Sub,0,sizeof(struct Receipt_Data_Sub));
			PayData_Sub.pay_ryo = wk_pay_ryo;
			wrcnt_sub = 0;
			discount_tyushi = 0;									/*　割引額　　　　　　　　　　　　　　　　　　*/
		}															/*								*/
		for( i = 0; i < (short)tki_ken; i++ )						/* 保存して有る料金計算 		*/
		{															/* 分ﾙｰﾌﾟ						*/
			memset( &req_rkn.param,0x00,21);						/* 料金要求ﾃｰﾌﾞﾙｸﾘｱ				*/
			memcpy( (char *)&req_rkn,								/* 保存してた料金計算要 		*/
					(char *)&req_tki[i],10 );						/* 求を再び要求					*/
			if((req_rkn.param == RY_FRE_K)							/* 拡張1回数券割引要求			*/
			 ||(req_rkn.param == RY_FRE)							/* 回数券割引要求				*/
			 ||(req_rkn.param == RY_FRE_K2)							/* 拡張2回数券割引要求			*/
			 ||(req_rkn.param == RY_FRE_K3)							/* 拡張3回数券割引要求			*/

			 ||(req_rkn.param == RY_EMY)							/* 電子決済カード				*/
// MH810105(S) MH364301 QRコード決済対応
			 ||(req_rkn.param == RY_EQR)							/* QRコード決済     			*/
// MH810105(E) MH364301 QRコード決済対応
			 ||(req_rkn.param == RY_GNG)							/* 減額			*/
// 不具合修正(S) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
			 ||(req_rkn.param == RY_GNG_FEE)						/* 減額			*/
			 ||(req_rkn.param == RY_GNG_TIM)						/* 減額			*/
// 不具合修正(E) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
			 ||(req_rkn.param == RY_FRK)							/* 振替			*/
			 ||(req_rkn.param == RY_PCO_K2)							/* 拡張2ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ			*/
			 ||(req_rkn.param == RY_PCO_K3)							/* 拡張3ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ			*/
			 ||(req_rkn.param == RY_PCO_K)							/* 拡張1ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ			*/
			 ||(req_rkn.param == RY_PCO))							/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ					*/
			{														/*								*/
			   req_rkn.param = 0xff;								/*								*/
			}														/*								*/
			et02();													/* 料金計算処理					*/
		}															/*								*/
		tki_ken = 0;												/* 料金計算保存券数ｸﾘｱ			*/
		re_req_flg = OFF;											/* 時間帯定期後時間割引 		*/
	}																/* ﾌﾗｸﾞﾘｾｯﾄ						*/
																	/*								*/
// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
#if 1	// ここから：クラウド料金計算のryo_buf_n_get(cal_cloud/cal_cloud.c)に相当。
		// ※修正する際はryo_buf_n_get側も確認すること。
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
	ryo_buf.zankin  = ryo_buf_n.require ;							/* 駐車料金ｾｯﾄ					*/
	ryo_buf.tax     = ryo_buf_n.tax;								/* 税金ｾｯﾄ						*/
	ryo_buf.waribik = ryo_buf_n.dis;								/* 割引額						*/
	if( OPECTL.Ope_mod != 22 )										/* 定期更新時でないなら			*/
	ryo_buf.syubet  = (char)(rysyasu - 1);							/* 料金種別ｾｯﾄ(種別切換後)		*/
																	/*								*/
	if( r_knd == 1 )												/* 定期券処理?					*/
	{																/*								*/
		ryo_buf.ryo_flg += 2;										/*								*/
	}																/*								*/
																	/*								*/
	if( ryo_buf.ryo_flg < 2 )										/* 駐車券精算処理				*/
	{																/*								*/
		ryo_buf.tik_syu = rysyasu;									/* 料金種別						*/
		ryo_buf.tyu_ryo = ryo_buf_n.ryo;							/* 駐車料金ｾｯﾄ					*/
	}																/*								*/
	else															/*								*/
	{																/*								*/
		ryo_buf.pas_syu = rysyasu;									/* 定期券の料金種別				*/
		ryo_buf.tei_ryo = ryo_buf_n.ryo;							/* 定期券料金ｾｯﾄ				*/
	}																/*								*/
// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
#endif	// ここまで：クラウド料金計算のryo_buf_n_get(cal_cloud/cal_cloud.c)に相当。
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
																	/*								*/
	/*====================================================*/		/*								*/
	// 定期券処理前の各割引処理を削除する							/*								*/
	/*====================================================*/		/*								*/
	for( cnt = 0; cnt < rhspnt; cnt++ )								/* 料金計算後ﾃﾞｰﾀ数になるまで	*/
	{																/*								*/
		rhs_p = &req_rhs[cnt];										/*								*/
		if(( rhs_p->param == RY_PKC		||							/* 料金計算要求が定期券処理の場合	*/
			 rhs_p->param == RY_PKC_K	||							/*								*/
			 rhs_p->param == RY_PKC_K2	||							/*								*/
			 rhs_p->param == RY_PKC_K3	||				/*								*/
			 ((rhs_p->param == RY_KCH) && (( rhs_p->data[1] >> 16 )== 2))) &&	// 定期券車種切替
			 cnt != 0 )
		{															/*								*/
			for( i = (char)(cnt-1); i > 0; i-- )					/*								*/
			{														/*								*/
				rhs_p = &req_rhs[i];								/*								*/
				if( rhs_p->param == RY_TWR || 						/* 料金割引処理					*/
					rhs_p->param == RY_RWR || 						/* 時間割引処理					*/
					rhs_p->param == RY_SKC || 						/* ｻｰﾋﾞｽ券処理					*/
					rhs_p->param == RY_SKC_K ||						/* 拡張ｻｰﾋﾞｽ券処理				*/
					rhs_p->param == RY_SKC_K2 ||					/* 拡張2ｻｰﾋﾞｽ券処理				*/
					rhs_p->param == RY_SKC_K3 ||					/* 拡張3ｻｰﾋﾞｽ券処理				*/
					rhs_p->param == RY_KAK ||						/* 掛売券処理					*/
					rhs_p->param == RY_KAK_K ||						/* 拡張1掛売券					*/
					rhs_p->param == RY_KAK_K2 ||					/* 拡張2掛売券					*/
					rhs_p->param == RY_KAK_K3 ||					/* 拡張3掛売券					*/
					rhs_p->param == RY_SSS ||						/* 修正							*/
					rhs_p->param == RY_CSK ||						/* 精算中止ｻｰﾋﾞｽ券				*/
					rhs_p->param == RY_SNC			)				/* 店割引処理					*/
				{													/*								*/
					rhs_p->param = 0xff;							/* 要求ﾃﾞｰﾀFFｸﾘｱ				*/
				}													/*								*/
			}														/*								*/
			break;													/*								*/
		}															/*								*/
	}																/*								*/
	/*==============================================================================================*/
	/* 料金ﾊﾟﾗﾒｰﾀより領収証(個別精算情報含む)、精算ﾃﾞｰﾀの割引部分を全てここで作成する				*/
	/*==============================================================================================*/
	wk_dat = (ushort)(1+6*(rysyasu-1));
	wk_dat2 = (ushort)prm_tim( COM_PRM, S_SHA, (short)wk_dat );		/* wk_dat2 = 種別時間割引値		*/

// MH321800(S) 割引種別8000のセット方法を修正
//// MH321800(S) D.Inaba ICクレジット対応
////	if( (r_knd != 0) || (wk_dat2) )									/* 駐車券以外 or 種別時間割引設定あり	*/
//	// r_kind = 0であるが、クレジット精算時は割引情報に問合せ番号(8000)をセットする
//	if( (r_knd != 0) || (wk_dat2) || (r_knd == 0 && vl_now == V_CRE) )	/* 駐車券以外 or 種別時間割引設定あり	*/
//// MH321800(E) D.Inaba ICクレジット対応
	if( (r_knd != 0) || (wk_dat2) )									/* 駐車券以外 or 種別時間割引設定あり	*/
// MH321800(E) 割引種別8000のセット方法を修正
	{																/*								*/
		memset( PayData.DiscountData, 0, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );/* 精算ﾃﾞｰﾀ割引情報作成ｴﾘｱｸﾘｱ	*/
		NTNET_Data152_DiscDataClear();
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
		NTNET_Data152_DetailDataClear();
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
		/* 種別時間割引があれば WDicData の先頭にセットする */
		/* （割引額0円でもNT-NET電文で送信する）			*/
		if( wk_dat2 ){												/* 種別時間割引設定あり			*/
			PayData.DiscountData[0].ParkingNo = CPrmSS[S_SYS][1];	/* 駐車場№（基本駐車場№）		*/
			PayData.DiscountData[0].DiscSyu = NTNET_SYUBET;			/* 割引種別（種別時間割引）		*/
			PayData.DiscountData[0].DiscNo = rysyasu;				/* 割引区分（料金種別A～L：変更後）	*/
			PayData.DiscountData[0].DiscCount =	1;					/* 使用枚数						*/
// GG129000(S) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
// 			PayData.DiscountData[0].Discount = PayData.SyuWariRyo;	/* 割引額						*/
// GG129000(E) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 			PayData.DiscountData[0].uDiscData.common.MoveMode = 1;	/* 使用							*/
// 			PayData.DiscountData[0].uDiscData.common.DiscFlg = 0;	/* 割引済み(新規精算)			*/
			PayData.DiscountData[0].uDiscData.common.PrevDiscount = 0;			// 今回使用した前回精算までの割引金額
			PayData.DiscountData[0].uDiscData.common.PrevUsageDiscount  = 0;	// 前回精算までの使用済み割引金額
			PayData.DiscountData[0].uDiscData.common.PrevUsageDiscCount = 0;	// 前回精算までの使用済み枚数
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[0], NTNET_152_DPARKINGNO, 0);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[0], NTNET_152_DSYU, 0);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[0], NTNET_152_DNO, 0);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[0], NTNET_152_DCOUNT, 0);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[0], NTNET_152_DISCOUNT, 0);
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[0], NTNET_152_PREVDISCOUNT, 0);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[0], NTNET_152_PREVUSAGEDISCOUNT, 0);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[0], NTNET_152_PREVUSAGEDCOUNT, 0);
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
			++wrcnt;												/*								*/
			/* 以下は駐車券（他情報無し）以外時の処理なので、元の処理通りSkipする */
// MH321800(S) 割引種別8000のセット方法を修正
//// MH321800(S) D.Inaba ICクレジット対応
////			if( r_knd == 0 ){										/* 割引情報無し駐車券			*/
//			// r_kind = 0であるが、クレジット精算時は割引情報に問合せ番号(8000)をセットするため処理終了させない
//			if( r_knd == 0 && vl_now != V_CRE ){						/* 割引情報無し駐車券			*/
//// MH321800(E) D.Inaba ICクレジット対応
// MH810105 GG119202(S) 種別割引後のクレジット精算で精算データに割引情報がセットされない
//			if( r_knd == 0 ){										/* 割引情報無し駐車券			*/
			if( r_knd == 0 && vl_now != V_CRE ){					/* 割引情報無し駐車券（クレジット以外）*/
// MH810105 GG119202(E) 種別割引後のクレジット精算で精算データに割引情報がセットされない
// MH321800(E) 割引種別8000のセット方法を修正
				goto	ryo_cal_10;									/* if処理終了					*/
			}
		}
																	/*								*/
		for( cnt = 0; cnt < rhspnt; cnt++ )							/*								*/
		{															/*								*/
			rhs_p = &req_rhs[cnt];									/*								*/
			switch( rhs_p->param )									/* 料金計算要求ｺｰﾄﾞにより分岐	*/
			{														/*								*/
			  case RY_SKC:											/* 基本ｻｰﾋﾞｽ券					*/
			  case RY_SKC_K:										/* 拡張1ｻｰﾋﾞｽ券					*/
			  case RY_SKC_K2:										/* 拡張2ｻｰﾋﾞｽ券					*/
			  case RY_SKC_K3:										/* 拡張3ｻｰﾋﾞｽ券					*/
				wk_dat = (ushort)((rhs_p->data[0] &					/* 種類ｾｯﾄ						*/
									0xffff0000) >> 16);				/*								*/
				if( rhs_p->data[1] == 0 ){							/*								*/
					rhs_p->data[1] = 1;								/* 枚数が0なら1ｾｯﾄ				*/
				}													/*								*/
// MH810100(S) K.Onodera 2020/01/27 車番チケットレス(割引済み対応)
				DiscStatus = (uchar)(( rhs_p->data[0] >> 15 ) & 0x00000001 );	/* 割引ステータス	*/
// MH810100(E) K.Onodera 2020/01/27 車番チケットレス(割引済み対応)
																	/*								*/
				if(( wk_dat >= 1 )&&( wk_dat <= SVS_MAX )){			/* ｻｰﾋﾞｽ券A～O					*/
					// 精算ﾃﾞｰﾀ用									/*								*/
					wk_dat2 = (ushort)CPrmSS[S_SER][1+3*(wk_dat-1)];/*								*/
					if( wk_dat2 == 1 ){								/*								*/
						wk_dat2 = NTNET_SVS_T;						/* 割引種別=ｻｰﾋﾞｽ券(時間)		*/
					}else{											/*								*/
						wk_dat2 = NTNET_SVS_M;						/* 割引種別=ｻｰﾋﾞｽ券(料金)		*/
					}												/*								*/
																	/*								*/
					if( rhs_p->param == RY_SKC ){					/* 基本駐車場№					*/
						wk_dat3 = CPrmSS[S_SYS][1];					/* 駐車場№ｾｯﾄ					*/
					}else if( rhs_p->param == RY_SKC_K ){			/* 拡張1駐車場№				*/
						wk_dat3 = CPrmSS[S_SYS][2];					/* 駐車場№ｾｯﾄ					*/
					}else if( rhs_p->param == RY_SKC_K2 ){			/* 拡張2駐車場№				*/
						wk_dat3 = CPrmSS[S_SYS][3];					/* 駐車場№ｾｯﾄ					*/
					}else{											/* 拡張3駐車場№				*/
						wk_dat3 = CPrmSS[S_SYS][4];					/* 駐車場№ｾｯﾄ					*/
					}												/*								*/
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
					if ( rhs_p->data[1] >= 2000 ) {			// 前回精算までの割引
						//  前回精算までの割引の枚数
						wk_dat7 = (ushort)(rhs_p->data[1] - 2000);
					} else if ( rhs_p->data[1] >= 1000 ) {	// 今回使用した前回精算までの割引
						// 今回使用した前回精算までの割引の枚数
						wk_dat7 = (ushort)(rhs_p->data[1] - 1000);
					} else {								// 今回使用した割引
						// 今回使用した割引の枚数
						wk_dat7 = (ushort)(rhs_p->data[1]);
					}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
																	/*								*/
					for( i=0; i < WTIK_USEMAX; i++ ){				/*								*/
						if(( PayData.DiscountData[i].ParkingNo == wk_dat3 )&&	/* 駐車場№が等しい?*/
						   ( PayData.DiscountData[i].DiscSyu == wk_dat2 )&&		/* 割引種別が等しい?*/
						   ( PayData.DiscountData[i].DiscNo == wk_dat )&&		/* 割引区分が等しい?*/
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// // MH810100(S) 2020/06/15 車番チケットレス(#4229 【事業部レビュー指摘】前回半端な割引適応後、再精算で残りの割引が適応された場合、NT-NETで差額送信するが使用枚数は0としてほしい)
// 						   (rhs_p->data[1] < 1000 ) &&		// フラグが立っていない
// 						   (PayData.DiscountData[i].uDiscData.common.MoveMode < 1000) &&
// // MH810100(E) 2020/06/15 車番チケットレス(#4229 【事業部レビュー指摘】前回半端な割引適応後、再精算で残りの割引が適応された場合、NT-NETで差額送信するが使用枚数は0としてほしい)
// // MH810100(S) K.Onodera 2020/02/10 車番チケットレス(割引済み対応)
// //						   ( PayData.DiscountData[i].DiscInfo1 == (rhs_p->data[0] & 0x0000ffff) ))	/* 割引情報1(掛売り先)が等しい? */
// 						   ( PayData.DiscountData[i].DiscInfo1 == (rhs_p->data[0] & 0x00007fff) )&&	// 割引情報1(掛売り先)が等しい?
// 						   ( PayData.DiscountData[i].uDiscData.common.DiscFlg == DiscStatus ))		// 割引済み状態が等しい?
// // MH810100(S) K.Onodera 2020/02/10 車番チケットレス(割引済み対応)
						   ( PayData.DiscountData[i].DiscInfo1 == (rhs_p->data[0] & 0x00007fff) ))	// 割引情報1(掛売り先)が等しい?
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
						{											/*								*/
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 							PayData.DiscountData[i].DiscCount +=	/* 使用枚数						*/
// 										(ushort)rhs_p->data[1];		/*								*/
// 							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
// 							PayData.DiscountData[i].Discount += rhs_p->data[2];	/* 割引額			*/
// 							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
							if ( DiscStatus ) {	// 割引済み
								// 前回精算までの使用済み枚数
								PayData.DiscountData[i].uDiscData.common.PrevUsageDiscCount += (uchar)wk_dat7;
								NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_PREVUSAGEDCOUNT, i);

								// 前回精算までの使用済み割引金額
								PayData.DiscountData[i].uDiscData.common.PrevUsageDiscount += (ulong)rhs_p->data[2];
								NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_PREVUSAGEDISCOUNT, i);
							} else {			// 未割引
								if ( (rhs_p->data[1] < 2000) && (rhs_p->data[1] >= 1000) ) {	// 今回使用した前回精算までの割引
									// 今回使用した前回精算までの割引金額
									PayData.DiscountData[i].uDiscData.common.PrevDiscount += (ulong)rhs_p->data[2];
									NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_PREVDISCOUNT, i);
								}

								// 今回使用した枚数
								PayData.DiscountData[i].DiscCount += (uchar)wk_dat7;
								NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);

								// 今回使用した割引金額
								PayData.DiscountData[i].Discount += (ulong)rhs_p->data[2];
								NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
							}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
							if( wk_dat2 == NTNET_SVS_T ){			/* 時間割引?					*/
								PayData.DiscountData[i].uDiscData.common.DiscInfo2 +=	/*			*/
									prm_tim( COM_PRM,S_SER,(short)(2+3*(wk_dat-1)));	/* 割引情報2(割引時間数：分) */
								NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO2, i);
							}										/*								*/
							break;									/*								*/
						}											/*								*/
					}												/*								*/
					if( i >= WTIK_USEMAX && wrcnt < WTIK_USEMAX ){	/* 割引該当無し					*/
						PayData.DiscountData[wrcnt].ParkingNo = wk_dat3;/* 駐車場№					*/
						PayData.DiscountData[wrcnt].DiscSyu = wk_dat2;	/* 割引種別					*/
						PayData.DiscountData[wrcnt].DiscNo = wk_dat;	/* 割引区分(ｻｰﾋﾞｽ券A～C)	*/
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// // MH810100(S) 2020/06/15 車番チケットレス(#4229 【事業部レビュー指摘】前回半端な割引適応後、再精算で残りの割引が適応された場合、NT-NETで差額送信するが使用枚数は0としてほしい)
// //						PayData.DiscountData[wrcnt].DiscCount =		/* 使用枚数						*/
// //										(ushort)rhs_p->data[1];		/*								*/
// 						if( rhs_p->data[1] >= 2000){
// 							PayData.DiscountData[wrcnt].DiscCount =		/* 使用枚数						*/
// 											(ushort)rhs_p->data[1]-2000;		/*								*/
// 						}else if(rhs_p->data[1] < 1000){
// 							PayData.DiscountData[wrcnt].DiscCount =		/* 使用枚数						*/
// 											(ushort)rhs_p->data[1];		/*								*/
// 						}else{
// 							PayData.DiscountData[wrcnt].DiscCount =	0;	/* 使用枚数						*/
// 						}
// // MH810100(E) 2020/06/15 車番チケットレス(#4229 【事業部レビュー指摘】前回半端な割引適応後、再精算で残りの割引が適応された場合、NT-NETで差額送信するが使用枚数は0としてほしい)
// 						PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];	/* 割引額			*/
						if ( DiscStatus ) {	// 割引済み
							// 前回精算までの使用済み枚数
							PayData.DiscountData[wrcnt].uDiscData.common.PrevUsageDiscCount = (uchar)wk_dat7;

							// 前回精算までの使用済み割引金額
							PayData.DiscountData[wrcnt].uDiscData.common.PrevUsageDiscount = (ulong)rhs_p->data[2];
						} else {			// 未割引
							if ( (rhs_p->data[1] < 2000) && (rhs_p->data[1] >= 1000) ) {	// 今回使用した前回精算までの割引
								// 今回使用した前回精算までの割引金額
								PayData.DiscountData[wrcnt].uDiscData.common.PrevDiscount = (ulong)rhs_p->data[2];
							}

							// 今回使用した枚数
							PayData.DiscountData[wrcnt].DiscCount = (uchar)wk_dat7;

							// 今回使用した割引金額
							PayData.DiscountData[wrcnt].Discount = (ulong)rhs_p->data[2];
						}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
						PayData.DiscountData[wrcnt].DiscInfo1 =		/*								*/
// MH810100(S) K.Onodera 2020/01/27 車番チケットレス(割引済み対応)
//									rhs_p->data[0] & 0x0000ffff;	/* 割引情報1(掛売り先)			*/
									rhs_p->data[0] & 0x00007fff;	/* 割引情報1(掛売り先)			*/
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 						PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg =	/* 割引済み			*/
// 												(ushort)DiscStatus;	/* 								*/
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// MH810100(E) K.Onodera 2020/01/27 車番チケットレス(割引済み対応)
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DPARKINGNO, wrcnt);
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DSYU, wrcnt);
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DNO, wrcnt);
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DCOUNT, wrcnt);
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DISCOUNT, wrcnt);
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO1, wrcnt);
						if( wk_dat2 == NTNET_SVS_T ){				/* 時間割引?					*/
							PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 =	/*			*/
								prm_tim( COM_PRM,S_SER,(short)(2+3*(wk_dat-1)));/* 割引情報2(割引時間数) */
						}else{										/*								*/
							PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = 0L;	/* 割引情報2(未使用) */
						}											/*								*/
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO2, wrcnt);
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// // MH810100(S) 2020/06/15 車番チケットレス(#4229 【事業部レビュー指摘】前回半端な割引適応後、再精算で残りの割引が適応された場合、NT-NETで差額送信するが使用枚数は0としてほしい)
// //						PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;	/* 使用			*/
// 						if(rhs_p->data[1] == 1000){
// 							// 領収証でまとめる為フラグをつけておく
// 							PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1001;	/* 使用			*/
// 						}else if( rhs_p->data[1] >= 2000){
// 							PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 2001;	/* 使用			*/
// 						}else{
// 							PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;	/* 使用			*/
// 						}
// // MH810100(E) 2020/06/15 車番チケットレス(#4229 【事業部レビュー指摘】前回半端な割引適応後、再精算で残りの割引が適応された場合、NT-NETで差額送信するが使用枚数は0としてほしい)
// // MH810100(S) K.Onodera 2020/01/27 車番チケットレス(割引済み対応)
// //						PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;	/* 割引済み(新規精算) */
// // MH810100(E) K.Onodera 2020/01/27 車番チケットレス(割引済み対応)
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_PREVDISCOUNT, wrcnt);
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_PREVUSAGEDISCOUNT, wrcnt);
						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_PREVUSAGEDCOUNT, wrcnt);
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
						wrcnt++;									/*								*/
					}												/*								*/
				}													/*								*/
				break;												/*								*/
																	/*								*/
			  case RY_KAK:											/* 基本掛売券(店№割引)			*/
			  case RY_KAK_K:										/* 拡張1掛売券(店№割引)		*/
			  case RY_KAK_K2:										/* 拡張2掛売券(店№割引)		*/
			  case RY_KAK_K3:										/* 拡張3掛売券(店№割引)		*/
				if( rhs_p->data[1] == 0 ){							/*								*/
					rhs_p->data[1] = 1;								/* 枚数が0なら1ｾｯﾄ				*/
				}													/*								*/
// MH810100(S) 2020/09/09 #4755 店割引の割引種類対応（【連動評価指摘事項】NT-NET精算データに割引種別2,102の割引情報の値が格納されない(02-0045)）
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算処理の変更)
//				wk_dat5 = (ushort)((rhs_p->data[0] &					/* 種類ｾｯﾄ						*/
//									0xffff0000) >> 16);				/*								*/
				wk_dat5 = (ushort)((rhs_p->data[0] >> 16) & 0x000000ff);	/* 店割引種類セット			*/
				wk_dat6 = (ushort)((rhs_p->data[0] >> 24) & 0x000000ff);	/* 多店舗割引種別セット		*/
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算処理の変更)
// MH810100(E) 2020/09/09 #4755 店割引の割引種類対応（【連動評価指摘事項】NT-NET精算データに割引種別2,102の割引情報の値が格納されない(02-0045)）
				// 精算ﾃﾞｰﾀ用										/*								*/
// MH810100(S) K.Onodera 2020/01/27 車番チケットレス(割引済み対応)
//				wk_dat = (ushort)rhs_p->data[0];					/* 店№							*/
				wk_dat = (ushort)( rhs_p->data[0] & 0x00007fff );	/* 店№							*/
				DiscStatus = (uchar)(( rhs_p->data[0] >> 15 ) & 0x00000001 );	/* 割引ステータス	*/
// MH810100(E) K.Onodera 2020/01/27 車番チケットレス(割引済み対応)
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算処理の変更)
//				wk_dat2 = (ushort)CPrmSS[S_STO][1+3*(wk_dat-1)];	/*								*/
//				if( wk_dat2 == 1 ){									/*								*/
//					wk_dat2 = NTNET_KAK_T;							/* 割引種別=店割引(時間)		*/
//				}else{												/*								*/
//					wk_dat2 = NTNET_KAK_M;							/* 割引種別=店割引(料金)		*/
//				}													/*								*/
				if ( wk_dat6 ) {									/* 多店舗割引					*/
					wk_adr = (short)(wk_dat6 * 2);
					wk_ses = (short)S_TAT;
					if ( prm_get(COM_PRM, wk_ses, wk_adr, 1, 1) == 1 ) {
						wk_dat2 = NTNET_TKAK_T;
					} else {
						wk_dat2 = NTNET_TKAK_M;
					}
				} else {											/* 店割引/買物割引				*/
					wk_adr = (short)((wk_dat * 3) - 2);
					wk_ses = (short)S_STO;
					if ( prm_get(COM_PRM, wk_ses, wk_adr, 1, 1) == 1 ) {
						wk_dat2 = NTNET_KAK_T;
					} else {
						wk_dat2 = NTNET_KAK_M;
					}
				}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算処理の変更)
																	/*								*/
				if( rhs_p->param == RY_KAK ){						/* 基本駐車場№					*/
					wk_dat3 = CPrmSS[S_SYS][1];						/* 駐車場№ｾｯﾄ					*/
				}else if( rhs_p->param == RY_KAK_K ){				/* 拡張1駐車場№				*/
					wk_dat3 = CPrmSS[S_SYS][2];						/* 駐車場№ｾｯﾄ					*/
				}else if( rhs_p->param == RY_KAK_K2 ){				/* 拡張2駐車場№				*/
					wk_dat3 = CPrmSS[S_SYS][3];						/* 駐車場№ｾｯﾄ					*/
				}else{												/* 拡張3駐車場№				*/
					wk_dat3 = CPrmSS[S_SYS][4];						/* 駐車場№ｾｯﾄ					*/
				}													/*								*/
// MH810100(S) K.Onodera 2020/02/27 車番チケットレス(買物割引対応)
				wk_dat4 = 0;										/* 割引情報1クリア				*/
				CheckShoppingDisc( wk_dat3, &wk_dat2, wk_dat, &wk_dat4 );	/* 買物割引チェック		*/
// MH810100(E) K.Onodera 2020/02/27 車番チケットレス(買物割引対応)
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
				if ( rhs_p->data[1] >= 2000 ) {			// 前回精算までの割引
					//  前回精算までの割引の枚数
					wk_dat7 = (ushort)(rhs_p->data[1] - 2000);
				} else if ( rhs_p->data[1] >= 1000 ) {	// 今回使用した前回精算までの割引
					// 今回使用した前回精算までの割引の枚数
					wk_dat7 = (ushort)(rhs_p->data[1] - 1000);
				} else {								// 今回使用した割引
					// 今回使用した割引の枚数
					wk_dat7 = (ushort)(rhs_p->data[1]);
				}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
																	/*								*/
				for( i=0; i < WTIK_USEMAX; i++ ){					/*								*/
// GG124100(S) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 					if(( PayData.DiscountData[i].ParkingNo == wk_dat3 )&&	/* 駐車場№が等しい?	*/
// 					   ( PayData.DiscountData[i].DiscSyu == wk_dat2 )&&	/* 割引種別が等しい?		*/
// // MH810100(S) 2020/06/15 車番チケットレス(#4229 【事業部レビュー指摘】前回半端な割引適応後、再精算で残りの割引が適応された場合、NT-NETで差額送信するが使用枚数は0としてほしい)
// 					   (rhs_p->data[1] < 1000 ) &&		// フラグが立っていない
// 					   (PayData.DiscountData[i].uDiscData.common.MoveMode < 1000) &&
// // MH810100(E) 2020/06/15 車番チケットレス(#4229 【事業部レビュー指摘】前回半端な割引適応後、再精算で残りの割引が適応された場合、NT-NETで差額送信するが使用枚数は0としてほしい)
// // MH810100(S) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
// //					   ( PayData.DiscountData[i].DiscNo == wk_dat ))	/* 割引区分が等しい?		*/
// 					   ( PayData.DiscountData[i].DiscNo == wk_dat )&&	/* 割引区分が等しい?		*/
// 					   ( PayData.DiscountData[i].uDiscData.common.DiscFlg == DiscStatus ))	/* 割引済み状態が等しい?*/
// // MH810100(E) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
// 					{												/*								*/
// 						PayData.DiscountData[i].DiscCount +=		/* 使用枚数						*/
// 									(ushort)rhs_p->data[1];			/*								*/
// 						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
// 						PayData.DiscountData[i].Discount += rhs_p->data[2];	/* 割引額				*/
// 						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
					if (   (PayData.DiscountData[i].ParkingNo  == wk_dat3)					// 駐車場№
						&& (PayData.DiscountData[i].DiscSyu    == wk_dat2)					// 割引種別
						&& (PayData.DiscountData[i].DiscNo     == wk_dat )					// 割引区分
						&& ((((wk_dat2 == NTNET_KAK_M) || (wk_dat2 == NTNET_KAK_T)) &&		// 店割引
						     (PayData.DiscountData[i].DiscInfo1 == wk_dat5))				// 割引情報1(店割引種類)
						||  (((wk_dat2 == NTNET_TKAK_M) || (wk_dat2 == NTNET_TKAK_T)) &&	// 多店舗割引
						     (PayData.DiscountData[i].DiscInfo1 == wk_dat6)))				// 割引情報1(多店舗割引種別)
						&& (wk_dat2 != NTNET_SHOP_DISC_AMT)									// 買物割引(金額)ではない
						&& (wk_dat2 != NTNET_SHOP_DISC_TIME)								// 買物割引(時間)ではない
					) {
						if ( DiscStatus ) {	// 割引済み
							// 前回精算までの使用済み枚数
							PayData.DiscountData[i].uDiscData.common.PrevUsageDiscCount += (uchar)wk_dat7;
							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_PREVUSAGEDCOUNT, i);

							// 前回精算までの使用済み割引金額
							PayData.DiscountData[i].uDiscData.common.PrevUsageDiscount += (ulong)rhs_p->data[2];
							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_PREVUSAGEDISCOUNT, i);
						} else {			// 未割引
							if ( (rhs_p->data[1] < 2000) && (rhs_p->data[1] >= 1000) ) {	// 今回使用した前回精算までの割引
								// 今回使用した前回精算までの割引金額
								PayData.DiscountData[i].uDiscData.common.PrevDiscount += (ulong)rhs_p->data[2];
								NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_PREVDISCOUNT, i);
							}

							// 今回使用した枚数
							PayData.DiscountData[i].DiscCount += (uchar)wk_dat7;
							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);

							// 今回使用した割引金額
							PayData.DiscountData[i].Discount += (ulong)rhs_p->data[2];
							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
						}
// GG124100(E) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// MH810100(S) K.Onodera 2020/02/28 車番チケットレス(買物割引対応)
//						if( wk_dat2 == NTNET_KAK_T ){				/* 時間割引?					*/
						if(( wk_dat2 == NTNET_KAK_T ) ||			/* 時間割引?					*/
// GG124100(S) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// // MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算処理の変更)
// 						   ( wk_dat2 == NTNET_TKAK_T ) ||		 	/*								*/
// // MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算処理の変更)
// 						   ( wk_dat2 == NTNET_SHOP_DISC_TIME )){	/*								*/
						   ( wk_dat2 == NTNET_TKAK_T )) {
// GG124100(E) R.Endo 2022/08/09 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// MH810100(E) K.Onodera 2020/02/28 車番チケットレス(買物割引対応)
							PayData.DiscountData[i].uDiscData.common.DiscInfo2 +=	/*				*/
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算処理の変更)
//								prm_tim( COM_PRM,S_STO,(short)(2+3*(wk_dat-1)));	/* 割引情報2(割引時間数：分) */
								prm_tim(COM_PRM, wk_ses, (wk_adr + 1));	/* 割引情報2(割引時間数：分) */
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算処理の変更)
							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO2, i);
						}											/*								*/
						break;										/*								*/
					}												/*								*/
				}													/*								*/
				if( i >= WTIK_USEMAX && wrcnt < WTIK_USEMAX ){		/* 割引該当無し					*/
					PayData.DiscountData[wrcnt].ParkingNo = wk_dat3;/* 駐車場№						*/
					PayData.DiscountData[wrcnt].DiscSyu = wk_dat2;	/* 割引種別						*/
					PayData.DiscountData[wrcnt].DiscNo = wk_dat;	/* 割引区分(店№)				*/
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// // MH810100(S) 2020/06/15 車番チケットレス(#4229 【事業部レビュー指摘】前回半端な割引適応後、再精算で残りの割引が適応された場合、NT-NETで差額送信するが使用枚数は0としてほしい)
// //					PayData.DiscountData[wrcnt].DiscCount =			/* 使用枚数						*/
// //										(ushort)rhs_p->data[1];		/*								*/
// 					if( rhs_p->data[1] >= 2000){
// 						PayData.DiscountData[wrcnt].DiscCount =		/* 使用枚数						*/
// 										(ushort)rhs_p->data[1]-2000;		/*								*/
// 					}else if(rhs_p->data[1] < 1000){
// 						PayData.DiscountData[wrcnt].DiscCount =		/* 使用枚数						*/
// 										(ushort)rhs_p->data[1];		/*								*/
// 					}else{
// 						PayData.DiscountData[wrcnt].DiscCount =	0;	/* 使用枚数						*/
// 					}
// // MH810100(E) 2020/06/15 車番チケットレス(#4229 【事業部レビュー指摘】前回半端な割引適応後、再精算で残りの割引が適応された場合、NT-NETで差額送信するが使用枚数は0としてほしい)
// 					PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];	/* 割引額				*/
					if ( DiscStatus ) {	// 割引済み
						// 前回精算までの使用済み枚数
						PayData.DiscountData[wrcnt].uDiscData.common.PrevUsageDiscCount = (uchar)wk_dat7;

						// 前回精算までの使用済み割引金額
						PayData.DiscountData[wrcnt].uDiscData.common.PrevUsageDiscount = (ulong)rhs_p->data[2];
					} else {			// 未割引
						if ( (rhs_p->data[1] < 2000) && (rhs_p->data[1] >= 1000) ) {	// 今回使用した前回精算までの割引
							// 今回使用した前回精算までの割引金額
							PayData.DiscountData[wrcnt].uDiscData.common.PrevDiscount = (ulong)rhs_p->data[2];
						}

						// 今回使用した枚数
						PayData.DiscountData[wrcnt].DiscCount = (uchar)wk_dat7;

						// 今回使用した割引金額
						PayData.DiscountData[wrcnt].Discount = (ulong)rhs_p->data[2];
					}
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// MH810100(S) 2020/09/09 #4755 店割引の割引種類対応（【連動評価指摘事項】NT-NET精算データに割引種別2,102の割引情報の値が格納されない(02-0045)）
//// MH810100(S) K.Onodera 2020/02/27 車番チケットレス(買物割引対応)
////					PayData.DiscountData[wrcnt].DiscInfo1 = 0L;		/* 割引情報1(未使用)			*/
//					PayData.DiscountData[wrcnt].DiscInfo1 = (ulong)wk_dat4;	/* 割引情報1			*/
//// MH810100(E) K.Onodera 2020/02/27 車番チケットレス(買物割引対応)
					if( wk_dat2 == NTNET_KAK_M || wk_dat2 == NTNET_KAK_T){
						PayData.DiscountData[wrcnt].DiscInfo1 = (ulong)wk_dat5;	/* 割引情報1			*/
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算処理の変更)
					} else if ( (wk_dat2 == NTNET_TKAK_M) || (wk_dat2 == NTNET_TKAK_T) ) {
						PayData.DiscountData[wrcnt].DiscInfo1 = (ulong)wk_dat6;	/* 割引情報1			*/
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算処理の変更)
					}else{
						PayData.DiscountData[wrcnt].DiscInfo1 = (ulong)wk_dat4;	/* 割引情報1			*/
					}
// MH810100(E) 2020/09/09 #4755 店割引の割引種類対応（【連動評価指摘事項】NT-NET精算データに割引種別2,102の割引情報の値が格納されない(02-0045)）
// MH810100(S) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 					PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg =	/* 割引済み				*/
// 												(ushort)DiscStatus;	/* 								*/
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// MH810100(E) K.Onodera 2020/02/04 車番チケットレス(割引済み対応)
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DPARKINGNO, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DSYU, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DNO, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DCOUNT, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DISCOUNT, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO1, wrcnt);
// MH810100(S) K.Onodera 2020/02/27 車番チケットレス(買物割引対応)
//					if( wk_dat2 == NTNET_KAK_T ){					/* 時間割引?					*/
					if(( wk_dat2 == NTNET_KAK_T ) ||				/* 時間割引?					*/
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算処理の変更)
					   ( wk_dat2 == NTNET_TKAK_T ) ||			 	/*								*/
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算処理の変更)
					   ( wk_dat2 == NTNET_SHOP_DISC_TIME )){		/*								*/
// MH810100(E) K.Onodera 2020/02/27 車番チケットレス(買物割引対応)
						PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 =	/*				*/
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算処理の変更)
//							prm_tim( COM_PRM,S_STO,(short)(2+3*(wk_dat-1)));/* 割引情報2(割引時間数)*/
							prm_tim(COM_PRM, wk_ses, (wk_adr + 1));	/* 割引情報2(割引時間数：分)	*/
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算処理の変更)
					}else{											/*								*/
						PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = 0L;/* 割引情報2(未使用) */
					}												/*								*/
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO2, wrcnt);
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// // MH810100(S) 2020/06/15 車番チケットレス(#4229 【事業部レビュー指摘】前回半端な割引適応後、再精算で残りの割引が適応された場合、NT-NETで差額送信するが使用枚数は0としてほしい)
// //					PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;	/* 使用				*/
// 					if(rhs_p->data[1] == 1000){
// 						// 領収証でまとめる為フラグをつけておく
// 						PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1001;	/* 使用			*/
// 					}else if( rhs_p->data[1] >= 2000){
// 						PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 2001;	/* 使用			*/
// 					}else{
// 						PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;	/* 使用			*/
// 					}
// // MH810100(E) 2020/06/15 車番チケットレス(#4229 【事業部レビュー指摘】前回半端な割引適応後、再精算で残りの割引が適応された場合、NT-NETで差額送信するが使用枚数は0としてほしい)
// // MH810100(S) K.Onodera 2020/02/27 車番チケットレス(割引済み対応)
// //					PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;	/* 割引済み(新規精算)*/
// // MH810100(E) K.Onodera 2020/02/27 車番チケットレス(割引済み対応)
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_PREVDISCOUNT, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_PREVUSAGEDISCOUNT, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_PREVUSAGEDCOUNT, wrcnt);
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
					wrcnt++;										/*								*/
				}													/*								*/
				break;												/*								*/
																	/*								*/
			  case RY_PCO:											/* 基本ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ				*/
			  case RY_PCO_K:										/* 拡張1ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ			*/
			  case RY_PCO_K2:										/* 拡張2ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ			*/
			  case RY_PCO_K3:										/* 拡張3ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ			*/
				/* 空きｴﾘｱ検索 */
				for( i=0; (i < WTIK_USEMAX) &&								/* データがある間				*/
						  (0 != PayData.DiscountData[i].ParkingNo); i++ ){
					;
				}

				wk_dat2 = NTNET_PRI_W;										/* 割引種別=割引ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ	*/
				if( rhs_p->param == RY_PCO ){								/* 基本駐車場№					*/
					wk_dat3 = CPrmSS[S_SYS][1];								/* 駐車場№ｾｯﾄ					*/
				}else if( rhs_p->param == RY_PCO_K ){						/* 拡張1駐車場№				*/
					wk_dat3 = CPrmSS[S_SYS][2];								/* 駐車場№ｾｯﾄ					*/
				}else if( rhs_p->param == RY_PCO_K2 ){						/* 拡張2駐車場№				*/
					wk_dat3 = CPrmSS[S_SYS][3];								/* 駐車場№ｾｯﾄ					*/
				}else{														/* 拡張3駐車場№				*/
					wk_dat3 = CPrmSS[S_SYS][4];								/* 駐車場№ｾｯﾄ					*/
				}															/*								*/

				/* 新規登録 */
				if( i < WTIK_USEMAX ){										/* ｾｯﾄｴﾘｱに空きがある場合 		*/
					PayData.DiscountData[i].ParkingNo = wk_dat3;			/* 駐車場№						*/
					PayData.DiscountData[i].DiscSyu = wk_dat2;				/* 割引種別						*/
					PayData.DiscountData[i].DiscNo =						/* ｶｰﾄﾞNo.						*/
						(ushort)(PPC_Data_Detail.ppc_data_detail[pricnt].ppc_id >> 16);	/*					*/
					PayData.DiscountData[i].DiscCount =						/*								*/
						(ushort)(PPC_Data_Detail.ppc_data_detail[pricnt].ppc_id & 0x0000ffff);	/*			*/
					PayData.DiscountData[i].Discount = rhs_p->data[2];		/* 割引（利用）料金 			*/
					PayData.DiscountData[i].DiscInfo1 =						/* 利用後ｶｰﾄﾞ残額				*/
						PPC_Data_Detail.ppc_data_detail[pricnt].pay_after;	/*								*/
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DPARKINGNO, i);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DSYU, i);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DNO, i);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO1, i);
					PayData.DiscountData[i].uDiscData.ppc_data.pay_befor =	/* 支払前残額					*/
						PPC_Data_Detail.ppc_data_detail[pricnt].pay_befor;	/*								*/
					PayData.DiscountData[i].uDiscData.ppc_data.kigen_year =	/* 有効期限年					*/
						PPC_Data_Detail.ppc_data_detail[pricnt].kigen_year;	/*								*/
					PayData.DiscountData[i].uDiscData.ppc_data.kigen_mon =	/* 有効期限月					*/
						PPC_Data_Detail.ppc_data_detail[pricnt].kigen_mon;	/*								*/
					PayData.DiscountData[i].uDiscData.ppc_data.kigen_day =	/* 有効期限日					*/
						PPC_Data_Detail.ppc_data_detail[pricnt].kigen_day;	/*								*/
					pricnt++;												/*								*/
					wrcnt++;												/*								*/
				}															/*								*/
				break;												/*								*/

// MH321800(S) 割引種別8000のセット方法を修正
//			case RY_EMY:											/* 電子マネー処理				*/
//				/* 新規登録（Suica・Edyは1精算で1枚のみ使用可） */
//				/* 新規登録（Suicaは1精算で1枚のみ使用可） */
//				wk_media_Type = Ope_Disp_Media_Getsub(1);					// 電子マネー媒体種別 取得
//				if(wk_media_Type == OPE_DISP_MEDIA_TYPE_SUICA) {			// Suica
//					wk_DiscSyu1 = NTNET_SUICA_1;							/* 割引種別：Suicaｶｰﾄﾞ番号		*/
//					wk_DiscSyu2 = NTNET_SUICA_2;							/* 割引種別：Suica支払額、残額	*/
//				}
//				else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_PASMO) {		// PASMO
//					wk_DiscSyu1 = NTNET_PASMO_1;							/* 割引種別：PASMOｶｰﾄﾞ番号		*/
//					wk_DiscSyu2 = NTNET_PASMO_2;							/* 割引種別：PASMO支払額、残額	*/
//				}
//				else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_ICOCA) {		// ICOCA
//					wk_DiscSyu1 = NTNET_ICOCA_1;							/* 割引種別：ICOCAｶｰﾄﾞ番号		*/
//					wk_DiscSyu2 = NTNET_ICOCA_2;							/* 割引種別：ICOCA支払額、残額	*/
//				}
//				else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_ICCARD) {		// IC-Card
//					wk_DiscSyu1 = NTNET_ICCARD_1;							/* 割引種別：ICOCAｶｰﾄﾞ番号		*/
//					wk_DiscSyu2 = NTNET_ICCARD_2;							/* 割引種別：ICOCA支払額、残額	*/
//				}
//// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
////				else if(wk_media_Type == OPE_DISP_MEDIA_TYPE_EDY) {			// Edy
////					wk_DiscSyu1 = NTNET_EDY_1;								/* 割引種別：Edyｶｰﾄﾞ番号		*/
////					wk_DiscSyu2 = NTNET_EDY_2;								/* 割引種別：Edy支払額、残額	*/
////				}
//// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//// MH321800(S) hosoda ICクレジット対応
//				else if (wk_media_Type == OPE_DISP_MEDIA_TYPE_EC) {			// 決済リーダ
//					switch (PayData.Electron_data.Ec.e_pay_kind) {			// 決済種別から振り分け
//					case	EC_EDY_USED:
//						wk_DiscSyu1 = NTNET_EDY_1;							/* 割引種別：Edyｶｰﾄﾞ番号		*/
//						wk_DiscSyu2 = NTNET_EDY_2;							/* 割引種別：Edy支払額、残額	*/
//						break;
//					case	EC_NANACO_USED:
//						wk_DiscSyu1 = NTNET_NANACO_1;						/* 割引種別：nanacoｶｰﾄﾞ番号		*/
//						wk_DiscSyu2 = NTNET_NANACO_2;						/* 割引種別：nanaco支払額、残額	*/
//						break;
//					case	EC_WAON_USED:
//						wk_DiscSyu1 = NTNET_WAON_1;							/* 割引種別：WAONｶｰﾄﾞ番号		*/
//						wk_DiscSyu2 = NTNET_WAON_2;							/* 割引種別：WAON支払額、残額	*/
//						break;
//					case	EC_SAPICA_USED:
//						wk_DiscSyu1 = NTNET_SAPICA_1;						/* 割引種別：SAPICAｶｰﾄﾞ番号		*/
//						wk_DiscSyu2 = NTNET_SAPICA_2;						/* 割引種別：SAPICA支払額、残額	*/
//						break;
//					case	EC_KOUTSUU_USED:
//						// 交通系ICカードはSuicaのIDを使用する
//						wk_DiscSyu1 = NTNET_SUICA_1;						/* 割引種別：Suicaｶｰﾄﾞ番号		*/
//						wk_DiscSyu2 = NTNET_SUICA_2;						/* 割引種別：Suica支払額、残額	*/
//						break;
//					case	EC_ID_USED:
//						wk_DiscSyu1 = NTNET_ID_1;							/* 割引種別：iDｶｰﾄﾞ番号		*/
//						wk_DiscSyu2 = NTNET_ID_2;							/* 割引種別：iD支払額、残額	*/
//						break;
//					case	EC_QUIC_PAY_USED:
//						wk_DiscSyu1 = NTNET_QUICPAY_1;						/* 割引種別：QUICPayｶｰﾄﾞ番号		*/
//						wk_DiscSyu2 = NTNET_QUICPAY_2;						/* 割引種別：QUICPay支払額、残額	*/
//						break;
//					default	:
//					// ありえない
//						break;
//					}
//				}
//// MH321800(E) hosoda ICクレジット対応
//				for( i=0; (i < WTIK_USEMAX) && (0 != PayData.DiscountData[i].ParkingNo); i++ ){
//					;														/* 格納場所[位置]を探す			*/
//				}
//// MH321800(S) D.Inaba ICクレジット対応
//				if( wk_media_Type == OPE_DISP_MEDIA_TYPE_EC ){
//					if( i < WTIK_USEMAX-2 ){									/* ｾｯﾄｴﾘｱに３つ分の空きがある場合 */
//						memset(&wk_DicData, 0x20, sizeof(wk_DicData));
//						wk_DicData.ParkingNo = CPrmSS[S_SYS][1];				/* 駐車場№：未使用				*/
//						wk_DicData.DiscSyu = wk_DiscSyu1;						/* 割引種別：Suicaｶｰﾄﾞ番号		*/
//						if( PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement != 1 ){ // みなし決済ではない？
//							memcpy(wk_DicData.CardNo,
//									PayData.Electron_data.Ec.Card_ID, sizeof(wk_DicData.CardNo));
//						}
//
//						memcpy(&PayData.DiscountData[i], &wk_DicData, sizeof(wk_DicData));
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DPARKINGNO, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DSYU, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DNO, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO1, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO2, i);
//
//						i++;
//						PayData.DiscountData[i].ParkingNo = CPrmSS[S_SYS][1];	/* 駐車場№：未使用				*/
//						PayData.DiscountData[i].DiscSyu = wk_DiscSyu2;			/* 割引種別：Suica支払額、残額	*/
//						PayData.DiscountData[i].DiscNo = 0;						/* 割引区分：支払い（固定）		*/
//						PayData.DiscountData[i].DiscCount = 1;					/* 使用枚数：1枚（固定）		*/
//						PayData.DiscountData[i].Discount = PayData.Electron_data.Ec.pay_ryo;		/* 割引額：支払金額 */
//						PayData.DiscountData[i].DiscInfo1 = PayData.Electron_data.Ec.pay_after;		/* 割引情報１：残額 */
//						/* 割引情報２ */
//						switch (PayData.Electron_data.Ec.e_pay_kind) {		// 決済種別から振り分け
//							case	EC_EDY_USED:
//								PayData.DiscountData[i].uDiscData.common.DiscInfo2 = PayData.Electron_data.Ec.Brand.Edy.DealNo;			/* Edy取引通番 */
//								break;
//							case	EC_NANACO_USED:
//								PayData.DiscountData[i].uDiscData.common.DiscInfo2 = PayData.Electron_data.Ec.Brand.Nanaco.DealNo;		/* nanaco取引通番 */
//								break;
//							case	EC_WAON_USED:
//								PayData.DiscountData[i].uDiscData.common.DiscInfo2 = PayData.Electron_data.Ec.Brand.Waon.point;			/* WAON今回付与ポイント */
//								break;
//							case	EC_SAPICA_USED:
//								PayData.DiscountData[i].uDiscData.common.DiscInfo2 = PayData.Electron_data.Ec.Brand.Sapica.Details_ID;	/* SAPICA一件明細ID */
//								break;
//							case	EC_KOUTSUU_USED:
//								// セットする項目なし
//								break;
//							case	EC_ID_USED:
//								// TODO:詳細決まり次第
//								break;
//							case	EC_QUIC_PAY_USED:
//								// TODO:詳細決まり次第
//								break;
//							default	:
//							// ありえない
//								break;
//						}
//						PayData.DiscountData[i].uDiscData.common.MoveMode = 0;	/* 使用／返却：返却	（固定）	*/
//						PayData.DiscountData[i].uDiscData.common.DiscFlg = 0;	/* 割引済み：新規精算（固定)	*/
//
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DPARKINGNO, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DSYU, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DNO, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO1, i);
//
//						i++;
//						memset(&wk_DicData, 0, sizeof(wk_DicData));
//						wk_DicData.ParkingNo = CPrmSS[S_SYS][1];				/* 駐車場№：未使用				*/
//						wk_DicData.DiscSyu = NTNET_INQUIRY_NUM;					/* 割引種別：問合せ番号	       	*/
//						memcpy( wk_DicData.CardNo, PayData.Electron_data.Ec.inquiry_num, sizeof(wk_DicData.CardNo) );// 問合せ番号
//
//						memcpy(&PayData.DiscountData[i], &wk_DicData, sizeof(wk_DicData));
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DPARKINGNO, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DSYU, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DNO, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO1, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO2, i);
//					}
//				}
//				else {
//// MH321800(E) D.Inaba ICクレジット対応
//				if( i < WTIK_USEMAX-1 ){									/* ｾｯﾄｴﾘｱに２つ分の空きがある場合 */
//					memset(&wk_DicData, 0, sizeof(wk_DicData));
//					wk_DicData.ParkingNo = CPrmSS[S_SYS][1];				/* 駐車場№：未使用				*/
//					wk_DicData.DiscSyu = wk_DiscSyu1;						/* 割引種別：Suicaｶｰﾄﾞ番号		*/
//
//					memset(wk_DicData.CardNo, 0x20, sizeof(wk_DicData.CardNo));
//// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
////					if( wk_media_Type == OPE_DISP_MEDIA_TYPE_EDY ){			/* 割引種別：Edyｶｰﾄﾞ番号		*/
////						BCDtoASCII( PayData.Electron_data.Edy.Card_ID, wk_DicData.CardNo, 8 );
////					}else{
//					{
//// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//						memcpy( wk_DicData.CardNo,
//								PayData.Electron_data.Suica.Card_ID,
//								sizeof(PayData.Electron_data.Suica.Card_ID));
//					}
//					memcpy(&PayData.DiscountData[i], &wk_DicData, sizeof(wk_DicData));
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DPARKINGNO, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DSYU, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DNO, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO1, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO2, i);
//					i++;
//					PayData.DiscountData[i].ParkingNo = CPrmSS[S_SYS][1];	/* 駐車場№：未使用				*/
//					PayData.DiscountData[i].DiscSyu = wk_DiscSyu2;			/* 割引種別：Suica支払額、残額	*/
//					PayData.DiscountData[i].DiscNo = 0;						/* 割引区分：支払い（固定）		*/
//					PayData.DiscountData[i].DiscCount = 1;					/* 使用枚数：1枚（固定）		*/
//					PayData.DiscountData[i].Discount = PayData.Electron_data.Suica.pay_ryo;		/* 割引額：支払金額 */
//					PayData.DiscountData[i].DiscInfo1 = PayData.Electron_data.Suica.pay_after;	/* 割引情報１：残額 */
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DPARKINGNO, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DSYU, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DNO, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO1, i);
//// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
////					if( wk_media_Type == OPE_DISP_MEDIA_TYPE_EDY ){
////						PayData.DiscountData[i].uDiscData.common.DiscInfo2 = PayData.Electron_data.Edy.deal_no;	/* 割引情報２：Edy取引通番 */
////						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO2, i);
////					}
//// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//					PayData.DiscountData[i].uDiscData.common.MoveMode = 0;	/* 使用／返却：返却	（固定）	*/
//					PayData.DiscountData[i].uDiscData.common.DiscFlg = 0;	/* 割引済み：新規精算（固定)	*/
//				}
//// MH321800(S) D.Inaba ICクレジット対応
//				}
//// MH321800(E) D.Inaba ICクレジット対応
//				break;
// MH321800(E) 割引種別8000のセット方法を修正

			  case RY_FRE:											/* 基本回数券					*/
			  case RY_FRE_K:										/* 拡張1回数券					*/
			  case RY_FRE_K2:										/* 拡張2回数券					*/
			  case RY_FRE_K3:										/* 拡張3回数券					*/


				/** 内部ﾃﾞｰﾀへの登録 **/

				if( 0xff == (ushort)rhs_p->data[1] )				/* 全額割引回数券 				*/
					wk_dat = 99;
				else
					wk_dat = 0;


				/** 通信電文用ﾃﾞｰﾀへの登録 **/

				wk_dat2 = NTNET_FRE;								/* 割引種別=回数券				*/
																	/*								*/
				/* 駐車場No get */
				if( rhs_p->param == RY_FRE_K )						/* 拡張1駐車場№				*/
					wk_dat3 = CPrmSS[S_SYS][2];						/* 駐車場№ｾｯﾄ					*/
				else if( rhs_p->param == RY_FRE_K2 )				/* 拡張2駐車場№				*/
					wk_dat3 = CPrmSS[S_SYS][3];						/* 駐車場№ｾｯﾄ					*/
				else if( rhs_p->param == RY_FRE_K3 )				/* 拡張3駐車場№				*/
					wk_dat3 = CPrmSS[S_SYS][4];						/* 駐車場№ｾｯﾄ					*/
				else												/* 基本駐車場№					*/
					wk_dat3 = CPrmSS[S_SYS][1];						/* 駐車場№ｾｯﾄ					*/
																	/*								*/
				/* 既存ｴﾘｱへの加算ﾁｪｯｸ */
				for( i=0; i < WTIK_USEMAX; i++ ){					/*								*/

					if(( PayData.DiscountData[i].ParkingNo == wk_dat3 )&&	/* 駐車場№が等しい?	*/
					   ( PayData.DiscountData[i].DiscSyu == wk_dat2 ))		/* 割引種別が等しい?	*/
					{												/*								*/
						if( (99 != PayData.DiscountData[i].DiscNo) &&
							(99 != wk_dat) )						/* 料金割引回数券同士			*/
						{
							PayData.DiscountData[i].DiscNo +=
							(uchar)(rhs_p->data[0] & 0x0000ffff);	/* 使用度数 加算				*/

							PayData.DiscountData[i].DiscCount += (uchar)1;		/* 使用枚数			*/
							PayData.DiscountData[i].Discount += rhs_p->data[2];	/* 割引額			*/
							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DNO, i);
							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
							NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
							break;									/*								*/
						}
					}												/*								*/
				}													/*								*/

				/* 新規登録 */
				if( i >= WTIK_USEMAX && wrcnt < WTIK_USEMAX ){		/* 割引該当無し					*/
					PayData.DiscountData[wrcnt].ParkingNo = wk_dat3;/* 駐車場№						*/
					PayData.DiscountData[wrcnt].DiscSyu = wk_dat2;	/* 割引種別						*/
					if( 99 == wk_dat )								/*								*/
						PayData.DiscountData[wrcnt].DiscNo = (ushort)wk_dat;	/* 割引区分 (使用度数、99=全額割引) */
					else											/*								*/
						PayData.DiscountData[wrcnt].DiscNo =		/*								*/
							(uchar)(rhs_p->data[0] & 0x0000ffff);	/* 使用度数 加算				*/
																	/*								*/
					PayData.DiscountData[wrcnt].DiscCount = (uchar)1;		/* 使用枚数				*/
					PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];	/* 割引額				*/
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DPARKINGNO, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DSYU, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DNO, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DCOUNT, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DISCOUNT, wrcnt);
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 					PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 0;	/* 返却				*/
// 					PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;	/* 割引済み(新規精算)*/
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
					wrcnt++;										/*								*/
				}													/*								*/
				break;												/*								*/

			case	RY_GNG:											/* 減額精算						*/
// 不具合修正(S) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
			case	RY_GNG_FEE:
			case	RY_GNG_TIM:
// 不具合修正(E) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)


				// 精算ﾃﾞｰﾀ用
				if( wrcnt < WTIK_USEMAX ){										// 割引ﾃﾞｰﾀ数以下のみ
// MH322914(S) K.Onodera 2016/08/08 AI-V対応：遠隔精算(精算金額指定)
					// 遠隔精算中割引(金額指定)
// 不具合修正(S) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
//					if( vl_now == V_DIS_FEE ){
					if( rhs_p->param == RY_GNG_FEE ){
// 不具合修正(E) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
						PayData.DiscountData[wrcnt].ParkingNo = CPrmSS[S_SYS][1];						// 駐車場№：基本駐車場№
						PayData.DiscountData[wrcnt].DiscSyu = g_PipCtrl.stRemoteFee.DiscountKind;		// 割引種別
						PayData.DiscountData[wrcnt].DiscNo = g_PipCtrl.stRemoteFee.DiscountType;		// 割引区分
						PayData.DiscountData[wrcnt].DiscCount = g_PipCtrl.stRemoteFee.DiscountCnt;		// 割引使用枚数
						PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];							// 割引額
						PayData.DiscountData[wrcnt].DiscInfo1 = g_PipCtrl.stRemoteFee.DiscountInfo1;	// 割引情報１
						PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = g_PipCtrl.stRemoteFee.DiscountInfo2;	// 割引情報２
					}
					// 遠隔精算中割引(入庫時刻指定)
// 不具合修正(S) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
//					else if( vl_now == V_DIS_TIM ){
					else if( rhs_p->param == RY_GNG_TIM ){
// 不具合修正(E) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
						PayData.DiscountData[wrcnt].ParkingNo = CPrmSS[S_SYS][1];						// 駐車場№：基本駐車場№
						PayData.DiscountData[wrcnt].DiscSyu = g_PipCtrl.stRemoteTime.DiscountKind;		// 割引種別
						PayData.DiscountData[wrcnt].DiscNo = g_PipCtrl.stRemoteTime.DiscountType;		// 割引区分
						PayData.DiscountData[wrcnt].DiscCount = g_PipCtrl.stRemoteTime.DiscountCnt;		// 割引使用枚数
						PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];							// 割引額
						PayData.DiscountData[wrcnt].DiscInfo1 = g_PipCtrl.stRemoteTime.DiscountInfo1;	// 割引情報１
						PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = g_PipCtrl.stRemoteTime.DiscountInfo2;	// 割引情報２
					}
					// 減額精算
					else{
// MH322914(E) K.Onodera 2016/08/08 AI-V対応：遠隔精算(精算金額指定)
						PayData.DiscountData[wrcnt].ParkingNo = CPrmSS[S_SYS][1];	// 駐車場№：基本駐車場№
						PayData.DiscountData[wrcnt].DiscSyu = NTNET_GENGAKU;		// 割引種別：減額精算
						PayData.DiscountData[wrcnt].DiscNo = 0;						// 割引区分：支払い（固定）
						PayData.DiscountData[wrcnt].DiscCount = 1;					// 使用枚数：1枚（固定）
						PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];		// 割引額
						PayData.DiscountData[wrcnt].DiscInfo1 = 0L;					// 割引情報1(未使用)
						PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = 0L;// 割引情報2(未使用)
// MH322914(S) K.Onodera 2016/08/08 AI-V対応：遠隔精算(精算金額指定)
					}
// MH322914(E) K.Onodera 2016/08/08 AI-V対応：遠隔精算(精算金額指定)
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DPARKINGNO, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DSYU, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DNO, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DCOUNT, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DISCOUNT, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO1, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO2, wrcnt);
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 					PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;	// 使用
// 					PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;	// 割引済み(新規精算)
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
					wrcnt++;
				}

				break;

			case	RY_FRK:											/* 振替精算						*/

				// 領収証,個別精算情報用
// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
				if( PiP_GetFurikaeSts() ){
// 仕様変更(S) K.Onodera 2016/12/05 振替額変更
//					// 振替元と振替先の料金種別が同じ or 料金種別が異なってもサービス券/掛売り券振替する？
//					if( prm_get(COM_PRM, S_CEN, 32, 1, 1) || ((ryo_buf.syubet+1) == vl_frs.syubetu) ){
//						OrgDiscount = ( vl_frs.price - (vl_frs.genkin_furikaegaku + vl_frs.card_furikaegaku) );	// 振替割引
//					}else{
//						OrgDiscount = ( vl_frs.in_price - (vl_frs.genkin_furikaegaku + vl_frs.card_furikaegaku) );	// 振替割引
//					}
//					// 振替先の駐車料金を超える？
//					if( OrgDiscount > vl_frs.furikaegaku ){
//						OrgDiscount = vl_frs.furikaegaku;
//					}
// 仕様変更(E) K.Onodera 2016/12/05 振替額変更
					if( g_PipCtrl.stFurikaeInfo.Remain <= 0 ){
// 仕様変更(S) K.Onodera 2016/12/02 振替額変更
//						ulong	kabarai = 0;
//						kabarai = (g_PipCtrl.stFurikaeInfo.DestFeeDiscount - vl_frs.furikaegaku);
						// 過払い = 振替対象額 - 振替可能額
// 仕様変更(E) K.Onodera 2016/12/02 振替額変更
						// 現金使用あり？
						if( vl_frs.genkin_furikaegaku ){
// 仕様変更(S) K.Onodera 2016/12/02 振替額変更
//							if( kabarai > vl_frs.genkin_furikaegaku ){
//								kabarai = vl_frs.genkin_furikaegaku;
//							}
// 仕様変更(E) K.Onodera 2016/12/02 振替額変更
							PayData.FRK_Return = 					// 振替過払い金(クレジット等の後日返金も含む)
// 仕様変更(S) K.Onodera 2016/12/02 振替額変更
//							ryo_buf.kabarai = kabarai;				// 振替過払い金(返金動作する額)
							ryo_buf.kabarai = vl_frs.kabarai;
// 仕様変更(E) K.Onodera 2016/12/02 振替額変更
// MH810100(S) K.Onodera 2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//// 不具合修正(S) K.Onodera 2016/12/08 #1642 振替過払い金(現金による払い戻し)が、 精算情報データの釣銭額に加算されない
//							NTNET_Data152Save((void *)(&PayData.FRK_Return), NTNET_152_KABARAI);
//// 不具合修正(E) K.Onodera 2016/12/08 #1642 振替過払い金(現金による払い戻し)が、 精算情報データの釣銭額に加算されない
// MH810100(E) K.Onodera 2019/12/24 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
						}
						// クレジット又は電子マネー使用あり？
						else if( vl_frs.card_type && vl_frs.card_furikaegaku ){
// 仕様変更(S) K.Onodera 2016/12/02 振替額変更
//							if( kabarai > vl_frs.card_furikaegaku ){
//								kabarai = vl_frs.card_furikaegaku;
//							}
// 仕様変更(E) K.Onodera 2016/12/02 振替額変更
// 仕様変更(S) K.Onodera 2016/12/02 振替額変更
//							PayData.FRK_Return = kabarai;			// 振替過払い金(クレジット等の後日返金も含む)
							PayData.FRK_Return = vl_frs.kabarai;
// 仕様変更(E) K.Onodera 2016/12/02 振替額変更
							ryo_buf.kabarai = 0;					// 振替過払い金(返金動作する額)=0
							PayData.FRK_RetMod = vl_frs.card_type;	// 媒体
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
							NTNET_Data152Save((void *)&PayData.FRK_RetMod, NTNET_152_CARD_FUSOKU_TYPE );
							NTNET_Data152Save((void *)&PayData.FRK_Return, NTNET_152_CARD_FUSOKU );
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
						}
					}
					for( DetailCnt=0; DetailCnt<DETAIL_SYU_MAX; DetailCnt++ ){
						switch( DetailCnt ){
							case 0:	// 振替精算
								PayData.DetailData[DetailCnt].ParkingNo = CPrmSS[S_SYS][1];												// 駐車場№：基本駐車場№
								PayData.DetailData[DetailCnt].DiscSyu = NTNET_FURIKAE_2;
								PayData.DetailData[DetailCnt].uDetail.Furikae.Pos = (ulong)(LockInfo[vl_frs.lockno - 1].area*10000L
															 									+ LockInfo[vl_frs.lockno - 1].posi);	// 振替元区画情報
								PayData.DetailData[DetailCnt].uDetail.Furikae.OrgFee = vl_frs.price;									// 振替元駐車料金
								PayData.DetailData[DetailCnt].uDetail.Furikae.Total  = vl_frs.furikaegaku;								// 振替額
								PayData.DetailData[DetailCnt].uDetail.Furikae.Oiban  = vl_frs.seisan_oiban.i;							// 振替元精算追番※上位２桁を料金種別として
																																		// 使用しているが、現状追番は0～99999なので問題ない
								PayData.DetailData[DetailCnt].uDetail.Furikae.Oiban  += (vl_frs.syubetu * 100000L);						// 振替元料金種別
								break;
							case 1:	// 振替精算内訳
								PayData.DetailData[DetailCnt].ParkingNo = CPrmSS[S_SYS][1];							// 駐車場№：基本駐車場№
								PayData.DetailData[DetailCnt].DiscSyu = NTNET_FURIKAE_DETAIL;
								PayData.DetailData[DetailCnt].uDetail.FurikaeDetail.Mod = vl_frs.card_type;			// 媒体種別
								if( vl_frs.card_type ){
									PayData.DetailData[DetailCnt].uDetail.FurikaeDetail.FrkMoney = 0;
// 仕様変更(S) K.Onodera 2016/12/05 振替額変更
//									PayData.DetailData[DetailCnt].uDetail.FurikaeDetail.FrkCard = ( vl_frs.furikaegaku - OrgDiscount );
									PayData.DetailData[DetailCnt].uDetail.FurikaeDetail.FrkCard = vl_frs.card_furikaegaku;
// 仕様変更(E) K.Onodera 2016/12/05 振替額変更
								}else{
// 仕様変更(S) K.Onodera 2016/12/05 振替額変更
//									PayData.DetailData[DetailCnt].uDetail.FurikaeDetail.FrkMoney = ( vl_frs.furikaegaku - OrgDiscount );
									PayData.DetailData[DetailCnt].uDetail.FurikaeDetail.FrkMoney = vl_frs.genkin_furikaegaku;
// 仕様変更(E) K.Onodera 2016/12/05 振替額変更
									PayData.DetailData[DetailCnt].uDetail.FurikaeDetail.FrkCard = 0;
								}
// 仕様変更(S) K.Onodera 2016/12/05 振替額変更
//								PayData.DetailData[DetailCnt].uDetail.FurikaeDetail.FrkDiscount = OrgDiscount;		// 振替額(割引)
								PayData.DetailData[DetailCnt].uDetail.FurikaeDetail.FrkDiscount = vl_frs.wari_furikaegaku;	// 振替額(割引)
// 仕様変更(E) K.Onodera 2016/12/05 振替額変更
								break;
							default:
								continue;
								break;
						}
// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
						NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[DetailCnt], NTNET_152_DPARKINGNO, DetailCnt);
						NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[DetailCnt], NTNET_152_DSYU, DetailCnt);
						NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[DetailCnt], NTNET_152_DNO, DetailCnt);
						NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[DetailCnt], NTNET_152_DCOUNT, DetailCnt);
						NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[DetailCnt], NTNET_152_DISCOUNT, DetailCnt);
						NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[DetailCnt], NTNET_152_DINFO1, DetailCnt);
						NTNET_Data152_DetailDataSave((void *)&PayData.DetailData[DetailCnt], NTNET_152_DINFO2, DetailCnt);
// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
					}
				}else{
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//					PayData.MMPlace = (ulong)(LockInfo[vl_frs.lockno - 1].area*10000L
//												 + LockInfo[vl_frs.lockno - 1].posi);
//					PayData.MMPrice = vl_frs.price;
//					PayData.MMInPrice = vl_frs.in_price;
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
				}
// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応

				// 精算ﾃﾞｰﾀ用
				if( wrcnt < WTIK_USEMAX ){											// 割引ﾃﾞｰﾀ数以下のみ
// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
//// MH322914(S) K.Onodera 2016/09/15 AI-V対応：振替精算
//					if( PiP_GetFurikaeSts() ){
//						PayData.DiscountData[wrcnt].ParkingNo = CPrmSS[S_SYS][1];		// 駐車場№：基本駐車場№
//						PayData.DiscountData[wrcnt].DiscSyu = NTNET_FURIKAE_2;			// 2002
//						wk_dat3 = (LockInfo[vl_frs.lockno-1].area*10000L) + LockInfo[vl_frs.lockno-1].posi;
//						*(ulong*)&PayData.DiscountData[wrcnt].DiscNo = wk_dat3;								// 割引元区画情報
//						PayData.DiscountData[wrcnt].Discount = vl_frs.furikaegaku;							// 支払済金額(振替可能額)
//						if( g_PipCtrl.stFurikaeInfo.Remain > 0 ){
//							PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = g_PipCtrl.stFurikaeInfo.Remain;	// 不足金額
//						}else{
//// 不具合修正(S) K.Onodera 2016/10/13 #1514 振替先で定期を利用すると通知した上で定期を使用せずに精算した時の釣銭が正しくない
////// 不具合修正(S) K.Onodera 2016/10/07 #1516 時間割定期券を使用した振替精算で払い戻される金額が少ない
//////							ryo_buf.turisen = (g_PipCtrl.stFurikaeInfo.DestFeeDiscount - ryo_buf.tyu_ryo);
////							ryo_buf.turisen = (g_PipCtrl.stFurikaeInfo.DestFeeDiscount - vl_frs.furikaegaku);			// 振替元振替可能額 - 振替額
////// 不具合修正(E) K.Onodera 2016/10/07 #1516 時間割定期券を使用した振替精算で払い戻される金額が少ない
////							if( ryo_buf.turisen > vl_frs.genkin_furikaegaku ){
////								ryo_buf.turisen = vl_frs.genkin_furikaegaku;
////							}
////							PayData.DiscountData[wrcnt].DiscInfo1 = ryo_buf.turisen;									// 過払金額(現金のみ)
//							PayData.DiscountData[wrcnt].DiscInfo1 = (g_PipCtrl.stFurikaeInfo.DestFeeDiscount - vl_frs.furikaegaku);
//							// 現金使用あり？
//							if( vl_frs.genkin_furikaegaku ){
//								if( PayData.DiscountData[wrcnt].DiscInfo1 > vl_frs.genkin_furikaegaku ){
//									PayData.DiscountData[wrcnt].DiscInfo1 = vl_frs.genkin_furikaegaku;
//								}
//								PayData.FRK_Return = 										// 振替過払い金(クレジット等の後日返金も含む)
//								ryo_buf.kabarai = PayData.DiscountData[wrcnt].DiscInfo1;	// 振替過払い金(返金動作する額)
//							}
//							// クレジット又は電子マネー使用あり？
//							else if( vl_frs.card_type && vl_frs.card_furikaegaku ){
//								if( PayData.DiscountData[wrcnt].DiscInfo1 > vl_frs.card_furikaegaku ){
//									PayData.DiscountData[wrcnt].DiscInfo1 = vl_frs.card_furikaegaku;
//								}
//								PayData.FRK_Return = PayData.DiscountData[wrcnt].DiscInfo1;	// 振替過払い金(クレジット等の後日返金も含む)
//								ryo_buf.kabarai = 0;										// 振替過払い金(返金動作する額)=0
//								PayData.FRK_RetMod = vl_frs.card_type;						// 媒体
//							}
//// 仕様変更(S) K.Onodera 2016/10/28 振替過払い金と釣銭切り分け
//
//// 仕様変更(E) K.Onodera 2016/10/28 振替過払い金と釣銭切り分け
//// 不具合修正(E) K.Onodera 2016/10/13 #1514 振替先で定期を利用すると通知した上で定期を使用せずに精算した時の釣銭が正しくない
//						}
//					}
//					else{
//// MH322914(E) K.Onodera 2016/09/15 AI-V対応：振替精算
					if( PiP_GetFurikaeSts() == 0 ){
// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応
					PayData.DiscountData[wrcnt].ParkingNo = CPrmSS[S_SYS][1];		// 駐車場№：基本駐車場№
					PayData.DiscountData[wrcnt].DiscSyu = NTNET_FURIKAE;			// 割引種別：減額精算

					wk_dat3 = (LockInfo[vl_frs.lockno-1].area*10000L) + LockInfo[vl_frs.lockno-1].posi;
					*(ulong*)&PayData.DiscountData[wrcnt].DiscNo = wk_dat3;

					PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];			// 割引額
					PayData.DiscountData[wrcnt].DiscInfo1 = (ulong)rhs_p->data[4];	// 割引情報1：振替元 精算月日時分
					PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = CountSel(&vl_frs.seisan_oiban);// 割引情報2：振替元 精算追番
// MH322914(S) K.Onodera 2016/09/15 AI-V対応：振替精算
					}
// MH322914(E) K.Onodera 2016/09/15 AI-V対応：振替精算
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DPARKINGNO, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DSYU, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DNO, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DCOUNT, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO1, wrcnt);
					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO2, wrcnt);
// GG124100(S) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
// 					PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;		// 使用
// 					PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;		// 割引済み(新規精算)
// GG124100(E) R.Endo 2022/07/21 車番チケットレス3.0 #6465 割引10種対応 [共通改善項目 No1526]
					wrcnt++;
				}
				break;

			case	RY_SSS:
// MH810100(S) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
//				// 領収証,個別精算情報用
//				PayData.MMSyubetu = vl_frs.syubetu;									// 振替元・料金種別
//				PayData.MMPlace = (ulong)(LockInfo[vl_frs.lockno - 1].area*10000L
//											 + LockInfo[vl_frs.lockno - 1].posi);
//				PayData.MMPrice = vl_frs.price;										// 間違い精算：駐車料金
//				PayData.MMInPrice = syusei[vl_frs.lockno-1].gen;					// 間違い精算：現金入金額
//				PayData.MMRwari = syusei[vl_frs.lockno-1].sy_wari;					// 間違い精算：料金割引金額
//				PayData.MMPwari = vl_frs.seisan_oiban;								// 修正元 精算追番
//
//				if(prm_get(COM_PRM, S_TYP, 62, 1, 2) == 1){							// 同一種別のみ時間割引を含む
//					if(vl_frs.syubetu == (char)(ryo_buf.syubet+1)){					// 修正元と修正先で種別が同じ？
//						PayData.MMRwari += syusei[vl_frs.lockno-1].sy_time;			// 間違い精算：時間割引金額
//					}
//				}else{																// 時間割引を含む
//					// 設定は0～2の範囲で0の時はここには来ないため、1以外の判断とする
//					PayData.MMRwari += syusei[vl_frs.lockno-1].sy_time;				// 間違い精算：時間割引金額
//				}
//				if( (syusei[vl_frs.lockno-1].infofg & SSS_ZENWARI)&&				// 全額割引あり
//					(prm_get(COM_PRM, S_TYP, 98, 1, 2)) ){							// 全額割引する
//					PayData.Zengakufg |= 0x01;										// 全額割引bit0を1とする
//				}
//
//				// 精算ﾃﾞｰﾀ用
//				if( wrcnt < (WTIK_USEMAX-2) ){						// 割引ﾃﾞｰﾀ数以下のみ
//					// 割引エリア1
//					PayData.DiscountData[wrcnt].ParkingNo = CPrmSS[S_SYS][1];		// 駐車場№：基本駐車場№
//					PayData.DiscountData[wrcnt].DiscSyu = NTNET_SYUSEI_1;			// 割引種別：修正精算(92)
//					wk_dat3 = (LockInfo[vl_frs.lockno-1].area*10000L) + LockInfo[vl_frs.lockno-1].posi;
//					*(ulong*)&PayData.DiscountData[wrcnt].DiscNo = wk_dat3;
//
//					PayData.DiscountData[wrcnt].Discount = 0;						// 割引額：修正元 0⇒払戻額(ntnetctrlで格納)
//					PayData.DiscountData[wrcnt].DiscInfo1 = (ulong)rhs_p->data[4];	// 割引情報1：振替元 精算月日時分
//					PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = CountSel(&vl_frs.seisan_oiban);// 割引情報2：振替元 精算追番
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DPARKINGNO, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DSYU, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DNO, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DISCOUNT, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO1, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO2, wrcnt);
//					PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;		// 使用
//					PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;		// 割引済み(新規精算)
//					wrcnt++;
//
//					// 割引エリア2
//					PayData.DiscountData[wrcnt].ParkingNo = CPrmSS[S_SYS][1];		// 駐車場№：基本駐車場№
//					PayData.DiscountData[wrcnt].DiscSyu = NTNET_SYUSEI_2;			// 割引種別：修正精算(93)
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DPARKINGNO, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DSYU, wrcnt);
//					if( PayData.Zengakufg&0x01 ){									// 全額割引有り
//						PayData.DiscountData[wrcnt].DiscNo = 1;						// 割引区分：
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DNO, wrcnt);
//					}
//					PayData.DiscountData[wrcnt].Discount = rhs_p->data[2];			// 割引額：修正元 振替額
//					PayData.DiscountData[wrcnt].DiscInfo1 = PayData.MMInPrice;		// 割引情報1：振替元 入金額
//					PayData.DiscountData[wrcnt].uDiscData.common.DiscInfo2 = PayData.MMRwari;// 割引情報2：振替元 割引額
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DISCOUNT, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO1, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DINFO2, wrcnt);
//					PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;		// 使用
//					PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;		// 割引済み(新規精算)
//					wrcnt++;
//
//					// 割引エリア3
//					PayData.DiscountData[wrcnt].ParkingNo = CPrmSS[S_SYS][1];		// 駐車場№：基本駐車場№
//					PayData.DiscountData[wrcnt].DiscSyu = NTNET_SYUSEI_3;			// 割引種別：修正精算(94)
//					PayData.DiscountData[wrcnt].Discount = 0;						// 割引額：修正元 0⇒未入金額(ntnetctrlで格納)
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DPARKINGNO, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DSYU, wrcnt);
//					NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[wrcnt], NTNET_152_DISCOUNT, wrcnt);
//					PayData.DiscountData[wrcnt].uDiscData.common.MoveMode = 1;		// 使用
//					PayData.DiscountData[wrcnt].uDiscData.common.DiscFlg = 0;		// 割引済み(新規精算)
//					wrcnt++;
//				}
// MH810100(E) K.Onodera 2019/11/25 車番チケットレス(領収証印字)
				break;
// MH321800(S) 割引種別8000のセット方法を修正
//// MH321800(S) D.Inaba ICクレジット対応
//			case RY_CRE:
//				// 決済リーダのｸﾚｼﾞｯﾄ決済のみ有効
//				if( isEC_USE() ){
//					for( i=0; (i < WTIK_USEMAX) && (0 != PayData.DiscountData[i].ParkingNo); i++ ){
//						;														/* 格納場所[位置]を探す			*/
//					}
//					if( i < NTNET_DIC_MAX ){									/* ｾｯﾄｴﾘｱに1つ分の空きがある場合 */
//						memset(&wk_DicData, 0, sizeof(wk_DicData));
//						wk_DicData.ParkingNo = CPrmSS[S_SYS][1];				/* 駐車場№：未使用				*/
//						wk_DicData.DiscSyu = NTNET_INQUIRY_NUM;					/* 割引種別：問合せ番号	       	*/
//						// ｸﾚｼﾞｯﾄ決済は問合せ番号がない為0x20埋め
//						memset( &wk_DicData.CardNo[0], 0x20, sizeof(wk_DicData.CardNo) );
//						memcpy(&PayData.DiscountData[i], &wk_DicData, sizeof(wk_DicData));
//
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DPARKINGNO, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DSYU, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DNO, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DCOUNT, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DISCOUNT, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO1, i);
//						NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[i], NTNET_152_DINFO2, i);
//					}
//				}
//				break;
//// MH321800(E) D.Inaba ICクレジット対応
// MH321800(E) 割引種別8000のセット方法を修正
			}														/*								*/
		}															/*								*/
		CardUseSyu = wrcnt;											/* 1精算の割引種類の件数ｸﾘｱ		*/
	}																/*								*/
ryo_cal_10:
																	/*								*/
// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
#if 1	// ここから：クラウド料金計算のryo_buf_calc(cal_cloud/cal_cloud.c)に相当。
		// ※修正する際はryo_buf_calc側も確認すること。
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
	if( ryo_buf.ryo_flg <= 1 )										/* 定期使用無し精算？			*/
	{																/*								*/
		if(	ryo_buf.tyu_ryo >= ryo_buf.waribik )					/*								*/
		{															/*								*/
			ryo_buf.kazei = ryo_buf.tyu_ryo -	ryo_buf.waribik;	/* 課税対象額＝駐車料金 － 割引額計	*/
		} else {													/*								*/
			ryo_buf.kazei = 0l;										/*								*/
		}															/*								*/
	} else {														/* 定期使用						*/
		if(	ryo_buf.tei_ryo >= ryo_buf.waribik )					/*								*/
		{															/*								*/
			ryo_buf.kazei = ryo_buf.tei_ryo -	ryo_buf.waribik;	/* 課税対象額＝定期券駐車料金 － 割引額計	*/
		} else {													/*								*/
			ryo_buf.kazei = 0l;										/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
	if( CPrmSS[S_CAL][19] )											/* 税率設定あり					*/
	{																/*								*/
		if( prm_get( COM_PRM,S_CAL,20,1,1 ) >= 2L )					/* 外税?						*/
		{															/*								*/
			sot_tax = ryo_buf.tax;									/*								*/
		}															/*								*/
	}																/*								*/
																	/*								*/
	if( ryo_buf.credit.pay_ryo != 0 )
	{
		if( ryo_buf.zankin == 0 ) {
			if ((ryo_buf.credit.cre_type == CREDIT_CARD) || (ryo_buf.credit.cre_type == CREDIT_HOJIN)){
				// tyu_ryo は時間帯定期での出庫時など、このﾀｲﾐﾝｸﾞで 0 の場合がありえる、 dsp_ryo を使う.
				if( ryo_buf.dsp_ryo < ( ryo_buf.nyukin + ryo_buf.credit.pay_ryo ) ){
					//入金と、クレジット認識のすれ違い時に起こりうる
					ryo_buf.turisen =  ( ryo_buf.nyukin + ryo_buf.credit.pay_ryo ) - ryo_buf.dsp_ryo;
				}
				ryo_buf.dsp_ryo = 0L;
			}
			else {
				// サービス券、プリペイドカードなどの場合と同じく、入金分をdsp_ryoに残し
				// opeのinvcrd()で釣銭を計算させる。
				// （クレジットカード時もiDと同じで良いはずだが他機種との互換を考慮して変えない）
				if (ryo_buf.dsp_ryo >= ryo_buf.credit.pay_ryo) {
					ryo_buf.dsp_ryo -= ryo_buf.credit.pay_ryo;
				}
				else {
					ryo_buf.dsp_ryo = 0;
				}
			}
		}
		goto L_RyoCalCrePay;
	}

	switch( ryo_buf.ryo_flg )										/*								*/
	{																/*								*/
		case 0:														/* 通常(定期使用無し)精算		*/
			if(( ryo_buf.tyu_ryo + sot_tax )						/* 駐車料金＋外税 ＜ 入金額＋割引額計?	*/
					< ( ryo_buf.nyukin + ryo_buf.waribik + c_pay ))	/*								*/
			{														/*								*/
				dat = ( ryo_buf.nyukin + ryo_buf.waribik + c_pay )	/*								*/
						- ( ryo_buf.tyu_ryo + sot_tax );			/*								*/
// MH322914(S) K.Onodera 2016/12/12 AI-V対応：振替精算
				if( OPECTL.op_faz == 2 && PiP_GetFurikaeSts() ){
					// 定期あり振替で定期を使用せずに釣銭ありの精算を行った場合
					// 釣銭は算出済みのため抜ける
					break;
				}
// MH322914(E) K.Onodera 2016/12/12 AI-V対応：振替精算
				if( dat <= ryo_buf.nyukin )							/*								*/
				{													/*								*/
					ryo_buf.turisen = ryo_buf.nyukin - dat;			/* 多い入金分を戻す				*/
				}													/*								*/
			}														/*								*/

			if( e_inflg == 1 ) {									// 電子マネーによる入金時
				if(( ryo_buf.tyu_ryo + sot_tax ) > ( ryo_buf.waribik + c_pay + ryo_buf.emonyin ))
				{													/*								*/
					ryo_buf.dsp_ryo = ( ryo_buf.tyu_ryo + sot_tax  )/* 表示料金＝(駐車料金＋外税)	*/
									- ( ryo_buf.waribik + c_pay + ryo_buf.emonyin );	/*	－割引額計	*/
				} else {											/*								*/
					ryo_buf.dsp_ryo = 0l;							/* 表示料金＝０					*/
					if( ryo_buf.nyukin != 0 )						/* 入金有り?					*/
					{												/*								*/
						ryo_buf.turisen = ryo_buf.nyukin;			/* 全額戻し						*/
					}												/*								*/
				}													/*								*/
			}else{									 				// 入金額 = 駐車料金
				if(( ryo_buf.tyu_ryo + sot_tax ) > ( ryo_buf.waribik + c_pay + ryo_buf.emonyin ))
				{													/*								*/
					ryo_buf.dsp_ryo = ( ryo_buf.tyu_ryo + sot_tax  )/* 表示料金＝(駐車料金＋外税)	*/
// MH810100(S) m.saito 2020/05/26 車番チケットレス(#4184 再精算時の不具合対応)
//// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
////									- ( ryo_buf.waribik + c_pay + ryo_buf.emonyin );	/*	－割引額計	*/
//									- ( ryo_buf.waribik + c_pay + ryo_buf.emonyin + ryo_buf.zenkai );	/*	－割引額計	*/
//// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス(LCD_IF対応)
									- ( ryo_buf.waribik + c_pay + ryo_buf.emonyin );	/*	－割引額計	*/
// MH810100(E) m.saito 2020/05/26 車番チケットレス(#4184 再精算時の不具合対応)
				} else {											/*								*/
					ryo_buf.dsp_ryo = 0l;							/* 表示料金＝０					*/
					if( ryo_buf.nyukin != 0 )						/* 入金有り?					*/
					{												/*								*/
						ryo_buf.turisen = ryo_buf.nyukin;			/* 全額戻し						*/
					}												/*								*/
				}													/*								*/
			}
			break;													/*								*/
		case 1:														/* 修正(定期使用無し)精算		*/
			if(( ryo_buf.tyu_ryo + sot_tax ) >= ( ryo_buf.waribik + c_pay ))	/*					*/
			{														/*								*/
				ryo_buf.dsp_ryo = ( ryo_buf.tyu_ryo + sot_tax )		/* 表示料金＝(駐車料金＋外税)	*/
								 	- ( ryo_buf.waribik + c_pay );	/*					－割引額計	*/
			} else {												/*								*/
				ryo_buf.dsp_ryo = 0l;								/* 表示料金＝０					*/
				if( ryo_buf.nyukin != 0 )							/* 入金有り?					*/
				{													/*								*/
					ryo_buf.turisen = ryo_buf.nyukin;				/* 全額戻し						*/
				}													/*								*/
			}														/*								*/
			break;													/*								*/
		case 2:														/* 通常(定期使用)精算			*/

			if( e_inflg == 1 ) {									// 電子マネーによる入金時
				if(( ryo_buf.tei_ryo + sot_tax )						/* 定期駐車料金＋外税 ＜ 入金額＋割引額計？*/
						< ( ryo_buf.nyukin + ryo_buf.waribik + c_pay + ryo_buf.emonyin))
				{														/*								*/
					dat = ( ryo_buf.nyukin + ryo_buf.waribik + c_pay + ryo_buf.emonyin)
						- ( ryo_buf.tei_ryo + sot_tax );				/*								*/
					if( dat <= ryo_buf.nyukin )							/*								*/
					{													/*								*/
						ryo_buf.turisen = ryo_buf.nyukin - dat;			/* 多い入金分を戻す				*/
					}													/*								*/
				}														/*								*/
				if(( ryo_buf.tei_ryo + sot_tax ) > ( ryo_buf.waribik + c_pay + ryo_buf.emonyin))
				{														/*								*/
					ryo_buf.dsp_ryo = ( ryo_buf.tei_ryo + sot_tax  )	/* 表示料金＝(定期駐車料金＋外税)	*/
									- ( ryo_buf.waribik + c_pay + ryo_buf.emonyin);	/*		－割引額計	*/
				} else {												/*								*/
				ryo_buf.dsp_ryo = 0l;									/* 表示料金＝０					*/
					if( ryo_buf.nyukin != 0 )							/* 入金有り?					*/
					{													/*								*/
						ryo_buf.turisen = ryo_buf.nyukin;				/* 全額戻し						*/
					}													/*								*/
				}														/*								*/
			}else{									 				// 入金額 = 駐車料金
			if(( ryo_buf.tei_ryo + sot_tax )						/* 定期駐車料金＋外税 ＜ 入金額＋割引額計？*/
						< ( ryo_buf.nyukin + ryo_buf.waribik + c_pay + ryo_buf.emonyin))	/*			*/
				{														/*								*/
					dat = ( ryo_buf.nyukin + ryo_buf.waribik + c_pay + ryo_buf.emonyin)	/*				*/
						- ( ryo_buf.tei_ryo + sot_tax );				/*								*/
					if( dat <= ryo_buf.nyukin )							/*								*/
					{													/*								*/
						ryo_buf.turisen = ryo_buf.nyukin - dat;			/* 多い入金分を戻す				*/
					}													/*								*/
				}														/*								*/
				if(( ryo_buf.tei_ryo + sot_tax ) > ( ryo_buf.waribik + c_pay + ryo_buf.emonyin))	/*	*/
				{														/*								*/
					ryo_buf.dsp_ryo = ( ryo_buf.tei_ryo + sot_tax  )	/* 表示料金＝(定期駐車料金＋外税)	*/
									- ( ryo_buf.waribik + c_pay + ryo_buf.emonyin);	/*		－割引額計	*/
				} else {												/*								*/
				ryo_buf.dsp_ryo = 0l;									/* 表示料金＝０					*/
					if( ryo_buf.nyukin != 0 )							/* 入金有り?					*/
					{													/*								*/
						ryo_buf.turisen = ryo_buf.nyukin;				/* 全額戻し						*/
					}													/*								*/
				}														/*								*/
			}
			break;													/*								*/
		case 3:														/* 修正(定期使用)精算			*/
			if(( ryo_buf.tei_ryo + sot_tax ) >= (ryo_buf.waribik + c_pay ))	/*						*/
			{														/*								*/
				ryo_buf.dsp_ryo = ( ryo_buf.tei_ryo + sot_tax )		/* 表示料金＝(定期駐車料金＋外税)	*/
								  - ( ryo_buf.waribik + c_pay );	/*					－割引額計	*/
			} else {												/*								*/
				ryo_buf.dsp_ryo = 0l;								/* 表示料金＝０					*/
				if( ryo_buf.nyukin != 0 )							/* 入金有り?					*/
				{													/*								*/
					ryo_buf.turisen = ryo_buf.nyukin;				/* 全額戻し						*/
				}													/*								*/
			}														/*								*/
			break;													/*								*/
	}																/*								*/
// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
#endif	// ここまで：クラウド料金計算のryo_buf_calc(cal_cloud/cal_cloud.c)に相当。
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応

L_RyoCalCrePay:
	WACDOG;// RX630はWDGストップが出来ない仕様のためWDGリセットにする
	memcpy( &car_ot, &Adjustment_Beginning, sizeof( car_ot ) );		/*								*/
	return;															/*								*/
}																	/*								*/
																	/*								*/
																	/*								*/
#if SYUSEI_PAYMENT
/*[]----------------------------------------------------------------------------------------------[]*/
/*| 修正精算用料金計算処理ﾒｲﾝ																	   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: sryo_cal( num )																   |*/
/*| PARAMETER	: num	; 内部処理用駐車位置番号(1～324)										   |*/
/*| RETURN VALUE: void	;																		   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: R.Hara(AMANO)																	   |*/
/*| Date		: 2005-07-12																	   |*/
/*| Update		: 																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	sryo_cal( ushort num )
{
	cm27();																// ﾌﾗｸﾞｸﾘｱ

	ryo_cal( 100, num );												// 修正精算料金計算

	if( syusei[num - 1].tei_id != 0 ){									// 間違え時定期使用?
		vl_passet( num );
		ryo_cal( 1, num );												// 定期券料金計算
	}
}
#endif		// SYUSEI_PAYMENT

/*[]----------------------------------------------------------------------------------------------[]*/
/*| サービスタイム中チェック																	   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: SvsTimChk( ushort no )														   |*/
/*|				  																				   |*/
/*| PARAMETER	: no = ﾌﾗｯﾌﾟ／ﾛｯｸ装置No.(1～324)												   |*/
/*|				  																				   |*/
/*| RETURN VALUE: チェック結果																	   |*/
/*|				  	OK:ｻｰﾋﾞｽﾀｲﾑ中																   |*/
/*|				  	NG:ｻｰﾋﾞｽﾀｲﾑ終了																   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| Author		: K.Motohashi(AMANO)															   |*/
/*| Date		: 2006-10-13																	   |*/
/*| Update		: 																				   |*/
/*|				  																				   |*/
/*[]------------------------------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
char	SvsTimChk( ushort no )
{
	char			ans = NG;			// ﾁｪｯｸ結果（戻り値）
	flp_com			*pk_info;			// 駐車位置情報ﾎﾟｲﾝﾀｰ
	long			svstime;			// ｻｰﾋﾞｽﾀｲﾑ設定値
	uchar			syu;				// 車種
	struct	CAR_TIM wk_intm;										// 入庫時刻
	struct	CAR_TIM	now_time;			// 現在時刻
	short			ret;				// 時刻比較結果
// MH810100(S) K.Onodera 2019/12/05 車番チケットレス(料金計算)
//	long	svtm,uptm;
// MH810100(E) K.Onodera 2019/12/05 車番チケットレス(料金計算)

	if( (no > 0) && (no <= LOCK_MAX) ){						// 装置No.ﾁｪｯｸ

		pk_info = &FLAPDT.flp_data[no-1];					// 駐車位置情報ﾎﾟｲﾝﾀｰｾｯﾄ

		if( pk_info->nstat.bits.b00 == 1 ){					// 車室状態ﾁｪｯｸ
			// 車両あり

			// 入庫時刻ﾃﾞｰﾀ取得

			wk_intm.year = pk_info->year;							// 入庫（年）
			wk_intm.mon  = pk_info->mont;							// 入庫（月）
			wk_intm.day  = pk_info->date;							// 入庫（日）
																	// 入庫（曜日）
			wk_intm.week = (char)youbiget( wk_intm.year, (short)wk_intm.mon, (short)wk_intm.day );
			wk_intm.hour = pk_info->hour;							// 入庫（時）
			wk_intm.min  = pk_info->minu;							// 入庫（分）

			// 現在時刻ﾃﾞｰﾀ取得
			now_time.year = CLK_REC.year;					// 現在（年）
			now_time.mon  = CLK_REC.mont;					// 現在（月）
			now_time.day  = CLK_REC.date;					// 現在（日）
			now_time.week = CLK_REC.week;					// 現在（曜日）
			now_time.hour = CLK_REC.hour;					// 現在（時）
			now_time.min  = CLK_REC.minu;					// 現在（分）

			// ｻｰﾋﾞｽﾀｲﾑ設定値取得
			if( pk_info->lag_to_in.BIT.LAGIN == ON ){		// ﾗｸﾞﾀｲﾏｰﾀｲﾑｱｳﾄによる再入庫？
				svstime = 0;								// Yes→ｻｰﾋﾞｽﾀｲﾑは無効
			}
			else{
				if( prm_get( COM_PRM,S_STM,1,1,1 ) == 0 ){	// 全共通ｻｰﾋﾞｽﾀｲﾑ？
					svstime = CPrmSS[S_STM][2];				// 共通ｻｰﾋﾞｽﾀｲﾑ設定値取得
				}else{
					syu = LockInfo[no-1].ryo_syu;
					svstime = CPrmSS[S_STM][5+(3*(syu-1))];	// 種別毎ｻｰﾋﾞｽﾀｲﾑ設定値取得
				}

// MH322914 (s) kasiyama 2016/07/12 ロック閉タイムをサービスタイムまで延長する[共通バグNo.1211](MH341106)
//				if(	( pk_info->mode == FLAP_CTRL_MODE3 )||( pk_info->mode == FLAP_CTRL_MODE4 ) )
//				{
//					svtm = svstime * 60;
//
//					if( no >= LOCK_START_NO)
//					{
//						uptm = (long)(( CPrmSS[S_TYP][69] / 100 ) * 60 );		// 分を秒に変換し格納
//						uptm += (long)( CPrmSS[S_TYP][69] % 100 );				// 秒の設定を加算し全てを秒換算する
//					}
//					else
//					{
//						uptm = (long)(( CPrmSS[S_TYP][118] / 100 ) * 60 );		// 分を秒に変換し格納
//						uptm += (long)( CPrmSS[S_TYP][118] % 100 );				// 秒の設定を加算し全てを秒換算する
//					}
//					if( svtm == uptm )
//					{
//						if( prm_get( COM_PRM,S_TYP,62,1,2 ) ) { 				// 修正精算機能あり設定
//							if( syusei[no-1].sei != 2 ){						// 復元車室でない
//								svstime = 0;									// ｻｰﾋﾞｽﾀｲﾑ無効
//							}													//
//						}else{													// 修正精算機能なし設定であれば復元車室なし
//							svstime = 0;										// ｻｰﾋﾞｽﾀｲﾑ無効
//						}
//					}
//				}
// MH810100(S) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)
//				if( prm_get( COM_PRM, S_TYP, 68, 1, 5 ) == 0 ) {
//					// ロック閉タイマーとサービスタイムが同じ時間の時にサービスタイムをロック閉までとする
//					if(	( pk_info->mode == FLAP_CTRL_MODE3 )||( pk_info->mode == FLAP_CTRL_MODE4 ) )
//					{
//						svtm = svstime * 60;
//
//						if( no >= LOCK_START_NO)
//						{
//							uptm = Carkind_Param(ROCK_CLOSE_TIMER, LockInfo[no-1].ryo_syu, 5,1);
//						}
//						else
//						{
//							uptm = Carkind_Param(FLAP_UP_TIMER, LockInfo[no-1].ryo_syu, 5,1);
//						}
//						uptm = (long)(((uptm/100)*60) + (uptm%100));
//
//						if( svtm == uptm )
//						{
//							if( prm_get( COM_PRM,S_TYP,62,1,2 ) ) { 				// 修正精算機能あり設定
//								if( syusei[no-1].sei != 2 ){						// 復元車室でない
//									svstime = 0;									// ｻｰﾋﾞｽﾀｲﾑ無効
//								}													//
//							}else{													// 修正精算機能なし設定であれば復元車室なし
//								svstime = 0;										// ｻｰﾋﾞｽﾀｲﾑ無効
//							}
//						}
//					}
//				}
// MH810100(E) K.Onodera 2019/12/26 車番チケットレス(不要処理削除)
// MH322914 (e) kasiyama 2016/07/12 ロック閉タイムをサービスタイムまで延長する[共通バグNo.1211](MH341106)
			}

			ec70( &wk_intm, svstime );										// 入庫時刻にｻｰﾋﾞｽﾀｲﾑを加算
			ret = ec64( &now_time, &wk_intm );								// 時刻比較


			if( ret != -1 ){
				// 現在時刻 ≦ 入庫時刻+ｻｰﾋﾞｽﾀｲﾑ
				if( svstime != 0 )
					ans = OK;								// ｻｰﾋﾞｽﾀｲﾑ中
			}
		}
	}
	return( ans );
}

/*[]----------------------------------------------------------------------------------------------[]*/
/*| 遠隔精算かチェック																			   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: is_paid_remote(Receipt_data*)													   |*/
/*| PARAMETER	: num	; 1精算情報ﾃﾞｰﾀ															   |*/
/*| RETURN VALUE: wari_dataのｲﾝﾃﾞｯｸｽ/ない場合-1													   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
int	is_paid_remote(Receipt_data* dat)
{
	int i;
	
	for (i = 0; i < WTIK_USEMAX; i++) {
		if (dat->DiscountData[i].DiscSyu == NTNET_GENGAKU) {
			return i;
		}
		if (dat->DiscountData[i].DiscSyu == NTNET_FURIKAE) {
			return i;
		}
	}
	
	return -1;
}
// 仕様変更(S) K.Onodera 2016/11/02 精算データフォーマット対応
int	is_ParkingWebFurikae( Receipt_data* dat )
{
	int i;

	for (i = 0; i < DETAIL_SYU_MAX; i++) {
		if (dat->DetailData[i].DiscSyu == NTNET_FURIKAE_2) {
			return i;
		}
	}

	return -1;
}
// 仕様変更(E) K.Onodera 2016/11/02 精算データフォーマット対応

/*[]----------------------------------------------------------------------------------------------[]*/
/*| 新修正精算（修正精算かチェック）															   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: is_paid_syusei(Receipt_data*)													   |*/
/*| PARAMETER	: num	; 1精算情報ﾃﾞｰﾀ															   |*/
/*| RETURN VALUE: wari_dataのｲﾝﾃﾞｯｸｽ/ない場合-1													   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
short	is_paid_syusei(Receipt_data* dat)
{
	short i;
	
	for (i = 0; i < WTIK_USEMAX; i++) {
		if (dat->DiscountData[i].DiscSyu == NTNET_SYUSEI_1) {
			return i;
		}
	}
	
	return -1;
}


/*[]----------------------------------------------------------------------------------------------[]*/
/*| 新修正精算（syuseiﾊﾞｯﾌｧからryo_bufにｾｯﾄ）													   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME	: ryo_SyuseiRyobufSet(void)	     												   |*/
/*| PARAMETER	:                            													   |*/
/*| RETURN VALUE:                            													   |*/
/*[]----------------------------------------------------------------------------------------------[]*/
void	ryo_SyuseiRyobufSet( void )
{
uchar	set;

	if( OPECTL.Pay_mod == 2 ){													// 修正精算の時
		ryo_buf.mis_ryo	= syusei[vl_frs.lockno-1].gen;							// 間違い現金領収額ｾｯﾄ
		ryo_buf.mis_wari= syusei[vl_frs.lockno-1].sy_wari;						// 間違い料金割引額ｾｯﾄ

		set = (uchar)prm_get(COM_PRM, S_TYP, 62, 1, 2);							// 時間割引含む？
		if( set == 1){															// 同一種別のみ時間割引を含む
			if(syusei[vl_frs.lockno-1].syubetu == (char)(ryo_buf.syubet+1)){	// 修正元と修正先で種別が同じ？
				ryo_buf.mis_wari += syusei[vl_frs.lockno-1].sy_time;			// 間違い精算：時間割引金額
			}
		}else if( set == 2){													// 時間割引を含む
			ryo_buf.mis_wari += syusei[vl_frs.lockno-1].sy_time;				// 間違い精算：時間割引金額
		}

		if( syusei[vl_frs.lockno-1].infofg & SSS_ZENWARI ){						// 全額割引使用していた?
			if( prm_get(COM_PRM, S_TYP, 98, 1, 2) ){							// 全額割引する
				ryo_buf.mis_zenwari = 1;										// 全額割引ﾌﾗｸﾞON
			}
		}
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| 種別毎に設定するのか判定 				       					       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Get_Pram_Syubet	                                       |*/
/*| PARAMETER    : no  (設定項目)                                          |*/
/*| RETURN VALUE : set_mod                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2010-09-07                                              |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
char Get_Pram_Syubet( char no ){
	char	set_mod;
	if(no <= 5){
		set_mod = (char)prm_get(COM_PRM, S_LTM, 1, 1, (char)(6-no));		
		if( (no == 4)||(no == 5) ){
			set_mod = 0;
		}
	}
	else{
		set_mod = (char)prm_get(COM_PRM, S_LTM, 2, 1, (char)(12-no));
		if( no == 6 ){
			set_mod = 0;
		}
	}
	return set_mod;
}

/*[]----------------------------------------------------------------------[]*/
/*| 種別毎の設定値取得 						       					       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Get_Pram_Syubet	                                       |*/
/*| PARAMETER    : no  (設定項目)                                          |*/
/*|              : syu (車種)                                              |*/
/*|              : len (桁数)                                              |*/
/*|              : pos (設定位置)※未使用の場合は０を指定する              |*/
/*| RETURN VALUE : param(設定値)                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| note     種別毎と全車種共通で設定が分かれている項目の設定値を取得する  |*/
/*|          全車種共通設定の場合ラグタイムアップ後の料金は設定を参照しない|*/
/*| Date         : 2010-09-07                                              |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
long Carkind_Param( char no, char syu, char len, char pos)
{
	long	param;
	short	sec,adr;
	
	adr = 0;
	sec = 0;
	syu -= 1;							// 種別は１から指定されてくるので-1する
	if(Get_Pram_Syubet(no)){			// 種別毎
		sec = S_LTM;					// セクション
		switch(no){
		case FLAP_UP_TIMER:				// フラップ上昇タイマー
			adr = 31+syu;				// アドレス
			break;
		case ROCK_CLOSE_TIMER:			// ロック閉タイマー
			adr = 11+syu;				// アドレス
			break;
		case FLP_ROCK_INTIME:			// ロック閉・フラップ上昇・ラグタイム内制御
			adr = 51+syu;				// アドレス
			break;
		case TYUU_DAISUU_MODE:			// 駐車台数管理方式
			adr = 91+syu;				// アドレス
			break;
		case IN_CHK_TIME:				// 入庫判定タイム
			adr = 111+syu;				// アドレス
			break;
		case LAG_PAY_PTN:				// ラグタイムアップ後の料金
			switch(syu){				// 種別毎のアドレスの指定
				case 0:
				case 1:
				case 2:
					adr = 71;
					break;
				case 3:
				case 4:
				case 5:
					adr = 72;
					break;
				case 6:
				case 7:
				case 8:
					adr = 73;
					break;
				case 9:
				case 10:
				case 11:
					adr = 74;
					break;
			}
			pos = (char)(6-((syu%3)*2)-1);	// 設定位置の算出
			break;
		case WARI_PAY_PTN:				// 時間割引/定期後の料金
			switch(syu){				// 種別毎のアドレスの指定
				case 0:
				case 1:
				case 2:
					adr = 81;
					break;
				case 3:
				case 4:
				case 5:
					adr = 82;
					break;
				case 6:
				case 7:
				case 8:
					adr = 83;
					break;
				case 9:
				case 10:
				case 11:
					adr = 84;
					break;
			}
			pos = (char)(6-((syu%3)*2)-1);	// 設定位置の算出
			break;
		}
	}
	else{								// 全車種共通
		switch(no){
		case FLAP_UP_TIMER:				// フラップ上昇タイマー
			sec	= S_TYP;
			adr = 118;
			break;
		case ROCK_CLOSE_TIMER:			// ロック閉タイマー
			sec	= S_TYP;
			adr = 69;
			break;
		case FLP_ROCK_INTIME:			// ロック閉・フラップ上昇・ラグタイム内制御
			sec	= S_TYP;
			adr = 68;
			break;
		case TYUU_DAISUU_MODE:			// 駐車台数管理方式
			sec = S_SYS;
			adr = 39;
			break;
		case IN_CHK_TIME:				// 入庫判定タイム
			sec = S_TYP;
			adr = 47;
			break;
		case LAG_PAY_PTN:				// ラグタイムアップ後の料金
			/* 全車種共通設定の場合ラグタイムアップ後の料金は基本からとるで固定とする */
			return 0;

		case WARI_PAY_PTN:				// 時間割引/定期後の料金
			sec = S_CAL;
			adr = 17;
			break;
		}
	}
	param = prm_get(COM_PRM,sec,adr,len,pos);
	return param;
}

// MH810100(S) Y.Watanabe 2020/02/26 車番チケットレス(買物割引対応)
//[]----------------------------------------------------------------------[]
///	@brief		割引の料金計算関連変数クリア
//[]----------------------------------------------------------------------[]
///	@param		none
///	@return		none
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void ClearRyoCalDiscInfo( void )
{
	struct REQ_RKN *rhs_p = NULL;
	ushort i, cnt = 0;

	// 料金計算の割引関連変数クリア
	tol_dis		 = 0;
	discount	 = 0;
	mae_dis		 = 0;
	discount_tyushi = 0;
	tyushi_mae_dis = 0;
	jik_dtm		 = 0;

	cnt = rhspnt;
	for( i=1; i<cnt; i++ ){
		rhs_p = &req_rhs[cnt - i];
// MH810100(S) 2020/06/02 【連動評価指摘事項】再精算時に、残額以上かつ駐車料金未満のQR割引を行うと請求額が不正な値となる(No.02-0014)
//		if( rhs_p->param == RY_TSA || rhs_p->param == RY_PKC ){
		if( rhs_p->param == RY_TSA || rhs_p->param == RY_PKC || rhs_p->param == RY_RWR ){
// MH810100(E) 2020/06/02 【連動評価指摘事項】再精算時に、残額以上かつ駐車料金未満のQR割引を行うと請求額が不正な値となる(No.02-0014)
			break;
		}
		memset( rhs_p, 0, sizeof(struct REQ_RKN) );
		rhspnt--;
	}
	tb_number = rhspnt;
}
// MH810100(E) Y.Watanabe 2020/02/26 車番チケットレス(買物割引対応)
// MH810100(S) 2020/06/10 #4216【連動評価指摘事項】車種設定種別時間割引設定時にQR割引(時間割引)を行っても適用されない
void IncCatintime( struct	CAR_TIM		*wok_tm )
{
	if(su_jik_dtm){
		ec70( wok_tm,(long)su_jik_dtm );
	}
	
}
// MH810100(E) 2020/06/10 #4216【連動評価指摘事項】車種設定種別時間割引設定時にQR割引(時間割引)を行っても適用されない
