/*[]----------------------------------------------------------------------[]*/
/*|		New I/F(Master) common include									   |*/
/*|			�E�����ɂ́A�VI/F�Ձi�e�@�j���̑S�R�^�X�N���Q�Ƃ���̈��	   |*/
/*|			  �錾���܂��B												   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  T.Hayase		                                           |*/
/*| Date        :  2005-01-18                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#ifndef _IFM_H_
#define _IFM_H_

#include	"IF.h"
#include	"IFmail.h"

/*----------------------------------*/
/*		area style define			*/
/*----------------------------------*/
/** ��M�o�b�t�@�Ǘ��e�[�u�� **/
typedef struct {
    ushort  RcvCnt;						/* ��M�o�C�g�� */
	ushort	OvfCount;					/* ��M�o�b�t�@�I�[�o�[�t���[������ */
	ushort	ComerrStatus;				/* �ʐM�G���[��ԁi�r�b�g���āH�j*/

	ushort	usBetweenChar;				/* �����Ԏ�M�Ď��^�C�} */

	BOOL			RcvCmpFlg;			/* ��M���������t���O */
} t_SciRcvCtrl;

/** ���M�o�b�t�@�Ǘ��e�[�u�� **/
typedef struct {
    ushort  SndReqCnt;					/* ���M�v���o�C�g�� */
    ushort  SndCmpCnt;					/* ���M�v���o�C�g�� */
    ushort  ReadIndex;					/* next send data (read) index */
    ushort  SndCmpFlg;					/* ���M���������t���O */
										/*		1=���� */
										/*		0=������ */
} t_SciSndCtrl;

/* ��M�G���[��� */
typedef struct {
	ushort	usOVRcnt;					/* �I�[�o�[�����G���[������ */
	ushort	usFRMcnt;					/* �t���[�~���O�G���[������ */
	ushort	usPRYcnt;					/*     �p���e�B�G���[������ */
} t_SciRcvError;

/* �ʐM�G���[��� */
typedef struct {
	ulong	ulRcv_HeaderString;			/* ��F�w�b�_�����G���[������ */
	ulong	ulRcv_Size;					/* ��F�f�[�^���G���[������ */
	ulong	ulRcv_CRC;					/* ��FCRC�ُ픭���� */
	ulong	ulRcv_Kind;					/* ��F�f�[�^��ʈُ픭���� */
	ulong	ulSnd_noEmpty;				/* ���F���M���uTDR��v�ł͖��������� */
} t_SciComError;

/* �G���[��� */
typedef struct {
	uchar	ucState;					/* �G���[��� */
										/*		0=���� */
										/*		1=���� */
										/*		2=�����E�������� */
	ulong	ulCount;					/* �G���[������ */
} t_IF_Error;

/* ���b�N���u���[�J�[�ʂ̐M���o�͎��ԃe�[�u�� */
typedef struct {
	ushort	usLockOnTime;		/* ��(�{��)����A�M���o�͎��ԁi�~150msec�v�Z�O��150msec�P�ʁj*/
	ushort	usLockOffTime;		/* �J(�J��)����A�M���o�͎��ԁi�~150msec�v�Z�O��150msec�P�ʁj*/
} t_IF_Signal;



