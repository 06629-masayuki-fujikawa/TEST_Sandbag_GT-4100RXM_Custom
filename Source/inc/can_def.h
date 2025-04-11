#ifndef _CAN_DEF_
#define _CAN_DEF_
//[]----------------------------------------------------------------------[]
///	@file		can_def.h
///	@brief		CAN�ʐM�^�X�N�p�w�b�_
///	@date		2012/01/24
///	@author		m.nagashima
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
#include	"can_api.h"

enum {				// �m�[�hID(1h-7Fh)
	NID_RXM	= 1,	// ���C�����
	NID_RXF,		// �t�����g���

	NID_CNT			// �ڑ��m�[�h��
};

#define _MAX_LENGTH_TRANSFERT	(0x7F * 7)	// �u���b�N�]���ő�f�[�^��

enum {
	CAN_NETWORK = 0,
	CAN_EMERGENCY,
	CAN_EVENT,
	CAN_CONTROL,
	CAN_DISPLAY,
	CAN_HEARTBEAT,

	SIDMAX
};

/* Status of the SDO transmission */
#define SDO_RESET						0x0		// Transmission not started. Init state.
#define SDO_FINISHED					0x1		// data are available
#define SDO_ABORTED_RCV					0x80	// Received an abort message. Data not available
#define SDO_ABORTED_INTERNAL			0x85	// Aborted but not because of an abort message.
#define SDO_INITIATE_DOWNLOAD_REQUEST	0x2
#define SDO_DOWNLOAD_IN_PROGRESS		0x3
#define SDO_UPLOAD_IN_PROGRESS			0x4
#define SDO_BLOCK_DOWNLOAD_IN_PROGRESS	0x5
#define SDO_END_DOWNLOAD_REQUEST		0x6
#define SDO_BLOCK_UPLOAD_IN_PROGRESS	0x7

enum {	// CAN�ʐM���
	CAN_STS_INIT,						// ���������
	CAN_STS_OPE,						// �I�y���[�V���i�����
	CAN_STS_STOP						// ��~���
};

/* Massage Box settings */
#define CANBOX_TX		0
//NOTE:FIFO���[�h���̎�M�̓��[���{�b�N�X28���g�p����
#define CANBOX_RX		28
#define CANBOX_RX_28		28// FIFO�ݒ�p
#define CANBOX_RX_29		29// FIFO�ݒ�p
#define CANBOX_RX_30		30// FIFO�ݒ�p
#define CANBOX_RX_31		31// FIFO�ݒ�p

typedef union{
	unsigned char	byte[3];
	struct{
		unsigned char	num_key_0:1;	// �e���L�[0(on=push)
		unsigned char	num_key_1:1;	// �e���L�[1(on=push)
		unsigned char	num_key_2:1;	// �e���L�[2(on=push)
		unsigned char	num_key_3:1;	// �e���L�[3(on=push)
		unsigned char	num_key_4:1;	// �e���L�[4(on=push)
		unsigned char	num_key_5:1;	// �e���L�[5(on=push)
		unsigned char	num_key_6:1;	// �e���L�[6(on=push)
		unsigned char	num_key_7:1;	// �e���L�[7(on=push)
		unsigned char	num_key_8:1;	// �e���L�[8(on=push)
		unsigned char	num_key_9:1;	// �e���L�[9(on=push)
		unsigned char	num_key_f1:1;	// �e���L�[F1(on=push)
		unsigned char	num_key_f2:1;	// �e���L�[F2(on=push)
		unsigned char	num_key_f3:1;	// �e���L�[F3(on=push)
		unsigned char	num_key_f4:1;	// �e���L�[F4(on=push)
		unsigned char	num_key_f5:1;	// �e���L�[F5(on=push)
		unsigned char	num_key_c:1;	// �e���L�[C(on=push)
		unsigned char	cndrsw_key:1;	// �R�C�����b�N��(on=open)
		unsigned char	notesf_key:1;	// �������[�_�[��(on=open)
		unsigned char	coinsf_key:1;	// �R�C�����ɒE��(on=drop)
		unsigned char	ntfall_key:1;	// �������[�_�[�E��(on=drop)
		unsigned char	vibsns_key:1;	// �U���Z���T(on=detect)
		unsigned char	mandet_key:1;	// �l�̌��m�Z���T(on=detect)
		unsigned char	reserve1:1;		// �ƌ��������{�^��1(�\��)
		unsigned char	reserve2:1;		// �ƌ��������{�^��2(�\��)
	} bit;
} t_can_input_state;

typedef struct {
	can_std_frame_t	*dataframe;
	unsigned char	count;
	unsigned char	countmax;
	unsigned char	readidx;
	unsigned char	writeidx;
} t_can_quectrl;

typedef struct {
	unsigned short	sid;
	unsigned char	mode;
	unsigned char	state;
	unsigned char	toggle;
	unsigned short	count;
	unsigned char	blksize;
	unsigned short	offset;
	unsigned short	crc;
	unsigned long	timer;
	t_can_quectrl	*sndque;
} t_can_transfer;


/*------------------------------------------------------------------------------*/
/* cantask.c																	*/
/*------------------------------------------------------------------------------*/
extern	void			cantask	( void );
extern	unsigned int	caninit(void);
extern	short			canevent( void );
extern	void			can_rcv_queset(can_std_frame_t *frame_p);
extern	void			can_snd_data0(unsigned char d_node, unsigned char sts);
extern	void			can_snd_data2(struct clk_rec *clock);
extern	void			can_snd_data3(char ctr_cd, char out1);
extern	void			can_snd_data4(void *data, unsigned short length);
extern	void			can_snd_data14(void);

extern	t_can_transfer	transfer[SIDMAX];	// ����M�Ǘ����
extern	unsigned char	g_ucCAN1RxFlag;		// CAN1��M�����t���O
extern	unsigned char	g_ucCAN1SendFlag;	// CAN1���M���t���O
extern	unsigned char	g_ucCAN1ErrInfo;	// CAN1�G���[���
extern	unsigned char	g_ucCAN1LostErr;
extern	unsigned char	g_ucCAN1RcvSensFlag;	// CAN1�Z���T�[����M�����t���O

#endif
