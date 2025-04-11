// MH810100(S) K.Onodera  2019/11/11 車番チケットレス(GT-8700(LZ-122601)流用)
#include	<string.h>
#include	<stdio.h>
#include	"system.h"
#include	"message.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
#include	"common.h"
#include	"aes_sub.h"
#include	"pkt_def.h"
#include	"pkt_com.h"
#include	"pkt_buffer.h"
#include	"pktctrl.h"
#include	"lcdbm_ctrl.h"
#include	"trsocket.h"
#include	"trsntp.h"
// MH810105(S) R.Endo 2021/12/01 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
#include	"trmacro.h"
#include	"trproto.h"
#include	"trglobal.h"
// MH810105(E) R.Endo 2021/12/01 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発


// ------------------------------------- //
// Declaration
// ------------------------------------- //
int		PKT_Socket = -1;							// ソケット
// MH810105(S) R.Endo 2021/12/01 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
int		SocketOpenErrorCount = 0;					// ソケットオープンエラーのカウンター
// MH810105(E) R.Endo 2021/12/01 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発

// ------------------------------------- //
// Definition
// ------------------------------------- //
#define TIMEOUT_CONNECT				TIMEOUT1		// 接続コマンド開始タイマタイムアウトイベント
#define TIMEOUT_CON_RES_WT			TIMEOUT2		// コネクション応答待ち時間
#define TIMEOUT_CON_INTVL			TIMEOUT3		// コネクション送信間隔タイムアウトイベント
#define TIMEOUT_CON_DIS_WT			TIMEOUT4		// コネクション切断待ち時間タイムアウトイベント
#define TIMEOUT_RES_WT				TIMEOUT5		// パケット応答待ち時間タイムアウトイベント
#define TIMEOUT_KEEPALIVE			TIMEOUT6		// キープアライブ用タイムアウトイベント
#define TIMEOUT_RTPAY_RES			TIMEOUT7		// 精算応答データ待ちタイマタイムアウトイベント
#define TIMEOUT_DC_QR_RES			TIMEOUT8		// QR確定・取消応答データ待ちタイマタイムアウトイベント
#define TIMEOUT_RTPAY_RESND			TIMEOUT9		// 再送ウェイトタイマタイムアウトイベント
#define TIMEOUT_DC_QR_RESND			TIMEOUT10		// 再送ウェイトタイマタイムアウトイベント
#define TIMEOUT_DISCON_WT			TIMEOUT11		// コネクション切断検出待ちタイマタイムアウトイベント
#define TIMEOUT_ERR_GUARD			TIMEOUT12		// 接続エラーガードタイマタイムアウトイベント
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
#define TIMEOUT_DC_LANE_RES			TIMEOUT13		// レーンモニタ応答データ待ちタイマタイムアウトイベント
#define TIMEOUT_DC_LANE_RESND		TIMEOUT14		// 再送ウェイトタイマタイムアウトイベント
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
#define TIMEOUT_RTRECEIPT_RES		TIMEOUT15		// 領収証データ応答待ちタイマタイムアウトイベント
#define TIMEOUT_RTRECEIPT_RESND		TIMEOUT16		// 再送ウェイトタイマタイムアウトイベント
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

#define RESET_KEEPALIVE_COUNT		pktctrl.keepalive_timer = 0

#define RES_ACK						0x06
#define RES_NAK						0x15

#define PKT_RES_SIZE				28				// ACK/NAKデータサイズ

typedef enum{
	TARGET_NORMAL_DATA,
	TARGET_RT_PAY,
	TARGET_DC_QR,
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	TARGET_DC_LANE,
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	TARGET_RT_RECEIPT,
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	TARGET_MAX
} eSEND_TARGET;

enum{
	LOG_PHASE_IDLE,
	LOG_PHASE_WAIT_ACK,			// 1:ログ応答(ACK/NAK)待ち
	LOG_PHASE_RESEND_CANCEL,	// 2:ログデータ再送中断中
	LOG_PHASE_WAIT_RES_CMD,		// 3:ログ応答データ待ち
};

// ------------------------------------------------------------------------------------------------
// PKT Ctrl
static struct{
	int		ready;
	char	buf_type;			// S:送信バッファ，B:再送バッファ(ベース)，A:再送バッファ(追加)

	struct{
		ushort		flag;		// 受信中フラグ
		ushort		usNackRetryNum;
		PKT_RCVRES	buff;
	}recv;

	struct{
		ushort		commited;	// コマンドバッファーにあるデータ数
		ushort		sent;		// ソケットの送信済みバッファー数
		PKT_SNDCMD	buff;
		uchar		res_wait;	// 応答待ち
		ushort		wait_seqno;	// 応答待ちのSEQ
		uchar		rtry_cnt;	// 送信リトライ回数
		uchar		RT_LogSend;	// RTログデータ送信フラグ
		uchar		QR_LogSend;	// QRログデータ送信フラグ
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
		uchar		LANE_LogSend;	// レーンモニタデータ送信フラグ
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
		uchar		RECEIPT_LogSend;	// RT領収証ログデータ送信フラグ
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	}send;

	ushort	recv_port;			// 直接にソケットIDで保存
	uchar	open_mode;			// PKT_SERVER:サーバーモード，PKT_CLIENT:クライアントモード
	uchar	start_up;			// 起動完了通知受信フラグ(0:RT精算データ送信不可/1:送信可)
	uchar	reset_req;			// リセット要求フラグ(0:リセット要求不要/1:要)
	short	keepalive_timer;
	uchar	f_wait_discon;		// リセット通知受信－コネクション切断検出待ちタイマ起動中フラグ

	uchar	sckt_sts;			// ソケットポート状態(通信部からのステータス通知で更新)

	ushort	snd_seqno;			// 送信シーケンス№
	ushort	rcv_seqno;			// 受信シーケンス№

	ulong	ipa;				// 設定№48-0001～0004：LCDパネルＩＰアドレス①～④
	ushort	port;				// 設定№48-0005：LCDポート番号
	uchar	mac[6];				// 通信先MACアドレス

	ulong	ipa_m;				// 設定№41-0002～0003：自局IPアドレス
	ushort	port_m;				// 設定№48-0007：自局ポート

	uchar	err_cnt;			// 設定№48-0010：回線エラー判定回数
	uchar	con_res_tm;			// 設定№48-0011：コネクション応答待ち時間(秒)
	ushort	con_snd_intvl;		// 設定№48-0012：コネクション送信間隔(秒)
	uchar	con_dis_wt_tm;		// 設定№48-0013：コネクション切断待ち時間(秒)
	uchar	rsnd_rtry_ov;		// 設定№48-0014：リトライオーバー時の再送信
	uchar	snd_rtry_cnt;		// 設定№48-0015：パケット送信リトライ回数(無応答)
	uchar	res_wt_tm;			// 設定№48-0016：パケット応答待ち時間(ACK/NAK)(秒)
	uchar	rcv_rtry_cnt;		// 設定№48-0017：パケット受信リトライ回数(NAK)
	ushort	kpalv_snd_intvl;	// 設定№48-0019：キープアライブ送信間隔(秒)
	uchar	kpalv_rtry_cnt;		// 設定№48-0020：キープアライブリトライ回数
	uchar	RTPay_res_tm;		// 設定№48-0023：精算応答データ待ちタイマ(秒)
	uchar	RTPay_resnd_tm;		// 設定№48-0024：再送ウェイトタイマ(秒)
	ushort	discon_wait_tm;		// 設定№48-0025：コネクション切断検出待ちタイマ(秒)
	ushort	err_guard_tm;		// 設定№48-0027：接続エラーガードタイマ(秒)
	uchar	EncDecrypt;			// 設定№48-0040：暗号化通信(0:暗号化なし，1:暗号化あり)

} pktctrl;

typedef	union {
	struct {
		uchar	buff[16];
		ushort	wp;
	} string;
	struct {
		ulong	data_size;		// パケットデータ長(data_size～crc16h)
		uchar	identifier[12];	// ヘッダーコード("AMANO_LCDPKT")
		ushort	dummy;
	} packet;
} RCV_DATA_ROLLER;

static PKT_TRAILER			res_trailer;	// ACK/NAK送信用バッファ
static PKT_TRAILER			snd_trailer;	// コマンド送信用送信バッファ
static PKT_TRAILER			rcv_trailer;	// コマンド受信用バッファ
static lcdbm_cmd_pydt_0pd_t	log_pay_wk;		// RT精算データログ取り出し用work
static lcdbm_cmd_qrdt_0pd_t	log_qr_wk;		// QR確定取消データログ取り出し用work
static uchar EncDecrypt_buff[DECRYPT_COM];	// 暗号化/復号用work
static ulong				Pay_Data_ID;	// 応答チェック用RT精算データID
static ulong				QR_Data_ID;		// 応答チェック用QR確定・取消データID
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
static ulong				LANE_Data_ID;		// 応答チェック用レーンモニタデータID
static lcdbm_cmd_lanedt_0pd_t	log_lane_wk;	// レーンモニタデータログ取り出し用work
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
static lcdbm_cmd_receiptdt_0pd_t	log_receipt_wk;		// RT領収証データログ取り出し用work
static ulong						Receipt_Data_ID;	// 応答チェック用RT領収証データID
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

static uchar	restore_unsent_flg = 1;		// 復旧データ未送信フラグ

static short rcv_data_size;					// 現電文の受信サイズ(パケット跨ぎの場合も含む)
static short prc_data_size_bf;				// パケットを跨いでこれまで受信した現電文のサイズ(現パケットを除く)
static short prc_data_size_nw;				// 1パケット内の解析済みの電文サイズ
static RCV_DATA_ROLLER rcv_data_roller;
static uchar pkt_id_found_flg = 0;			// パケット識別子発見フラグ

static ushort PKTrcv_Receive_Packet( ushort rcv_seqno, PKT_TRAILER *p_trailer );
static void pkt_tenkey_status_check( uchar *tenkey );
static ulong pkt_ksg_err_getreg( int fid, char code, char knd );
static void pkt_can_all_com_err( void );
// MH810105(S) R.Endo 2021/12/01 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
#if DEBUG_JOURNAL_PRINT
static void pkt_socket_debug_print( void );
#endif
// MH810105(E) R.Endo 2021/12/01 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発


// ------------------------------------------------------------------------------------------------

#define	_ref_sckt		(pktctrl.sckt_sts)
#define	_ref_recv		(pktctrl.recv)
#define	_ref_send		(pktctrl.send)
#define	_ref_buf_type	(pktctrl.buf_type)

void PktSocCbk( int sock, int event );

//[]----------------------------------------------------------------------[]
///	@brief		起動通知状態更新
//[]----------------------------------------------------------------------[]
///	@param[in]	faze	0:起動通知受信不可(OPEタスク初期化未完了/RT精算データ送信不可)
///						1:起動通知受信可(OPEタスク初期化完了/RT精算データ送信不可)
///						2:起動完了通知受信済み(OPEタスク初期化完了/RT精算データ送信可)
///	@return		none
/// @note		精算機電源OFF/ON、LCDとのコネクション切断時、RT精算データ
///				送信不可状態とし、LCDからの起動完了通知受信で送信可とする
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void PktStartUpStsChg( uchar faze )
{
	pktctrl.start_up = faze;
}

//[]----------------------------------------------------------------------[]
///	@brief		起動通知状態取得
//[]----------------------------------------------------------------------[]
///	@param[in]	none	
///	@return		faze	0:起動通知受信不可(OPEタスク初期化未完了/RT精算データ送信不可)
///						1:起動通知受信可(OPEタスク初期化完了/RT精算データ送信不可)
///						2:起動完了通知受信済み(OPEタスク初期化完了/RT精算データ送信可)
/// @note		起動完了状態を取得する
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
uchar PktStartUpStsGet( void )
{
	return pktctrl.start_up;
}

//[]----------------------------------------------------------------------[]
///	@brief		リセット要求フラグセット処理
//[]----------------------------------------------------------------------[]
///	@param[in]	rst_req		0:リセット要求不要
///							1:リセット要求要
///	@return		none
/// @note		LCDに対するリセット要求フラグをセットする
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void PktResetReqFlgSet( uchar rst_req )
{
	pktctrl.reset_req = rst_req;
}

//[]----------------------------------------------------------------------[]
///	@brief		リセット要求フラグ取得
//[]----------------------------------------------------------------------[]
///	@param[in]	none
///	@return		reset_req	0:リセット要求不要
///							1:リセット要求要
/// @note		LCDに対するリセット要求フラグを取得する
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
uchar PktResetReqFlgRead( void )
{
	return pktctrl.reset_req;
}

//[]----------------------------------------------------------------------[]
///	@brief			設定取得
//[]----------------------------------------------------------------------[]
///	@param			なし
///	@return			なし
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/10/31<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void GetSetting( void )
{
	char	ip[20];					// IPアドレス変換用

	// 接続先(LCD)IPアドレス
// MH810100(S) S.Nishimoto 2020/04/07 静的解析(20200407:13,14,15,16)対応
//	sprintf( ip, "%03d.%03d.%03d.%03d",
	sprintf( ip, "%03ld.%03ld.%03ld.%03ld",
// MH810100(E) S.Nishimoto 2020/04/07 静的解析(20200407:13,14,15,16)対応
			prm_get( COM_PRM, S_PKT, 1, 3, 1 ),
			prm_get( COM_PRM, S_PKT, 2, 3, 1 ),
			prm_get( COM_PRM, S_PKT, 3, 3, 1 ),
			prm_get( COM_PRM, S_PKT, 4, 3, 1 )
	);
	pktctrl.ipa = htonl( inet_addr(ip) );

	// 接続先(LCD)ポート
	pktctrl.port = (ushort)prm_get(COM_PRM, S_PKT, 5, 5, 1 );

	// 自局IPアドレス
// MH810100(S) S.Nishimoto 2020/04/07 静的解析(20200407:17,18,19,20)対応
//	sprintf( ip, "%03d.%03d.%03d.%03d",
	sprintf( ip, "%03ld.%03ld.%03ld.%03ld",
// MH810100(E) S.Nishimoto 2020/04/07 静的解析(20200407:17,18,19,20)対応
			prm_get( COM_PRM, S_MDL, 2, 3, 4 ),
			prm_get( COM_PRM, S_MDL, 2, 3, 1 ),
			prm_get( COM_PRM, S_MDL, 3, 3, 4 ),
			prm_get( COM_PRM, S_MDL, 3, 3, 1 )
	);
	pktctrl.ipa_m = htonl( inet_addr(ip) );

	// 自局ポート
	pktctrl.port_m = (ushort)prm_get( COM_PRM, S_PKT, 7, 5, 1 );

	// 回線エラー判定回数
	pktctrl.err_cnt = (ushort)prm_get( COM_PRM, S_PKT, 10, 2, 1 );
	if( 99 < pktctrl.err_cnt ){
		pktctrl.err_cnt = 1;
	}

	// コネクション応答待ち時間(秒)
	pktctrl.con_res_tm = (uchar)prm_get( COM_PRM, S_PKT, 11, 2, 1 );
	if( pktctrl.con_res_tm < 1 || 20 < pktctrl.con_res_tm ){
		pktctrl.con_res_tm = 2;
	}

	// コネクション送信間隔(秒)
	pktctrl.con_snd_intvl = (ushort)prm_get( COM_PRM, S_PKT, 12, 3, 1 );
// MH810100(S) K.Onodera  2020/03/25 #4093 車番チケットレス(コネクション送信間隔に0秒を指定すると、LCDが起動しなくなる不具合対応)
//	if( 999 < pktctrl.con_snd_intvl ){
	if( pktctrl.con_snd_intvl < 1 || 999 < pktctrl.con_snd_intvl ){
// MH810100(E) K.Onodera  2020/03/25 #4093 車番チケットレス(コネクション送信間隔に0秒を指定すると、LCDが起動しなくなる不具合対応)
		pktctrl.con_snd_intvl = 3;
	}

	// コネクション切断待ち時間(秒)
	pktctrl.con_dis_wt_tm = (uchar)prm_get( COM_PRM, S_PKT, 13, 2, 1 );
// MH810100(S) K.Onodera  2020/03/25 車番チケットレス(#4093と同様に「0」は無効値とする)
//	if( 99 < pktctrl.con_dis_wt_tm ){
	if( pktctrl.con_dis_wt_tm < 1 || 99 < pktctrl.con_dis_wt_tm ){
// MH810100(E) K.Onodera  2020/03/25 車番チケットレス(#4093と同様に「0」は無効値とする)
		pktctrl.con_dis_wt_tm = 3;
	}

	// リトライオーバー時の再送信
	pktctrl.rsnd_rtry_ov = (uchar)prm_get( COM_PRM, S_PKT, 14, 1, 1 );
	if( 1 < pktctrl.rsnd_rtry_ov ){
		pktctrl.rsnd_rtry_ov = 1;
	}

	// パケット送信リトライ回数
	pktctrl.snd_rtry_cnt = (uchar)prm_get( COM_PRM, S_PKT, 15, 2, 1 );
	if( 99 < pktctrl.snd_rtry_cnt ){
		pktctrl.snd_rtry_cnt = 1;
	}

	// パケット応答待ち時間(ACK/NAK)(秒)
	pktctrl.res_wt_tm = (uchar)prm_get( COM_PRM, S_PKT, 16, 2, 1 );
// MH810100(S) K.Onodera  2020/03/25 車番チケットレス(#4093と同様に「0」は無効値とする)
//	if( 99 < pktctrl.res_wt_tm ){
	if( pktctrl.res_wt_tm < 1 || 99 < pktctrl.res_wt_tm ){
// MH810100(E) K.Onodera  2020/03/25 車番チケットレス(#4093と同様に「0」は無効値とする)
		pktctrl.res_wt_tm = 3;
	}

	// パケット受信リトライ回数(NAK)
	pktctrl.rcv_rtry_cnt = (uchar)prm_get( COM_PRM, S_PKT, 17, 2, 1 );
	if( 99 < pktctrl.rcv_rtry_cnt ){
		pktctrl.rcv_rtry_cnt = 3;
	}

	// キープアライブ送信間隔(秒)
	// 実際には1秒タイマが(設定№48-0019)回タイムアウトで送信間隔到達とみなす
	pktctrl.kpalv_snd_intvl = (ushort)prm_get( COM_PRM, S_PKT, 19, 3, 1 );
	if( 999 < pktctrl.kpalv_snd_intvl ){
		pktctrl.kpalv_snd_intvl = 10;
	}

	// キープアライブリトライ回数
	pktctrl.kpalv_rtry_cnt = (uchar)prm_get( COM_PRM, S_PKT, 20, 2, 1 );
	if( pktctrl.kpalv_rtry_cnt < 1 || 10 < pktctrl.kpalv_rtry_cnt ){
		pktctrl.kpalv_rtry_cnt = 3;
	}

	// 精算応答データ待ちタイマ(秒)
	pktctrl.RTPay_res_tm = (uchar)prm_get( COM_PRM, S_PKT, 23, 2, 1 );
	if( pktctrl.RTPay_res_tm < 1 || 99 < pktctrl.RTPay_res_tm ){
		pktctrl.RTPay_res_tm = 15;
	}

	// 再送ウェイトタイマ(秒)
	pktctrl.RTPay_resnd_tm = (uchar)prm_get( COM_PRM, S_PKT, 24, 2, 1 );
	if( pktctrl.RTPay_resnd_tm < 1 || 99 < pktctrl.RTPay_resnd_tm ){
		pktctrl.RTPay_resnd_tm = 10;
	}

	// コネクション切断検出待ちタイマ(秒)
	pktctrl.discon_wait_tm = (ushort)prm_get( COM_PRM, S_PKT, 25, 3, 1 );
	if( pktctrl.discon_wait_tm < 1 || 999 < pktctrl.discon_wait_tm ){
		pktctrl.discon_wait_tm = 300;
	}

	// 接続エラーガードタイマ(秒)
	pktctrl.err_guard_tm = (ushort)prm_get( COM_PRM, S_PKT, 27, 3, 1 );
	if( pktctrl.err_guard_tm < 1 || 999 < pktctrl.err_guard_tm ){
		pktctrl.err_guard_tm = 90;
	}

	// 暗号化通信(0:暗号化なし，1:暗号化あり)
	pktctrl.EncDecrypt = (uchar)prm_get( COM_PRM, S_PKT, 40, 1, 1 );
	//if( 1 < pktctrl.EncDecrypt ){
	//	pktctrl.EncDecrypt = 1;
	//}

}

