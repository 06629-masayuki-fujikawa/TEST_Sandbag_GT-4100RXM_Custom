#ifndef	___RAUH___
#define	___RAUH___
/*[]----------------------------------------------------------------------[]
 *|	filename: rau.h
 *[]----------------------------------------------------------------------[]
 *| summary	: RAUモジュール RAM共有化用ラッパー定義
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/

#include	"raudef.h"
#include	"rauIDproc.h"

#define RAU_IBW_TEMP_BUFF_SIZE	31044		// 970 * 32 + 4 (NT-DATA * 32 + Len + CRC)
#define RAU_HOST_TEMP_BUFF_SIZE	25224		// 970 * 26 + 4 (NT-DATA * 26 + Len + CRC)
#define RAU_SPECIAL_SEND_BUFF_SIZE	16494	// 970 * 17 + 4 (NT-DATA * 17 + Len + CRC) ※Ｔ合計･Ｔ小計･金銭管理データの遠隔ＰＣ送信用特殊バッファの必要サイズ
#define RAU_TABLE_DATA_MAX	31				// 25から31へ変更 #001

#define		RAU_IN_PARKING_SIZE		0			// 入庫データ			（ID20）
#define		RAU_OUT_PARKING_SIZE	0			// 出庫データ			（ID21）
#define		RAU_PAID_DATA22_SIZE	0			// 精算データ			（ID22）
#define		RAU_PAID_DATA23_SIZE	0			// 精算データ			（ID23）
#define		RAU_TOTAL_SIZE			37000		// T合計集計データ		（ID30～38・41）
#define		RAU_ERROR_SIZE			0			// エラーデータ			（ID120）
#define		RAU_ALARM_SIZE			0			// アラームデータ		（ID121）
#define		RAU_MONITOR_SIZE		0			// モニターデータ		（ID122）
#define		RAU_OPE_MONITOR_SIZE	0			// 操作モニターデータ	（ID123）
#define		RAU_COIN_TOTAL_SIZE		0			// コイン金庫集計データ	（ID131）
#define		RAU_MONEY_TORAL_SIZE	0			// 紙幣金庫集計データ	（ID133）
#define		RAU_PARKING_NUM_SIZE	0			// 駐車台数・満車データ	（ID236）
#define		RAU_AREA_NUM_SIZE		0			// 区画台数・満車データ	（ID237）
#define		RAU_MONEY_MANAGE_SIZE	0			// 金銭管理データ		（ID126）(176 + 7 + 4) * 32
#define		RAU_TURI_MANAGE_SIZE	0			// 釣銭管理集計データ	 (ID135)
#define		RAU_REMOTE_MONITOR_SIZE	0			// 遠隔監視データ		（ID125）
// GG120600(S) // Phase9 センター用端末情報データに受信電文リビジョンを追加
//#define		RAU_CENTER_TERM_INFO_SIZE		492	// センター用端末情報データ	（ID65）sizeof(_ctib)=246 * 2
#define		RAU_CENTER_TERM_INFO_SIZE		582	// センター用端末情報データ	（ID65）sizeof(_ctib)=292 * 2
// GG120600(E) // Phase9 センター用端末情報データに受信電文リビジョンを追加
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
#define		RAU_LONG_PARKING_INFO_SIZE		0	// 長期駐車情報データ	（ID61）
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
#define		RAU_SEND_THROUGH_DATA_SIZE		31168	// スルーデータ	（970 + 4）× 32
#define		RAU_RECEIVE_THROUGH_DATA_SIZE	25324	// スルーデータ	（970 + 4）× 26

#define		RAU_NTDATA_MAX			25222	//(970 * 26) + 2

#define		RAU_LOG_BUFF_SIZE		20000

/****************************************************************/
/*				 １byteのグローバル変数定義						*/
/****************************************************************/

#define		RAU_f_CommuTest_rq	RAU_Tm_CommuTest.bits0.f_rq	// 通信チェック用タイマの起動フラグ
#define		RAU_f_CommuTest_ov	RAU_Tm_CommuTest.bits0.f_ov	// 通信チェック用タイマのタイムアウトフラグ

