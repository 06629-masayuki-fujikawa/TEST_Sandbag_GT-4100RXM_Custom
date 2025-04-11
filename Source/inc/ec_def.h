//[]----------------------------------------------------------------------[]
///	@brief			���σ��[�_R/W
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	: 19/02/07<br>
///					Update
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
#ifndef _EC_DEF_H_
#define _EC_DEF_H_

#include	"system.h"
// MH810103 GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
#include	"common.h"
// MH810103 GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
// MH810103 GG119202(S) ������������L�^�ɍĐ��Z�����󎚂���
#include	"mem_def.h"
// MH810103 GG119202(E) ������������L�^�ɍĐ��Z�����󎚂���

typedef	struct {
	uchar	type;		// R/W type
	uchar	mdl;		// module code
	void	(*init)(void);
	void	(*event)(void);
	short	(*recv)(uchar*, short);
	void	(*log_regist)(uchar*, ushort, uchar);
} t_JVMA_SETUP;

// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//// GG119202(S) ���Z�@�̖����m�艞���҂��^�C�}��ύX����
////#define	OPE_EC_MIRYO_TIME		65									// �����Ď��^�C���A�E�g:65s
//#define	OPE_EC_MIRYO_TIME		80									// �����Ď��^�C���A�E�g:80s
//// GG119202(E) ���Z�@�̖����m�艞���҂��^�C�}��ύX����
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�

#define	OPE_EC_RECEPT_SEND_TIME	10									// ���̎�t�����M:200ms(200ms/20ms=10)
// MH810103 GG119202(S) E3210�o�^�����C��
//// MH321800(S) E3210�o�^�^�C�~���O�ύX
//#define	OPE_EC_WAIT_BOOT_TIME	180									// �������V�[�P���X�����҂�����
//// MH321800(E) E3210�o�^�^�C�~���O�ύX
#define	OPE_EC_WAIT_BOOT_TIME	(7*60)								// �������V�[�P���X�����҂�����
// MH810103 GG119202(E) E3210�o�^�����C��
// MH810105 GG119202(S) ���σ��[�_�o�[�W�����A�b�v���̕��d�Ή�
#define	OPE_EC_WAIT_BOOT_TIME_VERUP	(40*60)							// �������V�[�P���X�����҂����ԁi���σ��[�_�o�[�W�����A�b�v���j
// MH810105 GG119202(E) ���σ��[�_�o�[�W�����A�b�v���̕��d�Ή�

// ���σ��[�_�Œ�l(��SX-10�ł͐ݒ�Q��)
#define	EC_STATUS_TIME			0									// ��ԊĎ��^�C�}�[(�Œ�F��)
#define	EC_INIT_RETRY_CNT		0									// �ď��������g���C��(�Œ�)
#define	EC_ENABLE_WAIT_TIME		10									// ��t��M�҂��^�C�}�[(�Œ�F�b)
#define	EC_ENABLE_RETRY_CNT		0									// �^�C���A�E�g�����g���C��
#define	EC_DISABLE_WAIT_TIME	5									// ��t�s��M�҂��^�C�}�[(�Œ�F�b)
#define	EC_DISABLE_RETRY_CNT	1									// �^�C���A�E�g�����g���C��
#define	EC_MIN_VOLUME			1									// �ŏ�����
#define	EC_MAX_VOLUME			15									// �ő剹��
#define	EC_CMDWAIT_TIME			1000								// �����e�i���X���ʃf�[�^��M�҂���ϰ�F20s(20000ms/20ms=1000)
// GG116202(S) JVMA���Z�b�g�񐔐����Ή�
#define	EC_JVMA_RESET_CNT		3									// JVMA���Z�b�g���g���C��
// GG116202(E) JVMA���Z�b�g�񐔐����Ή�
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
#define	EC_BRAND_NEGO_WAIT_TIME	250									// �u�����h�l�S�V�G�[�V�������̉����҂��^�C�}�[:5s(5000ms/20ms=250)
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
// MH810103 GG118807_GG118907(S) ��ʌnIC�̌��x�z��10���~�Ƃ���
#define	EC_PAY_LIMIT_KOUTSUU	100000								// ��ʌnIC�̐��Z���x�z�i10���~�j
// MH810103 GG118807_GG118907(E) ��ʌnIC�̌��x�z��10���~�Ƃ���

// �t�F�[�Y
enum {
	EC_PHASE_INIT = 0,				// ��������
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
//	EC_PHASE_PAY,					// ���Z��(�m�[�}��)
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
	EC_PHASE_BRAND,					// �u�����h���킹��
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
	EC_PHASE_VOL_CHG,				// ���ʕύX��
	EC_PHASE_PAY,					// ���Z��(�m�[�}��)
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
// MH810103 GG119202(S) JVMA���Z�b�g�����s�
	EC_PHASE_WAIT_RESET,			// JVMA���Z�b�g�҂�
// MH810103 GG119202(E) JVMA���Z�b�g�����s�
};

