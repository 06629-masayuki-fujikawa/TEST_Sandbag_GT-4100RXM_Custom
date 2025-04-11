#include	"rau.h"
#include	"raudef.h"
/*------------------------------------------------------------------------------*/
#pragma	section	_UNINIT3		/* "B":Uninitialized data area in external RAM2 */
/*------------------------------------------------------------------------------*/
// CS2:アドレス空間0x06039000-0x060FFFFF(1MB)

RAU_TIME_AREA	RAU_Tm_DoPaAtCom;				// DoPaｺﾏﾝﾄﾞﾀｲﾏ
RAU_TIME_AREA	RAU_Tm_No_Action;				// 無通信タイマ(上り回線によるダミーデータ送信間隔)
RAU_TIME_AREA	RAU_Tm_Port_Watchdog;			// 下り回線通信監視タイマ(下り回線のＴＣＰ接続強制切断時間)
RAU_TIME_AREA	RAU_Tm_TCP_CNCT;				// TCPコネクション接続待ちタイマー
RAU_TIME_AREA	RAU_Tm_DISCONNECT;				// コネクション切断待ちタイマー
RAU_TIME_AREA	RAU_Tm_TCP_TIME_WAIT;			// ＴＣＰコネクションＴＩＭＥ＿ＷＡＩＴ解除待ちタイマ
RAU_TIME_AREA	RAU_Tm_data_rty;				// データ再送待ちタイマー(分) 100msベースの1minタイマー使用
RAU_TIME_AREA	RAU_Tm_TCP_DISCNCT;				// TCPコネクション切断待ちタイマー

RAU_TIME_AREA	CRE_tm_TCP_TIME_WAIT;			// ＴＣＰコネクションＴＩＭＥ＿ＷＡＩＴ解除待ちタイマ
unsigned long	RAU_x_1mLifeTime;				/* timer counter. up to every 10ms */

t_DPA_Ctrl	DPA2_Snd_Ctrl;						// 下り回線の送信キュー
t_DPA2_Ctrl	DPA2_Rcv_Ctrl;						// 上り回線の受信キュー

t_DPA_Ctrl	DPA_Snd_Ctrl;						// 上り回線の送信キュー
t_DPA_Ctrl	DPA_Rcv_Ctrl;						// 下り回線の受信キュー

uchar		RAU_temp_data[970];					// 各種通知用のテンポラリバッファ（１NTブロック分）


