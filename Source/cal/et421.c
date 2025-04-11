/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：通常時間割引額算出　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et421() 　　　　　　　　　：　通常の時間割引額の算出処理を行う。　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　無し　　　　　　　　　　　　割引額　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
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
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
long	et421()															/*　時間割引処理　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	struct	CAR_TIM		wok_in;											/*　時刻構造体 car_tim型構造帯名　ｗｏｒｋ入庫*/
	struct	CAR_TIM		wok_in2;										/*　時刻構造体 car_tim型構造帯名　ｗｏｒｋ２入庫*/
	short		ret 	= 0;											/*　リタンコード　　　　　　　　　　　　　　　*/
	short		ryo_pat = 0;											/*　料金の取りかた　　　　　　　　　　　　　　*/
// MH810103(S) R.Endo 2021/07/29 車番チケットレス フェーズ2.2 #5841 【検証課指摘事項】「QR割引券で種別切換して精算」 → 「ラグタイム後 かつ 追加料金発生前に再精算」 を行うと割引金額に不正な値がセットされる
//	long	disc;														/*　割引額　　　　　　　　　　　　　　　　　　*/
	long	disc = 0;													/*　割引額　　　　　　　　　　　　　　　　　　*/
// MH810103(E) R.Endo 2021/07/29 車番チケットレス フェーズ2.2 #5841 【検証課指摘事項】「QR割引券で種別切換して精算」 → 「ラグタイム後 かつ 追加料金発生前に再精算」 を行うと割引金額に不正な値がセットされる
	long	wok_ryo = 0;												/*　ｗｏｒｋ料金　　　　　　　　　　　　　　　*/
	char	car;														/*　車種　　　　　　　　　　　　　　　　　　　*/
	char	para_dt = 0;												/*　　　　　　　　　　　　　　　　　　　　　　*/
	struct	CAR_TIM		wok_in3;										/*　時刻構造体 car_tim型構造帯名　ｗｏｒｋ２入庫*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	para_dt	= req_rkn.param;											/*　計算要求テーブルよりパラメータを参照　　　*/
	ryo_pat = (short)Carkind_Param(WARI_PAY_PTN, rysyasu, 2,1);		/*　料金の取り方参照　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( ctyp_flg == 4 )													/*　計算方法４の場合　　　　　　　　　　　　　*/
		wok_ryo = base_ryo + ryo_buf_n.tax;								/*　時間割引対象額＝基本駐車料金＋駐車消費税　*/
	else																/*　計算方法１・２・３の場合　　　　　　　　　*/
		wok_ryo = base_ryo;												/*　時間割引対象額＝基本駐車料金　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/

	memcpy( &wok_in,&carin_mt,7 );										// 入庫時刻をｗｏｒｋ入庫時刻に保存
	memcpy ( &car_ot,&carot_mt,7 );										// 基本出庫時刻をＧ出庫時刻とする
																		// 
	if((req_rkn.param == RY_CSK) || (req_rkn.param == RY_CMI)){			// ｻｰﾋﾞｽ券、掛売中止券使用時
		memcpy( &wok_in2,&car_in,7 );									// 入庫時刻をｗｏｒｋ２入庫時刻に保存
		memcpy( &car_in,&recalc_carin,7 );								// 入車時刻を再計算用入庫時刻にする
	}																	// 
	ec70( &car_in,jik_dtm );											// 日付更新処理(加算)
	ret = ec64( &car_in,&car_ot );										/*　年月日時分比較処理　　　　　　　　　　　　*/
	switch ( ret )														/*　更新入庫時刻と出庫時刻の比較により分岐　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		case 0:															/*　更新入庫時刻＝出庫時刻　　　　　　　　　　*/
			disc = wok_ryo;												/*　割引額＝時間割引対象額　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		case 1:															/*　更新入庫時刻＜出庫時刻　　　　　　　　　　*/
			wok_ryo = base_ryo;											/*　　　　　　　　　　　　　　　　　　　　　　*/
			car = req_rkn.syubt;										/*　車種の保存　　　　　　　　　　　　　　　　*/
			req_rkn.syubt = rysyasu;									/*　車種セット　　　　　　　　　　　　　　　　*/
			et20( ryo_pat );											/*　料金算出処理を行う　　　　　　　　　　　　*/
			req_rkn.syubt = car;										/*　車種を戻す　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ryoukin >= wok_ryo )									/*　料金≧駐車料金（割引前）　　　　　　　　　*/
				disc = 0;												/*　割引額＝０　　　　　　　　　　　　　　　　*/
			else														/*　料金＜駐車料金（割引前）　　　　　　　　　*/
				disc = wok_ryo - ryoukin;								/*　割引額＝駐車料金－料金　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		case -1:														/*　更新入庫時刻＞出庫時刻　　　　　　　　　　*/
			if( jitu_wari == 1 )										/* 実割引の場合								*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				disc = wok_ryo;											/*　割引額＝時間割引対象額　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　実割引ではない場合　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				memcpy( &wok_in3,&car_in,7 );
				memcpy( &car_ot,&car_in,7 );							/*　時間割引後入庫時刻を出庫時刻とする　　　　*/
				if(( su_jik_dtm != 0 )&&( para_dt != 0 ))				/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					para_dt = 0;										/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( tki_flg == PAS_DISC ){							// 割引時間定期の時
						memcpy( &wok_in,&tki_tm,7 );
					}else{
					
						ec70( &wok_in,su_jik_dtm );						/*　日付更新処理　種別割引時間プラス　　　　　*/
					}
					memcpy( &car_in,&wok_in,7 );						/*　ｗｏｒｋ入庫時刻を入庫時刻とする　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					memcpy( &car_in,&carin_mt,7 );						/*　ｗｏｒｋ入庫時刻を入庫時刻とする　　　　　*/
					if( tki_flg == PAS_DISC ){							// 割引時間定期の時
						memcpy( &car_in,&tki_tm,7 );
					}
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				ryo_pat = 0;											/*　基本からとる　　　　　　　　　　　　　　　*/
				car = req_rkn.syubt;									/*　車種の保存　　　　　　　　　　　　　　　　*/
				req_rkn.syubt = rysyasu;								/*　車種セット　　　　　　　　　　　　　　　　*/
				et20( ryo_pat );										/*　料金算出処理を行う　　　　　　　　　　　　*/
				req_rkn.syubt = car;									/*　車種を戻す　　　　　　　　　　　　　　　　*/
				disc = ryoukin;											/*　割引額＝料金　　　　　　　　　　　　　　　*/
				memcpy( &car_in,&wok_in3,7);
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	if((req_rkn.param == RY_CSK) || (req_rkn.param == RY_CMI)){
		memcpy( &recalc_carin,&car_in,7 );								/* 入車時刻ｸﾘｱ					*/
		memcpy( &car_in,&wok_in2,7 );									/*　ｗｏｒｋ２入庫時刻を入庫時刻とする　　　　*/
	}
	return( disc );														/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
