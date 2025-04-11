void	et2201( short *);												/*　　　　　　　　　　　　　　　　　　　　　　*/
short	et2202( short );												/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：逓減帯料金計算　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et2200()　　　　　　　　　：逓減帯方式料金計算処理　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　開始パタン指定　　　　　　　無し　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
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
#include	"common.h"
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
extern	short	midni_ok;												/*　　　　　　　　　　　　　　　　　　　　　　*/
extern char	h24_mid;													/*　　　　　　　　　　　　　　　　　　　　　　*/
extern char	teig_mid;													/*　逓減帯24H最大加算ﾌﾗｸﾞ　　　　　　　　　　 */
extern char	teig_cycl;													/* 逓減帯24H最大ﾌﾗｸﾞ,昼夜帯には影響無し		  */
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	et2200( 														/*　逓減帯方式料金計算処理　　　　　　　　　　*/
short	pa_ptn															/*　開始パタン指定　　　　　　　　　　　　　　*/
)																		/*　　　　　　　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	short			loop_out;											/*　ループを抜けるべきか見分ける変数　　　　　*/
	short			first;												/*　その日最初の日付変更を表すフラグ　　　　　*/
	short			first_flg;											/*　その日の入車でも開始以前フラグ　　　　　　*/
	short			et_len;												/*　パターン単位料金　　　　　　　　　　　　　*/
	short			midflg;												/*　深夜帯有無フラグ　　　　　　　　　　　　　*/
	short			tgn_strt;											/*　逓減帯開始時刻　　　　　　　　　　　　　　*/
	unsigned long	et_fee;												/*　パターン単位料金　　　　　　　　　　　　　*/
	short			kasan_flg;											/*　1体系に２回開始時刻有り 　　　　　　　　　*/
	short 			cyclt1;												/*　　　　　　　　　　　　　　　　　　　　　　*/
	short			cyclt2;												/*　　　　　　　　　　　　　　　　　　　　　　*/
	short			chk_ptn = 0;										/*　　　　　　　　　　　　　　　　　　　　　　*/
	char			sumi = 0;											/*　　　　　　　　　　　　　　　　　　　　　　*/
	char			FeeMaxType;											// 最大料金タイプ
