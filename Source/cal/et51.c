/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：店番号別割引処理　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et51()　　　　　　　　　　：　店番号・枚数により、時間割引・料金割引を行う。　　　　　　　：　*/
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
// MH810100(S) K.Onodera 2020/02/12 車番チケットレス(割引済み対応)
#include	"ope_def.h"
// MH810100(E) K.Onodera 2020/02/12 車番チケットレス(割引済み対応)
#include	"flp_def.h"
// MH810100(S) Y.Watanabe 2020/02/04 車番チケットレス(LCD_IF対応)
#include	"ntnet_def.h"
// MH810100(E) Y.Watanabe 2020/02/04 車番チケットレス(LCD_IF対応)
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	et51()															/*　店Ｎｏ．割引処理　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	struct	REQ_RKN	*req_p;												/*　現在計算後格納エリア　　　　　　　　　　　*/
	struct	RYO_BUF_N	*r;												/*　現在計算後格納エリア　　　　　　　　　　　*/
	long			cash_dis;											/*　割引料金　　　　　　　　　　　　　　　　　*/
	long			wk_ryo		= 0L;									/*　旧料金　　　　　　　　　　　　　　　　　　*/
	long			wk_dis		= 0L;									/*　旧料金　　　　　　　　　　　　　　　　　　*/
	long			sav_dat 	= 0L;									/*　データセーブ　　　　　　　　　　　　　　　*/
	long			wok_pdis 	= 0L;									/*　ｗｏｒｋ％割引額　　　　　　　　　　　　　*/
	short			shop_num;											/*　店番号　　　　　　　　　　　　　　　　　　*/
	short			maisu;												/*　割引券枚数　　　　　　　　　　　　　　　　*/
	short			wk_adr;												/*　　　　　　　　　　　　　　　　　　　　　　*/
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店番号別割引処理の変更)
	short			wk_ses;												/*　セッション　　　　　　　　　　　　　　　　*/
	short			wk_dat;												/*　多店舗割引種別　　　　　　　　　　　　　　*/
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店番号別割引処理の変更)
	long			wk_ryo2 = 0;										/*　旧料金退避エリア　　　　　　　　　　　　　*/
// MH321800(S) hosoda ICクレジット対応 (アラーム取引)
	long			role;												/* 役割                                       */
// MH321800(E) hosoda ICクレジット対応 (アラーム取引)
// MH810100(S) K.Onodera 2020/02/10 車番チケットレス(割引済み対応)
	struct	REQ_RKN	*n_req_p;
	uchar			disc_sts;
// MH810103(S) R.Endo 2021/07/29 車番チケットレス フェーズ2.2 #5841 【検証課指摘事項】「QR割引券で種別切換して精算」 → 「ラグタイム後 かつ 追加料金発生前に再精算」 を行うと割引金額に不正な値がセットされる
//	ulong			last_disc;
	ulong			last_disc 	= 0;
// MH810103(E) R.Endo 2021/07/29 車番チケットレス フェーズ2.2 #5841 【検証課指摘事項】「QR割引券で種別切換して精算」 → 「ラグタイム後 かつ 追加料金発生前に再精算」 を行うと割引金額に不正な値がセットされる
	ulong			new_disc;
// MH810100(E) K.Onodera 2020/02/10 車番チケットレス(割引済み対応)
// MH810100(S) 2020/06/30 【連動評価指摘事項】再精算時の割引適応方法の変更
	char			disFlag = 0;
// MH810100(E) 2020/06/30 【連動評価指摘事項】再精算時の割引適応方法の変更
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	req_p	 = &req_rhs[tb_number];										/*　現在料金計算要求ポインター　　　　　　　　*/
	r 		 = &ryo_buf_n;												/*　現在料金計算要求ポインター　　　　　　　　*/
	memmove( req_p , &req_rkn , sizeof(req_rkn) );						/*　現在料金計算要求TBLを計算結果TBLへセット　*/
// MH810100(S) K.Onodera 2020/01/27 車番チケットレス(割引済み対応)
//	shop_num = (short)req_rkn.data[0];									/*　店番号参照　　　　　　　　　　　　　　　　*/
	shop_num = (short)( req_rkn.data[0] & 0x00007fff );					/*　店番号参照　　　　　　　　　　　　　　　　*/
