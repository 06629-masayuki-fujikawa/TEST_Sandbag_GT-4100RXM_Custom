/*[]----------------------------------------------------------------------[]
 *|	filename: sysmntcomdr.c
 *[]----------------------------------------------------------------------[]
 *| summary	: SCIドライバ(1パケット抽出機能)
 *| author	: machida.k
 *| date	: 2005-07-22
 *| update	:
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/

/*--------------------------------------------------------------------------*/
/*			I N C L U D E													*/
/*--------------------------------------------------------------------------*/

#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"sysmnt_def.h"

/*--------------------------------------------------------------------------*/
/*			E X T E R N														*/
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/*			D E F I N I T I O N S											*/
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/*			V A L I A B L E S												*/
/*--------------------------------------------------------------------------*/

/* timer counters */
ushort	SYSMNTcomdr_c2c_timer;
ushort	SYSMNTcomdr_sndcmp_timer;

/* COMDR制御データ */
static struct {
	eSYSMNT_COMDR_STS	status;		/* COMDRステータス */
	T_SYSMNT_SCI_PACKET	r_packet;	/* 受信バッファ */
	T_SYSMNT_SCI_PACKET	s_packet;	/* 送信バッファ */
}SYSMNTcomdr_Ctrl;

/*--------------------------------------------------------------------------*/
/*			P R O T O T Y P E S												*/
/*--------------------------------------------------------------------------*/

static	void	port_reset(void);

static	BOOL	pktchk(const T_SYSMNT_SCI_PACKET *rcvpkt);
static	BOOL	bccchk(const T_SYSMNT_SCI_PACKET *rcvpkt);
static	void	pkt2data(T_SYSMNT_PKT *pkt_bin, const T_SYSMNT_SCI_PACKET *pkt_ascii);
static	void	data2pkt(T_SYSMNT_SCI_PACKET *pkt, const T_SYSMNT_PKT *send_data, int type);

static	BOOL	send(uchar *data, int len);

static	void	int_scircv(uchar rcvdata);
static	void	int_scierr(uchar errtype);

/*--------------------------------------------------------------------------*/
/*			F U N C T I O N S												*/
/*--------------------------------------------------------------------------*/

/*====================================================================================[PUBLIC]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: SYSMNTcomdr_Open()
 *[]----------------------------------------------------------------------[]
 *| summary	: SYSMNTcomdrモジュール初期化
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	SYSMNTcomdr_Open(void)
{

/* SCIインタフェース初期化(割り込み無効) */
	SYSMNTsci_Init(int_scircv, int_scierr);
/* 制御データ初期化 */
	SYSMNTcomdr_Reset();
/* SCI割り込み有効化 */
	SYSMNTsci_Start();
}

/*[]----------------------------------------------------------------------[]
 *|	name	: SYSMNTcomdr_Reset()
 *[]----------------------------------------------------------------------[]
 *| summary	: SYSMNTcomdrモジュールリセット
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	SYSMNTcomdr_Reset(void)
{
	ulong	ists;
	
	ists = _di2();
	SYSMNTcomdr_Ctrl.status			= SYSMNT_COMDR_NO_PKT;
	SYSMNTcomdr_Ctrl.r_packet.len	= 0;
	SYSMNTcomdr_sndcmp_timer		= SYSMNT_TIMER_STOP;
	SYSMNTcomdr_c2c_timer			= SYSMNT_TIMER_STOP;
	_ei2(ists);
}

/*[]----------------------------------------------------------------------[]
 *|	name	: SYSMNTcomdr_GetRcvPacket()
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信パケット取得
 *| param	: buf - 受信パケットバッファ	<OUT>
 *|					(戻り値がSYSMNT_SCI_PKT_RCVDのとき有効)
 *| return	: COMDRステータス
 *[]----------------------------------------------------------------------[]*/
