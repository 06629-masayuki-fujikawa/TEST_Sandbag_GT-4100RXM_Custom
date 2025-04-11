//[]-----------------------------------------------------------------------[]
//| System      : FT4000													|
//| Module      : クレジット制御関数群										|
//| 			  OPETASK から関数コールされる。							|
//[]-----------------------------------------------------------------------[]
//| Date        : 2013-07-01												|
//| Update      :															|
//[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>
#include	"iodefine.h"
#include	"system.h"
#include	"LKmain.h"
#include	"flp_def.h"
#include	"oiban.h"
#include	"common.h"
#include	"mem_def.h"
#include	"Message.h"
#include	"tbl_rkn.h"
#include	"ope_def.h"
#include	"lcd_def.h"
#include	"mdl_def.h"
#include	"prm_tbl.h"
#include	"raudef.h"
#include	"ntnet_def.h"
#include	"cre_ctrl.h"
#include	"aes_sub.h"
#include	"ksg_def.h"

// MH810103 GG119202(S) 不要機能削除(センタークレジット)
////
//// 制御エリア実体宣言
////
//td_cre_ctl			cre_ctl;			// 制御エリア
//td_creSeisanInfo	creSeisanInfo;		// 精算情報
//extern CRE_SALENG	cre_saleng_work;	// 売上拒否ﾃﾞｰﾀ１０件分編集用ワーク
//
//static	ulong	creConvAtoB( uchar *asc_buf, ushort len );
//
//
//#define	ON_OFF	2
//
////[]-----------------------------------------------------------------------[]
////| クレジットOPE制御メイン													|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creCtrl( event )											|
////| PARAMETER    : event : 起動要因											|
////| RETURN VALUE : 起動要因別の戻り値,戻り値に意味の無いものは 0 固定		|
////|				   creSeisanInfo = 精算情報									|
////|				   cre_ctl.RcvData = 受信データ								|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creCtrl( short event )
//{
//	short	ret = 0;
//
//	cre_ctl.Event = event;
//	switch( event ){
//	case	CRE_EVT_SEND_OPEN:			// 開局コマンド(01)
//		ret = creSendData_OPEN();
//		break;
//	case	CRE_EVT_SEND_CONFIRM:		// 与信問合せデータ送信要求(03)
//		ret = creSendData_CONFIRM();
//		break;
//	case	CRE_EVT_SEND_SALES:			// 売上依頼データ送信要求(05)
//		ret = creSendData_SALES();
//		break;
//	case	CRE_EVT_SEND_ONLINETEST:	// ｵﾝﾗｲﾝﾃｽﾄ送信要求(07)
//		ret = creSendData_ONLINETEST();
//		break;
//	case	CRE_EVT_SEND_RETURN:		// 返品問合せデータ送信要求(09)
//		ret = creSendData_RETURN();
//		break;
//	case	CRE_EVT_TIME_UP:			// ﾀｲﾑｱｳﾄ
//		ret = creTimeOut();
//		break;
//	case	CRE_EVT_RECV_ANSWER:		// 応答受信
//		ret = creRecvData();
//		break;
//	default:
//		break;
//	}
//
//	return ret;
//}
//
////[]-----------------------------------------------------------------------[]
////| クレジット制御エリア初期化												|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creCtrlInit( uchar flg )									|
////| PARAMETER    : uchar	flg	: 1=端末処理通番を初期化する				|
////| RETURN VALUE : void														|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creCtrlInit( uchar flg )
//{
//	if( flg == 1 ) {
//		cre_slipno = 0;								// 端末処理通番(使用前に加算されるので、初期値は0とする)
//	}
//	
//	memset( &cre_ctl, 0, sizeof(cre_ctl) );
//
//	// 開局コマンドリトライ制御データ初期化
//	creOpen_Init();
//
//	// 売上依頼データリトライ制御データ初期化
//	creSales_Init();
//
//	// 生存確認の制御データ初期化
//	creOnlineTest_Init();
//
//	// 応答データ受信タイムアウト（秒）
//	cre_ctl.Result_Wait = (uchar)prm_get( COM_PRM, S_CRE, 5, 3, 1 );	// 出口機の推奨値：10秒
//	if( cre_ctl.Result_Wait < 1 || cre_ctl.Result_Wait > 99 ){
//		cre_ctl.Result_Wait = 10;					// 設定値が範囲外の場合は１０秒とする
//	}
//	
//	if( cre_saleng.ken >= CRE_SALENG_MAX ) {
//		cre_ctl.Credit_Stop = 1;					// 売上拒否データが上限の場合はクレジット精算不可
//	}
//}
//
////[]-----------------------------------------------------------------------[]
////| クレジット精算可／不可チェック											|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creStatusCheck											|
////| PARAMETER    : void														|
////| RETURN VALUE : short													|
////|					0:精算可 / 1:通信中 / 2:売上収集未完了					|
////|					-1:ｸﾚｼﾞｯﾄｻｰﾊﾞｰ接続なし / -2:停止中 / -3:未開局			|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]-----------------------------------------------------------------------[]
////| クレジットサーバーへ精算要求が可能かチェックする  						|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//uchar	creStatusCheck( void )
//{
//	uchar	retsts = CRE_STATUS_OK;
//
//	if( ! CREDIT_ENABLED() ){
//		retsts |= CRE_STATUS_DISABLED;		//接続なし.or.イニシャル未完了
//	}
//	if( cre_ctl.Initial_Connect_Done == 0 ){
//		retsts |= CRE_STATUS_NOTINIT;		//初回接続未完了
//	}
//	if( cre_ctl.Credit_Stop ){
//		retsts |= CRE_STATUS_STOP;			//停止中
//	}
//	if( cre_uriage.UmuFlag == ON ){
//		retsts |= CRE_STATUS_UNSEND;		//売上依頼(05)未送信
//	}
//	if( cre_ctl.Status != CRE_STS_IDLE ){
//		retsts |= CRE_STATUS_SENDING;		//通信処理中
//	}
//	if( KSG_gPpp_RauStarted == 0 ) {		//PPP未接続
//		retsts |= CRE_STATUS_PPP_DISCONNECT;
//	}
//
//	return retsts;
//}
//
////[]-----------------------------------------------------------------------[]
////| 精算金額の限度額以下かチェックする										|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creLimitCheck											|
////| PARAMETER    : Amount : 精算しようとしている金額						|
////| RETURN VALUE : 0 : 以下 / -1 : 超えている								|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creLimitCheck( ulong Amount )
//{
//	ulong	Limit;
//	Limit = (ulong)prm_get( COM_PRM, S_CRE, 2, 6, 1 );
//
//	if( Limit != 0 && Amount > Limit ){
//		return -1;
//	}
//	return 0;
//}
//
////[]-----------------------------------------------------------------------[]
////| １分周期処理															|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creMinCyclCheckProc   	                           		|
////| PARAMETER    : void                                                  	|
////| RETURN VALUE : void														|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]-----------------------------------------------------------------------[]
////| ｵﾝﾗｲﾝﾃｽﾄを送信する必要がある場合、送信する								|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creOneMinutesCheckProc( void )
//{
//	short	do_flag = 0;
//
//	if( ! CREDIT_ENABLED() ){
//		return;		// 未接続.or.接続なし
//	}
//
//	// 生存確認の経過時間をカウント
//	creOnlineTest_Count();
//
//	if( cre_ctl.Status != CRE_STS_IDLE ){
//		return; 	// 通信処理中
//	}
//	// 初回接続未完了
//	if( cre_ctl.Initial_Connect_Done == 0 ){
//		// 開局コマンドの経過時間をチェック
//		if( creOpen_Check() != 0 ){
//			do_flag = 1;
//		}
//		goto LSEND_CHECK;	// 初回接続完了するまでは以降のチェックは不要
//	}
//	// 売上依頼結果(06)データ未受信時（05送信中）
//	if( cre_uriage.UmuFlag == ON ){
//		// 売上依頼データの経過時間をチェック
//		if( creSales_Check() != 0 ){
//			do_flag = 2;
//		}
//		goto LSEND_CHECK;
//	}
//	// 生存確認の経過時間をチェック
//	if( creOnlineTest_Check() != 0 ){
//		do_flag = 3;
//		goto LSEND_CHECK;
//	}
//
//LSEND_CHECK:
//	switch( do_flag ){
//	case 1:
//		creOpen_Send();				// 開局コマンド(01)送信
//		break;
//	case 2:
//		creSales_Send(CRE_KIND_RETRY);
//		break;
//	case 3:
//		creOnlineTest_Send();		// 生存確認(07)送信
//		break;
//	default:
//		break;
//	}
//}
//
////[]-----------------------------------------------------------------------[]
////| ｵﾝﾗｲﾝﾃｽﾄ送信処理														|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOnlineTestCheck										|
////| PARAMETER    : void                                                  	|
////| RETURN VALUE : short : 1:送信正常受付 1以外:送信不可/失敗				|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]-----------------------------------------------------------------------[]
////| ﾕｰｻﾞｰﾒﾝﾃ　拡張機能　ｸﾚｼﾞｯﾄ接続確認時に呼ばれる							|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creOnlineTestCheck( void )
//{
//	if( ! CREDIT_ENABLED() ){
//		return -1;		// 未接続.or.接続なし
//	}
//	if( cre_ctl.Status != CRE_STS_IDLE ){
//		return -2; 		// 通信処理中
//	}
//
//	cre_ctl.OpenKind = CRE_KIND_MANUAL;		// 開局発生要因に手動を設定
//	return creCtrl( CRE_EVT_SEND_OPEN );	// 開局コマンド(01)
//}
//
////[]-----------------------------------------------------------------------[]
////| クレジット停止要因エラーチェック										|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creErrorCheck											|
////| RETURN VALUE : 0:なし / 1:売上依頼のみ, 2:売上拒否のみ, 3:両方			|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creErrorCheck( void )
//{
//	short	ret = 0;
//
//	// クレジット停止中
//	if( cre_ctl.Credit_Stop ){
//
//		// 要因：売上依頼未送信
//		if( cre_uriage.UmuFlag == ON ){
//			ret += 1;
//		}
//
//		// 要因：売上拒否データフル
//		if( cre_saleng.ken >= CRE_SALENG_MAX ){
//			ret += 2;
//		}
//
//		// 要因：オンラインテストでサーバーより致命的エラーを受信（未対応）
//		;	// 該当処理なし
//
//	}
//	return ret;
//}
//
////[]-----------------------------------------------------------------------[]
////| 01 送信処理																|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSendData_OPEN											|
////| PARAMETER    : void														|
////| RETURN VALUE : 0 : OK  / !=0 NG											|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creSendData_OPEN( void )
//{
//	short	retsts = 0;
//
//	if( cre_ctl.Status != CRE_STS_IDLE ){
//		return -2;	// 通信中
//	}
//
//	Lagcan( OPETCBNO, 9 );
//
//	// モニタ出力
//	creRegMonitor(OPMON_CRE_SEND_COMMAND, CRE_SNDCMD_OPEN, (ulong)cre_ctl.OpenKind);
//
//	//送信データ編集～送信キューに格納
//	retsts = (NTNET_Snd_Data136_01( 0 ) ? 0 : 1);
//
//	cre_ctl.Status = CRE_STS_WAIT_OPEN;		// 状態更新（02待ち）
//	Lagtim( OPETCBNO, 9, (ushort)(50 * cre_ctl.Result_Wait) );
//
//	return retsts;
//}
//
////[]-----------------------------------------------------------------------[]
////| 03 送信処理																|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSendData_CONFIRM										|
////| PARAMETER    : void														|
////| RETURN VALUE : 0 : OK  / !=0 NG											|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
////short	creSendData_DATASEND( void )
//short	creSendData_CONFIRM( void )
//{
//	short	retsts = 0;
//
//	if( cre_ctl.Status != CRE_STS_IDLE ){
//		return -2;	// 通信中
//	}
//
//	Lagcan( OPETCBNO, 9 );
//
//	// モニタ出力
//	creRegMonitor(OPMON_CRE_SEND_COMMAND, CRE_SNDCMD_CONFIRM, 0);
//
//	//送信データ編集～送信キューに格納
//	retsts = (NTNET_Snd_Data136_03() ? 0 : 1);
//
//	cre_ctl.Status = CRE_STS_WAIT_CONFIRM;	// 状態更新（04待ち）
//	Lagtim( OPETCBNO, 9, (ushort)(50 * cre_ctl.Result_Wait) );
//
//	return retsts;
//}
//
////[]-----------------------------------------------------------------------[]
////| 05 送信処理																|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSendData_SALES										|
////| PARAMETER    : void														|
////| RETURN VALUE : 0 : OK  / !=0 NG											|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creSendData_SALES( void )
//{
//	short	retsts = 0;
//
//	if( cre_ctl.Status != CRE_STS_IDLE ){
//		return -2;	// 通信中
//	}
//
//	Lagcan( OPETCBNO, 9 );
//
//	// モニタ出力
//	creRegMonitor(OPMON_CRE_SEND_COMMAND, CRE_SNDCMD_SALES, (ulong)cre_ctl.SalesKind);
//
//	// 送信データ編集～送信キューに格納
//	retsts = (NTNET_Snd_Data136_05( 0 ) ? 0 : 1);
//
//	cre_ctl.Status = CRE_STS_WAIT_SALES;	// 状態更新（06待ち）
//	Lagtim( OPETCBNO, 9, (ushort)(50 * cre_ctl.Result_Wait) );
//
//	return retsts;
//}
//
////[]-----------------------------------------------------------------------[]
////| 07 送信処理																|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSendData_ONLINETEST									|
////| PARAMETER    : void														|
////| RETURN VALUE : 0 : OK  / !=0 NG											|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creSendData_ONLINETEST( void )
//{
//	short	retsts = 0;
//
//	if( cre_ctl.Status != CRE_STS_IDLE ){
//		return -2;	// 通信中
//	}
//
//	Lagcan( OPETCBNO, 9 );
//
//	// モニタ出力
//	creRegMonitor(OPMON_CRE_SEND_COMMAND, CRE_SNDCMD_ONLINETEST, 0);
//
//	// 送信データ編集～送信キューに格納
//	retsts = (NTNET_Snd_Data136_07( 0 ) ? 0 : 1);
//
//	cre_ctl.Status = CRE_STS_WAIT_ONLINETEST;	// 状態更新（08待ち）
//	Lagtim( OPETCBNO, 9, (ushort)(50 * cre_ctl.Result_Wait) );
//
//	return retsts;
//}
//
////[]-----------------------------------------------------------------------[]
////| 09 送信処理																|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSendData_RETURN										|
////| PARAMETER    : void														|
////| RETURN VALUE : 0 : OK  / !=0 NG											|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creSendData_RETURN( void )
//{
//	short	retsts = 0;
//
//	if( cre_ctl.Status != CRE_STS_IDLE ){
//		return -2;	// 通信中
//	}
//
//	Lagcan( OPETCBNO, 9 );
//
//	// モニタ出力
//	creRegMonitor(OPMON_CRE_SEND_COMMAND, CRE_SNDCMD_RETURN, 0);
//
//	// 送信データ編集～送信キューに格納
//	retsts = (NTNET_Snd_Data136_09( 0 ) ? 0 : 1);
//
//	cre_ctl.Status = CRE_STS_WAIT_RETURN;	// 状態更新（0A待ち）
//	Lagtim( OPETCBNO, 9, (ushort)(50 * cre_ctl.Result_Wait) );
//
//	return retsts;
//}
//
////[]-----------------------------------------------------------------------[]
////| 応答受信処理															|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creRecvData												|
////| PARAMETER    : void														|
////| RETURN VALUE : 0 : OK  / !=0 NG											|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creRecvData( void )
//{
//	short retsts = 1;
//	DATA_KIND_137_02	*Recv137_02;
//	DATA_KIND_137_04	*Recv137_04;
//	DATA_KIND_137_06	*Recv137_06;
//	DATA_KIND_137_08	*Recv137_08;
//	DATA_KIND_137_0A	*Recv137_0A;
//	short	ReSendFlg = 0;
//	short	kind = 0;					// 送信要因
//	ulong	result = 0;					// 結果①②
//	short	tout_flg = 0;				// タイムアウト通知あり／なし
//
//	creOnlineTest_Reset();	// 無通信時間リセット
//	err_chk2( ERRMDL_CREDIT, ERR_CREDIT_CONNECTCHECK_ERR, 0x00, 0, 0, NULL );	// エラー(解除)
//
//	cre_ctl.RcvData = Cre_GetRcvNtData();
//
//	Recv137_02 = (DATA_KIND_137_02 *)cre_ctl.RcvData;
//	Recv137_04 = (DATA_KIND_137_04 *)cre_ctl.RcvData;
//	Recv137_06 = (DATA_KIND_137_06 *)cre_ctl.RcvData;
//	Recv137_08 = (DATA_KIND_137_08 *)cre_ctl.RcvData;
//	Recv137_0A = (DATA_KIND_137_0A *)cre_ctl.RcvData;
//
//	switch( Recv137_02->Common.DataIdCode1 ){	// 02 を代表で使う
//	case CRE_RCVCMD_OPEN:
//		result = (ulong)Recv137_02->Result1 * 10000 + (ulong)Recv137_02->Result2;
//		break;
//	case CRE_RCVCMD_CONFIRM:
//		result = (ulong)Recv137_04->Result1 * 10000 + (ulong)Recv137_04->Result2;
//		break;
//	case CRE_RCVCMD_SALES:
//		result = (ulong)Recv137_06->Result1 * 10000 + (ulong)Recv137_06->Result2;
//		break;
//	case CRE_RCVCMD_ONLINETEST:
//		result = (ulong)Recv137_08->Result1 * 10000 + (ulong)Recv137_08->Result2;
//		break;
//	case CRE_RCVCMD_RETURN:
//		result = (ulong)Recv137_0A->Result1 * 10000 + (ulong)Recv137_0A->Result2;
//		break;
//	default:
//		// 想定外の種別は無視
//		return retsts;
//	}
//
//	// フェーズずれ & 追い番チェック
//	retsts = creResultCheck();
//	if( retsts != 0 ){
//		return retsts;	// エラーありの場合は読み捨てる。処理結果①、②がエラーの場合は通す。
//	}
//
//	// コマンド応答受信モニタ登録
//	creRegMonitor(OPMON_CRE_RECV_RESPONSE, (uchar)Recv137_02->Common.DataIdCode1, result);
//
//	//                  //
//	// 受信種別ごと処理 //
//	//                  //
//	switch( Recv137_02->Common.DataIdCode1 ){	// 02 を代表で使う
//	case CRE_RCVCMD_OPEN:		// 開局コマンド応答(02)
//		if( cre_ctl.Status == CRE_STS_WAIT_OPEN ){
//			cre_ctl.Status = CRE_STS_IDLE;
//			retsts = (short)Recv137_02->Result1;
//			if( retsts == 0 ){
//				cre_ctl.Initial_Connect_Done = 1;	// 開局完了
//				err_chk2( ERRMDL_CREDIT, ERR_CREDIT_OPEN_ERR, 0x00, 0, 0, NULL );	// エラー(解除)
//				//売上依頼フラグ ON ?（売上依頼結果(06)未受信）
//				if( cre_uriage.UmuFlag != 0 ){
//					ReSendFlg = 1;		// 売上依頼(05)の再送信
//					kind = CRE_KIND_AUTO;				// 送信要因に自動を設定
//				} else {
//					// 外部照会中フラグON（与信問合せ送信する為に、開局コマンドを送信した）
//					if( OPECTL.InquiryFlg == 1 ){
//						ReSendFlg = 2;		// 与信問合せ(03)の再送信
//					}
//				}
//			}else{
//				// １回目の開局リトライがタイムアウトしたときにエラー登録する
//				if( cre_ctl.Open.Cnt == 1 ){
//					if( cre_ctl.Initial_Connect_Done == 0 ) {	// 未開局の場合のみエラーと登録する
//						err_chk2( ERRMDL_CREDIT, ERR_CREDIT_OPEN_ERR, 0x01, 0, 0, NULL );	// エラー(発生)
//					}
//				}
//				// 開局コマンド送信許可に変更（リトライ時間の計測開始）
//				if( cre_ctl.Initial_Connect_Done == 0 ) {	// 未開局の場合のみリトライする
//					creOpen_Enable();
//				}
//			}
//			queset( OPETCBNO, ( retsts==0 ? CRE_EVT_02_OK : CRE_EVT_02_NG ), 0, 0 );	// OpeMainへ通知
//		}
//		break;
//	case CRE_RCVCMD_CONFIRM:	// 与信問合せ結果データ(04)
//		if( cre_ctl.Status == CRE_STS_WAIT_CONFIRM ){
//			cre_ctl.Status = CRE_STS_IDLE;
//			retsts = (short)Recv137_04->Result1;
//			if( retsts == 0 ){
//				// 集計エリアとその他 更新
//				creUpdatePayData( Recv137_04 );
//				queset( OPETCBNO, CRE_EVT_04_OK, 0, 0 );	// OpeMainへ通知
//			}else{
//				if(
//					( retsts == 3 ) ||
//					( retsts >= 10 && retsts <= 19 ) ||
//					( retsts >= 20 && retsts <= 29 ) ||
//					( retsts >= 30 && retsts <= 39 ) ||
//					( retsts >= 40 && retsts <= 49 )
//				){
//					queset( OPETCBNO, CRE_EVT_04_NG1, 0, 0 );	// OpeMainへ通知
//				}else{
//					queset( OPETCBNO, CRE_EVT_04_NG2, 0, 0 );	// OpeMainへ通知
//				}
//			}
//		}
//		break;
//	case CRE_RCVCMD_SALES:		// 売上依頼結果データ(06)
//		if( cre_ctl.Status == CRE_STS_WAIT_SALES ){
//			cre_ctl.Status = CRE_STS_IDLE;
//			retsts = (short)Recv137_06->Result1;
//			if( retsts == 0 ){
//				creSales_Reset();	// 売上依頼データの経過時間カウントをリセット
//				cre_uriage.UmuFlag = OFF;						// 売上依頼データ(05)送信中ﾌﾗｸﾞON（06受信でOFF）
//				memset( &cre_uriage, 0, sizeof(cre_uriage));	// 売上依頼データエリアをクリア
//				if( creErrorCheck() == 0 ){
//					cre_ctl.Credit_Stop = 0;	// クレジット停止要因がなくなったら解除する
//				}
//			}else{
//				// ＮＧ応答を受信した場合は、売上拒否データに登録する
//				if( Recv137_06->Result1 == 99 ) {				// 処理結果①=99の場合はタイムアウトと同様に売上拒否としない
//					// 処理結果①=99の場合はタイムアウトと同様の処理を行う
//					creRegMonitor(OPMON_CRE_COMMAND_TIMEOUT, CRE_RCVCMD_SALES, 0);	// モニタ登録
//					// 外部照会中フラグONのとき
//					if( OPECTL.InquiryFlg == 1 ){
//						queset( OPETCBNO, CRE_EVT_06_TOUT, 0, 0 );	// OpeMainへ通知（問合せ中の状態を解消する為）
//						tout_flg = 1;				// タイムアウト通知あり／なし
//					}
//				}else{
//					creSaleNG_Add();							// クレジット売上拒否データ登録
//					err_chk2( ERRMDL_CREDIT, ERR_CREDIT_NO_ACCEPT, 0x01, 0, 0, NULL );		// エラー(発生)
//					creSales_Reset();	// 売上依頼データの経過時間カウントをリセット
//					cre_uriage.UmuFlag = OFF;						// 売上依頼データ(05)送信中ﾌﾗｸﾞON（06受信でOFF）
//					memset( &cre_uriage, 0, sizeof(cre_uriage));	// 売上依頼データエリアをクリア
//					switch( creErrorCheck() ){
//					case 0:		// エラーなし
//						cre_ctl.Credit_Stop = 0;	// クレジット停止要因がなくなったら解除する
//						break;
//					case 1:		// 売上依頼未送信
//						;	// 該当処理なし
//						break;
//					case 2:		// 売上拒否データＭＡＸ
//					default:	// 両方
//						// 外部照会中フラグONのとき
//						if( OPECTL.InquiryFlg == 1 ){
//							queset( OPETCBNO, CRE_EVT_06_TOUT, 0, 0 );	// OpeMainへ通知（問合せ中の状態を解消する為）
//							tout_flg = 1;			// タイムアウト通知あり／なし
//						}
//						break;
//					}
//				}
//			}
//			if( cre_ctl.Credit_Stop == 0 ) {	// クレジット停止中でないときのみ与信問合せを送信する
//				if( Recv137_06->Result1 != 99 ) {	// 処理結果①=99以外の場合のみ与信問合せを送信する
//					// 外部照会中フラグON（与信問合せ送信する為に、開局コマンドを送信した）
//					if( OPECTL.InquiryFlg == 1 ){
//						ReSendFlg = 2;		// 与信問合せ(03)の再送信
//					}
//				}
//			}
//			if( tout_flg == 0 ){				// タイムアウト通知なし
//				queset( OPETCBNO, ( retsts==0 ? CRE_EVT_06_OK : CRE_EVT_06_NG ), 0, 0 );	// OpeMainへ通知
//			}
//		}
//		break;
//	case CRE_RCVCMD_ONLINETEST:	// ｵﾝﾗｲﾝﾃｽﾄ応答(08)
//		if( cre_ctl.Status == CRE_STS_WAIT_ONLINETEST ){
//			cre_ctl.Status = CRE_STS_IDLE;
//			retsts = (short)Recv137_08->Result1;
//			if(retsts == 0){
//				// 外部照会中フラグON（分歩進による生存確認中にクレジットカード挿入された場合）
//				if( OPECTL.InquiryFlg == 1 ){
//					ReSendFlg = 2;		// 与信問合せ(03)の送信
//				}
//			}else{
//				//// サーバーで致命的なエラーが発生しているエラーコードがセットされていた場合は、ここでクレジット停止にする。
//				//cre_ctl.Credit_Stop = 1;	// クレジット停止
//			}
//			queset( OPETCBNO, ( retsts==0 ? CRE_EVT_08_OK : CRE_EVT_08_NG ), 0, 0 );	// OpeMainへ通知
//		}
//		break;
//	case CRE_RCVCMD_RETURN:		// 返品問合せ結果データ(0A)
//		if( cre_ctl.Status == CRE_STS_WAIT_RETURN ){
//			cre_ctl.Status = CRE_STS_IDLE;
//			retsts = (short)Recv137_0A->Result1;
//			if( retsts == 0 ){
//				;	// 該当処理なし
//			}else{
//				;	// 該当処理なし
//			}
//			queset( OPETCBNO, ( retsts==0 ? CRE_EVT_0A_OK : CRE_EVT_0A_NG ), 0, 0 );	// OpeMainへ通知
//		}
//		break;
//	}
//	if( cre_ctl.Status == CRE_STS_IDLE ){
//		Lagcan( OPETCBNO, 9 );
//	}
//	switch( ReSendFlg ){
//	case 1:
//		creSales_Send(kind);	// 売上依頼データ再送信(05)
//		break;
//	case 2:
//		creSendData_CONFIRM();	// 与信問合せデータ送信(03)
//		break;
//	}
//
//	return retsts;
//}
//
///*[]-----------------------------------------------------------------------[]*/
///*|  クレジット売上拒否データ  データ登録                                   |*/
///*[]-----------------------------------------------------------------------[]*/
///*| MODULE NAME  : CRE_SaleNG_Delete                                        |*/
///*| PARAMETER    : ofs	= 削除したいデータのテーブル先頭からのオフセット    |*/
///*| RETURN VALUE : none                                                     |*/
///*[]-----------------------------------------------------------------------[]*/
///*| REMARK       : 削除後の先頭データが配列の[0]にくるようデータを構成し直す|*/
///*|                                                                         |*/
///*[]-----------------------------------------------------------------------[]*/
///*| Date         : 2013-07-01                                               |*/
///*| Update       :                                                          |*/
///*[]-------------------------------------  Copyright(C) 2013 AMANO Corp.---[]*/
//void	creSaleNG_Add( void )
//{
//	short i;
//
//	if( cre_saleng.ken >= CRE_SALENG_MAX ){
//		return;	// 最大件数に達している
//	}
//
//	memset( &cre_saleng_work, 0, sizeof(CRE_SALENG) );
//	memcpy( &cre_saleng_work.back[0], &cre_saleng.back[0], sizeof(struct DATA_BK) * cre_saleng.ken );
//
//	i = (short)cre_saleng.ken;
//	cre_saleng_work.ken = (char)(i + 1);
//
//	cre_saleng_work.back[i].ryo = cre_uriage.back.ryo;														// ﾊﾞｯｸｱｯﾌﾟ料金
//	memcpy( &cre_saleng_work.back[i].time, &cre_uriage.back.time, sizeof(cre_saleng_work.back[0].time) );	// 精算時刻
//	cre_saleng_work.back[i].slip_no = cre_uriage.back.slip_no;												// ｸﾚｼﾞｯﾄｶｰﾄﾞ伝票番号
//	memcpy( &cre_saleng_work.back[i].AppNoChar[0], &cre_uriage.back.AppNoChar[0], 6 );						// ｸﾚｼﾞｯﾄｶｰﾄﾞ承認番号(英数字)
//	memcpy( &cre_saleng_work.back[i].shop_account_no[0], &cre_uriage.back.shop_account_no[0], 20 );			// ｸﾚｼﾞｯﾄｶｰﾄﾞ加盟店取引番号
//
//	nmisave( &cre_saleng, &cre_saleng_work, sizeof(CRE_SALENG) );
//
//	memset( &cre_uriage, 0, sizeof( cre_uriage ) );		// クレジット売上依頼データ
//
//	if( cre_saleng.ken >= CRE_SALENG_MAX ){
//		// 一件追加後に最大件数に達した場合
//		cre_ctl.Credit_Stop	= 1;		// ｸﾚｼﾞｯﾄ停止
//	}
//}
//
////[]-----------------------------------------------------------------------[]
////| 領収書 (集計)用エリア更新												|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creUpdatePayData 										|
////| PARAMETER    : void														|
////| RETURN VALUE : 0 : OK  / !=0 NG											|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creUpdatePayData( DATA_KIND_137_04 *RcvData )
//{
//	ushort	num;
//	int		i;
//	ulong	turisen_wk = 0;
//	//集計・領収書 用
//	memset( &ryo_buf.credit, 0, sizeof(credit_use) );
//
//	memcpy( &ryo_buf.credit.card_name[0], &RcvData->CompanyName[0], 12 );					// ｸﾚｼﾞｯﾄｶｰﾄﾞ会社略号
//	memcpy( &ryo_buf.credit.card_no[0], &RcvData->CardNo[0], 16 );
//	AesCBCDecrypt((uchar*)&ryo_buf.credit.card_no[0], 16);
//																							// ｸﾚｼﾞｯﾄｶｰﾄﾞ会員番号（右詰め）
//	ryo_buf.credit.slip_no				= RcvData->SlipNo;									// ｸﾚｼﾞｯﾄｶｰﾄﾞ伝票番号
//	ryo_buf.credit.pay_ryo				= creSeisanInfo.amount;								// クレジット決済額
//	ryo_buf.credit.CenterProcTime.Year	= RcvData->Pay.Year;								// ｸﾚｼﾞｯﾄｶｰﾄﾞｾﾝﾀｰ処理日時（秒あり）
//	ryo_buf.credit.CenterProcTime.Mon	= RcvData->Pay.Mon;									// 〃
//	ryo_buf.credit.CenterProcTime.Day	= RcvData->Pay.Day;									// 〃
//	ryo_buf.credit.CenterProcTime.Hour	= RcvData->Pay.Hour;								// 〃
//	ryo_buf.credit.CenterProcTime.Min	= RcvData->Pay.Min;									// 〃
//	ryo_buf.credit.CenterProcTime.Sec	= RcvData->Pay.Sec;									// 〃
//	memcpy( &ryo_buf.credit.ShopAccountNo[0], &RcvData->ShopAccountNo[0], 20 );				// 加盟店取引番号
//	
//	memset( &ryo_buf.credit.AppNoChar[0], 0x20, sizeof(ryo_buf.credit.AppNoChar) );
//	memcpy( &ryo_buf.credit.AppNoChar[0], &RcvData->AppNo[0], sizeof(RcvData->AppNo) );		// 端末識別番号
//	ryo_buf.credit.app_no = creConvAtoB( &RcvData->AppNo[0], 6 );							// ｸﾚｼﾞｯﾄｶｰﾄﾞ承認番号(通信用)
//	
//	memset( &ryo_buf.credit.CCT_Num[0], 0x20, sizeof(ryo_buf.credit.CCT_Num) );
//	memcpy( &ryo_buf.credit.CCT_Num[0], &RcvData->TerminalId[0], sizeof(RcvData->TerminalId) );	// 端末識別番号
//	
//	memcpy( &ryo_buf.credit.kid_code[0], &RcvData->KidCode[0], 6 );							// KID コード
//
//	//精算情報データ 用
//	NTNET_Data152Save((void *)(&ryo_buf.credit.pay_ryo), NTNET_152_CPAYRYO);				// クレジット決済額
//	NTNET_Data152Save((void *)(&ryo_buf.credit.card_no[0]), NTNET_152_CCARDNO);				// クレジットカード会員番号
//	NTNET_Data152Save((void *)(&ryo_buf.credit.CCT_Num[0]), NTNET_152_CCCTNUM);				// 端末識別番号
//	NTNET_Data152Save((void *)(&ryo_buf.credit.kid_code[0]), NTNET_152_CKID);				// ＫＩＤコード
//	NTNET_Data152Save((void *)(&ryo_buf.credit.app_no), NTNET_152_CAPPNO);
//// 仕様変更(S) K.Onodera 2016/11/04 精算情報データフォーマット対応
//	NTNET_Data152Save((void *)(&ryo_buf.credit.ShopAccountNo[0]), NTNET_152_CTRADENO);
//	NTNET_Data152Save((void *)(&ryo_buf.credit.slip_no), NTNET_152_SLIPNO);
//// 仕様変更(E) K.Onodera 2016/11/04 精算情報データフォーマット対応
//
//	// 売上情報
//	cre_uriage.back.ryo			= creSeisanInfo.amount;					// ｸﾚｼﾞｯﾄ売上金額（creSeisanInfo.amount と ryo_buf.credit.pay_ryo は同じ値）
//	cre_uriage.back.time.Year	= RcvData->Pay.Year;					// 精算 年
//	cre_uriage.back.time.Mon	= RcvData->Pay.Mon;						//      月
//	cre_uriage.back.time.Day	= RcvData->Pay.Day;						//      日
//	cre_uriage.back.time.Hour	= RcvData->Pay.Hour;					//      時
//	cre_uriage.back.time.Min	= RcvData->Pay.Min;						//      分
//	cre_uriage.back.slip_no		= RcvData->SlipNo;						// 端末処理通番（伝票№）
//	memcpy( &cre_uriage.back.AppNoChar[0], &RcvData->AppNo[0], 6 );		// ｸﾚｼﾞｯﾄｶｰﾄﾞ承認番号
//	memcpy( &cre_uriage.back.shop_account_no[0],
//		&RcvData->ShopAccountNo[0], CRE_SHOP_ACCOUNTBAN_MAX );			// ｸﾚｼﾞｯﾄｶｰﾄﾞ加盟店取引番号
//	cre_uriage.back.CMachineNo = 0;										// 駐車券機械№(入庫機械№)	0～255
//	cre_uriage.back.PayMethod2 = (uchar)OpeNtnetAddedInfo.PayMethod;	// 精算方法(0＝券なし精算/1＝駐車券精算/2＝定期券精算/3＝定期券併用精算/4＝紛失精算)
//	cre_uriage.back.PayClass = (uchar)OpeNtnetAddedInfo.PayClass;		// 処理区分(0＝精算/1＝再精算/2＝精算中止
//
//	for( i = 0; i < WTIK_USEMAX; i++ ){
//		if (PayData.DiscountData[i].DiscSyu == NTNET_GENGAKU) break;
//		if (PayData.DiscountData[i].DiscSyu == NTNET_FURIKAE) break;
//// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
////// MH322914(S) K.Onodera 2016/09/15 AI-V対応：振替精算
////		if (PayData.DiscountData[i].DiscSyu == NTNET_FURIKAE_2) break;
////// MH322914(E) K.Onodera 2016/09/15 AI-V対応：振替精算
//// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応
//	}
//	if( i != WTIK_USEMAX ){
//		// 減額/振替精算あり
//		cre_uriage.back.PayMode = 4;										// 精算モード(遠隔精算)
//	}else{
//		cre_uriage.back.PayMode = 0;										// 精算モード(自動精算)
//	}
//// 仕様変更(S) K.Onodera 2016/11/01 精算データフォーマット対応
//	for( i = 0; i < DETAIL_SYU_MAX; i++ ){
//		if (PayData.DetailData[i].DiscSyu == NTNET_FURIKAE_2) break;
//	}
//	if( i != DETAIL_SYU_MAX ){
//		cre_uriage.back.PayMode = 4;										// 精算モード(遠隔精算)
//	}
//// 仕様変更(E) K.Onodera 2016/11/01 精算データフォーマット対応
//
//	if( ryo_buf.pkiti == 0xffff ){// マルチ精算?
//		cre_uriage.back.FlapArea = (ushort)(LOCKMULTI.LockNo / 10000L);		// 区画(1～26)
//		cre_uriage.back.FlapParkNo = (ushort)(LOCKMULTI.LockNo % 10000L);	// 駐車位置№(1～9999)
//	}else{
//		num = ryo_buf.pkiti - 1;
//		cre_uriage.back.FlapArea = (ushort)LockInfo[num].area;				// 区画(1～26)
//		cre_uriage.back.FlapParkNo = (ushort)LockInfo[num].posi;			// 駐車位置№(1～9999)
//	}
//	if( ryo_buf.ryo_flg < 2 ){
//		cre_uriage.back.Price = ryo_buf.tyu_ryo;							// 駐車料金
//	}else{// 定期精算処理
//		cre_uriage.back.Price = ryo_buf.tei_ryo;							// 定期料金
//	}
//	cre_uriage.back.OptionPrice = 0;										// その他料金	0～
//	if(	ryo_buf.dsp_ryo < ( ryo_buf.nyukin + ryo_buf.credit.pay_ryo )){
//		// 入金と、クレジット認識のすれ違い時ryo_buf.nyukinには加算されるがこの時点でryo_buf.turisenに加算されないのでここで判断する
//		// 以下で余計に入金された分を算出する
//		turisen_wk = ( ryo_buf.nyukin + ryo_buf.credit.pay_ryo ) - ryo_buf.dsp_ryo;
//		cre_uriage.back.CashPrice = ryo_buf.nyukin - turisen_wk;// 現金売上(現金領収金額)
//	}else{
//		cre_uriage.back.CashPrice = ryo_buf.nyukin;			// 現金売上(現金領収金額)
//	}
//	// 割引額 = 駐車料金 - 現金売上額 - クレジット売上金額
//	cre_uriage.back.Discount = cre_uriage.back.Price - cre_uriage.back.CashPrice - ryo_buf.credit.pay_ryo;// 割引額(合算)
//	CountGet( PAYMENT_COUNT, &PayData.Oiban );								// 追番取得
//	CountFree(PAYMENT_COUNT);// ここで一旦返却、PayData_set()関数でも取得するため
//	cre_uriage.back.PayCount = CountSel( &PayData.Oiban );// 精算追番(0～99999)
//
//	// FT-4000では「精算媒体」に設定できるカードは定期券のみなので定期の使用有無で判断する
//	memset( &cre_uriage.back.Media.MediaCardNo[0], 0, sizeof( cre_uriage.back.Media.MediaCardNo ) );
//	memset( &cre_uriage.back.Media.MediaCardInfo[0], 0, sizeof( cre_uriage.back.Media.MediaCardInfo ) );
//	if( ryo_buf.ryo_flg >= 2 ){
//		cre_uriage.back.MediaKind = PayData.teiki.pkno_syu+1;// 定期券駐車場№
//		intoasl(&cre_uriage.back.Media.MediaCardNo[0], PayData.teiki.id, 5);		// カード番号	"0"～"9"、"A"～"Z"(英数字) 左詰め
//		intoasl(&cre_uriage.back.Media.MediaCardInfo[0], PayData.teiki.syu, 2);		// カード情報	"0"～"9"、"A"～"Z"(英数字) 左詰め
//	}else{
//		cre_uriage.back.MediaKind = 0;// 種別なし
//	}
//}
//
////[]-----------------------------------------------------------------------[]
////| TimeOut 処理 (OPETCBNO,Timer = 12)										|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creTimeOut												|
////| PARAMETER    : void                                                  	|
////| RETURN VALUE : 0 : OK  / !=0 NG											|
////|					-1:リトライ送信失敗										|
////|					-2:CRE電文応答タイムアウト								|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creTimeOut( void )
//{
//	short 	retsts = 0;
//
//	cre_ctl.Timeout_Announce = 0;	// 受信タイムアウト時のアナウンス可否フラグ
//
//
//	switch( cre_ctl.Status ){
//	case CRE_STS_WAIT_OPEN:			// 02待ち
//		creRegMonitor(OPMON_CRE_COMMAND_TIMEOUT, CRE_RCVCMD_OPEN, 0);	// モニタ登録
//		retsts = -2;
//		// １回目の開局リトライがタイムアウトしたときにエラー登録する
//		if( cre_ctl.Open.Cnt == 1 ){
//			if( cre_ctl.Initial_Connect_Done == 0 ) {	// 未開局の場合のみエラーと登録する
//				err_chk2( ERRMDL_CREDIT, ERR_CREDIT_OPEN_ERR, 0x01, 0, 0, NULL );	// エラー(発生)
//			}
//		}
//		// 外部照会中フラグON（与信問合せ送信する為に、開局コマンドを送信した）
//		queset( OPETCBNO, CRE_EVT_02_NG, 0, 0 );	// OpeMainへ通知（問合せ中の状態を解消する為）
//		if( cre_ctl.Initial_Connect_Done == 0 ) {	// 未開局の場合のみリトライする
//			creOpen_Enable();
//		}
//		break;
//	case CRE_STS_WAIT_CONFIRM:		// 04待ち
//		creRegMonitor(OPMON_CRE_COMMAND_TIMEOUT, CRE_RCVCMD_CONFIRM, 0);	// モニタ登録
//		retsts = -2;
//		cre_ctl.Timeout_Announce = 1;	// 受信タイムアウト時のアナウンス可否フラグ
//		break;
//	case CRE_STS_WAIT_SALES:		// 06待ち
//		creRegMonitor(OPMON_CRE_COMMAND_TIMEOUT, CRE_RCVCMD_SALES, 0);	// モニタ登録
//		retsts = -2;
//		// 外部照会中フラグONのとき
//		if( OPECTL.InquiryFlg == 1 ){
//			queset( OPETCBNO, CRE_EVT_06_TOUT, 0, 0 );	// OpeMainへ通知（問合せ中の状態を解消する為）
//		}
//		break;
//	case CRE_STS_WAIT_ONLINETEST:	// 08待ち
//		creRegMonitor(OPMON_CRE_COMMAND_TIMEOUT, CRE_RCVCMD_ONLINETEST, 0);	// モニタ登録
//		retsts = -2;
//		err_chk2( ERRMDL_CREDIT, ERR_CREDIT_CONNECTCHECK_ERR, 0x01, 0, 0, NULL );	// エラー(発生)
//		// 外部照会中フラグONのとき
//		if( OPECTL.InquiryFlg == 1 ){
//			queset( OPETCBNO, CRE_EVT_08_NG, 0, 0 );	// OpeMainへ通知（問合せ中の状態を解消する為）
//		}
//		break;
//	case CRE_STS_WAIT_RETURN:		// 0A待ち
//		creRegMonitor(OPMON_CRE_COMMAND_TIMEOUT, CRE_RCVCMD_RETURN, 0);	// モニタ登録
//		retsts = -2;
//		break;
//	}
//	if( retsts != 0 ){	// ﾘﾄﾗｲ以外
//		cre_ctl.Status = CRE_STS_IDLE;
//	}
//	return retsts;
//}
//
////[]-----------------------------------------------------------------------[]
////| CRE 電文戻り情報チェック&エラー処理										|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creResultCheck											|
////| PARAMETER    : void                                                  	|
////| RETURN VALUE : 0 : 正常, -1: フェーズずれ, -2: 整合性エラー				|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//short	creResultCheck( void )
//{
//	short	retsts = 0;
//	DATA_KIND_137_02	*Recv137_02;
//	DATA_KIND_137_04	*Recv137_04;
//	DATA_KIND_137_06	*Recv137_06;
//	DATA_KIND_137_08	*Recv137_08;
//	DATA_KIND_137_0A	*Recv137_0A;
//
//	Recv137_02 = (DATA_KIND_137_02 *)cre_ctl.RcvData;
//	Recv137_04 = (DATA_KIND_137_04 *)cre_ctl.RcvData;
//	Recv137_06 = (DATA_KIND_137_06 *)cre_ctl.RcvData;
//	Recv137_08 = (DATA_KIND_137_08 *)cre_ctl.RcvData;
//	Recv137_0A = (DATA_KIND_137_0A *)cre_ctl.RcvData;
//
//	cre_ctl.Timeout_Announce = 0;	// 受信タイムアウト時のアナウンス可否フラグ
//
//	switch( cre_ctl.Status ){
//	case CRE_STS_WAIT_OPEN:			// 02待ち
//		if( Recv137_02->Common.DataIdCode1 == CRE_RCVCMD_OPEN ){
//			// 処理追い番チェック
//			if( Recv137_02->SeqNo != cre_ctl.Save.SeqNo ){
//				retsts = -2;
//				break;
//			}
//			// 開局年月日チェック
//			if(
//				(Recv137_02->Proc.Year != cre_ctl.Save.Date.Year) ||
//				(Recv137_02->Proc.Mon  != cre_ctl.Save.Date.Mon ) ||
//				(Recv137_02->Proc.Day  != cre_ctl.Save.Date.Day ) ||
//				(Recv137_02->Proc.Hour != cre_ctl.Save.Date.Hour) ||
//				(Recv137_02->Proc.Min  != cre_ctl.Save.Date.Min ) ||
//				(Recv137_02->Proc.Sec  != cre_ctl.Save.Date.Sec )
//			){
//				retsts = -2;
//				break;
//			}
//		} else {
//			retsts = -1;
//		}
//		break;
//	case CRE_STS_WAIT_CONFIRM:		// 04待ち
//		if( Recv137_04->Common.DataIdCode1 == CRE_RCVCMD_CONFIRM ){
//			// 処理追い番チェック
//			if( Recv137_04->SeqNo != cre_ctl.Save.SeqNo ){
//				retsts = -2;
//				break;
//			}
//			// 精算年月日チェック
//			if(
//				(Recv137_04->Pay.Year != cre_ctl.Save.Date.Year) ||
//				(Recv137_04->Pay.Mon  != cre_ctl.Save.Date.Mon ) ||
//				(Recv137_04->Pay.Day  != cre_ctl.Save.Date.Day ) ||
//				(Recv137_04->Pay.Hour != cre_ctl.Save.Date.Hour) ||
//				(Recv137_04->Pay.Min  != cre_ctl.Save.Date.Min ) ||
//				(Recv137_04->Pay.Sec  != cre_ctl.Save.Date.Sec )
//			){
//				retsts = -2;
//				break;
//			}
//			// 端末処理通番（伝票番号）チェック
//			if( Recv137_04->SlipNo != cre_ctl.Save.SlipNo ){
//				retsts = -2;
//				break;
//			}
//			// 売上金額チェック
//			if( Recv137_04->Amount != cre_ctl.Save.Amount ){
//				retsts = -2;
//				break;
//			}
//		} else {
//			retsts = -1;
//		}
//		cre_ctl.Timeout_Announce = 1;	// 受信タイムアウト時のアナウンス可否フラグ
//		break;
//	case CRE_STS_WAIT_SALES:		// 06待ち
//		if( Recv137_06->Common.DataIdCode1 == CRE_RCVCMD_SALES ){
//			// 処理追い番チェック
//			if( Recv137_06->SeqNo != cre_ctl.Save.SeqNo ){
//				retsts = -2;
//				break;
//			}
//			// 精算年月日チェック
//			if(
//				(Recv137_06->Pay.Year != cre_ctl.Save.Date.Year) ||
//				(Recv137_06->Pay.Mon  != cre_ctl.Save.Date.Mon ) ||
//				(Recv137_06->Pay.Day  != cre_ctl.Save.Date.Day ) ||
//				(Recv137_06->Pay.Hour != cre_ctl.Save.Date.Hour) ||
//				(Recv137_06->Pay.Min  != cre_ctl.Save.Date.Min ) ||
//				(Recv137_06->Pay.Sec  != cre_ctl.Save.Date.Sec )
//			){
//				retsts = -2;
//				break;
//			}
//			// 端末処理通番（伝票番号）チェック
//			if( Recv137_06->SlipNo != cre_ctl.Save.SlipNo ){
//				retsts = -2;
//				break;
//			}
//			// 売上金額チェック
//			if( Recv137_06->Amount != cre_ctl.Save.Amount ){
//				retsts = -2;
//				break;
//			}
//			// 承認番号チェック
//			if( 0 != memcmp( &Recv137_06->AppNo[0], &cre_ctl.Save.AppNo[0], 6 ) ){
//				retsts = -2;
//				break;
//			}
//			// 加盟店取引番号チェック
//			if( 0 != memcmp( &Recv137_06->ShopAccountNo[0], &cre_ctl.Save.ShopAccountNo[0], 20 ) ){
//				retsts = -2;
//				break;
//			}
//		} else {
//			retsts = -1;
//		}
//		cre_ctl.Timeout_Announce = 1;	// 受信タイムアウト時のアナウンス可否フラグ
//		break;
//    case CRE_STS_WAIT_ONLINETEST:	// 08待ち
//		if( Recv137_08->Common.DataIdCode1 == CRE_RCVCMD_ONLINETEST ){
//			// 処理追い番チェック
//			if( Recv137_08->SeqNo != cre_ctl.Save.SeqNo ){
//				retsts = -2;
//				break;
//			}
//			// テスト種別
//			if( Recv137_08->TestKind != cre_ctl.Save.TestKind ){
//				retsts = -2;
//				break;
//			}
//		} else {
//			retsts = -1;
//		}
//		break;
//	case CRE_STS_WAIT_RETURN:		// 0A待ち
//		if( Recv137_0A->Common.DataIdCode1 == CRE_RCVCMD_RETURN ){
//			// 処理追い番チェック
//			if( Recv137_0A->SeqNo != cre_ctl.Save.SeqNo ){
//				retsts = -2;
//				break;
//			}
//			// 処理年月日チェック
//			if(
//				(Recv137_0A->Proc.Year != cre_ctl.Save.Date.Year) ||
//				(Recv137_0A->Proc.Mon  != cre_ctl.Save.Date.Mon ) ||
//				(Recv137_0A->Proc.Day  != cre_ctl.Save.Date.Day ) ||
//				(Recv137_0A->Proc.Hour != cre_ctl.Save.Date.Hour) ||
//				(Recv137_0A->Proc.Min  != cre_ctl.Save.Date.Min ) ||
//				(Recv137_0A->Proc.Sec  != cre_ctl.Save.Date.Sec )
//			){
//				retsts = -2;
//				break;
//			}
//			// 端末処理通番（伝票番号）チェック
//			if( Recv137_0A->SlipNo != cre_ctl.Save.SlipNo ){
//				retsts = -2;
//				break;
//			}
//			// 承認番号チェック
//			if( 0 != memcmp( &Recv137_0A->AppNo[0], &cre_ctl.Save.AppNo[0], 6 ) ){
//				retsts = -2;
//				break;
//			}
//			// 加盟店取引番号チェック
//			if( 0 != memcmp( &Recv137_0A->ShopAccountNo[0], &cre_ctl.Save.ShopAccountNo[0], 20 ) ){
//				retsts = -2;
//				break;
//			}
//		} else {
//			retsts = -1;
//		}
//		break;
//	}
//
//	return retsts;
//}
//
///*[]----------------------------------------------------------------------[]*/
///*| 右寄せにしてコピーし、先頭をスペースで埋める                           |*/
///*| ※１ スペース(0x20)だけでなく、ヌル(0x0)も削除対象とする               |*/
///*[]----------------------------------------------------------------------[]*/
///*| PARAMETER    : *dist    : 格納先ポインター                             |*/
///*|				   *src     : 格納元ポインター                             |*/
///*|				   dist_len : 格納先バッファ長                             |*/
///*|				   src_len  : 格納元バッファ長                             |*/
///*| RETURN VALUE : none                                                    |*/
///*[]----------------------------------------------------------------------[]*/
///*| Author       :                                                         |*/
///*| Date         :                                                         |*/
///*| Update       :                                                         |*/
///*[]------------------------------------- Copyright(C) 2013 AMANO Corp.---[]*/
//void creMemCpyRight(uchar *dist, uchar *src, ushort dist_len, ushort src_len)
//{
//	ushort	moji_left, moji_len, len, i;
//
//	moji_left	= 0;
//	moji_len	= 0;
//	memset( dist, 0x20, (size_t)dist_len );				// 格納先をスペースで初期化
//
//	len = src_len;
//	if ( dist_len < src_len ) {
//		len = dist_len;									// バッファ長の短い方に合わせる
//	}
//
//	if ( len < 2 ) return;								// 文字列長が2より小さい
//
//	// 文字列の先頭位置検索
//	for ( i = 0; i < len; i++ ) {
//		if (( src[i] != 0x0 ) && ( src[i] != 0x20 )) {	// 0x0と0x20は省く
//			break;
//		}
//	}
//	if (i >= len) return;								// 文字列が存在しない
//	moji_left = i;										// 文字列先頭位置
//
//	// 文字列の末尾位置検索
//	for ( i = len - 1; i > moji_left; i-- ) {
//		if (( src[i] != 0x0 ) && ( src[i] != 0x20 )) {	// 0x0と0x20は省く
//			break;
//		}
//	}
//	moji_len = (uchar)(( i + 1 ) - moji_left );				// 文字列長
//
//	memcpy( &dist[dist_len - moji_len], &src[moji_left], (size_t)moji_len );	// 文字列を右寄せでコピー
//}
//
////[]-----------------------------------------------------------------------[]
////| LCD表示とエラー時のアナウンスを行う										|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creMessageAnaOnOff 	 									|
////| PARAMETER    : OnOff(i) : 1:表示する 0:表示を戻す						|
////|				   num(i)   : cre 内部表示ﾊﾟﾀｰﾝ番号							|
////|								2 以上はエラー								|
////| RETURN VALUE : void														|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creMessageAnaOnOff( short OnOff, short num )
//{
//	// 前回の後処理
//	switch( cre_ctl.MessagePtnNum ){
//	case 1:
//		blink_end();
//	case 4:
//	case 6:
//		Vram_BkupRestr( 0, 6, 1 );	//Line=6 Restore
//		Vram_BkupRestr( 0, 7, 1 );	//Line=7 Restore
//		break;
//	case 2:
//	case 3:
//	case 7:
//	case 8:
//		Vram_BkupRestr( 0, 7, 1 );	//Line=7 Restore
//		break;
//	case 5:
//	case 9:
//		break;
//	}
//	cre_ctl.MessagePtnNum = 0;
//
//	// 今回の処理
//	if( OnOff == 1 ){
//		// 画面表示
//		switch( num ){
//		case 1:											// 
//			Vram_BkupRestr( 1, 6, 0 );	//Line=6 Backup
//			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
//			grachr( 6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[0] );	// Line=6:反転表示 "クレジットカード照会中です"
//															// Line=7:点滅表示 "しばらくお待ち下さい"
//			blink_reg( 7, 0, 30, 0, COLOR_DARKSLATEBLUE, OPE_CHR[7] );			// 
//			break;
//		case 2:											// 
//			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
//			grachr( 7, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, ERR_CHR[44] );		// Line=7:反転表示 " 只今、お取り扱いができません "
//			break;
//		case 3:											// "このカードはお取扱いできません"
//			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup 	   
//			grachr( 7, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, ERR_CHR[1] );		// Line=7:反転表示 "このカードは使えません"
//			break;
//		case 4:											// 
//			Vram_BkupRestr( 1, 6, 0 );	//Line=6 Backup
//			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
//			grachr( 6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[1] );	// Line=6:反転表示 "クレジットカードでの"
//			grachr( 7, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[2] );	// Line=7:反転表示 "精算限度額をこえています"
//			break;
//		case 5:											// 
//			grachr( 3, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[3] );	// Line=3:反転 "クレジットカードを取り、      "
//			grachr( 4, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[4] );	// Line=4:反転 "再度定期券を入れてください    "
//			grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[10] );	// Line=6: "中止する場合は、【取消】ボタン",
//			grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[11] );	// Line=7: "を押して下さい                ",
//			break;
//		case 6:											// 
//			Vram_BkupRestr( 1, 6, 0 );	//Line=6 Backup
//			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
//			grachr( 6, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[5] );	// Line=6:反転表示 "    このクレジットカードは    "
//			grachr( 7, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[6] );	// Line=7:反転表示 "          使えません          "
//			break;
//		case 7:											// 
//			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
//			grachr( 7, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[7] );	// Line=7:反転表示 "クレジットカードを入れて下さい"
//			break;
//		case 8:											// 
//			Vram_BkupRestr( 1, 7, 0 );  //Line=7 Backup
//			grachr( 7, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[8] );	// Line=7:反転表示 "最初と同じ定期券を入れて下さい"
//			break;
//		case 9:
//			grachr( 3, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[9] );	// Line=3:反転表示 "定期券を取り、                "
//			grachr( 4, 0, 30, 1, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[7] );	// Line=4:反転表示 "クレジットカードを入れて下さい"
//			grachr( 6, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[10] );	// Line=6: "中止する場合は、【取消】ボタン",
//			grachr( 7, 0, 30, 0, COLOR_BLACK, LCD_BLINK_OFF, OPE_CHR_CRE[11] );	// Line=7: "を押して下さい                ",
//			break;
//		}
//		// 音声案内
//		switch( num ){
//		case 1:
//			ope_anm( AVM_CREWAIT );		// ｱﾅｳﾝｽ 「しばらくお待ち下さい」
//			break;
//		case 3:
//		case 4:
//			ope_anm( AVM_CARD_ERR4 );	// ｱﾅｳﾝｽ 「このカードはお取り扱い出来ません。」
//			break;
//		case 2:
//		case 6:
//			ope_anm( AVM_CREJIT_NG );	// ｱﾅｳﾝｽ 「只今、クレジットカードはお取り扱い出来ません。」
//			break;
//		case 7:
//		case 8:
//			break;
//		}
//		cre_ctl.MessagePtnNum = num;	// 表示をけすときのために保存
//	}
//}
//
////[]-----------------------------------------------------------------------[]
////| ｸﾚｼﾞｯﾄ電文用追い番初期化												|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSeqNo_Init 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creSeqNo_Init( void )
//{
//	cre_ctl.SeqNo	= 0;	// 初期化（起動後初回のみ）
//}
//
////[]-----------------------------------------------------------------------[]
////| ｸﾚｼﾞｯﾄ電文用追い番カウント												|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSeqNo_Count 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//ushort	creSeqNo_Count( void )
//{
//	if( ++cre_ctl.SeqNo == 0 ){	// カウント(+1)
//		cre_ctl.SeqNo = CRE_SEQ_MIN;
//	}
//	return cre_ctl.SeqNo;
//}
//
////[]-----------------------------------------------------------------------[]
////| ｸﾚｼﾞｯﾄ電文用追い番の取得												|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSeqNo_Get 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//ushort	creSeqNo_Get( void )
//{
//	return cre_ctl.SeqNo;		// 現在値を返す
//}
//
////[]-----------------------------------------------------------------------[]
////| 端末処理通番（伝票番号）カウント										|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSlipNo_Count 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//ulong	creSlipNo_Count( void )
//{
//	if( ++cre_slipno > CRE_SLIP_MAX ){	// カウント(+1)
//		cre_slipno = CRE_SLIP_MIN;
//	}
//	return cre_slipno;
//}
//
////[]-----------------------------------------------------------------------[]
////| 端末処理通番（伝票番号）の取得											|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSlipNo_Get 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//ulong	creSlipNo_Get( void )
//{
//	return cre_slipno;				// 現在値を返す
//}
//
////[]-----------------------------------------------------------------------[]
////| 売上依頼データのリトライ制御データ初期化								|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSales_Init		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creSales_Init( void )
//{
//	memset( &cre_ctl.Sales, 0, sizeof(cre_ctl.Sales) );	// クリア
//
//	// 売上依頼データ再送リトライ間隔
//	cre_ctl.Sales.Wait[0] = CRE_RETRY_WAIT_0;	// 分
//	cre_ctl.Sales.Wait[1] = CRE_RETRY_WAIT_1;	// 分
//}
//
////[]-----------------------------------------------------------------------[]
////| 売上依頼データの経過時間カウントをリセット								|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSales_Reset	 			 							|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creSales_Reset( void )
//{
//	cre_ctl.Sales.Cnt = 0;
//	cre_ctl.Sales.Stage = 0;
//	cre_ctl.Sales.Min = 0;
//}
//
////[]-----------------------------------------------------------------------[]
////| 売上依頼データの経過時間をチェック										|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSales_Check		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//uchar	creSales_Check( void )
//{
//	uchar	ret = 0;
//
//	// 経過時間チェック
//	if( ++cre_ctl.Sales.Min >= cre_ctl.Sales.Wait[ cre_ctl.Sales.Stage ] ){
//		cre_ctl.Sales.Min = 0;		// 分をリセット
//		ret = 1;
//	}
//
//	return ret;
//}
//
////[]-----------------------------------------------------------------------[]
////| 売上依頼データ再送回数をカウント										|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSales_Count 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creSales_Count( void )
//{
//	if( ++cre_ctl.Sales.Cnt == CRE_RETRY_CNT_0 ){
//		cre_ctl.Sales.Stage = 1;		// 0:２回目まで、1:３回目以降
//	}
//	if( cre_ctl.Sales.Cnt >= CRE_RETRY_CNT_1 ){
//		cre_ctl.Sales.Cnt = CRE_RETRY_CNT_1;	// ３回以上はカウントしない
//	}
//	cre_ctl.Sales.Min = 0;				// 分をリセット
//}
//
////[]-----------------------------------------------------------------------[]
////| 売上依頼データ再送処理													|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creSales_Send 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creSales_Send( short kind )
//{
//	cre_ctl.SalesKind = kind;
//
//	// 売上依頼データ(05)を送信する
//	creSendData_SALES();
//}
//
////[]-----------------------------------------------------------------------[]
////| 開局コマンドのリトライ制御データ初期化									|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOpen_Init 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creOpen_Init( void )
//{
//	memset( &cre_ctl.Open, 0, sizeof(cre_ctl.Open) );	// クリア
//
//	// 開局コマンドリトライ間隔
//	cre_ctl.Open.Wait[0] = CRE_RETRY_WAIT_0;	// 分
//	cre_ctl.Open.Wait[1] = CRE_RETRY_WAIT_1;	// 分
//}
//
////[]-----------------------------------------------------------------------[]
////| 開局コマンドのリトライ送信許可											|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOpen_Enable 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creOpen_Enable( void )
//{
//	cre_ctl.Open.Min = 0;	// 送信可
//}
//
////[]-----------------------------------------------------------------------[]
////| 開局コマンドのリトライ送信禁止											|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOpen_Disable		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creOpen_Disable( void )
//{
//	cre_ctl.Open.Min = (char)-1;	// 送信不可
//}
//
////[]-----------------------------------------------------------------------[]
////| 開局コマンドの経過時間をチェック										|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOpen_Check 			 								|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//uchar	creOpen_Check( void )
//{
//	uchar	ret = 0;
//
//	if( cre_ctl.Open.Min != (char)-1 ){	// 送信可
//		// 経過時間チェック
//		if( ++cre_ctl.Open.Min >= cre_ctl.Open.Wait[ cre_ctl.Open.Stage ] ){
//			cre_ctl.Open.Min = 0;		// 分をリセット
//			ret = 1;
//		}
//	}
//
//	return ret;
//}
//
////[]-----------------------------------------------------------------------[]
////| 開局コマンド送信処理													|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOpen_Send 		 									|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creOpen_Send( void )
//{
//	cre_ctl.OpenKind = CRE_KIND_RETRY;	// 開局発生要因にリトライを設定
//
//	// 開局コマンド(01)を送信する
//	if( creSendData_OPEN() == -2 ){
//		return;
//	}
//
//	// 開局コマンド送信禁止に変更
//	creOpen_Disable();
//
//	if( ++cre_ctl.Open.Cnt == CRE_RETRY_CNT_0 ){
//		cre_ctl.Open.Stage = 1;		// 0:２回目まで、1:３回目以降
//	}
//	if( cre_ctl.Open.Cnt >= CRE_RETRY_CNT_1 ){
//		cre_ctl.Open.Cnt = CRE_RETRY_CNT_1;		// ３回以上はカウントしない
//	}
//}
//
////[]-----------------------------------------------------------------------[]
////| ｵﾝﾗｲﾝﾃｽﾄの制御データ初期化												|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOnlineTest_Init 		 								|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creOnlineTest_Init( void )
//{
//	memset( &cre_ctl.OnlineTest, 0, sizeof(cre_ctl.OnlineTest) );	// クリア
//
//	// ｵﾝﾗｲﾝﾃｽﾄの実行間隔
//	cre_ctl.OnlineTest.Wait = (uchar)prm_get( COM_PRM, S_CRE, 65, 2, 1 );	// 分
//	if( cre_ctl.OnlineTest.Wait != 0 &&
//		( cre_ctl.OnlineTest.Wait < 5 || cre_ctl.OnlineTest.Wait > 60) ){
//			/* 設定値が範囲外の場合は５５分とする(０はオンラインテストしない設定) */
//			cre_ctl.OnlineTest.Wait = 55;
//	}
//
//}
//
////[]-----------------------------------------------------------------------[]
////| ｵﾝﾗｲﾝﾃｽﾄの経過時間カウントをリセット									|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOnlineTest_Reset	 		 							|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creOnlineTest_Reset( void )
//{
//	cre_ctl.OnlineTest.Min = 0;
//}
//
////[]-----------------------------------------------------------------------[]
////| ｵﾝﾗｲﾝﾃｽﾄの経過時間をカウント											|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOnlineTest_Count 		 								|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creOnlineTest_Count( void )
//{
//	if(cre_ctl.OnlineTest.Wait != 0){
//		++cre_ctl.OnlineTest.Min;
//	}
//}
//
////[]-----------------------------------------------------------------------[]
////| ｵﾝﾗｲﾝﾃｽﾄの経過時間をチェック											|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOnlineTest_Check 		 								|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//uchar	creOnlineTest_Check( void )
//{
//	uchar	ret = 0;
//
//	if(cre_ctl.OnlineTest.Wait == 0){
//		/* 設定値が０の場合はオンラインテストしない */
//		return 0;
//	}
//	if( cre_ctl.OnlineTest.Min >= cre_ctl.OnlineTest.Wait ){
//		ret = 1;
//	}
//
//	return ret;
//}
//
////[]-----------------------------------------------------------------------[]
////| ｵﾝﾗｲﾝﾃｽﾄの送信処理														|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creOnlineTest_Send 		 								|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creOnlineTest_Send( void )
//{
//	// ｵﾝﾗｲﾝﾃｽﾄ(07)を送信する
//	if( creSendData_ONLINETEST() == -2 ){
//		return;
//	}
//
//	creOnlineTest_Reset();	// 無通信時間リセット
//}
//
////[]-----------------------------------------------------------------------[]
////| クレジット情報初期化													|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creInfoInit( void )										|
////| PARAMETER    : void														|
////| RETURN VALUE : void														|
////[]-----------------------------------------------------------------------[]
////| Date         : 2013-07-01												|
////| Update       :															|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creInfoInit( void )
//{
//	memset( &creSeisanInfo, 0, sizeof(creSeisanInfo) );	// クレジット精算情報クリア
//}
////[]-----------------------------------------------------------------------[]
/////	@brief			アスキーTOバイナリー（１０進)
////[]-----------------------------------------------------------------------[]
/////	@param[in]		asc_buf(i)	: 文字列バッファポインター
/////	@param[in]		len(i)		: 文字数(MAX=10)
/////	@return			void
////[]-----------------------------------------------------------------------[]
/////	@author			T.Okamoto
/////	@date			Create	: 2006/07/07<br>
/////					Update	: 
////[]------------------------------------- Copyright(C) 2005 AMANO Corp.----[]
//static	ulong creConvAtoB( uchar *asc_buf, ushort len )
//{
//	uchar	work[6];
//	ushort	i;
//
//	creMemCpyRight( &work[0], &asc_buf[0], sizeof( work ), len );
//	for( i = 0; i < len; i++ ){
//		if((work[i] < 0x30) || (work[i] > 0x39)){
//			work[i] = 0x30;
//		}
//	}
//	return astoinl( &work[0], (short)len );
//}
//
////[]-----------------------------------------------------------------------[]
////| クレジット関連モニタデータ登録											|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creRegMonitor( void )									|
////| PARAMETER    : ushort	code	モニタ種別コード						|
////|                uchar	type	コマンド種別(CRE_SNDCMD_*,CRE_RCVCMD_*) |
////|                ulong	info	付加情報								|
////| RETURN VALUE : void														|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creRegMonitor( ushort code, uchar type, ulong info )
//{
//	uchar	regType;										// モニタ登録選択
//	uchar	moni_wk[10];									// バイナリデータ登録用ワーク
//	
//	regType = (uchar)prm_get( COM_PRM, S_CRE, 67, 1, 1 );
//	if(code == OPMON_CRE_SEND_COMMAND || 					// コマンド送信
//		code == OPMON_CRE_RECV_RESPONSE ||					// コマンド応答
//		code == OPMON_CRE_COMMAND_TIMEOUT) {				// コマンドタイムアウト
//		switch(type) {
//		case CRE_SNDCMD_OPEN:								// 開局コマンド送信
//		case CRE_RCVCMD_OPEN:								// 開局結果受信
//			if(regType < 2) {
//				return;										// モニタを登録しない
//			}
//			break;
//		case CRE_SNDCMD_CONFIRM:							// 与信コマンド送信
//		case CRE_RCVCMD_CONFIRM:							// 与信結果受信
//		case CRE_SNDCMD_SALES:								// 売上依頼コマンド送信
//		case CRE_RCVCMD_SALES:								// 売上依頼結果受信
//			if(regType < 1) {
//				return;										// モニタを登録しない
//			}
//			break;
//		default:
//			if(regType < 3) {
//				return;										// モニタを登録しない
//			}
//		}
//	}
//	else {
//		return;
//	}
//	
//	switch(code) {
//	case OPMON_CRE_SEND_COMMAND:							// クレジットコマンド送信
//		wmonlg(code, 0, _MAKELONG((ushort)type, (ushort)info));
//		break;
//	case OPMON_CRE_RECV_RESPONSE:							// クレジットコマンド応答受信
//		memset(moni_wk, 0, sizeof(moni_wk));
//		moni_wk[5] = type;									// コマンド種別
//		memcpy(&moni_wk[6], &info, sizeof(info));			// 結果①②
//		wmonlg(code, moni_wk, 0);
//		break;
//	case OPMON_CRE_COMMAND_TIMEOUT:							// クレジットコマンドタイムアウト
//		wmonlg(code, 0, (ulong)type);
//		break;
//	default:
//		break;
//	}
//}
//
////[]-----------------------------------------------------------------------[]
////| 売上拒否データチェック													|
////[]-----------------------------------------------------------------------[]
////| MODULE NAME  : creCheckRejectSaleData( void )							|
////| PARAMETER    : void														|
////| RETURN VALUE : void														|
////[]------------------------------------- Copyright(C) 2013 AMANO Corp.----[]
//void	creCheckRejectSaleData( void )
//{
//	if( cre_saleng.ken > 0 ) {
//		err_chk2( ERRMDL_CREDIT, ERR_CREDIT_NO_ACCEPT, 0x01, 0, 0, NULL );	// E8861
//	}
//}
short creCtrl(short event)
{
	return 0;
}

void creCtrlInit(uchar flg)
{
}

void creInfoInit(void)
{
}

uchar creStatusCheck(void)
{
	return CRE_STATUS_DISABLED;
}

void creCheckRejectSaleData(void)
{
}

void creOneMinutesCheckProc(void)
{
}

void Credit_main(void)
{
}

uchar Credit_GetSeqFlag(void)
{
	return RAU_SND_SEQ_FLG_NORMAL;
}

void Credit_TcpConnReq_Clear(void)
{
}

void Credit_SetError(uchar ucErrCode)
{
}
// MH810103 GG119202(E) 不要機能削除(センタークレジット)
