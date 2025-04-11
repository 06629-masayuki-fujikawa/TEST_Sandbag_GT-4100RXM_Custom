/****************************************************************************/
/*																			*/
/*		�V�X�e�����@:�@RAU�V�X�e��											*/
/*		�t�@�C����	:  DATATABLE.C											*/
/*		�@�\		:  �f�[�^�e�[�u������									*/
/*																			*/
/****************************************************************************/
//	2006.09.21 #001 m-onouchi �e�[�u���f�[�^�̃o�b�t�@�j�A�t�������𑗐M����悤�ɏC���B
//	2006.10.02 #002 m-onouchi �e�[�u���`�k�k�N���A���ɓK�؂ɕϐ������������Ă��Ȃ��s����C���B
//	2007.01.24 #003 m-onouchi �e�[�u���f�[�^�̃o�b�t�@�j�A�t�������������R�O������O���ɕύX�B
//	2007.02.26 #004 m-onouchi �j�A�t���������̃e�[�u���N���A�Ńj�A�t�������𑗐M���Ȃ��s����C���B
//===========================================================================

#include	<stdio.h>
#include	<string.h>
#include	<stddef.h>
#include	"system.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
#include	"rau.h"
#include	"rauconstant.h"
#include	"rauIDproc.h"


const RAU_ID_INIT_INFO	init_info[RAU_TABLE_MAX] = 
{
	{RAU_IN_PARKING_SIZE	, 20	},		// ID20		���Ƀf�[�^
	{RAU_OUT_PARKING_SIZE	, 21	},		// ID21		�o�Ƀf�[�^
	{RAU_PAID_DATA22_SIZE	, 22	},		// ID22		���Z�f�[�^
	{RAU_PAID_DATA23_SIZE	, 23	},		// ID23		���Z�f�[�^
	{RAU_ERROR_SIZE			, 120	},			// ID120	�G���[�f�[�^
	{RAU_ALARM_SIZE			, 121	},			// ID121	�A���[���f�[�^
	{RAU_MONITOR_SIZE		, 122	},		// ID122	���j�^�f�[�^
	{RAU_OPE_MONITOR_SIZE	, 123	},		// ID123	���샂�j�^�f�[�^
	{RAU_COIN_TOTAL_SIZE	, 131	},		// ID131	�R�C�����ɏW�v���v�f�[�^
	{RAU_MONEY_TORAL_SIZE	, 133	},		// ID133	�������ɏW�v���v�f�[�^
	{RAU_PARKING_NUM_SIZE	, 236	},		// ID236	���ԑ䐔�f�[�^
	{RAU_AREA_NUM_SIZE		, 237	},		// ID237	���䐔�E���ԃf�[�^
	{RAU_MONEY_MANAGE_SIZE	, 135	},		// ID135	�ޑK�Ǘ��W�v�f�[�^
	{RAU_TOTAL_SIZE			, 0	},			// ID41		T���v�W�v�f�[�^
	{RAU_TOTAL_SIZE			, 0	},			// ID41		GT���v�W�v�f�[�^
	{RAU_MONEY_MANAGE_SIZE	, 126	},		// ID126	���K�Ǘ��f�[�^
	{RAU_REMOTE_MONITOR_SIZE, 125	},		// ID125	���u�Ď��f�[�^
	{RAU_CENTER_TERM_INFO_SIZE	, 65	},	// ID65		�Z���^�[�p�[�����f�[�^
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	{RAU_LONG_PARKING_INFO_SIZE	, 61	},	// ID61		�������ԏ��f�[�^
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	{RAU_SEND_THROUGH_DATA_SIZE	, 0	},		// ���M�X���[�f�[�^�iID �s��j�����M�̂�32block�Ɋg�� 2006.08.31:m-onouchi
	{RAU_RECEIVE_THROUGH_DATA_SIZE	, 0	}	// ��M�X���[�f�[�^�iID �s��j
};


typedef struct {
	uchar	id;
	uchar	index;
} RAU_ID_BUFF_INFO;

const RAU_ID_BUFF_INFO	RAU_id_buff[RAU_ID_BUFF_NUM] = 
{
	{20		,RAU_IN_PARKING_TABLE			},		// ID20		���Ƀf�[�^
	{21		,RAU_OUT_PARKING_TABLE			},		// ID21		�o�Ƀf�[�^
	{22		,RAU_PAID_DATA22_TABLE			},		// ID22		���Z�f�[�^
	{23		,RAU_PAID_DATA23_TABLE			},		// ID23		���Z�f�[�^
	{54		,RAU_IN_PARKING_TABLE			},		// ID54		���Ƀf�[�^
	{55		,RAU_OUT_PARKING_TABLE			},		// ID55		�o�Ƀf�[�^
	{56		,RAU_PAID_DATA22_TABLE			},		// ID56		���Z�f�[�^
	{57		,RAU_PAID_DATA23_TABLE			},		// ID57		���Z�f�[�^
	{120	,RAU_ERROR_TABLE				},		// ID120	�G���[�f�[�^
	{121	,RAU_ALARM_TABLE				},		// ID121	�A���[���f�[�^
	{122	,RAU_MONITOR_TABLE				},		// ID122	���j�^�f�[�^
	{123	,RAU_OPE_MONITOR_TABLE			},		// ID123	���샂�j�^�f�[�^
	{131	,RAU_COIN_TOTAL_TABLE			},		// ID131	�R�C�����ɏW�v���v�f�[�^
	{133	,RAU_MONEY_TORAL_TABLE			},		// ID133	�������ɏW�v���v�f�[�^
	{236	,RAU_PARKING_NUM_TABLE			},		// ID236	���ԑ䐔�f�[�^
	{58		,RAU_PARKING_NUM_TABLE			},		// ID58		Web�p���ԑ䐔�f�[�^
	{237	,RAU_AREA_NUM_TABLE				},		// ID237	���䐔�E���ԃf�[�^
	{30		,RAU_TOTAL_TABLE				},		// ID30		T���v�W�v�f�[�^
	{31		,RAU_TOTAL_TABLE				},		// ID31		T���v�W�v�f�[�^
	{32		,RAU_TOTAL_TABLE				},		// ID32		T���v�W�v�f�[�^
	{33		,RAU_TOTAL_TABLE				},		// ID33		T���v�W�v�f�[�^
	{34		,RAU_TOTAL_TABLE				},		// ID34		T���v�W�v�f�[�^
	{35		,RAU_TOTAL_TABLE				},		// ID35		T���v�W�v�f�[�^
	{36		,RAU_TOTAL_TABLE				},		// ID36		T���v�W�v�f�[�^
	{37		,RAU_TOTAL_TABLE				},		// ID37		T���v�W�v�f�[�^
	{38		,RAU_TOTAL_TABLE				},		// ID38		T���v�W�v�f�[�^
	{41		,RAU_TOTAL_TABLE				},		// ID41		T���v�W�v�f�[�^
	{42		,RAU_TOTAL_TABLE				},		// ID42		�t�H�[�}�b�gRevNo.10 �W�v��{�f�[�^
	{43		,RAU_TOTAL_TABLE				},		// ID43		�t�H�[�}�b�gRevNo.10 ������ʖ��W�v�f�[�^
	{45		,RAU_TOTAL_TABLE				},		// ID45		�t�H�[�}�b�gRevNo.10 �����W�v�f�[�^
	{46		,RAU_TOTAL_TABLE				},		// ID46		�t�H�[�}�b�gRevNo.10 ����W�v�f�[�^
	{49		,RAU_TOTAL_TABLE				},		// ID49		�t�H�[�}�b�gRevNo.10 �^�C�����W�W�v�f�[�^
	{53		,RAU_TOTAL_TABLE				},		// ID53		�t�H�[�}�b�gRevNo.10 �W�v�I���ʒm�f�[�^
	{126	,RAU_MONEY_MANAGE_TABLE			},		// ID126	���K�Ǘ��f�[�^
	{135	,RAU_TURI_MANAGE_TABLE			},		// ID135	�ޑK�Ǘ��W�v�f�[�^
	{125	,RAU_REMOTE_MONITOR_TABLE		},		// ID125	���u�Ď��f�[�^
	{65		,RAU_CENTER_TERM_INFO_TABLE		},		// ID65		�Z���^�[�p�[�����f�[�^
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	{61		,RAU_LONG_PARKING_INFO_TABLE	},		// ID61		�������ԏ��f�[�^
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	{0		,RAU_SEND_THROUGH_DATA_TABLE	},		// ���M�X���[�f�[�^		�iID �s��j
	{0		,RAU_RECEIVE_THROUGH_DATA_TABLE	},		// ��M�X���[�f�[�^		�iID �s��j
};

const	short	RAU_id_LogIDList[RAU_ID_LOGIDLIST_COUNT] = {
// MH810100(S) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//	eLOG_ENTER,				// ����
	-1,						// ����
// MH810100(E) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
	-1,						// �o��
	eLOG_PAYMENT,			// ���Z(ID22)
	-1,						// ���Z(ID23)
	eLOG_ERROR,				// �G���[
	eLOG_ALARM,				// �A���[��
	eLOG_MONITOR,			// ���j�^
	eLOG_OPERATE,			// ����
	eLOG_COINBOX,			// �R�C�����ɏW�v(ram�̂�)
	eLOG_NOTEBOX,			// �������ɏW�v(ram�̂�)
// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	eLOG_PARKING,			// ���ԑ䐔�f�[�^
	-1,						// ���ԑ䐔�f�[�^
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	-1,						// ���䐔�E���ԃf�[�^
	eLOG_MNYMNG_SRAM,		// �ޑK�Ǘ�(SRAM)
	eLOG_TTOTAL,			// T�W�v
	eLOG_GTTOTAL,			// GT�W�v
	eLOG_MONEYMANAGE,		// ���K�Ǘ�(SRAM)
	eLOG_REMOTE_MONITOR,	// ���u�Ď�
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	eLOG_LONGPARK_PWEB,		// ��������(ParkingWeb�p)
	-1,						// ��������(ParkingWeb�p)
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
};

/* NT data structure */
struct _ntdata {
	char	len[2];
	char	len_zero_cut[2];
	char	dummy;
	char	id1;
	char	id2;
	char	id;
	char	kind;
	char	flag;
	char	seq;
	char	parking[4];
	char	code[2];
	char	term[4];
	char	term_seq[6];
	char	data[1];		/* date & sevral data */
};



/********************* FUNCTION DEFINE **************************/
void	RAU_data_table_init(void);

void	data_rau_init(void);				// RAU�C�j�V�����C�Y

extern	void	RAU_CrcCcitt(ushort length, ushort length2, uchar cbuf[], uchar cbuf2[], uchar *result, uchar type );
void	RAUdata_DeleteTsumGroup(ushort ucSendCount);
void	RAUdata_ReadNtData(uchar *pData, ushort wReadLen, RAU_DATA_TABLE_INFO *pBuffInfo, T_SEND_NT_DATA *pSendNtDataInf);
BOOL	RAUdata_WriteNtData(uchar *pData, ushort wDataLen, RAU_DATA_TABLE_INFO *pBuffInfo);
void	RAUdata_DelNtData(RAU_DATA_TABLE_INFO *pBuffInfo, uchar cPosition);
BOOL	RAUdata_GetNtData(RAU_DATA_TABLE_INFO *pBuffInfo, T_SEND_NT_DATA *pSendNtDataInf);
BOOL	RAUdata_GetTsumData(T_SEND_NT_DATA *pSendNtDataInf, uchar *pTsumBuff);
uchar*	RAUdata_GetNtDataAdrr(RAU_DATA_TABLE_INFO *pBuffInfo, ushort wCount);
uchar*	RAUdata_GetNtDataNextAdrr(RAU_DATA_TABLE_INFO *pBuffInfo, T_SEND_NT_DATA *pSendNtDataInf);
void	RAUdata_GetNtDataInfoThrough(T_SEND_DATA_ID_INFO *pSendDataInfo);
void	RAUdata_GetNtDataInfo(T_SEND_DATA_ID_INFO *pSendDataInfo, RAU_SEND_REQUEST *pReqInfo, uchar ucSendReq);
eRAU_TABLEDATA_SET	RAUdata_SetThroughData(uchar *pData, ushort wDataLen, uchar ucDirection);
eRAU_TABLEDATA_SET	RAUdata_SetTableData(uchar *pData, ushort wDataLen, uchar ucDataId);
ulong	RAUdata_GetUseBuff(RAU_DATA_TABLE_INFO *pBuffInfo);
BOOL	RAUdata_CheckNearFull( void );
BOOL	RAUdata_CheckCapacity(RAU_DATA_TABLE_INFO *pBuffInfo, ushort wDataLen);
void	RAUdata_Create0Data(RAU_NTDATA_HEADER *pNtDataHeader, uchar ucDataId);
void	RAUdata_CreateNearFullData(RAU_NEAR_FULL *pNearFullData);
void	RAUdata_CreatTableDataCount(RAU_TABLE_DATA_COUNT *pDataCount);
void	RAUdata_TableClear(uchar ucTableKind);
void	RAUdata_CheckNearFullLogData(void);
BOOL	RAUdata_GetCenterTermInfoData(T_SEND_NT_DATA *pSendNtDataInf, uchar *pTsumBuff);


uchar	*RAU_raudata_ReadTable(RAU_DATA_TABLE_INFO *table, const uchar *rptr, uchar *data, size_t size);

#define	RAU_ptrSeek(p, sz, top, end)	(uchar*)(((size_t)(p) + (sz) >= (size_t)(end)) ? \
										(size_t)(top) + ((size_t)(p) + (sz) - (size_t)(end)) : (size_t)(p) + (sz))

extern	void	RAUhost_SetError(uchar ucErrCode);
extern	void	RAU_Word2Byte(unsigned char *data, ushort uShort);
extern	void	RAUhost_ClearSendRequeat(void);
extern	uchar	RAUhost_GetRcvSeqFlag(void);
extern	BOOL	RAU_isTableData(uchar ucDataId);

////////////////////////////////////////////////////////////////////////////////
//�� sizeof(struct _ctib)�͒����o�C�g���݂�4+4+237+1=246
#define	_CENTER_TERM_INFO_SIZE		237		// �����[�gNT-NET�w�b�_10bytes + �f�[�^225bytes + CRC2bytes
#define	_CENTER_TERM_INFO_MAX		2

/* center terminal info. buffer */
struct _ctib {
	char	*next;
	long	length;
	char	ntdata[_CENTER_TERM_INFO_SIZE];
};

/*------------------------------------------------------------------*/
/*	T-sum data(ID=3x,41)											*/
/*------------------------------------------------------------------*/

/*------------------------------------------------------------------*/
/*	nt data															*/
/*	reset															*/
/*------------------------------------------------------------------*/
void	reset_ntbuf(struct _ntman *ntman, char *buff, long size)
{
	ntman->top = buff;
	ntman->bottom = buff + size;
	ntman->freearea = buff;
	ntman->sendtop = NULL;
	ntman->sendnow = NULL;
}

/*------------------------------------------------------------------*/
/*	nt data															*/
/*	init															*/
/*------------------------------------------------------------------*/
int		check_que(struct _ntman *ntman, struct _ntbuf *top)
{
	struct _ntbuf	*pre_ntbuf;
	struct _ntbuf	*cur_ntbuf;
	int length;
	long	remain;
	char	*p;

	cur_ntbuf = top;
 	if (cur_ntbuf == NULL) {
		return 0;
	}

	pre_ntbuf = NULL;

	do {
	/* check send queue */
		if ((char*)cur_ntbuf < ntman->top || (char*)cur_ntbuf >= ntman->bottom) {
			if (pre_ntbuf == NULL) {
				return 1;
			} else {
				pre_ntbuf->next = NULL;
				break;
			}
		}
		if (cur_ntbuf->length <= 0 || cur_ntbuf->length >= RAU_NTDATA_MAX) {
			if (pre_ntbuf == NULL) {
				return 1;
			} else {
				pre_ntbuf->next = NULL;
				break;
			}
		}
		if (pre_ntbuf!= NULL) {
			length = ALIGN(pre_ntbuf->length) + (int)sizeof(struct _ntbuf);
			p = (char*)pre_ntbuf + length;
			if (p >= ntman->bottom) {
				// �̈斖�[�𒴉�
				remain = p - ntman->bottom;
				p = ntman->top + remain;
			}
			if (p != (char*)cur_ntbuf) {
				return	1;
			}
		}
		pre_ntbuf = cur_ntbuf;
		cur_ntbuf = (struct _ntbuf*)cur_ntbuf->next;
	} while(cur_ntbuf != NULL);

	return 0;
}

/*[]----------------------------------------------------------------------[]*
 *|			init_ntbuf()
 *[]----------------------------------------------------------------------[]*
 *|			NT�f�[�^�o�b�t�@�Ǘ������������iNG�Ȃ烊�Z�b�g�j
 *[]----------------------------------------------------------------------[]*
 *|	param	*ntman		: NT�f�[�^�o�b�t�@�Ǘ����
 *|	param	*dirty		: T���v�f�[�^�Q�i��{�f�[�^�`�I���ʒm�f�[�^�܂Łj
 *|						  ��M����܂ł̉�NT�f�[�^�o�b�t�@�Ǘ����
 *|	param	*buff		: NT�f�[�^�o�b�t�@�G���A
 *|	param	size		: NT�f�[�^�o�b�t�@�G���A�T�C�Y
 *[]----------------------------------------------------------------------[]*
 *|	return	0 = normal, 1 = reset buffer
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:	MATSUSHITA(2014.1.31)
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
int		init_ntbuf(struct _ntman *ntman, char *dirty, char *buff, long size)
{
	struct _ntbuf	*ntbuf;
	int length;
	long	remain;
	char	*p;

	if (ntman->top != buff) {
		goto	_reset;
	}
	if (ntman->bottom != (buff+size)) {
		goto	_reset;
	}
	ntbuf = (struct _ntbuf *)ntman->sendtop;
	if (ntbuf != NULL) {
		if (check_que(ntman, ntbuf) != 0) {
			goto	_reset;
		}
	}
	if (dirty != NULL) {
		ntbuf = (struct _ntbuf *)dirty;
		if (check_que(ntman, ntbuf) != 0) {
			goto	_reset;
		}
	}

	if (ntbuf == NULL) {
		ntman->freearea = ntman->top;
	}
	else {
		while(ntbuf->next != NULL) {
			ntbuf = (struct _ntbuf*)ntbuf->next;
		}
		length = ALIGN(ntbuf->length) + (int)sizeof(struct _ntbuf);
		p = (char*)ntbuf + length;
		if (p >= ntman->bottom) {
		// �̈斖�[�𒴉�
			remain = p - ntman->bottom;
			p = ntman->top + remain;
		}
		ntman->freearea = p;
	}
	ntman->sendnow = ntman->sendtop;		// �擪����đ�
	return 0;
_reset:
	reset_ntbuf(ntman, buff, size);
	return 1;
}

/*------------------------------------------------------------------*/
/*	nt data															*/
/*	allocate buffer													*/
/*------------------------------------------------------------------*/
/*
param	
param	
return	�m�ۂł����ꍇ�F�V�����t���[�G���A�擪
*/