eSYSMNT_COMDR_STS	SYSMNTcomdr_GetRcvPacket(T_SYSMNT_PKT *buf)
{
	eSYSMNT_COMDR_STS ret;
	
	ret = SYSMNTcomdr_Ctrl.status;
	
	switch (ret) {
	case SYSMNT_COMDR_NO_PKT:					/* パケット未受信 */
		if (SYSMNTcomdr_Ctrl.r_packet.len > 0) {
			/* 受信中 */
			if (SYSMNTcomdr_c2c_timer <= 0) {
				SYSMNTcomdr_Ctrl.status = SYSMNT_COMDR_PKT_RCVD;
				/* キャラクタ間タイムアウト発生＝1パケット受信完了 */
				if (pktchk(&SYSMNTcomdr_Ctrl.r_packet)) {
					if (SYSMNTcomdr_Ctrl.r_packet.data[0] == STX) {
						if (!bccchk(&SYSMNTcomdr_Ctrl.r_packet)) {
						/* BCCエラー */
							ret = SYSMNT_COMDR_BCC_ERR;
						}
					}
					if (ret != SYSMNT_COMDR_BCC_ERR) {
					/* 受信パケット正常 */
						/* ACII⇒バイナリ変換 */
						pkt2data(buf, &SYSMNTcomdr_Ctrl.r_packet);
						ret = SYSMNT_COMDR_PKT_RCVD;
					}
				}
				else {
				/* 受信パケット異常 */
					ret = SYSMNT_COMDR_PKT_ERR;
				}
				SYSMNTcomdr_Reset();
			}
		}
		break;
	default:								/* 受信エラー発生 */
		SYSMNTcomdr_Reset();
		break;
	}
	
	return ret;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: SYSMNTcomdr_SendPacket()
 *[]----------------------------------------------------------------------[]
 *| summary	: パケット送信(送信完了かタイムアウトまでリターンしない))
 *| param	: buf - 送信パケット	<IN>
 *|			  type - 送信パケットタイプ(SYSMNT_TYPE_SEL,SYSMNT_TYPE_POL,SYSMNT_TYPE_STX)
 *| return	: TRUE - 送信成功
 *[]----------------------------------------------------------------------[]*/
BOOL	SYSMNTcomdr_SendPacket(T_SYSMNT_PKT *buf, int type)
{
	T_SYSMNT_SCI_PACKET *sbuf = &SYSMNTcomdr_Ctrl.s_packet;
	
	data2pkt(sbuf, buf, type);
	
	if (!send(sbuf->data, sbuf->len)) {
		return FALSE;
	}
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: SYSMNTcomdr_SendByte()
 *[]----------------------------------------------------------------------[]
 *| summary	: バイト送信(送信完了かタイムアウトまでリターンしない))
 *| param	: c - 送信データ
 *| return	: TRUE - 送信成功
 *[]----------------------------------------------------------------------[]*/
BOOL	SYSMNTcomdr_SendByte(uchar c)
{
	if (!send(&c, 1)) {
		return FALSE;
	}
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: SYSMNTcomdr_IsSendCmp()
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信完了チェック
 *| param	: timeout - TRUE=タイムアウト発生				<OUT>
 *| return	: TRUE=送信完了orタイムアウト発生
 *[]----------------------------------------------------------------------[]*/
BOOL	SYSMNTcomdr_IsSendCmp(BOOL *timeout)
{
	*timeout = FALSE;
	
	if (SYSMNTsci_IsSndCmp()){
		SYSMNTcomdr_sndcmp_timer = SYSMNT_TIMER_STOP;
		return TRUE;		/* 送信成功 */
	}
	
	if (SYSMNTcomdr_sndcmp_timer <= 0) {
		*timeout = TRUE;
		port_reset();
		return TRUE;		/* 送信タイムアウト */
	}
	
	return FALSE;				/* 送信未完了 */
}

/*[]----------------------------------------------------------------------[]
 *|	name	: SYSMNTcomdr_Close()
 *[]----------------------------------------------------------------------[]
 *| summary	: シリアル割り込み停止
 *| param	: none
 *| return	: none
 *|-----------------------------------------------------------------------[]
 *| remark	: 本関数後、シリアル割り込み再開は SYSMNTcomdr_Open() で行う
 *[]----------------------------------------------------------------------[]*/
void	SYSMNTcomdr_Close(void)
{
	/* SCI割り込み停止 */
	SYSMNTsci_Stop();
}

/*====================================================================================[PRIVATE]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: port_reset()
 *[]----------------------------------------------------------------------[]
 *| summary	: SCIポートリセット
 *| param	: none
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	port_reset(void)
{
	/* SCI割り込み停止 */
	SYSMNTsci_Stop();
	/* COMDR制御データ初期化 */
	SYSMNTcomdr_Reset();
	/* SCIポートリセット＆割り込み有効化 */
	SYSMNTsci_Start();
}

