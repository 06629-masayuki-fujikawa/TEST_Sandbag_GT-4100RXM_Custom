/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：回数券処理　　　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et44()　　　　　　　　　　：　回数券単価・枚数、制限度数より割引額を算出し、料金及び消費税：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：額を求める。　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
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
#include	"prm_tbl.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"tbl_rkn.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	et44()															/*　時間割引処理　　　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	short			i;													/*　カウンター　　　　　　　　　　　　　　　　*/
	short			maisu = 0;											/*　回数券枚数　　　　　　　　　　　　　　　　*/
	short			wkmai = 0;											/*　ｗｏｒｋ使用枚数　　　　　　　　　　　　　*/
	long			wk_ryo = 0;											/*　ｗｏｒｋ料金　　　　　　　　　　　　　　　*/
	long			wk_ryo2 = 0;										/*　ｗｏｒｋ料金　　　　　　　　　　　　　　　*/
	long			price = 0;											/*　１枚当りの割引額　　　　　　　　　　　　　*/
	struct	REQ_RKN	*req_p;												/*　現在計算後格納エリア　　　　　　　　　　　*/
	struct	RYO_BUF_N	*ryo_b;											/*　現在計算後格納エリア　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	req_p = &req_rhs[tb_number];										/*　現在料金計算要求ポインター　　　　　　　　*/
	ryo_b = &ryo_buf_n;													/*　現在料金計算要求ポインター　　　　　　　　*/
	memmove( req_p,&req_rkn,sizeof(req_rkn) );							/*　現在料金計算要求TBLを計算結果TBLへセット　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( tki_flg == OFF )												/*　定期処理後ｆｌｇがＯＦＦの場合　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( tki_ken < 20 )												/*　定期処理後料金計算要求件数が２０件以下なら*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			req_tki[tki_ken].syubt 	 = req_rhs[tb_number].syubt;		/*　定期処理後料金計算要求テーブルのセット　　*/
			req_tki[tki_ken].param 	 = req_rhs[tb_number].param;		/*　　　　　　　　　　　　　　　　　　　　　　*/
			req_tki[tki_ken].data[0] = req_rhs[tb_number].data[0];		/*　　　　　　　　　　　　　　　　　　　　　　*/
			req_tki[tki_ken].data[1] = req_rhs[tb_number].data[1];		/*　　　　　　　　　　　　　　　　　　　　　　*/
			tki_ken ++;													/*　定期処理後料金計算要求件数インクリメント　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	maisu = (short)req_rkn.data[0];										/*　計算要求テーブルより枚数を参照　　　　　　*/
	price = req_rkn.data[1];											/*　計算要求テーブルより割引金額を参照　　　　*/
	discount = 0l;														/*　割引額０クリア　　　　　　　　　　　　　　*/
	if( ctyp_flg == 4 )													/*　割引対象額が料金＋税金の場合　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		wk_ryo = base_ryo - tol_dis + ryo_b->tax - azukari	;			/*　割引額＝料金　　　　　　　　　　　		　*/
		cons_tax = 0;													/*　税金＝０（使用しない）　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　　　　　　　　　　　　　　　　　　　　　　*/
		wk_ryo = base_ryo - tol_dis - azukari ;							/*　割引額＝料金　　　　　　　　　　　		　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	wk_ryo -= e_pay;													/*　割引額（電子マネー入金分を加味）		　*/

	if(( vl_kas.rim == 1 )&&( price == (long)0xff ))					/*　制限度数が１で金額がｆｆｈの場合（無料券）*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		discount = wk_ryo;												/*　割引額＝旧料金　　　　　　　　　　　　　　*/
		wkmai = 1;														/*　　　　　　　　　　　　　　　　　　　　　　*/
		ryoukin = 0;													/*　割引額＝料金　　　　　　　　　　　　　　　*/
		syu_tax = 0;													/*　税金＝０　　　　　　　　　　　　　　　　　*/
		base_dis_flg = ON;												/*　基本料金全額割引フラグＯＮ　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　通常の回数券の場合　　　　　　　　　　　　*/
 	{																	/*　回数券の使用する枚数を求める　　　　　　　*/
		if( maisu <= vl_kas.rim )										/*　枚数≦制限度数の場合　　　　　　　　　　　*/
			wkmai = maisu;												/*　使用枚数＝枚数　　　　　　　　　　　　　　*/
		else															/*　枚数＞制限度数の場合　　　　　　　　　　　*/
			wkmai = vl_kas.rim;											/*　使用枚数＝制限度数とする　　　　　　　　　*/
		wk_ryo2 = wk_ryo;												/*　ｗ料金＝ｗ料金　　　　　　　　　　　　　　*/
		for( i = 1; i <= wkmai; i ++ )									/*　制限枚数分ループする　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			wk_ryo2 = wk_ryo2 - price;									/*　ｗ料金＝ｗ料金－１枚あたりの割引額　　　　*/
			if( wk_ryo2 <= 0 )											/*　ｗ料金が０未満になった場合　　　　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				wkmai = i;												/*　ループカウンタを使用枚数とする　　　　　　*/
				base_dis_flg = 1;										/*　基本料金全額割引フラグＯＮ　　　　　　　　*/
				break;													/*　ループより抜ける　　　　　　　　　　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		discount = price * wkmai;										/*　割引額＝１枚あたりの割引額×使用枚数　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( wk_ryo <= discount )										/*　料金より割引額の方が大きい場合　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( jitu_wari == 1 )										/*　実割引の場合　　　　　　　　　　　　　　　*/
				discount = wk_ryo;										/*　割引額＝旧料金　　　　　　　　　　　　　　*/
			ryoukin		 = 0;											/*　料金＝旧料金－割引額　　　　　　　　　　　*/
			syu_tax		 = 0;											/*　税金＝０　　　　　　　　　　　　　　　　　*/
			base_dis_flg = ON;											/*　税金＝０　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　料金≠０の場合　　　　　　　　　　　　　　*/
			ryoukin = wk_ryo - discount;								/*　料金＝料金＋割引　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if(( ctyp_flg != 4 )&&( cons_tax_ratio != 0 ))						/*　割引対象が料金かつ　　　　　　　　　　　　*/
		cons_tax = ec68( ryoukin,cons_tax_ratio );						/*　消費税額＝割引後料金×消費税率　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	tol_dis = tol_dis + discount;										/*　割引額合計＝割引額合計＋割引額　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( ctyp_flg != 4 )													/*　　　　　　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		ryo_b->tax = cons_tax;											/*　駐車料金消費税のセーブ　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( ctyp_flg == 1 )												/*　内税の場合　　　　　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			syu_tax = cons_tax;											/*　消費税を集計用エリアへ　　　　　　　　　　*/
			cons_tax = 0;												/*　消費税を０　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	ryo_b->dis = tol_dis;												/*　駐車料金消費税のセーブ　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	req_p->data[0] = maisu - wkmai;										/*　回数券残り枚数のセット　　　　　　　　　　*/
	KaisuuAfterDosuu = (uchar)req_p->data[0];							/*  回数券利用後残回数						  */
	req_p->data[0] <<= 16;
	req_p->data[0] |= (ushort)wkmai;
	req_p->data[2] = discount;											/*　割引額のセット　　　　　　　　　　　　　　*/
	req_p->data[3] = ryoukin;											/*　割引後額のセット　　　　　　　　　　　　　*/
	req_p->data[4] = cons_tax;											/*　消費税額のセット　　　　　　　　　　　　　*/

	if( ryoukin != 0 )			husoku_flg = ON;						/*　料金がある場合　不足フラグＯＮ　　　　　　*/
	else						husoku_flg = OFF;						/*　                不足フラグＯＦＦ　　　　　*/

	ryo_buf.fee_amount += discount;										/*　割引額加算　　　　　　　　　　　　　　　　*/
	KaisuuWaribikiGoukei += discount;									/*  回数券利用額加算 						  */

	KaisuuUseDosuu = (uchar)wkmai;										/*  今回使用度数							  */
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
