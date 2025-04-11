/**********************************************************************************************************************/
/*                                                                                                                    */
/*  関数名称          ：減額精算電文処理            記述                                                              */
/*                                                ：--------------------------------------------------------------：  */
/*  関数シンボル      ：et95()                    ：  減額精算電文処理                                            ：  */
/*                                                ：                                                              ：  */
/*                                                ：--------------------------------------------------------------：  */
/*                                                ：入力パラメータ              出力パラメータ                    ：  */
/*                                                ：  無し                        無し                            ：  */
/*                                                ：                                                              ：  */
/*                                                ：--------------------------------------------------------------：  */
/**********************************************************************************************************************/
#include	<string.h>
#include	"system.h"
#include	"mem_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"Tbl_rkn.h"
#include	"prm_tbl.h"
// MH322914(S) K.Onodera 2016/08/08 AI-V対応：遠隔精算(精算金額指定)
#include	"ope_def.h"
// MH322914(E) K.Onodera 2016/08/08 AI-V対応：遠隔精算(精算金額指定)

void	et95()
{
	struct	REQ_RKN	*req_p;			/*  現在計算後格納エリア                      */
	struct	RYO_BUF_N	*r;			/*  現在計算後格納エリア                      */
	long	wk_ryo = 0;

	req_p		= &req_rhs[tb_number];				/*  現在料金計算要求ポインター                */
	r			= &ryo_buf_n;						/*  現在料金計算要求ポインター                */

	discount	= 0l;								/*  割引額０クリア                            */
	memmove( req_p, &req_rkn, sizeof(req_rkn) );	/*  現在料金計算要求TBLを計算結果TBLへセット  */

	wk_ryo 	 = r->ryo - r->dis + r->tax;			/*  割引額＝料金                              */
	cons_tax = 0;									/*  税金＝０（使用しない）                    */

// MH322914(S) K.Onodera 2016/08/08 AI-V対応：遠隔精算(精算金額指定)
//	discount = (long)(vl_ggs.waribikigaku);
	// 遠隔精算中割引？
// 不具合修正(S) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
//	if( vl_now == V_DIS_FEE ){
	if( req_rkn.param == RY_GNG_FEE ){
// 不具合修正(E) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
		discount = (long)(g_PipCtrl.stRemoteFee.Discount);
	}
// 不具合修正(S) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
//	else if(  vl_now == V_DIS_TIM ){
	else if( req_rkn.param == RY_GNG_TIM ){
// 不具合修正(E) K.Onodera 2017/01/12 連動評価指摘(遠隔精算割引の種別が正しくセットされない)
		discount = (long)(g_PipCtrl.stRemoteTime.Discount);
	}
	// 減額精算？
	else{
		discount = (long)(vl_ggs.waribikigaku);
	}
// MH322914(E) K.Onodera 2016/08/08 AI-V対応：遠隔精算(精算金額指定)

	if (MACRO_WARIBIKI_GO_RYOUKIN <= discount)		/*  割引額が基本料金より大きい場合            */
	{												/*                                            */
		discount =  MACRO_WARIBIKI_GO_RYOUKIN;		/*    割引後の料金を求める      			  */
		tol_dis  += discount;						/*  割引額合計＝割引額合計＋割引額            */

		ryoukin  	 = 0;							/*  割引後の料金を求める                      */
		syu_tax  	 = 0;							/*  税金＝０                                  */
		base_dis_flg = ON;							/*  基本料金全額割引フラグＯＮ                */

	} else  {

		ryoukin  = wk_ryo  - discount;				/*  割引額＜料金   割引後の料金を求める       */
		tol_dis += discount;						/*  割引額合計＝割引額合計＋割引額            */
	}

	r->dis			= tol_dis;						/*  駐車料金消費税のセーブ                    */
	req_p->data[2]	= discount;						/*  割引額をセット                            */
	req_p->data[3]	= ryoukin;						/*  割引後額をセット                          */
	req_p->data[4]	= cons_tax;						/*  消費税額をセット                          */
													/*                                            */
	return ;
}