/*[]----------------------------------------------------------------------[]
 *|	name	: pktchk()
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信パケットのフォーマットチェック
 *| param	: rcvpkt - 受信パケット
 *| return	: TRUE - パケット正常
 *[]----------------------------------------------------------------------[]*/
BOOL	pktchk(const T_SYSMNT_SCI_PACKET *rcvpkt)
{
	switch (rcvpkt->data[0]) {
	case ACK:	/* no break */
	case NAK:	/* no break */
	case EOT:
		if (rcvpkt->len != 1) {
			return FALSE;
		}
		break;
	case STX:
		if (rcvpkt->len <= 10) {
			return FALSE;		/* データ部なし */
		}
		/* データブロックNo. */
		if (rcvpkt->data[1] < '0' && '9' < rcvpkt->data[1]) {
			return FALSE;
		}
		if (rcvpkt->data[2] < '0' && '9' < rcvpkt->data[2]) {
			return FALSE;
		}
		/* 中間/最終ブロックフラグ */
		if (rcvpkt->data[3] != '0') {
			return FALSE;
		}
		if (rcvpkt->data[4] != '0' && rcvpkt->data[4] != '1') {
			return FALSE;
		}
		/* データ種別 */
		if (rcvpkt->data[5] != 'B') {
			return FALSE;
		}
		if (rcvpkt->data[6] != '1') {
			return FALSE;
		}
		/* ETX */
		if (rcvpkt->data[rcvpkt->len-3] != ETX) {
			return FALSE;
		}
		break;
	default:
		return FALSE;		/* プロトコルでサポートしない制御コード */
	}
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: bccchk()
 *[]----------------------------------------------------------------------[]
 *| summary	: 受信パケットのBCCチェック
 *| param	: rcvpkt - 受信パケット
 *| return	: TRUE - BCC正常
 *[]----------------------------------------------------------------------[]*/
BOOL	bccchk(const T_SYSMNT_SCI_PACKET *rcvpkt)
{
	uchar bcc = bcccal((char*)&rcvpkt->data[1], (short)(rcvpkt->len-(1+2)));	/* 1+2 = STX+BCC */
	
	if (bcc != astohx((void*)(&rcvpkt->data[rcvpkt->len-2]))) {
		return FALSE;
	}
	
	return TRUE;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: pkt2data()
 *[]----------------------------------------------------------------------[]
 *| summary	: ASCIIフォーマットの受信パケットをバイナリフォーマットに変換する
 *| param	: rcv_data - バイナリフォーマットのパケット		<OUT>
 *|			  pkt - ASCIIフォーマットのパケット		<IN>
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	pkt2data(T_SYSMNT_PKT *rcv_data, const T_SYSMNT_SCI_PACKET *pkt)
{
	int i, len;
	
	/* データをバイナリ変換 */
	rcv_data->type	= pkt->data[0];
	if (rcv_data->type == STX) {
		/* ID1 */
		rcv_data->header.blkno		= (uchar)astoin((void*)(&pkt->data[1]), 2);
		/* ID2 */
		rcv_data->header.blksts		= astohx((void*)(&pkt->data[3]));
		/* ID3 */
		rcv_data->header.data_kind	= astohx((void*)(&pkt->data[5]));
		/* ID4 */
		rcv_data->header.data_format	= astohx((void*)(&pkt->data[7]));
		/* DATA */
		len = pkt->len - SYSMNT_SCI_HEADER_SIZE - 1 - 2;	/* "-1-2"=etx+bcc */
		if (rcv_data->header.data_format & _SYSMNT_DATA_VISIBLE) {
			memcpy(rcv_data->data.data, &pkt->data[SYSMNT_SCI_HEADER_SIZE], (size_t)len);
		}
		else {
			for (i = 0; i < len; i += 2) {
				rcv_data->data.data[i/2] = astohx((void*)(&pkt->data[SYSMNT_SCI_HEADER_SIZE+i]));
			}
			len /= 2;
		}
		/* データ長 */
		rcv_data->data.len = len;
	}
	else {
		rcv_data->data.len = 0;
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: data2pkt()
 *[]----------------------------------------------------------------------[]
 *| summary	: 送信データをパケットに変換する
 *| param	: pkt - 変換後のASCIIパケット		<OUT>
 *|			  send_data - 変換前のバイナリパケット	<IN>
 *|			  type - 送信パケットタイプ(SYSMNT_TYPE_SEL,SYSMNT_TYPE_POL,SYSMNT_TYPE_STX)
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	data2pkt(T_SYSMNT_SCI_PACKET *pkt, const T_SYSMNT_PKT *send_data, int type)
{
	int i, len;
	uchar bcc;
	
	len = 0;
	if (type == SYSMNT_TYPE_STX) {
		/* STX */
		pkt->data[len] = STX;
		len++;
		/* ID1 */
		intoas(&pkt->data[len], (ushort)send_data->header.blkno, 2);
		len += 2;
		/* ID2 */
		hxtoas(&pkt->data[len], send_data->header.blksts);
		len += 2;
		/* ID3 */
		hxtoas(&pkt->data[len], send_data->header.data_kind);
		len += 2;
		/* ID4 */
		hxtoas(&pkt->data[len], send_data->header.data_format);
		len += 2;
		/* DATA */
		if (send_data->header.data_format & _SYSMNT_DATA_VISIBLE) {
			for (i = 0; i < send_data->data.len; i++) {
				pkt->data[len++] = send_data->data.data[i];
			}
		}
		else {
			for (i = 0; i < send_data->data.len; i++) {
				hxtoas(&pkt->data[len], send_data->data.data[i]);
				len += 2;
			}
		}
		/* ETX */
		pkt->data[len] = ETX;
		len++;
		/* BCCセット */
		bcc = bcccal((char*)&pkt->data[1], (short)(len-1));
		hxtoas(&pkt->data[len], bcc);
		len += 2;
	}
	else if (type == SYSMNT_TYPE_SEL || type == SYSMNT_TYPE_POL) {
		pkt->data[0]	= EOT;
		pkt->data[1]	= '0';
		pkt->data[2]	= '0';
		pkt->data[3]	= '0';
		if (type == SYSMNT_TYPE_POL) {
			pkt->data[4] = '0';
		}
		else {	/* type == SYSMNT_TYPE_SEL */
			pkt->data[4] = '1';
		}
		pkt->data[5]	= ENQ;
		len = 6;
	}
	/* パケット長 */
	pkt->len = len;
}

/*[]----------------------------------------------------------------------[]
 *|	name	: send()
 *[]----------------------------------------------------------------------[]
 *| summary	: データ送信
 *| param	: data - 送信データ		<IN>
 *| 		  len - 送信データ長
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
BOOL	send(uchar *data, int len)
{
	/* ドライバに送信を要求 */
	if (!SYSMNTsci_SndReq(data, len)) {
		port_reset();
		return FALSE;			/* 送信不可 */
	}
	
	/* タイムアウト設定 */
	SYSMNTcomdr_sndcmp_timer = _To20msVal(_SYSMNT_COMM_TIMEOUT);
	
	return TRUE;
}

/*====================================================================================[INTERRUPTS]====*/

/*[]----------------------------------------------------------------------[]
 *|	name	: int_scircv()
 *[]----------------------------------------------------------------------[]
 *| summary	: 1バイト受信割り込みハンドラ
 *| param	: rcvdata - 受信した1バイト
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	int_scircv(uchar rcvdata)
{
	if (SYSMNTcomdr_Ctrl.status == SYSMNT_COMDR_NO_PKT) {
		/* キャラクタ間タイムアウト監視開始 */
		SYSMNTcomdr_c2c_timer = _To2msVal(SYSMNT_SCI_C2C_TIMEOUT);
		/* 受信データバッファリング */
		if (SYSMNTcomdr_Ctrl.r_packet.len < sizeof(SYSMNTcomdr_Ctrl.r_packet.data)) {
			SYSMNTcomdr_Ctrl.r_packet.data[SYSMNTcomdr_Ctrl.r_packet.len++] = rcvdata;
		}
		else {
			SYSMNTcomdr_Ctrl.status = SYSMNT_COMDR_ERR_INVALID_LEN;		/* パケット長異常 */
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: int_scierr()
 *[]----------------------------------------------------------------------[]
 *| summary	: 通信エラー割り込みハンドラ
 *| param	: errtype - 発生した通信エラー
 *| return	: none
 *[]----------------------------------------------------------------------[]*/
void	int_scierr(uchar errtype)
{
	switch (errtype) {
	case SYSMNT_SCI_ERR_OVERRUN:
		break;
	case SYSMNT_SCI_ERR_FRAME:
		break;
	case SYSMNT_SCI_ERR_PARITY:
	default:
		break;
	}
	SYSMNTcomdr_Ctrl.status = SYSMNT_COMDR_ERR_SCI;
}

