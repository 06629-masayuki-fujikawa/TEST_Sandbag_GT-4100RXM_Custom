/*[]----------------------------------------------------------------------[]
 *| System      : UT8500
 *| Module      : NT-NET�ʐM���W���[��(��ʃ��C���[)
 *[]----------------------------------------------------------------------[]
 *| Author      : machida kei
 *| Date        : 2005.08.23
 *| Update      :
 *|		2005.09.01	machida.k	���M���g���C���I�����Ȃ��s��̏C��
 *|								���M���g���C�񐔂��Œ�(�ݒ�p�����[�^�͎Q�Ƃ��Ȃ�)�Ƃ���
 *|								��M���̓��g���C�񐔂��`�F�b�N���Ȃ�
 *|								IBK�o�b�t�@FULL���ɂ�����đ���WAIT��������
 *|		2006-01-17	machida.k	�A�C�h�����׌y�� (MH544401)
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*			I N C L U D E													*/
/*--------------------------------------------------------------------------*/

#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"prm_tbl.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"common.h"
#include	"ntnet.h"
#include	"ntnetauto.h"
#include	"mnt_def.h"

/*--------------------------------------------------------------------------*/
/*			C O N S T A N T S												*/
/*--------------------------------------------------------------------------*/

/* �o�b�t�@�R�[�h */
enum {
	/* for send-> */
	NTUPR_SBUF_NORMAL,				/* ���M�ʏ�f�[�^ */
	NTUPR_SBUF_PRIOR,				/* ���M�D��f�[�^ */
	/* <-for send */
	
	/* for receive-> */
	NTUPR_RBUF_NORMAL,				/* ��M�ʏ�f�[�^ */
	NTUPR_RBUF_PRIOR,				/* ��M�D��f�[�^ */
	NTUPR_RBUF_BROADCAST,			/* ��M����f�[�^ */
	/* <-for receive */
	
	NTUPR_BUF_MAX
};

/* �{���C���[�̃X�e�[�^�X */
typedef enum {
	NTUPR_IDLE,						/* �A�C�h�� */
	NTUPR_SEND_FREE,				/* FREE�f�[�^���M�����҂� */
	NTUPR_SEND_PRIOR,				/* �D��f�[�^���M�����҂� */
	NTUPR_SEND_NORMAL				/* �ʏ�f�[�^���M�����҂� */
}eNTUPR_STS;

/* ���d�����p�X�e�[�^�X */
typedef enum {
	NTUPR_FUKUDEN_NONE,				/* ���d�������� */
	NTUPR_FUKUDEN_DATAGET,			/* �f�[�^�擾�� */
	NTUPR_FUKUDEN_DATACLR,			/* �f�[�^�폜�� */
	NTUPR_FUKUDEN_HOLDING,			/* �f�[�^�ێ��� */
}eNTUPR_FUKUDEN_STS;

/* ���M���g���C�� */
#define	NTUPR_SND_RETRY	3			/* #2 */

/*--------------------------------------------------------------------------*/
/*			S T R U C T U R E S												*/
/*--------------------------------------------------------------------------*/

/* �d������M�p�̃o�b�t�@�̎��̈� */
typedef struct {
	uchar		snormal[26000];		/* ���M�ʏ�f�[�^ */
	uchar		sprior[1000];		/* ���M�D��f�[�^ */
	uchar		rnormal[26000];		/* ��M�ʏ�f�[�^ */
	uchar		rprior[1000];		/* ��M�D��f�[�^ */
	uchar		rbroadcast[1000];	/* ��M����f�[�^ */
}t_NtUpr_TelePool;

/* NTUPR�w�p���d�f�[�^ */
typedef struct {
	eNTUPR_FUKUDEN_STS	phase;		/* ���d�����X�e�[�^�X */
	eNTNET_BUF_KIND		bufkind;	/* �f�[�^�擾��o�b�t�@ */
	t_NtNet_ClrHandle	h;			/* �f�[�^�폜�p�n���h�� */
}t_NtUpr_Fukuden;

/* NT-NET1�d���Ǘ� */
typedef struct {
	ushort				tele_len;			/* �f�[�^�� */
	uchar				*data;				/* �f�[�^�{�� */
	ushort				blknum;				/* ���M/��M�ς݃u���b�N�� */
	long				blkno_retry;		/* �u���b�NNo.�ُ�ɂ�鑗�M���g���C��(���M�o�b�t�@���̂ݎg�p) */
	long				buffull_retry;		/* IBK�o�b�t�@FULL�ɂ�鑗�M���g���C��(���M�o�b�t�@���̂ݎg�p) */
	ulong				buffull_timer;		/* IBK�o�b�t�@FULL�ɂ�鑗�M���g���C�p�^�C�}�[(���M�o�b�t�@���̂ݎg�p) */
	ushort				send_bytenum;		/* ���M�ς݃f�[�^��(���M�o�b�t�@���̂ݎg�p) */
	ushort				cur_send_bytenum;	/* ���񑗐M�f�[�^��(���M�o�b�t�@���̂ݎg�p) */
	t_NtUpr_Fukuden		fukuden;			/* ���d�����p�f�[�^ */
	ushort				UpperQueKind;		/* ���M�ް��擾�������ʁi���M�ޯ̧�̂݁j	*/
											/* "NTNET_SNDBUF_xxxx" �̒l�����B			*/
}t_NtUpr_Telegram;

/* FREE�f�[�^���M���� */
typedef struct {
	BOOL	isdata;					/* TRUE=�f�[�^���� */
	uchar	data[FREEPKT_DATASIZE];	/* FREE�f�[�^ */
}t_NtUpr_FreeCtrl;

