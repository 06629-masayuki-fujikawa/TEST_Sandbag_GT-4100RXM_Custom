#ifndef	___RAUCONSTANTH___
#define	___RAUCONSTANTH___
/************************************************************************************/
/*																					*/
/*		�V�X�e�����@:�@RAU�V�X�e��													*/
/*		�t�@�C����	:  rauconstant.h												*/
/*		�@�\		:  �萔�A�}�N����`�t�@�C��										*/
/*																					*/
/************************************************************************************/

#include	"system.h"

#define		RAU_SET			1				/*	SET	*/
#define		RAU_CLR			0				/*	CLR	*/

/****	�[������ԑJ��	****/
#define		S_T_IDLE				0		/* �A�C�h�����			*/
#define		S_T_SEND				1		/* ���M�����҂����		*/
#define		S_T_ACK					2		/* �`�b�j�҂����		*/
#define		S_T_TEXT				3		/* �e�L�X�g��M�����	*/
#define		S_T_BCC					4		/* �a�b�b��M�҂����	*/

/****	�g�n�r�s����ԑJ��	****/
#define		S_H_IDLE				0		/* �A�C�h�����			*/
#define		S_H_SEND				1		/* ���M�����҂����		*/
#define		S_H_ACK					2		/* �`�b�j�҂����		*/
#define		S_H_SOH					3		/* �r�n�g��M�҂����	*/
#define		S_H_TEXT				4		/* �r�s�w��M�҂����	*/
#define		S_H_ETX					5		/* �d�s�a�^�d�s�w��M�҂����	*/
#define		S_H_CRC_1				6		/* �b�q�b��ʃo�C�g��M�҂���� */
#define		S_H_CRC_2				7		/* �b�q�b���ʃo�C�g��M�҂���� */

#define		S_H_BEFOR				10		/* ���f���ڑ��O��� */
#define		S_H_BEFOR_ID			11		/* ���f���ڑ��O��� */
#define		S_H_BEFOR_ID_OK			12		/* ���f���ڑ��O��� */
#define		S_H_BEFOR_ID_NG			13		/* ���f���ڑ��O��� */
#define		S_H_BEFOR_PW			14		/* ���f���ڑ��O��� */
#define		S_H_BEFOR_PW_OK			15		/* ���f���ڑ��O��� */
#define		S_H_BEFOR_PW_NG			16		/* ���f���ڑ��O��� */
#define		S_H_BEFOR_TEL_SEND_WAIT	17		/* ���f���ڑ��O��� */
#define		S_H_BEFOR_TEL_OK_WAIT	18		/* ���f���ڑ��O��� */


/*****	�e���[�N�l�`�w�l	*****/
#define		RAU_DATA_MAX		1448			// ����M�ő�T�C�Y
#define		RAU_RCV_MAX_T		500				/* �[����M�G���A�l�`�w�T�C�Y		*/
#define		RAU_RCV_MAX_H		RAU_DATA_MAX	/* �g�n�r�s��M�G���A�l�`�w�T�C�Y	*/
#define		RAU_SEND_MAX_T		500				/* �[�����M�G���A�l�`�w�T�C�Y		*/
#define		RAU_SEND_MAX_H		RAU_DATA_MAX	/* �g�n�r�s���M�G���A�l�`�w�T�C�Y	*/
#define		RAU_NET_RES_LENGTH_MAX	29


/***  Host������߰�ݸގ�M�Ď��װѴر(�ŐV��2�������܂�)  ***/
#define	HOST_NONE_POL_QUE_SIZE		2				/* ��΂�2�̏搔�ɂ��Ă������� */
#define	HOST_NONE_POL_HAPPEN		1
#define	HOST_NONE_POL_RESTORE		2
#define	HOST_NONE_POL_EVENT_NONE	0
#define	HOST_NONE_POL_ALM_TH		3000			/* �װтƔ��f����گ�����(�� x100ms) */

