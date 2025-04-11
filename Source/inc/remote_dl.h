#ifndef	_REMOTE_DL_TASK_H_
#define	_REMOTE_DL_TASK_H_
//[]----------------------------------------------------------------------[]
///	@mainpage		
///	remote download function <br><br>
///	<b>Copyright(C) 2010 AMANO Corp.</b>
///	CREATE			2010/06/04 Namioka<br>
///	UPDATE			
///	@file			remote_dl.c
///	@date			2010/06/04
///	@version		MH702207
//[]------------------------------------- Copyright(C) 2010 AMANO Corp.---[]


#include "ntnet_def.h"
// GG120600(S) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
#define CHG_INFO_VERSION					1
// GG120600(E) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���

// MH322915(S) K.Onodera 2017/05/22 ���u�_�E�����[�h(���ԃ`�F�b�N�ύX)
typedef enum{
	VER_TYPE_MH,
	VER_TYPE_GM,
	VER_TYPE_GW,
	VER_TYPE_GG,

	VER_TYPE_MAX,
} eVER_TYPE;
// MH322915(E) K.Onodera 2017/05/22 ���u�_�E�����[�h(���ԃ`�F�b�N�ύX)

// MH322915(S) K.Onodera 2017/05/22 ���u�_�E�����[�h�C��
#define		UNSENT_TIME_BEFORE_SW	5		// �؊����O�̖����M���O�̑��M�J�n����(�ؑւ̉����O��)
// MH322915(E) K.Onodera 2017/05/22 ���u�_�E�����[�h�C��

/*
#define	R_DL_IDLE 			0	// �ҋ@���
#define	R_DL_REQ_RCV 		1	// ���u�_�E�����[�h�v����M�i�J�n�҂��j
#define	R_SW_WAIT 			2	// �v���O�����X�V�҂��i�\��j
#define	R_DL_START 			3	// ���u�_�E�����[�h���s�J�n�i�\��j
#define	R_SW_START 			4	// �v���O�����ؑ֊J�n
#define	R_UL_RES_RESV_DL	5	// �_�E�����[�h���ʏ�񑗐M�\��
#define	R_UL_RES_RESV_SW	6	// �v���O�����X�V���ʏ�񑗐M�\��
#define	R_UL_RES_RESV_CN	7	// FTP�ڑ��m�F��񑗐M�\��
#define	R_DL_EXEC			8	// ���u�_�E�����[�h���s�iTask�N���j
#define	R_UL_DLRES_EX		9	// �_�E�����[�h���ʏ�񑗐M���s�iTask�N���j
#define	R_UL_SWRES_EX		10	// �X�V���ʌ��ʏ�񑗐M���s�iTask�N���j
#define	R_UL_CNRES_EX		11	// FTP�ڑ��m�F��񑗐M���s�iTask�N���j
*/
enum{
	R_DL_IDLE 				=0, // �ҋ@���
	R_DL_REQ_RCV, 				// ���u�����e�i���X�v����M�i�J�n�҂��j
	R_UP_WAIT,					// �A�b�v���[�h�҂�
	R_RESET_WAIT,				// ���Z�b�g�҂�
	R_SW_WAIT, 					// �v���O�����X�V�҂��i�\��j
// -----------------------------------�� NOT_DOWNLOADING()
// -----------------------------------�� DOWNLOADING()
	R_DL_START, 				// ���u�_�E�����[�h���s�J�n�i�\��j
	R_UP_START,					// �A�b�v���[�h���s�J�n
	R_RESET_START,				// ���Z�b�g�J�n
	R_TEST_CONNECT,				// FTP�ڑ��e�X�g�J�n
	R_SW_START,					// �v���O�����ؑ֊J�n
	R_DL_EXEC,					// ���u�_�E�����[�h���s
};

// ���u�v���ԍ�
enum {
	VER_UP_REQ = 1,					// �o�[�W�����A�b�v�v��
	PROGNO_CHG_REQ,					// ���ԕύX�v��
	PARAM_CHG_REQ,					// �ݒ�ύX�v��
	PARAM_UPLOAD_REQ,				// �ݒ�v��(�A�b�v���[�h)
	RESET_REQ,						// ���Z�b�g�v��
	PROG_ONLY_CHG_REQ,				// �v���O�����ؑ֗v��
	FTP_CHG_REQ,					// FTP�ݒ�ύX�v��
	REMOTE_FEE_REQ,					// ���u�����ݒ�v��

	FTP_TEST_REQ = 10,				// FTP�ڑ��e�X�g�v��
// MH810100(S) Y.Yamauchi 2019/11/27 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	PARAM_DIFF_CHG_REQ = 13,		// �����ݒ�ύX�v��
	REMOTE_REQ_MAX
// MH810100(E) Y.Yamauchi 2019/11/27 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
};
// MH810100(S) Y.Yamauchi 2019/11/27 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
//#define REMOTE_REQ_MAX				(REMOTE_FEE_REQ+1)
// MH810100(E) Y.Yamauchi 2019/11/27 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)

