/************************************************************************************************/
/*																								*/
/*	ﾓｼﾞｭｰﾙ名称	:ksgRauModemDriver.c:---------------------------------------------------------: */
/*	ﾓｼﾞｭｰﾙｼﾝﾎﾞﾙ	:					:ｼﾘｱﾙ通信ﾄﾞﾗｲﾊﾞ 										  : */
/*																								*/
/************************************************************************************************/
																		/*							*/
#include 	<machine.h>													/* 							*/
#include 	"ksgmac.h"													/* MAC 搭載機能ﾍｯﾀﾞ			*/
#include 	<string.h>													/* 							*/
#include	"ksgRauModem.h"												/* ﾓﾃﾞﾑ関連関数郡			*/
#include	"ksgRauModemData.h"											/* ﾓﾃﾞﾑ関連ﾃﾞｰﾀ定義			*/
#include 	"ksgRauTable.h"												/* ﾃｰﾌﾞﾙﾃﾞｰﾀ定義			*/
																		/*							*/
unsigned char	KSG_mdm_Tm_1000ms;										/* 1000msec ｶｳﾝﾄRAM			*/
unsigned char	KSG_mdm_Tm_60s;											/* 60sec ｶｳﾝﾄRAM			*/
extern	void	KSG_RauTResetOut( void );								/*							*/
																		/*							*/
/*[]------------------------------------------------------------------------------------------[]*/
/*|	ﾀｲﾏの初期化																				   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmTimerInit();													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmTimerInit( void )
{																		/*							*/
	KSG_Tm_ReDial.tm = 0;												/* ﾘﾀﾞｲｱﾙ間隔ﾀｲﾏ ﾘｾｯﾄ		*/
	KSG_Tm_Gurd.tm = 0;													/* ﾀﾞｲｱﾙｶﾞｰﾄﾞﾀｲﾏ ﾘｾｯﾄ		*/
	KSG_Tm_No_Action.tm = 0;											/* 無通信ﾀｲﾏ ﾘｾｯﾄ			*/
	KSG_Tm_TCP_CNCT.tm = 0;												/* TCPｺﾈｸｼｮﾝ接続待ちﾀｲﾏ ﾘｾｯﾄ*/
	KSG_Tm_DISCONNECT.tm= 0;											/* ｺﾈｸｼｮﾝ切断待ちﾀｲﾏ ﾘｾｯﾄ	*/
	KSG_Tm_TCP_DISCNCT.tm = 0;											/* TCPｺﾈｸｼｮﾝ切断待ちﾀｲﾏ ﾘｾｯﾄ*/
	KSG_Tm_DataRetry.tm = 0;											/*							*/
	KSG_f_data_rty_ov = 1;												/*							*/
	KSG_mdm_Tm_1000ms = 0;												/*							*/
	KSG_mdm_Tm_60s = 0;													/*							*/
	KSG_Tm_ERDR_TIME.tm = 0;
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	10msﾀｲﾏ処理																				   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdm10msInt();														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdm10msInt( void )
{																		/*							*/
	if( 0 != KSG_mdm_f_TmStart ) {										/*							*/
		KSG_RauSciLineCheck();											/*							*/
	}																	/*							*/
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	100msﾀｲﾏ処理																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdm100msInt();														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdm100msInt( void )
{																		/*							*/
	if( 0 != KSG_mdm_f_TmStart ) {										/*							*/
		if( KSG_f_Gurd_rq == 1 ) 				KSG_Tm_Gurd.tm++;		/* ﾀﾞｲｱﾙｶﾞｰﾄﾞﾀｲﾏｶｳﾝﾀ		*/
		if( KSG_Tm_AtCom.BIT.f_rq == 1 )		KSG_Tm_AtCom.tm++;		/* ATｺﾏﾝﾄﾞｶｳﾝﾄﾀｲﾏ			*/
		if( KSG_f_DialConn_rq == 1 ) 			KSG_Tm_DialConn.tm++;	/* 回線接続待ちﾀｲﾏ 60秒ﾀｲﾏ	*/
		if( KSG_f_TCPcnct_rq == 1 ) 			KSG_Tm_TCP_CNCT.tm++;	/* TCPｺﾈｸｼｮﾝ接続待ちﾀｲﾏ	(&UDP)*/
		if( KSG_f_discnnect_rq == 1 )			KSG_Tm_DISCONNECT.tm++;	/* ｺﾈｸｼｮﾝ切断待ちﾀｲﾏ		*/
		if( KSG_f_Ackwait_rq == 1 )				KSG_Tm_Ackwait.tm++;	/* ACK待ちﾀｲﾏ				*/
		if( KSG_f_ERDRtime_rq == 1 )			KSG_Tm_ERDR_TIME.tm++;	// ER > DR 監視ﾀｲﾏ			//
		KSG_RauTResetOut();												/* 機器ﾘｾｯﾄ出力処理			*/
		KSG_mdm_Tm_1000ms++;											/*							*/
		if( KSG_mdm_Tm_1000ms >= 10 ) {									/* 1秒ﾀｲﾏ					*/
			KSG_mdm_Tm_1000ms = 0;										/*							*/
			if( KSG_f_ReDial_rq == 1 ) 			KSG_Tm_ReDial.tm++;		/* ﾘﾀﾞｲｱﾙ待ちﾀｲﾏｶｳﾝﾀ		*/
			if( KSG_f_TCPdiscnct_rq == 1 ) 		KSG_Tm_TCP_DISCNCT.tm++;/* TCPｺﾈｸｼｮﾝ切断待ちﾀｲﾏ(秒)	*/
			if( KSG_f_No_Action_rq == 1 ) 		KSG_Tm_No_Action.tm++;	/* 無通信ﾀｲﾏ(秒)			*/
			if( KSG_f_ReDialTime_rq == 1 ) 		KSG_Tm_ReDialTime.tm++;	/* ﾘﾀﾞｲﾔﾙ中ﾀｲﾏ(180分)		*/
			KSG_mdm_Tm_60s++;											/*							*/
			if( KSG_mdm_Tm_60s >= 60 ) {								/* 1分ﾀｲﾏ					*/
				KSG_mdm_Tm_60s = 0;										/*							*/
				if( KSG_f_data_rty_rq == 1 ) 	KSG_Tm_DataRetry.tm++;	/* 再々発呼待ちﾀｲﾏ(分)(TCP)	*/
				if( KSG_Tm_DopaReset.BIT.f_rq == 1){					/* 要求あり?(Y)				*/
					KSG_Tm_DopaReset.tm++;								/* HOST無応答時のDOPAﾘｾｯﾄ	*/
				}														/*							*/
			}															/*							*/
		}																/*							*/
	}																	/*							*/
}																		/*							*/
																		/*							*/
