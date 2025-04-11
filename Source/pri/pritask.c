/*[]---------------------------------------------------------------------------[]*/
/*|		ﾌﾟﾘﾝﾀｰ制御部：ﾒｲﾝ処理													|*/
/*|																				|*/
/*|	ﾌｧｲﾙ名称	:	PriTask.c													|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author		: K.Motohashi													|*/
/*|	Date		: 2005-07-19													|*/
/*|	UpDate		:																|*/
/*|																				|*/
/*[]--------------------------------------- Copyright(C) 2005 AMANO Corp.------[]*/
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"mem_def.h"
#include	"Message.h"
#include	"prm_tbl.h"
#include	"rkn_def.h"
#include	"rkn_cal.h"
#include	"AppServ.h"
#include	"fla_def.h"
#include	"ope_def.h"
#include	"Pri_def.h"
#include	"ntnet_def.h"
#include	"cre_ctrl.h"

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾌﾟﾘﾝﾀ初期化処理														|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	prn_int( void )										|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-08-26													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	prn_int( void )
{
	MsgBuf		*msb;		// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀ
	MsgBuf		msg;		// 受信ﾒｯｾｰｼﾞ格納ﾊﾞｯﾌｧ
	t_TARGET_MSGGET_PRM	Target_WaitMsgID;

// MH810104 GG119201(S) 電子ジャーナル対応
//	Target_WaitMsgID.Count = 5;
	Target_WaitMsgID.Count = 8;
// MH810104 GG119201(E) 電子ジャーナル対応
	Target_WaitMsgID.Command[0] = PREQ_INIT_END_R;
	Target_WaitMsgID.Command[1] = PREQ_INIT_END_J;
	Target_WaitMsgID.Command[2] = TIMEOUT1;
	Target_WaitMsgID.Command[3] = PREQ_SND_COMP_RP;
	Target_WaitMsgID.Command[4] = PREQ_SND_COMP_JP;
// MH810104 GG119201(S) 電子ジャーナル対応
	Target_WaitMsgID.Command[5] = PREQ_MACHINE_INFO;	// 機器情報通知
	Target_WaitMsgID.Command[6] = PREQ_JUSHIN_INFO;		// プリンタステータス変化通知
// MH810104 GG119201(S) 電子ジャーナル対応（初期設定完了待ちタイマ変更）
//	Target_WaitMsgID.Command[7] = TIMEOUT9;				// 時刻／初期設定待ちタイムアウト
	Target_WaitMsgID.Command[7] = EJA_INIT_WAIT_TIMEOUT;	// 時刻／初期設定待ちタイムアウト
// MH810104 GG119201(E) 電子ジャーナル対応（初期設定完了待ちタイマ変更）
// MH810104 GG119201(E) 電子ジャーナル対応
	f_partial_cut = 1;// パーシャルカット動作実行済み(起動前レシートプリンタがパーシャルカットしたかどうかわからないため実行したと見なしておく)

// MH810104 GG119201(S) 電子ジャーナル対応
	memset(&eja_prn_buff, 0, _offsetof(PRN_EJA_DATA_BUFF, eja_proc_data));
	eja_prn_buff.EjaUseFlg = -1;
// MH810104 GG119201(S) 電子ジャーナル対応（起動時処理修正）
//	PrnCmd_InfoReq(PINFO_MACHINE_INFO);													// 機器情報要求
	if (PrnJnlCheck() == ON) {															// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続あり？
		PrnCmd_InfoReq(PINFO_MACHINE_INFO);												// 機器情報要求
	}
// MH810104 GG119201(E) 電子ジャーナル対応（起動時処理修正）
// MH810104 GG119201(E) 電子ジャーナル対応
	PrnInit();																			// ﾌﾟﾘﾝﾀ初期化ｺﾏﾝﾄﾞ送信
	Lagtim( OPETCBNO, 1, PRN_INIT_TIMER );												// ﾌﾟﾘﾝﾀ初期化完了待ちﾀｲﾏｰｽﾀｰﾄ（３秒）

	/*----------------------------------------------*/
	/*	ﾌﾟﾘﾝﾀ初期化完了待ち処理ﾙｰﾌﾟ					*/
	/*----------------------------------------------*/
	for( ; ; ){

		taskchg( IDLETSKNO );															// ｱｲﾄﾞﾙﾀｽｸ切替
		msb = Target_MsgGet( OPETCBNO, &Target_WaitMsgID);								// 期待するﾒｰﾙだけ受信（他は溜めたまま）
		if( NULL == msb ){																// 期待するﾒｰﾙなし（未受信）
			continue;																	// 受信ﾒｯｾｰｼﾞなし → continue
		}

		//	受信ﾒｯｾｰｼﾞあり
		memcpy( &msg , msb , sizeof(MsgBuf) );											// 受信ﾒｯｾｰｼﾞ格納
		FreeBuf( msb );																	// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ開放

		switch( msg.msg.command ){														// ﾒｯｾｰｼﾞｺﾏﾝﾄﾞにより各処理へ分岐

			case PREQ_INIT_END_R:														// ＜ﾚｼｰﾄﾌﾟﾘﾝﾀ初期化終了通知＞

				rct_init_sts = 1;														// ﾚｼｰﾄﾌﾟﾘﾝﾀ初期化状態ｾｯﾄ（初期化完了）
				break;

			case PREQ_INIT_END_J:														// ＜ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ初期化終了通知＞

				jnl_init_sts = 1;														// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ初期化状態ｾｯﾄ（初期化完了）
				break;

			case TIMEOUT1:																// ＜ﾌﾟﾘﾝﾀ初期化完了待ちﾀｲﾏｰﾀｲﾑｱｳﾄ＞

				if( rct_init_sts == 0 ){												// ﾚｼｰﾄﾌﾟﾘﾝﾀ状態ﾁｪｯｸ

					// 初期化完了待ちの場合
					if( rct_init_rty < RCT_INIT_RCNT ){
						//	初期化ﾘﾄﾗｲ回数を終了していない場合
						PrnInit_R();													// ﾚｼｰﾄﾌﾟﾘﾝﾀ初期化（ﾘﾄﾗｲ）
						rct_init_rty++;													// ﾘﾄﾗｲｶｳﾝﾄ＋１
					}
					else{
						//	初期化ﾘﾄﾗｲ回数を終了した場合
						rct_init_sts = 2;												// ﾚｼｰﾄﾌﾟﾘﾝﾀ初期化状態ｾｯﾄ（初期化失敗）
						Prn_errlg( ERR_PRNT_INIT_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );// エラー情報登録（ﾚｼｰﾄﾌﾟﾘﾝﾀ初期化失敗）
					}
				}

				if( jnl_init_sts == 0 ){												// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ状態ﾁｪｯｸ

// MH810104 GG119201(S) 電子ジャーナル対応
					if ( eja_prn_buff.EjaUseFlg == 0 ) {
// MH810104 GG119201(E) 電子ジャーナル対応
					// 初期化完了待ちの場合
					if( jnl_init_rty < JNL_INIT_RCNT ){
						//	初期化ﾘﾄﾗｲ回数を終了していない場合
						PrnInit_J();													// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ初期化（ﾘﾄﾗｲ）
						jnl_init_rty++;													// ﾘﾄﾗｲｶｳﾝﾄ＋１
					}
					else{
						//	初期化ﾘﾄﾗｲ回数を終了した場合
						jnl_init_sts = 2;												// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ初期化状態ｾｯﾄ（初期化失敗）
						Prn_errlg( ERR_PRNT_INIT_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );// エラー情報登録（ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ初期化失敗）
					}
// MH810104 GG119201(S) 電子ジャーナル対応
					}
// MH810104 GG119201(E) 電子ジャーナル対応
				}
				if( (rct_init_sts == 0) || (jnl_init_sts == 0) ){
					// ﾚｼｰﾄ、ｼﾞｬｰﾅﾙどちらか一方でも初期化処理が終了していない場合
					Lagtim( OPETCBNO, 1, PRN_INIT_TIMER );								// ﾌﾟﾘﾝﾀ初期化完了待ちﾀｲﾏｰｽﾀｰﾄ（３秒）
				}

				break;
			case	PREQ_SND_COMP_RP:
				RP_I2CSndReq( I2C_NEXT_SND_REQ );
				break;
				
			case	PREQ_SND_COMP_JP:
				JP_I2CSndReq( I2C_NEXT_SND_REQ );
				break;

// MH810104 GG119201(S) 電子ジャーナル対応
			case PREQ_MACHINE_INFO:			// 機器情報通知
// MH810104 GG119201(S) 電子ジャーナル対応（SDカードマウント完了を待つ）
				if (jnl_init_sts != 0) {
					// 初期化処理終了後は無視する
					break;
				}
// MH810104 GG119201(E) 電子ジャーナル対応（SDカードマウント完了を待つ）
				if (eja_prn_buff.EjaUseFlg < 0) {
					// 機器情報要求
					PrnCmd_InfoReq(PINFO_MACHINE_INFO);
					break;
				}
				if (isEJA_USE()) {
// MH810104 GG119201(S) 電子ジャーナル対応（SDカードマウント完了を待つ）
// MH810105 MH321800(S) ロックSDを挿入でE2203が発生する
					if ((jnl_prn_buff.PrnState[0] & 0x0A) != 0) {
						// 「SDカード使用不可」、「ファイルシステム異常」の場合は初期化失敗とする
						jnl_init_sts = 2;	// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ初期化状態ｾｯﾄ（初期化失敗）
						break;
					}
// MH810105 MH321800(E) ロックSDを挿入でE2203が発生する
					if ((jnl_prn_buff.PrnState[0] & 0x40) == 0) {
						// 時刻／初期設定=未設定のプリンタステータスを待つ
						LagTim500ms(LAG500_EJA_RESET_WAIT_TIMER,
									(short)((PRN_SD_MOUNT_WAIT_TIMER*2)+1),
									PrnCmd_ResetTimeout);
						break;
					}
// MH810104 GG119201(E) 電子ジャーナル対応（SDカードマウント完了を待つ）
					// 初期設定
					PrnCmd_InitSetting();	// [時刻／初期設定]
				}
				else {
					PrnInit_J();			// [初期化]
				}
				break;

			case PREQ_JUSHIN_INFO:			// プリンタステータス変化通知
				PrnCmd_Sts_Proc();
				break;

// MH810104 GG119201(S) 電子ジャーナル対応（初期設定完了待ちタイマ変更）
//			case TIMEOUT9:					// 時刻／初期設定待ちタイムアウト
			case EJA_INIT_WAIT_TIMEOUT:		// 時刻／初期設定待ちタイムアウト
// MH810104 GG119201(E) 電子ジャーナル対応（初期設定完了待ちタイマ変更）
// MH810104 GG119201(S) 電子ジャーナル対応（SDカードマウント完了を待つ）
				if (isEJA_USE()) {
					if ((jnl_prn_buff.PrnState[0] & 0x0A) != 0) {
						// 「SDカード使用不可」、「ファイルシステム異常」の場合は初期化失敗とする
						jnl_init_sts = 2;	// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ初期化状態ｾｯﾄ（初期化失敗）
						break;
					}
				}
// MH810104 GG119201(S) 電子ジャーナル対応（プラテンオープンしていると機器情報要求タイムアウトする）
				else {
					if (jnl_prn_buff.PrnState[0] != 0) {
						// 機器情報要求のタイムアウト、かつ、プリンタステータス受信済みの場合は
						// ジャーナルプリンタ接続と判定する
						// プラテンオープンしたまま起動すると機器情報要求に対する応答を
						// 返さないジャーナルプリンタがあるため
						eja_prn_buff.EjaUseFlg = 0;
						queset(OPETCBNO, PREQ_MACHINE_INFO, 0, NULL);
						break;
					}
				}
// MH810104 GG119201(E) 電子ジャーナル対応（プラテンオープンしていると機器情報要求タイムアウトする）
// MH810104 GG119201(E) 電子ジャーナル対応（SDカードマウント完了を待つ）
				// ハードリセット
				PrnCmd_EJAReset(1);
				break;
// MH810104 GG119201(E) 電子ジャーナル対応

			default:
				break;

		}
		if( (rct_init_sts != 0) && (jnl_init_sts != 0) ){								// ﾚｼｰﾄ、ｼﾞｬｰﾅﾙ両方とも初期化処理終了？

			// 初期化処理終了の場合
			Lagcan( OPETCBNO, 1 );														// ﾌﾟﾘﾝﾀ初期化完了待ちﾀｲﾏｰ解除
// MH810104 GG119201(S) 電子ジャーナル対応（初期設定完了待ちタイマ変更）
			Lagcan( OPETCBNO, TIMERNO_EJA_INIT_WAIT );									// 時刻／初期設定待ちﾀｲﾏｰ解除
// MH810104 GG119201(E) 電子ジャーナル対応（初期設定完了待ちタイマ変更）
			break;																		// 初期化処理を抜ける
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾌﾟﾘﾝﾀ制御ﾒｲﾝ処理													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	pritask( void )										|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-16													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	pritask( void )
{
	MsgBuf		*msb;		// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀ
	MsgBuf		msg;		// 受信ﾒｯｾｰｼﾞ格納ﾊﾞｯﾌｧ
	MsgBuf		*OutMsg;	// ﾒｯｾｰｼﾞ登録用ﾊﾞｯﾌｧﾎﾟｲﾝﾀｰ
	uchar		pri_kind;	// 印字対象ﾌﾟﾘﾝﾀ種別
// MH810104 GG119201(S) 電子ジャーナル対応
	uchar		cmd;
	uchar		ret;
// MH810104 GG119201(E) 電子ジャーナル対応

	/*----------------------------------------------*/
	/*	ﾌﾟﾘﾝﾀ初期化完了待ちﾙｰﾌﾟ						*/
	/*----------------------------------------------*/
	for( ; ; ){

		taskchg( IDLETSKNO );									// ｱｲﾄﾞﾙﾀｽｸ切替

		if( (rct_init_sts != 0) && (jnl_init_sts != 0) ){		// ﾚｼｰﾄ、ｼﾞｬｰﾅﾙ両方とも初期化処理終了？

			// 初期化処理終了

			Header_Rsts	= FLT_ReadHeader( &Header_Data[0][0] );	// ﾍｯﾀﾞｰ印字ﾃﾞｰﾀをﾌﾗｯｼｭﾒﾓﾘから読込
			Footer_Rsts	= FLT_ReadFooter( &Footer_Data[0][0] );	// ﾌｯﾀｰ印字ﾃﾞｰﾀをﾌﾗｯｼｭﾒﾓﾘから読込
			AcceptFooter_Rsts = FLT_ReadAcceptFooter( &AcceptFooter_Data[0][0] );
			Syomei_Rsts	= FLT_ReadSyomei( &Syomei_Data[0][0] );	// 署名欄印字ﾃﾞｰﾀをﾌﾗｯｼｭﾒﾓﾘから読込
			Kamei_Rsts	= FLT_ReadKamei( &Kamei_Data[0][0] );	// 加盟店印字ﾃﾞｰﾀをﾌﾗｯｼｭﾒﾓﾘから読込
			AzuFtr_Rsts = FLT_ReadAzuFtr( &AzuFtr_Data[0][0] );
// 仕様変更(S) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
			CreKbrFtr_Rsts = FLT_ReadCreKbrFtr( &CreKbrFtr_Data[0][0] );
			EpayKbrFtr_Rsts = FLT_ReadEpayKbrFtr( &EpayKbrFtr_Data[0][0] );
// 仕様変更(E) K.Onodera 2016/10/28 #1531 領収証フッターを印字する
// 仕様変更(S) K.Onodera 2016/11/08 後日支払額フッター
			FutureFtr_Rsts = FLT_ReadFutureFtr( &FutureFtr_Data[0][0] );
// 仕様変更(E) K.Onodera 2016/11/08 後日支払額フッター
// MH810105(S) MH364301 QRコード決済対応
			EmgFooter_Rsts = FLT_ReadEmgFooter( &EmgFooter_Data[0][0] );	// 障害連絡票ﾌｯﾀｰ印字ﾃﾞｰﾀをﾌﾗｯｼｭﾒﾓﾘから読込
// MH810105(E) MH364301 QRコード決済対応
// GG129001(S) データ保管サービス対応（登録番号をセットする）
			PrnGetRegistNum();
// GG129001(E) データ保管サービス対応（登録番号をセットする

// MH810104 GG119201(S) 電子ジャーナル対応
			// 起動時のエラーチェック
			PrnCmd_EJAErrRegist();
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
			// pritask宛のメッセージクリア
			PrnMsgBoxClear();
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
			// 印字途中のデータの書込み再開
			ret = PrnCmd_ReWriteCheck();
// MH810104 GG119201(E) 電子ジャーナル対応
			break;												// 初期化完了待ちﾙｰﾌﾟを抜ける
		}
	}

	/*----------------------------------------------*/
	/*	ﾌﾟﾘﾝﾀﾀｽｸ処理ﾒｲﾝﾙｰﾌﾟ							*/
	/*----------------------------------------------*/
	for( ; ; ){

		taskchg( IDLETSKNO );												// ｱｲﾄﾞﾙﾀｽｸ切替

		PriDummyMsg = OFF;													// ﾀﾞﾐｰﾒｯｾｰｼﾞ登録状態：ＯＦＦ
		TgtMsg = NULL;														// 処理ﾒｯｾｰｼﾞ保存状態：ＯＦＦ
// MH810104 GG119201(S) 電子ジャーナル対応
		if (ret != 0) {
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
//			PrnCmd_ReWriteStart(1);											// データ書込み再開（復電）
//			ret = 0;
			switch (ret) {
			case 98:
				// データ書込み途中で復電の印字中（印字待ちなし）
				// 印字データキューの印字中（印字待ちなし）
				if (rct_proc_data.Printing == 0 &&
					jnl_proc_data.Printing == 0) {
					ret = 0;
				}
				break;
			case 99:
				// データ書込み途中で復電の印字中（印字待ちあり）
				if (rct_proc_data.Printing == 0 &&
					jnl_proc_data.Printing == 0) {
					// データ書込み途中で復電の印字完了
					PrnCmd_ReWriteCheck();
					// データ書込み再開（復電）
					PrnCmd_ReWriteStart(1);
					ret = 0;
				}
				break;
			default:
				// データ書込み途中で復電の印字開始
				// 印字データキューの印字開始
				PrnCmd_ReWriteStart(1);
				if (ret == 3) {
					// データ書込み途中で復電の印字中（印字待ちあり）
					ret = 99;
				}
				else {
					// データ書込み途中で復電の印字中（印字待ちなし）
					// 印字データキューの印字中（印字待ちなし）
					ret = 98;
				}
				break;
			}
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
		}
// MH810104 GG119201(E) 電子ジャーナル対応

		_di();																// 割込み禁止

		for( ; ; ){															// 処理対象ﾒｯｾｰｼﾞ取得処理ﾙｰﾌﾟ

			if( ( msb = GetMsg( PRNTCBNO ) ) == NULL ){						// ﾌﾟﾘﾝﾀﾀｽｸ受信ﾒｯｾｰｼﾞ取得
				break;														// 受信ﾒｯｾｰｼﾞなし → 検索終了
			}
			// 受信ﾒｯｾｰｼﾞあり
			if( TgtMsGet( msb ) == OK ){									// 処理対象とするﾒｯｾｰｼﾞを取得するまでﾒｯｾｰｼﾞ受信を繰り返す
				break;														// 処理対象ﾒｯｾｰｼﾞ取得あり → 検索終了
			}
		}
		_ei();																// 割込み禁止解除

		if( msb == NULL ){
			continue;														// 受信ﾒｯｾｰｼﾞなし→ continue（ｱｲﾄﾞﾙﾀｽｸ切替）
		}
		// 受信ﾒｯｾｰｼﾞあり
		memcpy( &msg ,msb ,sizeof(MsgBuf) );								// 処理ﾒｯｾｰｼﾞ←受信ﾒｯｾｰｼﾞ
		FreeBuf( msb );														// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ開放

		if( msg.msg.command == PREQ_DUMMY ){

			// ﾀﾞﾐｰﾒｯｾｰｼﾞ受信（全登録ﾒｯｾｰｼﾞ検索終了）の場合
			if( TgtMsg == NULL ){
				// 処理ﾒｯｾｰｼﾞ保存なし
				continue;													// 処理対象ﾒｯｾｰｼﾞなし → continue（ｱｲﾄﾞﾙﾀｽｸ切替）
			}
			// 処理ﾒｯｾｰｼﾞ保存あり
			memcpy( &msg ,TgtMsg ,sizeof(MsgBuf) );							// 処理ﾒｯｾｰｼﾞ←保存ﾒｯｾｰｼﾞ
			FreeBuf( TgtMsg );												// 保存ﾒｯｾｰｼﾞﾊﾞｯﾌｧ開放
		}

		if( CMND_CHK1_SNO <= (msg.msg.command & MSG_CMND_MASK) && (msg.msg.command & MSG_CMND_MASK) <= CMND_CHK1_ENO ){

			//	ﾌﾟﾘﾝﾀｰ制御ﾒｯｾｰｼﾞの場合

			/*----------------------------------------------*/
			/*	処理対象とするﾌﾟﾘﾝﾀ種別を取得する			*/
			/*----------------------------------------------*/
			pri_kind = msg.msg.data[0];		// ﾒｯｾｰｼﾞﾃﾞｰﾀよりﾌﾟﾘﾝﾀ種別を取得

			if( msg.msg.command & INNJI_NEXTMASK ){

				//	印字処理中の次編集要求ﾒｯｾｰｼﾞ
				terget_pri	= (uchar)( (pri_kind & NEXT_PRI_MASK) >> 4 );	// 処理対象ﾌﾟﾘﾝﾀ種別ｾｯﾄ
				next_prn_msg = ON;											// 次編集要求ﾒｯｾｰｼﾞﾌﾗｸﾞ：ON
			}
			else{
				//	その他のﾒｯｾｰｼﾞ
				next_prn_msg	= OFF;										// 次編集要求ﾒｯｾｰｼﾞﾌﾗｸﾞ：OFF

				if( msg.msg.command == PREQ_TGOUKEI			// 「Ｔ合計」
				||	msg.msg.command == PREQ_GTGOUKEI 		// 「ＧＴ合計」
				||	msg.msg.command == PREQ_MTGOUKEI ) {	// 「ＭＴ合計」


					//	合計記録印字要求の場合
					terget_pri	= (uchar)(pri_kind & REQ_PRI_MASK);			// 処理対象ﾌﾟﾘﾝﾀ種別ｾｯﾄ

					if( terget_pri == 0 ){

						// 「印字先ﾌﾟﾘﾝﾀの設定＝なし」の場合
						rct_goukei_pri = OFF;
						jnl_goukei_pri = OFF;

						PrnEndMsg(	msg.msg.command,						// 「印字終了（正常終了）」ﾒｯｾｰｼﾞ送信
									PRI_NML_END,
									PRI_ERR_NON,
									R_PRI );

						continue;											// 印字先ﾌﾟﾘﾝﾀなし → continue
					}
				}
				else if( YES == PrnGoukeiChk( msg.msg.command ) ){		// 印字要求種別＝合計記録？
					if( pri_kind == R_PRI || pri_kind == RJ_PRI ) {
						// opeで操作するのでturikan_pri_statusは1回クリアする
						terget_pri = PrnGoukeiPri(msg.msg.command);			// 合計記録印字の処理対象ﾌﾟﾘﾝﾀ種別ｾｯﾄ
					}
					else {
						terget_pri	= (uchar)(pri_kind & REQ_PRI_MASK);		// 処理対象ﾌﾟﾘﾝﾀ種別ｾｯﾄ
					}

					if( terget_pri == 0 ){

						// 「印字先ﾌﾟﾘﾝﾀの設定＝なし」の場合
						rct_goukei_pri = OFF;
						jnl_goukei_pri = OFF;

						PrnEndMsg(	msg.msg.command,						// 「印字終了（正常終了）」ﾒｯｾｰｼﾞ送信
									PRI_NML_END,
									PRI_ERR_NON,
									R_PRI );

						continue;											// 印字先ﾌﾟﾘﾝﾀなし → continue
					}
				}
				else if( msg.msg.command == PREQ_TURIKAN  )  {				// 釣銭管理合計？
					if( pri_kind == R_PRI || pri_kind == RJ_PRI ) {
						// opeで操作するのでturikan_pri_statusは1回クリアする
						turikan_pri_status = 0;								// 釣銭管理ﾌﾟﾘﾝﾄが印字終了をｾｯﾄする
						terget_pri = PrnGoukeiPri(msg.msg.command);			// 合計記録印字の処理対象ﾌﾟﾘﾝﾀ種別ｾｯﾄ
					}
					else {
						terget_pri	= (uchar)(pri_kind & REQ_PRI_MASK);		// 処理対象ﾌﾟﾘﾝﾀ種別ｾｯﾄ
					}

					if( terget_pri == 0 ){

						// 「印字先ﾌﾟﾘﾝﾀの設定＝なし」の場合
						rct_goukei_pri = OFF;
						jnl_goukei_pri = OFF;

						PrnEndMsg(	msg.msg.command,						// 「印字終了（正常終了）」ﾒｯｾｰｼﾞ送信
									PRI_NML_END,
									PRI_ERR_NON,
									R_PRI );

						continue;											// 印字先ﾌﾟﾘﾝﾀなし → continue
					}
				}
				else if( msg.msg.command == PREQ_AT_SYUUKEI ){				// 印字要求種別＝自動集計？

					//	自動集計印字要求の場合

					if( pri_kind == R_PRI || pri_kind == RJ_PRI ) {
						terget_pri = PrnAsyuukeiPri();						// 自動集計印字の処理対象ﾌﾟﾘﾝﾀ種別ｾｯﾄ
					}
					else {
						terget_pri = J_PRI;
					}

					if( terget_pri == 0 ){

						// 「印字先ﾌﾟﾘﾝﾀの設定＝なし」の場合
						rct_atsyuk_pri = OFF;
						jnl_atsyuk_pri = OFF;

						PrnEndMsg(	msg.msg.command,						// 「印字終了（正常終了）」ﾒｯｾｰｼﾞ送信
									PRI_NML_END,
									PRI_ERR_NON,
									R_PRI );

						continue;											// 印字先ﾌﾟﾘﾝﾀなし → continue
					}
				}
				else{
					//	合計記録印字要求、自動集計印字要求　以外の場合
					if(  msg.msg.command != PREQ_INNJI_TYUUSHI ) {
						terget_pri	= (uchar)(pri_kind & REQ_PRI_MASK);			// 処理対象ﾌﾟﾘﾝﾀ種別ｾｯﾄ
					}
				}
			}
			/*----------------------------------------------*/
			/*	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀﾁｪｯｸ								*/
			/*----------------------------------------------*/
			if( PrnJnlCheck() != ON ){		// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続あり？

				//	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ接続なしの場合

				if( terget_pri == J_PRI ){
					continue;							// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ処理要求 → continue
				}
				else if( terget_pri == RJ_PRI ){
					terget_pri = R_PRI;					// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ処理要求 → 対象をﾚｼｰﾄﾌﾟﾘﾝﾀのみとする
				}

			}
			/*----------------------------------------------*/
			/*	印字ﾃﾞｰﾀ編集処理可／不可ﾁｪｯｸ				*/
			/*----------------------------------------------*/
			if( CMND_CHK2_SNO <= (msg.msg.command & MSG_CMND_MASK) && (msg.msg.command & MSG_CMND_MASK) <= CMND_CHK2_ENO ){

				// 印字処理可／不可ﾁｪｯｸ対象ﾒｯｾｰｼﾞ（印字ﾃﾞｰﾀ編集要求）の場合

				if( RcvCheckWait( &msg.msg ) == NG ){	// 印字処理可／不可ﾁｪｯｸ
					continue;							// 印字処理不可 → continue
				}
// MH810104 GG119201(S) 電子ジャーナル対応
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
//				if (terget_pri == J_PRI && isEJA_USE()) {
//					// FT-4000ではレシート＋ジャーナルの同時印字はない
//					// データ書込み開始を通知する
//					PrnCmd_WriteStartEnd(0, &msg.msg);
//				}
//				else if (terget_pri == RJ_PRI && isEJA_USE()) {
				if (terget_pri == RJ_PRI && isEJA_USE()) {
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
					// レシート＋ジャーナル印字要求時の復電対策として
					// 印字制御データと印字メッセージを保持する
					memcpy(&eja_prn_buff.eja_proc_data, &rct_proc_data, sizeof(rct_proc_data));
					memcpy(&eja_prn_buff.PrnMsg, &msg.msg, sizeof(msg.msg));
				}
// MH810104 GG119201(E) 電子ジャーナル対応
			}
			prn_edit_wk_clr();							// 編集処理ﾜｰｸｸﾘｱ
		}

		/*----------------------------------------------*/
		/*	受信ﾒｯｾｰｼﾞ処理								*/
		/*----------------------------------------------*/
		if( CMND_CHK2_SNO <= (msg.msg.command & MSG_CMND_MASK) && (msg.msg.command & MSG_CMND_MASK) <= CMND_CHK2_ENO && Inji_Cancel_chk( terget_pri ) != YES ){	// 印字中止時ではない場合。
			if( terget_pri == R_PRI || terget_pri == RJ_PRI ){
				Prn_LogoHead_PostHead ( );		/* 先印字で全て出せなかった場合、残りを印字し、空改行する */
			}
		}

		if( CMND_CHK2_SNO <= msg.msg.command && msg.msg.command <= CMND_CHK2_ENO && terget_pri == RJ_PRI){
			// レシート＋ジャーナルの場合はジャーナル印字もあるフラグを立てて、対象をレシートのみする
			PriBothPrint = 1;
			terget_pri = R_PRI;
		}
		
		switch( msg.msg.command ){	// ﾒｯｾｰｼﾞｺﾏﾝﾄﾞにより各処理へ分岐

			case	PREQ_RYOUSYUU:			// ＜領収証＞印字要求	※精算中止印字を含む

				PrnRYOUSYUU( &msg.msg );
				break;

			case	PREQ_AZUKARI:			// ＜預り証＞印字要求

				PrnAZUKARI( &msg.msg );
				break;

			case	PREQ_UKETUKE:			// ＜受付券＞印字要求

				PrnUKETUKE( &msg.msg );
				break;

			case	PREQ_TSYOUKEI:			// ＜Ｔ小計＞印字要求

				syuukei_kind = TSYOUKEI;	// 集計種別ｾｯﾄ
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_TGOUKEI:			// ＜Ｔ合計＞印字要求

				syuukei_kind = TGOUKEI;		// 集計種別ｾｯﾄ
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_GTSYOUKEI:			// ＜ＧＴ小計＞印字要求

				syuukei_kind = GTSYOUKEI;	// 集計種別ｾｯﾄ
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_GTGOUKEI:			// ＜ＧＴ合計＞印字要求

				syuukei_kind = GTGOUKEI;	// 集計種別ｾｯﾄ
				PrnSYUUKEI( &msg.msg );
				break;
				
			case	PREQ_MTSYOUKEI:			// ＜ＭＴ小計＞印字要求

				syuukei_kind = MTSYOUKEI;	// 集計種別ｾｯﾄ
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_MTGOUKEI:			// ＜ＭＴ合計＞印字要求

				syuukei_kind = MTGOUKEI;	// 集計種別ｾｯﾄ
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_F_TSYOUKEI:		// ＜複数Ｔ小計＞印字要求

				syuukei_kind = F_TSYOUKEI;	// 集計種別ｾｯﾄ
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_F_TGOUKEI:			// ＜複数Ｔ合計＞印字要求

				syuukei_kind = F_TGOUKEI;	// 集計種別ｾｯﾄ
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_F_GTSYOUKEI:		// ＜複数ＧＴ小計＞印字要求

				syuukei_kind = F_GTSYOUKEI;	// 集計種別ｾｯﾄ
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_F_GTGOUKEI:		// ＜複数ＧＴ合計＞印字要求

				syuukei_kind = F_GTGOUKEI;	// 集計種別ｾｯﾄ
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_TGOUKEI_Z:			// ＜前回Ｔ合計＞印字要求

				syuukei_kind = TGOUKEI_Z;	// 集計種別ｾｯﾄ
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_GTGOUKEI_Z:		// ＜前回ＧＴ合計＞印字要求

				syuukei_kind = GTGOUKEI_Z;	// 集計種別ｾｯﾄ
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_MTGOUKEI_Z:		// ＜前回ＭＴ合計＞印字要求

				syuukei_kind = MTGOUKEI_Z;	// 集計種別ｾｯﾄ
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_F_TGOUKEI_Z:		// ＜前回複数Ｔ合計＞印字要求

				syuukei_kind = F_TGOUKEI_Z;	// 集計種別ｾｯﾄ
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_F_GTGOUKEI_Z:		// ＜前回複数ＧＴ合計＞印字要求

				syuukei_kind = F_GTGOUKEI_Z;// 集計種別ｾｯﾄ
				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_TGOUKEI_J:			// ＜Ｔ合計情報＞印字要求

				syuukei_kind = TGOUKEI;		// 集計種別ｾｯﾄ
				PrnSYUUKEI_LOG( &msg.msg );
				break;

			case	PREQ_GTGOUKEI_J:		// ＜ＧＴ合計情報＞印字要求

				syuukei_kind = GTGOUKEI;		// 集計種別ｾｯﾄ
				PrnSYUUKEI_LOG( &msg.msg );
				break;

			case	PREQ_AT_SYUUKEI:		// ＜自動集計＞印字要求

				syuukei_kind = TGOUKEI;		// 集計種別ｾｯﾄ（自動集計はＴ合計と同様）

				PrnSYUUKEI( &msg.msg );
				break;

			case	PREQ_COKINKO_G:			// ＜ｺｲﾝ金庫合計＞印字要求

				syuukei_kind = COKINKO_G;	// 集計種別ｾｯﾄ
				PrnCOKI_SK( &msg.msg );
				break;

			case	PREQ_COKINKO_S:			// ＜ｺｲﾝ金庫小計＞印字要求

				syuukei_kind = COKINKO_S;	// 集計種別ｾｯﾄ
				PrnCOKI_SK( &msg.msg );
				break;

			case	PREQ_COKINKO_Z:			// ＜前回ｺｲﾝ金庫合計＞印字要求

				syuukei_kind = COKINKO_Z;	// 集計種別ｾｯﾄ
				PrnCOKI_SK( &msg.msg );
				break;

			case	PREQ_COKINKO_J:			// ＜ｺｲﾝ金庫集計情報＞印字要求

				syuukei_kind = COKINKO_G;	// 集計種別ｾｯﾄ
				PrnCOKI_JO( &msg.msg );
				break;

			case	PREQ_SIKINKO_G:			// ＜紙幣金庫合計＞印字要求

				syuukei_kind = SIKINKO_G;	// 集計種別ｾｯﾄ
				PrnSIKI_SK( &msg.msg );
				break;

			case	PREQ_SIKINKO_S:			// ＜紙幣金庫小計＞印字要求

				syuukei_kind = SIKINKO_S;	// 集計種別ｾｯﾄ
				PrnSIKI_SK( &msg.msg );
				break;

			case	PREQ_SIKINKO_Z:			// ＜前回紙幣金庫合計＞印字要求

				syuukei_kind = SIKINKO_Z;	// 集計種別ｾｯﾄ
				PrnSIKI_SK( &msg.msg );
				break;

			case	PREQ_SIKINKO_J:			// ＜紙幣金庫集計情報＞印字要求

				syuukei_kind = SIKINKO_G;	// 集計種別ｾｯﾄ
				PrnSIKI_JO( &msg.msg );
				break;

			case	PREQ_TURIKAN:			// ＜釣銭管理集計＞印字要求

				syuukei_kind = TURIKAN;		// 集計種別ｾｯﾄ
				PrnTURIKAN( &msg.msg );
				break;

			case	PREQ_TURIKAN_S:			// ＜釣銭管理集計（小計）＞印字要求

				syuukei_kind = TURIKAN_S;	// 集計種別ｾｯﾄ
				PrnTURIKAN( &msg.msg );
				break;

			case	PREQ_TURIKAN_LOG:		// ＜釣銭管理集計履歴＞印字要求

				syuukei_kind = TURIKAN;		// 集計種別ｾｯﾄ
				PrnTURIKAN_LOG( &msg.msg );
				break;

			case	PREQ_ERR_JOU:			// ＜エラー情報＞印字要求

				PrnERR_JOU( &msg.msg );
				break;

			case	PREQ_ERR_LOG:			// ＜エラー情報履歴＞印字要求

				PrnERR_LOG( &msg.msg );
				break;

			case	PREQ_ARM_JOU:			// ＜アラーム情報＞印字要求

				PrnARM_JOU( &msg.msg );
				break;

			case	PREQ_ARM_LOG:			// ＜アラーム情報履歴＞印字要求

				PrnARM_LOG( &msg.msg );
				break;

			case	PREQ_OPE_JOU:			// ＜操作情報＞印字要求

				PrnOPE_JOU( &msg.msg );
				break;

			case	PREQ_OPE_LOG:			// ＜操作情報履歴＞印字要求

				PrnOPE_LOG( &msg.msg );
				break;

			case	PREQ_MON_JOU:			// ＜モニタ情報＞印字要求

				PrnMON_JOU( &msg.msg );
				break;

			case	PREQ_MON_LOG:			// ＜モニタ情報履歴＞印字要求

				PrnMON_LOG( &msg.msg );
				break;

			case	PREQ_RMON_JOU:			// ＜遠隔監視情報＞印字要求
				PrnRMON_JOU( &msg.msg );
				break;

			case	PREQ_RMON_LOG:			// ＜遠隔監視情報履歴＞印字要求
				PrnRMON_LOG( &msg.msg );
				break;

			case	PREQ_NG_LOG:			// ＜不正ログ＞印字要求

				PrnNG_LOG( &msg.msg );
				break;
			case	PREQ_IO_LOG:			// ＜入出庫ログ＞印字要求

				PrnIO_LOG( &msg.msg );
				break;

			case	PREQ_SETTEIDATA:		// ＜設定データ＞印字要求

				PrnSETTEI( &msg.msg );
				break;

			case	PREQ_DOUSACNT:			// ＜動作カウント＞印字要求

				PrnDOUSAC( &msg.msg );
				break;

			case	PREQ_LK_DOUSACNT:		// ＜ﾛｯｸ装置動作ｶｳﾝﾄ＞印字要求

				PrnLOCK_DCNT( &msg.msg );
				break;

			case	PREQ_LOCK_PARA:			// ＜車室パラメータ＞印字要求

				PrnLOCK_PARA( &msg.msg );
				break;

			case	PREQ_LOCK_SETTEI:		// ＜ロック装置設定＞印字要求

				PrnLOCK_SETTEI( &msg.msg );
				break;

			case	PREQ_PKJOU_NOW:			// ＜車室情報（現在）＞印字要求

				PrnPKJOU_NOW( &msg.msg );
				break;

			case	PREQ_PKJOU_SAV:			// ＜車室情報（退避）＞印字要求

				PrnPKJOU_SAV( &msg.msg );
				break;

			case	PREQ_TEIKI_DATA1:		// ＜定期有効／無効＞印字要求

				PrnTEIKIDATA1( &msg.msg );
				break;

			case	PREQ_TEIKI_DATA2:		// ＜定期入庫／出庫＞印字要求

				PrnTEIKIDATA2( &msg.msg );
				break;

			case	PREQ_SVSTIK_KIGEN:		// ＜ｻｰﾋﾞｽ券期限＞印字要求

				PrnSVSTIK_KIGEN( &msg.msg );
				break;

			case	PREQ_SPLDAY:			// ＜特別日＞印字要求

				PrnSPLDAY( &msg.msg );
				break;

			case	PREQ_TEIRYUU_JOU:		// ＜停留車情報＞印字要求

				PrnTEIRYUU_JOU( &msg.msg );
				break;

			case	PREQ_TEIFUK_JOU:		// ＜停復電情報＞印字要求

				PrnTEIFUK_JOU( &msg.msg );
				break;

			case	PREQ_TEIFUK_LOG:		// ＜停復電履歴＞印字要求

				PrnTEIFUK_LOG( &msg.msg );
				break;

			case	PREQ_KOBETUSEISAN:		// ＜個別精算情報＞印字要求	

				PrnSEISAN_LOG( &msg.msg );
				break;

			case	PREQ_FUSKYO_JOU:		// ＜不正・強制出庫情報＞印字要求

				PrnFUSKYO_JOU( &msg.msg );
				break;

			case	PREQ_FUSKYO_LOG:		// ＜不正・強制出庫履歴＞印字要求

				PrnFUSKYO_LOG( &msg.msg );
				break;

			case	PREQ_IREKAE_TEST:		// ＜用紙入れ替え時テスト＞印字要求	

				PrnIREKAE_TEST( &msg.msg );
				break;

			case	PREQ_PRINT_TEST:		// ＜プリンタテスト＞印字要求	

				PrnPRINT_TEST( &msg.msg );
				break;

			case	PREQ_LOGO_REGIST:		// ＜ロゴ印字データ＞登録要求	

				PrnLOGO_REGIST( &msg.msg );
				break;

// MH321800(S) G.So ICクレジット対応 不要機能削除(CCT)
//			case	PREQ_CREDIT_CANCEL:		// ＜クレジット取消情報＞印字要求
//				break;
//
// MH321800(E) G.So ICクレジット対応 不要機能削除(CCT)
// MH810103 GG119202(S) 不要機能削除(センタークレジット)
//			case	PREQ_CREDIT_USE:		// ＜クレジット利用明細＞印字要求
//			case	PREQ_HOJIN_USE:
//				PrnCRE_USE( &msg.msg );
//				break;
//
//			case	PREQ_CREDIT_UNSEND:		// ＜クレジット未送信売上依頼データ＞印字要求
//				PrnCRE_UNSEND( &msg.msg );
//				break;
//			case	PREQ_CREDIT_SALENG:		// ＜クレジット売上拒否データ＞印字要求
//				PrnCRE_SALENG( &msg.msg );
//				break;
// MH810103 GG119202(E) 不要機能削除(センタークレジット)

			case	PREQ_INNJI_TYUUSHI:		// 印字中止要求

				PrnINJI_TYUUSHI( &msg.msg );
				break;

			case	PREQ_INNJI_END:			// 印字編集ﾃﾞｰﾀ１ﾌﾞﾛｯｸ印字終了

				PrnINJI_END( &msg.msg );
				break;

			case	PREQ_PRINTER_ERR:		// ﾌﾟﾘﾝﾀｰｴﾗｰ処理

				if( msg.msg.data[0] == R_PRI ){
					// ﾚｼｰﾄﾌﾟﾘﾝﾀｴﾗｰ割込み発生
					Prn_errlg( ERR_PRNT_ERR_IRQ, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// エラー情報登録（ﾚｼｰﾄﾌﾟﾘﾝﾀｴﾗｰ割込み発生）
				}
				else{
					// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀｴﾗｰ割込み発生
					Prn_errlg( ERR_PRNT_ERR_IRQ, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// エラー情報登録（ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀｴﾗｰ割込み発生）
				}
		 		LagTim500ms( LAG500_PRINTER_ERR_RECIEVE, 60*2, PriErrRecieve );	// １分ﾀｲﾏｰ
				break;

			case	PREQ_JUSHIN_ERR1:		// 受信エラー１（ﾍﾟｰﾊﾟｰﾆｱｴﾝﾄﾞ以外のｽﾃｰﾀｽ変化）
				if( (OutMsg = GetBuf()) != NULL )
				{
					OutMsg->msg.command = msg.msg.command;	// ｺﾏﾝﾄﾞ	：受信エラー１
					OutMsg->msg.data[0] = terget_pri;		// data[0]	：ﾌﾟﾘﾝﾀ種別（ﾚｼｰﾄ／ｼﾞｬｰﾅﾙ）
					PutMsg( OPETCBNO, OutMsg );				// ｵﾍﾟﾚｰｼｮﾝﾀｽｸへﾒｯｾｰｼﾞ送信
				}

				PrnERREND_PROC();							// 印字異常終了制御

				break;

			case	PREQ_JUSHIN_ERR2:		// 受信エラー２(ﾍﾟｰﾊﾟｰﾆｱｴﾝﾄﾞ変化時)
				if( (OutMsg = GetBuf()) != NULL )
				{
					if( terget_pri == R_PRI ){
						Lagtim( PRNTCBNO ,3 ,PRN_PNEND_TIMER );	// ﾚｼｰﾄ		：ﾍﾟｰﾊﾟｰﾆｱｴﾝﾄﾞﾀｲﾏｰｽﾀｰﾄ（１０秒）
					}
					else{
						Lagtim( PRNTCBNO ,4 ,PRN_PNEND_TIMER );	// ｼﾞｬｰﾅﾙ	：ﾍﾟｰﾊﾟｰﾆｱｴﾝﾄﾞﾀｲﾏｰｽﾀｰﾄ（１０秒）
					}
					OutMsg->msg.command = PREQ_JUSHIN_ERR1;		// ｺﾏﾝﾄﾞ	：受信エラー１
					OutMsg->msg.data[0] = terget_pri;			// data[0]	：ﾌﾟﾘﾝﾀ種別（ﾚｼｰﾄ／ｼﾞｬｰﾅﾙ）
					PutMsg( OPETCBNO, OutMsg );					// ｵﾍﾟﾚｰｼｮﾝﾀｽｸへﾒｯｾｰｼﾞ送信
				}
				break;

// MH810104 GG119201(S) 電子ジャーナル対応
			case PREQ_JUSHIN_INFO:			// プリンタステータス変化通知（電子ジャーナル用）
				if ((OutMsg = GetBuf()) != NULL) {
					OutMsg->msg.command = msg.msg.command;	// ｺﾏﾝﾄﾞ	：プリンタステータス変化通知
					OutMsg->msg.data[0] = terget_pri;		// data[0]	：ﾌﾟﾘﾝﾀ種別（ｼﾞｬｰﾅﾙ）
					PutMsg( OPETCBNO, OutMsg );				// ｵﾍﾟﾚｰｼｮﾝﾀｽｸへﾒｯｾｰｼﾞ送信
				}

				PrnERREND_PROC();							// 印字異常終了制御
				PrnCmd_Sts_Proc();							// プリンタステータス処理
				break;

			case PREQ_PRN_INFO:				// プリンタ情報要求
				cmd = msg.msg.data[1];
				PrnCmd_InfoReq(cmd);
				break;

			case PREQ_CLOCK_REQ:			// 時刻設定要求
// MH810104 GG119201(S) 電子ジャーナル対応（書込み中に時計設定しない）
				if (jnl_proc_data.Printing != 0 &&
					(jnl_prn_buff.PrnState[0] & 0x40) == 0) {
					// 書込み中、かつ、時刻／初期設定≠未設定の場合は
					// 書込み完了を待つ
					eja_prn_buff.PrnClkReqFlg = 1;
					break;
				}
				PrnCmd_Clock();
				break;
			case PREQ_INIT_SET_REQ:			// 時刻／初期設定要求
// MH810104 GG119201(E) 電子ジャーナル対応（書込み中に時計設定しない）
// MH810105 MH321800(S) ロックSDを挿入でE2203が発生する
				if ((jnl_prn_buff.PrnState[0] & 0x0A) != 0) {
					// 「SDカード使用不可」、「ファイルシステム異常」の場合は
					// 時刻／初期設定コマンドを送信しない
					break;
				}
// MH810105 MH321800(E) ロックSDを挿入でE2203が発生する
				PrnCmd_InitSetting();
				break;

			case PREQ_CLOCK_SET_CMP:		// 時刻設定完了
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
				if (ret != 0) {
					// 復電後のデータ書込み再開中
					break;
				}
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
				PrnCmd_ReWriteStart(0);						// データ書込み再開
				break;
// MH810104 GG119201(E) 電子ジャーナル対応

			case TIMEOUT1:	//	ﾚｼｰﾄﾌﾟﾘﾝﾀBusyTimer	※ﾄﾘｶﾞｰは、Sci0Snd
				// BUSYはUARTコントローラーで制御するので、送信異常時の制御として使用する。
				// 待ち時間は同じとする（１０秒以上：約１１秒）
				if( rct_prn_buff.PrnBufCnt ){				// 送信処理中？
					rct_prn_buff.PrnBusyCnt++;				// ﾌﾟﾘﾝﾀﾋﾞｼﾞｰ継続監視ｶｳﾝﾀ更新
					if( (rct_prn_buff.PrnBusyCnt >= PRN_BUSY_TCNT) && (rct_proc_data.Printing != 0) ){

						//	印字処理中にﾌﾟﾘﾝﾀﾋﾞｼﾞｰが１０秒以上継続した場合
						err_chk( ERRMDL_PRINTER, ERR_PRNT_R_PRINTCOM, 1, 0, 0 );// ﾚｼｰﾄﾌﾟﾘﾝﾀ通信異常登録
						Inji_ErrEnd(						// 印字異常終了処理（異常終了ﾒｯｾｰｼﾞ送信）
										rct_proc_data.Printing,
										PRI_ERR_BUSY,
										R_PRI
									);
						JP_I2CSndReq( I2C_PRI_REQ );		/* ジャーナルに送信するデータがあれば送信する */
					}
					else{
						Lagtim( PRNTCBNO, 1, PRN_BUSY_TIMER );	// ５秒 wait
					}
				}
				break;

			case TIMEOUT2:	//	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀBusyTimer	※ﾄﾘｶﾞｰは、Sci3Snd
				// BUSYはUARTコントローラーで制御するので、送信異常時の制御として使用する。
				// 待ち時間は同じとする（１０秒以上：約１１秒）
				if( jnl_prn_buff.PrnBufCnt ){				// 送信処理中？
					jnl_prn_buff.PrnBusyCnt++;				// ﾌﾟﾘﾝﾀﾋﾞｼﾞｰ継続監視ｶｳﾝﾀ更新
					if( (jnl_prn_buff.PrnBusyCnt >= PRN_BUSY_TCNT) && (jnl_proc_data.Printing != 0) ){

						//	印字処理中にﾌﾟﾘﾝﾀﾋﾞｼﾞｰが１０秒以上継続した場合
// MH810104 GG119201(S) 電子ジャーナル対応
						if (isEJA_USE()) {
// MH810104 GG119201(S) 電子ジャーナル対応（通信不良検出中はリセットしない）
							Lagcan(PRNTCBNO, 10);
// MH810104 GG119201(E) 電子ジャーナル対応（通信不良検出中はリセットしない）
							err_chk(ERRMDL_EJA, ERR_EJA_COMFAIL, 1, 0, 0);	// 通信不良登録
						}
						else {
// MH810104 GG119201(E) 電子ジャーナル対応
						err_chk( ERRMDL_PRINTER, ERR_PRNT_J_PRINTCOM, 1, 0, 0 );// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ通信異常登録
// MH810104 GG119201(S) 電子ジャーナル対応
						}
// MH810104 GG119201(E) 電子ジャーナル対応
						Inji_ErrEnd(						// 印字異常終了処理（異常終了ﾒｯｾｰｼﾞ送信）
										jnl_proc_data.Printing,
										PRI_ERR_BUSY,
										J_PRI
									);
						RP_I2CSndReq( I2C_PRI_REQ );		/* レシートに送信するデータがあれば送信する */
					}
					else{
						Lagtim( PRNTCBNO, 2, PRN_BUSY_TIMER );	// ５秒 wait
					}
				}
				break;

			case TIMEOUT3:	// ﾚｼｰﾄﾍﾟｰﾊﾟｰﾆｱｴﾝﾄﾞTimer
				if( rct_prn_buff.PrnState[0] != rct_prn_buff.PrnStWork )
				{
					rct_prn_buff.PrnState[2] = rct_prn_buff.PrnState[1];
					rct_prn_buff.PrnState[1] = rct_prn_buff.PrnState[0];
					rct_prn_buff.PrnState[0] = rct_prn_buff.PrnStWork;
					if( (OutMsg = GetBuf()) != NULL )
					{
						OutMsg->msg.command = PREQ_JUSHIN_ERR1;	// ｺﾏﾝﾄﾞ	：受信エラー１
						OutMsg->msg.data[0] = R_PRI;			// data[0]	：ﾌﾟﾘﾝﾀ種別（ﾚｼｰﾄ）
						PutMsg( OPETCBNO, OutMsg );				// ｵﾍﾟﾚｰｼｮﾝﾀｽｸへﾒｯｾｰｼﾞ送信
					}
				}
				break;

			case TIMEOUT4:	// ｼﾞｬｰﾅﾙﾍﾟｰﾊﾟｰﾆｱｴﾝﾄﾞTimer
				if( jnl_prn_buff.PrnState[0] != jnl_prn_buff.PrnStWork )
				{
					jnl_prn_buff.PrnState[2] = jnl_prn_buff.PrnState[1];
					jnl_prn_buff.PrnState[1] = jnl_prn_buff.PrnState[0];
					jnl_prn_buff.PrnState[0] = jnl_prn_buff.PrnStWork;
					if( (OutMsg = GetBuf()) != NULL )
					{
						OutMsg->msg.command = PREQ_JUSHIN_ERR1;	// ｺﾏﾝﾄﾞ	：受信エラー１
						OutMsg->msg.data[0] = J_PRI;			// data[0]	：ﾌﾟﾘﾝﾀ種別（ｼﾞｬｰﾅﾙ）
						PutMsg( OPETCBNO, OutMsg );				// ｵﾍﾟﾚｰｼｮﾝﾀｽｸへﾒｯｾｰｼﾞ送信
					}
				}
				break;

			case TIMEOUT5:	// 印字ﾃﾞｰﾀ編集遅延ﾀｲﾏｰ（ﾚｼｰﾄ）

				if( rct_proc_data.EditWait == ON ){	// 編集遅延ﾀｲﾏｰ起動中？
					PrnNext( &NextMsg_r, R_PRI );	// 印字ﾃﾞｰﾀ次ﾌﾞﾛｯｸ編集要求ﾒｯｾｰｼﾞ送信（ﾚｼｰﾄ）
					rct_proc_data.EditWait = OFF;	// 印字ﾃﾞｰﾀ編集遅延ﾀｲﾏｰ起動状態ﾘｾｯﾄ
				}

				break;

			case TIMEOUT6:	// 印字ﾃﾞｰﾀ編集遅延ﾀｲﾏｰ（ｼﾞｬｰﾅﾙ）

				if( jnl_proc_data.EditWait == ON ){	// 編集遅延ﾀｲﾏｰ起動中？
					PrnNext( &NextMsg_j, J_PRI );	// 印字ﾃﾞｰﾀ次ﾌﾞﾛｯｸ編集要求ﾒｯｾｰｼﾞ送信（ｼﾞｬｰﾅﾙ）
					jnl_proc_data.EditWait = OFF;	// 印字ﾃﾞｰﾀ編集遅延ﾀｲﾏｰ起動状態ﾘｾｯﾄ
				}

				break;

			case TIMEOUT7:	// レシート印字完了後タイマー
				rct_timer_end = 0;					// このフラグがONの場合は、レシートプリンタ印字中とし、ジャーナルプリンタへの印字を抑止
				break;

// MH810104 GG119201(S) 電子ジャーナル対応
// MH810104 GG119201(S) 電子ジャーナル対応（データ書込み完了待ちタイマ変更）
//			case TIMEOUT8:		// データ書込み完了待ちタイマ
			case TIMEOUT10:		// データ書込み完了待ちタイマ
// MH810104 GG119201(E) 電子ジャーナル対応（データ書込み完了待ちタイマ変更）
// MH810104 GG119201(S) 電子ジャーナル対応（通信不良検出中はリセットしない）
				if (jnl_prn_buff.PrnBufCnt) {
					// データ送信中はタイマリセット
					Lagtim(PRNTCBNO, 10, PRN_WRITE_CMP_TIMER);
					break;
				}
				if (IsErrorOccuerd((char)ERRMDL_EJA, (char)ERR_EJA_COMFAIL)) {
					// 通信不良発生時はソフトリセットしない
					break;
				}
				// no break
// MH810104 GG119201(E) 電子ジャーナル対応（通信不良検出中はリセットしない）
			case PREQ_FS_ERR:	// ファイルシステム異常通知
				PrnCmd_EJAReset(0);
				break;

			case TIMEOUT9:		// 時刻／初期設定待ちタイムアウト
// MH810104 GG119201(S) 電子ジャーナル対応（リセット要求イベント追加）
			case PREQ_RESET:	// リセット要求
// MH810104 GG119201(E) 電子ジャーナル対応（リセット要求イベント追加）
				PrnCmd_EJAReset(1);
				break;
// MH810104 GG119201(E) 電子ジャーナル対応

			case	PREQ_SUICA_LOG:			// ＜Suica通信ログ＞印字要求

				PrnSuica_LOG( &msg.msg );
				break;
			case	PREQ_SUICA_LOG2:		// ＜Suica通信ログ直近データ＞印字要求

				PrnSuica_LOG2( &msg.msg );
				break;

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//			case	PREQ_EDY_STATUS:		// ＜Edy設定＆ステータス＞印字要求
//
//				PrnEdy_Status( &msg.msg );
//				break;
//
//			case	PREQ_EDY_ARM_R:			// ＜ｱﾗｰﾑﾚｼｰﾄ＞印字要求	
//
//				PrnEdyArmPri( &msg.msg );
//				break;
//
//			case	PREQ_EDY_ARMPAY_LOG:	// ＜アラーム取引情報LOG＞印字要求
//
//				PrnEDYARMPAY_LOG( &msg.msg );
//				break;
//
//			case	PREQ_EDY_SHIME_R:		// ＜Ｅｄｙ締め記録＞印字要求	
//
//				PrnEdyShimePri( &msg.msg );
//				break;
//
//			case	PREQ_EDY_SHIME_LOG:		// ＜Ｅｄｙ締め記録情報＞印字要求
//
//				PrnEDYSHIME_LOG( &msg.msg );
//				break;
//#endif
//			case	PREQ_EDY_USE_LOG:		// ＜Ｅｄｙ利用明細＞印字要求
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
			case	PREQ_SCA_USE_LOG:		// ＜Ｓｕｉｃａ利用明細＞印字要求
// MH321800(S) D.Inaba ICクレジット対応
			case	PREQ_EC_USE_LOG:		// ＜決済リーダ利用明細＞印字要求
// MH321800(E) D.Inaba ICクレジット対応
// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
			case	PREQ_EC_MINASHI_LOG:	// ＜みなし決済プリント＞印字要求
// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
				PrnDigi_USE( &msg.msg );	// カード毎利用明細印字処理
				break;

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//			case	PREQ_EDY_SYU_LOG:		// ＜Ｅｄｙ集計＞印字要求
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
			case	PREQ_SCA_SYU_LOG:		// ＜Ｓｕｉｃａ集計＞印字要求
// MH321800(S) D.Inaba ICクレジット対応
			case	PREQ_EC_SYU_LOG:		// ＜決済リーダ集計＞印字要求
// MH321800(E) D.Inaba ICクレジット対応
				PrnDigi_SYU( &msg.msg );
				break;
// MH321800(S) D.Inaba ICクレジット対応
			case	PREQ_EC_ALM_R:			// ＜ｱﾗｰﾑﾚｼｰﾄ＞印字要求	
				PrnEcAlmRctPri( &msg.msg );
				break;
			case	PREQ_EC_BRAND_COND:		// ＜決済リーダブランド状態＞印字要求
				PrnEcBrandPri( &msg.msg );
				break;
			case	PREQ_EC_ALARM_LOG:		// 処理未了取引記録印字要求
				PrnEcAlmRctLogPri( &msg.msg );
				break;
// MH810103 GG119202(S) ICクレジットみなし決済印字フォーマット変更対応
//			case	PREQ_EC_DEEMED_LOG:		// みなし決済復電ログ印字要求	
//				PrnEcDeemedJnlPri( &msg.msg );
//				break;
// MH810103 GG119202(E) ICクレジットみなし決済印字フォーマット変更対応
			case	PREQ_RECV_DEEMED:		// 決済精算中止(復決済)データ印字要求
				PrnRecvDeemedDataPri( &msg.msg );
				break;
// MH321800(E) D.Inaba ICクレジット対応				
			case	PREQ_ATEND_INV_DATA:	// ＜係員無効データ＞印字要求

				PrnAtendValidData( &msg.msg );
				break;
			case	PREQ_CHARGESETUP:		// ＜料金設定印字＞印字要求

				PrnCHARGESETUP( &msg.msg );
				break;
			case	SETDIFFLOG_PRINT:		// 設定変更履歴印字
				PrnSetDiffLogPrn ( &msg.msg );
				break;
			case	PREQ_RYOSETTEI_LOG:
				PrnRrokin_Setlog(&msg.msg);		/* 料金設定通信ログプリント */
				break;
			case	PREQ_CHK_PRINT:
				PrnSettei_Chk(&msg.msg);
				break;
			case PREQ_LOOP_DATA:				// フラップループデータ要求結果印字
				PrnFlapLoopData(&msg.msg);
				break;
			case PREQ_RT_PAYMENT_LOG:			// 料金テストログプリント
				RT_PrnSEISAN_LOG( &msg.msg );
				break;
			case PREQ_MNT_STACK:				// スタック使用量印字
				PrnMntStack( &msg.msg );
				break;
			case PREQ_CHKMODE_RESULT:
				PrnChkResult(&msg.msg);
				break;
// MH810105(S) R.Endo 2021/12/01 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
			case PREQ_DEBUG:					// デバッグ用印字要求
				PrnDEBUG(&msg.msg);
				break;
// MH810105(E) R.Endo 2021/12/01 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
// MH810105(S) MH364301 QRコード決済対応
			case	PREQ_RECV_FAILURECONTACT:	// 障害連絡票データ印字要求
				PrnFailureContactDataPri( &msg.msg );
				break;
// MH810105(E) MH364301 QRコード決済対応
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
			case	PREQ_STOCK_PRINT:
				PrnRYOUSYUU_StockPrint();
				break;
			case	PREQ_STOCK_CLEAR:
				PrnRYOUSYUU_StockClear();
				break;
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
			default:		// その他
				break;
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		処理対象ﾒｯｾｰｼﾞ取得													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	TgtMsGet( *msb )									|*/
/*|																			|*/
/*|	PARAMETER		:	MsgBuf	*msb	=	受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀｰ			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret＝取得結果								|*/
/*|							NG : 取得なし									|*/
/*|							OK : 取得あり									|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-09-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	TgtMsGet( MsgBuf *msb )
{
	uchar	ret;		// 戻り値
	uchar	tgtchk;		// ﾒｯｾｰｼﾞﾁｪｯｸ結果
	ushort	cmd1;	// 受信ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ
	ushort	cmd2;	// 受信ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ
	MsgBuf	*OutMsg;	// ﾒｯｾｰｼﾞ登録用ﾊﾞｯﾌｧﾎﾟｲﾝﾀｰ

	if( (rct_proc_data.Printing == 0) && (jnl_proc_data.Printing == 0) ){
		
		cmd1 = msb->msg.command;
		cmd2 = msb->msg.command & MSG_CMND_MASK;
		if( ( CMND_CHK2_SNO <= cmd2 ) && ( cmd2 <= CMND_CHK2_ENO ) ){
			if( !(cmd1 & INNJI_NEXTMASK) ){
				// 新規印字要求の場合
				if(msb->msg.data[0] == J_PRI && rct_timer_end) {
					goto Lexclsive;						// メッセージ再登録処理へ
				}
			}
		}
		else if(cmd2 == PREQ_INNJI_TYUUSHI){
			if(PriBothPrint == 2 && rct_timer_end){
				/* 両方印字指定でレシートへの印字を完了してジャーナルへの印字を待っている */
				msb->msg.data[0] = J_PRI;
				goto Lexclsive;						// メッセージ再登録処理へ
			}
			if(msb->msg.data[0] == J_PRI && rct_timer_end) {
				goto Lexclsive;						// メッセージ再登録処理へ
			}
		}
		// 印字処理中の次編集要求の場合
		ret = OK;
	}
	else{
		// 印字処理中の場合
Lexclsive:
		tgtchk = TgtMsgChk( msb );						// 印字処理中の処理対象ﾒｯｾｰｼﾞ判定

		if( tgtchk == OK ){								// 判定？

			// 処理対象ﾒｯｾｰｼﾞの場合

			if( msb->msg.command == PREQ_DUMMY ){

				// ﾀﾞﾐｰﾒｯｾｰｼﾞ（全登録ﾒｯｾｰｼﾞの検索終了）の場合

				ret = OK;								// 取得結果＝取得あり
			}
			else if(msb->msg.command == PREQ_INNJI_TYUUSHI){		
			/* 中止要求処理時に対象プリンタが印字中ではない場合、中止しようとしている印字要求がまだ処理されていない可能性がある */
			/* 以降のメッセージキューに対象プリンタの印字要求があれば、中止要求を再登録する */
				if( Cancel_repue_chk((T_FrmPrnStop*)msb->msg.data)){	
					/* キャンセルの再登録を行う */
					if( PriDummyMsg == OFF ){

						// ﾀﾞﾐｰﾒｯｾｰｼﾞ登録済みでない場合（全登録ﾒｯｾｰｼﾞの検索終了を判断する為、ﾀﾞﾐｰﾒｯｾｰｼﾞを登録する）

						if( (OutMsg = GetBuf()) != NULL ){		// ﾒｯｾｰｼﾞ送信ﾊﾞｯﾌｧ取得

							// ﾀﾞﾐｰﾒｯｾｰｼﾞ送信ﾊﾞｯﾌｧ取得ＯＫ

							OutMsg->msg.command = PREQ_DUMMY;
							PutMsg( PRNTCBNO, OutMsg );			// ﾀﾞﾐｰﾒｯｾｰｼﾞ登録
							PriDummyMsg = ON;					// ﾀﾞﾐｰﾒｯｾｰｼﾞ登録ﾌﾗｸﾞ：ＯＮ

							PutMsg( PRNTCBNO, msb );			// 受信ﾒｯｾｰｼﾞ再登録
						}
						else{
							// ﾀﾞﾐｰﾒｯｾｰｼﾞ送信ﾊﾞｯﾌｧ取得ＮＧ
							FreeBuf( msb );						// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ開放
						}
					}else{
						PutMsg( PRNTCBNO, msb );				// 受信ﾒｯｾｰｼﾞ再登録
					}
					ret = NG;
				}
				else{
					ret = OK;
				}
			}
			else{
				// ﾀﾞﾐｰﾒｯｾｰｼﾞ以外
				if( PriDummyMsg == OFF ){

					// ﾀﾞﾐｰﾒｯｾｰｼﾞ登録済みでない（受信ﾒｯｾｰｼﾞの再登録なし）場合

					ret = OK;							// 取得結果＝取得あり
				}
				else{
					// ﾀﾞﾐｰﾒｯｾｰｼﾞ登録済み（受信ﾒｯｾｰｼﾞの再登録あり）の場合

					if( TgtMsg == NULL ){

						// 処理ﾒｯｾｰｼﾞ保存がされていない場合（受信ﾒｯｾｰｼﾞを保存する）

						TgtMsg	= msb;					// 処理対象ﾒｯｾｰｼﾞ保存
						ret		= NG;					// 取得結果＝取得なし
					}
					else{
						// 処理ﾒｯｾｰｼﾞ保存が既にされている場合（受信ﾒｯｾｰｼﾞを保存できないので再登録する）

						PutMsg( PRNTCBNO, msb );		// 受信ﾒｯｾｰｼﾞ再登録
						ret = NG;						// 取得結果＝取得なし
					}
				}
			}
		}
		else{
			// 処理対象ﾒｯｾｰｼﾞでない場合（受信ﾒｯｾｰｼﾞを再登録する）
			if( PriDummyMsg == OFF ){

				// ﾀﾞﾐｰﾒｯｾｰｼﾞ登録済みでない場合（全登録ﾒｯｾｰｼﾞの検索終了を判断する為、ﾀﾞﾐｰﾒｯｾｰｼﾞを登録する）

				if( (OutMsg = GetBuf()) != NULL ){		// ﾒｯｾｰｼﾞ送信ﾊﾞｯﾌｧ取得

					// ﾀﾞﾐｰﾒｯｾｰｼﾞ送信ﾊﾞｯﾌｧ取得ＯＫ

					OutMsg->msg.command = PREQ_DUMMY;
					PutMsg( PRNTCBNO, OutMsg );			// ﾀﾞﾐｰﾒｯｾｰｼﾞ登録
					PriDummyMsg = ON;					// ﾀﾞﾐｰﾒｯｾｰｼﾞ登録ﾌﾗｸﾞ：ＯＮ

					PutMsg( PRNTCBNO, msb );			// 受信ﾒｯｾｰｼﾞ再登録
				}
				else{
					// ﾀﾞﾐｰﾒｯｾｰｼﾞ送信ﾊﾞｯﾌｧ取得ＮＧ
					FreeBuf( msb );						// 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧ開放
				}
			}else{
				PutMsg( PRNTCBNO, msb );				// 受信ﾒｯｾｰｼﾞ再登録
			}
			ret = NG;									// 取得結果＝取得なし
		}
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		印字処理中の処理対象ﾒｯｾｰｼﾞ判定										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	TgtMsgChk( *msb )									|*/
/*|																			|*/
/*|	PARAMETER		:	MsgBuf	*msb	=	受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀｰ			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret＝判定結果								|*/
/*|							OK : 処理対象ﾒｯｾｰｼﾞ								|*/
/*|							NG : 処理対象ﾒｯｾｰｼﾞ以外							|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-09-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	TgtMsgChk( MsgBuf *msb )
{
	uchar	ret;	// 戻り値
	ushort	cmd1;	// 受信ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ
	ushort	cmd2;	// 受信ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ

	cmd1 = msb->msg.command;
	cmd2 = msb->msg.command & MSG_CMND_MASK;

	if( ( CMND_CHK1_SNO <= cmd2 ) && ( cmd2 <= CMND_CHK1_ENO ) ){

		//	ﾌﾟﾘﾝﾀｰ制御ﾒｯｾｰｼﾞ(0x601～0x6ff)の場合

		if( ( CMND_CHK2_SNO <= cmd2 ) && ( cmd2 <= CMND_CHK2_ENO ) ){

			// 印字要求ﾒｯｾｰｼﾞの場合

			if( cmd1 & INNJI_NEXTMASK ){

				// 印字処理中の次編集要求の場合
				ret = OK;						// 判定結果：ＯＫ
			}
			else{
				// 新規印字要求の場合
				ret = NG;						// 判定結果：ＮＧ
			}
		}
		else{
			// 印字要求ﾒｯｾｰｼﾞ以外の場合
			ret = OK;							// 判定結果：ＯＫ
		}
	}
	else{
		// ﾌﾟﾘﾝﾀｰ制御ﾒｯｾｰｼﾞ以外の場合
		ret = OK;								// 判定結果：ＯＫ
	}
	return( ret );
}
/*[]-----------------------------------------------------------------------[]*/
/*|		印字キャンセル判定													|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Cancel_repue_chk( T_FrmPrnStop* data )				|*/
/*|	PARAMETER		:	MsgBuf	*msb	=	受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀｰ			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret＝判定結果								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	sugata														|*/
/*|	Date	:	2012-04-04													|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar Cancel_repue_chk(T_FrmPrnStop* data)
{
	uchar pri_kind = data->prn_kind;
	uchar	ret = 0;
	
	switch(pri_kind){
		case R_PRI:
			if(rct_proc_data.Printing == 0){
				ret = PreqMsgChk( R_PRI );
			}
			break;
		case J_PRI:
			if(jnl_proc_data.Printing == 0 || PriBothPrint == 2){
				ret = PreqMsgChk( J_PRI );
			}
			break;
		case RJ_PRI:
		default:
			break;
	}
	
	return ret;
}
/*[]-----------------------------------------------------------------------[]*/
/*|		印字要求判定														|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Cancel_repue_chk( T_FrmPrnStop* data )				|*/
/*|	PARAMETER		:	MsgBuf	*msb	=	受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀｰ			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret＝判定結果								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	sugata														|*/
/*|	Date	:	2012-04-04													|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
char	PreqMsgChk( char kind ){
	MsgBuf	*msg_add;										// ﾒｰﾙｷｭｰ内ﾁｪｯｸ対象ﾒｰﾙｱﾄﾞﾚｽｾｯﾄｴﾘｱ
	ushort	MailCommand;									// ﾒｰﾙｷｭｰ内ﾁｪｯｸ対象ﾒｰﾙのﾒｯｾｰｼﾞID
	ulong	ist;											// 現在の割込受付状態
	uchar	i;
	char	ret = 0;
	
	if( tcb[PRNTCBNO].msg_top == NULL ) {							// ﾒｰﾙなし
		return( NULL );
	}

	ist = _di2();											// 割込み禁止
	msg_add = tcb[PRNTCBNO].msg_top;								// ﾁｪｯｸ対象ﾒｰﾙｱﾄﾞﾚｽget（最初は先頭）

	// 検索処理
	for( i=0; i<MSGBUF_CNT; ++i ){							// ﾒｰﾙｷｭｰ内 全ﾒｰﾙﾁｪｯｸ（MSGBUF_CNTは単にLimitter）

		MailCommand = msg_add->msg.command;					// ﾁｪｯｸ対象ﾒｰﾙのﾒｯｾｰｼﾞIDget

		if( ( CMND_CHK2_SNO <= MailCommand ) && ( MailCommand <= CMND_CHK2_ENO ) ){	// 印字要求の場合
			if(msg_add->msg.data[0] == kind ){					// データの先頭が印字種別
				if(PriBothPrint == 2){
					if(MailCommand == BothPrintCommand){
						ret = 1;
						break;
					}
				}
				else{
					ret = 1;
					break;
				}
			}
		}

		// 次のﾒｰﾙへ（ﾁｪｯｸ準備）
		msg_add = (MsgBuf*)(msg_add->msg_next);				// 次ﾁｪｯｸ対象ﾒｰﾙｱﾄﾞﾚｽget
		if( NULL == msg_add ){								// 次ﾒｰﾙなし
			ret = 0;
			break;
		}

		// 16回に1回 WDTｸﾘｱ
		if( 0x0f == (i & 0x0f) ){
			WACDOG;
		}
	}
	_ei2( ist );
	return ret;
}

/*[]-----------------------------------------------------------------------[]*/
/*|		印字処理可／不可ﾁｪｯｸ												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	RcvCheckWait( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : 印字処理可									|*/
/*|							NG : 印字処理不可								|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	RcvCheckWait( MSG *msg )
{
	uchar	pri_sts;		// ﾌﾟﾘﾝﾀ状態
	uchar	ret = OK;		// 戻り値
	uchar	ret_rct = OK;	// 印字可／不可（ﾚｼｰﾄ）
	uchar	ret_jnl = OK;	// 印字可／不可（ｼﾞｬｰﾅﾙ）

	if( (rct_proc_data.Printing == 0) && (jnl_proc_data.Printing == 0) ){	// 印字処理中？

		//	印字処理中でない場合

		f_Prn_R_SendTopChar = 1;								// 1=ﾚｼｰﾄﾌﾟﾘﾝﾀ印字要求の先頭行文字送信前
		f_Prn_J_SendTopChar = 1;								// 1=ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ印字要求の先頭行文字送信前

		switch( terget_pri ){	// 処理対象ﾌﾟﾘﾝﾀ種別？

			case	R_PRI:		// ﾚｼｰﾄ

				if( msg->command == PREQ_AT_SYUUKEI ){			// 自動集計 印字要求？
					rct_atsyuk_pri = ON;						// 自動集計印字状態（ﾚｼｰﾄ用）ｾｯﾄ
				}

				/*------	ﾚｼｰﾄﾌﾟﾘﾝﾀ印字開始ﾁｪｯｸ	-----*/
				pri_sts = PriStsCheck( R_PRI );					// ﾚｼｰﾄﾌﾟﾘﾝﾀ状態ﾁｪｯｸ
				if( pri_sts != PRI_ERR_NON ){

					// 印字不可状態の場合
					Inji_ErrEnd( msg->command, pri_sts, R_PRI );// 印字異常終了処理（異常終了ﾒｯｾｰｼﾞ送信）
					ret = NG;
				}
				else{
					// 印字可能状態の場合
					prn_proc_data_clr_R();						// ﾚｼｰﾄ編集処理制御ﾃﾞｰﾀｸﾘｱ
					rct_proc_data.Printing = msg->command;		// 受信ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ保存

					if( YES == PrnGoukeiChk( msg->command ) ){	// 合計記録 印字要求？
						rct_goukei_pri = ON;					// 合計記録印字状態（ﾚｼｰﾄ用）ｾｯﾄ
						jnl_goukei_pri = OFF;					// 合計記録印字状態（ｼﾞｬｰﾅﾙ用）ﾘｾｯﾄ
					}
				}
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				if( msg->command == PREQ_AT_SYUUKEI ){			// 自動集計 印字要求？
					jnl_atsyuk_pri = ON;						// 自動集計印字状態（ｼﾞｬｰﾅﾙ用）ｾｯﾄ
				}

				/*------	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ印字開始ﾁｪｯｸ		-----*/
				pri_sts = PriStsCheck( J_PRI );					// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ状態ﾁｪｯｸ
				if( pri_sts != PRI_ERR_NON ){
// MH810104 GG119201(S) 電子ジャーナル対応
					if (pri_sts == PRI_ERR_WAIT_INIT) {
						PrnCmd_MsgResend(msg);
						ret = NG;
						break;
					}
// MH810104 GG119201(E) 電子ジャーナル対応

					// 印字不可状態の場合
					Inji_ErrEnd( msg->command, pri_sts, J_PRI );// 印字異常終了処理（異常終了ﾒｯｾｰｼﾞ送信）
					ret = NG;
				}
				else{
					// 印字可能状態の場合
					prn_proc_data_clr_J();						// ｼﾞｬｰﾅﾙ編集処理制御ﾃﾞｰﾀｸﾘｱ
					jnl_proc_data.Printing = msg->command;		// 受信ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ保存

					if( YES == PrnGoukeiChk( msg->command ) ){	// 合計記録 印字要求？
						if(PriBothPrint == 0) {
							rct_goukei_pri = OFF;				// 合計記録印字状態（ﾚｼｰﾄ用）ﾘｾｯﾄ
						}
						jnl_goukei_pri = ON;					// 合計記録印字状態（ｼﾞｬｰﾅﾙ用）ｾｯﾄ
					}
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
					// 印字要求受信時にデータ書込み開始を通知する
					PrnCmd_WriteStartEnd(0, msg);
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
				}
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				if( msg->command == PREQ_AT_SYUUKEI ){			// 自動集計 印字要求？

					// 自動集計印字要求の場合
					rct_atsyuk_pri = ON;						// 自動集計印字状態（ﾚｼｰﾄ用）ｾｯﾄ
					jnl_atsyuk_pri = ON;						// 自動集計印字状態（ｼﾞｬｰﾅﾙ用）ｾｯﾄ
				}
				/*------	ﾚｼｰﾄﾌﾟﾘﾝﾀ印字開始ﾁｪｯｸ	-----*/

				pri_sts = PriStsCheck( R_PRI );					// ﾚｼｰﾄﾌﾟﾘﾝﾀ状態ﾁｪｯｸ
				if( pri_sts != PRI_ERR_NON ){

					// 印字不可状態の場合
					Inji_ErrEnd( msg->command, pri_sts, R_PRI );// 印字異常終了処理（異常終了ﾒｯｾｰｼﾞ送信）
					ret_rct = NG;
				}
				else{
					// 印字可能状態の場合
					prn_proc_data_clr_R();						// ﾚｼｰﾄ編集処理制御ﾃﾞｰﾀｸﾘｱ
					rct_proc_data.Printing = msg->command;		// 受信ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ保存
					if( msg->command == PREQ_AT_SYUUKEI ){		// 自動集計 印字要求？
						rct_goukei_pri = ON;					// 合計記録印字状態（ﾚｼｰﾄ用）ｾｯﾄ
					}
				}

				/*------	ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ印字開始ﾁｪｯｸ		-----*/
				pri_sts = PriStsCheck( J_PRI );					// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ状態ﾁｪｯｸ
				if( pri_sts != PRI_ERR_NON ){

					// 印字不可状態の場合
					Inji_ErrEnd( msg->command, pri_sts, J_PRI );// 印字異常終了処理（異常終了ﾒｯｾｰｼﾞ送信）
					ret_jnl = NG;
				}
				else{
					// 印字可能状態の場合
					prn_proc_data_clr_J();						// ｼﾞｬｰﾅﾙ編集処理制御ﾃﾞｰﾀｸﾘｱ
					// ここではjnl_proc_data.Printingは設定せず
					if(	rct_proc_data.Printing == 0) {
						jnl_proc_data.Printing = msg->command;	// 受信ﾒｯｾｰｼﾞｺﾏﾝﾄﾞ保存
					}
					// 別のメッセージにしたジャーナル印字時にセットする
					if( msg->command == PREQ_AT_SYUUKEI ){		// 自動集計 印字要求？
						jnl_goukei_pri = ON;					// 合計記録印字状態（ﾚｼｰﾄ用）ｾｯﾄ
					}
// GG129001(S) 電子ジャーナル対応（書込み開始コマンドが2回送信される）
//// MH364301(S) 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
//					// 印字要求受信時にデータ書込み開始を通知する
//					PrnCmd_WriteStartEnd(0, msg);
//// MH364301(E) 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
// GG129001(E) 電子ジャーナル対応（書込み開始コマンドが2回送信される）
				}

				if( YES == PrnGoukeiChk( msg->command ) ){		// 合計記録 印字要求？

					//	合計記録の場合、ﾚｼｰﾄ、ｼﾞｬｰﾅﾙともに印字可能な場合のみ印字を開始する（一方が印字不可の場合印字しない）
					//	（印字が正常終了したﾀｲﾐﾝｸﾞで集計ﾃﾞｰﾀｸﾘｱを行う為、両方のﾌﾟﾘﾝﾀが印字可能な場合のみ印字開始とする）
					if( ret_rct == OK && ret_jnl == OK ){

						// ﾚｼｰﾄ、ｼﾞｬｰﾅﾙともに印字可能の場合
						rct_goukei_pri = ON;					// 合計記録印字状態（ﾚｼｰﾄ用）ｾｯﾄ
						jnl_goukei_pri = ON;					// 合計記録印字状態（ｼﾞｬｰﾅﾙ用）ｾｯﾄ
					}
					else{
						// ﾚｼｰﾄ、ｼﾞｬｰﾅﾙどちらか一方でも印字不可の場合
							rct_proc_data.Printing = 0;
							jnl_proc_data.Printing = 0;
							rct_goukei_pri = OFF;				// 合計記録印字状態（ﾚｼｰﾄ用）ﾘｾｯﾄ
							jnl_goukei_pri = OFF;				// 合計記録印字状態（ｼﾞｬｰﾅﾙ用）ﾘｾｯﾄ
							ret = NG;
					}
				}
				else{

					//	合計記録印字以外の場合、印字可能なﾌﾟﾘﾝﾀへ印字を行う

					if( ret_rct == NG && ret_jnl == NG ){

						// ﾚｼｰﾄ、ｼﾞｬｰﾅﾙともに印字不可の場合
						ret = NG;
					}
					else if( ret_rct == OK && ret_jnl == NG ){

						// ﾚｼｰﾄのみ印字可の場合
						terget_pri = R_PRI;						// 処理対象をﾚｼｰﾄﾌﾟﾘﾝﾀのみとする
					}
					else if( ret_rct == NG && ret_jnl == OK ){

						// ｼﾞｬｰﾅﾙのみ印字可の場合
						terget_pri = J_PRI;						// 処理対象をｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀのみとする
					}
				}

				break;

			default:
				ret = NG;		// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
				break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
		}
	}
	else{
		//	印字処理中の場合

		if( next_prn_msg == ON ){					// 印字処理中の次編集要求？

			//	印字処理中の次編集要求

			switch( terget_pri ){
				case	R_PRI:						// ﾚｼｰﾄ
					ret = PriOutCheck(R_PRI);		// ﾌﾟﾘﾝﾀ出力可能ﾁｪｯｸ(ﾚｼｰﾄ)
					break;
				case	J_PRI:						// ｼﾞｬｰﾅﾙ
					ret = PriOutCheck(J_PRI);		// ﾌﾟﾘﾝﾀ出力可能ﾁｪｯｸ（ｼﾞｬｰﾅﾙ）
					break;
				default:							// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
					ret = NG;
// MH322914 (s) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
					break;
// MH322914 (e) kasiyama 2016/07/08 [break]を入れる(共通改善No.896)(MH341106)
			}
			if( ret == OK){							// ﾌﾟﾘﾝﾀﾁｪｯｸＯＫ？
				msg->command &= MSG_CMND_MASK;		// ﾏｽｸﾌﾗｸﾞをﾘｾｯﾄしｺﾏﾝﾄﾞ部分のみとする
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
				if (terget_pri == J_PRI) {
					if (jnl_proc_data.Split == 0) {
						// 復電後の再印字開始時にデータ書込み開始を通知する
						PrnCmd_WriteStartEnd(0, msg);
					}
					else if (jnl_proc_data.Final != 0) {
						// 最終ブロック印字完了して要る場合は印字処理しない
						ret = NG;
					}
				}
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
			}
		}
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾚｼｰﾄ印字処理可／不可ﾁｪｯｸ											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PriRctCheck( )										|*/
/*|																			|*/
/*|	PARAMETER		:	NON													|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : 印字処理可									|*/
/*|							NG : 印字処理不可								|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PriRctCheck( void )
{
	uchar	ret = OK;		// 戻り値

	if( ((rct_proc_data.Printing == 0) && (PriStsCheck(R_PRI) != PRI_ERR_NON)) ||
		((rct_proc_data.Printing != 0) && (PriOutCheck(R_PRI) != OK)) ) {
	//  (印字処理中でない              && 印字不可)                            ||
	//  (印字処理中                    && 編集開始NG)
		ret = NG;
	}

	return( ret );
}


/*----------------------------------------------------------------------------------------------*/
/*		受信ﾒｯｾｰｼﾞ対応処理																		*/
/*----------------------------------------------------------------------------------------------*/

/*[]-----------------------------------------------------------------------[]*/
/*|		領収証印字要求ﾒｯｾｰｼﾞ受信処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnRYOUSYUU( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-25													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnRYOUSYUU( MSG *msg )
{
	T_FrmReceipt	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	Receipt_data	*Ryousyuu;		// 領収証印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	date_time_rec	*PriTime;		// 再ﾌﾟﾘﾝﾄ日時ﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmReceipt *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	Ryousyuu = msg_data->prn_data;			// 領収証印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	PriTime = &msg_data->PriTime;			// 再ﾌﾟﾘﾝﾄ日時ﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	if( next_prn_msg == OFF ){				// 新規の領収証印字要求？

		//	新規の領収証印字要求の場合
		PrnRYOUSYUU_datachk( Ryousyuu );	// 領収証ﾃﾞｰﾀﾁｪｯｸ
		if( msg_data->reprint == OFF ){
			// 通常印字要求
			memset( &Repri_Time, 0, sizeof(Repri_Time) );							// 再発行日時０ｸﾘｱ
			Repri_kakari_no = 0;													// 再ﾌﾟﾘﾝﾄ係員No. 0ｸﾘｱ
		}
		else{
			// 再発行印字要求
			memcpy( (char *)&Repri_Time, (char *)PriTime, sizeof( Repri_Time ) );	// 再発行日時ﾃﾞｰﾀｾｯﾄ
			Repri_kakari_no = msg_data->kakari_no;									// 再ﾌﾟﾘﾝﾄ係員No.ｾｯﾄ
		}
		if( Ryousyuu->testflag == ON ){
			// テスト印字要求
			memcpy( (char *)&Repri_Time, (char *)PriTime, sizeof( Repri_Time ) );	// テスト発行日時ﾃﾞｰﾀｾｯﾄ
		}
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
				if( isNewModel_R() && next_prn_msg == OFF ){
					// 新レシートプリンタの場合は領収証印字を溜め印字とする
					if( !IS_INVOICE && Ryousyuu->WFusoku && prm_get(COM_PRM, S_SYS, 44, 1, 1) == 0 ){
						// 非インボイス預り証単独発行時のみ印字完了まで待機する(200ms)
						// レシートプリンタ起動中は溜め印字不可の為
						xPause_PRNTSK(20);
					}

					// データ書込み開始
					PCMD_WRITE_START(R_PRI);
				}
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
				RYOUSYUU_edit( Ryousyuu, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 領収証印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				RYOUSYUU_edit( Ryousyuu, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 領収証印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				RYOUSYUU_edit( Ryousyuu, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 領収証印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				RYOUSYUU_edit( Ryousyuu, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 領収証印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}
// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
		// レシートプリンタの領収証編集処理終了
		if( rct_proc_data.Final == ON && terget_pri == R_PRI ){
			if( isNewModel_R() ){													// 新プリンタ
				if( msg_data->dummy != 1 ){											// ストック対象データではない
					// ジャーナル印字完了を待たないデータのため
					// 印字開始（書き込み終了）を送信する
					MsgSndFrmPrn(PREQ_STOCK_PRINT, R_PRI, 0);
				}
				else{																// ストック対象データ
					// ジャーナル印字完了を待つデータのため
					// 印字開始（書き込み終了）はまだ送信しない
					// RP_I2CSndReq( I2C_EVENT_QUE_REQ )を実施したいためプリンタ起動処理を行う
					PrnOut( R_PRI );
				}
				// 新プリンタでの終了処理（End_Set）はここではしない
				return;
			}
		}
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
		End_Set( msg, terget_pri );		// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		預り証印字要求ﾒｯｾｰｼﾞ受信処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnAZUKARI( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-11													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnAZUKARI( MSG *msg )
{
	T_FrmAzukari	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	Azukari_data	*Azukari;		// 預り証印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmAzukari *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	Azukari = msg_data->prn_data;			// 預り証印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ


	PrnAZUKARI_datachk( Azukari );			// 預り証ﾃﾞｰﾀﾁｪｯｸ

	switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ

			AZUKARI_edit( Azukari, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 預り証印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ

			AZUKARI_edit( Azukari, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 預り証印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

			AZUKARI_edit( Azukari, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 預り証印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			AZUKARI_edit( Azukari, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 預り証印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}

	End_Set( msg, terget_pri );		// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
}

/*[]-----------------------------------------------------------------------[]*/
/*|		受付券印字要求ﾒｯｾｰｼﾞ受信処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnAZUKARI( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-11													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnUKETUKE( MSG *msg )
{
	T_FrmUketuke	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	Uketuke_data	*Uketuke;		// 受付券印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	date_time_rec	*PriTime;		// 再ﾌﾟﾘﾝﾄ日時ﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmUketuke *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	Uketuke = msg_data->prn_data;			// 受付券印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	PriTime = &msg_data->PriTime;			// 再ﾌﾟﾘﾝﾄ日時ﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	PrnUKETUKE_datachk( Uketuke );			// 受付券ﾃﾞｰﾀﾁｪｯｸ

	if( msg_data->reprint == OFF ){
		// 通常印字要求
		memset( &Repri_Time, 0, sizeof(Repri_Time) );								// 再発行日時０ｸﾘｱ
		Repri_kakari_no = 0;														// 再ﾌﾟﾘﾝﾄ係員No. 0ｸﾘｱ
	}
	else if( msg_data->reprint == 2 ){
		memset( &Repri_Time, 0, sizeof(Repri_Time) );								// 再発行日時０ｸﾘｱ
		Repri_kakari_no = msg_data->kakari_no;										// 再ﾌﾟﾘﾝﾄ係員No.ｾｯﾄ
	}
	else{
		// 再発行印字要求
		memcpy( (char *)&Repri_Time, (char *)PriTime, sizeof( Repri_Time ) );		// 再発行日時ﾃﾞｰﾀｾｯﾄ
		Repri_kakari_no = msg_data->kakari_no;										// 再ﾌﾟﾘﾝﾄ係員No.ｾｯﾄ
	}

	switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ

			UKETUKE_edit( Uketuke, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 受付券印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ

			UKETUKE_edit( Uketuke, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 受付券印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

			UKETUKE_edit( Uketuke, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 受付券印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			UKETUKE_edit( Uketuke, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 受付券印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}

	End_Set( msg, terget_pri );		// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
}

/*[]-----------------------------------------------------------------------[]*/
/*|		集計印字要求ﾒｯｾｰｼﾞ受信処理											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSYUUKEI( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnSYUUKEI( MSG *msg )
{
	T_FrmSyuukei	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	SYUKEI			*Syuukei;		// 集計印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	date_time_rec	*PriTime;		// 再ﾌﾟﾘﾝﾄ日時ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
// MH810105 GG119202(S) T合計連動印字対応
	T_FrmSyuukeiEc	*msg_ec;
// MH810105 GG119202(E) T合計連動印字対応


	msg_data = (T_FrmSyuukei *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	Syuukei = msg_data->prn_data;			// 集計印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	PriTime = &msg_data->PriTime;			// 再ﾌﾟﾘﾝﾄ日時ﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	if( next_prn_msg == OFF ){				// 新規の集計印字要求？

		//	新規の集計印字要求の場合
		PrnSYUUKEI_datachk( Syuukei );											// 集計ﾃﾞｰﾀﾁｪｯｸ
		memcpy( (char *)&Repri_Time, (char *)PriTime, sizeof( Repri_Time ) );	// 再ﾌﾟﾘﾝﾄ日時ﾃﾞｰﾀｾｯﾄ
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

// MH810105 GG119202(S) T合計連動印字対応
		if (msg_data->print_flag != 0) {
			// T合計連動印字
			msg_ec = (T_FrmSyuukeiEc *)msg->data;
			switch (terget_pri) {
			case	R_PRI:
				SYUUKEI_EC_edit( Syuukei, R_PRI, msg_data->print_flag, &msg_ec->Ec, (PRN_PROC_DATA *)&rct_proc_data );	// 集計印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;
			case	J_PRI:
				SYUUKEI_edit( Syuukei, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 集計印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;
			default:
				return;
			}
		}
		else {
// MH810105 GG119202(E) T合計連動印字対応
		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				SYUUKEI_edit( Syuukei, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 集計印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				SYUUKEI_edit( Syuukei, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 集計印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;


			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}
// MH810105 GG119202(S) T合計連動印字対応
		}
// MH810105 GG119202(E) T合計連動印字対応
		End_Set( msg, terget_pri );		// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		集計履歴印字要求ﾒｯｾｰｼﾞ受信処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSYUUKEI_LOG( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-16													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnSYUUKEI_LOG( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmLogPriReq2 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				SYUUKEILOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 集計履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				SYUUKEILOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 集計履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				SYUUKEILOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 集計履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				SYUUKEILOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 集計履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}

		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ｺｲﾝ金庫集計印字要求ﾒｯｾｰｼﾞ受信処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnCOKI_SK( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-25													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnCOKI_SK( MSG *msg )
{
	T_FrmCoSyuukei	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	COIN_SYU		*Coinkinko;		// ｺｲﾝ金庫集計ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	date_time_rec	*PriTime;		// 再ﾌﾟﾘﾝﾄ日時ﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data	= (T_FrmCoSyuukei *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	Coinkinko	= msg_data->prn_data;			// ｺｲﾝ金庫集計ﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	PriTime		= &msg_data->PriTime;			// 再ﾌﾟﾘﾝﾄ日時ﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	PrnCOKI_SK_datachk( Coinkinko );												// ｺｲﾝ金庫集計ﾃﾞｰﾀﾁｪｯｸ
	memcpy( (char *)&Repri_Time, (char *)PriTime, sizeof( Repri_Time ) );			// 再ﾌﾟﾘﾝﾄ日時ﾃﾞｰﾀｾｯﾄ

	switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ

			CoKiSk_edit( Coinkinko, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｺｲﾝ金庫集計印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ

			CoKiSk_edit( Coinkinko, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｺｲﾝ金庫集計印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

			CoKiSk_edit( Coinkinko, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｺｲﾝ金庫集計印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			CoKiSk_edit( Coinkinko, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｺｲﾝ金庫集計印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}

	End_Set( msg, terget_pri );		// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ｺｲﾝ金庫集計情報印字要求ﾒｯｾｰｼﾞ受信処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnCOKI_JO( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-09													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnCOKI_JO( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmLogPriReq2 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				CoKiJo_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｺｲﾝ金庫集計情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				CoKiJo_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｺｲﾝ金庫集計情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				CoKiJo_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｺｲﾝ金庫集計情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				CoKiJo_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｺｲﾝ金庫集計情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}

		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		紙幣金庫集計印字要求ﾒｯｾｰｼﾞ受信処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSIKI_SK( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-25													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnSIKI_SK( MSG *msg )
{
	T_FrmSiSyuukei	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	NOTE_SYU		*Siheikinko;	// 紙幣金庫集計ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	date_time_rec	*PriTime;		// 再ﾌﾟﾘﾝﾄ日時ﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data	= (T_FrmSiSyuukei *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	Siheikinko	= msg_data->prn_data;			// 紙幣金庫集計ﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	PriTime		= &msg_data->PriTime;			// 再ﾌﾟﾘﾝﾄ日時ﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	PrnSIKI_SK_datachk( Siheikinko );											// 紙幣金庫集計ﾃﾞｰﾀﾁｪｯｸ
	memcpy( (char *)&Repri_Time, (char *)PriTime, sizeof( Repri_Time ) );		// 再ﾌﾟﾘﾝﾄ日時ﾃﾞｰﾀｾｯﾄ

	switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ

			SiKiSk_edit( Siheikinko, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 紙幣金庫集計印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ

			SiKiSk_edit( Siheikinko, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 紙幣金庫集計印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

			SiKiSk_edit( Siheikinko, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 紙幣金庫集計印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			SiKiSk_edit( Siheikinko, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 紙幣金庫集計印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}

	End_Set( msg, terget_pri );		// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
}

/*[]-----------------------------------------------------------------------[]*/
/*|		紙幣金庫集計情報印字要求ﾒｯｾｰｼﾞ受信処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSIKI_JO( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-10													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnSIKI_JO( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmLogPriReq2 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				SiKiJo_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 紙幣金庫集計情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				SiKiJo_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 紙幣金庫集計情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				SiKiJo_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 紙幣金庫集計情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				SiKiJo_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 紙幣金庫集計情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}

		End_Set( msg, terget_pri );				// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理

	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		釣銭管理集計印字要求ﾒｯｾｰｼﾞ受信処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTURIKAN( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-01													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnTURIKAN( MSG *msg )
{
	T_FrmTuriKan	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	TURI_KAN		*TuriKan;		// 釣銭管理集計印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmTuriKan *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	TuriKan = msg_data->prn_data;			// 釣銭管理集計印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( next_prn_msg == OFF ){				// 新規の釣銭管理集計印字要求？

		//	新規の集計印字要求の場合
		PrnTURIKAN_datachk( TuriKan );											// 釣銭管理集計ﾃﾞｰﾀﾁｪｯｸ
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				TURIKAN_edit( TuriKan, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 釣銭管理集計印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				TURIKAN_edit( TuriKan, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 釣銭管理集計印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				TURIKAN_edit( TuriKan, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 釣銭管理集計印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				TURIKAN_edit( TuriKan, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 釣銭管理集計印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}
		End_Set( msg, terget_pri );		// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		釣銭管理集計履歴印字要求ﾒｯｾｰｼﾞ受信処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTURIKAN_LOG( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnTURIKAN_LOG( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmLogPriReq2 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				TURIKANLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 釣銭管理集計履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				TURIKANLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 釣銭管理集計履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				TURIKANLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 釣銭管理集計履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				TURIKANLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 釣銭管理集計履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}

		End_Set( msg, terget_pri );				// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理

	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		エラー情報印字要求ﾒｯｾｰｼﾞ受信処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnERR_JOU( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-10													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnERR_JOU( MSG *msg )
{
	T_FrmErrJou		*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	Err_log			*Errlog;		// ｴﾗｰ情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data	= (T_FrmErrJou *)msg->data;			// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	Errlog		= msg_data->prn_data;				// ｴﾗｰ情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	PrnERRJOU_datachk( Errlog );					// ｴﾗｰ情報ﾃﾞｰﾀﾁｪｯｸ

	switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ

			ERRJOU_edit( Errlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ｴﾗｰ情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ

			ERRJOU_edit( Errlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ｴﾗｰ情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

			ERRJOU_edit( Errlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ｴﾗｰ情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			ERRJOU_edit( Errlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ｴﾗｰ情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}

	End_Set( msg, terget_pri );												// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
}

/*[]-----------------------------------------------------------------------[]*/
/*|		エラー情報履歴印字要求ﾒｯｾｰｼﾞ受信処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnERR_LOG( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-10													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnERR_LOG( MSG *msg )
{
	T_FrmLogPriReq2		*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmLogPriReq2 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( next_prn_msg == OFF ){					// 新規の印字要求？

		//	新規の印字要求の場合
		PrnERRLOG_datachk( msg_data );			// ｴﾗｰ情報履歴印字要求ﾃﾞｰﾀﾁｪｯｸ
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				ERRLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				ERRLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				ERRLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				ERRLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}

		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		アラーム情報印字要求ﾒｯｾｰｼﾞ受信処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnARM_JOU( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-13													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnARM_JOU( MSG *msg )
{
	T_FrmArmJou		*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	Arm_log			*Armlog;		// ｱﾗｰﾑ情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data	= (T_FrmArmJou *)msg->data;			// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	Armlog		= msg_data->prn_data;				// ｱﾗｰﾑ情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	PrnARMJOU_datachk( Armlog );					// ｱﾗｰﾑ情報ﾃﾞｰﾀﾁｪｯｸ

	switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ

			ARMJOU_edit( Armlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ｱﾗｰﾑ情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ

			ARMJOU_edit( Armlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ｱﾗｰﾑ情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

			ARMJOU_edit( Armlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ｱﾗｰﾑ情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			ARMJOU_edit( Armlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ｱﾗｰﾑ情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}

	End_Set( msg, terget_pri );												// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
}

/*[]-----------------------------------------------------------------------[]*/
/*|		アラーム情報履歴印字要求ﾒｯｾｰｼﾞ受信処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnARM_LOG( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-13													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnARM_LOG( MSG *msg )
{
	T_FrmLogPriReq2		*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data = (T_FrmLogPriReq2 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( next_prn_msg == OFF ){					// 新規の印字要求？

		//	新規の印字要求の場合
		PrnARMLOG_datachk( msg_data );			// ｱﾗｰﾑ情報履歴印字要求ﾃﾞｰﾀﾁｪｯｸ
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				ARMLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｱﾗｰﾑ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				ARMLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｱﾗｰﾑ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				ARMLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｱﾗｰﾑ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				ARMLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｱﾗｰﾑ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}

		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		操作情報印字要求ﾒｯｾｰｼﾞ受信処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnOPE_JOU( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-13													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnOPE_JOU( MSG *msg )
{
	T_FrmOpeJou		*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	Ope_log			*Opelog;		// 操作情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data	= (T_FrmOpeJou *)msg->data;			// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	Opelog		= msg_data->prn_data;				// 操作情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	PrnOPEJOU_datachk( Opelog );					// 操作情報ﾃﾞｰﾀﾁｪｯｸ

	switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ

			OPEJOU_edit( Opelog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 操作情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ

			OPEJOU_edit( Opelog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 操作情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

			OPEJOU_edit( Opelog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 操作情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			OPEJOU_edit( Opelog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 操作情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}

	End_Set( msg, terget_pri );												// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
}

/*[]-----------------------------------------------------------------------[]*/
/*|		操作情報履歴印字要求ﾒｯｾｰｼﾞ受信処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnOPE_LOG( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-13													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnOPE_LOG( MSG *msg )
{
	T_FrmLogPriReq2		*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data = (T_FrmLogPriReq2 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( next_prn_msg == OFF ){					// 新規の印字要求？

		//	新規の印字要求の場合
		PrnOPELOG_datachk( msg_data );			// 操作情報履歴印字要求ﾃﾞｰﾀﾁｪｯｸ
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				OPELOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 操作情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				OPELOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 操作情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				OPELOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 操作情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				OPELOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 操作情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}

		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		モニタ情報印字要求ﾒｯｾｰｼﾞ受信処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnMON_JOU( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	machida kei													|*/
/*|	Date	:	2005-11-30													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnMON_JOU( MSG *msg )
{
	T_FrmMonJou		*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	Mon_log			*Monlog;		// ﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data	= (T_FrmMonJou *)msg->data;			// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	Monlog		= msg_data->prn_data;				// ﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	PrnMONJOU_datachk( Monlog );					// ﾃﾞｰﾀﾁｪｯｸ

	switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ

			MONJOU_edit( Monlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ

			MONJOU_edit( Monlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

			MONJOU_edit( Monlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			MONJOU_edit( Monlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}

	End_Set( msg, terget_pri );												// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
}

/*[]-----------------------------------------------------------------------[]*/
/*|		モニタ情報履歴印字要求ﾒｯｾｰｼﾞ受信処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnMON_LOG( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	R.HARA														|*/
/*|	Date	:	2006-02-28													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
void	PrnMON_LOG( MSG *msg )
{
	T_FrmLogPriReq2		*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data = (T_FrmLogPriReq2 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( next_prn_msg == OFF ){					// 新規の印字要求？

		//	新規の印字要求の場合
		PrnMONLOG_datachk( msg_data );			// モニタ情報履歴印字要求ﾃﾞｰﾀﾁｪｯｸ
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				MONLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 操作情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				MONLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 操作情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				MONLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 操作情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				MONLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 操作情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}

		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}


/*[]-----------------------------------------------------------------------[]*/
/*|		不正ログ印字要求ﾒｯｾｰｼﾞ受信処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnNG_LOG( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	M.Suzuki													|*/
/*|	Date	:	2006-02-20													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
void	PrnNG_LOG( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data = (T_FrmLogPriReq2 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？
		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{
		NgLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 不正券ログ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}
/*[]-----------------------------------------------------------------------[]*/
/*|		入出庫ログ印字要求ﾒｯｾｰｼﾞ受信処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnIO_LOG( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	M.Suzuki													|*/
/*|	Date	:	2006-03-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
void	PrnIO_LOG( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data = (T_FrmLogPriReq2 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？
		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{
		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				IoLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 不正券ログ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				IoLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 不正券ログ印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				IoLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 不正券ログ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				IoLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 不正券ログ印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}
		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}


/*[]-----------------------------------------------------------------------[]*/
/*|		設定ﾃﾞｰﾀ印字要求ﾒｯｾｰｼﾞ受信処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSETTEI( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-13													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnSETTEI( MSG *msg )
{
	T_FrmSetteiData	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmSetteiData *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ


	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				SETTEI_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 設定ﾃﾞｰﾀ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				SETTEI_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 設定ﾃﾞｰﾀ印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				SETTEI_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 設定ﾃﾞｰﾀ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				SETTEI_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 設定ﾃﾞｰﾀ印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}
		End_Set( msg, terget_pri );															// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		動作ｶｳﾝﾄ印字要求ﾒｯｾｰｼﾞ受信処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnDOUSAC( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-25													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnDOUSAC( MSG *msg )
{
	T_FrmDousaCnt	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmDousaCnt *)msg->data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ

			DOUSAC_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 動作ｶｳﾝﾄ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ

			DOUSAC_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 動作ｶｳﾝﾄ印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

			DOUSAC_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 動作ｶｳﾝﾄ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			DOUSAC_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 動作ｶｳﾝﾄ印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}

	End_Set( msg, terget_pri );															// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾛｯｸ装置動作ｶｳﾝﾄ印字要求ﾒｯｾｰｼﾞ受信処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnLOCK_DCNT( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-07-20													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnLOCK_DCNT( MSG *msg )
{
	T_FrmLockDcnt	*msg_data;							// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmLockDcnt *)msg->data;				// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){			// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );					// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				LOCKDCNT_edit( msg_data->Kikai_no, msg_data->Req_syu, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ﾛｯｸ装置動作ｶｳﾝﾄ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				LOCKDCNT_edit( msg_data->Kikai_no, msg_data->Req_syu, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ﾛｯｸ装置動作ｶｳﾝﾄ印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				LOCKDCNT_edit( msg_data->Kikai_no, msg_data->Req_syu, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ﾛｯｸ装置動作ｶｳﾝﾄ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				LOCKDCNT_edit( msg_data->Kikai_no, msg_data->Req_syu, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ﾛｯｸ装置動作ｶｳﾝﾄ印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}
		End_Set( msg, terget_pri );																// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		車室ﾊﾟﾗﾒｰﾀ印字要求ﾒｯｾｰｼﾞ受信処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnLOCK_PARA( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-30													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnLOCK_PARA( MSG *msg )
{
	T_FrmLockPara	*msg_data;							// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmLockPara *)msg->data;				// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){			// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );					// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				LOCKPARA_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 車室ﾊﾟﾗﾒｰﾀ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				LOCKPARA_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 車室ﾊﾟﾗﾒｰﾀ印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				LOCKPARA_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 車室ﾊﾟﾗﾒｰﾀ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				LOCKPARA_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 車室ﾊﾟﾗﾒｰﾀ印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}
		End_Set( msg, terget_pri );																// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾛｯｸ装置設定印字要求ﾒｯｾｰｼﾞ受信処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnLOCK_SETTEI( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-30													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnLOCK_SETTEI( MSG *msg )
{
	T_FrmLockSettei	*msg_data;							// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmLockSettei *)msg->data;			// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){			// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );					// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				LOCKSETTEI_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ﾛｯｸ装置設定印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				LOCKSETTEI_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ﾛｯｸ装置設定印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				LOCKSETTEI_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ﾛｯｸ装置設定印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				LOCKSETTEI_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ﾛｯｸ装置設定印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}
		End_Set( msg, terget_pri );																// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		車室情報（現在）印字要求ﾒｯｾｰｼﾞ受信処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnPKJOU_NOW( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-11													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnPKJOU_NOW( MSG *msg )
{
	T_FrmPkjouNow	*msg_data;							// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmPkjouNow *)msg->data;				// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){			// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );					// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ
				PKJOUNOW_edit( msg_data->Kikai_no, R_PRI, msg_data->prn_menu, (PRN_PROC_DATA *)&rct_proc_data );	// 車室情報（現在）印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ
				PKJOUNOW_edit( msg_data->Kikai_no, J_PRI, msg_data->prn_menu, (PRN_PROC_DATA *)&jnl_proc_data );	// 車室情報（現在）印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
				PKJOUNOW_edit( msg_data->Kikai_no, R_PRI, msg_data->prn_menu, (PRN_PROC_DATA *)&rct_proc_data );	// 車室情報（現在）印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				PKJOUNOW_edit( msg_data->Kikai_no, J_PRI, msg_data->prn_menu, (PRN_PROC_DATA *)&jnl_proc_data );	// 車室情報（現在）印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}
		End_Set( msg, terget_pri );																// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		車室情報（退避）印字要求ﾒｯｾｰｼﾞ受信処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnPKJOU_SAV( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-11													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnPKJOU_SAV( MSG *msg )
{
	T_FrmPkjouSav	*msg_data;							// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmPkjouSav *)msg->data;				// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){			// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );					// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				PKJOUSAV_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 車室情報（退避）印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				PKJOUSAV_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 車室情報（退避）印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				PKJOUSAV_edit( msg_data->Kikai_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 車室情報（退避）印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				PKJOUSAV_edit( msg_data->Kikai_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 車室情報（退避）印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}
		End_Set( msg, terget_pri );																// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		定期有効／無効印字要求ﾒｯｾｰｼﾞ受信処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTEIKIDATA1( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-09													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnTEIKIDATA1( MSG *msg )
{
	T_FrmTeikiData1	*msg_data;							// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmTeikiData1 *)msg->data;			// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( next_prn_msg == OFF ){							// 新規の印字要求？

		//	新規の印字要求の場合
		PrnTEIKID1_datachk( msg_data );					// 定期有効／無効印字要求ﾃﾞｰﾀﾁｪｯｸ
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){			// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );					// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				TEIKID1_edit(							// 定期有効／無効印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								msg_data->Pkno_syu,
								msg_data->Req_syu,
								R_PRI,
								(PRN_PROC_DATA *)&rct_proc_data
							);
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				TEIKID1_edit(							// 定期有効／無効印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								msg_data->Pkno_syu,
								msg_data->Req_syu,
								J_PRI,
								(PRN_PROC_DATA *)&jnl_proc_data
							);
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				TEIKID1_edit(							// 定期有効／無効印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								msg_data->Pkno_syu,
								msg_data->Req_syu,
								R_PRI,
								(PRN_PROC_DATA *)&rct_proc_data
							);

				TEIKID1_edit(							// 定期有効／無効印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								msg_data->Pkno_syu,
								msg_data->Req_syu,
								J_PRI,
								(PRN_PROC_DATA *)&jnl_proc_data
							);

				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}
		End_Set( msg, terget_pri );						// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		定期入庫／出庫印字要求ﾒｯｾｰｼﾞ受信処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTEIKIDATA2( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-09													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnTEIKIDATA2( MSG *msg )
{
	T_FrmTeikiData2	*msg_data;							// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmTeikiData2 *)msg->data;			// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( next_prn_msg == OFF ){							// 新規の印字要求？

		//	新規の印字要求の場合
		PrnTEIKID2_datachk( msg_data );					// 定期入庫／出庫印字要求ﾃﾞｰﾀﾁｪｯｸ
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){			// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );					// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				TEIKID2_edit(							// 定期入庫／出庫印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								msg_data->Pkno_syu,
								msg_data->Req_syu,
								R_PRI,
								(PRN_PROC_DATA *)&rct_proc_data
							);
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				TEIKID2_edit(							// 定期入庫／出庫印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								msg_data->Pkno_syu,
								msg_data->Req_syu,
								J_PRI,
								(PRN_PROC_DATA *)&jnl_proc_data
							);
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				TEIKID2_edit(							// 定期入庫／出庫印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								msg_data->Pkno_syu,
								msg_data->Req_syu,
								R_PRI,
								(PRN_PROC_DATA *)&rct_proc_data
							);

				TEIKID2_edit(							// 定期入庫／出庫印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								msg_data->Pkno_syu,
								msg_data->Req_syu,
								J_PRI,
								(PRN_PROC_DATA *)&jnl_proc_data
							);

				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}
		End_Set( msg, terget_pri );						// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ｻｰﾋﾞｽ券期限印字要求ﾒｯｾｰｼﾞ受信処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSVSTIK_KIGEN( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-25													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnSVSTIK_KIGEN( MSG *msg )
{
	T_FrmStikKigen	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmStikKigen *)msg->data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ

			SVSTIKK_edit( msg_data->Kikai_no, msg_data->Kakari_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ｻｰﾋﾞｽ券期限印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ

			SVSTIKK_edit( msg_data->Kikai_no, msg_data->Kakari_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ｻｰﾋﾞｽ券期限印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

			SVSTIKK_edit( msg_data->Kikai_no, msg_data->Kakari_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// ｻｰﾋﾞｽ券期限印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			SVSTIKK_edit( msg_data->Kikai_no, msg_data->Kakari_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// ｻｰﾋﾞｽ券期限印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}

	End_Set( msg, terget_pri );															// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
}

/*[]-----------------------------------------------------------------------[]*/
/*|		特別日印字要求ﾒｯｾｰｼﾞ受信処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSPLDAY( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-26													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnSPLDAY( MSG *msg )
{
	T_FrmSplDay	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmSplDay *)msg->data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				SPLDAY_edit( msg_data->Kikai_no, msg_data->Kakari_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 特別日印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				SPLDAY_edit( msg_data->Kikai_no, msg_data->Kakari_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 特別日印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				SPLDAY_edit( msg_data->Kikai_no, msg_data->Kakari_no, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 特別日印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				SPLDAY_edit( msg_data->Kikai_no, msg_data->Kakari_no, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 特別日印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}

		End_Set( msg, terget_pri );															// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		停留車情報印字要求ﾒｯｾｰｼﾞ受信処理									*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTEIRYUU_JOU( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-07-20													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnTEIRYUU_JOU( MSG *msg )
{
	T_FrmTeiRyuuJou	*msg_data;					// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data = (T_FrmTeiRyuuJou *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				TRJOU_edit(										// 停留車情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
							msg_data->Kikai_no,
							msg_data->Kakari_no,
							msg_data->Day,
							msg_data->Cnt,
							msg_data->Data,
							R_PRI,
							(PRN_PROC_DATA *)&rct_proc_data );

				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				TRJOU_edit(										// 停留車情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
							msg_data->Kikai_no,
							msg_data->Kakari_no,
							msg_data->Day,
							msg_data->Cnt,
							msg_data->Data,
							J_PRI,
							(PRN_PROC_DATA *)&jnl_proc_data );

				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				TRJOU_edit(										// 停留車情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
							msg_data->Kikai_no,
							msg_data->Kakari_no,
							msg_data->Day,
							msg_data->Cnt,
							msg_data->Data,
							R_PRI,
							(PRN_PROC_DATA *)&rct_proc_data );

				TRJOU_edit(										// 停留車情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
							msg_data->Kikai_no,
							msg_data->Kakari_no,
							msg_data->Day,
							msg_data->Cnt,
							msg_data->Data,
							J_PRI,
							(PRN_PROC_DATA *)&jnl_proc_data );

				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}

		End_Set( msg, terget_pri );								// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		停復電情報印字要求ﾒｯｾｰｼﾞ受信処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTEIFUK_JOU( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-27													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnTEIFUK_JOU( MSG *msg )
{
	T_FrmTeiFukJou	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	Pon_log			*Ponlog;		// 停復電情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data	= (T_FrmTeiFukJou *)msg->data;			// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	Ponlog		= &(msg_data->Ponlog);					// 停復電情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	PrnTFJOU_datachk( Ponlog );							// 停復電情報ﾃﾞｰﾀﾁｪｯｸ

	switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ

			TFJOU_edit( Ponlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 停復電情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ

			TFJOU_edit( Ponlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 停復電情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

			TFJOU_edit( Ponlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 停復電情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			TFJOU_edit( Ponlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 停復電情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}

	End_Set( msg, terget_pri );												// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
}

/*[]-----------------------------------------------------------------------[]*/
/*|		停復電履歴印字要求ﾒｯｾｰｼﾞ受信処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTEIFUK_LOG( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-27													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnTEIFUK_LOG( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data = (T_FrmLogPriReq2 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ
				TFLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 停復電履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ
				TFLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 停復電履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
				TFLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 停復電履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				TFLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 停復電履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}
		End_Set( msg, terget_pri );															// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		個別精算情報印字要求ﾒｯｾｰｼﾞ受信処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSEISAN_LOG( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnSEISAN_LOG( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmLogPriReq2 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				SEISANLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 個別精算情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				SEISANLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 個別精算情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				SEISANLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 個別精算情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				SEISANLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 個別精算情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}

		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		不正・強制出庫情報印字要求ﾒｯｾｰｼﾞ受信処理							|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnFUSKYO_JOU( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-08-02													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnFUSKYO_JOU( MSG *msg )
{
	T_FrmFusKyo		*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	flp_log			*fuskyo;		// 不正強制出庫情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data	= (T_FrmFusKyo *)msg->data;				// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	fuskyo		= (flp_log *)(msg_data->prn_data);		// 不正強制出庫情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰｯﾄ

	PrnFKJOU_datachk( fuskyo );							// 不正強制出庫情報ﾃﾞｰﾀﾁｪｯｸ

	switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ

			FUSKYOJOU_edit( fuskyo, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 不正強制出庫情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ

			FUSKYOJOU_edit( fuskyo, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 不正強制出庫情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

			FUSKYOJOU_edit( fuskyo, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 不正強制出庫情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			FUSKYOJOU_edit( fuskyo, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 不正強制出庫情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}

	End_Set( msg, terget_pri );												// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
}

/*[]-----------------------------------------------------------------------[]*/
/*|		不正・強制出庫履歴印字要求ﾒｯｾｰｼﾞ受信処理							|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnFUSKYO_LOG( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-15													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnFUSKYO_LOG( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmLogPriReq2 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				FUSKYOLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 不正・強制出庫履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				FUSKYOLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 不正・強制出庫履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				FUSKYOLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 不正・強制出庫履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				FUSKYOLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 不正・強制出庫履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}

		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		用紙入替時テスト印字要求ﾒｯｾｰｼﾞ受信処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnIREKAE_TEST( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-04-12													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnIREKAE_TEST( MSG *msg )
{

	switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ

			IREKAETST_edit( R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 用紙入替時ﾃｽﾄ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ

			IREKAETST_edit( J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 用紙入替時ﾃｽﾄ印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

			IREKAETST_edit( R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 用紙入替時ﾃｽﾄ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			IREKAETST_edit( J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 用紙入替時ﾃｽﾄ印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}

	End_Set( msg, terget_pri );																	// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾌﾟﾘﾝﾀﾃｽﾄ印字要求ﾒｯｾｰｼﾞ受信処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnPRINT_TEST( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-06													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnPRINT_TEST( MSG *msg )
{


	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				PRITEST_edit( R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ﾌﾟﾘﾝﾀﾃｽﾄ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				PRITEST_edit( J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ﾌﾟﾘﾝﾀﾃｽﾄ印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				PRITEST_edit( R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ﾌﾟﾘﾝﾀﾃｽﾄ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				PRITEST_edit( J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ﾌﾟﾘﾝﾀﾃｽﾄ印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）

				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}
		End_Set( msg, terget_pri );											// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ロゴ印字データ登録要求ﾒｯｾｰｼﾞ受信処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnLOGO_REGIST( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-07													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnLOGO_REGIST( MSG *msg )
{

	switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ

			LOGOREG_edit( R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ロゴ印字データ登録（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ

			LOGOREG_edit( J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ロゴ印字データ登録（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

			LOGOREG_edit( R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ロゴ印字データ登録（ﾚｼｰﾄ）
			LOGOREG_edit( J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ロゴ印字データ登録（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}

	End_Set( msg, terget_pri );											// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
}

/*[]-----------------------------------------------------------------------[]*/
/*|		印字中止ﾒｯｾｰｼﾞ受信処理												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnINJI_TYUUSHI( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnINJI_TYUUSHI( MSG *msg )
{
	switch( terget_pri ){	// 対象ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ
			PrnINJI_TYUUSHI_R( msg );	// 印字中止ﾒｯｾｰｼﾞ受信処理（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ
			PrnINJI_TYUUSHI_J( msg );	// 印字中止ﾒｯｾｰｼﾞ受信処理（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			PrnINJI_TYUUSHI_R( msg );	// 印字中止ﾒｯｾｰｼﾞ受信処理（ﾚｼｰﾄ）
			PrnINJI_TYUUSHI_J( msg );	// 印字中止ﾒｯｾｰｼﾞ受信処理（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（対象ﾌﾟﾘﾝﾀｴﾗｰ）
			return;
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ﾚｼｰﾄ印字中止ﾒｯｾｰｼﾞ受信処理											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnINJI_TYUUSHI_R( *msg )							|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnINJI_TYUUSHI_R( MSG *msg )
{
	PRN_PROC_DATA	*p_proc_data;		// 編集処理制御ﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	p_proc_data = (PRN_PROC_DATA *)&rct_proc_data;	// 編集処理制御ﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( p_proc_data->Printing != 0 ){				// 印字処理中？

		//	印字処理中の場合
		if( p_proc_data->Tyushi_Cmd == OFF ){		// 印字中止要求未受信？

			Rct_top_edit( ON );						// ﾚｼｰﾄ先頭印字ﾃﾞｰﾀ編集処理（ﾛｺﾞ印字／ﾍｯﾀﾞｰ印字／用紙ｶｯﾄ制御）
			PrnOut( R_PRI );						// ﾌﾟﾘﾝﾀ起動

			p_proc_data->Tyushi_Cmd = ON;			// 印字中止ﾒｯｾｰｼﾞ受信ﾌﾗｸﾞON
			p_proc_data->Final = OFF;				// 分割印字最終ﾌﾞﾛｯｸ通知：OFF
		}
	}
	else{
		//	印字処理中でない場合
		Inji_Cancel( msg, R_PRI );					// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ｼﾞｬｰﾅﾙ印字中止ﾒｯｾｰｼﾞ受信処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnINJI_TYUUSHI_J( *msg )							|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnINJI_TYUUSHI_J( MSG *msg )
{
	PRN_PROC_DATA	*p_proc_data;		// 編集処理制御ﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	p_proc_data = (PRN_PROC_DATA *)&jnl_proc_data;	// 編集処理制御ﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( p_proc_data->Printing != 0 ){				// 印字処理中？

		//	印字処理中の場合
		if( p_proc_data->Tyushi_Cmd == OFF ){		// 印字中止要求未受信？

			p_proc_data->Tyushi_Cmd = ON;			// 印字中止ﾒｯｾｰｼﾞ受信ﾌﾗｸﾞON
			p_proc_data->Final = OFF;				// 分割印字最終ﾌﾞﾛｯｸ通知：OFF
		}
	}
	else{
		//	印字処理中でない場合
		Inji_Cancel( msg, J_PRI );					// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		印字編集ﾃﾞｰﾀ１ﾌﾞﾛｯｸ印字終了ﾒｯｾｰｼﾞ受信処理							|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnINJI_END( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-02-24													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnINJI_END( MSG *msg )
{
	PRN_PROC_DATA	*p_proc_data;		// 編集処理制御ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	MSG				*p_next_msg;		// 印字要求ﾒｯｾｰｼﾞ保存ﾊﾞｯﾌｧﾎﾟｲﾝﾀ
	uchar			gyou_cnt;			// 印字要求行数
	ushort			timer;				// 印字ﾃﾞｰﾀ編集遅延ﾀｲﾏｰ値


	switch( terget_pri ){	// 対象ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// 	ﾚｼｰﾄ

			p_proc_data = (PRN_PROC_DATA *)&rct_proc_data;	// ﾚｼｰﾄ編集処理制御ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ
			p_next_msg	= (MSG *)&NextMsg_r;				// ﾚｼｰﾄ印字要求ﾒｯｾｰｼﾞ保存ﾊﾞｯﾌｧﾎﾟｲﾝﾀｾｯﾄ

// GG129000(S) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
			GyouCnt_All_r += GyouCnt_r;						// 送信済み印字ﾃﾞｰﾀ全行数取得
// GG129000(E) 富士急ハイランド機能改善（領収証発行POPUPをレシート印字完了で削除）（GM803003流用）
			gyou_cnt	= GyouCnt_r;						// 送信済み印字ﾃﾞｰﾀ行数取得
			GyouCnt_r	= 0;								// 送信済み印字ﾃﾞｰﾀ行数０ｸﾘｱ

			break;

		case	J_PRI:		// 	ｼﾞｬｰﾅﾙ

			p_proc_data = (PRN_PROC_DATA *)&jnl_proc_data;	// ｼﾞｬｰﾅﾙ編集処理制御ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ
			p_next_msg	= (MSG *)&NextMsg_j;				// ｼﾞｬｰﾅﾙ印字要求ﾒｯｾｰｼﾞ保存ﾊﾞｯﾌｧﾎﾟｲﾝﾀｾｯﾄ

// GG129000(S) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
			GyouCnt_All_j += GyouCnt_j;						// 送信済み印字ﾃﾞｰﾀ全行数取得
// GG129000(E) D.Inaba 2023/09/21 ゲート式車番チケットレスシステム対応（詳細設計#5373：新レシートプリンタ対応）
			gyou_cnt	= GyouCnt_j;						// 送信済み印字ﾃﾞｰﾀ行数取得
			GyouCnt_j	= 0;								// 送信済み印字ﾃﾞｰﾀ行数０ｸﾘｱ

			break;

		default:			// その他（対象ﾌﾟﾘﾝﾀｴﾗｰ）
			return;
	}
	if( p_proc_data->Printing != 0 ){

		// 印字処理中の場合

		if( (p_proc_data->Final != ON) && (p_next_msg->command != 0) ){

			// 最終ﾌﾞﾛｯｸの印字終了でない場合

			if( p_proc_data->EditWait == OFF ){					// 印字ﾃﾞｰﾀ編集遅延ﾀｲﾏｰ起動状態？

				// 遅延ﾀｲﾏｰ起動中でない場合

				timer = (ushort)(PRN_WAIT_TIMER * gyou_cnt);	// 印字ﾃﾞｰﾀ編集遅延ﾀｲﾏｰ計算（送信済み印字ﾃﾞｰﾀ行数×ﾀｲﾏｰ値）
// MH810104 GG119201(S) 電子ジャーナル対応
				if (terget_pri == J_PRI && isEJA_USE()) {
// MH810105(S) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
//					// 電子ジャーナルは遅延なしとする
//					timer = 0;
					// 電子ジャーナルへの印字データは1ブロックごとに書込み開始・終了を送信しないよう変更したので
					// 固定で遅延タイマを動作させる（20×5＝100ms）
					timer = 5;
// MH810105(E) MH364301 電子ジャーナル対応（ジャーナル印字データの停電保障対策）
				}
// MH810104 GG119201(E) 電子ジャーナル対応

				if( timer == 0 ){
					// 印字ﾃﾞｰﾀ編集遅延なし
					PrnNext( p_next_msg, terget_pri );			// 印字ﾃﾞｰﾀ次ﾌﾞﾛｯｸ編集要求ﾒｯｾｰｼﾞ送信
				}
				else{
					// 印字ﾃﾞｰﾀ編集遅延あり
					if( terget_pri == R_PRI ){
						Lagtim( PRNTCBNO , 5 , timer  );		// 印字ﾃﾞｰﾀ編集遅延ﾀｲﾏｰｽﾀｰﾄ（ﾚｼｰﾄ）
					}
					else{
						Lagtim( PRNTCBNO , 6 , timer  );		// 印字ﾃﾞｰﾀ編集遅延ﾀｲﾏｰｽﾀｰﾄ（ｼﾞｬｰﾅﾙ）
					}
					p_proc_data->EditWait = ON;					// 印字ﾃﾞｰﾀ編集遅延ﾀｲﾏｰ起動状態ｾｯﾄ
				}
				p_proc_data->Split ++;							// 印字処理中のﾌﾞﾛｯｸ数＋１
			}
		}
		else{
			// 最終ﾌﾞﾛｯｸの印字終了の場合
			prn_proc_data_clr( terget_pri );				// ﾌﾟﾘﾝﾀ毎の制御ｴﾘｱ初期化
			p_next_msg->command = 0;
// MH810104 GG119201(S) 電子ジャーナル対応（書込み中に時計設定しない）
			if (eja_prn_buff.PrnClkReqFlg != 0) {
				PrnCmd_Clock();
				eja_prn_buff.PrnClkReqFlg = 0;
			}
			if (eja_prn_buff.PrnInfReqFlg != 0) {
				PrnCmd_InfoReq(eja_prn_buff.PrnInfReqFlg);
				eja_prn_buff.PrnInfReqFlg = 0;
			}
// MH810104 GG119201(E) 電子ジャーナル対応（書込み中に時計設定しない）
		}
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		印字異常終了制御処理												|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnERREND_PROC( void )								|*/
/*|																			|*/
/*|	PARAMETER		:	void												|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-03													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnERREND_PROC( void )
{
	PRN_DATA_BUFF	*p_data_buff;		// ﾌﾟﾘﾝﾀ送受信ﾃﾞｰﾀ管理ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	PRN_PROC_DATA	*p_proc_data;		// 編集処理制御ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	ushort			command;			// 印字処理中ｺﾏﾝﾄﾞ
	uchar			pri_sts;			// ﾌﾟﾘﾝﾀｽﾃｰﾀｽ


	switch( terget_pri ){	// 対象ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// 	ﾚｼｰﾄ

			p_data_buff = (PRN_DATA_BUFF *)&rct_prn_buff;	// ﾌﾟﾘﾝﾀ送受信ﾃﾞｰﾀ管理ﾃﾞｰﾀﾎﾟｲﾝﾀｰ（ﾚｼｰﾄ）
			p_proc_data = (PRN_PROC_DATA *)&rct_proc_data;	// 編集処理制御ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ		（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// 	ｼﾞｬｰﾅﾙ

			p_data_buff = (PRN_DATA_BUFF *)&jnl_prn_buff;	// ﾌﾟﾘﾝﾀ送受信ﾃﾞｰﾀ管理ﾃﾞｰﾀﾎﾟｲﾝﾀｰ（ｼﾞｬｰﾅﾙ）
			p_proc_data = (PRN_PROC_DATA *)&jnl_proc_data;	// 編集処理制御ﾃﾞｰﾀﾎﾟｲﾝﾀｾｯﾄ		（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（対象ﾌﾟﾘﾝﾀｴﾗｰ）
			return;
	}
	command	= p_proc_data->Printing;						// 印字処理中の印字要求ｺﾏﾝﾄﾞ			取得
	pri_sts	= (uchar)(p_data_buff->PrnState[0] & 0x0e);		// ﾌﾟﾘﾝﾀ現在ｽﾃｰﾀｽ（ﾆｱｴﾝﾄﾞｽﾃｰﾀｽは除く）	取得

// MH810104 GG119201(S) 電子ジャーナル対応（印字データを再送しない）
//	if( command != 0 && pri_sts != 0 ){
	if( command != 0 && pri_sts != 0 && isPrnRetryOver() ){
// MH810104 GG119201(E) 電子ジャーナル対応（印字データを再送しない）

		//	印字処理中　且つ　印字不可状態　の場合
		Inji_ErrEnd( command, PRI_ERR_STAT, terget_pri );	// 印字異常終了処理（異常終了ﾒｯｾｰｼﾞ送信）
	}
}

/*[]-----------------------------------------------------------------------[]*/
/*|		領収証印字要求ﾃﾞｰﾀﾁｪｯｸ処理											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnRYOUSYUU_datachk( *Ryousyuu )					|*/
/*|																			|*/
/*|	PARAMETER		:	Receipt_data *Ryousyuu = 領収証ﾃﾞｰﾀﾎﾟｲﾝﾀｰ			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : 正常										|*/
/*|							NG : 異常										|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-09													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnRYOUSYUU_datachk( Receipt_data *Ryousyuu )
{
	uchar		ret = OK;		// ﾁｪｯｸ結果
	uchar		data_no;		// 割引券ﾃﾞｰﾀNo.
	wari_tiket	*p_wari_tiket;	// 割引券ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	uchar		wtik_syu;		// 割引券種別
	wari_tiket	wari_dt;

	//	印字種別１（通常／復電）ﾁｪｯｸ
	if( NG == Prn_data_chk( RYOUSYUU_PRN_SYU1, (unsigned long)Ryousyuu->WFlag ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	印字種別２（通常／精算中止）ﾁｪｯｸ
	if( NG == Prn_data_chk( RYOUSYUU_PRN_SYU2, (unsigned long)Ryousyuu->chuusi ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	駐車位置ﾃﾞｰﾀﾁｪｯｸ
	if( NG == Prn_data_chk( PKICHI_DATA, (unsigned long)Ryousyuu->WPlace ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	精算種別ﾁｪｯｸ
	if( NG == Prn_data_chk( SEISAN_SYU, (unsigned long)Ryousyuu->Seisan_kind ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	入庫時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&Ryousyuu->TInTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	出庫時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&Ryousyuu->TOutTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	料金種別ﾁｪｯｸ
	if( NG == Prn_data_chk( RYOUKIN_SYU, (unsigned long)Ryousyuu->syu ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	定期券
	if( Ryousyuu->teiki.id != 0 ){	// 定期券使用？
		//	定期券種別ﾁｪｯｸ
		if( NG == Prn_data_chk( TEIKI_SYU, (unsigned long)Ryousyuu->teiki.syu ) ){
			Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
			ret = NG;
		}
	}
	//	割引券
	for( data_no = 0 ; data_no < WTIK_USEMAX ; data_no++ ){

		disc_wari_conv( &Ryousyuu->DiscountData[data_no], &wari_dt );
		p_wari_tiket = &wari_dt;									// ﾁｪｯｸ対象割引券ﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
		wtik_syu = p_wari_tiket->tik_syu;							// 割引券種別ﾃﾞｰﾀ取得

			//	割引券ﾃﾞｰﾀﾁｪｯｸ
			switch( wtik_syu ){

				case	SERVICE:	// ｻｰﾋﾞｽ券
				case	C_SERVICE:	// 精算中止ｻｰﾋﾞｽ券
					//	駐車場Ｎｏ．種別ﾁｪｯｸ
					if( NG == Prn_data_chk( PKNO_SYU, (unsigned long)p_wari_tiket->pkno_syu ) ){
						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
						ret = NG;
					}
					//	ｻｰﾋﾞｽ券種別ﾁｪｯｸ
					if( NG == Prn_data_chk( SERVICE_SYU, (unsigned long)p_wari_tiket->syubetu ) ){
						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
						ret = NG;
					}
// MH810100(S) 2020/08/26 #4763【初回精算で割引を満額適用されずに再精算を行うとE0251が発生する
// 0枚を許容するのでコメントアウト
//					//	使用枚数ﾁｪｯｸ
//					if( NG == Prn_data_chk( TIKUSE_CNT, (unsigned long)p_wari_tiket->maisuu ) ){
//						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
//						ret = NG;
//					}
// MH810100(E) 2020/08/26 #4763【初回精算で割引を満額適用されずに再精算を行うとE0251が発生する
					break;

				case	KAKEURI:	// 掛売券
				case	C_KAKEURI:	// 精算中止掛売券
// MH810100(S) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
				case	SHOPPING:	// 買物割引
				case 	C_SHOPPING:	// 買物割引
// MH810100(E) 2020/07/10 車番チケットレス(#4531 仕様変更 買物割引の領収証の印字を分けて印字する)
					//	駐車場Ｎｏ．種別ﾁｪｯｸ
					if( NG == Prn_data_chk( PKNO_SYU, (unsigned long)p_wari_tiket->pkno_syu ) ){
						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
						ret = NG;
					}
// GG124100(S) R.Endo 2022/09/08 車番チケットレス3.0 #6580 店番号101以上の割引を使って精算した場合、印字要求データエラー(E0251)が発生する [共通改善項目 No1530]
// 					//	店Ｎｏ．ﾁｪｯｸ
// 					if( NG == Prn_data_chk( MISE_NO, (unsigned long)p_wari_tiket->syubetu ) ){
// 						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
// 						ret = NG;
// 					}
// GG124100(E) R.Endo 2022/09/08 車番チケットレス3.0 #6580 店番号101以上の割引を使って精算した場合、印字要求データエラー(E0251)が発生する [共通改善項目 No1530]
// MH810100(S) 2020/08/26 #4763【初回精算で割引を満額適用されずに再精算を行うとE0251が発生する
// 0枚を許容するのでコメントアウト
//					//	使用枚数ﾁｪｯｸ
//					if( NG == Prn_data_chk( TIKUSE_CNT, (unsigned long)p_wari_tiket->maisuu ) ){
//						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
//						ret = NG;
//					}
// MH810100(E) 2020/08/26 #4763【初回精算で割引を満額適用されずに再精算を行うとE0251が発生する
					break;

				case	KAISUU:		// 回数券
				case	C_KAISUU:	// 精算中止回数券
					//	駐車場Ｎｏ．種別ﾁｪｯｸ
					if( NG == Prn_data_chk( PKNO_SYU, (unsigned long)p_wari_tiket->pkno_syu ) ){
						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
						ret = NG;
					}
					//	使用枚数ﾁｪｯｸ
					if( NG == Prn_data_chk( TIKUSE_CNT, (unsigned long)p_wari_tiket->maisuu ) ){
						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
						ret = NG;
					}
					break;

				case	WARIBIKI:	// 割引券
					//	駐車場Ｎｏ．種別ﾁｪｯｸ
					if( NG == Prn_data_chk( PKNO_SYU, (unsigned long)p_wari_tiket->pkno_syu ) ){
						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
						ret = NG;
					}
					//	割引種別ﾁｪｯｸ
					if( NG == Prn_data_chk( WARIBIKI_SYU, (unsigned long)p_wari_tiket->syubetu ) ){
						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
						ret = NG;
					}
					//	使用枚数ﾁｪｯｸ
					if( NG == Prn_data_chk( TIKUSE_CNT, (unsigned long)p_wari_tiket->maisuu ) ){
						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
						ret = NG;
					}
					break;

				case	PREPAID:	// プリペイドカード
					if( NG == Prn_data_chk( PKNO_SYU, (unsigned long)p_wari_tiket->pkno_syu ) ){
						Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
						ret = NG;
					}
					break;

				default:		// その他
					break;
			}
	}
// MH810100(S) m.saito 2020/05/13 車番チケットレス(領収証印字：未精算出庫対応)
	//	精算未精算種別ﾁｪｯｸ
	if( NG == Prn_data_chk( UNPAID_SYU, (unsigned long)Ryousyuu->shubetsu ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
// MH810100(E) m.saito 2020/05/13 車番チケットレス(領収証印字：未精算出庫対応)

	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		預り証印字要求ﾃﾞｰﾀﾁｪｯｸ処理											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnAZUKARI_datachk( *Azukari )						|*/
/*|																			|*/
/*|	PARAMETER		:	Azukari_data *Azukari = 預り証ﾃﾞｰﾀﾎﾟｲﾝﾀｰ			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : 正常										|*/
/*|							NG : 異常										|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-11													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnAZUKARI_datachk( Azukari_data *Azukari )
{
	uchar		ret = OK;	// ﾁｪｯｸ結果


	//	駐車位置ﾃﾞｰﾀﾁｪｯｸ
	if( NG == Prn_data_chk( PKICHI_DATA, (unsigned long)Azukari->WPlace ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&Azukari->TTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		受付券印字要求ﾃﾞｰﾀﾁｪｯｸ処理											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnUKETUKE_datachk( *Uketuke )						|*/
/*|																			|*/
/*|	PARAMETER		:	Uketuke_data *Uketuke = 受付券ﾃﾞｰﾀﾎﾟｲﾝﾀｰ			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : 正常										|*/
/*|							NG : 異常										|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnUKETUKE_datachk( Uketuke_data *Uketuke )
{
	uchar		ret = OK;	// ﾁｪｯｸ結果


	//	駐車位置ﾃﾞｰﾀﾁｪｯｸ
	if( NG == Prn_data_chk( PKICHI_DATA, (unsigned long)Uketuke->WPlace ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	発行時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&Uketuke->ISTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	入庫時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&Uketuke->TTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		集計印字要求ﾃﾞｰﾀﾁｪｯｸ処理											|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSYUUKEI_datachk( *Syuukei )						|*/
/*|																			|*/
/*|	PARAMETER		:	SYUKEI *Syuukei = 集計ﾃﾞｰﾀﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : 正常										|*/
/*|							NG : 異常										|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-14													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnSYUUKEI_datachk( SYUKEI *Syuukei )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果


	//	今回集計時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&Syuukei->NowTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	前回集計時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&Syuukei->OldTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		ｺｲﾝ金庫集計印字要求ﾃﾞｰﾀﾁｪｯｸ処理										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnCOKI_SK_datachk( *Coinkinko )					|*/
/*|																			|*/
/*|	PARAMETER		:	COIN_SYU	*Coinkinko	= ｺｲﾝ金庫集計ﾃﾞｰﾀﾎﾟｲﾝﾀｰ		|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : 正常										|*/
/*|							NG : 異常										|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-25													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnCOKI_SK_datachk( COIN_SYU	*Coinkinko )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果


	//	今回集計時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&Coinkinko->NowTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	前回集計時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&Coinkinko->OldTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		紙幣金庫集計印字要求ﾃﾞｰﾀﾁｪｯｸ処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSIKI_SK_datachk( *Siheikinko )					|*/
/*|																			|*/
/*|	PARAMETER		:	NOTE_SYU	*Siheikinko	= 紙幣金庫集計ﾃﾞｰﾀﾎﾟｲﾝﾀｰ	|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : 正常										|*/
/*|							NG : 異常										|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-03-25													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnSIKI_SK_datachk( NOTE_SYU	*Siheikinko )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果


	//	今回集計時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&Siheikinko->NowTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	前回集計時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&Siheikinko->OldTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		釣銭管理集計印字要求ﾃﾞｰﾀﾁｪｯｸ処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTURIKAN_datachk( *TuriKan )						|*/
/*|																			|*/
/*|	PARAMETER		:	TURI_KAN *TuriKan = 釣銭管理集計ﾃﾞｰﾀﾎﾟｲﾝﾀｰ			|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : 正常										|*/
/*|							NG : 異常										|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-06-01													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnTURIKAN_datachk( TURI_KAN *TuriKan )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果


	//	今回集計時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&TuriKan->NowTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	前回集計時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&TuriKan->OldTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		ｴﾗｰ情報印字要求ﾃﾞｰﾀﾁｪｯｸ処理												|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnERRJOU_datachk( *Errlog )							|*/
/*|																				|*/
/*|	PARAMETER		:	Err_log	*Errlog= ｴﾗｰ情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰ						|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : 正常											|*/
/*|							NG : 異常											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-06-10														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	PrnERRJOU_datachk( Err_log *Errlog )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果


	//	発生時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&Errlog->Date_Time ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	ｴﾗｰ種別ﾁｪｯｸ
	if( Errlog->Errsyu > ERR_INFO_SYU_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	ｴﾗｰｺｰﾄﾞﾁｪｯｸ
	if( Errlog->Errcod > ERR_INFO_COD_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		ｴﾗｰ情報履歴印字要求ﾃﾞｰﾀﾁｪｯｸ処理											|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnERRLOG_datachk( *msg_data )							|*/
/*|																				|*/
/*|	PARAMETER		:	T_FrmLogPriReq1		*msg_data = ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ		|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : 正常											|*/
/*|							NG : 異常											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-06-10														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	PrnERRLOG_datachk( T_FrmLogPriReq2 *msg_data )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果

	//	指定日時ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&msg_data->TSttTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		ｱﾗｰﾑ情報印字要求ﾃﾞｰﾀﾁｪｯｸ処理											|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnARMJOU_datachk( *Errlog )							|*/
/*|																				|*/
/*|	PARAMETER		:	Arm_log	*Armlog	= ｱﾗｰﾑ情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰ					|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : 正常											|*/
/*|							NG : 異常											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-06-10														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	PrnARMJOU_datachk( Arm_log *Armlog )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果


	//	発生時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&Armlog->Date_Time ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	ｱﾗｰﾑ種別ﾁｪｯｸ
	if( Armlog->Armsyu > ARM_INFO_SYU_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	ｱﾗｰﾑｺｰﾄﾞﾁｪｯｸ
	if( Armlog->Armcod > ARM_INFO_COD_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		ｱﾗｰﾑ情報履歴印字要求ﾃﾞｰﾀﾁｪｯｸ処理										|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnARMLOG_datachk( *msg_data )							|*/
/*|																				|*/
/*|	PARAMETER		:	T_FrmLogPriReq1		*msg_data = ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ		|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : 正常											|*/
/*|							NG : 異常											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-06-13														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	PrnARMLOG_datachk( T_FrmLogPriReq2 *msg_data )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果

	//	指定日時ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&msg_data->TSttTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		操作情報印字要求ﾃﾞｰﾀﾁｪｯｸ処理											|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnOPEJOU_datachk( *Opelog )							|*/
/*|																				|*/
/*|	PARAMETER		:	Ope_log	*Opelog	= 操作情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰ					|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : 正常											|*/
/*|							NG : 異常											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-06-10														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	PrnOPEJOU_datachk( Ope_log *Opelog )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果


	//	操作時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&Opelog->Date_Time ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	操作種別ﾁｪｯｸ
	if( Opelog->OpeKind > OPE_INFO_SYU_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	操作ｺｰﾄﾞﾁｪｯｸ
	if( Opelog->OpeCode > OPE_INFO_COD_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		操作情報履歴印字要求ﾃﾞｰﾀﾁｪｯｸ処理										|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnOPELOG_datachk( *msg_data )							|*/
/*|																				|*/
/*|	PARAMETER		:	T_FrmLogPriReq1		*msg_data = ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ		|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : 正常											|*/
/*|							NG : 異常											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-06-13														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	PrnOPELOG_datachk( T_FrmLogPriReq2 *msg_data )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果

	//	指定日時ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&msg_data->TSttTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		モニタ情報印字要求ﾃﾞｰﾀﾁｪｯｸ処理											|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnMONJOU_datachk( *Monlog )							|*/
/*|																				|*/
/*|	PARAMETER		:	Mon_log	*Monlog	= モニタ情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰ					|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : 正常											|*/
/*|							NG : 異常											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	machida kei														|*/
/*|	Date	:	2005-11-30														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	PrnMONJOU_datachk( Mon_log *Monlog )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果


	//	発生時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&Monlog->Date_Time ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	モニタ種別ﾁｪｯｸ
	if( Monlog->MonKind > MON_INFO_SYU_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	モニタｺｰﾄﾞﾁｪｯｸ
	if( Monlog->MonCode > MON_INFO_COD_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		モニタ情報履歴印字要求ﾃﾞｰﾀﾁｪｯｸ処理										|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnMONLOG_datachk( *msg_data )							|*/
/*|																				|*/
/*|	PARAMETER		:	T_FrmLogPriReq1		*msg_data = ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ		|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : 正常											|*/
/*|							NG : 異常											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	R.HARA															|*/
/*|	Date	:	2006-02-28														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.--------[]*/
uchar	PrnMONLOG_datachk( T_FrmLogPriReq2 *msg_data )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果

	//	指定日時ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&msg_data->TSttTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		定期有効／無効印字要求ﾃﾞｰﾀﾁｪｯｸ処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTEIKID1_datachk( *msg_data )						|*/
/*|																			|*/
/*|	PARAMETER		:	T_FrmTeikiData1 *msg_data = ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ		|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : 正常										|*/
/*|							NG : 異常										|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-09													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnTEIKID1_datachk( T_FrmTeikiData1 *msg_data )
{
	uchar		ret = OK;		// ﾁｪｯｸ結果


	//	駐車場Ｎｏ．種別ﾁｪｯｸ
	if( NG == Prn_data_chk( PKNO_SYU, (unsigned long)msg_data->Pkno_syu ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	印字要求種別ﾁｪｯｸ(0:全指定、1:有効のみ、2:無効のみ)
	if( msg_data->Req_syu > 2 ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		定期入庫／出庫印字要求ﾃﾞｰﾀﾁｪｯｸ処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTEIKID2_datachk( *msg_data )						|*/
/*|																			|*/
/*|	PARAMETER		:	T_FrmTeikiData2 *msg_data = ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ		|*/
/*|																			|*/
/*|	RETURN VALUE	:	uchar	ret											|*/
/*|							OK : 正常										|*/
/*|							NG : 異常										|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-09													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
uchar	PrnTEIKID2_datachk( T_FrmTeikiData2 *msg_data )
{
	uchar		ret = OK;		// ﾁｪｯｸ結果


	//	駐車場Ｎｏ．種別ﾁｪｯｸ
	if( NG == Prn_data_chk( PKNO_SYU, (unsigned long)msg_data->Pkno_syu ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	印字要求種別ﾁｪｯｸ(0:全指定、1:入庫のみ、2:出庫のみ、3:初回のみ)
	if( msg_data->Req_syu > 3 ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		停復電情報印字要求ﾃﾞｰﾀﾁｪｯｸ処理											|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnTFJOU_datachk( *Ponlog )								|*/
/*|																				|*/
/*|	PARAMETER		:	Pon_log	*Ponlog= 停復電情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰ					|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : 正常											|*/
/*|							NG : 異常											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-04-27														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	PrnTFJOU_datachk( Pon_log *Ponlog )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果


	//	停電時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&Ponlog->Pdw_Date ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	復電時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&Ponlog->Pon_Date ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		不正・強制出庫情報印字要求ﾃﾞｰﾀﾁｪｯｸ処理									|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnFKJOU_datachk( *fuskyo )								|*/
/*|																				|*/
/*|	PARAMETER		:	flp_log	*fuskyo= 不正・強制出庫情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰ			|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : 正常											|*/
/*|							NG : 異常											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi														|*/
/*|	Date	:	2005-08-02														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
uchar	PrnFKJOU_datachk( flp_log *fuskyo )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果


	//	発生時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&fuskyo->Date_Time ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	入庫時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&fuskyo->In_Time ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		ｴﾗｰ受信割り込みの発生を連続してｴﾗｰ登録させないため						|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PriErrRecieve( void )									|*/
/*|	PARAMETER		:	void													|*/
/*|	RETURN VALUE	:	void													|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	T.Hashimoto														|*/
/*|	Date	:	2006-01-12														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.--------[]*/
void PriErrRecieve( void )
{
	_di();
	PriErrCount = 0;					// ｴﾗｰ受信の登録を許可する。Sci3Err()参照。
	_ei();
}

/*[]-----------------------------------------------------------------------[]*/
/*|		Suica通信ログ印字要求ﾒｯｾｰｼﾞ受信処理	　　							|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSuica_LOG( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	T.Namioka													|*/
/*|	Date	:	2005-07-27													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
void	PrnSuica_LOG( MSG *msg )
{
	T_FrmLogPriReq1		*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmLogPriReq1 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( next_prn_msg == OFF ){					// 新規の印字要求？

		//	新規の印字要求の場合
		PrnSuicaLOG_datachk( msg_data );			// ｴﾗｰ情報履歴印字要求ﾃﾞｰﾀﾁｪｯｸ
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				Suica_Log_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				Suica_Log_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				Suica_Log_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				Suica_Log_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}

		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}
/*[]-----------------------------------------------------------------------[]*/
/*|		Suica通信ログ直近データ用印字要求ﾒｯｾｰｼﾞ受信処理	　　				|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSuica_LOG2( *msg )								|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	A.iiizumi													|*/
/*|	Date	:	2012-09-10													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
void	PrnSuica_LOG2( MSG *msg )
{
	T_FrmLogPriReq1		*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmLogPriReq1 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( next_prn_msg == OFF ){					// 新規の印字要求？

		//	新規の印字要求の場合
		PrnSuicaLOG_datachk( msg_data );			// ｴﾗｰ情報履歴印字要求ﾃﾞｰﾀﾁｪｯｸ
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				Suica_Log_edit2( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				Suica_Log_edit2( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				Suica_Log_edit2( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				Suica_Log_edit2( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}

		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}
/*[]---------------------------------------------------------------------------[]*/
/*|		Suica通信ﾛｸﾞ印字要求ﾃﾞｰﾀﾁｪｯｸ処理										|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnSuicaLOG_datachk( *msg_data )						|*/
/*|																				|*/
/*|	PARAMETER		:	T_FrmLogPriReq1		*msg_data = ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ		|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : 正常											|*/
/*|							NG : 異常											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	T.Namioka												    	|*/
/*|	Date	:	2005-07-27												    	|*/
/*|	Update	:														　   		|*/
/*[]------------------------------------- Copyright(C) 2006 AMANO Corp.--------[]*/
uchar	PrnSuicaLOG_datachk( T_FrmLogPriReq1 *msg_data )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果

	//	印字時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&msg_data->PriTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

// MH810103 GG119202(S) 不要機能削除(センタークレジット)
///*[]-----------------------------------------------------------------------[]*/
///*|		ｸﾚｼﾞｯﾄ利用明細ﾃﾞｰﾀ印字要求ﾒｯｾｰｼﾞ受信処理							|*/
///*[]-----------------------------------------------------------------------[]*/
///*|	MODULE NAME		:	PrnCRE_USE( *msg )									|*/
///*|																			|*/
///*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
///*|																			|*/
///*|	RETURN VALUE	:	void												|*/
///*|																			|*/
///*[]-----------------------------------------------------------------------[]*/
///*|	Author	:	M.Yanase(COSMO)												|*/
///*|	Date	:	2006-07-14													|*/
///*|	Update	:																|*/
///*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
//void	PrnCRE_USE( MSG *msg )
//{
//	T_FrmLogPriReq3	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
//
//	msg_data = (T_FrmLogPriReq3 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
//
//	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？
//		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
//		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
//	}
//	else{
//		CreUseLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 利用明細ログ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
//		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
//	}
//}
///*[]-----------------------------------------------------------------------[]*/
///*|		ｸﾚｼﾞｯﾄ未送信売上依頼ﾃﾞｰﾀ印字要求ﾒｯｾｰｼﾞ受信処理						|*/
///*[]-----------------------------------------------------------------------[]*/
///*|	MODULE NAME		:	PrnCRE_UNSEND( *msg )								|*/
///*|																			|*/
///*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
///*|																			|*/
///*|	RETURN VALUE	:	void												|*/
///*|																			|*/
///*[]-----------------------------------------------------------------------[]*/
///*|	Date	:	2013-07-01													|*/
///*|	Update	:																|*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]*/
//void	PrnCRE_UNSEND( MSG *msg )
//{
//	T_FrmUnSendPriReq	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
//
//	msg_data = (T_FrmUnSendPriReq *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
//
//	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？
//		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
//		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
//	}
//	else{
//		CreUnSend_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 売上依頼ﾃﾞｰﾀ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
//		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
//	}
//}
///*[]-----------------------------------------------------------------------[]*/
///*|		ｸﾚｼﾞｯﾄ売上拒否ﾃﾞｰﾀ印字要求ﾒｯｾｰｼﾞ受信処理							|*/
///*[]-----------------------------------------------------------------------[]*/
///*|	MODULE NAME		:	PrnCRE_SALENG( *msg )								|*/
///*|																			|*/
///*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
///*|																			|*/
///*|	RETURN VALUE	:	void												|*/
///*|																			|*/
///*[]-----------------------------------------------------------------------[]*/
///*|	Date	:	2013-07-01													|*/
///*|	Update	:																|*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]*/
//void	PrnCRE_SALENG( MSG *msg )
//{
//	T_FrmSaleNGPriReq	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
//
//	msg_data = (T_FrmSaleNGPriReq *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
//
//	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？
//		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
//		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
//	}
//	else{
//		CreSaleNG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 売上拒否ﾃﾞｰﾀ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
//		End_Set( msg, terget_pri );				// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
//	}
//}
// MH810103 GG119202(E) 不要機能削除(センタークレジット)

// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
// #ifdef	FUNCTION_MASK_EDY
// /*[]-----------------------------------------------------------------------[]*/
// /*|		Edy設定&ｽﾃｰﾀｽﾌﾟﾘﾝﾄ印字要求ﾒｯｾｰｼﾞ受信処理　							|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	MODULE NAME		:	PrnEdy_Status( *msg )								|*/
// /*|																			|*/
// /*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
// /*|																			|*/
// /*|	RETURN VALUE	:	void												|*/
// /*|																			|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	Author	:	T.Namioka													|*/
// /*|	Date	:	2005-07-27													|*/
// /*|	Update	:																|*/
// /*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
//void	PrnEdy_Status( MSG *msg )
//{
//	T_FrmLogPriReq1		*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
//
//
//	msg_data = (T_FrmLogPriReq1 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
//
//
//	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？
//
//		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
//		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
//	}
//	else{
//
//		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？
//
//			case	R_PRI:		// ﾚｼｰﾄ
//
//				Edy_Status_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
//				break;
//
//			case	J_PRI:		// ｼﾞｬｰﾅﾙ
//
//				Edy_Status_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
//				break;
//
//			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
//
//				Edy_Status_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
//				Edy_Status_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
//				break;
//
//			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
//				return;
//		}
//
//		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
//	}
//}
//
// /*[]-----------------------------------------------------------------------[]*/
// /*|		Edyｱﾗｰﾑﾚｼｰﾄ印字要求ﾒｯｾｰｼﾞ受信処理	　　							|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	MODULE NAME		:	PrnEdyArmPri( *msg )								|*/
// /*|																			|*/
// /*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
// /*|																			|*/
// /*|	RETURN VALUE	:	void												|*/
// /*|																			|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	Author	:	T.Namioka													|*/
// /*|	Date	:	2006-10-31													|*/
// /*|	Update	:																|*/
// /*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
//void	PrnEdyArmPri( MSG *msg )
//{
//	T_FrmEdyArmReq	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
//	edy_arm_log		*pribuf;		// 領収証印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
//
//
//	msg_data = (T_FrmEdyArmReq *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
//	pribuf = msg_data->priedit;				// 領収証印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
//
//	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？
//
//		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
//		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
//	}
//	else{
//
//		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？
//
//			case	R_PRI:		// ﾚｼｰﾄ
//
//				Edy_Arm_edit( pribuf, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｱﾗｰﾑﾚｼｰﾄ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
//				break;
//
//			case	J_PRI:		// ｼﾞｬｰﾅﾙ
//
//				Edy_Arm_edit( pribuf, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｱﾗｰﾑﾚｼｰﾄ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
//				break;
//
//			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
//
//				Edy_Arm_edit( pribuf, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｱﾗｰﾑﾚｼｰﾄ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
//				Edy_Arm_edit( pribuf, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｱﾗｰﾑﾚｼｰﾄ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
//				break;
//
//			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
//				return;
//		}
//
//		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
//	}
//}
//
// /*[]-----------------------------------------------------------------------[]*/
// /*|		アラーム取引情報印字要求ﾒｯｾｰｼﾞ受信処理								|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	MODULE NAME		:	PrnEDYARMPAY_LOG( *msg )							|*/
// /*|																			|*/
// /*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
// /*|																			|*/
// /*|	RETURN VALUE	:	void												|*/
// /*|																			|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	Author	:	suzuki														|*/
// /*|	Date	:	2006-16-15													|*/
// /*|	Update	:																|*/
// /*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
//void	PrnEDYARMPAY_LOG( MSG *msg )
//{
//	T_FrmLogPriReq2	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
//
//
//	msg_data = (T_FrmLogPriReq2 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
//
//	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？
//
//		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
//		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
//	}
//	else{
//
//		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？
//
//			case	R_PRI:		// ﾚｼｰﾄ
//
//				EDYARMPAYLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 個別精算情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
//				break;
//
//			case	J_PRI:		// ｼﾞｬｰﾅﾙ
//
//				EDYARMPAYLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 個別精算情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
//				break;
//
//			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
//
//				EDYARMPAYLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 個別精算情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
//				EDYARMPAYLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 個別精算情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
//				break;
//
//			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
//				return;
//		}
//
//		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
//	}
//}
//
// /*[]-----------------------------------------------------------------------[]*/
// /*|		Ｔ合計後のEdy締め記録印字要求ﾒｯｾｰｼﾞ受信処理							|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	MODULE NAME		:	PrnEdyShimePri( *msg )								|*/
// /*|																			|*/
// /*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
// /*|																			|*/
// /*|	RETURN VALUE	:	void												|*/
// /*|																			|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	Author	:	Suzuki														|*/
// /*|	Date	:	2006-12-15													|*/
// /*|	Update	:																|*/
// /*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
//void	PrnEdyShimePri( MSG *msg )
//{
//	T_FrmEdyShimeReq	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
//	edy_shime_log		*pribuf;		// 領収証印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰ
//
//
//	msg_data = (T_FrmEdyShimeReq *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
//	pribuf = msg_data->priedit;					// Edy締め記録印字ﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
//
//	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？
//
//		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
//		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
//	}
//	else{
//
//		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？
//
//			case	R_PRI:		// ﾚｼｰﾄ
//
//				Edy_Shime_edit( pribuf, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｱﾗｰﾑﾚｼｰﾄ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
//				break;
//
//			case	J_PRI:		// ｼﾞｬｰﾅﾙ
//
//				Edy_Shime_edit( pribuf, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｱﾗｰﾑﾚｼｰﾄ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
//				break;
//
//			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
//
//				Edy_Shime_edit( pribuf, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｱﾗｰﾑﾚｼｰﾄ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
//				Edy_Shime_edit( pribuf, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｱﾗｰﾑﾚｼｰﾄ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
//				break;
//
//			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
//				return;
//		}
//
//		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
//	}
//}
//
// /*[]-----------------------------------------------------------------------[]*/
// /*|		Ｅｄｙ締め記録情報印字要求ﾒｯｾｰｼﾞ受信処理							|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	MODULE NAME		:	PrnEDYSHIME_LOG( *msg )								|*/
// /*|																			|*/
// /*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
// /*|																			|*/
// /*|	RETURN VALUE	:	void												|*/
// /*|																			|*/
// /*[]-----------------------------------------------------------------------[]*/
// /*|	Author	:	suzuki														|*/
// /*|	Date	:	2006-12-15													|*/
// /*|	Update	:																|*/
// /*[]------------------------------------- Copyright(C) 2006 AMANO Corp.----[]*/
//void	PrnEDYSHIME_LOG( MSG *msg )
//{
//	T_FrmLogPriReq2	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
//
//
//	msg_data = (T_FrmLogPriReq2 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
//
//	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？
//
//		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
//		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
//	}
//	else{
//
//		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？
//
//			case	R_PRI:		// ﾚｼｰﾄ
//
//				EDYSHIMELOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );
//				break;
//
//			case	J_PRI:		// ｼﾞｬｰﾅﾙ
//
//				EDYSHIMELOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );
//				break;
//
//			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
//
//				EDYSHIMELOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );
//				EDYSHIMELOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );
//				break;
//
//			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
//				return;
//		}
//
//		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
//	}
//}
//#endif
//
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
/*[]-----------------------------------------------------------------------[]*/
/*|		過電流防止のための24V系ユニット駆動の排他							|*/
/*[]-----------------------------------------------------------------------[]*/
/*| ﾚｼｰﾄﾌﾟﾘﾝﾀ，ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀは起動時に電流ピークとなり、これらを				|*/
/*| 同時に起動した場合、装置電源（24V）のリミットを超えてしまうので、起動	|*/
/*| イベントが同時発生した場合は、後者にdelayを持たせる。					|*/
/*| 各Unit毎に、同時起動禁止時間が異なる。									|*/
/*|																			|*/
/*| ［同時起動禁止時間］													|*/
/*|   ① プリンタ（R&Jの2種）＝ 100ms期間禁止。								|*/
/*|																			|*/
/*| 上記Unitの起動前にこの関数をCallする事とし、起動禁止期間であれば		|*/
/*| ① プリンタは時間経過するまで本関数でwaitする。							|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	Printer_Lifter_Wait( ReqUnit )						|*/
/*|	PARAMETER		:	ReqUnit : 動作開始要求元ユニット					|*/
/*|								    0=レシートプリンタ						|*/
/*|								    1=ジャーナルプリンタ					|*/
/*|	RETURN VALUE	:	1 = 駆動可能										|*/
/*|							ReqUnit = 0 or 1 の場合、戻り値は1固定。		|*/
/*|						0 = 駆動不可										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	namioka														|*/
/*|	Date	:	2007/05/31													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2007 AMANO Corp.----[]*/
#define		PRINT_START_INHIBIT_TIME	10			// ﾌﾟﾘﾝﾀ印字開始後 他Unit動作禁止時間(x20ms) 200ms

uchar Printer_Lifter_Wait( char ReqUnit )
{
	ushort	wkJR_Print_Wait_tim[2];										// ﾌﾟﾘﾝﾀ駆動からの経過時間 (20ms)
	uchar	f_Wait;														// 1=動作禁止期間中
	uchar	ret=1;


	while( 1 ){

		/* タイマ値読出し（割り込み処理と共有するカウンタのため）*/
		wkJR_Print_Wait_tim[0] = (ushort)JR_Print_Wait_tim[0];
		wkJR_Print_Wait_tim[1] = (ushort)JR_Print_Wait_tim[1];
		f_Wait = 0;														// フラグ初期化（動作可能期間）

		/* 要求元ユニット制御部 毎に分岐 */
		switch( ReqUnit ){
		case 0:	// ﾚｼｰﾄﾌﾟﾘﾝﾀ動作開始？
			if( (jnl_proc_data.Printing != 0) &&						// ｼﾞｬｰﾅﾙ動作中で
				(wkJR_Print_Wait_tim[1] < PRINT_START_INHIBIT_TIME) )	// 他ﾕﾆｯﾄ動作禁止時間である
			{
				f_Wait = 1;												// 動作禁止期間とする
			}
			break;

		case 1:	// ｼﾞｬｰﾅﾙﾌﾟﾘﾝﾀ動作開始？
			if( (rct_proc_data.Printing != 0) &&						// ﾚｼｰﾄ動作中で
				(wkJR_Print_Wait_tim[0] < PRINT_START_INHIBIT_TIME) )	// 他ﾕﾆｯﾄ動作禁止時間である
			{
				f_Wait = 1;												// 動作禁止期間とする
			}
			break;

		}

		/* ﾌﾟﾘﾝﾄが待たされる場合の処理 */
		if( f_Wait ){
			xPause_PRNTSK( 2L );										// ﾌﾟﾘﾝﾀﾀｽｸを 20ms wait
		}
		else
			break;
	}
	return	ret;
}

//[]----------------------------------------------------------------------[]
///	@brief			Ｅｄｙ・Ｓｕｉｃａ利用明細ﾃﾞｰﾀ印字要求ﾒｯｾｰｼﾞ受信処理
//[]----------------------------------------------------------------------[]
///	@param[in]		msg				受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ
///	@return			void
///	@author			Y.Ise
///	@attention		None
///	@note			None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/11/11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2006 AMANO Corp.---[]
void	PrnDigi_USE( MSG *msg )
{
	T_FrmLogPriReq4	*msg_data;					// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data = (T_FrmLogPriReq4 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？
		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理(編集ﾃﾞｰﾀｸﾘｱ ＆「印字終了(ｷｬﾝｾﾙ)」ﾒｯｾｰｼﾞ送信)
	}
	else{
// MH810103 MHUT40XX(S) みなし決済の集計を新規に設ける
//// MHUT40XX(S) D.Inaba ICクレジット対応(判定変更)
////		if(msg->command == PREQ_SCA_USE_LOG)
//		if(msg->command == PREQ_SCA_USE_LOG || msg->command == PREQ_EC_USE_LOG)
//// MHUT40XX(E) D.Inaba ICクレジット対応(判定変更)
//		{
//			switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？
//			case	R_PRI:		// ﾚｼｰﾄ
//				SuicaUseLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 利用明細ログ印字処理
//				break;
//
//			case	J_PRI:		// ｼﾞｬｰﾅﾙ
//				SuicaUseLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 利用明細ログ印字処理
//				break;
//
//			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
//				SuicaUseLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 利用明細ログ印字処理
//				SuicaUseLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 利用明細ログ印字処理
//				break;
//
//			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
//				return;
//			}
//		}
		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？
		case	R_PRI:		// ﾚｼｰﾄ
			SuicaUseLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data, (short)msg->command );		// 利用明細ログ印字処理
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ
			SuicaUseLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data, (short)msg->command );		// 利用明細ログ印字処理
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			SuicaUseLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data, (short)msg->command );		// 利用明細ログ印字処理
			SuicaUseLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data, (short)msg->command );		// 利用明細ログ印字処理
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
		}
// MH810103 MHUT40XX(E) みなし決済の集計を新規に設ける
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//		else if(msg->command == PREQ_EDY_USE_LOG)
//		{
//			switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？
//			case	R_PRI:		// ﾚｼｰﾄ
//				EdyUseLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 利用明細ログ印字処理
//				break;
//
//			case	J_PRI:		// ｼﾞｬｰﾅﾙ
//				EdyUseLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 利用明細ログ印字処理
//				break;
//
//			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
//				EdyUseLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 利用明細ログ印字処理
//				EdyUseLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 利用明細ログ印字処理
//				break;
//
//			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
//				return;
//			}
//		}
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
		End_Set( msg, terget_pri );				// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

//[]----------------------------------------------------------------------[]
///	@brief			Ｅｄｙ・Ｓｕｉｃａ集計情報印字要求ﾒｯｾｰｼﾞ受信処理
//[]----------------------------------------------------------------------[]
///	@param[in]		msg			受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ
///	@return			void
///	@author			Y.Ise
///	@attention		None
///	@note			None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2008/11/11<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.---[]
void	PrnDigi_SYU( MSG *msg )
{
	T_FrmLogPriReq2	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data = (T_FrmLogPriReq2 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		if(msg->command == PREQ_SCA_SYU_LOG)
		{
			switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

				case	R_PRI:		// ﾚｼｰﾄ

					SuicaSyuLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 精算中止情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
					break;

				case	J_PRI:		// ｼﾞｬｰﾅﾙ

					SuicaSyuLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 精算中止情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
					break;

				case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

					SuicaSyuLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 精算中止情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
					SuicaSyuLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 精算中止情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
					break;

				default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
					return;
			}
		}
// MH321800(S) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
//#ifdef	FUNCTION_MASK_EDY
//		else if(msg->command == PREQ_EDY_SYU_LOG)
//		{
//			switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？
//
//				case	R_PRI:		// ﾚｼｰﾄ
//
//					EdySyuLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 精算中止情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
//					break;
//
//				case	J_PRI:		// ｼﾞｬｰﾅﾙ
//
//					EdySyuLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 精算中止情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
//					break;
//
//				case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
//
//					EdySyuLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 精算中止情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
//					EdySyuLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 精算中止情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
//					break;
//
//				default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
//					return;
//			}
//		}
//#endif
// MH321800(E) Y.Tanizaki ICクレジット対応 不要機能削除(Edy)
// MH321800(S) D.Inaba ICクレジット対応(決済リーダ日毎集計印字)
		else if(msg->command == PREQ_EC_SYU_LOG)
		{
			switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

				case	R_PRI:		// ﾚｼｰﾄ
					EcSyuLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 精算中止情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
					break;

				case	J_PRI:		// ｼﾞｬｰﾅﾙ

					EcSyuLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 精算中止情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
					break;

				case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

					EcSyuLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 精算中止情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
					EcSyuLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 精算中止情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
					break;

				default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
					return;
			}
		}
// MH321800(E) D.Inaba ICクレジット対応(決済リーダ日毎集計印字)
		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}
