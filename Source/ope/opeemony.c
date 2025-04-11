/*[]----------------------------------------------------------------------[]*/
/*| 電子マネー関連制御                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Suzuki                                                  |*/
/*| Date         : 2006-08-02                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"Message.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"mdl_def.h"
#include	"lcd_def.h"
#include	"prm_tbl.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"suica_def.h"
#include	"ope_ifm.h"
// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応
#include	"cal_cloud.h"
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応


/*[]----------------------------------------------------------------------[]*/
/*| 電子マネー精算処理                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_emony( msg, mod )                                    |*/
/*| PARAMETER    : msg : ﾒｯｾｰｼﾞ                                            |*/
/*|                paymod : 0=通常 1=修正                                  |*/
/*| RETURN VALUE : ret : 精算完了ﾓｰﾄﾞ                                      |*/
/*| 				   :  0 = 処理継続                                     |*/
/*|                    :  1 = 精算完了(釣り無し) 	                       |*/
/*|                    :  2 = 精算完了(釣り有り)    	                   |*/
/*|                    : 10 = 精算中止(待機へ戻る)      	               |*/
/*|					   : 0x105 = 定期券通信ﾁｪｯｸ要求実施（ﾁｪｯｸ要求送信済み）|*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Suzuki                                                  |*/
/*| Date         : 2006-08-02                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short	al_emony( ushort msg, ushort paymod )
{
	short	ret = 0;

	switch( msg ){

		case SUICA_EVT:									// Suica（決済結果：完了ﾃﾞｰﾀ受信のみ呼出される）
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//		case IBK_EDY_RCV:								// Edy（決済結果：完了ﾃﾞｰﾀ受信のみ呼出される）
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)

			Ope_Last_Use_Card = 3;						// 電子決済カード利用

// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応
			if ( CLOUD_CALC_MODE ) {	// クラウド料金計算モード
				// 電子マネー設定
				cal_cloud_emoney_set();
			} else {					// 通常料金計算モード
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応

			vl_now = V_EMY;								// 現在ＶＬカードデータに電子マネーを指定（ec09で使用）
			ryo_cal( 2, OPECTL.Pr_LokNo );				// 料金計算

// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応
			}
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応

			if( OPECTL.op_faz == 0 ){					// ｵﾍﾟﾚｰｼｮﾝﾌｪｰｽﾞ(0:入金可送信)
				OPECTL.op_faz = 1;						// 1:入金中(入金有り時にｾｯﾄ)
			}
			ac_flg.cycl_fg = 10;						// 精算ｻｲｸﾙﾌﾗｸﾞ:入金
			ret = in_mony( msg, paymod );				// 入金処理
// MH321800(S) D.Inaba ICクレジット対応
// MH810103 GG119202(S) 決済精算中止印字処理修正
//			ac_flg.ec_alarm = 0;						// みなし決済復電用フラグOFF
			ac_flg.ec_deemed_fg = 0;					// みなし決済復電用フラグOFF
// MH810103 GG119202(E) 決済精算中止印字処理修正
			OPECTL.InquiryFlg = 0;
// MH321800(E) D.Inaba ICクレジット対応
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//			FurikaeMotoSts = 0;
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
			break;
// MH321800(S) Y.Tanizaki ICクレジット対応
		case EC_EVT_CRE_PAID:							// 決済リーダでのクレジット決済OK
// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応
			if ( CLOUD_CALC_MODE ) {	// クラウド料金計算モード
				// クレジットカード設定
				cal_cloud_credit_set();
			} else {					// 通常料金計算モード
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応

			vl_now = V_CRE;	//料金計算で参照.
			// vl_now : を参照して処理 > Ryo_Cal
			ryo_cal( 0, OPECTL.Pr_LokNo );

// GG124100(S) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応
			}
// GG124100(E) R.Endo 2022/07/27 車番チケットレス3.0 #6343 クラウド料金計算対応

			if( prm_get( COM_PRM, S_CRE, 3, 1, 1 ) == 1 ){	//利用明細書/領収書 自動発行する
				if( OPECTL.RECI_SW == 0 ){					// 領収書ﾎﾞﾀﾝ未使用?
					OPECTL.RECI_SW = 1;						// 領収書ﾎﾞﾀﾝ使用
// MH810100(S) S.Fujii 2020/07/01 車番チケットレス(電子決済端末対応)
					PKTcmd_notice_ope( LCDBM_OPCD_RCT_AUTO_ISSUE, (ushort)0 );	// 操作通知送信
// MH810100(E) S.Fujii 2020/07/01 車番チケットレス(電子決済端末対応)
				}
			}

			ac_flg.cycl_fg = 10;						// 入金
			in_mony( msg, 0 );
// MH810103 GG119202(S) 決済精算中止印字処理修正
//			ac_flg.ec_alarm = 0;						// みなし決済復電用フラグOFF
			ac_flg.ec_deemed_fg = 0;					// みなし決済復電用フラグOFF
// MH810103 GG119202(E) 決済精算中止印字処理修正

			OPECTL.InquiryFlg = 0;
// MH810100(S) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
//			FurikaeMotoSts = 0;							// 振替元状態フラグ 0:チェック不要
// MH810100(E) K.Onodera 2020/03/10 車番チケットレス(不要処理削除)
			break;
// MH321800(E) Y.Tanizaki ICクレジット対応
// MH810105(S) MH364301 QRコード決済対応
		case EC_EVT_QR_PAID:
// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
			if ( CLOUD_CALC_MODE ) {	// クラウド料金計算モード
				// QRコード決済設定
				cal_cloud_eqr_set();
			} else {					// 通常料金計算モード
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応

			vl_now = V_EQR;								// 現在ＶＬカードデータにQRコードを指定（ec09で使用）
			ryo_cal( 2, OPECTL.Pr_LokNo );				// 料金計算

// GG124100(S) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応
			}
// GG124100(E) R.Endo 2022/08/22 車番チケットレス3.0 #6343 クラウド料金計算対応

			if( OPECTL.op_faz == 0 ){					// ｵﾍﾟﾚｰｼｮﾝﾌｪｰｽﾞ(0:入金可送信)
				OPECTL.op_faz = 1;						// 1:入金中(入金有り時にｾｯﾄ)
			}
			ac_flg.cycl_fg = 10;						// 精算ｻｲｸﾙﾌﾗｸﾞ:入金
			ret = in_mony( msg, paymod );				// 入金処理

			ac_flg.ec_deemed_fg = 0;					// みなし決済復電用フラグOFF
			OPECTL.InquiryFlg = 0;
// MH810105(S) MH364301 QRコード決済対応 GT-4100
//			FurikaeMotoSts = 0;
// MH810105(E) MH364301 QRコード決済対応 GT-4100
			break;
// MH810105(E) MH364301 QRコード決済対応

	    default:
			return( -1 );
	}
	return( ret );
}

// MH810105(S) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
//// MH321800(S) G.So ICクレジット対応
////[]----------------------------------------------------------------------[]
/////	@brief			電子マネー相殺割引精算処理
////[]----------------------------------------------------------------------[]
/////	@param[in]		void
/////	@return			ret			精算完了ﾓｰﾄﾞ<br>
/////								 0 = 処理継続<br>
/////                              1 = 精算完了(釣り無し)<br>
/////                              2 = 精算完了(釣り有り)<br>
/////                             10 = 精算中止(待機へ戻る)<br>
/////	@author			G.So
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2019/01/29<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//short	al_emony_sousai(void)
//{
//	short	ret;
//
//	vl_sousai((ushort)prm_get(COM_PRM ,S_ECR, 11, 4, 1));	// 相殺割引処理
//// MH810103(s) 電子マネー対応 # 5396 50-0011=110100設定時、WAONで未了残高照会中に同一カードタッチで精算完了しても、リアルタイムの精算データに店割引の情報がセットされない
//	// m_stDiscをセットする
//	RTPay_pre_sousai();
//// MH810103(e) 電子マネー対応 # 5396 50-0011=110100設定時、WAONで未了残高照会中に同一カードタッチで精算完了しても、リアルタイムの精算データに店割引の情報がセットされない
//
//	ryo_cal( 9, 0 );										// 料金計算(相殺割引)
//	if (OPECTL.op_faz == 0) {								// ｵﾍﾟﾚｰｼｮﾝﾌｪｰｽﾞ(0:入金可送信)
//		OPECTL.op_faz = 1;									// 1:入金中(入金有り時にｾｯﾄ)
//	}
//	ac_flg.cycl_fg = 10;									// 精算ｻｲｸﾙﾌﾗｸﾞ:入金
//	// サービス券か掛売券に置き換えるのでカードイベントで処理する
//	ret = in_mony(ARC_CR_R_EVT, 0);							// 入金処理
//// MH810103(s) 電子マネー対応 # 5396 50-0011=110100設定時、WAONで未了残高照会中に同一カードタッチで精算完了しても、リアルタイムの精算データに店割引の情報がセットされない
//	RTPay_set_sousai();
//// MH810103(e) 電子マネー対応 # 5396 50-0011=110100設定時、WAONで未了残高照会中に同一カードタッチで精算完了しても、リアルタイムの精算データに店割引の情報がセットされない
//
//	return ret;
//}
//// MH321800(E) G.So ICクレジット対応
// MH810105(E) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
