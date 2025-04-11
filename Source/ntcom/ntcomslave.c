// GM849100(S) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
// MH364300 GG119A03 (S) NTNET通信制御対応（標準UT4000：MH341111流用）
		// 本ファイルを追加			nt task(NT-NET通信処理部)
// MH364300 GG119A03 (E) NTNET通信制御対応（標準UT4000：MH341111流用）
/*[]----------------------------------------------------------------------[]*
 *|		NT-NET task communications part
 *[]----------------------------------------------------------------------[]*
 *| Author      :  J.Mitani
 *| Date        :  2005-06-09
 *| Update      :	2005-06-18	#001	J.Mitani	仕様変更のためENQ電文にパケット優先モードを追加
 *| Update      :	2005-06-18	#002	J.Mitani	仕様変更のためACK02リトライアウトで電文削除
 *|	Update		:	2005-09-13	#003	machida.k	送信用優先データバッファは端末数分確保する
 *| Update		:	2005-12-20			machida.k	RAM共有化
 *| Update		:	2006-01-11	#004	J.Mitani	不具合修正
 *[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	<stdlib.h>
#include	"system.h"
#include	"common.h"
#include	"mem_def.h"
#include	"ope_def.h"
#include	"ntnet.h"
#include	"ntcom.h"
#include	"ntcomdef.h"


/*----------------------------------*/
/*		area style define			*/
/*----------------------------------*/
#define			_NTComData_GetRemainBlkNum(tele)	((tele)->blknum - (tele)->curblk)

/*----------------------------------*/
/*			value define			*/
/*----------------------------------*/

/*----------------------------------*/
/*			area define				*/
/*----------------------------------*/

/*----------------------------------*/
/*			table define			*/
/*----------------------------------*/

/*----------------------------------*/
/*	local function prottype define	*/
/*----------------------------------*/
static void slave_goto_idle( void );

static uchar slave_check_receive_data( void );
static uchar slave_get_terminal_status( uchar tele_kind, uchar send_req_flag, uchar packet_mode );

static void slave_data_received( void );				// データ受信完了
static void slave_idol_prosess( void );
static void slave_stx_send( void );
static void slave_ack_send( void );
static void slave_broad_cast( void );
static uchar slave_check_send_data( void );
static void slave_start_timer(uchar timer_kind);
static void slave_check_response_detail( void );
static void slave_check_crc_ctrl_tele(BOOL bCrc);

static void int_com_slave_timer_10m(void);
static void int_com_slave_timer_100m(void);



/*[]----------------------------------------------------------------------[]*
 *|             NTComSlave_Start()                                         |*
 *[]----------------------------------------------------------------------[]*
 *|		NTCom READER communication parts initialize routine	               |*
 *[]----------------------------------------------------------------------[]*/
void NTComSlave_Start(T_NT_BLKDATA* send, T_NT_BLKDATA* receive)
{

	NTComComm_Ctrl_Slave.status = NT_COM_SLAVE_IDOL;				// 従局ステータス
	NTComComm_Ctrl_Slave.master_status = NT_NORMAL;				// 主局ステータス
	NTComComm_Ctrl_Slave.broad_cast_received = 0;					// 同報データ受信済み
	NTComComm_Ctrl_Slave.block_send_count = 0;						// ブロック送信数
	NTComComm_Ctrl_Slave.block_receive_count = 0;					// ブロック受信数
	NTComComm_Ctrl_Slave.terminal_num = NTCom_InitData.TerminalNum;	// 主局時：接続されるターミナル数	従局時：自局番号
	NTComComm_Ctrl_Slave.receive_packet_crc_status = TRUE;			// 受信したパケットのCRCエラー状態
	NTComComm_Ctrl_Slave.receive_crc_err_count = 0;				// CRCエラーの受信回数｡
	NTComComm_Ctrl_Slave.receive_crc_err_count_ctrl = 0;			// CRCエラーの受信回数｡
	NTComComm_Ctrl_Slave.send_retry_count = 0;						// 送信リトライ回数
	NTComComm_Ctrl_Slave.ack02_receive_count = 0;					// ACK02リトライ回数	#002
	NTComComm_Ctrl_Slave.timeout_kind = 0;							// タイムアウト種別	#004

	NTComSlave_send_blk = send;											// 送信バッファ		初期化時に引数で渡す
	NTComSlave_receive_blk = receive;										// 受信バッファ

	NTComComm_Ctrl_Slave.timer_10m.timerid = NTComTimer_Create(10, 10, int_com_slave_timer_10m, TRUE);
	NTComComm_Ctrl_Slave.timer_100m.timerid = NTComTimer_Create(100, 100, int_com_slave_timer_100m, TRUE);

	NTComComm_Ctrl_Slave.timer_100m.count = NTCom_InitData.Time_LineMonitoring * 10;
	NTComTimer_Start(NTComComm_Ctrl_Slave.timer_100m.timerid);

}

/*[]----------------------------------------------------------------------[]*
 *|             NTComSlave_Clear()                                         |*
 *[]----------------------------------------------------------------------[]*
 *|		NTCom READER communication parts initialize routine	               |*
 *[]----------------------------------------------------------------------[]*/
void NTComSlave_Clear( void )
{

	// 現在使用している、電文情報をリセットする｡
	NTComData_ResetTelegram(&normal_telegram);
	NTComData_ResetTelegram(&prior_telegram);
	last_telegram = NULL;

}


/*[]----------------------------------------------------------------------[]*
 *|             NTComSlave_Main()                                          |*
 *[]----------------------------------------------------------------------[]*
 *|		NTCom READER function communication parts Main routine			   |*
 *[]----------------------------------------------------------------------[]*/