// �������
enum {
	PROC_KIND_EXEC,					// ���s
	PROC_KIND_EXEC_RT,				// �������s
	PROC_KIND_CANCEL,				// ���
//	PROC_KIND_CHG_TIME,				// �����ύX
	PROC_KIND_RESV_INFO,			// �\����e
};

// ���u�v������
enum {
	REQ_NONE,						// �ύX�Ȃ�
	REQ_ACCEPT,						// ��t
	REQ_NOT_ACCEPT,					// ��t�s��
	REQ_PROGNO_ERR,					// �v�����C���v���O�������Ԉُ�
	REQ_RCV_NONE,					// �v������M
	REQ_NOT_ACCEPT_RT,				// ��t�s��(�������s�p)
	REQ_SW_PROG_NONE,				// �v���O�����f�[�^�Ȃ�
	REQ_SW_PROGNO_ERR,				// �؊����ԕs��v

	REQ_CONN_OK = 8,				// �ڑ�����
	REQ_CONN_NG,					// �ڑ����s

	REQ_PROG_DL_WAIT = 11,			// �v���O�����_�E�����[�h�J�n�҂�
	REQ_PROG_DL_EXEC,				// �v���O�����_�E�����[�h��
	REQ_PROG_SW_WAIT,				// �v���O�����X�V�J�n�҂�
	REQ_PARAM_DL_WAIT,				// �ݒ�_�E�����[�h�J�n�҂�
	REQ_PARAM_DL_EXEC,				// �ݒ�_�E�����[�h��
	REQ_PARAM_SW_WAIT,				// �ݒ�X�V�J�n�҂�
	REQ_PARAM_UL_WAIT,				// �ݒ�A�b�v���[�h�҂�
	REQ_PARAM_UL_EXEC,				// �ݒ�A�b�v���[�h��
	REQ_RESET_WAIT,					// ���Z�b�g�J�n�҂�
	
	REQ_TEST_NOW = 20,				// FTP�ڑ��e�X�g��
	
	REQ_CANCEL_ACCEPT = 50,			// �����t
	REQ_CANCEL_NOT_ACCEPT,			// �����t�s��
};

// �ؑ֎��
enum {
	SW_PROG,						// �v���O����
	SW_WAVE,						// ����
	SW_PARAM,						// �ݒ�
	SW_PROG_EX,						// �v���O�����؊��v��
// GG120600(S) // Phase9 �p�����[�^�ؑւ𕪂���
	SW_PARAM_DIFF,					// �ݒ�i�����j
// GG120600(E) // Phase9 �p�����[�^�ؑւ𕪂���
	SW_MAX,
};

// �v�����
#define REQ_KIND_VER_UP				(1 << 0)		// �o�[�W�����A�b�v�v��
#define REQ_KIND_PROGNO_CHG			(1 << 1)		// ���ԕύX�v��
#define REQ_KIND_PARAM_CHG			(1 << 2)		// �ݒ�ύX�v��
#define REQ_KIND_PARAM_UL			(1 << 3)		// �ݒ�v��
#define REQ_KIND_RESET				(1 << 4)		// ���Z�b�g�v��
#define REQ_KIND_PROG_ONLY_CHG		(1 << 5)		// �v���O�����ؑ֗v��
#define REQ_KIND_FTP_CHG			(1 << 6)		// FTP�ݒ�ύX�v��
#define REQ_KIND_TEST				(1 << 7)		// �ڑ��e�X�g
// MH810100(S) Y.Yamauchi 20191224 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
#define REQ_KIND_PARAM_DIF_CHG		(1 << 8)		// �����ݒ�ύX�v��
// MH810100(S) Y.Yamauchi 20191224 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
#define REQ_KIND_ALL				(0xFFFFFFFF)

// �J�n�������
enum {
	PROG_DL_TIME,					// �v���O�����_�E�����[�h����
	PARAM_DL_TIME,					// �ݒ�_�E�����[�h����
	PARAM_UP_TIME,					// �ݒ�A�b�v���[�h����
	RESET_TIME,						// ���Z�b�g����
	PROG_ONLY_TIME,					// �v���O�����ؑ֎���
// MH810100(S) Y.Yamauchi 20191223 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)	
	PARAM_DL_DIF_TIME,				// �����ݒ�_�E�����[�h����
// MH810100(E) Y.Yamauchi 20191223 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)	
	TEST_TIME,						// �ڑ��e�X�g
	TIME_MAX,
};
#define TIME_INFO_MAX				TEST_TIME

