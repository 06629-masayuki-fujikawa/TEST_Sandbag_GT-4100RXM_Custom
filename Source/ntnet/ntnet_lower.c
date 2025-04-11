/*[]----------------------------------------------------------------------[]
 *| System      : UT8500
 *| Module      : NT-NET�ʐM���W���[��(���ʃ��C���[)
 *[]----------------------------------------------------------------------[]
 *| Author      : ogura.s
 *| Date        : 2005.08.22
 *| Update      :
 *|		2006-01-17	machida.k	�A�C�h�����׌y�� (MH544401)
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*			I N C L U D E													*/
/*--------------------------------------------------------------------------*/
#include	<string.h>
#include	<stdio.h>
#include	"system.h"
#include	"prm_tbl.h"
#include	"common.h"
#include	"message.h"
#include	"ntnet.h"

/*--------------------------------------------------------------------------*/
/*			C O N S T A N T S												*/
/*--------------------------------------------------------------------------*/
/* ���ʐݒ�p�����[�^ */
#define	PRM_TERMINAL_NO			CPrmSS[S_NTN][NTPRM_TERMINALNO]		/* �[���ԍ� */
#define	PRM_RESPONSE_TIME		CPrmSS[S_NTN][NTPRM_RESPONSE_TIME]	/* MAIN���W���[������̉����҂����� */
#define	PRM_RETRY_COUNT			CPrmSS[S_NTN][NTPRM_RETRY]			/* MAIN���W���[���̃��g���C�� */

#define	SEND_INTERVAL			500

/* ���M����p��ϰ�N�� */
#define	START_SNDCTRL_TIMER(t)	Lagtim(NTNETTCBNO, 1, (ushort)((t)/20))
/* ���M����p��ϰ���� */
#define	STOP_SNDCTRL_TIMER()	Lagcan(NTNETTCBNO, 1)
/* ���M����p��ѱ�Ĳ���� */
#define	TIMEOUT_SNDCTRL			TIMEOUT1

/* ARC����p��ϰ�N�� */
#define	START_ARCCTRL_TIMER(t)	Lagtim(NTNETTCBNO, 2, (ushort)((t)/20))
/* ARC����p��ϰ���� */
#define	STOP_ARCCTRL_TIMER()	Lagcan(NTNETTCBNO, 2)
/* ARC����p��ѱ�Ĳ���� */
#define	TIMEOUT_ARCCTRL			TIMEOUT2

#define	CMDID_SNDDATA			'S'		/* ���M�f�[�^ID */
#define	CMDID_SNDRSLT			'r'		/* ���M���ʃf�[�^ID */
#define	CMDID_RCVDATA			's'		/* ��M�f�[�^ID */
#define	CMDID_RCVRSLT			'R'		/* ��M���ʃf�[�^ID */
#define	CMDID_ERRDATA			'e'		/* �G���[�f�[�^ */
#define	CMDID_ERRRSLT			'E'		/* �G���[���ʃf�[�^ */

/* ARC�u���b�N����M���ʃf�[�^�ɂ�����d�����ł̃I�t�Z�b�g */
#define	OFS_TERMINAL_NO			1			/* �[��No */
#define	OFS_ARCBLOCK_RESULT_ID	11			/* ARC�u���b�N����ID */
#define	OFS_RESULT_DATA			12			/* ���� */

/* ARC�u���b�N����M���ʃf�[�^�̃f�[�^���̃T�C�Y */
#define	ARC_BLOCK_RESULT_SIZE			15

#define	OFS_ARCBLOCK_RESULT_ID2	(5-2)			/* ARC�u���b�N����ID */
#define	OFS_RESULT_DATA2		(6-2)			/* ���� */
#define	ARC_BLOCK_RESULT_SIZE2	7

/* ARC���M�f�[�^�̍ő�T�C�Y */
#define	ARC_1STBLK_SIZE		498
#define	ARC_2NDBLK_SIZE		500
#define	ARC_TOTAL_SIZE		(ARC_1STBLK_SIZE+ARC_2NDBLK_SIZE)

/* ARCNET�Ǘ��w�p�C�x���g */
enum {
	EVT_NONE = 0,		/* ���� */
	EVT_SND_OK,			/* ���M����I�� */
	EVT_RCV_NORMAL,		/* NTNET�ʏ��M�f�[�^��M */
	EVT_RCV_FREE,		/* FREE�p�P�b�g�f�[�^��M */
	EVT_RCV_ERR,		/* �G���[�f�[�^��M */
	EVT_RCV_BLKRST,		/* ARC�u���b�N����M���ʃf�[�^��M�C�x���g */
	EVT_TIMEOUT			/* �^�C�}�[�C�x���g */
};

enum {
	_1STBLK = 0,	/* ARC���u���b�N */
	_2NDBLK			/* ARC���u���b�N */
};

/* ���M��� */
typedef enum {
	SNDSTS_IDLE = 0,		/* ���M�ҋ@ */
	SNDSTS_WAIT_SNDBLK1,	/* ��1��ۯ����M�҂� */
	SNDSTS_WAIT_SNDFIN1,	/* ��1��ۯ����M�����҂� */
	SNDSTS_WAIT_SNDBLK2,	/* ��2��ۯ����M�҂� */
	SNDSTS_WAIT_SNDFIN2,	/* ��2��ۯ����M�����҂� */
	SNDSTS_WAIT_BLKRST		/* ���M���ʃf�[�^��M�҂� */
} eNTLWR_SNDSTS;

/* ��M��� */
typedef enum {
	RCVSTS_WAIT_BLK1 = 0,	/* ��1��ۯ���M�҂� */
	RCVSTS_WAIT_BLK2,		/* ��2��ۯ���M�҂� */
	RCVSTS_WAIT_SNDRST		/* ��M���ʃf�[�^���M�҂� */
} eNTLWR_RCVSTS;

/* ARC����-���M��� */
typedef enum {
	ARCSTS_SND_IDLE = 0,	/* �ҋ@�� */
	ARCSTS_WAIT_SNDFIN,		/* ���M�����҂� */
	ARCSTS_WAIT_RESND,		/* �đ��҂� */
	ARCSTS_WAIT_SNDRST		/* ���M���ʎ�M�҂� */
} eNTLWR_ARCSTS;