// MH810100(E) K.Onodera 2020/01/27 車番チケットレス(割引済み対応)
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店番号別割引処理の変更)
//	wk_adr = (short)(1+3*(shop_num-1));									/*                                            */
	wk_dat = (req_rkn.data[0] >> 24) & 0x000000ff;
	if( wk_dat ) {														/*　多店舗割引　　　　　　　　　　　　　　　　*/
		wk_adr = (short)(wk_dat * 2);
		wk_ses = (short)S_TAT;
	} else {															/*　店割引/買物割引 　　　　　　　　　　　　　*/
		wk_adr = (short)((shop_num * 3) - 2);
		wk_ses = (short)S_STO;
	}
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店番号別割引処理の変更)
	discount = 0l;														/*　割引額０クリア　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if ( ! req_rkn.data[1] )	maisu = 1;								/*  枚数１とする                              */
	else						maisu = (short)req_rkn.data[1];			/*  枚数参照                                  */
																		/*                                            */
// MH321800(S) hosoda ICクレジット対応 (アラーム取引)
//-	if( CPrmSS[S_STO][wk_adr] == 1 || CPrmSS[S_STO][wk_adr] == 2 ||		/*　役割がある場合のみ　　　　　　　　　　　　*/
//-	    CPrmSS[S_STO][wk_adr] == 4 )									/*　　　　　　　　　　　　　　　　　　　　　　*/
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店番号別割引処理の変更)
//	role		= CPrmSS[S_STO][wk_adr];								/*                                            */
	role = prm_get(COM_PRM, wk_ses, wk_adr, 1, 1);
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店番号別割引処理の変更)
// MH810105(S) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
//	if (f_sousai != 0) {
//		role = 4;
//	}
// MH810105(E) MH364301 WAONの未了残高照会タイムアウト時の精算完了処理修正
	if ( role == 1 || role == 2  || role == 4 )							/*  役割が１か２か４の場合のみ処理をする          */
// MH321800(E) hosoda ICクレジット対応 (アラーム取引)
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		maisu = (short)req_rkn.data[1];									/*　枚数参照　　　　　　　　　　　　　　　　　*/
		if(ctyp_flg == 4)												/*　割引対象額が料金＋税金の場合　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			wk_ryo 	 = r->ryo	- r->dis + r->tax;						/*　割引額＝料金　　　　　　　　　　　　　　　*/
			cons_tax = 0;												/*　税金＝０（使用しない）　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			wk_ryo	 = r->ryo	- r->dis;								/*　割引額＝料金　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
// MH321800(S) hosoda ICクレジット対応 (アラーム取引)
//-		switch((char)CPrmSS[S_STO][wk_adr])								/*　役割により処理を分割　　　　　　　　　　　*/
		switch( role )													/*  役割により処理を分割                      */