/**********************************************************************************************************************/
	h24_mid	= 0;														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	pa_ptn = 0;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	CMN_UNUSED_PARAMETER(pa_ptn);
	if( strt_ptn == 0 )													/*　一定料金帯後、叉は開始パターンが０の場合　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		strt_ptn = 1;													/*　開始パターンを１とする　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	cal_time = (short)cal_tm.hour * 60 + (short)cal_tm.min;				/*　計算時刻構造体より計算時刻を算出　　　　　*/
	first = ON;															/*その日最初の日付変更を表すフラグをＯＮにする*/
	first_flg = OFF;													/*　その日の入車でも開始以前フラグ　　　　　　*/
	kasan_flg = OFF;													/*　1体系に２回開始時刻有り　　　　 　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	FeeMaxType = sp_mx;													// 
	if( EXP_FeeCal == 1 ){												// 拡張料金計算有り
		FeeMaxType = sp_mx_exp[car_type-1];								// 最大料金タイプ（料金種別毎）
	}																	// 
																		// 
	style = ec61( &cal_tm );											/*　今日の体系算出処理　　　　　　　　　　　　*/
	memcpy( (char *)&cardmy,(char *)&cal_tm,7 );						/*　現在計算時刻を保存する　　　　　　　　　　*/
	ec63( &cardmy );													/*　日を１日進める（次の体系開始時刻参照用）　*/
	next_style = ec61( &cardmy );										/*　次の日の体系算出処理　　　　　　　　　　　*/
	next_style_strt = teigen_strt[next_style-1] + T_DAY;				/*　次の料金体系開始時刻算出　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	ot_time = (short)car_ot.hour * 60 + (short)car_ot.min;				/*　出庫時刻セット　　　　　　　　　　　　　　*/
	if(( cal_tm.year != car_ot.year )||									/*　出庫時刻は今日でない　　　　　　　　　　　*/
			( cal_tm.day != car_ot.day )||								/*　　　　　　　　　　　　　　　　　　　　　　*/
			( cal_tm.mon != car_ot.mon ))								/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		ot_time += T_DAY;												/*　出庫時刻＋１４４０分　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(( cardmy.year == car_ot.year )&&								/*　出庫時刻は次体系と同じ日にち　　　　　　　*/
				( cardmy.day  == car_ot.day )&&							/*　　　　　　　　　　　　　　　　　　　　　　*/
				( cardmy.mon  == car_ot.mon ))							/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( (ot_time + T_DAY) > next_style_strt )					/*　出庫時刻は次体系開始時刻より大きい　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				ot_time += T_DAY;										/*　出庫時刻＋１４４０分　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( cal_time < teigen_strt[style-1] )								/*計算時刻（入庫時刻）が体系開始時刻以前の場合*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		next_style = style;												/*　次の料金体系を求める　　　　　　　　　　　*/
		next_style_strt = teigen_strt[style-1] + T_DAY;					/*　次の料金体系開始時刻算出　　　　　　　　　*/
		memcpy( (char *)&cardmy,(char *)&cal_tm,7 );					/*　現在計算時刻を保存する　　　　　　　　　　*/
		ec62( &cardmy );												/*　日を１日戻す　（前の体系開始時刻参照用）　*/
		style = ec61( &cardmy );										/*　前の日の体系算出処理　　　　　　　　　　　*/
		memcpy( (char *)&cardmy,(char *)&cal_tm,7 );					/*　現在計算時刻を保存する　　　　　　　　　　*/
		if( FeeMaxType == SP_MX_612HOUR && in_time <= cal_time ){		/*　Ｎ時間最大料金指定	　　　　　　　　　　　*/
			in_time += T_DAY;											/*　　　　　　　　　　　　　　　　　　　　　　*/
			wk_h24t = wk_h24t + T_DAY;									/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		ot_time += T_DAY;												/*　出庫時刻に２４時間加算する　　　　　　　　*/
		if( FeeMaxType != SP_MX_612HOUR ){								/*　Ｎ時間最大料金指定以外　　　　　　　　　　*/
			wk_h24t = wk_h24t + T_DAY;									/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																// 
		first = OFF;													/*　最初の日付変更を表すフラグをＯＦＦにする　*/
		first_flg = ON;													/*　その日の入車でも開始以前フラグ　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( (style != next_style) && (style_type[style-1] == 2))		/*　次体系と現体系が異なり深夜帯が有る　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			kasan_flg = 3;												/*　調整フラグセット　　　　　　　　　　　　　*/
			if( (teigen_strt[style-1] < teigen_strt[next_style-1]) &&	/*　次体系の開始以前に現体系の開始が２回有る　*/
				( cal_time >= teigen_strt[style-1]))					/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				midnig_strt[style-1] += T_DAY;							/*　深夜帯開始時刻に１４４０加算　　　　　　　*/
				kasan_flg = ON;											/*　調整フラグセット　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		cal_time = cal_time + T_DAY;									/*　計算時刻に２４時間加算する　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( style_type[style-1] == 0 )										/*　その体系は使用しない　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		ec63( &cal_tm );												/*　日を１日進める（次の体系開始時刻参照用）　*/
		cal_tm.hour = (char)(( next_style_strt - T_DAY ) / 60 );		/*◎　　　　　　　　　　　　　　　　　　　　　*/
		cal_tm.min = (char)(( next_style_strt - T_DAY ) % 60 );			/*◎　　　　　　　　　　　　　　　　　　　　　*/
		return;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	if(( befr_style != style )&&( befr_flag == ON ))					/*　前日の体型有りで今日と違う体型なら　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		strt_ptn = 1;													/*　開始料金パタンセット　　　　　　　　　　　*/
		ptn_ccnt = 0;													/*　現在単位係数カウンタクリア　　　　　　　　*/
		cycl_ccnt = 0;													/*　現在繰り返し回数カウンタクリア　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( ot_time == 0 )													/*　出庫時刻が０時００分なら　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		ot_time += T_DAY;												/*　出庫時刻＋１４４０分　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( (FeeMaxType == 1||FeeMaxType == SP_MX_612HOUR)&&(h24time == 0) )/*　２４時間最大チェックかつＩＮ＋２４すぎた　*/
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
	loop_out = OFF;														/*　ループを抜けるべきか見分ける変数をＯＦＦ　*/
	tgn_strt = teigen_strt[style-1];									/*　逓減帯開始時刻　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	cyclt1 = cycl_frst[style-1];		       							/*　　　　　　　　　　　　　　　　　　　　　　*/
	cyclt2 = cycl_last[style-1];		       							/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	for ( ; ; )															/*											  */
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( strt_ptn >= 13 )											/*　１２パターン以上有る場合　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			strt_ptn = 12;												/*　もう１度パタン１２を繰り返す　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		while( ptn_ccnt < ptn_cycl[style-1][strt_ptn-1] )				/*　パターン単位係数回ループ　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(style_type[style-1] == 2)								/*　深夜時間を考慮するなら　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( cal_time >= midnig_strt[style-1] )					/*　深夜帯開始時刻以降，次体系開始時刻より前　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					midflg = ON;										/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( midnig_ovl[style-1] != 0 )						/*　重複時間を考慮するなら　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						et2201( &midflg );								/*　重複帯内出庫ＣＨＥＣＫ　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					midni_ok = midflg;									/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( midflg == ON )									/*　深夜帯料金をとる　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						ryoukin += fee_midnig[style-1][car_type-1];		/*　駐車料金に深夜料金を加算　　　　　　　　　*/
						if( teigen_strt[style-1]+T_DAY < next_style_strt )/*次料金帯が次開始時刻より大きい　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							cal_time = teigen_strt[style-1]+T_DAY;		/*　計算時刻を逓減帯開始時刻にセット　　　　　*/
							cal_time += teigen_ovl[style-1];			/*　重複時間を加算　　　　　　　　　　　　　　*/
							kasan_flg = 3;								/*　調整フラグセット　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						else											/*　次料金帯が次開始時刻以下　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							cal_time = next_style_strt;					/*　計算時刻を次逓減帯開始時刻にセット　　　　*/
							cal_time += teigen_ovl[next_style-1];		/*　重複時間を加算　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(( cardmy.year != cal_tm.year )||				/*　現在時刻は次の体系と違う　　　　　　　　　*/
							( cardmy.day  != cal_tm.day )||				/*　　　　　　　　　　　　　　　　　　　　　　*/
							( cardmy.mon  != cal_tm.mon ))				/*　　　　　　　　　　　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							ec63( &cal_tm );							/*　次の日にセット　　　　　　　　　　　　　　*/
							first = OFF;								/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						else 											/*　　　　　　　　　　　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							if( first_flg == ON )						/*　前日入車　　　　　　　　　　　　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								cal_time -= T_DAY;						/*　check用に１日分引く　　　　　　　　　　　 */
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
							if(( cal_time > T_DAY )&&					/*　時刻を加算した結果日をまたいでかつ前日入車*/
							   (( first_flg == ON )||( first== ON )))	/*　かつ日繰り越し有りなら　　　　　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								ec63( &cal_tm );						/*　次の日にセット　　　　　　　　　　　　　　*/
								first = OFF;							/*　　　　　　　　　　　　　　　　　　　　　　*/
								if( cal_time >= T_DAY*2 )				/*　２日以上だったら　　　　　　　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									cal_time -= T_DAY;					/*　　次の日に更新したので１日分引く　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
							if((cal_time >= T_DAY*2 )&&(first_flg==OFF))/*　２日以上だったら　　　　　　　　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								first = ON;								/*　　次の日の更新フラグ　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
							if( first_flg == ON )						/*　前日入車　　　　　　　　　　　　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								cal_time += T_DAY;						/*　check用に１日分戻す　　　　　　　　　　　 */
								first_flg = OFF;						/*　その日の入車でも開始以前フラグ　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						et_len = 0;										/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( (FeeMaxType != 0)&&(FeeMaxType != 3) )		/*　日毎最大料金ＣＨＥＣＫ有りなら　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							ec65( et_len,tgn_strt );					/*　日毎最大料金検査結果を求める　　　　　　　*/

							if( teig_mid == 1 )							/*　　　　　　　　　　　　　　　　　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								if(teig_cycl != 1)						/*　　　　　　　　　　　　　　　　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									ryoukin += 							/*　　　　　　　　　　　　　　　　　　　　　　*/
										fee_midnig[style-1][car_type-1];/*　駐車料金に深夜料金を加算　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						iti_flg = ON;									/*　一定料金帯フラグセット　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( amr_flg != OFF )							/*　一定料金帯余り時間加算処理有りの時　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							cal_time +=  amartim;						/*　　計算時刻に一定料金帯余り時間加算　　　　*/
							amr_flg = OFF;								/*　  余りフラグリセット　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(	sp_mx_1 == ON )								/*　IN+24最大時cal_time調整フラグＯＮなら 　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							cal_time += T_DAY;							/*　cal_timeを取りあえずet2101終了まで+1440分 */
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						first = et2202( first );						/*　計算分時刻セット　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(	sp_mx_1 == ON )								/*　IN+24最大時cal_time調整フラグＯＮなら 　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							cal_time -= T_DAY;							/*　cal_timeを取りあえずet2101終了まで+1440分 */
							sp_mx_1 = OFF;								/*　IN+24最大時cal_time調整フラグＯＦＦ　　　 */
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( ec64( &cal_tm,&car_ot) != 1 )				/*　時刻比較（計算時刻が仮出庫時刻以降の場合）*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							loop_out = ON;								/*　ループを抜けるべきか見分ける変数をＯＮ　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
							if( prm_get( 0,S_CAL,17,1,3 ) == 1 ){		/*　深夜帯後の料金の取り方が指定されている場合*/
								ptn_ccnt = 0;							/*　パターンの単位係数をクリアする。　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
							break;										/*　while ループを抜ける　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( cal_time >= next_style_strt )				/*　次体系開始時刻以降の場合　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							loop_out = ON;								/*　ループを抜けるべきか見分ける変数をＯＮ　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
							if( prm_get( 0,S_CAL,17,1,3 ) == 1 ){		/*　深夜帯後の料金の取り方が指定されている場合*/
								ptn_ccnt = 0;							/*　パターンの単位係数をクリアする。　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*                                            */
							break;										/*　for ループを抜ける　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( kasan_flg == 3)								/*　　　　　　　　　　　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							midnig_strt[style-1] += T_DAY;				/*　深夜帯開始時刻に１４４０加算　　　　　　　*/
							kasan_flg = ON;								/*　調整フラグセット　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		/*  定期後加算処理　 */											/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( amr_flg == T_TX_TIM )									/*　あまり加算処理有り　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				amr_flg = OFF;											/*　あまりフラグOFF 　　　　　　　　　　　　　*/
				cal_time += amartim;									/*　あまり時間加算　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				first = et2202( first );								/*　計算分時刻セット　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( ec64( &cal_tm,&car_ot ) != 1 )						/*　時刻比較（計算時刻が仮出庫時刻以降の場合）*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					loop_out = ON;										/*　ループを抜けるべきか見分ける変数をＯＮ　　*/
					break;												/*　while ループを抜ける　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		/*  定期後加算処理　 */											/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(	(midni_ok == 1)&&(prm_get( 0,S_CAL,17,1,3 ) == 1L) )	/*　逓減帯後のパターンを変更する。　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				chk_ptn  = (short)prm_get( 0,S_CAL,17,2,1 );			/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( chk_ptn == 0 )			chk_ptn = 1;				/*　　　　　　　　　　　　　　　　　　　　　　*/
				if((( chk_ptn == strt_ptn )&&( sumi == 1 ))||( sumi == 0 ))		/*　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					strt_ptn = chk_ptn;									/*　　　　　　　　　　　　　　　　　　　　　　*/
					sumi = 1;											/*　　　　　　　　　　　　　　　　　　　　　　*/
					h24_mid = 1;										/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			et_fee  = (unsigned long)fee[style-1][car_type-1][strt_ptn-1] ;	/*　駐車料金にパターン単位料金を加算　　　*/
			et_len = ptn_leng[style-1][strt_ptn-1];						/*　計算時刻にパターン単位時間を加算　　　　　*/
			ryoukin  = ryoukin + (unsigned long)(et_fee * 10);			/*　駐車料金にパターン単位料金を加算　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( (FeeMaxType != 0)&&(FeeMaxType != 3) )					/*　日毎最大料金ＣＨＥＣＫ有りなら　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/

				ec65( et_len,tgn_strt );								/*　日毎最大料金検査結果を求める　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				cal_time += et_len;										/*　計算時刻にパターン単位時間を加算　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			iti_flg = OFF;												/*　一定料金帯フラグセット　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(( cal_time >= midnig_strt[style-1] )&&					/*　深夜帯開始時刻以降　かつ 　　　　　　　　 */
				( style_type[style-1] == 2 ))							/*　深夜帯を考慮する　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				midflg = ON;											/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( midnig_ovl[style-1] != 0 )							/*　重複時間を考慮するなら　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					et2201(&midflg);									/*　重複帯内出庫ＣＨＥＣＫ　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				midni_ok = midflg;										/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( midflg == ON )										/*　深夜帯料金をとる　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( sp_it == 1 )									/*　時間余りを加算する　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						amartim = cal_time - midnig_strt[style-1];		/*　時間余りを算出　　　　　　　　　  　　　　*/
						amr_flg = ON;									/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					cal_time = midnig_strt[style-1] + midnig_ovl[style-1];/*　深夜帯開始時刻にセット　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( cal_time >= next_style_strt )							/*　次体系開始時刻以降の場合　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( style != next_style )								/*　次体系は今回と異なる　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( sp_op == 1 )									/*　余り処理が切捨ての場合　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						cal_time = next_style_strt;						/*　次体系開始時刻を計算時刻とする　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( sp_mx_1 == ON )											/*　IN+24最大時cal_time調整フラグＯＮなら 　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				cal_time += T_DAY;										/*　cal_timeを取りあえずet2101終了まで+1440分 */
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			first = et2202( first );									/*　計算分時刻セット　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( sp_mx_1 == ON )											/*　IN+24最大時cal_time調整フラグＯＮなら 　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				cal_time -= T_DAY;										/*　cal_timeを取りあえずet2101終了まで+1440分 */
				sp_mx_1 = OFF;											/*　IN+24最大時cal_time調整フラグＯＦＦ　　　 */
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ec64( &cal_tm,&car_ot ) != 1 )							/*　時刻比較（計算時刻が仮出庫時刻以降の場合）*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				loop_out = ON;											/*　ループを抜けるべきか見分ける変数をＯＮ　　*/
				break;													/*　while ループを抜ける　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			ptn_ccnt++;													/*　現在単位係数カウンタＵＰ　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( cal_time >= next_style_strt )							/*　次体系開始時刻以降の場合　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				loop_out = ON;											/*　while ループを抜ける　　　　　　　　　　　*/
				break;													/*　for ループを抜ける　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　while　ＥＮＤ 　　　　　　　　　　　　　　*/
		if(loop_out == ON )												/*ループを抜けるべきか見分ける変数がＯＮの場合*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ptn_ccnt == ptn_cycl[style-1][strt_ptn-1] )				/*　パターン単位係数は終了？　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(( cyclt1	!= 0 )||( cyclt2 != 0 ))					/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if(( strt_ptn >= (int)(cycl_last[style-1]) )&&  	/*　繰り返しパターンの場合で　　　　　　　　　*/
						( cycl_ccnt < cycl_cnt[style-1] ))				/*　　繰り返し回数が設定以下　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						strt_ptn = cycl_frst[style-1] - 1;				/*　パターンを繰り返し開始パターンに戻す　　　*/
						cycl_ccnt++;									/*　パターン繰り返し回数カウント用変数に１加算*/
						if( cycl_cnt[style-1] == 1440 )					/*　繰り返し回数が永遠　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							cycl_ccnt = 0;								/*　パターン繰り返し回数カウントクリア　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				strt_ptn++;												/*　パターンカウントＵＰ　　　　　　　　　　　*/
				ptn_ccnt = 0;											/*　現在単位係数カウンタクリア　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			break;														/*　while ループを抜ける　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			ptn_ccnt = 0;												/*　現在単位係数カウンタクリア　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
        if(( cyclt1	!= 0 )||( cyclt2 != 0 ))							/*　　　　　　　　　　　　　　　　　　　　　　*/
       	{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(( strt_ptn >= (short)(cycl_last[style-1]) )&&  			/*　繰り返しパターンの場合で　　　　　　　　　*/
				( cycl_ccnt < cycl_cnt[style-1] ))						/*　　繰り返し回数が設定以下　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				strt_ptn = cycl_frst[style-1] - 1;						/*　パターンを繰り返し開始パターンに戻す　　　*/
				cycl_ccnt++;											/*　パターン繰り返し回数カウント用変数に１加算*/
				if( cycl_cnt[style-1] == 1440 )							/*　繰り返し回数が永遠　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					cycl_ccnt = 0;										/*　パターン繰り返し回数カウントクリア　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
       		}															/*　　　　　　　　　　　　　　　　　　　　　　*/
       	}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		strt_ptn++;														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( strt_ptn == 13 )											/*　最終パターンなら　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ptn_cycl[style-1][strt_ptn-1]!= 0 )						/*　パターン単位係数回ループ０でない　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				strt_ptn = 1;											/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			cycl_ccnt = 0;												/*　パターン繰り返し回数カウントクリア　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　while　ＥＮＤ 　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( cal_time >= next_style_strt )									/*　次体系開始時刻以降の場合　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( style != next_style )										/*　次体系は今回と異なる　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( sp_op == 1 ) 											/*　余り処理が切捨ての場合　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				cal_time = next_style_strt;								/*　次体系開始時刻を計算時刻とする　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	first = et2202( first );											/*　計算分時刻セット　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( FeeMaxType == 1 || FeeMaxType == SP_MX_612HOUR )				/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		chk_tim.hour = cal_tm.hour;										/*　現在計算時セーブ　　　　　　　　　　　　　*/
		chk_tim.min = cal_tm.min ;										/*　現在計算分セーブ　　　　　　　　　　　　　*/
		chk_time = cal_time;											/*　現在時分セーブ　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( kasan_flg == ON )												/*　加算調整フラグＯＮなら　　　　　　　　　　*/
	{														 			/*　　　　　　　　　　　　　　　　　　　　　　*/
		midnig_strt[style-1] -= T_DAY;						 			/*　元に戻す　　　　　　　　　　　　　　　　　*/
		kasan_flg = OFF;									 			/*　　　　　　　　　　　　　　　　　　　　　　*/
	}														 			/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( ptn_cycl[style-1][strt_ptn-1] == 1440 )							/*　パターン単位計数が最後まで繰り返すならば　*/
	{																	/*　パターンカウントを０にする　　　　　　　　*/
		ptn_ccnt = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	return;																/*　逓減帯方式料金計算処理を終了する　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：逓減帯料金計算　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et2201()　　　　　　　　　：逓減帯料金計算処理：現在料金帯の重複帯有無CHECK　 　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　設計者（日付）　　：	　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　重複料金帯フラグ　　　　　　重複料金帯フラグ　　　　　　　　：　*/
/*　変更者（日付）　　：多造			　　　　　：　重複帯ＥＮＤ時刻　　　　　　重複帯ＥＮＤ時刻　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/**********************************************/
void	et2201( 														/*　　　　　　　　　　　　　　　　　　　　　　*/
short	*pa_mid															/*　　　　　　　　　　　　　　　　　　　　　　*/
)																		/*　　　　　　　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	char	flg;														/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	et_midtm;													/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	flg = OFF;															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	et_midtm = midnig_strt[style-1] + midnig_ovl[style-1];				/*　深夜帯重複時間　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if(( cardmy.year == car_ot.year )&&									/*　出庫時刻は次体系と同じ日にち　　　　　　　*/
		( cardmy.day == car_ot.day )&&									/*　　　　　　　　　　　　　　　　　　　　　　*/
		( cardmy.mon == car_ot.mon ))									/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		flg = ON;														/*　チェックフラグＯＮ　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( ( cal_tm.year == car_ot.year )&&								/*　出庫時刻は今日と同じ　　　　　　　　　　　*/
		( cal_tm.day == car_ot.day )&&									/*　　　　　　　　　　　　　　　　　　　　　　*/
		( cal_tm.mon == car_ot.mon ))									/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		flg = ON;														/*　チェックフラグＯＮ　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( flg == ON )														/*　チェックフラグＯＮなら　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( ot_time <= et_midtm )										/*　深夜重複帯内に出庫有りなら　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			*pa_mid = OFF;												/*　深夜料金をとらない　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：逓減帯料金計算　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et2202()　　　　　　　　　：逓減帯料金計算処理：計算分時刻セット　　　　　　  　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　設計者（日付）　　：	　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　日付け変更フラグ　　　　　　日付け変更フラグ　　　　　　　　：　*/
