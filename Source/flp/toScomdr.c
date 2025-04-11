/*[]----------------------------------------------------------------------[]*/
/*|		�Ύq�@�ʐM�^�X�N ��M�h���C�o��									   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"common.h"
#include	"IFM.h"
#include	"toS.h"
#include	"toSdata.h"


/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
void	toScomdr_Init(void);
void	toScomdr_Main(void);
void	toScomdr_RcvInit(void);


/*[]----------------------------------------------------------------------[]*/
/*|             toScomdr_Init() 	                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		initial routine                                                	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScomdr_Init( void )
{
	toScomdr_f_RcvCmp  = 0;		/* ��M�C�x���g���N���A */
	toScomdr_RcvLength = 0;		/* ��M�f�[�^�����N���A */

	toS_RcvCtrl.RcvCnt   = 0;	/* ��M�f�[�^�����N���A */
	toS_RcvCtrl.OvfCount = 0;	/* ��M�o�b�t�@�I�[�o�[�t���[�����񐔂́A�N�����̂݃��Z�b�g */
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScomdr_Main() 	                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		communication main routine                                  	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScomdr_Main( void )
{
	/*
	*	��M�C�x���g�́A�ȉ��̏��Ŕ����A���o���B
	*	�@��M�G���[���������H�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E2
	*	�A��M�C�x���g�����Z�b�g����Ă��Ȃ��H�E�E�E�E�E�E�E�E�E�E�E�E�E�E��0
	*	�B��M���������H�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E1
	*	�C�����N(�`��)�^�C�}(10msec)���^�C���A�b�v�����H�i���g�p�j�E�E�E�E3
	*	�D��M(�����҂�)�Ď��^�C�}(10msec)�^�C�}���^�C���A�b�v�����H�E�E�E4
	*	������ɂ��Y�����Ȃ���΁A�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E0
	*		�E������M���Ă��Ȃ��B�܂���
	*		�E��M��
	*/

	/* �@��M�G���[���� */
	if (toS_RcvCtrl.ComerrStatus != 0) {
		toScomdr_f_RcvCmp = 2;				/* ��M�G���[������Ԃ� */
		if(toS_RcvCtrl.ComerrStatus & 0x01) {	/* over run set	*/
			IFM_SlaveError(E_IFS_Overrun, E_IF_BOTH, toScom_GetSlaveNow());	/* �I�[�o�[�����G���[[����] */
		}

		if(toS_RcvCtrl.ComerrStatus & 0x02) {	/* frame error set		*/
			IFM_SlaveError(E_IFS_Frame, E_IF_BOTH, toScom_GetSlaveNow());	/* �t���[�~���O�G���[[����] */
		}

		if(toS_RcvCtrl.ComerrStatus & 0x04) {	/* parity error set		*/
			IFM_SlaveError(E_IFS_Parity, E_IF_BOTH, toScom_GetSlaveNow());	/* �p���e�B�G���[[����] */
		}
		toScomdr_RcvInit();
		return;
	}

	/*
	*	�A��M�C�x���g��(���Z�b�g���ꂸ��)���̂܂܎c���Ă���
	*	�EXXXcom_Main()���������Ă��Ȃ����Ƃ������B�E�E�E���蓾�Ȃ��P�[�X
	*/
	if (toScomdr_f_RcvCmp != 0) {
		return;								/* �������ĂȂ�����A�O��C�x���g���̂܂܂�Ԃ��B*/
	}

	/* �B��M���������i�����ԃ^�C���A�E�g�ŁA�^�C�}�[���荞�ݒ��ɁuPAY_RcvCtrl.RcvCmpFlg�v�t���O���Z�b�g�����j*/
	if (toS_RcvCtrl.RcvCmpFlg) {
		toS_RcvCtrl.RcvCmpFlg = 0;			/*�i���m�����̂Łj��M���������Z�b�g	2005.06.13 �����E�ǉ� */

		/* �����Ŏ�M�d������M�����ƔF�����邩��A�^�X�N���x���̎�M�o�b�t�@�֓]�� */
		toScom_PktLength = toScomdr_RcvLength;								/* ��M�d�������m�� */
		memcpy(toScom_RcvPkt, toScomdr_RcvData, (size_t)toScom_PktLength);	/* ��M�d��(�f�[�^)���m�� */

		/*
		*	�����ŁAPAYcomdr_RcvInit()���Ăяo���Ȃ����Ƃɒ��ӁB
		*	�E�Ăяo���ƁA���荞�݃��x���Ńo�b�t�@�����O���Ă����M�f�[�^�����N���A����Ă��܂�
		*/
		toScomdr_f_RcvCmp = 1;				/* ��M������Ԃ� */
		return;
	}

	/* �C����10msec�^�C�}�́A���݁u���g�p�v*/
	if (toScom_LinkTimeout()) {
		toScomdr_f_RcvCmp = 3;				/* �����N���x���Ń^�C���A�E�g�������� */
		toScomdr_RcvInit();
		return;
	}

	/* �D����10msec�^�C�}�́A��M�Ď��i�����҂��j�Ɏg���B*/
	if (toScom_LineTimeout()) {
		toScomdr_f_RcvCmp = 4;				/* �q�@�����R�}���h���M��A�����҂��������o�߂����B�܂��̓|�[�����O�����o�߂��� */
		toScomdr_RcvInit();
		return;
	}

	/* ����ȊO�́u��M�C�x���g�v�Ȃ� */
}

/*[]----------------------------------------------------------------------[]*/
/*|             toScomdr_RcvInit() 	                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|		communication initial routine                                  	   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  Hayase                                                  |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	toScomdr_RcvInit( void )
{
	toS_RcvCtrl.RcvCnt       = 0;		/* ��M�f�[�^�����N���A */
	toS_RcvCtrl.ComerrStatus = 0;		/* ��M�G���[�����Z�b�g */
	toS_RcvCtrl.RcvCmpFlg    = 0;		/* ��M���������Z�b�g	2005.06.13 �����E�ǉ� */
}