// ���σ��[�_����
struct	ECCTL_rec {
	uchar				phase;					// ���σ��[�_�t�F�[�Y
	uchar				step;					// �t�F�[�Y���X�e�b�v
	uchar				pon_err_alm_regist;		// 1=��t�A���[���A�u�����h�G���[�o�^����
	uchar				not_ready_timer;		// 1=���[�_�[��t�����䎸�s�^�C�}�[�Ď���
	uchar				brand_num;				// ���[�_���u�����h��
// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
//	uchar				dmy[3];
	uchar				prohibit_snd_bsy;		// ����f�[�^�i��t�֎~�j�����Ď����
												// 0x00:�����Ď��Ȃ�
												// 0x01:��t�֎~�����Ď���
												//(0x02:�ċN���͉����Ď��ΏۊO)
												// 0x08:���z�ύX�����Ď���
												// 0x80:����I�������Ď���
	uchar				transact_end_snd;		// ����I�����M�v�����
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	uchar				dmy;
	uchar				RecvVolumeRes;			// ��M�������ʕύX���ʃf�[�^
	uchar				ec_Disp_No;				// ���ό��ʏڍ׃G���[�R�[�h�̈ē��\���p�^�[��
												//  0	:�\���Ȃ�
												//  1�`	:�ڍ׃G���[�\��
												// 99	:�G���[�o�^���Ȃ��ڍ׃G���[(�\���Ȃ�)
	uchar				Ec_Settlement_Sts;		// ���Ϗ��
	uchar				timer_recept_send_busy;	// 1=���Z����ec������M�^�C�}�[���쒆
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//	uchar				dmy;
	uchar				brandsel_stop;			// �u�����h������~
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	ushort				Product_Select_Brand;	// ���i�I���f�[�^���M�̃u�����h�ԍ�
	ushort				ec_MessagePtnNum;		// LCD�\���p�^�[��
												//  0:�\���Ȃ�
												//  1:�N���W�b�g������
												//  2:�d�q�}�l�[������
												//  3:�J�[�h�������ē�
												//  4:���ό���NG��M���̃J�[�h�������ē�
												// 98:���ό���NG��M���̏ڍ׃G���[�ɂ��G���[�\��
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
// MH810103 GG119202(S) �d�q�}�l�[�V���O���ݒ�ňē��������s��
	uchar				anm_ec_emoney;			// �ē������t���O�i0=������,1=��t�҂�,0xFF=�����ς݁j
// MH810103 GG119202(E) �d�q�}�l�[�V���O���ݒ�ňē��������s��
// MH810103 GG118808_GG118908(S) �d�q�}�l�[�V���O���ݒ�̓ǂݎ��҂��^�C���A�E�g�����ύX
	uchar				ec_single_timeout;		// �d�q�}�l�[�V���O���̃^�C���A�E�g�t���O�i1=�^�C���A�E�g����j
// MH810103 GG118808_GG118908(E) �d�q�}�l�[�V���O���ݒ�̓ǂݎ��҂��^�C���A�E�g�����ύX
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	uchar				Ec_FailureContact_Sts;	// ��Q�A���[���s����
												//  0:��Q�A���[���s�s��
												//  1:��Q�A���[���s�҂�
												//  2:��Q�A���[���s��
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
};
extern	struct ECCTL_rec	ECCTL;

// �R�}���h
#define	S_PRINT_DATA		0x1C	// �󎚈˗��f�[�^
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
//#define	S_RECEIVE_DEEMED	0x29	// ���ϐ��Z���~(������)�ް�
#define	S_RECEIVE_DEEMED	0x49	// ���O����ް�
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
#define	S_SETT_STS_DATA		0x28	// ���Ϗ�ԃf�[�^

#define	S_BRAND_SET_DATA	0x14	// �u�����h�ݒ�f�[�^1
#define	S_BRAND_STS_DATA	0x1D	// �u�����h��ԃf�[�^1
#define	S_BRAND_SET_DATA2	0x24	// �u�����h�ݒ�f�[�^2
#define	S_BRAND_STS_DATA2	0x2D	// �u�����h��ԃf�[�^2

#define	S_VOLUME_CHG_DATA	0x31	// ���ʕύX�f�[�^
#define	S_READER_MNT_DATA	0x32	// ���[�_�����e�i���X�f�[�^
#define	S_BRAND_SEL_DATA	0x33	// �u�����h�I���f�[�^

#define	S_VOLUME_RES_DATA	0x34	// ���ʕύX���ʃf�[�^
#define	S_MODE_CHG_DATA		0x35	// ���[�_�����e�i���X���ʃf�[�^
#define	S_BRAND_RES_DATA	0x36	// �u�����h�I�����ʃf�[�^
// MH810103 GG119202(S) �u�����h�I�����ʉ����f�[�^
#define	S_BRAND_RES_RSLT_DATA	0x39	// �u�����h�I�����ʉ����f�[�^
// MH810103 GG119202(E) �u�����h�I�����ʉ����f�[�^
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
#define	S_BRAND_RES_RSLT_DATA_with_HOLD	0x139	// �u�����h�I�����ʉ����f�[�^
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
#define S_SUB_BRAND_RES_DATA	0x4D	// �T�u�u�����h�f�[�^
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

// ����f�[�^��bit
#define	S_CNTL_REBOOT_OK_BIT			0x02	// �ċN����
#define	S_CNTL_MIRYO_ZANDAKA_END_BIT	0x04	// �����c���I���ʒm
#define	S_CNTL_PRICE_CHANGE_BIT			0x08	// ���z�ύX
// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
#define	S_CNTL_TRANSACT_END_BIT			0x80	// ����I��
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�

#define	ECARDID_SIZE_SUICA			16			// Suica�J�[�hID�T�C�Y
#define	ECARDID_SIZE_SAPICA			17			// SAPICA�J�[�hID�T�C�Y
#define	ECARDID_SIZE_KOUTSUU		17			// ��ʌnIC�J�[�hID�T�C�Y
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
#define	ECARDID_SIZE_CREDIT			16			// �N���W�b�g�J�[�hID�T�C�Y
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX

#define EC_CMD_LEN_CTRL_DATA		3			// ����f�[�^�R�}���h��
#define EC_CMD_LEN_SLCT_DATA		74			// �I�����i�f�[�^�R�}���h��
#define EC_CMD_LEN_INDI_DATA		88			// �ʃf�[�^�R�}���h��
#define EC_CMD_LEN_BSET_DATA		233			// �u�����h�ݒ�f�[�^�R�}���h��
#define EC_CMD_LEN_VSET_DATA		21			// ���ʕύX�f�[�^�R�}���h��
#define EC_CMD_LEN_MENT_DATA		5			// ���[�_�����e�i���X�f�[�^�R�}���h��
// MH810103 GG119202(S) �u�����h�I���f�[�^�t�H�[�}�b�g�ύX
//#define EC_CMD_LEN_BSEL_DATA		43			// �u�����h�I���f�[�^�R�}���h��
#define EC_CMD_LEN_BSEL_DATA		45			// �u�����h�I���f�[�^�R�}���h��
// MH810103 GG119202(E) �u�����h�I���f�[�^�t�H�[�}�b�g�ύX
// MH810103 GG119202(S) �u�����h�I�����ʉ����f�[�^
#define EC_CMD_LEN_BRRT_DATA		5			// �u�����h�I�����ʉ����f�[�^�R�}���h��
// MH810103 GG119202(E) �u�����h�I�����ʉ����f�[�^