/****************************/
/* �e�@�^�X�N�ԃL���[		*/
/****************************/
/*
*	�{�f�[�^�͐e�@�ł̃L���[�̎��̂������܂��B
*	�E���[���f�[�^�t�H�[�}�b�g�`���Ƌ��ʉ����܂��B
*
*	1)�|�[�����O�����f�[�^
*	�E���Z�@����̃|�[�����O�Ɂu�f�[�^����v�ŉ������邽�߂̃L���[�ł��B
*	�E���b�N�Ǘ��^�X�N��<�L���[>���ΐ��Z�@�ʐM�^�X�N
*			��(22H)I/F�՗v���f�[�^
*			��(61H)�ʏ��ԃf�[�^
*			��(63H)���b�N���u�J�e�X�g�����i�S���b�N/�w�胍�b�N���ʁj
*			��(64H)�����e�i���X��񉞓��i���ғ����j
*
*	2)�q�@���M�҂��f�[�^
*	�E�q�@�ւ̃��b�N����́A��x�ɂP���b�N���u�ɂ����ł��Ȃ��̂ŁA
*	  ����v���𗭂ߍ��ނ��߂̃L���[�i�҂����X�g�j�ł��B
*	�E���b�N�Ǘ��^�X�N��<�L���[>��(���䊮����)���b�N�Ǘ��^�X�N��[���[��]���q�@�ʐM�^�X�N
*			��(49H�F'I')�����ݒ�f�[�^
*			��(57H�F'W')��ԏ����݃f�[�^
*			��(52H�F'R')��ԗv���f�[�^�i�o�[�W�����v���j
*/

/****************************/
/* �e�@�^�X�N�ԃ��[���S�̑�	*/
/****************************/
// ���C�����փf�[�^���󂯓n���`
typedef union {
t_IF_Mail_Header2	sCommon;		// ���ʈ����B�f�[�^��ʊm��O�ɖ{���p�̂������Ƃ��g�p����
									// �ΐ��Z�@�ʐM�^�X�N���Ύq�@�ʐM�^�X�N
	t_PAY_22_mail	s22;			// (22H)I/F�՗v���f�[�^
	t_PAY_61_mail	s61;			// (61H)�ʏ푗�M�f�[�^
	t_PAY_62_mail	s62;			// (62H)���b�N���u�J�e�X�g�����i�S���b�N/�w�胍�b�N���ʁj
	t_PAY_63_mail	s63;			// (63H)�����e�i���X��񉞓��i���ғ����j
	t_PAY_64_mail	s64;			// (64H)�o�[�W�����v������
	t_PAY_66_mail	s66;			// (66H)�G���[�f�[�^�ʒm
} t_IFM_mail;

/* �L���[�̎��� */
typedef union {
t_IF_Mail_Header	sCommon;		/* ���ʈ����B�f�[�^��ʊm��O�ɖ{���p�̂������Ƃ��g�p���� */


//�i�q�@�ցj�Z���N�e�B���O�҂����X�g
	t_IF_I49_mail	sI49;			/*	(49H�F'I')�����ݒ�f�[�^ */
	t_IF_W57_mail	sW57;			/*	(57H�F'W')��ԏ����݃f�[�^ */
	t_IF_R52_mail	sR52;			/*	(52H�F'R')��ԗv���f�[�^�i�o�[�W�����v���j*/
	t_Flap_W57_mail	sFlapW57;		// (57H�F'W')��ԏ����݃f�[�^
	t_Flap_I49_mail	sFlapI49;		// (49H�F'I')CRR�����ݒ�f�[�^

//�i���b�N�Ǘ��^�X�N�ɂāj���슮���҂����X�g
									/*	(57H�F'W')��ԏ����݃f�[�^�i����j*/

//�i���b�N�Ǘ��^�X�N�ցj�q�@��ԃ��X�g
	t_IF_A41_mail	sA41;			/* 	(41H�F'A')�q�@��ԃf�[�^ */
	t_IF_V56_mail	sV56;			/* 	(56H�F'V')�q�@�o�[�W�����f�[�^ */
	t_Flap_A41_mail	sFlapA41;		// (41H�F'A')�q�@��ԃf�[�^
	t_Flap_S53_mail	sFlapS53;		// (53H�F'S')�q�@��ԃf�[�^�i���[�v�f�[�^�����j
	t_IF_V56_mail	sFlapv76;		// (76H�F'v')CRR�o�[�W�����f�[�^
	t_IF_t74_mail	sFlapt74;		// (74H�F't')CRR�܂�Ԃ��e�X�g����
} t_IFM_Queue;


