// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
//[]----------------------------------------------------------------------[]
///	@file	cal_cloud.h
///	@brief	クラウド料金計算処理ヘッダー
///	@date	2022/06/02
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]

#ifndef	_CAL_CLOUD_H_
#define	_CAL_CLOUD_H_

#include "mem_def.h"
#include "mnt_def.h"
#include "ope_def.h"

// 料金計算モード判定
// GG124100(S) R.Endo 2022/09/29 車番チケットレス3.0 #6615 設定01-0079(料金計算切替)の設定の意味を変更する / デフォルト値を変更する
// #define CLOUD_CALC_MODE (g_calcMode == 1)	// クラウド料金計算モード
#define CLOUD_CALC_MODE (g_calcMode == 0)	// クラウド料金計算モード
// GG124100(E) R.Endo 2022/09/29 車番チケットレス3.0 #6615 設定01-0079(料金計算切替)の設定の意味を変更する / デフォルト値を変更する

// クラウド料金計算では設定禁止とするアドレス
extern const PRM_MASK param_mask_cc[];	// マスクデータ
extern const short	prm_mask_max_cc;	// マスクデータ件数

// クラウド料金計算用ユーザーメニュー
enum {
	__MNT_ERARM_CC = 0,	// .エラー・アラーム確認
	__MNT_MNCNT_CC,		// .釣銭管理
	__MNT_TTOTL_CC,		// .T集計
	__MNT_GTOTL_CC,		// .GT集計
	__MNT_STSVIEW_CC,	// .状態確認
	__MNT_REPRT_CC,		// .領収書再発行
	__MNT_QRCHECK_CC,	// .QRデータ確認
	__MNT_CLOCK_CC,		// .時計合せ
// 	__MNT_SERVS_CC,		// .サービスタイム
// 	__MNT_SPCAL_CC,		// .特別日/特別期間
	__MNT_OPCLS_CC,		// .営休業切替
	__MNT_PWMOD_CC,		// .係員パスワード
	__MNT_KEYVL_CC,		// .キー音量調整
	__MNT_CNTRS_CC,		// .輝度調整
	__MNT_VLSW_CC,		// .音声案内時間
	__MNT_OPNHR_CC,		// .営業開始時刻
// 	__MNT_MNYSET_CC,	// .料金設定
	__MNT_EXTEND_CC,	// .拡張機能
	USER_MENU_MAX_CC
};
extern const unsigned char UMMENU_CC[][27];
extern const unsigned short USER_TBL_CC[][4];

// クラウド料金計算用システムメンテナンスメニュー
#define SYS_MENU_MAX_CC (7)
extern const unsigned char SMMENU_CC[][31];
extern const unsigned short SYS_TBL_CC[][4];

// クラウド料金計算用ログファイルプリントメニュー
#define LOG_MENU_MAX_CC (14)
extern const unsigned char LOGMENU_CC[][31];
extern const unsigned short LOG_FILE_TBL_CC[][4];

// クラウド料金計算 精算開始日時チェック
extern short cal_cloud_pay_start_time_check(ushort ndatTarget);

// クラウド料金計算 初期化処理
extern void cal_cloud_init();

// クラウド料金計算 入庫情報チェック(料金)
extern short cal_cloud_fee_check();

// クラウド料金計算 料金設定
// GG124100(S) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる
// extern short cal_cloud_fee_set();
extern short cal_cloud_fee_set(uchar firstFlg);
// GG124100(E) R.Endo 2022/09/16 車番チケットレス3.0 #6605 現金投入後、割引券を使用すると投入金額がなかったことになる

// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
// クラウド料金計算 料金設定 遠隔精算(入庫時刻指定)
extern short cal_cloud_fee_set_remote_time();

// クラウド料金計算 料金設定 遠隔精算(金額指定)
extern short cal_cloud_fee_set_remote_fee();
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

// クラウド料金計算 定期チェック
extern eSEASON_CHK_RESULT cal_cloud_season_check();

// クラウド料金計算 定期設定
extern short cal_cloud_season_set();

// クラウド料金計算 QRデータチェック
extern OPE_RESULT cal_cloud_qrdata_check(tMediaDetail *pMedia, QR_YMDData* pYmdData);

// クラウド料金計算 入庫情報チェック(割引)
extern short cal_cloud_discount_check();

// クラウド料金計算 割引設定
extern short cal_cloud_discount_set();

// GG124100(S) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)
// クラウド料金計算 割引設定 遠隔精算(入庫時刻指定)
extern short cal_cloud_discount_set_remote_time();

// クラウド料金計算 割引設定 遠隔精算(金額指定)
extern short cal_cloud_discount_set_remote_fee();
// GG124100(E) R.Endo 2022/09/06 車番チケットレス3.0 #6343 クラウド料金計算対応(遠隔精算)

// クラウド料金計算 電子マネー設定
extern short cal_cloud_emoney_set();

// クラウド料金計算 QRコード決済設定
extern short cal_cloud_eqr_set();

// クラウド料金計算 クレジットカード設定
extern short cal_cloud_credit_set();

// GG129000(S) 改善連絡No.53 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）
// クラウド料金計算 入庫情報チェック(確認のみ)
extern short cal_cloud_discount_check_only();
// GG129000(E) 改善連絡No.53 再精算時のレーンモニターデータの精算中（入金済み（割引含む））について（GM803002流用）

#endif	// _CAL_CLOUD_H_
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
