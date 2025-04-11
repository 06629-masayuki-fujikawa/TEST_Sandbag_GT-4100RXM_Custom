/*[]----------------------------------------------------------------------[]*/
/*| Maintenance control header file                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :                                                          |*/
/*| Date        :  2XXX-XX-XX                                              |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/

#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"Message.h"
#include	"lcd_def.h"
#include	"Strdef.h"
#include	"mnt_def.h"
#include	"PRM_TBL.H"
#include	"LKmain.h"
#include	"flp_def.h"
#include	"remote_dl.h"

/*--------------------------------------------------------------------------*/
/*	Menu Table																*/
/*--------------------------------------------------------------------------*/
/* Attendant */
/* User Maintenance 0-59 */
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�����e�i���X��\��)
// ��USER_TBL��ύX����ꍇ��USER_TBL_CC�������Ƀ����e�i���X����
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�����e�i���X��\��)
const	unsigned short	USER_TBL[][4] = {
							{ 0,	26,	__MNT_ERARM,		MNT_ERARM      },	// .�G���[�E�A���[���m�F
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							{ 0,	20,	__MNT_FLCTL,		MNT_FLCTL      },	// .���b�N���u�J��
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	14,	__MNT_MNCNT,		MNT_MNCNT      },	// .�ޑK�Ǘ�
							{ 0,	12,	__MNT_TTOTL,		MNT_TTOTL      },	// .�s�W�v
							{ 0,	14,	__MNT_GTOTL,		MNT_GTOTL      },	// .�f�s�W�v
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							{ 0,	16,	__MNT_STAYD,		MNT_STAYD      },	// .�◯�ԏ��
//							{ 0,	22,	__MNT_LOGPRN,		MNT_LOGPRN     },	// .�������v�����g
//						    { 0,	14,	__MNT_FLCNT,		MNT_FLCNT      },	// .���ԑ䐔
//						    { 0,	22,	__MNT_FLCTRL,		MNT_FLCTRL     },	// .���ԃR���g���[��
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	14,	__MNT_STSVIEW,		MNT_STSVIEW    },	// .��Ԋm�F
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//						    { 0,	14,	__MNT_FLSTS,		MNT_FLSTS      },	// .�Ԏ����
//						    { 0,	14,	__MNT_CARFAIL,		MNT_CARFAIL    },	// .�Ԏ��̏�
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	18,	__MNT_REPRT,		MNT_REPRT      },	// .�̎��؍Ĕ��s
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//						    { 0,	16,	__MNT_UKERP,		MNT_UKERP      },	// .���ԏؖ���
//						    { 0,	20,	__MNT_INVLD,		MNT_INVLD      },	// .����L���^����
//						    { 0,	20,	__MNT_ENTRD,		MNT_ENTRD      },	// .������Ɂ^�o��
//						    { 0,	26,	__MNT_PSTOP,		MNT_PSTOP      },	// .��������Z���~�f�[�^
//						    { 0,	20,	__MNT_PASCK,		MNT_PASCK      },	// .������`�F�b�N
//						    { 0,	18,	__MNT_TKTDT,		MNT_TKTDT      },	// .���f�[�^�m�F
							{ 0,	28,	__MNT_QRCHECK,		MNT_QRCHECK     },	// .QR�f�[�^�m�F
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	16,	__MNT_CLOCK,		MNT_CLOCK      },	// .���v���킹
							{ 0,	20,	__MNT_SERVS,		MNT_SERVS      },	// .�T�[�r�X�^�C��
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							{ 0,	26,	__MNT_FLTIM,		MNT_FLTIM      },	// .���b�N���u�J�^�C�}�[
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	22,	__MNT_SPCAL,		MNT_SPCAL      },	// .���ʓ��^���ʊ���
							{ 0,	16,	__MNT_OPCLS,		MNT_OPCLS      },	// .�c�x�Ɛؑ�
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							{ 0,	20,	__MNT_SHTER,		MNT_SHTER      },	// .�V���b�^�[�J��
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	20,	__MNT_PWMOD,		MNT_PWMOD      },	// .�W���p�X���[�h
// MH810100(S) K.Onodera 2020/03/05 #3902 LCD�^�b�`�̌��ʉ���2���
//							{ 0,	18,	__MNT_KEYVL,		MNT_KEYVL      },	// .�L�[���ʒ���
							{ 0,	20,	__MNT_KEYVL,		MNT_KEYVL      },	// .�u�U�[���ʒ���
// MH810100(E) K.Onodera 2020/03/05 #3902 LCD�^�b�`�̌��ʉ���2���
							{ 0,	22,	__MNT_CNTRS,		MNT_CNTRS      },	// .�P�x����
							{ 0,	18,	__MNT_VLSW,			MNT_VLSW       },	// .�����ē�����
							{ 0,	18,	__MNT_OPNHR,		MNT_OPNHR      },	// .�c�ƊJ�n����
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							{ 0,	12,	__MNT_TKEXP,		MNT_TKEXP      },	// .������
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	14,	__MNT_MNYSET,		MNT_MNYSET     },	// .�����ݒ�							
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							{ 0,	20,	__MNT_ATTENDDATA,	MNT_ATTENDDATA },	// .�W���L���f�[�^
//							{ 0,	22,	__MNT_PWDKY,		MNT_PWDKY	   },	// .�����ԍ������o�� */
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	14,	__MNT_EXTEND,		MNT_EXTEND     },	// .�g���@�\
};

/* Engeneer */
/* System Maintenance 60-79 */
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�����e�i���X��\��)
// ��SYS_TBL��ύX����ꍇ��SYS_TBL_CC�������Ƀ����e�i���X����
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�����e�i���X��\��)
const	unsigned short	SYS_TBL[][4] = {
							{ 0,	20,	0,	MNT_PRSET },	/*  1.�p�����[�^�[�ݒ� */
							{ 0,	16,	1,	MNT_FNCHK },	/*  2.����`�F�b�N */
							{ 0,	16,	2,	MNT_FNCNT },	/*  3.����J�E���g */
							{ 0,	24,	3,	MNT_LOGFL },	/*  4.���O�t�@�C���v�����g */
							{ 0,	26,	4,	MNT_BAKRS },	/*  5.�o�b�N�A�b�v�^���X�g�A */
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							 { 0,	14,	5,	MNT_CDISS },	/*  6.�J�[�h���s */
//							{ 0,	16,	6,	MNT_FLTRF },	/*  7.�t�@�C���]�� */
//							{ 0,	16,	7,	MNT_SYSSW },	/*  8.�V�X�e���ؑ� */
//							{ 0,	18,	8,	MNT_RYOTS },	/*  9.�����v�Z�e�X�g */
							{ 0,	16,	5,	MNT_FLTRF },	/*  6.�t�@�C���]�� */
							{ 0,	16,	6,	MNT_SYSSW },	/*  7.�V�X�e���ؑ� */
							{ 0,	18,	7,	MNT_RYOTS },	/*  8.�����v�Z�e�X�g */
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	26,	8,	MNT_INITI },	/* 10.�X�[�p�[�C�j�V�����C�Y */
};

/* Parameter Setting 200-202 */
const	unsigned short	PARA_SET_TBL[][4] = {
							{ 0,	20,	0,	BASC_PARA },	/* 1.��{�p�����[�^�[ */
							{ 0,	20,	1,	CELL_PARA },	/* 2.�Ԏ��p�����[�^�[ */
							{ 0,	26,	2,	ROCK_PARA },	/* 3.���b�N��ʃp�����[�^�[ */
};
/* Basic Parameter Setting 220-224 */
const	unsigned short	BPARA_SET_TBL[][4] = {
							{ 0,	20,	0,	CMON_PARA },	/* 1.���ʃp�����[�^�[ */
							{ 0,	12,	1,	PRNT_PARA },	/* 2.�v�����g */
							{ 0,	20,	2,	DEFA_PARA },	/* 3.�f�t�H���g�Z�b�g */
};

// �t�@�C���]��
const	unsigned short	FILE_TRF_TBL[][4] = {
							{ 0,	30,	0,	FILE_FTP_CLI},		/* 2.FTP�ڑ�(�N���C�A���g) */
							{ 0,	26,	1,	FILE_FTP_SEV},		/* 3.FTP�ڑ�(�T�[�o�[) */
};

// �V�X�e���ؑ�
const	unsigned short	SYS_SW_TBL[][4] = {
							{ 0,	20,	0,	SYS_MAIN_PRO},		/* 1.���C���v���O���� */
// MH810100(S) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							{ 0,	18,	1,	SYS_SUB_PRO},		/* 2.�T�u�v���O���� */
//							{ 0,	20,	2,	SYS_COM_PRM},		/* 3.���ʃp�����[�^�[ */
							{ 0,	20,	1,	SYS_COM_PRM},		/* 2.���ʃp�����[�^�[ */
// MH810100(E) K.Onodera 2019/10/17 �Ԕԃ`�P�b�g���X(�����e�i���X)
};

// NTNET�`�F�b�N���j���[
const	unsigned short	FNTNET_CHK_TBL[][4] = {
							{ 0,	16,	0,	FNTNET1_CHK },		// 1: �f�[�^�N���A
							{ 0,	24,	1,	FNTNET2_CHK },		// 2: �ʐM���W���[��������
							{ 0,	12,	2,	FNTNET3_CHK },		// 3: ��ԕ\��
};

const	unsigned short	UPDN_TBL[][4] = {
							{ 0,	16,	0,	UPDN_UP },
							{ 0,	16,	1,	UPDN_DN },
};

// ����۰��ð���
const	unsigned short	UPLORD_TBL[][4] = {
							{ 0,	18,	0,	LD_LOGO },
							{ 0,	22,	1,	LD_HDR },
							{ 0,	22,	2,	LD_FTR },
							{ 0,	22,	3,	LD_PARA },
							{ 0,	22,	4,	LD_LOCKPARA },
							{ 0,	16,	5,	LD_SLOG },
							{ 0,	16,	6,	LD_TLOG },
							{ 0,	24,	7,	LD_ACCEPTFTR },
							{ 0,	22,	8,	LD_SYOM },
							{ 0,	24,	9,	LD_KAME },
							{ 0,	24,	10,	LD_EDYAT },
							{ 0,	18,11,	LD_USERDEF },
							{ 0,	16,12,	LD_LOG_DATA },
							{ 0,	14,13,	LD_CAR_INFO },
							{ 0,	22,14,	LD_TCARD_FTR },
							{ 0,	20,15,	LD_AZU_FTR },
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
							{ 0,	20,16,	LD_CREKBR_FTR },
							{ 0,	20,17,	LD_EPAYKBR_FTR },
							{ 0,	20,18,	LD_FUTURE_FTR },
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
};

