/*[]----------------------------------------------------------------------[]
 *|	filename: raudef.h
 *[]----------------------------------------------------------------------[]
 *| summary	: RAUタスク定義データヘッダ
 *| date	: 2012-09-11
 *| update	:
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/

#ifndef	RAUDEF_H
#define	RAUDEF_H

#include	"system.h"
#include	"common.h"
#include	"mem_def.h"
#include	"ntnet.h"
#include	"rauconstant.h"

#define		RAU_NTDATABUF_COUNT		20
#define		RAU_BLKSIZE				1024					// 1パケット長MAX

#define		RAU_SET				1							// SET
#define		RAU_CLR				0							// CLR

#define		COMMON_CMD			0							// 共通コマンド
#define		UPLINE				1							// 上り回線
#define		DOWNLINE			2							// 下り回線
#define		CREDIT				3							// クレジット回線
#define		RAU_ACK				0x06						// ACK
#define		RAU_NAK				0x15						// NAK

#define		RAU_DUMMY_STR_LEN	16							// ダミーデータサイズ

#define		RAU_CONN_RETRY_MAX	3							// コネクション確立リトライ回数
#define		RAU_CONN_TIMEOUT	40							// タイムアウト時間(秒)
#define		RAU_TCP_CLOSE_TIME	5 * 100						// アンテナレベルチェック時のTCP切断待ち時間(10ms単位)

#define		RAU_RECONNECT_MAX	9							// ACK待ち中の切断時の再接続回数
#define		RAU_RECONNECT_INTERVAL	3						// ACK待ち中の切断時の再接続間隔

#define	NET_STA_IDLE			0	// アイドル
#define	NET_STA_PPP_OPENNING	1	// ＰＰＰ接続中
#define	NET_STA_PPP_OPEN		2	// ＰＰＰ接続確立中
#define	NET_STA_TCP_OPENNING	3	// ＴＣＰ接続中
#define	NET_STA_TCP_OPEN		4	// ＴＣＰ接続確立中
#define	NET_STA_TCP_CLOSING		5	// ＴＣＰ切断中
#define	NET_STA_TCP_CLOSE		6	// ＴＣＰの切断確認済み
#define	NET_STA_PPP_CLOSING		7	// ＰＰＰ切断中
#define	NET_STA_PPP_CLOSE		8	// ＰＰＰの切断確認済み

extern	ulong	RAU_x_1mLifeTime;			/* timer counter. up to every 1ms */
#define		RAU_DECIMAL_32BIT	4294967					// 32BITの小数点部の計算に使う値 (2^32 / 1000)
#define		RAU_TIMEZONE		32400;					// タイムゾーン 日本 +9:00 9*60*60 = 32400秒

// ソケット通信状態
typedef enum {
	RAU_SOCK_INIT,											// 初期化
	RAU_SOCK_OPEN,											// ソケットオープン
	RAU_SOCK_CONNECTING,									// TCP確立中
	RAU_SOCK_CONNECTED,										// TCP確立
	RAU_SOCK_LISTENING,										// TCP接続待ち
	RAU_SOCK_IDLE,											// TCP接続中
	RAU_SOCK_CLOSING,										// TCP切断中
	RAU_SOCK_CLOSED,										// TCP切断
} RAU_SOCK_CONDITION;

typedef enum {
	RAU_LINE_NO_ERROR,										// 正常
	RAU_LINE_SEND,											// データ送信実行
	RAU_LINE_ERROR,											// エラー発生
} RAU_LINE_STATE;

typedef enum {
	RAU_NET_IDLE,											// 未接続
	RAU_NET_PPP_OPENED,										// PPP確立中
	RAU_NET_TCP_CONNECTIED,									// TCP確立中
} RAU_NET_STATE;

// ビット参照用
union	rau_bits_reg {
	struct	rau_bit_tag {
		uchar	bit_7	: 1 ;
		uchar	bit_6	: 1 ;
		uchar	bit_5	: 1 ;
		uchar	bit_4	: 1 ;
		uchar	bit_3	: 1 ;
		uchar	bit_2	: 1 ;
		uchar	bit_1	: 1 ;
		uchar	bit_0	: 1 ;
	} bits;
	uchar	byte;
};
typedef union rau_bits_reg	RAU_BITS;

union _BF2
{
	struct {
		uchar	reserve		:5;
		uchar	credit		:1;
		uchar	upload		:1;
		uchar	download	:1;
	} port;
	uchar	both;
};
typedef	union _BF2	BF2;

// RAUタスク受信バッファ（対OPE層）
typedef struct {
	uchar	ntDataBuf[RAU_NTDATABUF_COUNT][RAU_BLKSIZE];	// NT-NETデータバッファ
	ushort	readIndex;										// 読込みインデックス
	ushort	writeIndex;										// 書込みインデックス
	ushort	count;											// 格納データ数
} RauNtDataBuf;

extern	RauNtDataBuf	RauOpeSendNtDataBuf;				// RAUタスク送信バッファ（対OPE層）
extern	RauNtDataBuf	RauOpeRecvNtDataBuf;				// RAUタスク受信バッファ（対OPE層）

// RAUタスク制御テーブル
typedef struct {
	BF2		tcpConnectFlag;									// TCPコネクション状態フラグ
	uchar	TCPcnct_req;									// TCPコネクションリクエストフラグ
	uchar	upConnecting;									// 上りコネクション接続中フラグ 0:未接続 1:確立中 2:接続中
	uchar	DPA_TCP_DummySend;								// ダミーパケット送信フラグ
	uchar	upNetState;										// 上り回線状態
	uchar	downNetState;									// 下り回線状態
	uchar	reserved1;
	ushort	connRetryCount;									// コネクションリトライカウンタ
	T_RAU_BLKDATA	mainRecvData;							// 現在処理中のメインからの受信データ
} tRauCtrl;
extern	tRauCtrl RauCtrl;

// RAU設定テーブル
typedef struct  {
	char		id22_23_tel_flg;		/* ＩＤ自動発信フラグ  （精算データ）*/
	char		id30_41_tel_flg;		/* ＩＤ自動発信フラグ  （集計データ）*/
	char		id121_tel_flg;			/* ＩＤ自動発信フラグ  （アラームデータ）*/
	char		id123_tel_flg;			/* ＩＤ自動発信フラグ  （操作モニターデータ）*/
	char		id120_tel_flg;			/* ＩＤ自動発信フラグ  （エラーデータ）*/
	char		id20_21_tel_flg;		/* ＩＤ自動発信フラグ  （入庫出庫データ）*/
	char		id122_tel_flg;			/* ＩＤ自動発信フラグ  （モニターデータ）*/
	char		id131_tel_flg;			/* ＩＤ自動発信フラグ  （コイン金庫集計データ）*/
	char		id133_tel_flg;			/* ＩＤ自動発信フラグ  （紙幣金庫集計データ）*/
	char		id236_tel_flg;			/* ＩＤ自動発信フラグ  （駐車台数データ）*/
	char		id237_tel_flg;			/* ＩＤ自動発信フラグ  （区画台数・満車データ）*/
	char		id126_tel_flg;			/* ＩＤ自動発信フラグ  （金銭管理データ）*/
	char		id135_tel_flg;			/* ＩＤ自動発信フラグ  （釣銭管理集計データ）*/
	char		id125_tel_flg;			/* ＩＤ自動発信フラグ  （遠隔監視データ）*/
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	char		id61_tel_flg;			/* ＩＤ自動発信フラグ  （長期駐車情報データ）*/
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	char		id22_23_mask_flg;		/* 送信マスク設定  （精算データ）*/
	char		id30_41_mask_flg;		/* 送信マスク設定  （集計データ）*/
	char		id121_mask_flg;			/* 送信マスク設定  （アラームデータ）*/
	char		id123_mask_flg;			/* 送信マスク設定  （操作モニターデータ）*/
	char		id120_mask_flg;			/* 送信マスク設定  （エラーデータ）*/
	char		id20_21_mask_flg;		/* 送信マスク設定  （入庫出庫データ）*/
	char		id122_mask_flg;			/* 送信マスク設定  （モニターデータ）*/
	char		id131_mask_flg;			/* 送信マスク設定  （コイン金庫集計データ）*/
	char		id133_mask_flg;			/* 送信マスク設定  （紙幣金庫集計データ）*/
	char		id236_mask_flg;			/* 送信マスク設定  （駐車台数データ）*/
	char		id237_mask_flg;			/* 送信マスク設定  （区画台数・満車データ）*/
	char		id126_mask_flg;			/* 送信マスク設定  （金銭管理データ）*/
	char		id135_mask_flg;			/* 送信マスク設定  （釣銭管理集計データ）*/
	char		id125_mask_flg;			/* 送信マスク設定  （遠隔監視データ）*/
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	char		id61_mask_flg;			/* 送信マスク設定  （長期駐車情報データ）*/
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	char		alarm_send_level;		/* アラームデータの送信レベル(34-0037)		*/
	char		error_send_level;		/* エラーデータの送信レベル(34-0037)		*/
	char		opemonitor_send_level;	/* 操作モニターデータの送信レベル(34-0037)	*/
	char		monitor_send_level;		/* モニターデータの送信レベル(34-0037)		*/
	char		rmon_send_level;		/* 遠隔監視データの送信レベル				*/
	char		serverTypeSendTsum;		/* サーバータイプ時のデータ送信 34-0026④	*/
	char		deta_full_proc;			/* データ処理方法 */
	ushort		IBW_RespWtime;			/* 送信結果受信完了待ち時間（単位＝秒）*/
	uchar		system_select;			/* システム選択 */
	uchar		modem_exist;			/* モデムの有無 */
	uchar		Dpa_proc_knd;			/* Dopa 処理区分 [0]=常時通信なし, [1]=常時通信あり */
	ulong		Dpa_IP_h;				/* Dopa HOST局IPアドレス */
	ushort		Dpa_port_h;				/* Dopa HOST局ポート番号 */
	ulong		Dpa_IP_m;				/* Dopa 自局IPアドレス */
	ushort		Dpa_port_m;				/* Dopa 自局ポート番号 */
	ushort		Dpa_nosnd_tm;			/* Dopa 無通信タイマー(秒) */
	ushort		Dpa_cnct_rty_tm;		/* Dopa 再発呼待ちタイマー(秒) */
	ushort		Dpa_com_wait_tm;		/* Dopa コマンド(ACK)待ちタイマー(秒) */
	ushort		Dpa_data_rty_tm;		/* Dopa データ再送待ちタイマー(分) */
	ushort		Dpa_discnct_tm;			/* Dopa TCPコネクション切断待ちタイマー(秒) */
	ushort		Dpa_cnct_rty_cn;		/* Dopa 再発呼回数 */
	uchar		Dpa_data_rty_cn;		/* Dopa データ再送回数(NAK,無応答) */
	uchar		Dpa_data_code;			/* Dopa HOST側電文コード [0]=文字コード, [1]=バイナリコード */
	ushort		Dpa_com_wait_reconnect_tm;		/* Dopa 再接続待ちタイマー(秒) */
	uchar		Dpa_data_rty_cn_disconnected;		/* Dopa データ再送回数(TCP切断) */

	uchar		tcp_disconect_flag;		// TCP切断あり／なし 0=あり 1=なし
	uchar		interrupt_proc;			/* 中断要求時の処理 */
	ushort		interrupt_proc_block;	/* 中断要求受信時の判定ブロック */

	uchar		Dpa_data_snd_rty_cnt;	// ＤｏＰａ下り回線パケット送信リトライ回数(無応答)
	ushort		Dpa_ack_wait_tm;		// ＤｏＰａ下り回線パケット応答待ち時間(ＡＣＫ/ＮＡＫ)
	uchar		Dpa_data_rcv_rty_cnt;	// ＤｏＰａ下り回線パケット受信リトライ回数(ＮＡＫ)
	ushort		Dpa_port_watchdog_tm;	// ＤｏＰａ下り回線通信監視タイマ(秒)
	ushort		Dpa_port_m2;			// ＤｏＰａ下り回線 自局ポート番号
	ulong		Dpa_IP_Cre;				/* クレジットセンターIPアドレス */
	ushort		Dpa_port_Cre;			/* クレジットセンターポート番号 */
	ushort		Dpa_port_m3;			/* クレジット用自局ポート番号 */
	uchar		Credit_snd_rty_cnt;		// クレジット回線パケット送信リトライ回数(無応答)
	ushort		Credit_ack_wait_tm;		// クレジット回線パケット応答待ち時間(ＡＣＫ/ＮＡＫ)
	uchar		Credit_nak_rty_cnt;		// クレジット回線パケット受信リトライ回数(ＮＡＫ)
	ushort		Credit_discnct_tm;		// TCPコネクション切断待ちタイマー(秒) 
	uchar		Credit_cnct_rty_cnt;	// コネクションリトライ回数
	uchar		Credit_snd_rty_cnt_disconnected;		// クレジット回線パケット送信リトライ回数(TCP切断)
	ushort		Credit_ack_wait_reconnect_tm;	// クレジット回線パケット応答待ち中切断の再接続時間
} tRauConfig;

extern	tRauConfig	RauConfig;


typedef struct {
	ushort		Len;
	uchar		Dat[RAU_DATA_MAX];
} t_DPA_DATA;

typedef struct {						// 送受信キュー
	uchar		Count;
	uchar		ReadIdx;
	uchar		WriteIdx;
	uchar		dummy;
	t_DPA_DATA	dpa_data[4];
} t_DPA_Ctrl;

typedef struct {						// 送受信キュー
	uchar		Count;
	uchar		ReadIdx;
	uchar		WriteIdx;
	uchar		dummy;
	uchar		dpa_data[4][30];
} t_DPA2_Ctrl;

extern	t_DPA_Ctrl	DPA2_Snd_Ctrl;		// 下り回線の送信キュー
extern	t_DPA2_Ctrl	DPA2_Rcv_Ctrl;		// 上り回線の受信キュー

extern	t_DPA_Ctrl	DPA_Snd_Ctrl;		// 上り回線の送信キュー
extern	t_DPA_Ctrl	DPA_Rcv_Ctrl;		// 下り回線の受信キュー


extern	uchar	RAU_SetSendNtData(const uchar* pData, ushort size);
extern	uchar*	RAU_GetSendNtData(void);
extern	uchar	RAU_SetSendTransactionData(const uchar* pData, ushort size);
extern	uchar*	RAU_GetSendTransactionData(void);

extern	void	RAU_err_chk(char code, char kind, char f_data, char err_ctl, void *pData);

extern	void	RAU_SockMain(void);
extern	void	RAU_SockInit(void);
extern	uchar	RAU_ReceiveUpData(uchar* pBuffer, ushort* pSize);
extern	uchar	RAU_ReceiveDownData(uchar* pBuffer, ushort* pSize);
extern	RAU_NET_STATE	RAU_GetUpNetState(void);
extern	RAU_NET_STATE	RAU_GetDownNetState(void);
extern	void	SetUpNetState(uchar state);
extern	uchar	GetUpNetState(void);
extern	void	SetDownNetState(uchar state);
extern	uchar	GetDownNetState(void);
extern	void	RAU_SetAntennaLevelState(uchar state);
extern	uchar	RAU_GetAntennaLevelState(void);

extern	void	RAU_CrcCcitt(ushort length, ushort length2, uchar cbuf[], uchar cbuf2[], uchar *result, uchar type );
extern	void	RAU_Word2Byte(unsigned char *data, ushort uShort);
extern	ushort	RAU_Byte2Word(unsigned char *data);
extern	ulong	RAU_Byte2Long(unsigned char *data);

extern	uchar	RAUhost_GetSndSeqFlag(void);
extern	uchar	RAUhost_GetRcvSeqFlag(void);
extern	BOOL	RAUdata_CanSendTableData(short elogID);
extern	void	RAU_RequestCheckNearfull(void);
extern	ulong	RAU_GetCenterSeqNo(RAU_SEQNO_TYPE type);
extern	void	RAU_UpdateCenterSeqNo(RAU_SEQNO_TYPE type);
extern	void	RAU_SNTPStart( uchar onoff ,ushort msg, ulong now_sec, ushort now_msec );
extern	void	RAU_SNTPGetPacket( void *voidPtr );

// MH322915(S) K.Onodera 2017/05/18 遠隔ダウンロード修正
extern	void	RAU_Cancel_RetryTimer();
// MH322915(E) K.Onodera 2017/05/18 遠隔ダウンロード修正

										// 送信シーケンスフラグのセット
extern	void	RAUhost_SetSndSeqFlag(uchar ucSndSeqFlag);
										// 受信シーケンスフラグのセット
extern	void	RAUhost_SetRcvSeqFlag(uchar ucRcvSeqFlag);
										// 送信要求を解除する
extern	void	RAUhost_ClearSendRequeat(void);
										// データテーブルのサイズにより先頭ポインタを求める
extern	void	RAU_data_table_init( void );
										// T-sum data
extern	int		init_tsum(char *buff, long size);
extern	int		init_center_term_info(char *buff);
extern	void	RAU_TimerInit( void );	// 全タイマ停止（立ち上がり時にCallされる）
extern	void	RAUid_Init(void);		// 初期化
										// TCP Connection 要求失敗時のクリア処理
extern	void	RAUhost_TcpConnReq_Clear ( void );
										// 下り回線通信監視タイマーがタイムアウトした時の処理
extern	void	RAUhost_DwonLine_common_timeout(void);
										// 集計データの作成、tsum格納(フォーマットRevNo.10用)
extern	void	RAUhost_CreateTsumData_r10(ushort type);
										// 集計データの作成、tsum格納
extern	void	RAUhost_CreateTsumData(void);
										// テーブルデータのニアフル通知を行う
extern	void	RAUdata_CheckNearFullLogData(void);
extern	eRAU_TABLEDATA_SET	RAUid_SetNtBlockData(t_RemoteNtNetBlk *pBlockData, ushort size);
										// ホストから受信したNT-NETデータのブロック数を取得
extern	int		RAUdata_GetRcvNtDataBlkNum(void);
										// ホストから受信したNT-NETデータを取得
extern	ushort	RAUdata_GetRcvNtBlkData(uchar *data, uchar blkno);
										// ホストから受信したNT-NETデータを削除
extern	void	RAUdata_DelRcvNtData(void);
										// Timer start value get. (return 1ms counter)
extern	ulong	RAU_c_1mTim_Start( void );
										// Dopa command read from receive queue.
extern	uchar	RAU_DpaRcvQue_Read(uchar *pData, ushort *len, int port);
										// Dopa command set to send queue.
extern	void	DpaSndQue_Set(uchar *pData, ushort len, int port);
extern	void	reset_tsum(char *buff, long size);
extern	int		write_tsum(char *ntdata, int length);
										// ｼｽﾃﾑｴﾗｰ発生/解除関数
extern	void	RauSysErrSet( unsigned char n , unsigned char s );
extern	void	RAU_StopDownline(void);

extern	uchar	RAU_Credit_Enabale;				// クレジット使用設定
extern	uchar	Ntnet_Remote_Comm;
#define	_is_credit_only()	(Ntnet_Remote_Comm != 2 && RAU_Credit_Enabale != 0)
extern	uchar	Cre_SetSendNtData(const uchar* pData, ushort size);
extern	uchar*	Cre_GetSendNtData(void);
extern	uchar*	Cre_GetRcvNtData(void);
extern	eRAU_TABLEDATA_SET	Credit_SetSendNtBlockData(t_RemoteNtNetBlk *pBlockData, ushort size);
extern	eRAU_TABLEDATA_SET	Credit_SetRcvNtData(uchar *pData, ushort uiDataLen, ushort uiTcpBlkNo, uchar ucTcpBlkSts);
extern	eRAU_TABLEDATA_SET	Credit_SetThroughData(uchar *pData, ushort wDataLen, uchar ucDirection);
extern	void	Credit_DelRcvNtData(void);
extern	int		Credit_GetRcvNtDataBlkNum(void);
extern	ushort	Credit_GetRcvNtBlkData(uchar *data, uchar blkno);
extern	RAU_NET_STATE	RAU_GetCreditNetState(void);
extern	void	Credit_SetSeqFlag(uchar ucRcvSeqFlag);
extern	uchar	Credit_GetSeqFlag(void);

typedef struct {
	uchar	TCPcnct_req;									// TCPコネクションリクエストフラグ
	uchar	CenterConnecting;								// 接続中フラグ 0:未接続 1:確立中 2:接続中(未使用)
	uchar	NetState;										// 回線状態
	uchar	TCPdiscnct_req;									// TCP切断要求フラグ
	ushort	connRetryCount;									// コネクションリトライカウンタ
//	T_RAU_BLKDATA	mainRecvData;							// 現在処理中のメインからの受信データ
} tCreditCtrl;
extern	tCreditCtrl CreditCtrl;

// クレジット用受信バッファ（対OPE層）
#define CREDIT_DATABUF_COUNT	3
typedef struct {
	uchar	ntDataBuf[CREDIT_DATABUF_COUNT][RAU_BLKSIZE];	// NT-NETデータバッファ
	ushort	readIndex;										// 読込みインデックス
	ushort	writeIndex;										// 書込みインデックス
	ushort	count;											// 格納データ数
} CreNtDataBuf;

extern	CreNtDataBuf	CreOpeSendNtDataBuf;				// RAUタスク送信バッファ（対OPE層）
extern	CreNtDataBuf	RecvCreditDt;							/* NE-NET受信ﾊﾞｯﾌｧ(ﾜｰｸ)			*/

#endif	// RAUDEF_H