#define EC_BRAND_NAME_LEN			20			// �u�����h���̒�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
#define	EC_SUB_BRAND_MAX			124			// �T�u�u�����h�ő吔
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

// ����f�[�^�̎�����
enum {
	EPAY_RESULT_NONE = 0x00,		// ���̑�
	EPAY_RESULT_OK,					// OK
	EPAY_RESULT_NG,					// NG
	EPAY_RESULT_CANCEL,				// ���σL�����Z��
	EPAY_RESULT_CANCEL_PAY_OFF,		// ���Z�L�����Z��
	EPAY_RESULT_MIRYO,				// �����m��
// MH810103 GG119202(S) �����d�l�ύX�Ή�
//	EPAY_RESULT_MIRYO_AFTER_OK,		// �����m��㌈��OK
// MH810103 GG119202(E) �����d�l�ύX�Ή�
	EPAY_RESULT_MIRYO_ZANDAKA_END,	// �����c���Ɖ��
// MH810105 GG119202(S) ���Ϗ������ɏ�Q�������������̓���
	EPAY_RESULT_PAY_MIRYO = 0x11,	// �x�������i����OK�j
// MH810105 GG119202(E) ���Ϗ������ɏ�Q�������������̓���
};

// ���Ϗ�ԃf�[�^�̌��Ϗ��
enum {
	EC_SETT_STS_NONE = 0x00,		// ���L�ȊO
	EC_SETT_STS_MIRYO = 0x02,		// ������
	EC_SETT_STS_CARD_PROC,			// �J�[�h������
	EC_SETT_STS_INQUIRY_OK,			// �Ɖ�OK
// MH810103 GG119202(S) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
	EC_SETT_STS_DEEMED_SETTLEMENT,	// �݂Ȃ�����
// MH810103 GG119202(E) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
};

// ����f�[�^(�^�A���[�����V�[�g)
// �u�����h�ԍ��F����f�[�^/�u�����h��ԃf�[�^��M���̃u�����h�ԍ�
#define		BRANDNO_KOUTSUU		1000
// MH810105(S) MH364301 PiTaPa�Ή�
//#define		BRANDNO_SAPICA		1025		// ���ԍ�
#define		BRANDNO_SAPICA		1030		// ���ԍ�
// MH810105(E) MH364301 PiTaPa�Ή�
#define		BRANDNO_EDY			1020
#define		BRANDNO_WAON		1021
#define		BRANDNO_NANACO		1022
#define		BRANDNO_ID			1023
#define		BRANDNO_QUIC_PAY	1024
// MH810105(S) MH364301 PiTaPa�Ή�
#define		BRANDNO_PITAPA		1025
// MH810105(E) MH364301 PiTaPa�Ή�
#define		BRANDNO_CREDIT		2000
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
#define		BRANDNO_QR			3000
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
#define		BRANDNO_TCARD		7000
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
#define		BRANDNO_HOUJIN		8000
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
#define		BRANDNO_HOUSE_S		7001
#define		BRANDNO_HOUSE_E		7999
#define		BRANDNO_for_SETTLEMENT		6999
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
#define		BRANDNO_UNKNOWN		0xffff
#define		BRANDNO_ZERO		0				// �u�����h�ԍ�0

// MH810103 GG119202(S) ������������L�^�ɍĐ��Z�����󎚂���
//#define		EC_BRAND_MAX		10				// �u�����h���ő�l
//#define		EC_BRAND_TOTAL_MAX	(EC_BRAND_MAX*2)	// ���u�����h���ő�l
// MH810103 GG119202(E) ������������L�^�ɍĐ��Z�����󎚂���
#define		EC_VOL_MAX			6				// ���ʕύX�p�^�[�����ő�l

// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//#define		WAON_POINT_JMB_MILE		50001		// �J�[�h���ށFJMB�}�C��
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�

// �u�����h�I�����ʃf�[�^
#define		EC_BRAND_SELECTED		1			// �u�����h�I���ς�
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//#define		EC_BRAND_UNSELECTED		0			// �u�����h���I��
#define		EC_BRAND_CANCELED		0			// �u�����h�I���L�����Z��
#define		EC_BRAND_UNSELECTED		2			// �u�����h���I��

// �u�����h�I�����ʃf�[�^ ���R�R�[�h
#define		EC_BRAND_REASON_0		0			// �L�����Z���{�^���������ꂽ�B
#define		EC_BRAND_REASON_1		1			// �^�C���A�E�g�����B
#define		EC_BRAND_REASON_99		99			// �u�����h�I���f�[�^�̏������ł��Ȃ��B
#define		EC_BRAND_REASON_201		201			// �J�[�h�̖₢���킹���ł��Ȃ��B
#define		EC_BRAND_REASON_202		202			// �J�[�h�̖₢���킹���ʂ���M�ł��Ȃ��B
#define		EC_BRAND_REASON_203		203			// ���f�[�^���܂܂��J�[�h��ǂݎ�肵���B
#define		EC_BRAND_REASON_204		204			// �f�[�^���j�����Ă���J�[�h��ǂݎ�肵���B
#define		EC_BRAND_REASON_205		205			// �Ӑ}���Ȃ��J�[�h��ǂݎ�肵���B
#define		EC_BRAND_REASON_206		206			// �L�������؂�̃J�[�h��ǂݎ�肵���B
#define		EC_BRAND_REASON_207		207			// �l�K�J�[�h��ǂݎ�肵���B
#define		EC_BRAND_REASON_208		208			// �L���ȃJ�[�h�ł��邪�g�p�ł��Ȃ��B
#define		EC_BRAND_REASON_209		209			// �L���ȃJ�[�h�ł��邪�x�������x�z�𒴂��Ă���B
#define		EC_BRAND_REASON_210		210			// �L���ȃJ�[�h�ł��邪�c�����s�����Ă���B
#define		EC_BRAND_REASON_211		211			// ��t�ς݂̃J�[�h��ǂݎ�肵���B
#define		EC_BRAND_REASON_212		212			// �ǂݎ����x���𒴂��ăJ�[�h��ǂݎ�肵���B
#define		EC_BRAND_REASON_299		299			// ���̑��̃G���[
#define		EC_BRAND_REASON_INVALID 0xffff		// �����l(�����R�[�h�ł��B���σ��[�_�ɑ��M�s�ł�)
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j