/* DataTable.c */
T_RAU_TABLE_DATA		RAU_table_data;
RAU_DATA_TABLE_INFO		rau_data_table_info[RAU_TABLE_MAX];
int						rau_table_crear_flag;
uchar					*RAU_table_top[RAU_TABLE_MAX];
/* Host.c */
uchar					RAU_ucSendSeqFlag;						// 送信シーケンスフラグ
uchar					RAU_ucReceiveSeqFlag;					// 受信シーケンスフラグ
uchar					RAU_ucTcpBlockSts;						// TCPブロックステータス
uchar					RAU_ucTcpBlockSts2;						// 下り回線送信データ用ＴＣＰブロックステータス
ushort					RAU_uiTcpBlockNo;						// TCPブロックナンバー
ushort					RAU_uiTcpBlockNo2;						// 下り回線送信データ用ＴＣＰブロック番号
RAU_SEND_REQUEST		RAUhost_SendRequest;					// 送信要求データ
uchar					RAUhost_SendRequestFlag;				// 送信要求データありフラグ
uchar					RAUhost_Error01;						// エラーコード01発生状況
T_SEND_NT_DATA			RAUhost_SendNtDataInfo;					// 送信対象のテーブルバッファから取得したNTデータの情報
T_SEND_DATA_ID_INFO		*pRAUhost_SendIdInfo;					// 送信対象のテーブルバッファ情報の参照
T_SEND_NT_DATA			RAUhost_SendNtThroughInfo;				// 下り回線から送信するスルーデータテーブルのＮＴデータ情報
T_SEND_DATA_ID_INFO		*pRAUhost_SendThroughInfo;				// 下り回線から送信するスルーデータテーブルの情報
uchar					RAU_f_RcvSeqCnt_rq;						// 受信シーケンスタイマーフラグ
ushort					RAU_Tm_RcvSeqCnt;						// 受信シーケンスフラグキャンセルタイマー
uchar					RAU_f_SndSeqCnt_rq;						// 受信シーケンスタイマーフラグ
ushort					RAU_Tm_SndSeqCnt;						// 受信シーケンスフラグキャンセルタイマー
/* IDproc.c */
uchar					RAU_special_table_send_buffer[RAU_SPECIAL_SEND_BUFF_SIZE];		// Ｔ合計･金銭管理データの遠隔ＰＣ送信用特殊バッファ
RAU_COMMUNICATION_TEST	RemoteCheck_Buffer100_101;				// 通信チェック用バッファ(ＩＤ１００・１０１)
RAU_COMMUNICATION_TEST	RemoteCheck_Buffer116_117;				// 通信チェック用バッファ(ＩＤ１１６・１１７)
uchar					RauCT_SndReqID;							// 通信チェックデータ送信要求ＩＤ(0:要求なし, 100:ＩＤ１００送信要求, 117:ＩＤ１１７送信要求)
uchar					Rau_SedEnable;							// 送信許可
uchar					RAU_ucLastNtBlockNum;					// 最後に受信したＮＴデータブロックナンバー
ushort					RAU_uiArcReceiveLen;					// 受信したＮＴブロック長の合計
ushort					RAU_uiLastTcpBlockNum;					// 最後に受信したＮＴデータブロックナンバー
ushort					RAU_uiDopaReceiveLen;					// 受信したＮＴブロック長の合計
/* Que.c */
T_SEND_DATA_ID_INFO		RAUque_SendDataInfo[RAU_SEND_TABLE_MAX];	// 送信すべき各テーブルの情報(スルーデータ(下り回線)含む)
ushort					RAUque_CurrentSendData;
/* RAUinRam.c */
ushort					RAU_Tm_Reset_t;							// 機器リセット出力タイマ
/* RAUmain.c */
uchar					RAU_f_TmStart;							// 1=Start, 0=don't do process
/* Terminal.c */
T_RAU_ARC_SNDBLK		RAUarc_SndNtBlk;
/* ram.h */
uchar					RAU_uc_mode_h;							//	（HOST通信側）通信状態モード
uchar					RAU_uc_mode_h2;							// 下り回線の通信状態モード(S_H_IDLE:アイドル, S_H_ACK:ＡＣＫ待ち)
uchar					RAU_uc_retrycnt_h;						//	（HOST通信側）送信リトライカウンタ
uchar					RAU_uc_retrycnt_h2;						// 下り回線用の送信リトライカウンタ
uchar					RAU_uc_txdata_h_keep[26];				//	（HOST通信側）前回送信したデータを保存
uchar					RAU_uc_txdata_h_keep2[26];				// 下り回線から送信したデータのＤｏＰａヘッダ部分を保持するバッファ
uchar					RAU_uc_rcvretry_cnt;					// 下り回線からの同一データ受信回数カウンタ
ushort					RAU_ui_txdlength_t;						//	（端末通信側）送信データ長（メイン用）
ushort					RAU_ui_data_length_h;					//	（HOST通信側）受信バイト数カウンタ
ushort					RAU_ui_txdlength_h;						//	（HOST通信側）送信データ長（メイン用）
ushort					RAU_ui_txdlength_h2;					// 下り回線へ送信するデータの長さ
ushort					RAU_ui_txdlength_h_keep;
ushort					RAU_ui_txdlength_h_keep2;
ulong					RAU_ui_seq_bak;							// シーケンシャル№バックアップ
uchar					RAU_huc_rcv_work_t[RAU_RCV_MAX_T];		//	（端末通信側）受信テキストワークバッファ
uchar					RAU_huc_txdata_h[RAU_SEND_MAX_H];		//	（HOST通信側）送信電文バッファ
uchar					RAU_huc_txdata_h2[RAU_SEND_MAX_H];		// 下り回線の送信電文バッファ
uchar					RAU_huc_rcv_work_h[RAU_RCV_MAX_H];		//	（HOST通信側）受信テキストワークバッファ
uchar					RAU_huc_rcv_tmp[RAU_RCV_MAX_H];			//	（HOST通信側）受信テキストワークバッファ
uchar					RAU_huc_rcv_buf[3][RAU_RCV_MAX_H];		//	（HOST通信側）受信テキストワークバッファ
ushort					RAU_huc_rcv_len[3];						//	（HOST通信側）受信バイト数カウンタ
RAU_TIME_AREA			RAU_Tm_CommuTest;						// 通信チェック用タイマ
RAU_TIME_AREA			RAU_Tm_Ackwait;							// 上り回線からデータ送信するときのＡＣＫ待ちタイマ
RAU_TIME_AREA			RAU_Tm_Ackwait2;						// 下り回線からデータ送信するときのＡＣＫ待ちタイマ
uchar					RAU_uc_retrycnt_reconnect_h;			//	（HOST通信側）再接続リトライカウンタ
RAU_TIME_AREA			RAU_Tm_Ackwait_reconnect;				// 上り回線からデータ送信するときの再接続待ちタイマ
t_DPA_Ctrl	DPA_Credit_SndCtrl;						// クレジット回線の送信キュー
t_DPA_Ctrl	DPA_Credit_RcvCtrl;						// クレジット回線の受信キュー

