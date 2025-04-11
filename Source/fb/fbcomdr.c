/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		���C���[�_�^�X�N �h���C�o�[�֐�																		   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	fbcomdr.c																				   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| �ESCI��M�f�[�^��COM�C���^�[�t�F�[�X																	   |*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
#include	<string.h>															/*								*/
																				/*								*/
#include	"system.h"															/*								*/
#include	"iodefine.h"														/*								*/
#include	"fbcom.h"															/*								*/
																				/*								*/
/*[]--- �������ߐ錾 -----------------------------------------------------------------------------------------[]*/
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
// MH810103 GG119202(S) IC�N���W�b�g�Ή��iPCI-DSS�j
	memset(FB_RcvBuf, 0, sizeof(FB_RcvBuf));
// MH810103 GG119202(E) IC�N���W�b�g�Ή��iPCI-DSS�j
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
	if( FBcom_2mTimeout() == 0 ){												/* ��������ѱ�ĂȂ�				*/
		if( FBcom_20mTimeout2()== 0 ){											/* ������M��ѱ�ĂȂ�			*/
			return;																/* No							*/
		}																		/*								*/
	}																			/*								*/
																				/*								*/
	_di();
	FBcomdr_CharSetToRcvArea();													/* ��M����Ĕ���				*/
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
	FBcomdr_SciErrorState = FB_RcvCtrl.ComerrStatus;							/* �V���A���G���[��Ԃ�ݒ�		*/
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
