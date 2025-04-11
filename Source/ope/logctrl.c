/*[]----------------------------------------------------------------------[]*/
/*| LOG�֘A�֐�                                                            |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : T.Hashimoto                                              |*/
/*| Date        : 2001.10.31                                               |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
#include	<string.h>
#include	"system.h"
#include	"tbl_rkn.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"cnm_def.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"pri_def.h"
#include	"prm_tbl.h"
#include	"LKmain.h"
#include	"ntnet.h"
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
#include	"flp_def.h"
#include	"LKcom.h"
#include	"ntnet_def.h"
#include	"fla_def.h"
#include	"rau.h"
#include	"rauconstant.h"
#include	"ifm_ctrl.h"
#include	"oiban.h"

#define	LOGOPERATION_COM	0x01	/* �ʐM */
#define	LOGOPERATION_PRI	0x02	/* �� */
#define	LOGOPERATION_RAM	0x04	/* ���O */
#define	LOGOPERATION_ALL	0xFF	/* �S�Ď��s */

/* ���j�^/���샂�j�^�f�[�^����e�[�u���\�� */

typedef struct {
	uchar	ID;						/* �R�[�h */
	uchar	Level;					/* ���x�� */
	uchar	Operation;				/* �������@ */
									/* (LOGOPERATION_COM/LOGOPERATION_PRI/LOGOPERATION_RAM�̃r�b�g���) */
//	char	*Msg;					/* ���b�Z�[�W(H8�nCPU�ł̓Z�b�g���Ȃ�) */
} t_NTNET_MonitorCtrl;

typedef struct {
	uchar	Kind;					/* ��� */
	ulong	DataNum;				/* �z��uCtrlData�v�̗v�f�� */
	const t_NTNET_MonitorCtrl	*CtrlData;	/* �e�R�[�h���̎�舵����� */
} t_NTNET_MonitorTbl;
static TD_SETDIFFLOG_INFO	SetDiffWorkBuff;					// �f�[�^�ҏW���[�N
static unsigned char		SetDiffFirstFlag;					// ����i�[����t���O
extern unsigned short		SetDiffLogDtHeadPon;				// �ʃw�b�_�[�ʒu
static long*				CPrmMrr[C_PRM_SESCNT_MAX];			// �~���[�̈�Ǘ��p�A�h���X�z��
extern const ushort AppServ_CParamSizeTbl[C_PRM_SESCNT_MAX];	// �Z�b�V�������p�����[�^���e�[�u��
extern t_CParam CPMirror;										// ���ʃp�����[�^�~���[�̈�
void SetSetDiff(unsigned char syubetu);
void SetSetDiff_Set(unsigned char syubetu);
void SetDiff_PutLogInfoData( short ses, short addr, long dataBef, long dataAft );
void SetDiff_PutLogInfoHead( unsigned char syubetu );
void SetDiff_PutLogInfoSave( unsigned char cnt_add_flag );
void SetDiffLogBackUp( void );
void SetDiffLogRecoverCheck_Go( void );
void SetDiffLogMirrorAddress(void);
unsigned short SetDiff_UpdatePon ( unsigned short OldPon, unsigned short AddPon );

extern	ushort	FLT_Get_LogSectorCount(short id);