/* ����f�[�^�\�� */
typedef struct {
	eNTUPR_STS			sts;		/* �{���C���[�̃X�e�[�^�X */
	uchar				seqno;		/* ���M�V�[�P���XNo. */
	uchar				rcv_seqno;	/* ��M�V�[�P���XNo. */
	t_NtUpr_FreeCtrl	freedata;	/* FREE�f�[�^���M����f�[�^ */
}t_NtUpr_Ctrl;

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (backuped)									*/
/*--------------------------------------------------------------------------*/

#pragma section		_UNINIT2


/* ����M�o�b�t�@���̈� */
t_NtUpr_TelePool			z_NtNetTelePool;

/* ����M�o�b�t�@ */
t_NtUpr_Telegram			z_NtUpr_Telegram[NTUPR_BUF_MAX];


#pragma section

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (not backuped)								*/
/*--------------------------------------------------------------------------*/

/* ���M����f�[�^ */
static t_NtUpr_Ctrl		z_NtUpr_Ctrl;

/* 1�u���b�N���M�pWORK */
static t_NtNetBlk		z_NtUpr_BlkWork;

char	z_NtRemoteAutoManual;
static	char	z_1minutePending;

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/

#define	_ntupr_BufSndReset(b)	(b)->blknum			= 0; \
								(b)->send_bytenum	= 0

#define	_ntupr_BufReset(b)	_ntupr_BufSndReset(b);	 \
							(b)->blkno_retry	= 0; \
							(b)->buffull_retry	= 0

#define	_ntupr_BufClr(b)	(b)->tele_len		= 0; \
							_ntupr_BufReset(b)

#define	_ntupr_NextSeqno(n)	(((n) >= 99) ? 1 : (n) + 1)

static void		ntupr_ChkSendData(t_NtUpr_Ctrl *ctrl, t_NtUpr_Telegram *telegram);
static void		ntupr_Send(t_NtUpr_Ctrl *ctrl, t_NtUpr_Telegram *telegram);
static eNTNET_RRSLT		ntupr_SetRcvNtBlk(t_NtUpr_Ctrl *ctrl, t_NtUpr_Telegram *telegram, uchar *data, ushort len);
static BOOL		ntupr_GetSndNtData(t_NtUpr_Ctrl *ctrl, t_NtUpr_Telegram *tele);
static void		ntupr_SetRcvFreeData(uchar *data);
static BOOL		ntupr_IsWaiting(t_NtUpr_Telegram *tele);
#define	_ntupr_SetRcvErrData(d)	NTBUF_SetRcvErrData(d)

static ushort	ntupr_Unpack(uchar *dst, t_NtNetBlk *ntblk, ushort blklen);
static ushort	ntupr_MkNtNetBlk(t_NtNetBlk *ntblk, t_NtUpr_Telegram *tele, uchar mode);
static int		ntupr_BufErrCode(eNTNET_BUF_KIND bufkind);

/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/

