/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：料金割引処理　　　　　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et43()　　　　　　　　　　：　割引対象額より料金を割引く処理を行う。　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
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
#include	"Tbl_rkn.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"prm_tbl.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	et43()															/*　料金割引処理処理　　　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	long			wk_ryo = 0;											/*　旧料金　　　　　　　　　　　　　　　　　　*/
	struct	REQ_RKN	*req_p;												/*　現在計算後格納エリア　　　　　　　　　　　*/
	struct	RYO_BUF_N	*ryo_b;											/*　現在計算後格納エリア　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	req_p = &req_rhs[tb_number];										/*　現在料金計算要求ポインター　　　　　　　　*/
	ryo_b = &ryo_buf_n;													/*　現在料金計算要求ポインター　　　　　　　　*/
	memmove( req_p,&req_rkn,sizeof(req_rkn) );							/*　現在料金計算要求TBLを計算結果TBLへセット　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	car_type = req_rkn.syubt;											/*　計算要求テーブルの車種を参照する　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( ctyp_flg == 4 )													/*　　　　　　　　　　　　　　　　　　　　　　*/
		wk_ryo = base_ryo + ryo_b->tax - tol_dis;						/*　料金＝料金＋税金　　　　　　　　　　　　　*/
	else																/*　　　　　　　　　　　　　　　　　　　　　　*/
		wk_ryo = base_ryo - tol_dis;									/*　料金＝料金＋税金　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	discount = req_rkn.data[0];											/*　料金割引額セット　　　　　　　　　　　　　*/
	if( ryo_buf.mis_zenwari == 1 )										/*　全額割引　　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		discount = wk_ryo;												/*　全額割引（割引額＝料金）　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		ryoukin		 = 0;												/*　料金＝０　　　　　　　　　　　　　　　　　*/
		syu_tax		 = 0;												/*　税金＝０　　　　　　　　　　　　　　　　　*/
		base_dis_flg = ON;												/*　税金＝０　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	else																/*　９９９以外の場合（設定額割引）　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( MACRO_WARIBIKI_GO_RYOUKIN <= discount )						/*  料金より割引額の方が大きい場合            */
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( jitu_wari == 1 )										/*　実割引の場合　　　　　　　　　　　　　　　*/
				discount = MACRO_WARIBIKI_GO_RYOUKIN;					/*　割引額＝旧料金　　　　　　　　　　　　　　*/
			ryoukin		 = 0;											/*　　　　　　　　　　　　　　　　　　　　　　*/
			syu_tax		 = 0;											/*　税金＝０　　　　　　　　　　　　　　　　　*/
			base_dis_flg = ON;											/*　税金＝０　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
		else															/*　料金≠０の場合　　　　　　　　　　　　　　*/
			ryoukin = wk_ryo - discount;								/*　料金＝料金＋割引　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	if(( ctyp_flg != 4 )&&( cons_tax_ratio != 0 ))						/*　割引対象が料金かつ　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		cons_tax = ec68( ryoukin,cons_tax_ratio );						/*　消費税額＝割引後料金×消費税率　　　　　　*/
		ryo_b->tax = cons_tax;											/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	tol_dis = tol_dis + discount;										/*　割引額合計＝割引額合計＋割引額　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( ctyp_flg == 1 )													/*　内税の場合　　　　　　　　　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		syu_tax = cons_tax;												/*　消費税を集計用エリアへ　　　　　　　　　　*/
		cons_tax = 0;													/*　消費税を０　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	ryo_b->dis = tol_dis;												/*　駐車料金消費税のセーブ　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	req_p->data[2] = discount;											/*　割引額をセット　　　　　　　　　　　　　　*/
	req_p->data[3] = ryoukin;											/*　割引後額をセット　　　　　　　　　　　　　*/
	req_p->data[4] = cons_tax;											/*　消費税額をセット　　　　　　　　　　　　　*/
	ryo_buf.dis_fee += discount;										/*　前回料金割引額加算　　　　　　　　　　　　*/
	ryo_buf.fee_amount += req_rkn.data[0];								/*　料金割引額加算　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
}																		/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/**********************************************/
