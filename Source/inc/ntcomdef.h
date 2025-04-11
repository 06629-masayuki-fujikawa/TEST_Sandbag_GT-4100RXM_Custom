// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
// MH364300 GG119A03 (S) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
	// UT4000�� "ntcomdef.h"���x�[�X��FT4000�̍������ڐA���đΉ�
// MH364300 GG119A03 (E) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
#ifndef	___NTCOMDEFH___
#define	___NTCOMDEFH___
/*[]----------------------------------------------------------------------[]
 *|	filename: ntdef.h
 *[]----------------------------------------------------------------------[]
 *| summary	: NT�^�X�N��`�f�[�^�w�b�_
 *| date	: 2013-02-22
 *| update	:
 *[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
#include	"system.h"
#include	"common.h"
#include	"mem_def.h"


/*--------------------------------------------------------------------------*/
/*			C O N S T A N T S												*/
/*--------------------------------------------------------------------------*/

/*----------------------------------------- DATA --*/

#define	NT_BLKSIZE				1024		/* 1�p�P�b�g��MAX */

#define	NT_NORMAL_BLKNUM_MAX	26			/* �p�P�b�g�ۑ���MAX */
#define	NT_PRIOR_BLKNUM_MAX		1			/* #007 */
#define	NT_BROADCAST_BLKNUM_MAX	1			/* ����p�P�b�g�ۑ���MAX */

#define	NT_ARC_ERRQUE_COUNT		20

typedef enum {
	NT_NORMAL,
	NT_ABNORMAL
}eNT_STS;

/* �f�[�^��� */
typedef enum {
	NT_DATA_KIND_NORMAL_SND,
	NT_DATA_KIND_NORMAL_RCV,
	NT_DATA_KIND_PRIOR_SND,
	NT_DATA_KIND_PRIOR_RCV,
	NT_DATA_KIND_BROADCAST_SND,
	NT_DATA_KIND_BROADCAST_RCV
}eNT_DATA_KIND;

/* �f�[�^�Z�b�g�֐��̖߂�l */
typedef enum {
	NT_DATA_NORMAL,							/* ���� */
	NT_DATA_BUFFERFULL,						/* �o�b�t�@����t or NT_NORMAL_BLKNUM_MAX�҂��Ă��ŏI�u���b�N�����Ȃ� */
	NT_DATA_BLOCK_INVALID1,					/* ACK02 */
	NT_DATA_BLOCK_INVALID2,					/* ACK03 */
	NT_DATA_NO_MORE_TELEGRAM				/* �o�b�t�@�Ɋ��ɓd���L�� */
}eNT_DATASET;

/* �u���b�N�V�[�N�֐��̃p�����[�^ */
typedef enum {
	NT_SEEK_SET,							/* �d���擪���N�_ */
	NT_SEEK_CUR,							/* ���݈ʒu���N�_ */
	NT_SEEK_END								/* �d���I�[���N�_ */
}eNT_SEEK;

/*----------------------------------------- PROT --*/

//#define	NT_TERMINALNUM_MAX		8			/* �V���A�����̐ڑ��[����MAX */

/* ���/�]�� */
//#define	NT_MASTER				0
#define	NT_SLAVE				1

/* �d�����ł̃I�t�Z�b�g */
#define	NT_OFS_DATA_SIZE_HI				5			/* �]������ */
#define	NT_OFS_DATA_SIZE_LOW			6			/* �]������ */
#define	NT_OFS_SEND_REQ_FLAG			7			/* ���M�v���t���O */
#define	NT_OFS_DIRECTION				7			/* �]������ */
#define	NT_OFS_TERMINAL_NO				8			/* �]���� �[��No. */
#define	NT_OFS_SLAVE_TERMINAL_NO1		9			/* ���ʓ]���p�[��No.(1) */
#define	NT_OFS_SLAVE_TERMINAL_NO2		10			/* ���ʓ]���p�[��No.(2) */
#define	NT_OFS_LINK_NO					11			/* �ʐM�����NNo. */
#define	NT_OFS_TERMINAL_STS				12			/* �[���X�e�[�^�X */
#define	NT_OFS_BROADCASTED_FLAG			13			/* �����M�ς݃t���O */
#define	NT_OFS_DATASIZE_WITH_ZERO		14			/* �[���J�b�g����O�̃f�[�^�T�C�Y */
#define	NT_OFS_PACKET_MODE				16			/* �p�P�b�g�D�惂�[�h */
#define	NT_OFS_TELEGRAM_KIND			18			/* �d�����(STX/ACK/NACK/ENQ/EOT) */
#define	NT_OFS_RESPONSE_DETAIL			19			/* �����ڍ� */
#define	NT_OFS_BLOCK_NO					20			/* �u���b�NNo. */
#define	NT_OFS_LAST_BLOCK_FLAG			21			/* �ŏI�u���b�N���� */
#define	NT_OFS_SYSTEM_ID				22			/* �V�X�e��ID */
#define	NT_OFS_DATA_KIND				23			/* �f�[�^��� */
#define	NT_OFS_BUFFERING_FLAG			24			/* �f�[�^�ێ��t���O */

