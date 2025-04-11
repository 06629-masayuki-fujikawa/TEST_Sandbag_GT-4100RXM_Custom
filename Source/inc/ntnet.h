#ifndef	___NTNETH___
#define	___NTNETH___
/*[]----------------------------------------------------------------------[]
 *| System      : UT8500
 *| Module      : NT-NET�ʐM���W���[�����ʒ�`
 *[]----------------------------------------------------------------------[]
 *| Author      : machida kei
 *| Date        : 2005.08.16
 *| Update      :
 *|		2005.09.07	machida.k	NT-NET�o�b�t�@�f�[�^�S�N���AIF(NTBUF_AllClr)�ǉ�
 *|		2005.09.21	machida.k	NT-NET�o�b�t�@��Ԏ擾IF(NTBUF_GetBufState)�ǉ�
 *|								�o�b�t�@FULL/�j�AFULL�Ή��̂��� NTNET_BUFSET_STATE_CHG �ǉ�
 *|		2005-11-30	machida.k	STEP2�Ή�
 *|		2006-01-17	machida.k	�A�C�h�����׌y�� (MH544401)
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*			C O N S T A N T S												*/
/*--------------------------------------------------------------------------*/

/* �o�b�t�@FULL���̈��� */
#define	NTNET_BUFSET_DEL_OLD	0x00	/* ������ */
#define	NTNET_BUFSET_DEL_NEW	0x01	/* �V���� */
#define	NTNET_BUFSET_CANT_DEL	0x02	/* �x�� */

#define	NTNET_BUFSET_COMERR		0x80	/* �ʐM��Q���̂��߃o�b�t�@�����O���Ȃ� */
#define	NTNET_BUFSET_NORMAL		0xFF	/* �o�b�t�@FULL�łȂ� */
#define	NTNET_BUFSET_STATE_CHG	0xA0	/* �o�b�t�@��ԕω�(FULLor�j�AFULL����) */


// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
///* FREE�f�[�^�T�C�Y */
//#define	FREEPKT_DATASIZE		500
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j

/* NT-NET�u���b�N�f�[�^�T�C�Y */
#define	NTNET_BLKDATA_MAX		960

/* �u���b�N��MAX */
#define	NTNET_BLKMAX_NORMAL		26		/* �ʏ�f�[�^ */
#define	NTNET_BLKMAX_PRIOR		1		/* �D��f�[�^ */
#define	NTNET_BLKMAX_BROADCAST	1		/* ����f�[�^ */

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
///* NTUPR_SetRcvPkt()�߂�l */
//typedef enum {
//	NTNET_RRSLT_NORMAL,					/* ���튮�� */
//	NTNET_RRSLT_BLKNO_INVALID,			/* �u���b�NNo.�ُ�(1NT�u���b�N��M�̏ꍇ�̂�) */
//	NTNET_RRSLT_BLKNO_VALID,			/* �u���b�NNo.�ُ�(1NT�u���b�N��M�̏ꍇ�̂�) */
//}eNTNET_RRSLT;
//
///* NTLWR_SendReq()�߂�l */
//typedef enum {
//	NTNET_SRSLT_NORMAL,					/* ARC�u���b�N����M���ʁ�00H�A01H�A86H�̂Ƃ� */
//										/* FREE�f�[�^�̏ꍇ�͑��M���ʃf�[�^��M�̂Ƃ� */
//	NTNET_SRSLT_BUFFULL,				/* ARC�u���b�N����M���ʁ�82H�̂Ƃ� */
//	NTNET_SRSLT_BLKNO,					/* ARC�u���b�N����M���ʁ�85H�̂Ƃ� */
//	NTNET_SRSLT_SENDING,				/* �f�[�^���M��(�����[�g�̂�)=90H�̂Ƃ�  */
//	NTNET_SRSLT_RETRY_OVER				/* ���g���C�I�[�o�[���� */
//}eNTNET_SRSLT;
//
///* NTUPR_SetRcvPkt()�^NTLWR_SendReq() ���� */
//typedef enum {
//	NTNET_FREEDATA,						/* FREE�f�[�^ */
//	NTNET_ERRDATA,						/* �G���[�f�[�^ */
//	NTNET_NTDATA						/* NT-NET�f�[�^ */
//}eNTNET_DATA_KIND;
typedef enum {							// NTNET, NTCom�^�X�N��API����
	NTNET_NORMAL,						// ����
	NTNET_DEL_OLD,						// �ŌÃf�[�^�㏑��
	NTNET_ERR_BUFFERFULL,				// �o�b�t�@�t��
	NTNET_ERR_BLOCKOVER,				// NTNET�u���b�N�I�[�o�[�i�D��A����F1�u���b�N�A�ʏ�A26�u���b�N�ȏ�̃f�[�^�j
	NTNET_ERR_OTHER,					// ���̑��̃G���[
} eNTNET_RESULT;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j

