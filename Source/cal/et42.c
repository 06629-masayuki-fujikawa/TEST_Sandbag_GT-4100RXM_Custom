/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：時間割引処理　　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et42()　　　　　　　　　　：　通常の時間割引処理、及び定期処理後の時間割引処理を行う。　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　無し　　　　　　　　　　　　無し　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/**********************************************/
#include	<string.h>													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"system.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"mem_def.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"rkn_def.h"													/*　全デファイン統括　　　　　　　　　　　　　*/
#include	"rkn_cal.h"													/*　料金関連データ　　　　　　　　　　　　　　*/
#include	"rkn_fun.h"													/*　全サブルーチン宣言　　　　　　　　　　　　*/
#include	"Tbl_rkn.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"prm_tbl.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	et42()															/*　時間割引処理　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	short		tkiki	 = 0;											/*　定期券種別　　　　　　　　　　　　　　　　*/
	long	c_ryo	 = 0;												/*　ｗｏｒｋ料金　　　　　　　　　　　　　　　*/
	long	wk_ryo	 = 0;												/*　ｗｏｒｋ料金　　　　　　　　　　　　　　　*/
	long	wok_dis	 = 0;												/*　ｗｏｒｋ割引額　　　　　　　　　　　　　　*/
	long	wok_pdis = 0;												/*　ｗｏｒｋ％割引額　　　　　　　　　　　　　*/
	struct	REQ_RKN	*req_p;												/*　現在計算後格納エリア　　　　　　　　　　　*/
	struct	RYO_BUF_N	*ryo_b;											/*　現在計算後格納エリア　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	req_p = &req_rhs[tb_number];										/*　現在料金計算要求ポインター　　　　　　　　*/
	ryo_b = &ryo_buf_n;													/*　現在料金計算バッファポインター　　　　　　*/
	discount = 0l;														/*　割引額０クリア　　　　　　　　　　　　　　*/
	car_type = req_rkn.syubt;											/*　車種　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( ctyp_flg == 4 )													/*　　　　　　　　　　　　　　　　　　　　　　*/
		wk_ryo = base_ryo + ryo_b->tax - tol_dis;						/*　料金＝料金＋税金ー割引額　　　　　　　　　*/
	else																/*　　　　　　　　　　　　　　　　　　　　　　*/
		wk_ryo = base_ryo - tol_dis;									/*　料金＝料金ー割引額　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	tkiki = (short)req_rkn.data[1];										/*　定期券種別　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	jik_dtm = req_rkn.data[0];
	req_p->data[0] = jik_dtm;											/*　割引時間数セット　　　　　　　　　　　　　*/
	ryo_buf.tim_amount += (ulong)jik_dtm;								/*　割引時間数加算　　　　　　　　　　　　　　*/
	memcpy( &car_ot,&carot_mt,7 );										/*　出庫時刻を自己出庫時刻にコピーする　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	ryoukin = wk_ryo;													/*　ワーク料金を料金とする　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( tki_flg == PAS_TIME )											// 定期処理後FLGが時間帯定期の場合
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(( su_jik_dtm != 0 )&&( CPrmSS[S_CAL][30] == 1 )				// 種別割引時間があり設定が１で
					&&( su_jik_plus	== 0 ))								// １回目ならば/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			jik_dtm = jik_dtm + su_jik_dtm;								/*　割引時間に種別割引時間をプラス　　　　　　*/
			su_jik_plus = 1;											/*　定期種別割引時間プラスｆｌｇＯＮ　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		discount = et422();												/*　定期時間割引額算出　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　定期処理後ｆｌｇがＯＮ以外　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
  		discount = et421();												/*　通常時間割引額算出　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	wok_dis = discount;													/*　ｗｏｒｋ割引額　　　　　　　　　　　　　　*/
	discount = discount - mae_dis;										/*　今回時間割引額算出　　　　　　　　　　　　*/
	mae_dis = wok_dis;													/*　旧時間割引額（トータル時間割引額）　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( percent != 0 )													/*　％割引があった場合　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		wok_pdis = discount * percent / 10000 ;							/*　１００００で割る　　　　　　　　　　　　　*/
		discount = discount - wok_pdis;									/*　％割引の対応処理　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	c_ryo = wk_ryo - discount;											/*　料金＝割引対象額－時間割引額　　　　　　　*/
	if( c_ryo <= 0 )													/*　駐車料金≦０の場合　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( jitu_wari == 1 )											/*　実割引の場合　　　　　　　　　　　　　　　*/
			discount = wk_ryo;											/*　割引額＝前料金　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		ryoukin  = 0;													/*　駐車料金は０円　　　　　　　　　　　　　　*/
		syu_tax  = 0;													/*　税金＝０　　　　　　　　　　　　　　　　　*/
		base_dis_flg = ON;												/*　税金＝０　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　　　　　　　　　　　　　　　　　　　　　　*/
		ryoukin = c_ryo;												/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if(( ctyp_flg != 4 )&&( cons_tax_ratio != 0 ))						/*　割引対象が料金かつ　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		ryo_b->tax = ec68( c_ryo,cons_tax_ratio );						/*　駐車料金消費税の算出　　　　　　　　　　　*/
		cons_tax = ryo_b->tax;											/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( ctyp_flg == 1 )												/*　内税の場合　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			syu_tax = cons_tax;											/*　消費税を集計用エリアへ　　　　　　　　　　*/
			cons_tax = 0;												/*　消費税を０　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	tol_dis		= tol_dis + discount;									/*　割引額合計＝割引額合計＋割引額　　　　　　*/
	ryo_b->dis 	= tol_dis;												/*　駐車料金消費税のセーブ　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	req_p->syubt 	= req_rkn.syubt;									/*　計算結果テーブルに車種コピー　　　　　　　*/
	req_p->param 	= req_rkn.param;									/*　計算結果テーブルにパラメータコピー　　　　*/
	req_p->data[1] 	= (long)tkiki;										/*　定期種別のセット　　　　　　　　　　　　　*/
	req_p->data[2] 	= discount;											/*　割引額をセット　　　　　　　　　　　　　　*/
	req_p->data[3] 	= ryoukin;											/*　割引後額をセット　　　　　　　　　　　　　*/
	req_p->data[4] 	= cons_tax;											/*　消費税額をセット　　　　　　　　　　　　　*/
	ryo_buf.dis_tim += discount;										/*　前回時間割引額加算　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