//[]----------------------------------------------------------------------[]
///	@brief			ソケット設定
//[]----------------------------------------------------------------------[]
///	@param			なし
///	@return			なし
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/01<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static int PktSocCfg( void )
{
	int		sock = -1;				// ソケットディスクリプタ
	int		on = 1;					// オプション有効
	int		maxrt;
	struct sockaddr_in	myAddr;		// 自局アドレス・ポート番号

	// ソケット作成
	sock = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( sock == -1 ){
// MH810105(S) R.Endo 2021/12/21 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
// 		pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_SOC, 2 );
		if ( SocketOpenErrorCount < 50 ) {
			// 50回まではエラー登録
			pkt_ksg_err_getreg(sock, ERR_TKLSLCD_KSG_SOC, 2);
#if DEBUG_JOURNAL_PRINT
			// 10回毎にデバッグ印字
			if ( (SocketOpenErrorCount % 10) == 0 ) {
				pkt_socket_debug_print();
			}
#endif
		} else {
			// 50回を超えたら待機か休業か初期化状態であればリセット
			if ( (OPECTL.Mnt_mod == 0) && ((OPECTL.Ope_mod == 0) || (OPECTL.Ope_mod == 100) || (OPECTL.Ope_mod == 255)) ) {
				System_reset();	// Main CPU reset (It will not retrun from this function)
			}
		}
		SocketOpenErrorCount++;
// MH810105(E) R.Endo 2021/12/21 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
		return sock;
	}
// MH810105(S) R.Endo 2021/12/01 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
	SocketOpenErrorCount = 0;
// MH810105(E) R.Endo 2021/12/01 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発

	// ノンブロッキング
	if( -1 == tfBlockingState( sock, TM_BLOCKING_OFF ) ){
		pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_BLK_ST, 2 );
		if( -1 == tfClose(sock) ){
			pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_CLS, 2 );
		}
		return -1;
	}

	// TIME_WAITでもポートのオープンを可能にする
	if( -1 == setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on) ) ){
		pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_SOC_OP, 2 );
		if( -1 == tfClose(sock) ){
			pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_CLS, 2 );
		}
		return -1;
	}

	// connectのタイムアウト時間を変更する
	maxrt = PKT_CONN_TIMEOUT;
	if( -1 == setsockopt( sock, IP_PROTOTCP, TCP_MAXRT, (char*)&maxrt, sizeof(maxrt) ) ){
		pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_SOC_OP, 2 );
		if( -1 == tfClose( sock ) ){
			pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_CLS, 2 );
		}
		return -1;
	}

	// ソケットイベントコールバック関数の登録
	if( -1 == tfRegisterSocketCB( sock, PktSocCbk,
						TM_CB_RECV | TM_CB_RECV_OOB |								// データ受信
						TM_CB_REMOTE_CLOSE | TM_CB_SOCKET_ERROR |					// 相手からの切断、エラー
						TM_CB_RESET | TM_CB_CLOSE_COMPLT | TM_CB_CONNECT_COMPLT )){	// 相手からのリセット、クローズ完了、接続完了
		pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_SOC_CB, 2 );
	}

	// 自局ポートを割り当てる
	memset( &myAddr, 0x00, sizeof(myAddr) );
	myAddr.sin_family      = AF_INET;
	myAddr.sin_port        = pktctrl.port_m;	// 自局ポート番号
	myAddr.sin_addr.s_addr = pktctrl.ipa_m;		// 自局IPアドレス

	if( -1 == bind( sock, (struct sockaddr *)&myAddr, sizeof(myAddr) ) ){
		pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_BND, 2 );
		if( -1 == tfClose( sock ) ){
			pkt_ksg_err_getreg( sock, ERR_TKLSLCD_KSG_CLS, 2 );
		}
		sock = -1;
	}

	return sock;
}

//[]----------------------------------------------------------------------[]
///	@brief			TCP送信処理
//[]----------------------------------------------------------------------[]
///	@param			なし
///	@return			なし
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/01<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
int PktTcpSend( uchar *data, ulong len )
{
	int iRet;

	iRet = send( PKT_Socket, (char*)data, len, 0 );
	// 送信失敗？
	if( iRet <= 0 ){
		pkt_ksg_err_getreg( PKT_Socket, ERR_TKLSLCD_KSG_SND, 2 );
	}
	taskchg( IDLETSKNO );

	return iRet;
}

//[]----------------------------------------------------------------------[]
///	@brief			TCP受信処理
//[]----------------------------------------------------------------------[]
///	@param			なし
///	@return			なし
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/01<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void PktTcpRecv( int fid )
{
	int er;
	ushort i;
	ushort iDiff;
	int		sockError;
	ushort	err_cnt = 0;
	uchar	rst_cause;

	if( !_ref_recv.flag ){	// 受信パケット処理中ではない
		rcv_data_size = 0;
		prc_data_size_bf = 0;
		prc_data_size_nw = 0;
		memset(&rcv_trailer, 0x00, sizeof(rcv_trailer));	// 受信電文格納領域クリア
		pkt_id_found_flg = 0;			// パケット識別子発見フラグ
	}

	while(1) {
		er = recv(fid, (char*)&_ref_recv.buff.rcv.rcv_data, PKT_DATA_MAX, 0);
		if( er > 0 ){
			// 受信データあり
			if( !rcv_data_size ){	// 初回受信処理
				if( er < 28 ){	// ヘッダサイズ(26Byte)＋CRC16よりも受信データ長が短い
					err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_RCV_LENGTH, 2, 0, 0, NULL );
					return;	// 受信データ破棄
				}
			}
			_ref_recv.flag = 1;

			_ref_recv.buff.com.kind = PKT_RCV_NOTIFY;
			_ref_recv.buff.rcv.data_size = er;
			pktctrl.recv_port = (ushort)fid;				// fidを保存

			_ref_sckt = PKT_PORT_ESTABLISH;
			err_cnt = 0;
		}
		else{
			sockError = (int)pkt_ksg_err_getreg( fid, ERR_TKLSLCD_KSG_RCV, 2 );
			if( sockError == TM_EWOULDBLOCK ){
				// ノンブロッキングモード(エラーでない)
				if( _ref_recv.flag == 1 ){	// 受信中
					if( ++err_cnt < 10 ){
						taskchg( IDLETSKNO );
						continue;
					}
				}
			}
			else{
				// エラー発生(TCP切断)
				rst_cause = ERR_TKLSLCD_KSG_RCV;
				queset( PKTTCBNO, PKT_RST_PRT, 1, &rst_cause );
			}
			return;
		}

		// パケット受信処理
		if (_ref_recv.buff.rcv.data_size) {
			if ((PKT_RCV_NOTIFY == _ref_recv.buff.com.kind)) {
				if( !rcv_data_size ){	// 初回受信処理
					// ヘッダ情報格納エリアクリア
					memset(&rcv_data_roller, 0x00, sizeof(rcv_data_roller));
				}	// 分割された後続データ受信時はヘッダ情報が格納済みなのでクリアしない
				iDiff = 0;
				i = 0;
				while(1)
				{
					if( pkt_id_found_flg == 0){
						// iDiffは、0は受信した最初。それ以外は今回受信した残りの解析
						for (i=iDiff; i<_ref_recv.buff.rcv.data_size; i++) {	// 今回受信サイズ分ループ
							WACDOG;	// ウォッチドックリセット実行
							rcv_data_roller.string.buff[rcv_data_roller.string.wp++] = _ref_recv.buff.rcv.rcv_data[i];
							if (rcv_data_roller.string.wp == 16) {
								if (memcmp(rcv_data_roller.packet.identifier, "AMANO_LCDPKT", 12) == 0) {
									memcpy(rcv_trailer.data, rcv_data_roller.string.buff, 16);
									rcv_data_size = 16;				// 16に変更
									pkt_id_found_flg = 1;			// パケット識別子発見フラグ
									++i;							// 
									break;
								}else{
									// 見つからない場合は、1文字ずらして探していく
									memmove(&rcv_data_roller.string.buff[0], &rcv_data_roller.string.buff[1], 15);
									rcv_data_roller.string.wp--;
								}
							}
						}
					}
					if( pkt_id_found_flg == 1){
						// 受信したサイズ or 1電文のサイズの小さい方をCOPY
//						er = min(_ref_recv.buff.rcv.data_size - i,rcv_trailer.part.data_size - rcv_data_size);
						er = _ref_recv.buff.rcv.data_size - i;
// MH810100(S) 2020/06/26 #4508【エージング】DCNETの割引認証登録・取消要求データが再送されることがある
//						if( er > (rcv_trailer.part.data_size)){
//							er = rcv_trailer.part.data_size;
//						}
						if( er > (rcv_trailer.part.data_size - rcv_data_size)){
							er = rcv_trailer.part.data_size - rcv_data_size;
						}
// MH810100(S) 2020/06/26 #4508【エージング】DCNETの割引認証登録・取消要求データが再送されることがある
						memcpy(&rcv_trailer.data[rcv_data_size],&_ref_recv.buff.rcv.rcv_data[i],er);
						rcv_data_size = rcv_data_size + er;

						if( rcv_data_size >= rcv_trailer.part.data_size ){	// 1電文受信完了
							ushort seqTemp = PKTrcv_Receive_Packet(pktctrl.rcv_seqno, &rcv_trailer);
							if(seqTemp != 0xFFFF) {
								pktctrl.rcv_seqno = seqTemp;
							}
							// 今回受信パケット内に未処理の電文あり？(今回受信総サイズ - (今回解析電文サイズ(現パケットのみ) - 解析済み電文サイズ))
							if( (short)_ref_recv.buff.rcv.data_size - ((rcv_data_size - prc_data_size_bf) + prc_data_size_nw) > 0 ){
								prc_data_size_nw += rcv_data_size - prc_data_size_bf;		// 解析済み電文サイズに今回解析電文サイズ(現パケットのみ)を加算
								iDiff += rcv_data_size;
								rcv_data_size = 0;											// 次電文解析の為に受信サイズクリア
								prc_data_size_bf = 0;										// 1電文受信完了したのでパケット跨ぎの受信電文サイズクリア
								memset(&rcv_trailer, 0x00, sizeof(rcv_trailer));			// 受信電文格納領域クリア
								memset(&rcv_data_roller, 0x00, sizeof(rcv_data_roller));	// ヘッダ格納領域クリア
								pkt_id_found_flg = 0;			// パケット識別子発見フラグ
								continue;	// 今回受信パケットの次電文受信・分析へ
							}else{	// 未処理電文なし
								break;	// ループを抜ける(受信完了)
							}
						}
					}
					// 見つからない場合は抜ける
					break;
				}
				// 今回受信サイズ分の処理完了
				if( rcv_data_size >= rcv_trailer.part.data_size ){
					// 今回受信サイズ分処理し、かつ電文サイズ分処理完了
					rcv_data_size = 0;
					prc_data_size_bf = 0;
					prc_data_size_nw = 0;
					_ref_recv.flag = 0;
					pkt_id_found_flg = 0;			// パケット識別子発見フラグ
					break;
				}else{
					// パケット識別子が見つかっていない場合は、破棄。見つかっている場合は引継ぎ
					if(pkt_id_found_flg == 1){
						prc_data_size_bf = rcv_data_size;	// これまでの受信サイズをバックアップ
						prc_data_size_nw = 0;				// 1パケット内の解析済みの電文サイズクリア
					}else{
						rcv_data_size = 0;
						prc_data_size_bf = 0;
						prc_data_size_nw = 0;
						_ref_recv.flag = 0;
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)

switch(pktctrl.send.QR_LogSend){
case LOG_PHASE_IDLE:
wmonlg( OPMON_TEST, 0, 30 );
	break;
case LOG_PHASE_WAIT_ACK:			// 1:ログ応答(ACK/NAK)待ち
wmonlg( OPMON_TEST, 0, 31 );
	break;
case LOG_PHASE_RESEND_CANCEL:	// 2:ログデータ再送中断中
wmonlg( OPMON_TEST, 0, 32 );
	break;
case LOG_PHASE_WAIT_RES_CMD:		// 3:ログ応答データ待ち
wmonlg( OPMON_TEST, 0, 33 );
	break;
default:
wmonlg( OPMON_TEST, 0, 34 );
	break;
}
#endif
// 調査用 (e)
						break;
					}
				}
			}
		}
		taskchg( IDLETSKNO );
	}
}

static void pkt_send_req_sndAckNack(uchar ucKind, uchar ucDetail, short SndSeqNo)
{
	PKT_TRAILER*	p_trailer = &res_trailer;
	union {
		ushort	us;
		struct {
			uchar	high;
			uchar	low;
		} uc;
	} crc16;

	memset(p_trailer, 0, sizeof(PKT_TRAILER));

	p_trailer->part.data_size = 28;		// CRC16(2byte)
	memcpy(&p_trailer->part.identifier, "AMANO_LCDPKT", sizeof(p_trailer->part.identifier));
	intoas(p_trailer->part.seqno, SndSeqNo, sizeof(p_trailer->part.seqno));
	p_trailer->part.id = 0x00;
	p_trailer->part.kind = 0x10;
	p_trailer->part.blkno_h = ucKind;		// Ack Nak
	p_trailer->part.blkno_l = ucDetail;	// Detail
	p_trailer->part.block_sts = 0;
	p_trailer->part.encrypt = 0;
	p_trailer->part.pad_size = 0;

	crc_ccitt((ushort)(p_trailer->part.data_size-2), (uchar *)&p_trailer->data[0], (uchar *)&crc16.us, (uchar)R_SHIFT);	// CRC16算出(little endian)
	p_trailer->data[p_trailer->part.data_size-2] = crc16.uc.high;	// little endianのまま格納
	p_trailer->data[p_trailer->part.data_size-1] = crc16.uc.low;	//

	PktTcpSend((uchar*)p_trailer, (ulong)p_trailer->part.data_size);
}

