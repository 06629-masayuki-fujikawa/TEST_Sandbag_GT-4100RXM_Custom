/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：閏年チェック　　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：cm14(par1)　　　　　　　　：閏年チェックを行なう　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　年　　　　　　　　　　　　　結果　０：平年　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　１：閏年　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　ー１：ＮＧ　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/**********************************************/
#include	"system.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"rkn_def.h"													/*　全デファイン統括　　　　　　　　　　　　　*/
#include	"rkn_cal.h"													/*　料金関連データ　　　　　　　　　　　　　　*/
#include	"rkn_fun.h"													/*　全サブルーチン宣言　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
char	cm14( short pa_yer )											/*　　　　　　　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	char	cm_ret;														/*　結果　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	cm_ret= 0;															/*　結果初期化（ＮＧ）　　　　　　　　　　　　*/

	if(( 1990 <= pa_yer ) && ( pa_yer <= 2079 ))						/*　範囲チェック（１９９０年〜２０７９年）　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		cm_ret = 0;														/*　結果を平年としておく　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if (( (pa_yer % 4) == 0 )&&( (pa_yer % 100) != 0 ))				/*　閏年の場合　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			cm_ret = 1;													/*　結果補正（閏年）　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if ((pa_yer % 400) == 0 )										/*　閏年の場合　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			cm_ret = 1;													/*　結果補正（閏年）　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	return( cm_ret );													/*　結果を返す　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