// �޳�۰��ð���
const	unsigned short	DWLORD_TBL[][4] = {
// �����[�U��`���ǉ����ꂽ���ʁA�ԍ��Q���ɂȂ�A���]�����ς�����B
							{ 0,	18,	0,	LD_LOGO },
							{ 0,	22,	1,	LD_HDR },
							{ 0,	22,	2,	LD_FTR },
							{ 0,	22,	3,	LD_PARA },
							{ 0,	22,	4,	LD_LOCKPARA },
							{ 0,	24,	5,	LD_ACCEPTFTR },
							{ 0,	22,	6,	LD_SYOM },
							{ 0,	24,	7,	LD_KAME },
							{ 0,	24,	8,	LD_EDYAT },
							{ 0,	18,	9,	LD_USERDEF },
							{ 0,	14,10,	LD_CAR_INFO },
							{ 0,	22,11,	LD_TCARD_FTR },
							{ 0,	20,12,	LD_AZU_FTR },
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
							{ 0,	20,13,	LD_CREKBR_FTR },
							{ 0,	20,14,	LD_EPAYKBR_FTR },
							{ 0,	20,15,	LD_FUTURE_FTR },
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
};

const	unsigned short	UPLOAD_LOG_TBL[][4] = {
							{ 0,	16,	0,	LD_ERR },
							{ 0,	18,	1,	LD_ARM },
							{ 0,	16,	2,	LD_MONI },
							{ 0,	20,	3,	LD_OPE_MONI },
							{ 0,	26,	4,	LD_PAY_CAL },
};

const	char	UPLD_MAX = (char)(sizeof(UPLORD_TBL)/sizeof(UPLORD_TBL[0]));
const	char	DNLD_MAX = (char)(sizeof(DWLORD_TBL)/sizeof(DWLORD_TBL[0]));

const	unsigned short	BKRS_TBL[][4] = {
							{ 0,	24,	0,	BKRS_BK },
							{ 0,	20,	1,	BKRS_RS },
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							{ 0,	16,	2,	BKRS_FLSTS },
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
};

/* Function check 90-110 */
const	unsigned short	FUN_CHK_TBL[][4] = {
							{ 0,	22,	__swt_check,		SWT_CHK },			/* : �X�C�b�`�`�F�b�N		*/
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//						    { 0,	22,	__kbd_check,		KBD_CHK },			/* : �L�[���̓`�F�b�N		*/
//							{ 0,	20,	__lcd_check,		LCD_CHK },			/* : �k�b�c�`�F�b�N		*/
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	20,	__led_check,		LED_CHK },			/* : �k�d�c�`�F�b�N		*/
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							{ 0,	24,	__sht_check,		SHT_CHK },			/* : �V���b�^�[�`�F�b�N	*/
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	24,	__sig_check,		SIG_CHK },			/* : ���o�͐M���`�F�b�N	*/
							{ 0,	26,	__cmc_check,		CMC_CHK },			/* : �R�C�����b�N�`�F�b�N	*/
							{ 0,	26,	__bna_check,		BNA_CHK },			/* : �������[�_�[�`�F�b�N	*/
							{ 0,	22,	__prt_check,		PRT_CHK },			/* : �v�����^�`�F�b�N		*/
// MH810100(S) Y.Yamauchi 2019/10/08 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							{ 0,	26,	__rdr_check,		RDR_CHK },			/* : ���C���[�_�[�`�F�b�N	*/
							{ 0,	26,	__qr_check,			QR_CHK },			/* : �p�q���[�_�[�`�F�b�N*/
// MH810100(E) Y.Yamauchi 2019/10/08 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	24,	__ann_check,		ANN_CHK },			/* : �A�i�E���X�`�F�b�N	*/
							{ 0,	22,	__mem_check,		MEM_CHK },			/* : �������[�`�F�b�N		*/
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//						    { 0,	24,	__lck_check,		LCK_CHK },			/* : ���b�NIF�Ճ`�F�b�N	*/
//							{ 0,	26,	__flp_check,		FLP_CHK },			/* : �t���b�v���u�`�F�b�N	*/
//							{ 0,	30,	__crr_check,		CRR_CHK },			/* : �t���b�v�����`�F�b�N	*/
//							{ 0,	30,	__sca_check,		SUICA_CHK },		/* : ��ʌnIC���[�_�[�`�F�b�N*/
//							{ 0,	30,	__pip_check,		PIP_CHK },			/* : Park i Pro�`�F�b�N		*/
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	24,	__lan_connect_chk,	LAN_CONNECT_CHK },	/* : �k�`�m�ڑ��`�F�b�N	*/
							{ 0,	26,	__funope_check,		FUNOPE_CHK },		/* : �e�`�m�`�F�b�N	*/
							{ 0,	26,	__centercom_check,	CCM_CHK },			/* : �Z���^�[�ʐM�`�F�b�N	*/
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//						    { 0,	26,	__cre_check,		CRE_CHK },			/* : �N���W�b�g�`�F�b�N	*/
							{ 0,	30,	__realtime_check,	REAL_CHK },			/* : ���A���^�C���ʐM�`�F�b�N	*/
							{ 0,	30,	__dc_net_check,		DC_CHK },			/* : �c�b�|�m�d�s�ʐM�`�F�b�N	*/
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	24,	__ver_check,		VER_CHK },			/* : �o�[�W�����`�F�b�N	*/
};

// MH321800(S) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�`�F�b�N�֕ύX)
const	unsigned short	FUN_CHK_TBL2[][4] = {
							{ 0,	22,	__swt_check2,		SWT_CHK },			// : �X�C�b�`�`�F�b�N
// MH810100(S) K.Onodera 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							{ 0,	22,	__kbd_check,		KBD_CHK },			// : �L�[���̓`�F�b�N
//							{ 0,	20,	__lcd_check,		LCD_CHK },			// : �k�b�c�`�F�b�N
// MH810100(E) K.Onodera 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	20,	__led_check2,		LED_CHK },			// : �k�d�c�`�F�b�N
// MH810100(S) K.Onodera 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							{ 0,	24,	__sht_check,		SHT_CHK },			// : �V���b�^�[�`�F�b�N
// MH810100(E) K.Onodera 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	24,	__sig_check2,		SIG_CHK },			// : ���o�͐M���`�F�b�N
							{ 0,	26,	__cmc_check2,		CMC_CHK },			// : �R�C�����b�N�`�F�b�N
							{ 0,	26,	__bna_check2,		BNA_CHK },			// : �������[�_�[�`�F�b�N
							{ 0,	22,	__prt_check2,		PRT_CHK },			// : �v�����^�`�F�b�N
// MH810100(S) K.Onodera 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							{ 0,	26,	__rdr_check,		RDR_CHK },			// : ���C���[�_�[�`�F�b�N
							{ 0,	26,	__qr_check2,			QR_CHK },			/* : �p�q���[�_�[�`�F�b�N*/
// MH810100(E) K.Onodera 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	24,	__ann_check2,		ANN_CHK },			// : �A�i�E���X�`�F�b�N
							{ 0,	22,	__mem_check2,		MEM_CHK },			// : �������[�`�F�b�N
// MH810100(S) K.Onodera 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							{ 0,	24,	__lck_check,		LCK_CHK },			// : ���b�NIF�Ճ`�F�b�N
//							{ 0,	26,	__flp_check,		FLP_CHK },			// : �t���b�v���u�`�F�b�N
//							{ 0,	30,	__crr_check,		CRR_CHK },			// : �t���b�v�����`�F�b�N
// MH810100(E) K.Onodera 2019/10/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	24,	__sca_check2,		ECR_CHK },			// : ���σ��[�_�`�F�b�N
// MH810100(S) Y.Yamauchi 2020/1/29 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							{ 0,	30,	__pip_check2,		PIP_CHK },			// : Park i Pro�`�F�b�N
// MH810100(E) Y.Yamauchi 2020/1/29 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	24,	__lan_connect_chk2,	LAN_CONNECT_CHK },	// : �k�`�m�ڑ��`�F�b�N
							{ 0,	26,	__funope_check2,	FUNOPE_CHK },		// : �e�`�m�`�F�b�N
							{ 0,	26,	__centercom_check2,	CCM_CHK },			// : �Z���^�[�ʐM�`�F�b�N
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							{ 0,	26,	__cre_check,		CRE_CHK },			// : �N���W�b�g�`�F�b�N
							{ 0,	30,	__realtime_check2,	REAL_CHK },			/* : ���A���^�C���ʐM�`�F�b�N	*/
							{ 0,	30,	__dc_net_check2,	DC_CHK },			/* : DC-NET�ʐM�`�F�b�N	*/
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	24,	__ver_check2,		VER_CHK },			// : �o�[�W�����`�F�b�N
};
// MH321800(E) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�`�F�b�N�֕ύX)


/* Function check 121-124 */
const	unsigned short	FLCD_CHK_TBL[][4] = {
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)	
//							{ 0,	22,	0,	FLCD1_CHK },		/*  1: RW Test */
//							{ 0,	20,	1,	FLCD2_CHK },		/*  2: Print Test */
							{ 0,	22,	0,	FLCD2_CHK },		/*  1: Print Test */
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)	
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)	
//							{ 0,	14,	2,	FLCD3_CHK },		/*  3: Status Check */
//							{ 0,	16,	3,	FLCD4_CHK },		/*  4: Version Check */
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)	
};

/* Function check 127-128 */
const	unsigned short	FSIG_CHK_TBL[][4] = {
							{ 0,	22,	0,	FSIG1_CHK },		/*  1: ���o�͐M���`�F�b�N		*/
							{ 0,	26,	1,	FSIG2_CHK },		/*  2: ���o�͐M�������`�F�b�N	*/
};

