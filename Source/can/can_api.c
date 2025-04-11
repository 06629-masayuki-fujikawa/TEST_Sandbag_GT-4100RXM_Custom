//[]----------------------------------------------------------------------[]
///	@file		can_api.c
///	@brief		CANドライバAPI
/// @date		2012/01/19
///	@author		m.onouchi
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]

/*--- Include -------------------*/
#include <string.h>
#include <machine.h>
#include "config_can_api.h"
#include "iodefine.h"
#include "can_api.h"
#include	"can_def.h"

/*--- Pragma --------------------*/


/*--- Prototype -----------------*/
static void CanClearSentData(const unsigned long ch_nr, const unsigned long mbox_nr);
static unsigned long R_CAN_WaitTxRx(const unsigned long ch_nr, const unsigned long mbox_nr);
static void R_CAN_ConfigCANinterrupts(const unsigned long ch_nr);

/*--- Define --------------------*/
// ハードウェアに起因する障害でのロックを回避するためのタイマ
#define DEC_CHK_CAN_SW_TMR			(--can_tmo_cnt != 0)
#define RESET_CAN_SW_TMR			(can_tmo_cnt = MAX_CAN_SW_DELAY);
#define MAX_CAN_SW_DELAY			(0x8000)
#define CHECK_MBX_NR				{if (mbox_nr > 31) return R_CAN_SW_BAD_MBX;}

// Board specific port defines.
#define CAN_TRX_PDR(x, y)			CAN_TRX_PDR_PREPROC(x, y)
#define CAN_TRX_PDR_PREPROC(x, y)	(PORT ## x .PDR.BIT.B ## y)
#define CAN_TRX_PODR(x, y)			CAN_TRX_PODR_PREPROC(x, y)
#define CAN_TRX_PODR_PREPROC(x, y)	(PORT ## x .PODR.BIT.B ## y)

/*--- Gloval Value --------------*/


/*--- Extern --------------------*/


