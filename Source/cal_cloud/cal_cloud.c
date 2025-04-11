// GG124100(S) R.Endo 2022/09/14 車番チケットレス3.0 #6343 クラウド料金計算対応
//[]----------------------------------------------------------------------[]
///	@file	cal_cloud.c
///	@brief	クラウド料金計算処理
///	@date	2022/06/02
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]

#include <string.h>

#include "system.h"
#include "strdef.h"
#include "flp_def.h"
#include "mem_def.h"
#include "ntnet_def.h"
#include "ope_def.h"
#include "rkn_def.h"
#include "prm_tbl.h"
#include "tbl_rkn.h"

#include "cal_cloud.h"


extern void lcdbm_notice_dsp(ePOP_DISP_KIND kind, uchar DispStatus);

static void pay_result_error_alarm_check(ushort shPayResult);
static void fee_init();
// GG124100(S) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
static void season_init();
// GG124100(E) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
static void discount_init();
static void ryo_buf_n_set();
static void ryo_buf_n_calc();
static void ryo_buf_n_get(uchar seasonFlg);
static void ryo_buf_calc();

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 精算開始日時チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		ushort	: 指定日付(ノーマライズ)
///	@return			ret		: 精算開始日時チェック結果<br>
///							  -1 = 指定日付より前<br>
///							   0 = 指定日付と同じ<br>
///							   1 = 指定日付より後<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_pay_start_time_check(ushort ndatTarget)
{
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// 料金計算結果
	struct clk_rec *pTime;		// 精算開始日時
	ushort	nminSwitch;			// 日付切替時刻(ノーマライズ)

	// 精算開始日時を取得
	pTime = GetPayStartTime();

	// 日付切替時刻を取得
	nminSwitch = tnrmlz((short)0, (short)0, (short)pFeeCalc->DateSwitchHours, (short)pFeeCalc->DateSwitchMinute);

	// 精算開始時刻が日付切替時刻に達していない場合は指定日付を＋１日
	if ( pTime->nmin < nminSwitch ) {
		ndatTarget++;
	}

	// 精算開始日付と指定日付の比較
	if ( pTime->ndat < ndatTarget ) {
		return -1;	// 指定日付より前
	}
	if ( pTime->ndat > ndatTarget ) {
		return 1;	// 指定日付より後
	}

	return 0;	// 指定日付と同じ
}

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 初期化処理
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void cal_cloud_init()
{
	// 料金計算モード
	g_calcMode = (uchar)prm_get(COM_PRM, S_SYS, 79, 1, 1);
}

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 入庫情報チェック(料金)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 入庫情報チェック結果<br>
///							  0 = 料金設定へ<br>
///							  1 = 精算不可<br>
///							  2 = 精算済み<br>
// GG124100(S) R.Endo 2022/09/29 車番チケットレス3.0 #6614 料金計算結果が「43」の時のポップアップの内容が不正
///							  3 = 精算済み(中止)<br>
// GG124100(E) R.Endo 2022/09/29 車番チケットレス3.0 #6614 料金計算結果が「43」の時のポップアップの内容が不正
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_fee_check()
{
	stZaishaInfo_t *pZaisha = &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo;	// 在車情報
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// 料金計算結果

	// 料金計算結果エラーアラームチェック
	pay_result_error_alarm_check(pFeeCalc->shPayResult);

	// ラグタイム内精算のチェック
	FLAPDT.flp_data[(OPECTL.Pr_LokNo - 1)].lag_to_in.BIT.LAGIN = 0x00;
	switch( pFeeCalc->PaymentTimeType ) {
	case 0:		// 通常精算
	case 1:		// サービスタイム内精算
		if ( (pZaisha->PaymentType == 1) || (pZaisha->PaymentType == 2) ) {	// 精算/再精算
			if ( (pZaisha->dtPaymentDateTime.dtTimeYtoSec.byMonth > 0) &&
			   (pZaisha->dtPaymentDateTime.dtTimeYtoSec.byDay > 0) ) {	// 精算月日あり
				FLAPDT.flp_data[(OPECTL.Pr_LokNo - 1)].lag_to_in.BIT.LAGIN = 0x01;
			}
		}
		break;
	case 2:		// ラグタイム内精算
		return 2;	// 精算済み
	default:	// その他
		return 1;	// 精算不可
	}

	// 割引情報取得結果のチェック
	switch( pFeeCalc->CalcResult ) {
	case 0:		// 成功
	case 2:		// 処理なし
		break;
	case 1:		// 処理失敗(センター割引エンジンエラー)
	default:	// その他
		return 1;	// 精算不可
	}

	// 料金計算結果のチェック
	switch( pFeeCalc->shPayResult ) {
	case 0:		// 成功
	case 1001:	// 処理なし
		break;
// GG124100(S) R.Endo 2022/09/29 車番チケットレス3.0 #6614 料金計算結果が「43」の時のポップアップの内容が不正
	case 43:	// 料金計算不可(リペイタイム内の再精算は不可です)
		return 3;	// 精算済み(中止)
// GG124100(E) R.Endo 2022/09/29 車番チケットレス3.0 #6614 料金計算結果が「43」の時のポップアップの内容が不正
	default:	// その他
		return 1;	// 精算不可
	}

	return 0;	// 料金設定へ
}

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 料金設定
//[]----------------------------------------------------------------------[]
// GG124100(S) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
// ///	@param[in]		void
///	@param[in]		uchar	: 初回フラグ<br>
///							  0 = 二回目以降<br>
///							  1 = 初回<br>
// GG124100(E) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
///	@return			ret		: 料金設定結果<br>
///							  0 = 処理成功<br>
///							  1 = 処理失敗<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
// GG124100(S) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
// short cal_cloud_fee_set()
short cal_cloud_fee_set(uchar firstFlg)
// GG124100(E) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
{
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// 料金計算結果
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
	uchar	f_in_coin = 0;
	ushort	in_coin[5];
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）

	// 現在VLカードデータ
	vl_now = V_CHM;	// 駐車券 精算前

// GG124100(S) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
	// 料金種別
	syashu = pFeeCalc->FeeType;
// GG124100(E) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

// GG124100(S) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
// 	// 料金初期化
// 	fee_init();
	if ( firstFlg ) {	// 初回
		// 料金初期化
		fee_init();
	} else {			// 二回目以降
// GG129000(S) H.Fujinaga 2023/01/21 ゲート式車番チケットレスシステム対応（遠隔精算対応）
		if ( PayInfoChange_StateCheck() == 1 ){
			// 精算中変更データ受信
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
			if (ryo_buf.nyukin) {
				// 入金情報をバックアップ
				memcpy(in_coin, ryo_buf.in_coin, sizeof(ryo_buf.in_coin));
				f_in_coin = 1;
			}
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
			// 料金初期化
			fee_init();
// GG129000(S) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
			if (f_in_coin) {
				// 入金情報をリストア
				memcpy(ryo_buf.in_coin, in_coin, sizeof(ryo_buf.in_coin));
			}
// GG129000(E) T.Nagai 2023/10/02 ゲート式車番チケットレスシステム対応（遠隔精算対応）（精算中変更データ受信でRXMに問い合わせ）
		}else{
// GG129000(E) H.Fujinaga 2023/01/21 ゲート式車番チケットレスシステム対応（遠隔精算対応）
		// 定期初期化
		season_init();

		// 割引初期化
		discount_init();
// GG129000(S) H.Fujinaga 2023/01/21 ゲート式車番チケットレスシステム対応（遠隔精算対応）
		}
// GG129000(E) H.Fujinaga 2023/01/21 ゲート式車番チケットレスシステム対応（遠隔精算対応）
	}
// GG124100(E) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる

// GG124100(S) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
	// 車番情報
	PayData.CarSearchFlg = vl_car_no.CarSearchFlg;			// 車番/日時検索種別
	memcpy(PayData.CarSearchData, vl_car_no.CarSearchData,
		sizeof(PayData.CarSearchData));						// 車番/日時検索データ
	memcpy(PayData.CarNumber, vl_car_no.CarNumber,
		sizeof(PayData.CarNumber));							// センター問い合わせ時の車番
	memcpy(PayData.CarDataID, vl_car_no.CarDataID,
		sizeof(PayData.CarDataID));							// 車番データID(問合せに使用するID)
// GG124100(E) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

#if 1	// 参考：ec09(V_CHM)
	// 前回料金割引フラグ
	if ( ryo_buf.zenkai > 0 ) {
		PayData.PRTwari.BIT.RWARI = 1;	// 前回料金割引あり
	}
#endif	// 参考：ec09(V_CHM)

#if 1	// 参考：et02(et40)
	// 料金データバッファＮクリア
	memset(&ryo_buf_n, 0, sizeof(ryo_buf_n));

	// サービスタイム内精算
	if ( pFeeCalc->PaymentTimeType == 1 ) {
		// サービスタイム内精算フラグ
		ryo_buf.svs_tim = 1;	// サービスタイム内精算あり

		// 料金額
		ryo_buf_n.ryo = 0;
	} else {
		// 料金額
		ryo_buf_n.ryo = pFeeCalc->ParkingFee;	// 駐車料金
	}

	// 消費税額
	ryo_buf_n.tax = 0;	// 0％固定

	// 割引額
	ryo_buf_n.dis = ryo_buf.zenkai;	// 前回支払額(割引を除く)
#endif	// 参考：et02(et40)

	// 料金データバッファＮ計算
	ryo_buf_n_calc();

// GG124100(S) R.Endo 2022/09/07 車番チケットレス3.0 #6565 前回領収額がある状態で再精算すると、領収証の合計金額が不正になる
// #if 1	// 参考：et02(et43)
// 	// 前回料金割引額合計
// 	ryo_buf.dis_fee += ryo_buf_n.dis;		// 実割引を考慮した前回支払額
// 
// 	// 料金割引の合計金額(ポイントカードは除く)
// 	ryo_buf.fee_amount += ryo_buf.zenkai;	// 実割引を考慮しない前回支払額
// #endif	// 参考：et02(et43)
	// 前回支払額は割引額に含めるが、副作用で前回料金割引額合計等にも加算されていた。
	// 通常料金計算ではそれをOnlineDiscount(ope/opesub.c)でクリアしていたが、
	// クラウド料金計算では加算自体を行わないようにする。
// GG124100(E) R.Endo 2022/09/07 車番チケットレス3.0 #6565 前回領収額がある状態で再精算すると、領収証の合計金額が不正になる

	// 料金データバッファＮ取得
	ryo_buf_n_get(0);	// 通常

	// 料金データバッファ計算
	ryo_buf_calc();

#if 1	// 参考：op_mod02
	// 入金処理サブ：車番チケットレス精算(入庫時刻指定)
	in_mony(OPE_REQ_LCD_CALC_IN_TIME, 0);

	// 精算方法
	if ( lcdbm_rsp_in_car_info_main.shubetsu == 1 ) {	// 未精算出庫精算
		OpeNtnetAddedInfo.PayMethod = 13;	// 後日精算
// GG129000(S) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（遠隔精算対応）
	} else if ( lcdbm_rsp_in_car_info_main.shubetsu == 2 ) {	// 遠隔料金精算(精算中変更データ)
		OpeNtnetAddedInfo.PayMethod = 0;	// 券なし精算
// GG129000(E) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（遠隔精算対応）
	} else {											// 事前精算
// GG129000(S) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（QR読取画面対応）
//		OpeNtnetAddedInfo.PayMethod = 0;	// 券なし精算
		if ( lcdbm_rsp_in_car_info_main.kind == 2 ) {
			// QR検索
			OpeNtnetAddedInfo.PayMethod = 1;	// 駐車券精算
		} else {
			// 車番検索,日時検索
			OpeNtnetAddedInfo.PayMethod = 0;	// 券なし精算
		}
// GG129000(E) H.Fujinaga 2023/02/14 ゲート式車番チケットレスシステム対応（QR読取画面対応）
	}

	// 精算モード
	OpeNtnetAddedInfo.PayMode = 0;	// 自動精算

	// 処理区分
	switch ( lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo.PaymentType ) {
	case 1:	// 精算
	case 2:	// 再精算
	case 4:	// 再精算中止
		OpeNtnetAddedInfo.PayClass = 1;	// 再精算
		break;
	default:
// GG124100(S) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
		OpeNtnetAddedInfo.PayClass = 0;	// 精算
// GG124100(E) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
		break;
	}
#endif	// 参考：op_mod02

	return 0;	// 処理成功
}