/* �L���[�̖{�� */
struct t_IF_Queue {
	uchar	bQueue;					/* �L���[�g�p���t���O */
	uchar	used;					/* ���M���t���O 0:�����M 1:���M�E��M�҂��� */
t_IFM_Queue	*pData;					/* ���f�[�^�ւ̃|�C���^ */
struct t_IF_Queue	*pPrev;			/* �O�ւ̃`�F�C���i�����^�j*/
struct t_IF_Queue	*pNext;			/* ���ւ̃`�F�C���i�����^�j*/
};

/* �L���[�Ǘ���� */
typedef struct {
	ushort	usCount;				/* �L���[�̌� */
struct t_IF_Queue	*pTop;			/* �L���[�̐擪 */
struct t_IF_Queue	*pEnd;			/* �L���[�̖��� */
} t_IF_Queue_ctl;

typedef struct {
	t_IF_Queue_ctl	New;		// �V�K
	t_IF_Queue_ctl	Retry;		// ���g���C
} t_LCMque_Ctrl_Action;			// �L���[�Ǘ����e�[�u��

/************************************
*	���b�N���u�Ǘ��e�[�u���i�e�@�ێ��p�j
*	�E�����ݒ�f�[�^���܂�
*	�E�e�@�̑S�R�^�X�N���g�p����B
*	�E�ΐ��Z�@�ʐM�^�X�N�������ݒ肵�A
*	�E���b�N�Ǘ��^�X�N���������݁A
*	�E�ΐ��Z�@�ʐM�^�X�N/�Ύq�@�ʐM�^�X�N���Q�Ƃ���B
*************************************/
typedef struct {
	uchar	ucBitRateForSlave;		/* DIP-SW���	�ʐMBPS 0=2400/1=9600/2=19200/3=38400 (������������)		*/

	uchar	ucOperation;			/* �����敪�i���Z�@����̂ǂ�ȗv���ɋN�����Ă��邩�j�E�E�E�A������/�ʐM�p */
	BOOL	bTest;					/* �J�e�X�g���쒆�t���O */

	struct {
		ushort	usLineWatch;		/*@�ʐM��M(���)�Ď��^�C�}			�i0�`9999�A10msec�P��	60msec�j*/
                                    /*		���肩��̉�����M���Ď�����^�C�}�[ */
		ushort	usPolling;			/*@POL/SEL�Ԋu						�i0�`9999�A10msec�P��	0msec�j	*/
                                    /*		�e�q�@�ɑ΂��A���̃|�[�����O�𑗐M����܂ł̊Ԋu */
		uchar	ucRetry;			/*@�f�[�^�đ���					�i0�`98��A99�͖�����	3��j*/
                                    /*		NAK�A�������Ȃǂɂ��e�L�X�g�f�[�^�̍đ������������ꍇ�̍đ��� */
		uchar	ucSkipNoAnswer;		/*@�������G���[�����				�i0�`98��A99�͖�����	10��j*/
                                    /* ����A���������Ȃ�A�G���[�B�G���[������͂���|�[�����O���X�L�b�v */
	} toSlave;

//�q�@MAX�P�T��̏��
	struct {						/* �q�@��MAX�P�T��ڑ��\						�q�@�C���f�b�N�X�́A�O����n�܂�B		*/
		struct {					/* ���b�N���u��MAX�U��ڑ��\�i�̈�͂W�䕪�j	���b�N���u�C���f�b�N�X�́A�O����n�܂�B*/
									/*		[0]���b�N���uNo.1													*/
									/*		[6]���b�N���uNo.7�i�\���j											*/
									/*		[7]���b�N���uNo.8�i�\���j											*/
			uchar	ucConnect;		/*@���b�N���u�̐ڑ��i��ԁj													*/
									/*		0�F�ڑ�����															*/
									/*		1�F�ڑ��L��i������Љp�c�G���W�j�A�����O�����b�N���u�i���]�ԁj�j	*/
									/*		2�F�ڑ��L��i������Љp�c�G���W�j�A�����O�����b�N���u�i�o�C�N�j�j	*/
									/*		3�F�ڑ��L��i������Ћg�����쏊�����b�N���u�i���]�ԁA�o�C�N���p�j�j	*/
									/*		4�F�ڑ��L��i�R���Y�W���p�������b�N���u�i���]�ԁj�j					*/
									/*		5�F�ڑ��L��iHID�����b�N���u�i���]�ԁA�o�C�N���p�j�j				*/
//�q�@���痈����
	t_IF_LockSensor	unSensor;		/* ���b�N���u�Z���T�[��ԁi�o�C�i���j										*/
									/*		bit0�F���b�N�J�Z���T�[���	0=OFF/1=ON								*/
									/*		bit1�F���b�N�Z���T�[���	0=OFF/1=ON								*/
									/*		bit2�F�ԗ����m�Z���T�[���	0=OFF/1=ON								*/
									/*		bit3�F���b�N���u������	0=���슮��/1=���쒆			2005.03.04 �����E�ύX */
									/*		bit4�F���b�N���u�������	0=�������/1=�J�������				*/
									/*		bit5�F�\��					0=�Œ�									*/
									/*		bit6�F�\��					0=�Œ�									*/
									/*		bit7�F�\��					0=�Œ�									*/
//���Z�@�֕Ԃ����
			uchar	ucCarState;		/* �ԗ����m���																*/
									/*		0�F�ڑ�����															*/
									/*		1�F�ԗ��L��															*/
									/*		2�F�ԗ��Ȃ�															*/
									/*		3�F�蓮���[�h�ŋ����o��												*/
			uchar	ucLockState;	/* ���b�N���u���															*/
									/*		0�F�����l															*/
									/*	 0xff�F���b�N���u���쒆													*/
									/*			�������̏�Ԃ́A���Z�@�ւ͒ʒm���Ȃ��̂Œ��ӁB				*/
									/*		1�F���b�N���u�ς݁i����j											*/
									/*		2�F���b�N���u�J�ς݁i����j											*/
									/*		3�F����ُ�i�̏�j												*/
									/*		4�F�J����ُ�i�̏�j												*/
									/*		5�F�̏����															*/
									/*		6�F�w�胍�b�N���u�ڑ�����											*/
									/*		7�F�����e�i���X���[�h���i�蓮���[�h���j								*/
									/*			�����̏�Ԃ́A�����ł͕ێ����Ȃ��B���Z�@�ւ͒ʒm����̂Œ��ӁB	*/
									/*		8�F�����e�i���X���[�h�����i�������[�h���j							*/
									/*			�����̏�Ԃ́A�����ł͕ێ����Ȃ��B���Z�@�ւ͒ʒm����̂Œ��ӁB	*/
//���݂̐�����
			uchar	ucOperation;	/* �����敪�i���Z�@����̂ǂ�ȗv���ɋN�����Ă��邩�j						*/
			uchar	ucAction;		/* ���b�N���u���엚���i�A�����쎞�Ɏg�p����B�P������ł͖��Ӗ��H�j			*/
									/*		0�F���̓���v�������Ă��Ȃ�											*/
									/*		1�F�{���i�j����v������											*/
									/*		2�F�J���i�J�j����v������											*/
			uchar	ucActionBack;	/* ���b�N���u�߂������i�����e�i���X�ޔ�p�j			2005.06.07	�����E�ǉ�	*/
									/*		���j�����e�i���X���[�h�I���ŁA���ɖ߂����b�N��������邽�߂ɕK�v	*/
									/*		0�F�߂��Ȃ��i�߂��Ȃ��j												*/
									/*		1�F�{���i�j�����֖߂�												*/
									/*		2�F�J���i�J�j�����֖߂�												*/
			uchar	ucActionMainte;	/* ���b�N���u�����e�i���X���엚��					2005.06.07	�����E�ǉ�	*/
									/*		���j�����e�i���X���[�h�I���ŁA���ɖ߂����b�N��������邽�߂ɕK�v	*/
									/*		0�F�����e���ɁA���̓�������Ă��Ȃ�									*/
									/*		   �܂��́A�����e�I�����A���ɖ߂����b�N����ς�						*/
									/*		1�F�����e���ɁA(�J/���ꂩ��)���삵��								*/
									/*		3�F�����e���Ɂu�����o�Ɂv����										*/
			ushort	usRetry;		/* ���b�N���u�̏�̂��߂̍đ��J�E���^										*/
//�ғ����


			BOOL	bChangeState;	/* ��ԕω��ʒm�ς݃t���O�i�����ݒ�f�[�^��M�Ń��Z�b�g�����j*/

			uchar	ucResult;		/* �J�e�X�g���� */

			ushort	usLockOnTime;	/* ���b�N���u���̐M���o�͎��ԁB�ߋ��̓��쎞��	�i1�`99 150�~���b�P�ʁj		*/
			ushort	usLockOffTime;	/* ���b�N���u�J���̐M���o�͎��ԁB�ߋ��̓��쎞��	�i1�`99 150�~���b�P�ʁj		*/

		} sLock[IFS_ROOM_MAX];

		BOOL	bComm;				/* �q�@�֒ʐM����K�v���邩�H�t���O�i�S�Ẵ��b�N���u���u�ڑ��Ȃ��v��FALSE�j*/
		BOOL	bInitialACK;		/* �q�@�֏����ݒ�f�[�^�𑗂���ACK�ԐM���ꂽ���H�t���O						*/
// ���̃t���O�͐��Z�@��CRB���ʓd���ƂȂ萸�Z�@�݂̂��d���ċN���������ɏ����ݒ�f�[�^�̕ύX���������ꍇ�A���M�̔��������t���O
		BOOL	bInitialsndACK;		/* �q�@�֏����ݒ�f�[�^�𑗂������t���O										*/

		BOOL	bSomeone;			/* �q�@���L�̃��b�N���u�����ꂩ�ЂƂł��u�ڑ���ԁv�ɂ��邩�ۂ��H�t���O	*/
		BOOL	bChange;			/* �q�@���L�̃��b�N���u�����ꂩ�ЂƂł��u��Ԃɕω��v�����������H�t���O	*/

		uchar	ucNakRetry;			/* (NAK�������ꂽ�Ƃ���)�đ��J�E���^										*/
		uchar	ucSkipCount;		/* (�������̂��߂�)�X�L�b�v�J�E���^											*/
		BOOL	bSkip;				/* �X�L�b�v���t���O															*/
									/*		1�F�X�L�b�v���i�������̂��߁j										*/
									/*		0�F�X�L�b�v����														*/
		BOOL	bSkipSave;			/* ��L�t���O�̑ޔ�p								2005.07.26 �����E�ǉ�	*/
		uchar	ucWhatLastCmd;		/* ���O���M�R�}���h															*/
		uchar	ucWhereLastCmd;		/* ���O���M�R�}���h���u���܉����ɂ��邩�H�v�敪								*/
									/*		0�F�����ɂ��Ȃ��i�|�[�����O�̂��߂́u��ԗv���f�[�^�v�̃P�[�X�j		*/
									/*		1�F�D��L���[�ɂ���i�Z���N�e�B���O�ŏ��̃P�[�X�j					*/
									/*		2�F���ԃ{�[�h�ɂ���i�Z���N�e�B���O�đ��̃P�[�X�B�������̂��߁j		*/
	t_IFM_Queue	*psNext;			/* ���ԃ{�[�h��A�|�[�����O/�Z���N�e�B���O�E�R�}���h�ւ̃|�C���^			*/

		ushort	usEntryTimer;		/*@���Ɏԗ����m�^�C�}�[		 (�ԗ����m�m�莞��  ��1�`99 1�b�P��)			*/
		ushort	usExitTimer;		/*@�o�Ɏԗ����m�^�C�}�[		 (�ԗ��񌟒m�m�莞�ԁ�1�`99 1�b�P��)			*/
		ushort	usRetryCount;		/*@���b�N���u����ɂă��g���C�����( "00"�`"98"  ��B"99"���������B���̓���v���܂�) */
		ushort	usRetryInterval;	/*@���b�N���u����ɂă��g���C����Ԋu("001"�`"999" �b)						*/

		uchar	ucContinueCnt;		/* �p���J�E���^�i�o�[�W�����v��/�����Ŏg�p�j*/
		char	cVersion[8];		/* �q�@�o�[�W�����i���ԁj*/

	t_IF_Error	sIFSerror[E_IFS_END];	/* �q�@�G���[��� */
		t_IF_FlatLoopCounter	tLoopCount;		/* ���[�v�J�E���g													*/
		ushort	usFlapUpTime;			/* �t���b�v���u�㏸����														*/
		ushort	usFlapDownTime;			/* �t���b�v���u���~����														*/
	} sSlave[IF_SLAVE_MAX];
	struct{
		BOOL	bComm;				/* �q�@�֒ʐM����K�v���邩�H�t���O�i�S�Ẵ��b�N���u���u�ڑ��Ȃ��v��FALSE�j*/
		uchar	ucWhatLastCmd;		/* ���O���M�R�}���h															*/
		uchar	ucWhereLastCmd;		/* ���O���M�R�}���h���u���܉����ɂ��邩�H�v�敪								*/
		char	cVersion[8];		/* �q�@�o�[�W�����i���ԁj*/
		t_IF_Error	sIFSerror[E_IFS_END];	/* �q�@�G���[��� */
	} sSlave_CRR[IFS_CRR_MAX];		/* CRR�Ǘ��p																*/
	uchar	ucConnect_Tbl[IFS_CRR_MAX][IFS_CRRFLAP_MAX];	// CRR����A�t���b�v��No.���A�^�[�~�i���ԍ��w��

	ushort	usWaitCount;			/* ���b�N����(����)�҂����E�E�E�u��̐e�ɂ��Ă��܂Łv���s����\�Ƃ��� */
									/*	�����́w�����P�x�Ƃ���B�܂�P�ł����슮�����Ă��Ȃ���΁A�҂������ */
	uchar	ucFlapWait;				/* �t���b�v���슮���҂� 1:���슮���҂�										*/
	uchar	ucCommLockCommand;		/* ���b�N���u�R�}���h�����҂����i�q�@���M���x��)							*/
	uchar	ucCommFlapCommand;		/* �t���b�v�R�}���h�����҂����i�q�@���M���x��)								*/
t_IF_Error	sIFMerror[E_IFM_END];	/* �e�@�G���[��� */
} t_IFM_InitData;

