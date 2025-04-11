// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
//[]----------------------------------------------------------------------[]
///	@file	cal_cloud.h
///	@brief	�N���E�h�����v�Z�����w�b�_�[
///	@date	2022/06/02
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]

#ifndef	_CAL_CLOUD_H_
#define	_CAL_CLOUD_H_

#include "mem_def.h"
#include "mnt_def.h"
#include "ope_def.h"

// �����v�Z���[�h����
// GG124100(S) R.Endo 2022/09/29 �Ԕԃ`�P�b�g���X3.0 #6615 �ݒ�01-0079(�����v�Z�ؑ�)�̐ݒ�̈Ӗ���ύX���� / �f�t�H���g�l��ύX����
// #define CLOUD_CALC_MODE (g_calcMode == 1)	// �N���E�h�����v�Z���[�h
#define CLOUD_CALC_MODE (g_calcMode == 0)	// �N���E�h�����v�Z���[�h
// GG124100(E) R.Endo 2022/09/29 �Ԕԃ`�P�b�g���X3.0 #6615 �ݒ�01-0079(�����v�Z�ؑ�)�̐ݒ�̈Ӗ���ύX���� / �f�t�H���g�l��ύX����

// �N���E�h�����v�Z�ł͐ݒ�֎~�Ƃ���A�h���X
extern const PRM_MASK param_mask_cc[];	// �}�X�N�f�[�^
extern const short	prm_mask_max_cc;	// �}�X�N�f�[�^����

// �N���E�h�����v�Z�p���[�U�[���j���[
enum {
	__MNT_ERARM_CC = 0,	// .�G���[�E�A���[���m�F
	__MNT_MNCNT_CC,		// .�ޑK�Ǘ�
	__MNT_TTOTL_CC,		// .T�W�v
	__MNT_GTOTL_CC,		// .GT�W�v
	__MNT_STSVIEW_CC,	// .��Ԋm�F
	__MNT_REPRT_CC,		// .�̎����Ĕ��s
	__MNT_QRCHECK_CC,	// .QR�f�[�^�m�F
	__MNT_CLOCK_CC,		// .���v����
// 	__MNT_SERVS_CC,		// .�T�[�r�X�^�C��
// 	__MNT_SPCAL_CC,		// .���ʓ�/���ʊ���
	__MNT_OPCLS_CC,		// .�c�x�Ɛؑ�
	__MNT_PWMOD_CC,		// .�W���p�X���[�h
	__MNT_KEYVL_CC,		// .�L�[���ʒ���
	__MNT_CNTRS_CC,		// .�P�x����
	__MNT_VLSW_CC,		// .�����ē�����
	__MNT_OPNHR_CC,		// .�c�ƊJ�n����
// 	__MNT_MNYSET_CC,	// .�����ݒ�
	__MNT_EXTEND_CC,	// .�g���@�\
	USER_MENU_MAX_CC
};
extern const unsigned char UMMENU_CC[][27];
extern const unsigned short USER_TBL_CC[][4];

// �N���E�h�����v�Z�p�V�X�e�������e�i���X���j���[
#define SYS_MENU_MAX_CC (7)
extern const unsigned char SMMENU_CC[][31];
extern const unsigned short SYS_TBL_CC[][4];

// �N���E�h�����v�Z�p���O�t�@�C���v�����g���j���[
#define LOG_MENU_MAX_CC (14)
extern const unsigned char LOGMENU_CC[][31];
extern const unsigned short LOG_FILE_TBL_CC[][4];

// �N���E�h�����v�Z ���Z�J�n�����`�F�b�N
extern short cal_cloud_pay_start_time_check(ushort ndatTarget);

// �N���E�h�����v�Z ����������
extern void cal_cloud_init();

// �N���E�h�����v�Z ���ɏ��`�F�b�N(����)
extern short cal_cloud_fee_check();

// �N���E�h�����v�Z �����ݒ�
// GG124100(S) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�
// extern short cal_cloud_fee_set();
extern short cal_cloud_fee_set(uchar firstFlg);
// GG124100(E) R.Endo 2022/09/16 �Ԕԃ`�P�b�g���X3.0 #6605 ����������A���������g�p����Ɠ������z���Ȃ��������ƂɂȂ�

// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
// �N���E�h�����v�Z �����ݒ� ���u���Z(���Ɏ����w��)
extern short cal_cloud_fee_set_remote_time();

// �N���E�h�����v�Z �����ݒ� ���u���Z(���z�w��)
extern short cal_cloud_fee_set_remote_fee();
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

// �N���E�h�����v�Z ����`�F�b�N
extern eSEASON_CHK_RESULT cal_cloud_season_check();

// �N���E�h�����v�Z ����ݒ�
extern short cal_cloud_season_set();

// �N���E�h�����v�Z QR�f�[�^�`�F�b�N
extern OPE_RESULT cal_cloud_qrdata_check(tMediaDetail *pMedia, QR_YMDData* pYmdData);

// �N���E�h�����v�Z ���ɏ��`�F�b�N(����)
extern short cal_cloud_discount_check();

// �N���E�h�����v�Z �����ݒ�
extern short cal_cloud_discount_set();

// GG124100(S) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)
// �N���E�h�����v�Z �����ݒ� ���u���Z(���Ɏ����w��)
extern short cal_cloud_discount_set_remote_time();

// �N���E�h�����v�Z �����ݒ� ���u���Z(���z�w��)
extern short cal_cloud_discount_set_remote_fee();
// GG124100(E) R.Endo 2022/09/06 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(���u���Z)

// �N���E�h�����v�Z �d�q�}�l�[�ݒ�
extern short cal_cloud_emoney_set();

// �N���E�h�����v�Z QR�R�[�h���ϐݒ�
extern short cal_cloud_eqr_set();

// �N���E�h�����v�Z �N���W�b�g�J�[�h�ݒ�
extern short cal_cloud_credit_set();

// GG129000(S) ���P�A��No.53 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j
// �N���E�h�����v�Z ���ɏ��`�F�b�N(�m�F�̂�)
extern short cal_cloud_discount_check_only();
// GG129000(E) ���P�A��No.53 �Đ��Z���̃��[�����j�^�[�f�[�^�̐��Z���i�����ς݁i�����܂ށj�j�ɂ��āiGM803002���p�j

#endif	// _CAL_CLOUD_H_
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