uchar	Credit_Snd_Buf[RAU_BLKSIZE];			// (*T_SEND_DATA_ID_INFO)->pbuff_info.pc_AreaTopに指定する場所
uchar	Credit_Rcv_Buf[RAU_BLKSIZE];
T_RAU_ARC_SNDBLK		Credit_SndNtBlk;

uchar					Cre_ucLastNtBlockNum;				// 最後に受信したＮＴデータブロックナンバー
ushort					Cre_uiArcReceiveLen;					// 受信したＮＴブロック長の合計
ushort					Cre_uiLastTcpBlockNum;				// 最後に受信したＮＴデータブロックナンバー
ushort					Cre_uiDopaReceiveLen;				// 受信したＮＴブロック長の合計
uchar					RAU_uc_mode_CRE;						//	（クレジット通信側）通信状態モード
uchar					RAU_uc_retrycnt_CRE;					//	（クレジット通信側）送信リトライカウンタ
uchar					Credit_nakretry_cnt;
uchar					RAU_uc_txdata_CRE_keep[26];				//	（クレジット通信側）前回送信したデータを保存
uchar					RAU_ui_txdlength_CRE_keep;
ushort					RAU_ui_data_length_CRE;					//	（クレジット通信側）受信バイト数カウンタ
ushort					RAU_ui_txdlength_CRE;					//	（クレジット通信側）送信データ長（メイン用）
uchar					RAU_huc_txdata_CRE[RAU_SEND_MAX_H];		//	（クレジット通信側）送信電文バッファ
uchar					RAU_huc_rcv_work_CRE[RAU_RCV_MAX_H];	//	（クレジット通信側）受信テキストワークバッファ
RAU_TIME_AREA			RAU_Tm_Ackwait_CRE;							// 上り回線からデータ送信するときのＡＣＫ待ちタイマ
uchar					RAU_uc_retrycnt_disconnected_CRE;		//	（クレジット通信側）TCP切断時送信リトライカウンタ
RAU_TIME_AREA			RAU_Tm_Ackwait_reconnect_CRE;			// クレジット回線からデータ送信するときのＡＣＫ待ち中の再接続タイマ

ulong					Credit_seq_bak;							// シーケンシャル№バックアップ
uchar					Credit_SeqFlag;					// 受信シーケンスフラグ
uchar					Credit_TcpBlockSts;						// TCPブロックステータス
ushort					Credit_TcpBlockNo;						// TCPブロックナンバー
uchar					Credit_Error01;						// エラーコード01発生状況
T_SEND_NT_DATA			Credit_SendNtDataInfo;				// 送信対象のテーブルバッファから取得したNTデータの情報
T_SEND_DATA_ID_INFO		Credit_SendIdInfo;					// 送信すべきクレジットデータの情報
RAU_DATA_TABLE_INFO		Credit_SndBuf_info;					// クレジットデータのバッファー情報
RAU_DATA_TABLE_INFO		Credit_RcvBuf_info;					// クレジットデータのバッファー情報
ushort					Credit_RAUDPA_seq;						// クレジット用送信時シーケンシャル番号
uchar					Credit_rcvretry_cnt;					// 下り回線からの同一データ受信回数カウンタ
uchar					Credit_SeqReset_rq;						// シーケンスタイマーフラグ
ushort					Credit_SeqReset_tm;						// シーケンスフラグキャンセルタイマー
/* IDProc.h */
ushort					RAU_ui_RAUDPA_seq;						// 遠隔Dopa用送信時シーケンシャル番号
ushort					RAU_ui_RAUDPA_seq2;						// 下り回線送信データ用ＤｏＰａパケットシーケンシャル番号

_tsum_man				RAU_tsum_man;
_centerterminfo_man		RAU_centerterminfo_man;
