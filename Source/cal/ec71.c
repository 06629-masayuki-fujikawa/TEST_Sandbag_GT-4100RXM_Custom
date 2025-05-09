/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：時間差算出処理　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：ec71(par1,par2) 　　　　　：　ｐａｒ１時刻からｐａｒ２時刻までの時間差を分で算出する。　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：もしｐａｒ１＞ｐａｒ２の場合は、処理しない。　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　Ｍａｘ（１０×９９）時間　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　５９４００分　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　４１．２５日　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　ａ：時刻構造帯　　　　　　　時間差（分）　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　ｂ：時刻構造帯　　　　　　　−１：ｅｒｒ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/**********************************************/
#include	"system.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"rkn_def.h"													/*　全デファイン統括　　　　　　　　　　　　　*/
#include	"rkn_cal.h"													/*　料金関連データ　　　　　　　　　　　　　　*/
#include	"rkn_fun.h"													/*　全サブルーチン宣言　　　　　　　　　　　　*/
#include	"Tbl_rkn.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
unsigned long	ec71( 													/*　年月日時分比較処理　　　　　　　　　　　　*/
struct	CAR_TIM		*a	,												/*　時刻構造体 car_tim型構造帯名　ａ　　　　　*/
struct	CAR_TIM		*b	)												/*　時刻構造体 car_tim型構造帯名　ｂ　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	short			rtn = 0;											/*　リターンコード　　　　　　　　　　　　　　*/
	unsigned long	ret = (ulong)-1;									/*　リターンコード（時間差）　　　　　　　　　*/
	unsigned long	atim = 0;											/*　計算ａ時刻（分換算）　　　　　　　　　　　*/
	unsigned long	btim = 0;											/*　計算ｂ時刻（分換算）　　　　　　　　　　　*/
	unsigned long	mtim = 0;											/*　ｗｏｒｋ時刻（分換算）　　　　　　　　　　*/
	unsigned long	mosa = 0;											/*　月差　　　　　　　　　　　　　　　　　　　*/
	unsigned long	aji[5];												/*　ａ時刻配列　　　　　　　　　　　　　　　　*/
	unsigned long	bji[5];												/*　ｂ時刻配列　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	rtn = ec64( a,b );													/*　年月日時分比較処理　　　　　　　　　　　　*/
	if( rtn >= 0 )														/*　ａ＜ｂの場合のみ処理をする　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		aji[0] = (unsigned long)a->year;								/*　ａ時刻構造帯をａ時刻配列にセット　　　　　*/
		aji[1] = (unsigned long)a->mon;									/*　　　　　　　　　　　　　　　　　　　　　　*/
		aji[2] = (unsigned long)a->day;									/*　　　　　　　　　　　　　　　　　　　　　　*/
		aji[3] = (unsigned long)a->hour;								/*　　　　　　　　　　　　　　　　　　　　　　*/
		aji[4] = (unsigned long)a->min;									/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		bji[0] = (unsigned long)b->year;								/*　ｂ時刻構造帯をｂ時刻配列にセット　　　　　*/
		bji[1] = (unsigned long)b->mon;									/*　　　　　　　　　　　　　　　　　　　　　　*/
		bji[2] = (unsigned long)b->day;									/*　　　　　　　　　　　　　　　　　　　　　　*/
		bji[3] = (unsigned long)b->hour;								/*　　　　　　　　　　　　　　　　　　　　　　*/
		bji[4] = (unsigned long)b->min;									/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		maxmonth[2] = 28;												/*　２月の最大日を２８日とする　　　　　　　　*/
		if((( aji[0] % 4 ) == 0 )&&(( aji[0] % 100 ) != 0 ))			/*　閏年の場合　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			maxmonth[2] = 29;											/*　２月の最大日を２９日とする　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(( aji[0] % 400 ) == 0 )										/*　閏年の場合　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			maxmonth[2] = 29;											/*　２月の最大日を２９日とする　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( aji[0] != bji[0] )											/*　ａ年とｂ年が異なる場合　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( ( bji[0] - aji[0] ) == 1 )								/*　ａ年とｂ年の差が１ヶ月の場合　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if((( aji[1] == 11 ) || ( aji[1] == 12 ))				/*　ａ月が１１月または１２月　かつ　　　　　　*/
						&& (( bji[1] == 1 ) || ( bji[1] == 2 )))		/*　　ｂ月が１月または２月の場合　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( aji[1] == 11 )									/*　ａ月が１１月の場合　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						atim = 44640;									/*　ａ時間を１２月分換算する　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					atim = atim + 1440 - ( aji[3] * 60 + aji[4] )		/*　ａ時間＝ａ月最終日までの時間＋ａ時間　　　*/
						 + ( (unsigned long)maxmonth[aji[1]] - aji[2] ) * 1440;	/*　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( bji[1] == 2 )									/*　ｂ月が２月の場合　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						btim = 44640;									/*　ｂ時間を１月分換算する　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					btim = btim + ( bji[2] - 1 ) * 1440					/*　ｂ時間＝ｂ月への月変わり時間＋ｂ時間　　　*/
											+ bji[3] * 60 + bji[4];		/*　　　　　　　　　　　　　　　　　　　　　　*/
					ret = atim + btim;									/*　時間差＝ａ時間＋ｂ時間　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　ａ年＝ｂ年の場合　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( aji[1] != bji[1] )										/*　ａ月≠ｂ月の場合　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				mosa = bji[1] - aji[1];									/*　月の差を求める　　　　　　　　　　　　　　*/
				if( mosa <= 2 )											/*　月の差が２ヶ月以内の場合　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					atim = 1440 - ( aji[3] * 60 + aji[4] )				/*　ａ時間＝ａ月最終時刻　　　　　　　　　　　*/
							 + ( (unsigned long)maxmonth[aji[1]] - aji[2] ) * 1440;	/*　　　　　　　　　　　　　　　　*/
					btim = ( bji[2] - 1 )*1440 + bji[3] * 60 + bji[4];	/*　ｂ時間＝ｂ月開始時刻　　　　　　　　　　　*/
					ret = atim + btim;									/*　時間差＝ａ時間＋ｂ時間　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( mosa != 1 )										/*　月の差が１ヶ月ではない場合　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						mtim = (unsigned long)maxmonth[aji[1] + 1] * 1440;		/*　月時間＝ａ＋１月の分換算　　　　　*/
						ret = atim + btim + mtim;						/*　時間差＝ａ時間＋ｂ時間＋月時間　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　ａ月＝ｂ月の場合　　　　　　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				atim = ( aji[2] - 1 ) * 1440 + aji[3] * 60 + aji[4];	/*　ａ時間＝ａ月開始時間　　　　　　　　　　　*/
				btim = ( bji[2] - 1 ) * 1440 + bji[3] * 60 + bji[4];	/*　ｂ時間＝ｂ（ａ）月開始時間　　　　　　　　*/
				ret = btim - atim;										/*　時間差＝ｂ時間−ａ時間　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	return( ret );														/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
unsigned long	ec71a( 												// 
struct	CAR_TIM		*a	,											// 時刻構造体 car_tim型構造帯名 ａ
struct	CAR_TIM		*b	)											// 時刻構造体 car_tim型構造帯名 ｂ
{																	// 
ulong	w_day;														// 
ulong	w_hou1,w_hou2;												// 
																	// 第1引数の分変換
	w_day = (ulong)dnrmlzm( a->year,(short)a->mon,(short)a->day );	//  年月日の日換算
	w_hou1 = w_day*1440L + (ulong)(a->hour*60 + a->min);			//  分に変換する
																	// 第2引数の分変換
	w_day = (ulong)dnrmlzm( b->year,(short)b->mon,(short)b->day );	//  年月日の日換算
	w_hou2 = w_day*1440L + (ulong)(b->hour*60 + b->min);			//  分に変換する
																	// 
	if( w_hou1 <= w_hou2 ){											// 
		return( w_hou2 - w_hou1 );									// 
	}else{															// 
		return( 0 );												// 
	}																// 
}																	// 