/*====================================================================================[PUBLIC]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: NTUPR_Init
 *[]----------------------------------------------------------------------[]
 *| summary	: �@�\��������
 *| param	: clr - 1=��d�ۏ؃f�[�^���N���A
 *|			        0=��d�ۏ؃f�[�^�̓N���A���Ȃ�
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTUPR_Init(uchar clr)
{
	int i;
	
	/* ����f�[�^�̃N���A */
	z_NtUpr_Ctrl.sts				= NTUPR_IDLE;
	z_NtUpr_Ctrl.freedata.isdata	= FALSE;
	z_NtUpr_Ctrl.seqno				= 0;
	z_NtUpr_Ctrl.rcv_seqno			= 0;										/* ��M���ݽNo.�ر */
	
	z_1minutePending = (char)-1;

	/* �o�b�t�@���������蓖�� */
	z_NtUpr_Telegram[NTUPR_SBUF_NORMAL].data	= z_NtNetTelePool.snormal;		/* ���M�ʏ�f�[�^ */
	z_NtUpr_Telegram[NTUPR_SBUF_PRIOR].data		= z_NtNetTelePool.sprior;		/* ���M�D��f�[�^ */
	z_NtUpr_Telegram[NTUPR_RBUF_NORMAL].data	= z_NtNetTelePool.rnormal;		/* ��M�ʏ�f�[�^ */
	z_NtUpr_Telegram[NTUPR_RBUF_PRIOR].data		= z_NtNetTelePool.rprior;		/* ��M�D��f�[�^ */
	z_NtUpr_Telegram[NTUPR_RBUF_BROADCAST].data	= z_NtNetTelePool.rbroadcast;	/* ��M����f�[�^ */
	
	/* ����M�o�b�t�@�f�[�^�̃N���A */
	for (i = 0; i < NTUPR_BUF_MAX; i++) {
		if (clr) {
			_ntupr_BufClr(&z_NtUpr_Telegram[i]);
			z_NtUpr_Telegram[i].fukuden.phase = NTUPR_FUKUDEN_NONE;
		}
		else {
			/* ���d���� */
			if (z_NtUpr_Telegram[i].fukuden.phase != NTUPR_FUKUDEN_NONE) {
				ntupr_GetSndNtData(&z_NtUpr_Ctrl, &z_NtUpr_Telegram[i]);
			}
			/* �o�b�t�@������ */
			if (z_NtUpr_Telegram[i].tele_len > 0
				&& ((t_NtNetDataHeader*)z_NtUpr_Telegram[i].data)->data_save == NTNET_DO_BUFFERING) {
				/* �f�[�^�ێ��t���O��ON�̃f�[�^�͒�d�ۏؑΏ� */
					_ntupr_BufReset(&z_NtUpr_Telegram[i]);
			}
			else {
				_ntupr_BufClr(&z_NtUpr_Telegram[i]);
			}
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTUPR_SetRcvPkt
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M�f�[�^�Z�b�g
 *| param	: data - ��M�f�[�^
 *|			  len - ��M�f�[�^��
 *|			  kind - ��M�f�[�^�̎��
 *| return	: �u���b�NNo.�`�F�b�N����
 *[]----------------------------------------------------------------------[]*/
eNTNET_RRSLT	NTUPR_SetRcvPkt(uchar *data, ushort len, eNTNET_DATA_KIND kind)
{
	eNTNET_RRSLT	result;
	
	result = NTNET_RRSLT_NORMAL;
	switch (kind) {
	case NTNET_NTDATA:		/* NT-NET�f�[�^��M */
		result = ntupr_SetRcvNtBlk(&z_NtUpr_Ctrl, z_NtUpr_Telegram, data, len);
		break;
	case NTNET_FREEDATA:	/* FREE�f�[�^��M */
		ntupr_SetRcvFreeData(data);
		break;
	case NTNET_ERRDATA:		/* �G���[�f�[�^��M */
		_ntupr_SetRcvErrData(data);
		break;
	}
	
	return result;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTUPR_ChkDispatch
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET�ʐM���W���[�� ��ʃ��C���[ �f�B�X�p�b�`�`�F�b�N
 *| param	: none
 *| return	: TRUE = Dispatch�v������
 *[]----------------------------------------------------------------------[]*/
BOOL	NTUPR_ChkDispatch(void)
{
	eNTNET_SRSLT	dummy;
	
	switch (z_NtUpr_Ctrl.sts) {
	case NTUPR_IDLE:				/* �A�C�h�� */
		if (z_NtUpr_Telegram[NTUPR_SBUF_NORMAL].tele_len > 0	/* ���M���̒ʏ�f�[�^���� */
			|| z_NtUpr_Telegram[NTUPR_SBUF_PRIOR].tele_len > 0	/* ���M���̗D��f�[�^���� */
			|| _NTBUF_SndDataExist()) {							/* �o�b�t�@���W���[���ɑ��M�f�[�^���� */
			return TRUE;
		}
		break;
	case NTUPR_SEND_FREE:			/* FREE�f�[�^���M�����҂� */
		/* no break */
	case NTUPR_SEND_PRIOR:			/* �D��f�[�^���M�����҂� */
		/* no break */
	case NTUPR_SEND_NORMAL:			/* �ʏ�f�[�^���M�����҂� */
		if (NTLWR_IsSendComplete(&dummy)) {
			return TRUE;
		}
		break;
	default:
		/* ���̃p�X�͒ʂ�Ȃ��͂� */
		break;
	}
	if ((int)z_1minutePending >= 0) {
		if (z_1minutePending != CLK_REC.minu) {
			// 1���o�߁��y���f�B���O����
			NTBUF_SetIBKPendingByID(0xff);
			z_1minutePending = (char)-1;
			return TRUE;
		}
	}
	return FALSE;
}


/*[]----------------------------------------------------------------------[]
 *|	name	: NTUPR_Main
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET�ʐM���W���[�� ��ʃ��C���[���C��
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTUPR_Main(void)
{
	/* ���M�̃X�P�W���[�����O�����s */
	ntupr_Send(&z_NtUpr_Ctrl, z_NtUpr_Telegram);
}

/*====================================================================================[PRIVATE]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_Send
 *[]----------------------------------------------------------------------[]
 *| summary	: NTNET�f�[�^���M�I�y���[�V����
 *| param	: ctrl - ����f�[�^
 *|			  telegram - �{���W���[���ŊǗ����鑗��M�o�b�t�@
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntupr_Send(t_NtUpr_Ctrl *ctrl, t_NtUpr_Telegram *telegram)
{
	t_NtUpr_Telegram *tele;
	eNTUPR_STS		sts;
	eNTNET_SRSLT	result;
	ushort			len;
	
	switch (ctrl->sts) {
	case NTUPR_IDLE:		/*--------------------------------  IDLE ----  */
		/* ���M�f�[�^�̗L�����`�F�b�N */
		ntupr_ChkSendData(ctrl, telegram);
		
		if (ctrl->freedata.isdata) {
			/* FREE�f�[�^���M */
			NTLWR_SendReq(ctrl->freedata.data, sizeof(ctrl->freedata.data), NTNET_FREEDATA);
			ctrl->sts = NTUPR_SEND_FREE;
		}
		else {
			len = 0;
			if (telegram[NTUPR_SBUF_PRIOR].tele_len > 0) {
				if (!ntupr_IsWaiting(&telegram[NTUPR_SBUF_PRIOR])) {
			/* �D��f�[�^���M */
					len = ntupr_MkNtNetBlk(&z_NtUpr_BlkWork, &telegram[NTUPR_SBUF_PRIOR], NTNET_PRIOR_DATA);
					sts = NTUPR_SEND_PRIOR;
				}
			}
			else if (telegram[NTUPR_SBUF_NORMAL].tele_len > 0) {
				if (!ntupr_IsWaiting(&telegram[NTUPR_SBUF_NORMAL])) {
			/* �ʏ�f�[�^���M */
					len = ntupr_MkNtNetBlk(&z_NtUpr_BlkWork, &telegram[NTUPR_SBUF_NORMAL], NTNET_NORMAL_DATA);
					sts = NTUPR_SEND_NORMAL;
				}
			}
			if (len > 0) {
				if (NTLWR_SendReq((uchar*)&z_NtUpr_BlkWork, len, NTNET_NTDATA)) {
					ctrl->sts = sts;
				}
			}
		}
		break;
	case NTUPR_SEND_FREE:	/*----------------------- FREE�f�[�^���M�� ----*/
		if (NTLWR_IsSendComplete(&result)) {
			ctrl->freedata.isdata = FALSE;
			ctrl->sts = NTUPR_IDLE;
		}
		break;
	case NTUPR_SEND_PRIOR:	/*----------------------- �D��f�[�^���M�� ----*/
		/* no break */
	case NTUPR_SEND_NORMAL:	/*----------------------- �ʏ�f�[�^���M�� ----*/
		if (ctrl->sts == NTUPR_SEND_PRIOR) {
			tele = &telegram[NTUPR_SBUF_PRIOR];
		}
		else {	/* ctrl->sts == NTUPR_SEND_NORMAL */
			tele = &telegram[NTUPR_SBUF_NORMAL];
		}
		
		if (NTLWR_IsSendComplete(&result)) {
			// ���u�w����x�d���̏ꍇ�A���g���C�͂����ɒʒm�̂�
			if (z_NtUpr_BlkWork.data.system_id == REMOTE_SYSTEM) {
				// OK/NG�Ɋւ�炸�d���͍폜
				// �ʒm
				if (z_NtRemoteAutoManual == REMOTE_AUTO) {
					ntautoPostResult(z_NtUpr_BlkWork.data.data_type, (uchar)result);
				} else {
					// �����e�ʒm
					IBKCtrl_SetRcvData_manu(z_NtUpr_BlkWork.data.data_type, (uchar)result);
				}
				// ���M����=�d���p��
				_ntupr_BufClr(tele);
				ctrl->sts = NTUPR_IDLE;
				break;
			}
			switch (result) {
			case NTNET_SRSLT_NORMAL:		/* ���튮�� */
				/* IBK�o�b�t�@FULL�ɂ�郊�g���C�񐔃N���A */
				tele->buffull_retry = 0;
				tele->send_bytenum += tele->cur_send_bytenum;
				if (tele->send_bytenum >= tele->tele_len) {
					_ntupr_BufClr(tele);	/* ���M����=�d���p�� */
					if (tele->fukuden.phase == NTUPR_FUKUDEN_HOLDING) {
						tele->fukuden.phase = NTUPR_FUKUDEN_DATACLR;
						ntupr_GetSndNtData(ctrl, tele);		// �����Ńo�b�t�@����폜
					}
				}
				break;
			case NTNET_SRSLT_BUFFULL:		/* IBK�o�b�t�@FULL */
				// �f�[�^�͕ێ������܂ܑ��M��~/�j�A�t�������ő��M�ĊJ
				if (_is_ntnet_remote()) {
					// buffer�w�Ƀj�A�t���ʒm
					NTBUF_SetIBKNearFullByID(z_NtUpr_BlkWork.data.data_type);
					// ���M���~�A���[�N�G���A�̓d���p��
					_ntupr_BufClr(tele);					// �o�b�t�@���̌��f�[�^�͈ێ�
					break;
				}
				/* ��莞�Ԃ�WAIT��A�����f�[�^���đ� */
				if (tele->buffull_retry < CPrmSS[S_NTN][NTPRM_BUFFULL_RETRY]) {
					if (++tele->buffull_retry >= CPrmSS[S_NTN][NTPRM_BUFFULL_RETRY]) {
						NTNET_Err(ERR_NTNET_BUFFULL_RETRY, NTERR_ONESHOT);	/* �o�b�t�@FULL�ɂ��đ��񐔒��� */
					}
				}
				_ntupr_BufSndReset(tele);
				tele->buffull_timer = LifeTimGet();
				break;
			case NTNET_SRSLT_RETRY_OVER:	/* ���g���C�񐔃I�[�o�[ */
				NTNET_Err(ERR_NTNET_ARCBLKNO_INVALID, NTERR_ONESHOT);	/* ARC�u���b�NNo.�ُ픭�� */
				/* no break */
			case NTNET_SRSLT_BLKNO:			/* NT�u���b�NNo.�ُ� */
				/* IBK�o�b�t�@FULL�ɂ�郊�g���C�񐔃N���A */
				tele->buffull_retry = 0;
				if (tele->blkno_retry++ < NTUPR_SND_RETRY) {
				/* �擪�u���b�N����đ� */
					_ntupr_BufSndReset(tele);
				}
				else {
					_ntupr_BufClr(tele);
					if (tele->fukuden.phase == NTUPR_FUKUDEN_HOLDING) {
						tele->fukuden.phase = NTUPR_FUKUDEN_DATACLR;
						ntupr_GetSndNtData(ctrl, tele);		// �����Ńo�b�t�@����폜
					}
					NTNET_Err(ERR_NTNET_DISCARD_SENDDATA, NTERR_ONESHOT);	/* ���g���C�I�[�o�[�ɂ��f�[�^�j�� */
				}
				break;
			case NTNET_SRSLT_SENDING:			/* �f�[�^���M��(�����[�g�̂�) */
				// �Y���f�[�^��1�����i���Ƃɍđ�
				// �Y���o�b�t�@�y���f�B���O�ʒm
				NTBUF_SetIBKPendingByID(z_NtUpr_BlkWork.data.data_type);
				z_1minutePending = CLK_REC.minu;
				// �d���j��
				_ntupr_BufClr(tele);					// �o�b�t�@���̌��f�[�^�͈ێ�
				break;
			}
			ctrl->sts = NTUPR_IDLE;
		}
		break;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_ChkSendData
 *[]----------------------------------------------------------------------[]
 *| summary	: �{���W���[���ŊǗ�����o�b�t�@�ɑ��M�f�[�^�����邩�`�F�b�N�B
 *|			  �Ȃ���Ώ�ʃ��W���[������f�[�^���擾����B
 *| param	: ctrl - ����f�[�^
 *|			  telegram - �{���W���[���ŊǗ����鑗��M�o�b�t�@
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntupr_ChkSendData(t_NtUpr_Ctrl *ctrl, t_NtUpr_Telegram *telegram)
{
	t_NtUpr_Telegram *tele;
	uchar nomal_chk;
	
	if (ctrl->sts != NTUPR_IDLE) {
		return;
	}
	
	/* FREE�f�[�^ */
	ctrl->freedata.isdata = NTBUF_GetSndFreeData(ctrl->freedata.data);
	if (ctrl->freedata.isdata) {
		return;
	}
	/* �D��f�[�^ */
	tele = &telegram[NTUPR_SBUF_PRIOR];
	nomal_chk = ( _is_ntnet_remote() && telegram[NTUPR_SBUF_NORMAL].tele_len > 0 );
	if( tele->tele_len <= 0 && !nomal_chk ){
		/* �茳�Ƀf�[�^���Ȃ��̂Ńo�b�t�@����V�����f�[�^�����炤 */
		tele->fukuden.bufkind = NTNET_BUF_PRIOR;
		tele->fukuden.phase = NTUPR_FUKUDEN_DATAGET;
		if (ntupr_GetSndNtData(ctrl, tele)) {
			return;
		}
	}
	
	/* �ʏ�f�[�^ */
	tele = &telegram[NTUPR_SBUF_NORMAL];
	if (tele->tele_len <= 0) {
		/* �茳�Ƀf�[�^���Ȃ��̂Ńo�b�t�@����V�����f�[�^�����炤 */
		tele->fukuden.bufkind = NTNET_BUF_NORMAL;
		tele->fukuden.phase = NTUPR_FUKUDEN_DATAGET;
		if (ntupr_GetSndNtData(ctrl, tele)) {
			return;
		}
	}
	
	/* ���M�\�ȃf�[�^���� */
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_GetSndNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: ��ʃ��W���[������f�[�^�擾���폜
 *| param	: ctrl - ����f�[�^
 *|			  telegram - �{���W���[���ŊǗ����鑗��M�o�b�t�@
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
BOOL	ntupr_GetSndNtData(t_NtUpr_Ctrl *ctrl, t_NtUpr_Telegram *tele)
{
	BOOL ret = FALSE;
	
	switch (tele->fukuden.phase) {
	case NTUPR_FUKUDEN_DATAGET:
	/* �f�[�^�擾 */
		tele->tele_len = NTBUF_GetSndNtData(tele->data, tele->fukuden.bufkind, &(tele->UpperQueKind) );
		if (tele->tele_len <= 0) {
			break;	/* �f�[�^���� */
		}
		/* ���M����f�[�^������ */
		_ntupr_BufReset(tele);
		/* �f�[�^�폜���� */
		NTBUF_ClrSndNtData_Prepare(tele->data, tele->fukuden.bufkind, &tele->fukuden.h);
		/* �V�[�P���XNo.�Z�b�g */
		if (tele->data[0] != REMOTE_SYSTEM) {
			((t_NtNetDataHeader*)(tele->data))->seq_no = ctrl->seqno;
			ctrl->seqno = (uchar)_ntupr_NextSeqno(ctrl->seqno);
		}
		if (_is_ntnet_remote() && tele->data[0] != REMOTE_SYSTEM) {
			tele->fukuden.phase = NTUPR_FUKUDEN_HOLDING;
			tele->UpperQueKind = 0xFF;	// �f�[�^����2�d�J�E���g�̖h�~
			break;						// �폜���Ȃ�
		}
		tele->fukuden.phase = NTUPR_FUKUDEN_DATACLR;
		/* no break */
	case NTUPR_FUKUDEN_DATACLR:
	/* �f�[�^�폜 */
		NTBUF_ClrSndNtData_Exec(&tele->fukuden.h);
		
		tele->fukuden.phase = NTUPR_FUKUDEN_NONE;
		ret = TRUE;
		break;
	default:
	case NTUPR_FUKUDEN_HOLDING:
		break;
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_SetRcvNtBlk
 *[]----------------------------------------------------------------------[]
 *| summary	: �u���b�NNo.�`�F�b�N�����Ď�M�f�[�^���o�b�t�@�����O
 *|			  �ŏI�u���b�N��M�ŁA��ʃo�b�t�@�Ƀf�[�^���Z�b�g����
 *| param	: ctrl - ����f�[�^
 *|			  telegram - �{���W���[���ŊǗ����鑗��M�o�b�t�@
 *|			  data - ��MNT-NET�f�[�^
 *|			  len - data�̃o�C�g��
 *| return	: �u���b�N�ԍ��`�F�b�N����
 *[]----------------------------------------------------------------------[]*/
eNTNET_RRSLT	ntupr_SetRcvNtBlk(t_NtUpr_Ctrl *ctrl, t_NtUpr_Telegram *telegram, uchar *data, ushort len)
{
	eNTNET_RRSLT	ret;
	t_NtNetBlk		*ntblk;
	t_NtUpr_Telegram	*tele;
	int				bufkind;

	ret = NTNET_RRSLT_NORMAL;
	
	if (len > 0) {
		ntblk = (t_NtNetBlk*)data;
		if (ntblk->header.terminal_no == NTNET_BROADCAST
			|| ntblk->header.mode == NTNET_PRIOR_DATA) {
		/* ���� or �D��f�[�^ */
			if (ntblk->header.terminal_no == NTNET_BROADCAST) {
			/* ����f�[�^ */
				tele	= &telegram[NTUPR_RBUF_BROADCAST];
				bufkind	= NTNET_BUF_BROADCAST;
			}
			else {
			/* �D��f�[�^ */
				tele	= &telegram[NTUPR_RBUF_PRIOR];
				bufkind	= NTNET_BUF_PRIOR;
			}
			if (ntblk->data.blk_no == 1 && ntblk->data.is_lastblk == NTNET_LAST_BLOCK) {
				_ntupr_BufClr(tele);
				tele->data[0] = ntblk->data.system_id;	/* ID3 */
				tele->data[1] = ntblk->data.data_type;	/* ID4 */
				tele->data[2] = ntblk->data.save;		/* �f�[�^�ێ��t���O */
				tele->tele_len = 3;
				tele->tele_len += ntupr_Unpack(&tele->data[tele->tele_len], ntblk, len);

				/* ============= ��M�V�[�P���XNo.�`�F�b�N(NTNET�d��) ================= */
				/* 0���͑O���M���̃V�[�P���XNo.�ȊO�A����99�ȉ��Ȃ��M�B���̑��͔j�� */
				/* ==================================================================== */
				if(	(((t_NtNetDataHeader*)(tele->data))->seq_no	==	0 					||
					((t_NtNetDataHeader*)(tele->data))->seq_no	!=	ctrl->rcv_seqno)	&&
					((t_NtNetDataHeader*)(tele->data))->seq_no	<	100
					){
					/* �V�[�P���XNo.�X�V */
					ctrl->rcv_seqno = ((t_NtNetDataHeader*)(tele->data))->seq_no;

					/* ���Z�@��荞�� */
					/* ��ʃo�b�t�@���W���[���ɏ������� */
					if (NTBUF_SetRcvNtData(tele->data, tele->tele_len, (eNTNET_BUF_KIND)bufkind) != NTNET_BUFSET_NORMAL) {
						NTNET_Err(ntupr_BufErrCode((eNTNET_BUF_KIND)bufkind), NTERR_ONESHOT);	/* �o�b�t�@�I�[�o�[���C�g���� */
					}
				}

			}
			else {
				ret = NTNET_RRSLT_BLKNO_INVALID;
			}
		}
		else if (ntblk->header.mode == NTNET_NORMAL_DATA) {
		/* �ʏ�f�[�^ */
			/* �u���b�NNo.���`�F�b�N */
			tele = &telegram[NTUPR_RBUF_NORMAL];
			if (ntblk->data.blk_no == 1 && tele->blknum > 0) {
				_ntupr_BufClr(tele);
				ret = NTNET_RRSLT_BLKNO_VALID;		/* �u���b�NNo.�ُ�(����L��) */
				NTNET_Err(ERR_NTNET_NTBLKNO_VALID, NTERR_ONESHOT);	/* �u���b�NNo.�ُ픭�� */
			}
			else if (ntblk->data.blk_no != tele->blknum + 1) {
				_ntupr_BufClr(tele);
				ret = NTNET_RRSLT_BLKNO_INVALID;	/* �u���b�NNo.�ُ�(���񖳌�) */
				NTNET_Err(ERR_NTNET_NTBLKNO_INVALID, NTERR_ONESHOT);	/* �u���b�NNo.�ُ픭�� */
			}
			else if (ntblk->data.blk_no == NTNET_BLKMAX_NORMAL
					&& ntblk->data.is_lastblk == NTNET_NOT_LAST_BLOCK) {
				_ntupr_BufClr(tele);
				ret = NTNET_RRSLT_BLKNO_INVALID;	/* �u���b�NNo.�ُ�(���񖳌�) */
				NTNET_Err(ERR_NTNET_RECV_BLK_OVER, NTERR_ONESHOT);	/* �u���b�NNo.�ُ픭�� */
			}
			
			if (ret != NTNET_RRSLT_BLKNO_INVALID) {
			/* �f�[�^�ۑ� */
				if (ntblk->data.blk_no == 1) {
				/* �擪�u���b�N */
					tele->data[0] = ntblk->data.system_id;
					tele->data[1] = ntblk->data.data_type;
					tele->data[2] = ntblk->data.save;
					tele->tele_len = 3;
				}
				tele->tele_len += ntupr_Unpack(&tele->data[tele->tele_len], ntblk, len);
				tele->blknum++;
				if (ntblk->data.is_lastblk == NTNET_LAST_BLOCK) {

					/* ============= ��M�V�[�P���XNo.�`�F�b�N(NTNET�d��) ================= */
					/* 0���͑O���M���̃V�[�P���XNo.�ȊO�A����99�ȉ��Ȃ�]���B���̑��͔j�� */
					/* ==================================================================== */
					if(	((t_NtNetDataHeader*)(tele->data))->system_id == REMOTE_SYSTEM) {
						/* ��ʃo�b�t�@���W���[���ɏ������� */
						// ���̏ꍇ�Aseq_no�͑��݂��Ȃ�
						// �܂��A�D��o�b�t�@�Ƃ���
						if (NTBUF_SetRcvNtData(tele->data, tele->tele_len, NTNET_BUF_PRIOR) != NTNET_BUFSET_NORMAL) {
							NTNET_Err(ntupr_BufErrCode(NTNET_BUF_PRIOR), NTERR_ONESHOT);	/* �o�b�t�@�I�[�o�[���C�g�ʒm */
						}
					}
					else	// �ȉ��ʏ�d����
					if(	(((t_NtNetDataHeader*)(tele->data))->seq_no	==	0 					||
						((t_NtNetDataHeader*)(tele->data))->seq_no	!=	ctrl->rcv_seqno)	&&
						((t_NtNetDataHeader*)(tele->data))->seq_no	<	100
						){
						/* �V�[�P���XNo.�X�V */
						ctrl->rcv_seqno = ((t_NtNetDataHeader*)(tele->data))->seq_no;

						/* NETDOPA_MAF�d���p�V�X�e��ID�̏ꍇMAF�֓]�� */
						/* ��ʃo�b�t�@���W���[���ɏ������� */
						if (NTBUF_SetRcvNtData(tele->data, tele->tele_len, NTNET_BUF_NORMAL) != NTNET_BUFSET_NORMAL) {
							NTNET_Err(ntupr_BufErrCode(NTNET_BUF_NORMAL), NTERR_ONESHOT);	/* �o�b�t�@�I�[�o�[���C�g�ʒm */
						}
					}
					_ntupr_BufClr(tele);
				}
			}
		}
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_SetRcvFreeData
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M����FREE�f�[�^����ʃo�b�t�@�ɃZ�b�g����
 *| param	: data - ��MFREE�f�[�^
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntupr_SetRcvFreeData(uchar *data)
{
	if (!NTBUF_SetRcvFreeData(data)) {
		NTNET_Err(ERR_NTNET_FREE_RCVBUF, NTERR_ONESHOT);	/* �o�b�t�@�I�[�o�[���C�g�ʒm */
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_IsWaiting
 *[]----------------------------------------------------------------------[]
 *| summary	: IBK�o�b�t�@FULL�ɂ�鑗�M���g���C�Ԋu�̊Ď�
 *| param	: tele - ���M�������d���f�[�^
 *| return	: TRUE - ���MWAIT��
 *[]----------------------------------------------------------------------[]*/
BOOL	ntupr_IsWaiting(t_NtUpr_Telegram *tele)
{
	if (tele->buffull_retry <= 0) {
		return FALSE;		/* WAIT���łȂ� */
	}
	if (LifePastTimGet(tele->buffull_timer) >= _To10msVal((ulong)CPrmSS[S_NTN][NTPRM_BUFFULL_TIMER])) {
		return FALSE;		/* WAIT���� */
	}
	
	return TRUE;			/* WAIT�� */
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_Unpack
 *[]----------------------------------------------------------------------[]
 *| summary	: 0�J�b�g���k���𓀂���dst�ɃR�s�[
 *| param	: dst - �𓀌�̃f�[�^�R�s�[��
 *|			  ntblk - ��M����NT-NET�u���b�N
 *|			  blklen - ntblk�̗L���f�[�^��
 *| return	: dst�ɃR�s�[�����T�C�Y
 *[]----------------------------------------------------------------------[]*/
ushort	ntupr_Unpack(uchar *dst, t_NtNetBlk *ntblk, ushort blklen)
{
	ushort len_before0cut, len_after0cut;

	len_after0cut	= blklen - (ushort)_offsetof(t_NtNetBlk, data.data);
	len_before0cut	= _MAKEWORD(ntblk->header.len_before0cut);
	
	/* 0�J�b�g��̃f�[�^���R�s�[ */
	memcpy(dst, ntblk->data.data, (ulong)len_after0cut);
	/* 0�J�b�g���̐L�� */
	while (len_after0cut < len_before0cut) {
		dst[len_after0cut++] = 0;
	}
	
	return len_before0cut;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_MkNtNetBlk
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET�u���b�N�f�[�^�쐬
 *| param	: ntblk - NT-NET�u���b�N�f�[�^�쐬��				<OUT>
 *|			  tele - �쐬����u���b�N�f�[�^�̌��f�[�^
 *|			  mode - �p�P�b�g�D�惂�[�h
 *| return	: ntblk�̗L���f�[�^��
 *[]----------------------------------------------------------------------[]*/
ushort	ntupr_MkNtNetBlk(t_NtNetBlk *ntblk, t_NtUpr_Telegram *tele, uchar mode)
{
	ushort	datalen;
	uchar	blksts;

	if (tele->tele_len <= 0) {
		return 0;
	}
	
	if (tele->blknum <= 0) {
		/* 1�u���b�N�� */
		tele->send_bytenum = 3;		/* ID3,ID4,�f�[�^�ێ��t���O */
	}
	/* ���񑗐M�f�[�^���Z�o */
	if (tele->tele_len - tele->send_bytenum > NTNET_BLKDATA_MAX) {
		datalen = NTNET_BLKDATA_MAX;
		blksts = NTNET_NOT_LAST_BLOCK;
	}
	else {
		datalen = tele->tele_len - tele->send_bytenum;
		blksts = NTNET_LAST_BLOCK;
	}
	
	/* NT-NET�f�[�^�g�ݏグ */
	memset(ntblk, 0, sizeof(t_NtNetBlk));
	ntblk->header.terminal_no		= (uchar)CPrmSS[S_NTN][NTPRM_TERMINALNO];
	ntblk->header.len_before0cut[0]	= (uchar)(datalen >> 8);
	ntblk->header.len_before0cut[1]	= (uchar)datalen;
	ntblk->header.mode				= mode;
	ntblk->header.telegram_type		= STX;
	ntblk->data.blk_no				= (uchar)++tele->blknum;
	ntblk->data.is_lastblk			= blksts;
	ntblk->data.system_id			= tele->data[0];
	ntblk->data.data_type			= tele->data[1];
	ntblk->data.save				= tele->data[2];
	// ���u����d���̏ꍇ�A�f�[�^�ێ��t���O�̍ŏ�ʃr�b�g������/�蓮������
	if (ntblk->data.system_id == REMOTE_SYSTEM) {
		// ���u�d�����M�t���O�Ɏ���/�蓮���Z�b�g
		if (ntblk->data.save & 0x80) {
			z_NtRemoteAutoManual = REMOTE_MANUAL;
		} else {
			z_NtRemoteAutoManual = REMOTE_AUTO;
		}
		ntblk->data.save &= 0x7f;
	}
	memcpy((char*)ntblk->data.data, &tele->data[tele->send_bytenum], (ulong)datalen);
	/* ���񑗐M�f�[�^����ۑ� */
	tele->cur_send_bytenum = datalen;
	
	/* 0�J�b�g�����f�[�^�����Z�o */
	while (datalen > 0) {
		if (ntblk->data.data[datalen-1] != 0) {
			break;
		}
		datalen--;
	}

	return (ushort)(datalen + _offsetof(t_NtNetBlk, data.data));
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_BufErrCode
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�t�@��ʃR�[�h�˃G���[�R�[�h�ւ̕ϊ�
 *| param	: bufkind - �o�b�t�@���
 *| return	: �o�b�t�@�I�[�o�[���C�g�G���[�R�[�h
 *[]----------------------------------------------------------------------[]*/
int		ntupr_BufErrCode(eNTNET_BUF_KIND bufkind)
{
	switch (bufkind) {
	case NTNET_BUF_BROADCAST:					/* ����f�[�^ */
		return ERR_NTNET_BROADCAST_RCVBUF;
	case NTNET_BUF_PRIOR:						/* �D��f�[�^ */
		return ERR_NTNET_PRIOR_RCVBUF;
	default:	/* case NTNET_BUF_NORMAL: */	/* �ʏ�f�[�^ */
		return ERR_NTNET_NORMAL_RCVBUF;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_BufErrCode
 *[]----------------------------------------------------------------------[]
 *| summary	: ���ݑ��M�o�b�t�@�ɂ��関���M�p�P�b�g�������
 *|			  �ǂ̃L���[�ɂ������d������Ԃ��B
 *|			  �i�w�肵���L���[�킩�甲��������p�P�b�g�������M���ۂ���Ԃ��j
 *| param	: �L���[��
 *|			  0�`10 = �����M�p�P�b�g������ȑO�ɕۑ�����Ă����L���[��
 *|			  ("NTNET_SNDBUF_xxxx" �Œ�`����Ă���l)
 *| return	: 0 = �w�肵���L���[��p�P�b�g�͖����M�ł͂Ȃ�
 *|			  1 = �w�肵���L���[��p�P�b�g�͖����M�ł���
 *[]----------------------------------------------------------------------[]*/
uchar	NTUPR_SendingPacketKindGet( ushort UpperQueKind )
{
	// �D�摗�M�o�b�t�@
	if( (0 != z_NtUpr_Telegram[ NTUPR_SBUF_PRIOR ].tele_len) &&					// �����M�ް�����
		(UpperQueKind == z_NtUpr_Telegram[ NTUPR_SBUF_PRIOR ].UpperQueKind) )	// �w�肳�ꂽ�L���[����ް��ł���
	{
		return	(uchar)1;
	}

	// �ʏ푗�M�o�b�t�@
	if( (0 != z_NtUpr_Telegram[ NTUPR_SBUF_NORMAL ].tele_len) &&				// �����M�ް�����
		(UpperQueKind == z_NtUpr_Telegram[ NTUPR_SBUF_NORMAL ].UpperQueKind) )	// �w�肳�ꂽ�L���[����ް��ł���
	{
		return	(uchar)1;
	}

	return	(uchar)0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTUPR_Init2
 *[]----------------------------------------------------------------------[]
 *| summary	: �^�]�r���ōs���@�\�������i�����e����̃I�[���N���A�j
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTUPR_Init2( void )
{
	int i;
	
	/* ����f�[�^�̃N���A */
	z_NtUpr_Ctrl.sts				= NTUPR_IDLE;
	z_NtUpr_Ctrl.freedata.isdata	= FALSE;
	z_NtUpr_Ctrl.seqno				= 0;
	z_NtUpr_Ctrl.rcv_seqno			= 0;										/* ��M���ݽNo.�ر */
	
	/* ����M�o�b�t�@�f�[�^�̃N���A */
	for (i = 0; i < NTUPR_BUF_MAX; i++) {
		_ntupr_BufClr(&z_NtUpr_Telegram[i]);
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntupr_NtNetDopaSndBufCheck_Del
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M�ޯ̧�������/�ر
 *| param	: bufkind 1:�D�摗�M�ޯ̧ 0:�ʏ푗�M�ޯ̧
 *|			  status  1:���M�ޯ̧������ 0:���M�ޯ̧�̸ر
 *|
 *| return	: ���M�ޯ̧�ɖ����M�ް����Ȃ��ꍇ�F0  
 *|			  ���M�ޯ̧�ɖ����M�ް�������ꍇ�F1
 *|			  ���M�ޯ̧�ر�̏ꍇ(���)		 �F0
 *[]----------------------------------------------------------------------[]*/
ushort		ntupr_NtNetDopaSndBufCheck_Del(uchar bufkind, uchar status)
{
	ushort res = 0;
	if(bufkind) {
		if(status){
			if(z_NtUpr_Telegram[NTUPR_SBUF_PRIOR].tele_len  > 0)
				res = 1;
		}
	}else{
		if(status){
			if(z_NtUpr_Telegram[NTUPR_SBUF_NORMAL].tele_len  > 0)
				res = 1;
		}else{
			_ntupr_BufClr(&z_NtUpr_Telegram[NTUPR_SBUF_PRIOR]);
			_ntupr_BufClr(&z_NtUpr_Telegram[NTUPR_SBUF_NORMAL]);
		}
	}
		
	return res;
}