void	NTComSlave_Main( void )
{
	if (NTComComm_Ctrl_Slave.timer_10m.count == 0) {	// タイムアウトしていたら受信失敗としてアイドルへ
		slave_goto_idle();
	}


	// NE-NETパケット検出
	switch(NTComComdr_GetRcvPacket(NTComSlave_receive_blk)){
	case	NT_COMDR_NO_PKT:						// アイドル
		break;
	case	NT_COMDR_PKT_RCVING:					// パケット受信中
		if ((NTComComm_Ctrl_Slave.timeout_kind == 3) && (NTComComm_Ctrl_Slave.timer_10m.count > 0)) {		// #004
			// 起動しているタイマーがT3だったら
			// T2 タイマー起動開始
			NTComComm_Ctrl_Slave.timeout_kind = 2;
			slave_start_timer(NTComComm_Ctrl_Slave.timeout_kind);
		}
		break;
	case	NT_COMDR_PKT_RCVD:						// パケット受信完了
		// パケットチェック
		if (NTComComm_packet_check(NTComSlave_receive_blk, NTComComm_Ctrl_Slave.cur_seq_no, NTCom_InitData.TerminalNum)) {
			// TURE
			NTComComm_Ctrl_Slave.receive_packet_crc_status = NTComComm_crc_check(NTComSlave_receive_blk);

			// T2タイマーストップ
			NTComComm_Ctrl_Slave.timer_10m.count = -1;
			slave_data_received();		// 基本的な電文チェックを通過したので、プロトコルに依存した処理を開始。
		}
		break;
	case	NT_COMDR_ERR_SCI:						// 通信エラー(オーバーランエラー/フレーミングエラー/パリティエラー)
		// 通信エラー発生　→　IBWにエラー発生を通知

		break;
	case	NT_COMDR_ERR_INVALID_LEN:				// パケットデータ長不正
	case	NT_COMDR_ERR_TIMEOUT:					// パケット受信完了前にキャラクタ間タイムアウト
	default:
		break;
	}

	if (NTComComm_Ctrl_Slave.status == NT_COM_SLAVE_IDOL
	 && NTComComm_Ctrl_Slave.timer_100m.count == 0
	 && NTComComm_Ctrl_Slave.master_status == NT_NORMAL) {	// 通信回線監視タイマーチェック

		NTCom_err_chk(NT_ERR_NO_RESPONSE, 1, 0, 0, NULL);
		NTComData_SetTerminalStatus(NTComComm_Ctrl_Slave.terminal_num, NT_ABNORMAL, &normal_telegram, &prior_telegram);
		NTComComm_Ctrl_Slave.master_status = NT_ABNORMAL;
	}
}
/*[]-----------------------------------------------------------------------[]*
 *|			NTComSlave_isIdle()
 *[]-----------------------------------------------------------------------[]*
 *|			タスクのアイドル状態を取得する
 *|	param
 *|	return
 *[]-----------------------------------------------------------------------[]*/
BOOL NTComSlave_isIdle( void )
{
	if (NTComComm_Ctrl_Slave.status == NT_COM_SLAVE_IDOL) {
		return TRUE;
	}
	return FALSE;
}

/*[]----------------------------------------------------------------------[]*
 *|             slave_goto_idle() 		    	                           |*
 *[]----------------------------------------------------------------------[]*
 *|		phase change to IDLE                     						   |*
 *[]----------------------------------------------------------------------[]*/
void	slave_goto_idle( void )
{

	// アイドル状態へ移行
	NTComComm_Ctrl_Slave.status = NT_COM_SLAVE_IDOL;

	// 通信回線監視タイマー再開
	NTComComm_Ctrl_Slave.timer_100m.count = NTCom_InitData.Time_LineMonitoring * 10;

	// 10mタイマー停止
	NTComComm_Ctrl_Slave.timer_10m.count = -1;

//	NTCom_Com_Status(NT_LED_IDLE);

}


/*[]----------------------------------------------------------------------[]*
 *|             data_recived()												|*
 *[]----------------------------------------------------------------------[]*
 *|			                       		   |*
 *[]----------------------------------------------------------------------[]*/
void slave_data_received( void ) {

	switch (NTComComm_Ctrl_Slave.status) {
	case NT_COM_SLAVE_IDOL:				/* アイドル (ENQ待ち)*/
		slave_idol_prosess();
		break;
	case NT_COM_SLAVE_STX_SEND:			/* STX送信後 */
		slave_stx_send();
		break;
	case NT_COM_SLAVE_ACK_SEND:			/* ACK送信後 */
		slave_ack_send();
		break;
	case NT_COM_SLAVE_BROAD_CAST_WAIT:	/* 同報受信待ち */
		slave_broad_cast();
		break;
	default:
		break;
	}
}
/*[]-----------------------------------------------------------------------[]*
 *|				slave_idol_prosess()												|*
 *[]-----------------------------------------------------------------------[]*
 *|		従局状態　アイドル中の処理											|*
 *[]-----------------------------------------------------------------------[]*
 *|		0 == false 		0 != ture											|*
 *[]-----------------------------------------------------------------------[]*/