/*[]-----------------------------------------------------------------------[]*/
/*|		係員有効データ印字要求ﾒｯｾｰｼﾞ受信処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnAtendValidData( *msg )							|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	K.Motohashi													|*/
/*|	Date	:	2005-05-09													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnAtendValidData( MSG *msg )
{
	T_FrmLogPriReq1	*msg_data;							// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmLogPriReq1 *)msg->data;			// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ


	if( YES == Inji_Cancel_chk( terget_pri ) ){			// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );					// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				AtendValidData_edit(						// 係員有効データ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								R_PRI,
								(PRN_PROC_DATA *)&rct_proc_data
							);
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				AtendValidData_edit(						// 係員無効データ印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								J_PRI,
								(PRN_PROC_DATA *)&jnl_proc_data
							);
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				AtendValidData_edit(						// 係員有効データ印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								R_PRI,
								(PRN_PROC_DATA *)&rct_proc_data
							);

				AtendValidData_edit(						// 係員無効データ印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
								msg_data->Kikai_no,
								msg_data->Kakari_no,
								J_PRI,
								(PRN_PROC_DATA *)&jnl_proc_data
							);

				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}
		End_Set( msg, terget_pri );						// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}
//[]----------------------------------------------------------------------[]
///	@brief			料金設定印字要求ﾒｯｾｰｼﾞ受信処理
//[]----------------------------------------------------------------------[]
///	@param[in]		msg		: 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀｰ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	:	2008/01/15<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	PrnCHARGESETUP( MSG *msg )
{
	T_FrmChargeSetup	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data	= (T_FrmChargeSetup *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	if( YES == Inji_Cancel_chk( terget_pri ) ){		// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );				// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{
		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ
			ChargeSetup_edit(R_PRI, (PRN_PROC_DATA *)&rct_proc_data, msg_data);
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ
			ChargeSetup_edit(J_PRI, (PRN_PROC_DATA *)&jnl_proc_data, msg_data);
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			ChargeSetup_edit(R_PRI, (PRN_PROC_DATA *)&rct_proc_data, msg_data);
			ChargeSetup_edit(J_PRI, (PRN_PROC_DATA *)&jnl_proc_data, msg_data);
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
		}
	}

	End_Set( msg, terget_pri );		// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
}

//[]----------------------------------------------------------------------[]
///	@brief			料金設定印字要求ﾒｯｾｰｼﾞ受信処理
//[]----------------------------------------------------------------------[]
///	@param[in]		msg		: 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀｰ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			Imai
///	@date			Create	:	2008/01/15<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void	PrnSetDiffLogPrn( MSG *msg )
{
	T_FrmLogPriReq1 *msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data	= (T_FrmLogPriReq1 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	if( YES == Inji_Cancel_chk( terget_pri ) ){		// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );				// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{
		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ
			PrnSetDiffLog_edit(msg_data, (PRN_PROC_DATA *)&rct_proc_data, R_PRI);
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ
			PrnSetDiffLog_edit(msg_data, (PRN_PROC_DATA *)&jnl_proc_data, J_PRI);
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			PrnSetDiffLog_edit(msg_data, (PRN_PROC_DATA *)&rct_proc_data, R_PRI);
			PrnSetDiffLog_edit(msg_data, (PRN_PROC_DATA *)&jnl_proc_data, J_PRI);
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
		}
	}

	End_Set( msg, terget_pri );		// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
}

//[]----------------------------------------------------------------------[]
///	@brief			料金設定通信ログ印字要求ﾒｯｾｰｼﾞ受信処理
//[]----------------------------------------------------------------------[]
///	@param[in]		msg		: 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀｰ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			sugata
///	@date			Create	:	2009/10/14<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void PrnRrokin_Setlog(MSG *msg)
{
	T_FrmLogPriReq2	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data	= (T_FrmLogPriReq2 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	if( YES == Inji_Cancel_chk( terget_pri ) ){		// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );				// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{
		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	J_PRI:		// ｼﾞｬｰﾅﾙ
			PrnRrokin_Setlog_edit(msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data);
			break;
		case	R_PRI:		// ﾚｼｰﾄ
			PrnRrokin_Setlog_edit(msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data);
			break;
		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			PrnRrokin_Setlog_edit(msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data);
			PrnRrokin_Setlog_edit(msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data);
			break;
		default:
			break;
		}
	}
	End_Set( msg, terget_pri );		// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理	
}
//[]----------------------------------------------------------------------[]
///	@brief			工場検査用印字処理
//[]----------------------------------------------------------------------[]
///	@param[in]		msg		: 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀｰ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			sugata
///	@date			Create	: 2011/02/21<br>
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void PrnSettei_Chk(MSG *msg)
{
	T_FrmAnyData *msg_data;
	msg_data	= (T_FrmAnyData *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	if( YES == Inji_Cancel_chk( terget_pri ) ){		// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );				// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{
		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	J_PRI:		// ｼﾞｬｰﾅﾙ
			PrnSettei_Chk_edit(msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data);
			break;
		case	R_PRI:		// ﾚｼｰﾄ
			PrnSettei_Chk_edit(msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data);
			break;
		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			PrnSettei_Chk_edit(msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data);
			PrnSettei_Chk_edit(msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data);
			break;
		default:
			break;
		}
	}
	End_Set( msg, terget_pri );		// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理	
	
}
//[]----------------------------------------------------------------------[]
///	@brief			フラップループデータ印字要求ﾒｯｾｰｼﾞ受信処理
//[]----------------------------------------------------------------------[]
///	@param[in]		msg		: 受信ﾒｯｾｰｼﾞﾊﾞｯﾌｧﾎﾟｲﾝﾀｰ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@author			sugata
///	@date			Create	:	2009/10/14<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2008 AMANO Corp.---[]
void PrnFlapLoopData(MSG *msg)
{
	T_FrmAnyData	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data	= (T_FrmAnyData *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	if( YES == Inji_Cancel_chk( terget_pri ) ){		// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );				// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{
		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	J_PRI:		// ｼﾞｬｰﾅﾙ
			PrnFlapLoopData_edit(msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data);
			break;
		case	R_PRI:		// ﾚｼｰﾄ
			PrnFlapLoopData_edit(msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data);
			break;
		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			PrnFlapLoopData_edit(msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data);
			PrnFlapLoopData_edit(msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data);
			break;
		default:
			break;
		}
	}
	End_Set( msg, terget_pri );		// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理	
}
//[]-----------------------------------------------------------------------[]*/
//|		料金テスト情報印字要求ﾒｯｾｰｼﾞ受信処理									|*/
//[]-----------------------------------------------------------------------[]*/
//|	MODULE NAME		:	RT_PrnSEISAN_LOG( *msg )							|*/
//|																			|*/
//|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
//|																			|*/
//|	RETURN VALUE	:	void												|*/
//|																			|*/
//[]-----------------------------------------------------------------------[]*/
//|	Author	:	Y.Shiraishi													|*/
//|	Date	:	2012-10-15													|*/
//|	Update	:																|*/
//[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	RT_PrnSEISAN_LOG( MSG *msg )
{
	T_FrmLogPriReq1	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmLogPriReq1 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				RT_SEISANLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 個別精算情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				RT_SEISANLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 個別精算情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				RT_SEISANLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 個別精算情報印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				RT_SEISANLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 個別精算情報印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}

		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}