struct _ntbuf	*alloc_ntbuf(struct _ntman *ntman, char *dirty, int length)
{
	long	remain;
	char	*free = ntman->freearea;
	char	*datatop;
/* check free area size */
	length = ALIGN(length);	// �A���C�����g����
	length += (int)sizeof(struct _ntbuf);	// ntbuf�T�C�Y�����Z

	// �v�����ꂽ�������m�ۂł��邩�`�F�b�N

	// sendtop�������Ƃ��Adirty�̈ʒu�Ń`�F�b�N����
	// T���v�̂Ƃ��A��A�̃f�[�^�Q����M�I���܂�sendtop�ɓo�^���Ȃ��̂�
	// �擪�f�[�^���Ȃ��Ƃ����A�o�b�t�@���g�p���Ă��邱�Ƃ�����B
	datatop = ntman->sendtop;
	if (datatop == NULL) {
		datatop = dirty;		// �ꎞ�I�ȃf�[�^���m�ۂ���Ă��Ȃ��H
	}
	if (datatop == NULL) {
		// datatop��NULL�@�f�[�^���Ȃ�
		
		// �]��͗̈�̐擪���疖�[
		remain = ntman->bottom - ntman->top;
	}
	else if (free <= datatop) {
//		      f            st
//		|++++++------------+++++++++++++|
//		t                               b
		remain = datatop - free;
	}
	else {
//		      st           f
//		|-----++++++++++++++------------|
//		t                               b
		remain = ntman->bottom - free;
		remain += (datatop - ntman->top);
	}
	if (length > remain)
		return NULL;	// ��������̂Ŋm�ۂł��Ȃ�

/* update freearea */
	// �t���[�G���A�𒷂����ړ�

// ��d�΍�
	datatop = ntman->freearea + length;

	if (datatop >= ntman->bottom) {
// �̈斖�[�𒴉�
//		      st           |----------length---------|f
//		|-----++++++++++++++------------|
//		t                               b
		// ���߂�������̈�擪����t������
		remain = datatop - ntman->bottom;
		datatop = ntman->top + remain;
	}

	if ((datatop+sizeof(struct _ntbuf)) >= ntman->bottom) {
		// ���߂�ꂽ�t���[�G���A�̐擪�Ƀo�b�t�@�����������߂Ȃ��ꍇ�͐擪�ɐݒ肷��
		datatop = ntman->top;
	}
	ntman->freearea = datatop;
	return (struct _ntbuf*)free;
}

/*------------------------------------------------------------------*/
/*	nt data															*/
/*	write data into buffer											*/
/*------------------------------------------------------------------*/
void	write_data(struct _ntman *ntman, struct _ntbuf *ntbuf, char *ntdata)
{
	char	*p = _GET_DATA(ntbuf);
	long	remain = (p + ntbuf->length) - ntman->bottom;
	if (remain > 0) {
		// �������񂾌��ʗ̈斖�[�𒴉߂���
		remain = ntman->bottom - p;//�������݈ʒu����I�[�܂ł��R�s�[����
		memcpy(p, ntdata, remain);
		ntdata += remain;
		remain = ntbuf->length - remain;
		p = ntman->top;
	}
	else {
		// ���߂��Ȃ�
		remain = ntbuf->length;
		if (remain == 0)
			p = ntman->top;		// Length�O�̃f�[�^���������ނƂ��ɂ��łɗ̈斖�[�ɓ��B���Ă������߁A�擪�ɖ߂�
	}
	memcpy(p, ntdata, remain);
}

/*------------------------------------------------------------------*/
/*	nt data															*/
/*	read data from buffer											*/
/*------------------------------------------------------------------*/
void	read_data(struct _ntman *ntman, struct _ntbuf *ntbuf, char *ntdata)
{
	char	*p = _GET_DATA(ntbuf);
	long	remain = (p + ntbuf->length) - ntman->bottom;
	if (remain > 0) {
		remain = ntman->bottom - p;//�ǂݍ��݈ʒu����I�[�܂ł��R�s�[����
		memcpy(ntdata, p, remain);
		ntdata += remain;
		remain = ntbuf->length - remain;
		p = ntman->top;
	}
	else {
		remain = ntbuf->length;
		if (remain == 0)
			p = ntman->top;
	}
	memcpy(ntdata, p, remain);
}

/*------------------------------------------------------------------*/
/*	nt data															*/
/*	add ntbuf to queue												*/
/*------------------------------------------------------------------*/
void	add_que(struct _ntbuf **que, struct _ntbuf *ntbuf)
{
	while(*que != NULL) {
		que = (struct _ntbuf**)(*que);
	}
	*que = ntbuf;
}

/*------------------------------------------------------------------*/
/*	nt data															*/
/*	read data														*/
/*------------------------------------------------------------------*/
int		read_ntbuf(struct _ntman *ntman, char *ntdata, int reset)
{
	struct _ntbuf	*ntbuf;
	if (reset)
		ntman->sendnow = ntman->sendtop;
	ntbuf = (struct _ntbuf*)ntman->sendnow;
	if (ntbuf == NULL)
		return 0;
	ntman->sendnow = (char*)ntbuf->next;
	read_data(ntman, ntbuf, ntdata);
	return (int)(ntbuf->length);
}

/*------------------------------------------------------------------*/
/*	nt data															*/
/*	remove data														*/
/*------------------------------------------------------------------*/
int		remove_ntbuf(struct _ntman *ntman)
{
	if (ntman->sendtop == NULL)
		return 0;
	ntman->sendtop = ntman->sendnow;
	return 1;
}

/*------------------------------------------------------------------*/
/*	nt data															*/
/*	remove one data													*/
/*------------------------------------------------------------------*/
int		remove_data(struct _ntman *ntman)
{
	struct _ntbuf	*ntbuf;

	if (ntman->sendtop == NULL)
		return 0;

	ntbuf = (struct _ntbuf*)ntman->sendtop;
	ntman->sendtop = (char*)ntbuf->next;
	ntman->sendnow = (char*)ntman->sendtop;
// �o�b�t�@���Z�b�g���̃C���[�W����A������sendtop��NULL�ɂȂ�Ƃ��A
// freearea���o�b�t�@�g�b�v�ɖ߂������Ƃ��낾���AT���v�f�[�^�̏ꍇ�A
// ���̃f�[�^�Q����M�r���idirty�Ƀo�b�t�@������j�̂��Ƃ�����̂ŁA
// �ς��邱�Ƃ��ł��Ȃ��B

	return 1;
}

/*------------------------------------------------------------------*/
/*	T-sum data														*/
/*	reset															*/
/*------------------------------------------------------------------*/
void	reset_tsum(char *buff, long size)
{
	RAU_tsum_man.dirty.sendtop = NULL;
	reset_ntbuf(&RAU_tsum_man.man, buff, size);
}

/*------------------------------------------------------------------*/
/*	T-sum data														*/
/*	init															*/
/*------------------------------------------------------------------*/
int		init_tsum(char *buff, long size)
{
	int		sts;

	sts = init_ntbuf(&RAU_tsum_man.man, RAU_tsum_man.dirty.sendtop, buff, size);
	if (sts != 0) {
		RAU_tsum_man.dirty.sendtop = NULL;
	}
	return sts;
}

/*------------------------------------------------------------------*/
/*	T-sum data														*/
/*	check data														*/
/*------------------------------------------------------------------*/
//int		is_tsum(char *ntdata)
//{
//	struct	_ntdata	*nt = (struct _ntdata*)ntdata;
//	if (nt->kind >= 30 && nt->kind < 39)
//		return 1;
//	if (nt->kind == 41)
//		return 2;
//	return 0;
//}

/*------------------------------------------------------------------*/
/*	T-sum data														*/
/*	write data														*/
/*------------------------------------------------------------------*/
int		write_tsum(char *ntdata, int length)
{
	struct _tsumm	*tsumm = &RAU_tsum_man;
//	struct _ntdata	*nt = (struct _ntdata*)ntdata;
	RAU_NTDATA_HEADER	*nt = (RAU_NTDATA_HEADER*)ntdata;
	uchar	endDataId;
	struct _ntbuf	*ntbuf;
	ntbuf = alloc_ntbuf(&tsumm->man, tsumm->dirty.sendtop, length);

	if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		endDataId = 53;
	}
	else {
		endDataId = 41;
	}

	if (ntbuf == NULL) {
		// ����ȏ㏑�����ނ��Ƃ��ł��Ȃ�
		if (tsumm->dirty.sendtop != NULL) {
			// 41��҂��Ă���Œ��Ȃ̂ŁA30�ԑ�J�n�O�̏�Ԃɏ����߂�
		/* flush dirty queue */
			tsumm->man.freearea = tsumm->dirty.sendtop;
			tsumm->dirty.sendtop = NULL;
		}
		return 0;
	}

	ntbuf->next = NULL;
	ntbuf->length = length;
	write_data(&RAU_tsum_man.man, ntbuf, ntdata);
	add_que((struct _ntbuf**)&tsumm->dirty.sendtop, ntbuf);		/* add ntdata to dirty queue */
	if (nt->id4 == endDataId) {
	/* validate dirty queue */
		add_que((struct _ntbuf**)&tsumm->man.sendtop, (struct _ntbuf*)tsumm->dirty.sendtop);
		if (tsumm->man.sendnow == NULL)
			tsumm->man.sendnow = tsumm->man.sendtop;
		tsumm->dirty.sendtop = NULL;
	}
	return 1;
}

/*------------------------------------------------------------------*/
/*	T-sum data														*/
/*	read data														*/
/*------------------------------------------------------------------*/
int		read_tsum(char *ntdata, int reset)
{
	return read_ntbuf(&RAU_tsum_man.man, ntdata, reset);
}

/*------------------------------------------------------------------*/
/*	T-sum data														*/
/*	remove data														*/
/*------------------------------------------------------------------*/
int		remove_tsum(void)
{
	return remove_ntbuf(&RAU_tsum_man.man);
}

/*------------------------------------------------------------------*/
/*	T-sum data														*/
/*	remove one data													*/
/*------------------------------------------------------------------*/
int		remove_tsum_data(void)
{
	return remove_data(&RAU_tsum_man.man);
}

/*------------------------------------------------------------------*/
/*	T-sum data														*/
/*	calculate capacity of use										*/
/*------------------------------------------------------------------*/
long	calc_capacity_tsum(void)
{
	if (RAU_tsum_man.man.sendtop == NULL)
		return 0;

	if (RAU_tsum_man.man.sendtop < RAU_tsum_man.man.freearea) {
		return RAU_tsum_man.man.freearea - RAU_tsum_man.man.sendtop;
	} else {
		return (RAU_tsum_man.man.freearea - RAU_tsum_man.man.top) + (RAU_tsum_man.man.bottom - RAU_tsum_man.man.sendtop);
	}
}

/*------------------------------------------------------------------*/
/*	center terminal infomation data									*/
/*	reset															*/
/*------------------------------------------------------------------*/
void	reset_center_term_info(char *buff)
{
	struct _ctim	*ctim = &RAU_centerterminfo_man;
	struct _ctib *ctib;
	int		i;
	ctim->top = NULL;		/* buffer is dirty */
	ctim->freeq = buff;
	ctim->sendq = NULL;
	ctim->readq = NULL;
	ctib = (struct _ctib*)buff;
	for (i = 0; i < _CENTER_TERM_INFO_MAX-1; i++) {
		buff = (char*)&ctib[1];
		ctib->next = buff;
		ctib->length = 0;
		ctib = (struct _ctib*)buff;
	}
	ctib->next = NULL;
	ctib->length = 0;
	ctim->bottom = ctim->freeq + (sizeof(*ctib)*_CENTER_TERM_INFO_MAX);
	ctim->top = ctim->freeq;
}

/*------------------------------------------------------------------*/
/*	center terminal infomation data									*/
/*	check queue														*/
/*------------------------------------------------------------------*/
int		check_center_term_que(char *que)
{
	struct _ctim	*ctim = &RAU_centerterminfo_man;
	struct _ctib *ctib;
	int		i;
	ctib = (struct _ctib*)que;
	i = 0;
	while(ctib != NULL) {
		if (++i > _CENTER_TERM_INFO_MAX)
			return -1;
		if ((char*)ctib < ctim->top || (char*)ctib > ctim->bottom)
			return -1;
		ctib = (struct _ctib*)ctib->next;
	}
	return i;
}

/*------------------------------------------------------------------*/
/*	center terminal infomation data									*/
/*	search queue													*/
/*------------------------------------------------------------------*/
int		search_center_term_que(char *que, struct _ctib *tar)
{
	struct _ctib	*ctib;
	ctib = (struct _ctib*)que;
	while(ctib != NULL) {
		if (ctib == tar)
			return 1;
		ctib = (struct _ctib*)ctib->next;
	}
	return 0;
}

/*------------------------------------------------------------------*/
/*	center terminal infomation data									*/
/*	init															*/
/*------------------------------------------------------------------*/
int		init_center_term_info(char *buff)
{
	struct _ctim	*ctim = &RAU_centerterminfo_man;
	struct _ctib *ctib;
	int		i, j;
	if (buff != ctim->top)
		goto	_treset;
	i = check_center_term_que(ctim->freeq);
	if (i < 0)
		goto	_treset;
	j = check_center_term_que(ctim->sendq);
	if (j < 0)
		goto	_treset;
	i += j;
	if (i > _CENTER_TERM_INFO_MAX)
		goto	_treset;
	if (i < _CENTER_TERM_INFO_MAX) {
		if (i != (_CENTER_TERM_INFO_MAX-1))
			goto	_treset;
	/* there is one buffer removed from free que */
		ctib = (struct _ctib*)buff;
		for (i = 0; i < _CENTER_TERM_INFO_MAX; i++) {
			if (!search_center_term_que(ctim->freeq, &ctib[i])) {
				if (!search_center_term_que(ctim->sendq, &ctib[i])) {
				/* detect it! */
					ctib = &ctib[i];
					ctib->next = ctim->freeq;
					ctim->freeq = (char*)ctib;
					break;
				}
			}
		}
		if (i >= _CENTER_TERM_INFO_MAX)
			goto	_treset;		/* it is panic!! */
	}
	ctim->readq = ctim->sendq;
	return 0;

_treset:
	reset_center_term_info(buff);
	return 1;
}

/*------------------------------------------------------------------*/
/*	center terminal infomation data									*/
/*	write data														*/
/*------------------------------------------------------------------*/
int		write_center_term_info(char *ntdata, int length)
{
	union {
		uchar	uc[2];
		ushort	us;
	}m1;
	union {
		uchar	uc[2];
		ushort	us;
	}m2;

	union {
		uchar	uc[4];
		ulong	ul;
	}u1;
	union {
		uchar	uc[4];
		ulong	ul;
	}u2;

	struct _ctim	*ctim = &RAU_centerterminfo_man;
	struct _ctib *prev, *next, *tmp;
	struct _ntdata	*nt;
	prev = (struct _ctib*)&ctim->sendq;
	next = (struct _ctib*)ctim->sendq;
	tmp = NULL;

	m2.uc[0] = ((struct _ntdata*)ntdata)->code[0];		// �@��R�[�h
	m2.uc[1] = ((struct _ntdata*)ntdata)->code[1];
	u2.uc[0] = ((struct _ntdata*)ntdata)->term[0];
	u2.uc[1] = ((struct _ntdata*)ntdata)->term[1];
	u2.uc[2] = ((struct _ntdata*)ntdata)->term[2];
	u2.uc[3] = ((struct _ntdata*)ntdata)->term[3];

/* feed to queue end & search buffer that has same terminal no. */
	while(next != NULL) {
		nt = (struct _ntdata*)next->ntdata;

		m1.uc[0] = nt->code[0];
		m1.uc[1] = nt->code[1];
		u1.uc[0] = nt->term[0];
		u1.uc[1] = nt->term[1];
		u1.uc[2] = nt->term[2];
		u1.uc[3] = nt->term[3];

		if (m1.us == m2.us && u1.ul == u2.ul) {
		// �@��R�[�h���A����[��no.�ł��邱��
			tmp = next;
			prev->next = next->next;	/* ctib pointed by tmp is removed from any queue */
		}
		else {
			prev = next;
		}
		next = (struct _ctib*)next->next;
	}
	if (tmp == NULL) {
	/* there is no buffer that has same terminal no., then alloc new buffer */
		tmp = (struct _ctib*)ctim->freeq;
		if (tmp != NULL) {
		// allocate ok!
			ctim->freeq = tmp->next;		/* ctib pointed by tmp is removed from any queue */
		}
		else {
		// remove oldest data from sendq and use it!
			tmp = (struct _ctib*)ctim->sendq;
			if (tmp == NULL) {
				return 0;		/* no data */
			}
			ctim->sendq = tmp->next;		/* ctib is removed from any queue */
			ctim->readq = ctim->sendq;
		}
	}
	tmp->next = NULL;
	tmp->length = length;
	memcpy(tmp->ntdata, ntdata, length);
	prev->next = (char*)tmp;		/* add end of send queue */
	ctim->readq = ctim->sendq;
	return 1;
}

/*------------------------------------------------------------------*/
/*	center terminal infomation data									*/
/*	read data														*/
/*------------------------------------------------------------------*/
int		read_center_term_info(char *ntdata)
{
	struct _ctim	*ctim = &RAU_centerterminfo_man;
	struct _ctib *ctib;
	ctib = (struct _ctib*)ctim->readq;
	if (ctib == NULL)
		return 0;	/* no data */
	ctim->readq = ctib->next;
	memcpy(ntdata, ctib->ntdata, ctib->length);
	return (int)ctib->length;
}

/*------------------------------------------------------------------*/
/*	center terminal infomation data									*/
/*	reset read pointer												*/
/*------------------------------------------------------------------*/
void	reset_center_term_readq(void)
{
	struct _ctim	*ctim = &RAU_centerterminfo_man;
	ctim->readq = ctim->sendq;			// read_cash_info�ł��ꂽreadq��߂�
}

/*------------------------------------------------------------------*/
/*	center terminal infomation data									*/
/*	remove 1st data from queue										*/
/*------------------------------------------------------------------*/
int		remove_center_term_info(void)
{
	struct _ctim	*ctim = &RAU_centerterminfo_man;
	struct _ctib *ctib;
	ctib = (struct _ctib*)ctim->sendq;
	if (ctib == NULL)
		return 0;		/* no data */
	ctim->sendq = ctib->next;		/* ctib is removed from any queue */
	ctim->readq = ctim->sendq;
	ctib->next = ctim->freeq;
	ctim->freeq = (char*)ctib;	/* add top of free queue */
	return 1;
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
int		count_tsum_data(struct _ntman *ntman, uchar count_tsum)
{
	struct _ntbuf	*ntbuf;
	int i;
	RAU_NTDATA_HEADER	*p;

	ntbuf = (struct _ntbuf*)ntman->sendtop;
	i = 0;

	if (ntbuf == NULL) {
		return 0;
	}

	do {
		p = (RAU_NTDATA_HEADER*)_GET_DATA(ntbuf);
		if (count_tsum == 1) {
			if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
				if (p->id4 == 53) {
					i++;
				}
			}
			else {
				if (p->id4 == 41) {
					i++;
				}
			}
		} else {
			i++;
		}
		ntbuf = (struct _ntbuf*)ntbuf->next;
	} while(ntbuf != NULL);

	return i;
}