void slave_idol_prosess( void ) {

	uchar send_enable;
	uchar terminal_status;

	switch (NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]) {
	case ENQ:

		// 制御電文CRCチェック
		slave_check_crc_ctrl_tele(NTComComm_Ctrl_Slave.receive_packet_crc_status);

		// ENQが受信できたので通信回線監視タイマーをとめる
		NTComComm_Ctrl_Slave.timer_100m.count = -1;

		if (NTComComm_Ctrl_Slave.receive_packet_crc_status == FALSE) {
			// CRCエラーのENQを受信したので無視。
			return;
		}

		if (NTComComm_Ctrl_Slave.master_status == NT_ABNORMAL) {
			//端末状態アブノーマルのとき　ENQがあったらノーマルに戻る
			// 通信不良解除
			NTCom_err_chk(NT_ERR_NO_RESPONSE, 0, 0, 0, NULL);
			// 端末ステータス通常
			NTComData_SetTerminalStatus(NTComComm_Ctrl_Slave.terminal_num, NT_NORMAL, &normal_telegram, &prior_telegram);
			// 端末状態変更
			NTComComm_Ctrl_Slave.master_status = NT_NORMAL;
		}

		NTComComm_Ctrl_Slave.cur_seq_no = NTComSlave_receive_blk->data[NT_OFS_LINK_NO];	// シーケンスナンバー取得

		if (NTComSlave_receive_blk->data[NT_OFS_TERMINAL_NO] == 0xFF) {
			// 同報セレクティング
			NTComComm_Ctrl_Slave.status = NT_COM_SLAVE_BROAD_CAST_WAIT;
			// t6でタイマー起動
			NTComComm_Ctrl_Slave.timeout_kind = 6;
			slave_start_timer(NTComComm_Ctrl_Slave.timeout_kind);
		} else {
			// 同報以外

			if (NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG] != 0x00) {
				// 送信要求フラグが0でないのでセレクティング
				NTComComm_Ctrl_Slave.block_send_count = NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG];	// 送信ブロック数を取得
				NTComComm_Ctrl_Slave.block_receive_count = 0;
			}

			send_enable = slave_check_send_data();
			if (send_enable) {	// 送信可能なデータがあるか？　主局が受信可能か？

				NTComComm_Ctrl_Slave.send_retry_count = 0;			// 再送信回数クリア

				// 送信可能
				if (send_enable == SEND_ENABLE_PRIOR) {
					// 優先データ送信
					NTComData_GetSendBlock(&prior_telegram, NTComSlave_send_blk);
					// 再送用に最後に送った電文の情報を保存しておく
					last_telegram = &prior_telegram;
				} else if (send_enable == SEND_ENABLE_NORMAL) {
					// 通常データ送信
					NTComData_GetSendBlock(&normal_telegram, NTComSlave_send_blk);
					// 再送用に最後に送った電文の情報を保存しておく
					last_telegram = &normal_telegram;
				}
//	#001	→
				NTComComm_Ctrl_Slave.last_terminal_status = slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
																				, NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
																				, NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE]);
//	←	#001

				if ((NTComComm_Ctrl_Slave.last_terminal_status & 0xFF) == 0) {	// 自局が受信可能か？	#004
					NTComComm_create_stx_telegram(NTComSlave_send_blk
									  , NT_DIRECTION_TO_MASTER
									  , NTComComm_Ctrl_Slave.terminal_num
									  , NTComComm_Ctrl_Slave.cur_seq_no
									  , NTComComm_Ctrl_Slave.last_terminal_status
									  , NTComComm_Ctrl_Slave.broad_cast_received
									  , NT_RES_NORMAL);
				} else {
					NTComComm_create_stx_telegram(NTComSlave_send_blk
									  , NT_DIRECTION_TO_MASTER
									  , NTComComm_Ctrl_Slave.terminal_num
									  , NTComComm_Ctrl_Slave.cur_seq_no
									  , NTComComm_Ctrl_Slave.last_terminal_status
									  , NTComComm_Ctrl_Slave.broad_cast_received
									  , NT_RES_BUFFER_FULL);
				}
				NTComComm_Ctrl_Slave.broad_cast_received = NT_NOT_BROADCASTED;

				NTComComm_Ctrl_Slave.status = NT_COM_SLAVE_STX_SEND;
				NTComComm_Ctrl_Slave.timeout_kind = 3;
			} else {
				// 送信不可
				if (NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG] == 0) {
					// ポーリング
					// EOT電文生成
					NTComComm_create_eot_telegram(NTComSlave_send_blk
									  , NT_DIRECTION_TO_MASTER
									  , NTComComm_Ctrl_Slave.terminal_num
									  , NTComComm_Ctrl_Slave.cur_seq_no
//	#001	→
									  , slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
																  , NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
																  , NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE])
//	←	#001
									  , NTComComm_Ctrl_Slave.broad_cast_received
									  , NT_RES_NORMAL);

					NTComComm_Ctrl_Slave.broad_cast_received = NT_NOT_BROADCASTED;

				} else {
					// セレクティング
//	#001	→
					terminal_status = slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
																, NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
																, NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE]);
//	←	#001
					if ((terminal_status & 0xFF) == 0) {	// 自局が受信可能か？	#004

						// 受信可能
						// ACK電文生成
						NTComComm_create_ack_telegram(NTComSlave_send_blk
										  , NT_DIRECTION_TO_MASTER
										  , NTComComm_Ctrl_Slave.terminal_num
										  , NTComComm_Ctrl_Slave.cur_seq_no
										  , terminal_status
										  , NTComComm_Ctrl_Slave.broad_cast_received
										  , NT_RES_NORMAL);

						NTComComm_Ctrl_Slave.broad_cast_received = NT_NOT_BROADCASTED;

						// ACK送信後へ移行
						NTComComm_Ctrl_Slave.status = NT_COM_SLAVE_ACK_SEND;
						NTComComm_Ctrl_Slave.timeout_kind = 3;
					} else {
						// 受信不可能
						// EOT電文生成
						NTComComm_create_eot_telegram(NTComSlave_send_blk
										  , NT_DIRECTION_TO_MASTER
										  , NTComComm_Ctrl_Slave.terminal_num
										  , NTComComm_Ctrl_Slave.cur_seq_no
										  , terminal_status
										  , NTComComm_Ctrl_Slave.broad_cast_received
										  , NT_RES_BUFFER_FULL);

						NTComComm_Ctrl_Slave.broad_cast_received = NT_NOT_BROADCASTED;

					}
				}
			}
			// 電文送信
			NTComComdr_SendPacket(NTComSlave_send_blk, NTCom_InitData.Time_t9, NTCom_InitData.Time_t2);
			if (NTComComm_Ctrl_Slave.timeout_kind == 3) {
				slave_start_timer(NTComComm_Ctrl_Slave.timeout_kind);
			} else {
				// アイドル状態へ移行
				slave_goto_idle();
			}
		}
		break;
	case ACK:
	case STX:
	case NAK:
	case EOT:
	default:
		break;
	}
}
/*[]-----------------------------------------------------------------------[]*
 *|				slave_wait_ack()												|*
 *[]-----------------------------------------------------------------------[]*
 *|		従局状態　STX送信後の処理											|*
 *[]-----------------------------------------------------------------------[]*
 *|		0 == false 		0 != ture											|*
 *[]-----------------------------------------------------------------------[]*/
