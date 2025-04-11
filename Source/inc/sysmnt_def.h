#ifndef	___SYSMNT_DEFH___
#define	___SYSMNT_DEFH___
/*[]----------------------------------------------------------------------[]
 *|	filename: sysmnt_def.h
 *[]----------------------------------------------------------------------[]
 *| summary	: �V�X�e�������e�i���X���ʒ�`
 *| author	: machida.k
 *| date	: 2005.07.01
 *| update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

#include "common.h"

/*=====================================================================================< sysmntsci.c >====*/

/*--------------------------------------------------------------------------*/
/*			D E F I N I T I O N S											*/
/*--------------------------------------------------------------------------*/

#define	STX						0x02
#define	ETX						0x03
#define	EOT						0x04
#define	ENQ						0x05
#define	ACK						0x06
#define	NAK						0x15

#define	SEL						0xFE
#define	POL						0xFF

/* �����ԃ^�C���A�E�g */
#define	SYSMNT_SCI_C2C_TIMEOUT	2

#define	SYSMNT_TIMER_STOP		0x8000

/* SCI�p�P�b�g�w�b�_���T�C�Y */
#define	SYSMNT_SCI_HEADER_SIZE	9	/* stx+id1+id2+id3+id4 */

/* SCI�p�P�b�g�f�[�^���T�C�Y */
#define	SYSMNT_DATALEN_MAX		2048

/* �ő�p�P�b�g�� */
#define	SYSMNT_SCI_PKTLEN_MAX	(SYSMNT_SCI_HEADER_SIZE+SYSMNT_DATALEN_MAX+1+2)	/* "+1+2" = etx+bcc */

/* �o�C�i���ϊ���̍ő�f�[�^�� */

/* REMARK : ----------------------------
     AppServ_ConvParam2CSV_Init()�͉��LSYSMNT_DATALEN_MAX��1112�ȏ��
     ���邱�Ƃ�O��Ƃ��Ă��邽�߁A"SYSMNT_DATALEN_MAX" ������ȉ��ƂȂ�ʂ�
     �����ӂ��邱��
 --------------------------------------- */

/* SCI�G���[ */
enum {
	SYSMNT_SCI_ERR_OVERRUN,
	SYSMNT_SCI_ERR_FRAME,
	SYSMNT_SCI_ERR_PARITY
};

typedef void (*SYSMNT_SCI_FUNC)(uchar);

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/

void	SYSMNTsci_Init(SYSMNT_SCI_FUNC RcvFunc, SYSMNT_SCI_FUNC StsFunc);
void	SYSMNTsci_Start(void);
BOOL	SYSMNTsci_SndReq(uchar *buf, int len);
BOOL	SYSMNTsci_IsSndCmp(void);
void	SYSMNTsci_Stop(void);
void	SYSMNTsci_PC_Start(void);

void	SYSMNT_Int_ERI( void );
void	SYSMNT_Int_RXI( void );
void	SYSMNT_Int_TXI( void );
/*	NOTE:
	SYSMNT_Int_xxx�͂P���̃n���h���ł͂Ȃ��̂Ń|�[�g�ԍ����֐����Ɏ��K�v���Ȃ��B
	�����\�[�X�Ƃ̌݊��̂��߁A�ȉ��̂悤�ɍĒ�`�����Ă����B
*/
#define	SYSMNT_Int_ERI2		SYSMNT_Int_ERI
#define	SYSMNT_Int_RXI2		SYSMNT_Int_RXI
#define	SYSMNT_Int_TXI2		SYSMNT_Int_TXI

/*=====================================================================================< sysmntcomdr.c >====*/

/*--------------------------------------------------------------------------*/
/*			D E F I N I T I O N S											*/
/*--------------------------------------------------------------------------*/

/* ����M�^�C���A�E�g */
#define	_SYSMNT_COMM_TIMEOUT		5000	/* [msec] */

/* �v���g�R����` */
#define	_SYSMNT_NOT_LAST_BLK		0x00
#define	_SYSMNT_LAST_BLK			0x01
#define	_SYSMNT_UPLOAD				0xA1
#define	_SYSMNT_DOWNLOAD_REQ		0xA2
#define	_SYSMNT_DOWNLOAD			0xB1
#define	_SYSMNT_FILE_EXIST			0x00
#define	_SYSMNT_NO_FILE				0x81
#define	_SYSMNT_PC_ERR				0x8F
#define	_SYSMNT_DATA_VISIBLE		0x10
#define	_SYSMNT_DATA_INVISIBLE		0x00