#define		RAU_f_Ackwait_rq	RAU_Tm_Ackwait.bits0.f_rq	// 上り回線からデータ送信するときのＡＣＫ待ちタイマ起動フラグ
#define		RAU_f_Ackwait_ov	RAU_Tm_Ackwait.bits0.f_ov	// 上り回線からデータ送信するときのＡＣＫ待ちタイマタイムアウトフラグ
#define		RAU_f_Ackwait_reconnect_rq	RAU_Tm_Ackwait_reconnect.bits0.f_rq	// 上り回線からデータ送信するときの再接続タイマ起動フラグ
#define		RAU_f_Ackwait_reconnect_ov	RAU_Tm_Ackwait_reconnect.bits0.f_ov	// 上り回線からデータ送信するときの再接続タイマタイムアウトフラグ

#define		RAU_f_Ackwait2_rq	RAU_Tm_Ackwait2.bits0.f_rq	// 下り回線からデータ送信するときのＡＣＫ待ちタイマ起動フラグ
#define		RAU_f_Ackwait2_ov	RAU_Tm_Ackwait2.bits0.f_ov	// 下り回線からデータ送信するときのＡＣＫ待ちタイマタイムアウトフラグ
#define		Credit_Ackwait_rq	RAU_Tm_Ackwait_CRE.bits0.f_rq
#define		Credit_Ackwait_ov	RAU_Tm_Ackwait_CRE.bits0.f_ov
#define		Credit_Ackwait_reconnect_rq	RAU_Tm_Ackwait_reconnect_CRE.bits0.f_rq
#define		Credit_Ackwait_reconnect_ov	RAU_Tm_Ackwait_reconnect_CRE.bits0.f_ov

enum {
	RAU_IN_PARKING_TABLE,			// 入庫データ			（ID20）
	RAU_OUT_PARKING_TABLE,			// 出庫データ			（ID21）
	RAU_PAID_DATA22_TABLE,			// 精算データ			（ID22）
	RAU_PAID_DATA23_TABLE,			// 精算データ			（ID23）
	RAU_ERROR_TABLE,				// エラーデータ			（ID120）
	RAU_ALARM_TABLE,				// アラームデータ		（ID121）
	RAU_MONITOR_TABLE,				// モニターデータ		（ID122）
	RAU_OPE_MONITOR_TABLE,			// 操作モニターデータ	（ID123）
	RAU_COIN_TOTAL_TABLE,			// コイン金庫集計データ	（ID131）
	RAU_MONEY_TORAL_TABLE,			// 紙幣金庫集計データ	（ID133）
	RAU_PARKING_NUM_TABLE,			// 駐車台数・満車データ	（ID236）
	RAU_AREA_NUM_TABLE,				// 区画台数・満車データ	（ID237）
	RAU_TURI_MANAGE_TABLE,			// 釣銭管理集計データ	（ID135）
	RAU_TOTAL_TABLE,				// T合計集計データ		（ID30～38・41）
	RAU_GTOTAL_TABLE,				// GT合計集計データ		（ID42,43,44,45,46,48）
	RAU_MONEY_MANAGE_TABLE,			// 金銭管理データ		（ID126）
	RAU_REMOTE_MONITOR_TABLE,		// 遠隔監視データ		（ID125）
	RAU_CENTER_TERM_INFO_TABLE,		// センター用端末情報データ（ID65）
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	RAU_LONG_PARKING_INFO_TABLE,	// 長期駐車情報データ	(ID61）
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	RAU_SEND_THROUGH_DATA_TABLE,	// 送信スルーデータ		（ID 不定）
	RAU_RECEIVE_THROUGH_DATA_TABLE,	// 受信スルーデータ		（ID 不定）
	RAU_TABLE_MAX
};
#define	RAU_SEND_TABLE_MAX			(RAU_SEND_THROUGH_DATA_TABLE+1)		// 送信すべき各テーブルの情報(スルーデータ(下り回線)含む)