// GG124100(S) R.Endo 2022/09/14 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 遠隔精算(入庫時刻指定) 料金設定
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 料金設定結果<br>
///							  0 = 処理成功<br>
///							  1 = 処理失敗<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_fee_set_remote_time()
{
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// 料金計算結果

	// 現在VLカードデータ
	vl_now = V_CHM;	// 駐車券 精算前

	// 料金種別
	syashu = pFeeCalc->FeeType;

	// 料金初期化
	fee_init();

#if 1	// 参考：et02(et40)
	// 料金データバッファＮクリア
	memset(&ryo_buf_n, 0, sizeof(ryo_buf_n));

	// 料金額
	ryo_buf_n.ryo = pFeeCalc->ParkingFee;	// 駐車料金

	// 消費税額
	ryo_buf_n.tax = 0;	// 0％固定
#endif	// 参考：et02(et40)

	// 料金データバッファＮ計算
	ryo_buf_n_calc();

	// 料金データバッファＮ取得
	ryo_buf_n_get(0);	// 通常

	// 料金データバッファ計算
	ryo_buf_calc();

#if 1	// 参考：op_mod02
	// 精算方法
	OpeNtnetAddedInfo.PayMethod = 0;	// 券なし精算

	// 精算モード
	OpeNtnetAddedInfo.PayMode = 4;	// 遠隔精算

	// 遠隔精算入庫時刻指定指定応答への設定用
	g_PipCtrl.stRemoteTime.RyoSyu = ryo_buf.syubet + 1;
	g_PipCtrl.stRemoteTime.Price = ryo_buf.tyu_ryo;
#endif	// 参考：op_mod02

	return 0;	// 処理成功
}

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 遠隔精算(金額指定) 料金設定
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 料金設定結果<br>
///							  0 = 処理成功<br>
///							  1 = 処理失敗<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_fee_set_remote_fee()
{
	// 現在VLカードデータ
	vl_now = V_FUN;	// 紛失券扱い

	// 料金初期化
	fee_init();

#if 1	// 参考：ec09(V_FUN)
	if ( (g_PipCtrl.stRemoteFee.RyoSyu >= 1) &&
		 (g_PipCtrl.stRemoteFee.RyoSyu <= 12) ) {	// 要求に種別あり
		syashu = (char)g_PipCtrl.stRemoteFee.RyoSyu;
	} else {
		syashu = (char)prm_get(COM_PRM, S_CEN, 40, 2, 1);
		if ( !syashu ) {
			syashu = 1;	// A種固定
		}
		g_PipCtrl.stRemoteFee.RyoSyu = syashu;
	}
#endif	// 参考：ec09(V_FUN)

#if 1	// 参考：et02(et48)
	// 料金データバッファＮクリア
	memset(&ryo_buf_n, 0, sizeof(ryo_buf_n));

	// 料金額
	ryo_buf_n.ryo = g_PipCtrl.stRemoteFee.Price;	// 駐車料金

	// 消費税額
	ryo_buf_n.tax = 0;	// 0％固定

	// 紛失料金は固定料金フラグ
	fun_kti = ON;

	// 出庫・入庫時刻不明フラグ
	ryo_buf.fumei_fg = 2;	// 入庫時刻不明扱い

	// 不明フラグ詳細理由
	ryo_buf.fumei_reason = fumei_LOST;	// 紛失精算扱い
#endif	// 参考：et02(et48)

	// 料金データバッファＮ計算
	ryo_buf_n_calc();

	// 料金データバッファＮ取得
	ryo_buf_n_get(0);	// 通常

	// 料金データバッファ計算
	ryo_buf_calc();

#if 1	// 参考：op_mod02
	// 精算方法
	if ( g_PipCtrl.stRemoteFee.Type == 1 ) {	// 後日精算
		OpeNtnetAddedInfo.PayMethod = 13;	// 後日精算
	} else {									// 出庫精算
		OpeNtnetAddedInfo.PayMethod = 0;	// 券なし精算
	}

	// 精算モード
	OpeNtnetAddedInfo.PayMode = 4;	// 遠隔精算
#endif	// 参考：op_mod02

	return 0;	// 処理成功
}
// GG124100(E) R.Endo 2022/09/14 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 定期チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 定期チェック結果<br>
///							  SEASON_CHK_OK = エラーなし<br>
///							  SEASON_CHK_INVALID_SEASONDATA = 無効定期エラー<br>
///							  SEASON_CHK_PRE_SALE_NG = 販売前定期エラー<br>
///							  SEASON_CHK_BEFORE_VALID = 期限前エラー<br>
///							  SEASON_CHK_AFTER_VALID = 期限切れエラー<br>
///							  SEASON_CHK_INVALID_PARKINGLOT = 駐車場№エラー<br>
///	@note	定期券アラーム情報等をチェックする処理。<br>
///			通常料金計算のCheckSeasonCardData(ope/opemain.c)に相当。<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
eSEASON_CHK_RESULT cal_cloud_season_check()
{
	stMasterInfo_t *pMaster = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo;	// マスター情報
	stZaishaInfo_t *pZaisha = &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo;	// 在車情報
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// 料金計算結果
	stParkKindNum_t *pCard = NULL;
	int i;
	stDatetTimeYtoSec_t processTime;
	ushort pass_id;
	uchar ascDat[ALM_LOG_ASC_DAT_SIZE];
	uchar typ;

	// カード検索
	for ( i = 0; i < ONL_MAX_CARDNUM; i++ ) {
		if( pMaster->stCardDataInfo[i].CardType == CARD_TYPE_PASS ) {	// 定期
			pCard = &pMaster->stCardDataInfo[i];
			break;
		}
	}
	if ( pCard == NULL ) {	// 定期が見つからない
		return SEASON_CHK_OK;	// エラーなし
	}

	// 初期化
	memset(&CRD_DAT.PAS, 0, sizeof(pas_rcc));

	// アラーム用情報設定
	memset(ascDat, 0x00, ALM_LOG_ASC_DAT_SIZE);
	memcpy(&ascDat[0], &pCard->ParkingLotNo, 4);
	pass_id = astoin(pCard->byCardNo, 5);
	memcpy(&ascDat[4], &pass_id, 2);

	// 定期券アラーム情報のチェック
	switch( pFeeCalc->PassAlarm ) {
	case 1:		// 無効定期
		alm_chk2(ALMMDL_SUB2, ALARM_GT_MUKOU_PASS_USE, 2, 1, 1, (void *)&ascDat);
		return SEASON_CHK_INVALID_SEASONDATA;	// 無効定期エラー
	case 2:		// 販売前定期
		return SEASON_CHK_PRE_SALE_NG;			// 販売前定期エラー
	case 3:		// 期限前定期
// GG129000(S) R.Endo 2023/01/16 車番チケットレス4.0 #6807 期限切れの定期の扱いを期限切れ受付する相当の動作に変更する（設定07-0005⑥）
// 		alm_chk2(ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&ascDat);
// 		return SEASON_CHK_BEFORE_VALID;			// 期限前エラー
		// 定期1～15 期限切れ受付設定(07-0005⑥,07-0015⑥,,,07-0145⑥)が
		// 「する(=1)」相当の動作とする為、期限前エラーを無視して受け付ける。
// GG129000(E) R.Endo 2023/01/16 車番チケットレス4.0 #6807 期限切れの定期の扱いを期限切れ受付する相当の動作に変更する（設定07-0005⑥）
	case 4:		// 期限切れ定期
// GG129000(S) R.Endo 2023/01/16 車番チケットレス4.0 #6807 期限切れの定期の扱いを期限切れ受付する相当の動作に変更する（設定07-0005⑥）
// 		alm_chk2(ALMMDL_SUB2, ALARM_GT_VALIDITY_TERM_OUT, 2, 1, 1, (void *)&ascDat);
// 		return SEASON_CHK_AFTER_VALID;			// 期限切れエラー
		// 定期1～15 期限切れ受付設定(07-0005⑥,07-0015⑥,,,07-0145⑥)が
		// 「する(=1)」相当の動作とする為、期限切れエラーを無視して受け付ける。
// GG129000(E) R.Endo 2023/01/16 車番チケットレス4.0 #6807 期限切れの定期の扱いを期限切れ受付する相当の動作に変更する（設定07-0005⑥）
	case 0:		// 正常
	default:	// その他
		break;
	}

	// 駐車場№のチェック
	for ( typ = 0; typ < 4; typ++ ) {
		if ( pCard->ParkingLotNo == prm_get(COM_PRM, S_SYS, (1 + typ), 6, 1) ) {
			break;
		}
	}
	if ( (pCard->ParkingLotNo == 0) ||	// 駐車場№が0
		 (typ >= 4) ) {					// 駐車場№が基本・拡張１～３の設定と一致しない
		alm_chk2( ALMMDL_SUB2, ALARM_GT_PARKING_NO_NG, 2, 1, 1, (void *)&ascDat);
		return SEASON_CHK_INVALID_PARKINGLOT;	// 駐車場№エラー
	}

	// 処理月日時分の設定
	memset(&processTime, 0, sizeof(processTime));
	switch ( pMaster->InOutStatus ) {
	case 1:	// 出庫中
		memcpy(&processTime, &pZaisha->dtPaymentDateTime.dtTimeYtoSec, sizeof(processTime));
		break;
	case 2:	// 入庫中
		memcpy(&processTime, &pZaisha->dtEntryDateTime.dtTimeYtoSec, sizeof(processTime));
		break;
	case 0:	// 初期状態
	default:
		break;
	}

	// Pass Kind
	CRD_DAT.PAS.knd = pMaster->SeasonKind;			// 定期種別

	// Pass Start Day (BIN)
	CRD_DAT.PAS.std = dnrmlzm(
		(short)pMaster->StartDate.shYear,			// 有効開始年
		(short)pMaster->StartDate.byMonth,			// 有効開始月
		(short)pMaster->StartDate.byDay);			// 有効開始日

	// Pass End Day (BIN)
	CRD_DAT.PAS.end = dnrmlzm(
		(short)pMaster->EndDate.shYear,				// 有効終了年
		(short)pMaster->EndDate.byMonth,			// 有効終了月
		(short)pMaster->EndDate.byDay);				// 有効終了日

	// [0]-[2] = Start Day, [3]-[5] = End Day
	CRD_DAT.PAS.std_end[0] = (char)(pMaster->StartDate.shYear % 100);	// 有効開始年
	CRD_DAT.PAS.std_end[1] = (char)pMaster->StartDate.byMonth;			// 有効開始月
	CRD_DAT.PAS.std_end[2] = (char)pMaster->StartDate.byDay;			// 有効開始日
	CRD_DAT.PAS.std_end[3] = (char)(pMaster->EndDate.shYear % 100);		// 有効終了年
	CRD_DAT.PAS.std_end[4] = (char)pMaster->EndDate.byMonth;			// 有効終了月
	CRD_DAT.PAS.std_end[5] = (char)pMaster->EndDate.byDay;				// 有効終了日

	// Parking No.
	CRD_DAT.PAS.pno = (long)pCard->ParkingLotNo;	// 駐車場№

	// GT Format Flag
	CRD_DAT.PAS.GT_flg = 1;

	// Personal Code
	CRD_DAT.PAS.cod = astoin(pCard->byCardNo, 5);	// カード番号

	// Status
	CRD_DAT.PAS.sts = (short)pMaster->InOutStatus;	// 入出庫ステータス

	// Tranzaction Day-Time
	CRD_DAT.PAS.trz[0] = processTime.byMonth;		// 処理月
	CRD_DAT.PAS.trz[1] = processTime.byDay;			// 処理日
	CRD_DAT.PAS.trz[2] = processTime.byHours;		// 処理時
	CRD_DAT.PAS.trz[3] = processTime.byMinute;		// 処理分

	// Pass Type
	CRD_DAT.PAS.typ = (char)typ;					// 定期区分

	return SEASON_CHK_OK;	// エラーなし
}

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 定期設定
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 定期設定結果<br>
///							  0 = 処理成功<br>
///							  1 = 処理失敗<br>
///	@note			前提条件：料金設定(cal_cloud_fee_set)を実施済み
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_season_set()
{
	stMasterInfo_t *pMaster = &lcdbm_rsp_in_car_info_main.crd_info.dtMasterInfo;	// マスター情報
// GG129000(S) R.Endo 2023/01/16 車番チケットレス4.0 #6807 期限切れの定期の扱いを期限切れ受付する相当の動作に変更する（設定07-0005⑥）
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// 料金計算結果
// GG129000(E) R.Endo 2023/01/16 車番チケットレス4.0 #6807 期限切れの定期の扱いを期限切れ受付する相当の動作に変更する（設定07-0005⑥）
	ushort ndatTemp;

	// 現在VLカードデータ
	vl_now = V_TSC;										// 定期券(駐車券併用有り)

	// 精算方法
	OpeNtnetAddedInfo.PayMethod = 2;					// 定期券精算

#if 1	// 参考：al_pasck_set
	// 定期券種別
	PayData.teiki.syu = (uchar)CRD_DAT.PAS.knd;			// 定期種別
	NTNET_Data152Save((void *)(&PayData.teiki.syu), NTNET_152_TEIKISYU);

	// 定期券ステータス(読取時)
	PayData.teiki.status = (uchar)CRD_DAT.PAS.sts;		// 入出庫ステータス

	// 定期券ID
	PayData.teiki.id = CRD_DAT.PAS.cod;					// カード番号
	NTNET_Data152Save((void *)(&PayData.teiki.id), NTNET_152_TEIKIID);

	// 定期券駐車場No.種別
	PayData.teiki.pkno_syu = (uchar)CRD_DAT.PAS.typ;	// 定期区分
	NTNET_Data152Save((void *)(&PayData.teiki.pkno_syu), NTNET_152_PKNOSYU);

	// 更新月数
	PayData.teiki.update_mon = 0;

	// 有効期限(開始:年)
	PayData.teiki.s_year = pMaster->StartDate.shYear;	// 有効開始年

	// 有効期限(開始:月)
	PayData.teiki.s_mon = pMaster->StartDate.byMonth;	// 有効開始月

	// 有効期限(開始:日)
	PayData.teiki.s_day = pMaster->StartDate.byDay;		// 有効開始日

	// 有効期限(終了:年)
	PayData.teiki.e_year = pMaster->EndDate.shYear;		// 有効終了年

	// 有効期限(終了:月)
	PayData.teiki.e_mon = pMaster->EndDate.byMonth;		// 有効終了月

	// 有効期限(終了:日)
	PayData.teiki.e_day = pMaster->EndDate.byDay;		// 有効終了日

	// 処理月日時分
	memcpy(PayData.teiki.t_tim, CRD_DAT.PAS.trz, 4);	// 処理月日時分

	// 定期更新精算時の更新結果
	PayData.teiki.update_rslt1 = 0;

	// 定期更新精算時のラベル発行結果
	PayData.teiki.update_rslt2 = 0;

	// 駐車場No.
	PayData.teiki.ParkingNo = (ulong)CRD_DAT.PAS.pno;
	NTNET_Data152Save((void *)(&PayData.teiki.ParkingNo), NTNET_152_PARKNOINPASS);

	// 【GT-4100未使用】定期券同一カードn分チェック
	PassPkNoBackup = 0;
	PassIdBackup = 0;
	PassIdBackupTim = 0;

	// カード毎使用枚数(定期券)
	card_use[USE_PAS] += 1;
#endif	// 参考：al_pasck_set

#if 1	// 参考：op_mod02
// GG129000(S) R.Endo 2023/01/16 車番チケットレス4.0 #6807 期限切れの定期の扱いを期限切れ受付する相当の動作に変更する（設定07-0005⑥）
	// 期限前・期限切れ定期を受け付けた場合は期限切れ間近チェックをしない
	if ( (pFeeCalc->PassAlarm != 3) &&	// 期限前定期
	     (pFeeCalc->PassAlarm != 4) ) {	// 期限切れ定期
// GG129000(E) R.Endo 2023/01/16 車番チケットレス4.0 #6807 期限切れの定期の扱いを期限切れ受付する相当の動作に変更する（設定07-0005⑥）

	// 定期期限切れ間近チェック
	ndatTemp = (ushort)prm_get(COM_PRM, S_TIK, 13, 2, 1);	// 定期券期限切れ予告
	if ( ndatTemp <= 15 ) {		// 期限切れn日前
		if ( ndatTemp == 0 ) {	// 期限切れ3日前
			ndatTemp = 3;
		}
		ndatTemp = CRD_DAT.PAS.end - ndatTemp;
		if ( cal_cloud_pay_start_time_check(ndatTemp) >= 0 ) {
			OPECTL.PassNearEnd = 1;	// 期限切れ間近
		}
	}

// GG129000(S) R.Endo 2023/01/16 車番チケットレス4.0 #6807 期限切れの定期の扱いを期限切れ受付する相当の動作に変更する（設定07-0005⑥）
	}
// GG129000(E) R.Endo 2023/01/16 車番チケットレス4.0 #6807 期限切れの定期の扱いを期限切れ受付する相当の動作に変更する（設定07-0005⑥）
#endif	// 参考：op_mod02

	// 料金データバッファＮ設定
	ryo_buf_n_set();

#if 1	// 参考：et02(et47)
	// クラウド料金計算では最初から定期を反映した駐車料金なので改めて設定しない
#endif	// 参考：et02(et47)

	// 料金データバッファＮ計算
	ryo_buf_n_calc();

	// 料金データバッファＮ取得
	ryo_buf_n_get(1);	// 定期設定

	// 料金データバッファ計算
	ryo_buf_calc();

#if 1	// 参考：op_mod02
	// オペレーションフェーズ
	if ( OPECTL.op_faz == 0 ) {	// 入金可送信
		OPECTL.op_faz = 1;	// 入金中
	}

	// 精算サイクルフラグ
	ac_flg.cycl_fg = 10;	// 入金

	// 入金処理サブ：車番チケットレス精算(入庫時刻指定)
	in_mony(OPE_REQ_LCD_CALC_IN_TIME, 0);
#endif	// 参考：op_mod02

	return 0;	// 処理成功
}

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 QRデータチェック
//[]----------------------------------------------------------------------[]
///	@param[in]		tMediaDetail	: QRデータ
///	@param[in]		QR_YMDData		: QR日付構造体
///	@return			ret		: QRデータチェック結果<br>
///							  RESULT_NO_ERROR = 使用可能<br>
///							  RESULT_DISCOUNT_TIME_MAX = 割引上限<br>
///							  RESULT_BAR_EXPIRED = 有効期限外<br>
///							  RESULT_BAR_ID_ERR = 対象外<br>
///	@note			QR日付構造体はope_CanUseBarcode_subで期限変更テーブル<br>
///					(08-0008～)を考慮した日付を入れている。
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
OPE_RESULT cal_cloud_qrdata_check(tMediaDetail *pMedia, QR_YMDData* pYmdData)
{
	QR_DiscountInfo *pDisc = &pMedia->Barcode.QR_data.DiscountType;	// QRデータ情報 QR割引券
	int i;
	ushort ndatStart = 0;		// 有効開始日(無期限)
	ushort ndatEnd = 0xffff;	// 有効終了日(無期限)
	ushort dayTemp;
	uchar foundFlag = 0;		// 見つからない
// GG129000(S) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
	uchar kaimonoCount = 0;		// 買物割引なし

	// 買物割引を検索
	for ( i = 0; i < CardUseSyu; i++ ) {
		if ( (PayData.DiscountData[i].DiscSyu == NTNET_SHOP_DISC_AMT) ||	// 買物割引(金額)
			 (PayData.DiscountData[i].DiscSyu == NTNET_SHOP_DISC_TIME) ) {	// 買物割引(時間)
			kaimonoCount += 1;	// 1精算で1つしかセットされないため見つけたら加算して抜ける
			break;
		}
	}
// GG129000(E) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する

	switch ( pMedia->Barcode.id ) {
	case BAR_ID_DISCOUNT:	// QR割引券
		// QR割引券：割引種別(端末の制限)
		switch ( pDisc->DiscKind ) {
		case NTNET_SVS_M:			// サービス券割引(金額)
		case NTNET_SVS_T:			// サービス券割引(時間)
			// サービス券：QR割引券割引区分の値域(端末の制限)
			if ( !rangechk(1, SVS_MAX, pDisc->DiscClass) ) {
				return RESULT_BAR_ID_ERR;	// 対象外
			}

			// 同種の割引を検索
			for ( i = 0; i < CardUseSyu; i++ ) {
				if (   (PayData.DiscountData[i].ParkingNo == pDisc->ParkingNo)	// 駐車場№
					&& (PayData.DiscountData[i].DiscSyu   == pDisc->DiscKind)	// 割引種別
					&& (PayData.DiscountData[i].DiscNo    == pDisc->DiscClass)	// サービス券種別
					&& (PayData.DiscountData[i].DiscInfo1 == pDisc->ShopNp)		// 掛売り先
				) {
					foundFlag = 1;	// 見つかった
					break;
				}
			}

			break;
		case NTNET_KAK_M:			// 店割引(金額)
		case NTNET_KAK_T:			// 店割引(時間)
		case NTNET_TKAK_M:			// 多店舗割引(金額)
		case NTNET_TKAK_T:			// 多店舗割引(時間)
			// 同種の割引を検索
			for ( i = 0; i < CardUseSyu; i++ ) {
				if (   (PayData.DiscountData[i].ParkingNo == pDisc->ParkingNo)	// 駐車場№
					&& (PayData.DiscountData[i].DiscSyu   == pDisc->DiscKind)	// 割引種別
					&& (PayData.DiscountData[i].DiscNo    == pDisc->ShopNp)		// 店№
					&& (PayData.DiscountData[i].DiscInfo1 == pDisc->DiscClass)	// 割引種類/割引種別
				) {
					foundFlag = 1;	// 見つかった
					break;
				}
			}
			break;
		default:
			return RESULT_BAR_ID_ERR;	// 対象外
		}

		// QR割引券：同一割引券種数(端末の制限)
// GG129000(S) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
// 		if ( CardUseSyu >= WTIK_USEMAX ) {	// 同一割引券種数が最大値以上
// 			// 同種の割引が見つからない場合は割引上限とする
// 			if ( !foundFlag ) {
// 				return RESULT_DISCOUNT_TIME_MAX;	// 割引上限
// 			}
		if ( (CardUseSyu - kaimonoCount - foundFlag) >= (WTIK_USEMAX - 1) ) {	// 同種と買物割引を除いた券種数が既に10種以上
			return RESULT_DISCOUNT_TIME_MAX;	// 割引上限
// GG129000(E) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
		}

		// 有効期限を設定
		if ( (pYmdData->StartDate.Year != 0) ||
			 (pYmdData->StartDate.Mon  != 0) ||
			 (pYmdData->StartDate.Day  != 0) ||
			 (pYmdData->EndDate.Year != 99) ||
			 (pYmdData->EndDate.Mon  != 99) ||
			 (pYmdData->EndDate.Day  != 99) ) {		// 有効期限あり
			ndatStart = dnrmlzm(
				(short)pYmdData->StartDate.Year,
				(short)pYmdData->StartDate.Mon,
				(short)pYmdData->StartDate.Day);	// 有効開始日
			ndatEnd = dnrmlzm(
				(short)pYmdData->EndDate.Year,
				(short)pYmdData->EndDate.Mon,
				(short)pYmdData->EndDate.Day);		// 有効終了日
		}

		break;
	case BAR_ID_AMOUNT:	// QR買上券
// GG129000(S) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
		// QR買上券：買物金額合計を検索
		for ( i = 0; i < CardUseSyu; i++ ) {
			if ( PayData.DiscountData[i].DiscSyu == NTNET_KAIMONO_GOUKEI ) {	// 割引種別 買物金額合計
				foundFlag = 1;	// 見つかった
				break;
			}
		}

		// QR買上券：同一割引券種数(端末の制限)
		if ( (CardUseSyu - kaimonoCount - foundFlag) >= (WTIK_USEMAX - 1) ) {	// 同種と買物割引を除いた券種数が既に10種以上
			return RESULT_DISCOUNT_TIME_MAX;	// 割引上限
		}
// GG129000(E) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する

		// 有効期限を設定
		ndatStart = dnrmlzm(
			(short)pYmdData->IssueDate.Year,
			(short)pYmdData->IssueDate.Mon,
			(short)pYmdData->IssueDate.Day);	// 有効開始日
		dayTemp = (ushort)prm_get(COM_PRM, S_SYS, 77, 2, 1);	// 有効期間
		if ( dayTemp == 0 ) {
			dayTemp = 99;
		}
		ndatEnd = ndatStart + dayTemp - 1;

		break;
	}

	// QR有効期限
	if ( (cal_cloud_pay_start_time_check(ndatStart) < 0) ||	// 有効開始日前
		 (cal_cloud_pay_start_time_check(ndatEnd) > 0) ) {	// 有効終了日後
		return RESULT_BAR_EXPIRED;	// 有効期限外
	}

	return RESULT_NO_ERROR;	// 使用可能
}

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 入庫情報チェック(割引)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 入庫情報チェック結果<br>
///							  0 = 割引設定へ<br>
///							  1 = 入庫情報NG<br>
///							  2 = 入庫情報NG(QR取消なし)<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_discount_check()
{
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// 料金計算結果

	// 料金計算結果エラーアラームチェック
	pay_result_error_alarm_check(pFeeCalc->shPayResult);

	// 割引重複利用チェックのチェック
	switch( lcdbm_rsp_in_car_info_main.crd_info.ValidCHK_Dup ) {
	case 0:		// チェックなし
	case 1:		// 割引重複なし
		break;
	default:	// その他
		return 2;	// 入庫情報NG(QR取消なし)
	}

	// 割引情報取得結果のチェック
	switch( pFeeCalc->CalcResult ) {
	case 0:		// 成功
	case 2:		// 処理なし
		break;
	case 1:		// 処理失敗(センター割引エンジンエラー)
	default:	// その他
		lcdbm_notice_dsp(POP_UNABLE_DISCOUNT, 6);	// 6 = 割引適用失敗
		return 1;	// 入庫情報NG
	}

	// 料金計算結果のチェック(ポップアップと結果)
	switch( pFeeCalc->shPayResult ) {
	case 0:		// 成功
	case 1001:	// 処理なし
		break;
	case 64:	// 割引関連エラー(未対応の割引が含まれています)
		lcdbm_notice_dsp(POP_UNABLE_DISCOUNT, 1);	// 1 = 割引種別NG
		return 1;	// 入庫情報NG
	case 65:	// 割引関連エラー(割引の使用枚数の合計が全割引使用限度個数を超過しています)
	case 66:	// 割引関連エラー(サービス券の使用枚数が使用制限枚数を超過しています)
		lcdbm_notice_dsp(POP_UNABLE_DISCOUNT, 2);	// 2 = 枚数NG
		return 1;	// 入庫情報NG
	case 62:	// 割引関連エラー(利用不可車種に設定されている為、サービス券が利用できません)
	case 63:	// 割引関連エラー(2枚目以降の%割引は利用できません)
		lcdbm_notice_dsp(POP_UNABLE_DISCOUNT, 3);	// 3 = カード区分NG
		return 1;	// 入庫情報NG
	case 61:	// 割引関連エラー(初回精算以外は種別切替は利用不可)
	case 67:	// 割引関連エラー(再精算で%割引は新たに利用できません)
		lcdbm_notice_dsp(POP_UNABLE_DISCOUNT, 4);	// 4 = 車種切替不可
		return 1;	// 入庫情報NG
	default:	// その他
		lcdbm_notice_dsp(POP_UNABLE_DISCOUNT, 6);	// 6 = 割引適用失敗
		return 1;	// 入庫情報NG
	}

	return 0;	// 割引設定へ
}

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 割引設定
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 割引設定結果<br>
///							  0 = 処理成功<br>
///							  1 = 処理失敗<br>
///	@note			前提条件：料金設定(cal_cloud_fee_set)を実施済み
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_discount_set()
{
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// 料金計算結果
	ulong discAmountTemp;
	int i;
	uchar countTemp;
	uchar addFlag;
	uchar j;

// GG124100(S) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
// 	// 割引初期化
// 	discount_init();
// GG124100(E) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる

	for ( i = 0; i < ONL_MAX_DISC_NUM; i++ ) {
		if ( !(pFeeCalc->stDiscountInfo[i].DiscParkNo) ) {
			break;
		}

		// サービス券・掛売券の枚数
		switch ( pFeeCalc->stDiscountInfo[i].DiscSyu ) {
		case NTNET_SVS_M:			// サービス券割引(金額)
		case NTNET_SVS_T:			// サービス券割引(時間)
		case NTNET_KAK_M:			// 店割引(金額)
		case NTNET_KAK_T:			// 店割引(時間)
		case NTNET_TKAK_M:			// 多店舗割引(金額)
		case NTNET_TKAK_T:			// 多店舗割引(時間)
		case NTNET_SHOP_DISC_AMT:	// 買物割引(金額)
		case NTNET_SHOP_DISC_TIME:	// 買物割引(時間)
			countTemp = pFeeCalc->stDiscountInfo[i].DiscCount;
			break;
		default:
			countTemp = 0;
			break;
		}

		// カード毎使用枚数テーブル(サービス券・掛売券)
		card_use[USE_SVC] += countTemp;

		// カード毎使用枚数テーブル(新規サービス券・掛売券)
		if ( pFeeCalc->stDiscountInfo[i].DiscStatus < 2 ) {
			card_use[USE_N_SVC] += countTemp;
		}

		// 現在VLカードデータ
		vl_now = V_SAK;	// サービス券

		// 料金データバッファＮ設定
		ryo_buf_n_set();

#if 1	// 参考：et02(et45/et51)
		// 割引金額
		discAmountTemp  = pFeeCalc->stDiscountInfo[i].DiscAmount;
		discAmountTemp += pFeeCalc->stDiscountInfo[i].PrevUsageDiscAmount;

		switch ( pFeeCalc->stDiscountInfo[i].DiscSyu ) {
		case NTNET_SYUBET_TIME:		// 種別割引(時間)
		case NTNET_SYUBET:			// 種別割引(金額)
			// 料金額から引く
			if ( ryo_buf_n.ryo < discAmountTemp ) {
				ryo_buf_n.ryo = 0;
			} else {
				ryo_buf_n.ryo -= discAmountTemp;
			}
			break;
// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6549 買い物金額合計(2001)の「合計金額」の情報を駐車料金の割引に適用している
// 		default:
		case NTNET_SVS_M:			// サービス券割引(金額)
		case NTNET_KAK_M:			// 店割引(金額)
		case NTNET_TKAK_M:			// 多店舗割引(金額)
		case NTNET_SHOP_DISC_AMT:	// 買物割引(金額)
		case NTNET_SVS_T:			// サービス券割引(時間)
		case NTNET_KAK_T:			// 店割引(時間)
		case NTNET_TKAK_T:			// 多店舗割引(時間)
		case NTNET_SHOP_DISC_TIME:	// 買物割引(時間)
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6549 買い物金額合計(2001)の「合計金額」の情報を駐車料金の割引に適用している
			// 割引額に足す
			ryo_buf_n.dis += discAmountTemp;
			break;
// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6549 買い物金額合計(2001)の「合計金額」の情報を駐車料金の割引に適用している
		default:
			break;
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6549 買い物金額合計(2001)の「合計金額」の情報を駐車料金の割引に適用している
		}

		switch ( pFeeCalc->stDiscountInfo[i].DiscSyu ) {
		case NTNET_SVS_M:			// サービス券割引(金額)
		case NTNET_KAK_M:			// 店割引(金額)
		case NTNET_TKAK_M:			// 多店舗割引(金額)
		case NTNET_SHOP_DISC_AMT:	// 買物割引(金額)
		case NTNET_SYUBET:			// 種別割引(金額)
			// 料金割引の合計金額(ポイントカードは除く)
			ryo_buf.fee_amount += discAmountTemp;
			break;
		case NTNET_SVS_T:			// サービス券割引(時間)
		case NTNET_KAK_T:			// 店割引(時間)
		case NTNET_TKAK_T:			// 多店舗割引(時間)
		case NTNET_SHOP_DISC_TIME:	// 買物割引(時間)
		case NTNET_SYUBET_TIME:		// 種別割引(時間)
			// 時間割引の合計金額(ポイントカードは除く)
			ryo_buf.tim_amount += pFeeCalc->stDiscountInfo[i].DiscTime;
			ryo_buf.tim_amount += pFeeCalc->stDiscountInfo[i].PrevUsageDiscTime;
			break;
// GG129000(S) R.Endo 2024/01/12 #7217 お買物合計表示対応
		case NTNET_KAIMONO_GOUKEI:	// 買物金額合計
			// 買物の合計金額
			ryo_buf.shopping_total += pFeeCalc->stDiscountInfo[i].DiscAmountSetting;
			break;
// GG129000(E) R.Endo 2024/01/12 #7217 お買物合計表示対応
// GG129004(S) M.Fujikawa 2024/12/11 買物金額割引情報対応
		case NTNET_KAIMONO_INFO:	// 買物情報
			if(pFeeCalc->stDiscountInfo[i].DiscInfo == 0){
				ryo_buf.shopping_info = 0xFFFFFFFF;
			}else{ 
				ryo_buf.shopping_info = pFeeCalc->stDiscountInfo[i].DiscInfo;
			}
			break;
// GG129004(E) M.Fujikawa 2024/12/11 買物金額割引情報対応
		default:
			break;
		}
#endif	// 参考：et02(et45/et51)

		// 料金データバッファＮ計算
		ryo_buf_n_calc();

		// 料金データバッファＮ取得
		ryo_buf_n_get(0);	// 通常

#if 1	// 参考：ryo_cal
		// 割引追加フラグをクリア
		addFlag = 0;

		switch ( pFeeCalc->stDiscountInfo[i].DiscSyu ) {
		case NTNET_SVS_M:			// サービス券割引(金額)
		case NTNET_SVS_T:			// サービス券割引(時間)
			// 同種の割引はまとめる
			for ( j = 0; j < CardUseSyu; j++ ) {
				if (   (PayData.DiscountData[j].ParkingNo == pFeeCalc->stDiscountInfo[i].DiscParkNo)	// 駐車場№
					&& (PayData.DiscountData[j].DiscSyu   == pFeeCalc->stDiscountInfo[i].DiscSyu)		// 割引種別
					&& (PayData.DiscountData[j].DiscNo    == pFeeCalc->stDiscountInfo[i].DiscCardNo)	// サービス券種別
					&& (PayData.DiscountData[j].DiscInfo1 == pFeeCalc->stDiscountInfo[i].DiscInfo)		// 掛売り先
				) {
					// 割引を更新
					addFlag = 1;

					break;
				}
			}

			// 同種の割引が見つからない場合は追加する
			if ( (j >= CardUseSyu) && (CardUseSyu < WTIK_USEMAX) ) {
				// 割引を追加
				addFlag = 2;
			}

			break;
		case NTNET_KAK_M:			// 店割引(金額)
		case NTNET_KAK_T:			// 店割引(時間)
		case NTNET_TKAK_M:			// 多店舗割引(金額)
		case NTNET_TKAK_T:			// 多店舗割引(時間)
			// 同種の割引はまとめる
			for ( j = 0; j < CardUseSyu; j++ ) {
				if (   (PayData.DiscountData[j].ParkingNo  == pFeeCalc->stDiscountInfo[i].DiscParkNo)	// 駐車場№
					&& (PayData.DiscountData[j].DiscSyu    == pFeeCalc->stDiscountInfo[i].DiscSyu)		// 割引種別
					&& (PayData.DiscountData[j].DiscNo     == pFeeCalc->stDiscountInfo[i].DiscCardNo)	// 店№
					&& (PayData.DiscountData[j].DiscInfo1  == pFeeCalc->stDiscountInfo[i].DiscInfo)		// 割引種類/割引種別
				) {
					// 割引を更新
					addFlag = 1;

					break;
				}
			}

			// 同種の割引が見つからない場合は追加する
			if ( (j >= CardUseSyu) && (CardUseSyu < WTIK_USEMAX) ) {
				// 割引を追加
				addFlag = 2;
			}

			break;
		case NTNET_SHOP_DISC_AMT:	// 買物割引(金額)
		case NTNET_SHOP_DISC_TIME:	// 買物割引(時間)
			// 割引を追加(買物割引は1つしかセットしない)
			addFlag = 2;

			break;
		case NTNET_SYUBET:			// 種別割引(金額)
		case NTNET_SYUBET_TIME:		// 種別割引(時間)
			// 割引を追加(種別割引は1つしかセットしない)
			addFlag = 2;

// GG129000(S) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
// 			// 種別割引料金
// 			PayData.SyuWariRyo += pFeeCalc->stDiscountInfo[i].DiscAmount;	// 今回の割引金額(実割)
// GG129000(E) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する

			break;
// GG124100(S) R.Endo 2022/08/04 車番チケットレス3.0 #6125 【PK事業部要求】NT-NET精算データの割引情報に2001(買物金額合計)をセット
		case NTNET_KAIMONO_GOUKEI:	// 買物金額合計
			// 買物金額合計を追加(買物金額合計は1つしかセットしない)
			addFlag = 3;

			break;
// GG124100(E) R.Endo 2022/08/04 車番チケットレス3.0 #6125 【PK事業部要求】NT-NET精算データの割引情報に2001(買物金額合計)をセット
		default:
			break;
		}

		// NT-NET精算データ用のデータを設定
		switch ( addFlag ) {
		case 1:	// 割引を更新
			// 割引時間数
			switch ( pFeeCalc->stDiscountInfo[i].DiscSyu ) {
			case NTNET_SVS_T:			// サービス券割引(時間)
			case NTNET_KAK_T:			// 店割引(時間)
			case NTNET_TKAK_T:			// 多店舗割引(時間)
			case NTNET_SHOP_DISC_TIME:	// 買物割引(時間)
			case NTNET_SYUBET_TIME:		// 種別割引(時間)
// GG129000(S) R.Endo 2022/12/14 車番チケットレス4.0 #6758 同一種別の時間割引を再精算時も利用した場合、NT-NET精算データの割引情報2にセットする割引時間の値が不正
// 				PayData.DiscountData[j].uDiscData.common.DiscInfo2 +=
// 					pFeeCalc->stDiscountInfo[i].DiscTimeSetting;
				if ( pFeeCalc->stDiscountInfo[i].DiscStatus != 2 ) {	// 未割引
					PayData.DiscountData[j].uDiscData.common.DiscInfo2 +=
						pFeeCalc->stDiscountInfo[i].DiscTimeSetting;
				}
// GG129000(E) R.Endo 2022/12/14 車番チケットレス4.0 #6758 同一種別の時間割引を再精算時も利用した場合、NT-NET精算データの割引情報2にセットする割引時間の値が不正
				break;
			default:
				break;
			}

			// 今回使用した割引金額
			PayData.DiscountData[j].Discount +=
				pFeeCalc->stDiscountInfo[i].DiscAmount;

			// 前回精算までの使用済み割引金額
// GG129000(S) R.Endo 2022/12/08 車番チケットレス4.0 #6755 PayDataの使用済み割引の配列インデックスの変数間違い
// 			PayData.DiscountData[jik_dtm].uDiscData.common.PrevUsageDiscount +=
			PayData.DiscountData[j].uDiscData.common.PrevUsageDiscount +=
// GG129000(E) R.Endo 2022/12/08 車番チケットレス4.0 #6755 PayDataの使用済み割引の配列インデックスの変数間違い
				pFeeCalc->stDiscountInfo[i].PrevUsageDiscAmount;

			if ( pFeeCalc->stDiscountInfo[i].DiscStatus == 2 ) {	// 割引済み
				// 今回使用した前回精算までの割引金額
				PayData.DiscountData[j].uDiscData.common.PrevDiscount +=
					pFeeCalc->stDiscountInfo[i].DiscAmount;

				// 前回精算までの使用済み枚数
				PayData.DiscountData[j].uDiscData.common.PrevUsageDiscCount +=
					pFeeCalc->stDiscountInfo[i].DiscCount;
			} else {												// 未割引
				// 今回使用した枚数
				PayData.DiscountData[j].DiscCount +=
					pFeeCalc->stDiscountInfo[i].DiscCount;
			}

			// NT-NET精算情報データ用保存領域に保存
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[j], NTNET_152_DCOUNT, j);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[j], NTNET_152_DISCOUNT, j);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[j], NTNET_152_DINFO2, j);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[j], NTNET_152_PREVDISCOUNT, j);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[j], NTNET_152_PREVUSAGEDISCOUNT, j);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[j], NTNET_152_PREVUSAGEDCOUNT, j);

			break;
		case 2:	// 割引を追加
			// 格納できる件数を超えた場合は格納しない
			if ( CardUseSyu >= WTIK_USEMAX ) {
				break;
			}

			// 駐車場№
			PayData.DiscountData[CardUseSyu].ParkingNo =
				pFeeCalc->stDiscountInfo[i].DiscParkNo;

			// 割引種別
			PayData.DiscountData[CardUseSyu].DiscSyu =
				pFeeCalc->stDiscountInfo[i].DiscSyu;

			// 割引区分
			PayData.DiscountData[CardUseSyu].DiscNo =
				pFeeCalc->stDiscountInfo[i].DiscCardNo;

			// 今回使用した割引金額
			PayData.DiscountData[CardUseSyu].Discount =
				pFeeCalc->stDiscountInfo[i].DiscAmount;

			// 割引情報1
			PayData.DiscountData[CardUseSyu].DiscInfo1 =
				(ulong)pFeeCalc->stDiscountInfo[i].DiscInfo;

			// 割引情報2
			switch ( pFeeCalc->stDiscountInfo[i].DiscSyu ) {
			case NTNET_SVS_T:			// サービス券割引(時間)
			case NTNET_KAK_T:			// 店割引(時間)
			case NTNET_TKAK_T:			// 多店舗割引(時間)
			case NTNET_SHOP_DISC_TIME:	// 買物割引(時間)
			case NTNET_SYUBET_TIME:		// 種別割引(時間)
// GG129000(S) R.Endo 2022/12/14 車番チケットレス4.0 #6758 同一種別の時間割引を再精算時も利用した場合、NT-NET精算データの割引情報2にセットする割引時間の値が不正
// 				PayData.DiscountData[CardUseSyu].uDiscData.common.DiscInfo2 =
// 					pFeeCalc->stDiscountInfo[i].DiscTimeSetting;
				if ( pFeeCalc->stDiscountInfo[i].DiscStatus != 2 ) {	// 未割引
					PayData.DiscountData[CardUseSyu].uDiscData.common.DiscInfo2 =
						pFeeCalc->stDiscountInfo[i].DiscTimeSetting;
				}
// GG129000(E) R.Endo 2022/12/14 車番チケットレス4.0 #6758 同一種別の時間割引を再精算時も利用した場合、NT-NET精算データの割引情報2にセットする割引時間の値が不正
				break;
			default:
				break;
			}

			// 前回精算までの使用済み割引金額
			PayData.DiscountData[CardUseSyu].uDiscData.common.PrevUsageDiscount =
				pFeeCalc->stDiscountInfo[i].PrevUsageDiscAmount;

			if ( pFeeCalc->stDiscountInfo[i].DiscStatus == 2 ) {	// 割引済み
				// 今回使用した前回精算までの割引金額
				PayData.DiscountData[CardUseSyu].uDiscData.common.PrevDiscount =
					pFeeCalc->stDiscountInfo[i].DiscAmount;

				// 前回精算までの使用済み枚数
				PayData.DiscountData[CardUseSyu].uDiscData.common.PrevUsageDiscCount =
					pFeeCalc->stDiscountInfo[i].DiscCount;
			} else {												// 未割引
				// 今回使用した枚数
				PayData.DiscountData[CardUseSyu].DiscCount =
					pFeeCalc->stDiscountInfo[i].DiscCount;
			}

