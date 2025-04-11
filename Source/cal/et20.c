/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：駐車料金額算出　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et20(par1)　　　　　　　　：　与えられた料金の取りかたにより、駐車料金の算出を行う。　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　料金の取りかた　昼夜帯　　　逓減帯　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　０：基本　　０／１　：第１パターン　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　１：追加　　２～１２：第２～１２パターン　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
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
#include	"prm_tbl.h"													// 
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
short	midni_ok = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	et20( 															/*　駐車料金計算処理　　　　　　　　　　　　　*/
short	ryo_pat	)														/*　料金の取りかた（昼夜帯、逓減帯別）　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	struct	CAR_TIM		wok_in;											/*　ｗｏｒｋ時刻　　　　　　　　　　　　　　　*/
	short	flg = 1;													/*　ループｆｌｇ　　　　　　　　　　　　　　　*/
	short	ret = 0;													/*　リタンコード　　　　　　　　　　　　　　　*/
	ulong	in_cal_tm;				// MH702200-T00 料金計算EG修正 09/07/22	// 現在までの経過時間格納用変数
	short	wk_calcnt;				// MH702200-T00 料金計算EG修正 09/07/22	// 
	char	FeeMaxType;													// 最大料金タイプ
	short	nmax_tim1;													// 
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	memcpy( &wok_in,&car_in ,7 );										/*　入庫時刻をｗｏｒｋ時刻にコピーする　　　　*/
	ryoukin = 0;														/*　料金クリア　　　　　　　　　　　　　　　　*/
	total = 0;															/*　駐車料金集計用変数クリア　　　　　　　　　*/
	car_type = req_rkn.syubt;											/*　計算要求テーブルの車種を参照する　　　　　*/
	in_time = (short)car_in.hour * 60 + (short)car_in.min;				/*　入庫時刻を分で表現　　　　　　　　　　　　*/
	ot_time = (short)car_ot.hour * 60 + (short)car_ot.min;				/*　入庫時刻を分で表現　　　　　　　　　　　　*/
	memcpy( (char *)&cal_tm,(char *)&car_in,7 );						/*　入庫時刻を計算時刻とする　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	wk_h24t   = in_time;												/*▲入庫　　　　　　　　　　　　　　　　　　　*/
	h24time   = 0;														/*　　　　　　　　　　　　　　　　　　　　　　*/
	befr_flag = OFF;													/*　今日の体系をセーブフラグセット　　　　　　*/
	rmax_cnt = 0 ;														/*　最大料金到達回数　　　　　　　　　　　　　*/
	rmax_ptn = 0 ;														/*　最大料金到達時パタン　　　　　　　　　　　*/
	strt_ptn  = ryo_pat;												/*　開始料金パタンセット　　　　　　　　　　　*/
	ptn_ccnt  = 0;														/*　現在単位係数カウンタクリア　　　　　　　　*/
	cycl_ccnt = 0;														/*　現在繰り返し回数カウンタクリア　　　　　　*/
	sitei_flg = 0;														/*　指定時刻日毎最大フラグ　　　　　　　　　　*/
	amr_flg = 0;
	n_hour_style = 0;													/*　ｎ時間最大の最大料金を取る料金体系　　　　*/
	n_hour_cnt = 0;														/*　ｎ時間最大ｍ回の適用回数　　　　　　　　　*/
	ntnet_nmax_flg = 0;													// 
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	nmax_style	= 0;													// ｎ時間最大の最大料金を取る料金体系
	in_calcnt	= 0;													// 
	nmax_cnt	= 0;													// ｎ時間最大料金徴収カウント
	n_itei_adj	= 0;													// 一定料金後基準時刻調整
	nmax_amari	= 0;													// 
	nmax_amari2 = 0;													// ｎ２の余り時間
																		// 
	FeeMaxType = sp_mx;													// 最大料金タイプ
	nmax_tim1 = NMAX_HOUR1;												// ｎ時間最大(外)時間
	if( EXP_FeeCal == 1 ){												// 料金種別
		FeeMaxType = sp_mx_exp[car_type-1];								// 
																		// 
		nmax_tim1 = EXP_NMAX_HOUR1[car_type-1];							// 
	}																	// 
																		// 
	if( (FeeMaxType == SP_MX_N_MH_NEW)||(FeeMaxType == SP_MX_N_MHOUR_W) ){	// Ｎ時間最大
		memcpy( &nmax_in,	&org_in, 7 );								// 実際の入庫時刻をＮ時間基点時刻バッファへ格納
		switch( nmax_tim_disc ){										// 設定：Ｎ時間最大の基準時刻を
																		//       時間割引分入庫時刻を進める／進めない
		case 0:															// 割引後の入庫時刻を基準とする
			memcpy( &nmax_in, &car_in, 7 );								// Ｎ時間最大基準時刻を進める
			break;														// 
		case 1:															// 実際の入庫時刻を基準とする
			in_cal_tm = ec71a( &org_in, &car_in );						// 実際の入庫時刻から何分経過したか算出する
			wk_calcnt = (short)(in_cal_tm/nmax_tim1);					// 何回ｎ時間に到達したか求める
			if( nmax_countup != 1 ){									// 最大を越えた時にｶｳﾝﾄする
				nmax_cnt = (wk_calcnt - in_calcnt);						// 最大料金領収回数分ｶｳﾝﾄｱｯﾌﾟ
			}															// 
			in_calcnt = wk_calcnt;										// 割引時間内のN時間回数の更新
			break;														// 
		}																// 
	}																	// 
	ret = ec64( &car_ot,&car_in );										/*　年月日時分比較処理　　　　　　　　　　　　*/
	if( ret == -1 )														/*　サービスタイム外出庫の場合　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		while( flg )													/*　計算時刻が出庫時刻に達するまで　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( cal_type == 0 )											/*　料金計算方式が０の場合　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				et2100( ryo_pat );										/*　昼夜帯方式料金計算処理　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　料金計算方式が１の場合　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				et2200( ryo_pat );										/*　逓減帯方式料金計算処理　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			befr_style = style;											/*　今日の体系をセーブ　　　　　　　　　　　　*/
			befr_flag = ON;												/*　今日の体系をセーブフラグセット　　　　　　*/
			if( (FeeMaxType == SP_MX_INTIME)||							// 
				(FeeMaxType == SP_MX_612HOUR)||							// 
				(FeeMaxType == SP_MX_N_HOUR) ) 							// 
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				h24time = h24time + (chk_time - wk_h24t);				/*　現在までの計算時間トータル算出　　　　　　*/
				wk_h24t = (short)chk_tim.hour * 60 + (short)chk_tim.min;/*　現在までの計算時間　　　　　　　　　　　　*/
				if( FeeMaxType == SP_MX_612HOUR ){						/*　ＩＮ＋Ｎ時間最大料金ＣＨＥＣＫなら　　　　*/
					in_time = in_time % T_DAY;							/* 日毎最大基点時刻更新						  */
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( FeeMaxType == SP_MX_NON )								// 
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				total = ryoukin + total;								/*　料金を集計中の駐車料金に加算　　　　　　　*/
				ryoukin = 0;											/*　料金を０クリア　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
/*　絶対最大料金ＣＨＥＣＫ */											/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( total >= abs_max[style-1][car_type-1] )					/*　集計中の駐車料金が絶対最大料金に達した場合*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				total = abs_max[style-1][car_type-1];					/*　絶対最大料金を駐車料金とする　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ec64( &cal_tm,&car_ot ) != 1 )							/*　出庫時刻に達している場合　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( FeeMaxType != SP_MX_NON )							/*　日毎最大料金ＣＨＥＣＫなしなら　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					total = ryoukin + total;							/*　料金を集計中の駐車料金に加算　　　　　　　*/
																		/*　絶対最大料金ＣＨＥＣＫ　　　　　　　　　　*/
					if( total >= abs_max[style-1][car_type-1] )			/*　集計中の駐車料金が絶対最大料金に達した場合*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						total = abs_max[style-1][car_type-1];			/*　絶対最大料金を駐車料金とする　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				ryoukin = total;										/*　集計中の駐車料金の値を料金に代入　　　　　*/
				break;													/*　while ループを抜ける　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			WACDOG;// RX630はWDGストップが出来ない仕様のためWDGリセットにする
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	memcpy( &car_in,&wok_in,7 );										/*　入庫時刻を本来の時刻に戻す　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	midni_ok = 0;														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