/* COMDR�X�e�[�^�X */
typedef enum {
	SYSMNT_COMDR_NO_PKT,				/* �A�C�h�� */
	SYSMNT_COMDR_PKT_ERR,				/* �p�P�b�g�t�H�[�}�b�g�ُ팟�o */
	SYSMNT_COMDR_ERR_INVALID_LEN,		/* �f�[�^���ُ팟�o */
	SYSMNT_COMDR_ERR_SCI,				/* SCI�G���[ */
	SYSMNT_COMDR_BCC_ERR,				/* BCC�G���[ */
	SYSMNT_COMDR_PKT_RCVD				/* �p�P�b�g��M���� */
}eSYSMNT_COMDR_STS;

/*--------------------------------------------------------------------------*/
/*			S T R U C T U R E S												*/
/*--------------------------------------------------------------------------*/

/* ���M�p�P�b�g�^�C�v */
enum {
	SYSMNT_TYPE_SEL,
	SYSMNT_TYPE_POL,
	SYSMNT_TYPE_STX
};

/* �p�P�b�g(���f�[�^) */
typedef struct {
	int		len;
	uchar	data[SYSMNT_SCI_PKTLEN_MAX];
}T_SYSMNT_SCI_PACKET;

/* �p�P�b�g�w�b�_(BINARY�ϊ���) */
typedef struct {
	uchar	blkno;						/* ID1:�f�[�^�u���b�NNo. */
	uchar	blksts;						/* ID2:�u���b�N�X�e�[�^�X */
	uchar	data_kind;					/* ID3:�f�[�^��� */
	uchar	data_format;				/* ID4:�f�[�^�t�H�[�}�b�g */
}T_SYSMNT_PKT_HEADER;

/* �p�P�b�g�f�[�^(T_SYSMNT_PKT_HEADER::data_format�ɏ]�����ϊ���) */
typedef struct {
	int		len;						/* �f�[�^�� */
	uchar	data[SYSMNT_DATALEN_MAX];	/* �f�[�^ */
}T_SYSMNT_PKT_DATA;

/* �p�P�b�g�f�[�^(�A�v���P�[�V�����p) */
typedef struct {
	uchar					type;		/* �d���擪�̐���R�[�h */
	T_SYSMNT_PKT_HEADER		header;		/* �w�b�_�� */
	T_SYSMNT_PKT_DATA		data;		/* �f�[�^�� */
}T_SYSMNT_PKT;

// �����v�Z�e�X�g �g�p�J�[�h���f�[�^�\��
typedef struct {
	char			kind;				// �J�[�h���
	short			info;				// �J�[�h���
} t_Card_Info;

// �����v�Z�e�X�g ���ʃp�����[�^�ݒ�N������f�[�^�\��
typedef struct {
	char			set;				// ���ʃp�����[�^�ݒ�v��(OFF:�ݒ�v���Ȃ�/ON:�ݒ�v������)
	long			no;					// ���ʃp�����[�^�A�h���X
} t_param_Info;

// �����v�Z�e�X�g ����f�[�^�\��
typedef struct {
	char			mode;				// �e��ʃ��[�h
	char			sub_mode;			// �q��ʃ��[�h
	short			pos;				// �J�[�\���ʒu
	char			nrml_strt;			// �ʏ�N���t���O(OFF:���ʃp�����[�^����̋N��/ON:�ʏ�N��)
	t_Card_Info		last_card;			// �ŏI�g�p�J�[�h���
	t_param_Info	param;				// ���ʃp�����[�^����f�[�^
} t_RT_Ctrl;

/*--------------------------------------------------------------------------*/
/*			E X T E R N A L   V A L I A B L E S								*/
/*--------------------------------------------------------------------------*/

extern ushort	SYSMNTcomdr_c2c_timer;
extern ushort	SYSMNTcomdr_sndcmp_timer;
extern	t_RT_Ctrl	rt_ctrl;			// �����v�Z�e�X�g����f�[�^

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/

void	SYSMNTcomdr_Open(void);
void	SYSMNTcomdr_Reset(void);
eSYSMNT_COMDR_STS	SYSMNTcomdr_GetRcvPacket(T_SYSMNT_PKT *buf);
BOOL	SYSMNTcomdr_SendPacket(T_SYSMNT_PKT *buf, int type);
BOOL	SYSMNTcomdr_SendByte(uchar c);
BOOL	SYSMNTcomdr_IsSendCmp(BOOL *timeout);
void	SYSMNTcomdr_Close(void);

/*=====================================================================================< systemmnt.c >====*/

/*--------------------------------------------------------------------------*/
/*			D E F I N I T I O N S											*/
/*--------------------------------------------------------------------------*/

/* Mifare���s��� �y�[�W��MAX */
#define	ISSMIF_PAGE_MAX	3