// MH810103 GG119202(S) �J�ǁE�L�������ύX
//// ���Z�۔����� - isEcReady()�̈���
//enum {
//	EC_CHECK_STATUS = 1,
//	EC_CHECK_EMONEY,
//	EC_CHECK_CREDIT,
//};
// MH810103 GG119202(E) �J�ǁE�L�������ύX

typedef union{
	ushort		status_data;					// 2Byte
	struct {
		// ��ԃf�[�^1
		uchar	RebootReq:1;					// Bit 7 = �ċN���v���i0�F�ċN���v���Ȃ� 1�F�ċN���v���j
		uchar	DetectionCard:1;				// Bit 6 = �J�[�h�������ݏ�ԁi0�F�J�[�h�Ȃ� 1�F�J�[�h����j
		uchar	ReqTimerUpdate:1;				// Bit 5 = ���i��I���^�C�}�[�X�V�˗��i0�F�X�V�˗� 1�F�Ȃ��j
		uchar	MiryoStatus:1;					// Bit 4 = ����������ԁi0�F�������Ŗ��� 1�F�����������j
		uchar	MiryoZandakaStop:1;				// Bit 3 = �����c�����~�i0�F�Ȃ� 1�F�����c���Ɖ�~�j
		uchar	Running:1;						// Bit 2 = ���s���i0�F���s���Ŗ��� 1�F���s���j
		uchar	TradeCansel:1;					// Bit 1 = �����ݾفi0�F�����ݾقŖ��� 1�F�����ݾَ�t�j
		uchar	ReceptStatus:1;					// Bit 0 = ��t��ԁi0�F��t�� 1�F��t�s�j
		// ��ԃf�[�^2
// MH810103 GG119202(S) �����d�l�ύX�Ή�
//		uchar	YOBI:8;							// Bit 0-7 = �\��
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//		uchar	YOBI:6;							// Bit 2-7 = �\��
// MH810104 GG119201(S) ���[�_���撆�͐���f�[�^�i��t���j�𑗐M���Ȃ�
//		uchar	YOBI:5;							// Bit 3-7 = �\��
		uchar	YOBI:4;							// Bit 4-7 = �\��
		uchar	LastSetteRunning:1;				// Bit 3 = ���O����v�����i0�F�v�����Ŗ��� 1�F�v�����j
// MH810104 GG119201(E) ���[�_���撆�͐���f�[�^�i��t���j�𑗐M���Ȃ�
		uchar	CardProcTimerUpdate:1;			// Bit 2 = �J�[�h�ǎ�҂��^�C�}�X�V�˗��i0�F�Ȃ� �P�F�X�V�˗��j
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
		uchar	MiryoConfirm:1;					// Bit 1 = �����m��i0�F�Ȃ� 1�F�����m��j
		uchar	ZandakaInquiry:1;				// Bit 0 = �c���Ɖ�i0�F�Ȃ� 1�F�c���Ɖ�j
// MH810103 GG119202(E) �����d�l�ύX�Ή�
	} StatusInfo;
} t_EC_STATUS_DATA_INFO;
extern t_EC_STATUS_DATA_INFO	EC_STATUS_DATA;				// ��ԃf�[�^
extern t_EC_STATUS_DATA_INFO	PRE_EC_STATUS_DATA;			// �O���M������ԃf�[�^
extern t_EC_STATUS_DATA_INFO	EC_STATUS_DATA_WAIT;		// ��ԃf�[�^(��M�҂��p)

// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
// Ec_info��pack����Ă��邽��
#pragma pack
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��

// MH810103 GG119202(S) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
typedef struct {
// ��ʌnIC
	uchar				SPRW_ID[13];			// SPRWID
	uchar				dmy1;
	uchar				Kamei[30];				// �����X��
	uchar				TradeKind;				// ������
	uchar				dmy2;
} EC_TRADE_RES_KOUTSUU;
// MH810103 GG119202(E) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��

typedef	struct {
// Edy
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//	long				DealNo;					// ����ʔ�(hex:4byte)
//	long				CardDealNo;				// �J�[�h����ʔ�(hex:4byte)
//	long				TerminalNo[2];			// �[���ԍ�(hex:8byte)
	uchar				DealNo[10];				// Edy����ʔ�
	uchar				CardDealNo[5];			// �J�[�h����ʔ�
	uchar				dmy;
	uchar				TerminalNo[8];			// ��ʒ[��ID
	uchar				Kamei[30];				// �����X��
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
} EC_TRADE_RES_EDY;

typedef	struct {
// nanaco
// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//	long				TerminalNo[3];			// ��ʒ[��ID(bcd:10byte)
//	long				DealNo;					// ��ʎ���ʔ�(hex:4byte)
	uchar				Kamei[30];				// �����X��
	uchar				DealNo[6];				// �[������ʔ�
	uchar				TerminalNo[20];			// ��ʒ[��ID
// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
} EC_TRADE_RES_NANACO;

typedef	struct {
// WAON
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//	long				GetPoint;				// ����|�C���g(hex:4byte)
//	long				TotalPoint;				// �݌v�|�C���g(hex:4byte)
//	long				CardType;				// �J�[�h����(bcd:4byte)
//	long				PeriodPoint;			// �Q�N�O�܂łɊl�������|�C���g(hex:4byte)
//	long				Period;					// �Q�N�O�܂łɊl�������|�C���g�̗L������(bcd:4byte)
//	uchar				SPRW_ID[14];			// SPRWID(���ۂ�13��)
//	uchar				PointStatus;			// ���p��(hex:1byte)
//	uchar				DealCode;				// �����ʃR�[�h
	ulong				GetPoint;				// ����|�C���g(hex:4byte)
	ulong				TotalPoint[2];			// �݌v�|�C���g(hex:8byte)
	uchar				SPRW_ID[13];			// SPRWID
	uchar				PointStatus;			// �݌v�|�C���g���b�Z�[�W
												// 0=�u�݌vWAON�|�C���g���́A���߂���WAON�X�e�[�V�����Ŋm�F���������B�v
												// 1=�u����WAON�J�[�h�̓|�C���g�ΏۊO�ł��B�v
												// 2=�uJMB WAON�̓}�C�������܂��B�v
	uchar				Kamei[30];				// �����X��
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
} EC_TRADE_RES_WAON;

