/*[]----------------------------------------------------------------------[]*/
/*| idletask, message control                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : R.Hara                                                  |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
#include	<string.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"pri_def.h"
#include	"Message.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"rkn_fun.h"
#include	"ope_def.h"
#include	"mdl_def.h"
#include	"irq1.h"
#include	"flp_def.h"
#include	"lcd_def.h"
#include	"prm_tbl.h"
#include	"common.h"
#include	"ntnet.h"
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#include	"edy_def.h"
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
#include	"ntnetauto.h"
#include	"AppServ.h"
#include	"remote_dl.h"
#include	"can_api.h"
#include	"can_def.h"
#include	"fb_def.h"
#include 	"ksg_def.h"
#include	"I2c_driver.h"
#include	"rtc_readwrite.h"
#include	"updateope.h"
#include	"ifm_ctrl.h"
#include	"raudef.h"
#if (1 == AUTO_PAYMENT_PROGRAM)

#include	"LKcom.h"
#include	"LKmain.h"
#include	"tbl_rkn.h"

static void AutoPaymentIn(t_AutoPayment_In *In);
static void AutoPaymentOut(t_AutoPayment_Out *Out);

#endif

#if (1 == AUTO_PAYMENT_PROGRAM)
static	void	AutoPayment_sub(void);
#endif

extern unsigned short PRG_CPY_S_TM;
extern unsigned short PRG_CPY_R_TM;
// MH321800(S) G.So ICクレジット対応
extern volatile short TXRXWAIT;
// MH321800(E) G.So ICクレジット対応


/*[]----------------------------------------------------------------------[]*/
/*| idletask                                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : idletask                                                |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hashimoto, modified by Hara                             |*/
/*| Date         : 2005-02-01                                              |*/
/*| UpDate       :                                                         |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	idletask( void )
{																// 起動時の初期化処理

// MH810100(S) K.Onodera  2020/02/21 #3895 車番チケットレス(予約のRSWで起動時、初期画面表示NGとなる不具合修正)
//	switch( read_rotsw() ){										// 音声データインストールモード
//		case 5:
//			for( ; ; ) {
//				taskchg( OPETSKNO );										// Change to opration task
//				WACDOG;
//				taskchg( KSGTSKNO );
//				WACDOG;
//				taskchg( FLATSKNO );										// Change to flashrom task
//				WACDOG;
//			}
//			break;
//		case 6:
//			for( ; ; ) {
//				taskchg( OPETSKNO );										// Change to opration task
//				WACDOG;
//			}
//			break;
//	}
// MH810100(E) K.Onodera  2020/02/21 #3895 車番チケットレス(予約のRSWで起動時、初期画面表示NGとなる不具合修正)

	for( ; ; ) {
#if (1 == AUTO_PAYMENT_PROGRAM)
		AutoPayment_sub();
#endif
		taskchg( OPETSKNO );										// Change to opration task
		WACDOG;

// MH810100(S) K.Onodera 2020/04/07 車番チケットレス（Kasagoタスク負荷軽減）
		taskchg(KSGTSKNO);
		WACDOG;
// MH810100(E) K.Onodera 2020/04/07 車番チケットレス（Kasagoタスク負荷軽減）

		if( CNMTSK_START ){
			taskchg( CNMTSKNO );									// Change to coin&note control task
			WACDOG;
// MH321800(S) G.So ICクレジット対応
			while(TXRXWAIT > 0) {
			// phase 3 動作中はタイムアウトまで占有する
			// ※コマンド送信後、リーダーからのACK待ちタイムアウトとなりエラー登録されてしまうケースがあるため
			//   その対策として、phase 3のみタスクを最大60ms占有する
				taskchg( CNMTSKNO );								// Change to coin&note control task
				WACDOG;
			}
// MH321800(E) G.So ICクレジット対応
		}

// MH810100(S) K.Onodera 2019/10/17 車番チケットレス（フラップ非対応化）
//		taskchg( LKCOMTSKNO );										// Change to lkcom task
//		WACDOG;
		taskchg( PKTTSKNO );										// Change to PKT task
		WACDOG;
// MH810100(E) K.Onodera 2019/10/17 車番チケットレス（フラップ非対応化）

		taskchg(KSGTSKNO);
		WACDOG;

		taskchg( FLATSKNO );										// Change to flashrom task
		WACDOG;
// MH810100(S) K.Onodera 2020/04/07 車番チケットレス（Kasagoタスク負荷軽減）
		taskchg(KSGTSKNO);
		WACDOG;
// MH810100(E) K.Onodera 2020/04/07 車番チケットレス（Kasagoタスク負荷軽減）
		taskchg( PRNTSKNO );										// Change to coin&note control task
		WACDOG;
		ntautoSendCtrl();
// MH810100(S) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
//		ntautoSendCtrl_pcarsWeb();									//Web用駐車台数データ要求
// MH810100(E) K.Onodera  2019/12/20 車番チケットレス(フラップ式->ゲート式変更に伴う処理見直し)
		WACDOG;

		if(tcb[CANTCBNO].event == MSG_SET || canevent() != 0){
			taskchg(CANTSKNO);										// Change to can Send Receive task
			WACDOG;
		}
// MH810100(S) K.Onodera 2019/10/17 車番チケットレス（磁気リーダー非対応化）
//		cr_snd();													// ﾘｰﾀﾞｰﾃﾞｰﾀ送信
//
//		taskchg(MRDTSKNO);
//		WACDOG;
// MH810100(E) K.Onodera 2019/10/17 車番チケットレス（磁気リーダー非対応化）

		if(_is_ntnet_remote() || RAU_Credit_Enabale != 0) {
			taskchg(RAUTSKNO);											// rau task
			WACDOG;
		}
// MH810100(S) K.Onodera 2020/04/07 車番チケットレス（Kasagoタスク負荷軽減）
		taskchg(KSGTSKNO);
		WACDOG;
// MH810100(E) K.Onodera 2020/04/07 車番チケットレス（Kasagoタスク負荷軽減）
// GG129000(S) R.Endo 2022/12/26 車番チケットレス4.0 #6766 プログラムダウンロード処理リトライ後の遠隔メンテナンス要求でE0011(メッセージキューフル)発生 [共通改善項目 No1538]
// 		if( TASK_START_FOR_DL() ){
		// 遠隔ダウンロード処理中にタスク起動状態を変更され中断されたままとなるケースが発生した為、
		// ここでの判定をやめ、remotedl_taskのtaskchgまでは処理を終わらせるようにする。
// GG129000(E) R.Endo 2022/12/26 車番チケットレス4.0 #6766 プログラムダウンロード処理リトライ後の遠隔メンテナンス要求でE0011(メッセージキューフル)発生 [共通改善項目 No1538]
			taskchg( REMOTEDLTSKNO );								// Change to Remote Download task
			WACDOG;
// GG129000(S) R.Endo 2022/12/26 車番チケットレス4.0 #6766 プログラムダウンロード処理リトライ後の遠隔メンテナンス要求でE0011(メッセージキューフル)発生 [共通改善項目 No1538]
// 		}
// GG129000(E) R.Endo 2022/12/26 車番チケットレス4.0 #6766 プログラムダウンロード処理リトライ後の遠隔メンテナンス要求でE0011(メッセージキューフル)発生 [共通改善項目 No1538]
// MH322914(S) K.Onodera 2016/10/12 AI-V対応
//		if (_is_pip()){
		if (_is_Normal_pip()){
// MH322914(E) K.Onodera 2016/10/12 AI-V対応
			taskchg( PIPTSKNO );									// Change to ParkiPro task
			WACDOG;
		}
// MH810100(S) K.Onodera 2020/04/07 車番チケットレス（Kasagoタスク負荷軽減）
		taskchg( PKTTSKNO );										// Change to PKT task
		WACDOG;
		taskchg(KSGTSKNO);
		WACDOG;
// MH810100(E) K.Onodera 2020/04/07 車番チケットレス（Kasagoタスク負荷軽減）

		if( SHTTER_CTRL ){
			_di();
			SHTTER_CTRL = 0;
			_ei();
			shtctrl();												// ｼｬｯﾀｰ制御
		}
		taskchg( IDLESUBTSKNO );									// Change to idle sub task
		
	}/*for*/
}
//[]----------------------------------------------------------------------[]
///	@brief			idle_subtask
//[]----------------------------------------------------------------------[]
///	@param[in]		none	: 
///	@return			void	: 
///	@author			A.iiizumi
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2012/03/12<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]
void	idle_subtask( void )
{
// NOTE:アイドルタスクでコールされているサービスルーチンの中でエラーの登録を行いログDBにアクセスするためこの時、
// アイドルタスクからアイドルタスクへタスクチェンジしてしまい動作不能のなるために追加する
	for( ; ; ) {
		taskchg( IDLETSKNO );										/* Change task to idletask		*/

		if( TIM500_START ){
			tim500_mon();											// 500ms monitor
		}
		if( FLAGPT.event_CtrlBitData.BIT.UNKOWN_VECT ){				// UnKown_Vectの割り込み処理発生
			ex_errlg( ERRMDL_MAIN, ERR_MAIN_UNKOWNVECT, 2, 0 );		// エラー登録
			FLAGPT.event_CtrlBitData.BIT.UNKOWN_VECT = 0;			// UnKown_Vectの割り込みフラグOFF
		}
		if( FLAGPT.event_CtrlBitData.BIT.MESSAGEFULL ){				//メッセージバッファフル発生
// GG129000(S) R.Endo 2022/12/20 車番チケットレス4.0 #6765 遠隔ダウンロード処理中にE0011(メッセージキューフル)が発生すると再起動を繰り返す [共通改善項目 No1539]
// 			err_chk( ERRMDL_MAIN, ERR_MAIN_MESSAGEFULL, 1, 0, 0 );			// ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾌﾙ
// 			FLAGPT.event_CtrlBitData.BIT.MESSAGEFULL = 0;//メッセージバッファフル検出フラグクリア
			// メッセージキューフルはerr_chk内部でシステムリセットするため先にフラグをクリアする
			FLAGPT.event_CtrlBitData.BIT.MESSAGEFULL = 0;			// メッセージバッファフル検出フラグクリア
			err_chk(ERRMDL_MAIN, ERR_MAIN_MESSAGEFULL, 1, 0, 0);	// メッセージバッファフル
// GG129000(E) R.Endo 2022/12/20 車番チケットレス4.0 #6765 遠隔ダウンロード処理中にE0011(メッセージキューフル)が発生すると再起動を繰り返す [共通改善項目 No1539]
		}
		I2C_Event_Analize();
		I2C_BusError_Analize();
		Bluetooth_unit();// bluetooth通信機能
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| I2C割り込みイベント処理					                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : I2C_Event_Analize                                       |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.namioka					                           |*/
/*| Date         : 2012-02-02                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	I2C_Event_Analize( void )
{
	t_I2C_EVENT	event;
	ulong	ist;													// 現在の割込受付状態
	ushort	msg;

	uchar	rp_err = RP_OV_PA_FR;
	uchar	jp_err = JP_OV_PA_FR;
	
	if( !I2C_Event_Info.USHORT && !RP_OV_PA_FR && !JP_OV_PA_FR ){
		return;
	}
//	event.USHORT = I2C_Event_Info.USHORT;
//	ist = _di2();
	ist = _di2();
	event.USHORT = I2C_Event_Info.USHORT;
	I2C_Event_Info.USHORT = 0;
	RP_OV_PA_FR = 0;
	JP_OV_PA_FR = 0;
	_ei2( ist );
	
	if( event.BIT.RTC_1_MIN_IRQ ){
		RTC_ClkDataReadReq( RTC_READ_QUE_OPE );
	}
// Sc16RecvInSending_IRQ5_Level_L start
//	if( event.BIT.Print1ByteSendReq ){
//		wait2us( 200L );				// 19200(54us x 8 = 432us)
//		wait2us( 100L );				// 19200(54us x 8 = 432us)
//		wait2us( 2000L );				// 19200(54us x 8 = 432us)
//		wait2us( 1500L );				// 19200(54us x 8 = 432us)
//		Sc16SendInI2C_RP();
//		Sc16SendInI2C_JP();
//	}
// Sc16RecvInSending_IRQ5_Level_L end
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
//	if( event.USHORT & 0x000e ){
	if( event.USHORT & 0x040e ){
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
		if( event.BIT.R_PRINTER_SND_CMP ){
			err_chk( ERRMDL_PRINTER, ERR_PRNT_R_PRINTCOM, 0, 0, 0 );	// ﾚｼｰﾄﾌﾟﾘﾝﾀ通信異常解除
			if( !rct_init_sts ){
				queset(OPETCBNO, PREQ_SND_COMP_RP, 0, NULL);
			}else{
				RP_I2CSndReq( I2C_NEXT_SND_REQ );
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/レシート、ジャーナル排他処理改善）
				JP_I2CSndReq( I2C_PRI_REQ );				// レシート送信で保留された送信を再開
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/レシート、ジャーナル排他処理改善）
			}
		}
		
		if( event.BIT.R_PRINTER_RCV_CMP ){
			if( event.BIT.R_PRINTER_NEAR_END ){
				msg = PREQ_JUSHIN_ERR2;
			}else{
				msg = PREQ_JUSHIN_ERR1;
			}
			MsgSndFrmPrn(	msg,
							R_PRI,
							0 );
		}
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
		if (event.BIT.R_PRINTER_RCV_INFO) {
			// プリンタ情報応答受信
			PrnCmd_InfoRes_Msg_R();
		}
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
	}
	
// MH810104 GG119201(S) 電子ジャーナル対応
//	if( event.USHORT & 0x0070 ){
	if( event.USHORT & 0x0370 ){
// MH810104 GG119201(E) 電子ジャーナル対応
		if( event.BIT.J_PRINTER_SND_CMP ){
// MH810104 GG119201(S) 電子ジャーナル対応
			err_chk(ERRMDL_EJA, ERR_EJA_COMFAIL, 0, 0, 0);	// 通信不良解除
// MH810104 GG119201(E) 電子ジャーナル対応
			err_chk( ERRMDL_PRINTER, ERR_PRNT_J_PRINTCOM, 0, 0, 0 );	// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ通信異常解除
			if( !jnl_init_sts ){
				queset(OPETCBNO, PREQ_SND_COMP_JP, 0, NULL);
			}else{
				JP_I2CSndReq( I2C_NEXT_SND_REQ );
// GG129000(S) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/レシート、ジャーナル排他処理改善）
			// 電子ジャーナルへの制御コマンド送信でレシート印字データの送信がブロックされることがあるため、
			// ジャーナル送信後レシート側残留データがあれば送信する。
			// （レシートとジャーナルはpritaskで排他管理して同時にデータは発生しない）
				RP_I2CSndReq( I2C_PRI_REQ );				// ジャーナル送信で保留された送信を再開
// GG129000(E) H.Fujinaga 2022/12/08 ゲート式車番チケットレスシステム対応（インボイス対応/レシート、ジャーナル排他処理改善）
			}
		}
		
		if( event.BIT.J_PRINTER_RCV_CMP ){
// MH810104 GG119201(S) 電子ジャーナル対応
			if (event.BIT.EJA_RCV_STS) {
				msg = PREQ_JUSHIN_INFO;
			}else
// MH810104 GG119201(E) 電子ジャーナル対応
			if( event.BIT.J_PRINTER_NEAR_END ){
				msg = PREQ_JUSHIN_ERR2;
			}else{
				msg = PREQ_JUSHIN_ERR1;
			}
// MH810104 GG119201(S) 電子ジャーナル対応
			if (!jnl_init_sts) {
				queset(OPETCBNO, msg, 0, NULL);
			}
			else {
// MH810104 GG119201(E) 電子ジャーナル対応
			MsgSndFrmPrn(	msg,
							J_PRI,
							0 );
// MH810104 GG119201(S) 電子ジャーナル対応
			}
// MH810104 GG119201(E) 電子ジャーナル対応
		}
// MH810104 GG119201(S) 電子ジャーナル対応
		if (event.BIT.J_PRINTER_RCV_INFO) {
			// プリンタ情報応答受信
			PrnCmd_InfoRes_Msg();
		}
// MH810104 GG119201(E) 電子ジャーナル対応
	}
	
	if( rp_err ){
		queset( OPETCBNO, I2C_RP_ERR_EVT, 1, &rp_err );	// メインに通知
	}
	
	if( jp_err ){
		queset( OPETCBNO, I2C_JP_ERR_EVT, 1, &jp_err );	// メインに通知
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| I2C_Busエラー処理						                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Date         : 2012-03-26                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
void	I2C_BusError_Analize( void )
{
	t_I2C_BUSERROR		Error;
	ulong				ist;

	ist = _di2();
	Error.USHORT = I2C_BusError_Info.USHORT;
	//	以下の5つの発生解除同時のBitを <--- 0する
	I2C_BusError_Info.BIT.I2C_PREV_TOUT = 0;		// Bit 4 = 前回Status変化待ちTimeout	発生解除同時
	I2C_BusError_Info.BIT.I2C_BUS_BUSY = 0;			// Bit 3 = バスBUSY						発生解除同時
	I2C_BusError_Info.BIT.I2C_SEND_DATA_LEVEL = 0;	// Bit 2 = 送信データレベルエラー		発生解除同時
	I2C_BusError_Info.BIT.I2C_CLOCK_LEVEL = 0;		// Bit 1 = クロックレベルエラー			発生解除同時 
	I2C_BusError_Info.BIT.I2C_R_W_MODE = 0;			// Bit 0 = Read/Writeモードエラー		発生解除同時
	_ei2( ist );

	if (Error.USHORT & 0x0001) {
		err_chk(ERRMDL_I2C, ERR_I2C_R_W_MODE, 2, 0, 0);			// Read/Writeモードエラー		発生解除同時
	}
	if (Error.USHORT & 0x0002) {
		err_chk(ERRMDL_I2C, ERR_I2C_CLOCK_LEVEL, 2, 0, 0);		// クロックレベルエラー			発生解除同時
	}
	if (Error.USHORT & 0x0004) {
		err_chk(ERRMDL_I2C, ERR_I2C_SEND_DATA_LEVEL, 2, 0, 0);	// 送信データレベルエラー		発生解除同時
	}
	if (Error.USHORT & 0x0008) {
		err_chk(ERRMDL_I2C, ERR_I2C_BUS_BUSY, 2, 0, 0);			// バスBUSY						発生解除同時
	}
	if (Error.USHORT & 0x0010) {
		err_chk(ERRMDL_I2C, ERR_I2C_PREV_TOUT, 2, 0, 0);		// 前回Status変化待ちTimeout	発生解除同時
	}

	//	以下の4つは発生/解除別なのでBitを <--- 0しないで<- 1している箇所でErrorが発生しない時に<- 0する
	//	また0 -> 1, 1 -> 0の変化があった時のみerr_chk()をcallする
	if(I2cErrEvent.SendRP == 1) {
		I2cErrEvent.SendRP = 0;
		err_chk(ERRMDL_I2C, ERR_I2C_BFULL_SEND_RP, Error.BIT.I2C_BFULL_SEND_RP, 0, 0);	// レシートプリンタ送信バッファフル(『I2Cﾚｼｰﾄ送信ﾊﾞｯﾌｧﾌﾙ』)		発生/解除別
	}
	if(I2cErrEvent.RecvRP == 1) {
		I2cErrEvent.RecvRP = 0;
		err_chk(ERRMDL_I2C, ERR_I2C_BFULL_RECV_RP, Error.BIT.I2C_BFULL_RECV_RP, 0, 0);	// レシートプリンタ受信バッファフル(『I2Cﾚｼｰﾄ受信ﾊﾞｯﾌｧﾌﾙ』)		発生/解除別
	}
	if(I2cErrEvent.SendJP == 1) {
		I2cErrEvent.SendJP = 0;
		err_chk(ERRMDL_I2C, ERR_I2C_BFULL_SEND_JP, Error.BIT.I2C_BFULL_SEND_JP, 0, 0);	// ジャーナルプリンタ送信バッファフル(『I2Cｼﾞｬｰﾅﾙ送信ﾊﾞｯﾌｧﾌﾙ』)	発生/解除別
	}
	if(I2cErrEvent.RecvJP == 1) {
		I2cErrEvent.RecvJP = 0;
		err_chk(ERRMDL_I2C, ERR_I2C_BFULL_RECV_JP, Error.BIT.I2C_BFULL_RECV_JP, 0, 0);	// ジャーナルプリンタ受信バッファフル(『I2Cｼﾞｬｰﾅﾙ受信ﾊﾞｯﾌｧﾌﾙ』)	発生/解除別
	}
}
/*[]----------------------------------------------------------------------[]*/
/*| RTC分歩進割り込み発生/時計データ取得処理                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : RTC_ClkDataReadReq                                      |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : T.namioka					                           |*/
/*| Date         : 2012-02-02                                              |*/
/*[]------------------------------------- Copyright(C) 2012 AMANO Corp.---[]*/
ushort	RTC_ClkDataReadReq( short que )
{
	ushort					ret;
	I2C_REQUEST 			request;
	unsigned char	BcdClock[11];
	extern	void ChgTimedata_RTCBCD( struct	RTC_rec *Time, unsigned char *BCDTime);
	union {
		unsigned long	ul;
		unsigned char	uc[4];
	} u_LifeTime;

	request.TaskNoTo	 			= I2C_TASK_OPE;					// opeタスクへI2CREQUEST処理完了を通知
	request.DeviceNo 				= I2C_DN_RTC;					// RTCへの要求
	request.RequestCode    			= I2C_RC_RTC_TIME_RCV;			// 
	request.I2cReqDataInfo.RWCnt	= RTC_RW_SIZE;					// 
	request.I2cReqDataInfo.pRWData	= &RTC_CLOCK.freq100sec;		// 

	ret = (ushort)I2C_Request( &request, EXE_MODE_FORCE );			// 
	if( ret == RIIC_RSLT_OK ){
		ChgTimedata_RTCBCD(&RTC_CLOCK, BcdClock);
		// msのLifeTimeをｾｯﾄ
		u_LifeTime.ul = LifeTim2msGet();
		BcdClock[7] = u_LifeTime.uc[0];
		BcdClock[8] = u_LifeTime.uc[1];
		BcdClock[9] = u_LifeTime.uc[2];
		BcdClock[10] = u_LifeTime.uc[3];
		//
		if( que == RTC_READ_QUE_OPE ){
			queset( OPETCBNO, CLOCK_CHG, 11, BcdClock );
		}
		op_RtcFlagRegisterRead();									//flagレジスタ取得要求
	}
	return	ret;
}

/*[]----------------------------------------------------------------------[]*/
/*| 500ms監視処理                                                          |*/
/*| ｻｰﾋﾞｽ/ﾗｸﾞﾀｲﾑ監視、ｴﾗｰ/ｱﾗｰﾑ監視、ｻﾌﾞCPU歩進監視                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : tim500_mon                                              |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Hashimoto, modified by Hara                             |*/
/*| Date         : 2005-02-01                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	tim500_mon( void )
{
	uchar	i;
	uchar	saveTIM500_START;
	static uchar on_off = 0;

	_di();
	saveTIM500_START = TIM500_START;
	TIM500_START = 0;
	_ei();

	// プログラムまたは音声データがある時は通常動作で500ms間隔で点滅する
	on_off ^= 1;
	ExIOSignalwt(EXPORT_M_LD1, on_off );// ST1 LEDを500ms間隔で点滅(プログラム有を示す)
	if(f_wave_datchk != WAVE_AREA_NODATA) {// 音声データが存在する場合
		ExIOSignalwt(EXPORT_M_LD2, on_off );// ST2 LEDを500ms間隔で点滅
	} else {// 音声データが無い時は消灯
		ExIOSignalwt(EXPORT_M_LD2, 0 );// ST2 LEDを消灯
	}
	for( i=0; i < saveTIM500_START; ++i ){							// 前回から経過した時間分

		/* 500ms 関数Call型 Lagtim timeout process */
		LagChk500ms();

		/* ｻｰﾋﾞｽﾀｲﾑ、ﾗｸﾞﾀｲﾑ監視 */
		SrvTimMng();

		/* ｻﾌﾞCPU監視処理 */
		SUBCPU_MONIT++; 											// ｻﾌﾞCPU監視ｶｳﾝﾀ更新

		if( PassIdBackupTim > 0 ){									// n分ﾁｪｯｸ用ﾀｲﾏｰ起動
			PassIdBackupTim--;
		}

		if( RECI_SW_Tim > 0 ){										// 領収証発行可能ﾀｲﾏｰ
			RECI_SW_Tim--;
			if( RECI_SW_Tim == 0 ){
				queset( OPETCBNO, MID_RECI_SW_TIMOUT, 0, NULL );	// 領収証発行可能ﾀｲﾏｰﾀｲﾑｱｳﾄ通知
			}
		}

		if( SUBCPU_MONIT > 360 ){									// 3分経過?
			WACDOG;// RX630はWDGストップが出来ない仕様のためWDGリセットにする
			_di();
			err_chk( ERRMDL_MAIN, ERR_MAIN_SUBCPU, 1, 0, 0 );		// 分歩進なし
			SUBCPU_MONIT = 0;
			if( timerd( &CLK_REC ) != 0 ){
				ex_errlg( ERRMDL_MAIN, ERR_MAIN_CLOCKREADFAIL, 2, 0 );		// 時計読み込み失敗
			}
			_ei();
			dog_init();
		}
	}

	// 5secおきに出力信号等のﾁｪｯｸを行う	*/
	SIGCHK_TRG++;
	if( SIGCHK_TRG > 10 ){

		SIGCHK_TRG = 0;
		/* ﾒﾓﾘ監視 */
		memorychk();
		/* 信号出力ﾁｪｯｸ */
		sig_chk();

		LkErrTimMng();
	}
// MH322917(S) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
	// 3secおきに長期駐車のチェック(ドア閉アラーム送信用)を行う
	LONG_PARK_COUNT2++;
	if( LONG_PARK_COUNT2 > 6 ){
		LONG_PARK_COUNT2 = 0;
		LongTermParkingCheck_Resend();		// 長期駐車チェック(ドア閉時にセンターに対して送信する処理)
	}
	// 10secおきに長期駐車のチェック(アラームデータ)を行う
	LONG_PARK_COUNT++;
	if( LONG_PARK_COUNT > 20 ){
		LONG_PARK_COUNT = 0;
		if(f_LongParkCheck_resend == 0){
			// ドア閉アラーム送信処理が行われていないときにチェックする
			LongTermParkingCheck();			// 長期駐車検出(アラームデータによる通知)
		}
	}
	
	// 10secおきに長期駐車のチェック(長期駐車ログ)を行う
	LONG_PARK_COUNT3++;
	if( LONG_PARK_COUNT3 > 20 ){
		LONG_PARK_COUNT3 = 0;
		LongTermParkingCheck_r10_prmcng();		// 設定変更による強制解除処理
		if(LongPark_Prmcng.f_prm_cng == 0){
			// 設定変更による強制解除処理が行われていないときにチェックする
			LongTermParkingCheck_r10();			// 長期駐車検出(長期駐車データによる通知)
		}
	}
// MH322917(E) A.Iiizumi 2018/09/03 長期駐車検出機能の拡張対応(検出部分)
// MH810102(S) 電子マネー対応 #5329 精算機の電源をOFF/ONした時、E2615-265が発生する
	WAKEUP_COUNT++;								// 起動タイマカウンタ
	if( WAKEUP_COUNT > 120){					// 60Sec
		WAKEUP_COUNT = 0;
		WAKEUP_OVER = 1;						// 起動済みフラグ
	}
// MH810102(E) 電子マネー対応 #5329 精算機の電源をOFF/ONした時、E2615-265が発生する

// MH810100(S) S.Takahashi 2020/03/18 自動連続精算
//// 自動連続精算
//#if (1 == AUTO_PAYMENT_PROGRAM)
//	// 1secおきに自動連続精算処理を行う
//	if (OPECTL.Seisan_Chk_mod == ON) {
//		AutoPayment.AUTOPAY_TRG++;
//		if (AutoPayment.AUTOPAY_TRG > 1) {
//			AutoPaymentIn(&(AutoPayment.In));		// 入庫処理
//			AutoPaymentOut(&(AutoPayment.Out));		// 精算処理
//			AutoPayment.AUTOPAY_TRG = 0;
//		}
//	}
//#endif
// MH810100(E) S.Takahashi 2020/03/18 自動連続精算

}