/* Function check 131-134 */
const	unsigned short	FPRN_CHK_TBL1[][4] = {
							{ 0,	20,	0,	FPRNR_CHK },		/*  1: ���V�[�g�v�����^ 	*/
							{ 0,	22,	1,	FPRNJ_CHK },		/*  2: �W���[�i���v�����^ 	*/
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
							{ 0,	18,	2,	FPRNEJ_CHK },		/*  3: �d�q�W���[�i��		*/
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
};
const	unsigned short	FPRN_CHK_TBL2[][4] = {					// ���V�[�g�v�����^
							{ 0,	14,	0,	FPRN7_CHK },		/*  1: ���V�[�g�e�X�g���	*/
							{ 0,	12,	1,	FPRN2_CHK },		/*  2: ��Ԋm�F			*/
							{ 0,	22,	2,	FPRN5_CHK },		/*  3: ���S�󎚃f�[�^�o�^	*/
};
const	unsigned short	FPRN_CHK_TBL3[][4] = {					// �W���[�i���v�����^
							{ 0,	14,	0,	FPRN3_CHK },		/*  1: �e�X�g��		*/
							{ 0,	12,	1,	FPRN4_CHK },		/*  2: ��Ԋm�F			*/
};
const	unsigned short	FPRN_CHK_TBL4[][4] = {					// ���V�[�g�e�X�g���
							{ 0,	20,	0,	FPRN1_CHK },		/*  1: �e�X�g�p�^�����		*/
							{ 0,	14,	1,	FPRN8_CHK },		/*  2: �̎��؈��			*/
};
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
const	unsigned short	FPRN_CHK_TBL5[][4] = {					// �W���[�i���v�����^
							{ 0,	24,	0,	FPRN_RW_CHK },		//  �P: ���[�h�E���C�g�e�X�g
							{ 0,	18,	1,	FPRN_SD_CHK },		//  �Q: �r�c�J�[�h���
							{ 0,	12,	2,	FPRN4_CHK },		//  �R: ��Ԋm�F
							{ 0,	18,	3,	FPRN_VER_CHK },		//  �S: �o�[�W�����m�F
};
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

/* Function check 141-144 */
const	unsigned short	FRDR_CHK_TBL[][4] = {
							{ 0,	24,	0,	FRD1_CHK },			//  1: ���[�h�E���C�g�e�X�g
							{ 0,	14,	1,	FRD2_CHK },			//  2: �󎚃e�X�g
							{ 0,	12,	2,	FRD3_CHK },			//  3: ��Ԋm�F
							{ 0,	24,	3,	FRD5_CHK },			//  4: �f�B�b�v�X�C�b�`�m�F
							{ 0,	18,	4,	FRD4_CHK },			//  5: �o�[�W�����m�F
};

const	unsigned short	FCMC_CHK_TBL[][4] = {
							{ 0,	14,	0,	FCM1_CHK },			// 1: �����e�X�g
							{ 0,	16,	1,	FCM2_CHK },			// 2: ���o���e�X�g
							{ 0,	12,	2,	FCM3_CHK },			// 3: ��Ԋm�F
							{ 0,	18,	3,	FCM4_CHK },			// 4: �o�[�W�����m�F
};

const	unsigned short	FBNA_CHK_TBL[][4] = {
							{ 0,	14,	0,	FBN1_CHK },			//  1: �����e�X�g
							{ 0,	12,	1,	FBN2_CHK },			//  2: ��Ԋm�F
							{ 0,	18,	2,	FBN3_CHK },			//  3: �o�[�W�����m�F
};

/* ۸�̧���ƭ� 171-183 */
// GG124100(S) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�����e�i���X��\��)
// ��LOG_FILE_TBL��ύX����ꍇ��LOG_FILE_TBL_CC�������Ƀ����e�i���X����
// GG124100(E) R.Endo 2022/06/02 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�(�����e�i���X��\��)
const	unsigned short	LOG_FILE_TBL[][4] = {
							{ 0,	16,	0,	ERR_LOG },			/*  1: �G���[��� 			*/
							{ 0,	18,	1,	ARM_LOG },			/*  2: �A���[����� 		*/
							{ 0,	16,	2,	TGOU_LOG },			/*  3: �s���v��� 			*/
							{ 0,	18,	3,	GTGOU_LOG },		/*  4: �f�s���v��� 		*/
							{ 0,	20,	4,	COIN_LOG },			/*  5: �R�C�����ɏ�� 		*/
							{ 0,	18,	5,	NOTE_LOG },			/*  6: �������ɏ�� 		*/
							{ 0,	18,	6,	TURI_LOG },			/*  7: �ޑK�Ǘ���� 		*/

							{ 0,	18,	7,	SEISAN_LOG },		/*  8: �ʐ��Z��� 		*/							
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							{ 0,	24,	8,	FUSKYO_LOG },		/*  9: �s���E�����o�ɏ�� 	*/
//							{ 0,	14,	9,	OPE_LOG },			/* 10: ������ 			*/
//							{ 0,	16,	10,	TEIFUK_LOG },		/* 11: �╜�d��� 			*/
//							{ 0,	16,	11,	MONI_LOG },			/* 12: ���j�^���			*/
//							{ 0,	18,	12,	ADDPRM_LOG },		/* 13: �ݒ�X�V���			*/
//							{ 0,	20,	13,	RTPAY_LOG },		/* 14: �����e�X�g���		*/
//							{ 0,	18,	14,	RMON_LOG },			/* 15: ���u�Ď����			*/
//							{ 0,	24,	15,	CLEAR_LOG },		/* 16: ���O�t�@�C���N���A	*/
							{ 0,	14,	8,	OPE_LOG },			/*  9: ������ 			*/
							{ 0,	16,	9,	TEIFUK_LOG },		/* 10: �╜�d��� 			*/
							{ 0,	16,	10,	MONI_LOG },			/* 11: ���j�^���			*/
							{ 0,	18,	11,	ADDPRM_LOG },		/* 12: �ݒ�X�V���			*/
							{ 0,	20,	12,	RTPAY_LOG },		/* 13: �����e�X�g���		*/
							{ 0,	18,	13,	RMON_LOG },			/* 14: ���u�Ď����			*/
							{ 0,	24,	14,	CLEAR_LOG },		/* 15: ���O�t�@�C���N���A	*/
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)							
};

/* Cell Parameter Setting 220-224 */
const	unsigned short	CPARA_SET_TBL[][4] = {
							{ 0,	20,	0,	CARP_PARA },		/* Cell Parameter */
							{ 0,	12,	1,	CARP_PRNT },		/* Print All Parameter */
							{ 0,	20,	2,	CARP_DEFA },		/* Default All Parameter */
};

/* Rock Parameter Setting 230-234 */
const	unsigned short	RPARA_SET_TBL[][4] = {
							{ 0,	26,	0,	RCKP_PARA },		/* Rockr Parameter */
							{ 0,	12,	1,	RCKP_PRNT },		/* Print All Parameter */
							{ 0,	20,	2,	RCKP_DEFA },		/* Default All Parameter */
};

const	unsigned short	USM_MNC_TBL1[][4] = {
							{ 0,	12,	0,	MNT_MNCNT },		/*  1: �ޑK��[				*/
							{ 0,	16,	1,	MNT_IVTRY },		/*  2: �C���x���g��			*/
							{ 0,	12,	2,	MNT_MNYBOX },		/*  3: ���Ɋm�F				*/
};

const	unsigned short	USM_MNC_TBL2[][4] = {
							{ 0,	12,	0,	MNT_MNCNT },		/*  1: �ޑK��[				*/
							{ 0,	16,	1,	MNT_IVTRY },		/*  2: �C���x���g��			*/
							{ 0,	12,	2,	MNT_MNYBOX },		/*  3: ���Ɋm�F				*/
							{ 0,	12,	3,	MNT_MNYCHG },		/*  4: �ޑK����				*/
							{ 0,	24,	4,	MNT_MNYPRI },		/*  5: �ޑK�Ǘ����v�v�����g	*/
};

const	unsigned short	USM_MNS_TBL[][4] = {
							{ 0,	12,	0,	MNT_MNYLIM },		/* 1:Limit money setting */
							{ 0,	18,	1,	MNT_MNYTIC },		/* 2:Service ticket setting */
							{ 0,	10,	2,	MNT_MNYSHP },		/* 3:Shop setting */
							{ 0,	16,	3,	MNT_MNYTAX },		/* 4:Tax setting */
							{ 0,	16,	4,	MNT_MNYKID },		/* 5:A-L kind setting */
							{ 0,	18,	5,	MNT_MNYCHA },		/* 6:Charge setting */
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//�@MH810101 Takei(S) �t�F�[�Y2�@����Ή� 2021/02/16
						    	{ 0,	16,	6,	MNT_MNYTWK },		/* 7:Ticket disable week setting */
							{ 0,	12,	7,	MNT_PRISET },		/* 8:Print of the rate setting */
//  MH810101 Takei(DEL) �t�F�[�Y2�@����Ή� 2021/02/16	{ 0,	12,	6,	MNT_PRISET },		/* 7:Print of the rate setting */
//�@MH810101 Takei(E) �t�F�[�Y2�@����Ή� 2021/02/16
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
};


const	unsigned short	USM_LOG_TBL[][4] = {
							{ 0,	14,	0,	MNT_NGLOG  },		/* 1:�g�p�J�[�h */
							{ 0,	10,	1,	MNT_IOLOG  },		/* 2:���o�� */
							{ 0,	18,	2,	FUSKYO_LOG },		/* 3:�s���E�����o�� */
};
const	unsigned short	MNT_STRCHK_TBL[][4] = {
							{ 0,	28,	0,	MNTLOG1_CHK },		// 1: 
};

//----------------------------------------------------------------------------------------------------------
//�����̃��j���[�ƈႢ�A�g���@�\�̂ݐݒ�ɂ�胁�j���[���ڂ��ςƂȂ邽�߃e�[�u�����\�z���Ȃ���
//���g���@�\���j���[�𑝂₷�ꍇ�́AUSM_EXT_MAX, EXTENDMENU_BASE, MENU_NUMBER, USM_EXT_TBL���ꏏ�ɑ��₷����
//��USM_EXT_TBL[][2]�͔ԍ���U��Ȃ������߈Ӗ��������Ȃ�
//----------------------------------------------------------------------------------------------------------
unsigned char	Ext_Menu_Max;									/* ���j���[�\�����ڐ��i�ݒ�ɂ��ρj*/
unsigned char	USM_ExtendMENU[USM_EXT_MAX][31];				/* �g���@�\ Menu �i�\�z�p�j*/
unsigned short	USM_ExtendMENU_TBL[USM_EXT_MAX][4];				/* �g���@�\ Menu TBL�i�\�z�p�j*/