//[]----------------------------------------------------------------------[]
///	@brief			AMANO_LCDPKTの受信処理
//[]----------------------------------------------------------------------[]
///	@param[in]		rcv_seqno	: 前回受信シーケンス№(0～999)
///	@param[out]		*p_trailer	: 受信したパケットデータ
///	@return			受信シーケンス№(0～999)
///	@note			正常なパケットかをチェックしたあと受信バッファに格納します。<br>
///					またコマンド単位でオペレーションタスクに通知します。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2009/09/29<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
static ushort PKTrcv_Receive_Packet(ushort rcv_seqno, PKT_TRAILER *p_trailer)
{
	ushort	seqno;
	union {
		ushort	us;
		struct {
			uchar	high;
			uchar	low;
		} uc;
	} crc16;
	ushort	crc16us;
	uchar	*rcv;
	long	len;
	ushort	size;
	ushort	rp;
	ushort	msg;
	ushort	cmd_sz;
	uchar	cmd_dat[MSGBUFSZ];
	int		insize;
	ulong	id;
	uchar	result;

	seqno = astoin(p_trailer->part.seqno, sizeof(p_trailer->part.seqno));
	if(_ref_send.res_wait) {
		crc16.us = crc16us = 0;
		crc16.uc.high = p_trailer->part.data[p_trailer->part.data_size-28];	// little endianのまま評価
		crc16.uc.low = p_trailer->part.data[p_trailer->part.data_size-27];	//
		crc_ccitt((ushort)(p_trailer->part.data_size-2), (uchar *)&p_trailer->data[0], (uchar *)&crc16us, (uchar)R_SHIFT);	// CRC16算出(little endian)
		if(( p_trailer->part.id == 0x00 ) &&		// データ部コード
		   ( p_trailer->part.kind == 0x10 ) &&		// 電文識別コード
		   ( crc16.us == crc16us ) &&
		   ( p_trailer->part.blkno_h == RES_ACK || p_trailer->part.blkno_h == RES_NAK))		// 06H=ACK,15H=NAK
		{
			// ---------------------------------------------------
			// ACK / NACK受信時処理
			// ---------------------------------------------------
			// Ack/Nakが返ってきたので初期化
			// キープアライブタイマリセット
			Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, 50 );
			RESET_KEEPALIVE_COUNT;
			// パケット応答待ち時間タイマ停止
			Lagcan( PKTTCBNO, PKT_TIM_RES_WT );
			_ref_send.res_wait = 0;

			// ACK受信？
			if( p_trailer->part.blkno_h == RES_ACK ){
				_ref_send.rtry_cnt = 0;	// リトライカウンタクリア
				// 送信したのはRTログデータ？
				if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_ACK ){
					// 正常ACK？
					if( p_trailer->part.blkno_l == 0x00 ){
						Lagtim( PKTTCBNO, PKT_TIM_RTPAY_RES, (ushort)(pktctrl.RTPay_res_tm*50) );	// パケット応答データ待ちタイマ開始
						pktctrl.send.RT_LogSend = LOG_PHASE_WAIT_RES_CMD;		// ログ応答データ待ち
					}
					// 強制ACK応答(リトライオーバー)
					else{
						Ope_Log_UndoTargetDataVoidRead( eLOG_RTPAY, eLOG_TARGET_LCD );	// 空読みを元に戻す
						pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信状態に戻す
						_ref_buf_type = 'S';
					}
				}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
				// 送信したのはRTログデータ？
				else if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_ACK ) {
					// 正常ACK？
					if ( p_trailer->part.blkno_l == 0x00 ) {
						Lagtim(PKTTCBNO, PKT_TIM_RTRECEIPT_RES, (ushort)(pktctrl.RTPay_res_tm*50));	// パケット応答データ待ちタイマ開始
						pktctrl.send.RECEIPT_LogSend = LOG_PHASE_WAIT_RES_CMD;	// ログ応答データ待ち
					}
					// 強制ACK応答(リトライオーバー)
					else {
						Ope_Log_UndoTargetDataVoidRead( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// 空読みを元に戻す
						pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信状態に戻す
						_ref_buf_type = 'S';
					}
				}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
				// 送信したのはQRログデータ？
				else if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_ACK ){
					// 正常ACK？
					if( p_trailer->part.blkno_l == 0x00 ){
						Lagtim( PKTTCBNO, PKT_TIM_DC_QR_RES, (ushort)(pktctrl.RTPay_res_tm*50) );	// パケット応答データ待ちタイマ開始
						pktctrl.send.QR_LogSend = LOG_PHASE_WAIT_RES_CMD;		// ログ応答データ待ち
					}
					// 強制ACK応答(リトライオーバー)
					else{
						// ログデータは強制ACK応答受信でも破棄しない
						Ope_Log_UndoTargetDataVoidRead( eLOG_DC_QR, eLOG_TARGET_LCD );	// 空読みを元に戻す
						pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信状態に戻す
						_ref_buf_type = 'S';
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 2 );
#endif
// 調査用 (e)
					}
				}
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
				// 送信したのはレーンモニタログデータ？
				else if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_ACK ){
					// 正常ACK？
					if( p_trailer->part.blkno_l == 0x00 ){
						Lagtim( PKTTCBNO, PKT_TIM_DC_QR_RES, (ushort)(pktctrl.RTPay_res_tm*50) );	// パケット応答データ待ちタイマ開始
						pktctrl.send.LANE_LogSend = LOG_PHASE_WAIT_RES_CMD;		// ログ応答データ待ち
					}
					// 強制ACK応答(リトライオーバー)
					else{
						// ログデータは強制ACK応答受信でも破棄しない
						Ope_Log_UndoTargetDataVoidRead( eLOG_DC_LANE, eLOG_TARGET_LCD );	// 空読みを元に戻す
						pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信状態に戻す
						_ref_buf_type = 'S';
					}
				}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
				// 通常データに対する応答？
				else{
					if( PktResetReqFlgRead() ){	// リセット要求に対するACK？
						PktResetReqFlgSet( 0 );	// リセット要求フラグクリア
					}
					// 強制ACK応答も含めて送信完了として次データ送信とする
					_ref_send.sent += 1;
					if( _ref_send.sent > 9999 ){
						_ref_send.sent = 0;
					}
					// ログデータ再送中断中？
					if( pktctrl.send.RT_LogSend == LOG_PHASE_RESEND_CANCEL ){
						pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// ログデータ再送中断解除
						_ref_buf_type = 'S';
					}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
					else if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_RESEND_CANCEL ) {
						pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;	// ログデータ再送中断解除
						_ref_buf_type = 'S';
					}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
					else if( pktctrl.send.QR_LogSend == LOG_PHASE_RESEND_CANCEL ){
						pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// ログデータ再送中断解除
						_ref_buf_type = 'S';
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 13 );
#endif
// 調査用 (e)
					}
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
					else if( pktctrl.send.LANE_LogSend == LOG_PHASE_RESEND_CANCEL ){
						pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;	// ログデータ再送中断解除
						_ref_buf_type = 'S';
					}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
				}
				if( p_trailer->part.blkno_l == 0x01 ){	// 強制ACK応答(リトライオーバー)受信
					err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_RCV_FORCED_ACK, 2, 0, 0, NULL );
				}
				// 再送バッファをクリアして次データを送信
				PKTbuf_ClearReSendCommand();
				queset( PKTTCBNO, PKT_SND_PRT, 0, NULL );
			}
			// NAK受信？
			else{
				if( ++_ref_send.rtry_cnt > pktctrl.snd_rtry_cnt ){	// 送信リトライオーバー
					_ref_send.rtry_cnt = 0;	// リトライカウンタクリア
					// 送信したのはRT精算ログデータ？
					if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_ACK ){
						// 再送を中断してログデータ以外の送信を先に行う
						PKTbuf_ClearReSendCommand();	// 再送バッファクリア
						pktctrl.send.RT_LogSend = LOG_PHASE_RESEND_CANCEL;	// ログデータ再送中断
						return 0xFFFF;
					}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
					// 送信したのはRT領収証ログデータ？
					else if( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_ACK ){
						// 再送を中断してログデータ以外の送信を先に行う
						PKTbuf_ClearReSendCommand();	// 再送バッファクリア
						pktctrl.send.RECEIPT_LogSend = LOG_PHASE_RESEND_CANCEL;	// ログデータ再送中断
						return 0xFFFF;
					}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
					// 送信したのはQR確定・取消ログデータ？
					else if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_ACK ){
						// 再送を中断してログデータ以外の送信を先に行う
						PKTbuf_ClearReSendCommand();	// 再送バッファクリア
						pktctrl.send.QR_LogSend = LOG_PHASE_RESEND_CANCEL;	// ログデータ再送中断
						return 0xFFFF;
					}
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
					// 送信したのはレーンモニタログデータ？
					else if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_ACK ){
						// 再送を中断してログデータ以外の送信を先に行う
						PKTbuf_ClearReSendCommand();	// 再送バッファクリア
						pktctrl.send.LANE_LogSend = LOG_PHASE_RESEND_CANCEL;	// ログデータ再送中断
						return 0xFFFF;
					}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
					// 送信したのは通常データ？
					else{
						if( ( pktctrl.rsnd_rtry_ov ) &&	// リトライオーバー時の再送信する
							( PKTbuf_CheckReSendCommand() != LCDBM_CMD_NOTIFY_CONFIG )){	// 基本設定要求でない
							;	// 再送信(基本設定要求はリトライオーバー時の再送信対象外とする)
						}else{
							err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_SND_RTRY_OV, 2, 0, 0, NULL );
							_ref_send.sent += 1;	// 送信済みとする
							PKTbuf_ClearReSendCommand();	// 再送バッファクリア
							// RT精算ログデータ再送中断中？
							if( pktctrl.send.RT_LogSend == LOG_PHASE_RESEND_CANCEL ){
								// RT精算データ再送中断を解除してRT精算データ再送開始
								pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;
								_ref_buf_type = 'S';
							}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
							// RT領収証ログデータ再送中断中？
							else if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_RESEND_CANCEL ) {
								// RT領収証データ再送中断を解除してRT領収証データ再送開始
								pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;
								_ref_buf_type = 'S';
							}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
							// QR確定・取消ログデータ再送中断中？
							else if( pktctrl.send.QR_LogSend == LOG_PHASE_RESEND_CANCEL ){
								// QR確定・取消データ再送中断を解除してQR確定・取消データ再送開始
								pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;
								_ref_buf_type = 'S';
							}
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
							// レーンモニタログデータ再送中断中？
							else if( pktctrl.send.LANE_LogSend == LOG_PHASE_RESEND_CANCEL ){
								// レーンモニタデータ再送中断を解除してレーンモニタデータ再送開始
								pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;
								_ref_buf_type = 'S';
							}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
							return 0xFFFF;
						}
					}
				}
				// 再送(ここに来るのはリトライオーバ前とリトライオーバ時も再送する場合)

				// RT通信？
				if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_ACK ){
					Ope_Log_UndoTargetDataVoidRead( eLOG_RTPAY, eLOG_TARGET_LCD );	// 空読みを元に戻す
					pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// RT精算データ未送信状態に戻す
					_ref_buf_type = 'S';
				}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
				// RT通信(領収証データ)？
				else if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_ACK ) {
					Ope_Log_UndoTargetDataVoidRead( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// 空読みを元に戻す
					pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;	// RT領収証データ未送信状態に戻す
					_ref_buf_type = 'S';
				}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
				// DC-NET通信？
				else if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_ACK ){
					Ope_Log_UndoTargetDataVoidRead( eLOG_DC_QR, eLOG_TARGET_LCD );	// 空読みを元に戻す
					pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// QR確定・取消データ未送信状態に戻す
					_ref_buf_type = 'S';
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 3 );
#endif
// 調査用 (e)
				}
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
				// DC-NET通信(レーンモニタデータ)？
				else if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_ACK ){
					Ope_Log_UndoTargetDataVoidRead( eLOG_DC_LANE, eLOG_TARGET_LCD );	// 空読みを元に戻す
					pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;	// レーンモニタデータ未送信状態に戻す
					_ref_buf_type = 'S';
				}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
				// 通常データ
				else{
					_ref_buf_type = 'A';
				}
				queset( PKTTCBNO, PKT_SND_PRT, 0, NULL );
			}
			return 0xFFFF;
		}
		if(( p_trailer->part.id != 0x00 ) ||		// データ部コード
		   ( p_trailer->part.kind != 0x10)){		// 電文識別コード
			err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_RCV_HDR, 2, 0, 0, NULL );
		}else if( crc16.us != crc16us ){
			err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_RCV_CRC, 2, 0, 0, NULL );
		}
	}
	else {
		if(p_trailer->part.data_size == PKT_RES_SIZE) {
			// TCP再送等により応答待ち以外で応答データを受信した場合は破棄する
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
switch(pktctrl.send.QR_LogSend){
case LOG_PHASE_IDLE:
wmonlg( OPMON_TEST, 0, 8 );
	break;
case LOG_PHASE_WAIT_ACK:			// 1:ログ応答(ACK/NAK)待ち
wmonlg( OPMON_TEST, 0, 9 );
	break;
case LOG_PHASE_RESEND_CANCEL:	// 2:ログデータ再送中断中
wmonlg( OPMON_TEST, 0, 10 );
	break;
case LOG_PHASE_WAIT_RES_CMD:		// 3:ログ応答データ待ち
wmonlg( OPMON_TEST, 0, 11 );
	break;
default:
wmonlg( OPMON_TEST, 0, 12 );
	break;
}
#endif
// 調査用 (e)
			return 0xFFFF;
		}
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
else{
switch(pktctrl.send.QR_LogSend){
case LOG_PHASE_WAIT_ACK:			// 1:ログ応答(ACK/NAK)待ち
wmonlg( OPMON_TEST, 0, 45 );
}
}
#endif
// 調査用 (e)
	}


	if((seqno == 0) || (seqno != rcv_seqno)){
		crc16.us = crc16us = 0;
		crc16.uc.high = p_trailer->part.data[p_trailer->part.data_size-28];	// little endianのまま評価
		crc16.uc.low = p_trailer->part.data[p_trailer->part.data_size-27];	//
		// CRCは暗号化されている場合もそのまま算出
		crc_ccitt((ushort)(p_trailer->part.data_size-2), (uchar *)&p_trailer->data[0], (uchar *)&crc16us, (uchar)R_SHIFT);	// CRC16算出(little endian)
		if(( p_trailer->part.id == 0x00 )	&&	// データ部コード(00H 固定)
		   ( p_trailer->part.kind == 0x10 ) &&	// 電文識別コード(10H=パケットトレイラ（固定）)
		   ( p_trailer->part.blkno_l == 1 ) &&	// ブロックNo.(1固定)
		   ( p_trailer->part.block_sts == 1 )){	// ブロックステータス(1固定)
			if( crc16.us == crc16us ){
				rp = 0;
				if( p_trailer->part.encrypt == 1 ){	// 暗号化方式=AES
					// 復号化する
					insize = (int)(p_trailer->part.data_size-26-2);	// データ部サイズ＝パケットデータ長－ヘッダサイズ－CRC16サイズ
					if( insize > DECRYPT_COM ){	// 受信電文長＞復号化用エリアサイズ
						insize = 0;	// 復号化NGとする
					}else{
						insize = DecryptWithKeyAndIV_NoPadding( CRYPT_KEY_RXMLCDPK, (uchar*)&p_trailer->part.data, EncDecrypt_buff, insize );
					}
					if( !insize ){	// 復号化NG
						// 受信データ復号化エラー(発生／解除同時)
						err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_RCV_DCRYPT_FL, 2, 0, 0, NULL );
						_ref_recv.usNackRetryNum++;
						if(_ref_recv.usNackRetryNum > pktctrl.rcv_rtry_cnt ) {
							// NAKリトライオーバー(ACK:応答詳細=01H(リトライオーバーの為強制ACK応答))
							err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_SND_FORCED_ACK, 2, 0, 0, NULL );
							pkt_send_req_sndAckNack(RES_ACK, 0x01, seqno);

							// RTログ応答データ待ち？
							if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_RES_CMD ){
								Lagcan( PKTTCBNO, PKT_TIM_RTPAY_RES );	//  ログ応答データ待ちタイマ停止
								Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTPAY, eLOG_TARGET_LCD );	// 送信済みとする
								pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信状態に戻す
								// 通常データ送信中の可能性もあるので_ref_buf_typeは変更しない
							}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
							// RTログ応答データ(領収証データ)待ち？
							if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_RES_CMD ) {
								Lagcan( PKTTCBNO, PKT_TIM_RTRECEIPT_RES );	//  ログ応答データ待ちタイマ停止
								Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// 送信済みとする
								pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信状態に戻す
								// 通常データ送信中の可能性もあるので_ref_buf_typeは変更しない
							}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
							// DCログ応答データ待ち？
							if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_RES_CMD ){
								Lagcan( PKTTCBNO, PKT_TIM_DC_QR_RES );	//  ログ応答データ待ちタイマ停止
								Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_QR, eLOG_TARGET_LCD );	// 送信済みとする
								pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信状態に戻す
								// 通常データ送信中の可能性もあるので_ref_buf_typeは変更しない
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 14 );
#endif
// 調査用 (e)
							}
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
							// DCログ応答データ(レーンモニタデータ)待ち？
							if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_RES_CMD ){
								Lagcan( PKTTCBNO, PKT_TIM_DC_QR_RES );	//  ログ応答データ待ちタイマ停止
								Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_LANE, eLOG_TARGET_LCD );	// 送信済みとする
								pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信状態に戻す
								// 通常データ送信中の可能性もあるので_ref_buf_typeは変更しない
							}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
						} else {
							pkt_send_req_sndAckNack(RES_NAK, 0x11, seqno);
						}
						return 0xFFFF;
					}
					rcv = EncDecrypt_buff;
				}else{	// 暗号化なし
					rcv = p_trailer->part.data;
				}
				// キープアライブタイマリセット
				Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, 50 );
				RESET_KEEPALIVE_COUNT;
				_ref_recv.usNackRetryNum = 0;
				pkt_send_req_sndAckNack(RES_ACK, 0x00,seqno);
				len = *(ushort *)&rcv[rp];				// 最初の2byteはパケットのデータ長を格納するエリア
				rp += 2;

				while(0<len) {
					taskchg(IDLETSKNO);
					size = *(ushort *)&rcv[rp];
					rp += 2;
					// コマンドID/サブコマンドIDをチェックし、メッセージ番号に変換
					// (以下は「==」ではなく戻り値(msg)が「0」か否かを見ています)
					if (msg = lcdbm_check_message(*(ulong *)&rcv[rp])) {
						cmd_sz = size;
						if ( cmd_sz > MSGBUFSZ ) {
							cmd_sz = MSGBUFSZ;
						}
						memcpy( cmd_dat, &rcv[rp], cmd_sz );
						queset( OPETCBNO, msg, cmd_sz, cmd_dat );
					} else {
						switch( *(ulong *)&rcv[rp] ){
							case LCDBM_RSP_NOTIFY_CONFIG:	// 基本設定応答
								// 基本設定応答の受信情報を保存(opeタスクへの通知なし)
								lcdbm_receive_config((lcdbm_rsp_config_t *)&rcv[rp]);
								// キープアライブリトライカウンタリセット
								lcdbm_reset_keepalive_status();

								// 復旧データ送信要(起動時)？
								if( pkt_get_restore_unsent_flg() == 1 ){
									// リアルタイム精算データあり？
									if( Ope_Log_UnreadCountGet( eLOG_RTPAY, eLOG_TARGET_LCD ) ){
										result = 1;		// 未送信有り
									}
									// データなし？
									else{
										result = 0;		// 全データ送信完了
									}
									PKTcmd_notice_ope( LCDBM_OPCD_RESTORE_NOTIFY, (ushort)result );
									pkt_set_restore_unsent_flg(0);
								}
								break;
							case LCDBM_RSP_TENKEY_KEYDOWN:	// テンキー押下情報
								// 受信ボタン状態の変化をチェック(関数内でopeタスクへqueset())
								pkt_tenkey_status_check(&rcv[rp+4]);
								break;
							case LCDBM_RSP_NOTICE_OPE:	// 操作通知
								// 復旧データ要求受信
								if( rcv[rp+4] == LCDBM_OPCD_RESTORE_REQUEST ){
									// リアルタイム精算データあり？
									if( Ope_Log_UnreadCountGet( eLOG_RTPAY, eLOG_TARGET_LCD ) ){
										result = 1;		// 未送信有り
									}
									// データなし？
									else{
										result = 0;		// 全データ送信完了
									}
									PKTcmd_notice_ope( LCDBM_OPCD_RESTORE_RESULT, (ushort)result );
									pkt_set_restore_unsent_flg(0);
									break;
								}
								// リセット通知
								if( rcv[rp+4] == LCDBM_OPCD_RESET_NOT ){
									// リセット通知なら切断検出待ちタイマ開始(その間の切断検出はエラー登録対象外)
									Lagtim( PKTTCBNO, PKT_TIM_DISCON_WAIT, pktctrl.discon_wait_tm*50 );	// コネクション切断検出待ちタイマ開始
									pktctrl.f_wait_discon = 1;	// コネクション切断検出待ちタイマ起動中フラグON
								}
								cmd_sz = size;
								if ( cmd_sz > MSGBUFSZ ) {
									cmd_sz = MSGBUFSZ;
								}
								memcpy( cmd_dat, &rcv[rp], cmd_sz );
								queset( OPETCBNO, LCD_OPERATION_NOTICE, cmd_sz, cmd_dat );
								break;
							case LCDBM_RSP_IN_INFO:			// 入庫情報
								// 入庫情報の受信情報を保存(関数内でopeタスクへqueset())
								lcdbm_receive_in_car_info((lcdbm_rsp_in_car_info_t *)&rcv[rp]);
								break;
							case LCDBM_RSP_QR_DATA:			// QRデータ
								// QRデータの受信情報を保存(関数内でopeタスクへqueset())
								lcdbm_receive_QR_data((lcdbm_rsp_QR_data_t *)&rcv[rp]);
								break;
							case LCDBM_RSP_PAY_DATA_RES:	// 精算応答データ
								// 精算応答データ待ち？
								if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_RES_CMD ){
									// 受信データ解析
									lcdbm_receive_RTPay_res( (lcdbm_rsp_pay_data_res_t *)&rcv[rp], &id, &result );
									if( id == Pay_Data_ID ){	// 精算データのIDと一致
										Lagcan( PKTTCBNO, PKT_TIM_RTPAY_RES );	//  ログデータ応答データ待ちタイマ停止
										if( result == 0 ){	// 結果OK
											Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTPAY, eLOG_TARGET_LCD );	// 送信済みとする

											// リアルタイム精算データ残なし？
											if( 0 == Ope_Log_UnreadCountGet( eLOG_RTPAY, eLOG_TARGET_LCD ) ){
												// 復旧データ送信要(復旧時)？
												if( pkt_get_restore_unsent_flg() == 2 ){
													// 復旧データ通知(全データ送信完了)
													PKTcmd_notice_ope( LCDBM_OPCD_RESTORE_RESULT, (ushort)0 );
													pkt_set_restore_unsent_flg(0);
												}
											}
										}else{	// 結果NG
											// 再送ウェイトタイマ後に再送
											Ope_Log_UndoTargetDataVoidRead( eLOG_RTPAY, eLOG_TARGET_LCD );	// 空読みを元に戻す
											Lagtim( PKTTCBNO, PKT_TIM_RTPAY_RESND, pktctrl.RTPay_resnd_tm*50 );	// 再送ウェイトタイマ開始
										}
										pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信
										// ログデータ待ち中は他データ送信中の可能性もあるので_ref_buf_typeは変更しない
									}	// ID不一致の時は受け捨て(応答待ちタイマ継続)
								}	// 応答待ちでない時は受け捨て

// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
else{
switch(pktctrl.send.RT_LogSend){
case LOG_PHASE_IDLE:
wmonlg( OPMON_TEST, 0, 40 );
	break;
case LOG_PHASE_WAIT_ACK:			// 1:ログ応答(ACK/NAK)待ち
wmonlg( OPMON_TEST, 0, 41 );
	break;
case LOG_PHASE_RESEND_CANCEL:	// 2:ログデータ再送中断中
wmonlg( OPMON_TEST, 0, 42 );
	break;
case LOG_PHASE_WAIT_RES_CMD:		// 3:ログ応答データ待ち
wmonlg( OPMON_TEST, 0, 43 );
	break;
default:
wmonlg( OPMON_TEST, 0, 44 );
	break;
}
switch(pktctrl.send.QR_LogSend){
case LOG_PHASE_IDLE:
wmonlg( OPMON_TEST, 0, 35 );
	break;
case LOG_PHASE_WAIT_ACK:			// 1:ログ応答(ACK/NAK)待ち
wmonlg( OPMON_TEST, 0, 36 );
	break;
case LOG_PHASE_RESEND_CANCEL:	// 2:ログデータ再送中断中
wmonlg( OPMON_TEST, 0, 37 );
	break;
case LOG_PHASE_WAIT_RES_CMD:		// 3:ログ応答データ待ち
wmonlg( OPMON_TEST, 0, 38 );
	break;
default:
wmonlg( OPMON_TEST, 0, 39 );
	break;
}

}
#endif
// 調査用 (e)
								break;
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
							case LCDBM_RSP_RECEIPT_DATA_RES:	// 領収証データ応答
								// 領収証データ応答待ち？
								if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_RES_CMD ) {
									// 受信データ解析
									lcdbm_receive_RTReceipt_res( (lcdbm_rsp_receipt_data_res_t *)&rcv[rp], &id, &result );
									if ( id == Receipt_Data_ID ) {	// 領収証データのIDと一致
										Lagcan( PKTTCBNO, PKT_TIM_RTRECEIPT_RES );	//  ログデータ応答データ待ちタイマ停止
										if ( result == 0 ) {	// 結果OK
											Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// 送信済みとする
										} else {				// 結果NG
											// 再送ウェイトタイマ後に再送
											Ope_Log_UndoTargetDataVoidRead( eLOG_RTRECEIPT, eLOG_TARGET_LCD );		// 空読みを元に戻す
											Lagtim( PKTTCBNO, PKT_TIM_RTRECEIPT_RESND, pktctrl.RTPay_resnd_tm*50 );	// 再送ウェイトタイマ開始
										}
										pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信
										// ログデータ待ち中は他データ送信中の可能性もあるので_ref_buf_typeは変更しない
									}	// ID不一致の時は受け捨て(応答待ちタイマ継続)
								}	// 応答待ちでない時は受け捨て
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
							case LCDBM_RSP_QR_CONF_CAN_RES:	// QR確定・取消データ応答
								// QR確定取消応答データ待ち？
								if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_RES_CMD ){
									// 受信データ解析
									lcdbm_receive_DC_QR_res( (lcdbm_rsp_QR_conf_can_res_t *)&rcv[rp], &id, &result );
									if( id == QR_Data_ID ){	// QR確定・取消データのIDと一致
										Lagcan( PKTTCBNO, PKT_TIM_DC_QR_RES );	//  ログデータ応答データ待ちタイマ停止
// MH810100(S) K.Onodera  2020/03/16 車番チケットレス(#4034 DC-NETで送信データ強制停止を受信してもデータを再送し続けてしまう)
//										if( result == 0 ){	// 結果OK
										// 結果OK or Nack99で送信済み
										if( result == 0 || result == 1 ){
// MH810100(E) K.Onodera  2020/03/16 車番チケットレス(#4034 DC-NETで送信データ強制停止を受信してもデータを再送し続けてしまう)
											Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_QR, eLOG_TARGET_LCD );	// 送信済みとする
										}else{	// 結果NG
											// 再送ウェイトタイマ後に再送
											Ope_Log_UndoTargetDataVoidRead( eLOG_DC_QR, eLOG_TARGET_LCD );	// 空読みを元に戻す
											Lagtim( PKTTCBNO, PKT_TIM_DC_QR_RESND, pktctrl.RTPay_resnd_tm*50 );	// 再送ウェイトタイマ開始
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 4 );
#endif
// 調査用 (e)
										}
										pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信
										// ログデータ待ち中は他データ送信中の可能性もあるので_ref_buf_typeは変更しない
									}	// ID不一致の時は受け捨て(応答待ちタイマ継続)
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
else{
	wmonlg( OPMON_TEST, 0, 15 );
}
#endif
// 調査用 (e)
								}	// 応答待ちでない時は受け捨て
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
else{
//	wmonlg( OPMON_TEST, 0, 16 );

switch(pktctrl.send.QR_LogSend){
case LOG_PHASE_IDLE:
wmonlg( OPMON_TEST, 0, 25 );
	break;
case LOG_PHASE_WAIT_ACK:			// 1:ログ応答(ACK/NAK)待ち
wmonlg( OPMON_TEST, 0, 26 );
	break;
case LOG_PHASE_RESEND_CANCEL:	// 2:ログデータ再送中断中
wmonlg( OPMON_TEST, 0, 27 );
	break;
case LOG_PHASE_WAIT_RES_CMD:		// 3:ログ応答データ待ち
wmonlg( OPMON_TEST, 0, 28 );
	break;
default:
wmonlg( OPMON_TEST, 0, 29 );
	break;
}

}
#endif
// 調査用 (e)
								break;
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
							case LCDBM_RSP_LANE_RES:	// レーンモニタデータ応答
								// レーンモニタデータ応答待ち？
								if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_RES_CMD ){	
									// 受信データ解析
									lcdbm_receive_DC_LANE_res( (lcdbm_rsp_LANE_res_t *)&rcv[rp], &id, &result );
									if( id == LANE_Data_ID ){	// レーンモニタデータのIDと一致
										Lagcan( PKTTCBNO, PKT_TIM_DC_LANE_RES );	//  ログデータ応答データ待ちタイマ停止
										// 結果OK or Nack99で送信済み
										if( result == 0 || result == 1 ){
											Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_LANE, eLOG_TARGET_LCD );	// 送信済みとする
										}else{	// 結果NG
											// 再送ウェイトタイマ後に再送
											Ope_Log_UndoTargetDataVoidRead( eLOG_DC_LANE, eLOG_TARGET_LCD );	// 空読みを元に戻す
											Lagtim( PKTTCBNO, PKT_TIM_DC_LANE_RESND, pktctrl.RTPay_resnd_tm*50 );	// 再送ウェイトタイマ開始
										}
										pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信
										// ログデータ待ち中は他データ送信中の可能性もあるので_ref_buf_typeは変更しない
									}	// ID不一致の時は受け捨て(応答待ちタイマ継続)
								}	// 応答待ちでない時は受け捨て
								break;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
							case LCDBM_RSP_MNT_QR_CTRL_RES:	// QRリーダ制御応答
								// QRリーダ制御応答の受信情報を保存(関数内でopeタスクへqueset())
								lcdbm_receive_QR_ctrl_res((lcdbm_rsp_QR_ctrl_res_t *)&rcv[rp]);
								break;
							case LCDBM_RSP_MNT_QR_RD_RSLT:	// QR読取結果
								// QR読取結果の受信情報を保存(関数内でopeタスクへqueset())
								lcdbm_receive_QR_rd_rslt((lcdbm_rsp_QR_rd_rslt_t *)&rcv[rp]);
								break;
							default:	// 不明コマンドは破棄
								break;
						}
					}
					rp += size;
					len -= (size + 2);
				}
			}
			// CRCチェックNG
			else{
				// 受信データCRCエラー(発生／解除同時)
				err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_RCV_CRC, 2, 0, 0, NULL );
				_ref_recv.usNackRetryNum++;
				if(_ref_recv.usNackRetryNum > pktctrl.rcv_rtry_cnt ) {
					// Nakリトライオーバー(ACK:応答詳細=01H(リトライオーバーの為強制ACK応答))
					err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_SND_FORCED_ACK, 2, 0, 0, NULL );
					pkt_send_req_sndAckNack(RES_ACK, 0x01, seqno);

					// RTログ応答データ待ち？
					if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_RES_CMD ){
						Lagcan( PKTTCBNO, PKT_TIM_RTPAY_RES );	//  RT精算応答データ待ちタイマ停止
						Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTPAY, eLOG_TARGET_LCD );	// 送信済みとする
						pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信状態に戻す
						// 精算応答データ待ち中は他データ送信中の可能性もあるので
						// _ref_buf_typeは変更しない
					}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
					// RT領収証ログ応答データ待ち？
					if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_RES_CMD ) {
						Lagcan( PKTTCBNO, PKT_TIM_RTRECEIPT_RES );	// RT領収証データ応答待ちタイマ停止
						Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// 送信済みとする
						pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信状態に戻す
						// 精算応答データ待ち中は他データ送信中の可能性もあるので
						// _ref_buf_typeは変更しない
					}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
					// QRログ応答データ待ち？
					if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_RES_CMD ){
						Lagcan( PKTTCBNO, PKT_TIM_DC_QR_RES );	//  RT精算応答データ待ちタイマ停止
						Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_QR, eLOG_TARGET_LCD );	// 送信済みとする
						pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信状態に戻す
						// 精算応答データ待ち中は他データ送信中の可能性もあるので
						// _ref_buf_typeは変更しない
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 17 );
#endif
// 調査用 (e)
					}
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
					// レーンモニタログ応答データ待ち？
					if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_RES_CMD ){
						Lagcan( PKTTCBNO, PKT_TIM_DC_QR_RES );	//  RT精算応答データ待ちタイマ停止
						Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_LANE, eLOG_TARGET_LCD );	// 送信済みとする
						pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信状態に戻す
						// 精算応答データ待ち中は他データ送信中の可能性もあるので
						// _ref_buf_typeは変更しない
					}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
				} else {
					pkt_send_req_sndAckNack(RES_NAK, 0x11, seqno);
				}
				return 0xFFFF;
			}
		}
		else{
			// 受信データヘッダエラー(発生／解除同時)
			err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_RCV_HDR, 2, 0, 0, NULL );
		}
	}
	else {	// 同一のSEQ
		// 受信データシーケンス№エラー(発生／解除同時)
		err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_RCV_SEQ_NO, 2, 0, 0, NULL );
		_ref_recv.usNackRetryNum++;
		if(_ref_recv.usNackRetryNum > pktctrl.rcv_rtry_cnt ) {
			// Nakリトライオーバー(ACK:応答詳細=01H(リトライオーバーの為強制ACK応答))
			err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_SND_FORCED_ACK, 2, 0, 0, NULL );
			pkt_send_req_sndAckNack(RES_ACK, 0x1, seqno);

			// RTログ応答データ待ち？
			if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_RES_CMD ){
				Lagcan( PKTTCBNO, PKT_TIM_RTPAY_RES );	//  RT精算応答データ待ちタイマ停止
				Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTPAY, eLOG_TARGET_LCD );	// 送信済みとする
				pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// RT精算データ未送信状態に戻す
				// 精算応答データ待ち中は他データ送信中の可能性もあるので
				// _ref_buf_typeは変更しない
			}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
			// RT領収証ログ応答データ待ち？
			if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_RES_CMD ) {
				Lagcan( PKTTCBNO, PKT_TIM_RTRECEIPT_RES );	// RT領収証データ応答待ちタイマ停止
				Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// 送信済みとする
				pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;	// RT領収証データ未送信状態に戻す
				// 領収証データ応答待ち中は他データ送信中の可能性もあるので
				// _ref_buf_typeは変更しない
			}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
			// QRログ応答データ待ち？
			if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_RES_CMD ){
				Lagcan( PKTTCBNO, PKT_TIM_DC_QR_RES );	//  QR確定・取消応答データ待ちタイマ停止
				Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_QR, eLOG_TARGET_LCD );	// 送信済みとする
				pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// QR確定・取消データ未送信状態に戻す
				// 精算応答データ待ち中は他データ送信中の可能性もあるので
				// _ref_buf_typeは変更しない
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 18 );
#endif
// 調査用 (e)
			}
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
			// レーンモニタログ応答データ待ち？
			if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_RES_CMD ){
				Lagcan( PKTTCBNO, PKT_TIM_DC_QR_RES );	//  QR確定・取消応答データ待ちタイマ停止
				Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_LANE, eLOG_TARGET_LCD );	// 送信済みとする
				pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;	// レーンモニタデータ未送信状態に戻す
				// 精算応答データ待ち中は他データ送信中の可能性もあるので
				// _ref_buf_typeは変更しない
			}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
		} else {
			pkt_send_req_sndAckNack(RES_NAK, 0x11, seqno);
		}
		return 0xFFFF;
	}

	memset(p_trailer, 0, sizeof(PKT_TRAILER));
	return(seqno);
}