// MH321800(E) hosoda ICクレジット対応 (アラーム取引)
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			case 1:														/*　役割が１（時間割引き）の場合　　　　　　　*/
				memcpy( &car_ot , &carot_mt , 7 );						// 出庫時刻を自己出庫時刻にコピーする
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店番号別割引処理の変更)
//				jik_dtm =												/*　割引時間を算出　　　　　　　　　　　　　　*/
//					(long)(prm_tim( 0,S_STO,(short)(wk_adr+1) )*maisu);	/*　　　　　　　　　　　　　　　　　　　　　　*/
				jik_dtm = prm_tim(COM_PRM, wk_ses, (wk_adr + 1)) * maisu;	/*　割引時間を算出　　　　　　　　　　　　*/
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店番号別割引処理の変更)
																		/*                                            */
				if(req_rkn.param != RY_CMI)								/*　再精算時は割引時間の加算を行わない　　　　*/
					ryo_buf.tim_amount += (ulong)jik_dtm;				/*　割引時間数加算　　　　　　　　　　　　　　*/
				if( tki_flg == PAS_TIME )								// 定期処理後FLGが時間帯定期の場合
				{														/*　　　　　　　　　　　　　　　　　　　	  */
					req_rkn.data[1] = tsn_tki.kind;						/*　定期種別セット　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					sav_dat 		 = req_rkn.data[1];					/*　データセーブ　　　　　　　　　　　　　　　*/
					req_rkn.data[1]  = tsn_tki.kind;					/*　定期種別セット　　　　　　　　　　　　　　*/
					re_req_flg 		 = ON;								/*　時間帯定期後時間割引フラグセット　　　　　*/
					if(( su_jik_dtm != 0 )&&( CPrmSS[S_CAL][30] == 1 )	/*　種別割引時間があり設定が１で　　　　　　　*/
								&&( su_jik_plus	== 0 ))					/*　１回目ならば　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						jik_dtm = jik_dtm + su_jik_dtm;					/*　割引時間に種別割引時間をプラス　　　　　　*/
						su_jik_plus = 1;								/*　定期種別割引時間プラスｆｌｇＯＮ　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					discount 		 = et422();							/*　定期時間割引額算出　　　　　　　　　　　　*/
					req_rkn.data[1]  = sav_dat;							/*　データセーブ　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else		discount = et421();							/*　定期処理後flgがON以外　通常時間割引額算出 */

/* 割引額が前回より少ないと今回割引額がﾏｲﾅｽﾃﾞｰﾀとなってしまい、集計ﾃﾞｰﾀ上でもﾏｲﾅｽで集計されてしまう為                 */
/* 前回割引額より今回割引額が少ない場合は､今回割引額を前回割引額とし､今回分を０円とする.                              */

				if(req_rkn.param != RY_CMI){							/*　再精算時トータル割引時間額をセットしない　*/
					if( mae_dis > discount ){							/*                                            */
						discount = mae_dis;								/*                                            */
					}													/*                                            */
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					wk_dis 		= discount;								/*　ｗｏｒｋ割引額　　　　　　　　　　　　　　*/
					discount 	= discount - mae_dis;					/*　今回時間割引額算出　　　　　　　　　　　　*/
					mae_dis 	= wk_dis;								/*　旧時間割引額（トータル時間割引額）　　　　*/
				}														/*                                            */
				else{ 
					if( tyushi_mae_dis > discount ){					/*                                            */
						discount = tyushi_mae_dis;						/*                                            */
					}													/*                                            */
					wk_dis		= discount;								/*  ｗｏｒｋ割引額                            */
					discount	= discount - tyushi_mae_dis;			/*  今回時間割引額算出                        */
					tyushi_mae_dis		= wk_dis;						/*  旧時間割引額（トータル時間割引額）        */
				}														/*                                            */
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( percent != 0 )										/*　％割引があった場合　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( CPrmSS[S_CAL][21] == 1 )						/*　％割引の未満処理をする場合　　　　　　　　*/
						wok_pdis = ec68( discount,(short)percent );		/*　未満処理をする　　　　　　　　　　　　　　*/
					else												/*　未満処理をしない場合　　　　　　　　　　　*/
						wok_pdis = discount * percent / 10000 ;			/*　１００００で割る　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					discount	 = discount - wok_pdis;					/*　％割引の対応処理　　　　　　　　　　　  　*/
				}														/*　　　　　　　　　　　　　　　　			  */
			break;														/*　処理終了　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			case 2:														/*　役割が２（料金割引）の場合　　　　　　　　*/
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店番号別割引処理の変更)
//				cash_dis = CPrmSS[S_STO][wk_adr+1];						/*　割引料金を算出　　　　　　　　			  */
				cash_dis = prm_get(COM_PRM, wk_ses, (wk_adr + 1), 4, 1);	/*　割引料金を算出　　　　　　　　　　　　*/
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店番号別割引処理の変更)
																		/*　　　　　　　　　　　　　　　　　　　  	　*/
				discount	 = cash_dis * (long)maisu;					/*　割引額＝１枚当りの割引額×枚数　　　　　　*/
			break;														/*　処理終了　　　　　　　　　　　　　　　　　*/
			case 4:														/*　役割が４（料金割引全額割引）の場合　　　　*/
				discount	 = MACRO_WARIBIKI_GO_RYOUKIN;				/*  割引額＝旧料金－プリペ利用額              */
				ryoukin		 = 0;										/*　割引額＝料金　　　　　　　　　　　　　　　*/
				syu_tax		 = 0;										/*　税金＝０　　　　　　　　　　　　　　　　　*/
				base_dis_flg = ON;										/*　基本料金全額割引フラグＯＮ　 　  　　　	  */
			break;														/*　処理終了　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(req_rkn.param == RY_CMI){									/*　再精算時　　　　　　　　　　　　　　　　　*/
			wk_ryo2 = wk_ryo;											/*　旧料金を退避し割引前料金を使用する　　　　*/
			wk_ryo = r->ryo - discount_tyushi;							/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*                                            */