/*--------------------------------------------------------------------------*/
/*			S T R U C T U R E S												*/
/*--------------------------------------------------------------------------*/
/* ARCNET�d���F�ʏ푗�M�f�[�^ */
typedef struct {
	uchar	SeqNo;			/* �V�[�P���V����No */
	uchar	FuncID;			/* ����@�\ID */
	uchar	BlkNo;			/* ARC�u���b�NNo */
	uchar	CmdID;			/* �f�[�^��� */
	uchar	Size[2];		/* �f�[�^�T�C�Y */
	uchar	Oiban;			/* �f�[�^�ǔ� */
	uchar	BlkSts;			/* ARC�u���b�N�X�e�[�^�X */
	struct {
		uchar	rsv1;			/* �\�� */
		uchar	rsv2;			/* �\�� */
		uchar	data[498];		/* ���ۂ̑��M�f�[�^ */
	} SndData;				/* ���M�f�[�^ */
} t_ArcSndData;

/* ARCNET�d���F���M���ʃf�[�^ */
typedef struct {
	uchar	SeqNo;			/* �V�[�P���V����No */
	uchar	FuncID;			/* ����@�\ID */
	uchar	brank;			/* �� */
	uchar	CmdID;			/* �f�[�^��� */
	uchar	Oiban;			/* �f�[�^�ǔ� */
	uchar	rsv1;			/* �\�� */
	uchar	Result;			/* ���� */
	uchar	rsv2;			/* �\�� */
} t_ArcSndResult;

/* ARCNET�d���F�ʏ��M�f�[�^ */
typedef struct {
	uchar	SeqNo;			/* �V�[�P���V����No */
	uchar	FuncID;			/* ����@�\ID */
	uchar	BlkNo;			/* ARC�u���b�NNo */
	uchar	CmdID;			/* �f�[�^��� */
	uchar	Size[2];		/* �f�[�^�T�C�Y */
	uchar	Oiban;			/* �f�[�^�ǔ� */
	uchar	BlkSts;			/* ARC�u���b�N�X�e�[�^�X */
	struct {
		uchar	NodeID;			/* �m�[�hID */
		uchar	ComPrt;			/* �ʐM�|�[�g */
		uchar	data[498];		/* ���ۂ̎�M�f�[�^ */
	} RcvData;				/* ��M�f�[�^ */
} t_ArcRcvData;

/* ARCNET�d���F��M���ʃf�[�^ */
typedef struct {
	uchar	SeqNo;			/* �V�[�P���V����No */
	uchar	FuncID;			/* ����@�\ID */
	uchar	brank;			/* �� */
	uchar	CmdID;			/* �f�[�^��� */
	uchar	Oiban;			/* �f�[�^�ǔ� */
	uchar	rsv1;			/* �\�� */
	uchar	Result;			/* ���� */
	uchar	rsv2;			/* �\�� */
} t_ArcRcvResult;

/* ARCNET�d���F�G���[�f�[�^ */
typedef struct {
	uchar	SeqNo;			/* �V�[�P���V����No */
	uchar	FuncID;			/* ����@�\ID */
	uchar	brank;			/* �� */
	uchar	CmdID;			/* �f�[�^��� */
	uchar	Oiban;			/* �f�[�^�ǔ� */
	uchar	TarmNo;			/* �^�[�~�i��No */
	uchar	Module;			/* ���W���[���R�[�h */
	uchar	Err;			/* �G���[�R�[�h */
	uchar	OnOff;			/* �����^���� */
	uchar	rsv[7];			/* �\�� */
} t_ArcErrData;

/* ARCNET�d���F�G���[���ʃf�[�^ */
typedef struct {
	uchar	SeqNo;			/* �V�[�P���V����No */
	uchar	FuncID;			/* ����@�\ID */
	uchar	brank;			/* �� */
	uchar	CmdID;			/* �f�[�^��� */
	uchar	Oiban;			/* �f�[�^�ǔ� */
	uchar	rsv1;			/* �\�� */
	uchar	Rrsult;			/* ���� */
	uchar	rsv2;			/* �\�� */
} t_ArcErrResult;

/* ���M����\���� */
typedef struct {
	eNTLWR_SNDSTS		status;			/* ���M��� */
	uchar				retry;			/* ���g���C�� */
	eNTNET_DATA_KIND	kind;			/* ���M��� */
	eNTNET_SRSLT		result;			/* ���M���� */
	uchar				buff[2][508];	/* ���M�o�b�t�@ */
	ushort				size[2];		/* ���M�o�b�t�@�i�[�ς݃T�C�Y */
} t_SndCtrl;

/* ��M����\���� */
typedef struct {
	eNTLWR_RCVSTS		status;			/* ��M��� */
	uchar				retry;			/* ���g���C�� */
	uchar				buff[1000];		/* ��MNT�p�P�b�g�L���p�o�b�t�@ */
	ushort				size;			/* ��M�o�b�t�@�i�[�ς݃T�C�Y */
	uchar				blk_rslt[23];	/* ARC��ۯ���M���ʃf�[�^���M�p�o�b�t�@ */
	ushort				rslt_size;		/* ARC��ۯ���M���ʃf�[�^���M�p�o�b�t�@�f�[�^�� */
} t_RcvCtrl;

/* ARC����\���� */
typedef struct {
	eNTLWR_ARCSTS		status;		/* ���M��� */
	uchar				*snd_data;	/* ���M�f�[�^ */
	ushort				snd_size;	/* ���M�f�[�^�T�C�Y */
	uchar				snd_oiban;	/* ���M�f�[�^�ǂ��� */
	uchar				rcv_oiban;	/* ��M�f�[�^�ǂ��� */
	uchar				err_oiban;	/* �G���[�f�[�^�ǂ��� */
} t_ArcCtrl;


/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S												*/
/*--------------------------------------------------------------------------*/
/* NT���ʃ��C���[����f�[�^ */
static struct {
	t_ArcCtrl	arc_ctrl;			/* ARC����f�[�^ */
	t_SndCtrl	snd_ctrl;			/* ���M��������f�[�^ */
	t_RcvCtrl	rcv_ctrl;			/* ��M��������f�[�^ */
} NtLwrCtrl;