/* NT-NET�f�[�^�o�b�t�@��� */
typedef enum {
	NTNET_BUF_BROADCAST,				/* ����f�[�^(��M�o�b�t�@�̂�) */
	NTNET_BUF_PRIOR,					/* �o�b�t�@�����O�f�[�^�ł͂Ȃ��D��f�[�^ */
	NTNET_BUF_BUFFERING,				/* �o�b�t�@�����O�f�[�^(���M�o�b�t�@�̂�) */
	NTNET_BUF_NORMAL,					/* �o�b�t�@�����O�f�[�^�ł͂Ȃ��ʏ�f�[�^ */
	NTNET_BUF_INIT = 0xff				/* �����l */
}eNTNET_BUF_KIND;

/* �o�b�t�@���̃f�[�^�ێ���� */
typedef enum {
	NTBUF_SAVE,							/* �f�[�^�ێ����� */
	NTBUF_NOT_SAVE,						/* �f�[�^�ێ����Ȃ� */
	NTBUF_SAVE_BY_DATA,					/* �f�[�^�ێ����邩���Ȃ����͊e�f�[�^�ɂ�� */
}eNTBUF_SAVE;

/* �]���� �[��No. */
#define	NTNET_BROADCAST			0xFF	/* ����Z���N�e�B���O�̃f�[�^ */

/* �p�P�b�g�D�惂�[�h */
#define	NTNET_NORMAL_DATA		0x00	/* �ʏ�f�[�^ */
#define	NTNET_PRIOR_DATA		0x01	/* �D��f�[�^ */

/* �d����� */
#define	STX						0x02

/* �ŏI�u���b�N���� */
#define	NTNET_NOT_LAST_BLOCK	0x00	/* ���ԃu���b�N */
#define	NTNET_LAST_BLOCK		0x01	/* �ŏI�u���b�N */

/* �f�[�^�ێ��t���O */
#define	NTNET_DO_BUFFERING		0x00	/* �o�b�t�@�����O���� */
#define	NTNET_NOT_BUFFERING		0x01	/* �o�b�t�@�����O���Ȃ� */

/* �ݒ�p�����[�^ */
#define	NTPRM_TERMINALNO		3		/* �^�[�~�i��No. */
#define	NTPRM_RESPONSE_TIME		19		/* MAIN���W���[������̉����҂����� */
#define	NTPRM_RETRY				21		/* MAIN���W���[���̃��g���C�� */
#define	NTPRM_BUFFULL_TIMER		23		/* �đ��v�����̑��MWait */
#define	NTPRM_BUFFULL_RETRY		24		/* �đ��G���[����� */
#define	NTPRM_BUFFULL			41		/* ���M�f�[�^�o�b�t�@FULL���̈��� */
/* �ݒ�p�����[�^41 �e�f�[�^�ɑΉ������ */
#define	NTPRM_PLACE_SALE	1
#define	NTPRM_PLACE_CARIN	10
#define	NTPRM_PLACE_CAROUT	100
#define	NTPRM_PLACE_TTOTAL	1000
#define	NTPRM_PLACE_COIN	10000
#define	NTPRM_PLACE_NOTE	100000

/* NTNET_Err() ���� */
#define	NTERR_EMERGE	1				/* ���� */
#define	NTERR_RELEASE	0				/* ���� */
#define	NTERR_ONESHOT	2				/* ���������� */

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
///* FREE�f�[�^���ʃR�[�h */
//#define	FREEDATA_SYMBOL			"FREE"
//
///* FREE�f�[�^�v����� */
//#define	FREEDATA_KIND_INITDAT	1		/* �����ݒ�f�[�^�v�� */
//#define	FREEDATA_KIND_IBKSTS	2		/* IBK��ԗv�� */
//#define	FREEDATA_KIND_DATCNT	3		/* ���M�f�[�^�����v�� */
//#define	FREEDATA_KIND_TERMSTS	4		/* �[����ԗv�� */
//#define	FREEDATA_KIND_DATACLR	5		/* �f�[�^�N���A�v�� */
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j

/* NT-NET�o�b�t�@��� */
#define	NTBUF_BUFFER_NORMAL		0x00
#define	NTBUF_BUFFER_NEAR_FULL	0x01
#define	NTBUF_BUFFER_FULL		0x02

// MH322914(S) K.Onodera 2016/08/08 AI-V�Ή�
#define		PIP_SYSTEM		16
// MH322914(E) K.Onodera 2016/08/08 AI-V�Ή�
#define		REMOTE_SYSTEM	30
#define		REMOTE_DL_SYSTEM	40
#define		REMOTE_PC_TERMINAL_NO	99

#define		REMOTE_AUTO		0
#define		REMOTE_MANUAL	1

