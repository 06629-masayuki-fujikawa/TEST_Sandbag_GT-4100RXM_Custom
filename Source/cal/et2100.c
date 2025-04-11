short	et2101( short ) ;												/*　　　　　　　　　　　　　　　　　　　　　　*/
void	et2103( short *,short,short,struct CAR_TIM * );					/*　　　　　　　　　　　　　　　　　　　　　　*/
void	et2104( short,short,short,char *,short,short );					/*　　　　　　　　　　　　　　　　　　　　　　*/
void	et2105( short,char *,char,short );								/*　　　　　　　　　　　　　　　　　　　　　　*/
void	et2107( short *,short *,char *,char,struct CAR_TIM * );			/*　　　　　　　　　　　　　　　　　　　　　　*/
void	et2108( char *,short *,short );									/*　　　　　　　　　　　　　　　　　　　　　　*/
char	et2109( void );													/*　　　　　　　　　　　　　　　　　　　　　　*/
char	et2110( short *,short * );										/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：昼夜帯料金計算　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et2100()　　　　　　　　　：昼夜帯料金計算処理　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　基本／追加指定　　　　　　　無し　　　　　　　　　　　　　　：　*/
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
extern	char	h24_flg;												/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	char	tyu24_flg;												/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	char	max24_fg;												/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	et2100( 														/*　昼夜帯方式料金計算処理　　　　　　　　　　*/
short	pa_strt 														/*　基本／追加指定　　　　　　　　　　　　　　*/
)																		/*　　　　　　　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	kyoko;														/*　料金帯の残り時間　　　　　　　　　　　　　*/
	short	kyon1;														/*　料金帯の残り時間　　　　　　　　　　　　　*/
	short	rest;														/*　料金帯の残り時間　　　　　　　　　　　　　*/
	short	ogot_time;													/*　本来の出庫時刻（分換算）　　　　　　　　　*/
	short	first;														/*　その日最初の日付変更を表すフラグ　　　　　*/
	short	i;															/*　ループカウント　　　　　　　　　　　　　　*/
	short	next1;														/*　次料金帯の開始時刻　　　　　　　　　　　　*/
	volatile short	ret;														/*　サブルーチンのｒｅｔ　　　　　　　　　　　*/
	char	lp_flg;														/*　ループフラグ　　　　　　　　　　　　　　　*/
	char	et_ifl;														/*　一定料金帯フラグ　　　　　　　　　　　　　*/
	char	et_iflop;													/*　一定料金帯基本時間以内特殊フラグ　　　　　*/
																		/*　叉は、一定帯が次体系基本以内突入フラグ　　*/
	char	et_flg;														/*　重複料金帯フラグ(et2108)　　　　　　　　　*/
	char	et_nxdb;													/*　次体型開始に重複帯１有り　　　　　　　　　*/
	short	et_jkn;														/*　重複帯ＥＮＤ時刻　　　　　　　　　　　　　*/
	short	ck_len;														/*　一定料金帯フラグ　　　　　　　　　　　　　*/
	short	bnd_st;														/*　料金帯開始時刻　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	struct	CAR_TIM	in24_time;											// 定期帯に対応した入庫時刻
	char	N_style;													// 
	short	wk_time;													// 
	unsigned long	in_cal_tm;											// 
	unsigned long	wk_cal_tm;											// 
	char	FeeMaxType;													// 最大料金タイプ
	short	nmax_tim1;													// 
	short	nm_set1;													// 
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	car_type = (char)req_rkn.syubt;										/*　計算要求テーブルの車種を参照する　　　　　*/
	cal_time = (short)cal_tm.hour*60+(short)cal_tm.min;					/*　計算時刻構造体より計算時刻を算出　　　　　*/
	ogot_time = (short)car_ot.hour*60+(short)car_ot.min;				/*本来の出庫時刻構造体より本来の出庫時刻を算出*/
	band = 0;															/*　過去の料金帯クリア　　　　　　　　　　　　*/
	first = ON;															/*その日最初の日付変更を表すフラグをＯＮにする*/
	et_flg = OFF;														/*　重複料金帯フラグクリア　　　　　　　　　　*/
	et_nxdb = OFF;														/*　次体型開始に重複帯１有り　　　　　　　　　*/
	bnd_end_tm = 0;														/*　料金帯終了時刻をクリア　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	FeeMaxType = sp_mx;													// 
	nmax_tim1 = NMAX_HOUR1;												// ｎ時間最大1時間(全種別共通)
	nm_set1 = nmax_set1;												// ｎ時間最大回数(全種別共通)
	if( EXP_FeeCal == 1 ){												// 料金種別毎最大料金あり
		FeeMaxType = sp_mx_exp[car_type-1];								// 最大料金タイプ取得
		nmax_tim1 = EXP_NMAX_HOUR1[car_type-1];							// ｎ時間最大1時間(種別毎)
		nm_set1 = exp_nmax_set1[car_type-1];							// ｎ時間最大回数(種別毎)
	}																	// 
																		// 
	style = ec61( &cal_tm );											/*　今日の体系算出処理　　　　　　　　　　　　*/
	memcpy( (char *)&cardmy,(char *)&cal_tm,7 );						/*　現在計算時刻を保存する　　　　　　　　　　*/
	ec63( &cardmy );													/*　日を１日進める（次の体系開始時刻参照用）　*/
	next_style = ec61( &cardmy );										/*　次の日の体系算出処理　　　　　　　　　　　*/
	next_style_strt = bnd_strt[next_style-1][0] + T_DAY;				/*　次の料金体系開始時刻算出　　　　　　　　　*/
																		/******** 体系開始時刻以前の処理 **************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	ot_time = (short)car_ot.hour * 60 + (short)car_ot.min;				/*　出庫時刻セット　　　　　　　　　　　　　　*/
	if(( cal_tm.year != car_ot.year )||									/*　出庫時刻は今日でない　　　　　　　　　　　*/
		( cal_tm.day != car_ot.day )||									/*　　　　　　　　　　　　　　　　　　　　　　*/
		( cal_tm.mon != car_ot.mon ))									/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		ot_time += T_DAY;												/*　出庫時刻＋１４４０分　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(( cardmy.year == car_ot.year )&&								/*　出庫時刻は次体系と同じ日にち　　　　　　　*/
			( cardmy.day == car_ot.day )&&								/*　　　　　　　　　　　　　　　　　　　　　　*/
			( cardmy.mon == car_ot.mon ))								/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( (ot_time + T_DAY) > next_style_strt )					/*　出庫時刻は次体系開始時刻より大きい　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				ot_time += T_DAY;										/*　出庫時刻＋１４４０分　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( cal_time < bnd_strt[style-1][0] )								/*計算時刻（入庫時刻）が体系開始時刻以前の場合*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		next_style = style;												/*　次の料金体系を求める　　　　　　　　　　　*/
		next_style_strt = bnd_strt[style-1][0] + T_DAY;					/*　次の料金体系開始時刻算出　　　　　　　　　*/
		memcpy( (char *)&cardmy,(char *)&cal_tm,7 );					/*　現在計算時刻を保存する　　　　　　　　　　*/
		ec62( &cardmy );												/*　日を１日戻す　（前の体系開始時刻参照用）　*/
		style = ec61( &cardmy );										/*　前の日の体系算出処理　　　　　　　　　　　*/
		memcpy( (char *)&cardmy,(char *)&cal_tm,7 );					/*　cardmyを元に戻す　　　　　　　　　　　　　*/
		if ( FeeMaxType == SP_MX_612HOUR && in_time <= cal_time ){		/*　6,12時間最大料金指定	　　　　　　　　　*/
			in_time += T_DAY;											/*　　　　　　　　　　　　　　　　　　　　　　*/
			wk_h24t = wk_h24t + T_DAY;									/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		cal_time += T_DAY;												/*　計算時刻に２４時間加算する　　　　　　　　*/
		ot_time += T_DAY;												/*　出庫時刻に２４時間加算する　　　　　　　　*/
		if ( (FeeMaxType != SP_MX_612HOUR)&&(FeeMaxType < SP_MX_N_MH_NEW) ){	/*　6,12時間最大料金指定以外　　　　　　　　　*/
			wk_h24t = wk_h24t + T_DAY;									/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																// 
		first = OFF;													/*　最初の日付変更を表すフラグをＯＦＦにする　*/
		if( cal_time >= bnd_strt[style-1][0] + T_DAY )					/*　入車時刻が２度目の開始時刻を過ぎてる　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			sitei_flg = ON;												/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( ot_time == 0 )													/*　出庫時刻が０時００分なら　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		ot_time += T_DAY;												/*　出庫時刻＋１４４０分　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( max_bnd[style-1] == 0 )											/*◎5/22　　　　　　　　　　　　　　　　　　　*/
	{																	/*◎　　　　　　　　　　　　　　　　　　　　　*/
		ec63( &cal_tm );												/*　日を１日進める（次の体系開始時刻参照用）　*/
		cal_tm.hour = (char)(( next_style_strt - T_DAY ) / 60 );		/*◎　　　　　　　　　　　　　　　　　　　　　*/
		cal_tm.min = (char)(( next_style_strt - T_DAY ) % 60 );			/*◎　　　　　　　　　　　　　　　　　　　　　*/
		return;															/*◎　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*◎　　　　　　　　　　　　　　　　　　　　　*/
	if( (FeeMaxType == SP_MX_N_HOUR)&&(n_hour_style == 0) ){			/*　ｎ時間最大初回?(Y)　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		int  n_max_set = (short)CPrmSS[S_CAL][14];						/*　ｎ時間最大ｍ回の適用回数設定　　　　　　　*/
		n_hour_style = style;											// 
		if(( n_hour_cnt >= n_max_set )&&( n_max_set != 0 )){			// 
			chk_max1 = N_MAX_OVER;										// 
		}else{															// 
			chk_max1 = day_max1[n_hour_style-1][car_type-1];			// 
		}																// 
	}																	// 
																		// 
	if( ((FeeMaxType == SP_MX_N_MH_NEW)||(FeeMaxType == SP_MX_N_MHOUR_W))	// 
			&&(nmax_style == 0) ){										// ｎ時間最大初回?(Y)
																		// 
		nmax_style = style;												// ｎ時間最大の最大料金を取る料金体系
		chk_max1 = day_max1[nmax_style-1][car_type-1];					// ｎ時間最大料金
		chk_max2 = day_max2[nmax_style-1][car_type-1];					// 2種類目のｎ時間最大
																		// 
																		// 
// --------------------------------------------------------------------	// 
// 定期帯後のリセットした場合には、定期帯終了時刻がcal_tmとなる			// 
// 時間割引のみの場合には、nmax_inから時間割引を進めた時刻がcal_tmとなる// 
// 入庫基準の時のみ必要な処理											// 
// 時間割引分、入庫時刻を進める/進めないで分ける						// 
// --------------------------------------------------------------------	// 
																		// 
		if( (nmax_point == NMAX_IN_PIT)&&								// 入庫基準
			(nmax_tim_disc == 1) ){										// ｎ時間最大の基準時刻を時間割引分進めない
																		// 
			if( (nmax_pass_reset != 0)||								// 定期帯後にリセットあり
				((nmax_pass_reset == 0)&&(nmax_pass_calc == 1))||		// 定期帯後にリセットなし かつ 定期帯後は入庫時刻
																		// 
				((nmax_pass_reset == 0)&&(nmax_pass_calc == 0)&&		// 定期帯後にリセットなし かつ 定期帯後は定期帯終了時刻
				 (tki_flg != PAS_TIME)&&(pass_tm_flg == 0)) ){			// かつ 定期未使用 かつ 定期処理からのコールでない
																		// 
				memcpy( (char *)&in24_time, (char *)&cal_tm, 7 );		// 
				in24_time.hour = 0;										// 
				in24_time.min  = 0;										// 
				ec70( &in24_time,(long)cal_time );						// 
				if( ec71a( &cal_tm, &in24_time ) >= 1440L ){			// 
					ec62( &in24_time );									// 
				}														// 
				in_cal_tm = ec71a( &nmax_in, &in24_time );				// 実際の入庫時刻から何分経過したか算出する
				wk_cal_tm = in_cal_tm%nmax_tim1;						// 前回のｎ時間基点時刻から何分経過したか求める
				ec66( &in24_time, (long)wk_cal_tm );					// 前回のｎ時間基点時刻を求める
				if( ec64( &nmax_in, &in24_time ) != -1 ){				// 
					N_style = ec61( &in24_time );						// 入庫基準での24時間基点体系を求める
					wk_time = in24_time.hour * 60 + in24_time.min;		// 
					if( wk_time < bnd_strt[N_style-1][0] ){				// Ｎ時間最大基点時刻が体系開始時刻以前の場合
						ec66( &in24_time, (long)nmax_tim1 );			// もうひとつ前の基準時刻に戻す
						N_style = ec61( &in24_time );					// 入庫基準での24時間基点体系を求める
					}													// 
					chk_max1 = day_max1[N_style-1][car_type-1];			// ｎ時間最大料金
					chk_max2 = day_max2[N_style-1][car_type-1];			// 2種類目のｎ時間最大
				}														// 
			}															// 
			else if( (nmax_pass_reset == 0)&&(nmax_pass_calc == 0) ){	// 定期帯後にリセットなし かつ定期帯後は定期帯終了時刻
				if( tki_flg == PAS_TIME ){								// 時間帯定期使用済み
					N_style = ec61( &tki_fa );							// 定期帯終了時刻（料金発生時刻）の体系の最大料金とする
					chk_max1 = day_max1[N_style-1][car_type-1];			// ｎ時間最大料金
					chk_max2 = day_max2[N_style-1][car_type-1];			// 2種類目のｎ時間最大
				}														// 
			}															// 
		}																// 

		if(( nmax_cnt >= nm_set1 )&&( nm_set1 != 0 )){					// ｎ時間最大料金徴収回数制限ありで上限超え
			chk_max1 = NMAX_OVER;										// ｎ時間最大ｍ回経過後の最大料金
			chk_max2 = NMAX_OVER;										// 2種類目のｎ時間最大
		}																// ｍ回継続中または∞?(Y)
	}																	// 
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( ((FeeMaxType == SP_MX_INTIME)||(FeeMaxType == SP_MX_612HOUR))&&(h24time == 0) )	/*　２４時間最大チェックかつＩＮ＋２４すぎた　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		chk_max1 = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(( cal_tm.year == car_ot.year )&&								/*　出庫時刻は次体系と同じ日にち　　　　　　　*/
			( cal_tm.day == car_ot.day )&&								/*　　　　　　　　　　　　　　　　　　　　　　*/
			( cal_tm.mon == car_ot.mon ))								/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(( in_time + NTIME > next_style_strt )&&					/*　ＩＮ＋２４が次料金帯　かつ　　　　　　　　*/
						( ot_time > next_style_strt ))					/*　出庫時刻が次料金帯　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				chk_max1 = day_max1[next_style-1][car_type-1];			/*　２４時間最大料金を次の最大料金とする　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( cal_time > next_style_strt )							/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				chk_max1 = day_max1[next_style-1][car_type-1];			/*　２４時間最大料金を次の最大料金とする　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(( in_time + NTIME > next_style_strt)&&					/*　ＩＮ＋２４が次料金帯　かつ　　　　　　　　*/
						( cal_time > next_style_strt ))					/*　出庫時刻が次料金帯　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				chk_max1 = day_max1[next_style-1][car_type-1];			/*　２４時間最大料金を次の最大料金とする　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( chk_max1 == 0 )												/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			chk_max1 = day_max1[style-1][car_type-1];					/*　２４時間最大料金を今の最大料金とする　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/

																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( max_bnd[style-1] == 1 )											/*◎5/22　　　　　　　　　　　　　　　　　　　*/
	{																	/*◎　　　　　　　　　　　　　　　　　　　　　*/
		last_bnd = 1;													/*◎　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*◎　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for( i = 0; i < (max_bnd[style-1]*2)-1; i++ )					/*　第１料金帯から予備料金帯の分までループ　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(( next_style_strt > bnd_strt[style-1][i] )&&				/*　次料金体系開始時刻がその料金帯内の場合　　*/
				( next_style_strt <= bnd_strt[style-1][i+1] ))			/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				last_bnd = i+1;											/*ループカウントを次体系突入直前の料金帯とする*/
				break;													/*　ループを抜ける　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	lp_flg = LOOP;														/*　　　　　　　　　　　　　　　　　　　　　　*/
	bnd_st = bnd_strt[style-1][0];										/*料金帯開始時刻　　　　　　　　　　　　　　　*/
	if( ((FeeMaxType == SP_MX_INTIME)||(FeeMaxType == SP_MX_612HOUR)||	// 
		 (FeeMaxType == SP_MX_N_HOUR)||(FeeMaxType == SP_MX_BAND))&&	// 
		(rmax_cnt != 0)&&												/*24H最大で料金帯切換前に最大料金をすでに越え */
	  ((h24time + (cal_time - wk_h24t)) < T_DAY) )						/* 入庫からの時刻が24時間に到達していない　	　*/
	{																	/*　ramx_cntをｸﾘｱしない 　　　　			　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		rmax_cnt = 0;													/*　rmax_cntをｸﾘｱする 　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	rmax_ptn = 0;														/*　最大料金到達時パタン　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	while( lp_flg )														/*料金体系切換,絶対最大料金到達,出庫までループ*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		et_jiot = OFF;													/*　重複料金帯内入出庫フラグ(et2109)クリア　　*/
		et_ifl = OFF;													/*　一定料金帯フラグクリア　　　　　　　　　　*/
		et_iflop = OFF;													/*　一定料金帯基本時間以内特殊フラグ　　　　　*/
		et_dbl = OFF;													/*　重複料金帯フラグクリア　　　　　　　　　　*/
		first = et2101( first );										/*　計算分時刻セット　　　　　　　　　　　　　*/
																		/********** 料金帯を求める ********************/
		if( cal_time >= next_style_strt )								/*計算時刻が次体系開始時刻（補正済）以降の場合*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( et_nxdb == OFF )										/*　次体型開始に重複帯１有りでない　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( et_ifl == NOF )										/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					cal_time = next_style_strt;							/*　　　　　　　　　　　　　　　　　　　　　　*/
					first = et2101( first );							/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				memcpy( (char *)&cardmy,(char *)&cal_tm,7 );			/*　現在計算時刻を保存する　　　　　　　　　　*/
				ec63( &cardmy );										/*　日を１日進める（次の体系開始時刻参照用）　*/
				break;													/*　while ループを抜ける　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　次体型開始に重複帯１有り　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				et_nxdb = OFF;											/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( max_bnd[style-1] == 1)										/*◎5/22　　　　　　　　　　　　　　　　　　　*/
		{																/*◎　　　　　　　　　　　　　　　　　　　　　*/
			band = 1;													/*◎　　　　　　　　　　　　　　　　　　　　　*/
		}																/*◎　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			for( i = 0 ; i < (max_bnd[style-1]*2)-1 ; i++ )				/*　第１料金帯から予備料金帯の分までループ　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(( cal_time >= bnd_strt[style-1][i] )&&				/*　計算時刻がその料金帯内の場合　　　　　　　*/
					( cal_time < bnd_strt[style-1][i+1] ))				/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					band = (char)(i+1);									/*　ループカウントを料金帯とする　　　　　　　*/
					break;												/*　ループを抜ける　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		/*  定期後加算処理　 */											/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( amr_flg == T_TX_TIM )										/*　あまり加算処理有り　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( band > (char)max_bnd[style-1] )							/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				ck_len = leng_exp[style-1][band - max_bnd[style-1]-1];	/*　追加時間取り出し　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				ck_len = leng_exp[style-1][band-1];						/*　追加時間取り出し　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ck_len != 0 )											/*　一定料金帯でないなら　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				amr_flg = OFF;											/*　あまりフラグOFF 　　　　　　　　　　　　　*/
				cal_time += amartim;									/*　あまり時間加算　　　　　　　　　　　　　　*/
				first = et2101( first );								/*　計算分時刻セット　　　　　　　　　　　　　*/
				memcpy( (char *)&car_in,(char *)&cal_tm,7 );			/*　入庫時刻を変える　　　　　　　　　　　　　*/
				if( ec64( &cal_tm,&car_ot ) != 1 )						/*　時刻比較処理　計算時刻が出庫時刻以降の場合*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					break;												/*　　break 　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				continue;												/*　もう一度while　よりCHECK　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		/*  定期後加算処理　 */											/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		et_jiot = et2109();												/*　重複体内入出庫ＣＨＥＣＫ　　　　　　　　　*/
		if( et_jiot == ON )												/*　重複体内入出庫で重複０なら　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			band = (char)( band - 1 );									/*　現在料金帯を－１する　　　　　　　　　　　*/
			if( band == 0 )												/*　料金帯が０になったら　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				et_jiot = et2110( &last_bnd,&first );					/*　曜日を１日戻す　　　　　　　　　　　　　　*/
				if( et_jiot == ON )										/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					return;												/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			et_jiot = NOF;												/*　重複帯内入出庫フラグセット　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(( ec64(&cal_tm,&car_in) == 0 )&&( pa_strt == 0 ))			/*時刻比較処理（計算時刻が入庫時刻）かつ　　　*/
		{																/*　基本時間から計算なら　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			et2105( last_bnd,&et_ifl,et_jiot,bnd_st );					/*　基本料金算出　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			first = et2101( first );									/*　計算分時刻セット　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ( ret = ec64( &cal_tm,&car_ot )) != 1 )					/*時刻比較処理　計算時刻が仮出庫時刻以降の場合*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( band != (char)last_bnd )							/*　料金帯が次体系直前の料金帯以外の場合　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( ot_time <= bnd_strt[style-1][band] )			/*　次料金帯の開始時刻より小さい　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						break;											/*　while ループを抜ける　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　料金帯が次体系直前の料金帯の場合　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( ot_time <= next_style_strt )					/*　次料金帯の開始時刻より小さい　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						break;											/*　while ループを抜ける　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( sp_op == 0 )										/*　　継続なら　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( et_ifl == NOF )									/*　次料金帯が一定料金帯で基本時間以内　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						et_ifl = OFF;									/*　while ループを抜けず、フラグＯＦＦ　　　　*/
						et_iflop = ON;									/*　一定料金帯基本時間以内特殊フラグ　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					else 												/*　次料金帯が一定料金帯でないなら　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						break;											/*　while ループを抜ける　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( et_ifl == NOF )									/*　次料金帯が一定料金帯で基本時間以内　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						et_ifl = OFF;									/*　while ループを抜けず、フラグＯＦＦ　　　　*/
						et_iflop = ON;									/*　一定料金帯基本時間以内特殊フラグ　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				et2108( &et_flg,&et_jkn,last_bnd );						/*　現在料金帯の重複帯有無ＣＨＥＣＫ　　　　　*/
				if( et_flg == ON )										/*　重複帯有り　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( et_jiot == NOF )								/*　重複帯内入出庫　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						break;											/*　while ループを抜ける　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					if(( ogot_time <= et_jkn )&&( ot_time <= et_jkn ))	/*　重複帯内出庫か　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						break;											/*　while ループを抜ける　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( et_jkn >= T_DAY )								/*　２４時間を過ぎていたら　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						et_jkn = et_jkn - T_DAY;						/*　　出庫時刻と比較するために時分を合わす　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( band != (char)last_bnd )									/*　料金帯が次体系直前の料金帯以外の場合　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( band >= (char)max_bnd[style-1] )						/*　料金帯が仮想料金帯の場合　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				et2107( &kyoko,&rest,&et_dbl,et_jiot,&cardmy );			/*　　　　　　　　　　　　　　　　　　　　　　*/
				bnd_end_tm = kyoko;										/*　残り時間を料金帯終了時刻に設定　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　料金帯が実在料金帯の場合　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(( ovl_type[style-1][band] == 1 )||( et_jiot == NOF ))/*　重複方式＝１の場合か重複帯内入出庫の時　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					kyoko = bnd_strt[style-1][band]+					/*　料金帯の残り時間に重複時間を加算　　　　　*/
										ovl_time[style-1][band];		/*　　　　　　　　　　　　　　　　　　　　　　*/
					bnd_end_tm = kyoko;									/*　残り時間を料金帯終了時刻に設定　　　　　　*/
					et_dbl = NOF;										/*　重複方式１の時フラグセット　　　　　　　　*/
					if( kyoko >= cal_time )								/*　　　　　　　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						rest = kyoko - cal_time;						/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					else												/*　　　　　　　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						rest = 0;										/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					wk_strt = ovl_time[style-1][band];					/*　時刻指定最大checkに使用する加算時刻 　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　重複方式＝０の場合　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( ovl_time[style-1][band] != 0 )					/*　　　　　　　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						next1 = bnd_strt[style-1][band];				/*　次料金帯開始時刻　　　　　　　　　　　　　*/
						kyoko = next1 + ovl_time[style-1][band];		/*　料金帯の残り時間に重複時間を加算　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						kyon1 = ot_time;								/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(( kyon1 > next1 )&&( kyon1 <= kyoko ))		/*　出庫時刻が重複帯内　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							if(( cardmy.day == car_ot.day )&&			/*　計算時刻構造体の日が出庫日と同一かつ　　　*/
								( cardmy.mon == car_ot.mon )&&			/*　計算時刻構造体の月が出庫月と同一かつ　　　*/
								( cardmy.year == car_ot.year ))			/*　計算時刻構造体の年が出庫年と同一の場合　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								et_dbl = ON;							/*　　重複フラグセット　　　　　　　　　　　　*/
								wk_strt = ovl_time[style-1][band];		/*　時刻指定最大checkに使用する加算時刻 　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
							else if(( cal_tm.day == car_ot.day )&&		/*　計算時刻構造体の日が出庫日と同一かつ　　　*/
									( cal_tm.mon == car_ot.mon )&&		/*　計算時刻構造体の月が出庫月と同一かつ　　　*/
									( cal_tm.year == car_ot.year ))		/*　計算時刻構造体の年が出庫年と同一の場合　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								et_dbl = ON;							/*　　重複フラグセット　　　　　　　　　　　　*/
								wk_strt = ovl_time[style-1][band];		/*　時刻指定最大checkに使用する加算時刻 　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( et_dbl == OFF )									/*　重複無しの時　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						kyoko = bnd_strt[style-1][band];				/*　次料金帯の時間を求める　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					bnd_end_tm = kyoko;									/*　残り時間を料金帯終了時刻に設定　　　　　　*/
					if( kyoko >= cal_time )								/*　　　　　　　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
					 	rest = kyoko - cal_time;						/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					else												/*　　　　　　　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						rest = 0;										/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if((cal_time <= bnd_strt[style-1][band])					/*　次料金帯以下の追加を求める　　　　　　　　*/
								||(et_dbl != OFF)||(et_ifl == OFF))		/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				et2103( &rest,ot_time,kyoko,&cardmy );					/*　出庫迄時間計算　　　　　　　　　　　　　　*/
				if(( et_ifl != ON )||( et_jiot == NOF ))				/*　今回一定料金帯でなく重複帯内入出庫なら　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					next1 = bnd_strt[style-1][band];					/*　次料金帯開始時刻　　　　　　　　　　　　　*/
					et2104( kyoko,rest,last_bnd,&et_ifl,next1,bnd_st);	/*　追加料金算出　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( sp_mx_1 == ON )									/*　IN+24最大時cal_time調整フラグＯＮなら 　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						cal_time += T_DAY;								/*　cal_timeを取りあえずet2101終了まで+1440分 */
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				first = et2101( first );								/*　計算分時刻セット　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( sp_mx_1 == ON )										/*　IN+24最大時cal_time調整フラグＯＮなら 　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					cal_time -= T_DAY;									/*　cal_timeを取りあえずet2101終了まで+1440分 */
					sp_mx_1 = OFF;										/*　IN+24最大時cal_time調整フラグＯＦＦ　　　 */
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( ec64( &cal_tm,&car_ot ) != 1 )						/*時刻比較処理　計算時刻が仮出庫時刻以降の場合*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( ot_time <= bnd_strt[style-1][band] )			/*　次料金帯の開始時刻より小さい　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						break;											/*　while ループを抜ける　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					else												/*　次料金帯以上　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(( sp_op == 0 )&&( et_ifl == OFF ))			/*　　継続で次料金帯が一定料金帯でないなら　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							break;										/*　while ループを抜ける　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( amr_flg == NOF )							/*　一定料金帯余り加算後が出庫より大きい　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							break;										/*　while ループを抜ける　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					et2108( &et_flg,&et_jkn,last_bnd );					/*　現在料金帯の重複帯有無ＣＨＥＣＫ　　　　　*/
					if( et_flg == ON )									/*　重複帯有り　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( et_jiot == NOF )							/*　重複帯内入出庫　　　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							break;										/*　while ループを抜ける　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(( ogot_time<=et_jkn )&&( ot_time<=et_jkn ))	/*　重複帯内出庫か　　　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							break;										/*　while ループを抜ける　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( et_jkn >= T_DAY )							/*　２４時間を過ぎていたら　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							et_jkn = et_jkn - T_DAY;					/*　　出庫時刻と比較するために時分を合わす　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　料金帯が次体系直前の料金帯の場合　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(( ovl_type[next_style-1][0] == 1)||( et_jiot == NOF ))	/*　重複方式＝１の場合か重複帯内入出庫の時　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				kyoko = next_style_strt +								/*　料金帯の残り時間に重複時間を加算　　　　　*/
									ovl_time[next_style-1][0];			/*　　　　　　　　　　　　　　　　　　　　　　*/
				bnd_end_tm = kyoko;										/*　残り時間を料金帯終了時刻に設定　　　　　　*/
				et_dbl = NOF;											/*　重複方式１の時フラグセット　　　　　　　　*/
				if( kyoko >= cal_time )									/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					rest = kyoko - cal_time;							/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					rest = 0;											/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				wk_strt = ovl_time[next_style-1][0];					/*　時刻指定最大checkに使用する加算時刻 　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　重複方式＝０の場合　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( ovl_time[next_style-1][0] != 0 )					/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					next1 = next_style_strt;							/*　料金帯の残り時間に重複時間を加算　　　　　*/
					kyoko = next1 + ovl_time[next_style-1][0];			/*　料金帯の残り時間に重複時間を加算　　　　　*/
					kyon1 = ot_time;									/*　　　　　　　　　　　　　　　　　　　　　　*/
					if(( kyon1 > next1 )&&( kyon1 <= kyoko ))			/*　出庫時刻が重複体内　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(( cardmy.day == car_ot.day )&&				/*　計算時刻構造体の日が出庫日と同一かつ　　　*/
							( cardmy.mon == car_ot.mon )&&				/*　計算時刻構造体の月が出庫月と同一かつ　　　*/
							( cardmy.year == car_ot.year ))				/*　計算時刻構造体の年が出庫年と同一の場合　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							et_dbl = ON;								/*　　重複フラグセット　　　　　　　　　　　　*/
							wk_strt = ovl_time[next_style-1][0];		/*　時刻指定最大checkに使用する加算時刻 　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						else if(( cal_tm.day == car_ot.day )&&			/*　計算時刻構造体の日が出庫日と同一かつ　　　*/
								( cal_tm.mon == car_ot.mon )&&			/*　計算時刻構造体の月が出庫月と同一かつ　　　*/
								( cal_tm.year == car_ot.year ))			/*　計算時刻構造体の年が出庫年と同一の場合　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							et_dbl = ON;								/*　　重複フラグセット　　　　　　　　　　　　*/
							wk_strt = ovl_time[next_style-1][0];		/*　時刻指定最大checkに使用する加算時刻 　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( et_dbl == OFF )										/*　重複無しの時　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					kyoko = next_style_strt;							/*　次料金帯の時間を求める　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				bnd_end_tm = kyoko;										/*　残り時間を料金帯終了時刻に設定　　　　　　*/
				if( kyoko >= cal_time )									/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
				 	rest = kyoko - cal_time;							/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					rest = 0;											/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( (cal_time <= next_style_strt)							/*　次料金帯以下の追加を求める　　　　　　　　*/
								||(et_dbl != OFF)||(et_ifl == OFF) )	/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				et2103( &rest,ot_time,kyoko,&cardmy );					/*　出庫迄時間計算　　　　　　　　　　　　　　*/
				if(( et_ifl != ON )||( et_jiot == NOF ))				/*　今回一定料金帯でなく重複帯内入出庫なら　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					next1 = next_style_strt;							/*　料金帯の残り時間に重複時間を加算　　　　　*/
					et2104( kyoko,rest,last_bnd,&et_ifl,next1,bnd_st );	/*　追加料金算出　　　　　　　　　　　　　　　*/
					if( sp_mx_1 == ON )									/*　IN+24最大時cal_time調整フラグＯＮなら 　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						cal_time += T_DAY;								/*　cal_timeを取りあえずet2101終了まで+1440分 */
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				first = et2101( first );								/*　計算分時刻セット　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( sp_mx_1 == ON )										/*　IN+24最大時cal_time調整フラグＯＮなら 　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					cal_time -= T_DAY;									/*　cal_timeを取りあえずet2101終了まで+1440分 */
					sp_mx_1 = OFF;										/*　IN+24最大時cal_time調整フラグＯＦＦ　　　 */
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( ec64( &cal_tm,&car_ot ) != 1 )						/*時刻比較処理　計算時刻が仮出庫時刻以降の場合*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( ot_time <= next_style_strt )					/*　次料金帯の開始時刻より小さい　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						break;											/*　while ループを抜ける　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					else												/*　次料金帯以上　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(( sp_op == 0 )&&( et_ifl == OFF ))			/*　　継続で次料金帯が一定料金帯でないなら　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							break;										/*　while ループを抜ける　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( amr_flg == NOF )							/*　一定料金帯余り加算後が出庫より大きい　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							break;										/*　while ループを抜ける　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( et_ifl == ON )									/*　　　　　　　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						et_ifl = OFF;									/*　　　　　　　　　　　　　　　　　　　　　　*/
						et_iflop = ON;									/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					et2108( &et_flg,&et_jkn,last_bnd );					/*　現在料金帯の重複帯有無ＣＨＥＣＫ　　　　　*/
					if( et_flg == ON )									/*　重複帯有り　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( et_jiot == NOF )							/*　重複帯内入出庫　　　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							break;										/*　while ループを抜ける　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(( ogot_time<=et_jkn )&&( ot_time<=et_jkn ))	/*　重複帯内出庫か　　　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							break;										/*　while ループを抜ける　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( et_jkn >= T_DAY )							/*　２４時間を過ぎていたら　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							et_jkn = et_jkn - T_DAY;					/*　　出庫時刻と比較するために時分を合わす　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( (FeeMaxType == SP_MX_INTIME)||(FeeMaxType == SP_MX_612HOUR)||		// 
			(FeeMaxType == SP_MX_N_HOUR)||(FeeMaxType == SP_MX_BAND)||			// 
			(FeeMaxType == SP_MX_N_MH_NEW)||(FeeMaxType == SP_MX_N_MHOUR_W) )	// 
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			chk_tim.hour = cal_tm.hour;									/*　現在計算時セーブ　　　　　　　　　　　　　*/
			chk_tim.min = cal_tm.min;									/*　現在計算分セーブ　　　　　　　　　　　　　*/
			chk_time = cal_time;										/*　現在時分セーブ　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( (( sp_op == 1 ) && ( sp_it == 0 ))||						/*　余り処理方式＝１（切り捨て）の場合かつ　　*/
			(( sp_op == 1 ) && ( sp_it == 1 ) && ( amr_flg == OFF ))||	/*　sp_op=1 叉は　余り処理方式＝１（切り捨て）*/
			( et_iflop == ON ))											/*　の場合かつsp_op=1 叉は　基本時間内一定料金*/
																		/*　計算時　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( band == (char)last_bnd )								/*　料金帯が最高使用料金帯の場合　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(( FeeMaxType == SP_MX_NON )||						/*　最大時間が設定されていない　　　　　　　　*/
				   (( FeeMaxType != SP_MX_NON )&&						/*　最大時間が設定されていて計算時刻が　　　　*/
				    ( cal_time >= next_style_strt )))					/*　　　　　　　次料金体系開始時刻を越えたら　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					cal_time = next_style_strt;							/*　最大時間ワークも補正する　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　料金帯が最高使用料金帯以外の場合　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(( cal_tm.year == cardmy.year )&&						/*　計算時刻が次料金体系で　　　　　　　　　　*/
					( cal_tm.mon == cardmy.mon )&&						/*　　　　　　　　　　　　　　　　　　　　　　*/
					( cal_tm.day == cardmy.day ))						/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if(( cal_time >= T_DAY )&&							/*　現在時刻が２４時を過ぎていて　　　　　　　*/
						( bnd_strt[style-1][band] < T_DAY ))			/*　　次料金帯開始時刻は２４時を過ぎていない　*/
					{													/*　（日を越えたのに又日が戻る時）　　　　　　*/
						ec62( &cal_tm );								/*　　計算時刻を１日戻す　　　　　　　　　　　*/
						first = ON;										/*　　その日最初の日付変更要求フラグＯＮ　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( cal_time >= bnd_strt[style-1][band] )				/*　計算時刻が次料金帯開始時刻を超えていたら　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					cal_time = bnd_strt[style-1][band];					/*　次料金帯開始時刻を計算時刻とする　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( (FeeMaxType == SP_MX_INTIME)||(FeeMaxType == SP_MX_612HOUR)||		// 
				(FeeMaxType == SP_MX_N_HOUR)||(FeeMaxType == SP_MX_BAND)||			// 
				(FeeMaxType == SP_MX_N_MH_NEW)||(FeeMaxType == SP_MX_N_MHOUR_W) )	// 
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				first = et2101( first );								/*　計算分時刻セット　　　　　　　　　　　　　*/
				chk_tim.hour = cal_tm.hour;								/*　現在計算時セーブ　　　　　　　　　　　　　*/
				chk_tim.min = cal_tm.min;								/*　現在計算分セーブ　　　　　　　　　　　　　*/
				chk_time = cal_time;									/*　現在時分セーブ　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( et_dbl == NOF )											/*　重複方式１の時　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( band != (char)last_bnd )							/*　料金帯が最高使用料金帯の場合　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( band >= (char)max_bnd[style-1] )				/*　料金帯が仮想料金帯の場合　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						cal_time +=										/*　料金帯の残り時間に重複時間を加算　　　　　*/
							ovl_time[style-1][band-max_bnd[style-1]];	/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					else												/*　　　　　　　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						cal_time += ovl_time[style-1][band];			/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					cal_time += ovl_time[next_style-1][0];				/*　料金帯の残り時間に重複時間を加算　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(( next_style_strt <= cal_time )&&					/*　次料金計算時間が次体型開始時刻で　　　　　*/
					( ovl_type[next_style-1][0] == 1 ))					/*　　重複１タイプなら　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( band != (char)last_bnd )						/*　ラスト料金帯でないなら　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						cal_time = next_style_strt;						/*　　　　　　　　　　　　　　　　　　　　　　*/
						et_nxdb = ON;									/*　次体型開始に重複帯１有り　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			first = et2101( first );									/*計算分時刻セット　　　　　　　　　　　　　　*/
			if( ec64( &cal_tm,&car_ot ) != 1 )							/*時刻比較処理　計算時刻が仮出庫時刻以降の場合*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				break;													/*　while ループを抜ける　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( sp_op == 0 )												/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( et_dbl == NOF )											/*　重複方式１の時　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(( next_style_strt <= cal_time )&&					/*　次料金計算時間が次体型開始時刻で　　　　　*/
					( ovl_type[next_style-1][0] == 1 ))					/*　　重複１タイプなら　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( band != (char)last_bnd )						/*　ラスト料金帯でないなら　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						et_nxdb = ON;									/*　次体型開始に重複帯１有り　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　while ループ終端　　　　　　　　　　　　　*/
	return;																/*　昼夜帯方式料金計算処理を終了する　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：昼夜帯料金計算　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et2101()　　　　　　　　　：昼夜帯料金計算処理：計算分時刻セット　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/**********************************************/