void slave_stx_send( void ) {

	uchar terminal_status;
	uchar response_detail;

	switch (NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]) {
	case ENQ:

		// 制御電文CRCチェック
		slave_check_crc_ctrl_tele(NTComComm_Ctrl_Slave.receive_packet_crc_status);

		if (NTComSlave_receive_blk->data[NT_OFS_TERMINAL_NO] == 0xFF) {		// 同報ENQ？
			// 同報ENQ
			// 同報待ちへ状態を移行
			NTComComm_Ctrl_Slave.status = NT_COM_SLAVE_BROAD_CAST_WAIT;
			NTComComm_Ctrl_Slave.cur_seq_no = NTComSlave_receive_blk->data[NT_OFS_LINK_NO];	// シーケンスナンバー取得
			// t6でタイマー起動
			NTComComm_Ctrl_Slave.timeout_kind = 6;
			slave_start_timer(NTComComm_Ctrl_Slave.timeout_kind);
		} else {
			// 通常ENQ
			// EOT電文生成
//	#001	→
			terminal_status = slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
														, NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
														, NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE]);
//	←	#001
			NTComComm_create_eot_telegram(NTComSlave_send_blk
							  , NT_DIRECTION_TO_MASTER
							  , NTComComm_Ctrl_Slave.terminal_num
							  , NTComSlave_receive_blk->data[NT_OFS_LINK_NO]
							  , terminal_status
							  , NT_NOT_BROADCASTED
							  , NT_RES_SEQUENCE_ERR);

			// アイドル状態へ移行
			slave_goto_idle();
			NTComComdr_SendPacket(NTComSlave_send_blk, NTCom_InitData.Time_t9, NTCom_InitData.Time_t2);

		}
		break;
	case ACK:

		// 制御電文CRCチェック
		slave_check_crc_ctrl_tele(NTComComm_Ctrl_Slave.receive_packet_crc_status);

		// 応答詳細を確認する。
		slave_check_response_detail();

		// アイドル状態へ移行
		slave_goto_idle();
		break;
	case STX:

		// 一回前に送信した端末ステータスと比較する。
		if (NTComComm_Ctrl_Slave.last_terminal_status & NT_ALL_BUFFER_FULL) {
			// 一回前に端末ステータスフルで返したのにSTXが送られた。
//	#001	→
			terminal_status = slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
														, NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
														, NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE]);
//	←	#001
			// EOT81で返す。
			NTComComm_create_eot_telegram(NTComSlave_send_blk
							  , NT_DIRECTION_TO_MASTER
							  , NTComComm_Ctrl_Slave.terminal_num
							  , NTComSlave_receive_blk->data[NT_OFS_LINK_NO]
							  , terminal_status
							  , NT_NOT_BROADCASTED
							  , NT_RES_BUFFER_FULL);

			// アイドル状態へ移行
			slave_goto_idle();
		} else {
			// 一回前に端末ステータスフル以外
			// 応答詳細を確認する。
			slave_check_response_detail();

			response_detail = slave_check_receive_data();
//	#001	→
			terminal_status = slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
														, NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
														, NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE]);

//	←	#001
			if (response_detail == NT_RES_CRC_ERR) {
				NTComComm_create_nak_telegram(NTComSlave_send_blk
								  , NT_DIRECTION_TO_MASTER
								  , NTComComm_Ctrl_Slave.terminal_num
								  , NTComComm_Ctrl_Slave.cur_seq_no
								  , terminal_status
								  , NT_NOT_BROADCASTED
								  , response_detail);
				// NAKを返したときは状態遷移しない。
			} else {
				NTComComm_create_ack_telegram(NTComSlave_send_blk
								  , NT_DIRECTION_TO_MASTER
								  , NTComComm_Ctrl_Slave.terminal_num
								  , NTComComm_Ctrl_Slave.cur_seq_no
								  , terminal_status
								  , NT_NOT_BROADCASTED
								  , response_detail);
				if ((NTComComm_Ctrl_Slave.block_receive_count < NTComComm_Ctrl_Slave.block_send_count)		// 送信要求フラグ数？
				 && (response_detail != NT_RES_BLOCKNO_ERR1)) {											// ブロック抜けでなければ受信数をチェック　#004
					// 送信要求フラグ数未満
					// STX後にデータを受信したのでACK送信後に遷移
					NTComComm_Ctrl_Slave.status = NT_COM_SLAVE_ACK_SEND;
					NTComComm_Ctrl_Slave.timeout_kind = 3;
				} else {
					// 送信要求フラグ数に到達
					// アイドル状態へ移行
					slave_goto_idle();
				}
			}
		}
		NTComComdr_SendPacket(NTComSlave_send_blk, NTCom_InitData.Time_t9, NTCom_InitData.Time_t2);
		if (NTComComm_Ctrl_Slave.timeout_kind == 3) {
			slave_start_timer(NTComComm_Ctrl_Slave.timeout_kind);
		}

		break;
	case NAK:

		// 制御電文CRCチェック
		slave_check_crc_ctrl_tele(NTComComm_Ctrl_Slave.receive_packet_crc_status);

		if (NTComSlave_receive_blk->data[NT_OFS_RESPONSE_DETAIL] == NT_RES_CRC_ERR) {
			// NAK　かつ　0x80
			NTComComm_Ctrl_Slave.send_retry_count++;			// 再送信回数インクリメント
			if (NTComComm_Ctrl_Slave.send_retry_count > (NTCom_InitData.Retry + 3)) {
				// ACK01応答がないので、エンドレス防止のため強制終了｡
				NTCom_err_chk(NT_ERR_RETRY_OVER, 2, 0, 0, NULL);
				NTComData_DeleteTelegram(last_telegram);
				NTComComm_Ctrl_Slave.send_retry_count = 0;			// 再送信回数クリア
				NTComComm_Ctrl_Slave.ack02_receive_count = 0;		// 電文を削除したのでACK02の数もクリア	#002
				slave_goto_idle();		//#004
			} else {
				// ブロック再取得
				NTComData_GetSendBlock(last_telegram, NTComSlave_send_blk);
				// 電文再生成
				NTComComm_create_stx_telegram(NTComSlave_send_blk
								  , NT_DIRECTION_TO_MASTER
								  , NTComComm_Ctrl_Slave.terminal_num
								  , NTComComm_Ctrl_Slave.cur_seq_no
//	#001	→
								  , slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
															  , NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
															  , NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE])
//	←	#001
								  , NTComComm_Ctrl_Slave.broad_cast_received
								  , NT_RES_NORMAL);
				// 再送信
				NTComComdr_SendPacket(NTComSlave_send_blk, NTCom_InitData.Time_t9, NTCom_InitData.Time_t2);
				NTComComm_Ctrl_Slave.timeout_kind = 3;
				slave_start_timer(NTComComm_Ctrl_Slave.timeout_kind);
			}
		}
		break;
	case EOT:
		// 制御電文CRCチェック
		slave_check_crc_ctrl_tele(NTComComm_Ctrl_Slave.receive_packet_crc_status);
		// アイドルへ
		slave_goto_idle();

		break;
	default:
		// アイドルへ
		slave_goto_idle();

		break;
	}
}
/*[]-----------------------------------------------------------------------[]*
 *|				slave_wait_stx()											|*
 *[]-----------------------------------------------------------------------[]*
 *|		従局状態　ACK送信後の処理											|*
 *[]-----------------------------------------------------------------------[]*
 *|		0 == false 		0 != ture											|*
 *[]-----------------------------------------------------------------------[]*/