/*--------------------------------------------------------------------------*/
/*			S T R U C T U R E S												*/
/*--------------------------------------------------------------------------*/

/* NT-NET�ʐM�f�[�^�̃w�b�_(�f�[�^����{����) */
typedef struct {
	uchar	system_id;					/* �V�X�e��ID */
	uchar	data_kind;					/* �f�[�^��� */
	uchar	data_save;					/* �f�[�^�ێ��t���O */
	uchar	seq_no;						/* �V�[�P���V����No. */
	uchar	parking_no[4];				/* ���ԏ�No. */
	uchar	machine_code[2];			/* �@��R�[�h */
	uchar	terminal_no[4];				/* �[���@�BNo. */
	uchar	serial_no[6];				/* �[���V���A��No. */
	uchar	year;						/* �����N */
	uchar	month;						/* ������ */
	uchar	day;						/* ������ */
	uchar	hour;						/* ������ */
	uchar	minute;						/* ������ */
	uchar	second;						/* �����b */
}t_NtNetDataHeader;

/* 1NTNET�u���b�N�̃w�b�_�[�� */
typedef struct {
//	uchar	sig[5];						/* "NTCOM" */
//	uchar	len;						/* �f�[�^�T�C�Y */
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
	uchar	sig[5];						/* "NTCOM" */
	uchar	len[2];						/* �f�[�^�T�C�Y */
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
	uchar	vect;						/* �]������ */
	uchar	terminal_no;				/* �[��No. */
	uchar	lower_terminal_no1;			/* ���ʓ]���p�[��No.(1) */
	uchar	lower_terminal_no2;			/* ���ʓ]���p�[��No.(2) */
	uchar	link_no;					/* �p�P�b�g SEQ No. */
	uchar	status;						/* �[���X�e�[�^�X */
	uchar	broadcast_rcvd;				/* �����M�ς݃t���O */
	uchar	len_before0cut[2];			/* �[���J�b�g�O�̃f�[�^�T�C�Y */
	uchar	mode;						/* �p�P�b�g�D�惂�[�h */
	uchar	reserve;					/* �\�� */
	uchar	telegram_type;				/* STX */
	uchar	res_detail;					/* �����ڍ� */
}t_NtNetBlkHeader;

/* 1NT-NET�u���b�N�̃f�[�^�� */
typedef struct {
	uchar	blk_no;						/* ID1 */
	uchar	is_lastblk;					/* ID2 */
	uchar	system_id;					/* ID3 */
	uchar	data_type;					/* ID4 */
	uchar	save;						/* �f�[�^�ێ��t���O */
	uchar	data[NTNET_BLKDATA_MAX];	/* �f�[�^�� */
}t_NtNetBlkData;

// (�ȉ��Antnet_upper.c���ړ�)
typedef struct {
	t_NtNetBlkHeader	header;		/* �w�b�_�� */
	t_NtNetBlkData		data;		/* �f�[�^�� */
}t_NtNetBlk;

// (�ȉ��A�V�K�쐬)
typedef struct {
	uchar	packet_size[2];				/* �p�P�b�g�T�C�Y */
	uchar	len_before0cut[2];			/* �[���J�b�g�O�̃f�[�^�T�C�Y */
	uchar	reserve;					/* �\�� */
}t_RemoteNtNetBlkHeader;

typedef struct {
	t_RemoteNtNetBlkHeader	header;		/* �w�b�_�� */
	t_NtNetBlkData		data;			/* �f�[�^�� */
}t_RemoteNtNetBlk;

/* ��M�f�[�^1���폜�p�̃n���h�� */
typedef struct {
	int				bufcode;			/* �o�b�t�@No. */
	long			rofs;				/* �n���h���擾���_�ł̃o�b�t�@�ǂݏo���I�t�Z�b�g */
}t_NtNet_ClrHandle;

/* �o�b�t�@���(NTBUF_BUFFER_NORMAL/NTBUF_BUFFER_FULL/NTBUF_BUFFER_NEAR_FULL ��OR��� */
typedef struct {
	uchar	sale;							/* ���Z�f�[�^�e�[�u��(ID22 or 23) */
	uchar	car_out;						/* �o�Ƀf�[�^�e�[�u��(ID21) */
	uchar	car_in;							/* ���Ƀf�[�^�e�[�u��(ID20) */
	uchar	ttotal;							/* T���v�W�v�f�[�^(ID30�`38�A41) */
	uchar	coin;							/* �R�C�����ɏW�v���v�f�[�^(ID131) */
	uchar	note;							/* �������ɏW�v���v�f�[�^(ID133) */
}t_NtBufState;

