/*[]----------------------------------------------------------------------[]*/
/*| JVMA Communication Control											   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author      : G.So                                                     |*/
/*| Date        : 2019.02.07                                               |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
#include	<stdio.h>
#include	<string.h>
#include	"system.h"
#include	"Message.h"
#include	"mem_def.h"
#include	"cnm_def.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"ope_def.h"
#include	"prm_tbl.h"
#include	"suica_def.h"
#include	"ec_def.h"

//---------------------------------------------------------------------------
// Production Note
// suica_ctrl.cに手を加えられないため、同ソースを下敷きに作り直したものです。
// このため、重複する機能は一杯あります。
//---------------------------------------------------------------------------
// MH810103 GG119202(S) JVMAリセット処理不具合
//void	jvma_init( void );
//void	jvma_trb( unsigned char err_kind );
//void	jvma_event( void );
//
//void	jvma_command_set( void );
//short	jvma_act( void );
// MH810103 GG119202(E) JVMAリセット処理不具合

// local
void	jvma_nop( void );
void	jvma_event_ex( void );
short	jvma_phase_01( void );
short	jvma_phase_02( void );
short	jvma_phase_03( void );
short	jvma_phase_04( void );
uchar 	jvma_send_que_check( void );

/*[]----------------------------------------------------------------------[]*/
/*| JVMA Line Initialize                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : jvma_init                                               |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void jvma_init( void )
{
// MH810103 GG119202(S) JVMAリセット処理変更
	Ec_Jvma_Reset_Count = 0;											// リセット可回数をクリア
// MH810103 GG119202(E) JVMAリセット処理変更
	memset(&jvma_setup, 0, sizeof(jvma_setup));
	jvma_setup.init = jvma_nop;
	jvma_setup.event = jvma_nop;
	jvma_setup.recv = (short (*)(uchar*, short))jvma_nop;
	jvma_setup.log_regist = Suica_Log_regist;
	jvma_setup.type = 0;
	jvma_setup.type = (uchar)prm_get(COM_PRM, S_PAY, 24, 1, 3);		// SX-20接続設定取得
// MH810103 GG119202(S) 接続設定参照処理改善
	// 02-0024④は、接続なし:0、接続あり:1のみ有効
	if( jvma_setup.type != 1 ){
		jvma_setup.type = 0;
	}
// MH810103 GG119202(E) 接続設定参照処理改善
	if(prm_get(COM_PRM, S_PAY, 24, 1, 6) != 0) {
		if(jvma_setup.type == 0) {
			jvma_setup.type = 4;	// 決済リーダ有効
		}
	}
	switch(jvma_setup.type) {
	case	0:
	default:
		return;
	case	1:
// MH810103 GG119202(S) 接続設定参照処理改善
//	ボタン付きないため、削除
//	case	2:		// 支払ボタン付き
// MH810103 GG119202(E) 接続設定参照処理改善
		jvma_setup.mdl = ERRMDL_SUICA;
		jvma_setup.init = suica_init;
		jvma_setup.event = suica_event;
		jvma_setup.recv = suica_recv;
		break;
	case	4:
		jvma_setup.event = jvma_event_ex;
		if (jvma_init_tbl[jvma_setup.type] == NULL) {
			return;
		}
		jvma_setup.init = (void (*)(void))jvma_init_tbl[jvma_setup.type];
// MH810103 GG119202(S) 起動初期化調整
		// 起動時の初期化をここでも行う。
		ec_flag_clear(1);							// 関連フラグ初期化
// MH810103 GG119202(E) 起動初期化調整
		break;
	}
}

/*[]----------------------------------------------------------------------[]*/
/*| Suica Trouble   	                                                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suicatrb                                                |*/
/*| PARAMETER	 : err_kind:ｴﾗｰ種別							               |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void jvma_trb( unsigned char err_kind )
{
	// 初期化予約済み？ or 未初期化？
	if (Suica_Rec.Data.BIT.RESET_RESERVED ||
		!Suica_Rec.Data.BIT.INITIALIZE) {
		return;
	}
	if( err_kind == ERR_SUICA_COMFAIL ){									/* ｴﾗｰ種別が通信不良の場合 */
		err_suica_chk(  &suica_errst ,&suica_errst_bak,ERR_SUICA_COMFAIL );	/* ｴﾗｰﾁｪｯｸ */
		Suica_Rec.suica_err_event.BIT.COMFAIL = 1;							/* 通信不良ｴﾗｰﾌﾗｸﾞON */
		suica_errst_bak = suica_errst;										/* ｴﾗｰ状態をﾊﾞｯｸｱｯﾌﾟ */
// MH810103 GG119202(S) JVMAリセット処理変更
		if (isEC_USE()) {
// MH810103 GG119202(E) JVMAリセット処理変更
		Suica_Rec.Data.BIT.CTRL_CARD = 0;									// カードなしとする
// MH810103 GG119202(S) 起動シーケンス完了条件見直し
//		Suica_Rec.Data.BIT.BRAND_STS_RCV = 0;								// Brand Status Receive
		memset(&Suica_Rec.ec_negoc_data, 0, sizeof(Suica_Rec.ec_negoc_data));
																			// ブランドネゴシエーション管理情報クリア
// MH810103 GG119202(E) 起動シーケンス完了条件見直し
// MH810103 GG119202(S) 電子マネー選択後に硬貨・紙幣を入金不可にする
		EcEnableNyukin();
// MH810103 GG119202(E) 電子マネー選択後に硬貨・紙幣を入金不可にする
// MH810103 GG119202(S) ブランド選択後のキャンセル処理変更
//		Suica_Rec.Data.BIT.SLCT_BRND_BACK = 0;								// ブランド選択の「戻る」検出フラグ初期化
// MH810103 GG119202(E) ブランド選択後のキャンセル処理変更
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
//		memset(&RecvBrandResTbl, 0, sizeof(RecvBrandResTbl));
		Suica_Rec.Data.BIT.SELECT_SND = 0;									// 選択商品データ送信中 解除
		Suica_Rec.Data.BIT.BRAND_SEL = 0;									// ブランド選択中 解除
		EcBrandClear();														// ブランド選択結果クリア
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810103 GG119202(S) 決済リーダ関連判定処理見直し
//		Ec_Settlement_Sts = EC_SETT_STS_NONE;								/* 決済状態もOFFにしておく */
		ECCTL.Ec_Settlement_Sts = EC_SETT_STS_NONE;							/* 決済状態もOFFにしておく */
// MH810103 GG119202(E) 決済リーダ関連判定処理見直し
		if(Suica_Rec.Data.BIT.SETTLMNT_STS_RCV) {							// 決済処理中に通信異常が発生？
			queset( OPETCBNO, EC_EVT_DEEMED_SETTLEMENT, 0, 0 );				// opeへ通知
			Suica_Rec.Data.BIT.SETTLMNT_STS_RCV = 0;						// 決済処理中受信フラグ初期化
		}
// MH810103 GG119202(S) JVMAリセット処理変更
//		else {
		{
// MH810103 GG119202(E) JVMAリセット処理変更
			queset( OPETCBNO, EC_EVT_COMFAIL, 0, 0 );						// 通信不良発生をopeへ通知
		}
// MH810103 GG119202(S) JVMAリセット処理変更
		}
		else {
			queset( OPETCBNO, EC_EVT_COMFAIL, 0, 0 );						// 通信不良発生をopeへ通知
		}
// MH810103 GG119202(E) JVMAリセット処理変更
	}
// MH810103 GG119202(S) JVMAリセット処理変更
	else if (err_kind == ERR_SUICA_RECEIVE) {
// MH810103 GG119202(S) 電子マネー選択後に硬貨・紙幣を入金不可にする
		Suica_Rec.Data.BIT.CTRL_CARD = 0;									// カードなしとする
		memset(&Suica_Rec.ec_negoc_data, 0, sizeof(Suica_Rec.ec_negoc_data));
																			// ブランドネゴシエーション管理情報クリア
		EcEnableNyukin();
		Suica_Rec.Data.BIT.SELECT_SND = 0;									// 選択商品データ送信中 解除
		Suica_Rec.Data.BIT.BRAND_SEL = 0;									// ブランド選択中 解除
		edy_dsp.BIT.suica_zangaku_dsp = 0;
// MH810103 GG119202(E) 電子マネー選択後に硬貨・紙幣を入金不可にする
		queset( OPETCBNO, EC_EVT_NOT_AVAILABLE_ERR, 0, 0 );					// 決済リーダ切り離しエラー発生をopeへ通知
// MH810103 GG119202(S) JVMAリセット処理不具合
//		return;
// MH810103 GG119202(E) JVMAリセット処理不具合
	}
// MH810103 GG119202(E) JVMAリセット処理変更

// MH810103 GG119202(S) JVMAリセット処理変更
//	// ここでINITIALIZEを落とすと、リセット前にリーダーイベント処理で初期化をしてしまう
//	// ここで初期化を実行すると精算中の場合に現金精算できなくなるためここでは実施しない
//	Suica_Rec.Data.BIT.RESET_RESERVED = 1;									// 初期化を予約する
//	// 初期化シーケンス予約中なので初期化済みフラグをクリアする
//	Suica_Rec.Data.BIT.INITIALIZE = 0;
//// GG119200(S) 起動シーケンス不具合修正
//	// 初期化シーケンス予約中なのでフェーズを初期化中に更新する
//	ECCTL.phase = EC_PHASE_INIT;
//// GG119200(E) 起動シーケンス不具合修正
// MH810103 GG119202(E) JVMAリセット処理変更
// MH810103 GG119202(S) JVMAリセット処理不具合
	// 通信不良、異常データ受信時はJVMAリセットが完了するまで
	// 決済リーダとの通信は行わず、コインメック等の通信を優先させる
	// ec_init()内でECCTL.phaseを更新する
	Suica_Rec.Data.BIT.INITIALIZE = 0;
	ECCTL.phase = EC_PHASE_WAIT_RESET;
// MH810103 GG119202(E) JVMAリセット処理不具合
}