enum {
	RAU_ID20_IN_PARKING,			// ID20		入庫データ
	RAU_ID21_OUT_PARKING,			// ID21		出庫データ
	RAU_ID22_PAID_DATA22,			// ID22		精算データ
	RAU_ID23_PAID_DATA23,			// ID23		精算データ
	RAU_ID54_IN_PARKING,			// ID54		入庫データ
	RAU_ID55_OUT_PARKING,			// ID55		出庫データ
	RAU_ID56_PAID_DATA22,			// ID56		精算データ
	RAU_ID57_PAID_DATA23,			// ID57		精算データ
	RAU_ID120_ERROR,				// ID120	エラーデータ
	RAU_ID121_ALARM,				// ID121	アラームデータ
	RAU_ID122_MONITOR,				// ID122	モニタデータ
	RAU_ID123_OPE_MONITOR,			// ID123	操作モニタデータ
	RAU_ID131_COIN_TOTAL,			// ID131	コイン金庫集計合計データ
	RAU_ID133_MONEY_TORAL,			// ID133	紙幣金庫集計合計データ
	RAU_ID236_PARKING_NUM,			// ID236	駐車台数データ
	RAU_ID58_PARKING_NUM,			// ID58		Web用駐車台数データ
	RAU_ID237_AREA_NUM,				// ID237	区画台数・満車データ
	RAU_ID30_TOTAL,					// ID30		T合計集計データ
	RAU_ID31_TOTAL,					// ID31		T合計集計データ
	RAU_ID32_TOTAL,					// ID32		T合計集計データ
	RAU_ID33_TOTAL,					// ID33		T合計集計データ
	RAU_ID34_TOTAL,					// ID34		T合計集計データ
	RAU_ID35_TOTAL,					// ID35		T合計集計データ
	RAU_ID36_TOTAL,					// ID36		T合計集計データ
	RAU_ID37_TOTAL,					// ID37		T合計集計データ
	RAU_ID38_TOTAL,					// ID38		T合計集計データ
	RAU_ID41_TOTAL,					// ID41		T合計集計データ
	RAU_ID42_TOTAL,					// ID42		フォーマットRevNo.10 集計基本データ
	RAU_ID43_TOTAL,					// ID43		フォーマットRevNo.10 料金種別毎集計データ
	RAU_ID45_TOTAL,					// ID44		フォーマットRevNo.10 割引集計データ
	RAU_ID46_TOTAL,					// ID45		フォーマットRevNo.10 定期集計データ
	RAU_ID49_TOTAL,					// ID46		フォーマットRevNo.10 タイムレジ集計データ
	RAU_ID53_TOTAL,					// ID48		フォーマットRevNo.10 集計終了通知データ
	RAU_ID126_MONEY_MANAGE,			// ID126	金銭管理データ
	RAU_ID135_TURI_MANAGE,			// ID135	釣銭管理集計データ
	RAU_ID59_REMOTE_MONITOR,		// ID125	遠隔監視データ
	RAU_ID65_CENTER_TERM_INFO,		// ID65		センター用端末情報データ
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	RAU_ID61_LONG_PARKING_INFO,		// ID61		長期駐車情報データ
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	RAU_ID0_SEND_THROUGH_DATA,		// 送信スルーデータ		（ID 不定）
	RAU_ID0_RECEIVE_THROUGH_DATA,	// 受信スルーデータ		（ID 不定）
	RAU_ID_BUFF_MAX,
	RAU_ID_BUFF_NUM
};

typedef struct {
	uchar total[RAU_TOTAL_SIZE];						// T合計集計データ		（ID30～38・41）
	uchar center_term_info[RAU_CENTER_TERM_INFO_SIZE];	// センター用端末情報データ（ID65）
	uchar send_through_data[RAU_SEND_THROUGH_DATA_SIZE];		// 送信スルーデータ		（ID 不定）※送信のみ32blockに拡張 2006.08.31:m-onouchi
	uchar receive_through_data[RAU_RECEIVE_THROUGH_DATA_SIZE];	// 受信スルーデータ		（ID 不定）
}T_RAU_TABLE_DATA;

// タイマ定義
union RAU_time_area
{
	struct	tim_tag0
	{
		ushort	f_ov	 : 1  ;		/* ビット　１５		*/
		ushort	f_rq	 : 1  ;		/* ビット　１４		*/
		ushort	bit_0_13 : 14 ;		/* ビット０～１３	*/

	} bits0;
	ushort	tm;
};
typedef	union	RAU_time_area	RAU_TIME_AREA;	/* タイマ・エリア		*/

typedef struct {
	ulong	size;
	uchar	id;
} RAU_ID_INIT_INFO;

typedef struct _tsumm {
	struct _ntman	man;
	struct {
		char	*sendtop;
	} dirty;
} _tsum_man;

typedef struct _ctim {
	char	*top;		/* center term info buffer top */
	char	*bottom;	/* center term info buffer bottom */
	char	*freeq;		/* free buffer queue */
	char	*sendq;		/* send buffer queue */
	char	*readq;		/* send buffer queue */
} _centerterminfo_man;

/* ソース上シンボル名の解決 */
/* DataTable.c */
extern		T_RAU_TABLE_DATA		RAU_table_data;
extern		RAU_DATA_TABLE_INFO		rau_data_table_info[RAU_TABLE_MAX];
extern		int						rau_table_crear_flag;
extern		uchar					*RAU_table_top[RAU_TABLE_MAX];
/* Host.c */
extern		uchar					RAU_ucSendSeqFlag;						// 送信シーケンスフラグ
extern		uchar					RAU_ucReceiveSeqFlag;					// 受信シーケンスフラグ
extern		uchar					RAU_ucTcpBlockSts;						// TCPブロックステータス
extern		uchar					RAU_ucTcpBlockSts2;						// 下り回線送信データ用ＴＣＰブロックステータス
extern		ushort					RAU_uiTcpBlockNo;						// TCPブロックナンバー
extern		ushort					RAU_uiTcpBlockNo2;						// 下り回線送信データ用ＴＣＰブロック番号
extern		RAU_SEND_REQUEST		RAUhost_SendRequest;					// 送信要求データ
extern		uchar					RAUhost_SendRequestFlag;				// 送信要求データありフラグ
extern		uchar					RAUhost_Error01;						// エラーコード01発生状況
extern		T_SEND_NT_DATA			RAUhost_SendNtDataInfo;					// 送信対象のテーブルバッファから取得したNTデータの情報
extern		T_SEND_DATA_ID_INFO		*pRAUhost_SendIdInfo;					// 送信対象のテーブルバッファ情報の参照
extern		T_SEND_NT_DATA			RAUhost_SendNtThroughInfo;				// 下り回線から送信するスルーデータテーブルのＮＴデータ情報
extern		T_SEND_DATA_ID_INFO		*pRAUhost_SendThroughInfo;				// 下り回線から送信するスルーデータテーブルの情報
extern		uchar					RAU_f_RcvSeqCnt_rq;						// 受信シーケンスタイマーフラグ
extern		ushort					RAU_Tm_RcvSeqCnt;						// 受信シーケンスフラグキャンセルタイマー
extern		uchar					RAU_f_SndSeqCnt_rq;						// 受信シーケンスタイマーフラグ
extern		ushort					RAU_Tm_SndSeqCnt;						// 受信シーケンスフラグキャンセルタイマー
/* IDproc.c */
extern		uchar					RAU_special_table_send_buffer[RAU_SPECIAL_SEND_BUFF_SIZE];		// Ｔ合計･金銭管理データの遠隔ＰＣ送信用特殊バッファ
extern		RAU_COMMUNICATION_TEST	RemoteCheck_Buffer100_101;				// 通信チェック用バッファ(ＩＤ１００・１０１)
extern		RAU_COMMUNICATION_TEST	RemoteCheck_Buffer116_117;				// 通信チェック用バッファ(ＩＤ１１６・１１７)
extern		uchar					RauCT_SndReqID;							// 通信チェックデータ送信要求ＩＤ(0:要求なし, 100:ＩＤ１００送信要求, 117:ＩＤ１１７送信要求)
extern		uchar					Rau_SedEnable;							// 送信許可
extern		uchar					RAU_ucLastNtBlockNum;					// 最後に受信したＮＴデータブロックナンバー
extern		ushort					RAU_uiArcReceiveLen;					// 受信したＮＴブロック長の合計
extern		ushort					RAU_uiLastTcpBlockNum;					// 最後に受信したＮＴデータブロックナンバー
extern		ushort					RAU_uiDopaReceiveLen;					// 受信したＮＴブロック長の合計
/* Que.c */
extern		T_SEND_DATA_ID_INFO		RAUque_SendDataInfo[RAU_SEND_TABLE_MAX];	// 送信すべき各テーブルの情報(スルーデータ(下り回線)含む)
extern		ushort					RAUque_CurrentSendData;
/* RAUinRam.c */
extern		ushort					RAU_Tm_Reset_t;							// 機器リセット出力タイマ
/* RAUmain.c */
extern		uchar					RAU_f_TmStart;							// 1=Start, 0=don't do process
/* Terminal.c */
extern		T_RAU_ARC_SNDBLK		RAUarc_SndNtBlk;
/* ram.h */
extern		uchar					RAU_uc_mode_h;							//	（HOST通信側）通信状態モード
extern		uchar					RAU_uc_mode_h2;							// 下り回線の通信状態モード(S_H_IDLE:アイドル, S_H_ACK:ＡＣＫ待ち)
extern		uchar					RAU_uc_retrycnt_h;						//	（HOST通信側）送信リトライカウンタ
extern		uchar					RAU_uc_retrycnt_h2;						// 下り回線用の送信リトライカウンタ
extern		uchar					RAU_uc_txdata_h_keep[26];				//	（HOST通信側）前回送信したデータを保存
extern		uchar					RAU_uc_txdata_h_keep2[26];				// 下り回線から送信したデータのＤｏＰａヘッダ部分を保持するバッファ
extern		uchar					RAU_uc_rcvretry_cnt;					// 下り回線からの同一データ受信回数カウンタ
extern		ushort					RAU_ui_txdlength_t;						//	（端末通信側）送信データ長（メイン用）
extern		ushort					RAU_ui_data_length_h;					//	（HOST通信側）受信バイト数カウンタ
extern		ushort					RAU_ui_txdlength_h;						//	（HOST通信側）送信データ長（メイン用）
extern		ushort					RAU_ui_txdlength_h2;					// 下り回線へ送信するデータの長さ
extern		ushort					RAU_ui_txdlength_h_keep;
extern		ushort					RAU_ui_txdlength_h_keep2;
extern		ulong					RAU_ui_seq_bak;							// シーケンシャル№バックアップ
extern		uchar					RAU_huc_rcv_work_t[RAU_RCV_MAX_T];		//	（端末通信側）受信テキストワークバッファ
extern		uchar					RAU_huc_txdata_h[RAU_SEND_MAX_H];		//	（HOST通信側）送信電文バッファ
extern		uchar					RAU_huc_txdata_h2[RAU_SEND_MAX_H];		// 下り回線の送信電文バッファ
extern		uchar					RAU_huc_rcv_work_h[RAU_RCV_MAX_H];		//	（HOST通信側）受信テキストワークバッファ
extern		uchar					RAU_huc_rcv_tmp[RAU_RCV_MAX_H];			//	（HOST通信側）受信テキストワークバッファ
extern		uchar					RAU_huc_rcv_buf[3][RAU_RCV_MAX_H];		//	（HOST通信側）受信テキストワークバッファ
extern		ushort					RAU_huc_rcv_len[3];						//	（HOST通信側）受信バイト数カウンタ
extern		RAU_TIME_AREA			RAU_Tm_CommuTest;						// 通信チェック用タイマ
extern		RAU_TIME_AREA			RAU_Tm_Ackwait;							// 上り回線からデータ送信するときのＡＣＫ待ちタイマ
extern		RAU_TIME_AREA			RAU_Tm_Ackwait2;						// 下り回線からデータ送信するときのＡＣＫ待ちタイマ
extern		uchar					RAU_uc_retrycnt_reconnect_h;			//	（HOST通信側）再接続リトライカウンタ
extern		RAU_TIME_AREA			RAU_Tm_Ackwait_reconnect;				// 上り回線からデータ送信するときの再接続タイマ
/* IDProc.h */
extern		ushort					RAU_ui_RAUDPA_seq;						// 遠隔Dopa用送信時シーケンシャル番号
extern		ushort					RAU_ui_RAUDPA_seq2;						// 下り回線送信データ用ＤｏＰａパケットシーケンシャル番号

extern		_tsum_man				RAU_tsum_man;
extern		_centerterminfo_man		RAU_centerterminfo_man;

extern		RAU_TIME_AREA			RAU_Tm_No_Action;				// 無通信タイマ(上り回線によるダミーデータ送信間隔)
#define		RAU_f_No_Action_rq		RAU_Tm_No_Action.bits0.f_rq
#define		RAU_f_No_Action_ov		RAU_Tm_No_Action.bits0.f_ov

extern		RAU_TIME_AREA			RAU_Tm_DISCONNECT;				// ＴＣＰ切断待ちタイマ
#define		RAU_f_discnnect_rq		RAU_Tm_DISCONNECT.bits0.f_rq
#define		RAU_f_discnnect_ov		RAU_Tm_DISCONNECT.bits0.f_ov

extern		RAU_TIME_AREA			RAU_Tm_Port_Watchdog;			// 下り回線通信監視タイマ(下り回線のＴＣＰ接続強制切断時間)
#define		RAU_f_Port_Watchdog_rq	RAU_Tm_Port_Watchdog.bits0.f_rq
#define		RAU_f_Port_Watchdog_ov	RAU_Tm_Port_Watchdog.bits0.f_ov

extern		RAU_TIME_AREA			RAU_Tm_TCP_TIME_WAIT;			// ＴＣＰコネクションＴＩＭＥ＿ＷＡＩＴ解除待ちタイマ
#define		RAU_f_TCPtime_wait_rq	RAU_Tm_TCP_TIME_WAIT.bits0.f_rq
#define		RAU_f_TCPtime_wait_ov	RAU_Tm_TCP_TIME_WAIT.bits0.f_ov

extern		RAU_TIME_AREA			RAU_Tm_data_rty;				// データ再送待ちタイマー(分) 100msベースの1minタイマー使用
#define		RAU_f_data_rty_rq		RAU_Tm_data_rty.bits0.f_rq
#define		RAU_f_data_rty_ov		RAU_Tm_data_rty.bits0.f_ov

extern		RAU_TIME_AREA			RAU_Tm_TCP_DISCNCT;				// TCPコネクション切断待ちタイマー
#define		RAU_f_TCPdiscnct_rq		RAU_Tm_TCP_DISCNCT.bits0.f_rq
#define		RAU_f_TCPdiscnct_ov		RAU_Tm_TCP_DISCNCT.bits0.f_ov

extern		uchar	RAU_temp_data[970];								// 各種通知用のテンポラリバッファ（１NTブロック分）
extern uchar				RAU_LogData[RAU_LOG_BUFF_SIZE];

extern	t_DPA_Ctrl	DPA_Credit_SndCtrl;						// クレジット回線の送信キュー
extern	t_DPA_Ctrl	DPA_Credit_RcvCtrl;						// クレジット回線の受信キュー

extern	uchar	Credit_Snd_Buf[RAU_BLKSIZE];			// (*T_SEND_DATA_ID_INFO)->pbuff_info.pc_AreaTopに指定する場所
extern	uchar	Credit_Rcv_Buf[RAU_BLKSIZE];			// １ブロックサイズ×４件分
extern	T_RAU_ARC_SNDBLK		Credit_SndNtBlk;