/* �o�b�t�@���̃f�[�^�� */
typedef struct {
	ulong sndbuf_prior;				/* ���̑��̗D��f�[�^�c�� */
	ulong sndbuf_sale;				/* ���Z�f�[�^�c��(ID22orID33) */
	ulong sndbuf_ttotal;			/* T���v�W�v�f�[�^�c��(ID30�`38�AID41) */
	ulong sndbuf_coin;				/* �R�C�����ɏW�v���v�f�[�^�c��(ID131) */
	ulong sndbuf_note;				/* �������ɏW�v���v�f�[�^�c��(ID133) */
	ulong sndbuf_error;				/* �G���[�f�[�^�c��(ID120) */
	ulong sndbuf_alarm;				/* �A���[���f�[�^�c��(ID121) */
	ulong sndbuf_money;				/* ���K�Ǘ��f�[�^�c��(ID126) */
	ulong sndbuf_monitor;			/* ���j�^�f�[�^�c��(ID122) */
	ulong sndbuf_ope_monitor;		/* ���샂�j�^�f�[�^�c��(ID123) */
	ulong sndbuf_normal;			/* ���̑��̒ʏ�f�[�^�c�� */
	ulong rcvbuf_broadcast;			/* ����f�[�^��M�o�b�t�@�c�� */
	ulong rcvbuf_prior;				/* �D��f�[�^��M�o�b�t�@�c�� */
	ulong rcvbuf_normal;			/* �ʏ�f�[�^��M�o�b�t�@�c�� */
	ulong sndbuf_incar;				/* ���Ƀf�[�^�c��(ID20) */
	ulong sndbuf_outcar;			/* �o�Ƀf�[�^�e�[�u��(ID21)    */
	ulong sndbuf_car_count;			/* ���ԑ䐔�f�[�^�c��(ID236)   */
	ulong sndbuf_area_count;		/* ���䐔�E���ԃf�[�^(ID237) */
	ulong sndbuf_turi;				/* �ޑK�Ǘ��W�v�f�[�^�c��(ID135) */
	ulong sndbuf_rmon;				/* ���u�Ď��f�[�^�c��(ID125) */
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	ulong sndbuf_lpark;				/* �������ԏ�ԃf�[�^�c��(ID61) */
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
}t_NtBufCount;

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
typedef struct {
	ushort				length;						// �f�[�^��
	uchar				*data;						// �f�[�^�{��
	uchar				type;						// �f�[�^���
	ushort				logid;						// ���OID
	long				buffull_retry;				// IBK�o�b�t�@FULL�ɂ�鑗�M���g���C��
	ulong				buffull_timer;				// IBK�o�b�t�@FULL�ɂ�鑗�M���g���C�p�^�C�}�[
}t_Ntnet_SendDataCtrl;
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/
/*------------------------------------------------------------ ntnet_lower.c ----*/

void	NTNET_Init(uchar clr);
void	NTNET_Err(int errcode, int occur);
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j
///* �A�C�h�����׌y�� */
//#define	_NTNET_ChkDispatch()	(NTUPR_ChkDispatch() || _NTLWR_ChkDispatch())
//
///*------------------------------------------------------------ ntnet_lower.c ----*/
//
//void	NTLWR_Init(void);
//void	NTLWR_Main(MsgBuf *pmsg);
//BOOL	NTLWR_SendReq(uchar *data, ushort len, eNTNET_DATA_KIND kind);
//BOOL	NTLWR_IsSendComplete(eNTNET_SRSLT *result);
///* �A�C�h�����׌y�� */
//extern	BOOL	NtLwr_Dispatch;
//#define	_NTLWR_ChkDispatch()	(NtLwr_Dispatch)
//
///*------------------------------------------------------------ ntnet_upper.c ----*/
//
//void	NTUPR_Init(uchar clr);
//void	NTUPR_Main(void);
//eNTNET_RRSLT	NTUPR_SetRcvPkt(uchar *data, ushort len, eNTNET_DATA_KIND kind);
///* �A�C�h�����׌y�� */
//BOOL	NTUPR_ChkDispatch(void);
//
//uchar	NTUPR_SendingPacketKindGet( ushort UpperQueKind );
//
//void	NTUPR_Init2( void );
extern	void	NTNET_Main(void);
extern	eNTNET_RESULT	NTNET_SetReceiveData(const uchar* pNtData, ushort size, uchar type);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304�Q�l�j

/*------------------------------------------------------------ ntnet_buffer.c ----*/

