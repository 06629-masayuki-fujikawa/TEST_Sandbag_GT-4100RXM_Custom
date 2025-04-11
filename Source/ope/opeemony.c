/*[]----------------------------------------------------------------------[]*/
/*| �d�q�}�l�[�֘A����                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Suzuki                                                  |*/
/*| Date         : 2006-08-02                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"Message.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"mdl_def.h"
#include	"lcd_def.h"
#include	"prm_tbl.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnet_def.h"
#include	"suica_def.h"
#include	"ope_ifm.h"
// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
#include	"cal_cloud.h"
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�


/*[]----------------------------------------------------------------------[]*/
/*| �d�q�}�l�[���Z����                                                     |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : al_emony( msg, mod )                                    |*/
/*| PARAMETER    : msg : ү����                                            |*/
/*|                paymod : 0=�ʏ� 1=�C��                                  |*/
/*| RETURN VALUE : ret : ���Z����Ӱ��                                      |*/
/*| 				   :  0 = �����p��                                     |*/
/*|                    :  1 = ���Z����(�ނ薳��) 	                       |*/
/*|                    :  2 = ���Z����(�ނ�L��)    	                   |*/
/*|                    : 10 = ���Z���~(�ҋ@�֖߂�)      	               |*/
/*|					   : 0x105 = ������ʐM�����v�����{�i�����v�����M�ς݁j|*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Suzuki                                                  |*/
/*| Date         : 2006-08-02                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
short	al_emony( ushort msg, ushort paymod )
{
	short	ret = 0;

	switch( msg ){

		case SUICA_EVT:									// Suica�i���ό��ʁF�����ް���M�̂݌ďo�����j
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		case IBK_EDY_RCV:								// Edy�i���ό��ʁF�����ް���M�̂݌ďo�����j
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

			Ope_Last_Use_Card = 3;						// �d�q���σJ�[�h���p

// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
			if ( CLOUD_CALC_MODE ) {	// �N���E�h�����v�Z���[�h
				// �d�q�}�l�[�ݒ�
				cal_cloud_emoney_set();
			} else {					// �ʏ헿���v�Z���[�h
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

			vl_now = V_EMY;								// ���݂u�k�J�[�h�f�[�^�ɓd�q�}�l�[���w��iec09�Ŏg�p�j
			ryo_cal( 2, OPECTL.Pr_LokNo );				// �����v�Z

// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
			}
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

			if( OPECTL.op_faz == 0 ){					// ���ڰ���̪���(0:�������M)
				OPECTL.op_faz = 1;						// 1:������(�����L�莞�ɾ��)
			}
			ac_flg.cycl_fg = 10;						// ���Z�����׸�:����
			ret = in_mony( msg, paymod );				// ��������
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
// MH810103 GG119202(S) ���ϐ��Z���~�󎚏����C��
//			ac_flg.ec_alarm = 0;						// �݂Ȃ����ϕ��d�p�t���OOFF
			ac_flg.ec_deemed_fg = 0;					// �݂Ȃ����ϕ��d�p�t���OOFF
// MH810103 GG119202(E) ���ϐ��Z���~�󎚏����C��
			OPECTL.InquiryFlg = 0;
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//			FurikaeMotoSts = 0;
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
			break;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
		case EC_EVT_CRE_PAID:							// ���σ��[�_�ł̃N���W�b�g����OK
// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
			if ( CLOUD_CALC_MODE ) {	// �N���E�h�����v�Z���[�h
				// �N���W�b�g�J�[�h�ݒ�
				cal_cloud_credit_set();
			} else {					// �ʏ헿���v�Z���[�h
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

			vl_now = V_CRE;	//�����v�Z�ŎQ��.
			// vl_now : ���Q�Ƃ��ď��� > Ryo_Cal
			ryo_cal( 0, OPECTL.Pr_LokNo );

// GG124100(S) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
			}
// GG124100(E) R.Endo 2022/07/27 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

			if( prm_get( COM_PRM, S_CRE, 3, 1, 1 ) == 1 ){	//���p���׏�/�̎��� �������s����
				if( OPECTL.RECI_SW == 0 ){					// �̎������ݖ��g�p?
					OPECTL.RECI_SW = 1;						// �̎������ݎg�p
// MH810100(S) S.Fujii 2020/07/01 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
					PKTcmd_notice_ope( LCDBM_OPCD_RCT_AUTO_ISSUE, (ushort)0 );	// ����ʒm���M
// MH810100(E) S.Fujii 2020/07/01 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
				}
			}

			ac_flg.cycl_fg = 10;						// ����
			in_mony( msg, 0 );
// MH810103 GG119202(S) ���ϐ��Z���~�󎚏����C��
//			ac_flg.ec_alarm = 0;						// �݂Ȃ����ϕ��d�p�t���OOFF
			ac_flg.ec_deemed_fg = 0;					// �݂Ȃ����ϕ��d�p�t���OOFF
// MH810103 GG119202(E) ���ϐ��Z���~�󎚏����C��

			OPECTL.InquiryFlg = 0;
// MH810100(S) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//			FurikaeMotoSts = 0;							// �U�֌���ԃt���O 0:�`�F�b�N�s�v
// MH810100(E) K.Onodera 2020/03/10 �Ԕԃ`�P�b�g���X(�s�v�����폜)
			break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		case EC_EVT_QR_PAID:
// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
			if ( CLOUD_CALC_MODE ) {	// �N���E�h�����v�Z���[�h
				// QR�R�[�h���ϐݒ�
				cal_cloud_eqr_set();
			} else {					// �ʏ헿���v�Z���[�h
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

			vl_now = V_EQR;								// ���݂u�k�J�[�h�f�[�^��QR�R�[�h���w��iec09�Ŏg�p�j
			ryo_cal( 2, OPECTL.Pr_LokNo );				// �����v�Z

// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
			}
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

			if( OPECTL.op_faz == 0 ){					// ���ڰ���̪���(0:�������M)
				OPECTL.op_faz = 1;						// 1:������(�����L�莞�ɾ��)
			}
			ac_flg.cycl_fg = 10;						// ���Z�����׸�:����
			ret = in_mony( msg, paymod );				// ��������

			ac_flg.ec_deemed_fg = 0;					// �݂Ȃ����ϕ��d�p�t���OOFF
			OPECTL.InquiryFlg = 0;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100
//			FurikaeMotoSts = 0;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100
			break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	    default:
			return( -1 );
	}
	return( ret );
}

// MH810105(S) MH364301 WAON�̖����c���Ɖ�^�C���A�E�g���̐��Z���������C��
//// MH321800(S) G.So IC�N���W�b�g�Ή�
////[]----------------------------------------------------------------------[]
/////	@brief			�d�q�}�l�[���E�������Z����
////[]----------------------------------------------------------------------[]
/////	@param[in]		void
/////	@return			ret			���Z����Ӱ��<br>
/////								 0 = �����p��<br>
/////                              1 = ���Z����(�ނ薳��)<br>
/////                              2 = ���Z����(�ނ�L��)<br>
/////                             10 = ���Z���~(�ҋ@�֖߂�)<br>
/////	@author			G.So
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2019/01/29<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//short	al_emony_sousai(void)
//{
//	short	ret;
//
//	vl_sousai((ushort)prm_get(COM_PRM ,S_ECR, 11, 4, 1));	// ���E��������
//// MH810103(s) �d�q�}�l�[�Ή� # 5396 50-0011=110100�ݒ莞�AWAON�Ŗ����c���Ɖ�ɓ���J�[�h�^�b�`�Ő��Z�������Ă��A���A���^�C���̐��Z�f�[�^�ɓX�����̏�񂪃Z�b�g����Ȃ�
//	// m_stDisc���Z�b�g����
//	RTPay_pre_sousai();
//// MH810103(e) �d�q�}�l�[�Ή� # 5396 50-0011=110100�ݒ莞�AWAON�Ŗ����c���Ɖ�ɓ���J�[�h�^�b�`�Ő��Z�������Ă��A���A���^�C���̐��Z�f�[�^�ɓX�����̏�񂪃Z�b�g����Ȃ�
//
//	ryo_cal( 9, 0 );										// �����v�Z(���E����)
//	if (OPECTL.op_faz == 0) {								// ���ڰ���̪���(0:�������M)
//		OPECTL.op_faz = 1;									// 1:������(�����L�莞�ɾ��)
//	}
//	ac_flg.cycl_fg = 10;									// ���Z�����׸�:����
//	// �T�[�r�X�����|�����ɒu��������̂ŃJ�[�h�C�x���g�ŏ�������
//	ret = in_mony(ARC_CR_R_EVT, 0);							// ��������
//// MH810103(s) �d�q�}�l�[�Ή� # 5396 50-0011=110100�ݒ莞�AWAON�Ŗ����c���Ɖ�ɓ���J�[�h�^�b�`�Ő��Z�������Ă��A���A���^�C���̐��Z�f�[�^�ɓX�����̏�񂪃Z�b�g����Ȃ�
//	RTPay_set_sousai();
//// MH810103(e) �d�q�}�l�[�Ή� # 5396 50-0011=110100�ݒ莞�AWAON�Ŗ����c���Ɖ�ɓ���J�[�h�^�b�`�Ő��Z�������Ă��A���A���^�C���̐��Z�f�[�^�ɓX�����̏�񂪃Z�b�g����Ȃ�
//
//	return ret;
//}
//// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810105(E) MH364301 WAON�̖����c���Ɖ�^�C���A�E�g���̐��Z���������C��
