/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：駐車料金額算出（定期帯）　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et4712(par1)　　　　　　　：　与えられた料金の取りかたにより、駐車料金の算出を行う。　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　料金の取りかた　昼夜帯　　　逓減帯　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　０：基本　　０／１　：第１パターン　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　１：追加　　２～１２：第２～１２パターン　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　料金の取りかた　　　　　　　無し　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/**********************************************/
#include	<string.h>													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"system.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"rkn_def.h"													/*　全デファイン統括　　　　　　　　　　　　　*/
#include	"rkn_cal.h"													/*　料金関連データ　　　　　　　　　　　　　　*/
#include	"rkn_fun.h"													/*　全サブルーチン宣言　　　　　　　　　　　　*/
#include	"Tbl_rkn.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	short	midni_ok;												/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	et4712( 														/*　駐車料金計算処理　　　　　　　　　　　　　*/
				short	ryo_pat											/*　料金の取りかた（昼夜帯、逓減帯別）　　　　*/
			  )															/*　　　　　　　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	struct	CAR_TIM		wok_in;											/*　ｗｏｒｋ時刻　　　　　　　　　　　　　　　*/
	short	flg = 1;													/*　ループｆｌｇ　　　　　　　　　　　　　　　*/
	short	ret = 0;													/*　リタンコード　　　　　　　　　　　　　　　*/
	char	FeeMaxType;													// 最大料金タイプ
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	memcpy( &wok_in,&car_in,7 );										/*　入庫時刻をｗｏｒｋ時刻にコピーする　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	car_type = req_rkn.syubt;											/*　計算要求テーブルの車種を参照する　　　　　*/
																		// 
	FeeMaxType = sp_mx;													// 
	if( EXP_FeeCal == 1 ){												// 
		FeeMaxType = sp_mx_exp[car_type-1];								// 
	}																	// 
																		// 
	if( (FeeMaxType != SP_MX_INTIME)&&(FeeMaxType < SP_MX_N_HOUR) )		/*　２４時間最大料金以外なら　　　　　　　　　*/
		total = 0;														/*　駐車料金集計用変数クリア　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	ot_time = (short)car_ot.hour * 60 + (short)car_ot.min;				/*　入庫時刻を分で表現　　　　　　　　　　　　*/
	memcpy( (char *)&cal_tm,(char *)&car_in,7 );						/*　入庫時刻を計算時刻とする　　　　　　　　　*/
																		// 
	if( pass_tm_flg == 1 ){												// 時間割引ではなく定期としてコールされた
		memcpy( (char *)&tki_fa,(char *)&car_in,7 );					// 定期有効開始時間を保持する
	}																	// 
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	befr_flag = OFF;													/*　今日の体系をセーブフラグセット　　　　　　*/
	if( FeeMaxType != SP_MX_612HOUR ){									/*　２４時間最大料金以外なら　　　　　　　　　*/
		rmax_cnt = 0;													/*　最大料金到達回数　　　　　　　　　　　　　*/
	}																	// 
	rmax_ptn = 0;														/*　最大料金到達時パタン　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	strt_ptn = ryo_pat;													/*　開始料金パタンセット　　　　　　　　　　　*/
	ptn_ccnt = 0;														/*　現在単位係数カウンタクリア　　　　　　　　*/
	cycl_ccnt = 0;														/*　現在繰り返し回数カウンタクリア　　　　　　*/
	sitei_flg = 0;														/*　指定時刻日毎最大フラグ　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	ret = ec64( &car_ot,&car_in );										/*　年月日時分比較処理　　　　　　　　　　　　*/
	if( ret == -1 )														/*　サービスタイム外出庫の場合　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		while( flg )													/*　計算時刻が出庫時刻に達するまで　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( cal_type == 0 )											/*　料金計算方式が０の場合　　　　　　　　　　*/
				et2100( ryo_pat );										/*　昼夜帯方式料金計算処理　　　　　　　　　　*/
			else														/*　料金計算方式が１の場合　　　　　　　　　　*/
				et2200( ryo_pat );										/*　逓減帯方式料金計算処理　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			befr_style = style;											/*　今日の体系をセーブ　　　　　　　　　　　　*/
			befr_flag = ON;												/*　今日の体系をセーブフラグセット　　　　　　*/
			if( (FeeMaxType == SP_MX_INTIME)||							// 
				(FeeMaxType == SP_MX_612HOUR)||							// 
				(FeeMaxType == SP_MX_N_HOUR) )							// 
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				chk_tim.hour = cal_tm.hour;								/*　現在計算時セーブ　　　　　　　　　　　　　*/
				chk_tim.min = cal_tm.min;								/*　現在計算分セーブ　　　　　　　　　　　　　*/
				chk_time = cal_time;									/*　現在時分セーブ　　　　　　　　　　　　　　*/
				if( FeeMaxType != SP_MX_612HOUR ){						/*　Ｎ時間最大料金指定以外　　　　　　　　　　*/
					h24time = h24time + (chk_time - wk_h24t);			/*　現在までの計算時間トータル算出　　　　　　*/
					wk_h24t = (short)chk_tim.hour * 60 + (short)chk_tim.min;/*　現在までの計算時間　　　　　　　　　　　　*/
				}														// 
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( FeeMaxType == SP_MX_NON )								/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				total = ryoukin + total;								/*　料金を集計中の駐車料金に加算　　　　　　　*/
				ryoukin = 0;											/*　料金を０クリア　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ec64( &cal_tm,&car_ot ) != 1 )							/*　出庫時刻に達している場合　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( (FeeMaxType != SP_MX_INTIME)&&						// 
					(FeeMaxType < SP_MX_N_HOUR) )						/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		// 
					switch( FeeMaxType ){								// 
																		// 
					case 2:												// 日毎最大の時
						total = ryoukin + total;						// 料金を集計中の駐車料金に加算
						ryoukin = total;								// 集計中の駐車料金の値を料金に代入
						break;											// 
																		// 
					case 3:												// ２種日毎最大の時
						jmax_ryo = jmax_ryo + total;					// ２種日毎最大確定料金のバックアップ
						break;											// 
																		// 
					default:											// 最大なし
						ryoukin = total;								// 集計中の駐車料金の値を料金に代入
						break;											// 
					}													// 
																		// 
				} else if ( FeeMaxType == SP_MX_612HOUR ){				/*　Ｎ時間最大料金指定	　　　　　　　　　　　*/
					chk_time = (int)car_ot.hour * 60 + (int)car_ot.min;	/*　現在時分セーブ　　　　　　　　　　　　*/
					if ( chk_time < wk_h24t ){							/*　　　　　　　　　　　　　　　　　　　　　　*/
						chk_time = chk_time + T_DAY;					/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					h24time = h24time + (chk_time - wk_h24t);			/*　現在までの計算時間トータル算出　　　　　　*/
					in_time = in_time % T_DAY;							/* 日毎最大基点時刻更新						  */
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				break;													/*　while ループを抜ける　　　　　　　　　　　*/
			} else if ( FeeMaxType == SP_MX_612HOUR ){					/*　計算継続 Ｎ時間最大料金指定	　　　　　　　*/
				h24time = h24time + (chk_time - wk_h24t);				/*　現在までの計算時間トータル算出　　　　　　*/
				wk_h24t = (int)chk_tim.hour * 60 + (int)chk_tim.min;	/*　現在までの計算時間　　　　　　　　　　　　*/
				in_time = in_time % T_DAY;								/* 日毎最大基点時刻更新						  */
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			tt_x = tt_x + total;										/*　時刻指定用ｗｏｒｋ　　　　　　　　　　　　*/
			tt_y = tt_y + ryoukin;										/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( (FeeMaxType == SP_MX_TIME)||(FeeMaxType == SP_MX_WTIME) )/*　　　　　　　　　　　　　　　　　　　　　　*/
				total = 0;												/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	memcpy( &car_in,&wok_in,7 );										/*　入庫時刻を本来の時刻に戻す　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	midni_ok = 0;														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