//[]----------------------------------------------------------------------[]
///	@brief			HIFデータ送信処理
//[]----------------------------------------------------------------------[]
///	@param			なし
///	@return			なし
///	@note			HIF送信バッファにHIFデータを格納します。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/01<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void pkt_send_start( void )
{
	uchar	rst_cause;

	int iRet = PktTcpSend( (uchar*)&_ref_send.buff.snd.snd_data, (ulong)_ref_send.buff.snd.data_size );

	// 送信成功（要求サイズと送信済みサイズが一致）
	if( iRet == _ref_send.buff.snd.data_size ){
		// パケット応答待ちタイマを開始して応答待ち状態とする
		Lagtim( PKTTCBNO, PKT_TIM_RES_WT, (ushort)(pktctrl.res_wt_tm*50) );
		_ref_send.res_wait = 1;
		_ref_send.wait_seqno = pktctrl.snd_seqno;
	}
	// 送信失敗？
	else {
		// 故障発生するため、ソケットリセット
		rst_cause = ERR_TKLSLCD_KSG_SND;
		queset( PKTTCBNO, PKT_RST_PRT, 1, &rst_cause );
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 19 );
#endif
// 調査用 (e)
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			TCP Connect要求
//[]----------------------------------------------------------------------[]
///	@param			なし
///	@return			なし
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/01<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static int pkt_send_req_con( void )
{
	struct sockaddr_in	serverAddr;			// HOSTアドレス・ポート番号
	int sockError;							// ソケットエラーコード
	int ret = -1;

	if( pktctrl.ipa > 0 && pktctrl.port > 0 ){
		memset( &serverAddr, 0, sizeof(serverAddr) );
		serverAddr.sin_family      = AF_INET;
		serverAddr.sin_port        = pktctrl.port;		// 接続先(LCD)ポート番号
		serverAddr.sin_addr.s_addr = pktctrl.ipa;		// 接続先(LCD)IPアドレス

		ret = connect( PKT_Socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr) );
		if( ret == -1 ){
			sockError = (int)pkt_ksg_err_getreg( PKT_Socket, ERR_TKLSLCD_KSG_CON, 2 );
			if( sockError == TM_EINPROGRESS ||			// ノンブロッキングなので接続中はエラーじゃない
				sockError == TM_EISCONN ){				// 既に接続中はエラーではない(connect～GetSockError間で接続完了)
				ret = 0;								// エラーとしない
			}
		}
	}
	return ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			パケット送信要求
//[]----------------------------------------------------------------------[]
///	@param[in]		fid		: 機能ID(FID_PT1～FID_PT4)
///	@param[in]		size	: データサイズ
///	@param[in]		data	: データ
///	@param[in]		push	: Pushフラグ(サブCPUの送信オプション)
///	@return			なし
///	@attention		Pushフラグを1にした場合は即時パケット化されて送信します。
///	@note			指定されたデータにデータヘッダ(10byte)を付加して送信する。<br>
///					(HIFヘッダはHIF送信タスクが付加します)
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/01<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void pkt_send_req_snd( ushort size, uchar *data, uchar push )
{
	memset( &(_ref_send.buff), 0, sizeof(PKT_SND_CMD) );

	_ref_send.buff.snd.kind = PKT_PKT_SND_REQ;
	// 送信データ識別番号
	_ref_send.buff.snd.snd_no = 0;
	// ブロックステータス
	_ref_send.buff.snd.block_sts = push;
	// データサイズ
	_ref_send.buff.snd.data_size = size;
	// データ
	memcpy( _ref_send.buff.snd.snd_data, data, (size_t)(_ref_send.buff.snd.data_size) );

	pkt_send_start();
}

//[]----------------------------------------------------------------------[]
///	@brief			ノンブロッキングのコールバック
//[]----------------------------------------------------------------------[]
///	@param			なし
///	@return			なし
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/01<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void PktSocCbk( int sock, int event )
{
	uchar	rst_cause;

// MH810105(S) R.Endo 2021/12/13 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
#if DEBUG_JOURNAL_PRINT
	T_FrmDebugData frm_debug_data;
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
	struct clk_rec	wk_CLK_REC_debug;
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）

	// クローズ完了、相手からのリセット、相手からの切断、エラー
	if ( (event & TM_CB_CLOSE_COMPLT) ||
		 (event & TM_CB_RESET) ||
		 (event & TM_CB_REMOTE_CLOSE) ||
		 (event & TM_CB_SOCKET_ERROR) ) {
		// クリア
		memset(&frm_debug_data, 0x00, sizeof(frm_debug_data));

		// 現在日時
		c_Now_CLK_REC_ms_Read(
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
//			&frm_debug_data.wk_CLK_REC,
			&wk_CLK_REC_debug,
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
			&frm_debug_data.wk_CLK_REC_msec);
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		// 7バイトのみCOPY
		memcpy( &frm_debug_data.wk_CLK_YMDHMS, &wk_CLK_REC_debug, sizeof(DATE_YMDHMS) );
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）

		// ジャーナル印字
		frm_debug_data.prn_kind = J_PRI;

		// 1行目
		frm_debug_data.first_line = TRUE;
		sprintf(frm_debug_data.data, "cur_sock:%-2d sock:%-2d event:%04x", PKT_Socket, sock, event);
		queset(PRNTCBNO, PREQ_DEBUG, sizeof(T_FrmDebugData), &frm_debug_data);
	}
#endif
// MH810105(E) R.Endo 2021/12/13 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発

	// ソケットが異なるまたは既にクローズ済み？
	if( PKT_Socket != sock ){
		return;
	}

	// 相手からのReset / 相手からの切断 / エラー発生
	if( (event & TM_CB_RESET) || (event & TM_CB_REMOTE_CLOSE) || (event & TM_CB_SOCKET_ERROR) ){
		if( PKT_Socket != -1 ){
			if( event & TM_CB_RESET ){
				rst_cause = ERR_TKLSLCD_RMT_RST;
			}else if( event & TM_CB_REMOTE_CLOSE ){
				rst_cause = ERR_TKLSLCD_RMT_DSC;
			}else{
				rst_cause = ERR_TKLSLCD_ERR_OCC;
			}
			// ソケットクローズはタスクでイベントを受信時に行う
			queset( PKTTCBNO, PKT_RST_PRT, 1, &rst_cause );
		}
	}
	// 接続完了
	else if( event & TM_CB_CONNECT_COMPLT ){
		queset( PKTTCBNO, PKT_SOC_CON, 0, NULL );
	}
	// データ受信
	else if( (event & TM_CB_RECV) || (event & TM_CB_RECV_OOB) ){
		queset( PKTTCBNO, PKT_SOC_RCV, 0, NULL );
	}

	return;
}

//[]----------------------------------------------------------------------[]
///	@brief			パケット通信機能初期化
//[]----------------------------------------------------------------------[]
///	@param			なし
///	@return			なし
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/10/31<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void pkt_init( void )
{
	// 以下、0クリアされていても明示的に初期値が分かるように記述している
	// ※起動時、以下が動作する前にopetask()からリセット要求フラグを書き
	//   込みに来るので、ここでpktctrlをmemset()でallクリアするのは不可
	_ref_sckt = PKT_PORT_DISCONNECT;// ソケットポート状態
	_ref_buf_type = 'S';			// S:送信バッファ，B:再送バッファ(ベース)，A:再送バッファ(追加)
	_ref_send.commited = 0;			// コマンドバッファーにあるデータ数
	_ref_send.sent = 0;				// ソケットの送信済みバッファー数
	_ref_send.res_wait = 0;			// 応答待ち
	_ref_send.wait_seqno = 0;		// 応答待ちのSEQ
	_ref_recv.usNackRetryNum = 0;
	_ref_recv.flag = 0;
	pktctrl.start_up = 0;			// 起動未完了(RT精算データ送信不可)
	pktctrl.send.RT_LogSend = 0;	// RT精算データ送信フラグ
	pktctrl.send.QR_LogSend = 0;	// QR確定・取消データ送信フラグ
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
	pktctrl.send.LANE_LogSend = 0;	// レーンモニタデータ送信フラグ
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
	pktctrl.send.RECEIPT_LogSend = 0;	// RT領収証ログデータ送信フラグ
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
	pktctrl.f_wait_discon = 0;		// リセット通知受信－コネクション切断検出待ちタイマ起動中フラグ
	lcdbm_reset_keepalive_status();	// キープアライブリトライカウンタリセット

	// PKTバッファ初期化
	PKTbuf_init();

	pktctrl.ready = 1;			// HIF電文受信許可
}

//[]----------------------------------------------------------------------[]
///	@brief			AMANO_LCDPKTの生成
//[]----------------------------------------------------------------------[]
///	@param[in]		snd_seqno	: 送信シーケンス№(0～999)
///	@param[out]		*p_trailer	: 生成したパケットデータ
///	@return			パケットのデータ長
///	@note			バッファリングされているコマンドからパケットデータを生成します。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/05<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ushort PKTsnd_Create_Packet( ushort snd_seqno, PKT_TRAILER *p_trailer )
{
	ushort	len = 2;	// 最初の2byteはパケットのデータ長を格納するエリア
	ushort	size = 0;
	ushort	limit = PKT_CMD_MAX - 2;
	ushort	num = 0;
	union {
		ushort	us;
		struct {
			uchar	high;
			uchar	low;
		} uc;
	} crc16;

	memset( p_trailer, 0, sizeof(PKT_TRAILER) );
	memset( EncDecrypt_buff, 0, sizeof(EncDecrypt_buff) );

	do {
		len += size;
		limit -= size;

		switch( _ref_buf_type ){
			case 'A':
				size = PKTbuf_ReadReSendCommand( num++, &EncDecrypt_buff[len] );
				break;
			default:	// 'S'
				size = PKTbuf_ReadSendCommand( &EncDecrypt_buff[len], limit );
				if( size ){
					PKTbuf_SetReSendCommand( &EncDecrypt_buff[len], size );
				}
				break;
		}
	} while(size);

	len -= 2;
	// リードデータあり
	if( len ){
		*(ushort *)&EncDecrypt_buff[0] = len;
		len += 2;

		memcpy( &p_trailer->part.identifier, "AMANO_LCDPKT", sizeof(p_trailer->part.identifier) );
		intoas( p_trailer->part.seqno, snd_seqno, sizeof(p_trailer->part.seqno) );
		p_trailer->part.id = 0x00;
		p_trailer->part.kind = 0x10;
		p_trailer->part.blkno_h = 0;
		p_trailer->part.blkno_l = 1;	// ブロック№は2byte(境界のためこのように･･･)
		p_trailer->part.block_sts = 1;
		// 暗号化あり？
		if( pktctrl.EncDecrypt ){
			p_trailer->part.encrypt = 1;
			// パディング必要？
			if( len % 16 ){
				p_trailer->part.pad_size = (16 - (len % 16));
			}
			len += p_trailer->part.pad_size;
			// 暗号化してデータコピー
			len = EncryptWithKeyAndIV_NoPadding( CRYPT_KEY_RXMLCDPK, EncDecrypt_buff, (uchar*)p_trailer->part.data, (int)len );
			if( !len ){	// 暗号化NG
				return 0;
			}
		}else{
			memcpy( p_trailer->part.data, EncDecrypt_buff, len );
		}

		// 電文長算出(暗号化が必要な場合、暗号化後に算出)
		p_trailer->part.data_size = ( (uchar *)&p_trailer->part.data[0] - (uchar *)&p_trailer->part.data_size );	// ヘッダサイズ
		p_trailer->part.data_size += len;	// 電文長(電文長自身を含む(暗号化時はパディングサイズも含む))
		p_trailer->part.data_size += 2;		// CRC16(2byte)

		// CRC16算出(little endian) ※little endianのまま格納
		crc_ccitt( (ushort)(p_trailer->part.data_size-2), (uchar *)&p_trailer->data[0], (uchar *)&crc16.us, (uchar)R_SHIFT );
		p_trailer->data[p_trailer->part.data_size-2] = crc16.uc.high;
		p_trailer->data[p_trailer->part.data_size-1] = crc16.uc.low;
	}

	return ((ushort)p_trailer->part.data_size);
}

//[]----------------------------------------------------------------------[]
///	@brief			ログデータ用AMANO_LCDPKTの生成
//[]----------------------------------------------------------------------[]
///	@param[in]		target		: 対象種別(1=RT精算/1=DC_QR)
///	@param[in]		snd_seqno	: 送信シーケンス№(0～999)
///	@param[out]		*p_trailer	: 生成したパケットデータ
///	@return			パケットのデータ長
///	@note			ログデータ(RT精算/DC_QR)からパケットデータを生成します。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/11/20<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ushort PKTsnd_Create_LogData_Packet( eSEND_TARGET target, ushort snd_seqno, PKT_TRAILER *p_trailer )
{
// MH810100(S) S.Nishimoto 2020/04/07 静的解析(20200407:24)対応
//	int	len;
	int	len = 0;
// MH810100(E) S.Nishimoto 2020/04/07 静的解析(20200407:24)対応
	union {
		ushort	us;
		struct {
			uchar	high;
			uchar	low;
		} uc;
	} crc16;

	memset( p_trailer, 0, sizeof(PKT_TRAILER) );

	memcpy( &p_trailer->part.identifier, "AMANO_LCDPKT", sizeof(p_trailer->part.identifier) );
	intoas( p_trailer->part.seqno, snd_seqno, sizeof(p_trailer->part.seqno) );
	p_trailer->part.id = 0x00;			// データ部コード(00H固定)
	p_trailer->part.kind = 0x10;		// 電文識別コード(10H=パケットトレイラ(固定))
	p_trailer->part.blkno_h = 0;		// ブロックNo.(上位)
	p_trailer->part.blkno_l = 1;		// ブロックNo.(下位)(1固定)
	p_trailer->part.block_sts = 1;		// ブロックステータス(1固定)

	// 暗号化前のコマンド作成
	switch( target ){
		case TARGET_RT_PAY:
			memset( &log_pay_wk, 0, sizeof(lcdbm_cmd_pydt_0pd_t) );
			log_pay_wk.data_len = sizeof(lcdbm_cmd_pay_data_t);
			log_pay_wk.pay_dat.command.length = (ushort)(sizeof(lcdbm_cmd_pay_data_t) - 2);	// コマンド長(2Byte)分差し引く
			log_pay_wk.pay_dat.command.id = LCDBM_CMD_ID_PAY_INFO_NOTICE;
			log_pay_wk.pay_dat.command.subid = LCDBM_CMD_SUBID_PAY_DATA;
			Ope_Log_TargetDataVoidRead( eLOG_RTPAY, &log_pay_wk.pay_dat.id, eLOG_TARGET_LCD, TRUE );
			Pay_Data_ID = log_pay_wk.pay_dat.id;	// 応答受信時のチェック用にIDを保存

			// 暗号化あり？
			if( pktctrl.EncDecrypt ){
				// 暗号化してデータコピー
				len = (ushort)(2 + log_pay_wk.data_len + PADSIZE);	// 電文長(2Byte)＋RT精算データ長＋パディングサイズ
				// 入力:log_pay_wk/出力:p_trailer->part.data
				len = EncryptWithKeyAndIV_NoPadding(CRYPT_KEY_RXMLCDPK, (uchar*)&log_pay_wk, (uchar*)p_trailer->part.data, (int)len);
				if( !len ){	// 暗号化NG
					Ope_Log_UndoTargetDataVoidRead( eLOG_RTPAY, eLOG_TARGET_LCD );	// 空読みを元に戻す
					return 0;
				}
				p_trailer->part.encrypt = 1;	// 暗号化方式
				p_trailer->part.pad_size = PADSIZE;
			}
			// 暗号化なし？
			else{
				len = (ushort)(2 + log_pay_wk.data_len);
				memcpy( p_trailer->part.data, &log_pay_wk, len );
			}
			break;

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
		case TARGET_RT_RECEIPT:
			memset( &log_receipt_wk, 0, sizeof(lcdbm_cmd_receiptdt_0pd_t) );
			log_receipt_wk.data_len = sizeof(lcdbm_cmd_receipt_data_t);
			log_receipt_wk.receipt_dat.command.length = (ushort)(sizeof(lcdbm_cmd_receipt_data_t) - 2);	// コマンド長(2Byte)分差し引く
			log_receipt_wk.receipt_dat.command.id = LCDBM_CMD_ID_PAY_INFO_NOTICE;
			log_receipt_wk.receipt_dat.command.subid = LCDBM_CMD_SUBID_RECEIPT_DATA;
			Ope_Log_TargetDataVoidRead( eLOG_RTRECEIPT, &log_receipt_wk.receipt_dat.id, eLOG_TARGET_LCD, TRUE );
			Receipt_Data_ID = log_receipt_wk.receipt_dat.id;	// 応答受信時のチェック用にIDを保存

			// 暗号化あり？
			if ( pktctrl.EncDecrypt ) {
				// 暗号化してデータコピー
				len = (ushort)(2 + log_receipt_wk.data_len + RECEIPT_PADSIZE);	// 電文長(2Byte)＋RT領収証データ長＋パディングサイズ
				// 入力:log_receipt_wk/出力:p_trailer->part.data
				len = EncryptWithKeyAndIV_NoPadding(CRYPT_KEY_RXMLCDPK, (uchar*)&log_receipt_wk, (uchar*)p_trailer->part.data, (int)len);
				if ( !len ) {	// 暗号化NG
					Ope_Log_UndoTargetDataVoidRead( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// 空読みを元に戻す
					return 0;
				}
				p_trailer->part.encrypt = 1;	// 暗号化方式
				p_trailer->part.pad_size = RECEIPT_PADSIZE;
			}
			// 暗号化なし？
			else {
				len = (ushort)(2 + log_receipt_wk.data_len);
				memcpy(p_trailer->part.data, &log_receipt_wk, len);
			}
			break;
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
		case TARGET_DC_QR:
			memset( &log_qr_wk, 0, sizeof(lcdbm_cmd_QR_conf_can_t) );
			log_qr_wk.data_len = sizeof(lcdbm_cmd_QR_conf_can_t);
			log_qr_wk.qr_dat.command.length = (ushort)(sizeof(lcdbm_cmd_QR_conf_can_t) - 2);	// コマンド長(2Byte)分差し引く
			log_qr_wk.qr_dat.command.id = LCDBM_CMD_ID_PAY_INFO_NOTICE;
			log_qr_wk.qr_dat.command.subid = LCDBM_CMD_SUBID_QR_CONF_CAN;
			Ope_Log_TargetDataVoidRead( eLOG_DC_QR, &log_qr_wk.qr_dat.id, eLOG_TARGET_LCD, TRUE );
			QR_Data_ID = log_qr_wk.qr_dat.id;	// 応答受信時のチェック用にIDを保存

			// 暗号化あり？
			if( pktctrl.EncDecrypt ){
				// 暗号化してデータコピー
				len = (ushort)(2 + log_qr_wk.data_len + QR_PADSIZE);	// 電文長(2Byte)＋RT精算データ長＋パディングサイズ
				// 入力:log_qr_wk/出力:p_trailer->part.data
				len = EncryptWithKeyAndIV_NoPadding(CRYPT_KEY_RXMLCDPK, (uchar*)&log_qr_wk, (uchar*)p_trailer->part.data, (int)len);
				if( !len ){	// 暗号化NG
					Ope_Log_UndoTargetDataVoidRead( eLOG_DC_QR, eLOG_TARGET_LCD );	// 空読みを元に戻す
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 5 );
#endif
// 調査用 (e)
					return 0;
				}
				p_trailer->part.encrypt = 1;	// 暗号化方式
				p_trailer->part.pad_size = QR_PADSIZE;
			}
			// 暗号化なし？
			else{
				len = (ushort)(2 + log_qr_wk.data_len);
				memcpy( p_trailer->part.data, &log_qr_wk, len );
			}
			break;
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
		case TARGET_DC_LANE:
			memset( &log_lane_wk, 0, sizeof(lcdbm_cmd_lane_t) );
			log_lane_wk.data_len = sizeof(lcdbm_cmd_lane_t);
			log_lane_wk.lane_dat.command.length = (ushort)(sizeof(lcdbm_cmd_lane_t) - 2);	// コマンド長(2Byte)分差し引く
			log_lane_wk.lane_dat.command.id = LCDBM_CMD_ID_PAY_INFO_NOTICE;
			log_lane_wk.lane_dat.command.subid = LCDBM_CMD_SUBID_LANE;
			Ope_Log_TargetDataVoidRead( eLOG_DC_LANE, &log_lane_wk.lane_dat.id, eLOG_TARGET_LCD, TRUE );
			LANE_Data_ID = log_lane_wk.lane_dat.id;	// 応答受信時のチェック用にIDを保存

			// 暗号化あり？
			if( pktctrl.EncDecrypt ){
				// 暗号化してデータコピー
				len = (ushort)(2 + log_lane_wk.data_len + LANE_PADSIZE);	// 電文長(2Byte)＋RT精算データ長＋パディングサイズ
				// 入力:log_lane_wk/出力:p_trailer->part.data
				len = EncryptWithKeyAndIV_NoPadding(CRYPT_KEY_RXMLCDPK, (uchar*)&log_lane_wk, (uchar*)p_trailer->part.data, (int)len);
				if( !len ){	// 暗号化NG
					Ope_Log_UndoTargetDataVoidRead( eLOG_DC_LANE, eLOG_TARGET_LCD );	// 空読みを元に戻す
					return 0;
				}
				p_trailer->part.encrypt = 1;	// 暗号化方式
				p_trailer->part.pad_size = LANE_PADSIZE;
			}
			// 暗号化なし？
			else{
				len = (ushort)(2 + log_lane_wk.data_len);
				memcpy( p_trailer->part.data, &log_lane_wk, len );
			}
			break;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

		default:
			break;
	}

	// パケットデータ長＝パケットデータ長～CRC16(データ部は暗号化後の長さ)
	p_trailer->part.data_size = 26;		// ヘッダ部サイズ
	p_trailer->part.data_size += len;	// 暗号化後のデータ部サイズ加算
	p_trailer->part.data_size += 2;		// CRC16(2byte)サイズ加算

	// CRC16算出(little endian) ※little endianのまま格納
	crc_ccitt( (ushort)(p_trailer->part.data_size-2), (uchar *)&p_trailer->data[0], (uchar *)&crc16.us, (uchar)R_SHIFT );
	p_trailer->data[p_trailer->part.data_size-2] = crc16.uc.high;
	p_trailer->data[p_trailer->part.data_size-1] = crc16.uc.low;

	return ((ushort)p_trailer->part.data_size);
}

//[]----------------------------------------------------------------------[]
///	@brief			LCD通信制御ﾒｲﾝ処理
//[]----------------------------------------------------------------------[]
///	@param			なし
///	@return			なし
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2019/10/31<br>
///					Update	: ----/--/--
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	PktTask( void )
{
	ushort	data_size;
	MsgBuf	*msb;		// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀ
	MsgBuf	msg;		// 受信ﾒｯｾｰｼﾞ格納ﾊﾞｯﾌｧ
	uchar	rst_cause;
	char	err_code;

	// 初期化
	pkt_init();

	// 設定取得(IP/ポート)
	GetSetting();

	// 接続コマンド開始タイマを起動
	Lagtim( PKTTCBNO, PKT_TIM_CONNECT, 3*50 );	// 3秒

	// 接続エラーガードタイマを起動
	Lagtim( PKTTCBNO, PKT_TIM_ERR_GUARD, pktctrl.err_guard_tm*50 );

	// ================ //
	// メインループ
	// ================ //
	while( 1 ){
		taskchg( IDLETSKNO );

		if( ( msb = GetMsg( PKTTCBNO ) ) == NULL ){
			continue;		// 受信ﾒｯｾｰｼﾞなし→ continue（ｱｲﾄﾞﾙﾀｽｸ切替）
		}

		// 受信メッセージあり
		memcpy( &msg ,msb ,sizeof(MsgBuf) );				// 処理ﾒｯｾｰｼﾞ←受信ﾒｯｾｰｼﾞ
		FreeBuf( msb );										// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ開放

		// ------------------ //
		// イベント毎処理
		// ------------------ //
		switch( msg.msg.command ){

			// 接続完了(非同期)
			case PKT_SOC_CON:
				// コネクション応答待ちタイマを停止
				Lagcan( PKTTCBNO, PKT_TIM_CON_RES_WT );
				// 接続エラーガードタイマを停止
				Lagcan( PKTTCBNO, PKT_TIM_ERR_GUARD );
				// まずは基本設定要求を送信する
				PKTcmd_request_config();
				// 無効であっても送信処理を行うために1秒タイマ開始する
				Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, 50 );
				// キープアライブタイマカウンタリセット
				RESET_KEEPALIVE_COUNT;
				// キープアライブリトライカウンタリセット
				lcdbm_reset_keepalive_status();
				// 通信状態を更新
				_ref_sckt = PKT_PORT_ESTABLISH;
				// パケット応答待ち時間タイマ停止
				Lagcan( PKTTCBNO, PKT_TIM_RES_WT );
				_ref_send.res_wait = 0;
				// 通信不良関連エラー全解除
				pkt_can_all_com_err();
				break; // そのまま送信処理にするためにbreak

			// 接続コマンド開始タイマタイムアウト
			case TIMEOUT_CONNECT:
				if( PktStartUpStsGet() < 1 ){	// OPEタスク初期化未完了
					// 接続コマンド開始タイマを再起動(OPEタスク初期化完了まで待ち合わせる)
					Lagtim( PKTTCBNO, PKT_TIM_CONNECT, 50 );	// 1秒
					continue;
				}
				// no break
			// コネクション送信間隔タイマタイムアウト
			case TIMEOUT_CON_INTVL:
				// 既に接続中
				if( _ref_sckt == PKT_PORT_CONNECTING ){
					;
				}
				else {
					if( PKT_Socket != -1 ){
						// ソケットを閉じる
						if( -1 == tfClose( PKT_Socket ) ){
							pkt_ksg_err_getreg( PKT_Socket, ERR_TKLSLCD_KSG_CLS, 2 );
						}
						PKT_Socket = -1;
					}
					// ソケット設定
					PKT_Socket = PktSocCfg();
					// ソケット設定OK？
					if( PKT_Socket != -1 ){
						// コネクション接続要求OK
						if( -1 != pkt_send_req_con() ){
							_ref_sckt = PKT_PORT_CONNECTING;			// 接続中
							// コネクション応答待ちタイマを起動
							Lagtim( PKTTCBNO, PKT_TIM_CON_RES_WT, (ushort)pktctrl.con_res_tm*50 );
						}
						// コネクション接続要求NG
						else{
							// コネクション送信間隔タイマを起動
							Lagtim( PKTTCBNO, PKT_TIM_CON_INTVL, (ushort)pktctrl.con_snd_intvl*50 );
							err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_COMM_FAIL, 1, 0, 0, NULL );
							queset( OPETCBNO, LCD_COMMUNICATION_CUTTING, 0, NULL );
						}
					}
					// ソケット設定NG？
					else{
						// コネクション送信間隔タイマを起動
						Lagtim( PKTTCBNO, PKT_TIM_CON_INTVL, (ushort)pktctrl.con_snd_intvl*50 );
						err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_COMM_FAIL, 1, 0, 0, NULL );
						queset( OPETCBNO, LCD_COMMUNICATION_CUTTING, 0, NULL );
					}
				}
				continue;
				break;

			// キープアライブタイムアウト
			// 1秒毎にチェックを行い、キープアライブ送信間隔(設定№48-0019)に到達したら基本設定要求送信
			// キープアライブ送信間隔未到達或いはキープアライブ無効(設定№48-0019=0)時は送信処理を行う
			// 但し1秒毎のチェック時にリセット要求があった場合はリセット要求を優先して行う
			case TIMEOUT_KEEPALIVE:
				if( PktResetReqFlgRead() ){	// リセット要求あり
					PKTcmd_notice_ope( LCDBM_OPCD_RST_REQ, 0 );
					Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, 50 );
					RESET_KEEPALIVE_COUNT;
					// キープアライブリトライカウンタリセット
					lcdbm_reset_keepalive_status();
					break;	// 送信処理へ
				}
				// キープアライブ有効？
				if( pktctrl.kpalv_snd_intvl ){
					// キープアライブ実行？
				if( ++pktctrl.keepalive_timer >= pktctrl.kpalv_snd_intvl ){
						// リトライチェックOK？
						if( lcdbm_check_keepalive_status( pktctrl.kpalv_rtry_cnt ) ){
							// 基本設定要求送信要求
							PKTcmd_request_config();
							Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, 50 );
							RESET_KEEPALIVE_COUNT;
						}
						// リトライオーバー？
						else{
							// キープアライブタイマーを停止
							Lagcan( PKTTCBNO, PKT_TIM_KEEPALIVE );
							// キープアライブリトライカウンタリセット
							lcdbm_reset_keepalive_status();
							// 再送バファクリア
							PKTbuf_ClearReSendCommand();
							rst_cause = ERR_TKLSLCD_KPALV_RTY_OV;
							queset( PKTTCBNO, PKT_RST_PRT, 1, &rst_cause );
						}
						continue;
					}
				}
				// 無効であっても送信処理を行うために1秒タイマ再開する
				Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, 50 );
				break;	// 送信処理へ

			// ポートリセット要求
			case PKT_RST_PRT:
				_ref_recv.flag = 0;
				if( PktStartUpStsGet() >= 2 ){	// 起動完了通知受信済み(OPEタスク初期化完了/RT精算データ送信可)
					PktStartUpStsChg( 1 );	// 起動通知受信可(OPEタスク初期化完了/RT精算データ送信不可)
				}
				// no break;
			// コネクション応答待ちタイマタイムアウト
			case TIMEOUT_CON_RES_WT:
				if( msg.msg.command == PKT_RST_PRT ){
					err_code = (char)msg.msg.data[0];
				}else if( msg.msg.command == TIMEOUT_CON_RES_WT ){
					err_code = (char)ERR_TKLSLCD_CON_RES_TO;
				}
				if(( msg.msg.command == PKT_RST_PRT ) &&		// ポートリセット要求
				   (( err_code == ERR_TKLSLCD_RMT_RST ) ||		// 相手からのReset
				    ( err_code == ERR_TKLSLCD_RMT_DSC )) &&		// 相手からの切断
				    ( pktctrl.f_wait_discon )){					// リセット通知受信後
					// リセット通知受信後のコネクション切断はエラー登録しない
					Lagcan( PKTTCBNO, PKT_TIM_DISCON_WAIT );	// コネクション切断検出待ちタイマ停止
					pktctrl.f_wait_discon = 0;					// コネクション切断検出待ちタイマ起動中フラグOFF
				}else{
					if( !LagChkExe( PKTTCBNO, PKT_TIM_ERR_GUARD ) ){	// 接続エラーガードタイマ停止中？
						// 接続エラーガードタイマ動作中はエラー登録しない
						if(( err_code != ERR_TKLSLCD_KSG_SND ) &&	// 送受信エラーは検出時に登録済み
						   ( err_code != ERR_TKLSLCD_KSG_RCV )){
							err_chk2( ERRMDL_TKLSLCD, err_code, 1, 0, 0, NULL );
// MH810105(S) R.Endo 2021/12/13 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
#if DEBUG_JOURNAL_PRINT
							if ( err_code == ERR_TKLSLCD_RMT_DSC ) {
								// E2603発生時点のデバッグ印字
								pkt_socket_debug_print();
							}
#endif
// MH810105(E) R.Endo 2021/12/13 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
						}
						err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_COMM_FAIL, 1, 0, 0, NULL );
					}
				}
				// OPEに切断を通知
				queset( OPETCBNO, LCD_COMMUNICATION_CUTTING, 0, NULL );
				// キープアライブタイマーを停止
				Lagcan( PKTTCBNO, PKT_TIM_KEEPALIVE );
				if( _ref_send.res_wait ){	// パケット応答待ち
					// パケット応答待ち時間タイマ停止
					Lagcan( PKTTCBNO, PKT_TIM_RES_WT );
					_ref_send.res_wait = 0;
				}
				if( pktctrl.send.RT_LogSend ){	// ログデータ処理中
					Ope_Log_UndoTargetDataVoidRead( eLOG_RTPAY, eLOG_TARGET_LCD );	// 空読みを元に戻す
					//  ログ応答データ待ち
					if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_RES_CMD ){
						Lagcan( PKTTCBNO, PKT_TIM_RTPAY_RES );	//  RT精算応答データ待ちタイマ停止
					}
					pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信状態に戻す
				}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
				if ( pktctrl.send.RECEIPT_LogSend ) {	// ログデータ処理中
					Ope_Log_UndoTargetDataVoidRead( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// 空読みを元に戻す
					//  ログ応答データ待ち
					if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_RES_CMD ) {
						Lagcan( PKTTCBNO, PKT_TIM_RTRECEIPT_RES );	// RT領収証データ応答待ちタイマ停止
					}
					pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信状態に戻す
				}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
				if( pktctrl.send.QR_LogSend ){	// ログデータ処理中
					Ope_Log_UndoTargetDataVoidRead( eLOG_DC_QR, eLOG_TARGET_LCD );	// 空読みを元に戻す
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 6 );
#endif
// 調査用 (e)
					//  ログ応答データ待ち
					if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_RES_CMD ){
						Lagcan( PKTTCBNO, PKT_TIM_DC_QR_RES );	//  QR確定・取消応答データ待ちタイマ停止
					}
					pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信状態に戻す
				}
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
				if( pktctrl.send.LANE_LogSend ){	// ログデータ処理中
					Ope_Log_UndoTargetDataVoidRead( eLOG_DC_LANE, eLOG_TARGET_LCD );	// 空読みを元に戻す
					//  ログ応答データ待ち
					if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_RES_CMD ){
						Lagcan( PKTTCBNO, PKT_TIM_DC_LANE_RES );	//  QR確定・取消応答データ待ちタイマ停止
					}
					pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信状態に戻す
				}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
				if( LagChkExe(PKTTCBNO, PKT_TIM_RTPAY_RESND) ){	//  再送ウェイトタイマ動作中
					// 再送ウェイトタイマ動作中は「pktctrl.send.LogSend = 0」かつ「ログ＝未送信」
					Lagcan( PKTTCBNO, PKT_TIM_RTPAY_RESND );	//  再送ウェイトタイマ停止
				}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
				if ( LagChkExe(PKTTCBNO, PKT_TIM_RTRECEIPT_RESND) ) {	//  再送ウェイトタイマ動作中
					// 再送ウェイトタイマ動作中は「pktctrl.send.LogSend = 0」かつ「ログ＝未送信」
					Lagcan(PKTTCBNO, PKT_TIM_RTRECEIPT_RESND);	//  再送ウェイトタイマ停止
				}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
				if( LagChkExe(PKTTCBNO, PKT_TIM_DC_QR_RESND) ){	//  再送ウェイトタイマ動作中
					// 再送ウェイトタイマ動作中は「pktctrl.send.LogSend = 0」かつ「ログ＝未送信」
					Lagcan( PKTTCBNO, PKT_TIM_DC_QR_RESND );	//  再送ウェイトタイマ停止
				}
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
				if( LagChkExe(PKTTCBNO, PKT_TIM_DC_LANE_RESND) ){	//  再送ウェイトタイマ動作中
					// 再送ウェイトタイマ動作中は「pktctrl.send.LogSend = 0」かつ「ログ＝未送信」
					Lagcan( PKTTCBNO, PKT_TIM_DC_LANE_RESND );	//  再送ウェイトタイマ停止
				}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
				if( PKT_Socket != -1 ){
					// ソケットを閉じる
					if( -1 == tfClose( PKT_Socket ) ){
						pkt_ksg_err_getreg( PKT_Socket, ERR_TKLSLCD_KSG_CLS, 2 );
					}
					PKT_Socket = -1;
				}
				// コネクション送信間隔タイマを起動
				Lagtim( PKTTCBNO, PKT_TIM_CON_INTVL, (ushort)pktctrl.con_snd_intvl*50 );
				_ref_sckt = PKT_PORT_DISCONNECT;
				continue;
				break;

			// コマンド送信要求
			case PKT_SND_PRT:
				if (_ref_send.commited == _ref_send.sent) {
					// データ送信が終わられるとき、次の送信を実行
					_ref_send.commited += 1;
					if( _ref_send.commited > 9999 ){
						_ref_send.commited = 0;
					}
				}
				break;	// 送信処理へ

			// 非同期受信イベント
			case PKT_SOC_RCV:
				if (0 == pktctrl.ready) {	// not readyなら保存せずにOKを返す（読み捨て）
					// while()に入る前にpkt_init()でセットしているので通らないはず
					break;	// 送信処理へ
				}
				PktTcpRecv( PKT_Socket );
				break;	// 送信処理へ

			// パケット応答待ち時間タイムアウト
			case TIMEOUT_RES_WT:
				if( _ref_send.res_wait ){	// 応答待ち？
					_ref_send.res_wait = 0;
					if( ++_ref_send.rtry_cnt > pktctrl.snd_rtry_cnt ){	// 送信リトライオーバー
						_ref_send.rtry_cnt = 0;	// リトライカウンタクリア
						// 送信したのはRTログデータ？
						if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_ACK ){
							// 再送を中断してRT精算データ以外の送信を先に行う
							PKTbuf_ClearReSendCommand();	// 再送バッファクリア
							Ope_Log_UndoTargetDataVoidRead( eLOG_RTPAY, eLOG_TARGET_LCD );	// 空読みを元に戻す
							pktctrl.send.RT_LogSend = LOG_PHASE_RESEND_CANCEL;	// ログデータ再送中断
							break;	// 送信処理へ
						}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
						// 送信したのはRT領収証ログデータ？
						else if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_ACK ) {
							// 再送を中断してRT領収証データ以外の送信を先に行う
							PKTbuf_ClearReSendCommand();	// 再送バッファクリア
							Ope_Log_UndoTargetDataVoidRead(eLOG_RTRECEIPT, eLOG_TARGET_LCD);	// 空読みを元に戻す
							pktctrl.send.RECEIPT_LogSend = LOG_PHASE_RESEND_CANCEL;	// ログデータ再送中断
							break;	// 送信処理へ
						}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
						// 送信したのはQRログデータ？
						else if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_ACK ){
							// 再送を中断してRT精算データ以外の送信を先に行う
							PKTbuf_ClearReSendCommand();	// 再送バッファクリア
							Ope_Log_UndoTargetDataVoidRead( eLOG_DC_QR, eLOG_TARGET_LCD );	// 空読みを元に戻す
							pktctrl.send.QR_LogSend = LOG_PHASE_RESEND_CANCEL;	// ログデータ再送中断
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 7 );
#endif
// 調査用 (e)
							break;	// 送信処理へ
						}
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
						// 送信したのはレーンモニタログデータ？
						else if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_ACK ){
							// 再送を中断してRT精算データ以外の送信を先に行う
							PKTbuf_ClearReSendCommand();	// 再送バッファクリア
							Ope_Log_UndoTargetDataVoidRead( eLOG_DC_LANE, eLOG_TARGET_LCD );	// 空読みを元に戻す
							pktctrl.send.LANE_LogSend = LOG_PHASE_RESEND_CANCEL;	// ログデータ再送中断
							break;	// 送信処理へ
						}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
						// 送信したのは通常データ？
						else{
							if( ( pktctrl.rsnd_rtry_ov ) &&	// リトライオーバー時の再送信する
								( PKTbuf_CheckReSendCommand() != LCDBM_CMD_NOTIFY_CONFIG )){	// 基本設定要求でない
								;	// 再送信(基本設定要求はリトライオーバー時の再送信対象外とする)
							}else{
								err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_SND_RTRY_OV, 2, 0, 0, NULL );
								_ref_send.sent += 1;	// 送信済みとする
								PKTbuf_ClearReSendCommand();	// 再送バッファクリア
								// RT精算データ再送中断中？
								if( pktctrl.send.RT_LogSend == LOG_PHASE_RESEND_CANCEL ){
									// RT精算データ再送中断を解除してRT精算データ再送開始
									pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;
									_ref_buf_type = 'S';
								}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
								// RT領収証データ再送中断中？
								else if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_RESEND_CANCEL ) {
									// RT領収証データ再送中断を解除してRT領収証データ再送開始
									pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;
									_ref_buf_type = 'S';
								}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
								// QR確定・取消データ再送中断中？
								else if( pktctrl.send.QR_LogSend == LOG_PHASE_RESEND_CANCEL ){
									// QR確定取消データ再送中断を解除してQR確定・取消データ再送開始
									pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;
									_ref_buf_type = 'S';
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 21 );
#endif
// 調査用 (e)
								}
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
								// レーンモニタデータ再送中断中？
								else if( pktctrl.send.LANE_LogSend == LOG_PHASE_RESEND_CANCEL ){
									// QR確定取消データ再送中断を解除してレーンモニタデータ再送開始
									pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;
									_ref_buf_type = 'S';
								}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
								break;	// 送信処理へ
							}
						}
					}
					// 再送(ここに来るのはリトライオーバ前とリトライオーバ時も再送する場合)

					// 送信したのはRT精算データ？
					if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_ACK ){
						Ope_Log_UndoTargetDataVoidRead( eLOG_RTPAY, eLOG_TARGET_LCD );	// 空読みを元に戻す
						pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// RT精算データ未送信状態に戻す
						_ref_buf_type = 'S';
					}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
					// 送信したのはRT領収証データ？
					else if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_ACK ) {
						Ope_Log_UndoTargetDataVoidRead( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// 空読みを元に戻す
						pktctrl.send.RECEIPT_LogSend = LOG_PHASE_IDLE;	// RT領収証データ未送信状態に戻す
						_ref_buf_type = 'S';
					}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
					// 送信したのはQR確定・取消データ？
					else if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_ACK ){
						Ope_Log_UndoTargetDataVoidRead( eLOG_DC_QR, eLOG_TARGET_LCD );	// 空読みを元に戻す
						pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// QR確定・取消データ未送信状態に戻す
						_ref_buf_type = 'S';
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 1 );
#endif
// 調査用 (e)
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
					}
					// 送信したのはレーンモニタデータ？
					else if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_ACK ){
						Ope_Log_UndoTargetDataVoidRead( eLOG_DC_LANE, eLOG_TARGET_LCD );	// 空読みを元に戻す
						pktctrl.send.LANE_LogSend = LOG_PHASE_IDLE;	// レーンモニタデータ未送信状態に戻す
						_ref_buf_type = 'S';
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
					}else{
						_ref_buf_type = 'A';
					}
					break;	// 送信処理へ
				}
				// 応答待ちでなければ戻る
				continue;
				break;

			// 精算応答データ待ちタイマタイムアウト
			case TIMEOUT_RTPAY_RES:
				// ログ応答データ待ち中？
				if( pktctrl.send.RT_LogSend == LOG_PHASE_WAIT_RES_CMD ){
// MH810100(S) K.Onodera  2020/04/27 #4135 車番チケットレス(DC-NET通信復旧時に複数の割引認証登録・取消要求データを送信してしまう)
//					err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_PAY_RES_TO, 2, 0, 0, NULL );	// RT精算応答データ待ちエラー(発生解除同時)
//					Ope_Log_UndoTargetDataVoidRead( eLOG_RTPAY, eLOG_TARGET_LCD );		// 空読みを元に戻す
//					pktctrl.send.RT_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信状態に戻す
//					// 精算応答データ待ち中は他データ送信中の可能性もあるので
//					// _ref_buf_typeは変更しない
					Lagtim( PKTTCBNO, PKT_TIM_RTPAY_RES, (ushort)(pktctrl.RTPay_res_tm*50) );	// パケット応答データ待ちタイマ開始
// MH810100(E) K.Onodera  2020/04/27 #4135 車番チケットレス(DC-NET通信復旧時に複数の割引認証登録・取消要求データを送信してしまう)
					break;	// 送信処理へ
				}
				// 応答待ちでなければ戻る
				continue;
				break;

