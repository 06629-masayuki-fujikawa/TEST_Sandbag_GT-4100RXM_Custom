//[]----------------------------------------------------------------------[]
///	@brief			���σ��[�_R/W Operation control
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"cnm_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
#include	"irq1.h"
#include	"suica_def.h"
#include	"ec_def.h"
#include	"tbl_rkn.h"
#include	"lcd_def.h"
#include	"mdl_def.h"
#include	"flp_def.h"
#include	"lkmain.h"

/*
	CRW-MJA�}���`�}�l�[R/W�̐�����s�Ȃ��܂��B
	ope_suica_ctrl.c�����~���Ɏ�M�������J�X�^�}�C�Y���Ă��܂��B
*/

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S												*/
/*--------------------------------------------------------------------------*/

extern	uchar	suica_dsp_buff[30];						// ��ʕ\���p�\��������i�[�̈�
extern	uchar	suica_Log_wbuff[S_BUF_MAXSIZE];			// �ʐM���O�p�ꎞ�ҏW�̈�
extern	uchar	wrcvbuf[S_BUF_MAXSIZE+1];				// �f�[�^��M�p�̈ꎞ�ҏW�̈�
extern	uchar	err_wk[40];
extern	uchar	moni_wk[10];
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//// MH810103 GG119202(S) �݂Ȃ����ψ������̓���
//extern	const uchar	msg_idx[4];
//// MH810103 GG119202(E) �݂Ȃ����ψ������̓���
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

extern	const char *EcBrandKind_prn[];
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
extern	const char *QrBrandKind_prn[];
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

#define		Ec_Status_Chg()		Suica_Status_Chg()	// Rename

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/
static	uchar	Ec_Read_RcvQue(void);
static	void	ec_data_save(unsigned char *buf, short siz);
static	void	set_brand_tbl(EC_BRAND_TBL *tbl, EC_BRAND_TBL *recvtbl);
static	void	check_brand_tbl(EC_BRAND_TBL *tbl, EC_BRAND_TBL *recvtbl);
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
static	void	check_sub_brand_tbl(uchar *p, EC_SUB_BRAND_TBL *tbl);
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810103 GG119202(S) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
static	void	update_brand_tbl(EC_BRAND_TBL *srctbl, EC_BRAND_TBL *recvtbl);
// MH810103 GG119202(E) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
//static	void	ec_settlement_data_save(EC_SETTLEMENT_RES *data, uchar *buf);
static	void	ec_settlement_data_save(EC_SETTLEMENT_RES *data, uchar *buf, uchar errchk);
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��

static  void	snd_enable_timeout(void);
static  void	snd_disable_timeout(void);
static	void	snd_no_response_timeout(void);
// MH810103 GG119202(S) �s�v�����폜
//static	void	status_err_timeout(void);
// MH810103 GG119202(E) �s�v�����폜
static	void	not_open_timeout(void);

static short	EcRecvStatusData(uchar*);
static short	EcRecvSettData(uchar*);
static short	EcRecvBrandData(uchar*);

static	void	ReqEcAlmRctPrint(uchar prikind, EC_SETTLEMENT_RES *data);

static	void	recv_unexpected_status_data(void);
static	void	save_deemed_data(short brandno);

// MH810103 GG119202(S) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
//static	void	ec_errcode_err_chk(uchar *err_wk, uchar *ec_err_code);
static	void	ec_errcode_err_chk(uchar *err_wk, uchar *ec_err_code, uchar brand_index);
// MH810105 GG119202(S) �݂Ȃ����σv�����g�ɒ��O������e���󎚂���
//// MH810103 GG119202(E) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
//static	uchar	ec_errcode_err_chk_judg( uchar *ec_err_code );
//// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
static	uchar	ec_errcode_err_chk_judg(uchar *wk_err_code, const uchar ErrTbl[][3], uchar max);
// MH810105 GG119202(E) �݂Ȃ����σv�����g�ɒ��O������e���󎚂���
//static	void	ec_WarningMessage(uchar num);
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j

static	void	ecRegistBrandError( EC_BRAND_CTRL *brand_ctrl, uchar err_type );
// MH810103 GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
static	void	ec_set_brand_selected( void );
static	BOOL	ec_check_valid_brand( ushort brand_no );
// MH810103 GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
static	void	setEcBrandNegoComplete(void);
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
static BOOL	IsValidBrand(int brand_i, int brand_j);
static BOOL	IsBrandSpecified(ushort brandno);
// MH810103 GG119202(S) �n�E�X�J�[�h�ɂ��J�[�h�������^�C���A�E�g�͐��Z���~�Ƃ���
//static BOOL IsSettlementBrand(ushort brandno);
// MH810103 GG119202(E) �n�E�X�J�[�h�ɂ��J�[�h�������^�C���A�E�g�͐��Z���~�Ƃ���
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή��Q�l�j
static	uchar	EcNotifyCardData(ushort brand_no, uchar *dat);
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή��Q�l�j
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
static	void	EcTimerUpdate(void);
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�

// MH810100(S) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
extern	void	lcdbm_notice_dsp2( ePOP_DISP_KIND kind, uchar DispStatus, ulong add_info );
// MH810100(E) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
// MH810105 GG119202(S) ������������W�v�d�l���P
//// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//static	void	EcSetDeemedSettlementResult(EC_SETTLEMENT_RES *data);
//// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
// MH810105 GG119202(E) ������������W�v�d�l���P
// MH810105 GG119202(S) ���Ϗ������ɏ�Q�������������̓���
static	uchar	EcGetActionWhenFailure( ushort brand_no );
// MH810105 GG119202(E) ���Ϗ������ɏ�Q�������������̓���
// MH810105 GG119202(S) �d�q�}�l�[���σG���[�������̉�ʖ߂�d�l�ύX
static	uchar	IsEcContinueReading(uchar *pErrCode);
// MH810105 GG119202(E) �d�q�}�l�[���σG���[�������̉�ʖ߂�d�l�ύX
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
static	uchar	IsQrContinueReading(uchar *pErrCode);
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
static	uchar	EcGetMiryoTimeoutAction(ushort brand_no);
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX


/*--------------------------------------------------------------------------*/
/*			C O N S T A N T S												*/
/*--------------------------------------------------------------------------*/
typedef short (*CMD_FUNC)(uchar*);

typedef struct{
	unsigned char Command;
	CMD_FUNC Func;
}RCV_CMD_FUNC;

static	const
RCV_CMD_FUNC EcTbl[] = {
//	{ ��M������ޯ�,�����֐�},				/* ������ */
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	{ 0x02,			EcRecvSettData},		/* ���ό��ʃf�[�^ */
//	{ 0x08,			EcRecvStatusData},		/* ��ԃf�[�^ */
//	{ 0x10,			EcRecvBrandData},		/* �u�����h�I�����ʃf�[�^ */
	{ 0x01,			EcRecvSettData},		/* ���ό��ʃf�[�^ */
	{ 0x02,			EcRecvStatusData},		/* ��ԃf�[�^ */
	{ 0x08,			EcRecvBrandData},		/* �u�����h�I�����ʃf�[�^ */
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
};

// �u�����h�ʐݒ�Q�ƃe�[�u��
typedef struct {
	ushort		brand_no;				// BRANDNO_*
// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//	uchar		address;				// ���ʃp�����[�^�[ �A�h���X
//	uchar		pos;					// ���ʃp�����[�^�[ �ݒ�ʒu
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
// MH810103 GG119202(S) ���ς���u�����h�I��ݒ���Q�Ƃ���
	uchar		address;				// ���ʃp�����[�^�[ �A�h���X
	uchar		pos;					// ���ʃp�����[�^�[ �ݒ�ʒu
// MH810103 GG119202(E) ���ς���u�����h�I��ݒ���Q�Ƃ���
	uchar		brand_err_no;			// �u�����h�ُ�G���[�ԍ�
} EC_BRAND_ENABLED_PARAM_TBL;

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
typedef struct {
	uchar		brand_no;				// �T�u�u�����h���
	uchar		address;				// ���ʃp�����[�^�[ �A�h���X
	uchar		pos;					// ���ʃp�����[�^�[ �ݒ�ʒu
	uchar		dmy;
} EC_SUB_BRAND_ENABLED_PARAM_TBL;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//const EC_BRAND_ENABLED_PARAM_TBL EcBrandEnabledParamTbl[] = {
//	// �e�u�����h�̎g�p�L���ݒ�Ή� (���ʃp�����[�^�[ 50-xxxx)
//	//	BRANDNO_*			address	pos    brand_err_no
//	{	BRANDNO_KOUTSUU,	1,		6,	ERR_EC_KOUTU_ABNORMAL,		},
//	{	BRANDNO_QUIC_PAY,	1,		5,	ERR_EC_QUICPAY_ABNORMAL,	},
//	{	BRANDNO_ID,			1,		4,	ERR_EC_ID_ABNORMAL,			},
//	{	BRANDNO_SAPICA,		1,		3,	0,							},
//	{	BRANDNO_WAON,		1,		2,	ERR_EC_WAON_ABNORMAL,		},
//	{	BRANDNO_NANACO,		1,		1,	ERR_EC_NANACO_ABNORMAL,		},
//	{	BRANDNO_EDY,		2,		6,	ERR_EC_EDY_ABNORAML,		},
////	{	BRANDNO_HOUJIN,		2,		2,	0,							},
//	{	BRANDNO_CREDIT,		2,		1,	ERR_EC_CREDIT_ABNORMAL,		},
//};
// MH810103 GG119202(S) ���ς���u�����h�I��ݒ���Q�Ƃ���
//const EC_BRAND_ENABLED_PARAM_TBL EcBrandEnabledParamTbl[] = {
//	//	BRANDNO_*			brand_err_no
//	{	BRANDNO_KOUTSUU,	ERR_EC_KOUTU_ABNORMAL,		},
//	{	BRANDNO_QUIC_PAY,	ERR_EC_QUICPAY_ABNORMAL,	},
//	{	BRANDNO_ID,			ERR_EC_ID_ABNORMAL,			},
//	{	BRANDNO_SAPICA,		0,							},
//	{	BRANDNO_WAON,		ERR_EC_WAON_ABNORMAL,		},
//	{	BRANDNO_NANACO,		ERR_EC_NANACO_ABNORMAL,		},
//	{	BRANDNO_EDY,		ERR_EC_EDY_ABNORAML,		},
//	{	BRANDNO_CREDIT,		ERR_EC_CREDIT_ABNORMAL,		},
//// �񌈍σu�����h
//	{	BRANDNO_TCARD,		ERR_EC_TCARD_ABNORAML,		},
//// �㌈�σu�����h
//	{	BRANDNO_HOUJIN,		ERR_EC_HOUJIN_ABNORAML,		},
//};
	// ���ς���u�����h�̂�
const EC_BRAND_ENABLED_PARAM_TBL EcBrandEnabledParamTbl[] = {
	//	brand_no			address	pos	brand_err_no
	{	BRANDNO_KOUTSUU,	1,		6,	ERR_EC_KOUTU_ABNORMAL,		},
	{	BRANDNO_QUIC_PAY,	1,		5,	ERR_EC_QUICPAY_ABNORMAL,	},
	{	BRANDNO_ID,			1,		4,	ERR_EC_ID_ABNORMAL,			},
	{	BRANDNO_SAPICA,		1,		3,	0,							},
	{	BRANDNO_WAON,		1,		2,	ERR_EC_WAON_ABNORMAL,		},
	{	BRANDNO_NANACO,		1,		1,	ERR_EC_NANACO_ABNORMAL,		},
	{	BRANDNO_EDY,		2,		6,	ERR_EC_EDY_ABNORAML,		},
	{	BRANDNO_CREDIT,		2,		1,	ERR_EC_CREDIT_ABNORMAL,		},
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	{	BRANDNO_QR,			2,		2,	ERR_EC_QR_ABNORMAL,			},
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105(S) MH364301 PiTaPa�Ή�
	{	BRANDNO_PITAPA,		2,		5,	ERR_EC_PITAPA_ABNORMAL		},
// MH810105(E) MH364301 PiTaPa�Ή�
};
// MH810103 GG119202(E) ���ς���u�����h�I��ݒ���Q�Ƃ���
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
// �T�u�u�����h�����u�����h�ԍ����ǉ����ꂽ�ꍇ��
// ���L�e�[�u���ɒǉ����邱��
const ushort EcSubBrandUseTbl[] = {
	BRANDNO_QR,
};

// MH810105(S) MH364301 QR�R�[�h���ϑΉ��i�d�l�ύX�j
//const EC_SUB_BRAND_ENABLED_PARAM_TBL QrSubBrandEnabledParamTbl[] = {
//	//	brand_no			address	pos
//	{	DPAY,				50,		1	},
//	{	PAYPAY,				50,		2,	},
//	{	AUPAY,				50,		3,	},
//	{	RAKUTENPAY,			50,		4,	},
//	{	MERPAY,				50,		5,	},
//};
// MH810105(E) MH364301 QR�R�[�h���ϑΉ��i�d�l�ύX�j
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

// MH810103 GG119202(S) �s�v�����폜
//const ushort EcBrandEmoney[] = {
//	// �d�q�}�l�[�̃u�����h
//	EC_UNKNOWN_USED,
//	EC_ZERO_USED,
//	EC_EDY_USED,
//	EC_NANACO_USED,
//	EC_WAON_USED,
//	EC_SAPICA_USED,
//	EC_KOUTSUU_USED,
//	EC_ID_USED,
//	EC_QUIC_PAY_USED,
//};
//
//const ushort EcBrandCredit[] = {
//	// �N���W�b�g�̃u�����h
//	EC_CREDIT_USED,
//};
// MH810103 GG119202(E) �s�v�����폜

typedef struct {
	//	EC_*_USED��BRANDNO_*�̑Ή���`�p
	ushort			brand_ec;				// EC_*_USED
	ushort			brand_no;				// BRANDNO_*
} EC_BRAND_NO_CONV_TBL;

// ���σu�����h�̂�
const EC_BRAND_NO_CONV_TBL EcBrandNoConvTbl[] = {
	//	EC_*_USED��BRANDNO_*�̑Ή�
	//	EC_*_USED				BRANDNO_*
	{	EC_EDY_USED,			BRANDNO_EDY			},
	{	EC_NANACO_USED,			BRANDNO_NANACO		},
	{	EC_WAON_USED,			BRANDNO_WAON		},
	{	EC_SAPICA_USED,			BRANDNO_SAPICA		},
	{	EC_KOUTSUU_USED,		BRANDNO_KOUTSUU		},
	{	EC_ID_USED,				BRANDNO_ID			},
	{	EC_QUIC_PAY_USED,		BRANDNO_QUIC_PAY	},
	{	EC_CREDIT_USED,			BRANDNO_CREDIT		},
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	{	EC_QR_USED,				BRANDNO_QR,			},
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105(S) MH364301 PiTaPa�Ή�
	{	EC_PITAPA_USED,			BRANDNO_PITAPA		},
// MH810105(E) MH364301 PiTaPa�Ή�
};

// �G���[�R�[�h���� ���L��EcErrCodeTbl�ƈ�v
enum {
	EC_ERR_C,
	EC_ERR_D,
	EC_ERR_E,
	EC_ERR_G,
	EC_ERR_K,
	EC_ERR_P,
	EC_ERR_S,
// MH810103 GG119202(S) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
	EC_ERR_a,
	EC_ERR_c,
	EC_ERR_d,
	EC_ERR_e,
	EC_ERR_g,
	EC_ERR_h,
	EC_ERR_i,
// MH810103 GG119202(E) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	EC_ERR_QR_P,
	EC_ERR_QR_Q,
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	EC_ERR_MAX,
};

// �e�G���[�R�[�h�ԍ��ɃG���[�����݂��Ȃ� = 0
// �����A����舵�����ł��܂���/�����A�N���W�b�g�J�[�h�͂���舵���ł��܂��� = 1
// ���̃J�[�h�͖����ł� = 2
// ���̃J�[�h�͎g���܂��� = 3
// ���̃J�[�h�͊����؂�ł� = 4
// �J�[�h�̓ǂݎ��Ɏ��s���܂��� = 5
// MH810103 GG119202(S) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
//// �J�[�h�̎c��������܂��� = 6
// �c���s���ł� = 6
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
// ����Ɏ��s���܂����B���̎x�����@�������p�������� = 10
// ���̃R�[�h�͊����؂�ł� = 11
// �R�[�h���X�V���A������x��蒼���Ă������� = 12
// ���̃R�[�h�͎g���܂��� = 13
// ���̃R�[�h�͎c���s���ł� = 14
// ���Z���x�z�𒴂��Ă��܂��B���̎x�����@�������p�������� = 15
// ������x��蒼���Ă������� = 16
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105 GG119202(S) iD���ώ��̈ē��\���Ή�
// iD�A�v�������b�N����Ă��܂� = 94
// MH810105 GG119202(E) iD���ώ��̈ē��\���Ή�
// GG118809_GG118909(S) ����������m�F���̖������R�ɑΉ�����
// �����c���Ɖ�Ŗ�����������m�F = 96
// GG118809_GG118909(E) ����������m�F���̖������R�ɑΉ�����
// �����Ȃ� = 98
// MH810103 GG119202(E) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
// �e�ڍ׃G���[�ɂĕ\�������L�A�����ԍ���ݒ�
const uchar EcErrCodeTbl[EC_ERR_MAX][ERR_NUM_MAX] = {
//		0  1  2  3	4  5  6  7  8  9 
	{	// C�R�[�h�G���[
		0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 	/* ���ށF 0�` 9 */
		0, 0, 1, 1, 1, 1, 0, 0, 0, 0,  	/* ���ށF10�`19 */
		1, 1, 1, 1, 1, 0, 0, 0, 0, 0,  	/* ���ށF20�`29 */
		0, 0, 0, 1, 0, 0, 0, 0, 0, 0,  	/* ���ށF30�`39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF40�`49 */
		1, 1, 0, 1, 1, 1, 1, 1, 1, 0,  	/* ���ށF50�`59 */
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF60�`69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF70�`79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF80�`89 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF90�`99 */
	},
	{	// D�R�[�h�G���[
		0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 	/* ���ށF 0�` 9 */
		1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 	/* ���ށF10�`19 */
		1, 1, 2, 1, 5, 5, 5, 5, 1, 1, 	/* ���ށF20�`29 */
		1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 	/* ���ށF30�`39 */
		1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 	/* ���ށF40�`49 */
		1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 	/* ���ށF50�`59 */
		1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 	/* ���ށF60�`69 */
		0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 	/* ���ށF70�`79 */
		1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 	/* ���ށF80�`89 */
		1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 	/* ���ށF90�`99 */
	},
	{	// E�R�[�h�G���[
		0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 	/* ���ށF 0�` 9 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF10�`19 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF20�`29 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF30�`39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF40�`49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF50�`59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF60�`69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF70�`79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF80�`89 */
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF90�`99 */
	},
	{	// G�R�[�h�G���[
		0, 0, 0, 0, 0, 0, 6, 3, 0, 0, 	/* ���ށF 0�` 9 */
		0, 0, 3, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF10�`19 */
		0, 0, 3, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF20�`29 */
		3, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF30�`39 */
		0, 0, 3, 3, 0, 0, 0, 0, 0, 0,  	/* ���ށF40�`49 */
		0, 0, 0, 0, 3, 3, 3, 0, 0, 0,  	/* ���ށF50�`59 */
		3, 2, 0, 0, 0, 3, 0, 3, 3, 3,  	/* ���ށF60�`69 */
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  	/* ���ށF70�`79 */
		3, 3, 0, 4, 0, 3, 0, 0, 0, 0,  	/* ���ށF80�`89 */
		0, 1, 0, 0, 3, 1, 3, 3, 3, 3,  	/* ���ށF90�`99 */
	},
	{	// K�R�[�h�G���[
		0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 	/* ���ށF 0�` 9 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF10�`19 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF20�`29 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF30�`39 */
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF40�`49 */
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF50�`59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF60�`69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF70�`79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF80�`89 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF90�`99 */
	},
	{	// P�R�[�h�G���[
		0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 	/* ���ށF 0�` 9 */
		0, 0, 1, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF10�`19 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF20�`29 */
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF30�`39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF40�`49 */
		1, 1, 1, 1, 1, 1, 0, 0, 0, 0,  	/* ���ށF50�`59 */
		0, 0, 0, 0, 0, 5, 0, 0, 5, 5,  	/* ���ށF60�`69 */
		5, 5, 5, 5, 5, 5, 5, 0, 5, 0,  	/* ���ށF70�`79 */
		5, 5, 0, 5, 5, 0, 0, 0, 0, 0,  	/* ���ށF80�`89 */
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF90�`99 */
	},
	{	// S�R�[�h�G���[
		0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 	/* ���ށF 0�` 9 */
		0, 0, 1, 0, 1, 0, 0, 0, 1, 1,  	/* ���ށF10�`19 */
		1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF20�`29 */
		0, 0, 0, 0, 0, 1, 0, 0, 0, 0,  	/* ���ށF30�`39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF40�`49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF50�`59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF60�`69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF70�`79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1,  	/* ���ށF80�`89 */
		0, 1, 1, 1, 1, 0, 0, 0, 0, 0,  	/* ���ށF90�`99 */
	},
// MH810103 GG119202(S) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
	{	// a�R�[�h�G���[
		0,98, 0, 0, 0, 0, 0, 0, 0, 0, 	/* ���ށF 0�` 9 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF10�`19 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF20�`29 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF30�`39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF40�`49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF50�`59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF60�`69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF70�`79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF80�`89 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF90�`99 */
	},
	{	// c�R�[�h�G���[
// MH810105 GG119202(S) ���σG���[�R�[�h�e�[�u��������
//// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
////		0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 	/* ���ށF 0�` 9 */
//		0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 	/* ���ށF 0�` 9 */
//// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
		0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 	/* ���ށF 0�` 9 */
// MH810105 GG119202(E) ���σG���[�R�[�h�e�[�u��������
// MH810105 GG119202(S) ���σG���[�R�[�h�e�[�u��������
//		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF10�`19 */
// MH810105(S) MH364301 PiTaPa�Ή�
//	   98, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF10�`19 */
	   98, 1, 1, 1, 1, 0, 0, 0, 0, 0,  	/* ���ށF10�`19 */
// MH810105(E) MH364301 PiTaPa�Ή�
// MH810105 GG119202(E) ���σG���[�R�[�h�e�[�u��������
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF20�`29 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF30�`39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF40�`49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF50�`59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF60�`69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF70�`79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF80�`89 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF90�`99 */
	},
	{	// d�R�[�h�G���[
// MH810105 GG119202(S) ���σG���[�R�[�h�e�[�u��������
//// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
////		0, 6, 5, 0,98, 3, 0, 3, 3, 0, 	/* ���ށF 0�` 9 */
//		0, 6, 5, 5,98, 3, 3, 3, 3, 3, 	/* ���ށF 0�` 9 */
//// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
		0, 6, 5, 5,98, 3, 3, 0, 0, 3, 	/* ���ށF 0�` 9 */
// MH810105 GG119202(E) ���σG���[�R�[�h�e�[�u��������
// MH810105(S) MH364301 PiTaPa�Ή�
//// MH810104(S) nanaco�EiD�EQUICPay�Ή�2
////// MH810103(s) �d�q�}�l�[�Ή� #5458 �ꕔ�̏ڍ׃G���[�R�[�h��M���Ɏd�l���̋L�ڂƈقȂ�G���[����������
//////		0, 0, 0, 3, 0, 0, 0, 0, 0, 0,  	/* ���ށF10�`19 */
////		0, 0, 0, 3, 5, 98, 98, 0, 0, 0,  	/* ���ށF10�`19 */
////// MH810103(s) �d�q�}�l�[�Ή� #5458 �ꕔ�̏ڍ׃G���[�R�[�h��M���Ɏd�l���̋L�ڂƈقȂ�G���[����������
//		0, 0, 3, 3, 5, 98, 98, 0, 0, 0,  	/* ���ށF10�`19 */
//// MH810104(E) nanaco�EiD�EQUICPay�Ή�2
//		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF20�`29 */
		0, 0, 3, 3, 5, 98, 98, 0, 0, 1,  	/* ���ށF10�`19 */
		1, 1, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF20�`29 */
// MH810105(E) MH364301 PiTaPa�Ή�
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF30�`39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF40�`49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF50�`59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF60�`69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF70�`79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF80�`89 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF90�`99 */
	},
	{	// e�R�[�h�G���[
// MH810105 GG119202(S) iD���ώ��̈ē��\���Ή�
//		0, 0, 3, 3, 4, 0, 0, 0, 0, 0, 	/* ���ށF 0�` 9 */
		0, 0, 3, 3, 4,94, 0, 0, 0, 0, 	/* ���ށF 0�` 9 */
// MH810105 GG119202(E) iD���ώ��̈ē��\���Ή�
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF10�`19 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF20�`29 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF30�`39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF40�`49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF50�`59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF60�`69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF70�`79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF80�`89 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF90�`99 */
	},
	{	// g�R�[�h�G���[
		3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 	/* ���ށF 0�` 9 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF10�`19 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF20�`29 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF30�`39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF40�`49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF50�`59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF60�`69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF70�`79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF80�`89 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF90�`99 */
	},
	{	// h�R�[�h�G���[
// MH810104(S) nanaco�EiD�EQUICPay�Ή�2
//// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
////		0, 0, 0,98, 0, 0, 0, 0, 0, 0, 	/* ���ށF 0�` 9 */
//		0, 0, 3,98, 0, 0, 0, 0, 0, 0, 	/* ���ށF 0�` 9 */
//// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
// MH810105 GG119202(S) ���σG���[�R�[�h�e�[�u��������
//		0, 3, 3,98, 0, 0, 0, 0, 0, 0, 	/* ���ށF 0�` 9 */
//// MH810104(E) nanaco�EiD�EQUICPay�Ή�2
		0, 0, 3,98, 0, 0, 0, 0, 0, 0, 	/* ���ށF 0�` 9 */
// MH810105 GG119202(E) ���σG���[�R�[�h�e�[�u��������
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF10�`19 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF20�`29 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF30�`39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF40�`49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF50�`59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF60�`69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF70�`79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF80�`89 */
// MH810103 GG118809_GG118909(S) ����������m�F���̖������R�ɑΉ�����
//	   98,98, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF90�`99 */
	   96,98, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF90�`99 */
// MH810103 GG118809_GG118909(E) ����������m�F���̖������R�ɑΉ�����
	},
	{	// i�R�[�h�G���[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//		1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 	/* ���ށF 0�` 9 */
		1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 	/* ���ށF 0�` 9 */
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF10�`19 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF20�`29 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF30�`39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF40�`49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF50�`59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF60�`69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF70�`79 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF80�`89 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF90�`99 */
	},
// MH810103 GG119202(E) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	{	// P�R�[�h�G���[�iQR�j
	   10,10,10, 0, 0, 0, 0, 0, 0, 0, 	/* ���ށF 0�` 9 */
	   10,10,10, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF10�`19 */
	   11,12,13, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF20�`29 */
	   14,10,15, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF30�`39 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF40�`49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF50�`59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF60�`69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF70�`79 */
	   10, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF80�`89 */
		1, 1, 1, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF90�`99 */
	},
	{	// Q�R�[�h�G���[�iQR�j
	   10, 0,10,10, 0, 0, 0, 0, 0, 0, 	/* ���ށF 0�` 9 */
	   10,10,10, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF10�`19 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF20�`29 */
		0, 0, 0,10,16,16, 0, 0, 0, 0,  	/* ���ށF30�`39 */
	   16, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF40�`49 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF50�`59 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF60�`69 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF70�`79 */
	   10, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF80�`89 */
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  	/* ���ށF90�`99 */
	},
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
};

// �G���[�R�[�h���� ���L��EcTimeoutErrTbl�ƈ�v
enum {
	NG308,
// MH810103 GG118809_GG118909(S) ����������m�F���̖������R�ɑΉ�����
//// LH039701_LH040101(S) �L�����Z���{�^���������ɃG���[�o�^���Ȃ�
//	NGa01,
//// LH039701_LH040101(E) �L�����Z���{�^���������ɃG���[�o�^���Ȃ�
// MH810103 GG118809_GG118909(E) ����������m�F���̖������R�ɑΉ�����
	NG_ERR_MAX,
};

// ���L�e�[�u�����̏ڍ׃G���[����������M�O�Ɏ�M�����ꍇ�A�^�C���A�E�g�G���[�Ƃ��Ĉ���
const uchar EcTimeoutErrTbl[NG_ERR_MAX][3] = {
	{'3', '0', '8'},	// �ǂݎ��҂��^�C���A�E�g
// MH810103 GG118809_GG118909(S) ����������m�F���̖������R�ɑΉ�����
	// �����c���Ɖ�ŃL�����Z���{�^���������������Ƃ��L�^�c�����߁A
	// �e�[�u������폜����
//// LH039701_LH040101(S) �L�����Z���{�^���������ɃG���[�o�^���Ȃ�
//	{'a', '0', '1'},	// �ǂݎ��L�����Z��
//// LH039701_LH040101(E) �L�����Z���{�^���������ɃG���[�o�^���Ȃ�
// MH810103 GG118809_GG118909(E) ����������m�F���̖������R�ɑΉ�����
};

// MH810105 GG119202(S) �݂Ȃ����σv�����g�ɒ��O������e���󎚂���
// ������e�������m��(���s)�ƂȂ�ڍ׃G���[�R�[�h
static const uchar EcMiryoFailErrTbl[3] = {
	'h', '9', '0',		// �������肪����Ă��Ȃ�
};
// MH810105 GG119202(E) �݂Ȃ����σv�����g�ɒ��O������e���󎚂���

// MH810105 GG119202(S) �d�q�}�l�[���σG���[�������̉�ʖ߂�d�l�ύX
static const uchar EcContinueReadingTbl[3] = {
	'd', '0', '2',		// �������J�[�h���o
};
// MH810105 GG119202(E) �d�q�}�l�[���σG���[�������̉�ʖ߂�d�l�ύX

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
// �G���[�R�[�h���� ���L��QrContinueReadingTbl�ƈ�v
enum {
	NG306,
	NGP20,
	NGP21,
	NG_QR_ERR_MAX,
};
static const uchar QrContinueReadingTbl[NG_QR_ERR_MAX][3] = {
	{'3', '0', '6',},		// �ǂݎ�莸�s
	{'P', '2', '0',},		// �����؂�R�[�h
	{'P', '2', '1',},		// �R�[�h�G���[
};
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

static uchar Card_Work[20];
static EC_SETTLEMENT_RES alm_rct_info;

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
const uchar *sub_brand_name_list_QR[QR_PAY_KIND_MAX] = {
	"���ތ���",			// [00]
	"Alipay",			// [01]
	"d����",			// [02]
	"LINE Pay",			// [03]
	"PayPay",			// [04]
	"�y�V�y�C",			// [05]
	"WeChat Pay",		// [06]
	"��� QR �R�[�h",	// [07]
	"au PAY",			// [08]
	"�����y�C",			// [09]
	"Origami Pay",		// [10]
	"��s Pay",			// [11]
	"QUO �J�[�h Pay",	// [12]
	"VIA-Dash",			// [13]
	"VIA-Global Pay",	// [14]
	"Smart Code",		// [15]
	"J-Coin Pay",		// [16]
	"Amazon Pay",		// [17]
	"JKO Pay",			// [18]
	"GLN Pay",			// [19]
	"Bank Pay",			// [20]
	"FamiPay",			// [21]
	"atone",			// [22]
};
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/
//[]----------------------------------------------------------------------[]
///	@brief			Ec Snd Data que Regist
//[]----------------------------------------------------------------------[]
///	@return			0
//[]----------------------------------------------------------------------[]
///	@note			Suica_Snd_regist()���Q�l(���g�𕪊�����)
///					�\���̖��Ȃǂ�Suica�̂܂�(�]����Suica�폜���ɕύX���邱��)
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar	Ec_Snd_Que_Regist(uchar kind, uchar *snddata)
{
	uchar	count_status, Read_pt;

	count_status = 1;															// �ޯ̧�ð�����
	for (Read_pt = 0; Read_pt < 5; Read_pt++) {									// �����M�ް��̌���
		if (!Suica_Snd_Buf.Suica_Snd_q[Read_pt].snd_kind) {						// �����M�ް��������
			count_status = 0;													// �ޯ̧�ð�����X�V
			break;
		}
	}
	if (count_status != 0) {													// �ޯ̧���ق̏ꍇ
		for (Read_pt = 0; Read_pt < BUF_MAX_DELAY_COUNT; Read_pt++) {			// �����M�ް��̌���
			xPause(BUF_MAX_DELAY_TIME);											// �ޯ̧MAX���̑��M�����҂�
			if (!Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.write_wpt].snd_kind) {	// �����M�ް�������Α��M�ް��̓o�^���s
				break;
			}
		}
	}

	if (Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.write_wpt].snd_kind != 0) {		// �ް������łɊi�[�ς݂̏ꍇ
		if (Suica_Snd_Buf.read_wpt > 3) {										// ذ���߲����MAX�ȏ�H
			Suica_Snd_Buf.read_wpt = 0;											// ����̐擪���w��
		} else {
			Suica_Snd_Buf.read_wpt++;											// ذ���߲������UP
		}
	}

	Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.write_wpt].snd_kind = kind;
	memcpy(Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.write_wpt].Snd_Buf, snddata, (size_t)S_BUF_MAXSIZE);

	if (Suica_Snd_Buf.write_wpt > 3) {
		Suica_Snd_Buf.write_wpt = 0;
	} else {
		Suica_Snd_Buf.write_wpt++;
	}

	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			Ec Snd Data que read
//[]----------------------------------------------------------------------[]
///	@return			���M�f�[�^��
//[]----------------------------------------------------------------------[]
///	@note			Suica_Snd_regist()���Q�l(���g�𕪊�����)
///					�\���̖��Ȃǂ�Suica�̂܂�(�]����Suica�폜���ɕύX���邱��)
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
short	Ec_Snd_Que_Read(uchar *kind, uchar *snddata)
{
	short	wpkind;

	wpkind = 0;
	if (Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].snd_kind != 0) {
		memcpy(snddata, Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].Snd_Buf, (size_t)S_BUF_MAXSIZE);
		*kind = Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].snd_kind;
		switch (*kind) {
		case S_CNTL_DATA: 									/* �����ް��̑��M�v�� */
			wpkind = EC_CMD_LEN_CTRL_DATA;
			break;
		case S_SELECT_DATA:									/* �I�����i�ް��̑��M�v�� */
			wpkind = EC_CMD_LEN_SLCT_DATA;
			break;
		case S_INDIVIDUAL_DATA:								/* ���ް��̑��M�v�� */
			wpkind = EC_CMD_LEN_INDI_DATA;
			break;
		case S_BRAND_SET_DATA:								/* �u�����h�ݒ��ް��̑��M�v�� */
		case S_BRAND_SET_DATA2:								/* �u�����h�ݒ��ް�2�̑��M�v�� */
			wpkind = EC_CMD_LEN_BSET_DATA;
			break;
		case S_VOLUME_CHG_DATA:								/* ���ʕύX�ް��̑��M�v�� */
			wpkind = EC_CMD_LEN_VSET_DATA;
			break;
		case S_READER_MNT_DATA:								/* ���[�_�����e�i���X�ް��̑��M�v�� */
			wpkind = EC_CMD_LEN_MENT_DATA;
			break;
		case S_BRAND_SEL_DATA:								/* �u�����h�I���ް��̑��M�v�� */
			wpkind = EC_CMD_LEN_BSEL_DATA;
			break;
// MH810103 GG119202(S) �u�����h�I�����ʉ����f�[�^
		case S_BRAND_RES_RSLT_DATA:							/* �u�����h�I�����ʉ����ް��̑��M�v�� */
			wpkind = EC_CMD_LEN_BRRT_DATA;
			break;
// MH810103 GG119202(E) �u�����h�I�����ʉ����f�[�^
		default:											/* ����ޑ��M�v���Ȃ� */
			break;
		}
	}
	return wpkind;
}

//[]----------------------------------------------------------------------[]
///	@brief			Ec Snd Data Set
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@note			Suica_Data_Snd()���Q�l
///					�\���̖��Ȃǂ�Suica�̂܂�(�]����Suica�폜���ɕύX���邱��)
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	Ec_Data_Snd(uchar kind, void *snddata)
{
	int		i, j;
	uchar	*p;
	long	tmp = 0L;
	ulong	paydata = 0L;
	EC_BRAND_TBL		*brand_tbl;
	EC_VOLUME_TBL		*volume_tbl;
	EC_MNT_TBL			*mnt_tbl;
	EC_BRAND_SEL_TBL	*brandsel_tbl;
	uchar	work_buf[3];
// MH810103 GG119202(S) �u�����h�I�����ʉ����f�[�^
	EC_BRAND_RES_RSLT_TBL	*brandresrslt_tbl;
// MH810103 GG119202(E) �u�����h�I�����ʉ����f�[�^

	/* ���σ��[�_���g�p�H */
	if (isEC_USE() == 0) {
	// ���σ��[�_�ڑ�����
		return;
	}
	if (Suica_Rec.suica_err_event.BIT.PAY_DATA_ERR != 0) {
	// ��M�������ϊz�Ǝ�M�����I�����i�f�[�^�Ƃō��ق�������
		return;			// �ȍ~���ׂĂ̓d�����M�����ɔ�����
	}
	if (OPECTL.Ope_mod == 13) {
	/* ̪��ނ��C�����Z�������ꍇ�d�����M���Ȃ� */
		return;
	}

	memset(work_buf, 0, sizeof(work_buf));
	memset(suica_work_buf, 0, sizeof(suica_work_buf));
	Suica_Rec.snd_kind = kind;	/* ���M��ʂ̾�� */

	switch (kind) {
	case S_CNTL_DATA:
	/* �����ް��쐬 */
// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�(�R�����g�̂�)
		// ����f�[�^�𑗐M����ꍇ�A�K��Ec_Pri_Data_Snd()�o�R�œ��Y�֐����N�����邱��
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�(�R�����g�̂�)
		memcpy(&suica_work_buf, snddata, sizeof(suica_work_buf[0]));
		if (Suica_Rec.Data.BIT.INITIALIZE == 0) {			/* �������I���H */
			return;
		}
		if (suica_work_buf[0] == 0x80 && Suica_Rec.Data.BIT.SEND_CTRL80 != 0) {	// ���M�Ώۂ�����I��(0x80)�ő��M�\��ԂłȂ��ꍇ
			return;																// ���M���Ȃ��Ŕ�����
		}
		if (suica_work_buf[0] == 0x80) {
			Suica_Rec.Data.BIT.SEND_CTRL80 = 1;									// ����I�����M�ς݃t���O�Z�b�g
		}
Suica_Data_Snd_10:
		if ((suica_work_buf[0] & 0x01 ) == 0) {
		// ��t�֎~/����I��
			// �u�J�[�h�������v�͎�t�֎~�𑗐M���Ȃ�
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//			if(Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC) {
			if(isEC_STS_CARD_PROC()) {
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
				return;
			}
// MH810103 GG119202(S) �}���`�u�����h�d�l�ύX�Ή�
//// GG119200(S) �u�����h�I��҂����ɓ����ŋ��z�ύX�r�b�g�𗧂ĂȂ�
//			if(( suica_work_buf[0] & S_CNTL_PRICE_CHANGE_BIT ) &&	// ���z�ύX�r�b�gON����
//			   ( !Suica_Rec.Data.BIT.ADJUSTOR_NOW )){				// �I�����i�f�[�^�����M�H
//				suica_work_buf[0] &= ~S_CNTL_PRICE_CHANGE_BIT;		// ���z�ύX�r�b�g��OFF���Ď�t�֎~�𑗐M����
//			}
//// GG119200(E) �u�����h�I��҂����ɓ����ŋ��z�ύX�r�b�g�𗧂ĂȂ�
// MH810103 GG119202(E) �}���`�u�����h�d�l�ύX�Ή�
			Suica_Rec.Data.BIT.OPE_CTRL = 0;
// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
//			if (suica_work_buf[0] == 0 || suica_work_buf[0] == S_CNTL_PRICE_CHANGE_BIT) {
//			// ��t�֎~/���z�ύX
			if( !( suica_work_buf[0] & S_CNTL_REBOOT_OK_BIT ) ){	// �ċN����bit OFF�H
				// �ċN����bit��OFF�̂��̂̂ݑ҂����킹/�đ����g���C�ΏۂƂ���
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
				tmp = EC_DISABLE_WAIT_TIME;
				LagTim500ms(LAG500_SUICA_NONE_TIMER, (short)((tmp * 2) + 1), snd_disable_timeout);	// ��t�֎~�����ް����M��������ϊJ�n(5s)
				LagCan500ms( LAG500_SUICA_NO_RESPONSE_TIMER );					// ��t�������ް����M���������ؾ��
				Status_Retry_Count_OK = 0;										/* ��ײ���Đ��������� */
				LagCan500ms(LAG500_EC_NOT_READY_TIMER);							// ���[�_NotReady�^�C�}�[���̎�t�֎~���M�Ń^�C�}�[�L�����Z��
				ECCTL.not_ready_timer = 0;
// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
				ECCTL.prohibit_snd_bsy = (uchar)( suica_work_buf[0] | 0x01 );	// �����Ď��Ώێw�ߎ�ʕۑ�(��t����/�֎~bit��ON����)
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
			}
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
			// �I�����i�f�[�^���M���������F���M�f�[�^�̓L���[�C���O����邽�߁A��ԃf�[�^��M���܂��L���[�ɂ���\��������̂ŁA�ł��������M�ŉ�������
			Suica_Rec.Data.BIT.SELECT_SND = 0;									// �I�����i�f�[�^���M���@������
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		} else if ((suica_work_buf[0] & 0x01) == 1) {
		// ��t����
			if ((OPECTL.op_faz == 8)											// �d�q�}�̒�~����
			||	(OPECTL.InquiryFlg != 0)										// �J�[�h��������
			||	(OPECTL.ChkPassSyu != 0)										// ����߽��������
			||	((Suica_Rec.suica_err_event.BYTE & 0xFB)!= 0)					// ���σ��[�_�G���[����(�J�ǈُ�͏���)
			||	(Suica_Rec.Data.BIT.COMM_ERR != 0)								// ���σ��[�_�ʐM�ُ픭��
			||	(Suica_Rec.Data.BIT.MIRYO_TIMEOUT != 0)) {						// Suica�����^�C���A�E�g
				return;
			}
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//			if (OPECTL.Ope_mod == 2 && 
//				(!WAR_MONEY_CHECK && !isEcEnabled(EC_CHECK_CREDIT)) ) {
//			// ���Z���Ɍ�������������A�N���W�b�g���ϕs�̏ꍇ�͎�t���𑗐M���Ȃ�
//				return;															// �d�����M�����ɔ�����
//			}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
			Suica_Rec.Data.BIT.OPE_CTRL = 1;
			LagCan500ms(LAG500_SUICA_NONE_TIMER);								// ��t�֎~�����ް����M��������ϊJ�n(5s)
		}
		Ope_Suica_Status = 1;
		break;

	case S_SELECT_DATA:
	/* �I�����i�ް��쐬 */
		if ((OPECTL.op_faz == 8)								// �d�q�}�̒�~����
		||	(OPECTL.ChkPassSyu != 0)							// ����߽��������
		||	(Suica_Rec.Data.BIT.MIRYO_TIMEOUT != 0)) {			// ���[�_�����^�C���A�E�g
		// Ec��~�����̏ꍇ�́A�I�����i�f�[�^�𑗐M�����ɒ�~���������s����
			Suica_Rec.snd_kind = S_CNTL_DATA;
			memset(suica_work_buf, 0, sizeof(suica_work_buf));
			goto Suica_Data_Snd_10;
		}
		else if (!Suica_Rec.Data.BIT.ADJUSTOR_START) {
		// �I�����i�f�[�^���M�s���
			return;												// ���M�����ɔ�����
		}
		else if (OPECTL.InquiryFlg != 0 ||
				 Suica_Rec.Data.BIT.CTRL_MIRYO != 0) {
		// �J�[�h�������A�܂��́A�������͑I�����i�f�[�^�𑗐M���Ȃ�
			return;
		}
		else if (ECCTL.phase != EC_PHASE_PAY) {
		// ���Z�ȊO
			return;
		}
		else if (Suica_Rec.suica_err_event.BYTE != 0) {
		// ���σ��[�_�G���[������(E3201�`06,09,59,62�`64)
			return;
		}
		else if( Suica_Rec.Data.BIT.COMM_ERR ) {
			// ���σ��[�_�ʐM�ُ�
			return;
		}
// MH810103 GG119202(S) �ēx�A�I�����i�f�[�^�𑗐M���邱�Ƃ�����
		else if(Suica_Rec.Data.BIT.ADJUSTOR_NOW != 0) {
			// ����L�����Z���A�܂��́A3�f�[�^(��ԁA���Ϗ�ԁA���ό���)��M��
			// Suica_Rec.Data.BIT.ADJUSTOR_NOW�̓N���A�����
			return;
		}
// MH810103 GG119202(E) �ēx�A�I�����i�f�[�^�𑗐M���邱�Ƃ�����
// MH810103 GG119202(S) �d�q�}�l�[���ώ��̓W���[�i���v�����^�ڑ��ݒ���Q�Ƃ���
		else if(isEcBrandNoEMoney(RecvBrandResTbl.no, 0) &&
				Suica_Rec.Data.BIT.PRI_NG != 0) {
			// �d�q�}�l�[�u�����h�̏ꍇ�̓W���[�i���v�����^�g�p�s����
			// �I�����i�f�[�^�𑗐M���Ȃ�
			return;
		}
// MH810103 GG119202(E) �d�q�}�l�[���ώ��̓W���[�i���v�����^�ڑ��ݒ���Q�Ƃ���

		tmp = 10000;
		suica_work_buf[0] = 0x01;
		memcpy(&paydata, snddata, sizeof(paydata));
		// �������񃏁[�N�ɉ��i�ɃZ�b�g
		work_buf[2] = binbcd((uchar)(paydata / tmp));
		paydata %= tmp;
		tmp /= 100;
		work_buf[1] = binbcd((uchar)(paydata / tmp));
		paydata %= tmp;
		work_buf[0] = binbcd((uchar)paydata);

		p = &suica_work_buf[4];
		for(i = 0; i < TBL_CNT(RecvBrandTbl); i++) {
			for (j = 0; j < EC_BRAND_MAX; j++) {
				if(RecvBrandTbl[i].num <= j) {
					p += 3;
					continue;
				}
				// �u�����h��I�����Ă���ꍇ�͑I�������u�����h�̂݉��i���Z�b�g����
// MH810103 GG119202(S) �J�ǁE�L�������ύX
//				if ((RecvBrandTbl[i].ctrl[j].status & 0x01) == 0x01 && 			// �J�ǂ��Ă���u�����h��
//					((RecvBrandResTbl.res == EC_BRAND_SELECTED &&				// �u�����h�I���ς݁A���A�I�������u�����h
				// �u�����h�I�������Ƀu�����h���p�ۂ����Ă��邽�߁A�����ł͌��Ȃ�
				if ( ((RecvBrandResTbl.res == EC_BRAND_SELECTED &&				// �I�������u�����h
// MH810103 GG119202(E) �J�ǁE�L�������ύX
// MH810103 GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
//					 RecvBrandTbl[i].ctrl[j].no == RecvBrandResTbl.no) ||
//					 RecvBrandResTbl.res == EC_BRAND_UNSELECTED) ) {			// �܂��́A�u�����h��I�����Ă��Ȃ�
					 RecvBrandTbl[i].ctrl[j].no == RecvBrandResTbl.no)) ) {
// MH810103 GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
				// �L��
// MH810103 GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
//					if( (WAR_MONEY_CHECK && !e_incnt) ||
//						RecvBrandTbl[i].ctrl[j].no == BRANDNO_CREDIT) {
//						// ��������Ă��Ȃ��A�܂��́A�N���W�b�g
// MH810103 GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
						*p++ = work_buf[0];
						*p++ = work_buf[1];
						*p++ = work_buf[2];
						// ���M����u�����h�ԍ���ێ�
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//						Product_Select_Brand = RecvBrandTbl[i].ctrl[j].no;
						ECCTL.Product_Select_Brand = RecvBrandTbl[i].ctrl[j].no;
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
// MH810103 GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
//					} else {
//						// ��������Ă���ꍇ�͓d�q�}�l�[�͖���
//						p += 3;
//					}
// MH810103 GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
				} else {
				// ����
					p += 3;
				}
			}
		}
		// ���Z�@�̌��ݎ������Z�b�g
		time_set_snd(p, &CLK_REC);

		Product_Select_Data = *((long*)snddata);
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		Suica_Rec.Data.BIT.SELECT_SND = 1;					// �I�����i�f�[�^���M��
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		break;

	case S_INDIVIDUAL_DATA:
	/* ���ް��쐬 */
		suica_work_buf[0] = 0x00;							// ���[�J�R�[�h
		suica_work_buf[1] = 0x62;							// ���(98�F���Z�@���Z�b�g����)
		suica_work_buf[2] = 0x00;							// �\�t�g�o�[�W����
		time_set_snd(&suica_work_buf[3], snddata);			// ���ݎ���(���Z�@�̎��������[�_�ɃZ�b�g����)

		suica_work_buf[11] = 0xFE;							// ���i��I���^�C�}(254�b�Œ�)
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//		suica_work_buf[22] = OPE_EC_MIRYO_TIME;				// ���σ��g���C�^�C�}�l(255�b�Œ�)
		suica_work_buf[22] = 0;								// ���σ��g���C�^�C�}�l(0�Œ�)
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
	// ���u�����h����99�ɂ��邱�ƂőS�u�����h���̎�M���\�ƂȂ�
		suica_work_buf[23] = 0x99;							// �u�����h��
// MH810103 GG119202(S) �ʃf�[�^�t�H�[�}�b�g�ύX
		memcpy( &suica_work_buf[24], VERSNO.ver_part, 8 );	// �\�t�g�E�F�A�o�[�W����
// MH810103 GG119202(E) �ʃf�[�^�t�H�[�}�b�g�ύX
		break;

	case S_BRAND_SET_DATA:
	case S_BRAND_SET_DATA2:
	/* �u�����h�ݒ��ް��쐬 */
		brand_tbl = (EC_BRAND_TBL*)snddata;
		p = &suica_work_buf[0];
		*p++ = binbcd(ECCTL.brand_num);									// �u�����h��
		for (i = 0; i < brand_tbl->num; i++) {
			*p++ = binbcd((uchar)(brand_tbl->ctrl[i].no % 100));		// �u�����h�ԍ�
			*p++ = binbcd((uchar)(brand_tbl->ctrl[i].no / 100));
			*p++ = brand_tbl->ctrl[i].status;							// �u�����h�ݒ�
			memcpy(p, brand_tbl->ctrl[i].name, EC_BRAND_NAME_LEN);		// �u�����h����
			p += EC_BRAND_NAME_LEN;
		}
		break;

	case S_VOLUME_CHG_DATA:
	/* ���ʕύX�ް��쐬 */
		volume_tbl = (EC_VOLUME_TBL*)snddata;
		p = &suica_work_buf[0];
		if( volume_tbl->num > EC_VOL_MAX ) {
			volume_tbl->num = EC_VOL_MAX;
		}
		*p++ = binbcd(volume_tbl->num);									// �ؑփp�^�[����
		i = 0;
		do {
			// �؊��Ȃ�(�p�^�[����=0)�ł��A�p�^�[��1�ɂ̓f�[�^���Z�b�g����(���ʂ��Z�b�g���邽��)
			*p++ = binbcd((uchar)(volume_tbl->ctrl[i].time / 100));		// �ؑ֎���(��)
			*p++ = binbcd((uchar)(volume_tbl->ctrl[i].time % 100));		// �ؑ֎���(��)
			*p++ = binbcd(volume_tbl->ctrl[i].vol);						// ����
			i++;
		} while( i < volume_tbl->num );
		break;

	case S_READER_MNT_DATA:
	/* ���[�_�����e�i���X�ް��쐬 */
		mnt_tbl = (EC_MNT_TBL*)snddata;
		p = &suica_work_buf[0];
		*p++ = binbcd(mnt_tbl->cmd);									// �R�}���h
		*p++ = binbcd(mnt_tbl->mode);									// ���[�h
		*p++ = binbcd(mnt_tbl->vol);									// �e�X�g����
		break;

	case S_BRAND_SEL_DATA:
	/* �u�����h�I���ް��쐬 */
		if ((OPECTL.op_faz == 8)								// �d�q�}�̒�~����
		||	(OPECTL.ChkPassSyu != 0)							// ����߽��������
		||	(Suica_Rec.Data.BIT.MIRYO_TIMEOUT != 0)) {			// ���[�_�����^�C���A�E�g
		// Ec��~�����̏ꍇ�́A�I�����i�f�[�^�𑗐M�����ɒ�~���������s����
			Suica_Rec.snd_kind = S_CNTL_DATA;
			memset(suica_work_buf, 0, sizeof(suica_work_buf));
			goto Suica_Data_Snd_10;
		}
		else if (!Suica_Rec.Data.BIT.ADJUSTOR_START) {
		// �u�����h�I���ް����M�s���
			return;												// ���M�����ɔ�����
		}
		else if (OPECTL.InquiryFlg != 0 ||
				 Suica_Rec.Data.BIT.CTRL_MIRYO != 0) {
		// �J�[�h�������A�܂��́A�������̓u�����h�I���f�[�^�𑗐M���Ȃ�
			return;
		}
		else if (ECCTL.phase != EC_PHASE_PAY) {
		// ���Z�ȊO
			return;
		}
		else if (Suica_Rec.suica_err_event.BYTE != 0) {
		// ���σ��[�_�G���[������(E3201�`06,09,59,62�`64)
			return;
		}
		else if( Suica_Rec.Data.BIT.COMM_ERR ) {
			// ���σ��[�_�ʐM�ُ�
			return;
		}
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή��Q�l�j
		else if (Suica_Rec.Data.BIT.BRAND_SEL != 0) {
			// ��ԃf�[�^�i��t�s�j��M�A�܂��́A�u�����h�I�����ʉ����f�[�^���M��
			// Suica_Rec.Data.BIT.BRAND_SEL�̓N���A�����
			return;
		}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή��Q�l�j

// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		Suica_Rec.Data.BIT.BRAND_CAN = 0;
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810103 GG119202(S) �u�����h�I�𒆃t���O���N���A����Ȃ��΍�
//��������BRAND_SEL=1�Ƃ��邱�ƂŁA�I���f�[�^�����M�ł��A���̌�̎������ŁABRAND_CAN���Z�b�g�ł���悤�ɂ���
//�iBRAND_CAN��ON�ɂȂ��Ă��Ȃ��ƁA��t�s��BRAND_SEL���N���A�ł��Ȃ��܂܎��̐��Z���n�߂Ă��܂��j
		Suica_Rec.Data.BIT.BRAND_SEL = 1;			// �u�����h�I��
// MH810103 GG119202(E) �u�����h�I�𒆃t���O���N���A����Ȃ��΍�
		brandsel_tbl = (EC_BRAND_SEL_TBL*)snddata;
		p = &suica_work_buf[0];
		if( brandsel_tbl->num > EC_BRAND_TOTAL_MAX ) {
			brandsel_tbl->num = EC_BRAND_TOTAL_MAX;
		}
// MH810103 GG119202(S) �u�����h�I���f�[�^�t�H�[�}�b�g�ύX
		*p++ = binbcd(brandsel_tbl->btn_enable);						// �{�^���L���w��
		*p++ = binbcd(brandsel_tbl->btn_type);							// �{�^���^�C�v
// MH810103 GG119202(E) �u�����h�I���f�[�^�t�H�[�}�b�g�ύX
		*p++ = binbcd(brandsel_tbl->num);								// �u�����h��
		for (i = 0; i < brandsel_tbl->num; i++) {
			*p++ = binbcd((uchar)(brandsel_tbl->no[i] % 100));			// �u�����h�ԍ�
			*p++ = binbcd((uchar)(brandsel_tbl->no[i] / 100));
		}
		break;

// MH810103 GG119202(S) �u�����h�I�����ʉ����f�[�^
	case S_BRAND_RES_RSLT_DATA:
		brandresrslt_tbl = (EC_BRAND_RES_RSLT_TBL*)snddata;
		p = &suica_work_buf[0];
		*p++ = binbcd(brandresrslt_tbl->res_rslt);
		*p++ = binbcd((uchar)(brandresrslt_tbl->no % 100));			// �u�����h�ԍ�
		*p++ = binbcd((uchar)(brandresrslt_tbl->no / 100));
		break;
// MH810103 GG119202(E) �u�����h�I�����ʉ����f�[�^

	default:
		break;
	}

	Ec_Snd_Que_Regist(Suica_Rec.snd_kind, suica_work_buf);
	CNMTSK_START = 1;												/* �^�X�N�N�� */
}

//[]----------------------------------------------------------------------[]
///	@brief			���M�f�[�^�Z�b�g�̑O����
//[]----------------------------------------------------------------------[]
///	@param[in]		req		: �v���R�}���h
///	@param[in]		type	: ���M�f�[�^(1byte��)
///	@return			void
//[]----------------------------------------------------------------------[]
///	@note			Suica_Ctrl()���Q�l
///					�\���̖��Ȃǂ�Suica�̂܂�(�]����Suica�폜���ɕύX���邱��)
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	Ec_Pri_Data_Snd(ushort req, uchar type)
{
	uchar ctrl_work = type;
	uchar i, j;
// MH810103 GG119202(S) E3210�o�^�����C��
//	short timer;
// MH810103 GG119202(E) E3210�o�^�����C��
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	ushort no;
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j

	switch (req) {
	case S_CNTL_DATA:
		if (EC_STATUS_DATA.StatusInfo.RebootReq &&
			OPECTL.Ope_mod != 2 && OPECTL.Ope_mod != 3) {
			// �ċN���v��bitON�Ő��Z���A���Z�����ȊO�ł����
			// �ċN��OKbit��ON����
			ctrl_work |= S_CNTL_REBOOT_OK_BIT;
// MH810103 GG119202(S) E3210�o�^�����C��
			// �ċN�����M��͏������V�[�P���X�����̊Ď������Ȃ�
//// MH321800(S) E3210�o�^�^�C�~���O�ύX
////			// �N�������҂��^�C�}
////			timer = (short)prm_get(COM_PRM, S_ECR, 20, 3, 1);
////			if (timer == 0) {
////				timer = 300;
////			} else if(timer < 10){
////				timer = 10;
////			}
//			// �ċN���v������3��(�Œ�)�o�߂��Ă����σ��[�_�̏������V�[�P���X���������Ȃ��ꍇ�A
//			// E3210��o�^����
//			timer = OPE_EC_WAIT_BOOT_TIME;
//// MH321800(E) E3210�o�^�^�C�~���O�ύX
//			timer *= 2;
//			LagTim500ms(LAG500_EC_WAIT_BOOT_TIMER, (short)(timer+1), ec_wakeup_timeout);
// MH810103 GG119202(E) E3210�o�^�����C��
// MH810105 GG119202(S) ���σ��[�_�o�[�W�����A�b�v���̕��d�Ή�
			// ���σ��[�_�o�[�W�����A�b�v���t���O�Z�b�g
			EcReaderVerUpFlg = 1;
// MH810105 GG119202(E) ���σ��[�_�o�[�W�����A�b�v���̕��d�Ή�
// MH810103 GG119202(S) ���σ��[�_����̍ċN���v�����L�^����
			err_chk((char)jvma_setup.mdl, ERR_EC_REBOOT, 1, 0, 0);	// �G���[�o�^
// MH810103 GG119202(E) ���σ��[�_����̍ċN���v�����L�^����
		// ���u�[�g�����M��ACK���󂯂����_�ŁAINITIALIZE���N���A���ď�����
		}

// MH810103 GG119202(S) �}���`�u�����h�d�l�ύX�Ή�
//		if (type != 0x80) {
//			RecvBrandResTbl.res = EC_BRAND_UNSELECTED;		// ����f�[�^�𑗂�Ƃ��͈�U�A�I���u�����h�Ȃ��ɖ߂�
//			RecvBrandResTbl.no = BRANDNO_UNKNOWN;			// ��t��̓u�����h�I������蒼��
//// GG119202(S) �u�����h�I����̃L�����Z�������ύX
////			Suica_Rec.Data.BIT.SLCT_BRND_BACK = 0;			// �u�����h�I���́u�߂�v���o�t���O������
//// GG119202(E) �u�����h�I����̃L�����Z�������ύX
//		}
// MH810103 GG119202(E) �}���`�u�����h�d�l�ύX�Ή�
// MH810103 GG119202(S) �����d�l�ύX�Ή�
//		if (((ctrl_work & 0x01) == 0) &&
//			Suica_Rec.Data.BIT.CTRL_MIRYO != 0 &&
//			Suica_Rec.Data.BIT.MIRYO_CONFIRM != 0 &&
//			Suica_Rec.Data.BIT.MIRYO_NO_ANSWER == 0) {
//			// �����m���̎�t�֎~���M���͖����c���I���ʒmbit��ON����
//			ctrl_work |= S_CNTL_MIRYO_ZANDAKA_END_BIT;
//			// �����m���ɖ����c���I���ʒm�𑗐M����ہAdsp_change��2�ɂ���
//			dsp_change = 2;
		if (((ctrl_work & 0x01) == 0) && ((ctrl_work & 0x80) != 0x80 ) &&	// ��t�֎~�A����I���ȊO
			(Suica_Rec.Data.BIT.CTRL_MIRYO != 0) &&							// ������
			(Suica_Rec.Data.BIT.MIRYO_CONFIRM != 0) &&						// �����m��(��ԃf�[�^)��M�ς�
			(Suica_Rec.Data.BIT.MIRYO_NO_ANSWER == 0)) {					// �����^�C���A�E�g�O�ɂƂ肯���{�^������
			// �����m���̎�t�֎~���M���͖����c���I���ʒmbit��ON����
			ctrl_work |= S_CNTL_MIRYO_ZANDAKA_END_BIT;
// MH810103 GG119202(E) �����d�l�ύX�Ή�
		}
		if (type != 1) {
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
			if (Suica_Rec.Data.BIT.BRAND_SEL != 0 &&		// �u�����h�I��
				OPECTL.InquiryFlg == 0) {					// ���ɃJ�[�h�������ł͂Ȃ��Ƃ�
				Suica_Rec.Data.BIT.BRAND_CAN = 1;			// �L�����Z������
			}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//			timer_recept_send_busy = 0;
			ECCTL.timer_recept_send_busy = 0;
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
			Lagcan(OPETCBNO, TIMERNO_EC_RECEPT_SEND);		// ��t�����M�^�C�}�[��~
// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
			if( ECCTL.prohibit_snd_bsy != 0 ){				// ����f�[�^�i��t�֎~�j�����Ď���
				// �Ď����͐���f�[�^�i��t�֎~�j�𑗐M���Ȃ�
				if( ctrl_work & S_CNTL_TRANSACT_END_BIT ){	// ����I��bit ON�H
					// �����M���Ɏ���I���𑗐M����ׁA����I�����M�v�������ԂƂ���
					ECCTL.transact_end_snd = ctrl_work;
				}
				break;
			}
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
		}
// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
		// �e��Ԃ����������đ��M���s��
		ECCTL.prohibit_snd_bsy = 0;							// ����f�[�^�i��t�֎~�j�����Ď��Ȃ���ԂƂ���
		ECCTL.transact_end_snd = 0;							// ����I�����M�v���Ȃ���ԂƂ���
		LagCan500ms(LAG500_SUICA_NONE_TIMER);				// ��t�֎~�����ް����M���������ؾ��
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		// �u�����h�I�𒆂̋��z�ύX�͎�t�֎~�ɂ��đ��M����
		if (Suica_Rec.Data.BIT.ADJUSTOR_NOW == 0) {
			if (ctrl_work == S_CNTL_PRICE_CHANGE_BIT) {
				ctrl_work = 0;
			}
		}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		Ec_Data_Snd(S_CNTL_DATA, &ctrl_work);				// �����ް����M
		break;
	case S_SELECT_DATA:
		if (e_incnt <= 0) {
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//// GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
//			if( ENABLE_MULTISETTLE() == 1 ){				// �P�ꌈ�ϗL��
//				ec_set_brand_selected();					// �L���u�����h��I���ς݂Ƃ���
//			}
//// GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
			w_settlement = ryo_buf.zankin;
			Ec_Data_Snd(S_SELECT_DATA, &w_settlement);		// ���ԗ�����I�����i�ް��Ƃ��đ��M����
		}
		break;
	case S_VOLUME_CHG_DATA:
		Ec_Data_Snd(S_VOLUME_CHG_DATA, &VolumeTbl);			// ���ʕύX�ް����M
		break;
	case S_READER_MNT_DATA:
		Ec_Data_Snd(S_READER_MNT_DATA, &MntTbl);			// ���[�_�����e�i���X�ް����M
		break;
	case S_BRAND_SEL_DATA:
// MH810103 GG119202(S) �u�����h�I����̃L�����Z�������ύX
//		Suica_Rec.Data.BIT.SLCT_BRND_BACK = 0;				// �u�߂�v������
// MH810103 GG119202(E) �u�����h�I����̃L�����Z�������ύX
		memset(&BrandSelTbl, 0x00, sizeof(BrandSelTbl));
// MH810103 GG119202(S) �u�����h�I���f�[�^�t�H�[�}�b�g�ύX
		// ���σ��[�_�Ƀu�����h�I���{�^����\������
		BrandSelTbl.btn_enable = 1;							// �{�^���L
		// ���σ��[�_�Ɂu�͂��v�A�u�������v��2���{�^����\�����Ȃ�
		BrandSelTbl.btn_type = 0;							// 2��\���Ȃ�
// MH810103 GG119202(E) �u�����h�I���f�[�^�t�H�[�}�b�g�ύX
		for (i = 0; i < TBL_CNT(RecvBrandTbl); i++) {
			for (j = 0; j < EC_BRAND_MAX; j++) {
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//// GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
////				if ((RecvBrandTbl[i].ctrl[j].status & 0x01) == 0x01) {
//				if( ec_check_valid_brand( RecvBrandTbl[i].ctrl[j].no ) ){	// �u�����h�L���H
//// GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
				if( IsValidBrand( i, j ) ){					// �u�����h�L���H
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
					BrandSelTbl.no[BrandSelTbl.num++] = RecvBrandTbl[i].ctrl[j].no;
				}
			}
		}
		Ec_Data_Snd(S_BRAND_SEL_DATA, &BrandSelTbl);		// �u�����h�I���ް����M
		break;
// MH810103 GG119202(S) �u�����h�I�����ʉ����f�[�^
	case S_BRAND_RES_RSLT_DATA:
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	case S_BRAND_RES_RSLT_DATA_with_HOLD:
		// �u�����h�I�����ʉ�����NG�𑗐M����ꍇ�́A�\�� BrandResRsltTbl.no�Ɂu���R�R�[�h�v���Z�b�g����
		// type=0�ŌĂяo������
		no = BrandResRsltTbl.no;
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		memset(&BrandResRsltTbl, 0x00, sizeof(BrandResRsltTbl));
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//		BrandResRsltTbl.res_rslt = RecvBrandResTbl.res;
//		BrandResRsltTbl.no = RecvBrandResTbl.no;
		// type : �u�����h�I������ 1:OK, 0:NG
		BrandResRsltTbl.res_rslt = type;
		if(type == 0) {	// NG
			BrandResRsltTbl.no = no;
			if (no == EC_BRAND_REASON_99) {					// �u�����h�I���f�[�^�̏������ł��Ȃ��B
				ECCTL.brandsel_stop = 1;					// �u�����h�I��������~
			}
		} else {
			BrandResRsltTbl.no = RecvBrandResTbl.no;
		}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		Ec_Data_Snd(S_BRAND_RES_RSLT_DATA, &BrandResRsltTbl);		// �u�����h�I�����ʉ����ް����M
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		Suica_Rec.Data.BIT.BRAND_SEL = 0;					// �u�����h�I�𒆂�����
		if (req == S_BRAND_RES_RSLT_DATA) {
			EcBrandClear();									// �u�����h�I�����ʃN���A
		}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		break;
// MH810103 GG119202(E) �u�����h�I�����ʉ����f�[�^
	default:
		break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_Ec_Event
//[]----------------------------------------------------------------------[]
///	@param[in]		msg			:getmassege�ɂĎ擾����ID 
///					ope_faze	:��ٌ���̪��
///	@return			ret			:���Z�I������
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
short Ope_Ec_Event(ushort msg, uchar ope_faze)
{
	short	ret = 0;
	uchar	i;

	for ( ; suica_rcv_que.count != 0; ) {							// ��M�ς݂̃f�[�^��S�ď�������܂ŉ�
		if (Ec_Read_RcvQue() == 1) {								// ��M�҂����킹���̏ꍇ��
			continue;												// �㑱�̃f�[�^��M������ׁA��͏����͂����Ȃ�
		}

		for (i = 0; i < TBL_CNT(EcTbl); i++) {						// ��M�f�[�^�e�[�u���������s
			if (EcTbl[i].Command == (Suica_Rec.suica_rcv_event.BYTE & EcTbl[i].Command)) {	// ��M�����f�[�^�������\�ȓd���̏ꍇ
				if ((ret = EcTbl[i].Func(&ope_faze)) != 0) {		// �d�����̉�͏��������s
					break;
				}
			}
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			��M�L���[����d����Read����
//[]----------------------------------------------------------------------[]
///	@return			ret 0:�ް���ʖ��̉�͏������{ 1:��M�ް��҂����킹
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static
uchar	Ec_Read_RcvQue(void)
{
	uchar	*pData = (uchar*)&suica_rcv_que.rcvdata[suica_rcv_que.readpt];	// ��M������߲����ܰ��̈�ݒ�
	uchar	i, bc;
	uchar	ret = 1;
	ushort	cnt;
	ushort	w_counter[3];

	memcpy(w_counter, &suica_rcv_que.readpt, sizeof(w_counter));			// �߲�������̈�̏�����

	for (i = 0; (i < 6 && *pData != ack_tbl[i]); i++) {						// ��M���(ACK1�`NACK)����
		;
	}
	if (i == 1) {															// ACK2 �f�[�^��M ?
		pData += 2;															// �ް��߲���̍X�V
		ec_data_save(pData, (short)*(pData-1)); 							// ��M�ް��̉�͏���
		if (ec_split_data_check() == 0) {									// �ް��҂����킹���菈��
			ret = 0;														// ��ʖ��̉�͏������{
		}
	} else if ((i == 2) || (i == 0)) {										// ACK3 or ACK1 �f�[�^��M?
		bc = *(pData+1);													// ��M�ް�����ێ�
		pData += 2;															// �ް��߲���̍X�V
		for (cnt = 0; cnt < bc; ) {											// �S�Ă̎�M�ް��̉�͂���������܂�ٰ��
			cnt += (*pData + 1);											// �ް���ʖ����ް����ނ����Z
			pData++;														// �ް��߲���̍X�V
			ec_data_save(pData,(short)*(pData-1)); 							// ��M�ް��̉�͏���
			pData += *(pData-1);											// �ް��߲���̍X�V(�����ް��܂Ői�߂�)
		}
		if (ec_split_data_check() == 0) {									// �ް��҂����킹���菈��
			ret = 0;														// ��ʖ��̉�͏������{
		}
	}

	if (++w_counter[0] >= SUICA_QUE_MAX_COUNT) {							// ذ���߲����MAX�ɓ��B
		w_counter[0] = 0;													// ذ���߲����擪�Ɉړ�
	}
	if (w_counter[2] != 0) {												// ��M�ް����Đ����O�łȂ��ꍇ
		w_counter[2]--;														// �޸���Ď��{
	}

	nmisave(&suica_rcv_que.readpt, w_counter, sizeof(w_counter));			// �߲���̍X�V���d�ۏ�ōs��
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			��M�ް����ނ��Aү���޷����o�^����
//[]----------------------------------------------------------------------[]
///	@param[in]		*buf 	�F��M�L���[������o�����f�[�^�i�[�̈�
///					siz		�F��M�f�[�^�T�C�Y
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static
void	ec_data_save(unsigned char *buf, short siz)
{
	int				i;
	short			brandno;
	unsigned long	work, work_brand;
	unsigned char	*p;
	unsigned char	*pt;
// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�(���g�p�ϐ��폜)
//	unsigned char	status_timer;
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�(���g�p�ϐ��폜)
	unsigned char	pay_ng;
	unsigned char	brand_index;
	unsigned char	kind;
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
//	T_FrmReceipt	DeemedData;		// ���ϐ��Z���~(������)�󎚗v���p�f�[�^
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
// MH810103 GG119202(S) �d�q�}�l�[���ώ��̌��ό��ʎ�M�҂��^�C�}�ǉ�
//	unsigned short	ec_inquiry_wait_time = (ushort)prm_get(COM_PRM, S_ECR, 12, 3, 1);
//
//	if(ec_inquiry_wait_time == 0) {
//		ec_inquiry_wait_time = 360;
//	} else if(ec_inquiry_wait_time < 240){
//		ec_inquiry_wait_time = 240;
//	}
//	ec_inquiry_wait_time *= 50;
	unsigned short	ec_inquiry_wait_time = 0;
// MH810103 GG119202(E) �d�q�}�l�[���ώ��̌��ό��ʎ�M�҂��^�C�}�ǉ�

	pt = buf + 1;	/* �ް����̐擪���ڽ(��ʂ̎�) */
	memset(wrcvbuf,0,sizeof(wrcvbuf));
	memcpy(wrcvbuf, pt, (size_t)siz);
// GG116202(S) JVMA���Z�b�g�񐔐����Ή�
	if (!hasEC_JVMA_RESET_CNT_EXCEEDED) {							// JVMA���Z�b�g���g���C�񐔈ȉ�
// GG116202(E) JVMA���Z�b�g�񐔐����Ή�
// MH321800(S) �؂藣���G���[������̑ҋ@��Ԃ�JVMA���Z�b�g���s��
	// �ʐM�s�ǃt���O���N���A����
	Suica_Rec.suica_err_event.BIT.COMFAIL = 0;
// MH321800(E) �؂藣���G���[������̑ҋ@��Ԃ�JVMA���Z�b�g���s��
// GG116202(S) JVMA���Z�b�g�񐔐����Ή�
	}
// GG116202(E) JVMA���Z�b�g�񐔐����Ή�

	switch (*buf) {													/* ��M�ް����� */
	case S_ERR_DATA: /* �ُ�f�[�^ */
		err_ec_chk(wrcvbuf, &err_data);								/* �װ���� */

		err_data = wrcvbuf[0];
// MH810103 GG119202(S) �s�v�����폜
//		work = EC_STATUS_TIME;										/* CRW-MJA��ԊĎ���ϒl�擾 */
//		if (err_data != 0) {										/* �ُ픭���H */
//			Suica_Rec.suica_err_event.BIT.ERR_RECEIVE = 1;			/* �ُ��ް���M�׸�ON */
//
//			if (work) {												/* CRW-MJA��ԊĎ���ϒl������l�H */
//				LagTim500ms(LAG500_SUICA_STATUS_TIMER, (short)(120*work), status_err_timeout);	// CRW-MJA��ԊĎ���ϊJ�n
//			}
//		} else {													/* �װ���� */
//			LagCan500ms(LAG500_SUICA_STATUS_TIMER);					/* CRW-MJA��ԊĎ����ؾ�� */
//		}
		if (err_data != 0) {										/* �ُ픭���H */
			Suica_Rec.suica_err_event.BIT.ERR_RECEIVE = 1;			/* �ُ��ް���M�׸�ON */
// MH810103 GG119202(S) JVMA���Z�b�g�����s�
			jvma_trb(ERR_SUICA_RECEIVE);
// MH810103 GG119202(E) JVMA���Z�b�g�����s�
		}
// MH810103 GG119202(E) �s�v�����폜
// MH810103 GG119202(S) �ُ�f�[�^�t�H�[�}�b�g�ύX
		// 2�o�C�g�ڂ̃r�b�g��JVMA���Z�b�g�ΏۊO�Ƃ���
		err_ec_chk2(&wrcvbuf[1], &err_data2);						// �G���[�`�F�b�N
		err_data2 = wrcvbuf[1];
		if (err_data2 != 0) {
			// ERR_RECEIVE2�t���O�͌��σ��[�_���p�s�ƂȂ邪�AJVMA���Z�b�g�͂��Ȃ�
			Suica_Rec.suica_err_event.BIT.ERR_RECEIVE2 = 1;
		}
// MH810103 GG119202(E) �ُ�f�[�^�t�H�[�}�b�g�ύX
		break;

	case S_FIX_DATA: /* �Œ�f�[�^ */
		memcpy(fix_data,wrcvbuf, sizeof(fix_data));					/* ��M�ް�����ʕ\���p�ɕێ� */
		break;

	case S_BRAND_STS_DATA:	/* �u�����h��ԃf�[�^1	*/
	case S_BRAND_STS_DATA2:	/* �u�����h��ԃf�[�^2	*/
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
//		if(OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 3) {
//			// ���Z���̃u�����h��ԃf�[�^�͖�������
//			break;
//		}
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
// MH810103 GG119202(S) �N���V�[�P���X��������������
//		if ( Suica_Rec.Data.BIT.BRAND_STS_RCV == 0 ) {				/* �N����A��x�������ޏ���ް���M���ĂȂ� �H*/
//			Suica_Rec.Data.BIT.BRAND_STS_RCV = 1;					/* �����ޏ���ް���M�׸�ON */
//		}
// MH810103 GG119202(E) �N���V�[�P���X��������������
		brand_index = 0;
		kind = S_BRAND_SET_DATA;
		if(*buf == S_BRAND_STS_DATA2) {
			brand_index = 1;
			kind = S_BRAND_SET_DATA2;
		}
// MH810103 GG119202(S) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
//		memset(&RecvBrandTbl[brand_index], 0, sizeof(EC_BRAND_TBL));		// ��M�e�[�u���N���A
//		p = &wrcvbuf[0];
//		RecvBrandTbl[brand_index].num = bcdbin(*p++);						// �u�����h����
//		ECCTL.brand_num = RecvBrandTbl[brand_index].num;					// �u�����h������ێ�
//		if( RecvBrandTbl[brand_index].num > EC_BRAND_MAX ) {
//			if(*buf == S_BRAND_STS_DATA) {
//				RecvBrandTbl[brand_index].num = EC_BRAND_MAX;				// �u�����h��ԃf�[�^�P�̃u�����h��
//			} else {
//				RecvBrandTbl[brand_index].num -= EC_BRAND_MAX;				// �u�����h��ԃf�[�^�Q�̃u�����h��
//			}
//		}
//		for (i = 0; i < RecvBrandTbl[brand_index].num; i++) {
//			RecvBrandTbl[brand_index].ctrl[i].no		= bcdbin3(p);		// �u�����h�ԍ�
//			p += 2;
//			RecvBrandTbl[brand_index].ctrl[i].status	= *p++;				// �u�����h���
//			memcpy(RecvBrandTbl[brand_index].ctrl[i].name, p, 20);			// �u�����h����
//			p += 20;
//		}
		memset(&RecvBrandTblTmp, 0, sizeof(RecvBrandTblTmp));				// ��M�e�[�u���N���A
		p = &wrcvbuf[0];
		RecvBrandTblTmp[brand_index].num = bcdbin(*p++);					// �u�����h����
		ECCTL.brand_num = RecvBrandTblTmp[brand_index].num;					// �u�����h������ێ�
		if( RecvBrandTblTmp[brand_index].num > EC_BRAND_MAX ) {
			if(*buf == S_BRAND_STS_DATA) {
				RecvBrandTblTmp[brand_index].num = EC_BRAND_MAX;			// �u�����h��ԃf�[�^�P�̃u�����h��
			} else {
				RecvBrandTblTmp[brand_index].num -= EC_BRAND_MAX;			// �u�����h��ԃf�[�^�Q�̃u�����h��
			}
		}
		for (i = 0; i < RecvBrandTblTmp[brand_index].num; i++) {
			RecvBrandTblTmp[brand_index].ctrl[i].no		= bcdbin3(p);		// �u�����h�ԍ�
			p += 2;
			RecvBrandTblTmp[brand_index].ctrl[i].status	= *p++;				// �u�����h���
			memcpy(RecvBrandTblTmp[brand_index].ctrl[i].name, p, 20);		// �u�����h����
			p += 20;
		}
// MH810103 GG119202(E) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
		queset(OPETCBNO, EC_BRAND_UPDATE, 0, NULL);							// �u�����h��ԍX�V��ʒm

// MH810103 GG119202(S) �u�����h��Ԕ��菈���ύX
//		if (ECCTL.phase == EC_PHASE_PAY) {
//		// ���Z��
//			ECCTL.phase = EC_PHASE_BRAND;									// �u�����h���킹��
//			ECCTL.step = 0;
//		}
//		// else {}		// ���̑��̃t�F�[�Y�ł̓t�F�[�Y��ς����Ƀu�����h�ݒ�f�[�^�݂̂𑗐M����
// MH810103 GG119202(E) �u�����h��Ԕ��菈���ύX

		// �u�����h����e�[�u���X�V
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
//// GG119202(S) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
////		set_brand_tbl(&BrandTbl[brand_index], &RecvBrandTbl[brand_index]);
//		set_brand_tbl(&BrandTbl[brand_index], &RecvBrandTblTmp[brand_index]);
//		update_brand_tbl(&RecvBrandTbl[brand_index], &RecvBrandTblTmp[brand_index]);
//// GG119202(E) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
//		Ec_Data_Snd(kind, &BrandTbl[brand_index]);							// �u�����h�ݒ�f�[�^���M
		// �u�����h���킹���ȊO�Ɏ�M�����ꍇ�̓e�[�u���X�V���Ȃ�
		if (ECCTL.phase == EC_PHASE_BRAND) {
			set_brand_tbl(&BrandTbl[brand_index], &RecvBrandTblTmp[brand_index]);
			update_brand_tbl(&RecvBrandTbl[brand_index], &RecvBrandTblTmp[brand_index]);
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
// MH810103 GG119202(S) �u�����h�e�[�u���������Ȃ��s�
			if (brand_index == 0 && ECCTL.brand_num <= EC_BRAND_MAX) {
				// �u�����h����10�ȉ��̏ꍇ�A
				// �u�����h��ԃf�[�^2�͒ʒm����Ȃ��̂ł��̃^�C�~���O�ŃN���A����
				nmicler(&RecvBrandTbl[1], sizeof(RecvBrandTbl[1]));
			}
// MH810103 GG119202(E) �u�����h�e�[�u���������Ȃ��s�
// MH810103 GG119202(S) �N���V�[�P���X��������������
			setEcBrandStsDataRecv( kind );									// �u�����h��ԃf�[�^��M������ݒ�
// MH810103 GG119202(E) �N���V�[�P���X��������������
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
			if (OPECTL.Ope_mod != 2 && OPECTL.Ope_mod != 3) {
				// ���Z���ȊO�̏ꍇ�A�u�����h�ݒ�f�[�^�𑗐M����
				// ���Z���̏ꍇ�͑ҋ@��Ԃɖ߂����^�C�~���O�ő��M����
				Ec_Data_Snd(kind, &BrandTbl[brand_index]);					// �u�����h�ݒ�f�[�^���M
				if (isEcBrandStsDataRecv()) {
					// �u�����h�l�S�V�G�[�V��������
					if (ECCTL.phase == EC_PHASE_BRAND) {
						Lagcan(OPETCBNO, TIMERNO_EC_BRAND_NEGO_WAIT);
						// �u�����h���킹������͉��ʕύX�t�F�[�Y�Ɉڍs����
						ECCTL.phase = EC_PHASE_VOL_CHG;						// ���ʕύX��
						ECCTL.step = 0;

						// ���ʕύX�f�[�^���M
						SetEcVolume();
						Ec_Data_Snd( S_VOLUME_CHG_DATA, &VolumeTbl );
						Lagtim(OPETCBNO, TIMERNO_EC_BRAND_NEGO_WAIT, EC_BRAND_NEGO_WAIT_TIME);	// ��M�҂��^�C�}�J�n
					}
				}
				else {
					// ���̃u�����h��ԃf�[�^��M�҂�
					Lagtim(OPETCBNO, TIMERNO_EC_BRAND_NEGO_WAIT, EC_BRAND_NEGO_WAIT_TIME);		// ��M�҂��^�C�}�J�n
				}
			}
		}
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
		break;

	case S_TIME_DATA: /* ���ԓ����f�[�^ */
		if (!Suica_Rec.Data.BIT.INITIALIZE) {						/* CRW-MJA�������I���H */
			break;
		}
		// �����σ��[�_�ł͐��������Ȃ�
		break;

	case S_STATUS_DATA: /* ��ԃf�[�^ */
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//		if (timer_recept_send_busy == 2) {
//			timer_recept_send_busy = 0;										// �Ď��I��
		if (ECCTL.timer_recept_send_busy == 2) {
			ECCTL.timer_recept_send_busy = 0;								// �Ď��I��
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
			Lagcan( OPETCBNO, TIMERNO_EC_RECEPT_SEND );
		}
		PRE_EC_STATUS_DATA.status_data = EC_STATUS_DATA.status_data;
		memcpy(&EC_STATUS_DATA, wrcvbuf, sizeof(EC_STATUS_DATA));	/* ��M�ް��ð����� */

// MH810105 GG119202(S) ���[�_���璼�撆�̏�ԃf�[�^��M�����ۂ̐V�K�A���[����݂���
		if( EC_STATUS_DATA.StatusInfo.LastSetteRunning &&					// �����M�f�[�^�̒��撆bit ON
		    !PRE_EC_STATUS_DATA.StatusInfo.LastSetteRunning ){				// �O���M�f�[�^�̒��撆bit OFF
			// �u���σ��[�_���p�s�v�o�^
			alm_chk(ALMMDL_SUB, ALARM_EC_LASTSETTERUN, 1);					// �װ�۸ޓo�^�i�o�^�j
		}
		else if( !EC_STATUS_DATA.StatusInfo.LastSetteRunning &&				// �����M�f�[�^�̒��撆bit OFF
		    PRE_EC_STATUS_DATA.StatusInfo.LastSetteRunning ){				// �O���M�f�[�^�̒��撆bit ON
			if( ac_flg.ec_recv_deemed_fg == 0 ){							// ���O����f�[�^�̏����҂��ł͂Ȃ�
				// �u���σ��[�_���p�s�v����
				alm_chk(ALMMDL_SUB, ALARM_EC_LASTSETTERUN, 0);				// �װ�۸ޓo�^�i�����j
			}
		}
// MH810105 GG119202(E) ���[�_���璼�撆�̏�ԃf�[�^��M�����ۂ̐V�K�A���[����݂���

		if ((PRE_EC_STATUS_DATA.status_data ^ EC_STATUS_DATA.status_data) == 0x2000) {	// ���i��I���^�C�}�[�X�V�˗�bit�̂ݕω�
			// ���i��I���^�C�}�X�V�˗�bit�̂ݕω������ꍇ�͖�������
			break;
		}
		if( !Suica_Rec.Data.BIT.MIRYO_CONFIRM && EC_STATUS_DATA.StatusInfo.MiryoZandakaStop != 0 ){
			// �����m��O�ɖ����c�����~����M�����ꍇ�͖�������
			break;
		}
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
		if (EC_STATUS_DATA.StatusInfo.CardProcTimerUpdate != 0) {
			// �^�C�}�X�V�˗�����
			EcTimerUpdate();
		}
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�

		Suica_Rec.Data.BIT.PRE_CTRL = Suica_Rec.Data.BIT.CTRL;		// �O���M�ް���ذ�ް��Ԃ�ێ����Ă���
		Suica_Rec.Data.BIT.CTRL = EC_STATUS_DATA.StatusInfo.ReceptStatus ^ 0x01;	/* ��M�ް���1Byte�ڂ��Q�Ƃ��Aذ�ް��ԂƂ��Ď擾���� */

		if(EC_STATUS_DATA.StatusInfo.DetectionCard &&				/* �J�[�h����Bit ON */
			!Suica_Rec.Data.BIT.CTRL_CARD) {						/* �J�[�h�������OFF */
			Suica_Rec.Data.BIT.CTRL_CARD = 1;						/* �J�[�h������Ԃ�ON */
		} else if(!EC_STATUS_DATA.StatusInfo.DetectionCard &&		/* �J�[�h����Bit OFF */
			Suica_Rec.Data.BIT.CTRL_CARD) {							/* �J�[�h�������ON */
			Suica_Rec.Data.BIT.CTRL_CARD = 0;						/* �J�[�h������Ԃ�OFF */
			queset(OPETCBNO, EC_EVT_CARD_STS_UPDATE, 0, NULL);		// �J�[�h��������ʒm
		}

		if (EC_STATUS_DATA.StatusInfo.MiryoStatus &&				/* ����Bit���`�F�b�N */
			!Suica_Rec.Data.BIT.CTRL_MIRYO)	{						/* �܂��������������Ă��Ȃ� */

			if(OPECTL.Mnt_mod != 0 || OPECTL.Ope_mod != 2) {
				// �����e�i���X���A���Z���ȊO�ɏ�����������M
				recv_unexpected_status_data();
				break;
// MH810103 GG119202(S) �}���`�u�����h�d�l�ύX�Ή�
//// GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
////// GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
//////			} else if( (ENABLE_MULTISETTLE() == 0 &&				// �N���W�b�g���ς̂ݗL��
////			} else if( (ENABLE_MULTISETTLE() == 1 &&				// �N���W�b�g���ς̂ݗL��
////// GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
////						isEcEnabled(EC_CHECK_CREDIT)) ||			// �܂���
//			} else if( ENABLE_MULTISETTLE() == 1 &&					// �P�ꌈ�ς�
//// GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//						RecvBrandResTbl.no == BRANDNO_CREDIT) {		// �N���W�b�g���ς�I���ς�
			} else if( !isEcBrandNoEMoney(RecvBrandResTbl.no, 0) ){	// �d�q�}�l�[�u�����h�ȊO���I���ς�
// MH810103 GG119202(E) �}���`�u�����h�d�l�ύX�Ή�
				// �N���W�b�g���ςŏ�����������M
				recv_unexpected_status_data();
				break;
			}
// MH810103 GG119202(S) �����������̉�ʕ\���s�
// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
			if (Suica_Rec.Data.BIT.MIRYO_TIMEOUT != 0) {
				// �����^�C���A�E�g��̖�������M�͖�������
				break;
			}
// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
			ec_MessageAnaOnOff(0, 0);
// MH810103 GG119202(E) �����������̉�ʕ\���s�
			Suica_Rec.Data.BIT.CTRL_MIRYO = 1;						/* �����ð����� */
// MH810103 GG119202(S) �����d�l�ύX�Ή�
			Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;					// �����m��(��ԃf�[�^)��M���N���A
// MH810103 GG119202(E) �����d�l�ύX�Ή�
// MH810103 GG119202(S) �������Ɉُ�f�[�^��M�łƂ肯���{�^�������҂��ɂȂ�Ȃ�
			OPECTL.InquiryFlg = 0;									// �O���Ɖ�t���OOFF
// MH810103 GG119202(E) �������Ɉُ�f�[�^��M�łƂ肯���{�^�������҂��ɂȂ�Ȃ�
// MH810104 GG119201(S) �����؂������̐��Z�ŁA���C���[�_�ɒ�������������҂��̏�Ԃł�������������Ǝ��C���[�_�̃V���b�^�[�����Ă��܂�
			if ((RD_mod < 10) || (RD_mod > 13)) {					// �������҂�or�ړ����ȊO
// MH810104 GG119201(E) �����؂������̐��Z�ŁA���C���[�_�ɒ�������������҂��̏�Ԃł�������������Ǝ��C���[�_�̃V���b�^�[�����Ă��܂�
			read_sht_cls();											/* ���Cذ�ް������۰�� */
// MH810104 GG119201(S) �����؂������̐��Z�ŁA���C���[�_�ɒ�������������҂��̏�Ԃł�������������Ǝ��C���[�_�̃V���b�^�[�����Ă��܂�
			}
// MH810104 GG119201(E) �����؂������̐��Z�ŁA���C���[�_�ɒ�������������҂��̏�Ԃł�������������Ǝ��C���[�_�̃V���b�^�[�����Ă��܂�
			cn_stat( 2, 2 );										/* �����s�� */
			if( MIFARE_CARD_DoesUse ){								// Mifare���L���ȏꍇ
				op_MifareStop_with_LED();							// Mifare����
			}
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//			work = OPE_EC_MIRYO_TIME;								/* ������ײ��ϰ�l�擾 */
//			LagTim500ms(LAG500_SUICA_MIRYO_RESET_TIMER, (short)(2*work+1), ec_miryo_timeout);	// ������ԊĎ���ϊJ�n
			EcTimerUpdate();										// �����Ď��^�C�}�J�n
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
			wmonlg(OPMON_EC_MIRYO_START, NULL, 0);					// ���j�^�o�^
		} else {
			switch ((EC_STATUS_DATA.status_data >> 8) & 0x0003) {	/* ��2Bit�̏�Ԃ��`�F�b�N */
			case 0:		/* ��t�� */
				if (!Ope_Suica_Status) {							/* ���Z�@������̑��M�v���Ȃ� */
					Suica_Rec.Data.BIT.OPE_CTRL = 1;				/* �Ō�ɐ��Z�@������̑��M�v���͎�t���Ƃ��� */
				}
				if (Suica_Rec.Data.BIT.OPE_CTRL) {					/* �Ō�ɐ��Z�@������̑��M�v���͎�t���H */
					LagCan500ms( LAG500_SUICA_NO_RESPONSE_TIMER );	/* ��t�������ް����M���������ؾ�� */
					LagCan500ms(LAG500_EC_NOT_READY_TIMER);			/* ��t�������ް����M���������ؾ�� */
					ECCTL.not_ready_timer = 0;
					if (ECCTL.pon_err_alm_regist == 0) {
					// �N����ŏ��̎�t��M
						ECCTL.pon_err_alm_regist = 1;				// �ȍ~�A�A���[���^�G���[�o�^����
					}
					err_chk((char)jvma_setup.mdl, ERR_EC_OPEN, 0, 0, 0); 	/* �װ۸ޓo�^�i�����j*/
					alm_chk(ALMMDL_SUB, ALARM_EC_OPEN, 0);			/* �װ�۸ޓo�^�i�����j*/
					Suica_Rec.suica_err_event.BIT.OPEN = 0;			/* �J�ǈُ��׸ނ����� */
					Status_Retry_Count_OK = 0;						/* ��ײ���Đ��������� */
// MH321800(S) �؂藣���G���[������̑ҋ@��Ԃ�JVMA���Z�b�g���s��
					err_ec_clear();
// MH810103 GG116201(E) �؂藣���G���[������̑ҋ@��Ԃ�JVMA���Z�b�g���s��
				}
// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
//				else {												/* �Ō�ɐ��Z�@������̑��M�v���͎�t�֎~�H */
//					status_timer = EC_DISABLE_WAIT_TIME;
//					LagTim500ms(LAG500_SUICA_NONE_TIMER, (short)((status_timer * 2) + 1), snd_disable_timeout);	// ��t�֎~�����ް����M��������ϊJ�n(5s)
//				}
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
				break;
			case 0x01:	/* ��t�s�� */
				if (!Ope_Suica_Status) {							/* ���Z�@������̑��M�v���Ȃ� */
					Suica_Rec.Data.BIT.OPE_CTRL = 0; 				/* �Ō�ɐ��Z�@������̑��M�v���͎�t�֎~�Ƃ��� */
				}
				if (!Suica_Rec.Data.BIT.INITIALIZE) {				/* �������I���H */
					break;
				}
				if (!Suica_Rec.Data.BIT.OPE_CTRL) {					/* �Ō�ɐ��Z�@������̑��M�v���͎�t�֎~�H */
					LagCan500ms(LAG500_SUICA_NONE_TIMER);			/* ��t�֎~�����ް����M���������ؾ�� */
					if (Suica_Rec.Data.BIT.CTRL_MIRYO != 0) {		/* ������Ԃ����t�s����M�H */ 
						if (Suica_Rec.Data.BIT.MIRYO_NO_ANSWER != 0) {		/* �����^�C���A�E�g��(�������I���̓`�F�b�N�ς݂Ȃ̂ŏȗ�����)	*/
						// �����^�C���A�E�g
						// ��snd_no_response_timeout()�̏������e�ɍ��킹��
							LagCan500ms(LAG500_SUICA_MIRYO_RESET_TIMER);	/* ������ԊĎ����ؾ�� */
							Suica_Rec.Data.BIT.CTRL_MIRYO = 0;				/* ������ԉ��� */
// MH810103 GG119202(S) �����d�l�ύX�Ή�
//							if (Suica_Rec.Data.BIT.MIRYO_CONFIRM == 0) {
//								// �����m���M�Ȃ��̏ꍇ�̓J�[�hID��'?'�Ŗ��߂�
//								Ec_Settlement_Res.brand_no = EcDeemedLog.EcDeemedBrandNo;	// �u�����h�ԍ�
//								Ec_Settlement_Res.settlement_data = Product_Select_Data;	// ���ϊz
//								memset(Ec_Settlement_Res.Card_ID, '\?', sizeof(Ec_Settlement_Res.Card_ID));
//							}
//							else {
//								Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;		// �����m���M�t���O�N���A
//							}
							Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;			// �����m��(��ԃf�[�^)��M���N���A
// MH810103 GG119202(E) �����d�l�ύX�Ή�
							if (Suica_Rec.Data.BIT.ADJUSTOR_NOW != 0) {		/* ���ϒ��̎���L�����Z�������������ꍇ */
								EC_STATUS_DATA.StatusInfo.TradeCansel = 1;	/* �����ݾ��׸ނ�ON */
							}
						}
					}
					Status_Retry_Count_NG = 0;						/* ��ײ���Đ��������� */
				}
				break;
			case 0x02:	/* ����L�����Z����t */
			case 0x03:	/* ��t�s������L�����Z����t */
				LagCan500ms(LAG500_SUICA_NONE_TIMER);				/* ��t�֎~�����ް����M���������ؾ�� */
// MH810105 GG119202(S) �����m��̒���f�[�^��M����R0176��o�^����
				// �����c���Ɖ�łƂ肯���{�^�����������ꍇ�ɐ���f�[�^�i��t�֎~�j�ɑ΂���
				// ��ԃf�[�^�i��t�s������L�����Z���j����������邱�Ƃ͂Ȃ��̂�
				// ���L�������폜����
//				if (Suica_Rec.Data.BIT.CTRL_MIRYO) {				/* ������ԁH */
//					Suica_Rec.Data.BIT.CTRL_MIRYO = 0;				/* ������ԉ��� */
//					LagCan500ms(LAG500_SUICA_MIRYO_RESET_TIMER);	/* ������ײ���ؾ�� */
//// MH810103 GG119202(S) �����d�l�ύX�Ή�
//					Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;			// �����m��(��ԃf�[�^)��M���N���A
//					if (Suica_Rec.Data.BIT.MIRYO_NO_ANSWER != 0) {	// �����^�C���A�E�g��(�������I���̓`�F�b�N�ς݂Ȃ̂ŏȗ�����)
//						Suica_Rec.Data.BIT.MIRYO_NO_ANSWER = 0;		// �����^�C���A�E�g�������{�ς݃N���A
//					}
//// MH810103 GG119202(E) �����d�l�ύX�Ή�
//					if (!SUICA_CM_BV_RD_STOP && OPECTL.op_faz != 3) {	/* ������̓���͑S���Z�}�̂��~���Ȃ��ꍇ �����Z���~�������ł͂Ȃ� */
//						miryo_timeout_after_proc();
//					}
//					wmonlg(OPMON_EC_MIRYO_TIMEOUT, NULL, 0);		// ���j�^�o�^
//				}
// MH810105 GG119202(E) �����m��̒���f�[�^��M����R0176��o�^����
				Status_Retry_Count_NG = 0;							/* ��ײ���Đ��������� */
				break;
			default:
				break;
			}
		}

		if (Suica_Rec.Data.BIT.INITIALIZE) {						/* �������I���H */
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//			Suica_Rec.suica_rcv_event.BIT.STATUS_DATA = 1;			/* �����ް���M�׸�ON */
			Suica_Rec.suica_rcv_event.EC_BIT.STATUS_DATA = 1;		/* �����ް���M�׸�ON */
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
		}
		Ope_Suica_Status = 0;
		break;

	case S_SETT_STS_DATA:	/* ���Ϗ�ԃf�[�^ */
		if (!Suica_Rec.Data.BIT.INITIALIZE) {						/* CRW-MJA�������I���H */
			break;
		}
		if(OPECTL.Ope_mod != 2) {
			// ���Z���ȊO�̌��Ϗ�ԃf�[�^�͖�������
			break;
		}
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//		if (!Suica_Rec.Data.BIT.ADJUSTOR_NOW) {
			// �I�����i�f�[�^�𑗐M���Ă��Ȃ��̂Ɍ��Ϗ�Ԃ���M�����ꍇ�͖�������
		if (Suica_Rec.Data.BIT.ADJUSTOR_NOW == 0 &&					// �I�����i�f�[�^�𑗐M���Ă��Ȃ�
			Suica_Rec.Data.BIT.BRAND_SEL == 0) {					// �u�����h�I�������Ă��Ȃ�
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
			break;
		}
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//		Ec_Settlement_Sts = wrcvbuf[0];
//		brandno = bcdbin3(&wrcvbuf[1]);
//		switch (Ec_Settlement_Sts) {
		ECCTL.Ec_Settlement_Sts = wrcvbuf[0];
		brandno = bcdbin3(&wrcvbuf[1]);
		switch (ECCTL.Ec_Settlement_Sts) {
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
		case EC_SETT_STS_MIRYO:				/* ������ */
			Suica_Rec.Data.BIT.SETTLMNT_STS_RCV = 1;							// ���Ϗ�����
// MH810103 GG119202(S) �������̒�d�Ō��ϐ��Z���~�i���j���󎚂��Ȃ�
//			save_deemed_data(brandno);											// ���d�p�f�[�^�ۑ�
//			Lagcan( OPETCBNO, TIMERNO_EC_INQUIRY_WAIT );						// �⍇��(������)���ʑ҂��^�C�}�[���Z�b�g
//// GG119202(S) �����d�l�ύX�Ή�
//			// �����m���M�Ȃ��̏ꍇ�̓J�[�hID��'*'�Ŗ��߂�
//			Ec_Settlement_Res.brand_no = EcDeemedLog.EcDeemedBrandNo;			// �u�����h�ԍ�
//			Ec_Settlement_Res.settlement_data = Product_Select_Data;			// ���ϊz
//			memset(Ec_Settlement_Res.Card_ID, '*', sizeof(Ec_Settlement_Res.Card_ID));
//																				// Card ID
//// GG119202(E) �����d�l�ύX�Ή�
			// �J�[�h��������M��ɖ���������M���邽�߁A��������M���ɕ��d�p�f�[�^��ۑ����Ȃ�
			Lagcan( OPETCBNO, TIMERNO_EC_INQUIRY_WAIT );						// �⍇��(������)���ʑ҂��^�C�}�[���Z�b�g
// MH810103 GG119202(E) �������̒�d�Ō��ϐ��Z���~�i���j���󎚂��Ȃ�
// MH810105 GG119202(S) ������������W�v�d�l���P
//// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//			EcDeemedLog.MiryoFlg = 1;											// ������
//// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
// MH810105 GG119202(E) ������������W�v�d�l���P
			break;
		case EC_SETT_STS_CARD_PROC:			/* �J�[�h������ */
			if( Suica_Rec.Data.BIT.CTRL_MIRYO ){								// ���łɖ������Ȃ�j��
				break;
			}
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//			Suica_Rec.Data.BIT.SETTLMNT_STS_RCV = 1;							// ���Ϗ�����
//			save_deemed_data(brandno);											// ���d�p�f�[�^�ۑ�
			if (Suica_Rec.Data.BIT.ADJUSTOR_NOW) {
				Suica_Rec.Data.BIT.SETTLMNT_STS_RCV = 1;						// ���Ϗ�����
				save_deemed_data(brandno);										// ���d�p�f�[�^�ۑ�
			}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j

			// �O���Ɖ����Ƃ��A���C���ɂ���~�҂��̓L�����Z�����Č���ԋp����i�x��ăJ�[�h���󂯎�����Ƃ������l�j
			// ���Z������OPECTL.credit_in_mony�ŕێ����A�O���Ɖ�I����ɍĔ��肷��
			// �������͎󂯕t���Ȃ��������Ƃɂ���
			if (OPECTL.op_faz == 8) {											// �d�q�}�̒�~�҂����킹��
				if (CCT_Cansel_Status.BIT.STOP_REASON == REASON_CARD_READ) {	// STOP_REASON���J�[�h�}���̏ꍇ�Ͷ��ނ̓f���߂����s��
					// �J�[�h�}���ɂ���~�҂��̏ꍇ�̓J�[�h��ԋp����
					if (OPECTL.other_machine_card) {
						opr_snd( 13 );											// �ۗ��ʒu����ł��߂�
					}
					else {
						opr_snd( 2 );											// �O�r�o
					}
				}
				else {
					// ���Z�����ɂ���~�҂��̏ꍇ�͂���Ⴂ�����t���O���Z�b�g����
					// ���Z������҂�����
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
					// �u�����h�I�����̂s�A�@�l�J�[�h�ǎ��Ɛ��Z�����������ɔ��������Ƃ���
					// ��Ԃ��ꎞ�I�ɖ߂��ău�����h�I�����ʂ�����̂�҂����킹��B
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
					OPECTL.credit_in_mony = 1;									// �ڼޯĂ���Ⴂ��������
					read_sht_cls();												// ���Cذ�ް������۰��
				}
				// ��~�҂��X�e�[�^�X���N���A����op_faz��������ɖ߂�
				CCT_Cansel_Status.BYTE = 0;										// ��~�҂��X�e�[�^�X���N���A
				OPECTL.op_faz = 1;												// ������
			}
			else {
// MH810103 GG119202(S) ���C���[�_�̔������҂����̓V���b�^�[�߂Ȃ�
				if ((RD_mod < 10) || (RD_mod > 13)) {							// �������҂�or�ړ����ȊO
// MH810103 GG119202(E) ���C���[�_�̔������҂����̓V���b�^�[�߂Ȃ�
				read_sht_cls();													// ���Cذ�ް������۰��
// MH810103 GG119202(S) ���C���[�_�̔������҂����̓V���b�^�[�߂Ȃ�
				}
// MH810103 GG119202(E) ���C���[�_�̔������҂����̓V���b�^�[�߂Ȃ�
				if (OPECTL.op_faz == 3 ||										// �Ƃ肯��
// MH810103 GG119202(S) ������^�C���A�E�g�Ɠd�q�}�l�[�^�b�`�����Ō��σ��[�_�����Z�b�g����
//					OPECTL.op_faz == 9) {										// �d�q�}�̒�~�҂����킹��(�Ƃ肯��)
					OPECTL.op_faz == 9 ||										// �d�q�}�̒�~�҂����킹��(�Ƃ肯��)
					time_out == 1) {											// ������������\�����̖�����^�C���A�E�g
// MH810103 GG119202(E) ������^�C���A�E�g�Ɠd�q�}�l�[�^�b�`�����Ō��σ��[�_�����Z�b�g����
					// �Ƃ肯���������ɃJ�[�h��������M�͂Ƃ肯���������~�߂�
					// ��~�҂��X�e�[�^�X���N���A����op_faz��������ɖ߂�
					OPECTL.CAN_SW = 0;											// �Ƃ肯���{�^��OFF
// MH810103 GG119202(S) ������^�C���A�E�g�Ɠd�q�}�l�[�^�b�`�����Ō��σ��[�_�����Z�b�g����
					time_out = 0;												// ������^�C���A�E�g�t���OOFF
// MH810103 GG119202(E) ������^�C���A�E�g�Ɠd�q�}�l�[�^�b�`�����Ō��σ��[�_�����Z�b�g����
					Suica_Rec.Data.BIT.ADJUSTOR_START = 1;						// ���Z�J�n�t���O�Z�b�g
					cansel_status.BYTE = 0;										// ��~�҂��X�e�[�^�X���N���A
					OPECTL.op_faz = 1;											// ������
					ac_flg.cycl_fg = 10;										// ������
				}
			}

			Lagcan( OPETCBNO, 1 );												// ��ϰ1ؾ��(���ڰ��ݐ���p)
			Lagcan( OPETCBNO, 2 );												// ��ϰ2ؾ��(ү��Ď��p)
			Lagcan( OPETCBNO, 8 );												// ��ϰ8ؾ��(������߂蔻����ϰ)
			Lagcan( OPETCBNO, TIMERNO_EC_CYCLIC_DISP );							// ���p�\�}�̻��د��\���p��ϰؾ��
			LagCan500ms(LAG500_SUICA_NONE_TIMER);								// ����f�[�^(��t�֎~)�̉����҂��^�C�}�[���Z�b�g
// MH810103 GG119202(S) ������^�C���A�E�g�Ɠd�q�}�l�[�^�b�`�����Ō��σ��[�_�����Z�b�g����
			LagCan500ms(LAG500_SUICA_ZANDAKA_RESET_TIMER);						// �ҋ@���A�p�ŏI�^�C�}���Z�b�g
// MH810103 GG119202(E) ������^�C���A�E�g�Ɠd�q�}�l�[�^�b�`�����Ō��σ��[�_�����Z�b�g����
// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
			ECCTL.prohibit_snd_bsy = 0;											// ����f�[�^(��t�֎~)�̉����Ď���ԃN���A
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
// MH810103 MHUT40XX(S) Edy�EWAON�Ή��^���ό��ʎ�M�҂��^�C�}������
//// GG119202(S) �d�q�}�l�[���ώ��̌��ό��ʎ�M�҂��^�C�}�ǉ�
//			if (brandno == BRANDNO_CREDIT) {
//				ec_inquiry_wait_time = (ushort)prm_get(COM_PRM, S_ECR, 12, 3, 1);
//// GG119202(S) �J�[�h�������^�C�}�l�ύX
////				if (ec_inquiry_wait_time == 0) {
////					ec_inquiry_wait_time = 360;
////				}
////				else if (ec_inquiry_wait_time < 240) {
////					ec_inquiry_wait_time = 240;
////				}
//				if (ec_inquiry_wait_time == 0) {
//					ec_inquiry_wait_time = 90;
//				}
//				else if (ec_inquiry_wait_time < 70) {
//					ec_inquiry_wait_time = 70;
//				}
//// GG119202(E) �J�[�h�������^�C�}�l�ύX
//			}
//			else {
//// MHUT40XX(S) Edy�EWAON�Ή�
////				// �N���W�b�g�J�[�h�ȊO�i�d�q�}�l�[�^����J�[�h���j
//				// �N���W�b�g�J�[�h�E�d�q�}�l�[�ȊO
//				if (!IsSettlementBrand((ushort)brandno)) {
//// MHUT40XX(E) Edy�EWAON�Ή�
//				ec_inquiry_wait_time = (ushort)prm_get(COM_PRM, S_ECR, 16, 2, 1);
//				if (ec_inquiry_wait_time == 0) {
//					ec_inquiry_wait_time = 50;
//				}
//				else if (ec_inquiry_wait_time < 40) {
//					ec_inquiry_wait_time = 40;
//				}
//// MHUT40XX(S) Edy�EWAON�Ή�
//				}
//// MHUT40XX(E) Edy�EWAON�Ή�
//			}
//// MHUT40XX(S) Edy�EWAON�Ή�
//			if (isEcBrandNoEMoney((ushort)brandno, 0)) {
//				// �d�q�}�l�[�u�����h�̃^�C�}�J�n
//				EcTimerUpdate();
//			}
//			else {
//// MHUT40XX(E) Edy�EWAON�Ή�
//			ec_inquiry_wait_time *= 50;
//// GG119202(E) �d�q�}�l�[���ώ��̌��ό��ʎ�M�҂��^�C�}�ǉ�
//			Lagtim( OPETCBNO, TIMERNO_EC_INQUIRY_WAIT, ec_inquiry_wait_time );	// �⍇��(������)���ʑ҂��^�C�}�[�N��
//// MHUT40XX(S) Edy�EWAON�Ή�
//			}
//// MHUT40XX(E) Edy�EWAON�Ή�
			ec_inquiry_wait_time = 0;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//			if (isEcBrandNoEMoney((ushort)brandno, 0)) {
//				// �d�q�}�l�[�u�����h�̃^�C�}�J�n
			if (EcUseKindCheck(convert_brandno((ushort)brandno))) {
				// �d�q�}�l�[�u�����h�AQR�R�[�h���σu�����h�̃^�C�}�J�n
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
				EcTimerUpdate();
			}
			else {
				if (brandno == BRANDNO_CREDIT) {
					// �N���W�b�g�J�[�h
					ec_inquiry_wait_time = (ushort)prm_get(COM_PRM, S_ECR, 12, 3, 1);
					if (ec_inquiry_wait_time == 0) {
						ec_inquiry_wait_time = 180;
					}
					else if (ec_inquiry_wait_time < 140) {
						ec_inquiry_wait_time = 140;
					}
				}
				else {
					// �d�q�}�l�[�E�N���W�b�g�J�[�h�ȊO
					ec_inquiry_wait_time = (ushort)prm_get(COM_PRM, S_ECR, 16, 3, 1);
					if (ec_inquiry_wait_time == 0) {
						ec_inquiry_wait_time = 90;
					}
					else if (ec_inquiry_wait_time < 70) {
						ec_inquiry_wait_time = 70;
					}
				}
				ec_inquiry_wait_time *= 50;
				Lagtim( OPETCBNO, TIMERNO_EC_INQUIRY_WAIT, ec_inquiry_wait_time );	// �⍇��(������)���ʑ҂��^�C�}�[�N��
			}
// MH810103 MHUT40XX(E) Edy�EWAON�Ή��^���ό��ʎ�M�҂��^�C�}������

			OPECTL.InquiryFlg = 1;												// �O���Ɖ�t���OON
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
			if (Suica_Rec.Data.BIT.BRAND_SEL != 0 &&
				Suica_Rec.Data.BIT.BRAND_CAN != 0) {
				Suica_Rec.Data.BIT.BRAND_CAN = 0;								// �u�����h�I���������𖳂��������Ƃɂ���
			}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
			cn_stat( 2, 2 );													// �����s��
			if( MIFARE_CARD_DoesUse ){											// Mifare���L���ȏꍇ
				op_MifareStop_with_LED();										// Mifare����
			}
			if(brandno == BRANDNO_CREDIT) {
				ec_MessageAnaOnOff( 1, 1 );										// �N���W�b�g�J�[�h������
			} else {
				ec_MessageAnaOnOff( 1, 2 );										// �J�[�h������
			}
			if( OPECTL.op_faz == 0 ){											// ������
				OPECTL.op_faz = 1;												// ������
			}
			break;
// MH810103 GG119202(S) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
		case EC_SETT_STS_DEEMED_SETTLEMENT:	// �݂Ȃ�����
// MH810103 GG119202(E) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
		case EC_SETT_STS_INQUIRY_OK:		/* �Ɖ�OK */
		case EC_SETT_STS_NONE:				/* ��L�ȊO */
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//			Suica_Rec.suica_rcv_event.BIT.SETTSTS_DATA = 1;
			Suica_Rec.suica_rcv_event.EC_BIT.SETTSTS_DATA = 1;
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
			break;
		default:
			break;
		}
// MH810103 MH810103(s) �d�q�}�l�[�Ή�
//// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
//		// ���Ϗ���ް�_LCD_ICC_SETTLEMENT_STS��OpeTask�֑��M
//		queset(OPETCBNO, LCD_ICC_SETTLEMENT_STS, sizeof(unsigned char), &Ec_Settlement_Sts);		// ���Ϗ���ް�
//// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
		// ���Ϗ���ް�_LCD_ICC_SETTLEMENT_STS��OpeTask�֑��M
		queset(OPETCBNO, LCD_ICC_SETTLEMENT_STS, sizeof(unsigned char), &ECCTL.Ec_Settlement_Sts);		// ���Ϗ���ް�
// MH810103 MH810103(e) �d�q�}�l�[�Ή�
		break;

// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
//	case S_RECEIVE_DEEMED:	/* ���ϐ��Z���~(������)�f�[�^ */
//
//		memset(&DeemedData, 0, sizeof(T_FrmReceipt));
//		work_brand = convert_brandno( bcdbin3(&wrcvbuf[2]) );
//
//		/* ��M�ް���� */
//		ec_settlement_data_save(&Ec_Settlement_Res, wrcvbuf);
//		// EcRecvDeemedData�Ɏ�M�f�[�^���Z�b�g
//		EcElectronSet_DeemedData(&Ec_Settlement_Res);
//
//		if (work_brand != EC_UNKNOWN_USED) {
//			ac_flg.cycl_fg = 57;										// 57:�s�ر�W�v���Z����
//			// �s���u�����h�ȊO���O�o�^
//			Log_Write(eLOG_PAYMENT, &EcRecvDeemedData, TRUE);
//		}
//
//		if( PrnJnlCheck() == ON ){
//			DeemedData.prn_kind = J_PRI;								/* �������ʁF�ެ��� */
//			DeemedData.prn_data = &EcRecvDeemedData;					/* �̎��؈��ް����߲����� */
//
//			// pritask�֒ʒm
//			queset(PRNTCBNO, PREQ_RECV_DEEMED, sizeof(T_FrmReceipt), &DeemedData);
//		}
//		break;
//
	case S_RECEIVE_DEEMED:	/* ���ϒ��O����f�[�^ */

		// �f�[�^���Z�b�g���Ȃ��ꍇ�͔�����
		if( !EcRecvDeemed_DataSet( wrcvbuf ) ){
			break;
		}

		// �ҋ@�܂��͋x�Ƃ̏ꍇ�A���O�o�^�A�󎚏�������
		if( OPECTL.Ope_mod == 0 || OPECTL.Ope_mod == 100 ){
			EcRecvDeemed_RegistPri();
		}
		// �ҋ@�܂��͋x�Ƃł͂Ȃ��ꍇ�A�����t���O��ON����i�ҋ@�J�ڌ�Ɏ��{����j
		else{
			ac_flg.ec_recv_deemed_fg = 1;							// ���O����f�[�^�����t���OON
		}
		break;
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
	case S_SETTLEMENT_DATA:	/* ���ό��ʃf�[�^ */
		if (wrcvbuf[0] == 0) {
			break;
		}
		work_brand = convert_brandno( bcdbin3(&wrcvbuf[2]) );

		switch (wrcvbuf[0]) {
		case EPAY_RESULT_OK:										// ����OK
// MH810103 GG119202(S) �����d�l�ύX�Ή�
//		case EPAY_RESULT_MIRYO_AFTER_OK:							// �����m��㌈��OK
// MH810103 GG119202(E) �����d�l�ύX�Ή�
		case EPAY_RESULT_MIRYO_ZANDAKA_END:							// �����c���Ɖ��
			memset(moni_wk, 0, sizeof(moni_wk));

			moni_wk[0] = (uchar)(work_brand - EC_USED + 1);			// �J�[�h���(0���Ə��Ȃ��ƂȂ��Ă��܂����߁A1�`�Ƃ��Ă���)

			memset(Card_Work, 0x20, sizeof(Card_Work));
			memcpyFlushRight(Card_Work, &wrcvbuf[13], sizeof(Card_Work), 20);
			memcpy(&moni_wk[1], &Card_Work[16], 4);					// �J�[�h�ԍ��̉��S�����Z�b�g
			moni_wk[5] = wrcvbuf[0];								// ���ό���
// MH810105 GG119202(S) ���Ϗ������ɏ�Q�������������̓���
			if (ECCTL.Ec_Settlement_Sts == EC_SETT_STS_DEEMED_SETTLEMENT) {
				// �݂Ȃ����ϒʒm���͎x�������i0x11�j���Z�b�g����
				moni_wk[5] = EPAY_RESULT_PAY_MIRYO;
				// �J�[�h�ԍ����X�y�[�X���߂̏ꍇ��'*'���Z�b�g����
				if (!strncmp((char*)&moni_wk[1], "    ", 4)) {
					memset(&moni_wk[1], '*', 4);
				}
			}
// MH810105 GG119202(E) ���Ϗ������ɏ�Q�������������̓���
			switch(work_brand)
			{
				case EC_CREDIT_USED:
					work = settlement_amount_ex(&wrcvbuf[4], 3);	// ���ϋ��z
					if (work > 0xffff) {
					// ����z��short�l�ȏ�Ȃ�␳
						work = 0xffff;
					}
					moni_wk[6] = (uchar)(work & 0xff);
					moni_wk[7] = (uchar)((work >> 8) & 0xff);
					wmonlg(OPMON_EC_CRE_SETTLEMENT, moni_wk, 0);	// ���j�^�o�^
					break;
				case EC_EDY_USED:
				case EC_NANACO_USED:
				case EC_WAON_USED:
				case EC_SAPICA_USED:
				case EC_KOUTSUU_USED:
// MH810104 GG119202(S) nanaco�EiD�EQUICPay�Ή�
//				case EC_ID_USED:
//				case EC_QUIC_PAY_USED:
//				case EC_UNKNOWN_USED:
//				case EC_ZERO_USED:
// MH810104 GG119202(E) nanaco�EiD�EQUICPay�Ή�
					work = settlement_amount_ex(&wrcvbuf[7], 3);	// ���ϑO�c�z
					if (work > 0xffff) {
					// ���ϑO�c�z��short�l�ȏ�Ȃ�␳
						work = 0xffff;
					}
					moni_wk[6] = (uchar)(work & 0xff);
					moni_wk[7] = (uchar)((work >> 8) & 0xff);
					work = settlement_amount_ex(&wrcvbuf[10], 3);	// ���ό�c�z
					if (work > 0xffff) {
					// ���ό�c�z��short�l�ȏ�Ȃ�␳
						work = 0xffff;
					}
					moni_wk[8] = (uchar)(work & 0xff);
					moni_wk[9] = (uchar)((work >> 8) & 0xff);
					wmonlg(OPMON_EC_SETTLEMENT, moni_wk, 0);		// ���j�^�o�^
					break;
// MH810104 GG119202(S) nanaco�EiD�EQUICPay�Ή�
				case EC_ID_USED:
				case EC_QUIC_PAY_USED:
// MH810105(S) MH364301 PiTaPa�Ή�
				case EC_PITAPA_USED:
// MH810105(E) MH364301 PiTaPa�Ή�
				case EC_UNKNOWN_USED:
				case EC_ZERO_USED:
					if (work_brand == EC_ID_USED) {
						memset(&moni_wk[2], '*', 3);				// ��3���}�X�N
					}
					else if (work_brand == EC_QUIC_PAY_USED) {
						memset(&moni_wk[1], '*', 4);				// ��4���}�X�N
					}
					work = settlement_amount_ex(&wrcvbuf[4], 3);	// ���ϋ��z
					if (work > 0xffff) {
					// ����z��short�l�ȏ�Ȃ�␳
						work = 0xffff;
					}
					moni_wk[6] = (uchar)((work >> 8) & 0xff);
					moni_wk[7] = (uchar)(work & 0xff);
					wmonlg(OPMON_EC_SETTLEMENT, moni_wk, 0);		// ���j�^�o�^
					break;
// MH810104 GG119202(E) nanaco�EiD�EQUICPay�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
				case EC_QR_USED:
					work = settlement_amount_ex(&wrcvbuf[4], 3);	// ���ϋ��z
					if (work > 0xffff) {
					// ����z��short�l�ȏ�Ȃ�␳
						work = 0xffff;
					}
					moni_wk[6] = (uchar)(work & 0xff);
					moni_wk[7] = (uchar)((work >> 8) & 0xff);
					moni_wk[8] = wrcvbuf[44];	// �x�����
					wmonlg(OPMON_EC_SETTLEMENT, moni_wk, 0);		// ���j�^�o�^
					break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
				default:
					break;
			}
			break;
		default:
			break;
		}

		if (!Suica_Rec.Data.BIT.INITIALIZE) {						/* CRW-MJA�������I���H */
			// ���������ɗL���Ȍ��ό��ʃf�[�^����M�����ꍇ�͌��ψُ�Ƃ��ēo�^����
			EcSettlementPhaseError(wrcvbuf, 7);
			break;
		}
		if (Suica_Rec.Data.BIT.PAY_CTRL == 1) {								/* ���łɂP�x�A�d�q���ς��s���Ă���ꍇ */
			EcSettlementPhaseError(wrcvbuf, 3);								/* �G���[�o�^ */
			break;															/* �Ȍ�̏��������Ȃ��̂Ŕ����� */
		}
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//		if (Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 1 &&			/* ���łɌ��ό��ʃf�[�^��M�ς݂� */
//			Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA == 0) {  	/* �܂�Ope���������s���Ă��Ȃ��ꍇ or */
		if (Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA == 1 &&		/* ���łɌ��ό��ʃf�[�^��M�ς݂� */
			Suica_Rec.suica_rcv_event_sumi.EC_BIT.SETTLEMENT_DATA == 0) {	/* �܂�Ope���������s���Ă��Ȃ��ꍇ or */
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
			EcSettlementPhaseError(wrcvbuf, 2);								/* �G���[�o�^ */
			break;															/* �Ȍ�̏��������Ȃ��̂Ŕ����� */
		}
		if (!Suica_Rec.Data.BIT.ADJUSTOR_NOW) {
			// ���ϒ��ł͂Ȃ��̂ɗL���Ȍ��ό��ʃf�[�^����M�����ꍇ�͌��ψُ�Ƃ��ēo�^����
			EcSettlementPhaseError(wrcvbuf, 8);
			break;
		}
		if (StopStatusGet(1) == 0x07) {
			// ���Z���~���őS���W���[����~��Ɍ��ό��ʃf�[�^����M
			EcSettlementPhaseError(wrcvbuf, 9);
			break;
		}

		/* ��M�ް���� */
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
//		ec_settlement_data_save(&Ec_Settlement_Res, wrcvbuf);
		ec_settlement_data_save(&Ec_Settlement_Res, wrcvbuf, 1);
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��

		pay_ng = 1;
		// ��M�����u�����h�ԍ���0�H
		if (work_brand != EC_ZERO_USED) {
			// �I�����i�f�[�^�ɂđ��M�����u�����h�ԍ��ƈ�v�H
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//			if( Product_Select_Brand == Ec_Settlement_Res.brand_no ){
			if( ECCTL.Product_Select_Brand == Ec_Settlement_Res.brand_no ){
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
				pay_ng = 0;
				// E3265����
				err_chk((char)jvma_setup.mdl, ERR_EC_BRANDNO_ZERO, 0, 0, 0);
			} else {
				// ��v���Ȃ������A�G���[(E3263)�o�^
				pay_ng = 1;
			}
		} else {
			// ��M�����u�����h�ԍ���0�Ȃ�G���[(E3263)�o�^���Ȃ�
			pay_ng = 0;
		}

		if (pay_ng != 0) {
		// ���Ή��^���ݒ�u�����h�̎���f�[�^��M
		// �ȍ~�A���ꐸ�Z���ł̓d�q�}�l�[���p�s��(�����z�A�W�v�͍s��Ȃ����A���j�^�o�^�͍s��)
			err_chk((char)jvma_setup.mdl, ERR_EC_PAYBRAND_ERR, 1, 0, 0);// �װ۸ޓo�^�i�o�^�j
			// E3263������ɐ��Z�𒆎~�����E3259�G���[�����������B
			// ���P�A��#10�̉��P�ɂ��E3263�̔��������������������A
			// E3263�����������ꍇ�AE3262�������ɔ���������
			// �d�q�}�l�[�𗘗p�s�Ƃ���B
			// 2019/07/29 �{���ۂ����������ꍇ�A���ς����������Ă���ȍ~�̌��ς�s�Ƃ���悤�ɕύX
			Suica_Rec.suica_err_event.BIT.SETTLEMENT_ERR = 1;			// EC���ψُ픭���t���O
		}

		switch (Ec_Settlement_Res.Result) {
		case EPAY_RESULT_OK:					// ����OK
// MH810103 GG119202(S) �����c���Ɖ�̂Ƃ��A���ϊz(0�~�܂�)�Ə��i�I���f�[�^�ō�������ꍇ�A����ُ�ƂȂ�A����L�[�����҂��ƂȂ邪�A�L�[�������Ȃ��C��
		//����ُ�̖����m��́A����ُ킹���A������s/��������ʕ\������
//		case EPAY_RESULT_MIRYO:					// �����m��
// MH810103 GG119202(E) �����c���Ɖ�̂Ƃ��A���ϊz(0�~�܂�)�Ə��i�I���f�[�^�ō�������ꍇ�A����ُ�ƂȂ�A����L�[�����҂��ƂȂ邪�A�L�[�������Ȃ��C��
// MH810103 GG119202(S) �����d�l�ύX�Ή�
//		case EPAY_RESULT_MIRYO_AFTER_OK:		// �����m��㌈��OK
// MH810103 GG119202(E) �����d�l�ύX�Ή�
		case EPAY_RESULT_MIRYO_ZANDAKA_END:		// �����c���Ɖ��
			if (Product_Select_Data != Ec_Settlement_Res.settlement_data) {
			// �v���z�ƌ��ϊz���s��v
				// �G���[���O�o�^
				memset(err_wk, 0, sizeof(err_wk));
				memcpy(err_wk, "\?\?-0000   00000:00000", 21);
				// �J�[�h���
				memcpy(&err_wk[0], EcBrandKind_prn[work_brand - EC_USED], 2);
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
				if( work_brand == EC_QR_USED ){							// QR�̏ꍇ�A�x����ʂ��Z�b�g
					memset(&err_wk[3], 0x20, 4);						// �S�����X�y�[�X���Z�b�g
					memcpy(&err_wk[3], QrBrandKind_prn[Ec_Settlement_Res.Brand.Qr.PayKind], 2);
				}
				else{
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
				// �J�[�h�ԍ�
				memset(Card_Work, 0x20, sizeof(Card_Work));
				memcpyFlushRight(Card_Work, Ec_Settlement_Res.Card_ID, sizeof(Card_Work), sizeof(Ec_Settlement_Res.Card_ID));
				memcpy(&err_wk[3], &Card_Work[16], 4);					// �J�[�h�ԍ��̉��S�����Z�b�g
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
				}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
				// ����w���z
				work = Product_Select_Data;
				if (work > 0xffff) {
				// ����w���z��short�l�ȏ�Ȃ�␳
					memcpy(&err_wk[10], "FFFFH", 5);
				} else {
					intoasl(&err_wk[10], work, 5);
				}
				// ������
				if (Ec_Settlement_Res.Result != EPAY_RESULT_OK) {
					memcpy(&err_wk[7], "(A)", 3);
				}
				// ����z
				work = Ec_Settlement_Res.settlement_data;
				if (work > 0xffff) {
				// ����z��short�l�ȏ�Ȃ�␳
					memcpy(&err_wk[16], "FFFFH", 5);
				} else {
					intoasl(&err_wk[16], work, 5);
				}
				err_chk2((char)jvma_setup.mdl, ERR_EC_PAY_ERR, 1, 1, 0, err_wk);	/* �װ۸ޓo�^�i�o�^�j*/
				// E3262�o�^�i���ϕs�ǁFASCII�j 			"E3262   08/03/17 14:38   ����/����"
				//											"        ED-3456   01000:01100     "
				//											"        ED-3456(A)01000:01100     "
				//												 ��ʁA�J�[�h�ԍ���4���A�����ʁA����w���z�A����z
#if (4 != AUTO_PAYMENT_PROGRAM)										// �����p�ɃJ�[�h�c�z�����܂����ꍇ�iýėp�j
// ����{�I��(0 == AUTO_PAYMENT_PROGRAM)�Ȃ̂ŁA�L��
//   ���σ��[�_�ł�SUICA_LOG_REC,SUICA_LOG_REC_FOR_ERR�̗̈���g�p����
				memcpy(&SUICA_LOG_REC_FOR_ERR, &SUICA_LOG_REC, sizeof(struct suica_log_rec));	// �ُ�۸ނƂ��Č��݂̒ʐM۸ނ�o�^����B
#endif
				Ec_Pri_Data_Snd(S_CNTL_DATA, 0x80);								// �����ް��i����I���j�𑗐M����
				ec_MessageAnaOnOff( 0, 0 );										// �\�����̕���������
				OPECTL.InquiryFlg = 0;											// �O���Ɖ�t���OOFF
				Suica_Rec.suica_err_event.BIT.PAY_DATA_ERR = 1;					// ��M�������ϊz�Ƒ��M�����I�����i�f�[�^�ɍ��ق�����
				Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 1;							// �����^�C���A�E�g�������������Ƃɂ���
				Suica_Rec.Data.BIT.ADJUSTOR_START = 0;							// �I�����i�f�[�^���M�ۃt���O�N���A
// MH810103 GG119202(S) �����c���Ɖ�ŗv���z�ƌ��ϊz���s��v�̌��ό��ʁF�����c���Ɖ������M�����Ƃ��A����L�[�������Ȃ��B
				Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;
				Suica_Rec.Data.BIT.CTRL_MIRYO = 0;
// MH810103 GG119202(E) �����c���Ɖ�ŗv���z�ƌ��ϊz���s��v�̌��ό��ʁF�����c���Ɖ������M�����Ƃ��A����L�[�������Ȃ��B
// MH810103 MHUT40XX(S) �J�[�h�������҂����b�Z�[�W�\�����㏑�������
				// E3262�����Ō��σ��[�_�؂藣������̂ŁA�J�[�h�������ݏ�Ԃ̓N���A����
				Suica_Rec.Data.BIT.CTRL_CARD = 0;								// �J�[�h�������ݏ�Ԃ��N���A
// MH810103 MHUT40XX(E) �J�[�h�������҂����b�Z�[�W�\�����㏑�������
				dsp_change = 0;													// �����\���׸ނ�������
				queset(OPETCBNO, SUICA_PAY_DATA_ERR, 0, NULL);					// ���ψُ��ʒm
// MH810103 GG119202(S) E3259�AE3262�AE3263������JVMA���Z�b�g���Ȃ�
				// ���ϕs�ǔ�������JVMA���Z�b�g����������܂�
				// ���σ��[�_�Ƃ̒ʐM�͍s�킸�A�R�C�����b�N���̒ʐM��D�悳����
				// ec_init()����ECCTL.phase���X�V����
				Suica_Rec.Data.BIT.INITIALIZE = 0;
				ECCTL.phase = EC_PHASE_WAIT_RESET;
// MH810103 GG119202(E) E3259�AE3262�AE3263������JVMA���Z�b�g���Ȃ�
// MH810103 GG119202(S) E3262�������ɂ݂Ȃ����ςɂȂ��Ă��܂�
				Lagcan( OPETCBNO, TIMERNO_EC_INQUIRY_WAIT );					// �⍇��(������)���ʑ҂��^�C�}�[���Z�b�g
// MH810103 GG119202(E) E3262�������ɂ݂Ȃ����ςɂȂ��Ă��܂�
				return;															// ���Ϗ����͂��Ȃ��i�������Ƃ���j
			}
// MH810103 GG119202(S) ���Ϗ�ԁi�݂Ȃ����ρj�̔���C��
			if (ECCTL.Ec_Settlement_Sts == EC_SETT_STS_DEEMED_SETTLEMENT) {
// MH810105(S) MH364301 �݂Ȃ����σf�[�^��M�G���[�ǉ�
				err_chk((char)jvma_setup.mdl, ERR_EC_RCV_DEEMED_SETTLEMENT, 2, 0, 0);
// MH810105(E) MH364301 �݂Ȃ����σf�[�^��M�G���[�ǉ�
				// ���Ϗ��=�݂Ȃ����ς̏ꍇ�͐ݒ�50-0014�ɏ]��
				EcDeemedSettlement(&OPECTL.Ope_mod);
// MH810105(S) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P
//				if (IsErrorOccuerd((char)jvma_setup.mdl, ERR_EC_SETTLE_ABORT)) {
//					// �x�Ƃł���΁A���ό��ʃf�[�^���������Ȃ�
//// MH810104 GG119202(S) �݂Ȃ��x�ƂɂȂ�Ƃ�����I�����o�Ȃ�
//					Ec_Pri_Data_Snd(S_CNTL_DATA, 0x80);							// �����ް��i����I���j�𑗐M����
//// MH810104 GG119202(E) �݂Ȃ��x�ƂɂȂ�Ƃ�����I�����o�Ȃ�
//					return;
//				}
				if (PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 0) {
					// ���Ϗ�ԁ��݂Ȃ����ώ�M�Łu����OK�Ƃ��Đ��Z�����v���Ȃ��ꍇ��
					// ���ό��ʃf�[�^���������Ȃ�
					Ec_Pri_Data_Snd(S_CNTL_DATA, 0x80);							// �����ް��i����I���j�𑗐M����
					return;
				}
// MH810105(E) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P
// MH810105 GG119202(S) �����c���Ɖ�ɏ�Q�����������ꍇ�͐ݒ�50-0014�ɏ]��
//// MH810103 GG118807_GG118907(S) �����c���Ɖ�ɒʐM�s�ǔ����ł݂Ȃ����ςɂȂ�
//				if (Suica_Rec.Data.BIT.MIRYO_TIMEOUT != 0) {
//					// �����m���͌��ό��ʃf�[�^���������Ȃ�
//					return;
//				}
//// MH810103 GG118807_GG118907(E) �����c���Ɖ�ɒʐM�s�ǔ����ł݂Ȃ����ςɂȂ�
//// MH810104 GG119201(S) WAON���Z���A�����c���������Ɍ��Ϗ��(�݂Ȃ����ρA����OK)��M�ŏ�����������L�^��2���o�^����
//				if ( work_brand == EC_WAON_USED &&
//// MH810104 GG119202(S) WAON���Z���A�����Ƃ݂Ȃ����ς̋����ł݂Ȃ����ς��������Ȃ�
////					 OPECTL.op_faz == 8 &&
////					 CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END ){
////					// �u�����h��WAON�ł���A�����m��ςł���Ό��ό��ʃf�[�^���������Ȃ�
////					// �u�����h��WAON�ł���50-0011�@=2�ȊO�̏ꍇ�ASuica_Rec.Data.BIT.MIRYO_TIMEOUT�t���O��ON�ɂȂ�Ȃ�����
////					// CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END(���Z����)�ɂ��OPECTL.op_faz=8(��~��)�ł���ꍇ
////					// ���ό��ʃf�[�^���������Ȃ��悤�ɂ�����return����B
//					 EcDeemedLog.MiryoFlg == 2 &&
//					 IsErrorOccuerd((char)jvma_setup.mdl, ERR_EC_ALARM_TRADE_WAON) == 0 ){
//					// �u�����h��WAON�ł���A�����m��ςł���Ό��ό��ʃf�[�^���������Ȃ�
//					// �u�����h��WAON�ł���50-0011�@=2�ȊO�̏ꍇ(���Z������������ꍇ)�ASuica_Rec.Data.BIT.MIRYO_TIMEOUT�t���O��ON�ɂȂ�Ȃ�����
//					// EcDeemedLog.MiryoFlg=2(�����m��)�ł���ERR_EC_ALARM_TRADE_WAON(E3267)�̃G���[���������Ă��Ȃ��ꍇ
//					// ���ό��ʃf�[�^���������Ȃ��悤�ɂ�����return����B
//// MH810104 GG119202(E) WAON���Z���A�����Ƃ݂Ȃ����ς̋����ł݂Ȃ����ς��������Ȃ�
//					return;
//				}
//// MH810104 GG119201(E) WAON���Z���A�����c���������Ɍ��Ϗ��(�݂Ȃ����ρA����OK)��M�ŏ�����������L�^��2���o�^����
// MH810105 GG119202(E) �����c���Ɖ�ɏ�Q�����������ꍇ�͐ݒ�50-0014�ɏ]��
// MH810105(S) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P
//// MH810104 GG119201(S) ���������Ɠ����ɃJ�[�h�^�b�`�������Ő��Z���������ɓd�q�}�l�[�Ő��Z�������Ă��܂�
//				if ( ryo_buf.zankin == 0 &&
//					 PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin == 0){
//					// ���Ϗ��=�݂Ȃ�����(50-0014�E=1)�Ɠ����̋����ɂ��c����0�~�ł���ꍇ��
//					// �����ł̐��Z�����������邽��(deemed_sett_fin�F�݂Ȃ����ςɂ�鐸�Z�����t���O��OFF)
//					// ���ό��ʃf�[�^���������Ȃ��悤�ɂ�����return����B
//					return;
//				}
//// MH810104 GG119201(E) ���������Ɠ����ɃJ�[�h�^�b�`�������Ő��Z���������ɓd�q�}�l�[�Ő��Z�������Ă��܂�
//// MH810105 GG119202(S) ���Ϗ������ɏ�Q�������������̓���
//				if (Ec_Settlement_Res.Result == EPAY_RESULT_NG) {
//					// �݂Ȃ����ϒʒm�{���Z�p���i50-0014�C=2�j�̏ꍇ��
//					// �����Ŕ�����
//					return;
//				}
//// MH810105 GG119202(E) ���Ϗ������ɏ�Q�������������̓���
// MH810105(E) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P
			}
// MH810103 GG119202(E) ���Ϗ�ԁi�݂Ȃ����ρj�̔���C��
			break;
// MH810103 GG119202(S) �J�[�h��������M��̌���NG�i304�j�̈���
		case EPAY_RESULT_NG:
			if (Suica_Rec.Data.BIT.CTRL_MIRYO != 0) {
				// ����������
				if (IsErrorOccuerd((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_304)) {
// MH810104 GG119202(S) �݂Ȃ��x�ƂɂȂ�Ƃ�����I�����o�Ȃ�
					Ec_Pri_Data_Snd(S_CNTL_DATA, 0x80);							// �����ް��i����I���j�𑗐M����
// MH810104 GG119202(E) �݂Ȃ��x�ƂɂȂ�Ƃ�����I�����o�Ȃ�
// MH810103 GG119001_GG119101(S) �����c���Ɖ�ɒʐM�s�ǔ����ł݂Ȃ����ςɂȂ�
//					if (Suica_Rec.Data.BIT.MIRYO_CONFIRM != 0) {
//						// �����m���̌���NG�i304�j��M�͖����c���Ɖ�^�C���A�E�g�����Ƃ���
//						ec_miryo_timeout();
//					}
//					else {
//						// �����m��O�̌���NG�i304�j��M�݂͂Ȃ����ψ����Ƃ���
//						EcDeemedSettlement(&OPECTL.Ope_mod);
//					}
					// �����m��O�̌���NG�i304�j��M�݂͂Ȃ����ψ����Ƃ���
					// �����m���̌���NG�i304�j��M�͖����c���Ɖ�^�C���A�E�g�����Ƃ���
					EcDeemedSettlement(&OPECTL.Ope_mod);
// MH810103 GG119001_GG119101(E) �����c���Ɖ�ɒʐM�s�ǔ����ł݂Ȃ����ςɂȂ�
				}
				// �����������̌���NG�͖�������
				return;
			}
			else if (OPECTL.InquiryFlg != 0) {
				// �J�[�h��������M�ς�
// MH810103 GG119202(S) �N���W�b�g���ςŌ���NG�i304�j��M����Ƃ݂Ȃ����ς��Ă��܂�
//				if (IsErrorOccuerd((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_304)) {
				if (IsErrorOccuerd((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_304) &&
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//					isEcBrandNoEMoney(Ec_Settlement_Res.brand_no, 0)) {
					EcUseKindCheck(convert_brandno(Ec_Settlement_Res.brand_no))) {	//�d�q�}�l�[�g�p���H
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
					// �d�q�}�l�[�u�����h�݂݂̂Ȃ����ψ����Ƃ���
// MH810103 GG119202(E) �N���W�b�g���ςŌ���NG�i304�j��M����Ƃ݂Ȃ����ς��Ă��܂�
					// �J�[�h��������M��̌���NG�i304�j��M�݂͂Ȃ����ψ����Ƃ���
// MH810104 GG119202(S) �݂Ȃ��x�ƂɂȂ�Ƃ�����I�����o�Ȃ�
					Ec_Pri_Data_Snd(S_CNTL_DATA, 0x80);					// �����ް��i����I���j�𑗐M����
// MH810104 GG119202(E) �݂Ȃ��x�ƂɂȂ�Ƃ�����I�����o�Ȃ�
					EcDeemedSettlement(&OPECTL.Ope_mod);
					return;
				}
			}
			break;
// MH810103 GG119202(E) �J�[�h��������M��̌���NG�i304�j�̈���
		default:
			break;
		}
		// ������A�c�z����P�[�X�͂��肦�Ȃ����A����Ⴂ�΍�Ƃ��Ďc���Ă���
		if (ryo_buf.zankin > Ec_Settlement_Res.settlement_data && 			/* �c�z���܂�����H */
			OPECTL.op_faz != 3 && 											/* ���Z���~�������ł͂Ȃ� */
			(OPECTL.op_faz == 8 && wrcvbuf[0] == EPAY_RESULT_OK)) {			/* �d�q�}�̒�~�������H */
			cn_stat(3, 2);													// ���� & COIN�ē�����
			if (!pas_kep || ((RED_stat[2] & TICKET_MOUTH) ==  TICKET_MOUTH)) {	/* ��������}�����ɂ��� */
				read_sht_opn();												/* ���Cذ�ް���������� */
			}
		}
// ýėp ��������z�����ɖ߂�
#ifdef	EC_DEBUG
		if (CPrmSS[S_SCA][98] != 0L) {
			Ec_Settlement_Res.settlement_data *= 10;
		}
#endif
// ýėp ��������z�����ɖ߂�

		if (Suica_Rec.Data.BIT.INITIALIZE) {								/* �������I���H */
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//			Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA = 1;				/* ����ް���M�׸�ON */
			Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA = 1;			/* ����ް���M�׸�ON */
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
		}

// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//		if (Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 1 &&			// ���ό����ް���M�׸�ON?
		if (Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA == 1 &&		// ���ό����ް���M�׸�ON?
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
			(Ec_Settlement_Res.Result == EPAY_RESULT_OK ||					// ����OK�H
// MH810103 GG119202(S) �����d�l�ύX�Ή�
//			 Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO_AFTER_OK ||		// �����m��㌈��OK�H
// MH810103 GG119202(E) �����d�l�ύX�Ή�
			 Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO_ZANDAKA_END)) {	// �����c���Ɖ���H
			// �����׸ނ�ر�������ݸނ͐��Z۸ދy�ѐ��Z���~۸ނɌ��σf�[�^��o�^���邩�A
			// �����Z���̐��Z���J�n�����ꍇ�Ƃ���B
			Suica_Rec.Data.BIT.LOG_DATA_SET = 1;							/* ���ό����ް���M�ς� */

			// ���̃t���O�͐��Z���J�n����O�y�ь��ς��s��ꂽ�ꍇ�ɃN���A���A
			// �Z�b�g����^�C�~���O�͐��Z�J�n��A���ԗ��������������ꍇ�Ƃ���
			Suica_Rec.Data.BIT.ADJUSTOR_START = 0;							/* �I�����i�f�[�^���M�ۃt���O�N���A */
		}
// MH810103 GG119202(S) E3259�AE3262�AE3263������JVMA���Z�b�g���Ȃ�
		if (pay_ng != 0) {
			// ���ݒ�u�����h����f�[�^��M����JVMA���Z�b�g����������܂�
			// ���σ��[�_�Ƃ̒ʐM�͍s�킸�A�R�C�����b�N���̒ʐM��D�悳����
			// ec_init()����ECCTL.phase���X�V����
			EcBrandClear();
			Suica_Rec.Data.BIT.INITIALIZE = 0;
			ECCTL.phase = EC_PHASE_WAIT_RESET;
		}
// MH810103 GG119202(E) E3259�AE3262�AE3263������JVMA���Z�b�g���Ȃ�
		break;

	case S_VOLUME_RES_DATA:	/* ���ʕύX���ʃf�[�^ */
		if(!Suica_Rec.Data.BIT.INITIALIZE) {
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
			if (ECCTL.phase != EC_PHASE_VOL_CHG) {
				// �t�F�[�Y�Ⴂ�Ŏ�M�����ꍇ�͖�������
				break;
			}
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
// MH810103 GG119202(S) �N���V�[�P���X��������������
			if( Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.VOL_RES_RCV == 0 ) {	// ���ʕύX���ʃf�[�^����M
				Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.VOL_RES_RCV = 1;		// ���ʕύX���ʃf�[�^��M�ς�
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
				Lagcan(OPETCBNO, TIMERNO_EC_BRAND_NEGO_WAIT);
				setEcBrandNegoComplete();
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
			}
// MH810103 GG119202(E) �N���V�[�P���X��������������
			break;
		}
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//		RecvVolumeRes = wrcvbuf[0];									// ���ʕύX���ʂ�ۑ�
		ECCTL.RecvVolumeRes = wrcvbuf[0];							// ���ʕύX���ʂ�ۑ�
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
		// �����e�ɒʒm
		queset(OPETCBNO, EC_VOLUME_UPDATE, 0, NULL);			// ���ʕύX�X�V��ʒm
		break;
	case S_MODE_CHG_DATA:	/* ���[�_�����e�i���X���ʃf�[�^ */
		if(!Suica_Rec.Data.BIT.INITIALIZE) {
			break;
		}
		p = &wrcvbuf[0];
		RecvMntTbl.cmd = *p++;										// ���[�_�����e�i���X���ʂ�ۑ�
		RecvMntTbl.mode = *p++;
		RecvMntTbl.vol = *p++;
		queset(OPETCBNO, EC_MNT_UPDATE, 0, NULL);					// ���[�_�����e�i���X�X�V��ʒm
		break;
	case S_BRAND_RES_DATA:	/* �u�����h�I�����ʃf�[�^ */
		if (!Suica_Rec.Data.BIT.INITIALIZE) {						/* CRW-MJA�������I���H */
			break;
		}
		p = &wrcvbuf[0];
// MH810103 GG119202(S) �u�����h�I����̃L�����Z�������ύX
//		if(RecvBrandResTbl.res == EC_BRAND_SELECTED &&				// �I����Ԃ���
//			*p == EC_BRAND_UNSELECTED) {							// ���I����Ԃɖ߂��ꂽ
//			Suica_Rec.Data.BIT.SLCT_BRND_BACK = 1;					// �u�߂�v���o
//		}
// MH810103 GG119202(E) �u�����h�I����̃L�����Z�������ύX
		RecvBrandResTbl.res = *p++;									// �u�����h�I�����ʂ�ۑ�
		if(RecvBrandResTbl.res == EC_BRAND_SELECTED) {				// �I��
			RecvBrandResTbl.no  = bcdbin3(p);
			p += 2;
			RecvBrandResTbl.siz = *p++;
			memcpy(RecvBrandResTbl.dat, p, EC_CARDDATA_LEN);
		} else {													// ���I��
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//			RecvBrandResTbl.no  = (ushort)BRANDNO_UNKNOWN;
			RecvBrandResTbl.no  = bcdbin3(p);
			p += 2;
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
			RecvBrandResTbl.siz = 0;
			memset(RecvBrandResTbl.dat, 0x20, EC_CARDDATA_LEN);
		}
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//		Suica_Rec.suica_rcv_event.BIT.BRAND_DATA = 1;				// �u�����h�I�������ް���M�׸�ON
		Suica_Rec.suica_rcv_event.EC_BIT.BRAND_DATA = 1;			// �u�����h�I�������ް���M�׸�ON
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
		break;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	case S_SUB_BRAND_RES_DATA:	/*�T�u�u�����h�ꗗ�f�[�^*/
		memset(&RecvSubBrandTbl, 0, sizeof(RecvSubBrandTbl));
		p = &wrcvbuf[0];
		check_sub_brand_tbl(p, &RecvSubBrandTbl);
		break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	default:
		return;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�u�����h���킹
//[]----------------------------------------------------------------------[]
///	@param[out]		*tbl	�F�u�����h����e�[�u��
///	@param[in]		index	�F��M�u�����h�e�[�u���C���f�b�N�X
///	@param[in]		*recvtbl�F��M�u�����h��ԃe�[�u��
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static
void	set_brand_tbl(EC_BRAND_TBL *tbl, EC_BRAND_TBL *recvtbl)
{
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//	int		i, j;
//	EC_BRAND_CTRL ctrl;
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	
// ��M�f�[�^�`�F�b�N
	// �u�����h���킹�`�F�b�N
	check_brand_tbl(tbl, recvtbl);

// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//���\�[�g�s�v
//// ��M�����u�����h��ԃf�[�^�𑗐M����u�����h�ݒ�f�[�^�Ƀ\�[�g���ăZ�b�g(�u�����h�ԍ��̎Ꮗ�ɕ��ёւ���)
//	for (i = 0; i < (tbl->num-1); i++) {
//		for (j = 0; j < (tbl->num-1-i); j++) {
//			if (tbl->ctrl[j].no > tbl->ctrl[j+1].no) {
//				memcpy(&ctrl, &(tbl->ctrl[j]), sizeof(EC_BRAND_CTRL));
//				memcpy(&(tbl->ctrl[j]), &(tbl->ctrl[j+1]), sizeof(EC_BRAND_CTRL));
//				memcpy(&(tbl->ctrl[j+1]), &ctrl, sizeof(EC_BRAND_CTRL));
//			}
//		}
//	}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
}

//[]----------------------------------------------------------------------[]
///	@brief			�u�����h���킹�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		*tbl		�F���M�����u�����h����e�[�u��
///	@param[in]		*recvtbl	�F��M�u�����h��ԃe�[�u��
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static
void	check_brand_tbl(EC_BRAND_TBL *tbl, EC_BRAND_TBL *recvtbl)
{
	int		i, j;
	int		err = 1;		// 1=�L���u�����h�Ȃ�

// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//// ��M�f�[�^�`�F�b�N
//	if (recvtbl->num == 0) {
//		return;
//	}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j

	tbl->num = 0;
	for (i=0; i<recvtbl->num; i++) {
		if (recvtbl->ctrl[i].no == 0) {
			continue;
		}
		tbl->num++;
		tbl->ctrl[i].no = recvtbl->ctrl[i].no;
		memcpy(&(tbl->ctrl[i].name[0]), &(recvtbl->ctrl[i].name[0]), 20);
		if (isEcBrandNoEnabledForSetting(recvtbl->ctrl[i].no)) {
// MH810103 GG119202(S) �u�����h��Ԕ��菈���ύX
//			if (recvtbl->ctrl[i].status == 0x01) {
			if (isEC_BRAND_STS_KAIKYOKU(recvtbl->ctrl[i].status)) {
// MH810103 GG119202(E) �u�����h��Ԕ��菈���ύX
			// ��M������ԁF�J�ǁA�L������v
				ecRegistBrandError(&recvtbl->ctrl[i], 0);			// �G���[����
				tbl->ctrl[i].status = 0x00;		// �L���A�J��
				err = 0;
			} else {
			// ��M������ԁF�J�ǁA�L���ȊO���s��v
				ecRegistBrandError(&recvtbl->ctrl[i], 1);			// �G���[�o�^
				tbl->ctrl[i].status = 0x11;		// �����A�J�ǋ֎~
			}
		} else {
			tbl->ctrl[i].status = 0x11;		// �����A�J�ǋ֎~
		}
	}

	if (err != 0) {
	// �L���u�����h�Ȃ�
		// �ݒ�ς݂̃u�����h���܂߂ă`�F�b�N����
		for(i = 0; i < _countof(BrandTbl); i++) {
			for(j = 0; j < BrandTbl[i].num; j++) {
				if(BrandTbl[i].ctrl[j].status == 0) {
					err = 0;		// ���ɗL���ȃu�����h���ݒ肳��Ă���
					break;
				}
			}
			if(err == 0) {
				break;
			}
		}
		// �ēx�L���u�����h�̗L�����`�F�b�N
		if(err != 0) {
			Suica_Rec.suica_err_event.BIT.BRAND_NONE = 1;	// �L���u�����h�Ȃ��׸�ON
		} else {
			Suica_Rec.suica_err_event.BIT.BRAND_NONE = 0;	// �L���u�����h�Ȃ��׸�OFF
		}
	} else {
	// ��ł��u�����h�X�e�[�^�X����v
		Suica_Rec.suica_err_event.BIT.BRAND_NONE = 0;	// �L���u�����h�Ȃ��׸�OFF
	}
// MH810103 GG119202(S) �N���V�[�P���X�s��C��
//	if (ECCTL.phase == EC_PHASE_BRAND) {
//	// ���u�����h���킹�������A�t�F�[�Y���u�����h���킹���Ȃ�t�F�[�Y��ς���
//		ECCTL.phase = EC_PHASE_PAY;						// �u�����h���킹������
//		ECCTL.step = 0;
//	}
// MH810103 GG119202(E) �N���V�[�P���X�s��C��
}

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ��i�d�l�ύX�j
////[]----------------------------------------------------------------------[]
/////	@brief			���σ��[�_ �T�u�u�����h�g�p�ݒ蔻��
////[]----------------------------------------------------------------------[]
/////	@param[in]		brand_no		: �u�����h�ԍ�
/////	@param[in]		sub_brand_no	: �T�u�u�����h���
/////	@return			0:����`�T�u�u�����h�A1:���g�p�ݒ�T�u�u�����h�A2:�g�p�ݒ�T�u�u�����h
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2021/11/26<br>
/////					Update	:
////[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
//uchar isSubBrandNoEnabledForSetting(const ushort brand_no, const uchar sub_brand_no)
//{
//	uchar	ret = 0;
//	uchar	tbl_cnt;
//	ushort	i;
//	const EC_SUB_BRAND_ENABLED_PARAM_TBL *tbl;
//
//
//	if( isEC_USE() == 0 ){
//		return ret;
//	}
//
//	switch (brand_no) {
//	case BRANDNO_QR:
//		tbl = QrSubBrandEnabledParamTbl;
//		tbl_cnt = TBL_CNT(QrSubBrandEnabledParamTbl);
//		break;
//	default:
//		return ret;
//	}
//
//	// ���σ��[�_�[���瑗���Ă����T�u�u�����h���g���Ă�����0�A�g���ĂȂ�������1��Ԃ�
//	for (i = 0; i < tbl_cnt; i++, tbl++) {
//		if (sub_brand_no == tbl->brand_no) {
//			if (prm_get(COM_PRM, S_ECR, (short)tbl->address, 1, (char)tbl->pos) == 1) {
//				// �g�p�ݒ�T�u�u�����h
//				ret = 2;
//			}
//			else {
//				// ���g�p�ݒ�T�u�u�����h
//				ret = 1;
//			}
//			break;
//		}
//	}
//	return ret;
//}
//
////[]----------------------------------------------------------------------[]
/////	@brief			�T�u�u�����h�e�[�u���`�F�b�N�imore�j
////[]----------------------------------------------------------------------[]
/////	@param[in]		recv_tbl	: �T�u�u�����h�e�[�u��
/////	@return			0:�e�[�u���Ɏg�p�ݒ�T�u�u�����h�S�Ă���
/////					1:�e�[�u���ɖ��g�p�ݒ�T�u�u�����h����
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2022/03/30
////[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
//static uchar check_sub_brand_tbl_for_more(EC_SUB_BRAND_TBL *recv_tbl)
//{
//	ushort	i, j;
//	ushort	brand_no;
//	uchar	sub_brand_no;
//	uchar	ofs;
//	uchar	tbl_sts;
//
//
//	ofs = 0;
//	tbl_sts = 0;
//	for (i = 0; i < recv_tbl->brand_num; i++) {
//		// �u�����h�ԍ�
//		brand_no = recv_tbl->brand_no[i];
//
//		for (j = 0; j < recv_tbl->sub_brand_num[i]; j++) {
//			sub_brand_no = recv_tbl->sub_brand_no[ofs+j];
//			if (isSubBrandNoEnabledForSetting(brand_no, sub_brand_no) != 2) {
//				// ���g�p�ݒ�A�܂��́A����`�T�u�u�����h
//				err_chk((char)jvma_setup.mdl, ERR_EC_SUB_BRAND_FAILURE, 1, 0, 0);	// �G���[�o�^
//				tbl_sts = 1;
//				break;
//			}
//		}
//
//		if (tbl_sts != 0) {
//			// �G���[�����ς݂ł���΁A���[�v�𔲂���
//			break;
//		}
//
//		// �I�t�Z�b�g�X�V
//		ofs += recv_tbl->sub_brand_num[i];
//	}
//
//	return tbl_sts;
//}
//
////[]----------------------------------------------------------------------[]
/////	@brief			�T�u�u�����h�e�[�u���`�F�b�N�iless�j
////[]----------------------------------------------------------------------[]
/////	@param[in]		recv_tbl	: �T�u�u�����h�e�[�u��
/////	@param[in]		chk_brand	: �����u�����h�ԍ�
/////	@param[in]		chk_tbl		: �����T�u�u�����h�ݒ�e�[�u��
/////	@param[in]		chk_tbl_cnt	: �����T�u�u�����h�ݒ�e�[�u������
/////	@return			0:�g�p����T�u�u�����h��S�Ď�M�����A
/////					2:�g�p����T�u�u�����h����M���Ă��Ȃ�
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2022/03/30
////[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
//static uchar check_sub_brand_tbl_for_less(EC_SUB_BRAND_TBL *recv_tbl, ushort chk_brand,
//											const EC_SUB_BRAND_ENABLED_PARAM_TBL *chk_tbl,
//											uchar chk_tbl_cnt)
//{
//	ushort	i, j, k;
//	uchar	sub_brand_no;
//	uchar	ofs;
//	uchar	chk;
//	uchar	tbl_sts;
//
//
//	// �u�����h�ԍ��`�F�b�N
//	chk = 0;
//	tbl_sts = 0;
//	for (k = 0; k < recv_tbl->brand_num; k++) {
//		// �u�����h�ԍ�
//		if (chk_brand == recv_tbl->brand_no[k]) {
//			chk = 1;
//			break;
//		}
//	}
//
//	if (chk == 0) {
//		// �u�����h�ԍ��Ȃ�
//		return 1;
//	}
//
//	// �I�t�Z�b�g���v�Z
//	ofs = 0;
//	for (i = 0; i < k; i++) {
//		ofs += recv_tbl->sub_brand_num[i];
//	}
//
//	// �����T�u�u�����h�ݒ�e�[�u���Ǝ�M�T�u�u�����h�e�[�u�����r����
//	for (i = 0; i < chk_tbl_cnt; i++, chk_tbl++) {
//		if (prm_get(COM_PRM, S_ECR, (short)chk_tbl->address, 1, (char)chk_tbl->pos) == 0) {
//			// �T�u�u�����h�ݒ肪�Ȃ�
//			continue;
//		}
//
//		chk = 0;
//		for (j = 0; j < recv_tbl->sub_brand_num[k]; j++) {
//			sub_brand_no = recv_tbl->sub_brand_no[ofs+j];
//			if (chk_tbl->brand_no == sub_brand_no) {
//				chk = 1;
//				break;
//			}
//		}
//
//		if (chk == 0) {
//			// �g�p�ݒ肵���T�u�u�����h����M���Ă��Ȃ�
//			err_chk((char)jvma_setup.mdl, ERR_EC_SUB_BRAND_FAILURE, 1, 0, 0);	// �G���[�o�^
//			tbl_sts = 2;
//			break;
//		}
//	}
//
//	return tbl_sts;
//}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ��i�d�l�ύX�j

//[]----------------------------------------------------------------------[]
///	@brief			�T�u�u�����h���킹�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		data		: �T�u�u�����h�ꗗ�f�[�^
///	@param[in]		recv_tbl	: �T�u�u�����h�e�[�u��
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/31<br>
///					Update
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static void	check_sub_brand_tbl(uchar *data, EC_SUB_BRAND_TBL *recv_tbl)
{
	ushort	i, j;
	uchar	idx;
	uchar	ofs;

	recv_tbl->tbl_sts.BYTE = 0;
	// �T�u�u�����h�e�[�u����M
	recv_tbl->tbl_sts.BIT.RECV = 1;
	// �u�����h���Z�b�g
	idx = 0;
	recv_tbl->brand_num = data[idx++];

	ofs = 0;
	for (i = 0; i < recv_tbl->brand_num; i++) {
		// �u�����h�ԍ��Z�b�g
		recv_tbl->brand_no[i] = bcdbin3(&data[idx]);
		idx += 2;

		// �T�u�u�����h���Z�b�g
		recv_tbl->sub_brand_num[i] = data[idx++];

		// �T�u�u�����h��ʃZ�b�g
		for (j = 0; j < recv_tbl->sub_brand_num[i]; j++) {
			recv_tbl->sub_brand_no[ofs+j] = data[idx++];
		}

		// �I�t�Z�b�g�X�V
		ofs += recv_tbl->sub_brand_num[i];
	}

// MH810105(S) MH364301 QR�R�[�h���ϑΉ��i�d�l�ύX�j
//	// E3279
//	// ���Z�@�Ŏg�p�ݒ肵�Ă��Ȃ��T�u�u�����h����M����(more)
//	recv_tbl->tbl_sts.BIT.MORE_LESS = check_sub_brand_tbl_for_more(recv_tbl);
//	if (recv_tbl->tbl_sts.BIT.MORE_LESS != 0) {
//		// �G���[�����ς݂ł���΁Aless�̃`�F�b�N�Ȃ�
//		return;
//	}
//
//	// ���Z�@�Őݒ肵���g�p�ݒ肵���T�u�u�����h����M���Ȃ�����(less)
//	// QR�R�[�h����
//	recv_tbl->tbl_sts.BIT.MORE_LESS = check_sub_brand_tbl_for_less(recv_tbl, BRANDNO_QR,
//																	QrSubBrandEnabledParamTbl,
//																	(uchar)TBL_CNT(QrSubBrandEnabledParamTbl));
//
//	if (recv_tbl->tbl_sts.BIT.MORE_LESS == 0) {
//		// more�Aless�̃`�F�b�NOK�̂��߁A�G���[����
//		err_chk((char)jvma_setup.mdl, ERR_EC_SUB_BRAND_FAILURE, 0, 0, 0);	// �G���[����
//	}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ��i�d�l�ύX�j
}

//[]----------------------------------------------------------------------[]
///	@brief			�T�u�u�����h�����u�����h�ԍ��`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no	: �u�����h�ԍ�
///	@return			-1:�T�u�u�����h�������Ȃ��A
///					 0:�T�u�u�����h�Ȃ��A
///					 1�`:�T�u�u�����h�C���f�b�N�X
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/31
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
int EcSubBrandCheck(ushort brand_no)
{
	uchar	i;
	uchar	chk;

	// �T�u�u�����h�����u�����h�ԍ����H
	chk = 0;
	for (i = 0; i < TBL_CNT(EcSubBrandUseTbl); i++) {
		if (brand_no == EcSubBrandUseTbl[i]) {
			chk = 1;
			break;
		}
	}

	if (chk == 0) {
		// �T�u�u�����h�������Ȃ�
		return -1;
	}

	// �T�u�u�����h�e�[�u�����猟��
	for (i = 0; i < RecvSubBrandTbl.brand_num; i++) {
		// �u�����h�ԍ�����v�A���A�T�u�u�����h������
		if (brand_no == RecvSubBrandTbl.brand_no[i] &&
			RecvSubBrandTbl.sub_brand_num[i] > 0) {
			return (int)(i+1);
		}
	}

	// �T�u�u�����h�Ȃ�
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			�T�u�u�����h���擾
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no	: �u�����h�ԍ�
///	@param[in]		kind		: 0:�T�u�u�����h�e�[�u�����猟���A
///								  1:�T�u�u�����h���O���X�g����擾�A
///	@param[in]		idx			: kind=0 �T�u�u�����h�C���f�b�N�X�A
///								  kind=1 �T�u�u�����h���
///	@param[in,out]	buff		: �T�u�u�����h��
///	@return			�T�u�u�����h���̒���
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/31
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
int EcGetSubBrandName(ushort brand_no, uchar kind, uchar idx, uchar *buff)
{
	uchar	i;
	uchar	ofs;
	ushort	sub_brand_no = 0;
	const uchar	**list;
	uchar	list_cnt;
	int		len = 0;


	switch (brand_no) {
	case BRANDNO_QR:
		list = sub_brand_name_list_QR;
		list_cnt = QR_PAY_KIND_MAX;
		break;
	default:
		return 0;
	}


	// �T�u�u�����h�e�[�u�����猟��
	if (kind == 0) {
		ofs = 0;
		for (i = 0; i < RecvSubBrandTbl.brand_num; i++) {
			// �u�����h�ԍ�
			if (brand_no == RecvSubBrandTbl.brand_no[i]) {
				sub_brand_no = RecvSubBrandTbl.sub_brand_no[ofs+idx];
				break;
			}

			// �I�t�Z�b�g�X�V
			ofs += RecvSubBrandTbl.sub_brand_num[i];
		}

		if (sub_brand_no == 0) {
			// �T�u�u�����h��ʂO�͂Ȃ�
			return 0;
		}
	}
	// �T�u�u�����h���O���X�g����擾
	else if (kind == 1) {
		sub_brand_no = idx;
	}


	if (sub_brand_no >= list_cnt) {
		sub_brand_no = 0;
	}
	strcpy((char*)buff, (char*)list[sub_brand_no]);
	len = (int)strlen((char*)buff);

	return len;
}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

// MH810103 GG119202(S) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
//[]----------------------------------------------------------------------[]
///	@brief			�u�����h�e�[�u���X�V
//[]----------------------------------------------------------------------[]
///	@param[in]		*srctbl		�F���Z�@���ێ����Ă���u�����h�e�[�u��
///	@param[in]		*recvtbl	�F��M�u�����h��ԃe�[�u��
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/07/09<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void update_brand_tbl(EC_BRAND_TBL *srctbl, EC_BRAND_TBL *recvtbl)
{
	ushort	i, j;

	if (memcmp(srctbl, recvtbl, sizeof(EC_BRAND_TBL)) == 0) {
		// ��v����ꍇ�͉������Ȃ�
		return;
	}

	// �u�����h�ǉ��^�폜�`�F�b�N
	for (i = 0; i < srctbl->num; i++) {
		for (j = 0; j < recvtbl->num; j++) {
			// �u�����hNo.����v���邩�H
			if (srctbl->ctrl[i].no == recvtbl->ctrl[j].no) {
				// �q�b�g�����̂�reserved�ɂ��邵������
				recvtbl->ctrl[j].reserved = 1;
				break;
			}
		}
		if (j >= recvtbl->num) {
			// @todo �u�����h�폜���L�^
		}
	}

	for (i = 0; i < recvtbl->num; i++) {
		// reserved��1���H
		if (recvtbl->ctrl[i].reserved == 0) {
			// @todo �u�����h�ǉ����L�^
		}
		recvtbl->ctrl[i].reserved = 0;
	}

	// �u�����h�e�[�u�����X�V(��M�����u�����h�e�[�u���ŏ㏑��)
	nmisave(srctbl, recvtbl, (short)sizeof(EC_BRAND_TBL));
}
// MH810103 GG119202(E) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���

//[]----------------------------------------------------------------------[]
///	@brief			���σf�[�^�Z�b�g
//[]----------------------------------------------------------------------[]
///	@param[out]		*data	:	�f�[�^�Z�b�g�\����
///	@param[in]		*buf	:	��M�f�[�^
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
///	@param[in]		errchk	:	�ڍ׃G���[�R�[�h�o�^�L���i0=�G���[�o�^���Ȃ��@1=�G���[�o�^����j
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
//void	ec_settlement_data_save(EC_SETTLEMENT_RES *data, uchar *buf)
void	ec_settlement_data_save(EC_SETTLEMENT_RES *data, uchar *buf, uchar errchk)
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
{
	uchar	brand_index = convert_brandno( bcdbin3(&buf[2]) );
	ushort	brand_no;
	uchar	*p = buf;
// MH810105 GG119202(S) ������������W�v�d�l���P
	ulong	work;
// MH810105 GG119202(E) ������������W�v�d�l���P

	// ����f�[�^�p�̍�Ɨ̈��������
	memset(data, 0, sizeof(EC_SETTLEMENT_RES));
	memset(data->Card_ID, 0x20, sizeof(data->Card_ID));

	data->Result					= *p++;										// ���ό���
	data->Column_No					= *p++;										// ���σR�����ԍ�
	data->brand_no					= bcdbin3(p);								// �u�����h�ԍ�
	p += 2;
	data->settlement_data			= settlement_amount_ex(p, 3);				// ���ϊz
	p += 3;
	data->settlement_data_before	= settlement_amount_ex(p, 3);				// ���ϑO�c��
	p += 3;
	data->settlement_data_after		= settlement_amount_ex(p, 3);				// ���ό�c��
	p += 3;
	memcpy(data->Card_ID, p, sizeof(data->Card_ID));							// �J�[�hID
	p += 20;

	// ���ώ���
	data->settlement_time.seco		= bcdbin(*p++);
	data->settlement_time.minu		= bcdbin(*p++);
	data->settlement_time.hour		= bcdbin(*p++);
	data->settlement_time.week		= bcdbin(*p++);
	data->settlement_time.date		= bcdbin(*p++);
	data->settlement_time.mont		= bcdbin(*p++);
	data->settlement_time.year		= bcdbin3(p);
	p += 2;

// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
//	// �ڍ׃G���[�R�[�h
//	memcpy(Ec_Settlement_Rec_Code, p, sizeof(Ec_Settlement_Rec_Code));			// �ڍ׃G���[�R�[�h
//	memset(err_wk, 0, sizeof(err_wk));
//	memcpy(err_wk, "�ڍ׃R�[�h000", 13);
//	memcpy(&err_wk[10], Ec_Settlement_Rec_Code, sizeof(Ec_Settlement_Rec_Code));
//// GG119202(S) ���σ��[�_�֘A���菈��������
////	ec_Disp_No = 0;																// ���ό��ʏڍ׃G���[�R�[�h�̈ē��\���p�^�[��
//	ECCTL.ec_Disp_No = 0;														// ���ό��ʏڍ׃G���[�R�[�h�̈ē��\���p�^�[��
//// GG119202(E) ���σ��[�_�֘A���菈��������
//
//	// �ڍ׃G���[�R�[�h�� 000 �ł͂Ȃ�
//	if (!(Ec_Settlement_Rec_Code[0] == '0' &&
//		  Ec_Settlement_Rec_Code[1] == '0' &&
//		  Ec_Settlement_Rec_Code[2] == '0')) {
//// GG119202(S) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
////			ec_errcode_err_chk(err_wk, Ec_Settlement_Rec_Code);					// �ڍ׃G���[�o�^
//			ec_errcode_err_chk(err_wk, Ec_Settlement_Rec_Code, brand_index);	// �ڍ׃G���[�o�^
//// GG119202(E) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
//	}
//	// �ڍ׃G���[�R�[�h201�ł͂Ȃ� && �u�����h�ԍ��� 0
//	if(!(Ec_Settlement_Rec_Code[0] == '2' &&
//		 Ec_Settlement_Rec_Code[1] == '0' &&
//		 Ec_Settlement_Rec_Code[2] == '1') &&
//		 brand_index == EC_ZERO_USED){
//			err_chk((char)jvma_setup.mdl, ERR_EC_BRANDNO_ZERO, 1, 0, 0); 		// E3265�o�^
//	}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	if (data->brand_no == BRANDNO_QR) {
		// �ڍ׃G���[�ŃT�u�u�����h���g�p���邽�߁A�G���[�`�F�b�N������ɕۑ�
		data->Brand.Qr.PayKind = *(p+3);										// �x�����
	}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	memcpy(Ec_Settlement_Rec_Code, p, sizeof(Ec_Settlement_Rec_Code));			// �ڍ׃G���[�R�[�h
// MH810105 GG119202(S) �ē��\���p�^�[�����N���A����
	ECCTL.ec_Disp_No = 0;														// ���ό��ʏڍ׃G���[�R�[�h�̈ē��\���p�^�[��
// MH810105 GG119202(E) �ē��\���p�^�[�����N���A����
// MH810105(S) MH364301 PiTaPa�Ή�
	if(data->brand_no == BRANDNO_PITAPA){
		memcpy(&data->Brand.Pitapa.Reason_Code, p+74, sizeof(data->Brand.Pitapa.Reason_Code));	// ���R�R�[�h(6�o�C�g)
	}
// MH810105(E) MH364301 PiTaPa�Ή�
	// errchk=1�̏ꍇ�ڍ׃G���[�o�^����
	if( errchk ){
		EcErrCodeChk( Ec_Settlement_Rec_Code, brand_index );
	}
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
	p += 3;

	brand_no = data->brand_no;
	// �s���u�����h�H || �u�����h�ԍ�0�H
	if( brand_index == EC_UNKNOWN_USED || brand_index == EC_ZERO_USED ){
		// �I�����i�f�[�^�ő��M�����u�����h�ŌŗL�f�[�^���Z�b�g����
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//		brand_no = Product_Select_Brand;
		brand_no = ECCTL.Product_Select_Brand;
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
	}
// MH810105 GG119202(S) ������������W�v�d�l���P
	if (ECCTL.ec_Disp_No == 96) {
		// ����������m�F����R0252��o�^����
		memset(moni_wk, 0, sizeof(moni_wk));
		moni_wk[0] = (uchar)(brand_index - EC_USED + 1);	// �J�[�h���
		memcpyFlushRight(Card_Work, data->Card_ID, sizeof(Card_Work), sizeof(data->Card_ID));
		memcpy(&moni_wk[1], &Card_Work[16], 4);				// �J�[�h�ԍ��̉��S�����Z�b�g
		moni_wk[5] = data->Result;							// ���ό���
		work = data->settlement_data_before;				// ���ϑO�c�z
		if (work > 0xffff) {
		// ���ϑO�c�z��short�l�ȏ�Ȃ�␳
			work = 0xffff;
		}
		moni_wk[6] = (uchar)(work & 0xff);
		moni_wk[7] = (uchar)((work >> 8) & 0xff);
		moni_wk[8] = moni_wk[6];							// ���ό�c���i���ϑO�c���Ɠ������z���Z�b�g����j
		moni_wk[9] = moni_wk[7];
		wmonlg(OPMON_EC_SETTLEMENT, moni_wk, 0);			// ���j�^�o�^
	}
// MH810105 GG119202(E) ������������W�v�d�l���P
	// �d�q�}�l�[���σf�[�^���
	switch (brand_no) {
// MH810103 GG119202(S) �d�q�}�l�[�Ή�
	case	BRANDNO_KOUTSUU:
	// ��ʌn
		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));					// �⍇���ԍ�(15�o�C�g���̂܂܃R�s�[)
		p += 15;
// MH810103 GG119202(S) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
// MH810105(S) MH364301 �s�v�ϐ��폜
//		memcpy(&data->Termserial_No, p, sizeof(data->Termserial_No));				// �[�����ʔԍ�(30�o�C�g���̂܂܃R�s�[)
// MH810105(E) MH364301 �s�v�ϐ��폜
		p += 30;
		memcpy(&data->Brand.Koutsuu.SPRW_ID, p, sizeof(data->Brand.Koutsuu.SPRW_ID));	// SPRWID(13�o�C�g)
		p += 13;
		memcpy(&data->Brand.Koutsuu.Kamei, p, sizeof(data->Brand.Koutsuu.Kamei));		// �����X��(30�o�C�g)
		p += 30;
		data->Brand.Koutsuu.TradeKind = *p++;										// ������(1�o�C�g)
// MH810103 GG119202(E) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
		break;
// MH810103 GG119202(E) �d�q�}�l�[�Ή�
	case	BRANDNO_EDY:
	// Edy
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//		memcpy(&data->Brand.Edy.DealNo, p, 4);										// ����ʔ�(4�o�C�g���̂܂܃R�s�[)
//		p += 4;
//		memcpy(&data->Brand.Edy.CardDealNo, p, 4);									// �J�[�h����ʔ�(4�o�C�g���̂܂܃R�s�[)
//		p += 4;
//		memcpy(&data->Brand.Edy.TerminalNo, p, 8);									// �[���ԍ�(8�o�C�g���̂܂܃R�s�[)
//		p += 8;
//		memset(EcEdyTerminalNo, 0, sizeof(EcEdyTerminalNo));
//		sprintf(EcEdyTerminalNo, "%08lX%08lX"
//			, data->Brand.Edy.TerminalNo[0], data->Brand.Edy.TerminalNo[1]);		// ��ʒ[��ID�i16�i���F16���A0�p�f�B���O�j
		memcpy(&data->Brand.Edy.DealNo, p, 10);										// ����ʔ�(10�o�C�g���̂܂܃R�s�[)
		p += 10;
		memcpy(&data->Brand.Edy.CardDealNo, p, 5);									// �J�[�h����ʔ�(5�o�C�g���̂܂܃R�s�[)
		p += 5;
		memcpy(&data->Brand.Edy.TerminalNo, p, 8);									// ��ʒ[��ID(8�o�C�g���̂܂܃R�s�[)
		p += 8;
// MH810105 GG119202(S) Edy��ʒ[��ID��������
		if (data->Result == EPAY_RESULT_OK ||
			data->Result == EPAY_RESULT_MIRYO_ZANDAKA_END) {
// MH810105 GG119202(E) Edy��ʒ[��ID��������
		memset(EcEdyTerminalNo, 0, sizeof(EcEdyTerminalNo));
		memcpy(EcEdyTerminalNo, data->Brand.Edy.TerminalNo, sizeof(data->Brand.Edy.TerminalNo));	// ��ʒ[��ID
// MH810105 GG119202(S) Edy��ʒ[��ID��������
		}
// MH810105 GG119202(E) Edy��ʒ[��ID��������
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
// MH810103 GG119202(S) �d�q�}�l�[�Ή�
		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));					// �⍇���ԍ�(15�o�C�g���̂܂܃R�s�[)
		p += 15;
// MH810103 GG119202(E) �d�q�}�l�[�Ή�
// MH810103 GG119202(S) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
// MH810105(S) MH364301 �s�v�ϐ��폜
//		memcpy(&data->Termserial_No, p, sizeof(data->Termserial_No));				// �[�����ʔԍ�(30�o�C�g���̂܂܃R�s�[)
// MH810105(E) MH364301 �s�v�ϐ��폜
		p += 30;
// MH810103 GG119202(E) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
		memcpy(&data->Brand.Edy.Kamei, p, sizeof(data->Brand.Edy.Kamei));			// �����X��(30�o�C�g)
		p += 30;
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
		break;
	case	BRANDNO_NANACO:
	// nanaco
// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//		data->Brand.Nanaco.TerminalNo[0] = settlement_amount_ex(p, 4);				// ��ʒ[��ID�@
//		p += 4;
//		data->Brand.Nanaco.TerminalNo[1] = settlement_amount_ex(p, 4);				// ��ʒ[��ID�A
//		p += 4;
//		data->Brand.Nanaco.TerminalNo[2] = settlement_amount_ex(p, 4);				// ��ʒ[��ID�B
//		p += 4;
//		memcpy(&data->Brand.Nanaco.DealNo, p, 4);									// ��ʎ���ʔ�(4�o�C�g���̂܂܃R�s�[)
//		p += 4;
//// GG119202(S) �d�q�}�l�[�Ή�
//		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));					// �⍇���ԍ�(15�o�C�g���̂܂܃R�s�[)
//		p += 15;
//// GG119202(E) �d�q�}�l�[�Ή�
//// GG119202(S) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
//		memcpy(&data->Termserial_No, p, sizeof(data->Termserial_No));				// �[�����ʔԍ�(30�o�C�g���̂܂܃R�s�[)
//		p += 30;
//// GG119202(E) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
		memcpy(&data->Brand.Nanaco.Kamei, p, sizeof(data->Brand.Nanaco.Kamei));		// �����X��(30�o�C�g)
		p += 30;
		memcpy(&data->Brand.Nanaco.DealNo, p, sizeof(data->Brand.Nanaco.DealNo));	// �[������ʔ�(6�o�C�g)
		p += 6;
		memcpy(&data->Brand.Nanaco.TerminalNo, p, sizeof(data->Brand.Nanaco.TerminalNo));	// ��ʒ[��ID(20�o�C�g)
		p += 20;
// MH810105(S) MH364301 �s�v�ϐ��폜
//		memcpy(&data->Termserial_No, p, sizeof(data->Termserial_No));				// �[�����ʔԍ�(30�o�C�g)
// MH810105(E) MH364301 �s�v�ϐ��폜
		p += 30;
		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));					// �⍇���ԍ�(15�o�C�g)
		p += 15;
// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
		break;
	case	BRANDNO_WAON:
	// WAON
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));					// �⍇���ԍ�(15�o�C�g���̂܂܃R�s�[)
		p += 15;
// MH810105(S) MH364301 �s�v�ϐ��폜
//		memcpy(&data->Termserial_No, p, sizeof(data->Termserial_No));				// �[�����ʔԍ�(30�o�C�g���̂܂܃R�s�[)
// MH810105(E) MH364301 �s�v�ϐ��폜
		p += 30;
		memcpy(&data->Brand.Waon.SPRW_ID, p, sizeof(data->Brand.Waon.SPRW_ID));		// SPRWID(13�o�C�g)
		p += 13;
		memcpy(&data->Brand.Waon.Kamei, p, sizeof(data->Brand.Waon.Kamei));			// �����X��(30�o�C�g)
		p += 30;
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
		memcpy(&data->Brand.Waon.GetPoint, p, 4);									// ����|�C���g(4�o�C�g���̂܂܃R�s�[)
		p += 4;
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//		memcpy(&data->Brand.Waon.TotalPoint, p, 4);									// �݌v�|�C���g(4�o�C�g���̂܂܃R�s�[)
//		p += 4;
//		data->Brand.Waon.CardType		= settlement_amount_ex(p, 4);				// �J�[�h����
//		p += 4;
//		memcpy(&data->Brand.Waon.PeriodPoint, p, 4);								// �Q�N�O�܂łɊl�������|�C���g(4�o�C�g���̂܂܃R�s�[)
//		p += 4;
//		data->Brand.Waon.Period			= settlement_amount_ex(p, 4);				// �Q�N�O�܂łɊl�������|�C���g�̗L������
//		p += 4;
//		memcpy(&data->Brand.Waon.SPRW_ID, p, 14);									// SPRWID
//		p += 14;
//		data->Brand.Waon.PointStatus	= *p++;										// ���p��
//		data->Brand.Waon.DealCode		= *p++;										// �����ʃR�[�h
//// GG119202(S) �d�q�}�l�[�Ή�
//		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));					// �⍇���ԍ�(15�o�C�g���̂܂܃R�s�[)
//		p += 15;
//// GG119202(E) �d�q�}�l�[�Ή�
//// GG119202(S) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
//		memcpy(&data->Termserial_No, p, sizeof(data->Termserial_No));				// �[�����ʔԍ�(30�o�C�g���̂܂܃R�s�[)
//		p += 30;
//// GG119202(E) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
		memcpy(&data->Brand.Waon.TotalPoint, p, 8);									// �݌v�|�C���g(8�o�C�g���̂܂܃R�s�[)
		p += 8;
		data->Brand.Waon.PointStatus = *p++;										// �݌v�|�C���g���b�Z�[�W(1�o�C�g)
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
		break;
	case	BRANDNO_SAPICA:
	// SAPICA
		memcpy(data->Brand.Sapica.Terminal_ID, p, 8);								// ���̒[��ID
		p += 8;
		memcpy(&data->Brand.Sapica.Details_ID, p, 4);								// �ꌏ����ID(4�o�C�g���̂܂܃R�s�[)
		p += 4;
// MH810103 GG119202(S) �d�q�}�l�[�Ή�
		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));					// �⍇���ԍ�(15�o�C�g���̂܂܃R�s�[)
		p += 15;
// MH810103 GG119202(E) �d�q�}�l�[�Ή�
// MH810103 GG119202(S) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
// MH810105(S) MH364301 �s�v�ϐ��폜
//		memcpy(&data->Termserial_No, p, sizeof(data->Termserial_No));				// �[�����ʔԍ�(30�o�C�g���̂܂܃R�s�[)
// MH810105(E) MH364301 �s�v�ϐ��폜
		p += 30;
// MH810103 GG119202(E) ��ʌn�u�����h�̌��ό��ʃf�[�^�t�H�[�}�b�g�C��
		break;
	case	BRANDNO_ID:
	// ID
// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//		memcpy(data->Brand.Sapica.Terminal_ID, p, 8);								// ���̒[��ID
//		p += 8;
//		memcpy(&data->Brand.Sapica.Details_ID, p, 4);								// �ꌏ����ID(4�o�C�g���̂܂܃R�s�[)
//		p += 4;
		memcpy(&data->Brand.Id.Kamei, p, sizeof(data->Brand.Id.Kamei));				// �����X��(30�o�C�g)
		p += 30;
		memcpy(&data->Brand.Id.Approval_No, p, sizeof(data->Brand.Id.Approval_No));	// ���F�ԍ�(7�o�C�g)
		p += 7;
		memcpy(&data->Brand.Id.TerminalNo, p, sizeof(data->Brand.Id.TerminalNo));	// ��ʒ[��ID(13�o�C�g)
		p += 13;
		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));					// �⍇���ԍ�(15�o�C�g)
		p += 15;
// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
		break;
	case	BRANDNO_QUIC_PAY:
	// QUIC_PAY
// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//		memcpy(data->Brand.Sapica.Terminal_ID, p, 8);								// ���̒[��ID
//		p += 8;
//		memcpy(&data->Brand.Sapica.Details_ID, p, 4);								// �ꌏ����ID(4�o�C�g���̂܂܃R�s�[)
//		p += 4;
		memcpy(&data->Brand.QuicPay.Kamei, p, sizeof(data->Brand.QuicPay.Kamei));	// �����X��(30�o�C�g)
		p += 30;
		memcpy(&data->Brand.QuicPay.Approval_No, p, sizeof(data->Brand.QuicPay.Approval_No));	// ���F�ԍ�(7�o�C�g)
		p += 7;
		memcpy(&data->Brand.QuicPay.TerminalNo, p, sizeof(data->Brand.QuicPay.TerminalNo));	// ��ʒ[��ID(13�o�C�g)
		p += 13;
		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));					// �⍇���ԍ�(15�o�C�g)
		p += 15;
// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
		break;
// MH810105(S) MH364301 PiTaPa�Ή�
	case	BRANDNO_PITAPA:
	// PiTaPa
		memcpy(&data->Brand.Pitapa.Kamei, p, sizeof(data->Brand.Pitapa.Kamei));				// �����X��(30�o�C�g)
		p += 30;
		memcpy(&data->Brand.Pitapa.Slip_No, p, sizeof(data->Brand.Pitapa.Slip_No));			// �`�[�ԍ�(5�o�C�g)
		p += 5;
		memcpy(&data->Brand.Pitapa.TerminalNo, p, sizeof(data->Brand.Pitapa.TerminalNo));	// ��ʒ[��ID(13�o�C�g)
		p += 13;
		memcpy(&data->Brand.Pitapa.Approval_No, p, sizeof(data->Brand.Pitapa.Approval_No));	// ���F�ԍ�(8�o�C�g)
		p += 8;
		memcpy(&data->inquiry_num, p, sizeof(data->inquiry_num));							// �⍇���ԍ�(15�o�C�g)
		p += 15;
		memcpy(&data->Brand.Pitapa.Reason_Code, p, sizeof(data->Brand.Pitapa.Reason_Code));	// ���R�R�[�h(6�o�C�g)
		p += 6;
		break;
// MH810105(E) MH364301 PiTaPa�Ή�
	case	BRANDNO_CREDIT:		// �N���W�b�g�J�[�h����
		data->Brand.Credit.Slip_No = (10000U * bcdbin(*(p+2))) + (100U * bcdbin(*(p+1))) + bcdbin(*p);		// �[�������ʔԁi�`�[�ԍ��j
		p += 3;
		memcpy(data->Brand.Credit.Approval_No, p, 7);								// ���F�ԍ�
		p += 7;
		// 0x20���߁A�R�[�h��3�������l��
// MH321800(S) NT-NET���Z�f�[�^�d�l�ύX
//		memcpyFlushLeft2( &data->Brand.Credit.KID_Code[0], &p[3], sizeof(data->Brand.Credit.KID_Code), 3 );	// KID �R�[�h
		memcpyFlushLeft( &data->Brand.Credit.KID_Code[0], &p[3], sizeof(data->Brand.Credit.KID_Code), 3 );	// KID �R�[�h
// MH321800(E) NT-NET���Z�f�[�^�d�l�ύX
		p += 6;
		memcpy(data->Brand.Credit.Id_No, p, 13);									// �[�����ʔԍ�
		p += 13;
		memcpy(data->Brand.Credit.Credit_Company, p, 24);							// �N���W�b�g�J�[�h��Ж�
		p += 24;
		data->Brand.Credit.Identity_Ptrn = *p++;									// �{�l�m�F�p�^�[��
		break;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	case	BRANDNO_QR:
	// QR�R�[�h
		p += 1;
		// �x����ʃR�[�h
		p += 16;
		memcpy(&data->settlement_data_after, p, sizeof(data->settlement_data_after));	// �c��
		p += 4;
		memcpy(data->QR_Kamei, p, sizeof(data->QR_Kamei));								// �����X��
		p += 30;
		memcpy(data->Brand.Qr.PayTerminalNo, p, sizeof(data->Brand.Qr.PayTerminalNo));	// �x���[��ID
		p += 32;
		memcpy(data->inquiry_num, p, sizeof(data->inquiry_num));						// ����ԍ�
		p += 15;
		memcpy(data->Brand.Qr.MchTradeNo, p, sizeof(data->Brand.Qr.MchTradeNo));		// Mch����ԍ�
		p += 32;
		break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	default	:
	// UnKnown
		break;
	}
// MH810100(S) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
	// ���ό��ʏ��_LCD_ICC_SETTLEMENT_RSLT��OpeTask�֑��M
// MH810103 MH810103(s) �d�q�}�l�[�Ή� #5592 �y�d�q�}�l�[����Ή��z�w���O������e�v���x�̌��ʂ��W���[�i���L�^����
//	queset(OPETCBNO, LCD_ICC_SETTLEMENT_RSLT, sizeof(unsigned char), &data->Result);		// ���ό��ʏ��
	if(errchk){
		// ���O�������Ă΂ꂽ�ꍇ�ȊO�ɑ��M����
// MH810105(S) #6199 ���Ϗ������̏�Q�����ł݂Ȃ����ρ{���ό���OK��M���A50-0014�̐ݒ�𖳎����ď�������������
//		queset(OPETCBNO, LCD_ICC_SETTLEMENT_RSLT, sizeof(unsigned char), &data->Result);		// ���ό��ʏ��
		// �݂Ȃ��̎��́AEcDeemedSettlement�ŏ���������
		if (ECCTL.Ec_Settlement_Sts != EC_SETT_STS_DEEMED_SETTLEMENT) {
			queset(OPETCBNO, LCD_ICC_SETTLEMENT_RSLT, sizeof(unsigned char), &data->Result);		// ���ό��ʏ��
		}
// MH810105(E) #6199 ���Ϗ������̏�Q�����ł݂Ȃ����ρ{���ό���OK��M���A50-0014�̐ݒ�𖳎����ď�������������
	}
// MH810103 MH810103(e) �d�q�}�l�[�Ή� #5592 �y�d�q�}�l�[����Ή��z�w���O������e�v���x�̌��ʂ��W���[�i���L�^����
// MH810100(E) Y.Watanabe 2019/11/15 �Ԕԃ`�P�b�g���X((LCD_IF�Ή�)_�ǉ�))
}

//[]----------------------------------------------------------------------[]
///	@brief			�u�����h�ԍ����u�����h�C���f�b�N�X�ϊ�
//[]----------------------------------------------------------------------[]
///	@param[in]		brandno	:	�u�����h�ԍ�(bin)
///	@return			�u�����h�C���f�b�N�X(EC_EDY_USED�`)
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar	convert_brandno(const ushort brandno)
{
	uchar	index;

	switch (brandno) {
	case BRANDNO_ZERO:
		index = EC_ZERO_USED;
		break;
	case BRANDNO_KOUTSUU:
		index = EC_KOUTSUU_USED;
		break;
	case BRANDNO_EDY:
		index = EC_EDY_USED;
		break;
	case BRANDNO_NANACO:
		index = EC_NANACO_USED;
		break;
	case BRANDNO_WAON:
		index = EC_WAON_USED;
		break;
	case BRANDNO_SAPICA:
		index = EC_SAPICA_USED;
		break;
	case BRANDNO_ID:
		index = EC_ID_USED;
		break;
	case BRANDNO_QUIC_PAY:
		index = EC_QUIC_PAY_USED;
		break;
// MH810105(S) MH364301 PiTaPa�Ή�
	case BRANDNO_PITAPA:
		index = EC_PITAPA_USED;
		break;
// MH810105(E) MH364301 PiTaPa�Ή�
	case BRANDNO_CREDIT:
		index = EC_CREDIT_USED;
		break;
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	case BRANDNO_TCARD:
		index = EC_TCARD_USED;
		break;
	case BRANDNO_HOUJIN:
		index = EC_HOUJIN_USED;
		break;
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	case BRANDNO_QR:
		index = EC_QR_USED;
		break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	default:	// unknown
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		// @todo �n�E�X�J�[�h�͎b��
		if (brandno >= BRANDNO_HOUSE_S && brandno <= BRANDNO_HOUSE_E) {
			index = EC_HOUSE_USED;
			break;
		}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		index = EC_UNKNOWN_USED;
		break;
	}

	return index;
}

// MH810103 GG119202(S) �s�v�֐��폜
////[]----------------------------------------------------------------------[]
/////	@brief			����f�[�^�F��t���v��
////[]----------------------------------------------------------------------[]
/////	@param[in]		msg		:	�C�x���g���b�Z�[�W
/////	@return			void
////[]----------------------------------------------------------------------[]
/////	@author			G.So
/////	@date			Create	:	2019/02/07<br>
/////					Update
/////	@file			ope_ec_ctrl.c
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//void	ec_usable_req(ushort msg)
//{
//	if (ECCTL.phase != EC_PHASE_PAY) {
//	// ���Z���ȊO�͗v���s��
//		return;
//	}
//	if (OPECTL.Mnt_mod != 0) {
//	// �����e�i���X��
//		return;
//	}
//	else {
//	// �^�p
//		if (OPECTL.Ope_mod == 100 || OPECTL.Ope_mod == 110) {
//		// �x�ƁE�x��A���[���̏ꍇ
//			return;
//		}
//	}
//
//	if ((Suica_Rec.Data.BIT.CTRL == 0)
//	&&	(Suica_Rec.Data.BIT.OPE_CTRL == 0)) {
//	// ��t�s��� && ��t���v�����Ă��Ȃ�
//		Ec_Pri_Data_Snd(S_CNTL_DATA, 0x01);		// �d�q�}�l�[���p���ɂ��邱�ƂőI�����i�f�[�^�𑗐M����
//	}
//}
// MH810103 GG119202(E) �s�v�֐��폜

//[]----------------------------------------------------------------------[]
///	@brief			��M�f�[�^�҂�����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret: 0:Continue	1:recv wait
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar	ec_split_data_check(void)
{
	uchar	ret = 0;
	uchar	rcv_judge = 0;
	t_Suica_rcv_event	w_rcv_data;

	w_rcv_data.BYTE = Suica_Rec.suica_rcv_event.BYTE;							// ��M�f�[�^�������[�N�G���A�ɃZ�b�g
	if (!w_rcv_data.BYTE) {														// ��M����Ă����������ꍇ
		return 1;																// ��M��͏����͂��Ȃ�
	}

// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	rcv_judge = 0x2A;															// ���Ϗ�ԃf�[�^�A��ԃf�[�^�A���ό��ʃf�[�^
	rcv_judge = SETTLEMENT_COMPLETE_COND;										// ���Ϗ�ԃf�[�^�A��ԃf�[�^�A���ό��ʃf�[�^
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
	w_rcv_data.BYTE &= rcv_judge;												// �\���˗��E���ό��ʁE��ԃf�[�^�̂ݒ��o
	if (!Suica_Rec.Data.BIT.DATA_RCV_WAIT) {									// �҂����킹���H
		if (Suica_Rec.Data.BIT.ADJUSTOR_NOW) {									// CRW-MJA���Z���H(�I�����i�f�[�^���M���ɃZ�b�g�����)
		/* ���σ��[�_�͌��ϏI�����A
			�����`OK���ʂƎ�t�s��
			���s�`NG���ʂƎ���L�����Z���{��t�s��
			�𑗂��Ă���B�i�����I���ANG�I���������l�ł��̓_��SX-10�Ǝ኱�قȂ�j
		*/
		/*	��t�֎~����Ɩ����Ƃ��Ԃ���ƁA���s����ʒm���Ă���B
			�����I�����ɁA���ό��ʁA��ԃf�[�^�A���ό��ʃf�[�^�������Ă��A��ԃf�[�^���܂������{���s��
			�ɂȂ��Ă���Ƃ�������A�u�����v�̂��ߐ��Z�������ł��Ȃ��Ń��b�N����B����������t�s��
			������̂ő҂����킹��B
		*/
			if (w_rcv_data.BYTE == rcv_judge) {									// �S�Ẵf�[�^������Ƃ�
				if (EC_STATUS_DATA.StatusInfo.MiryoStatus != 0	||				// ���������A
					EC_STATUS_DATA.StatusInfo.Running != 0) {					// ���s���̏ꍇ
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//					w_rcv_data.BIT.STATUS_DATA = 0;								// ��ԃf�[�^�𖳌��ɂ��ĉ��������̂�҂�
					w_rcv_data.EC_BIT.STATUS_DATA = 0;							// ��ԃf�[�^�𖳌��ɂ��ĉ��������̂�҂�
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
																				// �����A���s����EC_STATUS_DATA_WAIT�ɕۑ����Ȃ�
																				// �����������A��ԃf�[�^�����Ŏ��s���Ă��܂��̂��h�~����
				}
			}
			if (w_rcv_data.BYTE == rcv_judge &&									// �S�Ẵf�[�^������
				EC_STATUS_DATA.StatusInfo.ReceptStatus != 0) {					// ��t�s��
			// ���ό��� + ���(��t�s��) + ���Ϗ�ԃf�[�^��M�ς�
				Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;							// CRW-MJA���Z���t���O�N���A
				ret = 0;														// �����I��0���Z�b�g
			}
		/*	WAON���g���Ė����^�C���A�E�g���N�������A���ό��ʂ�NG���ʒm����邪����f�[�^��
			�t�����đ����ė��Ȃ��������Ƃ��������B���ό��ʂ�����Ǝ���f�[�^�Ŗ����I����
			����s�����߁A����f�[�^��҂����킹��B�P�Ȃ钆�~�ł͌��ό��ʂ����Ȃ����߁A��
			�ԃf�[�^�Ŏ�����I������B
		*/
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//			else if ((w_rcv_data.BIT.SETTLEMENT_DATA == 0)						// ���ό��ʂ��Ȃ���
//				&&	 (w_rcv_data.BIT.STATUS_DATA != 0)							// ��ԃf�[�^������Ƃ�
			else if ((w_rcv_data.EC_BIT.SETTLEMENT_DATA == 0)					// ���ό��ʂ��Ȃ���
				&&	 (w_rcv_data.EC_BIT.STATUS_DATA != 0)						// ��ԃf�[�^������Ƃ�
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
				&&	 (EC_STATUS_DATA.StatusInfo.TradeCansel != 0)
				&&	 (EC_STATUS_DATA.StatusInfo.ReceptStatus != 0)) {
			// ��ԃf�[�^��M(����L�����Z�� + ��t�s��)
				Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;							// CRW-MJA���Z���t���O�N���A
				ret = 0;														// �����I��0���Z�b�g
			}
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//			else if ((w_rcv_data.BIT.SETTLEMENT_DATA == 0)						// ���ό��ʂ��Ȃ���
//				&&	 (w_rcv_data.BIT.SETTSTS_DATA != 0) &&						// ���Ϗ�ԃf�[�^������Ƃ�
//					(Ec_Settlement_Sts == EC_SETT_STS_MIRYO)					// ��������
//				&&	 (w_rcv_data.BIT.STATUS_DATA != 0) &&						// ��ԃf�[�^������Ƃ�
			else if ((w_rcv_data.EC_BIT.SETTLEMENT_DATA == 0)					// ���ό��ʂ��Ȃ���
				&&	 (w_rcv_data.EC_BIT.SETTSTS_DATA != 0) &&					// ���Ϗ�ԃf�[�^������Ƃ�
					(isEC_STS_MIRYO())											// ��������
				&&	 (w_rcv_data.EC_BIT.STATUS_DATA != 0) &&					// ��ԃf�[�^������Ƃ�
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
					(EC_STATUS_DATA.StatusInfo.MiryoStatus ||					// ����������
					  EC_STATUS_DATA.StatusInfo.Running)) {						// ���s���̏ꍇ�́A���Z��Ԃ͌p�����AOpeTask�ւ̒ʒm���s��
				ret = 0;														// �����I��0���Z�b�g
			}
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//			else if ((w_rcv_data.BIT.SETTLEMENT_DATA == 0)						// ���ό��ʂ��Ȃ���
//				&&	 (w_rcv_data.BIT.STATUS_DATA != 0) &&						// ��ԃf�[�^������Ƃ�
			else if ((w_rcv_data.EC_BIT.SETTLEMENT_DATA == 0)					// ���ό��ʂ��Ȃ���
				&&	 (w_rcv_data.EC_BIT.STATUS_DATA != 0) &&					// ��ԃf�[�^������Ƃ�
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
// MH810103 GG119202(S) ����{�^���������A����f�[�^�F�֎~�𑗐M���A��ԃf�[�^�̂݉���������Ƃ��A�u������x�����J�[�h���^�b�`���Ă��������v�ƕ\������C��
					 (isEC_CONF_MIRYO() == 0) &&								// ��ԃf�[�^(�����m��)��M���Ă��Ȃ��Ƃ�
// MH810103 GG119202(E) ����{�^���������A����f�[�^�F�֎~�𑗐M���A��ԃf�[�^�̂݉���������Ƃ��A�u������x�����J�[�h���^�b�`���Ă��������v�ƕ\������C��
					  (!EC_STATUS_DATA.StatusInfo.ReceptStatus &&				// ��t������L�����Z����tBIT�������Ă��Ȃ��ꍇ�́A���Z��Ԃ͌p�����AOpeTask�ւ̒ʒm�͍s��Ȃ�
					   !EC_STATUS_DATA.StatusInfo.TradeCansel)) {
				ret = 0;														// �����I��0���Z�b�g
			}
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//			else if ((w_rcv_data.BIT.SETTLEMENT_DATA == 0)						// ���ό��ʂ��Ȃ���
//				&&	 (w_rcv_data.BIT.STATUS_DATA != 0) &&						// ��ԃf�[�^������Ƃ�
			else if ((w_rcv_data.EC_BIT.SETTLEMENT_DATA == 0)					// ���ό��ʂ��Ȃ���
				&&	 (w_rcv_data.EC_BIT.STATUS_DATA != 0) &&					// ��ԃf�[�^������Ƃ�
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
					  EC_STATUS_DATA.StatusInfo.RebootReq) {					// �ċN���v��BIT�������Ă���ꍇ�́A���Z��Ԃ͌p�����AOpeTask�ւ̒ʒm���s��
				ret = 0;														// �����I��0���Z�b�g
			}
// MH810103 GG119202(S) �����d�l�ύX�Ή�
//			else if (w_rcv_data.BIT.SETTLEMENT_DATA != 0 &&						// ���ό��ʂ���
//					 Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO &&			// ���ό���=�����m��
//					 w_rcv_data.BIT.SETTSTS_DATA == 0 &&						// ���Ϗ�ԃf�[�^�Ȃ�
//					 w_rcv_data.BIT.STATUS_DATA == 0) {							// ��ԃf�[�^�Ȃ�
//				ret = 0;														// �����I��0���Z�b�g
//			}
// MH810103 GG119202(E) �����d�l�ύX�Ή�
// MH810103 GG119202(S) �u�����h�I����̃L�����Z�������ύX
//			else if(Suica_Rec.Data.BIT.SLCT_BRND_BACK) {						// �u�����h�I����Ԃ��疢�I���ɂȂ���
//				ret = 0;
//			}
// MH810103 GG119202(E) �u�����h�I����̃L�����Z�������ύX
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
			else if (w_rcv_data.EC_BIT.BRAND_DATA != 0) {
				ret = 0;
			}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810103 GG119202(S) ��ԃf�[�^�i�����m��j��M�̃f�[�^�҂����킹�����C��
			else if ((w_rcv_data.EC_BIT.SETTLEMENT_DATA == 0)					// ���ό��ʂ��Ȃ���
				&&	 (w_rcv_data.EC_BIT.SETTSTS_DATA == 0)						// ���Ϗ�ԃf�[�^���Ȃ���
				&&	 (w_rcv_data.EC_BIT.STATUS_DATA != 0)						// ��ԃf�[�^������Ƃ�
				&&	  isEC_CONF_MIRYO()) {										// ��ԃf�[�^(�����m��)BIT�������Ă���ꍇ
				ret = 0;
			}
// MH810103 GG119202(E) ��ԃf�[�^�i�����m��j��M�̃f�[�^�҂����킹�����C��
			else {
				EC_STATUS_DATA_WAIT.status_data = 0;
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//				if (w_rcv_data.BIT.STATUS_DATA) {								// ��M�����f�[�^������f�[�^�ő҂����킹�̏ꍇ
				if (w_rcv_data.EC_BIT.STATUS_DATA) {							// ��M�����f�[�^������f�[�^�ő҂����킹�̏ꍇ
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
					EC_STATUS_DATA_WAIT.status_data = EC_STATUS_DATA.status_data;	// �������҂����킹�悤�ɃZ�[�u
				}
				Suica_Rec.rcv_split_event.BYTE = w_rcv_data.BYTE;				// �҂����킹�p�̃G���A�ɏ�Ԃ�ۑ�
				Suica_Rec.Data.BIT.DATA_RCV_WAIT = 1;							// �f�[�^��M�҂��t���O�Z�b�g
				ret = 1;														// ��M�����͂��Ȃ��i�҂����킹���j
			}
		}
	} else {
		Suica_Rec.rcv_split_event.BYTE |= w_rcv_data.BYTE;						// ��M�����f�[�^�̏�Ԃ�҂����킹�G���A�ɔ��f

// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//		if (w_rcv_data.BIT.STATUS_DATA) {										// ��M�����f�[�^����ԃf�[�^�̏ꍇ
		if (w_rcv_data.EC_BIT.STATUS_DATA) {									// ��M�����f�[�^����ԃf�[�^�̏ꍇ
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
			EC_STATUS_DATA.status_data |= EC_STATUS_DATA_WAIT.status_data;		// �҂����킹�p�̃f�[�^���}�[�W����
		}

		if (Suica_Rec.rcv_split_event.BYTE == rcv_judge &&						// �S�Ẵf�[�^������
			EC_STATUS_DATA.StatusInfo.ReceptStatus != 0) {							// ��t�s��
		// ���ό��� + ��� + ���Ϗ�Ԏ�M�ς�(���ʂ͋C�ɂ��Ȃ�)
			ret = 0;															// �����I��0���Z�b�g
		}
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//		else if ((Suica_Rec.rcv_split_event.BIT.SETTLEMENT_DATA == 0)			// ���ό��ʂ��Ȃ���
//			&&	 (Suica_Rec.rcv_split_event.BIT.STATUS_DATA != 0)				// ��ԃf�[�^������Ƃ�
		else if ((Suica_Rec.rcv_split_event.EC_BIT.SETTLEMENT_DATA == 0)		// ���ό��ʂ��Ȃ���
			&&	 (Suica_Rec.rcv_split_event.EC_BIT.STATUS_DATA != 0)			// ��ԃf�[�^������Ƃ�
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
			&&	 (EC_STATUS_DATA.StatusInfo.TradeCansel != 0)
			&&	 (EC_STATUS_DATA.StatusInfo.ReceptStatus != 0)) {
		// ��ԃf�[�^��M(����L�����Z�� + ��t�s��)
			ret = 0;															// �����I��0���Z�b�g
		}
		else {
			ret = 1;															// ��M�����͂��Ȃ��i�҂����킹���j
		}
		if (ret == 0) {
			Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;								// CRW-MJA���Z���t���O�N���A
			Suica_Rec.suica_rcv_event.BYTE = Suica_Rec.rcv_split_event.BYTE;	// ��M����p�̃G���A���X�V
			Suica_Rec.Data.BIT.DATA_RCV_WAIT = 0;								// �҂����킹�t���O�N���A
		/* DATA_RCV_WAIT�Z�b�g���Ɉړ�
			EC_STATUS_DATA_WAIT.status_data = 0;								// ��ԃf�[�^�҂����킹�p�G���A�N���A
		*/
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�g���f�[�^�ϊ�
//[]----------------------------------------------------------------------[]
///	@param[in]		*buf	:	�ϊ��f�[�^�擪�ʒu
///	@param[in]		count	:	�ϊ��o�C�g��
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
long	settlement_amount_ex(unsigned char *buf, unsigned char count)
{
	long	pay;

	for (pay = 0; count > 0 ;count--) {
		pay *= 100;
		pay += bcdbin(buf[count-1]);
	}

	return (pay);
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_EcArmClearCheck
//[]----------------------------------------------------------------------[]
///	@param[in]		msg				:getmassege�ɂĎ擾����ID
///	@return			ret				: 0=�����p�� 1=ٰ�ߏ�����Continue����
//[]----------------------------------------------------------------------[]
///	@note			Ope_ArmClearCheck()����Ă΂��
///					�\���̖��Ȃǂ�Suica�̂܂�(�]����Suica�폜���ɕύX���邱��)
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
char	Ope_EcArmClearCheck(ushort msg)
{
	char	ret = 0;
	char	exec = 0;

	// �G���[�\���̔��]ү���ޕ\������������
	switch (msg) {
	case SUICA_EVT:											// ���[�_����̎�M�ް�
		// �J�[�h�^�b�`�ŁA����f�[�^�A��ԃf�[�^�A���ό��ʃf�[�^��K����M����(ec_split_data_check()�Q��)
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//		if (Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 0) {
		if (Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA == 0) {
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
		// ����f�[�^��M�Ȃ�
			break;
		}
		else
		if (Ec_Settlement_Res.Result == EPAY_RESULT_NG) {
		// ����f�[�^��M���� && ����=�c���s��or����NG
			break;
		}
	// no break;
	case COIN_EVT:											// Coin Mech event
	case NOTE_EVT:											// Note Reader event
		if ((COIN_EVT == msg && !(OPECTL.CN_QSIG == 1 || OPECTL.CN_QSIG == 5))
		||	(NOTE_EVT == msg && !(OPECTL.NT_QSIG == 1 || OPECTL.NT_QSIG == 5))) {
			break;
		}
	// no break;
	case ARC_CR_R_EVT:										// ����IN
	case KEY_TEN_F4:										// ������� ON
// MH810105(S) MH364301 �x�Ɖ�ʂɁ�����Ɏ��s���܂��������\�������
	case AUTO_CANCEL:										// ���Z�����L�����Z��
// MH810105(E) MH364301 �x�Ɖ�ʂɁ�����Ɏ��s���܂��������\�������
	case ELE_EVENT_CANSEL:									// �I���C�x���g�i���Z���~�j
	case ELE_EVT_STOP:										// ���Z�����C�x���g�i�d�q�}�̂̒�~�j
	case COIN_RJ_EVT:										// ���Z���~(���o�[)
		if (msg == ELE_EVT_STOP && CCT_Cansel_Status.BIT.STOP_REASON != REASON_PAY_END) {
		// ���Z�������ȊO�͔�����
			break;
		}
		if ( (msg == COIN_RJ_EVT || msg == KEY_TEN_F4) && OPECTL.CAN_SW == 1) {
		// ������݂̍ĉ����̏ꍇ
			ret = 1;										// ��������������Ȃ��i����Ĕj���j
		}
		if (OPECTL.InquiryFlg != 0 || Suica_Rec.Data.BIT.CTRL_MIRYO != 0) {
		// �J�[�h�������H or �����������H
			break;
		}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		if (ECCTL.Ec_FailureContact_Sts == 1) {
		// ��Q�A���[���s���H
			break;
		}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

		// Suica�c�z�s���\���� �� Suica�̖�����ѱ�Č��ү���ޕ\����
		if (dsp_fusoku != 0) {
			exec = 1;
		}
		else if (Suica_Rec.Data.BIT.MIRYO_ARM_DISP != 0) {
// MH810105(S) MH364301 �x�Ɖ�ʂɁ�����Ɏ��s���܂��������\�������
//			if (!SUICA_CM_BV_RD_STOP || msg == KEY_TEN_F4 || (OPECTL.op_faz == 3 || OPECTL.op_faz == 9)) {
			if (!SUICA_CM_BV_RD_STOP || msg == KEY_TEN_F4 || msg == AUTO_CANCEL ||
				(OPECTL.op_faz == 3 || OPECTL.op_faz == 9)) {
// MH810105(E) MH364301 �x�Ɖ�ʂɁ�����Ɏ��s���܂��������\�������
				exec = 1;
			}
		}
		else if (DspSts == LCD_WMSG_ON) {
			exec = 1;
		}
		if (exec != 0) {
			LagCan500ms(LAG500_SUICA_ZANDAKA_RESET_TIMER);	// �c�z�s���\�������p����ϰ��ݾ�
			op_SuicaFusokuOff();							// ���ݕ\������ү���ނ�����
			dsp_change = 0;
		}
		break;

	default :
		break;
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_TimeOut_Cyclic_Disp
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze		:��ٌ���̪���
///	@param[in]		e_pram_set		:�d�q���ώg�p�ݒ�
///	@return			void
//[]----------------------------------------------------------------------[]
///	@note			�\���̖��Ȃǂ�Suica��Ope_TimeOut_10()�̂܂�
///					(�]����Suica�폜���ɕύX���邱��)
///	@author			G.So
///	@date			Create	:	2019/03/15<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	Ope_TimeOut_Cyclic_Disp(uchar ope_faze, ushort e_pram_set)
{
	switch (ope_faze) {
	case	0:
		if (e_pram_set == 1) {
		// �d�q�Ȱ�g�p�\�ݒ�H
			op_mod01_dsp_sub();
			grachr(6, 0, 30, 0, COLOR_DARKSLATEBLUE, LCD_BLINK_OFF, OPE_CHR[DspWorkerea[3]]);	// 6�s�ڂɉ�ʐؑ֗p�\��������
			Lagtim(OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[1] * 50));			// ��ʐؑ֗p��ϰ�N��(�T�C�N���b�N�\���^�C�}�[)
		}
		break;

	case	2:
		if (OPECTL.op_faz == 8) {
		// �ڼޯĎg�p�œd�q�}�̒�~���̏ꍇ�͏����������ɔ�����
			break;
		}

		if (EcFirst_Pay_Dsp() != 0) {
		// ����\���i�����\���j
			if (!Ex_portFlag[EXPORT_CHGNEND]) {
			// �`�����p�ł��܂��\�����ޑK�s���łȂ����̏ꍇ
				dspCyclicMsgRewrite(4);
				Lagtim(OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50));	// ���p�\�}�̻��د��\���p��ϰ1000mswait
			}
			if (OPECTL.op_faz == 3 || OPECTL.op_faz == 9) {
			// ������������s���̏ꍇ
				if (Suica_Rec.Data.BIT.CTRL && Suica_Rec.Data.BIT.OPE_CTRL){
				// ���[�_���L�����Ō�ɑ��M�����̂���t���̏ꍇ
					Ec_Pri_Data_Snd(S_CNTL_DATA, 0);										// ���p��s�ɂ���
				}
			}
		}
		break;

	default :
		break;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_TimeOut_Recept_Send
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze		:��ٌ���̪���
///	@param[in]		e_pram_set		:�d�q���ώg�p�ݒ�
///	@return			void
//[]----------------------------------------------------------------------[]
///	@note			�\���̖��Ȃǂ�Suica��Ope_TimeOut_11()�̂܂�
///					(�]����Suica�폜���ɕύX���邱��)
///	@author			G.So
///	@date			Create	:	2019/03/15<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	Ope_TimeOut_Recept_Send( uchar ope_faze, ushort  e_pram_set )
{
// NOTE:���̃^�C�}�[�͎�t�����M��p�Ƃ��A���[�_�[��������ł̂ݎg�p���܂��B
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	if (timer_recept_send_busy == 2) {
//	// ���ԓ��ɏ�ԃf�[�^�̎�M�����������ꍇ�đ�����
//		timer_recept_send_busy = 0;
//		if ((Suica_Rec.suica_err_event.BYTE & 0xFB) == 0) {		// �J�ǈُ�ȊO�̃G���[��������΍đ�
//			EcSendCtrlEnableData();
//		}
//		return;
//	}
//	if (timer_recept_send_busy != 1) {
//	// ���M�v�����Ă��Ȃ��Ȃ牽�����Ȃ�
//		return;
//	}
//	timer_recept_send_busy = 0;	// ���Z����ec������M�^�C�}�[��~��
	if (ECCTL.timer_recept_send_busy == 2) {
	// ���ԓ��ɏ�ԃf�[�^�̎�M�����������ꍇ�đ�����
		ECCTL.timer_recept_send_busy = 0;
		if ((Suica_Rec.suica_err_event.BYTE & 0xFB) == 0) {		// �J�ǈُ�ȊO�̃G���[��������΍đ�
			EcSendCtrlEnableData();
		}
		return;
	}
	if (ECCTL.timer_recept_send_busy != 1) {
	// ���M�v�����Ă��Ȃ��Ȃ牽�����Ȃ�
		return;
	}
	ECCTL.timer_recept_send_busy = 0;	// ���Z����ec������M�^�C�}�[��~��
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
// MH810104 GG119202(S) ����f�[�^�i��t���j�̉����҂��^�C�}���~�߂�
	if ((Suica_Rec.suica_err_event.BYTE & 0xFB) != 0) {
		// ��t�������M�ł��Ȃ���Ԃ̏ꍇ�̓^�C�}���~����
		LagCan500ms(LAG500_SUICA_NO_RESPONSE_TIMER);
		// ���g���C�J�E���g�����^�C�}����~�����^�C�~���O�ŏ���������
		Status_Retry_Count_OK = 0;		//��ײ���Đ���������
		return;
	}
// MH810104 GG119202(E) ����f�[�^�i��t���j�̉����҂��^�C�}���~�߂�
	switch (ope_faze) {
	case	0:
// MH810103 MH321800(S) �����c���Ɖ�^�C���A�E�g��ʂ������Ȃ�
		if (OPECTL.Mnt_mod != 0) {
			// �����e��ʂ͈�U�A����f�[�^�i��t���j�𑗐M����
			Ec_Pri_Data_Snd(S_CNTL_DATA, 0x01);										// ���[�_���p���ɂ���
			ECCTL.timer_recept_send_busy = 2;
			break;
		}
		// no break
// MH810103 GG119202(E) �����c���Ɖ�^�C���A�E�g��ʂ������Ȃ�
// MH810103 GG116201(S) ��t�������M����Ȃ��s��C��
	case	1:
// MH810103 MH321800(E) ��t�������M����Ȃ��s��C��
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
		// �܂����O����f�[�^���������Ă��Ȃ���Ύ�t�𑗐M���Ȃ�
		// ���O����f�[�^�����i���O�o�^�j�������������t�𑗐M����
		if( ac_flg.ec_recv_deemed_fg ){
			break;
		}
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
// MH810104 GG119201(S) ���[�_���撆�͐���f�[�^�i��t���j�𑗐M���Ȃ�
		if( EC_STATUS_DATA.StatusInfo.LastSetteRunning ){
			// ���O����v�����͖�������
			break;
		}
// MH810104 GG119201(E) ���[�_���撆�͐���f�[�^�i��t���j�𑗐M���Ȃ�
		if (!Suica_Rec.Data.BIT.CTRL) {
		// ����L�����Z����̐����ް����M
			Ec_Pri_Data_Snd(S_CNTL_DATA, 0x01);									// ���[�_���p���ɂ���
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//			timer_recept_send_busy = 2;
			ECCTL.timer_recept_send_busy = 2;
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
		}
		break;

	case	2:
		if (!Suica_Rec.Data.BIT.CTRL_MIRYO	&&									// ���Z������ԂŖ����ꍇ�� ����
// MH810104 GG119202(S) ����f�[�^�i��t���j�̉����҂��^�C�}���~�߂�
			!Suica_Rec.Data.BIT.MIRYO_TIMEOUT &&								// �����^�C���A�E�g�������Ă��Ȃ�
// MH810104 GG119202(E) ����f�[�^�i��t���j�̉����҂��^�C�}���~�߂�
			!Suica_Rec.Data.BIT.CTRL		&&									// �����ް����s�� ����
			OPECTL.op_faz != 3				&&									// ������{�^���������Ŗ��� ����
			ryo_buf.zankin != 0L) {												// �c�z������ꍇ
			if (OPECTL.op_faz == 9) {
				break;
			}
			if (OPECTL.op_faz == 8) {
			// Suica��~�҂����킹����
				break;
			}
			if (ope_faze == 22) {
			} else {
				if (MifStat == MIF_WROTE_FAIL) {
				// Mifare�����ݎ��s
					break;
				}
			}
// MH810104 GG119202(S) ����f�[�^�i��t���j�̉����҂��^�C�}���~�߂�
			if (OPECTL.ChkPassSyu != 0) {
				break;
			}
// MH810104 GG119202(E) ����f�[�^�i��t���j�̉����҂��^�C�}���~�߂�
// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//			if ( (!WAR_MONEY_CHECK || e_incnt) &&
//				 (!isEcEnabled(EC_CHECK_CREDIT)) ) {
//			// �������� or �d�q�}�l�[�g�p�ς݁A���A�N���W�b�g���Z�s��
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//			if (ENABLE_MULTISETTLE() == 0) {
			if (ENABLE_MULTISETTLE() == 0 && 					// ���p�\�u�����h�Ȃ�
				check_enable_multisettle() <= 1) {				// �}���`�u�����h�ł͂Ȃ�
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
			// ���ω\�ȃu�����h�Ȃ��̏ꍇ�͎�t���𑗐M���Ȃ�
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
				break;
			}
			if( Suica_Rec.Data.BIT.CTRL_CARD ){
			// ���ލ������͑��M���Ȃ�
				break;
			}
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
			// �܂����O����f�[�^���������Ă��Ȃ���Ύ�t�𑗐M���Ȃ�
			// ���O����f�[�^�����i���O�o�^�j�������������t�𑗐M����
			if( ac_flg.ec_recv_deemed_fg ){
				break;
			}
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
// MH810104 GG119201(S) ���[�_���撆�͐���f�[�^�i��t���j�𑗐M���Ȃ�
			if( EC_STATUS_DATA.StatusInfo.LastSetteRunning ){
				// ���O����v�����͖�������
				break;
			}
// MH810104 GG119201(E) ���[�_���撆�͐���f�[�^�i��t���j�𑗐M���Ȃ�
// MH810104 GG119202(S) �݂Ȃ��x�ƂɂȂ�Ƃ�����I�����o�Ȃ�
			if (IsErrorOccuerd((char)jvma_setup.mdl, ERR_EC_SETTLE_ABORT)) {
				// �݂Ȃ��x�Ǝ��͖�������
				break;
			}
// MH810104 GG119202(E) �݂Ȃ��x�ƂɂȂ�Ƃ�����I�����o�Ȃ�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
			if (isEC_PAY_CANCEL()) {
				// ���Z���~�A�܂��́A�x�Ƃɂ��
				// �Ƃ肯���{�^�������҂����̓��[�_���p�s��
				break;
			}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// GG129000(S) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j
			if (OPECTL.f_rtm_remote_pay_flg != 0) {
				// ���u���Z�J�n��t��
				break;
			}
// GG129000(E) T.Nagai 2023/10/02 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j�i���Z���ύX�f�[�^��M��RXM�ɖ₢���킹�j

			Ec_Pri_Data_Snd(S_CNTL_DATA, 0x01);									// ���[�_���p���ɂ���
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//			timer_recept_send_busy = 2;
			ECCTL.timer_recept_send_busy = 2;
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
			w_settlement = 0;													// �c���s�����̐��Z�z�i�O���͎c���s���łȂ����Ӗ�����j
		}
		break;

// MH810103 GG119202(S) �����c���Ɖ�^�C���A�E�g��ʂ������Ȃ�
	case	100:
		// �x�Ə�Ԃ͈�U�A����f�[�^�i��t���j�𑗐M����
		Ec_Pri_Data_Snd(S_CNTL_DATA, 0x01);										// ���[�_���p���ɂ���
		ECCTL.timer_recept_send_busy = 2;
		break;
// MH810103 GG119202(E) �����c���Ɖ�^�C���A�E�g��ʂ������Ȃ�

	default :
		break;
	}
// ��t�����M��A�����Ď�����
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	if (timer_recept_send_busy == 2) {
	if (ECCTL.timer_recept_send_busy == 2) {
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
		Lagtim(OPETCBNO, TIMERNO_EC_RECEPT_SEND, 5*50);
	}
// MH810104 GG119201(S) �^�C�}���~�����ۂɃ��g���C�J�E���g��������������
	else {
		// ��t�������M�ł��Ȃ���Ԃ̏ꍇ�̓^�C�}���~����
		LagCan500ms(LAG500_SUICA_NO_RESPONSE_TIMER);
		// ���g���C�J�E���g�����^�C�}����~�����^�C�~���O�ŏ���������
		Status_Retry_Count_OK = 0;												//��ײ���Đ���������
	}
// MH810104 GG119201(E) �^�C�}���~�����ۂɃ��g���C�J�E���g��������������
}

//[]----------------------------------------------------------------------[]
///	@brief			snd_enable_timeout
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@note			snd_ctrl_timeout2()���Q�l
///					�\���̖��Ȃǂ�Suica�̂܂�(�]����Suica�폜���ɕύX���邱��)
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	snd_enable_timeout(void)
{
	ushort	tmp = 0;

	LagCan500ms( LAG500_SUICA_NO_RESPONSE_TIMER );				/* ��t�������ް����M���������ؾ�� */

	// �Ō�ɑ��M�����f�[�^����t�֎~�̏ꍇ�̓^�C���A�E�g���������Ȃ�
	if( Suica_Rec.Data.BIT.OPE_CTRL == 0 ){
		return;
	}

	if (Suica_Rec.Data.BIT.CTRL != 0) {
	/* ��Ԃ���t�̏ꍇ */
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//		Suica_Rec.suica_rcv_event.BIT.STATUS_DATA = 1;			/* ����ް���M�׸�ON */
		Suica_Rec.suica_rcv_event.EC_BIT.STATUS_DATA = 1;		/* ����ް���M�׸�ON */
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
		EC_STATUS_DATA.StatusInfo.ReceptStatus = 0;				/* ����ް����e�Ɏ�t�¾�� */
		EcRecvStatusData(&OPECTL.Ope_mod);						/* ��ԃf�[�^��M�������{ */
	} else {

		Suica_Rec.suica_err_event.BIT.OPEN = 1;					/* �J�ǈُ��׸ނ�� */
		if (ECCTL.not_ready_timer == 0) {						// ���[�_�[��t�����䎸�s������
			if (ECCTL.pon_err_alm_regist != 0) {
			// �N�����ȍ~�̃��[�_�[��t�����䎸�s
				alm_chk(ALMMDL_SUB, ALARM_EC_OPEN, 1);			// �װ�۸ޓo�^�i�o�^�j
			}
			ECCTL.not_ready_timer = 1;
			tmp = (ushort)prm_get(COM_PRM, S_ECR, 6, 3, 1);
			if(tmp == 0){										// 0�b�ݒ��70�b�Ƃ��Ĉ���
				tmp = 70;
			} else if(tmp < 10){								// 10�b������10�b�Ƃ��Ĉ���
				tmp = 10;
			}
			tmp = tmp - EC_ENABLE_WAIT_TIME;					// ��t���^�C���A�E�g�ƕ�����,1�b�P��(10�b�`999�b)
			LagTim500ms(LAG500_EC_NOT_READY_TIMER, (short)((tmp * 2) + 1), not_open_timeout);	// ��t�������ް����M��������ϊJ�n
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			snd_disable_timeout
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@note			snd_ctrl_timeout3()���Q�l
///					�\���̖��Ȃǂ�Suica�̂܂�(�]����Suica�폜���ɕύX���邱��)
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	snd_disable_timeout(void)
{
	uchar	Retry_count = 0;
// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
	uchar	cmd_typ = ECCTL.prohibit_snd_bsy;
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�

// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
	if( cmd_typ ){											// ����f�[�^�i��t�֎~�j�����Ď���
		ECCTL.prohibit_snd_bsy = 0;							// ����f�[�^�i��t�֎~�j�����Ď��I��
	}
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
	LagCan500ms(LAG500_SUICA_NONE_TIMER);					/* ��t�֎~�����ް����M���������ؾ�� */
// MH810105 GG119202(S) �����c���Ɖ�ɏ�Q�����������ꍇ�͐ݒ�50-0014�ɏ]��
	if ((cmd_typ & S_CNTL_MIRYO_ZANDAKA_END_BIT) != 0) {
		// �����c���Ɖ�L�����Z���̉����҂��^�C���A�E�g�ł݂Ȃ����ςƂ��邽�߁A�����Ŕ�����
		return;
	}
// MH810105 GG119202(E) �����c���Ɖ�ɏ�Q�����������ꍇ�͐ݒ�50-0014�ɏ]��
	if ((Suica_Rec.Data.BIT.MIRYO_NO_ANSWER == 1)
	&&	(EC_STATUS_DATA.StatusInfo.Running != 0)) {
	// ���[�_���牞�����Ȃ��A�����^�C���A�E�g�����ꍇ
	// ���Z�@�̃^�C�}�[�ɂĖ����^�C���A�E�g�ɂȂ�����̎�t�֎~�̉��������s���̏ꍇ�͉i�v�Ɏ�t�֎~���J��Ԃ��B
	// �������A��t�֎~�̉��������s��BIT�������Ȃ��f�[�^����M����΁A�ʏ�̃��g���C�����Ɉڍs����B
		EC_STATUS_DATA.StatusInfo.Running = 0;				// �s�̉����Ŏ��s������M���A�ēx��t�֎~�𑗐M����ꍇ�͎��s���׸ނ𗎂Ƃ�
		Status_Retry_Count_NG = 0;							// ���s���ŕԂ��Ă����ꍇ�ɂ̓��g���C�J�E���g���N���A����B
		Ec_Pri_Data_Snd(S_CNTL_DATA, 0);					/* �����ް��i��t�֎~�j�𑗐M */
		return;
	}

	// �Ō�ɑ��M�����f�[�^����t���̏ꍇ�̓^�C���A�E�g���������Ȃ�
	if (Suica_Rec.Data.BIT.OPE_CTRL == 1) {
		return;
	}

	if ((Suica_Rec.Data.BIT.CTRL && !SUICA_USE_ERR)) {
	/* ���[�_����t��ԁH */
		Retry_count = EC_DISABLE_RETRY_CNT;
		if (Status_Retry_Count_NG < Retry_count) {
		/* ���̶݂��Đ����ݒ�l�ȉ��H */
// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
//			Ec_Pri_Data_Snd(S_CNTL_DATA, 0);				/* �����ް��i��t�֎~�j�𑗐M */
			if( ECCTL.transact_end_snd ){					// �����҂����Ɏ���I�����M�v������H
				cmd_typ = ECCTL.transact_end_snd;			// ����I���𑗐M����
				Status_Retry_Count_NG = 0;					// ��ײ����=0����đ��M
			}else if( cmd_typ ){							// ��t�֎~�����Ď����H
				cmd_typ &= 0xfe;							// �w�߃f�[�^����t�֎~(��t����/�֎~bit��OFF(�֎~))�Ƃ���
			}	// cmd_typ = 0 �̎��͎�t�֎~�݂̂̐���f�[�^�𑗐M
			Ec_Pri_Data_Snd( S_CNTL_DATA, cmd_typ );		// �����ް��i��t�֎~�j�𑗐M
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
			Status_Retry_Count_NG++;						/* ��ײ���Ă�ݸ���� */
			return;
		} else {
		// ���g���C�I�[�o�[
// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
//			snd_no_response_timeout();
			if( ECCTL.transact_end_snd ){					// �����҂����Ɏ���I�����M�v������H
				cmd_typ = ECCTL.transact_end_snd;			// ����I���𑗐M����
				Status_Retry_Count_NG = 0;					// ��ײ����=0����đ��M
				Ec_Pri_Data_Snd( S_CNTL_DATA, cmd_typ );	// �����ް��i��t�֎~�j�𑗐M
			}else{
				snd_no_response_timeout();
			}
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
		}
	} else {
	/* ���[�_����t�s��ԁH */
		snd_no_response_timeout();
	}
	Status_Retry_Count_NG = 0;								/* ��ײ���Ă�ر */
}

//[]----------------------------------------------------------------------[]
///	@brief			snd_no_response_timeout(�����^�C���A�E�g�����������̓���)
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@note			No_Responce_Timeout()���Q�l
///					�\���̖��Ȃǂ�Suica�̂܂�(�]����Suica�폜���ɕύX���邱��)
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	snd_no_response_timeout(void)
{
	Suica_Rec.Data.BIT.CTRL = 0;
	if (Suica_Rec.Data.BIT.MIRYO_NO_ANSWER && Suica_Rec.Data.BIT.CTRL_MIRYO) {
	/* ������� */
		Suica_Rec.Data.BIT.CTRL_MIRYO = 0;				/* ������ԉ��� */
// MH810103 GG119202(S) �����d�l�ύX�Ή�
//		if (Suica_Rec.Data.BIT.MIRYO_CONFIRM == 0) {
//			// �����m���M�Ȃ��̏ꍇ�̓J�[�hID��'?'�Ŗ��߂�
//			Ec_Settlement_Res.brand_no = EcDeemedLog.EcDeemedBrandNo;	// �u�����h�ԍ�
//			Ec_Settlement_Res.settlement_data = Product_Select_Data;	// ���ϊz
//			memset(Ec_Settlement_Res.Card_ID, '\?', sizeof(Ec_Settlement_Res.Card_ID));
//		}
//		else {
//			Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;		// �����m���M�t���O�N���A
//		}
		Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;			// �����m��(��ԃf�[�^)��M���N���A
// MH810103 GG119202(E) �����d�l�ύX�Ή�
// MH810105 GG119202(S) �����m��̒���f�[�^��M����R0176��o�^����
//// MH810103 GG119202(S) �������Ɉُ�f�[�^��M�łƂ肯���{�^�������҂��ɂȂ�Ȃ�
//		wmonlg(OPMON_EC_MIRYO_TIMEOUT, NULL, 0);		// ���j�^�o�^
//// MH810103 GG119202(E) �������Ɉُ�f�[�^��M�łƂ肯���{�^�������҂��ɂȂ�Ȃ�
// MH810105 GG119202(E) �����m��̒���f�[�^��M����R0176��o�^����
	}
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	Suica_Rec.suica_rcv_event.BIT.STATUS_DATA = 1;		/* ����ް���M�׸�ON */	
	Suica_Rec.suica_rcv_event.EC_BIT.STATUS_DATA = 1;	/* ����ް���M�׸�ON */	
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
	EC_STATUS_DATA.StatusInfo.ReceptStatus = 1;			/* ����ް����e�Ɏ�t�s�¾�� */
	if (Suica_Rec.Data.BIT.ADJUSTOR_NOW != 0) {
	/* ���ϒ��̎�t�֎~�����������ꍇ */
		EC_STATUS_DATA.StatusInfo.TradeCansel = 1;		/* �����ݾ��׸ނ�ON */
// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
		Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;			// CRW-MJA���Z���t���O�N���A
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
	}
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	Ec_Settlement_Sts = EC_SETT_STS_NONE;				/* ���Ϗ�Ԃ�OFF�ɂ��Ă��� */
	ECCTL.Ec_Settlement_Sts = EC_SETT_STS_NONE;			/* ���Ϗ�Ԃ�OFF�ɂ��Ă��� */
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
	EcRecvStatusData(&OPECTL.Ope_mod);					/* ����f�[�^��M�������{ */
// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
	EC_STATUS_DATA.StatusInfo.TradeCansel = 0;			// �����ݾ��׸�OFF
	ECCTL.transact_end_snd = 0;							// ����I�����M�v���Ȃ���ԂƂ���
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
}

// MH810103 GG119202(S) �s�v�����폜
////[]----------------------------------------------------------------------[]
/////	@brief			status_err_timeout
////[]----------------------------------------------------------------------[]
/////	@return			void
////[]----------------------------------------------------------------------[]
/////	@author			G.So
/////	@date			Create	:	2019/02/07<br>
/////					Update
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//void	status_err_timeout(void)
//{
//	unsigned char wks = 0;
//
//	LagCan500ms(LAG500_SUICA_STATUS_TIMER);				/* CRW-MJA��ԊĎ����ؾ�� */
//	wks = EC_INIT_RETRY_CNT;
//	if (Retrycount < wks) {								/* �ď�������ײ�񐔵��ް�H */
//		suicatrb(ERR_SUICA_RECEIVE);					/* �������������s */
//		Retrycount++;									/* ��ײ�����̍X�V */
//	}
//}
// MH810103 GG119202(E) �s�v�����폜

//[]----------------------------------------------------------------------[]
///	@brief			not_open_timeout
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	not_open_timeout(void)
{
	LagCan500ms(LAG500_EC_NOT_READY_TIMER);					/* ��t�������ް����M���������ؾ�� */
	ECCTL.not_ready_timer = 0;
	if (ECCTL.pon_err_alm_regist == 0) {
		ECCTL.pon_err_alm_regist = 1;						// �ȍ~�A�A���[���^�G���[�o�^����
	// �N����ŏ��̎�t��M
		alm_chk(ALMMDL_SUB, ALARM_EC_OPEN, 1);				/* �װ�۸ޓo�^�i�o�^�j*/
	}
	err_chk((char)jvma_setup.mdl, ERR_EC_OPEN, 1, 0, 0);	/* �װ۸ޓo�^�i�o�^�j*/
}

//[]----------------------------------------------------------------------[]
///	@brief			ec_auto_cancel_timeout
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	ec_auto_cancel_timeout(void)
{
	LagCan500ms(LAG500_EC_AUTO_CANCEL_TIMER);			// �A���[�������̐��Z�����L�����Z���^�C�}
	queset(OPETCBNO, AUTO_CANCEL, 0, NULL);				// �������Z���~
}

// MH810103 GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
//[]----------------------------------------------------------------------[]
///	@brief			ec_check_cre_pay_limit
//[]----------------------------------------------------------------------[]
///	@return			ret			�FTRUE	:�N���W�b�g���Z�\(���x�z�ȉ�)
///								�FFALSE	:�N���W�b�g���Z�s��(���x�z�I�[�o�[)
//[]----------------------------------------------------------------------[]
///	@author			M.Nagashima
///	@date			Create	:	2020/05/14<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
BOOL	Ec_check_cre_pay_limit(void)
{
	ulong	limit = (ulong)prm_get( COM_PRM, S_CRE, 2, 6, 1 );

	if(( !limit ) || ( OPECTL.Ope_mod != 2 )){	// ���Z���x�z�`�F�b�N���Ȃ��H
		return TRUE;	// �N���W�b�g���Z�\
	}
	if( ryo_buf.zankin <= limit ){	// ���Z�c���̓N���W�b�g���Z���x�z�ȉ��H
		return TRUE;	// �N���W�b�g���Z�\(���x�z�ȉ�)
	}

	return FALSE;	// �N���W�b�g���Z�s��(���x�z�I�[�o�[)
}
// MH810103 GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�

//[]----------------------------------------------------------------------[]
///	@brief			��ԃf�[�^����
//[]----------------------------------------------------------------------[]
///	@param[in]		*ope_faze	�F���݂�Ope̪���
///	@return			ret			�F0�Œ�
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
short	EcRecvStatusData(uchar *ope_faze)
{
	short	ret = 0;
	uchar	type = 0;
// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
	uchar	cmd_typ = ECCTL.prohibit_snd_bsy;
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
// MH810105 GG119202(S) �����c���Ɖ�ɏ�Q�����������ꍇ�͐ݒ�50-0014�ɏ]��
//// MH810103 GG119202(S) �J�[�h��������M��̌���NG�i304�j�̈���
//	uchar	brand_index = 0;
//// MH810103 GG119202(E) �J�[�h��������M��̌���NG�i304�j�̈���
// MH810105 GG119202(E) �����c���Ɖ�ɏ�Q�����������ꍇ�͐ݒ�50-0014�ɏ]��

// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
	if( cmd_typ ){														// ����f�[�^�i��t�֎~�j�����Ď���
		ECCTL.prohibit_snd_bsy = 0;										// ����f�[�^�i��t�֎~�j�����Ď��I��
	}
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
// MH810103 GG119202(S) �}���`�u�����h�d�l�ύX�Ή�
//	if (Suica_Rec.Data.BIT.CTRL == 0) {									// ��t�s���󂯂��Ƃ�
//		RecvBrandResTbl.res = EC_BRAND_UNSELECTED;						// ��U�A�I���u�����h�Ȃ��ɖ߂�
//		RecvBrandResTbl.no = BRANDNO_UNKNOWN;							// ��t��̓u�����h�I������蒼��
//	/*	Suica_Rec.Data.BIT.SLCT_BRND_BACK = 0;							// �u�����h�I���́u�߂�v���o�t���O������	*/
//	}
// MH810103 GG119202(E) �}���`�u�����h�d�l�ύX�Ή�
// MH810103 GG119201(S) ���[�_�̃L�����Z���҂����ɐ��Z�@�������e�i���X���[�h�ɐ؂�ւ���ƁA�d��OFF/ON����܂Œ��Ԍ����ԋp����Ȃ�
//	if (OPECTL.Mnt_mod != 0) {
	// �t���b�v���ł͔������Ȃ����Q�[�g���Ə��������킹��
	if (OPECTL.Ope_mod == 0 && OPECTL.Mnt_mod != 0) {
// MH810103 GG119201(E) ���[�_�̃L�����Z���҂����ɐ��Z�@�������e�i���X���[�h�ɐ؂�ւ���ƁA�d��OFF/ON����܂Œ��Ԍ����ԋp����Ȃ�
// MH810103 GG118809_GG118909(S) ���σ��[�_�r�������l���R��Ή�
		if (PRE_EC_STATUS_DATA.StatusInfo.ZandakaInquiry &&
			!EC_STATUS_DATA.StatusInfo.ZandakaInquiry) {
			edy_dsp.BIT.suica_zangaku_dsp = 0;
		}
// MH810103 GG118809_GG118909(E) ���σ��[�_�r�������l���R��Ή�
		if(Suica_Rec.Data.BIT.CTRL ||
// MH810103 GG119202(S) E3209���������Ă��܂����̑΍�
			Suica_Rec.Data.BIT.OPE_CTRL ||
// MH810103 GG119202(E) E3209���������Ă��܂����̑΍�
		   (!PRE_EC_STATUS_DATA.StatusInfo.RebootReq &&
			EC_STATUS_DATA.StatusInfo.RebootReq)) {
			// �����e�i���X���Ɏ�t������M����
			// �ċN���v��bit��0->1�ɕω�����
			Ec_Pri_Data_Snd(S_CNTL_DATA, 0);		// ��t�֎~���M
		}
		if (EC_STATUS_DATA.StatusInfo.TradeCansel) {
			// �����e�i���X���Ɏ���L�����Z������M����
			recv_unexpected_status_data();
		}
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		Suica_Rec.suica_rcv_event.EC_BIT.STATUS_DATA = 0;
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		return ret;
	}
	Ec_Status_Chg();

	// �ċN���v��bit��0->1�ɕω������Ƃ��̂ݐ���f�[�^�ŉ�������
	if (!PRE_EC_STATUS_DATA.StatusInfo.RebootReq &&
		EC_STATUS_DATA.StatusInfo.RebootReq) {
		type = Suica_Rec.Data.BIT.CTRL;				// ��t���^�֎~���Z�b�g
		Ec_Pri_Data_Snd(S_CNTL_DATA, type);			// ����f�[�^���M
	}

	switch (*ope_faze) {
// MH810103(s) �d�q�}�l�[�Ή� �ҋ@��ʂł̎c���Ɖ��
// 	case	0:	// �ҋ@���
// 	case	1:	// �Ԏ��ԍ����͉��
// 		if (Suica_Rec.Data.BIT.CTRL == 0 && EC_STATUS_DATA.StatusInfo.ReceptStatus) {
// 			if (OPECTL.op_faz == 8) { 										// �d�q�}�̒�~������
// 				Op_StopModuleWait_sub(0);									// CRW-MJA��~�ςݏ���
// 			}
// 			EcSendCtrlEnableData();											// ����f�[�^(��t����)���M
// 		}
// // GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
// 		else if( Suica_Rec.Data.BIT.CTRL && !Suica_Rec.Data.BIT.OPE_CTRL ){	// ��t����M���Ō�ɑ��M�����͎̂�t�֎~�H
// 			Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );								// ����f�[�^(��t�֎~)���M
// 		}
// // GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
// 		break;
	case	0:	// �ҋ@���
		if (!isEcEmoneyEnabled(0, 0)) {
			// �d�q�}�l�[�Ȃ����͎�t�����M��A��t�ɂȂ�܂Ŏ�t���𑗐M����
			if (Suica_Rec.Data.BIT.CTRL == 0) {
				EcSendCtrlEnableData();										// ����f�[�^(��t����)���M
			}
// MH810103(s) �d�q�}�l�[�Ή� #5519 �ҋ@��ʂ�VP6800�֕s���𑗐M���Ȃ��悤�ɂ���
			// �s���M
			lcdbm_notice_ope(LCDBM_OPCD_INQUIRY_PAY_AFTER,1);
// MH810103(e) �d�q�}�l�[�Ή� #5519 �ҋ@��ʂ�VP6800�֕s���𑗐M���Ȃ��悤�ɂ���
		}
		else {
			if(OPECTL.lcd_query_onoff != 0){								// �ҋ@��ʂł̎c���Ɖ��(0:NG/1:OK/2:���̑�)
				if (Suica_Rec.Data.BIT.CTRL == 0) {
					EcSendCtrlEnableData();										// ����f�[�^(��t����)���M
				}
// MH810103(s) �d�q�}�l�[�Ή� #5519 �ҋ@��ʂ�VP6800�֕s���𑗐M���Ȃ��悤�ɂ���
				else{
					// �\���M
					lcdbm_notice_ope(LCDBM_OPCD_INQUIRY_PAY_AFTER,0);
				}
// MH810103(e) �d�q�}�l�[�Ή� #5519 �ҋ@��ʂ�VP6800�֕s���𑗐M���Ȃ��悤�ɂ���
			}else{
				if (Suica_Rec.Data.BIT.CTRL == 1 || Suica_Rec.Data.BIT.OPE_CTRL == 1) {
					Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );								// ����f�[�^(��t�֎~)���M
				}
// MH810103(s) �d�q�}�l�[�Ή� #5519 �ҋ@��ʂ�VP6800�֕s���𑗐M���Ȃ��悤�ɂ���
				// �s���M
				lcdbm_notice_ope(LCDBM_OPCD_INQUIRY_PAY_AFTER,1);
// MH810103(e) �d�q�}�l�[�Ή� #5519 �ҋ@��ʂ�VP6800�֕s���𑗐M���Ȃ��悤�ɂ���
			}
		}
		break;
	case	1:	// �Ԏ��ԍ����͉��
// MH810104 GG119201(S) ���O����f�[�^�������͎�t����M�������t�֎~�𑗐M����
		if( Suica_Rec.Data.BIT.CTRL &&
			(ac_flg.ec_recv_deemed_fg || EC_STATUS_DATA.StatusInfo.LastSetteRunning)){
			// ����f�[�^���܂��������Ă��炸�A��t����M�������t�֎~�𑗐M����
			Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );								// ����f�[�^(��t�֎~)���M
			break;
		}
// MH810104 GG119201(E) ���O����f�[�^�������͎�t����M�������t�֎~�𑗐M����
		if (Suica_Rec.Data.BIT.CTRL == 0 && EC_STATUS_DATA.StatusInfo.ReceptStatus) {
			if (OPECTL.op_faz == 8) { 										// �d�q�}�̒�~������
				Op_StopModuleWait_sub(0);									// CRW-MJA��~�ςݏ���
			}
			EcSendCtrlEnableData();											// ����f�[�^(��t����)���M
		}
		else if( Suica_Rec.Data.BIT.CTRL && !Suica_Rec.Data.BIT.OPE_CTRL ){	// ��t����M���Ō�ɑ��M�����͎̂�t�֎~�H
			Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );								// ����f�[�^(��t�֎~)���M
		}
		break;
// MH810103(e) �d�q�}�l�[�Ή� �ҋ@��ʂł̎c���Ɖ��

	case	2:		// ���Z��
		if (!Suica_Rec.Data.BIT.CTRL_MIRYO) {								// ���Z������ԂŖ����ꍇ�̂ݏ��������{
			if (Suica_Rec.Data.BIT.CTRL) {									// �����ް�����Ԃ̎���
		// ��t�|�|�|�|�|�|�|��
				if (OPECTL.op_faz == 3 ||									// �d�q�}�̒�~�҂����킹���H
					OPECTL.op_faz == 8) {
					// �d�q�}�̒�~�҂����킹���ł̎�t��M���́A̪��ލ��킹�ׂ̈Ɏ�t�֎~�𑗐M����B
					Ec_Pri_Data_Snd(S_CNTL_DATA, 0);
					break;
				}
// MH810104 GG119201(S) ���O����f�[�^�������͎�t����M�������t�֎~�𑗐M����
				if( ac_flg.ec_recv_deemed_fg || EC_STATUS_DATA.StatusInfo.LastSetteRunning ){
					// ����f�[�^���܂��������Ă��炸�A��t����M�������t�֎~�𑗐M����
					Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );						// ����f�[�^(��t�֎~)���M
					break;
				}
// MH810104 GG119201(E) ���O����f�[�^�������͎�t����M�������t�֎~�𑗐M����
				if (OPECTL.op_faz >= 9 || 									// ���Z���~���̌��σ��[�_��~�҂����킹��(����ɒ�~���Ă��Ȃ��ꍇ)��
					EC_STATUS_DATA.StatusInfo.Running) {					// ��M�X�e�[�^�X�����s��
					break;
				}
				if (EC_STATUS_DATA.StatusInfo.ReqTimerUpdate != 0) {
				// �����Ƀy�A�ƂȂ��ԃf�[�^���ēx����̂ŁA���i��I���^�C�}�[�X�V�˗����̏�ԃf�[�^�͎󂯎̂Ă���
					break;
				}
				if ((Suica_Rec.Data.BIT.CTRL == 1)
// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
//				&&	(Suica_Rec.Data.BIT.PRE_CTRL == Suica_Rec.Data.BIT.CTRL)) {
//				// ����M����ذ�ޏ�Ԃ���t�� && �ω��Ȃ����I�����i�f�[�^�𑗐M���Ȃ�
				&& (( Suica_Rec.Data.BIT.PRE_CTRL == Suica_Rec.Data.BIT.CTRL) ||
					( !Suica_Rec.Data.BIT.OPE_CTRL ))){
					// ����M����ذ�ޏ�Ԃ���t�� && (�ω��Ȃ� || �Ō�ɑ��M�����͎̂�t�֎~)���I�����i�f�[�^�𑗐M���Ȃ�
					if( !Suica_Rec.Data.BIT.OPE_CTRL ){						// �Ō�ɑ��M�����͎̂�t�֎~
						// ��t�֎~�𑗐M�����̂Ɏ�t�\����M�����̂Ŏ�t�֎~���đ�����
						if( cmd_typ & 0x80 ){								// �Ō�ɑ��M������t�֎~�̎���I��BitON�H
							Suica_Rec.Data.BIT.SEND_CTRL80 = 0;				// ����I���f�[�^���M��ԃt���O�N���A
						}
						if( ECCTL.transact_end_snd ){						// �����҂����Ɏ���I�����M�v������H
							cmd_typ = ECCTL.transact_end_snd;				// ����I���𑗐M����
						}else if( cmd_typ ){								// ��t�֎~�����Ď����H
							cmd_typ &= 0xfe;								// �w�߃f�[�^����t�֎~(��t����/�֎~bit��OFF(�֎~))�Ƃ���
						}	// cmd_typ = 0 �̎��͎�t�֎~�݂̂̐���f�[�^�𑗐M
						Ec_Pri_Data_Snd( S_CNTL_DATA, cmd_typ );			// �����ް��i��t�֎~�j�𑗐M
					}
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
					break;
				}

// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//				if (WAR_MONEY_CHECK && !e_incnt) {
//				// �������E�d�q�}�l�[���g�p
//					if (OPECTL.op_faz < 2) {									// ���Z���ł��邱��
//// GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
////						if ((ENABLE_MULTISETTLE() == 0) || 						// �P�ꌈ�� ||
////							(RecvBrandResTbl.res == EC_BRAND_SELECTED)) {		// �u�����h�I���ς݂ł��邱��
//						if( ENABLE_MULTISETTLE() == 0 ){						// ���ω\�u�����h�Ȃ�
//							Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );					// ����f�[�^(��t�֎~)���M
//						}
//						else if( ENABLE_MULTISETTLE() == 1 ){ 					// �P�ꌈ�ϗL��
//// GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
//							if (w_settlement != 0) {							// �c���s�����̈�������
//								Ec_Data_Snd(S_SELECT_DATA, &w_settlement);		// �c����I�����i�ް��Ƃ��đ��M����
//							} else {											// �ʏ펞
//								Ec_Pri_Data_Snd(S_SELECT_DATA, 0);				// ���ԗ�����I�����i�ް��Ƃ��đ��M����
//								w_settlement = 0;								// �I�����i�f�[�^�ҏW�̈��ر����
//							}
//						} else {
//							// �������ρA���A�u�����h���I��
//							Ec_Pri_Data_Snd( S_BRAND_SEL_DATA, 0 );				// �u�����h�I���f�[�^�𑗐M����
//						}
//					}
//				} else {
//				// ��������
//					if (OPECTL.op_faz < 2) {
//						if (isEcReady(EC_CHECK_CREDIT)) {						// �N���W�b�g���ω\
//							if( RecvBrandResTbl.no != BRANDNO_CREDIT ){			// �u�����h�I���ŃN���W�b�g��I�����Ă��Ȃ���
//								RecvBrandResTbl.res = EC_BRAND_UNSELECTED;		// �����u�����h�I������Ă��Ȃ����ɂ���(��������œd�q�}�l�[�͂��肦�Ȃ���)
//							}
//							if (w_settlement != 0) {							// �c���s�����̈�������
//								Ec_Data_Snd(S_SELECT_DATA, &w_settlement);		// �c���i10�~�����؎̂āj��I�����i�ް��Ƃ��đ��M����
//							} else {											// �ʏ펞
//								Ec_Pri_Data_Snd(S_SELECT_DATA, 0);				// ���ԗ�����I�����i�ް��Ƃ��đ��M����
//								w_settlement = 0;								// �I�����i�f�[�^�ҏW�̈��ر����
//							}
//						}
//					}
//				}
				if (OPECTL.op_faz < 2) {										// ���Z���ł��邱��
					// ��ʑJ�ڐ�͉��L�̒ʂ�
					// 1.����NG��M�i�c���s���Ȃǁj�i�J�[�h�ǂݎ��^�C���A�E�g�͏����j���J�[�h�ǂݎ��҂����
					// 2.�T�[�r�X�����}�����J�[�h�ǂݎ��҂����
					// 3.�����������J�[�h�ǂݎ��҂����
					//   ���d�q�}�l�[�͌������p�s�̂��ߎx�����I����ʂɖ߂�
					// 4.�J�[�h�ǂݎ��҂��^�C���A�E�g���x�����I�����
					// 5.�L�����Z���{�^���������x�����I�����
					switch(check_enable_multisettle()) {
					default:													// ����
						if( RecvBrandResTbl.res != EC_BRAND_UNSELECTED ) {		// �u�����h�I���ς�
							if (! ec_check_valid_brand(RecvBrandResTbl.no)) {
// MH810103 GG119202(S) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
								EcEnableNyukin();
// MH810103 GG119202(E) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
								EcBrandClear();									// �I������
							}
						}
						if( RecvBrandResTbl.res == EC_BRAND_UNSELECTED ) {		// �u�����h�I���ŃN���W�b�g��I�����Ă��Ȃ���
							// �������ρA���A�u�����h���I��
							Ec_Pri_Data_Snd( S_BRAND_SEL_DATA, 0 );				// �u�����h�I���f�[�^�𑗐M����
							break;
						}
					//	break;
					case	1:													// �P��
						{
						// �c�z�s���͌���NG�ɂȂ邽�߁A�c�z����������@�\�͍폜����
// MH810103 GG119202(S) 0�~�̑I�����i�f�[�^�𑗐M���Ă��܂�
							if (RecvBrandResTbl.res == EC_BRAND_UNSELECTED) {	// �u�����h���I�����
								ec_set_brand_selected();
							}
// MH810103 GG119202(E) 0�~�̑I�����i�f�[�^�𑗐M���Ă��܂�
							Ec_Pri_Data_Snd(S_SELECT_DATA, 0);					// ���ԗ�����I�����i�ް��Ƃ��đ��M����
							w_settlement = 0;									// �I�����i�f�[�^�ҏW�̈��ر����
// MH810103 GG119202(S) �d�q�}�l�[�V���O���ݒ�ňē��������s��
							op_EcEmoney_Anm();
// MH810103 GG119202(E) �d�q�}�l�[�V���O���ݒ�ňē��������s��
						}
						break;
					case	0:													// �u�����h�Ȃ�
						Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );						// ����f�[�^(��t�֎~)���M
						break;
					}
				}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
			}
			else {															// �����ް����s��Ԃ̎���
		// ��t�s�|�|�|�|�|�|��
				if (EC_STATUS_DATA.StatusInfo.ReceptStatus &&				// ��t�s�f�[�^�y��
					EC_STATUS_DATA.StatusInfo.TradeCansel) {				// �����ݾَ�t�f�[�^����M��
// MH810105 GG119202(S) �����c���Ɖ�ɏ�Q�����������ꍇ�͐ݒ�50-0014�ɏ]��
					// �����c���Ɖ�ɏ�Q���������ꍇ��EcDeemedSettlement()���Ă΂��
					// �ݒ�50-0014�ɏ]�����߁A���L�����͍폜����
//					if ( dsp_change == 1 ) {								// �����c���I���ʒm�𖢑��M�̎�t�s����L�����Z����M��
//					// �������^�C���A�E�g���̎�t�֎~�v���ɑ΂��Ė������̏ꍇ�Asnd_no_response_timeout()����Ă΂��
//						op_SuicaFusokuOff();								// ���Z�������̔��]ү���ޕ\������������
//						dsp_change = 0;
//						miryo_timeout_after_mif(0);
//// MH810104 GG119201(S) ���������Ɠ����ɃJ�[�h�^�b�`�������c���Ɖ�ɂ݂Ȃ����ώ�M�ŉ�ʂ����b�N����
//						if( ryo_buf.nyukin && OPECTL.credit_in_mony ){		// ��������œd�q�}�l�[�Ƃ̂���Ⴂ�����̏ꍇ
//							// �����z >= ���ԗ����̏�ԂŖ����^�C���A�E�g�����ꍇ��
//							// ���Z�����Ƃ��Ĉ����A�Ƃ肯���{�^�������҂��ɂ��Ȃ�
//							in_mony ( COIN_EVT, 0 );						// ��ݲ���ē���������āA��������(ryo_buf�ر�̍X�V)���s��
//							OPECTL.credit_in_mony = 0;						// ����Ⴂ�׸޸ر
//						}
//// MH810104 GG119201(E) ���������Ɠ����ɃJ�[�h�^�b�`�������c���Ɖ�ɂ݂Ȃ����ώ�M�ŉ�ʂ����b�N����
//						nyukin_delay_check(nyukin_delay, delay_count);		// �ۗ����̲���Ă�����΍�post����
//						ec_miryo_timeout_after_disp(&Ec_Settlement_Res);	// �����^�C���A�E�g��̕\�����s��
//						Lagcan( OPETCBNO, 1 );										// ��ϰ1ؾ��(���ڰ��ݐ���p)
//// MH810103 GG119202(S) �����c���Ɖ�^�C���A�E�g���Ă���10���o�߂Ő���f�[�^�i��t���j�����M�����
////						Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );	// ��ϰ8(XXs)�N��(������߂蔻����ϰ)
//// MH810103 GG119202(E) �����c���Ɖ�^�C���A�E�g���Ă���10���o�߂Ő���f�[�^�i��t���j�����M�����
//// MH810103 GG119202(S) �J�[�h��������M��̌���NG�i304�j�̈���
//						brand_index = convert_brandno(Ec_Settlement_Res.brand_no);
//						if( brand_index == EC_UNKNOWN_USED || brand_index == EC_ZERO_USED ){
//							// �I�����i�f�[�^�ő��M�����u�����h�����ό��ʂŎ�M�������ɂ���
//							Ec_Settlement_Res.brand_no = ECCTL.Product_Select_Brand;
//							brand_index = convert_brandno(ECCTL.Product_Select_Brand);
//						}
//						ec_alarm_syuu((uchar)brand_index, (ulong)Product_Select_Data);	// �A���[������W�v
//						ReqEcAlmRctPrint(J_PRI, &Ec_Settlement_Res);					// �A���[�����V�[�g�󎚗v��
//						EcAlarmLog_Regist(&Ec_Settlement_Res);							// ��������������O�o�^
//// MH810103 GG119202(E) �J�[�h��������M��̌���NG�i304�j�̈���
//					}
// MH810105 GG119202(E) �����c���Ɖ�ɏ�Q�����������ꍇ�͐ݒ�50-0014�ɏ]��
// MH810103 GG119202(S) �����d�l�ύX�Ή�
//					else if ( dsp_change == 2 ){							// �����c���I���ʒm�𑗐M������̎�t�s����L�����Z����M��
//						nyukin_delay_check(nyukin_delay, delay_count);		// �ۗ����̲���Ă�����΍�post����
//					}
// MH810103 GG119202(E) �����d�l�ύX�Ή�
					if (Suica_Rec.Data.BIT.SEND_CTRL80 == 0) {
						Ec_Pri_Data_Snd(S_CNTL_DATA, 0x80);					// �����ް��i����I���j�𑗐M
					}
// MH810103 GG119202(S) �J�[�h�������҂����b�Z�[�W���\������Ȃ�
					if (OPECTL.op_faz < 2) {								// ���Z��
						// �J�[�h����̏ꍇ�̓J�[�h�������҂����s��
						ec_start_removal_wait();
					}
// MH810103 GG119202(E) �J�[�h�������҂����b�Z�[�W���\������Ȃ�
				}
// MH810103 GG119202(S) ����f�[�^�i��t�֎~�j�̉����҂��Ή�
				else if( !Suica_Rec.Data.BIT.OPE_CTRL ){					// �Ō�ɑ��M�����͎̂�t�֎~�H
					if( ECCTL.transact_end_snd ){							// �����҂����Ɏ���I�����M�v������H
						Ec_Pri_Data_Snd( S_CNTL_DATA, ECCTL.transact_end_snd );	// ����I���𑗐M����
					}
				}
// MH810103 GG119202(E) ����f�[�^�i��t�֎~�j�̉����҂��Ή�

				// �ۗ����Ă����C�x���g���ĊJ����
				if(OPECTL.holdPayoutForEcStop) {
					OPECTL.holdPayoutForEcStop = 0;
					queset( OPETCBNO, COIN_EN_EVT, 0, 0 );
				}

// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
				if (Suica_Rec.Data.BIT.BRAND_SEL != 0 &&
					Suica_Rec.Data.BIT.BRAND_CAN != 0) {
				// �u�����h�I�𒆂Ɏ�t�s����M�����Ƃ��A���Z�@����̎�t�֎~�ɗR������Ƃ�
				// �u�����h�I�����ʂ̒ʒm�����Ȃ��̂ł����ŏ�Ԃ��N���A����
					Suica_Rec.Data.BIT.BRAND_SEL = 0;
					Suica_Rec.Data.BIT.BRAND_CAN = 0;
					EcBrandClear();
				}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j

				if (OPECTL.op_faz == 3 || OPECTL.op_faz == 9 || time_out) {	// ���Z���~���݉�����Suica��~�҂����킹����
					Op_Cansel_Wait_sub(0);									// CRW-MJA��~�ς݂Ƃ��Ē�~���f���s��
					break;
				}

				if (OPECTL.op_faz == 8) {									// �d�q�}�̒�~�҂����킹����
					if (Op_StopModuleWait_sub(0) == 1) {					// CRW-MJA��~�ς݂Ƃ��Ē�~���f���s��
						ret = 0;
						break;
					}
				}

				// ��ԃf�[�^�i��t�s�j��M���ɐ���f�[�^�i��t���j��200ms��ɑ��M����
				// ����L�����Z���A�܂��́A3�f�[�^(��ԁA���Ϗ�ԁA���ό���)��M���͐���f�[�^�i��t���j�𑗐M���Ȃ�
				if (EC_STATUS_DATA.StatusInfo.ReceptStatus &&				// ذ�ް����t�s��ԂƂȂ����ꍇ��
					!EC_STATUS_DATA.StatusInfo.TradeCansel &&				// �����ݾقȂ�
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//					!Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA) {	// ���ό��ʃf�[�^�𖢏����̏ꍇ�́A�Ċ����������̃^�C�}�[���X�^�[�g
					!Suica_Rec.suica_rcv_event_sumi.EC_BIT.SETTLEMENT_DATA) {	// ���ό��ʃf�[�^�𖢏����̏ꍇ�́A�Ċ����������̃^�C�}�[���X�^�[�g
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
																			// ��t���d���𑗐M���邩�ǂ����̓^�C���A�E�g��Ŕ��肷��
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//					if (!time_out && Suica_Rec.Data.BIT.CTRL_CARD == 0) {	// �ҋ@���A�^�C�}�[�^�C���A�E�g�ȑO�A�������J�[�h�Ȃ��Ȃ�
					if (!time_out && Suica_Rec.Data.BIT.CTRL_CARD == 0 &&	// �ҋ@���A�^�C�}�[�^�C���A�E�g�ȑO�A�������J�[�h�Ȃ��Ȃ�
						Suica_Rec.Data.BIT.BRAND_SEL == 0) {				// �u�����h�I�𒆂̎�t�s�ω��͑҂����킹��
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810100(S) S.Fujii 2020/08/25 #4753 QR�R�[�h�ǂݎ���ɓ��ɏ���M�O��IC�N���W�b�g��t�ɂȂ�
//						EcSendCtrlEnableData();							// ����f�[�^(��t����)���M
						if( lcdbm_Flag_QRuse_ICCuse == 0 ){					// QR���p���ł͂Ȃ�(QR���p���͓��ɏ���M�Ŏ�t���𑗐M����)
							EcSendCtrlEnableData();							// ����f�[�^(��t����)���M
						}
// MH810100(E) S.Fujii 2020/08/25 #4753 QR�R�[�h�ǂݎ���ɓ��ɏ���M�O��IC�N���W�b�g��t�ɂȂ�
					}
				}
			}
		} else {
		// ���������|�|�|�|�|�|��
			if (EC_STATUS_DATA.StatusInfo.MiryoZandakaStop != 0) {			// �����c�����~��M
				// �����c�����~��M���͎����Ƃ肯���Ƃ���
				ec_auto_cancel_timeout();
				break;
			}
			if (MIFARE_CARD_DoesUse) {										// Mifare���L���ȏꍇ
				LagCan500ms(LAG500_MIF_LED_ONOFF);							// Mifare�ėL������ϰ�į��
				op_MifareStop_with_LED();									// Mifare����
			}
			Lagcan(OPETCBNO, 1);											// ���ڰ��݊֘A��ϰؾ��
			Lagcan(OPETCBNO, 8);											// ������߂蔻����ϰؾ��
			Lagcan(OPETCBNO, TIMERNO_EC_CYCLIC_DISP);						// �T�C�N���b�N�\���^�C�}�[ؾ��
// MH810103 GG119202(S) ���������̈ē��\�������C��
//			dsp_change = 1;													// �����װѕ\�����׸޾��
// MH810103 GG119202(E) ���������̈ē��\�������C��
// MH810103 GG119202(S) �����d�l�ύX�Ή�
//			lcd_wmsg_dsp_elec(1, OPE_CHR[91], OPE_CHR[92], 0, 0, COLOR_RED, LCD_BLINK_OFF);		// "������x�A�����J�[�h��"
			if (EC_STATUS_DATA.StatusInfo.MiryoConfirm &&
// MH810104 GG119202(S) nanaco�EiD�EQUICPay�Ή�
//				Suica_Rec.Data.BIT.MIRYO_CONFIRM == 0) {					// �����������ɖ����m��(��ԃf�[�^)��M
				Suica_Rec.Data.BIT.MIRYO_CONFIRM == 0 &&					// �����������ɖ����m��(��ԃf�[�^)��M
				isEcBrandNoEMoney(RecvBrandResTbl.no, 1)) {					// �c���Ɖ�\�ȃu�����h
// MH810104 GG119202(E) nanaco�EiD�EQUICPay�Ή�
				op_SuicaFusokuOff();
// MH810103(s) �d�q�}�l�[�Ή� �����ʒm
//				lcd_wmsg_dsp_elec( 1, ERR_CHR[20], ERR_CHR[53], 0, 0, COLOR_RED, LCD_BLINK_OFF); // "�J�[�h����Ɏ��s���܂���"
//																								 // "�������������c���Ɖ���s���܂�"
				lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x05);	// ����(���σ^�C���A�E�g) // "�J�[�h����Ɏ��s���܂���"
																							 // "�������������c���Ɖ���s���܂�"
// MH810103(e) �d�q�}�l�[�Ή� �����ʒm
				Suica_Rec.Data.BIT.MIRYO_CONFIRM = 1;						// �����m��(��ԃf�[�^)��M�ς݂��Z�b�g
// MH810105 GG119202(S) ������������W�v�d�l���P
//// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//				EcDeemedLog.MiryoFlg = 2;									// �����c���Ɖ
//// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
// MH810105 GG119202(E) ������������W�v�d�l���P
			}
// MH810103 GG119202(S) �������Ɉُ�f�[�^��M�łƂ肯���{�^�������҂��ɂȂ�Ȃ�
//			else if (EC_STATUS_DATA.StatusInfo.MiryoStatus) {				// ����������M
			else if (EC_STATUS_DATA.StatusInfo.MiryoStatus &&				// ����������M
// MH810103 GG119202(S) ��ԃf�[�^�i�����m��j��M�̃f�[�^�҂����킹�����C��
					 isEC_CONF_MIRYO() == 0 &&								// �����m�薢����
// MH810103 GG119202(E) ��ԃf�[�^�i�����m��j��M�̃f�[�^�҂����킹�����C��
// MH810103 GG119202(S) ���������̈ē��\�������C��
					 dsp_change == 0 &&										// �����װѕ\�����׸ޖ����
// MH810103 GG119202(E) ���������̈ē��\�������C��
					 Suica_Rec.Data.BIT.MIRYO_TIMEOUT == 0) {				// �����^�C���A�E�g������
// MH810103 GG119202(E) �������Ɉُ�f�[�^��M�łƂ肯���{�^�������҂��ɂȂ�Ȃ�
// MH810103(s) �d�q�}�l�[�Ή� �����ʒm
//				lcd_wmsg_dsp_elec(1, OPE_CHR[91], OPE_CHR[92], 0, 0, COLOR_RED, LCD_BLINK_OFF);	// "������x�A�����J�[�h��"
//																								// "�^�b�`���Ă�������"
				lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x03);	// ����(�ă^�b�`�҂�) 	// "������x�A�����J�[�h��"
																							// "�^�b�`���Ă�������"
// MH810103(e) �d�q�}�l�[�Ή� �����ʒm
// MH810103 GG119202(S) ���������̈ē��\�������C��
				dsp_change = 1;												// �����װѕ\�����׸޾��
// MH810103 GG119202(E) ���������̈ē��\�������C��
			}
// MH810103 GG119202(E) �����d�l�ύX�Ή�
																								// "�^�b�`���Ă�������"
			if (dsp_fusoku) {												// �s���A���[���\�����H
				LagCan500ms(LAG500_SUICA_ZANDAKA_RESET_TIMER);				// �s���A���[��������ϰ���~�߂�
			}
		}
		break;

	case	3:		// ���Z����
		if (Suica_Rec.Data.BIT.SEND_CTRL80 == 0) {
		// ���M���Ă��Ȃ����
			Ec_Pri_Data_Snd(S_CNTL_DATA, 0x80);								// �����ް��i����I���j�𑗐M����
		} else if (Suica_Rec.Data.BIT.CTRL) {
			Ec_Pri_Data_Snd(S_CNTL_DATA, 0);								// �����ް��i��t�֎~�j�𑗐M����
		}
		if (EC_STATUS_DATA.StatusInfo.TradeCansel) {
			// ���Z������Ɏ���L�����Z������M����
			recv_unexpected_status_data();
		}
		break;

// MH810103 GG119202(S) �c���Ɖ�{�^����\���̎b��΍�
	case	100:	// �x��
// MH810103 GG119202(S) E3209���������Ă��܂����̑΍�
// MH810103 GG118809_GG118909(S) ���σ��[�_�r�������l���R��Ή�
		if (PRE_EC_STATUS_DATA.StatusInfo.ZandakaInquiry &&
			!EC_STATUS_DATA.StatusInfo.ZandakaInquiry) {
			edy_dsp.BIT.suica_zangaku_dsp = 0;
		}
// MH810103 GG118809_GG118909(E) ���σ��[�_�r�������l���R��Ή�
		// ���L�̏����͐���f�[�^�i��t���j�ɑ΂��ď�ԃf�[�^�i��t�s�j����M�����Ƃ���
		// �������M���Ȃ����߁ALAG500_SUICA_NO_RESPONSE_TIMER�̃^�C�}���~�߂�_�@���Ȃ��Ȃ�
		// �d�q�}�l�[�Ȃ����͎�t�s��Ԃɂ���K�v���Ȃ��̂ŁA�d�q�}�l�[����^�Ȃ��ŏ����𕪂���
//		if (Suica_Rec.Data.BIT.CTRL == 1 && !EC_STATUS_DATA.StatusInfo.ReceptStatus) {
		if (!isEcEmoneyEnabled(0, 0)) {
			// �d�q�}�l�[�Ȃ����͎�t�����M��A��t�ɂȂ�܂Ŏ�t���𑗐M����
			if (Suica_Rec.Data.BIT.CTRL == 0) {
				EcSendCtrlEnableData();										// ����f�[�^(��t����)���M
			}
		}
		else {
			// �d�q�}�l�[���莞�͎�t�����M��A��t�s�ɂȂ�܂Ŏ�t�֎~�𑗐M����
			if (Suica_Rec.Data.BIT.CTRL == 1 || Suica_Rec.Data.BIT.OPE_CTRL == 1) {
// MH810103 GG119202(E) E3209���������Ă��܂����̑΍�
			// �����c���Ɖ�^�C���A�E�g���̃��[�_��ʑJ�ڌ_�@������f�[�^�i��t���j��M�̂���
			// �x�Ə�ԂŐ���f�[�^�i��t���j�𑗐M���邪�A�c���Ɖ�{�^�����\������Ă��܂��B
			// ���̂��߁A��U�A��t�ɂ��Ă����t�s�ɂ���b��΍���s���B
			// �c���Ɖ�V�[�P���X�ύX�̖{�΍��ɂ��̏������폜����B
			Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );								// ����f�[�^(��t�֎~)���M
// MH810103 GG119202(S) E3209���������Ă��܂����̑΍�
			}
// MH810103 GG119202(E) E3209���������Ă��܂����̑΍�
		}
		break;
// MH810103 GG119202(E) �c���Ɖ�{�^����\���̎b��΍�
	}
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	Suica_Rec.suica_rcv_event.EC_BIT.STATUS_DATA = 0;
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			���(���ό���)�f�[�^����
//[]----------------------------------------------------------------------[]
///	@param[in]		*ope_faze	�F���݂�Ope̪���
///	@return			ret			�F0�Œ�
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static
short	EcRecvSettData(uchar *ope_faze)
{
	short	ret = 0;
	uchar	brand_index = 0;

// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	if ((Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA == 1) && 		// ���ό����ް�����M��
//		(Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA == 0))		// ���ό����ް��������̏ꍇ
	if ((Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA == 1) && 		// ���ό����ް�����M��
		(Suica_Rec.suica_rcv_event_sumi.EC_BIT.SETTLEMENT_DATA == 0))	// ���ό����ް��������̏ꍇ
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
	{
		Suica_Rec.Data.BIT.SETTLMNT_STS_RCV = 0;						// �݂Ȃ����σg���K����OFF
		switch (OPECTL.Ope_mod) {
		case 2:															// �����\��,���������t�F�[�Y
			// �u���ό��ʃf�[�^�v�d���������ł���t�F�[�Y
			// �i�����ł�����M���Ȃ��͂��j
			break;
		default:	// �u���ό��ʃf�[�^�v�d�����������Ȃ������t�F�[�Y�Ŏ�M�����ꍇ
			EcSettlementPhaseError((uchar*)&Ec_Settlement_Res, 1);		// �G���[�o�^����
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//			Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA = 1;		// ���ό��ʃf�[�^�����ςɂ���
			Suica_Rec.suica_rcv_event_sumi.EC_BIT.SETTLEMENT_DATA = 1;	// ���ό��ʃf�[�^�����ςɂ���
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
			break;
		}
	}

	switch (*ope_faze) {
	case	2:
	// ���Z��
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//		if (Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA == 0) {	// ���ό����ް���M�ς݂Ŗ������̏ꍇ
//			Suica_Rec.suica_rcv_event_sumi.BIT.SETTLEMENT_DATA = 1;
		if (Suica_Rec.suica_rcv_event_sumi.EC_BIT.SETTLEMENT_DATA == 0) {	// ���ό����ް���M�ς݂Ŗ������̏ꍇ
			Suica_Rec.suica_rcv_event_sumi.EC_BIT.SETTLEMENT_DATA = 1;
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
			
			brand_index = convert_brandno(Ec_Settlement_Res.brand_no);
			if (dsp_change) {											// ���Z����ү���ނ��\�������܂܂̏ꍇ
				op_SuicaFusokuOff();
// MH810103 GG119202(S) �����d�l�ύX�Ή�
//				if (Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO) {	// �����m��
//					Suica_Rec.Data.BIT.MIRYO_CONFIRM = 1;				// �����m���M
//
//					// �����c���Ɖ�\��
//					lcd_wmsg_dsp_elec( 1, ERR_CHR[20], ERR_CHR[53], 0, 0, COLOR_RED, LCD_BLINK_OFF); // "�J�[�h����Ɏ��s���܂���"
//																									 // "�������������c���Ɖ���s���܂�"
//					dsp_change = 1;
//					// �s���u�����h�H || �u�����h�ԍ�0�H
//					if( brand_index == EC_UNKNOWN_USED || brand_index == EC_ZERO_USED ){
//						// �I�����i�f�[�^�ő��M�����u�����h�����ό��ʂŎ�M�������ɂ���
//						Ec_Settlement_Res.brand_no = Product_Select_Brand;
//						brand_index = convert_brandno(Product_Select_Brand);
//					}
//					ec_alarm_syuu((uchar)brand_index, (ulong)Product_Select_Data);	// �A���[������W�v
//					ReqEcAlmRctPrint(J_PRI, &Ec_Settlement_Res);					// �A���[�����V�[�g�󎚗v��
//					EcAlarmLog_Regist(&Ec_Settlement_Res);							// ��������������O�o�^
//					break;
//				}
//				else {
//					// �����m��ȊO����M
//					dsp_change = 0;
//				}
				dsp_change = 0;
// MH810103 GG119202(E) �����d�l�ύX�Ή�
			}
			Ec_Pri_Data_Snd(S_CNTL_DATA, 0x80);							// �����ް��i����I���j�𑗐M����

			Lagcan( OPETCBNO, TIMERNO_EC_INQUIRY_WAIT );				// �⍇��(������)���ʑ҂��^�C�}�[���Z�b�g
			ec_MessageAnaOnOff( 0, 0 );
// MH810105 GG119202(S) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
			Ope2_ErrChrCyclicDispStop();								// �T�C�N���b�N�\����~
// MH810105 GG119202(E) �����ă^�b�`�҂����b�Z�[�W�\���Ή�

			switch (Ec_Settlement_Res.Result) {
			case	EPAY_RESULT_CANCEL_PAY_OFF:							// �����L�����Z��

// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810103 GG119202(S) �J�[�h�^�b�`��Ɏx�����I����ʂɖ߂�Ȃ�
//				if (check_enable_multisettle() > 1 &&
//					OPECTL.InquiryFlg == 0) {
//					// �}���`�u�����h���̏����L�����Z��(�J�[�h�������Ȃ�)�͐��Z���~���Ȃ�
//					// �x�����I����ʂɖ߂�
				if (check_enable_multisettle() > 1) {
					// �d�q�}�l�[���ώ��i�}���`�u�����h�j�̏����L�����Z���͎x�����I����ʂɖ߂�
					// �N���W�b�g�J�[�h���ώ��i�}���`�u�����h�j��
					// �����L�����Z���i�J�[�h��������M�O�j�͎x�����I����ʂɖ߂�
					if (RecvBrandResTbl.no == BRANDNO_CREDIT &&
						OPECTL.InquiryFlg != 0) {
						// �N���W�b�g�J�[�h���ώ��i�}���`�u�����h�j��
						// �����L�����Z���i�J�[�h��������M��j�͐��Z���~����
						goto EC_CANCEL_PAY_OFF_DUMMY;
					}
// MH810103 GG119202(S) �d�q�}�l�[�^�b�`��̃L�����Z���œ����ł��Ȃ�
					if (OPECTL.InquiryFlg != 0) {
						// �d�q�}�l�[�ǂݎ��҂���ʂ̂݃^�b�`����L�����Z���{�^�������\
						// ����NG��M���Ɠ��l�̏���������
						ac_flg.ec_deemed_fg = 0;								// �݂Ȃ����ϕ��d�p�t���OOFF
						Lagcan( OPETCBNO, 1 );									// ��ϰ1ؾ��(���ڰ��ݐ���p)
						Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );// ��ϰ8(XXs)�N��(������߂蔻����ϰ)
						queset(OPETCBNO, EC_EVT_CRE_PAID_NG, 0, 0);				// ope�֒ʒm
					}
// MH810103 GG119202(E) �d�q�}�l�[�^�b�`��̃L�����Z���œ����ł��Ȃ�
					OPECTL.InquiryFlg = 0;								// �O���Ɖ�t���OOFF
// MH810103 GG119202(E) �J�[�h�^�b�`��Ɏx�����I����ʂɖ߂�Ȃ�
// MH810103 GG119202(S) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
					EcEnableNyukin();
// MH810103 GG119202(E) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
					EcBrandClear();
					Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ�N��
					break;
				}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810103 GG118808_GG118908(S) �d�q�}�l�[�V���O���ݒ�̓ǂݎ��҂��^�C���A�E�g�����ύX
				else {
					if (RecvBrandResTbl.no == BRANDNO_CREDIT) {
						// �N���W�b�g�J�[�h���ώ��̏����L�����Z���͐��Z���~����
						goto EC_CANCEL_PAY_OFF_DUMMY;
					}
					// �d�q�}�l�[�V���O���ݒ�ŃL�����Z���{�^�����������ꍇ��
					// �x�����@�I����ʂ�\������
					ECCTL.ec_single_timeout = 1;
					if (OPECTL.InquiryFlg != 0) {
						// �d�q�}�l�[�ǂݎ��҂���ʂ̂݃^�b�`����L�����Z���{�^�������\
						// ����NG��M���Ɠ��l�̏���������
						ac_flg.ec_deemed_fg = 0;						// �݂Ȃ����ϕ��d�p�t���OOFF
						Lagcan( OPETCBNO, 1 );							// ��ϰ1ؾ��(���ڰ��ݐ���p)
						Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );// ��ϰ8(XXs)�N��(������߂蔻����ϰ)
						queset(OPETCBNO, EC_EVT_CRE_PAID_NG, 0, 0);		// ope�֒ʒm
					}
					OPECTL.InquiryFlg = 0;								// �O���Ɖ�t���OOFF
					EcBrandClear();
					Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ�N��
					break;
				}
// MH810103 GG118808_GG118908(E) �d�q�}�l�[�V���O���ݒ�̓ǂݎ��҂��^�C���A�E�g�����ύX
// MH810103 GG119202(S) �d�q�}�l�[�V���O���͌���NG�i308�j��M�Ő��Z���~����
EC_CANCEL_PAY_OFF_DUMMY:
// MH810103 GG119202(E) �d�q�}�l�[�V���O���͌���NG�i308�j��M�Ő��Z���~����
// MH810103 GG119202(S) ���ϐ��Z���~�󎚏����C��
//				ac_flg.ec_alarm = 0;									// �݂Ȃ����ϕ��d�p�t���OOFF
				ac_flg.ec_deemed_fg = 0;								// �݂Ȃ����ϕ��d�p�t���OOFF
// MH810103 GG119202(E) ���ϐ��Z���~�󎚏����C��
				Lagcan(OPETCBNO, 8);									// ������߂蔻����ϰؾ��(���ڰ��ݐ���p)
				Lagcan(OPETCBNO, 1);									// ���ڰ��݊֘A��ϰؾ��
				Lagcan(OPETCBNO, TIMERNO_EC_CYCLIC_DISP);				// �T�C�N���b�N�\���^�C�}�[ؾ��

				if (OPECTL.InquiryFlg == 0) {							// �J�[�h��������M�Ȃ�
					// �ۗ����Ă����C�x���g���ĊJ����
					if(OPECTL.holdPayoutForEcStop) {
						OPECTL.holdPayoutForEcStop = 0;
						queset( OPETCBNO, COIN_EN_EVT, 0, 0 );
					}
					if (EC_STATUS_DATA.StatusInfo.ReceptStatus) {		// ��t�s���
						if ((OPECTL.op_faz == 8 && 						// �d�q�}�̒�~�҂����킹��(���Z����)
							CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END) ||
							OPECTL.op_faz == 3 ||						// �Ƃ肯��
							OPECTL.op_faz == 9) {						// �d�q�}�̒�~�҂����킹��(�Ƃ肯��)
							break;
						}
					}
				}
				OPECTL.InquiryFlg = 0;									// �O���Ɖ�t���OOFF
				queset(OPETCBNO, EC_EVT_CANCEL_PAY_OFF, 0, 0);			// ope�֒ʒm
				break;
			case	EPAY_RESULT_NG:										// ����NG
			case	EPAY_RESULT_CANCEL:									// ���σL�����Z��
			// ����NG(�g�p�s�¶������,�c���s���Ȃ�)��M��

// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//				if( ec_Disp_No == 99 ){									// �^�C���A�E�g�G���[����M����
				if( ECCTL.ec_Disp_No == 99 ){							// �^�C���A�E�g�G���[����M����
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
// MH810105 GG119202(S) iD�̓ǂݎ��҂��^�C���A�E�g�ňē��\��
					if (RecvBrandResTbl.no == BRANDNO_ID) {
						// iD�̃^�C���A�E�g�݈̂ē��\������
						ECCTL.ec_Disp_No = 89;
						ec_WarningMessage((ushort)ECCTL.ec_Disp_No);
					}
// MH810105 GG119202(E) iD�̓ǂݎ��҂��^�C���A�E�g�ňē��\��
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
					else if (RecvBrandResTbl.no == BRANDNO_QR) {
						// QR�R�[�h���ς̃^�C���A�E�g�ē��\������
						ECCTL.ec_Disp_No = 16;
						ec_WarningMessage((ushort)ECCTL.ec_Disp_No);
					}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810103 GG119202(S) �d�q�}�l�[�V���O���͌���NG�i308�j��M�Ő��Z���~����
//					// �^�C���A�E�g�G���[��M���͉����������Ȃ�
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//					if (ECCTL.brand_num == 1) {
					if (check_enable_multisettle() == 1) {
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
						// �N���W�b�g�J�[�h�i�V���O���j
						if (RecvBrandResTbl.no == BRANDNO_CREDIT) {
							// �������Ȃ�
						}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//						// �d�q�}�l�[�i�V���O���j
//						else if (isEcBrandNoEMoney(RecvBrandResTbl.no, 0)) {
						// �d�q�}�l�[�i�V���O���jor QR�i�V���O���j
						else if (EcUseKindCheck(convert_brandno(RecvBrandResTbl.no))) {
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810103 GG118808_GG118908(S) �d�q�}�l�[�V���O���ݒ�̓ǂݎ��҂��^�C���A�E�g�����ύX
//							// ���Z���~�������s��
//							goto EC_CANCEL_PAY_OFF_DUMMY;
							// �d�q�}�l�[�V���O���ݒ�œǂݎ��҂��^�C���A�E�g�����ꍇ��
							// �x�����@�I����ʂ�\������
							ECCTL.ec_single_timeout = 1;
							EcBrandClear();
							Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ�N��
// MH810103 GG118808_GG118908(E) �d�q�}�l�[�V���O���ݒ�̓ǂݎ��҂��^�C���A�E�g�����ύX
						}
					}
					else {
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//						// �}���`�u�����h���̓u�����h�Ɋւ�炸�������Ȃ�
						// �}���`�u�����h���̃^�C���A�E�g�͐��Z���~���Ȃ�
						// �x�����I����ʂɖ߂�
// MH810103 GG119202(S) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
						EcEnableNyukin();
// MH810103 GG119202(E) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
						EcBrandClear();
						Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ�N��
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
					}
// MH810103 GG119202(E) �d�q�}�l�[�V���O���͌���NG�i308�j��M�Ő��Z���~����
					break;
				}

// MH810103 GG119202(S) ���ϐ��Z���~�󎚏����C��
//				ac_flg.ec_alarm = 0;									// �݂Ȃ����ϕ��d�p�t���OOFF
				ac_flg.ec_deemed_fg = 0;								// �݂Ȃ����ϕ��d�p�t���OOFF
// MH810103 GG119202(E) ���ϐ��Z���~�󎚏����C��
				Lagcan( OPETCBNO, 1 );									// ��ϰ1ؾ��(���ڰ��ݐ���p)
				Lagtim( OPETCBNO, 8, (ushort)(CPrmSS[S_TYP][66]*50+1) );// ��ϰ8(XXs)�N��(������߂蔻����ϰ)

				if (OPECTL.InquiryFlg != 0) {							// �J�[�h��������M��
					OPECTL.InquiryFlg = 0;								// �O���Ɖ�t���OOFF
				//���ēx���Z�����邽�߁AholdPayoutForEcStop�̔��f��opemain�ōs��
					queset(OPETCBNO, EC_EVT_CRE_PAID_NG, 0, 0);			// ope�֒ʒm
				}
				else {													// �J�[�h��������M�����Ɍ��ό��ʎ�M
				//���J�[�h�ǎ�莸�s���Ȃ�
					// �ۗ����Ă����C�x���g���ĊJ����
					if(OPECTL.holdPayoutForEcStop) {
						OPECTL.holdPayoutForEcStop = 0;
						queset( OPETCBNO, COIN_EN_EVT, 0, 0 );
					}
				//�����ό��ʂ��󂯂��Ƃ��A��ԃf�[�^�͎�t�s������͂�
					if (EC_STATUS_DATA.StatusInfo.ReceptStatus) {		// ��t�s���
						if ((OPECTL.op_faz == 8 &&						// �d�q�}�̒�~�҂����킹��(���Z����)
							CCT_Cansel_Status.BIT.STOP_REASON == REASON_PAY_END) ||
							OPECTL.op_faz == 3 ||						// �Ƃ肯��
							OPECTL.op_faz == 9) {						// �d�q�}�̒�~�҂����킹��(�Ƃ肯��)
							break;
						}
						// ���ލ������H
						else if( Suica_Rec.Data.BIT.CTRL_CARD ){
				//�����́i�J�[�h�������ł͂Ȃ�����NG�A�L�����Z���ɂȂ�j�ꍇ�A�J�[�h����Ƃ͂Ȃ�Ȃ��͂�
							if( OPECTL.op_faz == 8 &&
								(CCT_Cansel_Status.BIT.STOP_REASON == REASON_CARD_READ)) {	// STOP_REASON���J�[�h�}���̏ꍇ�Ͷ��ނ̓f���߂����s��
								if( OPECTL.other_machine_card ) {
									opr_snd( 13 );						// �ۗ��ʒu����ł��߂�
								}
								else {
									opr_snd( 2 );						// �O�r�o
								}
								CCT_Cansel_Status.BYTE = 0;				// ��~�҂��X�e�[�^�X���N���A
							}
							else {
								read_sht_cls();							// ���Cذ�ް������۰��
							}
							cn_stat( 2, 2 );							// �����s��
						}
					}
// MH810103 GG119202(S) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
					if (Suica_Rec.suica_err_event.BIT.SETTLEMENT_ERR != 0) {
						EcEnableNyukin();
						EcBrandClear();
					}
// MH810103 GG119202(E) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
				}

				// NG��M���̃��b�Z�[�W�A�A�i�E���X
// MH810105 GG119202(S) ���Ϗ������ɏ�Q�������������̓���
				if( Ec_Settlement_Res.Column_No == 0xFF ){				// �݂Ȃ����ςɂ�錋��NG�����Ȃ�G���[�\�������Ȃ�
					Ec_Settlement_Res.Column_No = 0;
				}
				else {
// MH810105 GG119202(E) ���Ϗ������ɏ�Q�������������̓���
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//				ec_WarningMessage(ec_Disp_No);
				ec_WarningMessage(ECCTL.ec_Disp_No);
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
// MH810105 GG119202(S) ���Ϗ������ɏ�Q�������������̓���
				}
// MH810105 GG119202(E) ���Ϗ������ɏ�Q�������������̓���
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
				if (RecvBrandResTbl.no == BRANDNO_QR) {
					if (!IsQrContinueReading(Ec_Settlement_Rec_Code)) {
						EcEnableNyukin();
						EcBrandClear();
					}
				} else {
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105 GG119202(S) �d�q�}�l�[���σG���[�������̉�ʖ߂�d�l�ύX
				if (!IsEcContinueReading(Ec_Settlement_Rec_Code)) {
					if (isEcBrandNoEMoney(RecvBrandResTbl.no, 0)) {
						// �d�q�}�l�[���ςŎx�����@�I����ʂɖ߂�G���[
						EcEnableNyukin();
						EcBrandClear();
					}
				}
// MH810105 GG119202(E) �d�q�}�l�[���σG���[�������̉�ʖ߂�d�l�ύX
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
				}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

				// ���ނ���H
				if( Suica_Rec.Data.BIT.CTRL_CARD ){
					// ���ލ������H NG���b�Z�[�W�\����(6�b)�҂��ăJ�[�h�������ųݽ�J�n
					LagTim500ms(LAG500_EC_START_REMOVAL_TIMER, (short)(6*2+1), ec_start_removal_wait);
				}
				Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[0]*50) );	// ���p�\�}�̻��د��\���p��ϰ�N��(�T�C�N���b�N�\���͂�����)
				break;
			case	EPAY_RESULT_OK:										// ����OK
// MH810103 GG119202(S) �����d�l�ύX�Ή�
//			case	EPAY_RESULT_MIRYO_AFTER_OK:							// �����m��㌈��OK
// MH810103 GG119202(E) �����d�l�ύX�Ή�
			case	EPAY_RESULT_MIRYO_ZANDAKA_END:						// �����c���Ɖ��
				// ���ό��ʂ����������̎��i�c���s����̍ă^�b�`�����܂ށj

				if (Suica_Rec.Data.BIT.CTRL_MIRYO != 0) {				// ������Ԕ������H 
					LagCan500ms(LAG500_SUICA_MIRYO_RESET_TIMER);		// ������ԊĎ����ؾ��
					Suica_Rec.Data.BIT.CTRL_MIRYO = 0;					// ������ԉ���
// MH810103 GG119202(S) �����d�l�ύX�Ή�
					Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;				// �����m��(��ԃf�[�^)��M���N���A
// MH810103 GG119202(E) �����d�l�ύX�Ή�
				}
				Lagcan(OPETCBNO, 8);									// ������߂蔻����ϰؾ��(���ڰ��ݐ���p)
				Lagcan(OPETCBNO, 1);									// ���ڰ��݊֘A��ϰؾ��
				Lagcan(OPETCBNO, TIMERNO_EC_CYCLIC_DISP);				// �T�C�N���b�N�\���^�C�}�[ؾ��
				nyukin_delay_check(nyukin_delay, delay_count);			// �ۗ����̲���Ă�����΍�post����

				// �s���u�����h�H || �u�����h�ԍ�0�H
				if( brand_index == EC_UNKNOWN_USED || brand_index == EC_ZERO_USED ){
					// �I�����i�f�[�^�ő��M�����u�����h�����ό��ʂŎ�M�������ɂ���
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//					Ec_Settlement_Res.brand_no = Product_Select_Brand;
//					brand_index = convert_brandno(Product_Select_Brand);
					Ec_Settlement_Res.brand_no = ECCTL.Product_Select_Brand;
					brand_index = convert_brandno(ECCTL.Product_Select_Brand);
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
				}
				// �I�����i�f�[�^�ɂđ��M�����u�����h�ԍ��ƈ�v���Ȃ��H
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//				else if ( Product_Select_Brand != Ec_Settlement_Res.brand_no ){
				else if ( ECCTL.Product_Select_Brand != Ec_Settlement_Res.brand_no ){
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
					// �u�����h�s��v�t���O
					PayData.Electron_data.Ec.E_Status.BIT.brand_mismatch = 1;
				}

				// ���σu�����h���̌��Ϗ��������{
				switch(brand_index) {
					case EC_EDY_USED:
					case EC_NANACO_USED:
					case EC_WAON_USED:
					case EC_KOUTSUU_USED:
					case EC_SAPICA_USED:
					case EC_ID_USED:
					case EC_QUIC_PAY_USED:
// MH810105(S) MH364301 PiTaPa�Ή�
					case EC_PITAPA_USED:
// MH810105(E) MH364301 PiTaPa�Ή�
						e_inflg = 1;										// �d�q�}�l�[���ςł���
						e_pay = Ec_Settlement_Res.settlement_data;			// �d�q�}�l�[���ϊz��ۑ�
// MH810103 GG119202(S) �݂Ȃ����ώ��͎c���\�����Ȃ�
// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//						if (Ec_Settlement_Res.Column_No == 0xFF) {
						if (Ec_Settlement_Res.Column_No == 0xFF ||
// MH810105(S) MH364301 PiTaPa�Ή�
//							brand_index == EC_ID_USED ||
//							brand_index == EC_QUIC_PAY_USED) {
							EcBrandPostPayCheck(brand_index)) {
// MH810105(E) MH364301 PiTaPa�Ή�
// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
							e_zandaka = -1;									// �݂Ȃ����ώ��͎c�����\���Ƃ���
						}
						else {
// MH810103 GG119202(E) �݂Ȃ����ώ��͎c���\�����Ȃ�
						e_zandaka = Ec_Settlement_Res.settlement_data_after;// �d�q�}�l�[�c����ۑ�
// MH810103 GG119202(S) �݂Ȃ����ώ��͎c���\�����Ȃ�
						}
// MH810103 GG119202(E) �݂Ȃ����ώ��͎c���\�����Ȃ�
						EcElectronSet_PayData(&Ec_Settlement_Res);			// �d�q�}�l�[�Ő��Z
						ret = al_emony(SUICA_EVT , 0);						// �d�q�}�l�[���Z����
// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
// MH810105 GG119202(S) ������������W�v�d�l���P
//						if (Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO_ZANDAKA_END ||
//							Suica_Rec.Data.BIT.MIRYO_NO_ANSWER != 0) {
//							// �����c���Ɖ���A�܂��́A�݂Ȃ����ρi�������j�͖����񐔂��J�E���g����
//							ec_alarm_syuu(brand_index, (ulong)Product_Select_Data);	// �A���[������W�v
//							if (Suica_Rec.Data.BIT.MIRYO_NO_ANSWER != 0) {
//								// �݂Ȃ����ς̏ꍇ�݂͂Ȃ��p���ό��ʃf�[�^���쐬����
//								EcSetDeemedSettlementResult(&Ec_Settlement_Res);
//							}
						if (Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO_ZANDAKA_END) {
// MH810105 GG119202(E) ������������W�v�d�l���P
							ReqEcAlmRctPrint(J_PRI, &Ec_Settlement_Res);			// �A���[�����V�[�g�󎚗v��
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
//							EcAlarmLog_Regist(&Ec_Settlement_Res);					// ��������������O�o�^
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
						}
// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
						break;
					case EC_CREDIT_USED:
						EcCreUpdatePayData(&Ec_Settlement_Res);				// �N���W�b�g���σf�[�^�ۑ�
						ret = al_emony(EC_EVT_CRE_PAID , 0);				// �N���W�b�g���Z����
						break;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
					case EC_QR_USED:
						q_inflg = 1;										// QR�R�[�h���ςł���
						e_pay = Ec_Settlement_Res.settlement_data;			// QR�R�[�h���ϊz��ۑ�
						e_zandaka = -1;										// ��\���Ƃ���
						EcQrSet_PayData(&Ec_Settlement_Res);				// QR�R�[�h�Ő��Z
						ret = al_emony(EC_EVT_QR_PAID , 0);					// QR�R�[�h���Z����
						break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
					default:
						break;
				}

// MH810103 GG119202(S) ���ϒ�~�G���[�̓o�^
				if (Ec_Settlement_Res.Column_No != 0xFF) {
// MH810103 GG119202(E) ���ϒ�~�G���[�̓o�^
				// 301�G���[����
				err_chk((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_301, 0, 0, 0);
// MH810103 GG119202(S) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
				// �d�q�}�l�[�n�d��ڍ׃G���[����
				err_chk((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_CRITICAL_ERR, 0, 0, 0);
// MH810103 GG119202(E) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
// MH810103 GG119202(S) ���ϒ�~�G���[�̓o�^
					// ����OK��M��JVMA���Z�b�g�ώZ�񐔂��N���A
					Ec_Jvma_Reset_Count = 0;
				}
// MH810103 GG119202(E) ���ϒ�~�G���[�̓o�^

				if (brand_index != EC_CREDIT_USED && ryo_buf.dsp_ryo) {
					EcSendCtrlEnableData();								// ����f�[�^(��t����)���M
					if (OPECTL.op_faz == 3) {							// ��������̏ꍇ
						Op_Cansel_Wait_sub(0);							// CRW-MJA��~�ςݏ������s
					}
				}
				// �ۗ����Ă����C�x���g���ĊJ����
				if(OPECTL.holdPayoutForEcStop) {
					OPECTL.holdPayoutForEcStop = 0;
					queset( OPETCBNO, COIN_EN_EVT, 0, 0 );
				}
				break;
// MH810103 GG119202(S) �����d�l�ύX�Ή�
			case	EPAY_RESULT_MIRYO:									// ���ϖ����m��
// MH810103 GG119202(S) �����c���Ɖ�^�C���A�E�g���Ă���10���o�߂Ő���f�[�^�i��t���j�����M�����
				Lagcan(OPETCBNO, 8);									// ������߂蔻����ϰؾ��(���ڰ��ݐ���p)
// MH810103 GG119202(E) �����c���Ɖ�^�C���A�E�g���Ă���10���o�߂Ő���f�[�^�i��t���j�����M�����
				LagCan500ms(LAG500_SUICA_MIRYO_RESET_TIMER);			// ������ԊĎ����ؾ��
				if (Suica_Rec.Data.BIT.CTRL_MIRYO != 0) {				// ������Ԕ������H 
					Suica_Rec.Data.BIT.CTRL_MIRYO = 0;					// ������ԉ���
					Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;				// �����m��(��ԃf�[�^)��M���N���A
					wmonlg(OPMON_EC_MIRYO_TIMEOUT, NULL, 0);			// ���j�^�o�^
				}
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
//// MH810103 GG118808_GG118908(S) ���������Ɠ����ɃJ�[�h�^�b�`�����������Ń��b�N����
//				if( ryo_buf.nyukin && OPECTL.credit_in_mony ){			// ��������œd�q�}�l�[�Ƃ̂���Ⴂ�����̏ꍇ
//					// �����z >= ���ԗ����̏�ԂŖ����m�����M�����ꍇ��
//					// ���Z�����Ƃ��Ĉ����A�Ƃ肯���{�^�������҂��ɂ��Ȃ�
//					if( OPECTL.credit_in_mony == 1 ){
//						in_mony ( COIN_EVT, 0 );						// ��ݲ���ē���������āA��������(ryo_buf�ر�̍X�V)���s��
//					}
//					else{/* OPECTL.credit_in_mony == 2 */
//						in_mony ( NOTE_EVT, 0 );						// ��������ē���������āA��������(ryo_buf�ر�̍X�V)���s��
//					}
//					OPECTL.credit_in_mony = 0;							// ����Ⴂ�׸޸ر
//				}
//// MH810103 GG118808_GG118908(E) ���������Ɠ����ɃJ�[�h�^�b�`�����������Ń��b�N����
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
				OPECTL.InquiryFlg = 0;									// �O���Ɖ�t���OOFF
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
//// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//				ac_flg.ec_deemed_fg = 0;								// �݂Ȃ����ϕ��d�p�t���OOFF
//// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
// MH810105 GG119202(S) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
				ope_anm( AVM_STOP );									// ������~�ųݽ
// MH810105 GG119202(E) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
				Lagcan(OPETCBNO, TIMERNO_EC_CYCLIC_DISP);				// �T�C�N���b�N�\���^�C�}�[ؾ��
				nyukin_delay_check(nyukin_delay, delay_count);			// �ۗ����̲���Ă�����΍�post����

				ec_miryo_timeout_after_disp(&Ec_Settlement_Res);		// ������s/��������ʕ\��

// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
// ������������W�v�A�A���[�����V�[�g�󎚗v���A��������������O�o�^��ec_miryo_timeout_after_disp�ɂčs���悤�ɕύX
//				if( brand_index == EC_UNKNOWN_USED || brand_index == EC_ZERO_USED ){
//					// �I�����i�f�[�^�ő��M�����u�����h�����ό��ʂŎ�M�������ɂ���
//					Ec_Settlement_Res.brand_no = ECCTL.Product_Select_Brand;
//					brand_index = convert_brandno(ECCTL.Product_Select_Brand);
//				}
//// MH810105 GG119202(S) ������������W�v�d�l���P
////				ec_alarm_syuu((uchar)brand_index, (ulong)Product_Select_Data);	// �A���[������W�v
////				ReqEcAlmRctPrint(J_PRI, &Ec_Settlement_Res);					// �A���[�����V�[�g�󎚗v��
////				EcAlarmLog_Regist(&Ec_Settlement_Res);							// ��������������O�o�^
//				if (ECCTL.ec_Disp_No != 96 &&									// ����������m�F�ł͂Ȃ�
//					brand_index != EC_ID_USED &&								// iD���ςł͂Ȃ�
//					brand_index != EC_QUIC_PAY_USED) {							// QUICPay���ςł͂Ȃ�
//					// ���ϖ����m��͏�����������̎x���s���񐔂��J�E���g����
//					ec_alarm_syuu(0, (ulong)Product_Select_Data);		// ������������i�x���s���j�W�v
//					ReqEcAlmRctPrint(J_PRI, &Ec_Settlement_Res);		// �A���[�����V�[�g�󎚗v��
//					EcAlarmLog_Regist(&Ec_Settlement_Res);				// ��������������O�o�^
//				}
//// MH810105 GG119202(E) ������������W�v�d�l���P
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
				break;
// MH810103 GG119202(E) �����d�l�ύX�Ή�
			default:
				break;
			}
		}
		break;
	default:
		break;
	}

	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			�u�����h�I�����ʃf�[�^����
//[]----------------------------------------------------------------------[]
///	@param[in]		*ope_faze	�F���݂�Ope̪���
///	@return			ret			�F0�Œ�
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/13<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
short	EcRecvBrandData(uchar *ope_faze)
{
	short	ret = 0;
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	uchar	inq;
	uchar	can;
// GG129000(S) #7001 �y�A�~���v���U���������n���No.20�z�����؂��JQ�J�[�h��ǂݎ����QR�R�[�h���[�_�[���ǂݎ��ĊJ���Ȃ��iGM803002���p�j
	ushort	res_wait_time;

	res_wait_time = (ushort)prm_get( COM_PRM, S_PKT, 21, 2, 1 );	// �f�[�^��M�Ď��^�C�}
	if( res_wait_time == 0 || res_wait_time > 99 ){
		res_wait_time = 5;	// �͈͊O��5�b�Ƃ���
	}
// GG129000(E) #7001 �y�A�~���v���U���������n���No.20�z�����؂��JQ�J�[�h��ǂݎ����QR�R�[�h���[�_�[���ǂݎ��ĊJ���Ȃ��iGM803002���p�j

	if (isEC_STS_CARD_PROC()) {							// ���Ϗ�� �������̏ꍇ�͑҂����킹��B
		return ret;										// ����
	}
	Suica_Rec.suica_rcv_event.EC_BIT.BRAND_DATA = 0;
	inq = OPECTL.InquiryFlg;			// ���Ϗ�Ԃ̒ʒm���󂯂Ă�����
	OPECTL.InquiryFlg = 0;
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810103 GG119202(S) �n�E�X�J�[�h�ɂ��J�[�h�������^�C���A�E�g�͐��Z���~�Ƃ���
	if (inq != 0) {
		Lagcan( OPETCBNO, TIMERNO_EC_INQUIRY_WAIT );	// �⍇��(������)���ʑ҂��^�C�}�[���Z�b�g
	}
// MH810103 GG119202(E) �n�E�X�J�[�h�ɂ��J�[�h�������^�C���A�E�g�͐��Z���~�Ƃ���
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	can = (uchar)Suica_Rec.Data.BIT.BRAND_CAN;
	Suica_Rec.Data.BIT.BRAND_CAN = 0;	// ���~�v���Ɠ����ɑI������������
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j

	switch (*ope_faze) {
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//	case	0:	// �ҋ@���
//	case	1:	// �Ԏ��ԍ����͉��
//	case	3:	// ���Z����
	default:
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		break;

	case	2:		// ���Z��
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//		if (!Suica_Rec.Data.BIT.CTRL_MIRYO) {								// ���Z������ԂŖ����ꍇ�̂ݏ��������{
//			if (OPECTL.op_faz == 3 ||										// �d�q�}�̒�~�҂����킹���H
//				OPECTL.op_faz == 8) {
//				// �d�q�}�̒�~�҂����킹���ł̎�t��M���́A̪��ލ��킹�ׂ̈Ɏ�t�֎~�𑗐M����B
//				Ec_Pri_Data_Snd(S_CNTL_DATA, 0);
//				break;
//			}
//			if (OPECTL.op_faz >= 9 || 										// ���Z���~���̌��σ��[�_��~�҂����킹��(����ɒ�~���Ă��Ȃ��ꍇ)��
//				EC_STATUS_DATA.StatusInfo.Running) {						// ��M�X�e�[�^�X�����s��
//				break;
//			}
		if (Suica_Rec.Data.BIT.BRAND_SEL) {									// �u�����h�I�������Ă���ꍇ
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
			if (OPECTL.op_faz < 2) {										// ���Z��
				if(RecvBrandResTbl.res == EC_BRAND_SELECTED) {				// �u�����h��I������
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//					if(isEcBrandNoReady(RecvBrandResTbl.no)) {				// �I�������u�����h�͊J�ǁH�i�O�̂��߂̊m�F�j
//						if ((WAR_MONEY_CHECK && !e_incnt) ||				// ��������
//							(RecvBrandResTbl.no == BRANDNO_CREDIT) ) {		// �I�������u�����h���N���W�b�g
//							// �I�����i�f�[�^���M
//							if (w_settlement != 0) {
//								Ec_Data_Snd(S_SELECT_DATA, &w_settlement);
//							} else {
//								Ec_Pri_Data_Snd(S_SELECT_DATA, 0);
//								w_settlement = 0;
//							}
//						}
//					} else {
//						// ���肦�Ȃ����A���M�����u�����h�ȊO�̃u�����h���I������Ă���
//						// ��t�s�����蒼���i����t���u�����h�I���f�[�^�j
//						Ec_Pri_Data_Snd(S_CNTL_DATA, 0);
//					}
//				}
//				else {
//					// ���I���ɂ��ꂽ�̂Ő���f�[�^�i��t�֎~�j�𑗐M����
//					Ec_Pri_Data_Snd(S_CNTL_DATA, 0);
//
//				}
					if (IsBrandSpecified(RecvBrandResTbl.no)) {			// �I��v�����o�����u�����h�͎󂯕t����
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή��Q�l�j
						// ���ς���u�����h���I�����ꂽ�ꍇ�̓u�����h�I�����ʉ����f�[�^(OK)�𑗐M����
						// ���ςȂ��u�����h�^�㌈�σu�����h���I�����ꂽ�ꍇ�̓J�[�h�f�[�^�̃`�F�b�N���������Ƃ�
						// �u�����h�I�����ʉ����f�[�^(OK/NG)�𑗐M����
						if (!IsSettlementBrand(RecvBrandResTbl.no)) {
							// ���ςȂ��u�����h�^�㌈�σu�����h
							if (!EcNotifyCardData(RecvBrandResTbl.no, RecvBrandResTbl.dat)) {
								// ����`�̓u�����h�͂n�j����������ŃL�����Z����������
								Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 1);					// �u�����h�I�����ʉ���OK
								if (inq != 0) {
									ec_MessageAnaOnOff(0, 0);
									EnableNyukin();
								}
								queset( OPETCBNO, EC_BRAND_RESULT_CANCELED, 0, 0);
							}
						}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή��Q�l�j
						else {
						// ���σu�����h�͑I�����i�f�[�^���M
							if (can != 0) {
							// ����Ⴂ�Ŏ�t�֎~���M�ς݂Ȃ�΁A�I�����ʉ������M��A��t�s�ɂȂ�̂őI�����i�f�[�^���o���Ȃ��B
							// ��OK���ēǂݎ̂Ă�
								Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 1);					// �u�����h�I�����ʉ���OK
								queset( OPETCBNO, EC_BRAND_RESULT_CANCELED, 0, 0);
							}
							else {
								Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA_with_HOLD, 1);		// ���ϑI������̂Ƃ��A�u�����h�I�����ʉ����f�[�^�Z�b�g�𑗐M����
																							// �u�����h�I�����ʂ͕ێ�����
								Ec_Pri_Data_Snd(S_SELECT_DATA, 0);
								w_settlement = 0;
// MH810103 GG119202(S) �d�q�}�l�[�u�����h�I�����̈ē�����
								if (isEcBrandNoEMoney(RecvBrandResTbl.no, 0)) {
									// �d�q�}�l�[�u�����h�I�����̈ē�����
									ope_anm(AVM_SELECT_EMONEY);
// MH810103(s) �d�q�}�l�[�Ή� �d�q�}�l�[�u�����h�I�����̈ē�����
									lcdbm_notice_ope(LCDBM_OPCD_BRAND_SELECTED,convert_brandno(RecvBrandResTbl.no));
// MH810103(e) �d�q�}�l�[�Ή� �d�q�}�l�[�u�����h�I�����̈ē�����
// MH810103 GG119202(S) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
									cn_stat( 2, 2 );										// �����s��
									if (LCDNO == 3) {
										// �u�d�݂܂��͎����Ő��Z���ĉ������v��\���̂��ߍĕ`��
										OpeLcd(3);
									}
// MH810103 GG119202(E) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
								}
// MH810103 GG119202(E) �d�q�}�l�[�u�����h�I�����̈ē�����
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
								else if( RecvBrandResTbl.no == BRANDNO_QR ){
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100
									lcdbm_notice_ope(LCDBM_OPCD_BRAND_SELECTED,convert_brandno(RecvBrandResTbl.no));
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100
									cn_stat( 2, 2 );										// �����s��
									if (LCDNO == 3) {
										// �u�d�݂܂��͎����Ő��Z���ĉ������v��\���̂��ߍĕ`��
										OpeLcd(3);
									}
								}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6412 �x�Ƃ̈ē��\��(�|�b�v�A�b�v)����Ȃ����Ƃ�����
								else if( RecvBrandResTbl.no != BRANDNO_ZERO ){
									lcdbm_notice_ope(LCDBM_OPCD_BRAND_SELECTED,convert_brandno(RecvBrandResTbl.no));
								}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6412 �x�Ƃ̈ē��\��(�|�b�v�A�b�v)����Ȃ����Ƃ�����
							}
						}
						return ret;		// �����܂�
					}
				//	else	�������ēǂݎ̂�
				}
				else if (RecvBrandResTbl.res == EC_BRAND_CANCELED) {
					BrandResRsltTbl.no = RecvBrandResTbl.no;	// ���R�R�[�h�̓u�����h�I�����ʂŎ�M�����R�[�h�����̂܂܃Z�b�g����
					Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);	// �u�����h�I�����ʉ���NG
					if (inq != 0) {
						ec_MessageAnaOnOff(0, 0);
						EnableNyukin();
					}
					queset( OPETCBNO, EC_BRAND_RESULT_CANCELED, 0, 0);
					return ret;
				}
				else {
					BrandResRsltTbl.no = RecvBrandResTbl.no;	// ���R�R�[�h�̓u�����h�I�����ʂŎ�M�����R�[�h�����̂܂܃Z�b�g����
					Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);	// �u�����h�I�����ʉ���NG
					if (inq != 0) {
						ec_MessageAnaOnOff(0, 0);
						EnableNyukin();
					}
					queset( OPETCBNO, EC_BRAND_RESULT_UNSELECTED, 0, 0);
// GG129000(S) #7001 �y�A�~���v���U���������n���No.20�z�����؂��JQ�J�[�h��ǂݎ����QR�R�[�h���[�_�[���ǂݎ��ĊJ���Ȃ��iGM803002���p�j
					if (BrandResRsltTbl.no == EC_BRAND_REASON_205) {
						// �Ӑ}���Ȃ��J�[�h�̓ǎ掞
						// QR�J�n�v�����M
						lcdbm_notice_ope( LCDBM_OPCD_QR_STA_REQ, 0 );
						// �����҂��^�C�}�[�X�^�[�g
						Lagtim( OPETCBNO, TIMERNO_QR_START_RESP, (res_wait_time * 50) );
					}
// GG129000(E) #7001 �y�A�~���v���U���������n���No.20�z�����؂��JQ�J�[�h��ǂݎ����QR�R�[�h���[�_�[���ǂݎ��ĊJ���Ȃ��iGM803002���p�j
					return ret;
				}
			}
		}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		break;
	}
// MH810103 GG119202(S) �u�����h�I����̃L�����Z�������ύX
//	Suica_Rec.Data.BIT.SLCT_BRND_BACK = 0;
// MH810103 GG119202(E) �u�����h�I����̃L�����Z�������ύX
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// �����@�u�����h�I�����ʂ͕K����������K�v������@����
	// ���ς���u�����h�̏ꍇ�͓K�pOK�𑗐M����B(JVMA�R�}���h�d�l(�d�q����)_200817_Rev4.07�Q��)
	if ((RecvBrandResTbl.res == EC_BRAND_SELECTED) 
     && (IsSettlementBrand(RecvBrandResTbl.no))) {
		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 1);					// �u�����h�I�����ʉ���OK
	} else {
		BrandResRsltTbl.no = EC_BRAND_REASON_205;					// �Ӑ}���Ȃ��J�[�h�ǎ��
		Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0);					// �u�����h�I�����ʉ���NG
	}
	if (inq != 0) {													// ���Z�����ƃu�����h�I��������Ⴄ�ꍇ�A�I�����ʂ͓ǂݎ̂Ă�A
		ec_MessageAnaOnOff(0, 0);									// �I�����̃J�[�h�������������ŃN���A����
		EnableNyukin();
	}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810103 GG119202(S) �I���u�����h�������ł��Ȃ��ꍇ�̓���
	if (OPECTL.op_faz < 2) {										// ���Z��
		// �u�����h�I���f�[�^�đ����邽�߁A����f�[�^�i��t�֎~�j�𑗐M����
		Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );							// ����f�[�^(��t�֎~)���M
	}
// MH810103 GG119202(E) �I���u�����h�������ł��Ȃ��ꍇ�̓���

	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			����f�[�^��M̪��ވُ펞�̃G���[�o�^����
//[]----------------------------------------------------------------------[]
///	@param[in]		*dat �F ����f�[�^
///	@param[in]		kind �F 1:���ϕs��̪��ނŎ���f�[�^����M
///				   		 �F 2:Ope����������O�ɍēx����f�[�^����M
///				   		 �F 3:Ope����x���ς����̂ɂ��ւ�炸�A�ēx
///				   		 	  ����f�[�^����M�����ꍇ
///				   		 �F 4:����f�[�^����M��A���ZLOG���ɓo�^
///				   			  ����O��̪��ނ��ς�����ꍇ
///				   		 �F 5:��M��������f�[�^��OPE�^�X�N������
///				   			  ���Ȃ������ɐ��Z�@�����Z�b�g�����ꍇ
///				   		 �F 6:OPE�^�X�N����M�C�x���g����������O��
///				   			  ��M�L���[���t���ɂȂ�C�㏑�����悤��
///				   			  ���Ă���o�b�t�@�Ɋi�[����Ă���f�[�^
///				   			  ������f�[�^�������ꍇ
///				   		 �F 7:�C�j�V�����V�[�P���X�ŗL���Ȍ��ό��ʃf�[�^
///				   			  ����M�����ꍇ
///				   		 �F 8:���Z�J�n��A�I�����i�f�[�^�𑗐M���Ă��Ȃ�
///				   			  �ɂ�������炸���ό��ʃf�[�^����M�����ꍇ
///						 �F 9:���Z���~������Ɍ��ό��ʃf�[�^����M�����ꍇ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	EcSettlementPhaseError(uchar *dat, uchar kind)
{
	uchar	work_brand;
	long	work;
	EC_SETTLEMENT_RES *work_set;

	memset(err_wk, 0, sizeof(err_wk));
	memcpy(&err_wk[0], "\?\?-0000   00000 fz:000-00", 25);
	if (kind == 1 || kind == 4) {									// ���ό��ʃf�[�^�G���A�ɂ��łɃf�[�^���i�[����Ă���ꍇ�͂�����
		work_set = (EC_SETTLEMENT_RES *)dat;						// �f�[�^�Z�b�g

		// �J�[�h���
		work_brand = convert_brandno(work_set->brand_no);
		memcpy(&err_wk[0], EcBrandKind_prn[work_brand - EC_USED], 2);
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		if( work_brand == EC_QR_USED ){							// QR�̏ꍇ�A�x����ʂ��Z�b�g
			memset(&err_wk[3], 0x20, 4);						// �S�����X�y�[�X���Z�b�g
			memcpy(&err_wk[3], QrBrandKind_prn[work_set->Brand.Qr.PayKind], 2);
		}
		else{
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		// �J�[�h�ԍ�
		memset(Card_Work, 0x20, sizeof(Card_Work));
		memcpyFlushRight(Card_Work, work_set->Card_ID, sizeof(Card_Work), sizeof(work_set->Card_ID));
		memcpy(&err_wk[3], &Card_Work[16], 4);					// �J�[�h�ԍ��̉��S�����Z�b�g
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		// ������
		if (work_set->Result == EPAY_RESULT_MIRYO ||
// MH810103 GG119202(S) �����d�l�ύX�Ή�
//			work_set->Result == EPAY_RESULT_MIRYO_AFTER_OK ||
// MH810103 GG119202(E) �����d�l�ύX�Ή�
			work_set->Result == EPAY_RESULT_MIRYO_ZANDAKA_END) {
			memcpy(&err_wk[7], "(A)", 3);
		}
		// ����z
		work = work_set->settlement_data;
		if (work > 0xffff) {
		// ����z��short�l�ȏ�Ȃ�␳
			memcpy(&err_wk[10], "FFFFH", 5);
		} else {
			intoasl(&err_wk[10], work, 5);
		}
		// ���Z�@�������(OPE̪���)
		intoas(&err_wk[19], (ushort)OPECTL.Ope_mod, 3);				// �o�^���̵��̪��ނ��Z�b�g
		// �o�^�v��
		intoas(&err_wk[23], (ushort)kind, 2);						// �o�^��ʂ��Z�b�g
	}else{															// ��M�f�[�^�����̂܂܎g���ꍇ�͂�����
		if (kind == 5) {
			if (dat[0] == 0) {
				return;		// ���ό��ʁ��Ȃ��͓ǂݎ̂Ă�
			}
		}

		// �J�[�h���
		work_brand = convert_brandno( bcdbin3(&dat[2]) );
		memcpy(&err_wk[0], EcBrandKind_prn[work_brand - EC_USED], 2);
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		if( work_brand == EC_QR_USED ){							// QR�̏ꍇ�A�x����ʂ��Z�b�g
			memset(&err_wk[3], 0x20, 4);						// �S�����X�y�[�X���Z�b�g
			memcpy(&err_wk[3], QrBrandKind_prn[dat[44]], 2);
		}
		else{
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		// �J�[�h�ԍ�
		memset(Card_Work, 0x20, sizeof(Card_Work));
		memcpyFlushRight(Card_Work, &dat[13], sizeof(Card_Work), 20);
		memcpy(&err_wk[3], &Card_Work[16], 4);					// �J�[�h�ԍ��̉��S�����Z�b�g
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		// ������
		if (dat[0] == EPAY_RESULT_MIRYO ||
// MH810103 GG119202(S) �����d�l�ύX�Ή�
//			dat[0] == EPAY_RESULT_MIRYO_AFTER_OK ||
// MH810103 GG119202(E) �����d�l�ύX�Ή�
			dat[0] == EPAY_RESULT_MIRYO_ZANDAKA_END) {
			memcpy(&err_wk[7], "(A)", 3);
		}
		// ����z
		work = settlement_amount_ex(&dat[4], 3);					// ����z(BCD�`���̌��ϊz��Bin�`���ɕϊ�)
		if (work > 0xffff) {
		// ����z��short�l�ȏ�Ȃ�␳
			memcpy(&err_wk[10], "FFFFH", 5);
		} else {
			intoasl(&err_wk[10], work, 5);
		}
		// ���Z�@�������(OPE̪���)
		intoas(&err_wk[19], (ushort)OPECTL.Ope_mod, 3);				// �o�^���̵��̪��ނ��Z�b�g
		// �o�^�v��
		intoas(&err_wk[23], (ushort)kind, 2);						// �o�^��ʂ��Z�b�g
	}
// MH321800(S) �t�F�[�Y�G���[�������Ɍ���OK�ȊO�̓��[�_�[�؂藣�����Ȃ�
	// ���ό���OK����E3259��o�^�����[�_��؂藣��
	if( dat[0] == EPAY_RESULT_OK ){
// MH321800(E) �t�F�[�Y�G���[�������Ɍ���OK�ȊO�̓��[�_�[�؂藣�����Ȃ�
	err_chk2((char)jvma_setup.mdl, (char)ERR_EC_SETTLE_ERR, (char)1, (char)1, (char)1, (void*)err_wk);
			// E3259�o�^�i���ϊz�p�����[�^����FASCII�j "E3259   08/03/17 14:38   ����/����"
			//											"        ED-3456   01000 fz:003-1  "
			//											"        ED-3456(A)01000 fz:003-1  "
			//												 ��ʁA�J�[�h�ԍ���4���A�����ʁA����z�A���̎���OPmod�A�o�^�v��
// MH321800(S) �t�F�[�Y�G���[�������Ɍ���OK�ȊO�̓��[�_�[�؂藣�����Ȃ�
	}
	// ���ό���OK�ȊO��E3249��o�^�����[�_��؂藣���Ȃ�
	else {
// MH810105(S) MH364301 E3249�𔭐��^�����ʂɕύX
//		err_chk2((char)jvma_setup.mdl, (char)ERR_EC_NOTOK_SETTLE_ERR, (char)2, (char)1, (char)1, (void*)err_wk);
		err_chk2((char)jvma_setup.mdl, (char)ERR_EC_NOTOK_SETTLE_ERR, (char)1, (char)1, (char)1, (void*)err_wk);
// MH810105(E) MH364301 E3249�𔭐��^�����ʂɕύX
			// E3249�o�^�i���ϊz�p�����[�^����FASCII�j "E3249   08/03/17 14:38   ����/����"
			//											"        ED-3456   01000 fz:003-1  "
			//											"        ED-3456(A)01000 fz:003-1  "
			//												 ��ʁA�J�[�h�ԍ���4���A�����ʁA����z�A���̎���OPmod�A�o�^�v��	
	}
// MH321800(E) �t�F�[�Y�G���[�������Ɍ���OK�ȊO�̓��[�_�[�؂藣�����Ȃ�
#if (4 != AUTO_PAYMENT_PROGRAM)										// �����p�ɃJ�[�h�c�z�����܂����ꍇ�iýėp�j
// ����{�I��(0 == AUTO_PAYMENT_PROGRAM)�Ȃ̂ŁA�L��
//   ���σ��[�_�ł�SUICA_LOG_REC,SUICA_LOG_REC_FOR_ERR�̗̈���g�p����
	memcpy(&SUICA_LOG_REC_FOR_ERR, &SUICA_LOG_REC, sizeof(struct suica_log_rec));	// �ُ�۸ނƂ��Č��݂̒ʐM۸ނ�o�^����B
#endif

	// �d���o�^��h�����߂ɂ��łɑ��̗v���ŃG���[�o�^����Ă����ꍇ�́Akind=4�̎���
	// �G���[�o�^�v���׸ނ��N���A����
	Suica_Rec.Data.BIT.LOG_DATA_SET = 0;

// MH321800(S) �t�F�[�Y�G���[�������Ɍ���OK�ȊO�̓��[�_�[�؂藣�����Ȃ�
//	Suica_Rec.suica_err_event.BIT.SETTLEMENT_ERR = 1;				// EC���ψُ픭���t���O
// MH810105(S) MH364301 E3249�𔭐��^�����ʂɕύX
//	// ���ό���OK����E3259��o�^�����[�_��؂藣��
//	if( dat[0] == EPAY_RESULT_OK ){
	{																// E3249,E3259�������ɂ�JVMA���Z�b�g������
// MH810105(E) MH364301 E3249�𔭐��^�����ʂɕύX
		Suica_Rec.suica_err_event.BIT.SETTLEMENT_ERR = 1;			// EC���ψُ픭���t���O
// MH810103 GG119202(S) E3259�AE3262�AE3263������JVMA���Z�b�g���Ȃ�
		// ���ψُ픭������JVMA���Z�b�g����������܂�
		// ���σ��[�_�Ƃ̒ʐM�͍s�킸�A�R�C�����b�N���̒ʐM��D�悳����
		// ec_init()����ECCTL.phase���X�V����
		Suica_Rec.Data.BIT.INITIALIZE = 0;
		ECCTL.phase = EC_PHASE_WAIT_RESET;
		queset( OPETCBNO, EC_EVT_NOT_AVAILABLE_ERR, 0, 0 );			// ���σ��[�_�؂藣���G���[������ope�֒ʒm
// MH810103 GG119202(E) E3259�AE3262�AE3263������JVMA���Z�b�g���Ȃ�
	}
// MH321800(E) �t�F�[�Y�G���[�������Ɍ���OK�ȊO�̓��[�_�[�؂藣�����Ȃ�

// MH810105(S) MH364301 E3249�𔭐��^�����ʂɕύX
	// E3249,E3259��������JVMA���Z�b�g���s�����߈ȉ��̏���
	// �����ް��i����I���j�𑗐M����K�v���Ȃ��B
//// MH810104(S) MH321800(S) ACK5��M�Ńt�F�[�Y���ꂪ��������
//	// ���ό��ʃf�[�^��M���Ƀt�F�[�Y���ꂪ���������ꍇ�͎���I���𑗐M����
//	switch (kind) {
//	case 4:
//	case 5:
//		// ����I�����M�ς݁A�܂��́A���d���̂��ߑ��M���Ȃ�
//		break;
//	default:
//		Ec_Pri_Data_Snd(S_CNTL_DATA, 0x80);							// �����ް��i����I���j�𑗐M����
//		break;
//	}
//// MH810104(E) MH321800(E) ACK5��M�Ńt�F�[�Y���ꂪ��������
// MH810105(E) MH364301 E3249�𔭐��^�����ʂɕύX
}

//[]----------------------------------------------------------------------[]
///	@brief			���σ��[�_���g�p���ꂽ���ǂ����̔�����s��
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	�F�g�p�J�[�h���
///	@return			ret		�F1�F�g�p�ς� 0�F���g�p
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar	EcUseKindCheck(uchar kind)
{
	uchar ret = 0;

	if ((kind == EC_EDY_USED)	 ||	// Edy�g�p��
		(kind == EC_NANACO_USED) ||	// nanaco�g�p��
		(kind == EC_WAON_USED)	 ||	// WAON�g�p��
		(kind == EC_KOUTSUU_USED)||	// ��ʌnIC�J�[�h�g�p��
		(kind == EC_SAPICA_USED) ||	// SAPICA�g�p��
		(kind == EC_ID_USED)	 ||	// iD�g�p��
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//		(kind == EC_QUIC_PAY_USED)) {	// QUICPay�g�p��
		(kind == EC_QUIC_PAY_USED)	||	// QUICPay�g�p��
// MH810105(S) MH364301 PiTaPa�Ή�
		(kind == EC_PITAPA_USED) ||		// PiTaPa�g�p��
// MH810105(E) MH364301 PiTaPa�Ή�
		(kind == EC_QR_USED)) {			// QR�g�p��
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		// �N���W�b�g�J�[�h�́A�����ł͔��肵�Ȃ�
		ret = 1;
	}
	return ret;	
}

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//[]----------------------------------------------------------------------[]
///	@brief			�d�q�}�l�[���σ`�F�b�N�i���σ��[�_�j
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: �g�p�J�[�h���
///	@return			ret		: 1�F�g�p�ς݁A0�F���g�p
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/08/03
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
uchar EcUseEMoneyKindCheck(uchar kind)
{
	uchar ret = 0;

	if ((kind == EC_EDY_USED)		||	// Edy�g�p��
		(kind == EC_NANACO_USED)	||	// nanaco�g�p��
		(kind == EC_WAON_USED)		||	// WAON�g�p��
		(kind == EC_KOUTSUU_USED)	||	// ��ʌnIC�J�[�h�g�p��
		(kind == EC_SAPICA_USED)	||	// SAPICA�g�p��
		(kind == EC_ID_USED)		||	// iD�g�p��
// MH810105(S) MH364301 PiTaPa�Ή�
		(kind == EC_PITAPA_USED) 	||	// PiTaPa�g�p��
// MH810105(E) MH364301 PiTaPa�Ή�
		(kind == EC_QUIC_PAY_USED)) {	// QUICPay�g�p��
		// �N���W�b�g�J�[�h�AQR�R�[�h�͂����ł͔��肵�Ȃ�
		ret = 1;
	}
	return ret;	
}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

//[]----------------------------------------------------------------------[]
///	@brief			�A���[�����V�[�g�󎚗v��
//[]----------------------------------------------------------------------[]
///	@param			prikind		:	�󎚐�
///	@param			*data		:	��M����f�[�^
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/02/07<br>
///					Update
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	ReqEcAlmRctPrint(uchar prikind, EC_SETTLEMENT_RES *data)
{
// �X�e�[�^�X��M�Ǝ���f�[�^��M�������邽��
	T_FrmEcAlmRctReq	FrmAlmRctReq;

	if (prikind == 0) {
	// �󎚐�ݒ�Ȃ�
		return;
	}

	memset(&FrmAlmRctReq, 0, sizeof(FrmAlmRctReq));
	memset(&alm_rct_info, 0, sizeof(alm_rct_info));
	memcpy(&alm_rct_info, data, sizeof(alm_rct_info));						// �A���[�����V�[�g�f�[�^���
	FrmAlmRctReq.prn_kind = prikind;										// �󎚐�
	memcpy(&FrmAlmRctReq.TOutTime, &CLK_REC, sizeof(date_time_rec));		// ���ݎ������
	FrmAlmRctReq.alm_rct_info = &alm_rct_info;
	queset(PRNTCBNO, PREQ_EC_ALM_R, sizeof(FrmAlmRctReq), &FrmAlmRctReq);
}

//[]----------------------------------------------------------------------[]
///	@brief			���σ��[�_ ��������������O�N���A
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			G.So
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/07<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void EcAlarmLog_Clear( void )
{
	// �f�[�^�N���A
	nmicler(&EC_ALARM_LOG_DAT, sizeof(EC_ALARM_LOG_DAT));
	EC_ALARM_LOG_DAT.array_cnt = EC_ALARM_LOG_CNT;
	EC_ALARM_LOG_DAT.data_size = sizeof(t_ALARM_SETTLEMENT_DATA);
}

//[]----------------------------------------------------------------------[]
///	@brief			���σ��[�_ ��������������O�o�^
//[]----------------------------------------------------------------------[]
///	@param[in]		*data 	: ���σf�[�^
///	@return			void
///	@author			G.So
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/07<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void EcAlarmLog_Regist( EC_SETTLEMENT_RES *data )
{
	t_ALARM_SETTLEMENT_DATA	*alarm_settlement;
	short					ctpt[2];
	ushort					num;
// MH810103 GG119202(S) ������������L�^�̃J�[�h�ԍ��̓}�X�N���Ĉ󎚂���
	ushort					i, max;
// MH810103 GG119202(E) ������������L�^�̃J�[�h�ԍ��̓}�X�N���Ĉ󎚂���


	// �f�[�^�N���A
	alarm_settlement = &EC_ALARM_LOG_DAT.log[EC_ALARM_LOG_DAT.log_wpt];
	memset(alarm_settlement, 0, (size_t)EC_ALARM_LOG_DAT.data_size);
// MH810103 GG119202(S) ������������L�^�ɍĐ��Z�����󎚂���
	memset(&EC_ALARM_LOG_DAT.paylog[EC_ALARM_LOG_DAT.log_wpt], 0, sizeof(Receipt_data));
// MH810103 GG119202(E) ������������L�^�ɍĐ��Z�����󎚂���

	// �f�[�^�Z�b�g
	memcpy(&alarm_settlement->alarm_time, &CLK_REC, sizeof(date_time_rec));				// ��������
	alarm_settlement->alarm_time.Sec = (ushort)CLK_REC.seco;

	memcpy(&alarm_settlement->alarm_data, data, sizeof(EC_SETTLEMENT_RES));			// ��������f�[�^
	// PayData��PayData_set�܂ŃZ�b�g����Ȃ����߁APayData_set�ŃZ�b�g�����̂Ɠ��l�ɃZ�b�g����
// MH810103 GG119202(S) ������������L�^�̃J�[�h�ԍ��̓}�X�N���Ĉ󎚂���
	// �����^�C���A�E�g�������̌��ό��ʃf�[�^�ɂ̓}�X�N����Ȃ��J�[�h�ԍ����ʒm����邽�߁A
	// �����ŃJ�[�h�ԍ����}�X�N����i�W���[�i���Ɉ󎚂���J�[�h�ԍ��̓}�X�N���Ȃ��j
	switch (alarm_settlement->alarm_data.brand_no) {
	case BRANDNO_KOUTSUU:
		max = ECARDID_SIZE_KOUTSUU - 4;
		for (i = 2; i < max; i++) {
			alarm_settlement->alarm_data.Card_ID[i] = '*';								// ���2���A����4���ȊO���}�X�N
		}
		break;
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
	case BRANDNO_EDY:
// MH810104(S) nanaco�EiD�EQUICPay�Ή�2
//// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//	case BRANDNO_NANACO:
//// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
// MH810104(E) nanaco�EiD�EQUICPay�Ή�2
		memset(&alarm_settlement->alarm_data.Card_ID[0], '*', 12);						// ����4���ȊO���}�X�N
		break;
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
// MH810104(S) nanaco�EiD�EQUICPay�Ή�2
//// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//	case BRANDNO_ID:
//	case BRANDNO_QUIC_PAY:
//		// iD, QUICPay��ec_miryo_timeout_after_disp()����
//		// �����������V�[�g�p�̃}�X�N�������s�����߁A�����ł͕s�v
//		break;
//// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
	case BRANDNO_ID:
		memset(&alarm_settlement->alarm_data.Card_ID[13], '*', 3);						// ����3�����}�X�N
		break;
	case BRANDNO_QUIC_PAY:
		memset(&alarm_settlement->alarm_data.Card_ID[16], '*', 4);						// ����4�����}�X�N
		break;
// MH810104(E) nanaco�EiD�EQUICPay�Ή�2
	default:
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
	case BRANDNO_WAON:
		// WAON�̓}�X�N����Ēʒm�����
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
// MH810104 GG119202(S) nanaco�EiD�EQUICPay�Ή�
	case BRANDNO_NANACO:
		// nanaco�̓}�X�N����Ēʒm�����
// MH810104 GG119202(E) nanaco�EiD�EQUICPay�Ή�
		break;
	}
// MH810103 GG119202(E) ������������L�^�̃J�[�h�ԍ��̓}�X�N���Ĉ󎚂���
// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
	// �����������R
	alarm_settlement->reason = 0;
	switch (data->Result) {
	case EPAY_RESULT_MIRYO_ZANDAKA_END:
		alarm_settlement->reason = ALARM_REASON_HIKISARI_OK;
		break;
	case EPAY_RESULT_MIRYO:
// MH810105 GG119202(S) ������������W�v�d�l���P
//// MH810103 GG118809_GG118909(S) ����������m�F���̖������R�ɑΉ�����
//		if (ECCTL.ec_Disp_No == 96) {
//			// ����������m�F
//			alarm_settlement->reason = ALARM_REASON_HIKISARI_NG;
//			break;
//		}
//		// �����c���Ɖ�^�C���A�E�g�A�܂��́A�L�����Z��
//// MH810103 GG118809_GG118909(E) ����������m�F���̖������R�ɑΉ�����
// MH810105 GG119202(E) ������������W�v�d�l���P
		alarm_settlement->reason = ALARM_REASON_TIMEOUT;
		break;
	default:
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//		// �݂Ȃ����ώ��͗��R���^�C���A�E�g�Ƃ���
//		alarm_settlement->reason = ALARM_REASON_TIMEOUT;
		// ���Ϗ������ɏ�Q�����������ꍇ�͎x���m�F���s�Ƃ���
		alarm_settlement->reason = ALARM_REASON_CHECKFAIL;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		break;
	}
// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
	// �Ԏ��ԍ�
	alarm_settlement->syu = (char)(ryo_buf.syubet + 1);									// ���

	if( PayData.teiki.update_mon ){														// �X�V���Z?
		alarm_settlement->WPlace = 9999;												// �X�V���͎Ԏ��ԍ�9999�Ƃ���B
	}else if( ryo_buf.pkiti == 0xffff ){												// ������Z?
		alarm_settlement->WPlace = LOCKMULTI.LockNo;									// �ڋq�p���Ԉʒu�ԍ�
// MH810103 GG119202(S) ���Z���~�i���j�󎚂ŎԎ��ԍ����󎚂���Ȃ�
//	}else if( ryo_buf.pkiti == 0 || ryo_buf.pkiti > LOCK_MAX ){							// ���Ԉʒu�ԍ����͈͊O
//		alarm_settlement->WPlace = 0;													// �Ԏ��ԍ�0�Ƃ���
	}else if( ryo_buf.pkiti == 0 ){														// ������Z�H
		// ���u���Z�i������Z�j�͒��Ԉʒu�ԍ����O�̂��߁A��M�����Ԏ������Z�b�g����
		alarm_settlement->WPlace = OPECTL.Op_LokNo;
// MH810103 GG119202(E) ���Z���~�i���j�󎚂ŎԎ��ԍ����󎚂���Ȃ�
	}else{
		num = ryo_buf.pkiti - 1;
		alarm_settlement->WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );
																						// �ڋq�p���Ԉʒu�ԍ�
	}
// MH810103(s) �d�q�}�l�[�Ή� ��������L�^�Ή�
	alarm_settlement->CarSearchFlg = PayData.CarSearchFlg;		// �Ԕ�/�����������
	memcpy(alarm_settlement->CarSearchData, PayData.CarSearchData, sizeof(alarm_settlement->CarSearchData));// �Ԕ�/���������f�[�^
	memcpy(alarm_settlement->CarNumber, PayData.CarNumber, sizeof(alarm_settlement->CarNumber));			// �Ԕ�(���Z)
	memcpy(alarm_settlement->CarDataID, PayData.CarDataID, sizeof(alarm_settlement->CarDataID));			// �Ԕԃf�[�^ID
// MH810103(e) �d�q�}�l�[�Ή� ��������L�^�Ή�

	// ���Ɏ���
	alarm_settlement->TInTime.Year = car_in_f.year;										// ���� �N
	alarm_settlement->TInTime.Mon  = car_in_f.mon;										//      ��
	alarm_settlement->TInTime.Day  = car_in_f.day;										//      ��
	alarm_settlement->TInTime.Hour = car_in_f.hour;										//      ��
	alarm_settlement->TInTime.Min  = car_in_f.min;										//      ��

	// ���O�Ǘ����X�V
	ctpt[0] = EC_ALARM_LOG_DAT.log_cnt;
	if (ctpt[0] < EC_ALARM_LOG_CNT) {	
		ctpt[0]++;
	}
	ctpt[1] = EC_ALARM_LOG_DAT.log_wpt + 1;
	if (ctpt[1] >= EC_ALARM_LOG_CNT) {
		ctpt[1] = 0;
	}
	nmisave(&EC_ALARM_LOG_DAT.log_cnt, ctpt, 4);
}

// MH810103 GG119202(S) ������������L�^�ɍĐ��Z�����󎚂���
//[]----------------------------------------------------------------------[]
///	@brief			������������̍Đ��Z���o�^
//[]----------------------------------------------------------------------[]
///	@param[in]		*data 	: �Đ��Z���
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/08/03<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void EcAlarmLog_RepayLogRegist(Receipt_data *data)
{
	t_ALARM_SETTLEMENT_DATA	*alarm;
	ushort	i;
	ushort	log_cnt;
	int		rpt;

	log_cnt = EcAlarmLog_GetCount();
	rpt = EC_ALARM_LOG_DAT.log_wpt;
	// ���C�g�|�C���^-1���猟�����J�n����
	for (i = 0; i < log_cnt; i++) {
		rpt--;
		if (rpt < 0) {
			rpt = EC_ALARM_LOG_CNT - 1;
		}
		alarm = &EC_ALARM_LOG_DAT.log[rpt];
// MH810105 GG119202(S) ������������W�v�d�l���P
//// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//		if (alarm->reason == ALARM_REASON_HIKISARI_OK) {
//			// �����m����������ς݂ł���΁A�Đ��Z����o�^���Ȃ�
//			continue;
//		}
//// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
// MH810105 GG119202(E) ������������W�v�d�l���P

// MH810103(s) �d�q�}�l�[�Ή� #5525 ������������ɑΉ�����Đ��Z��񂪈󎚂���Ȃ�
//		// �Y�����鏈�������������������(�Ԏ��ԍ��A������ʁA���Ɏ���)
//		if (data->WPlace == alarm->WPlace &&
//			data->syu == alarm->syu &&
		// �Y�����鏈�������������������(�Ԕԃf�[�^ID�A���Ɏ���)
		if (!memcmp(&data->CarDataID, &alarm->CarDataID, sizeof(alarm->CarDataID)) &&
// MH810103(e) �d�q�}�l�[�Ή� #5525 ������������ɑΉ�����Đ��Z��񂪈󎚂���Ȃ�
			!memcmp(&data->TInTime, &alarm->TInTime, sizeof(date_time_rec))) {
			// �Đ��Z�����Z�b�g����
			memcpy(&EC_ALARM_LOG_DAT.paylog[rpt], data, sizeof(*data));
			break;
		}
	}
}
// MH810103 GG119202(E) ������������L�^�ɍĐ��Z�����󎚂���

//[]----------------------------------------------------------------------[]
///	@brief			���σ��[�_ ��������������O�����擾
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			��������������O����
///	@author			G.So
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/07<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort EcAlarmLog_GetCount( void )
{
	return EC_ALARM_LOG_DAT.log_cnt;
}

//[]----------------------------------------------------------------------[]
///	@brief			���σ��[�_ ��������������O�擾
//[]----------------------------------------------------------------------[]
///	@param[in]		idx 	: �擾���郍�O�̈ʒu(0=�ŐV�`4=5����)
///	@param[out]		*log 	: ���σf�[�^
///	@return			short	: 1:�f�[�^�Z�b�gOK�A0:����0�A-1:�Y���f�[�^�Ȃ�
///	@author			G.So
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/07<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810103 GG119202(S) ������������L�^�ɍĐ��Z�����󎚂���
//short EcAlarmLog_LogRead( short idx, t_ALARM_SETTLEMENT_DATA *log )
short EcAlarmLog_LogRead( short idx, t_ALARM_SETTLEMENT_DATA *log, Receipt_data *paylog )
// MH810103 GG119202(E) ������������L�^�ɍĐ��Z�����󎚂���
{
	int	pos;

	if (EC_ALARM_LOG_DAT.log_cnt == 0) {
	// 0��
		return 0;
	}
	
	if ((idx < 0) || (idx >= EC_ALARM_LOG_DAT.log_cnt)) {
	// idx�͈͊O
		return -1;
	}
	
	pos = (EC_ALARM_LOG_CNT + EC_ALARM_LOG_DAT.log_wpt - (idx + 1)) % EC_ALARM_LOG_CNT;

	// ���O�R�s�[
	memcpy(log, &EC_ALARM_LOG_DAT.log[pos], (size_t)EC_ALARM_LOG_DAT.data_size);
// MH810103 GG119202(S) ������������L�^�ɍĐ��Z�����󎚂���
	memcpy(paylog, &EC_ALARM_LOG_DAT.paylog[pos], sizeof(Receipt_data));
// MH810103 GG119202(E) ������������L�^�ɍĐ��Z�����󎚂���

	return 1;
}
//[]----------------------------------------------------------------------[]
///	@brief			�����^�C�}�A�E�g����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
///	@author			G.So
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/18<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void ec_miryo_timeout( void )
{
	LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );		/* ������ײ���ؾ�� */

	Suica_Rec.Data.BIT.MIRYO_NO_ANSWER = 1;				// �������Ȃ��A�����^�C���A�E�g�����ꍇ

// MH810103 GG119202(S) �����d�l�ύX�Ή�
//	if (dsp_change) {
//		op_SuicaFusokuOff();							// ���Z�������̔��]ү���ޕ\������������
//		dsp_change = 0;
//		nyukin_delay_check(nyukin_delay, delay_count);	// �ۗ����̲���Ă�����΍�post����
//		wmonlg(OPMON_EC_MIRYO_TIMEOUT, NULL, 0);		// ���j�^�o�^
//	}
// MH810103 GG119202(E) �����d�l�ύX�Ή�

// MH810103 GG119001_GG119101(S) �����c���Ɖ�ɒʐM�s�ǔ����ł݂Ȃ����ςɂȂ�
//// GG119202(S) �J�[�h��������M��̌���NG�i304�j�̈���
//	// ���Ϗ�񂪃Z�b�g����Ă��Ȃ��̂ŁA�����ŃZ�b�g����
//	// �u�����h�ԍ�
//	Ec_Settlement_Res.brand_no = EcDeemedLog.EcDeemedBrandNo;
//	// �x���z
//	Ec_Settlement_Res.settlement_data = Product_Select_Data;
//	// �₢���킹�ԍ�
//	memset(Ec_Settlement_Res.inquiry_num, '*', sizeof(Ec_Settlement_Res.inquiry_num));
//	Ec_Settlement_Res.inquiry_num[4] = '-';
//	switch (Ec_Settlement_Res.brand_no) {
//	case BRANDNO_KOUTSUU:
//		// �J�[�h�ԍ�
//		memset(Ec_Settlement_Res.Card_ID, '*', ECARDID_SIZE_KOUTSUU);
//		// SPRWID
//		memset(Ec_Settlement_Res.Brand.Koutsuu.SPRW_ID, '*', sizeof(Ec_Settlement_Res.Brand.Koutsuu.SPRW_ID));
//		break;
//// MHUT40XX(S) Edy�EWAON�Ή�
//	case BRANDNO_EDY:
//		// �J�[�h�ԍ�
//		memset(Ec_Settlement_Res.Card_ID, '*', ECARDID_SIZE_SUICA);
//		// Edy����ʔ�
//		memset(Ec_Settlement_Res.Brand.Edy.DealNo, '*', sizeof(Ec_Settlement_Res.Brand.Edy.DealNo));
//		// �J�[�h����ʔ�
//		memset(Ec_Settlement_Res.Brand.Edy.CardDealNo, '*', sizeof(Ec_Settlement_Res.Brand.Edy.CardDealNo));
//		// ��ʒ[��ID
//		memset(Ec_Settlement_Res.Brand.Edy.TerminalNo, '*', sizeof(Ec_Settlement_Res.Brand.Edy.TerminalNo));
//		break;
//	case BRANDNO_WAON:
//		// �J�[�h�ԍ�
//		memset(Ec_Settlement_Res.Card_ID, '*', ECARDID_SIZE_SUICA);
//		// SPRWID
//		memset(Ec_Settlement_Res.Brand.Waon.SPRW_ID, '*', sizeof(Ec_Settlement_Res.Brand.Waon.SPRW_ID));
//		break;
//// MHUT40XX(E) Edy�EWAON�Ή�
//// MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//	case BRANDNO_NANACO:
//		// �J�[�h�ԍ�
//		memset(Ec_Settlement_Res.Card_ID, '*', ECARDID_SIZE_SUICA);
//		// �[������ʔ�
//		memset(Ec_Settlement_Res.Brand.Nanaco.DealNo, '*', sizeof(Ec_Settlement_Res.Brand.Nanaco.DealNo));
//		// ��ʒ[��ID
//		memset(Ec_Settlement_Res.Brand.Nanaco.TerminalNo, '*', sizeof(Ec_Settlement_Res.Brand.Nanaco.TerminalNo));
//		break;
//	case BRANDNO_ID:
//		// �J�[�h�ԍ�
//		memset(Ec_Settlement_Res.Card_ID, '*', ECARDID_SIZE_SUICA);
//		// ��ʒ[��ID
//		memset(Ec_Settlement_Res.Brand.Id.TerminalNo, '*', sizeof(Ec_Settlement_Res.Brand.Id.TerminalNo));
//		break;
//	case BRANDNO_QUIC_PAY:
//		// �J�[�h�ԍ�
//		memset(Ec_Settlement_Res.Card_ID, '*', sizeof(Ec_Settlement_Res.Card_ID));
//		// ��ʒ[��ID
//		memset(Ec_Settlement_Res.Brand.QuicPay.TerminalNo, '*', sizeof(Ec_Settlement_Res.Brand.QuicPay.TerminalNo));
//		break;
//// MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
//	default:
//		break;
//	}
//// GG119202(E) �J�[�h��������M��̌���NG�i304�j�̈���
//// GG119202(S) �������Ɉُ�f�[�^��M�łƂ肯���{�^�������҂��ɂȂ�Ȃ�
//	if ((Suica_Rec.suica_err_event.BYTE & EC_NOT_AVAILABLE_ERR) != 0) {
//		// �ُ�f�[�^��M���͐���f�[�^�i��t�֎~�j�����M�s�̂��߁A
//		// ��ԃf�[�^�������������s���A�Ƃ肯���{�^�������҂��ɑJ�ڂ�����
//		snd_no_response_timeout();
//		return;
//	}
//// GG119202(E) �������Ɉُ�f�[�^��M�łƂ肯���{�^�������҂��ɂȂ�Ȃ�
//	Ec_Pri_Data_Snd(S_CNTL_DATA, 0);					// �����ް��i��t�֎~�j�𑗐M����
// MH810105 GG119202(S) �����c���Ɖ�ɏ�Q�����������ꍇ�͐ݒ�50-0014�ɏ]��
//	if (Suica_Rec.Data.BIT.MIRYO_CONFIRM == 0) {
//		// �����m��O�͖⍇��(������)�҂��^�C���A�E�g�Ƃ���
//		queset(OPETCBNO, EC_INQUIRY_WAIT_TIMEOUT, 0, 0);
//	}
//	else {
//		// �����m���͏�ԃf�[�^�������������s���A�Ƃ肯���{�^�������҂��ɑJ�ڂ�����
//		EcSetDeemedSettlementResult(&Ec_Settlement_Res);
//		snd_no_response_timeout();
//		ac_flg.ec_deemed_fg = 0;						// �݂Ȃ����ϕ��d�p�t���OOFF
//	}
	// �����m�����⍇��(������)�҂��^�C���A�E�g�Ƃ���
	queset(OPETCBNO, EC_INQUIRY_WAIT_TIMEOUT, 0, 0);
// MH810105 GG119202(E) �����c���Ɖ�ɏ�Q�����������ꍇ�͐ݒ�50-0014�ɏ]��
// MH810103 GG119001_GG119101(E) �����c���Ɖ�ɒʐM�s�ǔ����ł݂Ȃ����ςɂȂ�
}

//[]----------------------------------------------------------------------[]
///	@brief			�������ω\�����f����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
// MH810103 GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
/////	@return			1:�������ω�, 0:�������ϕs��
///	@return			=  0:���ω\�u�����h�Ȃ�
///					=  1:�P�ꌈ�ϗL��
///					>= 2:�������ϗL��
///	@note			�����󋵁A���Z���x�z�����`�F�b�N������Ō��ω\��
///					�u�����h����ԋp����
// MH810103 GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
///	@author			G.So
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar get_enable_multisettle(void)
{
// MH810103 GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
//	uchar ret, eMenoy, credit, houjin;
	uchar ret;
// MH810103 GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
	int i, j;

// MH810103 GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
//	ret = eMenoy = credit = houjin = 0;
	ret = 0;
// MH810103 GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
	for (i = 0; i < TBL_CNT(RecvBrandTbl); i++) {
		for (j = 0; j < RecvBrandTbl[i].num; j++) {
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//			if ((RecvBrandTbl[i].ctrl[j].status & 0x01) != 0x01) {
//				continue;
//			}
//			switch(RecvBrandTbl[i].ctrl[j].no) {
//				case BRANDNO_KOUTSUU:
//				case BRANDNO_EDY:
//				case BRANDNO_NANACO:
//				case BRANDNO_WAON:
//				case BRANDNO_SAPICA:
//				case BRANDNO_ID:
//				case BRANDNO_QUIC_PAY:
//// GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
////					eMenoy = 1;
//					if( WAR_MONEY_CHECK && !e_incnt ){	// �����Ȃ�
//						ret += 1;
//					}
//// GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
//					break;
//				case BRANDNO_CREDIT:
//// GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
////					credit = 1;
//					if( Ec_check_cre_pay_limit() ){	// �N���W�b�g���Z��(���x�z�ȉ�)
//						ret += 1;
//					}
//// GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
//					break;
//				case BRANDNO_HOUJIN:
//// GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
////					houjin = 1;
//					ret += 1;
//// GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
//					break;
//				default:
//					break;
//			}
			if (IsValidBrand(i, j)) {
				ret += 1;
			}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810103 GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
//			if ((eMenoy + credit + houjin) > 1){
//				ret = 1;
//				break;
//			}
// MH810103 GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
		}

// MH810103 GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
//		if (ret == 1) {
//			break;
//		}
// MH810103 GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
	}

	return ret;
}

// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//[]----------------------------------------------------------------------[]
///	@brief			�������ω\�����f����(���Z�󋵂Ɉˑ����Ȃ�)
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			=  0:���ω\�u�����h�Ȃ�
///					=  1:�P�ꌈ�ϗL��
///					>= 2:�������ϗL��
///	@note			���ω\�ȃu�����h����ԋp����
///	@author			A.Shindoh
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/06/24<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
uchar check_enable_multisettle(void)
{
	uchar ret;
	uchar ret2;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	uchar QRCheck = FALSE;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	int i, j;

	ret = 0;
	ret2 = 0;
	for (i = 0; i < TBL_CNT(RecvBrandTbl); i++) {
		for (j = 0; j < RecvBrandTbl[i].num; j++) {
// MH810103 GG119202(S) �u�����h��Ԕ��菈���ύX
//			if ((RecvBrandTbl[i].ctrl[j].status & 0x01) == 0x01) {		// �J�ǂ��Ă���
			if (isEC_BRAND_STS_KAIKYOKU(RecvBrandTbl[i].ctrl[j].status)) {	// �J�ǂ��Ă���
// MH810103 GG119202(E) �u�����h��Ԕ��菈���ύX
				switch(RecvBrandTbl[i].ctrl[j].no) {
				case BRANDNO_TCARD:		// T-�J�[�h 					
//					if (Tcard_Use_Chk() != 0) {							// T�J�[�h�g�p��
//						ret += 1;
//					}
					break;
				case BRANDNO_HOUJIN:	// �@�l�J�[�h
//					if (prm_get(COM_PRM, S_RPK, 1, 1, 1) == 1 &&		// Rism�ʐM�ݒ肠��
//						prm_get(COM_PRM, S_RPK, 105, 1, 1) == 1){		// �@�l�J�[�h�ݒ肠��  
//						ret += 1;
//					}
					break;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
				case BRANDNO_QR:		// QR�R�[�h����
					QRCheck = TRUE;
					// no break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
				default:
					if (IsSettlementBrand(RecvBrandTbl[i].ctrl[j].no)) {
						ret2++;
					}
					ret += 1;
					break;
				}
			}
		}
	}
	if (ret == 1 && ret2 == 0) {
	// �B��̃u�����h���񌈍ςȂ�}���`�u�����h��I��
		ret = 2;
	}
// MH810103 GG118808_GG118908(S) �d�q�}�l�[�V���O���ݒ�̓ǂݎ��҂��^�C���A�E�g�����ύX
	if (ret == 1 && ECCTL.ec_single_timeout == 1) {
		// �V���O���u�����h�œǂݎ��҂��^�C���A�E�g�A�L�����Z���{�^���������ꂽ�ꍇ��
		// �ȍ~�̓}���`�u�����h�Ƃ��Ĉ���
		ret += 1;
	}
// MH810103 GG118808_GG118908(E) �d�q�}�l�[�V���O���ݒ�̓ǂݎ��҂��^�C���A�E�g�����ύX
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	if (ret == 1 && QRCheck == TRUE) {
		// QR�R�[�h���ς̃V���O���u�����h�̓}���`�u�����h�Ƃ��Ĉ���
		ret = 2;
	}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	if (ret > 1) {
	// �u�����h�I��������~��͂O�ɂ���
		if (ECCTL.brandsel_stop != 0) {
			ret = 0;
		}
	}

	return ret;
}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j

// MH810103 GG119202(S) �J�ǁE�L�������ύX
////[]----------------------------------------------------------------------[]
/////	@brief			���σ��[�_ ���Z�۔���
////[]----------------------------------------------------------------------[]
/////	@param[in]		kind:	������
/////	@return			1:���Z��, 0:���Z�s��
/////	@author			hosoda
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2019/02/05<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//uchar isEcReady( const uchar kind )
//{
//	uchar ret = 0;
//
//	// ���ʃ`�F�b�N
//	if ( isEC_USE()											// ���σ��[�_�g�p�ݒ�
//		&& !Suica_Rec.suica_err_event.BYTE					// ���[�_�[�g�p��
//// GG119202(S) ���Z�\���菈���C��
//		&& Suica_Rec.Data.BIT.INITIALIZE					// ����������
//// GG119202(E) ���Z�\���菈���C��
//// GG119202(S) �ڋq��ʂ̓d�q�}�l�[�Ή�
//		&& Suica_Rec.Data.BIT.CTRL							// ��t���
//// GG119202(E) �ڋq��ʂ̓d�q�}�l�[�Ή�
//		// TODO: ��������ݒ� (�d�l������)
//		// �W���[�i���v�����^���g�p�\
//		// ���V�[�g�v�����^���g�p�\(�g�p�s���Ɍ��ϕs�ݒ莞)
//	) {
//		// �����ʂ��Ƃ̃`�F�b�N
//		switch (kind)
//		{
//		case EC_CHECK_STATUS:								// ���Z�s�ł͂Ȃ���
//			return 1;
//			break;
//
//		case EC_CHECK_EMONEY:								// �d�q�}�l�[�Ő��Z�\��
//// GG119202(S) �s�v�����폜
////			if ( isAnyEcBrandReady(EcBrandEmoney, TBL_CNT(EcBrandEmoney)) ) {
//			if (isEcEmoneyEnabled(1, 0)) {
//// GG119202(E) �s�v�����폜
//				ret = 1;
//			}
//			break;
//
//		case EC_CHECK_CREDIT:								// �N���W�b�g�Ő��Z�\��
//// GG119202(S) �s�v�����폜
////			if ( isAnyEcBrandReady(EcBrandCredit, TBL_CNT(EcBrandCredit)) ) {
//			if (isEcBrandNoReady(BRANDNO_CREDIT)) {
//// GG119202(E) �s�v�����폜
//				ret = 1;
//			}
//			break;
//
//		default:
//			break;
//		}	
//	}
//
//	return ret;
//}
//[]----------------------------------------------------------------------[]
///	@brief			�u�����h���p�\�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no:	�u�����h�ԍ�
///	@return			1:���p��, 0:���p�s��
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/10/7<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
//�@�u�����h�����Z�ɗ��p�ł��邩���`�F�b�N����B
//�@���[�_�[�̃G���[��ԂȂǂ͉����֐��ł͌��Ă��Ȃ����߂����Ŏ��{����B
//�@�P�ɊJ�ǂ��Ă��邩��isEcBrandNoReady()���g�p���邱�ƁB
//�@�d�q�}�l�[�𕡍��I�Ƀ`�F�b�N�������ꍇ�́AisEcEmoneyEnabled()���g�p���邱�ƁB(Ready,Enable���Ή��ł��B)
uchar isEcBrandNoEnabled(ushort brand_no)
{
	uchar	ret = 0;

	// ���ʃ`�F�b�N
	if ( isEC_USE()											// ���σ��[�_�g�p�ݒ�
		&& !Suica_Rec.suica_err_event.BYTE					// ���[�_�[�g�p��
		&& Suica_Rec.Data.BIT.INITIALIZE					// ����������
		&& Suica_Rec.Data.BIT.CTRL							// ��t���
	) {
		if (ec_check_valid_brand(brand_no)) {
			ret = 1;
		}
	}
	return ret;
}
// MH810103 GG119202(E) �J�ǁE�L�������ύX
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6482 VP�Ő��Z���ɃN���W�b�g���d�q�}�l�[��I������ƁA�u�����h��I�������^�C�~���O�Ő��Z��ʂ̐��Z���@����u�R�[�h���ρv��������
uchar isEcBrandNoEnabledNoCtrl(ushort brand_no)
{
	uchar	ret = 0;

	// ���ʃ`�F�b�N
	if ( isEC_USE()											// ���σ��[�_�g�p�ݒ�
		&& !Suica_Rec.suica_err_event.BYTE					// ���[�_�[�g�p��
		&& Suica_Rec.Data.BIT.INITIALIZE					// ����������
//		&& Suica_Rec.Data.BIT.CTRL							// ��t���
	) {
		if (ec_check_valid_brand(brand_no)) {
			ret = 1;
		}
	}
	return ret;
}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6482 VP�Ő��Z���ɃN���W�b�g���d�q�}�l�[��I������ƁA�u�����h��I�������^�C�~���O�Ő��Z��ʂ̐��Z���@����u�R�[�h���ρv��������

// MH810103 GG119202(S) �s�v�����폜
////[]----------------------------------------------------------------------[]
/////	@brief			���σ��[�_ ���Z�۔��� (�w��u�����h�̂��������ꂩ)
////[]----------------------------------------------------------------------[]
/////	@param[in]		brand_ec_ary:	���肷��u�����h�̔z�� (EC_*_USED)
/////	@param[in]		ary_size: brand_ec_ary�̗v�f��
/////	@return			1:�����ꂩ�Ő��Z��, 0:���ׂĐ��Z�s��
/////	@author			hosoda
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2019/02/20<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//uchar isAnyEcBrandReady(const ushort *brand_ec_ary, const ushort ary_size)
//{
//	uchar	ret = 0;
//	int		i;
//	ushort	brand_no;
//
//	for(i = 0; i < ary_size; i++)
//	{
//		brand_no = ConvertEcBrandToNo(brand_ec_ary[i]);		// EC_*_USED����BRANDNO_*�ւ̕ϊ�
//		if ( brand_no == 0 ) {
//		// �s���u�����h
//			continue;
//		}
//
//		if ( isEcBrandNoReady(brand_no) ) {
//		// �u�����h�����Z�\
//			ret = 1;
//			break;			// 1�u�����h�ł����Z�\�Ȃ画��I��
//		}
//	}
//
//	return ret;
//}
// MH810103 GG119202(E) �s�v�����폜

//[]----------------------------------------------------------------------[]
///	@brief			���σ��[�_ ���Z�۔��� (�w��u�����h�ԍ�)
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no:	���肷��u�����h�ԍ�(BRANDNO_*)
///	@return			1:���Z��, 0:���Z�s��
///	@author			hosoda
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar isEcBrandNoReady(const ushort brand_no)
{
	uchar	ret = 0;
	uchar	found = 0;
	int		i, j;

	for(i = 0; i < TBL_CNT(RecvBrandTbl); i++)
	{
		for(j = 0; j < RecvBrandTbl[i].num; j++)
		{
			if ( RecvBrandTbl[i].ctrl[j].no == brand_no ) {
// MH810103 GG119202(S) �u�����h��Ԕ��菈���ύX
//				if ( (RecvBrandTbl[i].ctrl[j].status & 0x01) == 0x01 ) {
//				// ����,�ꌏ���ז��t,�T�[�r�X��~�̂�����ł��Ȃ��J�Ǎς�
				if (isEC_BRAND_STS_KAIKYOKU(RecvBrandTbl[i].ctrl[j].status)) {
					// �J�ǁ��L��
// MH810103 GG119202(E) �u�����h��Ԕ��菈���ύX
					ret = 1;
				}
				found = 1;
				break;
			}
		}
		if ( found ) {
			break;
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			���σ��[�_ �d�q�}�l�[ (�w��u�����h�ԍ�)
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no:	���肷��u�����h�ԍ�(BRANDNO_*)
// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
///	@param[in]		zandaka_chk:�c���Ɖ�\���`�F�b�N
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
///	@return			1:�d�q�}�l�[, 0:�d�q�}�l�[�ȊO
///	@author			hosoda
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/03/18<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//uchar isEcBrandNoEMoney(const ushort brand_no)
uchar isEcBrandNoEMoney(const ushort brand_no, uchar zandaka_chk)
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
{
	uchar	ret = 0;

	switch(brand_no)
	{
		case BRANDNO_KOUTSUU:
		case BRANDNO_EDY:
		case BRANDNO_NANACO:
		case BRANDNO_WAON:
		case BRANDNO_SAPICA:
		case BRANDNO_ID:
		case BRANDNO_QUIC_PAY:
// MH810105(S) MH364301 PiTaPa�Ή�
		case BRANDNO_PITAPA:
// MH810105(E) MH364301 PiTaPa�Ή�
// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
			if (zandaka_chk) {
				// �c���Ɖ�\�`�F�b�N
// MH810105(S) MH364301 PiTaPa�Ή�
//				switch (brand_no) {
//				case BRANDNO_ID:
//				case BRANDNO_QUIC_PAY:
//					// �c���Ɖ�s��
//					return 0;
//				default:
//					// �c���Ɖ�\
//					break;
//				}
				if(EcBrandPostPayCheck(convert_brandno(brand_no))) {
					// �c���Ɖ�s��
					return 0;
				}
// MH810105(E) MH364301 PiTaPa�Ή�
			}
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
			ret = 1;
			break;
		default:
			break;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			EC_*_USED����BRANDNO_*�ւ̕ϊ�
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_ec:	�u�����h(EC_*_USED)
///	@return			�u�����h�ԍ�(BRANDNO_*), 0:�ϊ��s��
///	@author			hosoda
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
ushort ConvertEcBrandToNo(const ushort brand_ec)
{
	ushort	brand_no = 0;
	int		i;

	// �񌈍σu�����h�ɂ͑Ή����Ȃ�
	for(i = 0; i < TBL_CNT(EcBrandNoConvTbl); i++)
	{
		if ( brand_ec == EcBrandNoConvTbl[i].brand_ec ) {
			brand_no = EcBrandNoConvTbl[i].brand_no;
			break;
		}
	}

	return brand_no;
}

//[]----------------------------------------------------------------------[]
///	@brief			���σ��[�_ �g�p�ݒ蔻�� (�w��u�����h�ԍ�)
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no:	���肷��u�����h�ԍ�(BRANDNO_*)
///	@return			1:�g�p����, 0:�g�p���Ȃ�
///	@author			hosoda
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/20<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar isEcBrandNoEnabledForSetting(const ushort brand_no)
{
	uchar	ret = 0;
// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//	int		i;
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
// MH810103 GG119202(S) ���ς���u�����h�I��ݒ���Q�Ƃ���
	ushort	i;
// MH810103 GG119202(E) ���ς���u�����h�I��ݒ���Q�Ƃ���

// MH810103 GG116201(S) ���σ��[�_�ڑ��Ȃ��Ȃ�u�����h�g�p�ݒ���Q�Ƃ��Ȃ�
	if( isEC_USE() == 0 ){
		return ret;
	}
// MH810103 GG116201(E) ���σ��[�_�ڑ��Ȃ��Ȃ�u�����h�g�p�ݒ���Q�Ƃ��Ȃ�

// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//	for(i = 0; i < TBL_CNT(EcBrandEnabledParamTbl); i++) {
//		if ( brand_no == EcBrandEnabledParamTbl[i].brand_no ) {
//			if (prm_get(COM_PRM, S_ECR, EcBrandEnabledParamTbl[i].address, 1, EcBrandEnabledParamTbl[i].pos) != 0) {
//				ret = 1;
//			}
//			break;
//		}
//	}
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
// MH810103 GG119202(S) ���ς���u�����h�I��ݒ���Q�Ƃ���
	if (IsSettlementBrand(brand_no)) {
		// ���ς���u�����h
		ret = 1;
		for (i = 0; i < TBL_CNT(EcBrandEnabledParamTbl); i++) {
			if (brand_no == EcBrandEnabledParamTbl[i].brand_no) {
				if (prm_get(COM_PRM, S_ECR, EcBrandEnabledParamTbl[i].address, 1, EcBrandEnabledParamTbl[i].pos) == 0) {
					// �g�p�Ȃ�
					ret = 0;
				}
				break;
			}
		}
	}
	else {
		// ���ςȂ��u�����h�^�㌈�σu�����h
// MH810103 GG119202(E) ���ς���u�����h�I��ݒ���Q�Ƃ���
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	switch(brand_no) {
	case BRANDNO_TCARD:
	case BRANDNO_HOUJIN:
//		if (prm_get(COM_PRM, S_RPK, 1, 1, 1) == 1) {		// Rism�ʐM�ݒ肠��
//			ret = 1;
//		} else {
//			ret = 0;
//		}
		break;
	default:
		ret = 1;
		break;
	}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810103 GG119202(S) ���ς���u�����h�I��ݒ���Q�Ƃ���
	}
// MH810103 GG119202(E) ���ς���u�����h�I��ݒ���Q�Ƃ���

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�u�����h�L������i��M�u�����h�e�[�u������j
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no:	���肷��u�����h�ԍ�(BRANDNO_*)
///	@return			1:�L��, 0:����
///	@author			T.Nagai
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/09/10<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar isEcBrandNoEnabledForRecvTbl(const ushort brand_no)
{
	uchar	ret = 0;
	int		i, j;

// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
	if (!isEC_USE()) {
		return ret;
	}
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�

	for (i = 0; i < TBL_CNT(RecvBrandTbl); i++) {
		for (j = 0; j < RecvBrandTbl[i].num; j++) {
			// ��M�����u�����h�e�[�u���ƈ�v���邩�H
// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//// GG119202(S) �����d�l�ύX�Ή�
////			if (brand_no == RecvBrandTbl[i].ctrl[j].no) {
//			if (brand_no == RecvBrandTbl[i].ctrl[j].no &&
//				RecvBrandTbl[i].ctrl[j].status == 0x01) {
//			// ��M������ԁF�J�ǂ̏ꍇ�A�L��
//// GG119202(E) �����d�l�ύX�Ή�
			if (brand_no == RecvBrandTbl[i].ctrl[j].no) {
				// �J�Ǐ�Ԃ͌��Ȃ�
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
				ret = 1;
				break;
			}
		}
		if (ret != 0) {
			break;
		}
	}
	return ret;
}

// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
////[]----------------------------------------------------------------------[]
/////	@brief			���σ��[�_ �u�����h�L������
////[]----------------------------------------------------------------------[]
/////	@param[in]		kind:	������
/////	@return			1:�L��, 0:����
/////	@author			T.Nagai
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2019/09/10<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//uchar isEcEnabled( const uchar kind )
//{
//	uchar ret = 0;
//
//	// ���ʃ`�F�b�N
//	if ( isEC_USE() ) {										// ���σ��[�_�g�p�ݒ�
//		// �����ʂ��Ƃ̃`�F�b�N
//		switch (kind) {
//		case EC_CHECK_EMONEY:								// �d�q�}�l�[�Ő��Z�\��
//			if ( isAnyEcBrandEnabled(EcBrandEmoney, TBL_CNT(EcBrandEmoney)) ) {
//				ret = 1;
//			}
//			break;
//
//		case EC_CHECK_CREDIT:								// �N���W�b�g�Ő��Z�\��
//			if ( isAnyEcBrandEnabled(EcBrandCredit, TBL_CNT(EcBrandCredit)) ) {
//				ret = 1;
//			}
//			break;
//
//		default:
//			break;
//		}
//	}
//
//	return ret;
//}
//
////[]----------------------------------------------------------------------[]
/////	@brief			���σ��[�_ �u�����h�L������ (�w��u�����h�̂��������ꂩ)
////[]----------------------------------------------------------------------[]
/////	@param[in]		brand_ec_ary:	���肷��u�����h�̔z�� (EC_*_USED)
/////	@param[in]		ary_size: brand_ec_ary�̗v�f��
/////	@return			1:�����ꂩ�L��, 0:���ׂĖ���
/////	@author			T.Nagai
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2019/09/10<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//uchar isAnyEcBrandEnabled(const ushort *brand_ec_ary, const ushort ary_size)
//{
//	uchar	ret = 0;
//	int		i;
//	ushort	brand_no;
//
//	for(i = 0; i < ary_size; i++) {
//		brand_no = ConvertEcBrandToNo(brand_ec_ary[i]);		// EC_*_USED����BRANDNO_*�ւ̕ϊ�
//		if ( brand_no == 0 ) {
//		// �s���u�����h
//			continue;
//		}
//
//		if ( isEcBrandNoEnabledForRecvTbl(brand_no) ) {
//		// �u�����h����M�����u�����h�e�[�u���ƈ�v���邩
//			ret = 1;
//			break;			// 1�u�����h�ł���v����Ȃ画��I��
//		}
//	}
//
//	return ret;
//}
//[]----------------------------------------------------------------------[]
///	@brief			�d�q�}�l�[���p�\�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		open_chk:		���ω\�`�F�b�N
///	@param[in]		zandaka_chk:	�c���Ɖ�\�`�F�b�N
///	@return			1:�d�q�}�l�[���p��, 0:�d�q�}�l�[���p�s��
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/07/06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
uchar isEcEmoneyEnabled(uchar open_chk, uchar zandaka_chk)
{
	uchar	ret = 0;
	int		i, j;

	if (!isEC_USE()) {
		return ret;
	}

// MH810103 GG119202(S) �J�ǁE�L�������ύX
	if (open_chk || zandaka_chk) {
		if (!Suica_Rec.suica_err_event.BYTE					// ���[�_�[�g�p��
			&& Suica_Rec.Data.BIT.INITIALIZE				// ����������
			&& Suica_Rec.Data.BIT.CTRL) {					// ��t���
			;		// �R���f�B�V����OK
		}
		else {
			return ret;
		}
	}
// MH810103 GG119202(E) �J�ǁE�L�������ύX

	for (i = 0; i < TBL_CNT(RecvBrandTbl); i++) {
		for (j = 0; j < RecvBrandTbl[i].num; j++) {
			// ��M�����u�����h�e�[�u���ɓd�q�}�l�[�u�����h���܂܂�Ă��邩�H
			if (isEcBrandNoEMoney(RecvBrandTbl[i].ctrl[j].no, zandaka_chk)) {
				if (open_chk) {
					// ���ω\�`�F�b�N
// MH810103 GG119202(S) �J�ǁE�L�������ύX
//					if (!SUICA_USE_ERR &&							// �G���[������
//// GG119202(S) �ڋq��ʂ̓d�q�}�l�[�Ή�
//						Suica_Rec.Data.BIT.CTRL &&					// ��t���
//// GG119202(E) �ڋq��ʂ̓d�q�}�l�[�Ή�
//						RecvBrandTbl[i].ctrl[j].status == 0x01) {	// �J�ǁE�L��
					if (IsValidBrand(i, j)) {
// MH810103 GG119202(E) �J�ǁE�L�������ύX
						ret = 1;
						break;
					}
				}
				else {
					ret = 1;
					break;
				}
			}
		}
		if (ret != 0) {
			break;
		}
	}
	return ret;
}
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�

// MH810103 GG119202(S) �u�����h���̓u�����h��񂩂�擾����
//[]----------------------------------------------------------------------[]
//	@brief		�u�����h���擾
//[]----------------------------------------------------------------------[]
//	@param[in]	*brand	: �u�����h���
//	@param[out]	*buff	: �u�����h��
//	@return		�u�����h���̒���
//[]----------------------------------------------------------------------[]
//	@author		MATSUSHITA
//	@date		Create	: 20/10/01<br>
//				Update	: 
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
int	getBrandName(EC_BRAND_CTRL *brand, uchar *buff)
{
	int		i, j;

	j = (int)sizeof(brand->name);

	for (i = 0; i < j && brand->name[i] != '\0'; i++) {}	// ������I�[��
	for (i--; i >= 0 && (brand->name[i] == ' '); i--) {}		// �X�y�[�X�J�b�g
	i++;
	if (i == 0) {
		buff[0] = '(';
		buff[5] = ')';
		intoasl(&buff[1], (ulong)brand->no, 4);
		i = 6;
	}
	else {
		memcpy(buff, brand->name, (size_t)i);
	}
	buff[i] = '\0';
	return i;
}
// MH810103 GG119202(E) �u�����h���̓u�����h��񂩂�擾����

//[]----------------------------------------------------------------------[]
///	@brief			����\����ʔ���֐�
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret �F0:����(�����ς�)��� 1:������
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	: 2019/03/15<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
char	EcFirst_Pay_Dsp( void )
{
	char	ret = 0;

	if((0 == ryo_buf.waribik) &&
	   (0 == c_pay) &&
	   (0 == e_pay) &&
	   ( !carduse() ) &&
	   ( vl_ggs.ryokin == 0 &&								// vl_ggs �F ���z���Z�̈� ��0�̎��ɻ��د��\���ĊJ
	   ( vl_frs.lockno == 0 || vl_frs.antipassoff_req) ) &&	// vl_frs �F �U�֐��Z�̈� ��0�A�������͒�����p�L��̎��ɻ��د��\���ĊJ
	   (ryo_buf.nyukin == 0)){
		ret = 1;
	}

	return ret;	
}

//[]----------------------------------------------------------------------[]
///	@brief			Ope_EcEleUseDsp
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	: 2019/04/01<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void Ope_EcEleUseDsp( void )
{
	e_incnt = 0;															// ���Z���́u�d�q�}�l�[�v�g�p�񐔁i�܂ޒ��~�j���N���A
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//	memset(&RecvBrandResTbl, 0x00, sizeof(RecvBrandResTbl));				// �u�����h�I�����ʃf�[�^������
//	RecvBrandResTbl.no = (ushort)BRANDNO_UNKNOWN;							// �I���u�����h�ԍ��͕s���ŏ�����
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	timer_recept_send_busy = 0;
	ECCTL.timer_recept_send_busy = 0;
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�s�
	EcBrandClear();
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�s�
	EcSendCtrlEnableData();
	Suica_Rec.Data.BIT.PAY_CTRL = 0;										// Suica���Z�׸ނ�ؾ��
	DspChangeTime[0] = (ushort)CPrmSS[S_SCA][6];							// �c���\�����Ծ��

	DspChangeTime[1] = 2; 													// ��ʐ؂�ւ����Ծ��

	Lagtim( OPETCBNO, TIMERNO_EC_CYCLIC_DISP, (ushort)(DspChangeTime[1]*50) );	// ���p�\�}�̻��د��\���p��ϰ�N��
	Suica_Rec.Data.BIT.SEND_CTRL80 = 0;										// ����I���f�[�^���M��ԃt���O�N���A
// MH810103 GG119202(S) �����c���Ɖ�^�C���A�E�g��ʂ������Ȃ�
	Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 0;									// �����^�C���A�E�g�t���O�N���A
// MH810103 GG119202(E) �����c���Ɖ�^�C���A�E�g��ʂ������Ȃ�
}

//[]----------------------------------------------------------------------[]
///	@brief			recv_unexpected_status_data
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	: 2019/05/09<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void recv_unexpected_status_data(void)
{
// MH321800(S) E3269��o�^���Ȃ����Ή�
//	memset(err_wk, 0, sizeof(err_wk));
//
//	// �I�y���[�V�������[�h
//	intoas(&err_wk[0], (ushort)OPECTL.Mnt_mod, 2);
//	err_wk[2] = '-';
//	intoas(&err_wk[3], (ushort)OPECTL.Ope_mod, 3);
//	err_wk[6] = ' ';
//	// MVT��ԃf�[�^
//	hxtoas2(&err_wk[7], EC_STATUS_DATA.status_data);
//
//	err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_IGL_STS_DATA, 2, 1, 1, err_wk);	/* �װ۸ޓo�^�i�o�^�j*/
// MH321800(E) E3269��o�^���Ȃ����Ή�
}

//[]----------------------------------------------------------------------[]
///	@brief			SetEcVolume
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Y.Tanizaki
///	@date			Create	: 2019/05/23<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void SetEcVolume( void )
{
	short i;
	memset(&VolumeTbl, 0x0,sizeof(EC_VOLUME_TBL));
	VolumeTbl.num = (char)prm_get(COM_PRM, S_SYS, 53, 1, 1);	// 01-0053
	if(VolumeTbl.num > 3) {
		VolumeTbl.num = 3;
	}

	if(VolumeTbl.num == 0) {
		VolumeTbl.ctrl[0].time = (ushort)prm_get(COM_PRM, S_SYS, 54, 4, 1);
		VolumeTbl.ctrl[0].vol = (uchar)prm_get(COM_PRM, S_ECR, 27, 2, 5);
		if(VolumeTbl.ctrl[0].vol == 0) {
			VolumeTbl.ctrl[0].vol = 1;
		} else if(VolumeTbl.ctrl[0].vol > EC_MAX_VOLUME) {
			VolumeTbl.ctrl[0].vol = EC_MAX_VOLUME;
		}
	} else {
		for(i=0; i < VolumeTbl.num; i++) {
			VolumeTbl.ctrl[i].time = (ushort)prm_get(COM_PRM, S_SYS, (short)(54+i), 4, 1);
			VolumeTbl.ctrl[i].vol = (uchar)(prm_get(COM_PRM, S_ECR, 27, 2, (char)(5-(i*2))));
			if(VolumeTbl.ctrl[i].vol > EC_MAX_VOLUME) {
				VolumeTbl.ctrl[i].vol = EC_MAX_VOLUME;
			}
		}
	}
}

// MH810105(S) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P
////[]----------------------------------------------------------------------[]
/////	@brief			EcDeemedSettlement
////[]----------------------------------------------------------------------[]
/////	@param[in]		uchar *ope_faze
/////	@return			void
////[]----------------------------------------------------------------------[]
/////	@author			Y.Tanizaki
/////	@date			Create	: 2019/06/10<br>
/////					Update
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//void EcDeemedSettlement( uchar *ope_faze )
//{
//// MH810103 GG119202(S) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
//	uchar	sts = ECCTL.Ec_Settlement_Sts;
//// MH810103 GG119202(E) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
//// MH810105 GG119202(S) ������������W�v�d�l���P
////// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
////	uchar	brand_index = 0;
////// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//// MH810105 GG119202(E) ������������W�v�d�l���P
//// MH810105 GG119202(S) ���Ϗ������ɏ�Q�������������̓���
//	uchar	process = 0;											// 0�F�݂Ȃ����ςŐ��Z����
//																	// 1�F�x��
//																	// 2�F�N���W�b�g���L�̏���(���ό���NG�Ƃ���)
//																	// 3�FQR�R�[�h���L�̏���(��Q�A���\���s)
//// MH810105 GG119202(E) ���Ϗ������ɏ�Q�������������̓���
//
//	// ��Ԃ̃N���A
//	OPECTL.InquiryFlg = 0;											// �J�[�h�������t���OOFF
//// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
////	Ec_Settlement_Sts = EC_SETT_STS_NONE;							// ���Ϗ�Ԃ̃N���A
//	ECCTL.Ec_Settlement_Sts = EC_SETT_STS_NONE;						// ���Ϗ�Ԃ̃N���A
//// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
//// MH810105 GG119202(S) �����c���Ɖ�ɏ�Q�����������ꍇ�͐ݒ�50-0014�ɏ]��
////// MH810103 GG119001_GG119101(S) �����c���Ɖ�ɒʐM�s�ǔ����ł݂Ȃ����ςɂȂ�
////	if (Suica_Rec.Data.BIT.MIRYO_CONFIRM != 0) {
////		ec_miryo_timeout();
////		return;
////	}
////// MH810103 GG119001_GG119101(E) �����c���Ɖ�ɒʐM�s�ǔ����ł݂Ȃ����ςɂȂ�
//// MH810105 GG119202(E) �����c���Ɖ�ɏ�Q�����������ꍇ�͐ݒ�50-0014�ɏ]��
//
//// MH810105 GG119202(S) ���Ϗ������ɏ�Q�������������̓���
////// MH810103 GG119202(S) �݂Ȃ����ψ������̓���
////	// �݂Ȃ����ψ������̓���i50-0014�j
////	if (prm_get(COM_PRM, S_ECR, 14, 1, 1) == 1) {
////		// �x�ƂƂ���
//	// ���Ϗ������ɏ�Q�������������̓���(50-0014)
//	process = EcGetActionWhenFailure( EcDeemedLog.EcDeemedBrandNo );
//
//	if( process == 2 || process == 3 ){
//		// ���ό���NG�Ƃ��Đ��Z�p��(�N���W�b�g�A�d�q�}�l�[�i�|�X�g�y�C�^�j)
//		// ��Q�A���[�𔭍s�����ό���NG�Ƃ��Đ��Z�p��(QR�R�[�h)
//		Ec_Settlement_Res.Result = EPAY_RESULT_NG;					// ���ό���NG�Ƃ��ď���������
//		OPECTL.InquiryFlg = 1;										// �J�[�h�������t���OON
//
//		// ���ό��ʃf�[�^����M�������Ƃɂ���
//		Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA = 1;
//		Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;
//		Suica_Rec.Data.BIT.CTRL_CARD = 0;							// �݂Ȃ����ώ��̓J�[�h����OFF�ɂ���(ON�ł�OFF�����o�ł��Ȃ�����)
//		Ec_Settlement_Res.Column_No = 0xFF;
//		EcRecvSettData(ope_faze);									// ���ό���NG����M�����Ƃ��ď���������
//
//		if (Suica_Rec.Data.BIT.CTRL_MIRYO != 0) {					// ������Ԕ������H 
//			LagCan500ms(LAG500_SUICA_MIRYO_RESET_TIMER);			// ������ԊĎ����ؾ��
//			Suica_Rec.Data.BIT.CTRL_MIRYO = 0;						// ������ԉ���
//		}
//		Suica_Rec.Data.BIT.CTRL = 0;
//		EC_STATUS_DATA.StatusInfo.ReceptStatus = 1;					// ����ް����e�Ɏ�t�s�¾��
//		EcRecvStatusData(ope_faze);
//// MH810105(S) #6199 ���Ϗ������̏�Q�����ł݂Ȃ����ρ{���ό���OK��M���A50-0014�̐ݒ�𖳎����ď�������������
//		// ��Ԃ��X�V
//		dspCyclicErrMsgRewrite();
//
//		lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x00);	// �|�b�v�A�b�v����(����I��)
//// MH810105(E) #6199 ���Ϗ������̏�Q�����ł݂Ȃ����ρ{���ό���OK��M���A50-0014�̐ݒ�𖳎����ď�������������
//
//		if( process == 3 ){
//// TODO�F��Q�A���\���s
//		}
//		return;
//	}
//	else if( process == 1 ){
//		// �x�ƂƂ���
//// MH810105 GG119202(E) ���Ϗ������ɏ�Q�������������̓���
//
//// MH810103 GG119202(S) ���Ϗ�ԁi�݂Ȃ����ρj�̔���C��
//		Lagcan( OPETCBNO, TIMERNO_EC_INQUIRY_WAIT );				// �⍇��(������)���ʑ҂��^�C�}�[���Z�b�g
//// MH810103 GG119202(E) ���Ϗ�ԁi�݂Ȃ����ρj�̔���C��
//		ec_MessageAnaOnOff(0, 0);
//// MH810103 GG118808_GG118908(S) ���������Ɠ����ɃJ�[�h�^�b�`�����������Ń��b�N����
//		if( ryo_buf.nyukin && OPECTL.credit_in_mony ){				// ��������œd�q�}�l�[�Ƃ̂���Ⴂ�����̏ꍇ
//			// �����z >= ���ԗ����̏�Ԃł݂Ȃ����ς̈������x�Ƃ̏ꍇ��
//			// ���Z�����Ƃ��Ĉ����A�Ƃ肯���{�^�������҂��ɂ��Ȃ�
//			if( OPECTL.credit_in_mony == 1 ){
//				in_mony ( COIN_EVT, 0 );							// ��ݲ���ē���������āA��������(ryo_buf�ر�̍X�V)���s��
//			}
//			else{/* OPECTL.credit_in_mony == 2 */
//				in_mony ( NOTE_EVT, 0 );							// ��������ē���������āA��������(ryo_buf�ر�̍X�V)���s��
//			}
//			OPECTL.credit_in_mony = 0;								// ����Ⴂ�׸޸ر
//		}
//
//		if (ryo_buf.zankin > 0) {
//			// �c��>0�̏ꍇ�̂ݕ\������
//// MH810103 GG118808_GG118908(E) ���������Ɠ����ɃJ�[�h�^�b�`�����������Ń��b�N����
//
//// MH810103 GG119202(S) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
////		EcRegistSettlementAbort();
//		// ���Ϗ������s(�x��)
//		EcRegistDeemedSettlementError(1);
//// MH810103 GG119202(E) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
//
//// MH810103(s) �d�q�}�l�[�Ή� �����ʒm
//		lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x0A);	// 10=10=�݂Ȃ����ϋx�ƈē�
//// MH810103(e) �d�q�}�l�[�Ή� �����ʒm
//// MH810105 GG119202(S) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
////		Ope2_ErrChrCyclicDispStart(2000, msg_idx);	// ���[�j���O�T�C�N���b�N�\��(2�b�Ԋu)
//		Ope2_ErrChrCyclicDispStart(2000, msg_idx, 0);	// ���[�j���O�T�C�N���b�N�\��(2�b�Ԋu)
//// MH810105 GG119202(E) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
//		LagTim500ms(LAG500_EC_AUTO_CANCEL_TIMER, (short)((30 * 2) + 1), ec_auto_cancel_timeout);	// ���Z�����L�����Z���^�C�}�J�n
//// MH810103 GG118808_GG118908(S) ���������Ɠ����ɃJ�[�h�^�b�`�����������Ń��b�N����
//		}
//// MH810103 GG118808_GG118908(E) ���������Ɠ����ɃJ�[�h�^�b�`�����������Ń��b�N����
//
//// MH810103 GG119202(S) �������ɒʐM�s�ǂŋx�Ɖ�ʂɑJ�ڂ��Ȃ�
//		if(Suica_Rec.Data.BIT.CTRL_MIRYO) {
//			LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );			/* ������ײ���ؾ�� */
//			Suica_Rec.Data.BIT.CTRL_MIRYO = 0;						// ������ԉ���
//// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
////// GG119202(S) �J�[�h��������M��̌���NG�i304�j�̈���
////			if (!IsErrorOccuerd((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_304)) {
////// GG119202(E) �J�[�h��������M��̌���NG�i304�j�̈���
//// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//			Suica_Rec.Data.BIT.MIRYO_NO_ANSWER = 1;					// �������Ȃ��A�����^�C���A�E�g�����ꍇ
//			Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 1;					// �����^�C���A�E�g����
//// MH810105 GG119202(S) ������������W�v�d�l���P
////			wmonlg(OPMON_EC_MIRYO_TIMEOUT, NULL, 0);				// ���j�^�o�^
////// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//////// GG119202(S) �J�[�h��������M��̌���NG�i304�j�̈���
//////			}
//////// GG119202(E) �J�[�h��������M��̌���NG�i304�j�̈���
////			// �u�݂Ȃ����ψ�����̓��쁁�x�Ɓv�̏ꍇ�������񐔂��J�E���g����
////			brand_index = convert_brandno(EcDeemedLog.EcDeemedBrandNo);
////			ec_alarm_syuu(brand_index, (ulong)Product_Select_Data);	// �A���[������W�v
////			if (sts != EC_SETT_STS_DEEMED_SETTLEMENT) {
////				// ���Ϗ�ԁ��u�݂Ȃ����ρv�ȊO�݂͂Ȃ��p���ό��ʃf�[�^���쐬����
////				EcSetDeemedSettlementResult(&Ec_Settlement_Res);
////			}
////			ReqEcAlmRctPrint(J_PRI, &Ec_Settlement_Res);			// �A���[�����V�[�g�󎚗v��
////			EcAlarmLog_Regist(&Ec_Settlement_Res);					// ��������������O�o�^
////// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//// MH810105 GG119202(E) ������������W�v�d�l���P
//		}
//		Suica_Rec.Data.BIT.CTRL = 0;
//		Suica_Rec.Data.BIT.OPE_CTRL = 0;
//// MH810103 GG119202(E) �������ɒʐM�s�ǂŋx�Ɖ�ʂɑJ�ڂ��Ȃ�
//		Suica_Rec.Data.BIT.MIRYO_ARM_DISP = 1;		// �װ�ү���ޕ\���p�׸޾��
//		Suica_Rec.Data.BIT.ADJUSTOR_START = 0;		// ���i�I���f�[�^���M�ۃt���O�N���A
//// MH810103 GG119202(S) �J�[�h��������M��̌���NG�i304�j�̈���
//		Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;		// ���ϒ�����t���O�N���A
//// MH810103 GG119202(E) �J�[�h��������M��̌���NG�i304�j�̈���
//// MH810104 GG119202(S) �݂Ȃ��x�Ǝ��̃t���O�N���A
//		Suica_Rec.Data.BIT.DATA_RCV_WAIT = 0;		// �҂����킹�t���O�N���A
//		Suica_Rec.suica_rcv_event.BYTE = 0;			// ��M�C�x���g�t���O�N���A
//// MH810104 GG119202(E) �݂Ȃ��x�Ǝ��̃t���O�N���A
//		ac_flg.ec_deemed_fg = 0;					// �݂Ȃ����ϕ��d�p�t���OOFF
//		return;
//	}
//// MH810103 GG119202(E) �݂Ȃ����ψ������̓���
//
//// MH810103 GG119202(S) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
//	// �݂Ȃ�����(���Z����)
//	EcRegistDeemedSettlementError(0);
//// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
//	// �݂Ȃ����ώ���deemed_sett_fin�t���O��ON�ɂ���
//	// deemed_sett_fin�t���O�̂�ON����Ă��鎞�́A�݂Ȃ����ρ{���ό���Ok��M�ƂȂ�
//	// �ʏ�݂̂Ȃ����ς�deemed_sett_fin��deemed_settlement�̃t���O��ON�ɂȂ�
//	PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin = 1;
//// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
//	if (sts == EC_SETT_STS_DEEMED_SETTLEMENT) {
//		// ���Ϗ�ԁ��u�݂Ȃ����ρv�͌��ό��ʃf�[�^�������ɒʒm����邽�߁A
//		// �G���[�o�^�݂̂Ƃ���
//// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//		if (Suica_Rec.Data.BIT.CTRL_MIRYO) {
//			LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );			/* ������ײ���ؾ�� */
//			Suica_Rec.Data.BIT.CTRL_MIRYO = 0;						// ������ԉ���
//			Suica_Rec.Data.BIT.MIRYO_NO_ANSWER = 1;					// �������Ȃ��A�����^�C���A�E�g�����ꍇ
//// MH810105 GG119202(S) �����m��̒���f�[�^��M����R0176��o�^����
////			wmonlg(OPMON_EC_MIRYO_TIMEOUT, NULL, 0);				// ���j�^�o�^
//// MH810105 GG119202(E) �����m��̒���f�[�^��M����R0176��o�^����
//		}
//// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//		return;
//	}
//// MH810103 GG119202(E) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
//
//	// �݂Ȃ����σf�[�^�쐬
//	memset(&Ec_Settlement_Res,0, sizeof(Ec_Settlement_Res));
//	Ec_Settlement_Res.Result = EPAY_RESULT_OK;						// ���ό��ʂ͏��OK
//	Ec_Settlement_Res.Column_No = 0xFF;								// ���σR�����ԍ��͒ʏ�1�̂��߁A0xFF���݂Ȃ����ςƂ���
//	Ec_Settlement_Res.settlement_data = Product_Select_Data;		// �I�����i�f�[�^�ő��M�������ϊz���Z�b�g
//
//	// �u�����h�ԍ��͌��Ϗ�ԃf�[�^��M���ɕۑ��������̂��g�p����
//	Ec_Settlement_Res.brand_no = (ushort)EcDeemedLog.EcDeemedBrandNo;
//
//	// ���Z�����͕s���̂��ߐ��Z�@�̓������Z�b�g
//	memcpy(&Ec_Settlement_Res.settlement_time, &CLK_REC, sizeof(struct clk_rec));
//
//	if(Suica_Rec.Data.BIT.CTRL_MIRYO) {
//		LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );				/* ������ײ���ؾ�� */
//		Suica_Rec.Data.BIT.CTRL_MIRYO = 0;							// ������ԉ���
//// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
////// GG119202(S) �J�[�h��������M��̌���NG�i304�j�̈���
////		if (!IsErrorOccuerd((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_304)) {
////// GG119202(E) �J�[�h��������M��̌���NG�i304�j�̈���
//// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//		Suica_Rec.Data.BIT.MIRYO_NO_ANSWER = 1;						// �������Ȃ��A�����^�C���A�E�g�����ꍇ
//// MH810105 GG119202(S) �����m��̒���f�[�^��M����R0176��o�^����
////		wmonlg(OPMON_EC_MIRYO_TIMEOUT, NULL, 0);					// ���j�^�o�^
//// MH810105 GG119202(E) �����m��̒���f�[�^��M����R0176��o�^����
//// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
////// GG119202(S) �J�[�h��������M��̌���NG�i304�j�̈���
////		}
////// GG119202(E) �J�[�h��������M��̌���NG�i304�j�̈���
//// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//	}
//
//// MH810103 GG119202(S) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
////// GG116201(S) �݂Ȃ����ϔ������ɃG���[�o�^����
////	// �݂Ȃ����σG���[�o�^(������������)
////	err_chk((char)jvma_setup.mdl, ERR_EC_DEEMED_SETTLEMENT, 2, 0, 0);
////// GG116201(E) �݂Ȃ����ϔ������ɃG���[�o�^����
//// MH810103 GG119202(E) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
//
//	// ���ό��ʃf�[�^����M�������Ƃɂ���
//// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
////	Suica_Rec.suica_rcv_event.BIT.SETTLEMENT_DATA = 1;
//	Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA = 1;
//// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
//	Suica_Rec.Data.BIT.LOG_DATA_SET = 1;
//	Suica_Rec.Data.BIT.ADJUSTOR_START = 0;
//// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//	Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;
//// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
//	Suica_Rec.Data.BIT.CTRL_CARD = 0;								// �݂Ȃ����ώ��̓J�[�h����OFF�ɂ���(ON�ł�OFF�����o�ł��Ȃ�����)
//	EcRecvSettData(ope_faze);
//
//	// ��ԃf�[�^����M�������Ƃɂ���
//	Suica_Rec.Data.BIT.CTRL = 0;
//	EC_STATUS_DATA.StatusInfo.ReceptStatus = 1;						// ����ް����e�Ɏ�t�s�¾��
//	EcRecvStatusData(ope_faze);
//}
//[]----------------------------------------------------------------------[]
///	@brief			���Ϗ������ɏ�Q�������������̓���
//[]----------------------------------------------------------------------[]
///	@param[in]		ope_faze	: OPE�̏��
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/16
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void EcDeemedSettlement(uchar *ope_faze)
{
	uchar	sts = ECCTL.Ec_Settlement_Sts;
	uchar	ec_result = Ec_Settlement_Res.Result;
	uchar	process = 0;											// 0�F����OK�Ƃ��Đ��Z����
																	// 1�F�x��
																	// 2�F���Z���~
																	// 3�FQR�R�[�h���ϓ��L�̏����i��Q�A���[���s�j
// MH810105(S) MH364301 �x�ƈē��ɖ₢���킹���\������
//	const uchar	msg_idx[4] = {										// �x�Ǝ��̃��b�Z�[�W
//		76, 50,														// "������Ɏ��s���܂�����"
//																	// "�W�������Ăщ�����"
//		84, 52,														// "�Ƃ肯���{�^����������"
//																	// "��ʂ��I�����܂�"
//	};
	uchar	msg_idx[4] = {											// �x�Ǝ��̃��b�Z�[�W
		76,  0,														// "����Ɏ��s���܂���"
																	// ""
		84, 52,														// "�Ƃ肯���{�^����������"
																	// "��ʂ��I�����܂�"
	};
	uchar	disp;
// MH810105(E) MH364301 �x�ƈē��ɖ₢���킹���\������


	// ��Ԃ̃N���A
	OPECTL.InquiryFlg = 0;											// �J�[�h�������t���OOFF
	ECCTL.Ec_Settlement_Sts = EC_SETT_STS_NONE;						// ���Ϗ�Ԃ̃N���A

	if (ec_result == EPAY_RESULT_MIRYO) {
		// �����c���Ɖ�^�C���A�E�g���̓���
		process = EcGetMiryoTimeoutAction(EcDeemedLog.EcDeemedBrandNo);
	}
	else {
		// ���Ϗ������ɏ�Q�������������̓���
		process = EcGetActionWhenFailure(EcDeemedLog.EcDeemedBrandNo);
	}
	if (process == 0) {
		// 0�F���ό���OK�i�݂Ȃ����ρj�Ƃ��Đ��Z����
		EcRegistDeemedSettlementError(0);							// �݂Ȃ����ρi���Z�����j

		if (Suica_Rec.Data.BIT.CTRL_MIRYO) {
			LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );			// ������ײ���ؾ��
			Suica_Rec.Data.BIT.CTRL_MIRYO = 0;						// ������ԉ���
			Suica_Rec.Data.BIT.MIRYO_NO_ANSWER = 1;					// �������Ȃ��A�����^�C���A�E�g�����ꍇ
		}

		// �݂Ȃ����ώ���deemed_sett_fin�t���O��ON�ɂ���
		// deemed_sett_fin�t���O�̂�ON����Ă��鎞�́A�݂Ȃ����ρ{���ό���OK��M�ƂȂ�
		// �ʏ�݂̂Ȃ����ς�deemed_sett_fin��deemed_settlement�̃t���O��ON�ɂȂ�
		PayData.Electron_data.Ec.E_Status.BIT.deemed_sett_fin = 1;
		if (sts == EC_SETT_STS_DEEMED_SETTLEMENT) {
			// ���Ϗ�ԁ��u�݂Ȃ����ρv�͌��ό��ʃf�[�^�������ɒʒm����邽�߁A
			// �G���[�o�^�݂̂Ƃ���
			return;
		}

		if (isEcBrandNoEMoney(EcDeemedLog.EcDeemedBrandNo, 1) &&
			ec_result == EPAY_RESULT_MIRYO) {
			// �v���y�C�h�^�Ō��ρ������m���M��
			// ����OK��M�������s�����߁A�t���O�N���A����
			Suica_Rec.suica_rcv_event_sumi.EC_BIT.SETTLEMENT_DATA = 0;
		}

		// �݂Ȃ����σf�[�^�쐬
		memset(&Ec_Settlement_Res, 0, sizeof(Ec_Settlement_Res));
		Ec_Settlement_Res.Result = EPAY_RESULT_OK;					// ���ό��ʂ͏��OK
		Ec_Settlement_Res.Column_No = 0xFF;							// ���σR�����ԍ��͒ʏ�1�̂��߁A0xFF���݂Ȃ����ςƂ���
		Ec_Settlement_Res.settlement_data = Product_Select_Data;	// �I�����i�f�[�^�ő��M�������ϊz���Z�b�g

		// �u�����h�ԍ��͌��Ϗ�ԃf�[�^��M���ɕۑ��������̂��g�p����
		Ec_Settlement_Res.brand_no = EcDeemedLog.EcDeemedBrandNo;

		// ���Z�����͕s���̂��ߐ��Z�@�̓������Z�b�g
		memcpy(&Ec_Settlement_Res.settlement_time, &CLK_REC, sizeof(CLK_REC));

		// ���ό��ʃf�[�^����M�������Ƃɂ���
		Suica_Rec.suica_rcv_event.EC_BIT.SETTLEMENT_DATA = 1;
		Suica_Rec.Data.BIT.LOG_DATA_SET = 1;
		Suica_Rec.Data.BIT.ADJUSTOR_START = 0;
		Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;
		Suica_Rec.Data.BIT.CTRL_CARD = 0;							// �݂Ȃ����ώ��̓J�[�h����OFF�ɂ���(ON�ł�OFF�����o�ł��Ȃ�����)
		EcRecvSettData(ope_faze);

		// ��ԃf�[�^����M�������Ƃɂ���
		Suica_Rec.Data.BIT.CTRL = 0;
		EC_STATUS_DATA.StatusInfo.ReceptStatus = 1;					// ����ް����e�Ɏ�t�s�¾��
		EcRecvStatusData(ope_faze);
	}
	else {
		// 1�F�x��
		// 2�F���Z���~
		// 3�F��Q�A���[�𔭍s���Đ��Z���~

		Lagcan( OPETCBNO, TIMERNO_EC_INQUIRY_WAIT );				// �⍇��(������)���ʑ҂��^�C�}�[���Z�b�g
		ec_MessageAnaOnOff(0, 0);

		if( ryo_buf.nyukin && OPECTL.credit_in_mony ){				// ��������œd�q�}�l�[�Ƃ̂���Ⴂ�����̏ꍇ
			// �����z >= ���ԗ����̏�Ԃ�
			// ���Ϗ������ɏ�Q�������������̓��쁁�x�ƁA���Z���~�̏ꍇ��
			// ���Z�����Ƃ��Ĉ����A�Ƃ肯���{�^�������҂��ɂ��Ȃ�
			in_mony ( COIN_EVT, 0 );								// ��ݲ���ē���������āA��������(ryo_buf�ر�̍X�V)���s��
			OPECTL.credit_in_mony = 0;								// ����Ⴂ�׸޸ر
		}

		if (ryo_buf.zankin > 0) {
			// �c��������ꍇ
			switch (process) {
			case 1:		// �x��
				if (ec_result != EPAY_RESULT_MIRYO) {
					EcRegistDeemedSettlementError(1);				// ���Ϗ������s�i�x�Ɓj
				}
// MH810105(S) MH364301 �x�ƈē��ɖ₢���킹���\������
//				Ope2_ErrChrCyclicDispStart(2000, msg_idx, 0);		// ���[�j���O�T�C�N���b�N�\��(2�b�Ԋu)
				// �₢���킹��\��
				disp = (uchar)prm_get(COM_PRM, S_DSP, 37, 1, 1);
				if (disp > 5) {
					disp = 0;
				}

				msg_idx[1] = (uchar)(disp + 78);
				Ope2_ErrChrCyclicDispStart(2000, msg_idx, 1);		// ���[�j���O�T�C�N���b�N�\��(2�b�Ԋu)
// MH810105(E) MH364301 �x�ƈē��ɖ₢���킹���\������
				LagTim500ms(LAG500_EC_AUTO_CANCEL_TIMER,
							(short)((30 * 2) + 1),
							ec_auto_cancel_timeout);				// ���Z�����L�����Z���^�C�}�J�n
// MH810105(S) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P GT-4100
				lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x0A);	// 10=10=�݂Ȃ����ϋx�ƈē�
// MH810105(E) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P GT-4100
				break;
			case 2:		// ���Z���~
				EcRegistDeemedSettlementError(3);					// ���Ϗ������s�i���Z���~�j
				if (ec_result == EPAY_RESULT_MIRYO) {
					miryo_timeout_after_disp();
				}
				else {
					lcd_wmsg_dsp_elec(1, ERR_CHR[76], ERR_CHR[85], 0, 0, COLOR_RED, LCD_BLINK_OFF);
				}
// MH810105(S) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P GT-4100
				lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x06);	// 6=����(�Đ��Z�ē�)
// MH810105(E) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P GT-4100
				break;
			case 3:		// ��Q�A���[�𔭍s���Đ��Z���~
				EcRegistDeemedSettlementError(3);					// ���Ϗ������s�i���Z���~�j
				if (Ope_isPrinterReady() &&							// ���V�[�g�o�͉�
					sts == EC_SETT_STS_DEEMED_SETTLEMENT) {			// ���Ϗ�ԁ��u�݂Ȃ����ρv��M
					lcd_wmsg_dsp_elec(1, ERR_CHR[76], ERR_CHR[77], 0, 0, COLOR_RED, LCD_BLINK_OFF);
					ECCTL.Ec_FailureContact_Sts = 1;				// 1:��Q�A���[���s������
					if( OPECTL.RECI_SW == 1 ){						// �̎������ݎg�p�i�����j�ρH
						OPECTL.RECI_SW = 0;							// �̎������ݖ��g�p
					}
// MH810105(S) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P GT-4100
					lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x0D);	// 13=��Q�A���[���s�ē�(�R�[�h����)
// MH810105(E) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P GT-4100
				}
				else {
					lcd_wmsg_dsp_elec(1, ERR_CHR[76], ERR_CHR[85], 0, 0, COLOR_RED, LCD_BLINK_OFF);
					ECCTL.Ec_FailureContact_Sts = 0;				// 0:��Q�A���[���s�s�i���ł�0�ł��邪�����I�ɾ�āj
// MH810105(S) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P GT-4100
					lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x0F);	// 15=��Q�A���[���s���s(�R�[�h����)
// MH810105(E) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P GT-4100
				}
				break;
			default:
				break;
			}
		}

		if (sts == EC_SETT_STS_DEEMED_SETTLEMENT) {
			// ���Ϗ�ԁ��u�݂Ȃ����ρv�͌��ό��ʃf�[�^�������ɒʒm����邽�߁A
			// ���ό��ʂ̂ݏ���������
			Ec_Settlement_Res.Result = EPAY_RESULT_NG;				// ���ό���NG�Ƃ��ď���������
			// ���d�p���ό��ʃf�[�^���X�V
			memcpy(&EcAlarm.Ec_Res, &Ec_Settlement_Res, sizeof(Ec_Settlement_Res));
		}
		else {
			if (ec_result == EPAY_RESULT_MIRYO) {
				// ���Z�����͕s���̂��ߐ��Z�@�̓������Z�b�g
				memcpy(&Ec_Settlement_Res.settlement_time, &CLK_REC, sizeof(CLK_REC));
				// ���d�p���ό��ʃf�[�^���X�V
				memcpy(&EcAlarm.Ec_Res, &Ec_Settlement_Res, sizeof(Ec_Settlement_Res));
			}
		}

		if (ryo_buf.zankin > 0) {
			if( sts != EC_SETT_STS_DEEMED_SETTLEMENT ){
				// �݂Ȃ��ɂ�錈�ϒ��~�ł���
				// ���Ϗ�ԁ��u�݂Ȃ����ρv�ȊO��deemSettleCancal�׸ނ�ON
				EcAlarm.Ec_Res.E_Flag.BIT.deemSettleCancal = 1;
			}
			// �c��������ꍇ
			// ���Z���~�f�[�^�𑗐M���邽�߁A���Ϗ���PayData�ɃZ�b�g
			switch (EcDeemedLog.EcDeemedBrandNo) {
			case BRANDNO_CREDIT:
				EcCreUpdatePayData(&EcAlarm.Ec_Res);
				break;
			case BRANDNO_QR:
				EcQrSet_PayData(&EcAlarm.Ec_Res);
				break;
			default:
				EcElectronSet_PayData(&EcAlarm.Ec_Res);
				break;
			}
		}

		if (Suica_Rec.Data.BIT.CTRL_MIRYO) {
			LagCan500ms( LAG500_SUICA_MIRYO_RESET_TIMER );			// ������ײ���ؾ��
			Suica_Rec.Data.BIT.CTRL_MIRYO = 0;						// ������ԉ���
			Suica_Rec.Data.BIT.MIRYO_NO_ANSWER = 1;					// �������Ȃ��A�����^�C���A�E�g�����ꍇ
			Suica_Rec.Data.BIT.MIRYO_TIMEOUT = 1;					// �����^�C���A�E�g����
		}

		if (ec_result != EPAY_RESULT_MIRYO) {
			// �����m���M���ȊO
			Suica_Rec.Data.BIT.CTRL = 0;
			Suica_Rec.Data.BIT.OPE_CTRL = 0;
			Suica_Rec.Data.BIT.ADJUSTOR_START = 0;					// ���i�I���f�[�^���M�ۃt���O�N���A
			Suica_Rec.Data.BIT.ADJUSTOR_NOW = 0;					// ���ϒ�����t���O�N���A
			Suica_Rec.Data.BIT.DATA_RCV_WAIT = 0;					// �҂����킹�t���O�N���A
			Suica_Rec.suica_rcv_event.BYTE = 0;						// ��M�C�x���g�t���O�N���A
		}

		Suica_Rec.Data.BIT.MIRYO_ARM_DISP = 1;						// �װ�ү���ޕ\���p�׸޾��
		ac_flg.cycl_fg = 10;										// ������
		ac_flg.ec_deemed_fg = 0;									// �݂Ȃ����ϕ��d�p�t���OOFF
	}
}
// MH810105(E) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P

//[]----------------------------------------------------------------------[]
///	@brief			save_deemed_data
//[]----------------------------------------------------------------------[]
///	@param[in]		short brandno
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Y.Tanizaki
///	@date			Create	: 2019/06/11<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	save_deemed_data(short brandno)
{
// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//	ushort num = 0;

//	// �݂Ȃ����ϕ��d�p�f�[�^�ۑ�
//	memset(&EcDeemedLog, 0, sizeof(EcDeemedLog));
//	EcDeemedLog.PayPrice = Product_Select_Data;						// �݂Ȃ����ϊz
//	EcDeemedLog.syu = (uchar)(ryo_buf.syubet + 1);					// ���Ԏ��
//	if( PayData.teiki.update_mon ){									// �X�V���Z?
//		EcDeemedLog.WPlace = 9999;									// �X�V���͎Ԏ��ԍ�9999�Ƃ���B
//		EcDeemedLog.update_mon = PayData.teiki.update_mon;			// �X�V����
//	}else if( ryo_buf.pkiti == 0xffff ){							// ������Z?
//		EcDeemedLog.WPlace = LOCKMULTI.LockNo;						// �ڋq�p���Ԉʒu�ԍ�
//// GG119202(S) ���Z���~�i���j�󎚂ŎԎ��ԍ����󎚂���Ȃ�
////	}else if( !OPECTL.Pr_LokNo ){									// ������Z�H
//	}else if( ryo_buf.pkiti == 0 ){									// ������Z�H
//		// ���u���Z�i������Z�j�͒��Ԉʒu�ԍ����O�̂��߁A��M�����Ԏ������Z�b�g����
//// GG119202(E) ���Z���~�i���j�󎚂ŎԎ��ԍ����󎚂���Ȃ�
//		EcDeemedLog.WPlace = OPECTL.Op_LokNo;
//	}else{
//		num = ryo_buf.pkiti - 1;
//		EcDeemedLog.WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );	// �ڋq�p���Ԉʒu�ԍ�
//	}
//	if( ryo_buf.ryo_flg < 2 ){										// ���Ԍ����Z����
//		EcDeemedLog.WPrice = ryo_buf.tyu_ryo;						// ���ԗ���
//	}else{
//		EcDeemedLog.WPrice = ryo_buf.tei_ryo;						// �������
//	}
//// MH810100(S) S.Nishimoto 2020/08/20 �Ԕԃ`�P�b�g���X(#4602:���ϐ��Z���~�i���j�ŎԔԏ�񂪈������Ȃ�)
//	EcDeemedLog.CarSearchFlg = PayData.CarSearchFlg;
//	memcpy(EcDeemedLog.CarSearchData, PayData.CarSearchData, sizeof(EcDeemedLog.CarSearchData));
//	memcpy(EcDeemedLog.CarNumber, PayData.CarNumber, sizeof(EcDeemedLog.CarNumber));
//	memcpy(EcDeemedLog.CarDataID, PayData.CarDataID, sizeof(EcDeemedLog.CarDataID));
//// MH810100(E) S.Nishimoto 2020/08/20 �Ԕԃ`�P�b�g���X(#4602:���ϐ��Z���~�i���j�ŎԔԏ�񂪈������Ȃ�)
//	EcDeemedLog.TInTime.Year = car_in_f.year;						// ���� �N
//	EcDeemedLog.TInTime.Mon  = car_in_f.mon;						//      ��
//	EcDeemedLog.TInTime.Day  = car_in_f.day;						//      ��
//	EcDeemedLog.TInTime.Hour = car_in_f.hour;						//      ��
//	EcDeemedLog.TInTime.Min  = car_in_f.min;						//      ��
//	EcDeemedLog.EcDeemedBrandNo = (ushort)brandno;
//// GG119202(S) IC�N���W�b�g�݂Ȃ����ψ󎚃t�H�[�}�b�g�ύX�Ή�
////	memcpy(&EcDeemedLog.EcDeemedDate, &CLK_REC, sizeof(date_time_rec));
//	EcDeemedLog.TOutTime.Year = car_ot_f.year;						// �o�� �N
//	EcDeemedLog.TOutTime.Mon  = car_ot_f.mon;						//      ��
//	EcDeemedLog.TOutTime.Day  = car_ot_f.day;						//      ��
//	EcDeemedLog.TOutTime.Hour = car_ot_f.hour;						//      ��
//	EcDeemedLog.TOutTime.Min  = car_ot_f.min;						//      ��
//// GG119202(E) IC�N���W�b�g�݂Ȃ����ψ󎚃t�H�[�}�b�g�ύX�Ή�
//// GG119202(S) ���ϐ��Z���~�󎚏����C��
////	ac_flg.ec_alarm = 3;	// ���d�p�t���O�Z�b�g
//	ac_flg.ec_deemed_fg = 1;	// �݂Ȃ����ϕ��d�p�t���O�Z�b�g
//// GG119202(E) ���ϐ��Z���~�󎚏����C��
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
	EC_SETTLEMENT_RES	*pRes;
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
	// �݂Ȃ����ϕ��d�p�f�[�^�ۑ�
	memset(&EcDeemedLog, 0, sizeof(EcDeemedLog));
	EcDeemedLog.PayPrice = Product_Select_Data;					// �݂Ȃ����ϊz
	EcDeemedLog.EcDeemedBrandNo = (ushort)brandno;				// �u�����h�ԍ�

// MH810100(S) S.Nishimoto 2020/08/20 �Ԕԃ`�P�b�g���X(#4602:���ϐ��Z���~�i���j�ŎԔԏ�񂪈������Ȃ�)
	EcDeemedLog.CarSearchFlg = PayData.CarSearchFlg;
	memcpy(EcDeemedLog.CarSearchData, PayData.CarSearchData, sizeof(EcDeemedLog.CarSearchData));
	memcpy(EcDeemedLog.CarNumber, PayData.CarNumber, sizeof(EcDeemedLog.CarNumber));
	memcpy(EcDeemedLog.CarDataID, PayData.CarDataID, sizeof(EcDeemedLog.CarDataID));
// MH810100(E) S.Nishimoto 2020/08/20 �Ԕԃ`�P�b�g���X(#4602:���ϐ��Z���~�i���j�ŎԔԏ�񂪈������Ȃ�)

// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
	pRes = &EcAlarm.Ec_Res;
	memset(pRes, 0, sizeof(*pRes));
	pRes->Result = EPAY_RESULT_NG;								// ���ό���NG�Ƃ��ď���������
	pRes->settlement_data = EcDeemedLog.PayPrice;				// ���ϊz���Z�b�g
	pRes->brand_no = EcDeemedLog.EcDeemedBrandNo;				// ���σu�����h���Z�b�g
	// ���݂̓������Z�b�g
	memcpy(&pRes->settlement_time, &CLK_REC, sizeof(CLK_REC));

	// �J�[�h�ԍ����X�y�[�X�ŏ�����
	memset(pRes->Card_ID, 0x20, sizeof(pRes->Card_ID));
	// �₢���킹�ԍ�
	memset(pRes->inquiry_num, '*', sizeof(pRes->inquiry_num));
	pRes->inquiry_num[4] = '-';
	switch (brandno) {
	case BRANDNO_CREDIT:
		// �J�[�h�ԍ�
		memset(pRes->Card_ID, '*', ECARDID_SIZE_CREDIT);
		// ���FNo.
		memset(pRes->Brand.Credit.Approval_No, '*',
				sizeof(pRes->Brand.Credit.Approval_No));
		// �[��No.
		memset(pRes->Brand.Credit.Id_No, '*',
				sizeof(pRes->Brand.Credit.Id_No));
		break;
	case BRANDNO_KOUTSUU:
		// �J�[�h�ԍ�
		memset(pRes->Card_ID, '*', ECARDID_SIZE_KOUTSUU);
		// �[���ԍ�
		memset(pRes->Brand.Koutsuu.SPRW_ID, '*',
				sizeof(pRes->Brand.Koutsuu.SPRW_ID));
		break;
	case BRANDNO_EDY:
		// �J�[�h�ԍ�
		memset(pRes->Card_ID, '*', ECARDID_SIZE_SUICA);
		// ��ʒ[��ID
		memset(pRes->Brand.Edy.TerminalNo, '*',
				sizeof(pRes->Brand.Edy.TerminalNo));
		// Edy����ʔ�
		memset(pRes->Brand.Edy.DealNo, '*',
				sizeof(pRes->Brand.Edy.DealNo));
		// �J�[�h����ʔ�
		memset(pRes->Brand.Edy.CardDealNo, '*',
				sizeof(pRes->Brand.Edy.CardDealNo));
		break;
	case BRANDNO_WAON:
		// �J�[�h�ԍ�
		memset(pRes->Card_ID, '*', ECARDID_SIZE_SUICA);
		// SPRWID
		memset(pRes->Brand.Waon.SPRW_ID, '*',
				sizeof(pRes->Brand.Waon.SPRW_ID));
		break;
	case BRANDNO_NANACO:
		// �J�[�h�ԍ�
		memset(pRes->Card_ID, '*', ECARDID_SIZE_SUICA);
		// �[������ʔ�
		memset(pRes->Brand.Nanaco.DealNo, '*',
				sizeof(pRes->Brand.Nanaco.DealNo));
		// ��ʒ[��ID
		memset(pRes->Brand.Nanaco.TerminalNo, '*',
				sizeof(pRes->Brand.Nanaco.TerminalNo));
		break;
	case BRANDNO_QUIC_PAY:
		// �J�[�h�ԍ�
		memset(pRes->Card_ID, '*', sizeof(pRes->Card_ID));
		// ��ʒ[��ID
		memset(pRes->Brand.QuicPay.TerminalNo, '*',
				sizeof(pRes->Brand.QuicPay.TerminalNo));
		// ���F�ԍ�
		memset(pRes->Brand.QuicPay.Approval_No, '*',
				sizeof(pRes->Brand.QuicPay.Approval_No));
		break;
	case BRANDNO_ID:
		// �J�[�h�ԍ�
		memset(pRes->Card_ID, '*', ECARDID_SIZE_SUICA);
		// ��ʒ[��ID
		memset(pRes->Brand.Id.TerminalNo, '*',
				sizeof(pRes->Brand.Id.TerminalNo));
		// ���F�ԍ�
		memset(pRes->Brand.Id.Approval_No, '*',
				sizeof(pRes->Brand.Id.Approval_No));
		break;
	case BRANDNO_PITAPA:
		// �J�[�h�ԍ�
		memset(pRes->Card_ID, '*', ECARDID_SIZE_KOUTSUU);
		// ��ʒ[��ID
		memset(pRes->Brand.Pitapa.TerminalNo, '*',
				sizeof(pRes->Brand.Pitapa.TerminalNo));
		// ���F�ԍ�
		memset(pRes->Brand.Pitapa.Approval_No, '*',
				sizeof(pRes->Brand.Pitapa.Approval_No));
		// �`�[�ԍ�
		memset(pRes->Brand.Pitapa.Slip_No, '*',
				sizeof(pRes->Brand.Pitapa.Slip_No));
		break;
	case BRANDNO_QR:
		// �[��ID
		memset(pRes->Brand.Qr.PayTerminalNo, '*',
				sizeof(pRes->Brand.Qr.PayTerminalNo));
		// ����ԍ�
		pRes->inquiry_num[4] = '*';
		break;
	default:
		break;
	}
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX

	ac_flg.ec_deemed_fg = 1;									// �݂Ȃ����ϕ��d�p�t���O�Z�b�g
// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
}

//[]----------------------------------------------------------------------[]
///	@brief			ReqEcDeemedJnlPrint
//[]----------------------------------------------------------------------[]
///	@param			void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Y.Tanizaki
///	@date			Create	:	2019/06/11<br>
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	ReqEcDeemedJnlPrint(void)
{
// MH810103 GG119202(S) ���ϐ��Z���~�󎚏����C��
//// GG119202(S) IC�N���W�b�g�݂Ȃ����ψ󎚃t�H�[�}�b�g�ύX�Ή�
////	static T_FrmEcDeemedJnlReq	FrmDeemedJnlReq;
////	static EcDeemedFukudenLog	deemed_jnl_info;
////
////	memset(&FrmDeemedJnlReq, 0, sizeof(T_FrmEcAlmRctReq));
////	memset(&deemed_jnl_info, 0, sizeof(deemed_jnl_info));
////
////	FrmDeemedJnlReq.prn_kind = J_PRI;											// �󎚐�
////	memcpy(&deemed_jnl_info, &EcDeemedLog, sizeof(EcDeemedLog));				// �݂Ȃ����Ϗ��
////	FrmDeemedJnlReq.deemed_jnl_info = &deemed_jnl_info;
////	queset(PRNTCBNO, PREQ_EC_DEEMED_LOG, sizeof(FrmDeemedJnlReq), &FrmDeemedJnlReq);
//
//	T_FrmReceipt ReceiptPreqDeemedData;
//
//	memset( &ReceiptPreqDeemedData, 0, sizeof(T_FrmReceipt) );
//
//	PayData.WFlag = ON;															// ���d�t���OON
//	PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement = 1;				// �݂Ȃ����σt���OON
//	PayData.Kikai_no = (uchar)CPrmSS[S_PAY][2];									// �@�B��
//	// EcDeemedLog�̊e�f�[�^��Receipt_data�^�ɓ���ւ���
//	PayData.teiki.update_mon = EcDeemedLog.update_mon;
//	PayData.syu = EcDeemedLog.syu;
//	PayData.WPrice = EcDeemedLog.WPrice;
//	PayData.Electron_data.Ec.pay_ryo = EcDeemedLog.PayPrice;
//	memcpy( (date_time_rec *)&PayData.TInTime, &EcDeemedLog.TInTime, sizeof(date_time_rec) );
//	PayData.Electron_data.Ec.e_pay_kind = convert_brandno( EcDeemedLog.EcDeemedBrandNo );
//	memcpy( (date_time_rec *)&PayData.TOutTime, &EcDeemedLog.TOutTime, sizeof(date_time_rec) );
//
//	// �ڼޯĶ��ތ��ς̏ꍇ�A�ڼޯĎx�����z�ɂ��������Z�b�g
//	if( EcDeemedLog.EcDeemedBrandNo == BRANDNO_CREDIT ){
//		ryo_buf.credit.pay_ryo = EcDeemedLog.PayPrice;
//	}
//
//	// �������łȂ���΁A���Z���Z���~(��)���󎚂���
//	// �������ł���΁A�]���ʂ�̐��Z���~(��)���󎚂���
//	if( ac_flg.cycl_fg == 1 ){
//		CountGet( PAYMENT_COUNT, &PayData.Oiban );									// ���Z�ǔ�
//		// ryo_buf����PayData�ɓ���ւ���
//		PayData.credit.pay_ryo = ryo_buf.credit.pay_ryo;
//
//		ReceiptPreqDeemedData.prn_kind = J_PRI;										// �󎚐�
//		ReceiptPreqDeemedData.prn_data = (Receipt_data *)&PayData;
//		queset(PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqDeemedData);
//	}
//// GG119202(E) IC�N���W�b�g�݂Ȃ����ψ󎚃t�H�[�}�b�g�ύX�Ή�

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	// �݂Ȃ��t���O���Z�b�g����
//	PayData.Electron_data.Ec.E_Status.BIT.deemed_settlement = 1;
	// �݂Ȃ����ρi�݂Ȃ����ό��ʖ���M�j�ɂ�鐸�Z���~�t���O���Z�b�g
	PayData.Electron_data.Ec.E_Flag.BIT.deemSettleCancal = 1;
	// �����m��^��Q�����t���O���Z�b�g����
	PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm = 1;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	// ���ώ�ʂ��Z�b�g����
	PayData.Electron_data.Ec.e_pay_kind = convert_brandno(EcDeemedLog.EcDeemedBrandNo);
	// �������Z�b�g����
	if (EcDeemedLog.EcDeemedBrandNo == BRANDNO_CREDIT) {
		ryo_buf.credit.pay_ryo = EcDeemedLog.PayPrice;
	}
	else {
		PayData.Electron_data.Ec.pay_ryo = EcDeemedLog.PayPrice;
// MH810105 GG119202(S) ������������W�v�d�l���P
//// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
//		if (EcDeemedLog.MiryoFlg != 0) {
//			// �������̕��d�͖����񐔂��J�E���g����
//			ec_alarm_syuu(PayData.Electron_data.Ec.e_pay_kind, PayData.Electron_data.Ec.pay_ryo);
//// MH810103 GG118808_GG118908(S) �ă^�b�`�҂����ɕ��d�ŏ�����������L�^���o�^����Ȃ�
//			EcSetDeemedSettlementResult(&Ec_Settlement_Res);
//			ReqEcAlmRctPrint(J_PRI, &Ec_Settlement_Res);			// �A���[�����V�[�g�󎚗v��
//			EcAlarmLog_Regist(&Ec_Settlement_Res);					// ��������������O�o�^
//// MH810103 GG118808_GG118908(E) �ă^�b�`�҂����ɕ��d�ŏ�����������L�^���o�^����Ȃ�
//// MH810104 GG119201(S) �ă^�b�`�҂����̕��d��R0176���o�^����Ȃ�
//			wmonlg(OPMON_EC_MIRYO_TIMEOUT, NULL, 0);				// ���j�^�o�^
//// MH810104 GG119201(E) �ă^�b�`�҂����̕��d��R0176���o�^����Ȃ�
//			EcDeemedLog.MiryoFlg = 0;
//		}
//// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
// MH810105 GG119202(E) ������������W�v�d�l���P
	}

// MH810103 GG119202(S) �݂Ȃ����ψ������̓���
	// �݂Ȃ����ψ������̓���i���d�j�i50-0015�j
	if (prm_get(COM_PRM, S_ECR, 15, 1, 1) == 1) {
		// �x�ƂƂ���
		queset( OPETCBNO, EC_EVT_DEEMED_SETTLE_FUKUDEN, 0, 0 );
	}
// MH810103 GG119202(E) �݂Ȃ����ψ������̓���
// MH810103 GG119202(E) ���ϐ��Z���~�󎚏����C��
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	// ���ϐ��Z���~�i���j�ł�EcElectronSet_PayData���APayData�����֐���ʂ�Ȃ�
	// �����ŕ��d����ID152�f�[�^���Z�b�g����
	Ec_Data152Save();
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
}

//[]----------------------------------------------------------------------[]
///	@brief			ec_errcode_err_chk
//[]----------------------------------------------------------------------[]
// MH810103 GG119202(S) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
/////	@param			uchar *err_wk, uchar *ec_err_code
///	@param			uchar *err_wk, uchar *ec_err_code, uchar brand_index
// MH810103 GG119202(E) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			D.Inaba
///	@date			Create	:	2019/10/16<br>
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810103 GG119202(S) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
//static	void	ec_errcode_err_chk(uchar *err_wk, uchar *ec_err_code)
static	void	ec_errcode_err_chk(uchar *err_wk, uchar *ec_err_code, uchar brand_index)
// MH810103 GG119202(E) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
{
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
	uchar ec_Disp_No = 0;
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
	uchar registed_errcode = 0;				// 0 = �G���[���o�^�@1 = �G���[�o�^
	uchar err_code_under2 = (uchar)astoin(&ec_err_code[1], 2);	// �G���[�R�[�h��2��
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	uchar subbrand_index = Ec_Settlement_Res.Brand.Qr.PayKind;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	if( OPECTL.InquiryFlg == 0 ){			// �J�[�h�������ł͂Ȃ��H
// MH810105 GG119202(S) �݂Ȃ����σv�����g�ɒ��O������e���󎚂���
//		if( ec_errcode_err_chk_judg( &ec_err_code[0] ) ){	// �^�C���A�E�g�G���[�H
		if( ec_errcode_err_chk_judg( &ec_err_code[0], EcTimeoutErrTbl, NG_ERR_MAX ) ){	// �^�C���A�E�g�G���[�H
// MH810105 GG119202(E) �݂Ȃ����σv�����g�ɒ��O������e���󎚂���
			// �^�C���A�E�g�G���[�̏ꍇ�Aec_Disp_No�� 99 �ɂ���
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//			ec_Disp_No = 99;
			ECCTL.ec_Disp_No = 99;
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
			return;
		}
	}

// MH810103 GG119202(S) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	if ('a' <= ec_err_code[0] && ec_err_code[0] <= 'z') {
	if (('a' <= ec_err_code[0] && ec_err_code[0] <= 'z') || 
		('A' <= ec_err_code[0] && ec_err_code[0] <= 'Z')) {
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		sprintf((char*)&err_wk[13], "-%s", EcBrandKind_prn[brand_index - EC_USED]);
// MH810105(S) MH364301 PiTaPa�Ή�
// �x�����@��PiTaPa���A���R�R�[�h���Z�b�g����Ă���ꍇ�̂�
// �W���[�i���󎚃f�[�^�ɗ��R�R�[�h��ǉ�����
		if(brand_index == EC_PITAPA_USED && Ec_Settlement_Res.Brand.Pitapa.Reason_Code[0] != 0x00){
			memcpy(&err_wk[16],"���R�R�[�h",10);
			memcpy(&err_wk[26],
					Ec_Settlement_Res.Brand.Pitapa.Reason_Code,
					sizeof(Ec_Settlement_Res.Brand.Pitapa.Reason_Code));
		}
// MH810105(E) MH364301 PiTaPa�Ή�
	}
// MH810103 GG119202(E) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
	switch( ec_err_code[0] ){
		case 'C':
			if(EcErrCodeTbl[EC_ERR_C][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_CKP, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_C][err_code_under2];
			}
			break;
		case 'D':
			if(EcErrCodeTbl[EC_ERR_D][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_DES, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_D][err_code_under2];
			}
			break;
		case 'E':
			if(EcErrCodeTbl[EC_ERR_E][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_DES, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_E][err_code_under2];
			}
			break;
		case 'G':
			if(EcErrCodeTbl[EC_ERR_G][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_G, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_G][err_code_under2];
// MH810103(s) �d�q�}�l�[�Ή� WAON���ώ��̎c���s�����b�Z�[�W���e��ύX #5401 
				if (brand_index == EC_WAON_USED &&
					err_code_under2 == 6 ) {
					// �\�����郁�b�Z�[�W��ύX����iG06�j
					ec_Disp_No = 198;
				}
// MH810103(e) �d�q�}�l�[�Ή� WAON���ώ��̎c���s�����b�Z�[�W���e��ύX #5401 
			}
			break;
		case 'K':
			if(EcErrCodeTbl[EC_ERR_K][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_CKP, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_K][err_code_under2];
			}
			break;
		case 'P':
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
			if (brand_index == EC_QR_USED) {
				// QR�R�[�h�p�G���[
				switch (err_code_under2) {
				// �d��ڍ׃G���[�R�[�h
				case 0:
				case 1:
				case 2:
					if (EcErrCodeTbl[EC_ERR_QR_P][err_code_under2]) {
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_CRITICAL_ERR, 1, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = EcErrCodeTbl[EC_ERR_QR_P][err_code_under2];
					}
					break;
				// �ʏ�ڍ׃G���[�R�[�h
				default:
					if (EcErrCodeTbl[EC_ERR_QR_P][err_code_under2]) {
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = EcErrCodeTbl[EC_ERR_QR_P][err_code_under2];
					}
					break;
				}
				break;
			}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
			if(EcErrCodeTbl[EC_ERR_P][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_CKP, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_P][err_code_under2];
			}
			break;
		case 'S':
			if(EcErrCodeTbl[EC_ERR_S][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_DES, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_S][err_code_under2];
			}
			break;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		case 'Q':
			// QR�R�[�h�p�G���[
			switch (err_code_under2) {
			// �d��ڍ׃G���[�R�[�h
			case 0:
			case 3:
				if (EcErrCodeTbl[EC_ERR_QR_Q][err_code_under2]) {
					err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_CRITICAL_ERR, 1, 1, 0, (void*)err_wk);
					registed_errcode = 1;
					ec_Disp_No = EcErrCodeTbl[EC_ERR_QR_Q][err_code_under2];
				}
				break;
			// �ʏ�ڍ׃G���[�R�[�h
			default:
				if (EcErrCodeTbl[EC_ERR_QR_Q][err_code_under2]) {
					err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
					registed_errcode = 1;
					ec_Disp_No = EcErrCodeTbl[EC_ERR_QR_Q][err_code_under2];

					if (err_code_under2 == 35 && subbrand_index == ALIPAY) {	// Q35�̎�
						// ����Ɏ��s���܂���
						// ���̎x�����@�������p�������� 
						ec_Disp_No = 10;		// ALIPAY�̂݃��b�Z�[�W���ύX
					}
				}
				break;
			}
			break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810103 GG119202(S) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
		case 'a':
			if(EcErrCodeTbl[EC_ERR_a][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_a][err_code_under2];
			}
			break;
		case 'c':
			if(EcErrCodeTbl[EC_ERR_c][err_code_under2]){
// MH810105 GG119202(S) ���σG���[�R�[�h�e�[�u��������
//				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_CRITICAL_ERR, 1, 1, 0, (void*)err_wk);
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
// MH810105(S) MH364301 PiTaPa�Ή�
//				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
				switch (err_code_under2) {
				// �d��ڍ׃G���[�R�[�h
				case 11:
				case 12:
				case 13:
				case 14:
					err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_CRITICAL_ERR, 1, 1, 0, (void*)err_wk);
					break;
				default:
					err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
					break;
				}
// MH810105(E) MH364301 PiTaPa�Ή�
// MH810105 GG119202(E) ���σG���[�R�[�h�e�[�u��������
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_c][err_code_under2];
			}
			break;
		case 'd':
			if(EcErrCodeTbl[EC_ERR_d][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_d][err_code_under2];
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
				if (brand_index == EC_WAON_USED &&
					err_code_under2 == 9) {
					// �\�����郁�b�Z�[�W��ύX����id09�j
					ec_Disp_No = 97;
				}
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
// MH810103 MH810103(s) �d�q�}�l�[�Ή� WAON���ώ��̎c���s�����b�Z�[�W���e��ύX
				else if (brand_index == EC_WAON_USED &&
					err_code_under2 == 1 ) {
					// �\�����郁�b�Z�[�W��ύX����id01�j
					ec_Disp_No = 198;
				}
// MH810103 MH810103(e) �d�q�}�l�[�Ή� WAON���ώ��̎c���s�����b�Z�[�W���e��ύX
// MH810105 GG119202(S) iD���ώ��̈ē��\���Ή�
				if (brand_index == EC_ID_USED &&
					err_code_under2 == 5) {
					// �\�����郁�b�Z�[�W��ύX����id05�j
					ec_Disp_No = 92;
				}
				if (brand_index == EC_ID_USED &&
					err_code_under2 == 3) {
					// �\�����郁�b�Z�[�W��ύX����id03�j
					ec_Disp_No = 91;
				}
// MH810105 GG119202(E) iD���ώ��̈ē��\���Ή�
			}
			break;
		case 'e':
			if(EcErrCodeTbl[EC_ERR_e][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_e][err_code_under2];
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
				if (brand_index == EC_WAON_USED &&
					(err_code_under2 == 3 || err_code_under2 == 4)) {
					// �\�����郁�b�Z�[�W��ύX����ie03, e04�j
					ec_Disp_No = 97;
				}
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
// MH810105 GG119202(S) iD���ώ��̈ē��\���Ή�
				if (brand_index == EC_ID_USED &&
					(err_code_under2 == 2 || err_code_under2 == 3)) {
					// �\�����郁�b�Z�[�W��ύX����ie02, e03�j
					ec_Disp_No = 93;
				}
// MH810105 GG119202(E) iD���ώ��̈ē��\���Ή�
			}
			break;
		case 'g':
			if(EcErrCodeTbl[EC_ERR_g][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_g][err_code_under2];
			}
			break;
		case 'h':
			if(EcErrCodeTbl[EC_ERR_h][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_ERR, 2, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_h][err_code_under2];
// MH810105 GG119202(S) iD���ώ��̈ē��\���Ή�
				if (brand_index == EC_ID_USED &&
					err_code_under2 == 3) {
					// �\�����郁�b�Z�[�W��ύX����ih03�j�����m�莞�̃G���[
					ec_Disp_No = 90;
				}
// MH810105 GG119202(E) iD���ώ��̈ē��\���Ή�
			}
			break;
		case 'i':
			if(EcErrCodeTbl[EC_ERR_i][err_code_under2]){
				err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_EMONEY_CRITICAL_ERR, 1, 1, 0, (void*)err_wk);
				registed_errcode = 1;
				ec_Disp_No = EcErrCodeTbl[EC_ERR_i][err_code_under2];
			}
			break;
// MH810103 GG119202(E) �d�q�}�l�[�n�ڍ׃G���[�R�[�h�ǉ�
		default: // ����̃G���[�����ł͖�������
			break;
	}

	//	�܂���v�����G���[���Ȃ�(�G���[�o�^�ł��Ă��Ȃ�)
	if( !registed_errcode ){
		switch( ec_err_code[0] ){
			case '1':	// 1XX
				switch ( err_code_under2 ){
					case 0:		// 100
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_100, 1, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 1;
						break;
					case 1:		// 101
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_101, 2, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 1;
						break;
					default:
						break;
				}
				break;
			case '2':	// 2XX
				switch ( err_code_under2 ){
					case 0:		// 200
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_200, 1, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 1;
						break;
					case 1:		// 201
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_201, 1, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 1;
						break;
					default:
						break;
				}
				break;
			case '3':	// 3XX
				switch ( err_code_under2 ){
					case 0:		// 300
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_300, 1, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 1;
						break;
					case 1:		// 301
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_301, 1, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 1;
						break;
					case 4:		// 304
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_304, 1, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 1;
						break;
					case 5:		// 305
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_305, 1, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 1;
						break;
					case 6:		// 306
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_306, 2, 1, 0, (void*)err_wk);
						registed_errcode = 1;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//						ec_Disp_No = 5;
						if (brand_index == EC_QR_USED) {
							ec_Disp_No = 17;	// �R�[�h�̓ǂݎ��Ɏ��s���܂���
						} else {
							ec_Disp_No = 5;		// �J�[�h�̓ǂݎ��Ɏ��s���܂����B
						}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
						break;
					case 7:		// 307
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_307, 2, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 5;
						break;
					case 8:		// 308
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_308, 2, 1, 0, (void*)err_wk);
						registed_errcode = 1;
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//						ec_Disp_No = 5;
						if (brand_index == EC_QR_USED) {
							ec_Disp_No = 17;		// �R�[�h�̓ǂݎ��Ɏ��s���܂���
						} else {
							ec_Disp_No = 5;			// �J�[�h�̓ǂݎ��Ɏ��s���܂����B
						}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
						break;
					case 9:		// 309
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_309, 2, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 3;
						break;
					case 11:	// 311
						err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_311, 2, 1, 0, (void*)err_wk);
						registed_errcode = 1;
						ec_Disp_No = 5;
						break;
					default:
						break;
				}
				break;
			default: // ����̃G���[�����ł͖�������
				break;
		}
	}

	if( !registed_errcode ){
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
		ECCTL.ec_Disp_No = 0;
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
		// ���o�^�ڍ׃G���[ �������܂ŃG���[�o�^�ł��Ȃ��A���o�^�G���[�Ƃ��ēo�^
		err_chk2((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_UNREG, 2, 1, 0, (void*)err_wk);
	}
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
	else {
		ECCTL.ec_Disp_No = ec_Disp_No;
	}
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
}

// MH810105 GG119202(S) �݂Ȃ����σv�����g�ɒ��O������e���󎚂���
////[]----------------------------------------------------------------------[]
/////	@brief			�^�C���A�E�g�G���[�`�F�b�N
////[]----------------------------------------------------------------------[]
/////	@param[in]		wk_err_code : �ڍ׃G���[�R�[�h
/////	@return			ret			: 1=�^�C���A�E�g�G���[, 0=�^�C���A�E�g�G���[�ł͂Ȃ�
/////	@author			D.Inaba
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2019/12/04<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//static uchar ec_errcode_err_chk_judg( uchar *wk_err_code )
//{
//	uchar	ret = 0;
//	uchar	i;
//
//	// EcTimeoutErrTbl�Ɏ�M�����ڍ׃G���[�����邩�ǂ�������
//	for( i = 0; i < NG_ERR_MAX; i++){
//		if( wk_err_code[0] == EcTimeoutErrTbl[i][0] &&
//			wk_err_code[1] == EcTimeoutErrTbl[i][1] &&
//			wk_err_code[2] == EcTimeoutErrTbl[i][2] ){
//			ret = 1;
//			break;
//		}
//	}
//
//	return (ret);
//}
//[]----------------------------------------------------------------------[]
///	@brief			�ڍ׃G���[�R�[�h�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		wk_err_code : �ڍ׃G���[�R�[�h
///	@param[in]		ErrTbl		: ��r�������G���[�R�[�h���i�[����Ă���e�[�u��
///	@return			ret			: 0�F�e�[�u���ɂȂ��A1�F�e�[�u���ɂ���
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/07/16<br>
///					Update	:
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static uchar ec_errcode_err_chk_judg( uchar *wk_err_code, const uchar ErrTbl[][3], uchar max )
{
	uchar	ret = 0;
	uchar	i;

	for( i = 0; i < max; i++){
		if( !memcmp( &wk_err_code[0], &ErrTbl[i][0], sizeof(uchar)*3 ) ){
			ret = 1;
			break;
		}
	}

	return (ret);
}
// MH810105 GG119202(E) �݂Ȃ����σv�����g�ɒ��O������e���󎚂���

// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//[]----------------------------------------------------------------------[]
///	@brief			�u�����h�I�����ʕ\������
//[]----------------------------------------------------------------------[]
///	@return			none
//[]----------------------------------------------------------------------[]
///	@author			A.Shindoh
///	@date			Create	:	2020/09/01<br>
///					Update
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void ec_BrandResultDisp(ushort brand_no, ushort reason_code)
{
	ushort	num = 0;										// "    ���̃J�[�h�͎g���܂���    "

	// �G���[���b�Z�[�W�̕\��
	switch(reason_code) {
	case EC_BRAND_REASON_0:									// �L�����Z���{�^���������ꂽ�B
	case EC_BRAND_REASON_1:									// �^�C���A�E�g�����B
	case EC_BRAND_REASON_99:								// �u�����h�I���f�[�^�̏������ł��Ȃ��B
		return;												// �\�����A�i�E���X�Ȃ�

	case EC_BRAND_REASON_201:								// �J�[�h�̖₢���킹���ł��Ȃ��B
	case EC_BRAND_REASON_202:								// �J�[�h�̖₢���킹���ʂ���M�ł��Ȃ��B
		num = 1;											// "    �����A����舵�����o���܂���    "
		break;
	case EC_BRAND_REASON_203:								// ���f�[�^���܂܂��J�[�h��ǂݎ�肵���B
		num = 8;											// "�ēx�J�[�h���X���C�h���ĉ�����"
		break;
	case EC_BRAND_REASON_204:								// �f�[�^���j�����Ă���J�[�h��ǂݎ�肵���B
	case EC_BRAND_REASON_205:								// �Ӑ}���Ȃ��J�[�h��ǂݎ�肵���B
		num = 5;											// "�J�[�h�̓ǂݎ��Ɏ��s���܂���"
		break;
	case EC_BRAND_REASON_206:								// �L�������؂�̃J�[�h��ǂݎ�肵���B
		num = 4;											// "    ���̃J�[�h�͊����؂�ł�    "
		break;
	case EC_BRAND_REASON_207:								// �l�K�J�[�h��ǂݎ�肵���B
		if (brand_no == BRANDNO_HOUJIN) {
			num = 7;										// "���̃J�[�h�͂��戵���ł��܂���"
		} else {
			num = 2;										// "    ���̃J�[�h�͖����ł�    "
		}
		break;
	case EC_BRAND_REASON_208:								// �L���ȃJ�[�h�ł��邪�g�p�ł��Ȃ��B
		if (brand_no == BRANDNO_TCARD) {
//			num = FX_WARN_MSG(51);							// " �s�J�[�h�͂��戵���ł��܂��� "
		} else {
			num = 1;										// "    �����A����舵�����o���܂���    "
		}
		break;
	case EC_BRAND_REASON_209:								// �L���ȃJ�[�h�ł��邪�x�������x�z�𒴂��Ă���B
		// ���s�@�͐��Z���x�z�𒴂��Ă���ꍇ�́A�u�@�l�J�[�h�ł�\r\n���Z���x�z�𒴂��Ă��܂��v�ƕ\�����邪�A
		// ���σ��[�_�ł͐��Z���x�z�𒴂��Ă���ꍇ�̓u�����h�I���Ɋ܂܂�Ȃ����߁A�{�G���[���������邱�Ƃ͐���n�ɂ����đ��݂��Ȃ��B
		// �]���͎��C���[�_����J�[�h�������^�C�~���O�Ń��b�Z�[�W���������Ă��邪�A���σ��[�_�ɂ͂��̃^�C�~���O���Ȃ����ƁB
		// �����s�\���̃��[�j���O�\���@�\�ɑΉ����Ă��Ȃ����Ƃ���A���̃J�[�h�͎g���܂��� �ɍ����ւ���(���V���l���k�ς݁@2020/09/01)
		break;
	case EC_BRAND_REASON_210:								// �L���ȃJ�[�h�ł��邪�c�����s�����Ă���B
		num = 6;											// �c���s���ł�
		break;
	case EC_BRAND_REASON_211:								// ��t�ς݂̃J�[�h��ǂݎ�肵���B
		if (brand_no == BRANDNO_TCARD) {
//			num = FX_WARN_MSG(53);							// "    �s�J�[�h�͎�t�ς݂ł�    "
		}
		break;
	case EC_BRAND_REASON_212:								// �ǂݎ����x���𒴂��ăJ�[�h��ǂݎ�肵���B
		num = 9;											// "    ���x�����𒴂��Ă��܂�    "
		break;
//	case EC_BRAND_REASON_299:								// ���̑��̃G���[
	default:
		break;
	}
	ec_WarningMessage(num);
}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j

//[]----------------------------------------------------------------------[]
///	@brief			ec_WarningMessage
//[]----------------------------------------------------------------------[]
///	@param			uchar num
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			G.So
///	@date			Create	:	2019/10/17<br>
///	@file			ope_ec_ctrl.c
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//static void ec_WarningMessage( uchar num )
void ec_WarningMessage( ushort num )
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
{

	short	avm_no = 0;
	const uchar	*massage = NULL;
// MH810100(S) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
	ulong	add_info = 0;
// MH810100(E) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)

	switch( num ){
		case 1:
			massage = ERR_CHR[44];			// "    �����A����舵�����o���܂���    "
			avm_no = AVM_CARD_ERR5;			// �ųݽ�u�����A����舵�����o���܂���v
			break;
		case 2:
			massage = ERR_CHR[3];			// "    ���̃J�[�h�͖����ł�    "
			avm_no = AVM_CARD_ERR3;			// �ųݽ�u���̃J�[�h�͖����ł��v
			break;
		case 0:
		case 3:
			massage = ERR_CHR[1];			// "    ���̃J�[�h�͎g���܂���    "
			avm_no = AVM_CARD_ERR1;			// �ųݽ�u���̃J�[�h�͎g���܂���v
			break;
		case 4:
			massage = ERR_CHR[43];			// "    ���̃J�[�h�͊����؂�ł�    "
			avm_no = AVM_KIGEN_END;			// �ųݽ�u���̃J�[�h�͊����؂�ł��v
			break;
		case 5:
			massage = ERR_CHR[54];			// "    �J�[�h�̓ǂݎ��Ɏ��s���܂���    "
			avm_no = AVM_CARD_ERR6;			// �ųݽ�u�J�[�h�̓ǂݎ��Ɏ��s���܂����v
			break;
		case 6:
			massage = ERR_CHR[1];			// "    ���̃J�[�h�͎g���܂���    "
											// "  �c���s���i�c���O�O�O�O�O�~�j"
// MH810103 GG119202(S) �ē������̓d�q�}�l�[�Ή�
//			avm_no = AVM_CARD_ERR7;			// �ųݽ�u�J�[�h�̎c��������܂���v
			avm_no = AVM_CARD_ERR7;			// �ųݽ�u�c���s���ł��v
// MH810103 GG119202(E) �ē������̓d�q�}�l�[�Ή�
			break;
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή��Q�l�j
		case 7:
			massage = ERR_CHR[41];			// "���̃J�[�h�͂��戵���ł��܂���"
			avm_no = AVM_CARD_ERR4;			// �ųݽ �u���̃J�[�h�͂���舵���ł��܂���v
			break;
		case 8:
			massage = ERR_CHR[55];			// "�ēx�J�[�h���X���C�h���ĉ�����"
			avm_no = AVM_CARD_RETRY;		// �ųݽ �u�ēx���̕����ɓ���ĉ������v
			break;
		case 9:
			massage = ERR_CHR[8];			// "    ���x�����𒴂��Ă��܂�    "
			avm_no = AVM_CARD_ERR1;			// �ųݽ �u���̃J�[�h�͎g���܂���v
			break;
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή��Q�l�j
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		case 10:
			massage = ERR_CHR[58];			// "���̎x�����@�ł��x������������"
			break;
		case 11:
			massage = ERR_CHR[70];			// "���̃R�[�h�͊����؂�ł�"
			break;
		case 12:
			massage = ERR_CHR[71];			// "���̃R�[�h�͎g���܂���"
			break;
		case 13:
			massage = ERR_CHR[71];			// "���̃R�[�h�͎g���܂���"
			break;
		case 14:
			massage = ERR_CHR[72];			// "���̃R�[�h�͎c���s���ł�"
			break;
		case 15:
			massage = OPE_CHR_CRE[2];		// "���Z���x�z�𒴂��Ă��܂�"
			break;
		case 16:
			massage = ERR_CHR[59];			// "������x��蒼���Ă�������"
			break;
		case 17:
			massage = ERR_CHR[73];			// "�R�[�h�̓ǂݎ��Ɏ��s���܂���"
			break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105 GG119202(S) iD�̓ǂݎ��҂��^�C���A�E�g�ňē��\��
		case 89:							// �ǂݎ��҂��^�C���A�E�g�iiD�j
			massage = ERR_CHR[59];			// "  ������x��蒼���Ă�������  "
			// �ē������Ȃ�
			break;
// MH810105 GG119202(E) iD�̓ǂݎ��҂��^�C���A�E�g�ňē��\��
// MH810105 GG119202(S) iD���ώ��̈ē��\���Ή�
		case 91:							// iD:d05
			massage = ERR_CHR[59];			// "  ������x��蒼���Ă�������  "
			avm_no = AVM_CARD_ERR1;			// �ųݽ �u���̃J�[�h�͎g���܂���v
			break;
		case 92:							// iD:d03
			massage = ERR_CHR[58];			// " ���̎x�����@�������p�������� "
			avm_no = AVM_CARD_ERR6;			// �ųݽ�u�J�[�h�̓ǂݎ��Ɏ��s���܂����v
			break;
		case 93:							// iD:e02,e03
			massage = ERR_CHR[60];			// " �J�[�h���s���ɂ��A���������� "
			avm_no = AVM_CARD_ERR1;			// �ųݽ �u���̃J�[�h�͎g���܂���v
			break;
		case 94:							// iD:e05
			massage = ERR_CHR[58];			// " ���̎x�����@�������p�������� "
			avm_no = AVM_CARD_ERR6;			// �ųݽ�u�J�[�h�̓ǂݎ��Ɏ��s���܂����v
			break;
// MH810105 GG119202(E) iD���ώ��̈ē��\���Ή�
// MH810105 MH321800(S) WAON���ώ��̎c���s�����b�Z�[�W���e��ύX
		case 95:
			massage = ERR_CHR[57];			// " �c���s��  ���̎x�����@�֕ύX "
			avm_no = AVM_CARD_ERR7;			// �ųݽ�u�c���s���ł��v
			break;
// MH810105 MH321800(E) WAON���ώ��̎c���s�����b�Z�[�W���e��ύX
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
		case 97:
			massage = ERR_CHR[57];			// "���̃J�[�h�͂��戵���ł��܂���"
			avm_no = AVM_CARD_ERR1;			// �ųݽ�u���̃J�[�h�͎g���܂���v
			break;
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
// MH810103 MH810103(s) �d�q�}�l�[�Ή� WAON���ώ��̎c���s�����b�Z�[�W���e��ύX
		case 198:
			massage = ERR_CHR[1];			// "    ���̃J�[�h�͎g���܂���    "
											// "  �c���s���i�c���O�O�O�O�O�~�j"
			avm_no = AVM_CARD_ERR7;			// �ųݽ�u�c���s���ł��v
			break;
// MH810103 MH810103(e) �d�q�}�l�[�Ή� WAON���ώ��̎c���s�����b�Z�[�W���e��ύX
		case 99:
		default:
			// �\���������ē��������Ȃ�
			break;
	}

	if( massage != NULL ){
// MH810103 MH810103(s) �d�q�}�l�[�Ή� WAON���ώ��̎c���s�����b�Z�[�W���e��ύX
//		if( num == 6 ){
		if( num == 6 || num == 198){
// MH810103 MH810103(e) �d�q�}�l�[�Ή� WAON���ώ��̎c���s�����b�Z�[�W���e��ύX
			// �c���s���\��
// MH810100(S) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
//			dsp_fusoku = 1;
//			op_SuicaFusoku_Msg((ulong)Ec_Settlement_Res.settlement_data_before, suica_dsp_buff, 1);
//			lcd_wmsg_dsp_elec(1, suica_dsp_buff, massage, 1, 1, COLOR_RED, LCD_BLINK_OFF);
			add_info = (ulong)Ec_Settlement_Res.settlement_data_before;
// MH810100(E) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
		} else {
			// 6sec�\��
// MH810100(S) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
//			Ope2_WarningDisp( 6*2, massage );
// MH810100(E) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
		}
// MH810103 MH810103(s) �d�q�}�l�[�Ή� �G���[�|�b�v�A�b�v�ύX
//// MH810100(S) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
//		lcdbm_notice_dsp2( POP_INQUIRY_NG, num, add_info );
//// MH810100(E) S.Fujii 2020/07/20 �Ԕԃ`�P�b�g���X(�d�q���ϒ[���Ή�)
		if( Ec_Settlement_Res.brand_no == BRANDNO_CREDIT){
			lcdbm_notice_dsp2( POP_INQUIRY_NG, num, add_info );
		}else{
			lcdbm_notice_dsp2( POP_EMONEY_INQUIRY_NG, num, add_info );
		}
// MH810103 MH810103(e) �d�q�}�l�[�Ή� �G���[�|�b�v�A�b�v�ύX
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//		if( avm_no != 0 ){
//			ope_anm(avm_no);
//		}
		if (RecvBrandResTbl.no == BRANDNO_QR) {
			// QR���ς̏ꍇ�̓u�U�[���iNG�j��炷
			BUZPIPI();
		}
		else{
			if( avm_no != 0 ){
				ope_anm(avm_no);
			}
		}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//		ec_MessagePtnNum = 98;
		ECCTL.ec_MessagePtnNum = 98;
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�u�����h�G���[�o�^
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_ctrl:	�u�����h���
///	@param[in]		err_type:	1:�o�^ 0�F����
///	@author			D.Inaba
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/11/01<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void ecRegistBrandError( EC_BRAND_CTRL *brand_ctrl, uchar err_type )
{
	char	err_no = 0;
	int		i;
	long	brand_no;

// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	if (brand_ctrl->no >= BRANDNO_HOUSE_S &&
		brand_ctrl->no <= BRANDNO_HOUSE_E) {
		err_no = ERR_EC_HOUSE_ABNORAML;
	}
	else {
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	for(i = 0; i < TBL_CNT(EcBrandEnabledParamTbl); i++){
		if ( brand_ctrl->no == EcBrandEnabledParamTbl[i].brand_no ) {
			err_no = EcBrandEnabledParamTbl[i].brand_err_no;
		}
	}
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j

	// ��v����G���[�������� = �G���[�o�^(����)
	if( err_no ){
		memset(err_wk, 0, sizeof(err_wk));
		memcpy(&err_wk[0], "\?\?-00", 5);
		brand_no = convert_brandno(brand_ctrl->no);
		memcpy(&err_wk[0], EcBrandKind_prn[brand_no - EC_USED], 2);

		sprintf((char*)&err_wk[3], "%02X", brand_ctrl->status);

		err_chk2((char)jvma_setup.mdl, err_no, (char)err_type, (char)1, (char)0, (void*)err_wk);	// �G���[�o�^
	}
}

//[]-----------------------------------------------------------------------[]
//| LCD�\���ƃG���[���̃A�i�E���X���s��										|
//[]-----------------------------------------------------------------------[]
//| MODULE NAME  : ec_MessageAnaOnOff 	 									|
//| PARAMETER    : OnOff(i) : 1:�\������ 0:�\����߂�						|
//|				   num(i)   : cre �����\������ݔԍ�							|
//| RETURN VALUE : void														|
//| COMMENT		 : creMessageAnaOnOff�Ɠ�����								|
//[]-----------------------------------------------------------------------[]
//| Date         : 2019-11-08												|
//| Update       :															|
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.----[]
void ec_MessageAnaOnOff( short OnOff, short num )
{
	
	// �O��̌㏈��(�\�������ɖ߂�)
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	switch( ec_MessagePtnNum ){
	switch( ECCTL.ec_MessagePtnNum ){
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
	case 1:
	case 2:
		blink_end();
		Vram_BkupRestr( 0, 6, 1 );
		Vram_BkupRestr( 0, 7, 1 );
		break;
	case 3:
		blink_end();
		if( LCDNO == 3 ){		// ���񗿋��\���̂� Line=6 restore
			Vram_BkupRestr( 0, 6, 1 );
		}
		Vram_BkupRestr( 0, 7, 1 );
		break;
	case 4:
		blink_end();
		Vram_BkupRestr( 0, 7, 1 );
		break;
// MH321800(S) �������̕\���������Ȃ��s��C��
	case 98:
		// LAG500_ERROR_DISP_DELAY��ϰ�����ec_MessageAnaOnOff���Ă΂ꂽ�ꍇ
		// �����ŕ\�������ɖ߂�
		Ope2_WarningDispEnd();
		break;
// MH321800(E) �������̕\���������Ȃ��s��C��
	default:
		break;
	}

// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	ec_MessagePtnNum = 0;
	ECCTL.ec_MessagePtnNum = 0;
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������

	// ����̏���(�V���ɕ\��������)
	if( OnOff == 1 ){
		// ��ʕ\��
		switch( num ){
		case 1:		// �N���W�b�g�J�[�h������
			Vram_BkupRestr( 1, 6, 0 );	//Line=6 Backup
			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
			grachr( 6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[158] );	// ���] "�N���W�b�g�J�[�h�������ł�"
			blink_reg( 7, 0, 30, 0, COLOR_DARKSLATEBLUE, OPE_CHR[7] );			// �_�� "���΂炭���҂�������"
			break;
		case 2:		// �J�[�h������
			Vram_BkupRestr( 1, 6, 0 );	//Line=6 Backup
			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
// MH810105 GG119202(S) iD���ώ��̈ē��\���Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//			if (RecvBrandResTbl.no == BRANDNO_ID) {
			if (RecvBrandResTbl.no == BRANDNO_ID ||
				RecvBrandResTbl.no == BRANDNO_QR) {
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
				grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );	// ��s
				blink_reg( 7, 0, 30, 0, COLOR_DARKSLATEBLUE, OPE_CHR_CRE[14] );	// �_�� "���σ��[�_�̉�ʂɏ]���ĉ�����"
			}
			else {
// MH810105 GG119202(E) iD���ώ��̈ē��\���Ή�
			grachr( 6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[159] );	// ���] "�J�[�h�������ł�"
			blink_reg( 7, 0, 30, 0, COLOR_DARKSLATEBLUE, OPE_CHR[7] );			// �_�� "���΂炭���҂�������"
// MH810105 GG119202(S) iD���ώ��̈ē��\���Ή�
			}
// MH810105 GG119202(E) iD���ώ��̈ē��\���Ή�
			break;
		case 3:		// �J�[�h�������҂��ē�(6�s�ځA7�s��)
			if( LCDNO == 3 ){													// ���񗿋��\�� Line=6 Backup ���āA�󔒂ŏ㏑��
				Vram_BkupRestr( 1, 6, 0 );
				grachr( 6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR[0] );	// "�@�@�@�@�@�@�@�@�@�@"
			}
			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
			blink_reg( 7, 0, 30, 0, COLOR_RED, OPE_CHR[160] );					// �_��"�@�@�J�[�h�𔲂��Ă��������@�@"
			break;
		case 4:		// ���ό���NG��M���̶��ޔ������҂��ē�(7�s�ڂ݂̂�ύX����)
			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
			blink_reg( 7, 0, 30, 0, COLOR_RED, OPE_CHR[160] );					// �_��"�@�@�J�[�h�𔲂��Ă��������@�@"
			break;
		default:
			break;
		}

		// �����ē�
		switch( num ){
		case 1:
		case 2:
			// �N���W�b�g�J�[�h(�J�[�h)�������͉����ųݽ�����Ȃ�
			break;
		case 3:
		case 4:
// MH810103 GG119202(S) �ē������̓d�q�}�l�[�Ή�
//			ope_anm( AVM_ICCARD_ERR1 );			// �J�[�h������肭������
			ope_anm( AVM_ICCARD_ERR1 );			// �J�[�h�𔲂��Ă�������
// MH810103 GG119202(E) �ē������̓d�q�}�l�[�Ή�
			break;
		default:
			break;
		}

// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//		ec_MessagePtnNum = num;					// �\�����̕\���p�^�[����ێ�
		ECCTL.ec_MessagePtnNum = num;					// �\�����̕\���p�^�[����ێ�
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
	}
	// ec_MessageAnaOnOff( 0, 0 )���Ă΂ꂽ��
	// ��L�A�O��̌㏈��(�\�������ɖ߂�)�ł͕\����߂�����Ȃ��ꍇ������ׁA
	// �����ōēx8�s�ڂ̕\��������
	else {
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		if (! isEC_USE()) {
			return;
		}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810105(S) MH364301 �C���{�C�X�Ή�
//		if( num == 0 ){
		if( num == 0 && OPECTL.f_RctErrDisp != 1 ){
// MH810105(E) MH364301 �C���{�C�X�Ή�
			if( LCDNO == 4 || LCDNO == 5 ){
				Lcd_Receipt_disp();
			}else{
				dsp_intime( 7, OPECTL.Pr_LokNo );			// ���Ɏ����\��
			}
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���Z�J�n���̌��σ��[�_����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/11/25<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void Ope_EcPayStart(void)
{
	if (!isEC_USE()) {
		return;
	}

// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	if (Suica_Rec.Data.BIT.BRAND_SEL != 0) {
		return;		// �u�����h�I�𒆂͑҂i�O�񕪂̃u�����h�I�������A�㏈�����������j
	}
	EcBrandClear();
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j

	if (!Suica_Rec.Data.BIT.CTRL) {								// ��t�s���
		if (!Suica_Rec.Data.BIT.OPE_CTRL) {
			EcSendCtrlEnableData();
		}
	}
	else {														// ��t���
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//// GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
////		if (ENABLE_MULTISETTLE() == 0) {						// �������ϖ���
//		if( ENABLE_MULTISETTLE() == 0 ){						// ���ω\�u�����h�Ȃ�
//			Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );					// �����ް��i��t�֎~�j�𑗐M����
//		}
//		else if( ENABLE_MULTISETTLE() == 1 ){					// �P�ꌈ�ϗL��
//// GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
//			Ec_Pri_Data_Snd( S_SELECT_DATA, 0 );				// ���ԗ�����I�����i�ް��Ƃ��đ��M����
//		}
//		else {													// �������ϗL��
//// GG119202(S) �}���`�u�����h�d�l�ύX�Ή�
//			// �}���`�u�����h���͕K���u�����h�I���f�[�^�𑗐M����
//			// �x�����I����ʂ�\��������
//// GG119202(E) �}���`�u�����h�d�l�ύX�Ή�
//			Ec_Pri_Data_Snd( S_BRAND_SEL_DATA, 0 );				// �u�����h�I���f�[�^���ɑ��M����
//		}
		if (!Suica_Rec.Data.BIT.OPE_CTRL) {
			return;		// ��t�֎~�������Ȃ̂ŁA��Ԓʒm��҂�
		}
		switch( check_enable_multisettle() ){
		case	0:
			Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );					// �����ް��i��t�֎~�j�𑗐M����
			break;
		case	1:		// �V���O��
			if( ENABLE_MULTISETTLE() == 0 ){					// ���A���ω\�u�����h�͂Ȃ�
				Ec_Pri_Data_Snd( S_CNTL_DATA, 0 );				// �����ް��i��t�֎~�j�𑗐M����
			}
			else {
				ec_set_brand_selected();
				Ec_Pri_Data_Snd( S_SELECT_DATA, 0 );			// ���ԗ�����I�����i�ް��Ƃ��đ��M����
			}
			break;
		default:		// �}���`
			Ec_Pri_Data_Snd( S_BRAND_SEL_DATA, 0 );				// �u�����h�I���f�[�^���ɑ��M����
			break;
		}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�N�������҂��^�C���A�E�g����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/12/01<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void ec_wakeup_timeout(void)
{
	// E3210����
	err_chk( ERRMDL_EC, ERR_EC_UNINITIALIZED, 1, 0, 0 );
}

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
////[]----------------------------------------------------------------------[]
/////	@brief			�����^�C���A�E�g����
////[]----------------------------------------------------------------------[]
/////	@param[in]		*pRes	�F���ό��ʃf�[�^
/////	@return			ret		�Fnone
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2019/11/13<br>
/////					Update
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//void ec_miryo_timeout_after_disp(EC_SETTLEMENT_RES *pRes)
//{
//	char	err = 0;
//	uchar	brand_index;
//// MH810103 GG119202(S) ������������L�^�̃J�[�h�ԍ��̓}�X�N���Ĉ󎚂���
//	ushort	i, max;
//// MH810103 GG119202(E) ������������L�^�̃J�[�h�ԍ��̓}�X�N���Ĉ󎚂���
//// MH810105 GG119202(S) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
//	Ope2_ErrChrCyclicDispStop();							// �T�C�N���b�N�\����~
//// MH810105 GG119202(E) �����ă^�b�`�҂����b�Z�[�W�\���Ή�
//
//	// �����c���Ɖ�^�C���A�E�g���̓�����ȉ���3�P�[�X�ɕ��򂷂�
//	// ��WAON:�Ƃ肯���{�^���Ő��Z�I��
//	// WAON  :WAON�����Ƃ��Ďc�z��S�Ċ������ďo�ɂ�����
//	//       :�x��
//	brand_index = convert_brandno(pRes->brand_no);
//	if (brand_index == EC_WAON_USED) {
//// MH810103 GG118809_GG118909(S) WAON�̖����c���Ɖ�Ŗ�����������m�F�����ꍇ�͂Ƃ肯���{�^�������҂��Ƃ���
//// GG118808_GG118908(S) ���������Ɠ����ɃJ�[�h�^�b�`�����������Ń��b�N����
////		if (ECCTL.ec_Disp_No == 96) {
//		if (ECCTL.ec_Disp_No == 96 ||
//			ryo_buf.zankin == 0) {
//// MH810103 GG118808_GG118908(E) ���������Ɠ����ɃJ�[�h�^�b�`�����������Ń��b�N����
//			// ����������m�F
//			miryo_timeout_after_disp();						// �����^�C���A�E�g��̕\�����s��
//			err = 1;
//			// ���̌�A�Ƃ肯���{�^�������҂��Ƃ���
//// MH810103(s) �d�q�}�l�[�Ή� �����ʒm
//			lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x06);	// 6=����(�Đ��Z�ē�)
//// MH810103(e) �d�q�}�l�[�Ή� �����ʒm
//		}
//		else
//// MH810103 GG118809_GG118909(E) WAON�̖����c���Ɖ�Ŗ�����������m�F�����ꍇ�͂Ƃ肯���{�^�������҂��Ƃ���
//	// WAON�̎�
//		if (prm_get(COM_PRM, S_ECR, 11, 1, 6) == 2) {
//		// �����^�C���A�E�g���͋x�ƂƂ���
//			miryo_timeout_after_disp();						// �����^�C���A�E�g��̕\�����s��
//			err = 2;										// E3267��ʒm
//			// ���̌�A�Ƃ肯���{�^�������������͎����L�����Z���Ő��Z�@�͋x�ƂƂ���
//// MH810103(s) �d�q�}�l�[�Ή� �����ʒm
//			lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x06);	// 6=����(�Đ��Z�ē�)
//// MH810103(e) �d�q�}�l�[�Ή� �����ʒm
//		}
//		else {
//		// �o�ɂ�����
//			al_emony_sousai();								// ���z�𑊎E�i�K�����Z�����j
//			err = (char)prm_get(COM_PRM, S_ECR, 11, 1, 5);
//		}
//	}
//	else {
//	// WAON�ȊO
//		miryo_timeout_after_disp();							// �����^�C���A�E�g��̕\�����s��
//		err = (char)prm_get(COM_PRM, S_ECR, 10, 1, 5);
//// MH810105(S) 2021/12/03 iD�Ή�
////// MH810103(s) �d�q�}�l�[�Ή� �����ʒm
////		lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x06);	// 6=����(�Đ��Z�ē�)
////// MH810103(e) �d�q�}�l�[�Ή� �����ʒm
//		if( brand_index != EC_ID_USED){
//			lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x06);	// 6=����(�Đ��Z�ē�)
//		}else{
//			lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x0c);	// 12=����(���σ^�C���A�E�g)(iD)
//		}
//// MH810105(E) 2021/12/03 iD�Ή�
//	}
//// MH810105 GG119202(S) ������������W�v�d�l���P
//	if (ECCTL.ec_Disp_No == 96 ||							// ����������m�F�i�x�����s�j
//		brand_index == EC_ID_USED ||						// iD����
//		brand_index == EC_QUIC_PAY_USED) {					// QUICPay����
//		// E3266��o�^���Ȃ�
//		err = 0;
//	}
//// MH810105 GG119202(E) ������������W�v�d�l���P
//
//	// �G���[�o�^
//	if (err != 0) {
//		memset(err_wk, 0, sizeof(err_wk));
//		memset(&err_wk[0], ' ', 31);
//
//		// �J�[�h���,��؂蕶��
//		memcpy(&err_wk[0], "\?\?:", 3);
//		memcpy(&err_wk[0], EcBrandKind_prn[brand_index - EC_USED], 2);
//		
//		// �J�[�h�ԍ�
//		if (brand_index != EC_UNKNOWN_USED && brand_index != EC_ZERO_USED) {
//			
//			if (brand_index == EC_SAPICA_USED) {
//			// SAPICA
//				memcpy(&err_wk[3], &pRes->Card_ID, ECARDID_SIZE_SAPICA);	// �J�[�h�ԍ�
//			}
//			else if (brand_index == EC_KOUTSUU_USED) {
//			// ��ʌnIC�J�[�h
//				memcpy(&err_wk[3], &pRes->Card_ID, ECARDID_SIZE_KOUTSUU);	// �J�[�h�ԍ�
//			}
//// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//			else if (brand_index == EC_QUIC_PAY_USED) {
//			// QUICPay
//				memcpy(&err_wk[3], &pRes->Card_ID, sizeof(pRes->Card_ID));	// �J�[�h�ԍ�
//			}
//// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
//			else {
//				memcpy(&err_wk[3], &pRes->Card_ID, ECARDID_SIZE_SUICA);		// �J�[�h�ԍ�
//			}
//// MH810103 GG119202(S) ������������L�^�̃J�[�h�ԍ��̓}�X�N���Ĉ󎚂���
////			if (brand_index == EC_WAON_USED) {
////			// WAON
////				memset(&err_wk[3], '*', 4);									// ���4���}�X�L���O
////			}
////			else if (brand_index == EC_NANACO_USED) {
////			// NANACO
////				memset(&err_wk[15], '*', 4);								// ����4���}�X�L���O
////			}
//			switch (brand_index) {
//			case EC_KOUTSUU_USED:
//				max = ECARDID_SIZE_KOUTSUU - 4;
//				for (i = 2; i < max; i++) {
//					err_wk[3+i] = '*';										// ���2���A����4���ȊO���}�X�N
//				}
//				break;
//// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
////			case EC_WAON_USED:
////				memset(&err_wk[3], '*', 4);									// ���4���}�X�L���O
//// MH810104(S) nanaco�EiD�EQUICPay�Ή�2
////// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
////			case EC_NANACO_USED:
////// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
//// MH810104(E) nanaco�EiD�EQUICPay�Ή�2
//			case EC_EDY_USED:
//				memset(&err_wk[3], '*', 12);								// ����4���ȊO���}�X�N
//// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
//				break;
//// MH810104(S) nanaco�EiD�EQUICPay�Ή�2
////// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//////			case EC_NANACO_USED:
//////				memset(&err_wk[15], '*', 4);								// ����4���}�X�L���O
//////				break;
////			case EC_ID_USED:
////				memset(&err_wk[16], '*', 3);								// ����3�����}�X�N
////				memset(&pRes->Card_ID[13], '*', 3);							// ����3�����}�X�N
////				break;
////			case EC_QUIC_PAY_USED:
////				memset(&err_wk[19], '*', 4);								// ����4�����}�X�N
////				memset(&pRes->Card_ID[16], '*', 4);							// ����4�����}�X�N
////				break;
////// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
//// MH810105 GG119202(S) ������������W�v�d�l���P
////			case EC_ID_USED:
////				memset(&err_wk[16], '*', 3);								// ����3�����}�X�N
////				break;
////			case EC_QUIC_PAY_USED:
////				memset(&err_wk[19], '*', 4);								// ����4�����}�X�N
////				break;
////// MH810104(E) nanaco�EiD�EQUICPay�Ή�2
//// MH810105 GG119202(E) ������������W�v�d�l���P
//			default:
//// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//			case EC_WAON_USED:
//				// WAON�̓}�X�N����Ēʒm�����
//// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
//// MH810104 GG119202(S) nanaco�EiD�EQUICPay�Ή�
//			case EC_NANACO_USED:
//				// nanaco�̓}�X�N����Ēʒm�����
//// MH810104 GG119202(E) nanaco�EiD�EQUICPay�Ή�
//				break;
//			}
//// MH810103 GG119202(E) ������������L�^�̃J�[�h�ԍ��̓}�X�N���Ĉ󎚂���
//		}
//		
//		// ����z
//		memset(suica_work_buf, 0, sizeof(suica_work_buf));
//		if (pRes->settlement_data >= 1000) {
//		// 4���ȏ�
//			sprintf((char*)suica_work_buf, "\\%ld,%03ld", pRes->settlement_data/1000, pRes->settlement_data%1000);
//		}
//		else {
//		// 3���ȉ�
//			sprintf((char*)suica_work_buf, "\\%ld", pRes->settlement_data);
//		}
//		sprintf((char*)&err_wk[23], "%8s", suica_work_buf);
//
//		// �G���[���O�o�^
//		if (err == 1) {
//		// E3266��ʒm
//			err_chk2((char)jvma_setup.mdl, ERR_EC_ALARM_TRADE, 2, 1, 0, (void*)err_wk);
//		} else {
//		// E3267��ʒm
//			err_chk2((char)jvma_setup.mdl, ERR_EC_ALARM_TRADE_WAON, 1, 1, 0, (void*)err_wk);
//		}
//	}
//}
//[]----------------------------------------------------------------------[]
///	@brief			�����c���Ɖ�^�C���A�E�g����
//[]----------------------------------------------------------------------[]
///	@param[in]		pRes	�F���ό��ʃf�[�^
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/07
///					Update
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void ec_miryo_timeout_after_disp(EC_SETTLEMENT_RES *pRes)
{
	uchar	brand_index;

	// �����c���Ɖ�^�C���A�E�g���̓���
	// WAON�F���Z�����A�x��
	// WAON�ȊO�̃v���y�C�h�^�F���Z�����A�x�ƁA���Z���~
	// �|�X�g�y�C�^�F���Z���~
	brand_index = convert_brandno(pRes->brand_no);
	if (!EcBrandPostPayCheck(brand_index)) {
		// �v���y�C�h�^
		if (ECCTL.ec_Disp_No == 96) {
			// ����������̏ꍇ
			miryo_timeout_after_disp();							// �����^�C���A�E�g��̕\�����s��
// MH810105(S) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P GT-4100
			lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x06);	// 6=����(�Đ��Z�ē�)
// MH810105(E) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P GT-4100
		}
		else {
			ReqEcAlmRctPrint(J_PRI, pRes);						// �A���[�����V�[�g�󎚗v��
			EcDeemedSettlement(&OPECTL.Ope_mod);				// ��Q�������̏���
// MH810105(S) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P GT-4100
			// EcDeemedSettlement ����LCD�֒ʒm
// MH810105(E) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P GT-4100
		}
	}
	else {
		// �|�X�g�y�C�^
		miryo_timeout_after_disp();								// �����^�C���A�E�g��̕\�����s��
// MH810105(S) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P GT-4100
		if( brand_index != EC_ID_USED){
			lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x06);	// 6=����(�Đ��Z�ē�)
		}else{
			lcdbm_notice_ope(LCDBM_OPCD_CARD_PAY_STATE, 0x0c);	// 12=����(���σ^�C���A�E�g)(iD)
		}
// MH810105(E) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P GT-4100
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			�����^�C���A�E�g�G���[�o�^
//[]----------------------------------------------------------------------[]
///	@param[in]		*pRes	�F���ό��ʃf�[�^
///	@param[in]		err		�F�G���[�ԍ�
///							  0: �G���[�ʒm�Ȃ�
///							  1: �G���[(E3266)��ʒm����
///							  2: �G���[(E3267)��ʒm����
///	@return			ret		�Fnone
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/02<br>
///					Update
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
void EcRegistMiryoTimeoutError(EC_SETTLEMENT_RES *pRes, char err)
{
	uchar	brand_index;
	ushort	size;

	brand_index = convert_brandno(pRes->brand_no);							// �u�����h�擾

	// �G���[�o�^
	if (err != 0) {
		memset(err_wk, 0, sizeof(err_wk));
		memset(&err_wk[0], ' ', 31);

		// �J�[�h���,��؂蕶��
		memcpy(&err_wk[0], "\?\?:", 3);
		memcpy(&err_wk[0], EcBrandKind_prn[brand_index - EC_USED], 2);

		// �J�[�h�ԍ�
		if (brand_index != EC_UNKNOWN_USED && brand_index != EC_ZERO_USED) {

			switch (brand_index) {
			case EC_KOUTSUU_USED:
				// 17��
				size = ECARDID_SIZE_KOUTSUU;
				break;
			default:
				// 16��
				size = ECARDID_SIZE_SUICA;
				break;
			}
			memcpy(&err_wk[3], &pRes->Card_ID, (size_t)size);

			// �}�X�N����
			switch (brand_index) {
			case EC_KOUTSUU_USED:
				memset(&err_wk[5], '*', 11);								// ���2���A����4���ȊO���}�X�N
				break;
			case EC_EDY_USED:
				memset(&err_wk[3], '*', 12);								// ����4���ȊO���}�X�N
				break;
			default:
			case EC_WAON_USED:
				// WAON�̓}�X�N����Ēʒm�����
			case EC_NANACO_USED:
				// nanaco�̓}�X�N����Ēʒm�����
				break;
			}
		}

		// ����z
		memset(suica_work_buf, 0, sizeof(suica_work_buf));
		if (pRes->settlement_data >= 1000) {
		// 4���ȏ�
			sprintf((char*)suica_work_buf, "\\%ld,%03ld", pRes->settlement_data/1000, pRes->settlement_data%1000);
		}
		else {
		// 3���ȉ�
			sprintf((char*)suica_work_buf, "\\%ld", pRes->settlement_data);
		}
		sprintf((char*)&err_wk[23], "%8s", suica_work_buf);

		// �G���[���O�o�^
		if (err == 1) {
		// E3266��ʒm
			err_chk2((char)jvma_setup.mdl, ERR_EC_ALARM_TRADE, 2, 1, 0, (void*)err_wk);
		} else {
		// E3267��ʒm
			err_chk2((char)jvma_setup.mdl, ERR_EC_ALARM_TRADE_WAON, 1, 1, 0, (void*)err_wk);
		}
	}
}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
//[]----------------------------------------------------------------------[]
///	@brief			���ό���NG��M���̶��ޔ������ųݽ�N��
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/12/05<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void ec_start_removal_wait(void)
{
	ushort	ec_removal_wait_time = (ushort)prm_get(COM_PRM, S_ECR, 13, 3, 1);

	if(ec_removal_wait_time == 0) {
		ec_removal_wait_time = 360;
	} else if( ec_removal_wait_time < 30 ){
		ec_removal_wait_time = 30;
	}
	ec_removal_wait_time *= 50;

// MH810103 GG119202(S) �����������҂����b�Z�[�W��������
	if (OPECTL.op_faz == 3 || OPECTL.op_faz == 9) {
		return;
	}
// MH810103 GG119202(E) �����������҂����b�Z�[�W��������

	// ���ލ������H
	if( Suica_Rec.Data.BIT.CTRL_CARD ){
		Lagtim( OPETCBNO, TIMERNO_EC_REMOVAL_WAIT, ec_removal_wait_time );			// ���������ϰ�N��
		ec_MessageAnaOnOff( 1, 4 );													// �u�J�[�h������艺�����v�\���ƃA�i�E���X������
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			���σ��[�_�֘A�t���O�N���A
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/12/27<br>
///					Update
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void ec_flag_clear(uchar init)
{
// MH810103 GG119202(S) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
	ushort	i, j;
// MH810103 GG119202(E) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���

	if (!isEC_USE()) {
		return;
	}

	if (init) {
	// �N����������
		Suica_Rec.Data.BIT.INITIALIZE = 0;						// Init Suica Reader
// MH810103 GG119202(S) �N���V�[�P���X��������������
//		Suica_Rec.Data.BIT.BRAND_STS_RCV = 0;					// Brand Status Receive
// MH810103 GG119202(E) �N���V�[�P���X��������������
		Suica_Rec.Data.BIT.CTRL_CARD = 0;						// �J�[�h�Ȃ�
// MH810103 GG119202(S) JVMA���Z�b�g�����ύX
		Suica_Rec.suica_err_event.BYTE &= (uchar)(~EC_NOT_AVAILABLE_ERR);	// ���σ��[�_�؂藣���G���[���N���A����
// MH810103 GG119202(E) JVMA���Z�b�g�����ύX
		Suica_Rec.suica_err_event.BIT.BRAND_NONE = 1;			// �L���u�����h�Ȃ��ŏ�����

		memset(&ECCTL, 0, sizeof(ECCTL));
		memset(&BrandTbl[0], 0, sizeof(BrandTbl));
// MH810103 GG119202(S) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
//		memset(&RecvBrandTbl[0], 0, sizeof(RecvBrandTbl));
		for(i = 0; i < TBL_CNT(RecvBrandTbl); i++) {
			for(j = 0; j < RecvBrandTbl[i].num; j++) {
				RecvBrandTbl[i].ctrl[j].status = 0x0A;			// �ǁ^�����^�T�[�r�X��~��
			}
		}
// MH810103 GG119202(E) �u�����h�e�[�u�����o�b�N�A�b�v�Ώۂɂ���
		memset(&RecvMntTbl, 0, sizeof(RecvMntTbl));
		memset(&MntTbl, 0, sizeof(MntTbl));
		memset(&BrandSelTbl, 0, sizeof(BrandSelTbl));

		Status_Retry_Count_OK = 0;								// ��t���̐����ް����M����
		Status_Retry_Count_NG = 0;								// ��t�s���̐����ް����M����

		ECCTL.phase = EC_PHASE_INIT;							// ��������
// MH810103 GG119202(S) �u�����h�I����̃L�����Z�������ύX
//		Suica_Rec.Data.BIT.SLCT_BRND_BACK = 0;					// �u�����h�I���́u�߂�v���o�t���O������
// MH810103 GG119202(E) �u�����h�I����̃L�����Z�������ύX
		Suica_Rec.Data.BIT.SETTLMNT_STS_RCV = 0;				// ���Ϗ�������M�t���O������
		Suica_Rec.Data.BIT.MIRYO_CONFIRM = 0;					// �����m��t���O������
// MH810103 GG119202(S) �u�����h�I�����ʏ���������
//		memset(&RecvBrandResTbl, 0, sizeof(RecvBrandResTbl));
		EcBrandClear();
// MH810103 GG119202(E) �u�����h�I�����ʏ���������
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//		Ec_Settlement_Sts = EC_SETT_STS_NONE;					/* ���Ϗ�Ԃ�OFF�ɂ��Ă��� */
		ECCTL.Ec_Settlement_Sts = EC_SETT_STS_NONE;				/* ���Ϗ�Ԃ�OFF�ɂ��Ă��� */
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
// MH810103 GG119202(S) �N���V�[�P���X��������������
		memset(&Suica_Rec.ec_negoc_data, 0, sizeof(Suica_Rec.ec_negoc_data));
																// �u�����h�l�S�V�G�[�V�����Ǘ����N���A
// MH810103 GG119202(E) �N���V�[�P���X��������������
// MH810103 GG119202(S) �u�����h�I�����ʉ����f�[�^
		memset(&BrandResRsltTbl, 0, sizeof(BrandResRsltTbl));
// MH810103 GG119202(E) �u�����h�I�����ʉ����f�[�^
	}
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	else {
	// �V�������Z���n�߂�O�ɁA�݂Ȃ��������Z�b�g����
		EcDeemedLog.EcDeemedBrandNo = BRANDNO_UNKNOWN;
// MH810103 GG119202(S) �u�����h�I�𒆃t���O���N���A����Ȃ��΍�
		Suica_Rec.Data.BIT.SELECT_SND = 0;							// �I�����i�f�[�^���M�� ����
		Suica_Rec.Data.BIT.BRAND_SEL = 0;							// �u�����h��I��
// MH810103 GG119202(E) �u�����h�I�𒆃t���O���N���A����Ȃ��΍�
// MH810103 GG119202(S) �d�q�}�l�[�V���O���ݒ�ňē��������s��
		ECCTL.anm_ec_emoney = 0;
// MH810103 GG119202(E) �d�q�}�l�[�V���O���ݒ�ňē��������s��
// MH810103 GG118808_GG118908(S) �d�q�}�l�[�V���O���ݒ�̓ǂݎ��҂��^�C���A�E�g�����ύX
		ECCTL.ec_single_timeout = 0;
// MH810103 GG118808_GG118908(E) �d�q�}�l�[�V���O���ݒ�̓ǂݎ��҂��^�C���A�E�g�����ύX
	}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j

	OPECTL.holdPayoutForEcStop = 0;								// ���o�C�x���g�ۗ��t���O������
	memset(&Ec_Settlement_Res, 0, sizeof(Ec_Settlement_Res));	// ���ό��ʃf�[�^������

// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	ec_MessagePtnNum = 0;
//	Product_Select_Brand = 0;
	ECCTL.ec_MessagePtnNum = 0;
	ECCTL.Product_Select_Brand = 0;
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	ECCTL.brandsel_stop = 0;
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	ECCTL.Ec_FailureContact_Sts = 0;							// 0:��Q�A���[���s�s��
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
}

//[]----------------------------------------------------------------------[]
///	@brief			����f�[�^(��t����)���M
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/01/08<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void EcSendCtrlEnableData(void)
{
	ushort	time = OPE_EC_RECEPT_SEND_TIME;

// MH810103 GG119202(S) ���R�R�[�h�X�X��M���͎�t���𑗐M���Ȃ�
	if (ECCTL.brandsel_stop != 0) {
		// �u�����h������~���͎�t���𑗐M���Ȃ�
		return;
	}
// MH810103 GG119202(E) ���R�R�[�h�X�X��M���͎�t���𑗐M���Ȃ�
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	if (timer_recept_send_busy != 0) {
	if (ECCTL.timer_recept_send_busy != 0) {
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
		return;			// ���M�x���^�C�}�[���s���͂Q�d�Ɋ|���Ȃ�
	}
	if (Status_Retry_Count_OK == 0) {
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//// GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
//		if(( OPECTL.Ope_mod == 2 ) &&							// ���Z������
//		   ( ENABLE_MULTISETTLE() == 0 )){						// ���ωu�����h�Ȃ�
//			return;	// ����f�[�^(��t����)�͑��M���Ȃ�
//		}
//// GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		// ����f�[�^(��t����)���M����10�b�o�߂��Ă�
		// ��ԃf�[�^(��t��)����M���Ȃ��ꍇ��A0193��o�^����B
		LagTim500ms( LAG500_SUICA_NO_RESPONSE_TIMER, (short)(EC_ENABLE_WAIT_TIME*2+1), snd_enable_timeout );
	}
	Lagtim(OPETCBNO, TIMERNO_EC_RECEPT_SEND, time);		// ��t�����M�^�C�}�[�X�^�[�g
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	timer_recept_send_busy = 1;							// ec������M�^�C�}�[���쒆
	ECCTL.timer_recept_send_busy = 1;					// ec������M�^�C�}�[���쒆
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������

	Status_Retry_Count_OK = 1;
}
// MH810103 GG119202(S) �N���V�[�P���X��������������
//[]----------------------------------------------------------------------[]
///	@brief			�u�����h��ԃf�[�^��M������ݒ�
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	�F�u�����h��ԃf�[�^���
///	@return			ret		�Fnone
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/05/22<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void setEcBrandStsDataRecv( uchar kind )
{
	switch (kind) {					// ��M�ް�����
	case S_BRAND_SET_DATA: 			// �u�����h��ԃf�[�^�P��M
		if( ( Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS1_RCV == 0 ) &&
			( Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS2_RCV == 0 ) ) {
			Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS1_RCV = 1;	// �u�����h��ԃf�[�^�P(1���)��M�ς�
		}
		else if( ( Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS1_RCV == 1 ) &&
				 ( Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS2_RCV == 0 ) ) {
			Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS2_RCV = 1;	// �u�����h��ԃf�[�^�P(2���)��M�ς�
		}
		break;
		
	case S_BRAND_SET_DATA2: 		// �u�����h��ԃf�[�^�Q��M
		if( ( Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS3_RCV == 0 ) &&
			( Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS4_RCV == 0 ) ) {
			Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS3_RCV = 1;	// �u�����h��ԃf�[�^�Q(1���)��M�ς�
		}
		else if( ( Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS3_RCV == 1 ) &&
				 ( Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS4_RCV == 0 ) ) {
			Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS4_RCV = 1;	// �u�����h��ԃf�[�^�Q(2���)��M�ς�
		}
		break;

	default:
		break;
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			�u�����h��ԃf�[�^��M�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			TRUE:�S�Ď�M�ς�, FALSE:����M�f�[�^����
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/05/22<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
ushort isEcBrandStsDataRecv(void)
{
	ushort	ret = FALSE;

	if( ( ECCTL.brand_num <= EC_BRAND_MAX ) &&
		( ( Suica_Rec.ec_negoc_data.brand_rcv_event.ULONG & EC_BRAND10_STS_ALL_RECV ) == EC_BRAND10_STS_ALL_RECV ) ) {
																	// �u�����h����10�ȉ��A�u�����h�l�S�V�G�[�V�����P���������Ă��Ȃ�
		ret = TRUE;
	}
	else if( ( ( ECCTL.brand_num <= EC_BRAND_TOTAL_MAX ) && ( ECCTL.brand_num > EC_BRAND_MAX ) ) &&
		( ( Suica_Rec.ec_negoc_data.brand_rcv_event.ULONG & EC_BRAND20_STS_ALL_RECV ) == EC_BRAND20_STS_ALL_RECV ) ) {
																	// �u�����h����10�ȏ�A�u�����h�l�S�V�G�[�V�����P�A�Q���������Ă��Ȃ�
		ret = TRUE;
	}

	return( ret );
}
// MH810103 GG119202(E) �N���V�[�P���X��������������

// MH810103 GG119202(S) �N���V�[�P���X�s��C��
//[]----------------------------------------------------------------------[]
///	@brief			�u�����h�l�S�V�G�[�V��������
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	�F�u�����h��ԃf�[�^���
///	@return			ret		�Fnone
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/07/15<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static void setEcBrandNegoComplete(void)
{
	Suica_Rec.Data.BIT.INITIALIZE = 1;				// �������V�[�P���X����
	ECCTL.phase = EC_PHASE_PAY;						// ���Z��
	ECCTL.step = 0;

	LagCan500ms(LAG500_EC_WAIT_BOOT_TIMER);			// �N�������҂��^�C�}��~
	// E3210����
	err_chk( ERRMDL_EC, ERR_EC_UNINITIALIZED, 0, 0, 0 );
// MH810103 GG119202(S) ���σ��[�_����̍ċN���v�����L�^����
	// E3213����
	err_chk((char)jvma_setup.mdl, ERR_EC_REBOOT, 0, 0, 0);	// �G���[����
// MH810103 GG119202(E) ���σ��[�_����̍ċN���v�����L�^����
// MH810105 GG119202(S) ���σ��[�_�o�[�W�����A�b�v���̕��d�Ή�
	// ���σ��[�_�o�[�W�����A�b�v���t���O�N���A
	EcReaderVerUpFlg = 0;
// MH810105 GG119202(E) ���σ��[�_�o�[�W�����A�b�v���̕��d�Ή�

// MH810103 GG119202(S) JVMA���Z�b�g�����s�
	if ((Suica_Rec.suica_err_event.BYTE & EC_NOT_AVAILABLE_ERR) != 0) {
		// �N���V�[�P���X���Ɉُ�f�[�^��M�����ꍇ��JVMA���Z�b�g���s��
		jvma_trb(ERR_SUICA_RECEIVE);
		return;
	}
// MH810103 GG119202(E) JVMA���Z�b�g�����s�
	queset( OPETCBNO, SUICA_INITIAL_END, 0, NULL );	// OpeTask�ɏ�����������ʒm
}

//[]----------------------------------------------------------------------[]
///	@brief			�ۗ����Ă����u�����h�l�S�V�G�[�V�����ĊJ
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			ret		�Fnone
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/07/15<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void Ec_check_PendingBrandNego(void)
{
	t_TARGET_MSGGET_PRM	Target_WaitMsgID;
	MsgBuf	*msb;
	ushort	msg;
	uchar	brand_index;
	uchar	kind;

	if (!isEC_USE()) {
		return;
	}

	if (hasEC_JVMA_RESET_CNT_EXCEEDED) {
		// JVMA���Z�b�g���g���C�I�[�o�[���Ă���ꍇ�͉������Ȃ�
		return;
	}
	if (ECCTL.phase != EC_PHASE_BRAND) {
		// �u�����h���킹���t�F�[�Y�ȊO�͉������Ȃ�
		return;
	}

	if (Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS1_RCV != 0) {
		// �u�����h��ԃf�[�^��M�ς�
		brand_index = 0;
		kind = S_BRAND_SET_DATA;
		if (ECCTL.brand_num > EC_BRAND_MAX &&
			Suica_Rec.ec_negoc_data.brand_rcv_event.BIT.BR_STS3_RCV != 0) {
			brand_index = 1;
			kind = S_BRAND_SET_DATA2;
		}
		Ec_Data_Snd(kind, &BrandTbl[brand_index]);	// �u�����h�ݒ�f�[�^���M
		if (isEcBrandStsDataRecv()) {
			// �u�����h�l�S�V�G�[�V��������
			ECCTL.phase = EC_PHASE_VOL_CHG;			// ���ʕύX��
			ECCTL.step = 0;

			// ���ʕύX�f�[�^���M
			SetEcVolume();
			Ec_Data_Snd( S_VOLUME_CHG_DATA, &VolumeTbl );
		}
		else {
			// ���̃u�����h��ԃf�[�^��M�҂�
		}
	}
	else {
		// �u�����h��ԃf�[�^����M�̏ꍇ��Ope_Ec_Event()�ł̎�M������҂�
	}
	// �u�����h��ԃf�[�^��M�҂�
	Lagtim(OPETCBNO, TIMERNO_EC_BRAND_NEGO_WAIT, EC_BRAND_NEGO_WAIT_TIME);	// ��M�҂��^�C�}�J�n

	// �u�����h�l�S�V�G�[�V����������҂�
	// SUICA_EVT��EC_BRAND_NEGO_WAIT_TIMEOUT����������
	Target_WaitMsgID.Count = 2;
	Target_WaitMsgID.Command[0] = SUICA_EVT;
	Target_WaitMsgID.Command[1] = EC_BRAND_NEGO_WAIT_TIMEOUT;
	do {
		taskchg( IDLETSKNO );

		if (ECCTL.phase == EC_PHASE_INIT) {
			// JVMA���Z�b�g�����ꍇ�̓��[�v�𔲂���
			break;
		}
		if (hasEC_JVMA_RESET_CNT_EXCEEDED) {
			// JVMA���Z�b�g���g���C�I�[�o�[�����ꍇ�̓��[�v�𔲂���
			break;
		}

		msb = Target_MsgGet(OPETCBNO, &Target_WaitMsgID);		// ���҂���Ұق�����M�i���͗��߂��܂܁j
		if (msb == NULL) {
			continue;
		}

		msg = msb->msg.command;
		FreeBuf(msb);

		switch (msg) {
		case SUICA_EVT:
			// ��M����
			Ope_Ec_Event(msg, OPECTL.Ope_mod);
			break;
		case EC_BRAND_NEGO_WAIT_TIMEOUT:
			// �t�F�[�Y�����������ɖ߂��ăX�^���o�C�R�}���h�����蒼��
			ECCTL.phase = EC_PHASE_INIT;
			ECCTL.step = 0;
			break;
		default:
			break;
		}

	} while (ECCTL.phase != EC_PHASE_PAY);
}
// MH810103 GG119202(E) �N���V�[�P���X�s��C��

// MH810103 GG119202(S) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�
//[]----------------------------------------------------------------------[]
///	@brief			�P��u�����h���A�L���u�����h��I���ς݂Ƃ��鏈��
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/05/25<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static void ec_set_brand_selected( void )
{
	int i, j;

// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	EcBrandClear();
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	for( i = 0; i < TBL_CNT(RecvBrandTbl); i++ ){
		for(j = 0; j < RecvBrandTbl[i].num; j++){
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//			if( ec_check_valid_brand( RecvBrandTbl[i].ctrl[j].no ) ){	// �u�����h�L���H
			if( IsValidBrand( i, j ) ){								// �u�����h�L���H
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
				// �P��u�����h���Ȃ̂ŗL���u�����h����������I���ς݂Ƃ��Ė߂�
				RecvBrandResTbl.no = RecvBrandTbl[i].ctrl[j].no;	// �L���u�����hNo.�Z�b�g
				RecvBrandResTbl.res = EC_BRAND_SELECTED;	// �u�����h�I���ς݂Ƃ���
				return;
			}
		}
	}

	// �P��u�����h�L���̍ۂɋN�������̂ł��̃��[�g�͒ʂ�Ȃ��͂�����
	// �����L���u�����h�Ȃ��ƂȂ����ꍇ�̓u�����h���I�����Z�b�g���Ė߂�
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//	RecvBrandResTbl.no = BRANDNO_UNKNOWN;
//	RecvBrandResTbl.res = EC_BRAND_UNSELECTED;	// �u�����h���I���Ƃ���
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			�w��u�����h���ϗL��/�������菈��
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no	:���肷��u�����hNo.
///	@return			ret			:FALSE(0) = ���ϖ���/TRUE(1) = ���ϗL��
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/05/25<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static BOOL ec_check_valid_brand( ushort brand_no )
{
	int i, j;
	BOOL ret = FALSE;	// �u�����h���ϖ���

	for( i = 0; i < TBL_CNT(RecvBrandTbl); i++ ){
		for( j = 0; j < EC_BRAND_MAX; j++ ){
			if( RecvBrandTbl[i].ctrl[j].no == brand_no ){	// �u�����hNo.��v�H
				// �u�����hNo.��v�Ȃ猈�ϗL��/�������`�F�b�N���Ė߂�
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//				if(( RecvBrandTbl[i].ctrl[j].status & 0x01 ) == 0x01 ){	// �J�Ǐ�ԁH
//					switch( RecvBrandTbl[i].ctrl[j].no ){
//						case BRANDNO_KOUTSUU:
//						case BRANDNO_EDY:
//						case BRANDNO_NANACO:
//						case BRANDNO_WAON:
//						case BRANDNO_SAPICA:
//						case BRANDNO_ID:
//						case BRANDNO_QUIC_PAY:
//							if( WAR_MONEY_CHECK && !e_incnt ){	// �����Ȃ�
//								ret = TRUE;	// ���ϗL��
//							}
//							break;	// ��������Ȃ猈�ϖ���
//						case BRANDNO_CREDIT:
//							if( Ec_check_cre_pay_limit() ){	// ���Z���x�z�ȉ�
//								ret = TRUE;	// ���ϗL��
//							}
//							break;	// ���Z���x�z���߂Ȃ猈�ϖ���
//						case BRANDNO_HOUJIN:
//							ret = TRUE;		// ���ϗL��
//							break;
//						default:
//							// ����`�u�����h�͌��ϖ���
//							break;
//					}
//				}	// �Ǐ�ԂȂ猈�ϖ���
//				return ret;
				return IsValidBrand(i, j);
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
			}
		}
	}

	return ret;	// �w��u�����hNo.��v�Ȃ��Ȃ猈�ϖ���
}
// MH810103 GG119202(E) �N���W�b�g�J�[�h���Z���x�z�ݒ�Ή�

// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//[]----------------------------------------------------------------------[]
///	@brief			�u�����h�f�[�^�N���A
//[]----------------------------------------------------------------------[]
///	@return			none
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/07/02 A.Shindoh <br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void EcBrandClear(void)
{
	memset(&RecvBrandResTbl, 0, sizeof(RecvBrandResTbl));
	RecvBrandResTbl.res = EC_BRAND_UNSELECTED;			// �I���u�����h�Ȃ�
	RecvBrandResTbl.no = BRANDNO_UNKNOWN;				// �I���u�����h�Ȃ� 
}

//[]----------------------------------------------------------------------[]
///	@brief			�u�����h�I���f�[�^�̕ύX�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		in_mony_flg TRUE: ������ FALSE:�������łȂ�
///                 �������̏ꍇ�͈ꎞ�I��ryo_buf.zankin���X�V���Ă��画�肷��
///	@return			TRUE :�ύX����
///					FALSE:�ύX�Ȃ�
///  @note			�u�����h�I�𒆂łȂ��ꍇ��TRUE��Ԃ��B
///                 �u�����h�I�𒆂͑O�񑗐M�����u�����h�f�[�^�ƕύX������
//                  �ꍇ��TRUE��Ԃ��B�����łȂ��ꍇ��FALSE��Ԃ��B
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/07/07 A.Shindoh <br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
BOOL EcCheckBrandSelectTable(BOOL in_mony_flg)
{
	BOOL ret;
	EC_BRAND_SEL_TBL brand_sel_tbl;
	int i, j;
	unsigned long tmp_zankin;

	if (Suica_Rec.Data.BIT.BRAND_SEL == 0) {	// �u�����h�I�𒆂łȂ�
		ret = TRUE;
	} else  {
		tmp_zankin = ryo_buf.zankin;
		if (in_mony_flg) {	// �������͎c�����ꎞ�I�ɍX�V����
			// ��������ryo_buf.zankin�����X�V�ł���B
			// �N���W�b�g�J�[�h�A�@�l�J�[�h�̃u�����h�I���f�[�^���茋�ʂɉe�����邽��
			// �ꎞ�I�ɍX�V���Ĕ����Ɍ��ɖ߂��B
			//  (ryo_buf.zankin�͕��d�����ŎQ�Ƃ��Ă��Ȃ����߁A�ꎞ�I�ɍX�V���Ă����Ȃ�)
			ryo_buf.zankin = ryo_buf.dsp_ryo - ryo_buf.nyukin;
		}
		
		memset(&brand_sel_tbl, 0x00, sizeof(brand_sel_tbl));
		for (i = 0; i < TBL_CNT(RecvBrandTbl); i++) {
			// �u�����h�I���f�[�^�쐬
			for (j = 0; j < EC_BRAND_MAX; j++) {
				if( IsValidBrand( i, j ) ){				// �u�����h�L���H
					brand_sel_tbl.no[brand_sel_tbl.num++] = RecvBrandTbl[i].ctrl[j].no;
				}
			}
		}

		ryo_buf.zankin = tmp_zankin;

		// �O��̃u�����h�I���f�[�^�Ɣ�r����B
		if (memcmp(&brand_sel_tbl.no, &BrandSelTbl.no, sizeof(brand_sel_tbl.no)) != 0) {
			ret = TRUE;									/* �u�����h�e�[�u�����قȂ� */
		} else {
			ret = FALSE;										// �u�����h�e�[�u���������ꍇ�͋��z�ύX�𑗐M���Ȃ�
		}
	}
	return	ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�u�����h�L������
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_i : �u�����h�C���f�b�N�X
///	@param[in]		brand_j : �u�����h�C���f�b�N�X
///	@return			TRUE:�L��  FALSE:����
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/08/12 A.Shindoh <br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static BOOL IsValidBrand(int brand_i, int brand_j)
{
	BOOL ret = FALSE;
	ushort brand_no;

// MH810103 GG119202(S) �J�ǁE�L�������ύX
	if (OPECTL.Ope_mod != 2) {
		// ���Z���ȊO�͊J�Ǐ�Ԃ̂ݔ��肷��
// MH810103 GG119202(S) �T�[�r�X��~���u�����h�̓u�����h�ݒ�f�[�^�ɃZ�b�g���Ȃ�
//		if (isEC_BRAND_STS_KAIKYOKU(RecvBrandTbl[brand_i].ctrl[brand_j].status)) {
		if (isEC_BRAND_STS_ENABLE(RecvBrandTbl[brand_i].ctrl[brand_j].status)) {
// MH810103 GG119202(E) �T�[�r�X��~���u�����h�̓u�����h�ݒ�f�[�^�ɃZ�b�g���Ȃ�
			return TRUE;
		}
	}
// MH810103 GG119202(E) �J�ǁE�L�������ύX

// MH810103 GG119202(S) �u�����h��Ԕ��菈���ύX
//	if( (RecvBrandTbl[brand_i].ctrl[brand_j].status & 0x01 ) == 0x01 ){
// MH810103 GG119202(S) �T�[�r�X��~���u�����h�̓u�����h�ݒ�f�[�^�ɃZ�b�g���Ȃ�
//	if (isEC_BRAND_STS_KAIKYOKU(RecvBrandTbl[brand_i].ctrl[brand_j].status)) {
	if (isEC_BRAND_STS_ENABLE(RecvBrandTbl[brand_i].ctrl[brand_j].status)) {
// MH810103 GG119202(E) �T�[�r�X��~���u�����h�̓u�����h�ݒ�f�[�^�ɃZ�b�g���Ȃ�
// MH810103 GG119202(E) �u�����h��Ԕ��菈���ύX
		brand_no = RecvBrandTbl[brand_i].ctrl[brand_j].no;
		switch( brand_no ) {
		case BRANDNO_KOUTSUU:
		case BRANDNO_EDY:
		case BRANDNO_NANACO:
		case BRANDNO_WAON:
		case BRANDNO_SAPICA:
		case BRANDNO_ID:
		case BRANDNO_QUIC_PAY:
// MH810105(S) MH364301 PiTaPa�Ή�
		case BRANDNO_PITAPA:
// MH810105(E) MH364301 PiTaPa�Ή�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		case BRANDNO_QR:
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810103 GG119202(S) �d�q�}�l�[���ώ��̓W���[�i���v�����^�ڑ��ݒ���Q�Ƃ���
			if (Suica_Rec.Data.BIT.PRI_NG != 0) {
				// �W���[�i���v�����^�g�p�s���͓d�q�}�l�[���ϕs��
				ret = FALSE;
				break;
			}
// MH810103 GG119202(E) �d�q�}�l�[���ώ��̓W���[�i���v�����^�ڑ��ݒ���Q�Ƃ���
// MH810103 GG118807_GG118907(S) ��ʌnIC�̌��x�z��10���~�Ƃ���
			if (brand_no == BRANDNO_KOUTSUU &&
				ryo_buf.zankin >= EC_PAY_LIMIT_KOUTSUU) {
				// ����10���~�ȏ�̏ꍇ�͌�ʌnIC��
				// �u�����h�I���f�[�^�Ɋ܂߂Ȃ�
				ret = FALSE;
				break;
			}
// MH810103 GG118807_GG118907(E) ��ʌnIC�̌��x�z��10���~�Ƃ���
			if ( WAR_MONEY_CHECK && !e_incnt ) {	// �����Ȃ�
				ret = TRUE;
			} else {
				ret = FALSE;
			}
			break;
		case BRANDNO_CREDIT:
			ret = Ec_check_cre_pay_limit();		// �N���W�b�g���Z��(���x�z�ȉ�)
			break;
		case BRANDNO_TCARD:
//			if (( chk_Tcard_use() == 0 )			// T�J�[�h�g�p��
//			 && ( T_Card_Use == 0 )     ) {		// T�J�[�h���g�p
//				ret = TRUE;
//			 } else {
//			 	ret = FALSE;
//			 }	
			break;
		case BRANDNO_HOUJIN:
//			ret = ec_check_houjin_use();
			break;
		default:
			ret = FALSE;
			break;
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			�u�����h�L������(�I��v������������)
//[]----------------------------------------------------------------------[]
///	@param[in]		brandno : �u�����h�ԍ�
///	@return			TRUE:�L��  FALSE:����
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/08/26 MATSUSHITA <br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static	BOOL	IsBrandSpecified(ushort brandno)
{
	int i;

	for (i = 0; i < BrandSelTbl.num; i++) {
		if (BrandSelTbl.no[i] == brandno) {
			return TRUE;
		}
	}
	return FALSE;
}

//[]----------------------------------------------------------------------[]
///	@brief			���ϗL��u�����h����
//[]----------------------------------------------------------------------[]
///	@param[in]		brandno : �u�����h�ԍ�
///	@return			TRUE:�L��  FALSE:����
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/09/07 A.Shindoh <br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
// MH810103 GG119202(S) �n�E�X�J�[�h�ɂ��J�[�h�������^�C���A�E�g�͐��Z���~�Ƃ���
//static	BOOL	IsSettlementBrand(ushort brandno)
BOOL	IsSettlementBrand(ushort brandno)
// MH810103 GG119202(E) �n�E�X�J�[�h�ɂ��J�[�h�������^�C���A�E�g�͐��Z���~�Ƃ���
{
	return (BOOL)(brandno <= BRANDNO_for_SETTLEMENT);
}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j

// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή��Q�l�j
//[]----------------------------------------------------------------------[]
///	@brief			�J�[�h�f�[�^�ʒm�i���ςȂ��u�����h�^�㌈�σu�����h�j
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no	:�I���u�����hNo.
///	@param[in]		*dat		:�J�[�h�f�[�^
///	@return			ret			:0=����`�u�����h
///								:1=��`���ꂽ���ςȂ��u�����h�^�㌈�σu�����h
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/09/24<br>
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static uchar EcNotifyCardData(ushort brand_no, uchar *dat)
{
	uchar	ret = 0;

	switch (brand_no) {
	// ���ςȂ��u�����h�^�㌈�σu�����h�̏ꍇ��OPE�^�X�N�փC�x���g��ʒm����
	// �J�[�h�f�[�^�`�F�b�N���s���Ă�������
	// �J�[�h�f�[�^OK�̏ꍇ��Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 1)�����s���Ă�������
	// �J�[�h�f�[�^NG�̏ꍇ��Ec_Pri_Data_Snd(S_BRAND_RES_RSLT_DATA, 0)�����s���Ă�������
	case BRANDNO_TCARD:			// T�J�[�h
//		memset(&MAGred_HOUJIN[0], 0x00, sizeof(MAGred_HOUJIN));	
//		memcpy(&MAGred_HOUJIN[MAG_ID_CODE], dat, EC_CARDDATA_LEN);	// MAGred_HOUJIN 256 byte > RecvBrandResTbl.dat 128byte
//		queset( OPETCBNO, EC_BRAND_RESULT_TCARD, 0, 0);				// opetask�Ƀu�����h�I������ T�J�[�h��ʒm
//		ret = 1;
		break;
	case BRANDNO_HOUJIN:		// �@�l�J�[�h
//		memset(&MAGred_HOUJIN[0], 0x00, sizeof(MAGred_HOUJIN));	
//		memcpy(&MAGred_HOUJIN[MAG_ID_CODE], dat, EC_CARDDATA_LEN);	// MAGred_HOUJIN 256 byte > RecvBrandResTbl.dat 128byte
//		queset( OPETCBNO, EC_BRAND_RESULT_HOUJIN, 0, 0); 			// opetask�Ƀu�����h�I������ Houjin�J�[�h��ʒm
//		ret = 1;
		break;
	default:
		break;
	}
	return ret;
}
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή��Q�l�j

// MH810103 GG119202(S) JVMA���Z�b�g�����ύX
//[]----------------------------------------------------------------------[]
///	@brief			�ۗ����Ă���JVMA���Z�b�g�v��
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/08/20
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void Ec_check_PendingJvmaReset(void)
{
// MH810103 GG119202(S) JVMA���Z�b�g�����s�
//	if ((Suica_Rec.suica_err_event.BYTE & EC_NOT_AVAILABLE_ERR) == 0 &&
//		 Suica_Rec.suica_err_event.BIT.COMFAIL == 0) {
//		// �ʐM�s�ǁA�܂��́A���σ��[�_�؂藣���G���[�������͉������Ȃ�
//		return;
//	}
	short	timer;

	if (ECCTL.phase != EC_PHASE_WAIT_RESET) {
		// ���Z�b�g�҂��ȊO�͉������Ȃ�
		return;
	}
// MH810103 GG119202(E) JVMA���Z�b�g�����s�
// MH810103 GG119202(S) JVMA���Z�b�g�����ύX
//	if (Suica_Rec.Data.BIT.RESET_COUNT != 0) {
	if (Suica_Rec.Data.BIT.RESET_COUNT != 0 ||
		Suica_Rec.Data.BIT.RESET_RESERVED != 0) {
		// JVMA���Z�b�g�v�����͉������Ȃ�
// MH810103 GG119202(E) JVMA���Z�b�g�����ύX
		// JVMA���Z�b�g�ς݂ŃX�^���o�C�ɑ΂��鉞���Ȃ��͉������Ȃ�
		return;
	}
	if (hasEC_JVMA_RESET_CNT_EXCEEDED) {
		// JVMA���Z�b�g���g���C�I�[�o�[�͉������Ȃ�
		return;
	}
	else {
		// JVMA���Z�b�g���g���C�񐔃C���N�������g
		Ec_Jvma_Reset_Count++;
		if (hasEC_JVMA_RESET_CNT_EXCEEDED) {
			// JVMA���Z�b�g���g���C�I�[�o�[�͉������Ȃ�
// MH810103 GG119202(S) ���ϒ�~�G���[�̓o�^
			err_chk( (char)jvma_setup.mdl, ERR_EC_PAY_STOP, 1, 0, 0 );
// MH810103 GG119202(E) ���ϒ�~�G���[�̓o�^
			return;
		}
	}

	// JVMA���Z�b�g��v������
	Suica_Rec.Data.BIT.RESET_RESERVED = 1;
// MH810103 GG119202(S) JVMA���Z�b�g�����s�
//// GG119202(S) JVMA���Z�b�g�����ύX
//	// jvma_trb()���ړ�
//	// �������V�[�P���X�\�񒆂Ȃ̂ŏ������ς݃t���O���N���A����
//	Suica_Rec.Data.BIT.INITIALIZE = 0;
//	// �������V�[�P���X�\�񒆂Ȃ̂Ńt�F�[�Y�����������ɍX�V����
//	ECCTL.phase = EC_PHASE_INIT;
//// GG119202(E) JVMA���Z�b�g�����ύX
	// �R�C�����b�N�^�X�N�ł�JVMA���Z�b�g�҂�
	xPause(16);

	// JVMA���Z�b�g����3��(�Œ�)�o�߂��Ă����σ��[�_�̏������V�[�P���X���������Ȃ��ꍇ�A
	// E3210��o�^����
	timer = OPE_EC_WAIT_BOOT_TIME * 2;
	LagTim500ms(LAG500_EC_WAIT_BOOT_TIMER, (short)(timer+1), ec_wakeup_timeout);
// MH810103 GG119202(E) JVMA���Z�b�g�����s�
}
// MH810103 GG119202(E) JVMA���Z�b�g�����ύX

// MH810103 GG119202(S) �݂Ȃ����ψ������̓���
//[]----------------------------------------------------------------------[]
///	@brief			�݂Ȃ����ψ����̃G���[�o�^
//[]----------------------------------------------------------------------[]
///	@param[in]		kind	: 0=�݂Ȃ�����(���Z����), 1=���Ϗ������s(�x��),
///							  2=���Ϗ������s(�x��)(���d),
///							  3=���Ϗ������s(���Z���~)
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/08/20
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
// MH810103 GG119202(S) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
//void EcRegistSettlementAbort(void)
void EcRegistDeemedSettlementError(uchar kind)
// MH810103 GG119202(E) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
{
	uchar	brand_index;
// MH810103 GG119202(S) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
	uchar	reason;
// MH810103 GG119202(E) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�

	memset(err_wk, 0, sizeof(err_wk));
	memcpy(err_wk, "\?\?:", 3);
	// �J�[�h���
	brand_index = convert_brandno(EcDeemedLog.EcDeemedBrandNo);
	memcpy(&err_wk[0], EcBrandKind_prn[brand_index - EC_USED], 2);
// MH810103 MHUT40XX(S) E3247�󎚎��̗��R�R�[�h�ԈႢ���C��
	if (kind == 2) {
		// ���Z�@�d��OFF
		reason = 5;
	}
	else
// MH810103 MHUT40XX(E) E3247�󎚎��̗��R�R�[�h�ԈႢ���C��
// MH810103 GG119202(S) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
	if (Suica_Rec.suica_err_event.BIT.COMFAIL != 0) {
		// �ʐM�s��
		reason = 1;
	}
	else if (Suica_Rec.suica_err_event.BIT.SETT_RCV_FAIL != 0) {
		// �J�[�h�������^�C���A�E�g
		reason = 2;
	}
// MH810103 GG119202(S) �J�[�h��������M��̌���NG�i304�j�̈���
	else if (IsErrorOccuerd((char)jvma_setup.mdl, ERR_EC_RCV_SETT_ERR_304)) {
		// �J�[�h��������M��Ɍ���NG�i304�j��M�i�����m��O�j
		reason = 4;
	}
// MH810103 GG119202(E) �J�[�h��������M��̌���NG�i304�j�̈���
// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
	else if (Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO) {
		// �����c���Ɖ�^�C���A�E�g�i50-0010�A11�@��1�j
		reason = 6;
	}
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
	else {
		// ���Ϗ��=�u�݂Ȃ����ρv��M
		reason = 3;
	}
	sprintf((char*)&err_wk[3], "%02d", reason);
// MH810103 GG119202(E) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
	// ����z
	memset(suica_work_buf, 0, sizeof(suica_work_buf));
	if (EcDeemedLog.PayPrice >= 1000) {
	// 4���ȏ�
		sprintf((char*)suica_work_buf, "\\%ld,%03ld", EcDeemedLog.PayPrice/1000, EcDeemedLog.PayPrice%1000);
	}
	else {
	// 3���ȉ�
		sprintf((char*)suica_work_buf, "\\%ld", EcDeemedLog.PayPrice);
	}
// MH810103 GG119202(S) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
//	sprintf((char*)&err_wk[3], " %8s", suica_work_buf);
//	err_chk2((char)jvma_setup.mdl, ERR_EC_SETTLE_ABORT, 1, 1, 0, (void*)err_wk);
	sprintf((char*)&err_wk[5], " %8s", suica_work_buf);
	if (kind == 0) {
		err_chk2((char)jvma_setup.mdl, ERR_EC_DEEMED_SETTLEMENT, 2, 1, 0, (void*)err_wk);
	}
// MH810105(S) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P
	else if (kind == 3) {
		err_chk2((char)jvma_setup.mdl, ERR_EC_PROCESS_FAILURE, 2, 1, 0, (void*)err_wk);
	}
// MH810105(E) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P
	else {
		err_chk2((char)jvma_setup.mdl, ERR_EC_SETTLE_ABORT, 1, 1, 0, (void*)err_wk);
	}
// MH810103 GG119202(E) ���Ϗ�ԂɁu�݂Ȃ����ρv�ǉ�
}
// MH810103 GG119202(E) �݂Ȃ����ψ������̓���

// MH810103 GG119202(S) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���
//[]----------------------------------------------------------------------[]
///	@brief			���σ��[�_�ڑ����̓���������
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/12/07
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void EcEnableNyukin(void)
{
	if (!isEC_USE()) {
		return;
	}

	if (check_enable_multisettle() <= 1) {
		// �}���`�u�����h�ȊO�͉������Ȃ�
		return;
	}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	if (!isEcBrandNoEMoney(RecvBrandResTbl.no, 0)) {
//		// �d�q�}�l�[�I����ԂłȂ���΁A�������Ȃ�
	if (!EcUseKindCheck(convert_brandno(RecvBrandResTbl.no))) {
		// �d�q�}�l�[�I����ԂłȂ���΁A�������Ȃ�
		// QR�R�[�h�I����ԂłȂ���΁A�������Ȃ�
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		return;
	}
	if (Suica_Rec.Data.BIT.SETTLMNT_STS_RCV) {
		// ���Ϗ������͉������Ȃ�
		return;
	}

	queset(OPETCBNO, EC_EVT_ENABLE_NYUKIN, 0, NULL);
}
// MH810103 GG119202(E) �d�q�}�l�[�I����ɍd�݁E����������s�ɂ���

// MH810103 MHUT40XX(S) Edy�EWAON�Ή��^���ό��ʎ�M�҂��^�C�}������
//[]----------------------------------------------------------------------[]
///	@brief			�^�C�}�X�V����
//[]----------------------------------------------------------------------[]
///	@param[in]		void
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/02/19<br>
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static void EcTimerUpdate(void)
{
	ushort	wait_time;

	if (!isEC_STS_CARD_PROC() &&
		!Suica_Rec.Data.BIT.CTRL_MIRYO) {
		// �J�[�h�������A�������ȊO�̓^�C�}�X�V���Ȃ�
		return;
	}

	// �J�[�h�������A�������̊Ď��^�C�}���X�V����
	// ���σ��[�_����^�C�}�X�V�˗�������I�ɒʒm�����
	switch (RecvBrandResTbl.no) {
	case BRANDNO_KOUTSUU:
		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 16, 3, 1);
		if (wait_time == 0) {
			wait_time = 90;
		}
		else if (wait_time < 70) {
			wait_time = 70;
		}
		break;
	case BRANDNO_EDY:
		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 41, 3, 1);
		if (wait_time == 0) {
			wait_time = 90;
		}
		else if (wait_time < 70) {
			wait_time = 70;
		}
		break;
	case BRANDNO_WAON:
		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 42, 3, 1);
		if (wait_time == 0) {
			wait_time = 90;
		}
		else if (wait_time < 70) {
			wait_time = 70;
		}
		break;
// MH810104(S) nanaco�EiD�EQUICPay�Ή�2
//// MH810103 MHUT40XX(S) nanaco�EiD�EQUICPay�Ή�
//	case BRANDNO_NANACO:
//		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 43, 3, 1);
//		if (wait_time == 0) {
//			wait_time = 50;
//		}
//		else if (wait_time < 40) {
//			wait_time = 40;
//		}
//		break;
//	case BRANDNO_ID:
//		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 44, 3, 1);
//		if (wait_time == 0) {
//			wait_time = 50;
//		}
//		else if (wait_time < 40) {
//			wait_time = 40;
//		}
//		break;
//	case BRANDNO_QUIC_PAY:
//		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 45, 3, 1);
//		if (wait_time == 0) {
//			wait_time = 50;
//		}
//		else if (wait_time < 40) {
//			wait_time = 40;
//		}
//		break;
//// MH810103 MHUT40XX(E) nanaco�EiD�EQUICPay�Ή�
	case BRANDNO_NANACO:
		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 43, 3, 1);
		if (wait_time == 0) {
			wait_time = 90;
		}
		else if (wait_time < 70) {
			wait_time = 70;
		}
		break;
	case BRANDNO_ID:
		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 44, 3, 1);
		if (wait_time == 0) {
			wait_time = 90;
		}
		else if (wait_time < 70) {
			wait_time = 70;
		}
		break;
	case BRANDNO_QUIC_PAY:
		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 45, 3, 1);
		if (wait_time == 0) {
			wait_time = 90;
		}
		else if (wait_time < 70) {
			wait_time = 70;
		}
		break;
// MH810104(E) nanaco�EiD�EQUICPay�Ή�2
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	case BRANDNO_QR:
		// QR�R�[�h
		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 47, 3, 1);
		if (wait_time == 0) {
			wait_time = 90;
		}
		else if (wait_time < 70) {
			wait_time = 70;
		}
		break;
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105(S) MH364301 PiTaPa�Ή�
	case BRANDNO_PITAPA:
		wait_time = (ushort)prm_get(COM_PRM, S_ECR, 46, 3, 1);
		if (wait_time == 0) {
			wait_time = 90;
		}
		else if (wait_time < 70) {
			wait_time = 70;
		}
		break;
// MH810105(E) MH364301 PiTaPa�Ή�
	default:
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//		// �d�q�}�l�[�u�����h�̂ݍX�V����
		// �d�q�}�l�[�u�����h�AQR�R�[�h�u�����h�ȊO�͍X�V���Ȃ�
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
		return;
	}

	if (Suica_Rec.Data.BIT.CTRL_MIRYO != 0) {
		wait_time = (wait_time * 2) + 1;
		LagTim500ms(LAG500_SUICA_MIRYO_RESET_TIMER, (short)wait_time, ec_miryo_timeout);
	}
	else if (isEC_STS_CARD_PROC()) {
		wait_time *= 50;
		Lagtim(OPETCBNO, TIMERNO_EC_INQUIRY_WAIT, wait_time);
	}
}
// MH810103 MHUT40XX(E) Edy�EWAON�Ή��^���ό��ʎ�M�҂��^�C�}������
// MH810103(s) �d�q�}�l�[�Ή� ���Z��Ԓʒm��Ope_mod�ɂ��؂�ւ���
//[]----------------------------------------------------------------------[]
///	@brief			�d�q�}�l�[���p�\�`�F�b�N(��t�ی��Ȃ���)
//[]----------------------------------------------------------------------[]
///	@param[in]		open_chk:		���ω\�`�F�b�N
///	@param[in]		zandaka_chk:	�c���Ɖ�\�`�F�b�N
///	@return			1:�d�q�}�l�[���p��, 0:�d�q�}�l�[���p�s��
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2020/07/06<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
uchar isEcEmoneyEnabledNotCtrl(uchar open_chk, uchar zandaka_chk)
{
	uchar	ret = 0;
	int		i, j;

	if (!isEC_USE()) {
		return ret;
	}

	// ��t�ۂ͌��Ȃ�
	if (open_chk || zandaka_chk) {
		if (!Suica_Rec.suica_err_event.BYTE					// ���[�_�[�g�p��
			&& Suica_Rec.Data.BIT.INITIALIZE				// ����������
//			&& Suica_Rec.Data.BIT.CTRL						// ��t���
			) {	
			;		// �R���f�B�V����OK
		}
		else {
			return ret;
		}
	}

	for (i = 0; i < TBL_CNT(RecvBrandTbl); i++) {
		for (j = 0; j < RecvBrandTbl[i].num; j++) {
			// ��M�����u�����h�e�[�u���ɓd�q�}�l�[�u�����h���܂܂�Ă��邩�H
			if (isEcBrandNoEMoney(RecvBrandTbl[i].ctrl[j].no, zandaka_chk)) {
				if (open_chk) {
					// ���ω\�`�F�b�N
					if (IsValidBrand(i, j)) {
						ret = 1;
						break;
					}
				}
				else {
					ret = 1;
					break;
				}
			}
		}
		if (ret != 0) {
			break;
		}
	}
	return ret;
}
// MH810103(e) �d�q�}�l�[�Ή� ���Z��Ԓʒm��Ope_mod�ɂ��؂�ւ���

// MH810105 GG119202(S) ������������W�v�d�l���P
//// MH810103 GG118807_GG118907(S) �u�����c���Ɖ���v���ɂ������m����J�E���g����
////[]----------------------------------------------------------------------[]
/////	@brief			�݂Ȃ����ϔ������̌��ό��ʃf�[�^�Z�b�g
////[]----------------------------------------------------------------------[]
/////	@param[out]		*data	: ���ό��ʃf�[�^
/////	@return			void
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2021/03/05
////[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
//static void EcSetDeemedSettlementResult(EC_SETTLEMENT_RES *data)
//{
//	// �u�����h�ԍ�
//	data->brand_no = EcDeemedLog.EcDeemedBrandNo;
//	// �x���z
//	data->settlement_data = Product_Select_Data;
//	// �₢���킹�ԍ�
//	memset(data->inquiry_num, '*', sizeof(data->inquiry_num));
//	data->inquiry_num[4] = '-';
//
//	memset(data->Card_ID, 0x20, sizeof(data->Card_ID));
//	switch (data->brand_no) {
//	case BRANDNO_KOUTSUU:
//		// �J�[�h�ԍ�
//		memset(data->Card_ID, '*', ECARDID_SIZE_KOUTSUU);
//		// SPRWID
//		memset(data->Brand.Koutsuu.SPRW_ID, '*', sizeof(data->Brand.Koutsuu.SPRW_ID));
//		break;
//	case BRANDNO_EDY:
//		// �J�[�h�ԍ�
//		memset(data->Card_ID, '*', ECARDID_SIZE_SUICA);
//		// Edy����ʔ�
//		memset(data->Brand.Edy.DealNo, '*', sizeof(data->Brand.Edy.DealNo));
//		// �J�[�h����ʔ�
//		memset(data->Brand.Edy.CardDealNo, '*', sizeof(data->Brand.Edy.CardDealNo));
//		// ��ʒ[��ID
//		memset(data->Brand.Edy.TerminalNo, '*', sizeof(data->Brand.Edy.TerminalNo));
//		break;
//	case BRANDNO_WAON:
//		// �J�[�h�ԍ�
//		memset(data->Card_ID, '*', ECARDID_SIZE_SUICA);
//		// SPRWID
//		memset(data->Brand.Waon.SPRW_ID, '*', sizeof(data->Brand.Waon.SPRW_ID));
//		break;
//// MH810104 GG119202(S) nanaco�EiD�EQUICPay�Ή�
//	case BRANDNO_NANACO:
//		// �J�[�h�ԍ�
//		memset(data->Card_ID, '*', ECARDID_SIZE_SUICA);
//		// �[������ʔ�
//		memset(data->Brand.Nanaco.DealNo, '*', sizeof(data->Brand.Nanaco.DealNo));
//		// ��ʒ[��ID
//		memset(data->Brand.Nanaco.TerminalNo, '*', sizeof(data->Brand.Nanaco.TerminalNo));
//		break;
//	case BRANDNO_ID:
//		// �J�[�h�ԍ�
//		memset(data->Card_ID, '*', ECARDID_SIZE_SUICA);
//		// ��ʒ[��ID
//		memset(data->Brand.Id.TerminalNo, '*', sizeof(data->Brand.Id.TerminalNo));
//		break;
//	case BRANDNO_QUIC_PAY:
//		// �J�[�h�ԍ�
//		memset(data->Card_ID, '*', sizeof(data->Card_ID));
//		// ��ʒ[��ID
//		memset(data->Brand.QuicPay.TerminalNo, '*', sizeof(data->Brand.QuicPay.TerminalNo));
//		break;
//// MH810104 GG119202(E) nanaco�EiD�EQUICPay�Ή�
//	default:
//		break;
//	}
//}
//// MH810103 GG118807_GG118907(E) �u�����c���Ɖ���v���ɂ������m����J�E���g����
// MH810105 GG119202(E) ������������W�v�d�l���P

// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
//[]----------------------------------------------------------------------[]
///	@brief			��M�������O����f�[�^���i�[����֐�
//[]----------------------------------------------------------------------[]
///	@return			ret		:	FALSE=�Z�b�g���Ȃ�(�ȍ~�̏������Ȃ�)/TRUE=�Z�b�g����(�ȍ~�̏����Ɉڍs)
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/05/07
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
uchar EcRecvDeemed_DataSet(uchar *buf)
{
	ec_settlement_data_save(&Ec_Settlement_Res, buf, 0);

	if( Ec_Settlement_Res.Result == EPAY_RESULT_CANCEL_PAY_OFF ){
		// �u�����h�Ɋ֌W�Ȃ��A���ό��ʁ������L�����Z���̏ꍇ�͉����������Ȃ�
		return FALSE;
	}
	if( (Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO_ZANDAKA_END && Ec_Settlement_Res.brand_no == BRANDNO_CREDIT)||
		(Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO && Ec_Settlement_Res.brand_no == BRANDNO_CREDIT) ){
		// �u�����h���N���W�b�g�ł���A���ό��ʁ������c���Ɖ��or�����m��̏ꍇ�͉����������Ȃ�
		return FALSE;
	}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	if (Ec_Settlement_Res.brand_no == BRANDNO_QR &&
		(Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO ||
		 Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO_ZANDAKA_END)) {
		// �u�����h��QR�R�[�h�Ō��ό��ʁ������m��A�����c���Ɖ���̏ꍇ�͉����������Ȃ�
		return FALSE;
	}
	if (Ec_Settlement_Res.brand_no != BRANDNO_QR &&
		Ec_Settlement_Res.Result == EPAY_RESULT_PAY_MIRYO) {
		// �u�����h��QR�R�[�h�ȊO�Ō��ό��ʁ��x�������̏ꍇ�͉����������Ȃ�
		return FALSE;
	}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	// EcRecvDeemedData�Ɏ�M�f�[�^���Z�b�g
	EcElectronSet_DeemedData(&Ec_Settlement_Res);
	// �G���[�R�[�h��EcRecvDeemedData�ɃZ�b�g����
	memcpy( &EcRecvDeemedData.EcErrCode, &Ec_Settlement_Rec_Code, sizeof(EcRecvDeemedData.EcErrCode) );
// MH810105 GG119202(S) �����m��̒���f�[�^��M����R0176��o�^����
	if (Ec_Settlement_Res.Result == EPAY_RESULT_MIRYO) {
		// ���挋�ʁ������m��̏ꍇ��R0176��o�^����
		wmonlg(OPMON_EC_MIRYO_TIMEOUT, NULL, 0);
	}
// MH810105 GG119202(E) �����m��̒���f�[�^��M����R0176��o�^����

	return TRUE;
}
//[]----------------------------------------------------------------------[]
///	@brief			���O����f�[�^�̃��O�o�^�A�󎚏���
//[]----------------------------------------------------------------------[]
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/04/28
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
void EcRecvDeemed_RegistPri( void )
{
	uchar	brand;
	T_FrmReceipt	DeemedData;

	memset(&DeemedData, 0, sizeof(T_FrmReceipt));
	brand = EcRecvDeemedData.Electron_data.Ec.e_pay_kind;

	// �o�Ɏ����ɐ��Z�@�̌��ݎ������Z�b�g����i�󎚎����Ƃ��Ă������j
	memcpy(&EcRecvDeemedData.TOutTime, &CLK_REC, sizeof(date_time_rec));

	if ( brand != EC_UNKNOWN_USED && EcRecvDeemedData.EcResult != EPAY_RESULT_NG ) {
		// �s���u�����h�ȊO���O�o�^
		// ���ό���NG�̏ꍇ�̓��O�o�^���Ȃ�
		ac_flg.cycl_fg = 57;											// 57:���ϐ��Z���~��
		ac_flg.ec_recv_deemed_fg = 0;									// ���O����f�[�^�����t���OOFF
		Log_Write(eLOG_PAYMENT, &EcRecvDeemedData, TRUE);
	}
	else {
		ac_flg.cycl_fg = 58;											// 58:���ϐ��Z���~�f�[�^��t
		ac_flg.ec_recv_deemed_fg = 0;									// ���O����f�[�^�����t���OOFF
	}

	if( PrnJnlCheck() == ON ){
		DeemedData.prn_kind = J_PRI;									/* �������ʁF�ެ��� */
		DeemedData.prn_data = &EcRecvDeemedData;						/* �̎��؈��ް����߲����� */

		// pritask�֒ʒm
		queset(PRNTCBNO, PREQ_RECV_DEEMED, sizeof(T_FrmReceipt), &DeemedData);
	}

	// �ڍ׃G���[�R�[�h��o�^����
	EcErrCodeChk( EcRecvDeemedData.EcErrCode, brand );

	ac_flg.cycl_fg = 0;													/* ���O����f�[�^�󎚏�������*/
// MH810105 GG119202(S) ���[�_���璼�撆�̏�ԃf�[�^��M�����ۂ̐V�K�A���[����݂���
	if( !EC_STATUS_DATA.StatusInfo.LastSetteRunning ){					// ��ԃf�[�^�̒��撆bit OOFF
		// �u���σ��[�_���p�s�v����
		alm_chk(ALMMDL_SUB, ALARM_EC_LASTSETTERUN, 0);					// �װ�۸ޓo�^�i�����j
	}
// MH810105 GG119202(E) ���[�_���璼�撆�̏�ԃf�[�^��M�����ۂ̐V�K�A���[����݂���	
}

//[]----------------------------------------------------------------------[]
///	@brief			�ڍ׃G���[�R�[�h�A�G���[�o�^����
//[]----------------------------------------------------------------------[]
///	@param[in]		err_code	   �@�F�@���������ڍ׃G���[�R�[�h
///	@param[in]		err_brand_index�@�F�@���������ڍ׃G���[�R�[�h�̃u�����h
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/04/27
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
void EcErrCodeChk( uchar *err_code, uchar err_brand_index )
{
	// �ڍ׃G���[�R�[�h
	memset(err_wk, 0, sizeof(err_wk));
	memcpy(err_wk, "�ڍ׃R�[�h000", 13);
	memcpy(&err_wk[10], err_code, sizeof(Ec_Settlement_Rec_Code));

	ECCTL.ec_Disp_No = 0;												// ���ό��ʏڍ׃G���[�R�[�h�̈ē��\���p�^�[��

	// �ڍ׃G���[�R�[�h�� 000 �ł͂Ȃ�
	if (!(err_code[0] == '0' &&
		  err_code[1] == '0' &&
		  err_code[2] == '0')) {
		ec_errcode_err_chk(err_wk, err_code, err_brand_index);			// �ڍ׃G���[�o�^
	}
	// �ڍ׃G���[�R�[�h201�ł͂Ȃ� && �u�����h�ԍ��� 0
	if(!(err_code[0] == '2' &&
		 err_code[1] == '0' &&
		 err_code[2] == '1') &&
		 err_brand_index == EC_ZERO_USED){
		err_chk((char)jvma_setup.mdl, ERR_EC_BRANDNO_ZERO, 1, 0, 0);	// E3265�o�^
	}
}
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��

// MH810105 GG119202(S) ���Ϗ������ɏ�Q�������������̓���
//[]----------------------------------------------------------------------[]
///	@brief			���Ϗ������ɏ�Q�������������̃u�����h���̓���(50-0014)
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no	: �u�����hNo
///	@return			ret			: 0=�݂Ȃ����ςŐ��Z����
///								: 1=�x��
///								: 2=�N���W�b�g���L�̏���(���ό���NG�Ƃ���)
///								: 3=QR�R�[�h���L�̏���(��Q�A���\���s+���ό���NG)
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/07/16<br>
///					Update	:
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
static uchar EcGetActionWhenFailure( ushort brand_no )
{
	uchar	brand_system = 0;									// 1�F�d�q�}�l�[�i�v���y�C�h�j 2�F�d�q�}�l�[�i�|�X�g�y�C�j
																// 3�F�N���W�b�g 4�FQR�R�[�h
	uchar	wk_process = 0;										// 0�F�݂Ȃ����ςŐ��Z����
	char	pos;

	// �݂Ȃ��������������ϕ��@���m�F��
	// ���ꂼ��̐ݒ�ɂ�菈����ύX����
	brand_system = isEcBrandNoEMoney(EcDeemedLog.EcDeemedBrandNo, 0);
	if (brand_system != 0) {
		if (isEcBrandNoEMoney(EcDeemedLog.EcDeemedBrandNo, 1)) {
			// �d�q�}�l�[�i�v���y�C�h�j
			brand_system = 1;
		}
		else {
			// �d�q�}�l�[�i�|�X�g�y�C�j
			brand_system = 2;
		}
	}

	// �d�q�}�l�[�ł͂Ȃ�����
	if( brand_system == 0 ){
		// �N���W�b�g�����`�F�b�N
		if( brand_no == BRANDNO_CREDIT ){
			brand_system = 3;									// 2�F�N���W�b�g
		}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
		else if( brand_no == BRANDNO_QR ){
			brand_system = 4;									// 4�FQR�R�[�h�n
		}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	}

	switch( brand_system ){
		case 1:													// �d�q�}�l�[�i�v���y�C�h�j
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
			if (brand_no == BRANDNO_WAON) {
				switch (prm_get(COM_PRM, S_ECR, 14, 1, 4)){
				case 1:
					wk_process = 1;								// 1�F�x��
					break;
				default:
					wk_process = 0;								// 0�F�݂Ȃ����ςŐ��Z����
					break;
				}
			}
			else {
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
			switch (prm_get(COM_PRM, S_ECR, 14, 1, 2)){
				case 1:
					wk_process = 1;								// 1�F�x��
					break;
// MH810105(S) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P
				case 2:
					wk_process = 2;								// 2�F���Z���~
					break;
// MH810105(E) MH364301 ���Ϗ������ɏ�Q�������������̓��쏈�����P
				default:
					wk_process = 0;								// 0�F�݂Ȃ����ςŐ��Z����
					break;
			}
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
			}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
			break;
		case 2:													// �d�q�}�l�[�i�|�X�g�y�C�j
		case 3:													// �N���W�b�g
			// �p�����[�^�̎Q�ƈʒu
			if (brand_system == 2) {
				pos = 3;
			}
			else {
				pos = 1;
			}
			switch (prm_get(COM_PRM, S_ECR, 14, 1, pos)){
				case 1:
					wk_process = 1;								// 1�F�x��
					break;
				case 2:
					wk_process = 2;								// 2�F�N���W�b�g���L�̏���(���ό���NG�Ƃ���)
					break;
				default:
					wk_process = 0;								// 0�F�݂Ȃ����ςŐ��Z����
					break;
			}
			break;
		case 4:													// QR�R�[�h�n
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//			switch (prm_get(COM_PRM, S_ECR, 14, 1, 4)){
//				case 1:
//					wk_process = 1;								// 1�F�x��
//					break;
//				case 2:
//					wk_process = 3;								// 3�FQR�R�[�h���L�̏���(��Q�A���\���s+���ό���NG)
//					break;
//				default:
//					wk_process = 0;								// 0�F�݂Ȃ����ςŐ��Z����
//					break;
//			}
			switch (prm_get(COM_PRM, S_ECR, 14, 1, 5)){
			case 1:
				wk_process = 1;									// 1�F�x��
				break;
			case 2:
				wk_process = 0;									// 0�F�݂Ȃ����ςŐ��Z����
				break;
			default:
				wk_process = 3;									// 3�FQR�R�[�h���L�̏���(��Q�A���\���s+���Z���~)
				break;
			}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
			break;
		default:
			break;
	}
	return wk_process;
}
// MH810105 GG119202(E) ���Ϗ������ɏ�Q�������������̓���

// MH810105(S) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX
//[]----------------------------------------------------------------------[]
///	@brief			�����c���Ɖ�^�C���A�E�g���̓���
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_no	: �u�����hNo.
///	@return			ret			: 0=�݂Ȃ����ςŐ��Z����
///								: 1=�x��
///								: 2=���Z���~
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2022/03/17
//[]------------------------------------- Copyright(C) 2022 AMANO Corp.---[]
static uchar EcGetMiryoTimeoutAction(ushort brand_no)
{
	uchar	wk_process = 0;
	char	err;


	if (brand_no == BRANDNO_WAON) {
		switch (prm_get(COM_PRM, S_ECR, 11, 1, 6)) {
		case 2:
			wk_process = 1;							// 1�F�x��

			// E3267��ʒm����
			err = 2;
			break;
		default:
			wk_process = 0;							// 0�F�݂Ȃ����ςŐ��Z����

			// �����^�C���A�E�g�ʒm���@���擾
			err = (char)prm_get(COM_PRM, S_ECR, 11, 1, 5);
			break;
		}
	}
	else {
		switch (prm_get(COM_PRM, S_ECR, 10, 1, 6)) {
		case 2:
			wk_process = 1;							// 1�F�x��

			// E3267��ʒm����
			err = 2;
			break;
		case 0:
			wk_process = 2;							// 2�F���Z���~

			// �����^�C���A�E�g�ʒm���@���擾
			err = (char)prm_get(COM_PRM, S_ECR, 10, 1, 5);
			break;
		default:
			wk_process = 0;							// 0�F�݂Ȃ����ςŐ��Z����

			// �����^�C���A�E�g�ʒm���@���擾
			err = (char)prm_get(COM_PRM, S_ECR, 10, 1, 5);
			break;
		}
	}

	// �����c���Ɖ�^�C���A�E�g���̃G���[�o�^
	EcRegistMiryoTimeoutError(&Ec_Settlement_Res, err);

	return wk_process;
}
// MH810105(E) MH364301 �����c���Ɖ�^�C���A�E�g���̓���d�l�ύX

// MH810105 GG119202(S) �݂Ȃ����σv�����g�ɒ��O������e���󎚂���
//[]----------------------------------------------------------------------[]
///	@brief			�����������莸�s����
//[]----------------------------------------------------------------------[]
///	@param[in]		pData		: PayData�̃|�C���^
///	@return			ret			: 0=�������莸�s�ȊO
///								: 1=�������莸�s
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/07/30
///					Update	:
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
uchar IsEcMiryoHikisariNG(Receipt_data *pData)
{
	uchar	ret = 0;

	// ���ό��ʁ������m��A���A�ڍ׃G���[��h90�̏ꍇ��
	// �����������莸�s�Ɣ��肷��
	if (pData->EcResult == EPAY_RESULT_MIRYO &&
		!memcmp(pData->EcErrCode, EcMiryoFailErrTbl, 3)) {
		ret = 1;
	}
	return ret;
}
// MH810105 GG119202(E) �݂Ȃ����σv�����g�ɒ��O������e���󎚂���

// MH810105 GG119202(S) �d�q�}�l�[���σG���[�������̉�ʖ߂�d�l�ύX
//[]----------------------------------------------------------------------[]
///	@brief			�ǂݎ��҂��p������
//[]----------------------------------------------------------------------[]
///	@param[in]		pErrCode	: �ڍ׃G���[�̃|�C���^
///	@return			ret			: 0=�ǂݎ��҂��p���Ȃ�
///								: 1=�ǂݎ��҂��p������
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/09/17
///					Update	:
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
uchar IsEcContinueReading(uchar *pErrCode)
{
	uchar	ret = 0;

	// �ǂݎ��҂��p������G���[�𔻒肷��
	if (!memcmp(pErrCode, EcContinueReadingTbl, 3)) {
		ret = 1;
	}
	return ret;
}
// MH810105 GG119202(E) �d�q�}�l�[���σG���[�������̉�ʖ߂�d�l�ύX

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//[]----------------------------------------------------------------------[]
///	@brief			�ǂݎ��҂��p������
//[]----------------------------------------------------------------------[]
///	@param[in]		pErrCode	: �ڍ׃G���[�̃|�C���^
///	@return			ret			: 0=�ǂݎ��҂��p���Ȃ�
///								: 1=�ǂݎ��҂��p������
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/12/16
///					Update	:
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
uchar IsQrContinueReading(uchar *pErrCode)
{
	uchar	ret = 0;
	uchar	i = 0;

	// �ǂݎ��҂��p������G���[�𔻒肷��
	for (i = 0;i < TBL_CNT(QrContinueReadingTbl);i++) {
		if (!memcmp(pErrCode, QrContinueReadingTbl[i], 3)) {
			ret = 1;
			break;
		}
	}
	return ret;
}
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

// MH810105(S) MH364301 PiTaPa�Ή�
//[]----------------------------------------------------------------------[]
///	@brief			�d�q�}�l�[�u�����h�|�X�g�y�C�^�`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]		brand_index	: �u�����h�ԍ�
///	@return			ret			: 0=�v���y�C�h�^
///								: 1=�|�X�g�y�C�^
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/11/29
///					Update	:
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
uchar EcBrandPostPayCheck( uchar brand_index )
{
	uchar	ret = 0;

	// �|�X�g�y�C�^���𔻒肷��
	switch ( brand_index ){
		case EC_ID_USED:			// iD����
		case EC_QUIC_PAY_USED:		// QUICPay����
		case EC_PITAPA_USED:		// PiTaPa����
			ret = 1;
			break;
		default:
			break;
	}
	return ret;
}
// MH810105(E) MH364301 PiTaPa�Ή�
