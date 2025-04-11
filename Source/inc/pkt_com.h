// MH810100(S) K.Onodera 2019/11/11 車番チケットレス(GT-8700(LZ-122601)流用)
//[]----------------------------------------------------------------------[]
///	@brief			パケット通信コマンド関連ヘッダーファイル
//[]----------------------------------------------------------------------[]
///	@author			m.yatsuya
///	@date			Create	: 2009/09/04
///	@file			pkt_com.h
///	@version		LH068004
//[]------------------------------------- Copyright(C) 2009 AMANO Corp.---[]
#ifndef	_PKT_COM_H_
#define	_PKT_COM_H_

// MH810104(S) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算DLLインターフェースの修正)
#include "system.h"
// MH810104(E) R.Endo 2021/08/18 車番チケットレス フェーズ2.3 #5792 多店舗割引対応(料金計算DLLインターフェースの修正)

// 電文
enum {
// メイン→サブ：コマンド
	PKT_STS_REQ			= 0x00,			// ステータス要求		: B(Base), P(Port)
	PKT_BUF_CLR_REQ,					// バッファクリア要求	: P
	PKT_PRT_OPN_REQ		= 0x10,			// ポートOpen要求		: B
	PKT_TCP_CON_REQ,					// TCP Connect要求		: P
	PKT_PKT_SND_REQ,					// パケット送信要求		: P
	PKT_PRT_CLS_REQ,					// ポートClose要求		: B

// メイン→サブ：レスポンス

// サブ→メイン：レスポンス
	PKT_STS_RES			= 0x80,			// ステータス応答
	PKT_BUF_CLR_RES,					// バッファクリア応答
	PKT_PRT_OPN_RES		= 0x90,			// ポートOpen応答
	PKT_TCP_CON_RES,					// TCP Connect応答
	PKT_PKT_SND_RES,					// パケット送信応答
	PKT_PRT_CLS_RES,					// ポートClose応答

// サブ→メイン：コマンド
	PKT_RCV_NOTIFY		= 0xA0			// パケット受信通知
};

// 状態
enum {
	PKT_BASE_CLOSE,
	PKT_BASE_OPEN,
};

enum {
	PKT_PORT_DISCONNECT,
	PKT_PORT_CONNECTING,
	PKT_PORT_LISTEN,
	PKT_PORT_ESTABLISH,
	PKT_PORT_SHUTDOWN
};

#define PKT_PORT_MAX		1
#define PKT_CONN_TIMEOUT	40			// タイムアウト時間(秒)
//#define PKT_DATA_MAX	490	// パケットデータの最大サイズ
//#define PKT_CMD_MAX		462	// コマンドの最大サイズ
// @todo	GT-4100:最大サイズ調整要
#define PKT_CMD_MAX		4096			// データ部の最大サイズ
#define PKT_DATA_MAX	PKT_CMD_MAX+28	// パケットデータ最大長＝データ部の最大サイズ＋ヘッダサイズ(26Byte)＋CRC16(2Byte)
#define PKT_NOTIFY_MBX_MAX	2

typedef struct {
	uchar	kind;						// 電文種別
	uchar	res_sts;					// レスポンスステータス
} PKT_CMN;

typedef struct {
	uchar	kind;						// 電文種別
	uchar	reserve;					// 予約
	uchar	empty[2];					// 空き
} PKT_STS_CMD;

typedef struct {
	uchar	kind;						// 電文種別
	uchar	res_sts;					// レスポンスステータス
	uchar	sckt_sts;					// ソケットポート状態
	uchar	nfull_alm;					// 未送信パケットニアフルアラーム
	uchar	src_IP[4];					// 自局IPアドレス
	ushort	src_port;					// 自局ポート番号
	uchar	dst_IP[4];					// 相手局IPアドレス
	ushort	dst_port;					// 相手局ポート番号
	ushort	unsnd_rate;					// 未送信パケットメモリ使用率
	uchar	empty[6];					// 空き
} PKT_STS_RES_PORT;

typedef struct {
	uchar	kind;						// 電文種別
	uchar	reserve;					// 予約
	uchar	empty[2];					// 空き
} PKT_CLR_CMD;

typedef struct {
	uchar	kind;						// 電文種別
	uchar	res_sts;					// レスポンスステータス
	uchar	empty[2];					// 空き
} PKT_CLR_RES;

typedef struct {
	uchar	kind;						// 電文種別
	uchar	reserve;					// 予約
	uchar	port_no;					// 通信ポート
	uchar	opn_mode;					// Openモード
	ushort	src_port;					// 自局ポート番号
	uchar	empty[2];					// 空き
} PKT_OPN_CMD;

typedef struct {
	uchar	kind;						// 電文種別
	uchar	res_sts;					// レスポンスステータス
	uchar	port_no;					// 通信ポート
	uchar	opn_mode;					// Openモード
	ushort	src_port;					// 自局ポート番号
	uchar	empty[2];					// 空き
} PKT_OPN_RES;

typedef struct {
	uchar	kind;						// 電文種別
	uchar	reserve;					// 予約
	uchar	dst_IP[4];					// 相手局IPアドレス
	ushort	dst_port;					// 相手局ポート番号
	uchar	wait_time;					// 接続最大待ち時間
	uchar	empty;						// 空き
} PKT_CON_CMD;

typedef struct {
	uchar	kind;						// 電文種別
	uchar	res_sts;					// レスポンスステータス
	uchar	dst_IP[4];					// 相手局IPアドレス
	ushort	dst_port;					// 相手局ポート番号
	uchar	wait_time;					// 接続最大待ち時間
	uchar	empty;						// 空き
} PKT_CON_RES;

typedef struct {
	uchar	kind;						// 電文種別
	uchar	reserve;					// 予約
	uchar	snd_no;						// 送信データ識別番号
	uchar	block_sts;					// ブロックステータス
	uchar	empty[4];					// 空き
	ushort	data_size;					// データサイズ
	uchar	snd_data[PKT_DATA_MAX];		// データ
} PKT_SND_CMD;

typedef struct {
	uchar	kind;						// 電文種別
	uchar	res_sts;					// レスポンスステータス
	uchar	snd_no;						// 送信データ識別番号
	uchar	block_sts;					// ブロックステータス
	uchar	empty[4];					// 空き
	ushort	data_size;					// データサイズ
} PKT_SND_RES;

typedef struct {
	uchar	kind;						// 電文種別
	uchar	rcv_sts;					// 受信ステータス
	uchar	empty[6];					// 空き
	ushort	data_size;					// データサイズ
	uchar	rcv_data[PKT_DATA_MAX];		// データ
} PKT_RCV_RES;

typedef struct {
	uchar	kind;						// 電文種別
	uchar	reserve;					// 予約
	uchar	port_no;					// 通信ポート
	uchar	empty;						// 空き
} PKT_CLS_CMD;

typedef struct {
	uchar	kind;						// 電文種別
	uchar	res_sts;					// レスポンスステータス
	uchar	port_no;					// 通信ポート
	uchar	empty;						// 空き
} PKT_CLS_RES;

typedef union {
	PKT_CMN			com;
	PKT_OPN_CMD		opn;				// Open
	PKT_CON_CMD		con;				// Connect
	PKT_SND_CMD		snd;				// Send
	PKT_CLS_CMD		cls;				// Close
} PKT_SNDCMD;

typedef union {
	PKT_CMN				com;
	PKT_OPN_RES			opn;			// Open
	PKT_CON_RES			con;			// Connect
	PKT_SND_RES			snd;			// Send
	PKT_RCV_RES			rcv;			// Receive
	PKT_CLS_RES			cls;			// Close
} PKT_RCVRES;

typedef	union {
	uchar	data[PKT_DATA_MAX];
	struct {
		ulong	data_size;				// パケットデータ長(data_size～crc16h)
		uchar	identifier[12];			// ヘッダーコード("AMANO_PACKET")
		uchar	seqno[3];				// シーケンス№("000"～"999")
		uchar	id;						// データ部コード(0x00)
		uchar	kind;					// 電文識別コード(0x10)
		uchar	blkno_h;				// 
		uchar	blkno_l;				// ブロックNo.(1)
		uchar	block_sts;				// ブロックステータス(1)
		uchar	encrypt;				// 暗号化方式(0:暗号化なし,1:AES)
		uchar	pad_size;				// パディングサイズ(0～16:暗号化時のデータ部のパディングサイズ)
		uchar	data[PKT_CMD_MAX];		// データ部(可変長)
		uchar	pad[16];				// 暗号化時に16Byte単位とする為の0パディング用エリア(RT精算データがPKT_CMD_MAXとなった場合を考慮)
//-		uchar	check_area[16];			// EncryptWithKeyAndIV()起動条件(チェックで使用)エリア
//		ushort	crc16;					// CRC16(データ部が可変長のため定義しない)
	} part;
} PKT_TRAILER;

#endif	// _PKT_COM_H_
// MH810100(E) K.Onodera 2019/11/11 車番チケットレス(GT-8700(LZ-122601)流用)
