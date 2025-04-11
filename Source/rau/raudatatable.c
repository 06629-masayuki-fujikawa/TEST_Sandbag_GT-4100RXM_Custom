/****************************************************************************/
/*																			*/
/*		システム名　:　RAUシステム											*/
/*		ファイル名	:  DATATABLE.C											*/
/*		機能		:  データテーブル処理									*/
/*																			*/
/****************************************************************************/
//	2006.09.21 #001 m-onouchi テーブルデータのバッファニアフル解除を送信するように修正。
//	2006.10.02 #002 m-onouchi テーブルＡＬＬクリア時に適切に変数を初期化していない不具合を修正。
//	2007.01.24 #003 m-onouchi テーブルデータのバッファニアフル解除条件を３０％から０％に変更。
//	2007.02.26 #004 m-onouchi ニアフル発生中のテーブルクリアでニアフル解除を送信しない不具合を修正。
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
	{RAU_IN_PARKING_SIZE	, 20	},		// ID20		入庫データ
	{RAU_OUT_PARKING_SIZE	, 21	},		// ID21		出庫データ
	{RAU_PAID_DATA22_SIZE	, 22	},		// ID22		精算データ
	{RAU_PAID_DATA23_SIZE	, 23	},		// ID23		精算データ
	{RAU_ERROR_SIZE			, 120	},			// ID120	エラーデータ
	{RAU_ALARM_SIZE			, 121	},			// ID121	アラームデータ
	{RAU_MONITOR_SIZE		, 122	},		// ID122	モニタデータ
	{RAU_OPE_MONITOR_SIZE	, 123	},		// ID123	操作モニタデータ
	{RAU_COIN_TOTAL_SIZE	, 131	},		// ID131	コイン金庫集計合計データ
	{RAU_MONEY_TORAL_SIZE	, 133	},		// ID133	紙幣金庫集計合計データ
	{RAU_PARKING_NUM_SIZE	, 236	},		// ID236	駐車台数データ
	{RAU_AREA_NUM_SIZE		, 237	},		// ID237	区画台数・満車データ
	{RAU_MONEY_MANAGE_SIZE	, 135	},		// ID135	釣銭管理集計データ
	{RAU_TOTAL_SIZE			, 0	},			// ID41		T合計集計データ
	{RAU_TOTAL_SIZE			, 0	},			// ID41		GT合計集計データ
	{RAU_MONEY_MANAGE_SIZE	, 126	},		// ID126	金銭管理データ
	{RAU_REMOTE_MONITOR_SIZE, 125	},		// ID125	遠隔監視データ
	{RAU_CENTER_TERM_INFO_SIZE	, 65	},	// ID65		センター用端末情報データ
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	{RAU_LONG_PARKING_INFO_SIZE	, 61	},	// ID61		長期駐車情報データ
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	{RAU_SEND_THROUGH_DATA_SIZE	, 0	},		// 送信スルーデータ（ID 不定）※送信のみ32blockに拡張 2006.08.31:m-onouchi
	{RAU_RECEIVE_THROUGH_DATA_SIZE	, 0	}	// 受信スルーデータ（ID 不定）
};


typedef struct {
	uchar	id;
	uchar	index;
} RAU_ID_BUFF_INFO;

const RAU_ID_BUFF_INFO	RAU_id_buff[RAU_ID_BUFF_NUM] = 
{
	{20		,RAU_IN_PARKING_TABLE			},		// ID20		入庫データ
	{21		,RAU_OUT_PARKING_TABLE			},		// ID21		出庫データ
	{22		,RAU_PAID_DATA22_TABLE			},		// ID22		精算データ
	{23		,RAU_PAID_DATA23_TABLE			},		// ID23		精算データ
	{54		,RAU_IN_PARKING_TABLE			},		// ID54		入庫データ
	{55		,RAU_OUT_PARKING_TABLE			},		// ID55		出庫データ
	{56		,RAU_PAID_DATA22_TABLE			},		// ID56		精算データ
	{57		,RAU_PAID_DATA23_TABLE			},		// ID57		精算データ
	{120	,RAU_ERROR_TABLE				},		// ID120	エラーデータ
	{121	,RAU_ALARM_TABLE				},		// ID121	アラームデータ
	{122	,RAU_MONITOR_TABLE				},		// ID122	モニタデータ
	{123	,RAU_OPE_MONITOR_TABLE			},		// ID123	操作モニタデータ
	{131	,RAU_COIN_TOTAL_TABLE			},		// ID131	コイン金庫集計合計データ
	{133	,RAU_MONEY_TORAL_TABLE			},		// ID133	紙幣金庫集計合計データ
	{236	,RAU_PARKING_NUM_TABLE			},		// ID236	駐車台数データ
	{58		,RAU_PARKING_NUM_TABLE			},		// ID58		Web用駐車台数データ
	{237	,RAU_AREA_NUM_TABLE				},		// ID237	区画台数・満車データ
	{30		,RAU_TOTAL_TABLE				},		// ID30		T合計集計データ
	{31		,RAU_TOTAL_TABLE				},		// ID31		T合計集計データ
	{32		,RAU_TOTAL_TABLE				},		// ID32		T合計集計データ
	{33		,RAU_TOTAL_TABLE				},		// ID33		T合計集計データ
	{34		,RAU_TOTAL_TABLE				},		// ID34		T合計集計データ
	{35		,RAU_TOTAL_TABLE				},		// ID35		T合計集計データ
	{36		,RAU_TOTAL_TABLE				},		// ID36		T合計集計データ
	{37		,RAU_TOTAL_TABLE				},		// ID37		T合計集計データ
	{38		,RAU_TOTAL_TABLE				},		// ID38		T合計集計データ
	{41		,RAU_TOTAL_TABLE				},		// ID41		T合計集計データ
	{42		,RAU_TOTAL_TABLE				},		// ID42		フォーマットRevNo.10 集計基本データ
	{43		,RAU_TOTAL_TABLE				},		// ID43		フォーマットRevNo.10 料金種別毎集計データ
	{45		,RAU_TOTAL_TABLE				},		// ID45		フォーマットRevNo.10 割引集計データ
	{46		,RAU_TOTAL_TABLE				},		// ID46		フォーマットRevNo.10 定期集計データ
	{49		,RAU_TOTAL_TABLE				},		// ID49		フォーマットRevNo.10 タイムレジ集計データ
	{53		,RAU_TOTAL_TABLE				},		// ID53		フォーマットRevNo.10 集計終了通知データ
	{126	,RAU_MONEY_MANAGE_TABLE			},		// ID126	金銭管理データ
	{135	,RAU_TURI_MANAGE_TABLE			},		// ID135	釣銭管理集計データ
	{125	,RAU_REMOTE_MONITOR_TABLE		},		// ID125	遠隔監視データ
	{65		,RAU_CENTER_TERM_INFO_TABLE		},		// ID65		センター用端末情報データ
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	{61		,RAU_LONG_PARKING_INFO_TABLE	},		// ID61		長期駐車情報データ
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	{0		,RAU_SEND_THROUGH_DATA_TABLE	},		// 送信スルーデータ		（ID 不定）
	{0		,RAU_RECEIVE_THROUGH_DATA_TABLE	},		// 受信スルーデータ		（ID 不定）
};

const	short	RAU_id_LogIDList[RAU_ID_LOGIDLIST_COUNT] = {
// MH810100(S) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
//	eLOG_ENTER,				// 入庫
	-1,						// 入庫
// MH810100(E) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
	-1,						// 出庫
	eLOG_PAYMENT,			// 精算(ID22)
	-1,						// 精算(ID23)
	eLOG_ERROR,				// エラー
	eLOG_ALARM,				// アラーム
	eLOG_MONITOR,			// モニタ
	eLOG_OPERATE,			// 操作
	eLOG_COINBOX,			// コイン金庫集計(ramのみ)
	eLOG_NOTEBOX,			// 紙幣金庫集計(ramのみ)
// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	eLOG_PARKING,			// 駐車台数データ
	-1,						// 駐車台数データ
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	-1,						// 区画台数・満車データ
	eLOG_MNYMNG_SRAM,		// 釣銭管理(SRAM)
	eLOG_TTOTAL,			// T集計
	eLOG_GTTOTAL,			// GT集計
	eLOG_MONEYMANAGE,		// 金銭管理(SRAM)
	eLOG_REMOTE_MONITOR,	// 遠隔監視
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	eLOG_LONGPARK_PWEB,		// 長期駐車(ParkingWeb用)
	-1,						// 長期駐車(ParkingWeb用)
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
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

void	data_rau_init(void);				// RAUイニシャライズ

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
//※ sizeof(struct _ctib)は調整バイト込みで4+4+237+1=246
#define	_CENTER_TERM_INFO_SIZE		237		// リモートNT-NETヘッダ10bytes + データ225bytes + CRC2bytes
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
				// 領域末端を超過
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
 *|			NTデータバッファ管理情報を初期化（NGならリセット）
 *[]----------------------------------------------------------------------[]*
 *|	param	*ntman		: NTデータバッファ管理情報
 *|	param	*dirty		: T合計データ群（基本データ～終了通知データまで）
 *|						  受信するまでの仮NTデータバッファ管理情報
 *|	param	*buff		: NTデータバッファエリア
 *|	param	size		: NTデータバッファエリアサイズ
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
		// 領域末端を超過
			remain = p - ntman->bottom;
			p = ntman->top + remain;
		}
		ntman->freearea = p;
	}
	ntman->sendnow = ntman->sendtop;		// 先頭から再送
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
return	確保できた場合：新しいフリーエリア先頭
*/