void	NTBUF_Init(uchar clr);
void	NTBUF_AllClr(void);
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
void	NTBUF_AllClr_startup(void);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
uchar	NTBUF_SetSendNtData(const void *data, ushort len, eNTNET_BUF_KIND bufkind);
ushort	NTBUF_GetRcvNtData(void *data, eNTNET_BUF_KIND bufkind);
void	NTBUF_ClrRcvNtData_Prepare(eNTNET_BUF_KIND bufkind, t_NtNet_ClrHandle *handle);
void	NTBUF_ClrRcvNtData_Exec(const t_NtNet_ClrHandle *handle);
void	NTBUF_CommErr(uchar sts);
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//void	NTBUF_DataClr(int req);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//ushort	NTBUF_GetSndNtData(void *data, eNTNET_BUF_KIND bufkind, ushort *UpperQueKind);
ushort	NTBUF_GetSndNtData(t_Ntnet_SendDataCtrl* pCtrl, eNTNET_BUF_KIND bufkind);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
uchar	NTBUF_SetRcvNtData(const void *data, ushort len, eNTNET_BUF_KIND bufkind);
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//void	NTBUF_ClrSndNtData_Prepare(const void *data, eNTNET_BUF_KIND bufkind, t_NtNet_ClrHandle *handle);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
void	NTBUF_ClrSndNtData_Exec(const t_NtNet_ClrHandle *handle);

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//uchar	NTBUF_SetSendFreeData(const void *data);
//uchar	NTBUF_GetRcvFreeData(void *data);
//uchar	NTBUF_SetRcvFreeData(const void *data);
//uchar	NTBUF_GetSndFreeData(void *data);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

void	NTBUF_SetRcvErrData(const void *data);

const t_NtBufState	*NTBUF_GetBufState(void);

void NTBUF_SetIBKNearFull(ulong mask, ulong occur);
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//void NTBUF_SetIBKNearFullByID(uchar ID);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
void NTBUF_SetIBKPendingByID(uchar ID);
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
void	NTBUF_EraseSendData(t_Ntnet_SendDataCtrl* pCtrl, BOOL force);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j


/* �A�C�h�����׌y�� */
extern	BOOL z_NtBuf_SndDataExist;
#define	_NTBUF_SndDataExist()	(z_NtBuf_SndDataExist)

// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
//void	NTBUF_GetBufCount(t_NtBufCount *buf);
void	NTBUF_GetBufCount(t_NtBufCount *buf, BOOL isRemote);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j

ushort	ntupr_NtNetDopaSndBufCheck_Del(uchar bufkind, uchar status);

void	NTNET_GetRevData( ushort msg, uchar * );
void	NTNET_RevData01( void );
void	NTNET_RevData02( void );
void	NTNET_RevData03( void );
void	NTNET_RevData04( void );
void	NTNET_RevData22( void );
void	NTNET_RevData100( void );
void	NTNET_RevData103( void );
void	NTNET_RevData109( void );
void	NTNET_RevData154( void );
void	NTNET_RevData156( void );							// ���������ݒ�v��
void	NTNET_RevData119( void );
void	NTNET_RevData143( void );
void	BasicDataMake( uchar knd, uchar keep );
void	NTNET_Snd_Data01( ulong op_lokno );
void	NTNET_Snd_Data02( ulong op_lokno, ushort pr_lokno, ushort ans );
void	NTNET_Snd_Data05( ulong op_lokno, uchar loksns, uchar lokst );
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�s�v�����폜�j
//void	NTNET_Snd_Data12( ulong MachineNo );
//void	NTNET_Snd_Data20( ushort pr_lokno );
//void	NTNET_Snd_Data20_frs( ushort pr_lokno, void *data );
//void	NTNET_Snd_Data22_LO( ushort pr_lokno, ushort paymethod, ushort payclass, ushort outkind );
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�s�v�����폜�j
void	NTNET_Snd_DataParkCarNumWeb( uchar kind );
void	NTNET_Snd_Data58( void );
void	NTNET_Snd_Data101( ulong MachineNo );
void	NTNET_Snd_Data104( ulong req );
void	NTNET_Snd_Data120( void );
void	NTNET_Snd_Data120_CL( void );
void	NTNET_Snd_Data121( void );
void	NTNET_Snd_Data121_CL( void );
void	NTNET_Snd_Data126( ulong MachineNo, ushort payclass );
void	NTNET_Snd_Data229( void );
void	NTNET_Snd_Data229_Exec( void );
void	NTNET_Snd_Data230( ulong MachineNo );
void	NTNET_Snd_Data231( ulong MachineNo );
void	NTNET_Snd_Data142( ulong pkno, ulong passid );
void	NTNET_Snd_DataFree( void );
void	NTNET_Snd_Data100( void );

void	NTNET_Snd_Data90(uchar type);