//[]----------------------------------------------------------------------[]
///	@brief			CANペリフェラル初期化
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: チャネル番号(1)
///	@return			R_CAN_OK				: 正常終了<br>
///					R_CAN_SW_BAD_MBX		: 存在しないメールボックス番号<br>
///					R_CAN_BAD_CH_NR			: 存在しないチャネル番号<br>
///					R_CAN_BAD_ACTION_TYPE:	: 未定義の設定指示<br>
///					R_CAN_SW_RST_ERR		: CANペリフェラルがリセットモードにならない<br>
///					R_CAN_SW_WAKEUP_ERR		: CANペリフェラルがウェイクアップしない<br>
///					R_CAN_SW_SLEEP_ERR		: CANペリフェラルがスリープモードになった<br>
///					R_CAN_SW_HALT_ERR		: CANペリフェラルがホルトモードにならない
///	@author			m.onouchi
///	@note			ビットレート，マスク，メールボックスのデフォルト，CAN 割り込みを設定する。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_Create(const unsigned long ch_nr)
{
	unsigned long api_status = R_CAN_OK;
	unsigned long i, j;
	unsigned long can_tmo_cnt = MAX_CAN_SW_DELAY;

	volatile struct st_can __evenaccess * can_block_p;

	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	/* Exit Sleep mode. */
	api_status |= R_CAN_Control(ch_nr, EXITSLEEP_CANMODE);	// スリープモードから復帰

	/* Sleep -> RESET mode. */
	api_status |= R_CAN_Control(ch_nr, RESET_CANMODE);		// リセットモードへ移行

	/*** Setting of CAN0 Control register.***/
	can_block_p->CTLR.BIT.BOM = 0;							// バスオフ復帰モード:ノーマル(ISO11898-1仕様準拠)
	can_block_p->CTLR.BIT.MBM = 1;							// FIFOメールボックスモード(FIFOを使用する)
	can_block_p->CTLR.BIT.IDFM = 0;							// 標準IDモード(拡張IDを使用しない)
	can_block_p->CTLR.BIT.MLM = 1;							// 未読時は受信しない（オーバーランモード）
	can_block_p->CTLR.BIT.TPM = 0;							// ID優先送信モード
	can_block_p->CTLR.BIT.TSRC = 0;							// タイムスタンプカウンタのリセットをしない
	can_block_p->CTLR.BIT.TSPS = 3;							// タイムスタンププリスケーラ:8ビットタイムごと

	/* Set BAUDRATE */
	R_CAN_SetBitrate(ch_nr);								// 通信速度設定

	can_block_p->MKIVLR.LONG = 0xFFFFFFFF;					// 全メールボックス:マスク無効
	can_block_p->TFCR.BIT.TFE = 0;							// 送信FIFO許可bit 0:送信FIFO禁止
	can_block_p->RFCR.BIT.RFE = 0;							// 受信FIFO許可bit 0:受信FIFO禁止

	/* Configure CAN interrupts. */ 
	R_CAN_ConfigCANinterrupts(ch_nr);						// CAN割り込み初期化

	/* Reset -> HALT mode */
	api_status |= R_CAN_Control(ch_nr, HALT_CANMODE);		// ホルトモードへ移行

	/* Configure mailboxes in Halt mode. */
	for (i = 0; i < 32; i++) {
		can_block_p->MB[i].ID.LONG = 0x00;
		can_block_p->MB[i].DLC = 0x00;
		for (j = 0; j < 8; j++) {
			can_block_p->MB[i].DATA[j] = 0x00;
		}
		for (j = 0; j < 2; j++) {
			can_block_p->MB[i].TS = 0x00;
		}
	}

	/* Halt -> OPERATION mode */
	/* Note: EST and BLIF flag go high here when stepping code in debugger. */
	api_status |= R_CAN_Control(ch_nr, OPERATE_CANMODE);	// 通常動作モードへ移行

	/* Time Stamp Counter reset. Set the TSRC bit to 1 in CAN Operation mode. */
	can_block_p->CTLR.BIT.TSRC = 1;									// タイムスタンプカウンタのリセットをする。
	while ((can_block_p->CTLR.BIT.TSRC) && DEC_CHK_CAN_SW_TMR) {	// レジスタの変化を確認
		;
	}
	if (can_tmo_cnt == 0) {		// タイムアウト
		api_status |= R_CAN_SW_TSRC_ERR;
	}

	/* Check for errors so far, report, and clear. */
	if (can_block_p->STR.BIT.EST) {		// エラー発生
		api_status |= R_CAN_SW_RST_ERR;
	}

	/* Clear Error Interrupt Factor Judge Register. */
	if (can_block_p->EIFR.BYTE) {		// バスエラー検出
										// エラーワーニング検出
										// エラーパッシブ検出
										// バスオフ開始検出
										// バスオフ復帰検出
										// 受信オーバラン検出
										// オーバロードフレーム送信検出
										// バスロック検出
		api_status |= R_CAN_SW_RST_ERR;
	}
	can_block_p->EIFR.BYTE = 0x00;		// エラー割り込み要因クリア

	/* Clear Error Code Store Register. */
	if (can_block_p->ECSR.BYTE) {		// スタッフエラー検出
										// フォームエラー検出
										// ACKエラー検出
										// CRCエラー検出
										// ビットエラー(レセシブ)検出
										// ビットエラー(ドミナント)検出
										// ACKデリミタエラー検出
										// 蓄積したエラーコードを出力
		api_status |= R_CAN_SW_RST_ERR;
	}
	can_block_p->ECSR.BYTE = 0x00;		// エラーコード格納レジスタクリア

	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			CANトランシーバポートの端子設定
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: チャネル番号(1)
///	@param[in]		action_type				: ENABLE=端子の有効化<br>
///											: DISABLE=端子の無効化<br>
///											: CANPORT_TEST_LISTEN_ONLY=リッスンオンリモード<br>
///											: CANPORT_TEST_0_EXT_LOOPBACK=セルフテストモード0(外部ループバック)<br>
///											: CANPORT_TEST_1_INT_LOOPBACK=セルフテストモード1(内部ループバック)<br>
///											: CANPORT_RETURN_TO_NORMAL=通常動作モードへの復帰
///	@return			R_CAN_OK				: 正常終了<br>
///					R_CAN_SW_BAD_MBX		: 存在しないメールボックス番号<br>
///					R_CAN_BAD_CH_NR			: 存在しないチャネル番号<br>
///					R_CAN_BAD_ACTION_TYPE:	: 未定義の設定指示<br>
///					R_CAN_SW_RST_ERR		: CANペリフェラルがリセットモードにならない<br>
///					R_CAN_SW_HALT_ERR		: CANペリフェラルがホルトモードにならない
///	@author			m.onouchi
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_PortSet(const unsigned long ch_nr, const unsigned long action_type)
{  
	unsigned long api_status = R_CAN_OK;
	volatile struct st_can __evenaccess * can_block_p;

	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	switch (action_type) {
	case ENABLE:
		/* P3_2 as CTX0 and P3_3 as CRX0. */
		PORT5.PDR.BIT.B4 = 1;				// P54出力設定(CTX1)
		PORT5.PDR.BIT.B5 = 0;				// P55入力設定(CRX1)
		break;

	case DISABLE:
		break;

	/* Run in Listen Only test mode. */
	case CANPORT_TEST_LISTEN_ONLY:
		api_status = R_CAN_Control(0, HALT_CANMODE);		// ホルトモードへ移行
		can_block_p->TCR.BYTE = 0x03;						// CANテストモード許可
															// リッスンオンリモード
		api_status |= R_CAN_Control(0, OPERATE_CANMODE);	// 通常動作モードへ移行
		api_status |= R_CAN_PortSet(0, ENABLE);				// 端子の有効化
		break;

	/* Run in External Loopback test mode. */
	case CANPORT_TEST_0_EXT_LOOPBACK:
		api_status = R_CAN_Control(0, HALT_CANMODE);		// ホルトモードへ移行
		can_block_p->TCR.BYTE = 0x05;						// CANテストモード許可
															// セルフテストモード0(外部ループバック)
		api_status |= R_CAN_Control(0, OPERATE_CANMODE);	// 通常動作モードへ移行
		api_status |= R_CAN_PortSet(0, ENABLE);				// 端子の有効化
		break;

	/* Run in Internal Loopback test mode. */
	case CANPORT_TEST_1_INT_LOOPBACK:
		api_status = R_CAN_Control(0, HALT_CANMODE);		// ホルトモードへ移行
		can_block_p->TCR.BYTE = 0x07;						// CANテストモード許可
															// セルフテストモード1(内部ループバック)
		api_status |= R_CAN_Control(0, OPERATE_CANMODE);	// 通常動作モードへ移行
		break;

	/* Return to default CAN bus mode. */
	case CANPORT_RETURN_TO_NORMAL:
		api_status = R_CAN_Control(0, HALT_CANMODE);		// ホルトモードへ移行
		can_block_p->TCR.BYTE = 0x00;						// CANテストモード禁止
															// CANテストモードではない
		api_status |= R_CAN_Control(0, OPERATE_CANMODE);	// 通常動作モードへ移行
		api_status |= R_CAN_PortSet(0, ENABLE);				// 端子の有効化
		break;

	default:	// 未定義の設定指示
		api_status = R_CAN_BAD_ACTION_TYPE;
		break;
	}
	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN動作モード設定
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: チャネル番号(1)
///	@param[in]		action_type				: EXITSLEEP_CANMODE=スリープモードから復帰<br>
///											: ENTERSLEEP_CANMODE=スリープモードへ移行<br>
///											: RESET_CANMODE=リセットモードへ移行<br>
///											: HALT_CANMODE=ホルトモードへ移行<br>
///											: OPERATE_CANMODE=通常動作モードへ移行
///	@return			R_CAN_OK				: 正常終了<br>
///					R_CAN_SW_BAD_MBX		: 存在しないメールボックス番号<br>
///					R_CAN_BAD_CH_NR			: 存在しないチャネル番号<br>
///					R_CAN_BAD_ACTION_TYPE:	: 未定義の設定指示<br>
///					R_CAN_SW_RST_ERR		: CANペリフェラルがリセットモードにならない<br>
///					R_CAN_SW_WAKEUP_ERR		: CANペリフェラルがウェイクアップしない<br>
///					R_CAN_SW_SLEEP_ERR		: CANペリフェラルがスリープモードにならない<br>
///					R_CAN_SW_HALT_ERR		: CANペリフェラルがホルトモードにならない
///	@author			m.onouchi
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_Control(const unsigned long ch_nr, const unsigned long action_type)
{
	unsigned long api_status = R_CAN_OK;
	unsigned long can_tmo_cnt = MAX_CAN_SW_DELAY;
	volatile struct st_can __evenaccess * can_block_p;

	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	switch (action_type) {
	case EXITSLEEP_CANMODE:		// スリープモードから復帰
		can_block_p->CTLR.BIT.SLPM = CAN_NOT_SLEEP;						// CANスリープモードを解除
		while ((can_block_p->STR.BIT.SLPST) && DEC_CHK_CAN_SW_TMR) {	// ステータスレジスタでモードの変化を確認
			nop();
		}
		if (can_tmo_cnt == 0) {	// タイムアウト
			api_status = R_CAN_SW_WAKEUP_ERR;
		}
		break;

	case ENTERSLEEP_CANMODE:	// スリープモードへ移行
		can_block_p->CTLR.BIT.SLPM = CAN_SLEEP;							// CANスリープモードに変更
		while ((!can_block_p->STR.BIT.SLPST) && DEC_CHK_CAN_SW_TMR) {	// ステータスレジスタでモードの変化を確認
			nop();
		}
		if (can_tmo_cnt == 0) {	// タイムアウト
			api_status = R_CAN_SW_SLEEP_ERR;
		}
		break;

	case RESET_CANMODE:			// リセットモードへ移行
		can_block_p->CTLR.BIT.CANM = CAN_RESET;							// CANリセットモードに変更
		while ((!can_block_p->STR.BIT.RSTST) && DEC_CHK_CAN_SW_TMR) {	// ステータスレジスタでモードの変化を確認
			;
		}
		if (can_tmo_cnt == 0) {	// タイムアウト
			api_status = R_CAN_SW_RST_ERR;
		}
		break;

	case HALT_CANMODE:			// ホルトモードへ移行
		can_block_p->CTLR.BIT.CANM = CAN_HALT;							// CANホルトモードに変更
		while ((!can_block_p->STR.BIT.HLTST) && DEC_CHK_CAN_SW_TMR) {	// ステータスレジスタでモードの変化を確認
			;
		}
		if (can_tmo_cnt == 0) {	// タイムアウト
			api_status = R_CAN_SW_HALT_ERR;
		}
		break;

	case OPERATE_CANMODE:		// 通常動作モードへ移行
		MSTP_CAN1  =  0;												// CAN1のモジュールストップ状態を解除
		while ((MSTP_CAN1) && DEC_CHK_CAN_SW_TMR) {						// レジスタの変化を確認
			nop();
		}
		can_block_p->CTLR.BIT.CANM = CAN_OPERATION;						// CANオペレーションモードに変更

		while ((can_block_p->STR.BIT.RSTST) && DEC_CHK_CAN_SW_TMR) {	// ステータスレジスタでモードの変化を確認
			;
		}
		if (can_tmo_cnt == 0) {	// タイムアウト
			api_status = R_CAN_SW_RST_ERR;
		}
		break;

	default:	// 未定義の設定指示
		api_status = R_CAN_BAD_ACTION_TYPE;
		break;
	}

	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			送信メールボックスの設定
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: チャネル番号(1)
///	@param[in]		mbox_nr					: メールボックス番号(0-31)<br>
///					*frame_p				: データフレーム<br>
///					frame_type				: DATA_FRAME=通常のデータフレーム<br>
///											: REMOTE_FRAME=リモートデータフレーム
///	@return			R_CAN_OK				: 正常終了<br>
///					R_CAN_SW_BAD_MBX		: 存在しないメールボックス番号<br>
///					R_CAN_BAD_CH_NR			: 存在しないチャネル番号<br>
///					R_CAN_BAD_ACTION_TYPE:	: 未定義の設定指示<br>
///	@author			m.onouchi
///	@note			指定されたフレームをメールボックスに書き込みR_CAN_Txをコールしてバスに送信する。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_TxSet(const unsigned long ch_nr, const unsigned long mbox_nr, const can_std_frame_t *frame_p, const unsigned long frame_type)
{
	unsigned long api_status = R_CAN_OK;
	unsigned long i;
	volatile struct st_can __evenaccess * can_block_p;

	CHECK_MBX_NR		// 引数mbox_nrの範囲チェック
	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	/* Wait for any previous transmission to complete. */
	api_status = R_CAN_WaitTxRx(0, mbox_nr);				// 前の送信が完了するまで待つ

	/* Interrupt disable the mailbox.in case it was a receive mailbox */
	can_block_p->MIER.LONG &= ~(bit_set[mbox_nr]);			// メールボックス割り込み禁止

	/* Clear message mailbox control register (trmreq to 0). */
	can_block_p->MCTL[mbox_nr].BYTE = 0;					// メッセージ制御レジスタクリア

	/*** Set Mailbox. ***/
	/* Set CAN message mailbox buffer Standard ID */
	can_block_p->MB[mbox_nr].ID.BIT.SID = frame_p->id;		// IDセット

	/* Set the Data Length Code */
	can_block_p->MB[mbox_nr].DLC = frame_p->dlc;			// データ長セット

 	/* Frame select: Data frame = 0, Remote = 1 */
	if (frame_type == REMOTE_FRAME) {
		can_block_p->MB[mbox_nr].ID.BIT.RTR = 1;			// リモートフレームを選択
	} else {
		can_block_p->MB[mbox_nr].ID.BIT.RTR = 0;			// データフレームを選択
	}

	/* Frame select: Standard = 0, Extended = 1 */
	can_block_p->MB[mbox_nr].ID.BIT.IDE = 0;				// 標準ID

	/* Copy frame data into mailbox */
	for (i = 0; ((i < frame_p->dlc) && (i<8)); i++) {		// データコピー
		can_block_p->MB[mbox_nr].DATA[i] = frame_p->data[i];
	}

	/* Interrupt enable the mailbox */
	can_block_p->MIER.LONG |= (bit_set[mbox_nr]);			// メールボックス割り込み許可

	R_CAN_Tx(ch_nr, mbox_nr);	// メッセージ送信指示

	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			メッセージ送信指示
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: チャネル番号(1)
///	@param[in]		mbox_nr					: メールボックス番号(0-31)<br>
///	@return			R_CAN_OK				: 正常終了<br>
///					R_CAN_SW_BAD_MBX		: 存在しないメールボックス番号<br>
///					R_CAN_BAD_CH_NR			: 存在しないチャネル番号<br>
///					R_CAN_SW_SET_TX_TMO:	: 前の送信の終了待ちがタイムアウトした。<br>
///					R_CAN_SW_SET_RX_TMO:	: 前の受信の終了待ちがタイムアウトした。
///	@author			m.onouchi
///	@note			指定されたメールボックスにフレームの送信を指示する。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_Tx(const unsigned long ch_nr, const unsigned long mbox_nr)
{
	unsigned long api_status = R_CAN_OK;
	volatile struct st_can __evenaccess * can_block_p;

	CHECK_MBX_NR		// 引数mbox_nrの範囲チェック
	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	/* Wait for any previous transmission to complete. */
	api_status = R_CAN_WaitTxRx(0, mbox_nr);		// 前の送信が完了するまで待つ

	/* Clear SentData flag since we are about to send anew. */
	CanClearSentData(ch_nr, mbox_nr);				// 前データの送信状態解除
	
	/* Set TrmReq bit to "1" */
	can_block_p->MCTL[mbox_nr].BIT.TX.TRMREQ = 1;	// 送信要求
	
 	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			メッセージ送信完了チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: チャネル番号(1)
///	@param[in]		mbox_nr					: メールボックス番号(0-31)<br>
///	@return			R_CAN_OK				: 送信完了<br>
///					R_CAN_SW_BAD_MBX		: 存在しないメールボックス番号<br>
///					R_CAN_BAD_CH_NR			: 存在しないチャネル番号<br>
///					R_CAN_MSGLOST			: メッセージは上書きされたか失われた。<br>
///					R_CAN_NO_SENTDATA		: メッセージが送信されなかった。
///	@author			m.onouchi
///	@note			指定されたメールボックスからデータフレームが送信されたことを確認する。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_TxCheck(const unsigned long ch_nr, const unsigned long mbox_nr)
{
	unsigned long api_status = R_CAN_OK;
	volatile struct st_can __evenaccess * can_block_p;

	CHECK_MBX_NR		// 引数mbox_nrの範囲チェック
	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	if (can_block_p->MCTL[mbox_nr].BIT.TX.SENTDATA == 0) {
		api_status = R_CAN_NO_SENTDATA;
	} else {								// 送信完了
		/* Clear SentData flag. */
		CanClearSentData(ch_nr, mbox_nr);	// 前データの送信状態解除
	}

	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			メッセージ送信の中止(abort)
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: チャネル番号(1)
///	@param[in]		mbox_nr					: メールボックス番号(0-31)<br>
///	@return			R_CAN_OK				: 正常終了<br>
///					R_CAN_SW_BAD_MBX		: 存在しないメールボックス番号<br>
///					R_CAN_BAD_CH_NR			: 存在しないチャネル番号<br>
///					R_CAN_SW_ABORT_ERR		: アボート待ちタイムアウト
///	@author			m.onouchi
///	@attention		データが既に送信済みの場合はR_CAN_SW_ABORT_ERRが返ります。
///	@note			指定されたメールボックスのデータ送信を中止する。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_TxStopMsg(const unsigned long ch_nr, const unsigned long mbox_nr)
{
	unsigned long api_status = R_CAN_OK;
	unsigned long can_tmo_cnt = MAX_CAN_SW_DELAY;   
	volatile struct st_can __evenaccess * can_block_p;

	CHECK_MBX_NR		// 引数mbox_nrの範囲チェック
	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	/* Clear message mailbox control register. Setting TRMREQ to 0 should abort. */
	can_block_p->MCTL[mbox_nr].BYTE = 0;			// メッセージ制御レジスタクリア

	/* Wait for abort. */
	while ((can_block_p->MCTL[mbox_nr].BIT.TX.TRMABT) && DEC_CHK_CAN_SW_TMR) {	// 送信アボートの完了待ち
		;
	}
	if (can_tmo_cnt == 0) {		// タイムアウト
		api_status = R_CAN_SW_ABORT_ERR;
	}

	/* Clear abort flag. */
	can_block_p->MCTL[mbox_nr].BIT.TX.TRMABT = 0;	// 送信アボート完了フラグクリア

	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			前データの送信状態解除
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr		: チャネル番号(1)
///	@param[in]		mbox_nr		: メールボックス番号(0-31)
///	@return			void
///	@author			m.onouchi
///	@note			指定されたメールボックスに対して次のフレーム送信を開始できる状態にする。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
#pragma inline(CanClearSentData)
static void CanClearSentData(const unsigned long ch_nr, const unsigned long mbox_nr)
{
	volatile struct st_can __evenaccess * can_block_p;

	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return;
	}

	/* Clear SentData to 0 *after* setting TrmReq to 0. */
	can_block_p->MCTL[mbox_nr].BIT.TX.TRMREQ = 0;	// 次の送信を要求するための手続き①
	nop();
	can_block_p->MCTL[mbox_nr].BIT.TX.SENTDATA = 0;	// 次の送信を要求するための手続き②
}


//[]----------------------------------------------------------------------[]
///	@brief			メッセージ受信指示
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: チャネル番号(1)
///	@param[in]		mbox_nr					: メールボックス番号(0-31)<br>
///	@param[in]		sid						: 受信するCAN ID(0x0000-0x07FF)
///					frame_type				: DATA_FRAME=通常のデータフレーム<br>
///											: REMOTE_FRAME=リモートデータフレーム
///	@return			R_CAN_OK				: 正常終了<br>
///					R_CAN_SW_BAD_MBX		: 存在しないメールボックス番号<br>
///					R_CAN_BAD_CH_NR			: 存在しないチャネル番号<br>
///					R_CAN_SW_SET_TX_TMO:	: 前の送信の終了待ちがタイムアウトした。<br>
///					R_CAN_SW_SET_RX_TMO:	: 前の受信の終了待ちがタイムアウトした。
///	@author			m.onouchi
///	@note			指定されたIDを持つデータフレームを受信するようメールボックスに指示する。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_RxSet(const unsigned long ch_nr, const unsigned long mbox_nr, const unsigned long sid, const unsigned long frame_type)
{
	unsigned long api_status = R_CAN_OK;
	volatile struct st_can __evenaccess * can_block_p;

	CHECK_MBX_NR		// 引数mbox_nrの範囲チェック
	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	/* Wait for any previous transmission/reception to complete. */
	api_status = R_CAN_WaitTxRx(0, mbox_nr);		// 前の送信が完了するまで待つ

	/* Interrupt disable the mailbox. */
	// NOTE:FIFO受信モードの場合はMBX_28を使用する
	can_block_p->MIER.LONG &= ~(bit_set[mbox_nr]);	// メールボックス割り込み禁止
	//NOTE:MB29に該当する受信FIFO割込発生タイミング制御ビットは「0：毎回受信完了で発生」とする。(初期化で0としてあるのでここでは変更しない)

	/* Clear message mailbox control register. */
	can_block_p->RFCR.BYTE = 0;						// 受信FIFO制御レジスタクリア

	// NOTE:FIFO受信モードの場合はFIDCR1を使用する
	/*** Set Mailbox. ***/
	can_block_p->FIDCR1.BIT.SID = sid;		// IDセット
	can_block_p->FIDCR1.BIT.EID = 0;
	/* Dataframe = 0, Remote frame = 1	*/
	if (frame_type == REMOTE_FRAME) {
		can_block_p->FIDCR1.BIT.RTR = 1;	// リモートフレームを選択
	} else {
		can_block_p->FIDCR1.BIT.RTR = 0;	// データフレームを選択
	}
	/* Frame select: Standard = 0, Extended = 1 */
	can_block_p->FIDCR1.BIT.IDE = 0;		// 標準ID

	/* Interrupt enable the mailbox */
	can_block_p->MIER.LONG |= (bit_set[mbox_nr]);	// メールボックス割り込み許可

	/* Request to receive the frame with RecReq bit. */
	can_block_p->RFCR.BIT.RFE = 1;							// 受信FIFO許可bit 1:受信FIFO許可

	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			メッセージ受信完了チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: チャネル番号(1)
///	@param[in]		mbox_nr					: メールボックス番号(0-31)<br>
///	@return			R_CAN_OK				: 受信メッセージあり<br>
///					R_CAN_NOT_OK			: 受信メッセージなし<br>
///					R_CAN_SW_BAD_MBX		: 存在しないメールボックス番号<br>
///					R_CAN_BAD_CH_NR			: 存在しないチャネル番号<br>
///					R_CAN_RXPOLL_TMO		: 受信中のメッセージがタイムアウトした。
///	@author			m.onouchi
///	@note			指定されたメールボックスがデータフレームを受信したことを確認する。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_RxPoll(const unsigned long ch_nr, const unsigned long mbox_nr)
{
	unsigned long api_status = R_CAN_NOT_OK;
	unsigned long poll_delay = MAX_CAN_REG_POLLTIME;
	volatile struct st_can __evenaccess * can_block_p;

	CHECK_MBX_NR		// 引数mbox_nrの範囲チェック
	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	/* Wait if new data is currently being received. */
	while ((can_block_p->MCTL[mbox_nr].BIT.RX.INVALDATA) && poll_delay) {	// メッセージ受信中(待ち)
		poll_delay--;
	}
	if (poll_delay == 0) {	// タイムアウト
		api_status = R_CAN_RXPOLL_TMO;
	} else {
		if (can_block_p->MCTL[mbox_nr].BIT.RX.NEWDATA == 1) {	// 新メッセージ受信完了
			api_status = R_CAN_OK;
		}
	}
	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			受信メッセージの取り出し
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: チャネル番号(1)
///	@param[in]		mbox_nr					: メールボックス番号(0-31)<br>
///	@param[out]		*frame_p				: データフレーム
///	@return			R_CAN_OK				: 受信メッセージあり<br>
///					R_CAN_SW_BAD_MBX		: 存在しないメールボックス番号<br>
///					R_CAN_BAD_CH_NR			: 存在しないチャネル番号<br>
///					R_CAN_MSGLOST			: メッセージは上書きされたか失われた。
///	@author			m.onouchi
///	@note			指定したメールボックスからデータフレームを取得する。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_RxRead(const unsigned long ch_nr, const unsigned long mbox_nr, can_std_frame_t *const frame_p)
{
	unsigned long i;
	unsigned long api_status = R_CAN_OK;
	volatile struct st_can __evenaccess * can_block_p;

	CHECK_MBX_NR		// 引数mbox_nrの範囲チェック
	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	if(can_block_p->RFCR.BIT.RFEST == 0){	// 受信FIFO未読メッツセージ有り
		api_status = R_CAN_OK;
		if(can_block_p->RFCR.BIT.RFMLF){// メッセージロスト発生
			can_block_p->RFCR.BIT.RFMLF = 0;
			// オーバーロード（メッセージロスト）エラーの登録
			g_ucCAN1LostErr = 1;
		}
		//	/* Copy received data from message mailbox to memory */
		frame_p->id = can_block_p->MB[mbox_nr].ID.BIT.SID;				// ID取得
		frame_p->dlc = can_block_p->MB[mbox_nr].DLC;					// データ長取得
		for (i = 0; i < can_block_p->MB[mbox_nr].DLC; i++) {			// データコピー
			frame_p->data[i] = can_block_p->MB[mbox_nr].DATA[i];
		}
		can_block_p->RFPCR = 0xFF;//FIFOのポインタ更新
	}else{
		api_status = R_CAN_NOT_OK;
	}

	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			メッセージの受け入れマスク設定
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: チャネル番号(1)
///	@param[in]		mbox_nr					: メールボックス番号(0-31)<br>
///	@param[in]		sid_mask_value			: マスク値
///	@return			void
///	@author			m.onouchi
///	@attention		マスクの設定は連続する4つのメールボックスで共有されます。
///	@note			指定したメールボックスにフィルタをかけて受信するメッセージを制限する。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void R_CAN_RxSetMask(const unsigned long ch_nr, const unsigned long mbox_nr, const unsigned long sid_mask_value)
{
	volatile struct st_can __evenaccess * can_block_p;

	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return;
	}

	/* Write to MKR0 to MKR7 in CAN reset mode or CAN halt mode. */
	R_CAN_Control(0, HALT_CANMODE);							// ホルトモードへ移行

	/* Set mask for the goup of mailboxes. */
	can_block_p->MKR[mbox_nr/4].BIT.SID = sid_mask_value;	// マスク値を設定

	/* Set mailbox mask to be used. (0 = mask VALID.) */
//NOTE:FIFO受信モードの場合はMBX28～MBX31を「マスク有効」とする
	can_block_p->MKIVLR.LONG &= ~(bit_set[CANBOX_RX_28]);		// マスク有効化
	can_block_p->MKIVLR.LONG &= ~(bit_set[CANBOX_RX_29]);		// マスク有効化
	can_block_p->MKIVLR.LONG &= ~(bit_set[CANBOX_RX_30]);		// マスク有効化
	can_block_p->MKIVLR.LONG &= ~(bit_set[CANBOX_RX_31]);		// マスク有効化

	R_CAN_Control(0, OPERATE_CANMODE);						// 通常動作モードへ移行
}


//[]----------------------------------------------------------------------[]
///	@brief			前の送信が完了するまで待つ
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr					: チャネル番号(1)
///	@param[in]		mbox_nr					: メールボックス番号(0-31)
///	@return			R_CAN_OK				: 受信メッセージあり<br>
///					R_CAN_SW_BAD_MBX		: 存在しないメールボックス番号<br>
///					R_CAN_BAD_CH_NR			: 存在しないチャネル番号<br>
///					R_CAN_SW_SET_TX_TMO:	: 前の送信の終了待ちがタイムアウトした。<br>
///					R_CAN_SW_SET_RX_TMO:	: 前の受信の終了待ちがタイムアウトした。
///	@author			m.onouchi
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
#pragma inline(R_CAN_WaitTxRx)
static unsigned long R_CAN_WaitTxRx(const unsigned long ch_nr, const unsigned long mbox_nr)
{
	unsigned long api_status = R_CAN_OK;
	unsigned long can_tmo_cnt = MAX_CAN_SW_DELAY;
	volatile struct st_can __evenaccess * can_block_p;

	CHECK_MBX_NR		// 引数mbox_nrの範囲チェック
	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	/* Wait for any previous transmission to complete. */
	if (can_block_p->MCTL[mbox_nr].BIT.TX.TRMREQ) {		// 送信要求あり
		while ((can_block_p->MCTL[mbox_nr].BIT.TX.SENTDATA == 1) && DEC_CHK_CAN_SW_TMR) {	// 送信完了待ち
			;
		}
		if (can_tmo_cnt == 0) {	// タイムアウト
			api_status = R_CAN_SW_SET_TX_TMO;
		}
	} else if (can_block_p->MCTL[mbox_nr].BIT.TX.RECREQ) {	// 受信要求あり
		while ((can_block_p->MCTL[mbox_nr].BIT.RX.INVALDATA == 1) && DEC_CHK_CAN_SW_TMR) {	// 受信完了待ち
			;
		}
		if (can_tmo_cnt == 0) {	// タイムアウト
			api_status = R_CAN_SW_SET_RX_TMO;
		}
	}
	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			CANユニットの状態チェック
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr						: チャネル番号(1)
///	@return			R_CAN_STATUS_ERROR_ACTIVE	: エラーアクティブ<br>
///					R_CAN_STATUS_ERROR_PASSIVE	: エラーパッシブ<br>
///					R_CAN_STATUS_BUSOFF			: バスオフ
///	@author			m.onouchi
///	@note			CANペリフェラルのステータスフラグをチェックして現在の状態を返す。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
unsigned long R_CAN_CheckErr(const unsigned long ch_nr)
{
	unsigned long api_status = R_CAN_STATUS_ERROR_ACTIVE;
	volatile struct st_can __evenaccess * can_block_p;

	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return R_CAN_BAD_CH_NR;
	}

	if (can_block_p->STR.BIT.EST) {
		if (can_block_p->STR.BIT.EPST) {
			api_status = R_CAN_STATUS_ERROR_PASSIVE;
		} else if (can_block_p->STR.BIT.BOST) {
			api_status = R_CAN_STATUS_BUSOFF;
		}
	}

	return api_status;
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN通信速度設定
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr		: チャネル番号(1)
///	@return			void
///	@author			m.onouchi
///	@note			config_can_api.hで定義された値でCANの通信速度を設定する。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void R_CAN_SetBitrate(const unsigned long ch_nr)
{
	volatile struct st_can __evenaccess * can_block_p;

	if (ch_nr == 1) {
		can_block_p = (struct st_can __evenaccess *) 0x91200;
	} else {
		return;
	}

	/* Set TSEG1, TSEG2 and SJW. */
	can_block_p->BCR.BIT.BRP = CAN_BRP - 1;
	can_block_p->BCR.BIT.TSEG1 = CAN_TSEG1 - 1;
	can_block_p->BCR.BIT.TSEG2 = CAN_TSEG2 - 1;
	can_block_p->BCR.BIT.SJW = CAN_SJW - 1;
}


//[]----------------------------------------------------------------------[]
///	@brief			CAN割り込み初期化
//[]----------------------------------------------------------------------[]
///	@param[in]		ch_nr	: チャネル番号(1)
///	@return			void
///	@author			m.onouchi
///	@attention		メールボックスの割り込みは使用時に個別に許可する。
//[]----------------------------------------------------------------------[]
///	@date			Create	: 2012/01/19<br>
///					Update	: 
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void R_CAN_ConfigCANinterrupts(const unsigned long ch_nr)
{
	if (ch_nr == 1) {
		/* Configure CAN Tx interrupt. */
		IEN(CAN1, TXM1) = 1;				// vector 55(CAN1 TXM1):割り込み要求許可
		IPR(CAN1, TXM1) = CAN1_INT_LVL;		// 割り込み優先レベル

		/* Configure CAN Rx interrupt. */
		IEN(CAN1, RXF1) = 1;				// vector 52(CAN1 RXF1):割り込み要求許可
		IPR(CAN1, RXF1) = CAN1_INT_LVL_RCV;		// 割り込み優先レベル

		/* Configure CAN Error interrupt. */
		ICU.IER[IER_ICU_GROUPE0].BIT.IEN2 = 1;	// vector 106:割り込み要求許可
		ICU.GEN[GEN_CAN1_ERS1].BIT.EN1 = 1;		// CAN1 ERS1:割り込み要求許可
		ICU.IPR[IPR_ICU_GROUPE0].BIT.IPR = CAN1_INT_LVL;	// 割り込み優先レベル

		CAN1.EIER.BYTE = 0x18;				// バスエラー割り込み禁止
											// エラーワーニング割り込み禁止
											// エラーパッシブ割り込み禁止
											// バスオフ開始割り込み許可
											// バスオフ復帰割り込み許可
											// 受信オーバラン割り込み禁止
											// オーバロードフレーム送信割り込み禁止
											// バスロック割り込み禁止

		CAN1.MIER.LONG = 0x00000000;		// 全メールボックス割り込み禁止
	}
}