// GG124100(S) R.Endo 2022/08/19 車番チケットレス3.0 #6520 使用したサービス券の情報が領収証に印字されない
			// 種別切換先車種
			PayData.DiscountData[CardUseSyu].uDiscData.common.FeeKindSwitchSetting =
				pFeeCalc->stDiscountInfo[i].FeeKindSwitchSetting;

			// 割引役割
			if ( pFeeCalc->stDiscountInfo[i].DiscTimeSetting ) {			// 時間割引
				PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 1;
			} else if ( pFeeCalc->stDiscountInfo[i].DiscAmountSetting ) {	// 料金割引
				PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 2;
			} else if ( pFeeCalc->stDiscountInfo[i].DiscPercentSetting ) {	// ％割引
				PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 3;
			} else {														// 役割なし
// GG124100(S) R.Endo 2022/09/13 車番チケットレス3.0 #6589 QR割引券の全額割引を使用すると領収証に割引情報が印字されない
// 				PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 0;
				// 全額割引対応
				if ( (pFeeCalc->stDiscountInfo[i].DiscAmount > 0) ||				// 今回の割引金額(実割)あり
					 (pFeeCalc->stDiscountInfo[i].FeeKindSwitchSetting == 0) ) {	// 種別切換先車種なし
					PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 4;
				} else {
					PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 0;
				}
// GG124100(E) R.Endo 2022/09/13 車番チケットレス3.0 #6589 QR割引券の全額割引を使用すると領収証に割引情報が印字されない
			}