void	NTNET_RevData80( void );
void	NTNET_RevData91( void );
void	NTNET_RevData93( void );
void	NTNET_RevData94( void );
void	NTNET_RevData95( void );
void	NTNET_RevData97( void );
void	NTNET_RevData98( void );
void	NTNET_RevData110( void );
void	NTNET_RevData116( void );
void	NTNET_RevData117( void );
void	NTNET_Snd_Data99(ulong MachineNo, int CompleteKind);
void	NTNET_Snd_Data101_2( void );
void	NTNET_Snd_Data109( ulong req, char value );
void	NTNET_Snd_Data116( uchar ProcMode, ulong PassId, ulong ParkingId, uchar UseParkingKind, uchar Status, uchar UpdateStatus, date_time_rec *ExitTime );
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�s�v�����폜�j
//void	NTNET_Snd_Data122( uchar kind, uchar code, uchar level, uchar *info, uchar *message );
//void	NTNET_Snd_Data123( uchar kind, uchar code, uchar level, uchar *before, uchar *after, uchar *message );
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�s�v�����폜�j
void	NTNET_Snd_Data208(ulong mac, ushort sts, ushort seg);
void	_NTNET_Snd_Data208(ulong mac, ushort sts, ushort seg, ushort addr, ushort count);
void	NTNET_RevData83( void );
void	NTNET_Snd_Data84(ulong MachineNo, uchar ng);
void	NTNET_Snd_Data219(uchar Mode, struct TKI_CYUSI *Data);
void	NTNET_Snd_Data221( void );
void	NTNET_Snd_Data222( void );
void	NTNET_Snd_Data223( void );
void	NTNET_Snd_Data225( ulong MachineNo );
void	NTNET_Snd_Data226( ulong MachineNo );
void	NTNET_RevData66( void );
void	NTNET_RevData77( void );
void	NTNET_RevData194( void );
void	NTNET_RevData205( void );
void	NTNET_Snd_TGOUKEI( void );
void	NTNET_Snd_TSYOUKEI( void );
void	NTNET_RevData78( void );
void	NTNET_RevData114( void );
void	NTNET_RevData234( void );
char	NTNET_is234StateValid(void);

void	NTNET_RevData92(void);
void	NTNET_RevData240(void);
// MH810100(S) K.Onodera 2019/12/19 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//void	NTNET_RevData243(void);
// MH810100(E) K.Onodera 2019/12/19 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

void	NTNET_RevData80_R(void);
void	NTNET_RevData90_R(void);
void	NTNET_RevData116_R(void);

void	NTNET_Snd_Data115( ulong MachineNo, uchar ProcMode );
void	NTNET_Snd_Data130( void );
void	NTNET_Snd_Data132( void );
void	NTNET_Snd_Data211( void );
void	NTNET_Snd_Data211_Exec( void );
// GM849100(S) M.Fujikawa 2025/01/15 ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j
//void	NTNET_Snd_Data228( ulong MachineNo );
void	NTNET_Snd_Data228( ulong MachineNo, uchar from );
// GM849100(E) M.Fujikawa 2025/01/15 ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j
void	NTNET_Snd_Data235(char bReq);
void	NTNET_Snd_Data235_Exec(char bReq);
void	NTNET_Snd_Data131(void);
void	NTNET_Snd_Data133(void);
void	NTNET_Snd_DataParkCarNum(uchar kind);
void	NTNET_Snd_Data236(void);
void	NTNET_Snd_Data105(ulong MachineNo, ushort code);
void	NTNET_Snd_Data110(ulong MachineNo, ushort code);
void	NTNET_Snd_Data155(ulong MachineNo, ushort code);	// �Z���^�p�f�[�^�v��NG���M
// �d�l�ύX(S) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
//void	NTNET_Snd_Data152(void);							// ���Z��ԃf�[�^
// MH810105(S) MH364301 QR�R�[�h���ϑΉ�
//void	NTNET_Snd_Data152_r12(void);						// ���Z��ԃf�[�^
void	NTNET_Snd_Data152_rXX(void);						// ���Z��ԃf�[�^
// MH810105(E) MH364301 QR�R�[�h���ϑΉ�
// �d�l�ύX(E) K.Onodera 2016/11/04 ���Z���f�[�^�t�H�[�}�b�g�Ή�
void	NTNET_Snd_Data153(void);							// ���A���^�C�����
void	NTNET_Snd_Data157(ulong MachineNo, ushort code, uchar setget);	// ���������ݒ�v�� ����
// MH810100(S) K.Onodera 2019/12/19 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//void	NTNET_Snd_Data238(void);
//void	NTNET_Snd_Data244(ushort code);
// MH810100(E) K.Onodera 2019/12/19 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
void	NTNET_Snd_Data245(ushort GroupNo);
void	BasicDataMake_R( uchar knd, uchar keep );
void	NTNET_Snd_Data60_R(ulong req);
void	NTNET_Snd_DataTblCtrl(uchar id, ulong req, uchar bAuto);
void	NTNET_Snd_Data100_R(uchar mode, uchar chk_num);
void	NTNET_RevData83_R(void);
void	NTNET_Rev_Data101_R(void);

void	NTNET_ClrSetup(void);
void	NTNET_RevData78( void );

void	NTNET_RevData150( void );
void	NTNET_RevData254( void );
void	NTNET_RevData84( void );
void	NTNET_Snd_Data108(ushort KakariNo, uchar Status);