short	et2101( 														/*　　　　　　　　　　　　　　　　　　　　　　*/
short	pa_fir															/*　　　　　　　　　　　　　　　　　　　　　　*/
)																		/*　　　　　　　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( cal_time >= 1440 )												/*　計算時刻が２４時以降の場合　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( pa_fir == ON )												/*　最初の日付変更を表すフラグがＯＮの場合　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			pa_fir = OFF;												/*　最初の日付変更を表すフラグをＯＦＦにする　*/
			ec63( &cal_tm );											/*　日付１日繰上げ処理　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		cal_tm.hour = (char)(( cal_time - 1440) / 60 );					/*　計算時刻（時）を計算時刻構造体に格納　　　*/
		cal_tm.min = (char)(( cal_time - 1440) % 60 );					/*　計算時刻（分）を計算時刻構造体に格納　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　計算時刻が２４時に達していない場合　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		cal_tm.hour = (char)( cal_time / 60 );							/*　計算時刻（時）を計算時刻構造体に格納　　　*/
		cal_tm.min = (char)( cal_time % 60 );							/*　計算時刻（分）を計算時刻構造体に格納　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( cal_tm.hour == 24 )												/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		cal_tm.hour = 0;												/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	return( pa_fir );													/*　リターンコードを返す　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：昼夜帯料金計算　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et2103()　　　　　　　　　：昼夜帯料金計算処理：出庫迄時間計算　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/**********************************************/
