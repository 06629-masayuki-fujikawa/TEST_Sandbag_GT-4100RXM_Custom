/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：昼夜帯設定参照処理　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：mc101() 　　　　　　　　　：　設定項目一括参照（昼夜帯）　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　無し　　　　　　　　　　　　無し　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/**********************************************/
#include	<string.h>
#include	"system.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"mem_def.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"rkn_def.h"													/*　全デファイン統括　　　　　　　　　　　　　*/
#include	"rkn_cal.h"													/*　料金関連データ　　　　　　　　　　　　　　*/
#include	"rkn_fun.h"													/*　全サブルーチン宣言　　　　　　　　　　　　*/
#include	"prm_tbl.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	mc101()															/*　　　　　　　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	i,j,k;														/*　ループカウント用変数　　　　　　　　　　　*/
	short	prm_wk;														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	//------------------
	// 第1〜第3料金体系
	//------------------
	for ( i = 0 ; i < 3 ; i ++ )										/*　第１〜３料金体系　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for ( j = 0 ; j < 12 ; j ++ )									/*　Ａ〜Ｄ１車種　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			for ( k = 0 ; k < 6 ; k ++ )								/*　第１〜６料金帯　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				fee_std[i][j][k] =										/*　基本料金[体系][車種][帯]算出　　　　　　　*/
						(short)(CPrmSS[S_RAT][64+300*i+20*j+2*k]/10);	/*　　　　　　　　　　　　　　　　　　　　　　*/
				fee_exp[i][j][k] =										/*　追加料金[体系][車種][帯]算出　　　　　　　*/
						(short)(CPrmSS[S_RAT][65+300*i+20*j+2*k]/10);	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	//------------------
	// 第4〜第6料金体系
	//------------------
	for ( i = 0 ; i < 3 ; i ++ )										/*　第１〜３料金体系　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for ( j = 0 ; j < 12 ; j ++ )									/*　Ａ〜Ｄ１車種　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			for ( k = 0 ; k < 6 ; k ++ )								/*　第１〜６料金帯　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				fee_std[i+3][j][k] =									/*　基本料金[体系][車種][帯]算出　　　　　　　*/
						(short)(CPrmSS[S_CLA][64+300*i+20*j+2*k]/10);	/*　　　　　　　　　　　　　　　　　　　　　　*/
				fee_exp[i+3][j][k] =									/*　追加料金[体系][車種][帯]算出　　　　　　　*/
						(short)(CPrmSS[S_CLA][65+300*i+20*j+2*k]/10);	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	//------------------
	// 第7〜第9料金体系
	//------------------
	for ( i = 0 ; i < 3 ; i ++ )										/*　第１〜３料金体系　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for ( j = 0 ; j < 12 ; j ++ )									/*　Ａ〜Ｄ１車種　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			for ( k = 0 ; k < 6 ; k ++ )								/*　第１〜６料金帯　　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				fee_std[i+6][j][k] =									/*　基本料金[体系][車種][帯]算出　　　　　　　*/
						(short)(CPrmSS[S_CLB][64+300*i+20*j+2*k]/10);	/*　　　　　　　　　　　　　　　　　　　　　　*/
				fee_exp[i+6][j][k] =									/*　追加料金[体系][車種][帯]算出　　　　　　　*/
						(short)(CPrmSS[S_CLB][65+300*i+20*j+2*k]/10);	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	//------------------
	// 第1〜第3料金体系
	//------------------
	for ( i = 0 ; i < 3 ; i ++ )										/*　第１〜３料金体系　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for ( j = 0 ; j < 6 ; j ++ )									/*　第１〜６料金帯　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			bnd_strt[i][j] =											/*　料金帯開始時刻[体系][帯]算出　　　　　　　*/
					(short)prm_tim( 0,S_RAT,(short)(4+300*i+4*j) );		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			ovl_type[i][j] =											/*　重複方式[体系][帯]算出　　　　　　　　　　*/
					(char)prm_get( 0,S_RAT,(short)(5+300*i+4*j),1,4 );	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			prm_wk = (short)prm_get( 0,S_RAT,(short)(5+300*i+4*j),3,1 );/*　　　　　　　　　　　　　　　　　　　　　　*/
			ovl_time[i][j] = (prm_wk/100) * 60 + (prm_wk%100);			/*　重複時間[体系][帯]算出　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			leng_std[i][j] =											/*　基本時間[体系][帯]算出　　　　　　　　　　*/
					(short)prm_tim( 0,S_RAT,(short)(6+300*i+4*j) );		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			leng_exp[i][j] =											/*　追加時間[体系][帯]算出　　　　　　　　　　*/
					(short)prm_tim( 0,S_RAT,(short)(7+300*i+4*j) );		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	//------------------
	// 第4〜第6料金体系
	//------------------
	for ( i = 0 ; i < 3 ; i ++ )										/*　第１〜３料金体系　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for ( j = 0 ; j < 6 ; j ++ )									/*　第１〜６料金帯　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			bnd_strt[i+3][j] =											/*　料金帯開始時刻[体系][帯]算出　　　　　　　*/
					(short)prm_tim( 0,S_CLA,(short)(4+300*i+4*j) );		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			ovl_type[i+3][j] =											/*　重複方式[体系][帯]算出　　　　　　　　　　*/
					(char)prm_get( 0,S_CLA,(short)(5+300*i+4*j),1,4 );	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			prm_wk = (short)prm_get( 0,S_CLA,(short)(5+300*i+4*j),3,1 );/*　　　　　　　　　　　　　　　　　　　　　　*/
			ovl_time[i+3][j] = (prm_wk/100) * 60 + (prm_wk%100);		/*　重複時間[体系][帯]算出　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			leng_std[i+3][j] =											/*　基本時間[体系][帯]算出　　　　　　　　　　*/
					(short)prm_tim( 0,S_CLA,(short)(6+300*i+4*j) );		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			leng_exp[i+3][j] =											/*　追加時間[体系][帯]算出　　　　　　　　　　*/
					(short)prm_tim( 0,S_CLA,(short)(7+300*i+4*j) );		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	//------------------
	// 第7〜第9料金体系
	//------------------
	for ( i = 0 ; i < 3 ; i ++ )										/*　第１〜３料金体系　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for ( j = 0 ; j < 6 ; j ++ )									/*　第１〜６料金帯　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			bnd_strt[i+6][j] =											/*　料金帯開始時刻[体系][帯]算出　　　　　　　*/
					(short)prm_tim( 0,S_CLB,(short)(4+300*i+4*j) );		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			ovl_type[i+6][j] =											/*　重複方式[体系][帯]算出　　　　　　　　　　*/
					(char)prm_get( 0,S_CLB,(short)(5+300*i+4*j),1,4 );	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			prm_wk = (short)prm_get( 0,S_CLB,(short)(5+300*i+4*j),3,1 );/*　　　　　　　　　　　　　　　　　　　　　　*/
			ovl_time[i+6][j] = (prm_wk/100) * 60 + (prm_wk%100);		/*　重複時間[体系][帯]算出　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			leng_std[i+6][j] =											/*　基本時間[体系][帯]算出　　　　　　　　　　*/
					(short)prm_tim( 0,S_CLB,(short)(6+300*i+4*j) );		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			leng_exp[i+6][j] =											/*　追加時間[体系][帯]算出　　　　　　　　　　*/
					(short)prm_tim( 0,S_CLB,(short)(7+300*i+4*j) );		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	for ( i = 0 ; i < 3 ; i ++ )										/*　第１〜３料金体系　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		max_bnd[i] = (short)CPrmSS[S_RAT][3+300*i];						/*　第何料金帯まで使用するか算出　　　　　　　*/
		max_bnd[i+3] = (short)CPrmSS[S_CLA][3+300*i];					/*　第何料金帯まで使用するか算出　　　　　　　*/
		max_bnd[i+6] = (short)CPrmSS[S_CLB][3+300*i];					/*　第何料金帯まで使用するか算出　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	Change_Ryokin_Data();												/*　０時区切り変換処理　　　　　　　　　　　　*/	
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	for ( i = 0 ; i < 3 ; i ++ )										/*　第１〜３料金体系　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for ( j = 0 ; j < 12 ; j ++ )									/*　Ａ〜Ｄ１車種　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			day_max2[i][j] = CPrmSS[S_RAT][63+300*i+20*j];				/*　日毎最大料金２[体系][車種]算出　　　　　　*/
			day_max2[i+3][j] = CPrmSS[S_CLA][63+300*i+20*j];			/*　日毎最大料金２[体系][車種]算出　　　　　　*/
			day_max2[i+6][j] = CPrmSS[S_CLB][63+300*i+20*j];			/*　日毎最大料金２[体系][車種]算出　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	for ( i = 0 ; i < 3 ; i ++ )										/*　第１〜３料金体系　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for (j = 0 ; j < max_bnd[i]-1 ; j++)							/*最高使用料金帯の１つ前の料金帯の分までループ*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if ( bnd_strt[i][j] > bnd_strt[i][j+1])						/*料金帯開始時刻が次料金帯開始時刻より前の場合*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				bnd_strt[i][j+1] += T_DAY;								/*次料金帯開始時刻を換算（１４４０分加算）　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	for ( i = 0 ; i < 3 ; i ++ )										/*　第１〜３料金体系　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for (j = 0 ; j < max_bnd[i+3]-1 ; j++)							/*最高使用料金帯の１つ前の料金帯の分までループ*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if ( bnd_strt[i+3][j] > bnd_strt[i+3][j+1])					/*料金帯開始時刻が次料金帯開始時刻より前の場合*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				bnd_strt[i+3][j+1] += T_DAY;							/*次料金帯開始時刻を換算（１４４０分加算）　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	for ( i = 0 ; i < 3 ; i ++ )										/*　第１〜３料金体系　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for (j = 0 ; j < max_bnd[i+6]-1 ; j++)							/*最高使用料金帯の１つ前の料金帯の分までループ*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if ( bnd_strt[i+6][j] > bnd_strt[i+6][j+1])					/*料金帯開始時刻が次料金帯開始時刻より前の場合*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				bnd_strt[i+6][j+1] += T_DAY;							/*次料金帯開始時刻を換算（１４４０分加算）　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	for ( i = 0 ; i < 3 ; i ++ )										/*　第１〜３料金体系　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for (j = 0 ; j < max_bnd[i] ; j++)								/*　最高料金帯〜予備料金帯の分までループ　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			bnd_strt[i][j + max_bnd[i]] = bnd_strt[i][j] + T_DAY;		/*　予備料金帯開始時刻に　　　　　　　　　　　*/
		}																/*　第１料金帯開始時刻を換算した時刻を代入　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	for ( i = 0 ; i < 3 ; i ++ )										/*　第１〜３料金体系　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for (j = 0 ; j < max_bnd[i+3] ; j++)							/*　最高料金帯〜予備料金帯の分までループ　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			bnd_strt[i+3][j + max_bnd[i+3]] = bnd_strt[i+3][j] + T_DAY;	/*　予備料金帯開始時刻に　　　　　　　　　　　*/
		}																/*　第１料金帯開始時刻を換算した時刻を代入　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	for ( i = 0 ; i < 3 ; i ++ )										/*　第１〜３料金体系　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		for (j = 0 ; j < max_bnd[i+6] ; j++)							/*　最高料金帯〜予備料金帯の分までループ　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			bnd_strt[i+6][j + max_bnd[i+6]] = bnd_strt[i+6][j] + T_DAY;	/*　予備料金帯開始時刻に　　　　　　　　　　　*/
		}																/*　第１料金帯開始時刻を換算した時刻を代入　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
/*[]----------------------------------------------------------------------[]*/
/*| ０時区切りする・しないの判定                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GET_RYO_SETMODE( void )                                 |*/
/*| PARAMETER    : syubetu   料金種別(1〜12)                               |*/
/*|              : taikei    料金体系(1〜9)                                |*/
/*| RETURN VALUE : mode 区切り設定 2: ０時区切り変換する                   |*/
/*|                                1: ０時区切り変換しない                 |*/
/*|                                0: 指定された料金体系使用していない     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	GET_RYO_SETMODE(uchar syubetu, uchar taikei)
{
	char	syu_no, i, max_typ, seg, ret;
	short 	mode, cnt, st_tim;
	
	ret = 0;
	mode = (short)prm_get(COM_PRM, S_CAL, 11, 1, 1); 				// 区切り設定取得
	seg = GET_PARSECT_NO((char)(taikei+1));							// セクション番号
	cnt = (short)CPrmSS[seg][3+RYO_TAIKEI_SETCNT*(taikei%3)];			// 使用料金帯の数
	st_tim = (short)CPrmSS[seg][4+RYO_TAIKEI_SETCNT*(taikei%3)];		// 第１料金帯開始時刻取得
	if(mode == 0){								// 区切りあり
		if(RYO_CAL_EXTEND){					// 料金計算拡張有り
			max_typ = (char)prm_get(COM_PRM, S_CLX, (short)(2+(syubetu/3)), 2, (char)(6-(((syubetu%3)*2)+1)) );		// 種別毎の最大料金タイプ取得
		}
		else {
			max_typ = (char)CPrmSS[S_CAL][2];						// 最大料金タイプ取得
		}
		switch( max_typ ){
			case 2:		// 時刻指定日毎最大
			case 3:		// ２種時刻指定日毎最大
			case 5:		// 時間帯最大
			case 10:	// ｎ時間ｍ回最大
			case 20:	// Ｗ上限
				break;

			default:
				max_typ = 0;	// 上記以外は最大なしで動作する。
				break;
		}

		for(i=0;i<11;i++){								// 曜日(日〜土・特別日・特別期間1〜3)
			if(i <= 5){
				syu_no = (char)prm_get(COM_PRM, S_SHA, (short)(3+(syubetu*6)), 1, (char)(6-i));		// 曜日毎の料金体系取得
			}
			else{
				syu_no = (char)prm_get(COM_PRM, S_SHA, (short)(4+(syubetu*6)), 1, (char)(12-i));
			}
			if(syu_no == 0){							// 設定値が０の場合は
				syu_no = 1;								// 第１料金体系とする
			}
			if(syu_no == taikei+1){							// 料金体系が一致
				if((max_typ != 5 && max_typ != 3 ) && st_tim != 0 && (cnt >= 1 && cnt <= 5)){
					// 時間帯最大・第1料金帯開始時刻が0時・料金帯の数が１〜５以外の場合は区切りしない
					ret = 2;							// 区切りする
				}
				else {
					ret = 1;							// 区切りしない
				}
				break;
			}
		}
	}
	
	return ret;	
}
/*[]----------------------------------------------------------------------[]*/
/*| ０時区切り変換処理                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Change_Ryokin_Data( void )                              |*/
/*| PARAMETER    :                                                         |*/
/*| RETURN VALUE :                                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void Change_Ryokin_Data( void )
{
	uchar	i, j, k;

	char	Change_tble[FEE_TAIKEI_MAX]={0};
	char	chgflag = 0;
	short	maxband_wk[FEE_TAIKEI_MAX];

	memcpy(maxband_wk, max_bnd, sizeof(max_bnd));					// ローカルにコピー
	for(i=0; i < SYUBET_MAX; i++){									// 車種(Ａ〜Ｌ)
		for(j=0; j < FEE_TAIKEI_MAX; j++){							// 料金体系
			if((chgflag = GET_RYO_SETMODE(i,j)) == 2 ){
				for(k = (uchar)max_bnd[j]; k > 0; k--){
					fee_std[j][i][k] = fee_std[j][i][k-1];			// 基本料金
					fee_exp[j][i][k] = fee_exp[j][i][k-1];			// 追加料金
					if( Change_tble[j] == 0 ){
						bnd_strt[j][k] = bnd_strt[j][k-1];			// 料金帯開始時刻
						ovl_type[j][k] = ovl_type[j][k-1];			// 重複方式
						ovl_time[j][k] = ovl_time[j][k-1];			// 重複時間
						leng_std[j][k] = leng_std[j][k-1];			// 基本時間
						leng_exp[j][k] = leng_exp[j][k-1];			// 追加時間
					}
				}
				fee_std[j][i][0] = fee_std[j][i][max_bnd[j]];		// 基本料金
				fee_exp[j][i][0] = fee_exp[j][i][max_bnd[j]];		// 追加料金
				if( Change_tble[j] == 0 ){
					ovl_type[j][0] = ovl_type[j][max_bnd[j]];		// 重複方式
					ovl_time[j][0] = ovl_time[j][max_bnd[j]];		// 重複時間
					leng_std[j][0] = leng_std[j][max_bnd[j]];		// 基本時間
					leng_exp[j][0] = leng_exp[j][max_bnd[j]];		// 追加時間
					bnd_strt[j][0] = 0;								// 料金帯開始時刻(０固定)
					maxband_wk[j] += 1;								// 料金帯の数+1
				}
			}
			if(!Change_tble[j]){									// 未変換
				Change_tble[j] = chgflag;							// 変換状態セット
			}
		}
	}
	memcpy(max_bnd, maxband_wk, sizeof(max_bnd));					// 変換後の料金帯の数をコピー
}
/*[]----------------------------------------------------------------------[]*/
/*| 料金体系のセクションNoを取得する                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GET_PARSECT_NO                                          |*/
/*| PARAMETER    :  n  (料金体系)                                          |*/
/*| RETURN VALUE : seg                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
char	GET_PARSECT_NO(char no)
{
	char	seg = 0;
	switch(no){
		case 1:
		case 2:
		case 3:
			seg = S_RAT;					// セクション31
			break;
		case 4:
		case 5:
		case 6:
			seg = S_CLA;					// セクション58
			break;
		case 7:
		case 8:
		case 9:
			seg = S_CLB;					// セクション59
			break;
	}
	return seg;
}
