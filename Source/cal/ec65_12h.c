/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：日毎最大料金処理　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：ec65_12h(par1)　　　　　　：日毎最大料金処理　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：１２時間最大（ＧＷ４７１８）を標準化　　　　　　　　　　　　　：　*/
/*　コンパイラ　　　　：ＭＳ－Ｃ Ｖｅｒ ６．０Ａ　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　ターゲットＣＰＵ　：Ｖ２５　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　ＯＳ　　　　　　　：ＡＭ―Ｘ０　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　対応機種　　　　　：ＦＣ　　　　　　　　　　　：　計算対象分　　　　　　　　　リタンコード　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　修正者（日付）　　：中山 (04/07/16) 　　　　　：・GW4718を標準化。ec65()ﾍﾞｰｽ。                        　　　　：　*/
/**********************************************************************************************************************/
																		/**********************************************/
#include	"system.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"mem_def.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"rkn_def.h"													/*　全デファイン統括　　　　　　　　　　　　　*/
#include	"rkn_cal.h"													/*　料金関連データ　　　　　　　　　　　　　　*/
#include	"rkn_fun.h"													/*　全サブルーチン宣言　　　　　　　　　　　　*/
#include	"Tbl_rkn.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"prm_tbl.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	ec65_12h( short, short );										/*　日毎最大料金処理　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
extern char	h24_mid ;													/*　　　　　　　　　　　　　　　　　　　　　　*/
extern char	teig_mid ;													/* 逓減帯24H最大ﾌﾗｸﾞ,昼夜帯には影響無し		  */
extern char	teig_cycl;													/* 逓減帯24H最大ﾌﾗｸﾞ,昼夜帯には影響無し		  */
extern char	tyu24_flg;													/* 昼夜帯､一定料金帯での24H最大ﾌﾗｸﾞ　　　 　　*/
extern char	max24_fg ;													/* 一定料金帯料金の取り方設定が１の時に使う	　*/
extern char	h24_flg ;													/*　　　　　　　　　　　　　　　　　　　　　　*/