void	et2103( 														/*　　　　　　　　　　　　　　　　　　　　　　*/
short	*pa_res,														/*　　　　　　　　　　　　　　　　　　　　　　*/
short	pa_ott ,														/*　　　　　　　　　　　　　　　　　　　　　　*/
short	pa_kyo ,														/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	CAR_TIM		*pacar 												/*　　　　　　　　　　　　　　　　　　　　　　*/
)																		/*　　　　　　　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( pa_ott >= T_DAY )												/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(( pa_ott < pa_kyo )&&										/*　出庫時刻が次料金帯開始時刻より前でかつ　　*/
			( cal_time < pa_ott )&&										/*　計算時刻が出庫時刻より前でかつ　　　　　　*/
			( pacar->day == car_ot.day )&&								/*　計算時刻構造体の日が出庫日と同一かつ　　　*/
			( pacar->mon == car_ot.mon )&&								/*　計算時刻構造体の月が出庫月と同一かつ　　　*/
			( pacar->year == car_ot.year ))								/*　計算時刻構造体の年が出庫年と同一の場合　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			*pa_res = pa_ott - cal_time;								/*　出庫までの時間を料金帯の残り時間とする　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(( pa_ott < pa_kyo )&&										/*　出庫時刻が次料金帯開始時刻より前でかつ　　*/
			( cal_time < pa_ott )&&										/*　計算時刻が出庫時刻より前でかつ　　　　　　*/
			( cal_tm.day == car_ot.day )&&								/*　計算時刻構造体の日が出庫日と同一かつ　　　*/
			( cal_tm.mon == car_ot.mon )&&								/*　計算時刻構造体の月が出庫月と同一かつ　　　*/
			( cal_tm.year == car_ot.year ))								/*　計算時刻構造体の年が出庫年と同一の場合　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			*pa_res = pa_ott - cal_time;								/*　出庫までの時間を料金帯の残り時間とする　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：昼夜帯料金計算　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et2104()　　　　　　　　　：昼夜帯料金計算処理：追加料金算出　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　設計者（日付）　　：　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　無し　　　　　　　　　　　　無し　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　修正者（日付）　　：秋葉 (96/04/26) 　　　　　：・24H(重複帯,深夜帯)料金計算修正(MM131303)　　　　			  ：　*/