/*[]----------------------------------------------------------------------[]*/
/*| JVMA Line Event                                                        |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_event                                             |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void jvma_event( void )
{
	jvma_setup.event();
}

/*[]----------------------------------------------------------------------[]*/
/*| no operation                                                           |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : jvma_nop                                                |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void	jvma_nop(void)
{
// no operation
}

/*[]----------------------------------------------------------------------[]*/
/*| Suica Command Set                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_event                                             |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void jvma_event_ex( void )
{
// MH810103 GG119202(S) JVMAリセット処理不具合
//	if( !Suica_Rec.Data.BIT.INITIALIZE ){		/* 決済リーダ初期化済？ */
//// GG119200(S) 起動シーケンス不具合修正
////// MH321800(S) 切り離しエラー発生後の待機状態でJVMAリセットを行う
//////		if( OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 3 ){	/* 精算中？、精算完了？ */
////		if( OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 3 ||	/* 精算中？、精算完了？ */
////			Suica_Rec.Data.BIT.RESET_RESERVED != 0 ){		// 初期化保留中
////// MH321800(E) 切り離しエラー発生後の待機状態でJVMAリセットを行う
//		// 初期化保留中は何も送信しない
//		// （車室番号入力～精算完了）、かつ、初期化中の場合は何も送信しない
//		if( (Suica_Rec.Data.BIT.RESET_RESERVED != 0) ||
//// GG119200(S) JVMAリセット処理変更
////			((OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 3) &&
//			((1 <= OPECTL.Ope_mod && OPECTL.Ope_mod <= 3) &&
//// GG119200(E) JVMAリセット処理変更
//			 (ECCTL.phase == EC_PHASE_INIT)) ){
//// GG119200(E) 起動シーケンス不具合修正
//			return;
//		} else {
//// GG119200(S) 起動シーケンス不具合修正
//			// フェーズが初期化中の場合はスタンバイコマンドからやり直し
//			if (ECCTL.phase == EC_PHASE_INIT) {
//// GG119200(E) 起動シーケンス不具合修正
//			// 精算中、精算完了以外は再度起動シーケンスを処理する
//			jvma_setup.init();
//			return;
//// GG119200(S) 起動シーケンス不具合修正
//			}
//// GG119200(E) 起動シーケンス不具合修正
//		}
//	}
	if (!Suica_Rec.Data.BIT.INITIALIZE) {
		switch (ECCTL.phase) {
		case EC_PHASE_INIT:
			// 決済リーダの起動シーケンス未完了、かつ、初期化中の場合は
			// スタンバイコマンドからやり直し
			jvma_setup.init();
			return;
		case EC_PHASE_WAIT_RESET:
			// JVMAリセット待ち中は何も送信しない
			return;
		default:
			break;
		}
	}
// MH810103 GG119202(E) JVMAリセット処理不具合

	jvma_command_set();							/* 実行するｺﾏﾝﾄﾞを選択する */
	switch( jvma_act() ){						/* Suicaﾒｲﾝﾛｼﾞｯｸ */
	case 1: /* OK */
		break;
	case 2: 									/* Retry Err */
		suica_errst = suica_err[0];
		break;
	case 4: /* ACK4 */
		ex_errlg( jvma_setup.mdl, ERR_EC_ACK4RECEIVE, 2, 0 ); /* ｴﾗｰﾛｸﾞ登録(ACK4受信) */
		suica_errst = suica_err[0];
		if (OPECTL.Ope_mod == 2 || OPECTL.Ope_mod == 3) {
			// 精算中、または、精算完了状態は何もしない
			// suica_errstにエラーをセットするとcnmtaskでjvma_trb()が実行されてしまうため
			// 初期化済みフラグをクリアして待機状態で初期化シーケンスを開始する
			// （リセットをするまでACK4の状態を維持する）
			Suica_Rec.Data.BIT.INITIALIZE = 0;
			break;
		}
		cnwait( 50 );							/* 1sec wait */
		jvma_setup.init();						/* Excute Initial */
		break;
	case -1: 									/* Receive Time Out */
	case -9: 									/* Send Time Out */
		if( (suica_errst & (S_COM_RCV_TIMEOUT_ERR|S_COM_SND_TIMEOUT_ERR) ) == 0 ){	/* Already Error? YJ */
			suica_errst = suica_err[0];
		}
		break;
	default:
		break;
	}

	return;
}