/*[]----------------------------------------------------------------------[]*/
/*| tcb初期化                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Tcb_Init                                                |*/
/*| PARAMETER    :                                                         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Sunrich,modified by hashimoto                           |*/
/*| Date         : 2001-10-31                                              |*/
/*| Update       : 2002-07-30 M.Okuda                                      |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void	Tcb_Init( void )
{
	short	i;

	for(i=0; i<TCB_CNT; ++i){
		tcb[i].event = MSG_EMPTY;
		tcb[i].level = 0;
		tcb[i].status = 0;
		tcb[i].msg_top = NULL;
		tcb[i].msg_end = NULL;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| mcb初期化                                                              |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Mcb_Init                                                |*/
/*| PARAMETER    :                                                         |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Sunrich,modified by hashimoto                           |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void	Mcb_Init( void )
{
	short	i;

	for( i=0; i < (MSGBUF_CNT-1); i++ ){
		msgbuf[i].msg_next = (char *)&msgbuf[i+1];
	}
	msgbuf[i].msg_next = (char *)NULL;

	mcb.msg_top = &msgbuf[0];
	mcb.msg_end = &msgbuf[MSGBUF_CNT-1];
}

/*[]----------------------------------------------------------------------[]*/
/*| 送信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ取得                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetBuf( void )                                          |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀ,未取得はNULL                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Sunrich,modified by hashimoto                           |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
MsgBuf	*GetBuf( void )
{
	MsgBuf	*msg_add;
	ulong	ist;													// 現在の割込受付状態
// GG124100(S) R.Endo 2022/08/29 車番チケットレス3.0 #6560 再起動発生（直前のエラー：メッセージキューフル E0011） [共通改善項目 No1528]
#if DEBUG_MESSAGEFULL
	static ulong	prev_buf_count = 0;
	ulong	buf_count = 0;
	ulong	msg_count = 0;
	ulong	max_msg_count = 0;
	ulong	data = 0;
	uchar	id = 0;
	uchar	max_id = 0;

	ist = _di2();

	// 取得可能なバッファをカウント
	msg_add = mcb.msg_top;
	while ( msg_add ) {
		msg_add = (MsgBuf*)msg_add->msg_next;
		buf_count++;
	}

	// 使用量の多いタスクIDを確認
	for ( id = 0; id < TCB_CNT; id++ ) {
		msg_add = tcb[id].msg_top;
		while ( msg_add ) {
			msg_add = (MsgBuf*)msg_add->msg_next;
			msg_count++;
		}
		if ( msg_count > max_msg_count ) {
			max_msg_count = msg_count;
			max_id = id;
		}
	}

	_ei2( ist );

	// アラームに設定する情報を作成(4桁中下位3桁：取得可能なバッファの変化量)
	if ( prev_buf_count >= buf_count ) {
		data = prev_buf_count - buf_count;
	} else {
		data = buf_count - prev_buf_count;
	}
	prev_buf_count = buf_count;

	// アラームに設定する情報を作成(4桁中上位1桁：使用量の多いタスクID)
	data += (max_id * 1000);

	// 取得可能なバッファが100未満であればアラームを発生
	if ( buf_count < 100 ) {
		alm_chk2(DEBUG_ALARM_MD, DEBUG_ALARM_NO,
			1, 2, 0, (void *)&data);	// 発生
	}

	// 取得可能なバッファが150以上であればアラームを解除
	if ( buf_count >= 150 ) {
		alm_chk2(DEBUG_ALARM_MD, DEBUG_ALARM_NO,
			0, 2, 0, (void *)&data);	// 解除
	}
#endif
// GG124100(E) R.Endo 2022/08/29 車番チケットレス3.0 #6560 再起動発生（直前のエラー：メッセージキューフル E0011） [共通改善項目 No1528]

	if( mcb.msg_top != NULL ){
		ist = _di2();
		msg_add = mcb.msg_top;
		mcb.msg_top = (MsgBuf*)mcb.msg_top->msg_next;
		_ei2( ist );
		return( msg_add );
	}else{
		FLAGPT.event_CtrlBitData.BIT.MESSAGEFULL = 1;//メッセージバッファフル検出
		return( NULL );
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾒｯｾｰｼﾞ送信                                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : PutMsg( id, msg )                                       |*/
/*| PARAMETER    : id = 送信ﾀｽｸﾅﾝﾊﾞｰ                                       |*/
/*|                msg = 送信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀ                                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Sunrich,modified by hashimoto                           |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void	PutMsg( unsigned char id, MsgBuf *msg )
{
	ulong	ist;													// 現在の割込受付状態

	if( id >= TCB_CNT ){
		FreeBuf( msg );
		return;
	}
	ist = _di2();
	tcb[id].event = MSG_SET;
	if( tcb[id].msg_end != NULL ){
		tcb[id].msg_end->msg_next = (char*)msg;
	}
	msg->msg_next = NULL;
	tcb[id].msg_end = msg;
	if( tcb[id].msg_top == NULL ){
		tcb[id].msg_top = msg;
	}
	_ei2( ist );
}

/*[]----------------------------------------------------------------------[]*/
/*| 自ﾒｯｾｰｼﾞ取得                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetMsg( id )                                            |*/
/*| PARAMETER    : id = 取得ﾀｽｸﾅﾝﾊﾞｰ                                       |*/
/*| RETURN VALUE : 取得ﾒｯｾｰｼﾞﾎﾟｲﾝﾀ,取得ﾒｯｾｰｼﾞがなければNULL                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Sunrich,modified by hashimoto                           |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
MsgBuf	*GetMsg( uchar id )
{
	MsgBuf	*msg_add;
	ulong	ist;													// 現在の割込受付状態

	if( tcb[id].msg_top != NULL ) {
		ist = _di2();
		msg_add = tcb[id].msg_top;
		tcb[id].msg_top = (MsgBuf *)tcb[id].msg_top->msg_next;
		if( tcb[id].msg_top == NULL ) {
			tcb[id].msg_end = NULL;
			tcb[id].event = MSG_EMPTY;
		}
		_ei2( ist );
		return( msg_add );
	}else{
		return( NULL );
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾒｯｾｰｼﾞﾊﾞｯﾌｧ解放                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : FreeBuf( msg )                                          |*/
/*| PARAMETER    : msg = 解放するﾒｯｾｰｼﾞﾊﾞｯﾌｧのﾎﾟｲﾝﾀ                        |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Sunrich,modified by hashimoto                           |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void
FreeBuf( msg )
MsgBuf	*msg;
{
	ulong	ist;													// 現在の割込受付状態

	ist = _di2();
	mcb.msg_end->msg_next = (char*)msg;
	msg->msg_next = NULL;
	mcb.msg_end = msg;
	if( mcb.msg_top == NULL ){
		mcb.msg_top = msg;
	}
	_ei2( ist );
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾒｯｾｰｼﾞを受信するまでWAITさせる                                         |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : GetMsgWait                                              |*/
/*| PARAMETER    : id  = 受信待ちするﾒｯｾｰｼﾞ№(TCB)                         |*/
/*|                msg = 受信したﾒｯｾｰｼﾞ                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Sunrich,modified by hashimoto                           |*/
/*| Date         : 2001-10-31                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
void GetMsgWait( uchar id, MsgBuf *msg )
{
	MsgBuf	*rcv;

	for( ; ; )
	{
		taskchg( IDLETSKNO );										// Change task to idletask
		if( (rcv = GetMsg( id )) != NULL )
		{
			memcpy( msg , rcv , sizeof(MsgBuf) );
			FreeBuf( rcv );
			break;
		}
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| ﾒｰﾙｷｭｰから狙ったﾒｰﾙを取得する                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| PARAMETER    : id = 取得ﾀｽｸﾅﾝﾊﾞｰ                                       |*/
/*|				   pReq = 検索対象ﾒｯｾｰｼﾞID（最大 TARGET_MSGGET_PRM_MAX 個）|*/
/*| RETURN VALUE : 取得ﾒｯｾｰｼﾞﾎﾟｲﾝﾀ,取得ﾒｯｾｰｼﾞがなければNULL                |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Okuda												   |*/
/*| Date         : 2006/06/22                                              |*/
/*[]------------------------------------- Copyright(C) 2001 AMANO Corp.---[]*/
MsgBuf	*Target_MsgGet( uchar id, t_TARGET_MSGGET_PRM *pReq )
{
	MsgBuf	*msg_add;										// ﾒｰﾙｷｭｰ内ﾁｪｯｸ対象ﾒｰﾙｱﾄﾞﾚｽｾｯﾄｴﾘｱ
	ushort	MailCommand;									// ﾒｰﾙｷｭｰ内ﾁｪｯｸ対象ﾒｰﾙのﾒｯｾｰｼﾞID
	ushort	ReqCount;										// 要求された検索対象ﾒｯｾｰｼﾞID数
	ulong	ist;											// 現在の割込受付状態
	uchar	i,j;
	MsgBuf	*msg_add_Prev;									// ﾒｰﾙｷｭｰ内(1回前)ﾁｪｯｸ対象ﾒｰﾙｱﾄﾞﾚｽｾｯﾄｴﾘｱ

	if( tcb[id].msg_top == NULL ) {							// ﾒｰﾙなし
		return( NULL );
	}

	ReqCount = pReq->Count;									// 要求された検索対象ﾒｯｾｰｼﾞID数get

	ist = _di2();											// 割込み禁止
	msg_add = tcb[id].msg_top;								// ﾁｪｯｸ対象ﾒｰﾙｱﾄﾞﾚｽget（最初は先頭）

	// 検索処理
	for( i=0; i<MSGBUF_CNT; ++i ){							// ﾒｰﾙｷｭｰ内 全ﾒｰﾙﾁｪｯｸ（MSGBUF_CNTは単にLimitter）

		MailCommand = msg_add->msg.command;					// ﾁｪｯｸ対象ﾒｰﾙのﾒｯｾｰｼﾞIDget

		for( j=0; j<ReqCount; ++j ){						// 探して欲しいﾒｰﾙID数分
			if( pReq->Command[j] == MailCommand ){			// 発見
				goto Target_MsgGet_10;						// 見つけたﾒｰﾙをｷｭｰから抜く
			}
		}

		// 次のﾒｰﾙへ（ﾁｪｯｸ準備）
		msg_add_Prev = msg_add;								// 前ﾒｰﾙｱﾄﾞﾚｽｾｰﾌﾞ（抜く時に必要）
		msg_add = (MsgBuf*)(msg_add->msg_next);				// 次ﾁｪｯｸ対象ﾒｰﾙｱﾄﾞﾚｽget
		if( NULL == msg_add ){								// 次ﾒｰﾙなし
			break;											// 検索終了
		}

		// 16回に1回 WDTｸﾘｱ
		if( 0x0f == (i & 0x0f) ){
			WACDOG;
		}
	}

	// 発見できなかった場合（残りﾒｰﾙなし）
	_ei2( ist );
	return( NULL );

	// 発見した場合
Target_MsgGet_10:
	// Target messageをｷｭｰから抜く

	if( 0 == i ){											// 先頭ﾒｰﾙの場合
		tcb[id].msg_top = (MsgBuf *)tcb[id].msg_top->msg_next;
		if( tcb[id].msg_top == NULL ) {						// ﾒｰﾙが1件しかなかった場合
			tcb[id].msg_end = NULL;
			tcb[id].event = MSG_EMPTY;
		}
	}
	else if( tcb[id].msg_end == msg_add ){					// 末尾ﾒｰﾙの場合（2件以上ﾒｰﾙがある）
		msg_add_Prev->msg_next = NULL;						// 最終ﾒｰﾙﾏｰｸset
		tcb[id].msg_end = msg_add_Prev;
	}
	else{													// 途中ﾒｰﾙの場合（2件以上ﾒｰﾙがある）
		msg_add_Prev->msg_next = msg_add->msg_next;			// 最終ﾒｰﾙﾏｰｸset
	}

	_ei2( ist );
	return( msg_add );
}

