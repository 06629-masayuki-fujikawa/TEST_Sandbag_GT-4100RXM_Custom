/**********************************************************************************************************************/
/*                                                                                                                    */
/*  関数名称          ：振替精算電文処理            記述                                                              */
/*                                                ：--------------------------------------------------------------：  */
/*  関数シンボル      ：et96()                    ：  振替精算電文処理                                            ：  */
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
// MH322914(S) K.Onodera 2016/09/13 AI-V対応：振替精算
#include	"ntnet_def.h"
// MH322914(E) K.Onodera 2016/09/13 AI-V対応：振替精算

void	et96()
{
	struct	REQ_RKN	*req_p;			/*  現在計算後格納エリア                      */
	struct	RYO_BUF_N	*r;			/*  現在計算後格納エリア                      */
	long	time;

	req_p		= &req_rhs[tb_number];				/*  現在料金計算要求ポインター                */
	r			= &ryo_buf_n;						/*  現在料金計算要求ポインター                */

	discount	= 0l;								/*  割引額０クリア                            */
	memmove( req_p, &req_rkn, sizeof(req_rkn) );	/*  現在料金計算要求TBLを計算結果TBLへセット  */

	cons_tax = 0;									/*  税金＝０（使用しない）                    */

	if (vl_frs.furikaegaku >= vl_frs.zangaku) {
		discount = vl_frs.zangaku;		// 割引額
		tol_dis  += vl_frs.zangaku;		// 割引額合計

		ryoukin  	 = 0;				// 割引後料金
		syu_tax  	 = 0;
		base_dis_flg = ON;

	} else  {

		discount =  vl_frs.furikaegaku;	// 割引額
		ryoukin  = vl_frs.zangaku  - vl_frs.furikaegaku;	// 割引後料金
		tol_dis += discount;			// 割引額合計
	}

	r->dis			= tol_dis;						/*  駐車料金消費税のセーブ                    */
	req_p->data[2]	= discount;						/*  割引額をセット                            */
	req_p->data[3]	= ryoukin;						/*  割引後額をセット                          */

	time = (vl_frs.seisan_time.Mon/10)*16 + (vl_frs.seisan_time.Mon%10);
	time <<= 8;
	time += (vl_frs.seisan_time.Day/10)*16 + (vl_frs.seisan_time.Day%10);
	time <<= 8;
	time += (vl_frs.seisan_time.Hour/10)*16 + (vl_frs.seisan_time.Hour%10);
	time <<= 8;
	time += (vl_frs.seisan_time.Min/10)*16 + (vl_frs.seisan_time.Min%10);
	req_p->data[4]	= time;							// 振替元・精算時刻として使用

	return ;
}