/***  Term������߰�ݸގ�M�Ď��װѴر	***/
#define	TERM_NONE_POL_ALM_TH		3000			/* �װтƔ��f����گ�����(�� x100ms) */
#define	HOST_SCU_CHECK_TIMING		600				/* �װт������������(�� x100ms) = 1�� */
#define	TEST_RCQ_SIZE	16
#define	TEST_HOST_NONE_POL_COUNTER_60S	((ushort)36000)
#define	TEST_ERROR_COUNT_LIMIT	((ushort)999)

/** ���M�v���d���L���[ **/
#define	RAU_IDB0_QUE_COUNT	16
#define	RAU_IDB0_QUE_SIZE	40

/*------------------------NT-NET�Ή�------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*			T Y P E S														*/
/*--------------------------------------------------------------------------*/

#define	RAU_ARC_SNDQUE_COUNT		4	/* send queue count */
#define	RAU_ARC_ERRQUE_COUNT		20
#define	RAU_COM_ARCBUF_SIZE		(500)	/* �����O�p�P�b�g�Ώ� */

#define	RAU_BLKSIZE				1024		/* 1�p�P�b�g��MAX */

#define	NT_NORMAL_BLKNUM_MAX	26			/* �p�P�b�g�ۑ���MAX */
#define	RAU_SEND_NTBLKNUM_MAX		32		// �����uPC�ւ̍ő呗�M�p�P�b�g��(32block) 2006.08.31:m-onouchi
#define	RAU_RECEIVE_NTBLKNUM_MAX	26		// �����uPC����̍ő��M�p�P�b�g��(26block) 2006.08.31:m-onouchi
#define	NT_DOPANT_HEAD_SIZE		10			/* Dopa�p NT-NET�̃w�b�_�[�T�C�Y */

#define	DOPA_HEAD_SIZE			26			// �c�������p�P�b�g�w�b�_�T�C�Y
#define	DOPA_DATA_SIZE			1420		// �c�������p�P�b�g�f�[�^�T�C�Y
#define	XPORT_DATA_SIZE			1420		// Ether�p�P�b�g�f�[�^�T�C�Y

//------------------------�G���[�R�[�h----------------------------------------
#define	RAU_MODULE_CODE			0x41		// �q�`�t���W���[���R�[�h

#define	RAU_ERR_MODEM_CONNECT_STATE		0x02	// ���f���@�ڑ��E�ؒf�@�X�e�[�^�X
#define	RAU_ERR_DATA_CRC_ERROR			0x05	// �G���[�R�[�h�O�T(�o�b�t�@�f�[�^�b�q�b�G���[)
#define	RAU_ERR_PASSWORD_BREAK			0x09	// �G���[�R�[�h�O�X(�p�X���[�h�j��ɂ�胁����������)
#define	RAU_ERR_BLK_VALID				0x13	// �G���[�R�[�h�P�X(�m�s�|�m�d�s�u���b�N�ԍ��ُ�[����L��])
#define	RAU_ERR_BLK_INVALID				0x14	// �G���[�R�[�h�Q�O(�m�s�|�m�d�s�u���b�N�ԍ��ُ�[���񖳌�])
#define	RAU_ERR_ARCBLK_RETRY_OVER		0x30	// �G���[�R�[�h�S�W(�`�q�b�m�d�s�r���u���b�N�ُ�)
#define	RAU_ERR_BLK_OVER_ARC			0x32	// �G���[�R�[�h�T�O(�m�s�|�m�d�s��M�u���b�N�I�[�o�[)

