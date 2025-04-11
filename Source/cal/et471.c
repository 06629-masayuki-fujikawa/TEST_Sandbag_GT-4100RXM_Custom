/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：時間帯定期券処理　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et471(par1,par2)　　　　　：　グレースタイムの処理を行い、定期帯を除いた時間の料金計算を行：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：う。又設定により定期終了時刻（更新入庫時刻）の更新、一定料金帯：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：の処理を行う。　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　定期種別　　　　　　　　　　無し　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　料金の取りかた　　　　　　　　　　　　　　　　　　　　　　　：　*/
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
static	void	et471_nmax( char,short );								// ｎ時間最大計算用Sub関数
																		// 
void	et471( 															/*　　　　　　　　　　　　　　　　　　　　　　*/
				short	t_kind,											/*　定期券種別　　　　　　　　　　　　　　　　*/
				short	pat												/*　料金の取りかた　　　　　　　　　　　　　　*/
			 )															/*											  */
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	struct	CAR_TIM		tki_st;											/*　時刻構造体　定期開始時刻　　　　　　　　　*/
	struct	CAR_TIM		tki_ed;											/*　時刻構造体　定期終了時刻　　　　　　　　　*/
	struct	CAR_TIM		tki_st_m;										/*　時刻構造体　定期開始時刻　　　　　　　　　*/
	struct	CAR_TIM		tki_ed_m;										/*　時刻構造体　定期終了時刻　　　　　　　　　*/
	struct	CAR_TIM		ren_in;											/*　時刻構造体　更新時刻　　　　　　　　　　　*/
	struct	CAR_TIM		wok_tm;											/*　時刻構造体　ｗｏｒｋ時刻　　　　　　　　　*/
	struct	CAR_TIM		bas_ot;											/*　時刻構造体　グレースタイム処理後出庫時刻　*/
	struct	CAR_TIM		wok;											/*　時刻構造体　ｗｏｒｋ時刻　　　　　　　　　*/
	struct	CAR_TIM		j_wok;											/*　時刻構造体　ｗｏｒｋ時刻　　　　　　　　　*/
	short	taik = 0;													/*　体系　　　　　　　　　　　　　　　　　　　*/
	short	ret = 0;													/*　リタンコード　　　　　　　　　　　　　　　*/
	short	loflg = 1;													/*　ループｆｌｇ　　　　　　　　　　　　　　　*/
	char	okng = NG;													/*　定期有効無効ｆｌｇ　　　　　　　　　　　　*/
	char	kou = NG;													/*　更新済みｆｌｇ　　　　　　　　　　　　　　*/
	long	ryo = 0;													/*　料金　　　　　　　　　　　　　　　　　　　*/
	char	ryoprc = NG;												/*　料金計算要求ｆｌｇ　　　　　　　　　　　　*/
	char	car;														/*　車種　　　　　　　　　　　　　　　　　　　*/
	short	ichi = 0;													/*　位置　　　　　　　　　　　　　　　　　　　*/
	char	st_flg = ON;												/*　初回料金計算ｆｌｇＯＮ　　　　　　　　　　*/
	short	jta_st = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
	int  n_max_set;														/*　ｎ時間最大ｍ回の適用回数設定　　　　　　　*/
	int  n_hour_wk;														/*　ｎ時間計算のワーク　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	struct	CAR_TIM		w_tki_tm;										// 時刻構造体 ｗｏｒｋ時刻
	struct	CAR_TIM		w_ren_in;										// 時刻構造体 更新時刻/
	short	w_ren_tm;													// 定期帯更新用ﾜｰｸ構造体
	char	ps_flg;														// 前回料金算出時の定期帯保存ﾌﾗｸﾞ
	char	FeeMaxType;													// 最大料金タイプ
	short	bnd_st;														// 料金帯開始時刻ワーク
	short	wk_st;														// 定期帯開始時刻ワーク
	short	wk_ed;														// 定期帯終了時刻ワーク
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	memcpy( &bas_ot,&car_ot,7 );										/*　基本出庫時刻をＧ出庫時刻とする　　　　　　*/
	memcpy( &ren_in,&car_in,7 );										/*　計算入庫時刻を更新時刻とする　　　　　　　*/
	memset( (char *)&tki_st,0,7 );										/*　定期開始時刻クリア　　　　　　　　　　　　*/
	memset( (char *)&tki_ed,0,7 );										/*　定期終了時刻クリア　　　　　　　　　　　　*/
	memset( (char *)&tki_st_m,0,7 );									/*　定期開始時刻クリア　　　　　　　　　　　　*/
	memset( (char *)&tki_ed_m,0,7 );									/*　定期終了時刻クリア　　　　　　　　　　　　*/
	memset( (char *)&j_wok,0,7 );										/*　定期終了時刻クリア　　　　　　　　　　　　*/
	memcpy( &wok,&car_in,7 );											/*　計算入庫時刻を更新時刻とする　　　　　　　*/
	ryoukin	 = 0;														/*　料金＝０　　　　　　　　　　　　　　　　　*/
	total	 = 0;														/*　　　　　　　　　　　　　　　　　　　　　　*/
	tt_tim	 = 0;														/*　定期帯時間　　　　　　　　　　　　　　　　*/
	tt_ryo	 = 0;														/*　最大料金用料金　　　　　　　　　　　　　　*/
	tt_x	 = 0;														/*　時刻指定用ｔｏｔａｌ　ｗｏｒｋ　　　　　　*/
	tt_y	 = 0;														/*　時刻指定用ｒｙｏｕｋｉｎ　ｗｏｒｋ　　　　*/
	jmax_ryo = 0;														/*　　　　　　　　　　　　　　　　　　　　　　*/
	amr_flg = 0;														// 
	in_calcnt	= 0;													// 
	nmax_cnt	= 0;													// Ｎ時間最大徴収カウント
	ps_flg		= 0;													// 前回料金算出時の定期帯保存ﾌﾗｸﾞ
	nmax_amari	= 0;													// 
	n_itei_adj	= 0;													// 一定料金後基準時刻調整
	ntnet_nmax_flg = 0;													// 
	nmax_amari2 = 0;													// ｎ2の余り時間
	FeeMaxType = sp_mx;													// 最大料金タイプをセット（全種別共通） 
	if( EXP_FeeCal == 1 ){												// 料金種別毎最大料金選択あり
		FeeMaxType = sp_mx_exp[car_type-1];								// 最大料金タイプをセット（料金種別毎）
	}																	//
																		// 
	kou = et4711( t_kind,&ren_in );										/*　初回更新入庫時刻算出　　　　　　　　　　　*/
	if( kou == OK )														/*　更新された場合　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		memcpy( &car_in,&ren_in,7 );									/*　計算入庫時刻に更新時刻のセットをする　　　*/
		memcpy( &nmax_orgin,&car_in,7 );								// 
		pat = (short)Carkind_Param(WARI_PAY_PTN, car_type, 2,1);		/*　料金の取りかたの参照　　　　　　　　*/
		ret = ec64( &bas_ot,&ren_in );									/*　出庫時刻と計算入庫時刻の比較　　　　　　　*/
		if( ret >= 0 )													/*　出庫時刻＜計算入庫時刻の場合　　　　　　　*/
			loflg = 0;													/*　ループｆｌｇクリア　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	memcpy( &ren_in,&car_in,7 );										/*　計算入庫時刻を更新時刻にセットをする　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	while( loflg == 1 )													/*　ループｆｌｇが１の場合処理を行う　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( st_flg != ON )												/*　２回目以降料金計算なら　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ps_flg == 1 ){											// 前回料金算出後のとき
				memcpy( &tki_st_m,&tki_st,7 );							// 前の定期開始時刻をセーブ
				memcpy( &tki_ed_m,&tki_ed,7 );							// 前の定期終了時刻をセーブ
			}															// 
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		memcpy ( &tki_st,&ren_in,7 );									/*　計算入庫時刻に更新時刻のセットをする　　　*/
		okng = ec67( t_kind,&tki_st,&tki_ed,0 );						// 更新時刻の定期帯を求める
		if( okng == OK )												/*　定期有効の場合　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			ichi = ec72( &car_in,&tki_st,&tki_ed );						/*　定期帯と計算入庫時刻チェック　　　　　　　*/
			switch( ichi )												/*　計算入庫時刻の位置により分岐　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				case 0:													/*　計算入庫時刻＜定期開始時刻の場合　　　　　*/
					ret = ec64( &bas_ot,&tki_st );						/*　出庫時刻と定期開始時刻の比較　　　　　　　*/
					if ( ret >= 0 )										/*　出庫時刻≦定期開始時刻の場合　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						loflg = 0;										/*　ループｆｌｇクリア　　　　　　　　　　　　*/
						memcpy( &car_ot,&bas_ot,7 );					/*　出庫時刻を計算出庫時刻とする　　　　　　　*/
						ryoprc = OK;									/*　料金計算処理要求　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					else												/*　出庫時刻＞定期開始時刻の場合　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						ret = ec64( &bas_ot,&tki_ed );					/*　出庫時刻と定期終了時刻の比較　　　　　　　*/
						if( ret >= 0 ){									/*　出庫時刻≦定期終了時刻の場合　　　　　　　*/
							loflg = 0;									/*　ループｆｌｇクリア　　　　　　　　　　　　*/
						}else{											/*　出庫時刻＞定期終了時刻の場合　　　　　　　*/
							if(	pass_week_chg == PASS_WEEK_DAY_SP ){	// 定期帯を0:00で区切る
								memcpy( &w_ren_in,&ren_in,7 );			// 
								taik = ec61( &w_ren_in );				// その日の体系を求める
								if(ps_sepa[taik-1][t_kind-1] == 1){		// 定期帯が0:00で分割
									memcpy( &ren_in, &tki_ed, 7 );		// 
								}else{									// 
									ec63( &ren_in );					// 更新時刻を１日繰り上げる
								}										// 
							}else{										// 
								ec63( &ren_in );						// 更新時刻を１日繰り上げる
							}											// 
						}												// 
						memcpy( &car_ot,&tki_st,7 );					/*　出庫時刻を定期開始時刻とする　　　　　　　*/
						ryoprc = OK;									/*　料金計算処理要求　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				break;													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				case 1:													/*　定期帯内に計算入庫時刻がある場合　　　　　*/
					ret = ec64( &bas_ot,&tki_ed );						/*　出庫時刻と定期終了時刻の比較　　　　　　　*/
					if( ret >= 0 ){										/*　出庫時刻≦定期終了時刻の場合　　　　　　　*/
						loflg = 0;										/*　ループｆｌｇクリア　　　　　　　　　　　　*/
					}else{												/*　出庫時刻＞定期終了時刻の場合　　　　　　　*/

						memcpy( &car_in,&tki_ed,7 );					/*　定期終了時刻を計算入庫時刻とする　　　　　*/
						memcpy( &nmax_orgin,&tki_ed,7 );				// 
						if(	pass_week_chg == PASS_WEEK_DAY_SP ){		// 定期帯を0:00で区切る
							memcpy( &w_ren_in,&ren_in,7 );				// 
							taik = ec61( &w_ren_in );					// その日の体系を求める
							if(ps_sepa[taik-1][t_kind-1] == 1){			// 定期帯が0:00で分割
								w_ren_tm = w_ren_in.hour*60+w_ren_in.min;		// 
								if( w_ren_tm < ps_ed2[taik-1][t_kind-1] ){		// 
									ren_in.hour = 								// 
										(char)(ps_st1[taik-1][t_kind-1]/60);	// 
									ren_in.min  = 								// 
										(char)(ps_st1[taik-1][t_kind-1]%60);	// 
								}else{											// 
									ren_in.hour = 								// 
										(char)(ps_ed1[taik-1][t_kind-1]/60);	// 
									ren_in.min  = 								// 
										(char)(ps_ed1[taik-1][t_kind-1]%60);	// 
									ec63( &ren_in );					// 更新時刻を１日繰り上げる
								}										// 
							}else{										// 
								ec63( &ren_in );						// 更新時刻を１日繰り上げる
							}											// 
						}else{											// 
							ec63( &ren_in );							// 更新時刻を１日繰り上げる
						}												// 
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					pat = (short)Carkind_Param(WARI_PAY_PTN, car_type, 2,1);		/*　料金の取りかたの参照（２回目以降）　　*/
				break;													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				case 2:													/*　定期終了時刻より後に計算入庫時刻がある場合*/
					if(	pass_week_chg == PASS_WEEK_DAY_SP ){			// 定期帯を0:00で区切る
						memcpy( &w_ren_in,&ren_in,7 );					// 
						w_ren_in.hour = 0;								// 
						w_ren_in.min  = 0;								// 
						taik = ec61( &w_ren_in );						// その日の体系を求める
						if(ps_sepa[taik-1][t_kind-1] == 1){				// 定期帯が0:00で分割
							ren_in.hour = 0;							// 
							ren_in.min = 0;								// 
						}												// 
						ec63( &ren_in );								// 更新時刻を１日繰り上げる
					}else{												// 
						ec63( &ren_in );								// 更新時刻を１日繰り上げる
					}													// 
				break;													/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　定期無効の場合　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			ret = ec64( &bas_ot,&ren_in );								/*　出庫時刻と更新時刻の比較　　　　　　　　　*/
			if( ret >= 0 )												/*　出庫時刻≦更新時刻の場合　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				loflg = 0;												/*　ループｆｌｇクリア　　　　　　　　　　　　*/
				memcpy( &car_ot,&bas_ot,7 );							/*　出庫時刻を計算出庫時刻とする　　　　　　　*/
				ryoprc = OK;											/*　料金計算処理要求　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　出庫時刻＞更新時刻の場合　　　　　　　　　*/
			{															// 
				if(	pass_week_chg == PASS_WEEK_DAY_SP ){				// 定期帯を0:00で区切る
					memcpy( &w_ren_in,&ren_in,7 );						// 
					taik = ec61( &w_ren_in );							// その日の体系を求める
					if(ps_sepa[taik-1][t_kind-1] == 1){					// 定期帯が0:00で分割
						ren_in.hour = (char)(ps_ed1[taik-1][t_kind-1]/60); 
						ren_in.min  = (char)(ps_ed1[taik-1][t_kind-1]%60); 
					}													// 
					ec63( &ren_in );									// 更新時刻を１日繰り上げる
				}else{													// 
					ec63( &ren_in );									// 更新時刻を１日繰り上げる
				}														// 
			}															// 
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		ps_flg = 0;														// 前回定期帯保持要求ﾌﾗｸﾞｸﾘｱ
		if( ryoprc == OK )												/*　料金計算処理要求がある場合　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			ps_flg = 1;													// 前回定期帯保持要求ﾌﾗｸﾞｾｯﾄ
			car = req_rkn.syubt;										/*　車種の保存　　　　　　　　　　　　　　　　*/
			req_rkn.syubt = rysyasu;									/*　車種セット　　　　　　　　　　　　　　　　*/
			if( amr_flg != OFF )										/*　余りフラグがＯＦＦではない場合　　　　　　*/
				amr_flg = T_TX_TIM;										/*　余りフラグに定期後加算時間有りをセットする*/
			iti_flg = OFF;												/*　一定料金帯フラグＯＦＦ　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( (FeeMaxType == SP_MX_N_MH_NEW)||						// ｎ時間最大の時
				(FeeMaxType == SP_MX_N_MHOUR_W) ){						// 2種類のｎ時間最大の時
				et471_nmax( st_flg, t_kind );							// amr_flgの後に挿入
			}															// 
																		// 
			if( st_flg == ON )											/*　初回料金計算なら　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				nmax_style = 0;											// ｎ時間最大の最大料金を取る料金体系
																		// 
				if( (FeeMaxType != SP_MX_N_MH_NEW)&&					// ｎ時間最大
					(FeeMaxType != SP_MX_N_MHOUR_W) ){					// 2種類のｎ時間最大以外
																		// 
				tt_tim = (short)ec71( &wok,&car_in );					/*　定期帯時間算出　　　　　　　　　　　　　　*/
				in_time = (short)wok.hour * 60 + (short)wok.min;		/*　入庫時刻を分で表現　　　　　　　　　　　　*/
				wk_h24t = in_time;										/*　入庫　　　　　　　　　　　　　　　　　　　*/
				h24time = tt_tim;										/*　定期有効から始まった場合に処理済時間とする*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				n_hour_style = 0;										/*　ｎ時間最大の最大料金を取る料金体系　　　　*/
				n_hour_cnt = 0;											/*　ｎ時間最大ｍ回の適用回数　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( FeeMaxType == SP_MX_N_HOUR ){						/*　ｎ時間最大　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					in_time = (int)car_in.hour * 60 + (int)car_in.min;	/*　次回入庫時刻　　　　　　　　　　　　　　　*/
					wk_h24t = in_time;									/*　入庫　　　　　　　　　　　　　　　　　　　*/
					/*--------------------------------------------------------*//*　　　　　　　　　　　　　　　　　　　　　　*/
					/* ｎ時間経過後、ｍ回判定                                 *//*　　　　　　　　　　　　　　　　　　　　　　*/
					/*--------------------------------------------------------*//*　　　　　　　　　　　　　　　　　　　　　　*/
																				/*　入庫時刻と定期終了後の入庫時刻が異なる場合*/
																				/*　入庫時刻から定期終了後までのｎ時間を取得　*/
					n_hour_wk = tt_tim;											/*　経過時間を算出　　　　　　　　　　　　　　*/
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
																				/*　計算済み時間算出　　　　　　　　　　　　　*/
																				/*　ｎ時間の端数を計算済みとしてセット　　　　*/
																				/*　ｎ時間の倍数の場合０をセット　　　　　　　*/
						h24time = n_hour_wk % T_N_HOUR;							/*　　　　　　　　　　　　　　　　　　　　　　*/
					}															/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/


				}														// 
				tt_ryo = 0;												/*　最大料金用料金　　　　　　　　　　　　　　*/
				ryoukin = 0;											/*　駐車料金集計用変数クリア　　　　　　　　　*/
				chk_time = 0;											/*　　　　　　　　　　　　　　　　　　　　　　*/
				chk_tim.min = 0;										/*　　　　　　　　　　　　　　　　　　　　　　*/
				chk_tim.hour = 0;										/*　　　　　　　　　　　　　　　　　　　　　　*/
				tt_x = 0;												/*　　　　　　　　　　　　　　　　　　　　　　*/
				tt_y = 0;												/*　　　　　　　　　　　　　　　　　　　　　　*/
				st_flg = OFF;											/*　初回料金計算ｆｌｇＯＦＦ　　　　　　　　　*/
				memcpy( &wok,&car_ot,7 );								/*　定期帯時間算出の為データセーブ　　　　　　*/
				jmax_ryo = 0;											// 
																		// 
				if( (FeeMaxType == SP_MX_INTIME)||						// 
					(FeeMaxType == SP_MX_N_HOUR)||						// 
					(FeeMaxType == SP_MX_BAND)||						// 
					(FeeMaxType >= SP_MX_612HOUR) )						// 
					total = 0;											/*　駐車料金集計用変数クリア　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　２回目以降なら　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				tt_tim = (short)ec71( &wok,&car_in );					/*　定期帯時間算出　　　　　　　　　　　　　　*/

				if( FeeMaxType == SP_MX_N_HOUR ){						/*　ｎ時間最大　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					n_hour_wk = h24time + tt_tim ;						/*　基点からの計算済み時間＋次回入庫時刻　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( n_hour_wk >= T_N_HOUR ) 						/*　ｎ時間経過している?(Y)　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						h24time = n_hour_wk % T_N_HOUR;					/*　ｎ時間最大の基点からの経過時間更新　　　　*/
						wk_h24t = (int)car_in.hour * 60 + (int)car_in.min;/*　次回料金計算開始時刻　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						n_max_set=(short)CPrmSS[S_CAL][14];				/*　ｍ回適用　０は∞適用とする　　　　　　　　*/
					 													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(( n_max_set != 0 )&&( n_hour_cnt < n_max_set)){/*　∞以外かつｍ回カウント中?(Y)　　　　　　*/
					 													/*　　　　　　　　　　　　　　　　　　　　　　*/
							n_hour_cnt += ( n_hour_wk / T_N_HOUR);		/*　ｎ時間最大ｍ回の適用回数　　　　　　　　　*/
																		/*　一定時間を考慮するとT_N_HOURの倍数を　　　*/
																		/*　カウントする必要がある　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						total = ryoukin + total;						/*　料金を集計中の駐車料金に加算　　　　　　　*/
						ryoukin = 0;									/*　料金を０クリア　　　　　　　　　　　　　　*/
			/*--------------------------------------------------------*//*　　　　　　　　　　　　　　　　　　　　　　*/
			/* ｎ時間最大判定、ｍ回適用判定					          *//*　　　　　　　　　　　　　　　　　　　　　　*/
			/*--------------------------------------------------------*//*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						n_max_set=(short)CPrmSS[S_CAL][14];				/*　ｍ回適用　０は∞適用とする　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						if(( n_hour_cnt >= n_max_set )&&( n_max_set != 0 )){/*　ｍ回終了かつ∞でない?(Y)　　　　　　　　　*/
							chk_max1 = N_MAX_OVER;						/*　ｎ時間最大ｍ回経過後の最大料金　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						}else{											/*　ｍ回継続中または∞?(Y)　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
							if( cal_time == next_style_strt ){			/*　次の料金体系の開始時間とちょうど同じ?(Y)　*/
								n_hour_style = next_style;				/*　ｎ時間最大の最大料金を取る料金体系　　　　*/
							}else{										/*　　　　　　　　　　　　　　　　　　　　　　*/
								n_hour_style = style;					/*　ｎ時間最大の最大料金を取る料金体系　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
							chk_max1 = day_max1[n_hour_style-1][car_type-1];/*　ｎ時間最大料金　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						rmax_cnt = 0;									/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			/*--------------------------------------------------------*//*　　　　　　　　　　　　　　　　　　　　　　*/
			/* 時間帯上限料金（料金帯上限料金）				          *//*　　　　　　　　　　　　　　　　　　　　　　*/
			/*--------------------------------------------------------*//*　　　　　　　　　　　　　　　　　　　　　　*/
				if( FeeMaxType == SP_MX_BAND ){							/*　時間帯上限料金?(Y)　　　　　　　　　　　　*/
					wk_st = tki_st_m.hour * 60 + tki_st_m.min;			// 定期帯開始時刻
					wk_ed = tki_ed_m.hour * 60 + tki_ed_m.min;			// 定期帯終了時刻
					if( band != last_bnd ){								// 
						bnd_st = bnd_strt[style-1][band];				// 
					}else{												// 
						bnd_st = next_style_strt%1440;					// 
					}													// 
					if( wk_st > wk_ed ){								// 定期帯が日を跨ぐ
						if( (bnd_st >= wk_st)||(bnd_st <= wk_ed) ){		// 定期帯内に料金帯切換有り
							total = ryoukin + total;					// 
							ryoukin = 0;								// 
						}												// 
					}else{												// 定期帯が日を跨がない
						if( (bnd_st >= wk_st)&&(bnd_st <= wk_ed) ){		// 定期帯内に料金帯切換有り
							total = ryoukin + total;					// 
							ryoukin = 0;								// 
						}												// 
					}													// 
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( (FeeMaxType == SP_MX_N_MH_NEW)||					// ｎ時間最大
					(FeeMaxType == SP_MX_N_MHOUR_W) ){					// 2種類のｎ時間最大
					if( nmax_point == NMAX_OUT_PIT ){ 					// ｎ時間最大が出庫基準の時
						chk_max1 = 0;									// 
						chk_max2 = 0;									// 
					}													// 
					if( chk_max1 == 0 ){								// 
						if( cal_time >= next_style_strt ){				// 
							chk_max1 = day_max1[next_style-1][car_type-1];	// ｎ時間最大料金を次の料金体系最大料金とする
						}else{											// 
							chk_max1 = day_max1[style-1][car_type-1];	// ｎ時間最大料金を今の料金体系最大料金とする
						}												// 
					}													// 
					if( chk_max2 == 0 ){								// 
						if( cal_time >= next_style_strt ){				// 
							chk_max2 = day_max2[next_style-1][car_type-1];	// ｎ時間最大料金を次の料金体系最大料金とする
						}else{											// 
							chk_max2 = day_max2[style-1][car_type-1];	// ｎ時間最大料金を今の料金体系最大料金とする
						}												// 
					}													// 
				}														// 
																		// 
				memcpy( &wok,&car_ot,7 );								/*　定期帯時間算出の為データセーブ　　　　　　*/

																		// ◆定期帯内に第1料金帯切換時刻が含まれている？
				if( (FeeMaxType == SP_MX_TIME)||						// 
					(FeeMaxType == SP_MX_WTIME) )						// 時刻指定／２種の最大料金の場合
				{														// 
					memcpy( &j_wok,&tki_st_m,7 );						// 体系開始時刻算出の為
					taik = ec61( &j_wok );								// その日の体系を求める
					jta_st = ta_st_sel( (char)taik, 0 );				// 第1料金帯(体系)開始時刻を求める
					j_wok.hour = (char)( jta_st / 60 );					// 
					j_wok.min = (char)( jta_st % 60 );					// 
					ret = ec64( &tki_st_m,&j_wok );						// 定期開始時刻と体系開始時刻の比較
					if(( ret == 0 )||( ret == 1 ))						// 定期開始時刻≦体系開始時刻の場合
					{													// 
						ret = ec64( &j_wok,&car_in );					// 料金計算開始時刻と体系開始時刻の比較
						if(( ret == 0 )||( ret == 1 ))					// 体系開始時刻≦料金計算開始時刻の場合
						{												// 
							jmax_ryo = jmax_ryo + ryoukin;				// 料金のセーブ
							ryoukin = 0;								// 
						}												// 
					}													// 
					else{												// 
						ec63( &j_wok );									// １日繰上げ
						ret = ec64( &tki_st_m,&j_wok );					// 
						if(( ret == 0 )||( ret == 1 ))					// 体系開始時刻≦料金計算開始時刻の場合
						{												// 
							ret = ec64( &j_wok,&car_in );				// 料金計算開始時刻と体系開始時刻の比較
							if(( ret == 0 )||( ret == 1 ))				// 体系開始時刻≦料金計算開始時刻の場合
							{											// 
								jmax_ryo = jmax_ryo + ryoukin;			// 料金のセーブ
								ryoukin = 0;							// 
							}											// 
						}												// 
					}													// 
				}														// 
																		// ◆定期帯内に第4料金帯切換時刻が含まれている？
				if( FeeMaxType == SP_MX_WTIME )							// ２種の最大料金の場合
				{														// 
					memcpy( &j_wok,&tki_st_m,7 );						// 体系開始時刻算出の為
					jta_st = bnd_strt[taik-1][4-1]%1440;				// 第4料金帯開始時刻[体系][帯]算出
					j_wok.hour = (char)( jta_st / 60 );					// 
					j_wok.min = (char)( jta_st % 60 );					// 
					ret = ec64( &tki_st_m,&j_wok );						// 定期開始時刻と料金帯開始時刻の比較
					if(( ret == 0 )||( ret == 1 ))						// 定期開始時刻≦料金帯開始時刻の場合
					{													// 
						ret = ec64( &j_wok,&car_in );					// 料金計算開始時刻と料金帯開始時刻の比較
						if(( ret == 0 )||( ret == 1 ))					// 料金帯開始時刻≦料金計算開始時刻の場合
						{												// 
							jmax_ryo = jmax_ryo + ryoukin;				// 料金のセーブ
							ryoukin = 0;								// 
						}												// 
					}													// 
				}														// 
																		// 
				if( cal_type == 0 )										/*　料金計算方式　０：昼夜帯方式　　　　　　　*/
					pat = 1;											/*　料金の取りかたのは追加からとる　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			et4712( pat );												/*　駐車料金額算出　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			tt_ryo = ryoukin;											/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			ryoprc = NG;												/*　料金計算処理要求クリア　　　　　　　　　　*/
			req_rkn.syubt = car;										/*　車種を戻す　　　　　　　　　　　　　　　　*/
			pat = (short)Carkind_Param(WARI_PAY_PTN, car_type, 2,1);	/*　料金の取りかたの参照（２回目以降）　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( loflg != 0 )											/*　ループｆｌｇがクリアされていない場合　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( sp_it == 1 )										/*　終了時刻に加算する場合　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					memcpy( &wok_tm,&cal_tm,7 );						/*　料金計算後時刻をｗｏｒｋ時刻にセット　　　*/
					if(( amr_flg == ON )&&( amartim > 0 ))				/*　余りｆｌｇがＯＮかつ余り時間＞０の場合　　*/
						ec66( &wok_tm,(long)amartim );					// work時刻を余り分減算する
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					ret = ec64( &bas_ot,&wok_tm );						/*　出庫時刻と計算終了後時刻の比較　　　　　　*/
					if( ret >= 0 )										/*　出庫時刻≦料金計算後時刻の場合　　　　　　*/
						loflg = 0;										/*　ループｆｌｇクリア　　　　　　　　　　　　*/
					else												/*　出庫時刻＞料金計算後時刻の場合　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						ret = ec64( &wok_tm,&tki_ed );					/*　料金計算後時刻と定期終了時刻の比較　　　　*/
						if( ret >= 0 )									/*　料金計算後時刻≦定期終了時刻の場合　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							memcpy( &car_in,&tki_ed,7 );				/*　定期終了時刻を計算入庫時刻とする　　　　　*/
							memcpy( &nmax_orgin,&tki_ed,7 );			// 
							if( (pass_week_chg == PASS_WEEK_DAY_SP)&&	// 定期帯を0:00で区切る
								((tki_ed.hour < tki_st.hour)||			// 全日定期以外で日を跨ぐ場合
								 ((tki_ed.hour == tki_st.hour)&&		// 
								  (tki_st.hour != 0))) ){				// 全日定期で日を跨ぐ場合 
																		// 
								memcpy( &w_tki_tm,&car_in,7 );			// 
								w_tki_tm.hour = 0;						// 
								w_tki_tm.min  = 0;						// 
								okng = ec69( &w_tki_tm,t_kind );		// 
								if( okng == (char)NG ){					// 無効の時
									car_in.hour = 0;					// 
									car_in.min  = 0;					// 
									if( (FeeMaxType == SP_MX_N_MH_NEW)||	// ｎ時間最大
										(FeeMaxType == SP_MX_N_MHOUR_W) ){	// 2種類のｎ時間最大
										et471_nmax( st_flg, t_kind );	// 
									}									// 
								}										// 
							}											// 
							if(( amr_flg == OFF )&&( iti_flg == OFF ))	/*　余りフラグＯＦＦかつ一定料金帯フラグＯＦＦ*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								if( (FeeMaxType != SP_MX_612HOUR)||		// 
									(h24time > 0) ){					// Ｎ時間最大でＮ時間到達直後は
									amartim = (short)ec71( &tki_st,&cal_tm );/*　余り時間＝料金計算後時刻－定期開始時刻　　*/
									if( amartim != 0 )					/*　余り時間が０ではない場合　　　　　　　　　*/
										amr_flg = ON;					/*　余りフラグをＯＮ　　　　　　　　　　　　　*/
								}										// 
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
						else											/*　料金計算後時刻＞定期終了時刻の場合　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							memcpy( &car_in,&wok_tm,7 );				/*　計算入庫時刻を計算入庫時刻とする　　　　　*/
							if( iti_flg == ON )							/*　一定料金帯フラグがＯＮの場合　　　　　　　*/
								pat = 1;								/*　料金の取りかたを追加（第１パタン）とする　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　終了時刻に加算しない場合　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					ret = ec64( &bas_ot,&cal_tm );						/*　出庫時刻と計算終了後時刻の比較　　　　　　*/
					if( ret >= 0 )										/*　出庫時刻≦料金計算後時刻の場合　　　　　　*/
						loflg = 0;										/*　ループｆｌｇクリア　　　　　　　　　　　　*/
					else												/*　出庫時刻＞料金計算後時刻の場合　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						ret = ec64( &cal_tm,&tki_ed );					/*　料金計算後時刻と定期終了時刻の比較　　　　*/
						if( ret >= 0 )									/*　料金計算後時刻≦定期終了時刻の場合　　　　*/
						{												// 
							memcpy( &car_in,&tki_ed,7 );				/*　定期終了時刻を計算入庫時刻とする　　　　　*/
							memcpy( &nmax_orgin,&tki_ed,7 );			/*　時刻構造体　定期開始時刻　　　　　　　　　*/
						}												// 
						else											/*　料金計算後時刻＞定期終了時刻の場合　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							memcpy( &car_in,&cal_tm,7 );				/*　料金計算後時刻を計算入庫時刻とする　　　　*/
							if( iti_flg == ON )							/*　一定料金帯フラグがＯＮの場合　　　　　　　*/
								pat = 1;								/*　料金の取りかたを追加（第１パタン）とする　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( FeeMaxType == SP_MX_NON )								/*　最大料金指定がない場合　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				ryo = ryo + tt_ryo;										/*　料金を加算する　　　　　　　　　　　　　　*/
				ryoukin = 0;											/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　最大指定がある場合　　　　　　　　　　　　*/
				ryo = tt_ryo;											/*　料金はそのまま　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			ryoprc = NG;												/*　料金計算要求をクリアする　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	switch( FeeMaxType )												/*　最大料金指定がない場合　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		case 0:															/*　最大料金指定がない場合　　　　　　　　　　*/
			ryoukin = ryo;												/*　　　　　　　　　　　　　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		case 1:															/*　　　　　　　　　　　　　　　　　　　　　　*/
		case SP_MX_N_HOUR:												/*　　　　　　　　　　　　　　　　　　　　　　*/
		case SP_MX_BAND:												/*　　　　　　　　　　　　　　　　　　　　　　*/
		case SP_MX_612HOUR:												// 
		case 10:														// ｎ時間最大
		case SP_MX_N_MHOUR_W:											// 2種類のｎ時間最大
			ryoukin = ryoukin + total;									/*　料金を集計中の駐車料金に加算　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		case 2:															/*　　　　　　　　　　　　　　　　　　　　　　*/
		case 3:															/*　　　　　　　　　　　　　　　　　　　　　　*/
			ryoukin = tt_x + tt_y + ryoukin + jmax_ryo;					/*　　　　　　　　　　　　　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		// 
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/***********************************************************************************************************************/
static void et471_nmax( char first, short t_kind )						// 
{																		// 
	short	wk_calcnt;													// 
	char	wk_car;														// 
	char	wk_style;													// 
	long	wk_chk_max1;												// 
	struct	CAR_TIM		wok_tm;											// 
	struct	CAR_TIM		tki_st;											// 
	struct	CAR_TIM		tki_ed;											// 
																		// 
	short	nmax_tim1;													// 
	short	nm_set1;													// 
																		// 
	nmax_tim1 = NMAX_HOUR1;												// 
	nm_set1 = nmax_set1;												// 
	if( EXP_FeeCal == 1 ){												// 
		nmax_tim1 = EXP_NMAX_HOUR1[car_type-1];							// 
		nm_set1 = exp_nmax_set1[car_type-1];							// 
	}																	// 
	memcpy( &wok_tm, &cal_tm24, 7 );									// 
	memcpy( &cal_tm24, &car_in, 7 );									// 現在の計算時刻を更新する
																		// 
	wk_car = (char)req_rkn.syubt;										// 計算要求テーブルの車種を参照する
	if( !nmax_tim_disc ){												// 設定：Ｎ時間最大の基準時刻を時間割引分進める
		wk_style = ec61( &cal_tm24 );									// 今日の体系算出処理
	}else{																// 
		wk_style = ec61( &nmax_orgin );									// 
	}																	// 
	wk_chk_max1 = day_max1[wk_style-1][wk_car-1];						// 
																		// 
	if( nmax_pass_reset == 1 ){											// ◆定期帯後の基準時刻のﾘｾｯﾄあり
																		// 
		if( nmax_countup == 1 ){										// 最大を越えた時にｶｳﾝﾄする
			if( ryoukin >= wk_chk_max1 ){								// 駐車料金がｎ時間最大料金を超えた場合
				nmax_cnt ++;											// 最大料金徴収回数ｶｳﾝﾄｱｯﾌﾟ
			}															// 
		}else{															// 駐車料金に関係なくｎ時間を超えた場合
			if( first != ON ){											// 初回計算でない時
				if( (ec71a( &nmax_in,&wok_tm )%nmax_tim1) ){			// 
					nmax_cnt ++;										// ｎ時間最大料金徴収ｶｳﾝﾄｱｯﾌﾟ
				}														// 
			}															// 
		}																// 
		total		+= ryoukin;											// totalに加算しryoukinはｸﾘｱ
		ryoukin		= 0;												// 
		nmax_style	= 0;												// ◆定期帯後ﾘｾｯﾄする場合にはchk_max1の更新が必要
																		//   et2100先頭で現在のstyleのchk_max1にするためnmax_styleをｸﾘｱする
		in_calcnt	= 0;												// 
		wk_calcnt = 0;													// 
		nmax_amari2 = 0;												// ｎ2の余り時間
		if( !nmax_tim_disc ){											// 設定：Ｎ時間最大の基準時刻を時間割引分進める
			memcpy( &nmax_in,&car_in,7 );								// 入庫基点時刻の更新
		}else{															// 
			memcpy( &nmax_in,&nmax_orgin,7 );							// 時刻構造体　定期開始時刻
			if( first == ON ){											// 初回計算の時
				if( (CPrmSS[S_CAL][30] == 0)&&							// 
					((su_jik_dtm)||(ec64( &nmax_in, &car_in ) == 1)) ){	// 
					memcpy ( &tki_st,&nmax_in,7 );						// 計算入庫時刻に更新時刻のセットをする
					if( ec67( t_kind,&tki_st,&tki_ed,0 ) == OK ){		// 更新時刻の定期帯を求める
						memcpy( &nmax_orgin,&tki_ed,7 );				// 時刻構造体　定期開始時刻
						memcpy( &nmax_in,&nmax_orgin,7 );				// 時刻構造体　定期開始時刻
					}													// 
				}														// 
				wk_calcnt = 											// 
					(short)(ec71a(&nmax_in,&cal_tm24)/nmax_tim1);		// 現在のｎ時間経過を確認する
				nmax_cnt = wk_calcnt;									// 
			}															// 
			in_calcnt = wk_calcnt;										// 
		}																// 
																		// 
	}else{																// ◆定期帯後の基準時刻のﾘｾｯﾄなし
																		// 
		memcpy( &wok_tm, &car_in, 7 );									// 
		if( amr_flg == T_TX_TIM ){										// 
			ec70( &wok_tm, (long)amartim );								// 
		}																// 
																		// ◆定期有効時間帯分の時間を進めている為ｎ時間達成回数も再計算
		wk_calcnt = (short)(ec71a( &nmax_in, &wok_tm )/nmax_tim1);		// 現在のｎ時間経過を確認する
		if( wk_calcnt != in_calcnt ){									// ｎ時間経過の更新がある場合
			if( nmax_countup == 1 ){									// 最大を越えた時にｶｳﾝﾄする
				if( ryoukin >= wk_chk_max1 ){							// 駐車料金がｎ時間最大料金を超えた場合
					nmax_cnt ++;										// 最大料金徴収回数+1ｶｳﾝﾄｱｯﾌﾟ
				}														// 
			}else{														// 駐車料金に関係なくｎ時間を超えた場合
				if( wk_calcnt >= in_calcnt ){ 							// 
					nmax_cnt += (wk_calcnt - in_calcnt);				// 最大料金領収回数分ｶｳﾝﾄｱｯﾌﾟ
				}														// 
			}															// 
			total		+= ryoukin;										// TOTALに加算
			ryoukin		= 0;											// 
			nmax_style	= 0;											// Ｎ時間経過でchk_max1も更新が必要
																		// ◆et2100先頭で現在のstyleのchk_max1にするためnmax_styleをｸﾘｱする
		}																// 
		in_calcnt = wk_calcnt;											// 
		if( nmax_pass_calc == 0 ){										// 0=料金発生時の体系の場合
			nmax_style = 0;												// 定期帯後の料金計算開始時のchk_max1を更新する
		}																// 
	}																	// 
	if(( nmax_cnt >= nm_set1 )&&( nm_set1 != 0 )){						// ｎ時間最大料金徴収回数制限ありで上限超え
			chk_max1 = NMAX_OVER;										// ｎ時間最大ｍ回経過後の最大料金
			chk_max2 = NMAX_OVER;										// ｎ時間最大ｍ回経過後の最大料金
	}																	// ｍ回継続中または∞?(Y)
}																		// 
