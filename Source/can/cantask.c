/*[]----------------------------------------------------------------------[]*/
/*| can task control                                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : m.nagashima                                             |*/
/*| Date         : 2012-01-24                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/

/*--- Include -------------------*/
#include	<string.h>
#include	<machine.h>
#include	"system.h"
#include	"mem_def.h"
#include	"message.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"can_api.h"
#include	"can_def.h"
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
#include	"mnt_def.h"
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
/*--- Prototype -----------------*/
static void can_ic_init(void);
static void canchk(MsgBuf *pmsg);
static void canrcv(void);
static void cansnd(void);
static void get_can_key_event(can_std_frame_t *dataframe);

/*--- Define --------------------*/
#define _GET_CAN()		&can_ctr

#define CAN_TERM_PODR(x, y)			CAN_TERM_PODR_PREPROC(x, y)
#define CAN_TERM_PODR_PREPROC(x, y)	(PORT ## x .PODR.BIT.B ## y)

#define	CAN_RES_TIME						(100/10)			// CAN_RES信号High継続時間:100ms
#define	CAN_RESP_TIME						(1000/10)			// 応答待ち時間:1s
#define	CAN_HEARTBEAT_TIME					10					// ハートビート周期:10s
#define	CAN_START_KEEP_ALIVE_TIMER(nid, t)	Lagtim(CANTCBNO, (unsigned char)(nid-NID_RXM), (unsigned short)((t)/20))	// キープアライブタイマ起動('t'ms)
#define	CAN_STOP_KEEP_ALIVE_TIMER(nid)		Lagcan(CANTCBNO, (unsigned char)(nid-NID_RXM))								// キープアライブタイマ停止
#define	CAN_TIMEOUT_KEEP_ALIVE_02			(TIMEOUT+NID_RXF-NID_RXM)	// キープアライブタイマタイムアウトイベント(NID_RXF)
#define ECSR_DELAYTIM	5										// ECSRのディレイタイムアウト値100ms
#define CAN_TIMER_ECSR_DELAY	2										// ECSR検出ディレイタイマのID
#define	CAN_TIMEOUT_ECSR_DELAY			(TIMEOUT+CAN_TIMER_ECSR_DELAY)	// ECSR検出ディレイタイムアウト

typedef union{								// data[0]分析用構造体
	unsigned char	byte;
	struct{
		unsigned char	command		:3;		// コマンド
		unsigned char	t			:1;		// トグルビット(segment送信の度にビットを反転)
		unsigned char	undefined	:2;		// 未定義
		unsigned char	e			:1;		// 0:シングル転送, 1:ブロック転送
		unsigned char	c			:1;		// 0:次segmentあり, 1:最終segment
	} bit;
} t_data0;

typedef struct {
	unsigned char	data[_MAX_LENGTH_TRANSFERT];	// ディスプレイ送信バッファ
	unsigned short	count;
	unsigned short	countmax;
	unsigned short	readidx;
	unsigned short	writeidx;
} t_can_dispbuf;

/*--- Gloval Value --------------*/
t_can_transfer	transfer[SIDMAX];					// 送受信管理情報

static  struct _canctr {							// ワーク領域
	t_can_quectrl	sndque[3];						// 送信キュー
	t_can_quectrl	rcvque;							// 受信キュー
	can_std_frame_t	tx_dataframe[32];				// 送信データフレーム(共通)
	can_std_frame_t	ctrl_tx_dataframe[16];			// 送信データフレーム(CONTROL)
	can_std_frame_t	disp_tx_dataframe[0x7F+2];		// 送信データフレーム(DISPLAY)
	can_std_frame_t	rx_dataframe[128];				// 受信データフレーム
	t_can_dispbuf	dispbuf;						// ディスプレイ送信バッファ
	unsigned char	can_net_stat[NID_CNT];			// ネットワーク状態(CAN_STS_INIT:初期化状態, CAN_STS_OPE:オペレーショナル状態, CAN_STS_STOP:停止状態)
} can_ctr;

static const struct {
	unsigned short	sid;
	t_can_quectrl	*sndque;
	can_std_frame_t	*dataframe;
	unsigned char	countmax;
} t_can_transfer_Initializer[SIDMAX] = {
	{0x00,	&can_ctr.sndque[0],	&can_ctr.tx_dataframe[0],		32},	// CAN_NETWORK
	{0x80,	NULL,				NULL,							0},		// CAN_EMERGENCY
	{0x100,	NULL,				NULL,							0},		// CAN_EVENT
	{0x180,	&can_ctr.sndque[1],	&can_ctr.ctrl_tx_dataframe[0],	16},	// CAN_CONTROL
	{0x200,	&can_ctr.sndque[2],	&can_ctr.disp_tx_dataframe[0],	0x7F+2},// CAN_DISPLAY
	{0x700,	&can_ctr.sndque[0],	&can_ctr.tx_dataframe[0],		32},	// CAN_HEARTBEAT
};

const unsigned short scan_event_table[KeyDatMax][8] = {
	{FNT_DIPSW1,FNT_DIPSW2,	FNT_DIPSW3,	FNT_DIPSW4,	FNT_YOBI1,	FNT_YOBI2,	FNT_YOBI3,	FNT_YOBI4},
	{KEY_TEN0,	KEY_TEN1,	KEY_TEN2,	KEY_TEN3,	KEY_TEN4,	KEY_TEN5,	KEY_TEN6,	KEY_TEN7},
	{KEY_TEN8,	KEY_TEN9,	KEY_TEN_F1,	KEY_TEN_F2,	KEY_TEN_F3,	KEY_TEN_F4,	KEY_TEN_F5,	KEY_TEN_CL},
	{KEY_CNDRSW,KEY_NOTESF,	KEY_COINSF,	KEY_NTFALL,	KEY_VIBSNS,	KEY_MANDET,	KEY_RESSW1,	KEY_RESSW2},
};