extern	uchar					RAU_uc_mode_CRE;						//	（クレジット通信側）通信状態モード
extern	uchar					RAU_uc_retrycnt_CRE;					//	（クレジット通信側）送信リトライカウンタ
extern	uchar					Credit_nakretry_cnt;
extern	uchar					Credit_rcvretry_cnt;
extern	uchar					RAU_uc_txdata_CRE_keep[26];				//	（クレジット通信側）前回送信したデータを保存
extern	uchar					RAU_ui_txdlength_CRE_keep;
extern	ushort					RAU_ui_data_length_CRE;					//	（クレジット通信側）受信バイト数カウンタ
extern	ushort					RAU_ui_txdlength_CRE;					//	（クレジット通信側）送信データ長（メイン用）
extern	uchar					RAU_huc_txdata_CRE[RAU_SEND_MAX_H];		//	（クレジット通信側）送信電文バッファ
extern	uchar					RAU_huc_rcv_work_CRE[RAU_RCV_MAX_H];	//	（クレジット通信側）受信テキストワークバッファ
extern	RAU_TIME_AREA			RAU_Tm_Ackwait_CRE;							// 上り回線からデータ送信するときのＡＣＫ待ちタイマ
extern	uchar					RAU_uc_retrycnt_disconnected_CRE;		//	（クレジット通信側）TCP切断時送信リトライカウンタ
extern	RAU_TIME_AREA			RAU_Tm_Ackwait_reconnect_CRE;			// クレジット回線からデータ送信するときのＡＣＫ待ち中の再接続タイマ

extern	uchar					Cre_ucLastNtBlockNum;				// 最後に受信したＮＴデータブロックナンバー
extern	ushort					Cre_uiArcReceiveLen;					// 受信したＮＴブロック長の合計
extern	ushort					Cre_uiLastTcpBlockNum;				// 最後に受信したＮＴデータブロックナンバー
extern	ushort					Cre_uiDopaReceiveLen;				// 受信したＮＴブロック長の合計

extern	ulong					Credit_seq_bak;							// シーケンシャル№バックアップ
extern	uchar					Credit_SeqFlag;					// 受信シーケンスフラグ
extern	uchar					Credit_TcpBlockSts;						// TCPブロックステータス
extern	ushort					Credit_TcpBlockNo;						// TCPブロックナンバー
extern	uchar					Credit_Error01;						// エラーコード01発生状況
extern	T_SEND_NT_DATA			Credit_SendNtDataInfo;				// 送信対象のテーブルバッファから取得したNTデータの情報
extern	T_SEND_DATA_ID_INFO		Credit_SendIdInfo;					// 送信すべきクレジットデータの情報
extern	RAU_DATA_TABLE_INFO		Credit_SndBuf_info;					// クレジットデータのバッファー情報
extern	RAU_DATA_TABLE_INFO		Credit_RcvBuf_info;					// クレジットデータのバッファー情報
extern	ushort					Credit_RAUDPA_seq;						// クレジット用送信時シーケンシャル番号
extern		RAU_TIME_AREA			CRE_tm_TCP_TIME_WAIT;			// ＴＣＰコネクションＴＩＭＥ＿ＷＡＩＴ解除待ちタイマ
#define		Credit_TCPtime_wait_rq	CRE_tm_TCP_TIME_WAIT.bits0.f_rq
#define		Credit_TCPtime_wait_ov	CRE_tm_TCP_TIME_WAIT.bits0.f_ov

extern	uchar					Credit_SeqReset_rq;					// シーケンスタイマーフラグ
extern	ushort					Credit_SeqReset_tm;					// シーケンスフラグキャンセルタイマー

#endif/* ___RAUH___ */