/**********************************************************************************************************************/
																		/**********************************************/
void	et2104( 														/*　　　　　　　　　　　　　　　　　　　　　　*/
short	pa_kyo ,														/*　次体系料金帯開始時刻(重複時間考慮してある)*/
short	pa_res ,														/*　　　　　　　　　　　　　　　　　　　　　　*/
short	pa_bnd ,														/*　次体系突入直前の料金帯　　　　　　　　　　*/
char	*pa_ifl,														/*　　　　　　　　　　　　　　　　　　　　　　*/
short	pa_nxt ,														/*　次体系料金帯開始時刻　　　　　　　　　　　*/
short	pa_str 															/*　料金帯開始時刻　　　　　　　　　　　　　　*/
)																		/*　　　　　　　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	short			et_adj;												/*　重複時間帯調整用変数　　　　　　　　　　　*/
	short			et_cnt;												/*　追加単位数　　　　　　　　　　　　　　　　*/
	short			et_len;												/*　追加時間　　　　　　　　　　　　　　　　　*/
	short			et_len2;											/*　基本時間　　　　　　　　　　　　　　　　　*/
	unsigned long	et_fee;												/*　追加料金　　　　　　　　　　　　　　　　　*/
	long			et_ryo;												/*　料金計算用　　　　　　　　　　　　　　　　*/
	short			et_nxt;												/*　次料金帯開始時刻　　　　　　　　　　　　　*/
	short			wrk;												/*　　　　　　　　　　　　　　　　　　　　　　*/
	short			bd;													/*　　　　　　　　　　　　　　　　　　　　　　*/
	short			wk_len	= 0;										/*　24H最大,一定料金帯有り,重複帯有り　　　　 */
	short			wk_len2 = 0;										/*　一定料金の取り方設定が1の時　　　　　　　 */
	short			itei_end = 0;										/* 一定料金帯重複時間の終わり 　　　　　　　　*/
	short			wk_otm	= 0;										/* 出庫時刻用ﾜｰｸ変数　　　　　　　　　　　　　*/
	short			add_ck = 0;											/* 24H余り時間料金算出用ﾌﾗｸﾞ　　　　　　　　　*/
	short			in_remain = 0;										/* 入庫時刻単位時間あたりの余り時間 　　　　　*/
	short			ot_remain = 0;										/* 出庫時刻単位時間当たりの余り時間 　　　　　*/
	short			wk_wrk;												// 
	short			n_cl_tm;											// 
	char			n_amr_flg;											// 
	short			wk_wrk2;											// 
	char	FeeMaxType;													// 最大料金タイプ
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	FeeMaxType = sp_mx;													// 
	if( EXP_FeeCal == 1 ){												// 
		FeeMaxType = sp_mx_exp[car_type-1];								// 
	}																	// 
	n_amr_flg = 0;				 
																		// 
	et_adj = 0;															/*　重複時間帯調整用変数クリア　　　　　　　　*/
	bd = band - max_bnd[style-1];										/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( band > (char)max_bnd[style-1] )									/*　料金帯が仮想料金帯の場合　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		et_len = leng_exp[style-1][bd-1];								/*　追加時間取り出し　　　　　　　　　　　　　*/
		et_len2 = leng_std[style-1][bd-1];								/*　基本時間を求める　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(( et_len == 0 )||( et_len2 == 0 ))							/*　一定料金帯の場合　　　　　　　　　　　　　*/
		{																/*　一定料金処理を行う　　　　　　　　　　　　*/
			et_len = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
			iti_flg = ON;												/*　一定料金帯フラグセット　　　　　　　　　　*/
			et_fee = (unsigned long)fee_std[style-1][car_type-1][bd-1];	/*　基本料金（一定料金）を求める　　　　　　　*/
			et_fee = et_fee * 10;										/*　追加料金を１０倍する　　　　　　　　　　　*/
			ryoukin += et_fee;											/*　追加料金を加算　　　　　　　　　　　　　　*/
			cal_time = pa_kyo;											/*　計算時刻を次の料金帯開始時刻にする　　　　*/
			if( amr_flg != OFF )										/*　一定料金帯余り時間加算処理有りの時　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				pa_kyo = cal_time + amartim;							/*　　計算時刻に一定料金帯余り時間加算　　　　*/
				cal_time = pa_kyo;										/*　計算時刻を次の料金帯開始時刻にする　　　　*/
				amr_flg = NOF;											/*　余りフラグリセット　　　　　　　　　　　　*/
				n_itei_adj = 1;		 
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( FeeMaxType != SP_MX_NON )								/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				tyu24_flg = 1;											/*　　　　　　　　　　　　　　　　　　　　　　*/
				cal_time = pa_kyo;										/*　計算時刻を次の料金帯開始時刻にする　　　　*/

				ec65( et_len,pa_str );									/*　日毎最大料金検査結果を求める　　　　　　　*/
				cal_time = pa_kyo;										/*　計算時刻を次の料金帯開始時刻にする　　　　*/
				tyu24_flg = 0;											/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( h24_flg == 1 ){										/*　　　　　　　　　　　　　　　　　　　　　　*/
					ryoukin += et_fee;									/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			wk_strt = 0;												/*　時刻指定最大check加算時刻クリア 　　　　　*/
			n_itei_adj = 0;			 
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　追加時間は有るか　　　　　　　　　　　　　*/
		{																/*　通常追加料金処理を行う　　　　　　　　　　*/
			n_itei_adj = 0;			 
			et_fee = (unsigned long)fee_exp[style-1][car_type-1][bd-1];	/*　追加料金を求める　　　　　　　　　　　　　*/
			et_fee = et_fee * 10;										/*　追加料金を１０倍する　　　　　　　　　　　*/
			iti_flg = OFF;												/*　一定料金帯フラグセット　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( FeeMaxType != SP_MX_NON )								/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/

				if( (FeeMaxType == SP_MX_N_MH_NEW)||					// ｎ時間ｍ回最大
					(FeeMaxType == SP_MX_N_MHOUR_W) ){					// 2種類のｎ時間最大
					for( wrk = 1440; wrk < (pa_res+1440); wrk+=et_len )	// 
					{													// 
						ryoukin += et_fee ;								// 追加料金を加算
						ec65( et_len,pa_str );							// 日毎最大料金検査結果を求める
						if( (FeeMaxType == SP_MX_N_MH_NEW)&&			// 
							(nmax_amari != 0) ){						// Ｎ時間最大で単位時間の余りあり
							if( (nmax_tani_type == 1)||					// 単位時間の余りを必ず切り捨てる
								(nmax_tani_type == 2) ){				// 最大料金を越えた場合は切り捨てる
																		// 
								n_amr_flg = 0;							// 
								wk_wrk = wrk - nmax_amari;				// 
 								if( sp_op == 1 ){ 						// 単位時間の切捨て
 									wk_wrk2 = wk_wrk+et_len;			// 
 								}else{									// 
 									wk_wrk2 = wk_wrk;					// 
 								}										// 
 								if( wk_wrk2 < (pa_res+1440) ){			// 
									wrk = wk_wrk;						// 単位時間徴収回数の調整
									n_cl_tm = cal_time;					// 
									if( cal_time <= nmax_amari ){		// 計算時刻が余り時間より小さい
										cal_time += 1440;				// 
									}									// 
									cal_time -= (short)nmax_amari;		// 計算時刻を余り時間分戻す
									n_amr_flg = 1;						// 
								}else{									// 
									nmax_amari = 0;						// 
								}										// 
							}											// 
						}												// 
						if( FeeMaxType == SP_MX_N_MHOUR_W ){			// 2種類のｎ時間最大
							if( (nmax_tani_type == 1)||					// 単位時間の余りを必ず切り捨てる
								(nmax_tani_type == 2) ){				// 最大料金を越えた場合は切り捨てる
																		// 
								if(nmax_amari != 0){					// ｎ時間最大(外)で単位時間の余りあり
									n_amr_flg = 0;						// 
									wk_wrk = wrk - nmax_amari;			// 
 									if( sp_op == 1 ){ 					// 単位時間の切捨て
 										wk_wrk2 = wk_wrk+et_len;		// 
 									}else{								// 
 										wk_wrk2 = wk_wrk;				// 
 									}									// 
 									if( wk_wrk2 < (pa_res+1440) ){		// 
										wrk = wk_wrk;					// 単位時間徴収回数の調整
										n_cl_tm = cal_time;				// 
										if( cal_time <= nmax_amari ){	// 計算時刻が余り時間より小さい
											cal_time += 1440;			// 
										}								// 
										cal_time -= (short)nmax_amari;	// 計算時刻を余り時間分戻す
										n_amr_flg = 1;					// 
									}else{								// 
										nmax_amari = 0;					// 
									}									// 
								}										// 
								else if( nmax_amari2 != 0 ){			// ｎ時間最大(内)で単位時間の余りあり
									n_amr_flg = 0;						// 
									wk_wrk = wrk - nmax_amari2;			// 
 									if( sp_op == 1 ){ 					// 単位時間の切捨て
 										wk_wrk2 = wk_wrk+et_len;		// 
 									}else{								// 
 										wk_wrk2 = wk_wrk;				// 
 									}									// 
 									if( wk_wrk2 < (pa_res+1440) ){		// 
										wrk = wk_wrk;					// 単位時間徴収回数の調整
										n_cl_tm = cal_time;				// 
										if( cal_time <= nmax_amari2 ){	// 計算時刻が余り時間より小さい
											cal_time += 1440;			// 
										}								// 
										cal_time -= (short)nmax_amari2;	// 計算時刻を余り時間分戻す
										n_amr_flg = 1;					// 
									}else{								// 
										nmax_amari2 = 0;				// 
									}									// 
								}										// 
							}											// 
						}												// 
					}													// 
				}else{													// 既存の最大料金計算系
					for( wrk = 0; wrk < pa_res; wrk+=et_len )			/*　　　　　　　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						ryoukin += et_fee ;								/*　追加料金を加算　　　　　　　　　　　　　　*/
						ec65( et_len,pa_str );							/*　日毎最大料金検査結果を求める　　　　　　　*/
						if( FeeMaxType == SP_MX_612HOUR && koetim != 0 ){	/*　　Ｎ時間超過				　　　　　*/
							wrk = wrk - koetim;							/*　Ｎ時間超過分の時間			　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														// 
				wk_strt = 0;											/*　時刻指定最大check加算時刻クリア 　　　　　*/
				if( (FeeMaxType == SP_MX_INTIME)&&						// 
					(ovl_type[style-1][bd] == 1)&&						/* 24H最大で,重複帯の取り方が設定１			  */
					(ovl_time[style-1][bd] != 0))						/* 次の料金帯が一定料金帯で重複あり 		  */
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					add_ck = 1;											/*　昼夜帯料金加算ﾁｪｯｸﾌﾗｸﾞON　　　　　　　　　*/
					wk_len = leng_exp[style-1][bd];						/*　追加時間取り出し　　　　　　　　　　　　　*/
					wk_len2 = leng_std[style-1][bd];					/*　基本時間を求める　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					wk_otm = ot_time;									/*　　　　　　　　　　　　　　　　　　　　　　*/
					if(ot_time >= T_DAY)								/*　出庫時刻が24Hを越えている 　　　　　　　　*/
						wk_otm -= T_DAY;								/*　出庫時刻－24H 　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					in_remain = in_time % et_len;						/*　単位料金あたりの入庫余り時間を算出　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if((add_ck == 1)&&(wk_len == 0)&&(wk_len2 == 0)			/* 次の料金帯が一定料金帯である 　　　　　　　*/
					&&(max24_fg == 1)&&									/* 重複帯の料金算出の設定が１				  */
					(in_remain != 0)&&(wk_otm != in_time))				/* 入庫の余り時間があり,入庫と出庫の時刻が違う*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					itei_end = bnd_strt[style-1][bd]					/*　一定料金帯の重複時間を求める　　　　　　　*/
								+ovl_time[style-1][bd];					/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					if((cal_time > bnd_strt[style-1][bd])&&				/* 計算時刻が重複帯内にあれば 　　　　　　　　*/
						(cal_time < itei_end))							/*　　　　　　　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						ot_remain = ot_time % et_len;					/* 出庫時刻の余り時間を算出 　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						if((ot_remain == 0)&&							/* 出庫の余り時間が０でかつ出庫時刻が 　　　　*/
							(ot_time != bnd_strt[style-1][bd]))			/*　　一定料金帯開始時刻と同じ時間でない時　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							ot_remain = et_len;							/* 出庫の余り時間を単位時間とする 　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						ryoukin += et_fee;								/* 単位時間あたりの料金を加算 　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						if((ot_time >= bnd_strt[style-1][bd])&&		 	/* 出庫時刻が一定料金帯内重複帯内の時 　　	  */
							(ot_time <= itei_end))						/*　　　　　　　　　　　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							if(ot_remain <= in_remain)					/* 入庫の余り時間より 　　　　　　　　　　　　*/
								ryoukin -= et_fee;						/* 先に加算した料金を引く 　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						else											/* 出庫時刻が一定料金帯重複帯以外の時 　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							if((cal_time-et_len)						/* 23:58-翌23:59の時に加算されるための回避　　*/
									< bnd_strt[style-1][bd])			/* 計算時刻-単位時間が一定料金帯開始時刻より前*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								if(itei_end <= T_DAY)					/* 一定料金帯重複帯終了時刻が0:00を越えてない */
								{											/*　　　　　　　　　　　　　　　　　　　　*/
									if((in_time >= bnd_strt[style-1][bd])	/* 入庫時刻が一定料金帯重複帯内の時 　　　*/
										&&(in_time <= itei_end))			/*　　　　　　　　　　　　　　　　　　　　*/
									{	
										}									/*　　　　　　　　　　　　　　　　　　　　*/
									else									/* 入庫時刻が一定料金帯重複帯以外の時 　　*/
									{										/*　　　　　　　　　　　　　　　　　　　　*/
										ryoukin -= et_fee;					/* 先に加算した料金を引く 　　　　　　　　*/
									}										/*　　　　　　　　　　　　　　　　　　　　*/
								}											/*　　　　　　　　　　　　　　　　　　　　*/
								else if(bnd_strt[style-1][bd] >= T_DAY)		/* 一定料金帯開始時刻が0:00を越えている 　*/
								{											/*　　　　　　　　　　　　　　　　　　　　*/
									if((in_time >= (bnd_strt[style-1][bd]-T_DAY))/* 入庫時刻が一定料金帯重複帯内の時  */
										&&(in_time <= (itei_end-T_DAY)))	/*　　　　　　　　　　　　　　　　　　　　*/
									{	
										}									/*　　　　　　　　　　　　　　　　　　　　*/
									else									/* 入庫時刻が一定料金帯重複帯以外の時 　　*/
									{										/*　　　　　　　　　　　　　　　　　　　　*/
										ryoukin -= et_fee;					/* 先に加算した料金を引く 　　　　　　　　*/
									}										/*　　　　　　　　　　　　　　　　　　　　*/
								}											/*　　　　　　　　　　　　　　　　　　　　*/
								else										/* 重複帯が0:00をまたいでいる 　　　　　　*/
								{											/*　　　　　　　　　　　　　　　　　　　　*/
									if((in_time >= bnd_strt[style-1][bd])	/* 入庫時刻が一定料金帯重複帯内の時 　　　*/
										||(in_time <= (itei_end-T_DAY)))	/*　　　　　　　　　　　　　　　　　　　　*/
									{	
										}									/*　　　　　　　　　　　　　　　　　　　　*/
									else									/* 入庫時刻が一定料金帯重複帯以外の時 　　*/
									{										/*　　　　　　　　　　　　　　　　　　　　*/
										ryoukin -= et_fee;					/* 先に加算した料金を引く 　　　　　　　　*/
									}										/*　　　　　　　　　　　　　　　　　　　　*/
								}											/*　　　　　　　　　　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　	  */
						n_itei_adj = 0;		 

						ec65( et_len,pa_str );							/* 時間の更新を行う　　　　　　　　　　　　　 */

					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( ( pa_res % et_len ) != 0 )							/*　料金帯あまり時間が発生した時　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					et_adj = 1;											/*　重複時間帯調整用変数を１とする　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				et_cnt = ( pa_res / et_len ) + et_adj;					/*　追加時間が何単位あるか計算　　　　　　　　*/
				et_ryo = (long)( et_cnt * et_fee );						/*　追加時間の料金を求める　　　　　　　　　　*/
				ryoukin = ryoukin + et_ryo;								/*　追加料金を加算　　　　　　　　　　　　　　*/
				cal_time = cal_time + ( et_len * et_cnt);				/*　計算時刻を追加回数×追加時間進める		　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　料金帯が実在料金帯の場合　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		et_len = leng_exp[style-1][band-1];								/*　追加時間取り出し　　　　　　　　　　　　　*/
		et_len2 = leng_std[style-1][band-1];							/*　基本時間を求める　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(( et_len == 0 )||( et_len2 == 0 ))							/*　一定料金帯の場合　　　　　　　　　　　　　*/
		{																/*　一定料金処理を行う　　　　　　　　　　　　*/
			et_len = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
			iti_flg = ON;												/*　一定料金帯フラグセット　　　　　　　　　　*/
			et_fee = (unsigned long)fee_std[style-1][car_type-1][band -1];	/*　基本料金（一定料金）を求める　　　　　*/
			et_fee = et_fee * 10;										/*　追加料金を１０倍する　　　　　　　　　　　*/
			ryoukin += et_fee;											/*　追加料金を加算　　　　　　　　　　　　　　*/
			cal_time = pa_kyo;											/*　計算時刻を次の料金帯開始時刻にする　　　　*/
			if( amr_flg != OFF )										/*　一定料金帯余り時間加算処理有りの時　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				pa_kyo = cal_time + amartim;							/*　　計算時刻に一定料金帯余り時間加算　　　　*/
				cal_time = pa_kyo;										/*　計算時刻を次の料金帯開始時刻にする　　　　*/
				amr_flg = NOF;											/*　余りフラグリセット　　　　　　　　　　　　*/
				n_itei_adj = 1;		 
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( FeeMaxType != SP_MX_NON )								/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				tyu24_flg = 1;											/*　　　　　　　　　　　　　　　　　　　　　　*/
				cal_time = pa_nxt;										/*　計算時刻を次の料金帯開始時刻にする　　　　*/
				cal_time = pa_kyo;										/*　計算時刻を次の料金帯開始時刻にする　　　　*/


				ec65( et_len,pa_str );									/* 時間の更新を行う　　　　　　　　　　　　　 */

				tyu24_flg = 0;											/*　　　　　　　　　　　　　　　　　　　　　　*/
				cal_time = pa_kyo;										/*　計算時刻を次の料金帯開始時刻にする　　　　*/
				if( h24_flg == 1 ){										/*　　　　　　　　　　　　　　　　　　　　　　*/
					ryoukin += et_fee;									/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			wk_strt = 0;												/*　時刻指定最大check加算時刻クリア 　　　　　*/
			n_itei_adj = 0;			// MH702200-T00 料金計算EG修正 09/07/22	// 
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　追加時間は有るか　　　　　　　　　　　　　*/
		{																/*　通常追加料金処理を行う　　　　　　　　　　*/
			n_itei_adj = 0;			// MH702200-T00 料金計算EG修正 09/07/22	// 
			iti_flg = OFF;												/*　一定料金帯フラグセット　　　　　　　　　　*/
			et_fee = (unsigned long)fee_exp[style-1][car_type-1][band-1];	/*　追加料金を求める　　　　　　　　　　　*/
			et_fee = et_fee * 10;										/*　追加料金を１０倍する　　　　　　　　　　　*/
			if( FeeMaxType != SP_MX_NON )								/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( (FeeMaxType == SP_MX_N_MH_NEW)||					// 
					(FeeMaxType == SP_MX_N_MHOUR_W) ){					// Ｎ時間最大、料金帯最大、時刻指定最大
																		// 
					for( wrk = 1440; wrk < (pa_res+1440); wrk+=et_len )	// 
					{													// 
						ryoukin += et_fee ;								// 追加料金を加算
						ec65( et_len,pa_str );							// 日毎最大料金検査結果を求める
						if( (FeeMaxType == SP_MX_N_MH_NEW)&&			// 
							(nmax_amari != 0) ){						// Ｎ時間最大で単位時間の余りあり
							if( (nmax_tani_type == 1)||					// 単位時間の余りを必ず切り捨てる
								(nmax_tani_type == 2) ){				// 最大料金を越えた場合は切り捨てる
																		// 
								n_amr_flg = 0;							// 
								wk_wrk = wrk - nmax_amari;				// 
 								if( sp_op == 1 ){ 						// 単位時間の切捨て
 									wk_wrk2 = wk_wrk+et_len;			// 
 								}else{									// 
 									wk_wrk2 = wk_wrk;					// 
 								}										// 
 								if( wk_wrk2 < (pa_res+1440) ){			// 
									wrk = wk_wrk;						// 単位時間徴収回数の調整
									n_cl_tm = cal_time;					// 
									if( cal_time <= nmax_amari ){		// 計算時刻が余り時間より小さい
										cal_time += 1440;				// 
									}									// 
									cal_time -= (short)nmax_amari;		// 計算時刻を余り時間分戻す
									n_amr_flg = 1;						// 
								}else{									// 
									nmax_amari = 0;						// 
								}										// 
							}											// 
						}												// 
						if( FeeMaxType == SP_MX_N_MHOUR_W ){			// 2種類のｎ時間最大
							if( (nmax_tani_type == 1)||					// 単位時間の余りを必ず切り捨てる
								(nmax_tani_type == 2) ){				// 最大料金を越えた場合は切り捨てる
																		// 
								if(nmax_amari != 0){					// ｎ時間最大(外)で単位時間の余りあり
									n_amr_flg = 0;						// 
									wk_wrk = wrk - nmax_amari;			// 
 									if( sp_op == 1 ){ 					// 単位時間の切捨て
 										wk_wrk2 = wk_wrk+et_len;		// 
 									}else{								// 
 										wk_wrk2 = wk_wrk;				// 
 									}									// 
 									if( wk_wrk2 < (pa_res+1440) ){		// 
										wrk = wk_wrk;					// 単位時間徴収回数の調整
										n_cl_tm = cal_time;				// 
										if( cal_time <= nmax_amari ){	// 計算時刻が余り時間より小さい
											cal_time += 1440;			// 
										}								// 
										cal_time -= (short)nmax_amari;	// 計算時刻を余り時間分戻す
										n_amr_flg = 1;					// 
									}else{								// 
										nmax_amari = 0;					// 
									}									// 
								}										// 
								else if( nmax_amari2 != 0 ){			// ｎ時間最大(内)で単位時間の余りあり
									n_amr_flg = 0;						// 
									wk_wrk = wrk - nmax_amari2;			// 
 									if( sp_op == 1 ){ 					// 単位時間の切捨て
 										wk_wrk2 = wk_wrk+et_len;		// 
 									}else{								// 
 										wk_wrk2 = wk_wrk;				// 
 									}									// 
 									if( wk_wrk2 < (pa_res+1440) ){		// 
										wrk = wk_wrk;					// 単位時間徴収回数の調整
										n_cl_tm = cal_time;				// 
										if( cal_time <= nmax_amari2 ){	// 計算時刻が余り時間より小さい
											cal_time += 1440;			// 
										}								// 
										cal_time -= (short)nmax_amari2;	// 計算時刻を余り時間分戻す
										n_amr_flg = 1;					// 
									}else{								// 
										nmax_amari2 = 0;				// 
									}									// 
								}										// 
							}											// 
						}												// 
					}													// 
				}else{													// 
					for( wrk = 0; wrk < pa_res; wrk+=et_len )				/*　　　　　　　　　　　　　　　　　　　　　　*/
					{														/*　　　　　　　　　　　　　　　　　　　　　　*/
						ryoukin += et_fee ;									/*　追加料金を加算　　　　　　　　　　　　　　*/
						ec65( et_len,pa_str );								/*　日毎最大料金検査結果を求める　　　　　　　*/
						if ( FeeMaxType == SP_MX_612HOUR && koetim != 0 ){	/*　　Ｎ時間超過				　　　　　　　*/
							wrk = wrk - koetim;								/*　Ｎ時間超過分の時間			　　　　　　　*/
						}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														// 
				wk_strt = 0;											/*　時刻指定最大check加算時刻クリア 　　　　　*/
				if( (FeeMaxType == SP_MX_INTIME)&&(ovl_type[style-1][band] == 1)&&		/* 24H最大で,重複帯の取り方が設定１			  */
					(ovl_time[style-1][band] != 0) ) 					/* 次の料金帯が一定料金帯で重複あり 		  */
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					add_ck = 1;											/*　昼夜帯料金加算ﾁｪｯｸﾌﾗｸﾞON　　　　　　　　　*/
					wk_len = leng_exp[style-1][band];					/*　追加時間取り出し　　　　　　　　　　　　　*/
					wk_len2 = leng_std[style-1][band];					/*　基本時間を求める　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					wk_otm = ot_time;									/*　　　　　　　　　　　　　　　　　　　　　　*/
					if(ot_time >= T_DAY)								/*　出庫時刻が24Hを越えている 　　　　　　　　*/
						wk_otm -= T_DAY;								/*　出庫時刻－24H 　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					in_remain = in_time % et_len;						/*　単位料金あたりの入庫余り時間を算出　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if((add_ck == 1)&&(wk_len == 0)&&(wk_len2 == 0)			/* 次の料金帯が一定料金帯である 　　　　　　　*/
					&&(max24_fg == 1)&&									/* 重複帯の料金算出の設定が１				  */
					(in_remain != 0)&&(wk_otm != in_time))				/* 入庫の余り時間があり,入庫と出庫の時刻が違う*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					itei_end = bnd_strt[style-1][band]					/*　一定料金帯の重複時間を求める　　　　　　　*/
								+ovl_time[style-1][band];				/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					if((cal_time > bnd_strt[style-1][band])&&			/* 計算時刻が重複帯内にあれば 　　　　　　　　*/
						(cal_time < itei_end))							/*　　　　　　　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						ot_remain = ot_time % et_len;					/* 出庫時刻の余り時間を算出 　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						if((ot_remain == 0)&&							/* 出庫の余り時間が０でかつ出庫時刻が 　　　　*/
							(ot_time != bnd_strt[style-1][band]))		/*　　一定料金帯開始時刻と同じ時間でない時　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							ot_remain = et_len;							/* 出庫の余り時間を単位時間とする 　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						ryoukin += et_fee;								/* 単位時間あたりの料金を加算 　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						if((ot_time >= bnd_strt[style-1][band])&&	 	/* 出庫時刻が一定料金帯内重複帯内の時 　　	  */
							(ot_time <= itei_end))						/*　　　　　　　　　　　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							if(ot_remain <= in_remain)					/* 入庫の余り時間より 　　　　　　　　　　　　*/
								ryoukin -= et_fee;						/* 先に加算した料金を引く 　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						else											/* 出庫時刻が一定料金帯重複帯以外の時 　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							if((cal_time-et_len)						/* 23:58-翌23:59の時に加算されるための回避　　*/
									< bnd_strt[style-1][band])			/* 計算時刻-単位時間が一定料金帯開始時刻より前*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								if(itei_end <= T_DAY)					/* 一定料金帯重複帯終了時刻が0:00を越えてない */
								{											/*　　　　　　　　　　　　　　　　　　　　*/
									if((in_time >= bnd_strt[style-1][band])	/* 入庫時刻が一定料金帯重複帯内の時 　　　*/
										&&(in_time <= itei_end))			/*　　　　　　　　　　　　　　　　　　　　*/
									{	
										}									/*　　　　　　　　　　　　　　　　　　　　*/
									else									/* 入庫時刻が一定料金帯重複帯以外の時 　　*/
									{										/*　　　　　　　　　　　　　　　　　　　　*/
										ryoukin -= et_fee;					/* 先に加算した料金を引く 　　　　　　　　*/
									}										/*　　　　　　　　　　　　　　　　　　　　*/
								}											/*　　　　　　　　　　　　　　　　　　　　*/
								else if(bnd_strt[style-1][band] >= T_DAY)	/* 一定料金帯開始時刻が0:00を越えている 　*/
								{											/*　　　　　　　　　　　　　　　　　　　　*/
									if((in_time >= (bnd_strt[style-1][band]-T_DAY))/* 入庫時刻が一定料金帯重複帯内の時*/
										&&(in_time <= (itei_end-T_DAY)))	/*　　　　　　　　　　　　　　　　　　　　*/
									{	
										}									/*　　　　　　　　　　　　　　　　　　　　*/
									else									/* 入庫時刻が一定料金帯重複帯以外の時 　　*/
									{										/*　　　　　　　　　　　　　　　　　　　　*/
										ryoukin -= et_fee;					/* 先に加算した料金を引く 　　　　　　　　*/
									}										/*　　　　　　　　　　　　　　　　　　　　*/
								}											/*　　　　　　　　　　　　　　　　　　　　*/
								else										/* 重複帯が0:00をまたいでいる 　　　　　　*/
								{											/*　　　　　　　　　　　　　　　　　　　　*/
									if((in_time >= bnd_strt[style-1][band])	/* 入庫時刻が一定料金帯重複帯内の時 　　　*/
										||(in_time <= (itei_end-T_DAY)))	/*　　　　　　　　　　　　　　　　　　　　*/
									{	
										}									/*　　　　　　　　　　　　　　　　　　　　*/
									else									/* 入庫時刻が一定料金帯重複帯以外の時 　　*/
									{										/*　　　　　　　　　　　　　　　　　　　　*/
										ryoukin -= et_fee;					/* 先に加算した料金を引く 　　　　　　　　*/
									}										/*　　　　　　　　　　　　　　　　　　　　*/
								}											/*　　　　　　　　　　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　	  */
						n_itei_adj = 0;		 
						ec65( et_len,pa_str );							/* 時間の更新を行う　　　　　　　　　　　　　 */

					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(( pa_res % et_len ) != 0 )							/*　料金帯のあまり時間が発生した場合　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					et_adj = 1;											/*　重複時間帯調整用変数を１とする　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				et_cnt = ( pa_res / et_len ) + et_adj;					/*　追加時間が何単位とれるか計算　　　　　　　*/
				et_ryo = (long)( et_cnt * et_fee );						/*　追加時間の料金を求める　　　　　　　　　　*/
				ryoukin = ryoukin + et_ryo;								/*　追加料金加算　　　　　　　　　　　　　　　*/
				cal_time = cal_time + ( et_len * et_cnt);				/*　計算時刻を追加回数×追加時間進める		　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( band == (char)pa_bnd )											/*　料金帯が最高使用料金帯の場合　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		et_nxt = next_style_strt;										/*　次料金体系開始時刻を計算時刻とする　　　　*/
		if( et_dbl == NOF ){											/*　　　　　　　　　　　　　　　　　　　　　　*/
			et_nxt += ovl_time[next_style-1][0];						/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　料金帯が最高使用料金帯以外の場合　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( band > (char)max_bnd[style-1] )								/*　料金帯が仮想料金帯の場合　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			et_nxt = bnd_strt[style-1][bd];								/*　次料金帯開始時刻を計算時刻とする　　　　　*/
			if( et_dbl == NOF ){										/*　　　　　　　　　　　　　　　　　　　　　　*/
				et_nxt += ovl_time[style-1][bd];						/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			et_nxt = bnd_strt[style-1][band];							/*　次料金帯開始時刻を計算時刻とする　　　　　*/
			if( et_dbl == NOF ){										/*　　　　　　　　　　　　　　　　　　　　　　*/
				et_nxt += ovl_time[style-1][band];						/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( et_nxt <= cal_time )											/*　計算時刻が次料金帯以上で一定料金帯のCKECK */
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( band == (char)pa_bnd )										/*　料金帯が最高使用料金帯の場合　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			et_len = leng_std[next_style-1][0];							/*　次料金帯の基本時間を求める　　　　　　　　*/
			et_len2 = leng_exp[next_style-1][0];						/*　追加時間取り出し　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( band >= (char)max_bnd[style-1] )						/*　料金帯が仮想料金帯の場合　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				et_len = leng_std[style-1][bd];							/*　次料金帯の基本時間を求める　　　　　　　　*/
				et_len2 = leng_exp[style-1][bd];						/*　追加時間取り出し　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　料金帯が実在料金帯の場合　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				et_len = leng_std[style-1][band];						/*　次料金帯の基本時間を求める　　　　　　　　*/
				et_len2 = leng_exp[style-1][band];						/*　追加時間を求める　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(( et_len == 0 )||( et_len2 == 0 ))							/*　次料金帯が一定料金帯の場合　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			*pa_ifl = ON ;												/*　一定料金帯フラグセット　　　　　　　　　　*/
			if( sp_it == 1 )											/*　時間余りを加算する　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( (FeeMaxType == SP_MX_N_MH_NEW)&&(nmax_amari != 0)&&	// 
					(n_amr_flg == 1) ){									// Ｎ時間最大で単位時間の余りあり
					if( (nmax_tani_type == 1)||							// 単位時間の余りを必ず切り捨てる
						(nmax_tani_type == 2) ){						// 最大料金を越えた場合は切り捨てる
						cal_time = n_cl_tm;								// 
					}													// 
				}														// 
				if( (FeeMaxType == SP_MX_N_MHOUR_W)&&					// 
					((nmax_amari != 0)||(nmax_amari2 != 0))&&			// 
					(n_amr_flg == 1) ){									// Ｎ時間最大で単位時間の余りあり
					if( (nmax_tani_type == 1)||							// 単位時間の余りを必ず切り捨てる
						(nmax_tani_type == 2) ){						// 最大料金を越えた場合は切り捨てる
						cal_time = n_cl_tm;								// 
					}													// 
				}														// 
				amartim = cal_time - et_nxt;							/*　時間余りを算出　　　　　　　　　　　　　　*/
				amr_flg = ON;											/*　余りフラグセット　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：昼夜帯料金計算　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et2105()　　　　　　　　　：昼夜帯料金計算処理：基本料金算出　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/**********************************************/
void	et2105(															/*　　　　　　　　　　　　　　　　　　　　　　*/
short	pa_bnd ,														/*　次体系突入直前の料金帯　　　　　　　　　　*/
char	*pa_ifl,														/*　　　　　　　　　　　　　　　　　　　　　　*/
char	pa_jiot,														/*　　　　　　　　　　　　　　　　　　　　　　*/
short	pa_str 															/*　料金帯開始時刻　　　　　　　　　　　　　　*/
)																		/*　　　　　　　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	short			et_len;												/*　基本時間　　　　　　　　　　　　　　　　　*/
	short			et_len2;											/*　基本時間　　　　　　　　　　　　　　　　　*/
	unsigned long	et_fee;												/*　基本料金　　　　　　　　　　　　　　　　　*/
	short			et_nxt;												/*　次料金帯開始時刻　　　　　　　　　　　　　*/
	short			bd;													/*　　　　　　　　　　　　　　　　　　　　　　*/
	char	FeeMaxType;													// 最大料金タイプ
	int		kyoko;														/*　料金帯の残り時間　　　　　　　　　　　　　*/
	int		kyon1;														/*　料金帯の残り時間　　　　　　　　　　　　　*/
	int		next1;														/*　次料金帯の開始時刻　　　　　　　　　　　　*/
	char	wk_et_dbl;													/*　重複フラグワーク　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	FeeMaxType = sp_mx;													// 
	if( EXP_FeeCal == 1 ){												// 
		FeeMaxType = sp_mx_exp[car_type-1];								// 
	}																	// 
	bd = band - max_bnd[style-1];										/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	iti_flg = OFF;														/*　一定料金帯フラグセット　　　　　　　　　　*/
	amr_flg = OFF;														/*　余りフラグリセット　　　　　　　　　　　　*/
	wk_et_dbl = OFF;													/*　重複フラグワークリセット　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( band > (char)max_bnd[style-1] )									/*　料金帯が仮想料金帯の場合　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		et_fee = (unsigned long)fee_std[style-1][car_type-1][bd-1];		/*　基本料金を求める　　　　　　　　　　　　　*/
		et_len = leng_std[style-1][bd-1];								/*　基本時間を求める　　　　　　　　　　　　　*/
		et_len2 = leng_exp[style-1][bd-1];								/*　追加時間取り出し　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　料金帯が実在料金帯の場合　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		et_fee = (unsigned long)fee_std[style-1][car_type-1][band-1];	/*　基本料金を求める　　　　　　　　　　　　　*/
		et_len = leng_std[style-1][band-1];								/*　基本時間を求める　　　　　　　　　　　　　*/
		et_len2 = leng_exp[style-1][band-1];							/*　追加時間取り出し　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	et_fee = et_fee * 10;												/*　基本料金を１０倍する　　　　　　　　　　　*/
	ryoukin += et_fee;													/*　駐車料金に基本料金を加える　　　　　　　　*/
	if( FeeMaxType == SP_MX_NON )										/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		cal_time = cal_time + et_len;									/*　計算時刻を基本時間分進める　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( FeeMaxType == SP_MX_TIME || FeeMaxType == SP_MX_WTIME )			/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( ((cal_time + et_len) >= (next_style_strt ) )||				/*時刻指定で基本時間加算で次体系か　　　　　　*/
			((cal_time + et_len) >= (bnd_strt[style-1][0]+T_DAY) ))		/*時刻指定で基本時間加算で次料金帯なら 　　　 */
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( (bnd_strt[style-1][0]+T_DAY < next_style_strt) &&		/*　重複帯内に次体系の開始がある　　　　　　　*/
				(next_style_strt < bnd_strt[style-1][0]+T_DAY+ovl_time[style-1][0]) )/*　　　　　　　　　　　　　　　 */
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( ovl_type[style-1][0] == 1 )							/*　重複１なら　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					wk_strt = next_style_strt -(bnd_strt[style-1][0]+T_DAY);/*　その差を重複とする　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( ovl_type[style-1][0] == 1 )							/*　重複１なら　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					wk_strt = ovl_time[style-1][0];						/*　重複時間を考慮　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if(( ot_time > next_style_strt )&&( ot_time <= next_style_strt + ovl_time[next_style-1][0])) /*	  */
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(( cardmy.day == car_ot.day )&&				/*　計算時刻構造体の日が出庫日と同一かつ　　　*/
							( cardmy.mon == car_ot.mon )&&				/*　計算時刻構造体の月が出庫月と同一かつ　　　*/
							( cardmy.year == car_ot.year ))				/*　計算時刻構造体の年が出庫年と同一の場合　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							wk_strt = ovl_time[next_style-1][0];		/*　時刻指定最大checkに使用する加算時刻 　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						else if(( cal_tm.day == car_ot.day )&&			/*　計算時刻構造体の日が出庫日と同一かつ　　　*/
								( cal_tm.mon == car_ot.mon )&&			/*　計算時刻構造体の月が出庫月と同一かつ　　　*/
								( cal_tm.year == car_ot.year ))			/*　計算時刻構造体の年が出庫年と同一の場合　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							wk_strt = ovl_time[next_style-1][0];		/*　時刻指定最大checkに使用する加算時刻 　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if(( et_len == 0 )||( et_len2 == 0 ))								/*　一定料金帯の場合（現在）　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		pa_jiot = pa_jiot;												/*　　　　　　　　　　　　　　　　　　　　　　*/
		et_len = 0;														/*　　　　　　　　　　　　　　　　　　　　　　*/
		iti_flg = ON;													/*　一定料金帯フラグセット　　　　　　　　　　*/
		*pa_ifl = ON;													/*　一定料金帯フラグセット（現在ならＯＮ）　　*/
		if( band == (char)pa_bnd )										/*　料金帯が最高使用料金帯の場合　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			cal_time = next_style_strt;									/*　次料金体系開始時刻を計算時刻とする　　　　*/
			cal_time += ovl_time[next_style-1][0];						/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(  ovl_type[next_style-1][0] == 0 )						/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				memcpy( (char *)&ren_tm,(char *)&org_ot,7 );			/*　現在計算時刻を保存する　　　　　　　　　　*/
				ec62( &ren_tm );										/*　日を１日戻す　（前の体系開始時刻参照用）　*/
				if(( cal_tm.day == org_ot.day )&&						/*　計算時刻構造体の日が出庫日と同一かつ　　　*/
					( cal_tm.mon == org_ot.mon ))						/*　計算時刻構造体の月が出庫月と同一かつ　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( ot_time > cal_time )							/*　　　　　　　　　　　　　　　　　　　　　　*/
						cal_time -= ovl_time[next_style-1][0];			/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if(( cal_tm.day != ren_tm.day )||					/*　計算時刻構造体の日が出庫日と同一かつ　　　*/
						( cal_tm.mon != ren_tm.mon ))					/*　計算時刻構造体の月が出庫月と同一かつ　　　*/
						cal_time -= ovl_time[next_style-1][0];			/*　　　　　　　　　　　　　　　　　　　　　　*/
					else if( ot_time > cal_time )						/*　　　　　　　　　　　　　　　　　　　　　　*/
						cal_time -= ovl_time[next_style-1][0];			/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　料金帯が最高使用料金帯以外の場合　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			cal_time = bnd_strt[style-1][band];							/*　次料金帯開始時刻を計算時刻とする　　　　　*/
			if( band >= (char)max_bnd[style-1] )						/*　料金帯が仮想料金帯の場合　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				cal_time += ovl_time[style-1][bd];						/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(  ovl_type[style-1][bd] == 0 )						/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					memcpy( (char *)&ren_tm,(char *)&org_ot,7 );		/*　現在計算時刻を保存する　　　　　　　　　　*/
					ec62( &ren_tm );									/*　日を１日戻す　（前の体系開始時刻参照用）　*/
					if(( cal_tm.day == org_ot.day )&&					/*　計算時刻構造体の日が出庫日と同一かつ　　　*/
						( cal_tm.mon == org_ot.mon ))					/*　計算時刻構造体の月が出庫月と同一かつ　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( ot_time > cal_time )						/*　　　　　　　　　　　　　　　　　　　　　　*/
							cal_time -= ovl_time[style-1][bd];			/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					else												/*　　　　　　　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(( cal_tm.day != ren_tm.day )||				/*　計算時刻構造体の日が出庫日と同一かつ　　　*/
							( cal_tm.mon != ren_tm.mon ))				/*　計算時刻構造体の月が出庫月と同一かつ　　　*/
							cal_time -= ovl_time[style-1][bd];			/*　　　　　　　　　　　　　　　　　　　　　　*/
						else if( ot_time > cal_time )					/*　　　　　　　　　　　　　　　　　　　　　　*/
							cal_time -= ovl_time[style-1][bd];			/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　料金帯が実在料金帯の場合　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				cal_time += ovl_time[style-1][band]; 					/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(  ovl_type[style-1][band] == 0 )						/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					memcpy( (char *)&ren_tm,(char *)&org_ot,7 );		/*　現在計算時刻を保存する　　　　　　　　　　*/
					ec62( &ren_tm );									/*　日を１日戻す　（前の体系開始時刻参照用）　*/
					if(( cal_tm.day == org_ot.day )&&					/*　計算時刻構造体の日が出庫日と同一かつ　　　*/
						( cal_tm.mon == org_ot.mon ))					/*　計算時刻構造体の月が出庫月と同一かつ　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( ot_time > cal_time )						/*　　　　　　　　　　　　　　　　　　　　　　*/
							cal_time -= ovl_time[style-1][band];		/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					else												/*　　　　　　　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(( cal_tm.day != ren_tm.day )||				/*　計算時刻構造体の日が出庫日と同一かつ　　　*/
							( cal_tm.mon != ren_tm.mon ))				/*　計算時刻構造体の月が出庫月と同一かつ　　　*/
							cal_time -= ovl_time[style-1][band];		/*　　　　　　　　　　　　　　　　　　　　　　*/
						else if( ot_time > cal_time )					/*　　　　　　　　　　　　　　　　　　　　　　*/
							cal_time -= ovl_time[style-1][band];		/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( FeeMaxType != SP_MX_NON )										/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( sp_mx == 3 )												/*　２種類の時刻指定日毎最大設定の時のみ　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ovl_type[style-1][band] == 0 )							/*　重複方式＝０の場合　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( ovl_time[style-1][band] != 0 )						/*　重複時間≠０　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					next1 = bnd_strt[style-1][band];					/*　次料金帯開始時刻　　　　　　　　　　　　　*/
					kyoko = next1 + ovl_time[style-1][band];			/*　次料金帯開始時刻に重複時間を加算　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					kyon1 = ot_time;									/*　　　　　　　　　　　　　　　　　　　　　　*/
					if(( kyon1 > next1 )&&( kyon1 <= kyoko ))			/*　出庫時刻が重複帯内　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(( cardmy.day == car_ot.day )&&				/*　計算時刻構造体の日が出庫日と同一かつ　　　*/
							( cardmy.mon == car_ot.mon )&&				/*　計算時刻構造体の月が出庫月と同一かつ　　　*/
							( cardmy.year == car_ot.year ))				/*　計算時刻構造体の年が出庫年と同一の場合　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							wk_et_dbl = ON;								/*　　重複フラグセット　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						else if(( cal_tm.day == car_ot.day )&&			/*　計算時刻構造体の日が出庫日と同一かつ　　　*/
								( cal_tm.mon == car_ot.mon )&&			/*　計算時刻構造体の月が出庫月と同一かつ　　　*/
								( cal_tm.year == car_ot.year ))			/*　計算時刻構造体の年が出庫年と同一の場合　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							wk_et_dbl = ON;								/*　　重複フラグセット　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( wk_et_dbl == OFF )									/*　重複無しの時　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					kyoko = bnd_strt[style-1][band];					/*　次料金帯の時間を求める　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				bnd_end_tm = kyoko;										/*　料金帯終了時刻に設定　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		n_itei_adj = 0;			 

		ec65( et_len,pa_str );											/*　日毎最大料金検査結果を求める　　　　　　　*/

		wk_strt = 0;													/*　時刻指定最大check加算時刻クリア 　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if(( et_len != 0 )&&( et_len2 != 0 ))								/*　一定料金帯でない場合（現在）　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( band == (char)pa_bnd )										/*　料金帯が最高使用料金帯の場合　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			et_nxt = next_style_strt;									/*　次料金体系開始時刻を計算時刻とする　　　　*/
			if( et_dbl == NOF ){										/*　　　　　　　　　　　　　　　　　　　　　　*/
				et_nxt += ovl_time[next_style-1][0];					/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　料金帯が最高使用料金帯以外の場合　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( band > (char)max_bnd[style-1] )							/*　料金帯が仮想料金帯の場合　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				et_nxt = bnd_strt[style-1][bd];							/*　次料金帯開始時刻を計算時刻とする　　　　　*/
				if( et_dbl == NOF ){									/*　　　　　　　　　　　　　　　　　　　　　　*/
					et_nxt += ovl_time[style-1][bd];					/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				et_nxt = bnd_strt[style-1][band];						/*　次料金帯開始時刻を計算時刻とする　　　　　*/
				if( et_dbl == NOF ){									/*　　　　　　　　　　　　　　　　　　　　　　*/
					et_nxt += ovl_time[style-1][band];					/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( et_nxt <= cal_time )										/*　計算時刻が次料金帯以上で一定料金帯のCKECK */
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( band == (char)pa_bnd )									/*　料金帯が最高使用料金帯の場合　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				et_len = leng_std[next_style-1][0];						/*　次料金帯の基本時間を求める　　　　　　　　*/
				et_len2 = leng_exp[next_style-1][0];					/*　追加時間取り出し　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( band >= (char)max_bnd[style-1] )					/*　料金帯が仮想料金帯の場合　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					et_len = leng_std[style-1][bd];						/*　次料金帯の基本時間を求める　　　　　　　　*/
					et_len2 = leng_exp[style-1][bd];					/*　追加時間取り出し　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　料金帯が実在料金帯の場合　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					et_len = leng_std[style-1][band];					/*　次料金帯の基本時間を求める　　　　　　　　*/
					et_len2 = leng_exp[style-1][band];					/*　追加時間を求める　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(( et_len == 0 )||( et_len2 == 0 ))						/*　次料金帯が一定料金帯の場合　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				*pa_ifl = NOF;											/*　一定料金帯フラグセット(次料金帯ならＮＯＦ)*/
				if( sp_it == 1 )										/*　時間余りを加算する　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					amartim = cal_time - et_nxt;						/*　時間余りを算出　　　　　　　　　　　　　　*/
					amr_flg = ON;										/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：昼夜帯料金計算　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et2107()　　　　　　　　　：昼夜帯料金計算処理　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/**********************************************/
void	et2107(															/*　　　　　　　　　　　　　　　　　　　　　　*/
short	*pa_kyo,														/*　　　　　　　　　　　　　　　　　　　　　　*/
short	*pa_res,														/*　　　　　　　　　　　　　　　　　　　　　　*/
char	*pa_dbl,														/*　　　　　　　　　　　　　　　　　　　　　　*/
char	pa_jio ,														/*　　　　　　　　　　　　　　　　　　　　　　*/
struct	CAR_TIM		*pa_dmy 											/*　　　　　　　　　　　　　　　　　　　　　　*/
)																		/*　　　　　　　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	et_no1;														/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	et_no2;														/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	et_no3;														/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	kyon1; 														/*　料金帯の残り時間　　　　　　　　　　　　　*/
	short	bd;															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	bd = band - max_bnd[style-1];										/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	et_no1 = (short)ovl_type[style-1][bd];								/*　　　　　　　　　　　　　　　　　　　　　　*/
	et_no2 = ovl_time[style-1][bd];										/*　　　　　　　　　　　　　　　　　　　　　　*/
	et_no3 = bnd_strt[style-1][band];									/*　次料金帯開始時刻　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if(( et_no1 == 1 )||( pa_jio == NOF ))								/*　重複方式＝１の場合か重複帯内入出庫の時　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		*pa_kyo = et_no3 + et_no2;										/*　料金帯の残り時間に重複時間を加算　　　　　*/
		*pa_dbl = NOF;													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( *pa_kyo >= cal_time )										/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			*pa_res = *pa_kyo - cal_time;								/*　料金帯の残り時間に重複時間を加算　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			*pa_res = 0;												/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( (bnd_strt[style-1][0]+T_DAY < next_style_strt) &&			/*　　　　　　　　　　　　　　　　　　　　　　*/
			(next_style_strt < *pa_kyo+T_DAY) )							/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			*pa_res = next_style_strt - cal_time;						/*　　　　　　　　　　　　　　　　　　　　　　*/
			*pa_kyo = next_style_strt;									/*　　　　　　　　　　　　　　　　　　　　　　*/
			wk_strt = next_style_strt -(bnd_strt[style-1][0]+T_DAY);	/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　重複方式＝０の場合　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( et_no2 != 0 )												/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			*pa_kyo = et_no3 + et_no2;									/*　料金帯の残り時間に重複時間を加算　　　　　*/
			kyon1 = ot_time;											/*　　出庫時刻＋１４４０分　　　　　　　　　　*/
			if(( kyon1 > et_no3 )&&( kyon1 <= *pa_kyo )) 				/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(( pa_dmy->day == car_ot.day )&&						/*　計算時刻構造体の日が出庫日と同一かつ　　　*/
					( pa_dmy->mon == car_ot.mon )&&						/*　計算時刻構造体の月が出庫月と同一かつ　　　*/
					( pa_dmy->year == car_ot.year ))					/*　計算時刻構造体の年が出庫年と同一の場合　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					*pa_dbl = ON;										/*　　重複フラグセット　　　　　　　　　　　　*/
					wk_strt = et_no2;									/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else if(( cal_tm.day == car_ot.day )&&					/*　計算時刻構造体の日が出庫日と同一かつ　　　*/
						( cal_tm.mon == car_ot.mon )&&					/*　計算時刻構造体の月が出庫月と同一かつ　　　*/
						( cal_tm.year == car_ot.year ))					/*　計算時刻構造体の年が出庫年と同一の場合　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					*pa_dbl = ON;										/*　　重複フラグセット　　　　　　　　　　　　*/
					wk_strt = et_no2;									/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				*pa_dbl = ON;											/*　　重複フラグセット　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( *pa_dbl == OFF )											/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			*pa_kyo = et_no3;											/*　料金帯の残り時間を求める　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( *pa_kyo >= cal_time )										/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
		 	*pa_res = *pa_kyo - cal_time;								/*　料金帯の残り時間を求める　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			*pa_res = 0;												/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：昼夜帯料金計算　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et2108()　　　　　　　　　：昼夜帯料金計算処理：現在料金帯の重複帯有無CHECK　 　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　設計者（日付）　　：　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　重複料金帯フラグ　　　　　　重複料金帯フラグ　　　　　　　　：　*/
/*　変更者（日付）　　：　　　　　　　　　　　　　：　重複帯ＥＮＤ時刻　　　　　　重複帯ＥＮＤ時刻　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/**********************************************/
void	et2108(															/*　　　　　　　　　　　　　　　　　　　　　　*/
char	*pa_flg,														/*　重複料金帯フラグ　　　　　　　　　　　　　*/
short	*pa_jkn,														/*　重複帯ＥＮＤ時刻　　　　　　　　　　　　　*/
short	 pa_lbd 														/*　　　　　　　　　　　　　　　　　　　　　　*/
)																		/*　　　　　　　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	bd;															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	bd = band - max_bnd[style-1];										/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( band != (char)pa_lbd )											/*　料金帯が次体系直前の料金帯以外の場合　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( band >= (char)max_bnd[style-1] )							/*　料金帯が仮想料金帯の場合　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ovl_type[style-1][bd] == 1 )							/*　重複方式＝１の場合　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				*pa_flg = ON;											/*　　　　　　　　　　　　　　　　　　　　　　*/
				*pa_jkn = bnd_strt[style-1][band+1-1] +					/*　次料金帯開始時間に重複時間を加算　　　　　*/
										ovl_time[style-1][bd];			/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　重複方式＝０の場合　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( ovl_time[style-1][bd] != 0 )						/*　重複時間が設定されている場合　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					*pa_flg = ON;										/*　　　　　　　　　　　　　　　　　　　　　　*/
					*pa_jkn = bnd_strt[style-1][band+1-1] +				/*　次料金帯開始時間に重複時間を加算　　　　　*/
											ovl_time[style-1][bd];		/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　料金帯が実在料金帯の場合　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ovl_type[style-1][band-1+1] == 1 )						/*　重複方式＝１の場合　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				*pa_flg = ON;											/*　　　　　　　　　　　　　　　　　　　　　　*/
				*pa_jkn = bnd_strt[style-1][band]+						/*　次料金帯開始時間に重複時間を加算　　　　　*/
									ovl_time[style-1][band];			/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　重複方式＝０の場合　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( ovl_time[style-1][band-1+1] != 0 )					/*　重複時間が設定されている場合　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					*pa_flg = ON;										/*　　　　　　　　　　　　　　　　　　　　　　*/
					*pa_jkn = bnd_strt[style-1][band]+					/*　次料金帯開始時間に重複時間を加算　　　　　*/
									ovl_time[style-1][band];			/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( ovl_type[next_style-1][0] == 1 )							/*　重複方式＝１の場合　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			*pa_flg = ON;												/*　　　　　　　　　　　　　　　　　　　　　　*/
			*pa_jkn = next_style_strt +									/*　次料金帯開始時間に重複時間を加算　　　　　*/
							ovl_time[next_style-1][0];					/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　重複方式＝０の場合　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ovl_time[next_style-1][0] != 0 )						/*　重複時間が設定されている場合　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				*pa_flg = ON;											/*　　　　　　　　　　　　　　　　　　　　　　*/
				*pa_jkn = next_style_strt +								/*　次料金帯開始時間に重複時間を加算　　　　　*/
								ovl_time[next_style-1][0];				/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：昼夜帯料金計算　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et2109()　　　　　　　　　：昼夜帯料金計算処理：重複帯入出車CHECK 　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　設計者（日付）　　：　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　入庫時間　　　　　　　　　　０　重複帯入出車である　　　　　：　*/
