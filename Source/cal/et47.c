/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：定期券処理　　　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et47()　　　　　　　　　　：　定期の種別により、時間帯定期・時間割引定期・車種切り替え後時：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：間割引定期の３通りの処理を行う。　　　　　　　　　　　　　　　：　*/
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
char	sy_jkn_flg = 0;													// 種別時間割引ありでの時間帯定期使用ﾌﾗｸﾞ
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	et47()															/*　定期券処理　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	char	prc		 = NG;												/*　処理可・不可ｆｌｇ　　　　　　　　　　　　*/
	char	t_prc	 = OK;												/*　定期処理ｆｌｇ　　　　　　　　　　　　　　*/
	short	tki		 = 0;												/*　定期種別　　　　　　　　　　　　　　　　　*/
	short	pass	 = 0;												/*　定期使用目的　　　　　　　　　　　　　　　*/
	short	ret		 = 0;												/*　リタンコード　　　　　　　　　　　　　　　*/
	short	ryo_pat	 = 0;												/*　料金の取りかた　　　　　　　　　　　　　　*/
	short	taik	 = 0;												/*　体系　　　　　　　　　　　　　　　　　　　*/
	short	ret_w	 = 0;												/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	wok[2];														/*　サービスタイム・グレースタイム　　　　　　*/
	short	yuko_strt;													/*　時間割引有効開始時刻　　　　　　　　　　　*/
	short	yuko_end;													/*　時間割引有効終了時刻　　　　　　　　　　　*/
	short	yuko_tm;													/*　時間割引有効時刻　　　　　　　　　　　　　*/
	long	jik_new	 = 0l;												/*　時間割引後入庫時間　　　　　　　　　　　　*/
	struct	RYO_BUF_N	*r;												/*　料金データバッファ　　　　　　　　　　　　*/
	struct	REQ_RKN	*req_p;												/*　現在計算後格納エリア　　　　　　　　　　　*/
	struct	CAR_TIM		wok_tm;											/*　時刻構造体　ｗｏｒｋ時刻　　　　　　　　　*/
	volatile long	su_discount;												/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	in_m = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	ot_m = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*											*/
	short	wk_adr;														/*											*/
	char	tmp_cyu_in = 0;
																		/*											*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	r	  		 =	&ryo_buf_n;											/*　料金計算バッファポインタ　　　　　　　　　*/
	req_p 		 = &req_rhs[tb_number];									/*　現在料金計算要求ポインター　　　　　　　　*/
	memmove( req_p,&req_rkn,sizeof(req_rkn) );							/*　現在料金計算要求TBLを計算結果TBLへセット　*/
	memset( r,0,sizeof(ryo_buf_n) );									/*　料金計算バッファクリア　　　　　　　　　　*/
	tol_dis		 = 0;													/*　割引額＝０　　　　　　　　　　　　　　　　*/
	discount	 = 0;													/*　割引額＝０　　　　　　　　　　　　　　　　*/
	mae_dis		 = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
	jik_dtm		 = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
	percent		 = 0;													/*　％割引率クリア　　　　　　　　　　　　　　*/
	tyushi_mae_dis = 0;
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( rag_flg == ON )													/*　ラグタイムｆｌｇがＯＮの場合　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		wok[0] = 0;														/*　サービスタイム＝０　　　　　　　　　　　　*/
		wok[1] = 0;														/*　グレースタイム＝０　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　ラグタイムｆｌｇがＯＦＦの場合　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		wok[0] = se_svt.stim;											/*　サービスタイムのセット　　　　　　　　　　*/
		wok[1] = se_svt.gtim;											/*　グレースタイムのセット　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	amr_flg = OFF;														/*　余りフラグをＯＦＦとする　　　　　　　　　*/
	tki	= (short)tik_syubet;											// 
	tki_syu = tki;														/*　　　　　　　　　　　　　　　　　　　　　　*/
	pass = ps_type[tki-1];						 						/*　定期使用目的の算出　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	memcpy( &carin_mt,&org_in,7 );										/*　入庫時刻を基本入庫時刻とする　　　　　　　*/
	memcpy( &carot_mt,&org_ot,7 );										/*　出庫時刻を基本出庫時刻とする　　　　　　　*/
	memcpy( &car_in,&carin_mt,7 );										/*　入庫時刻を基本入庫時刻とする　　　　　　　*/
	memcpy( &car_ot,&carot_mt,7 );										/*　出庫時刻を基本出庫時刻とする　　　　　　　*/
	memcpy( &recalc_carin,&car_in,7 );									/* 再計算用入庫時刻を基本入庫時刻とする		*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	ec70( &car_in,(long)wok[0] );										/*　日付更新処理（設定サービスタイム分）　　　*/
	ret = ec64( &carot_mt,&car_in );									/*　年月日時分比較処理　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( ret >= 0 )														/*　サービスタイム内出庫の場合　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		ryoukin = 0;													/*　料金＝０　　　　　　　　　　　　　　　　　*/
		t_prc = NG;														/*　定期処理ＮＧ　　　　　　　　　　　　　　　*/
		ryo_buf.svs_tim = 1;											/*　サービスタイム内精算有り　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　サービスタイム外出庫の場合　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		memcpy( &car_in,&carin_mt,7 );									/*　入庫時刻を基本入庫時刻とする　　　　　　　*/
		ec66( &carot_mt,(long)wok[1]);									// 日付更新処理（出庫時刻をグレースタイム分）
		ret = ec64( &carin_mt,&carot_mt );								/*　年月日時分比較処理　　　　　　　　　　　　*/
		if( ret != 1 )													/*　入庫時刻≧出庫時刻の場合　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			memcpy( &carot_mt,&carin_mt,sizeof(struct CAR_TIM) );		/*　入庫時刻を基本出庫時刻とする　　　　　　　*/
			ec70( &carot_mt,(long)1 );									/*　日付更新処理（料金を１単位分取るため）　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		memcpy( &car_ot,&carot_mt,7 );									/*　出庫時刻を基本出庫時刻とする　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		wk_adr = (short)(1+6*(req_rkn.syubt-1));						/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( prm_get(COM_PRM,S_SHA,wk_adr,2,5) == 1 )					/*　車種が料金種別の場合　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			su_jik_dtm = Prm_RateDiscTime[req_rkn.syubt-1];				/*　種別割引時間を求める　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( su_jik_dtm != 0 )											/*　種別割引時間がある場合　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			in_time = (short)car_in.hour * 60 + (short)car_in.min;		/*　入庫時刻を分で表現　　　　　　　　　　　　*/
			taik = ec61( &car_in );										/*　入庫時刻の体系算出処理　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( in_time < ta_st_sel((char)taik,1) )						// 体系の開始時刻より前の場合
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				memcpy( &wok_tm,&carin_mt,7 );							/*　　　　　　　　　　　　　　　　　　　　　　*/
				ec62( &wok_tm );										/*　ｗｏｒｋ時刻を前日とする　　　　　　　　　*/
				taik = ec61( &wok_tm );									/*　ｗｏｒｋ時刻の体系算出処理　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( cal_type == 0 )										/*　料金計算方式　０：昼夜帯方式　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( ovl_time[taik-1][0] != 0 )						/*　体系切替による重複帯があれば　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						memcpy( &wok_tm,&carin_mt,7 );					/*　　　　　　　　　　　　　　　　　　　　　　*/
						wok_tm.hour = (char)( ta_st_sel((char)taik,1) / 60 );	// 体系開始年月日を求める
						wok_tm.min = (char)( ta_st_sel((char)taik,1) % 60 );	// 
						ec70( &wok_tm,(long)ovl_time[taik-1][0] );		/*　重複時間を加算する　　　　　　　　　　　　*/
						ret_w = ec64( &carot_mt,&wok_tm );				/*　出庫時刻と重複帯の時刻比較　　　　　　　　*/
						if( ret_w != -1 )								/*　重複帯内出庫の場合　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							if( ovl_type[taik-1][0] == 1 )				/*　重複方式が１の場合　　　　　　　　　　　　*/
							{											/*　　　　　　　　　　　　　　　　　　　　　　*/
								memcpy( &wok_tm,&carin_mt,7 );			/*　　　　　　　　　　　　　　　　　　　　　　*/
								ec62( &wok_tm );						/*　ｗｏｒｋ時刻を前日とする　　　　　　　　　*/
								taik = ec61( &wok_tm );					/*　ｗｏｒｋ時刻の体系算出処理　　　　　　　　*/
							}											/*　　　　　　　　　　　　　　　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( jitu_wari == 1 ){
				if( org_ot.mon == org_in.mon && org_ot.day == org_in.day ){	/*　　　　　　　　　　　　　　　　　　　　*/
					in_m = (short)org_in.hour * 60 + (short)org_in.min;		/*　基本入庫時刻を分で表現　　　　　　　　*/
					ot_m = (short)org_ot.hour * 60 + (short)org_ot.min;		/*　基本出庫時刻を分で表現　　　　　　　　*/
					if( su_jik_dtm >= ( ot_m - in_m ))						/*　　　　　　　　　　　　　　　　　　　　*/
						su_jik_dtm = ot_m - in_m;							/*　　　　　　　　　　　　　　　　　　　　*/
				}															/*　　　　　　　　　　　　　　　　　　　　*/
			}																/*　　　　　　　　　　　　　　　　　　　　*/
			yuko_strt = ji_st[taik-1];									/*　時間割引の有効開始時刻　　　　　　　　　　*/
			yuko_end = ji_ed[taik-1];									/*　時間割引の有効終了時刻　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(( yuko_strt > yuko_end )&&( in_time < yuko_end ))		/*　有効開始時刻が有効終了時刻以降の場合　　　*/
			{															/*　入庫時刻が有効終了時刻より前の場合　　　　*/
				prc = OK;												/*　処理ＯＫ　　　　　　　　　　　　　　　　　*/
				yuko_tm = yuko_end - in_time;							/*　有効時間＝有効終了時間－有効開始時間　　　*/
				if( su_jik_dtm > yuko_tm )								/*　割引時間＞有効時間の場合　　　　　　　　　*/
					su_jik_dtm = yuko_tm;								/*　有効時間が割引時間となる　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　　　　　　　　　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( yuko_strt > yuko_end )								/*　有効開始時刻が有効終了時刻以降の場合　　　*/
					yuko_end = yuko_end + 1440;							/*　有効終了時刻に24時間加算　　　　　　　　　*/
				yuko_tm = yuko_end - yuko_strt;							/*　有効時間＝有効終了時間－有効開始時間　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( yuko_tm == 0 )										/*　有効開始時刻＝有効終了時刻の場合　　　　　*/
					prc = OK;											/*　処理ＯＫ　　　　　　　　　　　　　　　　　*/
				else													/*　有効開始時刻≠有効終了時刻の場合　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if(( in_time >= yuko_strt )&&( in_time < yuko_end ))/*　有効時間内の場合　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						prc = OK;										/*　処理ＯＫ　　　　　　　　　　　　　　　　　*/
						if( su_jik_dtm > yuko_tm )						/*　割引時間＞有効時間の場合　　　　　　　　　*/
							su_jik_dtm = yuko_tm;						/*　有効時間が割引時間となる　　　　　　　　　*/
						jik_new = (long)in_time + su_jik_dtm;			/*　時間割引後入庫時間の算出　　　　　　　　　*/
						if( (long)yuko_end < jik_new )					/*　有効終了時刻より割引後時刻が大きい場合　　*/
							su_jik_dtm = yuko_end - in_time;			/*　割引時間は、入庫時刻～有効終了時刻　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( prc == OK )												/*　処理ＯＫの場合　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				ec70( &car_in,(long)su_jik_dtm );						// 日付更新処理（種別時間割引分）
				ret = ec64( &carot_mt,&car_in );						/*　年月日時分比較処理　　　　　　　　　　　　*/
				if( ret >= 0 )											/*　種別時間割引内出庫の場合　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					ryoukin = 0;										/*　料金＝０　　　　　　　　　　　　　　　　　*/
					t_prc = NG;											/*　定期処理ＮＧ　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			su_jik_plus = 0;
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( t_prc == OK )													/*　定期処理ＯＫなら　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		switch( pass )													/*　定期の使用目的により分岐　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			case 1:														/*　時間帯定期の場合　　　　　　　　　　　　　*/
				if( CPrmSS[S_CAL][30] == 0 )							/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					memcpy( &carin_mt,&car_in,7 );						/*　入庫時刻を基本入庫時刻とする　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　　　　　　　　　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					memcpy( &car_in,&carin_mt,7 );						// 入庫時刻を基本入庫時刻とする
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				memcpy( &carot_mt,&car_ot,7 );							/*　出庫時刻を基本出庫時刻とする　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( prc == OK )											/*　　　　　　　　　　　　　　　　　　　　　　*/
					ryo_pat = (short)Carkind_Param(WARI_PAY_PTN, car_type, 2,1);	/*　料金の取り方参照（初回）　　　　　　　　*/
				else													/*　　　　　　　　　　　　　　　　　　　　　　*/
					ryo_pat = 0;										/*　料金の取りかた初回は基本から　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				pass_tm_flg = 1;										// 
				et471( tki,ryo_pat );									/*　定期処理　　　　　　　　　　　　　　　　　*/
				pass_tm_flg = 0;										// 
				tki_flg = PAS_TIME;										// 定期使用後ﾌﾗｸﾞを時間帯定期とする
																		// 
				if(( su_jik_dtm != 0 )&&( CPrmSS[S_CAL][30] == 1 ))		// 種別割引時間があり設定が１ならば
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					memcpy( &car_in,&carin_mt,7 );						/*　入庫時刻を基本入庫時刻とする　　　　　　　*/
					jik_dtm = su_jik_dtm;								/*　　　　　　　　　　　　　　　　　　　　　　*/
					sy_jkn_flg = 1;										// 種別時間割引ありでの時間帯定期使用ﾌﾗｸﾞ
					su_discount = et422();								/*　　　　　　　　　　　　　　　　　　　　　　*/
					sy_jkn_flg = 0;										// 種別時間割引ありでの時間帯定期使用ﾌﾗｸﾞ
					jik_dtm = 0;										/*　									  　　*/
					memcpy( &wok_tm,&car_in,7 );
					tmp_cyu_in = 1;
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				style = ec61( &carot_mt );								/*　出庫日の体系算出処理　　　　　　　　　　　*/
				if( ryoukin >= abs_max[style-1][car_type-1] )			/*　集計中の駐車料金が絶対最大料金に達した場合*/
					ryoukin = abs_max[style-1][car_type-1];				/*　絶対最大料金を駐車料金とする　　　　　　　*/
				memcpy( &car_in,&carin_mt,7 );							/*　入庫時刻を基本入庫時刻にもどす　　　　　　*/
				memcpy( &car_ot,&carot_mt,7 );							/*　出庫時刻を基本出庫時刻にもどす　　　　　　*/
				if( tmp_cyu_in == 1 )									/*　種別割引時間があり設定が１ならば　　　　　*/
					memcpy( &recalc_carin,&wok_tm,7);
				else
					memcpy( &recalc_carin,&car_in,7 );
			break;														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			case 2:														/*　割引時間定期の場合　　　　　　　　　　　　*/
				et472();												/*　割引時間定期券処理　　　　　　　　　　　　*/
				memcpy( &recalc_carin,&car_in,7 );
				memcpy( &tki_tm,&car_in,7 );
				tki_flg = PAS_DISC;										// 定期使用後ﾌﾗｸﾞを割引時間定期とする
			break;														/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	base_ryo = ryoukin;													/*　　　　　　　　　　　　　　　　　　　　　　*/
	r->ryo = base_ryo;													/*　　　　　　　　　　　　　　　　　　　　　　*/
	r->dis = 0;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	discount = 0;														/*　　　　　　　　　　　　　　　　　　　　　　*/
	tol_dis = 0;														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	r->ryo = base_ryo;													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( cons_tax_ratio != 0 )											/*　消費税率が０％ではない場合　　　　　　　　*/
		r->tax = ec68( ryoukin,cons_tax_ratio );						/*　消費税額　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	switch( ctyp_flg )													/*　料金計算方法により分岐　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		case 1:															/*　税方式が内税方式の場合　　　　　　　　　　*/
			syu_tax = r->tax;											/*　集計用税額に消費税のセット　　　　　　　　*/
			cons_tax = 0;												/*　集計用税額に消費税のセット　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		case 4:															/*　割引対象額が課税後の場合　　　　　　　　　*/
			ryoukin = ryoukin;											/*　料金＝駐車料金＋駐車料金消費税　　　　　　*/
			syu_tax = r->tax;											/*　集計用税額に消費税のセット　　　　　　　　*/
			cons_tax = 0;												/*　集計用税額に消費税のセット　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		default:														/*　通常外税　　　　　　　　　　　　　　　　　*/
			syu_tax = 0;												/*　消費税＝駐車料金消費税　　　　　　　　　　*/
			cons_tax = r->tax;											/*　消費税＝駐車料金消費税　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	req_p->data[2] = cons_tax;											/*　消費税額をセット　　　　　　　　　　　　　*/
	req_p->data[3] = ryoukin;											/*　駐車料金をセット　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( base_ryo == 0 )													/*　料金が０の場合　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		cons_tax = 0;													/*　消費税額クリア　　　　　　　　　　　　　　*/
		tol_dis	 = 0;													/*　割引額合計クリア　　　　　　　　　　　　　*/
		discount = 0;													/*　割引額クリア　　　　　　　　　　　　　　　*/
		base_ryo = 0;													/*　基本料金クリア　　　　　　　　　　　　　　*/
		percent	 = 0;													/*　％割引率クリア　　　　　　　　　　　　　　*/
		jik_dtm	 = 0;													/*　時間割引処理用割引時間クリア　　　　　　　*/
		jik_flg	 = NG;													/*　時間割引処理ｆｌｇクリア　　　　　　　　　*/
		rag_flg  = OFF;													/*　ラグタイムｆｌｇ　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/**********************************************/