/*　変更者（日付）　　：多造			　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
short 	et2202( 														/*　　　　　　　　　　　　　　　　　　　　　　*/
short	pa_fst															/*　　　　　　　　　　　　　　　　　　　　　　*/
)																		/*　　　　　　　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	cal_wrk;													/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	cal_wrk = cal_time;													/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( cal_time >= T_DAY )												/*　計算時刻が２４時以降の場合　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		cal_tm.hour = (char)((cal_time-T_DAY)/60 );						/*　計算時刻（時）を構造帯に格納　　　　　　　*/
		cal_tm.min  = (char)((cal_time-T_DAY)%60 );						/*　計算時刻（分）を構造帯に格納　　　　　　　*/
		if( pa_fst == ON )												/*　最初の日付変更を表すフラグがＯＮの場合　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			pa_fst = OFF;												/*　最初の日付変更を表すフラグをＯＦＦにする　*/
			ec63( &cal_tm );											/*　日を１日進める　　　　　　　　　　　　　　*/
			cal_wrk -= T_DAY;											/*　　　　　　　　　　　　　　　　　　　　　　*/
			for( ; cal_wrk >= T_DAY; cal_wrk -= T_DAY)					/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				style = ec61( &cal_tm );								/*　今日の体系算出処理　　　　　　　　　　　　*/
				memcpy( (char *)&cardmy,(char *)&cal_tm,7 );			/*　現在計算時刻を保存する　　　　　　　　　　*/
				ec63( &cardmy );										/*　日を１日進める（次の体系開始時刻参照用）　*/
				next_style = ec61( & cardmy ); 							/*　次の日の体系算出処理　　　　　　　　　　　*/
				next_style_strt = teigen_strt[next_style-1] + T_DAY;	/*　次の料金体系開始時刻算出　　　　　　　　　*/
				if( style != next_style )								/*　次体系は今回と異なる　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( sp_op == 1 )									/*　余り処理が切捨ての場合　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						cal_time = next_style_strt;						/*　次体系開始時刻を計算時刻とする　　　　　　*/
						cal_tm.hour = (char)((cal_time-T_DAY)/60 );		/*　計算時刻（時）を構造帯に格納　　　　　　　*/
						cal_tm.min = (char)((cal_time-T_DAY)%60 );		/*　計算時刻（分）を構造帯に格納　　　　　　　*/
						pa_fst = ON;									/*　　　　　　　　　　　　　　　　　　　　　　*/
						break;											/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				ec63( &cal_tm );										/*　日を１日進める　　　　　　　　　　　　　　*/
				cal_tm.hour = (char)((cal_wrk -T_DAY)/60 );				/*　計算時刻（時）を構造帯に格納　　　　　　　*/
				cal_tm.min = (char)((cal_wrk -T_DAY)%60 );				/*　計算時刻（分）を構造帯に格納　　　　　　　*/
				cal_time = cal_wrk;										/*　計算時刻を合わせる　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　計算時刻が２４時に達しない場合　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		cal_tm.hour = (char)(cal_time/60);								/*　計算時刻（時）を構造帯に格納　　　　　　　*/
		cal_tm.min = (char)(cal_time%60);								/*　計算時刻（分）を構造帯に格納　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( cal_tm.hour >= 24 )												/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		cal_tm.hour -= 24;												/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	return( pa_fst );													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：逓減帯料金計算　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et2203()　　　　　　　　　：逓減帯料金計算処理：計算分時刻セット　　　　　　  　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　設計者（日付）　　：	　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　日付け変更フラグ　　　　　　日付け変更フラグ　　　　　　　　：　*/