/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
int		seek_tsum_data( void )
{
	struct _ntbuf	*ntbuf;
	int i;
	RAU_NTDATA_HEADER	*p;

	ntbuf = (struct _ntbuf*)RAU_tsum_man.man.sendtop;
	i = 0;

	if (ntbuf == NULL) {
		return 0;
	}

	do {
		p = (RAU_NTDATA_HEADER*)_GET_DATA(ntbuf);
		i++;
		if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
			if (p->id4 == 53) {
				break;
			}
		}
		else {
			if (p->id4 == 41) {
				break;
			}
		}
		ntbuf = (struct _ntbuf*)ntbuf->next;
	} while(ntbuf != NULL);

	return i;
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAU_reset_send_tsum(void) {
	RAU_tsum_man.man.sendnow = RAU_tsum_man.man.sendtop;
}

////////////////////////////////////////////////////////////////////////////////

/****************************************************************************/
/*	�֐���		=  RAU_data_table_init()									*/
/*	�@�\		=  �f�[�^�e�[�u���G���A���t									*/
/*	�T���@�@�@�@=�@�f�[�^�e�[�u���̃T�C�Y�ɂ��擪�|�C���^�����߂�			*/
/****************************************************************************/
void	RAU_data_table_init( void )
{
	int i;
	uchar ucDataFullProc = 0;

	rau_table_crear_flag = -1;

	RAU_table_top[RAU_IN_PARKING_TABLE]				= 0;
	RAU_table_top[RAU_OUT_PARKING_TABLE]			= 0;
	RAU_table_top[RAU_PAID_DATA22_TABLE]			= 0;
	RAU_table_top[RAU_PAID_DATA23_TABLE]			= 0;
	RAU_table_top[RAU_ERROR_TABLE]					= 0;
	RAU_table_top[RAU_ALARM_TABLE]					= 0;
	RAU_table_top[RAU_MONITOR_TABLE]				= 0;
	RAU_table_top[RAU_OPE_MONITOR_TABLE]			= 0;
	RAU_table_top[RAU_COIN_TOTAL_TABLE]				= 0;
	RAU_table_top[RAU_MONEY_TORAL_TABLE]			= 0;
	RAU_table_top[RAU_PARKING_NUM_TABLE]			= 0;
	RAU_table_top[RAU_AREA_NUM_TABLE]				= 0;
	RAU_table_top[RAU_TURI_MANAGE_TABLE]			= 0;
	RAU_table_top[RAU_TOTAL_TABLE]					= RAU_table_data.total;
	RAU_table_top[RAU_GTOTAL_TABLE]					= RAU_table_data.total;
	RAU_table_top[RAU_MONEY_MANAGE_TABLE]			= 0;
	RAU_table_top[RAU_REMOTE_MONITOR_TABLE]			= 0;
	RAU_table_top[RAU_CENTER_TERM_INFO_TABLE]		= RAU_table_data.center_term_info;
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	RAU_table_top[RAU_LONG_PARKING_INFO_TABLE]		= 0;
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	RAU_table_top[RAU_SEND_THROUGH_DATA_TABLE]		= RAU_table_data.send_through_data;
	RAU_table_top[RAU_RECEIVE_THROUGH_DATA_TABLE]	= RAU_table_data.receive_through_data;

	memset( &RAU_table_data, 0x00, sizeof(RAU_table_data) );

	for (i = 0; i < RAU_TABLE_MAX; i++) {
		rau_data_table_info[i].pc_AreaTop				= RAU_table_top[i];
		rau_data_table_info[i].ul_AreaSize				= init_info[i].size;
		rau_data_table_info[i].ul_NearFullMaximum		= rau_data_table_info[i].ul_AreaSize * 80 / 100;
		rau_data_table_info[i].ul_NearFullMinimum		= rau_data_table_info[i].ul_AreaSize *  0 / 100;	// <-#003
		rau_data_table_info[i].i_NearFullStatus			= RAU_NEARFULL_CANCEL;	// #001->
		rau_data_table_info[i].i_NearFullStatusBefore	= RAU_NEARFULL_CANCEL;	// <-#001
		rau_data_table_info[i].pc_ReadPoint				= rau_data_table_info[i].pc_AreaTop;
		rau_data_table_info[i].pc_WritePoint			= rau_data_table_info[i].pc_AreaTop;
		rau_data_table_info[i].ui_DataCount				= 0;
		rau_data_table_info[i].ui_SendDataCount			= 0;	// #002
		rau_data_table_info[i].c_DataFullProc			= 0;
		if(i == RAU_TOTAL_TABLE) {
			if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
				rau_data_table_info[i].uc_DataId		= 53;
			}
			else {
				rau_data_table_info[i].uc_DataId		= 41;
			}
		}
		else if(i == RAU_GTOTAL_TABLE) {
			rau_data_table_info[i].uc_DataId			= 53;
		}
		else {
			rau_data_table_info[i].uc_DataId			= init_info[i].id;
		}
	};

	rau_data_table_info[RAU_ERROR_TABLE].c_DataFullProc			= ucDataFullProc;	// �G���[�e�[�u���̃f�[�^�������@���Z�b�g
	rau_data_table_info[RAU_ALARM_TABLE].c_DataFullProc			= ucDataFullProc;	// �A���[���e�[�u���̃f�[�^�������@���Z�b�g
	rau_data_table_info[RAU_MONITOR_TABLE].c_DataFullProc		= ucDataFullProc;	// ���j�^�[�e�[�u���̃f�[�^�������@���Z�b�g
	rau_data_table_info[RAU_OPE_MONITOR_TABLE].c_DataFullProc	= ucDataFullProc;	// ���샂�j�^�[�e�[�u���̃f�[�^�������@���Z�b�g
	rau_data_table_info[RAU_REMOTE_MONITOR_TABLE].c_DataFullProc	= ucDataFullProc;	// ���u�Ď��f�[�^�e�[�u���̃f�[�^�������@���Z�b�g
// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
	// �o�b�t�@�t�����ŌÃf�[�^
	rau_data_table_info[RAU_LONG_PARKING_INFO_TABLE].c_DataFullProc	= ucDataFullProc;	// �������ԏ��f�[�^�e�[�u���̃f�[�^�������@���Z�b�g
// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)

	reset_tsum((char*)RAU_table_data.total, RAU_TOTAL_SIZE);
	reset_center_term_info((char*)RAU_table_data.center_term_info);
}

/****************************************************************************/
/*	�֐���		=  data_rau_init()											*/
/*	�@�\			=  �ʐM�ɂ��C�j�V�����C�Y								*/
/*	�T���@�@�@�@	=�@�I�[���N���A											*/
/*	����			=  ����													*/
/****************************************************************************/
void	data_rau_init( void )
{
	/* �e�[�u���f�[�^�G���A���� */
	RAU_data_table_init();

	RAU_ui_txdlength_h_keep = 0;
	RAU_ui_txdlength_h_keep2 = 0;
	RAU_uc_retrycnt_h = 0;
	RAU_uc_retrycnt_h2 = 0;
	RAU_uc_rcvretry_cnt = 0;
	RAU_uc_retrycnt_reconnect_h = 0;

	RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);	// ���M�V�[�P���X�O(�g�n�r�s�ւ̃e�[�u�����M�ҋ@���)
	RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// ��M�V�[�P���X�O(�ʏ���)
	RAUhost_ClearSendRequeat();

	return;
}


