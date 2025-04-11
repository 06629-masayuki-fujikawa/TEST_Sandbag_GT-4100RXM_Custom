/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：日付更新処理（加算）　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：ec70(par1,par2) 　　　　　：　計算用時刻を与えられた更新分をもとに算出する。　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　Ｍａｘ（１０×９９）時間　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　５９４００分　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　４１．２５日　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　時刻構造帯　　　　　　　　　無し　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　更新分　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/**********************************************/
#include	"system.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"rkn_def.h"													/*　全デファイン統括　　　　　　　　　　　　　*/
#include	"rkn_cal.h"													/*　料金関連データ　　　　　　　　　　　　　　*/
#include	"rkn_fun.h"													/*　全サブルーチン宣言　　　　　　　　　　　　*/
#include	"Tbl_rkn.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	ec70( 															/*　体系算出処理　　　　　　　　　　　　　　　*/
struct	CAR_TIM		*wok_tm ,											/*　時刻構造体 car_tim型構造帯名　　　　　　　*/
long	k_tim				)											/*　更新分　　　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	char	wkday = 0;													/*　ｗｏｒｋ用日　　　　　　　　　　　　　　　*/
	char	wkhor = 0;													/*　ｗｏｒｋ用時　　　　　　　　　　　　　　　*/
	char	wkmin = 0;													/*　ｗｏｒｋ用分　　　　　　　　　　　　　　　*/
	short	i;															/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if(( k_tim > 0 )&&( k_tim <= 59400 ))								/*　更新分がＭａｘ値より小さい場合  　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		maxmonth[2] = 28;												/*　２月の最大日を２８日とする　　　　　　　　*/
		if(( (wok_tm->year % 4) == 0 )&&( (wok_tm->year % 100) != 0 ))	/*　閏年の場合　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			maxmonth[2] = 29;											/*　２月の最大日を２９日とする　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if((wok_tm->year % 400) == 0 )									/*　閏年の場合　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			maxmonth[2] = 29;											/*　２月の最大日を２９日とする　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		wkday = (char)( k_tim / 1440 );									/*　更新日数算出　　　　　　　　　　　　　　　*/
		wkhor = (char)(( k_tim % 1440 ) / 60 );							/*　更新時間数算出　　　　　　　　　　　　　　*/
		wkmin = (char)(( k_tim % 1440 ) % 60 );							/*　更新分数算出　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		wok_tm->min = (char)( wkmin + wok_tm->min );					/*　計算分＝更新分＋計算分　　　　　　　　　　*/
		if( wok_tm->min >= 60 )											/*　計算分が６０分を越えた場合　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			wok_tm->min = (char)( wok_tm->min - 60 );					/*　計算分＝計算分−６０　　　　　　　　　　　*/
			wkhor = (char)( wkhor + 1 );								/*　更新時間＋１　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		wok_tm->hour = (char)( wkhor + wok_tm->hour );					/*　計算時＝更新時＋計算時　　　　　　　　　　*/
		if( wok_tm->hour >= 24 )										/*　計算時が２４を越えた場合　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			wok_tm->hour = (char)( wok_tm->hour - 24 );					/*　計算時＝計算時−２４　　　　　　　　　　　*/
			wkday = (char)( wkday + 1 );								/*　更新日＋１　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		wok_tm->week = (char)(( wok_tm->week + wkday ) % 7 );			/*　曜日を１日繰上げる　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		for( i = 0 ; i < (short)wkday ; i ++ )							/*　　　　　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( wok_tm->day == maxmonth[wok_tm->mon] )					/*　該当する日付がその月の最大日に当たるとき　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( wok_tm->mon == 12 )									/*　１２月であれば　　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					wok_tm->year++;										/*　年インクリメント　　　　　　　　　　　　　*/
					maxmonth[2] = 28;									/*　２月の最大日を２８日とする　　　　　　　　*/
					if(( (wok_tm->year % 4) == 0 )						/*　閏年の場合　　　　　　　　　　　　　　　　*/
									&&( (wok_tm->year % 100) != 0 ))	/*　　　　　　　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						maxmonth[2] = 29;								/*　２月の最大日を２９日とする　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
					if((wok_tm->year % 400) == 0 )						/*　閏年の場合　　　　　　　　　　　　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						maxmonth[2] = 29;								/*　２月の最大日を２９日とする　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					wok_tm->mon = 1;									/*　月を１月に設定する　　　　　　　　　　　　*/
					wok_tm->day = 1;									/*　日を１日に設定する　　　　　　　　　　　　*/
				}														/*　１２月以外であれば　　　　　　　　　　　　*/
				else													/*　１２月以外であれば　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					wok_tm->mon++;										/*　月インクリメント　　　　　　　　　　　　　*/
					wok_tm->day = 1;									/*　日を１日に設定する　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　その月の最大日に達していなければ　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				wok_tm->day++;											/*　日インクリメント　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