struct _ntbuf	*alloc_ntbuf(struct _ntman *ntman, char *dirty, int length)
{
	long	remain;
	char	*free = ntman->freearea;
	char	*datatop;
/* check free area size */
	length = ALIGN(length);	// アライメント調整
	length += (int)sizeof(struct _ntbuf);	// ntbufサイズを加算

	// 要求された長さが確保できるかチェック

	// sendtopが無いとき、dirtyの位置でチェックする
	// T合計のとき、一連のデータ群を受信終わるまでsendtopに登録しないので
	// 先頭データがないときも、バッファを使用していることがある。
	datatop = ntman->sendtop;
	if (datatop == NULL) {
		datatop = dirty;		// 一時的なデータも確保されていない？
	}
	if (datatop == NULL) {
		// datatopがNULL　データがない
		
		// 余りは領域の先頭から末端
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
		return NULL;	// 長すぎるので確保できない

/* update freearea */
	// フリーエリアを長さ分移動

// 停電対策
	datatop = ntman->freearea + length;

	if (datatop >= ntman->bottom) {
// 領域末端を超過
//		      st           |----------length---------|f
//		|-----++++++++++++++------------|
//		t                               b
		// 超過した分を領域先頭から付け直し
		remain = datatop - ntman->bottom;
		datatop = ntman->top + remain;
	}

	if ((datatop+sizeof(struct _ntbuf)) >= ntman->bottom) {
		// 求められたフリーエリアの先頭にバッファ情報を書き込めない場合は先頭に設定する
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
		// 書き込んだ結果領域末端を超過する
		remain = ntman->bottom - p;//書き込み位置から終端までをコピーする
		memcpy(p, ntdata, remain);
		ntdata += remain;
		remain = ntbuf->length - remain;
		p = ntman->top;
	}
	else {
		// 超過しない
		remain = ntbuf->length;
		if (remain == 0)
			p = ntman->top;		// Length０のデータを書き込むときにすでに領域末端に到達していたため、先頭に戻す
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
		remain = ntman->bottom - p;//読み込み位置から終端までをコピーする
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
// バッファリセット時のイメージから、ここでsendtopがNULLになるとき、
// freeareaをバッファトップに戻したいところだが、T合計データの場合、
// 次のデータ群を受信途中（dirtyにバッファがある）のことがあるので、
// 変えることができない。

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
		// これ以上書き込むことができない
		if (tsumm->dirty.sendtop != NULL) {
			// 41を待っている最中なので、30番代開始前の状態に書き戻す
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

	m2.uc[0] = ((struct _ntdata*)ntdata)->code[0];		// 機種コード
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
		// 機種コード毎、同一端末no.であること
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
	ctim->readq = ctim->sendq;			// read_cash_infoでずれたreadqを戻す
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
/*	関数名		=  RAU_data_table_init()									*/
/*	機能		=  データテーブルエリア割付									*/
/*	概略　　　　=　データテーブルのサイズにより先頭ポインタを求める			*/
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
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	RAU_table_top[RAU_LONG_PARKING_INFO_TABLE]		= 0;
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
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

	rau_data_table_info[RAU_ERROR_TABLE].c_DataFullProc			= ucDataFullProc;	// エラーテーブルのデータ処理方法をセット
	rau_data_table_info[RAU_ALARM_TABLE].c_DataFullProc			= ucDataFullProc;	// アラームテーブルのデータ処理方法をセット
	rau_data_table_info[RAU_MONITOR_TABLE].c_DataFullProc		= ucDataFullProc;	// モニターテーブルのデータ処理方法をセット
	rau_data_table_info[RAU_OPE_MONITOR_TABLE].c_DataFullProc	= ucDataFullProc;	// 操作モニターテーブルのデータ処理方法をセット
	rau_data_table_info[RAU_REMOTE_MONITOR_TABLE].c_DataFullProc	= ucDataFullProc;	// 遠隔監視データテーブルのデータ処理方法をセット
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	// バッファフル時最古データ
	rau_data_table_info[RAU_LONG_PARKING_INFO_TABLE].c_DataFullProc	= ucDataFullProc;	// 長期駐車情報データテーブルのデータ処理方法をセット
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)

	reset_tsum((char*)RAU_table_data.total, RAU_TOTAL_SIZE);
	reset_center_term_info((char*)RAU_table_data.center_term_info);
}

/****************************************************************************/
/*	関数名		=  data_rau_init()											*/
/*	機能			=  通信によるイニシャライズ								*/
/*	概略　　　　	=　オールクリア											*/
/*	履歴			=  多造													*/
/****************************************************************************/
void	data_rau_init( void )
{
	/* テーブルデータエリア生成 */
	RAU_data_table_init();

	RAU_ui_txdlength_h_keep = 0;
	RAU_ui_txdlength_h_keep2 = 0;
	RAU_uc_retrycnt_h = 0;
	RAU_uc_retrycnt_h2 = 0;
	RAU_uc_rcvretry_cnt = 0;
	RAU_uc_retrycnt_reconnect_h = 0;

	RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);	// 送信シーケンス０(ＨＯＳＴへのテーブル送信待機状態)
	RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// 受信シーケンス０(通常状態)
	RAUhost_ClearSendRequeat();

	return;
}


/*[]----------------------------------------------------------------------[]*
 *| 	RAUdata_DelRcvNtData
 *[]----------------------------------------------------------------------[]*
 *|	ホストから受信したNT-NETデータを削除
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
 *|	ホストから受信したNT-NETデータのブロック数を取得
 *[]----------------------------------------------------------------------[]*
 *|	param	: none
 *[]----------------------------------------------------------------------[]*
 *|	return	: ホストから受信したNT-NETデータのブロック数(0～26:-1はデータ無し)
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
		// CRCをチェック
		uwork.sndntdata.nt_data_start = NULL;
		if (!RAUdata_GetNtData(table, &uwork.sndntdata)) {
			// テーブルデータのCRCが異常
			RAUhost_SetError(RAU_ERR_DATA_CRC_ERROR);	// エラーコード０５(バッファデータＣＲＣエラー)

			RAUdata_DelRcvNtData();		// データは削除
			return blknum;
		}
		// 第1ブロックデータポインタ取得
		rptr = RAU_raudata_ReadTable(table, table->pc_ReadPoint, (uchar*)&uwork, 2);
		// ブロック数算出
		blknum = 0;
		while (1) {
			blknum++;
			// ブロックデータヘッダ取得
			RAU_raudata_ReadTable(table, rptr, (uchar*)&uwork.ntdatahdr, sizeof(RAU_NTDATA_HEADER));
			if (uwork.ntdatahdr.id2 == 0x01) {
				break;		// 最終ブロック検出
			}
			if (blknum >= RAU_RECEIVE_NTBLKNUM_MAX) {
				RAUdata_DelRcvNtData();
				return -1;	// 最大ブロック数オーバー(このパスは通らないはず)
			}
			// 次のブロックの先頭へ
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
 *|	ホストから受信したNT-NETデータを取得
 *[]----------------------------------------------------------------------[]*
 *|	param	: data	=取得データ書込み先領域						<OUT>
 *|			  blkno	=取得したいブロックNo. (1～26)
 *[]----------------------------------------------------------------------[]*
 *|	return	: dataにセットされたデータ長(0=データ無し)
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
		// 第1ブロックデータポインタ取得
		rptr = RAU_raudata_ReadTable(table, table->pc_ReadPoint, (uchar*)&dummy, sizeof(dummy));
		// 該当ブロック検索
		id2 = 0;
		for (i = 0; i < blkno; i++) {
			if (id2 != 0x00) {
				break;		// 最終ブロック検出
			}
			// 次のブロックの先頭へ
			rptr = RAU_ptrSeek(rptr, ulen.w, table->pc_AreaTop, tblend);
			// ブロックデータヘッダ取得
			RAU_raudata_ReadTable(table, rptr, data, sizeof(RAU_NTDATA_HEADER));
			ulen.c[0] = ((RAU_NTDATA_HEADER*)data)->data_len[0];
			ulen.c[1] = ((RAU_NTDATA_HEADER*)data)->data_len[1];
			id2 = ((RAU_NTDATA_HEADER*)data)->id2;
		}
		if (i == blkno) {
			RAU_raudata_ReadTable(table, rptr, data, ulen.w);	// 該当ブロック読み出し
		}
		else {
			// 該当ブロック不在
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

	ret = RAU_SEND_THROUGH_DATA_TABLE;	// 送信用スルーデータのインデックス

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
 *|	NTデータを目標のテーブルに格納する｡
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

	ushort	wWriteLen1;	// 折り返して、コピーを行うときの長さ前半
	ushort	wWriteLen2;	// 折り返して、コピーを行うときの長さ後半

	if (RAUdata_CheckCapacity(pBuffInfo, wDataLen) == FALSE) {
		// 領域の残り容量をチェック
		// 不足で書き込めない

			// バッファフルエラー発生
				
			// ニアフルステータス変更

			if (pBuffInfo->c_DataFullProc == 1) {
				// 最新上書き 
				if (pBuffInfo->ui_SendDataCount < pBuffInfo->ui_DataCount) {
					// 全データが送信対象になっていなければ、
					// 書き込める量が空くまで、最新データを削除。
					while (1) {
						RAUdata_DelNtData(pBuffInfo, 0);
						if (RAUdata_CheckCapacity(pBuffInfo, wDataLen) == TRUE) {
							break;
						}
						if (pBuffInfo->ui_SendDataCount >= pBuffInfo->ui_DataCount) {
							// 送信中データがあるので、削除できない｡
							return FALSE;
						}
						if (pBuffInfo->ui_DataCount == 0) {
							// データ件数が０になっても書き込めないので、バッファフル。
							return FALSE;
						}
					}
				} else {
					// 送信中データがあるので、削除できない｡
					return FALSE;
				}
			} else if (pBuffInfo->c_DataFullProc == 2) {
				// 最古上書き
				if (pBuffInfo->ui_SendDataCount == 0) {
					// 送信中データが無いとき｡
					// 書き込める量が空くまで、最古データを削除。
					while (1) {
						RAUdata_DelNtData(pBuffInfo, 1);
						if (RAUdata_CheckCapacity(pBuffInfo, wDataLen) == TRUE) {
							break;
						}
						if (pBuffInfo->ui_DataCount == 0) {
							// データ件数が０になっても書き込めないので、バッファフル。
							return FALSE;
						}
					}
				} else {
					// 送信中データがあるので、削除できない｡
					return FALSE;
				}
			} else {
				// 書き込み禁止
				// バッファフルエラーで書き込み禁止。アークブロック送受信結果82h
				return FALSE;
			}
		
	} else {
	}

	// 書き込み後の最新の書き込み位置とバッファの終端を比較する｡
	if ((pBuffInfo->pc_WritePoint + wDataLen) < (pBuffInfo->pc_AreaTop + pBuffInfo->ul_AreaSize) ) {
		// 書き込むデータの終端（CRCまで含んでいる）が、領域の範囲内に収まっている｡
		// データを書き込み
		memcpy(pBuffInfo->pc_WritePoint, pData, wDataLen);

		// データ書き込み位置を更新
		pBuffInfo->pc_WritePoint += wDataLen;

	} else {
		// データが終端位置をまたぐとき。
		// 折り返し位置を特定
		wWriteLen2 = (pBuffInfo->pc_WritePoint + wDataLen) - (pBuffInfo->pc_AreaTop + pBuffInfo->ul_AreaSize);
		wWriteLen1 = wDataLen - wWriteLen2;

		// 前半を書き込み
		memcpy(pBuffInfo->pc_WritePoint, pData, wWriteLen1);

		// 後半を書き込み（領域先頭から書き込み）
		memcpy(pBuffInfo->pc_AreaTop, pData + wWriteLen1, wWriteLen2);

		// データ書き込み位置を更新
		pBuffInfo->pc_WritePoint = pBuffInfo->pc_AreaTop + wWriteLen2;

	}

	// データ件数インクリメント
	pBuffInfo->ui_DataCount++;

	return TRUE;
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	目標のバッファから指定長データを読み出す｡
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

	ushort	wReadLen1;	// 折り返して、コピーを行うときの長さ前半
	ushort	wReadLen2;	// 折り返して、コピーを行うときの長さ後半

	// 書き込み位置＋書き込み量　と　バッファ終端　を比較
	if ((pSendNtDataInf->nt_data_next + wReadLen) < (pBuffInfo->pc_AreaTop + pBuffInfo->ul_AreaSize) ) {
		// 超えないとき。長さ分バッファにコピー
		memcpy(pData, pSendNtDataInf->nt_data_next, wReadLen);

		// 次回書き込み位置をコピーした長さ分進める｡
		pSendNtDataInf->nt_data_next += wReadLen;

	} else {
		// 超えるとき。
		// 折り返し位置を算出
		wReadLen2 = (pSendNtDataInf->nt_data_next + wReadLen) - (pBuffInfo->pc_AreaTop + pBuffInfo->ul_AreaSize);
		wReadLen1 = wReadLen - wReadLen2;

		// 前半をコピー
		memcpy(pData, pSendNtDataInf->nt_data_next, wReadLen1);

		// 後半をコピー
		memcpy(pData + wReadLen1, pBuffInfo->pc_AreaTop , wReadLen2);

		// 次回書き込み位置をコピーした長さ分進める｡
		pSendNtDataInf->nt_data_next = pBuffInfo->pc_AreaTop + wReadLen2;

	}
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	現在のバッファの先頭からN番目のデータのアドレスを返す
 *[]----------------------------------------------------------------------[]*
 *|	patam	pBuffInfo	バッファ情報構造体
 *|	patam	wCount		取得したいデータのアドレス
 *[]----------------------------------------------------------------------[]*
 *|	return	N番目のデータの先頭アドレス。見つからない場合はNULL
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
		// バッファにたまっている数より多い、要求がされたらNULLを返す｡
		return (uchar *)NULL;
	}

	// 先頭データのアドレスを設定する｡
	pNtData = pBuffInfo->pc_ReadPoint;

	// 末端の位置　=　先頭の位置　＋　サイズ
	pBuffEnd = pBuffInfo->pc_AreaTop + pBuffInfo->ul_AreaSize;

	for (i = 0; i < wCount; i++) {

		// データの長さを求める｡()
		u.uc[0] = pNtData[0];
		if (&pNtData[1] < pBuffEnd) {	// 長さの2バイト目が末端を越えているかチェック
			// 超えていないのでそのまま使用。
			u.uc[1] = pNtData[1];
		} else {
			// 超えているので、バッファの先頭が長さの2バイト目。
			u.uc[1] = pBuffInfo->pc_AreaTop[0];
		}

		// 次の位置　=　位置＋長さ（２）＋CRCの長さ（２）
		pNtData += u.us + 2;

		if (pNtData >= pBuffEnd) {
			// 次の位置　≧　末端の位置（折り返し必要）
			// 次の位置　＝　先頭の位置　＋　（次の位置　－　末端の位置）
			pNtData = pBuffInfo->pc_AreaTop + (pNtData - pBuffEnd);
		}
/*
		else {
			// 次の位置　＜　末端の位置（折り返し不要）
		}
*/
	}
	return pNtData;
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	目的のバッファのNTデータを1件削除する｡
 *[]----------------------------------------------------------------------[]*
 *|	patam	pBuffInfo	バッファ情報構造体
 *|	patam	position	削除位置
 *[]----------------------------------------------------------------------[]*
 *|	return	void
 *[]----------------------------------------------------------------------[]*
 *| Author	:	J.Mitani
 *| Date	:	2005-09-16
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAUdata_DelNtData(RAU_DATA_TABLE_INFO *pBuffInfo, uchar cPosition) {

	uchar	endDataId;

//	集計データは一括に削除する必要があるため、RAUdata_DeleteTsumGroup()を使用していてここに来ることはありません
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
		// 前から消す
			if (pBuffInfo->ui_DataCount > 1) {
			// 残りデータ数は複数
				// 削除後の次の先頭位置を取得
				pBuffInfo->pc_ReadPoint = RAUdata_GetNtDataAdrr(pBuffInfo, 1);
				// データ数をデクリメント
				pBuffInfo->ui_DataCount--;
			
			} else {
			// 最後のひとつ
				// リードポイント　＝　ライトポイント
				pBuffInfo->pc_ReadPoint = pBuffInfo->pc_WritePoint;
				// データ数　＝　０
				pBuffInfo->ui_DataCount = 0;
			}
		
		} else {
		// 後ろから消す
			if (pBuffInfo->ui_DataCount > 1) {
			// 残りデータ数は複数
				// 削除後の次の先頭位置を取得
				pBuffInfo->pc_WritePoint = RAUdata_GetNtDataAdrr(pBuffInfo, pBuffInfo->ui_DataCount - 1);
				// データ数をデクリメント
				pBuffInfo->ui_DataCount--;
			
			} else {
			// 最後のひとつ
				// ライトポイント　＝　リードポイント
				pBuffInfo->pc_WritePoint = pBuffInfo->pc_ReadPoint;
				// データ数　＝　０
				pBuffInfo->ui_DataCount = 0;
			}
		}
	}

}