void slave_ack_send( void ) {

	uchar terminal_status;
	uchar response_detail;

	switch (NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]) {
	case ENQ:

		// 制御電文CRCチェック
		slave_check_crc_ctrl_tele(NTComComm_Ctrl_Slave.receive_packet_crc_status);

		if (NTComSlave_receive_blk->data[NT_OFS_TERMINAL_NO] == 0xFF) {		// 同報ENQ？
			// 同報ENQ
			// 同報待ちへ状態を移行
			NTComComm_Ctrl_Slave.status = NT_COM_SLAVE_BROAD_CAST_WAIT;
			NTComComm_Ctrl_Slave.cur_seq_no = NTComSlave_receive_blk->data[NT_OFS_LINK_NO];	// シーケンスナンバー取得
			// t6でタイマー起動
			NTComComm_Ctrl_Slave.timeout_kind = 6;
			slave_start_timer(NTComComm_Ctrl_Slave.timeout_kind);
		} else {
			// 通常ENQ
			// EOT電文生成
//	#001	→
			terminal_status = slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
														, NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
														, NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE]);
//	←	#001
			NTComComm_create_eot_telegram(NTComSlave_send_blk
							  , NT_DIRECTION_TO_MASTER
							  , NTComComm_Ctrl_Slave.terminal_num
							  , NTComSlave_receive_blk->data[NT_OFS_LINK_NO]
							  , terminal_status
							  , NT_NOT_BROADCASTED
							  , NT_RES_SEQUENCE_ERR);
			NTComComdr_SendPacket(NTComSlave_send_blk, NTCom_InitData.Time_t9, NTCom_InitData.Time_t2);

			// アイドル状態へ移行
			slave_goto_idle();
		}
		break;
	case STX:
	// 前回ACK　STX送信時のバッファ状態が必要　前回バッファフルだったら無条件にEOT81
		response_detail = slave_check_receive_data();
//	#001	→
		terminal_status = slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
													, NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
													, NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE]);
//	←	#001

		if (response_detail == NT_RES_CRC_ERR) {
			NTComComm_create_nak_telegram(NTComSlave_send_blk
							  , NT_DIRECTION_TO_MASTER
							  , NTComComm_Ctrl_Slave.terminal_num
							  , NTComComm_Ctrl_Slave.cur_seq_no
							  , terminal_status
							  , NT_NOT_BROADCASTED
							  , response_detail);
		}
		 else {
			NTComComm_create_ack_telegram(NTComSlave_send_blk
							  , NT_DIRECTION_TO_MASTER
							  , NTComComm_Ctrl_Slave.terminal_num
							  , NTComComm_Ctrl_Slave.cur_seq_no
							  , terminal_status
							  , NT_NOT_BROADCASTED
							  , response_detail);
		}

		if ((NTComComm_Ctrl_Slave.block_receive_count < NTComComm_Ctrl_Slave.block_send_count)
		 && ((response_detail == NT_RES_NORMAL) || (response_detail == NT_RES_BLOCKNO_ERR2) || response_detail == NT_RES_CRC_ERR)) {		// 送信要求フラグ数？	#004

			// 送信要求フラグ数未満
			// STX後にデータを受信したのでACK送信後に遷移
			NTComComm_Ctrl_Slave.status = NT_COM_SLAVE_ACK_SEND;
			NTComComm_Ctrl_Slave.timeout_kind = 3;
		} else {
			// 送信要求フラグ数に到達
			// アイドル状態へ移行
			slave_goto_idle();
		}
		NTComComdr_SendPacket(NTComSlave_send_blk, NTCom_InitData.Time_t9, NTCom_InitData.Time_t2);
		if (NTComComm_Ctrl_Slave.timeout_kind == 3) {
			slave_start_timer(NTComComm_Ctrl_Slave.timeout_kind);
		}

		break;
	case ACK:
	case NAK:

		// 制御電文CRCチェック
		slave_check_crc_ctrl_tele(NTComComm_Ctrl_Slave.receive_packet_crc_status);

		// EOTを送信し