// MH321800(S) Y.Tanizaki ICクレジット対応(共通改善No.1362/精算中止と同時に電子マネータッチで、カード引去りするが精算中止してしまう不具合対策)
/**
 *	メールキューから狙ったメールを１つだけ削除する
 *
 *	@param[in]	task_id		タスクＩＤ
 *	@param[in]	message_id	メッセージＩＤ
 *	@retval	NULL		指定したメールは存在しなかった
 *	@retval	NULL以外	削除したメールデータへのポインタ
 *	@author	Hamada Yoshiaki < Yoshiaki_Hamada@amano.co.jp >
 *	@see	Target_MsgGet()
 *	@date	2009/11/12(木)
 *	@attention	返値でNULL以外が返った場合でも、そのポインタが指す領域はメールを削除済みなので、
 *				返値はメールを削除したかどうかの判定にのみ使用すること。
 */
const MsgBuf *Target_MsgGet_delete1( const unsigned char task_id, const unsigned short message_id )
{
	t_TARGET_MSGGET_PRM	Target_WaitMsgID;
	MsgBuf	*msb;

	// 受信したいメッセージＩＤを準備
	Target_WaitMsgID.Count = 1;
	Target_WaitMsgID.Command[0] = message_id;

	msb = Target_MsgGet( task_id, &Target_WaitMsgID );	// 期待するメールだけ受信（他は溜めたまま）
	if( NULL != msb ){
		FreeBuf( msb );		// 目的のメールを受信メッセージバッファから削除する
	}

	return msb;
}
// MH321800(E) Y.Tanizaki ICクレジット対応(共通改善No.1362/精算中止と同時に電子マネータッチで、カード引去りするが精算中止してしまう不具合対策)