// �����
enum {
	INFO_KIND_START,				// �J�n����
	INFO_KIND_SW,					// �X�V����
	INFO_KIND_MAX
};

// ���g���C���
enum {
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//	RETRY_KIND_CONNECT,				// �ڑ����g���C
//	RETRY_KIND_DL,					// �_�E�����[�h���g���C
//	RETRY_KIND_UL,					// �A�b�v���[�h���g���C
	RETRY_KIND_CONNECT_PRG,				// �ڑ����g���C�i�v���O�����j
	RETRY_KIND_CONNECT_PARAM_UP,		// �ڑ����g���C�i�p�����[�^UP�j
	RETRY_KIND_CONNECT_PARAM_DL,		// �ڑ����g���C�i�p�����[�^DL�j
	RETRY_KIND_CONNECT_PARAM_DL_DIFF,	// �ڑ����g���C�i�p�����[�^DL�����j
// ------------------------------
	RETRY_KIND_DL_PRG,					// �_�E�����[�h���g���C�i�v���O�����j
	RETRY_KIND_DL_PARAM,				// �_�E�����[�h���g���C�i�p�����[�^�j
	RETRY_KIND_DL_PARAM_DIFF,			// �_�E�����[�h���g���C�i�p�����[�^�����j
	RETRY_KIND_UL,						// �A�b�v���[�h���g���C
	RETRY_KIND_MAX,

	RETRY_KIND_CONNECT_MAX = RETRY_KIND_DL_PRG,
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
};

// ���Ԏ��
enum {
	PROGNO_KIND_DL,					// �_�E�����[�h����
	PROGNO_KIND_SW,					// �؊�����
	PROGNO_KIND_MAX
};

// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
enum {
	PRM_UP_DATE_NONE = 0,			// �Ȃ�
	PRM_UP_DATE_REQ,				// �Z���^�[�f�[�^�v��
	PRM_UP_DATE_FTP,				// FTP��

};
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�


// �Í����t�@�C���̊g���q
#define PROG_FILE_EXT		".enc\r\n"

// �p�����[�^�t�@�C����
#define PARAM_FILE_NAME		"param.enc"
// MH810100(S) K.Onodera 2019/11/19 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)
// �p�����[�^�t�@�C����(bin�`��)
#define PARAM_BIN_FILE_NAME		"param.dat"
// MH810100(E) K.Onodera 2019/11/19 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)

// �A�b�v���[�h����
#define PARAM_UP_COMP		0
// �e�X�g�ڑ�����
#define TEST_CHECK_COMP		0		// �ڑ��e�X�g����I��

// �_�E�����[�h����
#define	PROG_DL_COMP		0		// �v���O�����_�E�����[�h����I��
#define	FOMA_COMM_ERR		1		// FOMA�ʐM�ُ�
#define	CONN_TIMEOUT_ERR	2		// �ڑ��^�C���A�E�g
#define	LOGIN_ERR			3		// ���O�C���F�؃G���[
#define	SCRIPT_FILE_ERR		4		// �X�N���v�g���e�ُ�i�t�H�[�}�b�g�ُ�j
#define	SCRIPT_FILE_NONE	5		// �X�N���v�g�t�@�C�����Ȃ�
#define	PROG_DL_ERR			6		// �_�E�����[�h�����t�@�C���ُ�iSUM�l���قȂ�j
#define	PROG_DL_NONE		7		// �_�E�����[�h�t�@�C�����Ȃ�
#define	FLASH_WRITE_ERR		8		// �t���b�V���ւ̏����ُ݈�
#define	PROG_DL_RESET		9		// �v���O�����_�E�����[�h/�X�V���ɐ��Z�@���Z�b�g
#define TEST_UP_ERR			10		// �e�X�g�t�@�C���A�b�v���[�h���s
#define TEST_DOWN_ERR		11		// �e�X�g�t�@�C���_�E�����[�h���s
#define TEST_DEL_ERR		12		// �e�X�g�t�@�C���폜���s
#define	FTP_QUIT_ERR		13		// �ؒf�G���[
#define PROG_SUM_ERR		14		// �v���O����SUM�l�G���[
#define PARAM_UP_ERR		20		// �A�b�v���[�h���s
#define	EXCLUDED			0xff	// �ΏۊO�i�����s�j

// �X�V����
#define	PROG_SW_COMP		0		// �v���O�����X�V����I��
#define	BACKUP_ERR			1		// �S�f�[�^�o�b�N�A�b�v���s
#define	PROG_SW_NONE		2		// �ҋ@�ʂɐ؂�ւ��Ώۂ̃f�[�^���Ȃ�
#define	SW_INFO_WRITE_ERR	3		// �X�V���̏����݂Ɏ��s
#define	RESTORE_ERR			4		// ���X�g�A���s