// MH810100(S) 2020/06/30 【連動評価指摘事項】再精算時の割引適応方法の変更
		disc_sts = (uchar)(( req_p->data[0] >> 15 ) & 0x00000001 );			// 割引ステータス取り出し
		if( disc_sts ){														// 割引済
// MH810100(S) 2020/09/02 再精算時の複数枚対応
//			last_disc = Ope_GetLastDisc();									// 割引済額取出し
			last_disc = Ope_GetLastDisc(discount);							// 割引済額取出し
// MH810100(E) 2020/09/02 再精算時の複数枚対応
			
// MH810100(S) 2020/09/28 【連動評価指摘事項】初回精算請求額200円を全額割引した状態で再精算を行うと、再精算請求額が全額割引されない(No.02-0001)
//			// 前回分をこの時点で引く（同じか大きくなっている場合は、再度適用された。）
//			if( discount >= last_disc){
//				discount = discount - last_disc;
//				disFlag = 1;
//			// 全額割の場合はMAXにセット
//			}else if(base_dis_flg == ON){
//				discount = NMAX_OVER;
//				discount = discount - last_disc;
//				disFlag = 1;
			// 全額割の場合はMAXにセット
			 if(base_dis_flg == ON){
				discount = NMAX_OVER;
				discount = discount - last_disc;
				disFlag = 1;
			// 前回分をこの時点で引く（同じか大きくなっている場合は、再度適用された。）
			}else if( discount >= last_disc){
				discount = discount - last_disc;
				disFlag = 1;
// MH810100(E) 2020/09/28 【連動評価指摘事項】初回精算請求額200円を全額割引した状態で再精算を行うと、再精算請求額が全額割引されない(No.02-0001)
			}else{
				// 未満の場合は、制限に引っかかった場合
				discount = 0; 
				disFlag = 1;
			}
		}
// MH810100(E) 2020/06/30 【連動評価指摘事項】再精算時の割引適応方法の変更
																		/*                                            */
		if(MACRO_WARIBIKI_GO_RYOUKIN <= discount)						/*　基本料金≦割引額　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( jitu_wari == 1 )										/*                                            */
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				discount 	=  MACRO_WARIBIKI_GO_RYOUKIN;				/*　実割引の場合　割引額＝旧料金　　　　　　　*/
																		/*                                            */
				if(req_rkn.param != RY_CMI){							/*                                            */
					tol_dis 	+= discount;							/*　割引額合計＝割引額合計＋割引額  　　　　　*/
				}														/*                                            */
			}															/*　　　　　　　　　　　　　 　　　 　　　　　*/
			else 														/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(req_rkn.param != RY_CMI){							/*                                            */
					tol_dis 	+= wk_ryo;								/*　割引額合計＝割引額合計＋割引額　  　　　　*/
				}														/*                                            */
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			ryoukin		 = 0;											/*　料金＝０　　　　　　　　　　　　　　　　　*/
			syu_tax		 = 0;											/*　税金＝０　　　　　　　　　　　　　　　　　*/
			base_dis_flg = ON;											/*　基本料金全額割引フラグＯＮ　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(req_rkn.param != RY_CMI){
				tol_dis 	+= discount;								/*　割引額合計＝割引額合計＋旧割引額　　　　　*/
				ryoukin 	= wk_ryo  - discount;						/*　料金＝ワーク料金ー割引額　　　　　　　　　*/
			}
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( ctyp_flg != 4 && cons_tax_ratio != 0 )						/*　割引対象が料金かつ消費税率が０％以外の場合*/
			cons_tax = ec68(ryoukin,cons_tax_ratio);					/*　消費税額＝割引後料金×消費税率　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	// 精算中止機能強化
	if(req_rkn.param == RY_CMI){										/*　再精算時　　　　　　　　　　　　　　　　　*/
		PayData_Sub.wari_data[wrcnt_sub].tik_syu = 0x02;				/*　詳細中止ｴﾘｱに券種別をセット	　　　　　　　*/
// MH810104(S) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店番号別割引処理の変更)
// 		PayData_Sub.wari_data[wrcnt_sub].syubetu = (uchar)shop_num;		/*　詳細中止ｴﾘｱに店Noセット 　　　　　　　　　*/
		PayData_Sub.wari_data[wrcnt_sub].syubetu = (ushort)shop_num;	/*　詳細中止ｴﾘｱに店Noセット 　　　　　　　　　*/
		PayData_Sub.wari_data[wrcnt_sub].mise_no = (ushort)wk_dat;		/*　詳細中止ｴﾘｱに多店舗割引種別セット 　　　　*/
