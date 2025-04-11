/*[]----------------------------------------------------------------------[]*/
/*| headder file for common suica                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : T.Namioka                                                |*/
/*| DATE        : 2006-07-07                                               |*/
/*| UPDATE      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
#ifndef _SUICA_DEF_H_
#define _SUICA_DEF_H_

// MH321800(S) G.So IC�N���W�b�g�Ή�
#include	"ec_def.h"
#include	"system.h"
// MH321800(E) G.So IC�N���W�b�g�Ή�

#define	S_BUF_MAXSIZE		256		// ����M�ޯ̧����
#define TBL_CNT(a) (sizeof(a) / sizeof(a[0]))		/* ���R�[�h�����Z�o�p�}�N�� */

// MH321800(S) G.So IC�N���W�b�g�Ή�
//#define	SUICA_USE_ERR	(prm_get(COM_PRM, S_PAY, 24, 1, 3) != 1 || \
//						Suica_Rec.suica_err_event.BYTE || \
//						Suica_Rec.Data.BIT.MIRYO_TIMEOUT  )	
#define	SUICA_USE_ERR	(jvma_setup.type == 0 || \
						Suica_Rec.suica_err_event.BYTE || \
						Suica_Rec.Data.BIT.MIRYO_TIMEOUT || \
						!Suica_Rec.Data.BIT.INITIALIZE)
#define	isSX10_USE()			(jvma_setup.type == 1)		// SX-10�g�p�ݒ�(jvma_setup.type == 2�͊g��)
#define	isEC_USE()				(jvma_setup.type == 4)		// ���σ��[�_�g�p�ݒ�
#define	isEMoneyReader()		(jvma_setup.type != 0)		// �d�q�}�l�[�J�[�h���[�_�[�ڑ�����
#define	WAR_MONEY_CHECK			(!Suica_Rec.Data.BIT.MONEY_IN && !ryo_buf.nyukin )
#define	OPE_SUICA_MIRYO_TIME	prm_get(COM_PRM, S_SCA, 13, 3, 1)	// 255
#define	ENABLE_MULTISETTLE()	get_enable_multisettle() 	// �������ω\�����f
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
#define	isEC_MSG_DISP()			(isEC_USE() && ECCTL.ec_MessagePtnNum != 0)		// ���σ��[�_�֘A���b�Z�[�W�\����
#define	isEC_REMOVE_MSG_DISP()	(isEC_USE() && ECCTL.ec_MessagePtnNum == 4)		// �J�[�h������胁�b�Z�[�W�\����
#define	isEC_NG_MSG_DISP()		(isEC_USE() && ECCTL.ec_MessagePtnNum == 98)	// ����NG���b�Z�[�W�\����
#define	isEC_STS_CARD_PROC()	(isEC_USE() && \
								 ECCTL.Ec_Settlement_Sts == EC_SETT_STS_CARD_PROC)	// �J�[�h������
#define	isEC_STS_MIRYO()		(isEC_USE() && \
								 ECCTL.Ec_Settlement_Sts == EC_SETT_STS_MIRYO)	// ������
#define	isEC_CTRL_ENABLE()		(isEC_USE() && Suica_Rec.Data.BIT.CTRL && \
								 !SUICA_USE_ERR)								// ���σ��[�_����t��
#define	isEC_CARD_INSERT()		(isEC_USE() && Suica_Rec.Data.BIT.CTRL_CARD)	// ���σ��[�_�ɃJ�[�h����
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
// MH810103 GG119202(S) �����d�l�ύX�Ή�
#define	isEC_CONF_MIRYO()		(isEC_USE() && Suica_Rec.Data.BIT.MIRYO_CONFIRM)// [0x1B]EC��ԃf�[�^(�����m��)��M�ς�
// MH810103 GG119202(E) �����d�l�ύX�Ή�
// GG116202(S) JVMA���Z�b�g�񐔐����Ή�
#define	hasEC_JVMA_RESET_CNT_EXCEEDED	(isEC_USE() && Ec_Jvma_Reset_Count > EC_JVMA_RESET_CNT)		// JVMA���Z�b�g���g���C�񐔉z��
#define	isEC_JVMA_RESET_CNT_LESS_THAN	(isEC_USE() && Ec_Jvma_Reset_Count < EC_JVMA_RESET_CNT)		// JVMA���Z�b�g���g���C�񐔖���
// GG116202(E) JVMA���Z�b�g�񐔐����Ή�
// MH810103 GG119202(S) �����c���Ɖ�^�C���A�E�g���Ă���10���o�߂Ő���f�[�^�i��t���j�����M�����
#define	isEC_MIRYO_TIMEOUT()	(isEC_USE() && Suica_Rec.Data.BIT.MIRYO_TIMEOUT)// �����c���Ɖ�^�C���A�E�g
// MH810103 GG119202(E) �����c���Ɖ�^�C���A�E�g���Ă���10���o�߂Ő���f�[�^�i��t���j�����M�����
// MH810103 GG119202(S) �u�����h��Ԕ��菈���ύX
#define	isEC_BRAND_STS_KAIKYOKU(x)	(isEC_USE() && (((x) & 0x03) == 0x01))		// �u�����h��Ԃ��J�ǁ��L���ł���΁A�J�ǂƂ��Ĉ���
// MH810103 GG119202(E) �u�����h��Ԕ��菈���ύX
// MH810103 GG119202(S) �T�[�r�X��~���u�����h�̓u�����h�ݒ�f�[�^�ɃZ�b�g���Ȃ�
#define	isEC_BRAND_STS_ENABLE(x)	(isEC_USE() && (((x) & 0x0B) == 0x01))		// �u�����h��Ԃ��J�ǁ��L�����T�[�r�X�ғ����ł���΁A���ω\
// MH810103 GG119202(E) �T�[�r�X��~���u�����h�̓u�����h�ݒ�f�[�^�ɃZ�b�g���Ȃ�
// MH810105(S) MH364301 QR�R�[�h���ϑΉ��i���Z���~�f�[�^���M�Ή��j
#define	isEC_PAY_CANCEL()		(isEMoneyReader() && PayData.Electron_data.Ec.E_Status.BIT.miryo_confirm)
// MH810105(E) MH364301 QR�R�[�h���ϑΉ��i���Z���~�f�[�^���M�Ή��j

#define	S_CNTL_DATA			0x10	// �����ް� 
#define	S_SELECT_DATA		0x11	// �I�����i�ް�
#define	S_INDIVIDUAL_DATA	0x15	// ���ް� 
#define	S_PAY_DATA			0x16	// ���Z�ް�
#define	S_DSP_DATA			0x18	// �\���˗��ް�
#define	S_SETTLEMENT_DATA	0x19	// �����ް�
#define	S_ERR_DATA			0x1A	// �ُ��ް�
#define	S_STATUS_DATA		0x1B	// ����ް�
#define	S_TIME_DATA			0x1F	// ���ԓ����ް�
#define	S_FIX_DATA			0x0D	// �Œ��ް�

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//#define	SUICA_RTRY_COUNT_31	10		/* Retry Count 10	*/
//#define	SUICA_RTRY_COUNT_21	10		/* Retry Count 10	*/
#define	SUICA_RTRY_COUNT_31	31		/* Retry Count 31	*/
#define	SUICA_RTRY_COUNT_21	21		/* Retry Count 21	*/
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
#define	SUICA_RTRY_COUNT_3	3	/* Retry Count 3	*/

#define RESPONSE_TIME_LIMIT 250		// SX-10�����҂�����(��5�b)

#define S_DATA_RCV_TIMEOUT_ERR	0x04	// �ް���M������ѱ�Ĵװ
#define S_NAK_RCV_ERR			0x08	// NAK��M������ײ�װ
#define S_LRC_ERR				0x10	// LRC�װ
// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
//#define S_COM_SND_TIMEOUT_ERR	0x40	// ����ޑ��M��̎�M��ѱ�Ĵװ
#define S_COM_RCV_TIMEOUT_ERR	0x40	// ����ޑ��M��̎�M��ѱ�Ĵװ
#define S_COM_SND_TIMEOUT_ERR	0x80	// ���M��ѱ�Ĵװ
// MH321800(E) T.Nagai IC�N���W�b�g�Ή�

#define	PAY_RYO_MAX	99990
#define	DELAY_MAX	8

enum {
	STANDBY_BEFORE,					// ����޲�O
	STANDBY_SND_AFTER,				// ����޲���M��
	BATCH_SND_AFTER,				// �ꊇ�v�����M��	
	INPUT_SND_AFTER,				// ���͗v�����M��
	OUTPUT_SND_AFTER,				// �o�͎w�ߑ��M��
	REINPUT_SND_AFTER,				// ���͍ėv�����M��
	REOUTPUT_SND_AFTER,				// �o�͍Ďw�ߑ��M��
	DATA_SND_AFTER,					// �ް����M��
	DATA_RCV,						// �ް���M
	MULTI_DATA_RCV_WAIT,			// �����ް��҂�
};

typedef	struct {
	unsigned char		Result;										// ���σf�[�^
	unsigned char		Column_No;									// ���σR�����ԍ�
	unsigned char		Set_kind;									// ���Z���
	long				settlement_data;							// ���ۂ̌��ϊz
	long				settlement_data_before;						// ���Z�OSuica�c��
	long				settlement_data_after;						// ���ό�Suica�c��
	unsigned char		Suica_ID[16];								// Suica ID
} SUICA_SETTLEMENT_RES;

extern	SUICA_SETTLEMENT_RES	Settlement_Res;	// Suica�ް��Ǘ��\����

typedef union{
	unsigned long	ULONG;					// short����long�Ɋg��
	struct{	
// MH321800(S) G.So IC�N���W�b�g�Ή�
//		unsigned char	YOBI2:8;			// Bit 24-31 = �\��2
//		unsigned char	YOBI1:3;			// Bit 21-23 = �\��1
// MH810103 GG119202(S) �N���V�[�P���X��������������
//		unsigned char	YOBI:3;				// Bit 29-31 = �\��
//		unsigned char	MIRYO_CONFIRM:1;	// Bit 28 = �����m���M�t���O
//		unsigned char	BRAND_STS_RCV:1;	// Bit 27 = �u�����h��ԃf�[�^��M�t���O
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//		unsigned char	YOBI:4;				// Bit 28-31 = �\��
		unsigned char	YOBI:2;				// Bit 30-31 = �\��
		unsigned char	BRAND_SEL:1;		// Bit 29 = �u�����h�I�𒆃t���O �u�����h�I���f�[�^(33H)�`�u�����h�I�����ʃf�[�^(36H)�̂Ƃ�1�ɂȂ�B
		unsigned char	SELECT_SND:1;		// Bit 28 = �I�����i�f�[�^���M��
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
		unsigned char	MIRYO_CONFIRM:1;	// Bit 27 = ��ԃf�[�^�̖����m���M�t���O
// MH810103 GG119202(E) �N���V�[�P���X��������������
		unsigned char	RESET_RESERVED:1;	// Bit 26 = �������\��t���O
		unsigned char	SETTLMNT_STS_RCV:1;	// Bit 25 = ���ϒ���M�t���O
// MH810103 GG119202(S) �u�����h�I����̃L�����Z�������ύX
//		unsigned char	SLCT_BRND_BACK:1;	// Bit 24 = �u�����h�I���̖߂錟�o�t���O
// MH810103 GG119202(S) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
//		unsigned char	YOBI2:1;			// Bit 24 = �\��
		unsigned char	BRAND_CAN:1;		// Bit 24 = �u�����h�I�𒆎~�v��
// MH810103 GG119202(E) �u�����h�I���V�[�P���X�ύX�iT�^�@�l�Ή�����ڐA�j
// MH810103 GG119202(E) �u�����h�I����̃L�����Z�������ύX
		unsigned char	CTRL_CARD:1;		// Bit 23 = �J�[�h�������
		unsigned char	PRE_CTRL:1;			// Bit 22 = �O���M��������f�[�^�t���O
		unsigned char	COMM_ERR:1;			// Bit 21 = �ʐM�ُ�Ō��σ��[�_���p�s��
// MH321800(E) G.So IC�N���W�b�g�Ή�
		unsigned char	MIRYO_NO_ANSWER:1;	// Bit 20 = Suica���牞�����Ȃ��A���Z�@�^�C�}�[�Ń^�C���A�E�g���f�����Ƃ��ɃZ�b�g����
		unsigned char	PRI_NG:1;			// Bit 19 = �W���[�i���g�p�s��Suica���p�s��
		unsigned char	MONEY_IN:1;			// Bit 18 = �����C�x���g��M�ς�
		unsigned char	ADJUSTOR_START:1;	// Bit 17 = ���Z�J�n��ԃt���O(���i�I���f�[�^���M�\�t���O)
		unsigned char	SEND_CTRL80:1;		// Bit 16 = ����I���f�[�^���M��ԃt���O
		unsigned char	STATUS_DATA_SPLIT:1;// Bit 15 = ��ԃf�[�^�̕����҂����킹�t���O
		unsigned char	ADJUSTOR_NOW:1;		// Bit 14 = SUICA���Z������t���O
		unsigned char	DATA_RCV_WAIT:1;	// Bit 13 = ��M�f�[�^�҂����킹�t���O
		unsigned char	FUKUDEN_SET:1;		// Bit 12 = ���d���ɐ��Z�f�[�^���M�v���L
		unsigned char	LOG_DATA_SET:1;		// Bit 11 = ���ό��ʃf�[�^����M��A���Z���O�ɓo�^�������ǂ���
		unsigned char	SETTLEMENT_ERR_RCV:1;	// Bit 10 = Ope����������O�Ɍ��ό��ʂ�2�d��M�����׸�
		unsigned char	MIRYO_ARM_DISP:1;	// Bit 9 = Suica����������̃��b�Z�[�W�\��
		unsigned char	MIRYO_TIMEOUT:1;	// Bit 8 = Suica����������^�C���A�E�g
		unsigned char	OPE_CTRL:1;			// Bit 7 = �\��
		unsigned char	PAY_CANSEL:1;		// Bit 6 = �d�q�Ȱ�̐��Z���~���s��ꂽ���ǂ���
		unsigned char	PAY_CTRL:1;			// Bit 5 = ���Z����Suica���Z���s��ꂽ���ǂ���
		unsigned char	RESET_COUNT:1;		// Bit 4 = ؾ���׸�
		unsigned char	CTRL_MIRYO:1;		// Bit 3 = �����������
		unsigned char	CTRL:1;				// Bit 2 = ����f�[�^�t���O
		unsigned char	INITIALIZE:1;		// Bit 1 = �������t���O
		unsigned char	EVENT:1;			// Bit 0 = Suica�C�x���g�t���O
	} BIT;
} t_Suica_Data;

