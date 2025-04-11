#ifndef	_PRI_DEF_H_
#define	_PRI_DEF_H_
/*[]---------------------------------------------------------------------------[]*/
/*|		���������䕔�Fͯ�ް̧��													|*/
/*|																				|*/
/*|	̧�ٖ���	:	Pri_def.h													|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author		: K.Motohashi													|*/
/*|	Date		: 2005-07-19													|*/
/*|	UpDate		:																|*/
/*|																				|*/
/*[]--------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
#include	"mem_def.h"
// MH321800(S) G.So IC�N���W�b�g�Ή�
#include "suica_def.h"
// MH321800(E) G.So IC�N���W�b�g�Ή�

/*----------------------------------------------------------*/
/*	�����׸ޒ�`											*/
/*----------------------------------------------------------*/
#define		YES		1
#define		NO		0

#define		PRI_KYOUSEI		0			// �����o��
#define		PRI_FUSEI		1			// �s���o��


/*----------------------------------------------------------*/
/*	�f�o�b�O�p �ڍ׃G���[���b�Z�[�W�󎚃v���v���Z�b�T		*/
/*----------------------------------------------------------*/


/*----------------------------------------------------------*/
/*	ү���ފ֘A												*/
/*----------------------------------------------------------*/

/*----------------------------------*/
/*	ү���޺����						*/
/*----------------------------------*/
#define		PREQ_RYOUSYUU			0x0601	// �̎���			�󎚗v��
#define		PREQ_AZUKARI			0x0602	// �a���			�󎚗v��
#define		PREQ_UKETUKE			0x0603	// ��t��			�󎚗v��

#define		PREQ_TSYOUKEI			0x0604	// �W�v(�s���v)		�󎚗v��
#define		PREQ_TGOUKEI			0x0605	// �W�v(�s���v) 	�󎚗v��
#define		PREQ_GTSYOUKEI			0x0606	// �W�v(�f�s���v)	�󎚗v��
#define		PREQ_GTGOUKEI			0x0607	// �W�v(�f�s���v) 	�󎚗v��

#define		PREQ_F_TSYOUKEI			0x0608	// �W�v(�����s���v)	�󎚗v��
#define		PREQ_F_TGOUKEI			0x0609	// �W�v(�����s���v) �󎚗v��
#define		PREQ_F_GTSYOUKEI		0x060A	// �W�v(�����f�s���v)�󎚗v��
#define		PREQ_F_GTGOUKEI			0x060B	// �W�v(�����f�s���v)�󎚗v��

#define		PREQ_TGOUKEI_Z			0x060C	// �W�v(�O��s���v) 	�󎚗v��
#define		PREQ_GTGOUKEI_Z			0x060D	// �W�v(�O��f�s���v) 	�󎚗v��
#define		PREQ_F_TGOUKEI_Z		0x060E	// �W�v(�O�񕡐��s���v) �󎚗v��
#define		PREQ_F_GTGOUKEI_Z		0x060F	// �W�v(�O�񕡐��f�s���v)�󎚗v��

#define		PREQ_TGOUKEI_J			0x0610	// �W�v���(�s���v����) �󎚗v��

#define		PREQ_COKINKO_G			0x0611	// ��݋��ɍ��v		�󎚗v��
#define		PREQ_COKINKO_S			0x0612	// ��݋��ɏ��v		�󎚗v��
#define		PREQ_COKINKO_Z			0x0613	// �O��݋��ɍ��v	�󎚗v��
#define		PREQ_COKINKO_J			0x0614	// ��݋��ɏW�v���	�󎚗v��

#define		PREQ_SIKINKO_G			0x0615	// �������ɍ��v		�󎚗v��
#define		PREQ_SIKINKO_S			0x0616	// �������ɏ��v		�󎚗v��
#define		PREQ_SIKINKO_Z			0x0617	// �O�񎆕����ɍ��v	�󎚗v��
#define		PREQ_SIKINKO_J			0x0618	// �������ɏW�v���	�󎚗v��

#define		PREQ_TURIKAN			0x0619	// �ޑK�Ǘ�			�󎚗v��
#define		PREQ_TURIKAN_S			0x061A	// �ޑK�Ǘ��i���v�j	�󎚗v��
#define		PREQ_TURIKAN_LOG		0x061B	// �ޑK�Ǘ�����		�󎚗v��

#define		PREQ_ERR_JOU			0x061C	// �G���[���		�󎚗v��
#define		PREQ_ERR_LOG			0x061D	// �G���[��񗚗�	�󎚗v��

#define		PREQ_ARM_JOU			0x061E	// �A���[�����		�󎚗v��
#define		PREQ_ARM_LOG			0x061F	// �A���[����񗚗�	�󎚗v��

#define		PREQ_OPE_JOU			0x0620	// ������			�󎚗v��
#define		PREQ_OPE_LOG			0x0621	// �����񗚗�		�󎚗v��

#define		PREQ_SETTEIDATA			0x0622	// �ݒ�f�[�^		�󎚗v��

#define		PREQ_DOUSACNT			0x0623	// ����J�E���g		�󎚗v��

#define		PREQ_LK_DOUSACNT		0x0624	// ۯ����u���춳��	�󎚗v��

#define		PREQ_LOCK_PARA			0x0625	// �Ԏ��p�����[�^	�󎚗v��
#define		PREQ_LOCK_SETTEI		0x0626	// ���b�N���u�ݒ�	�󎚗v��

#define		PREQ_PKJOU_NOW			0x0627	// �Ԏ����i���݁j	�󎚗v��
#define		PREQ_PKJOU_SAV			0x0628	// �Ԏ����i�ޔ��j	�󎚗v��

#define		PREQ_TEIKI_DATA1		0x0629	// ����L���^����	�󎚗v��
#define		PREQ_TEIKI_DATA2		0x062A	// ������Ɂ^�o��	�󎚗v��

#define		PREQ_SVSTIK_KIGEN		0x062B	// ���޽������	�󎚗v��

#define		PREQ_SPLDAY				0x062C	// ���ʓ�		�󎚗v��

#define		PREQ_TEIRYUU_JOU		0x062D	// �◯�ԏ��	�󎚗v��

#define		PREQ_TEIFUK_JOU			0x062E	// �╜�d���	�󎚗v��
#define		PREQ_TEIFUK_LOG			0x062F	// �╜�d����	�󎚗v��

#define		PREQ_KOBETUSEISAN		0x0630	// �ʐ��Z���	�󎚗v��

#define		PREQ_FUSKYO_JOU			0x0632	// �s���E�����o�ɏ��	�󎚗v��
#define		PREQ_FUSKYO_LOG			0x0633	// �s���E�����o�ɗ���	�󎚗v��

#define		PREQ_ANYDATA_CNT		0x0634	// �e���ް��i�[����	�󎚗v��

#define		PREQ_IREKAE_TEST		0x0635	// �p������ւ����e�X�g�󎚗v��

#define		PREQ_PRINT_TEST			0x0636	// �v�����^�e�X�g�󎚗v��

#define		PREQ_AT_SYUUKEI			0x0637	// �����W�v�󎚗v��

#define		PREQ_LOGO_REGIST		0x0638	// ���S�󎚃f�[�^�o�^�v��

#define		PREQ_MON_JOU			0x0639	// ���j�^���			�󎚗v��
#define		PREQ_MON_LOG			0x063A	// ���j�^��񗚗�		�󎚗v��
#define		PREQ_CREDIT_CANCEL		0x063B	// �N���W�b�g������f�[�^�󎚗v��

#define		PREQ_CREDIT_USE			0x063C	// �N���W�b�g���p���׃��O�󎚗v��
#define		PREQ_SUICA_LOG			0x063E	// �ʐMLOG�ް���		�󎚗v��
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#define		PREQ_EDY_ARM_R			0x063F	// Edy�װ�ڼ���ް���		�󎚗v��
//#define		PREQ_EDY_STATUS			0x0640	// Edy�ݒ聕�X�e�[�^�X�f�[�^�󎚗v��
//#define		PREQ_EDY_ARMPAY_LOG		0x0641	// Edy�A���[��������󎚗v��
//#define		PREQ_EDY_SHIME_R		0x0642	// �d�������ߋL�^ڼ���ް���		�󎚗v��
//#define		PREQ_EDY_SHIME_LOG		0x0643	// �d�������ߋL�^�󎚗v��
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#define		PREQ_RMON_JOU			0x0644	// ���u�Ď����			�󎚗v��
#define		PREQ_RMON_LOG			0x0645	// ���u�Ď���񗚗�		�󎚗v��

#define		CMND_CHK2_SNO			PREQ_RYOUSYUU		// �󎚏����^�s�������Ώۺ���ށi�󎚗v��ү���޺���ށj�J�nNO.
enum{
	PREQ_ARC_MOD_VER = 0x0646, // ARCNET�o�[�W������
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	PREQ_EDY_USE_LOG,			// �d�������p���׃��O�󎚗v��
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	PREQ_SCA_USE_LOG,			// �r�����������p���׃��O�󎚗v��
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	PREQ_EDY_SYU_LOG,			// �d�����W�v���O�󎚗v��
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	PREQ_SCA_SYU_LOG,			// �r���������W�v���O�󎚗v��
	PREQ_NG_LOG,				// �s�������O	�󎚗v��
	PREQ_IO_LOG,				// ���o�Ƀ��O	�󎚗v��
	PREQ_ATEND_INV_DATA,		// �W�������f�[�^�󎚗v��
	PREQ_CHARGESETUP,			// �����ݒ��
	SETDIFFLOG_PRINT,			// �ݒ�X�V������
	PREQ_WCARD_INF,
	PREQ_HOJIN_USE,
	PREQ_RYOSETTEI_LOG,			// ���u�����ݒ胍�O��
	PREQ_CHK_PRINT,
	PREQ_LOOP_DATA,				// �t���b�v���[�v�f�[�^�v�����ʈ�
	PREQ_MNT_STACK,				// �X�^�b�N�g�p�ʈ�
	PREQ_CHKMODE_RESULT,
	PREQ_SUICA_LOG2,			// �ʐMLOG���߃f�[�^��		�󎚗v��
	PREQ_RT_PAYMENT_LOG,			// �����e�X�g���O�󎚗v��
	PREQ_MTSYOUKEI,				// �W�v(�l�s���v)			�󎚗v��(0x0659)
	PREQ_MTGOUKEI,				// �W�v(�l�s���v) 			�󎚗v��(0x065A)
	PREQ_MTGOUKEI_Z,			// �W�v(�O��l�s���v)		�󎚗v��(0x065B)
	PREQ_GTGOUKEI_J,			// �W�v���(�f�s���v����) 	�󎚗v��(0x065C)
	PREQ_CREDIT_UNSEND,			// �����M����˗��f�[�^	�󎚗v��
	PREQ_CREDIT_SALENG,			// ���㋑�ۃf�[�^	�󎚗v��
// MH321800(S) G.So IC�N���W�b�g�Ή�
	PREQ_EC_ALM_R,				// ���σ��[�_���[�_�A���[�����V�[�g�󎚗v��
	PREQ_EC_SYU_LOG,			// ���σ��[�_�����W�v���O�󎚗v��
	PREQ_EC_USE_LOG,			// ���σ��[�_���p���׃��O�󎚗v��
	PREQ_EC_BRAND_COND,			// ���σ��[�_�u�����h��Ԉ󎚗v��
	PREQ_EC_ALARM_LOG,			// ������������L�^�󎚗v��
	PREQ_EC_DEEMED_LOG,			// �݂Ȃ����ϕ��d���O�󎚗v��
	PREQ_RECV_DEEMED,			// ���ϐ��Z���~(������)�f�[�^�󎚗v��
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
	PREQ_EC_MINASHI_LOG,		// ���σ��[�_�݂Ȃ����σ��O�󎚗v��
// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
// MH810105(S) R.Endo 2021/12/21 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
	PREQ_DEBUG,					// �f�o�b�O�p�󎚗v��
// MH810105(E) R.Endo 2021/12/21 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
	PREQ_RECV_FAILURECONTACT,	// ��Q�A���[�f�[�^�󎚗v��
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
	PREQ_MAX,
};
#define	CMND_CHK2_ENO			PREQ_MAX		// �󎚏����^�s�������Ώۺ���ށi�󎚗v��ү���޺���ށj�I��NO.	

#define		PREQ_INNJI_END			0x0680	// �P��ۯ��󎚏I���ʒm

#define		PREQ_INIT_END_R			0x0681	// ڼ�������	�������I���ʒm
#define		PREQ_INIT_END_J			0x0682	// �ެ��������	�������I���ʒm
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
#define		PREQ_PRN_INFO			0x0683	// �v�����^���v��
#define		PREQ_CLOCK_REQ			0x0684	// �����ݒ�v��
#define		PREQ_CLOCK_SET_CMP		0x0685	// �����ݒ芮��
#define		PREQ_FS_ERR				0x0686	// �t�@�C���V�X�e���ُ�ʒm
#define		PREQ_MACHINE_INFO		0x0687	// �@����ʒm
#define		PREQ_SD_INFO			0x0688	// SD�J�[�h���ʒm
#define		PREQ_SW_VER_INFO		0x0689	// �\�t�g�o�[�W�����ʒm
#define		PREQ_SD_TEST_RW			0x068A	// SD�J�[�h�e�X�g�i���[�h���C�g�j���ʒʒm
#define		PREQ_SD_VER_INFO		0x068B	// SD�J�[�h�o�[�W�����ʒm
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i���Z�b�g�v���C�x���g�ǉ��j
#define		PREQ_RESET				0x068C	// �n�[�h���Z�b�g�v��
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i���Z�b�g�v���C�x���g�ǉ��j
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�����ݒ��Ɏ��v�ݒ肵�Ȃ��j
#define		PREQ_INIT_SET_REQ		0x068D	// �����^�����ݒ�v��
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�����ݒ��Ɏ��v�ݒ肵�Ȃ��j

#define 	PREQ_PRINTER_ERR		0x0690	// �v�����^�[�G���[����
#define 	PREQ_JUSHIN_ERR1		0x0691	// ��M�G���[�P						���S�T�d�w�FJUSHIN_ERR_SYORI
#define 	PREQ_JUSHIN_ERR2		0x0692	// ��M�G���[�Q(�j�A�G���h�ω���)	���S�T�d�w�FJUSHIN_ERR_SYORI2
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
#define 	PREQ_JUSHIN_INFO		0x0693	// �v�����^�X�e�[�^�X�ω��ʒm�i�d�q�W���[�i���p�j
#define		PREQ_ERR_END			0x0694	// �G���[�I���i���g���C�I�[�o�[�ŏI���j
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
#define		PREQ_MACHINE_INFO_R		0x0695	// ���V�[�g�v�����^�@����ʒm
#define		PREQ_STOCK_PRINT		0x0696	// �X�g�b�N�󎚊J�n�i�������ݏI���j
#define		PREQ_STOCK_CLEAR		0x0697	// �X�g�b�N�f�[�^�j���i�y�[�W�o�b�t�@�N���A�j
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j

#define		PREQ_SND_COMP_JP		0x06FC	// JP���M�����ʒm�i�󎚊����ł͂Ȃ��̂Œ��Ӂj
#define		PREQ_SND_COMP_RP		0x06FD	// RP���M�����ʒm�i�󎚊����ł͂Ȃ��̂Œ��Ӂj
#define		PREQ_DUMMY				0x06FE	// ��аү���ށi�����Ώ�ү���ތ����p�j
#define		PREQ_INNJI_TYUUSHI		0x06FF	// �󎚒��~�v��

/*----------------------------------*/
/*	ү���ޕҏW�p					*/
/*----------------------------------*/

// ���ڰ�������ւ̈󎚏I��ү���ވ󎚌���(BMode)
#define		PRI_NML_END		0x00	// ����I��
#define		PRI_CSL_END		0x01	// ��ݾٗv���ɂ��I��
#define		PRI_ERR_END		0x02	// �ُ�I��

#define		PRI_NO_PAPER	0x10	// ���؂�

// ���ڰ�������ւ̈󎚏I��ү���ވُ�I�����R(BStat)
#define		PRI_ERR_NON		0x00	// �󎚌��ʁ�����I���܂��ͷ�ݾُI���̏ꍇ
#define		PRI_ERR_STAT	0x01	// �������Ԉُ�
#define		PRI_ERR_BUSY	0x02	// �����BUSY
#define		PRI_ERR_ANY		0x03	// ���̑�
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
#define		PRI_ERR_WAIT_INIT	0x04	// �����������҂�
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

// ��������
#define		R_PRI			1		// ڼ��
#define		J_PRI			2		// �ެ���
#define		RJ_PRI			3		// ڼ��&�ެ���

#define		ERRPRI_TYP_LOG	0		// ������
#define		ERRPRI_TYP_JOU	1		// ����
// GG129000(S) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
#define		PRI_QR_AESSIZE	32		// QR���Ԍ� �Í����ΏۃG���A�T�C�Y
#define		PRI_QR_DATASIZE	124		// QR���Ԍ� �S�̃T�C�Y(CRC����)
// GG129000(E) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
// GG129000(S) M.Fujikawa 2023/04/27 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�R�[�h���Ԍ��̓�ǉ��Ή��i���Ԍ��̈Í���������ASCII�ϊ��j
#define		PRI_QR_HEADERSIZE		92	// QR���Ԍ� �w�b�_�[�T�C�Y
#define		PRI_QR_DATASIZEAS	156	// QR���Ԍ� �S�̃T�C�Y�iASCII������CRC�������j
// GG129000(E) M.Fujikawa 2023/04/27 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��@QR�R�[�h���Ԍ��̓�ǉ��Ή��i���Ԍ��̈Í���������ASCII�ϊ��j

/*----------------------*/
/* ү���޺��ĕҏW�p		*/
/*----------------------*/
#define		MSG_CMND_MASK		0x06FF	// �����Ͻ�
#define		CMND_CHK1_SNO		0x0601	// ������������ފJ�nNO.
#define		CMND_CHK1_ENO		0x06FF	// ������������ޏI��NO.

#define		INNJI_ENDMASK		0x1000	// �󎚏I��ү�����׸�(���ڰ�������ւ̑��M�p)
#define		INNJI_NEXTMASK		0x2000	// �󎚏�����ү�����׸ށi���������ւ̑��M�p�j

/*----------------------*/
/* ү�����ް��ҏW�p		*/
/*----------------------*/

// ��������
#define REQ_PRI_MASK			0x0F	// �Ώ���������Ͻ��ް�(���ڰ����������̈󎚗v��ү���ޗp)
#define NEXT_PRI_MASK			0xF0	// �Ώ���������Ͻ��ް�(������������̈󎚗v��ү���ޗp)

#define I2C_PRI_REQ				0		// �󎚗v��
#define I2C_NEXT_SND_REQ		1		// �󎚊����i���󎚃f�[�^�v���j
#define I2C_EVENT_QUE_REQ		2		// ���󎚎��̃C�x���g���s�v��


/*----------------------------------*/
/*	�󎚗v��ү���ވ��ް�̫�ϯ�	*/
/*----------------------------------*/

// ��������ʁitik_syu�j
#define		SERVICE			0x01	// ���޽��
#define		KAKEURI			0x02	// �|����
#define		KAISUU			0x03	// �񐔌�
#define		WARIBIKI		0x04	// ������
#define		PREPAID			0x05	// �v���y�C�h�J�[�h
// MH810100(S) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
#define		SHOPPING		0x06	// ��������
// MH810100(E) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
// GG129000(S) R.Endo 2022/10/25 �Ԕԃ`�P�b�g���X4.0 #6651 �Ԏ튄���Ŋ����]�͂��c���A�Đ��Z�����ꍇ�̏W�v�󎚂��s�� [���ʉ��P���� No1541]
#define		SYUBETU			0x07	// ��ʊ���
// GG129000(E) R.Endo 2022/10/25 �Ԕԃ`�P�b�g���X4.0 #6651 �Ԏ튄���Ŋ����]�͂��c���A�Đ��Z�����ꍇ�̏W�v�󎚂��s�� [���ʉ��P���� No1541]