// MH586504 2009/05/08 (s) okuda ���v�f�[�^�~���b�Ή�
/* �V���v�f�[�^(ID=119/229)�d����� */
#define	NT_TMPKT_ID1					119			/* ���v�f�[�^ �d����� */
#define	NT_TMPKT_ID2					229			/* ���v�f�[�^ �d����� */
#define	NT_TMPKT_OLD_TYPE_PKT_DATA_LEN	33			/* ���v�f�[�^ ���d�����i�`���f�[�^�����j */
#define	NT_TMPKT_NEW_TYPE_PKT_DATA_LEN	41			/* ���v�f�[�^ �V�d�����i�`���f�[�^�����j */
#define	NT_TMPKT_NEW_TYPE_PKT_TOTAL_LEN	(NT_TMPKT_NEW_TYPE_PKT_DATA_LEN + 25)
													/* ���v�f�[�^ �V�d�����i���f�[�^���F�w�b�_�܂ށj */
#define	NT_OFS_DATA_TMPKT_HOSEI			58			/* ���v�f�[�^(ID=119/229) �␳�l */
#define	NT_OFS_DATA_TMPKT_FREETIMER		62			/* ���v�f�[�^(ID=119/229) ��M���t���[�^�C�}�[�l�Z�b�g�G���A */

// MH586504 2009/05/08 (e) okuda ���v�f�[�^�~���b�Ή�

/* �]������ */
#define	NT_DIRECTION_TO_MASTER			0x00		/* ���ʁ���� */
#define	NT_DIRECTION_TO_SLAVE			0x01		/* ��L������ */
/* �]���� �[��No. */
#define	NT_ALL_TERMINAL					0x00		/* �S�[����(����ł͂Ȃ�)���M����f�[�^ */
#define	NT_TERMINAL_UNKOWN				0xF0		/* �[��No.�s��(�G���[�f�[�^�̃p�����[�^�Ƃ��Ă̂ݎg�p) */
#define	NT_BROADCAST					0xFF		/* ����Z���N�e�B���O�̃f�[�^ */
/* ���ʓ]���p�[��No.(1) */
/* ���ʓ]���p�[��No.(2) */
/* �ʐM�����NNo. */
/* �[���X�e�[�^�X */
#define	NT_BUFFER_NORMAL				0x00		/* ���� */
#define	NT_NORMAL_BUFFER_FULL			0x01		/* bit0   :�ʏ��M�o�b�t�@FULL�̂��ߒʏ�f�[�^��M�s��� */
#define	NT_PRIOR_BUFFER_FULL			0x02		/* bit1   :�D���M�o�b�t�@FULL�̂��ߗD��f�[�^��M�s��� */
#define	NT_ALL_BUFFER_FULL				0x03		/* bit1-2 :�ʏ�A�D���M�o�b�t�@FULL�̂��߁A�f�[�^��M�s��� */
/* �����M�ς݃t���O */
#define	NT_NOT_BROADCASTED				0x00		/* �Ȃ� */
#define	NT_BROADCASTED					0x01		/* �O��̒ʐM�œ���Z���N�e�B���O�Ńf�[�^��M */
#define	NT_BROADCASTED_CRC_ERR			0x81		/* ����Z���N�e�B���O�f�[�^��CRC�G���[�B(�����ڍׂ�00H�ő��M) */
/* �[���J�b�g����O�̃f�[�^�T�C�Y */
/* �p�P�b�g�D�惂�[�h */
#define	NT_NORMAL_DATA					0x00		/* �ʏ�f�[�^ */
#define	NT_PRIOR_DATA					0x01		/* �D��f�[�^ */
/* �d�����(STX/EOT/ENQ/ACK/NAC) */
#define	STX								0x02
#define	EOT								0x04
#define	ENQ								0x05
#define	ACK								0x06
#define	NAK								0x15
/* �����ڍ� */
#define	NT_RES_NORMAL					0x00		/* ���� */
#define	NT_RES_CRC_RETRY_OVER			0x01		/* ��MSTX�u���b�N��CRC�ُ��M���g���C�I�[�o�[�̂��߂ɋ���ACK���� */
#define	NT_RES_BLOCKNO_ERR1				0x02		/* STX�̃u���b�NNo.�ُ��M (��M�f�[�^����) */
#define	NT_RES_BLOCKNO_ERR2				0x03		/* STX�̃u���b�NNo.�ُ��M (��M�f�[�^�L��) */
#define	NT_RES_CRC_ERR					0x80		/* CRC�G���[ */
#define	NT_RES_BUFFER_FULL				0x81		/* ��M�o�b�t�@FULL */
#define	NT_RES_SEQUENCE_ERR				0x82		/* �V�[�P���X�ُ�ŋ����ؒf */