const	unsigned short	USM_EXT_TBL[][4] = {
// MH810100(S) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
//							{ 0,	16,	0,	MNT_FTOTL },		/* :�����䐔�W�v */
//							{ 0,	18,	0,	MNT_CREDIT },		/* :�N���W�b�g���� */
//							{ 0,	14,	0,	MNT_EDY },			/* :�d�������� */
//							{ 0,	18,	0,	MNT_SCA },			/* :�d�q�}�l�[���� */
//							{ 0,	14,	0,	MNT_HOJIN},			/* �@�l�J�[�h */
// MH810100(E) Y.Yamauchi 2019/10/07 �Ԕԃ`�P�b�g���X(�����e�i���X)
							{ 0,	12,	0,	MNT_MTOTL },		/* :�l�s�W�v */
							{ 0,	20,	0,	MNT_DLOCK},			/* �d�����b�N���� */
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
							{ 0,	18,	0,	MNT_ECR },			/* :���σ��[�_���� */
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
							{ 0,	18,	0,	MNT_EJ },			/* :�d�q�W���[�i�� */
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
};
const	unsigned short	USM_EXTCRE_TBL[][4] = {
							{ 0,	22,	0,	MNT_CREUSE    },	/* 1:�N���W�b�g���p����*/
							{ 0,	12,	1,	MNT_CRECONECT },	/* 2:�ڑ��m�F */
							{ 0,	24,	2,	MNT_CREUNSEND },	/* 3:�����M����˗��f�[�^ */
							{ 0,	18,	3,	MNT_CRESALENG },	/* 4:���㋑�ۃf�[�^ */
};

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//const	unsigned short	USM_EXTEDY_TBL[][4] = {
//							{ 0,	20,	0,	MNT_EDY_ARM    },	/* 1:�A���[�������� */
//							{ 0,	20,	1,	MNT_EDY_CENT   },	/* 2:�Z���^�[�ʐM�J�n */
//							{ 0,	16,	2,	MNT_EDY_SHIME  },	/* 3:���ߋL�^��� */
//							{ 0,	20,	3,	MNT_EDY_SHUUKEI},	/* 4:�����W�v�v�����g */
//							{ 0,	20,	4,	MNT_EDY_MEISAI },	/* 5:���p���׃v�����g */
//							{ 0,	18,	5,	MNT_EDY_TEST   },	/* 6:�d�����M�e�X�g */
//};
//
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
const	unsigned short	USM_EXTCRE_CAPPI_TBL[][4] = {
							{ 0,	22,	0,	MNT_CREUSE    },		/* 1:�N���W�b�g���p����*/
							{ 0,	24,	1,	MNT_CREUPDATE },		/* 2:�J�[�h���f�[�^�v��*/
};

const	unsigned short	USM_LBSET_TBL[][4] = {
							{ 0,	16,	0,	MNT_LBPAPER },		/* 1:�p���c�ʐݒ�			*/
							{ 0,	26,	1,	MNT_LBPAY },		/* 2:���x�����s�s���̐ݒ�	*/
							{ 0,	14,	2,	MNT_LBTESTP },		/* 3:�e�X�g��				*/
};

const	unsigned short	USM_LOCK_TBL[][4] = {
							{ 0,	28,	0,	MNT_FLAPUD 		},	/* 1.�t���b�v�㏸���~�i�ʁj	*/
							{ 0,	28,	1,	MNT_FLAPUD_ALL	},	/* 2.�t���b�v�㏸���~�i�S�āj	*/
							{ 0,	26,	2,	MNT_BIKLCK		},	/* 3.���փ��b�N�J�i�ʁj		*/
							{ 0,	26,	3,	MNT_BIKLCK_ALL	},	/* 4.���փ��b�N�J�i�S�āj		*/
};

const	unsigned short	USM_KIND_TBL[][4] = {
							{ 0,	8,	0,	MNT_CAR },		/* 1.����					*/
							{ 0,	8,	1,	MNT_BIK },		/* 2.����					*/
};

const	unsigned short	USM_KIND3_TBL[][4] = {
							{ 0,	8,	0,	MNT_INT_CAR },	/* 1.����					*/
							{ 0,   14,	1,	MNT_CAR },		/* 2.���ԁiIF-3000/3150�j	*/
							{ 0,	8,	2,	MNT_BIK },		/* 3.����					*/
};

const	unsigned short	USM_CERTI_TBL[][4] = {
							{ 0,	12,	0,	MNT_CERTI_SET },		// �ݒ�
							{ 0,	10,	1,	MNT_CERTI_ISSU },		// ���s
};
const	char			USM_CERTI_TBL_MAX = (char)(sizeof(USM_CERTI_TBL)/sizeof(USM_CERTI_TBL[0]));

const	unsigned short	MNT_SEL_TBL[][4] = {
							{ 0,	24,	0,	USR_MNT },			/* User Maintenance */
							{ 0,	24,	1,	SYS_MNT },			/* System Maintenance */
};

const	unsigned short	CARD_ISU_TBL[][4] = {
							{ 0,	8,	0,	CARD_PASS },		/* 1:��� */
							{ 0,	14,	1,	CARD_KKRI },		/* 2:�W���J�[�h */
};

const	unsigned short	FUNC_CNT_TBL1[][4] = {
							{ 0,	16,	0,	FCNT_FUNC },		/* 1.����J�E���g 				*/
							{ 0,	28,	1,	FCNT_FLAP },		/* 2.�t���b�v���u����J�E���g 	*/
};

const	unsigned short	FUNC_CNT_TBL2[][4] = {
							{ 0,	16,	0,	FCNT_FUNC },		/* 1.����J�E���g 				*/
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)							
//							 { 0,	28,	1,	FCNT_FLAP },		/* 2.�t���b�v���u����J�E���g 	*/
//							 { 0,	26,	2,	FCNT_ROCK },		/* 3.���b�N���u����J�E���g 	*/
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)							
};

const	unsigned short	MIFARE_CHK_TBL[][4] = {
							{ 0,	14,	0,	CHK_COMTST},		/* �ʐM�e�X�g */
							{ 0,	24,	1,	CHK_REDLITTST},		/* ���[�h�E���C�g�e�X�g  */
							{ 0,	24,	2,	CHK_CADPECRNUMRED},	/* �J�[�h�ŗL�ԍ����[�h  */
							{ 0,	22,	3,	CHK_VERCHK},		/* �o�[�W�����`�F�b�N  */
};
const	unsigned short	CCOM_CHK_TBL[][4] = {
							{ 0,	22,	0,	CCOM_DTCLR},		/* �����M�f�[�^�N���A */
							{ 0,	20,	1,	CCOM_FLSH},			/* �����M�f�[�^���M   */
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)						
//							{ 0,	22,	2,	CCOM_TEST},			/* �Z���^�[�ʐM�e�X�g */
//							 { 0,	26,	3,	CCOM_APN},			/* �ڑ���`�o�m�m�F */
//							 { 0,	20,	4,	CCOM_ATN},			/* �d�g��M�󋵊m�F */
//							{ 0,	22,	5,	CCOM_OIBANCLR},		/* �Z���^�[�ǔԃN���A */							
//							{ 0,	24,	6,	CCOM_REMOTE_DL},	/* ���u�����e�i���X�m�F */
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)						
							{ 0,	22,	2,	CCOM_OIBANCLR},		/* �Z���^�[�ǔԃN���A */							
							{ 0,	24,	3,	CCOM_REMOTE_DL},	/* ���u�����e�i���X�m�F */
// MH810100(S) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
//// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
//							 { 0,	22,	7,	CCOM_LONG_PARK_ALLREL},	/* �������ԑS�������M */
//// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
// MH810100(E) Y.Yamauchi 2019/10/03 �Ԕԃ`�P�b�g���X(�����e�i���X)
};

const	unsigned short	CCOM_CHK2_TBL[][4] = {
							{ 0,	22,	0,	CCOM_APN_AMS},		/* �`�l�r�p�`�o�m */
							{ 0,	22,	1,	CCOM_APN_AMN},		/* �A�}�m�p�`�o�m */
							{ 0,	24,	2,	CCOM_APN_PWEB},		/* ���ԏ�Z���^�[�p�`�o�m */
};

const	unsigned short	PIP_CHK_TBL[][4] = {
							{ 0,	16,	0,	PIP_DTCLR},			/* �f�[�^�N���A		  */
};

// MH810100(S) Y.Yamauchi 2019/10/08 �Ԕԃ`�P�b�g���X(�����e�i���X)
const	unsigned short	QR_CHK_TBL[][4] = {
							{ 0,	14,	0,	QR_READ},		/* �ǎ�e�X�g�@�@�@ */
							{ 0,	18,	1,	QR_VER},		/* �o�[�W�����m�F�@*/
};
const	unsigned short	REAL_CHK_TBL[][4] = {
							{ 0,	30,	0,	UNSEND_REAL_CLEAR},		/* �����M�f�[�^�N���A */
							{ 0,	30,	1,	CENTER_REAL_TEST},		/* �Z���^�[�ʐM�e�X�g */
							{ 0,	22,	2,	REAL_OIBANCLR},			/* �Z���^�[�ǔԃN���A */
};
const	unsigned short	DCNET_CHK_TBL[][4] = {
							{ 0,	30,	0,	DC_UNSEND_CLEAR},		/* �����M�f�[�^�N���A */
							{ 0,	30,	1,	DC_CONNECT_TEST},		/* �Z���^�[�ʐM�e�X�g */
							{ 0,	22,	2,	DC_OIBANCLR},			/* �Z���^�[�ǔԃN���A */
};
// MH810100(E) Y.Yamauchi 2019/10/08 �Ԕԃ`�P�b�g���X(�����e�i���X)