typedef union{
	unsigned char	BYTE;					// Byte
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//	struct{
//		unsigned char	YOBI:2;				// Bit 6-7 = �\��
//// MH321800(S) G.So IC�N���W�b�g�Ή�
////		unsigned char	FIX_DATA:1;			// Bit 5 = �Œ��ް�
////		unsigned char	TIME_DATA:1;		// Bit 4 = ���ԓ����ް�
//		unsigned char	SETTSTS_DATA:1;		// Bit 5 = ���Ϗ���ް�
//		unsigned char	BRAND_DATA:1;		// Bit 4 = �u�����h�I���ް�
//// MH321800(E) G.So IC�N���W�b�g�Ή�
//		unsigned char	STATUS_DATA:1;		// Bit 3 = ����ް�
//// MH321800(S) T.Nagai IC�N���W�b�g�Ή�
////		unsigned char	ERR_DATA:1;			// Bit 2 = �ُ��ް�
//		unsigned char	YOBI2:1;			// Bit 2 = �\��
//// MH321800(E) T.Nagai IC�N���W�b�g�Ή�
//		unsigned char	SETTLEMENT_DATA:1;	// Bit 1 = ���ό����ް�
//		unsigned char	DSP_DATA:1;			// Bit 0 = �\���˗��ް�
//	} BIT;
	struct{
		unsigned char	YOBI:2;				// Bit 6-7 = �\��
		unsigned char	FIX_DATA:1;			// Bit 5 = �Œ��ް�
		unsigned char	TIME_DATA:1;		// Bit 4 = ���ԓ����ް�
		unsigned char	STATUS_DATA:1;		// Bit 3 = ����ް�
		unsigned char	ERR_DATA:1;			// Bit 2 = �ُ��ް�
		unsigned char	SETTLEMENT_DATA:1;	// Bit 1 = ���ό����ް�
		unsigned char	DSP_DATA:1;			// Bit 0 = �\���˗��ް�
	} BIT;
	struct {
		unsigned char	YOBI:4;
		unsigned char	BRAND_DATA:1;		// Bit 3 = �u�����h�I���ް�
		unsigned char	SETTSTS_DATA:1;		// Bit 2 = ���Ϗ���ް�
		unsigned char	STATUS_DATA:1;		// Bit 1 = ����ް�
		unsigned char	SETTLEMENT_DATA:1;	// Bit 0 = ���ό����ް�
	} EC_BIT;
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
} t_Suica_rcv_event;

// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
#define	SETTLEMENT_COMPLETE_COND		0x07	// ���Ϗ�ԃf�[�^�A��ԃf�[�^�A���ό��ʃf�[�^
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������

typedef union{
	unsigned char	BYTE;					// Byte
	struct{
// MH321800(S) G.So IC�N���W�b�g�Ή�
//		unsigned char	YOBI:4;				// Bit 4-7 = �\��
// MH810103 GG119202(S) �J�[�h�������^�C���A�E�g�ł݂Ȃ����ψ����Ƃ���
//		unsigned char	YOBI:1;				// Bit 7 = �\��
		unsigned char	SETT_RCV_FAIL:1;	// Bit 7 = ���ό��ʎ�M���s
// MH810103 GG119202(E) �J�[�h�������^�C���A�E�g�ł݂Ȃ����ψ����Ƃ���
// �����[�_����t�ɂȂ�Ȃ�������ǉ�
		unsigned char	SETTLEMENT_ERR:1;	// Bit 6 = ���ψُ�
		unsigned char	BRAND_NONE:1;		// Bit 5 = �u�����h�ݒ�Ȃ�(�P�ł��g�p�\�Ȃ�OK)
// MH810103 GG119202(S) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
//		unsigned char	BRANDSETUP_NG:1;	// Bit 4 = �u�����h�ݒ���(E3264)
// MH810103 GG119202(S) �ُ�f�[�^�t�H�[�}�b�g�ύX
//		unsigned char	YOBI2:1;			// Bit 4 = �\��
		unsigned char	ERR_RECEIVE2:1;		// Bit 4 = �ُ�f�[�^�i2�o�C�g�ځj��M
// MH810103 GG119202(E) �ُ�f�[�^�t�H�[�}�b�g�ύX
// MH810103 GG119202(E) �g�p�}�l�[�I��ݒ�(50-0001,2)���Q�Ƃ��Ȃ�
// MH321800(E) G.So IC�N���W�b�g�Ή�
		unsigned char	PAY_DATA_ERR:1;		// Bit 3 = ���ϊz�Ə��i�I���f�[�^�Ƃ̊z�ɍ��ق�����
		unsigned char	OPEN:1;				// Bit 2 = �ُ��ް�
		unsigned char	ERR_RECEIVE:1;		// Bit 1 = SX-10�ُ��ް���M
		unsigned char	COMFAIL:1;			// Bit 0 = �ʐM�s��
	} BIT;
} t_Suica_Err_event;
// MH810103 MH321800(S) �؂藣���G���[������̑ҋ@��Ԃ�JVMA���Z�b�g���s��
// MH810103 GG119202(S) �J�[�h�������^�C���A�E�g�ł݂Ȃ����ψ����Ƃ���
//#define	EC_NOT_AVAILABLE_ERR	0x4A		// ���σ��[�_�؂藣���G���[(�ʐM�s�Ǐ���)
#define	EC_NOT_AVAILABLE_ERR	0xCA		// ���σ��[�_�؂藣���G���[(�ʐM�s�Ǐ���)
// MH810103 GG119202(E) �J�[�h�������^�C���A�E�g�ł݂Ȃ����ψ����Ƃ���
// MH810103 MH321800(E) �؂藣���G���[������̑ҋ@��Ԃ�JVMA���Z�b�g���s��
// MH810103 GG119202(S) �N���V�[�P���X��������������

#define EC_BRAND10_STS_ALL_RECV		0x00000003UL
#define EC_BRAND20_STS_ALL_RECV		0x0000000FUL

typedef union{
	unsigned long	ULONG;					// ULONG
	struct{
		unsigned long	YOBI:27;			// Bit 5-31 = �\��
		unsigned long	VOL_RES_RCV:1;		// Bit 4 = �����ύX���ʃf�[�^��M
		unsigned long	BR_STS4_RCV:1;		// Bit 3 = �u�����h��ԃf�[�^�Q��M(2���)
		unsigned long	BR_STS3_RCV:1;		// Bit 2 = �u�����h��ԃf�[�^�Q��M(1���)
		unsigned long	BR_STS2_RCV:1;		// Bit 1 = �u�����h��ԃf�[�^�P��M(2���)
		unsigned long	BR_STS1_RCV:1;		// Bit 0 = �u�����h��ԃf�[�^�P��M(1���)
	} BIT;
} t_Ec_BrandSts_event;