void	NTNET_Snd_Data190( void );
void	NTNET_RevData188( void *);
void	NTNET_Snd_Data189( uchar err_code, uchar data );
void	NTNET_Ans_Data99(uchar SysID, ulong MachineNo, int CompleteKind, uchar CompleteInfo);
void	NTNET_Ans_Data115(uchar SysID, ulong MachineNo, uchar ng);
void	NTNET_Ans_Data208(uchar SysID, ulong MachineNo, ushort addr, ushort count, uchar okng);
ulong	OrgCal( ushort Index );
void	regist_mon(ushort code, ushort error, ushort param, ushort num);
int		check_remote_addr(ushort kind, ushort addr, ushort count);
int		update_param(void);
uchar	cyushi_chk( ushort no );
// MH322914 (s) kasiyama 2016/07/15 AI-V�Ή�
void	NTNET_Snd_Data16_04( ushort rslt );
void	NTNET_Snd_Data16_08( ushort rslt );
void	NTNET_Snd_Data16_09( ushort rslt );
void	PiP_FurikaeEnd( void );
void	PiP_FurikaeStart( void );
ushort PiP_GetFurikaeSts( void );
void	PiP_FurikaeInit( void );
// MH322914 (e) kasiyama 2016/07/15 AI-V�Ή�

#define NTNET_Snd_Data61_R(req, sauto)	NTNET_Snd_DataTblCtrl(61, req, sauto)
#define NTNET_Snd_Data62_R(req)	NTNET_Snd_DataTblCtrl(62, req, 0)

extern t_NtBufCount IBK_BufCount;
extern uchar		IBK_ComChkResult;
extern uchar		stNtBufCountOffset[];

#define REMOTE_IBKCTRL_MAX				3
#define	REMOTE_IBKCTRL_TBLNUM			0x00000001		// �e�[�u�������f�[�^
#define	REMOTE_IBKCTRL_SET1				0x00000002		// ���uIBK�ݒ�f�[�^�P�v��
#define	REMOTE_IBKCTRL_SET2				0x00000004		// ���uIBK�ݒ�f�[�^�Q�v��

// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//#define NTNET_BUFCTRL_REQ_MAX			15
#define NTNET_BUFCTRL_REQ_MAX			16
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
#define	NTNET_BUFCTRL_REQ_INCAR			0x00000001		// ���Ƀf�[�^
#define	NTNET_BUFCTRL_REQ_OUTCAR		0x00000002		// �o�Ƀf�[�^
#define	NTNET_BUFCTRL_REQ_SALE			0x00000004		// ���Z�f�[�^
#define	NTNET_BUFCTRL_REQ_TTOTAL		0x00000008		// T���v�W�v�f�[�^
#define	NTNET_BUFCTRL_REQ_ERROR			0x00000010		// �G���[�f�[�^
#define	NTNET_BUFCTRL_REQ_ALARM			0x00000020		// �A���[���f�[�^
#define	NTNET_BUFCTRL_REQ_MONITOR		0x00000040		// ���j�^�f�[�^
#define	NTNET_BUFCTRL_REQ_OPE_MONITOR	0x00000080		// ���샂�j�^�f�[�^
#define	NTNET_BUFCTRL_REQ_COIN			0x00000100		// �R�C�����ɏW�v���v�f�[�^
#define	NTNET_BUFCTRL_REQ_NOTE			0x00000200		// �������ɏW�v���v�f�[�^
#define	NTNET_BUFCTRL_REQ_CAR_COUNT		0x00000400		// ���ԑ䐔�f�[�^
#define	NTNET_BUFCTRL_REQ_AREA_COUNT	0x00000800		// ���䐔�E���ԃf�[�^
#define	NTNET_BUFCTRL_REQ_MONEY			0x00001000		// ���K�Ǘ��f�[�^
#define	NTNET_BUFCTRL_REQ_TURI 			0x00002000		// �ޑK�Ǘ��W�v�f�[�^
#define	NTNET_BUFCTRL_REQ_RMON 			0x00004000		// ���u�Ď��f�[�^
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//#define	NTNET_BUFCTRL_REQ_ALL		0x00007fff		// �S�f�[�^
#define	NTNET_BUFCTRL_REQ_LONG_PARK		0x00008000		// �������ԏ�ԃf�[�^
// MH810100(S) Y.Yamauchi 20191030 �Ԕԃ`�P�b�g���X(�����e�i���X)
//#define	NTNET_BUFCTRL_REQ_ALL			0x0000ffff		// �S�f�[�^
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//#define	NTNET_BUFCTRL_REQ_ALL			0x0000fffe		// ���Ƀf�[�^�ȊO�̑S�f�[�^
#define	NTNET_BUFCTRL_REQ_ALL			0x000073fc		// ����/�o��/���ԑ䐔/���䐔�E����/�������ԏ�ԃf�[�^�ȊO�̑S�f�[�^
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
// MH810100(E) Y.Yamauchi 20191030 �Ԕԃ`�P�b�g���X(�����e�i���X)
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
// Phase1�ɂ͒ޑK�Ǘ����Ȃ����߁A�S�f�[�^�͈ȉ��̒l�ƂȂ�
// MH810100(S) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//#define	NTNET_BUFCTRL_REQ_ALL_PHASE1	0x00001fff		// �S�f�[�^(Phase1)
#define	NTNET_BUFCTRL_REQ_ALL_PHASE1	0x000013fc		// ����/�o��/���ԑ䐔/���䐔�E���ԃf�[�^�ȊO�̑S�f�[�^(Phase1)
// MH810100(E) K.Onodera 2019/12/23 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

