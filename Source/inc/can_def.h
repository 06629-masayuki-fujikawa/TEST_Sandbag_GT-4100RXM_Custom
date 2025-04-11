#ifndef _CAN_DEF_
#define _CAN_DEF_
//[]----------------------------------------------------------------------[]
///	@file		can_def.h
///	@brief		CAN通信タスク用ヘッダ
///	@date		2012/01/24
///	@author		m.nagashima
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
#include	"can_api.h"

enum {				// ノードID(1h-7Fh)
	NID_RXM	= 1,	// メイン基板
	NID_RXF,		// フロント基板

	NID_CNT			// 接続ノード数
};

#define _MAX_LENGTH_TRANSFERT	(0x7F * 7)	// ブロック転送最大データ長

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

enum {	// CAN通信状態
	CAN_STS_INIT,						// 初期化状態
	CAN_STS_OPE,						// オペレーショナル状態
	CAN_STS_STOP						// 停止状態
};

/* Massage Box settings */
#define CANBOX_TX		0
//NOTE:FIFOモード時の受信はメールボックス28を使用する
#define CANBOX_RX		28
#define CANBOX_RX_28		28// FIFO設定用
#define CANBOX_RX_29		29// FIFO設定用
#define CANBOX_RX_30		30// FIFO設定用
#define CANBOX_RX_31		31// FIFO設定用

typedef union{
	unsigned char	byte[3];
	struct{
		unsigned char	num_key_0:1;	// テンキー0(on=push)
		unsigned char	num_key_1:1;	// テンキー1(on=push)
		unsigned char	num_key_2:1;	// テンキー2(on=push)
		unsigned char	num_key_3:1;	// テンキー3(on=push)
		unsigned char	num_key_4:1;	// テンキー4(on=push)
		unsigned char	num_key_5:1;	// テンキー5(on=push)
		unsigned char	num_key_6:1;	// テンキー6(on=push)
		unsigned char	num_key_7:1;	// テンキー7(on=push)
		unsigned char	num_key_8:1;	// テンキー8(on=push)
		unsigned char	num_key_9:1;	// テンキー9(on=push)
		unsigned char	num_key_f1:1;	// テンキーF1(on=push)
		unsigned char	num_key_f2:1;	// テンキーF2(on=push)
		unsigned char	num_key_f3:1;	// テンキーF3(on=push)
		unsigned char	num_key_f4:1;	// テンキーF4(on=push)
		unsigned char	num_key_f5:1;	// テンキーF5(on=push)
		unsigned char	num_key_c:1;	// テンキーC(on=push)
		unsigned char	cndrsw_key:1;	// コインメック鍵(on=open)
		unsigned char	notesf_key:1;	// 紙幣リーダー鍵(on=open)
		unsigned char	coinsf_key:1;	// コイン金庫脱着(on=drop)
		unsigned char	ntfall_key:1;	// 紙幣リーダー脱落(on=drop)
		unsigned char	vibsns_key:1;	// 振動センサ(on=detect)
		unsigned char	mandet_key:1;	// 人体検知センサ(on=detect)
		unsigned char	reserve1:1;		// 照光式押しボタン1(予備)
		unsigned char	reserve2:1;		// 照光式押しボタン2(予備)
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

extern	t_can_transfer	transfer[SIDMAX];	// 送受信管理情報
extern	unsigned char	g_ucCAN1RxFlag;		// CAN1受信完了フラグ
extern	unsigned char	g_ucCAN1SendFlag;	// CAN1送信中フラグ
extern	unsigned char	g_ucCAN1ErrInfo;	// CAN1エラー情報
extern	unsigned char	g_ucCAN1LostErr;
extern	unsigned char	g_ucCAN1RcvSensFlag;	// CAN1センサー情報受信完了フラグ

#endif
