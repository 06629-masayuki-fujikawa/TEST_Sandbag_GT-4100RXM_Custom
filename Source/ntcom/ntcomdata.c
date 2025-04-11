// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
// MH364300 GG119A03 (S) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
		// �{�t�@�C����ǉ�			nt task(NT-NET�ʐM������)
// MH364300 GG119A03 (E) NTNET�ʐM����Ή��i�W��UT4000�FMH341111���p�j
/*[]----------------------------------------------------------------------[]
 *|	filename: NTComData.c
 *[]----------------------------------------------------------------------[]
 *| summary	: NTNET����M�f�[�^�Ǘ��@�\
 *| author	: machida.k
 *| date	: 2005.06.07
 *| Update  : 2005-06-18	#001	J.Mitani	�d�l�ύX�ɂ�著�M�p�d���̃u���b�N�`�F�b�N���@��ύX
 *|		2005.07.05	J.Mitani	#002	��d�ۏ؏����ǉ�
 *|		2005.09.13	machida.k	#003	��d�ۏ؃f�[�^�d�l�ύX
 *|										(���F�D��f�[�^�ȊO��ۏ؁@���@�V�F�f�[�^�ێ��t���O=ON�̃f�[�^��ۏ�)
 *|								#004	��d�ۏ؏����ǉ�
 *|								#005	���M�p�D��f�[�^�o�b�t�@��[�������m�ۂ���
 *|								#006	bug fix
 *|		2005-12-20	machida.k	RAM���L��
 *|		2006-01-11	machida.k	#007	bug fix(�u���b�NNo.�ُ�Ǝ�M�u���b�N���I�[�o�[�̃G���[�R�[�h��ʂɂ���)
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*			I N C L U D E													*/
/*--------------------------------------------------------------------------*/
#include	<string.h>
#include	<stdlib.h>
#include	"system.h"
#include	"common.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"ntnet.h"
#include	"ntcom.h"
#include	"ntcomdef.h"


/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/
#define	_NTComData_CheckBuffer(buf)	if ((buf)->fixed && (buf)->blknum <= 0) (buf)->fixed = FALSE	/* #004 */

static void			NTComData_ReconstructScheduler(T_NT_TELEGRAM_LIST *scheduler, T_NT_BUFFER *buffer, T_NT_FUKUDEN *fukuden);

static eNT_DATASET	NTComData_SetSndBlk(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk);
static eNT_DATASET	NTComData_SetRcvBlk(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk, BOOL *fixed);

static void			NTComData_SetRcvTeleSchedule(int terminal);
static int			NTComData_PeekRcvTeleSchedule(void);
/* #006-> */
static void			NTComData_DeleteRcvTeleSchedule(void);
/* <-#006 */

static BOOL			NTComData_AddBlk(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk);
static BOOL			NTComData_CreateTelegramInfo(T_NT_BUFFER *buffer, T_NT_TELEGRAM *telegram);
static BOOL			NTComData_GetBlkData(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk, int blk_no);
static void			NTComData_BufferClear(T_NT_BUFFER *buffer);

static BOOL			NTComData_SndBufClean(T_NT_BUFFER *buffer, eNT_STS terminal_failed, int terminal);
static BOOL			NTComData_ClrNotBufferingData(T_NT_BUFFER *buffer);							/* #003 */
static BOOL			NTComData_ChkBlkNo(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk);

static void			NTComData_CheckPowerFailure(void);			/* #002 */
static void			NTComData_Unpack0Cut(uchar *ucData);

/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/

/*====================================================================================[PUBLIC]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_Init()
 *[]----------------------------------------------------------------------[]
 *| summary	: �f�[�^�Ǘ����W���[�����̃f�[�^�S�ăN���A
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_Clear(void)
{
//	int i;

	/* NT�l�b�g�ւ̒ʏ�f�[�^���M�p�o�b�t�@ */
	NTComData_InitBuffer(&NTCom_SndBuf_Normal,
						NTCom_BufMemPool.snd_pool_normal,
						_countof(NTCom_BufMemPool.snd_pool_normal));
	NTCom_SndBuf_Normal.kind = NT_DATA_KIND_NORMAL_SND;

	/* NT�l�b�g����̒ʏ�f�[�^��M�p�o�b�t�@ */
	NTComData_InitBuffer(&NTCom_RcvBuf_Normal,
						NTCom_BufMemPool.rcv_pool_normal,
						_countof(NTCom_BufMemPool.rcv_pool_normal));
	NTCom_RcvBuf_Normal.kind = NT_DATA_KIND_NORMAL_RCV;

//	NTCom_RcvTeleSchedule.top		= 0;
	NTCom_RcvTeleSchedule.read		= 0;
//	NTCom_RcvTeleSchedule.telenum	= 0;
	NTCom_RcvTeleSchedule.write		= 0;

	/* NT�l�b�g�ւ̗D��f�[�^���M�p�o�b�t�@ */
	NTComData_InitBuffer(&NTCom_SndBuf_Prior,
						NTCom_BufMemPool.snd_pool_prior,
						_countof(NTCom_BufMemPool.snd_pool_prior));
	NTCom_SndBuf_Prior.kind = NT_DATA_KIND_PRIOR_SND;

	/* NT�l�b�g����̗D��f�[�^��M�p�o�b�t�@ */
	NTComData_InitBuffer(&NTCom_RcvBuf_Prior,
					NTCom_BufMemPool.rcv_pool_prior,
					_countof(NTCom_BufMemPool.rcv_pool_prior));
	NTCom_RcvBuf_Prior.kind = NT_DATA_KIND_PRIOR_RCV;
/* NT�l�b�g�ւ̓���f�[�^���M�p�o�b�t�@ */
	NTComData_InitBuffer(&NTCom_SndBuf_Broadcast,
					NTCom_BufMemPool.snd_pool_broadcast,
					_countof(NTCom_BufMemPool.snd_pool_broadcast));
	NTCom_SndBuf_Broadcast.kind = NT_DATA_KIND_BROADCAST_SND;
/* NT�l�b�g����̓���f�[�^��M�p�o�b�t�@ */
	NTComData_InitBuffer(&NTCom_RcvBuf_Broadcast,
					NTCom_BufMemPool.rcv_pool_broadcast,
					_countof(NTCom_BufMemPool.rcv_pool_broadcast));
	NTCom_RcvBuf_Broadcast.kind = NT_DATA_KIND_BROADCAST_RCV;

/* #004-> */
/* ��d�ۏ؏�������f�[�^ */
	NTCom_Fukuden.scheduler		= NT_FUKUDEN_SCHEDULER_NONE;