/* ���샂�j�^���O�f�[�^����e�[�u�� */
static const t_NTNET_MonitorCtrl c_OpeMonitorKind00[] = {
	{1,		5,	LOGOPERATION_ALL},// �S�C�j�V�����C�Y�N��(�X�[�p�[�C�j�V����)
	{2,		5,	LOGOPERATION_ALL},// �f�[�^�C�j�V�����N��(���O�C�j�V����)
};
static const t_NTNET_MonitorCtrl c_OpeMonitorKind01[] = {
	{1,		5,	LOGOPERATION_ALL},
	{2,		5,	LOGOPERATION_ALL},
	{3,		5,	LOGOPERATION_ALL},
	{4,		5,	LOGOPERATION_ALL},
	{5,		5,	LOGOPERATION_ALL},
	{6,		5,	LOGOPERATION_ALL},
	{7,		5,	LOGOPERATION_ALL},
	{8,		5,	LOGOPERATION_ALL},
	{11,	5,	LOGOPERATION_ALL},
	{12,	5,	LOGOPERATION_ALL},
	{15,	5,	LOGOPERATION_ALL},
	{20,	5,	LOGOPERATION_ALL},
	{21,	5,	LOGOPERATION_ALL},
	{22,	5,	LOGOPERATION_ALL},
	{28,	5,	LOGOPERATION_ALL},
	{29,	5,	LOGOPERATION_ALL},
	{30,	4,	LOGOPERATION_ALL},
	{31,	4,	LOGOPERATION_ALL},
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind11[] = {
	{1,		5,	LOGOPERATION_ALL},
	{2,		5,	LOGOPERATION_ALL},
	{3,		5,	LOGOPERATION_ALL},
	{4,		5,	LOGOPERATION_ALL},
	{5,		5,	LOGOPERATION_ALL},
	{6,		5,	LOGOPERATION_ALL},
	{7,		5,	LOGOPERATION_ALL},
	{8,		3,	LOGOPERATION_ALL},
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind12[] = {
	{1,		3,	LOGOPERATION_ALL},
	{2,		4,	LOGOPERATION_ALL},
	{3,		3,	LOGOPERATION_ALL},
	{4,		3,	LOGOPERATION_ALL},
	{5,		4,	LOGOPERATION_ALL},
	{6,		3,	LOGOPERATION_ALL},
	{7,		3,	LOGOPERATION_ALL},		// MT���v
	{8,		4,	LOGOPERATION_ALL},		// MT���v
	{9,		3,	LOGOPERATION_ALL},		// �O��MT���v
	{10,	3,	LOGOPERATION_ALL},
	{11,	4,	LOGOPERATION_ALL},
	{12,	3,	LOGOPERATION_ALL},
	{20,	3,	LOGOPERATION_ALL},
	{21,	4,	LOGOPERATION_ALL},
	{22,	3,	LOGOPERATION_ALL},
	{23,	4,	LOGOPERATION_ALL},
	{40,	5,	LOGOPERATION_ALL},		// T���v���~
	{41,	5,	LOGOPERATION_ALL},		// GT���v���~
	{42,	5,	LOGOPERATION_ALL},		// MT���v���~
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind13[] = {
	{1,		5,	LOGOPERATION_ALL},
	{2,		5,	LOGOPERATION_ALL},
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind15[] = {
	{1,		4,	LOGOPERATION_ALL},
	{2,		4,	LOGOPERATION_ALL},	// �S���b�N���u�J
	{11,	4,	LOGOPERATION_ALL},	// �t���b�v���~
	{12,	4,	LOGOPERATION_ALL},	// �S�t���b�v���~
	{21,	4,	LOGOPERATION_ALL},	// �t���b�v�Z���T����OFF
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind16[] = {
	{1,		4,	LOGOPERATION_ALL},
	{2,		4,	LOGOPERATION_ALL},	// �S���b�N���u��
	{11,	4,	LOGOPERATION_ALL},	// �t���b�v�㏸
	{12,	4,	LOGOPERATION_ALL},	// �S�t���b�v�㏸
	{21,	4,	LOGOPERATION_ALL},	// �t���b�v�Z���T����ON
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind17[] = {
	{1,		3,	LOGOPERATION_ALL},
	{2,		3,	LOGOPERATION_ALL},
	{3,		3,	LOGOPERATION_ALL},
	{4,		3,	LOGOPERATION_ALL},
	{5,		3,	LOGOPERATION_ALL},
	{6,		3,	LOGOPERATION_ALL},
	{7,		3,	LOGOPERATION_ALL},
	{8,		3,	LOGOPERATION_ALL},
	{10,	3,	LOGOPERATION_ALL},
	{11,	3,	LOGOPERATION_ALL},
	{12,	3,	LOGOPERATION_ALL},
	{13,	3,	LOGOPERATION_ALL},
	{14,	3,	LOGOPERATION_ALL},
	{15,	3,	LOGOPERATION_ALL},
	{16,	3,	LOGOPERATION_ALL},
	{17,	3,	LOGOPERATION_ALL},
	{18,	3,	LOGOPERATION_ALL},
	{19,	3,	LOGOPERATION_ALL},
	{20,	3,	LOGOPERATION_ALL},
	{21,	3,	LOGOPERATION_ALL},
	{22,	3,	LOGOPERATION_ALL},
	{23,	3,	LOGOPERATION_ALL},
	{24,	3,	LOGOPERATION_ALL},
	{25,	3,	LOGOPERATION_ALL},
	{26,	3,	LOGOPERATION_ALL},
	{50,	3,	LOGOPERATION_ALL},
	{51,	3,	LOGOPERATION_ALL},
	{52,	3,	LOGOPERATION_ALL},
	{53,	3,	LOGOPERATION_ALL},
	{68,	3,	LOGOPERATION_ALL},
	{69,	3,	LOGOPERATION_ALL},
	{70,	3,	LOGOPERATION_ALL},
	{71,	3,	LOGOPERATION_ALL},
	{72,	3,	LOGOPERATION_ALL},
	{73,	3,	LOGOPERATION_ALL},
	{74,	3,	LOGOPERATION_ALL},
	{75,	3,	LOGOPERATION_ALL},

// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	{76,	5,	LOGOPERATION_ALL},			// Edy�װю����������
//	{77,	5,	LOGOPERATION_ALL},			// Edy�װю�����ر
//	{78,	5,	LOGOPERATION_ALL},			// �֘A�ݒ�ύX
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)

	{79,	3,	LOGOPERATION_ALL},	// �����ē����ԕύX
	{80,	3,	LOGOPERATION_ALL},	// ���ʗj���ύX
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	{81,	5,	LOGOPERATION_ALL},			// Edy�װђ��ߋL�^��������
//	{82,	5,	LOGOPERATION_ALL},			// Edy�װђ��ߋL�^���ر
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	{83,	3,	LOGOPERATION_ALL},
	{90,	5,	LOGOPERATION_ALL},			// �v���O�����_�E�����[�h���{
	{91,	5,	LOGOPERATION_ALL},			// ���ʃp�����[�^�_�E�����[�h���{
	{92,	5,	LOGOPERATION_ALL},			// ���C���v���O�����^�p�ʐؑ֗\����{
	{93,	5,	LOGOPERATION_ALL},			// �T�u�v���O�����^�p�ʐؑ֗\����{
	{94,	5,	LOGOPERATION_ALL},			// ���ʃp�����[�^�[�^�p�ʐؑ֗\����{
	{95,	3,	LOGOPERATION_ALL},			// �����ݒ�v�����g

	{97,	3,	LOGOPERATION_ALL},
// MH321800(S) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	{98,	3,	LOGOPERATION_ALL},
// MH321800(E) G.So IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	{99,	3,	LOGOPERATION_ALL},

};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind18[] = {
	{1,		3,	(LOGOPERATION_ALL)},
	{2,		3,	(LOGOPERATION_ALL)},
	{3,		3,	(LOGOPERATION_ALL)},
	{4,		3,	(LOGOPERATION_ALL)},
	{5,		3,	(LOGOPERATION_ALL)},
	{6,		3,	(LOGOPERATION_ALL)},
	{30,	5,	(LOGOPERATION_ALL)},
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind20[] = {
	{1,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{2,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{3,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{10,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{11,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{12,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{20,	4,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind23[] = {
	{1,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{2,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{3,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{4,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{5,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{6,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{7,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{8,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{9,		0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{10,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{11,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{12,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{13,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{14,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{18,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{19,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{20,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{23,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{24,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{25,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{26,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{27,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{28,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{38,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{39,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{41,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{42,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// ��������(Park I PRO����)����o�^
	{43,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// ��������(���u�ʐM����)����o�^
	{44,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	{45,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// ��������(Edy����)����o�^ 
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	{46,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{47,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// ��������(ۯ����u���춳��-�ر)
	{48,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// ��������(�ׯ�ߓ��춳��)
	{49,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// ��������(�ׯ�ߓ��춳��-�ر)
	{50,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{61,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{70,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// CRR��o�[�W�����`�F�b�N���{
	{71,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// CRR��܂�Ԃ��e�X�g���{
	{72,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// Rism�T�[�o�[�ڑ��m�F
	{73,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{74,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{75,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{78,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{79,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{80,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	{83,	0,	(LOGOPERATION_COM|LOGOPERATION_RAM)},	// ���σ��[�_�`�F�b�N���{
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind25[] = {
	{2,	5,	LOGOPERATION_ALL},
	{20,	5,	LOGOPERATION_ALL},
	{21,	5,	LOGOPERATION_ALL},	// �Ԏ��̏�ݒ�i�̏�Ȃ�������j
	{22,	5,	LOGOPERATION_ALL},	// �Ԏ��̏�ݒ�i�̏Ⴀ�聨�Ȃ��j
	{23,	3,	LOGOPERATION_ALL},	// ��t�����s�ݒ�
	{70,	3,	LOGOPERATION_ALL},	// �N���W�b�g�����M����đ�
	{71,	3,	LOGOPERATION_ALL},	// �N���W�b�g�����M����폜
	{72,	3,	LOGOPERATION_ALL},	// �N���W�b�g���㋑�ۃf�[�^�폜
	{73,	3,	LOGOPERATION_ALL},	// �ڑ��m�F�e�X�g���{
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind80[] = {
	{1,		4,	LOGOPERATION_ALL},
	{2,		4,	LOGOPERATION_ALL},
	{5,		4,	LOGOPERATION_ALL},
	{10,	4,	LOGOPERATION_ALL},
	{11,	4,	LOGOPERATION_ALL},
	{12,	4,	LOGOPERATION_ALL},
	{13,	4,	LOGOPERATION_ALL},
	{14,	4,	LOGOPERATION_ALL},
	{15,	4,	LOGOPERATION_ALL},
	{16,	4,	LOGOPERATION_ALL},
	{17,	4,	LOGOPERATION_ALL},
	{28,	4,	LOGOPERATION_ALL},
	{30,	4,	LOGOPERATION_ALL},
	{31,	4,	LOGOPERATION_ALL},
	{32,	4,	LOGOPERATION_ALL},
	{33,	5,	LOGOPERATION_ALL},
	{34,	5,	LOGOPERATION_ALL},
	{35,	5,	LOGOPERATION_ALL},	// SNTP �������v�X�V�@�\�ɂ�鎞�v�Z�b�g
	{36,	5,	LOGOPERATION_ALL},	// SNTP ���������ύX
	{40,	4,	LOGOPERATION_ALL},
	{41,	4,	LOGOPERATION_ALL},
	{42,	4,	LOGOPERATION_ALL},	// �t���b�v���~
	{43,	4,	LOGOPERATION_ALL},	// �t���b�v�㏸
	{44,	4,	LOGOPERATION_ALL},	// �S���b�N���u�J
	{45,	4,	LOGOPERATION_ALL},	// �S���b�N���u��
	{46,	4,	LOGOPERATION_ALL},	// �S�t���b�v���~
	{47,	4,	LOGOPERATION_ALL},	// �S�t���b�v�㏸
	{48,	0,	LOGOPERATION_ALL},	// �Ïؔԍ�����
	{51,	5,	LOGOPERATION_ALL},	// ���z���Z
	{52,	5,	LOGOPERATION_ALL},	// �U�֐��Z
	{53,	4,	LOGOPERATION_ALL},	// ��t�����s
	{54,	4,	LOGOPERATION_ALL},	// �̎����Ĕ��s
// MH322914(S) K.Onodera 2016/09/16 AI-V�Ή��F�U�֐��Z
	{55,	5,	LOGOPERATION_ALL},	// �̎����Ĕ��s
// MH322914(E) K.Onodera 2016/09/16 AI-V�Ή��F�U�֐��Z
	{84,	4,	LOGOPERATION_ALL},		// ���u�_�E�����[�h�v������
	{85,	4,	LOGOPERATION_ALL},		// Rism�ڑ��ؒf�v��
	{86,	4,	LOGOPERATION_ALL},		// ���u�_�E�����[�h�I��
	{87,	4,	LOGOPERATION_ALL},		// �v���O�����X�V�I��
	{88,	4,	LOGOPERATION_ALL},		// ���u�_�E�����[�h�E�v���O�����X�V���ʃA�b�v���[�h
	{89,	4,	LOGOPERATION_ALL},		// Rism�ڑ������v��
	{90,	4,	LOGOPERATION_ALL},
	{91,	4,	LOGOPERATION_ALL},
	{92,	4,	LOGOPERATION_ALL},		// �v�J�[�h�F�؃e�[�u���X�V
	{93,	4,	LOGOPERATION_ALL},		// �@�l�J�[�h�F�؃e�[�u���X�V
	{94,	4,	LOGOPERATION_ALL},		// �s�J�[�h��{�|�C���g���[�g�X�V
	{95,	4,	LOGOPERATION_ALL},		// �s�J�[�h���ԃ|�C���g���[�g�X�V
	{96,	4,	LOGOPERATION_ALL},		// �s�J�[�h���p�ۍX�V
	{97,	4,	LOGOPERATION_ALL},		// �����W���f�[�^�o�^���
};

static const t_NTNET_MonitorCtrl c_OpeMonitorKind81[] = {
	{1,		4,	LOGOPERATION_ALL},
	{2,		4,	LOGOPERATION_ALL},
	{3,		4,	LOGOPERATION_ALL},
};

static const t_NTNET_MonitorTbl c_OpeMonitorTbl[] = {
	{0,		_countof(c_OpeMonitorKind00),	c_OpeMonitorKind00},
	{1,		_countof(c_OpeMonitorKind01),	c_OpeMonitorKind01},
	{11,	_countof(c_OpeMonitorKind11),	c_OpeMonitorKind11},
	{12,	_countof(c_OpeMonitorKind12),	c_OpeMonitorKind12},
	{13,	_countof(c_OpeMonitorKind13),	c_OpeMonitorKind13},
	{15,	_countof(c_OpeMonitorKind15),	c_OpeMonitorKind15},
	{16,	_countof(c_OpeMonitorKind16),	c_OpeMonitorKind16},
	{17,	_countof(c_OpeMonitorKind17),	c_OpeMonitorKind17},
	{18,	_countof(c_OpeMonitorKind18),	c_OpeMonitorKind18},
	{20,	_countof(c_OpeMonitorKind20),	c_OpeMonitorKind20},
	{23,	_countof(c_OpeMonitorKind23),	c_OpeMonitorKind23},
	{25,	_countof(c_OpeMonitorKind25),	c_OpeMonitorKind25},
	{80,	_countof(c_OpeMonitorKind80),	c_OpeMonitorKind80},
	{81,	_countof(c_OpeMonitorKind81),	c_OpeMonitorKind81},
};

/* ���j�^���O����e�[�u�� */
#define	_MONCODE(n)	(n%100)
static const t_NTNET_MonitorCtrl c_MonitorKind00[] = {
	{_MONCODE(OPMON_RSETUP_REFOK),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_REFNG),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_PREOK),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_PRENG),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_RCVOK),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_RCVNG),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_EXEOK),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_EXENG),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_CHGOK),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_CHGNG),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_CANOK),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_CANNG),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{_MONCODE(OPMON_RSETUP_NOREQ),	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
};
static const t_NTNET_MonitorCtrl c_MonitorKind02[] = {
	{50,	5,	(LOGOPERATION_PRI|LOGOPERATION_RAM)},
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	{52,	5,	(LOGOPERATION_PRI|LOGOPERATION_RAM)},	// �d�q�}�l�[�����茋�ʓd����M
	{53,	5,	(LOGOPERATION_PRI|LOGOPERATION_RAM)},	// �N���W�b�g���ό��ʓd����M
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�
};

static const t_NTNET_MonitorCtrl c_MonitorKind01[] = {
	{1,		5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{2,		5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{3,		5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{4,		5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{5,		5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{6,		5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{7,		5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{8,		5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{11,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{12,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{13,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{14,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{21,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{22,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{23,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{24,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{25,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{26,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{27,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{28,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},
	{45,	5,	(LOGOPERATION_ALL)},		// �J�[�h�l�܂蔭��
	{46,	5,	(LOGOPERATION_ALL)},		// �J�[�h�l�܂����
	{50,	5,	(LOGOPERATION_ALL)},		// ����ʐM�J�n
	{51,	5,	(LOGOPERATION_ALL)},		// ����ʐM�I��
	{52,	5,	(LOGOPERATION_ALL)},		// �蓮�s���v�ɂ��Z���^�[�ʐM�J�n
	{53,	5,	(LOGOPERATION_ALL)},		// �����s���v�ɂ��Z���^�[�ʐM�J�n
	{54,	5,	(LOGOPERATION_ALL)},		// �蓮�Z���^�[�ʐM�J�n�i�����e����j
	{55,	5,	(LOGOPERATION_ALL)},		// �����Z���^�[�ʐM�J�n
	{56,	5,	(LOGOPERATION_ALL)},		// �Z���^�[�ʐM�I��
	{57,	5,	(LOGOPERATION_ALL)},		// ���[�_�[���ݔF�،��X�V
// MH810103(s) �d�q�}�l�[�Ή� ���������␳�̃��j�^�C��
//	{70,	5,	(LOGOPERATION_COM|LOGOPERATION_RAM)},		// ���������␳
	{70,	5,	(LOGOPERATION_ALL)},		// ���������␳
// MH810103(e) �d�q�}�l�[�Ή� ���������␳�̃��j�^�C��
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
	{71,	5,	(LOGOPERATION_ALL)},		// �W�v�N���A�ُ�(�@�󎚏W�v�̃w�b�_�[�ƑO��W�v�̃w�b�_�[�̔�r)
	{72,	5,	(LOGOPERATION_ALL)},		// �W�v�N���A�ُ�(�A���ݏW�v�̒ǔԁA����W�v�����A�O��W�v������ �󎚏W�v�̒ǔԁA����W�v�����A�O��W�v�����̔�r)
	{73,	5,	(LOGOPERATION_ALL)},		// �W�v�N���A�ُ�(�B���ݏW�v���ڂ̒l�ƈ󎚏W�v���ڂ̒l�̔�r)
	{74,	5,	(LOGOPERATION_ALL)},		// �W�v�N���A�ُ�(�C���ݏW�v�̍���W�v������0�N���A����Ă��邩�m�F)
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
// MH321800(S) D.Inaba IC�N���W�b�g�Ή�
	{75,	5,	(LOGOPERATION_ALL)},		// �d�q�}�l�[������������
	{76,	5,	(LOGOPERATION_ALL)},		// �d�q�}�l�[���������^�C���A�E�g
// MH321800(E) D.Inaba IC�N���W�b�g�Ή�	
// GG129001(S) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
	{77,	5,	(LOGOPERATION_ALL)},		// �ޑK�Ǘ��N���A�ُ�(�C���ݏW�v�̍���W�v������0�N���A����Ă��邩�m�F)
	{78,	5,	(LOGOPERATION_ALL)},		// �ޑK�Ǘ��N���A�ُ�(�D���݂̒ǔԁA�W�����A�@�B����0�N���A����Ă���ꍇ�̓N���A�L�����Z��)
// GG129001(E) T���v�󎚂��}�C�i�X�ɂȂ�Ȃ��悤�΍�(���ʉ��PNo.1604)�iMH364304���p�j
	{85,	5,	(LOGOPERATION_ALL)},		// �N���W�b�g�@�R�}���h���M�v��
	{86,	5,	(LOGOPERATION_ALL)},		// �N���W�b�g�@�R�}���h��������
	{87,	5,	(LOGOPERATION_ALL)},		// �N���W�b�g�@�R�}���h�^�C���A�E�g

};

static const t_NTNET_MonitorCtrl c_MonitorKind05[] = {
	{4,		5,	LOGOPERATION_ALL},		// ���O�^�C������
	{5,		4,	LOGOPERATION_ALL},
	{6,		4,	LOGOPERATION_ALL},
	{10,	4,	LOGOPERATION_ALL},
	{11,	4,	LOGOPERATION_ALL},
	{15,	4,	LOGOPERATION_ALL},
	{16,	4,	LOGOPERATION_ALL},
	{17,	4,	LOGOPERATION_ALL},
	{18,	4,	LOGOPERATION_ALL},
	{20,	4,	LOGOPERATION_ALL},		// ����
	{21,	4,	LOGOPERATION_ALL},		// �o��
};
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
static const t_NTNET_MonitorCtrl c_MonitorKind06[] = {
	{1,		5,	LOGOPERATION_ALL},		// 
};
#endif
// �����p (e)

static const t_NTNET_MonitorTbl c_MonitorTbl[] = {
	{0,		_countof(c_MonitorKind00),	c_MonitorKind00},
	{1,		_countof(c_MonitorKind01),	c_MonitorKind01},
	{2,		_countof(c_MonitorKind02),	c_MonitorKind02},
	{5,		_countof(c_MonitorKind05),	c_MonitorKind05},
// �����p (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
	{99,	_countof(c_MonitorKind06),	c_MonitorKind06},
#endif
// �����p (e)
};

static const t_NTNET_MonitorCtrl	*getMonitorInfo(uchar kind, uchar code, const t_NTNET_MonitorTbl *tbl, size_t tbl_cnt);
#define	_getOpeLogInfo(kind, code)	getMonitorInfo(kind, code, c_OpeMonitorTbl, _countof(c_OpeMonitorTbl))
#define	_getMonLogInfo(kind, code)	getMonitorInfo(kind, code, c_MonitorTbl, _countof(c_MonitorTbl))


static void	Log_fusei_mk( void );
ushort	Ope_Log_GetLogMaxCount( short id );

#define	RAU_NTBUF_NEARFULL_COUNT			6

#define	TOTALLOG_NTDATA_COUNT				8	// �W�v���O����쐬����NT-NET�f�[�^���(���uPhase2��6���)
void	Log_SetTotalLogNtSeqNo(short Lno, SYUKEI* pTotalLog);

/*[]----------------------------------------------------------------------[]*/
/*| log�o�^                                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Log_regist( Lno )                                       |*/
/*| PARAMETER    : ۸އ�                                                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void Log_regist( short Lno )
{
	T_FrmReceipt	ReceiptPreqData;	// ���̎��؈󎚗v����				ү�����ް�
	T_FrmFusKyo		FusKyoPreqData;		// ���s���E�����o�ɏ��󎚗v����	ү�����ް�
	T_FrmTeiFukJou	TeiFukJouPreqData;	// ���╜�d���󎚗v����			ү�����ް�
	T_FrmOpeJou		OpeJouPreqData;		// ��������󎚗v����				ү�����ް�
	uchar			jnl_pri = OFF;		// �ެ��و󎚗L��
	T_FrmMonJou		MonJouPreqData;		// ��������󎚗v����				ү�����ް�
	T_FrmRmonJou	RmonJouPreqData;	// �����u�Ď����󎚗v����			ү�����ް�
	const t_NTNET_MonitorCtrl	*MonCtrl;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	ushort SyashitsuNum;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	ushort outkind;
	ushort			logCount;
	ushort			i;
// MH810105(S) MH364301 �C���{�C�X�Ή�
	long			jnl_pri_jadge;		// 17-0001�̒l�̈ꎞ�ۑ�
// MH810105(E) MH364301 �C���{�C�X�Ή�

	switch( Lno ){
		case LOG_PAYMENT:	/* �ʐ��Z��� */
			outkind = 0;
			if( ryo_buf.pkiti != 0xffff ){												// ���ݐ��Z�łȂ�
				if( (ryo_buf.svs_tim == 1)&&											// �T�[�r�X�^�C����
					(FLAPDT.flp_data[ryo_buf.pkiti-1].lag_to_in.BIT.LAGIN != ON )){		// ���O�^�C���A�b�v��ȊO
					outkind = 99;
				}
			}else{																		// ���ݐ��Z�̂Ƃ�
				if( (ryo_buf.svs_tim == 1)&&											// �T�[�r�X�^�C����
					(LOCKMULTI.lock_mlt.lag_to_in.BIT.LAGIN != ON )){					// ���O�^�C���A�b�v��ȊO
					outkind = 99;
				}
			}
// MH810100(S) K.Onodera  2020/02/20 #3912 �Ԕԃ`�P�b�g���X(�����Z�o�ɑΉ�)
			if( OpeNtnetAddedInfo.PayMethod == 13 ){	// ������Z
				outkind = 43;	// �����Z�o�Ɏԗ����Z(�㕥��)
			}
// MH810100(E) K.Onodera  2020/02/20 #3912 �Ԕԃ`�P�b�g���X(�����Z�o�ɑΉ�)
			PayData.OutKind = (uchar)outkind;
			NTNET_Data152Save((void *)(&PayData.OutKind), NTNET_152_OUTKIND);
			SetAddPayData();
// GG129002(S) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�̎��؂��r���ŃJ�b�g����錻�ۂ̑΍�j
//// GG129000(S) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
//			if( OPECTL.Ope_mod == 3 ){
//				pri_ryo_stock(R_PRI);
//			}
//// GG129000(E) D.Inaba 2023/09/21 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�ڍא݌v#5373�F�V���V�[�g�v�����^�Ή��j
// GG129002(E) �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�̎��؂��r���ŃJ�b�g����錻�ۂ̑΍�j
			Log_Write(eLOG_PAYMENT, &PayData, TRUE);
			IFM_Snd_Payment(&PayData);	// ���Z�f�[�^�~��
// MH810103 GG119202(S) ������������L�^�ɍĐ��Z�����󎚂���
			EcAlarmLog_RepayLogRegist(&PayData);
// MH810103 GG119202(E) ������������L�^�ɍĐ��Z�����󎚂���
			if( PrnJnlCheck() == ON ){
				//	�ެ���������ڑ�����̏ꍇ
// MH810105(S) MH364301 �C���{�C�X�Ή�
//				switch(CPrmSS[S_RTP][1]){	// �ެ��و󎚏����H
				jnl_pri_jadge = prm_get( COM_PRM, S_RTP, 1, 2, 1 );			// 17-0001�̒l��ϐ��ɕۑ�

				jnl_pri_jadge = invoice_prt_param_check( jnl_pri_jadge );	// �C���{�C�X���p�����[�^�ϊ�

				switch(jnl_pri_jadge){	// �ެ��و󎚏����H
// MH810105(E) MH364301 �C���{�C�X�Ή�
					case	0:
					case	1:
					case	2:
						// �ެ��ق͂O�~�����L�^
						jnl_pri = ON;
						break;
					case	3:
					case	4:
					case	5:
						// �ެ��ق͒��ԗ����O�~���ȊO�L�^
						if( PayData.WPrice != 0 ){
							//	���ԗ������O
							jnl_pri = ON;
						}
						break;

					case	6:
					case	7:
					case	8:
						// �ެ��ق͌����̎��O�~���ȊO�L�^
						if( PayData.WTotalPrice != 0 ){
							//	�����̎��z���O
							jnl_pri = ON;
						}
						break;
					case	9:
					case   10:
					case   11:
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή�
//						if( PayData.WTotalPrice != 0 || PayData.Electron_data.Suica.e_pay_kind ){
						if( PayData.WTotalPrice != 0 ||
							SuicaUseKindCheck(PayData.Electron_data.Suica.e_pay_kind) ||
							EcUseKindCheck(PayData.Electron_data.Ec.e_pay_kind) ){
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή�
							//	�����̎��z���O OR �d�q�}�̂ɂ�鐸�Z���s��ꂽ�ꍇ
							jnl_pri = ON;
						}
						break;
// GG129001(S) �̎��؃v�����g�ݒ�̓ǂݑւ��Ή�
					case   12:
					case   13:
					case   14:
// GG129001(S) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۊz���Z�b�g����j
//						if(Billingcalculation(&PayData) != 0){
						if(PayData.WTaxPrice != 0){
// GG129001(E) �f�[�^�ۊǃT�[�r�X�Ή��i�ېőΏۊz���Z�b�g����j
							jnl_pri = ON;
						}
						break;
// GG129001(E) �̎��؃v�����g�ݒ�̓ǂݑւ��Ή�
					default:
						jnl_pri = OFF;
// MH322914 (s) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)
						break;
// MH322914 (e) kasiyama 2016/07/08 [break]������(���ʉ��PNo.896)(MH341106)

				}

// MH810105(S) MH364301 �C���{�C�X�Ή��i���Z�����W�v�̃^�C�~���O�ύX�j
//				if( jnl_pri == ON && ryo_buf.ryos_fg == 0){
				if( jnl_pri == ON ){
// MH810105(E) MH364301 �C���{�C�X�Ή��i���Z�����W�v�̃^�C�~���O�ύX�j
					//	�ެ��و󎚏����𖞂����ꍇ
					ReceiptPreqData.prn_kind = J_PRI;											// �o��������i�ެ��فj
// MH810104 GG119201(S) �d�q�W���[�i���Ή�
//					Pri_Work[1].Receipt_data = PayData;
//					ReceiptPreqData.prn_data = &Pri_Work[1].Receipt_data;	// �ް��߲�����
					memcpy( &Cancel_pri_work, &PayData, sizeof( Receipt_data ) );
					ReceiptPreqData.prn_data = &Cancel_pri_work;			// �ް��߲�����
// MH810104 GG119201(E) �d�q�W���[�i���Ή�
					ReceiptPreqData.reprint = OFF;												// �Ĕ��s�׸�ؾ�āi�ʏ�j
					queset( PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqData );	// �󎚗v��ү���ޓo�^
// GG129000(S) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/���Z�������̗̎��ؔ��s���ɑҋ@��Ԃɖ߂�Ȃ��悤�ɂ���j
					if( IS_INVOICE ){															// �C���{�C�X
						if( OPECTL.Ope_mod == 3 && OPECTL.f_DelayRyoIsu == 0 ){					// ���Z�����A�W���[�i���󎚖����{
							// ���Z������ryo_isu�����O��kan_syuu(Log_regist)�����������ꍇ�AOPECTL.f_DelayRyoIsu�������Ȃ�����
							// �C���{�C�X�ݒ肠��A�W���[�i���󎚖����{�ł���΃W���[�i���󎚊����҂��Ƃ���
							OPECTL.f_DelayRyoIsu = 1;
						}
					}
// GG129000(E) H.Fujinaga 2022/12/08 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i�C���{�C�X�Ή�/���Z�������̗̎��ؔ��s���ɑҋ@��Ԃɖ߂�Ȃ��悤�ɂ���j
				}
			}

// MH810100(S) K.Onodera  2019/12/18 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//			NTNET_Snd_Data22_FusDel( ryo_buf.pkiti, (ushort)PayData.PayMethod, (ushort)PayData.PayClass, outkind );	// �s���o�ɏ�񂠂�Ώ���
// MH810100(E) K.Onodera  2019/12/18 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
			break;
		case LOG_PAYSTOP:	/* ���Z���~��� */
			SetAddPayData();
			memcpy( &Cancel_pri_work, &PayData, sizeof( Receipt_data ) );
// MH810100(S) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
			if( PAY_CAN_DATA_SEND ){	// ParkingWeb/RealTime���Z���~�f�[�^���M����
// MH810100(E) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
			Log_Write(eLOG_PAYMENT, &PayData, TRUE);
			IFM_Snd_Payment(&PayData);	// ���Z�f�[�^�~��
// MH810100(S) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
			}
// MH810100(E) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
			if( PrnJnlCheck() == ON ){
				//	�ެ���������ڑ�����̏ꍇ
				ReceiptPreqData.prn_kind = J_PRI;											// �o��������i�ެ��فj
				ReceiptPreqData.prn_data = &Cancel_pri_work;	// �ް��߲�����
				ReceiptPreqData.reprint = OFF;												// �Ĕ��s�׸�ؾ�āi�ʏ�j
				queset( PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqData );	// �󎚗v��ү���ޓo�^
			}

// MH810100(S) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
//			IoLog_write(IOLOG_EVNT_AJAST_STP, (ushort)Cancel_pri_work.WPlace, 0, 0);
// MH810100(E) K.Onodera 2020/03/26 �Ԕԃ`�P�b�g���X(�d�l�ύX:���Z���~���APW/RT���Z�f�[�^�𑗐M���Ȃ�)
			break;
// MH810105(S) #6207 �����ă^�b�`�҂����ɓd��OFF��ON��ANTNET���Z�f�[�^�𑗐M���Ȃ�
		case LOG_PAYSTOP_FU:												// ���d���̒��~�f�[�^
			SetAddPayData();
// MH810105(S) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6386 50-0014�Ő��Z���~�܂��́A�x�Ɛݒ�Ō��ώ��s���Ő��Z���~�����ۂɐ��Z�f�[�^�����M����Ȃ�
			// �����ɂ��Ă͂����ŃN���A���Ă����i8021�́ANTNET_Edit_Data56_rXX�œ������j
			memset( PayData.DiscountData, 0, sizeof( DISCOUNT_DATA )*WTIK_USEMAX );/* ���Z�ް��������쐬�ر�ر	*/
// MH810105(E) MH364301 QR�R�[�h���ϑΉ� GT-4100 #6386 50-0014�Ő��Z���~�܂��́A�x�Ɛݒ�Ō��ώ��s���Ő��Z���~�����ۂɐ��Z�f�[�^�����M����Ȃ�
			memcpy( &Cancel_pri_work, &PayData, sizeof( Receipt_data ) );
			// ParkingWeb�ɑ��M����
			Log_Write(eLOG_PAYMENT, &PayData, TRUE);
			IFM_Snd_Payment(&PayData);	// ���Z�f�[�^�~��
			if( PrnJnlCheck() == ON ){
				//	�ެ���������ڑ�����̏ꍇ
				ReceiptPreqData.prn_kind = J_PRI;											// �o��������i�ެ��فj
				ReceiptPreqData.prn_data = &Cancel_pri_work;	// �ް��߲�����
				ReceiptPreqData.reprint = OFF;												// �Ĕ��s�׸�ؾ�āi�ʏ�j
				queset( PRNTCBNO, PREQ_RYOUSYUU, sizeof(T_FrmReceipt), &ReceiptPreqData );	// �󎚗v��ү���ޓo�^
			}
			break;
// MH810105(E) #6207 �����ă^�b�`�҂����ɓd��OFF��ON��ANTNET���Z�f�[�^�𑗐M���Ȃ�
		case LOG_ABNORMAL:	/* �s���E�����o�ɏ�� */
			Log_fusei_mk();																	// Make Data
			// �s���E�����o�Ƀ��O��1���󎚑O�Ɏ��̃f�[�^����������\�������邽�߈󎚃o�b�t�@�Ƃ��ėp�ӂ���K�v�L
			if( FLP_LOG_DAT.Flp_wtp < 0 || FLP_LOG_DAT.Flp_wtp >= FLP_LOG_CNT ){
				FLP_LOG_DAT.Flp_wtp = 0;
			}
			memcpy( &FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp], &logwork.flp, sizeof( flp_log ) );
			Log_Write(eLOG_ABNORMAL, &FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp], TRUE);// �s���E�����o�Ƀ��O�o�^
			if( PrnJnlCheck() == ON ){
				//	�ެ���������ڑ�����̏ꍇ
				FusKyoPreqData.prn_kind = J_PRI;											// �o��������i�ެ��فj
				FusKyoPreqData.prn_data = &FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp];	// �ް��߲�����
				queset( PRNTCBNO, PREQ_FUSKYO_JOU, sizeof(T_FrmFusKyo), &FusKyoPreqData );	// �󎚗v��ү���ޓo�^
			}
			if(logwork.flp.Lok_inf == 0){
				IoLog_write(IOLOG_EVNT_FORCE_FIN, (ushort)FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp].WPlace, 0, 0);
				CountUp( KIYOUSEI_COUNT );													//�����o�ɒǂ���
				PayInfo_Class = 100;										// ���Z���f�[�^�p�����敪���Q�Ƃ���
			}else{
				IoLog_write(IOLOG_EVNT_OUT_ILLEGAL, (ushort)FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp].WPlace, 0, 0);
				CountUp( FUSEI_COUNT );														//�s���o�ɒǂ���
				PayInfo_Class = 101;										// ���Z���f�[�^�p�����敪���Q�Ƃ���
			}
			if(_is_ntnet_remote() && prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
				RAU_UpdateCenterSeqNo(RAU_SEQNO_PAY);
			}
			FLP_LOG_DAT.Flp_wtp++;															// ײ��߲�� +1
			if( FLP_LOG_DAT.Flp_wtp >= FLP_LOG_CNT ){
				FLP_LOG_DAT.Flp_wtp = 0;
			}
			break;
		case LOG_TTOTAL:	/* �s���v��� */
			Log_SetTotalLogNtSeqNo(LOG_TTOTAL, &TSYU_LOG_WK);											// �V�[�P���V����No.�̐ݒ�
			Log_Write(eLOG_TTOTAL, &TSYU_LOG_WK, TRUE);										// �s���v���O�o�^
// MH810100(S) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
// Log_Write()/Log_Write_Pon()����eLOG_TTOTAL�̏ꍇ��ac_flg.syusyu=15�Ƃ��A�X��eLOG_LCKTTL�̓o�^�ł�ac_flg.syusyu=19
// �Ƃ��Ă��邪�A�����ł�ac_flg.syusyu=19�Ƃ��Ă��A���d�����ł�ac_flg.syusyu=15/19���ɉ���������ac_flg.syusyu=16�ɂ�
// �邱�ƂɂȂ�̂ŁA�����ł͒P���ɎԎ����W�v���O�o�^���X�L�b�v����
//-			Log_Write(eLOG_LCKTTL, &LCKT_LOG_WK, TRUE);										// �Ԏ����W�v���O(31�Ԏ��ȍ~)�o�^
// MH810100(E) K.Onodera  2019/12/24 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
			break;
		case LOG_GTTOTAL:	/* �f�s���v��� */
			Log_SetTotalLogNtSeqNo(LOG_GTTOTAL, &skyprn);									// �V�[�P���V����No.�̐ݒ�
			Log_Write(eLOG_GTTOTAL, &skyprn, TRUE);											// �f�s���v���O�o�^
			break;
		case LOG_MONEYMANAGE:	/* ���K�Ǘ���� */
			// NT-NET�ł͍ŐV�̂P���݂̂�o�^����ׁA�o�^�ς݃f�[�^����ǂ݂���
			logCount = Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);
			if(logCount > 0){
				for(i=0; i<logCount; i++){
					Ope_Log_TargetDataGet( eLOG_MONEYMANAGE, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
				}
			}
			turi_kan.SeqNo = GetNtDataSeqNo();											// ���K�Ǘ��f�[�^�̃V�[�P���V����No.�̐ݒ�
			Log_Write(eLOG_MONEYMANAGE, &turi_kan, TRUE);								// ���K�Ǘ����O�o�^
			turi_kan.SeqNo = GetNtDataSeqNo();											// �ޑK�Ǘ��f�[�^�̃V�[�P���V����No.�̐ݒ�
			Log_Write(eLOG_MNYMNG_SRAM, &turi_kan, FALSE);								// ���K�Ǘ����O�o�^(SRAM)
			break;
		case LOG_COINBOX:	/* ��݋��ɏW�v��� */
			coin_syu.SeqNo = GetNtDataSeqNo();											// �V�[�P���V����No.�̐ݒ�
			Log_Write(eLOG_COINBOX, &coin_syu, FALSE);									// �R�C�����ɏW�v���O�o�^
			Make_Log_MnyMng( 10 );														// ���K�Ǘ����O�f�[�^�쐬
			// NT-NET�ł͍ŐV�̂P���݂̂�o�^����ׁA�o�^�ς݃f�[�^����ǂ݂���
			logCount = Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);
			if(logCount > 0){
				for(i=0; i<logCount; i++){
					Ope_Log_TargetDataGet( eLOG_MONEYMANAGE, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
				}
			}
			turi_kan.SeqNo = GetNtDataSeqNo();											// ���K�Ǘ��f�[�^�̃V�[�P���V����No.�̐ݒ�
			Log_Write(eLOG_MONEYMANAGE, &turi_kan, TRUE);								// ���K�Ǘ����O�o�^
			break;
		case LOG_NOTEBOX:	/* �������ɏW�v��� */
			note_syu.SeqNo = GetNtDataSeqNo();											// �V�[�P���V����No.�̐ݒ�
			Log_Write(eLOG_NOTEBOX, &note_syu, FALSE);									// �������ɏW�v���O�o�^
			Make_Log_MnyMng( 11 );														// ���K�Ǘ����O�f�[�^�쐬
			// NT-NET�ł͍ŐV�̂P���݂̂�o�^����ׁA�o�^�ς݃f�[�^����ǂ݂���
			logCount = Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);
			if(logCount > 0){
				for(i=0; i<logCount; i++){
					Ope_Log_TargetDataGet( eLOG_MONEYMANAGE, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
				}
			}
			turi_kan.SeqNo = GetNtDataSeqNo();											// ���K�Ǘ��f�[�^�̃V�[�P���V����No.�̐ݒ�
			Log_Write(eLOG_MONEYMANAGE, &turi_kan, TRUE);								// ���K�Ǘ����O�o�^
			break;
		case LOG_POWERON:	/* �╜�d��� */
			memcpy(&Pon_log_work.Pdw_Date, &clk_save, sizeof(date_time_rec));			// ��d�����o�^
			memcpy( &Pon_log_work.Pon_Date,
					&CLK_REC, sizeof( date_time_rec ) );				// ���d�����o�^

			Pon_log_work.Pon_nmi = (!FLAGPT.nmi_flg);					// nmi�׸ޓo�^ 
			Pon_log_work.Pon_kind = (unsigned char)(( StartingFunctionKey == 1 ) ? 3 :
																					( StartingFunctionKey == 2 ) ? 1 :
																					( StartingFunctionKey == 3 ) ? 2 :
																								StartingFunctionKey	);	/* nmi��ޓo�^ */
			// �������Ұ� FlashROM��SRAM���s��������set�iPon_kind��b7���g�p�F1=�s�����j
			if( 0 != (PowonSts_Param & 0x4000) || 0 != (PowonSts_LockParam & 0x4000) ){	// FLASH��RAM�ւ̃p�����[�^�f�[�^�]��
				Pon_log_work.Pon_kind |= 0x80;
			}
			Log_Write(eLOG_POWERON, &Pon_log_work, FALSE);									// �╜�d���O�o�^
			if( PrnJnlCheck() == ON ){
				//	�ެ���������ڑ�����̏ꍇ
				TeiFukJouPreqData.prn_kind = J_PRI;														// �o��������i�ެ��فj
				memcpy(&TeiFukJouPreqData.Ponlog.Pdw_Date, &clk_save, sizeof(date_time_rec));			// ��d�����o�^
				memcpy( &TeiFukJouPreqData.Ponlog.Pon_Date, &CLK_REC, sizeof( date_time_rec ) );		// ���d�����o�^
				TeiFukJouPreqData.Ponlog.Pon_nmi = (!FLAGPT.nmi_flg);									// nmi�׸ޓo�^
				TeiFukJouPreqData.Ponlog.Pon_kind = (unsigned char)(( StartingFunctionKey == 1 ) ? 3 :
																	( StartingFunctionKey == 2 ) ? 1 :
																	( StartingFunctionKey == 3 ) ? 2 :
																				StartingFunctionKey	);	/* nmi��ޓo�^ */
				// �������Ұ� FlashROM��SRAM���s��������set�iPon_kind��b7���g�p�F1=�s�����j
				if( 0 != (PowonSts_Param & 0x4000) || 0 != (PowonSts_LockParam & 0x4000) ){	// FLASH��RAM�ւ̃p�����[�^�f�[�^�]��
					TeiFukJouPreqData.Ponlog.Pon_kind |= 0x80;
				}
				queset( PRNTCBNO, PREQ_TEIFUK_JOU, sizeof(T_FrmTeiFukJou), &TeiFukJouPreqData );		// �󎚗v��ү���ޓo�^
			}
			break;
		case LOG_ERROR:	/* �װ��� */
			Err_work.ErrSeqNo = GetNtDataSeqNo();									// �V�[�P���V����No.�̐ݒ�
			Log_Write(eLOG_ERROR, &Err_work, TRUE);									// �G���[���O�o�^
			break;
		case LOG_ALARM:	/* �װя�� */
			Arm_work.ArmSeqNo = GetNtDataSeqNo();									// �V�[�P���V����No.�̐ݒ�
			Log_Write(eLOG_ALARM, &Arm_work, TRUE);									// �A���[�����O�o�^
			break;
		case LOG_OPERATE:	/* ������ */
			MonCtrl = _getOpeLogInfo(OPE_LOG_DAT.Ope_work.OpeKind, OPE_LOG_DAT.Ope_work.OpeCode);
			if (MonCtrl != NULL) {
				OPE_LOG_DAT.Ope_work.OpeLevel = MonCtrl->Level;
				// ���샍�O��1���󎚑O�Ɏ��̃f�[�^����������\�������邽�߈󎚃o�b�t�@�Ƃ��ėp�ӂ���K�v�L
				if( OPE_LOG_DAT.Ope_wtp < 0 || OPE_LOG_DAT.Ope_wtp >= OPE_LOG_CNT ){
					OPE_LOG_DAT.Ope_wtp = 0;
				}
				memcpy(&OPE_LOG_DAT.Ope_log_dat[OPE_LOG_DAT.Ope_wtp], &OPE_LOG_DAT.Ope_work, sizeof(Ope_log));
				if (MonCtrl->Operation & LOGOPERATION_RAM) {							// �R�[�h�A�g���r���[�g�`�F�b�N
					// RAM���M���O���s
					OPE_LOG_DAT.Ope_log_dat[OPE_LOG_DAT.Ope_wtp].OpeSeqNo = GetNtDataSeqNo();		// �V�[�P���V����No.�̐ݒ�
					Log_Write(eLOG_OPERATE, &OPE_LOG_DAT.Ope_log_dat[OPE_LOG_DAT.Ope_wtp], TRUE);	// ���샍�O�o�^
				}
				if (MonCtrl->Operation & LOGOPERATION_PRI) {							// �R�[�h�A�g���r���[�g�`�F�b�N
					if (MonCtrl->Level >= (uchar)prm_get(COM_PRM, S_PRN, 18, 1, 1)) {	// ���x���ݒ�`�F�b�N
						if (PrnJnlCheck() == ON) {										// �W���[�i���󎚎��s
							OpeJouPreqData.prn_kind = J_PRI;
							OpeJouPreqData.prn_data = &OPE_LOG_DAT.Ope_log_dat[OPE_LOG_DAT.Ope_wtp];
							queset(PRNTCBNO, PREQ_OPE_JOU, sizeof(T_FrmOpeJou), &OpeJouPreqData);
						}
					}
					// �ȉ��͈󎚗p�o�b�t�@�̍X�V
					OPE_LOG_DAT.Ope_wtp++;
					if (OPE_LOG_DAT.Ope_wtp >= OPE_LOG_CNT) {
						OPE_LOG_DAT.Ope_wtp = 0;
					}
				}
			}
			else {
				/* ����`�R�[�h */
			}
			break;
		case LOG_MONITOR:
			MonCtrl = _getMonLogInfo(MON_LOG_DAT.Mon_work.MonKind, MON_LOG_DAT.Mon_work.MonCode);
			if (MonCtrl != NULL) {
				MON_LOG_DAT.Mon_work.MonLevel = MonCtrl->Level;
				// ���j�^��1���󎚑O�Ɏ��̃f�[�^����������\�������邽�߈󎚃o�b�t�@�Ƃ��ėp�ӂ���K�v�L
				if( MON_LOG_DAT.Mon_wtp < 0 || MON_LOG_DAT.Mon_wtp >= MON_LOG_CNT ){
					MON_LOG_DAT.Mon_wtp = 0;
				}
				memcpy(&MON_LOG_DAT.Mon_log_dat[MON_LOG_DAT.Mon_wtp], &MON_LOG_DAT.Mon_work, sizeof(Mon_log));
				if (MonCtrl->Operation & LOGOPERATION_RAM) {							// �R�[�h�A�g���r���[�g�`�F�b�N
					// RAM���M���O���s
					MON_LOG_DAT.Mon_log_dat[MON_LOG_DAT.Mon_wtp].MonSeqNo = GetNtDataSeqNo();		// �V�[�P���V����No.�̐ݒ�
					Log_Write(eLOG_MONITOR, &MON_LOG_DAT.Mon_log_dat[MON_LOG_DAT.Mon_wtp], TRUE);	// ���j�^���O�o�^
				}
				if (MonCtrl->Operation & LOGOPERATION_PRI) {							// �R�[�h�A�g���r���[�g�`�F�b�N
					if(MON_LOG_DAT.Mon_work.MonKind == 1 &&
					  (MON_LOG_DAT.Mon_work.MonCode >= 85 && MON_LOG_DAT.Mon_work.MonCode <= 87) &&
					  0 == prm_get(COM_PRM, S_CRE, 66, 1, 1)) {
					  	// �N���W�b�g�֘A���j�^��38-0066=0�Ȃ�󎚂��Ȃ�
					  	jnl_pri = OFF;
					}
					else {
					  	jnl_pri = ON;
					}
					if (MonCtrl->Level >= (uchar)prm_get(COM_PRM, S_PRN, 18, 1, 2) && jnl_pri == ON) {	// ���x���ݒ�`�F�b�N
						if (PrnJnlCheck() == ON) {										// �W���[�i���󎚎��s
							MonJouPreqData.prn_kind = J_PRI;
							MonJouPreqData.prn_data = &MON_LOG_DAT.Mon_log_dat[MON_LOG_DAT.Mon_wtp];
							queset(PRNTCBNO, PREQ_MON_JOU, sizeof(T_FrmMonJou), &MonJouPreqData);
						}
					}
					MON_LOG_DAT.Mon_wtp++;
					if (MON_LOG_DAT.Mon_wtp >= MON_LOG_CNT) {
						MON_LOG_DAT.Mon_wtp = 0;
					}
				}
			}
			else {
				/* ����`�R�[�h */
			}
			break;
		case LOG_CREUSE:	/* �ڼޯė��p���� */
			memcpy( &meisai_work.PayTime, &PayData.TOutTime, sizeof(meisai_work.PayTime));					// ���Z����
			memcpy( &meisai_work.card_name, &PayData.credit.card_name, sizeof(meisai_work.card_name));		// �J�[�h��
			memcpy( &meisai_work.card_no, &PayData.credit.card_no, sizeof(meisai_work.card_no));			// �J�[�h�ԍ�
			meisai_work.slip_no = PayData.credit.slip_no;													// �`�[�ԍ�
			memcpy(meisai_work.AppNoChar, PayData.credit.AppNoChar, sizeof(meisai_work.AppNoChar));			// ���F�ԍ�
			memcpy(meisai_work.shop_account_no, PayData.credit.ShopAccountNo,
														sizeof(meisai_work.shop_account_no));				// �ڼޯĶ��މ����X����ԍ�
			meisai_work.pay_ryo = PayData.credit.pay_ryo;													// ���p���z

			if(PayData.credit.cre_type == CREDIT_CARD) {													// �N���W�b�g�J�[�h���p
				Log_Write(eLOG_CREUSE, &meisai_work, TRUE);													// �N���W�b�g���p���׃��O�o�^
			}
			else if(PayData.credit.cre_type == CREDIT_HOJIN) {												// �@�l�J�[�h���p
				Log_Write(eLOG_HOJIN_USE, &meisai_work, TRUE);												// �@�l�J�[�h���p���׃��O�o�^
			}
			break;
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//		case	LOG_EDYARM:			// Edy�A���[��������O
//			Edy_Arm_log_work.Kikai_no = (uchar)CPrmSS[S_PAY][2];
//			Edy_Arm_log_work.TD_Kikai_no = (uchar)PayData.Kikai_no;
//			if( !OPECTL.multi_lk )									// ������Z�⍇�����ȊO?
//				SyashitsuNum = OPECTL.Pr_LokNo;						// ���b�N���uNo
//			else
//				SyashitsuNum = (ushort)(LOCKMULTI.LockNo%10000);	// ���b�N���uNo
//
//			Edy_Arm_log_work.TicketNum = 
//			(ushort)(( LockInfo[(SyashitsuNum-1)].area * 10000L ) + LockInfo[(SyashitsuNum-1)].posi );
//
//			Edy_Arm_log_work.syubet = ryo_buf.tik_syu;
//			Edy_Arm_log_work.teiki = PayData.teiki;
//			memcpy( &Edy_Arm_log_work.TOutTime, &CLK_REC, sizeof( date_time_rec ));
//			memcpy( Edy_Arm_log_work.Edy_CardNo,
//					Edy_Settlement_Res.Edy_CardNo,
//					sizeof( Edy_Settlement_Res.Edy_CardNo ));
//			Edy_Arm_log_work.Pay_Ryo = Edy_Settlement_Res.Pay_Ryo;
//			Edy_Arm_log_work.Pay_Befor = Edy_Settlement_Res.Pay_Befor;
//			Edy_Arm_log_work.Pay_After = Edy_Settlement_Res.Pay_After;
//			Edy_Arm_log_work.Edy_Deal_num = Edy_Settlement_Res.Edy_Deal_num;
//			Edy_Arm_log_work.Card_Deal_num = Edy_Settlement_Res.Card_Deal_num;
//			memcpy( Edy_Arm_log_work.High_Terminal_ID,
//					Edy_Settlement_Res.High_Terminal_ID,
//					sizeof( Edy_Settlement_Res.High_Terminal_ID ));
//
//			Log_Write(eLOG_EDYARM, &Edy_Arm_log_work, FALSE);
//			break;
//
//		case	LOG_EDYSHIME:			// Edy���ߋL�^���O
//			Log_Write(eLOG_EDYSHIME, &EDY_SHIME_LOG_NOW, FALSE);
//			break;
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
// MH810100(S) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//		case	LOG_ENTER:				// ���Ƀ��O
//			Enter_data.CMN_DT.DT_54.SeqNo = GetNtDataSeqNo();							// �V�[�P���V����No.�̐ݒ�
//			Log_Write(eLOG_ENTER, &Enter_data, TRUE);
//			break;
		case	LOG_RTPAY:				// RT���Z�f�[�^���O
//			Enter_data.CMN_DT.DT_54.SeqNo = GetNtDataSeqNo();							// �V�[�P���V����No.�̐ݒ�
			Log_Write(eLOG_RTPAY, &RTPay_Data, TRUE);
			break;
// MH810100(E) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
		case	LOG_RTRECEIPT:			// RT�̎��؃f�[�^���O
			Log_Write(eLOG_RTRECEIPT, &RTReceipt_Data, TRUE);
			break;
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
		case	LOG_PARKING:			// ���ԑ䐔�f�[�^
			ParkCar_data.CMN_DT.DT_58.SeqNo = GetNtDataSeqNo();							// �V�[�P���V����No.�̐ݒ�
			Log_Write(eLOG_PARKING, &ParkCar_data, TRUE);
			break;
		case	LOG_LONGPARK:			// �������ԃf�[�^
// MH322917(S) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
//			Log_Write(eLOG_LONGPARK, &LongParking_data, TRUE);
			Log_Write(eLOG_LONGPARK_PWEB, &LongParking_data_Pweb, TRUE);
// MH322917(E) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
			break;
		case	LOG_MONEYMANAGE_NT:		// ���K�Ǘ��f�[�^(NT-NET)
			// NT-NET�ł͍ŐV�̂P���݂̂�o�^����ׁA�o�^�ς݃f�[�^����ǂ݂���
			logCount = Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);
			if(logCount > 0){
				for(i=0; i<logCount; i++){
					Ope_Log_TargetDataGet( eLOG_MONEYMANAGE, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
				}
			}
			turi_kan.SeqNo = GetNtDataSeqNo();											// �V�[�P���V����No.�̐ݒ�
			Log_Write(eLOG_MONEYMANAGE, &turi_kan, TRUE);								// ���K�Ǘ����O�o�^
			break;
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//		case	LOG_RISMEVENT:			// �������ԃf�[�^
//			Log_Write(eLOG_RISMEVENT, &RismEvent_data, TRUE);
//			break;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
		case LOG_REMOTE_MONITOR:
			// ���u�Ď���1���󎚑O�Ɏ��̃f�[�^����������\�������邽�߈󎚃o�b�t�@�Ƃ��ėp�ӂ���K�v�L
			if( RMON_LOG_DAT.Rmon_wtp < 0 || RMON_LOG_DAT.Rmon_wtp >= RMON_LOG_CNT ){
				RMON_LOG_DAT.Rmon_wtp = 0;
			}
			memcpy(&RMON_LOG_DAT.Rmon_log_dat[RMON_LOG_DAT.Rmon_wtp], &RMON_LOG_DAT.Rmon_work, sizeof(Rmon_log));
			RMON_LOG_DAT.Rmon_log_dat[RMON_LOG_DAT.Rmon_wtp].RmonSeqNo = GetNtDataSeqNo();	// �V�[�P���V����No.�̐ݒ�
			Log_Write(eLOG_REMOTE_MONITOR, &RMON_LOG_DAT.Rmon_log_dat[RMON_LOG_DAT.Rmon_wtp], TRUE);	// ���u�Ď����O�o�^

			if (PrnJnlCheck() == ON) {														// �W���[�i���󎚎��s
				RmonJouPreqData.prn_kind = J_PRI;
				RmonJouPreqData.prn_data = &RMON_LOG_DAT.Rmon_log_dat[RMON_LOG_DAT.Rmon_wtp];
				queset(PRNTCBNO, PREQ_RMON_JOU, sizeof(T_FrmRmonJou), &RmonJouPreqData);
			}
			RMON_LOG_DAT.Rmon_wtp++;
			if (RMON_LOG_DAT.Rmon_wtp >= RMON_LOG_CNT) {
				RMON_LOG_DAT.Rmon_wtp = 0;
			}
			break;
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
		// QR�m��E����f�[�^���O
		case LOG_DC_QR:
			Log_Write( eLOG_DC_QR, &DC_QR_work, TRUE );
			break;
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
		// ���[�����j�^�f�[�^���O
		case LOG_DC_LANE:
			Log_Write( eLOG_DC_LANE, &DC_LANE_work, TRUE );
			break;
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
		default:
			break;
	}

	return;
}


/*[]----------------------------------------------------------------------[]*/
/*| �s���E�����o�ɏ��f�[�^�쐬                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Log_fusei_mk( void )                                    |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.HARA                                                  |*/
/*| Date         : 2001-12-20                                              |*/
/*| UpDate       : 2004-06-18 T.Nakayama                                   |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
static void Log_fusei_mk( void )
{
	ushort	num;

	logwork.flp.Kikai_no = (uchar)CPrmSS[S_PAY][2];		// �@�BNo.

	logwork.flp.In_Time.Year = car_in_f.year;				/* ���ɔN�������� 	*/
	logwork.flp.In_Time.Mon  = car_in_f.mon;
	logwork.flp.In_Time.Day  = car_in_f.day;
	logwork.flp.In_Time.Hour = car_in_f.hour;
	logwork.flp.In_Time.Min  = car_in_f.min;
	logwork.flp.Date_Time.Year = car_ot_f.year;				/* ���Z�N�������� 	*/
	logwork.flp.Date_Time.Mon  = car_ot_f.mon;
	logwork.flp.Date_Time.Day  = car_ot_f.day;
	logwork.flp.Date_Time.Hour = car_ot_f.hour;
	logwork.flp.Date_Time.Min  = car_ot_f.min;

	num = ryo_buf.pkiti - 1;
	logwork.flp.WPlace = (ulong)(( LockInfo[num].area * 10000L ) + LockInfo[num].posi );	/* ���Ԉʒu 		*/

	logwork.flp.Lok_No = num +1;

	if( fusei.fus_d[0].kyousei == 1 || fusei.fus_d[0].kyousei == 11 ){
		CountGet( KIYOUSEI_COUNT, &logwork.flp.count );
		logwork.flp.Lok_inf = 0;							/* ���� 			*/
		PayData_set_SK( ryo_buf.pkiti, 0, 0, 1, 0 );		// �s���E�����o�ɗp���Z��񐶐�
		Log_Write(eLOG_PAYMENT, &PayData, TRUE);
	}
	else{					// 0�F�ʏ�s���A2�F�C���s��(��������)�A3�F�C���s��(�����Ȃ�)
		CountGet( FUSEI_COUNT, &logwork.flp.count );
		logwork.flp.Lok_inf = 1;							/* �s�� 			*/
		if( fusei.fus_d[0].kyousei != 0 ){
			logwork.flp.Lok_inf = 2;						/* �C�����Z�ɂ��s�� */
		}
		PayData_set_SK( ryo_buf.pkiti, 0, 0, 3, fusei.fus_d[0].kyousei );			// �s���E�����o�ɗp���Z��񐶐�
		Log_Write(eLOG_PAYMENT, &PayData, TRUE);
	}

	if( fusei.fus_d[0].kyousei == 3 ){
		logwork.flp.fusei_fee = 0;							/* ���ԗ��� 		*/
	}else{
		logwork.flp.fusei_fee = ryo_buf.dsp_ryo;			/* ���ԗ��� 		*/
	}
	logwork.flp.ryo_syu = ryo_buf.tik_syu;					/* ������� 		*/

	return;
}


/*[]----------------------------------------------------------------------[]*/
/*| Log�ر������                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : log_init                                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.Hashimoto                                             |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void	log_init( void )
{
	short i, j;

	if( read_rotsw() == 4 ){	// ----- �X�[�p�[�C�j�V�����C�Y(RSW=4)? -----
		memset(LOG_DAT, 0, sizeof(struct log_record)*eLOG_MAX);
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
//		memset( &IO_LOG_REC, 0, sizeof( struct Io_log_rec ) );
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
		for (i = 0; i < eLOG_MAX; i++) {
			for (j = 0; j < eLOG_TARGET_MAX; j++) {
				LOG_DAT[i].void_read[j] = 0;
				LOG_DAT[i].nearFull[j].NearFullMinimum = 0;
				LOG_DAT[i].nearFull[j].NearFullStatus = RAU_NEARFULL_CANCEL;
				LOG_DAT[i].nearFull[j].NearFullStatusBefore = RAU_NEARFULL_CANCEL;
			}
		}
	}
	memset( &SUICA_LOG_REC, 0, sizeof( struct suica_log_rec ) );
	memset( &SUICA_LOG_REC_FOR_ERR, 0, sizeof( struct	suica_log_rec ) );
	memset( &suica_rcv_que, 0, sizeof( t_SUICA_RCV_QUE ));
	memset(&OPE_LOG_DAT, 0, sizeof(OPE_LOG_DAT));
	memset(&MON_LOG_DAT, 0, sizeof(MON_LOG_DAT));
	memset(&FLP_LOG_DAT, 0, sizeof(FLP_LOG_DAT));
	memset(&Syuukei_sp, 0, sizeof(Syuukei_sp));
	memset(&RMON_LOG_DAT, 0, sizeof(RMON_LOG_DAT));
	memset(&fan_exe_time, 0, sizeof(fan_exe_time));

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief		LOG�f�[�^�Ǘ���ʎ擾
//[]----------------------------------------------------------------------[]
///	@param[in]	Lno		: LOG���
///	@return		strage	: LOG�f�[�^�Ǘ����(FALSE(=0):SRAM,TRUE(=1):SRAM+FlashROM)
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
BOOL Ope_Log_ManageKindGet(short Lno)
{
	// �t���b�V���ɕۑ�����Ƃ�TRUE
	// �t���b�V���^�X�N�ɏ������˗��ł��郍�O��LOG_STRAGEN�Ȃ̂ł�����g�p����B
	if (Lno < LOG_STRAGEN) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		LOG�o�^�����擾
//[]----------------------------------------------------------------------[]
///	@param[in]	Lno		: LOG���
///	@return		ushort	: �o�^��
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort Ope_Log_TotalCountGet( short id )
{
	long				num;
	struct log_record	*lp = LOG_DAT+id;

	if (Ope_Log_ManageKindGet(id) == FALSE) {
	// SRAM�̂�
		return (ushort)lp->count[0];
	}
	else {
	// �t���b�V������
		while(FLT_GetLogRecordNum(id, &num) != FLT_NORMAL) {
			taskchg( IDLETSKNO );
		}
		if (LogDatMax[id][0] <= RECODE_SIZE) {
		// SRAM�ꎞ�ۑ��f�[�^����
			num += lp->count[lp->kind];
		}
		return (ushort)num;
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		����LOG�o�^�����擾
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG���
///	@param[in]	target	: �ǂݏo�����
///	@return		ushort	: ����LOG�o�^����
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort Ope_Log_UnreadCountGet( short id, short target )
{
	struct log_record	*lp = LOG_DAT+id;
	ushort	num = 0;

	if( !Ope_Log_ManageKindGet( id ) ){		// SRAM�̂�
		//  RAM�݂̂Ƀf�[�^������LOG�̏ꍇ�Aunread��RAM�G���A�̐擪(s2[RECODE_SIZE2])����ǂ̈ʒu��ǂނ�������
		num = lp->f_unread[target];			// ���ǃf�[�^�L
	} else {// FROM/SRAM����
		//  FROM/RAM�̗����Ƀf�[�^������LOG�̏ꍇ�Aunread�͑������ɑ΂��ĉ������[�h���Ă��邩������
		if( Ope_Log_TotalCountGet(id)>lp->unread[target] ){
			num = Ope_Log_TotalCountGet(id) - lp->unread[target];
		}
	}
	return num;
}

//[]----------------------------------------------------------------------[]
///	@brief		����LOG��ǎ�ςƂ���
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG���
///	@param[in]	target	: �ǂݏo�����
///	@return		ushort	: ����LOG�o�^����
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort Ope_Log_UnreadToRead( short id, short target )
{
	struct log_record	*lp = LOG_DAT+id;

	if( !Ope_Log_ManageKindGet( id ) ){		// SRAM�̂�
		//  RAM�݂̂Ƀf�[�^������LOG�̏ꍇ�Aunread��RAM�G���A�̐擪(s2[RECODE_SIZE2])����ǂ̈ʒu��ǂނ�������
		lp->unread[target] += lp->f_unread[target];
		lp->f_unread[target] = 0;
	} else {// FROM/SRAM����
		//  FROM/RAM�̗����Ƀf�[�^������LOG�̏ꍇ�Aunread�͑������ɑ΂��ĉ������[�h���Ă��邩������
		lp->unread[target] = Ope_Log_TotalCountGet(id);
	}
	lp->void_read[target] = 0;
	lp->nearFull[target].NearFullStatus = RAU_NEARFULL_CANCEL;
	lp->nearFull[target].NearFullStatusBefore = RAU_NEARFULL_CANCEL;
	Log_CheckBufferFull(FALSE, id, target);
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief		�j�A�t���`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG���
///	@param[in]	target	: �ǂݏo�����
///	@return		ushort	: �j�A�t�����(0:�ʏ� 1:�j�A�t��)
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort	Ope_Log_CheckNearFull( short id, short target )
{
	struct log_record	*lp = LOG_DAT+id;
	ushort ret = 0;

	if( lp->nearFull[target].NearFullStatus == RAU_NEARFULL_NEARFULL ){
		if( lp->nearFull[target].NearFullStatusBefore != lp->nearFull[target].NearFullStatus ){
			// ���ݒl��O��l�ɏ������݁A���d�������Ȃ����
			lp->nearFull[target].NearFullStatusBefore = lp->nearFull[target].NearFullStatus;
			ret = 1;
		}
	}

	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief		�j�A�t����Ԏ擾
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG���
///	@param[in]	target	: �ǂݏo�����
///	@return		ushort	: �j�A�t�����(0:�ʏ� 1:�j�A�t��)
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort	Ope_Log_GetNearFull( short id, short target )
{
	struct log_record	*lp = LOG_DAT+id;
	
	return lp->nearFull[target].NearFullStatus;
}
//[]----------------------------------------------------------------------[]
///	@brief		LOG�i�[�����擾
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG���
///	@return		ushort	: LOG�i�[�\����(FROM+SRAM)
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort	Ope_Log_GetLogMaxCount( short id )
{
	ushort	sectorCount = 0;				// 1�Z�N�^���̃��O����
	ushort	maxCount = 0;					// ���O�ő匏��
	
	sectorCount = LOG_SECORNUM(id);			// SRAM�̍ő働�O����
	
	switch(id) {
	case eLOG_PAYMENT:						// ���Z
		maxCount += sectorCount + (FLT_LOG0_SECTORMAX * sectorCount);
		break;
// MH810100(S) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//	case eLOG_ENTER:						// ����
	case eLOG_RTPAY:						// RT���Z�f�[�^
// MH810100(E) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
		maxCount += sectorCount + (FLT_LOG1_SECTORMAX * sectorCount);
		break;
	case eLOG_TTOTAL:						// �W�v
		maxCount += sectorCount + (FLT_LOG2_SECTORMAX * sectorCount);
		break;
// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
//	case eLOG_LCKTTL:						// �Ԏ����W�v
	case eLOG_RTRECEIPT:					// RT�̎��؃f�[�^
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
		maxCount += sectorCount + (FLT_LOG3_SECTORMAX * sectorCount);
		break;
	case eLOG_ERROR:						// �G���[
		maxCount += sectorCount + (FLT_LOG4_SECTORMAX * sectorCount);
		break;
	case eLOG_ALARM:						// �A���[��
		maxCount += sectorCount + (FLT_LOG5_SECTORMAX * sectorCount);
		break;
	case eLOG_OPERATE:						// ����
		maxCount += sectorCount + (FLT_LOG6_SECTORMAX * sectorCount);
		break;
	case eLOG_MONITOR:						// ���j�^
		maxCount += sectorCount + (FLT_LOG7_SECTORMAX * sectorCount);
		break;
	case eLOG_ABNORMAL:						// �s�������o��
		maxCount += sectorCount + (FLT_LOG8_SECTORMAX * sectorCount);
		break;
	case eLOG_MONEYMANAGE:					// �ޑK�Ǘ�
		maxCount += sectorCount + (FLT_LOG9_SECTORMAX * sectorCount);
		break;
	case eLOG_PARKING:						// ���ԑ䐔�f�[�^
		maxCount += sectorCount + (FLT_LOG10_SECTORMAX * sectorCount);
		break;
	case eLOG_NGLOG:						// �s�����
		maxCount += sectorCount + (FLT_LOG11_SECTORMAX * sectorCount);
		break;
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
//	case eLOG_IOLOG:						// ���o�ɗ���
	case eLOG_DC_QR:						// QR�m��E����f�[�^
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(QR�m��E����f�[�^�Ή�)
		maxCount += sectorCount + (FLT_LOG12_SECTORMAX * sectorCount);
		break;
	case eLOG_CREUSE:						// �N���W�b�g���p
		maxCount += sectorCount + (FLT_LOG13_SECTORMAX * sectorCount);
		break;
	case eLOG_iDUSE:						// ID���p
		maxCount += sectorCount + (FLT_LOG14_SECTORMAX * sectorCount);
		break;
	case eLOG_HOJIN_USE:					// �@�l�J�[�h���p
		maxCount += sectorCount + (FLT_LOG15_SECTORMAX * sectorCount);
		break;
	case eLOG_REMOTE_SET:					// ���u�����ݒ�
		maxCount += sectorCount + (FLT_LOG16_SECTORMAX * sectorCount);
		break;
// MH322917(S) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
//	case eLOG_LONGPARK:						// ��������
	case eLOG_LONGPARK_PWEB:				// ��������(ParkingWeb�p)
// MH322917(E) A.Iiizumi 2018/08/31 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
		maxCount += sectorCount + (FLT_LOG17_SECTORMAX * sectorCount);
		break;
// GG129000(S) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
//	case eLOG_RISMEVENT:					// RISM�C�x���g
	case eLOG_DC_LANE:						// ���[�����j�^�f�[�^
// GG129000(E) H.Fujinaga 2023/01/19 �Q�[�g���Ԕԃ`�P�b�g���X�V�X�e���Ή��i���[�����j�^�f�[�^�Ή��j
		maxCount += sectorCount + (FLT_LOG18_SECTORMAX * sectorCount);
		break;
	case eLOG_GTTOTAL:						// GT�W�v
		maxCount += sectorCount + (FLT_LOG19_SECTORMAX * sectorCount);
		break;
	case eLOG_REMOTE_MONITOR:
		maxCount += sectorCount + (FLT_LOG20_SECTORMAX * sectorCount);
		break;
	case eLOG_COINBOX:						// �R�C�����ɏW�v(ram�̂�)
	case eLOG_NOTEBOX:						// �������ɏW�v(ram�̂�)
// MH321800(S) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
//	case eLOG_EDYARM:						// Edy�A���[��(ram�̂�)
//	case eLOG_EDYSHIME:						// Edy����(ram�̂�)
// MH321800(E) Y.Tanizaki IC�N���W�b�g�Ή� �s�v�@�\�폜(Edy)
	case eLOG_POWERON:						// ���d(ram�̂�)
	case eLOG_MNYMNG_SRAM:					// ���K�Ǘ�(SRAM)
		maxCount = sectorCount;
		break;
	default:
		break;
	}
	
	return maxCount;
}

//[]----------------------------------------------------------------------[]
///	@brief		LOG�j�A�t���f�[�^�����擾
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG���
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort	Ope_Log_GetNearFullCount( short id )
{
	ushort	cen_48_456;
	ushort	nearFullCount;

	cen_48_456 = prm_get(COM_PRM,S_NTN, 68, 3, 1);
	if (cen_48_456 > 100) {
		cen_48_456 = 100;
	}
	if (cen_48_456 <= 0) {
		cen_48_456 = 1;
	}
	nearFullCount = Ope_Log_GetLogMaxCount(id) * cen_48_456 / 100;
	if( (Ope_Log_GetLogMaxCount(id) * cen_48_456) % 100 ) {
		nearFullCount++;
	}
	if (nearFullCount <= 0) {
		nearFullCount = 1;
	}
	return nearFullCount;
}
//[]----------------------------------------------------------------------[]
///	@brief		LOG�f�[�^�擾
//[]----------------------------------------------------------------------[]
///	@param[in]	Lno		: LOG���
///	@param[in]	Index	: LOG�C���f�b�N�X�ԍ�
///	@param[out]	pSetBuf	: �f�[�^�i�[�o�b�t�@
///	@return		uchar	: 1=����A0=�G���[
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]

// ���O�����ݑO��Ō������ς��̂ŁA��A�̃��O���Q�Ƃ���ꍇ�́A���̊ԂɃ��O�������݂�
// �N����Ȃ��悤���ӂ��Ă��������B

uchar Ope_Log_1DataGet( short id, ushort Index, void *pSetBuf )
{
	long				num, maximum;
	uchar				*dp;
	struct log_record	*lp = LOG_DAT+id;

	if (Ope_Log_ManageKindGet(id) == FALSE) {
	// SRAM�݂̂̏ꍇ�́As2��Ń����O�o�b�t�@�ɂȂ�
		maximum = lp->count[0];
		dp = lp->dat.s2;
		if( Index>=maximum ){	// Index error
			return 0;
		}
		memcpy(pSetBuf, dp+Index*LogDatMax[id][0], LogDatMax[id][0]);
		return 1;
	}

// �t���b�V���������猏�����擾�i�������ݒ��Ȃ�҂����킹��j
	while( FLT_GetLogRecordNum(id, &num) != FLT_NORMAL ){
		taskchg( IDLETSKNO );
	}

	if( Index<num ){				// FlashROM�����ް��v��
		if( FLT_TargetLogRead(id, Index, pSetBuf)!=FLT_NORMAL ){
			return 0;
		}
	}
	else{							// SRAM�����ް��v��
		Index -= (ushort)num;
		if (LogDatMax[id][0] > RECODE_SIZE) {
		// SRAM���P�ʂ��������ꍇ�i�W�v�f�[�^�A�Ԏ����W�v�f�[�^�j�ASRAM�ɕۑ��ł��錏���͂P�ɂȂ�B
		// �ۑ��シ���Ƀt���b�V���ɏ������܂ꊮ���҂��܂ōs������SRAM�f�[�^�͎����㑶�݂��Ȃ��B
			return 0;
		}

	// SRAM�ɂQ�ʂ���ꍇ�A�^�p�ʑ����L���ŁA�ҋ@�ʕ��̓t���b�V���ɏ������݌�j������đ��݂��Ȃ�
		maximum = lp->count[lp->kind];
		dp = lp->dat.s1[lp->kind];
		if( Index>=maximum ){	// Index error
			return 0;
		}
		memcpy(pSetBuf, dp+Index*LogDatMax[id][0], LogDatMax[id][0]);
	}
	return 1;
}

//[]----------------------------------------------------------------------[]
///	@brief		LOG�f�[�^�擾�i�ǂݏo����ʁj
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG���
///	@param[out]	pSetBuf	: �f�[�^�i�[�o�b�t�@
///	@param[in]	Index	: �ǂݏo�����
///	@param[in]	Index	: ���[�h�|�C���^���Z�t���O
///	@return		uchar	: 1=����A0=�G���[
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar Ope_Log_TargetDataGet( short id, void *pSetBuf, short target, BOOL update )
{
	uchar	ret = 0;
	struct log_record	*lp = LOG_DAT+id;
	ushort	max_idx;

	if( !Ope_Log_ManageKindGet( id ) ){// SRAM�̂�
		//  RAM�݂̂Ƀf�[�^������LOG�̏ꍇ�Aunread��RAM�G���A�̐擪(s2[RECODE_SIZE2])����ǂ̈ʒu��ǂނ�������
		if(lp->f_unread[target] > 0){// ���ǃf�[�^�L
			ret = Ope_Log_1DataGet(id, lp->unread[target], pSetBuf);
			if( ret==1 && update ){
				lp->unread[target]++;
				lp->f_unread[target]--;// ���ǃf�[�^�����Z
				if(lp->f_unread[target] <= lp->nearFull[target].NearFullMinimum){
					// �j�A�t������
					lp->nearFull[target].NearFullStatus = RAU_NEARFULL_CANCEL;
					lp->nearFull[target].NearFullStatusBefore = RAU_NEARFULL_CANCEL;
				}
				max_idx = LOG_SECORNUM( id );						// �ő働�O���擾
				if( lp->unread[target] >= max_idx ){				// ���ǃ��R�[�h�ԍ����ő働�O���𒴂����H
					lp->unread[target] = 0;							// ���ǃ��R�[�h�ԍ����O�ɖ߂�
				}
			}
		}
	} else {// FROM/SRAM����
		//  FROM/RAM�̗����Ƀf�[�^������LOG�̏ꍇ�Aunread�͑������ɑ΂��ĉ������[�h���Ă��邩������
		if( Ope_Log_TotalCountGet(id)>lp->unread[target] ){
			ret = Ope_Log_1DataGet(id, lp->unread[target], pSetBuf);
			if( ret==1 && update ){
				lp->unread[target]++;
				if((Ope_Log_TotalCountGet(id) - lp->f_unread[target]) <= lp->nearFull[target].NearFullMinimum){
					// �j�A�t������
					lp->nearFull[target].NearFullStatus = RAU_NEARFULL_CANCEL;
					lp->nearFull[target].NearFullStatusBefore = RAU_NEARFULL_CANCEL;
				}
			}
		}
	}
	return ret;
}

// Ope_Log_TargetDataGet()�ł͂P�񕪂̋�ǂ݂݂̂ŕ����񐔕��̋�ǂ�
// ���s���ׂɒǉ�
//[]----------------------------------------------------------------------[]
///	@brief		LOG�f�[�^��ǂ݁i�ǂݏo����ʁj
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG���
///	@param[out]	pSetBuf	: �f�[�^�i�[�o�b�t�@
///	@param[in]	Index	: �ǂݏo�����
///	@param[in]	Index	: ��ǂ݊J�n�t���O(TRUE:�����ǂ� FALSE:����ȍ~��ǂ�)
///	@return		uchar	: 0:�G���[ 1=����
//[]----------------------------------------------------------------------[]
///	@author		y.iiduka
///	@date		Create	: 2012/02/07<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
uchar Ope_Log_TargetDataVoidRead( short id, void *pSetBuf, short target, BOOL start )
{
	uchar	ret = 0;
	ushort	voidReadIndex;
	struct log_record	*lp = LOG_DAT+id;

	if(start == TRUE){
		lp->void_read[target] = 0;
		// ��ǂ݊J�n�ȍ~�ɖ����M���㏑�����ꂽ�����ΏۂƂȂ�̂ŁA�J�n���ɃN���A����
		lp->overWriteUnreadCount[target] = 0;
	}

	if( !Ope_Log_ManageKindGet( id ) ){// SRAM�̂�
		//  RAM�݂̂Ƀf�[�^������LOG�̏ꍇ�Aunread��RAM�G���A�̐擪(s2[RECODE_SIZE2])����ǂ̈ʒu��ǂނ�������
		if(lp->f_unread[target] > 0){// ���ǃf�[�^�L
			voidReadIndex = lp->unread[target] + lp->void_read[target];
			if(voidReadIndex >= LOG_SECORNUM(id)) {
				voidReadIndex -= (LOG_SECORNUM(id));
			}
			ret = Ope_Log_1DataGet(id, voidReadIndex, pSetBuf);
		}
	} else {// FROM/SRAM����
		//  FROM/RAM�̗����Ƀf�[�^������LOG�̏ꍇ�Aunread�͑������ɑ΂��ĉ������[�h���Ă��邩������
		if( Ope_Log_TotalCountGet(id) > (lp->unread[target] + lp->void_read[target]) ){
			ret = Ope_Log_1DataGet(id, lp->unread[target] + lp->void_read[target], pSetBuf);
		}
	}
	if(ret == 1){
		lp->void_read[target]++;
	}
	return ret;
}
//[]----------------------------------------------------------------------[]
///	@brief		LOG�f�[�^��ǂ݂����ɖ߂�
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG���
///	@param[in]	Index	: �ǂݏo�����
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void Ope_Log_UndoTargetDataVoidRead( short id, short target )
{
	struct log_record	*lp = LOG_DAT+id;
	
	// ��ǂݏ�Ԃł����1�߂�
	if(lp->void_read[target]) {
		--lp->void_read[target];
	}
}

// GG129004(S) R.Endo 2024/11/19 �d�q�̎��ؑΉ�
// //[]----------------------------------------------------------------------[]
// ///	@brief		�W�v�f�[�^LOG�̒ǔԂƈ�v����Ԏ����W�v�f�[�^LOG����������
// //[]----------------------------------------------------------------------[]
// ///	@param[in]	syukei	: �W�v�f�[�^LOG�|�C���^
// ///	@param[in]	lckttl	: �Ԏ����W�v�f�[�^LOG�|�C���^
// ///	@return		uchar	: 1=����OK�A0=����NG
// //[]----------------------------------------------------------------------[]
// ///	@author		y.iiduka
// ///	@date		Create	: 2012/02/07<br>
// ///				Update	: 
// //[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
// uchar Ope_Log_ShashitsuLogGet( SYUKEI *syukei, LCKTTL_LOG *lckttl )
// {
// 	ushort	cnt_ttl;
// 	ushort	i;
// 	uchar	ret = 0;
// 
// 	cnt_ttl = Ope_Log_TotalCountGet( eLOG_LCKTTL );			// �Ԏ����W�v�f�[�^LOG�����擾
// 	for( i = 0; i < cnt_ttl; i++ ){							// �Ԏ����W�v�f�[�^LOG����������
// 		if( Ope_Log_1DataGet( eLOG_LCKTTL, i, lckttl ) ){	// �Ԏ����W�v�f�[�^LOG�P���擾
// 			if(( lckttl->Oiban.i == syukei->Oiban.i ) &&	// �ǔԁ{����W�v��������v�H
// 			   ( lckttl->Oiban.w == syukei->Oiban.w ) &&
// 			   ( memcmp( &lckttl->NowTime, &syukei->NowTime, sizeof( date_time_rec ) ) == 0 )){
// 				ret = 1;									// ����OK
// 				break;
// 			}
// 		}
// 	}
// 
// 	return ret;
// }
// GG129004(E) R.Endo 2024/11/19 �d�q�̎��ؑΉ�

//[]----------------------------------------------------------------------[]
///	@brief		LOG���[�h�|�C���^�X�V�i�ǂݏo����ʁj
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG���
///	@param[in]	Index	: �ǂݏo�����
///	@return
//[]----------------------------------------------------------------------[]
///	@author		S.Takahashi
///	@date		Create	: 2012/05/02<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Ope_Log_TargetReadPointerUpdate(short id, short target)
{
	struct log_record	*lp = LOG_DAT+id;
	ushort	max_idx;

	if( !Ope_Log_ManageKindGet( id ) ){// SRAM�̂�
		//  RAM�݂̂Ƀf�[�^������LOG�̏ꍇ�Aunread��RAM�G���A�̐擪(s2[RECODE_SIZE2])����ǂ̈ʒu��ǂނ�������
		if(lp->f_unread[target] > 0){// ���ǃf�[�^�L
			lp->unread[target]++;
			lp->f_unread[target]--;// ���ǃf�[�^�����Z
			max_idx = LOG_SECORNUM( id );						// �ő働�O���擾
			if( lp->unread[target] >= max_idx ){				// ���ǃ��R�[�h�ԍ����ő働�O���𒴂����H
				lp->unread[target] = 0;							// ���ǃ��R�[�h�ԍ����O�ɖ߂�
			}
		}
	} else {// FROM/SRAM����
		//  FROM/SRAM�̗����Ƀf�[�^������LOG�̏ꍇ�Aunread�͑������ɑ΂��ĉ������[�h���Ă��邩������
		if( Ope_Log_TotalCountGet(id)>lp->unread[target] ){
			lp->unread[target]++;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		��ǂ�LOG�̃��[�h�|�C���^�X�V�i�ǂݏo����ʁj
//[]----------------------------------------------------------------------[]
///	@param[in]	id		: LOG���
///	@param[in]	Index	: �ǂݏo�����
///	@return		��ǂ݂��烊�[�h�|�C���^�X�V�܂łɍ폜���ꂽ�����M�f�[�^��
//[]----------------------------------------------------------------------[]
///	@author		S.Takahashi
///	@date		Create	: 2012/05/02<br>
///				Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
ushort	Ope_Log_TargetVoidReadPointerUpdate(short id, short target)
{
	struct log_record	*lp = LOG_DAT+id;
	ushort	max_idx;
	ushort	ret = 0;

	if(lp->void_read[target] != 0){
		if( !Ope_Log_ManageKindGet( id ) ){// SRAM�̂�
			//  RAM�݂̂Ƀf�[�^������LOG�̏ꍇ�Aunread��RAM�G���A�̐擪(s2[RECODE_SIZE2])����ǂ̈ʒu��ǂނ�������
			if(lp->f_unread[target] > 0){// ���ǃf�[�^�L
				lp->unread[target] += lp->void_read[target];
				lp->f_unread[target] -= lp->void_read[target];		// ���ǃf�[�^�����Z
				max_idx = LOG_SECORNUM( id );						// �ő働�O���擾
				if( lp->unread[target] >= max_idx ){				// ���ǃ��R�[�h�ԍ����ő働�O���𒴂����H
					lp->unread[target] -= max_idx;	// ���ǃ��R�[�h�ԍ����O����U�蒼��
				}
				if(lp->f_unread[target] <= lp->nearFull[target].NearFullMinimum){
					// �j�A�t������
					lp->nearFull[target].NearFullStatus = RAU_NEARFULL_CANCEL;
					lp->nearFull[target].NearFullStatusBefore = RAU_NEARFULL_CANCEL;
				}
				Log_CheckBufferFull(FALSE, id, target);				// �o�b�t�@�t�������`�F�b�N
			}
		} else {// FROM/SRAM����
			//  FROM/SRAM�̗����Ƀf�[�^������LOG�̏ꍇ�Aunread�͑������ɑ΂��ĉ������[�h���Ă��邩������
			if( Ope_Log_TotalCountGet(id)>lp->unread[target] ){
				if(lp->overWriteUnreadCount[target]) {				// �폜���ꂽ�����M�f�[�^����
					lp->unread[target] += lp->void_read[target];
					ret = lp->overWriteUnreadCount[target];
					lp->overWriteUnreadCount[target] = 0;			// �폜�f�[�^�ɑΉ������̂ŃN���A����
				}
				else {
					lp->unread[target] += lp->void_read[target];
				}
			}
			if((Ope_Log_TotalCountGet(id) - lp->unread[target]) <= lp->nearFull[target].NearFullMinimum){
				// �j�A�t������
				lp->nearFull[target].NearFullStatus = RAU_NEARFULL_CANCEL;
				lp->nearFull[target].NearFullStatusBefore = RAU_NEARFULL_CANCEL;
			}
			Log_CheckBufferFull(FALSE, id, target);				// �o�b�t�@�t�������`�F�b�N
		}
		lp->void_read[target] = 0;
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief		LOG�f�[�^�o�b�t�@�t���`�F�b�N
//[]----------------------------------------------------------------------[]
///	@param[in]	occur	: TRUE:�����`�F�b�N FALSE:�����`�F�b�N
///	@param[in]	Lno		: LOG���
///	@param[in]	target	: �`�F�b�N�Ώۓǂݏo�����
/////	@param[in]	strage	: SRAM�t���O
///	@return		void
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	Log_CheckBufferFull(BOOL occur, short Lno, short target)
{
	ushort	cnt;
	ushort	maxcnt;
	ushort	errorCode;									// �G���[�R�[�h
	struct	log_record*	lp;
	uchar	bufferfull = 0;								// 1:�o�b�t�@�t������
	uchar	bufferfullRel = 0;							// 1:�o�b�t�@�t������
	uchar*	pState = NULL;								// �o�b�t�@��Ԋi�[�|�C���^
	t_NtBufState	*ntbufst;
	BOOL strage = Ope_Log_ManageKindGet(Lno);
	
	// ���uNT-NET�̂ݎ���
	switch(target) {
	case eLOG_TARGET_REMOTE:							// ���uNT-NET
		if(prm_get(COM_PRM, S_PAY, 24, 1, 1) != 2) {	// ���uNT-NET���g�p
			return;										// �`�F�b�N���Ȃ�
		}
		break;
	default:
		return;											// �`�F�b�N���Ȃ�
	}

	ntbufst = (t_NtBufState*)NTBUF_GetBufState();
	switch(Lno) {
// MH810100(S) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//	case eLOG_ENTER:									// ����
//		if( prm_get(COM_PRM, S_NTN, 61, 1, 6) == 1 ){
//			return;										// �`�F�b�N���Ȃ�
//		}
//		errorCode = ERR_RAU_ENTRY_BUFERFULL;
//		pState = &ntbufst->car_in;
//		break;
// MH810100(E) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
	case eLOG_PAYMENT:									// ���Z
		if( prm_get(COM_PRM, S_NTN, 61, 1, 5) == 1 ){
			return;										// �`�F�b�N���Ȃ�
		}
		errorCode = ERR_RAU_PAYMENT_BUFERFULL;
		pState = &ntbufst->sale;
		break;
	case eLOG_TTOTAL:									// �W�v
			if( prm_get(COM_PRM, S_NTN, 61, 1, 4) == 1 ){
				return;									// �`�F�b�N���Ȃ�
			}
		errorCode = ERR_RAU_TOTAL_BUFERFULL;
		pState = &ntbufst->ttotal;
		break;
	case eLOG_ERROR:									// �G���[
		if( (prm_get(COM_PRM, S_NTN, 61, 1, 3) == 1) || (prm_get(COM_PRM, S_NTN, 37, 1, 3) == 9) ){
			return;										// �`�F�b�N���Ȃ�
		}
		errorCode = ERR_RAU_ERROR_BUFERFULL;
		break;
	case eLOG_ALARM:									// �A���[��
		if( (prm_get(COM_PRM, S_NTN, 61, 1, 2) == 1) || (prm_get(COM_PRM, S_NTN, 37, 1, 4) == 9) ){
			return;										// �`�F�b�N���Ȃ�
		}
		errorCode = ERR_RAU_ALARM_BUFERFULL;
		break;
	case eLOG_MONITOR:									// ���j�^
		if( (prm_get(COM_PRM, S_NTN, 61, 1, 1) == 1) || (prm_get(COM_PRM, S_NTN, 37, 1, 1) == 9) ){
			return;										// �`�F�b�N���Ȃ�
		}
		errorCode = ERR_RAU_MONITOR_BUFERFULL;
		break;
	case eLOG_OPERATE:									// ����
		if( (prm_get(COM_PRM, S_NTN, 62, 1, 6) == 1)|| (prm_get(COM_PRM, S_NTN, 37, 1, 2) == 9) ){
			return;										// �`�F�b�N���Ȃ�
		}
		errorCode = ERR_RAU_OPE_MONITOR_BUFERFULL;
		break;
	case eLOG_COINBOX:									// �R�C�����ɏW�v(ram�̂�)
		if( prm_get(COM_PRM, S_NTN, 62, 1, 5) == 1 ){
			return;										// �`�F�b�N���Ȃ�
		}
		errorCode = ERR_RAU_COIN_BUFERFULL;
		pState = &ntbufst->coin;
		break;
	case eLOG_NOTEBOX:									// �������ɏW�v(ram�̂�)
		if( prm_get(COM_PRM, S_NTN, 62, 1, 4) == 1 ){
			return;										// �`�F�b�N���Ȃ�
		}
		errorCode = ERR_RAU_NOTE_BUFERFULL;
		pState = &ntbufst->note;
		break;
	case eLOG_PARKING:									// ���ԑ䐔�f�[�^
		if( prm_get(COM_PRM, S_NTN, 62, 1, 3) == 1 ){
			return;										// �`�F�b�N���Ȃ�
		}
		errorCode = ERR_RAU_PARK_CNT_BUFERFULL;
		break;
	case eLOG_MNYMNG_SRAM:								// �ޑK�Ǘ��W�v(ram�̂�)
		if( prm_get(COM_PRM, S_NTN, 62, 1, 2) == 1 ){
			return;										// �`�F�b�N���Ȃ�
		}
		errorCode = ERR_RAU_TURI_BUFERFULL;
		break;
	case eLOG_GTTOTAL:									// GT�W�v
		if( prm_get(COM_PRM, S_NTN, 61, 1, 4) == 1 ){
			return;										// �`�F�b�N���Ȃ�
		}
		errorCode = ERR_RAU_GTTOTAL_BUFERFULL;
		break;
	case eLOG_REMOTE_MONITOR:
		// @todo ���M�}�X�N�ݒ肪�ǉ����ꂽ��A�ݒ���Q�Ƃ��邱��
		errorCode = ERR_RAU_RMON_BUFERFULL;
		break;
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
	case eLOG_LONGPARK_PWEB:
		errorCode = ERR_RAU_LONGPARK_BUFERFULL;
		break;
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(���O�o�^)
	case eLOG_MONEYMANAGE:								// ���K�Ǘ�(�o�b�t�@�t���G���[�R�[�h�Ȃ�)
	default:
		return;											// ��L�ȊO�̓`�F�b�N���Ȃ�
	}
	
	cnt = LOG_SECORNUM(Lno);
	lp = LOG_DAT+Lno;
	
	if(strage) {										// FROM+SRAM�̃��O
		if(occur == TRUE) {								// �����`�F�b�N
			// FROM�ɏ����ލۂɖ����M�f�[�^���㏑�������ꍇ�̓o�b�t�@�t�������Ƃ���
			maxcnt = Ope_Log_GetLogMaxCount(Lno) - cnt;
			if((Ope_Log_TotalCountGet(Lno) - lp->unread[target]) >= maxcnt) {
				bufferfull = 1;
			}
		}
		else {											// �����`�F�b�N
			if(lp->Fcount == FLT_Get_LogSectorCount(Lno)) {					// FROM��1�����Ă���
				if(lp->unread[target] > lp->count[lp->kind]) {				// FROM�ɑ��M�ς݃f�[�^������
					if(cnt <= (lp->unread[target] - lp->count[lp->kind])) {	// 1�Z�N�^��FROM�ɋ󂫂�����
						bufferfullRel = 1;
					}
				}
			}
			else {
				bufferfullRel = 1;
			}
		}
	}
	else {												// SRAM�݂̂̃��O
		if(occur == TRUE) {								// �����`�F�b�N
			if(lp->f_unread[target] >= cnt) {
				bufferfull = 1;
			}
		}
		else {											// �����`�F�b�N
			if(lp->f_unread[target] < cnt) {
				bufferfullRel = 1;
			}
		}
	}
	
	if(bufferfull) {
		RAU_err_chk(errorCode, 1, 0, 0, NULL);
		if(pState) {
			if(Lno == eLOG_PAYMENT) {
				// ���Z�f�[�^�̓j�A�t�����ݒ肷��
				*pState = (uchar)(NTBUF_BUFFER_FULL|NTBUF_BUFFER_NEAR_FULL); 
			}
			else {
				*pState = (uchar)NTBUF_BUFFER_FULL;
			}
		}
	}
	else if(bufferfullRel) {
		RAU_err_chk(errorCode, 0, 0, 0, NULL);
		if(pState) {
			*pState ^= (uchar)NTBUF_BUFFER_FULL;
		}
	}
	else {
		if(Lno == eLOG_PAYMENT) {					// ���Z�f�[�^
			if(lp->unread[target] > lp->count[lp->kind]) {				// FROM�ɑ��M�ς݃f�[�^������
				if(RAU_NTBUF_NEARFULL_COUNT <= (lp->unread[target] - lp->count[lp->kind])) {
					*pState ^= (uchar)NTBUF_BUFFER_NEAR_FULL;
				}
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| LOG�f�[�^�A�N�Z�X���擾 (�Z�}�t�H�t���O�擾)                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Log_SemGet( DtNum )                                     |*/
/*| PARAMETER    : �f�[�^�ԍ��i"OPE_DTNUM_xxxx" �V���{�����g�p���邱�Ɓj   |*/
/*| RETURN VALUE : 0=�擾���s�i�g�p���j,1=����                             |*/
/*|				   �� �p�ς݌�͕K���J�����邱�ƁBLog_SemFre(DtNum)		   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2005-06-20                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	Log_SemGet( uchar DtNum )
{
	ulong	TargetBit;

	TargetBit = 1L << DtNum;

	if( 0 == (Ope_f_Sem_DataAccess & TargetBit) ){			/* �擾�\ (Y) */
		Ope_f_Sem_DataAccess |= TargetBit;
		return	(uchar)1;
	}
	return	(uchar)0;
}

/*[]----------------------------------------------------------------------[]*/
/*| LOG�f�[�^�A�N�Z�X���J�� (�Z�}�t�H�t���O�J��)                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Log_SemFre( DtNum )                                     |*/
/*| PARAMETER    : �f�[�^�ԍ��i"OPE_DTNUM_xxxx" �V���{�����g�p���邱�Ɓj   |*/
/*| RETURN VALUE : none													   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : M.Okuda                                                 |*/
/*| Date         : 2005-06-20                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	Log_SemFre( uchar DtNum )
{
	ulong	TargetBit;

	TargetBit = 0xffffffff;
	TargetBit ^= (1L << DtNum);
	Ope_f_Sem_DataAccess &= TargetBit;
}

/*[]----------------------------------------------------------------------[]*/
/*| ���j�^/���샂�j�^�f�[�^�̏������@�擾                                  |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : getMonitorInfo                                          |*/
/*| PARAMETER    : kind = ���j�^/���샂�j�^���                            |*/
/*|				   code = ���j�^/���샂�j�^�R�[�h						   |*/
/*|				   tbl  = �������@���̌����ΏۂƂ���e�[�u��			   |*/
/*|				   cnt  = "tbl"�͔z��Ȃ̂ŁA���̗v�f��					   |*/
/*| RETURN VALUE : ���j�^/���샂�j�^�f�[�^�̏������@                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : machida.k                                               |*/
/*| Date         : 2005-11-30                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
const t_NTNET_MonitorCtrl	*getMonitorInfo(uchar kind, uchar code, const t_NTNET_MonitorTbl *tbl, size_t tbl_cnt)
{
	size_t i, j;
	
	for (i = 0; i < tbl_cnt; i++) {
		if (tbl[i].Kind == kind) {
			for (j = 0; j < tbl[i].DataNum; j++) {
				if (tbl[i].CtrlData[j].ID == code) {
					return &tbl[i].CtrlData[j];
				}
			}
			break;
		}
	}
	
	return NULL;
}

/*[]----------------------------------------------------------------------[]*
 *| ascii(2byte) --> bcd(1byte)
 *[]----------------------------------------------------------------------[]*
 *| MODULE NAME  : asciitobcd
 *| PARAMETER    : ascii1:ascii(1byte), ascii2:ascii(1byte)
 *| RETURN VALUE : bcd(1byte)
 *| ex) ascii1=0x31, ascii2=0x32 --> retuen(0x12)
 *[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
uchar	asciitobcd( uchar ascii1, uchar ascii2 )
{
	uchar	bcd;

	ascii2 &= 0x0f;
	ascii1 <<= 4;
	ascii1 |= ascii2;
	bcd = ascii1;

	return(bcd);
}
/*[]-----------------------------------------------------------------------[]*/
/*|	20�����܂ł�ASCII������i10�i����:'0'�`'9'�j�� BCD(10byte)�ɕϊ�		|*/
/*|																			|*/
/*|	���l�߂ŋ󂫂ɂ�0x0���i�[�����B										|*/
/*|	ex) ASCII=(31|32|33) --> BCD=(00|00|00|00|00|00|00|00|01|23)			|*/
/*[]-----------------------------------------------------------------------[]*/
/*| PARAMETER    : ascii = �ϊ���Ascii������ւ̃|�C���^					|*/
/*|				   bcd = �ϊ���f�[�^�Z�b�g�G���A�ւ̃|�C���^				|*/
/*|				   ascii_len = ascii �̕�����								|*/
/*| RETURN VALUE : �Ȃ�														|*/
/*[]-----------------------------------------------------------------------[]*/
/*| Author       : M.Suzuki													|*/
/*| Date         : 2006/02/22												|*/
/*| Update       : 															|*/
/*[]-------------------------------------- Copyright(C) 2006 AMANO Corp.---[]*/
void	ASCIItoBCD( uchar *ascii, uchar *bcd, ushort ascii_len )
{
	uchar	w_ascii[20];	
	uchar	w_bcd[10];	
	ushort	p_ascii, p_bcd, i;

	memset( w_bcd, 0, sizeof(w_bcd) );
	memset( w_ascii, 0, sizeof(w_ascii) );
	memcpy( w_ascii, ascii,  (size_t)ascii_len );

	p_ascii = ascii_len - 1;
	p_bcd = 10 -1;
	
	for(i=0; i<ascii_len; ) {
		if( (ascii_len - i) < 2 ) {
			w_bcd[p_bcd] = asciitobcd( '0', w_ascii[p_ascii] );
		} else {
			w_bcd[p_bcd] = asciitobcd( w_ascii[p_ascii-1], w_ascii[p_ascii] );
		}
		p_ascii -= 2;
		p_bcd--; 
		i += 2;
	}
	memcpy( bcd, w_bcd, sizeof(w_bcd));
	return;
}
/*[]---------------------------------------------------------------------------[]*/
/*|		�s�������O�f�[�^�i�[����												|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	NgLog_write( card_type, cardno, cardno_len )			|*/
/*|																				|*/
/*|	PARAMETER		:	ushort card_type	=	�J�[�h���						|*/
/*|						char *cardno		=	�J�[�h�ԍ��ް��߲���			|*/
/*|						ushort cardno_len	=	�J�[�h�ԍ��ް��ݸ޽				|*/
/*|																				|*/
/*|	RETURN VALUE	:	void													|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	M.Suzuki														|*/
/*|	Date	:	2006-02-20														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.--------[]*/
void	NgLog_write( ushort card_type, uchar *cardno, ushort cardno_len )
{
	ushort	w_cardno;

	/* �J�[�h��ʃZ�b�g */
	nglog_data.Card_type = card_type;
	
	/* �J�[�h�ԍ��Z�b�g */
	switch( card_type ){
		case	NG_CARD_PASS:		// �����
		case	NG_CARD_TICKET:		// ���Ԍ�
		case	NG_CARD_AMANO:		// �s���A�}�m�W���J�[�h
		case	OK_CARD_AMANO:		// ����A�}�m�W���J�[�h
			w_cardno = astoin ( cardno, (short)cardno_len );
			nglog_data.Card_No.card_s = w_cardno;
			break;

		case	NG_CARD_CREDIT:		// �N���W�b�g�J�[�h
		case	NG_CARD_BUSINESS:	// �r�W�l�X�J�[�h
			ASCIItoBCD( cardno, nglog_data.Card_No.card_c, cardno_len );
			break;

		default:					// ����`�J�[�h���
			nglog_data.Card_No.card_s = 0;
			break;
	}

	/* ���t�Z�b�g�i���ݓ����j */
	memcpy( &nglog_data.NowTime, &CLK_REC, sizeof( date_time_rec ) );
	Log_Write(eLOG_NGLOG, &nglog_data, TRUE);					// �s�����O�o�^
	return;
}
/*[]---------------------------------------------------------------------------[]*/
/*|		���o�Ƀ��O�f�[�^�i�[����												|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	IoLog_write( event_type, sub_type )						|*/
/*|																				|*/
/*|	PARAMETER		:	ushort event_type	=	���o�ɃC�x���g���				|*/
/*|						ushort sub_type		=	���o�ɃC�x���g��ʁi�ڍׁj		|*/
/*|						ushort ticketnum	=	���ԍ��i���Z�J�n�p�j			|*/
/*|						ushort flg			=	���d���̂Q�d�o�^�����v(1)��(0)	|*/
/*|																				|*/
/*|	RETURN VALUE	:	void													|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	M.Suzuki														|*/
/*|	Date	:	2006-03-03														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.--------[]*/
void	IoLog_write(ushort event_type, ushort room, ulong ticketnum, ulong flg )
{
// MH810100(S) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
//	uchar	is_save = 1;																			/* ���O�������݃t���O(0:�s/1:��) */
//	ushort	 cnt;																					// ���o�Ƀ��O�i�[��, ���ۋL�^��
//	ushort	LockIndex;																				/* �ړI�̎Ԏ��ԍ����i�[����Ă���INDEX */
//	ST_OIBAN2	comp_oiban;																			/* ��r�Ώۂ̒ǔ� */
//	ST_OIBAN2	oiban;																				/* �擾�����ǔ� */
//
//	cnt = 0;
//	is_save = (uchar)LKopeGetLockNum(0, room, &LockIndex);											/* �Ԏ��ԍ�����INDEX���擾 */
//	if(is_save)																						/* �Ԏ��ԍ��Y������(�L�^�\) */
//	{
//		if(LockIndex > BIKE_START_INDEX)															/* ���ݎԎ��̑��u�����b�N */
//		{
//			is_save = (uchar)prm_get(COM_PRM, S_TYP, 127, 1, 1);									/* 03-0127�E����L�^�ێ擾 */
//			IO_LOG_REC.LockIndex = (LockIndex - BIKE_START_INDEX - 1) + INT_CAR_LOCK_MAX;			// ���b�N���u�p�̗̈��20�`
//			if (IO_LOG_REC.LockIndex >= IO_LOG_CNT) {
//				is_save = 0;
//			}
//		}
//		else if(LockIndex <= BIKE_START_INDEX)														/* ���ݎԎ��̑��u���t���b�v */
//		{
//			is_save = (uchar)prm_get(COM_PRM, S_TYP, 127, 1, 2);									/* 03-0127�D����L�^�ێ擾 */
//			IO_LOG_REC.LockIndex = (LockIndex - INT_CAR_START_INDEX - 1);							// �t���b�v���u�p�̗̈��0�`
//			if (IO_LOG_REC.LockIndex >= IO_LOG_CNT) {
//				is_save = 0;
//			}
//		}
//		else																						/* ��L�ȊO(�L�^�����ݒ�) */
//		{
//			is_save = 0;																			/* ���O�������ݖ��� */
//		}
//	}
//
//	if(is_save)																						/* ���O�������݉\�ݒ� */
//	{
//		if(event_type == IOLOG_EVNT_ENT_GARAGE)														// ���O��ʂ�����
//		{
//			// �w�肳�ꂽ�Ԏ��ԍ��̗̈悪�L�^��(���ɓ��ɏ�ԂōX�ɓ��ɂ���������)
//			if(IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].is_rec == 1) {
//				// ���̓��ɂŋL�^���̎Ԏ��͋L�^�I���ɂ��A�V���ȗ̈�Ƀ��O�̋L�^���J�n����B
//				IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].is_rec = 2;
//				Log_Write(eLOG_IOLOG, &IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex], TRUE);			// ���O�ɓo�^
//			}
//
//			// ���o�ɏ�񃍃O�i�[����
//			memset(&IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex], 0, sizeof(IoLog_Data));			// ���O�i�[�G���A��������
//			IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[0].Event_type = event_type;		// ����(����)
//			IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].room = room;								// �Ԏ��ԍ� 
//			IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].is_rec = 1;									// ���O�L�^���ɕύX 
//			// �C�x���g���������ۑ�
//			memcpy(&IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[0].NowTime, &CLK_REC, sizeof(date_time_rec));
//			if(flg)																					// �t���O�ɒl������(�f�o�b�O�p) 
//			{
//				IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[0].NowTime.Hour = (uchar)(flg);	// ���Ɏ���(��)
//				IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[0].NowTime.Min = (uchar)(0);		// ���Ɏ���(��)
//			}
//		}
//		else																						// ���O��ʂ����ɈȊO
//		{
//			// �w�肳�ꂽ�Ԏ��ԍ��̗̈悪�L�^��(���ɓ��ɏ�ԂōX�ɓ��ɂ���������)
//			if(IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].is_rec == 1) {
//				// �L�^���̈�ŋL�^�\�Ȉʒu��T��(�Ō�܂ŏ�񂪂�������L�^���Ȃ�) 
//				for(cnt = 0; (IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt].Event_type) && (cnt < IO_EVE_CNT); cnt++);
//			}
//
//			if(cnt < IO_EVE_CNT)																	// ���O�i�[�ʒu���Ō�܂œ��B���Ă��Ȃ�
//			{
//				memcpy(&IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt].NowTime, &CLK_REC, sizeof(date_time_rec));	// �C�x���g���������ۑ�
//				IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt].Event_type = event_type;// ���o�ɃC�x���g��ʃZ�b�g
//				comp_oiban = IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt-1].IO_INF.io_l;	// ���O�̎��ۂ̐��Z�ǔԂ��擾
//
//				switch( event_type ){																// �C�x���g���Ɋi�[�����𕪂���
//					case	IOLOG_EVNT_AJAST_FIN:													// ���Z����
//						CountGet( PAYMENT_COUNT, (ST_OIBAN *)&oiban );											// �ǔԂ̎擾
//						if((flg == 1) && (CountSel( (ST_OIBAN *)&comp_oiban ) == CountSel( (ST_OIBAN *)&oiban ))){			// ���O�̐��Z�ǔԂ�����Ɠ���
//							is_save = 0;															// ���O�ǉ�����
//							break;																	// ���򏈗��I��
//						}
//						/* ���Z�ǔԎ擾 */
//						IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt].IO_INF.io_l = oiban;
//						break;																		// ���򏈗��I��
//
//					case	IOLOG_EVNT_AJAST_STP:													// ���Z���~
//						CountGet( CANCEL_COUNT, (ST_OIBAN *)&oiban );											// �ǔԂ̎擾
//						if((flg == 1) && (CountSel( (ST_OIBAN *)&comp_oiban ) == CountSel( (ST_OIBAN *)&oiban ))){			// ���O�̐��Z�ǔԂ�����Ɠ���
//							is_save = 0;															// ���O�ǉ�����
//							break;																	// ���򏈗��I��
//						}
//						/* ���Z���~�ǔԎ擾 */
//						IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt].IO_INF.io_l = oiban;
//						break;																		// ���򏈗��I��
//
//					case	IOLOG_EVNT_LOCK_OP_FIN:													// ���փ��b�N�J����
//					case	IOLOG_EVNT_LOCK_OP_UFN:													// ���փ��b�N�J����
//					case	IOLOG_EVNT_FLAP_DW_FIN:													// �t���b�v���~����
//					case	IOLOG_EVNT_FLAP_DW_UFN:													// �t���b�v���~����
//						/* �O��̃C�x���g��ʂ�����Ɠ��� */
//						if((flg == 1) && (IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt-1].Event_type == event_type)){
//							is_save = 0;															// ���O�ǉ�����
//							break;																	// ���򏈗��I��
//						}
//						break;																		// ���򏈗��I��
//
//					case	IOLOG_EVNT_OUT_GARAGE:													// �o��
//						/* �O��̃C�x���g��ʂ�����Ɠ��� */
//						if((flg == 1) && (IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt-1].Event_type == event_type)){
//							is_save = 0;															// ���O�ǉ�����
//							break;																	// ���򏈗��I��
//						}
//						break;																		// ���򏈗��I��
//
//					case	IOLOG_EVNT_FORCE_FIN:													// �����o�Ɋ���
//						CountGet( KIYOUSEI_COUNT, (ST_OIBAN *)&oiban );											// �ǔԂ̎擾
//						if((flg == 1) && (CountSel( (ST_OIBAN *)&comp_oiban ) == CountSel( (ST_OIBAN *)&oiban ))){			// ���O�̐��Z�ǔԂ�����Ɠ���
//							is_save = 0;															// ���O�ǉ�����
//							break;																	// ���򏈗��I��
//						}
//					
//			/* �C�x���g���������ƕs���E�����o�ɂ̔��������͂����ꍇ������̂ŁA ���ݎ����ł͂Ȃ������o�ɂ̔����������Z�b�g������*/
//						memcpy(&IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt].NowTime, 
//							&FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp].Date_Time, sizeof(date_time_rec));
//						/* �����o�ɒǔԎ擾 */
//						IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt].IO_INF.io_l = oiban;
//						break;																		// ���򏈗��I��
//
//					case	IOLOG_EVNT_OUT_ILLEGAL:													// �s���o��
//						CountGet( FUSEI_COUNT, (ST_OIBAN *)&oiban );											// �ǔԂ̎擾
//						if((flg == 1) && (CountSel( (ST_OIBAN *)&comp_oiban ) == CountSel( (ST_OIBAN *)&oiban ))){			// ���O�̐��Z�ǔԂ�����Ɠ���
//							is_save = 0;															// ���O�ǉ�����
//							break;																	// ���򏈗��I��
//						}
//						/* �s���o�ɒǔԎ擾 */
//			/* �C�x���g���������ƕs���E�����o�ɂ̔��������͂����ꍇ������̂ŁA ���ݎ����ł͂Ȃ������o�ɂ̔����������Z�b�g������*/
//						memcpy(&IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt].NowTime, 
//							&FLP_LOG_DAT.Flp_log_dat[FLP_LOG_DAT.Flp_wtp].Date_Time, sizeof(date_time_rec));
//						IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt].IO_INF.io_l = oiban;
//						break;																		// ���򏈗��I��
//
//					case	IOLOG_EVNT_LOCK_OP_STA:													// ���փ��b�N�J�J�n
//					case	IOLOG_EVNT_LOCK_CL_STA:													// ���փ��b�N�J�n
//					case	IOLOG_EVNT_LOCK_CL_FIN:													// ���փ��b�N����
//					case	IOLOG_EVNT_LOCK_CL_UFN:													// ���փ��b�N����
//					case	IOLOG_EVNT_FLAP_DW_STA:													// �t���b�v���~�J�n
//					case	IOLOG_EVNT_FLAP_UP_STA:													// �t���b�v�㏸�J�n
//					case	IOLOG_EVNT_FLAP_UP_FIN:													// �t���b�v�㏸����
//					case	IOLOG_EVNT_FLAP_UP_UFN:													// �t���b�v�㏸����
//					case	IOLOG_EVNT_RENT_GARAGE:													// �ē���
//					case	IOLOG_EVNT_FORCE_STA:													// �����o�ɊJ�n
//					case	IOLOG_EVNT_AJAST_STA:													// ���Z�J�n
//					case	IOLOG_EVNT_OUT_ILLEGAL_START:											// �s���o�ɔ���
//						/* �O��̃C�x���g��ʂ�����Ɠ��� */
//						if((flg == 1) && (IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].iolog_data[cnt-1].Event_type == event_type)){
//							is_save = 0;															// ���O�ǉ�����
//						}
//						break;																		// ���򏈗��I��
//
//					default:																		// ��L�ȊO(�\�����Ȃ��C�x���g)
//						is_save = 0;																// ���O�ǉ�����
//						break;																		// ���򏈗��I��
//				}
//				if(is_save)																			// ���O�ǉ��\
//				{
//					if((cnt + 1) >= IO_EVE_CNT)														// ����̎��ۂ�20���
//					{
//						IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex].is_rec = 2;						// ���o�ɗ����L�^�I��
//						Log_Write(eLOG_IOLOG, &IO_LOG_REC.iolog_list[IO_LOG_REC.LockIndex], TRUE);	// ���O�ɓo�^
//					}
//				}
//			}
//		}
//	}
//	return;
// MH810100(E) K.Onodera  2019/12/26 �Ԕԃ`�P�b�g���X(�s�v�����폜)
}

//------------------------------------------------------------------------------------------
// �ݒ�l�G���A��r���A�Ⴂ�̂���A�h���X�̏���ݒ�ύX�����o�b�t�@�Ɋi�[����B
// �����i�[�p�̊֐����g�p����O�ɕK���R�[��
//
// Param:
//  syubetu ( in ) : �ύX���
//
//
//------------------------------------------------------------------------------------------
void SetSetDiff( unsigned char syubetu )
{

	SetDiffLogBack.cycl = SETDIFFLOG_CYCL_IDLE;						// �ݒ�X�V����ۑ�������
	SetDiffFirstFlag = 0;											// �ݒ�X�V��������i�[�L��
	SetDiffLogBackUp ( );											// ���d��ض��ؗp�ر���ޯ�����
	SetDiffLogBack.cycl = SETDIFFLOG_CYCL_ACTIVE;					// �ݒ�X�V����ۑ����쒆
	if(syubetu == SETDIFFLOG_SYU_DEFLOAD)							// �L�^�����񂪃w�b�_�̂�
	{
		if(SetDiffLogBuff.Info[SetDiffLogDtHeadPon].head.syu == SETDIFFLOG_SYU_PCTOOL)	// �O��w�b�_���PC����̕ύX�v��
		{
			SetDiffFirstFlag = 1;									// �ݒ�X�V��������i�[����
			SetSetDiff_Set(syubetu);								// ����LOG�ɓo�^.
		}
		SetDiff_PutLogInfoHead(syubetu);							// �f�t�H���g�Z�b�g���O�i�[
	}
	/* �ύX��ʂ��o�b�c�[������̕ύX�v�� �܂��� �o�b����̐ݒ�v�� */
// GG120600(S) // Phase9 �ݒ�DL�̗������c��
//	else if((syubetu == SETDIFFLOG_SYU_PCTOOL) || (syubetu == SETDIFFLOG_SYU_PCDEFLOAD))
	else if((syubetu == SETDIFFLOG_SYU_PCTOOL) || (syubetu == SETDIFFLOG_SYU_PCDEFLOAD) || (syubetu == SETDIFFLOG_SYU_REMSET))
// GG120600(E) // Phase9 �ݒ�DL�̗������c��
	{
		SetDiff_PutLogInfoHead(syubetu);							// ��ͯ�ް�̂݊i�[
	}
	else															// ���̃p�����[�^�ύX�o�^
	{
		SetSetDiff_Set(syubetu);									// ����LOG�ɓo�^.
	}
	SetDiffLogBack.cycl = SETDIFFLOG_CYCL_IDLE;						// �ݒ�X�V����ۑ�������
}
//------------------------------------------------------------------------------------------
// �ݒ�l�G���A���r���A�Ⴂ�̂���A�h���X�̏���ݒ�ύX�����o�b�t�@�Ɋi�[����B
//
// Param:
//  sta_add ( in ) : �J�n�A�h���X
//  end_add ( in ) : �I���A�h���X
//  syubetu ( in ) : �ύX���
//------------------------------------------------------------------------------------------
void SetSetDiff_Set(unsigned char syubetu)
{
	short	ses;													// �Q�Ƃ���Z�b�V����
	short	addr;													// �Q�Ƃ���A�h���X
	short	end_add;												// �Q�ƒ��Z�b�V�����̃A�h���X��

	for(ses = 0; ses < C_PRM_SESCNT_MAX; ses++)						// �Z�b�V�������̃��O�o�^����
	{
		end_add = (short)AppServ_CParamSizeTbl[ses];				// �p�����[�^���擾
		for(addr = 1; addr < end_add; addr++)						// �p�����[�^���������J��Ԃ�
		{
			// sta_add(1) end_add(1699) ����=380 : ICE�v�� 40�`50msec
			WACDOG;													// Reset Watch Dog Counter.
			// �����`�F�b�N
			if(CPrmSS[ses][addr] != CPrmMrr[ses][addr])				// �~���[�̈�Ɩ{�̈�Œl���Ⴄ
			{
				if(SetDiffFirstFlag == 0)							// ���񗚗�o�^
				{
					SetDiffFirstFlag = (unsigned char)1;			// ���񗚗�o�^����
					SetDiff_PutLogInfoHead(syubetu);				// ��ͯ�ް PUT
					SetDiffLogBack.cycl = SETDIFFLOG_CYCL_DIFFING;	// �w�b�_�[�i�[�����������i�[��
				}
				// 1��� Put
				SetDiff_PutLogInfoData(ses, addr, CPrmMrr[ses][addr], CPrmSS[ses][addr]);
			}
		}
	}
}
//------------------------------------------------------------------------------------------
// �����f�[�^��ݒ�ύX�����o�b�t�@�Ɋi�[����
//
// Param:
//  addr ( in ) : �A�h���X
//  dataBef ( in ) : �ύX�O�f�[�^ ( [x][0],[x][1],[x][2],[x][3]�� HEX4���łƂ炦���޲�ذ�f�[�^ )
//  dataAft ( in ) : �ύX��f�[�^ ( [x][0],[x][1],[x][2],[x][3]�� HEX4���łƂ炦���޲�ذ�f�[�^ )
//------------------------------------------------------------------------------------------
void SetDiff_PutLogInfoData( short ses, short addr, long dataBef, long dataAft )
{
	// �������̊i�[�������������
	if(SetDiffLogBuff.Info[SetDiffLogDtHeadPon].head.count < SETDIFFLOG_BAK)
	{
		SetDiffWorkBuff.diff.ses = ses;								// �Z�b�V����
		SetDiffWorkBuff.diff.add = addr;							// �A�h���X
		SetDiffWorkBuff.diff.bef = dataBef;							// �ύX�O�ݒ�l
		SetDiffWorkBuff.diff.aft = dataAft;							// �ύX��ݒ�l
		SetDiff_PutLogInfoSave(1);									// �ʃf�[�^�������i�[
	}
	SetDiffLogBuff.Info[SetDiffLogDtHeadPon].head.zen_cnt++;		// �����f�[�^�S�����͏�ɉ��Z����
	nmisave(&CPrmMrr[ses][addr], &CPrmSS[ses][addr], 4);			// �ݒ�l���~���[�̈�֔��f
}
//------------------------------------------------------------------------------------------
// �ݒ�ύX�����o�b�t�@�Ƀf�[�^�ʃw�b�_�[���i�[����
//
// Param:
//  syubetu (in) : �ύX���
//
//------------------------------------------------------------------------------------------
void SetDiff_PutLogInfoHead( unsigned char syubetu )
{
	// �Q�ƒ����ʃw�b�_�[�|�C���^�L��.����̍������i�[���ɎQ��.
	SetDiffLogDtHeadPon = SetDiffLogBuff.Header.InPon;
	SetDiffWorkBuff.head.hed_mark[0] = 'H';							// �w�b�_�[�}�[�N(���)
	SetDiffWorkBuff.head.hed_mark[1] = 'D';							// �w�b�_�[�}�[�N(����)
	SetDiffWorkBuff.head.syu = syubetu;								// �L�^�����
	SetDiffWorkBuff.head.count = 0;									// ��ͯ�ް����ѽ���߂͊܂܂�Ȃ���
	SetDiffWorkBuff.head.zen_cnt = 0;								// ��ͯ�ް����ѽ���߂͊܂܂�Ȃ���
	SetDiffWorkBuff.head.year = CLK_REC.year;						// �������(�N)
	SetDiffWorkBuff.head.mont = CLK_REC.mont;						// �������(��)
	SetDiffWorkBuff.head.date = CLK_REC.date;						// �������(��)
	SetDiffWorkBuff.head.hour = CLK_REC.hour;						// �������(��)
	SetDiffWorkBuff.head.minu = CLK_REC.minu;						// �������(��)
	SetDiff_PutLogInfoSave(0);										// �w�b�_�[�i�[����
}
//------------------------------------------------------------------------------------------
// �ݒ�ύX�����o�b�t�@�ɂP���i�[����.
// �i�[�ʒu���f�[�^�ʃw�b�_�[�̏ꍇ�́A���̂P�����܂�f�[�^��j������
//
// Param:
//  cnt_add_flag ( in ) : ��ͯ�ް�̶������Z����(1)/���Ȃ�(0)
//
// Global:
// SetDiffWorkBuff ( in ) : �i�[����f�[�^
//------------------------------------------------------------------------------------------
void SetDiff_PutLogInfoSave( unsigned char cnt_add_flag )
{

	if ( SetDiffLogBuff.Header.UseInfoCnt >= SETDIFFLOG_MAX )
	{
		SetDiffLogBuff.Header.UseInfoCnt = 							// ���g�p�� - n(+1=�ʃw�b�_�[)
				SetDiffLogBuff.Header.UseInfoCnt - (unsigned short)( SetDiffLogBuff.Info[SetDiffLogBuff.Header.InPon].head.count + 1 );
		SetDiffLogBuff.Header.OlderPon = 							// �ޯ̧ͯ�ް�̍ŌÃ|�C���^�X�V(+1=�ʃw�b�_�[)
				SetDiff_UpdatePon ( SetDiffLogBuff.Header.OlderPon, (unsigned short)(SetDiffLogBuff.Info[SetDiffLogBuff.Header.InPon].head.count + 1) );
	}

	// ���݊i�[�ʒu�֏��i�[
	memcpy ( &SetDiffLogBuff.Info[SetDiffLogBuff.Header.InPon], &SetDiffWorkBuff, sizeof(TD_SETDIFFLOG_INFO));

	if( cnt_add_flag ){												// �ۑ���񂪍������
		SetDiffLogBuff.Info[SetDiffLogDtHeadPon].head.count++;		// �f�[�^�ʃw�b�_�[�̃f�[�^�� + 1
	}
	//�o�b�t�@�w�b�_�[�̓��̓|�C���^�X�V
	SetDiffLogBuff.Header.InPon = SetDiff_UpdatePon ( SetDiffLogBuff.Header.InPon, 1 );

	if ( SetDiffLogBuff.Header.UseInfoCnt < SETDIFFLOG_MAX ) {		// �L���������̈�̖����ɒB���Ă��Ȃ�
		SetDiffLogBuff.Header.UseInfoCnt++;							// �ޯ̧ͯ�ް�̎g�p��񐔁{�P
	}
}
//------------------------------------------------------------------------------------------
// ���̓|�C���^�[�A�ŌÏ��|�C���^�[�� �w�萔�����i�߂�.�P�c�̏ꍇ�͐擪�ɂ܂킵����
//
// Param:
//  OldPon ( in ) : ���̓|�C���^�[�A�ŌÏ��|�C���^�[
//  AddPon ( in ) : ���Z����l
//
// Return:
//  ���Z��̒l
//------------------------------------------------------------------------------------------
unsigned short SetDiff_UpdatePon ( unsigned short OldPon, unsigned short AddPon )
{
	OldPon += AddPon;

	if( OldPon >= SETDIFFLOG_MAX )
	{
		return (unsigned short)( OldPon - SETDIFFLOG_MAX );
	}
	return OldPon;
}
//------------------------------------------------------------------------------------------
// �����r���̓d���f�������d���̃��J�o�[�G���A�� �P��������ƃo�b�t�@�w�b�_�[���Z�[�u����B
// �Z�[�u������́A�ŌÃf�[�^����P�������i�Ԃ����\������j�ő吔�s���B
//
// ���ꂩ��i�[���鍷�����̐��͂܂��킩��Ȃ��A�̂ŁA��ɂP�������ő吔�Z�[�u���Ă��܂�.
//
// SetDiffLogBack.Info[0] ����P�������ő�{�Q(ͯ�ޤ��ѽ����)�����ŌÃf�[�^���ۑ������
//------------------------------------------------------------------------------------------
void SetDiffLogBackUp ( void )
{
	unsigned short	i;
	unsigned short	pon;

	//�o�b�t�@�w�b�_�[�Z�[�u
	memcpy ( &SetDiffLogBack.Header, &SetDiffLogBuff.Header, sizeof(TD_SETDIFFLOG_INDX));

	//�������Z�[�u
	pon = SetDiffLogBuff.Header.OlderPon;					//�ŌÃ|�C���^�[
	for( i = 0; i < SETDIFFLOG_BAK + (2); i++ )
	{
		memcpy ( &SetDiffLogBack.Info[i], &SetDiffLogBuff.Info[pon], sizeof(TD_SETDIFFLOG_INDX));
		pon = SetDiff_UpdatePon ( pon, 1 );
	}
}
//------------------------------------------------------------------------------------------
// ���d���̃��J�o�[
// ���J�o�[�G���A�̃w�b�_�[�𕜊���A�������𕜊�������
//
// ��� SetDiffLogBack.Info[0] ����P�������ő�{�w�b�_�[�����ŌÃf�[�^���ۑ�����Ă���
//------------------------------------------------------------------------------------------
void SetDiffLogRecoverCheck_Go ( void )
{
	unsigned short	i;
	unsigned short	pon;

	//safe... ��̈�ԍŏ��̗����グ��,.or.�J����, �\������.(memclr() �O�ɂ��̏����s������)
	if ( SetDiffLogBuff.Header.UseInfoCnt > SETDIFFLOG_MAX ||
	     SetDiffLogBuff.Header.InPon >= SETDIFFLOG_MAX ||
	     SetDiffLogBuff.Header.OlderPon >= SETDIFFLOG_MAX )
	{
		memset ( &SetDiffLogBuff, 0, sizeof(SetDiffLogBuff) );
	}

	// ���J�o�[�̕K�v���邩�H
	if ( SetDiffLogBack.cycl == SETDIFFLOG_CYCL_ACTIVE &&
		 SetDiffLogBack.Info[0].head.hed_mark[0] == 'H' &&
		 SetDiffLogBack.Info[0].head.hed_mark[1] == 'D' ) {

		//�o�b�t�@�w�b�_�[
		memcpy ( &SetDiffLogBuff.Header, &SetDiffLogBack.Header, sizeof(TD_SETDIFFLOG_HEAD));

		pon = SetDiffLogBuff.Header.OlderPon;					//�ŌÃ|�C���^�[
		for( i = 0; i < SETDIFFLOG_BAK + (2); i++ )
		{
			memcpy ( &SetDiffLogBuff.Info[pon], &SetDiffLogBack.Info[i], sizeof(TD_SETDIFFLOG_INDX));
			pon = SetDiff_UpdatePon ( pon, 1 );
		}
	}
	SetDiffLogBack.cycl = SETDIFFLOG_CYCL_IDLE;

}
//------------------------------------------------------------------------------------------
// ���d���̗̈揉��������
// �~���[�̈�ւ̏�񕡎ʂ��s��
//
// �����ɂ���ď����������Ə����菇��ύX����
//------------------------------------------------------------------------------------------
void SetDiffLogReset( uchar ope_mode )
{
	switch(ope_mode)												// �����ɂ���ď����𕪊�
	{
	case 1:															// �ݒ�X�V������񏉊���
		memset(&SetDiffLogBuff, 0, sizeof(SetDiffLogBuff));			// ���ۑ��̈�S������
		memset(&SetDiffLogBack, 0, sizeof(SetDiffLogBack));			// �o�b�N�A�b�v�̈揉����
		SetDiffLogDtHeadPon = 0;									// �ŐV�w�b�_���ʒu������
	case 2:															// �~���[�����O����
		memcpy(&CPMirror, &CParam, sizeof(t_CParam));				// ���ʃp�����[�^���~���[�̈�֕���
	case 0:															// �ʏ�N����
		SetDiffLogMirrorAddress();									// �~���[�̈�̃A�h���X�z���K�p
		if(SetDiffLogBack.cycl == SETDIFFLOG_CYCL_DIFFING)			// �O���d���O�̓��삪�o�^��
		{
			SetSetDiff_Set(SETDIFFLOG_SYU_NONE);					// �ݒ�X�V����o�^�����ĊJ
		}
		break;														// ���򏈗��I��
	}
}
//------------------------------------------------------------------------------------------
// ���d���̗̈揉��������
// �~���[�̈�ւ̏�񕡎ʂ��s��
//
// �����ɂ���ď����������Ə����菇��ύX����
//------------------------------------------------------------------------------------------
void SetDiffLogMirrorAddress(void)
{
		CPrmMrr[0]	= CPMirror.CParam00;							/*								*/
		CPrmMrr[1]	= CPMirror.CParam01;							/*								*/
		CPrmMrr[2]	= CPMirror.CParam02;							/*								*/
		CPrmMrr[3]	= CPMirror.CParam03;							/*								*/
		CPrmMrr[4]	= CPMirror.CParam04;							/*								*/
		CPrmMrr[5]	= CPMirror.CParam05;							/*								*/
		CPrmMrr[6]	= CPMirror.CParam06;							/*								*/
		CPrmMrr[7]	= CPMirror.CParam07;							/*								*/
		CPrmMrr[8]	= CPMirror.CParam08;							/*								*/
		CPrmMrr[9]	= CPMirror.CParam09;							/*								*/
		CPrmMrr[10]	= CPMirror.CParam10;							/*								*/
		CPrmMrr[11]	= CPMirror.CParam11;							/*								*/
		CPrmMrr[12]	= CPMirror.CParam12;							/*								*/
		CPrmMrr[13]	= CPMirror.CParam13;							/*								*/
		CPrmMrr[14]	= CPMirror.CParam14;							/*								*/
		CPrmMrr[15]	= CPMirror.CParam15;							/*								*/
		CPrmMrr[16]	= CPMirror.CParam16;							/*								*/
		CPrmMrr[17]	= CPMirror.CParam17;							/*								*/
		CPrmMrr[18]	= CPMirror.CParam18;							/*								*/
		CPrmMrr[19]	= CPMirror.CParam19;							/*								*/
		CPrmMrr[20]	= CPMirror.CParam20;							/*								*/
		CPrmMrr[21]	= CPMirror.CParam21;							/*								*/
		CPrmMrr[22]	= CPMirror.CParam22;							/*								*/
		CPrmMrr[23]	= CPMirror.CParam23;							/*								*/
		CPrmMrr[24]	= CPMirror.CParam24;							/*								*/
		CPrmMrr[25]	= CPMirror.CParam25;							/*								*/
		CPrmMrr[26]	= CPMirror.CParam26;							/*								*/
		CPrmMrr[27]	= CPMirror.CParam27;							/*								*/
		CPrmMrr[28]	= CPMirror.CParam28;							/*								*/
		CPrmMrr[29]	= CPMirror.CParam29;							/*								*/
		CPrmMrr[30]	= CPMirror.CParam30;							/*								*/
		CPrmMrr[31]	= CPMirror.CParam31;							/*								*/
		CPrmMrr[32]	= CPMirror.CParam32;							/*								*/
		CPrmMrr[33]	= CPMirror.CParam33;							/*								*/
		CPrmMrr[34]	= CPMirror.CParam34;							/*								*/
		CPrmMrr[35]	= CPMirror.CParam35;							/*								*/
		CPrmMrr[36]	= CPMirror.CParam36;							/*								*/
		CPrmMrr[37]	= CPMirror.CParam37;							/*								*/
		CPrmMrr[38]	= CPMirror.CParam38;							/*								*/
		CPrmMrr[39]	= CPMirror.CParam39;							/*								*/
		CPrmMrr[40]	= CPMirror.CParam40;							/*								*/
		CPrmMrr[41]	= CPMirror.CParam41;							/*								*/
		CPrmMrr[42]	= CPMirror.CParam42;							/*								*/
		CPrmMrr[43]	= CPMirror.CParam43;							/*								*/
		CPrmMrr[44]	= CPMirror.CParam44;							/*								*/
		CPrmMrr[45]	= CPMirror.CParam45;							/*								*/
		CPrmMrr[46]	= CPMirror.CParam46;							/*								*/
		CPrmMrr[47]	= CPMirror.CParam47;							/*								*/
		CPrmMrr[48]	= CPMirror.CParam48;							/*								*/
		CPrmMrr[49]	= CPMirror.CParam49;							/*								*/
		CPrmMrr[50]	= CPMirror.CParam50;							/*								*/
		CPrmMrr[51]	= CPMirror.CParam51;							/*								*/
		CPrmMrr[52]	= CPMirror.CParam52;							/*								*/
		CPrmMrr[53]	= CPMirror.CParam53;							/*								*/
		CPrmMrr[54]	= CPMirror.CParam54;							/*								*/
		CPrmMrr[55]	= CPMirror.CParam55;							/*								*/
		CPrmMrr[56]	= CPMirror.CParam56;							/*								*/
		CPrmMrr[57]	= CPMirror.CParam57;							/*								*/
		CPrmMrr[58]	= CPMirror.CParam58;							/*								*/
		CPrmMrr[59]	= CPMirror.CParam59;							/*								*/
		CPrmMrr[60]	= CPMirror.CParam60;							/*								*/
}

//[]----------------------------------------------------------------------[]
///	@brief		�W�v���O��NT-NET�f�[�^�Őݒ肷��V�[�P���V����No.��ݒ肷��
//[]----------------------------------------------------------------------[]
///	@param[in]	Lno			: ���O���(LOG_TTOTAL/LOG_GTTOTAL)
///				pTotalLog	: �W�v���O�f�[�^
///	@return		none
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
void	Log_SetTotalLogNtSeqNo(short Lno, SYUKEI* pTotalLog)
{
	ushort	index = 0;

	if(Lno == LOG_TTOTAL || Lno == LOG_GTTOTAL) {			// T���v�܂���GT���v���Ώ�
		
		// �f�[�^��ʕ��̃V�[�P���V����No.��ݒ肷��
		for(index = 0; index < TOTALLOG_NTDATA_COUNT; ++index) {
			pTotalLog->SeqNo[index] = GetNtDataSeqNo();
		}
	}
}

// �e�X�g�p���O�쐬(S)
#ifdef TEST_LOGFULL
void Test_Ope_LogFull(void)
{
	ushort i;
	ushort max = 0;
	struct {
		uchar payment;
		uchar total;
		uchar gttotal;
		uchar enter;
		uchar power;
		uchar error;
		uchar alarm;
		uchar operation;
		uchar monitor;
		uchar coinbox;
		uchar notebox;
		uchar turikan;
	} target;				// �f�o�b�O�p
	
	memset(&target, 1, sizeof(target));
	
	// ���Z�f�[�^�̍쐬
	if(target.payment) {
		max = Ope_Log_GetLogMaxCount(eLOG_PAYMENT) - 1;
		for(i = 0; i < max; ++i) {
			OPECTL.f_KanSyuu_Cmp = 0;
			OPECTL.Op_LokNo = 1;
			car_in_f.year = car_ot_f.year = CLK_REC.year;
			car_in_f.mon = car_ot_f.mon = CLK_REC.mont;
			car_in_f.day = car_ot_f.day = CLK_REC.date;
			car_in_f.hour = car_ot_f.hour = CLK_REC.hour;
			car_in_f.min = car_ot_f.min = CLK_REC.minu;
			PayData_set(0, 0);
			kan_syuu();
			taskchg(IDLETSKNO);
		}
	}
	
	// �W�v�f�[�^�̍쐬
	if(target.total) {
		max = Ope_Log_GetLogMaxCount(eLOG_TTOTAL) - 1;
		for(i = 0; i < max; ++i) {
			syuukei_prn(PREQ_AT_SYUUKEI, &sky.tsyuk);
			memcpy( &sky.tsyuk.NowTime, &CLK_REC, sizeof(date_time_rec));
			ac_flg.syusyu = 11;							// 11:�s���v�󎚊���
			Make_Log_TGOUKEI();
			syuukei_clr(0);
			taskchg(IDLETSKNO);
		}
	}

	// GT�f�[�^�̍쐬
	if(target.gttotal) {
		max = Ope_Log_GetLogMaxCount(eLOG_GTTOTAL) - 1;
		for(i = 0; i < max; ++i) {
			syuukei_prn(PREQ_AT_SYUUKEI, &sky.gsyuk);
			memcpy( &sky.gsyuk.NowTime, &CLK_REC, sizeof(date_time_rec));
			ac_flg.syusyu = 22;							// 22:�f�s���v�󎚊���
			syuukei_clr(1);
			taskchg(IDLETSKNO);
		}
	}

// MH810100(S) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//	// ���Ƀf�[�^�̍쐬
//	if(target.enter) {
//		max = Ope_Log_GetLogMaxCount(eLOG_ENTER) - 1;
//		for(i = 0; i < max; ++i) {
//			FLAPDT.flp_data[1].year = CLK_REC.year;
//			FLAPDT.flp_data[1].mont = CLK_REC.mont;
//			FLAPDT.flp_data[1].date = CLK_REC.date;
//			FLAPDT.flp_data[1].hour = CLK_REC.hour;
//			FLAPDT.flp_data[1].minu = CLK_REC.minu;
//			Make_Log_Enter(1);
//			Log_regist(LOG_ENTER);
//			taskchg(IDLETSKNO);
//		}
//	}
// MH810100(E) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
	
	// �╜�d�f�[�^�̍쐬
	if(target.power) {
		max = Ope_Log_GetLogMaxCount(eLOG_POWERON) - 1;
		for(i = 0; i < max; ++i) {
			Log_regist(LOG_POWERON);
			taskchg(IDLETSKNO);
		}
	}
	
	// �G���[�f�[�^�̍쐬
	if(target.error) {
		memcpy( &Err_work.Date_Time, &CLK_REC, sizeof( date_time_rec ) );	// ��������
		Err_work.Errsyu = 255;											// �װ���
		Err_work.Errcod = 255;											// �װ����
		Err_work.Errdtc = 2;											// �װ����/����
		Err_work.Errlev = 5;											// �װ����
		Err_work.ErrDoor = 0;											// �ޱ��ԂƂ���
		Err_work.Errinf = 0;											// �װ���L��
		max = Ope_Log_GetLogMaxCount(eLOG_ERROR) - 1;
		for(i = 0; i < max; ++i) {
			Log_regist(LOG_ERROR);										// �װ۸ޓo�^
			taskchg(IDLETSKNO);
		}
	}
	
	// �A���[���f�[�^�̍쐬
	if(target.alarm) {
		memcpy( &Arm_work.Date_Time, &CLK_REC, sizeof( date_time_rec ) );	// ��������
		Arm_work.Armsyu = 255;											// �װю��
		Arm_work.Armcod = 255;											// �װѺ���
		Arm_work.Armdtc = 2;											// �װє���/����
		Arm_work.Armlev = 5;											// �װ�����
		Arm_work.ArmDoor = 0;											// �ޱ��ԂƂ���
		Arm_work.Arminf = 0;											// �װя��L��
		max = Ope_Log_GetLogMaxCount(eLOG_ALARM) - 1;
		for(i = 0; i < max; ++i) {
			Log_regist(LOG_ALARM);										// �װ�۸ޓo�^
			taskchg(IDLETSKNO);
		}
	}
	
	// ���샂�j�^�f�[�^
	if(target.operation) {
		max = Ope_Log_GetLogMaxCount(eLOG_OPERATE) - 1;
		for(i = 0; i < max; ++i) {
			wopelg(OPLOG_DOOROPEN, 0, 0);
			taskchg(IDLETSKNO);
		}
	}
	
	// ���j�^�f�[�^
	if(target.monitor) {
		max = Ope_Log_GetLogMaxCount(eLOG_MONITOR) - 1;
		for(i = 0; i < max; ++i) {
			wmonlg(OPMON_DOOROPEN, 0, 0);
			taskchg(IDLETSKNO);
		}
	}
	
	// �R�C�����ɏW�v�f�[�^�̍쐬
	if(target.coinbox) {
		max = Ope_Log_GetLogMaxCount(eLOG_COINBOX) - 1;
		for(i = 0; i < max; ++i) {
			kinko_syu(0, 0);
			taskchg(IDLETSKNO);
		}
	}
	
	// �������ɏW�v�f�[�^�̍쐬
	if(target.notebox) {
		max = Ope_Log_GetLogMaxCount(eLOG_NOTEBOX) - 1;
		for(i = 0; i < max; ++i) {
			kinko_syu(1, 0);
			taskchg(IDLETSKNO);
		}
	}
	
	// �ޑK�Ǘ��f�[�^�̍쐬
	if(target.turikan) {
		max = Ope_Log_GetLogMaxCount(eLOG_MNYMNG_SRAM) - 1;
		for(i = 0; i < max; ++i) {
			turi_kan.Kikai_no = (ushort)CPrmSS[S_PAY][2];						// �@�B��
			turi_kan.Kakari_no = OPECTL.Kakari_Num;								// �W���ԍ�set
			memcpy( &turi_kan.NowTime, &CLK_REC, sizeof( date_time_rec ) );		// ���ݎ���
			turikan_subtube_set();
			CountGet( TURIKAN_COUNT, &turi_kan.Oiban );							// �ǔ�
			CountUp(TURIKAN_COUNT);
			turikan_clr();
			taskchg(IDLETSKNO);
		}
	}
}
#endif	// TEST_LOGFULL
// �e�X�g�p���O�쐬(E)
// MH810103 GG119202 (s) #5320 ���j�^�R�[�h�́uR0252�v�A�uR0253�v���ʐM�f�[�^�ő��M����Ă��܂�					
//[]----------------------------------------------------------------------[]
///	@brief			���j�^�f�[�^���M�Ώۂ̔��菈��
//[]----------------------------------------------------------------------[]
///	@param[in]		MonKind   : ���
///	@param[in]		MonCode   : �R�[�h
///	@return			ret       : 0:���M�ΏۊO/1:���M�Ώ�
///	@attention		
///	@note			
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
uchar chk_mon_send_ntnet( uchar MonKind,uchar MonCode)
{
	const t_NTNET_MonitorCtrl	*MonCtrl;
	// ���j�^���擾
	MonCtrl = _getMonLogInfo(MonKind, MonCode);
	if(MonCtrl && (MonCtrl->Operation &LOGOPERATION_COM) ){
		// ���M��������
		return 1;
	}
	return 0;
}

//[]----------------------------------------------------------------------[]
///	@brief			���샂�j�^�f�[�^���M�Ώۂ̔��菈��
//[]----------------------------------------------------------------------[]
///	@param[in]		OpeMonKind: ���
///	@param[in]		OpeMonCode: �R�[�h
///	@return			ret       : 0:���M�ΏۊO/1:���M�Ώ�
///	@attention		
///	@note			
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
uchar chk_opemon_send_ntnet( uchar OpeMonKind,uchar OpeMonCode)
{
	const t_NTNET_MonitorCtrl	*MonCtrl;
	// ���샂�j�^���擾
	MonCtrl = _getOpeLogInfo(OpeMonKind, OpeMonCode);
	if(MonCtrl && (MonCtrl->Operation &LOGOPERATION_COM) ){
		// ���M��������
		return 1;
	}
	return 0;
}
// MH810103 GG119202 (e) #5320 ���j�^�R�[�h�́uR0252�v�A�uR0253�v���ʐM�f�[�^�ő��M����Ă��܂�					