/*　作成者（日付）　　：秋葉　(96/06/26)	　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/**********************************************************************************************************************/
																		/**********************************************/
char 	et2203( 														/*　　　　　　　　　　　　　　　　　　　　　　*/
char	pt_fg															/*　　　　　　　　　　　　　　　　　　　　　　*/
)																		/*　　　　　　　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	char	ret = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
	short 	cyclt1;														/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	cyclt2;														/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	cyclt1 = cycl_frst[style-1];		       							/*　　　　　　　　　　　　　　　　　　　　　　*/
	cyclt2 = cycl_last[style-1];		       							/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if(prm_get( 0,S_CAL,17,1,3 ) == 0)									/* 深夜帯後の料金の取り方指定無し			  */
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		ptn_ccnt ++;													/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(ptn_ccnt >= 													/*　　　　　　　　　　　　　　　　　　　　　　*/
			ptn_cycl[style-1][strt_ptn-1])								/*　パターン単位係数　　　　　　　　　		  */
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			ptn_ccnt = 0;												/*　　　　　　　　　　　　　　　　　　　　　　*/
			strt_ptn ++;												/*　　　　　　　　　　　　　　　　　　　　　　*/
	        if((cyclt1 != 0)||(cyclt2 != 0))							/*　ﾊﾟﾀｰﾝ開始または終了有り　　　　　　　　　 */
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if((strt_ptn < cyclt1)||								/*　　　　　　　　　　　　　　　　　　　　　　*/
					(strt_ptn > cyclt2))								/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					strt_ptn = cyclt1;									/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				strt_ptn = 0;											/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(pt_fg == 1)													/*　　　　　　　　　　　　　　　　　　　　　　*/
			ret = 1;													/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	return(ret);														/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
