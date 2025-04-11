/*[]----------------------------------------------------------------------[]
 *| System      : UT8500
 *| Module      : NT-NET�o�b�t�@�Ǘ����W���[��
 *[]----------------------------------------------------------------------[]
 *| Author      : machida kei
 *| Date        : 2005.08.19
 *| Update      :
 *|		2005.09.07	�S�f�[�^�N���AIF�ǉ�
 *|		2005.09.21	�o�b�t�@FULL�A�j�AFULL��Ԃ�V�݂��A���̔���/������OPE�֒ʒm���� GW841901
 *|		2006-01-17	machida.k	�A�C�h�����׌y�� (MH544401)
 *|		2006-01-19	machida.k	bug fix
 *[]----------------------------------------------------------------------[]
 *| Remark		:
 *|		�f�[�^��2�d�o�^�h�~�̂��߂̒�d�ۏ؂͎������Ȃ�
 *|		NT-NET�f�[�^���̃V�[�P���V����No.����ʃA�v���P�[�V�����ɂ�����
 *|		�`�F�b�N���邱�Ƃŕۏ؂���Ƃ������j�̂���
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
#include	"message.h"
#include	"common.h"
#include	"ntnet.h"
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
#include	"ntnet_def.h"
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
#include	"ope_def.h"
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
#include	"rau.h"
#include	"ntcom.h"
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
/*--------------------------------------------------------------------------*/
/*			C O N S T A N T S												*/
/*--------------------------------------------------------------------------*/

/* ���M�o�b�t�@���ʃR�[�h(�擪����D��x��) */
enum {
	/* prior data-> */
	NTNET_SNDBUF_PRIOR,						/* ���̑��̗D��f�[�^ */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
//	NTNET_SNDBUF_SALE,						/* ���Z�f�[�^(ID22orID33) */
//	/* <-prior data */
//	
//	/* normal data-> */
//	NTNET_SNDBUF_CAR_OUT,					/* �o�Ƀf�[�^�e�[�u��(ID21) */
//	NTNET_SNDBUF_CAR_IN,					/* ���Ƀf�[�^�e�[�u��(ID20) */
//	NTNET_SNDBUF_TTOTAL,					/* T���v�W�v�f�[�^(ID30�`38�AID41) */
//	NTNET_SNDBUF_COIN,						/* �R�C�����ɏW�v���v�f�[�^(ID131) */
//	NTNET_SNDBUF_NOTE,						/* �������ɏW�v���v�f�[�^(ID133) */
//	NTNET_SNDBUF_ERROR,						/* �G���[�f�[�^(ID120) */
//	NTNET_SNDBUF_ALARM,						/* �A���[���f�[�^(ID121) */
//	NTNET_SNDBUF_MONEY,						/* ���K�Ǘ��f�[�^(ID126) */
//	NTNET_SNDBUF_TURI,						/* �ޑK�Ǘ��W�v�f�[�^(ID135) */
//	NTNET_SNDBUF_CAR_COUNT,					/* ���ԑ䐔�f�[�^(ID236) */
//	NTNET_SNDBUF_AREA_COUNT,				/* ���䐔�E���ԃf�[�^(ID237) */
//	NTNET_SNDBUF_MONITOR,					/* ���j�^�f�[�^(ID122) */
//	NTNET_SNDBUF_OPE_MONITOR,				/* ���샂�j�^�f�[�^(ID123) */
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
	NTNET_SNDBUF_NORMAL,					/* ���̑��̒ʏ�f�[�^ */
	/* <-normal data */
	
	NTNET_SNDBUF_MAX
};

#define	NTNET_SNDBUF_PRIOR_TOP	NTNET_SNDBUF_PRIOR
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
//#define	NTNET_SNDBUF_PRIOR_END	NTNET_SNDBUF_SALE
//
//#define	NTNET_SNDBUF_NORMAL_TOP	NTNET_SNDBUF_CAR_OUT
#define	NTNET_SNDBUF_PRIOR_END	NTNET_SNDBUF_PRIOR
#define	NTNET_SNDBUF_NORMAL_TOP	NTNET_SNDBUF_NORMAL
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
#define	NTNET_SNDBUF_NORMAL_END	NTNET_SNDBUF_NORMAL

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
#define	NTNET_SNDLOG_MAX	(sizeof(z_NtSndLogInfo)/sizeof(z_NtSndLogInfo[0]))
// GM849100(S) ���S�����R�[���Z���^�[�Ή��i960�o�C�g�𒴂��邽�ߐ��Z�f�[�^��ʏ�f�[�^�ő��M����j�iGT-7700:GM747902�Q�l�j
//#define	NTNET_SNDLOG_PRIOR_TOP	0
//#define	NTNET_SNDLOG_PRIOR_END	0
//
//#define	NTNET_SNDLOG_NORMAL_TOP	(NTNET_SNDLOG_PRIOR_END + 1)
#define	NTNET_SNDLOG_NORMAL_TOP		0
// GM849100(E) ���S�����R�[���Z���^�[�Ή��i960�o�C�g�𒴂��邽�ߐ��Z�f�[�^��ʏ�f�[�^�ő��M����j�iGT-7700:GM747902�Q�l�j
#define	NTNET_SNDLOG_NORMAL_END	NTNET_SNDLOG_MAX
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

/* ��M�o�b�t�@���ʃR�[�h(�擪����D��x��) */
enum {
	NTNET_RCVBUF_BROADCAST,					/* ����f�[�^��M�o�b�t�@ */
	NTNET_RCVBUF_PRIOR,						/* �D��f�[�^��M�o�b�t�@ */
	NTNET_RCVBUF_NORMAL,					/* �ʏ�f�[�^��M�o�b�t�@ */
	NTNET_RCVBUF_MAX
};

/* ���d������� */
typedef enum {
	NTBUF_FUKUDEN_NONE,						/* ���d�������� */
	NTBUF_FUKUDEN_CLR,						/* �o�b�t�@�������� */
	NTBUF_FUKUDEN_DATAMOVE,					/* �o�b�t�@���f�[�^�ʒu�ړ��� */
}eNTBUF_FUKUDEN;

#define	NTBUF_PRIOR_DATASIZE_MAX		NTNET_BLKDATA_MAX	/* ���̑��̗D��f�[�^ */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//#define	NTBUF_SALE_DATASIZE_MAX			947					/* ���Z�f�[�^(ID22orID33) */
//#define	NTBUF_CAR_OUT_DATASIZE_MAX		435					/* �o�Ƀf�[�^�e�[�u��(ID21) */
//#define	NTBUF_CAR_IN_DATASIZE_MAX		159					/* ���Ƀf�[�^�e�[�u��(ID20) */
//#define	NTBUF_TTOTAL_DATASIZE_MAX		15591				/* T���v�W�v�f�[�^(ID35) */
//#define	NTBUF_COIN_DATASIZE_MAX			91					/* �R�C�����ɏW�v���v�f�[�^(ID131) */
//#define	NTBUF_NOTE_DATASIZE_MAX			91					/* �������ɏW�v���v�f�[�^(ID133) */
//#define	NTBUF_ERROR_DATASIZE_MAX		197					/* �G���[�f�[�^(ID120) */
//#define	NTBUF_ALARM_DATASIZE_MAX		197					/* �A���[���f�[�^(ID121) */
//#define	NTBUF_MONEY_DATASIZE_MAX		187					/* ���K�Ǘ��f�[�^(ID126) */
//#define	NTBUF_CAR_COUNT_DATASIZE_MAX	225					/* ���ԑ䐔�f�[�^(ID236) */
//#define	NTBUF_AREA_COUNT_DATASIZE_MAX	545					/* ���䐔�E���ԃf�[�^(ID237) */
//#define	NTBUF_MONITOR_DATASIZE_MAX		197					/* ���j�^�f�[�^(ID122) */
//#define	NTBUF_OPE_MONITOR_DATASIZE_MAX	194					/* ���샂�j�^�f�[�^(ID123) */
//#define	NTBUF_NORMAL_DATASIZE_MAX		24960	/* (960�~26 */	/* ���̑��̒ʏ�f�[�^ */
//#define	NTBUF_BROADCAST_DATASIZE_MAX	NTNET_BLKDATA_MAX	/* ����f�[�^��M�o�b�t�@ */
//
//#define	NTBUF_NEARFULL_COUNT			6
//#define NTBUF_SALE_MEMPOOL_MAX			180000L
#define	NTBUF_NORMAL_DATASIZE_MAX		24960	/* (960�~26 */	/* ���̑��̒ʏ�f�[�^ */
#define	NTBUF_BROADCAST_DATASIZE_MAX	NTNET_BLKDATA_MAX	/* ����f�[�^��M�o�b�t�@ */

#define	NTBUF_NOTSENDTOTALDATA			0xffff
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

/*--------------------------------------------------------------------------*/
/*			S T R U C T U R E S												*/
/*--------------------------------------------------------------------------*/

/* �o�b�t�@�ݒ�擾�֐��^ */
typedef	long (*NTBUF_SETTING_FUNC)(void);

/* ���d�����p�f�[�^ */
typedef struct {
	eNTBUF_FUKUDEN		kind;				/* ���d������� */
	
	/* for NTBUF_FUKUDEN_DATAMOVE */
	struct {
		long	rofs;						/* �ړ��Ώۃf�[�^�擪�I�t�Z�b�g */
		long	count;						/* �ړ��Ώۃf�[�^�� */
		long	move;						/* �ړ����鋗��(�o�C�g��) */
	}datamove;
}t_NtBufFukuden;

/* �o�b�t�@�Ǘ� */
typedef struct {
	eNTBUF_SAVE			save;				/* �f�[�^�ێ����@��� */
	long				wofs;				/* �������݃I�t�Z�b�g */
	long				rofs;				/* �ǂݍ��݃I�t�Z�b�g */
	uchar				*buffer;			/* �o�b�t�@�f�[�^�{�� */
	long				buffer_size;		/* �o�b�t�@�S�̃T�C�Y */
	ulong				datasize;			/* �o�b�t�@�Ɋi�[�����1�f�[�^�̍ő咷 */
	short				id;					/* �o�b�t�@�Ɋi�[�����f�[�^��� */
	NTBUF_SETTING_FUNC	setting;			/* �o�b�t�@FULL���̓���ݒ�擾�֐��|�C���^ */
	t_NtBufFukuden		fukuden;			/* ���d�����p�f�[�^ */
}t_NtBuf;

/* �o�b�t�@���̈� */
typedef struct {
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//	uchar	snd_pay[1];	/* ���Z�f�[�^�e�[�u��(ID22 or 23) */
//	uchar	snd_car_out[1];				/* �o�Ƀf�[�^�e�[�u��(ID21) */
//	uchar	snd_car_in[1];				/* ���Ƀf�[�^�e�[�u��(ID20) */
//	uchar	snd_ttotal[1];				/* T���v�W�v�f�[�^(ID30�`38�A41) */
//	uchar	snd_coin[1];				/* �R�C�����ɏW�v���v�f�[�^(ID131) */
//	uchar	snd_note[1];				/* �������ɏW�v���v�f�[�^(ID133) */
//	uchar	snd_err[1];					/* �G���[�f�[�^(ID120) */
//	uchar	snd_alarm[1];				/* �A���[���f�[�^(ID121) */
//	uchar	snd_money[1];				/* ���K�Ǘ��f�[�^(ID126) */
//	uchar	snd_car_count[1];			/* ���ԑ䐔�f�[�^(ID236) */
//	uchar	snd_area_count[1];			/* ���䐔�f�[�^(ID236) */
//	uchar	snd_monitor[1];				/* ���j�^�f�[�^(ID122) */
//	uchar	snd_ope_monitor[1];			/* ���샂�j�^�f�[�^(ID123) */
//	uchar	snd_normal[1];				/* ���M�ʏ�f�[�^ */
//	uchar	snd_prior[1];				/* ���M�D��f�[�^ */
//	uchar	rcv_normal[1];				/* ��M�ʏ�f�[�^ */
//	uchar	rcv_prior[1];				/* ��M�D��f�[�^ */
//	uchar	rcv_broadcast[1];			/* ��M����f�[�^ */
	uchar	snd_normal[26000];			/* ���M�ʏ�f�[�^ */
	uchar	snd_prior[10000];			/* ���M�D��f�[�^ */
	uchar	rcv_normal[26000];			/* ��M�ʏ�f�[�^ */
	uchar	rcv_prior[10000];			/* ��M�D��f�[�^ */
	uchar	rcv_broadcast[1000];		/* ��M����f�[�^ */
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
}t_NtBufPool;

/* �o�b�t�@��������� */
typedef struct {
	eNTBUF_SAVE			save;				/* �f�[�^�ێ����@��� */
	uchar				*pool;				/* �o�b�t�@�f�[�^�{�̃������v�[�� */
	ulong				size;				/* �������v�[���T�C�Y */
	ulong				datasize;			/* �o�b�t�@�Ɋi�[�����1�f�[�^�̍ő咷 */
	short				id;					/* �o�b�t�@�Ɋi�[�����f�[�^��� */
	NTBUF_SETTING_FUNC	setting;			/* �o�b�t�@FULL���̓���ݒ�擾�֐��|�C���^ */
}t_NtBufInfo;

/* �o�b�t�@���̊e�f�[�^�ɕt�������w�b�_��� */
typedef struct {
	uchar	len[2];							/* �f�[�^�� */
	uchar	len_before0cut[2];				/* 0�J�b�g�O�̃f�[�^�� */
	uchar	lower_terminal1;				/* ���ʓ]���p�[��No.(1) */
	uchar	lower_terminal2;				/* ���ʓ]���p�[��No.(2) */
	uchar	reserve;						/* �\�� */
}t_NtBufDataHeader;

/* NT-NET�f�[�^�w�b�_��� */
typedef struct {
	t_NtBufDataHeader	bufdata_header;		/* �o�b�t�@�f�[�^�w�b�_ */
	t_NtNetDataHeader	ntdata_header;		/* NT-NET�f�[�^�w�b�_ */
}t_NtBufNtDataHeader;

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
///* FREE�f�[�^�o�b�t�@ */
//typedef struct {
//	BOOL	isdata;							/* TRUE=�f�[�^�L�� */
//	uchar	buffer[FREEPKT_DATASIZE];		/* FREE�f�[�^ */
//}t_NtFreeBuf;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (backuped)									*/
/*--------------------------------------------------------------------------*/

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//#pragma section		_UNINIT2
#pragma section		_UNINIT4
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

/* �o�b�t�@���̈� */
static t_NtBufPool			z_NtBufPool;

/* ���M�o�b�t�@ */
static t_NtBuf				z_NtSndBuf[NTNET_SNDBUF_MAX];

/* ��M�o�b�t�@ */
static t_NtBuf				z_NtRcvBuf[NTNET_RCVBUF_MAX];

/* �S�f�[�^�N���A�� */
static BOOL					z_NtAllClr;

/* �j�A�t���d����M */
static ulong				z_NearFullBuff;
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//static ulong				z_PendingBuff;

// ���v���O�f�[�^
static uchar				z_NtBufTotalLog[sizeof(SYUKEI)];
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

#pragma section

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (not backuped)								*/
/*--------------------------------------------------------------------------*/

extern BOOL	z_NTNET_AtInitial;

/* NT-NET�f�[�^�w�b�_�擾�p���[�N�̈� */
static t_NtBufNtDataHeader	z_NtBufNtDataHeaderWork;

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
///* FREE�f�[�^���M�o�b�t�@ */
//static t_NtFreeBuf			z_NtSndFreeBuf;
//
///* FREE�f�[�^��M�o�b�t�@ */
//static t_NtFreeBuf			z_NtRcvFreeBuf;
// ���v�f�[�^ID���X�g
// MH341110(S) A.Iiizumi 2017/12/18 ParkingWeb�Ή���NT-NET�͎Ԏ����W�v�͕s�v�Ȃ��ߒ[���Ԃł͑��M���Ȃ��悤�ɏC�� (���ʉ��P��1391)
//static ushort	z_NtBufTotalIDList[] = {30, 31, 32, 33, 34, 35, 36, 41, 0};
static ushort	z_NtBufTotalIDList[9];
// MH341110(E) A.Iiizumi 2017/12/18 ParkingWeb�Ή���NT-NET�͎Ԏ����W�v�͕s�v�Ȃ��ߒ[���Ԃł͑��M���Ȃ��悤�ɏC�� (���ʉ��P��1391)

// ���v���O�f�[�^
static ushort	z_NtBufTotalDataIndex = NTBUF_NOTSENDTOTALDATA;

// ���M�f�[�^���� 0:�D��f�[�^ 1:�ʏ�f�[�^
t_Ntnet_SendDataCtrl	NTNET_SendCtrl[NTNET_SNDBUF_MAX];

// �D��NTNET�f�[�^�o�b�t�@
static	uchar	NtSendPriorData[1024];
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

/* �ʐM��Q�������t���O */
static BOOL					z_NtCommErr;

/* �o�b�t�@��� */
static t_NtBufState			z_NtBufState;

BOOL	z_NtBuf_SndDataExist;

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
const ushort c_NTBufTotalIDList_Normal[9] = {30, 31, 32, 33, 34, 36, 41, 0};
const ushort c_NTBufTotalIDList_ParkingWeb[9] = {30, 31, 32, 33, 34, 36, 41, 0, 0};
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/

#define	_ntbuf_IsNtData(b)	((b)->wofs != (b)->rofs)

#define	_ntbuf_GetPlaceSetting(s, p)	(((s) / (p)) % 10)

static void		ntbuf_InitNtBuffer(t_NtBuf *buffers, const t_NtBufInfo *bufinfo, int max, uchar clr);

