/*[]----------------------------------------------------------------------[]*/
/*|		RAU task                    									   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  S.Takahashi                                             |*/
/*| Date        :  2012-09-05                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/

#include	<string.h>
#include    <trsocket.h>
#include    <trmacro.h>
#include    <trtype.h>
#include    <trproto.h>
#include	"system.h"
#include	"common.h"
#include	"prm_tbl.h"
#include	"ope_def.h"
#include	"rau.h"
#include	"raudef.h"
#include	"rauIDproc.h"

/*----------------------------------*/
/*		function external define	*/
/*----------------------------------*/
extern	void	RAU_FuncStop( void );
extern	void	RAU_FuncMain( void );
extern	void	RAU_IniDatSave( void );
extern	void	RAU_IniDatUpdate( void );
extern	void	RAU_FuncStart( void );
extern	void	Credit_main(void);
/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
void	RAU_init( void );
void	RAU_ConfigInit( void );
void	RAU_main( void );
void	Credit_Init(void);

/*[]----------------------------------------------------------------------[]*/
/*|             RAU_Get_Tm_Reset_t()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|     return value of "RAU_Tm_Reset_t"                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      :  machida.k                                               |*/
/*| Date        :  2005-12-20                                              |*/
/*| Update      :                                                          |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
ushort	RAU_Get_Tm_Reset_t(void)
{
	return RAU_Tm_Reset_t;
}

/*[]----------------------------------------------------------------------[]*/
/*|             RAU_Set_Tm_Reset_t()                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*|     set "RAU_Tm_Reset_t"                                                   |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	RAU_Set_Tm_Reset_t(ushort val)
{
	RAU_Tm_Reset_t = val;
}

/*[]----------------------------------------------------------------------[]*/
/*|             RAU_init()  	                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*|          RAU task initial routine                                      |*/
/*[]------------------------------------- Copyright(C) 2003 AMANO Corp.---[]*/
void	RAU_init( void )
{
	short	Lno;								// ログID
	short	target;								// ログターゲット
	RAU_ConfigInit();
	
	/* area initialize */

	/* DataTable.c */
	memset(&RAU_table_data, 0, sizeof(RAU_table_data));
	memset(&rau_data_table_info , 0, sizeof(rau_data_table_info));
	rau_table_crear_flag = 0;
	memset(&RAU_table_top, 0, sizeof(RAU_table_top));
	/* Host.c */
	RAU_ucSendSeqFlag = 0;						// 送信シーケンスフラグ
	RAU_ucReceiveSeqFlag = 0;					// 受信シーケンスフラグ
	RAU_ucTcpBlockSts = 0;						// TCPブロックステータス
	RAU_ucTcpBlockSts2 = 0;						// 下り回線送信データ用ＴＣＰブロックステータス
	RAU_uiTcpBlockNo = 0;						// TCPブロックナンバー
	RAU_uiTcpBlockNo2 = 0;						// 下り回線送信データ用ＴＣＰブロック番号
	memset(&RAUhost_SendRequest, 0, sizeof(RAUhost_SendRequest));					// 送信要求データ
	RAUhost_SendRequestFlag = 0;				// 送信要求データありフラグ
	RAUhost_Error01 = 0;						// エラーコード01発生状況
	memset(&RAUhost_SendNtDataInfo, 0, sizeof(RAUhost_SendNtDataInfo));				// 送信対象のテーブルバッファから取得したNTデータの情報
	pRAUhost_SendIdInfo = NULL;					// 送信対象のテーブルバッファ情報の参照
	memset(&RAUhost_SendNtThroughInfo, 0, sizeof(RAUhost_SendNtThroughInfo));		// 下り回線から送信するスルーデータテーブルのＮＴデータ情報
	pRAUhost_SendThroughInfo = NULL;				// 下り回線から送信するスルーデータテーブルの情報
	RAU_f_RcvSeqCnt_rq = 0;						// 受信シーケンスタイマーフラグ
	RAU_Tm_RcvSeqCnt = 0;						// 受信シーケンスフラグキャンセルタイマー
	RAU_f_SndSeqCnt_rq = 0;						// 受信シーケンスタイマーフラグ
	RAU_Tm_SndSeqCnt = 0;						// 受信シーケンスフラグキャンセルタイマー
	/* IDproc.c */
	memset(&RAU_special_table_send_buffer, 0, sizeof(RAU_special_table_send_buffer));		// Ｔ合計･金銭管理データの遠隔ＰＣ送信用特殊バッファ
	memset(&RemoteCheck_Buffer100_101, 0, sizeof(RemoteCheck_Buffer100_101));				// 通信チェック用バッファ(ＩＤ１００・１０１)
	memset(&RemoteCheck_Buffer116_117, 0, sizeof(RemoteCheck_Buffer116_117));				// 通信チェック用バッファ(ＩＤ１１６・１１７)
	RauCT_SndReqID = 0;							// 通信チェックデータ送信要求ＩＤ(0:要求なし, 100:ＩＤ１００送信要求, 117:ＩＤ１１７送信要求)
	Rau_SedEnable = 0;							// 送信許可
	RAU_ucLastNtBlockNum = 0;					// 最後に受信したＮＴデータブロックナンバー
	RAU_uiArcReceiveLen = 0;					// 受信したＮＴブロック長の合計
	RAU_uiLastTcpBlockNum = 0;					// 最後に受信したＮＴデータブロックナンバー
	RAU_uiDopaReceiveLen = 0;					// 受信したＮＴブロック長の合計
	/* Que.c */
	memset(&RAUque_SendDataInfo, 0, sizeof(RAUque_SendDataInfo));				// 送信すべき各テーブルの情報(0:入庫テーブル, 1:出庫テーブル, ･･･ 14:スルーデータテーブル)
	RAUque_CurrentSendData = 0;
	/* RAUinRam.c */
	RAU_Tm_Reset_t = 0;							// 機器リセット出力タイマ
	/* RAUmain.c */
	RAU_f_TmStart = 0;							// 1=Start, 0=don't do process
	/* Terminal.c */
	memset(&RAUarc_SndNtBlk, 0, sizeof(RAUarc_SndNtBlk));
	/* ram.h */
	RAU_uc_mode_h = 0;							//	（HOST通信側）通信状態モード
	RAU_uc_mode_h2 = 0;							// 下り回線の通信状態モード(S_H_IDLE:アイドル, S_H_ACK:ＡＣＫ待ち)
	RAU_uc_retrycnt_h = 0;						//	（HOST通信側）送信リトライカウンタ
	RAU_uc_retrycnt_h2 = 0;						// 下り回線用の送信リトライカウンタ
	memset(&RAU_uc_txdata_h_keep, 0, sizeof(RAU_uc_txdata_h_keep));				//	（HOST通信側）前回送信したデータを保存
	memset(&RAU_uc_txdata_h_keep2, 0, sizeof(RAU_uc_txdata_h_keep2));				// 下り回線から送信したデータのＤｏＰａヘッダ部分を保持するバッファ
	RAU_uc_rcvretry_cnt = 0;					// 下り回線からの同一データ受信回数カウンタ
	RAU_ui_txdlength_t = 0;						//	（端末通信側）送信データ長（メイン用）
	RAU_ui_data_length_h = 0;					//	（HOST通信側）受信バイト数カウンタ
	RAU_ui_txdlength_h = 0;						//	（HOST通信側）送信データ長（メイン用）
	RAU_ui_txdlength_h2 = 0;					// 下り回線へ送信するデータの長さ
	RAU_ui_txdlength_h_keep = 0;
	RAU_ui_txdlength_h_keep2 = 0;
	RAU_ui_seq_bak = 0;							// シーケンシャル№バックアップ
	memset(&RAU_huc_rcv_work_t, 0, sizeof(RAU_huc_rcv_work_t));		//	（端末通信側）受信テキストワークバッファ
	memset(&RAU_huc_txdata_h, 0, sizeof(RAU_huc_txdata_h));			//	（HOST通信側）送信電文バッファ
	memset(&RAU_huc_txdata_h2, 0, sizeof(RAU_huc_txdata_h2));		// 下り回線の送信電文バッファ
	memset(&RAU_huc_rcv_work_h, 0, sizeof(RAU_huc_rcv_work_h));		//	（HOST通信側）受信テキストワークバッファ
	memset(&RAU_Tm_CommuTest, 0, sizeof(RAU_Tm_CommuTest));			// 通信チェック用タイマ
	memset(&RAU_Tm_Ackwait, 0, sizeof(RAU_Tm_Ackwait));				// 上り回線からデータ送信するときのＡＣＫ待ちタイマ
	memset(&RAU_Tm_Ackwait2, 0, sizeof(RAU_Tm_Ackwait2));			// 下り回線からデータ送信するときのＡＣＫ待ちタイマ
	RAU_uc_retrycnt_reconnect_h = 0;			//	（HOST通信側）再接続リトライカウンタ
	memset(&RAU_Tm_Ackwait_reconnect, 0, sizeof(RAU_Tm_Ackwait_reconnect));				// 上り回線からデータ送信するときの再接続待ちタイマ
	/* IDProc.h */
	RAU_ui_RAUDPA_seq = 0;						// 遠隔Dopa用送信時シーケンシャル番号
	RAU_ui_RAUDPA_seq2 = 0;						// 下り回線送信データ用ＤｏＰａパケットシーケンシャル番号

	memset(&RAU_tsum_man, 0, sizeof(RAU_tsum_man));

	RAU_ui_RAUDPA_seq = 0;					// 遠隔Dopa用テーブルデータ送信時シーケンシャル番号クリア
	RAU_ui_RAUDPA_seq2 = 0;					// 遠隔Dopa用スルーデータ送信時シーケンシャル番号クリア
	RAU_ui_seq_bak = 0;						// シーケンシャル№バックアップ
	
	memset(&RauCtrl, 0, sizeof(RauCtrl));	// 制御テーブルのクリア
	memset(&CreditCtrl, 0, sizeof(CreditCtrl));	// 制御テーブルのクリア

	RAUarc_SndNtBlk.arcblknum		= RAUARC_NODATA;
	RAUarc_SndNtBlk.ntblknum		= RAUARC_NODATA;
	RAUarc_SndNtBlk.retry = 0;
	RAUarc_SndNtBlk.ntblkno_retry = 0;
//--	RAUarc_ES_OiBan = 0xFF;
	
	memset(&RauOpeSendNtDataBuf, 0, sizeof(RauOpeSendNtDataBuf));	// RAUタスク送信バッファ（対OPE層）
	memset(&RauOpeRecvNtDataBuf, 0, sizeof(RauOpeRecvNtDataBuf));	// RAUタスク受信バッファ（対OPE層）

	RAUhost_SetSndSeqFlag(RAU_SND_SEQ_FLG_NORMAL);	// 送信シーケンス０(ＨＯＳＴへのテーブル送信待機状態)
	RAUhost_SetRcvSeqFlag(RAU_RCV_SEQ_FLG_NORMAL);	// 受信シーケンス０(通常状態)
	RAUhost_ClearSendRequeat();
	
	memset(&DPA2_Snd_Ctrl, 0,sizeof(DPA2_Snd_Ctrl));	// 下り回線の送信キュー
	memset(&DPA2_Rcv_Ctrl, 0, sizeof(DPA2_Rcv_Ctrl));	// 上り回線の受信キュー
	memset(&DPA_Snd_Ctrl, 0, sizeof(DPA_Snd_Ctrl));		// 上り回線の送信キュー
	memset(&DPA_Rcv_Ctrl, 0, sizeof(DPA_Rcv_Ctrl));		// 下り回線の受信キュー
	
	// タイマ関連の初期化
	RAU_Tm_No_Action.tm = 0;
	RAU_Tm_Port_Watchdog.tm = 0;
	RAU_Tm_DISCONNECT.tm = 0;
	RAU_Tm_TCP_TIME_WAIT.tm = 0;
	RAU_f_TCPtime_wait_ov = 1;
	RAU_Tm_data_rty.tm = 0;
	RAU_f_data_rty_ov = 1;
	RAU_Tm_TCP_DISCNCT.tm = 0;

	RAU_data_table_init();

	init_tsum((char*)&RAU_table_data.total, RAU_TOTAL_SIZE);
	init_center_term_info((char*)&RAU_table_data.center_term_info);

	RAU_f_TmStart = 1;						/* timer process start */
	// ログのニアフルしきい値を設定
	// ToDo: 暫定でRAUタスクで行う
	for (Lno = 0; Lno < eLOG_MAX; ++Lno) {
		for (target = 0; target < eLOG_TARGET_MAX; ++target) {
			LOG_DAT[Lno].nearFull[target].NearFullMaximum = Ope_Log_GetNearFullCount(Lno);
		}
	}
}