#define		MISHUU			0x10	// �������i���z���Z���Ɏg�p�j
#define		FURIKAE			0x11	// �U�֊z�i�U�֐��Z���Ɏg�p�j
// MH322914(S) K.Onodera 2016/10/11 AI-V�Ή��F�U�֐��Z
#define		FURIKAE2		0x20	// �U�֊z�iParkingWeb�ŐU�֐��Z�Ŏg�p�j
#define		FURIKAE_DETAIL	0x21	// �U�֏ڍ�
// MH322914(E) K.Onodera 2016/10/11 AI-V�Ή��F�U�֐��Z
#define		SYUUSEI			0x12	// �C���z�i�C�����Z���Ɏg�p�j

#define		C_SERVICE		0x81	// ���Z���~���޽��
#define		C_KAKEURI		0x82	// ���Z���~�|����
#define		C_KAISUU		0x83	// ���Z���~�񐔌�
#define		C_WARIBIKI		0x84	// ���Z���~������
// MH810100(S) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
#define		C_SHOPPING		0x86	// ��������
// MH810100(E) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
// GG129000(S) R.Endo 2022/10/25 �Ԕԃ`�P�b�g���X4.0 #6651 �Ԏ튄���Ŋ����]�͂��c���A�Đ��Z�����ꍇ�̏W�v�󎚂��s�� [���ʉ��P���� No1541]
#define		C_SYUBETU		0x87	// ��ʊ���
// GG129000(E) R.Endo 2022/10/25 �Ԕԃ`�P�b�g���X4.0 #6651 �Ԏ튄���Ŋ����]�͂��c���A�Đ��Z�����ꍇ�̏W�v�󎚂��s�� [���ʉ��P���� No1541]

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
#define		INVALID			0xff	// �s��
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�

// ���ԏ�m���D��ʁipkno_syu�j
#define		KIHON_PKNO		0		// ��{���ԏ�No.
#define		KAKUCHOU_1		1		// �g�����ԏ�No.1
#define		KAKUCHOU_2		2		// �g�����ԏ�No.2
#define		KAKUCHOU_3		3		// �g�����ԏ�No.3


	// �a��؈��ް�̫�ϯ�
	typedef struct {
		uchar			WFlag;		// �󎚎�ʂP�i0:�ʏ� 1:���d�j
		uchar			chuusi;		// �󎚎�ʂQ�i0:�ʏ� 1:���Z���~�j
		ulong			WPlace;		// ���Ԉʒu�ް�
		ushort			Kikai_no;	// �@�B�m��
// MH810100(S) Y.Yamauchi 2020/02/18 �Ԕԃ`�P�b�g���X�i�a��؈󎚏C���j
		uchar			CarSearchFlg; // �Ԕ�/�����������
		uchar			CarSearchData[6]; // �Ԕ�(����)
		uchar			CarNumber[4]; // �Ԕ�(���Z)
		uchar			CarDataID[32]; // �Ԕԃf�[�^ID
// MH810100(E) Y.Yamauchi 2020/02/18 �Ԕԃ`�P�b�g���X�i�a��؈󎚏C���j
		uchar			Seisan_kind;// ���Z���
		ST_OIBAN		Oiban;		// �ǔ�(�a��ǔ�)
		ST_OIBAN		Oiban2;		// �ǔ�(���Z�ǔ� �܂��� ���~�ǔ�)
		date_time_rec	TTime;		// ���s����
		ulong			WTotalPrice;// �����z		�����v
		ulong			WInPrice;	// �����z		�����a��
		ulong			WChgPrice;	// ���ߋ��z		�����ߊz
		ulong			WFusoku;	// ���ߕs�����z	�����o�s���z
		ushort			Lok_No;		//���b�N���uNo
		uchar			Syusei;		// �C�����Z�i0:�Ȃ� 1:����j
	} Azukari_data;

	// ��t�����ް�̫�ϯ�
	typedef struct {
		ulong			WPlace;		// ���Ԉʒu�ް�
		ushort			Kikai_no;	// �@�B�m��	
		ulong			Oiban;		// �ǔ�
		date_time_rec	ISTime;		// ���s����
		date_time_rec	TTime;		// ���ɓ���
		ushort			Pword;		// �߽ܰ��
		ushort			Lok_No;		//���b�N���uNo
		uchar			PrnCnt;		// ���ԏؖ������s��
	} Uketuke_data;


/*----------------------------------*/
/*	ү�����ް���̫�ϯ�				*/
/*----------------------------------*/

// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
#pragma	pack
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j

	// PREQ_RYOUSYUU�F�̎��؈󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		Receipt_data	*prn_data;		// �̎��؈��ް��߲���
		uchar			reprint;		// �Ĕ��s�׸ށiOFF�F�ʏ�/ON�F�Ĕ��s�j
		date_time_rec	PriTime;		// �Ĕ��s����
		uchar			kakari_no;		// �W��No.
	} T_FrmReceipt;

	// PREQ_AZUKARI�F�a��؈󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		Azukari_data	*prn_data;		// �a��؈��ް��߲���
	} T_FrmAzukari;

	// PREQ_UKETUKE�F��t���󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		Uketuke_data	*prn_data;		// ��t�����ް��߲���
		uchar			reprint;		// �Ĕ��s�׸ށiOFF�F�ʏ�/ON�F�Ĕ��s�j
		date_time_rec	PriTime;		// �Ĕ��s����
		uchar			kakari_no;		// �W��No.
	} T_FrmUketuke;

	// PREQ_TSYOUKEI	�F�W�v�i�s���v�j			�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_TGOUKEI		�F�W�v�i�s���v�j			�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_GTSYOUKEI	�F�W�v�i�f�s���v�j			�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_GTGOUKEI	�F�W�v�i�f�s���v�j			�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_F_TSYOUKEI	�F�W�v�i�����s���v�j		�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_F_TGOUKEI	�F�W�v�i�����s���v�j		�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_F_GTSYOUKEI	�F�W�v�i�����f�s���v�j		�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_F_GTGOUKEI	�F�W�v�i�����f�s���v�j		�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_TGOUKEI_Z	�F�W�v�i�O��s���v�j		�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_GTGOUKEI_Z	�F�W�v�i�O��f�s���v�j		�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_F_TGOUKEI_Z	�F�W�v�i�O�񕡐��s���v�j	�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_F_GTGOUKEI_Z�F�W�v�i�O�񕡐��f�s���v�j	�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_AT_SYUUKEI	�F�W�v�i�����W�v�j			�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		SYUKEI			*prn_data;		// �W�v���ް��߲���
		date_time_rec	PriTime;		// ������ē���
// MH810105 GG119202(S) T���v�A���󎚑Ή�
		ushort			print_flag;		// �e���ږ��̈󎚂̗L��
// MH810105 GG119202(E) T���v�A���󎚑Ή�
	} T_FrmSyuukei;

	// PREQ_COKINKO_G	�F��݋��ɍ��v		�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_COKINKO_S	�F��݋��ɏ��v		�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_COKINKO_Z	�F�O��݋��ɍ��v	�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		COIN_SYU		*prn_data;		// ��݋��ɏW�v���ް��߲���
		date_time_rec	PriTime;		// ������ē���
	} T_FrmCoSyuukei;

	// PREQ_SIKINKO_G	�F�������ɍ��v		�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_SIKINKO_S	�F�������ɏ��v		�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_SIKINKO_Z	�F�O�񎆕����ɍ��v	�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		NOTE_SYU		*prn_data;		// �������ɏW�v���ް��߲���
		date_time_rec	PriTime;		// ������ē���
	} T_FrmSiSyuukei;

	typedef struct {
		uchar			prn_kind;			// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;				// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;			// �@�B��
		ulong			Oiban;				// �ǔ�
		date_time_rec	NowTime;			// �������
	} T_FrmChargeSetup;

	// PREQ_TURIKAN		�F�ޑK�Ǘ�			�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_TURIKAN_S	�F�ޑK�Ǘ��i���v�j	�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		TURI_KAN		*prn_data;		// �ޑK�Ǘ��W�v���ް��߲���
	} T_FrmTuriKan;

	// PREQ_ERR_JOU		�F�G���[���	�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		Err_log			*prn_data;		// �װ�����ް��߲���
	} T_FrmErrJou;

	// PREQ_ARM_JOU		�F�A���[�����	�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		Arm_log			*prn_data;		// �װя����ް��߲���
	} T_FrmArmJou;

	// PREQ_OPE_JOU		�F������	�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		Ope_log			*prn_data;		// ��������ް��߲���
	} T_FrmOpeJou;

	// PREQ_MON_JOU		�F������	�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		Mon_log			*prn_data;		// ��������ް��߲���
	} T_FrmMonJou;

	// PREQ_RMON_JOU	�F���u�Ď����	�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		Rmon_log		*prn_data;		// ��������ް��߲���
	} T_FrmRmonJou;

	// PREQ_SETTEIDATA	�F�ݒ��ް��󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;		// �@�B�m���D
	} T_FrmSetteiData;

	// PREQ_DOUSACNT	�F���춳�Ĉ󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;		// �@�B�m���D
	} T_FrmDousaCnt;

	// PREQ_LK_DOUSACNT	�Fۯ����u���춳�Ĉ󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;		// �@�B�m���D
		uchar			Req_syu;		// �v����ʁi�P�F�ׯ�ߑ��u�^�Q�Fۯ����u�j
	} T_FrmLockDcnt;

	// PREQ_LOCK_PARA	�F�Ԏ��p�����[�^�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;		// �@�B�m���D
	} T_FrmLockPara;

	// PREQ_LOCK_SETTEI	�F���b�N���u�ݒ�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;		// �@�B�m���D
	} T_FrmLockSettei;

	// PREQ_PKJOU_NOW	�F�Ԏ����i���݁j�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy2;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;		// �@�B�m���D
		uchar			prn_menu;		// ���ƭ�(���Ԃ̂�/��Ԃ̂�/�S��)
		uchar			dummy;
	} T_FrmPkjouNow;

	// PREQ_PKJOU_SAV	�F�Ԏ����i�ޔ��j�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;		// �@�B�m���D
	} T_FrmPkjouSav;

	// PREQ_TEIKI_DATA1�F����L���^�����󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;		// �@�B�m���D
		ushort			Kakari_no;		// �W���m���D
		uchar			Pkno_syu;		// ���ԏ�m���D��ʁi��{�^�g�P�^�g�Q�^�g�R�j
		uchar			Req_syu;		// �v����ʁi�O�F�S�w��^�P�F�L���̂݁^�Q�F�����̂݁j
	} T_FrmTeikiData1;

	// PREQ_TEIKI_DATA2�F������Ɂ^�o�Ɉ󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;		// �@�B�m���D
		ushort			Kakari_no;		// �W���m���D
		uchar			Pkno_syu;		// ���ԏ�m���D��ʁi��{�^�g�P�^�g�Q�^�g�R�j
		uchar			Req_syu;		// �v����ʁi�O�F�S�w��^�P�F���ɂ̂݁^�Q�F�o�ɂ̂݁^�R�F����̂݁j
	} T_FrmTeikiData2;

	// PREQ_SVSTIK_KIGEN�F���޽�������󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;		// �@�B�m���D
		ushort			Kakari_no;		// �W���m���D
	} T_FrmStikKigen;

	// PREQ_SPLDAY�F���ʓ��󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;		// �@�B�m���D
		ushort			Kakari_no;		// �W���m���D
	} T_FrmSplDay;

	// PREQ_TEIRYUU_JOU�F�◯�ԏ��󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;		// �@�B�m���D
		ushort			Kakari_no;		// �W���m���D
		short			Day;			// �◯����
		short			Cnt;			// �◯�Ԑ�
		short			*Data;			// �◯���ް��߲���
	} T_FrmTeiRyuuJou;

	// PREQ_TEIFUK_JOU�F�╜�d���󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		Pon_log			Ponlog;			// �╜�d���
	} T_FrmTeiFukJou;

	// PREQ_FUSKYO_JOU	:�s���E�����o�ɏ��	�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		flp_log			*prn_data;		// �s���E�����o�ɏ���ް��߲���
	} T_FrmFusKyo;

	// PREQ_ANYDATA_CNT�F�e���ް��i�[�����󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;		// �@�B�m���D
	} T_FrmAnyData;

	// PREQ_IREKAE_TEST	�F�p�����֎�ýĈ󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
	} T_FrmIrekaeTest;

	// PREQ_PREQ_PRINT_TEST	�F�����ýĈ󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
	} T_FrmPrintTest;

	// PREQ_ERR_LOG		�F�G���[��񗚗�	�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_ARM_LOG		�F�A���[����񗚗�	�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_OPE_LOG		�F�����񗚗�		�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_TEIFUK_LOG	�F�╜�d��񗚗�	�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_MON_LOG		�F���j�^��񗚗�	�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;		// �@�B�m���D
		ushort			Kakari_no;		// �W���m���D
		date_time_rec	PriTime;		// ����ē���

	} T_FrmLogPriReq1;

	// PREQ_COKINKO_J		�F��݋��ɏW�v���		�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_SIKINKO_J		�F�������ɏW�v���		�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_KOBETUSEISAN	�F�ʐ��Z���			�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_SEISAN_CHUUSI	�F���Z���~���			�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_FUSKYO_LOG		�F�s���E�����o�ɗ���	�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_TGOUKEI_J		�F�W�v���(�s���v����) 	�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_TURIKAN_LOG		�F�ޑK�Ǘ�����			�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;		// �@�B�m���D
		ushort			Kakari_no;		// �W���m���D
		uchar			BType;			// �������@(0:�S�ā^1:���t�^2:����)
		date_time_rec	TSttTime;		// �J�n����
		date_time_rec	TEndTime;		// �I������
		ushort			LogCount;		// LOG�o�^����(�ʐ��ZLOG�E�W�vLOG�Ŏg�p)
										// FlashROM���f�[�^�ǂݎ��s���͌���������̂�
										// �ēxFlashROM����擾����̂ł͂��̒l���g�p����
		ushort			Room_no;		// �Ԏ��ԍ�
	} T_FrmLogPriReq2;
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;		// �@�B�m���D
		ushort			Kakari_no;		// �W���m���D
		uchar			BType;			// �������@(0:�S�ā^1:���t�^2:����)
		date_time_rec	TSttTime;		// �J�n����
		date_time_rec	TEndTime;		// �I������
		ushort			LogCount;		// LOG�o�^����(���p���׃��O�Ŏg�p)
		date_time_rec	PriTime;		// ����ē���
	} T_FrmLogPriReq3;

	// PREQ_EDY_USE_LOG		�FEdy���p����			�󎚗v��ү�����ް�̫�ϯ�
	// PREQ_SCA_USE_LOG		�FSuica���p����			�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;			// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;				// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;			// �@�B�m���D
		ushort			Kakari_no;			// �W���m���D
		uchar			BType;				// �������@(0:�S�ā^1:���t�^2:����)
		date_time_rec	TSttTime;			// �J�n����
		date_time_rec	TEndTime;			// �I������
		ushort			LogCount;			// LOG�o�^����(�ʐ��ZLOG�E�W�vLOG�Ŏg�p)
											// FlashROM���f�[�^�ǂݎ��s���͌���������̂�
											// �ēxFlashROM����擾����̂ł͂��̒l���g�p����
		ushort			Ffst_no;			// �����������ŏ��Ɉ�v����FlashROM�̏��ԍ�
		ushort			Flst_no;			// �����������Ō�Ɉ�v����FlashROM�̏��ԍ�
	} T_FrmLogPriReq4;

	// PREQ_CREDIT_UNSEND	�F�����M����˗��f�[�^	�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;		// �@�B�m���D
		date_time_rec	PriTime;		// ����ē���
		struct DATA_BK	*back;			// ����˗��ް�
	} T_FrmUnSendPriReq;
	// PREQ_CREDIT_SALENG	�F���㋑�ۃf�[�^	�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			Kikai_no;		// �@�B�m���D
		date_time_rec	PriTime;		// ����ē���
		CRE_SALENG		*ng_data;		// ���㋑���ް�
	} T_FrmSaleNGPriReq;

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY 
//	typedef struct {
//		uchar			prn_kind;		// ��������
//		edy_arm_log		*priedit;		// �@�B�m���D
//	} T_FrmEdyArmReq;
//
//	typedef struct {
//		uchar			prn_kind;		// ��������
//		edy_shime_log	*priedit;		// ���O�f�[�^�߲��
//	} T_FrmEdyShimeReq;
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

	// PREQ_LOGO_REGIST	�F���S�󎚃f�[�^�o�^�v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
	} T_FrmLogoRegist;

	// PREQ_INNJI_END�F�P��ۯ��󎚏I��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			prn_sts;		// ��������
	} T_FrmPrnBend;

	// PREQ_PRINTER_ERR�F�������װү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			ssr_data;		// SSR�ް�
	} T_FrmPrnErr0;

	// PREQ_JUSHIN_ERR1�F��M�װ�Pү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			prn_sts;		// ��������
	} T_FrmPrnErr1;

	// PREQ_JUSHIN_ERR2�F��M�װ�Qү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			prn_sts;		// ��������
	} T_FrmPrnErr2;

	// PREQ_INNJI_TYUUSHI�F�󎚒��~ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;		// ��������
										//	R_PRI(1) = ڼ��
										//	J_PRI(2) = �ެ���
										//	RJ_PRI(3)= ڼ�ā��ެ���
	} T_FrmPrnStop;

	// �󎚏I��ү�����ް�̫�ϯāi���ڰ�������ւ̈󎚏I��ү�����ް����j
	typedef struct {
		uchar			BMode;			// �󎚌���
										//	PRI_NML_END(0)	�F����I��
										//	PRI_CSL_END(1)	�F��ݾُI��
										//	PRI_ERR_END(2)	�F�ُ�I��

		uchar			BStat;			// �ُ�I�����R
										//	PRI_ERR_NON(0)	�F�󎚌��ʁ�����I���܂��ͷ�ݾُI���̏ꍇ
										//	PRI_ERR_STAT(1)	�F�������Ԉُ�
										//	PRI_ERR_BUSY(2)	�F�����BUSY
										//	PRI_ERR_ANY(3)	�F���̑�

		uchar			BPrinStat;		// ������ð��
										//	Bit0:�߰�߰Ʊ����	�i1=Ʊ���ށj
										//	Bit1:����ݵ����		�i1=����݁j
										//	Bit2:�߰�߰����		�i1=���ށj
										//	Bit3:ͯ�މ��x�ُ�	�i1=�ُ�j
										//	Bit4�`7:���g�p

		uchar			BPrikind;		// ��������
										//	R_PRI(1) = ڼ��
										//	J_PRI(2) = �ެ���
	} T_FrmEnd;

// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
#pragma	unpack
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j

	typedef	union	{
		Receipt_data	Receipt_data;
		Syu_log			Syu_log;
		TURI_KAN		mny_log;
		COIN_SYU		coin_syu;
		NOTE_SYU		note_syu;
		flp_log			Flp_log;
		meisai_log		cre_log;
		Ope_log 		Ope_work;
		Mon_log			Mon_work;
		Arm_log			Arm_work;										// �A���[�����O���[�N�o�b�t�@
		Err_log			Err_work;										// �G���[���O���[�N�o�b�t�@
		Pon_log			Pon_log_work;									// �╜�d���O���[�N�o�b�t�@
		t_Change_data	Change_data;
		Rmon_log		Rmon_work;
	} t_Pri_Work;

// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
#pragma	pack
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j

	typedef struct {
		uchar			prn_kind;			// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;				// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			prn_inf;			// �󎚂��镪�ގ��
		date_time_rec	PriTime;			// ������ē���
		ushort			Kikai_no;			// �@�B��
		ushort			Kakari_no;			// �W����
		syuukei_info	*syuukeiSP;
	} T_FrmSyuukei2;
	typedef struct {
		uchar			prn_kind;
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;				// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ulong			Serial_no;
		ushort			Kakari_no;
		ulong			Chk_no;
		ushort			Card_no;
		date_time_rec	ChkTime;
	} T_FrmChk_result;

