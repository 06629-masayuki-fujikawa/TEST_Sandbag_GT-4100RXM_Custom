/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：駐車料金計算処理　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et40()　　　　　　　　　　：　サービスタイム・グレースタイムを考慮し、駐車料金計算を行う。：　*/
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
#include	"lkmain.h"

#if (1 == AUTO_PAYMENT_PROGRAM)
static void Debug_Cal( void );
#endif
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	et40()															/*　駐車料金計算処理　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	ret = 0;													/*　リタンコード　　　　　　　　　　　　　　　*/
	short	ryo_pat = 0;												/*　料金の取りかた（昼夜帯、逓減帯別）　　　　*/
	short	yuko_strt;													/*　時間割引有効開始時刻　　　　　　　　　　　*/
	short	yuko_end;													/*　時間割引有効終了時刻　　　　　　　　　　　*/
	short	yuko_tm;													/*　時間割引有効時刻　　　　　　　　　　　　　*/
	short	taik = 0;													/*　体系　　　　　　　　　　　　　　　　　　　*/
	char	prc = NG;													/*　処理可・不可ｆｌｇ　　　　　　　　　　　　*/
	long	jik_new = 0l;												/*　時間割引後入庫時間　　　　　　　　　　　　*/
	struct	REQ_RKN		*req_p;											/*　現在計算後格納エリア　　　　　　　　　　　*/
	struct	CAR_TIM		wok_tm;											/*　時刻構造体　ｗｏｒｋ時刻　　　　　　　　　*/
	short	ret_w = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	in_m = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	ot_m = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	req_p = &req_rhs[tb_number];										/*　現在料金計算要求ポインター　　　　　　　　*/
	memmove( req_p,&req_rkn,sizeof(req_rkn) );							/*　現在料金計算要求TBLを計算結果TBLへセット　*/
	memset( &ryo_buf_n,0,sizeof(ryo_buf_n) );							/*　駐車料金データバッファクリア　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	memcpy( &car_in,&carin_mt,7 );										/*　入庫時刻を基本入庫時刻にセット　　　　　　*/
	memcpy( &car_ot,&carot_mt,7 );										/*　出庫時刻を基本出庫時刻にセット　　　　　　*/
	jik_dtm = (short)req_rkn.data[0];									/*　種別割引ｆｌｇ　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	discount = 0;														/*　割引額＝０　　　　　　　　　　　　　　　　*/
	ryoukin = 0;														/*　料金＝０　　　　　　　　　　　　　　　　　*/
	cons_tax = 0;														/*　税金＝０　　　　　　　　　　　　　　　　　*/
	syu_tax = 0;														/*　集計用税金＝０　　　　　　　　　　　　　　*/
	svcd_dat = 0;														/*　サービス券データ　　　　　　　　　　　　　*/
	fun_kti = OFF;														/*　紛失料金は固定料金ｆｌｇ　　　　　　　　　*/
	befr_style = 0;														/*　前日の体系　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( vl_now != V_CHG )												/*　精算後券以外		　　　　　　　　　　　*/
		ec70( &car_in,(long)se_svt.stim );								/*　日付更新処理（設定サービスタイム分）　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	ret = ec64( &carot_mt,&car_in );									/*　年月日時分比較処理　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( ret == -1 )														/*　サービスタイム外出庫の場合　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		ec66( &carot_mt,(long)se_svt.gtim );							// 日付更新処理（出庫時刻をグレースタイム分）
		ret = ec64( &carin_mt,&carot_mt );								/*　年月日時分比較処理　　　　　　　　　　　　*/
		if( ret != 1 )													/*　入庫時刻≧出庫時刻の場合　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			memcpy( &car_ot,&carin_mt,7 );								/*　入庫時刻を基本入庫時刻とする　　　　　　　*/
			ec70( &car_ot,(long)1 );									/*　日付更新処理（料金を１単位分取るため）　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　入庫時刻＜出庫時刻の場合　　　　　　　　　*/
			memcpy( &car_ot,&carot_mt,7 );								/*　出庫時刻を基本出庫時刻とする　　　　　　　*/
		memcpy( &car_in,&carin_mt,7 );									/*　入庫時刻を基本入庫時刻とする　　　　　　　*/
		if( ryo_buf.lag_tim_over ){										/*　ラグタイムオーバー後は　　　　　　　　　　*/
			ryo_pat = (short)Carkind_Param(LAG_PAY_PTN, (char)LockInfo[ryo_buf.pkiti - 1].ryo_syu, 2,0);	/*　料金の取り方参照　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		et20( ryo_pat );												/*　料金算出処理を行う　　　　　　　　　　　　*/
		base_ryo = ryoukin;												/*　駐車料金（基本型）のセット　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　サービスタイム内出庫の場合　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		ryo_buf.svs_tim = 1;											/*　サービスタイム内精算有り　　　　　　　　　*/
		base_ryo = 0;													/*　駐車料金（基本型）のセット　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
// MH810100(S) S.Takahashi 2020/03/18 自動連続精算
// 料金は問合せ結果の入庫時刻から求める
//#if (1 == AUTO_PAYMENT_PROGRAM)
//	if( CPrmSS[S_SYS][8] && ret == 0 ){									/*　デバッグ処理有効かつ同時刻精算の場合　　　*/
//		Debug_Cal();
//	}
//#endif
// MH810100(E) S.Takahashi 2020/03/18 自動連続精算
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( jik_dtm != 0 )													/*　種別割引がある場合　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		in_time = (short)carin_mt.hour * 60 + (short)carin_mt.min;		/*　基本入庫時刻を分で表現　　　　　　　　　　*/
		taik = ec61( &carin_mt );										/*　基本入庫時刻の体系算出処理　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( in_time < ta_st_sel((char)taik,0) )							// 体系の開始時刻より前の場合
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			memcpy( &wok_tm,&carin_mt,7 );								/*　　　　　　　　　　　　　　　　　　　　　　*/
			ec62( &wok_tm );											/*　ｗｏｒｋ時刻を前日とする　　　　　　　　　*/
			taik = ec61( &wok_tm );										/*　ｗｏｒｋ時刻の体系算出処理　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( cal_type == 0 )											/*　料金計算方式　０：昼夜帯方式　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( ovl_time[taik-1][0] != 0 )							/*　体系切替による重複帯があれば　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					memcpy( &wok_tm,&carin_mt,7 );						/*　　　　　　　　　　　　　　　　　　　　　　*/
					wok_tm.hour = (char)( ta_st_sel((char)taik,0)/60 );	// 体系開始年月日を求める
					wok_tm.min = (char)( ta_st_sel((char)taik,0)%60 );	// 
					ec70( &wok_tm,(long)ovl_time[taik-1][0] );			/*　重複時間を加算する　　　　　　　　　　　　*/
					ret_w = ec64( &carot_mt,&wok_tm );					/*　出庫時刻と重複帯の時刻比較　　　　　　　　*/
					if( ret_w != -1 )									/*　重複帯内出庫の場合　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						if( ovl_type[taik-1][0] == 1 )					/*　重複方式が１の場合　　　　　　　　　　　　*/
						{												/*　　　　　　　　　　　　　　　　　　　　　　*/
							memcpy( &wok_tm,&carin_mt,7 );				/*　　　　　　　　　　　　　　　　　　　　　　*/
							ec62( &wok_tm );							/*　ｗｏｒｋ時刻を前日とする　　　　　　　　　*/
							taik = ec61( &wok_tm );						/*　ｗｏｒｋ時刻の体系算出処理　　　　　　　　*/
						}												/*　　　　　　　　　　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( jitu_wari == 1 ){											/* 0=実割引としない/1=実割引とする			*/
			if( org_ot.mon == org_in.mon && org_ot.day == org_in.day ){	/*　　　　　　　　　　　　　　　　　　　　　　*/
				in_m = (short)org_in.hour * 60 + (short)org_in.min;		/*　基本入庫時刻を分で表現　　　　　　　　　　*/
				ot_m = (short)org_ot.hour * 60 + (short)org_ot.min;		/*　基本出庫時刻を分で表現　　　　　　　　　　*/
				if( jik_dtm >= ( ot_m - in_m ))							/*　　　　　　　　　　　　　　　　　　　　　　*/
					jik_dtm = ot_m - in_m;								/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		yuko_strt = ji_st[taik-1];										/*　時間割引の有効開始時刻　　　　　　　　　　*/
		yuko_end = ji_ed[taik-1];										/*　時間割引の有効終了時刻　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(( yuko_strt > yuko_end )&&( in_time < yuko_end ))			/*　有効開始時刻が有効終了時刻以降の場合　　　*/
		{																/*　入庫時刻が有効終了時刻より前の場合　　　　*/
			prc = OK;													/*　処理ＯＫ　　　　　　　　　　　　　　　　　*/
			yuko_tm = yuko_end - in_time;								/*　有効時間＝有効終了時間－有効開始時間　　　*/
			if( jik_dtm > yuko_tm )										/*　割引時間＞有効時間の場合　　　　　　　　　*/
				jik_dtm = yuko_tm;										/*　有効時間が割引時間となる　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( yuko_strt > yuko_end )									/*　有効開始時刻が有効終了時刻以降の場合　　　*/
				yuko_end = yuko_end + 1440;								/*　有効終了時刻に24時間加算　　　　　　　　　*/
			yuko_tm = yuko_end - yuko_strt;								/*　有効時間＝有効終了時間－有効開始時間　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( yuko_tm == 0 )											/*　有効開始時刻＝有効終了時刻の場合　　　　　*/
				prc = OK;												/*　処理ＯＫ　　　　　　　　　　　　　　　　　*/
			else														/*　有効開始時刻≠有効終了時刻の場合　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if(( in_time >= yuko_strt )&&( in_time < yuko_end ))	/*　有効時間内の場合　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					prc = OK;											/*　処理ＯＫ　　　　　　　　　　　　　　　　　*/
					if( jik_dtm > yuko_tm )								/*　割引時間＞有効時間の場合　　　　　　　　　*/
						jik_dtm = yuko_tm;								/*　有効時間が割引時間となる　　　　　　　　　*/
					jik_new = (long)in_time + jik_dtm;					/*　時間割引後入庫時間の算出　　　　　　　　　*/
					if( (long)yuko_end < jik_new )						/*　有効終了時刻より割引後時刻が大きい場合　　*/
						jik_dtm = yuko_end - in_time;					/*　割引時間は、入庫時刻～有効終了時刻　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( prc == OK )													/*　処理ＯＫの場合　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			discount = 0;												/*　割引額＝０　　　　　　　　　　　　　　　　*/
			discount = et421();											/*　通常時間割引額算出　　　　　　　　　　　　*/
			base_ryo = base_ryo - discount;								/*　　　　　　　　　　　　　　　　　　　　　　*/
			ryoukin = base_ryo;											/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( base_ryo <= 0 )											/*　料金≦０の場合　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				ryoukin = 0;											/*　料金＝０　　　　　　　　　　　　　　　　　*/
				base_ryo = 0;											/*　料金＝０　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			memcpy(&recalc_carin,&car_in,7);
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　処理しない場合　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			jik_dtm = 0;												/*　種別割引時間のクリア　　　　　　　　　　　*/
			discount = 0;												/*　割引額＝０　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	su_jik_dtm = jik_dtm;												/*　種別割引時間　　　　　　　　　　　　　　　*/
// GG129000(S) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
// 	PayData.SyuWariRyo = discount;										/*  種別割引料金							  */
// GG129000(E) R.Endo 2023/05/31 車番チケットレス4.1 #7029 割引9種使用しQR買上券を読んだ場合に精算ログから買物金額合計が欠落する
	su_jik_plus = 0;
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	ryo_buf_n.ryo = ryoukin;											/*　駐車料金のセット　　　　　　　　　　　　　*/
	cons_tax = 0;														/*　消費税＝０　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( cons_tax_ratio != 0 )											/*　消費税率が０％ではない場合　　　　　　　　*/
		ryo_buf_n.tax = ec68( ryoukin,cons_tax_ratio );					/*　消費税額　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	switch( ctyp_flg )													/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		case 1:															/*　税方式が内税方式の場合　　　　　　　　　　*/
			syu_tax = ryo_buf_n.tax;									/*　集計用税額に消費税のセット　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		case 4:															/*　割引対象額が課税後の場合　　　　　　　　　*/
			syu_tax = ryo_buf_n.tax;									/*　集計用税額に消費税のセット　　　　　　　　*/
			cons_tax = ryo_buf_n.tax;									/*　消費税＝駐車料金消費税　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		default:														/*　通常外税　　　　　　　　　　　　　　　　　*/
			syu_tax = ryo_buf_n.tax;									/*　集計用税額に消費税のセット　　　　　　　　*/
			cons_tax = ryo_buf_n.tax;									/*　消費税＝駐車料金消費税　　　　　　　　　　*/
		break;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	ryo_buf_n.ryo = base_ryo;											/*　料金バッファに基本料金セット　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	req_p->data[2] = discount;											/*　割引額をセット　　　　　　　　　　　　　　*/
	req_p->data[3] = ryoukin;											/*　割引後額をセット　　　　　　　　　　　　　*/
	req_p->data[4] = cons_tax;											/*　消費税額をセット　　　　　　　　　　　　　*/
}																		/**********************************************/

/*[]----------------------------------------------------------------------[]*/
/*| メカ評価用 デバッグ料金計算処理		                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Debug_Cal			                                   |*/
/*| PARAMETER    : void							                           |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Namioka                                               |*/
/*| Date         : 2009-08-06                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]*/
void	Debug_Cal( void )
{
	char	wstyle;
	char	wband = 1;
	struct	CAR_TIM		wcal_tm;
	short	wcal_time,i;
	
	
	memcpy( (char *)&wcal_tm,(char *)&car_in,7 );					/*　入庫時刻を計算時刻とする　　　　　　　　　*/
	wstyle = ec61( &wcal_tm );
	wcal_time = (short)wcal_tm.hour*60+(short)wcal_tm.min;
	
	if( max_bnd[wstyle-1] == 1)										/*◎5/22　　　　　　　　　　　　　　　　　　　*/
	{																/*◎　　　　　　　　　　　　　　　　　　　　　*/
		wband = 1;													/*◎　　　　　　　　　　　　　　　　　　　　　*/
	}																/*◎　　　　　　　　　　　　　　　　　　　　　*/
	else															/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																/*　　　　　　　　　　　　　　　　　　　　　　*/
		for( i = 0 ; i < (max_bnd[wstyle-1]*2)-1 ; i++ )			/*　第１料金帯から予備料金帯の分までループ　　*/
		{															/*　　　　　　　　　　　　　　　　　　　　　　*/
			if(( wcal_time >= bnd_strt[wstyle-1][i] )&&				/*　計算時刻がその料金帯内の場合　　　　　　　*/
				( wcal_time < bnd_strt[wstyle-1][i+1] ))			/*　　　　　　　　　　　　　　　　　　　　　　*/
			{														/*　　　　　　　　　　　　　　　　　　　　　　*/
				wband = (char)(i+1);								/*　ループカウントを料金帯とする　　　　　　　*/
				break;												/*　ループを抜ける　　　　　　　　　　　　　　*/
			}														/*　　　　　　　　　　　　　　　　　　　　　　*/
		}															/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																/*　　　　　　　　　　　　　　　　　　　　　　*/

	ryoukin = (unsigned long)fee_std[wstyle-1][req_rkn.syubt-1][wband - max_bnd[wstyle-1]];	/*　基本料金（一定料金）を求める　　　　　　　*/
	ryoukin *= 10;													/*　追加料金を１０倍する　　　　　　　　　　　*/
	base_ryo = ryoukin;												/*　駐車料金（基本型）のセット　　　　　　　　*/

}