typedef	struct {
// SAPICA
	uchar				Terminal_ID[8];			// ���̒[��ID(���ۂ� 7��)
	long				Details_ID;				// �ꌏ����ID(hex:4byte)
} EC_TRADE_RES_SAPICA;

typedef	struct {
// iD
// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//	uchar				Terminal_ID[8];			// ���̒[��ID(���ۂ� 7��)
//	long				Details_ID;				// �ꌏ����ID(hex:4byte)
	uchar				Kamei[30];				// �����X��
	uchar				Approval_No[7];			// ���F�ԍ�
	uchar				dmy1;
	uchar				TerminalNo[13];			// ��ʒ[��ID
	uchar				dmy2;
// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
} EC_TRADE_RES_ID;

typedef	struct {
// QuicPay
// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//	uchar				Terminal_ID[8];			// ���̒[��ID(���ۂ� 7��)
//	long				Details_ID;				// �ꌏ����ID(hex:4byte)
	uchar				Kamei[30];				// �����X��
	uchar				Approval_No[7];			// ���F�ԍ�
	uchar				dmy1;
	uchar				TerminalNo[13];			// ��ʒ[��ID
	uchar				dmy2;
// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
} EC_TRADE_RES_QUIC_PAY;

// MH810105(S) MH364301 PiTaPa�Ή�
typedef	struct {
// PiTaPa
	uchar				Kamei[30];				// �����X��
	uchar				Approval_No[8];			// ���F�ԍ�
	uchar				TerminalNo[13];			// ��ʒ[��ID
	uchar				dummy1;
	uchar				Slip_No[5];				// �`�[�ԍ�
	uchar				dummy2;
	uchar				Reason_Code[6];			// ���R�R�[�h
} EC_TRADE_RES_PITAPA;
// MH810105(E) MH364301 PiTaPa�Ή�

typedef	struct {
// Credit
	long				Slip_No;				// �[�������ʔԁi�`�[�ԍ��j
	unsigned char		Approval_No[7];			// ���F�ԍ�
	unsigned char		KID_Code[6];			// KID�R�[�h
	unsigned char		Id_No[13];				// �[�����ʔԍ�
	unsigned char		Credit_Company[24];		// �N���W�b�g�J�[�h��Ж�
	unsigned char		Identity_Ptrn;			// �{�l�m�F�p�^�[��
} EC_TRACE_RES_CREDIT;

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
typedef struct {
// QR
	uchar	PayKind;							// �x�����
	uchar	dmy1;
	uchar	PayTerminalNo[20];					// �x���[��ID
	uchar	MchTradeNo[32];						// Mch����ԍ�
} EC_TRADE_RES_QR;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
#pragma unpack
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��

typedef	struct {
	unsigned char		Result;										// ���σf�[�^
	unsigned char		Column_No;									// ���σR�����ԍ�
	ushort				brand_no;									// �u�����h�ԍ�
	long				settlement_data;							// ���ۂ̌��ϊz
	long				settlement_data_before;						// ���Z�OSuica�c��
	long				settlement_data_after;						// ���ό�Suica�c��
	unsigned char		Card_ID[20];								// Card ID
// MH810103 GG119202(S) �d�q�}�l�[�Ή�
//	unsigned char		inquiry_num[16];							// �⍇���ԍ�
	unsigned char		inquiry_num[15];							// �₢���킹�ԍ�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	unsigned char		dummy;										// �₢���킹�ԍ��ƍ��킹��16byte
	t_E_FLAG			E_Flag;										// �׸ފǗ�(uchar)
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810103 GG119202(E) �d�q�}�l�[�Ή�
	struct	clk_rec		settlement_time;							// ���Z����
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//// MH810103 GG119202(S) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
//	unsigned char		Termserial_No[30];							// �[�����ʔԍ�
//// MH810103 GG119202(E) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
	uchar				QR_Kamei[30];								// �����X���iQR�R�[�h���ρj
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	// �ȍ~�A�u�����h���Ɏ�M����f�[�^���e
	union {
	// �u�����h�ʎ���f�[�^
// MH810103 GG119202(S) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
		EC_TRADE_RES_KOUTSUU	Koutsuu;
// MH810103 GG119202(E) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
		EC_TRADE_RES_EDY		Edy;
		EC_TRADE_RES_NANACO		Nanaco;
		EC_TRADE_RES_WAON		Waon;
		EC_TRADE_RES_SAPICA		Sapica;
		EC_TRADE_RES_ID			Id;
		EC_TRADE_RES_QUIC_PAY	QuicPay;
		EC_TRACE_RES_CREDIT		Credit;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		EC_TRADE_RES_QR			Qr;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105(S) MH364301 PiTaPa�Ή�
//	// �o�b�N�A�b�v/���X�g�A�̑ΏۂƂ���̂ŁA�����̃u�����h�ǉ��p�ɃT�C�Y�ɗ]�T���������Ă���
		EC_TRADE_RES_PITAPA		Pitapa;
		// EC_TRADE_RES_PITAPA��64�o�C�g�̂��߁A
		// 64�o�C�g�𒴂���ꍇ��dummy���T�C�Y�ύX���邱��
// MH810105(E) MH364301 PiTaPa�Ή�
		uchar					dummy[64];
	} Brand;
} EC_SETTLEMENT_RES;

extern	EC_SETTLEMENT_RES	Ec_Settlement_Res;	// EC�f�[�^�Ǘ��\����
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//extern	unsigned char		Ec_Settlement_Sts;	// ���Ϗ�ԃf�[�^
//extern 	short				ec_MessagePtnNum;	// LCD�\���p�^�[��
//												//  0:�\���Ȃ�
//												//  1:�N���W�b�g������
//												//  2:�J�[�h������
//												//  3:�J�[�h�������ē�
//												//  4:���ό���NG��M���̃J�[�h�������ē�
//												// 98:���ό���NG��M���̏ڍ׃G���[�ɂ��G���[�\��
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������

#define	EC_CARDDATA_LEN			128				// �J�[�h�f�[�^����

// �u�����h������
typedef	struct {
	ushort	no;			// �u�����h�ԍ�
	uchar	reserved;
	uchar	status;		// �u�����h���
						//   b0:1=�J��,0=��
						//   b1:1=����,0=�L��
						//   b2:1=�ꌏ���ז��t,0=�󂫂���
						//   b3:1=�T�[�r�X��~��,0=�Ȃ�
						//   b4-7:����`
						// �u�����h�ݒ�
						//   ���bit(0xh=�J��/1xh=��)
						//   ����bit(x0h=�L��/x1h=����)
	uchar	name[20];	// �u�����h����
} EC_BRAND_CTRL;

typedef	struct {
	uchar	num;		// �u�����h��
	uchar	dmy;
	EC_BRAND_CTRL	ctrl[EC_BRAND_MAX];	// �u�����h���e�[�u��
} EC_BRAND_TBL;

// ���ʃp�^�[��������
typedef	struct {
	ushort	time;		// �ؑ֎���
	uchar	vol;		// ����
	uchar	dmy;
} EC_VOLUME_CTRL;

typedef	struct {
	uchar	num;		// �w�肳�ꂽ�ؑփp�^�[��
	uchar	dmy;
	EC_VOLUME_CTRL	ctrl[EC_VOL_MAX];	// �u�����h���e�[�u��
} EC_VOLUME_TBL;

typedef	struct {
	uchar	cmd;		// �����e�i���X�R�}���h
	uchar	mode;		// ���[�h�ڍs�w�߃f�[�^
	uchar	vol;		// �e�X�g���ʃf�[�^
	uchar	dmy;
} EC_MNT_TBL;

typedef	struct {
// MH810103 GG119202(S) �u�����h�I���f�[�^�t�H�[�}�b�g�ύX
	uchar	btn_enable;	// �{�^���L���w��
	uchar	btn_type;	// �{�^���^�C�v
// MH810103 GG119202(E) �u�����h�I���f�[�^�t�H�[�}�b�g�ύX
	uchar	num;		// �u�����h��
	uchar	dmy;
	ushort	no[EC_BRAND_TOTAL_MAX];		// �u�����h�ԍ�
} EC_BRAND_SEL_TBL;

typedef	struct {
	uchar	res;		// �u�����h�I������
	uchar	siz;		// �J�[�h�f�[�^�T�C�Y
	ushort	no;			// �I���u�����hNo
	uchar	dat[EC_CARDDATA_LEN];	// �J�[�h�f�[�^(ASCII)
} EC_BRAND_RES_TBL;

// MH810103 GG119202(S) �u�����h�I�����ʉ����f�[�^
typedef	struct {
	uchar	res_rslt;	// �u�����h�I��K�p����
	ushort	no;			// ����OK�F�u�����h�ԍ��^����NG�F���R
} EC_BRAND_RES_RSLT_TBL;
// MH810103 GG119202(E) �u�����h�I�����ʉ����f�[�^

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
typedef union {
	uchar	BYTE;
	struct {
		uchar	YOBI:5;
		uchar	MORE_LESS:2;			// 1:more�A2:less
		uchar	RECV:1;					// 0:��M�A1:����M
	} BIT;
} t_TBL_STS;
typedef	struct {
	t_TBL_STS	tbl_sts;				// �e�[�u�����
	uchar	brand_num;					// �u�����h��
	ushort	brand_no[EC_BRAND_MAX];		// �u�����h�ԍ�
	uchar	sub_brand_num[EC_BRAND_MAX];// �T�u�u�����h��
	uchar	sub_brand_no[EC_SUB_BRAND_MAX];	// �T�u�u�����h���
} EC_SUB_BRAND_TBL;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

// ���σ��[�_ �A���[��������O�p�\����(�t���b�v/���b�N�V�X�e���p)
typedef struct {
	uchar				syu;			// ���
// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//	uchar				yobi[3];
	uchar				yobi[2];
	uchar				reason;			// �����������R
// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
	ulong				WPlace;			// ���Ԉʒu�ް�
// MH810103 MH810103(s) �d�q�}�l�[�Ή� ��������L�^�Ή�
	uchar				CarSearchFlg;								// �Ԕ�/�����������
	uchar				CarSearchData[6];							// �Ԕ�/���������f�[�^
	uchar				CarNumber[4];								// �Ԕ�(���Z)
	uchar				CarDataID[32];								// �Ԕԃf�[�^ID
// MH810103 MH810103(e) �d�q�}�l�[�Ή� ��������L�^�Ή�
	date_time_rec2		TInTime;		// ���ɓ���
	date_time_rec2		alarm_time;		// �A���[�������������
	EC_SETTLEMENT_RES	alarm_data;		// �A���[��������
} t_ALARM_SETTLEMENT_DATA;

// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//#define		EC_ALARM_LOG_CNT	5			// �A���[��������O��
#define		EC_ALARM_LOG_CNT	20		// �A���[��������O��
typedef enum {
	ALARM_REASON_HIKISARI_OK = 1,		// �����m��� ���������
	ALARM_REASON_HIKISARI_NG,			// �����m��� ����������
	ALARM_REASON_TIMEOUT,				// �����m��� �^�C���A�E�g
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	ALARM_REASON_CHECKFAIL,				// �x���m�F���s
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
} ALARM_REASON;
// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����

typedef struct {
	ushort					array_cnt;		// =EC_ALARM_LOG_CNT
	ushort					data_size;		// =sizeof(t_ALARM_SETTLEMENT_DATA)
	ushort					log_cnt;		// ���O��������
	ushort					log_wpt;		// ���O�����݃|�C���^
	t_ALARM_SETTLEMENT_DATA	log[EC_ALARM_LOG_CNT];
// MH810103 GG119202(S) ������������L�^�ɍĐ��Z�����󎚂���
	Receipt_data			paylog[EC_ALARM_LOG_CNT];	// �Đ��Z���
// MH810103 GG119202(E) ������������L�^�ɍĐ��Z�����󎚂���
} Ec_Alarm_Log_rec;
extern	Ec_Alarm_Log_rec	EC_ALARM_LOG_DAT;		/* �A���[��������O				*/

// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
typedef struct {
	EC_SETTLEMENT_RES		Ec_Res;				// ���d�p���ό��ʃf�[�^
} EMoneyAlarmFukuden;
extern	EMoneyAlarmFukuden	EcAlarm;			// ���σ��[�_���d�p�A���[������f�[�^
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX

extern const unsigned char	ec_tbl[][2];
extern	EC_BRAND_TBL		RecvBrandTbl[2];	// ���[�_��M�����u�����h����e�[�u���\����
// MH810103 GG119202(S) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
extern	EC_BRAND_TBL		RecvBrandTblTmp[2];	// �u�����h�e�[�u����tmp
// MH810103 GG119202(E) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
extern	EC_BRAND_TBL		BrandTbl[2];		// �u�����h����e�[�u���\����
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//extern unsigned char		RecvVolumeRes;		// ��M�������ʕύX���ʃf�[�^
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
extern	EC_VOLUME_TBL		VolumeTbl;			// ���ʕύX����e�[�u���\����
extern	EC_MNT_TBL			RecvMntTbl;			// ���[�_��M���������e�i���X�e�[�u���\����
extern	EC_MNT_TBL			MntTbl;				// ���[�_�����e�i���X�e�[�u���\����
extern	EC_BRAND_RES_TBL	RecvBrandResTbl;	// ���[�_��M�����u�����h�I�����ʃe�[�u���\����
extern	EC_BRAND_SEL_TBL	BrandSelTbl;		// �u�����h�I���e�[�u���\����
// MH810103 GG119202(S) �u�����h�I�����ʉ����f�[�^
extern	EC_BRAND_RES_RSLT_TBL	BrandResRsltTbl;	// �u�����h�I�����ʉ����e�[�u���\����
// MH810103 GG119202(E) �u�����h�I�����ʉ����f�[�^
extern	unsigned char		Ec_Settlement_Rec_Code[3];	// ���ό��ʏڍ׃G���[�R�[�h
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//extern	unsigned char		ec_Disp_No;			// ���ό��ʏڍ׃G���[�R�[�h�̈ē��\���p�^�[��
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
extern	EC_SUB_BRAND_TBL	RecvSubBrandTbl;	// �T�u�u�����h�e�[�u���\����
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

extern	char			EcEdyTerminalNo[20];	// Edy��ʒ[��ID

// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//extern	uchar			timer_recept_send_busy;	// 1=���Z����ec������M�^�C�}�[���쒆
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������

extern	t_JVMA_SETUP	jvma_setup;
extern	void			*const jvma_init_tbl[];
// GG116202(S) JVMA���Z�b�g�񐔐����Ή�
extern	uchar			Ec_Jvma_Reset_Count;
// GG116202(E) JVMA���Z�b�g�񐔐����Ή�
// MH810103 GG119202(S) �ُ�f�[�^�t�H�[�}�b�g�ύX
extern	uchar			err_data2;
// MH810103 GG119202(E) �ُ�f�[�^�t�H�[�}�b�g�ύX

/*** function prototype ***/

// ope_ec_ctrl.c
extern	uchar 	Ec_Snd_Que_Regist(uchar kind, uchar *snddata);
extern	short 	Ec_Snd_Que_Read(uchar *kind, uchar *snddata);
extern	void	Ec_Data_Snd(uchar kind, void *snddata);
extern  void	Ec_Pri_Data_Snd(ushort req, uchar type);

extern	short	Ope_Ec_Event(ushort msg, uchar ope_faze);

extern	uchar	convert_brandno(const ushort brandno);
// MH810103 GG119202(S) �s�v�֐��폜
//extern	void	ec_usable_req(ushort msg);
// MH810103 GG119202(E) �s�v�֐��폜
extern	long	settlement_amount_ex(uchar *buf, uchar count);

extern	char	Ope_EcArmClearCheck(ushort msg);
extern	void	Ope_TimeOut_Cyclic_Disp(uchar ope_faze, ushort e_pram_set);
extern	void	Ope_TimeOut_Recept_Send(uchar ope_faze, ushort e_pram_set);
extern	void	ec_auto_cancel_timeout(void);

extern	void	EcSettlementPhaseError(uchar *dat, uchar kind);
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//extern	uchar	EcUseKindCheck(uchar kind);
extern	uchar	EcUseKindCheck(uchar kind);				// �d�q�}�l�[���ρAQR�R�[�h���σ`�F�b�N
extern	uchar	EcUseEMoneyKindCheck(uchar kind);		// �d�q�}�l�[���σ`�F�b�N
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

extern	void	ec_miryo_timeout(void);
extern	uchar	get_enable_multisettle(void);			// ���L���ȃu�����h��
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
extern	uchar	check_enable_multisettle(void);			// �@�\���L���ȃu�����h��
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j