/*[]----------------------------------------------------------------------[]
 *|	name	: RAU_ConfigInit
 *[]----------------------------------------------------------------------[]
 *| summary	: 共通パラメータ関連する設定を初期化する
 *[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	RAU_ConfigInit( void )
{
	short	s_work;
	ushort	us_work;
	char	ip[20];								// IPアドレス変換用
	
	memset(&RauConfig, 0, sizeof(RauConfig));
	
	RauConfig.id22_23_tel_flg 	= prm_get(COM_PRM, S_NTN, 63, 1, 5);	/* ＩＤ自動発信フラグ  （精算データ）*/
	RauConfig.id30_41_tel_flg 	= prm_get(COM_PRM, S_NTN, 63, 1, 4);	/* ＩＤ自動発信フラグ  （集計データ）*/
	RauConfig.id121_tel_flg 	= prm_get(COM_PRM, S_NTN, 63, 1, 2);	/* ＩＤ自動発信フラグ  （アラームデータ）*/
	RauConfig.id123_tel_flg 	= prm_get(COM_PRM, S_NTN, 64, 1, 6);	/* ＩＤ自動発信フラグ  （操作モニターデータ）*/
	RauConfig.id120_tel_flg 	= prm_get(COM_PRM, S_NTN, 63, 1, 3);	/* ＩＤ自動発信フラグ  （エラーデータ）*/
	RauConfig.id20_21_tel_flg 	= prm_get(COM_PRM, S_NTN, 63, 1, 6);	/* ＩＤ自動発信フラグ  （入庫出庫データ）*/
	RauConfig.id122_tel_flg 	= prm_get(COM_PRM, S_NTN, 63, 1, 1);	/* ＩＤ自動発信フラグ  （モニターデータ）*/
	RauConfig.id131_tel_flg 	= prm_get(COM_PRM, S_NTN, 64, 1, 5);	/* ＩＤ自動発信フラグ  （コイン金庫集計データ）*/
	RauConfig.id133_tel_flg 	= prm_get(COM_PRM, S_NTN, 64, 1, 4);	/* ＩＤ自動発信フラグ  （紙幣金庫集計データ）*/
	RauConfig.id236_tel_flg 	= prm_get(COM_PRM, S_NTN, 64, 1, 3);	/* ＩＤ自動発信フラグ  （駐車台数データ）*/
	RauConfig.id237_tel_flg 	= 0;									/* ＩＤ自動発信フラグ  （区画台数・満車データ）*/
	RauConfig.id126_tel_flg 	= prm_get(COM_PRM, S_NTN, 64, 1, 1);	/* ＩＤ自動発信フラグ  （金銭管理データ）*/
	RauConfig.id135_tel_flg 	= prm_get(COM_PRM, S_NTN, 64, 1, 2);	/* ＩＤ自動発信フラグ  （金銭管理集計データ）*/
	// @todo 設定がないため、1=即時送信をセット
	RauConfig.id125_tel_flg 	= 1;									/* ＩＤ自動発信フラグ  （遠隔監視データ）*/
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	// 即時送信固定
	RauConfig.id61_tel_flg 	= 1;										/* ＩＤ自動発信フラグ  （長期駐車情報データ）*/
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)

	RauConfig.id22_23_mask_flg 	= prm_get(COM_PRM, S_NTN, 61, 1, 5);	/* 送信マスク設定  （精算データ）*/
	RauConfig.id30_41_mask_flg 	= prm_get(COM_PRM, S_NTN, 61, 1, 4);	/* 送信マスク設定  （集計データ）*/
	RauConfig.id121_mask_flg 	= prm_get(COM_PRM, S_NTN, 61, 1, 2);	/* 送信マスク設定  （アラームデータ）*/
	RauConfig.id123_mask_flg 	= prm_get(COM_PRM, S_NTN, 62, 1, 6);	/* 送信マスク設定  （操作モニターデータ）*/
	RauConfig.id120_mask_flg 	= prm_get(COM_PRM, S_NTN, 61, 1, 3);	/* 送信マスク設定  （エラーデータ）*/
	RauConfig.id20_21_mask_flg 	= prm_get(COM_PRM, S_NTN, 61, 1, 6);	/* 送信マスク設定  （入庫出庫データ）*/
	RauConfig.id122_mask_flg 	= prm_get(COM_PRM, S_NTN, 61, 1, 1);	/* 送信マスク設定  （モニターデータ）*/
	RauConfig.id131_mask_flg 	= prm_get(COM_PRM, S_NTN, 62, 1, 5);	/* 送信マスク設定  （コイン金庫集計データ）*/
	RauConfig.id133_mask_flg 	= prm_get(COM_PRM, S_NTN, 62, 1, 4);	/* 送信マスク設定  （紙幣金庫集計データ）*/
	RauConfig.id236_mask_flg 	= prm_get(COM_PRM, S_NTN, 62, 1, 3);	/* 送信マスク設定  （駐車台数データ）*/
	RauConfig.id237_mask_flg 	= 0;									/* 送信マスク設定  （区画台数・満車データ）*/
	RauConfig.id126_mask_flg 	= prm_get(COM_PRM, S_NTN, 62, 1, 1);	/* 送信マスク設定  （金銭管理データ）*/
	RauConfig.id135_mask_flg 	= prm_get(COM_PRM, S_NTN, 62, 1, 2);	/* 送信マスク設定  （釣銭管理集計データ）*/
	// @todo 設定がないため、0=送信するをセット
	RauConfig.id125_mask_flg 	= 0;									/* 送信マスク設定  （遠隔監視データ）*/