/* �u���b�NNo. */
/* �ŏI�u���b�N���� */
#define	NT_NOT_LAST_BLOCK				0x00		/* ���ԃu���b�N */
#define	NT_LAST_BLOCK					0x01		/* �ŏI�u���b�N */
/* �V�X�e��ID */
/* �f�[�^��� */
/* �f�[�^�ێ��t���O */
#define	NT_DO_BUFFERING					0x00		/* �o�b�t�@�����O���� */
#define	NT_NOT_BUFFERING				0x01		/* �o�b�t�@�����O���Ȃ� */


/*----------------------------------------- SCI --*/

/* SCI�ʐM�G���[ */
enum {
	NT_SCI_ERR_OVERRUN,						/* �I�[�o�[�����G���[ */
	NT_SCI_ERR_FRAME,						/* �t���[�~���O�G���[ */
	NT_SCI_ERR_PARITY						/* �p���e�B�G���[ */
};

#define	NT_SCI_DATABIT					0	/* �f�[�^�r�b�g��	0=8bits, 1=7bits */
#define	NT_SCI_STOPBIT					0	/* �X�g�b�v�r�b�g��	0=1bit , 1=2bits */
#define	NT_SCI_PARITY					0	/* �p���e�B���		0=none , 1=odd  ,2=even */

/*----------------------------------------- COMDR --*/

/* COMDR�X�e�[�^�X */
typedef enum {
	NT_COMDR_NO_PKT,						/* �A�C�h�� */
	NT_COMDR_PKT_RCVING,					/* �p�P�b�g��M�� */
	NT_COMDR_PKT_RCVD,						/* �p�P�b�g��M���� */
	NT_COMDR_ERR_SCI,						/* �ʐM�G���[(�I�[�o�[�����G���[/�t���[�~���O�G���[/�p���e�B�G���[) */
	NT_COMDR_ERR_INVALID_LEN,				/* �p�P�b�g�f�[�^���s�� */
	NT_COMDR_ERR_TIMEOUT					/* �L�����N�^�ԃ^�C���A�E�g */
}eNT_COMDR_STS;

/* �p�P�b�g���M���� */
typedef enum {
	NT_COMDR_SND_NORMAL,					/* ���M���� */
	NT_COMDR_SND_TIMEOUT,					/* ���M�^�C���A�E�g���� */
	NT_COMDR_SND_ERR						/* ���M�s�� */
}eNT_COMDR_SND;

#define	NT_C2C_TIMINT			1			/* �L�����N�^�ԃ^�C���A�E�g�Ď��^�C�}�̃C���^�[�o�� */
#define NT_PKT_SIG				"NTCOM"		/* �p�P�b�g�擪�̃V�O�l�`�� */
#define	NT_PKT_HEADER_SIZE		7			/* �V�O�l�`���{�f�[�^�� */
#define	NT_PKT_CRC_SIZE			2			/* CRC�f�[�^�T�C�Y */

/*----------------------------------------- TIMER --*/

#define	NT_TIM_1MS_CH_MAX		1			/* 1ms�^�C�}�o�^��MAX */
#define	NT_TIM_10MS_CH_MAX		5			/* 10ms�^�C�}�o�^��MAX */
#define	NT_TIM_100MS_CH_MAX		5			/* 100ms�^�C�}�o�^��MAX */

#define	NT_TIM_UNIT_1MS			1
#define	NT_TIM_UNIT_10MS		10
#define	NT_TIM_UNIT_100MS		100

#define	NT_TIM_INVALID			0			/* �^�C�}���o�^ */
#define	NT_TIM_STOP				1			/* �^�C�}��~�� */
#define	NT_TIM_START			2			/* �^�C�}���쒆 */

#define	NT_TIM_1SHOT			1
#define	NT_TIM_CYCLE			2