// GG124100(E) R.Endo 2022/08/19 車番チケットレス3.0 #6520 使用したサービス券の情報が領収証に印字されない

			// NT-NET精算情報データ用保存領域に保存
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DPARKINGNO, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DSYU, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DNO, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DCOUNT, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DISCOUNT, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DINFO1, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DINFO2, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_PREVDISCOUNT, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_PREVUSAGEDISCOUNT, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_PREVUSAGEDCOUNT, CardUseSyu);
// GG124100(S) R.Endo 2022/08/19 車番チケットレス3.0 #6520 使用したサービス券の情報が領収証に印字されない
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_FEEKINDSWITCHSETTING, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DROLE, CardUseSyu);
// GG124100(E) R.Endo 2022/08/19 車番チケットレス3.0 #6520 使用したサービス券の情報が領収証に印字されない

			// 割引数を加算
			CardUseSyu++;

			break;
// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6125 【PK事業部要求】NT-NET精算データの割引情報に2001(買物金額合計)をセット
// 		case 0:	// 割引が更新も追加もされていない場合
		case 3:	// 買物金額合計を追加
			// 格納できる件数を超えた場合は格納しない
			if ( CardUseSyu >= WTIK_USEMAX ) {
				break;
			}

			// 駐車場№
			PayData.DiscountData[CardUseSyu].ParkingNo =
				pFeeCalc->stDiscountInfo[i].DiscParkNo;

			// 割引種別
			PayData.DiscountData[CardUseSyu].DiscSyu =
				pFeeCalc->stDiscountInfo[i].DiscSyu;


			// 今回使用した割引金額(合計金額)