/*[]----------------------------------------------------------------------[]*/
/*| 指定時間ﾀｽｸをWAITさせる                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : xPause                                                  |*/
/*| PARAMETER    : WaitTime = 待ち時間 (10ms unit value)                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : hara                                                    |*/
/*| Date         : 2005-01-24                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	xPause( ulong WaitTime )
{
	ulong	StartTime;

	xPauseStarting = 1;
	StartTime = LifeTimGet();
	for( ;; ){
		taskchg( IDLETSKNO );										// ﾀｽｸ切替
		if( WaitTime == 0 ){	// 待ち時間が0msで指定された場合には、idletaskを１周したら処理を戻す。
			break;
		}
		
		if( WaitTime <= LifePastTimGet( StartTime ) ){
			break;
		}
	}
	xPauseStarting = 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| 指定時間ﾀｽｸをWAITさせる（プリンタタスク専用）                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : xPause_PRNTSK                                           |*/
/*| PARAMETER    : WaitTime = 待ち時間 (10ms unit value)                   |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : namioka                                                 |*/
/*| Date         : 2007-05-31                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	xPause_PRNTSK( ulong WaitTime )
{
	ulong	StartTime;

	// xPauseでｵﾍﾟﾀｽｸを起動させる
	xPauseStarting_PRNTSK = 1;
	StartTime = LifeTimGet();
	for( ;; ){
		taskchg( IDLETSKNO );										// ﾀｽｸ切替
		if( WaitTime == 0 ){	// 待ち時間が0msで指定された場合には、idletaskを１周したら処理を戻す。
			break;
		}
		if( WaitTime <= LifePastTimGet( StartTime ) ){
			break;
		}
	}
	xPauseStarting_PRNTSK = 0;
}

/*[]----------------------------------------------------------------------[]*/
/*| 指定時間ﾀｽｸをWAITさせる                                                |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : xPause                                                  |*/
/*| PARAMETER    : WaitTime = 待ち時間 (2ms unit value)                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : akiba                                                   |*/
/*| Date         : 2005-01-24                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void	xPause2ms( ulong WaitTime )
{
	ulong	StartTime;

	xPauseStarting = 1;
	StartTime = LifeTim2msGet();
	do{
		taskchg( IDLETSKNO );										// ﾀｽｸ切替
	}while( 0 == LifePastTim2msGet(StartTime, WaitTime) );
	xPauseStarting = 0;
}

// 自動連続精算
#if (1 == AUTO_PAYMENT_PROGRAM)
/*[]----------------------------------------------------------------------[]*/
/*| 5secおきに自動連続入庫処理を行う                                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : AutoPaymentIn                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Nishizato                                               |*/
/*| Date         : 2006-11-13                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void AutoPaymentIn(t_AutoPayment_In *In)
{
	char i = 0;
	ushort offset = INT_CAR_START_INDEX;

	// 5secに1回入庫
	if (++(In->count) < 5) {
		return;
	}
	In->count = 0;

	// 空き車室のﾁｪｯｸ
	while (i++ < MAX_AUTO_PAYMENT_NO) {	// 見つかるまで全車室をｻｰﾁ
		if (In->index >= (ushort)MAX_AUTO_PAYMENT_NO) {
			In->index = 0;
			offset = (ushort)INT_CAR_START_INDEX;
		}
		if (In->index > (ushort)MAX_AUTO_PAYMENT_CAR) {
			offset = (ushort)(BIKE_START_INDEX - INT_CAR_LOCK_MAX);	// 車室ﾊﾟﾗﾒｰﾀ 駐輪指定用ｵﾌｾｯﾄ
		}
		if (FLAPDT.flp_data[In->index + offset].nstat.bits.b00 == 0) {
			// 車両のいない車室を発見 → ﾌﾗｯﾌﾟ/ﾛｯｸ装置ﾀﾞﾐｰから入庫を通知
			if(LKcom_Search_Ifno(LockInfo[In->index + offset].if_oya) == 0) {	// ロックの場合
				LK_AutoPayment_Rcv(In->index, LockInfo[In->index + offset].if_oya, 
									LockInfo[In->index + offset].lok_no, AUTOPAY_STS_IN);
			} else {															// フラップの場合 LockInfo[i].lok_no を「1」と読み替える
				LK_AutoPayment_Rcv(In->index, LockInfo[In->index + offset].if_oya, 
									1, AUTOPAY_STS_IN);
			}			
			(In->index)++;
			break;
		}
		(In->index)++;
	}
	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| 自動連続精算処理を行う                                                 |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : AutoPaymentOut                                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : Takahashi                                               |*/