#define	RAU_ERR_BUFF_FULL_20			0x3C	// �G���[�R�[�h�U�O(���Ƀf�[�^�o�b�t�@�t��)
#define	RAU_ERR_BUFF_FULL_21			0x3D	// �G���[�R�[�h�U�P(�o�Ƀf�[�^�o�b�t�@�t��)
#define	RAU_ERR_BUFF_FULL_22			0x3E	// �G���[�R�[�h�U�Q(���O���Z�f�[�^�o�b�t�@�t��)
#define	RAU_ERR_BUFF_FULL_23			0x3F	// �G���[�R�[�h�U�R(�o�����Z�f�[�^�o�b�t�@�t��)
#define	RAU_ERR_BUFF_FULL_30_41			0x40	// �G���[�R�[�h�U�S(�s���v�W�v�f�[�^�o�b�t�@�t��)
#define	RAU_ERR_BUFF_FULL_120			0x41	// �G���[�R�[�h�U�T(�G���[�f�[�^�o�b�t�@�t��)
#define	RAU_ERR_BUFF_FULL_121			0x42	// �G���[�R�[�h�U�U(�A���[���f�[�^�o�b�t�@�t��)
#define	RAU_ERR_BUFF_FULL_122			0x43	// �G���[�R�[�h�U�V(���j�^�f�[�^�o�b�t�@�t��)
#define	RAU_ERR_BUFF_FULL_123			0x44	// �G���[�R�[�h�U�W(���샂�j�^�f�[�^�o�b�t�@�t��)
#define	RAU_ERR_BUFF_FULL_131			0x45	// �G���[�R�[�h�U�X(�R�C�����ɏW�v���v�f�[�^�o�b�t�@�t��)
#define	RAU_ERR_BUFF_FULL_133			0x46	// �G���[�R�[�h�V�O(�������ɏW�v���v�f�[�^�o�b�t�@�t��)
#define	RAU_ERR_BUFF_FULL_236			0x47	// �G���[�R�[�h�V�P(���ԑ䐔�f�[�^�o�b�t�@�t��)
#define	RAU_ERR_BUFF_FULL_237			0x48	// �G���[�R�[�h�V�Q(���䐔�E���ԃf�[�^�o�b�t�@�t��)
#define	RAU_ERR_BUFF_FULL_126			0x49	// �G���[�R�[�h�V�R(���K�Ǘ��f�[�^�o�b�t�@�t��)

//------------------------ARC�u���b�N�`�F�b�N����------------------------------

#define	RAU_ARCBLK_NORMAL				0x00		// ���� 
#define	RAU_ARCBLK_SEND_END				0x01		// ARC�u���b�N�i���o�[�ُ� �đ��Ȃ� 
#define	RAU_ARCBLK_SEND_RETRY			0x81		// ARC�u���b�N�i���o�[�ُ� �đ��v�� 
#define	RAU_ARCBLK_INVALID				0x85		// NT�u���b�N�i���o�[�ُ� ���񖳌� 
#define	RAU_ARCBLK_VALID				0x86		// NT�u���b�N�i���o�[�ُ� ����L�� 
#define	RAU_ARCBLK_HOST_BUSY			0x90		// �z�X�g����̃R�l�N�V�������ɂ��f�[�^��M�s�� 

#define	RAU_DATA_ID			0x		// 

// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//#define		RAU_ID_LOGIDLIST_COUNT		17			// ���OID���X�g�f�[�^��
#define		RAU_ID_LOGIDLIST_COUNT		18			// ���OID���X�g�f�[�^��
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)

// IBW�����M�������M�d���̎��ʌ���
enum {
	RAU_ARC_NORMAL_PACKET,
	RAU_ARC_FREE_PACKET,
	RAU_ARC_BLOCK_CHECK_PACKET,
	RAU_ATC_UNKOWN_PACKET
};

enum {	// ���M�V�[�P���X�t���O
	RAU_SND_SEQ_FLG_NORMAL,			// �O�F�g�n�r�s�ւ̃e�[�u�����M�ҋ@���
	RAU_SND_SEQ_FLG_TABLE_SEND		// �P�F�g�n�r�s�ւ̃e�[�u�����M�J�n���(�h�a�v����̃e�[�u���f�[�^��M�֎~)
};
enum {	// ��M�V�[�P���X�t���O
	RAU_RCV_SEQ_FLG_NORMAL,			// �O�F�ʏ���(�h�a�v����̗v���f�[�^��M����)
	RAU_RCV_SEQ_FLG_WAIT,			// �P�F�g�n�r�s����̗v���f�[�^��M�����҂����(�h�a�v����̃X���[�f�[�^��M�֎~)
	RAU_RCV_SEQ_FLG_THROUGH_RCV,	// �Q�F�g�n�r�s����̗v���f�[�^��M�������
	RAU_RCV_SEQ_FLG_THROUGH_SEND	// �R�F�g�n�r�s�ւ̃X���[�f�[�^���M�J�n���(�h�a�v����̃X���[�f�[�^��M�֎~)
};

