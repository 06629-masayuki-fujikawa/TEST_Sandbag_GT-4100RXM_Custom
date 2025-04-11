/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：日付１日繰下げ処理　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：ec62(par1)　　　　　　　　：　ｐａｒ１の時刻（年月日時分）を１日繰下げる。　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　時刻構造帯　　　　　　　　　無し　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/**********************************************/
#include	"system.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"Tbl_rkn.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"rkn_def.h"													/*　全デファイン統括　　　　　　　　　　　　　*/
#include	"rkn_cal.h"													/*　料金関連データ　　　　　　　　　　　　　　*/
#include	"rkn_fun.h"													/*　全サブルーチン宣言　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	ec62( 															/*　日付１日繰下げ処理　　　　　　　　　　　　*/
struct	CAR_TIM		*wok_tm  )											/*　時刻構造体 car_tim型構造帯名　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	wok_tm->week = (char)(( wok_tm->week + 6 ) % 7 );					/*　曜日を１日繰下げる　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	maxmonth[2] = 28;													/*　２月の最大日を２８日とする　　　　　　　　*/
	if(( (wok_tm->year % 4) == 0 )&&( (wok_tm->year % 100) != 0 ))		/*　閏年の場合　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		maxmonth[2] = 29;												/*　２月の最大日を２９日とする　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( (wok_tm->year % 400) == 0 )										/*　閏年の場合　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		maxmonth[2] = 29;												/*　２月の最大日を２９日とする　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if(wok_tm->day == 1){												/*　該当する日付が１日の場合　　　　　　　　　*/
		if(wok_tm->mon == 1){											/*　１月であれば　　　　　　　　　　　　　　　*/
			wok_tm->year--;												/*　年デクリメント　　　　　　　　　　　　　　*/
			wok_tm->mon = 12;											/*　月を１２月に設定する　　　　　　　　　　　*/
			wok_tm->day = 31;											/*　日を３１日に設定する　　　　　　　　　　　*/
		}else{															/*　１月以外であれば　　　　　　　　　　　　　*/
			wok_tm->mon--;												/*　月デクリメント　　　　　　　　　　　　　　*/
			wok_tm->day = maxmonth[wok_tm->mon];						/*　日をその月の最大日に設定する　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}else{																/*　１日でなければ　　　　　　　　　　　　　　*/
		wok_tm->day--;													/*　日デクリメント　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