static void		ntbuf_RearrangeNtBuffer(t_NtBuf *buffer);
static void		ntbuf_DiscardNtData(t_NtBuf *buffer, long delete_data_ofs, ushort delete_data_len);
static void		ntbuf_MoveNtData(t_NtBuf *buffer);
static void		ntbuf_ChkRcvBuf(t_NtBuf *buffers);
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//static BOOL		ntbuf_ChkSndBuf(t_NtBuf *buffers, t_NtFreeBuf *freebuf);
static ulong	ntbuf_GetBufCount(t_NtBuf *buffer);
static BOOL		ntbuf_ChkSndBuf(t_NtBuf *buffers);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

static uchar	ntbuf_SetNtData(t_NtBuf *buffer, const void *data, ushort len);
static void		ntbuf_DeleteOldestNtData(t_NtBuf *buffer, long write_len);
static void		ntbuf_DeleteLatestNtData(t_NtBuf *buffer, long write_len);
static void		ntbuf_WriteNtData(t_NtBuf *buffer, const void *data, ushort len, ushort len_0cut);

static ushort	ntbuf_GetNtData0CutLen(const uchar *data, ushort len);
static int		ntbuf_GetSndNtBufCode(const void *data, eNTNET_BUF_KIND buffer);
static int		ntbuf_GetRcvNtBufCode(eNTNET_BUF_KIND buffer);
static long		ntbuf_GetNtBufRemain(t_NtBuf *buffer);
static BOOL		ntbuf_ChkSndNtDataLen(int bufcode, ushort len);
static BOOL		ntbuf_ChkRcvNtDataLen(int bufcode, ushort len);

static long		ntbuf_ReadNtBuf(t_NtBuf *buffer, void *dst, long top, ulong cnt);
static long		ntbuf_WriteNtBuf(t_NtBuf *buffer, long dst_ofs, const void *data, ulong cnt);

static ushort	ntbuf_GetNtData(t_NtBuf *buffer, void *data, uchar f_Expand);

static void		ntbuf_NtDataClr(t_NtBuf *buffer);

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
//static uchar	ntbuf_SetFreeData(t_NtFreeBuf *buffer, const void *data);
//static uchar	ntbuf_GetFreeData(t_NtFreeBuf *buffer, void *dst);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j

static BOOL		ntbuf_ChkBufState(t_NtBuf *buffer);
static BOOL		ntbuf_SetBufState(t_NtBuf *buffer, uchar state);
static uchar	ntbuf_GetBufState(t_NtBuf *buffer);

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
//static long		ntbuf_GetSaleBufSetting(void);
//static long		ntbuf_GetCarInBufSetting(void);
//static long		ntbuf_GetCarOutBufSetting(void);
//static long		ntbuf_GetTTotalBufSetting(void);
//static long		ntbuf_GetCoinBufSetting(void);
//static long		ntbuf_GetNoteBufSetting(void);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
static long		ntbuf_GetDefaultSetting(void);

static void		ntbuf_W2Bcpy(uchar *bdst, ushort wsrc);

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//char ntbuf_IsSendData(uchar id);
//static void ntbuf_SetIBKBuffStopByID(uchar ID, ulong *bit);
ushort	NTBUF_ConvertLogToNTNETData(ushort logType, uchar* pLogData, uchar* pNtnetData);
ushort	NTBUF_ConvertTotalLogToNTNETData(ushort logType, ushort logID, uchar* pLogData, uchar* pNtnetData);
static	void	ntbuf_InitSendDataCtrl(void);
static	void	ntbuf_ClearSendDataCtrl(t_Ntnet_SendDataCtrl* pCtrl);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S (const table)									*/
/*--------------------------------------------------------------------------*/
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
static const int	z_NtSndLogInfo[] = {	// ���O������Ă��鑗�M�o�b�t�@(�D�揇)
	// �D��f�[�^
	eLOG_PAYMENT,			// ���Z�f�[�^(ID22orID33)
	// �ʏ�f�[�^
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���g�p���O�i���ɁA�o�Ɂj�j
//	eLOG_ENTER,				// ���Ƀf�[�^�e�[�u��(ID20)
//// MH364300 GG119A34(S) ���P�A���\No.83�Ή�
//	eLOG_LEAVE,				// �o�Ƀf�[�^�e�[�u��(ID21)
//// MH364300 GG119A34(E) ���P�A���\No.83�Ή�
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���g�p���O�i���ɁA�o�Ɂj�j
	eLOG_TTOTAL,			// T���v�W�v�f�[�^(ID30�`38�AID41)
// �s��C��(S) K.Onodera 2016/12/26 #1701 �[���ԒʐM�ɂ�����GT���v�����{���Ă��W�v�f�[�^�����M����Ȃ�
	eLOG_GTTOTAL,			// GT���v�W�v�f�[�^
// �s��C��(E) K.Onodera 2016/12/26 #1701 �[���ԒʐM�ɂ�����GT���v�����{���Ă��W�v�f�[�^�����M����Ȃ�
// MH341110(S) A.Iiizumi 2018/02/15 NT-NET�[���ԒʐM�Œ��ԏ�Z���^�[�`���̏ꍇ�A�ޑK�Ǘ����v(ID135)�����Z�̖��ɑ��M�����s��Ή�(���ʉ��P��1402)
	eLOG_MNYMNG_SRAM,		// �ޑK�Ǘ��W�v�f�[�^(ID135)
// MH341110(E) A.Iiizumi 2018/02/15 NT-NET�[���ԒʐM�Œ��ԏ�Z���^�[�`���̏ꍇ�A�ޑK�Ǘ����v(ID135)�����Z�̖��ɑ��M�����s��Ή�(���ʉ��P��1402)
	eLOG_COINBOX,			// �R�C�����ɏW�v���v�f�[�^(ID131)
	eLOG_NOTEBOX,			// �������ɏW�v���v�f�[�^(ID133)
	eLOG_ERROR,				// �G���[�f�[�^(ID120)
	eLOG_ALARM,				// �A���[���f�[�^(ID121)
	eLOG_MONEYMANAGE,		// ���K�Ǘ��f�[�^(ID126)
	eLOG_PARKING,			// ���ԑ䐔�f�[�^(ID236)
	eLOG_MONITOR,			// ���j�^�f�[�^(ID122)
	eLOG_OPERATE,			// ���샂�j�^�f�[�^(ID123)
};
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

/* ���M�o�b�t�@�������p�e�[�u�� */
static const t_NtBufInfo	z_NtSndBufInfo[NTNET_SNDBUF_MAX] = {
	/* NTNET_SNDBUF_PRIOR */
	{
		NTBUF_NOT_SAVE,
		z_NtBufPool.snd_prior,
		sizeof(z_NtBufPool.snd_prior),
		NTBUF_PRIOR_DATASIZE_MAX,
		-1,
		ntbuf_GetDefaultSetting
	},
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
//	/* NTNET_SNDBUF_SALE */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_pay,
//		NTBUF_SALE_MEMPOOL_MAX,
//		NTBUF_SALE_DATASIZE_MAX,
//		22,
//		ntbuf_GetSaleBufSetting
//	},
//	/* NTNET_SNDBUF_CAR_OUT */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_car_out,
//		sizeof(z_NtBufPool.snd_car_out),
//		NTBUF_CAR_OUT_DATASIZE_MAX,
//		21,
//		ntbuf_GetCarOutBufSetting
//	},
//	/* NTNET_SNDBUF_CAR_IN */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_car_in,
//		sizeof(z_NtBufPool.snd_car_in),
//		NTBUF_CAR_IN_DATASIZE_MAX,
//		20,
//		ntbuf_GetCarInBufSetting
//	},
//	/* NTNET_SNDBUF_TTOTAL */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_ttotal,
//		sizeof(z_NtBufPool.snd_ttotal),
//		NTBUF_TTOTAL_DATASIZE_MAX,
//		41,
//		ntbuf_GetTTotalBufSetting
//	},
//	/* NTNET_SNDBUF_COIN */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_coin,
//		sizeof(z_NtBufPool.snd_coin),
//		NTBUF_COIN_DATASIZE_MAX,
//		131,
//		ntbuf_GetCoinBufSetting
//	},
//	/* NTNET_SNDBUF_NOTE */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_note,
//		sizeof(z_NtBufPool.snd_note),
//		NTBUF_NOTE_DATASIZE_MAX,
//		133,
//		ntbuf_GetNoteBufSetting
//	},
//	/* NTNET_SNDBUF_ERROR */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_err,
//		sizeof(z_NtBufPool.snd_err),
//		NTBUF_ERROR_DATASIZE_MAX,
//		120,
//		ntbuf_GetDefaultSetting
//	},
//	/* NTNET_SNDBUF_ALARM */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_alarm,
//		sizeof(z_NtBufPool.snd_alarm),
//		NTBUF_ALARM_DATASIZE_MAX,
//		121,
//		ntbuf_GetDefaultSetting
//	},
//	/* NTNET_SNDBUF_MONEY */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_money,
//		sizeof(z_NtBufPool.snd_money),
//		NTBUF_MONEY_DATASIZE_MAX,
//		126,
//		ntbuf_GetDefaultSetting
//	},
//	/* NTNET_SNDBUF_CAR_COUNT */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_car_count,
//		sizeof(z_NtBufPool.snd_car_count),
//		NTBUF_CAR_COUNT_DATASIZE_MAX,
//		236,
//		ntbuf_GetDefaultSetting
//	},
//	/* NTNET_SNDBUF_AREA_COUNT */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_area_count,
//		sizeof(z_NtBufPool.snd_area_count),
//		NTBUF_AREA_COUNT_DATASIZE_MAX,
//		237,
//		ntbuf_GetDefaultSetting
//	},
//	/* NTNET_SNDBUF_MONITOR */
//	{
//		NTBUF_SAVE,		
//		z_NtBufPool.snd_monitor,
//		sizeof(z_NtBufPool.snd_monitor),
//		NTBUF_MONITOR_DATASIZE_MAX,
//		122,
//		ntbuf_GetDefaultSetting
//	},
//	/* NTNET_SNDBUF_OPE_MONITOR */
//	{
//		NTBUF_SAVE,
//		z_NtBufPool.snd_ope_monitor,
//		sizeof(z_NtBufPool.snd_ope_monitor),
//		NTBUF_OPE_MONITOR_DATASIZE_MAX,
//		123,
//		ntbuf_GetDefaultSetting
//	},
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
	/* NTNET_SNDBUF_NORMAL */
	{
		NTBUF_NOT_SAVE,	
		z_NtBufPool.snd_normal,
		sizeof(z_NtBufPool.snd_normal),
		NTBUF_NORMAL_DATASIZE_MAX,
		-1,
		ntbuf_GetDefaultSetting
	},
};

/* ��M�o�b�t�@�������p�e�[�u�� */
static const t_NtBufInfo	z_NtRcvBufInfo[NTNET_RCVBUF_MAX] = {
	/* NTNET_RCVBUF_BROADCAST */
	{
		NTBUF_SAVE_BY_DATA,	
		z_NtBufPool.rcv_broadcast,	
		sizeof(z_NtBufPool.rcv_broadcast),	
		NTBUF_BROADCAST_DATASIZE_MAX,
		-1,
		ntbuf_GetDefaultSetting
	},
	/* NTNET_RCVBUF_PRIOR */
	{
		NTBUF_SAVE_BY_DATA,
		z_NtBufPool.rcv_prior,
		sizeof(z_NtBufPool.rcv_prior),
		NTBUF_PRIOR_DATASIZE_MAX,
		-1,
		ntbuf_GetDefaultSetting
	},
	/* NTNET_RCVBUF_NORMAL */
	{
		NTBUF_SAVE_BY_DATA,	
		z_NtBufPool.rcv_normal,	
		sizeof(z_NtBufPool.rcv_normal),
		NTBUF_NORMAL_DATASIZE_MAX,
		-1,
		ntbuf_GetDefaultSetting
	},
};

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
//const  long	z_NearFullBitEx[NTNET_SNDBUF_MAX] = {	// �o�b�t�@�D��x��
//	0x00000000,						// NTNET_SNDBUF_PRIOR,			/* �D��f�[�^��� */
//	NTNET_BUFCTRL_REQ_SALE,			// NTNET_SNDBUF_SALE,			/* ���Z�f�[�^(ID22/ID23) */
//	NTNET_BUFCTRL_REQ_OUTCAR,		// NTNET_SNDBUF_CAR_OUT,		/* �o�Ƀf�[�^�e�[�u��(ID21) */
//	NTNET_BUFCTRL_REQ_INCAR,		// NTNET_SNDBUF_CAR_IN,			/* ���Ƀf�[�^�e�[�u��(ID20) */
//	NTNET_BUFCTRL_REQ_TTOTAL,		// NTNET_SNDBUF_TTOTAL,			/* T���v�W�v�f�[�^(ID30�`38�AID41) */
//	NTNET_BUFCTRL_REQ_COIN,			// NTNET_SNDBUF_COIN,			/* �R�C�����ɏW�v���v�f�[�^(ID131) */
//	NTNET_BUFCTRL_REQ_NOTE,			// NTNET_SNDBUF_NOTE,			/* �������ɏW�v���v�f�[�^(ID133) */
//	NTNET_BUFCTRL_REQ_ERROR,		// NTNET_SNDBUF_ERROR,			/* �G���[�f�[�^(ID120) */
//	NTNET_BUFCTRL_REQ_ALARM,		// NTNET_SNDBUF_ALARM,			/* �A���[���f�[�^(ID121) */
//	NTNET_BUFCTRL_REQ_MONEY,		// NTNET_SNDBUF_MONEY,			/* ���K�Ǘ��f�[�^(ID126) */
//	NTNET_BUFCTRL_REQ_TURI, 										/* �ޑK�Ǘ��W�v�f�[�^(ID135) */
//	NTNET_BUFCTRL_REQ_CAR_COUNT,	// NTNET_SNDBUF_CAR_COUNT,		/* ���ԑ䐔�f�[�^(ID236) */
//	NTNET_BUFCTRL_REQ_AREA_COUNT,	// NTNET_SNDBUF_AREA_COUNT,		/* ���䐔�E���ԃf�[�^(ID237) */
//	NTNET_BUFCTRL_REQ_MONITOR,		// NTNET_SNDBUF_MONITOR,		/* ���j�^�f�[�^(ID122) */
//	NTNET_BUFCTRL_REQ_OPE_MONITOR,	// NTNET_SNDBUF_OPE_MONITOR,	/* ���샂�j�^�f�[�^(ID123) */
//	0x00000000,						// NTNET_SNDBUF_NORMAL,			/* �ʏ�f�[�^��� */
//};
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j

/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/

/*====================================================================================[PUBLIC]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_Init
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET�o�b�t�@�Ǘ����W���[��������
 *|			  �o�b�t�@���̐�������ۂ��߂̒�d�ۏ؏������s��
 *| param	: clr - 1=��d�ۏ؃f�[�^���N���A(���̂Ƃ���d�ۏ؏����͍s��Ȃ�)
 *|			        0=��d�ۏ؃f�[�^�̓N���A���Ȃ�
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTBUF_Init(uchar clr)
{
	z_NtCommErr = FALSE;
	
	if (clr) {
		z_NtAllClr = FALSE;
	}
	else {
		if (z_NtAllClr) {
			/* NTBUF_AllClr()�������ɒ�d�̂��߁A�S�f�[�^�N���A������⊮ */
			clr = 1;
		}
	}
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
//	if (clr || (!_is_ntnet_remote())) {
//		z_NearFullBuff = 0;
//	}
//	z_PendingBuff = 0;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j

	/* �o�b�t�@������ */
	ntbuf_InitNtBuffer(z_NtSndBuf, z_NtSndBufInfo, NTNET_SNDBUF_MAX, clr);	/* ���M�o�b�t�@ */
	ntbuf_InitNtBuffer(z_NtRcvBuf, z_NtRcvBufInfo, NTNET_RCVBUF_MAX, clr);	/* ��M�o�b�t�@ */
	
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//	z_NtSndFreeBuf.isdata = FALSE;											/* FREE�f�[�^���M�o�b�t�@ */
//	z_NtRcvFreeBuf.isdata = FALSE;											/* FREE�f�[�^��M�o�b�t�@ */
	// ���M����f�[�^�̏�����
	ntbuf_InitSendDataCtrl();									// ���M����f�[�^��������
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
	
	z_NtAllClr = FALSE;
	
	/* ��M�o�b�t�@���ɂ���f�[�^�����AOPE�փ��b�Z�[�W���M */
	ntbuf_ChkRcvBuf(z_NtRcvBuf);
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//	z_NtBuf_SndDataExist = ntbuf_ChkSndBuf(z_NtSndBuf, &z_NtSndFreeBuf);
	z_NtBuf_SndDataExist = ntbuf_ChkSndBuf(z_NtSndBuf);
// MH341110(S) A.Iiizumi 2017/12/18 ParkingWeb�Ή���NT-NET�͎Ԏ����W�v�͕s�v�Ȃ��ߒ[���Ԃł͑��M���Ȃ��悤�ɏC�� (���ʉ��P��1391)
// NOTE:ParkingWeb�Ή���NT-NET�̒[���ԒʐM�ŎԎ����W�v���M���邽�߂ɂ͑傫��RAM�̈悪�K�v�ɂȂ�B
// �d�l���ɂ͎Ԏ����W�v�͒�`����Ă��邪 ParkingWeb�ł͖��g�p�i�{�f�[�^���g�p����@�킪���Ȃ��j
// �Ƃ̋L�ڂ��Ԏ����W�v�͑��M���Ȃ��݌v�Ƃ���B
	if( prm_get(COM_PRM,S_NTN,121,1,1) != 0 ) { // �[���ԒʐM�F�Z���^�[�d�l
		memcpy( &z_NtBufTotalIDList, &c_NTBufTotalIDList_ParkingWeb, sizeof( z_NtBufTotalIDList ) );
	}
	else {										// �[���ԒʐM�F�W���d�l
		memcpy( &z_NtBufTotalIDList, &c_NTBufTotalIDList_Normal, sizeof( z_NtBufTotalIDList ) );
	}
