/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	 エラー出力																								   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author      : S.Takahashi																				   |*/
/*| Date        : 2012-09-10																				   |*/
/*|																											   |*/
/*[]------------------------------------------------------------------------- Copyright(C) 2011 AMANO Corp.---[]*/

#include	"system.h"
#include	"mem_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"ksgmac.h"
#include	"ksgRauModem.h"
#include	"ksg_def.h"
#include	"raudef.h"

extern unsigned short		reception_level;

/*[]------------------------------------------------------------------------------------------[]*/
/*|	ｼｽﾃﾑｴﾗｰ発生/解除関数																       |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : void RauSysErrSet()														   |*/
/*| PARAMETER1	: unsigned char		: ｴﾗｰ番号												   |*/
/*| PARAMETER2	: unsigned char		: 0:解除 1:発生											   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	RauSysErrSet( unsigned char n , unsigned char s )
{
	ulong	binData;
	char	err_mdl;

	if( s >= 2 ) {
		return;
	}
	err_mdl = ERRMDL_FOMAIBK;
	if(_is_credit_only()){			// クレジットのみ
		err_mdl = ERRMDL_CREDIT;
	}
	switch ( n ) {
	// 発生/解除
	case RAU_ERR_FMA_MODEMERR_DR:										// (74)DR信号が 3秒経過してもONにならない(MODEM ON時)
	case RAU_ERR_FMA_MODEMERR_CD:										// (75)CD信号が60秒経過してもOFFにならない(MODEM OFF時)
	case RAU_ERR_FMA_PPP_TIMEOUT:										// (76)PPPコネクションタイムアウト
	case RAU_ERR_FMA_RESTRICTION:										// (77)RESTRICTION(規制中)受信
	case RAU_ERR_FMA_NOCARRIER:											// (34)NO CARRIRE検出
	case RAU_ERR_FMA_SIMERROR:											// (35)SIMカード外れ検知
	case RAU_ERR_FMA_ADAPTER_NG:										// FOMAアダプタ設定不良
		err_chk(err_mdl, (char)n , (char)s, 0, 0);
		break;

	// 発生+解除
	case RAU_ERR_FMA_ANTENALOW1:										// (31)接続時アンテナレベル1検出
	case RAU_ERR_FMA_ANTENALOW2:										// (32)接続時アンテナレベル2検出
		binData = reception_level;
		err_chk2(err_mdl, (char)n , 2, 2, 0, &binData);
		break;
	case RAU_ERR_FMA_ANTENALOW:											// (79)接続時アンテナレベル0検出
		binData = reception_level;
		err_chk2(err_mdl, (char)n , 2, 2, 0, &binData);
		break;
	case RAU_ERR_FMA_REDIALOV:											// (94)リダイアル回数オーバー
		err_chk(err_mdl, (char)n , 2, 0, 0);
		break;
	case RAU_ERR_FMA_MODEMPOWOFF:										// (78)MODEM 電源OFF
		// 電波受信状況確認によるE7778は出力しない
		if(KSG_AntAfterPppStarted == 0){
			err_chk(err_mdl, (char)n , 2, 0, 0);
		}
		break;

	// その他
	default:
		break;
	}
}