enum {
	CTRL_NONE,
	CTRL_PROG_DL,					// �v���O�����_�E�����[�h
	CTRL_PARAM_DL,					// �ݒ�_�E�����[�h
	CTRL_PARAM_UPLOAD,				// �ݒ�A�b�v���[�h
	CTRL_RESET,						// ���Z�b�g
	CTRL_PROG_SW,					// �v���O�����؊�
	CTRL_CONNECT_CHK,				// �ڑ��e�X�g
// MH810100(S) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
	CTRL_PARAM_DIF_DL,				// �����ݒ�_�E�����[�h
// MH810100(E) Y.Yamauchi 2019/12/18 �Ԕԃ`�P�b�g���X(���u�_�E�����[�h)
// GG120600(S) // Phase9 LCD�p
	CTRL_PARAM_UPLOAD_LCD,			// �ݒ�A�b�v���[�h(LCD�p)
// GG120600(E) // Phase9 LCD�p
};

enum {
// GG120600(S) // Phase9 ���ʂ���ʂ���
//	RES_DL,							// �_�E�����[�h����
//	RES_SW,							// �X�V����
	RES_DL_PROG,					// �_�E�����[�h����
	RES_DL_PARAM,					// �_�E�����[�h����
	RES_DL_PARAM_DIF,				// �_�E�����[�h����
	RES_SW_PROG,					// �X�V����
	RES_SW_PARAM,					// �X�V����
	RES_SW_PARAM_DIF,				// �X�V����
// GG120600(E) // Phase9 ���ʂ���ʂ���
	RES_UP,							// �A�b�v���[�h����
	RES_COMM,						// �ڑ��`�F�b�N����
// GG120600(S) // Phase9 LCD�p
	RES_UP_LCD,						// �A�b�v���[�h����LCD�p
// GG120600(E) // Phase9 LCD�p
	RES_MAX,
};

// ���g���C�X�e�[�^�X
#define RETRY_OVER			0x80	// ���g���C�I�[�o�[
#define RETRY_EXEC			0x40	// ���g���C���s��

#define MAKE_FILENAME_SW				4
#define MAKE_FILENAME_PARAM_MKD			5
#define MAKE_FILENAME_PARAM_UP			6
#define MAKE_FILENAME_TEST_UP			7
// MH810100(S) K.Onodera 2019/11/19 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)
#define MAKE_FILENAME_PARAM_UP_FOR_LCD	8
// MH810100(E) K.Onodera 2019/11/19 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^)

// �����X�e�[�^�X
enum {
	EXEC_STS_NONE,					// ��
	EXEC_STS_COMP,					// ��
	EXEC_STS_ERR,					// �d
};

// ���u�Ď��f�[�^
#define RMON_OFFSET_REAL_TIME		30000		// �������s

// �J�n����o�ߎ���
#define NG_ELAPSED_TIME				30

#define	PROG_CHG_MAX		3		// �v���O�����X�V���MAX 0�FMAIN 1�F���� 2�F���ʃp�����[�^
									
#define	BACKUP_RETRY_COUNT	3
#define	RESTORE_RETRY_COUNT	3
#define	UNSENT_DATA_RETRY_COUNT	300	// ���M�V�[�P���X�L�����Z���J�E���g�i300�b�j: RAU_SND_SEQ_TIME_UP

#define RESULT_SND_TIMER	90		// �v���O�����X�V��̍ċN�����̌��ʏ�񑗐M�҂��^�C�}�[�i�b�j
enum{
	REMOTE_AUTO_SCRIPT = 0,	// ���u�_�E�����[�h�pFTP�N���C�A���g�X�N���v�g�i�����j
	REMOTE_MANUAL_SCRIPT,	// ���u�_�E�����[�h�pFTP�N���C�A���g�X�N���v�g�iRism����̎w��j
	MANUAL_SCRIPT,			// �蓮�pFTP�N���C�A���g�X�N���v�g(�����e�i���X)
	PARAMTER_SCRIPT,		// ���ʃp�����[�^�A�b�v���[�h�pFTP�N���C�A���g�X�N���v�g
	REMOTE_NO_SCRIPT,		// �X�N���v�g�Ȃ�(ParkingWeb����̎w��)
// MH810100(S) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^�j
	CONNECT_TEST,			// �ڑ��e�X�g
	LCD_NO_SCRIPT,			// �X�N���v�g�Ȃ�(LCD�ւ̃A�b�v���[�h)
// MH810100(E) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X�i��LCD �p�����[�^�j
	
	// �ǉ����͂������灪�ɒǉ�
	SCRIPT_MAX
};


