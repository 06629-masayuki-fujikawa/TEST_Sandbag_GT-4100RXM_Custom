/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：設定項目一括参照　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：mc10()　　　　　　　　　　：　イニシャル時、及び設定後に起動し、設定項目の一括参照を行う。：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　無し　　　　　　　　　　　　無し　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：  */
/**********************************************************************************************************************/
																		/**********************************************/
#include	"system.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"mem_def.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"rkn_def.h"													/*　全デファイン統括　　　　　　　　　　　　　*/
#include	"rkn_cal.h"													/*　料金関連データ　　　　　　　　　　　　　　*/
#include	"rkn_fun.h"													/*　全サブルーチン宣言　　　　　　　　　　　　*/
#include	"prm_tbl.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/*====================================================================================================================*/
void	mc10()															/*　設定項目一括参照　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	i;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	j;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	k;															/*　　　　　　　　　　　　　　　　　　　　　　*/
	long	work;														/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	prm_wk;														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	sp_op		= (char)CPrmSS[S_CAL][15];								/*　設定項目より余り処理方式参照　　　　　　　*/
	sp_it		= (char)CPrmSS[S_CAL][16];								/*　設定項目より一定料金帯余り処理方式参照　　*/
	sp_mx		= (char)CPrmSS[S_CAL][2];								/*　日毎最大料金方式参照　　　　　　　　　　　*/
	switch( sp_mx ){
		case 2:		// 時刻指定日毎最大
		case 3:		// ２種時刻指定日毎最大
		case 5:		// 時間帯最大
		case 10:	// ｎ時間ｍ回最大
		case 20:	// Ｗ上限
			break;

		default:
			sp_mx = 0;	// 上記以外は最大なしで動作する。
			break;
	}

	for( i=0; i<12; i++ ){												// 
		sp_mx_exp[i] = 													// 
			(char)prm_get( COM_PRM, S_CLX, (short)(2+i/3), 2, (char)(5-(i%3*2)) );	// 料金種別毎の最大料金タイプ設定

		switch( sp_mx_exp[i] ){
			case 2:		// 時刻指定日毎最大
			case 3:		// ２種時刻指定日毎最大
			case 5:		// 時間帯最大
			case 10:	// ｎ時間ｍ回最大
			case 20:	// Ｗ上限
				break;
			default:
				sp_mx_exp[i] = 0;	// 上記以外は最大なしで動作する。
				break;
		}
	}																	// 
	EXP_FeeCal = (char)CPrmSS[S_CLX][1];								// 料金計算拡張の有無
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	version.kind	= 3;												/*??ＴＭ　モデルＮｏ．　１ー４　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	switch ( version.kind )												/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	case  2 :															/*　　　　　　　　　　　　　　　　　　　　　　*/
	case  3 :															/*　　　　　　　　　　　　　　　　　　　　　　*/
		break ;															/*--------------------------------------------*/
	default :															/*　設定タイプが２／３／４以外の場合　　　　　*/
		version.kind	= 2	;											/*??ＴＭ　モデルＮｏ．２　　　　　　　　　　　*/
		break ;															/*--------------------------------------------*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	/*----------------------------------------------------------------------------------------------------------------*/
	/*　料金計算方式																								  */
	/*----------------------------------------------------------------------------------------------------------------*/
	if ( CPrmSS[S_CAL][1] != 1 )										/*　昼夜帯　設定参照処理　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		cal_type = 0 ;													/*　　　　　　　　　　　　　　　　　　　　　　*/
		mc101()	;														/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　逓減帯　設定参照処理　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		cal_type = 1 ;													/*　　　　　　　　　　　　　　　　　　　　　　*/
		mc102()	;														/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	for ( i = 0 ; i < 3 ; i ++ )										/*　第1～3料金体系　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for ( j = 0 ; j < 12 ; j ++ )									/*　車種A～L　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			// 第1～第3料金体系
			abs_max[i][j] = CPrmSS[S_RAT][300*i+61+20*j];				/*　車種毎絶対最大料金(000000～999990円)　　　*/
			day_max1[i][j] = CPrmSS[S_RAT][300*i+62+20*j];				/*　車種毎 日毎最大料金1(000000～999990円)　　*/
			// 第4～第6料金体系
			abs_max[i+3][j] = CPrmSS[S_CLA][RYO_TAIKEI_SETCNT*i+61+RYO_SYUBET_SETCNT*j];	/*　車種毎絶対最大料金(000000～999990円)　　　*/
			day_max1[i+3][j] = CPrmSS[S_CLA][RYO_TAIKEI_SETCNT*i+62+RYO_SYUBET_SETCNT*j];	/*　車種毎 日毎最大料金1(000000～999990円)　　*/
			// 第7～第9料金体系
			abs_max[i+6][j] = CPrmSS[S_CLB][RYO_TAIKEI_SETCNT*i+61+RYO_SYUBET_SETCNT*j];	/*　車種毎絶対最大料金(000000～999990円)　　　*/
			day_max1[i+6][j] = CPrmSS[S_CLB][RYO_TAIKEI_SETCNT*i+62+RYO_SYUBET_SETCNT*j];	/*　車種毎 日毎最大料金1(000000～999990円)　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	for ( i = 0 ; i < 3 ; i ++ )										/*　第1～3料金体系　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for ( j = 0 ; j < 12 ; j ++ )									/*　車種A～L　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			for ( k = 0 ; k < 6 ; k ++ ){								/*　料金帯1～6　　　　　　　　　　　　　　　　*/
				if( k == 5 ){											/*　　　　　　　　　　　　　　　　　　　　　　*/
					band_max[k][i][j] = CPrmSS[S_RAT][RYO_TAIKEI_SETCNT*i+63+RYO_SYUBET_SETCNT*j];	/*　料金帯最大　　　　　　　　　　　　　　　　*/
					band_max[k][i+3][j] = CPrmSS[S_CLA][RYO_TAIKEI_SETCNT*i+63+RYO_SYUBET_SETCNT*j];/*　料金帯最大　　　　　　　　　　　　　　　　*/
					band_max[k][i+6][j] = CPrmSS[S_CLB][RYO_TAIKEI_SETCNT*i+63+RYO_SYUBET_SETCNT*j];/*　料金帯最大　　　　　　　　　　　　　　　　*/
				}else{													/*　　　　　　　　　　　　　　　　　　　　　　*/
					band_max[k][i][j] = CPrmSS[S_RAT][RYO_TAIKEI_SETCNT*i+76+RYO_SYUBET_SETCNT*j+k];	/*　料金帯最大　　　　　　　　　　　　　　　　*/
					band_max[k][i+3][j] = CPrmSS[S_CLA][RYO_TAIKEI_SETCNT*i+76+RYO_SYUBET_SETCNT*j+k];	/*　料金帯最大　　　　　　　　　　　　　　　　*/
					band_max[k][i+6][j] = CPrmSS[S_CLB][RYO_TAIKEI_SETCNT*i+76+RYO_SYUBET_SETCNT*j+k];	/*　料金帯最大　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	WACDOG;																//

																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	for ( i = 0 ; i < TKI_MAX ; i ++ )									/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		ps_type[i] = (short)CPrmSS[S_PAS][1+10*i];						/*　定期使用目的の算出　ｉ＝定期種別１～８　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	cons_tax_ratio = (short)CPrmSS[S_CAL][19];							/*　消費税率算出　　　　　　　　　　　　　　　*/
	dis_tax = (short)CPrmSS[S_CAL][27];									/*　割引額の消費税計算方法参照　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	switch ( prm_get( COM_PRM,S_CAL,20,1,1 ) )							/*　消費税の取り方により分岐　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		case 0:															/*　内税の場合　　　　　　　　　　　　　　　　*/
		case 1:															/*　　　　　　　　　　　　　　　　　　　　　　*/
			work = (long)cons_tax_ratio;								/*　　　　　　　　　　　　　　　　　　　　　　*/
			cons_tax_ratio = (short)(( work * 10000 ) / ( 10000 + work ));/*　消費税率算出　　　　　　　　　　　　　　*/
			ctyp_flg = 1;												/*　　　　　　　　　　　　　　　　　　　　　　*/
			break;														/*　　　　　　　　　　　　　　　　　　　　　　*/
		case 2:															/*　外税の場合　　　　　　　　　　　　　　　　*/
		case 3:															/*　　　　　　　　　　　　　　　　　　　　　　*/
			switch( CPrmSS[S_CAL][27] )									/*　割引額の消費税計算方法　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
			case 0:														/*　割引き対象が課税前の場合　　　　　　　　　*/
				switch( CPrmSS[S_CAL][28] )								/*　プリペイドカードの支払方法　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					case 1:		ctyp_flg = 2;		break;				/*　ＰＰ支払対象が課税前の場合　　　　　　　　*/
					case 0:		ctyp_flg = 3;		break;				/*　ＰＰ支払対象が課税後の場合　　　　　　　　*/
					default:	ctyp_flg = 0;		break;				/*　ＰＰ支払対象が設定外の場合　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				break;													/*　　　　　　　　　　　　　　　　　　　　　　*/
			case 1:				ctyp_flg = 4;		break;				/*　割引き対象が課税後の場合　　　　　　　　　*/
			default:			ctyp_flg = 0;		break;				/*　割引対象額が設定外の場合　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			break;														/*　　　　　　　　　　　　　　　　　　　　　　*/
		default:				ctyp_flg = 0;		break;				/*　内・外税の設定が範囲外の場合　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	for( i = 0; i < 3; i++ )											/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	// 第1～第3料金体系
		ji_st[i] = (short)prm_tim( 0, S_RAT, (short)(1+300*i) );		/*　種別時間割引の有効開始時刻　　　　　　　　*/
		ji_ed[i] = (short)prm_tim( 0, S_RAT, (short)(2+300*i) );		/*　種別時間割引の有効終了時刻　　　　　　　　*/
	// 第4～第6料金体系
		ji_st[i+3] = (short)prm_tim( 0, S_CLA, (short)(1+RYO_TAIKEI_SETCNT*i) );		/*　種別時間割引の有効開始時刻　　　　　　　　*/
		ji_ed[i+3] = (short)prm_tim( 0, S_CLA, (short)(2+RYO_TAIKEI_SETCNT*i) );		/*　種別時間割引の有効終了時刻　　　　　　　　*/
	// 第7～第9料金体系
		ji_st[i+6] = (short)prm_tim( 0, S_CLB, (short)(1+RYO_TAIKEI_SETCNT*i) );		/*　種別時間割引の有効開始時刻　　　　　　　　*/
		ji_ed[i+6] = (short)prm_tim( 0, S_CLB, (short)(2+RYO_TAIKEI_SETCNT*i) );		/*　種別時間割引の有効終了時刻　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/

																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	for( i = 0; i < 3; i++ )											/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for( j = 0; j < TKI_MAX; j++ )									/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			ps_st[i][j] =												/*　定期開始時刻　　　　　　　　　　　　　　　*/
					(short)prm_tim( 0,S_PTM,(short)((1+30*i)+(2*j)));	/*　　　　　　　　　　　　　　　　　　　　　　*/
			ps_ed[i][j] =												/*　定期終了時刻　　　　　　　　　　　　　　　*/
					(short)prm_tim( 0,S_PTM,(short)((2+30*i)+(2*j)));	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ps_st[i][j] < ps_ed[i][j] )								/*　定期開始＜定期終了　　　　　　　　　　　　*/
				ps_tm[i][j] = ps_ed[i][j] - ps_st[i][j];				/*　定期時間＝定期終了－定期開始　　　　　　　*/
			else														/*　定期開始≧定期終了　　　　　　　　　　　　*/
				ps_tm[i][j] = ps_ed[i][j] + 1440 - ps_st[i][j];			/*　定期時間＝定期終了＋１４４０－定期開始　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		ta_st[i] = (short)prm_tim( 0,S_RAT,(short)(4+300*i) );			/*　体系開始時刻を分換算する　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	for( i = 0; i < 6; i++ )											/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for( j = 0; j < TKI_MAX; j++ )									/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			ps_st[i+3][j] =												/*　定期開始時刻　　　　　　　　　　　　　　　*/
					(short)prm_tim( 0,S_CLC,(short)((1+30*i)+(2*j)));	/*　　　　　　　　　　　　　　　　　　　　　　*/
			ps_ed[i+3][j] =												/*　定期終了時刻　　　　　　　　　　　　　　　*/
					(short)prm_tim( 0,S_CLC,(short)((2+30*i)+(2*j)));	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ps_st[i+3][j] < ps_ed[i+3][j] )							/*　定期開始＜定期終了　　　　　　　　　　　　*/
				ps_tm[i+3][j] = ps_ed[i+3][j] - ps_st[i+3][j];			/*　定期時間＝定期終了－定期開始　　　　　　　*/
			else														/*　定期開始≧定期終了　　　　　　　　　　　　*/
				ps_tm[i+3][j] = ps_ed[i+3][j] + 1440 - ps_st[i+3][j];	/*　定期時間＝定期終了＋１４４０－定期開始　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( i < 3 ){
			ta_st[i+3] = (short)prm_tim( 0,S_CLA,(short)(4+RYO_TAIKEI_SETCNT*i) );	/*　第4～6体系開始時刻を分換算する　　　　　　*/
		}else{
			ta_st[i+3] = (short)prm_tim( 0,S_CLB,(short)(4+RYO_TAIKEI_SETCNT*(i-3)));	/*　第7～9体系開始時刻を分換算する　　　　　　*/
		}
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		// 
	for( i=0; i<FEE_TAIKEI_MAX; i++ ){												// 
		for( j=0; j<TKI_MAX; j++ ){										// 
			ps_sepa[i][j] = 0;											// 設定変更時に前の設定のﾌﾗｸﾞが残らないようｸﾘｱする
																		// 単位時間の余り処理の関係で処理を変更
			if( (ps_st[i][j] < ps_ed[i][j])||							// 0:00を越えない時間帯定期
				((ps_st[i][j] == ps_ed[i][j])&&(!ps_st[i][j]))||		// 0:00ｽﾀｰﾄの全日定期
				(!ps_ed[i][j])){										// 0:00ENDの定期
																		// 
			}else{														// 
				ps_sepa[i][j] = 1;										// 分割ﾌﾗｸﾞをONとする
				ps_st1[i][j] = ps_st[i][j];								// 第1分割 XX:XX～00:00
				ps_ed1[i][j] = ps_st2[i][j] = 0;						// 第2分割 00:00～XX:XX
				ps_ed2[i][j] = ps_ed[i][j];								// 
				ps_tm1[i][j] = 1440 - ps_st1[i][j];						// 第1分割の定期時間
				ps_tm2[i][j] = ps_ed2[i][j];							// 第2分割の定期時間
			}															// 
		}																// 
	}																	// 
																		// 
	/*------------------*/
	/* 料金種別割引時間 */
	/*------------------*/
	for( i=0; i<12; i++ )												/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		prm_wk = (short)prm_get( 0,S_SHA,(short)(1+i*6),4,1 );			/*　　　　　　　　　　　　　　　　　　　　　　*/
		Prm_RateDiscTime[i] = (short)((prm_wk/100)*60 + prm_wk%100);	/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	/*------------------*/
	/* 特別日・特別期間	*/
	/*------------------*/
	for ( i = 0; i < SP_DAY_MAX; i++ )									/*　特別日　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		SP_DAY[i][0] = (uchar)prm_get( 0,S_TOK,(short)(9+i),2,3 );		/*　月　　　　　　　　　　　　　　　　　　　　*/
		SP_DAY[i][1] = (uchar)prm_get( 0,S_TOK,(short)(9+i),2,1 );		/*　日　　　　　　　　　　　　　　　　　　　　*/
		SP_DAY[i][2] = (uchar)prm_get( 0,S_TOK,(short)(9+i),1,6 );		/*　0:ｼﾌﾄしない,1:ｼﾌﾄする 　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	for ( i = 0; i < SP_RANGE_MAX; i++ )								/*　特別期間　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		SP_RANGE[i][0] = (uchar)prm_get( 0,S_TOK,(short)(1+i*2),2,3 );	/*　開始月　　　　　　　　　　　　　　　　　　*/
		SP_RANGE[i][1] = (uchar)prm_get( 0,S_TOK,(short)(1+i*2),2,1 );	/*　開始日　　　　　　　　　　　　　　　　　　*/
		SP_RANGE[i][2] = (uchar)prm_get( 0,S_TOK,(short)(2+i*2),2,3 );	/*　終了月　　　　　　　　　　　　　　　　　　*/
		SP_RANGE[i][3] = (uchar)prm_get( 0,S_TOK,(short)(2+i*2),2,1 );	/*　終了日　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	for ( i = 0; i < (SP_HAPPY_MAX/2); i++ )							/*　ﾊｯﾋﾟｰﾏﾝﾃﾞｰ　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		SP_HAPPY[i*2][0] = (uchar)prm_get( 0,S_TOK,(short)(42+i),2,5 );	/*　月　　　　　　　　　　　　　　　　　　　　*/
		SP_HAPPY[i*2][1] = (uchar)prm_get( 0,S_TOK,(short)(42+i),1,4 );	/*　週　　　　　　　　　　　　　　　　　　　　*/
		SP_HAPPY[i*2+1][0] = (uchar)prm_get( 0,S_TOK,(short)(42+i),2,2 );/*　月　　　　　　　　　　　　　　　　　　　　*/
		SP_HAPPY[i*2+1][1] = (uchar)prm_get( 0,S_TOK,(short)(42+i),1,1 );/*　週　　　　　　　　　　　　　　　　　　　　*/
	}
	for ( i = 0; i < SP_YAER_MAX; i++ )									/*　特別年月日　　　　　　　　　　　　　　　　*/
	{
		SP_YEAR[i][0] = (uchar)prm_get( 0,S_TOK,(short)(50+i),2,5 );	/*　年　　　　　　　　　　　　　　　　　　　　*/
		SP_YEAR[i][1] = (uchar)prm_get( 0,S_TOK,(short)(50+i),2,3 );	/*　月　　　　　　　　　　　　　　　　　　　　*/
		SP_YEAR[i][2] = (uchar)prm_get( 0,S_TOK,(short)(50+i),2,1 );	/*　日					  　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	for ( i = 0; i < (SP_WEEK_MAX); i++ )								/*　特別曜日	　　　　    　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		SP_WEEK[i][0] = (uchar)prm_get( 0,S_TOK,(short)(56+i),2,3 );	/*　月　　　　　　　　　　　　　　　　　　　　*/
		SP_WEEK[i][1] = (uchar)prm_get( 0,S_TOK,(short)(56+i),1,2 );	/*　週　　　　　　　　　　　　　　　　　　　　*/
		SP_WEEK[i][2] = (uchar)prm_get( 0,S_TOK,(short)(56+i),1,1 );	/*　曜日　　　　　　　　　　　　　　　　　　　*/
	}
	
	/*--------------------*/
	/* 頻繁に参照する設定 */
	/*--------------------*/
	jitu_wari = (char)CPrmSS[S_CAL][35];								/*　実割引の場合する/しない 　　　　　　　　　*/
	turi_wari = 0;														/*　割引を釣銭の対象とする/しないは　　　　　 */
																		/*　しない(28-0036=0)固定で動作させる　　　　 */
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		// 
	// Ｎ時間最大時間													// 
	NMAX_HOUR1 = (short)prm_get(COM_PRM, S_CAL, 38, 2, 3) * 60;			// ｎ時間最大1時間（全種別共通）
																		// 
	// 定期曜日切換設定													// 
	pass_week_chg	= (uchar)prm_get(COM_PRM, S_TIK, 12, 1, 1);			// 0=体系に連動
																		// 1=暦(0:00)に連動(定期帯内切り分けあり)
																		// 2=暦(0:00)に連動(定期帯内切り分けなし)
	// Ｎ時間最大基準設定												// 
	nmax_point		= (uchar)prm_get(COM_PRM, S_CAL, 38, 2, 5);			// 0=出庫基準(従来通り)
																		// 1=入庫基準
																		// 2=体系基準(未完のため使用禁止)
	if( EXP_FeeCal == 1 ){												// 
		nmax_point = (uchar)prm_get(COM_PRM, S_CLX, 10, 1, 1);			// 
	}																	// 
	nmax_point		= NMAX_IN_PIT;										// 1=入庫基準で固定
																		// 
	// Ｎ時間最大に一定料金帯を含める									// 
	nmax_itei		= (uchar)prm_get(COM_PRM, S_CAL, 39, 1, 3);			// 0=対象とする／1=対象としない
																		// 
	// 定期帯後のＮ時間最大ﾘｾｯﾄ											// 
	nmax_pass_reset= (uchar)prm_get(COM_PRM, S_CAL, 39, 1, 1);			// 定期帯後の基準時刻をﾘｾｯﾄする
																		// 
	// 定期帯後のＮ時間最大料金											// 
	nmax_pass_calc = (uchar)prm_get(COM_PRM, S_CAL, 39, 1, 2);			// 定期帯後の最大料金は設定に従う
																		// 
	nmax_tim_disc	= (uchar)prm_get(COM_PRM, S_CAL, 39, 1, 5);			// 時間割引後のＮ時間最大の入庫	時刻 0=入庫時刻を進める
																		//                                   1=実際の入庫時刻のまま
																		// 
	// Ｎ時間経過後の単位時間設定										// 
	nmax_tani_type = (uchar)prm_get(COM_PRM, S_CAL, 39, 1, 4);			// 
																		// 
	nmax_itei		= 1;												// N時間最大に一定料金帯を含めない
	nmax_pass_calc	= 1;												// 定期帯後の最大料金は設定に従う
	nmax_tim_disc	= 1;												// 時間割引後のＮ時間最大の入庫時刻は実際の時刻とする
																		// 
	nmax_set1 = (short)prm_get(COM_PRM, S_CAL, 38, 2, 1);				// ｍ回適用 0は∞適用とする
																		// 
	NMAX_HOUR2 = (short)prm_get(COM_PRM, S_CAL, 41, 2, 3) * 60;			// ｎ時間最大2（全種別共通）
																		// 
	if( EXP_FeeCal != 1){												// 料金計算拡張 なし
		if( sp_mx == 10 ){												// ｎ時間最大
			if( (!NMAX_HOUR1) || (NMAX_HOUR1/60 > 24) ){				// ｎ時間範囲ＮＧ
				NMAX_HOUR1 = 24 * 60;									// 強制的に24時間にする
			}															// 
		}																// 
		else if( sp_mx == 20 ){											// Ｗ上限
			if( (!NMAX_HOUR1) || (NMAX_HOUR1/60 > 24) ||				// ｎ１、ｎ２時間範囲ＮＧ
				(!NMAX_HOUR2) || (NMAX_HOUR2/60 > 24) ||				// ｎ１、ｎ２条件ＮＧ
				(NMAX_HOUR2 >=  NMAX_HOUR1) ){							// 
				NMAX_HOUR1 = 24 * 60;									// 強制的に24時間、12時間とする
				NMAX_HOUR2 = 12 * 60;									// 
			}															// 
		}																// 
	}																	// 
																		// 
	for( i=0; i<12; i++ ){												// 
		EXP_NMAX_HOUR1[i] = 											// 
			(short)prm_get(COM_PRM, S_CLX, (short)(32+i), 2, 3) * 60;	// 28-0038 = XXABXX  AB=1-24時間
		EXP_NMAX_HOUR2[i] = 											// 
			(short)prm_get(COM_PRM, S_CLX, (short)(20+i), 2, 3) * 60;	// 28-0038 = XXABXX  AB=1-24時間
																		// 
		exp_nmax_set1[i] = 												// 
			(short)prm_get(COM_PRM, S_CLX, (short)(32+i), 2, 1);		// ｍ回適用 0は∞適用とする
	}																	// 

	if( EXP_FeeCal == 1 ){													// 料金計算拡張 あり
		for( i=0; i<12; i++ ){												// 
			if( sp_mx_exp[i] == 10 ){										// ｎ時間最大
				if( (!EXP_NMAX_HOUR1[i]) || (EXP_NMAX_HOUR1[i]/60 > 24) ){	// ｎ時間範囲ＮＧ
					EXP_NMAX_HOUR1[i] = 24 * 60;							// 強制的に24時間にする
				}															// 
			}																// 
			else if( sp_mx_exp[i] == 20 ){									// Ｗ上限
				if( (!EXP_NMAX_HOUR1[i]) || (EXP_NMAX_HOUR1[i]/60 > 24) ||	// ｎ１、ｎ２時間範囲ＮＧ
					(!EXP_NMAX_HOUR2[i]) || (EXP_NMAX_HOUR2[i]/60 > 24) ||	// ｎ１、ｎ２条件ＮＧ
					(EXP_NMAX_HOUR2[i] >=  EXP_NMAX_HOUR1[i]) ){			// 
					EXP_NMAX_HOUR1[i] = 24 * 60;							// 強制的に24時間、12時間とする
					EXP_NMAX_HOUR2[i] = 12 * 60;							// 
				}															// 
			}																// 
		}																	// 
	}																		// 

	nmax_countup = (uchar)prm_get(COM_PRM, S_CAL, 40, 1, 5);			// Ｎ時間最大のカウント方法
	nmax_countup = 0;													// Ｎ時間最大のカウント方法は0固定

}																		/**********************************************/
/*====================================================================================================================*/
