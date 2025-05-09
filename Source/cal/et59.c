/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：料金差引割引　　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et59()　　　　　　　　　　：　精算中止駐車券の場合など、回数券・プリペイドカードの領収額を：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：駐車料金より差引、料金・消費税を算出する。　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　無し　　　　　　　　　　　　無し　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/**********************************************************************************************************************/
																		/**********************************************/
#include	<string.h>													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"system.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"rkn_def.h"													/*　全デファイン統括　　　　　　　　　　　　　*/
#include	"rkn_cal.h"													/*　料金関連データ　　　　　　　　　　　　　　*/
#include	"rkn_fun.h"													/*　全サブルーチン宣言　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	et59()															/*　時間割引処理　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	long	wk_ryo 		= 0;											/*　ｗｏｒｋ消費税金額　　　　　　　　　　　　*/
	struct	REQ_RKN		*req_p;											/*　現在計算後格納エリア　　　　　　　　　　　*/
	struct	RYO_BUF_N	*r;												/*　料金データバッファ　　　　　　　　　　　　*/
	short 	cardtype   	= 0;											/*　回数券/PPC(回数券 = 0 ,PPC != 0 )	 　 　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	r 		= &ryo_buf_n;												/*　　　　　　　　　　　　　　　　　　　　　　*/
	req_p 	= &req_rhs[tb_number];										/*　現在料金計算要求ポインター　　　　　　　　*/
	memmove( &req_rhs[tb_number],&req_rkn,sizeof(req_rkn) );			/*　現在料金計算要求TBLを計算結果TBLへセット　*/
	cardtype = (short)req_rkn.data[1];									/*　回数券/PPC　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( ctyp_flg == 4 )			wk_ryo = base_ryo - tol_dis + r->tax;	/*　割引対象額が料金+税金の場合　料金-割引+税 */
	else						wk_ryo = base_ryo - tol_dis;			/*　							 料金ｰ割引    */
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( wk_ryo < req_rkn.data[0] )										/*　料金よりＰ領収額が大きい場合　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		ryoukin = 0;													/*　料金ｸﾘｱ					                  */
		if( cardtype  == 0 )		tol_dis = req_rkn.data[0];			/*　回数券　割引額ｾｯﾄ 　　　　　　　　        */
		else						c_pay   = req_rkn.data[0];			/*　		ﾌﾟﾘﾍﾟｲﾄﾞ使用額ｾｯﾄ 　　　　        */
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　Ｐ領収額＜駐車料金　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/

		/* TF7700より回数券処理移植 */
		if( cardtype  == 0 )											/*　回数券　　　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			ryoukin = wk_ryo  - req_rkn.data[0];						/*　料金＝料金−割引料金　　                  */
			tol_dis = tol_dis + req_rkn.data[0];						/*　割引額合計のクリア　　　　　　　　  　　　*/
			discount = req_rkn.data[0];									/*　割引額合計のクリア　　　　　　　　  　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			ryoukin = wk_ryo  ;											/*　料金＝料金			　　                  */
			c_pay   = req_rkn.data[0];									/*　ﾌﾟﾘﾍﾟｲﾄﾞ使用額ｾｯﾄ 　　　　                */
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	r->dis 	= tol_dis;													/*　割引額合計のクリア　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( ryoukin >= 0 )													/*　料金>=０の場合　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if(( ctyp_flg != 4 )&&( cons_tax_ratio != 0 ))					/*　割引対象が料金かつ　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			cons_tax = ec68( ryoukin,cons_tax_ratio );					/*　消費税額＝割引後料金×消費税率　　　　　　*/
			r->tax = cons_tax;											/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	req_p->data[3] = ryoukin;											/*　割引後額のセット　　　　　　　　　　　　　*/
	if( cardtype != 0 )													/*　ﾌﾟﾘﾍﾟｲﾄﾞ使用? 　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		req_p->data[3] -= c_pay;										/*　割引後額-ﾌﾟﾘﾍﾟｲﾄﾞ使用額ｾｯﾄ  　　　　　　　*/
		c_pay_afr 		= req_p->data[3];								/*　ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ使用後料金　	　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	req_p->data[4] = cons_tax;											/*　消費税額のセット　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	return;																/*                      					  */
}																		/**********************************************/