/*　変更者（日付）　　：　　　　　　　　　　　　　：　出庫時間　　　　　　　　　　ff　該当しない　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/**********************************************/
char	et2109()														/*　　　　　　　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	et_jytm;													/*　重複時間　　　　　　　　　　　　　　　　　*/
	short	et_ovtm;													/*　重複時間　　　　　　　　　　　　　　　　　*/
	short	et_jytp;													/*　重複タイプ　　　　　　　　　　　　　　　　*/
	short	et_strt;													/*　重複開始時間　　　　　　　　　　　　　　　*/
	char	flg;														/*　１日繰り越しフラグ　　　　　　　　　　　　*/
	char	ret_flg;													/*　リターンフラグ　　　　　　　　　　　　　　*/
	short	day;														/*　１日繰り越し　　　　　　　　　　　　　　　*/
	short	fun;														/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	bd;															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	bd = band - max_bnd[style-1];										/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	flg = OFF;															/*　　　リターンセット　　　　　　　　　　　　*/
	fun = OFF;															/*　　　リターンセット　　　　　　　　　　　　*/
	day = OFF;															/*　　　リターンセット　　　　　　　　　　　　*/
	ret_flg = OFF;														/*　　　リターンセット　　　　　　　　　　　　*/
	et_strt = bnd_strt[style-1][band-1];								/*　現在料金帯開始時刻　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( band > (char)max_bnd[style-1] )									/*　料金帯が仮想料金帯の場合　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		et_jytm = et_strt + ovl_time[style-1][bd-1];					/*　現在料金帯開始時刻に重複時間を加算　　　　*/
		et_ovtm = ovl_time[style-1][bd-1];								/*　　　　　　　　　　　　　　　　　　　　　　*/
		et_jytp = ovl_type[style-1][bd-1];								/*　現在料金帯重複タイプ　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　料金帯が実在料金帯の場合　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		et_jytm = et_strt + ovl_time[style-1][band-1];					/*　現在料金帯開始時刻に重複時間を加算　　　　*/
		et_ovtm = ovl_time[style-1][band-1];							/*　　　　　　　　　　　　　　　　　　　　　　*/
		et_jytp = ovl_type[style-1][band-1];							/*　現在料金帯重複タイプ　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if(( cardmy.year == car_ot.year )&&									/*　出庫時刻は次体系と同じ日にち　　　　　　　*/
		( cardmy.day == car_ot.day )&&									/*　　　　　　　　　　　　　　　　　　　　　　*/
		( cardmy.mon == car_ot.mon ))									/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		flg = ON;														/*　チェックフラグＯＮ　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	if(( cal_tm.year == car_ot.year )&&									/*　出庫時刻は今日と同じ　　　　　　　　　　　*/
		( cal_tm.day == car_ot.day )&&									/*　　　　　　　　　　　　　　　　　　　　　　*/
		( cal_tm.mon == car_ot.mon ))									/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		flg = ON;														/*　チェックフラグＯＮ　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( flg == ON )														/*　チェックフラグＯＮなら　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(( et_jytp == 0 )&&( et_ovtm != 0 ))							/*　重複タイプが０で重複時間有り　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(( ot_time >= T_DAY )&&((ot_time - T_DAY) >= in_time ))	/*　入庫から出庫までの時間調整　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				day = T_DAY;											/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( in_time < et_strt )										/*　　駐車時間が重複時間内　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				fun = et_strt - in_time;								/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(((ot_time-day)-in_time-fun<=et_ovtm)&&(ot_time<=et_jytm))/*　　駐車時間が重複時間内　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				ret_flg = ON;											/*　　重複時間内入出庫有りフラグセット　　　　*/
				wk_strt = et_ovtm;										/*　時刻指定最大check加算時刻　　　 　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	return( ret_flg );													/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：昼夜帯料金計算　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et2110()　　　　　　　　　：昼夜帯料金計算処理：前日入車変更　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　設計者（日付）　　：　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　入庫時間　　　　　　　　　　０　ＯＫ　　　　　　　　　　　　：　*/