typedef	struct {
	t_Ec_BrandSts_event	brand_rcv_event;	// �u�����h��Ԏ�M����ďڍ�
} t_Ec_negoc_Data;
// MH810103 GG119202(E) �N���V�[�P���X��������������

typedef struct {
	unsigned char	Status;					// ��ԊǗ��ް�
	unsigned char	Com_kind;				// �R�}���h���
	unsigned char	snd_kind;				// �ް����
	unsigned char	faze;					// �t�F�[�Y�Ǘ�
	t_Suica_Data 	Data;					// Suica�p�C�x���g�Ǘ�
	unsigned char	Snd_Buf[S_BUF_MAXSIZE];	// ���M�ް��i�[�̈�
	short	Snd_Size;						// ���M�ް�����
	unsigned char	Rcv_Buf[S_BUF_MAXSIZE];	// ��M�ް��i�[�̈�
	short	Rcv_Size;						// ���M�ް�����
	t_Suica_rcv_event	suica_rcv_event;	// suica��M����ďڍ�
	t_Suica_rcv_event	suica_rcv_event_sumi;	// suica��M����ď����Ϗ��
	t_Suica_Err_event	suica_err_event;	// suicaErr����ďڍ�
	t_Suica_rcv_event	rcv_split_event;	// suica������M����ďڍ�	
// MH810103 GG119202(S) �N���V�[�P���X��������������
	t_Ec_negoc_Data		ec_negoc_data;		// ���σ��[�_ �u�����h�l�S�V�G�[�V�����Ǘ�
// MH810103 GG119202(E) �N���V�[�P���X��������������
} SUICA_REC;

typedef struct {
	unsigned char	snd_kind;				// �ް����
	unsigned char	Snd_Buf[S_BUF_MAXSIZE]; // ���M�ް��ޯ̧
} S_SEND_BUF;

typedef struct {
	unsigned char	write_wpt;			// �������߲��
	unsigned char	read_wpt;			// �ǂݍ����߲��
	S_SEND_BUF		Suica_Snd_q[5];		// ���M�ް����
} SUICA_SEND_BUFF;
typedef struct{
	unsigned char kind;
	unsigned char bc;
	unsigned char dc;
	unsigned char data[1];
}t_RCVBUFINFO,*p_RCVBUFINFO;
extern unsigned char suica_work_buf[S_BUF_MAXSIZE];
extern struct	clk_rec	suica_work_time;
typedef union{
	unsigned char	status_data;			// Byte
	struct{
		unsigned char	YOBI:3;				// Bit 5-7 = �\��
		unsigned char	MiryoStatus:1;		// Bit 4 = ����������ԁi0�F�������Ŗ��� 1�F�����������j
		unsigned char	DetectionSwitch:1;	// Bit 3 = ���߽������m�i0�F�����m��� 1�F���߽������m�j
		unsigned char	Running:1;			// Bit 2 = ���s���i0�F���s���Ŗ��� 1�F���s���j
		unsigned char	TradeCansel:1;		// Bit 1 = �����ݾفi0�F�����ݾقŖ��� 1�F�����ݾَ�t�j
		unsigned char	ReceptStatus:1;		// Bit 0 = ��t��ԁi0�F��t�� 1�F��t�s�j
	} StatusInfo;
} t_STATUS_DATA_INFO;

extern t_STATUS_DATA_INFO	STATUS_DATA_WAIT;		// ��ԃf�[�^(��M�҂��p)

extern	SUICA_REC	Suica_Rec;	// Suica�ް��Ǘ��\����
extern	unsigned char	suica_errst;		/* Communication  Status 		*/

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)�̂��߂̒�`�ړ�
typedef union{
	unsigned char	BYTE;
	struct {
		unsigned char	YOBI				:7;		/* B1-7 = �\�� */
		unsigned char	suica_zangaku_dsp	:1;		/* B0 = Suica�c�z�\���p�����׸� */
	} BIT;
} t_Edy_Dsp_erea;

extern t_Edy_Dsp_erea	edy_dsp;

#define	BUF_MAX_DELAY_TIME			100L		// �ޯ̧MAX���̑��M�����҂�����(1�b)
#define	BUF_MAX_DELAY_COUNT			5			// �ޯ̧MAX���̑��M�����҂���(5��)
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)�̂��߂̒�`�ړ�

extern unsigned long	pay_dsp;							// �\���˗��f�[�^
extern unsigned char	err_data;							// �ُ�f�[�^
extern t_STATUS_DATA_INFO	STATUS_DATA;					// ��ԃf�[�^
// MH321800(S) G.So IC�N���W�b�g�Ή�
extern t_STATUS_DATA_INFO	PRE_STATUS_DATA;				// �O���M������ԃf�[�^
// MH321800(E) G.So IC�N���W�b�g�Ή�
extern struct			clk_rec	time_data;					// ���ԓ����f�[�^
extern unsigned char	fix_data[2];						// �Œ�f�[�^
extern const unsigned char	suica_tbl[][2];
extern unsigned char 	suica_err[5];						/* Communication  Status */
extern unsigned char 	suica_errst;						/* Communication  Status */
extern unsigned char	suica_errst_bak;
extern unsigned char 	Retrycount;							/* Retry���Đ� */
extern unsigned char	dsp_work_buf[2];					// ��ʕ\���p�ꎞ�ޔ�̈�
extern unsigned char	Status_Retry_Count_NG;				// ��t�s���̐����ް����M����
extern unsigned char	Status_Retry_Count_OK;					// ��t�s���̐����ް����M����
extern unsigned char	Ope_Suica_Status;					// Ope������̑��M�ð��
extern	SUICA_SEND_BUFF	Suica_Snd_Buf;	// Suica�ް��Ǘ��\����
extern unsigned char	disp_media_flg;						// ��ʕ\���p�@���Z�}�̎g�p��(0)��(1)
extern unsigned short	DspChangeTime[2];
extern unsigned char	DspWorkerea[7];						// ���Ԍ��}���҂���ʁ@���p�}�̕\��ү����No.

// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//extern unsigned char	dsp_change;
extern unsigned char	dsp_change;						// �����������̉�ʕ\���׸� 0=�����֌W�\���Ȃ� 1=�����֌W�\���� 2=�����֌W�\����(����f�[�^�����c���ʒm�I�����M��)
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
extern unsigned char	dsp_fusoku;						// �c���s���װѕ\�������׸�
extern long	w_settlement;								// Suica�c���s�����̐��Z�z�i�O���͎c���s���łȂ����Ӗ�����j
extern unsigned char	suica_fusiku_flg;				// ���p�\�}�̻��د��\���̒�~�t���O�i0:�\���^1:��~�j
extern unsigned char	mode_Lagtim10;					// Lagtimer10�̎g�p��� 0:���p�\�}�̻��د��\�� 1:Suica��~��̎�t���đ��MWait�I��
extern unsigned short	nyukin_delay[DELAY_MAX];		// ������������ү���ނ�ێ� [0]:���ү�	[1]:����
extern unsigned char	delay_count;					// ������������ү���޶��Đ�
extern unsigned short	wk_media_Type;					// ���Z�}�́i�d�q�}�l�[�j���
extern unsigned	char	time_out;						// ��ʖ߂���ѱ�Ĕ�������̈�
				
extern long				Product_Select_Data;			// ���M�������i�I���f�[�^(���ώ��̔�r�p)
// MH810103 GG119202(S) ���σ��[�_�֘A���菈��������
//// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
//extern unsigned	short	Product_Select_Brand;			// ���i�I���f�[�^���M�̃u�����h�ԍ�
//// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
// MH810103 GG119202(E) ���σ��[�_�֘A���菈��������
/*** function prototype ***/

extern	void	suica_save( unsigned char * ,short );
extern	void	suica_init( void );
extern	void	suica_command_set( void );
extern	void	SUICA_RCLR( void );
extern	void	time_get_rcv( struct clk_rec *sndbuf, void *snddata);
extern	void	time_set_snd( unsigned char *sndbuf, void *snddata );
extern	void	Suica_Data_Snd( unsigned char kind, void *snddata );
extern	void	suicatrb( unsigned char err_kind );
extern	void	Suica_reset( void );
extern  void	snd_ctrl_timeout( void );
extern  void	snd_ctrl_timeout2( void );
extern  void	snd_ctrl_timeout3( void );
extern  void	miryo_timeout( void );

