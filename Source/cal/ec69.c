/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：定期券有効無効チェック　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：ec69(par1,par2) 　　　　　：　渡された定期種別によりその日が定期有効曜日か、無効曜日かを判：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：定する。　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　時刻構造帯　　　　　　　　　リタンコード：ＯＫ（有効）　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　定期種別　　　　　　　　　　　　　　　　　ＮＧ（無効）　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/**********************************************/
#include	<string.h>													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"system.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"mem_def.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"pri_def.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"rkn_def.h"													/*　全デファイン統括　　　　　　　　　　　　　*/
#include	"rkn_cal.h"													/*　料金関連データ　　　　　　　　　　　　　　*/
#include	"rkn_fun.h"													/*　全サブルーチン宣言　　　　　　　　　　　　*/
#include	"Tbl_rkn.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"ope_def.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"prm_tbl.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
char	ec69															/*　定期券有効無効チェック　　　　　　　　　　*/
(struct	CAR_TIM		*wok_tm,											/*　時刻構造体 car_tim型構造帯名　　　　　　　*/
short		no)															/*　定期券種類　　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	char	pass_flg = NG;												/*　定期有効無効ｆｌｇ　　　　　　　　　　　　*/
	char	toku = 0;													/*　特別日　　　　　　　　　　　　　　　　　　*/
	short	set;														/*											*/
	struct	CAR_TIM		t_st_tm;										/*　時刻構造体 car_tim型構造帯名　　　　　　　*/
	struct	CAR_TIM		t_ed_tm;										/*　時刻構造体 car_tim型構造帯名　　　　　　　*/
	short	ret = 0;													/*　　　　　　　　　　　　　　　　　　　　　　*/
	short	adr;														/**/
	char	pos;														/**/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	toku = tokubet( wok_tm->year, (short)wok_tm->mon, (short)wok_tm->day );	/*　特別日チエック　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( toku != 0 )														/*　本日が特別日の場合　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		set = toku + 6;													/*　7:特別日/8-10：特別期間1-3　　　 　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　本日が特別日でない場合　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		set = youbiget( (short)wok_tm->year,(short)wok_tm->mon, 		/*											*/
												(short)wok_tm->day );	/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	adr = (short)(6+(no-1)*10+set/6);									/*　　　　　　　　　　　　　　　　　　　　　　*/
	pos = (char)(6-set%6);												/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( prm_get( COM_PRM, S_PAS, adr, 1, pos ) == 0 )					/*　定期券種類毎に特別日が有効であれば　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		pass_flg = OK;													/*　定期有効とする　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　特別日が無効であれば　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		pass_flg = (char)NG;											/*　定期無効とする　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*											*/
	if( ((cr_tkpk_flg == 1 )||( cr_tkpk_flg == 3))&&( pass_flg == OK ) )/*　カードデータが定期券の場合　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		memset( &t_st_tm,0,7 );											/*　時刻構造体 car_tim型構造帯名　　　　　　　*/
		memset( &t_ed_tm,0,7 );											/*　時刻構造体 car_tim型構造帯名　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		t_st_tm.year = cm33( tsn_tki.data[0] );							/*　西暦　１９９０〜　　　　　　　　　　　　　*/
		t_st_tm.mon = tsn_tki.data[1];									/*　月　　　　　　　　　　　　　　　　　　　　*/
		t_st_tm.day = tsn_tki.data[2];									/*　日　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		t_ed_tm.year = cm33( tsn_tki.data[3] );							/*　西暦　１９９０〜　　　　　　　　　　　　　*/
		t_ed_tm.mon = tsn_tki.data[4];									/*　月　　　　　　　　　　　　　　　　　　　　*/
		t_ed_tm.day = tsn_tki.data[5];									/*　日　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		ec63( &t_ed_tm );												/*　1日更新 　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		ret = ec72( wok_tm,&t_st_tm,&t_ed_tm );							/*　定期期限範囲かをチェック　　　　　　　　　*/
		if(( ret == 0 )||( ret == 2 ))									/*　定期期限外の場合　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			pass_flg = NG;												/*　定期無効とする　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	return(pass_flg);													/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