/* NT���ʃ��C���[ �f�B�X�p�b�`�t���O */
BOOL	NtLwr_Dispatch;

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/
static void		ntlwr_rcv(uchar event, ulong msgprm);
static void		ntlwr_snd(uchar event, ulong msgprm);
static void		make_arcblk_rrslt(uchar	rslt);

static uchar	ntlwr_arc(ushort event, ulong msgprm);
static BOOL		send_arc_packet(uchar *pdata, ushort size);

static BOOL		ntlwr_MakeDispatchFlag(void);

static BOOL		ntlwr_SendRemoteData(uchar *data, ushort len);
static ushort	ntlwr_Read_Remote_1stBlk(uchar *data, ushort len);

/*--------------------------------------------------------------------------*/
/*			I N T E R F A S E     F U N C T I O N S							*/
/*--------------------------------------------------------------------------*/
/*[]----------------------------------------------------------------------[]
 *|	name	: NTLWR_Init
 *[]----------------------------------------------------------------------[]
 *| summary	: ���ʃ��C���[����������
 *| param	: void
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTLWR_Init(void)
{
	/* �[���N���A */
	memset(&NtLwrCtrl, 0, sizeof(NtLwrCtrl));

	NtLwrCtrl.arc_ctrl.status = ARCSTS_SND_IDLE;		/* ARC����-���M��� */
	NtLwrCtrl.arc_ctrl.snd_oiban = 0xff;				/* ARC����-���M�f�[�^�ǂ��� */
	NtLwrCtrl.arc_ctrl.rcv_oiban = 0xff;				/* ARC����-��M�f�[�^�ǂ��� */
	NtLwrCtrl.arc_ctrl.err_oiban = 0xff;				/* ARC����-�G���[�f�[�^�ǂ��� */

	NtLwrCtrl.snd_ctrl.status = SNDSTS_IDLE;			/* ���M����-��� */

	NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_BLK1;		/* ��M����-��� */
	NtLwr_Dispatch = FALSE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NtArc_SendReq
 *[]----------------------------------------------------------------------[]
 *| summary	: �f�[�^���M�v��
 *| param	: uchar            *data - ���M�f�[�^
 *| 		: ushort           len   - ���M�f�[�^��
 *| 		: eNTNET_DATA_KIND kind  - ���M�f�[�^���
 *| return	: TRUE  - �v����t
 *| 		: FALSE - �v����t�s��
*[]----------------------------------------------------------------------[]*/
BOOL	NTLWR_SendReq(uchar *data, ushort len, eNTNET_DATA_KIND kind)
{
	t_ArcSndData	*pdata;
	ushort			size;
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	/* ���M�f�[�^�ݒ�ς݁H */
	if (NtLwrCtrl.snd_ctrl.size[_1STBLK] != 0)
		return FALSE;

	/* ���M�d����ʓo�^ */
	NtLwrCtrl.snd_ctrl.kind = kind;

	/* NT-NET�d�� */
	if (kind == NTNET_NTDATA) {
			if (_is_ntnet_remote()) {
				return ntlwr_SendRemoteData(data, len);
			}

		/* �f�[�^���m�F */
		if (len > ARC_TOTAL_SIZE)
			return FALSE;

		/* ���M�f�[�^�o�^ */
		if (len > ARC_1STBLK_SIZE) {
			/* �Q�u���b�N�ڂ��Z�b�g���� */
			pdata = (t_ArcSndData *)NtLwrCtrl.snd_ctrl.buff[_2NDBLK];
			size = (ushort)(len - ARC_1STBLK_SIZE);
			memcpy(&pdata->SndData, &data[ARC_1STBLK_SIZE], (size_t)size);
			NtLwrCtrl.snd_ctrl.size[_2NDBLK] = (ushort)(8 + size);	/* ͯ��+�ް� */

			/* �w�b�_���쐬(���ݼ�����ް�E����@�\ID�́A�����ł͐ݒ肵�Ȃ� */
			pdata->BlkNo = 2;
			pdata->CmdID = CMDID_SNDDATA;
			u.us = size;
			pdata->Size[0] = u.uc[0];
			pdata->Size[1] = u.uc[1];
			pdata->BlkSts = NTNET_LAST_BLOCK;

			len = ARC_1STBLK_SIZE;
		}
		/* �P�u���b�N�ڂ��Z�b�g */
		pdata = (t_ArcSndData *)NtLwrCtrl.snd_ctrl.buff[_1STBLK];
		size = len;
		pdata->SndData.rsv1 = 0;
		pdata->SndData.rsv2 = 0;
		memcpy(pdata->SndData.data, data, (size_t)size);
		NtLwrCtrl.snd_ctrl.size[_1STBLK] = (ushort)(8 + 2 + size);	/* ͯ��+�\��+�ް� */

		/* �w�b�_���쐬(���ݼ��No�E����@�\ID�E�ǔԂ́A�����ł͐ݒ肵�Ȃ� */
		pdata->BlkNo = 1;
		pdata->CmdID = CMDID_SNDDATA;
		u.us = (ushort)(2 + size);				/* �\��+�ް� */
		pdata->Size[0] = u.uc[0];
		pdata->Size[1] = u.uc[1];
		/* 2��ۯ��ڂ�ݒ肵�Ă���΁A������ۯ� */
		pdata->BlkSts = (NtLwrCtrl.snd_ctrl.size[_2NDBLK]) ? NTNET_NOT_LAST_BLOCK : NTNET_LAST_BLOCK;
	}
	/* FREE�f�[�^(�G���[�f�[�^�𑗐M���邱�Ƃ͂Ȃ�) */
	else {	/* if (kind == NTNET_FREEDATA) */
		/* �f�[�^���m�F */
		if (len > ARC_2NDBLK_SIZE)
			return FALSE;

		/* �P�u���b�N�ڂ��Z�b�g */
		pdata = (t_ArcSndData *)NtLwrCtrl.snd_ctrl.buff[_1STBLK];
		size = len;
		memcpy(&pdata->SndData, data, (size_t)size);
		NtLwrCtrl.snd_ctrl.size[_1STBLK] = (ushort)(8 + size);	/* ͯ��+�ް� */

		/* �w�b�_���쐬(���ݼ��No�E����@�\ID�E�ǔԂ́A�����ł͐ݒ肵�Ȃ� */
		pdata->BlkNo = 1;
		pdata->CmdID = CMDID_SNDDATA;
		u.us = size;
		pdata->Size[0] = u.uc[0];
		pdata->Size[1] = u.uc[1];
		pdata->BlkSts = NTNET_LAST_BLOCK;
	}
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntlwr_SendRemoteData
 *[]----------------------------------------------------------------------[]
 *| summary	: �f�[�^�t�H�[�}�b�g�ύX(�ʏ큨���u)
 *| param	: 
 *| return	: 
 *[]----------------------------------------------------------------------[]*/
static	BOOL	ntlwr_SendRemoteData(uchar *data, ushort len)
{
	t_ArcSndData	*pdata;
	ushort			size;
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	t_NtNetBlk			*ntblk;
	t_RemoteNtNetBlk	*r_blk;
	uchar				*r_data;
	ushort				r_len;

	ntblk = (t_NtNetBlk *)data;
	// �Q�u���b�N�ڐ擪
	r_data = data;
	r_data += sizeof(pdata->SndData);
	r_data += (ushort)(sizeof(t_NtNetBlkHeader) - sizeof(t_RemoteNtNetBlkHeader));
	// �t�H�[�}�b�g�C����̒���
	len -= (ushort)(sizeof(t_NtNetBlkHeader) - sizeof(t_RemoteNtNetBlkHeader));
	r_len = len;

	/* NT-NET�d�� */
	/* �f�[�^���m�F */
	if (len > (sizeof(pdata->SndData)*2))
		return FALSE;

	/* ���M�f�[�^�o�^ */
	if (len > sizeof(pdata->SndData)) {
		/* �Q�u���b�N�ڂ��Z�b�g���� */
		pdata = (t_ArcSndData *)NtLwrCtrl.snd_ctrl.buff[_2NDBLK];
		size = (ushort)(len - sizeof(pdata->SndData));
		memcpy(&pdata->SndData, r_data, (size_t)size);
		NtLwrCtrl.snd_ctrl.size[_2NDBLK] = (ushort)(8 + size);	/* ͯ��+�ް� */

		/* �w�b�_���쐬(���ݼ�����ް�E����@�\ID�́A�����ł͐ݒ肵�Ȃ� */
		pdata->BlkNo = 2;
		pdata->CmdID = CMDID_SNDDATA;
		u.us = size;
		pdata->Size[0] = u.uc[0];
		pdata->Size[1] = u.uc[1];
		pdata->BlkSts = NTNET_LAST_BLOCK;

		len = sizeof(pdata->SndData);
	}
	/* �P�u���b�N�ڂ��Z�b�g */
	pdata = (t_ArcSndData *)NtLwrCtrl.snd_ctrl.buff[_1STBLK];
	size = len;
	r_blk = (t_RemoteNtNetBlk*)&pdata->SndData;
	u.us = (ushort)r_len;
	r_blk->header.packet_size[0] = u.uc[0];
	r_blk->header.packet_size[1] = u.uc[1];
	r_blk->header.len_before0cut[0] = ntblk->header.len_before0cut[0];
	r_blk->header.len_before0cut[1] = ntblk->header.len_before0cut[1];
	r_blk->header.reserve = ntblk->header.reserve;
	memcpy(&r_blk->data, &ntblk->data, (size_t)size-sizeof(t_RemoteNtNetBlkHeader));
	NtLwrCtrl.snd_ctrl.size[_1STBLK] = (ushort)(8 + size);	/* ͯ��+�ް� */

	/* �w�b�_���쐬(���ݼ��No�E����@�\ID�E�ǔԂ́A�����ł͐ݒ肵�Ȃ� */
	pdata->BlkNo = 1;
	pdata->CmdID = CMDID_SNDDATA;
	u.us = (ushort)(size);				/* �ް� */
	pdata->Size[0] = u.uc[0];
	pdata->Size[1] = u.uc[1];
	/* 2��ۯ��ڂ�ݒ肵�Ă���΁A������ۯ� */
	pdata->BlkSts = (NtLwrCtrl.snd_ctrl.size[_2NDBLK]) ? NTNET_NOT_LAST_BLOCK : NTNET_LAST_BLOCK;
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntlwr_Read_Remote_1stBlk
 *[]----------------------------------------------------------------------[]
 *| summary	: �f�[�^�t�H�[�}�b�g�ύX(���u���ʏ�)
 *| param	: 
 *| return	: 
 *[]----------------------------------------------------------------------[]*/
static	ushort	ntlwr_Read_Remote_1stBlk(uchar *data, ushort len)
{
	t_NtNetBlk			*n_blk;
	t_RemoteNtNetBlk	*r_blk;
	ushort	n_len;

	n_blk = (t_NtNetBlk*)NtLwrCtrl.rcv_ctrl.buff;
	r_blk = (t_RemoteNtNetBlk*)data;
	n_len = len;

	// �w�b�_(upper�ŎQ�Ƃ��Ă�����̂̂݃Z�b�g)
	n_blk->header.vect = 0;
	n_blk->header.terminal_no = 0;
	n_blk->header.lower_terminal_no1 = 0;
	n_blk->header.lower_terminal_no2 = 0;
	n_blk->header.link_no = 0;
	n_blk->header.status = 0;
	n_blk->header.broadcast_rcvd = 0;
	n_blk->header.len_before0cut[0] = r_blk->header.len_before0cut[0];
	n_blk->header.len_before0cut[1] = r_blk->header.len_before0cut[1];
	n_blk->header.mode = NTNET_NORMAL_DATA;
	n_blk->header.reserve = n_blk->header.reserve;
	n_blk->header.telegram_type = STX;
	n_blk->header.res_detail = 0;
	n_len += (sizeof(t_NtNetBlkHeader) - sizeof(t_RemoteNtNetBlkHeader));

	// �f�[�^
	memcpy((uchar*)(&n_blk->data), (uchar*)(&r_blk->data), len - sizeof(t_RemoteNtNetBlkHeader));

	return n_len;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NtArc_IsSendCmplete
 *[]----------------------------------------------------------------------[]
 *| summary	: �f�[�^���M���ʎ擾
 *| param	: eNTNET_SRSLT *result - ���M���ʊi�[�̈�
 *| return	: TRUE  - ���M����
 *| 		: FALSE - ���M������
 *[]----------------------------------------------------------------------[]*/
BOOL	NTLWR_IsSendComplete(eNTNET_SRSLT *result)
{
	/* ���M�f�[�^����H */
	if (NtLwrCtrl.snd_ctrl.size[_1STBLK] != 0)
		return FALSE;
	*result = NtLwrCtrl.snd_ctrl.result;
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTLWR_Main
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET�ʐM���W���[�� ���ʃ��C���[���C������
 *| param	: MsgBuf *pmsb - ��M���b�Z�[�W
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTLWR_Main(MsgBuf *pmsb)
{
	ushort	arc_evt;
	uchar	nt_evt;

	ulong	msgprm;
	
	if (pmsb != NULL) {
		memcpy(&msgprm, pmsb->msg.data, sizeof(msgprm));
		arc_evt	= pmsb->msg.command;
	}
	else {
		msgprm	= 0;
		arc_evt	= 0;
	}

	/* ARC���� */
	nt_evt = ntlwr_arc(arc_evt, msgprm);

	/* ��M���� */
	ntlwr_rcv(nt_evt, msgprm);
	/* ���M���� */
	ntlwr_snd(nt_evt, msgprm);

	NtLwr_Dispatch = ntlwr_MakeDispatchFlag();
}

/*--------------------------------------------------------------------------*/
/*			S T A T I C   F U N C T I O N S									*/
/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*			ARCNET�Ǘ��w													*/
/*--------------------------------------------------------------------------*/
/*[]----------------------------------------------------------------------[]
 *|	name	: ntlwr_rcv
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M���䏈��
 *| param	: uchar event - ��M����M����p�C�x���g
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static void	ntlwr_rcv(uchar event, ulong msgprm)
{
	t_ArcRcvData	*prcv;
	t_ArcErrData	*perr;
	eNTNET_RRSLT	ret;
	uchar			rslt;
	ushort	len;

	perr = 0;
	prcv = 0;
	switch (NtLwrCtrl.rcv_ctrl.status) {
/*--- ��P�A�[�N�u���b�N��M�҂� ---*/
	case RCVSTS_WAIT_BLK1:
/*--- ��Q�A�[�N�u���b�N��M�҂� ---*/
	case RCVSTS_WAIT_BLK2:
		/* NTNET�ʏ��M�f�[�^��M */
		if (event == EVT_RCV_NORMAL) {

			/* �u���b�N�ԍ��m�F */
			if (prcv->BlkNo == 1) {
				/* �u���b�N�X�e�[�^�X�m�F */
				if (prcv->BlkSts == NTNET_NOT_LAST_BLOCK) {
					/* ��M�f�[�^�L��(NodeID,ComNo�͏ȗ�����) */
					len = (ushort)_MAKEWORD(prcv->Size);
					if (_is_ntnet_remote()) {
						// ���u���ʏ�`���֕ϊ����ăR�s�[
						len	= ntlwr_Read_Remote_1stBlk((uchar*)&prcv->RcvData, len);
					}
					else {
						len -= 2;
						memcpy(NtLwrCtrl.rcv_ctrl.buff, &prcv->RcvData.data, (size_t)(len));
					}
					NtLwrCtrl.rcv_ctrl.size = len;
					/* ��Q�A�[�N�u���b�N��M�҂���Ԃ� */
					NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_BLK2;
					NtLwrCtrl.rcv_ctrl.retry = 0;
				}
				else if (prcv->BlkSts == NTNET_LAST_BLOCK) {
					/* �f�[�^��M�����ʒm(NodeID,ComNo�͏ȗ�����) */
					len = (ushort)_MAKEWORD(prcv->Size);
					if (_is_ntnet_remote()) {
						// ���u���ʏ�`���֕ϊ����ăR�s�[
						len	= ntlwr_Read_Remote_1stBlk((uchar*)&prcv->RcvData, len);
						ret = NTUPR_SetRcvPkt(NtLwrCtrl.rcv_ctrl.buff, len, NTNET_NTDATA);
					} else {
						ret = NTUPR_SetRcvPkt(prcv->RcvData.data, (ushort)(len-2), NTNET_NTDATA);
					}
					/* Arc�u���b�N��M���ʃf�[�^�쐬 */
					if (ret == NTNET_RRSLT_NORMAL) {
						rslt = 0x00;	/* ���� */
					}
					else if (ret == NTNET_RRSLT_BLKNO_INVALID) {
						rslt = 0x85;	/* NT��ۯ�No�ُ�(���񖳌�) */
					}
					else {	/* if (ret == NTNET_RRSLT_BLKNO_VALID) */
						rslt = 0x86;	/* NT��ۯ�No�ُ�(����L��) */
					}
					make_arcblk_rrslt(rslt);

					/* Arc�u���b�N��M���ʃf�[�^���M�v�� */
					if (send_arc_packet(NtLwrCtrl.rcv_ctrl.blk_rslt,
										NtLwrCtrl.rcv_ctrl.rslt_size) == FALSE) {
						/*  Arc�u���b�N��M���ʃf�[�^���M�҂���Ԃ� */
						NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_SNDRST;
					}
					else {
						/* ��P�A�[�N�u���b�N��M�҂���Ԃ� */
						NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_BLK1;
					}
					NtLwrCtrl.rcv_ctrl.retry = 0;
				}
			}
			else if (prcv->BlkNo == 2) {
				/* �u���b�N�X�e�[�^�X�m�F */
				if (prcv->BlkSts == NTNET_LAST_BLOCK) {
					/* ��P�A�[�N�u���b�N��M�҂� */
					if (NtLwrCtrl.rcv_ctrl.status == RCVSTS_WAIT_BLK1) {
						/* Arc�u���b�N��M���ʃf�[�^�쐬 */
						if (NtLwrCtrl.rcv_ctrl.retry < PRM_RETRY_COUNT+3) {
							rslt = 0x81;	/* ARC��ۯ�No�ُ�(�đ��v��) */
							NtLwrCtrl.rcv_ctrl.retry++;
						}
						else {
							rslt = 0x01;	/* ARC��ۯ�No�ُ�(�đ��Ȃ�) */
							NtLwrCtrl.rcv_ctrl.retry = 0;
						}
						make_arcblk_rrslt(rslt);
	
						/* Arc�u���b�N��M���ʃf�[�^���M�v�� */
						if (send_arc_packet(NtLwrCtrl.rcv_ctrl.blk_rslt,
											NtLwrCtrl.rcv_ctrl.rslt_size) == FALSE) {
							/*  Arc�u���b�N��M���ʃf�[�^���M�҂���Ԃ� */
							NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_SNDRST;
						}
					}
					/* ��Q�A�[�N�u���b�N��M�҂� */
					else {	/* if (NtLwrCtrl.rcv_ctrl.status == RCVSTS_WAIT_BLK2) */
						/* �f�[�^��M�����ʒm */
						memcpy(&NtLwrCtrl.rcv_ctrl.buff[NtLwrCtrl.rcv_ctrl.size], &prcv->RcvData, (size_t)_MAKEWORD(prcv->Size));
						NtLwrCtrl.rcv_ctrl.size += _MAKEWORD(prcv->Size);
						ret = NTUPR_SetRcvPkt(NtLwrCtrl.rcv_ctrl.buff, NtLwrCtrl.rcv_ctrl.size, NTNET_NTDATA);	
						/* Arc�u���b�N��M���ʃf�[�^�쐬 */
						if (ret == NTNET_RRSLT_NORMAL) {
							rslt = 0x00;	/* ���� */
						}
						else if (ret == NTNET_RRSLT_BLKNO_INVALID) {
							rslt = 0x85;	/* NT��ۯ�No�ُ�(���񖳌�) */
						}
						else {	/* if (ret == NTNET_RRSLT_BLKNO_VALID) */
							rslt = 0x86;	/* NT��ۯ�No�ُ�(����L��) */
						}
						make_arcblk_rrslt(rslt);
	
						/* Arc�u���b�N��M���ʃf�[�^���M�v�� */
						if (send_arc_packet(NtLwrCtrl.rcv_ctrl.blk_rslt,
											NtLwrCtrl.rcv_ctrl.rslt_size) == FALSE) {
							/*  Arc�u���b�N��M���ʃf�[�^���M�҂���Ԃ� */
							NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_SNDRST;
						}
						else {
							/* ��P�A�[�N�u���b�N��M�҂���Ԃ� */
							NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_BLK1;
						}
					}
				}
			}
		}
		/* FREE�f�[�^��M */
		else if (event == EVT_RCV_FREE) {
			/* �f�[�^��M�����ʒm */
			NTUPR_SetRcvPkt((uchar *)&prcv->RcvData, _MAKEWORD(prcv->Size), NTNET_FREEDATA);
			/* ��P�A�[�N�u���b�N��M�҂���Ԃ� */
			NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_BLK1;
			NtLwrCtrl.rcv_ctrl.retry = 0;
		}
		/* �G���[�f�[�^��M */
		else if (event == EVT_RCV_ERR) {
			/* �f�[�^��M�����ʒm */
			NTUPR_SetRcvPkt(&perr->TarmNo, 11, NTNET_ERRDATA);	/* �����No����11�o�C�g����ʂɒʒm���� */
			/* ��P�A�[�N�u���b�N��M�҂���Ԃ� */
			NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_BLK1;
			NtLwrCtrl.rcv_ctrl.retry = 0;
		}
		break;

/*--- Arc�u���b�N��M���ʃf�[�^���M�҂� ---*/
	default:	/* case RCVSTS_WAIT_SNDRST: */
		/* Arc�u���b�N��M���ʃf�[�^���M�v�� */
		if (send_arc_packet(NtLwrCtrl.rcv_ctrl.blk_rslt, NtLwrCtrl.rcv_ctrl.rslt_size) == TRUE) {
			/* ��P�A�[�N�u���b�N��M�҂���Ԃ� */
			NtLwrCtrl.rcv_ctrl.status = RCVSTS_WAIT_BLK1;
		}
		break;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntlwr_snd
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M���䏈��
 *| param	: uchar event - ��M����M����p�C�x���g
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static void	ntlwr_snd(uchar event, ulong msgprm)
{
	uchar	result;

	result = 0;
	switch (NtLwrCtrl.snd_ctrl.status) {
/*--- ���M�A�C�h�� ---*/
	case SNDSTS_IDLE:
		/* ���M�f�[�^�L��H */
		if (NtLwrCtrl.snd_ctrl.size[_1STBLK] == 0)
			break;
		NtLwrCtrl.snd_ctrl.retry = 0;
		NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_SNDBLK1;

/*--- ��1��ۯ�-���M�҂� ---*/
	case SNDSTS_WAIT_SNDBLK1:
		/* ��P�u���b�N���M */
		if (send_arc_packet(NtLwrCtrl.snd_ctrl.buff[_1STBLK], NtLwrCtrl.snd_ctrl.size[_1STBLK]) == TRUE) {
			NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_SNDFIN1;	/* ���M���=��1��ۯ��ް����M���ʑ҂� */
		}
		break;

/*--- ��1��ۯ�-���M�����҂� ---*/
	case SNDSTS_WAIT_SNDFIN1:
		/* ���M�����C�x���g */
		if (event == EVT_SND_OK) {
			if (NtLwrCtrl.snd_ctrl.size[_2NDBLK] == 0) {
				/* NT�ް����M�� */
				if (NtLwrCtrl.snd_ctrl.kind == NTNET_NTDATA) {
					START_SNDCTRL_TIMER(PRM_RESPONSE_TIME*1000);	/* ARC��ۯ������ް���M��ϰ�N�� */
					NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_BLKRST;	/* ���M���=Arc��ۯ���M�����ް���M�҂� */
				}
				/* FREE�ް����M�� */
				else {	/* if (NtLwrCtrl.snd_ctrl.kind == NTNET_FREEDATA) */
					NtLwrCtrl.snd_ctrl.size[_1STBLK] = 0;				/* ���M�f�[�^�J�� */
					NtLwrCtrl.snd_ctrl.size[_2NDBLK] = 0;
					NtLwrCtrl.snd_ctrl.result = NTNET_SRSLT_NORMAL;		/* ���M����=���� */
					NtLwrCtrl.snd_ctrl.status = SNDSTS_IDLE;			/* ���M���=�A�C�h�� */
				}
			}
			else {
				/* ��Q�u���b�N���M */
				if (send_arc_packet(NtLwrCtrl.snd_ctrl.buff[_2NDBLK], NtLwrCtrl.snd_ctrl.size[_2NDBLK]) == TRUE) {
					NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_SNDFIN2;	/* ���M���=��2��ۯ��ް����M�����҂� */
				}
				else {
					NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_SNDBLK2;	/* ���M���=��2��ۯ��ް����M�҂� */
				}
			}
		}
		break;

/*--- ��2��ۯ�-���M�҂� ---*/
	case SNDSTS_WAIT_SNDBLK2:
		/* ��Q�u���b�N���M */
		if (send_arc_packet(NtLwrCtrl.snd_ctrl.buff[_2NDBLK], NtLwrCtrl.snd_ctrl.size[_2NDBLK]) == TRUE) {
			NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_SNDFIN2;	/* ���M���=��2��ۯ��ް����M�����҂� */
		}
		break;

/*--- ��2��ۯ�-���M�����҂� ---*/
	case SNDSTS_WAIT_SNDFIN2:
		/* ���M�����C�x���g */
		if (event == EVT_SND_OK) {
			START_SNDCTRL_TIMER(PRM_RESPONSE_TIME*1000);		/* ARC��ۯ������ް���M��ϰ�N�� */
			NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_BLKRST;		/* ���M���=Arc��ۯ���M�����ް���M�҂� */
		}
		break;

/*--- Arc��ۯ������ް���M�҂� ---*/
	default:	/* case SNDSTS_WAIT_BLKRST: */
		/* Arc��ۯ������ް���M����� */
		if (event == EVT_RCV_BLKRST) {
			STOP_SNDCTRL_TIMER();	/* ARC��ۯ������ް���M��ϰ���� */
			/* ���ʊm�F */

			switch (result) {
			case 0x00:	/* ���� */
			case 0x01:	/* ARC��ۯ�No�ُ�-�đ��Ȃ� */
			case 0x86:	/* NT��ۯ�No�ُ�-����L�� */
				NtLwrCtrl.snd_ctrl.size[_1STBLK] = 0;				/* ���M�f�[�^�J�� */
				NtLwrCtrl.snd_ctrl.size[_2NDBLK] = 0;
				NtLwrCtrl.snd_ctrl.result = NTNET_SRSLT_NORMAL;		/* ���M����=���� */
				NtLwrCtrl.snd_ctrl.status = SNDSTS_IDLE;			/* ���M���=�A�C�h�� */
				break;
			case 0x82:	/* �ޯ̧�� */
				NtLwrCtrl.snd_ctrl.size[_1STBLK] = 0;				/* ���M�f�[�^�J�� */
				NtLwrCtrl.snd_ctrl.size[_2NDBLK] = 0;
				NtLwrCtrl.snd_ctrl.result = NTNET_SRSLT_BUFFULL;	/* ���M����=�o�b�t�@�t�� */
				NtLwrCtrl.snd_ctrl.status = SNDSTS_IDLE;			/* ���M���=�A�C�h�� */
				break;
			case 0x85:	/* NT��ۯ�No�ُ�-���񖳌� */
				NtLwrCtrl.snd_ctrl.size[_1STBLK] = 0;				/* ���M�f�[�^�J�� */
				NtLwrCtrl.snd_ctrl.size[_2NDBLK] = 0;
				NtLwrCtrl.snd_ctrl.result = NTNET_SRSLT_BLKNO;		/* ���M����=NT��ۯ�No�ُ� */
				NtLwrCtrl.snd_ctrl.status = SNDSTS_IDLE;			/* ���M���=�A�C�h�� */
				break;
			case 0x90:	/* �f�[�^���M�� */
				NtLwrCtrl.snd_ctrl.size[_1STBLK] = 0;				/* ���M�f�[�^�J�� */
				NtLwrCtrl.snd_ctrl.size[_2NDBLK] = 0;
				NtLwrCtrl.snd_ctrl.result = NTNET_SRSLT_SENDING;	/* ���M����=NT��ۯ�No�ُ� */
				NtLwrCtrl.snd_ctrl.status = SNDSTS_IDLE;			/* ���M���=�A�C�h�� */
				break;
			case 0x81:	/* ARC��ۯ�No�ُ�-�đ��v�� */
			default:
				/* ���g���C���� */
				if (NtLwrCtrl.snd_ctrl.retry < PRM_RETRY_COUNT) {
					/* ��1��ۯ��ް��đ� */
					if (send_arc_packet(NtLwrCtrl.snd_ctrl.buff[_1STBLK], NtLwrCtrl.snd_ctrl.size[_1STBLK]) == TRUE) {
						NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_SNDFIN1;	/* ���M���=��1��ۯ��ް����M���ʑ҂� */
					}
					else {
						NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_SNDBLK1;	/* ���M���=��1��ۯ��ް����M�҂� */
					}
					NtLwrCtrl.snd_ctrl.retry++;
				}
				else {
					NtLwrCtrl.snd_ctrl.size[_1STBLK] = 0;				/* ���M�f�[�^�J�� */
					NtLwrCtrl.snd_ctrl.size[_2NDBLK] = 0;
					NtLwrCtrl.snd_ctrl.result = NTNET_SRSLT_RETRY_OVER;	/* ���M����=���g���C�A�E�g */
					NtLwrCtrl.snd_ctrl.status = SNDSTS_IDLE;			/* ���M���=�A�C�h�� */
				}
				break;
			}
		}
		/* ��ѱ�Ĳ���� */
		else if (event == EVT_TIMEOUT){
			/* ARC��ۯ����ʑ҂���ѱ�Ď��͑�1��ۯ�����đ����� */
			if (send_arc_packet(NtLwrCtrl.snd_ctrl.buff[_1STBLK], NtLwrCtrl.snd_ctrl.size[_1STBLK]) == TRUE) {
				NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_SNDFIN1;	/* ���M���=��1��ۯ��ް����M���ʑ҂� */
			}
			else {
				NtLwrCtrl.snd_ctrl.status = SNDSTS_WAIT_SNDBLK1;	/* ���M���=��1��ۯ��ް����M�҂� */
			}
		}
		break;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: make_arcblk_rrslt
 *[]----------------------------------------------------------------------[]
 *| summary	: Arc�u���b�N��M���ʃf�[�^�쐬
 *| param	: uchar rslt - ���ʃf�[�^
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static void	make_arcblk_rrslt(uchar	rslt)
{
	t_ArcSndData	*psnd;
	union {
		uchar	uc[2];
		ushort	us;
	} u;

	memset(NtLwrCtrl.rcv_ctrl.blk_rslt, 0, sizeof(NtLwrCtrl.rcv_ctrl.blk_rslt));
	psnd = (t_ArcSndData *)NtLwrCtrl.rcv_ctrl.blk_rslt;
	psnd->BlkNo = 1;												/* ARC��ۯ�No */
	psnd->CmdID = CMDID_SNDDATA;									/* ����@�\ID */
	NtLwrCtrl.rcv_ctrl.rslt_size = 8;								// ARC�w�b�_�T�C�Y
	if (_is_ntnet_remote()) {
		NtLwrCtrl.rcv_ctrl.rslt_size += ARC_BLOCK_RESULT_SIZE2;
		u.us = ARC_BLOCK_RESULT_SIZE2;								/* �f�[�^�T�C�Y */
		psnd->Size[0] = u.uc[0];
		psnd->Size[1] = u.uc[1];
		psnd->BlkSts = NTNET_LAST_BLOCK;							/* ARC��ۯ��ð�� */
		psnd->SndData.data[OFS_ARCBLOCK_RESULT_ID2] = 0xff;			/* ARC�u���b�N����ID */
		psnd->SndData.data[OFS_RESULT_DATA2] = rslt;				/* ���� */
		return;
	}

	NtLwrCtrl.rcv_ctrl.rslt_size += ARC_BLOCK_RESULT_SIZE;
	u.us = ARC_BLOCK_RESULT_SIZE;									/* �f�[�^�T�C�Y */
	psnd->Size[0] = u.uc[0];
	psnd->Size[1] = u.uc[1];
	psnd->BlkSts = NTNET_LAST_BLOCK;								/* ARC��ۯ��ð�� */
	psnd->SndData.data[OFS_TERMINAL_NO] = (uchar)PRM_TERMINAL_NO;	/* �[��No */
	psnd->SndData.data[OFS_ARCBLOCK_RESULT_ID] = 0xff;				/* ARC�u���b�N����ID */
	psnd->SndData.data[OFS_RESULT_DATA] = rslt;						/* ���� */
}


/*--------------------------------------------------------------------------*/
/*			ARCNET�ʐM�w													*/
/*--------------------------------------------------------------------------*/
/*[]----------------------------------------------------------------------[]
 *|	name	: check_ArcPacket_length
 *[]----------------------------------------------------------------------[]
 *| summary	: ARC�V���[�g�p�P�b�g������
 *| param	: prm ��M�f�[�^
 *|           expected_len �ݒ�p�P�b�g��
 *| return	: �`�F�b�N����
 *[]----------------------------------------------------------------------[]*/
int check_ArcPacket_length(ulong prm, ushort expected_len)
{
	return 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntlwr_arc
 *[]----------------------------------------------------------------------[]
 *| summary	: ARC����M�Ǘ�
 *| param	: ushort event : ���b�Z�[�W�C�x���g
 *| return	: ��M����M����p�C�x���g
 *[]----------------------------------------------------------------------[]*/
static uchar	ntlwr_arc(ushort event, ulong msgprm)
{
	uchar			ret;
	ret = EVT_NONE;
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: send_arc_packet
 *[]----------------------------------------------------------------------[]
 *| summary	: ARC�p�P�b�g���M�v��
 *| param	: uchar  *pdata - ���M�f�[�^
 *| 		: ushort size   - ���M�f�[�^�T�C�Y
 *| return	: TRUE  - ���M��
 *| 		: FALSE - ���M�s��
 *[]----------------------------------------------------------------------[]*/
static BOOL	send_arc_packet(uchar *pdata, ushort size)
{
	if (NtLwrCtrl.arc_ctrl.snd_size)
		return FALSE;

	/* ���M�f�[�^�o�^ */
	NtLwrCtrl.arc_ctrl.snd_data = pdata;
	NtLwrCtrl.arc_ctrl.snd_size = size;

	/* �ǂ��ԃZ�b�g */
	if (++NtLwrCtrl.arc_ctrl.snd_oiban > 99)
		NtLwrCtrl.arc_ctrl.snd_oiban = 1;
	((t_ArcSndData *)pdata)->Oiban = NtLwrCtrl.arc_ctrl.snd_oiban;
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntlwr_MakeDispatchFlag
 *[]----------------------------------------------------------------------[]
 *| summary	: NTLWR���W���[�� �f�B�X�p�b�`�v���`�F�b�N
 *| param	: none
 *| return	: TRUE  - �����f�B�X�p�b�`�v������
 *[]----------------------------------------------------------------------[]*/
BOOL	ntlwr_MakeDispatchFlag(void)
{
	/* ARC���䃂�W���[�����`�F�b�N */
	if (NtLwrCtrl.arc_ctrl.status == ARCSTS_SND_IDLE && NtLwrCtrl.arc_ctrl.snd_size > 0) {
		return TRUE;
	}
	/* ��M�n�����䃂�W���[�����`�F�b�N */
	if (NtLwrCtrl.rcv_ctrl.status == RCVSTS_WAIT_SNDRST) {
		return TRUE;
	}
	/* ���M�n�����䃂�W���[�����`�F�b�N */
	switch (NtLwrCtrl.snd_ctrl.status) {
	case SNDSTS_WAIT_SNDBLK1:	/* ��1��ۯ����M�҂� */
		/* no break */
	case SNDSTS_WAIT_SNDBLK2:	/* ��2��ۯ����M�҂� */
		return TRUE;
	default:	/* SNDSTS_WAIT_SNDFIN1/SNDSTS_WAIT_SNDFIN2/SNDSTS_WAIT_BLKRST */
		break;
	}
	
	return FALSE;
}