/*----------------------------------------- MAIN&ARC --*/

#define	NT_ARC_SNDQUE_COUNT		4	/* send queue count */

#define	NT_COM_ARCBUF_SIZE		(500)	/* �����O�p�P�b�g�Ώ� */

#define	NT_MODULE_CODE			63		/* NTCom�[��Ӽޭ�ٺ��� 63=0x3F	*/

/* 7�Z�OLED��� */
typedef enum {
	NT_LED_IDLE = 1,
	NT_LED_SELPOLL
}eNT_LED;

/* IBW�����M�������M�d���̎��ʌ��� #003*/
enum {
	NT_ARC_NORMAL_PACKET,
	NT_ARC_FREE_PACKET,
	NT_ARC_BLOCK_CHECK_PACKET,
	NT_ATC_UNKOWN_PACKET
};

/* �G���[�R�[�h(ARCNET�́u�G���[�f�[�^�v�d���ő��M) */
#define	NT_ERR_NO_RESPONSE			0x01	/* �ʐM�s�� */
#define	NT_ERR_CRC_DATA				0x05	/* CRC16�`�F�b�NNG(�f�[�^) */
#define	NT_ERR_CRC_CODE				0x06	/* CRC16�`�F�b�NNG(�R�[�h) */
#define	NT_ERR_ACK01				0x07	/* CRC�ُ�Ńf�[�^�j�� */
#define	NT_ERR_ACK02				0x08	/* ACK02���g���C�I�[�o�[�Ńf�[�^�j�� */
#define	NT_ERR_BROADCAST_FAIL		0x0A	/* ���񉞓��Ȃ� */
#define	NT_ERR_BLK_ARC2				0x13	/* �u���b�N�ԍ��ُ�(����L��)(ARCNET) �iNT�|NET����ACK03�@�ɑ����j*/
#define	NT_ERR_BLK_ARC1				0x14	/* �u���b�N�ԍ��ُ�(���񖳌�)(ARCNET) �iNT�|NET����ACK02�@�ɑ����j*/
#define	NT_ERR_BLK_NT				0x15	/* �u���b�N�ԍ��ُ�(NT-NET) */
#define	NT_ERR_RETRY_OVER			0x1E	/* ���g���C�񐔃I�[�o�[ */
#define	NT_ERR_ARCBLK_RETRY_OVER	0x30	/* �r���u���b�N�ُ� */
#define	NT_ERR_BLK_OVER_ARC			0x32	/* ��M�u���b�N�I�[�o�[(ARCNET) */
#define	NT_ERR_BLK_OVER_NT			0x33	/* ��M�u���b�N�I�[�o�[(NT-NET) */

/*----------------------------------------- MASTER --*/

/* �|�[�����O�^�Z���N�e�B���O */
//typedef enum {
//	NT_COM_MODE_IDLE,					/* �A�C�h���� */
//	NT_COM_MODE_POLLING,				/* �|�[�����O�� */
//	NT_COM_MODE_SELECTING				/* �Z���N�e�B���O�� */
//}eNT_COM_POLLSEL;

/* ��ǃX�e�[�^�X */
//typedef enum {
//	NT_COM_MASTER_IDOL,				/* �A�C�h��  */
//	NT_COM_MASTER_ENQ_SEND,			/* ENQ���M�� */
//	NT_COM_MASTER_STX_SEND			/* STX���M�� */
//}eNT_COM_STS_MASTER;

/*----------------------------------------- SLAVE --*/

/* �]�ǃX�e�[�^�X */
typedef enum {
	NT_COM_SLAVE_IDOL,				/* �A�C�h�� (ENQ�҂�)*/
	NT_COM_SLAVE_STX_SEND,			/* STX���M�� */
	NT_COM_SLAVE_ACK_SEND,			/* ACK���M�� */
	NT_COM_SLAVE_BROAD_CAST_WAIT	/* �����M�҂� */
}eNT_COM_STS_SLAVE;

/* �]�ǃX�e�[�^�X */
enum {
	SEND_DISABLE = 0,		// �f�[�^�Ȃ��A��ǃo�b�t�@�t�����ő��M�s��
	SEND_ENABLE_PRIOR,		// �D��f�[�^���M�\
	SEND_ENABLE_NORMAL		// �ʏ�f�[�^���M�\
};






/*--------------------------------------------------------------------------*/
/*			T Y P E S														*/
/*--------------------------------------------------------------------------*/

#ifndef BOOL
#define BOOL	uchar
#define TRUE	1
#define	FALSE	0
#endif

/* �^�C�}���[�U�[�p */
typedef struct {
	ushort	timerid;								/* �^�C�}�[ID */
	ushort	timeout;								/* �^�C���A�E�g */
	long	count;									/* [<0] = ������, [0] = �^�C���A�E�g, [>0] = �^�C���A�E�g�Ď��� */
}T_NT_TIMER;

/*----------------------------------------- MAIN --*/

/* �����ݒ�f�[�^(ARC�p�P�b�g�`��) */
typedef struct {
	uchar			ExecMode;						/* ��ǁ^�]�� */
	uchar			TerminalNum;					/* �q�@�ڑ��䐔(��ǎ�)�^���[��No.(�]�ǎ�) */
	uchar			Baudrate;						/* �ʐMBPS */
	uchar			Time_t1[2];						/* ENQ���M��̉����҂�����(t1) */
	uchar			Time_t2[2];						/* ��M�f�[�^�ő厞��(t2) */
	uchar			Time_t3[2];						/* �e�L�X�g���M��̉����҂�����(t3) */
	uchar			Time_t4[2];						/* ���ǂƂ̃C���^�[�o������(t4) */
	uchar			Time_t5[2];						/* �����ԃ^�C�}�[(t5) */
	uchar			Time_t6[2];						/* ����ENQ��̃f�[�^���MWait����(t6) */
	uchar			Time_t7[2];						/* ��M�f�[�^�ő厞��(t7) */
	uchar			Time_t8[2];						/* ���񏈗��O��WAIT����(t8) */
	uchar			Time_t9[2];						/* ���M�E�F�C�g�^�C�}�[(t9) */
	uchar			Retry;							/* �f�[�^���g���C��M�� */
	uchar			BlkSndUnit;						/* �u���b�N���M�P�� */
	uchar			Time_LineMonitoring;			/* �ʐM����Ď��^�C�}�[ */
	uchar			NoResNum[2];					/* �������G���[����� */
	uchar			SkipCycle[2];					/* ���������X�L�b�v���� */
	uchar			Time_ResFromMain;				/* MAIN���W���[������̎�M�����҂����� */
}T_NT_INITIAL_DATA_FRAME;

/* �����ݒ�f�[�^(IBK�ł̕ۑ��`��) */
typedef struct {
	uchar			ExecMode;						/* ��ǁ^�]�� */
	uchar			TerminalNum;					/* �q�@�ڑ��䐔(��ǎ�)�^���[��No.(�]�ǎ�) */
	ushort			Baudrate;						/* �ʐMBPS */
	ushort			Time_t1;						/* ENQ���M��̉����҂�����(t1) */
	ushort			Time_t2;						/* ��M�f�[�^�ő厞��(t2) */
	ushort			Time_t3;						/* �e�L�X�g���M��̉����҂�����(t3) */
	ushort			Time_t4;						/* ���ǂƂ̃C���^�[�o������(t4) */
	ushort			Time_t5;						/* �����ԃ^�C�}�[(t5) */
	ushort			Time_t6;						/* ����ENQ��̃f�[�^���MWait����(t6) */
	ushort			Time_t7;						/* ��M�f�[�^�ő厞��(t7) */
	ushort			Time_t8;						/* ���񏈗��O��WAIT����(t8) */
	ushort			Time_t9;						/* ���M�E�F�C�g�^�C�}�[(t9) */
	uchar			Retry;							/* �f�[�^���g���C��M�� */
	uchar			BlkSndUnit;						/* �u���b�N���M�P�� */
	uchar			Time_LineMonitoring;			/* �ʐM����Ď��^�C�}�[ */
	ushort			NoResNum;						/* �������G���[����� */
	ushort			SkipCycle;						/* ���������X�L�b�v���� */
	uchar			Time_ResFromMain;				/* MAIN���W���[������̎�M�����҂����� */
}T_NT_INITIAL_DATA;

//typedef struct {
//	uchar	Count;
//	uchar	ReadIdx;
//	uchar	WriteIdx;
//	uchar	dummy;
//} t_NTCom_RcvQueCtrl;

/*----------------------------------------- DATA --*/
/* �f�[�^�Ǘ��@�\ ����f�[�^ */
typedef struct {
	BOOL	terminal_exists;						/* �ڑ��[���� */
	eNT_STS	terminal_status;						/* �ʐM��Q���̒[��No.(�Ȃ����-1) */
}T_NTComData_Ctrl;

