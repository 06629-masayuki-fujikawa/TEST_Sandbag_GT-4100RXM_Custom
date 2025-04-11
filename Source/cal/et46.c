/**********************************************************************************************************************/
/*　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　関数名称　　　　　：プリペイドカード処理　　　　記述　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　関数シンボル　　　：et46()　　　　　　　　　　：　プリペイドカードの処理を行うが、設定により算出方法が異なる。：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：--------------------------------------------------------------：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：入力パラメータ　　　　　　　出力パラメータ　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　無し　　　　　　　　　　　　無し　　　　　　　　　　　　　　：　*/
/*　　　　　　　　　　　　　　　　　　　　　　　　：　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　：　*/
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
#include	"tbl_rkn.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
#include	"ope_def.h"													/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
#define		PPC			2												/* ﾌﾟﾘﾍﾟｲﾄﾞｶｰﾄﾞ 　　　　　　　　　　　　　　　*/
void	PPC_Data_Detail_Set( ppc_tiket*	w_ppc_data );
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
void	et46()															/*　プリペイドカード処理　　　　　　　　　　　*/
{																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	long	backlog 	= 0;											/*　残額　　　　　　　　　　　　　　　　　　　*/
	long	card_pay 	= 0;											/*　プリペイドカード使用額　　　　　　　　　　*/
	long	p_ok 		= 0;											/*　プリペイドカード処理ＯＫ額　　　　　　　　*/
	long	p_ng 		= 0;											/*　プリペイドカード処理ＮＧ額　　　　　　　　*/
	long	p_ngok 		= 0;											/*　ワークエリア　　　　　　　　　　　　　　　*/
	long	wk_ryo 		= 0;											/*　料金ｗｏｒｋエリア　　　　　　　　　　　　*/
	struct	REQ_RKN	*req_p;												/*　現在計算後格納エリア　　　　　　　　　　　*/
	struct	RYO_BUF_N	*r;												/*　駐車料金格納エリア　　　　　　　　　　　　*/
	uchar	sub_data  = PPC_Data_Detail.ppc_sub_data.BIT.offset;
	ppc_tiket	*ppc_data;
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
/**********************************************************************************************************************/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	req_p 	= &req_rhs[tb_number];										/*　現在料金計算要求ポインター　　　　　　　　*/
	r	  	= &ryo_buf_n;												/*　現在料金計算要求ポインター　　　　　　　　*/
	memmove( req_p,&req_rkn,sizeof(req_rkn) );							/*　現在料金計算要求TBLを計算結果TBLへセット　*/
	backlog = req_rkn.data[0] + c_pay;									/*　計算要求テーブルの残額に前回使用額を加算　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( r->ryo != 0 )													/*　駐車料金が０円では無い場合　　　　　　　　*/
	{																	/*　　　　　　　　　　　　　　　　　　　　　　*/
		switch( ctyp_flg )												/*　精算形態により分岐　　　　　　　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			case 1:														/*　内税の場合　　　　　　　　　　　　　　　　*/
				if( cons_tax_ratio != 0 )								/*　消費税率が０ではない場合　　　　　　　　　*/
					r->tax = ec68( r->ryo,cons_tax_ratio );				/*　駐車料金消費税＝割引前駐車料金×消費税率　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				p_ok = ( r->ryo - r->dis ) - azukari -  e_pay;			/*　Ｐ処理ＯＫ額算出（電子マネー入金分を加味）*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( p_ok < 0 )											/*　Ｐ処理ＯＫ額が０円未満の場合　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					p_ok   = 0;											/*　Ｐ処理ＯＫ額＝０　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			break;														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			case 2:														/*　０／１設定の場合　　　　　　　　　　　　　*/
				if( cons_tax_ratio != 0 )								/*　消費税率が０ではない場合　　　　　　　　　*/
					r->tax = ec68( (r->ryo - r->dis),cons_tax_ratio );	/*　駐車料金消費税＝割引後駐車料金×消費税率　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				wk_ryo = r->ryo + r->tax - r->dis - azukari ;			/*　払戻処理後料金算出　　　　		931129	  */
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( wk_ryo < 0 )										/*　算出料金が０円未満の場合　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					p_ok = 0;											/*　Ｐ処理ＯＫ額＝０　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else													/*　算出後料金が０円以上の場合　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					if( wk_ryo >= r->tax )								/*　算出後料金≧駐車料金消費税の場合　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						p_ok 	= wk_ryo - r->tax;						/*　Ｐ処理ＯＫ額算出　　　　　　　　　　　　　*/
						p_ng	= r->tax;								/*　Ｐ処理ＮＧ額算出　　　　　　　　　　　　　*/
						p_ngok	= r->tax;								/*　Ｐ処理後駐車料金０円時クリアするＮＧ額　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
					else												/*　算出後料金＜駐車料金消費税の場合　　　　　*/
					{													/*　　　　　　　　　　　　　　　　　　　　　　*/
						p_ok	= 0;									/*　Ｐ処理ＯＫ額＝０　　　　　　　　　　　　　*/
						p_ng 	= wk_ryo;								/*　Ｐ処理ＮＧ額算出　　　　　　　　　　　　　*/
					}													/*　　　　　　　　　　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			break;														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			case 3:														/*　０／０設定の場合　　　　　　　　　　　　　*/
				if( cons_tax_ratio != 0 )								/*　消費税率が０ではない場合　　　　　　　　　*/
					r->tax = ec68( (r->ryo - r->dis),cons_tax_ratio );	/*　駐車料金消費税＝割引後駐車料金×消費税率　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				p_ok = r->ryo + r->tax - r->dis - azukari -  e_pay;		/*　Ｐ処理ＯＫ額算出（電子マネー入金分を加味）*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( p_ok < 0 )											/*　Ｐ処理ＯＫ額が０未満の場合　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					p_ok = 0;											/*　Ｐ処理ＯＫ額＝０　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			break;														/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
			case 4:														/*　１／０・１設定の場合　　　　　　　　　　　*/
				if( cons_tax_ratio != 0 )								/*　消費税率が０ではない場合　　　　　　　　　*/
					r->tax = ec68( r->ryo,cons_tax_ratio );				/*　駐車料金消費税＝割引前駐車料金×消費税率　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				p_ok = r->ryo + r->tax - r->dis - azukari -  e_pay;		/*　Ｐ処理ＯＫ額算出（電子マネー入金分を加味）*/
				if( p_ok < 0 )											/*　Ｐ処理ＯＫ額が０未満の場合　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					p_ok = 0;											/*　Ｐ処理ＯＫ額＝０　　　　　　　　　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
			break;														/*　　　　　　　　　　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
		if( p_ok != 0 )													/*　Ｐ処理ＯＫ額が０円では無い場合　　　　　　*/
		{																/*　　　　　　　　　　　　　　　　　　　　　　*/
			if( backlog >= p_ok )										/*　Ｐ使用可能額≧Ｐ処理ＯＫ額の場合　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				backlog 	 = backlog - p_ok; 							/*　Ｐ使用可能額再算出　　　　　　　　　　　　*/
				card_pay	+= p_ok;									/*　Ｐ支払額算出　　　　　　　　　　　　　　　*/
				p_ok 		 = 0;										/*　Ｐ処理ＯＫ額＝０　　　　　　　　　　　　　*/
				if( ctyp_flg != 3 && ctyp_flg != 4 )	r->tax 	 = 0;	/*　＊／０設定以外の場合　駐車料金消費税＝０　*/
				p_ng = p_ng - p_ngok;									/*　Ｐ使用不可能額から消費税額を除く　　　　　*/
				if( p_ngok != 0 )										/*　P消費税(0ｸﾘｱされた場合)　　　			　*/
					p_taxflg[3] = ON;									/*　　ﾌﾟﾘﾍﾟｲﾄﾞ税金処理flg(駐車料金)ON　		  */
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			else														/*　Ｐ使用可能額＜Ｐ処理ＯＫ額の場合　　　　　*/
			{															/*　　　　　　　　　　　　　　　　　　　　　　*/
				p_ok		 = p_ok - backlog;							/*　Ｐ処理ＯＫ額残額の算出　　　　　　　　　　*/
				card_pay 	+= backlog;									/*　Ｐ支払額算出　　　　　　　　　　　　　　　*/
				backlog		 = 0;										/*　Ｐ残額＝０　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
				if( ctyp_flg == 2 )										/*　０／１設定の場合　　　　　　　　　　　　　*/
				{														/*　　　　　　　　　　　　　　　　　　　　　　*/
					r->tax	= ec68( p_ok,cons_tax_ratio );				/*　駐車料金消費税＝割引後駐車料金×消費税率　*/
					p_ng	= r->tax + p_ok;							/*　Ｐ後駐車料金消費税＋Ｐ後駐車料金　　　　　*/
				}														/*　　　　　　　　　　　　　　　　　　　　　　*/
				else	p_ng += p_ok;									/*　Ｐ処理ＮＧ額にＰ処理ＯＫ残額を加算　　　　*/
			}															/*　　　　　　　　　　　　　　　　　　　　　　*/
			p_ok = 0;													/*　Ｐ処理ＯＫ額＝０　　　　　　　　　　　　　*/
		}																/*　　　　　　　　　　　　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	ryoukin = p_ng;														/*　料金＝Ｐ処理ＮＧ額　　　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( backlog < req_rkn.data[1] )			backlog = 0;				/*　Ｐ残額＜精算単位　Ｐ残額＝０　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	req_p->data[2] 	= card_pay - c_pay;									/*　今回使用額をセット　　　　　　　　　　　　*/
	ryo_buf.fee_amount += req_p->data[2];								/*　割引額加算　　　　　　　　　　　　　　　　*/
	req_p->data[3] 	= backlog;											/*　残高をセット　　　　　　　　　　　　　　　*/
	req_p->data[4] 	= ryoukin;											/*　料金＋消費税のセット　　　　　　　　　　　*/
	c_pay_afr 		= ryoukin;											/*　プリペイドカード使用後料金　　　　　　　　*/
	c_pay 			= card_pay;											/*　現在までの使用額を保存　　　　　　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( ryoukin != 0 )			husoku_flg = ON;						/*　料金がある場合　不足フラグＯＮ　　　　　　*/
	else						husoku_flg = OFF;						/*　                不足フラグＯＦＦ　　　　　*/
																		/*　　　　　　　　　　　　　　　　　　　　　　*/
	if( sub_data < WTIK_USEMAX ){										/*　　　　　　　　　　　　　　　　　　　　　　*/
		ppc_data = &PPC_Data_Detail.ppc_data_detail[sub_data];			/*　　　　　　　　　　　　　　　　　　　　　　*/
		ppc_data->ppc_id = (ulong)CRD_DAT.PRE.cno;						/*　プリペイドカードＮｏ．　　　　　　　　　　*/
		ppc_data->pay_befor = (ulong)CRD_DAT.PRE.ram;					/*　支払前残額　　　　　　　　　　　　　　　　*/
		ppc_data->pay_after = backlog;									/*　支払後残額　　　　　　　　　　　　　　　　*/
		ppc_data->kigen_year = CRD_DAT.PRE.kigen_year;					/*　有効期限年　　　　　　　　　　　　　　　　*/
		ppc_data->kigen_mon = CRD_DAT.PRE.kigen_mon;					/*　有効期限月　　　　　　　　　　　　　　　　*/
		ppc_data->kigen_day = CRD_DAT.PRE.kigen_day;					/*　有効期限日　　　　　　　　　　　　　　　　*/
		PPC_Data_Detail.ppc_sub_data.BIT.offset++;						/*　　　　　　　　　　　　　　　　　　　　　　*/
		ryo_buf.pri_mai[sub_data] = 1L;									/*　ﾌﾟﾘﾍﾟｲﾄﾞ使用回数ｾｯﾄ 　　　　　　　　　　　*/
	}																	/*　　　　　　　　　　　　　　　　　　　　　　*/
	CRD_DAT.PRE.ram = backlog;											/*　　　　　　　　　　　　　　　　　　　　　　*/
	return;																/*                      					  */
}																		/**********************************************/