unsigned char	g_ucCAN1Dispatch;		// CAN1ディスパッチ要求
unsigned char	g_ucCAN1RxFlag;			// CAN1受信完了フラグ
unsigned char	g_ucCAN1SendFlag;		// CAN1送信中フラグ
unsigned char	g_ucCAN1ErrInfo;		// CANエラー情報
unsigned char	g_ucCAN1LostErr;		// CANメッセージロスト情報
unsigned char	g_ucCAN1StaffErr;		// CANスタッフエラー情報
unsigned char	g_ucCAN1FormErr;		// CANフォームエラー情報
unsigned char	g_ECSR_Delay_f;			// ECSRディレイフラグクリア
unsigned char	g_ucCAN1RcvSensFlag;	// CAN1センサー情報受信完了フラグ

/*--- Extern --------------------*/


//[]----------------------------------------------------------------------[]
///	@brief			CAN通信タスクメイン
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			m.nagashima
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void cantask( void )
{
	MsgBuf	*pmsg;
	MsgBuf	msg;

	LagTim500ms(LAG500_CAN_HEARTBEAT_TIMER, 4, can_snd_data14);	// ハートビート周期(初回のみ2s)

	for( ; ; ){
		taskchg(IDLETSKNO);

		pmsg = GetMsg(CANTCBNO);
		if (pmsg != NULL) {
			memcpy(&msg, pmsg, sizeof(msg));
			FreeBuf(pmsg);
			pmsg = &msg;
		}

		canchk(pmsg);	// error check
		canrcv();		// receive
		cansnd();		// send
	}
}