#define		T_12HOUR	720												/*　１２時間分の分　　　　　　　　　　　　　　*/
int			h12_cnt = 0 ;												/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	ec65_12h( 														/*　日毎最大料金処理　　　　　　　　　　　　　*/
short	pa_len	,														/*　計算対象分　　　　　　　　　　　　　　　　*/
short	pa_strt	)														/*　料金帯開始時刻　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	int		wk_cal_time;												/*　　　　　　　　　　　　　　　　　　　　　　*/
	char	teigen24 = 0;												/*											  */
	int		itei_end = 0;												/* 一定料金帯の重複帯終了時刻				　*/
	char	mid24_00 = 0;												/*											  */
	int		wk_tgend = 0;												/* 逓減帯重複帯終了時刻						　*/
	int		in_add24 = 0;												/* 入庫時刻+24h								　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	wk_cal_time = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	h24_flg	= 0;														/* 昼夜帯24H加算ﾌﾗｸﾞ　　　　　　　　　　　　　*/
	teig_mid = 0;														/* 深夜帯料金加算ﾌﾗｸﾞ　　　　　　　　　　　　 */
	teig_cycl = 0;														/* 深夜帯ｼﾞｬﾝﾌﾟ後のﾊﾟﾀｰﾝ更新ﾌﾗｸﾞ　　　　　　　*/
																		/*											  */
	switch( sp_mx )														/*　最大料金取得方法により分類　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		case 1:															/*　２４時間最大の場合　　　　　　　　　　　　*/
			if(( h12_cnt )&&(CPrmSS[S_CAL][14]))						/*　駐車時間が１２時間を超えた場合　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				chk_max1 = 99900 ;										/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ryoukin >= chk_max1 )									/*　駐車料金が２４時間最大料金を超えた場合　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				ryoukin = chk_max1;										/*　２４時間最大料金を駐車料金とする　　　　　*/
				rmax_cnt++;												/*　最大到達カウントＵＰ　　　　　　　　　　　*/
				if(( rmax_cnt == 1 )&&( cal_type == 1 ))				/*　最大になった時のパタンセーブ（逓減）　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					ec65_1(0);											/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( ptn_cycl[style-1][strt_ptn-1] == T_DAY )		/*　　　　　　　　　　　　　　　　　　　　　　*/
						rmax_pcn = 0;									/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( cycl_cnt[style-1] == T_DAY )					/*　　　　　　　　　　　　　　　　　　　　　　*/
						rmax_cyc = 0;									/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　パタン繰り返し回数　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			cal_time  = cal_time + pa_len;								/*　計算時刻を追加回数×追加時間進める　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			wk_cal_time = cal_time;										/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(( iti_flg == ON )&&( sp_it == 1 ))						/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				wk_cal_time = cal_time + amartim;						/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if((CPrmSS[S_CAL][1] != 1)&&(rmax_cnt == 1)&&				/*　昼夜帯なら　　　　　　　　　　　　　　　　*/
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
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( (h24time + (wk_cal_time - wk_h24t)) >= T_12HOUR )		/*　計算時刻が入庫後２４時間経過　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				h12_cnt ++ ;											/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(( h12_cnt % 2 == 0 )||( h24time ))					/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					h24time = 0;										/*　現在までの計算時間トータルクリア　　　　　*/
					wk_h24t = cal_time;									/*　現在までの計算時間　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					wk_h24t = h24time + wk_h24t + T_12HOUR ;			/*　１２時間までの計算時間　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
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
						if( h12_cnt % 2 == 0 )							/*　　　　　　　　　　　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							in_add24 = in_time + T_DAY;					/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						else											/*　　　　　　　　　　　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							in_add24 = in_time + T_12HOUR 	;			/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
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
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
							if(((cal_time - pa_len) < T_DAY)&&			/*　1つ前の計算時刻が0:00前で今回の計算時刻が */
								(wk_cal_time >= T_DAY))					/*　0:00分を越えた　　　　　　　　　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								cal_time = in_add24;					/*　計算時刻をＩＮ＋２４とする　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
							else										/*　　　　　　　　　　　　　　　　　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								cal_time = in_add24;					/*　計算時刻をＩＮ＋２４とする　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					else												/*　　　　　　　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( h12_cnt % 2 == 0 )							/*　　　　　　　　　　　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							cal_time = in_time;							/*　計算時刻をＩＮ＋２４とする　　　　　　　　*/
						}else{											/*　　　　　　　　　　　　　　　　　　　　　　*/
							if( in_time > ot_time )						/*　　　　　　　　　　　　　　　　　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
								cal_time = in_time - T_12HOUR	;		/*　　　　　　　　　　　　　　　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( in_time == 0 )									/*　入庫が0時00分だった時のIN+24なら　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( h12_cnt % 2 == 0 )							/*　　　　　　　　　　　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							cal_time = T_DAY;							/*　計算時刻を２４時間とする　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						else											/*　　　　　　　　　　　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							cal_time = T_12HOUR;						/*　計算時刻を２４時間とする　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
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
						( wk_cal_time > next_style_strt ))				/*　出庫時刻が次料金帯　　　　　　　　　　　　*/
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
				if(( cal_time < bnd_strt[style-1][4-1] )||				/*　計算時刻が第４料金帯開始時刻以下／　　　　*/
				   ((cal_time == bnd_strt[style-1][4-1])&&(pa_len == 0))||	/*　第４料金帯開始時かつ一定料金／　　　　*/
				   ((cal_time >= bnd_strt[style-1][4-1])&&				/*  計算時刻が第４料金帯重複帯内で重複０で出庫*/
				    (cal_time <= bnd_strt[style-1][4-1] + ovl_time[style-1][4-1] )&&/*　か重複帯内入出庫ならば　　　　*/
				    ((( et_dbl != OFF )||( et_jiot == NOF ))&&(band < 4))))/*　　　　　　　　　　　　　　　　　　　　 */
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
					if((( cal_time >= bnd_strt[style-1][4-1] )&&		/*　計算時刻が第４料金帯開始時刻に達し，かつ　*/
					    ( et_dbl != ON ))||								/*　　　　　　　　　　　　　　　　　　　　　　*/
				 	   ((cal_time > bnd_strt[style-1][4-1] + wk_strt)&&	/*　　　　　　　　　　　　　　　　		      */
				 		( et_dbl == ON )))								/*　　　　　　　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						total = ryoukin + total;						/*　料金を集計中の駐車料金に加算　　　　　　　*/
						ryoukin = 0;									/*　料金を０クリア　　　　　　　　　　　　　　*/
						if( rmax_cnt > 1 )								/*　今の追加時間で最大となった訳でない！　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							if((et_dbl == ON)||(ovl_type[style-1][4-1] == 1)||	/*　　　　　　　　　　　　　　　　　　*/
							  ((et_jiot == NOF)&&(ovl_type[style-1][4-1] == 0)))/*　　　　　　　　　　　　　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								cal_time = bnd_strt[style-1][4-1]+wk_strt;	/*　計算時刻を第４料金帯開始時刻とする　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
							else										/*　　　　　　　　　　　　　　　　　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								cal_time = bnd_strt[style-1][4-1];		/*　計算時刻を第４料金帯開始時刻とする　　　　*/
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
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		default:														/*　日毎最大料金タイプが０～３以外の場合　　　*/
		break;															/*　条件を満たさない場合０を返す　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