/*[]----------------------------------------------------------------------[]*
 *|             RAUdata_GetNtDataNextAdrr()
 *[]----------------------------------------------------------------------[]*
 *|	次のNTデータの先頭アドレスを取得する
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

	// NTデータが領域の終端をまたがっているか確認｡
	if (pSendNtDataInf->nt_data_start < pSendNtDataInf->nt_data_end) {
		// またがっていない｡
		
		// データのはみ出し量を求める｡
		switch ((pBuffInfo->pc_AreaTop + pBuffInfo->ul_AreaSize) - pSendNtDataInf->nt_data_end) {
		case 0:
			// CRCが領域先頭ににあるので、次のデータは先頭から2進めた場所から。
			pNtData = pBuffInfo->pc_AreaTop + 2;
			break;
		case 1:
			// CRCがまたがっているので、次のデータは先頭から1進めた場所から。
			pNtData = pBuffInfo->pc_AreaTop + 1;
			break;
		case 2:
			// CRCが領域内に収まるので、次のデータは領域先頭から。
			pNtData = pBuffInfo->pc_AreaTop;
			break;
		default:
			// 次のデータはこのデータの後から始まる｡
			pNtData = pSendNtDataInf->nt_data_end + 2;
			break;
		}
	} else {
		// またがっている。
		pNtData = pSendNtDataInf->nt_data_end + 2;
		
	}

	return pNtData;
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	NTデータ情報を取得する
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

	// 送信用NTデータ情報がNULLでなければ、一度データを取得しているはずなので、次のデータを探す｡
	if (pSendNtDataInf->nt_data_start == NULL) {
		// データがNULL
//		pSendNtDataInf->nt_data_start = pSendNtDataInf->nt_data_next = pBuffInfo->pc_ReadPoint;
		pSendNtDataInf->nt_data_len = pBuffInfo->pc_ReadPoint;
		
	} else {
		// データがNULLでない
//		pSendNtDataInf->nt_data_start  = pSendNtDataInf->nt_data_next = RAUdata_GetNtDataNextAdrr(pBuffInfo, pSendNtDataInf);
		pSendNtDataInf->nt_data_len = RAUdata_GetNtDataNextAdrr(pBuffInfo, pSendNtDataInf);
	}

	// NTデータ終了位置は計算して求める｡

	// 末端の位置　=　先頭の位置　＋　サイズ
	pBuffEnd = pBuffInfo->pc_AreaTop + pBuffInfo->ul_AreaSize;

	u.uc[0] = pSendNtDataInf->nt_data_len[0];
	if (&pSendNtDataInf->nt_data_len[1] < pBuffEnd) {	// 長さの2バイト目がバッファの末端を越えているかチェック
		// 超えていないのでそのまま使用。
		u.uc[1] = pSendNtDataInf->nt_data_len[1];
	} else {
		// 超えているので、バッファの先頭が長さの2バイト目。
		u.uc[1] = pBuffInfo->pc_AreaTop[0];
	}

	if (u.us < 12) {
		return FALSE;
	}


	if (&pSendNtDataInf->nt_data_len[1] >= pBuffEnd) {	// データの先頭がバッファの末端を越えているかチェック
		// 長さが終端をまたいでいる｡
		// データ本体は領域先頭から1バイト目
		pSendNtDataInf->nt_data_start = &pBuffInfo->pc_AreaTop[1];
	} else if (&pSendNtDataInf->nt_data_len[2] >= pBuffEnd) {
		// 長さが終端で終わっている｡
		// 領域先頭がデータの先頭
		pSendNtDataInf->nt_data_start = pBuffInfo->pc_AreaTop;
	} else {
		pSendNtDataInf->nt_data_start = &pSendNtDataInf->nt_data_len[2];
	}

	pSendNtDataInf->nt_data_next = pSendNtDataInf->nt_data_start;

	pSendNtDataInf->nt_data_end = pSendNtDataInf->nt_data_len + u.us;

	// データがバッファの終端を超えている。
	if (pSendNtDataInf->nt_data_end >= pBuffEnd) {
		// 次の位置　＞　末端の位置（折り返し必要）
		// 次の位置　＝　先頭の位置　＋　（次の位置　－　末端の位置）
		pSendNtDataInf->nt_data_end = pBuffInfo->pc_AreaTop + (pSendNtDataInf->nt_data_end - pBuffEnd);
	}

	// 対象データのCRC計算を行う｡
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
	
	// 対象データのCRCを取得する｡
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

	// CRCチェックを行う｡
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
 *|	送信対象データの生成（スルーデータ）
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
 *|	送信対象データの生成（テーブルデータ）
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
	ushort	sendCnt = 0;	// 全データ送信要求によるデータ送信を判断するための送信データ数カウンタ

	if( ucSendReq ){					// ＩＢＷまたはホストからの送信要求データを受信している。
// MH810100(S) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
//		if( pReqInfo->in_parking ){		// 20:入庫データの送信を要求
//			pSendDataInfo[RAU_IN_PARKING_TABLE].send_req = 1;
//			if(RauConfig.id20_21_mask_flg == 0){		// 入庫データの送信マスクなし
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
// MH810100(E) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)

// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		if(pReqInfo->out_parking ){		// 21:出庫データの送信を要求
//			pSendDataInfo[RAU_OUT_PARKING_TABLE].send_req = 1;
//			if(RauConfig.id20_21_mask_flg == 0){		// 出庫データの送信マスクなし
//				// 出庫データは０件データのみ送信
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
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

		if( pReqInfo->paid_data ){		// 22,23:精算データの送信を要求
			pSendDataInfo[RAU_PAID_DATA22_TABLE].send_req = 1;
			if(RauConfig.id22_23_mask_flg == 0){		// 精算データの送信マスクなし
				// 事前精算データは０件データのみ送信
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
			if(RauConfig.id22_23_mask_flg == 0){		// 精算データの送信マスクなし
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

		if( pReqInfo->error ){		// 120:エラーデータの送信を要求
			pSendDataInfo[RAU_ERROR_TABLE].send_req = 1;
			if( (RauConfig.id120_mask_flg == 0) &&
				(RauConfig.error_send_level != 9) ){		// エラーデータの送信マスクなし
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

		if( pReqInfo->alarm ){		// 121:アラームデータの送信を要求
			pSendDataInfo[RAU_ALARM_TABLE].send_req = 1;
			if( (RauConfig.alarm_send_level != 9) &&
				(RauConfig.id121_mask_flg == 0) ){			// アラームデータの送信マスクなし
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

		if( pReqInfo->monitor ){	// 122:モニタデータの送信を要求
			pSendDataInfo[RAU_MONITOR_TABLE].send_req = 1;
			if( (RauConfig.monitor_send_level != 9) &&
				(RauConfig.id122_mask_flg == 0) ){			// モニタデータの送信マスクなし
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

		if( pReqInfo->ope_monitor ){	// 123:操作モニタデータの送信を要求
			pSendDataInfo[RAU_OPE_MONITOR_TABLE].send_req = 1;
			if( (RauConfig.opemonitor_send_level != 9) &&
				(RauConfig.id123_mask_flg == 0) ){			// 操作モニタデータの送信マスクなし
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

		if( pReqInfo->coin_total ){	// 131:コイン金庫集計合計データの送信を要求
			pSendDataInfo[RAU_COIN_TOTAL_TABLE].send_req = 1;
			if(RauConfig.id131_mask_flg == 0){				// コイン金庫集計合計データの送信マスクなし
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

		if( pReqInfo->money_toral ){	// 133:紙幣金庫集計合計データの送信を要求
			pSendDataInfo[RAU_MONEY_TORAL_TABLE].send_req = 1;
			if(RauConfig.id133_mask_flg == 0){				// 紙幣金庫集計合計データの送信マスクなし
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

// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		if( pReqInfo->parking_num ){	// 236:駐車台数データの送信を要求
//			pSendDataInfo[RAU_PARKING_NUM_TABLE].send_req = 1;
//			if(RauConfig.id236_mask_flg == 0){				// 駐車台数データの送信マスクなし
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
//		if( pReqInfo->area_num ){	// 237:区画台数・満車データの送信を要求
//			pSendDataInfo[RAU_AREA_NUM_TABLE].send_req = 0;
//			pSendDataInfo[RAU_AREA_NUM_TABLE].send_data_count = 0;
//			rau_data_table_info[RAU_AREA_NUM_TABLE].ui_SendDataCount = pSendDataInfo[RAU_AREA_NUM_TABLE].send_data_count;
//			pSendDataInfo[RAU_AREA_NUM_TABLE].send_complete_count = 0;
//			pSendDataInfo[RAU_AREA_NUM_TABLE].fill_up_data_count = 0;
//			pSendDataInfo[RAU_AREA_NUM_TABLE].crc_err_data_count = 0;
//			pSendDataInfo[RAU_AREA_NUM_TABLE].pbuff_info = &rau_data_table_info[RAU_AREA_NUM_TABLE];
//			// 区画台数・満車データはデータ送信要求で要求されることは無いため、カウントアップしない
//		}
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

		if( pReqInfo->total ){	// 30～38,41:Ｔ合計集計データの送信を要求
			logCount = Ope_Log_UnreadCountGet(eLOG_TTOTAL, eLOG_TARGET_REMOTE);
			if(logCount > 0){
				pSendDataInfo[RAU_TOTAL_TABLE].send_req = 1;
				pSendDataInfo[RAU_TOTAL_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].pbuff_info = &rau_data_table_info[RAU_TOTAL_TABLE];
				// ログから集計データを取得し、tsumデータに格納
				if(RauConfig.serverTypeSendTsum == 0){				// 34-0026④ 0:送信しない 1:送信する
					for(i=0; i<logCount; i++){
						Ope_Log_TargetDataGet( eLOG_TTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
					}
					pSendDataInfo[RAU_TOTAL_TABLE].send_data_count = 0;
				}
			}
			logCount = Ope_Log_UnreadCountGet(eLOG_GTTOTAL, eLOG_TARGET_REMOTE);
			if(logCount > 0){
				// ID42,43,45,46,49,53:ＧＴ合計集計データの送信を要求
// MH322914 (s) kasiyama 2016/07/07 同じT合計データが再送され続け、以後のT合計が送信されない(共通改善No.1250)
//				pSendDataInfo[RAU_GTOTAL_TABLE].send_req = 1;
//				pSendDataInfo[RAU_GTOTAL_TABLE].send_complete_count = 0;
//				pSendDataInfo[RAU_GTOTAL_TABLE].fill_up_data_count = 0;
//				pSendDataInfo[RAU_GTOTAL_TABLE].crc_err_data_count = 0;
//				pSendDataInfo[RAU_GTOTAL_TABLE].pbuff_info = &rau_data_table_info[RAU_GTOTAL_TABLE];
//				if(RauConfig.serverTypeSendTsum == 0){				// 34-0026④ 0:送信しない 1:送信する
//					for(i=0; i<logCount; i++){
//						Ope_Log_TargetDataGet( eLOG_GTTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
//					}
//					pSendDataInfo[RAU_GTOTAL_TABLE].send_data_count = 0;
//				}
				if(prm_get(COM_PRM,S_NTN,121,1,1) == 0) {
					// ParkRingの場合はGT合計を送信しない
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
					if(RauConfig.serverTypeSendTsum == 0){				// 34-0026④ 0:送信しない 1:送信する
						for(i=0; i<logCount; i++){
							Ope_Log_TargetDataGet( eLOG_GTTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
						}
						pSendDataInfo[RAU_GTOTAL_TABLE].send_data_count = 0;
					}
				}
// MH322914 (e) kasiyama 2016/07/07 同じT合計データが再送され続け、以後のT合計が送信されない(共通改善No.1250)
			}
			if((pSendDataInfo[RAU_TOTAL_TABLE].send_req == 0) &&
				(pSendDataInfo[RAU_GTOTAL_TABLE].send_req == 0)) {
				// T合計及びGT合計の件数が0件の場合、T合計の0件データを送る
				pSendDataInfo[RAU_TOTAL_TABLE].send_req = 1;
				pSendDataInfo[RAU_TOTAL_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].pbuff_info = &rau_data_table_info[RAU_TOTAL_TABLE];
			}
			sendCnt++;
		}

		if( pReqInfo->money_manage ){	// 126:金銭管理データの送信を要求
			pSendDataInfo[RAU_MONEY_MANAGE_TABLE].send_req = 1;
			if(RauConfig.id126_mask_flg == 0){				// 金銭管理データの送信マスクなし
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

		if( pReqInfo->turi_manage ){	// 135:釣銭管理集計データの送信を要求
			pSendDataInfo[RAU_TURI_MANAGE_TABLE].send_req = 1;
			if(RauConfig.id135_mask_flg == 0){				// 釣銭管理集計データの送信マスクなし
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

		if( pReqInfo->rmon ){			// 125:遠隔監視データの送信を要求
			pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_req = 1;
			if( (RauConfig.rmon_send_level != 9) &&
				(RauConfig.id125_mask_flg == 0) ){			// 遠隔監視データの送信マスクなし
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

// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//		if( pReqInfo->long_park ){		// 61:長期駐車状態データの送信を要求
//			pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_req = 1;
//			if(RauConfig.id61_mask_flg == 0){			// 長期駐車状態データの送信マスクなし
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
//// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

		if( sendCnt == REQUEST_TABLE_MAX - 1 && 	// 65:センター用端末情報データを送信（全データ送信要求時）
													// 区画台数・満車データはデータ送信要求で要求されることは無いため-1する
			check_center_term_que(RAU_centerterminfo_man.sendq) > 0 ) {
			pSendDataInfo[RAU_CENTER_TERM_INFO_TABLE].send_req = 1;
			pSendDataInfo[RAU_CENTER_TERM_INFO_TABLE].send_data_count = check_center_term_que(RAU_centerterminfo_man.sendq);
			pSendDataInfo[RAU_CENTER_TERM_INFO_TABLE].send_complete_count = 0;
			pSendDataInfo[RAU_CENTER_TERM_INFO_TABLE].fill_up_data_count = 0;
			pSendDataInfo[RAU_CENTER_TERM_INFO_TABLE].crc_err_data_count = 0;
			pSendDataInfo[RAU_CENTER_TERM_INFO_TABLE].pbuff_info = &rau_data_table_info[RAU_CENTER_TERM_INFO_TABLE];
		}
	} else {	// 送信要求データの受信なし
		if( RauConfig.Dpa_proc_knd == 1 ){	// ＤｏＰａ処理区分"(0:常時通信なし, 1:常時通信あり)
			for( i = 0; i < RAU_TOTAL_TABLE; i++ ){
// MH810100(S)
				// 連続を考慮して一度taskchg
				taskchg(IDLETSKNO);
// MH810100(E)
				pSendDataInfo[i].send_data_count = 0;
				switch(i){
				case RAU_IN_PARKING_TABLE:			// 入庫データ
// MH810100(S) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
//					if(RauConfig.id20_21_mask_flg == 0){
//						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_ENTER, eLOG_TARGET_REMOTE);
//					}
// MH810100(E) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
					break;
				case RAU_OUT_PARKING_TABLE:			// 出庫データ
					break;
				case RAU_PAID_DATA22_TABLE:			// 精算データ(事前)
					if(RauConfig.id22_23_mask_flg == 0){
						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_PAYMENT, eLOG_TARGET_REMOTE);
					}
					break;
				case RAU_PAID_DATA23_TABLE:			// 精算データ(出口)
					if(RauConfig.id22_23_mask_flg == 0){
						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_PAYMENT, eLOG_TARGET_REMOTE);
					}
					break;
				case RAU_ERROR_TABLE:				// エラーデータ
					if( (RauConfig.id120_mask_flg == 0) &&
						(RauConfig.error_send_level != 9) ){
						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_ERROR, eLOG_TARGET_REMOTE);
					}
					break;
				case RAU_ALARM_TABLE:				// アラームデータ
					if( (RauConfig.id121_mask_flg == 0) &&
						(RauConfig.alarm_send_level != 9) ){
						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_ALARM, eLOG_TARGET_REMOTE);
					}
					break;
				case RAU_MONITOR_TABLE:				// モニターデータ
					if( (RauConfig.id122_mask_flg == 0) &&
						(RauConfig.monitor_send_level != 9) ){
						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_MONITOR, eLOG_TARGET_REMOTE);
					}
					break;
				case RAU_OPE_MONITOR_TABLE:			// 操作モニターデータ
					if( (RauConfig.id123_mask_flg == 0) &&
						(RauConfig.opemonitor_send_level != 9) ){
						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_OPERATE, eLOG_TARGET_REMOTE);
					}
					break;
				case RAU_COIN_TOTAL_TABLE:			// コイン金庫集計データ
					if(RauConfig.id131_mask_flg == 0){
						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_COINBOX, eLOG_TARGET_REMOTE);
					}
					break;
				case RAU_MONEY_TORAL_TABLE:			// 紙幣金庫集計データ
					if(RauConfig.id133_mask_flg == 0){
						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_NOTEBOX, eLOG_TARGET_REMOTE);
					}
					break;
				case RAU_PARKING_NUM_TABLE:			// 駐車台数・満車データ
// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//					if(RauConfig.id236_mask_flg == 0){
//						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_PARKING, eLOG_TARGET_REMOTE);
//					}
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
					break;
				case RAU_AREA_NUM_TABLE:			// 区画台数・満車データ
					break;
				case RAU_TURI_MANAGE_TABLE:			// 釣銭管理集計データ
					if(RauConfig.id135_mask_flg == 0){
						pSendDataInfo[i].send_data_count = Ope_Log_UnreadCountGet(eLOG_MNYMNG_SRAM, eLOG_TARGET_REMOTE);
					}
					break;
				default:
					break;
				}
				if( pSendDataInfo[i].send_data_count ){	// Ｔ合計集計と金銭管理を除くすべてのテーブルにデータさえあれば
					pSendDataInfo[i].send_req = 2;			// 送信対象とする。
					rau_data_table_info[i].ui_SendDataCount = pSendDataInfo[i].send_data_count;
					pSendDataInfo[i].send_complete_count = 0;
					pSendDataInfo[i].fill_up_data_count = 0;
					pSendDataInfo[i].crc_err_data_count = 0;
					pSendDataInfo[i].pbuff_info = &rau_data_table_info[i];
				}
			}

			logCount = Ope_Log_UnreadCountGet(eLOG_TTOTAL, eLOG_TARGET_REMOTE);
			if((logCount > 0) &&
				(RauConfig.id30_41_mask_flg == 0) &&			// Ｔ合計集計データ
				(RauConfig.serverTypeSendTsum != 0)) {		 	// 34-0026④ 0:送信しない 1:送信する
				pSendDataInfo[RAU_TOTAL_TABLE].send_req = 2;
				pSendDataInfo[RAU_TOTAL_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].pbuff_info = &rau_data_table_info[RAU_TOTAL_TABLE];
				// ログから集計データを取得し、tsumデータに格納
			}
			if(logCount > 0) {
				if(RauConfig.serverTypeSendTsum == 0){				// 34-0026④ 0:送信しない 1:送信する
					for(i=0; i<logCount; i++){
						Ope_Log_TargetDataGet( eLOG_TTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
					}
				}
			}
// MH810100(S)
			// 連続を考慮して一度taskchg
			taskchg(IDLETSKNO);
// MH810100(E)

			logCount = Ope_Log_UnreadCountGet(eLOG_GTTOTAL, eLOG_TARGET_REMOTE);
			if((logCount > 0) &&
				(RauConfig.id30_41_mask_flg == 0) &&			// ＧＴ合計集計データ
				(RauConfig.serverTypeSendTsum != 0)) {		 	// 34-0026④ 0:送信しない 1:送信する
				pSendDataInfo[RAU_GTOTAL_TABLE].send_req = 2;
				pSendDataInfo[RAU_GTOTAL_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_GTOTAL_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_GTOTAL_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_GTOTAL_TABLE].pbuff_info = &rau_data_table_info[RAU_GTOTAL_TABLE];
			}
			if(logCount > 0) {
// MH322914 (s) kasiyama 2016/07/07 同じT合計データが再送され続け、以後のT合計が送信されない(共通改善No.1250)
//				if(RauConfig.serverTypeSendTsum == 0){				// 34-0026④ 0:送信しない 1:送信する
				if(prm_get(COM_PRM,S_NTN,121,1,1) == 0) {
					// ParkRingの場合はGT合計を送信しない
					pSendDataInfo[RAU_GTOTAL_TABLE].send_req = 0;
				}
				if((RauConfig.serverTypeSendTsum == 0) ||				// 34-0026④ 0:送信しない 1:送信する
					(prm_get(COM_PRM,S_NTN,121,1,1) == 0)) {
					// ParkRingの場合はGT合計を送信しない
// MH322914 (e) kasiyama 2016/07/07 同じT合計データが再送され続け、以後のT合計が送信されない(共通改善No.1250)
					for(i=0; i<logCount; i++){
						Ope_Log_TargetDataGet( eLOG_GTTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
					}
				}
			}

			if ((Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE) > 0) &&
				(RauConfig.id126_mask_flg == 0)){				// 金銭管理データ
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].send_req = 2;
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE);
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_MONEY_MANAGE_TABLE].pbuff_info = &rau_data_table_info[RAU_MONEY_MANAGE_TABLE];
			}

			if( (RauConfig.id125_mask_flg == 0) &&		// 遠隔監視データ
				(RauConfig.rmon_send_level != 9) ){
				pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_REMOTE_MONITOR, eLOG_TARGET_REMOTE);
				if( pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_data_count ){	// Ｔ合計集計と金銭管理を除くすべてのテーブルにデータさえあれば
					pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_req = 2;			// 送信対象とする。
					rau_data_table_info[RAU_REMOTE_MONITOR_TABLE].ui_SendDataCount = pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_data_count;
					pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].send_complete_count = 0;
					pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].fill_up_data_count = 0;
					pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].crc_err_data_count = 0;
					pSendDataInfo[RAU_REMOTE_MONITOR_TABLE].pbuff_info = &rau_data_table_info[RAU_REMOTE_MONITOR_TABLE];
				}
			}

// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//			if (RauConfig.id61_mask_flg == 0){			// 長期駐車状態データ
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
//// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

		} else {	// 常時通信なし → 個別常時接続確認
// MH810100(S) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
//			if ((RauConfig.id20_21_tel_flg == 1) &&
//				(RauConfig.id20_21_mask_flg == 0) &&
//				(Ope_Log_UnreadCountGet(eLOG_ENTER, eLOG_TARGET_REMOTE) > 0)){	// ＩＢＷパラメータ"入庫/出庫データ自動発信発信ＴＥＬ番号"('0':自動発信する, '1':自動発信しない)
//				pSendDataInfo[RAU_IN_PARKING_TABLE].send_req = 2;
//				pSendDataInfo[RAU_IN_PARKING_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_ENTER, eLOG_TARGET_REMOTE);
//				rau_data_table_info[RAU_IN_PARKING_TABLE].ui_SendDataCount = pSendDataInfo[RAU_IN_PARKING_TABLE].send_data_count;
//				pSendDataInfo[RAU_IN_PARKING_TABLE].send_complete_count = 0;
//				pSendDataInfo[RAU_IN_PARKING_TABLE].fill_up_data_count = 0;
//				pSendDataInfo[RAU_IN_PARKING_TABLE].crc_err_data_count = 0;
//				pSendDataInfo[RAU_IN_PARKING_TABLE].pbuff_info = &rau_data_table_info[RAU_IN_PARKING_TABLE];
//			} 
// MH810100(E) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)

// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
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
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

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

// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
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
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

			// T合計
			logCount = Ope_Log_UnreadCountGet(eLOG_TTOTAL, eLOG_TARGET_REMOTE);
			if ((RauConfig.id30_41_tel_flg == 1) &&
				(RauConfig.id30_41_mask_flg == 0) &&
				(RauConfig.serverTypeSendTsum != 0) && 	// 34-0026④ 0:送信しない 1:送信する
				(logCount > 0)) {
				pSendDataInfo[RAU_TOTAL_TABLE].send_req = 2;
				pSendDataInfo[RAU_TOTAL_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_TOTAL_TABLE].pbuff_info = &rau_data_table_info[RAU_TOTAL_TABLE];
				// ログから集計データを取得し、tsumデータに格納
			} 
			if(logCount > 0) {
				if(RauConfig.serverTypeSendTsum == 0){				// 34-0026④ 0:送信しない 1:送信する
					for(i=0; i<logCount; i++){
						Ope_Log_TargetDataGet( eLOG_TTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
					}
				}
			}

			// GT合計
			logCount = Ope_Log_UnreadCountGet(eLOG_GTTOTAL, eLOG_TARGET_REMOTE);
			if ((RauConfig.id30_41_tel_flg == 1) &&
				(RauConfig.id30_41_mask_flg == 0) &&
				(RauConfig.serverTypeSendTsum != 0) && 	// 34-0026④ 0:送信しない 1:送信する
				(logCount > 0)) {
				pSendDataInfo[RAU_GTOTAL_TABLE].send_req = 2;
				pSendDataInfo[RAU_GTOTAL_TABLE].send_complete_count = 0;
				pSendDataInfo[RAU_GTOTAL_TABLE].fill_up_data_count = 0;
				pSendDataInfo[RAU_GTOTAL_TABLE].crc_err_data_count = 0;
				pSendDataInfo[RAU_GTOTAL_TABLE].pbuff_info = &rau_data_table_info[RAU_GTOTAL_TABLE];
			}
			if(logCount > 0) {
// MH322914 (s) kasiyama 2016/07/07 同じT合計データが再送され続け、以後のT合計が送信されない(共通改善No.1250)
//				if(RauConfig.serverTypeSendTsum == 0){				// 34-0026④ 0:送信しない 1:送信する
				if(prm_get(COM_PRM,S_NTN,121,1,1) == 0) {
					// ParkRingの場合はGT合計を送信しない
					pSendDataInfo[RAU_GTOTAL_TABLE].send_req = 0;
				}
				if((RauConfig.serverTypeSendTsum == 0) ||				// 34-0026④ 0:送信しない 1:送信する
					(prm_get(COM_PRM,S_NTN,121,1,1) == 0)) {
					// ParkRingの場合はGT合計を送信しない
// MH322914 (e) kasiyama 2016/07/07 同じT合計データが再送され続け、以後のT合計が送信されない(共通改善No.1250)
					for(i=0; i<logCount; i++){
						Ope_Log_TargetDataGet( eLOG_GTTOTAL, RAU_LogData, eLOG_TARGET_REMOTE, TRUE );
					}
				}
			}

			// 金銭管理
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
			// 釣銭管理集計
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

			// 遠隔監視データ
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

// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//			// 長期駐車状態データ
//			if ((RauConfig.id61_tel_flg == 1) &&// 長期駐車状態データの自動発信あり
//				(RauConfig.id61_mask_flg == 0) &&// 長期駐車状態データの送信マスクなし
//				(Ope_Log_UnreadCountGet(eLOG_LONGPARK_PWEB, eLOG_TARGET_REMOTE) > 0)) {
//				pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_req = 2;
//				pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_data_count = Ope_Log_UnreadCountGet(eLOG_LONGPARK_PWEB, eLOG_TARGET_REMOTE);
//				rau_data_table_info[RAU_LONG_PARKING_INFO_TABLE].ui_SendDataCount = pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_data_count;
//				pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].send_complete_count = 0;
//				pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].fill_up_data_count = 0;
//				pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].crc_err_data_count = 0;
//				pSendDataInfo[RAU_LONG_PARKING_INFO_TABLE].pbuff_info = &rau_data_table_info[RAU_LONG_PARKING_INFO_TABLE];
//			}
//// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
		}

		if( check_center_term_que(RAU_centerterminfo_man.sendq) > 0 ) { // 65:センター用端末情報データを送信（即時）
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
//	スルーデータの格納
//
//	@argument	*pData		格納するＮＴ－ＮＥＴ電文(先頭にデータ長、末尾にＣＲＣあり)
//	@argument	wDataLen	データ長(先頭のデータ長と末尾のＣＲＣを含めたバイト数)
//	@argument	ucDirection	格納するデータバッファ(0:RAU_SEND_THROUGH_DATA_TABLE, 1:RAU_RECEIVE_THROUGH_DATA_TABLE)
//
//	@return		RAU_DATA_NORMAL		格納成功
//	@return		RAU_DATA_BUFFERFULL	格納失敗(バッファフル)
//
//	@attention	ＨＯＳＴから受信したスルーデータは受信スルーデータバッファに
//				ＩＢＷから受信したスルーデータは送信スルーデータバッファに格納されます。
//
//	@note		ＨＯＳＴまたはＩＢＷから受信したスルーデータおよびＩＢＫが独自に作成した
//				送信データをスルーデータバッファへ格納します。
//
//	@author		2007.01.05:m-onouchi
//======================================== Copyright(C) 2007 AMANO Corp. =======
eRAU_TABLEDATA_SET	RAUdata_SetThroughData(uchar *pData, ushort wDataLen, uchar ucDirection) {

	eRAU_TABLEDATA_SET ret;

	ret = RAU_DATA_BUFFERFULL;

	if (ucDirection != 1) {
		// HOST向けスルーデータバッファに登録
		if (rau_data_table_info[RAU_SEND_THROUGH_DATA_TABLE].ui_DataCount == 0) {
			if(RAUdata_WriteNtData(pData, wDataLen, &rau_data_table_info[RAU_SEND_THROUGH_DATA_TABLE]) == TRUE) {
				ret = RAU_DATA_NORMAL;
			} else {
				ret = RAU_DATA_BUFFERFULL;
			}
		}
	} else {
		// IBW向けスルーデータバッファに登録
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
 *|	任意の長さのデータが目的のテーブルデータバッファにかけるかチェックする｡
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
		// データ件数が０の時は領域サイズのみと比較。
		if ((pBuffInfo->ul_AreaSize - 1) >= wDataLen) {
			// リードポインタとライトポインタの重なりあいを回避するため、１だけずらす。
			return TRUE;
		}
	} else {
		// データ件数が０以上の時は算出した空き容量と比較する｡
		if ((pBuffInfo->ul_AreaSize - RAUdata_GetUseBuff(pBuffInfo) - 1) >= wDataLen) {
			// リードポインタとライトポインタの重なりあいを回避するため、１だけずらす。
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

	// 各バッファのニアフルステータスを確認し前回値と異なっていたら、TUREを返す｡
	for (i = 0; i < RAU_SEND_THROUGH_DATA_TABLE; i++) {
		if (rau_data_table_info[i].i_NearFullStatus != rau_data_table_info[i].i_NearFullStatusBefore) {
			ret = TRUE;
			// 現在値を前回値に書き込み、多重発生をなくす｡
			rau_data_table_info[i].i_NearFullStatusBefore = rau_data_table_info[i].i_NearFullStatus;
		}	// <-#001
	}

	return ret;
}


/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	テーブルデータバッファの使用量を取得する
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
		// データ件数が０以上の時は算出した空き容量と比較する｡
		if (pBuffInfo->pc_WritePoint > pBuffInfo->pc_ReadPoint) {

			return (ulong)(pBuffInfo->pc_WritePoint - pBuffInfo->pc_ReadPoint);

		} else if (pBuffInfo->pc_WritePoint < pBuffInfo->pc_ReadPoint) {

			return (ulong)((pBuffInfo->pc_WritePoint - pBuffInfo->pc_AreaTop) + ((pBuffInfo->pc_AreaTop + pBuffInfo->ul_AreaSize) - pBuffInfo->pc_ReadPoint));
		} else {
			// pBuffInfo->pc_WritePoint == pBuffInfo->pc_ReadPoint
			// の場合1byteも余さず使用。
			return pBuffInfo->ul_AreaSize;
		}
	}
}

/*[]----------------------------------------------------------------------[]*
 *|             ()
 *[]----------------------------------------------------------------------[]*
 *|	ニアフルデータ生成
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
	// T合計、GT合計のどちらかがニアフルならニアフルで通知する
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
 *|	0件データ生成
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
		// 駐車場センター形式
		switch(ucDataId){
			case 20:								// 入庫
				pNtDataHeader->id4 = 54;
				break;
			case 21:								// 出庫
				pNtDataHeader->id4 = 55;
				break;
			case 22:								// 精算(事前)
				pNtDataHeader->id4 = 56;
				break;
			case 23:								// 精算(出口)
				pNtDataHeader->id4 = 57;
				break;
			case 120:								// エラーデータ
				pNtDataHeader->id4 = 63;
				break;
			case 121:								// アラームデータ
				pNtDataHeader->id4 = 64;
				break;
			case 236:								// 駐車台数
				pNtDataHeader->id4 = 58;
				break;
			case 239:								// 区画台数
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
	
	pDataCount->data_len[1] = 48;				// リモートＮＴブロックのサイズ(ヘッダ１０バイト＋データ部[最大９６０])
	pDataCount->data_len_zero_cut[1] = 38;		// データ部[最大９６０]に対して後ろが０カットされる前のデータサイズ
	pDataCount->id1 = 0x01;
	pDataCount->id2 = 0x01;
	pDataCount->id3 = 30;
	pDataCount->id4 = 83;
	pDataCount->buffering_flag = 0x01;			// ここまでが共通部(これ以降がデータ部)
// MH810100(S) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
//	// 入庫データ			（ID20）
//	RAU_Word2Byte(pDataCount->in_parking, Ope_Log_UnreadCountGet(eLOG_ENTER, eLOG_TARGET_REMOTE));
// MH810100(E) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)

// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	// 出庫データ			（ID21）
//	RAU_Word2Byte(pDataCount->out_parking, 0);
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

	// 精算データ			（ID22）	ここでは22　23　を合算する｡
	RAU_Word2Byte(pDataCount->paid_data, Ope_Log_UnreadCountGet(eLOG_PAYMENT, eLOG_TARGET_REMOTE));

	// エラーデータ			（ID120）
	RAU_Word2Byte(pDataCount->error, Ope_Log_UnreadCountGet(eLOG_ERROR, eLOG_TARGET_REMOTE));

	// アラームデータ		（ID121）
	RAU_Word2Byte(pDataCount->alarm, Ope_Log_UnreadCountGet(eLOG_ALARM, eLOG_TARGET_REMOTE));

	// モニターデータ		（ID122）
	RAU_Word2Byte(pDataCount->monitor, Ope_Log_UnreadCountGet(eLOG_MONITOR, eLOG_TARGET_REMOTE));

	// 操作モニターデータ	（ID123）
	RAU_Word2Byte(pDataCount->ope_monitor, Ope_Log_UnreadCountGet(eLOG_OPERATE, eLOG_TARGET_REMOTE));

	// コイン金庫集計データ	（ID131）
	RAU_Word2Byte(pDataCount->coin_total, Ope_Log_UnreadCountGet(eLOG_COINBOX, eLOG_TARGET_REMOTE));

	// 紙幣金庫集計データ	（ID133）
	RAU_Word2Byte(pDataCount->money_toral, Ope_Log_UnreadCountGet(eLOG_NOTEBOX, eLOG_TARGET_REMOTE));

// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	// 駐車台数・満車データ	（ID236）
//	RAU_Word2Byte(pDataCount->parking_num, Ope_Log_UnreadCountGet(eLOG_PARKING, eLOG_TARGET_REMOTE));
//
//	// 区画台数・満車データ	（ID237）
//	RAU_Word2Byte(pDataCount->area_num, 0);
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)

	// T合計集計データ		（ID30～38・41）
	RAU_Word2Byte(pDataCount->total, Ope_Log_UnreadCountGet(eLOG_TTOTAL, eLOG_TARGET_REMOTE));

	// GT合計集計データ		（ID30～38・41）
	RAU_Word2Byte(pDataCount->gtotal, Ope_Log_UnreadCountGet(eLOG_GTTOTAL, eLOG_TARGET_REMOTE));

	// 金銭管理データ		（ID126）(176 + 7 + 4) * 32
	RAU_Word2Byte(pDataCount->money_manage, Ope_Log_UnreadCountGet(eLOG_MONEYMANAGE, eLOG_TARGET_REMOTE));

	// 遠隔監視データ
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

	memset( &RAUque_SendDataInfo[ucTableIndex], 0, sizeof(RAUque_SendDataInfo[ucTableIndex]) );	// 送信用のデータもクリア

	if (RAU_TOTAL_TABLE == ucTableIndex) {
		// T合計集計データ		（ID30～38・41）
		reset_tsum((char*)RAU_table_data.total, RAU_TOTAL_SIZE);
	}
	else if (RAU_CENTER_TERM_INFO_TABLE == ucTableIndex) {
		// センター用端末情報データ		（ID65）
		reset_center_term_info((char*)RAU_table_data.center_term_info);
	}

	rau_table_crear_flag = -1;
}

/*[]----------------------------------------------------------------------[]*
 *| 	RAU_raudata_ReadTable
 *[]----------------------------------------------------------------------[]*
 *|	テーブルバッファからデータ読み出し(バッファ終端での折り返しに対応)
 *[]----------------------------------------------------------------------[]*
 *|	param	: table	=受信データテーブル
 *|			  rptr	=読み出し開始位置
 *|			  data	=取得データ書込み先領域						<OUT>
 *|			  size	=読み出すサイズ
 *[]----------------------------------------------------------------------[]*
 *|	return	: データ読み出し後のrptrの位置
 *[]----------------------------------------------------------------------[]*
 *| Author	:	machida kei
 *| Date	:	2005-08-23
 *| Update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
uchar	*RAU_raudata_ReadTable(RAU_DATA_TABLE_INFO *table, const uchar *rptr, uchar *data, size_t size)
{
	size_t work;
	
	if (rptr + size >= &table->pc_AreaTop[table->ul_AreaSize]) {
		// 折り返しまでのデータ読み込み
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

	// 送信できたデータの数と41までの数を比較し、４１ごとにけす。
	while (ucSendCount > 0) {
		tsum_count = seek_tsum_data();
		if (tsum_count == 0) {
			break;
		}
		if (tsum_count <= ucSendCount) {
			for (i = 0; i < tsum_count; i++) {
				// 送信完了した分だけ消す｡
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
 *|	テーブルデータのニアフル通知を行う
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAUdata_CheckNearFullLogData(void)
{
	ushort	index;
	uchar	state;
	
	for(index = 0; index < RAU_ID_LOGIDLIST_COUNT; ++index) {
		if(RAU_id_LogIDList[index] == -1) {				// 対象外のログデータ
			continue;
		}
		if(!RAUdata_CanSendTableData(RAU_id_LogIDList[index])) {
			// 送信マスク設定がされるためニアフルを通知しない
			rau_data_table_info[index].i_NearFullStatusBefore = RAU_NEARFULL_CANCEL;
			rau_data_table_info[index].i_NearFullStatus = RAU_NEARFULL_CANCEL;
			continue;
		}
		// 現在のニアフル状態を取得
		state = Ope_Log_GetNearFull(RAU_id_LogIDList[index], eLOG_TARGET_REMOTE);
		
		// 前回と変更があれば状態を待避して、現在の状態を格納する
		if(rau_data_table_info[index].i_NearFullStatus != state) {
			rau_data_table_info[index].i_NearFullStatusBefore = rau_data_table_info[index].i_NearFullStatus;
			rau_data_table_info[index].i_NearFullStatus = state;
		}
	}
	
	if(TRUE == RAUdata_CheckNearFull()) {						// ニアフル状態に変化があるか
		// ニアフル通知を行う
		RAUdata_CreateNearFullData((RAU_NEAR_FULL*)&RAU_temp_data[2]);	// 状態変化していたら通知
		RAU_Word2Byte(RAU_temp_data, 32);
		RAU_CrcCcitt(30, 0, &RAU_temp_data[2], (uchar*)NULL, &RAU_temp_data[32], 1 );
		RAUdata_SetThroughData(RAU_temp_data, 34, 1);
	}
}
/*[]----------------------------------------------------------------------[]*
 *|             RAUdata_CanSendTableData()
 *[]----------------------------------------------------------------------[]*
 *|	テーブルデータ送信可能（マスクされていない）かチェックする
 *[]----------------------------------------------------------------------[]*
 *|	param	elogID	ログID(eLOG_*)
 *[]----------------------------------------------------------------------[]*
 *|	return	TRUE	送信可能
 *|			FALSE	送信不可（送信マスク設定）
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
BOOL	RAUdata_CanSendTableData(short elogID)
{
	uchar	sendMask;								// 送信マスク設定

	BOOL	ret;
	
	switch(elogID) {
// MH810100(S) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
//	case eLOG_ENTER:								// 入庫
//		sendMask = RauConfig.id20_21_mask_flg;
//		break;
// MH810100(E) K.Onodera  2019/11/15 車番チケットレス(RT精算データ対応)
	case eLOG_PAYMENT:								// 精算
		sendMask = RauConfig.id22_23_mask_flg;
		break;
	case eLOG_ERROR:								// エラー
		sendMask = RauConfig.id120_mask_flg;
		break;
	case eLOG_ALARM:								// アラーム
		sendMask = RauConfig.id121_mask_flg;
		break;
	case eLOG_MONITOR:								// モニタ
		sendMask = RauConfig.id122_mask_flg;
		break;
	case eLOG_OPERATE:								// 操作
		sendMask = RauConfig.id123_mask_flg;
		break;
	case eLOG_COINBOX:								// コイン金庫集計
		sendMask = RauConfig.id131_mask_flg;
		break;
	case eLOG_NOTEBOX:								// 紙幣金庫集計
		sendMask = RauConfig.id133_mask_flg;
		break;
// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//	case eLOG_PARKING:								// 駐車台数データ
//		sendMask = RauConfig.id236_mask_flg;
//		break;
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	case eLOG_TTOTAL:								// Ｔ集計
// MH322914 (s) kasiyama 2016/07/07 同じT合計データが再送され続け、以後のT合計が送信されない(共通改善No.1250)
//	case eLOG_GTTOTAL:								// GT集計
// MH322914 (e) kasiyama 2016/07/07 同じT合計データが再送され続け、以後のT合計が送信されない(共通改善No.1250)
		sendMask = RauConfig.id30_41_mask_flg;
		if(sendMask == 0 && RauConfig.serverTypeSendTsum == 0) {	// 集計送信しない(34-0026④=0)
			sendMask = 1;
		}
		break;
// MH322914 (s) kasiyama 2016/07/07 同じT合計データが再送され続け、以後のT合計が送信されない(共通改善No.1250)
	case eLOG_GTTOTAL:								// GT集計
		if(prm_get(COM_PRM,S_NTN,121,1,1) == 0) {
			// ParkRingの場合はGT合計を送信しない
			sendMask = 1;
		}
		else {
			sendMask = RauConfig.id30_41_mask_flg;
			if(sendMask == 0 && RauConfig.serverTypeSendTsum == 0) {	// 集計送信しない(34-0026④=0)
				sendMask = 1;
			}
		}
		break;
// MH322914 (e) kasiyama 2016/07/07 同じT合計データが再送され続け、以後のT合計が送信されない(共通改善No.1250)
	case eLOG_MONEYMANAGE:							// 金銭管理
		sendMask = RauConfig.id126_mask_flg;
		break;
	case eLOG_MNYMNG_SRAM:							// 釣銭管理
		sendMask = RauConfig.id135_mask_flg;
		break;
	case eLOG_REMOTE_MONITOR:						// 遠隔監視
		sendMask = RauConfig.id125_mask_flg;
		break;
// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
//	case eLOG_LONGPARK_PWEB:						// 長期駐車(ParkingWeb用)
//		sendMask = RauConfig.id61_mask_flg;
//		break;
//// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
	default:
		sendMask = 1;								// 送信しない
		break;
	}
	
	ret = (sendMask == 0) ? TRUE : FALSE;
	return ret;
}

/*[]----------------------------------------------------------------------[]*
 *|             RAUid_SetTableData()
 *[]----------------------------------------------------------------------[]*
 *|	テーブルデータをテーブルバッファに登録する
 *[]----------------------------------------------------------------------[]*
 *|	param	*pData		: 登録するデータ
 *|	param	wDataLen	: 登録するデータ長
 *|	param	ucDataId	: 登録するデータのＩＤ
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

	// センター用端末情報データ
	if (ucDataId == 0x41) {
		// センター用端末情報データは他のデータと違う管理方法
		if (write_center_term_info((char*)&pData[2], wDataLen - 2) == 1) {
			ret = RAU_DATA_NORMAL;
		} else {
			ret = RAU_DATA_BUFFERFULL;
		}
		// 受信シーケンス０(通常状態)に戻す処理、受信シーケンスキャンセルタイマの停止処理は、ID155:センター用データ要求応答が
		// 必ず返信されるため、その受信にて実行されているので、ここではすでに処理済みである。
	} else if (RAU_isTableData(ucDataId) == FALSE) {
		// トランザクションデータ以外のデータ（上り回線）、スルーデータ（下り回線）
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
		// CRC算出
		RAU_CrcCcitt(ret - 2
					, 0
					, pTsumBuff
					, (uchar*)NULL
					, crc_cal
					, 1);

		// CRCチェックを行う｡
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