//[]----------------------------------------------------------------------[]
///	@brief			接続基板のハードウェアリセット
//[]----------------------------------------------------------------------[]
///	@return			void
///	@author			m.onouchi
///	@note			CANで接続されている他のノードをリセットする。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/03<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void can_ic_init(void)
{
	unsigned long	StartTime;

	CP_CAN_RES = 1;		// CAN Reset 実行
	StartTime = LifeTimGet();

	do {
		g_ucCAN1Dispatch = 1;
		taskchg(IDLETSKNO);
	} while(LifePastTimGet(StartTime) < CAN_RES_TIME);

	CP_CAN_RES = 0;		// CAN Reset 解除
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN初期化
//[]----------------------------------------------------------------------[]
///	@return			R_CAN_OK				: 正常終了<br>
///					R_CAN_SW_BAD_MBX		: 存在しないメールボックス番号<br>
///					R_CAN_BAD_CH_NR			: 存在しないチャネル番号<br>
///					R_CAN_BAD_ACTION_TYPE:	: 未定義の設定指示<br>
///					R_CAN_SW_RST_ERR		: CANペリフェラルがリセットモードにならない<br>
///					R_CAN_SW_WAKEUP_ERR		: CANペリフェラルがウェイクアップしない<br>
///					R_CAN_SW_SLEEP_ERR		: CANペリフェラルがスリープモードになった<br>
///					R_CAN_SW_HALT_ERR		: CANペリフェラルがホルトモードにならない
///	@author			m.onouchi
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/20<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned int caninit(void)
{
	unsigned int	api_status;
	struct _canctr	*ctr = _GET_CAN();
	char			i;

	memset(ctr, 0, sizeof(*ctr));						// ワーク領域クリア

	ctr->can_net_stat[NID_RXM] = CAN_STS_INIT;			// ネットワーク状態(CAN_STS_INIT:初期化状態)

	/* CAN通信設定 */
	if (ExIOSignalrd(INSIG_DPSW3)) {					// DPSW3:CAN終端設定
		CAN_TERM_PODR(5, 6) = 0;						// P56:CAN終端抵抗OFF
	} else {
		CAN_TERM_PODR(5, 6) = 1;						// P56:CAN終端抵抗ON
	}

	MSTP_CAN1 = 0;										// CANのモジュールストップ状態を解除

	api_status = R_CAN_Create(1);						// CANペリフェラル初期化

	R_CAN_PortSet(1, ENABLE);							// CANトランシーバポートの端子設定

	memset(&ctr->dispbuf, 0, sizeof(ctr->dispbuf));
	for (i = 0; i < SIDMAX; i++) {						// 送信管理情報初期化
		transfer[i].sid = t_can_transfer_Initializer[i].sid;
		transfer[i].sndque = t_can_transfer_Initializer[i].sndque;
		transfer[i].sndque->dataframe = t_can_transfer_Initializer[i].dataframe;
		transfer[i].sndque->countmax = t_can_transfer_Initializer[i].countmax;
	}
	ctr->rcvque.dataframe = &ctr->rx_dataframe[0];
	ctr->rcvque.countmax = 128;

	g_ucCAN1Dispatch = 1;								// CAN1ディスパッチ要求
	g_ucCAN1RxFlag = 0;									// CAN1受信完了フラグ
	g_ucCAN1SendFlag = 0;								// CAN1送信中フラグ
	g_ucCAN1ErrInfo = 0;								// CANエラー情報
	g_ucCAN1LostErr = 0;								// CANメッセージロスト情報
	g_ucCAN1StaffErr = 0;								// CANスタッフエラー情報
	g_ucCAN1FormErr = 0;								// CANフォームエラー情報
	g_ECSR_Delay_f = 0;									// ECSRディレイフラグクリア
	g_ucCAN1RcvSensFlag = 0;							// CAN1センサー情報受信完了フラグ

	R_CAN_RxSet(1, CANBOX_RX, 0x002, DATA_FRAME);		// メッセージ受信指示
	R_CAN_RxSetMask(1, CANBOX_RX, 0x07F);				// メッセージの受け入れマスク設定

	return (api_status);
}


//[]----------------------------------------------------------------------[]
///	@brief			CANタスクイベント確認
//[]----------------------------------------------------------------------[]
///	@return			0:イベントなし, 1:イベントあり
///	@author			m.nagashima
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
short canevent( void )
{
	struct _canctr	*ctr = _GET_CAN();

	// 強制ディスパッチ要求チェック
	if (g_ucCAN1Dispatch) {
		g_ucCAN1Dispatch = 0;
		return 1;
	}

	// 受信データチェック
	if (g_ucCAN1RxFlag) {	// 受信データあり
		return 1;
	}

	// 送信要求チェック
	if (transfer[CAN_NETWORK].sndque->count) {
		return 1;
	}
	if (transfer[CAN_CONTROL].state == SDO_RESET) {
		if (0 != transfer[CAN_CONTROL].sndque->count) {
			return 1;
		}
	}
	if (transfer[CAN_DISPLAY].state == SDO_RESET) {
		if (0 != ctr->dispbuf.count) {
			return 1;
		}
	}

	return 0;
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN 送信データフレーム格納
//[]----------------------------------------------------------------------[]
///	@param[in]		*sndque	: 送信キュー
///	@param[in]		*frame_p: データフレーム
///	@return			ret		: void
///	@author			m.onouchi
///	@note			送信したいデータフレームを送信用リングバッファに格納する。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2011/12/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
void can_snd_queset(t_can_quectrl *sndque, can_std_frame_t *frame_p)
{
	volatile unsigned long	ist;			// 現在の割込受付状態

	ist = _di2();
	memcpy(&sndque->dataframe[sndque->writeidx], frame_p, sizeof(can_std_frame_t));

	++sndque->writeidx;
	if (sndque->writeidx >= sndque->countmax) {
		sndque->writeidx = 0;
	}

	if (sndque->count < sndque->countmax) {
		++sndque->count;
	} else {
		++sndque->readidx;
		if (sndque->readidx >= sndque->countmax) {
			sndque->readidx = 0;
		}
	}
	_ei2(ist);
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN 受信データフレーム格納
//[]----------------------------------------------------------------------[]
///	@param[in]		*frame_p: データフレーム
///	@return			ret		: void
///	@author			m.onouchi
///	@note			受信したデータフレームを受信用リングバッファに格納する。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2011/12/09<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2011 AMANO Corp.---[]
void can_rcv_queset(can_std_frame_t *frame_p)
{
	struct _canctr	*ctr = _GET_CAN();

	memcpy(&ctr->rcvque.dataframe[ctr->rcvque.writeidx], frame_p, sizeof(can_std_frame_t));

	++ctr->rcvque.writeidx;
	if (ctr->rcvque.writeidx >= ctr->rcvque.countmax) {
		ctr->rcvque.writeidx = 0;
	}

	if (ctr->rcvque.count < ctr->rcvque.countmax) {
		++ctr->rcvque.count;
	} else {
		++ctr->rcvque.readidx;
		if (ctr->rcvque.readidx >= ctr->rcvque.countmax) {
			ctr->rcvque.readidx = 0;
		}
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			CANエラー監視処理
//[]----------------------------------------------------------------------[]
///	@param[in]		*pmsg	: メッセージ
///	@return			ret		: void
///	@author			m.onouchi
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/16<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void canchk(MsgBuf *pmsg)
{
	unsigned char	err;
	unsigned char	lost;
	unsigned long	ist;		// 現在の割込受付状態

	volatile struct st_can __evenaccess * can_block_p;

	can_block_p = (struct st_can __evenaccess *) 0x91200;	// CAN1

	ist = _di2();
	err = g_ucCAN1ErrInfo;
	g_ucCAN1ErrInfo = 0;
	lost = g_ucCAN1LostErr;
	g_ucCAN1LostErr = 0;
	_ei2(ist);

	switch (pmsg->msg.command) {
	case CAN_TIMEOUT_KEEP_ALIVE_02:			// RXF
		err_chk(ERRMDL_CAN, ERR_CAN_NOANSWER_02, 1, 0, 0);	// E8612:CAN ノード2応答なし(発生)
		can_ic_init();	// 接続基板のハードウェアリセット
		break;
	case CAN_TIMEOUT_ECSR_DELAY:			// ECSRディレイタイムアウト
		g_ECSR_Delay_f = 0;// ECSRディレイフラグクリア
		break;
	default:
		break;
	}

	if (err & 0x08) {						// BOEIF:バスオフ開始検出
		err_chk(ERRMDL_CAN, ERR_CAN_COMFAIL, 1, 0, 0);		// E8601:CAN 通信不良(発生)
// MH810100(S) K.Onodera 2020/01/08 車番チケットレス(警告通知)
		lcdbm_notice_alm( lcdbm_alarm_check() );
// MH810100(E) K.Onodera 2020/01/08 車番チケットレス(警告通知)
	}
	if (err & 0x10) {						// BORIF:バスオフ復帰検出
		err_chk(ERRMDL_CAN, ERR_CAN_COMFAIL, 0, 0, 0);		// E8601:CAN 通信不良(解除)
// MH810100(S) K.Onodera 2020/01/08 車番チケットレス(警告通知)
		lcdbm_notice_alm( lcdbm_alarm_check() );
// MH810100(E) K.Onodera 2020/01/08 車番チケットレス(警告通知)
	}
	if(g_ECSR_Delay_f == 0) {
		if (can_block_p->ECSR.BIT.SEF) {	// スタッフエラー検出
			if(g_ucCAN1StaffErr == 0) {	// スタッフエラー未検出
				err_chk(ERRMDL_CAN, ERR_CAN_STUFF_ERR, 1, 0, 0);	// E8605:CAN スタッフエラー(発生)
				g_ucCAN1StaffErr = 1;// CANスタッフエラー情報セット
// MH810100(S) K.Onodera 2020/01/26 車番チケットレス (#3884 警告通知_E8601の未表示不具合修正)
				lcdbm_notice_alm( lcdbm_alarm_check() );
// MH810100(E) K.Onodera 2020/01/26 車番チケットレス (#3884 警告通知_E8601の未表示不具合修正)
			}
		} else {
			if(g_ucCAN1StaffErr != 0) {	// スタッフエラー検出中
				err_chk(ERRMDL_CAN, ERR_CAN_STUFF_ERR, 0, 0, 0);	// E8605:CAN スタッフエラー(解除)
// MH810100(S) Y.Watanabe 2020/01/26 車番チケットレス (#3884 警告通知_E8601の未表示不具合修正)
				lcdbm_notice_alm( lcdbm_alarm_check() );
// MH810100(E) Y.Watanabe 2020/01/26 車番チケットレス (#3884 警告通知_E8601の未表示不具合修正)
			}
			g_ucCAN1StaffErr = 0;// CANスタッフエラー情報クリア
		}
		if (can_block_p->ECSR.BIT.FEF) {	// フォームエラー検出
			if(g_ucCAN1FormErr == 0) {	// フォームエラー未検出
				err_chk(ERRMDL_CAN, ERR_CAN_FORM_ERR, 1, 0, 0);		// E8606:CAN フォームエラー(発生)
				g_ucCAN1FormErr = 1;// CANフォームエラー情報セット
// MH810100(S) Y.Watanabe 2020/01/26 車番チケットレス (#3884 警告通知_E8601の未表示不具合修正)
				lcdbm_notice_alm( lcdbm_alarm_check() );
// MH810100(E) Y.Watanabe 2020/01/26 車番チケットレス (#3884 警告通知_E8601の未表示不具合修正)
			}
		} else {
			if(g_ucCAN1FormErr != 0) {	// フォームエラー検出中
				err_chk(ERRMDL_CAN, ERR_CAN_FORM_ERR, 0, 0, 0);		// E8606:CAN フォームエラー(解除)
// MH810100(S) Y.Watanabe 2020/01/26 #3884 警告通知_E8601の未表示不具合修正
				lcdbm_notice_alm( lcdbm_alarm_check() );
// MH810100(E) Y.Watanabe 2020/01/26 #3884 警告通知_E8601の未表示不具合修正
			}
			g_ucCAN1FormErr = 0;// CANフォームエラー情報クリア
		}
		if (can_block_p->ECSR.BIT.CEF) {	// CRCエラー検出
			err_chk(ERRMDL_CAN, ERR_CAN_CRC_ERR, 2, 0, 0);			// E8607:CAN CRCエラー(発生解除同時)
		}
		if (can_block_p->ECSR.BYTE) {
			can_block_p->ECSR.BYTE = 0x00;							// エラーコード格納レジスタクリア
			// NOTE:本関数でECSRをクリアしてから次のECSR状態が確定するまで若干時間が必要なためにディレイを確保する必要がある
			// そうしないとRXMとRXF基板のCANケーブルが抜けた状態でスタッフエラー発生／解除が多発する
			Lagtim(CANTCBNO, CAN_TIMER_ECSR_DELAY, ECSR_DELAYTIM);// ECSRディレイタイマスタート
			g_ECSR_Delay_f = 1;// ECSRディレイフラグセット
		}
	}
	if (lost) {
		// オーバーロード（メッセージロスト）エラーの登録
		err_chk(ERRMDL_CAN, ERR_CAN_LOST_ERR, 2, 0, 0);				// E8608:CAN オーバロードエラー(発生解除同時)
	}
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN受信データ監視処理
//[]----------------------------------------------------------------------[]
///	@return			ret		: void
///	@author			m.nagashima
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void canrcv(void)
{
	volatile unsigned long	ist;			// 現在の割込受付状態
	struct _canctr	*ctr = _GET_CAN();
	t_data0			*p_dt0;
	unsigned short	length;
	unsigned char	dlc;
	int				i;
	int				j;
	int				seqno;
	union {
		unsigned short	us;
		struct {
			unsigned char	high;
			unsigned char	low;
		} uc;
	} crc16;
	can_std_frame_t	tx_dataframe;
	can_std_frame_t	rx_dataframe;

	/* 受信処理 */
	if (g_ucCAN1RxFlag) {						// 受信キューに受信メッセージあり
		memset(&rx_dataframe, 0, sizeof(can_std_frame_t));
		if (0 != ctr->rcvque.count) {
			ist = _di2();
			memcpy(&rx_dataframe, &ctr->rcvque.dataframe[ctr->rcvque.readidx], sizeof(can_std_frame_t));
			--ctr->rcvque.count;
			++ctr->rcvque.readidx;
			if (ctr->rcvque.readidx >= ctr->rcvque.countmax) {
				ctr->rcvque.readidx = 0;
			}
			if (0 == ctr->rcvque.count) {
				g_ucCAN1RxFlag = 0;
			}
			_ei2(ist);
		}

		for (i = 0; i < SIDMAX; i++) {
			if (transfer[i].sid == (rx_dataframe.id & 0x780)) {
				switch (i) {
				case CAN_EMERGENCY:				// エマージェンシー受信
					queset(OPETCBNO, CAN_EMG_EVT, rx_dataframe.dlc, rx_dataframe.data);
					break;
				case CAN_EVENT:					// イベント受信
					get_can_key_event(&rx_dataframe);
					break;
				case CAN_CONTROL:				// コントロール受信
					p_dt0 = (t_data0*)&rx_dataframe.data;
					switch (p_dt0->bit.command) {
					case 3:						// initiate → response
						if (transfer[i].state == SDO_INITIATE_DOWNLOAD_REQUEST) {
							ist = _di2();	// 	for illegal CanInt during 1st_SenddingData
							transfer[i].state = SDO_RESET;
							--transfer[CAN_CONTROL].sndque->count;
							++transfer[CAN_CONTROL].sndque->readidx;
							if (transfer[CAN_CONTROL].sndque->readidx >= transfer[CAN_CONTROL].sndque->countmax) {
								transfer[CAN_CONTROL].sndque->readidx = 0;
							}
							_ei2(ist);		// 	for illegal CanInt during 1st_SenddingData
						} else {
							// エラー処理
						}
						break;
					case 4:						// abort
						if (transfer[i].state == SDO_INITIATE_DOWNLOAD_REQUEST) {
							ist = _di2();	// 	for illegal CanInt during 1st_SenddingData
							transfer[i].state = SDO_RESET;
							_ei2(ist);		// 	for illegal CanInt during 1st_SenddingData
						}
						break;
					default:
						break;
					}
					break;
				case CAN_DISPLAY:				// ディスプレイ受信
					p_dt0 = (t_data0*)&rx_dataframe.data;
					switch (p_dt0->bit.command) {
					case 3:						// initiate → response
						if (transfer[i].state == SDO_INITIATE_DOWNLOAD_REQUEST) {
							ist = _di2();	// 	for illegal CanInt during 1st_SenddingData
							transfer[i].state = SDO_BLOCK_DOWNLOAD_IN_PROGRESS;
							_ei2(ist);		// 	for illegal CanInt during 1st_SenddingData
							/* segment */
							p_dt0 = (t_data0*)&tx_dataframe.data;
							length = transfer[i].count;
							for (seqno = 1; seqno <= transfer[i].blksize; seqno++) {
								memset(&tx_dataframe, 0, sizeof(can_std_frame_t));
								tx_dataframe.id = transfer[i].sid | NID_RXM;	// IDセット
								if (length <= 7) {
									tx_dataframe.dlc = length;
								} else {
									tx_dataframe.dlc = 7;
								}
								length -= tx_dataframe.dlc;
								p_dt0->bit.command = 0;
								p_dt0->bit.t = transfer[i].toggle = !(transfer[i].toggle & 1);
								if (seqno == transfer[i].blksize) {
									p_dt0->bit.c = 1;
								}
								tx_dataframe.dlc += 1;
								for (dlc = 1; dlc < tx_dataframe.dlc; dlc++) {
									transfer[i].crc ^= tx_dataframe.data[dlc] = ctr->dispbuf.data[transfer[i].offset];
									for (j = 0; j < CHAR_BIT; j++) {
										if (transfer[i].crc & 1) {
											transfer[i].crc = (transfer[i].crc >> 1) ^ CRCPOLY2;
										} else {
											transfer[i].crc >>= 1;
										}
									}
									transfer[i].offset++;
									if (transfer[i].offset >= _MAX_LENGTH_TRANSFERT) {
										transfer[i].offset = 0;
									}
								}
								can_snd_queset(transfer[i].sndque, &tx_dataframe);
							}
							/* end */
							memset(&tx_dataframe, 0, sizeof(can_std_frame_t));
							tx_dataframe.id = transfer[i].sid | NID_RXM;	// IDセット
							tx_dataframe.dlc = 3;
							p_dt0->bit.command = 6;

							crc16.uc.high = (unsigned char)(transfer[i].crc & 0x00ff);
							crc16.uc.low = (unsigned char)((transfer[i].crc & 0xff00) >> CHAR_BIT);
							transfer[i].crc = crc16.us;
							tx_dataframe.data[1] = crc16.uc.high;
							tx_dataframe.data[2] = crc16.uc.low;
							can_snd_queset(transfer[i].sndque, &tx_dataframe);
						}
						break;
					case 4:						// abort
						if ((transfer[i].state == SDO_INITIATE_DOWNLOAD_REQUEST)
						||  (transfer[i].state == SDO_BLOCK_DOWNLOAD_IN_PROGRESS)) {
							ist = _di2();	// 	for illegal CanInt during 1st_SenddingData
							transfer[i].state = SDO_RESET;
							transfer[i].toggle = 0;
							transfer[i].count = 0;
							transfer[i].blksize = 0;
							transfer[i].crc = 0;
							transfer[i].offset = 0;
							_ei2(ist);		// 	for illegal CanInt during 1st_SenddingData
						}
						break;
					case 5:						// end → response
						if (transfer[i].state == SDO_END_DOWNLOAD_REQUEST) {
							for (length = 0; length < transfer[i].count; length++) {
								--ctr->dispbuf.count;
								++ctr->dispbuf.readidx;
								if (ctr->dispbuf.readidx >= _MAX_LENGTH_TRANSFERT) {
									ctr->dispbuf.readidx = 0;
								}
							}
							ist = _di2();	// 	for illegal CanInt during 1st_SenddingData
							transfer[i].state = SDO_RESET;
							transfer[i].toggle = 0;
							transfer[i].count = 0;
							transfer[i].blksize = 0;
							transfer[i].crc = 0;
							transfer[i].offset = 0;
							_ei2(ist);		// 	for illegal CanInt during 1st_SenddingData
						}
						break;
					}
					break;
				case CAN_HEARTBEAT:				// ハートビート受信
					switch (rx_dataframe.id & 0x7F) {
					case NID_RXF:
						ctr->can_net_stat[rx_dataframe.id & 0x7F] = rx_dataframe.data[0];
						if (rx_dataframe.data[0] == CAN_STS_INIT) {			// ネットワーク状態(CAN_STS_INIT:初期化状態)
							ctr->can_net_stat[NID_RXM] = CAN_STS_OPE;		// ネットワーク状態(CAN_STS_OPE:オペレーショナル状態)
							can_snd_data0((unsigned char)(rx_dataframe.id & 0x7F), CAN_STS_OPE);
						}
						if (ERR_CHK[mod_can][ERR_CAN_NOANSWER_02]) {			// E8612:CAN ノード2応答なし 発生中
							err_chk(ERRMDL_CAN, ERR_CAN_NOANSWER_02, 0, 0, 0);	// E8612:CAN ノード2応答なし(解除)
						}
						if (rx_dataframe.dlc == 7) {
							memset(&RXF_VERSION[0], 0, sizeof(RXF_VERSION));
							for (dlc = 1, j = 0; dlc < 4; dlc++) {
								if (rx_dataframe.data[dlc] >= 0x41 && rx_dataframe.data[dlc] <= 0x5A) {	// Ａ～Ｚ
									RXF_VERSION[j] = rx_dataframe.data[dlc];
									j++;
								}
							}
							BCDtoASCII(&rx_dataframe.data[dlc], &RXF_VERSION[j], 3);
						}
						CAN_START_KEEP_ALIVE_TIMER(NID_RXF, (CAN_HEARTBEAT_TIME+2)*1000*3);	// キープアライブタイマリスタート
						break;
					default:
						break;
					}
					break;
				default:
					break;
				}
			}
		}
	}
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN送信データ監視処理
//[]----------------------------------------------------------------------[]
///	@return			ret		: void
///	@author			m.nagashima
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/24<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void cansnd(void)
{
	volatile unsigned long	ist;			// 現在の割込受付状態
	struct _canctr	*ctr = _GET_CAN();
	can_std_frame_t	tx_dataframe;
	t_data0			*p_dt0;

	// 応答待ち
	if (transfer[CAN_CONTROL].state == SDO_INITIATE_DOWNLOAD_REQUEST) {
		if (LifePastTimGet(transfer[CAN_CONTROL].timer) >= CAN_RESP_TIME) {
			transfer[CAN_CONTROL].state = SDO_RESET;
			transfer[CAN_CONTROL].toggle = 0;
			transfer[CAN_CONTROL].count = 0;
			transfer[CAN_CONTROL].blksize = 0;
			transfer[CAN_CONTROL].crc = 0;
			transfer[CAN_CONTROL].offset = 0;
		}
	}
	if ((transfer[CAN_DISPLAY].state == SDO_INITIATE_DOWNLOAD_REQUEST)
	||  (transfer[CAN_DISPLAY].state == SDO_END_DOWNLOAD_REQUEST)) {
		if (LifePastTimGet(transfer[CAN_DISPLAY].timer) >= CAN_RESP_TIME) {
			ist = _di2();	// 	for illegal CanInt during 1st_SenddingData
			transfer[CAN_DISPLAY].state = SDO_RESET;
			transfer[CAN_DISPLAY].toggle = 0;
			transfer[CAN_DISPLAY].count = 0;
			transfer[CAN_DISPLAY].blksize = 0;
			transfer[CAN_DISPLAY].crc = 0;
			transfer[CAN_DISPLAY].offset = 0;
			_ei2(ist);		// 	for illegal CanInt during 1st_SenddingData
		}
	}

	/* 送信処理 */
	if (g_ucCAN1SendFlag == 0) {						// 送信中でない
		if (!ERR_CHK[mod_can][ERR_CAN_COMFAIL]) {		// E8601:CAN 通信不良 発生なし
			if (0 != transfer[CAN_NETWORK].sndque->count) {
				g_ucCAN1SendFlag = 1;		// 送信中に設定
				R_CAN_TxSet(1, CANBOX_TX, &transfer[CAN_NETWORK].sndque->dataframe[transfer[CAN_NETWORK].sndque->readidx], DATA_FRAME);	// 送信メールボックスの設定
				--transfer[CAN_NETWORK].sndque->count;
				++transfer[CAN_NETWORK].sndque->readidx;
				if (transfer[CAN_NETWORK].sndque->readidx >= transfer[CAN_NETWORK].sndque->countmax) {
					transfer[CAN_NETWORK].sndque->readidx = 0;
				}
				goto _FIN;
			}
			if (transfer[CAN_CONTROL].state == SDO_RESET) {
				if (0 != transfer[CAN_CONTROL].sndque->count) {
					if (ctr->can_net_stat[NID_RXM] == CAN_STS_OPE) {	// ネットワーク状態(CAN_STS_OPE:オペレーショナル状態)
						g_ucCAN1SendFlag = 1;		// 送信中に設定
						transfer[CAN_CONTROL].state = SDO_INITIATE_DOWNLOAD_REQUEST;
						R_CAN_TxSet(1, CANBOX_TX, &transfer[CAN_CONTROL].sndque->dataframe[transfer[CAN_CONTROL].sndque->readidx], DATA_FRAME);	// 送信メールボックスの設定
						transfer[CAN_CONTROL].timer = LifeTimGet();							// 応答待ちタイマセット
						goto _FIN;
					}
				}
			}
			switch (transfer[CAN_DISPLAY].state) {
			case SDO_RESET:
				if (ctr->can_net_stat[NID_RXM] == CAN_STS_OPE) {	// ネットワーク状態(CAN_STS_OPE:オペレーショナル状態)
					if (0 != ctr->dispbuf.count) {
						ist = _di2();
						g_ucCAN1SendFlag = 1;	// 送信中に設定
						memset(&tx_dataframe, 0, sizeof(can_std_frame_t));
						transfer[CAN_DISPLAY].state = SDO_INITIATE_DOWNLOAD_REQUEST;
						transfer[CAN_DISPLAY].toggle = 1;
						transfer[CAN_DISPLAY].count = ctr->dispbuf.count;
						transfer[CAN_DISPLAY].blksize = (unsigned char)(transfer[CAN_DISPLAY].count / 7);
						if (transfer[CAN_DISPLAY].count % 7) {
							transfer[CAN_DISPLAY].blksize++;
						}
						transfer[CAN_DISPLAY].crc = 0;
						transfer[CAN_DISPLAY].offset = ctr->dispbuf.readidx;
						_ei2( ist );

						/* initiate */
						tx_dataframe.id = transfer[CAN_DISPLAY].sid | NID_RXM;				// IDセット
						tx_dataframe.dlc = 2;												// データ長
						p_dt0 = (t_data0*)&tx_dataframe.data;
						p_dt0->bit.command = 1;												// コマンドセット
						p_dt0->bit.e = 1;													// ブロック転送
						tx_dataframe.data[1] = transfer[CAN_DISPLAY].blksize;				// セグメント数

						R_CAN_TxSet(1, CANBOX_TX, &tx_dataframe, DATA_FRAME);
						transfer[CAN_DISPLAY].timer = LifeTimGet();							// 応答待ちタイマセット
					}
				}
				break;
			case SDO_BLOCK_DOWNLOAD_IN_PROGRESS:
			case SDO_ABORTED_INTERNAL:
				if (0 != transfer[CAN_DISPLAY].sndque->count) {
					if (ctr->can_net_stat[NID_RXM] == CAN_STS_OPE) {	// ネットワーク状態(CAN_STS_OPE:オペレーショナル状態)
						ist = _di2();	// 	for illegal CanInt during 1st_SenddingData
						g_ucCAN1SendFlag = 1;		// 送信中に設定
						R_CAN_TxSet(1, CANBOX_TX, &transfer[CAN_DISPLAY].sndque->dataframe[transfer[CAN_DISPLAY].sndque->readidx], DATA_FRAME);	// 送信メールボックスの設定
						--transfer[CAN_DISPLAY].sndque->count;
						++transfer[CAN_DISPLAY].sndque->readidx;
						if (transfer[CAN_DISPLAY].sndque->readidx >= transfer[CAN_DISPLAY].sndque->countmax) {
							transfer[CAN_DISPLAY].sndque->readidx = 0;
						}
						_ei2(ist);		// 	for illegal CanInt during 1st_SenddingData end
					}
				}
				break;
			default:
				break;
			}
		}
	}
_FIN:
	return;
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN ネットワーク送信(00h + NID_RXM)
//[]----------------------------------------------------------------------[]
///	@param[in]		d_node	: 送信先ノード
///	@param[in]		sts		: 0:Initialization, 1:Operational, 2:Stopped
///	@return			ret		: 0:送信OK, 1:送信NG
///	@author			m.nagashima
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2011/01/23<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void can_snd_data0(unsigned char d_node, unsigned char sts)
{
	can_std_frame_t tx_dataframe;

	memset(&tx_dataframe, 0, sizeof(can_std_frame_t));

	// 電文生成
	tx_dataframe.id = transfer[CAN_NETWORK].sid | NID_RXM;
	tx_dataframe.dlc = 2;
	tx_dataframe.data[0] = d_node;	// 送信先ノード
	tx_dataframe.data[1] = sts;		// 状態

	can_snd_queset(transfer[CAN_NETWORK].sndque, &tx_dataframe);
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN コントロール送信(180h + NID_RXM)
//[]----------------------------------------------------------------------[]
///	@param[in]		ctr_cd	: コントロールコード<br>
///							:	0x00:未定義<br>
///							:	0x10-0x1F:ブザー音(ピッ♪)吹鳴<br>
///							:	0x20-0x2F:ブザー音(ピピッ♪)吹鳴<br>
///							:	0x30-0x3F:ブザー音(ピピピッ♪)吹鳴<br>
///							:	0x40-0x4F:LCDバックライト輝度値変更
///	@param[in]		out1	: 出力１<br>
///							:	b7:釣銭取出口ガイドLED(0:消灯, 1:点灯)<br>
///							:	b6:テンキーガイドLED(0:消灯, 1:点灯)<br>
///							:	b5:照光式押しボタン１(0:消灯, 1:点灯)<br>
///							:	b4:照光式押しボタン１(0:消灯, 1:点灯)
///	@return			ret		: void
///	@author			m.nagashima
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/26<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void can_snd_data3(char ctr_cd, char out1)
{
	can_std_frame_t	tx_dataframe;
	t_data0			*p_dt0;

	memset(&tx_dataframe, 0, sizeof(can_std_frame_t));

	tx_dataframe.id = transfer[CAN_CONTROL].sid | NID_RXM;
	tx_dataframe.dlc = 4;
	p_dt0 = (t_data0*)&tx_dataframe.data;
	p_dt0->bit.command = 1;					// コマンドセット
	p_dt0->bit.e = 0;						// シングル転送
	tx_dataframe.data[1] = 0;				// セグメント数
	tx_dataframe.data[2] = ctr_cd;			// コントロールコード
	tx_dataframe.data[3] = out1;			// 出力１

	can_snd_queset(transfer[CAN_CONTROL].sndque, &tx_dataframe);
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN ディスプレイ送信(200h + NID_RXM)
//[]----------------------------------------------------------------------[]
///	@param[in]		*data	: 受信データ
///	@param[in]		length	: データサイズ
///	@return			ret		: void
///	@author			m.nagashima
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/27<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void can_snd_data4(void *data, unsigned short length)
{
	struct _canctr	*ctr = _GET_CAN();
	unsigned short	first_half;
	unsigned short	second_half;

	if (ctr->dispbuf.writeidx >= ctr->dispbuf.readidx) {
		first_half = _MAX_LENGTH_TRANSFERT - ctr->dispbuf.writeidx;
		second_half = ctr->dispbuf.readidx;
		if (ctr->dispbuf.count == _MAX_LENGTH_TRANSFERT) {
			first_half = second_half = 0;
		}
	} else {
		first_half = ctr->dispbuf.readidx - ctr->dispbuf.writeidx;
		second_half = 0;
	}

	if ((first_half + second_half) >= length) {
		if (length > first_half) {
			second_half = length - first_half;
		} else {
			first_half = length;
			second_half = 0;
		}
		memcpy(&ctr->dispbuf.data[ctr->dispbuf.writeidx], data, first_half);
		ctr->dispbuf.writeidx += first_half;
		if (ctr->dispbuf.writeidx == _MAX_LENGTH_TRANSFERT) {
			ctr->dispbuf.writeidx = 0;
		}
		if (second_half) {
			memcpy(&ctr->dispbuf.data[0], (unsigned char *)data + first_half, second_half);
			ctr->dispbuf.writeidx = second_half;
		}
		ctr->dispbuf.count += length;
	}
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN ハートビート送信(700h + NID_RXM)
//[]----------------------------------------------------------------------[]
///	@return			ret		: void
///	@author			m.onouchi
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/02<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void can_snd_data14(void)
{
	volatile struct st_can __evenaccess * can_block_p;
	struct _canctr	*ctr = _GET_CAN();
	can_std_frame_t tx_dataframe;

	LagTim500ms(LAG500_CAN_HEARTBEAT_TIMER, CAN_HEARTBEAT_TIME*2, can_snd_data14);	// ハートビート周期

	can_block_p = (struct st_can __evenaccess *) 0x91200;	// CAN1
	if (can_block_p->STR.BIT.BOST) {		// バスオフ状態
		return;
	}

	memset(&tx_dataframe, 0, sizeof(can_std_frame_t));

	tx_dataframe.id = transfer[CAN_HEARTBEAT].sid | NID_RXM;
	tx_dataframe.dlc = 1;
	tx_dataframe.data[0] = ctr->can_net_stat[NID_RXM];	// ネットワーク状態

	can_snd_queset(transfer[CAN_HEARTBEAT].sndque, &tx_dataframe);
}

//[]----------------------------------------------------------------------[]
///	@brief			入力信号変化イベントの通知
//[]----------------------------------------------------------------------[]
///	@param[in]		*data	: 受信データ
///	@return			ret		: void
///	@author			k.tanaka
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/02/15<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
static void get_can_key_event(can_std_frame_t *dataframe)
{
	unsigned char	i;
	unsigned char	j;
	unsigned char	wkbit;
	unsigned char	on_off;

	for(i = 0; i < dataframe->dlc; ++i) {
		wkbit = 0x80;
		for (j = 0; j < 8; ++j, wkbit>>=1) {
			if ((dataframe->data[i] & wkbit) != (key_dat[i].BYTE & wkbit)) {	// 変化あり
				if (dataframe->data[i] & wkbit) {
					on_off = 1;
				} else {
					on_off = 0;
				}
				queset(OPETCBNO, scan_event_table[i][j], 1, &on_off);
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
				// 人体検知？
				if( scan_event_table[i][j] == KEY_MANDET ){
					lcdbm_notice_ope( LCDBM_OPCD_MAN_DTC, on_off );
				}
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_追加))
			}
		}
		key_dat[i].BYTE = dataframe->data[i];	// key_dat更新
// MH810100(S) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
// 警告通知(lcdbm_notice_alm)のcall元の追加
		// 警告通知	事象	0=解除､1=発生(警告通知が変化した時のみ送信する)
		//		ｺｲﾝ金庫			key_dat[3].BIT.B5
		//		ｺｲﾝﾒｯｸの鍵		key_dat[3].BIT.B7
		//		紙幣ﾘｰﾀﾞｰの鍵	key_dat[3].BIT.B6
		lcdbm_notice_alm( lcdbm_alarm_check() );
// MH810100(E) Y.Watanabe 2019/11/15 車番チケットレス((LCD_IF対応)_変更))
	}
	if (g_ucCAN1RcvSensFlag == 0) {
		g_ucCAN1RcvSensFlag = 1;							// CAN1センサー情報受信完了フラグON
	}
}