// MH322917(S) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)
	// 送信する固定
	RauConfig.id61_mask_flg 	= 0;									/* 送信マスク設定  （長期駐車情報データ）*/
// MH322917(E) A.Iiizumi 2018/09/21 長期駐車検出機能の拡張対応(電文対応)

	RauConfig.alarm_send_level		= prm_get(COM_PRM, S_NTN, 37, 1, 4);
	RauConfig.error_send_level		= prm_get(COM_PRM, S_NTN, 37, 1, 3);
	RauConfig.opemonitor_send_level	= prm_get(COM_PRM, S_NTN, 37, 1, 2);
	RauConfig.monitor_send_level	= prm_get(COM_PRM, S_NTN, 37, 1, 1);
	// @todo 0=レベル0以上のデータを送信にセット
	RauConfig.rmon_send_level		= 0;
	RauConfig.serverTypeSendTsum	= prm_get(COM_PRM, S_NTN, 26, 1, 3);	// サーバータイプ時の送信 0:しない 1:する

	RauConfig.deta_full_proc 	= prm_get(COM_PRM, S_CEN, 51, 1, 1);	/* データ処理方法 */
	RauConfig.IBW_RespWtime = 20;										/* 送信結果受信完了待ち時間（単位＝秒）*/
	RauConfig.system_select = 0;										/* システム選択 */
	RauConfig.modem_exist = (unsigned char)prm_get(COM_PRM, S_CEN, 51, 1, 3);
	if(prm_get(COM_PRM, S_PAY, 24, 1, 1) != 2){							// Rau使用条件取得
		if(RAU_Credit_Enabale == 2){
			RauConfig.modem_exist = 0;
		}
		else{
			RauConfig.modem_exist = 1;
		}
	}
	RauConfig.Dpa_proc_knd	 	= prm_get(COM_PRM, S_CEN, 51, 1, 2);	/* Dopa 処理区分 [0]=常時通信なし, [1]=常時通信あり */
	
	sprintf(ip, "%03d.%03d.%03d.%03d",
			prm_get(COM_PRM, S_CEN, 62, 3, 4),
			prm_get(COM_PRM, S_CEN, 62, 3, 1),
			prm_get(COM_PRM, S_CEN, 63, 3, 4),
			prm_get(COM_PRM, S_CEN, 63, 3, 1));
	RauConfig.Dpa_IP_h			= inet_addr(ip);						/* Dopa HOST局IPアドレス */

	RauConfig.Dpa_port_h		= prm_get(COM_PRM, S_CEN, 64, 5, 1);	/* Dopa HOST局ポート番号 */

	if(RauConfig.modem_exist == 0) {									// モデム
		sprintf(ip, "%03d.%03d.%03d.%03d",
				prm_get(COM_PRM, S_CEN, 65, 3, 4),
				prm_get(COM_PRM, S_CEN, 65, 3, 1),
				prm_get(COM_PRM, S_CEN, 66, 3, 4),
				prm_get(COM_PRM, S_CEN, 66, 3, 1));
	}
	else {																// Ethernet
		sprintf(ip, "%03d.%03d.%03d.%03d",
				prm_get(COM_PRM, S_MDL, 2, 3, 4),
				prm_get(COM_PRM, S_MDL, 2, 3, 1),
				prm_get(COM_PRM, S_MDL, 3, 3, 4),
				prm_get(COM_PRM, S_MDL, 3, 3, 1));
	}
	RauConfig.Dpa_IP_m 			= inet_addr(ip);						/* Dopa 自局IPアドレス */
	
	RauConfig.Dpa_port_m		= prm_get(COM_PRM, S_CEN, 67, 5, 1);	/* Dopa 自局ポート番号 */
	RauConfig.Dpa_nosnd_tm		= 0 - (short)prm_get(COM_PRM, S_CEN, 54, 4, 1);	/* Dopa 無通信タイマー(秒) */
	RauConfig.Dpa_cnct_rty_tm	= prm_get(COM_PRM, S_CEN, 53, 3, 4);	/* Dopa 再発呼待ちタイマー(秒) */
	RauConfig.Dpa_com_wait_tm	= 0 - (short)prm_get(COM_PRM, S_CEN, 68, 3, 1);	/* Dopa コマンド(ACK)待ちタイマー(秒) */
	us_work	= prm_get(COM_PRM, S_CEN, 75, 3, 1);						/* Dopa データ再送待ちタイマー(分) */
	if(us_work <= 240) {
		RauConfig.Dpa_data_rty_tm	= 0 - us_work;
	}
	else {
		RauConfig.Dpa_data_rty_tm	= (ushort)(0 - 240);
	}
	
	// TCPコネクション切断待ち(TIME_WAIT)タイマー(秒)
	s_work = (short)prm_get(COM_PRM, S_CEN, 69, 4, 1);
	if(RauConfig.modem_exist == 0) {									// モデムの場合は10秒以上とする
		if(s_work < 10) {
			s_work = 10;
		}
	}
	RauConfig.Dpa_discnct_tm	= 0 - (s_work * 10);					// 100ms単位に変換
	
	RauConfig.Dpa_cnct_rty_cn	= prm_get(COM_PRM, S_CEN, 53, 3, 1);	/* Dopa 再発呼回数 */
	RauConfig.Dpa_data_rty_cn	= prm_get(COM_PRM, S_CEN, 70, 1, 2);	/* Dopa データ再送回数(NAK,無応答) */
	RauConfig.Dpa_data_code		= 0;									/* Dopa HOST側電文コード [0]=文字コード, [1]=バイナリコード */
	RauConfig.Dpa_com_wait_reconnect_tm	= (ushort)( 0 - RAU_RECONNECT_INTERVAL );	/* Dopa 再接続待ちタイマー(秒) */
	RauConfig.Dpa_data_rty_cn_disconnected	= RAU_RECONNECT_MAX;		/* Dopa 再接続回数(TCP切断) */

	RauConfig.interrupt_proc = 0;										/* 中断要求時の処理 */
	RauConfig.interrupt_proc_block = 0;									/* 中断要求受信時の判定ブロック */
	RauConfig.Dpa_data_snd_rty_cnt = prm_get(COM_PRM, S_CEN, 73, 1, 2);	// ＤｏＰａ下り回線パケット送信リトライ回数(無応答)
	RauConfig.Dpa_ack_wait_tm	= 0 - (short)prm_get(COM_PRM, S_CEN, 72, 3, 1);	// ＤｏＰａ下り回線パケット応答待ち時間(ＡＣＫ/ＮＡＫ)
	RauConfig.Dpa_data_rcv_rty_cnt = prm_get(COM_PRM, S_CEN, 73, 1, 1);	// ＤｏＰａ下り回線パケット受信リトライ回数(ＮＡＫ)
	RauConfig.Dpa_port_watchdog_tm = 0 - (short)prm_get(COM_PRM, S_CEN, 72, 3, 4);	// ＤｏＰａ下り回線通信監視タイマ(秒)
	RauConfig.Dpa_port_m2		= prm_get(COM_PRM, S_CEN, 71, 5, 1);	// ＤｏＰａ下り回線 自局ポート番号
	if(2 == prm_get( COM_PRM,S_CEN,78,1,1 )) {							// Dopa/FOMA切替
		RauConfig.tcp_disconect_flag = 1;								// TCP切断なし
	}
	else {
		RauConfig.tcp_disconect_flag = 0;								// TCP切断あり
	}
	sprintf(ip, "%03d.%03d.%03d.%03d",
			prm_get(COM_PRM, S_CRE, 21, 3, 4),
			prm_get(COM_PRM, S_CRE, 21, 3, 1),
			prm_get(COM_PRM, S_CRE, 22, 3, 4),
			prm_get(COM_PRM, S_CRE, 22, 3, 1));
	RauConfig.Dpa_IP_Cre = inet_addr(ip);								/* クレジットセンターIPアドレス */
	RauConfig.Dpa_port_Cre = prm_get(COM_PRM, S_CRE, 23, 5, 1);		/* クレジットセンターポート番号 */
	RauConfig.Dpa_port_m3 = prm_get(COM_PRM, S_CRE, 24, 5, 1);
	RauConfig.Credit_snd_rty_cnt = (uchar)prm_get(COM_PRM, S_CRE, 25, 1, 5);
	RauConfig.Credit_nak_rty_cnt = (uchar)prm_get(COM_PRM, S_CRE, 25, 1, 4);
	RauConfig.Credit_ack_wait_tm = 0 - (ushort)prm_get(COM_PRM, S_CRE, 25, 3, 1);
	RauConfig.Credit_snd_rty_cnt_disconnected = RAU_RECONNECT_MAX;
	RauConfig.Credit_ack_wait_reconnect_tm = (ushort)( 0 - RAU_RECONNECT_INTERVAL );
	s_work = (short)prm_get(COM_PRM, S_CRE, 26, 3, 1);
	RauConfig.Credit_discnct_tm	= 0 - (s_work * 10);