void	PrnMntStack(MSG *msg)
{
	T_FrmLogPriReq1		*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ


	msg_data = (T_FrmLogPriReq1 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ


	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				Mnt_Stack_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				Mnt_Stack_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				Mnt_Stack_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				Mnt_Stack_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}

		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
	
}
void	PrnChkResult(MSG *msg)
{
	T_FrmChk_result		*msg_data;
	msg_data = (T_FrmChk_result *)msg->data;
	
	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				ChkResult_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				ChkResult_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				ChkResult_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				ChkResult_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｴﾗｰ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}

		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
	
}
/*[]-----------------------------------------------------------------------[]*/
/*|		メッセージキューの先頭に挿入										|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnPushFrontMsg( MsgBuf *msg )						|*/
/*|	PARAMETER		:	MsgBuf *msg											|*/
/*|	RETURN VALUE	:														|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	sugata														|*/
/*|	Date	:	2012-04-04													|*/
/*[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]*/
void	PrnPushFrontMsg( MsgBuf *msg )
{
	ulong	ist;													// 現在の割込受付状態

	ist = _di2();
	tcb[PRNTCBNO].event = MSG_SET;
	if( tcb[PRNTCBNO].msg_top == NULL ){
		tcb[PRNTCBNO].msg_top = msg;
		tcb[PRNTCBNO].msg_end = msg;
	}
	else {
		msg->msg_next = (char*)tcb[PRNTCBNO].msg_top;
		tcb[PRNTCBNO].msg_top = msg;
	}
	_ei2( ist );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		遠隔監視情報印字要求ﾃﾞｰﾀﾁｪｯｸ処理										|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnRMONJOU_datachk( *Rmonlog )							|*/
/*|																				|*/
/*|	PARAMETER		:	Rmon_log	*Rmonlog	= 遠隔監視情報ﾃﾞｰﾀﾎﾟｲﾝﾀｰ		|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : 正常											|*/
/*|							NG : 異常											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	T.Nagai															|*/
/*|	Date	:	2014-12-10														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.--------[]*/
uchar	PrnRMONJOU_datachk( Rmon_log *Rmonlog )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果


	//	発生時刻ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&Rmonlog->Date_Time ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	端末機能番号ﾁｪｯｸ
	if( Rmonlog->RmonFuncNo > RMON_INFO_SYU_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	処理番号ﾁｪｯｸ
	if( Rmonlog->RmonProcNo > RMON_INFO_COD_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	処理情報番号ﾁｪｯｸ
	if( Rmonlog->RmonProcInfoNo > RMON_INFO_COD_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	状況番号ﾁｪｯｸ
	if( Rmonlog->RmonStatusNo > RMON_INFO_COD_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	//	状況詳細番号ﾁｪｯｸ
	if( Rmonlog->RmonStatusDetailNo > RMON_INFO_COD_MAX ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	
	return( ret );
}

/*[]---------------------------------------------------------------------------[]*/
/*|		遠隔監視情報履歴印字要求ﾃﾞｰﾀﾁｪｯｸ処理									|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnRMONLOG_datachk( *msg_data )							|*/
/*|																				|*/
/*|	PARAMETER		:	T_FrmLogPriReq1		*msg_data = ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ		|*/
/*|																				|*/
/*|	RETURN VALUE	:	uchar	ret												|*/
/*|							OK : 正常											|*/
/*|							NG : 異常											|*/
/*|																				|*/
/*[]---------------------------------------------------------------------------[]*/
/*|	Author	:	T.Nagai															|*/
/*|	Date	:	2014-12-10														|*/
/*|	Update	:																	|*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.--------[]*/
uchar	PrnRMONLOG_datachk( T_FrmLogPriReq2 *msg_data )
{
	uchar	ret = OK;	// ﾁｪｯｸ結果

	//	指定日時ﾁｪｯｸ
	if( NG == DateChk( (date_time_rec *)&msg_data->TSttTime ) ){
		Prn_errlg( ERR_PRNT_DATA_ERR, PRN_ERR_ONOFF, PRN_ERR_JOHO_ARI );	// 印字要求ﾃﾞｰﾀｴﾗｰ登録
		ret = NG;
	}
	return( ret );
}

/*[]-----------------------------------------------------------------------[]*/
/*|		遠隔監視情報印字要求ﾒｯｾｰｼﾞ受信処理									|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnRMON_JOU( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	T.Nagai														|*/
/*|	Date	:	2014-12-10													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.----[]*/
void	PrnRMON_JOU( MSG *msg )
{
	T_FrmRmonJou		*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	Rmon_log			*Rmonlog;		// ﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data	= (T_FrmRmonJou *)msg->data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	Rmonlog		= msg_data->prn_data;				// ﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	PrnRMONJOU_datachk( Rmonlog );					// ﾃﾞｰﾀﾁｪｯｸ

	switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ

			RMONJOU_edit( Rmonlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ

			RMONJOU_edit( Rmonlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

			RMONJOU_edit( Rmonlog, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );	// 印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			RMONJOU_edit( Rmonlog, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );	// 印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
	}

	End_Set( msg, terget_pri );												// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
}

/*[]-----------------------------------------------------------------------[]*/
/*|		遠隔監視情報履歴印字要求ﾒｯｾｰｼﾞ受信処理								|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	MODULE NAME		:	PrnRMON_LOG( *msg )									|*/
/*|																			|*/
/*|	PARAMETER		:	MSG	*msg	=	受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ					|*/
/*|																			|*/
/*|	RETURN VALUE	:	void												|*/
/*|																			|*/
/*[]-----------------------------------------------------------------------[]*/
/*|	Author	:	T.Nagai														|*/
/*|	Date	:	2014-12-10													|*/
/*|	Update	:																|*/
/*[]------------------------------------- Copyright(C) 2014 AMANO Corp.----[]*/
void	PrnRMON_LOG( MSG *msg )
{
	T_FrmLogPriReq2		*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data = (T_FrmLogPriReq2 *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( next_prn_msg == OFF ){					// 新規の印字要求？

		//	新規の印字要求の場合
		PrnRMONLOG_datachk( msg_data );			// 遠隔監視情報履歴印字要求ﾃﾞｰﾀﾁｪｯｸ
	}

	if( YES == Inji_Cancel_chk( terget_pri ) ){	// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else{

		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

			case	R_PRI:		// ﾚｼｰﾄ

				RMONLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 遠隔監視情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				break;

			case	J_PRI:		// ｼﾞｬｰﾅﾙ

				RMONLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 遠隔監視情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

				RMONLOG_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 遠隔監視情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
				RMONLOG_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 遠隔監視情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
				break;

			default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
				return;
		}

		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}
// MH321800(S) D.Inaba ICクレジット対応
//[]----------------------------------------------------------------------[]
///	@brief			決済リーダアラームレシート印字要求ﾒｯｾｰｼﾞ受信処理
//[]----------------------------------------------------------------------[]
///	@param[in]		msg			受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ
///	@return			void
///	@author			Inaba
///	@attention		None
///	@note			PrnSandenAlmRctPri移植
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/02/14<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	PrnEcAlmRctPri( MSG *msg )
{
	T_FrmEcAlmRctReq	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data = (T_FrmEcAlmRctReq *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if (YES == Inji_Cancel_chk(terget_pri)) {		// 印字中止ﾒｯｾｰｼﾞ受信済み？
		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );				// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else {
	// ※現状ジャーナルのみ
		switch (terget_pri) {	// 印字先ﾌﾟﾘﾝﾀ？
		case	R_PRI:		// ﾚｼｰﾄ
			Ec_AlmRct_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｱﾗｰﾑﾚｼｰﾄ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ
			Ec_AlmRct_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｱﾗｰﾑﾚｼｰﾄ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			Ec_AlmRct_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// ｱﾗｰﾑﾚｼｰﾄ情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			Ec_AlmRct_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// ｱﾗｰﾑﾚｼｰﾄ情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
		}
		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

//[]----------------------------------------------------------------------[]
//	@brief			決済リーダ ブランド状態印字要求ﾒｯｾｰｼﾞ受信処理
//[]----------------------------------------------------------------------[]
//	@param[in]		msg			受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ
//	@return			void
//	@author			Inaba
//	@attention		None
//	@note			PrnSandenBrandPri移植
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/02/14<br>
//					Update	:	
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
void	PrnEcBrandPri( MSG *msg )
{
	T_FrmEcBrandReq	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ

	msg_data = (T_FrmEcBrandReq *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if( YES == Inji_Cancel_chk( terget_pri ) ){		// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );				// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	/* 対象が機械№と締め時刻しかないので、データチェック処理はしない */
	else{
		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ

			Ec_Brand_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 決済リーダブランド状態印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ

			Ec_Brand_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 決済リーダブランド状態印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ

			Ec_Brand_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 決済リーダブランド状態印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
			Ec_Brand_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 決済リーダブランド状態印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
		}
		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}

//[]----------------------------------------------------------------------[]
//	@brief			処理未了取引ログ印字要求ﾒｯｾｰｼﾞ受信処理
//[]----------------------------------------------------------------------[]
//	@param[in]		msg			受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ
//	@return			void
//	@author			Inaba
//	@attention		None
//	@note			PrnSandenAlmRctLogPri移植
//[]----------------------------------------------------------------------[]
//	@date			Create	:	2019/02/14<br>
//					Update	:	
//[]------------------------------------- Copyright(C) 2014 AMANO Corp.---[]
void	PrnEcAlmRctLogPri( MSG *msg )
{
	T_FrmEcAlarmLog	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	
	msg_data = (T_FrmEcAlarmLog *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	
	if( YES == Inji_Cancel_chk( terget_pri ) ){		// 印字中止ﾒｯｾｰｼﾞ受信済み？

		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );				// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	// ※レシートのみ
	else{
		switch( terget_pri ){	// 印字先ﾌﾟﾘﾝﾀ？

		case	R_PRI:		// ﾚｼｰﾄ
			Ec_AlmRctLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 処理未了取引ログ印字ﾃﾞｰﾀ編集処理（ﾚｼｰﾄ）
			break;
			
		case	J_PRI:		// ｼﾞｬｰﾅﾙ
			Ec_AlmRctLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 処理未了取引ログ印字ﾃﾞｰﾀ編集処理（ｼﾞｬｰﾅﾙ）
			break;
			
		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			Ec_AlmRctLog_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 処理未了取引ログ印字ﾃﾞｰﾀ編集処理（ﾚｼｰﾄ）
			Ec_AlmRctLog_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// 処理未了取引ログ印字ﾃﾞｰﾀ編集処理（ｼﾞｬｰﾅﾙ）
			break;
			
		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
		}
		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
		if (rct_proc_data.Prn_no_wk[0] == 2) {
			queset(OPETCBNO, OPE_EC_ALARM_LOG_PRINT_1_END, 0, NULL);					// 処理未了取引ログ1件印字完了
		}
	}
}

// MH810103 GG119202(S) ICクレジットみなし決済印字フォーマット変更対応
////[]----------------------------------------------------------------------[]
/////	@brief			決済リーダみなし決済復電ログ印字要求ﾒｯｾｰｼﾞ受信処理
////[]----------------------------------------------------------------------[]
/////	@param[in]		msg			受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ
/////	@return			void
/////	@author			Inaba
/////	@attention		None
/////	@note			PrnSandenAlmRctPri移植
////[]----------------------------------------------------------------------[]
/////	@date			Create	:	2019/02/14<br>
/////					Update	:	
////[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
//void	PrnEcDeemedJnlPri( MSG *msg )
//{
//	T_FrmEcDeemedJnlReq	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
//
//	msg_data = (T_FrmEcDeemedJnlReq *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
//
//	if (YES == Inji_Cancel_chk(terget_pri)) {		// 印字中止ﾒｯｾｰｼﾞ受信済み？
//		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
//		Inji_Cancel( msg, terget_pri );				// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
//	}
//	else {
//	// ※現状ジャーナルのみ
//		switch (terget_pri) {	// 印字先ﾌﾟﾘﾝﾀ？
//		case	R_PRI:		// ﾚｼｰﾄ
//			Ec_DeemedJnl_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// みなし決済復電情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
//			break;
//
//		case	J_PRI:		// ｼﾞｬｰﾅﾙ
//			Ec_DeemedJnl_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// みなし決済復電情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
//			break;
//
//		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
//			Ec_DeemedJnl_edit( msg_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// みなし決済復電情報履歴印字ﾃﾞｰﾀ編集（ﾚｼｰﾄ）
//			Ec_DeemedJnl_edit( msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// みなし決済復電情報履歴印字ﾃﾞｰﾀ編集（ｼﾞｬｰﾅﾙ）
//			break;
//
//		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
//			return;
//		}
//		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
//	}
//}
// MH810103 GG119202(E) ICクレジットみなし決済印字フォーマット変更対応

//[]----------------------------------------------------------------------[]
///	@brief			決済精算中止（復決済）印字
//[]----------------------------------------------------------------------[]
///	@param[in]		msg			受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ
///	@return			void
///	@author			Inaba
///	@attention		None
///	@note			
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2019/10/02<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2019 AMANO Corp.---[]
void	PrnRecvDeemedDataPri( MSG *msg )
{
	T_FrmReceipt	*msg_data;		// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰ
	Receipt_data	*msg_data_receipt;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀ内のReceipt_dataﾎﾟｲﾝﾀｰ

	msg_data = (T_FrmReceipt *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ
	msg_data_receipt = (Receipt_data *)msg_data->prn_data;	// Receipt_dataﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if (YES == Inji_Cancel_chk(terget_pri)) {		// 印字中止ﾒｯｾｰｼﾞ受信済み？
		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );				// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else {
	// ※現状ジャーナルのみ
		switch (terget_pri) {	// 印字先ﾌﾟﾘﾝﾀ？
		case	R_PRI:		// ﾚｼｰﾄ
			Recv_DeemedData_edit( msg_data_receipt, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// みなし決済データ（復決済）（ﾚｼｰﾄ）
			break;

		case	J_PRI:		// ｼﾞｬｰﾅﾙ
			Recv_DeemedData_edit( msg_data_receipt, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// みなし決済データ（復決済）（ｼﾞｬｰﾅﾙ）
			break;

		case	RJ_PRI:		// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			Recv_DeemedData_edit( msg_data_receipt, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// みなし決済データ（復決済）（ﾚｼｰﾄ）
			Recv_DeemedData_edit( msg_data_receipt, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data );		// みなし決済データ（復決済）（ｼﾞｬｰﾅﾙ）
			break;

		default:			// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
		}
		End_Set( msg, terget_pri );					// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}
// MH321800(E) D.Inaba ICクレジット対応

// MH810105(S) R.Endo 2021/12/01 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
//[]----------------------------------------------------------------------[]
///	@brief			デバッグ用印字
//[]----------------------------------------------------------------------[]
///	@param[in]		msg			受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ
///	@return			void
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/12/01<br>
///					Update	:	
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
void	PrnDEBUG( MSG *msg )
{
	T_FrmDebugData *msg_data = (T_FrmDebugData *)msg->data;	// 受信ﾒｯｾｰｼﾞﾃﾞｰﾀﾎﾟｲﾝﾀｰｾｯﾄ

	if ( YES == Inji_Cancel_chk(terget_pri) ) {	// 印字中止ﾒｯｾｰｼﾞ受信済み？
		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel(msg, terget_pri);			// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	} else {
		switch( terget_pri ) {	// 印字先ﾌﾟﾘﾝﾀ？
		case J_PRI:				// ｼﾞｬｰﾅﾙ
			PrnDEBUG_edit(msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data);
			break;
		case RJ_PRI:			// ﾚｼｰﾄ＆ｼﾞｬｰﾅﾙ
			PrnDEBUG_edit(msg_data, J_PRI, (PRN_PROC_DATA *)&jnl_proc_data);
			break;
		case R_PRI:				// ﾚｼｰﾄ
		default:
			return;
		}
	}

	End_Set(msg, terget_pri);	// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
}
// MH810105(E) R.Endo 2021/12/01 車番チケットレス3.0 #6184 【現場指摘】精算後にE2610-224エラー頻発
// MH810105(S) MH364301 QRコード決済対応
//[]----------------------------------------------------------------------[]
///	@brief			障害連絡票印字
//[]----------------------------------------------------------------------[]
///	@param[in]		msg			受信ﾒｯｾｰｼﾞﾎﾟｲﾝﾀｰ
///	@return			void
///	@author			Sato
///	@attention		None
///	@note			None
//[]----------------------------------------------------------------------[]
///	@date			Create	:	2021/11/12<br>
///					Update	:
//[]------------------------------------- Copyright(C) 2021 AMANO Corp.---[]
void	PrnFailureContactDataPri( MSG *msg )
{
	T_FrmReceipt	*FailureContactData;

	FailureContactData = (T_FrmReceipt *)msg->data;				// 障害連絡票印字ﾃﾞｰﾀ

	if (YES == Inji_Cancel_chk(terget_pri)) {					// 印字中止ﾒｯｾｰｼﾞ受信済み？
		//	印字中止ﾒｯｾｰｼﾞ受信済みの場合
		Inji_Cancel( msg, terget_pri );							// 印字中止処理（編集処理制御ﾃﾞｰﾀｸﾘｱ＆「印字終了（ｷｬﾝｾﾙ）」ﾒｯｾｰｼﾞ送信）
	}
	else {
		switch (terget_pri) {	// 印字先ﾌﾟﾘﾝﾀ？
		case	R_PRI:			// ﾚｼｰﾄ
			Recv_FailureContactData_edit( FailureContactData->prn_data, R_PRI, (PRN_PROC_DATA *)&rct_proc_data );		// 障害連絡票（ﾚｼｰﾄ）
			break;
		default:				// その他（ﾌﾟﾘﾝﾀ種別ｴﾗｰ）
			return;
		}
		End_Set( msg, terget_pri );								// 印字ﾃﾞｰﾀ１ﾌﾞﾛｯｸ編集終了時処理
	}
}
// MH810105(E) MH364301 QRコード決済対応