// MH810104(E) R.Endo 2021/08/30 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店番号別割引処理の変更)
		PayData_Sub.wari_data[wrcnt_sub].maisuu = (uchar)maisu;			/*　詳細中止ｴﾘｱに枚数セット 　　　　　　　　　*/
		PayData_Sub.wari_data[wrcnt_sub].ryokin = discount;				/*　詳細中止ｴﾘｱに割引額セット 　　　　　　　　*/
		wrcnt_sub++;													/*　詳細中止エリアカウンタUP　　　　　　　　　*/
		wk_ryo = wk_ryo2;												/*　旧割引額をセット　　　　　　　　　　　　　*/
		discount_tyushi += discount;									/*　再精算用割引額を加算　　　　　　　　　　　*/
	}																	/*                                            */
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if(req_rkn.param != RY_CMI){										/*　再精算時割引額のセットを行わない　　　　　*/
																		/*                                            */
		if(ctyp_flg != 4)												/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			r->tax = cons_tax;											/*　駐車料金消費税のセーブ　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(ctyp_flg == 1)											/*　内税の場合　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				syu_tax 	= cons_tax;									/*　消費税を集計用エリアへ　　　　　　　　　　*/
				cons_tax 	= 0;										/*　消費税を０　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		r->dis			= tol_dis ;										/*　駐車料金消費税のセーブ　　　　　　　　　　*/
		req_p->data[2]	= discount;										/*　割引額をセット　　　　　　　　　　　　　　*/
		req_p->data[3]	= ryoukin ;										/*　割引後額をセット　　　　　　　　　　　　　*/
		req_p->data[4]	= cons_tax;										/*　消費税額をセット　　　　　　　　　　　　　*/
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店番号別割引処理の変更)
//		if( CPrmSS[S_STO][wk_adr] == 2 ){								/*　料金割引？　　　　　　　　　　　　　　　　*/
		if ( role == 2 ) {												/*　料金割引？　　　　　　　　　　　　　　　　*/
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店番号別割引処理の変更)
			ryo_buf.fee_amount += discount;								/*　割引額加算　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*                                            */
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_GT3890より追加)
	// オンラインデータの割引中なら
	// 精算機設定に合わせて割引種別も上書く
	if(( m_stDisc.DiscSyu % 100 ) == NTNET_KAK_M ){			// 店割引
		if( role == 1 ){	// 時間割引
			m_stDisc.DiscSyu = NTNET_KAK_T;
		}else{				// 料金割引・全額割引
			m_stDisc.DiscSyu = NTNET_KAK_M;
		}
// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店番号別割引処理の変更)
	} else if( (m_stDisc.DiscSyu % 100) == NTNET_TKAK_M ) {	// 多店舗割引
		if ( role == 1 ) {	// 時間割引
			m_stDisc.DiscSyu = NTNET_TKAK_T;
		} else {			// 料金割引・全額割引
			m_stDisc.DiscSyu = NTNET_TKAK_M;
		}
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(店番号別割引処理の変更)
	}else{													// 買物割引
		if( role == 1 ){	// 時間割引
			m_stDisc.DiscSyu = NTNET_SHOP_DISC_TIME;
		}else{				// 料金割引・全額割引
			m_stDisc.DiscSyu = NTNET_SHOP_DISC_AMT;
		}
	}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_GT3890より追加))