// GG124100(S) R.Endo 2022/09/14 車番チケットレス3.0 #6568 リアルタイム精算データ/NT-NET精算データの買物金額合計の「割引額」に値がセットされない
// 			PayData.DiscountData[CardUseSyu].Discount =
// 				pFeeCalc->stDiscountInfo[i].DiscAmount;
			PayData.DiscountData[CardUseSyu].Discount =
				pFeeCalc->stDiscountInfo[i].DiscAmountSetting;
// GG124100(E) R.Endo 2022/09/14 車番チケットレス3.0 #6568 リアルタイム精算データ/NT-NET精算データの買物金額合計の「割引額」に値がセットされない

			// 今回使用した枚数
			PayData.DiscountData[CardUseSyu].DiscCount =
				pFeeCalc->stDiscountInfo[i].DiscCount;

			// NT-NET精算情報データ用保存領域に保存
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DPARKINGNO, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DSYU, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DCOUNT, CardUseSyu);
			NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DISCOUNT, CardUseSyu);

			// 割引数を加算
			CardUseSyu++;

			break;
		case 0:	// 追加も更新もない場合
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6125 【PK事業部要求】NT-NET精算データの割引情報に2001(買物金額合計)をセット
		default:
			break;
		}
#endif	// 参考：ryo_cal

		// 料金データバッファ計算
		ryo_buf_calc();

#if 1	// 参考：DiscountForOnline
		// オペレーションフェーズ
		if ( OPECTL.op_faz == 0 ) {	// 入金可送信
			OPECTL.op_faz = 1;	// 入金中
		}

		// 精算サイクルフラグ
		ac_flg.cycl_fg = 10;	// 入金

		// 入金処理サブ：車番チケットレス精算(入庫時刻指定)
		in_mony(OPE_REQ_LCD_CALC_IN_TIME, 0);
#endif	// 参考：DiscountForOnline

#if 1	// 参考：OnlineDiscount
// GG124100(S) R.Endo 2022/09/14 車番チケットレス3.0 #6568 リアルタイム精算データ/NT-NET精算データの買物金額合計の「割引額」に値がセットされない
		// リアルタイム精算データ用のデータを設定
		switch ( addFlag ) {
		case 1:	// 割引を更新
		case 2:	// 割引を追加
// GG124100(E) R.Endo 2022/09/14 車番チケットレス3.0 #6568 リアルタイム精算データ/NT-NET精算データの買物金額合計の「割引額」に値がセットされない
			// 駐車場№
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscParkNo =
				pFeeCalc->stDiscountInfo[i].DiscParkNo;

			// 種別
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscSyu =
				pFeeCalc->stDiscountInfo[i].DiscSyu;

			// カード区分
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscCardNo =
				pFeeCalc->stDiscountInfo[i].DiscCardNo;

			// 区分
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscNo =
				pFeeCalc->stDiscountInfo[i].DiscNo;

			// 枚数
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscCount =
				pFeeCalc->stDiscountInfo[i].DiscCount;

			// 割引情報
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscInfo =
				pFeeCalc->stDiscountInfo[i].DiscInfo;

			// 対応カード種別
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscCorrType =
				pFeeCalc->stDiscountInfo[i].DiscCorrType;

			// ステータス
			if ( pFeeCalc->stDiscountInfo[i].DiscStatus == 0 ) {	// 未割引
				RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscStatus = 1;	// 今回割引(精算完了で割引済とする)
			} else {												// 未割引以外
				RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscStatus =
					pFeeCalc->stDiscountInfo[i].DiscStatus;
			}

			// 割引状況
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscFlg =
				pFeeCalc->stDiscountInfo[i].DiscFlg;

			// 割引額
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].Discount =
				pFeeCalc->stDiscountInfo[i].DiscAmount +
				pFeeCalc->stDiscountInfo[i].PrevUsageDiscAmount;

// GG124100(S) R.Endo 2022/09/14 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)
			// 時間
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscTime =
				pFeeCalc->stDiscountInfo[i].DiscTime +
				pFeeCalc->stDiscountInfo[i].PrevUsageDiscTime;
// GG124100(E) R.Endo 2022/09/14 車番チケットレス3.0 #6343 クラウド料金計算対応(カード情報Rev.No.11)

// GG124100(S) R.Endo 2022/09/14 車番チケットレス3.0 #6568 リアルタイム精算データ/NT-NET精算データの買物金額合計の「割引額」に値がセットされない
			break;
		case 3:	// 買物金額合計を追加
			// 駐車場№
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscParkNo =
				pFeeCalc->stDiscountInfo[i].DiscParkNo;

			// 種別
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscSyu =
				pFeeCalc->stDiscountInfo[i].DiscSyu;

			// カード区分
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscCardNo =
				pFeeCalc->stDiscountInfo[i].DiscCardNo;

			// 区分
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscNo =
				pFeeCalc->stDiscountInfo[i].DiscNo;

			// 枚数
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscCount =
				pFeeCalc->stDiscountInfo[i].DiscCount;

			// 割引情報
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscInfo =
				pFeeCalc->stDiscountInfo[i].DiscInfo;

			// 対応カード種別
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscCorrType =
				pFeeCalc->stDiscountInfo[i].DiscCorrType;

			// ステータス
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscStatus =
				pFeeCalc->stDiscountInfo[i].DiscStatus;

			// 割引状況
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].DiscFlg =
				pFeeCalc->stDiscountInfo[i].DiscFlg;

			// 割引額
			RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo[i].Discount =
				pFeeCalc->stDiscountInfo[i].DiscAmountSetting;

			break;
		case 0:	// 追加も更新もない場合
		default:
			break;
		}