/* �o�b�t�@���(TRUE=�d���L��) */
typedef struct {
	BOOL			normal_s;						/* �ʏ�f�[�^�o�b�t�@(NT�l�b�g�����M�p) */
	BOOL			normal_r;						/* �ʏ�f�[�^�o�b�t�@(NT�l�b�g�֑��M�p) */
	BOOL			prior_s;						/* �D��f�[�^�o�b�t�@(NT�l�b�g�֑��M�p) */
	BOOL			prior_r;						/* �D��f�[�^�o�b�t�@(NT�l�b�g�����M�p) */
	BOOL			broadcast_s;					/* ����f�[�^�o�b�t�@(NT�l�b�g�֑��M�p) */
	BOOL			broadcast_r;					/* ����f�[�^�o�b�t�@(NT�l�b�g�����M�p) */
}T_NT_BUF_STATUS;

/* �[���ڑ���� */
//typedef struct {
//	uchar			sts[NT_TERMINALNUM_MAX];
//}T_NT_TERMINAL_STATUS;

/* �o�b�t�@�Ɋ܂܂��u���b�N�� */
typedef struct {
	uchar			normal;
	uchar			prior;
}T_NT_BUF_COUNT;

/* �o�b�t�@ */
typedef struct {
	short			len;							/* �f�[�^���T�C�Y */
	uchar			data[NT_BLKSIZE];				/* �o�b�t�@�{�� */
}T_NT_BLKDATA;

/* �f�[�^�o�b�t�@ */
typedef struct {
	eNT_DATA_KIND	kind;							/* �f�[�^��� */
	BOOL			fixed;							/* TRUE=�d���m�� */
	int				blknum;							/* �L���u���b�N�� */
	int				max;							/* ���X�g�̍ő�v�f�� */
	T_NT_BLKDATA	*blkdata;						/* �o�b�t�@�{�� */
}T_NT_BUFFER;

/* �d�����(�f�[�^�Ǘ��N���X�̃��[�U�ł͒l��ς��Ȃ�����) */
typedef struct {
	eNT_DATA_KIND	kind;							/* �f�[�^��� */
	int				terminal;						/* �[���ԍ� */
	int				blknum;							/* �u���b�N�� */
	int				curblk;							/* ���݃u���b�N�ʒu */
/* #006-> */
	uchar			saveflag;						/* �f�[�^�ێ��t���O */
/* <-#006 */
}T_NT_TELEGRAM;

/*  */
typedef struct {
//	#002	��
//	int				top;							/* �����O�o�b�t�@�擪 */
//	int				telenum;						/* �d���� */
	int				read;							/* �����O�o�b�t�@�擪 */
	int				write;							/* �����O�o�b�t�@�I�[ */
//	��	#002
	int				ring[1 + 1];					/* �d�����ݏ��(�[��No.) */
}T_NT_TELEGRAM_LIST;


/* �o�b�t�@�������̎��� */
typedef struct {
	T_NT_BLKDATA	snd_pool_normal[NT_NORMAL_BLKNUM_MAX];			/* �ʏ�f�[�^���M�p */
	T_NT_BLKDATA	rcv_pool_normal[NT_NORMAL_BLKNUM_MAX];			/* �ʏ�f�[�^��M�p */
	T_NT_BLKDATA	snd_pool_prior[NT_PRIOR_BLKNUM_MAX];			/* �D��f�[�^���M�p */
	T_NT_BLKDATA	rcv_pool_prior[NT_PRIOR_BLKNUM_MAX];			/* �D��f�[�^��M�p */
	T_NT_BLKDATA	snd_pool_broadcast[NT_BROADCAST_BLKNUM_MAX];	/* ����f�[�^���M�p */
	T_NT_BLKDATA	rcv_pool_broadcast[NT_BROADCAST_BLKNUM_MAX];	/* ����f�[�^��M�p */
}T_NT_BUFMEM_POOL;

/* �G���[��� */
typedef struct {
	uchar			module;							/* ���W���[���R�[�h */
	uchar			errcode;						/* �G���[�R�[�h */
	uchar			occur;							/* ����(1)/����(0) */
	uchar			terminal;						/* �[��No. */
}T_NT_ERR;

/* �G���[���o�b�t�@ */
typedef struct {
	int				top;							/* �����O�o�b�t�@�擪 */ 
	int				num;							/* �L���v�f�� */
	T_NT_ERR		err[NT_ARC_ERRQUE_COUNT];		/* �o�b�t�@�{�� */
}T_NT_ERR_INFO;

