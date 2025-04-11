// MH810100(S) K.Onodera 2019/11/11 �Ԕԃ`�P�b�g���X(GT-8700(LZ-122601)���p)
//[]----------------------------------------------------------------------[]
///	@brief			�p�P�b�g�ʐM�R�}���h�֘A�w�b�_�[�t�@�C��
//[]----------------------------------------------------------------------[]
///	@author			m.yatsuya
///	@date			Create	: 2009/09/04
///	@file			pkt_com.h
///	@version		LH068004
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
#ifndef	_PKT_COM_H_
#define	_PKT_COM_H_

// MH810104(S) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�ZDLL�C���^�[�t�F�[�X�̏C��)
#include "system.h"
// MH810104(E) R.Endo 2021/08/18 �Ԕԃ`�P�b�g���X �t�F�[�Y2.3 #5792 ���X�܊����Ή�(�����v�ZDLL�C���^�[�t�F�[�X�̏C��)

// �d��
enum {
// ���C�����T�u�F�R�}���h
	PKT_STS_REQ			= 0x00,			// �X�e�[�^�X�v��		: B(Base), P(Port)
	PKT_BUF_CLR_REQ,					// �o�b�t�@�N���A�v��	: P
	PKT_PRT_OPN_REQ		= 0x10,			// �|�[�gOpen�v��		: B
	PKT_TCP_CON_REQ,					// TCP Connect�v��		: P
	PKT_PKT_SND_REQ,					// �p�P�b�g���M�v��		: P
	PKT_PRT_CLS_REQ,					// �|�[�gClose�v��		: B

// ���C�����T�u�F���X�|���X

// �T�u�����C���F���X�|���X
	PKT_STS_RES			= 0x80,			// �X�e�[�^�X����
	PKT_BUF_CLR_RES,					// �o�b�t�@�N���A����
	PKT_PRT_OPN_RES		= 0x90,			// �|�[�gOpen����
	PKT_TCP_CON_RES,					// TCP Connect����
	PKT_PKT_SND_RES,					// �p�P�b�g���M����
	PKT_PRT_CLS_RES,					// �|�[�gClose����

// �T�u�����C���F�R�}���h
	PKT_RCV_NOTIFY		= 0xA0			// �p�P�b�g��M�ʒm
};

// ���
enum {
	PKT_BASE_CLOSE,
	PKT_BASE_OPEN,
};

enum {
	PKT_PORT_DISCONNECT,
	PKT_PORT_CONNECTING,
	PKT_PORT_LISTEN,
	PKT_PORT_ESTABLISH,
	PKT_PORT_SHUTDOWN
};

#define PKT_PORT_MAX		1
#define PKT_CONN_TIMEOUT	40			// �^�C���A�E�g����(�b)
//#define PKT_DATA_MAX	490	// �p�P�b�g�f�[�^�̍ő�T�C�Y
//#define PKT_CMD_MAX		462	// �R�}���h�̍ő�T�C�Y
// @todo	GT-4100:�ő�T�C�Y�����v
#define PKT_CMD_MAX		4096			// �f�[�^���̍ő�T�C�Y
#define PKT_DATA_MAX	PKT_CMD_MAX+28	// �p�P�b�g�f�[�^�ő咷���f�[�^���̍ő�T�C�Y�{�w�b�_�T�C�Y(26Byte)�{CRC16(2Byte)
#define PKT_NOTIFY_MBX_MAX	2

typedef struct {
	uchar	kind;						// �d�����
	uchar	res_sts;					// ���X�|���X�X�e�[�^�X
} PKT_CMN;

typedef struct {
	uchar	kind;						// �d�����
	uchar	reserve;					// �\��
	uchar	empty[2];					// ��
} PKT_STS_CMD;

typedef struct {
	uchar	kind;						// �d�����
	uchar	res_sts;					// ���X�|���X�X�e�[�^�X
	uchar	sckt_sts;					// �\�P�b�g�|�[�g���
	uchar	nfull_alm;					// �����M�p�P�b�g�j�A�t���A���[��
	uchar	src_IP[4];					// ����IP�A�h���X
	ushort	src_port;					// ���ǃ|�[�g�ԍ�
	uchar	dst_IP[4];					// �����IP�A�h���X
	ushort	dst_port;					// ����ǃ|�[�g�ԍ�
	ushort	unsnd_rate;					// �����M�p�P�b�g�������g�p��
	uchar	empty[6];					// ��
} PKT_STS_RES_PORT;

typedef struct {
	uchar	kind;						// �d�����
	uchar	reserve;					// �\��
	uchar	empty[2];					// ��
} PKT_CLR_CMD;

typedef struct {
	uchar	kind;						// �d�����
	uchar	res_sts;					// ���X�|���X�X�e�[�^�X
	uchar	empty[2];					// ��
} PKT_CLR_RES;

typedef struct {
	uchar	kind;						// �d�����
	uchar	reserve;					// �\��
	uchar	port_no;					// �ʐM�|�[�g
	uchar	opn_mode;					// Open���[�h
	ushort	src_port;					// ���ǃ|�[�g�ԍ�
	uchar	empty[2];					// ��
} PKT_OPN_CMD;

