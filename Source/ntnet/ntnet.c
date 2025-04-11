/*[]----------------------------------------------------------------------[]
 *| System      : UT8500
 *| Module      : NT-NET通信モジュール
 *[]----------------------------------------------------------------------[]
 *| Author      : machida kei
 *| Date        : 2005.08.23
 *| Update      :
 *|		2005.09.21	machida.k	初期化中フラグ(z_NTNET_AtInitial)追加
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*			I N C L U D E													*/
/*--------------------------------------------------------------------------*/

#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"common.h"
#include	"prm_tbl.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"ntnetauto.h"

/*--------------------------------------------------------------------------*/
/*			D E F I N I T I O N S											*/
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (backuped)									*/
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (not backuped)								*/
/*--------------------------------------------------------------------------*/

BOOL	z_NTNET_AtInitial;

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/

/*====================================================================================[PUBLIC]====*/


/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_Init
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET通信機能初期化
 *| param	: clr - 1=停電保証データもクリア
 *|			        0=停電保証データはクリアしない
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTNET_Init(uchar clr)
{
	z_NTNET_AtInitial = TRUE;
	Ntnet_Remote_Comm = 0;

	if (Ntnet_Remote_Comm == 0) {
		Ntnet_Remote_Comm = (uchar)prm_get( COM_PRM,S_PAY,24,1,1 );
	}
	NTBUF_Init(clr);
	NTUPR_Init(clr);
	NTLWR_Init();
	NT_pcars_timer = -1;
	if( (prm_get(COM_PRM,S_NTN,121,1,1)!=0) &&
		(prm_get(COM_PRM,S_NTN,120,3,1)!=0) ) {
		NT_pcars_timer = 0;													// POW ON で1回送信
	}
	memset(&SData58_bk, 0, sizeof(DATA_KIND_58));
	memset( &ParkCar_data_Bk, 0, sizeof(ParkCar_data));
	z_NTNET_AtInitial = FALSE;
	memset(&ntNet_152_SaveData, 0, sizeof(ntNet_152_SaveData));
	ryo_test_flag = 0;

	PayInfo_Class = 8;								// 精算情報データ用処理区分

}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_Err
 *[]----------------------------------------------------------------------[]
 *| summary	: エラー通知IF
 *| param	: errcode - 発生したエラー
 *|			  occur - NTERR_EMERGE=発生
 *|			          NTERR_RELEASE=解除
 *|					  NTERR_ONESHOT=発生＆解除
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTNET_Err(int errcode, int occur)
{
	switch (occur) {
	case NTERR_EMERGE:
		err_chk(ERRMDL_NTNET, (char)errcode, NTERR_EMERGE, 0, 0);	/* 発生 */
		break;
	case NTERR_ONESHOT:										/* 発生＆解除 */
		err_chk(ERRMDL_NTNET, (char)errcode, NTERR_ONESHOT, 0, 0);
		break;
	default:	/* case NTERR_RELEASE: */					/* 解除 */
		err_chk(ERRMDL_NTNET, (char)errcode, NTERR_RELEASE, 0, 0);
		break;
	}
}

/*====================================================================================[TASK]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: ntnet_task
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET通信タスク本体
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntnet_task(void)
{
	MsgBuf *pmsg;
	MsgBuf msg;
	
	while (1) {
		taskchg(IDLETSKNO);
		
		pmsg = GetMsg(NTNETTCBNO);
		if (pmsg != NULL) {
			memcpy(&msg, pmsg, sizeof(msg));
			FreeBuf(pmsg);
			pmsg = &msg;
		}
		
		NTUPR_Main();
		NTLWR_Main(pmsg);
		ntautoSendCtrl();
	}
}