/* #005-> */
/* ��d�ۏ؃f�[�^ */
typedef enum {
	NT_FUKUDEN_SCHEDULER_NONE,						/* ���d�������� */
	NT_FUKUDEN_SCHEDULER_START,						/* �X�P�W���[���̃R�s�[�쐬�� */
	NT_FUKUDEN_SCHEDULER_MADE						/* �X�P�W���[���̃R�s�[�����߂��� */
}eNT_FUKUDEN_SCHEDULER;
typedef struct {
	eNT_FUKUDEN_SCHEDULER	scheduler;				/* �ʏ��M�f�[�^��ARCNET�ւ̑��M�X�P�W���[���̍č\���� */
	int						scheduler_work;			/* �X�P�W���[���̍č\���p���[�N */
	int						scheduler_worksize;		/* scheduler_ring���̗L���f�[�^�� */
}T_NT_FUKUDEN;
/* <-#005 */

/*----------------------------------------- TIMER --*/

/* �^�C�}���� */
typedef struct {
	uchar	sts;			/* �^�C�}�X�e�[�^�X(NTCOM_TIM_CYCLE/NTCOM_TIM_1SHOT/NTCOM_TIM_INVALID/NTCOM_TIM_STOP) */
	void	(*func)(void);	/* �^�C���A�E�g���ɃR�[�������֐� */
	ushort	time;			/* �^�C�}�����l */
	ushort	cnt;			/* �^�C�}���ݒl */
	BOOL	cyclic;			/* TRUE=�T�C�N���b�N�^�C�}, FALSE=1SHOT�^�C�} */
}T_NT_TIM_CTRL;

/* �R�[���o�b�N�֐� */
typedef void (*T_NT_SCI_CALLBACK)(uchar);

/*----------------------------------------- SCI --*/

/* SCI���� */
typedef struct {
	/* ---- port information ---- */
	uchar				port_no;			/* �|�[�g�ԍ� */
/* ART:machida (s) 05-12-20 */
//	struct st_sci		*pport;				/* �|�[�g���䃌�W�X�^�擪�A�h���X */
	volatile struct st_sci	*pport;			/* �|�[�g���䃌�W�X�^�擪�A�h���X */
/* ART:machida (e) 05-12-20 */
	uchar				speed;				/* �{�[���[�g */
	uchar				databit;			/* �f�[�^�r�b�g�� */
	uchar				stopbit;			/* �X�g�b�v�r�b�g�� */
	uchar				parity;				/* �p���e�B */
	/* ---- for sending ---- */
	uchar				*sdata;				/* ���M�f�[�^ */
	ushort				slen;				/* ���M�f�[�^�� */
	ushort				scnt;				/* ���M�ς݃f�[�^�� */
	BOOL				scomplete;			/* ���M�����t���O (1=complete, 0=yet) */
	/* ---- callback functions ---- */
	T_NT_SCI_CALLBACK	rcvfunc;			/* �f�[�^��M�ʒm�p�R�[���o�b�N�֐��|�C���^ */
	T_NT_SCI_CALLBACK	stsfunc;			/* �f�[�^��M�G���[�ʒm�p�R�[���o�b�N�֐��|�C���^ */
}T_NT_SCI_CTRL;

/*----------------------------------------- COMDR --*/

/* COMDR���� */
typedef struct {
	uchar		status;				/* COMDR�@�\�X�e�[�^�X */
	int			r_rcvlen;			/* ��M�d���� */
	int			r_datalen;			/* �L���d���� */
	uchar		r_buf[NT_BLKSIZE];	/* ��M�o�b�t�@ */
	T_NT_TIMER	timer_c2c;			/* �L�����N�^�ԃ^�C���A�E�g */
	T_NT_TIMER	timer_sndcmp;		/* ���M�����^�C���A�E�g */
}T_NT_COMDR_CTRL;

/*----------------------------------------- MAIN&ARC --*/

typedef struct {					/* queue control data */
	uchar	Count;
	uchar	ReadIdx;
	uchar	WriteIdx;
	uchar	dummy;
} t_NT_RcvQueCtrl;

typedef struct {
	ushort	Count; 		         	/* regist count */
	ushort	R_Index;	  	        /* next read offset index */
	ushort	W_Index;    	  	    /* next write offset index */
} t_QueCtrl;

typedef struct {
	uchar	BlkSts;					/* ARC block number */
	uchar	BlkNo;					/* ARC block status */
	ushort	Length;					/* data length 		*/
	uchar	Data[500];				/* data */
} t_SndQue;

/* error inform data for IBW queue data style */
typedef	struct {
	uchar	ModuleCode;				/* module code */
	uchar	ErrCode;				/* error code */
	uchar	f_Occur;				/* 1=occured , 0=release */
	uchar	TerminalNo;				/* number of terminal, error occurred on */
} t_ErrCode;