//	#001	→
		terminal_status = slave_get_terminal_status(NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]
												, NTComSlave_receive_blk->data[NT_OFS_SEND_REQ_FLAG]
												, NTComSlave_receive_blk->data[NT_OFS_PACKET_MODE]);
//	←	#001
		NTComComm_create_eot_telegram(NTComSlave_send_blk
						  , NT_DIRECTION_TO_MASTER
						  , NTComComm_Ctrl_Slave.terminal_num
						  , NTComComm_Ctrl_Slave.cur_seq_no
						  , terminal_status
						  , NT_NOT_BROADCASTED
						  , NT_RES_SEQUENCE_ERR);

		NTComComdr_SendPacket(NTComSlave_send_blk, NTCom_InitData.Time_t9, NTCom_InitData.Time_t2);
		// アイドルへ
		slave_goto_idle();
		break;
	case EOT:

		// 制御電文CRCチェック
		slave_check_crc_ctrl_tele(NTComComm_Ctrl_Slave.receive_packet_crc_status);

	default:
		// アイドルへ
		slave_goto_idle();
		break;
	}
}
/*[]-----------------------------------------------------------------------[]*
 *|				slave_broad_cast()											|*
 *[]-----------------------------------------------------------------------[]*
 *|		従局状態　同報待ちの処理											|*
 *[]-----------------------------------------------------------------------[]*
 *|		0 == false 		0 != ture											|*
 *[]-----------------------------------------------------------------------[]*/
void slave_broad_cast( void ) {
	switch (NTComSlave_receive_blk->data[NT_OFS_TELEGRAM_KIND]) {
	case STX:
		if (NTComComm_Ctrl_Slave.receive_packet_crc_status == FALSE) {
			// CRCエラーのデータを受信したので無視。
			NTComComm_Ctrl_Slave.broad_cast_received = NT_BROADCASTED_CRC_ERR;		// 同報データCRCエラー
			// アイドルへ
			slave_goto_idle();
		} else {
			// 受信バッファにデータをセットする。
			switch (NTComData_SetRcvBlock(NTComSlave_receive_blk)) {
			case NT_DATA_NORMAL:						/* 成功 */
				// 同報データ正常受信
				NTComComm_Ctrl_Slave.broad_cast_received = NT_BROADCASTED;		// 同報データ受信済み
				break;
			case NT_DATA_BLOCK_INVALID1:				/* ACK02 or NT_NORMAL_BLKNUM_MAX待っても最終ブロックが来ない場合 */
			case NT_DATA_BLOCK_INVALID2:				/* ACK03 */
			case NT_DATA_BUFFERFULL:				/* 優先バッファが一杯でセットできない */
			default:
				break;
			}
			// アイドル状態へ移行
			slave_goto_idle();
		}
		break;
	case ENQ:
	case ACK:
	case NAK:
	case EOT:
		// 制御電文CRCチェック
		slave_check_crc_ctrl_tele(NTComComm_Ctrl_Slave.receive_packet_crc_status);
	default:
		// アイドルへ
		slave_goto_idle();
		break;
	}
}
/*[]-----------------------------------------------------------------------[]*
 *|			slave_check_send_data()
 *[]-----------------------------------------------------------------------[]*
 *|				主局にデータ送信可能かチェックする
 *|	param
 *|	return
 *[]-----------------------------------------------------------------------[]*/
uchar slave_check_send_data( void ) {

	// 主局優先データ受信可能？
	if ((NTComSlave_receive_blk->data[NT_OFS_TERMINAL_STS] & NT_PRIOR_BUFFER_FULL) == 0) {
		// 最初に優先データのテレグラムをチェックする｡
		if (NTComData_IsValidTelegram(&prior_telegram)) {
			// テレグラムが有効だったら、優先データありで返す｡
			return SEND_ENABLE_PRIOR;
		} else {
			// テレグラムが無効だったらPeekしてみる
			if (NTComData_PeekSndTele_Prior(NTComComm_Ctrl_Slave.terminal_num, &prior_telegram)) {	/* #003 */
				// 戻り値が有効だったら優先データありで返す｡
				return SEND_ENABLE_PRIOR;
			}
		}

	}
	// 主局通常データ受信可能？
	if ((NTComSlave_receive_blk->data[NT_OFS_TERMINAL_STS] & NT_NORMAL_BUFFER_FULL) == 0) {
		// 優先データがなかったら、通常データをチェックする｡
		if (NTComData_IsValidTelegram(&normal_telegram)) {
			// テレグラムが有効だったら、通常データありで返す｡
			return SEND_ENABLE_NORMAL;
		} else {
			// テレグラムが無効だったら、Peekしてみる｡
			if (NTComData_PeekSndTele_Normal(NTComComm_Ctrl_Slave.terminal_num, &normal_telegram)) {
				// 戻り値が有効だったら通常データありで返す｡
				return SEND_ENABLE_NORMAL;
			}
		}
	}

	// どこにも引っかからなかったら、データなしで返す｡
	return SEND_DISABLE;

}

/*[]-----------------------------------------------------------------------[]*
 *|			slave_get_terminal_status()
 *[]-----------------------------------------------------------------------[]*
 *|				従局ターミナルステータスを取得
 *|	param		tele_kind		電文種別
 *|	param		send_req_flag	送信要求フラグ
 *|	return
 *[]-----------------------------------------------------------------------[]*/