/*| Date         : 2006-11-13                                              |*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]*/
void AutoPaymentOut(t_AutoPayment_Out *Out)
{
	ushort i, max;
	ushort no;
	ushort cnt;
	char   onof;
	
	switch (Out->mode) {
	case 0:
		// メンテナンス中でなく、待機状態である場合
		if ((OPECTL.Mnt_mod == 0) && (OPECTL.Ope_mod == 0) && (AppServ_IsLogFlashWriting(eLOG_PAYMENT) == 0) ) {
			cnt = 0;
			no  = (ushort)-1;
			max = (ushort)TOTAL_CAR_LOCK_MAX;
			for(i = 0; i < max; ++i) {
				if(FLAPDT.flp_data[i].mode == FLAP_CTRL_MODE4) {
					++cnt;
				}
			}
			if(cnt < 5) {
				return;
			}
			i   = (ushort)Out->next_no;

			while (i < max) {
				if (FLAPDT.flp_data[i].mode == FLAP_CTRL_MODE4) {
					if (no == (ushort)-1) {
						no = i + (ushort)1;
					}
					if(cnt >= 5) {
						break;
					}
				}
				if (++i == max) {
					i   = 0;
					max = Out->next_no;
				}
			}

			// 駐車台数が５台以上？
			if (cnt >= 5) {
				if(no <= TOTAL_CAR_LOCK_MAX) {
					no -= INT_CAR_START_INDEX;
				}
				else if (no > BIKE_START_INDEX && no <= (BIKE_START_INDEX + BIKE_LOCK_MAX)) {
					no -= BIKE_START_INDEX;
				}
				else {
					break;
				}
				Out->proc_no = no - 1;
				onof = 1;
				
				// 駐車位置番号のテンキーイベントを通知
				queset( OPETCBNO, (unsigned short)((no/100)+KEY_TEN0), 1, &onof );	// １桁目
				no %= 100;
				queset( OPETCBNO, (unsigned short)((no/10)+KEY_TEN0), 1, &onof ); 	// ２桁目
				queset( OPETCBNO, (unsigned short)((no%10)+KEY_TEN0), 1, &onof );	// ３桁目
				
				Out->mode = 1;
			}
		}
		break;
	case 1:
		// 駐車位置番号入力
		if (OPECTL.Ope_mod == 1) {
			// 偽精算キーイベント通知
			onof = 1;
			queset( OPETCBNO, KEY_TEN_F1, 1, &onof );
			Out->mode = 2;
		}
		break;
	case 2:
		// 入金中
		if (OPECTL.Ope_mod == 2) {
			ryo_buf.in_coin[2] = 1;		// 百円硬貨が１枚入っている事にする
			
			// 偽コインメックイベント通知
			queset( OPETCBNO, COIN_IN_EVT, 0, NULL );	// 入金あり
			queset( OPETCBNO, COIN_EN_EVT, 0, NULL );	// 払出し可状態
			
			Out->mode = 0;
			Out->next_no = Out->proc_no + 1;
		}
		else {
			Out->mode = 0;
		}
		break;
	default:
		break;
	}
}

