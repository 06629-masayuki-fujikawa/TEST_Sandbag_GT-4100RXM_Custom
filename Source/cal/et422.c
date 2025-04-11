/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：定期時間割引額算出　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et422() 　　　　　　　　　：　定期処理後の時間割引額の算出処理を行う。　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　無し　　　　　　　　　　　　割引額　　　　　　　　　　　　　：　*/
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
extern	char	sy_jkn_flg;												// 種別時間割引ありでの時間帯定期使用ﾌﾗｸﾞ
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
long	et422()															/*　定期処理後時間割引処理　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	struct	CAR_TIM		wok_in;											/*　時刻構造体 car_tim型構造帯名　ｗｏｒｋ入庫*/
	struct	CAR_TIM		ren_in;											/*　時刻構造体 car_tim型構造帯名　更新入庫時刻*/
	struct	CAR_TIM		wk_in;											/*　時刻構造体 car_tim型構造帯名　ｗｏｒｋ入庫*/
	struct	CAR_TIM		tki_st;											/*　時刻構造体 car_tim型構造帯名　定期開始時刻*/
	struct	CAR_TIM		tki_ed;											/*　時刻構造体 car_tim型構造帯名　定期終了時刻*/
	struct	CAR_TIM		wok_in2;										/*　時刻構造体 car_tim型構造帯名　ｗｏｒｋ入庫*/
	short	flg = 1;													/*　ループｆｌｇ　　　　　　　　　　　　　　　*/
	char	ps_flg = NG;												/*　定期有効無効ｆｌｇ　　　　　　　　　　　　*/
	short	ret = 0;													/*　時刻比較リタンコード　　　　　　　　　　　*/
	long	wok_ryo;													/*　割引前料金　　　　　　　　　　　　　　　　*/
// MH810103(S) R.Endo 2021/07/29 車番チケットレス フェーズ2.2 #5841 【検証課指摘事項】「QR割引券で種別切換して精算」 → 「ラグタイム後 かつ 追加料金発生前に再精算」 を行うと割引金額に不正な値がセットされる
//	long	disc;														/*　割引額　　　　　　　　　　　　　　　　　　*/
	long	disc = 0;													/*　割引額　　　　　　　　　　　　　　　　　　*/