/*[]----------------------------------------------------------------------[]*/
/*| Suica Command Set                                                      |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_command_set                                       |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
void jvma_command_set( void )
{
	uchar	snd_kind = 0;
	memset( suica_work_buf,0,sizeof( suica_work_buf ));
	Suica_Rec.Snd_Size = Ec_Snd_Que_Read(&snd_kind, suica_work_buf);
	Suica_Rec.snd_kind = 0;
	if (Suica_Rec.Snd_Size == 0) {
		Suica_Rec.Com_kind = 2;			/* Polling実施 */
		return;
	}
	Suica_Rec.snd_kind = snd_kind;

	Suica_Rec.Snd_Buf[0] = (unsigned char)(Suica_Rec.Snd_Size-1);	/* 送信ﾃﾞｰﾀ数＋種別の合計ｻｲｽﾞ */
	Suica_Rec.Snd_Buf[1] = Suica_Rec.snd_kind;						/* 送信種別 */
	Suica_Rec.Com_kind = 3;												/* ﾃﾞｰﾀ送信要求 */
	memcpy( &Suica_Rec.Snd_Buf[2],suica_work_buf,((sizeof(Suica_Rec.Snd_Buf[2]))*(Suica_Rec.Snd_Size-2)));	/* 送信ﾊﾞｯﾌｧにﾃﾞｰﾀをｾｯﾄ */
	Suica_Rec.Snd_Buf[Suica_Rec.Snd_Size] = bcccal( (char*)Suica_Rec.Snd_Buf, Suica_Rec.Snd_Size );		/* BCCﾃﾞｰﾀ付加 */
	Suica_Rec.Snd_Size = Suica_Rec.Snd_Size+1;															/* 送信ﾃﾞｰﾀのﾄｰﾀﾙｻｲｽﾞｾｯﾄ */
}