// MH321800(S) G.So IC�N���W�b�g�Ή�
	// PREQ_EC_ALM_R�F���σ��[�_�A���[�����V�[�g�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;				// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;					// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		date_time_rec	TOutTime;				// ��������
		EC_SETTLEMENT_RES	*alm_rct_info;		// �A���[�����V�[�g���
	} T_FrmEcAlmRctReq;

	// PREQ_EC_BRAND_COND�F���σ��[�_�u�����h��Ԉ󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar				prn_kind;			// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar				dummy;
		ulong				job_id;				// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort				Kikai_no;			// �@�B��
		EC_BRAND_TBL		*pridata;			// �󎚈˗��f�[�^
		EC_BRAND_TBL		*pridata2;			// �󎚈˗��f�[�^
	} T_FrmEcBrandReq;

	typedef struct {
		uchar				prn_kind;			// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar				dummy;
		ulong				job_id;				// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort				Kikai_no;			// �@�B��
// MH810105 GG119202(S) T���v�A���󎚑Ή�
		ulong				StartTime;			// �J�n����
		ulong				EndTime;			// �I������
// MH810105 GG119202(E) T���v�A���󎚑Ή�
	} T_FrmEcAlarmLog;

	typedef struct {
		uchar			prn_kind;				// ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;					// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		EcDeemedFukudenLog	*deemed_jnl_info;	// �݂Ȃ����Ϗ��
	} T_FrmEcDeemedJnlReq;
// MH321800(E) G.So IC�N���W�b�g�Ή�

// MH810105 GG119202(S) T���v�A���󎚑Ή�
	// PREQ_TGOUKEI		�FT���v�A���󎚗v��ү�����ް�̫�ϯ�
	// PREQ_TGOUKEI_Z	�F�O��T���v�A���󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		T_FrmLogPriReq4	Minashi;			// �݂Ȃ����σv�����g���
		T_FrmEcAlarmLog	Alarm;				// ������������L�^���
	} T_FrmTgoukeiEc;

	typedef struct {
		T_FrmSyuukei	syuk;				// T���v���
		T_FrmTgoukeiEc	Ec;					// T���v�A���󎚏��
	} T_FrmSyuukeiEc;
// �󎚃t���O
#define	TGOUKEI_EC_MINASHI			0x0001	// �݂Ȃ����σv�����g
#define	TGOUKEI_EC_ALARM			0x0002	// ������������L�^
// MH810105 GG119202(E) T���v�A���󎚑Ή�


// MH810105(S) R.Endo 2021/12/21 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
	// PREQ_DEBUG�F�f�o�b�O�p�󎚗v��ү�����ް�̫�ϯ�
	typedef struct {
		uchar			prn_kind;				// [ 1Byte] ��������
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			dummy;
		ulong			job_id;					// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		uchar			first_line;				// [ 1Byte] 1�s���׸�
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
//		struct clk_rec	wk_CLK_REC;				// [12Byte] ����

// pack�̊֌W�ŁADATE_YMDHMS�ɒu������
		DATE_YMDHMS		wk_CLK_YMDHMS;				// [7Byte] ����
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ushort			wk_CLK_REC_msec;		// [ 2Byte] ����(msec)
		char			data[33];				// [33Byte] ���ް�
	} T_FrmDebugData;
	// NOTE:queset�ő��M���郁�b�Z�[�W�f�[�^�̃t�H�[�}�b�g�Ȃ̂ŁA
	//      �T�C�Y��queset�̃o�b�t�@(MSGBUFSZ)�𒴂��Ȃ��悤�ɒ��ӂ���B
// MH810105(E) R.Endo 2021/12/21 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��

// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
#pragma	unpack
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j

/*----------------------------------------------------------*/
/*	��������M�ޯ̧���ޒ�`									*/
/*----------------------------------------------------------*/
#define	PRNBUF_SIZE				64		// ��������M�ޯ̧����
#define	PRNQUE_CNT				(64+4)		// ��������M�ޯ̧�� (+4:ۺ�+ͯ�ް��ͯ�ް��󎚗p�ɒǉ�) see.Prn_LogoHead_PostHead 

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
#define	PRN_EJA_BUF_SIZE		32			// �d�q�W���[�i���p��M�o�b�t�@�T�C�Y
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
#define	PRN_RCV_BUF_SIZE_R		34			// ���V�[�g�v�����^�p��M�o�b�t�@�T�C�Y
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j

/*----------------------------------------------------------*/
/*	���ް��ҏW�����֘A									*/
/*----------------------------------------------------------*/
// �P�s�󎚍ő包��
#define FONTA_MAX_COL			36		// ̫�Ă`
#define FONTB_MAX_COL			48		// ̫�Ăa
#define KANJI_MAX_COL			18		// ����

// �󎚈ʒu
#define		PRI_LEFT			0x01	// ������
#define		PRI_CENTER			0x02	// ����ݸ�
#define		PRI_RIGHT			0x03	// �E����

// �������
#define		FONT_A_NML			0x01	// ̫�Ă` �ʏ�
#define		FONT_A_TATEBAI		0x02	// ̫�Ă` �c�{�p
#define		FONT_A_YOKOBAI		0x03	// ̫�Ă` ���{�p
#define		FONT_A_4BAI			0x04	// ̫�Ă` �S�{�p

#define		FONT_B_NML			0x11	// ̫�Ăa �ʏ�
#define		FONT_B_TATEBAI		0x12	// ̫�Ăa �c�{�p
#define		FONT_B_YOKOBAI		0x13	// ̫�Ăa ���{�p
#define		FONT_B_4BAI			0x14	// ̫�Ăa �S�{�p

#define		FONT_K_NML			0x31	// ���� �ʏ�
#define		FONT_K_TATEBAI		0x32	// ���� �c�{�p
#define		FONT_K_YOKOBAI		0x33	// ���� ���{�p
#define		FONT_K_4BAI			0x34	// ���� �S�{�p

// �����̏c�ޯĐ��i�P�ޯ�=0.125mm�j
#define		FONT_A_N_HDOT		24		// ̫�Ă` �ʏ�	(24�~0.125=3mm)
#define		FONT_A_T_HDOT		48		// ̫�Ă` �c�{�p(48�~0.125=6mm)
#define		FONT_A_Y_HDOT		24		// ̫�Ă` ���{�p(24�~0.125=3mm)
#define		FONT_A_4_HDOT		48		// ̫�Ă` �S�{�p(48�~0.125=6mm)

#define		FONT_B_N_HDOT		17		// ̫�Ăa �ʏ�	(17�~0.125=2.125mm)
#define		FONT_B_T_HDOT		34		// ̫�Ăa �c�{�p(34�~0.125=4.25mm)
#define		FONT_B_Y_HDOT		17		// ̫�Ăa ���{�p(17�~0.125=2.125mm)
#define		FONT_B_4_HDOT		34		// ̫�Ăa �S�{�p(34�~0.125=4.25mm)

#define		FONT_K_N_HDOT		24		// ���� �ʏ�	(24�~0.125=3mm)
#define		FONT_K_T_HDOT		48		// ���� �c�{�p	(48�~0.125=6mm)
#define		FONT_K_Y_HDOT		24		// ���� ���{�p	(24�~0.125=3mm)
#define		FONT_K_4_HDOT		48		// ���� �S�{�p	(48�~0.125=6mm)

// ����������
#define		FONTA_WIDE			15		// 1.5mm(�E�X�y�[�X�O)
#define		KANJI_WIDE			30		// 3.0mm(���E�X�y�[�X�O)
#define		ROW_WIDE			540		// 54mm(�p���󎚕�)

// ڼ�Ķ�Đ���
#define		RCUT_END_FEED		48		// ۰َ��l�܂�΍�̈׶�Č�Ɏ����肷�黲�ށi�ޯĐ��j	��48�~0.125=6mm
#define		RCUT_END_FEED_AH	28		// ͯ�ވ󎚂̶݂̂�Č�Ɏ����肷�黲�ށi�ޯĐ��j		��28�~0.125=3.5mm(���s�ʕ�)
#define		RCUT_END_FEED_NP	52		// ۺށEͯ�ވ󎚂Ȃ����̶�Č�Ɏ����肷�黲�ށi�ޯĐ��j	��52�~0.125=6.5mm
#define		RCUT_END_PERTIAL_FEED 8		// �p�[�V�����J�b�g��A���̕������󎚂���O�Ƀt�B�[�h����h�b�h��(8�~0.125 = 1mm)

// ���S�󎚊֘A
#define		LOGO_HDOT_SIZE		144		// ۺނ̏c���ށi�ޯĐ��j								��144�~0.125=18mm
#define		LOGO_WDOT_SIZE		432		// ۺނ̉����ށi�ޯĐ��j								��432�~0.125=54mm

#define		LOGO_HEADER_DSIZE	62		// ���S�f�[�^�w�b�_�[�����̃f�[�^�T�C�Y
										// �ׯ����؏���ް�
#define		LOGO_1LINE_DSIZE	56		// ���S�f�[�^���P���C���i�ޯĒP�ʁj�̃f�[�^�T�C�Y
										// �ׯ����؏���ް�̫�ϯ�	��432�ޯā�8�ޯ�=54(+2�޲āF4�޲������ذ�̈�)

#define		LOGO_REG_TATE_DOT	24		// ���S�o�^���ɂ����镶���P�ʂł̏c�ޯĐ�
#define		LOGO_LINE_CNT		6		// ���S�󎚃f�[�^�̓o�^�s���i�����P�ʂł̍s���j
#define		LOGO_1LINE_BCNT		18		// ���S�󎚃f�[�^�P�s���̓o�^��ۯ����i�P��ۯ����Q�S�~�Q�S�ޯĂ��ޯ�ϯ�߁j

// �^�C�}�[�l
#define		PRN_INIT_TIMER		50*3	// ����������������҂���ϰ�l		�i�R�b�j
#define		PRN_PWON_TIMER		50*3	// �╜�d���󎚗v���҂��҂���ϰ�l	�i�R�b�j
#define		PRN_BUSY_TIMER		50*5	// ������޼ް�M���Ď���ϰ�l			�i�T�b�j
#define		PRN_PNEND_TIMER		50*10	// �߰�߰Ʊ���ފĎ���ϰ�l			�i�P�O�b�j
#define		PRN_WAIT_TIMER		2		// ���ް��ҏW�x����ϰ�l�i���ް��P�s���̒x����ϰ�l�j
										// �����ް��ҏW�x����ϰ�́A�P�s���Ɂu��ϰ�l�~�Q�O�����v�ƂȂ�
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
#define		PRN_PINFO_RES_WAIT_TIMER	(1)			// �v�����^��񉞓��҂��^�C�}�i500ms�j
#define		PRN_RW_RES_WAIT_TIMER		(60)		// ���[�h�E���C�g�e�X�g�҂��^�C�}�i60s�j
#define		PRN_RESET_WAIT_TIMER		(1)			// ���Z�b�g�����҂��^�C�}�i500ms�j
#define		PRN_SD_MOUNT_WAIT_TIMER		(30)		// SD�}�E���g�����҂��^�C�}�i30s�j
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
//#define		PRN_WRITE_CMP_TIMER			(50*1)		// �f�[�^�������݊����҂��^�C�}�i1s�j
#define		PRN_WRITE_CMP_TIMER			(50*10)		// �f�[�^�������݊����҂��^�C�}�i10s�j
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
#define		PRN_INIT_SET_WAIT_TIMER		(50*5)		// �����^�����ݒ�҂��^�C�}�i5s�j
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

#define		PRN_BUSY_TCNT		3		// ������޼ް�p���Ď������l(10sec�p��BUSY�Ŵװ�Ƃ���)

// ���̑�
#define		EDIT_GYOU_MAX		15		// �P��ۯ��ҏW�ő�s��
#define		RCT_INIT_RCNT		1		// ڼ�������	����������ޑ��M��ײ��
#define		JNL_INIT_RCNT		1		// �ެ��������	����������ޑ��M��ײ��



/*----------------------------------------------------------*/
/*	������������ޕҏW										*/
/*----------------------------------------------------------*/
#define LF		0x0A
#define ESC		0x1B
#define FS		0x1C
#define GS		0x1D
																					//		[ �@ �\ ]							������ށ�
//							���P�@�\��
#define	PCMD_R_SPACE( p )			PrnCmdLen( (uchar *)"\x1b\x20\x00" , 3 , p )	// [�����̉E��߰���(0mm)]				��ESC SPC n(=0x04)��

#define	PCMD_FONT_A_N( p )			PrnCmdLen( (uchar *)"\x1b\x21\x00" , 3 , p )	// [��Ӱ�ނ̈ꊇ�w��(̫�Ă`�ʏ�)]		��ESC ! n(=0x00)��
#define	PCMD_FONT_A_T( p )			PrnCmdLen( (uchar *)"\x1b\x21\x10" , 3 , p )	// [��Ӱ�ނ̈ꊇ�w��(̫�Ă`�c�{)]		��ESC ! n(=0x10)��
#define	PCMD_FONT_A_Y( p )			PrnCmdLen( (uchar *)"\x1b\x21\x20" , 3 , p )	// [��Ӱ�ނ̈ꊇ�w��(̫�Ă`���{)]		��ESC ! n(=0x20)��
#define	PCMD_FONT_A_4( p )			PrnCmdLen( (uchar *)"\x1b\x21\x30" , 3 , p )	// [��Ӱ�ނ̈ꊇ�w��(̫�Ă`�S�{)]		��ESC ! n(=0x30)��
#define	PCMD_FONT_B_N( p )			PrnCmdLen( (uchar *)"\x1b\x21\x01" , 3 , p )	// [��Ӱ�ނ̈ꊇ�w��(̫�Ăa�ʏ�)]		��ESC ! n(=0x01)��
#define	PCMD_FONT_B_T( p )			PrnCmdLen( (uchar *)"\x1b\x21\x11" , 3 , p )	// [��Ӱ�ނ̈ꊇ�w��(̫�Ăa�c�{)]		��ESC ! n(=0x11)��
#define	PCMD_FONT_B_Y( p )			PrnCmdLen( (uchar *)"\x1b\x21\x21" , 3 , p )	// [��Ӱ�ނ̈ꊇ�w��(̫�Ăa���{)]		��ESC ! n(=0x21)��
#define	PCMD_FONT_B_4( p )			PrnCmdLen( (uchar *)"\x1b\x21\x31" , 3 , p )	// [��Ӱ�ނ̈ꊇ�w��(̫�Ăa�S�{)]		��ESC ! n(=0x31)��

#define	PCMD_LINECLR( p )			PrnCmdLen( (uchar *)"\x1b\x2d\x00" , 3 , p )	// [���ްײ݂̎w��(����)]				��ESC - n(=0x00)��
#define	PCMD_LINESET1( p )			PrnCmdLen( (uchar *)"\x1b\x2d\x01" , 3 , p )	// [���ްײ݂̎w��(�P�ޯĕ�)]			��ESC - n(=0x01)��
#define	PCMD_LINESET2( p )			PrnCmdLen( (uchar *)"\x1b\x2d\x02" , 3 , p )	// [���ްײ݂̎w��(�Q�ޯĕ�)]			��ESC - n(=0x02)��

#define	PCMD_KAIGYO_NORMAL( p )		PrnCmdLen( (uchar *)"\x1b\x33\x20" , 3 , p )	// [���s�ʎw��(�W��=4mm)]				��ESC 3 n(=0x20)��
#define	PCMD_KAIGYO_NORMAL_1( p )		PrnCmdLen( (uchar *)"\x1b\x33\x18" , 3 , p )	// [���s�ʎw��(�W��=4mm)]				��ESC 3 n(=0x20)��
//#define	PCMD_KAIGYO_FONTA( p )		PrnCmdLen( (uchar *)"\x1b\x33\x20" , 3 , p )	// [���s�ʎw��(̫�Ă`�����{1mm=4mm)]	��ESC 3 n(=0x20)��
//#define	PCMD_KAIGYO_FONTB( p )		PrnCmdLen( (uchar *)"\x1b\x33\x19" , 3 , p )	// [���s�ʎw��(̫�Ăa�����{1mm=3.125mm)]��ESC 3 n(=0x19)��
//#define	PCMD_KAIGYO_TATEBAI( p )	PrnCmdLen( (uchar *)"\x1b\x33\x38" , 3 , p )	// [���s�ʎw��(�c�{���������{1mm=7mm)]	��ESC 3 n(=0x38)��
//#define	PCMD_KAIGYO_KANJI( p )		PrnCmdLen( (uchar *)"\x1b\x33\x20" , 3 , p )	// [���s�ʎw��(���������{1mm=4mm)]		��ESC 3 n(=0x20)��

#define	PCMD_INIT( p )				PrnCmdLen( (uchar *)"\x1b\x40" , 2 , p )		// [������̏�����]						��ESC @��

#define	PCMD_EMPHCLR( p )			PrnCmdLen( (uchar *)"\x1b\x45\x00" , 3 , p )	// [������(����)]						��ESC E n(=0x00)��
#define	PCMD_EMPHSET( p )			PrnCmdLen( (uchar *)"\x1b\x45\x01" , 3 , p )	// [������(�w��)]						��ESC E n(=0x01)��

#define	PCMD_LEFT( p )				PrnCmdLen( (uchar *)"\x1b\x61\x00" , 3 , p )	// [�ʒu����(������)]					��ESC a n(=0x00)��
#define	PCMD_CENTER( p )			PrnCmdLen( (uchar *)"\x1b\x61\x01" , 3 , p )	// [�ʒu����(����ݸ�)]					��ESC a n(=0x01)��
#define	PCMD_RIGHT( p )				PrnCmdLen( (uchar *)"\x1b\x61\x02" , 3 , p )	// [�ʒu����(�E����)]					��ESC a n(=0x02)��

#define	PCMD_CUT( p )				PrnCmdLen( (uchar *)"\x1b\x69" , 2 , p )		// [������p�����]						��ESC i��
#define	PCMD_BS_CUT( p )			PrnCmdLen( (uchar *)"\x08\x1b\x69" , 3 , p )	// [4mm�ޯ�̨��ނ���������p�����]		��BS,ESC i��
#define	PCMD_PCUT( p )				PrnCmdLen( (uchar *)"\x1b\x6d" , 2 , p )		// [������p���߰��ٶ��]					��ESC m��
#define	PCMD_BS_PCUT( p )			PrnCmdLen( (uchar *)"\x08\x1b\x6d" , 3 , p )	// [4mm�ޯ�̨��ނ���������p���߰��ٶ��]	��BS,ESC m��

#define	PCMD_STATUS_SEND( p )		PrnCmdLen((uchar*)"\x1b\x76",2 , p )			// [������ð���̑��M]					��ESC v��

#define	PCMD_FONT_K_N( p )			PrnCmdLen( (uchar *)"\x1c\x21\x00" , 3 , p )	// [������Ӱ�ވꊇ�w��(�ʏ�)]			��FS ! n(=0x00)��
#define	PCMD_FONT_K_Y( p )			PrnCmdLen( (uchar *)"\x1c\x21\x04" , 3 , p )	// [������Ӱ�ވꊇ�w��(���{)]			��FS ! n(=0x04)��
#define	PCMD_FONT_K_T( p )			PrnCmdLen( (uchar *)"\x1c\x21\x08" , 3 , p )	// [������Ӱ�ވꊇ�w��(�c�{)]			��FS ! n(=0x08)��
#define	PCMD_FONT_K_4( p )			PrnCmdLen( (uchar *)"\x1c\x21\x0c" , 3 , p )	// [������Ӱ�ވꊇ�w��(�S�{)]			��FS ! n(=0x0c)��

#define	PCMD_KANJILINECLR( p )		PrnCmdLen( (uchar *)"\x1c\x2d\x00" , 3 , p )	// [�������ްײ݂̎w��i�����j]			��FS - n(0x00)��
#define	PCMD_KANJILINESET1( p )		PrnCmdLen( (uchar *)"\x1c\x2d\x01" , 3 , p )	// [�������ްײ݂̎w��i�P�ޯĕ��j]		��FS - n(0x01)��
#define	PCMD_KANJILINESET2( p )		PrnCmdLen( (uchar *)"\x1c\x2d\x02" , 3 , p )	// [�������ްײ݂̎w��i�Q�ޯĕ��j]		��FS - n(0x02)��

#define	PCMD_JIS( p )				PrnCmdLen( (uchar *)"\x1c\x43\x00" , 3 , p )	// [�������ޑ̌n�I��(JIS)]				��FS C n(=0x00)��
#define	PCMD_SHIFTJIS( p )			PrnCmdLen( (uchar *)"\x1c\x43\x01" , 3 , p )	// [�������ޑ̌n�I��(���JIS)]			��FS C n(=0x01)��

#define	PCMD_SPACE_KANJI( p )		PrnCmdLen( (uchar *)"\x1c\x53\x0\x0" , 4 , p )	// [������߰��ʂ̎w��(��0mm�A�E0mm)]	��FS S n1(=0x00) n2(=0x04)��

#define	PCMD_KANJI4_CLR( p )		PrnCmdLen( (uchar *)"\x1c\x57\x00" , 3 , p )	// [�����̂S�{�p����(����)]				��FS W n(=0x00)��
#define	PCMD_KANJI4_SET( p )		PrnCmdLen( (uchar *)"\x1c\x57\x01" , 3 , p )	// [�����̂S�{�p����(�w��)]				��FS W n(=0x01)��

//#define	PCMD_DLBIMG_PRI0( p )		PrnCmdLen( (uchar *)"\x1d\x2f\x00" , 3 , p )	// [�޳�۰���ޯĲҰ�ވ󎚁iɰ��Ӱ�ށj]	��GS / m(=0x00)��
//#define	PCMD_DLBIMG_PRI1( p )		PrnCmdLen( (uchar *)"\x1d\x2f\x01" , 3 , p )	// [�޳�۰���ޯĲҰ�ވ󎚁i���{Ӱ�ށj]	��GS / m(=0x01)��
//#define	PCMD_DLBIMG_PRI2( p )		PrnCmdLen( (uchar *)"\x1d\x2f\x02" , 3 , p )	// [�޳�۰���ޯĲҰ�ވ󎚁i�c�{Ӱ�ށj]	��GS / m(=0x02)��
//#define	PCMD_DLBIMG_PRI3( p )		PrnCmdLen( (uchar *)"\x1d\x2f\x03" , 3 , p )	// [�޳�۰���ޯĲҰ�ވ󎚁i�S�{Ӱ�ށj]	��GS / m(=0x03)��

#define	PCMD_STATUS_AUTO( p )		PrnCmdLen( (uchar *)"\x1d\x76\x00" , 3 , p )	// [������ð���̎������M]				��GS v NULL��

#define	PCMD_LF( p )				PrnCmdLen( (uchar *)"\x0a" , 1 , p )			// [���s]

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
// �d�q�W���[�i����p�R�}���h
#define	PINFO_SD_INFO				0x01		// SD�J�[�h���v��
#define	PINFO_MACHINE_INFO			0x02		// �@����v��
#define	PINFO_VER_INFO				0x03		// �\�t�g�o�[�W�����v��
#define	PINFO_FS_INFO				0x04		// �t�@�C���V�X�e�����v��
#define	PINFO_SD_VER_INFO			0x05		// SD�J�[�h�o�[�W�����v��
#define	PINFO_SD_TEST_RW			0x10		// SD�J�[�h�e�X�g�i���[�h���C�g�j
#define	PINFO_SD_TEST_SEQ			0x11		// SD�J�[�h�e�X�g�iSeq���\�j
#define	PINFO_SD_TEST_4KB			0x12		// SD�J�[�h�e�X�g�i4KB���\�j
#define	PINFO_SD_TEST_512B			0x13		// SD�J�[�h�e�X�g�i512B���\�j
#define	PINFO_SD_TEST_CANCEL		0x1F		// SD�J�[�h�e�X�g�i�L�����Z���j

#define	PCMD_WRITE_START(p)			PrnCmdLen((uchar*)"\x1d\x47\x21", 3, p)			// [�f�[�^�����݊J�n]	��GS G n(=0x21)��
#define	PCMD_WRITE_END(p)			PrnCmdLen((uchar*)"\x1d\x47\x20", 3, p)			// [�f�[�^�����ݏI��]	��GS G n(=0x20)��
#define	PCMD_RESET(p)				PrnCmdLen((uchar*)"\x10\x18", 2, p)				// [�\�t�g���Z�b�g]		��DLE CAN��

#define	EJA_MACHINE_INFO			"EJA"		// �@����i���3�����j
#define	isCMD_MACHINE_INFO(p)		(!memcmp(p, "\x1b\x73\x02", 3))					// �@����ʒm
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
#define	isCMD_WRITE_START(p)		(!memcmp(p, "\x1d\x47\x21", 3))					// �f�[�^�����݊J�n�R�}���h
#define	isCMD_ENCRYPT(p)			(!memcmp(p, "\x1d\x4b", 2))						// �Í����L�[�R�}���h
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��s����QR�󎚂��x���Ĉ󎚓r���ň���������j�iGM803003���p�j
#define	PCMD_WRITE_RESET(p)			PrnCmdLen((uchar*)"\x18", 1, p)					// [�y�[�W�o�b�t�@�[�N���A]		��CAN��
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��s����QR�󎚂��x���Ĉ󎚓r���ň���������j�iGM803003���p�j
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
#define	PRN_MACHINE_NEWMODEL_R		"2705-A4"										// ���f�����i7���j
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j

/*----------------------------------------------------------*/
/*	����ر�ް��l��`										*/
/*----------------------------------------------------------*/
// �W�v���(syuukei_kind)

//	���W�v��
#define		TSYOUKEI				1		// �s���v
#define		TGOUKEI					2		// �s���v
#define		GTSYOUKEI				3		// �f�s���v
#define		GTGOUKEI				4		// �f�s���v
#define		F_TSYOUKEI				5		// �����s���v
#define		F_TGOUKEI				6		// �����s���v
#define		F_GTSYOUKEI				7		// �����f�s���v
#define		F_GTGOUKEI				8		// �����f�s���v
#define		TGOUKEI_Z				9		// �O��s���v
#define		GTGOUKEI_Z				10		// �O��f�s���v
#define		F_TGOUKEI_Z				11		// �O�񕡐��s���v
#define		F_GTGOUKEI_Z			12		// �O�񕡐��f�s���v
#define		MTSYOUKEI				13		// �l�s���v
#define		MTGOUKEI				14		// �l�s���v
#define		MTGOUKEI_Z				15		// �O��l�s���v

//	����݋��ɏW�v��
#define		COKINKO_G				1		// ��݋��ɍ��v
#define		COKINKO_S				2		// ��݋��ɏ��v
#define		COKINKO_Z				3		// �O��݋��ɍ��v

//	���������ɏW�v��
#define		SIKINKO_G				1		// �������ɍ��v
#define		SIKINKO_S				2		// �������ɏ��v
#define		SIKINKO_Z				3		// �O�񎆕����ɍ��v

//	���ޑK�Ǘ��W�v��
#define		TURIKAN					1		// �ޑK�Ǘ�
#define		TURIKAN_S				2		// �ޑK�Ǘ��i���v�j

/*----------------------------------------------------------*/
/*	�֐����Ұ�												*/
/*----------------------------------------------------------*/
// ͯ�ް�^̯�����
#define		PRI_HEADER				1		// ͯ�ް
#define		PRI_HOOTER				2		// ̯��
#define		PRI_TCARD_FOOTER		3		// T���ސ�p̯��
#define		PRI_AZU_FOOTER			4		// �a��ؐ�p̯��

// �x�����ށE�������^�����X�� ���
#define		PRI_SYOMEI				1		// �x�����ށE������
#define		PRI_KAMEI				2		// �����X��
// �װ۸ޓo�^�����iPrn_errlg�j�p
#define		PRN_ERR_OFF				0		// �װ����
#define		PRN_ERR_ON				1		// �װ����
#define		PRN_ERR_ONOFF			2		// �װ����/����
#define		PRN_ERR_JOHO_NON		0		// �װ��񂠂�
#define		PRN_ERR_JOHO_ARI		1		// �װ��񂠂�

// ���ް����������iPrn_data_chk�j�p
#define		RYOUSYUU_PRN_SYU1		1		// �̎��؈󎚎�ʂP	�����v��
#define		RYOUSYUU_PRN_SYU2		2		// �̎��؈󎚎�ʂQ	�����v��
#define		PKICHI_DATA				3		// ���Ԉʒu�ް�		�����v��
#define		SEISAN_SYU				4		// ���Z���			�����v��
#define		RYOUKIN_SYU				5		// �������			�����v��
#define		TEIKI_SYU				6		// ��������		�����v��
#define		PKNO_SYU				7		// ���ԏ�m���D���	�����v��
#define		SERVICE_SYU				8		// ���޽�����		�����v��
#define		TIKUSE_CNT				9		// �������g�p����	�����v��
#define		MISE_NO					10		// �X�m���D			�����v��
#define		WARIBIKI_SYU			11		// ������ʁi�������j�����v��
// MH810100(S) m.saito 2020/05/13 �Ԕԃ`�P�b�g���X(�̎��؈󎚁F�����Z�o�ɑΉ�)
#define		UNPAID_SYU				12		// ���Z�����Z���	�����v��
// MH810100(E) m.saito 2020/05/13 �Ԕԃ`�P�b�g���X(�̎��؈󎚁F�����Z�o�ɑΉ�)

/*----------------------------------------------------------*/
/*	�e���ް��͈͒�`										*/
/*----------------------------------------------------------*/

#define		RYOUSYUU_PRN_SYU1_MIN	0		// �̎��؈󎚎�ʂP�ް��ŏ��l
#define		RYOUSYUU_PRN_SYU1_MAX	1		// �̎��؈󎚎�ʂP�ް��ő�l

#define		RYOUSYUU_PRN_SYU2_MIN	0		// �̎��؈󎚎�ʂQ�ް��ŏ��l
#define		RYOUSYUU_PRN_SYU2_MAX	3		// �̎��؈󎚎�ʂQ�ް��ő�l

#define		PKICHI_NO_MIN			0		// ���Ԉʒu�ԍ��ް��ŏ��l�i���Ԉʒu�ԍ��Ȃ��j
#define		PKICHI_NO_MAX			9999	// ���Ԉʒu�ԍ��ް��ő�l

#define		PKICHI_KU_MIN			0		// ���Ԉʒu����ް��ŏ��l�i���Ȃ��j
#define		PKICHI_KU_MAX			26		// ���Ԉʒu����ް��ő�l�i1�`26�FA�`Z�j

#define		SEISAN_SYU_MIN			0		// ���Z����ް��ŏ��l
#define		SEISAN_SYU_MAX			2		// ���Z����ް��ő�l

#define		RYOUKIN_SYU_MIN			1		// ��������ް��ŏ��l
#define		RYOUKIN_SYU_MAX			12		// ��������ް��ő�l

#define		TEIKI_SYU_MIN			1		// ���������ް��ŏ��l
#define		TEIKI_SYU_MAX			15		// ���������ް��ő�l

#define		PKNO_SYU_MIN			0		// ���ԏ�m���D����ް��ŏ��l
#define		PKNO_SYU_MAX			3		// ���ԏ�m���D����ް��ő�l

#define		SERVICE_SYU_MIN			1		// ���޽������ް��ŏ��l
#define		SERVICE_SYU_MAX			15		// ���޽������ް��ő�l

#define		TIKUSE_CNT_MIN			1		// �������g�p�����ް��ŏ��l
#define		TIKUSE_CNT_MAX			99		// �������g�p�����ް��ő�l

#define		MISE_NO_MIN				1		// �X�m���D�ް��ŏ��l
#define		MISE_NO_MAX				100		// �X�m���D�ް��ő�l

#define		WARIBIKI_SYU_MIN		1		// ������ʁi�������j�ް��ŏ��l
#define		WARIBIKI_SYU_MAX		100		// ������ʁi�������j�ް��ő�l

#define		SIHEI_SYU_MIN			1		// ��������ް��ŏ��l
#define		SIHEI_SYU_MAX			4		// ��������ް��ő�l

#define		COIN_SYU_MIN			1		// �R�C������ް��ŏ��l
#define		COIN_SYU_MAX			4		// �R�C������ް��ő�l

#define		IN_SIG_MIN				1		// ���ɐM���ް��ŏ��l
#define		IN_SIG_MAX				3		// ���ɐM���ް��ő�l

#define		BUNRUI_SYU_MIN			1		// ���ޏW�v��ʍŏ��l
#define		BUNRUI_SYU_MAX			9		// ���ޏW�v��ʍő�l

#define		BUNRUI_MIN				1		// ���ޏW�v�敪�ŏ��l
#define		BUNRUI_MAX				48		// ���ޏW�v�敪�ő�l

#define		SYUUKEI_SYU_MIN			1		// �W�v��ʍŏ��l
#define		SYUUKEI_SYU_MAX			15		// �W�v��ʍő�l

#define		COKISK_SYU_MIN			1		// ��݋��ɏW�v��ʍŏ��l
#define		COKISK_SYU_MAX			3		// ��݋��ɏW�v��ʍő�l

#define		SIKISK_SYU_MIN			1		// �������ɏW�v��ʍŏ��l
#define		SIKISK_SYU_MAX			3		// �������ɏW�v��ʍő�l

#define		HEAD_GYO_MIN			0		// ͯ�ް�󎚍s���ŏ��l
#define		HEAD_GYO_MAX			4		// ͯ�ް�󎚍s���ő�l

#define		HOOT_GYO_MIN			0		// ̯���󎚍s���ŏ��l
#define		HOOT_GYO_MAX			4		// ̯���󎚍s���ő�l
#define		SYOMEI_GYO_MIN			0		// �x�������ށE�������󎚍s���ŏ��l
#define		SYOMEI_GYO_MAX			4		// �x�������ށE�������󎚍s���ő�l

#define		KAMEI_GYO_MIN			0		// �����X���󎚍s���ŏ��l
#define		KAMEI_GYO_MAX			4		// �����X���󎚍s���ő�l

#define		ERR_INFO_SYU_MAX		99		// �װ���̴װ��ʍő�l�i�Q���j
#define		ERR_INFO_COD_MAX		99		// �װ���̴װ���ލő�l�i�Q���j
#define		ARM_INFO_SYU_MAX		99		// �װя��̱װю�ʍő�l�i�Q���j
#define		ARM_INFO_COD_MAX		99		// �װя��̱װѺ��ލő�l�i�Q���j
#define		OPE_INFO_SYU_MAX		99		// ������̑����ʍő�l�i�Q���j
#define		OPE_INFO_COD_MAX		99		// ������̑��캰�ލő�l�i�Q���j
#define		MON_INFO_SYU_MAX		99		// ���j�^���̑����ʍő�l�i�Q���j
#define		MON_INFO_COD_MAX		99		// ���j�^���̑��캰�ލő�l�i�Q���j
#define		T_FOOTER_GYO_MAX		10		// T�J�[�h�t�b�^�[�̈󎚉\MAX�s��
#define		AZU_FTR_GYO_MAX			4		// �a��؃t�b�^�[�̈󎚉\MAX�s��
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
#define		KBR_FTR_GYO_MAX			4		// �ߕ����t�b�^�[�̈󎚉\MAX�s��
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
#define		FUTURE_FTR_GYO_MAX		4		// ����x���z�̈󎚉\MAX�s��
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
#define		EMGFOOT_GYO_MAX			4		// ��Q�A���[̯���󎚍s���ő�l
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
#define		RMON_INFO_SYU_MAX		99		// ���u�Ď����̎�ʍő�l
#define		RMON_INFO_COD_MAX		99		// ���u�Ď����̃R�[�h�ő�l

// MH810100(S) m.saito 2020/05/13 �Ԕԃ`�P�b�g���X(�̎��؈󎚁F�����Z�o�ɑΉ�)
#define		UNPAID_SYU_MIN			0		// ���Z�����Z��ʍŏ��l
// GG129000(S) T.Nagai 2023/02/15 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
//#define		UNPAID_SYU_MAX			1		// ���Z�����Z��ʍő�l
#define		UNPAID_SYU_MAX			2		// ���Z�����Z��ʍő�l
// GG129000(E) T.Nagai 2023/02/15 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���u���Z�Ή��j
// MH810100(E) m.saito 2020/05/13 �Ԕԃ`�P�b�g���X(�̎��؈󎚁F�����Z�o�ɑΉ�)

/*--------------------------------------------------*/
/*	��������䕔�ް��\���̒�`						*/
/*--------------------------------------------------*/

	// ���������M�ް��Ǘ��ޯ̧̫�ϯ�
	typedef struct {
		uchar	PrnBuf[PRNQUE_CNT][PRNBUF_SIZE];	// ��������M�ް��ޯ̧
		ushort	PrnBufWriteCnt;						// ��������M�ް��ޯ̧�o�^��
		ushort	PrnBufReadCnt;						// ��������M�ް��ޯ̧�Ǎ���
		ushort	PrnBufCnt;							// �P�ް��ޯ̧�̾��
		ushort	PrnBufCntLen[PRNQUE_CNT];			// ��������M�ް��ޯ̧�o�^�ް��ݸ޽
		uchar	PrnState[3];						// ������ð�� [0]:new,[1]:old [2]:MTB(Many Times Before)
		uchar	PrnStateRcvCnt;						// ������ð����M��
		uchar	PrnStateMnt;						// ������ð��			����������ݽ�p
		uchar	PrnStWork;							// ������ð��work
		uchar	PrnBusyCnt;							// ������޼ް�p���Ď�����
	}PRN_DATA_BUFF;

	// ������ҏW���������ް�̫�ϯ�
	typedef struct {
		ushort	Printing;			// �󎚏�����ԁi��������ү���޺���ނ��i�[�j
		ushort	Tyushi_Cmd;			// �󎚒��~ү���ގ�M�׸ށiON�F��M�^OFF�F����M�j
		ushort	Split;				// �󎚏���������ۯ���
		ushort	Final;				// �����󎚍ŏI��ۯ��ʒm�iON�F�ŏI��ۯ��ҏW�I���j
		ushort	EditWait;			// ���ް��ҏW�҂���ԁiON�F���ް��ҏW�x����ϰ�N�����j

		ushort	Prn_no;				// �󎚏����ς݂̍��ڔԍ�
		ushort	Prn_no_data1;		// �󎚏����ς݂̍��ڔԍ���ޏ��P
		ushort	Prn_no_data2;		// �󎚏����ς݂̍��ڔԍ���ޏ��Q
		ushort	Prn_no_wk[10];		// �󎚏����ėpܰ�

		uchar	Log_Start;			// ۸��ް��󎚊J�n�׸�	�iOFF:����̕ҏW�^ON:�Q��ڈȍ~�̕ҏW�j
		uchar	Log_Edit;			// ۸��ް��ҏW���׸�	�iON:�ҏW���j
		ushort	Log_Count;			// ۸��ް���
		ushort	Log_DataPt;			// ۸��ް��߲��
		ushort	Log_Sdate;			// ۸��ް������J�n��
		ushort	Log_Edate;			// ۸��ް������I����
		ushort	Log_Stime;			// ۸��ް������J�n����
		ushort	Log_Etime;			// ۸��ް������I������
		ushort	Log_no_wk[2];		// ۸ވ󎚏����ėpܰ�
		ushort	Log_Count_inFROM;	// (FlashROM��)۸��ް���	(0-720)
		ushort	Log_DataPt_inFROM;	// (FlashROM��)۸��ް��߲�� (0-719)
		ushort	Log_TotalCount;		// ۸��ް��� (SRAM+FlashROM)(0-839)

		uchar	Font_size;			// ̫�ĕ������ށi������Ɏw�肵��������ʂ��i�[�j
		uchar	Kanji_size;			// ����̫�Ļ��ށi������Ɏw�肵��������ʂ��i�[�j
		uchar	Kaigyou_size;		// ���s���ށi������Ɏw�肵�����s�ʁi���j���ޯĐ��P�ʂŊi�[�j
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
		ulong	Prn_Job_id;			// �W���uID
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
	}PRN_PROC_DATA;

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
	// �d�q�W���[�i����M�f�[�^�Ǘ��o�b�t�@
	typedef struct {
		short	EjaUseFlg;						// �d�q�W���[�i���ڑ��t���O
		uchar	PrnInfoReq;						// �v�����^���v�����t���O
		uchar	PrnRcvBufReadCnt;				// ��M�o�b�t�@���[�h�J�E���g
		uchar	PrnHResetRetryCnt;				// �n�[�h���Z�b�g�̃��g���C��
		uchar	PrnSResetRetryCnt;				// �\�t�g���Z�b�g�̃��g���C��
		uchar	PrnClkReqFlg;					// ���v�ݒ�v���t���O
		uchar	PrnInfReqFlg;					// �v�����^���v���t���O�i�O���j
		uchar	PrnRcvBuf[PRN_EJA_BUF_SIZE];	// ��M�o�b�t�@
		PRN_PROC_DATA	eja_proc_data;			// �f�[�^�����݊J�n���̈󎚏�������f�[�^
		MSG		PrnMsg;							// �f�[�^�����ݒ��̃��b�Z�[�W
	} PRN_EJA_DATA_BUFF;