/*----------------------------------*/
/*		RAM area define (external)	*/
/*----------------------------------*/
extern	t_IFM_InitData	IFM_LockTable;				/* ���b�N���u�Ǘ��e�[�u���i�����ݒ�f�[�^���܂ށj*/
extern	t_IF_Signal	IF_SignalOut[LOCK_MAKER_END];	/* ���b�N���u���[�J�[�ʂ̐M���o�͎��ԃe�[�u�� */
extern	t_IF_FlapSensor	IFM_FlapSensor[LOCK_IF_MAX];	/* �t���b�v�Z���T��ԁi���[�v�Z���T,�d���Z���T,���b�N���,�t���b�v�Z���T�j*/
extern  t_IF_LockAction IFM_LockAction;

/*----------------------------------*/
/*		function external			*/
/*----------------------------------*/

/*----------------------------------*/
/*			toScomdr.c				*/
/*----------------------------------*/
extern	void	toScomdr_RcvInit(void);		/* toS_1msInt(toStime.c) ���g�� */

/*----------------------------------*/
/*			toSsci.c				*/
/*----------------------------------*/
extern	t_SciRcvCtrl	toS_RcvCtrl;
extern	t_SciSndCtrl	toS_SndCtrl;
extern	unsigned char	toS_RcvBuf[TOS_SCI_RCVBUF_SIZE];
extern	unsigned char	toS_SndBuf[TOS_SCI_SNDBUF_SIZE];