/*[]----------------------------------------------------------------------[]*/
/*| Suica-reader Communication Main  				                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suicaact                                                |*/
/*| PARAMETER	 : None										               |*/
/*| RETURN VALUE : short		| 1:OK	<>1:Error                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
short jvma_act( void )
{
	short	rtn;

	rtn = 0;

	suica_errst = 0;								
	memset(suica_err,0,sizeof(suica_err));			/* ｴﾗｰ格納ｴﾘｱの初期化 */
	Suica_Rec.faze = 1;								/* ｺﾏﾝﾄﾞ送信ﾌｪｰｽﾞ移行 */

	for( ; rtn==0 ; ){
		switch( Suica_Rec.faze ){
		case 1:										/* ｺﾏﾝﾄﾞ送信 */
			rtn = jvma_phase_01();
			break;
		case 2:										/* ｺﾏﾝﾄﾞ送信結果取得ﾌｪｰｽﾞ */
			rtn = jvma_phase_02();
			break;
		case 3:										/* ﾃﾞｰﾀ送信ﾌｪｰｽﾞ */
			TXRXWAIT = 3;		// 3 * 20 = 60ms
			rtn = jvma_phase_03();
			TXRXWAIT = 0;
			break;
		case 4:										/* ﾃﾞｰﾀ送信結果受信ﾌｪｰｽﾞ */
			rtn = jvma_phase_04();
			break;
		default:
			break;
		}
		switch( rtn ) {
		case 0:	 								/* Continue */
			continue;
			break;
		case 4:  								/* ACK4 */
			break;
		case -9: 								/* 送信TimeOut */
			if( suica_errst & S_COM_SND_TIMEOUT_ERR ){	/* Already Timeout?  */
				suica_err[0] |= S_COM_SND_TIMEOUT_ERR;
				break;
			}
			suica_err[2]++;						/* Time Out Retry Counter +1 */
			if( suica_err[2] <= SUICA_RTRY_COUNT_31 ){	/* ﾘﾄﾗｲが31未満の場合 */
				Suica_Rec.faze = 1;						/* ﾘﾄﾗｲを実施 */
				Suica_Rec.Com_kind = 5;
				rtn = 0;
			}else{
				suica_err[0] |= S_COM_SND_TIMEOUT_ERR;	/*ｴﾗｰをｾｯﾄ */
			}
			break;
		case -1: 								/* 受信TimeOut */
			if( Suica_Rec.Status == STANDBY_SND_AFTER ){	/* ｽﾃｰﾀｽがｽﾀﾝﾊﾞｲｺﾏﾝﾄﾞ送信後 */
				Suica_Rec.Com_kind = 0;						/* 再度、ｽﾀﾝﾊﾞｲｺﾏﾝﾄﾞを送信する */
				Suica_Rec.faze = 1;
				suica_err[0] |= S_COM_RCV_TIMEOUT_ERR;		/*ｴﾗｰをｾｯﾄ */
				break;
			}
				
			if( suica_errst & S_COM_RCV_TIMEOUT_ERR ){		/* Already Timeout?  */
				suica_err[0] |= S_COM_RCV_TIMEOUT_ERR;
				break;
			}
			suica_err[1]++;						/* Time Out Retry Counter +1 */
			if( suica_err[1] <= SUICA_RTRY_COUNT_31 ){		/* ﾘﾄﾗｲが31未満の場合 */
				Suica_Rec.faze = 1;							/* ﾘﾄﾗｲを実施 */
				rtn = 0;
			}else{
				suica_err[0] |= S_COM_RCV_TIMEOUT_ERR;		/*ｴﾗｰをｾｯﾄ */
			}
			break;
		default:
			break;
		}
	}
	// リトライが発生している？
	if( (suica_err[1] != 0) || (suica_err[2] != 0) ||
		(suica_err[3] != 0) || (suica_err[4] != 0) ) {
		// エラー解除
// GG116202(S) 入力再要求、出力再要求のエラーは登録しない
//		err_chk((char)jvma_setup.mdl, ERR_EC_REINPUT, 0, 0, 0 );
//		err_chk((char)jvma_setup.mdl, ERR_EC_REOUTPUT, 0, 0, 0 );
// GG116202(E) 入力再要求、出力再要求のエラーは登録しない
		err_chk((char)jvma_setup.mdl, ERR_EC_NAKRECEIVE, 0, 0, 0 );
	}
	return( rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 1...Send Command                                             |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_01		                                           |*/
/*| PARAMETER	 : None									                   |*/
/*| RETURN VALUE : short 	| 0:Continue -9:Send Timeout                   |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
short jvma_phase_01()
{
	short	sig;
	unsigned char	*scp;

	sig = Suica_Rec.Com_kind;			/* 送信ｺﾏﾝﾄﾞをﾜｰｸｴﾘｱに設定 */
// MH810103 GG119202(S) JVMA通信制御不具合修正
//	if(isEC_USE()) {
//		scp = (unsigned char *)&ec_tbl[sig][0];				/* 送信ｺﾏﾝﾄﾞIDを設定 */
//	} else {
//		scp = (unsigned char *)&suica_tbl[sig][0];			/* 送信ｺﾏﾝﾄﾞIDを設定 */
//	}
	// Com_kindに6で再度入力要求が来たとしても、入力要求として処理する。
	// この関数はSX-20では使われない
	if (sig == 6) {
		sig = 2;
	}
	scp = (unsigned char *)&ec_tbl[sig][0];					/* 送信ｺﾏﾝﾄﾞIDを設定 */
	if (sig == 2) {
		if (suica_rcv_que.count == SUICA_QUE_MAX_COUNT) {
			return 10;		// 空き番号をNOPリターン値とする
		}
	}
// MH810103 GG119202(E) JVMA通信制御不具合修正
	SUICA_RCLR();						// 受信バッファクリア
	if( CNM_CMD( scp, sig ) ) {			/* Send COMMAND */
		return( -9 );					/* TIME OUT */
	}
	Suica_Rec.Status = (unsigned char)(sig+1); /* 現在の状態を保存 */

	switch( sig ){
	case 2:
	case 0:
	case 1:
	case 4:
	case 6:
		Suica_Rec.faze = 2;			/* ｺﾏﾝﾄﾞ送信結果取得ﾌｪｰｽﾞに移行 */
		break;
	case 3:
	case 5:
		Suica_Rec.faze = 3;			/* ﾃﾞｰﾀ送信ﾌｪｰｽﾞに移行 */
		break;
	default:
		break;
	}

// GG116202(S) 入力再要求、出力再要求のエラーは登録しない
//	// 初期化シーケンス中はエラー登録しない
//	if (Suica_Rec.Data.BIT.INITIALIZE) {
//		if( sig == 4 ){
//			err_chk((char)jvma_setup.mdl, ERR_EC_REINPUT, 1, 0, 0 );
//		}else if( sig == 5 ){
//			err_chk((char)jvma_setup.mdl, ERR_EC_REOUTPUT, 1, 0, 0 );
//		}
//	}
// GG116202(E) 入力再要求、出力再要求のエラーは登録しない
	return( 0 );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 2...Wait Recieve after Send Command                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_02                                                |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : short    | 0:Continue 1:OK ETC:Error                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
short jvma_phase_02( void )
{
	short	rtn;
	unsigned char	status;
	
	status = Suica_Rec.Status;												/* ｽﾃｰﾀｽの保持 */

	if( (rtn = CNM_RCV( Suica_Rec.Rcv_Buf, &Suica_Rec.Rcv_Size )) != 0 ){	/* ﾃﾞｰﾀ受信 */
		// 0以外は-1（タイムアウト）
		if( 2 == Suica_Rec.Com_kind ){										// 入力要求送信後の無応答
			Suica_Rec.Com_kind = 6;											// 入力要求コマンド送信
			Suica_Rec.faze = 1;
			return 0;
		}
		Suica_Rec.Com_kind = 2;		// ２だったことにする
jvma_02_10:
		rtn = -1;															// タイムアウトで処理する
		if( 2 == Suica_Rec.Com_kind ){										// 入力要求送信後の無応答
			Suica_Rec.Com_kind = 4;											// 入力再要求コマンド送信
		}																	// 一括要求の無応答は再度一括要求を行うこととする
		return( rtn );
	}

	switch( rtn = jvma_setup.recv( Suica_Rec.Rcv_Buf, Suica_Rec.Rcv_Size ) ){	/* 受信ﾃﾞｰﾀ解析処理 */
	case 1: /* ACK1 */
		if( ( status == STANDBY_SND_AFTER ||							/* ｺﾏﾝﾄﾞ送信後、ﾃﾞｰﾀ送信要求がある場合、   */
			  status == INPUT_SND_AFTER   ||							/* 出力要求ｺﾏﾝﾄﾞ送信後、ﾃﾞｰﾀ送信を行う */
			  status == REINPUT_SND_AFTER )) {
			  if( jvma_send_que_check() ){
				jvma_command_set();
				Suica_Rec.faze = 1;
				Suica_Rec.Com_kind = 3;									/* 出力要求ｺﾏﾝﾄﾞ送信処理を行う */
				rtn = 0;
				break;
			  }
		}
		rtn = 1;														/* 入力要求を行う */
		break;
	case 2: /* ACK2 */
	case 3: /* ACK3 */
		if( (Suica_Rec.Com_kind == 0) ||								// ｽﾀﾝﾊﾞｲｺﾏﾝﾄﾞ送信後
			(Suica_Rec.Com_kind == 1) ){								// 一括要求送信後
			goto jvma_02_10;											// 無視
		}
		rtn = 1;														
		break;
	case 6: /* NAK */
		if( (Suica_Rec.Com_kind == 0) ||								// ｽﾀﾝﾊﾞｲｺﾏﾝﾄﾞ送信後
			(Suica_Rec.Com_kind == 1) ){								// 一括要求送信後
			goto jvma_02_10;											// 再送
		}
		rtn = 1;
		Suica_Rec.Status = DATA_RCV;									// 他UNITへのPOLに移る
		break;
	case 4: /* ACK4 */
		Suica_Rec.Com_kind = 0;											/* ｽﾀﾝﾊﾞｲｺﾏﾝﾄﾞ送信 */
		Suica_Rec.faze = 1;
		Suica_Rec.Status = DATA_RCV;									// 初期ｼｰｹﾝｽ中であれば、他UNITへのPOLに移り次はスタンバイ
																		// 初期ｼｰｹﾝｽ中でなければ、即時スタンバイ送信
		break;
	case 5: /* ACK5 */
		goto jvma_02_10;
		break;

	case 9:	/* LRC ERROR */												// PHY error、レングス異常を含む
		if( Suica_Rec.Com_kind == 0 ){									// ｽﾀﾝﾊﾞｲ送信後
			goto jvma_02_10;											// 無視（ｽﾀﾝﾊﾞｲｺﾏﾝﾄﾞの再送）
		}
		if( suica_errst & S_LRC_ERR ){									/* Already RetryErr?  */
			suica_err[0] |= S_LRC_ERR;
			rtn = 2;
			break;
		}
		suica_err[4]++;													/* ﾘﾄﾗｲ回数をｶｳﾝﾄｱｯﾌﾟ */
		if( suica_err[4] <= SUICA_RTRY_COUNT_31 ){						// ﾘﾄﾗｲｵｰﾊﾞｰでない
			if( 2 == Suica_Rec.Com_kind ){								// 入力要求送信後の再送（一括要求時は再送も一括）
				Suica_Rec.Com_kind = 4;									// 入力再要求コマンド送信
			}
			Suica_Rec.faze = 1;
			rtn = 0;													/* Retry */
		}else{															/* ﾘﾄﾗｲ回数ｵｰﾊﾞｰ */
			suica_err[0] |= S_LRC_ERR;									/* ｴﾗｰｾｯﾄ */
			rtn = 2;
		}
		break;
	default:
		goto jvma_02_10;
	}
	return( rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 3...Wait Recieve after Send Command                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_03                                                |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : short	| 0:Continue 1:OK ETC:Error                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
short jvma_phase_03( void )
{
	short	rtn;
	unsigned char	status;
	
	status = Suica_Rec.Status;												/* ｽﾃｰﾀｽの保持 */

	if( (rtn = CNM_RCV( Suica_Rec.Rcv_Buf, &Suica_Rec.Rcv_Size )) != 0 ){	/* ﾃﾞｰﾀ受信 */
jvma_03_10:
		rtn = -1;															// タイムアウトで処理する
		return( rtn );
	}

	switch( rtn = jvma_setup.recv( Suica_Rec.Rcv_Buf, Suica_Rec.Rcv_Size ) ){	/* 受信ﾃﾞｰﾀ解析処理 */
	case 1: /* ACK1 */
		if( CNM_SND(Suica_Rec.Snd_Buf, Suica_Rec.Snd_Size) == 0 ){			/* ﾃﾞｰﾀ送信 */
			Suica_Rec.snd_kind = 0;
			jvma_setup.log_regist( &Suica_Rec.Snd_Buf[1],(ushort)(Suica_Rec.Snd_Size-2), 1 );	/* 通信ログ登録 */
			Suica_Rec.faze = 4;								/* ﾃﾞｰﾀ送信後、送信結果ﾌｪｰｽﾞに移行 */
			rtn = 0;
			break;
		}else{
			rtn = -9;
			break;
		}
	case 2: /* ACK2 */
	case 3: /* ACK3 */
		rtn = 1;											/* OK */
		break;
	case 4: /* ACK4 */										/* 出力要求・出力再要求ｺﾏﾝﾄﾞ送信後にACK4受信時 */
		Suica_Rec.faze = 1;									/* ｽﾀﾝﾊﾞｲｺﾏﾝﾄﾞ送信 */
		Suica_Rec.Com_kind = 0;
		break;
	case 5: /* ACK5 */
		if( status == REOUTPUT_SND_AFTER ){					/* 出力再指令送信後 */
// MH810104(S) MH321800(S) ACK5受信でフェーズずれが発生する
			// データ送信後のACK1受信時の後処理を出力再指令送信後のACK5受信でも行う
			if( Suica_Rec.Snd_Buf[1] == S_SELECT_DATA ){
				Suica_Rec.Data.BIT.ADJUSTOR_NOW = 1;		// Suica精算中フラグON！
				Suica_Rec.Data.BIT.SEND_CTRL80 = 0;
				Suica_Rec.Data.BIT.SELECT_SND = 0;			// 選択商品データ送信中 解除
			} else if(Suica_Rec.Snd_Buf[1] == S_BRAND_SEL_DATA) {
				Suica_Rec.Data.BIT.SEND_CTRL80 = 1;			// 選択商品データを送信するまでは取引終了を投げない
			} else if (Suica_Rec.Snd_Buf[1] == S_CNTL_DATA &&
					(Suica_Rec.Snd_Buf[2] & S_CNTL_REBOOT_OK_BIT) == S_CNTL_REBOOT_OK_BIT){
				// 初期化済みフラグをクリア
				Suica_Rec.Data.BIT.INITIALIZE = 0;			// 制御データ && 再起動フラグON = 初期化フラグOFF(起動シーケンス起動!)
				// 再起動要求に応答した場合、INITIALIZE=0になり、通信不良を見つけてもフラグ類を初期化しないのでここで実施する
				ec_flag_clear(1);
			}
// MH810104(E) MH321800(E) ACK5受信でフェーズずれが発生する
			Suica_Rec.faze = 4;								// 送信出来たと判断し、送信データ消去
			if( jvma_send_que_check() ){					/* ｺﾏﾝﾄﾞ送信要求がある場合 */
				jvma_command_set();
				Suica_Rec.faze = 1;
				Suica_Rec.Com_kind = 3;						/* 出力要求ｺﾏﾝﾄﾞ送信処理を行う */
				rtn = 0;		
			}
			else{
				Suica_Rec.faze = 3;							// 送信データ消去のために変えた値を元に戻す
				rtn = 1;									// 次の周期に出力要求する(ACK5時はデータの再送をしない)
			}
		}
		else{												// 出力指令
			goto jvma_03_10;								// 再送
		}
		break;
	case 6: /* NAK */
		ex_errlg( jvma_setup.mdl, ERR_EC_NAKRECEIVE, 1, 0 );/* ｴﾗｰﾛｸﾞ登録(NAK受信) */
		if( suica_errst & S_NAK_RCV_ERR ){					/* Already RetryErr?  */
			suica_err[0] |= S_NAK_RCV_ERR;					/* ｴﾗｰをｾｯﾄ  */
			rtn = 2;
			break;
		}
		suica_err[3]++;										/* ﾘﾄﾗｲ回数をｶｳﾝﾄｱｯﾌﾟ */
		if( suica_err[3] <= SUICA_RTRY_COUNT_21 ){			/* ﾘﾄﾗｲ回数が21回未満 */
			Suica_Rec.faze = 1;								/* ﾘﾄﾗｲ実施 */
			wait2us( 2500L ); 								/* 5ms wait */
			rtn = 0;
		}else{
			suica_err[0] |= S_NAK_RCV_ERR;					/* ｴﾗｰをｾｯﾄ */
			rtn = 2;
		}
		break;
	default:
		break;
	}
	return( rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| Com. Faze 4...Wait Recieve after Send Command                          |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : suica_04                                                |*/
/*| PARAMETER	 : void                                                    |*/
/*| RETURN VALUE : short | 0:Continue 1:OK ETC:Error                       |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
short jvma_phase_04( void )
{
	short	rtn;
	
	if( (rtn = CNM_RCV( Suica_Rec.Rcv_Buf, &Suica_Rec.Rcv_Size )) != 0 ){	/* ﾃﾞｰﾀ受信 */
		rtn = -1;															// タイムアウトで処理する
		Suica_Rec.Com_kind = 5;												// 次は出力再指令送信
		return( rtn );
	}
	switch( rtn = jvma_setup.recv( Suica_Rec.Rcv_Buf, Suica_Rec.Rcv_Size ) ){
	case 1: /* ACK1 */
		if( Suica_Rec.Snd_Buf[1] == S_SELECT_DATA ){
			Suica_Rec.Data.BIT.ADJUSTOR_NOW = 1;						// Suica精算中フラグON！
			Suica_Rec.Data.BIT.SEND_CTRL80 = 0;
// MH810103 GG119202(S) ブランド選択シーケンス変更（T／法人対応から移植）
			Suica_Rec.Data.BIT.SELECT_SND = 0;							// 選択商品データ送信中 解除
// MH810103 GG119202(E) ブランド選択シーケンス変更（T／法人対応から移植）
		} else if(Suica_Rec.Snd_Buf[1] == S_BRAND_SEL_DATA) {
			Suica_Rec.Data.BIT.SEND_CTRL80 = 1;			// 選択商品データを送信するまでは取引終了を投げない
// MH810103 GG119202(S) ブランド選択中フラグがクリアされない対策
//// GG119200(S) ブランド選択シーケンス変更（T／法人対応から移植）
//			Suica_Rec.Data.BIT.BRAND_SEL = 1;			// ブランド選択中
//// GG119200(E) ブランド選択シーケンス変更（T／法人対応から移植）
// MH810103 GG119202(E) ブランド選択中フラグがクリアされない対策
		} else if (Suica_Rec.Snd_Buf[1] == S_CNTL_DATA &&
				   (Suica_Rec.Snd_Buf[2] & S_CNTL_REBOOT_OK_BIT) == S_CNTL_REBOOT_OK_BIT){
			// 初期化済みフラグをクリア
			Suica_Rec.Data.BIT.INITIALIZE = 0;			// 制御データ && 再起動フラグON = 初期化フラグOFF(起動シーケンス起動!)
// MH810103 GG119202(S) 起動シーケンス不具合修正
			// 再起動要求に応答した場合、INITIALIZE=0になり、通信不良を見つけてもフラグ類を初期化しないのでここで実施する
			ec_flag_clear(1);
// MH810103 GG119202(E) 起動シーケンス不具合修正
		}
	// no break;
	case 5: /* ACK5 */
		if( jvma_send_que_check() ){
			jvma_command_set();
			Suica_Rec.faze = 1;
			rtn = 0;
		}else{
		 	rtn = 1;
		}
		break;
	case 2: /* ACK2 */
	case 3: /* ACK3 */
		rtn = 1; /* OK */
		break;
	case 4: /* ACK4 */										/* 出力要求・出力再要求ｺﾏﾝﾄﾞ送信後にACK4受信時 */
		Suica_Rec.faze = 1;									/* ｽﾀﾝﾊﾞｲｺﾏﾝﾄﾞ送信 */
		Suica_Rec.Com_kind = 0;
		break;
	case 6: /* NAK */
		ex_errlg( jvma_setup.mdl, ERR_EC_NAKRECEIVE, 1, 0 );/* ｴﾗｰﾛｸﾞ登録(NAK受信) */
		if( suica_errst & S_NAK_RCV_ERR ){					/* Already RetryErr?  */
			suica_err[0] |= S_NAK_RCV_ERR;
			rtn = 2;
			break;
		}
		suica_err[4]++;
		if( suica_err[4] <= SUICA_RTRY_COUNT_3 ){
			Suica_Rec.faze = 1;
			Suica_Rec.Com_kind = 5;							// 出力再指令
			rtn = 0;
		}else{
			suica_err[0] |= S_NAK_RCV_ERR;
			rtn = 2;
		}
		break;

	case 9:	/* LRC ERROR */
	default:
		if( suica_errst & S_LRC_ERR ){						/* Already RetryErr?  */
			suica_err[0] |= S_LRC_ERR;
			rtn = 2;
			break;
		}
		suica_err[4]++;
		if( suica_err[4] <= SUICA_RTRY_COUNT_3 ){
			Suica_Rec.Com_kind = 5;							// 次は出力再指令
			Suica_Rec.faze = 1;
			rtn = 0;										/* Retry */
		}else{
			suica_err[0] |= S_LRC_ERR;
			rtn = 2;
		}
		break;
	}
	return( rtn );
}

/*[]----------------------------------------------------------------------[]*/
/*| Suica Snd Data que Check                                               |*/
/*[]----------------------------------------------------------------------[]*/
/*| MODULE NAME  : Suica_que_check                                         |*/
/*| PARAMETER    : void                                                    |*/
/*| RETURN VALUE : void                                                    |*/
/*[]----------------------------------------------------------------------[]*/
/*| Author       : G.So                                                    |*/
/*| Date         : 2019.02.07                                              |*/
/*[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]*/
uchar jvma_send_que_check( void )
{
	uchar i;

	if( Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].snd_kind && Suica_Rec.faze == 4 ){
		Suica_Snd_Buf.Suica_Snd_q[Suica_Snd_Buf.read_wpt].snd_kind = 0;

		if( Suica_Snd_Buf.read_wpt > 3 ){
			Suica_Snd_Buf.read_wpt = 0;
		}
		else{
			Suica_Snd_Buf.read_wpt++;
		}
	}

	for( i=0;i<5;i++ ){
		if( Suica_Snd_Buf.Suica_Snd_q[i].snd_kind ){
			return 1;
		}
	}
	if( Suica_Snd_Buf.read_wpt != Suica_Snd_Buf.write_wpt ){	// 未送信ﾃﾞｰﾀがなく、RptとWptがずれている場合
		Suica_Snd_Buf.read_wpt = Suica_Snd_Buf.write_wpt;		// RptをWptにあわせ、送信の矛盾をなくす
		// エラー登録
		err_chk2((char)jvma_setup.mdl, (char)ERR_EC_SND_QUE_GAP, (char)2, (char)0, (char)0, (void*)NULL);
		Suica_Rec.Data.BIT.COMM_ERR = 1;
	}

	return 0;
}