/* Mifare���s��� �e�y�[�W���̕\������ */
enum {	/* page 0 */
	ISSMIF_CARD_TYPE,				/* �J�[�h�^�C�v */
	ISSMIF_PARKING_NO,				/* ���ԏ�No. */
	ISSMIF_TEIKI_NO,				/* ���No. */
	ISSMIF_TEIKI_KIND,				/* ������ */
	ISSMIF_INOUT_STS,				/* ���o�ɃX�e�[�^�X */
	ISSMIF_CHARGE_KIND,				/* ������� */
	ISSMIF_CURSORMAX_PAGE0
};
enum {	/* page 1 */
	ISSMIF_UPDATE_DATETIME,			/* �X�V�N�������� */
	ISSMIF_IN_DATETIME,				/* ���ɔN�������� */
	ISSMIF_PAY_DATETIME,			/* ���Z�N�������� */
	ISSMIF_OUT_DATETIME,			/* �o�ɔN�������� */
	ISSMIF_START_DATE,				/* �L���J�n�N���� */
	ISSMIF_END_DATE,				/* �L���I���N���� */
	ISSMIF_CURSORMAX_PAGE1
};
enum {	/* page 2 */
	ISSMIF_AREA_INFO,				/* ����� */
	ISSMIF_AREA_NO,					/* ���No. */
	ISSMIF_PARTITION_NO,			/* �Ԏ��ԍ� */
	ISSMIF_RACK_TYPE,				/* ���b�N�^�C�v */
	ISSMIF_RACK_POS,				/* ���b�N�ʒu */
	ISSMIF_CURSORMAX_PAGE2
};

/* Mifare���s��� t_IssMIF_Ctrl::mifare_status */
enum {
	ISSMIF_CARD_REQ,			/* "�J�[�h���������Ă�������" */
	ISSMIF_WRITING,				/* "���������ł���" */
	ISSMIF_OK,					/* "�������n�j�ł���" */
	ISSMIF_WRITE_NG,			/* "�������m�f�ł���" */
	ISSMIF_AUTH_NG,				/* "���F�؂m�f�ł���" */
	ISSMIF_EDIT,					/* �f�[�^�ҏW�� */
	ISSMIF_USRMNT				/* ���[�U�����e�i���X���f�[�^�m�F��� */
};

/*--------------------------------------------------------------------------*/
/*			S T R U C T U R E S												*/
/*--------------------------------------------------------------------------*/

/* Mifare���s��� �f�[�^�ۑ��p���[�N�̈� */
typedef struct {
	ulong			data[ISSMIF_PAGE_MAX][ISSMIF_CURSORMAX_PAGE0];
	/* �ȉ���ulong�ŃI�[�o�t���[���鐔�l�f�[�^ */
	uchar			card_no[10];						/* �J�[�h�ԍ� */
	uchar			in_datetime[10];					/* ���ɔN�������� */
	uchar			pay_datetime[10];					/* ���Z�N�������� */
	uchar			out_datetime[10];					/* �o�ɔN�������� */
	uchar			ope_datetime[10];					/* �����N�������� */
	uchar			update_datetime[10];				/* �X�V�N�������� */
}t_IssMIF_Data;

/* Mifare���s��� ����f�[�^ */
typedef struct {
	int				operation;			/* ��Mifare������� */
	int				page;				/* �\���y�[�W */
	int				cursor;				/* ���݃J�[�\���ʒu */
	ulong			cursor_val1;		/* �J�[�\���ʒu�̕\���l */
	uchar			cursor_val2[10];	/* ulong�ŃI�[�o�t���[(10��)���鐔�l�f�[�^ */
	uchar			input;				/* �J�[�\���ړ���TEN�L�[�����́�0 */
	t_IssMIF_Data	*mifare;			/* Mifare�f�[�^�o�b�t�@ */
}t_IssMIF_Ctrl;


/*--------------------------------------------------------------------------*/
/*			E X T E R N A L   V A L I A B L E S								*/
/*--------------------------------------------------------------------------*/

extern	ushort	SysMnt_RcvTimer;

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/
typedef void (*SYSMNT_MIFDSP_FUNC)(t_IssMIF_Ctrl *);

void	issmif_InitCtrl(t_IssMIF_Ctrl *ctrl, int ope);
void	issmif_GetAsciiDateTime(uchar *datetime, ushort year, uchar month, uchar day, uchar hour, uchar min);
void	issmif_DisplayPage0(t_IssMIF_Ctrl *ctrl);
void	issmif_DisplayPage1(t_IssMIF_Ctrl *ctrl);
void	issmif_DisplayPage2(t_IssMIF_Ctrl *ctrl);
void	issmif_DisplayPage3(t_IssMIF_Ctrl *ctrl);
void	issmif_DisplayDiscountPage(t_IssMIF_Ctrl *ctrl);
#define	_issmif_DisplayPage4	issmif_DisplayDiscountPage
#define	_issmif_DisplayPage5	issmif_DisplayDiscountPage
#define	_issmif_DisplayPage6	issmif_DisplayDiscountPage

ushort	SysMntLogoDataReg(void);					// logo�o�^
void	SysMntLogoDataRegDsp(const uchar *title);		// �\��

#endif	/* ___SYSMNT_DEFH___ */