#define TASK_START_FOR_DL()		(remotedl_task_startup_status_get() >= R_DL_EXEC )
#define DOWNLOADING()			(remotedl_status_get() >= R_DL_START)
#define NOT_DOWNLOADING()		(remotedl_status_get() <= R_SW_WAIT )
#define DOOR_OPEN_OR_SOLENOID_UNROCK()	(OPE_SIG_DOOR_Is_OPEN  || !ExIOSignalrd(INSIG_DOOR))

enum{
	FLASH_WRITE_BEFORE = 0,		// FLASH�����ݑO
	FLASH_FIRST_WRITE_START,	// ���񏑍��ݎ��s
	FLASH_WRITE_END,			// �����݊����i�f�Ѓf�[�^�j
	FLASH_WRITE_START,			// �����݊J�n�i�f�Ѓf�[�^�j
};


typedef struct{
	uchar			status;			// ���g���C���s���X�e�[�^�X
	date_time_rec	time;			// ���񃊃g���C����
	uchar			count;			// ���g���C�J�E���g

}t_retry_info;

typedef union {
	ulong			uladdr;
	uchar			ucaddr[4];
} u_ipaddr;

typedef struct {
	date_time_rec	sw_time;			// �X�V����
	uchar			exec;				// �X�V�t���O
} t_sw_info;

typedef struct {
	u_inst_no		u;					// ���ߔԍ�
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
	uchar			From;			// �v����
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
	date_time_rec	start_time;			// �J�n����
	u_ipaddr		ftpaddr;			// FTP�T�[�o�A�h���X
	ushort			ftpport;			// FTP�|�[�g�ԍ�
	uchar			ftpuser[24];		// FTP���[�U
	uchar			ftppasswd[24];		// FTP�p�X���[�h
} t_dl_info;

typedef struct {
	PRG_HDR			header;
	ulong			base_addr;
	ushort			sum;
	uchar			cur_dir_ent;
	uchar			resv;
// GG129000(S) R.Endo 2022/12/28 �Ԕԃ`�P�b�g���X4.0 #6754 �v���O�����_�E�����[�h�������g���C��ɉ��u�Ď��f�[�^���M���� [���ʉ��P���� No1540]
	ushort			uscrcTemp;
// GG129000(E) R.Endo 2022/12/28 �Ԕԃ`�P�b�g���X4.0 #6754 �v���O�����_�E�����[�h�������g���C��ɉ��u�Ď��f�[�^���M���� [���ʉ��P���� No1540]
} t_write_info;

typedef struct {
	date_time_rec	start_time;			// �J�n����
	date_time_rec	sw_time;			// �X�V����
	ulong			resv;
	uchar			status[INFO_KIND_MAX];	// �����X�e�[�^�X
} t_remote_time_info;

typedef struct {
	t_remote_time_info	time_info[TIME_INFO_MAX];
// GG120600(S) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
	t_remote_time_info	time_info2[3];	// �_�~�[
// GG120600(E) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
} t_remote_dl_info;

// GG120600(S) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
#define RETRY_KIND_MAX_V0			3
#define RES_MAX_V0					4
#define SW_MAX_V0					4
#define TIME_MAX_V0					6
#define TIME_INFO_MAX_V0			5

typedef struct {
	t_remote_time_info	time_info[TIME_INFO_MAX_V0];
} t_remote_dl_info_V0;

typedef struct {
	u_inst_no		u;					// ���ߔԍ�
	date_time_rec	start_time;			// �J�n����
	u_ipaddr		ftpaddr;			// FTP�T�[�o�A�h���X
	ushort			ftpport;			// FTP�|�[�g�ԍ�
	uchar			ftpuser[24];		// FTP���[�U
	uchar			ftppasswd[24];		// FTP�p�X���[�h
} t_dl_info_V0;

typedef struct{
	int				req_accept;		// ��t�����v�����
	uchar			connect_type;	// �������̎��
	uchar			status;			// �ڑ��X�e�[�^�X
	uchar			script_type;
	uchar			update;
	uchar			result[RES_MAX_V0];// ����
	ushort			retry_dl_cnt;	// �_�E�����[�h�J�n���g���C��
	ushort			failsafe_timer;	// FTP�t�F�[���Z�[�t�^�C�}�[
	ushort			monitor_info;	// �[���Ď����
	uchar			param_up;		// ���ʃp�����[�^�A�b�v���[�h�p�̎��
	uchar			exec_info;		// ���u�_�E�����[�h�̎��s��
	t_retry_info	retry[RETRY_KIND_MAX_V0];	// ���g���C���
	uchar			sw_status[SW_MAX_V0-1];	// sw_update�̃o�b�N�A�b�v�G���A
	uchar			pow_flg;		// ���d�t���O
	ulong			dummy1[10];
// -----------------------------------�� ���s�����
// -----------------------------------�� ��M���
	t_dl_info_V0	dl_info[TIME_MAX_V0];
	t_sw_info		sw_info[SW_MAX_V0];
	uchar			script[PROGNO_KIND_MAX][12];
	t_write_info	write_info;
	ulong			dummy2[10];
}t_prog_chg_info_V0;
// GG120600(E) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���

// GG120600(S) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
//typedef struct{
////	uchar			status;			// �ڑ��X�e�[�^�X
////									
////	uchar			connect_type;	// �ڑ���ʂO�F�v���O�����_�E�����[�h
////									//		   �P�F�v���O�����X�V
////									//		   �Q�F���ʏ�񑗐M����
////									//		   �R�F�ڑ��m�F�`�F�b�N
////									//		   �S�F���ʃp�����[�^�A�b�v���[�h
////									
////	uchar			sw_kind;		// �ؑ֎�ʂO�F���C���v���O����
////									//		   �P�F�T�uCPU
////									//		   �Q�F���ʃp�����[�^
////	uchar			sw_req;			// �ؑ֗v����M
////	uchar			result[RES_MAX];// [0] ���u�_�E�����[�h����
////									// [1] �v���O�����؂�ւ�����
////									// [2] ���ʏ��A�b�v���[�h����/FTP�ڑ��m�F
////	uchar			update;
////	uchar			bk_res;
////	t_dl_info		dl_info[3];
////	date_time_rec	dl_time;
////	uchar			script_type;
////	uchar			script[36];
////	t_retry_info	retry[2];
////	uchar			timeout;
////	uchar			sw_status[3];	// sw_update�̃o�b�N�A�b�v�G���A
////	uchar			param_up;		// ���ʃp�����[�^�A�b�v���[�h�p�̎��
////	uchar			exec_info;		// ���u�_�E�����[�h�̎��s��
////	uchar			Reserve[9];
//int				req_accept;		// ��t�����v�����
//uchar			connect_type;	// �������̎��
//uchar			status;			// �ڑ��X�e�[�^�X
//uchar			script_type;
//uchar			update;
//uchar			result[RES_MAX];// ����
//ushort			retry_dl_cnt;	// �_�E�����[�h�J�n���g���C��
//ushort			failsafe_timer;	// FTP�t�F�[���Z�[�t�^�C�}�[
//ushort			monitor_info;	// �[���Ď����
//uchar			param_up;		// ���ʃp�����[�^�A�b�v���[�h�p�̎��
//uchar			exec_info;		// ���u�_�E�����[�h�̎��s��
//t_retry_info	retry[RETRY_KIND_MAX];	// ���g���C���
//uchar			sw_status[SW_MAX-1];	// sw_update�̃o�b�N�A�b�v�G���A
//uchar			pow_flg;		// ���d�t���O
//ulong			dummy1[10];
//// -----------------------------------�� ���s�����
//// -----------------------------------�� ��M���
//t_dl_info		dl_info[TIME_MAX];
//t_sw_info		sw_info[SW_MAX];
//uchar			script[PROGNO_KIND_MAX][12];
//t_write_info	write_info;
//ulong			dummy2[10];
//}t_prog_chg_info;
typedef struct{

	int				req_accept;		// ��t�����v�����
	uchar			connect_type;	// �������̎��
	uchar			status;			// �ڑ��X�e�[�^�X
	uchar			script_type;
	uchar			update;
	uchar			result[RES_MAX];// ����
	uchar			result_dummy[4];// dummy
	ushort			retry_dl_cnt[TIME_INFO_MAX];	// �_�E�����[�h�J�n���g���C��
	ushort			retry_dl_cnt_dummy[4];	// �_�E�����[�h�J�n���g���C��
	ushort			failsafe_timer;	// FTP�t�F�[���Z�[�t�^�C�}�[
	ushort			monitor_info;	// �[���Ď����
	uchar			param_up;		// ���ʃp�����[�^�A�b�v���[�h�p�̎��
	uchar			exec_info;		// ���u�_�E�����[�h�̎��s��
	t_retry_info	retry[RETRY_KIND_MAX];	// ���g���C���
	t_retry_info	retry_dummy[4];	// ���g���C���
	uchar			sw_status[SW_MAX-1];	// sw_update�̃o�b�N�A�b�v�G���A
	uchar			sw_status_dummy[4];	// sw_update�̃o�b�N�A�b�v�G���A
	uchar			pow_flg;		// ���d�t���O
	ulong			dummy1[10];
// -----------------------------------�� ���s�����
// -----------------------------------�� ��M���
	t_dl_info		dl_info[TIME_MAX];
	t_dl_info		dl_info_dummy[4];
	t_sw_info		sw_info[SW_MAX];
	t_sw_info		sw_info_dummy[4];
	uchar			script[PROGNO_KIND_MAX][12];
	uchar			script_dummy[4][12];
	t_write_info	write_info;
// GG129000(S) R.Endo 2022/12/28 �Ԕԃ`�P�b�g���X4.0 #6754 �v���O�����_�E�����[�h�������g���C��ɉ��u�Ď��f�[�^���M���� [���ʉ��P���� No1540]
// 	t_write_info	write_info_dummy;
	uchar			write_info_dummy[24];
// GG129000(E) R.Endo 2022/12/28 �Ԕԃ`�P�b�g���X4.0 #6754 �v���O�����_�E�����[�h�������g���C��ɉ��u�Ď��f�[�^���M���� [���ʉ��P���� No1540]
	union{
		uchar crc[2];
		ushort uscrc;
	}CRC16;
	uchar			dummy4[1];		// 
	uchar			dummy3[1];		// 
	ulong			dummy2[9];
}t_prog_chg_info;
// GG120600(E) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���

extern	uchar	remotedl_status_get( void );
extern	void	remotedl_status_set( uchar );
extern	uchar	remotedl_task_startup_status_get( void );
extern	uchar	remotedl_connect_type_get( void );
extern	void	remotedl_connect_type_set( uchar );
extern	void	remotedl_info_clr( void );
extern	t_prog_chg_info*	remotedl_info_get( void );
extern	uchar	remotedl_restore( void );
extern	uchar	remotedl_BackUP( void );
extern	ushort	GetMessageRemote( void );
extern	void	AutoDL_UpdateTimeCheck( void );
extern	void	remote_dl_check( uchar	*rcvdata );
extern	void	ftp_remote_auto_update( void );
extern	void	ftp_remote_auto_switch( void );
extern	void	ftp_remote_auto_update_sub(void);
extern	void	moitor_regist( ushort , uchar );
extern	ushort	remotedl_disp( void );
extern	void	remotedl_result_set( uchar );
extern	uchar	remotedl_result_get( uchar );
extern	void	remotedl_result_clr( void );
extern	void	MakeRemoteFileName( uchar*, uchar*, char );
extern	uchar	remote_date_chk( date_time_rec *pTime );
extern	void	remotedl_script_typeset( uchar );
extern	uchar	remotedl_script_typeget( void );
extern	const 	char*	remotedl_script_get( void );
extern	uchar	flap_move_chk( void );
extern	uchar	remotedl_update_chk( void );
extern	void	remotedl_update_set( void );
extern	uchar	remotedl_first_comm_get( void );
extern	uchar	retry_info_set( uchar );
extern	uchar	dl_start_chk( date_time_rec*, uchar );
extern	void	remote_evt_recv( ushort, uchar* );
extern	void	Before_DL_Err_Function( uchar );
extern	uchar	now_retry_active_chk( uchar );
extern	void	remote_result_snd_timer( void );
extern	void	retry_info_clr( uchar );
extern	void	retry_info_connect_timer(void);
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
extern	void retry_info_connect_timer2(void);
extern	void retry_info_connect_timer3(void);
extern	void retry_info_connect_timer4(void);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
extern	void	retry_count_clr(uchar kind);
extern	void	retry_time_clr(uchar kind);
extern	void	connect_timeout( void );
extern	void	remotedl_timeout_sts_set( uchar );
extern	uchar	remotedl_timeout_sts_get( void );
extern	void	remotedl_connect_req( ushort, uchar );
extern	void	remotedl_sw_update_bk( uchar* );
extern	void	remotedl_sw_update_get( uchar* );
extern	void	remotedl_chg_info_bkup( void );
extern	void	remotedl_chg_info_restore( void );
extern	ushort	Param_Upload_ShortCut( void );
extern	void	Param_Upload_type_set( uchar );
extern	uchar	Param_Upload_type_get( void );
extern	void	remotedl_exec_info_set( uchar );
extern	uchar	remotedl_exec_info_get( void );
extern	uchar	GetSnd_RecvStatus( void );
extern	void	SetSnd_RecvStatus( uchar );
extern	void	get_script_file_name( char*, const char* );
extern	uchar	GetFTPMode( void );
extern uchar	up_wait_chk( date_time_rec *time, uchar status );
extern uchar	reset_start_chk( date_time_rec *time, uchar status );
extern ushort AppServ_MakeRemoteDl_TestConnect(char* pBuff,int nSize);
extern	uchar remotedl_proc_resv_exec(t_ProgDlReq *p, BOOL bRT);
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//extern	uchar remotedl_proc_cancel(uchar ReqKind, ulong *pInstNo, BOOL bMnt);
extern	uchar remotedl_proc_cancel(uchar ReqKind, ulong *pInstNo, BOOL bMnt,uchar from);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX
extern	uchar remotedl_proc_chg_time(t_ProgDlReq *p);
extern	uchar remotedl_proc_resv_info(t_ProgDlReq *p);
extern	ulong remotedl_ftp_ipaddr_get(void);
extern	ushort remotedl_ftp_port_get(void);
extern	void remotedl_ftp_user_get(char *user, int size);
extern	void remotedl_ftp_passwd_get(char *passwd, int size);
extern	int remotedl_dl_start_retry_check(void);
extern	int remotedl_sw_start_retry_check(void);
extern	int remotedl_reset_start_retry_check(void);
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
//extern	void remotedl_start_retry_clear(void);
extern	void remotedl_start_retry_clear(uchar dl_kind);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
extern	void remotedl_failsafe_timer_clear(void);
extern	int remotedl_failsafe_timer_check(void);
extern	int remotedl_convert_dl_status(void);
extern	void remotedl_accept_request(int request);
extern	void remotedl_clear_request(int request);
extern	uchar remotedl_is_accept(int request);
extern	void remotedl_complete_request(int request);
extern	void remotedl_recv_info_clear(uchar type);
extern	u_inst_no* remotedl_instNo_get(void);
extern	void rmon_regist(ulong Code);
// GG120600(S) // Phase9 ���u�Ď��f�[�^�ύX
//extern	void rmon_regist_ex(ulong Code, ulong *pInstNo);
extern	void rmon_regist_ex(ulong Code, ulong *pInstNo,uchar from);
// GG120600(E) // Phase9 ���u�Ď��f�[�^�ύX