/*----------------------------------*/
/*			IFM.c					*/
/*----------------------------------*/
extern	t_IFM_Queue			toSque_Body_Select[TOS_Q_WAIT_SELECT * 2];	/* �L���[���� */
extern	struct t_IF_Queue	toSque_Buff_Select[TOS_Q_WAIT_SELECT];		/* �L���[�{�� */
extern	t_IF_Queue_ctl		toSque_Ctrl_Select;							/* �L���[�Ǘ���� */

extern	t_IFM_Queue			toSque_Body_Status[LCM_Q_SLAVE_STATUS];	/* �L���[���� */
extern	struct t_IF_Queue	toSque_Buff_Status[LCM_Q_SLAVE_STATUS];	/* �L���[�{�� */
extern	t_IF_Queue_ctl		toSque_Ctrl_Status;						/* �L���[�Ǘ���� */


extern	t_IFM_Queue			LCMque_Body_Action[LCM_Q_WAIT_ACTION];	/* �L���[���� */
extern	struct t_IF_Queue	LCMque_Buff_Action[LCM_Q_WAIT_ACTION];	/* �L���[�{�� */
extern	t_LCMque_Ctrl_Action  LCMque_Ctrl_Action;	// �L���[�Ǘ����

extern	uchar	IFM_GetBPSforSalve(void);