// MH810103(E) R.Endo 2021/07/29 車番チケットレス フェーズ2.2 #5841 【検証課指摘事項】「QR割引券で種別切換して精算」 → 「ラグタイム後 かつ 追加料金発生前に再精算」 を行うと割引金額に不正な値がセットされる
	short	t_kind = 0;													/*　定期券種別　　　　　　　　　　　　　　　　*/
	short	ryo_pat = 0;												/*　料金の取りかた　　　　　　　　　　　　　　*/
	char	taik = 0;													/*　体系　　　　　　　　　　　　　　　　　　　*/
	short 	timfun = 0;													/*　入庫時刻の計算用　　　　　　　　　　　　　*/

	struct	CAR_TIM		w_ren_in;										// ren_in更新用ﾜｰｸ変数
	struct	CAR_TIM		tki_st_w;										// 定期開始時刻退避用変数
	struct	CAR_TIM		tki_ed_w;										// 定期終了時刻退避用変数
	char	ren_flg;													// 定期帯日付更新ﾌﾗｸﾞ
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if((req_rkn.param == RY_CSK) || (req_rkn.param == RY_CMI)){			// ｻｰﾋﾞｽ券、掛売中止券使用時(精算中止機能強化)
		memcpy( &wok_in2,&car_in,7 );									/*　入庫時刻をｗｏｒｋ２入庫時刻に保存　　　　*/
		memcpy( &car_in,&recalc_carin,7 );
	}
	memcpy( &wok_in,&car_in,7 );										/*　入庫時刻をｗｏｒｋ入庫時刻に保存　　　　　*/
	memcpy( &ren_in,&car_in,7 );										/*　入庫時刻を更新入庫時刻に保存　　　　　　　*/
	memcpy( &car_ot,&carot_mt,7 );										/*　出庫時刻は基本出庫時刻とする　　　　　　　*/
	memset( &tki_st,0,7 );												/*　定期開始時刻クリア　　　　　　　　　　　　*/
	memset( &tki_ed,0,7 );												/*　定期終了時刻クリア　　　　　　　　　　　　*/
	memcpy( &nmax_orgin,&org_in,7 );									// Ｎ時間最大定期用入庫時刻を保持
	t_kind = (short)tik_syubet;											// 定期種別のセット（定期処理用）
	pass_tm_flg = 0;													// 
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( tki_flg != OFF )												/*　時間帯定期後時間割引キー処理なら　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		t_kind = tki_syu;												/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	req_rkn.data[0] = req_rkn.data[1];									/*　定期種別のセット（定期処理用）　　　　　　*/
	req_rkn.data[1] = 0;												/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	ryo_pat = (short)Carkind_Param(WARI_PAY_PTN, car_type, 2,1);		/*　料金の取り方参照　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( (short)CPrmSS[S_CAL][30] == 0 )									/*　重複した時間割引は無効の場合　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		ec70( &car_in,jik_dtm );										// 更新入庫時刻を時間割引分進める
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　重複した時間割引は有効の場合　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		while( flg )													/*　《　更新入庫時刻を求める　》　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			ren_flg = 0;												// 定期帯日付更新ﾌﾗｸﾞﾘｾｯﾄ
			memcpy( &tki_st,&ren_in,7 );								/*　更新入庫時刻を計算時刻にセット　　　　　　*/
			ps_flg = ec67( t_kind,&tki_st,&tki_ed,0 );					// 定期帯範囲算出
			if( ps_flg == OK )											/*　定期有効の場合　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				ret = ec64( &car_in,&tki_st );							/*　更新入庫時刻と定期開始時刻の比較　　　　　*/
				switch( ret )											/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					case 0:												/*　更新入庫時刻＝定期開始時刻の場合　　　　　*/
						memcpy( &car_in,&tki_ed,7 );					/*　更新入庫時刻＝定期終了時刻　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
						memcpy( &nmax_orgin,&tki_ed,7 );				// 
						if(	pass_week_chg == PASS_WEEK_DAY_SP ){		// 定期帯を0:00で区切る設定の時
							memcpy( &w_ren_in,&ren_in,7 );				// 
							taik = ec61( &w_ren_in );					// その日の体系を求める
							if(ps_sepa[taik-1][t_kind-1] == 1){			// 定期帯が0:00で分割
								memcpy( &ren_in, &tki_ed, 7 );			// 
								ren_flg = 1;							// 定期帯日付更新ﾌﾗｸﾞｾｯﾄ（更新済み）
							}											// 
						}												// 
					break;												/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					case -1:											/*　更新入庫時刻＞定期開始時刻の場合　　　　　*/
						ret = ec64( &car_in,&tki_ed );					/*　更新入庫時刻と定期終了時刻の比較　　　　　*/
						if( ret != -1 )									/*　　　　　　　　　　　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							memcpy( &car_in,&tki_ed,7 );				/*　更新入庫時刻＝定期終了時刻　　　　　　　　*/
							memcpy( &nmax_orgin,&tki_ed,7 );			// 
							ret = ec64( &car_in,&car_ot );				/*　年月日時分比較処理　　　　　　　　　　　　*/
							if( ret == -1 ){							// 更新入庫時刻が出庫時刻を越えている
								flg = 0;								// ﾙｰﾌﾟを抜け料金計算処理へ
							}else{										// 更新入庫時刻が出庫時刻を越えていない
								ren_flg = 1;							// 定期帯日付更新ﾌﾗｸﾞｾｯﾄ（更新済み）
								if(	pass_week_chg == PASS_WEEK_DAY_SP ){// 定期帯を0:00で区切る
									memcpy( &w_ren_in,&ren_in,7 );		// 
									taik = ec61( &w_ren_in );			// その日の体系を求める
									if(ps_sepa[taik-1][t_kind-1] == 1){	// 定期帯が0:00で分割
										memcpy( &ren_in, &tki_ed, 7 );	// 
									}else{								// 
										ec63( &ren_in );				// 更新時刻を１日繰り上げる
									}									// 
								}else{									// 
									ec63( &ren_in );					// 更新時刻を１日繰り上げる
								}										// 
							}											// 
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					break;												/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					case 1:												/*　更新入庫時刻＜定期開始時刻の場合　　　　　*/
																		// 
						taik = ec61( &tki_st );							// 定期開始時刻で体系を求める(現在の体系を求める)
						timfun = ta_st_sel(taik,0);						// 現在の体系開始時刻を求める
						memcpy( &wk_in,&tki_st,7 );						// 
						if( timfun > 0 ){								// 体系開始時刻が0:00以外
							timfun -= 1;								// 
							wk_in.hour = (char)(timfun/60);				// 
							wk_in.min  = (char)(timfun%60);				// 
						}else{											// 体系開始時刻が0:00の時
							timfun = 1440-1;							// 
							ec62( &wk_in );								// 1日前に戻す
							wk_in.hour = (char)(timfun/60);				// 
							wk_in.min  = (char)(timfun%60);				// 
						}												// 
						taik = ec61( &wk_in );							// 1日前の体系を求める
						memcpy( &tki_st_w,&tki_st,7 );					// 現在の定期開始時刻を保存
						memcpy( &tki_ed_w,&tki_ed,7 );					// 現在の定期終了時刻を保存
						memcpy( &tki_st,&wk_in,7 );						// 1日前の定期帯算出のため仮入庫時刻を定期開始とする
						ps_flg = ec67( t_kind,&tki_st,&tki_ed,0 );		// 1日前の定期帯範囲算出
						if( ps_flg == OK ){								// 
							ret = ec64( &car_in,&tki_ed );				// 更新入庫時刻と1日前の定期終了時刻の比較
							if( ret == 1 ){								// 定期終了時刻より更新入庫時刻が前
								ret = ec64( &tki_st,&car_in );			// 
								if( ret != -1 ){						// 定期開始時刻より更新入庫時刻が後
									memcpy( &car_in,&tki_ed,7 );		// 定期終了時刻を更新入庫時刻とする
									memcpy( &nmax_orgin,&tki_ed,7 );	// 
								}										// 
							}											// 
						}												// 
						memcpy( &tki_st,&tki_st_w,7 );					// 保存した定期開始時刻へ戻す
						memcpy( &tki_ed,&tki_ed_w,7 );					// 保存した定期終了時刻へ戻す
						ec70( &car_in,jik_dtm );						// 日付更新処理(加算)
						ret = ec64( &tki_st,&car_in );					// 定期開始時刻と更新後入庫時刻の比較
						if( ret == 1 ){									// 定期開始時刻＜更新後入庫時刻の場合
							jik_dtm = (long) ec71( &tki_st,&car_in );	// 更新時刻＝更新後時刻－定期開始時刻
							memcpy( &car_in,&tki_ed,7 );				// 定期終了時刻を更新入庫時刻とする
							memcpy( &nmax_orgin,&tki_ed,7 );			// 
						}else{											// 定期開始時刻≧更新後入庫時刻の場合
							flg = 0;									// ループｆｌｇクリア
						}												// 
					break;												/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　定期無効の場合　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				memcpy( &tki_st_w,&tki_st,7 );							// 定期開始時刻を定期帯計算用ﾜｰｸへ格納
				memcpy( &tki_ed_w,&tki_ed,7 );							// 定期終了時刻を定期帯計算用ﾜｰｸへ格納
				ps_flg = ec67( t_kind,&tki_st_w,&tki_ed_w,1 );			// 強制的に定期有効として定期帯範囲算出
				memcpy( &tki_ed,&tki_ed_w,7 );							// 
																		// 
				ec70( &car_in,jik_dtm );								// 入庫時刻を時間割引分進める
				ret = ec64( &tki_ed,&car_in );							// 定期終了時刻と更新後入庫時刻の比較
				if( ret == 1 ){											// 定期終了時刻＜更新後入庫時刻の場合
					jik_dtm = (long) ec71( &tki_ed,&car_in );			// 時間割引の残り＝更新後入庫時刻－定期終了時刻
					memcpy( &car_in,&tki_ed,7 );						// 定期終了時刻を更新時刻とする
				}else{													// 定期終了時刻≧更新後入庫時刻の場合
					flg = 0;											// ループｆｌｇクリア
				}														// 
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ren_flg == 0 ){											// 定期帯日付更新ﾌﾗｸﾞ未ｾｯﾄ（更新未）
				if(	pass_week_chg == PASS_WEEK_DAY_SP ){				// 設定が定期帯を0:00で区切る
					memcpy( &w_ren_in,&ren_in,7 );						// 
					w_ren_in.hour = 0;									// 
					w_ren_in.min  = 0;									// 
					taik = ec61( &w_ren_in );							// その日の体系を求める
					if(ps_sepa[taik-1][t_kind-1] == 1){					// 定期帯が0:00で分割
						ren_in.hour = 0;								// 
						ren_in.min = 0;									// 
					}													// 
					ec63( &ren_in );									// 日付１日繰り上げ処理
				}else{													// 
					ec63( &ren_in );									// 日付１日繰り上げ処理
				}														// 
			}															// 
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	ret = ec64( &car_in,&car_ot );										/*　年月日時分比較処理　　　　　　　　　　　　*/
																		// 
	switch( ret )														/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		case 0:															/*　更新入庫時刻＝出庫時刻　　　　　　　　　　*/
			if(	sy_jkn_flg == 0 )										/*　種別時間割引ありでの時間帯定期でない　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				wok_ryo = base_ryo;										/*　定期処理後料金の保存　　　　　　　　　　　*/
				if( ctyp_flg == 4 )										/*　　　　　　　　　　　　　　　　　　　　　　*/
					disc = wok_ryo;										/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　　　　　　　　　　　　　　　　　　　　　　*/
					disc = base_ryo;									/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　種別時間割引ありでの時間帯定期　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				wok_ryo = base_ryo;										/*　定期処理後料金の保存　　　　　　　　　　　*/
				memcpy( &wk_in,&car_in,7 );								/*　ｗｏｒｋ入庫時刻に入庫時刻をセットする　　*/
				et471( t_kind,ryo_pat );								/*　定期処理　　　　　　　　　　　　　　　　　*/
				memcpy( &car_in,&wk_in,7 );								/*　ｗｏｒｋ入庫時刻を入庫時刻とする　　　　　*/
				if( ryoukin >= wok_ryo )								/*　料金≧駐車料金（割引前）　　　　　　　　　*/
					disc = 0;											/*　割引額＝０　　　　　　　　　　　　　　　　*/
				else													/*　料金＜駐車料金（割引前）　　　　　　　　　*/
					disc = wok_ryo - ryoukin;							/*　割引額＝定期処理後料金－料金　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		case 1:															/*　更新入庫時刻＜出庫時刻　　　　　　　　　　*/
			wok_ryo = base_ryo;											/*　定期処理後料金の保存　　　　　　　　　　　*/
			memcpy( &wk_in,&car_in,7 );									/*　ｗｏｒｋ入庫時刻に入庫時刻をセットする　　*/
			et471( t_kind,ryo_pat );									/*　定期処理　　　　　　　　　　　　　　　　　*/
			memcpy( &car_in,&wk_in,7 );									/*　ｗｏｒｋ入庫時刻を入庫時刻とする　　　　　*/
			if( ryoukin >= wok_ryo )									/*　料金≧駐車料金（割引前）　　　　　　　　　*/
				disc = 0;												/*　割引額＝０　　　　　　　　　　　　　　　　*/
			else														/*　料金＜駐車料金（割引前）　　　　　　　　　*/
				disc = wok_ryo - ryoukin;								/*　割引額＝定期処理後料金－料金　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		case -1:														/*　更新入庫時刻＞出庫時刻　　　　　　　　　　*/
			if( jitu_wari == 1 )										/*　実割引の場合　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(	sy_jkn_flg == 0 )									/*　時間帯定期からの呼び出しでない場合　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					wok_ryo = base_ryo;									/*　定期処理後料金の保存　　　　　　　　　　　*/
					if( ctyp_flg == 4 )									/*　　　　　　　　　　　　　　　　　　　　　　*/
						disc = wok_ryo;									/*　　　　　　　　　　　　　　　　　　　　　　*/
					else												/*　　　　　　　　　　　　　　　　　　　　　　*/
						disc = base_ryo;								/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　種別時間割引ありで時間帯定期使用　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					wok_ryo = base_ryo;									/*　定期処理後料金の保存　　　　　　　　　　　*/
					memcpy( &wk_in,&car_in,7 );							/*　ｗｏｒｋ入庫時刻に入庫時刻をセットする　　*/
					et471( t_kind,ryo_pat );							/*　定期処理　　　　　　　　　　　　　　　　　*/
					memcpy( &car_in,&wk_in,7 );							/*　ｗｏｒｋ入庫時刻を入庫時刻とする　　　　　*/
					if( ryoukin >= wok_ryo )							/*　料金≧駐車料金（割引前）　　　　　　　　　*/
						disc = 0;										/*　割引額＝０　　　　　　　　　　　　　　　　*/
					else												/*　料金＜駐車料金（割引前）　　　　　　　　　*/
						disc = wok_ryo - ryoukin;						/*　割引額＝定期処理後料金－料金　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　実割引ではない場合　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(	sy_jkn_flg == 0 )									/*　時間帯定期からの呼び出しでない時　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					memcpy( &car_ot,&car_in,7 );						/*　時間割引後入庫時刻を出庫時刻とする　　　　*/
					memcpy( &car_in,&carin_mt,7 );						/*　ｗｏｒｋ入庫時刻を入庫時刻とする　　　　　*/
					et471( t_kind,ryo_pat );							/*　定期処理　　　　　　　　　　　　　　　　　*/
					if( ctyp_flg == 4 )									/*　　　　　　　　　　　　　　　　　　　　　　*/
						disc = base_ryo;								/*　　　　　　　　　　　　　　　　　　　　　　*/
					else												/*　　　　　　　　　　　　　　　　　　　　　　*/
						disc = ryoukin;									/*　　　　　　　　　　　　　　　　　　　　　　*/
					memcpy( &car_in,&wok_in,7 );						/*　ｗｏｒｋ入庫時刻を入庫時刻とする　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　種別時間割引ありで時間帯定期使用　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					wok_ryo = base_ryo;									/*　定期処理後料金の保存　　　　　　　　　　　*/
					memcpy( &wk_in,&car_in,7 );							/*　ｗｏｒｋ入庫時刻に入庫時刻をセットする　　*/
					et471( t_kind,ryo_pat );							/*　定期処理　　　　　　　　　　　　　　　　　*/
					memcpy( &car_in,&wk_in,7 );							/*　ｗｏｒｋ入庫時刻を入庫時刻とする　　　　　*/
					if( ryoukin >= wok_ryo )							/*　料金≧駐車料金（割引前）　　　　　　　　　*/
						disc = 0;										/*　割引額＝０　　　　　　　　　　　　　　　　*/
					else												/*　料金＜駐車料金（割引前）　　　　　　　　　*/
						disc = wok_ryo - ryoukin;						/*　割引額＝定期処理後料金－料金　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/

	if((req_rkn.param == RY_CSK) || (req_rkn.param == RY_CMI))			// 精算中止機能強化
	{
		memcpy( &recalc_carin,&car_in,7 );
		memcpy( &car_in,&wok_in2,7 );									/*　ｗｏｒｋ２入庫時刻を入庫時刻とする　　　　*/
	}
	return( disc );														/*　割引額を返す　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