#endif

#if (1 == AUTO_PAYMENT_PROGRAM)
	static void	Debug_ErrAlm_OnOff( void );
	static void	Debug_LogFull( void );

static	void	AutoPayment_sub()
{
	Debug_ErrAlm_OnOff();
	if( CPrmSS[S_SYS][5] == 2 )
		Debug_LogFull();
}

//------------------------------------------------------------------------------
//	共通№01-0006を変更することにより、ｴﾗｰ,ｱﾗｰﾑの発生解除を行えるようにする。
//
//	01-0006=①②③④⑤⑥
//
//	①：0=ｴﾗｰ，1=ｱﾗｰﾑ，2=ﾓﾆﾀ，3=操作ﾓﾆﾀ
//	②③④⑤：ｺｰﾄﾞ（NT-NETレベルのｺｰﾄﾞ）
//	⑥：0=解除，1=発生，2=発生解除
//
//	例）E0120発生 → 001201
//		A0102解除 → 101020
//		E0070発生&解除 → 000702
//
//	１回実行後はプログラムで０クリアする。
//------------------------------------------------------------------------------
static void	Debug_ErrAlm_OnOff( void )
{
	extern	ulong	FLT_WriteParam1(uchar flags);

	char	md, no, knd;
	char	f_Change=0;
	ushort	MonCode;

	if( CPrmSS[S_SYS][6] == 0 ){					// 指示無し
		return;
	}

	md = (char)prm_get( COM_PRM,S_SYS,6,2,4 );			// 種別
	no = (char)prm_get( COM_PRM,S_SYS,6,2,2 );			// 番号
	knd = (char)prm_get( COM_PRM,S_SYS,6,1,1 );			// ｽﾃｰﾀｽ
	MonCode = (ushort)prm_get( COM_PRM,S_SYS,6,4,2 );

	switch( (ushort)prm_get( COM_PRM,S_SYS,6,1,6 ) ){	// 種別で分離
	case	0:	// ｴﾗｰ
		err_chk( md, no, knd, 0, 0 );
		f_Change = 1;
		break;

	case	1:	// ｱﾗｰﾑ
		alm_chk( md, no, knd );
		f_Change = 1;
		break;

	case	2:	// ﾓﾆﾀ
		wmonlg( MonCode, 0, 0 );
		f_Change = 1;
		break;

	case	3:	// 操作ﾓﾆﾀ
		wopelg( MonCode,0,0 );
		f_Change = 1;
		break;

	default:
		return;
	}

	if(	f_Change ){
		CPrmSS[S_SYS][6] = 0;
		(void)FLT_WriteParam1( 0 /*FLT_NOT_EXCLUSIVE*/ );	// FlashROM update
	}
}

/*----------------------------------------------------------*/
/*	精算中止，金銭管理，ｺｲﾝ&金庫LOGにFull件ﾃﾞｰﾀを登録する	*/
/*	01-0005=2 で実施。										*/
/*----------------------------------------------------------*/
static void	Debug_LogFull( void )
{

}
#endif