// GG129004(S) R.Endo 2024/11/19 電子領収証対応
			// 領収証データ応答待ちタイマタイムアウト
			case TIMEOUT_RTRECEIPT_RES:
				// ログ応答データ待ち中？
				if ( pktctrl.send.RECEIPT_LogSend == LOG_PHASE_WAIT_RES_CMD ) {
					Lagtim(PKTTCBNO, PKT_TIM_RTRECEIPT_RES, (ushort)(pktctrl.RTPay_res_tm*50) );	// パケット応答データ待ちタイマ開始
					break;	// 送信処理へ
				}
				// 応答待ちでなければ戻る
				continue;
				break;
// GG129004(E) R.Endo 2024/11/19 電子領収証対応

			// QR確定取消応答データ待ちタイマタイムアウト
			case TIMEOUT_DC_QR_RES:
				// ログ応答データ待ち中？
				if( pktctrl.send.QR_LogSend == LOG_PHASE_WAIT_RES_CMD ){
// MH810100(S) K.Onodera  2020/04/27 #4135 車番チケットレス(DC-NET通信復旧時に複数の割引認証登録・取消要求データを送信してしまう)
//					err_chk2( ERRMDL_TKLSLCD, ERR_TKLSLCD_QR_RES_TO, 2, 0, 0, NULL );	// QR確定取消応答データ待ちエラー(発生解除同時)
//					Ope_Log_UndoTargetDataVoidRead( eLOG_DC_QR, eLOG_TARGET_LCD );		// 空読みを元に戻す
//					pktctrl.send.QR_LogSend = LOG_PHASE_IDLE;	// ログデータ未送信状態に戻す
//					// 精算応答データ待ち中は他データ送信中の可能性もあるので
//					// _ref_buf_typeは変更しない
					Lagtim( PKTTCBNO, PKT_TIM_DC_QR_RES, (ushort)(pktctrl.RTPay_res_tm*50) );	// パケット応答データ待ちタイマ開始
// MH810100(E) K.Onodera  2020/04/27 #4135 車番チケットレス(DC-NET通信復旧時に複数の割引認証登録・取消要求データを送信してしまう)
					break;	// 送信処理へ
				}
				// 応答待ちでなければ戻る
				continue;
				break;

// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
			// レーンモニタ応答データ待ちタイマタイムアウト
			case TIMEOUT_DC_LANE_RES:
				// ログ応答データ待ち中？
				if( pktctrl.send.LANE_LogSend == LOG_PHASE_WAIT_RES_CMD ){
					Lagtim( PKTTCBNO, PKT_TIM_DC_LANE_RES, (ushort)(pktctrl.RTPay_res_tm*50) );	// パケット応答データ待ちタイマ開始
					break;	// 送信処理へ
				}
				// 応答待ちでなければ戻る
				continue;
				break;
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）

			// 再送ウェイトタイマタイムアウト
			case TIMEOUT_RTPAY_RESND:
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
			case TIMEOUT_RTRECEIPT_RESND:
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
			case TIMEOUT_DC_QR_RESND:
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
			case TIMEOUT_DC_LANE_RESND:
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
				;	// 空読み・ログデータ未送信状態戻し済み
				break;	// 送信処理へ

			// コネクション切断検出待ちタイマタイムアウト
			case TIMEOUT_DISCON_WT:
				pktctrl.f_wait_discon = 0;	// コネクション切断検出待ちタイマ起動中フラグOFF
				break;	// 送信処理へ

			// 接続エラーガードタイマタイムアウト
			case TIMEOUT_ERR_GUARD:
				break;	// 送信処理へ

			default:
				continue;
				break;

		}

		// -------------------- //
		// パケット送信処理
		// -------------------- //

		// 送信可能状態？
		if( (_ref_sckt == PKT_PORT_ESTABLISH) &&
		    (_ref_send.res_wait == 0)	// ACK/NAK待ちじゃない？
		){
			taskchg( IDLETSKNO );
			// ***************
			// 送信データ生成
			// ***************

			// ログデータの送信
			if( ( PktStartUpStsGet() >= 2 )	&&	// 起動完了通知受信済み(OPEタスク初期化完了/ログデータ送信可)
				( _ref_buf_type != 'A' )	){	// パケット再送でない

			   // RT精算データ未送信あり？
				if( Ope_Log_UnreadCountGet( eLOG_RTPAY, eLOG_TARGET_LCD ) && ( pktctrl.send.RT_LogSend == LOG_PHASE_IDLE ) && !LagChkExe(PKTTCBNO, PKT_TIM_RTPAY_RESND) ){
					data_size = PKTsnd_Create_LogData_Packet( TARGET_RT_PAY, pktctrl.snd_seqno, &snd_trailer );

					// パケット生成OK？
					if( data_size ){
						// ***************
						//    送信要求
						// ***************
						pkt_send_req_snd( data_size, (uchar *)&snd_trailer, snd_trailer.part.block_sts );
						// ログデータの応答待ちタイマはACK受信で開始
						pktctrl.send.RT_LogSend = LOG_PHASE_WAIT_ACK;

						// 送信してからインクリメントする
						pktctrl.snd_seqno++;
						if( pktctrl.snd_seqno > 999 ){
							pktctrl.snd_seqno = 1;
						}
						if( pktctrl.kpalv_snd_intvl ){	// キープアライブ有効
							// キープアライブタイマリセット
							Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, (ushort)(pktctrl.kpalv_snd_intvl*50) );
							RESET_KEEPALIVE_COUNT;
						}else{	// キープアライブ無効(以下、保険)
							Lagcan( PKTTCBNO, PKT_TIM_KEEPALIVE );
						}
					}
					// パケット生成NG？
					else{
						// パケット生成NGのログは送信済みとする(このルートは通らないはず)
						Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTPAY, eLOG_TARGET_LCD );	// 送信済みとする
						Pay_Data_ID = 0;
						QR_Data_ID = 0;
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
						Receipt_Data_ID = 0;
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
					}
					continue;	// RT精算データ送信を優先させる為、パケット生成NG時もcontinue
				}
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
				// RT領収証データ未送信あり？
				else if ( Ope_Log_UnreadCountGet( eLOG_RTRECEIPT, eLOG_TARGET_LCD ) && (pktctrl.send.RECEIPT_LogSend == LOG_PHASE_IDLE) && !LagChkExe(PKTTCBNO, PKT_TIM_RTRECEIPT_RESND) ) {
					data_size = PKTsnd_Create_LogData_Packet( TARGET_RT_RECEIPT, pktctrl.snd_seqno, &snd_trailer );

					// パケット生成OK？
					if ( data_size ) {
						// ***************
						//    送信要求
						// ***************
						pkt_send_req_snd( data_size, (uchar *)&snd_trailer, snd_trailer.part.block_sts );
						// ログデータの応答待ちタイマはACK受信で開始
						pktctrl.send.RECEIPT_LogSend = LOG_PHASE_WAIT_ACK;

						// 送信してからインクリメントする
						pktctrl.snd_seqno++;
						if ( pktctrl.snd_seqno > 999 ) {
							pktctrl.snd_seqno = 1;
						}
						if ( pktctrl.kpalv_snd_intvl ) {	// キープアライブ有効
							// キープアライブタイマリセット
							Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, (ushort)(pktctrl.kpalv_snd_intvl*50) );
							RESET_KEEPALIVE_COUNT;
						} else {	// キープアライブ無効(以下、保険)
							Lagcan( PKTTCBNO, PKT_TIM_KEEPALIVE );
						}
					}
					// パケット生成NG？
					else {
						// パケット生成NGのログは送信済みとする(このルートは通らないはず)
						Ope_Log_TargetVoidReadPointerUpdate( eLOG_RTRECEIPT, eLOG_TARGET_LCD );	// 送信済みとする
						Pay_Data_ID = 0;
						QR_Data_ID = 0;
						Receipt_Data_ID = 0;
					}
					continue;	// RT領収証データ送信を優先させる為、パケット生成NG時もcontinue
				}
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
				// QR確定取消データ未送信あり？
				else if( Ope_Log_UnreadCountGet( eLOG_DC_QR, eLOG_TARGET_LCD ) && ( pktctrl.send.QR_LogSend == LOG_PHASE_IDLE ) && !LagChkExe(PKTTCBNO, PKT_TIM_DC_QR_RESND) ){
			    	// パケット生成
					data_size = PKTsnd_Create_LogData_Packet( TARGET_DC_QR, pktctrl.snd_seqno, &snd_trailer );

					// パケット生成OK？
					if( data_size ){
						// ***************
						//    送信要求
						// ***************
						pkt_send_req_snd( data_size, (uchar *)&snd_trailer, snd_trailer.part.block_sts );
						// ログデータの応答待ちタイマはACK受信で開始
						pktctrl.send.QR_LogSend = LOG_PHASE_WAIT_ACK;

						// 送信してからインクリメントする
						pktctrl.snd_seqno++;
						if( pktctrl.snd_seqno > 999 ){
							pktctrl.snd_seqno = 1;
						}
						if( pktctrl.kpalv_snd_intvl ){	// キープアライブ有効
							// キープアライブタイマリセット
							Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, (ushort)(pktctrl.kpalv_snd_intvl*50) );
							RESET_KEEPALIVE_COUNT;
						}else{	// キープアライブ無効(以下、保険)
							Lagcan( PKTTCBNO, PKT_TIM_KEEPALIVE );
						}
					}
					// パケット生成NG？
					else{
						// パケット生成NGのログは送信済みとする(このルートは通らないはず)
						Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_QR, eLOG_TARGET_LCD );	// 送信済みとする
						Pay_Data_ID = 0;
						QR_Data_ID = 0;
