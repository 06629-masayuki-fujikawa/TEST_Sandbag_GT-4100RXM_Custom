/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		磁気リーダタスク ドライバー関数																		   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	fbcomdr.c																				   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| ・SCI受信データのCOMインターフェース																	   |*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
#include	<string.h>															/*								*/
																				/*								*/
#include	"system.h"															/*								*/
#include	"iodefine.h"														/*								*/
#include	"fbcom.h"															/*								*/
																				/*								*/
/*[]--- ﾌﾟﾛﾄﾀｲﾌﾟ宣言 -----------------------------------------------------------------------------------------[]*/
																				/*								*/
void		FBcomdr_Main( void );												/*								*/
void		FBcomdr_CharSetToRcvArea( void );									/*								*/
void		FBcomdr_RcvInit( void );											/*								*/
																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             FBcomdr_RcvInit()																			   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		communication initial routine																		   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
void	FBcomdr_RcvInit( void )													/*								*/
{																				/*								*/
// MH810103 GG119202(S) ICクレジット対応（PCI-DSS）
	memset(FB_RcvBuf, 0, sizeof(FB_RcvBuf));
// MH810103 GG119202(E) ICクレジット対応（PCI-DSS）
	FB_RcvCtrl.RcvCnt		= 0;												/*								*/
	FB_RcvCtrl.ReadIndex	= 0;												/*								*/
	FB_RcvCtrl.WriteIndex	= 0;												/*								*/
	FB_RcvCtrl.OvfCount		= 0;											/*								*/
	FB_RcvCtrl.ComerrStatus	= 0;											/*								*/
																				/*								*/
	FBcom_2mTimStop();															/*								*/
	FBcom_20mTimStop2();														/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             FBcomdr_Main()																				   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
void	FBcomdr_Main( void )													/*								*/
{																				/*								*/
																				/*								*/
	if( FBcom_2mTimeout() == 0 ){												/* 文字間ﾀｲﾑｱｳﾄなし				*/
		if( FBcom_20mTimeout2()== 0 ){											/* 文字受信ﾀｲﾑｱｳﾄなし			*/
			return;																/* No							*/
		}																		/*								*/
	}																			/*								*/
																				/*								*/
	_di();
	FBcomdr_CharSetToRcvArea();													/* 受信ｲﾍﾞﾝﾄ判定				*/
	_ei();
	FBcomdr_RcvInit();															/*								*/
}																				/*								*/
																				/*								*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|             FBcomdr_CharSetToRcvArea()																	   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		received character set to temporary area															   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| Author		: S.Takahashi(FSI)																			   |*/
/*| Date		: 2011-10-04																				   |*/
/*| Update      :																							   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
void	FBcomdr_CharSetToRcvArea( void )										/*								*/
{																				/*								*/
																				/*								*/
	if( FBcomdr_f_RcvCmp != 0){													/* received already received(Y)	*/
		return;
	}																			/*								*/
																				/*								*/
	if( FB_RcvCtrl.ComerrStatus != 0 ){											/*								*/
	FBcomdr_SciErrorState = FB_RcvCtrl.ComerrStatus;							/* シリアルエラー状態を設定		*/
		FBcomdr_f_RcvCmp = 2;													/*								*/
		return;
	}																			/*								*/
																				/*								*/
	if( FB_RcvCtrl.OvfCount != 0 ){ 											/*								*/
		FBcomdr_f_RcvCmp = 5;													/*								*/
		return;
	}																			/*								*/
																				/*								*/
	if( 0 != FB_RcvCtrl.RcvCnt ){												/* received character exist (Y) */
		FBcomdr_RcvLength = FBcom_AsciiToBinary(&FB_RcvBuf[FB_RcvCtrl.ReadIndex],
												FB_RcvCtrl.RcvCnt, &FBcomdr_RcvData[0]);
		FBcomdr_f_RcvCmp = 1;													/*								*/
	}
	else {
		FBcomdr_f_RcvCmp = 3;
	}																			/*								*/
	return;
}																				/*								*/
																				/*------------------------------*/
