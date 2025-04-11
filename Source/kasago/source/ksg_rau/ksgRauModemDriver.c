/************************************************************************************************/
/*																								*/
/*	Ӽޭ�ٖ���	:ksgRauModemDriver.c:---------------------------------------------------------: */
/*	Ӽޭ�ټ����	:					:�رْʐM��ײ�� 										  : */
/*																								*/
/************************************************************************************************/
																		/*							*/
#include 	<machine.h>													/* 							*/
#include 	"ksgmac.h"													/* MAC ���ڋ@�\ͯ��			*/
#include 	<string.h>													/* 							*/
#include	"ksgRauModem.h"												/* ���ъ֘A�֐��S			*/
#include	"ksgRauModemData.h"											/* ���ъ֘A�ް���`			*/
#include 	"ksgRauTable.h"												/* ð����ް���`			*/
																		/*							*/
unsigned char	KSG_mdm_Tm_1000ms;										/* 1000msec ����RAM			*/
unsigned char	KSG_mdm_Tm_60s;											/* 60sec ����RAM			*/
extern	void	KSG_RauTResetOut( void );								/*							*/
																		/*							*/
/*[]------------------------------------------------------------------------------------------[]*/
/*|	��ς̏�����																				   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdmTimerInit();													   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdmTimerInit( void )
{																		/*							*/
	KSG_Tm_ReDial.tm = 0;												/* ��޲�يԊu��� ؾ��		*/
	KSG_Tm_Gurd.tm = 0;													/* �޲�ٶް����� ؾ��		*/
	KSG_Tm_No_Action.tm = 0;											/* ���ʐM��� ؾ��			*/
	KSG_Tm_TCP_CNCT.tm = 0;												/* TCP�ȸ��ݐڑ��҂���� ؾ��*/
	KSG_Tm_DISCONNECT.tm= 0;											/* �ȸ��ݐؒf�҂���� ؾ��	*/
	KSG_Tm_TCP_DISCNCT.tm = 0;											/* TCP�ȸ��ݐؒf�҂���� ؾ��*/
	KSG_Tm_DataRetry.tm = 0;											/*							*/
	KSG_f_data_rty_ov = 1;												/*							*/
	KSG_mdm_Tm_1000ms = 0;												/*							*/
	KSG_mdm_Tm_60s = 0;													/*							*/
	KSG_Tm_ERDR_TIME.tm = 0;
}																		/*							*/

/*[]------------------------------------------------------------------------------------------[]*/
/*|	10ms��Ϗ���																				   |*/
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
/*|	100ms��Ϗ���																			   |*/
/*[]------------------------------------------------------------------------------------------[]*/
/*| MODULE NAME : KSG_RauMdm100msInt();														   |*/
/*[]------------------------------------------------------------------------------------------[]*/
void	KSG_RauMdm100msInt( void )
{																		/*							*/
	if( 0 != KSG_mdm_f_TmStart ) {										/*							*/
		if( KSG_f_Gurd_rq == 1 ) 				KSG_Tm_Gurd.tm++;		/* �޲�ٶް����϶���		*/
		if( KSG_Tm_AtCom.BIT.f_rq == 1 )		KSG_Tm_AtCom.tm++;		/* AT����޶������			*/
		if( KSG_f_DialConn_rq == 1 ) 			KSG_Tm_DialConn.tm++;	/* ����ڑ��҂���� 60�b���	*/
		if( KSG_f_TCPcnct_rq == 1 ) 			KSG_Tm_TCP_CNCT.tm++;	/* TCP�ȸ��ݐڑ��҂����	(&UDP)*/
		if( KSG_f_discnnect_rq == 1 )			KSG_Tm_DISCONNECT.tm++;	/* �ȸ��ݐؒf�҂����		*/
		if( KSG_f_Ackwait_rq == 1 )				KSG_Tm_Ackwait.tm++;	/* ACK�҂����				*/
		if( KSG_f_ERDRtime_rq == 1 )			KSG_Tm_ERDR_TIME.tm++;	// ER > DR �Ď����			//
		KSG_RauTResetOut();												/* �@��ؾ�ďo�͏���			*/
		KSG_mdm_Tm_1000ms++;											/*							*/
		if( KSG_mdm_Tm_1000ms >= 10 ) {									/* 1�b���					*/
			KSG_mdm_Tm_1000ms = 0;										/*							*/
			if( KSG_f_ReDial_rq == 1 ) 			KSG_Tm_ReDial.tm++;		/* ��޲�ّ҂���϶���		*/
			if( KSG_f_TCPdiscnct_rq == 1 ) 		KSG_Tm_TCP_DISCNCT.tm++;/* TCP�ȸ��ݐؒf�҂����(�b)	*/
			if( KSG_f_No_Action_rq == 1 ) 		KSG_Tm_No_Action.tm++;	/* ���ʐM���(�b)			*/
			if( KSG_f_ReDialTime_rq == 1 ) 		KSG_Tm_ReDialTime.tm++;	/* ��޲�ْ����(180��)		*/
			KSG_mdm_Tm_60s++;											/*							*/
			if( KSG_mdm_Tm_60s >= 60 ) {								/* 1�����					*/
				KSG_mdm_Tm_60s = 0;										/*							*/
				if( KSG_f_data_rty_rq == 1 ) 	KSG_Tm_DataRetry.tm++;	/* �āX���đ҂����(��)(TCP)	*/
				if( KSG_Tm_DopaReset.BIT.f_rq == 1){					/* �v������?(Y)				*/
					KSG_Tm_DopaReset.tm++;								/* HOST����������DOPAؾ��	*/
				}														/*							*/
			}															/*							*/
		}																/*							*/
	}																	/*							*/
}																		/*							*/
																		/*							*/