/* <-#004 */
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_Start()
 *[]----------------------------------------------------------------------[]
 *| summary	: �f�[�^�Ǘ����W���[��������
 *|			  terminal - �����ݒ�f�[�^
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_Start(void)
{
	BOOL	ret;

	/* �ʏ�o�b�t�@��d�ۏ؏��� */
	NTComData_CheckPowerFailure();		/* #004 �֐��������ړ� */

	/* �[���ڑ���� */
	NTComData_Ctrl.terminal_exists = FALSE;
	/* NTCom_InitData.ExecMode == NT_SLAVE */
	NTComData_Ctrl.terminal_exists = TRUE;

	/* �[���ُ��� */
	NTComData_Ctrl.terminal_status = NT_NORMAL;

// #003,#004->
	/* ��d�ۏؑΏۊO�̑���M�f�[�^���N���A */
	/* �ʏ�f�[�^�o�b�t�@ */
	ret = NTComData_ClrNotBufferingData(&NTCom_SndBuf_Normal);	// NT-NET�ւ̑��M�p
	if(ret) {
		if(NTComOpeRecvNormalDataBuf.fixed) {
			NTComOpeRecvNormalDataBuf.fixed = FALSE;
		}
	}
	NTComData_ClrNotBufferingData(&NTCom_RcvBuf_Normal);	// NT-NET����̎�M�p

	/* �ʏ��M�f�[�^��ARCNET�ւ̑��M�X�P�W���[��(��d�ۏؑΏۊO�ƂȂ薳���ɂȂ����f�[�^�����폜����) */
	NTCom_Fukuden.scheduler = NT_FUKUDEN_SCHEDULER_START;
	NTComData_ReconstructScheduler(&NTCom_RcvTeleSchedule, &NTCom_RcvBuf_Normal, &NTCom_Fukuden);

	/* �D��f�[�^�o�b�t�@ */
/* #005-> */
	ret = NTComData_ClrNotBufferingData(&NTCom_SndBuf_Prior);			// NT-NET�ւ̑��M�p
	if(ret) {
		if(NTComOpeRecvPriorDataBuf.fixed) {
			NTComOpeRecvPriorDataBuf.fixed = FALSE;
		}
	}
/* <-#005 */
	NTComData_ClrNotBufferingData(&NTCom_RcvBuf_Prior);			// NT-NET����̎�M�p

	/* ����f�[�^�o�b�t�@ */
	NTComData_ClrNotBufferingData(&NTCom_SndBuf_Broadcast);		// NT-NET�ւ̑��M�p
	NTComData_ClrNotBufferingData(&NTCom_RcvBuf_Broadcast);		// NT-NET����̎�M�p
// <-#003,#004

	/* �G���[��� */
	NTCom_Err.top = 0;
	NTCom_Err.num = 0;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_SetSndBlock()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M�o�b�t�@�Ƀf�[�^�Z�b�g
 *| param	: blk - �o�b�t�@�ɃZ�b�g����u���b�N�f�[�^		<IN/OUT>
 *| return	: eNT_DATASET�^
 *[]----------------------------------------------------------------------[]*/
eNT_DATASET	NTComData_SetSndBlock(T_NT_BLKDATA *blk)
{
	int terminal;

	terminal = blk->data[NT_OFS_TERMINAL_NO];

	if (terminal == NT_BROADCAST) {
	/* ����f�[�^ */
		return NTComData_SetSndBlk(&NTCom_SndBuf_Broadcast, blk);
	}
	else if (blk->data[NT_OFS_PACKET_MODE] == NT_PRIOR_DATA) {
	/* �D��f�[�^ */
/* #005-> */
		if (terminal == NT_ALL_TERMINAL) {
			/* �S�[�����̓d�����쐬 */
			if (NTComData_Ctrl.terminal_exists) {
				if (NTCom_SndBuf_Prior.fixed) {
					/* �ЂƂł��o�b�t�@�t���ł���΂��̓d���͕ۑ����Ȃ� */
					return NT_DATA_NO_MORE_TELEGRAM;
				}
			}
			if (NTComData_Ctrl.terminal_exists) {
				blk->data[NT_OFS_TERMINAL_NO] = 1;
				NTComData_SetSndBlk(&NTCom_SndBuf_Prior, blk);
			}
			return NT_DATA_NORMAL;
		}
		else {
			return NTComData_SetSndBlk(&NTCom_SndBuf_Prior, blk);
		}

/* <-#005 */
	}
	else {	/* blk->data[NT_OFS_PACKET_MODE] == NT_NORMAL_DATA */
	/* �ʏ�f�[�^ */
		if (terminal == NT_ALL_TERMINAL) {
			/* �d�����쐬 */
			if (NTComData_Ctrl.terminal_exists) {
				if (NTCom_SndBuf_Normal.fixed) {
					/* �ЂƂł��o�b�t�@�t���ł���΂��̓d���͕ۑ����Ȃ� */
					return NT_DATA_NO_MORE_TELEGRAM;
				}
			}
			if (NTComData_Ctrl.terminal_exists) {
				blk->data[NT_OFS_TERMINAL_NO] = 1;
				NTComData_SetSndBlk(&NTCom_SndBuf_Normal, blk);
			}
			return NT_DATA_NORMAL;
		}
		else {
			return NTComData_SetSndBlk(&NTCom_SndBuf_Normal, blk);
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_PeekSndTele_Normal()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M�\�Ȓʏ�f�[�^�d���̏����擾
 *| param	: terminal - ����[���ԍ�
 *|						(�]�Ǔ��쎞�͎��[��No.)
 *|			  telegram - �d�����				<OUT>
 *| return	: TRUE - �d���L��
 *|			  FALSE - �d���Ȃ�
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_PeekSndTele_Normal(int terminal, T_NT_TELEGRAM *telegram)
{
	if (terminal < 1) {
		return FALSE;
	}

	return NTComData_CreateTelegramInfo(&NTCom_SndBuf_Normal, telegram);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_PeekSndTele_Prior()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M�\�ȗD��f�[�^�d���̏����擾
 *| param	: terminal - ����[���ԍ�
 *|						(�]�Ǔ��쎞�͎��[��No.)
 *|			  telegram - �d�����				<OUT>
 *| return	: TRUE - �d���L��
 *|			  FALSE - �d���Ȃ�
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_PeekSndTele_Prior(int terminal, T_NT_TELEGRAM *telegram)
{
	if (terminal < 1) {
		return FALSE;
	}

	return NTComData_CreateTelegramInfo(&NTCom_SndBuf_Prior, telegram);
}
/* <-#005 */

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_PeekSndTele_Broadcast()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M�\�ȓ���f�[�^�d���̏����擾
 *| param	: telegram - �d�����				<OUT>
 *| return	: TRUE - �d���L��
 *|			  FALSE - �d���Ȃ�
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_PeekSndTele_Broadcast(T_NT_TELEGRAM *telegram)
{
	return NTComData_CreateTelegramInfo(&NTCom_SndBuf_Broadcast, telegram);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_SetRcvBlock()
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M�o�b�t�@�Ƀf�[�^�Z�b�g
 *| param	: blk - �u���b�N�f�[�^				<IN>
 *| return	: eNT_DATASET�^
 *[]----------------------------------------------------------------------[]*/
eNT_DATASET	NTComData_SetRcvBlock(T_NT_BLKDATA *blk)
{
	int		terminal;
	BOOL	fixed;
	eNT_DATASET ret;

	terminal = blk->data[NT_OFS_TERMINAL_NO];

	if (terminal == NT_BROADCAST) {
		/* ����f�[�^ */
		ret = NTComData_SetRcvBlk(&NTCom_RcvBuf_Broadcast, blk, &fixed);
	}
	else if (blk->data[NT_OFS_PACKET_MODE] == NT_PRIOR_DATA) {
		/* �D��f�[�^ */
		ret = NTComData_SetRcvBlk(&NTCom_RcvBuf_Prior, blk, &fixed);
	}
	else {	/* blk->data[NT_OFS_PACKET_MODE] == NT_NORMAL_DATA */
		/* �ʏ�f�[�^ */
		ret = NTComData_SetRcvBlk(&NTCom_RcvBuf_Normal, blk, &fixed);
		if (fixed) {
			NTComData_SetRcvTeleSchedule(terminal);
		}
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_PeekRcvTele_Normal()
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M���������ʏ�f�[�^�d���̏����擾
 *| param	: telegram - �d�����				<OUT>
 *| return	: TRUE - �d���L��
 *|			  FALSE - �d���Ȃ�
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_PeekRcvTele_Normal(T_NT_TELEGRAM *telegram)
{
	int terminal;

	terminal = NTComData_PeekRcvTeleSchedule();
	if (0 < terminal) {
		return NTComData_CreateTelegramInfo(&NTCom_RcvBuf_Normal, telegram);
	}

	return FALSE;		/* #006 */
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_PeekRcvTele_Prior()
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M���������D��f�[�^�d���̏����擾
 *| param	: telegram - �d�����				<OUT>
 *| return	: TRUE - �d���L��
 *|			  FALSE - �d���Ȃ�
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_PeekRcvTele_Prior(T_NT_TELEGRAM *telegram)
{
	return NTComData_CreateTelegramInfo(&NTCom_RcvBuf_Prior, telegram);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_PeekRcvTele_Broadcast()
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M������������f�[�^�d���̏����擾
 *| param	: telegram - �d�����				<OUT>
 *| return	: TRUE - �d���L��
 *|			  FALSE - �d���Ȃ�
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_PeekRcvTele_Broadcast(T_NT_TELEGRAM *telegram)
{
	return NTComData_CreateTelegramInfo(&NTCom_RcvBuf_Broadcast, telegram);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_GetRecvBlock()
 *[]----------------------------------------------------------------------[]
 *| summary	: �u���b�N�|�C���^�̈ʒu�ɂ���f�[�^���擾����
 *| param	: telegram - ����f�[�^�d�����		<IN/OUT>
 *|			  blk - �u���b�N�f�[�^				<OUT>
 *| return	: [0 �� X] : �Y���d���̎c��u���b�N��(0=�c��u���b�N�Ȃ�)
 *|			  [0 �� X] : �p�����[�^�G���[
 *[]----------------------------------------------------------------------[]*/
int		NTComData_GetRecvBlock(T_NT_TELEGRAM *telegram, T_NTCom_SndWork *sndWork)
{
	uchar i;
	union {
		uchar	uc[2];
		ushort	us;
	}u;

	memset(sndWork, 0, sizeof(T_NTCom_SndWork));

	switch (telegram->kind) {
	case NT_DATA_KIND_NORMAL_RCV:
		sndWork->len = 0;
		if(NTComOpeSendNormalDataBuf.count == 0) {
			return -1;
		}
		for(i=0; NTComOpeSendNormalDataBuf.count > 0; i++) {
			u.uc[0] = NTComOpeSendNormalDataBuf.ntDataBuf[NTComOpeSendNormalDataBuf.readIndex][14];
			u.uc[1] = NTComOpeSendNormalDataBuf.ntDataBuf[NTComOpeSendNormalDataBuf.readIndex][15];
			if(i == 0) {
				u.us+= 3;
				memcpy(sndWork->sendData, &NTComOpeSendNormalDataBuf.ntDataBuf[NTComOpeSendNormalDataBuf.readIndex][22], u.us);
			}
			else {
				memcpy(&sndWork->sendData[sndWork->len], &NTComOpeSendNormalDataBuf.ntDataBuf[NTComOpeSendNormalDataBuf.readIndex][25], u.us);
			}
			sndWork->len += u.us;

			++NTComOpeSendNormalDataBuf.readIndex;
			if(NTComOpeSendNormalDataBuf.readIndex == NT_NORMAL_BLKNUM_MAX) {
				NTComOpeSendNormalDataBuf.readIndex = 0;
			}
			--NTComOpeSendNormalDataBuf.count;				// �f�[�^�i�[�����Z
		}
		break;
	case NT_DATA_KIND_PRIOR_RCV:
		u.uc[0] = NTCom_RcvBuf_Prior.blkdata[telegram->curblk].data[14];
		u.uc[1] = NTCom_RcvBuf_Prior.blkdata[telegram->curblk].data[15];
		sndWork->len = u.us;
		memcpy(sndWork->sendData,
				&NTCom_RcvBuf_Prior.blkdata[telegram->curblk].data,
				NTCom_RcvBuf_Prior.blkdata[telegram->curblk].len);
		break;
	case NT_DATA_KIND_BROADCAST_RCV:
		u.uc[0] = NTCom_RcvBuf_Broadcast.blkdata[telegram->curblk].data[14];
		u.uc[1] = NTCom_RcvBuf_Broadcast.blkdata[telegram->curblk].data[15];
		sndWork->len = u.us;
		sndWork->len = NTCom_RcvBuf_Broadcast.blkdata[telegram->curblk].len;
		memcpy(sndWork->sendData,
				&NTCom_RcvBuf_Broadcast.blkdata[telegram->curblk].data,
				NTCom_RcvBuf_Broadcast.blkdata[telegram->curblk].len);
		break;
	default:
		return -1;
	}

	return telegram->blknum - telegram->curblk;
}

/*[]----------------------------------------------------------------------[]*
 *|             NTComData_Unpack0Cut()
 *[]----------------------------------------------------------------------[]*
 *|	0�J�b�g���ꂽ�f�[�^�𕜌�����
 *[]----------------------------------------------------------------------[]*
 *|	patam	void
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	NTComData_Unpack0Cut(uchar *ucData)
{

	ushort uiBefore0Cut;
	ushort uiAfter0Cut;

	union {
		uchar	uc[2];
		ushort	us;
	}u;

	u.uc[0] = ucData[5];
	u.uc[1] = ucData[6];

	// 0�J�b�g��̃T�C�Y���擾
	uiAfter0Cut = u.us - sizeof(t_NtNetBlkHeader) + 5;

	u.uc[0] = ucData[14];
	u.uc[1] = ucData[15];

	// 0�J�b�g�O�̃T�C�Y���擾
	uiBefore0Cut = u.us;

	while (uiBefore0Cut > uiAfter0Cut) {
		ucData[uiAfter0Cut + sizeof(t_NtNetBlkHeader) + 5] = 0x00;
		uiAfter0Cut++;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_GetSendBlock()
 *[]----------------------------------------------------------------------[]
 *| summary	: �u���b�N�|�C���^�̈ʒu�ɂ���f�[�^���擾����
 *| param	: telegram - ����f�[�^�d�����		<IN/OUT>
 *|			  blk - �u���b�N�f�[�^				<OUT>
 *| return	: [0 �� X] : �Y���d���̎c��u���b�N��(0=�c��u���b�N�Ȃ�)
 *|			  [0 �� X] : �p�����[�^�G���[
 *[]----------------------------------------------------------------------[]*/
int		NTComData_GetSendBlock(T_NT_TELEGRAM *telegram, T_NT_BLKDATA *blk)
{
	BOOL ret;

	switch (telegram->kind) {
	case NT_DATA_KIND_NORMAL_SND:
		ret = NTComData_GetBlkData(&NTCom_SndBuf_Normal, blk, telegram->curblk);
		break;
	case NT_DATA_KIND_PRIOR_SND:
		ret = NTComData_GetBlkData(&NTCom_SndBuf_Prior, blk, telegram->curblk);	/* #005 */
		break;
	case NT_DATA_KIND_BROADCAST_SND:
		ret = NTComData_GetBlkData(&NTCom_SndBuf_Broadcast, blk, telegram->curblk);
		break;
	default:
		return -1;
	}

	if (ret) {
		return telegram->blknum - telegram->curblk;
	}

	return -1;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_SeekBlock()
 *[]----------------------------------------------------------------------[]
 *| summary	: �u���b�N�|�C���^���ړ�����
 *| param	: telegram - ����f�[�^�d�����		<IN/OUT>
 *|			  offset - origin����̃I�t�Z�b�g
 *|			  origin - �����ʒu
 *|							NT_SEEK_SET - �d���擪
 *|							NT_SEEK_CUR - ���݈ʒu
 *|							NT_SEEK_END - �d���I�[
 *| return	: [0 �� X] : �Y���d���̎c��u���b�N��(0=�c��u���b�N�Ȃ�)
 *|			  [0 �� X] : �p�����[�^�G���[
 *[]----------------------------------------------------------------------[]*/
int		NTComData_SeekBlock(T_NT_TELEGRAM *telegram, int offset, eNT_SEEK origin)
{
	switch (origin) {
	case NT_SEEK_SET:
		telegram->curblk = offset;
		break;
	case NT_SEEK_CUR:
		telegram->curblk += offset;
		break;
	case NT_SEEK_END:
		telegram->curblk = telegram->blknum + offset;
		break;
	default:
		return -1;
	}
	if (telegram->curblk < 0) {
		telegram->curblk = 0;
	}
	else if (telegram->curblk > telegram->blknum) {
		telegram->curblk = telegram->blknum;
	}

	return telegram->blknum - telegram->curblk;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_DeleteTelegram()
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�t�@����d���f�[�^���폜����
 *| param	: telegram - �폜�ΏۂƂȂ�d��		<IN>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_DeleteTelegram(T_NT_TELEGRAM *telegram)
{
	switch (telegram->kind) {
	case NT_DATA_KIND_NORMAL_SND:		/* �ʏ�f�[�^�o�b�t�@(���M)�̓d�� */
		NTComData_BufferClear(&NTCom_SndBuf_Normal);
		NTComOpeRecvNormalDataBuf.fixed = FALSE;
		break;
	case NT_DATA_KIND_NORMAL_RCV:		/* �ʏ�f�[�^�o�b�t�@(��M)�̓d�� */
		NTComData_BufferClear(&NTCom_RcvBuf_Normal);
		NTComData_DeleteRcvTeleSchedule();	/* �X�P�W���[���̏����N���A */
		break;
	case NT_DATA_KIND_PRIOR_SND:		/* �D��f�[�^�o�b�t�@(���M)�̓d�� */
		NTComData_BufferClear(&NTCom_SndBuf_Prior);		/* #005 */
		NTComOpeRecvPriorDataBuf.fixed = FALSE;
		break;
	case NT_DATA_KIND_PRIOR_RCV:		/* �D��f�[�^�o�b�t�@(��M)�̓d�� */
		NTComData_BufferClear(&NTCom_RcvBuf_Prior);
		break;
	case NT_DATA_KIND_BROADCAST_SND:	/* ����f�[�^�o�b�t�@(���M)�̓d�� */
		NTComData_BufferClear(&NTCom_SndBuf_Broadcast);
		break;
	case NT_DATA_KIND_BROADCAST_RCV:	/* ����f�[�^�o�b�t�@(��M)�̓d�� */
		NTComData_BufferClear(&NTCom_RcvBuf_Broadcast);
		break;
	default:
		break;
	}
	/* �d�����\���̂𖳌��� */
	NTComData_ResetTelegram(telegram);
}

/* #004-> */
/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_IsValidTelegram()
 *[]----------------------------------------------------------------------[]
 *| summary	: �p�����[�^��telegram���L���f�[�^�ɑΉ�������̂��ǂ������`�F�b�N����
 *| param	: telegram - �`�F�b�N�Ώ�		<IN/OUT>
 *| return	: TRUE - �L��
 *|			  FALSE - ����
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_IsValidTelegram(T_NT_TELEGRAM *telegram)
{
	T_NT_BUFFER *buffer;

	if (telegram->terminal <= 0) {
		if (telegram->terminal != NT_BROADCAST
			|| (telegram->kind != NT_DATA_KIND_BROADCAST_SND && telegram->kind != NT_DATA_KIND_BROADCAST_RCV)) {
			NTComData_ResetTelegram(telegram);
			return FALSE;					// �[���ԍ��ُ�
		}
	}

	switch (telegram->kind) {
	case NT_DATA_KIND_NORMAL_SND:
		buffer = &NTCom_SndBuf_Normal;
		break;
	case NT_DATA_KIND_NORMAL_RCV:
		buffer = &NTCom_RcvBuf_Normal;
		break;
	case NT_DATA_KIND_PRIOR_SND:
		buffer = &NTCom_SndBuf_Prior;
		break;
	case NT_DATA_KIND_PRIOR_RCV:
		buffer = &NTCom_RcvBuf_Prior;
		break;
	case NT_DATA_KIND_BROADCAST_SND:
		buffer = &NTCom_SndBuf_Broadcast;
		break;
	case NT_DATA_KIND_BROADCAST_RCV:
		buffer = &NTCom_RcvBuf_Broadcast;
		break;
	default:
		NTComData_ResetTelegram(telegram);
		return FALSE;					// �d����ʈُ�
	}

	if (!buffer->fixed) {
		NTComData_ResetTelegram(telegram);
		return FALSE;					// telegram�������o�b�t�@�Ɋm��d������
	}

	if (telegram->curblk < 0) {
		return FALSE;					// ����telegaram
	}

	return TRUE;
}
/* <-#004 */

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_SetTerminalStatus()
 *[]----------------------------------------------------------------------[]
 *| summary	: �[���ڑ���ԕω��ʒm
 *| param	: terminal - �ʐM��Q�����������[���̒[��No.
 *|						(�]�Ǔ��쎞�͎��[��No.)
 *|			  sts - eNT_STS�^
 *| return	: TRUE as success
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_SetTerminalStatus(int terminal, eNT_STS sts, T_NT_TELEGRAM *telegram_normal, T_NT_TELEGRAM *telegram_prior)
{
	if (terminal <= 0) {
		return FALSE;
	}

	NTComData_Ctrl.terminal_status = sts;
	if (sts == NT_ABNORMAL) {
		/* �o�b�t�@�č\�� */
		/* �ʏ�f�[�^���M�o�b�t�@ */
		if (NTComData_SndBufClean(&NTCom_SndBuf_Normal, NTComData_Ctrl.terminal_status, terminal)) {
			if (telegram_normal != NULL) {
				NTComData_ResetTelegram(telegram_normal);	/* �o�b�t�@�N���A���ꂽ��d���������Z�b�g */
				if(NTComOpeRecvNormalDataBuf.fixed) {
					NTComOpeRecvNormalDataBuf.fixed = FALSE;
				}
			}
		}
		/* �D��f�[�^���M�o�b�t�@ */
		if (NTComData_SndBufClean(&NTCom_SndBuf_Prior, NTComData_Ctrl.terminal_status, terminal)) {	/* #005 */
			if (telegram_prior != NULL) {
				NTComData_ResetTelegram(telegram_prior);	/* �o�b�t�@�N���A���ꂽ��d���������Z�b�g */
				if(NTComOpeRecvPriorDataBuf.fixed) {
					NTComOpeRecvPriorDataBuf.fixed = FALSE;
				}
			}
		}
		/* ����f�[�^�͂��Ȃ�(�ʐM�ُ픭�����̓G���[�f�[�^(���񉞓�����)�𔭐�������d�l�̂���) */
	}

	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_GetBufferStatus()
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�t�@��Ԏ擾
 *| param	: bufsts - �o�b�t�@���				<OUT>
 *|			  terminal - �ʏ�f�[�^�o�b�t�@�̏�Ԃ��~�����[��No.
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_GetBufferStatus(T_NT_BUF_STATUS *bufsts, int terminal)
{
	/* �ʏ�f�[�^�o�b�t�@(���M�p) */
	bufsts->normal_s		= NTCom_SndBuf_Normal.fixed;
	/* �ʏ�f�[�^�o�b�t�@(��M�p) */
	bufsts->normal_r		= NTCom_RcvBuf_Normal.fixed;
	/* �D��f�[�^�o�b�t�@(���M�p) */
	bufsts->prior_s			= NTCom_SndBuf_Prior.fixed;		/* #005 */
	/* �D��f�[�^�o�b�t�@(��M�p) */
	bufsts->prior_r			= NTCom_RcvBuf_Prior.fixed;
	/* ����f�[�^�o�b�t�@(���M�p) */
	bufsts->broadcast_s		= NTCom_SndBuf_Broadcast.fixed;
	/* ����f�[�^�o�b�t�@(��M�p) */
	bufsts->broadcast_r		= NTCom_RcvBuf_Broadcast.fixed;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_GetSndBufferCount()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M�o�b�t�@�Ɋ܂܂��u���b�N�����擾����
 *| param	: bufcnt - �o�b�t�@�Ɋ܂܂��u���b�N��			<OUT>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_GetSndBufferCount(T_NT_BUF_COUNT *bufcnt)
{

	bufcnt->prior = 0;
	/* �ʏ�f�[�^�o�b�t�@�̃u���b�N�� */
	bufcnt->normal = NTCom_SndBuf_Normal.blknum;
	/* �D��f�[�^�o�b�t�@�̃u���b�N�� */
	bufcnt->prior += NTCom_SndBuf_Prior.blknum;
/* <-#005 */
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_GetRcvBufferCount()
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M�o�b�t�@�Ɋ܂܂��u���b�N�����擾����
 *| param	: bufcnt - �o�b�t�@�Ɋ܂܂��u���b�N��			<OUT>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_GetRcvBufferCount(T_NT_BUF_COUNT *bufcnt)
{

	/* �ʏ�f�[�^�o�b�t�@�̃u���b�N�� */
	bufcnt->normal = NTCom_RcvBuf_Normal.blknum;

	/* �D��f�[�^�o�b�t�@�̃u���b�N�� */
	bufcnt->prior = NTCom_RcvBuf_Prior.blknum;
}


/*====================================================================================[PRIVATE]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_InitBuffer()
 *[]----------------------------------------------------------------------[]
 *| summary	: �d�����X�g�̏�����
 *| param	: buffer - �f�[�^�o�b�t�@							<OUT>
 *|			  mempool  - �f�[�^�o�b�t�@�Ŏg�p����o�b�t�@������	<IN>
 *|			  mempool_size - mempool�̃T�C�Y(�z��v�f��)
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_InitBuffer(T_NT_BUFFER *buffer, T_NT_BLKDATA *mempool, int mempool_size)
{
	/* �������N���A */
	memset(mempool, 0, mempool_size);
	/* �������A���P�[�g */
	buffer->blkdata	= mempool;
	buffer->max		= mempool_size;
	/* �o�b�t�@����f�[�^������ */
	buffer->fixed	= FALSE;
	buffer->blknum	= 0;
}

/* #004-> */
/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_ReconstructScheduler()
 *[]----------------------------------------------------------------------[]
 *| summary	: �ʏ��M�f�[�^��ARCNET�ւ̑��M�X�P�W���[���̍č\��
 *| param	: scheduler - ���M�X�P�W���[��					<IN/OUT>
 *|			  buffer - ��M�ʏ�f�[�^�o�b�t�@				<IN>
 *|			  fukuden - ���d��������f�[�^					<IN/OUT>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_ReconstructScheduler(T_NT_TELEGRAM_LIST *scheduler, T_NT_BUFFER *buffer, T_NT_FUKUDEN *fukuden)
{
	int ringsize, i, read;

	switch (fukuden->scheduler) {
	case NT_FUKUDEN_SCHEDULER_START:
	/* �X�P�W���[�����烏�[�N�ցA�L���f�[�^�݂̂������o�� */
		ringsize = 0;
		read = scheduler->read;
		while (read != scheduler->write) {
			if (buffer->fixed) {
//				fukuden->scheduler_work[ringsize++] = scheduler->ring[read];
				fukuden->scheduler_work = scheduler->ring[read];
			}
			read = _offset(read, 1, _countof(scheduler->ring));
		}
		fukuden->scheduler_worksize = ringsize;
		fukuden->scheduler = NT_FUKUDEN_SCHEDULER_MADE;
		/* no break */
	case NT_FUKUDEN_SCHEDULER_MADE:
	/* ���[�N����X�P�W���[���ւ̏����߂� */
		for (i = 0; i < fukuden->scheduler_worksize; i++) {
//			scheduler->ring[i] = fukuden->scheduler_work[i];
			scheduler->ring[i] = fukuden->scheduler_work;
		}
		scheduler->read = 0;
		scheduler->write = fukuden->scheduler_worksize;
		fukuden->scheduler = NT_FUKUDEN_SCHEDULER_NONE;
		break;
	default:
		break;
	}
}
/* <-#004 */

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_SetSndBlk()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M�u���b�N�ۑ�
 *| param	: buffer - �ۑ���o�b�t�@			<IN/OUT>
 *|			  blk - �ۑ��u���b�N�f�[�^			<IN>
 *| return	: eNT_DATASET�^
 *| note	: #001
 *[]----------------------------------------------------------------------[]*/
eNT_DATASET	NTComData_SetSndBlk(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk)
{
	eNT_DATASET ret;
	int terminal;

	ret = NT_DATA_NORMAL;

	if (buffer->fixed) {
		return NT_DATA_NO_MORE_TELEGRAM;
	}

	if (!NTComData_ChkBlkNo(buffer, blk)) {
	/* �u���b�NNo.�ُ� */
		NTComData_BufferClear(buffer);
		if (blk->data[NT_OFS_BLOCK_NO] == 1) {
			/* �������u���b�N�f�[�^�̃u���b�NNo.��1�ł���΁A
			   �d���擪�̃f�[�^�Ƃ��Ă��炽�߂ăo�b�t�@�ۑ������ */
			ret = NT_DATA_BLOCK_INVALID2;	/* �f�[�^�L���Ȃ̂Ń��^�[�����Ȃ� */
		}
		else {
			return NT_DATA_BLOCK_INVALID1;			/* �u���b�N�f�[�^���� */
		}
	}

	/* �o�b�t�@�ɕۑ� */
	NTComData_AddBlk(buffer, blk);	/* �o�b�t�@�c�ʂ̃`�F�b�N�͂����ł͂Ȃ����L�^�C�~���O�ōs��
										(27�u���b�N�ڎ�M���ł͂Ȃ��ŏI�u���b�N�łȂ�26�u���b�N�ڎ�M�ŃG���[�Ƃ���d�l) */

	/* �ŏI�u���b�N�̃f�[�^�ł���Γd�����X�g�ɓd�������쐬 */
	if (blk->data[NT_OFS_LAST_BLOCK_FLAG] == NT_LAST_BLOCK) {
		buffer->fixed = TRUE;			/* �d���m��! */

		terminal = blk->data[NT_OFS_TERMINAL_NO];
		if (terminal != NT_ALL_TERMINAL && terminal != NT_BROADCAST) {
			/* �̏�[�����ăf�[�^�폜 */
			NTComData_SndBufClean(buffer, NTComData_Ctrl.terminal_status, terminal);
		}
	}
	else {
		/* �o�b�t�@�c�ʃ`�F�b�N */
		if (buffer->blknum >= buffer->max) {
			NTComData_BufferClear(buffer);
// #007->
//			return NT_DATA_BLOCK_INVALID1;
			return NT_DATA_BUFFERFULL;
// <-#007
		}
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_SetRcvBlk()
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M�u���b�N�ۑ�
 *| param	: buffer - �ۑ���o�b�t�@			<IN>
 *|			  blk - �ۑ�����u���b�N�f�[�^		<IN/OUT>
 *|			  fixed - 1�d����M�������o���ATRUE
 *| return	: eNT_DATASET�^
 *[]----------------------------------------------------------------------[]*/
eNT_DATASET	NTComData_SetRcvBlk(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk, BOOL *fixed)
{
//	int i;
	eNT_DATASET	ret = NT_DATA_NORMAL;
	union {
		uchar	uc[2];
		ushort	us;
	}u;

	if (buffer->fixed) {
		return NT_DATA_NO_MORE_TELEGRAM;
	}

	*fixed = FALSE;

	/* �u���b�NNo.�ُ� */
	if (!NTComData_ChkBlkNo(buffer, blk)) {
		/* �u���b�NNo.�ُ� */
		NTComData_BufferClear(buffer);
		if (blk->data[NT_OFS_BLOCK_NO] == 1) {
			/* �������u���b�N�f�[�^�̃u���b�NNo.��1�ł���΁A
			   �d���擪�̃f�[�^�Ƃ��ăo�b�t�@�ۑ����� */
			ret = NT_DATA_BLOCK_INVALID2;			/* �f�[�^�L���Ȃ̂Ń��^�[�����Ȃ� */
		}
		else {
			return NT_DATA_BLOCK_INVALID1;
		}
	}
	/* �o�b�t�@�ɕۑ� */
	NTComData_AddBlk(buffer, blk);	/* �o�b�t�@�c�ʂ̃`�F�b�N�͂����ł͂Ȃ����L�^�C�~���O�ōs��
										(27�u���b�N�ڎ�M���ł͂Ȃ��ŏI�u���b�N�łȂ�26�u���b�N�ڎ�M�ŃG���[�Ƃ���d�l) */
	if( (blk->data[NT_OFS_PACKET_MODE] == NT_NORMAL_DATA) &&
		(blk->data[NT_OFS_TERMINAL_NO] != 0xFF) ) {
		// �p�P�b�g�f�[�^�T�C�Y�擾
		u.uc[0] = blk->data[5];
		u.uc[1] = blk->data[6];
		memset(&NTComOpeSendNormalDataBuf.ntDataBuf[NTComOpeSendNormalDataBuf.writeIndex], 0, sizeof(T_NT_BLKDATA));
		memcpy(&NTComOpeSendNormalDataBuf.ntDataBuf[NTComOpeSendNormalDataBuf.writeIndex], blk->data, u.us);
		// 0�J�b�g���ꂽ�f�[�^�𕜌�����
		NTComData_Unpack0Cut(&NTComOpeSendNormalDataBuf.ntDataBuf[NTComOpeSendNormalDataBuf.writeIndex][0]);
		NTComOpeSendNormalDataBuf.writeIndex++;
		NTComOpeSendNormalDataBuf.count++;
	}

	/* �ŏI�u���b�N�̃f�[�^�ł���Γd�����X�g�ɓd�������쐬 */
	if (blk->data[NT_OFS_LAST_BLOCK_FLAG] == NT_LAST_BLOCK) {
		buffer->fixed = TRUE;			/* �d���m��! */
		*fixed = TRUE;
	}
	else {
		/* �o�b�t�@�c�ʃ`�F�b�N */
		if (buffer->blknum >= buffer->max) {
			NTComData_BufferClear(buffer);
// #007->
//			return NT_DATA_BLOCK_INVALID1;
			return NT_DATA_BUFFERFULL;
// <-#007
		}
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_SetRcvTeleSchedule()
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET�����M���������ʏ�f�[�^�d���̏����AIBW���M�X�P�W���[���ɒǉ�����
 *| param	: terminal - ��M�����ʏ�f�[�^�d���̑��M���[��No.
 *|						(�]�Ǔ��쎞�͎��[��No.)
 *| return	: none
 *|	update	:	#002
 *[]----------------------------------------------------------------------[]*/
void	NTComData_SetRcvTeleSchedule(int terminal)
{
	NTCom_RcvTeleSchedule.ring[NTCom_RcvTeleSchedule.write] = terminal;
	NTCom_RcvTeleSchedule.write = _offset(NTCom_RcvTeleSchedule.write, 1, _countof(NTCom_RcvTeleSchedule.ring));
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_PeekRcvTeleSchedule()
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET�����M���������ʏ�f�[�^�d���̏��Ԃ��L�^����Ă���IBW���M�X�P�W���[������A
 *|			  ����IBW�֑��M���ׂ��d�����ۑ�����Ă���o�b�t�@���擾����
 *| param	: terminal - ��M�����ʏ�f�[�^�d���̑��M���[��No.
 *|						(�]�Ǔ��쎞�͎��[��No.)
 *| return	: ���펞�F�[��No.
 *|			  �ُ펞�F-1
 *|	update	:	#002
 *[]----------------------------------------------------------------------[]*/
int		NTComData_PeekRcvTeleSchedule(void)
{
	int terminal;

	if (NTCom_RcvTeleSchedule.read == NTCom_RcvTeleSchedule.write) {
		return -1;
	}

	terminal = NTCom_RcvTeleSchedule.ring[NTCom_RcvTeleSchedule.read];

	return terminal;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_DeleteRcvTeleSchedule()
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET�����M���������ʏ�f�[�^�d���̏��Ԃ��L�^����Ă���IBW���M�X�P�W���[������A�Ō���̃f�[�^���폜
 *| param	: none
 *| return	: none
 *|	update	:	#002
 *[]----------------------------------------------------------------------[]*/
void	NTComData_DeleteRcvTeleSchedule(void)
{
	if (NTCom_RcvTeleSchedule.read == NTCom_RcvTeleSchedule.write) {
		return;
	}

	NTCom_RcvTeleSchedule.read = _offset(NTCom_RcvTeleSchedule.read, 1, _countof(NTCom_RcvTeleSchedule.ring));
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NT_IsNewTypeTimeSetPacket()
 *[]----------------------------------------------------------------------[]
 *| summary	: �V�`���̎��v�f�[�^�d�����ۂ��̔���֐�
 *| param	: blk - �ۑ�����f�[�^				<IN>
 *| return	: TRUE is BINGO
 *[]----------------------------------------------------------------------[]*/
BOOL	NT_IsNewTypeTimeSetPacket( T_NT_BLKDATA *blk )
{
	union {
		ushort	us;
		uchar	uc[2];
	} u;

	if( (blk->data[NT_OFS_LAST_BLOCK_FLAG] == NT_LAST_BLOCK) &&
		(blk->data[NT_OFS_BLOCK_NO] == 1) &&						// 1�u���b�N�d���� ����
		((blk->data[NT_OFS_DATA_KIND] == NT_TMPKT_ID1) || (blk->data[NT_OFS_DATA_KIND] == NT_TMPKT_ID2)) )
																	// ���v�f�[�^�d���̏ꍇ
	{
		u.uc[0] = blk->data[NT_OFS_DATASIZE_WITH_ZERO];
		u.uc[1] = blk->data[NT_OFS_DATASIZE_WITH_ZERO+1];			// 0�J�b�g�O�̃f�[�^�T�C�Yget
		if( u.us >= NT_TMPKT_NEW_TYPE_PKT_DATA_LEN ){				// �V���v�f�[�^�d���i�␳�l�t���j�ł����
			return TRUE;
		}
	}
	return FALSE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_AddBlk()
 *[]----------------------------------------------------------------------[]
 *| summary	: ���X�g�Ō���Ƀf�[�^��ۑ�����
 *| param	: buffer - �ۑ���o�b�t�@���X�g	<IN/OUT>
 *|			  blk - �ۑ�����f�[�^				<IN>
 *| return	: TRUE as success
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_AddBlk(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk)
{
	union {
		ushort	us;
		uchar	uc[2];
	} u;
	ulong		wkul;
	short		AddLength;		// ���v�X�V�v���d����0�J�b�g����ĒZ���ꍇ�A�⑫���钷��

	/* �o�b�t�@�c�ʃ`�F�b�N */
	if (buffer->blknum >= buffer->max) {
		return FALSE;
	}

	// ����M ���d���͖{�֐��Ŕw���p�o�b�t�@�ɃR�s�[�����
	// �����Ŏ��v�f�[�^�d���iID=119�C229�j�ł���Ύ�M�����Ƃ���
	// ���C�t�^�C�}�[�i�t���[�����j���O�^�C�}�j�l���Z�b�g����

	if( TRUE == NT_IsNewTypeTimeSetPacket(blk) ) {						// �V�`���̎��v�f�[�^�d��
		wkul = LifeTim1msGet();
		memcpy( &blk->data[NT_OFS_DATA_TMPKT_FREETIMER], &wkul, 4 );	// �d���Ɍ��݂̃��C�t�^�C�}�[�l���Z�b�g(*1)

		/* �d�������Ƀf�[�^���Z�b�g�����̂ŁA�d����0�J�b�g����Ă�����f�[�^���𖖔��܂ł̒l�ɃZ�b�g���� */
		if( blk->len < NT_TMPKT_NEW_TYPE_PKT_TOTAL_LEN ){
			AddLength = NT_TMPKT_NEW_TYPE_PKT_TOTAL_LEN - blk->len;		// �Z�������Z�o
			if( AddLength > 4 ){										// ����4byte�� *1�ŃZ�b�g���Ă���̂ŁA�㏑�����Ȃ��悤�ɂ���
				memset( &blk->data[blk->len], 0, (AddLength - 4) );		// �f�[�^����41byte�ɂ��āA�s������ 0 ���Z�b�g����B
			}

			blk->len = NT_TMPKT_NEW_TYPE_PKT_TOTAL_LEN;					// �����lLength�ύX

			u.us = NT_TMPKT_NEW_TYPE_PKT_TOTAL_LEN;						// �d���w�b�_�� �p�P�b�g�f�[�^�T�C�Yset
			blk->data[NT_OFS_DATA_SIZE_HI ] = u.uc[0];
			blk->data[NT_OFS_DATA_SIZE_LOW] = u.uc[1];
		}
	}

	/* �u���b�N�f�[�^�R�s�[ */
	memcpy(&buffer->blkdata[buffer->blknum], blk, sizeof(T_NT_BLKDATA));
	buffer->blknum++;

	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_CreateTelegramInfo()
 *[]----------------------------------------------------------------------[]
 *| summary	: �d�����擾
 *| param	: buffer - �o�b�t�@					<IN>
 *|			  telegram - �d�����				<OUT>
 *| return	: TRUE - �d���L��
 *|			  FALSE - �d���Ȃ�
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_CreateTelegramInfo(T_NT_BUFFER *buffer, T_NT_TELEGRAM *telegram)
{
	if (!buffer->fixed) {
		return FALSE;
	}
	telegram->curblk	= -1;		/* invalid */
	telegram->kind		= buffer->kind;
	telegram->blknum	= buffer->blknum;
	telegram->terminal	= buffer->blkdata[0].data[NT_OFS_TERMINAL_NO];
	telegram->saveflag	= buffer->blkdata[0].data[NT_OFS_BUFFERING_FLAG];		/* #003 */
	telegram->curblk	= 0;		/* valid */

	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_GetBlkData()
 *[]----------------------------------------------------------------------[]
 *| summary	: �u���b�N�|�C���^�̈ʒu�ɂ���f�[�^���擾����
 *|			  �f�[�^�擾������A�u���b�N�f�[�^�|�C���^��1�i�߂�
 *| param	: buffer - �o�b�t�@					<IN/OUT>
 *|			  blk - �u���b�N�f�[�^				<OUT>
 *|			  blk_no - �f�[�^�Ƃ��ė~�����u���b�N�̃u���b�NNo.
 *| return	: TRUE as success
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_GetBlkData(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk, int blk_no)
{
	if (blk_no < 0 || buffer->blknum <= blk_no) {
		return FALSE;		/* no more block */
	}

	memcpy(blk, &buffer->blkdata[blk_no], sizeof(T_NT_BLKDATA));

	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_BufferClear()
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�t�@�N���A
 *| param	: buffer - �u���b�N�f�[�^���N���A����o�b�t�@		<IN/OUT>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_BufferClear(T_NT_BUFFER *buffer)
{
//	buffer->fixed	= FALSE;
	buffer->blknum	= 0;
	buffer->fixed	= FALSE;	/* #004 */
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_SndBufClean()
 *[]----------------------------------------------------------------------[]
 *| summary	: �f�[�^�ێ��t���O��OFF�̌̏�[�����ēd�����폜����
 *| param	: buffer - �o�b�t�@
 *|			  terminal_failed - �ʐM�ُ�[�����
 *|			  terminal - �̏�[��No.
 *| return	: TRUE - �o�b�t�@�N���A���s��ꂽ
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_SndBufClean(T_NT_BUFFER *buffer, eNT_STS terminal_failed, int terminal)
{
	if (buffer->fixed) {
		if (terminal_failed == NT_ABNORMAL) {
			// �Y���[���͒ʐM�ُ풆
			if (buffer->blkdata[0].data[NT_OFS_TERMINAL_NO] == terminal) {
				// �o�b�t�@���f�[�^���Y���[��
				return NTComData_ClrNotBufferingData(buffer);		// #003
			}
		}
	}
	return FALSE;	/* ���m��̓d���͍ŏI�u���b�N��M��҂��č폜 */
}

// #003->
/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_SndBufClean()
 *[]----------------------------------------------------------------------[]
 *| summary	: �Y���o�b�t�@���Ɋ܂܂��f�[�^�̃f�[�^�ێ��t���O��OFF�ł���΁A�d���폜
 *| param	: buffer - �o�b�t�@
 *| return	: TRUE - �o�b�t�@�N���A���s��ꂽ
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_ClrNotBufferingData(T_NT_BUFFER *buffer)
{
	if (buffer->blknum > 0) {
		if (buffer->blkdata[0].data[NT_OFS_BUFFERING_FLAG] == NT_NOT_BUFFERING) {
			NTComData_BufferClear(buffer);
			return TRUE;
		}
	}

	return FALSE;
}
// <-#003

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_ChkBlkNo()
 *[]----------------------------------------------------------------------[]
 *| summary	: �u���b�N�f�[�^�̃u���b�NNo.�`�F�b�N
 *|			  buffer���̃f�[�^��blk�Ńu���b�NNo.���A�����Ă��邩���`�F�b�N����
 *| param	: buffer - �`�F�b�N����o�b�t�@
 *|			  blk - �o�b�t�@�̍Ō�ɕt�������\��̃u���b�N�f�[�^
 *| return	: TRUE - �u���b�NNo.����
 *[]----------------------------------------------------------------------[]*/
BOOL	NTComData_ChkBlkNo(T_NT_BUFFER *buffer, T_NT_BLKDATA *blk)
{
	int		i;
	uchar	prv_no;

	if (buffer->blknum <= 0) {
		if (blk->data[NT_OFS_BLOCK_NO] != 1) {
			return FALSE;
		}
	}
	else {
		prv_no = 0;
		for (i = 0; i < buffer->blknum; i++) {
			if (prv_no + 1 != buffer->blkdata[i].data[NT_OFS_BLOCK_NO]) {
				return FALSE;
			}
			prv_no++;
		}
		if (prv_no + 1 != blk->data[NT_OFS_BLOCK_NO]) {
			return FALSE;
		}
	}

	return TRUE;
}

/* #002-> */
/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_CheckPowerFailure()
 *[]----------------------------------------------------------------------[]
 *| summary	: ��d�ۏ؏���
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_CheckPowerFailure(void)
{
	int ofs, cnt, cnt_max;

/* #004-> */
// �e�o�b�t�@���f�[�^�폜�������`�F�b�N
	/* �ʏ�f�[�^�o�b�t�@ */
	_NTComData_CheckBuffer(&NTCom_SndBuf_Normal);	/* NT�l�b�g�ւ̑��M�p */
	_NTComData_CheckBuffer(&NTCom_RcvBuf_Normal);	/* NT�l�b�g����̎�M�p */

	/* �D��f�[�^�o�b�t�@ */
	_NTComData_CheckBuffer(&NTCom_SndBuf_Prior);		/* NT�l�b�g�ւ̑��M�p */

	_NTComData_CheckBuffer(&NTCom_RcvBuf_Prior);			/* NT�l�b�g����̎�M�p */
	/* ����f�[�^�o�b�t�@ */
	_NTComData_CheckBuffer(&NTCom_SndBuf_Broadcast);		/* NT�l�b�g�ւ̑��M�p */
	_NTComData_CheckBuffer(&NTCom_RcvBuf_Broadcast);		/* NT�l�b�g����̎�M�p */

// ��M�ʏ�f�[�^�p�̃X�P�W���[�����`�F�b�N
	if (NTCom_Fukuden.scheduler != NT_FUKUDEN_SCHEDULER_NONE) {
		NTComData_ReconstructScheduler(&NTCom_RcvTeleSchedule, &NTCom_RcvBuf_Normal, &NTCom_Fukuden);
	}

/* <-#004 */

// ��M�o�b�t�@�̎�M�f�[�^���M�X�P�W���[�� �Ԃ̕s�������`�F�b�N
	// ��M�o�b�t�@������s�������`�F�b�N
	if (NTCom_RcvBuf_Normal.fixed) {
		// �ʏ�d����M�o�b�t�@�Ɋm�肵���d�������݂�����
		ofs = NTCom_RcvTeleSchedule.read;

		if (NTCom_RcvTeleSchedule.read <= NTCom_RcvTeleSchedule.write) {
			cnt_max = NTCom_RcvTeleSchedule.write - NTCom_RcvTeleSchedule.read;
		} else {
			cnt_max = NTCom_RcvTeleSchedule.write - NTCom_RcvTeleSchedule.read + 1;
		}

		for (cnt = 0; cnt < cnt_max; cnt++) {
			if (NTCom_RcvTeleSchedule.ring[ofs] == 1) {
				// �ʏ�o�b�t�@�̓d�������łɓo�^����Ă�����A���̃o�b�t�@���`�F�b�N����
				break;
			}
			ofs = _offset(ofs, 1, _countof(NTCom_RcvTeleSchedule.ring));
		}
		if (cnt >= cnt_max) {
			// �d���o�^�r���Œ�d�����̃P�[�X�ւ̑Ή�
			// �ʐM���肩��đ������̂ŁA��M�O�̏�ԂɃ��[���o�b�N
			NTComData_BufferClear(&NTCom_RcvBuf_Normal);
		}
	}

	// ���M�X�P�W���[��������s�������`�F�b�N
	if (NTCom_RcvTeleSchedule.read != NTCom_RcvTeleSchedule.write) {
		// �X�P�W���[���[�ɗL���ȃf�[�^�������
//		ofs = _termno2ofs(NTCom_RcvTeleSchedule.ring[NTCom_RcvTeleSchedule.read]);
		if (!NTCom_RcvBuf_Normal.fixed) {
			// �d���폜�r���Œ�d�����̃P�[�X�ւ̑Ή�
			// �X�P�W���[������Ă���[���̎�M�o�b�t�@���󂾂�����A�X�P�W���[��������폜����
			NTComData_DeleteRcvTeleSchedule();
		}
	}
}
/* <-#002 */

/*[]----------------------------------------------------------------------[]
 *|	name	: NTComData_ResetTelegram()
 *[]----------------------------------------------------------------------[]
 *| summary	: �d����񃊃Z�b�g����
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTComData_ResetTelegram(T_NT_TELEGRAM *telegram)
{
	telegram->curblk = -1;

	if(telegram->kind == NT_DATA_KIND_PRIOR_SND) {
		if(NTComOpeRecvPriorDataBuf.fixed) {
			NTComOpeRecvPriorDataBuf.fixed = FALSE;
		}
	}
	if(telegram->kind == NT_DATA_KIND_NORMAL_SND) {
		if(NTComOpeRecvNormalDataBuf.fixed) {
			NTComOpeRecvNormalDataBuf.fixed = FALSE;
		}
	}
}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