// GG129004(S) R.Endo 2024/11/19 電子領収証対応
						Receipt_Data_ID = 0;
// GG129004(E) R.Endo 2024/11/19 電子領収証対応
// 調査用 (s)
#if (1 == AUTO_PAYMENT_PROGRAM)
wmonlg( OPMON_TEST, 0, 20 );
#endif
// 調査用 (e)
					}
					continue;	// RT精算データ送信を優先させる為、パケット生成NG時もcontinue
			    }
// GG129000(S) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
				// レーンモニタデータ未送信あり？
				else if( Ope_Log_UnreadCountGet( eLOG_DC_LANE, eLOG_TARGET_LCD ) && ( pktctrl.send.LANE_LogSend == LOG_PHASE_IDLE ) && !LagChkExe(PKTTCBNO, PKT_TIM_DC_LANE_RESND) ){
			    	// パケット生成
					data_size = PKTsnd_Create_LogData_Packet( TARGET_DC_LANE, pktctrl.snd_seqno, &snd_trailer );

					// パケット生成OK？
					if( data_size ){
						// ***************
						//    送信要求
						// ***************
						pkt_send_req_snd( data_size, (uchar *)&snd_trailer, snd_trailer.part.block_sts );
						// ログデータの応答待ちタイマはACK受信で開始
						pktctrl.send.LANE_LogSend = LOG_PHASE_WAIT_ACK;

						// 送信してからインクリメントする
						pktctrl.snd_seqno++;
						if( pktctrl.snd_seqno > 999 ){
							pktctrl.snd_seqno = 1;
						}
						if( pktctrl.kpalv_snd_intvl ){	// キープアライブ有効
							// キープアライブタイマリセット
							Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, (ushort)(pktctrl.kpalv_snd_intvl*50) );
							RESET_KEEPALIVE_COUNT;
						}else{	// キープアライブ無効(以下、保険)
							Lagcan( PKTTCBNO, PKT_TIM_KEEPALIVE );
						}
					}
					// パケット生成NG？
					else{
						// パケット生成NGのログは送信済みとする(このルートは通らないはず)
						Ope_Log_TargetVoidReadPointerUpdate( eLOG_DC_LANE, eLOG_TARGET_LCD );	// 送信済みとする
						LANE_Data_ID = 0;
					}
					continue;	// RT精算データ送信を優先させる為、パケット生成NG時もcontinue
				}