// �e�[�u���f�[�^�ݒ莞�̖߂�l 
typedef enum {
	RAU_DATA_NORMAL,							// ���� 
	RAU_DATA_BLOCK_INVALID1,					// �u���b�N�i���o�[�ُ�ɂ��f�[�^�j���i���񖳌��j 
	RAU_DATA_BLOCK_INVALID2,					// �u���b�N�i���o�[�ُ�ɂ��f�[�^�j���i����L���j 
	RAU_DATA_CONNECTING_FROM_HOST,				// �z�X�g����̐ڑ����̂��ߎ󂯕t���s�i�f�[�^���M�v���j 
	RAU_DATA_BUFFERFULL							// �o�b�t�@����t
}eRAU_TABLEDATA_SET;

enum {
	RAU_NEARFULL_NORMAL,
	RAU_NEARFULL_NEARFULL,
	RAU_NEARFULL_CANCEL		// 2:���� #001
};

typedef struct {					/* send data queue 	*/
	ushort	Length;					/* data length 		*/
	uchar	Data[500];				/* data 			*/
} t_RauQue;

typedef t_RauQue T_RAU_ARCBLK;

typedef struct {
	ushort	Count; 		         	/* regist count */
	ushort	R_Index;	  	        /* next read offset index */
	ushort	W_Index;    	  	    /* next write offset index */
} t_RauQueCtrl;

typedef struct {
	uchar	BlkSts;					/* ARC block number */
	uchar	BlkNo;					/* ARC block status */
	ushort	Length;					/* data length 		*/
	uchar	Data[500];				/* data */
} t_RauSndQue;

typedef	struct {
	uchar	ModuleCode;				/* module code */
	uchar	ErrCode;				/* error code */
	uchar	f_Occur;				/* 1=occured , 0=release */
} t_RauErrCode;

typedef struct{
	uchar	OiBan;					/* sequence number */
	uchar 	ModuleCode;				/* module code */
	uchar 	ErrCode;				/* error code */
	uchar 	f_Occur;				/* 1=occuerd, 0=released */
} t_RauID65data;


/* �o�b�t�@ */
typedef struct {
	short			len;							/* �f�[�^���T�C�Y */
	uchar			data[RAU_BLKSIZE];				/* �o�b�t�@�{�� */
}T_RAU_BLKDATA;

/* ARC�ł�NT-NET�u���b�N�f�[�^�̎�M */
typedef struct {
	BOOL			waitblk;		/* ��M�҂��u���b�NNo. */
	T_RAU_BLKDATA	blkdata;		/* ��M�����u���b�N�f�[�^ */
	int				retry;			/* �u���b�NNo.�ُ�A�������� */
}T_RAU_ARC_RCVBLK;

typedef struct {
	ushort			len;
	uchar			data[1024];
}T_RAU_NTNETBLK;

#define	RAUARC_NODATA	-1

typedef struct {
	int				arcblknum;		/* ���MARC�u���b�N��(1�`2:RAUARC_NODATA=�f�[�^����) */
	int				snd_arcblknum;	/* ���M�ς�ARC�u���b�N��(0�`2:2=�SARC�u���b�N���M������) */
	int				ntblknum;		/* ���MNT-NET�u���b�N��(1�`26:RAUARC_NODATA=�f�[�^����) */
	int				snd_ntblknum;	/* ���M�ς�NT-NET�u���b�N��(0�`26:26=�SNT-NET�u���b�N���M������) */
	T_RAU_NTNETBLK	ntblk;			/* 1NT-NET�u���b�N�擾�p���[�N�̈� */
	int				retry;			/* ���g���C�� */
	int				ntblkno_retry;	/* NT-NET�u���b�NNo.�ُ�ɂ�鑗�M���g���C�� */
	T_RAU_ARCBLK	arcblk[2];		/* ���M�f�[�^ */
}T_RAU_ARC_SNDBLK;

#endif	/* ___RAUCONSTANTH___ */