uchar slave_get_terminal_status( uchar tele_kind, uchar send_req_flag, uchar packet_mode ) {

	uchar ret = 0;
	T_NT_BUF_STATUS buf_status;
	T_NT_BUF_COUNT bufcnt;

	NTComData_GetBufferStatus(&buf_status, NTComComm_Ctrl_Slave.terminal_num);
	NTComData_GetRcvBufferCount(&bufcnt);

	if (tele_kind == ENQ) {
		if (packet_mode == NT_PRIOR_DATA) {
			// 優先データENQ
			if (buf_status.prior_r == TRUE		// 電文が存在する。
			 || bufcnt.prior != 0) {			// バッファ使用量が０でない。
				ret |= NT_PRIOR_BUFFER_FULL;
			}
		} else {
			// 通常データENQ
			if (buf_status.normal_r == TRUE		// 電文が存在する。
			 || send_req_flag + bufcnt.normal > NT_NORMAL_BLKNUM_MAX) {	// 使用量　＋　送信量　＞　Max
				ret |= NT_NORMAL_BUFFER_FULL;
			}
		}
		// ENQの時は通常データの送信要求フラグと、受信バッファ使用量を比較する｡
	} else {
		// ENQ以外の時はバッファの空きを監視｡
		if (buf_status.prior_r == TRUE) {	// 電文が存在する。
			ret |= NT_PRIOR_BUFFER_FULL;
		}
		if (buf_status.normal_r == TRUE) {	// 電文が存在する。
			ret |= NT_NORMAL_BUFFER_FULL;
		}
	}

	return ret;
}
/*[]-----------------------------------------------------------------------[]*
 *|			int_com_slave_timer_10m()
 *[]-----------------------------------------------------------------------[]*
 *|				1m sec タイマー割り込み処理
 *|	param
 *|	return
 *[]-----------------------------------------------------------------------[]*/
void int_com_slave_timer_10m(void) {

	if (NTComComm_Ctrl_Slave.timer_10m.count > 0) {
		NTComComm_Ctrl_Slave.timer_10m.count--;
	}

}
/*[]-----------------------------------------------------------------------[]*
 *|			int_com_slave_timer_100m()
 *[]-----------------------------------------------------------------------[]*
 *|				100m sec タイマー割り込み処理
 *|	param
 *|	return
 *[]-----------------------------------------------------------------------[]*/
void int_com_slave_timer_100m(void) {

	if (NTComComm_Ctrl_Slave.timer_100m.count > 0) {
		NTComComm_Ctrl_Slave.timer_100m.count--;
	}

}
/*[]-----------------------------------------------------------------------[]*
 *|			int_com_slave_timer_100m()
 *[]-----------------------------------------------------------------------[]*
 *|				100m sec タイマー割り込み処理
 *|	param
 *|	return
 *[]-----------------------------------------------------------------------[]*/
void slave_start_timer(uchar timer_kind) {

	switch (timer_kind) {
	case 1:
		NTComComm_Ctrl_Slave.timer_10m.count = _To10msVal(NTCom_InitData.Time_t1);
		break;
	case 2:
		NTComComm_Ctrl_Slave.timer_10m.count = _To10msVal(NTCom_InitData.Time_t2);
		break;
	case 3:
		NTComComm_Ctrl_Slave.timer_10m.count = _To10msVal(NTCom_InitData.Time_t3);
		break;
	case 4:
		NTComComm_Ctrl_Slave.timer_10m.count = _To10msVal(NTCom_InitData.Time_t4);
		break;
	case 5:
		NTComComm_Ctrl_Slave.timer_10m.count = _To10msVal(NTCom_InitData.Time_t5);
		break;
	case 6:
		NTComComm_Ctrl_Slave.timer_10m.count = _To10msVal((NTCom_InitData.Time_t6 + NTCom_InitData.Time_t7));
		break;
	case 7:
		NTComComm_Ctrl_Slave.timer_10m.count = _To10msVal(NTCom_InitData.Time_t7);
		break;
	case 8:
		NTComComm_Ctrl_Slave.timer_10m.count = _To10msVal(NTCom_InitData.Time_t8);
		break;
	case 9:
		NTComComm_Ctrl_Slave.timer_10m.count = _To10msVal(NTCom_InitData.Time_t9);
		break;
	default:
		break;
	}
	NTComTimer_Start(NTComComm_Ctrl_Slave.timer_10m.timerid);
}


/*[]-----------------------------------------------------------------------[]*
 *|			slave_check_response_detail()
 *[]-----------------------------------------------------------------------[]*
 *|				STX送信後の電文の応答詳細をチェックして、電文が正しく到達したかチェックする｡
 *|	param
 *|	return
 *[]-----------------------------------------------------------------------[]*/