typedef struct {					/* send data queue 	*/
	ushort	Length;					/* data length 		*/
	uchar	Data[500];				/* data(�����O�p�P�b�g�Ή�: 256 -> 500) */
} t_Que;


typedef t_Que T_NT_ARCBLK;

/* ARC�ł�NT-NET�u���b�N�f�[�^�̎�M */
//typedef struct {
//	BOOL			waitblk;		/* ��M�҂��u���b�NNo. */
//	T_NT_BLKDATA	blkdata;		/* ��M�����u���b�N�f�[�^ */
//	int				retry;			/* �u���b�NNo.�ُ�A�������� */
//}T_NT_ARC_RCVBLK;

/* #004 -> */
typedef struct {
	int				ntblkno_retry;	/* NT-NET�u���b�NNo.�ُ�ɂ�鑗�M���g���C�� */
	T_NT_TELEGRAM	telegram;
}T_NT_SND_TELEGRAM;
/* <- #004 */

typedef struct {					/* communication error data */
	uchar	Full;
	uchar	Comer;
	ushort	OVRcnt;
	ushort	FRMcnt;
	ushort	PRYcnt;
	uchar	dummy;
} t_NT_ComErr;

//typedef union {
typedef struct {
//	T_NT_BLKDATA	ntblk;
//	uchar			arcblk[NT_COM_ARCBUF_SIZE];
	ushort			len;
	uchar			sendData[26000];
}T_NTCom_SndWork;


/*----------------------------------------- SLAVE --*/

// �]�ǊǗ��p�\����
typedef struct {
	eNT_COM_STS_SLAVE	status;			// �]�ǃX�e�[�^�X
	eNT_STS	master_status;				// ��ǃX�e�[�^�X
	uchar	broad_cast_received;		// ����f�[�^��M�ς�
	uchar	block_send_count;			// �u���b�N���M��
	uchar	block_receive_count;		// �u���b�N��M��
	uchar	terminal_num;					// ��ǎ��F�ڑ������^�[�~�i����	�]�ǎ��F���ǔԍ�
	BOOL	receive_packet_crc_status;		// ��M�����p�P�b�g��CRC�G���[���
	uchar	receive_crc_err_count;			// CRC�G���[�̎�M�񐔡�istx�j
	uchar	receive_crc_err_count_ctrl;		// ����d��CRC�G���[�̎�M�񐔡�istx �ȊO�j
	uchar	last_terminal_status;			// ���O�ɑ��M�����[����ԁB

	uchar	send_retry_count;				// ���M���g���C��
	uchar	ack02_receive_count;			// Ack02����M������	�i�w��񐔎�M�œd���폜�j	#001
	uchar	cur_seq_no;						// �V�[�P���X�m���D
	uchar	timeout_kind;					// �^�C���A�E�g���

	T_NT_TIMER	timer_10m;				// 10m sec �^�C�}�[
	BOOL timeout_10m;					// 10m sec �^�C�}�[�@�^�C���A�E�g
	T_NT_TIMER	timer_100m;				// 100m sec �^�C�}�[
	BOOL timeout_100m;					// 100m sec �^�C�}�[�@�^�C���A�E�g
}T_NTCOM_SLAVE_CTRL;


/*----------------------------------------- BUF --*/
// NTCom�^�X�N�ʏ��M�o�b�t�@�i��OPE�w�j
typedef struct {
	uchar	ntDataBuf[NT_NORMAL_BLKNUM_MAX][NT_BLKSIZE];	// NT-NET�f�[�^�o�b�t�@
	ushort	readIndex;										// �Ǎ��݃C���f�b�N�X
	ushort	writeIndex;										// �����݃C���f�b�N�X
	ushort	count;											// �i�[�f�[�^��
	BOOL	fixed;											// TRUE:���M�� FALSE:�ҋ@��
}T_NTCOM_NORMAL_DATABUF;

// NTCom�^�X�N�D���M�o�b�t�@�i��OPE�w�j
typedef struct {
	uchar	ntDataBuf[NT_PRIOR_BLKNUM_MAX][NT_BLKSIZE];		// NT-NET�f�[�^�o�b�t�@
	ushort	readIndex;										// �Ǎ��݃C���f�b�N�X
	ushort	writeIndex;										// �����݃C���f�b�N�X
	ushort	count;											// �i�[�f�[�^��
	BOOL	fixed;											// TRUE:���M�� FALSE:�ҋ@��
}T_NTCOM_PRIOR_DATABUF;


#endif	/* ___NTCOMDEFH___ */
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