extern	t_remote_time_info* remotedl_time_info_get(uchar type);
// GG120600(S) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�
//extern	void remotedl_time_info_clear(uchar type);
//extern	void remotedl_time_info_clear_all(void);
extern	void remotedl_time_info_clear(uchar type,u_inst_no *pInstNo,uchar from);
extern	uchar remotedl_instFrom_get(void);
// GG120600(E) // Phase9 �㏑���Œ��~���ɖ��ߔԍ�����ɂȂ�
// GG120600(S) // Phase9 #5073 �y�A���]���w�E�����z���u�����e�i���X��ʂ��玞�����̃N���A������Ɖ��u�Ď��f�[�^�̖��ߔԍ�1�E2�ɒl������Ȃ�(No.02�]0003)
extern	u_inst_no* remotedl_instNo_get_with_Time_Type(uchar timeType);
extern	uchar remotedl_instFrom_get_with_Time_Type(uchar timeType);
// GG120600(E) // Phase9 #5073 �y�A���]���w�E�����z���u�����e�i���X��ʂ��玞�����̃N���A������Ɖ��u�Ď��f�[�^�̖��ߔԍ�1�E2�ɒl������Ȃ�(No.02�]0003)
extern	void remotedl_time_set(uchar kind, uchar type, date_time_rec *pTime);
extern	void remotedl_comp_set(uchar kind, uchar type, uchar status);
extern	void remotedl_ftp_info_set(t_FtpInfo *pFtpInfo);
extern	t_FtpInfo* remotedl_ftp_info_get(void);
extern	ulong remotedl_unsent_data_count_get(void);
extern	void remotedl_prog_no_get(char *pProgNo, int size);
extern	void remotedl_write_info_set(t_write_info *pInfo);
extern	void remotedl_write_info_get(t_write_info *pInfo);
extern	void remotedl_write_info_clear(void);
extern	int remotedl_check_request(uchar ReqKind);
extern	ushort	IsMainProgExist(void);
extern	void remotedl_monitor_info_set(ushort info);
extern	ushort remotedl_monitor_info_get(void);
extern	void remotedl_pow_flg_set(BOOL bFlg);
extern	const	uchar	ReqAcceptTbl[REMOTE_REQ_MAX];
//extern	t_ProgDlReq g_bk_ProgDlReq;
// GG120600(S) // Phase9 �ݒ�ύX�ʒm�Ή�
extern	void	rmon_regist_ex_FromTerminal(ulong Code);
// GG120600(S) // Phase9 ���g���C�̎�ʂ𕪂���
extern	void	rmon_regist_with_ConectType(ulong Code,uchar connect_type);
// GG120600(E) // Phase9 ���g���C�̎�ʂ𕪂���
extern	void	remotedl_arrange_next_request(void);
extern	void	remotedl_cancel_setting(void);
extern	void	remotedl_prog_crc_set( uchar uc1,uchar uc2);
extern	ushort	remotedl_prog_crc_get( void );			
// GG120600(E) // Phase9 �ݒ�ύX�ʒm�Ή�
// GG120600(S) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���
extern	void	remotedl_chg_info_log0_to1(void);
// GG120600(E) // Phase9 �ȑO��t_prog_chg_info����o�[�W�����A�b�v���s����悤�ɂ���

#endif