extern	void 	suica_event( void );
extern	short	suicaact( void );
extern	short	suica_recv( unsigned char *, short );
extern  long	settlement_amount( unsigned char *buf );

extern  short	Log_Count_search( unsigned char	search_kind );
extern	short	Ope_Suica_Event( unsigned short	msg, unsigned char ope_faze );
extern	void	Ope_EleUseDsp( void );
extern	short	Ope_TimeOut_10( unsigned char ope_faze, unsigned short  e_pram_set );
extern	void	Ope_TimeOut_11( unsigned char ope_faze, unsigned short  e_pram_set );
extern	char	Ope_ArmClearCheck( unsigned short	msg );
extern	char	Ope_MiryoEventCheck( unsigned short	msg );

extern	void	op_SuicaFusokuOff( void );
extern	void	op_SuicaFusoku_Msg( unsigned long dat, unsigned char *buff, unsigned char use_kind );
extern	void	op_mod01_dsp_sub( void );
extern	void	op_mod01_dsp( void );
extern	void	nyukin_delay_check( unsigned short *postdata, unsigned char count );

extern	char	First_Pay_Dsp( void );
extern	void	Cansel_Ele_Start( void );
extern	void	Op_ReturnLastTimer( void );

extern	unsigned char	Op_StopModuleWait( unsigned char );
extern  unsigned char   Op_StopModuleWait_sub( unsigned char );

extern  void	Settlement_rcv_faze_err( unsigned char *dat, unsigned char kind);

extern  unsigned char	rcv_split_data_check( void );
extern  void	Suica_rcv_split_flag_clear( void );

extern	void 	Suica_Log_regist( unsigned char* logdata, unsigned short log_size, unsigned char kind );
extern	unsigned char 	Suica_Snd_regist( unsigned char *kind, unsigned char *snddata );
extern	void	Op_Cansel_Wait( void );
extern  unsigned char    Op_Cansel_Wait_sub( unsigned char );
extern  void    Suica_fukuden_que_check( void );
extern  unsigned char    Miryo_Exec_check( void );
extern  char    in_credit_check( void );
// MH321800(S) G.So IC�N���W�b�g�Ή�
extern  void 	miryo_timeout_after_disp( void );
extern  void	lcd_wmsg_dsp_elec( char type, const unsigned char *msg1, const unsigned char *msg2, unsigned char timer, unsigned char ann, unsigned short color, unsigned short blink );
extern  void	Suica_Status_Chg( void );
// MH321800(E) G.So IC�N���W�b�g�Ή�
extern  void    miryo_timeout_after_proc( void );
extern  void    miryo_timeout_after_mif( unsigned char );
extern  unsigned short   SuicaErrCharChange( unsigned short ItemNum );
extern  void	Suica_Ctrl( unsigned short req, unsigned char type );

extern  unsigned char	SuicaUseKindCheck( unsigned char );

extern  unsigned char	StopStatusGet( unsigned char );

extern  unsigned char	DailyAggregateDataBKorRES( unsigned char );

extern	void	PayDataErrDisp( void );
extern	void	PayDataErrDispClr( void );

extern	short	SuicaParamGet( void );
// MH322914 (s) kasiyama 2016/07/13 �d�q�}�l�[���p�\�\���C��[���ʃo�ONo.1253](MH341106)
extern	void	PrinterCheckForSuica( void );
// MH322914 (e) kasiyama 2016/07/13 �d�q�}�l�[���p�\�\���C��[���ʃo�ONo.1253](MH341106)
// MH810103 GG119202(S) ������������L�^�ɍĐ��Z�����󎚂���
//// MH321800(S) G.So IC�N���W�b�g�Ή�
//// ���σ��[�_ ��������������O
//extern	void	EcAlarmLog_Clear( void );									// ���O�N���A
//extern	void	EcAlarmLog_Regist( EC_SETTLEMENT_RES *data );				// ���O�o�^
//extern	ushort	EcAlarmLog_GetCount( void );								// ���O�����擾
//extern	short	EcAlarmLog_LogRead( short idx, t_ALARM_SETTLEMENT_DATA *log );	// ���O�擾
//// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810103 GG119202(E) ������������L�^�ɍĐ��Z�����󎚂���

#endif	// _SUICA_DEF_H_