void slave_check_response_detail () {

	switch (NTComSlave_receive_blk->data[NT_OFS_RESPONSE_DETAIL]) {
	case NT_RES_NORMAL:
		if (last_telegram->kind == NT_DATA_KIND_PRIOR_SND) {
			// 優先データは１ブロックのはずなので、送れたら削除する。
			NTComData_DeleteTelegram(last_telegram);
			NTComComm_Ctrl_Slave.ack02_receive_count = 0;	//	#002
		} else {
			// ACKを受信したので次のブロックに進める。
			NTComData_SeekBlock(last_telegram, 1, NT_SEEK_CUR);

			// 1ブロック正常に送信完了
			if (_NTComData_GetRemainBlkNum(last_telegram) < 1) {		// 電文に未送信ブロックはあるか
				// 未送信ブロックなし
				// 1電文送信完了、送信バッファから電文削除
				NTComData_DeleteTelegram(last_telegram);
				NTComComm_Ctrl_Slave.ack02_receive_count = 0;	//	#002
			}
		}
		break;
	case NT_RES_CRC_RETRY_OVER:
		// CRCエラーでリトライオーバー、送信電文を削除。
		NTComData_DeleteTelegram(last_telegram);
		NTCom_err_chk(NT_ERR_ACK01, 2, 0, 0, NULL);
		NTComComm_Ctrl_Slave.ack02_receive_count = 0;	//	#002
		break;
	case NT_RES_BLOCKNO_ERR1:
		// ブロック連番エラーで主局側で送信データ全ブロック削除
		NTComComm_Ctrl_Slave.ack02_receive_count++;
		if (NTComComm_Ctrl_Slave.ack02_receive_count >= NTCom_InitData.Retry) {
			// リトライ回数分受信したので、送信電文破棄
			NTComData_DeleteTelegram(last_telegram);
			NTCom_err_chk(NT_ERR_ACK02, 2, 0, 0, NULL);
			NTComComm_Ctrl_Slave.ack02_receive_count = 0;	//	#002
		} else {
			// 次回電文先頭から再送信
			NTComData_SeekBlock(last_telegram, 0, NT_SEEK_SET);
		}
		break;
	case NT_RES_BLOCKNO_ERR2:
		if (last_telegram->kind == NT_DATA_KIND_PRIOR_SND) {
			// 優先データは１ブロックのはずなので、送れたら削除する。
			NTComData_DeleteTelegram(last_telegram);
			NTComComm_Ctrl_Slave.ack02_receive_count = 0;	//	#002
		} else {
			// 今回送った電文は有効なので次のブロックに進める。
			NTComData_SeekBlock(last_telegram, 1, NT_SEEK_CUR);

			// 1ブロックに送信完了
			if (_NTComData_GetRemainBlkNum(last_telegram) < 1) {		// 電文に未送信ブロックはあるか
				// 未送信ブロックなし
				// 1電文送信完了、送信バッファから電文削除
				NTComData_DeleteTelegram(last_telegram);
				NTComComm_Ctrl_Slave.ack02_receive_count = 0;	//	#002
			}
		}
	default:
		break;
	}

}


/*[]-----------------------------------------------------------------------[]*
 *|			slave_check_receive_data()
 *[]-----------------------------------------------------------------------[]*
 *|				受信した電文の正当性をチェックする｡
 *|	param
 *|	return		応答詳細を返す
 *[]-----------------------------------------------------------------------[]*/
uchar slave_check_receive_data () {

	uchar response_detail = NT_RES_NORMAL;
	uchar ret;

	// CRCエラーをチェック
	if (NTComComm_Ctrl_Slave.receive_packet_crc_status == FALSE) {
		NTComComm_Ctrl_Slave.receive_crc_err_count++;
		// CRCエラー
		if (NTComComm_Ctrl_Slave.receive_crc_err_count > NTCom_InitData.Retry) {// CRCエラーリトライ回数以上？
			NTComComm_Ctrl_Slave.receive_crc_err_count = 0;
			// CRCエラーリトライオーバー		ACK01
			response_detail = NT_RES_CRC_RETRY_OVER;
			NTCom_err_chk(NT_ERR_CRC_DATA, 2, 0, 0, NULL);
		} else {
			// CRCリトライ回数未満				NAK80
			response_detail = NT_RES_CRC_ERR;
		}

	} else {
		// CRCは正常
		ret = NTComData_SetRcvBlock(NTComSlave_receive_blk);
		switch (ret) {		// 電文を受信バッファにセット
		case NT_DATA_NORMAL:						/* 成功 */
			// ACK電文								ACK00
			response_detail = NT_RES_NORMAL;
			NTComComm_Ctrl_Slave.receive_crc_err_count = 0;	// 正常に受信できたので０にする。
			NTComComm_Ctrl_Slave.block_receive_count++;	// 受信ブロック数
			break;
		case NT_DATA_BLOCK_INVALID1:				/* ACK02 or NT_NORMAL_BLKNUM_MAX待っても最終ブロックが来ない場合 */
			// ACK02
			response_detail = NT_RES_BLOCKNO_ERR1;
			NTComComm_Ctrl_Slave.block_receive_count++;	// 受信ブロック数
			NTCom_err_chk(NT_ERR_BLK_NT, 2, 0, 0, NULL);
			break;
		case NT_DATA_BLOCK_INVALID2:				/* ACK03 */
			// ACK03
			response_detail = NT_RES_BLOCKNO_ERR2;
			NTComComm_Ctrl_Slave.block_receive_count++;	// 受信ブロック数
			break;
		case NT_DATA_BUFFERFULL:				/* 通常バッファが一杯でセットできない */
			// 基本的にありえない
			response_detail = NT_RES_BLOCKNO_ERR1;
			NTCom_err_chk(NT_ERR_BLK_OVER_NT, 2, 0, 0, NULL);
			break;
		case NT_DATA_NO_MORE_TELEGRAM:			/* すでに１電文存在している */
			// 基本的にありえない
			response_detail = NT_RES_BUFFER_FULL;
			break;
		default:
			break;
		}
	}

	return response_detail;
}


/*[]-----------------------------------------------------------------------[]*
 *|			slave_check_crc_ctrl_tele()
 *[]-----------------------------------------------------------------------[]*
 *|			制御電文のCRCチェックを行う｡
 *|	param
 *|	return
 *[]-----------------------------------------------------------------------[]*/
void slave_check_crc_ctrl_tele( BOOL bCrc ) {

	if (bCrc) {
		NTComComm_Ctrl_Slave.receive_crc_err_count_ctrl = 0;
	} else {
		if (NTComComm_Ctrl_Slave.receive_crc_err_count_ctrl < NTCom_InitData.Retry) {
			NTComComm_Ctrl_Slave.receive_crc_err_count_ctrl++;
		}
		if (NTComComm_Ctrl_Slave.receive_crc_err_count_ctrl >= NTCom_InitData.Retry) {
			NTComComm_Ctrl_Slave.receive_crc_err_count_ctrl = 0;
			NTCom_err_chk(NT_ERR_CRC_CODE, 2, 0, 0, NULL);
		}
	}
}
// GM849100(E) 名鉄協商コールセンター対応（NT-NET端末間通信）（FT-4000N：MH364304流用）
