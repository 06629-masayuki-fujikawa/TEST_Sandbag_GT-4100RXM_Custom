/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：日毎最大料金処理　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：ec65(par1)　　　　　　　　：日毎最大料金処理　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　計算対象分　　　　　　　　　リタンコード　　　　　　　　　　：　*/
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
char	h24_mid = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
char	teig_mid = 0;													/* 逓減帯24H最大ﾌﾗｸﾞ,昼夜帯には影響無し		  */
char	teig_cycl= 0;													/* 逓減帯24H最大ﾌﾗｸﾞ,昼夜帯には影響無し		  */
char	tyu24_flg = 0;													/* 昼夜帯､一定料金帯での24H最大ﾌﾗｸﾞ　　　 　　*/
char	max24_fg = 0;													/* 一定料金帯料金の取り方設定が１の時に使う	　*/
char	h24_flg = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		// 
ulong	chk_max1_bf;													// 体系基準方式用変数
uchar	chk_max1_flg;													// 体系基準方式用変数
																		// 
int			day_1st_check		( void );								/*											  */
short	hour612chk(short);												/*											  */
short	bnd_strt4_wk;													/*　料金帯開始時刻　　　　　　　　　　　　　　*/
short	bnd_strt_ovl_wk;												/*　料金帯開始時刻＋重複時間　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	ec65( 															/*　日毎最大料金処理　　　　　　　　　　　　　*/
short 	pa_len	,														/*　計算対象分　　　　　　　　　　　　　　　　*/
short 	pa_strt	)														/*　料金帯開始時刻　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	wk_cal_time;												/*　　　　　　　　　　　　　　　　　　　　　　*/
	char	teigen24 = 0;												/*											  */
	short	itei_end = 0;												/* 一定料金帯の重複帯終了時刻				　*/
	char	mid24_00 = 0;												/*											  */
	short	wk_tgend = 0;												/* 逓減帯重複帯終了時刻						　*/
	short	in_add24 = 0;												/* 入庫時刻+24h								　*/
	short	tmode;														/*　　　　　　　　　　　　　　　　　　　　　　*/
	int  n_max_set;														/*　ｎ時間最大ｍ回の適用回数設定　　　　　　　*/
	int  n_hour_wk;														/*　ｎ時間計算のワーク　　　　　　　　　　　　*/
	struct	CAR_TIM	wk_time;											// Ｎ時間最大経過時刻計算用ワーク構造体
	short	wk_calcnt;													// Ｎ時間経過回数ワーク変数
	ulong	in_cal_tm;													// 現在までの経過時間格納用変数
	char	itei_reset;													// 一定料金帯後のＮ時間リセット
	long	ryo_bak;													// 
	long	ryo_over;													// 
	short	ac_wtm,bc_wtm,bs_wtm,nm_wtm;								// 1単位時間にN時間と体系切換が含まれる時の計算変数
	short	bf_cal_time;												// 単位時間加算前のcal_time保存用ﾜｰｸ変数
	short	wk_style_strt;												// 
	char	FeeMaxType;													// 最大料金タイプ
	short	nmax_tim1;													// 
	short	nmax_tim2;													// 
	short	nm_set1;													// 
																		// 
	short	wmax_cal_tm;												// ｎ時間最大2(内)用の経過時間ワーク
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	wk_cal_time = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	h24_flg	= 0;														/* 昼夜帯24H加算ﾌﾗｸﾞ　　　　　　　　　　　　　*/
	teig_mid = 0;														/* 深夜帯料金加算ﾌﾗｸﾞ　　　　　　　　　　　　 */
	teig_cycl = 0;														/* 深夜帯ｼﾞｬﾝﾌﾟ後のﾊﾟﾀｰﾝ更新ﾌﾗｸﾞ　　　　　　　*/
	koetim = 0;															/*　Ｎ時間超過分の時間			　　　　　　　*/
																		/*											  */
	bf_cal_time = cal_time;												// 単位時間加算前の計算時刻を保存する
																		// 
	FeeMaxType = sp_mx;													// 最大料金タイプのセット
	if( EXP_FeeCal == 1 ){												// 料金計算拡張あり
		FeeMaxType = sp_mx_exp[car_type-1];								// 料金種別毎に最大料金タイプを取得する
	}																	// 
	switch( FeeMaxType )												/*　最大料金取得方法により分類　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		case 1:															/*　２４時間最大の場合　　　　　　　　　　　　*/
			if( ryoukin >= chk_max1 )									/*　駐車料金が２４時間最大料金を超えた場合　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(	(CPrmSS[S_CAL][14] == 0L)||							/*　２４時間最大または　　　　　　　　　　　　*/
					((CPrmSS[S_CAL][14])&&(day_1st_check()))){			/*　１回のみかつ・・・　　　　　　　　　　　　*/
					ryoukin = chk_max1;									/*　２４時間最大料金を駐車料金とする　　　　　*/
					rmax_cnt++;											/*　最大到達カウントＵＰ　　　　　　　　　　　*/
					if(( rmax_cnt == 1 )&&( cal_type == 1 ))			/*　最大になった時のパタンセーブ（逓減）　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						ec65_1(0);										/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( ptn_cycl[style-1][strt_ptn-1] == T_DAY )	/*　　　　　　　　　　　　　　　　　　　　　　*/
							rmax_pcn = 0;								/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( cycl_cnt[style-1] == T_DAY )				/*　　　　　　　　　　　　　　　　　　　　　　*/
							rmax_cyc = 0;								/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　パタン繰り返し回数　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			cal_time  = cal_time + pa_len;								/*　計算時刻を追加回数×追加時間進める　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			wk_cal_time = cal_time;										/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(( iti_flg == ON )&&( sp_it == 1 ))						/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				wk_cal_time = cal_time + amartim;						/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if((CPrmSS[S_CAL][1] != 1L)&&(rmax_cnt == 1)&&				/*　昼夜帯なら　　　　　　　　　　　　　　　　*/
				(band == 1)&&(max_bnd[style-1] > 1)&&					/*　現在第１料金帯で２つ以上の料金帯設定　　　*/
				(leng_exp[style-1][0] == 0)&&							/*　第１料金帯が一定料金設定　　　　　　　　　*/
				(leng_std[style-1][0] == 0)&&							/*　　　　　　　　　　　　　　　　　　　　　　*/
				(cal_time == (bnd_strt[style-1][band]					/*　計算時刻が次の料金帯開始である。　　　　　*/
						+ovl_time[style-1][band])))						/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*											  */
				if(cal_time >= T_DAY)									/*	計算時刻が24Hを越えた					  */
				{														/*											  */
					if((in_time >= bnd_strt[style-1][band-1]&&			/*	入庫時刻が一定帯開始時刻を越えかつ		  */
						in_time < T_DAY))								/*	0:00以内								  */
					{													/*											  */
						if(in_time < ot_time)							/*	出庫時刻が入庫時刻を越えた				  */
						{												/*											  */
							h24_flg = 1;								/*	料金加算ﾌﾗｸﾞON							  */
							rmax_cnt ++;								/*											  */
						}												/*											  */
					}													/*											  */
					else if((in_time >= 0)&&							/*	入庫時刻が0:00から次の料金帯以内		  */
						(in_time <= (cal_time - T_DAY)))				/*											  */
					{													/*											  */
						if(in_time < (ot_time - T_DAY))					/*	出庫時刻が入庫時刻を越えた				  */
						{												/*											  */
							h24_flg = 1;								/*	料金加算ﾌﾗｸﾞON							  */
							rmax_cnt ++;								/*											  */
						}												/*											  */
					}													/*											  */
				}														/*											  */
				else													/*	計算時刻が24Hを越えていない				  */
				{														/*											  */
					if((in_time >= bnd_strt[style-1][band-1])&&			/*	入庫時刻が一定帯内　　　				  */
						(in_time <= cal_time))							/*											  */
					{													/*											  */
						if(in_time < ot_time)							/*	出庫時刻が入庫時刻を越えた				  */
						{												/*											  */
							h24_flg = 1;								/*	料金加算ﾌﾗｸﾞON							  */
							rmax_cnt ++;								/*											  */
						}												/*											  */
					}													/*											  */
				}														/*											  */
			}															/*											  */
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( (h24time + (wk_cal_time - wk_h24t)) >= T_DAY ) 			/*　計算時刻が入庫後２４時間経過　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				h24time = 0;											/*　現在までの計算時間トータルクリア　　　　　*/
				wk_h24t = cal_time;										/*　現在までの計算時間　　　　　　　　　　　　*/
				max24_fg = 1;											/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(CPrmSS[S_CAL][1] == 1)								/*　料金計算方式が逓減帯　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if(style_type[style-1] == 2)						/*　深夜時間を考慮するなら　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(midnig_ovl[style-1] != 0)					/*　昼夜帯との重複帯あり　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							itei_end = midnig_strt[style-1]				/*　　　　　　　　　　　　　　　　　　　　　　*/
										+midnig_ovl[style-1];			/*　重複帯の終了時刻を求める　　　　　　　　　*/
							if(itei_end < T_DAY)						/*　重複帯終了時刻が０：００を越えていない　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								if((in_time >= midnig_strt[style-1])&&	/*　入庫時刻が重複帯内　　　　　　　　　　　　*/
									(in_time <= itei_end))				/*　　　　　　　　　　　　　　　　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									wk_h24t = in_time;					/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
							else										/*　重複帯の終了時刻が０：００を越えた　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								if(in_time >= midnig_strt[style-1])		/*　０：００分より前（重複帯内）　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									wk_h24t = in_time;					/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
								else if(in_time <= (itei_end-T_DAY))	/*　０：００分より後（重複帯内）　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									wk_h24t = in_time + T_DAY;			/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　料金計算方式が昼夜帯　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if(tyu24_flg == 1)									/*　一定料金帯の場合　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(ovl_time[style-1][band-1] != 0)				/*　一定料金帯に重複時間あり　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							itei_end = bnd_strt[style-1][band-1]		/*　一定料金帯の重複時間を求める　　　　　　　*/
									+ovl_time[style-1][band-1];			/*　　　　　　　　　　　　　　　　　　　　　　*/
							if(itei_end < T_DAY)						/*　重複帯の終了時刻が０：００を越えていない　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								if((in_time >= bnd_strt[style-1][band-1])&&	/*　入庫時刻が重複帯内　　　　　　　　　　*/
									(in_time <= itei_end))				/*　　　　　　　　　　　　　　　　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									wk_h24t = in_time;					/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
							else										/*　重複帯の終了時刻が０：００を越えた　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								if(in_time >= bnd_strt[style-1][band-1])/*　入庫時刻が重複帯内　　　　　　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									wk_h24t = in_time;					/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
								else if(in_time <= (itei_end-T_DAY))	/*　入庫時刻が重複帯内　　　　　　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									wk_h24t = in_time + T_DAY;			/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				total = ryoukin + total;								/*　料金を集計中の駐車料金に加算　　　　　　　*/
				ryoukin = 0;											/*　料金を０クリア　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( rmax_cnt > 1 )										/*　今の追加時間で最大となった訳でない！　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( cal_time >= T_DAY )								/*　計算時刻は１日を越えた　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(CPrmSS[S_CAL][1] != 1)						/*　昼夜帯なら　　　　　　　　　　　　　　　　*/
						{												/*											  */
							if(ot_time >= T_DAY)						/*　出車時刻は１日を越えた　　　　　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								if((in_time + T_DAY) < cal_time)		/*　入車時刻が次の料金帯開始時刻前である　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									if(ot_time > (in_time + T_DAY))		/*　出車時刻が入車時刻を越えた　　　　　　　　*/
										h24_flg = 1;					/*　24h最大ﾌﾗｸﾞON 　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
								else									/*　入車時刻が次の料金帯開始後である　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									if((cal_time != (in_time + T_DAY))&&/*											  */
										( ot_time > (in_time + T_DAY)))	/*　出車時刻が入車時刻を越えた　　　　　　　　*/
									{									/*　　　　　　　　　　　　　　　　　　　　　　*/
										h24_flg = 1;					/*　24h最大ﾌﾗｸﾞON 　　　　　　　　　　　　　　*/
									}									/*　　　　　　　　　　　　　　　　　　　　　　*/
									if(in_time > bnd_strt[style-1][band-1])	/*　　　　　　　　　　　　　　　　　　　　*/
									{									/*　　　　　　　　　　　　　　　　　　　　　　*/
										h24_flg = 1;					/*　24h最大ﾌﾗｸﾞON　　　　　　　　　　　　　　 */
									}									/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
							else										/*　出車時刻が１日を越えていない　　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								if(in_time < ot_time)					/*　出車時刻が入車時刻を越えた　　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									if((org_in.year == org_ot.year)&&	/*　出庫時刻は次体系と同じ日にち　　　　　　　*/
										(org_in.day == org_ot.day)&&	/*　　　　　　　　　　　　　　　　　　　　　　*/
										(org_in.mon == org_ot.mon))		/*　　　　　　　　　　　　　　　　　　　　　　*/
									{									/*　　　　　　　　　　　　　　　　　　　　　　*/
									}									/*　　　　　　　　　　　　　　　　　　　　　　*/
									else								/*　　　　　　　　　　　　　　　　　　　　　　*/
									{									/*　　　　　　　　　　　　　　　　　　　　　　*/
										if((in_time + T_DAY) > cal_time)/*　入車時刻が計算時刻を越えた　　　　　　　　*/
											h24_flg = 1;				/*　24h最大ﾌﾗｸﾞON 　　　　　　　　　　　　　　*/
									}									/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						in_add24 = in_time + T_DAY;						/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( CPrmSS[S_CAL][1] == 1 ){					/*　逓減帯なら　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
							teigen24 = 0;								/*　　　　　　　　　　　　　　　　　　　　　　*/
							if(cal_time >= ot_time)						/*　　計算時刻が出庫時刻を越えた？　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								teigen24 = 1;							/*　深夜帯料金加算ﾁｪｯｸﾌﾗｸﾞON　　　　　　　　　*/
								if(((cal_time - pa_len) < T_DAY)&&		/*　今回の計算時刻更新処理で1日を越えた		　*/
									(wk_cal_time >= T_DAY))				/*　　　　　　　　　　　　　　　　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									if(style_type[style-1] == 2)		/*　深夜帯考慮するなら　　				 　 　*/
									{									/*　　　　　　　　　　　　　　　　　　　　　　*/
										mid24_00 = 1;					/*　深夜帯到達前の逓減料金加算　　　　　　　　*/
										if(in_time >= (cal_time-pa_len))/*　前回の計算時刻≦入庫時刻＜０：００　　　　*/
										{								/*　　　　　　　　　　　　　　　　　　　　　　*/
											if(in_time >= ot_time)		/*　入庫時刻が出庫時刻に満たない　　　　　　　*/
												mid24_00 = 0;			/*　逓減料金加算なし　　　　　　　　　　　　　*/
										}								/*　　　　　　　　　　　　　　　　　　　　　　*/
										else							/*　入庫時刻≧０：００　　　　　　　　　　　　*/
										{								/*　　　　　　　　　　　　　　　　　　　　　　*/
											if((in_time+T_DAY) >= ot_time)/*　入庫時刻が出庫時刻に満たない　　　　　　*/
												mid24_00 = 0;			/*　逓減料金加算なし　　　　　　　　　　　　　*/
										}								/*　　　　　　　　　　　　　　　　　　　　　　*/
										if(mid24_00 == 1)				/*　　　　　　　　　　　　　　　　　　　　　　*/
											cal_time = in_time;			/*　　　　　　　　　　　　　　　　　　　　　　*/
									}									/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
								else									/*　　　　　　　　　　　　　　　　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									if(style_type[style-1] == 2)		/*　深夜帯考慮するなら　　 　	　			  */
									{									/*　　　　　　　　　　　　　　　　　　　　　　*/
										if((in_time+T_DAY) < ot_time)				/*　出車時刻が入車時刻を越えた　　*/
										{											/*　　　　　　　　　　　　　　　　*/
											itei_end = midnig_strt[style-1]			/*　　　　　　　　　　　　　　　　*/
												+ midnig_ovl[style-1];				/*　重複帯の終了時刻を求める　　　*/
																					/*　　　　　　　　　　　　　　　　*/
											if((midnig_strt[style-1] <= T_DAY)&&	/*	重複帯が0:00をまたいでいる	  */
												(itei_end >= T_DAY))				/*								  */
											{										/*　　　　　　　　　　　　　　　　*/
												if(itei_end >= T_DAY)				/*　重複帯が24Hを越えている　　　 */
													itei_end = itei_end - T_DAY;	/*　　　　　　　　　　　　　　　　*/
																					/*　　　　　　　　　　　　　　　　*/
												if((midnig_strt[style-1] == T_DAY)&&/*　深夜帯開始時刻が0:00　　　　　*/
													(midnig_ovl[style-1] != 0))		/*　深夜帯と逓減帯の重複有り　　　*/
													itei_end = midnig_ovl[style-1];	/*　　　　　　　　　　　　　　　　*/
																					/*　　　　　　　　　　　　　　　　*/
												if((in_time <= itei_end)&&			/*　重複帯内に入出庫時間あり　　　*/
													((ot_time-T_DAY) <= itei_end)&&	/*　　　　　　　　　　　　　　　　*/
													((cal_time-T_DAY) != teigen_strt[style-1]))/*					  */
												{									/*　　　　　　　　　　　　　　　　*/
													cal_time = in_add24;			/*　計算時刻をIN＋24Hとする		  */
												}									/*　　　　　　　　　　　　　　　　*/
											}										/*　　　　　　　　　　　　　　　　*/
											else									/*　　　　　　　　　　　　　　　　*/
											{										/*　　　　　　　　　　　　　　　　*/
												if(midnig_ovl[style-1] != 0)		/*　深夜帯と逓減帯の重複有り　　　*/
												{									/*　　　　　　　　　　　　　　　　*/
													if(midnig_strt[style-1] >= T_DAY)/*								  */
													{								/*								  */
														if((itei_end - T_DAY) >= in_time)/*							  */
															cal_time = in_add24;	/*							  	  */
													}								/*								  */
												}									/*　　　　　　　　　　　　　　　　*/
												else								/*　　　　　　　　　　　　　　　　*/
												{									/*　　　　　　　　　　　　　　　　*/
													if(ot_time <= midnig_strt[style-1])/*							  */
														cal_time = in_add24;		/*　計算時刻をIN+24Hとする　　　　*/
												}									/*　　　　　　　　　　　　　　　　*/
											}										/*　　　　　　　　　　　　　　　　*/
										}											/*　　　　　　　　　　　　　　　　*/
									}									/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
							else										/*　　　　　　　　　　　　　　　　　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								if(cal_time >= next_style_strt)			/*　計算時刻が次の体系の開始時間を越えた？　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									teigen24 = 1;						/*　深夜帯料金加算ﾁｪｯｸﾌﾗｸﾞON　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
								else									/*　　　　　　　　　　　　　　　　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									if(style_type[style-1] == 2)		/*　深夜時間を考慮するなら　　　　　　　　　　*/
									{									/*　　　　　　　　　　　　　　　　　　　　　　*/
										if(((cal_time - pa_len) < T_DAY)&&/*　今回の計算時刻更新処理で1日を越えた	　*/
											(wk_cal_time >= T_DAY))		/*　　　　　　　　　　　　　　　　　　　　　　*/
										{								/*　　　　　　　　　　　　　　　　　　　　　　*/
											cal_time = in_add24;		/*　計算時刻をIN＋24Hとする					　*/
											if(in_time > (wk_cal_time-T_DAY))/*										  */
												cal_time = in_time;		/*　計算時刻を入庫時刻とする　　　　　　　　　*/
										}								/*　　　　　　　　　　　　　　　　　　　　　　*/
										else							/*　　　　　　　　　　　　　　　　　　　　　　*/
										{								/*　　　　　　　　　　　　　　　　　　　　　　*/
											cal_time = in_add24;		/*　計算時刻をIN＋24Hとする					　*/
										}								/*　　　　　　　　　　　　　　　　　　　　　　*/
									}									/*　　　　　　　　　　　　　　　　　　　　　　*/
									else								/*　　　　　　　　　　　　　　　　　　　　　　*/
									{									/*　　　　　　　　　　　　　　　　　　　　　　*/
										cal_time = in_add24;			/*　計算時刻をIN＋24Hとする					　*/
									}									/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
							if(style_type[style-1] != 2)				/*　深夜時間を考慮しないなら　　　　　　　　　*/
								teigen24 = 0;							/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
							if((teigen24 == 1)&&(cal_time >= T_DAY))	/*　深夜料金加算ﾁｪｯｸﾌﾗｸﾞONで　　　　　　　　　*/
							{											/*　　計算時刻が1日を越えた　　　　　　　　　 */
								wk_tgend = teigen_strt[style-1]			/*　　　　　　　　　　　　　　　　　　　　　　*/
											+ teigen_ovl[style-1];		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
								if(ot_time >= T_DAY)					/*　出庫時刻が1日を越えた 　　　　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
/* 1h:300,7-22, MID:1000,18-9, MAX:3000 IN 22:00 OUT 0:01 */			/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
									if(in_time >= wk_tgend)				/*　入庫時刻が重複帯終了時刻を越えた　　　　　*/
									{									/*　　　　　　　　　　　　　　　　　　　　　　*/
										if(midnig_ovl[style-1] != 0)	/*　深夜帯重複時間あり　　　　　　　　　　　　*/
										{								/*　　　　　　　　　　　　　　　　　　　　　　*/
											if(ot_time > in_time)		/*　出庫時刻が入庫時刻を越えた　　　　　　　　*/
												teig_mid = 1;			/*　深夜料金加算ﾌﾗｸﾞON　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
											if(in_time == wk_tgend)		/*　入庫時刻が重複帯終了時刻を越えた　　　　　*/
												teig_cycl = 1;			/*	料金ﾊﾟﾀｰﾝ更新処理あり　　　　　　　　　　 */
										}								/*　　　　　　　　　　　　　　　　　　　　　　*/
										else							/*　深夜帯重複時間なし　　　　　　　　　　　　*/
										{								/*　　　　　　　　　　　　　　　　　　　　　　*/
											if((ot_time - T_DAY) >		/*　出庫時刻-24Hが深夜帯開始時刻を越えている　*/
												 midnig_strt[style-1])	/*　　　　　　　　　　　　　　　　　　　　　　*/
											{							/*　　　　　　　　　　　　　　　　　　　　　　*/
												if(ot_time > in_time)	/*　出庫時刻が入庫時刻を越えた　　　　　　　　*/
													teig_mid = 1;		/*　深夜料金加算ﾌﾗｸﾞON　　　　　　　　　　　　*/
											}							/*　　　　　　　　　　　　　　　　　　　　　　*/
											else						/*　　　　　　　　　　　　　　　　　　　　　　*/
											{							/*　　　　　　　　　　　　　　　　　　　　　　*/
												if(in_time > 			/*　　　　　　　　　　　　　　　　　　　　　　*/
													midnig_strt[style-1])/* mid:22:00 DB:0 IN:22:01 OT:0:00  		  */
												{						/*　　　　　　　　　　　　　　　　　　　　　　*/
													if(ot_time > in_time)/*											  */
														teig_mid = 1;	/*											  */
												}						/*　　　　　　　　　　　　　　　　　　　　　　*/
											}							/*　　　　　　　　　　　　　　　　　　　　　　*/
										}								/*　　　　　　　　　　　　　　　　　　　　　　*/
									}									/*　　　　　　　　　　　　　　　　　　　　　　*/
									else								/*　　　　　　　　　　　　　　　　　　　　　　*/
									{									/*　　　　　　　　　　　　　　　　　　　　　　*/
										if((ot_time - T_DAY) > in_time)	/*　出庫時刻が入庫時刻を越えた　　　　　　　　*/
										{								/*　　　　　　　　　　　　　　　　　　　　　　*/
											teig_mid = 1;				/*　深夜料金加算ﾌﾗｸﾞON　　　　　　　　　　　　*/
										}								/*　　　　　　　　　　　　　　　　　　　　　　*/
										else							/*　　　　　　　　　　　　　　　　　　　　　　*/
										{								/*　　　　　　　　　　　　　　　　　　　　　　*/
											if(((cardmy.mon != car_ot.mon)/*　　　　　　　　　　　　　　　　　　　　　*/
												||(cardmy.day != car_ot.day))/*　　　　　　　　　　　　　　　　　　　 */
												&&(in_time == 0)		/*　　　　　　　　　　　　　　　　　　　　　　*/
												&&(ot_time == T_DAY))	/*　　　　　　　　　　　　　　　　　　　　　　*/
											{							/*　　　　　　　　　　　　　　　　　　　　　　*/
												teig_mid = 1;			/*　深夜料金加算ﾌﾗｸﾞON　　　　　　　　　　　　*/
											}							/*　　　　　　　　　　　　　　　　　　　　　　*/
										}								/*　　　　　　　　　　　　　　　　　　　　　　*/
									}									/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
								else									/*　　　　　　　　　　　　　　　　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									if(in_time > wk_tgend)				/*　入庫時刻が重複帯終了時刻を越えた　　　　　*/
									{									/*　　　　　　　　　　　　　　　　　　　　　　*/
										if(ot_time > in_time)			/*　出庫時刻が入庫時刻を越えた　　　　　　　　*/
											teig_mid = 1;				/*　深夜料金加算ﾌﾗｸﾞON　　　　　　　　　　　　*/
									}									/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
							teigen24 = 0;								/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						else{											/*　　　　　　　　　　　　　　　　　　　　　　*/
/* IN:23:59 一定料金帯を含む時の昼夜帯処理 ****************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
							if(((cal_time - pa_len) < T_DAY)&&			/*　1つ前の計算時刻が0:00前で今回の計算時刻が */
								(wk_cal_time >= T_DAY))					/*　0:00分を越えた　　　　　　　　　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								cal_time = in_add24;					/*　計算時刻をＩＮ＋２４とする　　　　　　　　*/
								if(in_time > (wk_cal_time-T_DAY) )		/*											  */
									cal_time = in_time;					/*　計算時刻をＩＮとする　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
							else										/*　　　　　　　　　　　　　　　　　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								cal_time = in_add24;					/*　計算時刻をＩＮ＋２４とする　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
/* ********************************************************************************************************************/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					else												/*　　　　　　　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						cal_time = in_time;								/*　計算時刻をＩＮ＋２４とする　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( cal_time == 0 )									/*　入庫が0時00分だった時のIN+24なら　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						cal_time = T_DAY;								/*　計算時刻を２４時間とする　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( h24_mid == 1 ){									/*　　　　　　　　　　　　　　　　　　　　　　*/
						ptn_ccnt = 0;									/*　　　　　　　　　　　　　　　　　　　　　　*/
						cycl_ccnt = 0;									/*　パタン繰り返し回数　　　　　　　　　　　　*/
						h24_mid = 0;									/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					else{												/*　　　　　　　　　　　　　　　　　　　　　　*/
						ec65_1(1);										/*											  */
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				rmax_cnt = 0;											/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			chk_max1 = 0;												/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(( cal_tm.year == car_ot.year )&&							/*　出庫時刻は次体系と同じ日にち　　　　　　　*/
				( cal_tm.day == car_ot.day )&&							/*　（cardmyと比べてはいけない！）　　　　　　*/
				( cal_tm.mon == car_ot.mon ))							/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(( in_time + T_DAY > next_style_strt )&&				/*　ＩＮ＋２４が次料金帯　かつ　　　　　　　　*/
						( ot_time > next_style_strt ))					/*　出庫時刻が次料金帯　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					chk_max1 = day_max1[next_style-1][car_type-1];		/*　２４時間最大料金を次の最大料金とする　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( wk_cal_time > next_style_strt )						/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					chk_max1 = day_max1[next_style-1][car_type-1];		/*　２４時間最大料金を次の最大料金とする　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(( in_time + T_DAY > next_style_strt)&&				/*　ＩＮ＋２４が次料金帯　かつ　　　　　　　　*/
						( wk_cal_time >= next_style_strt ))				/*　出庫時刻が次料金帯　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					chk_max1 = day_max1[next_style-1][car_type-1];		/*　２４時間最大料金を次の最大料金とする　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( chk_max1 == 0 )											/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				chk_max1 = day_max1[style-1][car_type-1];				/*　２４時間最大料金を今の最大料金とする　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		break;															/*　条件を満たさない場合０を返す　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		case 2:															/*　時刻指定日毎最大の場合　　　　　　　　　　*/
			if( ryoukin >= day_max1[style-1][car_type-1] )				/*　駐車料金が時刻指定最大料金に達している場合*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( ryoukin > day_max1[style-1][car_type-1] ){			/*　駐車料金が時刻指定最大料金に達している場合（==の場合はセットしない）*/
					ntnet_nmax_flg = 1;									// 区切り最大は1をセット 			
				}
				ryoukin = day_max1[style-1][car_type-1];				/*　時刻指定日毎最大料金を駐車料金とする　　　*/
				rmax_cnt++;												/*　最大到達カウントＵＰ　　　　　　　　　　　*/
				if(( rmax_cnt == 1 )&&( cal_type == 1 ))				/*　最大になった時のパタンセーブ（逓減）　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					ec65_1(0);											/*											  */
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			cal_time  = cal_time + pa_len;								/*　計算時刻を追加回数×追加時間進める　　　　*/
			if( cal_time >= next_style_strt + wk_strt)					/*　計算時刻が次体系開始時刻(+重複)に達す 　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				total = ryoukin + total;								/*　料金を集計中の駐車料金に加算　　　　　　　*/
				ryoukin = 0;											/*　料金を０クリア　　　　　　　　　　　　　　*/
				if( rmax_cnt > 1 )										/*　今の追加時間で最大となった訳でない！　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					cal_time = next_style_strt+wk_strt;					/*　計算時刻を次体系開始時刻(+重複)とする　　 */
					ec65_1(1);											/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				rmax_cnt = 0;											/*　　　　　　　　　　　　　　　　　　　　　　*/
				sitei_flg = OFF;										/*指定時刻日毎最大　フラグ　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else if( (cal_time >= pa_strt + T_DAY + wk_strt) && (sitei_flg == OFF) )/*　計算時刻が次料金開始時刻に達す*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				total = ryoukin + total;								/*　料金を集計中の駐車料金に加算　　　　　　　*/
				ryoukin = 0;											/*　料金を０クリア　　　　　　　　　　　　　　*/
				if( rmax_cnt > 1 )										/*　今の追加時間で最大となった訳でない！　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					cal_time = pa_strt + T_DAY + wk_strt;				/*　計算時刻を次料金帯開始時刻(+重複)とする　 */
					ec65_1(1);											/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				rmax_cnt = 0;											/*　　　　　　　　　　　　　　　　　　　　　　*/
				sitei_flg = ON;											/*　指定時刻日毎最大フラグ　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		break;															/*　条件を満たさない場合０を返す　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		case 3:															/*　２種類の時刻指定日毎最大の場合　　　　　　*/
			if( max_bnd[style-1] >= 4 )									/*　その体系で使用される料金帯が４以上の場合　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/

				if(band > max_bnd[style-1]){							/*　仮想料金帯の場合　　　　　　　　　　　　　*/
					bnd_strt4_wk = bnd_strt[style-1][4-1]+T_DAY;
					bnd_strt_ovl_wk = bnd_strt4_wk + ovl_time[style-1][4-1];
				}else{
					bnd_strt4_wk = bnd_strt[style-1][4-1];
					bnd_strt_ovl_wk = bnd_strt4_wk + ovl_time[style-1][4-1];
				}
				
				if(( cal_time < bnd_strt4_wk )||						/*　計算時刻が第４料金帯開始時刻以下／　　　　*/
					
				   ((cal_time == bnd_strt4_wk)&&(pa_len == 0))||			/*　第４料金帯開始時かつ一定料金／　　　　*/
				   
				   ((cal_time >= bnd_strt4_wk)&&							/*  計算時刻が第４料金帯重複帯内で重複０で出庫*/
				    (cal_time <= bnd_strt_ovl_wk )&&						/*　か重複帯内入出庫ならば　　　　*/
				    ((( et_dbl != OFF )||( et_jiot == NOF ))&&(band < 4))))	/*　　　　　　　　　　　　　　　　　　　　 */
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( ryoukin  >= day_max1[style-1][car_type-1] )		/*　駐車料金が第１日毎最大料金に達している場合*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						ryoukin = day_max1[style-1][car_type-1];		/*　第１日毎最大料金を駐車料金とする　　　　　*/
						rmax_cnt++;										/*　最大到達カウントＵＰ　　　　　　　　　　　*/
						if(( rmax_cnt == 1 )&&( cal_type == 1 ))		/*　最大になった時のパタンセーブ（逓減）　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							ec65_1(0);									/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					cal_time  = cal_time + pa_len;						/*　計算時刻を追加回数×追加時間進める　　　　*/

					if(((ovl_type[style-1][4-1] == 0)&&					/*　（（重複方式０　&&　　　　　　　　　　　　*/
						(cal_time >= bnd_end_tm)&&						/*　　計算時刻が現料金帯終了時刻以降　&&　　　*/
						((cal_time >= bnd_strt4_wk)&&					/*　　計算時刻が第４料金帯開始時刻後　&&　　　*/
						 (cal_time - pa_len < bnd_strt4_wk)))|| 			/*　計算時刻－単位時間が第４料金帯前）||　　*/
						 
						((ovl_type[style-1][4-1] == 1)&&				/*　　（重複方式１　&&　　　　　　　　　　　　*/
						((cal_time >= bnd_strt_ovl_wk)&& 			/*　計算時刻が第４料金帯　　 */
						 ( cal_time - pa_len < bnd_strt_ovl_wk))))  		/*　重複後　&&　　*/
																		/*　　　計算時刻－単位時間が重複帯前））　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						total = ryoukin + total;						/*　料金を集計中の駐車料金に加算　　　　　　　*/
						ryoukin = 0;									/*　料金を０クリア　　　　　　　　　　　　　　*/
						if( rmax_cnt > 1 )								/*　今の追加時間で最大となった訳でない！　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							if((et_dbl == ON)||(ovl_type[style-1][4-1] == 1)||	/*　　　　　　　　　　　　　　　　　　*/
							  ((et_jiot == NOF)&&(ovl_type[style-1][4-1] == 0)))/*　　　　　　　　　　　　　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								cal_time = bnd_strt[style-1][4-1]+wk_strt;	/*　計算時刻を第４料金帯開始時刻とする　　*/
								if(band > max_bnd[style-1]){			/*　仮想料金帯の場合　　　　　　　　　　　　　*/
									cal_time += T_DAY;					/*　１日加算　　　　　　　　　　　　　　　　　*/
								}
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
							else										/*　　　　　　　　　　　　　　　　　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								cal_time = bnd_strt[style-1][4-1];		/*　計算時刻を第４料金帯開始時刻とする　　　　*/
								if(band > max_bnd[style-1]){			/*　仮想料金帯の場合　　　　　　　　　　　　　*/
									cal_time += T_DAY;					/*　１日加算　　　　　　　　　　　　　　　　　*/
								}
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
							ec65_1(1);									/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						rmax_cnt = 0;									/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else 													/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( ryoukin  >= day_max2[style-1][car_type-1] ) 	/*　駐車料金が第２日毎最大料金に達している場合*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						ryoukin = day_max2[style-1][car_type-1];		/*　第２日毎最大料金を駐車料金とする　　　　　*/
						rmax_cnt++;										/*　最大到達カウントＵＰ　　　　　　　　　　　*/
						if(( rmax_cnt == 1 )&&( cal_type == 1 ))		/*　最大になった時のパタンセーブ（逓減）　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							ec65_1(0);									/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					cal_time  = cal_time + pa_len;						/*　計算時刻を追加回数×追加時間進める　　　　*/
					if(( cal_time >= next_style_strt + wk_strt)||		/*　計算時刻が次体系開始時刻に達し，かつ　　　*/
						(( cal_time >= next_style_strt )&&( pa_len == 0 )))	/*　　　　　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						total = ryoukin + total;						/*　料金を集計中の駐車料金に加算　　　　　　　*/
						ryoukin = 0;									/*　料金を０クリア　　　　　　　　　　　　　　*/
						if( rmax_cnt > 1 )								/*　今の追加時間で最大となった訳でない！　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							cal_time = next_style_strt + wk_strt;		/*　計算時刻を次体系開始時刻とする　　　　　　*/
							ec65_1(1);									/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						rmax_cnt = 0;									/*　　　　　　　　　　　　　　　　　　　　　　*/
						sitei_flg = OFF;								/*指定時刻日毎最大　フラグ　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					else if((cal_time >= pa_strt + T_DAY + wk_strt)&&(sitei_flg==OFF))/*計算時刻が次料金開始時刻に達す*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						total = ryoukin + total;						/*　料金を集計中の駐車料金に加算　　　　　　　*/
						ryoukin = 0;									/*　料金を０クリア　　　　　　　　　　　　　　*/
            			if( rmax_cnt > 1 )								/*　今の追加時間で最大となった訳でない！　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							cal_time = pa_strt + T_DAY + wk_strt;		/*　計算時刻を次体系開始時刻とする　　　　　　*/
							ec65_1(1);									/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						rmax_cnt = 0;									/*　　　　　　　　　　　　　　　　　　　　　　*/
						sitei_flg = ON;									/*指定時刻日毎最大　フラグ　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		break;															/*　条件を満たさない場合０を返す　　　　　　　*/

		case SP_MX_612HOUR:												/*　6,12時間最大の場合	　　　　　　　　　　　*/
			if( CPrmSS[S_CAL][13] == 12 )
				tmode = 12;
			else
				tmode = 6;
			if( ryoukin >= chk_max1 ){									/*　駐車料金が日毎最大料金以上				　*/
				if( (CPrmSS[S_CAL][14] == 0) ||							/*　　　　　　　　　　　　　　　　　　　　　　*/
					(( CPrmSS[S_CAL][14] != 0 ) &&						/*　　　　　　　　　　　　　　　　　　　　　　*/
					 (hour612chk(tmode) != 0)) ) {						/*											  */
					ryoukin = chk_max1;									/*　１２時間最大料金を駐車料金とする　　　　　*/
					rmax_cnt++;											/*　最大到達カウントＵＰ　　　　　　　　　　　*/
					if(( rmax_cnt == 1 )&&( cal_type == 1 ))			/*　最大になった時のパタンセーブ（逓減）　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						ec65_1(0);										/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( ptn_cycl[style-1][strt_ptn-1] == T_DAY )	/*　　　　　　　　　　　　　　　　　　　　　　*/
							rmax_pcn = 0;								/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( cycl_cnt[style-1] == T_DAY )				/*　　　　　　　　　　　　　　　　　　　　　　*/
							rmax_cyc = 0;								/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　					　　　　　　　　　　　*/
				}														/*　					　　　　　　　　　　　*/				
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			cal_time  = cal_time + pa_len;								/*　計算時刻を追加回数×追加時間進める　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			wk_cal_time = cal_time;										/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ( h24time + ( wk_cal_time - wk_h24t )) >= NTIME ){		/*　計算時刻が入庫後 日毎最大時間経過	　　　*/
				h24time = 0;											/*　現在までの計算時間トータルクリア　　　　　*/
				total = ryoukin + total;								/*　料金を集計中の駐車料金に加算　　　　　　　*/
				ryoukin = 0;											/*　料金を０クリア　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				in_add24 = in_time + NTIME;								/*　　　　　　　　　　　　　　　　　　　　　　*/
				if (( in_add24 > wk_cal_time ) && ( in_add24 >= T_DAY )){	/*　１計算範囲内超過　　　　　　　　　　　*/
					in_add24 = in_add24 - T_DAY;						/*　計算時刻調整　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( rmax_cnt > 1 ){										/*　今の追加時間で最大となった訳でない！　　　*/
					cal_time = in_add24;								/*　計算時刻をＩＮ＋日毎最大時間とする　　　　*/
					ec65_1(1);											/*											  */
					if ( cal_time < wk_cal_time ){						/*　時間調整実施　　　　　　　　　　　　　　　*/
						koetim = wk_cal_time - cal_time;				/*　　Ｎ時間超過分の単位料金追加　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				} else {												/*　　　　　　　　　　　　　　　　　　　　　　*/
					h24time = cal_time - in_add24;						/*　　　　　　　　　　　　　　　　　　　　　　*/
					if ( h24time < 0 ){									/*　　　　　　　　　　　　　　　　　　　　　　*/
						h24time = h24time + 1440;						/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				rmax_cnt = 0;											/*　　　　　　　　　　　　　　　　　　　　　　*/
				in_time = in_add24;										/* 日毎最大基点時刻更新						  */
				wk_h24t = cal_time;										/*　現在までの計算時間　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			chk_max1 = 0;												/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(( cal_tm.year == car_ot.year )&&							/*　出庫時刻は次体系と同じ日にち　　　　　　　*/
				( cal_tm.day == car_ot.day )&&							/*　（cardmyと比べてはいけない！）　　　　　　*/
				( cal_tm.mon == car_ot.mon ))							/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(( in_time + NTIME > next_style_strt )&&				/*　ＩＮ＋日毎最大時間が次料金体系　かつ　　　*/
						( ot_time > next_style_strt )){					/*　出庫時刻が次料金体系　　　　　　　　　　　*/
					chk_max1 = day_max1[next_style-1][car_type-1];		/*　２４時間最大料金を次料金体系最大料金とする*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( wk_cal_time > next_style_strt ){					/*　　　　　　　　　　　　　　　　　　　　　　*/
					chk_max1 = day_max1[next_style-1][car_type-1];		/*　２４時間最大料金を次料金体系最大料金とする*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			} else {													/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(( in_time + NTIME > next_style_strt)&&				/*　ＩＮ＋日毎最大時間が次料金体系　かつ　　　*/
						( wk_cal_time >= next_style_strt )){			/*　計算時刻時刻が次料金体系　　　　　　　　　*/
					chk_max1 = day_max1[next_style-1][car_type-1];		/*　２４時間最大料金を次料金体系最大料金とする*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( chk_max1 == 0 ){										/*　　　　　　　　　　　　　　　　　　　　　　*/
				chk_max1 = day_max1[style-1][car_type-1];				/*２４時間最大料金を今の料金体系最大料金とする*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		break;															/*　条件を満たさない場合０を返す　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		case SP_MX_N_HOUR:												/*　ｎ時間最大ｍ回適用　　　　　　　　　　　　*/
																		/*　　時間割引定期不可　　　　　　　　　　　　*/
																		/*　　時間サービス券不可　　　　　　　　　　　*/
																		/*　　重複帯不可　　　　　　　　　　　　　　　*/
																		/*　　一定料金帯不可　　　　　　　　　　　　　*/
																		/*　　逓減帯不可　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			T_N_HOUR = (short)CPrmSS[S_CAL][13] * 60;					/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ryoukin >= chk_max1 )									/*　駐車料金がｎ時間最大料金を超えた場合　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( ryoukin > chk_max1 ){								/*　駐車料金がｎ時間最大料金を超えた場合（==の場合はセットしない）　　　*/
					ntnet_nmax_flg = 2;									// 時間最大は2をセット
				}
				ryoukin = chk_max1;										/*　ｎ時間最大料金を駐車料金とする　　　　　　*/
				rmax_cnt++;												/*　最大到達カウントＵＰ　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　逓減帯?(Y)　　　　　　　　　　　　　　　　*/
				if(( rmax_cnt == 1 )&&( cal_type == 1 ))				/*　最大になった時のパタンセーブ（逓減）　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					ec65_1(0);											/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( ptn_cycl[style-1][strt_ptn-1] == T_DAY )		/*　　　　　　　　　　　　　　　　　　　　　　*/
						rmax_pcn = 0;									/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( cycl_cnt[style-1] == T_DAY )					/*　　　　　　　　　　　　　　　　　　　　　　*/
						rmax_cyc = 0;									/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			cal_time  = cal_time + pa_len;								/*　計算時刻を追加回数×追加時間進める　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			wk_cal_time = cal_time;										/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(( iti_flg == ON )&&( sp_it == 1 ))						/*　一定料金帯に入った時の余り時間を　　　　　*/
			{															/*　終了時刻に加算する?(Y)　　　　　　　　　　*/
				wk_cal_time = cal_time + amartim;						/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/* ************ 第１料金帯が一定料金の時の料金計算調整 ****************************************************************/
			if(( CPrmSS[S_CAL][1] != 1)&&(rmax_cnt == 1)&&				/*　昼夜帯なら　　　　　　　　　　　　　　　　*/
				(band == 1)&&(max_bnd[style-1] > 1)&&					/*　現在第１料金帯で２つ以上の料金帯設定　　　*/
				(leng_exp[style-1][0] == 0)&&							/*　第１料金帯が一定料金設定　　　　　　　　　*/
				(leng_std[style-1][0] == 0)&&							/*　　　　　　　　　　　　　　　　　　　　　　*/
				(cal_time == (bnd_strt[style-1][band]					/*　計算時刻が次の料金帯開始である。　　　　　*/
						+ovl_time[style-1][band])))						/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*											  */
				if(cal_time >= T_DAY)									/*	計算時刻が24Hを越えた					  */
				{														/*											  */
					if((in_time >= bnd_strt[style-1][band-1]&&			/*	入庫時刻が一定帯開始時刻を越えかつ		  */
						in_time < T_DAY))								/*	0:00以内								  */
					{													/*											  */
						if(in_time < ot_time)							/*	出庫時刻が入庫時刻を越えた				  */
						{												/*											  */
							h24_flg = 1;								/*	料金加算ﾌﾗｸﾞON							  */
							rmax_cnt ++;								/*											  */
						}												/*											  */
					}													/*											  */
					else if((in_time >= 0)&&							/*	入庫時刻が0:00から次の料金帯以内		  */
						(in_time <= (cal_time - T_DAY)))				/*											  */
					{													/*											  */
						if(in_time < (ot_time - T_DAY))					/*	出庫時刻が入庫時刻を越えた				  */
						{												/*											  */
							h24_flg = 1;								/*	料金加算ﾌﾗｸﾞON							  */
							rmax_cnt ++;								/*											  */
						}												/*											  */
					}													/*											  */
				}														/*											  */
				else													/*	計算時刻が24Hを越えていない				  */
				{														/*											  */
					if((in_time >= bnd_strt[style-1][band-1])&&			/*	入庫時刻が一定帯内　　　				  */
						(in_time <= cal_time))							/*											  */
					{													/*											  */
						if(in_time < ot_time)							/*	出庫時刻が入庫時刻を越えた				  */
						{												/*											  */
							h24_flg = 1;								/*	料金加算ﾌﾗｸﾞON							  */
							rmax_cnt ++;								/*											  */
						}												/*											  */
					}													/*											  */
				}														/*											  */
			}															/*											  */
			/*--------------------------------------------------------*//*　　　　　　　　　　　　　　　　　　　　　　*/
			/* ｎ時間経過後、ｍ回判定                                 *//*　　　　　　　　　　　　　　　　　　　　　　*/
			/*--------------------------------------------------------*//*　　　　　　　　　　　　　　　　　　　　　　*/
			n_hour_wk = h24time + (wk_cal_time - wk_h24t);				/*　経過時間を算出　　　　　　　　　　　　　　*/
			if( n_hour_wk >= T_N_HOUR ) 								/*　ｎ時間経過?(Y)　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				n_max_set=(short)CPrmSS[S_CAL][14];						/*　ｍ回適用　０は∞適用とする　　　　　　　　*/
					 													/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(( n_max_set != 0 )&&( n_hour_cnt < n_max_set)){		/*　∞以外かつｍ回カウント中?(Y)　　　　　　　*/
					 													/*　　　　　　　　　　　　　　　　　　　　　　*/
					n_hour_cnt += ( n_hour_wk / T_N_HOUR);				/*　ｎ時間最大ｍ回の適用回数　　　　　　　　　*/
																		/*　一定時間を考慮するとT_N_HOURの倍数を　　　*/
																		/*　カウントする必要がある　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				n_hour_wk %= T_N_HOUR;									/*　ｎ時間かそれ以上か　　　　　　　　　　　　*/
				if( n_hour_wk != 0 ){									/*　ｎ時間以上?(Y)　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　一定時間帯の時にｎ時間以上がありえる　　　*/
					wk_h24t = h24time + cal_time - n_hour_wk;			/*　ｎ時間の基点から計算済みの時間数　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				}else{													/*　ｎ時間?(Y)　　　　　　　　　　　　　　　　*/
					wk_h24t = h24time + cal_time;						/*　ｎ時間の基点から計算済みの時間数　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			/*--------------------------------------------------------*//*　　　　　　　　　　　　　　　　　　　　　　*/
			/* sp_mx=1の処理をそのまま流用(実際はsp_mx=5,6)           *//*　　　　　　　　　　　　　　　　　　　　　　*/
			/*--------------------------------------------------------*//*　　　　　　　　　　　　　　　　　　　　　　*/
				max24_fg = 1;											/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(CPrmSS[S_CAL][1] == 1)								/*　料金計算方式が逓減帯　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if(style_type[style-1] == 2)						/*　深夜時間を考慮するなら　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(midnig_ovl[style-1] != 0)					/*　昼夜帯との重複帯あり　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							itei_end = midnig_strt[style-1]				/*　　　　　　　　　　　　　　　　　　　　　　*/
										+midnig_ovl[style-1];			/*　重複帯の終了時刻を求める　　　　　　　　　*/
							if(itei_end < T_DAY)						/*　重複帯終了時刻が０：００を越えていない　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								if((in_time >= midnig_strt[style-1])&&	/*　入庫時刻が重複帯内　　　　　　　　　　　　*/
									(in_time <= itei_end))				/*　　　　　　　　　　　　　　　　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									wk_h24t = in_time;					/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
							else										/*　重複帯の終了時刻が０：００を越えた　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								if(in_time >= midnig_strt[style-1])		/*　０：００分より前（重複帯内）　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									wk_h24t = in_time;					/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
								else if(in_time <= (itei_end-T_DAY))	/*　０：００分より後（重複帯内）　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									wk_h24t = in_time + T_DAY;			/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　料金計算方式が昼夜帯　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if(tyu24_flg == 1)									/*　一定料金帯の場合　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(ovl_time[style-1][band-1] != 0)				/*　一定料金帯に重複時間あり　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							itei_end = bnd_strt[style-1][band-1]		/*　一定料金帯の重複時間を求める　　　　　　　*/
									+ovl_time[style-1][band-1];			/*　　　　　　　　　　　　　　　　　　　　　　*/
							if(itei_end < T_DAY)						/*　重複帯の終了時刻が０：００を越えていない　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								if((in_time >= bnd_strt[style-1][band-1])&&	/*　入庫時刻が重複帯内　　　　　　　　　　*/
									(in_time <= itei_end))				/*　　　　　　　　　　　　　　　　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									wk_h24t = in_time;					/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
							else										/*　重複帯の終了時刻が０：００を越えた　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								if(in_time >= bnd_strt[style-1][band-1])/*　入庫時刻が重複帯内　　　　　　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									wk_h24t = in_time;					/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
								else if(in_time <= (itei_end-T_DAY))	/*　入庫時刻が重複帯内　　　　　　　　　　　　*/
								{										/*　　　　　　　　　　　　　　　　　　　　　　*/
									wk_h24t = in_time + T_DAY;			/*　　　　　　　　　　　　　　　　　　　　　　*/
								}										/*　　　　　　　　　　　　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				total = ryoukin + total;								/*　料金を集計中の駐車料金に加算　　　　　　　*/
				ryoukin = 0;											/*　料金を０クリア　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			/*--------------------------------------------------------*//*　　　　　　　　　　　　　　　　　　　　　　*/
			/* ｎ時間最大判定、ｍ回適用判定					          *//*　　　　　　　　　　　　　　　　　　　　　　*/
			/*--------------------------------------------------------*//*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				n_max_set=(short)CPrmSS[S_CAL][14];						/*　ｍ回適用　０は∞適用とする　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(( n_hour_cnt >= n_max_set )&&( n_max_set != 0 )){	/*　ｍ回終了かつ∞でない?(Y)　　　　　　　　　*/
					chk_max1 = N_MAX_OVER;								/*　ｎ時間最大ｍ回経過後の最大料金　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				}else{													/*　ｍ回継続中または∞?(Y)　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( cal_time == next_style_strt ){					/*　次の料金体系の開始時間とちょうど同じ?(Y)　*/
						n_hour_style = next_style;						/*　ｎ時間最大の最大料金を取る料金体系　　　　*/
					}else{												/*　　　　　　　　　　　　　　　　　　　　　　*/
						n_hour_style = style;							/*　ｎ時間最大の最大料金を取る料金体系　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					chk_max1 = day_max1[n_hour_style-1][car_type-1];	/*　ｎ時間最大料金　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				rmax_cnt = 0;											/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			break;														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			/*--------------------------------------------------------*//*　　　　　　　　　　　　　　　　　　　　　　*/
			/* 時間帯上限料金（料金帯上限料金）				          *//*　　　　　　　　　　　　　　　　　　　　　　*/
			/*--------------------------------------------------------*//*　　　　　　　　　　　　　　　　　　　　　　*/
		case SP_MX_BAND:												/*　時間帯上限料金　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			chk_max1 = band_max[band-1][style-1][car_type-1];			/*　時間帯上限料金参照　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ryoukin >= chk_max1 )									/*　駐車料金が時間帯最大料金を超えた場合　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( ryoukin > chk_max1 ){								/*　駐車料金が時間帯最大料金を超えた場合（==の場合はセットしない）　　　*/
					ntnet_nmax_flg = 3;									// 時間帯最大は3をセット
				}
				ryoukin = chk_max1;										/*　ｎ時間最大料金を駐車料金とする　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			cal_time  = cal_time + pa_len;								/*　計算時刻を追加回数×追加時間進める　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( band != (char)last_bnd )								/*　料金帯が次体系直前の料金帯以外の場合　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( cal_time >= bnd_strt[style-1][band] ){				/*　次料金帯の開始時刻以上　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					total = ryoukin + total;							/*　料金を集計中の駐車料金に加算　　　　　　　*/
					ryoukin = 0;										/*　料金を０クリア　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}else{														/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( cal_time >= next_style_strt ){						/*　次料金帯（次料金体系）の開始時刻以上　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					total = ryoukin + total;							/*　料金を集計中の駐車料金に加算　　　　　　　*/
					ryoukin = 0;										/*　料金を０クリア　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			break;														/*　　　　　　　　　　　　　　　　　　　　　　*/

		case 10:														// ｎ時間最大ｍ回適用
		case SP_MX_N_MHOUR_W:											// 
																		// 
			nmax_tim1 = NMAX_HOUR1;										// ｎ時間最大(外)時間
			nmax_tim2 = NMAX_HOUR2;										// ｎ時間最大(内)時間
			nm_set1 = nmax_set1;										// ｎ時間最大徴収回数
			if( EXP_FeeCal == 1 ){										// 料金種別毎最大料金あり
				nmax_tim1 = EXP_NMAX_HOUR1[car_type-1];					// 
				nmax_tim2 = EXP_NMAX_HOUR2[car_type-1];					// 
				nm_set1 = exp_nmax_set1[car_type-1];					// 
			}															// 
																		// 
			chk_max1_bf = 0;											// 体系基準方式で使用する変数
			chk_max1_flg = 0;											// 体系基準方式で使用する変数
			ryo_bak = 0;												// 
			ryo_over= 0;												// 
																		// 
			if( nmax_tani_type == 3 ){									// 
				ryo_bak = ryoukin;										// 
			}															// 
			if(	(nmax_itei != 0)&&(iti_flg == ON) ){					// 一定料金帯はｎ時間最大に含めない時
																		// 一定料金はそのまま加算とする
			}else{														// 一定料金帯もｎ時間最大に含める時
				if( ryoukin >= chk_max1 ){								// 駐車料金がｎ時間最大料金を超えた場合
					if( ryoukin > chk_max1 ){							// 駐車料金がｎ時間最大料金を超えた場合（==の場合はセットしない）
						if( FeeMaxType == SP_MX_N_MH_NEW ){
							ntnet_nmax_flg = 2;							// 時間最大は2をセット
						}else{	// FeeMaxType == SP_MX_N_MHOUR_W
							ntnet_nmax_flg = 4;							// 時間最大(W)は4をセット
						}
					}
					ryoukin = chk_max1;									// ｎ時間最大料金を駐車料金とする
				}														// 
			}															// 
			cal_time	= cal_time + pa_len;							// 計算時刻を追加回数×追加時間進める
			wk_cal_time = cal_time;										// 
																		// 
			if( (iti_flg == ON)&&(sp_it == 1) ){						// 一定料金帯に入った時の余り時間を終了時刻に加算する?(Y)
				if( amr_flg == ON ){									// amr_flg = ONの場合にはまだ余り時間が加算されていない
					wk_cal_time = wk_cal_time + amartim;				// 
				}														// 
			}															// 
																		// 
			memcpy( &cal_tm24, &cal_tm, 7 );							// 計算時刻をワークへコピー
			cal_tm24.hour = 0;											// ※cal_tmに現在の計算時刻が入っていないため
			cal_tm24.min  = 0;											//   cal_tmにcal_time分を加算して現在の計算時刻
			ec70( &cal_tm24,(long)(wk_cal_time) );						//   を求める
																		// 
// ---------------------------------------------------------------------------------------------------------------------
// 料金計算開始時のcal_tmは入庫日の体系切換時刻にセットされるため、入庫時刻が体系切換時刻より前の場合翌日となってしまう 
// 実際には1日を越えることは無いため、1日を越えていた場合には1日前に戻すようにする										
// 例：  体系切換時刻が8:00、入庫時刻が1:00の場合にはcal_timeは1500となり、cal_tm24が料金計算開始時は1日先の日付でセット
//		 となるよってcal_tmとcal_tm24を比較し1441以上の場合には1日戻す処理を追加する。 									
// ---------------------------------------------------------------------------------------------------------------------
			if( ec71a( &cal_tm, &cal_tm24 ) > 1440L ){					// 
				memcpy( &wk_time, &cal_tm, 7 );							// 計算時刻をワークへコピー
				wk_time.hour = 0;										// ※cal_tmに現在の計算時刻が入っていないため
				wk_time.min  = 0;										//   cal_tmにcal_time分を加算して現在の計算時刻
				ec70( &wk_time,(long)(wk_cal_time-pa_len) );			//   を求める
// -----------------------------------------------------------------	// 
// 入庫時刻をずらし今回の単位時間加算で体系切換時刻を越えると1日前に	// 
// 日付を戻してしまうため今回越えたのかを確認するように修正				// 
//------------------------------------------------------------------	// 
				if( ec71a( &cal_tm, &wk_time ) >= 1440L ){				// 
					ec62( &cal_tm24 );									// 
				}														// 
			}															// 
			in_cal_tm = ec71a( &nmax_in, &cal_tm24 );					// 実際の入庫時刻から何分経過したか算出する
// -----------------------------------------------------------------
// 設定にて一定料金帯はＮ時間最大の対象外となっていた場合には一定料	
// 金帯後の時刻を基準時刻にし強制的にＮ時間経過したとする。但し、	
// 一定料金帯突入でiti_flgがONとなっていることが前提となるため		
// iti_flgが正常に機能していない場合にはNGとなる					
// -----------------------------------------------------------------
			itei_reset = 0;												// 一定料金帯後のリセットフラグクリア
			if(	(nmax_itei != 0)&&(iti_flg == ON) ){					// 一定料金帯はＮ時間最大の対象外で今回一定料金帯
// -----------------------------------------------------------------
// 一定料金帯後の余りありでも余りなしの基準時刻にする
// -----------------------------------------------------------------
				if( (n_itei_adj == 1)&&(nmax_itei == 1) ){				// cal_timeに余り時間加算済み
					memcpy( &wk_time, &cal_tm24, 7 );					// 
					ec66( &wk_time, (long)amartim );					// 加算した余り時間を減算する
					memcpy( &nmax_in, &wk_time, 7 );					// 新しい基準時刻として登録する
				}else{													// 
					memcpy( &nmax_in, &cal_tm24, 7 );					// 基準時刻を一定料金帯後とする
				}														// 
				in_cal_tm = nmax_tim1;									// ｎ時間経過とする
				itei_reset = 1;											// 一定料金帯後のリセットフラグON
				in_calcnt = 0;											// ｎ時間回数クリア
			}															// 
																		// 
			/*----------------------------------------------------*/	// 
			/* ｎ時間経過後、ｍ回判定                             */	// 
			/*----------------------------------------------------*/	// 
			wk_calcnt = (short)(in_cal_tm/nmax_tim1);					// 何回ｎ時間に到達したか求める
																		// 
			if(FeeMaxType == SP_MX_N_MHOUR_W){							// 2種類のｎ時間最大
				if(wk_calcnt == in_calcnt){								// たった今ｎ１を越えた場合に入ってしまう為、条件を付ける
																		// 
					nmax_amari2 = 0;									// 
					wmax_cal_tm = (ushort)(in_cal_tm%nmax_tim1);		// ｎ1開始からの経過時間算出
					if(nmax_tim2 >= wmax_cal_tm ){						// ｎ1開始からの経過時間がｎ2時間以下（ｎ2最大対象）
						if( ryoukin >= chk_max2 ){						// 駐車料金がｎ時間最大料金を超えた場合
							if( ryoukin > chk_max2 ){					// 駐車料金がｎ時間最大料金を超えた場合（==の場合はセットしない）
								ntnet_nmax_flg = 4;						// W上限のｎ1最大料金超えは4をセット
							}
							ryoukin = chk_max2;							// ｎ時間最大料金を駐車料金とする
						}												// 
					}else{												// 
																		// ｎ1開始からの経過時間がｎ2を越えた
						nm_wtm = nmax_in.hour*60+nmax_in.min;			// ｎ1時間基点時刻を分単位で求める
						nm_wtm = (nm_wtm + 								// 
							(short)(((long)nmax_tim1*(long)wk_calcnt)%1440L))%1440;	// Ｎ時間基点時刻からＮ時間を求める
																		// 
						ac_wtm = wk_cal_time%1440;						// 今回の計算時刻
						if( ac_wtm < nm_wtm ){							// 
							ac_wtm += 1440;								// 
						}												// 
						ac_wtm = ac_wtm - nm_wtm;						// 
																		// 
						bc_wtm = bf_cal_time%1440;						// 前回の計算時刻
						if( bc_wtm < nm_wtm ){							// 
							bc_wtm += 1440;								// 
						}												// 
						bc_wtm = bc_wtm - nm_wtm;						// 
																		// 
						if( (nmax_tim2 > bc_wtm)&&						// 今回の単位時間の加算にてｎ2最大時間を越えた
							(nmax_tim2 <= ac_wtm) ){					// 
							nmax_amari2 = wmax_cal_tm - nmax_tim2;		// 料金が最大料金に到達している
							if( ryoukin >= chk_max2 ){					// 駐車料金がｎ時間最大料金を超えた場合
								if( ryoukin > chk_max2 ){				// 駐車料金がｎ時間最大料金を超えた場合（==の場合はセットしない）
									ntnet_nmax_flg = 4;					// W上限のｎ1最大料金超えは4をセット
								}
								ryoukin = chk_max2;						// ｎ時間最大料金を駐車料金とする
							}else{										// 料金が最大料金に到達していない
								if(	nmax_tani_type == 2 ){				// 設定が最大料金に達しているときに切り捨てる
									nmax_amari2 = 0;					// 
								}										// 
							}											// 
						}												// 
					}													// 
				}														// 
			}else{														// 
				nmax_amari2 = 0;										// 
			}															// 
																		// 
//--------------------------------------------------------------------------------------------------
// 単位時間の余りを切捨とした時にＮ時間を越えていなくてもMAX以上の料金を徴収してしまう。			
// 例： 第1料金体系-第1料金帯8:00 基本、拡張共に100円/1:0024時間最大1000円 入庫:前日8:05 出庫8:01 	
//		1100円と表示される（本当は24時間以内なので1000円）											
//--------------------------------------------------------------------------------------------------
			// -------------------------------------------------------
			// 単位時間内のｎ時間最大時刻と体系切換時刻の前後関係ﾁｪｯｸ 
			// -------------------------------------------------------
			if( (wk_calcnt != in_calcnt)&&								// ｎ時間到達回数が変化した
				( ((sp_op == 1) && (sp_it == 0))||						// 余り処理方式＝１（切り捨て）の場合かつ
				  ((sp_op == 1) && (sp_it == 1) && (amr_flg == OFF))) )	// sp_op=1 叉は 余り処理方式＝１（切り捨て）
			{															// 
				nm_wtm = nmax_in.hour*60+nmax_in.min;					// Ｎ時間基点時刻を分単位で求める
				nm_wtm = (nm_wtm + 										// 
					(short)(((long)nmax_tim1*(long)wk_calcnt)%1440L))%1440;	// Ｎ時間基点時刻からＮ時間を求める
																		// 
				ac_wtm = wk_cal_time%1440;								// 単位時間加算後の時刻
				bc_wtm = bf_cal_time%1440;								// 単位時間加算前の時刻
																		// 
				if( band == (char)last_bnd ){							// 料金帯が最高使用料金帯の場合
					wk_style_strt = next_style_strt;					// 
				}else{													// 料金帯が最高使用料金帯以外の場合
					wk_style_strt = bnd_strt[style-1][band];			// 
				}														// 
																		// 
				bs_wtm = wk_style_strt%1440;							// 
				if( wk_cal_time > wk_style_strt ){						// 今回の単位時間加算で次の料金帯切換時刻を越えた
																		// 
					if( bc_wtm > ac_wtm ){								// 今回の単位時間加算で0:00を越えた(0:00含む)
																		// 
						if( bc_wtm < bs_wtm ){							// next_styleは0:00より前(0:00は含まない)
							if( (bs_wtm <= nm_wtm)||(nm_wtm <= ac_wtm) ){	// 
								wk_calcnt = in_calcnt;					// 今回はＮ時間を越えていないとする
							}											// 
						}else{											// next_styleは0:00より後(0:00含む)
							if( (bs_wtm < nm_wtm)&&(nm_wtm <= ac_wtm) ){
								wk_calcnt = in_calcnt;					// 今回はＮ時間を越えていないとする
							}											// 
						}												// 
					}else{												// 今回の単位時間加算で0:00を越えていない
						if( bs_wtm < nm_wtm ){							// 料金帯切換時刻よりＮ時間が後にある場合
							wk_calcnt = in_calcnt;						// 今回はＮ時間を越えていないとする
						}												// 
					}													// 
				}														// 
			}															// 
																		// 
			if(	(nmax_point == NMAX_OUT_PIT)|| 							// 出庫基準(従来の方式)又は体系基準の時
				(nmax_point == NMAX_TAI_PIT) ){							// 
																		// 
				chk_max1_bf = chk_max1;									// 体系基準用最大料金保持
				chk_max1 = 0;											// 最大料金額格納変数クリア
																		// 
				if( (nmax_cnt >= nm_set1)&&(nm_set1 != 0) ){			// ｎ時間最大料金徴収回数制限ありで上限超え
					chk_max1 = NMAX_OVER;								// ｎ時間最大ｍ回経過後の最大料金
					chk_max2 = NMAX_OVER;								// 
				}														// 
			}															// 
																		// 
			nmax_amari = 0;												// ｎ時間経過時の単位時間の余り時間クリア
																		// 
			// ---------------------
			// ｎ時間を越えた時		
			// ---------------------
			if( wk_calcnt != in_calcnt ){								// ｎ時間到達回数が変化した(ｎ時間を越えた)
																		// 
				nmax_amari = (short)(in_cal_tm%nmax_tim1);				// 最大時刻到達時の単位時間の余り時間算出
				if(	nmax_tani_type == 2 ){								// 設定が最大料金に達しているときに切り捨てる
					if( ryoukin < chk_max1 ){							// 最大料金に達していない
						nmax_amari = 0;									// 単位時間の余りを無しにする
					}													// 
				}														// 
				if( nmax_tani_type == 3 ){								// 単位時間はそのままで最大時刻を越えているかで判定
					if( nmax_amari != 0 ){								// ｎ時間と単位時間に余り時間がある時
//------------------------------------------------------------------
// 現在の計算時刻から余り時間を引いた時刻がＮ時間到達時刻となるはず	
//------------------------------------------------------------------
						memcpy( &wk_time, &cal_tm24, 7 );				// 現在計算時刻から今回のＮ時間最大時刻を求める
						ec66( &wk_time, (long)nmax_amari );				// 
						if( ec64( &org_ot,&wk_time ) == -1 ){			// 出庫時刻がＮ時間到達時刻を越えている
//--------------------------------------------------------------------
// 最大料金を越えていない場合にはryoukinに対して補正がかかってないため
// ryoukin = ryo_bakとなるのでryo_overは必ず0となる					  
//--------------------------------------------------------------------
							ryo_over = ryo_bak - ryoukin;				// 次のＮ時間へ余り料金を引き継ぐ
							if( ryo_over >= chk_max1 ){					// 余り料金が最大料金以上の場合
								ryo_over = chk_max1;					// 余り料金を最大料金とする
							}											// 
						}												// 
					}													// 
				}														// 
																		// 
				if( itei_reset == 1 ){									// 一定料金帯は対象外の時
					wk_calcnt = in_calcnt = 0;							// 基準時刻をﾘｾｯﾄするのでｶｳﾝﾄもﾘｾｯﾄする
				}														// 
																		// 
				if( nmax_countup == 1 ){								// ｎ時間ｶｳﾝﾄは｢最大を越えた時にｶｳﾝﾄ｣する
					if( ryoukin >= chk_max1 ){							// 駐車料金がｎ時間最大料金を超えた場合
						nmax_cnt ++;									// 最大料金徴収回数ｶｳﾝﾄｱｯﾌﾟ
					}													// 
				}else{													// 最大料金に関係なくｎ時間を超えた時にｶｳﾝﾄの場合
					nmax_cnt ++;										// 最大料金領収回数ｶｳﾝﾄｱｯﾌﾟ
				}														// 
				chk_max1_flg = 1;										// 
				total = ryoukin + total;								// 料金を集計中の駐車料金に加算
																		// 
				if( nmax_tani_type == 3 ){								// 単位時間はそのままで最大時刻を越えているかで判定
					ryoukin = ryo_over;									// 
				}else{													// 
					ryoukin = 0;										// 
				}														// 
																		// 
			/*----------------------------------------------------*/	// 
			/* ｎ時間最大判定、ｍ回適用判定					      */	// 
			/*----------------------------------------------------*/	// 
				if(( nmax_cnt >= nm_set1 )&&( nm_set1 != 0 )){			// ｎ時間最大料金徴収回数制限ありで上限超え
					chk_max1 = NMAX_OVER;								// ｎ時間最大ｍ回経過後の最大料金
					chk_max2 = NMAX_OVER;								// ｎ時間最大ｍ回経過後の最大料金
				}else{													// ｍ回継続中または∞?(Y)
					wk_cal_time = cal_time;								// 
					if( nmax_tani_type ){								// Ｎ時間最大の単位時間の余り時間を継続する以外
						wk_cal_time = cal_time - nmax_amari;			// 余り時間分cal_timeを戻す
					}													// (計算時刻を戻した時点の体系の最大とする)
																		// 
					if( wk_cal_time >= next_style_strt ){				// 次の料金体系の開始時間を越えた?(Y)
						nmax_style = next_style;						// ｎ時間最大の最大料金を取る料金体系
					}else{												// 
						nmax_style = style;								// ｎ時間最大の最大料金を取る料金体系
					}													// 
					chk_max1 = day_max1[nmax_style-1][car_type-1];		// ｎ時間最大料金を更新する
					chk_max2 = day_max2[nmax_style-1][car_type-1];		// ｎ時間最大料金を更新する
				}														// 
			}															// 
																		// 
			if(	(nmax_point == NMAX_OUT_PIT)|| 							// 出庫基準(従来の方式)又は体系基準の時
				(nmax_point == NMAX_TAI_PIT) ){							// 
				if( chk_max1 == 0 ){									// 出庫基準、または体系基準の時
					if( cal_time >= next_style_strt ){					// 
						chk_max1 = day_max1[next_style-1][car_type-1];	// ｎ時間最大料金を次の体系の最大料金とする
						chk_max2 = day_max2[next_style-1][car_type-1];	// ｎ時間最大料金を更新する
					}else{												// 
						chk_max1 = day_max1[style-1][car_type-1];		// ｎ時間最大料金を今の体系の最大料金とする
						chk_max2 = day_max2[style-1][car_type-1];		// ｎ時間最大料金を更新する
					}													// 
																		// 
					if( nmax_point == NMAX_TAI_PIT ){ 					// 体系基準の時
						if( chk_max1_flg == 0 ){						// ｎ時間を越えていない
							if( chk_max1 < chk_max1_bf ){				// 更新した最大料金が前回の最大料金より小さい場合
								chk_max1 = ryoukin;						// 現在の料金を最大料金として維持する
							}											// 
						}												// 
					}													// 
				}														// 
			}															// 
			in_calcnt = wk_calcnt;										// ｎ時間経過回数を保存する
		break;															// 
																		// 
		default:														/*　日毎最大料金タイプが０～３以外の場合　　　*/
		break;															/*　条件を満たさない場合０を返す　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	n_itei_adj = 0;														// 一定料金帯後の余り時間の調整ﾌﾗｸﾞ
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：日毎最大料金処理　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：ec65_1(par1)　　　　　　　：日毎最大料金処理　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　計算対象分　　　　　　　　　リタンコード　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
void	ec65_1(															/*　　　　　　　　　　　　　　　　　　　　　　*/
char	bk_rst															/*　　　　　　　　　　　　　　　　　　　　　　*/
)																		/*											  */
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( bk_rst == 1 )													/*　ﾊﾞｯｸｱｯﾌﾟした変数を元に戻す　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		strt_ptn = rmax_ptn;											/*　　　　　　　　　　　　　　　　　　　　　　*/
		ptn_ccnt = rmax_pcn;											/*　　　　　　　　　　　　　　　　　　　　　　*/
		cycl_ccnt = rmax_cyc;											/*　パタン繰り返し回数　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　ﾊﾞｯｸｱｯﾌﾟする　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		rmax_ptn = strt_ptn;											/*　現在パタン　　　　　　　　　　　　　　　　*/
		rmax_pcn = ptn_ccnt;											/*　パタン係数　　　　　　　　　　　　　　　　*/
		rmax_cyc = cycl_ccnt;											/*　パタン繰り返し回数　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：                　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：day_1st_check   　　　　　：日毎最大料金処理　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　計算対象分　　　　　　　　　リタンコード　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
int	day_1st_check( void )												/*											  */
{																		/*											  */
	struct CAR_TIM		w_in;											/*											  */
	struct CAR_TIM		cal_wk;											/*											  */
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	int				tim;												/*											  */
																		/*											  */
	if( cal_type != 0 )			return( 0 );							/*											  */
																		/*											  */
	memcpy( &cal_wk, &cal_tm, sizeof( cal_wk ));						/*											  */
	memcpy( &w_in, &car_in, sizeof( w_in ));							/*											  */
	ec63( &w_in );														/*　計算日を１日進める				  　　　　*/
																		/*											  */
	cal_wk.hour = 0;													/*											  */
	cal_wk.min  = 0;													/*											  */
	ec70( &cal_wk, (long)cal_time );									/*　更新	　　　　　　　　　　　　　　　　　*/
																		/*											  */
	tim = ec64( &w_in,&cal_wk )	;										/*											  */
																		/*											  */
	if( tim == 0 )														/*											  */
	{																	/*											  */
		return( 1 )	;													/*											  */
	}else if( tim == -1 )												/*											  */
	{																	/*											  */
		return( 1 )	;													/*											  */
	}else																/*											  */
	{																	/*											  */
		return( 0 ) ;													/*											  */
	}																	/*											  */
}																		/*											  */

short	hour612chk(short tmod)											/*											  */
{																		/*											  */
	unsigned short	wtm, witm, wctm, wday;								/*											  */
	short	ret = 1;													/*											  */
																		/*											  */
	wday = dnrmlzm((short)cal_tm.year, (short)cal_tm.mon, (short)cal_tm.day) -	/*											  */
			 dnrmlzm((short)org_in.year, (short)org_in.mon, (short)org_in.day);	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*											  */
	witm = (org_in.hour * 60) + org_in.min;								/*　入庫時刻　　　　　　　　　　　　　　　　　*/
	wctm = ((((cal_time % 1440) / 60) * 60) + (cal_time % 1440) % 60);	/*　計算時刻								  */
																		/*											  */
	if (wday >= 2) {													/*											  */
		ret = 0;														/*											  */
	}																	/*											  */
	else {																/*											  */
		wtm = (wctm + (wday * 1440)) - witm;							/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*											  */
		if (wtm > (long)(tmod * 60)) {									/*											  */
			ret = 0;													/*											  */
		}																/*											  */
	}																	/*											  */
	return(ret);														/*											  */
}																		/*											  */