// MH810104 GG119201(E) �d�q�W���[�i���Ή�

// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
#define	PRN_JOB_ID_MAX		999999
#define	PRN_QUE_COUNT_MAX	1
#define	PRN_QUE_DAT_SIZE	MSGBUFSZ
#define	PRN_QUE_OFFSET		2

	typedef struct {
		ushort	command;						// �󎚗v���R�}���h
		uchar	prndata[PRN_QUE_DAT_SIZE];		// �󎚃f�[�^
	} PRN_DAT;
	typedef struct {
		ushort	count;							// �f�[�^����
		ushort	wtp;							// ���C�g�|�C���^
		ushort	rdp;							// ���[�h�|�C���^
		PRN_DAT	prn_dat[PRN_QUE_COUNT_MAX];
	} PRN_DAT_QUE;
extern PRN_DAT_QUE	prn_dat_que;
extern ulong	prn_job_id;
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
	// �d�q�W���[�i����M�f�[�^�Ǘ��o�b�t�@
	typedef struct {
		short	NewModelFlg;					// �V���f���t���O�i-1=�������O�^0=�]���i�^�C���A�E�g�j�^1=�V�j
		uchar	ModelMissFlg;					// ���f���s��v�t���O�i0=OK�^1=�s��v�G���[�j
		uchar	PrnInfoReq;						// �v�����^���v�����t���O
		uchar	PrnRcvBufReadCnt;				// ��M�o�b�t�@���[�h�J�E���g
		uchar	PrnRcvBuf[PRN_RCV_BUF_SIZE_R];	// ��M�o�b�t�@
	} PRN_RCV_DATA_BUFF_R;
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j

/*----------------------------------------------*/
/*	��������䕔�F�֐���������					*/
/*----------------------------------------------*/												
/*-------	pritask.c	-------*/
extern	void	prn_int( void );							// ���������������
extern	void	pritask( void );							// ���������Ҳݏ���

extern	uchar	TgtMsGet		( MsgBuf *msb );			// �����Ώ�ү���ގ擾
extern	uchar	TgtMsgChk		( MsgBuf *msb );			// �����Ώ�ү���ޔ���
extern	uchar	RcvCheckWait	( MSG *msg );				// �󎚏����^�s������
extern	uchar	PriRctCheck		( void );					// ڼ�Ĉ󎚏����^�s������

extern	void	PrnRYOUSYUU		( MSG *msg );				// ���̎��؈󎚗v����				ү���ގ�M����
extern	void	PrnAZUKARI		( MSG *msg );				// ���a��؈󎚗v����				ү���ގ�M����
extern	void	PrnUKETUKE		( MSG *msg );				// ����t���󎚗v����				ү���ގ�M����
extern	void	PrnSYUUKEI		( MSG *msg );				// ���W�v�󎚗v����					ү���ގ�M����
extern	void	PrnSYUUKEI_LOG	( MSG *msg );				// ���W�v�����󎚗v����				ү���ގ�M����
extern	void	PrnCOKI_SK		( MSG *msg );				// ����݋��ɏW�v�󎚗v����			ү���ގ�M����
extern	void	PrnCOKI_JO		( MSG *msg );				// ����݋��ɏW�v���󎚗v����		ү���ގ�M����
extern	void	PrnSIKI_SK		( MSG *msg );				// ���������ɏW�v�󎚗v����			ү���ގ�M����
extern	void	PrnSIKI_JO		( MSG *msg );				// ���������ɏW�v���󎚗v����		ү���ގ�M����
extern	void	PrnTURIKAN		( MSG *msg );				// ���ޑK�Ǘ��W�v�󎚗v����			ү���ގ�M����
extern	void	PrnTURIKAN_LOG	( MSG *msg );				// ���ޑK�Ǘ��W�v�����󎚗v����		ү���ގ�M����
extern	void	PrnERR_JOU		( MSG *msg );				// ���װ���󎚗v����				ү���ގ�M����
extern	void	PrnERR_LOG		( MSG *msg );				// ���װ��񗚗��󎚗v����			ү���ގ�M����
extern	void	PrnARM_JOU		( MSG *msg );				// ���װя��󎚗v����				ү���ގ�M����
extern	void	PrnARM_LOG		( MSG *msg );				// ���װя�񗚗��󎚗v����			ү���ގ�M����
extern	void	PrnOPE_JOU		( MSG *msg );				// ��������󎚗v����				ү���ގ�M����
extern	void	PrnOPE_LOG		( MSG *msg );				// �������񗚗��󎚗v����			ү���ގ�M����
extern	void	PrnMON_JOU		( MSG *msg );				// �����j�^���󎚗v����			ү���ގ�M����
extern	void	PrnMON_LOG		( MSG *msg );				// �����j�^��񗚗��󎚗v����		ү���ގ�M����
extern	void	PrnSETTEI		( MSG *msg );				// ���ݒ��ް��󎚗v����				ү���ގ�M����
extern	void	PrnDOUSAC		( MSG *msg );				// �����춳�Ĉ󎚗v����				ү���ގ�M����
extern	void	PrnLOCK_DCNT	( MSG *msg );				// ��ۯ����u���춳�Ĉ󎚗v����		ү���ގ�M����
extern	void	PrnLOCK_PARA	( MSG *msg );				// ���Ԏ����Ұ��󎚗v����			ү���ގ�M����
extern	void	PrnLOCK_SETTEI	( MSG *msg );				// ��ۯ����u�ݒ�󎚗v����			ү���ގ�M����
extern	void	PrnPKJOU_NOW	( MSG *msg );				// ���Ԏ����i���݁j�󎚗v����		ү���ގ�M����
extern	void	PrnPKJOU_SAV	( MSG *msg );				// ���Ԏ����i�ޔ��j�󎚗v����		ү���ގ�M����
extern	void	PrnTEIKIDATA1	( MSG *msg );				// ������L���^�����󎚗v����		ү���ގ�M����
extern	void	PrnTEIKIDATA2	( MSG *msg );				// ��������Ɂ^�o�Ɉ󎚗v����		ү���ގ�M����
extern	void	PrnSVSTIK_KIGEN	( MSG *msg );				// �����޽�������󎚗v����			ү���ގ�M����
extern	void	PrnSPLDAY		( MSG *msg );				// �����ʓ��󎚗v����				ү���ގ�M����
extern	void	PrnTEIRYUU_JOU	( MSG *msg );				// ���◯�ԏ��󎚗v����			ү���ގ�M����
extern	void	PrnTEIFUK_JOU	( MSG *msg );				// ���╜�d���󎚗v����			ү���ގ�M����
extern	void	PrnTEIFUK_LOG	( MSG *msg );				// ���╜�d�����󎚗v����			ү���ގ�M����
extern	void	PrnSEISAN_LOG	( MSG *msg );				// ���ʐ��Z���󎚗v����			ү���ގ�M����
extern	void	PrnFUSKYO_JOU	( MSG *msg );				// ���s���E�����o�ɏ��󎚗v����	ү���ގ�M����
extern	void	PrnFUSKYO_LOG	( MSG *msg );				// ���s���E�����o�ɗ����󎚗v����	ү���ގ�M����
extern	void	PrnANYDATA		( MSG *msg );				// ���e���ް��i�[�����󎚗v����		ү���ގ�M����
extern	void	PrnIREKAE_TEST	( MSG *msg );				// ���p�����֎�ýĈ󎚗v����		ү���ގ�M����
extern	void	PrnPRINT_TEST	( MSG *msg );				// �������ýĈ󎚗v����				ү���ގ�M����
extern	void	PrnLOGO_REGIST	( MSG *msg );				// �����S�󎚃f�[�^�o�^�v����		ү���ގ�M����
extern	void	PrnRMON_JOU		( MSG *msg );				// �����u�Ď����󎚗v����			ү���ގ�M����
extern	void	PrnRMON_LOG		( MSG *msg );				// �����u�Ď���񗚗��󎚗v����		ү���ގ�M����

extern	void	PrnINJI_TYUUSHI	( MSG *msg );				// ���󎚒��~��						ү���ގ�M����
extern	void	PrnINJI_TYUUSHI_R( MSG *msg );				// ���󎚒��~�iڼ�āj��				ү���ގ�M����
extern	void	PrnINJI_TYUUSHI_J( MSG *msg );				// ���󎚒��~�i�ެ��فj��			ү���ގ�M����
extern	void	PrnINJI_END		( MSG *msg );				// ���󎚕ҏW�ް��P��ۯ��󎚏I����	ү���ގ�M����
extern	void	PrnCHARGESETUP	( MSG *msg );				// �������ݒ�󎚗v����				ү���ގ�M����
void	PrnSetDiffLogPrn( MSG *msg );
extern	void	RT_PrnSEISAN_LOG( MSG *msg );				// �������e�X�g���󎚗v����		ү���ގ�M����

extern	void	PrnERREND_PROC	( void );					// �󎚈ُ�I�����䏈��

extern	uchar	PrnRYOUSYUU_datachk	( Receipt_data *Ryousyuu );			// �̎��؈�		�v���ް���������
extern	uchar	PrnAZUKARI_datachk	( Azukari_data *Azukari );			// �a��؈�		�v���ް���������
extern	uchar	PrnUKETUKE_datachk	( Uketuke_data *Uketuke );			// ��t����		�v���ް���������
extern	uchar	PrnSYUUKEI_datachk	( SYUKEI *Syuukei );				// �W�v��			�v���ް���������
extern	uchar	PrnCOKI_SK_datachk	( COIN_SYU *Coinkinko );			// ��݋��ɏW�v		�v���ް���������
extern	uchar	PrnSIKI_SK_datachk	( NOTE_SYU *Siheikinko );			// �������ɏW�v		�v���ް���������
extern	uchar	PrnTURIKAN_datachk	( TURI_KAN *TuriKan );				// �ޑK�Ǘ��W�v		�v���ް���������
extern	uchar	PrnERRJOU_datachk	( Err_log *Errlog );				// �װ���			�v���ް���������
extern	uchar	PrnERRLOG_datachk	( T_FrmLogPriReq2 *msg_data );		// �װ��񗚗�		�v���ް���������
extern	uchar	PrnARMJOU_datachk	( Arm_log *Errlog );				// �װя��			�v���ް���������
extern	uchar	PrnARMLOG_datachk	( T_FrmLogPriReq2 *msg_data );		// �װя�񗚗�		�v���ް���������
extern	uchar	PrnOPEJOU_datachk	( Ope_log *Errlog );				// ������			�v���ް���������
extern	uchar	PrnOPELOG_datachk	( T_FrmLogPriReq2 *msg_data );		// �����񗚗�		�v���ް���������
extern	uchar	PrnMONJOU_datachk	( Mon_log *Monlog );				// ������			�v���ް���������
extern	uchar	PrnMONLOG_datachk	( T_FrmLogPriReq2 *msg_data );		// �����񗚗�		�v���ް���������
extern	uchar	PrnTEIKID1_datachk	( T_FrmTeikiData1 *msg_data );		// ����L���^����	�v���ް���������
extern	uchar	PrnTEIKID2_datachk	( T_FrmTeikiData2 *msg_data );		// ������Ɂ^�o��	�v���ް���������
extern	uchar	PrnTFJOU_datachk	( Pon_log *Ponlog );				// �╜�d���		�v���ް���������
extern	uchar	PrnFKJOU_datachk	( flp_log *fuskyo );				// �s���E�������	�v���ް���������
extern	uchar	PrnRMONJOU_datachk	( Rmon_log *Monlog );				// ���u�Ď����		�v���ް���������
extern	uchar	PrnRMONLOG_datachk	( T_FrmLogPriReq2 *msg_data );		// ���u�Ď���񗚗�	�v���ް���������
extern	void	PriErrRecieve( void );
extern	void	PrnSuica_LOG		( MSG *msg );						// ��Suica�ʐM���O�󎚗v����				ү���ގ�M����
extern	void	PrnSuica_LOG2		( MSG *msg );						// ��Suica�ʐM���O���߃f�[�^�󎚗v����		ү���ގ�M����
extern	void	PrnCRE_USE( MSG *msg );									// ���ڼޯė��p�����ް��󎚗v����			ү���ގ�M����
extern	void	PrnCRE_UNSEND( MSG *msg );								// ���ڼޯĔ���˗��ް��󎚗v����			ү���ގ�M����
extern	void	PrnCRE_SALENG( MSG *msg );								// ���ڼޯĔ��㋑���ް��󎚗v����			ү���ގ�M����
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//extern	void	PrnEdy_Status		( MSG *msg );						// ��Edy�ݒ聕�X�e�[�^�X�󎚗v����			ү���ގ�M����
//extern	void	PrnEdyArmPri		( MSG *msg );						// ��Edy�װ�ڼ�Ĉ󎚗v����				ү���ގ�M����
//extern	uchar	Prn_edit_EdyKoteiMsg( uchar pri_kind );					// �Œ�ү���ޕҏW
//extern	uchar	Edy_ArmEdit_Hakkenj( edy_arm_log *Armdata, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// Edy�p�������
//extern	uchar	Edy_ArmEdit_Teiki( edy_arm_log *Armdata, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// Edy�p�������ʁA�_��m���D						
//extern	uchar	Prn_Line_edit( uchar pri_kind, uchar Line_kind );		// �d�؂���ҏW						
//extern	void	Prn_Edy_Err_SupplEdit( uchar Err_syu, uchar Err_cod, uchar* errdat );	// �װ�⑫���󎚏���
//extern	void	PrnEDYARMPAY_LOG( MSG *msg );
//extern	void	PrnEdyShimePri		( MSG *msg );						// ��Edy���ߋL�^ڼ�Ĉ󎚗v����				ү���ގ�M����
//extern	void	PrnEDYSHIME_LOG( MSG *msg );
//extern	uchar	Edy_Ryosyu_Edit( Receipt_data *Ryousyuu, uchar pri_kind );
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
extern	void	PrnArcModVer( MSG *msg );