typedef struct {
	uchar	kind;						// �d�����
	uchar	res_sts;					// ���X�|���X�X�e�[�^�X
	uchar	port_no;					// �ʐM�|�[�g
	uchar	opn_mode;					// Open���[�h
	ushort	src_port;					// ���ǃ|�[�g�ԍ�
	uchar	empty[2];					// ��
} PKT_OPN_RES;

typedef struct {
	uchar	kind;						// �d�����
	uchar	reserve;					// �\��
	uchar	dst_IP[4];					// �����IP�A�h���X
	ushort	dst_port;					// ����ǃ|�[�g�ԍ�
	uchar	wait_time;					// �ڑ��ő�҂�����
	uchar	empty;						// ��
} PKT_CON_CMD;

typedef struct {
	uchar	kind;						// �d�����
	uchar	res_sts;					// ���X�|���X�X�e�[�^�X
	uchar	dst_IP[4];					// �����IP�A�h���X
	ushort	dst_port;					// ����ǃ|�[�g�ԍ�
	uchar	wait_time;					// �ڑ��ő�҂�����
	uchar	empty;						// ��
} PKT_CON_RES;

typedef struct {
	uchar	kind;						// �d�����
	uchar	reserve;					// �\��
	uchar	snd_no;						// ���M�f�[�^���ʔԍ�
	uchar	block_sts;					// �u���b�N�X�e�[�^�X
	uchar	empty[4];					// ��
	ushort	data_size;					// �f�[�^�T�C�Y
	uchar	snd_data[PKT_DATA_MAX];		// �f�[�^
} PKT_SND_CMD;

typedef struct {
	uchar	kind;						// �d�����
	uchar	res_sts;					// ���X�|���X�X�e�[�^�X
	uchar	snd_no;						// ���M�f�[�^���ʔԍ�
	uchar	block_sts;					// �u���b�N�X�e�[�^�X
	uchar	empty[4];					// ��
	ushort	data_size;					// �f�[�^�T�C�Y
} PKT_SND_RES;

typedef struct {
	uchar	kind;						// �d�����
	uchar	rcv_sts;					// ��M�X�e�[�^�X
	uchar	empty[6];					// ��
	ushort	data_size;					// �f�[�^�T�C�Y
	uchar	rcv_data[PKT_DATA_MAX];		// �f�[�^
} PKT_RCV_RES;

typedef struct {
	uchar	kind;						// �d�����
	uchar	reserve;					// �\��
	uchar	port_no;					// �ʐM�|�[�g
	uchar	empty;						// ��
} PKT_CLS_CMD;

typedef struct {
	uchar	kind;						// �d�����
	uchar	res_sts;					// ���X�|���X�X�e�[�^�X
	uchar	port_no;					// �ʐM�|�[�g
	uchar	empty;						// ��
} PKT_CLS_RES;

typedef union {
	PKT_CMN			com;
	PKT_OPN_CMD		opn;				// Open
	PKT_CON_CMD		con;				// Connect
	PKT_SND_CMD		snd;				// Send
	PKT_CLS_CMD		cls;				// Close
} PKT_SNDCMD;

typedef union {
	PKT_CMN				com;
	PKT_OPN_RES			opn;			// Open
	PKT_CON_RES			con;			// Connect
	PKT_SND_RES			snd;			// Send
	PKT_RCV_RES			rcv;			// Receive
	PKT_CLS_RES			cls;			// Close
} PKT_RCVRES;

typedef	union {
	uchar	data[PKT_DATA_MAX];
	struct {
		ulong	data_size;				// �p�P�b�g�f�[�^��(data_size�`crc16h)
		uchar	identifier[12];			// �w�b�_�[�R�[�h("AMANO_PACKET")
		uchar	seqno[3];				// �V�[�P���X��("000"�`"999")
		uchar	id;						// �f�[�^���R�[�h(0x00)
		uchar	kind;					// �d�����ʃR�[�h(0x10)
		uchar	blkno_h;				// 
		uchar	blkno_l;				// �u���b�NNo.(1)
		uchar	block_sts;				// �u���b�N�X�e�[�^�X(1)
		uchar	encrypt;				// �Í�������(0:�Í����Ȃ�,1:AES)
		uchar	pad_size;				// �p�f�B���O�T�C�Y(0�`16:�Í������̃f�[�^���̃p�f�B���O�T�C�Y)
		uchar	data[PKT_CMD_MAX];		// �f�[�^��(�ϒ�)
		uchar	pad[16];				// �Í�������16Byte�P�ʂƂ���ׂ�0�p�f�B���O�p�G���A(RT���Z�f�[�^��PKT_CMD_MAX�ƂȂ����ꍇ���l��)
//-		uchar	check_area[16];			// EncryptWithKeyAndIV()�N������(�`�F�b�N�Ŏg�p)�G���A
//		ushort	crc16;					// CRC16(�f�[�^�����ϒ��̂��ߒ�`���Ȃ�)
	} part;
} PKT_TRAILER;

#endif	// _PKT_COM_H_
// MH810100(E) K.Onodera 2019/11/11 �Ԕԃ`�P�b�g���X(GT-8700(LZ-122601)���p)