// GG124100(E) R.Endo 2022/09/14 車番チケットレス3.0 #6568 リアルタイム精算データ/NT-NET精算データの買物金額合計の「割引額」に値がセットされない
// GG129004(S) R.Endo 2024/12/10 電子領収証対応
		// RT領収証データ用のデータを設定
		switch ( addFlag ) {
		case 1:	// 割引を更新
		case 2:	// 割引を追加
			// 割引 駐車場№
			RTReceipt_Data.receipt_info.arrstDiscount[i].ParkingLotNo[0] =
				((pFeeCalc->stDiscountInfo[i].DiscParkNo >> 16) & 0x000000FF);
			RTReceipt_Data.receipt_info.arrstDiscount[i].ParkingLotNo[1] =
				((pFeeCalc->stDiscountInfo[i].DiscParkNo >> 8) & 0x000000FF);
			RTReceipt_Data.receipt_info.arrstDiscount[i].ParkingLotNo[2] =
				 (pFeeCalc->stDiscountInfo[i].DiscParkNo & 0x000000FF);

			// 割引 種別
			RTReceipt_Data.receipt_info.arrstDiscount[i].Kind =
				pFeeCalc->stDiscountInfo[i].DiscSyu;

			// 割引 区分
			RTReceipt_Data.receipt_info.arrstDiscount[i].CardType =
				pFeeCalc->stDiscountInfo[i].DiscCardNo;

			if ( pFeeCalc->stDiscountInfo[i].DiscStatus == 2 ) {	// 割引済み
				// 割引 使用回数
				RTReceipt_Data.receipt_info.arrstDiscount[i].UsageCount = 0;

				// 割引 前回使用回数
				RTReceipt_Data.receipt_info.arrstDiscount[i].PrevUsageCount =
					pFeeCalc->stDiscountInfo[i].DiscCount;
			} else {
				// 割引 使用回数
				RTReceipt_Data.receipt_info.arrstDiscount[i].UsageCount =
					pFeeCalc->stDiscountInfo[i].DiscCount;

				// 割引 前回使用回数
				RTReceipt_Data.receipt_info.arrstDiscount[i].PrevUsageCount = 0;
			}

			// 割引 割引額
			RTReceipt_Data.receipt_info.arrstDiscount[i].DisAmount =
				pFeeCalc->stDiscountInfo[i].DiscAmount;

			// 割引 前回割引額
			RTReceipt_Data.receipt_info.arrstDiscount[i].PrevUsageDisAmount =
				pFeeCalc->stDiscountInfo[i].PrevUsageDiscAmount;

			// 割引 種別切換情報
			RTReceipt_Data.receipt_info.arrstDiscount[i].FeeKindSwitchSetting =
				pFeeCalc->stDiscountInfo[i].FeeKindSwitchSetting;

			// 割引 割引情報1
			RTReceipt_Data.receipt_info.arrstDiscount[i].DisType =
				pFeeCalc->stDiscountInfo[i].DiscInfo;

			// 割引 割引情報2
			RTReceipt_Data.receipt_info.arrstDiscount[i].DisType2 =
				pFeeCalc->stDiscountInfo[i].DiscTime +
				pFeeCalc->stDiscountInfo[i].PrevUsageDiscTime;

			// 割引 適格請求書 課税対象
			RTReceipt_Data.receipt_info.arrstDiscount[i].EligibleInvoiceTaxable = 0;

			break;
		case 3:	// 買物金額合計を追加
			// 割引 駐車場№
			RTReceipt_Data.receipt_info.arrstDiscount[i].ParkingLotNo[0] =
				((pFeeCalc->stDiscountInfo[i].DiscParkNo >> 16) & 0x000000FF);
			RTReceipt_Data.receipt_info.arrstDiscount[i].ParkingLotNo[1] =
				((pFeeCalc->stDiscountInfo[i].DiscParkNo >> 8) & 0x000000FF);
			RTReceipt_Data.receipt_info.arrstDiscount[i].ParkingLotNo[2] =
				 (pFeeCalc->stDiscountInfo[i].DiscParkNo & 0x000000FF);

			// 割引 種別
			RTReceipt_Data.receipt_info.arrstDiscount[i].Kind =
				pFeeCalc->stDiscountInfo[i].DiscSyu;

			// 割引 区分
			RTReceipt_Data.receipt_info.arrstDiscount[i].CardType =
				pFeeCalc->stDiscountInfo[i].DiscCardNo;

			if ( pFeeCalc->stDiscountInfo[i].DiscStatus == 2 ) {	// 割引済み
				// 割引 使用回数
				RTReceipt_Data.receipt_info.arrstDiscount[i].UsageCount = 0;

				// 割引 前回使用回数
				RTReceipt_Data.receipt_info.arrstDiscount[i].PrevUsageCount =
					pFeeCalc->stDiscountInfo[i].DiscCount;
			} else {
				// 割引 使用回数
				RTReceipt_Data.receipt_info.arrstDiscount[i].UsageCount =
					pFeeCalc->stDiscountInfo[i].DiscCount;

				// 割引 前回使用回数
				RTReceipt_Data.receipt_info.arrstDiscount[i].PrevUsageCount = 0;
			}

			// 割引 割引額
			RTReceipt_Data.receipt_info.arrstDiscount[i].DisAmount =
				pFeeCalc->stDiscountInfo[i].DiscAmount;

			// 割引 前回割引額
			RTReceipt_Data.receipt_info.arrstDiscount[i].PrevUsageDisAmount =
				pFeeCalc->stDiscountInfo[i].PrevUsageDiscAmount;

			// 割引 種別切換情報
			RTReceipt_Data.receipt_info.arrstDiscount[i].FeeKindSwitchSetting =
				pFeeCalc->stDiscountInfo[i].FeeKindSwitchSetting;

			// 割引 割引情報1
			RTReceipt_Data.receipt_info.arrstDiscount[i].DisType =
				pFeeCalc->stDiscountInfo[i].DiscInfo;

			// 割引 割引情報2
			RTReceipt_Data.receipt_info.arrstDiscount[i].DisType2 = 0;

			// 割引 適格請求書 課税対象
			RTReceipt_Data.receipt_info.arrstDiscount[i].EligibleInvoiceTaxable = 0;

			break;
		case 0:	// 追加も更新もない場合
		default:
			break;
		}
// GG129004(E) R.Endo 2024/12/10 電子領収証対応
#endif	// 参考：OnlineDiscount
	}

	return 0;
}

// GG124100(S) R.Endo 2022/09/14 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 遠隔精算(入庫時刻指定) 割引設定
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 料金設定結果<br>
///							  0 = 処理成功<br>
///							  1 = 処理失敗<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_discount_set_remote_time()
{
	// 現在VLカードデータ
	vl_now = V_DIS_TIM;	// 遠隔精算中割引(入庫時刻指定)

	// 料金データバッファＮ設定
	ryo_buf_n_set();

#if 1	// 参考：et02(et95)
	// 請求額を超える場合は請求額を割引金額とする
	if ( g_PipCtrl.stRemoteTime.Discount > ryo_buf_n.require ) {
		g_PipCtrl.stRemoteTime.Discount = ryo_buf_n.require;
	}

	// 割引額
	ryo_buf_n.dis = g_PipCtrl.stRemoteTime.Discount;
#endif	// 参考：et02(et95)

	// 料金データバッファＮ計算
	ryo_buf_n_calc();

	// 料金データバッファＮ取得
	ryo_buf_n_get(0);	// 通常

#if 1	// 参考：ryo_cal
	// 格納できる件数を超えた場合は格納しない
	if ( CardUseSyu < WTIK_USEMAX ) {
		// 駐車場№
		PayData.DiscountData[CardUseSyu].ParkingNo =
			(ulong)prm_get(COM_PRM, S_SYS, 1, 6, 1);

		// 割引種別
		PayData.DiscountData[CardUseSyu].DiscSyu =
			g_PipCtrl.stRemoteTime.DiscountKind;

		// 割引区分
		PayData.DiscountData[CardUseSyu].DiscNo =
			g_PipCtrl.stRemoteTime.DiscountType;

		// 今回使用した割引金額
		PayData.DiscountData[CardUseSyu].Discount =
			ryo_buf.waribik;

		// 割引情報1
		PayData.DiscountData[CardUseSyu].DiscInfo1 =
			g_PipCtrl.stRemoteTime.DiscountInfo1;

		// 割引情報2
		PayData.DiscountData[CardUseSyu].uDiscData.common.DiscInfo2 =
			g_PipCtrl.stRemoteTime.DiscountInfo2;

		// 今回使用した枚数
		PayData.DiscountData[CardUseSyu].DiscCount =
			(uchar)g_PipCtrl.stRemoteTime.DiscountCnt;

		// 割引役割
		switch ( g_PipCtrl.stRemoteTime.DiscountKind ) {
		case NTNET_SVS_M:			// サービス券割引(金額)
		case NTNET_KAK_M:			// 店割引(金額)
		case NTNET_TKAK_M:			// 多店舗割引(金額)
		case NTNET_SHOP_DISC_AMT:	// 買物割引(金額)
		case NTNET_SYUBET:			// 種別割引(金額)
			PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 1;	// 時間割引
			break;
		case NTNET_SVS_T:			// サービス券割引(時間)
		case NTNET_KAK_T:			// 店割引(時間)
		case NTNET_TKAK_T:			// 多店舗割引(時間)
		case NTNET_SHOP_DISC_TIME:	// 買物割引(時間)
		case NTNET_SYUBET_TIME:		// 種別割引(時間)
			PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 2;	// 料金割引
			break;
		default:
			PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 0;	// 役割なし
			break;
		}

		// NT-NET精算情報データ用保存領域に保存
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DPARKINGNO, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DSYU, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DNO, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DCOUNT, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DISCOUNT, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DINFO1, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DINFO2, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DROLE, CardUseSyu);

		// 割引数を加算
		CardUseSyu++;
	}
#endif	// 参考：ryo_cal

	// 料金データバッファ計算
	ryo_buf_calc();

#if 1	// 参考：op_mod02
	// 入金処理サブ：遠隔精算時刻指定精算要求
	in_mony(OPE_REQ_REMOTE_CALC_TIME, 0);
#endif	// 参考：op_mod02

	return 0;	// 処理成功
}

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 遠隔精算(金額指定) 割引設定
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 料金設定結果<br>
///							  0 = 処理成功<br>
///							  1 = 処理失敗<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_discount_set_remote_fee()
{
	// 現在VLカードデータ
	vl_now = V_DIS_FEE;	// 遠隔精算中割引(金額指定)

	// 料金データバッファＮ設定
	ryo_buf_n_set();

#if 1	// 参考：et02(et95)
	// 請求額を超える場合は請求額を割引金額とする
	if ( g_PipCtrl.stRemoteFee.Discount > ryo_buf_n.require ) {
		g_PipCtrl.stRemoteFee.Discount = ryo_buf_n.require;
	}

	// 割引額
	ryo_buf_n.dis = g_PipCtrl.stRemoteFee.Discount;
#endif	// 参考：et02(et95)

	// 料金データバッファＮ計算
	ryo_buf_n_calc();

	// 料金データバッファＮ取得
	ryo_buf_n_get(0);	// 通常

#if 1	// 参考：ryo_cal
	// 格納できる件数を超えた場合は格納しない
	if ( CardUseSyu < WTIK_USEMAX ) {
		// 駐車場№
		PayData.DiscountData[CardUseSyu].ParkingNo =
			(ulong)prm_get(COM_PRM, S_SYS, 1, 6, 1);

		// 割引種別
		PayData.DiscountData[CardUseSyu].DiscSyu =
			g_PipCtrl.stRemoteFee.DiscountKind;

		// 割引区分
		PayData.DiscountData[CardUseSyu].DiscNo =
			g_PipCtrl.stRemoteFee.DiscountType;

		// 今回使用した割引金額
		PayData.DiscountData[CardUseSyu].Discount =
			ryo_buf.waribik;

		// 割引情報1
		PayData.DiscountData[CardUseSyu].DiscInfo1 =
			g_PipCtrl.stRemoteFee.DiscountInfo1;

		// 割引情報2
		PayData.DiscountData[CardUseSyu].uDiscData.common.DiscInfo2 =
			g_PipCtrl.stRemoteFee.DiscountInfo2;

		// 今回使用した枚数
		PayData.DiscountData[CardUseSyu].DiscCount =
			(uchar)g_PipCtrl.stRemoteFee.DiscountCnt;

		// 割引役割
		switch ( g_PipCtrl.stRemoteFee.DiscountKind ) {
		case NTNET_SVS_M:			// サービス券割引(金額)
		case NTNET_KAK_M:			// 店割引(金額)
		case NTNET_TKAK_M:			// 多店舗割引(金額)
		case NTNET_SHOP_DISC_AMT:	// 買物割引(金額)
		case NTNET_SYUBET:			// 種別割引(金額)
			PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 1;	// 時間割引
			break;
		case NTNET_SVS_T:			// サービス券割引(時間)
		case NTNET_KAK_T:			// 店割引(時間)
		case NTNET_TKAK_T:			// 多店舗割引(時間)
		case NTNET_SHOP_DISC_TIME:	// 買物割引(時間)
		case NTNET_SYUBET_TIME:		// 種別割引(時間)
			PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 2;	// 料金割引
			break;
		default:
			PayData.DiscountData[CardUseSyu].uDiscData.common.DiscRole = 0;	// 役割なし
			break;
		}

		// NT-NET精算情報データ用保存領域に保存
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DPARKINGNO, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DSYU, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DNO, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DCOUNT, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DISCOUNT, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DINFO1, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DINFO2, CardUseSyu);
		NTNET_Data152_DiscDataSave((void *)&PayData.DiscountData[CardUseSyu], NTNET_152_DROLE, CardUseSyu);

		// 割引数を加算
		CardUseSyu++;
	}
#endif	// 参考：ryo_cal

	// 料金データバッファ計算
	ryo_buf_calc();

#if 1	// 参考：op_mod02
	// 入金処理サブ：遠隔精算金額指定精算要求
	in_mony(OPE_REQ_REMOTE_CALC_FEE, 0);