/*[]----------------------------------------------------------------------[]*
 *| 	RAUdata_DelRcvNtData
 *[]----------------------------------------------------------------------[]*
 *|	�z�X�g�����M����NT-NET�f�[�^���폜
 *[]----------------------------------------------------------------------[]*
 *|	param	: none
 *[]----------------------------------------------------------------------[]*
 *|	return	: none
 *[]----------------------------------------------------------------------[]*
 *| Author	:	machida kei
 *| Date	:	2005-08-23
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAUdata_DelRcvNtData(void)
{
	RAU_DATA_TABLE_INFO *table = &rau_data_table_info[RAU_RECEIVE_THROUGH_DATA_TABLE];
	
	table->pc_ReadPoint	 = table->pc_WritePoint;
	table->ui_DataCount	 = 0;
}

/*[]----------------------------------------------------------------------[]*
 *| 	RAUdata_GetRcvNtDataBlkNum
 *[]----------------------------------------------------------------------[]*
 *|	�z�X�g�����M����NT-NET�f�[�^�̃u���b�N�����擾
 *[]----------------------------------------------------------------------[]*
 *|	param	: none
 *[]----------------------------------------------------------------------[]*
 *|	return	: �z�X�g�����M����NT-NET�f�[�^�̃u���b�N��(0�`26:-1�̓f�[�^����)
 *[]----------------------------------------------------------------------[]*
 *| Author	:	machida kei
 *| Date	:	2005-08-23
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
int		RAUdata_GetRcvNtDataBlkNum(void)
{
	RAU_DATA_TABLE_INFO *table = &rau_data_table_info[RAU_RECEIVE_THROUGH_DATA_TABLE];
	uchar	*rptr, *tblend;
	int		blknum;
	union {
		uchar	c[2];
		ushort	w;
	}ulen;
	union {
		T_SEND_NT_DATA		sndntdata;
		RAU_NTDATA_HEADER	ntdatahdr;
	}uwork;
	
	tblend = &table->pc_AreaTop[table->ul_AreaSize];
	
	blknum = -1;
	if (table->ui_DataCount > 0) {
		// CRC���`�F�b�N
		uwork.sndntdata.nt_data_start = NULL;
		if (!RAUdata_GetNtData(table, &uwork.sndntdata)) {
			// �e�[�u���f�[�^��CRC���ُ�
			RAUhost_SetError(RAU_ERR_DATA_CRC_ERROR);	// �G���[�R�[�h�O�T(�o�b�t�@�f�[�^�b�q�b�G���[)

			RAUdata_DelRcvNtData();		// �f�[�^�͍폜
			return blknum;
		}
		// ��1�u���b�N�f�[�^�|�C���^�擾
		rptr = RAU_raudata_ReadTable(table, table->pc_ReadPoint, (uchar*)&uwork, 2);
		// �u���b�N���Z�o
		blknum = 0;
		while (1) {
			blknum++;
			// �u���b�N�f�[�^�w�b�_�擾
			RAU_raudata_ReadTable(table, rptr, (uchar*)&uwork.ntdatahdr, sizeof(RAU_NTDATA_HEADER));
			if (uwork.ntdatahdr.id2 == 0x01) {
				break;		// �ŏI�u���b�N���o
			}
			if (blknum >= RAU_RECEIVE_NTBLKNUM_MAX) {
				RAUdata_DelRcvNtData();
				return -1;	// �ő�u���b�N���I�[�o�[(���̃p�X�͒ʂ�Ȃ��͂�)
			}
			// ���̃u���b�N�̐擪��
			ulen.c[0]	= uwork.ntdatahdr.data_len[0];
			ulen.c[1]	= uwork.ntdatahdr.data_len[1];
			rptr = RAU_ptrSeek(rptr, ulen.w, table->pc_AreaTop, tblend);
		}
	}
	
	return blknum;
}

/*[]----------------------------------------------------------------------[]*
 *| 	RAUdata_GetRcvNtBlkData
 *[]----------------------------------------------------------------------[]*
 *|	�z�X�g�����M����NT-NET�f�[�^���擾
 *[]----------------------------------------------------------------------[]*
 *|	param	: data	=�擾�f�[�^�����ݐ�̈�						<OUT>
 *|			  blkno	=�擾�������u���b�NNo. (1�`26)
 *[]----------------------------------------------------------------------[]*
 *|	return	: data�ɃZ�b�g���ꂽ�f�[�^��(0=�f�[�^����)
 *[]----------------------------------------------------------------------[]*
 *| Author	:	machida kei
 *| Date	:	2005-08-23
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	RAUdata_GetRcvNtBlkData(uchar *data, uchar blkno)
{
	RAU_DATA_TABLE_INFO *table = &rau_data_table_info[RAU_RECEIVE_THROUGH_DATA_TABLE];
	uchar	i, id2;
	ushort	dummy;
	uchar	*rptr, *tblend;
	union {
		uchar	c[2];
		ushort	w;
	}ulen;
	
	if (blkno <= 0 || RAU_RECEIVE_NTBLKNUM_MAX < blkno) {
		return 0;
	}
	
	tblend = &table->pc_AreaTop[table->ul_AreaSize];
	
	ulen.w = 0;

	if (table->ui_DataCount > 0) {
		// ��1�u���b�N�f�[�^�|�C���^�擾
		rptr = RAU_raudata_ReadTable(table, table->pc_ReadPoint, (uchar*)&dummy, sizeof(dummy));
		// �Y���u���b�N����
		id2 = 0;
		for (i = 0; i < blkno; i++) {
			if (id2 != 0x00) {
				break;		// �ŏI�u���b�N���o
			}
			// ���̃u���b�N�̐擪��
			rptr = RAU_ptrSeek(rptr, ulen.w, table->pc_AreaTop, tblend);
			// �u���b�N�f�[�^�w�b�_�擾
			RAU_raudata_ReadTable(table, rptr, data, sizeof(RAU_NTDATA_HEADER));
			ulen.c[0] = ((RAU_NTDATA_HEADER*)data)->data_len[0];
			ulen.c[1] = ((RAU_NTDATA_HEADER*)data)->data_len[1];
			id2 = ((RAU_NTDATA_HEADER*)data)->id2;
		}
		if (i == blkno) {
			RAU_raudata_ReadTable(table, rptr, data, ulen.w);	// �Y���u���b�N�ǂݏo��
		}
		else {
			// �Y���u���b�N�s��
			ulen.w = 0;
		}
	}
	
	return ulen.w;
}


/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	RAUdata_Id2Index(uchar id) {
	int i;
	uchar ret;

	ret = RAU_SEND_THROUGH_DATA_TABLE;	// ���M�p�X���[�f�[�^�̃C���f�b�N�X

	for (i = 0; i < RAU_ID_BUFF_MAX; i++) {
		if (RAU_id_buff[i].id == id) {
			ret = RAU_id_buff[i].index;
			break;
		}
	}
	return ret;
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	NT�f�[�^��ڕW�̃e�[�u���Ɋi�[����
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	RAUdata_WriteNtData(uchar *pData, ushort wDataLen, RAU_DATA_TABLE_INFO *pBuffInfo) {

	ushort	wWriteLen1;	// �܂�Ԃ��āA�R�s�[���s���Ƃ��̒����O��
	ushort	wWriteLen2;	// �܂�Ԃ��āA�R�s�[���s���Ƃ��̒����㔼

	if (RAUdata_CheckCapacity(pBuffInfo, wDataLen) == FALSE) {
		// �̈�̎c��e�ʂ��`�F�b�N
		// �s���ŏ������߂Ȃ�

			// �o�b�t�@�t���G���[����
				
			// �j�A�t���X�e�[�^�X�ύX

			if (pBuffInfo->c_DataFullProc == 1) {
				// �ŐV�㏑�� 
				if (pBuffInfo->ui_SendDataCount < pBuffInfo->ui_DataCount) {
					// �S�f�[�^�����M�ΏۂɂȂ��Ă��Ȃ���΁A
					// �������߂�ʂ��󂭂܂ŁA�ŐV�f�[�^���폜�B
					while (1) {
						RAUdata_DelNtData(pBuffInfo, 0);
						if (RAUdata_CheckCapacity(pBuffInfo, wDataLen) == TRUE) {
							break;
						}
						if (pBuffInfo->ui_SendDataCount >= pBuffInfo->ui_DataCount) {
							// ���M���f�[�^������̂ŁA�폜�ł��Ȃ��
							return FALSE;
						}
						if (pBuffInfo->ui_DataCount == 0) {
							// �f�[�^�������O�ɂȂ��Ă��������߂Ȃ��̂ŁA�o�b�t�@�t���B
							return FALSE;
						}
					}
				} else {
					// ���M���f�[�^������̂ŁA�폜�ł��Ȃ��
					return FALSE;
				}
			} else if (pBuffInfo->c_DataFullProc == 2) {
				// �ŌÏ㏑��
				if (pBuffInfo->ui_SendDataCount == 0) {
					// ���M���f�[�^�������Ƃ��
					// �������߂�ʂ��󂭂܂ŁA�ŌÃf�[�^���폜�B
					while (1) {
						RAUdata_DelNtData(pBuffInfo, 1);
						if (RAUdata_CheckCapacity(pBuffInfo, wDataLen) == TRUE) {
							break;
						}
						if (pBuffInfo->ui_DataCount == 0) {
							// �f�[�^�������O�ɂȂ��Ă��������߂Ȃ��̂ŁA�o�b�t�@�t���B
							return FALSE;
						}
					}
				} else {
					// ���M���f�[�^������̂ŁA�폜�ł��Ȃ��
					return FALSE;
				}
			} else {
				// �������݋֎~
				// �o�b�t�@�t���G���[�ŏ������݋֎~�B�A�[�N�u���b�N����M����82h
				return FALSE;
			}
		
	} else {
	}

	// �������݌�̍ŐV�̏������݈ʒu�ƃo�b�t�@�̏I�[���r����
	if ((pBuffInfo->pc_WritePoint + wDataLen) < (pBuffInfo->pc_AreaTop + pBuffInfo->ul_AreaSize) ) {
		// �������ރf�[�^�̏I�[�iCRC�܂Ŋ܂�ł���j���A�̈�͈͓̔��Ɏ��܂��Ă���
		// �f�[�^����������
		memcpy(pBuffInfo->pc_WritePoint, pData, wDataLen);

		// �f�[�^�������݈ʒu���X�V
		pBuffInfo->pc_WritePoint += wDataLen;

	} else {
		// �f�[�^���I�[�ʒu���܂����Ƃ��B
		// �܂�Ԃ��ʒu�����
		wWriteLen2 = (pBuffInfo->pc_WritePoint + wDataLen) - (pBuffInfo->pc_AreaTop + pBuffInfo->ul_AreaSize);
		wWriteLen1 = wDataLen - wWriteLen2;

		// �O������������
		memcpy(pBuffInfo->pc_WritePoint, pData, wWriteLen1);

		// �㔼���������݁i�̈�擪���珑�����݁j
		memcpy(pBuffInfo->pc_AreaTop, pData + wWriteLen1, wWriteLen2);

		// �f�[�^�������݈ʒu���X�V
		pBuffInfo->pc_WritePoint = pBuffInfo->pc_AreaTop + wWriteLen2;

	}

	// �f�[�^�����C���N�������g
	pBuffInfo->ui_DataCount++;

	return TRUE;
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	�ڕW�̃o�b�t�@����w�蒷�f�[�^��ǂݏo���
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAUdata_ReadNtData(uchar *pData, ushort wReadLen, RAU_DATA_TABLE_INFO *pBuffInfo, T_SEND_NT_DATA *pSendNtDataInf) {

	ushort	wReadLen1;	// �܂�Ԃ��āA�R�s�[���s���Ƃ��̒����O��
	ushort	wReadLen2;	// �܂�Ԃ��āA�R�s�[���s���Ƃ��̒����㔼

	// �������݈ʒu�{�������ݗʁ@�Ɓ@�o�b�t�@�I�[�@���r
	if ((pSendNtDataInf->nt_data_next + wReadLen) < (pBuffInfo->pc_AreaTop + pBuffInfo->ul_AreaSize) ) {
		// �����Ȃ��Ƃ��B�������o�b�t�@�ɃR�s�[
		memcpy(pData, pSendNtDataInf->nt_data_next, wReadLen);

		// ���񏑂����݈ʒu���R�s�[�����������i�߂�
		pSendNtDataInf->nt_data_next += wReadLen;

	} else {
		// ������Ƃ��B
		// �܂�Ԃ��ʒu���Z�o
		wReadLen2 = (pSendNtDataInf->nt_data_next + wReadLen) - (pBuffInfo->pc_AreaTop + pBuffInfo->ul_AreaSize);
		wReadLen1 = wReadLen - wReadLen2;

		// �O�����R�s�[
		memcpy(pData, pSendNtDataInf->nt_data_next, wReadLen1);

		// �㔼���R�s�[
		memcpy(pData + wReadLen1, pBuffInfo->pc_AreaTop , wReadLen2);

		// ���񏑂����݈ʒu���R�s�[�����������i�߂�
		pSendNtDataInf->nt_data_next = pBuffInfo->pc_AreaTop + wReadLen2;

	}
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	���݂̃o�b�t�@�̐擪����N�Ԗڂ̃f�[�^�̃A�h���X��Ԃ�
 *[]----------------------------------------------------------------------[]*
 *|	patam	pBuffInfo	�o�b�t�@���\����
 *|	patam	wCount		�擾�������f�[�^�̃A�h���X
 *[]----------------------------------------------------------------------[]*
 *|	return	N�Ԗڂ̃f�[�^�̐擪�A�h���X�B������Ȃ��ꍇ��NULL
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar* RAUdata_GetNtDataAdrr(RAU_DATA_TABLE_INFO *pBuffInfo, ushort wCount) {

	int i;
	uchar *pNtData;
	uchar *pBuffEnd;

	union {
		uchar	uc[2];
		ushort	us;
	}u;


	if (pBuffInfo->ui_DataCount < wCount) {
		// �o�b�t�@�ɂ��܂��Ă��鐔��葽���A�v�������ꂽ��NULL��Ԃ��
		return (uchar *)NULL;
	}

	// �擪�f�[�^�̃A�h���X��ݒ肷��
	pNtData = pBuffInfo->pc_ReadPoint;

	// ���[�̈ʒu�@=�@�擪�̈ʒu�@�{�@�T�C�Y
	pBuffEnd = pBuffInfo->pc_AreaTop + pBuffInfo->ul_AreaSize;

	for (i = 0; i < wCount; i++) {

		// �f�[�^�̒��������߂�()
		u.uc[0] = pNtData[0];
		if (&pNtData[1] < pBuffEnd) {	// ������2�o�C�g�ڂ����[���z���Ă��邩�`�F�b�N
			// �����Ă��Ȃ��̂ł��̂܂܎g�p�B
			u.uc[1] = pNtData[1];
		} else {
			// �����Ă���̂ŁA�o�b�t�@�̐擪��������2�o�C�g�ځB
			u.uc[1] = pBuffInfo->pc_AreaTop[0];
		}

		// ���̈ʒu�@=�@�ʒu�{�����i�Q�j�{CRC�̒����i�Q�j
		pNtData += u.us + 2;

		if (pNtData >= pBuffEnd) {
			// ���̈ʒu�@���@���[�̈ʒu�i�܂�Ԃ��K�v�j
			// ���̈ʒu�@���@�擪�̈ʒu�@�{�@�i���̈ʒu�@�|�@���[�̈ʒu�j
			pNtData = pBuffInfo->pc_AreaTop + (pNtData - pBuffEnd);
		}
/*
		else {
			// ���̈ʒu�@���@���[�̈ʒu�i�܂�Ԃ��s�v�j
		}
*/
	}
	return pNtData;
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	�ړI�̃o�b�t�@��NT�f�[�^��1���폜����
 *[]----------------------------------------------------------------------[]*
 *|	patam	pBuffInfo	�o�b�t�@���\����
 *|	patam	position	�폜�ʒu
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAUdata_DelNtData(RAU_DATA_TABLE_INFO *pBuffInfo, uchar cPosition) {

	uchar	endDataId;

//	�W�v�f�[�^�͈ꊇ�ɍ폜����K�v�����邽�߁ARAUdata_DeleteTsumGroup()���g�p���Ă��Ă����ɗ��邱�Ƃ͂���܂���
	if(prm_get(COM_PRM, S_NTN, 121, 1, 1) != 0) {
		endDataId = 53;
	}
	else {
		endDataId = 41;
	}

	if (pBuffInfo->uc_DataId == endDataId) {
		remove_tsum_data();
	} else if (pBuffInfo->uc_DataId == 65) {
		remove_center_term_info();
	} else {
		if (cPosition) {
		// �O�������
			if (pBuffInfo->ui_DataCount > 1) {
			// �c��f�[�^���͕���
				// �폜��̎��̐擪�ʒu���擾
				pBuffInfo->pc_ReadPoint = RAUdata_GetNtDataAdrr(pBuffInfo, 1);
				// �f�[�^�����f�N�������g
				pBuffInfo->ui_DataCount--;
			
			} else {
			// �Ō�̂ЂƂ�
				// ���[�h�|�C���g�@���@���C�g�|�C���g
				pBuffInfo->pc_ReadPoint = pBuffInfo->pc_WritePoint;
				// �f�[�^���@���@�O
				pBuffInfo->ui_DataCount = 0;
			}
		
		} else {
		// ��납�����
			if (pBuffInfo->ui_DataCount > 1) {
			// �c��f�[�^���͕���
				// �폜��̎��̐擪�ʒu���擾
				pBuffInfo->pc_WritePoint = RAUdata_GetNtDataAdrr(pBuffInfo, pBuffInfo->ui_DataCount - 1);
				// �f�[�^�����f�N�������g
				pBuffInfo->ui_DataCount--;
			
			} else {
			// �Ō�̂ЂƂ�
				// ���C�g�|�C���g�@���@���[�h�|�C���g
				pBuffInfo->pc_WritePoint = pBuffInfo->pc_ReadPoint;
				// �f�[�^���@���@�O
				pBuffInfo->ui_DataCount = 0;
			}
		}
	}

}

/*[]----------------------------------------------------------------------[]*
 *|             RAUdata_GetNtDataNextAdrr()
 *[]----------------------------------------------------------------------[]*
 *|	����NT�f�[�^�̐擪�A�h���X���擾����
 *[]----------------------------------------------------------------------[]*
 *|	patam	
 *[]----------------------------------------------------------------------[]*
 *|	return	
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar* RAUdata_GetNtDataNextAdrr(RAU_DATA_TABLE_INFO *pBuffInfo, T_SEND_NT_DATA *pSendNtDataInf) {

	uchar *pNtData;

	// NT�f�[�^���̈�̏I�[���܂������Ă��邩�m�F�
	if (pSendNtDataInf->nt_data_start < pSendNtDataInf->nt_data_end) {
		// �܂������Ă��Ȃ��
		
		// �f�[�^�̂͂ݏo���ʂ����߂�
		switch ((pBuffInfo->pc_AreaTop + pBuffInfo->ul_AreaSize) - pSendNtDataInf->nt_data_end) {
		case 0:
			// CRC���̈�擪�ɂɂ���̂ŁA���̃f�[�^�͐擪����2�i�߂��ꏊ����B
			pNtData = pBuffInfo->pc_AreaTop + 2;
			break;
		case 1:
			// CRC���܂������Ă���̂ŁA���̃f�[�^�͐擪����1�i�߂��ꏊ����B
			pNtData = pBuffInfo->pc_AreaTop + 1;
			break;
		case 2:
			// CRC���̈���Ɏ��܂�̂ŁA���̃f�[�^�͗̈�擪����B
			pNtData = pBuffInfo->pc_AreaTop;
			break;
		default:
			// ���̃f�[�^�͂��̃f�[�^�̌ォ��n�܂�
			pNtData = pSendNtDataInf->nt_data_end + 2;
			break;
		}
	} else {
		// �܂������Ă���B
		pNtData = pSendNtDataInf->nt_data_end + 2;
		
	}

	return pNtData;
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	NT�f�[�^�����擾����
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL RAUdata_GetNtData(RAU_DATA_TABLE_INFO *pBuffInfo, T_SEND_NT_DATA *pSendNtDataInf) {

	uchar *pBuffEnd;
	uchar crc_cal[2];
	uchar crc_ram[2];
	union {
		uchar	uc[2];
		ushort	us;
	}u;

	if (pBuffInfo->pc_ReadPoint == pBuffInfo->pc_WritePoint) {
		return FALSE;
	}

	// ���M�pNT�f�[�^���NULL�łȂ���΁A��x�f�[�^���擾���Ă���͂��Ȃ̂ŁA���̃f�[�^��T���
	if (pSendNtDataInf->nt_data_start == NULL) {
		// �f�[�^��NULL
//		pSendNtDataInf->nt_data_start = pSendNtDataInf->nt_data_next = pBuffInfo->pc_ReadPoint;
		pSendNtDataInf->nt_data_len = pBuffInfo->pc_ReadPoint;
		
	} else {
		// �f�[�^��NULL�łȂ�
//		pSendNtDataInf->nt_data_start  = pSendNtDataInf->nt_data_next = RAUdata_GetNtDataNextAdrr(pBuffInfo, pSendNtDataInf);
		pSendNtDataInf->nt_data_len = RAUdata_GetNtDataNextAdrr(pBuffInfo, pSendNtDataInf);
	}

	// NT�f�[�^�I���ʒu�͌v�Z���ċ��߂�

	// ���[�̈ʒu�@=�@�擪�̈ʒu�@�{�@�T�C�Y
	pBuffEnd = pBuffInfo->pc_AreaTop + pBuffInfo->ul_AreaSize;

	u.uc[0] = pSendNtDataInf->nt_data_len[0];
	if (&pSendNtDataInf->nt_data_len[1] < pBuffEnd) {	// ������2�o�C�g�ڂ��o�b�t�@�̖��[���z���Ă��邩�`�F�b�N
		// �����Ă��Ȃ��̂ł��̂܂܎g�p�B
		u.uc[1] = pSendNtDataInf->nt_data_len[1];
	} else {
		// �����Ă���̂ŁA�o�b�t�@�̐擪��������2�o�C�g�ځB
		u.uc[1] = pBuffInfo->pc_AreaTop[0];
	}

	if (u.us < 12) {
		return FALSE;
	}


	if (&pSendNtDataInf->nt_data_len[1] >= pBuffEnd) {	// �f�[�^�̐擪���o�b�t�@�̖��[���z���Ă��邩�`�F�b�N
		// �������I�[���܂����ł���
		// �f�[�^�{�̂͗̈�擪����1�o�C�g��
		pSendNtDataInf->nt_data_start = &pBuffInfo->pc_AreaTop[1];
	} else if (&pSendNtDataInf->nt_data_len[2] >= pBuffEnd) {
		// �������I�[�ŏI����Ă���
		// �̈�擪���f�[�^�̐擪
		pSendNtDataInf->nt_data_start = pBuffInfo->pc_AreaTop;
	} else {
		pSendNtDataInf->nt_data_start = &pSendNtDataInf->nt_data_len[2];
	}

	pSendNtDataInf->nt_data_next = pSendNtDataInf->nt_data_start;

	pSendNtDataInf->nt_data_end = pSendNtDataInf->nt_data_len + u.us;

	// �f�[�^���o�b�t�@�̏I�[�𒴂��Ă���B
	if (pSendNtDataInf->nt_data_end >= pBuffEnd) {
		// ���̈ʒu�@���@���[�̈ʒu�i�܂�Ԃ��K�v�j
		// ���̈ʒu�@���@�擪�̈ʒu�@�{�@�i���̈ʒu�@�|�@���[�̈ʒu�j
		pSendNtDataInf->nt_data_end = pBuffInfo->pc_AreaTop + (pSendNtDataInf->nt_data_end - pBuffEnd);
	}

	// �Ώۃf�[�^��CRC�v�Z���s���
	if (pSendNtDataInf->nt_data_start < pSendNtDataInf->nt_data_end) {
		RAU_CrcCcitt(pSendNtDataInf->nt_data_end - pSendNtDataInf->nt_data_start
					, 0
					, pSendNtDataInf->nt_data_start
					, (uchar*)NULL
					, crc_cal
					, 1);
	} else {
		

		RAU_CrcCcitt(pBuffEnd - pSendNtDataInf->nt_data_start
					, pSendNtDataInf->nt_data_end - pBuffInfo->pc_AreaTop
					, pSendNtDataInf->nt_data_start
					, pBuffInfo->pc_AreaTop
					, crc_cal
					, 1);
	}
	
	// �Ώۃf�[�^��CRC���擾����
//	crc_ram[0] = pSendNtDataInf->nt_data_end[0];

	if (&pSendNtDataInf->nt_data_end[0] < pBuffEnd) {
		crc_ram[0] = pSendNtDataInf->nt_data_end[0];
		if (&pSendNtDataInf->nt_data_end[1] < pBuffEnd) {
			crc_ram[1] = pSendNtDataInf->nt_data_end[1];
			pSendNtDataInf->nt_data_crc = &pSendNtDataInf->nt_data_end[2];
		} else {
			crc_ram[1] = pBuffInfo->pc_AreaTop[0];
			pSendNtDataInf->nt_data_crc = &pBuffInfo->pc_AreaTop[1];
		}
	} else {
		crc_ram[0] = pBuffInfo->pc_AreaTop[0];
		crc_ram[1] = pBuffInfo->pc_AreaTop[1];
		pSendNtDataInf->nt_data_crc = &pBuffInfo->pc_AreaTop[2];
	}

	// CRC�`�F�b�N���s���
	if (crc_cal[0] != crc_ram[0]
	 || crc_cal[1] != crc_ram[1]) {
		return FALSE;
	} else {
		return TRUE;
	}
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL RAUdata_GetTsumData(T_SEND_NT_DATA *pSendNtDataInf, uchar *pTsumBuff) {

	int ret;

	ret = read_tsum((char*)pTsumBuff, 0);

	if (ret != 0) {
		pSendNtDataInf->nt_data_start = pSendNtDataInf->nt_data_next = pTsumBuff;
		pSendNtDataInf->nt_data_end = &pTsumBuff[ret];
		return TRUE;
	} else {
		return FALSE;
	}
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	���M�Ώۃf�[�^�̐����i�X���[�f�[�^�j
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAUdata_GetNtDataInfoThrough(T_SEND_DATA_ID_INFO *pSendDataInfo) {

	if( rau_data_table_info[RAU_SEND_THROUGH_DATA_TABLE].ui_DataCount ){
		pSendDataInfo[RAU_SEND_THROUGH_DATA_TABLE].send_req = 1;
		pSendDataInfo[RAU_SEND_THROUGH_DATA_TABLE].send_data_count = rau_data_table_info[RAU_SEND_THROUGH_DATA_TABLE].ui_DataCount;
		rau_data_table_info[RAU_SEND_THROUGH_DATA_TABLE].ui_SendDataCount = rau_data_table_info[RAU_SEND_THROUGH_DATA_TABLE].ui_DataCount;
		pSendDataInfo[RAU_SEND_THROUGH_DATA_TABLE].send_complete_count = 0;
		pSendDataInfo[RAU_SEND_THROUGH_DATA_TABLE].fill_up_data_count = 0;
		pSendDataInfo[RAU_SEND_THROUGH_DATA_TABLE].crc_err_data_count = 0;
		pSendDataInfo[RAU_SEND_THROUGH_DATA_TABLE].pbuff_info = &rau_data_table_info[RAU_SEND_THROUGH_DATA_TABLE];
	}
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	���M�Ώۃf�[�^�̐����i�e�[�u���f�[�^�j
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAUdata_GetNtDataInfo(T_SEND_DATA_ID_INFO *pSendDataInfo, RAU_SEND_REQUEST *pReqInfo, uchar ucSendReq) {

	int		i;
	ushort	logCount;
	ushort	sendCnt = 0;	// �S�f�[�^���M�v���ɂ��f�[�^���M�𔻒f���邽�߂̑��M�f�[�^���J�E���^

	if( ucSendReq ){					// �h�a�v�܂��̓z�X�g����̑��M�v���f�[�^����M���Ă���B
// MH810100(S) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//		if( pReqInfo->in_parking ){		// 20:���Ƀf�[�^�̑��M��v��
//			pSendDataInfo[RAU_IN_PARKING_TABLE].send_req = 1;
//			if(RauConfig.id20_21_mask_flg == 0){		// ���Ƀf�[�^�̑��M�}�X�N�Ȃ�
//				pSendDataInfo[RAU_IN_PARKING_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_ENTER, eLOG_TARGET_REMOTE);
//			}
//			else{
//				pSendDataInfo[RAU_IN_PARKING_TABLE].send_data_count = 0;
//			}
//			rau_data_table_info[RAU_IN_PARKING_TABLE].ui_SendDataCount = pSendDataInfo[RAU_IN_PARKING_TABLE].send_data_count;
//			pSendDataInfo[RAU_IN_PARKING_TABLE].send_complete_count = 0;
//			pSendDataInfo[RAU_IN_PARKING_TABLE].fill_up_data_count = 0;
//			pSendDataInfo[RAU_IN_PARKING_TABLE].crc_err_data_count = 0;
//			pSendDataInfo[RAU_IN_PARKING_TABLE].pbuff_info = &rau_data_table_info[RAU_IN_PARKING_TABLE];
//			sendCnt++;
//		}
// MH810100(E) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)

// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		if(pReqInfo->out_parking ){		// 21:�o�Ƀf�[�^�̑��M��v��
//			pSendDataInfo[RAU_OUT_PARKING_TABLE].send_req = 1;
//			if(RauConfig.id20_21_mask_flg == 0){		// �o�Ƀf�[�^�̑��M�}�X�N�Ȃ�
//				// �o�Ƀf�[�^�͂O���f�[�^�̂ݑ��M
//				pSendDataInfo[RAU_OUT_PARKING_TABLE].send_data_count = 0;
//			}
//			else{
//				pSendDataInfo[RAU_OUT_PARKING_TABLE].send_data_count = 0;
//			}
//			rau_data_table_info[RAU_OUT_PARKING_TABLE].ui_SendDataCount = pSendDataInfo[RAU_OUT_PARKING_TABLE].send_data_count;
//			pSendDataInfo[RAU_OUT_PARKING_TABLE].send_complete_count = 0;
//			pSendDataInfo[RAU_OUT_PARKING_TABLE].fill_up_data_count = 0;
//			pSendDataInfo[RAU_OUT_PARKING_TABLE].crc_err_data_count = 0;
//			pSendDataInfo[RAU_OUT_PARKING_TABLE].pbuff_info = &rau_data_table_info[RAU_OUT_PARKING_TABLE];
//			sendCnt++;
//		}
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

		if( pReqInfo->paid_data ){		// 22,23:���Z�f�[�^�̑��M��v��
			pSendDataInfo[RAU_PAID_DATA22_TABLE].send_req = 1;
			if(RauConfig.id22_23_mask_flg == 0){		// ���Z�f�[�^�̑��M�}�X�N�Ȃ�
				// ���O���Z�f�[�^�͂O���f�[�^�̂ݑ��M
				pSendDataInfo[RAU_PAID_DATA22_TABLE].send_data_count = 0;
			}
			else{
				pSendDataInfo[RAU_PAID_DATA22_TABLE].send_data_count = 0;
			}
			rau_data_table_info[RAU_PAID_DATA22_TABLE].ui_SendDataCount = pSendDataInfo[RAU_PAID_DATA22_TABLE].send_data_count;
			pSendDataInfo[RAU_PAID_DATA22_TABLE].send_complete_count = 0;
			pSendDataInfo[RAU_PAID_DATA22_TABLE].fill_up_data_count = 0;
			pSendDataInfo[RAU_PAID_DATA22_TABLE].crc_err_data_count = 0;
			pSendDataInfo[RAU_PAID_DATA22_TABLE].pbuff_info = &rau_data_table_info[RAU_PAID_DATA22_TABLE];

			pSendDataInfo[RAU_PAID_DATA23_TABLE].send_req = 1;
			if(RauConfig.id22_23_mask_flg == 0){		// ���Z�f�[�^�̑��M�}�X�N�Ȃ�
				pSendDataInfo[RAU_PAID_DATA23_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_PAYMENT, eLOG_TARGET_REMOTE);
			}
			else{
				pSendDataInfo[RAU_PAID_DATA23_TABLE].send_data_count = 0;
			}
			rau_data_table_info[RAU_PAID_DATA23_TABLE].ui_SendDataCount = pSendDataInfo[RAU_PAID_DATA23_TABLE].send_data_count;
			pSendDataInfo[RAU_PAID_DATA23_TABLE].send_complete_count = 0;
			pSendDataInfo[RAU_PAID_DATA23_TABLE].fill_up_data_count = 0;
			pSendDataInfo[RAU_PAID_DATA23_TABLE].crc_err_data_count = 0;
			pSendDataInfo[RAU_PAID_DATA23_TABLE].pbuff_info = &rau_data_table_info[RAU_PAID_DATA23_TABLE];
			sendCnt++;
		}

		if( pReqInfo->error ){		// 120:�G���[�f�[�^�̑��M��v��
			pSendDataInfo[RAU_ERROR_TABLE].send_req = 1;
			if( (RauConfig.id120_mask_flg == 0) &&
				(RauConfig.error_send_level != 9) ){		// �G���[�f�[�^�̑��M�}�X�N�Ȃ�
				pSendDataInfo[RAU_ERROR_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_ERROR, eLOG_TARGET_REMOTE);
			}
			else{
				pSendDataInfo[RAU_ERROR_TABLE].send_data_count = 0;
			}
			rau_data_table_info[RAU_ERROR_TABLE].ui_SendDataCount = pSendDataInfo[RAU_ERROR_TABLE].send_data_count;
			pSendDataInfo[RAU_ERROR_TABLE].send_complete_count = 0;
			pSendDataInfo[RAU_ERROR_TABLE].fill_up_data_count = 0;
			pSendDataInfo[RAU_ERROR_TABLE].crc_err_data_count = 0;
			pSendDataInfo[RAU_ERROR_TABLE].pbuff_info = &rau_data_table_info[RAU_ERROR_TABLE];
			sendCnt++;
		}

		if( pReqInfo->alarm ){		// 121:�A���[���f�[�^�̑��M��v��
			pSendDataInfo[RAU_ALARM_TABLE].send_req = 1;
			if( (RauConfig.alarm_send_level != 9) &&
				(RauConfig.id121_mask_flg == 0) ){			// �A���[���f�[�^�̑��M�}�X�N�Ȃ�
				pSendDataInfo[RAU_ALARM_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_ALARM, eLOG_TARGET_REMOTE);
			}
			else{
				pSendDataInfo[RAU_ALARM_TABLE].send_data_count = 0;
			}
			rau_data_table_info[RAU_ALARM_TABLE].ui_SendDataCount = pSendDataInfo[RAU_ALARM_TABLE].send_data_count;
			pSendDataInfo[RAU_ALARM_TABLE].send_complete_count = 0;
			pSendDataInfo[RAU_ALARM_TABLE].fill_up_data_count = 0;
			pSendDataInfo[RAU_ALARM_TABLE].crc_err_data_count = 0;
			pSendDataInfo[RAU_ALARM_TABLE].pbuff_info = &rau_data_table_info[RAU_ALARM_TABLE];
			sendCnt++;
		}

		if( pReqInfo->monitor ){	// 122:���j�^�f�[�^�̑��M��v��
			pSendDataInfo[RAU_MONITOR_TABLE].send_req = 1;
			if( (RauConfig.monitor_send_level != 9) &&
				(RauConfig.id122_mask_flg == 0) ){			// ���j�^�f�[�^�̑��M�}�X�N�Ȃ�
				pSendDataInfo[RAU_MONITOR_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_MONITOR, eLOG_TARGET_REMOTE);
			}
			else{
				pSendDataInfo[RAU_MONITOR_TABLE].send_data_count = 0;
			}
			rau_data_table_info[RAU_MONITOR_TABLE].ui_SendDataCount = pSendDataInfo[RAU_MONITOR_TABLE].send_data_count;
			pSendDataInfo[RAU_MONITOR_TABLE].send_complete_count = 0;
			pSendDataInfo[RAU_MONITOR_TABLE].fill_up_data_count = 0;
			pSendDataInfo[RAU_MONITOR_TABLE].crc_err_data_count = 0;
			pSendDataInfo[RAU_MONITOR_TABLE].pbuff_info = &rau_data_table_info[RAU_MONITOR_TABLE];
			sendCnt++;
		}

		if( pReqInfo->ope_monitor ){	// 123:���샂�j�^�f�[�^�̑��M��v��
			pSendDataInfo[RAU_OPE_MONITOR_TABLE].send_req = 1;
			if( (RauConfig.opemonitor_send_level != 9) &&
				(RauConfig.id123_mask_flg == 0) ){			// ���샂�j�^�f�[�^�̑��M�}�X�N�Ȃ�
				pSendDataInfo[RAU_OPE_MONITOR_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_OPERATE, eLOG_TARGET_REMOTE);
			}
			else{
				pSendDataInfo[RAU_OPE_MONITOR_TABLE].send_data_count = 0;
			}
			rau_data_table_info[RAU_OPE_MONITOR_TABLE].ui_SendDataCount = pSendDataInfo[RAU_OPE_MONITOR_TABLE].send_data_count;
			pSendDataInfo[RAU_OPE_MONITOR_TABLE].send_complete_count = 0;
			pSendDataInfo[RAU_OPE_MONITOR_TABLE].fill_up_data_count = 0;
			pSendDataInfo[RAU_OPE_MONITOR_TABLE].crc_err_data_count = 0;
			pSendDataInfo[RAU_OPE_MONITOR_TABLE].pbuff_info = &rau_data_table_info[RAU_OPE_MONITOR_TABLE];
			sendCnt++;
		}

		if( pReqInfo->coin_total ){	// 131:�R�C�����ɏW�v���v�f�[�^�̑��M��v��
			pSendDataInfo[RAU_COIN_TOTAL_TABLE].send_req = 1;
			if(RauConfig.id131_mask_flg == 0){				// �R�C�����ɏW�v���v�f�[�^�̑��M�}�X�N�Ȃ�
				pSendDataInfo[RAU_COIN_TOTAL_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_COINBOX, eLOG_TARGET_REMOTE);
			}
			else{
				pSendDataInfo[RAU_COIN_TOTAL_TABLE].send_data_count = 0;
			}
			rau_data_table_info[RAU_COIN_TOTAL_TABLE].ui_SendDataCount = pSendDataInfo[RAU_COIN_TOTAL_TABLE].send_data_count;
			pSendDataInfo[RAU_COIN_TOTAL_TABLE].send_complete_count = 0;
			pSendDataInfo[RAU_COIN_TOTAL_TABLE].fill_up_data_count = 0;
			pSendDataInfo[RAU_COIN_TOTAL_TABLE].crc_err_data_count = 0;
			pSendDataInfo[RAU_COIN_TOTAL_TABLE].pbuff_info = &rau_data_table_info[RAU_COIN_TOTAL_TABLE];
			sendCnt++;
		}

		if( pReqInfo->money_toral ){	// 133:�������ɏW�v���v�f�[�^�̑��M��v��
			pSendDataInfo[RAU_MONEY_TORAL_TABLE].send_req = 1;
			if(RauConfig.id133_mask_flg == 0){				// �������ɏW�v���v�f�[�^�̑��M�}�X�N�Ȃ�
				pSendDataInfo[RAU_MONEY_TORAL_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_NOTEBOX, eLOG_TARGET_REMOTE);
			}
			else{
				pSendDataInfo[RAU_MONEY_TORAL_TABLE].send_data_count = 0;
			}
			rau_data_table_info[RAU_MONEY_TORAL_TABLE].ui_SendDataCount = pSendDataInfo[RAU_MONEY_TORAL_TABLE].send_data_count;
			pSendDataInfo[RAU_MONEY_TORAL_TABLE].send_complete_count = 0;
			pSendDataInfo[RAU_MONEY_TORAL_TABLE].fill_up_data_count = 0;
			pSendDataInfo[RAU_MONEY_TORAL_TABLE].crc_err_data_count = 0;
			pSendDataInfo[RAU_MONEY_TORAL_TABLE].pbuff_info = &rau_data_table_info[RAU_MONEY_TORAL_TABLE];
			sendCnt++;
		}

// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//		if( pReqInfo->parking_num ){	// 236:���ԑ䐔�f�[�^�̑��M��v��
//			pSendDataInfo[RAU_PARKING_NUM_TABLE].send_req = 1;
//			if(RauConfig.id236_mask_flg == 0){				// ���ԑ䐔�f�[�^�̑��M�}�X�N�Ȃ�
//				pSendDataInfo[RAU_PARKING_NUM_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_PARKING, eLOG_TARGET_REMOTE);
//			}
//			else{
//				pSendDataInfo[RAU_PARKING_NUM_TABLE].send_data_count = 0;
//			}
//			rau_data_table_info[RAU_PARKING_NUM_TABLE].ui_SendDataCount = pSendDataInfo[RAU_PARKING_NUM_TABLE].send_data_count;
//			pSendDataInfo[RAU_PARKING_NUM_TABLE].send_complete_count = 0;
//			pSendDataInfo[RAU_PARKING_NUM_TABLE].fill_up_data_count = 0;
//			pSendDataInfo[RAU_PARKING_NUM_TABLE].crc_err_data_count = 0;
//			pSendDataInfo[RAU_PARKING_NUM_TABLE].pbuff_info = &rau_data_table_info[RAU_PARKING_NUM_TABLE];
//			sendCnt++;
//		}
//
//		if( pReqInfo->area_num ){	// 237:���䐔�E���ԃf�[�^�̑��M��v��
//			pSendDataInfo[RAU_AREA_NUM_TABLE].send_req = 0;
//			pSendDataInfo[RAU_AREA_NUM_TABLE].send_data_count = 0;
//			rau_data_table_info[RAU_AREA_NUM_TABLE].ui_SendDataCount = pSendDataInfo[RAU_AREA_NUM_TABLE].send_data_count;
//			pSendDataInfo[RAU_AREA_NUM_TABLE].send_complete_count = 0;
//			pSendDataInfo[RAU_AREA_NUM_TABLE].fill_up_data_count = 0;
//			pSendDataInfo[RAU_AREA_NUM_TABLE].crc_err_data_count = 0;
//			pSendDataInfo[RAU_AREA_NUM_TABLE].pbuff_info = &rau_data_table_info[RAU_AREA_NUM_TABLE];
//			// ���䐔�E���ԃf�[�^�̓f�[�^���M�v���ŗv������邱�Ƃ͖������߁A�J�E���g�A�b�v���Ȃ�
//		}
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

		if( pReqInfo->total ){	// 30�`38,41:�s���v�W�v�f�[�^�̑��M��v��
			logCount = Ope_Log_UnreadCountGet(eLOG_TTOTAL, eLOG_TARGET_REMOTE);
			if(logCount > 0){
				pSendDataInfo[RAU_TOTAL_TABLE].send_req = 1;
				pSendDataInfo[RAU_TOTAL_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].pbuff_info = &rau_data_table_info[RAU_TOTAL_TABLE];
				// ���O����W�v�f�[�^���擾���Atsum�f�[�^�Ɋi�[
				if(RauConfig.serverTypeSendTsum == 0){				// 34-0026�C 0:���M���Ȃ� 1:���M����
					for(i=0; i<logCount; i++){
						Ope_Log_TargetDataGet( eLOG_TTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
					}
					pSendDataInfo[RAU_TOTAL_TABLE].send_data_count = 0;
				}
			}
			logCount = Ope_Log_UnreadCountGet(eLOG_GTTOTAL, eLOG_TARGET_REMOTE);
			if(logCount > 0){
				// ID42,43,45,46,49,53:�f�s���v�W�v�f�[�^�̑��M��v��
// MH322914 (s) kasiyama 2016/07/07 ����T���v�f�[�^���đ����ꑱ���A�Ȍ��T���v�����M����Ȃ�(���ʉ��PNo.1250)
//				pSendDataInfo[RAU_GTOTAL_TABLE].send_req = 1;
//				pSendDataInfo[RAU_GTOTAL_TABLE].send_complete_count = 0;
//				pSendDataInfo[RAU_GTOTAL_TABLE].fill_up_data_count = 0;
//				pSendDataInfo[RAU_GTOTAL_TABLE].crc_err_data_count = 0;
//				pSendDataInfo[RAU_GTOTAL_TABLE].pbuff_info = &rau_data_table_info[RAU_GTOTAL_TABLE];
//				if(RauConfig.serverTypeSendTsum == 0){				// 34-0026�C 0:���M���Ȃ� 1:���M����
//					for(i=0; i<logCount; i++){
//						Ope_Log_TargetDataGet( eLOG_GTTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
//					}
//					pSendDataInfo[RAU_GTOTAL_TABLE].send_data_count = 0;
//				}
				if(prm_get(COM_PRM,S_NTN,121,1,1) == 0) {
					// ParkRing�̏ꍇ��GT���v�𑗐M���Ȃ�
					pSendDataInfo[RAU_GTOTAL_TABLE].send_req = 0;
					pSendDataInfo[RAU_GTOTAL_TABLE].send_complete_count = 0;
					pSendDataInfo[RAU_GTOTAL_TABLE].fill_up_data_count = 0;
					pSendDataInfo[RAU_GTOTAL_TABLE].crc_err_data_count = 0;
					pSendDataInfo[RAU_GTOTAL_TABLE].pbuff_info = &rau_data_table_info[RAU_GTOTAL_TABLE];
					for(i=0; i<logCount; i++){
						Ope_Log_TargetDataGet( eLOG_GTTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
					}
					pSendDataInfo[RAU_GTOTAL_TABLE].send_data_count = 0;
				}
				else {
					pSendDataInfo[RAU_GTOTAL_TABLE].send_req = 1;
					pSendDataInfo[RAU_GTOTAL_TABLE].send_complete_count = 0;
					pSendDataInfo[RAU_GTOTAL_TABLE].fill_up_data_count = 0;
					pSendDataInfo[RAU_GTOTAL_TABLE].crc_err_data_count = 0;
					pSendDataInfo[RAU_GTOTAL_TABLE].pbuff_info = &rau_data_table_info[RAU_GTOTAL_TABLE];
					if(RauConfig.serverTypeSendTsum == 0){				// 34-0026�C 0:���M���Ȃ� 1:���M����
						for(i=0; i<logCount; i++){
							Ope_Log_TargetDataGet( eLOG_GTTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
						}
						pSendDataInfo[RAU_GTOTAL_TABLE].send_data_count = 0;
					}
				}
// MH322914 (e) kasiyama 2016/07/07 ����T���v�f�[�^���đ����ꑱ���A�Ȍ��T���v�����M����Ȃ�(���ʉ��PNo.1250)
			}
			if((pSendDataInfo[RAU_TOTAL_TABLE].send_req == 0) &&
				(pSendDataInfo[RAU_GTOTAL_TABLE].send_req == 0)) {
				// T���v�y��GT���v�̌�����0���̏ꍇ�AT���v��0���f�[�^�𑗂�
				pSendDataInfo[RAU_TOTAL_TABLE].send_req = 1;
				pSendDataInfo[RAU_TOTAL_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].pbuff_info = &rau_data_table_info[RAU_TOTAL_TABLE];
			}
			sendCnt++;
		}

		if( pReqInfo->money_manage ){	// 126:���K�Ǘ��f�[�^�̑��M��v��
			pSendDataInfo[RAU_MONEY_MANAGE_TABLE].send_req = 1;
			if(RauConfig.id126_mask_flg == 0){				// ���K�Ǘ��f�[�^�̑��M�}�X�N�Ȃ�
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);
			}
			else{
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].send_data_count = 0;
			}
			pSendDataInfo[RAU_MONEY_MANAGE_TABLE].send_complete_count = 0;
			pSendDataInfo[RAU_MONEY_MANAGE_TABLE].fill_up_data_count = 0;
			pSendDataInfo[RAU_MONEY_MANAGE_TABLE].crc_err_data_count = 0;
			pSendDataInfo[RAU_MONEY_MANAGE_TABLE].pbuff_info = &rau_data_table_info[RAU_MONEY_MANAGE_TABLE];
			sendCnt++;
		}

		if( pReqInfo->turi_manage ){	// 135:�ޑK�Ǘ��W�v�f�[�^�̑��M��v��
			pSendDataInfo[RAU_TURI_MANAGE_TABLE].send_req = 1;
			if(RauConfig.id135_mask_flg == 0){				// �ޑK�Ǘ��W�v�f�[�^�̑��M�}�X�N�Ȃ�
				pSendDataInfo[RAU_TURI_MANAGE_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_MNYMNG_SRAM, eLOG_TARGET_REMOTE);
			}
			else{
				pSendDataInfo[RAU_TURI_MANAGE_TABLE].send_data_count = 0;
			}
			pSendDataInfo[RAU_TURI_MANAGE_TABLE].send_complete_count = 0;
			pSendDataInfo[RAU_TURI_MANAGE_TABLE].fill_up_data_count = 0;
			pSendDataInfo[RAU_TURI_MANAGE_TABLE].crc_err_data_count = 0;
			pSendDataInfo[RAU_TURI_MANAGE_TABLE].pbuff_info = &rau_data_table_info[RAU_TURI_MANAGE_TABLE];
			sendCnt++;
		}

		if( pReqInfo->rmon ){			// 125:���u�Ď��f�[�^�̑��M��v��
			pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_req = 1;
			if( (RauConfig.rmon_send_level != 9) &&
				(RauConfig.id125_mask_flg == 0) ){			// ���u�Ď��f�[�^�̑��M�}�X�N�Ȃ�
				pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_REMOTE_MONITOR, eLOG_TARGET_REMOTE);
			}
			else{
				pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_data_count = 0;
			}
			rau_data_table_info[RAU_REMOTE_MONITOR_TABLE].ui_SendDataCount = pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_data_count;
			pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_complete_count = 0;
			pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].fill_up_data_count = 0;
			pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].crc_err_data_count = 0;
			pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].pbuff_info = &rau_data_table_info[RAU_REMOTE_MONITOR_TABLE];
			sendCnt++;
		}

// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//		if( pReqInfo->long_park ){		// 61:�������ԏ�ԃf�[�^�̑��M��v��
//			pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_req = 1;
//			if(RauConfig.id61_mask_flg == 0){			// �������ԏ�ԃf�[�^�̑��M�}�X�N�Ȃ�
//				pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_LONGPARK_PWEB, eLOG_TARGET_REMOTE);
//			}
//			else{
//				pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_data_count = 0;
//			}
//			rau_data_table_info[RAU_LONG_PARKING_INFO_TABLE].ui_SendDataCount = pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_data_count;
//			pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_complete_count = 0;
//			pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].fill_up_data_count = 0;
//			pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].crc_err_data_count = 0;
//			pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].pbuff_info = &rau_data_table_info[RAU_LONG_PARKING_INFO_TABLE];
//			sendCnt++;
//		}
//// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

		if( sendCnt == REQUEST_TABLE_MAX - 1 && 	// 65:�Z���^�[�p�[�����f�[�^�𑗐M�i�S�f�[�^���M�v�����j
													// ���䐔�E���ԃf�[�^�̓f�[�^���M�v���ŗv������邱�Ƃ͖�������-1����
			check_center_term_que(RAU_centerterminfo_man.sendq) > 0 ) {
			pSendDataInfo[RAU_CENTER_TERM_INFO_TABLE].send_req = 1;
			pSendDataInfo[RAU_CENTER_TERM_INFO_TABLE].send_data_count = check_center_term_que(RAU_centerterminfo_man.sendq);
			pSendDataInfo[RAU_CENTER_TERM_INFO_TABLE].send_complete_count = 0;
			pSendDataInfo[RAU_CENTER_TERM_INFO_TABLE].fill_up_data_count = 0;
			pSendDataInfo[RAU_CENTER_TERM_INFO_TABLE].crc_err_data_count = 0;
			pSendDataInfo[RAU_CENTER_TERM_INFO_TABLE].pbuff_info = &rau_data_table_info[RAU_CENTER_TERM_INFO_TABLE];
		}
	} else {	// ���M�v���f�[�^�̎�M�Ȃ�
		if( RauConfig.Dpa_proc_knd == 1 ){	// �c���o�������敪"(0:�펞�ʐM�Ȃ�, 1:�펞�ʐM����)
			for( i = 0; i < RAU_TOTAL_TABLE; i++ ){
// MH810100(S)
				// �A�����l�����Ĉ�xtaskchg
				taskchg(IDLETSKNO);
// MH810100(E)
				pSendDataInfo[i].send_data_count = 0;
				switch(i){
				case RAU_IN_PARKING_TABLE:			// ���Ƀf�[�^
// MH810100(S) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//					if(RauConfig.id20_21_mask_flg == 0){
//						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_ENTER, eLOG_TARGET_REMOTE);
//					}
// MH810100(E) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
					break;
				case RAU_OUT_PARKING_TABLE:			// �o�Ƀf�[�^
					break;
				case RAU_PAID_DATA22_TABLE:			// ���Z�f�[�^(���O)
					if(RauConfig.id22_23_mask_flg == 0){
						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_PAYMENT, eLOG_TARGET_REMOTE);
					}
					break;
				case RAU_PAID_DATA23_TABLE:			// ���Z�f�[�^(�o��)
					if(RauConfig.id22_23_mask_flg == 0){
						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_PAYMENT, eLOG_TARGET_REMOTE);
					}
					break;
				case RAU_ERROR_TABLE:				// �G���[�f�[�^
					if( (RauConfig.id120_mask_flg == 0) &&
						(RauConfig.error_send_level != 9) ){
						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_ERROR, eLOG_TARGET_REMOTE);
					}
					break;
				case RAU_ALARM_TABLE:				// �A���[���f�[�^
					if( (RauConfig.id121_mask_flg == 0) &&
						(RauConfig.alarm_send_level != 9) ){
						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_ALARM, eLOG_TARGET_REMOTE);
					}
					break;
				case RAU_MONITOR_TABLE:				// ���j�^�[�f�[�^
					if( (RauConfig.id122_mask_flg == 0) &&
						(RauConfig.monitor_send_level != 9) ){
						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_MONITOR, eLOG_TARGET_REMOTE);
					}
					break;
				case RAU_OPE_MONITOR_TABLE:			// ���샂�j�^�[�f�[�^
					if( (RauConfig.id123_mask_flg == 0) &&
						(RauConfig.opemonitor_send_level != 9) ){
						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_OPERATE, eLOG_TARGET_REMOTE);
					}
					break;
				case RAU_COIN_TOTAL_TABLE:			// �R�C�����ɏW�v�f�[�^
					if(RauConfig.id131_mask_flg == 0){
						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_COINBOX, eLOG_TARGET_REMOTE);
					}
					break;
				case RAU_MONEY_TORAL_TABLE:			// �������ɏW�v�f�[�^
					if(RauConfig.id133_mask_flg == 0){
						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_NOTEBOX, eLOG_TARGET_REMOTE);
					}
					break;
				case RAU_PARKING_NUM_TABLE:			// ���ԑ䐔�E���ԃf�[�^
// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//					if(RauConfig.id236_mask_flg == 0){
//						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_PARKING, eLOG_TARGET_REMOTE);
//					}
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
					break;
				case RAU_AREA_NUM_TABLE:			// ���䐔�E���ԃf�[�^
					break;
				case RAU_TURI_MANAGE_TABLE:			// �ޑK�Ǘ��W�v�f�[�^
					if(RauConfig.id135_mask_flg == 0){
						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_MNYMNG_SRAM, eLOG_TARGET_REMOTE);
					}
					break;
				default:
					break;
				}
				if( pSendDataInfo[i].send_data_count ){	// �s���v�W�v�Ƌ��K�Ǘ����������ׂẴe�[�u���Ƀf�[�^���������
					pSendDataInfo[i].send_req = 2;			// ���M�ΏۂƂ���B
					rau_data_table_info[i].ui_SendDataCount = pSendDataInfo[i].send_data_count;
					pSendDataInfo[i].send_complete_count = 0;
					pSendDataInfo[i].fill_up_data_count = 0;
					pSendDataInfo[i].crc_err_data_count = 0;
					pSendDataInfo[i].pbuff_info = &rau_data_table_info[i];
				}
			}

			logCount = Ope_Log_UnreadCountGet(eLOG_TTOTAL, eLOG_TARGET_REMOTE);
			if((logCount > 0) &&
				(RauConfig.id30_41_mask_flg == 0) &&			// �s���v�W�v�f�[�^
				(RauConfig.serverTypeSendTsum != 0)) {		 	// 34-0026�C 0:���M���Ȃ� 1:���M����
				pSendDataInfo[RAU_TOTAL_TABLE].send_req = 2;
				pSendDataInfo[RAU_TOTAL_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].pbuff_info = &rau_data_table_info[RAU_TOTAL_TABLE];
				// ���O����W�v�f�[�^���擾���Atsum�f�[�^�Ɋi�[
			}
			if(logCount > 0) {
				if(RauConfig.serverTypeSendTsum == 0){				// 34-0026�C 0:���M���Ȃ� 1:���M����
					for(i=0; i<logCount; i++){
						Ope_Log_TargetDataGet( eLOG_TTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
					}
				}
			}
// MH810100(S)
			// �A�����l�����Ĉ�xtaskchg
			taskchg(IDLETSKNO);
// MH810100(E)

			logCount = Ope_Log_UnreadCountGet(eLOG_GTTOTAL, eLOG_TARGET_REMOTE);
			if((logCount > 0) &&
				(RauConfig.id30_41_mask_flg == 0) &&			// �f�s���v�W�v�f�[�^
				(RauConfig.serverTypeSendTsum != 0)) {		 	// 34-0026�C 0:���M���Ȃ� 1:���M����
				pSendDataInfo[RAU_GTOTAL_TABLE].send_req = 2;
				pSendDataInfo[RAU_GTOTAL_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_GTOTAL_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_GTOTAL_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_GTOTAL_TABLE].pbuff_info = &rau_data_table_info[RAU_GTOTAL_TABLE];
			}
			if(logCount > 0) {
// MH322914 (s) kasiyama 2016/07/07 ����T���v�f�[�^���đ����ꑱ���A�Ȍ��T���v�����M����Ȃ�(���ʉ��PNo.1250)
//				if(RauConfig.serverTypeSendTsum == 0){				// 34-0026�C 0:���M���Ȃ� 1:���M����
				if(prm_get(COM_PRM,S_NTN,121,1,1) == 0) {
					// ParkRing�̏ꍇ��GT���v�𑗐M���Ȃ�
					pSendDataInfo[RAU_GTOTAL_TABLE].send_req = 0;
				}
				if((RauConfig.serverTypeSendTsum == 0) ||				// 34-0026�C 0:���M���Ȃ� 1:���M����
					(prm_get(COM_PRM,S_NTN,121,1,1) == 0)) {
					// ParkRing�̏ꍇ��GT���v�𑗐M���Ȃ�
// MH322914 (e) kasiyama 2016/07/07 ����T���v�f�[�^���đ����ꑱ���A�Ȍ��T���v�����M����Ȃ�(���ʉ��PNo.1250)
					for(i=0; i<logCount; i++){
						Ope_Log_TargetDataGet( eLOG_GTTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
					}
				}
			}

			if ((Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE) > 0) &&
				(RauConfig.id126_mask_flg == 0)){				// ���K�Ǘ��f�[�^
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].send_req = 2;
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].pbuff_info = &rau_data_table_info[RAU_MONEY_MANAGE_TABLE];
			}

			if( (RauConfig.id125_mask_flg == 0) &&		// ���u�Ď��f�[�^
				(RauConfig.rmon_send_level != 9) ){
				pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_REMOTE_MONITOR, eLOG_TARGET_REMOTE);
				if( pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_data_count ){	// �s���v�W�v�Ƌ��K�Ǘ����������ׂẴe�[�u���Ƀf�[�^���������
					pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_req = 2;			// ���M�ΏۂƂ���B
					rau_data_table_info[RAU_REMOTE_MONITOR_TABLE].ui_SendDataCount = pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_data_count;
					pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_complete_count = 0;
					pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].fill_up_data_count = 0;
					pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].crc_err_data_count = 0;
					pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].pbuff_info = &rau_data_table_info[RAU_REMOTE_MONITOR_TABLE];
				}
			}

// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//			if (RauConfig.id61_mask_flg == 0){			// �������ԏ�ԃf�[�^
//				pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_LONGPARK_PWEB, eLOG_TARGET_REMOTE);
//				if( pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_data_count ){
//					pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_req = 2;
//					rau_data_table_info[RAU_LONG_PARKING_INFO_TABLE].ui_SendDataCount = pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_data_count;
//					pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_complete_count = 0;
//					pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].fill_up_data_count = 0;
//					pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].crc_err_data_count = 0;
//					pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].pbuff_info = &rau_data_table_info[RAU_LONG_PARKING_INFO_TABLE];
//				}
//			}
//// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

		} else {	// �펞�ʐM�Ȃ� �� �ʏ펞�ڑ��m�F
// MH810100(S) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//			if ((RauConfig.id20_21_tel_flg == 1) &&
//				(RauConfig.id20_21_mask_flg == 0) &&
//				(Ope_Log_UnreadCountGet(eLOG_ENTER, eLOG_TARGET_REMOTE) > 0)){	// �h�a�v�p�����[�^"����/�o�Ƀf�[�^�������M���M�s�d�k�ԍ�"('0':�������M����, '1':�������M���Ȃ�)
//				pSendDataInfo[RAU_IN_PARKING_TABLE].send_req = 2;
//				pSendDataInfo[RAU_IN_PARKING_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_ENTER, eLOG_TARGET_REMOTE);
//				rau_data_table_info[RAU_IN_PARKING_TABLE].ui_SendDataCount = pSendDataInfo[RAU_IN_PARKING_TABLE].send_data_count;
//				pSendDataInfo[RAU_IN_PARKING_TABLE].send_complete_count = 0;
//				pSendDataInfo[RAU_IN_PARKING_TABLE].fill_up_data_count = 0;
//				pSendDataInfo[RAU_IN_PARKING_TABLE].crc_err_data_count = 0;
//				pSendDataInfo[RAU_IN_PARKING_TABLE].pbuff_info = &rau_data_table_info[RAU_IN_PARKING_TABLE];
//			} 
// MH810100(E) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)

// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//			if ((RauConfig.id20_21_tel_flg == 1) &&
//				(RauConfig.id20_21_mask_flg == 0) &&
//				(rau_data_table_info[RAU_OUT_PARKING_TABLE].ui_DataCount)) {
//				pSendDataInfo[RAU_OUT_PARKING_TABLE].send_req = 2;
//				pSendDataInfo[RAU_OUT_PARKING_TABLE].send_data_count = 0;
//				rau_data_table_info[RAU_OUT_PARKING_TABLE].ui_SendDataCount = pSendDataInfo[RAU_OUT_PARKING_TABLE].send_data_count;
//				pSendDataInfo[RAU_OUT_PARKING_TABLE].send_complete_count = 0;
//				pSendDataInfo[RAU_OUT_PARKING_TABLE].fill_up_data_count = 0;
//				pSendDataInfo[RAU_OUT_PARKING_TABLE].crc_err_data_count = 0;
//				pSendDataInfo[RAU_OUT_PARKING_TABLE].pbuff_info = &rau_data_table_info[RAU_OUT_PARKING_TABLE];
//			} 
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

			if ((RauConfig.id22_23_tel_flg == 1) &&
				(RauConfig.id22_23_mask_flg == 0) &&
				(Ope_Log_UnreadCountGet(eLOG_PAYMENT, eLOG_TARGET_REMOTE) > 0)) {
				pSendDataInfo[RAU_PAID_DATA22_TABLE].send_req = 2;
				pSendDataInfo[RAU_PAID_DATA22_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_PAYMENT, eLOG_TARGET_REMOTE);
				rau_data_table_info[RAU_PAID_DATA22_TABLE].ui_SendDataCount = pSendDataInfo[RAU_PAID_DATA22_TABLE].send_data_count;
				pSendDataInfo[RAU_PAID_DATA22_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_PAID_DATA22_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_PAID_DATA22_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_PAID_DATA22_TABLE].pbuff_info = &rau_data_table_info[RAU_PAID_DATA22_TABLE];
			}

			if ((RauConfig.id22_23_tel_flg == 1) &&
				(RauConfig.id22_23_mask_flg == 0) &&
				(Ope_Log_UnreadCountGet(eLOG_PAYMENT, eLOG_TARGET_REMOTE) > 0)) {
				pSendDataInfo[RAU_PAID_DATA23_TABLE].send_req = 2;
				pSendDataInfo[RAU_PAID_DATA23_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_PAYMENT, eLOG_TARGET_REMOTE);
				rau_data_table_info[RAU_PAID_DATA23_TABLE].ui_SendDataCount = pSendDataInfo[RAU_PAID_DATA23_TABLE].send_data_count;
				pSendDataInfo[RAU_PAID_DATA23_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_PAID_DATA23_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_PAID_DATA23_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_PAID_DATA23_TABLE].pbuff_info = &rau_data_table_info[RAU_PAID_DATA23_TABLE];
			} 

			if ((RauConfig.id120_tel_flg == 1) &&
				(RauConfig.id120_mask_flg == 0) &&
				(RauConfig.error_send_level != 9) &&
				(Ope_Log_UnreadCountGet(eLOG_ERROR, eLOG_TARGET_REMOTE) > 0)) {
				pSendDataInfo[RAU_ERROR_TABLE].send_req = 2;
				pSendDataInfo[RAU_ERROR_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_ERROR, eLOG_TARGET_REMOTE);
				rau_data_table_info[RAU_ERROR_TABLE].ui_SendDataCount = pSendDataInfo[RAU_ERROR_TABLE].send_data_count;
				pSendDataInfo[RAU_ERROR_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_ERROR_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_ERROR_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_ERROR_TABLE].pbuff_info = &rau_data_table_info[RAU_ERROR_TABLE];
			} 

			if ((RauConfig.id121_tel_flg == 1) &&
				(RauConfig.id121_mask_flg == 0) &&
				(RauConfig.alarm_send_level != 9) &&
				(Ope_Log_UnreadCountGet(eLOG_ALARM, eLOG_TARGET_REMOTE) > 0)) {
				pSendDataInfo[RAU_ALARM_TABLE].send_req = 2;
				pSendDataInfo[RAU_ALARM_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_ALARM, eLOG_TARGET_REMOTE);
				rau_data_table_info[RAU_ALARM_TABLE].ui_SendDataCount = pSendDataInfo[RAU_ALARM_TABLE].send_data_count;
				pSendDataInfo[RAU_ALARM_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_ALARM_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_ALARM_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_ALARM_TABLE].pbuff_info = &rau_data_table_info[RAU_ALARM_TABLE];
			} 

			if ((RauConfig.id122_tel_flg == 1) &&
				(RauConfig.id122_mask_flg == 0) &&
				(RauConfig.monitor_send_level != 9) &&
				(Ope_Log_UnreadCountGet(eLOG_MONITOR, eLOG_TARGET_REMOTE) > 0)) {
				pSendDataInfo[RAU_MONITOR_TABLE].send_req = 2;
				pSendDataInfo[RAU_MONITOR_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_MONITOR, eLOG_TARGET_REMOTE);
				rau_data_table_info[RAU_MONITOR_TABLE].ui_SendDataCount = pSendDataInfo[RAU_MONITOR_TABLE].send_data_count;
				pSendDataInfo[RAU_MONITOR_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_MONITOR_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_MONITOR_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_MONITOR_TABLE].pbuff_info = &rau_data_table_info[RAU_MONITOR_TABLE];
			} 

			if ((RauConfig.id123_tel_flg == 1) &&
				(RauConfig.id123_mask_flg == 0) &&
				(RauConfig.opemonitor_send_level != 9) &&
				(Ope_Log_UnreadCountGet(eLOG_OPERATE, eLOG_TARGET_REMOTE) > 0)) {
				pSendDataInfo[RAU_OPE_MONITOR_TABLE].send_req = 2;
				pSendDataInfo[RAU_OPE_MONITOR_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_OPERATE, eLOG_TARGET_REMOTE);
				rau_data_table_info[RAU_OPE_MONITOR_TABLE].ui_SendDataCount = pSendDataInfo[RAU_OPE_MONITOR_TABLE].send_data_count;
				pSendDataInfo[RAU_OPE_MONITOR_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_OPE_MONITOR_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_OPE_MONITOR_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_OPE_MONITOR_TABLE].pbuff_info = &rau_data_table_info[RAU_OPE_MONITOR_TABLE];
			} 

			if ((RauConfig.id131_tel_flg == 1) &&
				(RauConfig.id131_mask_flg == 0) &&
				(Ope_Log_UnreadCountGet(eLOG_COINBOX, eLOG_TARGET_REMOTE) > 0)) {
				pSendDataInfo[RAU_COIN_TOTAL_TABLE].send_req = 2;
				pSendDataInfo[RAU_COIN_TOTAL_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_COINBOX, eLOG_TARGET_REMOTE);
				rau_data_table_info[RAU_COIN_TOTAL_TABLE].ui_SendDataCount = pSendDataInfo[RAU_COIN_TOTAL_TABLE].send_data_count;
				pSendDataInfo[RAU_COIN_TOTAL_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_COIN_TOTAL_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_COIN_TOTAL_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_COIN_TOTAL_TABLE].pbuff_info = &rau_data_table_info[RAU_COIN_TOTAL_TABLE];
			} 

			if ((RauConfig.id133_tel_flg == 1) &&
				(RauConfig.id133_mask_flg == 0) &&
				(Ope_Log_UnreadCountGet(eLOG_NOTEBOX, eLOG_TARGET_REMOTE) > 0)) {
				pSendDataInfo[RAU_MONEY_TORAL_TABLE].send_req = 2;
				pSendDataInfo[RAU_MONEY_TORAL_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_NOTEBOX, eLOG_TARGET_REMOTE);
				rau_data_table_info[RAU_MONEY_TORAL_TABLE].ui_SendDataCount = pSendDataInfo[RAU_MONEY_TORAL_TABLE].send_data_count;
				pSendDataInfo[RAU_MONEY_TORAL_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_MONEY_TORAL_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_MONEY_TORAL_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_MONEY_TORAL_TABLE].pbuff_info = &rau_data_table_info[RAU_MONEY_TORAL_TABLE];
			} 

// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//			if ((RauConfig.id236_tel_flg == 1) &&
//				(RauConfig.id236_mask_flg == 0) &&
//				(Ope_Log_UnreadCountGet(eLOG_PARKING, eLOG_TARGET_REMOTE) > 0)) {
//				pSendDataInfo[RAU_PARKING_NUM_TABLE].send_req = 2;
//				pSendDataInfo[RAU_PARKING_NUM_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_PARKING, eLOG_TARGET_REMOTE);
//				rau_data_table_info[RAU_PARKING_NUM_TABLE].ui_SendDataCount = pSendDataInfo[RAU_PARKING_NUM_TABLE].send_data_count;
//				pSendDataInfo[RAU_PARKING_NUM_TABLE].send_complete_count = 0;
//				pSendDataInfo[RAU_PARKING_NUM_TABLE].fill_up_data_count = 0;
//				pSendDataInfo[RAU_PARKING_NUM_TABLE].crc_err_data_count = 0;
//				pSendDataInfo[RAU_PARKING_NUM_TABLE].pbuff_info = &rau_data_table_info[RAU_PARKING_NUM_TABLE];
//			} 
//
//			if ((RauConfig.id237_tel_flg == 1) &&
//				(RauConfig.id237_mask_flg == 0) &&
//				(rau_data_table_info[RAU_AREA_NUM_TABLE].ui_DataCount)) {
//				pSendDataInfo[RAU_AREA_NUM_TABLE].send_req = 2;
//				pSendDataInfo[RAU_AREA_NUM_TABLE].send_data_count = 0;
//				rau_data_table_info[RAU_AREA_NUM_TABLE].ui_SendDataCount = pSendDataInfo[RAU_AREA_NUM_TABLE].send_data_count;
//				pSendDataInfo[RAU_AREA_NUM_TABLE].send_complete_count = 0;
//				pSendDataInfo[RAU_AREA_NUM_TABLE].fill_up_data_count = 0;
//				pSendDataInfo[RAU_AREA_NUM_TABLE].crc_err_data_count = 0;
//				pSendDataInfo[RAU_AREA_NUM_TABLE].pbuff_info = &rau_data_table_info[RAU_AREA_NUM_TABLE];
//			} 
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

			// T���v
			logCount = Ope_Log_UnreadCountGet(eLOG_TTOTAL, eLOG_TARGET_REMOTE);
			if ((RauConfig.id30_41_tel_flg == 1) &&
				(RauConfig.id30_41_mask_flg == 0) &&
				(RauConfig.serverTypeSendTsum != 0) && 	// 34-0026�C 0:���M���Ȃ� 1:���M����
				(logCount > 0)) {
				pSendDataInfo[RAU_TOTAL_TABLE].send_req = 2;
				pSendDataInfo[RAU_TOTAL_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].pbuff_info = &rau_data_table_info[RAU_TOTAL_TABLE];
				// ���O����W�v�f�[�^���擾���Atsum�f�[�^�Ɋi�[
			} 
			if(logCount > 0) {
				if(RauConfig.serverTypeSendTsum == 0){				// 34-0026�C 0:���M���Ȃ� 1:���M����
					for(i=0; i<logCount; i++){
						Ope_Log_TargetDataGet( eLOG_TTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
					}
				}
			}

			// GT���v
			logCount = Ope_Log_UnreadCountGet(eLOG_GTTOTAL, eLOG_TARGET_REMOTE);
			if ((RauConfig.id30_41_tel_flg == 1) &&
				(RauConfig.id30_41_mask_flg == 0) &&
				(RauConfig.serverTypeSendTsum != 0) && 	// 34-0026�C 0:���M���Ȃ� 1:���M����
				(logCount > 0)) {
				pSendDataInfo[RAU_GTOTAL_TABLE].send_req = 2;
				pSendDataInfo[RAU_GTOTAL_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_GTOTAL_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_GTOTAL_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_GTOTAL_TABLE].pbuff_info = &rau_data_table_info[RAU_GTOTAL_TABLE];
			}
			if(logCount > 0) {
// MH322914 (s) kasiyama 2016/07/07 ����T���v�f�[�^���đ����ꑱ���A�Ȍ��T���v�����M����Ȃ�(���ʉ��PNo.1250)
//				if(RauConfig.serverTypeSendTsum == 0){				// 34-0026�C 0:���M���Ȃ� 1:���M����
				if(prm_get(COM_PRM,S_NTN,121,1,1) == 0) {
					// ParkRing�̏ꍇ��GT���v�𑗐M���Ȃ�
					pSendDataInfo[RAU_GTOTAL_TABLE].send_req = 0;
				}
				if((RauConfig.serverTypeSendTsum == 0) ||				// 34-0026�C 0:���M���Ȃ� 1:���M����
					(prm_get(COM_PRM,S_NTN,121,1,1) == 0)) {
					// ParkRing�̏ꍇ��GT���v�𑗐M���Ȃ�
// MH322914 (e) kasiyama 2016/07/07 ����T���v�f�[�^���đ����ꑱ���A�Ȍ��T���v�����M����Ȃ�(���ʉ��PNo.1250)
					for(i=0; i<logCount; i++){
						Ope_Log_TargetDataGet( eLOG_GTTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
					}
				}
			}

			// ���K�Ǘ�
			if ((RauConfig.id126_tel_flg == 1) &&
				(RauConfig.id126_mask_flg == 0) &&
				(Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE) > 0)) {
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].send_req = 2;
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].pbuff_info = &rau_data_table_info[RAU_MONEY_MANAGE_TABLE];
			} 
			// �ޑK�Ǘ��W�v
			if ((RauConfig.id135_tel_flg == 1) &&
				(RauConfig.id135_mask_flg == 0) &&
				(Ope_Log_UnreadCountGet(eLOG_MNYMNG_SRAM, eLOG_TARGET_REMOTE) > 0)) {
				pSendDataInfo[RAU_TURI_MANAGE_TABLE].send_req = 2;
				pSendDataInfo[RAU_TURI_MANAGE_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_MNYMNG_SRAM, eLOG_TARGET_REMOTE);
				pSendDataInfo[RAU_TURI_MANAGE_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_TURI_MANAGE_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_TURI_MANAGE_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_TURI_MANAGE_TABLE].pbuff_info = &rau_data_table_info[RAU_TURI_MANAGE_TABLE];
			} 

			// ���u�Ď��f�[�^
			if ((RauConfig.id125_tel_flg == 1) &&
				(RauConfig.id125_mask_flg == 0) &&
				(RauConfig.rmon_send_level != 9) &&
				(Ope_Log_UnreadCountGet(eLOG_REMOTE_MONITOR, eLOG_TARGET_REMOTE) > 0)) {
				pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_req = 2;
				pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_REMOTE_MONITOR, eLOG_TARGET_REMOTE);
				rau_data_table_info[RAU_REMOTE_MONITOR_TABLE].ui_SendDataCount = pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_data_count;
				pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].pbuff_info = &rau_data_table_info[RAU_REMOTE_MONITOR_TABLE];
			}

// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//			// �������ԏ�ԃf�[�^
//			if ((RauConfig.id61_tel_flg == 1) &&// �������ԏ�ԃf�[�^�̎������M����
//				(RauConfig.id61_mask_flg == 0) &&// �������ԏ�ԃf�[�^�̑��M�}�X�N�Ȃ�
//				(Ope_Log_UnreadCountGet(eLOG_LONGPARK_PWEB, eLOG_TARGET_REMOTE) > 0)) {
//				pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_req = 2;
//				pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_LONGPARK_PWEB, eLOG_TARGET_REMOTE);
//				rau_data_table_info[RAU_LONG_PARKING_INFO_TABLE].ui_SendDataCount = pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_data_count;
//				pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_complete_count = 0;
//				pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].fill_up_data_count = 0;
//				pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].crc_err_data_count = 0;
//				pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].pbuff_info = &rau_data_table_info[RAU_LONG_PARKING_INFO_TABLE];
//			}
//// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
		}

		if( check_center_term_que(RAU_centerterminfo_man.sendq) > 0 ) { // 65:�Z���^�[�p�[�����f�[�^�𑗐M�i�����j
			pSendDataInfo[RAU_CENTER_TERM_INFO_TABLE].send_req = 1;
			pSendDataInfo[RAU_CENTER_TERM_INFO_TABLE].send_data_count = check_center_term_que(RAU_centerterminfo_man.sendq);
			pSendDataInfo[RAU_CENTER_TERM_INFO_TABLE].send_complete_count = 0;
			pSendDataInfo[RAU_CENTER_TERM_INFO_TABLE].fill_up_data_count = 0;
			pSendDataInfo[RAU_CENTER_TERM_INFO_TABLE].crc_err_data_count = 0;
			pSendDataInfo[RAU_CENTER_TERM_INFO_TABLE].pbuff_info = &rau_data_table_info[RAU_CENTER_TERM_INFO_TABLE];
		}
	}
}


//==============================================================================
//	�X���[�f�[�^�̊i�[
//
//	@argument	*pData		�i�[����m�s�|�m�d�s�d��(�擪�Ƀf�[�^���A�����ɂb�q�b����)
//	@argument	wDataLen	�f�[�^��(�擪�̃f�[�^���Ɩ����̂b�q�b���܂߂��o�C�g��)
//	@argument	ucDirection	�i�[����f�[�^�o�b�t�@(0:RAU_SEND_THROUGH_DATA_TABLE, 1:RAU_RECEIVE_THROUGH_DATA_TABLE)
//
//	@return		RAU_DATA_NORMAL		�i�[����
//	@return		RAU_DATA_BUFFERFULL	�i�[���s(�o�b�t�@�t��)
//
//	@attention	�g�n�r�s�����M�����X���[�f�[�^�͎�M�X���[�f�[�^�o�b�t�@��
//				�h�a�v�����M�����X���[�f�[�^�͑��M�X���[�f�[�^�o�b�t�@�Ɋi�[����܂��B
//
//	@note		�g�n�r�s�܂��͂h�a�v�����M�����X���[�f�[�^����тh�a�j���Ǝ��ɍ쐬����
//				���M�f�[�^���X���[�f�[�^�o�b�t�@�֊i�[���܂��B
//
//	@author		2007.01.05:m-onouchi
//======================================== Copyright(C) 2007 AMANO Corp. =======
eRAU_TABLEDATA_SET	RAUdata_SetThroughData(uchar *pData, ushort wDataLen, uchar ucDirection) {

	eRAU_TABLEDATA_SET ret;

	ret = RAU_DATA_BUFFERFULL;

	if (ucDirection != 1) {
		// HOST�����X���[�f�[�^�o�b�t�@�ɓo�^
		if (rau_data_table_info[RAU_SEND_THROUGH_DATA_TABLE].ui_DataCount == 0) {
			if(RAUdata_WriteNtData(pData, wDataLen, &rau_data_table_info[RAU_SEND_THROUGH_DATA_TABLE]) == TRUE) {
				ret = RAU_DATA_NORMAL;
			} else {
				ret = RAU_DATA_BUFFERFULL;
			}
		}
	} else {
		// IBW�����X���[�f�[�^�o�b�t�@�ɓo�^
			if (RAUdata_WriteNtData(pData, wDataLen, &rau_data_table_info[RAU_RECEIVE_THROUGH_DATA_TABLE]) == TRUE) {
				ret = RAU_DATA_NORMAL;
			} else {
				ret = RAU_DATA_BUFFERFULL;
			}
	}

	return ret;
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	�C�ӂ̒����̃f�[�^���ړI�̃e�[�u���f�[�^�o�b�t�@�ɂ����邩�`�F�b�N����
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	RAUdata_CheckCapacity(RAU_DATA_TABLE_INFO *pBuffInfo, ushort wDataLen) {
	
	if (pBuffInfo->ui_DataCount == 0) {
		// �f�[�^�������O�̎��͗̈�T�C�Y�݂̂Ɣ�r�B
		if ((pBuffInfo->ul_AreaSize - 1) >= wDataLen) {
			// ���[�h�|�C���^�ƃ��C�g�|�C���^�̏d�Ȃ肠����������邽�߁A�P�������炷�B
			return TRUE;
		}
	} else {
		// �f�[�^�������O�ȏ�̎��͎Z�o�����󂫗e�ʂƔ�r����
		if ((pBuffInfo->ul_AreaSize - RAUdata_GetUseBuff(pBuffInfo) - 1) >= wDataLen) {
			// ���[�h�|�C���^�ƃ��C�g�|�C���^�̏d�Ȃ肠����������邽�߁A�P�������炷�B
			return TRUE;
		}
	}
	return FALSE;
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
BOOL	RAUdata_CheckNearFull(void) {

	int i;
	BOOL ret;

	ret = FALSE;

	// �e�o�b�t�@�̃j�A�t���X�e�[�^�X���m�F���O��l�ƈقȂ��Ă�����ATURE��Ԃ��
	for (i = 0; i < RAU_SEND_THROUGH_DATA_TABLE; i++) {
		if (rau_data_table_info[i].i_NearFullStatus != rau_data_table_info[i].i_NearFullStatusBefore) {
			ret = TRUE;
			// ���ݒl��O��l�ɏ������݁A���d�������Ȃ����
			rau_data_table_info[i].i_NearFullStatusBefore = rau_data_table_info[i].i_NearFullStatus;
		}	// <-#001
	}

	return ret;
}


/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	�e�[�u���f�[�^�o�b�t�@�̎g�p�ʂ��擾����
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ulong	RAUdata_GetUseBuff(RAU_DATA_TABLE_INFO *pBuffInfo) {
	
	if (pBuffInfo->ui_DataCount == 0) {

		return 0;

	} else {
		// �f�[�^�������O�ȏ�̎��͎Z�o�����󂫗e�ʂƔ�r����
		if (pBuffInfo->pc_WritePoint > pBuffInfo->pc_ReadPoint) {

			return (ulong)(pBuffInfo->pc_WritePoint - pBuffInfo->pc_ReadPoint);

		} else if (pBuffInfo->pc_WritePoint < pBuffInfo->pc_ReadPoint) {

			return (ulong)((pBuffInfo->pc_WritePoint - pBuffInfo->pc_AreaTop) + ((pBuffInfo->pc_AreaTop + pBuffInfo->ul_AreaSize) - pBuffInfo->pc_ReadPoint));
		} else {
			// pBuffInfo->pc_WritePoint == pBuffInfo->pc_ReadPoint
			// �̏ꍇ1byte���]�����g�p�B
			return pBuffInfo->ul_AreaSize;
		}
	}
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	�j�A�t���f�[�^����
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAUdata_CreateNearFullData(RAU_NEAR_FULL *pNearFullData) {

	memset(pNearFullData, 0x00, sizeof(RAU_NEAR_FULL));

	pNearFullData->data_len[0] = 0;
	pNearFullData->data_len[1] = 29;
	pNearFullData->data_len_zero_cut[0] = 0;
	pNearFullData->data_len_zero_cut[1] = 22;

	pNearFullData->id1 = 1;
	pNearFullData->id2 = 1;
	pNearFullData->id3 = 30;
	pNearFullData->id4 = 90;

	pNearFullData->buffering_flag = 1;

	pNearFullData->in_parking		= rau_data_table_info[RAU_IN_PARKING_TABLE].i_NearFullStatus;
	pNearFullData->out_parking		= rau_data_table_info[RAU_OUT_PARKING_TABLE].i_NearFullStatus;
	pNearFullData->paid_data22		= rau_data_table_info[RAU_PAID_DATA22_TABLE].i_NearFullStatus;
	pNearFullData->paid_data23		= rau_data_table_info[RAU_PAID_DATA23_TABLE].i_NearFullStatus;
	// T���v�AGT���v�̂ǂ��炩���j�A�t���Ȃ�j�A�t���Œʒm����
	if( rau_data_table_info[RAU_TOTAL_TABLE].i_NearFullStatus == RAU_NEARFULL_NEARFULL ||
		rau_data_table_info[RAU_GTOTAL_TABLE].i_NearFullStatus == RAU_NEARFULL_NEARFULL) {
		pNearFullData->total		= RAU_NEARFULL_NEARFULL;
	}
	else {
		pNearFullData->total		= RAU_NEARFULL_CANCEL;
	}
	pNearFullData->error			= rau_data_table_info[RAU_ERROR_TABLE].i_NearFullStatus;
	pNearFullData->alarm			= rau_data_table_info[RAU_ALARM_TABLE].i_NearFullStatus;
	pNearFullData->monitor			= rau_data_table_info[RAU_MONITOR_TABLE].i_NearFullStatus;
	pNearFullData->ope_monitor		= rau_data_table_info[RAU_OPE_MONITOR_TABLE].i_NearFullStatus;
	pNearFullData->coin_total		= rau_data_table_info[RAU_COIN_TOTAL_TABLE].i_NearFullStatus;
	pNearFullData->money_toral		= rau_data_table_info[RAU_MONEY_TORAL_TABLE].i_NearFullStatus;
	pNearFullData->parking_num		= rau_data_table_info[RAU_PARKING_NUM_TABLE].i_NearFullStatus;
	pNearFullData->area_num			= rau_data_table_info[RAU_AREA_NUM_TABLE].i_NearFullStatus;
	pNearFullData->money_manage		= RAU_NEARFULL_NORMAL;	// #001
	pNearFullData->turi_manage		= rau_data_table_info[RAU_TURI_MANAGE_TABLE].i_NearFullStatus;
	pNearFullData->rmon				= rau_data_table_info[RAU_REMOTE_MONITOR_TABLE].i_NearFullStatus;
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	0���f�[�^����
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAUdata_Create0Data(RAU_NTDATA_HEADER *pNtDataHeader, uchar ucDataId) {

	memset(pNtDataHeader, 0x00, 10);

	pNtDataHeader->data_len[1] = 10;

	pNtDataHeader->data_len_zero_cut[1] = 0;

	pNtDataHeader->id1 = 1;
	pNtDataHeader->id2 = 1;
	pNtDataHeader->id3 = 0xFF;
	if(prm_get(COM_PRM, S_NTN, 121, 1, 1)) {
		// ���ԏ�Z���^�[�`��
		switch(ucDataId){
			case 20:								// ����
				pNtDataHeader->id4 = 54;
				break;
			case 21:								// �o��
				pNtDataHeader->id4 = 55;
				break;
			case 22:								// ���Z(���O)
				pNtDataHeader->id4 = 56;
				break;
			case 23:								// ���Z(�o��)
				pNtDataHeader->id4 = 57;
				break;
			case 120:								// �G���[�f�[�^
				pNtDataHeader->id4 = 63;
				break;
			case 121:								// �A���[���f�[�^
				pNtDataHeader->id4 = 64;
				break;
			case 236:								// ���ԑ䐔
				pNtDataHeader->id4 = 58;
				break;
			case 239:								// ���䐔
				pNtDataHeader->id4 = 59;
				break;
			default:
				pNtDataHeader->id4 = ucDataId;
				break;
		}
	}
	else{
		pNtDataHeader->id4 = ucDataId;
	}

	pNtDataHeader->buffering_flag= 1;
	
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void RAUdata_CreatTableDataCount(RAU_TABLE_DATA_COUNT *pDataCount) {

	memset(pDataCount, 0, sizeof(RAU_TABLE_DATA_COUNT));
	
	pDataCount->data_len[1] = 48;				// �����[�g�m�s�u���b�N�̃T�C�Y(�w�b�_�P�O�o�C�g�{�f�[�^��[�ő�X�U�O])
	pDataCount->data_len_zero_cut[1] = 38;		// �f�[�^��[�ő�X�U�O]�ɑ΂��Č�낪�O�J�b�g�����O�̃f�[�^�T�C�Y
	pDataCount->id1 = 0x01;
	pDataCount->id2 = 0x01;
	pDataCount->id3 = 30;
	pDataCount->id4 = 83;
	pDataCount->buffering_flag = 0x01;			// �����܂ł����ʕ�(����ȍ~���f�[�^��)
// MH810100(S) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//	// ���Ƀf�[�^			�iID20�j
//	RAU_Word2Byte(pDataCount->in_parking, Ope_Log_UnreadCountGet(eLOG_ENTER, eLOG_TARGET_REMOTE));
// MH810100(E) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)

// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	// �o�Ƀf�[�^			�iID21�j
//	RAU_Word2Byte(pDataCount->out_parking, 0);
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

	// ���Z�f�[�^			�iID22�j	�����ł�22�@23�@�����Z����
	RAU_Word2Byte(pDataCount->paid_data, Ope_Log_UnreadCountGet(eLOG_PAYMENT, eLOG_TARGET_REMOTE));

	// �G���[�f�[�^			�iID120�j
	RAU_Word2Byte(pDataCount->error, Ope_Log_UnreadCountGet(eLOG_ERROR, eLOG_TARGET_REMOTE));

	// �A���[���f�[�^		�iID121�j
	RAU_Word2Byte(pDataCount->alarm, Ope_Log_UnreadCountGet(eLOG_ALARM, eLOG_TARGET_REMOTE));

	// ���j�^�[�f�[�^		�iID122�j
	RAU_Word2Byte(pDataCount->monitor, Ope_Log_UnreadCountGet(eLOG_MONITOR, eLOG_TARGET_REMOTE));

	// ���샂�j�^�[�f�[�^	�iID123�j
	RAU_Word2Byte(pDataCount->ope_monitor, Ope_Log_UnreadCountGet(eLOG_OPERATE, eLOG_TARGET_REMOTE));

	// �R�C�����ɏW�v�f�[�^	�iID131�j
	RAU_Word2Byte(pDataCount->coin_total, Ope_Log_UnreadCountGet(eLOG_COINBOX, eLOG_TARGET_REMOTE));

	// �������ɏW�v�f�[�^	�iID133�j
	RAU_Word2Byte(pDataCount->money_toral, Ope_Log_UnreadCountGet(eLOG_NOTEBOX, eLOG_TARGET_REMOTE));

// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	// ���ԑ䐔�E���ԃf�[�^	�iID236�j
//	RAU_Word2Byte(pDataCount->parking_num, Ope_Log_UnreadCountGet(eLOG_PARKING, eLOG_TARGET_REMOTE));
//
//	// ���䐔�E���ԃf�[�^	�iID237�j
//	RAU_Word2Byte(pDataCount->area_num, 0);
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)

	// T���v�W�v�f�[�^		�iID30�`38�E41�j
	RAU_Word2Byte(pDataCount->total, Ope_Log_UnreadCountGet(eLOG_TTOTAL, eLOG_TARGET_REMOTE));

	// GT���v�W�v�f�[�^		�iID30�`38�E41�j
	RAU_Word2Byte(pDataCount->gtotal, Ope_Log_UnreadCountGet(eLOG_GTTOTAL, eLOG_TARGET_REMOTE));

	// ���K�Ǘ��f�[�^		�iID126�j(176 + 7 + 4) * 32
	RAU_Word2Byte(pDataCount->money_manage, Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE));

	// ���u�Ď��f�[�^
	RAU_Word2Byte(pDataCount->rmon, Ope_Log_UnreadCountGet(eLOG_REMOTE_MONITOR, eLOG_TARGET_REMOTE));
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAUdata_TableClear(uchar ucTableKind) {

	uchar ucTableIndex;
	ucTableIndex = RAUdata_Id2Index(ucTableKind);

	rau_table_crear_flag = ucTableIndex;

	rau_data_table_info[ucTableIndex].i_NearFullStatus			= RAU_NEARFULL_CANCEL;	// #001
	rau_data_table_info[ucTableIndex].pc_ReadPoint				= rau_data_table_info[ucTableIndex].pc_AreaTop;
	rau_data_table_info[ucTableIndex].pc_WritePoint				= rau_data_table_info[ucTableIndex].pc_AreaTop;
	rau_data_table_info[ucTableIndex].ui_DataCount				= 0;
	rau_data_table_info[ucTableIndex].ui_SendDataCount			= 0;
	rau_data_table_info[ucTableIndex].c_DataFullProc			= 0;

	memset( &RAUque_SendDataInfo[ucTableIndex], 0, sizeof(RAUque_SendDataInfo[ucTableIndex]) );	// ���M�p�̃f�[�^���N���A

	if (RAU_TOTAL_TABLE == ucTableIndex) {
		// T���v�W�v�f�[�^		�iID30�`38�E41�j
		reset_tsum((char*)RAU_table_data.total, RAU_TOTAL_SIZE);
	}
	else if (RAU_CENTER_TERM_INFO_TABLE == ucTableIndex) {
		// �Z���^�[�p�[�����f�[�^		�iID65�j
		reset_center_term_info((char*)RAU_table_data.center_term_info);
	}

	rau_table_crear_flag = -1;
}

/*[]----------------------------------------------------------------------[]*
 *| 	RAU_raudata_ReadTable
 *[]----------------------------------------------------------------------[]*
 *|	�e�[�u���o�b�t�@����f�[�^�ǂݏo��(�o�b�t�@�I�[�ł̐܂�Ԃ��ɑΉ�)
 *[]----------------------------------------------------------------------[]*
 *|	param	: table	=��M�f�[�^�e�[�u��
 *|			  rptr	=�ǂݏo���J�n�ʒu
 *|			  data	=�擾�f�[�^�����ݐ�̈�						<OUT>
 *|			  size	=�ǂݏo���T�C�Y
 *[]----------------------------------------------------------------------[]*
 *|	return	: �f�[�^�ǂݏo�����rptr�̈ʒu
 *[]----------------------------------------------------------------------[]*
 *| Author	:	machida kei
 *| Date	:	2005-08-23
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	*RAU_raudata_ReadTable(RAU_DATA_TABLE_INFO *table, const uchar *rptr, uchar *data, size_t size)
{
	size_t work;
	
	if (rptr + size >= &table->pc_AreaTop[table->ul_AreaSize]) {
		// �܂�Ԃ��܂ł̃f�[�^�ǂݍ���
		work = (size_t)(&table->pc_AreaTop[table->ul_AreaSize] - rptr);
		memcpy(data, rptr, work);
		rptr = table->pc_AreaTop;
		data += work;
		size -= work;
	}
	memcpy(data, rptr, size);
	
	return (uchar*)(rptr + size);
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	
 *[]----------------------------------------------------------------------[]*
 *|	patam
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAUdata_DeleteTsumGroup(ushort ucSendCount) {

	int i;
	int tsum_count;

	// ���M�ł����f�[�^�̐���41�܂ł̐����r���A�S�P���Ƃɂ����B
	while (ucSendCount > 0) {
		tsum_count = seek_tsum_data();
		if (tsum_count == 0) {
			break;
		}
		if (tsum_count <= ucSendCount) {
			for (i = 0; i < tsum_count; i++) {
				// ���M�������������������
				remove_tsum_data();
			}
			ucSendCount -= tsum_count;
		} else {
			break;
		}
	}

	RAU_reset_send_tsum();

}
/*[]----------------------------------------------------------------------[]*
 *|             RAUdata_CheckNearFullLogData()
 *[]----------------------------------------------------------------------[]*
 *|	�e�[�u���f�[�^�̃j�A�t���ʒm���s��
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAUdata_CheckNearFullLogData(void)
{
	ushort	index;
	uchar	state;
	
	for(index = 0; index < RAU_ID_LOGIDLIST_COUNT; ++index) {
		if(RAU_id_LogIDList[index] == -1) {				// �ΏۊO�̃��O�f�[�^
			continue;
		}
		if(!RAUdata_CanSendTableData(RAU_id_LogIDList[index])) {
			// ���M�}�X�N�ݒ肪����邽�߃j�A�t����ʒm���Ȃ�
			rau_data_table_info[index].i_NearFullStatusBefore = RAU_NEARFULL_CANCEL;
			rau_data_table_info[index].i_NearFullStatus = RAU_NEARFULL_CANCEL;
			continue;
		}
		// ���݂̃j�A�t����Ԃ��擾
		state = Ope_Log_GetNearFull(RAU_id_LogIDList[index], eLOG_TARGET_REMOTE);
		
		// �O��ƕύX������Ώ�Ԃ�Ҕ����āA���݂̏�Ԃ��i�[����
		if(rau_data_table_info[index].i_NearFullStatus != state) {
			rau_data_table_info[index].i_NearFullStatusBefore = rau_data_table_info[index].i_NearFullStatus;
			rau_data_table_info[index].i_NearFullStatus = state;
		}
	}
	
	if(TRUE == RAUdata_CheckNearFull()) {						// �j�A�t����Ԃɕω������邩
		// �j�A�t���ʒm���s��
		RAUdata_CreateNearFullData((RAU_NEAR_FULL*)&RAU_temp_data[2]);	// ��ԕω����Ă�����ʒm
		RAU_Word2Byte(RAU_temp_data, 32);
		RAU_CrcCcitt(30, 0, &RAU_temp_data[2], (uchar*)NULL, &RAU_temp_data[32], 1 );
		RAUdata_SetThroughData(RAU_temp_data, 34, 1);
	}
}
/*[]----------------------------------------------------------------------[]*
 *|             RAUdata_CanSendTableData()
 *[]----------------------------------------------------------------------[]*
 *|	�e�[�u���f�[�^���M�\�i�}�X�N����Ă��Ȃ��j���`�F�b�N����
 *[]----------------------------------------------------------------------[]*
 *|	param	elogID	���OID(eLOG_*)
 *[]----------------------------------------------------------------------[]*
 *|	return	TRUE	���M�\
 *|			FALSE	���M�s�i���M�}�X�N�ݒ�j
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
BOOL	RAUdata_CanSendTableData(short elogID)
{
	uchar	sendMask;								// ���M�}�X�N�ݒ�

	BOOL	ret;
	
	switch(elogID) {
// MH810100(S) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
//	case eLOG_ENTER:								// ����
//		sendMask = RauConfig.id20_21_mask_flg;
//		break;
// MH810100(E) K.Onodera  2019/11/15 �Ԕԃ`�P�b�g���X(RT���Z�f�[�^�Ή�)
	case eLOG_PAYMENT:								// ���Z
		sendMask = RauConfig.id22_23_mask_flg;
		break;
	case eLOG_ERROR:								// �G���[
		sendMask = RauConfig.id120_mask_flg;
		break;
	case eLOG_ALARM:								// �A���[��
		sendMask = RauConfig.id121_mask_flg;
		break;
	case eLOG_MONITOR:								// ���j�^
		sendMask = RauConfig.id122_mask_flg;
		break;
	case eLOG_OPERATE:								// ����
		sendMask = RauConfig.id123_mask_flg;
		break;
	case eLOG_COINBOX:								// �R�C�����ɏW�v
		sendMask = RauConfig.id131_mask_flg;
		break;
	case eLOG_NOTEBOX:								// �������ɏW�v
		sendMask = RauConfig.id133_mask_flg;
		break;
// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//	case eLOG_PARKING:								// ���ԑ䐔�f�[�^
//		sendMask = RauConfig.id236_mask_flg;
//		break;
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	case eLOG_TTOTAL:								// �s�W�v
// MH322914 (s) kasiyama 2016/07/07 ����T���v�f�[�^���đ����ꑱ���A�Ȍ��T���v�����M����Ȃ�(���ʉ��PNo.1250)
//	case eLOG_GTTOTAL:								// GT�W�v
// MH322914 (e) kasiyama 2016/07/07 ����T���v�f�[�^���đ����ꑱ���A�Ȍ��T���v�����M����Ȃ�(���ʉ��PNo.1250)
		sendMask = RauConfig.id30_41_mask_flg;
		if(sendMask == 0 && RauConfig.serverTypeSendTsum == 0) {	// �W�v���M���Ȃ�(34-0026�C=0)
			sendMask = 1;
		}
		break;
// MH322914 (s) kasiyama 2016/07/07 ����T���v�f�[�^���đ����ꑱ���A�Ȍ��T���v�����M����Ȃ�(���ʉ��PNo.1250)
	case eLOG_GTTOTAL:								// GT�W�v
		if(prm_get(COM_PRM,S_NTN,121,1,1) == 0) {
			// ParkRing�̏ꍇ��GT���v�𑗐M���Ȃ�
			sendMask = 1;
		}
		else {
			sendMask = RauConfig.id30_41_mask_flg;
			if(sendMask == 0 && RauConfig.serverTypeSendTsum == 0) {	// �W�v���M���Ȃ�(34-0026�C=0)
				sendMask = 1;
			}
		}
		break;
// MH322914 (e) kasiyama 2016/07/07 ����T���v�f�[�^���đ����ꑱ���A�Ȍ��T���v�����M����Ȃ�(���ʉ��PNo.1250)
	case eLOG_MONEYMANAGE:							// ���K�Ǘ�
		sendMask = RauConfig.id126_mask_flg;
		break;
	case eLOG_MNYMNG_SRAM:							// �ޑK�Ǘ�
		sendMask = RauConfig.id135_mask_flg;
		break;
	case eLOG_REMOTE_MONITOR:						// ���u�Ď�
		sendMask = RauConfig.id125_mask_flg;
		break;
// MH810100(S) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
//// MH322917(S) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
//	case eLOG_LONGPARK_PWEB:						// ��������(ParkingWeb�p)
//		sendMask = RauConfig.id61_mask_flg;
//		break;
//// MH322917(E) A.Iiizumi 2018/09/21 �������Ԍ��o�@�\�̊g���Ή�(�d���Ή�)
// MH810100(E) K.Onodera  2019/12/20 �Ԕԃ`�P�b�g���X(�t���b�v��->�Q�[�g���ύX�ɔ�������������)
	default:
		sendMask = 1;								// ���M���Ȃ�
		break;
	}
	
	ret = (sendMask == 0) ? TRUE : FALSE;
	return ret;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUid_SetTableData()
 *[]----------------------------------------------------------------------[]*
 *|	�e�[�u���f�[�^���e�[�u���o�b�t�@�ɓo�^����
 *[]----------------------------------------------------------------------[]*
 *|	param	*pData		: �o�^����f�[�^
 *|	param	wDataLen	: �o�^����f�[�^��
 *|	param	ucDataId	: �o�^����f�[�^�̂h�c
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:	MATSUSHITA(2014.1.31)
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
eRAU_TABLEDATA_SET	RAUdata_SetTableData(uchar *pData, ushort wDataLen, uchar ucDataId)
{
	eRAU_TABLEDATA_SET ret;

	// �Z���^�[�p�[�����f�[�^
	if (ucDataId == 0x41) {
		// �Z���^�[�p�[�����f�[�^�͑��̃f�[�^�ƈႤ�Ǘ����@
		if (write_center_term_info((char*)&pData[2], wDataLen - 2) == 1) {
			ret = RAU_DATA_NORMAL;
		} else {
			ret = RAU_DATA_BUFFERFULL;
		}
		// ��M�V�[�P���X�O(�ʏ���)�ɖ߂������A��M�V�[�P���X�L�����Z���^�C�}�̒�~�����́AID155:�Z���^�[�p�f�[�^�v��������
		// �K���ԐM����邽�߁A���̎�M�ɂĎ��s����Ă���̂ŁA�����ł͂��łɏ����ς݂ł���B
	} else if (RAU_isTableData(ucDataId) == FALSE) {
		// �g�����U�N�V�����f�[�^�ȊO�̃f�[�^�i������j�A�X���[�f�[�^�i�������j
		if (RAUdata_WriteNtData(pData, wDataLen, &rau_data_table_info[RAUdata_Id2Index(ucDataId)]) == TRUE) {
			ret = RAU_DATA_NORMAL;
		} else {
			ret = RAU_DATA_BUFFERFULL;
		}
	}

	return ret;
}

BOOL RAUdata_GetCenterTermInfoData(T_SEND_NT_DATA *pSendNtDataInf, uchar *pTsumBuff)
{
	int ret;
	uchar crc_cal[2];

	ret = read_center_term_info((char*)pTsumBuff);

	if (ret != 0) {
		// CRC�Z�o
		RAU_CrcCcitt(ret - 2
					, 0
					, pTsumBuff
					, (uchar*)NULL
					, crc_cal
					, 1);

		// CRC�`�F�b�N���s���
		if (crc_cal[0] != pTsumBuff[ret - 2]
		 || crc_cal[1] != pTsumBuff[ret - 1]) {
			return FALSE;
		} else {

			pSendNtDataInf->nt_data_start = pSendNtDataInf->nt_data_next = pTsumBuff;
			pSendNtDataInf->nt_data_end = &pTsumBuff[ret - 2];

			return TRUE;
		}
	} else {
		return FALSE;
	}
}