extern	void	PrnDigi_USE( MSG *msg );								// ���d�������p���׈󎚗v����
extern	void	PrnDigi_SYU( MSG *msg );
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//extern	void	EdyUseLog_edit( T_FrmLogPriReq4 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
// MH810103 MHUT40XX(S) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
//extern	void	SuicaUseLog_edit( T_FrmLogPriReq4 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
extern	void	SuicaUseLog_edit( T_FrmLogPriReq4 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data, short log_id );
// MH810103 MHUT40XX(E) �݂Ȃ����ς̏W�v��V�K�ɐ݂���
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//extern	void	EdySyuLog_edit( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
extern	void	SuicaSyuLog_edit( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
// MH321800(S) G.So IC�N���W�b�g�Ή�
extern	void	EcSyuLog_edit( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
extern	void	PrnEcAlmRctPri( MSG *msg );								// ���σ��[�_�A���[�����V�[�g�󎚗v��
extern	void	PrnEcBrandPri( MSG *msg );								// ���σ��[�_�u�����h��Ԉ󎚗v��
// MH810103 GG119202(S) IC�N���W�b�g�݂Ȃ����ψ󎚃t�H�[�}�b�g�ύX�Ή�
//extern	void	PrnEcDeemedJnlPri( MSG *msg );							// ���σ��[�_�݂Ȃ����ϕ��d���O�󎚗v��
// MH810103 GG119202(E) IC�N���W�b�g�݂Ȃ����ψ󎚃t�H�[�}�b�g�ύX�Ή�
extern	void	PrnRecvDeemedDataPri( MSG *msg );						// ���σ��[�_���ϐ��Z���~(������)�f�[�^�󎚗v��
// MH321800(E) G.So IC�N���W�b�g�Ή�
extern	uchar	is_match_receipt(Receipt_data* p_dat, ulong Sdate, ulong Edate, ushort MsgId);
extern	void	PrnRrokin_Setlog(MSG *msg);
extern	void	PrnSettei_Chk(MSG *msg);
extern	void	PrnFlapLoopData(MSG *msg);
extern	void	PrnMntStack(MSG *msg);
extern	uchar Cancel_repue_chk(T_FrmPrnStop* data);
extern	char PreqMsgChk( char kind );
extern	void	PrnChkResult(MSG *msg);
extern	void	PrnPushFrontMsg( MsgBuf *msg );
extern	void	PrnDeleteExclusiveMsg( void );
// MH321800(S) G.So IC�N���W�b�g�Ή�
extern	void	PrnEcAlmRctLogPri( MSG *msg );
// MH321800(E) G.So IC�N���W�b�g�Ή�

// MH810105(S) R.Endo 2021/12/01 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
extern	void	PrnDEBUG( MSG *msg );
// MH810105(E) R.Endo 2021/12/01 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
extern	void	PrnFailureContactDataPri( MSG *msg );					// ���σ��[�_��Q�A���[�f�[�^�󎚗v��
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�

/*-------	Pri_Edit.c	-------*/
extern	uchar	RYOUSYUU_edit			( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �̎��؈��ް��ҏW����
extern	uchar	RYOUSYUU_edit_hyoudai	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �\��i�̎��؁j
extern	uchar	SEICHUSI_edit_hyoudai	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �\��i���Z���~�j
extern	uchar	RYOUSYUU_edit_seisanj	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ���Z���
// GG129000(S) H.Fujinaga 2023/01/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�������󎚑Ή��j
extern	uchar	RYOUSYUU_edit_hakkenj	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �������
// GG129000(E) H.Fujinaga 2023/01/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�������󎚑Ή��j
extern	uchar	RYOUSYUU_edit_intime	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ���Ɏ���
extern	uchar	RYOUSYUU_edit_outtime	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �o�Ɏ���
extern	uchar	RYOUSYUU_edit_parktime	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ���Ԏ���
extern	uchar	RYOUSYUU_edit_churyo	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ���ԗ���
extern	uchar	RYOUSYUU_edit_tupdryo	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ����X�V����
extern	uchar	RYOUSYUU_edit_teiki		( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �������ʁA�_��m���D
extern	uchar	RYOUSYUU_edit_service	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ���޽��
extern	uchar	RYOUSYUU_edit_misewari	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �X����
extern	uchar	RYOUSYUU_edit_waribiki	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ������
extern	uchar	RYOUSYUU_edit_pripay	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pri_no );	// ����߲�޶���
extern	uchar	RYOUSYUU_edit_kaisuu	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pri_no );	// �񐔌�
extern	uchar	RYOUSYUU_edit_btwari	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �O�񎞊Ԋ���
extern	uchar	RYOUSYUU_edit_brwari	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �O�񗿋�����
extern	uchar	RYOUSYUU_edit_bpwari	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �O�񁓊���
extern	uchar	RYOUSYUU_edit_tax		( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �����
extern	uchar	RYOUSYUU_edit_ccpay		( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �ڼޯĶ��ގx���z
extern	uchar	RYOUSYUU_edit_goukei	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ���v���z
extern	uchar	RYOUSYUU_edit_azukari	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �a����z
extern	uchar	RYOUSYUU_edit_turisen	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �ނ�K�z�i�̎��؁j
extern	uchar	SEICHUSI_edit_turisen	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �ނ�K�z�i���Z���~�j
extern	uchar	RYOUSYUU_edit_turikire	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �ނ�؂���i�̎��؁j
extern	uchar	SEICHUSI_edit_turikire	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �ނ�؂���i���Z���~�j
extern	uchar	RYOUSYUU_edit_ccname	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �ڼޯĶ��މ�Ж�
extern	uchar	RYOUSYUU_edit_ccid		( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �ڼޯĶ��މ���m���D
extern	uchar	RYOUSYUU_edit_ccslno	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �ڼޯĶ��ޓ`�[�m���D
extern	uchar	RYOUSYUU_edit_ccapno	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �ڼޯĶ��ޏ��F�m���D
extern	uchar	RYOUSYUU_edit_syuuseij	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �C�����Z���
extern	uchar	RYOUSYUU_edit_pkno		( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ���Ԉʒu�ԍ�
extern	uchar	RYOUSYUU_edit_tupdmsg	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ����X�V���̊����ē����b�Z�[�W
extern	uchar	RYOUSYUU_edit_tukimsg	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar msg_no );	// �ނ�؂ꎞ�̌Œ�ү����
// �d�l�ύX(S) K.Onodera 2016/10/31 #1531 �̎��؃t�b�^�[���󎚂���
extern	uchar	RYOUSYUU_edit_Kbrmsg	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
// �d�l�ύX(E) K.Onodera 2016/10/31 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
extern	uchar	RYOUSYUU_edit_Futuremsg	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
extern	uchar	RYOUSYUU_edit_tupdlvlmsg( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ����X�V���̃��x�����s���b�Z�[�W
extern	uchar	RYOUSYUU_edit_tupdngmsg	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ������X�V�s���̃��b�Z�[�W

extern	uchar	RYOUSYUU_edit_suica     ( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pri_no );
extern	uchar	Electron_Ryosyu_Edit	( Receipt_data *Ryousyuu, uchar pri_kind );								// �d�q�}��"�V"���Z���ҏW����
// MH321800(S) G.So IC�N���W�b�g�Ή�
extern	uchar	ElectronEc_Ryosyu_Edit	( Receipt_data *Ryousyuu, uchar pri_kind,  PRN_PROC_DATA *p_proc_data );	// �d�q�}��"�V"���Z���ҏW����
// MH321800(E) G.So IC�N���W�b�g�Ή�
extern	uchar	RYOUSYUU_edit_content	( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar edit_no );	// ���p���e�󎚏���
extern	uchar	RYOUSYUU_edit_content_sub( char *first_work, ulong second_work, ulong third_work, uchar fourth_work, uchar pri_kind );	// ���p���e�󎚃f�[�^�o�^����
// MH810105(S) MH364301 �C���{�C�X�Ή�
//extern	void	RYOUSYUU_edit_title_sub	( uchar type,	char	*buf );											// ���v���z���ږ��ҏW����
extern	void	RYOUSYUU_edit_title_sub( Receipt_data *Ryousyuu, uchar type, char *buf, PRN_PROC_DATA *p_proc_data );	// ���v���z���ږ��ҏW����
// MH810105(E) MH364301 �C���{�C�X�Ή�

extern	uchar	RYOUSYUU_edit_cctno		( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �N���W�b�g�[�����ʔԍ�
// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(CCT)
//extern	uchar	RYOUSYUU_edit_cncno		( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �N���W�b�g�J�[�h�Z���^���������E�Z���^�ʔ�
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(CCT)
extern	uchar	RYOUSYUU_edit_ShopAccountNo( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data ); // �N���W�b�g�����X����ԍ�
// MH810100(S) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)
extern	uchar	RYOUSYUU_edit_Kaimono( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ��������
extern	uchar	SYUUKEI_edit_KaimonoWari( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// ��������
// MH810100(E) 2020/07/10 �Ԕԃ`�P�b�g���X(#4531 �d�l�ύX ���������̗̎��؂̈󎚂𕪂��Ĉ󎚂���)

extern	void	AZUKARI_edit			( Azukari_data *Azukari, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �a��؈��ް��ҏW

extern	void	UKETUKE_edit			( Uketuke_data *Uketuke, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ��t�����ް��ҏW

extern	uchar	SYUUKEI_edit			( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// �W�v���ް��ҏW
extern	uchar	SYUUKEI_edit_hyoudai	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// �\��
extern	uchar	SYUUKEI_edit_GenUriTryo	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ����������z
extern	uchar	SYUUKEI_edit_KakeUriTryo( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ���|���z
extern	uchar	SYUUKEI_edit_UriTryo	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ������z
extern	uchar	SYUUKEI_edit_TaxTryo	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ���ŋ��z
extern	uchar	SYUUKEI_edit_SeiTcnt	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// �����Z��
extern	uchar	SYUUKEI_edit_InOutTcnt	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// �����E�o�ɑ䐔
extern	uchar	SYUUKEI_edit_HunSei		( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// �������Z
extern	uchar	SYUUKEI_edit_RsyuSei	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ������ʖ����Z
extern	uchar	SYUUKEI_edit_RsyuWari	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ������ʖ�����
extern	uchar	SYUUKEI_edit_CcrdSei	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// �ڼޯĶ��ސ��Z
extern	uchar	SYUUKEI_edit_PcrdUse	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pkno_syu );	// ����߲�޶��ގg�p
extern	uchar	SYUUKEI_edit_KtikUse	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pkno_syu );	// �񐔌��g�p
extern	uchar	SYUUKEI_edit_StikUse	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pkno_syu );	// ���޽���g�p
extern	uchar	SYUUKEI_edit_MnoUse		( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pkno_syu );	// �X�m���D����
extern	uchar	SYUUKEI_edit_WtikUse	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pkno_syu );	// �������g�p
extern	uchar	SYUUKEI_edit_TeikiUse	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pkno_syu );	// ������g�p
extern	uchar	SYUUKEI_edit_TeikiKou	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pkno_syu );	// ������X�V
extern	uchar	SYUUKEI_edit_BunruiSk	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ���ޏW�v
extern	void	BunruiSk_Kubun_edit1	( uchar syu, uchar no, char *p_str );												// ���ޏW�v�W�v�͈͕ҏW�P
extern	void	BunruiSk_Kubun_edit2	( uchar syu, char *p_str );															// ���ޏW�v�W�v�͈͕ҏW�Q
extern	void	BunruiSk_Kubun_edit3	( uchar syu, char *p_str );															// ���ޏW�v�W�v�͈͕ҏW�R
extern	uchar	SYUUKEI_edit_RyousyuuPriCnt( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// �̎��ؔ��s��
extern	uchar	SYUUKEI_edit_InSvstSei	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ���޽��ѓ����Z��
extern	uchar	SYUUKEI_edit_InLagtSei( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );						// ׸���ѓ����Z��
extern	uchar	SYUUKEI_edit_ApassOffSei( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ��������߽OFF���Z��
extern	uchar	SYUUKEI_edit_HaraiHusoku( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ���o�s��
extern	uchar	SYUUKEI_edit_KinkoTryo	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ���ɑ������z
extern	uchar	SYUUKEI_edit_SiheiKinko	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ��������
extern	uchar	SYUUKEI_edit_CoinKinko	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// �R�C������
extern	uchar	SYUUKEI_edit_KinsenData	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ���K�f�[�^
extern	uchar	KinsenData_Coin			( ulong *data, uchar pri_kind );													// ���K�f�[�^�R�C���i���했�j�ҏW
extern	uchar	SYUUKEI_edit_SeiChusi	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ���Z���~
extern	uchar	SYUUKEI_edit_JuGouCnt	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// �z�R�C�����v��
extern	uchar	SYUUKEI_edit_ShGouCnt	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// �������o�@���v��
extern	uchar	SYUUKEI_edit_CkGouCnt	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// �R�C�����ɍ��v��
extern	uchar	SYUUKEI_edit_SkGouCnt	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// �������ɍ��v��
extern	uchar	SYUUKEI_edit_TuriModRyo	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// �ޑK���ߊz
extern	uchar	SYUUKEI_edit_AzukariPriCnt	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// �a��ؔ��s��
extern	uchar	SYUUKEI_edit_KyouseiOut	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// �����o��
extern	uchar	SYUUKEI_edit_HuseiOut	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// �s���o��
extern	uchar	SYUUKEI_edit_SyuuseiSei	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// �C�����Z
extern	uchar	SYUUKEI_edit_UketukePriCnt	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// ��t�����s��
extern	uchar	SYUUKEI_edit_ItibetuSk	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ���Ԉʒu�ʏW�v
extern	uchar	SYUUKEI_edit_MinyuSei( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );						// �������񐔁E���z

extern	uchar	SYUUKEI_edit_Electron	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// Suica�g�p
// MH321800(S) hosoda IC�N���W�b�g�Ή�
extern	uchar	SYUUKEI_edit_ElectronEc(SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data);						// ���σ��[�_
// MH321800(E) hosoda IC�N���W�b�g�Ή�
extern	uchar	SYUUKEI_edit_Electron2	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// PASMO�g�p
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//extern	uchar	SYUUKEI_edit_Electron3	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// Edy�g�p
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

extern	uchar	SYUUKEI_edit_Gengaku	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ���z���Z
extern	uchar	SYUUKEI_edit_Furikae	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// �U�֐��Z
// MH322914(S) K.Onodera 2016/12/08 AI-V�Ή�
extern	uchar	SYUUKEI_edit_Kabarai	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
// MH322914(E) K.Onodera 2016/12/08 AI-V�Ή�
extern	uchar	SYUUKEI_edit_LagExtCnt	( SYUKEI *Syuukei, uchar pri_kind, PRN_PROC_DATA *p_proc_data );					// ���O�^�C��������

extern	void	SYUUKEILOG_edit			( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );			// �W�v�������ް��ҏW

extern	void	CoKiSk_edit				( COIN_SYU *Coinkinko, uchar pri_kind, PRN_PROC_DATA *p_proc_data );			// ��݋��ɏW�v���ް��ҏW
extern	void	CoKiJo_edit				( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// ��݋��ɏW�v�����ް��ҏW
extern	void	CoKiSk_edit_sub			( COIN_SYU *Coinkinko, uchar pri_kind );										// ��݋��ɏW�v���ް��ҏW���

extern	void	SiKiSk_edit				( NOTE_SYU *Siheikinko, uchar pri_kind, PRN_PROC_DATA *p_proc_data );			// �������ɏW�v���ް��ҏW
extern	void	SiKiJo_edit				( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// �������ɏW�v�����ް��ҏW
extern	void	SiKiSk_edit_sub			( NOTE_SYU *Siheikinko, uchar pri_kind );										// �������ɏW�v���ް��ҏW���

extern	uchar	TURIKAN_edit			( TURI_KAN *TuriKan, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// �ޑK�Ǘ��W�v���ް��ҏW����
extern	uchar	TuriKanData_Coin		( TURI_KAN *TuriKan, uchar coin_syu, uchar pri_kind );							// �ޑK�Ǘ����했���ް��ҏW����
extern	void	TURIKANLOG_edit			( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// �ޑK�Ǘ��W�v�������ް��ҏW	

extern	void	ERRJOU_edit				( Err_log *Errlog, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// �װ�����ް��ҏW����
extern	void	ERRLOG_edit				( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// �װ��񗚗����ް��ҏW����
extern	void	ERRJOU_edit_data		( Err_log *Errlog, uchar pri_kind, uchar type );								// �װ���i�P�ް��j�ҏW����

extern	void	ARMJOU_edit				( Arm_log *Armlog, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// �װя����ް��ҏW����
extern	void	ARMLOG_edit				( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// �װя�񗚗����ް��ҏW����
extern	void	ARMJOU_edit_data		( Arm_log *Errlog, uchar pri_kind );											// �װя��i�P�ް��j�ҏW����

extern	void	OPEJOU_edit				( Ope_log *Opelog, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// ��������ް��ҏW����
extern	void	OPELOG_edit				( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// �����񗚗����ް��ҏW����
extern	void	OPEJOU_edit_data		( Ope_log *Errlog, uchar pri_kind );											// ������i�P�ް��j�ҏW����

extern	void	NgLog_edit				( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// �s�������O���ް��ҏW
extern	ushort	LogPtrGet				( ushort cnt, ushort wp, ushort max, uchar req );
extern	ushort	UsMnt_IoLog_GetLogCnt(ushort no, ushort* date);
extern	void	IoLog_edit( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
extern	ushort	LogPtrGet2(ushort no, ulong *inf);															// ���O�f�[�^�|�C���^�擾�����Q�i���o�ɏ��v�����g�p�j
extern	void	MONJOU_edit				( Mon_log *Monlog, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// ��������ް��ҏW����
extern	void	MONLOG_edit				( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// �����񗚗����ް��ҏW����
extern	void	MONJOU_edit_data		( Mon_log *Monlog, uchar pri_kind );											// ������i�P�ް��j�ҏW����
extern	void	RMONJOU_edit			( Rmon_log *Rmonlog, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// ���u�Ď������ް��ҏW����
extern	void	RMONLOG_edit			( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// ���u�Ď���񗚗����ް��ҏW����
extern	void	RMONJOU_edit_data		( Rmon_log *Rmonlog, uchar pri_kind );											// ���u�Ď����i�P�ް��j�ҏW����

extern	void	PrnNG_LOG		( MSG *msg );				// ���s�����O�󎚗v����				ү���ގ�M����
extern	void	PrnIO_LOG		( MSG *msg );				// �����o�Ƀ��O�󎚗v����			ү���ގ�M����

extern	void	SETTEI_edit				( ushort Kikai_no, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// �ݒ��ް����ް��ҏW
extern	uchar	SETTEI_edit_hyoudai		( ushort Kikai_no, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// �ݒ��ް��󎚁i�\��j�ҏW
extern	uchar	SETTEI_edit_sver		( uchar pri_kind, PRN_PROC_DATA *p_proc_data );									// �ݒ��ް��󎚁i����ް�ޮ݁j�ҏW
extern	uchar	SETTEI_edit_macaddress	( uchar pri_kind, PRN_PROC_DATA *p_proc_data );
extern	uchar	SETTEI_edit_cpara		( uchar pri_kind, PRN_PROC_DATA *p_proc_data );									// �ݒ��ް��󎚁i�������Ұ��j�ҏW
extern	void	SETTEI_edit_data		( ushort addr, ulong data, uchar pos, uchar	kind );								// �ݒ��ް��̈��ް��ҏW�����i���ڽ�ԍ����ް��j

extern	void	DOUSAC_edit				( ushort Kikai_no, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// ���춳�Ĉ��ް��ҏW

extern	void	LOCKDCNT_edit			( ushort Kikai_no, uchar Req_syu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ۯ����u���춳�Ĉ��ް��ҏW
extern	void	LOCKPARA_edit			( ushort Kikai_no, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// �Ԏ����Ұ����ް��ҏW
extern	void	LOCKSETTEI_edit			( ushort Kikai_no, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// ۯ����u�ݒ���ް��ҏW

extern	void	PKJOUNOW_edit			( ushort Kikai_no, uchar pri_kind, uchar prn_menu, PRN_PROC_DATA *p_proc_data );				// �Ԏ����i���݁j���ް��ҏW
extern	void	PKJOUSAV_edit			( ushort Kikai_no, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// �Ԏ����i�ޔ��j���ް��ҏW
extern	void	PKJOU_edit_proc			( uchar Edit_no, ushort Kikai_no, uchar pri_kind, uchar prn_menu, PRN_PROC_DATA *p_proc_data );	// �Ԏ������ް��ҏW

extern	void	TEIKID1_edit			( 																				// ����L���^�������ް��ҏW
											ushort			Kikai_no,
											ushort			Kakari_no,
											uchar			Pkno_syu,
											uchar			Req_syu,
											uchar			pri_kind,
											PRN_PROC_DATA	*p_proc_data
										);

extern	void	TEIKID2_edit			( 																				// ������Ɂ^�o�Ɉ��ް��ҏW
											ushort			Kikai_no,
											ushort			Kakari_no,
											uchar			Pkno_syu,
											uchar			Req_syu,
											uchar			pri_kind,
											PRN_PROC_DATA	*p_proc_data
										);

extern	uchar	TEIKID_edit_hyoudai		( uchar Pri_syu, ushort Kikai_no, ushort Kakari_no, uchar Pkno_syu, uchar pri_kind );	// ����󎚕\���ް��ҏW����
extern	uchar	TEIKID_edit_sts			( uchar Edit_no, uchar Pkno_syu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ����ð�����ް��ҏW����
extern	ushort	TEIKID_edit_idcnt		( uchar Pkno_syu );																// ������ް�ID����������
extern	void	TEIKID_edit_id			( ushort id, uchar pos );														// ���ID���ް��ҏW����

extern	void	SVSTIKK_edit			( ushort Kikai_no, ushort Kakari_no, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ���޽���������ް��ҏW

extern	void	SPLDAY_edit				( ushort Kikai_no, ushort Kakari_no, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// ���ʓ����ް��ҏW
extern	void	SPLDAY_edit_Kikan		( uchar pri_kind, uchar check );																// ���ʓ��i���ԁj���ް��ҏW
extern	void	SPLDAY_edit_Day			( uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar check);									// ���ʓ��i�����j���ް��ҏW
extern	void	SPLDAY_edit_Hmon		( uchar pri_kind, uchar check );																// ���ʓ��iʯ�߰���ް�j���ް��ҏW
extern	void	SPLDAY_edit_Yobi		( uchar pri_kind );																// ���ʓ��i�j���j���ް��ҏW
extern	void	SPLDAY_edit_SPYEAR		( uchar pri_kind, uchar check );																// ���ʓ��i���ʔN�����j���ް��ҏW
extern	void	SPLDAY_edit_SPWEEK		( uchar pri_kind, uchar check );																// ���ʓ��i���ʗj���j���ް��ҏW

extern	void	TRJOU_edit				(																				// �◯�ԏ����ް��ҏW
											ushort			Kikai_no,
											ushort			Kakari_no,
											short			Day,
											short			Cnt,
											short			*Data,
											uchar			pri_kind,
											PRN_PROC_DATA	*p_proc_data
										);

extern	void	TFJOU_edit				( Pon_log *Ponlog, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// �╜�d�����ް��ҏW
extern	uchar	TFJOU_edit_TimeTtl		( uchar pri_kind );																// �╜�d���F�������و��ް��ҏW����
extern	uchar	TFJOU_edit_TimeData		( Pon_log *Ponlog, uchar pri_kind );											// �╜�d���F��d�^���d�����ް��ҏW����
extern	uchar	TFJOU_edit_ClrData		( uchar pri_kind );																// �╜�d���F�ر����ް��ҏW����

extern	void	TFLOG_edit				( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// �╜�d�������ް��ҏW

extern	void	SEISANLOG_edit			( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// �ʐ��Z�����ް��ҏW
extern	void	FUSKYOJOU_edit			( flp_log *fuskyo, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// �s���E�����o�ɏ����ް��ҏW
extern	void	FUSKYOLOG_edit			( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// �s���E�����o�ɗ������ް��ҏW
extern	void	FUSKYOLOG_cnt_get		( T_FrmLogPriReq2 *msg_data, ushort *FusCnt, ushort *KyoCnt, PRN_PROC_DATA *p_proc_data );	// �s���E�����o���ް���������
extern	void	FUSKYO_edit_data		( flp_log *FusKyo, uchar pri_kind );											// �s���E�����o�Ɉ��ް��ҏW

extern	void	ANYDATA_edit			( ushort Kikai_no, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// �e���ް��i�[�������ް��ҏW

extern	void	IREKAETST_edit			( uchar pri_kind, PRN_PROC_DATA *p_proc_data );									// �p�����֎�ýĈ��ް��ҏW����

extern	void	PRITEST_edit			( uchar pri_kind, PRN_PROC_DATA *p_proc_data );									// �����ýĈ��ް��ҏW����
extern	void	PRITEST_edit_fontB		( uchar pri_kind );																// ̫�ĂaýĈ��ް��ҏW
extern	void	PRITEST_edit_fontA		( uchar pri_kind );																// ̫�Ă`ýĈ��ް��ҏW
extern	void	PRITEST_edit_Kanji		( uchar pri_kind );																// �����i�S�p�jýĈ��ް��ҏW

extern	void	LOGOREG_edit			( uchar pri_kind, PRN_PROC_DATA *p_proc_data );									// ���S�󎚃f�[�^�o�^����
extern	void	LOGOREG_edit_sub1		( char *dptr1, char *dptr2 );													// ۺވ��ް��ޯ�ϯ���ް��ϊ�����

extern	void	Suica_Log_edit			( T_FrmLogPriReq1 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// Suica�ʐM۸ޕҏW����
extern	void	Suica_Log_edit2			( T_FrmLogPriReq1 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// Suica�ʐM۸ޒ��߃f�[�^�p���ް��ҏW
extern	uchar	PrnSuicaLOG_datachk( T_FrmLogPriReq1 *msg_data );														// Suica��۸��ް�����
extern	ushort	Suica_edit_data( char *logdata, uchar pri_kind ,ushort data_size );										// Suica�ʐM۸��ް��ҏW����
extern	ushort	Suica_edit_data2( char *logdata, uchar pri_kind ,PRN_PROC_DATA *p_proc_data, ushort *wpt, struct suica_log_rec *wlogrec );	// Suica�ʐM۸��ް��ҏW����
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#ifdef	FUNCTION_MASK_EDY
//extern	void	Edy_Status_edit			( T_FrmLogPriReq1 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// Edy�ݒ聕�X�e�[�^�X�ҏW����
//extern	void	Edy_Arm_edit			( edy_arm_log *pribuf, uchar pri_kind, PRN_PROC_DATA *p_proc_data );			// Edy�װ�ڼ�ĕҏW����
//extern	void	Edy_Shime_edit			( edy_shime_log *pribuf, uchar pri_kind, PRN_PROC_DATA *p_proc_data );			// Edy���ߋL�^ڼ�ĕҏW����
//extern	void	EDYARMPAYLOG_edit( T_FrmLogPriReq2 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );				// Edy�A���[����������ް��ҏW����
//extern	uchar	Edy_ArnPayLog_edit( edy_arm_log *pribuf, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
//extern	void	EDYSHIMELOG_edit( T_FrmLogPriReq2 *msg_data, 	uchar pri_kind, PRN_PROC_DATA *p_proc_data );
//extern	uchar	Edy_ShimeLog_edit( edy_shime_log *pribuf, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
//#endif
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
// MH321800(S) G.So IC�N���W�b�g�Ή�
extern	void	Ec_AlmRct_edit		(T_FrmEcAlmRctReq *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data);	// ���σ��[�_�A���[�����V�[�g�ް��ҏW����
extern	uchar	Ec_AlmRct_edit_sub	(EC_SETTLEMENT_RES *almrct, uchar pay_brand, uchar pri_kind, PRN_PROC_DATA *p_proc_data);	// ���σ��[�_�A���[�����V�[�g�ް��ҏW�����T�u
extern	void	Ec_Brand_edit		(T_FrmEcBrandReq *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data);	// ���σ��[�_�u�����h����ް��ҏW����
extern	void	Ec_AlmRctLog_edit(T_FrmEcAlarmLog *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data);			// ��������������O���ް��ҏW����
// MH810105 GG119202(S) T���v�A���󎚑Ή�
//extern	void	Ec_AlmRctLog_edit_sub(uchar pri_kind, PRN_PROC_DATA *p_proc_data);										// ��������������O���ް��ҏW����
extern	void	Ec_AlmRctLog_edit_sub(uchar pri_kind, ulong StartTime, ulong EndTime, PRN_PROC_DATA *p_proc_data);	// ��������������O���ް��ҏW����
// MH810105 GG119202(E) T���v�A���󎚑Ή�
// MH810103 GG119202(S) IC�N���W�b�g�݂Ȃ����ψ󎚃t�H�[�}�b�g�ύX�Ή�
//extern	void	Ec_DeemedJnl_edit(T_FrmEcDeemedJnlReq *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data);			// �݂Ȃ����ϕ��d���O���ް��ҏW����
// MH810103 GG119202(E) IC�N���W�b�g�݂Ȃ����ψ󎚃t�H�[�}�b�g�ύX�Ή�
extern	uchar	Recv_DeemedData_edit(Receipt_data *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data);				// ���ϐ��Z���~(������)�ް��ҏW����
// MH321800(E) G.So IC�N���W�b�g�Ή�
// MH810105 GG119202(S) T���v�A���󎚑Ή�
extern	void	SYUUKEI_EC_edit(SYUKEI *Syuukei, uchar pri_kind, ushort print_flag, T_FrmTgoukeiEc *Ec_data, PRN_PROC_DATA *p_proc_data);	// �W�v���ް��ҏW
// MH810105 GG119202(E) T���v�A���󎚑Ή�

extern	void	CreUseLog_edit			( T_FrmLogPriReq3 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// ���p���׃��O���ް��ҏW
extern	void	CreUseLog_edit_ccid		( char *dat, uchar pri_kind );													// ���p���׃��O�p�J�[�h�ԍ��ҏW
extern	void	CreUnSend_edit			( T_FrmUnSendPriReq *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �ڼޯĔ���˗��ް��ҏW����
extern	void	CreSaleNG_edit			( T_FrmSaleNGPriReq *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );	// �ڼޯĔ��㋑���ް��ҏW����

extern	uchar	Prn_edit_logo( uchar pri_kind );																// ۺވ��ް��ҏW
extern	uchar	Prn_edit_headhoot( uchar edit_kind, uchar pri_kind );											// ͯ�ް�^̯�����ް��ҏW
extern	uchar	Prn_edit_headhoot_new ( uchar start_num, uchar count, uchar edit_kind, uchar pri_kind );
extern	uchar	Prn_edit_sikiri( uchar pri_kind );																// �d�؂�s���ް��ҏW
extern	uchar	Prn_edit_space( uchar pri_kind );																// �X�y�[�X�s���ް��ҏW
extern	uchar	Prn_edit_TurikireMsg( uchar msg_no, uchar pri_kind );											// �ނ�؂ꎞ�̌Œ�ү���ޕҏW
extern	uchar	Prn_edit_KoteiMsg( uchar msg_no, uchar pri_kind );												// �Œ�ү���ޕҏW
extern	uchar	Prn_edit_Title( const char *prn_data, uchar font, uchar pri_kind );									// �\����ް��ҏW
extern	uchar	Prn_edit_KaiKin( const char *prn_data, ulong kaisuu, ulong kingaku, uchar pri_kind );					// �񐔁^���z���ް��ҏW

extern	uchar	Prn_edit_KenKin( const char *prn_data, ulong kensuu, ulong kingaku, uchar pri_kind );
extern	uchar	Prn_edit_KaiKin_sub( char ptn, const char *prn_data, ulong kaisuu, ulong kingaku, uchar pri_kind );

extern	uchar	Prn_edit_Sitei1line( const char *prn_data, ulong data, const char *tanni1, const char *tanni2, uchar pri_kind );	// �P�ʎw��P�s���ް��ҏW
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
extern	uchar	Prn_edit_Sitei1line_2( const char *prn_data, ulong data, const char *tanni1, const char *tanni2, uchar pri_kind );	// �P�ʎw��P�s���ް��ҏW(*�t�\��)
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
extern	uchar	Prn_edit_SkDate( date_time_rec	*NowTime, date_time_rec	*OldTime, uchar pri_kind );				// �W�v�����i����^�O��j���ް��ҏW
extern	void	Prn_edit_NowDate( char *edit_wk );																// ���ݓ������ް��ҏW
extern	void	Prn_edit_TanKin( char *edit_wk, ulong kingaku );												// �P�ʋ��z���ް��ҏW
extern	void	Prn_edit_TanMai( char *edit_wk, ulong maisuu );													// �P�ʖ������ް��ҏW
extern	uchar	Prn_edit_pkposi( char *edit_wk, ulong pk_posi, uchar space );									// ���Ԉʒu�i���^�ԍ��j���ް��ҏW
extern	uchar	Prn_edit_Date( char *edit_wk, date_time_rec *date );											// �������ް��ҏW	�F"yyyy�Nmm��dd��(��) hh:mm"
extern	uchar	Prn_edit_Date2( char *edit_wk, date_time_rec *date );											// �������ް��ҏW	�F"yy.mm.dd hh:mm"
extern	uchar	Prn_edit_Date3( char *edit_wk, date_time_rec *date );											// �������ް��ҏW	�F"yyyy�Nmm��dd��(��) hh:mm"
extern	uchar	Prn_edit_Date4( char *edit_wk, date_time_rec *date );											// �������ް��ҏW	�F"yyyy�Nmm��dd��(��)"
extern	uchar	Prn_edit_RePriDate( uchar pri_kind );															// ������ē������ް��ҏW
extern	uchar	Prn_edit_TstPriDate( uchar pri_kind );															// ý�����ē������ް��ҏW
extern	uchar	Prn_edit_SyomeiKamei( uchar edit_kind, uchar pri_kind );										// �x�����ށE�������^�����X�����ް��ҏW
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
extern	uchar	Prn_edit_Ec_Syomei( Receipt_data *Ryousyuu, uchar pri_kind );									// �x�����ށE���������ް��ҏW
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
extern	uchar	Prn_edit_KakariNo( ushort Kakari_no, uchar pri_kind );											// �W���m���D���ް��ҏW

extern	void	KakariNoSet( char *data, ushort no );															// �W�������ް��ҏW

extern	void	KikaiNoSet( char *data, ushort no );															// �@�B�����ް��ҏW

extern	void	SeisanKindSet( char *data, uchar kind );														// ���Z�������ް��ҏW
extern	void	OibanSet( char *data, ulong no, uchar name_no );												// �ǔԈ��ް��ҏW
// GG129000(S) H.Fujinaga 2023/01/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�������󎚑Ή��j
extern	void	HakkenKiNoSet( char *data, uchar no );															// �����@��.���ް��ҏW
extern	void	HakkenNoSet( char *data, ulong no );															// ������.	���ް��ҏW
// GG129000(E) H.Fujinaga 2023/01/04 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�������󎚑Ή��j
extern	void	Camma_Set( uchar *ucTmp );																		// �R����؂菈��
extern	void	Str_Set1( const char *cFirst, const char *cSecond, uchar pri_kind );										// ��s���̈󎚕�����ݒ�(��)�����P
extern	void	Str_Set2( const char *cFirst, const char *cSecond, const char *cThird, const char *cFourth, uchar pri_kind );			// ��s���̈󎚕�����ݒ�(��)�����Q
extern	ushort	Moj_Point( const char *data );																		// ���������Z����
extern	ushort	Spc_Add( char *data, ushort cnt );																// ��߰��}������
extern	void	StrAdd( uchar *ucTmpA, uchar *ucTmpB );															// ������̒ǉ�����
extern	void	Prn_LogoHead_PreHead ( void );
extern	void	Prn_LogoHead_PostHead ( void );
extern	void	Prn_LogoHead_Init ( void );
extern	uchar	Prn_LogHead_PostHead_Offs;			/* ͯ�ް����ŏo���ꍇ�̊J�n�ݒ�I�t�Z�b�g == 0:�󎚂Ȃ� */

extern	void	ArcModVer_edit( T_FrmLogPriReq1 *msg_data, 	uchar pri_kind, PRN_PROC_DATA *p_proc_data );
extern	uchar	SearchTypeData_edit		( T_FrmLogPriReq2 *data, PRN_PROC_DATA *p_proc_data, const uchar *str );				// �������@�f�[�^�ҏW(��������)
extern	uchar	SearchTypeData_edit2	( T_FrmLogPriReq2 *data, PRN_PROC_DATA *p_proc_data, const uchar *str );				// �������@�f�[�^�ҏW(�����Ȃ�)
extern	void	PrnAtendValidData( MSG *msg );																	// ���W���L���f�[�^�󎚗v����				ү���ގ�M����
extern	void	AtendValidData_edit( ushort Kikai_no, ushort Kakari_no, uchar	pri_kind, PRN_PROC_DATA	*p_proc_data );// �W���L�����ް��ҏW
extern  void	AtendValid_edit_id( ushort id, uchar pos );														// �W���L���ް��󎚈ʒu�ҏW
extern	uchar	AtendValid_edit_hyoudai( ushort Kikai_no, ushort Kakari_no, uchar pri_kind );					// �\��i�W���L���ް��j
extern  uchar	AtendValid_edit_sts( uchar pri_kind, PRN_PROC_DATA *p_proc_data );								// �W���L���ް��󎚏���
extern	void	ChargeSetup_edit( uchar pri_kind, PRN_PROC_DATA *p_proc_data, T_FrmChargeSetup *msg);			// �����ݒ�v�����g���ް��ҏW
extern	void	PrnSetDiffLog_edit(T_FrmLogPriReq1 *data, PRN_PROC_DATA *p_proc_data, uchar pri_kind);			// �ݒ�X�V�������ް��ҏW
extern	uchar	Prn_edit_T_foot_edit ( uchar edit_kind, uchar pri_kind );
extern	uchar	Prn_edit_AzuFtr_edit ( uchar pri_kind );
// �d�l�ύX(S) K.Onodera 2016/10/31 #1531 �̎��؃t�b�^�[���󎚂���
extern	uchar	Prn_edit_KbrFtr_edit ( uchar pri_kind, ushort kind );
// �d�l�ύX(E) K.Onodera 2016/10/31 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
extern	uchar	Prn_edit_FutureFtr_edit ( uchar pri_kind );
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
extern	void	PrnRrokin_Setlog_edit( T_FrmLogPriReq2 *msg_data, 	uchar pri_kind, PRN_PROC_DATA *p_proc_data);
extern	void	PrnSettei_Chk_edit(T_FrmAnyData *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data);
extern	void	PrnFlapLoopData_edit(T_FrmAnyData *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data);
extern	void	Mnt_Stack_edit( T_FrmLogPriReq1 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
extern	void	ChkResult_edit( T_FrmChk_result *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
extern	void	RT_SEISANLOG_edit			( T_FrmLogPriReq1 *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data );		// �ʐ��Z�����ް��ҏW
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
extern	uchar	Recv_FailureContactData_edit(Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data);			// ��Q�A���[�ް��ҏW����
extern	uchar	Prn_edit_EmgFooter_edit( uchar pri_kind );
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// MH810105(S) MH364301 �C���{�C�X�Ή�
extern	uchar	Disp_Tax_Value( date_time_rec *time );
extern	short	date_exist_check(short y, short m, short d);
// MH810105(E) MH364301 �C���{�C�X�Ή�
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
extern	uchar	RYOUSYUU_edit_taxableDiscount( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data, uchar pri_no );
extern	uchar	RYOUSYUU_edit_totalAmount_taxRate( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data );
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή��j
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j
extern	void	PrnGetRegistNum(void);
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�W���[�i���ڑ��Ȃ��Ή��j

/*-------	Pri_Cmn.c	-------*/
extern	void	PrnInit( void );										// ���������������
extern	void	PrnInit_R( void );										// ����������������iڼ�āj
extern	void	PrnInit_J( void );										// ����������������i�ެ��فj

extern	void	PrnMode_idle( uchar pri_kind );							// �������Ӱ�ޏ�����Ԑݒ�
extern	void	PrnMode_idle_R( void );									// �������Ӱ�ޏ�����Ԑݒ�iڼ�āj
extern	void	PrnMode_idle_J( void );									// �������Ӱ�ޏ�����Ԑݒ�i�ެ��فj

extern	void	PrnFontSize( uchar font, uchar pri_kind );				// ������󎚕������ސݒ�

extern	void	prn_proc_data_clr( uchar pri_kind );					// �ҏW���������ް�������
extern	void	prn_proc_data_clr_R( void );							// �ҏW���������ް��������iڼ�āj
extern	void	prn_proc_data_clr_J( void );							// �ҏW���������ް��������i�ެ��فj

extern	void	prn_edit_wk_clr( void );								// �ҏW��������ܰ��ر������

extern	uchar	Inji_Cancel_chk( uchar pri_kind );						// �󎚒��~ү���ގ�M����

extern	void	Inji_Cancel( MSG *msg, uchar pri_kind );				// �󎚒��~����
extern	void	Inji_Cancel_R( MSG *msg );								// �󎚒��~�����iڼ�āj
extern	void	Inji_Cancel_J( MSG *msg );								// �󎚒��~�����i�ެ��فj

extern	void	Inji_ErrEnd( ushort command, uchar pri_sts, uchar pri_kind );	// �󎚈ُ�I������
extern	void	Inji_ErrEnd_R( ushort command, uchar pri_sts );			// �󎚈ُ�I�������iڼ�āj
extern	void	Inji_ErrEnd_J( ushort command, uchar pri_sts );			// �󎚈ُ�I�������i�ެ��فj

extern	void	End_Set( MSG *msg, uchar pri_kind );					// ���ް��P��ۯ��ҏW�I������
extern	void	End_Set_R( MSG *msg );									// ���ް��P��ۯ��ҏW�I�������iڼ�āj
extern	void	End_Set_J( MSG *msg );									// ���ް��P��ۯ��ҏW�I�������i�ެ��فj

extern	void	PrnEndMsg( 	ushort	comm,								// �󎚏I��ү���ޑ��M����
							uchar	result,
							uchar	stat,
							uchar	pri_kind );
extern	void	PrnEndMsg_R(ushort	comm,								// �󎚏I��ү���ޑ��M�����iڼ�āj
							uchar	result,
							uchar	stat );
extern	void	PrnEndMsg_J(ushort	comm,								// �󎚏I��ү���ޑ��M�����i�ެ��فj
							uchar	result,
							uchar	stat );

extern	void	PrnNext( MSG *Msg, uchar pri_kind );					// ���ް�����ۯ��ҏW�v��ү���ޑ��M����
extern	void	PrnNext_R( MSG *Msg );									// ���ް�����ۯ��ҏW�v��ү���ޑ��M�����iڼ�āj
extern	void	PrnNext_J( MSG *Msg );									// ���ް�����ۯ��ҏW�v��ү���ޑ��M�����i�ެ��فj

extern	void	MsgSndFrmPrn(	ushort cmd,								// ���������ւ�ү���ޑ��M
								uchar data1,
								uchar data2 );

extern	uchar	PrnJnlCheck( void );									// �ެ���������ڑ��L������
extern	uchar	PrnGoukeiChk( ushort command );							// ���v�L�^�󎚗v���̔���
extern	uchar	PrnGoukeiPri( ushort cmd );								// ���v�L�^�̈󎚐����������
extern	uchar	PrnAsyuukeiPri( void );									// �����W�v�̈󎚐����������
extern	uchar	PriStsCheck( uchar pri_kind );							// �������ԁi�󎚉^�s�j����
extern	uchar	PriOutCheck( uchar pri_kind );							// ��������M�ް��ޯ̧�������

extern	void	PrnCmdLen(	const uchar *dat,									// ������ւ̺���ޕҏW����
							ushort len,
							uchar pri_kind );
extern	void	PrnCmdLen_R( const uchar *dat , ushort len );			// ������ւ̺���ޕҏW�����iڼ�āj
extern	void	PrnCmdLen_J( const uchar *dat , ushort len );			// ������ւ̺���ޕҏW�����i�ެ��فj

extern	void	PrnCmd( char *dat, uchar pri_kind );					// ������ւ̕������ް��ҏW����
extern	void	PrnCmd_R( char *dat );									// ������ւ̕������ް��ҏW�����iڼ�āj
extern	void	PrnCmd_J( char *dat );									// ������ւ̕������ް��ҏW�����i�ެ��فj

extern	void	PrnStr( const char *dat, uchar pri_kind );					// ������ւ̂P�s���������ް��ҏW����
extern	void	PrnStr_R( const char *dat );							// ������ւ̂P�s���������ް��ҏW�����iڼ�āj
extern	void	PrnStr_J( const char *dat );							// ������ւ̂P�s���������ް��ҏW�����i�ެ��فj
extern	void	PrnStr_SPDAY( char *dat, uchar pos );

extern	void	PrnOut( uchar pri_kind );								// ������N������
extern	void	PrnOut_R( void );										// ������N�������iڼ�āj
extern	void	PrnOut_J( void );										// ������N�������i�ެ��فj

extern	void	PrnStop( uchar pri_kind );								// ������󎚒�~
extern	void	PrnStop_R( void );										// ������󎚒�~�iڼ�āj
extern	void	PrnStop_J( void );										// ������󎚒�~�i�ެ��فj

extern	void	Rct_top_edit( uchar cut );								// ڼ�Đ擪���ް��ҏW�����iۺވ󎚁^ͯ�ް�󎚁^�p����Đ���j
extern	void	Kaigyou_set(	uchar pri_kind,							// ���s�ް��ҏW�����i���s�ʁ��ݒ��ް��j
								uchar font,
								uchar cnt );
extern	void	Kaigyou_set2(	uchar pri_kind,							// ���s�ް��ҏW�����Q�i���s�ʁ����Ұ��w��j
								uchar font,
								uchar cnt,
								uchar data );
extern	void	Feed_set( uchar pri_kind, uchar size );					// �������ް��ҏW����

extern	uchar	DateChk( date_time_rec *date );							// ���t�ް���������
extern	uchar	Prn_data_chk( uchar req, ulong data );					// ���ް���������
extern	uchar	Prn_log_check(	ushort cnt,								// �e��۸ޏ��i�����A�߲���j��������
								ushort wp,
								ushort max,
								uchar log_no );
extern	ushort	Prn_log_search1( ushort cnt, ushort wp, ushort max );	// �e��۸ޏ��̍Ō��ް���������
extern	ushort	Prn_log_search2( ushort no, ushort max );				// �e��۸ޏ����ް��߲���X�V����
extern	void	Prn_pkposi_chg( ulong no, ushort *area, ushort *pkno );	// ���Ԉʒu�ް��ϊ�����
extern	ulong	Prn_pkposi_make( ushort lk_no );						// ���Ԉʒu�ް��쐬����
extern	void	Prn_errlg( uchar err_no, uchar knd, uchar data );		// ��������䕔�װ���o�^����
extern uchar	Prn_edit_sya_syu( char *edit_wk, ushort lk_no ,uchar flg ,uchar M_Flg);		//

extern	uchar Printer_Lifter_Wait( char event );

extern	uchar Prn_JRprintSelect( uchar set_wk, uchar p_kind );
extern	void	RP_I2CSndReq( int type );
extern	void	JP_I2CSndReq( int type );
extern	void	Back_Feed_set( uchar , uchar );
// MH322917(S) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)
extern	uchar	PriJrnExeCheck( void );
// MH322917(E) A.Iiizumi 2018/09/03 �������Ԍ��o�@�\�̊g���Ή�(���o����)

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
extern	void	PrnCmd_InfoReq(uchar kind);
extern	void	PrnCmd_InfoResTimeout(void);
extern	uchar	PrnCmd_InfoRes_Proc(uchar *pBuf, ushort RcvSize);
extern	void	PrnCmd_InfoRes_Msg(void);
// MH810104 GG119201(S) �d�q�W���[�i���Ή��i�����ݒ��Ɏ��v�ݒ肵�Ȃ��j
extern	void	PrnCmd_Clock(void);
// MH810104 GG119201(E) �d�q�W���[�i���Ή��i�����ݒ��Ɏ��v�ݒ肵�Ȃ��j
extern	void	PrnCmd_InitSetting(void);
extern	void	PrnCmd_ResetTimeout(void);
extern	void	PrnCmd_WriteStartEnd(uchar kind, MSG *msg);
extern	uchar	PrnCmd_CheckSendData(uchar *pBuf);
extern	void	PrnCmd_Sts_Proc(void);
extern	void	PrnCmd_EJAErrRegist(void);
extern	uchar	PrnCmd_ReWriteCheck(void);
extern	void	PrnCmd_ReWriteStart(uchar kind);
extern	void	PrnCmd_EJAReset(uchar kind);
extern	void	PrnCmd_MsgResend(MSG *msg);
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// MH810105(S) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j
extern	ulong	PrnDat_WriteQue(ushort command, uchar *buf, ushort size);
extern	void	PrnDat_ReadQue(MSG *msg);
extern	void	PrnDat_DelQue(ushort command, ulong job_id);
extern	ushort	PrnDat_GetQueCount(void);
extern	ulong	PrnDat_GetJobID(void);
extern	void	PrnMsgBoxClear(void);
// MH810105(E) MH364301 �d�q�W���[�i���Ή��i�W���[�i���󎚃f�[�^�̒�d�ۏ�΍�j

// MH810105(S) R.Endo 2021/12/01 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
extern	void	PrnDEBUG_edit(T_FrmDebugData *msg_data, uchar pri_kind, PRN_PROC_DATA *p_proc_data);
// MH810105(E) R.Endo 2021/12/01 �Ԕԃ`�P�b�g���X3.0 #6184 �y����w�E�z���Z���E2610-224�G���[�p��
// GG129000(S) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
extern	void	QrCode_set( uchar pri_kind, uchar size, char *string, ushort length  );		// QR�R�[�h�ҏW����
extern	uchar	Prn_edit_qrcode( Receipt_data *Ryousyuu );									// QR�R�[�h�ҏW
extern	uchar	RYOUSYUU_edit_PAYQR( Receipt_data *Ryousyuu, uchar pri_kind, PRN_PROC_DATA *p_proc_data);
// GG129000(E) H.Fujinaga 2022/12/05 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��iQR���Ԍ��Ή��j
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
extern	void	Prn_GetModel_R(void);
extern	void	PrnCmd_InfoReq_R(uchar kind);
extern	void	PrnCmd_InfoResTimeout_R(void);
extern	uchar	PrnCmd_InfoRes_Proc_R(uchar *pBuf, ushort RcvSize);
extern	void	PrnCmd_InfoRes_Msg_R(void);
extern	void	Prn_Model_Chk_R(void);
extern	void	PrnRYOUSYUU_StockPrint(void);
extern	void	PrnRYOUSYUU_StockClear(void);
extern	void	PrnCmd_PLayout_Design(uchar pri_kind, uchar dt);
extern	void	Prn_NewPrint_Clear_R(void);
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j

/*-------	Pri_Data.c	-------*/
extern	PRN_DATA_BUFF	rct_prn_buff;			// ڼ��(SCI0)	����M�ް��Ǘ��ޯ̧
extern	PRN_DATA_BUFF	jnl_prn_buff;			// �ެ���(SCI3)	����M�ް��Ǘ��ޯ̧

// MH810104 GG119201(S) �d�q�W���[�i���Ή�
extern	PRN_EJA_DATA_BUFF	eja_prn_buff;		// �d�q�W���[�i����M�f�[�^�Ǘ�
#define	isEJA_USE()			(eja_prn_buff.EjaUseFlg > 0)		// �d�q�W���[�i���ڑ�����
#define	isPrnInfoReq()		(eja_prn_buff.PrnInfoReq != 0)		// �v�����^���v����
#define	SetPrnInfoReq(x)	(eja_prn_buff.PrnInfoReq = (x))		// �v�����^���v���Z�b�g
#define	isPrnInfoInitRecv()	(eja_prn_buff.PrnRcvBufReadCnt == 0)		// �v�����^���v���̏����M
#define	isPrnInfoRes(x)		(((x) == 0xFF) || (((x) & 0xC0) == 0xC0))	// �v�����^��񉞓�
																		// 1�o�C�g��=0xFF�i�d�q�W���[�i���j
																		// 1�o�C�g�ڂ�bit 7,6��1�i�W���[�i���v�����^�j
#define	isPrnRetryOver()	(eja_prn_buff.PrnHResetRetryCnt > 0)		// �f�[�^�����݃��g���C�I�[�o�[
extern	uchar			eja_work_buff[PRN_EJA_BUF_SIZE];		// �d�q�W���[�i���p���[�N�o�b�t�@
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
extern	PRN_RCV_DATA_BUFF_R	pri_rcv_buff_r;
extern	uchar			ryo_stock;				// ���V�[�g�f�[�^�X�g�b�N�t���O 0:�X�g�b�N�Ȃ� 1:�X�g�b�N���� 2:�X�g�b�N�j�� 0xff:�X�g�b�N�󎚊���
#define	isNewModel_R()		(pri_rcv_buff_r.NewModelFlg == 1)			// ���V�[�g�v�����^�V���f���ڑ���
#define	SetModelMiss_R(x)	(pri_rcv_buff_r.ModelMissFlg = (x))			// ���V�[�g�v�����^���f���ݒ�s��v�G���[�t���O�Z�b�g
#define	isModelMiss_R()		(pri_rcv_buff_r.ModelMissFlg == 1)			// ���V�[�g�v�����^���f���ݒ�s��v�G���[
#define	SetPrnInfoReq_R(x)	(pri_rcv_buff_r.PrnInfoReq = (x))			// ���V�[�g�v�����^���v���Z�b�g
#define	isPrnInfoReq_R()	(pri_rcv_buff_r.PrnInfoReq != 0)			// ���V�[�g�v�����^���v����
#define	isPrnInfoRes_R(x)	((x) == 0xFF)								// �v�����^��񉞓� 1�o�C�g��=0xFF�i�d�q�W���[�i���j
#define	isPrnInfoInitRecv_R()(pri_rcv_buff_r.PrnRcvBufReadCnt == 0)		// �v�����^���v���̏����M
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j

extern	PRN_PROC_DATA	rct_proc_data;			// ڼ��		�󎚏��������ް�
extern	PRN_PROC_DATA	jnl_proc_data;			// �ެ���	�󎚏��������ް�

extern	uchar			rct_init_sts;			// ڼ��		���������(�O�F�����������҂��^�P�F�����������^�Q�F���������s�^�R�F���ڑ�)
extern	uchar			jnl_init_sts;			// �ެ���	���������(�O�F�����������҂��^�P�F�����������^�Q�F���������s�^�R�F���ڑ�)
extern	uchar			rct_init_rty;			// ڼ��		����������ޑ��M��ײ�����
extern	uchar			jnl_init_rty;			// �ެ���	����������ޑ��M��ײ�����

extern	uchar			GyouCnt_r;				// ڼ��		���M�ς݈��ް��s��
extern	uchar			GyouCnt_j;				// �ެ���	���M�ς݈��ް��s��

// GG129000(S) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
extern	uchar			GyouCnt_All_r;			// ڼ��		���M�ς݈��ް��S�s��
// GG129000(E) �x�m�}�n�C�����h�@�\���P�i�̎��ؔ��sPOPUP�����V�[�g�󎚊����ō폜�j�iGM803003���p�j
// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
extern	uchar			GyouCnt_All_j;			// �ެ���	���M�ς݈��ް��S�s��
// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j

extern	MSG 			NextMsg_r;				// ڼ��		�󎚗v��ү���ޕۑ��ޯ̧
extern	MSG 			NextMsg_j;				// �ެ���	�󎚗v��ү���ޕۑ��ޯ̧

extern	uchar			PriDummyMsg;			// ��аү���ޓo�^��ԁiON�F�o�^����^OFF�F�o�^�Ȃ��j
extern	MsgBuf			*TgtMsg;				// �����Ώ�ү�����ޯ̧�߲��

extern	uchar			terget_pri;				// �����Ώ���������
extern	uchar			next_prn_msg;			// �󎚏������̎��ҏW�v��ү�����׸�	�iON�F���ҏW�v���^OFF�F�V�K�ҏW�v���j
extern	uchar			syuukei_kind;			// �󎚏������̏W�v���

extern	uchar			rct_goukei_pri;			// ���v�L�^�󎚏�ԁiڼ�ėp�j	��ON�F�󎚏�����
extern	uchar			jnl_goukei_pri;			// ���v�L�^�󎚏�ԁi�ެ��ٗp�j	��ON�F�󎚏�����
extern	uchar			rct_atsyuk_pri;			// �����W�v�󎚏�ԁiڼ�ėp�j	��ON�F�󎚏�����
extern	uchar			jnl_atsyuk_pri;			// �����W�v�󎚏�ԁi�ެ��ٗp�j	��ON�F�󎚏�����
extern	uchar			turikan_pri_status;		// �ޑK�Ǘ�����Ă��󎚏I����Ă���

extern	uchar			prn_zero;				// �W�v�ް��O���󎚗L���i�ݒ��ް��j
extern	date_time_rec	Repri_Time;				// ������ē����i�O��W�v�󎚗p�j
extern	uchar			Repri_kakari_no;		// ������ČW��No.

extern	uchar			header_hight;			// ͯ�ް���ް��̏c���ށi�ޯĐ��j

extern	uchar			gyoukan_data;			// ���s���i�ޯĐ��j�w��i���O:�ݒ��ް��ɏ]���^���O:�{�w���ް��ɏ]���j

extern	char			prn_work[64];			// �󎚏���ܰ�
extern	char			cTmp1[256];				// �󎚏���ܰ�
extern	char			cTmp2[64];				// �󎚏���ܰ�
extern	char			cMnydata[64];			// �󎚏���ܰ�

extern	char			cEditwk1[64];			// ���ް��ҏWܰ��P
extern	char			cEditwk2[64];			// ���ް��ҏWܰ��Q
extern	char			cEditwk3[64];			// ���ް��ҏWܰ��R

extern	char			cErrlog[160];			// �װ۸ޗp

extern	ulong			Header_Rsts;			// ͯ�ް���ް��Ǎ����
extern	ulong			Footer_Rsts;			// ̯�����ް��Ǎ����
extern	char			Header_Data[4][36];		// ͯ�ް���ް�
extern	char			Footer_Data[4][36];		// ̯�����ް�

extern	ulong			AcceptFooter_Rsts;			// ��t��̯�����ް��Ǎ����
extern	char			AcceptFooter_Data[4][36];	// ��t��̯�����ް�

extern	char			Logo_Fdata[24][56];		// ۺވ��ް��Ǎ�ܰ�
extern	char			Logo_Fla_data[24];		// ۺވ��ް��ϊ�ܰ�1�i�ׯ����؂��ް��j
extern	char			Logo_Reg_data[24];		// ۺވ��ް��ϊ�ܰ�2�i�ׯ����؂�ϊ������ް��j
extern	uchar			Logo_Reg_sts_rct;		// ۺޓo�^������ԁiڼ�āj
extern	uchar			Logo_Reg_sts_jnl;		// ۺޓo�^������ԁi�ެ��فj

extern	t_Pri_Work		Pri_Work[];				// �v�����^�^�X�N���[�N�G���A

extern	const char 		*Sya_Prn[];				//�Ԏ�

extern	ulong			Syomei_Rsts;			// �x�����ށE���������ް��Ǎ����
extern	ulong			Kamei_Rsts;				// �����X�����ް��Ǎ����
extern	char			Syomei_Data[4][36];		// �x�����ށE���������ް�
extern	char			Kamei_Data[4][36];		// �����X�����ް�

extern	uchar			JR_Print_Wait_tim[2];	// �ެ��فEڼ���������󎚊J�n�҂����
extern	uchar			f_Prn_R_SendTopChar;	// ڼ��������󎚗v���̐擪�s�������M�O�t���O
extern	uchar			f_Prn_J_SendTopChar;	// �ެ���������󎚗v���̐擪�s�������M�O�t���O
extern	ushort			Pri_Tasck[TSKMAX][2];		// �^�X�N�g�p�ʁA��
extern	ulong			Pri_Pay_Syu[2][2];			// ���Z�A�W�v�f�[�^�T�C�Y
extern	uchar			Pri_program;				// �f�o�b�ONo
extern	uchar 			Cal_Parameter_Flg;	// �ݒ�l�ُ�t���O 0:����/1:�ُ�
extern	uchar			f_partial_cut;				// �p�[�V�����J�b�g����t���O(�p�[�V�����J�b�g�����s���ꂽ���Ƃ������t���O 0:�����s 1:���s)
extern	uchar			rct_timer_end;			// ���V�[�g�v�����^�󎚊�����^�C�}�[
extern	uchar			PriBothPrint;			// �v�����^�{�W���[�i���󎚃t���O
extern	uchar			BothPrnStatus;			// �v�����^�{�W���[�i���󎚂̃G���[�X�e�[�^�X
extern	ushort			BothPrintCommand;		// �v�����^�{�W���[�i���󎚂̃G���[�R�}���h
// MH322914 (s) kasiyama 2016/07/13 �̎��؈󎚒��͐��Z�J�n�����Ȃ�[���ʃo�ONo.1275](MH341106)
extern	uchar			ryo_inji;				// ���V�[�g�v�����^�̎��؈󎚏�� 0=�󎚒��ł͂Ȃ�/1=�󎚒�
// MH322914 (e) kasiyama 2016/07/13 �̎��؈󎚒��͐��Z�J�n�����Ȃ�[���ʃo�ONo.1275](MH341106)
// MH810103 MHUT40XX(S) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��
extern	EC_SETTLEMENT_RES	Deemedalmrct;
// MH810103 MHUT40XX(E) DC=29����49���֕ύX���A���ό��ʂ̃X�e�[�^�X�ɉ����ăW���[�i���󎚂��s��

/*-------	memdata.c	-------*/
extern	Uketuke_data	UketukeData;			// ��t�����ް�
extern	Azukari_data	AzukariData;			// �a��؈��ް�

/*-------	int_ram.c	-------*/
extern uchar			PriErrCount;			// �رٴװ���荞���׸�
extern	ulong			TCardFtr_Rsts;			// T�J�[�h̯�����ް��Ǎ����
extern	char			TCardFtr_Data[T_FOOTER_GYO_MAX][36];	// T�J�[�h̯�����ް�

extern	ulong			AzuFtr_Rsts;			// T�J�[�h̯�����ް��Ǎ����
extern	char			AzuFtr_Data[AZU_FTR_GYO_MAX][36];	// T�J�[�h̯�����ް�
// �d�l�ύX(S) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
extern	ulong			CreKbrFtr_Rsts;							// �N���W�b�g�ߕ���̯�����ް��Ǎ����
extern	char			CreKbrFtr_Data[KBR_FTR_GYO_MAX][36];	// �N���W�b�g�ߕ���̯�����ް�
extern	ulong			EpayKbrFtr_Rsts;						// �d�q�}�l�[�ߕ���̯�����ް��Ǎ����
extern	char			EpayKbrFtr_Data[KBR_FTR_GYO_MAX][36];	// �d�q�}�l�[�ߕ���̯�����ް�
// �d�l�ύX(E) K.Onodera 2016/10/28 #1531 �̎��؃t�b�^�[���󎚂���
// �d�l�ύX(S) K.Onodera 2016/11/08 ����x���z�t�b�^�[
extern	ulong			FutureFtr_Rsts;							// ����x���z̯�����ް��Ǎ����
extern	char			FutureFtr_Data[FUTURE_FTR_GYO_MAX][36];	// ����x���z̯�����ް�
// �d�l�ύX(E) K.Onodera 2016/11/08 ����x���z�t�b�^�[
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
extern	ulong			EmgFooter_Rsts;							// ��Q�A���[̯�����ް��Ǎ����
extern	char			EmgFooter_Data[EMGFOOT_GYO_MAX][36];	// ��Q�A���[̯�����ް�
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j
extern	uchar			RegistNum[14];			// �C���{�C�X�̓o�^�ԍ��iT+13���̐����j
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�o�^�ԍ����Z�b�g����j

#endif	//_PRI_DEF_H_
