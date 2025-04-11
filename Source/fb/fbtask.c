/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|		���C���[�_�^�X�N �h���C�o�[�֐�																		   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*|	FILE NAME	:	fbtask.c																				   |*/
/*|																											   |*/
/*[]----------------------------------------------------------------------------------------------------------[]*/
/*| �EFB7000�^�X�N���C������																				   |*/
/*|																											   |*/
/*[]------------------------------------------------------------------ Copyright(C) 2005 AMANO Corp.----------[]*/
#include	"system.h"										/*						*/
#include	"prm_tbl.h"
#include	"fbcom.h"										/*						*/

unsigned char	FB_Enabled;

void	FBtask(void)
{
	FB_Enabled = (unsigned char)prm_get(COM_PRM, S_PAY, 21, 1, 3);

	FBcom_Init();
	FBcomdr_RcvInit();
	FBcomdr_RcvLength = 0;									/* temporary buffer clear		*/

	while(1){
		taskchg( IDLETSKNO );

		if(FB_Enabled != 0) {
			FBcomdr_Main();
			FBcom_Main();
		}
	}

	return;
}