// GG129000(S) W.Hoshino 2023/03/23 #6986 クレジットのデータコネクション設定時、参照先の共通パラメータアドレスが間違えている[共通改善項目 No 1550]
//	RauConfig.Credit_cnct_rty_cnt = (uchar)prm_get(COM_PRM, S_CRE, 26, 2, 4);
	RauConfig.Credit_cnct_rty_cnt = (uchar)prm_get(COM_PRM, S_CRE, 26, 1, 4);
// GG129000(E) W.Hoshino 2023/03/23 #6986 クレジットのデータコネクション設定時、参照先の共通パラメータアドレスが間違えている[共通改善項目 No 1550]
}
void	Credit_Init(void)
{
// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（SRAM容量確保）
//	memset(&DPA_Credit_SndCtrl, 0, sizeof(DPA_Credit_SndCtrl));	// クレジット回線の送信キュー
//	memset(&DPA_Credit_RcvCtrl, 0, sizeof(DPA_Credit_RcvCtrl));	// クレジット回線の受信キュー
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（SRAM容量確保）
	memset(&Credit_Snd_Buf, 0, sizeof(Credit_Snd_Buf));
	memset(&Credit_Rcv_Buf, 0, sizeof(Credit_Rcv_Buf));
	memset(&Credit_SndNtBlk, 0, sizeof(Credit_SndNtBlk));
	Cre_ucLastNtBlockNum = 0;					// 最後に受信したＮＴデータブロックナンバー
	Cre_uiArcReceiveLen = 0;					// 受信したＮＴブロック長の合計
	Cre_uiLastTcpBlockNum = 0;					// 最後に受信したＮＴデータブロックナンバー
	Cre_uiDopaReceiveLen = 0;					// 受信したＮＴブロック長の合計
	RAU_uc_mode_CRE = 0;						//	（クレジット通信側）通信状態モード
	RAU_uc_retrycnt_CRE = 0;					//	（クレジット通信側）送信リトライカウンタ
	Credit_nakretry_cnt = 0;
	memset(&RAU_uc_txdata_CRE_keep, 0, sizeof(RAU_uc_txdata_CRE_keep));				//	（クレジット通信側）前回送信したデータを保存
	RAU_ui_txdlength_CRE_keep = 0;
	RAU_ui_data_length_CRE = 0;					//	（クレジット通信側）受信バイト数カウンタ
	RAU_ui_txdlength_CRE = 0;					//	（クレジット通信側）送信データ長（メイン用）
	memset(&RAU_huc_txdata_CRE, 0, sizeof(RAU_huc_txdata_CRE));		//	（クレジット通信側）送信電文バッファ
	memset(&RAU_huc_rcv_work_CRE, 0,sizeof(RAU_huc_rcv_work_CRE));	//	（クレジット通信側）受信テキストワークバッファ
	memset(&RAU_Tm_Ackwait_CRE, 0, sizeof(RAU_TIME_AREA));							// 上り回線からデータ送信するときのＡＣＫ待ちタイマ
	RAU_uc_retrycnt_disconnected_CRE = 0;		//	（クレジット通信側）TCP切断時送信リトライカウンタ
	memset(&RAU_Tm_Ackwait_reconnect_CRE, 0, sizeof(RAU_TIME_AREA));				// クレジット回線からデータ送信するときのＡＣＫ待ち中の再接続タイマ

	Credit_seq_bak = 0;							// シーケンシャル№バックアップ
	Credit_SeqFlag = 0;							// 受信シーケンスフラグ
	Credit_TcpBlockSts = 0;						// TCPブロックステータス
	Credit_TcpBlockNo = 0;						// TCPブロックナンバー
	Credit_Error01 = 0;							// エラーコード01発生状況
	memset(&Credit_SendNtDataInfo, 0, sizeof(T_SEND_NT_DATA));				// 送信対象のテーブルバッファから取得したNTデータの情報
	memset(&Credit_SendIdInfo, 0, sizeof(T_SEND_DATA_ID_INFO));

	Credit_RAUDPA_seq = 0;						// クレジット用送信時シーケンシャル番号
	
	CRE_tm_TCP_TIME_WAIT.tm = 0;
	Credit_TCPtime_wait_ov = 1;
	
	Credit_rcvretry_cnt = 0;
	Credit_SeqReset_rq = 0;
	Credit_SeqReset_tm = 0;
	
	/* バッファのクリア */
	memset(&RecvCreditDt, 0, sizeof(CreNtDataBuf));
	memset(&RAU_huc_txdata_CRE, 0, sizeof(RAU_huc_txdata_CRE));
	memset(&RAU_huc_rcv_work_CRE,0, sizeof(RAU_huc_rcv_work_CRE));
	
	memset(&Credit_SndBuf_info, 0, sizeof(RAU_DATA_TABLE_INFO));
	Credit_SndBuf_info.pc_AreaTop				= &Credit_Snd_Buf[0];
	Credit_SndBuf_info.ul_AreaSize				= RAU_BLKSIZE;
	Credit_SndBuf_info.pc_ReadPoint				= Credit_SndBuf_info.pc_AreaTop;
	Credit_SndBuf_info.pc_WritePoint			= Credit_SndBuf_info.pc_AreaTop;
	
	memset(&Credit_RcvBuf_info, 0, sizeof(RAU_DATA_TABLE_INFO));
	Credit_RcvBuf_info.pc_AreaTop				= &Credit_Rcv_Buf[0];
	Credit_RcvBuf_info.ul_AreaSize				= RAU_BLKSIZE;
	Credit_RcvBuf_info.pc_ReadPoint				= Credit_RcvBuf_info.pc_AreaTop;
	Credit_RcvBuf_info.pc_WritePoint			= Credit_RcvBuf_info.pc_AreaTop;
}

/*[]----------------------------------------------------------------------[]*/
/*|             rautask() 	                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*|          RAU task main routine                                         |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	rautask( void )
{
	RAU_TimerInit();
	RAU_init();
	RAUid_Init();
	RAU_SockInit();				// ソケット制御初期化
	Credit_Init();

	for( ;; ){
		taskchg( IDLETSKNO );
		
		RAU_SockMain();			// ソケット制御メイン処理

		if(_is_ntnet_remote()) {
			RAU_FuncMain();			/* RAU process exec */
		}		
		Credit_main();			/* クレジット処理 */
	}
}