// MH341110(E) A.Iiizumi 2017/12/18 ParkingWeb�Ή���NT-NET�͎Ԏ����W�v�͕s�v�Ȃ��ߒ[���Ԃł͑��M���Ȃ��悤�ɏC�� (���ʉ��P��1391)
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_AllClr
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET�o�b�t�@���̃f�[�^��S�č폜
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTBUF_AllClr(void)
{
	z_NtAllClr = TRUE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
	
	/* �o�b�t�@������ */
	ntbuf_InitNtBuffer(z_NtSndBuf, z_NtSndBufInfo, NTNET_SNDBUF_MAX, 1);	/* NT-NET�f�[�^���M�o�b�t�@ */
	ntbuf_InitNtBuffer(z_NtRcvBuf, z_NtRcvBufInfo, NTNET_RCVBUF_MAX, 1);	/* NT-NET�f�[�^��M�o�b�t�@ */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//	z_NtSndFreeBuf.isdata = FALSE;											/* FREE�f�[�^���M�o�b�t�@ */
//	z_NtRcvFreeBuf.isdata = FALSE;											/* FREE�f�[�^��M�o�b�t�@ */
//	
//	NTUPR_Init2();															/* UpperӼޭ�ّ���M̪��ނ��ޯ̧�ر */

	// ���O������
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���g�p���O�i���ɁA�o�Ɂj�j
//	Ope_Log_UnreadToRead(eLOG_ENTER, eLOG_TARGET_NTNET);		// ����
//// MH364300 GG119A34(S) ���P�A���\No.83�Ή�
//	Ope_Log_UnreadToRead(eLOG_LEAVE, eLOG_TARGET_NTNET);		// �o��
//// MH364300 GG119A34(E) ���P�A���\No.83�Ή�
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���g�p���O�i���ɁA�o�Ɂj�j
	Ope_Log_UnreadToRead(eLOG_PAYMENT, eLOG_TARGET_NTNET);		// ���Z
	Ope_Log_UnreadToRead(eLOG_TTOTAL, eLOG_TARGET_NTNET);		// T���v
	Ope_Log_UnreadToRead(eLOG_ERROR, eLOG_TARGET_NTNET);		// �G���[
	Ope_Log_UnreadToRead(eLOG_ALARM, eLOG_TARGET_NTNET);		// �A���[��
	Ope_Log_UnreadToRead(eLOG_MONITOR, eLOG_TARGET_NTNET);		// ���j�^
	Ope_Log_UnreadToRead(eLOG_OPERATE, eLOG_TARGET_NTNET);		// ���샂�j�^
	Ope_Log_UnreadToRead(eLOG_COINBOX, eLOG_TARGET_NTNET);		// �R�C�����ɏW�v
	Ope_Log_UnreadToRead(eLOG_NOTEBOX, eLOG_TARGET_NTNET);		// �������ɏW�v
	Ope_Log_UnreadToRead(eLOG_PARKING, eLOG_TARGET_NTNET);		// ���ԑ䐔
	Ope_Log_UnreadToRead(eLOG_MONEYMANAGE, eLOG_TARGET_NTNET);	// ���K�Ǘ�(SRAM)
	Ope_Log_UnreadToRead(eLOG_MNYMNG_SRAM, eLOG_TARGET_NTNET);	// �ޑK�Ǘ�
// MH341110(S) A.Iiizumi 2017/12/25 ����`�F�b�N-NT-NET�`�F�b�N�̃f�[�^�N���A������GT���v�����������Ȃ��s��C�� (���ʉ��P��1396)
	Ope_Log_UnreadToRead(eLOG_GTTOTAL, eLOG_TARGET_NTNET);		// GT���v
// MH341110(E) A.Iiizumi 2017/12/25 ����`�F�b�N-NT-NET�`�F�b�N�̃f�[�^�N���A������GT���v�����������Ȃ��s��C�� (���ʉ��P��1396)

	// ���M���W�v�f�[�^ID�C���f�b�N�X�N���A
	z_NtBufTotalDataIndex = NTBUF_NOTSENDTOTALDATA;
	
	// ���M����f�[�^�ɂ��郊�g���C�f�[�^���N���A
	ntbuf_InitSendDataCtrl();									// ���M����f�[�^��������
	
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

	z_NtAllClr = FALSE;		/* ��d�ۏ؏��������̃`�F�b�N�|�C���g */
}
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_AllClr_startup
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET�o�b�t�@���̃f�[�^��S�č폜(�N�����p)
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTBUF_AllClr_startup(void)
{
	z_NtAllClr = TRUE;		/* ��d�ۏ؂̃`�F�b�N�|�C���g */
	
	/* �o�b�t�@������ */
	ntbuf_InitNtBuffer(z_NtSndBuf, z_NtSndBufInfo, NTNET_SNDBUF_MAX, 1);	/* NT-NET�f�[�^���M�o�b�t�@ */
	ntbuf_InitNtBuffer(z_NtRcvBuf, z_NtRcvBufInfo, NTNET_RCVBUF_MAX, 1);	/* NT-NET�f�[�^��M�o�b�t�@ */
	// NOTE�F�N�����̓��O�𑗐M�ς݂ɂ���K�v���Ȃ�����NTBUF_AllClr()�̂悤�ȃ��O���M�ςݏ����͎��{���Ȃ�
	//       ���O�̏��������K�v�ȏꍇ�ʃ��W�b�N�Ŏ��{�����

	// ���M���W�v�f�[�^ID�C���f�b�N�X�N���A
	z_NtBufTotalDataIndex = NTBUF_NOTSENDTOTALDATA;
	
	// ���M����f�[�^�ɂ��郊�g���C�f�[�^���N���A
	ntbuf_InitSendDataCtrl();									// ���M����f�[�^��������
	z_NtAllClr = FALSE;		/* ��d�ۏ؏��������̃`�F�b�N�|�C���g */
}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_SetSendNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: ���MNTNET�f�[�^���o�b�t�@�ɏ�����
 *| param	: data - �������ݑΏۂƂȂ�f�[�^					<IN>
 *|			  len - data�̃o�C�g��
 *|			  bufkind - �o�b�t�@���
 *|					   (NTNET_BUF_BUFFERING/NTNET_BUF_PRIOR/NTNET_BUF_NORMAL)
 *|	return	: NTNET_BUFSET_NORMAL=����I��
 *|			  NTNET_BUFSET_DEL_OLD=�ŌÃf�[�^������
 *|			  NTNET_BUFSET_DEL_NEW=�ŐV�f�[�^������
 *|			  NTNET_BUFSET_CANT_DEL=�o�b�t�@FULL�����ݒ肪"�x��"�̂��ߏ����s��
 *|			  NTNET_BUFSET_COMERR=IBK�ŒʐM��Q�������̂��߃Z�b�g���Ȃ�
 *|			  NTNET_BUFSET_STATE_CHG=�o�b�t�@��ԕω�(�����݂͊���)
 *[]----------------------------------------------------------------------[]
 *| remark	:
 *|		data���w�������f�[�^�C���[�W�̃t�H�[�}�b�g�͎��̒ʂ�B
 *|			�wNT-NET �ʐM�t�H�[�}�b�g�d�l���x�́u5.�f�[�^�t�H�[�}�b�g�ڍׁv��
 *|			�L�ڂ��ꂽ�f�[�^�t�H�[�}�b�g�B(0�J�b�g���̈��k���s���Ă��Ȃ�����)
 *|			�f�[�^���ڒ��́u�V�[�P���XNo.�v�ɂ��Ă�NT-NET�^�X�N����̑��M����
 *|			�쐬����̂ŁA�s��ł悢�B
 *[]----------------------------------------------------------------------[]*/
uchar	NTBUF_SetSendNtData(const void *data, ushort len, eNTNET_BUF_KIND bufkind)
{
	int		bufcode;
	uchar	ret;
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
//	uchar *dt;
//
//	return	NTNET_BUFSET_NORMAL;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j

	/* NT-NET�ʐM�������͂����Œ�~���� */
	if( _not_ntnet() ){		/* NT-NET�ʐM���� */
		return	NTNET_BUFSET_NORMAL;
	}

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
//	// ���M�s�v�Ȃ�Z�b�g�����ɏI��
//	dt = (uchar*)data;
//	if (dt[0] != REMOTE_SYSTEM) {
//		if (ntbuf_IsSendData(dt[1]) == 0) {
//			return NTNET_BUFSET_NORMAL;
//		}
//	}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j

	/* �Y���o�b�t�@���擾 */
	bufcode = ntbuf_GetSndNtBufCode(data, bufkind);
	if (bufcode < 0 || NTNET_SNDBUF_MAX <= bufcode) {
		return NTNET_BUFSET_NORMAL;		/* ���̃p�X�͒ʂ�Ȃ��͂� */
	}
	/* �f�[�^���`�F�b�N */
	if (!ntbuf_ChkSndNtDataLen(bufcode, len)) {
		return NTNET_BUFSET_NORMAL;		/* ���̃p�X�͒ʂ�Ȃ��͂� */
	}
	/* �ʐM��Q��Ԃ��`�F�b�N */
	if (z_NtCommErr) {
		if (z_NtSndBuf[bufcode].save == NTBUF_NOT_SAVE) {
			return NTNET_BUFSET_COMERR;	/* �ʐM��Q���̂��߃Z�b�g���Ȃ� */
		}
	}
	/* �o�b�t�@��Ԃ��`�F�b�N */
	if (z_NtSndBuf[bufcode].setting() == NTNET_BUFSET_CANT_DEL) {
		if (ntbuf_GetBufState(&z_NtSndBuf[bufcode]) & NTBUF_BUFFER_FULL) {
			return NTNET_BUFSET_CANT_DEL;	/* �o�b�t�@FULL���̓���ݒ肪�u�x�Ɓv���A�o�b�t�@FULL */
		}
	}
	/* �o�b�t�@�Ƀf�[�^���Z�b�g */
	ret = ntbuf_SetNtData(&z_NtSndBuf[bufcode], data, len);
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
//	if ((ret != NTNET_BUFSET_CANT_DEL) &&
//		((z_NearFullBitEx[bufcode] & z_NearFullBuff) == 0) &&
//		((z_NearFullBitEx[bufcode] & z_PendingBuff) == 0)) {
	if (ret != NTNET_BUFSET_CANT_DEL) {
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
		z_NtBuf_SndDataExist = TRUE;
	}
	if (ret == NTNET_BUFSET_NORMAL) {
		if (ntbuf_ChkBufState(&z_NtSndBuf[bufcode])) {
			ret = NTNET_BUFSET_STATE_CHG;
		}
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_GetRcvNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: ��MNTNET�f�[�^�̎擾
 *| param	: data - �f�[�^���R�s�[��ƂȂ郏�[�N�o�b�t�@	<OUT>
 *|			  bufkind - �o�b�t�@���
 *|					   (NTNET_BUF_BROADCAST/NTNET_BUF_PRIOR/NTNET_BUF_NORMAL)
 *| return	: ����data�ɃR�s�[�����o�C�g��
 *|			  0���w��o�b�t�@�Ƀf�[�^�Ȃ�
 *[]----------------------------------------------------------------------[]*/
ushort	NTBUF_GetRcvNtData(void *data, eNTNET_BUF_KIND bufkind)
{
	ushort	ret;
	int		bufcode;
	
	ret = 0;
	
	/* �Y���o�b�t�@���擾 */
	bufcode = ntbuf_GetRcvNtBufCode(bufkind);
	if (0 <= bufcode && bufcode < NTNET_RCVBUF_MAX) {
		ret = ntbuf_GetNtData(&z_NtRcvBuf[bufcode], data, 1);	// 0��ĕ���L������Get
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_ClrRcvNtData_Prepare
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M�f�[�^1�����N���A(�`�F�b�N�|�C���g����)
 *| param	: bufkind - �f�[�^���폜����o�b�t�@
 *|					   (NTNET_BUF_BROADCAST/NTNET_BUF_PRIOR/NTNET_BUF_NORMAL)
 *|			  handle - �f�[�^�폜�n���h��					<OUT>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTBUF_ClrRcvNtData_Prepare(eNTNET_BUF_KIND bufkind, t_NtNet_ClrHandle *handle)
{
	int bufcode = ntbuf_GetRcvNtBufCode(bufkind);
	
	if (0 <= bufcode && bufcode < NTNET_RCVBUF_MAX) {
		handle->bufcode	= bufcode;
		handle->rofs	= z_NtRcvBuf[bufcode].rofs;
	}
	else {
		handle->bufcode = NTNET_RCVBUF_MAX;		/* ���̃p�X�͒ʂ�Ȃ��͂� */
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_ClrRcvNtData_Exec
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M�f�[�^1�����N���A(�N���A���s����)
 *| param	: handle - �f�[�^�폜�n���h��					<IN>
 *|					   (NTBUF_ClrRcvData_Check()�ō쐬���ꂽ���̂ł��邱��)
 *| return	: none
 *[]----------------------------------------------------------------------[]
 *|	remark	:
 *|		��d�������ɂ�����f�[�^��2�d�폜�h�~�̂��߁A�{�֐���NTBUF_ClrRcvData_Prepare��
 *|		�Z�b�g�Ŏg�p����B�ȉ��͎g�p��B
 *|		�y�ʏ펞�z
 *|			// ��d�ۏ؃f�[�^->
 *|			t_NtNet_ClrHandle	h;
 *|			uchar				phase = 0;
 *|			// <-��d�ۏ؃f�[�^
 *|			
 *|			// (���P)
 *|			
 *|			phase = 1;		// --------------------- �폜�`�F�b�N�|�C���g�J�n
 *|			// (���Q)
 *|			NTBUF_ClrRcvData_Prepare(NTNET_BUF_PRIOR, &h);
 *|			
 *|			phase = 2;		// --------------------- �폜�J�n
 *|			// (���R)
 *|			NTBUF_ClrRcvData_Exec(&h)
 *|			
 *|			phase = 3;		// --------------------- �폜����
 *|		
 *|		�y��d���A���Aphase=0�z
 *|			��L"���P"�n�_���珈��
 *|		
 *|		�y��d���A���Aphase=1�z
 *|			��L"���Q"�n�_���珈��
 *|		
 *|		�y��d���A���Aphase=2�z
 *|			��L"���R"�n�_���珈��
 *|		
 *|		�y��d���A���Aphase=3�z
 *|			���d�����Ȃ�
 *|		
 *[]----------------------------------------------------------------------[]*/
void	NTBUF_ClrRcvNtData_Exec(const t_NtNet_ClrHandle *handle)
{
	if (0 <= handle->bufcode && handle->bufcode < NTNET_RCVBUF_MAX) {
		if (z_NtRcvBuf[handle->bufcode].rofs == handle->rofs) {
			ntbuf_NtDataClr(&z_NtRcvBuf[handle->bufcode]);
		}
		else {
			/* �폜�ς� */
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_CommErr
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET�ʐM�ُ픭��/�����ʒm
 *| param	: sts - IBK����̃G���[�f�[�^"01H"���󂯎�����Ƃ��̔���(01H)/����(00H)
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTBUF_CommErr(uchar sts)
{
	int i;
	
	if (sts == 1) {
		/* ���MNT-NET�f�[�^�̂����A�f�[�^�ێ��t���O��OFF�̃o�b�t�@��S�ăN���A */
		z_NtCommErr = TRUE;
		for (i = 0; i < _countof(z_NtSndBuf); i++) {
			if (z_NtSndBuf[i].save == NTBUF_NOT_SAVE) {
				z_NtSndBuf[i].rofs = 
				z_NtSndBuf[i].wofs = 0;
			}
		}
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//		z_NtBuf_SndDataExist = ntbuf_ChkSndBuf(z_NtSndBuf, &z_NtSndFreeBuf);
		z_NtBuf_SndDataExist = ntbuf_ChkSndBuf(z_NtSndBuf);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
	}
	else {	/* sts == 0 */
		z_NtCommErr = FALSE;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_SetRcvNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: ��MNTNET�f�[�^���o�b�t�@�ɏ�����
 *| param	: data - �������ݑΏۂƂȂ�f�[�^					<IN>
 *|			  len - data�̃o�C�g��
 *|			  bufkind - �o�b�t�@���
 *|					   (NTNET_BUF_BROADCAST/NTNET_BUF_PRIOR/NTNET_BUF_NORMAL)
 *|	return	: NTNET_BUFSET_NORMAL=����I��
 *|			  NTNET_BUFSET_DEL_OLD=�ŌÃf�[�^������
 *[]----------------------------------------------------------------------[]*/
uchar	NTBUF_SetRcvNtData(const void *data, ushort len, eNTNET_BUF_KIND bufkind)
{
	uchar	ret;
	int		bufcode;
	
	/* �Y���o�b�t�@���擾 */
	bufcode = ntbuf_GetRcvNtBufCode(bufkind);
	if (bufcode < 0 || NTNET_RCVBUF_MAX <= bufcode) {
		return NTNET_BUFSET_NORMAL;		/* ���̃p�X�͒ʂ�Ȃ��͂� */
	}
	/* �f�[�^���`�F�b�N */
	if (!ntbuf_ChkRcvNtDataLen(bufcode, len)) {
		return NTNET_BUFSET_NORMAL;		/* ���̃p�X�͒ʂ�Ȃ��͂� */
	}
	/* �o�b�t�@�Ƀf�[�^���Z�b�g */
	ret = ntbuf_SetNtData(&z_NtRcvBuf[bufcode], data, len);
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
	if(ret == NTNET_BUFSET_DEL_NEW || ret == NTNET_BUFSET_DEL_OLD) {	// �o�b�t�@�t���̂��߃f�[�^�폜
		switch(bufkind) {
		case NTNET_BUF_PRIOR:
			NTNET_Err(ERR_NTNET_PRIOR_RCVBUF, NTERR_ONESHOT);		// �D��
			break;
		case NTNET_BUF_NORMAL:
			NTNET_Err(ERR_NTNET_NORMAL_RCVBUF, NTERR_ONESHOT);		// �ʏ�
			break;
		case NTNET_BUF_BROADCAST:
			NTNET_Err(ERR_NTNET_BROADCAST_RCVBUF, NTERR_ONESHOT);	// ����
			break;
		default:
			break;
		}
	}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

	/* OPE��NTNET�f�[�^��M��ʒm */
	queset(OPETCBNO, (ushort)IBK_NTNET_DAT_REC, 0, NULL);
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_GetSndNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: ���MNTNET�f�[�^���o�b�t�@����擾
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
// *| param	: data - ���MNT-NET�f�[�^�R�s�[��
 *| param	: pCtrl - ���M����f�[�^
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
 *|			  bufkind - �o�b�t�@���
 *|					   (NTNET_BUF_PRIOR/NTNET_BUF_NORMAL)
 *| return	: ����data�ɃR�s�[�����o�C�g��
 *|			  0���w��o�b�t�@�Ƀf�[�^�Ȃ�
 *[]----------------------------------------------------------------------[]*/
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//ushort	NTBUF_GetSndNtData(void *data, eNTNET_BUF_KIND bufkind, ushort *UpperQueKind)
ushort	NTBUF_GetSndNtData(t_Ntnet_SendDataCtrl *pCtrl, eNTNET_BUF_KIND bufkind)
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
{
	int		i;
	ushort	ret;
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//	t_NtBuf	*buffer;
	ret = 0;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
	
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
//	buffer = NULL;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
	switch (bufkind) {
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//	case NTNET_BUF_PRIOR:
//		/* �D��x���Ƀf�[�^�̂���o�b�t�@������ */
//		for (i = NTNET_SNDBUF_PRIOR_TOP; i <= NTNET_SNDBUF_PRIOR_END; i++) {
//			if (_ntbuf_IsNtData(&z_NtSndBuf[i])) {
//			// �w��̃o�b�t�@���j�A�t�����(IBK)�Ȃ�ǂݏo�����Ɏ��̃o�b�t�@���T�[�`
//				if ((z_NearFullBitEx[i] & z_NearFullBuff) ||
//					(z_NearFullBitEx[i] & z_PendingBuff))
//					continue;
//				buffer	= &z_NtSndBuf[i];
//				*UpperQueKind = (ushort)i;
//				break;
//			}
//		}
	case NTNET_BUF_PRIOR:	// �o�b�t�@�����O�f�[�^�ł͂Ȃ��D��f�[�^
		// �D��x���Ƀf�[�^�̂���o�b�t�@������
		// �D��f�[�^�o�b�t�@
		if (_ntbuf_IsNtData(&z_NtSndBuf[NTNET_SNDBUF_PRIOR])) {
			ret = ntbuf_GetNtData(&z_NtSndBuf[NTNET_SNDBUF_PRIOR], pCtrl->data, 1);	// ���M�o�b�t�@���̃f�[�^1�����[���J�b�g�L�����ăR�s�[����B
			ntbuf_NtDataClr(&z_NtSndBuf[NTNET_SNDBUF_PRIOR]);	// �f�[�^���o�b�t�@����폜
			pCtrl->logid = eLOG_MAX;							// ���O�f�[�^�ł͂Ȃ�
			break;
		}
		
// GM849100(S) ���S�����R�[���Z���^�[�Ή��i960�o�C�g�𒴂��邽�ߐ��Z�f�[�^��ʏ�f�[�^�ő��M����j�iGT-7700:GM747902�Q�l�j
//		// ���Z�f�[�^���O
//		if (Ope_Log_UnreadCountGet(eLOG_PAYMENT, eLOG_TARGET_NTNET)) {			// NT-NET���ǐ��ZLOG����
//				// �f�[�^�𖢑��M��Ԃ̂܂܎擾
//			if (Ope_Log_TargetDataVoidRead(eLOG_PAYMENT, RAU_LogData, eLOG_TARGET_NTNET, TRUE)) {	
//				// ���O�f�[�^��NTNET�f�[�^�ɕϊ�����
//				ret = NTNET_ConvertLogToNTNETData(eLOG_PAYMENT, RAU_LogData, pCtrl->data);
//// MH364300 GG119A23(S) // GG122600(S) ���O������e���O���o�^�����ƃ��Z�b�g���J��Ԃ�
//				if (ret == 0) {								// ��ǂݑΏۂȂ瑗�M�ςɂ���
//					Ope_Log_TargetVoidReadPointerUpdate(eLOG_PAYMENT, eLOG_TARGET_NTNET);
//					ret = 0;								// ���M�Ώۃf�[�^�Ȃ�
//				}
//// MH364300 GG119A23(E) // GG122600(E) ���O������e���O���o�^�����ƃ��Z�b�g���J��Ԃ�
//				pCtrl->logid = eLOG_PAYMENT;
//			}
//		}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��i960�o�C�g�𒴂��邽�ߐ��Z�f�[�^��ʏ�f�[�^�ő��M����j�iGT-7700:GM747902�Q�l�j
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
		break;
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//	case NTNET_BUF_NORMAL:
//		/* �D��x���Ƀf�[�^�̂���o�b�t�@������ */
//		for (i = NTNET_SNDBUF_NORMAL_TOP; i <= NTNET_SNDBUF_NORMAL_END; i++) {
//			if (_ntbuf_IsNtData(&z_NtSndBuf[i])) {
//			// �w��̃o�b�t�@���j�A�t�����(IBK)�Ȃ�ǂݏo�����Ɏ��̃o�b�t�@���T�[�`
//				if ((z_NearFullBitEx[i] & z_NearFullBuff) ||
//					(z_NearFullBitEx[i] & z_PendingBuff))
//					continue;
//				buffer	= &z_NtSndBuf[i];
//				*UpperQueKind = (ushort)i;
//				break;
//			}
//		}
	case NTNET_BUF_NORMAL:	// �o�b�t�@�����O�f�[�^�ł͂Ȃ��ʏ�f�[�^
		// �D��x���Ƀf�[�^�̂���o�b�t�@������
		// ���O�f�[�^
		if(z_NtBufTotalDataIndex == NTBUF_NOTSENDTOTALDATA) {			// �W�v�f�[�^�����M���
			for (i = NTNET_SNDLOG_NORMAL_TOP; i < NTNET_SNDLOG_NORMAL_END; i++) {
				if(eLOG_TTOTAL != z_NtSndLogInfo[i] && eLOG_GTTOTAL != z_NtSndLogInfo[i]) {	// �W�v�f�[�^�ȊO
					if (Ope_Log_UnreadCountGet(z_NtSndLogInfo[i], eLOG_TARGET_NTNET)) {	// NT-NET���ǐ��ZLOG����
						// �f�[�^�𖢑��M��Ԃ̂܂܎擾
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���u�ƒ[���ԒʐM�̃o�b�t�@��ʂɂ���j
//						if (Ope_Log_TargetDataVoidRead(z_NtSndLogInfo[i], RAU_LogData, eLOG_TARGET_NTNET, TRUE)) {
						if (Ope_Log_TargetDataVoidRead(z_NtSndLogInfo[i], NTNET_LogData, eLOG_TARGET_NTNET, TRUE)) {
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���u�ƒ[���ԒʐM�̃o�b�t�@��ʂɂ���j
							// ���O�f�[�^��NTNET�f�[�^�ɕϊ�����
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���u�ƒ[���ԒʐM�̃o�b�t�@��ʂɂ���j
//							ret = NTNET_ConvertLogToNTNETData(z_NtSndLogInfo[i], RAU_LogData, pCtrl->data);
							ret = NTNET_ConvertLogToNTNETData(z_NtSndLogInfo[i], NTNET_LogData, pCtrl->data);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���u�ƒ[���ԒʐM�̃o�b�t�@��ʂɂ���j
							if(ret == 0) {								// ���M���x���������̋�ǂݑΏۂȂ瑗�M�ςɂ���
								Ope_Log_TargetVoidReadPointerUpdate(z_NtSndLogInfo[i], eLOG_TARGET_NTNET);
								ret = 0;								// ���M�Ώۃf�[�^�Ȃ�
							}
							pCtrl->logid = z_NtSndLogInfo[i];
							break;
						}
					}
				}
				else {																	// �W�v�f�[�^
// MH364300 GG119A14(S) ���P�A���\No.11�Ή�
//// MH364300 GG119A11(S) // GM569000 (S) ParkingWeb�[���ԁi�t�F�[�Y�Q�j�ʐM�Ή�
//					if( (prm_get(COM_PRM, S_NTN, 121, 1, 1) == 0) && 
//								(eLOG_GTTOTAL == z_NtSndLogInfo[i]) ) {
//						continue;		// �ʏ�̂m�s�m�d�s�̏ꍇ�A�f�s�W�v�i���v�j�͑��M���Ȃ�
//					}
//// MH364300 GG119A11(E) // GM569000 (E) ParkingWeb�[���ԁi�t�F�[�Y�Q�j�ʐM�Ή�
// MH364300 GG119A14(E) ���P�A���\No.11�Ή�
					if (Ope_Log_UnreadCountGet(z_NtSndLogInfo[i], eLOG_TARGET_NTNET)) {	// NT-NET���ǐ��ZLOG����
						// �f�[�^�𖢑��M��Ԃ̂܂܎擾
						if (Ope_Log_TargetDataVoidRead(z_NtSndLogInfo[i], z_NtBufTotalLog, eLOG_TARGET_NTNET, TRUE)) {
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���Z�f�[�^�A�G���[�f�[�^�݂̂���j
							if( prm_get(COM_PRM, S_SSS, 1, 1, 1) == 1 ) {		// ���S�����d�lNT-NET�[���ԒʐM
								Ope_Log_TargetVoidReadPointerUpdate(z_NtSndLogInfo[i], eLOG_TARGET_NTNET);
								z_NtBufTotalDataIndex = NTBUF_NOTSENDTOTALDATA;	// �W�v�f�[�^�����M���
								ret = 0;// ���M�Ώۃf�[�^�Ȃ�
								break;
							}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���Z�f�[�^�A�G���[�f�[�^�݂̂���j
// MH341110(S) A.Iiizumi 2017/12/20 NT-NET�[���ԒʐM�Ŋ����`���̏ꍇ�AGT���v�𑗐M����s��C�� (���ʉ��P��1389)
							if( ( prm_get(COM_PRM, S_NTN, 121, 1, 1) == 0 )&&(eLOG_GTTOTAL == z_NtSndLogInfo[i]) ){
								// �����`���̏ꍇ�AGT���v�͑��M���Ȃ��d�l�Ȃ̂ŋ�ǂ݂��đ��M�ςɂ���
								// ���M�ς݂ɕύX����ꍇ�ł�Ope_Log_TargetDataVoidRead��Ope_Log_TargetVoidReadPointerUpdate�̎菇�𓥂ނ���
								Ope_Log_TargetVoidReadPointerUpdate(z_NtSndLogInfo[i], eLOG_TARGET_NTNET);
								z_NtBufTotalDataIndex = NTBUF_NOTSENDTOTALDATA;// �W�v�f�[�^�����M���
								ret = 0;// ���M�Ώۃf�[�^�Ȃ�
								break;
							}
// MH341110(E) A.Iiizumi 2017/12/20 NT-NET�[���ԒʐM�Ŋ����`���̏ꍇ�AGT���v�𑗐M����s��C�� (���ʉ��P��1389)
							// ���O�f�[�^��NTNET�f�[�^�ɕϊ�����
							// �W�v�f�[�^��ID30�`36, 41�ɕ������邽�ߐ�p�̃o�b�t�@�Ɋi�[����
							z_NtBufTotalDataIndex = 0;
							pCtrl->logid = z_NtSndLogInfo[i];
							ret = NTNET_ConvertTotalLogToNTNETData(pCtrl->logid,
																		z_NtBufTotalIDList[z_NtBufTotalDataIndex],
																		z_NtBufTotalLog, pCtrl->data);
							++z_NtBufTotalDataIndex;
							break;
						}
					}
				}
			}
		}
		else {															// �W�v�f�[�^���M��
			if(0 != z_NtBufTotalIDList[z_NtBufTotalDataIndex]) {		// ���M�f�[�^����
				ret = NTNET_ConvertTotalLogToNTNETData(pCtrl->logid,
														z_NtBufTotalIDList[z_NtBufTotalDataIndex],
														z_NtBufTotalLog, pCtrl->data);
				++z_NtBufTotalDataIndex;								// �W�v�f�[�^ID�C���f�b�N�X�X�V
				if(0 == z_NtBufTotalIDList[z_NtBufTotalDataIndex]) {	// �W�v�I���ʒm�f�[�^������
					z_NtBufTotalDataIndex = NTBUF_NOTSENDTOTALDATA;
				}
			}
			else {														// �W�v�f�[�^���M�����I��
				z_NtBufTotalDataIndex = NTBUF_NOTSENDTOTALDATA;
			}
			break;
		}

		// �ʏ�f�[�^�o�b�t�@
		if (_ntbuf_IsNtData(&z_NtSndBuf[NTNET_SNDBUF_NORMAL])) {
			ret = ntbuf_GetNtData(&z_NtSndBuf[NTNET_SNDBUF_NORMAL], pCtrl->data, 1);	// ���M�o�b�t�@���̃f�[�^1�����[���J�b�g�L�����ăR�s�[����B
			ntbuf_NtDataClr(&z_NtSndBuf[NTNET_SNDBUF_NORMAL]);			// �f�[�^���o�b�t�@����폜
			pCtrl->logid = eLOG_MAX;									// ���O�f�[�^�ł͂Ȃ�
			break;
		}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
		break;
	default:
		break;
	}
	
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
//	ret = 0;
//	if (buffer != NULL) {
//	/* �f�[�^�R�s�[ */
//		ret = ntbuf_GetNtData(buffer, data, 1);					// 0��ĕ���L������Get
//																// ���M���O�ɂ�����x0�J�b�g����
//	}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
	
	return ret;
}

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
///*[]----------------------------------------------------------------------[]
// *|	name	: NTBUF_ClrSndNtData_Prepare
// *[]----------------------------------------------------------------------[]
// *| summary	: ���M�f�[�^1�����N���A(�`�F�b�N�|�C���g����)
// *| param	: data - �폜����f�[�^
// *|			  bufkind - �f�[�^���폜����o�b�t�@
// *|					   (NTNET_BUF_BUFFERING/NTNET_BUF_PRIOR/NTNET_BUF_NORMAL)
// *|			  handle - �f�[�^�폜�n���h��					<OUT>
// *| return	: none
// *[]----------------------------------------------------------------------[]*/
//void	NTBUF_ClrSndNtData_Prepare(const void *data, eNTNET_BUF_KIND bufkind, t_NtNet_ClrHandle *handle)
//{
//	int bufcode = ntbuf_GetSndNtBufCode(data, bufkind);
//	
//	if (0 <= bufcode && bufcode < NTNET_SNDBUF_MAX) {
//		handle->bufcode	= bufcode;
//		handle->rofs	= z_NtSndBuf[bufcode].rofs;
//		{
//			ulong	fofs;
//			fofs = (ulong)z_NtSndBuf[bufcode].rofs + _offsetof(t_NtBufDataHeader, reserve);
//			if (fofs >= z_NtSndBuf[bufcode].buffer_size)
//				fofs -= z_NtSndBuf[bufcode].buffer_size;
//			z_NtSndBuf[bufcode].buffer[fofs] = 0xaa;		/* set prepared flag */
//		}
//	}
//	else {
//		handle->bufcode = NTNET_SNDBUF_MAX;		/* ���̃p�X�͒ʂ�Ȃ��͂� */
//	}
//}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_ClrSndNtData_Exec
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M�f�[�^1�����N���A(�N���A���s����)
 *| param	: handle - �f�[�^�폜�n���h��					<IN>
 *|					   (NTBUF_ClrSndData_Check()�ō쐬���ꂽ���̂ł��邱��)
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTBUF_ClrSndNtData_Exec(const t_NtNet_ClrHandle *handle)
{
	if (0 <= handle->bufcode && handle->bufcode < NTNET_SNDBUF_MAX) {
		if (z_NtSndBuf[handle->bufcode].rofs == handle->rofs) {
			ulong	fofs;
			fofs = (ulong)z_NtSndBuf[handle->bufcode].rofs + _offsetof(t_NtBufDataHeader, reserve);
			if (fofs >= z_NtSndBuf[handle->bufcode].buffer_size)
				fofs -= z_NtSndBuf[handle->bufcode].buffer_size;
			/* check  prepared flag */
			if (z_NtSndBuf[handle->bufcode].buffer[fofs] != 0xaa)
				return;		/* �폜�ς� */
			ntbuf_NtDataClr(&z_NtSndBuf[handle->bufcode]);
			/* �o�b�t�@��Ԃ��`�F�b�N */
			if (ntbuf_ChkBufState(&z_NtSndBuf[handle->bufcode])) {
				if (!z_NTNET_AtInitial) {
				/* OPE�փo�b�t�@��ԕω���ʒm */
					queset(OPETCBNO, (ushort)IBK_NTNET_BUFSTATE_CHG, sizeof(z_NtSndBuf[handle->bufcode].id), &z_NtSndBuf[handle->bufcode].id);
				}
			}
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//			z_NtBuf_SndDataExist = ntbuf_ChkSndBuf(z_NtSndBuf, &z_NtSndFreeBuf);
			z_NtBuf_SndDataExist = ntbuf_ChkSndBuf(z_NtSndBuf);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
		}
		else {
			/* �폜�ς� */
		}
	}
}

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
///*[]----------------------------------------------------------------------[]
// *|	name	: NTBUF_SetSendFreeData
// *[]----------------------------------------------------------------------[]
// *| summary	: ���MFREE�f�[�^���o�b�t�@�ɏ�������
// *| param	: data - �������ݑΏۂƂȂ�f�[�^					<IN>
// *| return	: 1������I��
// *|			  0���o�b�t�@�I�[�o�[���C�g����
// *[]----------------------------------------------------------------------[]
// *| remark	:
// *|		data�ɂ́A���M�f�[�^����500byte�����ʃR�[�h("FREE")�`FREEDATA����
// *|		�Z�b�g����Ă��邱��
// *[]----------------------------------------------------------------------[]*/
//uchar	NTBUF_SetSendFreeData(const void *data)
//{
//	uchar ret = ntbuf_SetFreeData(&z_NtSndFreeBuf, data);
//	
//	z_NtBuf_SndDataExist = TRUE;
//	
//	return ret;
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: NTBUF_GetRcvFreeData
// *[]----------------------------------------------------------------------[]
// *| summary	: ��MFREE�f�[�^�o�b�t�@����f�[�^���擾����
// *| param	: data - �f�[�^�R�s�[��					<OUT>
// *| return	: 1������I��
// *|			  0���f�[�^����
// *[]----------------------------------------------------------------------[]
// *| remark	: �f�[�^�R�s�[������A�o�b�t�@����FREE�f�[�^�̓N���A�����
// *[]----------------------------------------------------------------------[]*/
//uchar	NTBUF_GetRcvFreeData(void *data)
//{
//	return ntbuf_GetFreeData(&z_NtRcvFreeBuf, data);
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: NTBUF_SetRcvFreeData
// *[]----------------------------------------------------------------------[]
// *| summary	: ���MFREE�f�[�^���o�b�t�@�ɏ�������
// *| param	: data - �������ݑΏۂƂȂ�f�[�^					<IN>
// *| return	: 1������I��
// *|			  0���o�b�t�@�I�[�o�[���C�g����
// *[]----------------------------------------------------------------------[]
// *| remark	:
// *|		data�ɂ́A���M�f�[�^����500byte�����ʃR�[�h("FREE")�`FREEDATA����
// *|		�Z�b�g����Ă��邱��
// *[]----------------------------------------------------------------------[]*/
//uchar	NTBUF_SetRcvFreeData(const void *data)
//{
//	uchar ret = ntbuf_SetFreeData(&z_NtRcvFreeBuf, data);
//	
//	/* OPE��FREE�f�[�^��M��ʒm */
//	queset(OPETCBNO, (ushort)IBK_NTNET_FREE_REC, 0, NULL);
//	
//	return ret;
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: NTBUF_GetSndFreeData
// *[]----------------------------------------------------------------------[]
// *| summary	: ��MFREE�f�[�^�o�b�t�@����f�[�^���擾����
// *| param	: data - �f�[�^�R�s�[��					<OUT>
// *| return	: 1������I��
// *|			  0���f�[�^����
// *[]----------------------------------------------------------------------[]
// *| remark	: �f�[�^�R�s�[������A�o�b�t�@����FREE�f�[�^�̓N���A�����
// *[]----------------------------------------------------------------------[]*/
//uchar	NTBUF_GetSndFreeData(void *data)
//{
//
//	uchar ret = ntbuf_GetFreeData(&z_NtSndFreeBuf, data);
//	
//	if (ret) {
//		z_NtBuf_SndDataExist = ntbuf_ChkSndBuf(z_NtSndBuf, &z_NtSndFreeBuf);
//	}
//	
//	return ret;
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: NTBUF_SetRcvErrData
// *[]----------------------------------------------------------------------[]
// *| summary	: ��M�����G���[�f�[�^��OPE�ɒʒm����
// *| param	: data - ��M�����G���[�f�[�^
// *|					�^�[�~�i��No.�`�\����11�o�C�g
// *| return	: none
// *[]----------------------------------------------------------------------[]*/
//void	NTBUF_SetRcvErrData(const void *data)
//{
//	uchar errdata[10];
//	
//	/* OPE�փG���[�f�[�^��ʒm */
//	memcpy(errdata, &((uchar*)data)[1], sizeof(errdata));		/* CRM�͏]�ǌŒ�Ȃ̂Ń^�[�~�i��No.�͍��̂Ƃ��떢�g�p */
//	queset(OPETCBNO, (ushort)IBK_NTNET_ERR_REC, sizeof(errdata), errdata);
//}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_GetBufState
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�t�@��Ԃ�Ԃ�
 *| param	: none
 *| return	: �o�b�t�@��ԃf�[�^
 *[]----------------------------------------------------------------------[]*/
const t_NtBufState	*NTBUF_GetBufState(void)
{
	return &z_NtBufState;
}


/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_SetIBKNearFull
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�t�@���̃j�A�t����Ԃ��Z�b�g
 *| param	: 
 *| return	: 
 *[]----------------------------------------------------------------------[]*/
void NTBUF_SetIBKNearFull(ulong mask, ulong occur)
{
	z_NearFullBuff &= ~mask;	// �ω��̂������ꏊ�����
	z_NearFullBuff |= occur;	// ����/�����̕ύX
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
//
//	// ���M�o�b�t�@��Ԃ��X�V
//	z_NtBuf_SndDataExist = ntbuf_ChkSndBuf(z_NtSndBuf, &z_NtSndFreeBuf);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
}

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
///*[]----------------------------------------------------------------------[]
// *|	name	: NTBUF_SetIBKNearFull
// *[]----------------------------------------------------------------------[]
// *| summary	: �o�b�t�@���̃j�A�t����Ԃ��Z�b�g
// *| param	: 
// *| return	: 
// *[]----------------------------------------------------------------------[]*/
//void NTBUF_SetIBKNearFullByID(uchar ID)
//{
//	ntbuf_SetIBKBuffStopByID(ID, &z_NearFullBuff);
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: NTBUF_SetIBKNearFull
// *[]----------------------------------------------------------------------[]
// *| summary	: �o�b�t�@���̃y���f�B���O��Ԃ��Z�b�g
// *| param	: 
// *| return	: 
// *[]----------------------------------------------------------------------[]*/
//void NTBUF_SetIBKPendingByID(uchar ID)
//{
//	if (ID == 0xff) {
//		// 0xff�w��Ȃ�S�y���f�B���O����
//		z_PendingBuff = 0;
//	} else {
//		ntbuf_SetIBKBuffStopByID(ID, &z_PendingBuff);
//	}
//}
//
//void ntbuf_SetIBKBuffStopByID(uchar ID, ulong *bit)
//{
//	// ID��񂩂�r�b�g�����쐬
//	switch (ID) {
//	case 20:		// ���Ƀf�[�^�e�[�u��(ID20)
//	case 54:
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_CAR_IN];
//		break;
//	case 21:		// �o�Ƀf�[�^�e�[�u��(ID21)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_CAR_OUT];
//		break;
//	case 22:		// ���Z�f�[�^(ID22/ID23)
//	case 23:
//	case 56:
//	case 57:
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_SALE];
//		break;
//	case 120:		// �G���[�f�[�^(ID120)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_ERROR];
//		break;
//	case 121:		// �A���[���f�[�^(ID121)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_ALARM];
//		break;
//	case 122:		// ���j�^�f�[�^(ID122)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_MONITOR];
//		break;
//	case 123:		// ���샂�j�^�f�[�^(ID123)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_OPE_MONITOR];
//		break;
//	case 131:		// �R�C�����ɏW�v���v�f�[�^(ID131)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_COIN];
//		break;
//	case 133:		// �������ɏW�v���v�f�[�^(ID133)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_NOTE];
//		break;
//	case 236:		// ���ԑ䐔�f�[�^(ID236)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_CAR_COUNT];
//		break;
//	case 237:		// ���䐔�E���ԃf�[�^(ID237)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_AREA_COUNT];
//		break;
//	case 126:		// ���K�Ǘ��f�[�^(ID126)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_MONEY];
//		break;
//	default:		// T���v�W�v�f�[�^(ID30�`38�AID41)
//		*bit |= z_NearFullBitEx[NTNET_SNDBUF_TTOTAL];
//		break;
//	}
//
//	// ���M�o�b�t�@��Ԃ��X�V
//	z_NtBuf_SndDataExist = ntbuf_ChkSndBuf(z_NtSndBuf, &z_NtSndFreeBuf);
//}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j


/*====================================================================================[PRIVATE]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_InitNtBuffer
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�t�@������
 *| param	: buffers - �������Ώۃo�b�t�@
 *|			  bufinfo - �o�b�t�@���������e�[�u��
 *|			  max - �o�b�t�@��
 *|			  clr - 1=��d�ۏ؃f�[�^���N���A(���̂Ƃ���d�ۏ؏����͍s��Ȃ�)
 *|			        0=��d�ۏ؃f�[�^�̓N���A���Ȃ�
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_InitNtBuffer(t_NtBuf *buffers, const t_NtBufInfo *bufinfo, int max, uchar clr)
{
	int i;
	
	for (i = 0; i < max; i++) {
		buffers[i].save			= bufinfo[i].save;
		buffers[i].buffer		= bufinfo[i].pool;
		buffers[i].buffer_size	= bufinfo[i].size;
		buffers[i].setting		= bufinfo[i].setting;
		buffers[i].datasize		= bufinfo[i].datasize + 3 + sizeof(t_NtBufDataHeader);	/* "+3"=ID3,ID4,�f�[�^�ێ��t���O */
		buffers[i].id			= bufinfo[i].id;
		if (clr) {
			buffers[i].fukuden.kind	= NTBUF_FUKUDEN_CLR;
			buffers[i].wofs = 0;
			buffers[i].rofs = 0;
			buffers[i].fukuden.kind	= NTBUF_FUKUDEN_NONE;
		}
		else {
			/* ���d���� */
			switch (buffers[i].fukuden.kind) {
			case NTBUF_FUKUDEN_CLR:
				buffers[i].wofs = 0;
				buffers[i].rofs = 0;
				buffers[i].fukuden.kind	= NTBUF_FUKUDEN_NONE;
				break;
			case NTBUF_FUKUDEN_DATAMOVE:
				ntbuf_MoveNtData(&buffers[i]);
				buffers[i].fukuden.kind = NTBUF_FUKUDEN_NONE;
				break;
			case NTBUF_FUKUDEN_NONE:
				/* no break */
			default:
				/* do nothing */
				break;
			}
			/* �o�b�t�@�č\�� */
			switch (buffers[i].save) {
			case NTBUF_SAVE:
				/* do nothing */
				break;
			case NTBUF_SAVE_BY_DATA:
				ntbuf_RearrangeNtBuffer(&buffers[i]);
				break;
			case NTBUF_NOT_SAVE:
				/* no break */
			default:
				/* ���̃o�b�t�@�͖���N���A�����̂Œ�d�ۏؕs�v */
				buffers[i].wofs = 0;
				buffers[i].rofs = 0;
				break;
			}
		}
		/* �o�b�t�@��ԃf�[�^�쐬 */
		ntbuf_ChkBufState(&buffers[i]);
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_RearrangeNtBuffer
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�t�@�č\��
 *|			  �f�[�^�ێ��t���O��OFF�̃f�[�^���o�b�t�@����폜����
 *| param	: buffer - �f�[�^�폜�Ώۃo�b�t�@
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_RearrangeNtBuffer(t_NtBuf *buffer)
{
	long rofs, rofs_next;
	
	rofs_next = buffer->rofs;
	
	while (rofs_next != buffer->wofs) {
		/* �f�[�^�w�b�_�ǂݍ��� */
		rofs = rofs_next;
		ntbuf_ReadNtBuf(buffer, &z_NtBufNtDataHeaderWork, rofs, sizeof(z_NtBufNtDataHeaderWork));
		rofs_next = _offset(rofs, _MAKEWORD(z_NtBufNtDataHeaderWork.bufdata_header.len), buffer->buffer_size);
		if (z_NtBufNtDataHeaderWork.ntdata_header.data_save == NTNET_NOT_BUFFERING) {
		/* �f�[�^�ێ��t���O=OFF �Ȃ̂ł��̃f�[�^���̂Ă� */
			ntbuf_DiscardNtData(buffer, rofs, _MAKEWORD(z_NtBufNtDataHeaderWork.bufdata_header.len));
			rofs_next = buffer->rofs;	/* �ŏ�����Č��� */
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_DiscardNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: �w�肳�ꂽNTNET�f�[�^���o�b�t�@����̂Ă�(�̂Ă��f�[�^�����A���̃f�[�^���l�߂�)
 *| param	: buffer - �f�[�^�폜�Ώۃo�b�t�@
 *|			  delete_data_ofs - �폜�Ώۃf�[�^�擪�̃o�b�t�@���I�t�Z�b�g
 *|			  delete_data_len - �폜�Ώۃf�[�^�̃o�C�g��
 *| return	: none
 *[]----------------------------------------------------------------------[]
 *| remark	:
 *|			�ǂݍ��݃I�t�Z�b�g								�������݃I�t�Z�b�g
 *|				  ��												��
 *|		������������������������������������������������������������������������
 *|		�� (��) ���@�@�@�@�`�@�@�@�@�@�b�@�@�@�@�a�@�@�@�@���@�b�@�� (��) ��
 *|		������������������������������������������������������������������������
 *|			  						�b
 *|			  						�b�f�[�^�ێ��t���O���A�`�C�b���n�m�A�a���n�e�e�̏ꍇ
 *|			  						�b�a�̃f�[�^���폜���āA���̕��`���l�߂�
 *|			  						��
 *|		������������������������������������������������������������������������
 *|		���@�@�@�@�@�@(��)�@�@�@�@�@���@�@�@�@�`�@�@�@�@�@���@�b�@�� (��) ��
 *|		������������������������������������������������������������������������
 *|									  ��							��
 *|							�ǂݍ��݃I�t�Z�b�g				�������݃I�t�Z�b�g
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_DiscardNtData(t_NtBuf *buffer, long delete_data_ofs, ushort delete_data_len)
{
	ushort	len;
	long	count, ofs;
	
	if (delete_data_ofs != buffer->wofs) {
		/* �폜�p�f�[�^���� */
		buffer->fukuden.datamove.move	= delete_data_len;	/* �폜����f�[�^�̃T�C�Y */
		buffer->fukuden.datamove.rofs	= buffer->rofs;		/* �ړ��ΏۂƂ���f�[�^�̐擪 */
		count = 0;
		ofs = buffer->rofs;
		while (ofs != delete_data_ofs) {
			ntbuf_ReadNtBuf(buffer, &len, ofs, sizeof(len));
			ofs = _offset(ofs, len, buffer->buffer_size);
			count += len;
		}
		buffer->fukuden.datamove.count	= count;			/* �ړ��ΏۂƂȂ�f�[�^�̃T�C�Y */
		
		/* �f�[�^�̍폜 */
		buffer->fukuden.kind = NTBUF_FUKUDEN_DATAMOVE;	/* ���d�����̃`�F�b�N�|�C���g */
		ntbuf_MoveNtData(buffer);
		buffer->fukuden.kind = NTBUF_FUKUDEN_NONE;		/* ��d�ۏؑΏۃG���A�I�� */
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_MoveNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: ���o�b�t�@���ł̃f�[�^�ړ�
 *| param	: buffer - �f�[�^�ړ��Ώۃo�b�t�@
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_MoveNtData(t_NtBuf *buffer)
{
	long	src_ofs, dst_ofs;
	
	while (buffer->fukuden.datamove.count > 0) {
		src_ofs = _offset(buffer->fukuden.datamove.rofs, buffer->fukuden.datamove.count-1, buffer->buffer_size);
		dst_ofs = _offset(src_ofs, buffer->fukuden.datamove.move, buffer->buffer_size);
		buffer->buffer[dst_ofs] = buffer->buffer[src_ofs];
		buffer->fukuden.datamove.count--;
		WACDOG;		/* �O�̂��� */
	}
	
	buffer->rofs = _offset(buffer->fukuden.datamove.rofs, buffer->fukuden.datamove.move, buffer->buffer_size);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_ChkRcvBuf
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET�f�[�^��M�o�b�t�@���`�F�b�N���A���������f�[�^������
 *|			  IBK_NTNET_DAT_REC���b�Z�[�W��OPE�֑��M����
 *| param	: buffers - NT-NET�f�[�^��M�o�b�t�@
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_ChkRcvBuf(t_NtBuf *buffers)
{
	ushort	i, len;
	long	rofs;
	
	for (i = 0; i < NTNET_RCVBUF_MAX; i++) {
		rofs = buffers[i].rofs;
		while (rofs != buffers[i].wofs) {
			ntbuf_ReadNtBuf(&buffers[i], &len, rofs, sizeof(len));
			rofs = _offset(rofs, len, buffers[i].buffer_size);
			queset(OPETCBNO, (ushort)IBK_NTNET_DAT_REC, 0, NULL);	/* OPE��NTNET�f�[�^�L���ʒm */
		}
	}
}

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_GetBufCount
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET�f�[�^�o�b�t�@���̎c�f�[�^�����Z�o����
 *| param	: buffer - NT-NET�f�[�^�o�b�t�@
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static	ulong	ntbuf_GetBufCount(t_NtBuf *buffer)
{
	ushort	len;
	long	rofs;
	ulong	count = 0;
	
	rofs = buffer->rofs;
	while (rofs != buffer->wofs) {
		ntbuf_ReadNtBuf(buffer, &len, rofs, sizeof(len));
		rofs = _offset(rofs, len, buffer->buffer_size);
		count++;
		WACDOG;		/* �O�̂��� */
	}

	return count;
}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

/*[]----------------------------------------------------------------------[]
 *|	name	: NTBUF_GetBufCount
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�t�@���̎c�f�[�^������Ԃ�
 *| param	: buf - �c�f�[�^�����f�[�^						<OUT>
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
 *| 		  isRemote - TRUE : ���u FALSE : NT-NET
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//void	NTBUF_GetBufCount(t_NtBufCount *buf)
//{
//	buf->sndbuf_prior		= 	0UL;
//	buf->sndbuf_sale		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_PAYMENT, eLOG_TARGET_REMOTE);
//	if(prm_get(COM_PRM, S_NTN, 121, 1, 1) == 0) {
//		buf->sndbuf_ttotal		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_TTOTAL, eLOG_TARGET_REMOTE);
//	}
//	else {
//		// �Z���^�[�Ή�Phase2�ȍ~��T���v��GT���v�̍��v�Ƃ���
//		buf->sndbuf_ttotal		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_TTOTAL, eLOG_TARGET_REMOTE);
//		buf->sndbuf_ttotal		+= 	(ulong)Ope_Log_UnreadCountGet(eLOG_GTTOTAL, eLOG_TARGET_REMOTE);
//	}
//	buf->sndbuf_coin		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_COINBOX, eLOG_TARGET_REMOTE);
//	buf->sndbuf_note		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_NOTEBOX, eLOG_TARGET_REMOTE);
//	buf->sndbuf_error		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_ERROR, eLOG_TARGET_REMOTE);
//	buf->sndbuf_alarm		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_ALARM, eLOG_TARGET_REMOTE);
//	buf->sndbuf_money		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);
//	buf->sndbuf_turi		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_MNYMNG_SRAM, eLOG_TARGET_REMOTE);
//	buf->sndbuf_monitor		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_MONITOR, eLOG_TARGET_REMOTE);
//	buf->sndbuf_ope_monitor	= 	(ulong)Ope_Log_UnreadCountGet(eLOG_OPERATE, eLOG_TARGET_REMOTE);
//	buf->sndbuf_normal		= 	0UL;
//// MH810100(S) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
////	buf->sndbuf_incar		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_ENTER, eLOG_TARGET_REMOTE);
//	buf->sndbuf_incar		= 	0UL;
//// MH810100(E) K.Onodera 2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//	buf->sndbuf_outcar		= 	0UL;
//	buf->sndbuf_car_count	= 	(ulong)Ope_Log_UnreadCountGet(eLOG_PARKING, eLOG_TARGET_REMOTE);
//	buf->sndbuf_area_count	= 	0UL;
//	buf->sndbuf_rmon		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_REMOTE_MONITOR, eLOG_TARGET_REMOTE);
//// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//	buf->sndbuf_lpark		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_LONGPARK_PWEB, eLOG_TARGET_REMOTE);
//// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//	buf->rcvbuf_broadcast	= 	0UL;
//	buf->rcvbuf_prior		= 	0UL;
//	buf->rcvbuf_normal		= 	0UL;
//	return ;
//}
void	NTBUF_GetBufCount(t_NtBufCount *buf, BOOL isRemote)
{
	ushort	logTarget;									// �Ώۂ̃��O�^�[�Q�b�g
	ushort	index;
	
	logTarget = isRemote == TRUE ? eLOG_TARGET_REMOTE : eLOG_TARGET_NTNET;
	buf->sndbuf_sale		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_PAYMENT, logTarget);
// MH341110(S) A.Iiizumi 2017/12/28 ����`�F�b�N-NT-NET�`�F�b�N�̃f�[�^�N���A������GT���v�̖����M�����𐳂����擾�ł��Ȃ��s��C�� (���ʉ��P��1396)
//	if(FALSE == isRemote) {								// NT-NET
//		// NT-NET��T���v�݂̂̌���
//		buf->sndbuf_ttotal		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_TTOTAL, logTarget);
//	}
//	else {
// MH341110(E) A.Iiizumi 2017/12/28 ����`�F�b�N-NT-NET�`�F�b�N�̃f�[�^�N���A������GT���v�̖����M�����𐳂����擾�ł��Ȃ��s��C�� (���ʉ��P��1396)
		if(prm_get(COM_PRM, S_NTN, 121, 1, 1) == 0) {
			buf->sndbuf_ttotal		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_TTOTAL, logTarget);
		}
		else {
			// �Z���^�[�Ή�Phase2�ȍ~��T���v��GT���v�̍��v�Ƃ���
			buf->sndbuf_ttotal		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_TTOTAL, logTarget);
			buf->sndbuf_ttotal		+= 	(ulong)Ope_Log_UnreadCountGet(eLOG_GTTOTAL, logTarget);
		}
// MH341110(S) A.Iiizumi 2017/12/28 ����`�F�b�N-NT-NET�`�F�b�N�̃f�[�^�N���A������GT���v�̖����M�����𐳂����擾�ł��Ȃ��s��C�� (���ʉ��P��1396)
//	}
// MH341110(E) A.Iiizumi 2017/12/28 ����`�F�b�N-NT-NET�`�F�b�N�̃f�[�^�N���A������GT���v�̖����M�����𐳂����擾�ł��Ȃ��s��C�� (���ʉ��P��1396)
	buf->sndbuf_coin		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_COINBOX, logTarget);
	buf->sndbuf_note		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_NOTEBOX, logTarget);
	buf->sndbuf_error		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_ERROR, logTarget);
	buf->sndbuf_alarm		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_ALARM, logTarget);
	buf->sndbuf_money		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, logTarget);
	buf->sndbuf_turi		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_MNYMNG_SRAM, logTarget);
	buf->sndbuf_monitor		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_MONITOR, logTarget);
	buf->sndbuf_ope_monitor	= 	(ulong)Ope_Log_UnreadCountGet(eLOG_OPERATE, logTarget);
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���g�p���O�i���Ɂj�j
//	buf->sndbuf_incar		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_ENTER, logTarget);
	buf->sndbuf_incar		= 	0UL;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���g�p���O�i���Ɂj�j
// MH364300 GG119A34(S) ���P�A���\No.83�Ή�
//	buf->sndbuf_outcar		= 	0UL;
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���g�p���O�i�o�Ɂj�j
//	buf->sndbuf_outcar		= 	IS_SEND_OUTCAR_DATA ? (ulong)Ope_Log_UnreadCountGet(eLOG_LEAVE, logTarget) : 0UL;
	buf->sndbuf_outcar		= 	0UL;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���g�p���O�i�o�Ɂj�j
// MH364300 GG119A34(E) ���P�A���\No.83�Ή�
	buf->sndbuf_car_count	= 	(ulong)Ope_Log_UnreadCountGet(eLOG_PARKING, logTarget);
	buf->sndbuf_area_count	= 	0UL;
	buf->sndbuf_rmon		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_REMOTE_MONITOR, logTarget);
// MH364300 GG119A08(S) // MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	buf->sndbuf_lpark		= 	(ulong)Ope_Log_UnreadCountGet(eLOG_LONGPARK_PWEB, eLOG_TARGET_REMOTE);
// MH364300 GG119A08(E) // MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)

	if(FALSE == isRemote) {								// NT-NET�̓o�b�t�@�̃f�[�^�����擾����
		buf->sndbuf_prior		= 	ntbuf_GetBufCount(&z_NtSndBuf[NTNET_SNDBUF_PRIOR]);		// ���M�D��f�[�^�o�b�t�@
		buf->sndbuf_normal		= 	ntbuf_GetBufCount(&z_NtSndBuf[NTNET_SNDBUF_NORMAL]);	// ���M�ʏ�f�[�^�o�b�t�@
		buf->rcvbuf_broadcast	= 	ntbuf_GetBufCount(&z_NtRcvBuf[NTNET_RCVBUF_BROADCAST]);	// ��M����f�[�^�o�b�t�@
		buf->rcvbuf_prior		= 	ntbuf_GetBufCount(&z_NtRcvBuf[NTNET_RCVBUF_PRIOR]);		// ��M�D��f�[�^�o�b�t�@
		buf->rcvbuf_normal		= 	ntbuf_GetBufCount(&z_NtRcvBuf[NTNET_RCVBUF_NORMAL]);	// ��M�ʏ�f�[�^�o�b�t�@
		
		// ���M����o�b�t�@(NTNET)�ɂ���f�[�^�����Z����
		for(index = 0; index < NTNET_SNDBUF_MAX; ++index) {
			if(NTNET_SendCtrl[index].length != 0 && 		// ���M����f�[�^�ɑ��M�f�[�^����
			   NTNET_SendCtrl[index].logid == eLOG_MAX) {	// ���M�f�[�^�̓��O�ȊO(�D��A�ʏ�)
				// ���M����f�[�^�ɑ��M�f�[�^������ꍇ�͌����ɉ��Z����
				if(index == NTNET_SNDBUF_PRIOR) {		// �D��f�[�^
					++buf->sndbuf_prior;
				}
				else {									// �ʏ�f�[�^
					++buf->sndbuf_normal;
				}
			}
		}
		// ���M�o�b�t�@(NTCOM)�ɂ���D��f�[�^�����Z����
		switch(NTCom_GetSendPriorDataID()) {			// �D�摗�M�f�[�^
		case 0:											// �f�[�^�Ȃ�
			// �������Ȃ�
			break;
		case 22:										// ���Z(���O)
		case 23:										// ���Z(�o��)
// MH364300 GG119A11(S) // GM569000 (S) ParkingWeb�[���ԁi�t�F�[�Y�Q�j�ʐM�Ή�
		case 56:										// ���Z(���O)�i�[���ԃf�[�^�j
		case 57:										// ���Z(�o��)�i�[���ԃf�[�^�j
// MH364300 GG119A11(E) // GM569000 (E) ParkingWeb�[���ԁi�t�F�[�Y�Q�j�ʐM�Ή�
			++buf->sndbuf_sale;
			break;
		default:										// ���O�f�[�^�ȊO�̗D��f�[�^
			++buf->sndbuf_prior;
			break;
		}

		// ���M�o�b�t�@(NTCOM)�ɂ���ʏ�f�[�^�����Z����
		switch(NTCom_GetSendNormalDataID()) {			// �ʏ푗�M�f�[�^
		case 0:											// �f�[�^�Ȃ�
			// �������Ȃ�
			break;
		case 20:										// ����
// MH364300 GG119A11(S) // GM569000 (S) ParkingWeb�[���ԁi�t�F�[�Y�Q�j�ʐM�Ή�
		case 54:										// ���Ɂi�[���ԃf�[�^�j
// MH364300 GG119A11(E) // GM569000 (E) ParkingWeb�[���ԁi�t�F�[�Y�Q�j�ʐM�Ή�
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���g�p���O�i���Ɂj�j
//			++buf->sndbuf_incar;
			// �������Ȃ�
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���g�p���O�i���Ɂj�j
			break;
// MH364300 GG119A34(S) ���P�A���\No.83�Ή�
		case 21:										// �o��
		case 55:										// �o�Ɂi�[���ԃf�[�^�j
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���g�p���O�i�o�Ɂj�j
//			if (IS_SEND_OUTCAR_DATA) {
//				++buf->sndbuf_outcar;
//			}
			// �������Ȃ�
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���g�p���O�i�o�Ɂj�j
			break;
// MH364300 GG119A34(E) ���P�A���\No.83�Ή�
		case 41:										// �W�v�I���ʒm
// MH341110(S) A.Iiizumi 2017/12/28 ����`�F�b�N-NT-NET�`�F�b�N�̃f�[�^�N���A������GT���v�̖����M�����𐳂����擾�ł��Ȃ��s��C�� (���ʉ��P��1396)
		case 53:										// �W�v�I���ʒm �Z���^�[�Ή�Phase2�ȍ~
// MH341110(E) A.Iiizumi 2017/12/28 ����`�F�b�N-NT-NET�`�F�b�N�̃f�[�^�N���A������GT���v�̖����M�����𐳂����擾�ł��Ȃ��s��C�� (���ʉ��P��1396)
			++buf->sndbuf_ttotal;
			break;
		case 120:										// �G���[
// MH364300 GG119A11(S) // GM569000 (S) ParkingWeb�[���ԁi�t�F�[�Y�Q�j�ʐM�Ή�
		case 63:										// �G���[�i�[���ԃf�[�^�j
// MH364300 GG119A11(E) // GM569000 (E) ParkingWeb�[���ԁi�t�F�[�Y�Q�j�ʐM�Ή�
			++buf->sndbuf_error;
			break;
		case 121:										// �A���[��
// MH364300 GG119A11(S) // GM569000 (S) ParkingWeb�[���ԁi�t�F�[�Y�Q�j�ʐM�Ή�
		case 64:										// �A���[���i�[���ԃf�[�^�j
// MH364300 GG119A11(E) // GM569000 (E) ParkingWeb�[���ԁi�t�F�[�Y�Q�j�ʐM�Ή�
			++buf->sndbuf_alarm;
			break;
		case 122:										// ���j�^
			++buf->sndbuf_monitor;
			break;
		case 123:										// ���샂�j�^
			++buf->sndbuf_ope_monitor;
			break;
		case 126:										// ���K�Ǘ�(SRAM)
			++buf->sndbuf_money;
			break;
		case 131:										// �R�C�����ɏW�v
			++buf->sndbuf_coin;
			break;
		case 133:										// �������ɏW�v
			++buf->sndbuf_note;
			break;
		case 236:										// ���ԑ䐔
			++buf->sndbuf_car_count;
			break;
// MH364300 GG119A11(S) // GM569000 (S) ParkingWeb�[���ԁi�t�F�[�Y�Q�j�ʐM�Ή�
		case 135:										// �ޑK�Ǘ��W�v�f�[�^(SRAM)
			++buf->sndbuf_turi;
			break;
// MH364300 GG119A11(E) // GM569000 (E) ParkingWeb�[���ԁi�t�F�[�Y�Q�j�ʐM�Ή�
		default:										// ���O�f�[�^�ȊO�̒ʏ�f�[�^
			++buf->rcvbuf_normal;
			break;
		}
	}
	else {												// NT-NET�ȊO�͑ΏۊO
		buf->sndbuf_prior		= 	0UL;
		buf->sndbuf_normal		= 	0UL;
		buf->rcvbuf_broadcast	= 	0UL;
		buf->rcvbuf_prior		= 	0UL;
		buf->rcvbuf_normal		= 	0UL;
	}
	return ;
}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_ChkSndBuf
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET�f�[�^���M�o�b�t�@��̃f�[�^�L�����`�F�b�N
 *| param	: buffers - NT-NET�f�[�^���M�o�b�t�@
 *| return	: TRUE - �f�[�^����
 *[]----------------------------------------------------------------------[]*/
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//BOOL	ntbuf_ChkSndBuf(t_NtBuf *buffers, t_NtFreeBuf *freebuf)
BOOL	ntbuf_ChkSndBuf(t_NtBuf *buffers)
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
{
	ushort	i;
	
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
//	if (freebuf->isdata) {
//		return TRUE;
//	}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j

	for (i = 0; i < NTNET_SNDBUF_MAX; i++) {
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//		if ((_ntbuf_IsNtData(&buffers[i])) &&
//			((z_NearFullBitEx[i] & z_NearFullBuff) == 0) &&
//			((z_NearFullBitEx[i] & z_PendingBuff) == 0)) {
		if (_ntbuf_IsNtData(&buffers[i])) {
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
			return TRUE;
		}
	}
	
	return FALSE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_SetNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: �f�[�^���o�b�t�@�ɏ�����
 *| param	: buffer - �f�[�^���Z�b�g����o�b�t�@
 *|			  data - �������ݑΏۂƂȂ�f�[�^					<IN>
 *|			  len - data�̃o�C�g��
 *|	return	: NTNET_BUFSET_NORMAL=����I��
 *|			  NTNET_BUFSET_DEL_OLD=�ŌÃf�[�^������
 *|			  NTNET_BUFSET_DEL_NEW=�ŐV�f�[�^������
 *|			  NTNET_BUFSET_CANT_DEL=�o�b�t�@FULL�����ݒ肪"�x��"�̂��ߏ����s��
 *[]----------------------------------------------------------------------[]*/
uchar	ntbuf_SetNtData(t_NtBuf *buffer, const void *data, ushort len)
{
	ushort	write_len, len_0cut;
	uchar	ret;
	
	ret = NTNET_BUFSET_NORMAL;
	
	len_0cut = ntbuf_GetNtData0CutLen(data, len);
	write_len = (ushort)(len_0cut + sizeof(t_NtBufDataHeader));
	/* �o�b�t�@�󂫗e�ʂ̃`�F�b�N */
	if (ntbuf_GetNtBufRemain(buffer) < write_len) {
		/* �o�b�t�@FULL���̐ݒ�擾 */
		ret = (uchar)buffer->setting();
		switch (ret) {
		case NTNET_BUFSET_DEL_NEW:		/* �V���� */
			ntbuf_DeleteLatestNtData(buffer, (long)write_len);
			break;
		case NTNET_BUFSET_CANT_DEL:		/* �x�� */
			/* do nothing */
			break;
		case NTNET_BUFSET_DEL_OLD:		/* ������ */
			/* no break */
		default:
			ret = NTNET_BUFSET_DEL_OLD;
			ntbuf_DeleteOldestNtData(buffer, (long)write_len);
			break;
		}
	}
	
	if (ret != NTNET_BUFSET_CANT_DEL) {
	/* �o�b�t�@������ */
		ntbuf_WriteNtData(buffer, data, len, len_0cut);
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_GetNtData0CutLen
 *[]----------------------------------------------------------------------[]
 *| summary	: 0�J�b�g��̃f�[�^�T�C�Y�擾
 *| param	: data - �T�C�Y�Z�o�Ώۃf�[�^
 *|			  len - �f�[�^��
 *|	return	: 0�J�b�g��̃f�[�^�T�C�Y
 *[]----------------------------------------------------------------------[]*/
ushort	ntbuf_GetNtData0CutLen(const uchar *data, ushort len)
{
	while (len > 0) {
		if (data[len-1] != 0) {
			break;
		}
		len--;
	}
	
	return len;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_DeleteOldestNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: write_len���̃o�b�t�@�󂫗̈���m�ۂ���(�f�[�^�폜�͍ŌÃf�[�^���珇�ɍs��)
 *| param	: buffer - �f�[�^���폜����o�b�t�@
 *|			  len - �K�v�ȋ󂫃T�C�Y
 *|	return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_DeleteOldestNtData(t_NtBuf *buffer, long write_len)
{
	ushort len;
	
	while (ntbuf_GetNtBufRemain(buffer) < write_len) {
		ntbuf_ReadNtBuf(buffer, &len, buffer->rofs, sizeof(len));
		buffer->rofs = _offset(buffer->rofs, len, buffer->buffer_size);
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_DeleteLatestNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: write_len���̃o�b�t�@�󂫗̈���m�ۂ���(�f�[�^�폜�͍ŐV�f�[�^���珇�ɍs��)
 *| param	: buffer - �f�[�^���폜����o�b�t�@
 *|			  len - �K�v�ȋ󂫃T�C�Y
 *|	return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_DeleteLatestNtData(t_NtBuf *buffer, long write_len)
{
	ushort	len;
	long	ofs;
	
	while (ntbuf_GetNtBufRemain(buffer) < write_len) {
		/* �ŐV�f�[�^������ */
		ofs = buffer->rofs;
		len = 0;
		while (_offset(ofs, len, buffer->buffer_size) != buffer->wofs) {
			ofs = _offset(ofs, len, buffer->buffer_size);
			ntbuf_ReadNtBuf(buffer, &len, ofs, sizeof(len));
		}
		buffer->wofs = ofs;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_WriteNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: �f�[�^������
 *| param	: buffer - �f�[�^���Z�b�g����o�b�t�@
 *|			  data - �������ݑΏۂƂȂ�f�[�^					<IN>
 *|			  len - data�̃o�C�g��
 *|			  len_0cut - 0�J�b�g��̃f�[�^�T�C�Y
 *|	return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_WriteNtData(t_NtBuf *buffer, const void *data, ushort len_before0cut, ushort len_after0cut)
{
	t_NtBufDataHeader header;
	long	ofs;
	
	ofs = buffer->wofs;

	/* �f�[�^�w�b�_������ */
	ntbuf_W2Bcpy(header.len, (ushort)(len_after0cut + sizeof(t_NtBufDataHeader)));	/* �o�b�t�@�f�[�^�� */
	ntbuf_W2Bcpy(header.len_before0cut, len_before0cut);							/* 0�J�b�g�O�̃f�[�^�� */
	header.lower_terminal1	= 0;													/* ���ʓ]���p�[��No.(1) */
	header.lower_terminal2	= 0;													/* ���ʓ]���p�[��No.(2) */
	header.reserve			= 0;													/* �\�� */
	ofs = ntbuf_WriteNtBuf(buffer, ofs, &header, sizeof(header));
	/* �f�[�^������ */
	ofs = ntbuf_WriteNtBuf(buffer, ofs, data, (ulong)len_after0cut);

	/* �������݃I�t�Z�b�g�X�V */
	buffer->wofs = ofs;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_GetNtData
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET�f�[�^�ǂݍ���
 *| param	: buffer - �f�[�^�̓ǂݍ��݌��o�b�t�@
 *|			  data - �f�[�^�R�s�[��̈�						<OUT>
 *|			  f_Expand - 1=0�J�b�g����L�����ĕԂ��C0=0�J�b�g�̂܂ܕԂ�
 *|	return	: �擾�����f�[�^�̃f�[�^��
 *|			  0���f�[�^����
 *[]----------------------------------------------------------------------[]*/
ushort	ntbuf_GetNtData(t_NtBuf *buffer, void *data, uchar f_Expand)
{
	t_NtBufDataHeader header;
	ushort	ret, len, len_before0cut;
	long	ofs;
	
	ret = 0;
	if (_ntbuf_IsNtData(buffer)) {
		/* �f�[�^�w�b�_����f�[�^�����擾 */
		ofs = ntbuf_ReadNtBuf(buffer, &header, buffer->rofs, sizeof(t_NtBufDataHeader));
		len = _MAKEWORD(header.len);
		len -= sizeof(t_NtBufDataHeader);
		/* �f�[�^�ǂݍ��� */
		ntbuf_ReadNtBuf(buffer, data, ofs, (ulong)len);
		/* 0�J�b�g���̐L�� */
		if( f_Expand ){
			len_before0cut = _MAKEWORD(header.len_before0cut);
			while (len < len_before0cut) {
				((uchar*)data)[len++] = 0;
			}
		}
		ret = len;
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_NtDataClr
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�t�@����ŌÃf�[�^��1���폜
 *| param	: buffer - �f�[�^�̍폜�Ώۃo�b�t�@
 *|	return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_NtDataClr(t_NtBuf *buffer)
{
	ushort len;
	
	if (_ntbuf_IsNtData(buffer)) {
		ntbuf_ReadNtBuf(buffer, &len, buffer->rofs, sizeof(len));
		buffer->rofs = _offset(buffer->rofs, len, buffer->buffer_size);
	}
}

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
///*[]----------------------------------------------------------------------[]
// *|	name	: ntbuf_SetFreeData
// *[]----------------------------------------------------------------------[]
// *| summary	: FREE�f�[�^���o�b�t�@�ɏ�������
// *| param	: buffer - �f�[�^���Z�b�g����o�b�t�@
// *|			  data - �������ݑΏۂƂȂ�f�[�^					<IN>
// *| return	: 1������I��
// *|			  0���o�b�t�@�I�[�o�[���C�g����
// *[]----------------------------------------------------------------------[]*/
//uchar	ntbuf_SetFreeData(t_NtFreeBuf *buffer, const void *data)
//{
//	uchar ret;
//	
//	if (buffer->isdata) {
//		ret = 0;	/* overwrite! */
//	}
//	else {
//		ret = 1;
//	}
//	
//	memcpy(buffer->buffer, data, sizeof(buffer->buffer));
//	buffer->isdata = TRUE;	/* �f�[�^�L�� */
//	
//	return ret;
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: ntbuf_GetFreeData
// *[]----------------------------------------------------------------------[]
// *| summary	: FREE�f�[�^���R�s�[���A�o�b�t�@����폜
// *| param	: buffer - �f�[�^�̓ǂݍ��݌�����o�b�t�@
// *|			  data - �R�s�[��̈�					<OUT>
// *| return	: 1������I��
// *|			  0���f�[�^����
// *[]----------------------------------------------------------------------[]*/
//uchar	ntbuf_GetFreeData(t_NtFreeBuf *buffer, void *data)
//{
//	uchar ret;
//	
//	if (!buffer->isdata) {
//		ret = 0;	/* �f�[�^���� */
//	}
//	else {
//		ret = 1;
//		memcpy(data, buffer->buffer, sizeof(buffer->buffer));
//		buffer->isdata = FALSE;		/* �f�[�^�폜 */
//	}
//	
//	return ret;
//}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_GetSndNtBufCode
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET�f�[�^���M�o�b�t�@�R�[�h�̎擾
 *| param	: data - �������ݑΏۂƂȂ�f�[�^					<IN>
 *|			  buffer - �o�b�t�@���
 *|					   (NTNET_BUF_BUFFERING/NTNET_BUF_PRIOR/NTNET_BUF_NORMAL)
 *| return	: ����data�ɃR�s�[�����o�C�g��
 *|			  0���w��o�b�t�@�Ƀf�[�^�Ȃ�
 *[]----------------------------------------------------------------------[]*/
int	ntbuf_GetSndNtBufCode(const void *data, eNTNET_BUF_KIND buffer)
{
	int		ret;
	
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
//	switch (((t_NtNetDataHeader*)data)->data_kind) {
//	case 22:	/* no break */
//	case 23:
//	case 56:
//	case 57:
//	/* ���Z�f�[�^�e�[�u�� */
//		ret = NTNET_SNDBUF_SALE;
//		break;
//	case 21:
//	/* �o�Ƀf�[�^�e�[�u�� */
//		ret = NTNET_SNDBUF_CAR_OUT;
//		break;
//	case 20:
//	case 54:
//	/* ���Ƀf�[�^�e�[�u�� */
//		ret = NTNET_SNDBUF_CAR_IN;
//		break;
//	case 30:	/* no break */
//	case 31:	/* no break */
//	case 32:	/* no break */
//	case 33:	/* no break */
//	case 34:	/* no break */
//	case 35:	/* no break */
//	case 36:	/* no break */
//	case 37:	/* no break */
//	case 38:	/* no break */
//	case 41:	/* no break */
//	/* T���v�W�v�f�[�^ */
//		ret = NTNET_SNDBUF_TTOTAL;
//		break;
//	case 131:
//	/* �R�C�����ɏW�v���v�f�[�^ */
//		ret = NTNET_SNDBUF_COIN;
//		break;
//	case 133:
//	/* �������ɏW�v���v�f�[�^ */
//		ret = NTNET_SNDBUF_NOTE;
//		break;
//	case 120:
//	/* �G���[�f�[�^ */
//		ret = NTNET_SNDBUF_ERROR;
//		break;
//	case 121:
//	/* �A���[���f�[�^ */
//		ret = NTNET_SNDBUF_ALARM;
//		break;
//	case 126:
//	/* ���K�Ǘ��f�[�^ */
//		ret = NTNET_SNDBUF_MONEY;
//		break;
//	case 236:
//	/* ���ԑ䐔�f�[�^ */
//		ret = NTNET_SNDBUF_CAR_COUNT;
//		break;
//	case 237:
//	/* ���䐔�E���ԃf�[�^ */
//		ret = NTNET_SNDBUF_AREA_COUNT;
//		break;
//	case 122:
//	/* ���j�^�f�[�^ */
//		ret = NTNET_SNDBUF_MONITOR;
//		break;
//	case 123:
//	/* ���샂�j�^�f�[�^ */
//		ret = NTNET_SNDBUF_OPE_MONITOR;
//		break;
//	default:
//		switch (buffer) {
//		case NTNET_BUF_PRIOR:
//		/* �o�b�t�@�����O�f�[�^�ł͂Ȃ��D��f�[�^ */
//			ret = NTNET_SNDBUF_PRIOR;
//			break;
//		case NTNET_BUF_NORMAL:
//		/* �o�b�t�@�����O�f�[�^�ł͂Ȃ��ʏ�f�[�^ */
//			ret = NTNET_SNDBUF_NORMAL;
//			break;
//		default:
//			ret = -1;	/* ���̃p�X�͒ʂ�Ȃ��͂� */
//			break;
//		}
//	}
	switch (buffer) {
	case NTNET_BUF_PRIOR:
	/* �o�b�t�@�����O�f�[�^�ł͂Ȃ��D��f�[�^ */
		ret = NTNET_SNDBUF_PRIOR;
		break;
	case NTNET_BUF_NORMAL:
		/* �o�b�t�@�����O�f�[�^�ł͂Ȃ��ʏ�f�[�^ */
		ret = NTNET_SNDBUF_NORMAL;
		break;
	default:
		ret = -1;	/* ���̃p�X�͒ʂ�Ȃ��͂� */
		break;
	}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_GetRcvNtBufCode
 *[]----------------------------------------------------------------------[]
 *| summary	: NT-NET�f�[�^��M�o�b�t�@�R�[�h�̎擾
 *| param	: buffer - �o�b�t�@���
 *|					   (NTNET_BUF_BROADCAST/NTNET_BUF_PRIOR/NTNET_BUF_NORMAL)
 *| return	: ����data�ɃR�s�[�����o�C�g��
 *|			  0���w��o�b�t�@�Ƀf�[�^�Ȃ�
 *[]----------------------------------------------------------------------[]*/
int	ntbuf_GetRcvNtBufCode(eNTNET_BUF_KIND buffer)
{
	int ret;
	
	switch (buffer) {
	case NTNET_BUF_BROADCAST:
		ret = NTNET_RCVBUF_BROADCAST;
		break;
	case NTNET_BUF_PRIOR:
		ret = NTNET_RCVBUF_PRIOR;
		break;
	case NTNET_BUF_NORMAL:
		ret = NTNET_RCVBUF_NORMAL;
		break;
	default:
		ret = -1;
		break;
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_GetNtBufRemain
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�t�@�c�e�ʎ擾
 *| param	: buffer - �c�e�ʎ擾�������o�b�t�@
 *| return	: �o�b�t�@�c�e��
 *[]----------------------------------------------------------------------[]*/
long	ntbuf_GetNtBufRemain(t_NtBuf *buffer)
{
	long rofs;
	
	if (buffer->wofs == buffer->rofs) {
		return buffer->buffer_size - 1;
	}
	
	rofs = buffer->rofs;
	if (buffer->wofs > buffer->rofs) {
		rofs += buffer->buffer_size;
	}
		
	return rofs - buffer->wofs - 1;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_ChkSndNtDataLen
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M�f�[�^���`�F�b�N
 *| param	: bufcode - �f�[�^�i�[��o�b�t�@
 *|			  len - �f�[�^��
 *| return	: TRUE - OK
 *[]----------------------------------------------------------------------[]*/
BOOL	ntbuf_ChkSndNtDataLen(int bufcode, ushort len) 
{
	ushort limit;
	
	if (NTNET_SNDBUF_PRIOR_TOP <= bufcode && bufcode <= NTNET_SNDBUF_PRIOR_END) {
		limit = NTNET_BLKDATA_MAX + 3;		/* �D��f�[�^�̍ő�f�[�^��("+3"=ID3,ID4,�f�[�^�ێ��t���O) */
	}
	else {
		limit = NTNET_BLKDATA_MAX * 26 + 3;	/* �ʏ�f�[�^�̍ő�f�[�^��("+3"=ID3,ID4,�f�[�^�ێ��t���O) */
	}
	
	if (len > limit) {
		return FALSE;
	}
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_ChkRcvNtDataLen
 *[]----------------------------------------------------------------------[]
 *| summary	: ��M�f�[�^���`�F�b�N
 *| param	: bufcode - �f�[�^�i�[��o�b�t�@
 *|			  len - �f�[�^��
 *| return	: TRUE - OK
 *[]----------------------------------------------------------------------[]*/
BOOL	ntbuf_ChkRcvNtDataLen(int bufcode, ushort len) 
{
	ushort limit;
	
	switch (bufcode) {
	case NTNET_RCVBUF_NORMAL:
		limit = NTNET_BLKDATA_MAX * 26 + 3;	/* �ʏ�f�[�^�̍ő�f�[�^��("+3"=ID3,ID4,�f�[�^�ێ��t���O) */
		break;
	case NTNET_RCVBUF_BROADCAST:	/* no break */
	case NTNET_RCVBUF_PRIOR:		/* no break */
	default:
		limit = NTNET_BLKDATA_MAX + 3;		/* �D��,����f�[�^�̍ő�f�[�^��("+3"=ID3,ID4,�f�[�^�ێ��t���O) */
		break;
	}
	
	if (len > limit) {
		return FALSE;
	}
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_ReadNtBuf
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�t�@����̃f�[�^�ǂݍ���
 *| param	: buffer - �f�[�^�ǂݍ��ݑΏۃo�b�t�@
 *|			  dst - �f�[�^�Ǎ���̈�							<OUT>
 *|			  top - �ǂݍ��݌��f�[�^�擪
 *|			  cnt - �ǂݍ��݃f�[�^��
 *| return	: �ǂݍ��ݎ��s���top�ʒu
 *[]----------------------------------------------------------------------[]*/
long	ntbuf_ReadNtBuf(t_NtBuf *buffer, void *dst, long top, ulong cnt)
{
	ulong rcnt;
	uchar *cdst = (uchar*)dst;
	
	if (top + cnt >= (ulong)buffer->buffer_size) {
		rcnt = buffer->buffer_size - top;
		memcpy(cdst, &buffer->buffer[top], rcnt);
		cdst += rcnt;
		cnt -= rcnt;
		top = 0;
	}
	memcpy(cdst, &buffer->buffer[top], cnt);
	
	return (long)(top + cnt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_WriteNtBuf
 *[]----------------------------------------------------------------------[]
 *| summary	: �f�[�^������
 *| param	: buffer - �f�[�^���Z�b�g����o�b�t�@
 *|			  dst_ofs - �f�[�^�������݊J�n�I�t�Z�b�g
 *|			  data - �������ݑΏۂƂȂ�f�[�^					<IN>
 *|			  cnt - �f�[�^�������݃T�C�Y
 *|	return	: none
 *[]----------------------------------------------------------------------[]*/
long	ntbuf_WriteNtBuf(t_NtBuf *buffer, long dst_ofs, const void *data, ulong cnt)
{
	const uchar *csrc = (const uchar*)data;
	ulong wcnt;
	
	if (dst_ofs + cnt >= (ulong)buffer->buffer_size) {
		wcnt = buffer->buffer_size - dst_ofs;
		memcpy(&buffer->buffer[dst_ofs], csrc, wcnt);
		dst_ofs	= 0;
		csrc	+= wcnt;
		cnt		-= wcnt;
	}
	
	memcpy(&buffer->buffer[dst_ofs], csrc, cnt);
	
	return (long)(dst_ofs + cnt);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_ChkBufState
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�t�@��ԃ`�F�b�N
 *|			  �o�b�t�@��Ԃɕω�������΃o�b�t�@��ԃf�[�^���X�V����
 *| param	: buffer - �`�F�b�N�Ώۃo�b�t�@
 *| return	: TRUE - �o�b�t�@��ԕω�
 *|			  FALSE - �o�b�t�@��ԕω�����
 *[]----------------------------------------------------------------------[]*/
BOOL	ntbuf_ChkBufState(t_NtBuf *buffer)
{
	long	remain;
	uchar	state;
	
	if (buffer->setting == ntbuf_GetDefaultSetting) {
		return FALSE;	/* ��ɍŌÃf�[�^�ɏ㏑���̃o�b�t�@�Ȃ̂ŏ�ԊĎ��̕K�v�Ȃ� */
	}
	
	state = NTBUF_BUFFER_NORMAL;
	remain = ntbuf_GetNtBufRemain(buffer);
	if (remain < buffer->datasize) {
	/* �o�b�t�@FULL */
		if (buffer->id == 22) {
		/* ���Z�f�[�^�e�[�u���̏ꍇ�A�j�AFULL���������Ƃ��� */
			state = (NTBUF_BUFFER_FULL|NTBUF_BUFFER_NEAR_FULL);
		}
		else {
			state = NTBUF_BUFFER_FULL;
		}
	}
	else if (remain < buffer->datasize * NTBUF_NEARFULL_COUNT) {
		if (buffer->id == 22) {
	/* �j�AFULL(���Z�f�[�^�e�[�u���ł̂ݔ���) */
			state = NTBUF_BUFFER_NEAR_FULL;
		}
	}
	
	/* �o�b�t�@��ԃf�[�^�Z�b�g(�ω�������΃o�b�t�@��ԃf�[�^�X�V) */
	if (ntbuf_SetBufState(buffer, state)) {
		return TRUE;
	}
	
	return FALSE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_SetBufState
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�t�@��ԃf�[�^�Z�b�g
 *| param	: buffer - �Ώۃo�b�t�@
 *|			  state - ���݂̃o�b�t�@���(NTBUF_BUFFER_NORMAL/NTBUF_BUFFER_FULL/NTBUF_BUFFER_NEAR_FULL)
 *| return	: TRUE - �o�b�t�@��ԕω�
 *|			  FALSE - �o�b�t�@��ԕω�����
 *[]----------------------------------------------------------------------[]*/
BOOL	ntbuf_SetBufState(t_NtBuf *buffer, uchar state)
{
	uchar	*dst;
	
	switch (buffer->id) {
	case 22:	/* ���Z�f�[�^�e�[�u�� */
		dst = &z_NtBufState.sale;
		break;
	case 21:	/* �o�Ƀf�[�^�e�[�u�� */
		dst = &z_NtBufState.car_out;
		break;
	case 20:	/* ���Ƀf�[�^�e�[�u�� */
		dst = &z_NtBufState.car_in;
		break;
	case 41:	/* T���v�W�v�f�[�^ */
		dst = &z_NtBufState.ttotal;
		break;
	case 131:	/* �R�C�����ɏW�v���v�f�[�^ */
		dst = &z_NtBufState.coin;
		break;
	case 133:	/* �������ɏW�v���v�f�[�^ */
		dst = &z_NtBufState.note;
		break;
	default:	/* �o�b�t�@�c�ʊĎ��̕K�v�������f�[�^(�o�b�t�@FULL���͏�ɍŌÃf�[�^�ɏ㏑��) */
		dst = NULL;
		break;
	}
	
	if (dst != NULL) {
		if (*dst != state) {
			*dst = state;
			return TRUE;
		}
	}
	
	return FALSE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_GetBufState
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�t�@��ԃf�[�^�擾
 *| param	: buffer - �Ώۃo�b�t�@
 *| return	: �Ώۃo�b�t�@�̃o�b�t�@���
 *[]----------------------------------------------------------------------[]*/
uchar	ntbuf_GetBufState(t_NtBuf *buffer)
{
	switch (buffer->id) {
	case 22:	/* ���Z�f�[�^�e�[�u�� */
		return z_NtBufState.sale;
	case 21:	/* �o�Ƀf�[�^�e�[�u�� */
		return z_NtBufState.car_out;
	case 20:	/* ���Ƀf�[�^�e�[�u�� */
		return z_NtBufState.car_in;
	case 41:	/* T���v�W�v�f�[�^ */
		return z_NtBufState.ttotal;
	case 131:	/* �R�C�����ɏW�v���v�f�[�^ */
		return z_NtBufState.coin;
	case 133:	/* �������ɏW�v���v�f�[�^ */
		return z_NtBufState.note;
	default:	/* �o�b�t�@�c�ʊĎ��̕K�v�������f�[�^ */
		return NTBUF_BUFFER_NORMAL;
	}
}

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
///*[]----------------------------------------------------------------------[]
// *|	name	: ntbuf_GetSaleBufSetting
// *[]----------------------------------------------------------------------[]
// *| summary	: �o�b�t�@FULL���̓���ݒ�擾(���Z�f�[�^�e�[�u��)
// *| param	: none
// *| return	: �o�b�t�@FULL���̓���ݒ�
// *[]----------------------------------------------------------------------[]*/
//long	ntbuf_GetSaleBufSetting(void)
//{
//	return _ntbuf_GetPlaceSetting(CPrmSS[S_NTN][NTPRM_BUFFULL], NTPRM_PLACE_SALE);
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: ntbuf_GetCarInBufSetting
// *[]----------------------------------------------------------------------[]
// *| summary	: �o�b�t�@FULL���̓���ݒ�擾(���Ƀf�[�^�e�[�u��)
// *| param	: none
// *| return	: �o�b�t�@FULL���̓���ݒ�
// *[]----------------------------------------------------------------------[]*/
//long	ntbuf_GetCarInBufSetting(void)
//{
//	return _ntbuf_GetPlaceSetting(CPrmSS[S_NTN][NTPRM_BUFFULL], NTPRM_PLACE_CARIN);
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: ntbuf_GetCarInBufSetting
// *[]----------------------------------------------------------------------[]
// *| summary	: �o�b�t�@FULL���̓���ݒ�擾(�o�Ƀf�[�^�e�[�u��)
// *| param	: none
// *| return	: �o�b�t�@FULL���̓���ݒ�
// *[]----------------------------------------------------------------------[]*/
//long	ntbuf_GetCarOutBufSetting(void)
//{
//	return _ntbuf_GetPlaceSetting(CPrmSS[S_NTN][NTPRM_BUFFULL], NTPRM_PLACE_CAROUT);
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: ntbuf_GetTTotalBufSetting
// *[]----------------------------------------------------------------------[]
// *| summary	: �o�b�t�@FULL���̓���ݒ�擾(T���v�W�v�f�[�^)
// *| param	: none
// *| return	: �o�b�t�@FULL���̓���ݒ�
// *[]----------------------------------------------------------------------[]*/
//long	ntbuf_GetTTotalBufSetting(void)
//{
//	return _ntbuf_GetPlaceSetting(CPrmSS[S_NTN][NTPRM_BUFFULL], NTPRM_PLACE_TTOTAL);
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: ntbuf_GetCoinBufSetting
// *[]----------------------------------------------------------------------[]
// *| summary	: �o�b�t�@FULL���̓���ݒ�擾(�R�C�����ɏW�v���v�f�[�^)
// *| param	: none
// *| return	: �o�b�t�@FULL���̓���ݒ�
// *[]----------------------------------------------------------------------[]*/
//long	ntbuf_GetCoinBufSetting(void)
//{
//	return _ntbuf_GetPlaceSetting(CPrmSS[S_NTN][NTPRM_BUFFULL], NTPRM_PLACE_COIN);
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: ntbuf_GetNoteBufSetting
// *[]----------------------------------------------------------------------[]
// *| summary	: �o�b�t�@FULL���̓���ݒ�擾(�������ɏW�v���v�f�[�^)
// *| param	: none
// *| return	: �o�b�t�@FULL���̓���ݒ�
// *[]----------------------------------------------------------------------[]*/
//long	ntbuf_GetNoteBufSetting(void)
//{
//	return _ntbuf_GetPlaceSetting(CPrmSS[S_NTN][NTPRM_BUFFULL], NTPRM_PLACE_NOTE);
//}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_GetDefaultSetting
 *[]----------------------------------------------------------------------[]
 *| summary	: �o�b�t�@FULL���̓���ݒ�擾(�f�t�H���g)
 *| param	: none
 *| return	: �o�b�t�@FULL���̓���ݒ�
 *[]----------------------------------------------------------------------[]*/
long	ntbuf_GetDefaultSetting(void)
{
	return NTNET_BUFSET_DEL_OLD;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_W2Bcpy
 *[]----------------------------------------------------------------------[]
 *| summary	: WORD�l��BYTE�~2�ɃR�s�[
 *| param	: bdst - �R�s�[��o�C�g�z��
 *|			  wsrc - �R�s�[��WORD�l
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	ntbuf_W2Bcpy(uchar *bdst, ushort wsrc)
{
	*bdst++ = (uchar)(wsrc >> 8);
	*bdst = (uchar)wsrc;
}

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
///*[]----------------------------------------------------------------------[]
// *|	name	: ntbuf_IsSendData
// *[]----------------------------------------------------------------------[]
// *| summary	: ���uNT-NET�Ȃ�e�f�[�^�̑��M�ۂ��`�F�b�N�A���M�s�v�Ȃ�Z�b�g�����ɏI��
// *| param	: ID:�f�[�^ID
// *| return	: 1:OK / 0:NG
// *[]----------------------------------------------------------------------[]*/
//char ntbuf_IsSendData(uchar id)
//{
//	uchar	send[12];
//	char	ret = 1;
//	int		i;
//
//	if (_is_ntnet_remote()) {
//		for (i = 0; i < 6; i++) {
//			send[i] = (uchar)prm_get( COM_PRM,S_NTN,61,1,(char)(6-i) );
//		}
//		for (i = 0; i < 6; i++) {
//			send[i+6] = (uchar)prm_get( COM_PRM,S_NTN,62,1,(char)(6-i) );
//		}
//		switch (id) {
//		case 20:	// ���Ƀf�[�^
//			if (send[0]) ret = 0;
//			break;
//		case 22:	// 
//		case 23:	// ���Z�f�[�^
//			if (send[1]) ret = 0;
//			break;
//		case 120:	// �G���[�f�[�^
//			if (send[3]) ret = 0;
//			break;
//		case 121:	// �A���[���f�[�^
//			if (send[4]) ret = 0;
//			break;
//		case 122:	// ���j�^�f�[�^
//			if (send[5]) ret = 0;
//			break;
//		case 123:	// ���샂�j�^�f�[�^
//			if (send[6]) ret = 0;
//			break;
//		case 131:	// �R�C�����ɏW�v�f�[�^
//			if (send[7]) ret = 0;
//			break;
//		case 133:	// �������ɏW�v�f�[�^
//			if (send[8]) ret = 0;
//			break;
//		case 236:	// ���ԑ䐔�f�[�^
//			if (send[9]) ret = 0;
//			break;
//		case 126:	// ���K�Ǘ��f�[�^
//			if (send[11]) ret = 0;
//			break;
//		default:	// �W�v�f�[�^
//			if ((id >= 30 && id <= 38) || id == 41) {
//				if (send[2]) ret = 0;
//			}
//			break;
//		}
//	}
//	return ret;
//}
//
///*[]----------------------------------------------------------------------[]
// *|	name	: NTBUF_DataClr
// *[]----------------------------------------------------------------------[]
// *| summary	: NT-NET�o�b�t�@���̎w��f�[�^���폜
// *| param	: req  �폜�v���f�[�^ID
// *| return	: none
// *[]----------------------------------------------------------------------[]*/
//static const int buf_order[] = {
//	NTNET_SNDBUF_CAR_IN,			// ���Ƀf�[�^�e�[�u��
//	NTNET_SNDBUF_TTOTAL,			// T���v�W�v�f�[�^
//	NTNET_SNDBUF_SALE,				// ���Z�f�[�^�e�[�u��
//	NTNET_SNDBUF_COIN,				// �R�C�����ɏW�v���v�f�[�^
//	NTNET_SNDBUF_ERROR,				// �G���[�f�[�^
//	NTNET_SNDBUF_NOTE,				// �������ɏW�v���v�f�[�^
//	NTNET_SNDBUF_ALARM,				// �A���[���f�[�^
//	NTNET_SNDBUF_CAR_COUNT,			// ���ԑ䐔�f�[�^
//	NTNET_SNDBUF_MONITOR,			// ���j�^�f�[�^
//	NTNET_SNDBUF_MONEY,				// ���K�Ǘ��f�[�^
//	NTNET_SNDBUF_OPE_MONITOR,		// ���샂�j�^�f�[�^
//};
//
//void	NTBUF_DataClr(int req)
//{
//	int no;
//	
//	if (req == -1) {
//		ntbuf_InitNtBuffer(z_NtSndBuf, z_NtSndBufInfo, NTNET_SNDBUF_MAX, 1);	// NT-NET�f�[�^���M�o�b�t�@������
///
//	}
//	else {
//		no = buf_order[req];
//		z_NtSndBuf[no].save        = z_NtSndBufInfo[no].save;
//		z_NtSndBuf[no].buffer      = z_NtSndBufInfo[no].pool;
//		z_NtSndBuf[no].buffer_size = z_NtSndBufInfo[no].size;
//		z_NtSndBuf[no].setting     = z_NtSndBufInfo[no].setting;
//		z_NtSndBuf[no].datasize    = z_NtSndBufInfo[no].datasize + 3 + sizeof(t_NtBufDataHeader);  // "+3"=ID3,ID4,�f�[�^�ێ��t���O
//		z_NtSndBuf[no].id          = z_NtSndBufInfo[no].id;
//		
//		// ��d�ۏ؃f�[�^���N���A
//		z_NtSndBuf[no].fukuden.kind = NTBUF_FUKUDEN_CLR;
//		z_NtSndBuf[no].wofs = 0;
//		z_NtSndBuf[no].rofs = 0;
//		z_NtSndBuf[no].fukuden.kind = NTBUF_FUKUDEN_NONE;
//
//		// �o�b�t�@��ԃf�[�^�쐬
//		ntbuf_ChkBufState(&z_NtSndBuf[no]);
//	}
//}
/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_InitSendDataCtrl
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M����f�[�^����������
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static	void	ntbuf_InitSendDataCtrl(void)
{
	// ���M����f�[�^�̏�����
	memset(&NTNET_SendCtrl, 0, sizeof(NTNET_SendCtrl));
	NTNET_SendCtrl[0].data = NtSendPriorData;			// �D��NTNET�f�[�^�o�b�t�@
	NTNET_SendCtrl[0].type = NTNET_PRIOR_DATA;			// �D��f�[�^(�ʐM���֓n�����)
	NTNET_SendCtrl[0].logid = eLOG_MAX;					// ���M�Ώۃ��O eLOG_MAX:���O�f�[�^�ȊO
	
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���u�ƒ[���ԒʐM�̃o�b�t�@��ʂɂ���j
//	NTNET_SendCtrl[1].data = RAU_NtLogData;				// �ʏ�NTNET�f�[�^�o�b�t�@
	NTNET_SendCtrl[1].data = NTNET_NtLogData;			// �ʏ�NTNET�f�[�^�o�b�t�@
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i���u�ƒ[���ԒʐM�̃o�b�t�@��ʂɂ���j
	NTNET_SendCtrl[1].type = NTNET_NORMAL_DATA;			// �ʏ�f�[�^(�ʐM���֓n�����)
	NTNET_SendCtrl[1].logid = eLOG_MAX;					// ���M�Ώۃ��O eLOG_MAX:���O�f�[�^�ȊO
}

/*[]----------------------------------------------------------------------[]
 *|	name	: ntbuf_ClearSendDataCtrl
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M����f�[�^�N���A����
 *| param	: pCtrl		: �N���A���鑗�M����f�[�^
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
static	void	ntbuf_ClearSendDataCtrl(t_Ntnet_SendDataCtrl* pCtrl)
{
	if(pCtrl) {
		pCtrl->length = 0;								// ���M�f�[�^��
		pCtrl->logid = eLOG_MAX;						// ���M�Ώۃ��O eLOG_MAX:���O�f�[�^�ȊO
		pCtrl->buffull_retry = 0;						// ���M���g���C��
		pCtrl->buffull_timer = 0;						// ���M���g���C�^�C�}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: NTNET_EraseSendData
 *[]----------------------------------------------------------------------[]
 *| summary	: ���M�f�[�^���폜����
 *|				���O�̏ꍇ�͑��M�ςɂ���
 *|				���M�o�b�t�@�f�[�^�̏ꍇ�͉������Ȃ�
 *| param	: pCtrl		: �폜���鑗�M�f�[�^�̐���f�[�^
 *|			  force		: �����폜�t���O(T���v�AGT���v�̂ݗL��)
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	NTBUF_EraseSendData(t_Ntnet_SendDataCtrl* pCtrl, BOOL force)
{
	ushort	totalLogID = eLOG_MAX;
	
	if(pCtrl->logid != eLOG_MAX) {						// ���M�f�[�^�̓��O�f�[�^
		// ���O�f�[�^�̏ꍇ�͑��M�o�b�t�@�Ɋi�[�������_�ő��M�ςɂ���
		if(pCtrl->logid != eLOG_TTOTAL && pCtrl->logid != eLOG_GTTOTAL) {	// T���v, GT���v�ȊO
			Ope_Log_TargetVoidReadPointerUpdate(pCtrl->logid, eLOG_TARGET_NTNET);
		}
		else {											// T���v, GT���v
			if(z_NtBufTotalDataIndex == NTBUF_NOTSENDTOTALDATA || force) {
				// �I���ʒm(ID:41)�܂ŏ����ς܂��́A�����폜�̏ꍇ�͑��M�ςɂ���
				Ope_Log_TargetVoidReadPointerUpdate(pCtrl->logid, eLOG_TARGET_NTNET);
				if(force) {
					z_NtBufTotalDataIndex = NTBUF_NOTSENDTOTALDATA;
				}
			}
			else {										// T, GT���v�����̃f�[�^����
				totalLogID = pCtrl->logid;
			}
		}
	}
	else {												// ���O�f�[�^�ȊO
		// ���O�f�[�^�ȊO�Ȃ���Ƀo�b�t�@����폜����Ă���
	}
	
	ntbuf_ClearSendDataCtrl(pCtrl);						// ���M����f�[�^��������
	if(totalLogID == eLOG_TTOTAL || totalLogID == eLOG_GTTOTAL) {	// T���v, GT���v�ȊO
		pCtrl->logid = totalLogID;						// �Ҕ��������OID��ݒ肷��
	}
}
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