void	NTNET_Snd_Data63_R(void);
void	NTNET_Snd_Data117_R(uchar mode, char *data);

void	mc10exec( void );

void	NTNET_Snd_Data143(ulong ParkingNo, ulong PassID);	// ������⍇�����ʃf�[�^
void	NTNET_RevData142(void);								// ������⍇���f�[�^

typedef	struct {
	uchar		DataKind;
	void		*Src;
} NTNETCTRL_FUKUDEN_PARAM;

extern const	uchar	NTNET_SYSTEM_ID[10];

void	NTNET_UpdateParam(NTNETCTRL_FUKUDEN_PARAM *fukuden);
short	NTNET_isTotalEndReceived( ushort *result );
ulong NTNET_GetParkingNo(uchar ParkingKind);
enum {
	PKOFS_SEARCH_LOCAL,
	PKOFS_SEARCH_MASTER
};
uchar NTNET_GetParkingKind(ulong ParkingId, int search);

extern uchar	Ntnet_Remote_Comm;
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԒʐM�͌ʐݒ���Q�Ƃ���j
extern uchar	Ntnet_Term_Comm;							// NT-NET(���S�����R�[���Z���^�[)�g�p�ݒ�
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԒʐM�͌ʐݒ���Q�Ƃ���j
#define	_ntnet_by_hif		0x10
#define	_is_ntnet_remote()	((Ntnet_Remote_Comm & 0x0f) == 2)
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԒʐM�͌ʐݒ���Q�Ƃ���j
//#define	_is_ntnet_normal()	((Ntnet_Remote_Comm & 0x0f) == 1)
#define	_is_ntnet_normal()	((Ntnet_Term_Comm & 0x0f) == 1)
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԒʐM�͌ʐݒ���Q�Ƃ���j
#define	_is_ntnet			_is_ntnet_normal
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԒʐM�͌ʐݒ���Q�Ƃ���j
//#define	_not_ntnet()		(Ntnet_Remote_Comm == 0)
#define	_not_ntnet()		(Ntnet_Remote_Comm == 0 && Ntnet_Term_Comm == 0)
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԒʐM�͌ʐݒ���Q�Ƃ���j
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԒʐM�͌ʐݒ���Q�Ƃ���j
//#define	_is_ibk_ntnet()		(Ntnet_Remote_Comm == 1)
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԒʐM�͌ʐݒ���Q�Ƃ���j
#define	_is_ibk_ntnet_remote()		(Ntnet_Remote_Comm == 2)
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԂƉ��u�𕹗p����j
#define NTNET_TARGET_MAX	2						// �[����, ���u
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�i�[���ԂƉ��u�𕹗p����j

extern	void	Remote_Cal_Data_Restor( void );

extern 	char	z_NtRemoteAutoManual;
extern	void NTNET_RAUResult_Send(uchar systemID, uchar dataType, uchar result);

extern	void	NTNET_Snd_Data65( ulong MachineNo );
extern	uchar	GetNtDataSeqNo(void);
// GM849100(S) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
// ���M�f�[�^���� 0:�D��f�[�^ 1:�ʏ�f�[�^
extern	t_Ntnet_SendDataCtrl	NTNET_SendCtrl[2];

extern	ushort	NTNET_ConvertLogToNTNETData(ushort logType, uchar* pLogData, uchar* pNtnetData);
extern	ushort	NTNET_ConvertTotalLogToNTNETData(ushort logType, ushort logID, uchar* pLogData, uchar* pNtnetData);
// GM849100(E) ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j�iFT-4000N�FMH364304���p�j
// GM849100(S) M.Fujikawa 2025/01/10 ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j
void	NTNET_Snd_Data230_T( ulong MachineNo );
// GM849100(E) M.Fujikawa 2025/01/10 ���S�����R�[���Z���^�[�Ή��iNT-NET�[���ԒʐM�j

#endif	/* ___NTNETH___ */