const	unsigned short	CRE_CHK_TBL[][4] = {
							{ 0,	20,	0,	CREDIT_UNSEND},		/* �����M����f�[�^ */
							{ 0,	18,	1,	CREDIT_SALENG},		/* ���㋑�ۃf�[�^ */
};

const	unsigned short	FSUICA_CHK_TBL[][4] = {
							{ 0,	14,	0,	FISMF1_CHK },		// 1: �ʐM�e�X�g
							{ 0,	20,	1,	FISMF2_CHK },		// 2: �ʐM���O�v�����g
							{ 0,	28,	2,	FISMF3_CHK },		// 3: �ʐM���O�v�����g�i�ُ�j
							{ 0,	20,	3,	MNT_SCA_SHUUKEI},	/* 4: �����W�v�v�����g */
							{ 0,	20,	4,	MNT_SCA_MEISAI },	/* 5: ���p���׃v�����g */
};
const	unsigned short	FSUICA_CHK_TBL2[][4] = {
							{ 0,	28,	0,	MNT_SUICALOG_INJI },		// 1: �ʐM���O�v�����g(�S���O) 
							{ 0,	30,	1,	MNT_SUICALOG_INJI2 },		// 2: �ʐM���O�v�����g(���߃��O)
};

// MH321800(S) D.Inaba IC�N���W�b�g�Ή�(���σ��[�_�`�F�b�N�ǉ�)
const	unsigned short	FECR_CHK_TBL[][4] = {
							{ 0,	14,	0,	FISMF1_CHK },		// 1: �ʐM�e�X�g
							{ 0,	20,	1,	FISMF2_CHK },		// 2: �ʐM���O�v�����g
							{ 0,	28,	2,	FISMF3_CHK },		// 3: �ʐM���O�v�����g�i�ُ�j
							{ 0,	20,	3,	ECR_BRAND_COND},    // 4: �u�����h��Ԋm�F
							{ 0,	22,	4,	ECR_MNT    },		// 5: ���[�_�����e�i���X
};
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�(���σ��[�_�`�F�b�N�ǉ�)

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//const	unsigned short	FEDY_CHK_TBL[][4] = {
//							{ 0,	14,	0,	FEDY1_CHK },		// 1: �t�h�e�X�g
//							{ 0,	22,	1,	FEDY2_CHK },		// 2: �J�[�h���[�h�e�X�g
//							{ 0,	28,	2,	FEDY3_CHK },		// 3: �ݒ聕�X�e�[�^�X�v�����g
//							{ 0,	16,	3,	FEDY4_CHK },		// 4: ����ʐM���s
//							{ 0,	22,	4,	FEDY5_CHK },		// 5: �σ^���p�[�`�F�b�N
//							{ 0,	20,	5,	MNT_EDY_SHUUKEI},	// 6: �����W�v�v�����g
//							{ 0,	20,	6,	MNT_EDY_MEISAI },	// 7: ���p���׃v�����g
//};
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

const	unsigned short	USM_EXTSCA_TBL[][4] = {
							{ 0,	20,	0,	MNT_SCA_SHUUKEI},	/* 1:�����W�v�v�����g */
							{ 0,	20,	1,	MNT_SCA_MEISAI },	/* 2:���p���׃v�����g */
};

// MH321800(S) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�������j���[)
const	unsigned short	USM_EXTEC_TBL[][4] = {
							{ 0,	20,	0,	MNT_ECR_SHUUKEI},			/* 1:�����W�v�v�����g	*/
							{ 0,	20,	1,	MNT_ECR_MEISAI },			/* 2:���p���׃v�����g	*/
							{ 0,	20,	2,	MNT_ECR_ALARM_LOG},			/* 3:������������L�^	*/
// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
							{ 0,	22,	3,	MNT_ECR_MINASHI},			/* 4:�݂Ȃ����σv�����g	*/
// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
};
// MH321800(E) D.Inaba IC�N���W�b�g�Ή� (���σ��[�_�������j���[)

const	unsigned short	FUNC_FLP_TBL[][4] = {
							{ 0,   16,	0,	MNT_FLP_LOOP_DATA_CHK },	/* 1.���[�v�f�[�^			*/
							{ 0,   24,	1,	MNT_FLP_SENSOR_CTRL },		/* 2.�ԗ����m�Z���T�[����	*/
};
const	unsigned short	FUNC_CRR_TBL[][4] = {
							{ 0,   28,	0,	MNT_CRR_VER },				/* 1.�b�q�q�o�[�W�����`�F�b�N	*/
							{ 0,   28,	1,	MNT_CRR_TST },				/* 2.�b�q�q�܂�Ԃ��H��e�X�g	*/
};

const	unsigned short	FUNC_MAFCHK_TBL[][4] = {
							{ 0,   24,	0,	MNT_MAF_RISM_COMCHK },	/* �P�DRism�T�[�o�[�ڑ��m�F */
							{ 0,   22,	1,	MNT_WCARD_INF },		/* �Q�D�J�[�h�ǉ��ݒ���@ */
							{ 0,   28,	2,	MNT_PARAM_LOG},			/* �R�D�����ݒ�ʐM���O�v�����g */
};                                                          
const	unsigned short	FUNC_CAPPICHK_TBL[][4] = {
							{ 0,   16,	0,	MNT_CAPPI_RESET },		/* �P�D�ǔԃ��Z�b�g�@�@�@�@	*/
							{ 0,   25,	1,	MNT_CAPPI_COMCHK },		/* �Q�DCappi�T�[�o�[�ڑ��m�F */
};

const	unsigned short	FUNC_FTPCONNECTCHK_TBL[][4] = {
							{ 0,   12,	0,	FTP_CONNECT_FUNC },	/* �P�D�ڑ��m�F			*/
							{ 0,   26,	1,	FTP_PARAM_UPL },	/* �Q�D�p�����[�^�A�b�v���[�h		*/
};

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
const	unsigned short	USM_EXTEJ_TBL[][4] = {
							{ 0,	20,	0,	MNT_EJ_SD_INF},				/* �P�D�r�c�J�[�h���	*/
};
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

char	DP_CP[2];							/* 0:Cursor Possion ( 0�`2 )				*/
											/* 1:Item �� displayed in the first line	*/

short	gCurSrtPara;	// �Ԏ����ށi����/���ցj

// ���ް��\���p
union	CrDspRec	cr_dsp;

// ���샍�O�o�^�p
ushort	OpelogNo;
ushort	OpelogNo2;
uchar	MovCntClr;			// ���춳�ĸر���s�L���iOFF=���s���^ON=�L�j

// �ݒ�FLASH���������p
ushort	SetChange;

unsigned char	*fck_chk_adr;		// ������ذ���ڽ�i�����q�`�l��ذ�����p�j
unsigned char	fck_chk_err;		// ��������		�i�����q�`�l��ذ�����p�j
unsigned char	fck_sav_data;		// �ް��ޔ�ر	�i�����q�`�l��ذ�����p�j
unsigned char	fck_chk_data1;		// �����ް��P	�i�����q�`�l��ذ�����p�j
unsigned char	fck_chk_data2;		// �����ް��Q	�i�����q�`�l��ذ�����p�j

// �Z�p�Ҍ����ł͐ݒ�֎~�Ƃ���A�h���X���w��
const	PRM_MASK param_mask[] = {
// MH810100(S) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
//	{10005,10010},{10014,10038},{10042,10045},{10048,10050},{10052,10052},{10057,10059},{10064,10064},{10069,10069},{10073,10100},
//	{20001,20001},{20003,20009},{20015,20015},{20018,20020},{20022,20023},{20026,20026},{20035,20038},{20040,20040},{20044,20100},
//// MH322914(S) K.Onodera 2016/12/07 ���~���b�N��Ԃŏo�ɂ����ꍇ�A�ݒ�ɂăt���b�v���~�����g���C����[���ʃo�ONo.1336](GW872707)
////	{30001,30041},{30046,30046},{30048,30061},{30064,30067},{30070,30079},{30081,30081},{30083,30096},{30098,30098},{30101,30117},{30119,30120},{30123,30132},{30134,30150},
//// MH322916(S) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
////	{30001,30041},{30046,30046},{30048,30061},{30064,30067},{30070,30079},{30081,30081},{30083,30096},{30098,30098},{30101,30117},{30120,30120},{30123,30132},{30134,30150},
//// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
////	{30001,30041},{30046,30046},{30048,30061},{30064,30067},{30070,30079},{30081,30081},{30083,30096},{30098,30098},{30101,30117},{30120,30120},{30123,30132},{30134,30134},{30136,30150},
//	{30001,30041},{30046,30046},{30048,30061},{30064,30067},{30070,30071},{30073,30079},{30081,30081},{30083,30096},{30098,30098},{30101,30117},{30120,30120},{30123,30132},{30134,30134},{30137,30150},
//// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
//// MH322916(E) A.Iiizumi 2018/05/16 �������Ԍ��o�@�\�Ή�
//// MH322914(E) K.Onodera 2016/12/07 ���~���b�N��Ԃŏo�ɂ����ꍇ�A�ݒ�ɂăt���b�v���~�����g���C����[���ʃo�ONo.1336](GW872707)
// MH810100(S) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
//	{10005,10012},{10014,10039},{10042,10045},{10048,10050},{10052,10052},{10057,10059},{10064,10064},{10069,10070},{10073,10074},{10079,10100},
// MH810105(S) MH364301 �C���{�C�X�Ή�
//	{10005,10012},{10014,10039},{10042,10045},{10048,10050},{10052,10052},{10057,10059},{10064,10064},{10069,10069},{10072,10074},{10079,10100},
// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// 	{10005,10012},{10014,10015},{10017,10039},{10042,10045},{10048,10050},{10052,10052},{10057,10059},{10064,10064},{10069,10069},{10072,10074},{10079,10100},
	{10005,10012},{10014,10015},{10017,10039},{10042,10045},{10048,10050},{10052,10052},{10057,10059},{10061,10064},{10069,10069},{10072,10074},{10079,10100},
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// MH810105(E) MH364301 �C���{�C�X�Ή�
// MH810100(E) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
// MH810100(S) K.Onodera 2020/03/27 �Ԕԃ`�P�b�g���X(#4094 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
//	{20001,20015},{20018,20020},{20022,20023},{20026,20026},{20035,20038},{20040,20100},
//	{30001,30067},{30069,30150},

// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
///	{20001,20001},{20003,20015},{20018,20020},{20022,20023},{20026,20026},{20033,20038},{20040,20100},
	{20001,20001},{20003,20015},{20018,20020},{20023,20023},{20026,20026},{20033,20038},{20040,20100},
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j

// MH810103(S) R.Endo 2021/06/03 �Ԕԃ`�P�b�g���X �t�F�[�Y2.2 #5692 ���Z�����ē��^�C�}�[�ݒ�̃}�X�N���O��
//	{30001,30150},
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i���Z�������̉�ʕ\����ǉ�����j�iGM803003���p�j
//	{30001,30127},{30129,30150},
	{30001,30127},{30130,30150},
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i���Z�������̉�ʕ\����ǉ�����j�iGM803003���p�j
// MH810103(E) R.Endo 2021/06/03 �Ԕԃ`�P�b�g���X �t�F�[�Y2.2 #5692 ���Z�����ē��^�C�}�[�ݒ�̃}�X�N���O��
// MH810100(E) K.Onodera 2020/03/27 �Ԕԃ`�P�b�g���X(#4094 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
// MH810100(E) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j

	{40007,40008},{40040,40040},{40048,40048},{40068,40080},

// MH810100(S) K.Onodera 2020/03/27 �Ԕԃ`�P�b�g���X(#4094 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
//// MH322914 (s) kasiyama 2016/07/12 GT7000�ɍ��킹�Ċ����؂�ԋ߈ē����ϓ����Ƃ���(���ʉ��PNo.1212)(MH341106)
////	{50001,50008},{50011,50011},{50013,50030},
//	{50001,50008},{50011,50011},{50014,50030},
//// MH322914 (e) kasiyama 2016/07/12 GT7000�ɍ��킹�Ċ����؂�ԋ߈ē����ϓ����Ƃ���(���ʉ��PNo.1212)(MH341106)


//�@MH810101 Takei(S) �t�F�[�Y2�@����Ή� 2021/02/15
//	{50009,50030},
	{50001,50011},{50014,50030},
//�@MH810101 Takei(E) �t�F�[�Y2�@����Ή� 2021/02/15
	
// MH810100(E) K.Onodera 2020/03/27 �Ԕԃ`�P�b�g���X(#4094 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)

// MH810100(S) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
//	{60005,60006},{60011,60012},{60017,60018},{60023,60024},{60029,60030},{60035,60036},{60041,60042},{60047,60048},{60053,60054},{60059,60060},
//	{60065,60066},{60071,60075},
	{60002,60002},{60005,60006},{60008,60008},{60011,60012},{60014,60014},{60017,60018},{60020,60020},{60023,60024},{60026,60026},{60029,60030},
	{60032,60032},{60035,60036},{60038,60038},{60041,60042},{60044,60044},{60047,60048},{60050,60050},{60053,60054},{60056,60056},{60059,60060},
	{60062,60062},{60065,60066},{60068,60068},{60071,60075},
// MH810100(E) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)

// MH810100(S) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
//	{70003,70003},{70008,70008},{70013,70013},{70018,70018},{70023,70023},{70028,70028},{70033,70033},{70038,70038},{70043,70043},{70048,70048},
//	{70053,70053},{70058,70058},{70063,70063},{70068,70068},{70073,70073},{70078,70078},{70083,70083},{70088,70088},{70093,70093},{70098,70098},
//	{70103,70103},{70108,70108},{70113,70113},{70118,70118},{70123,70123},{70128,70128},{70133,70133},{70138,70138},{70143,70143},{70148,70148},
// MH810100(S) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
//	{70003,70004},{70008,70008},{70013,70014},{70018,70018},{70023,70024},{70028,70028},{70033,70034},{70038,70038},{70043,70044},{70048,70048},
//	{70053,70054},{70058,70058},{70063,70064},{70068,70068},{70073,70074},{70078,70078},{70083,70084},{70088,70088},{70093,70094},{70098,70098},
//	{70103,70104},{70108,70108},{70113,70114},{70118,70118},{70123,70124},{70128,70128},{70133,70134},{70138,70138},{70143,70144},{70148,70148},
	{70002,70004},{70008,70008},{70012,70014},{70018,70018},{70022,70024},{70028,70028},{70032,70034},{70038,70038},{70042,70044},{70048,70048},
	{70052,70054},{70058,70058},{70062,70064},{70068,70068},{70072,70074},{70078,70078},{70082,70084},{70088,70088},{70092,70094},{70098,70098},
	{70102,70104},{70108,70108},{70112,70114},{70118,70118},{70122,70124},{70128,70128},{70132,70134},{70138,70138},{70142,70144},{70148,70148},
// MH810100(E) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
// MH810100(E) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j

// MH810100(S) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
//	{80003,80004},{80006,80007},{80026,80027},
	{80002,80004},{80006,80007},{80026,80027},
// MH810100(E) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j

	{90046,90075},{90111,90140},

// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(���X�܊����ݒ�̃}�X�N����)
//	{110001,110186},
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(���X�܊����ݒ�̃}�X�N����)

	{120001,120301},

// MH810100(S) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
//// MH322914 (s) kasiyama 2016/07/12 GT7000�ɍ��킹�Ċ����؂�ԋ߈ē����ϓ����Ƃ���(���ʉ��PNo.1212)(MH341106)�񐔌������؂�\��
////	{130012,130015},
//	{130012,130023},
//// MH322914 (e) kasiyama 2016/07/12 GT7000�ɍ��킹�Ċ����؂�ԋ߈ē����ϓ����Ƃ���(���ʉ��PNo.1212)(MH341106)�񐔌������؂�\��
	{130001,130024},
// MH810100(E) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j

// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//// MH810100(S) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
////	{140002,140004},{140007,140007},{140011,140011},{140013,140013},{140015,140015},{140018,140025},
//	{140002,140004},{140007,140007},{140011,140011},{140013,140015},{140018,140025},
//// MH810100(E) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
	{140002,140004},{140011,140011},{140013,140015},{140018,140025},
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

	{160051,160051},{160055,160058},{160060,160060},

// GG129000(S) M.Fujikawa 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR���Ԍ��̃p�����[�^�ݒ�
//	{170003,170003},{170054,170060},
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// 	{170003,170003},{170054,170055},{170058,170060},
	{170003,170003},{170054,170055},{170059,170060},
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// GG129000(E) M.Fujikawa 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR���Ԍ��̃p�����[�^�ݒ�

// MH810100(S) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
//// MH322914 (S) S.Fujii 2017/03/30 AI-V�Ή�
////	{180013,180013},{180030,180034},{180040,180050},
//// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
////	{180013,180013},{180030,180034},{180040,180050},{180061,180065},{180071,180100},
//	{180013,180013},{180030,180032},{180034,180034},{180040,180050},{180061,180065},{180071,180100},
//// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
//// MH322914 (E) S.Fujii 2017/03/30 AI-V�Ή�
// MH810100(S) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
//	{180013,180013},{180016,180017},{180030,180034},{180040,180050},{180061,180065},{180071,180100},
// MH810105(S) MH364301 �C���{�C�X�Ή�
//	{180013,180013},{180016,180017},{180030,180034},{180040,180065},{180071,180100},
	{180013,180013},{180016,180017},{180030,180034},{180040,180065},{180079,180100},
// MH810105(E) MH364301 �C���{�C�X�Ή�
// MH810100(E) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
// MH810100(E) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j	

	{190001,190006},{190017,190027},{190029,190030},

	{200005,200008},{200011,200014},{200017,200020},{200023,200026},{200028,200029},{200031,200032},{200034,200052},{200054,200055},{200057,200065},

	{210015,210032},

	{220002,220013},

	{230001,230030},

// MH810100(S) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
//	{240002,240003},{240005,240017},{240019,240019},{240021,240029},{240036,240040},
	{240001,240040},
// MH810100(E) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j

// MH810100(S) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
	{250397,250400},
// MH810100(E) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//	{260001,260005},
	{260002,260002},{260006,260020},
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

// MH810100(S) S.Fujii 2020/06/25 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
	{270021,270100},
// MH810100(E) S.Fujii 2020/06/25 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)

// GG124100(S) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�
// 	{280003,280010},{280012,280014},{280018,280018},{280021,280021},{280029,280029},{280031,280034},{280036,280037},{280040,280040},{280042,280055},
	{280003,280010},{280012,280014},{280018,280018},{280021,280021},{280028,280029},{280031,280034},{280036,280037},{280040,280040},{280042,280055},
// GG124100(E) R.Endo 2022/08/22 �Ԕԃ`�P�b�g���X3.0 #6343 �N���E�h�����v�Z�Ή�

// MH810100(S) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
//	{300091,300150},
// MH810102(S) R.Endo 2021/04/16 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5509 ���ԑђ���֘A�ݒ�l�̃}�X�N���O��
//	{300001,300150},
// MH810102(E) R.Endo 2021/04/16 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5509 ���ԑђ���֘A�ݒ�l�̃}�X�N���O��
// MH810100(E) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j

	{320001,320240},

	{330001,330100},

// MH810100(S) K.Onodera 2020/03/27 �Ԕԃ`�P�b�g���X(#4094 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
//	{340001,340024},{340027,340036},{340038,340060},{340069,340069},{340119,340119},{340123,340150},
// MH810100(S) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
//	{340001,340024},{340027,340036},{340038,340060},{340069,340094},{340119,340120},{340123,340150},
	{340001,340024},{340027,340036},{340038,340060},{340069,340094},{340119,340121},{340123,340150},
// MH810100(E) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
// MH810100(E) K.Onodera 2020/03/27 �Ԕԃ`�P�b�g���X(#4094 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)

// MH810100(S) S.Fujii 2020/06/23 �Ԕԃ`�P�b�g���X�i�d�q���ϒ[���Ή��j
//	{350001,350005},{350008,350010},{350012,350099},
	{350001,350100},
// MH810100(E) S.Fujii 2020/06/23 �Ԕԃ`�P�b�g���X�i�d�q���ϒ[���Ή��j

// MH810100(S) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
//// MH322914(S) K.Onodera 2017/02/03 �ʐM�菇�f�t�H���g�ύX[���ʃo�ONo.1353]
////// �d�l�ύX(S) K.Onodera 2016/11/04 �U�֐��Z���̗̎��؎������s�̐ݒ��݂���
//////	{360001,360011},{360013,360031},{360033,360050},{360055,360061},{360077,360078},{360097,360100},
////	{360001,360011},{360013,360031},{360035,360039},{360041,360050},{360055,360061},{360077,360078},{360097,360100},
////// �d�l�ύX(E) K.Onodera 2016/11/04 �U�֐��Z���̗̎��؎������s�̐ݒ��݂���
//// MH322917(S) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
////	{360001,360011},{360013,360031},{360035,360039},{360041,360044},{360046,360050},{360055,360061},{360077,360077},{360097,360100},
//	{360001,360011},{360013,360031},{360035,360039},{360041,360044},{360046,360050},{360055,360061},{360077,360077},{360098,360100},
//// MH322917(E) A.Iiizumi 2018/08/30 ���A���^�C����� �t�H�[�}�b�gRev11�Ή�
//// MH322914(E) K.Onodera 2017/02/03 �ʐM�菇�f�t�H���g�ύX[���ʃo�ONo.1353]
// MH810100(S) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
//	{360001,360011},{360013,360031},{360035,360039},{360041,360044},{360046,360050},{360052,360061},{360074,360074},{360077,360077},{360079,360095},{360098,360100},
	{360001,360011},{360013,360033},{360035,360039},{360041,360044},{360046,360050},{360052,360061},{360065,360066},{360074,360074},{360077,360077},{360079,360095},{360098,360100},
// MH810100(E) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
// MH810100(E) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j

	{370001,370100},

// MH810100(S) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
//	{380001,380001},{380004,380004},{380006,380020},{380027,380064},{380068,380100},
// MH810100(S) S.Fujii 2020/06/23 �Ԕԃ`�P�b�g���X�i�d�q���ϒ[���Ή��j
//	{380001,380100},
	{380001,380001},{380004,380100},
// MH810100(E) S.Fujii 2020/06/23 �Ԕԃ`�P�b�g���X�i�d�q���ϒ[���Ή��j
// MH810100(E) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
	{390001,390200},

	{400001,400100},

	{410008,410092},{410095,410100},

	{420001,420100},

	{430001,430010},

	{440001,440010},

	{450001,450050},

// MH810100(S) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
//	{460003,460010},{460023,460030},{460043,460050},{460063,460070},{460075,460200},
// MH810100(S) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
//	{460003,460070},{460075,460200},
	{460001,460200},
// MH810100(E) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
// MH810100(E) K.Onodera 2019/12/03 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j

	{470001,470100},

// MH810100(S) K.Onodera 2019/11/26 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
//	{480001,480100},
// MH810100(S) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
//	{480006,480006},{480008,480009},{480018,480019},{480026,480029},{480031,480100},
	{480006,480006},{480008,480009},{480018,480019},{480026,480100},
// MH810100(E) Y.Yoshida 2020/06/04 �Ԕԃ`�P�b�g���X(#4109 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
// MH810100(E) K.Onodera 2019/11/26 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j

// GG120600(S) // Phase9 #6186 �Z�p������Ńp�����[�^�̃Z�N�V����49��ύX���邱�Ƃ��ł��Ȃ�
//	{490001,490100},
	{490011,490100},
// GG120600(E) // Phase9 #6186 �Z�p������Ńp�����[�^�̃Z�N�V����49��ύX���邱�Ƃ��ł��Ȃ�

// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//	{500001,500100},
// MH810103 GG119202(S) �݂Ȃ����ψ������̓���
//	{500005,500005},{500008,500009},{500014,500019},{500029,500100},
// MH810103 GG119202(S) �d�q�}�l�[���ώ��̌��ό��ʎ�M�҂��^�C�}�ǉ�
//	{500005,500005},{500008,500009},{500016,500019},{500029,500100},
// MH810103 MHUT40XX(S) Edy�EWAON�Ή�
//	{500005,500005},{500008,500009},{500017,500019},{500029,500100},
// MH810105 GG119202(S) T���v�A���󎚑Ή�
//	{500005,500005},{500007,500009},{500017,500026},{500029,500040},{500046,500100},
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//	{500007,500009},{500017,500026},{500029,500040},{500046,500100},
	{500007,500009},{500017,500026},{500029,500040},{500048,500049},{500053,500100},
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105 GG119202(S) T���v�A���󎚑Ή�
// MH810103 MHUT40XX(E) Edy�EWAON�Ή�
// MH810103 GG119202(E) �d�q�}�l�[���ώ��̌��ό��ʎ�M�҂��^�C�}�ǉ�
// MH810103 GG119202(E) �݂Ȃ����ψ������̓���
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�

	{510001,520020},

	{520001,520050},

	{530001,530100},

	{540001,540100},

	{550001,550100},

	{560006,560019},{560044,560100},

// MH810100(S) K.Onodera 2019/11/26 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j
//	{570181,570300},
// MH810102(S) R.Endo 2021/04/16 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5509 ���ԑђ���֘A�ݒ�l�̃}�X�N���O��
//	{570001,570300},
// MH810102(E) R.Endo 2021/04/16 �Ԕԃ`�P�b�g���X �t�F�[�Y2.5 #5509 ���ԑђ���֘A�ݒ�l�̃}�X�N���O��
// MH810100(E) K.Onodera 2019/11/26 �Ԕԃ`�P�b�g���X�i�ݒ茩�����j

	{580028,580060},{580328,580360},{580628,580660},

	{590028,590060},{590328,590360},{590628,590660},

// MH810100(S) K.Onodera 2020/01/21 �Ԕԃ`�P�b�g���X�iLCD�ݒ�j
//	{600001,600100},
	{600001,600010},{600016,600016},{600028,600029},{600034,600035},{600039,600039},{600042,600042},{600044,600050},
	{600056,600056},{600058,600059},{600064,600064},{600067,600079},
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� #6430 �x�ƈē��\���̎d�l�ύX(�x�ƈē��ɖ₢���킹��\������)�Ή�
//// MH810100(S) K.Onodera 2020/03/27 �Ԕԃ`�P�b�g���X(#4094 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
////	{600082,600089},{600091,600100},{600107,600110},{600112,600113},{600118,600120},
////	{600121,600125},{600127,600127},{600139,600140},{600144,600150},{600161,600170},{600185,600200},
//// MH810100(S) S.Nishimoto 2020/08/27 �Ԕԃ`�P�b�g���X(#4766 ������� �u�o��ς݂̎ԗ��𐸎Z�v�{�^�������ʃp�����[�^�Ő؂�ւ��\�Ƃ��Ăق���)
////	{600082,600089},{600092,600100},{600108,600110},{600112,600113},{600118,600120},
//// MH810104(S) R.Endo 2021/09/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5997 �����Z�p���Z���i���b�Z�[�W���\�����ꂽ��A�u�̎����v�{�^���ɑ����āu�͂��v�{�^���������ƃG���[���b�Z�[�W���\�����ꐸ�Z���ł��Ȃ��B
//// 	{600082,600089},{600092,600100},{600108,600110},{600113,600113},{600118,600120},
//	{600082,600089},{600092,600100},{600108,600110},{600113,600113},{600119,600120},
//// MH810104(E) R.Endo 2021/09/30 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5997 �����Z�p���Z���i���b�Z�[�W���\�����ꂽ��A�u�̎����v�{�^���ɑ����āu�͂��v�{�^���������ƃG���[���b�Z�[�W���\�����ꐸ�Z���ł��Ȃ��B
//// MH810100(E) S.Nishimoto 2020/08/27 �Ԕԃ`�P�b�g���X(#4766 ������� �u�o��ς݂̎ԗ��𐸎Z�v�{�^�������ʃp�����[�^�Ő؂�ւ��\�Ƃ��Ăق���)
// GG124100(S) W.Hoshino 2022/07/25 �Ԕԃ`�P�b�g���X �t�F�[�Y3.0 #6410 �ʐM�s�ǎ����b�Z�[�W�����ʃp�����[�^�̐ݒ�ɂ��ύX������
// 	{600082,600089},{600092,600092},{600094,600100},{600108,600110},{600113,600113},
// GG129000(S) R.Endo 2022/11/10 �Ԕԃ`�P�b�g���X4.0 #6678 �J���[LCD �������[���[�N(�ǉ��Ή�) [���ʉ��P���� No1544]
// 	{600082,600089},{600092,600092},{600094,600100},{600108,600110},
// GG129000(S) H.Fujinaga 2023/01/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڋq�ē��Ή��j
//	{600082,600089},{600094,600100},{600108,600110},
// MH810107(S) R.Endo 2023/09/27 #7138 �V�p�l���Ή�
// 	{600082,600089},{600095,600100},{600108,600110},
// GG129004(S) M.Fujikawa 2024/10/31 �ҋ@��ʂ̕\��������ύX����
//	{600082,600089},{600095,600100},{600108,600109},
// GG129004(S) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
// 	{600082,600089},{600096,600100},{600108,600109},
// GG132000(S) �������Z�̕\��������ύX����
//	{600082,600089},{600097,600100},{600108,600109},
	{600082,600089},{600098,600100},{600108,600109},
// GG132000(E) �������Z�̕\��������ύX����
// GG129004(E) R.Endo 2024/12/04 �d�q�̎��ؑΉ��iQR�R�[�h���s�ē��\���^�C�}�[�j
// GG129004(E) M.Fujikawa 2024/10/31 �ҋ@��ʂ̕\��������ύX����
// MH810107(E) R.Endo 2023/09/27 #7138 �V�p�l���Ή�
// GG129000(E) H.Fujinaga 2023/01/26 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڋq�ē��Ή��j
// GG129000(E) R.Endo 2022/11/10 �Ԕԃ`�P�b�g���X4.0 #6678 �J���[LCD �������[���[�N(�ǉ��Ή�) [���ʉ��P���� No1544]
// GG124100(E) W.Hoshino 2022/07/25 �Ԕԃ`�P�b�g���X �t�F�[�Y3.0 #6410 �ʐM�s�ǎ����b�Z�[�W�����ʃp�����[�^�̐ݒ�ɂ��ύX������
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� #6430 �x�ƈē��\���̎d�l�ύX(�x�ƈē��ɖ₢���킹��\������)�Ή�
// MH810100(S) S.Nishimoto 2020/09/04 �Ԕԃ`�P�b�g���X(���g�p�ݒ�̒ǉ�)
//	{600121,600125},{600127,600127},{600139,600140},{600141,600200},
// GG129000(S) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
//	{600121,600125},{600127,600127},{600136,600137},{600139,600140},{600141,600200},
// GG129000(S) M.Fujikawa 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR���Ԍ��̃p�����[�^�ݒ�
//	{600121,600125},{600127,600127},{600136,600137},{600141,600200},
	{600121,600125},{600127,600127},{600136,600137},{600139,600139},{600141,600200},
// GG129000(E) M.Fujikawa 2023/09/28 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR���Ԍ��̃p�����[�^�ݒ�
// GG129000(E) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
// MH810100(E) S.Nishimoto 2020/09/04 �Ԕԃ`�P�b�g���X(���g�p�ݒ�̒ǉ�)
// MH810100(E) K.Onodera 2020/03/27 �Ԕԃ`�P�b�g���X(#4094 �}�X�N���ڂ����ʃp�����[�^�d�l���ƈقȂ��Ă���)
// MH810100(E) K.Onodera 2020/01/21 �Ԕԃ`�P�b�g���X�iLCD�ݒ�j
// MH810104(S) R.Endo 2021/09/25 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6014 QR�t�H�[�}�b�g�p�^�[���ݒ�̃}�X�N���O��
	{600202,600208},{600210,600210},{600227,600230},{600252,600252},{600269,600290},{600292,600292},{600294,600294},{600298,600298},{600300,600300},{600304,600304},{600306,600306},{600310,600340},{600368,600400},
	{600402,600408},{600410,600410},{600427,600430},{600452,600452},{600469,600490},{600492,600492},{600494,600494},{600498,600498},{600500,600500},{600504,600504},{600506,600506},{600510,600540},{600568,600600},
	{600602,600608},{600610,600610},{600627,600630},{600652,600652},{600669,600690},{600692,600692},{600694,600694},{600698,600698},{600700,600700},{600704,600704},{600706,600706},{600710,600740},{600768,600800},
// MH810104(E) R.Endo 2021/09/25 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #6014 QR�t�H�[�}�b�g�p�^�[���ݒ�̃}�X�N���O��
};

const	PRM_MASK param_mask31_0[] = {
	//�����
	{310901,310901},
};

const	PRM_MASK param_mask31_1[] = {
	//������
	{310896,310900},
};

const	short	prm_mask_max = (sizeof(param_mask) / sizeof(PRM_MASK));	// �}�X�N�f�[�^����
const	short	prm_mask31_0_max = (sizeof(param_mask31_0) / sizeof(PRM_MASK));	// �}�X�N�f�[�^�����F�����
const	short	prm_mask31_1_max = (sizeof(param_mask31_1) / sizeof(PRM_MASK));	// �}�X�N�f�[�^�����F������
/*--------------------------------------------------------------------------*/
/*	Common function of Maintenance											*/
/*--------------------------------------------------------------------------*/

/* Role of Short Cut Key to Function Key *///��
unsigned short	StoF( unsigned short keyid, char mode ){

	unsigned short	sw;
	sw = keyid;
	return sw;
}

/*[]----------------------------------------------------------------------[]*/
/*| Menu Select                                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Menu_Slt                                                |*/
/*| PARAMETER    :                                                         |*/
/*| RETURN VALUE : short    ret;    0x00fe : F5 Key ( End Key )            |*/
/*|                                 0x00fe : Maintenance KEY               |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : B.Miyamoto                                              |*/
/*| Date         : 2002-10-04                                              |*/
/*| Update       : Y.Tanaka 2005-04-04                                     |*/
/*|              : ART:ogura 2005-07-12 F3�ɂ���߰�ސ؊�                  |*/
/*|              :           2005-07-20 modify                             |*/
/*[]------------------------------------- Copyright(C) 2002 AMANO Corp.---[]*/
unsigned short	Menu_Slt( const unsigned char (*d_str)[31], const unsigned short (*d_tbl)[4], char cnt, char mod )
{
	ushort	msg;
	char	i;
	short	wk;

	/* ���ڐ����U��葽���Ƃ��́AF3��"��"��ǉ����� */
	if (cnt > 6) {
		Fun_Dsp( FUNMSG[5] );		/* "�@���@�@���@�@���@ �Ǐo  �I�� " */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
	}else if( cnt == 1 ){
		Fun_Dsp( FUNMSG[38] );		/* "                   �Ǐo  �I�� " */
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
	}else {
		Fun_Dsp( FUNMSG[25] );		/* "�@���@�@���@�@�@�@ �Ǐo  �I�� " */
	}

	for (i = 0; i < 6; i++ ){
		if (DP_CP[0]+i >= cnt)
			break;
		/* ���j���[���ڕ\�� */
		grachr((ushort)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, d_str[d_tbl[i+DP_CP[0]][2]]);
	}
	/* �I�����ڔ��] */
	grachr((ushort)(DP_CP[1]-DP_CP[0]+1), d_tbl[DP_CP[1]][0], d_tbl[DP_CP[1]][1], 1, COLOR_BLACK, LCD_BLINK_OFF, &d_str[d_tbl[DP_CP[1]][2]][d_tbl[DP_CP[1]][0]]);

	wk = 0;
	for( ;; ){
// MH810100(S) S.Nishimoto 2020/08/25 �Ԕԃ`�P�b�g���X(#4750 ���ʃp�����[�^�̐ݒ�ύX��Ƀh�A�����Ă������e�i���X��ʂ̂܂�)
		if (CP_MODECHG) {
			// ���łɃh�A��(�ݒ胂�[�h�I��)�̏ꍇ�́A���[�h�ύX��ԋp����
			BUZPI();
			return MOD_CHG;
		}
// MH810100(E) S.Nishimoto 2020/08/25 �Ԕԃ`�P�b�g���X(#4750 ���ʃp�����[�^�̐ݒ�ύX��Ƀh�A�����Ă������e�i���X��ʂ̂܂�)

		msg = StoF( GetMessage(), 1 );
		switch (KEY_TEN0to9(msg)) {
	/* mode change */
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case LCD_DISCONNECT:
				return MOD_CUT;
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			case KEY_MODECHG:
			BUZPI();
			return MOD_CHG;
	/* number key */
		case KEY_TEN:
			if (cnt < 10){
				if (( msg >= KEY_TEN1 ) && ( msg <= ( cnt + KEY_TEN0 ))) {
					BUZPI();
					wk = (short)( msg - KEY_TEN0 );
				}
			}
			else {
				BUZPI();
				if(cnt < (wk * 10 + msg - KEY_TEN0)){
					wk = 0;
				}
				wk = (short)((wk*10 +  msg-KEY_TEN0) % 100);
			}
			/* �L���͈͊O�܂��́A�J�[�\���ړ����� */
			if ((wk < 1) || (wk > cnt) || (DP_CP[1] == wk-1))
				continue;
			DP_CP[1] = (char)(wk - 1);
			DP_CP[0] = (char)(DP_CP[1] / 6 * 6);
			break;
	/* F1:�� */
		case KEY_TEN_F1:
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			// ���ڂ�1�̏ꍇ�͖���
			if( cnt <= 1 ){
				continue;
			}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			BUZPI();
			if (DP_CP[1] <= 0) {
				DP_CP[1] = (char)(cnt - 1);
				DP_CP[0] = (char)(DP_CP[1] / 6 * 6);
			}
			else {
				DP_CP[1]--;
				if (DP_CP[1] < DP_CP[0]) {
					DP_CP[0] -= 6;
				}
			}
			wk = 0;
			break;
	/* F2:�� */
		case KEY_TEN_F2:
// MH810100(S) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			// ���ڂ�1�̏ꍇ�͖���
			if( cnt <= 1 ){
				continue;
			}
// MH810100(E) Y.Yamauchi 2019/12/16 �Ԕԃ`�P�b�g���X(�����e�i���X)
			BUZPI();
			if (DP_CP[1] >= cnt-1) {
				DP_CP[1] = DP_CP[0] = 0;
			}
			else {
				DP_CP[1]++;
				if (DP_CP[1] >= DP_CP[0]+6) {
					DP_CP[0] += 6;
				}
			}
			wk = 0;
			break;
	/* F3:�� */
		case KEY_TEN_F3:
			/* ���ڐ����U�ȉ��̏ꍇ��F3���������Ȃ� */
			if (cnt <= 6)
				continue;
			BUZPI();
			if ((DP_CP[0]+6) >= cnt) {
				DP_CP[0] = 0;
			}
			else {
				DP_CP[0] += 6;
			}
			DP_CP[1] = DP_CP[0];
			break;
	/* F4:�Ǐo */
		case KEY_TEN_F4:
			BUZPI();
			return d_tbl[DP_CP[1]][3];
	/* F5:�I�� */
		case KEY_TEN_F5:
			BUZPI();
			return MOD_EXT;

		default:
			continue;
		}

		for (i = 0; i < 6; i++ ){
			if (DP_CP[0]+i >= cnt)
				break;
			/* ���j���[���ڕ\�� */
			grachr((ushort)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, d_str[d_tbl[i+DP_CP[0]][2]]);
		}
		for ( ; i < 6; i++) {
			grachr((ushort)(i+1), 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, UMSTR3[16]);		/* �s�N���A */
		}
		/* �I�����ڔ��] */
		grachr((ushort)(DP_CP[1]-DP_CP[0]+1), d_tbl[DP_CP[1]][0], d_tbl[DP_CP[1]][1], 1, COLOR_BLACK, LCD_BLINK_OFF, &d_str[d_tbl[DP_CP[1]][2]][d_tbl[DP_CP[1]][0]]);
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Page control                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : pag_ctl( void )                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Update       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/

char	pag_ctl( unsigned short ind, unsigned short pos, unsigned short *top )
{
	unsigned short	topb;
	char ret;
	ret	= 0;
	topb = *top;

	if( pos > ind - 1 )
		pos = ind;
	*top = pos - pos % 5;

	ret = ( (unsigned short)((topb != *top) ) ? 1 : 0);
	return ret;

}

