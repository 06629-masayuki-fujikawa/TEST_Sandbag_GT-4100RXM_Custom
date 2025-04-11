/*[]----------------------------------------------------------------------[]*/
/*| PARKiPRO対応                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        : 2007-03-23                                               |*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdio.h>
#include	"system.h"
#include	"common.h"
#include	"ope_ifm.h"
#include	"ifm_ctrl.h"
#include	"prm_tbl.h"
#include	"message.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"tbl_rkn.h"
#include	"ope_def.h"
#include	"LKmain.h"


ushort		OpeImfStatus;		/* 減額/振替精算ステータスフラグ */

/*[]----------------------------------------------------------------------[]
 *|	name	: ope_imf_Init
 *[]----------------------------------------------------------------------[]
 *| summary	: 遠隔精算初期化
 *| return	: 
 *| NOTE	: 
 *[]----------------------------------------------------------------------[]*/
void	ope_imf_Init(void)
{
	OpeImfStatus = OPEIMF_STS_IDLE;
	memset(&vl_ggs, 0, sizeof(vl_ggs));
	memset(&vl_frs, 0, sizeof(vl_frs));
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ope_imf_Start
 *[]----------------------------------------------------------------------[]
 *| summary	: 遠隔精算受付開始
 *| return	: 
 *| NOTE	: 
 *[]----------------------------------------------------------------------[]*/
void	ope_imf_Start(void)
{
	OpeImfStatus = OPEIMF_STS_DSPRKN;
	memset(&vl_ggs, 0, sizeof(vl_ggs));
	memset(&vl_frs, 0, sizeof(vl_frs));
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ope_imf_Pay
 *[]----------------------------------------------------------------------[]
 *| summary	: 減額/振替電文受信通知
 *| return	: 
 *| NOTE	: 
 *[]----------------------------------------------------------------------[]*/
BOOL	ope_imf_Pay(ushort kind, void *param)
{
	if (OpeImfStatus != OPEIMF_STS_DSPRKN) {
		return FALSE;	// 現在料金表示中でなければNG
	}

	// 電文内容取得/キューセット
	if (kind == OPEIMF_RCVKIND_GENGAKU) {
	// 減額精算
		struct VL_GGS	*ggs = (struct VL_GGS*)param;
		vl_ggs = *ggs;
		OpeImfStatus = OPEIMF_STS_GGK_RCV;
		queset(OPETCBNO, IFMPAY_GENGAKU, 0, NULL);
	} else {
	// 振替精算
		struct VL_FRS	*frs = (struct VL_FRS*)param;
		vl_frs = *frs;
		OpeImfStatus = OPEIMF_STS_FRK_RCV;
		queset(OPETCBNO, IFMPAY_FURIKAE, 0, NULL);
	}
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ope_imf_Answer
 *[]----------------------------------------------------------------------[]
 *| summary	: 遠隔精算状態通知
 *| return	: 
 *| NOTE	: 
 *[]----------------------------------------------------------------------[]*/
void	ope_imf_Answer(ushort sts)
{
	switch (OpeImfStatus) {
	case OPEIMF_STS_GGK_RCV:
		if (sts) {
			OpeImfStatus = OPEIMF_STS_GGK_ACP;
			IFM_Snd_GengakuAns(1);				// 要求受付応答
		}
		else {
			OpeImfStatus = OPEIMF_STS_DSPRKN;	// 要求受信以前に戻る
			IFM_Snd_GengakuAns( OPECTL.InquiryFlg ? -7:-1 );			// 要求拒否応答
		}
		break;
	case OPEIMF_STS_GGK_ACP:
		OpeImfStatus = OPEIMF_STS_GGK_DONE;
		IFM_Snd_GengakuAns((sts)? 2 : -2);		// 完了時の応答
		break;
	case OPEIMF_STS_FRK_RCV:
		if (sts) {
			OpeImfStatus = OPEIMF_STS_FRK_ACP;
			IFM_Snd_FurikaeAns(1, &vl_frs);		// 要求受付応答
		}
		else {
			OpeImfStatus = OPEIMF_STS_DSPRKN;	// 要求受信以前に戻る
			IFM_Snd_FurikaeAns( (OPECTL.InquiryFlg ? -7:-1), &vl_frs);	// 要求拒否応答
		}
		break;
	case OPEIMF_STS_FRK_ACP:
		OpeImfStatus = OPEIMF_STS_FRK_DONE;
		IFM_Snd_FurikaeAns((sts)? 2 : -2, &vl_frs);		// 完了時の応答
		break;
	default:
		// 応答なし
		break;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ope_imf_End
 *[]----------------------------------------------------------------------[]
 *| summary	: 遠隔精算受付終了
 *| return	: 
 *| NOTE	: 
 *[]----------------------------------------------------------------------[]*/
void	ope_imf_End(void)
{
	switch (OpeImfStatus) {
	case OPEIMF_STS_GGK_RCV:
		// 受付拒否
		IFM_Snd_GengakuAns(-1);				// 要求受付前に精算中止
		break;
	case OPEIMF_STS_GGK_ACP:
		// 精算中止
		IFM_Snd_GengakuAns(-3);				// 要求受付後に精算中止
		break;
	case OPEIMF_STS_FRK_RCV:
		// 受付拒否
		IFM_Snd_FurikaeAns(-1, &vl_frs);	// 要求受付前に精算中止
		break;
	case OPEIMF_STS_FRK_ACP:
		// 精算中止
		IFM_Snd_FurikaeAns(-3, &vl_frs);	// 要求受付後に精算中止
		break;
	default:
		// 完了 → 応答なし
		break;
	}
	OpeImfStatus = OPEIMF_STS_IDLE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ope_imf_GetStatus
 *[]----------------------------------------------------------------------[]
 *| summary	: 遠隔精算状態取得
 *| return	: 0（通常）、1（減額）、2（振替）
 *| NOTE	: 
 *[]----------------------------------------------------------------------[]*/
ushort	ope_imf_GetStatus(void)
{
	if (OpeImfStatus >= OPEIMF_STS_FRK_ACP)
		return 2;		// 振替受付以降なら振替精算中
	if (OpeImfStatus >= OPEIMF_STS_GGK_ACP)
		return 1;		// 減額受付以降なら減額精算中
	return 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ope_ifm_GetFurikaeGaku
 *[]----------------------------------------------------------------------[]
 *| summary	: 振替金額計算
 *| return	: void
 *| NOTE	: 振替精算券データポインタ
 *[]----------------------------------------------------------------------[]*/
void	ope_ifm_GetFurikaeGaku(struct VL_FRS *frs)
{
uchar	wari_tim;

	if( OPECTL.Pay_mod == 2 ){		// 修正精算
		wari_tim = (uchar)((prm_get(COM_PRM, S_TYP, 62, 1, 2)==2)? 1:0);
	}else{							// 振替精算
		wari_tim = (uchar)prm_get(COM_PRM, S_CEN, 32, 1, 1);
	}

	frs->zangaku = ryo_buf.zankin;
	if( frs->syubetu == (char)(ryo_buf.syubet+1) ||	wari_tim == 1 ){
		frs->furikaegaku = frs->price;
	}else{
		frs->furikaegaku = frs->in_price;
	}

	if( frs->furikaegaku > frs->zangaku ){
		frs->furikaegaku = frs->zangaku;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ope_ifm_ExecFurikae
 *[]----------------------------------------------------------------------[]
 *| summary	: 振替額計上
 *| return	: void
 *[]----------------------------------------------------------------------[]*/
void	ope_ifm_FurikaeCalc(uchar type)
{
//振替処理実行関数
	vl_now = V_FRK;					// 振替精算
	if( type == 1 ){				// 修正精算のとき
		vl_now = V_SSS;				// 修正精算
	}
	ope_ifm_GetFurikaeGaku(&vl_frs);
	if( type == 1 ){				// 修正精算のとき
		ryo_SyuseiRyobufSet();
	}
	ryo_cal(3, OPECTL.Pr_LokNo);	// サービス券として計算
}