// MH810103 GG119202(S) �J�ǁE�L�������ύX
//extern	uchar	isEcReady( const uchar kind );
// MH810103 GG119202(E) �J�ǁE�L�������ύX
// MH810103 GG119202(S) �s�v�����폜
//extern	uchar	isAnyEcBrandReady(const ushort *brand_ec_ary, const ushort ary_size);
// MH810103 GG119202(E) �s�v�����폜
extern	uchar	isEcBrandNoReady(const ushort brand_no);
// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//extern	uchar	isEcBrandNoEMoney(const ushort brand_no);
extern	uchar	isEcBrandNoEMoney(const ushort brand_no, uchar zandaka_chk);
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
extern	ushort	ConvertEcBrandToNo(const ushort brand_ec);
extern	uchar	isEcBrandNoEnabledForSetting(const ushort brand_no);
extern	uchar	isEcBrandNoEnabledForRecvTbl(const ushort brand_no);
// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//extern	uchar	isEcEnabled( const uchar kind );
//extern	uchar	isAnyEcBrandEnabled(const ushort *brand_ec_ary, const ushort ary_size);
extern	uchar	isEcEmoneyEnabled(uchar open_chk, uchar zandaka_chk);
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
// MH810103 GG119202(S) �J�ǁE�L�������ύX
extern	uchar	isEcBrandNoEnabled(ushort brand_no);
// MH810103 GG119202(E) �J�ǁE�L�������ύX
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6482 VP�Ő��Z���ɃN���W�b�g���d�q�}�l�[��I������ƁA�u�����h��I�������^�C�~���O�Ő��Z��ʂ̐��Z���@����u�R�[�h���ρv��������
extern	uchar	isEcBrandNoEnabledNoCtrl(ushort brand_no);
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6482 VP�Ő��Z���ɃN���W�b�g���d�q�}�l�[��I������ƁA�u�����h��I�������^�C�~���O�Ő��Z��ʂ̐��Z���@����u�R�[�h���ρv��������
// MH810103 GG119202(S) �u�����h���̓u�����h��񂩂�擾����
extern	int		getBrandName(EC_BRAND_CTRL *brand, uchar *buff);
// MH810103 GG119202(E) �u�����h���̓u�����h��񂩂�擾����
extern	char	EcFirst_Pay_Dsp( void );
extern	uchar	ec_split_data_check(void);
extern	void	Ope_EcEleUseDsp( void );
extern	void	SetEcVolume( void );
extern	void	EcDeemedSettlement( uchar *ope_faze );
extern	void	ReqEcDeemedJnlPrint( void );
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
extern	void	ec_BrandResultDisp(ushort brand_no, ushort reason_code);
extern	void 	ec_WarningMessage( ushort num );
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
extern	void	ec_MessageAnaOnOff( short OnOff, short num );
extern	void	Ope_EcPayStart(void);
extern	void	ec_wakeup_timeout(void);
extern	void	ec_miryo_timeout_after_disp(EC_SETTLEMENT_RES *pRes);
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
extern	void	EcRegistMiryoTimeoutError(EC_SETTLEMENT_RES *pRes, char err);
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
extern	void	ec_start_removal_wait(void);
extern	void	ec_flag_clear(uchar init);
extern	void	EcSendCtrlEnableData(void);
// MH810103 GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
extern  BOOL	Ec_check_cre_pay_limit(void);
// MH810103 GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
// MH810103 GG119202(S) �N���V�[�P���X��������������
extern	ushort	isEcBrandStsDataRecv(void);
extern	void	setEcBrandStsDataRecv( uchar kind );
// MH810103 GG119202(E) �N���V�[�P���X��������������
// MH810103 GG119202(S) ������������L�^�ɍĐ��Z�����󎚂���
extern	void	EcAlarmLog_Clear( void );									// ���O�N���A
extern	void	EcAlarmLog_Regist( EC_SETTLEMENT_RES *data );				// ���O�o�^
extern	void	EcAlarmLog_RepayLogRegist(Receipt_data *data);				// �Đ��Z���o�^
extern	ushort	EcAlarmLog_GetCount( void );								// ���O�����擾
extern	short	EcAlarmLog_LogRead( short idx, t_ALARM_SETTLEMENT_DATA *log, Receipt_data *paylog );	// ���O�擾
// MH810103 GG119202(E) ������������L�^�ɍĐ��Z�����󎚂���
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
extern	void	Ec_check_PendingBrandNego(void);
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
// MH810103 GG119202(S) JVMA���Z�b�g�����ύX
extern	void	Ec_check_PendingJvmaReset(void);
// MH810103 GG119202(E) JVMA���Z�b�g�����ύX
// MH810103 GG119202(S) �݂Ȃ����ψ������̓���
// MH810103 GG119202(S) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
//extern	void	EcRegistSettlementAbort(void);
extern	void	EcRegistDeemedSettlementError(uchar kind);
// MH810103 GG119202(E) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
// MH810103 GG119202(E) �݂Ȃ����ψ������̓���
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
extern	void	EcBrandClear(void);
extern	BOOL	EcCheckBrandSelectTable(BOOL in_mony_flg);
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810103 GG119202(S) �n�E�X�J�[�h�ɂ��J�[�h�������^�C���A�E�g�͐��Z���~�Ƃ���
extern	BOOL	IsSettlementBrand(ushort brandno);
// MH810103 GG119202(E) �n�E�X�J�[�h�ɂ��J�[�h�������^�C���A�E�g�͐��Z���~�Ƃ���
// MH810103 GG119202(S) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
extern	void	EcEnableNyukin(void);
// MH810103 GG119202(E) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
// MH810103(s) �d�q�}�l�[�Ή� ���Z��Ԓʒm��Ope_mod�ɂ��؂�ւ���
extern	uchar isEcEmoneyEnabledNotCtrl(uchar open_chk, uchar zandaka_chk);
// MH810103(e) �d�q�}�l�[�Ή� ���Z��Ԓʒm��Ope_mod�ɂ��؂�ւ���
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
extern	uchar	EcRecvDeemed_DataSet(uchar *buf);
extern	void	EcRecvDeemed_RegistPri( void );
extern	void 	EcErrCodeChk(uchar *err_code, uchar err_brand_index);
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
// MH810105 GG119202(S) �݂Ȃ����σv�����g�ɒ��O������e���󎚂���
extern	uchar	IsEcMiryoHikisariNG(Receipt_data *pData);
// MH810105 GG119202(E) �݂Ȃ����σv�����g�ɒ��O������e���󎚂���
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
extern	int		EcSubBrandCheck(ushort brand_no);
extern	int		EcGetSubBrandName(ushort brand_no, uchar kind, uchar idx, uchar *buff);
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105(S) MH364301 PiTaPa�Ή�
extern	uchar	EcBrandPostPayCheck(uchar brand_index);
// MH810105(E) MH364301 PiTaPa�Ή�

// jvma_comm.c
extern	void	jvma_init(void);
extern	void	jvma_trb(uchar err_kind);
extern	void	jvma_event(void);
extern	void	jvma_command_set(void);
extern	short	jvma_act(void);

// ec_ctrl.c
extern	void	ec_init(void);
extern	short	ec_recv(uchar *buf, short size);
extern	void 	Ec_recv_data_regist(uchar *buf, ushort size);

#endif	// _EC_DEF_H_