#endif	// 参考：op_mod02

	return 0;	// 処理成功
}
// GG124100(E) R.Endo 2022/09/14 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 電子マネー設定
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 定期設定結果<br>
///							  0 = 処理成功<br>
///							  1 = 処理失敗<br>
///	@note			前提条件：料金設定(cal_cloud_fee_set)を実施済み
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_emoney_set()
{
	// 現在VLカードデータ
	vl_now = V_EMY;	// 電子マネー

	// 料金データバッファＮ設定
	ryo_buf_n_set();

#if 1	// 参考：et02(et94)
	// 電子マネー入金額計
	ryo_buf.emonyin += e_pay;

	// 電子マネー使用回数
	e_incnt++;

	// 請求額
	ryo_buf_n.require = 0;
#endif	// 参考：et02(et94)

// GG124100(S) R.Endo 2022/09/14 車番チケットレス3.0 #6573 VP6800で駐車料金を精算時、精算画面の「精算残高・状態表示エリア」が「あと　0円」にならない
// 	// 料金データバッファＮ計算
// 	ryo_buf_n_calc();
	// 電子マネーはryo_buf_n_calc関数にあたる処理を実施しない。
// GG124100(E) R.Endo 2022/09/14 車番チケットレス3.0 #6573 VP6800で駐車料金を精算時、精算画面の「精算残高・状態表示エリア」が「あと　0円」にならない

	// 料金データバッファＮ取得
	ryo_buf_n_get(0);	// 通常

	// 料金データバッファ計算
	ryo_buf_calc();

	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 QRコード決済設定
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 定期設定結果<br>
///							  0 = 処理成功<br>
///							  1 = 処理失敗<br>
///	@note			前提条件：料金設定(cal_cloud_fee_set)を実施済み
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_eqr_set()
{
	// 現在VLカードデータ
	vl_now = V_EQR;	// QRコード決済

	// 料金データバッファＮ設定
	ryo_buf_n_set();

#if 1	// 参考：et02(et94)
	// 電子マネー入金額計
	ryo_buf.emonyin += e_pay;

	// 電子マネー使用回数
	e_incnt++;

	// 請求額
	ryo_buf_n.require = 0;
#endif	// 参考：et02(et94)

// GG124100(S) R.Endo 2022/09/14 車番チケットレス3.0 #6573 VP6800で駐車料金を精算時、精算画面の「精算残高・状態表示エリア」が「あと　0円」にならない
// 	// 料金データバッファＮ計算
// 	ryo_buf_n_calc();
	// QRコード決済はryo_buf_n_calc関数にあたる処理を実施しない。
// GG124100(E) R.Endo 2022/09/14 車番チケットレス3.0 #6573 VP6800で駐車料金を精算時、精算画面の「精算残高・状態表示エリア」が「あと　0円」にならない

	// 料金データバッファＮ取得
	ryo_buf_n_get(0);	// 通常

	// 料金データバッファ計算
	ryo_buf_calc();

	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 クレジットカード設定
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 定期設定結果<br>
///							  0 = 処理成功<br>
///							  1 = 処理失敗<br>
///	@note			前提条件：料金設定(cal_cloud_fee_set)を実施済み
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
short cal_cloud_credit_set()
{
	// 現在VLカードデータ
	vl_now = V_CRE;	// クレジットカード

	// 料金データバッファＮ設定
	ryo_buf_n_set();

#if 1	// 参考：et02(et90)
	// 請求額
	ryo_buf_n.require = 0;
#endif	// 参考：et02(et90)

// GG124100(S) R.Endo 2022/09/14 車番チケットレス3.0 #6573 VP6800で駐車料金を精算時、精算画面の「精算残高・状態表示エリア」が「あと　0円」にならない
// 	// 料金データバッファＮ計算
// 	ryo_buf_n_calc();
	// クレジットはryo_buf_n_calc関数にあたる処理を実施しない。
// GG124100(E) R.Endo 2022/09/14 車番チケットレス3.0 #6573 VP6800で駐車料金を精算時、精算画面の「精算残高・状態表示エリア」が「あと　0円」にならない

	// 料金データバッファＮ取得
	ryo_buf_n_get(0);	// 通常

	// 料金データバッファ計算
	ryo_buf_calc();

	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 料金計算結果エラーアラームチェック
//[]----------------------------------------------------------------------[]
///	@param[in]		uchar	: 料金計算結果<br>
///	@return			void
///	@note	料金計算結果をチェックし、エラーアラームを登録する。<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void pay_result_error_alarm_check(ushort shPayResult)
{
	switch( shPayResult ) {
	case 1:		// 料金計算設定パラメータ不正(料金設定情報が見つかりません)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_PAY_SET_ERR, 2);			// 料金設定異常
		break;
	case 2:		// 料金計算設定パラメータ不正(定期券情報が見つかりません)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_PASS_SET_ERR, 2);			// 定期券設定異常
		break;
	case 3:		// 料金計算設定パラメータ不正(料金パターン情報が見つかりません)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_PAYPATTERN_SET_ERR, 2);		// 料金パターン設定異常
		break;
	case 4:		// 料金計算設定パラメータ不正(駐車時間最大料金情報が見つかりません)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_MAXCHARGE_SET_ERR, 2);		// 駐車時間最大料金設定異常
		break;
	case 5:		// 料金計算設定パラメータ不正(時間帯情報が見つかりません
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_TIMEZONE_SET_ERR, 2);		// 時間帯設定異常
		break;
	case 6:		// 料金計算設定パラメータ不正(単価時間が0分のため、計算できません)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_UNITTIME_SET_ERR, 2);		// 単位時間設定異常
		break;
	case 7:		// 料金計算設定パラメータ不正(サービス券情報が見つかりません)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_SVS_SET_ERR, 2);			// サービス券設定異常
		break;
	case 8:		// 料金計算設定パラメータ不正(店割引情報が見つかりません)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_KAK_SET_ERR, 2);			// 店割引設定異常
		break;
	case 9:		// 料金計算設定パラメータ不正(多店舗割引情報が見つかりません)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_TKAK_SET_ERR, 2);			// 多店舗割引設定異常
		break;
	case 10:	// 料金計算設定パラメータ不正(PMCサービス券情報が見つかりません)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_PMCSVS_SET_ERR, 2);			// PMCサービス券設定異常
		break;
	case 11:	// 料金計算設定パラメータ不正(車種割引情報が見つかりません)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_SYUBET_SET_ERR, 2);			// 車種割引設定異常
		break;
	case 12:	// 料金計算設定パラメータ不正(割引券情報が見つかりません
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_WRI_SET_ERR, 2);			// 割引券設定異常
		break;
	case 21:	// 料金計算要求データ不正(精算日時が無効データです)
		ex_errlg(ERRMDL_CAL_CLOUD, ERR_CC_PAYDATE_INVALID, 2, 0);		// 精算日時無効
		break;
	case 22:	// 料金計算要求データ不正([初回精算]入庫日時が無効データです)
		ex_errlg(ERRMDL_CAL_CLOUD, ERR_CC_ENTRYDATE_INVALID, 2, 0);		// 入庫日時無効（初回）
		break;
	case 23:	// 料金計算要求データ不正([再精算][入庫から再計算]入庫日時が無効データです)
		ex_errlg(ERRMDL_CAL_CLOUD, ERR_CC_ENTRYDATE_INVALID_OLD, 2, 0);	// 入庫日時無効（再精算）
		break;
	case 24:	// 料金計算要求データ不正([再精算][前回精算から計算]前回精算日時が無効データです)
		ex_errlg(ERRMDL_CAL_CLOUD, ERR_CC_PAYDATE_INVALID_OLD, 2, 0);	// 前回精算日時無効（再精算）
		break;
	case 41:	// 料金計算不可(入庫日時からの駐車時間が{0}年を超えています)
		ex_errlg(ERRMDL_CAL_CLOUD, ERR_CC_OVER1YEARCAL, 2, 0);			// 1年以上料金計算（入庫）
		break;
	case 42:	// 料金計算不可(前回精算日時からの駐車時間が{0}年を超えています)
		ex_errlg(ERRMDL_CAL_CLOUD, ERR_CC_OVER1YEARCAL_OLD, 2, 0);		// 1年以上料金計算（前回精算）
		break;
	case 64:	// 割引関連エラー(未対応の割引が含まれています)
		alm_chk(ALMMDL_CAL_CLOUD, ALMMDL_CC_UNSUPPORTED_DISC_USE, 2);	// 未対応割引使用
		break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 料金初期化
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@note	1精算毎の情報を初期化する処理。<br>
///			通常料金計算のryo_cal(cal/ryo_cal.c)の一部(※)に相当。<br>
///			※ryo_calの先頭。詳細範囲はryo_cal側のコメント参照。<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void fee_init()
{
	stZaishaInfo_t *pZaisha = &lcdbm_rsp_in_car_info_main.crd_info.dtZaishaInfo;	// 在車情報
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// 料金計算結果
	int i;

	// 初期化
	memset(&ryo_buf, 0, sizeof(ryo_buf));						// 料金データバッファ：RYO_BUF
	memset(&PayData, 0, sizeof(PayData));						// 精算情報,領収証印字データ：Receipt_data
	memset(&ntNet_56_SaveData, 0, sizeof(ntNet_56_SaveData));	// NT-NET精算データ用保存領域：ntNet_56_saveInf
	NTNET_Data152_SaveDataClear();								// NT-NET精算情報データ用保存領域：ntNet_152_saveInf
	memset(&OpeNtnetAddedInfo, 0, sizeof(OpeNtnetAddedInfo));	// NT-NET精算データ付加情報：t_OpeNtnetAddedInfo
	memset(card_use, 0, sizeof(card_use));						// カード毎使用枚数テーブル：uchar

	// GT-4100未使用
	memset(&PPC_Data_Detail, 0, sizeof(PPC_Data_Detail));		// プリペイドカードデータ詳細：t_PPC_DATA_DETAIL
	c_pay = 0;													// プリペイドカード使用額：long
	ntnet_nmax_flg = 0;											// 最大料金越えありフラグ：char
	memset(&PayData_Sub, 0, sizeof(PayData_Sub));				// 印字用詳細中止エリア：Receipt_Data_Sub
	discount_tyushi = 0;										// 割引額(精算中止機能強化)：long
	Flap_Sub_Num = 0;											// 詳細中止エリア番号：uchar

	// クラウド料金計算未使用
	memset(&req_rkn, 0, sizeof(req_rkn));						// 料金要求テーブル：REQ_RKN
	memset(req_rhs, 0, sizeof(req_rhs));						// 現在計算後格納域：REQ_RKN
	tb_number = 0;												// 計算結果格納テーブル通し番号：short
	rhspnt = 0;													// 現在格納数：char
	memset(req_crd, 0, sizeof(req_crd));						// 駐車券読み取り時券データ要求格納バッファ：REQ_CRD
	req_crd_cnt = 0;											// 駐車券読み取り時券データ要求件数：char
	memset(req_tkc, 0, sizeof(req_tkc));						// 定期券中止精算要求格納バッファ：REQ_TKC
	req_tkc_cnt = 0;											// 定期券中止精算要求格納件数：char

// GG124100(S) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
	// 定期初期化
	season_init();
// GG124100(E) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる

	// 割引初期化
	discount_init();

	// 精算した駐車位置番号
	ryo_buf.pkiti = OPECTL.Pr_LokNo;	// 内部処理用駐車位置番号

	// ラグタイムオーバー判定
	if ( FLAPDT.flp_data[(OPECTL.Pr_LokNo - 1)].lag_to_in.BIT.LAGIN == ON ) {
		ryo_buf.lag_tim_over = 1;	// ラグタイムオーバー
	}

// GG129000(S) H.Fujinaga 2023/01/21 ゲート式車番チケットレスシステム対応（遠隔精算対応）
//	// ラグタイムオーバー
//	if ( ryo_buf.lag_tim_over == 1 ) {
	// ラグタイムオーバー or 精算中変更データ受信時
	if ( ryo_buf.lag_tim_over == 1 || PayInfoChange_StateCheck() == 1 ) {
// GG129000(E) H.Fujinaga 2023/01/21 ゲート式車番チケットレスシステム対応（遠隔精算対応）
		// 前回支払額(割引を除く)
		ryo_buf.zenkai = pZaisha->GenkinFee;
		for ( i = 0; i < ONL_MAX_SETTLEMENT; i++ ) {
			if ( pZaisha->stSettlement[i].CargeType ) {	// 決済区分あり
				ryo_buf.zenkai += pZaisha->stSettlement[i].CargeFee;	// 決済金額
			}
		}

		// 前回割引額(支払額を除く)
		for ( i = 0; i < ONL_MAX_DISC_NUM; i++ ) {
			if ( !(pFeeCalc->stDiscountInfo[i].DiscParkNo) ) {
				break;
			}
			ryo_buf.zenkaiWari += pFeeCalc->stDiscountInfo[i].PrevUsageDiscAmount;	// 前回精算までの使用済み割引金額(実割)
		}
// GG129000(S) 改善連絡No.69,No.73 精算済みの在車を在車送信した時に精算中（事前精算：入金済み（割引含む））が（GM803002流用）
		if (ryo_buf.zenkai > 0 || ryo_buf.zenkaiWari > 0) {
			// オペレーションフェーズ
			if ( OPECTL.op_faz == 0 ) {	// 入金可送信
				OPECTL.op_faz = 1;	// 入金中
			}
		}
// GG129000(E) 改善連絡No.69,No.73 精算済みの在車を在車送信した時に精算中（事前精算：入金済み（割引含む））が（GM803002流用）
	}

	// NT-NET設定：データ形式設定 ⑥データ形式 駐車場センター形式(固定)
	if ( prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0 ) {
		// 精算情報データ用処理区分
		PayInfo_Class = 9;	// 精算途中

		// 在車情報の精算日時には未精算出庫精算時は出庫日時、
		// 事前精算時は精算開始日時が設定されている。
		// 参考：set_tim_only_out_card
		NTNetTime_152_wk.Year = pZaisha->dtPaymentDateTime.dtTimeYtoSec.shYear;
		NTNetTime_152_wk.Mon = pZaisha->dtPaymentDateTime.dtTimeYtoSec.byMonth;
		NTNetTime_152_wk.Day = pZaisha->dtPaymentDateTime.dtTimeYtoSec.byDay;
		NTNetTime_152_wk.Hour = pZaisha->dtPaymentDateTime.dtTimeYtoSec.byHours;
		NTNetTime_152_wk.Min = pZaisha->dtPaymentDateTime.dtTimeYtoSec.byMinute;
		NTNetTime_152_wk.Sec = pZaisha->dtPaymentDateTime.dtTimeYtoSec.bySecond;
	}

// GG124100(S) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
// 	// 車番情報
// 	PayData.CarSearchFlg = vl_car_no.CarSearchFlg;			// 車番/日時検索種別
// 	memcpy(PayData.CarSearchData, vl_car_no.CarSearchData,
// 		sizeof(PayData.CarSearchData));						// 車番/日時検索データ
// 	memcpy(PayData.CarNumber, vl_car_no.CarNumber,
// 		sizeof(PayData.CarNumber));							// センター問い合わせ時の車番
// 	memcpy(PayData.CarDataID, vl_car_no.CarDataID,
// 		sizeof(PayData.CarDataID));							// 車番データID(問合せに使用するID)
// 
// // GG124100(S) R.Endo 2022/08/04 車番チケットレス3.0 #6521 種別切換後の車種が領収証や精算データに反映されない
// 	// 料金種別
// 	syashu = pFeeCalc->FeeType;
// // GG124100(E) R.Endo 2022/08/04 車番チケットレス3.0 #6521 種別切換後の車種が領収証や精算データに反映されない
// GG124100(E) R.Endo 2022/08/26 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
}

// GG124100(S) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 定期初期化
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@note	1精算毎の定期関連情報を初期化する処理。<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void season_init()
{
	// 初期化
	memset(&PayData.teiki, 0, sizeof(PayData.teiki));				// 定期データ：teiki_use

	// カード毎使用枚数テーブルは定期券を初期化
	card_use[USE_PAS] = 0;
}
// GG124100(E) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 割引初期化
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@note	1精算毎の割引関連情報を初期化する処理。<br>
///			通常料金計算のOnlineDiscount(ope/opesub.c)の一部(※)に相当。<br>
///			※OnlineDiscountの先頭付近。詳細範囲はOnlineDiscount側のコメント参照。<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void discount_init()
{
	int i;

	// 初期化
	memset(PayData.DiscountData, 0, sizeof(PayData.DiscountData));	// 精算情報,領収証印字データ：DISCOUNT_DATA
// GG129000(S) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
// 	PayData.SyuWariRyo = 0;											// 種別割引料金：ulong
// GG129000(E) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
	memset(RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo, 0,
		sizeof(RTPay_Data.crd_info.dtZaishaInfo.stDiscountInfo));	// RT精算データログ：stDiscount_t
	NTNET_Data152_DiscDataClear();									// NT-NET精算情報データ用保存領域 割引詳細エリア
	NTNET_Data152_DetailDataClear();								// NT-NET精算情報データ用保存領域 明細エリア
	memset(card_use2, 0, sizeof(card_use2));						// サービス券毎使用枚数テーブル：uchar
	CardUseSyu = 0;													// 1精算の割引種類の件数：uchar
// GG129000(S) R.Endo 2024/01/12 #7217 お買物合計表示対応
	ryo_buf.shopping_total = 0;										// 買物割引金額合計：unsigned long
// GG129000(E) R.Endo 2024/01/12 #7217 お買物合計表示対応
// GG129004(S) M.Fujikawa 2024/10/22 買物金額割引情報対応
	ryo_buf.shopping_info = 0;										// 買物割引金額合計：unsigned long
// GG129004(E) M.Fujikawa 2024/10/22 買物金額割引情報対応

	// カード毎使用枚数テーブルは駐車券・定期券以外を初期化
	for ( i = 0; i < USE_MAX; i++ ) {
		switch ( i ) {
		case USE_TIK:	// 駐車券
		case USE_PAS:	// 定期券
			break;
		default:
			card_use[i] = 0;
			break;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 料金データバッファＮ設定
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@note	ryo_bufの値をryo_buf_nに設定する処理。<br>
///			通常料金計算のryo_cal(cal/ryo_cal.c)の一部(※)に相当。<br>
///			※ryo_calのec09(cal/ec09.c)前。詳細範囲はryo_cal側のコメント参照。<br>
///			<br>
///			通常料金計算からの変更点(細かなリファクタリングを除く)<br>
///			・料金額と消費税額の設定も追加。<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void ryo_buf_n_set()
{
	// 料金額
	switch ( ryo_buf.ryo_flg ) {
	case 2:	// 通常精算(定期あり)
	case 3:	// 修正精算(定期あり)(未使用)
		ryo_buf_n.ryo = ryo_buf.tei_ryo;	// 定期券での駐車料金
		break;
	case 0:	// 通常精算(定期なし)
	case 1:	// 修正精算(定期なし)(未使用)
	default:
		ryo_buf_n.ryo = ryo_buf.tyu_ryo;	// 駐車料金
		break;
	}

	// 消費税額
	ryo_buf_n.tax = ryo_buf.tax;			// 消費税額

	// 割引額
	ryo_buf_n.dis = ryo_buf.waribik;		// 割引額計

	// 入金額
	ryo_buf_n.nyu = ryo_buf.nyukin;			// 現金入金額計

	// 請求額
	ryo_buf_n.require = ryo_buf.zankin;		// 表示残り金額
}

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 料金データバッファＮ計算
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@note	ryo_buf_nの一部値を計算で求める処理。<br>
///			通常料金計算のet02(cal/et02.c)の一部(※)に相当。<br>
///			※et02の後半。詳細範囲はet02側のコメント参照。<br>
///			<br>
///			通常料金計算からの変更点(細かなリファクタリングを除く)<br>
///			・割引額を実割引にする処理を各料金計算処理(et45等)から移動。<br>
///			・消費税は0%(内税)固定とし、0%(内税)以外を想定した処理は削除。<br>
///			・サービス券フラグ(svcd_dat)の処理は未使用のため削除。<br>
///			・不足額(ryo_buf_n.hus)の処理は未使用のため削除。<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void ryo_buf_n_calc()
{
	// 請求額と割引額の計算
	if ( ryo_buf_n.ryo < ryo_buf_n.dis ) {		// 料金額より割引額が大きい
		ryo_buf_n.require = 0;

		// 料金計算基本設定：割引額 ⑥割引額 実割引とする(固定)
		if ( prm_get(COM_PRM, S_CAL, 35, 1, 1) == 1 ) {
			// 割引額を実割引にする
			ryo_buf_n.dis = ryo_buf_n.ryo;
		}
	} else {
		ryo_buf_n.require = ryo_buf_n.ryo - ryo_buf_n.dis;
	}

	// 請求額とプリペイドカード使用額(未使用)の計算
	if ( ryo_buf_n.require < c_pay ) {			// 請求額よりプリペイドカード使用額が大きい
		ryo_buf_n.require = 0;
	} else {
		ryo_buf_n.require -= c_pay;
	}

	// 請求額と入金額の計算
	if ( ryo_buf_n.require < ryo_buf_n.nyu ) {	// 請求額より入金額が大きい
		ryo_buf_n.require = 0;
	} else {
		ryo_buf_n.require -= ryo_buf_n.nyu;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 料金データバッファＮ取得
//[]----------------------------------------------------------------------[]
///	@param[in]		uchar			: 定期フラグ<br>
///									  0 = 通常<br>
///									  1 = 定期設定<br>
///	@return			void
///	@note	ryo_bufの値をryo_buf_nから取得する処理。<br>
///			通常料金計算のryo_cal(cal/ryo_cal.c)の一部(※)に相当。<br>
///			※ryo_calのet02(cal/et02.c)後。詳細範囲はryo_cal側のコメント参照。<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void ryo_buf_n_get(uchar seasonFlg)
{
	// 定期使用無し精算
	if ( seasonFlg ) {
		ryo_buf.ryo_flg += 2;				// 定期あり
	}

	switch ( ryo_buf.ryo_flg ) {
	case 2:	// 通常精算(定期あり)
	case 3:	// 修正精算(定期あり)(未使用)
		// 定期券料金種別
		ryo_buf.pas_syu = syashu;			// 車種

		// 定期券での駐車料金
		ryo_buf.tei_ryo = ryo_buf_n.ryo;	// 料金額

		break;
	case 0:	// 通常精算(定期なし)
	case 1:	// 修正精算(定期なし)(未使用)
	default:
		// 駐車券料金種別
		ryo_buf.tik_syu = syashu;			// 車種

		// 駐車料金
		ryo_buf.tyu_ryo = ryo_buf_n.ryo;	// 料金額

		break;
	}

	// 消費税額
	ryo_buf.tax = ryo_buf_n.tax;			// 消費税額

	// 割引額計
	ryo_buf.waribik = ryo_buf_n.dis;		// 割引額

	// 表示残り金額(電子マネー入金額計を引いていない)
	ryo_buf.zankin = ryo_buf_n.require;		// 請求額

	// 料金種別
	ryo_buf.syubet = (char)(syashu - 1);
}

//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 料金データバッファ計算
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@note	ryo_bufの一部値を計算で求める処理。<br>
///			通常料金計算のryo_cal(cal/ryo_cal.c)の一部(※)に相当。<br>
///			※ryo_calの末尾。詳細範囲はryo_cal側のコメント参照。<br>
///			<br>
///			通常料金計算からの変更点(細かなリファクタリングを除く)<br>
///			・定期ありと定期なしの処理差分が変数名のみのため共通化。<br>
///			・消費税は0%(内税)固定とし、0%(内税)以外を想定した処理は削除。<br>
///			・修正精算は非対応のため修正精算用の処理は削除。<br>
///			・振替精算は非対応のため振替精算用の処理は削除。<br>
///			・クレジット精算判別の処理を他と共通化。<br>
///			　(補足：CREDIT_CARD以外のcre_typeは未使用)<br>
///			・電子マネー有無による処理差分が無いため共通化。<br>
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void ryo_buf_calc()
{
	ulong ryoTemp;
	ulong sottaxTemp;
	ulong wariTemp;
	ulong overTemp;

	// 駐車料金
	switch ( ryo_buf.ryo_flg ) {
	case 2:	// 通常精算(定期あり)
	case 3:	// 修正精算(定期あり)(未使用)
		ryoTemp = ryo_buf.tei_ryo;
		break;
	case 0:	// 通常精算(定期なし)
	case 1:	// 修正精算(定期なし)(未使用)
	default:
		ryoTemp = ryo_buf.tyu_ryo;
		break;
	}

	// 課税対象額
	if ( ryoTemp < ryo_buf.waribik ) {	// 駐車料金より割引額計が大きい
		ryo_buf.kazei = 0;
	} else {
		ryo_buf.kazei = ryoTemp - ryo_buf.waribik;
	}

	// 外税額
	sottaxTemp = 0;	// 内税固定

	// 表示料金(電子マネー入金額計を引いている)と釣銭
	if ( (ryo_buf.zankin == 0) &&					// 表示残り金額あり
		 ntnet_decision_credit(&ryo_buf.credit) ) {	// クレジット精算
		// 処理のすれ違いでクレジットカードと入金の合計が表示料金を超える場合に入金を戻す
		if ( ryo_buf.dsp_ryo < (ryo_buf.nyukin + ryo_buf.credit.pay_ryo) ) {
			ryo_buf.turisen = (ryo_buf.nyukin + ryo_buf.credit.pay_ryo) - ryo_buf.dsp_ryo;
		}

		// 表示料金
		if ( ryo_buf.dsp_ryo < ryo_buf.credit.pay_ryo ) {
			ryo_buf.dsp_ryo = 0;
		} else {
			ryo_buf.dsp_ryo = ryo_buf.dsp_ryo - ryo_buf.credit.pay_ryo;
		}
	} else {										// 表示残り金額あり＆クレジット精算以外
		// 割引額計
		wariTemp = ryo_buf.waribik + c_pay + ryo_buf.emonyin;

		// 現金入金額計＋割引額計が駐車料金＋外税額を超える場合にオーバーした入金を戻す
		if ( (ryoTemp + sottaxTemp) < (ryo_buf.nyukin + wariTemp) ) {
			overTemp = (ryo_buf.nyukin + wariTemp) - (ryoTemp + sottaxTemp);

			// オーバーした入金を戻す
			if ( overTemp <= ryo_buf.nyukin ) {
				ryo_buf.turisen = ryo_buf.nyukin - overTemp;
			}
		}

		// 表示料金
		if ( (ryoTemp + sottaxTemp) < wariTemp ) {
			ryo_buf.dsp_ryo = 0;

			// 入金があれば全額戻す
			if ( ryo_buf.nyukin > 0 ) {
				ryo_buf.turisen = ryo_buf.nyukin;
			}
		} else {
			ryo_buf.dsp_ryo = (ryoTemp + sottaxTemp) - wariTemp;
		}
	}
}
// GG124100(E) R.Endo 2022/09/14 車番チケットレス3.0 #6343 クラウド料金計算対応
// GG129000(S) 改善連絡No.53 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）
//[]----------------------------------------------------------------------[]
///	@brief			クラウド料金計算 入庫情報チェック(確認のみ)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		: 入庫情報チェック結果<br>
///							  0 = 割引設定へ<br>
///							  1 = 入庫情報NG<br>
///							  2 = 入庫情報NG(QR取消なし)<br>
//[]------------------------------------- Copyright(C) 2023 AMANO Corp.---[]
short cal_cloud_discount_check_only()
{
	stFeeCalcInfo_t *pFeeCalc = &lcdbm_rsp_in_car_info_main.crd_info.dtFeeCalcInfo;	// 料金計算結果

	// 割引重複利用チェックのチェック
	switch( lcdbm_rsp_in_car_info_main.crd_info.ValidCHK_Dup ) {
	case 0:		// チェックなし
	case 1:		// 割引重複なし
		break;
	default:	// その他
		return 2;	// 入庫情報NG(QR取消なし)
	}

	// 割引情報取得結果のチェック
	switch( pFeeCalc->CalcResult ) {
	case 0:		// 成功
	case 2:		// 処理なし
		break;
	case 1:		// 処理失敗(センター割引エンジンエラー)
	default:	// その他
		return 1;	// 入庫情報NG
	}

	return 0;	// 割引設定へ
}
// GG129000(E) 改善連絡No.53 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）