/*　変更者（日付）　　：　　　　　　　　　　　　　：　出庫時間　　　　　　　　　　１　エラー　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/**********************************************/
char	et2110(															/*　　　　　　　　　　　　　　　　　　　　　　*/
short	*pa_lst,														/*　last_bnd　　　　　　　　　　　　　　　　　*/
short	*pa_fst 														/*　first　　　　　　　　　　　　　　　　　　 */
)																		/*　　　　　　　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	char	flg;														/*　リターンフラグ　　　　　　　　　　　　　　*/
	short	i;															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	flg = OFF;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	next_style = style;													/*　次の料金体系を求める　　　　　　　　　　　*/
	next_style_strt = bnd_strt[style-1][0] + T_DAY;						/*　次の料金体系開始時刻算出　　　　　　　　　*/
	memcpy( (char *)&cardmy,(char *)&cal_tm,7 );						/*　現在計算時刻を保存する　　　　　　　　　　*/
	ec62( &cardmy );													/*　日を１日戻す　（前の体系開始時刻参照用）　*/
	style = ec61( &cardmy );											/*　前の日の体系算出処理　　　　　　　　　　　*/
	memcpy( (char *)&cardmy,(char *)&cal_tm,7 );						/*　現在計算時刻を保存する　　　　　　　　　　*/
	cal_time += T_DAY;													/*　計算時刻に２４時間加算する　　　　　　　　*/
	*pa_fst = OFF;														/*　最初の日付変更を表すフラグをＯＦＦにする　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if(( cal_tm.year != car_ot.year )||									/*　出庫時刻は今日でない　　　　　　　　　　　*/
		( cal_tm.day != car_ot.day )||									/*　　　　　　　　　　　　　　　　　　　　　　*/
		( cal_tm.mon != car_ot.mon ))									/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		ot_time += T_DAY;												/*　出庫時刻＋１４４０分　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(( cardmy.year == car_ot.year )&&								/*　出庫時刻は次体系と同じ日にち　　　　　　　*/
			( cardmy.day == car_ot.day )&&								/*　　　　　　　　　　　　　　　　　　　　　　*/
			( cardmy.mon == car_ot.mon ))								/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( (ot_time + T_DAY) > next_style_strt )					/*　出庫時刻は次体系開始時刻より大きい　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				ot_time += T_DAY;										/*　出庫時刻＋１４４０分　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( max_bnd[style-1] == 0 )											/*◎5/22　　　　　　　　　　　　　　　　　　　*/
	{																	/*◎　　　　　　　　　　　　　　　　　　　　　*/
		ec63( &cal_tm );												/*　日を１日進める（次の体系開始時刻参照用）　*/
		cal_tm.hour = (char)(( next_style_strt - T_DAY ) / 60 );		/*◎　　　　　　　　　　　　　　　　　　　　　*/
		cal_tm.min = (char)(( next_style_strt - T_DAY ) % 60 );			/*◎　　　　　　　　　　　　　　　　　　　　　*/
		flg = ON;														/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*◎　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( flg == OFF )													/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for( i = 0; i < max_bnd[style-1]-1; i++ )						/*最高使用料金帯の１つ前の料金帯の分までループ*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( bnd_strt[style-1][i] > bnd_strt[style-1][i+1] )			/*料金帯開始時刻が次料金帯開始時刻より前の場合*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				bnd_strt[style-1][i+1] += T_DAY;						/*次料金帯開始時刻を換算（１４４０分加算）　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		for( i = 0; i < max_bnd[style-1]; i++ )							/*　最高料金帯～予備料金帯の分までループ　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			bnd_strt[style-1][i+max_bnd[style-1]]						/*　予備料金帯開始時刻に　　　　　　　　　　　*/
										=bnd_strt[style-1][i]+T_DAY;	/*　第１料金帯開始時刻を換算した時刻を代入　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		for( i = 0; i < (max_bnd[style-1]*2)-1; i++ )					/*　第１料金帯から予備料金帯の分までループ　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(( next_style_strt>bnd_strt[style-1][i] )&&				/*　次料金体系開始時刻がその料金帯内の場合　　*/
				( next_style_strt<=bnd_strt[style-1][i+1] ))			/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				*pa_lst = i+1;											/*ループカウントを次体系突入直前の料金帯とする*/
				break;													/*　ループを抜ける　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( max_bnd[style-1] == 1 )										/*◎　　　　　　　　　　　　　　　　　　　　　*/
		{																/*◎　　　　　　　　　　　　　　　　　　　　　*/
			*pa_lst = 1;												/*◎　　　　　　　　　　　　　　　　　　　　　*/
		}																/*◎　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		*pa_fst = et2101( *pa_fst );									/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/********** 料金帯を求める ********************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		for( i = 0; i < (max_bnd[style-1]*2)-1; i++ )					/*　第１料金帯から予備料金帯の分までループ　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(( cal_time >=bnd_strt[style-1][i] )&&					/*　計算時刻がその料金帯内の場合　　　　　　　*/
						( cal_time <bnd_strt[style-1][i+1] ))			/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				band = (char)(i+1);										/*　ループカウントを料金帯とする　　　　　　　*/
				if( cal_time == bnd_strt[style-1][i] )					/*　料金帯開始と同時刻なら(重複０タイプなので)*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					band = (char)(band - 1);							/*　前日MAX帯以上なので実在する料金帯とする 　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else if( next_style_strt == bnd_strt[style-1][i] )		/*　次体系開始が料金帯開始と同時刻なら　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					band = (char)(band - 1);							/*　前日MAX帯以上なので実在する料金帯とする 　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				break;													/*　ループを抜ける　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( max_bnd[style-1] == 1 )										/*◎5/22　　　　　　　　　　　　　　　　　　　*/
		{																/*◎　　　　　　　　　　　　　　　　　　　　　*/
			band = 1;													/*◎　　　　　　　　　　　　　　　　　　　　　*/
		}																/*◎　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	return( flg );														/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/**********************************************/