// MH810100(S) K.Onodera 2020/02/10 車番チケットレス(割引済み対応)
	disc_sts = (uchar)(( req_p->data[0] >> 15 ) & 0x00000001 );			// 割引ステータス取り出し
	if( disc_sts ){														// 割引済
// MH810100(S) 2020/09/02 再精算時の複数枚対応
//		last_disc = Ope_GetLastDisc();									// 割引済額取出し
// MH810100(E) 2020/09/02 再精算時の複数枚対応
// MH810100(S) 2020/06/30 【連動評価指摘事項】再精算時の割引適応方法の変更
		if(disFlag){
			// 戻す（データ作成の為に）
			discount = discount + last_disc;
		}
// MH810100(E) 2020/06/30 【連動評価指摘事項】再精算時の割引適応方法の変更
// MH810100(S) 2020/09/10 #4821 【連動評価指摘事項】1回の精算で同一店Noの施設割引を複数使用すると、再精算時に前回利用分としてカウントされる割引枚数が1枚となりNT-NET精算データに割引情報がセットされてしまう（No.02-0057）
		wk_ryo2 = last_disc;
		last_disc =	Ope_GetLastDiscOrg();
// MH810100(E) 2020/09/10 #4821 【連動評価指摘事項】1回の精算で同一店Noの施設割引を複数使用すると、再精算時に前回利用分としてカウントされる割引枚数が1枚となりNT-NET精算データに割引情報がセットされてしまう（No.02-0057）
// MH810100(S) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応
//		if( m_stDisc.Discount >= last_disc ){							// 既に割引済額を超えている？
		if( m_stDisc.Discount != 0 && m_stDisc.Discount >= last_disc ){							// 既に割引済額を超えている？
// MH810100(E) 2020/08/06 #4563【連動評価指摘事項】種別割引の順番対応
			// 今回割引額を全て新規割引とする
			req_p->data[0] &= 0xffff7fff;								// 割引済ビットを落とす
		}else{
			if( m_stDisc.Discount + discount > last_disc ){				// 今回割引で割引済額を超える？
				new_disc = ( m_stDisc.Discount + discount ) - last_disc;// 今回新規割引額算出
				tb_number++;											// 今回追加分割引は料金計算要求テーブルを分ける
				n_req_p =  &req_rhs[tb_number];
				memcpy( n_req_p, req_p, sizeof(req_rkn) );				// 現在料金計算要求テーブルコピー
				// 割引済分の料金計算要求テーブル修正
				req_p->data[2] = discount - new_disc;					// 割引済額 = 今回割引額 - 今回新規割引額
				req_p->data[3] = ryoukin + new_disc;					// 割引後額 = 割引後料金 + 今回新規割引額
// MH810100(S) 2020/06/15 車番チケットレス(#4229 【事業部レビュー指摘】前回半端な割引適応後、再精算で残りの割引が適応された場合、NT-NETで差額送信するが使用枚数は0としてほしい)
				req_p->data[1] = req_p->data[1] + 2000;					// フラグとして+2000
// MH810100(E) 2020/06/15 車番チケットレス(#4229 【事業部レビュー指摘】前回半端な割引適応後、再精算で残りの割引が適応された場合、NT-NETで差額送信するが使用枚数は0としてほしい)
				// 今回新規分割引分の料金計算要求テーブル修正
				n_req_p->data[0] &= 0xffff7fff;							// 割引済ビットを落とす
				n_req_p->data[2] = new_disc;							// 今回追加分割引額セット
// MH810100(S) 2020/06/15 車番チケットレス(#4229 【事業部レビュー指摘】前回半端な割引適応後、再精算で残りの割引が適応された場合、NT-NETで差額送信するが使用枚数は0としてほしい)
				n_req_p->data[1] = 1000;									// 0枚フラグとして+1000
// MH810100(E) 2020/06/15 車番チケットレス(#4229 【事業部レビュー指摘】前回半端な割引適応後、再精算で残りの割引が適応された場合、NT-NETで差額送信するが使用枚数は0としてほしい)
			}
// MH810100(S) 2020/06/30 【連動評価指摘事項】再精算時の割引適応方法の変更
// MH810100(S) 2020/09/10 #4821 【連動評価指摘事項】1回の精算で同一店Noの施設割引を複数使用すると、再精算時に前回利用分としてカウントされる割引枚数が1枚となりNT-NET精算データに割引情報がセットされてしまう（No.02-0057）
			last_disc = wk_ryo2;
// MH810100(E) 2020/09/10 #4821 【連動評価指摘事項】1回の精算で同一店Noの施設割引を複数使用すると、再精算時に前回利用分としてカウントされる割引枚数が1枚となりNT-NET精算データに割引情報がセットされてしまう（No.02-0057）
			if(( base_dis_flg == ON || disFlag == 1) && last_disc != 0){
				req_p->data[2] = last_disc;
				if(disFlag == 0){
					// RTデータ用に
					discount = last_disc;
				}
			}
// MH810100(E) 2020/06/30 【連動評価指摘事項】再精算時の割引適応方法の変更
		}
	}
	m_stDisc.Discount += (ulong)discount;
// MH810100(E) K.Onodera 2020/02/10 車番チケットレス(割引済み対応)
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