// GG129000(E) H.Fujinaga 2023/01/19 ゲート式車番チケットレスシステム対応（レーンモニタデータ対応）
			}

			// ログデータ以外のコマンドの送信
			// (ログデータはコマンドバッファにセットしないので_ref_send.sent/commited管理対象外)
			if( _ref_send.sent != _ref_send.commited ){
				data_size = PKTsnd_Create_Packet( pktctrl.snd_seqno, &snd_trailer );
				switch( _ref_buf_type ){
					case 'A':
						_ref_buf_type = 'S';
						break;
					default:
						break;
				}
				// データ準備OK
				if( data_size ){
					// ***************
					//    送信要求
					// ***************
					pkt_send_req_snd( data_size, (uchar *)&snd_trailer, snd_trailer.part.block_sts );
					// 送信してからインクリメントする
					pktctrl.snd_seqno++;
					if( pktctrl.snd_seqno > 999 ){
						pktctrl.snd_seqno = 1;
					}
					if( pktctrl.kpalv_snd_intvl ){	// キープアライブ有効
						// キープアライブタイマリセット
						Lagtim( PKTTCBNO, PKT_TIM_KEEPALIVE, (ushort)(pktctrl.kpalv_snd_intvl*50) );
						RESET_KEEPALIVE_COUNT;
					}else{	// キープアライブ無効(以下、保険)
						Lagcan( PKTTCBNO, PKT_TIM_KEEPALIVE );
					}
				}
				// 送信バッファにデータなし？
				else{
					_ref_send.sent = _ref_send.commited;	// 全て送信済みへ(フェイルセーフ)
				}
			}
		}
	}
}

const unsigned short tenkey_event_table[KeyDatMaxLCD][8] = {
	{KEY_TEN_CL,KEY_TEN_F5,	KEY_TEN_F4,	KEY_TEN_F3,	KEY_TEN_F2,	KEY_TEN_F1,	KEY_TEN9,	KEY_TEN8},
	{KEY_TEN7,	KEY_TEN6,	KEY_TEN5,	KEY_TEN4,	KEY_TEN3,	KEY_TEN2,	KEY_TEN1,	KEY_TEN0},
};
//[]----------------------------------------------------------------------[]
///	@brief		テンキーボタン状態チェック
//[]----------------------------------------------------------------------[]
///	@param[in]	tenkey		テンキーボタン状態へのポインタ
///	@return		none
/// @note		LCDから受信したテンキーボタン状態をチェックして
///				変化のあったテンキーボタンをopeタスクに通知する
//[]------------------------------------- Copyright(C) 2017 AMANO Corp.---[]
static void pkt_tenkey_status_check( uchar *tenkey )
{
	uchar i,j;
	uchar wkbit;
	uchar on_off;

	for (i = 0; i < KeyDatMaxLCD; i++) {
		wkbit = 0x80;
		for (j = 0; j < 8; j++, wkbit>>=1) {
			if ((tenkey[i] & wkbit) != (key_dat_LCD[i].BYTE & wkbit)) {	// 変化あり
				// 変化のあったテンキーボタン状態をopeタスクに通知する
				if (tenkey[i] & wkbit) {
					on_off = 1;
				} else {
					on_off = 0;
				}
				queset( OPETCBNO, tenkey_event_table[i][j], 1, &on_off );
			}
// MH810100(S) Y.Yamauchi 2020/03/05 #3901 バックライトの点灯/消灯が設定通りに動作しない
			// CキーのONは点灯用に常に送信
			else if( (tenkey[i] & wkbit) && (tenkey_event_table[i][j] == KEY_TEN_CL) ){
				on_off = 1;
				queset( OPETCBNO, tenkey_event_table[i][j], 1, &on_off );
			}
// MH810100(E) Y.Yamauchi 2020/03/05 #3901 バックライトの点灯/消灯が設定通りに動作しない
		}
		key_dat_LCD[i].BYTE = tenkey[i];	// key_dat_LCD更新
	}

}

//[]----------------------------------------------------------------------[]
///	@brief		kasago起動エラー情報取得/登録処理
//[]----------------------------------------------------------------------[]
///	@param[in]	fid		ソケットディスクリプタ
///	@param[in]	code	エラーコード(各関数毎にope_def.hで定義されている値)
///	@param[in]	knd		0:解除/1:発生/2:発生解除
///	@return		error	エラー情報
/// @note		kasago起動エラー発生時、エラー情報(bin)を取得して登録する
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static ulong pkt_ksg_err_getreg( int fid, char code, char knd )
{
	ulong	error;

	if( code == ERR_TKLSLCD_KSG_SOC ){	// socket()エラー？
		fid = TM_SOCKET_ERROR;
	}
	error = (ulong)tfGetSocketError( fid );		// エラー情報取得
	if(( code == ERR_TKLSLCD_KSG_CON ) &&		// connect()のエラー
	   (( error == TM_EINPROGRESS ) ||			// ノンブロッキングかつ現在の接続試行がまだ完了していない
	    ( error == TM_EISCONN ))){				// tfGetSocketErrorがコールされるまでの間に接続が完了した場合
	    ;	// エラー扱いとしないので登録しない
	}else if(( code == ERR_TKLSLCD_KSG_RCV ) &&	// recv()のエラー
			 ( error == TM_EWOULDBLOCK )){		// ノンブロッキングで読み込み可能なデータがない
	    ;	// エラー扱いとしないので登録しない
	}else{
		// 戻り値をエラー情報(bin)としてそのまま登録する
		err_chk2( ERRMDL_TKLSLCD, code, knd, 2, 0, &error );
	}

	return error;
}

//[]----------------------------------------------------------------------[]
///	@brief		通信不良関連エラー全解除処理
//[]----------------------------------------------------------------------[]
///	@param[in]	none
///	@return		none
/// @note		通信不良関連エラーを全て解除する
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
static void pkt_can_all_com_err( void )
{
	char	i;

	for( i = 0; i < PKT_COMM_FAIL_MAX; i++ ){
		err_chk2( ERRMDL_TKLSLCD, i, 0, 0, 0, NULL );
	}
}

//[]----------------------------------------------------------------------[]
///	@brief		復旧データ未送信フラグ取得
//[]----------------------------------------------------------------------[]
///	@param[in]	none
///	@return		復旧データ未送信フラグ
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
uchar pkt_get_restore_unsent_flg( void )
{
	return restore_unsent_flg;
}

//[]----------------------------------------------------------------------[]
///	@brief		復旧データ未送信フラグセット
//[]----------------------------------------------------------------------[]
///	@param[in]	フラグ状態<br>
///				(0=復旧データ送信済み / 1=復旧データ送信要(起動時) / 2=復旧データ送信要(復旧時))
///	@return		none
//[]------------------------------------- Copyright(C) 2020 AMANO Corp.---[]
void pkt_set_restore_unsent_flg( uchar flg )
{
	restore_unsent_flg = flg;
}
// MH810100(E) K.Onodera  2019/11/11 車番チケットレス(GT-8700(LZ-122601)流用)

// MH810105(S) R.Endo 2021/12/13 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
#if DEBUG_JOURNAL_PRINT
//[]----------------------------------------------------------------------[]
///	@brief		socketデバッグ印字処理
//[]----------------------------------------------------------------------[]
///	@param[in]	none
///	@return		none
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
static void pkt_socket_debug_print( void )
{
	const int row_max = 4;

	T_FrmDebugData frm_debug_data;
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
	struct clk_rec	wk_CLK_REC_debug;
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
	ttSocketEntryPtr socket_entry_ptr;
	int i;
	int data_index;
	int null_count;

	// クリア
	memset(&frm_debug_data, 0x00, sizeof(frm_debug_data));

	// 現在日時
	c_Now_CLK_REC_ms_Read(
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
//		&frm_debug_data.wk_CLK_REC,
		&wk_CLK_REC_debug,
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		&frm_debug_data.wk_CLK_REC_msec);
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
	// 7バイトのみCOPY
	memcpy( &frm_debug_data.wk_CLK_YMDHMS, &wk_CLK_REC_debug, sizeof(DATE_YMDHMS) );
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）

	// ジャーナル印字
	frm_debug_data.prn_kind = J_PRI;

	// 1行目(ソケットオープン数の現在値と最大値)
	frm_debug_data.first_line = TRUE;
	sprintf(frm_debug_data.data, "sock:%u/%u tcp_sock:%u/%u",
		tm_context(tvNumberOpenSockets), tm_context(tvMaxNumberSockets),
		tm_context(tvTcpVectAllocCount), tm_context(tvMaxTcpVectAllocCount));

	// 2行目以降(各ソケットのステータスフラグ)
	data_index = 0;
	null_count = 0;
	for ( i = 0; i < tm_context(tvMaxNumberSockets); i++ ) {
		if ( (i % row_max) == 0 ) {
			if ( null_count != row_max ) {
				queset(PRNTCBNO, PREQ_DEBUG, sizeof(T_FrmDebugData), &frm_debug_data);
			}
			data_index = 0;
			null_count = 0;
			memset(frm_debug_data.data, 0x00, sizeof(frm_debug_data.data));
			frm_debug_data.first_line = FALSE;
		}
		socket_entry_ptr = tfSocketCheckValidLock(i);
		if ( socket_entry_ptr == TM_SOCKET_ENTRY_NULL_PTR ) {
			data_index += sprintf(&frm_debug_data.data[data_index], "%2d:---- ", i);
			null_count++;
		} else {
			data_index += sprintf(&frm_debug_data.data[data_index], "%2d:%04x ", i, socket_entry_ptr->socFlags);
		}
	}
	if ( null_count != row_max ) {
		queset(PRNTCBNO, PREQ_DEBUG, sizeof(T_FrmDebugData), &frm_debug_data);
	}
}
#endif
// MH810105(E) R.Endo 2021/12/13 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