//�L���[�֘A
extern	void				EnQueue(t_IF_Queue_ctl *pCtl, struct t_IF_Queue *pAdd, int iWhere);
extern	struct t_IF_Queue	*GetQueue(t_IF_Queue_ctl *pCtl);
extern	int					CheckQueue(t_IF_Queue_ctl *pCtl, struct t_IF_Queue *pQue, int iWhere);
extern	struct t_IF_Queue	*DeQueue(t_IF_Queue_ctl *pCtl, int iWhere);
extern	struct t_IF_Queue	*GetQueueBuff(struct t_IF_Queue *pQ, ushort byteSize);
extern	t_IFM_Queue			*GetQueueBody(t_IFM_Queue *pQ, ushort byteSize);


//�G���[����/��������
extern	void	IFM_MasterError(int iError, uchar ucNow);
extern	void	IFM_SlaveError(int iError, uchar ucNow, int iSlave);

extern	void	EnQueue2(t_LCMque_Ctrl_Action *pCtl, struct t_IF_Queue *pAdd, int iWhere, BOOL bRetry);
extern	struct t_IF_Queue	*DeQueue2(t_LCMque_Ctrl_Action *pCtl);
extern	struct t_IF_Queue	*RemoveQueue2(t_LCMque_Ctrl_Action *pCtl, int iWhere);
extern	int	MatchRetryQueue_Lock(t_IF_Queue_ctl *pCtl, uchar ucSlave, uchar ucLock);

extern	int Seach_Close_command_sameno(t_IF_Queue_ctl *pCtl,uchar tno);
extern	int Seach_Close_command(t_IF_Queue_ctl *pCtl,ushort scount);
extern	int Get_Close_command_cnt(t_IF_Queue_ctl *pCtl);

/*----------------------------------*/
/*			LCMmain.c				*/
/*----------------------------------*/
extern	void	LCM_SetStartAction(t_IFM_Queue *pMail);	// toScom_Send_or_NextRetry(toScom.c) ���g��

extern	void	LCMtim_1secTimerStop(int iTerm, int iLock);
BOOL	LCM_HasFlapForceControl();

#endif	// _IFM_H_


